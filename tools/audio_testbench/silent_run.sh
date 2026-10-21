#!/bin/bash
# Runs a command on the private X server with its sound going into a null sink, so nothing is heard and no window shows up on the desktop.
# Usage: silent_run.sh <label> <seconds> [--keys "<key>@<second> ..."] [--close] -- <command> [arguments ...]
# --close ends the command by closing its window (WM_DELETE_WINDOW, needs python-xlib) instead of SIGTERM, so its own shutdown runs and must end within 10 seconds.
# The command must play on the device "fpl_test_null" by itself (FPL_AudioPlayer: --device=fpl_test_null).
# If its stream shows up on any other sink, the command is killed at once and the run fails.
# Results: runs/<label>/run.log and runs/<label>/result.txt

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export LC_ALL=C
export LANG=C

label=${1:?usage: silent_run.sh <label> <seconds> [--keys "..."] [--close] -- <command> [arguments ...]}
run_seconds=${2:?missing seconds}
shift 2
key_plan=""
is_close=0
while [ $# -gt 0 ] && [ "$1" != "--" ]; do
	case "$1" in
		--keys) key_plan=$2; shift 2 ;;
		--close) is_close=1; shift ;;
		*) echo "unknown option $1" >&2; exit 2 ;;
	esac
done
if [ "$1" != "--" ]; then
	echo "missing -- before the command" >&2
	exit 2
fi
shift

display=":77"
screen="1600x1000x24"
null_sink_name="fpl_test_null"
routing_timeout_seconds=5
stop_timeout_seconds=10

run_dir="$testbench_dir/runs/$label"
rm -rf "$run_dir"
mkdir -p "$run_dir"
log_file="$run_dir/run.log"
result_file="$run_dir/result.txt"

started_xvfb_pid=""
loaded_module_id=""

cleanup() {
	if [ -n "$loaded_module_id" ]; then
		pactl unload-module "$loaded_module_id" 2> /dev/null
	fi
	if [ -n "$started_xvfb_pid" ]; then
		# Wait until it is gone, the next run would reuse a dying server otherwise and lose its X connection
		kill "$started_xvfb_pid" 2> /dev/null
		wait "$started_xvfb_pid" 2> /dev/null
	fi
}
trap cleanup EXIT

# Prints the sink of every sink input of the given process. Native PipeWire streams (FPL) carry the process id
# only on their client, Pulse clients (paplay) on the sink input itself, so both are matched.
sink_inputs_of_process() {
	local pid=$1
	local client_ids
	client_ids=$(pactl list clients | awk -v pid="$pid" '
		/^Client #/ { client = substr($2, 2) }
		/application.process.id = / { gsub(/"/, "", $3); if ($3 == pid) { print client } }')
	pactl list sink-inputs | awk -v pid="$pid" -v clients="$client_ids" '
		BEGIN { count = split(clients, list); for (i = 1; i <= count; ++i) { wanted[list[i]] = 1 } }
		function flush() { if (inBlock && (byPid || (client in wanted))) { print sink } }
		/^Sink Input #/ { flush(); inBlock = 1; sink = ""; client = ""; byPid = 0 }
		/^[ \t]*Sink: / { sink = $2 }
		/^[ \t]*Client: / { client = $2 }
		/application.process.id = / { gsub(/"/, "", $3); if ($3 == pid) { byPid = 1 } }
		END { flush() }'
}

# none, null or wrong
routing_of_process() {
	local sinks
	sinks=$(sink_inputs_of_process "$1")
	if [ -z "$sinks" ]; then
		echo "none"
		return
	fi
	for sink in $sinks; do
		if [ "$sink" != "$null_sink_index" ]; then
			echo "wrong $sink"
			return
		fi
	done
	echo "null"
}

# Private X server, reused when it already runs
if ! xdpyinfo -display "$display" > /dev/null 2>&1; then
	Xvfb "$display" -screen 0 "$screen" +extension GLX > /dev/null 2>&1 &
	started_xvfb_pid=$!
	for attempt in $(seq 1 50); do
		if xdpyinfo -display "$display" > /dev/null 2>&1; then
			break
		fi
		sleep 0.1
	done
fi

# Null sink, reused when it already exists
null_sink_index=$(pactl list short sinks | awk -v name="$null_sink_name" '$2 == name { print $1 }')
if [ -z "$null_sink_index" ]; then
	loaded_module_id=$(pactl load-module module-null-sink sink_name="$null_sink_name" sink_properties=device.description="$null_sink_name")
	null_sink_index=$(pactl list short sinks | awk -v name="$null_sink_name" '$2 == name { print $1 }')
fi
if [ -z "$null_sink_index" ]; then
	echo "result: FAILED (no null sink)" | tee "$result_file"
	exit 1
fi
pactl list short sink-inputs > "$run_dir/sink_inputs_before.txt"

# Line buffered, so the log keeps everything up to the SIGTERM at the end.
# stdbuf preloads its library before the ASan runtime, which ASan refuses unless the link order check is off.
export ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}verify_asan_link_order=0"
DISPLAY=$display stdbuf -oL -eL "$@" > "$log_file" 2>&1 &
command_pid=$!
start_seconds=$SECONDS
start_milliseconds=$(date +%s%3N)

# Every sink input of the command must be on the null sink, as soon as it appears
routing_state="none"
routing_deadline=$((SECONDS + routing_timeout_seconds))
while [ $SECONDS -lt $routing_deadline ] && kill -0 "$command_pid" 2> /dev/null; do
	routing_state=$(routing_of_process "$command_pid")
	if [ "$routing_state" != "none" ]; then
		break
	fi
	sleep 0.05
done
if [[ "$routing_state" == wrong* ]]; then
	kill -9 "$command_pid" 2> /dev/null
	echo "result: FAILED (stream was routed to sink ${routing_state#wrong } instead of the null sink $null_sink_index, command killed)" | tee "$result_file"
	exit 1
fi
if [ "$routing_state" == "none" ] && kill -0 "$command_pid" 2> /dev/null; then
	kill -9 "$command_pid" 2> /dev/null
	echo "result: FAILED (no stream of the command found within ${routing_timeout_seconds}s, its routing is unknown, command killed)" | tee "$result_file"
	exit 1
fi

# Checked again while the command runs, a stream may reconnect to another sink later
kill_on_wrong_routing() {
	local state
	state=$(routing_of_process "$command_pid")
	if [[ "$state" == wrong* ]]; then
		kill -9 "$command_pid" 2> /dev/null
		echo "result: FAILED (stream moved to sink ${state#wrong } instead of the null sink $null_sink_index, command killed)" | tee "$result_file"
		exit 1
	fi
}

# Keys go to the window of the command on the private X server only
if [ -n "$key_plan" ]; then
	window_id=""
	for attempt in $(seq 1 50); do
		window_id=$(DISPLAY=$display xdotool search --pid "$command_pid" 2> /dev/null | head -n 1)
		if [ -n "$window_id" ]; then
			break
		fi
		sleep 0.1
	done
	for key_entry in $key_plan; do
		key_name=${key_entry%@*}
		key_second=${key_entry#*@}
		key_due_milliseconds=$(awk -v second="$key_second" 'BEGIN { printf "%d", second * 1000 }')
		while [ $(( $(date +%s%3N) - start_milliseconds )) -lt "$key_due_milliseconds" ]; do
			sleep 0.02
		done
		if [ -n "$window_id" ]; then
			DISPLAY=$display xdotool windowfocus --sync "$window_id" key "$key_name" 2>> "$run_dir/xdotool.log"
			echo "key $key_name at $(( $(date +%s%3N) - start_milliseconds )) ms" >> "$run_dir/keys.txt"
		fi
	done
fi

while [ $((SECONDS - start_seconds)) -lt "$run_seconds" ] && kill -0 "$command_pid" 2> /dev/null; do
	kill_on_wrong_routing
	sleep 0.2
done

exited_early=0
if ! kill -0 "$command_pid" 2> /dev/null; then
	exited_early=1
fi
if [ $is_close -eq 1 ]; then
	close_window_id=$(DISPLAY=$display xdotool search --pid "$command_pid" 2> /dev/null | head -n 1)
	if [ -n "$close_window_id" ]; then
		# The same message a window manager sends for the close button, there is none on the private X server
		python3 -c '
import sys
from Xlib import display, protocol, X
connection = display.Display(sys.argv[1])
window = connection.create_resource_object("window", int(sys.argv[2]))
protocols_atom = connection.intern_atom("WM_PROTOCOLS")
delete_atom = connection.intern_atom("WM_DELETE_WINDOW")
event = protocol.event.ClientMessage(window=window, client_type=protocols_atom, data=(32, [delete_atom, X.CurrentTime, 0, 0, 0]))
window.send_event(event, event_mask=X.NoEventMask)
connection.flush()
' "$display" "$close_window_id" 2>> "$run_dir/close.log"
		echo "closed window $close_window_id at $(( $(date +%s%3N) - start_milliseconds )) ms" >> "$run_dir/close.log"
	else
		echo "no window to close, SIGTERM" >> "$run_dir/close.log"
		kill -TERM "$command_pid" 2> /dev/null
	fi
else
	kill -TERM "$command_pid" 2> /dev/null
fi
stop_start_milliseconds=$(date +%s%3N)
stop_deadline=$((SECONDS + stop_timeout_seconds))
while kill -0 "$command_pid" 2> /dev/null && [ $SECONDS -lt $stop_deadline ]; do
	sleep 0.02
done
stop_milliseconds=$(( $(date +%s%3N) - stop_start_milliseconds ))
hung=0
if kill -0 "$command_pid" 2> /dev/null; then
	hung=1
	kill -9 "$command_pid" 2> /dev/null
fi
wait "$command_pid" 2> /dev/null
exit_code=$?

pactl list short sink-inputs > "$run_dir/sink_inputs_after.txt"
problem_count=$(grep -c -i -E "assert|too slow|underrun|segmentation|sanitizer|runtime error|unhandled" "$log_file")

{
	echo "routing: $routing_state"
	echo "exited_early: $exited_early"
	echo "hung_on_stop: $hung"
	echo "exit_code: $exit_code"
	echo "stop_ms: $stop_milliseconds"
	echo "problem_lines: $problem_count"
} > "$result_file"
# A closed command has to end by itself and without an error
close_failed=0
if [ $is_close -eq 1 ] && [ "$exit_code" -ne 0 ]; then
	close_failed=1
fi
if [ "$routing_state" == "null" ] && [ $hung -eq 0 ] && [ "$problem_count" -eq 0 ] && [ $close_failed -eq 0 ]; then
	echo "result: ok" >> "$result_file"
else
	echo "result: FAILED" >> "$result_file"
fi
cat "$result_file"
