#!/bin/bash
# Runs threadloop.c for many rounds per target and watches it: a target that finishes no round for a while counts as hung,
# its threads are dumped (gdb natively, winedbg under wine) and it is stopped.
# Usage: threadstress.sh <label> <rounds> [target ...]
# Targets: linux tsan wine64 wine32 (default: all). Results: demos/immediates/fpl_test_runs/<label>/threadstress/
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

label=${1:?usage: threadstress.sh <label> <rounds> [target ...]}
rounds=${2:?usage: threadstress.sh <label> <rounds> [target ...]}
shift 2
all_targets=(linux tsan wine64 wine32)
if [ $# -gt 0 ]; then
	targets=("$@")
else
	targets=("${all_targets[@]}")
fi

# One round takes a few seconds natively and under wine, ThreadSanitizer is slower
stall_seconds=180
poll_seconds=5
source_file="$FPL_TEST_TOOLS_DIR/threadloop.c"
test_defines=(-DFPL_IMPLEMENTATION -DFPL_LOGGING -DFT_IMPLEMENTATION)
test_includes=(-I "$FPL_REPO_ROOT" -I "$FPL_REPO_ROOT/demos/additions" -I "$FPL_REPO_ROOT/demos/dependencies")
linux_libs=(-lm -ldl -lpthread)

run_dir=$(fpl_test_prepare_run "$label")
fpl_test_start_xvfb "$run_dir" || exit 1
fpl_test_prepare_wine
stress_dir="$run_dir/threadstress"
mkdir -p "$stress_dir"
results_file="$stress_dir/results.tsv"
if [ ! -f "$results_file" ]; then
	echo -e "target\trounds_requested\trounds_done\tresult\tseconds" > "$results_file"
fi

build_target() {
	local target=$1
	local output=$2
	case "$target" in
		linux) gcc -std=c99 -g -O2 "${test_defines[@]}" "$source_file" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		tsan) gcc -std=c99 -g -O1 -fsanitize=thread "${test_defines[@]}" "$source_file" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		wine64) x86_64-w64-mingw32-gcc -std=c99 -g -O2 "${test_defines[@]}" "$source_file" "${test_includes[@]}" -o "$output" ;;
		wine32) i686-w64-mingw32-gcc -std=c99 -g -O2 "${test_defines[@]}" "$source_file" "${test_includes[@]}" -o "$output" ;;
		*) echo "unknown target $target" >&2; return 1 ;;
	esac
}

# Dumps every thread of a hung process into the target directory
dump_threads() {
	local target=$1
	local target_dir=$2
	local executable_name=$3
	if [[ "$target" == wine* ]]; then
		local windows_pid
		windows_pid=$(DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all winedbg --command "info proc" 2> /dev/null | grep -i "$executable_name" | awk '{print $1}' | head -1)
		if [ -n "$windows_pid" ]; then
			DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all timeout 120 winedbg --command "bt all" "0x$windows_pid" > "$target_dir/hang_backtraces.txt" 2>&1
		fi
	else
		local unix_pid
		unix_pid=$(pgrep -f "$target_dir/$executable_name" | head -1)
		if [ -n "$unix_pid" ]; then
			timeout 120 gdb -p "$unix_pid" -batch -ex "thread apply all bt" > "$target_dir/hang_backtraces.txt" 2>&1
		fi
	fi
}

for target in "${targets[@]}"; do
	target_dir="$stress_dir/$target"
	rm -rf "$target_dir"
	mkdir -p "$target_dir"
	executable_name="threadloop"
	if [[ "$target" == wine* ]]; then
		executable_name="threadloop.exe"
	fi
	if ! build_target "$target" "$target_dir/$executable_name" > "$target_dir/build.log" 2>&1; then
		echo -e "$target\t$rounds\t0\tBUILD-FAILED\t0" >> "$results_file"
		echo "$target: build FAILED"
		continue
	fi
	start_seconds=$SECONDS
	if [[ "$target" == wine* ]]; then
		(cd "$target_dir" && fpl_test_wine "./$executable_name" "$rounds" > run.log 2>&1) &
	else
		(cd "$target_dir" && TSAN_OPTIONS=halt_on_error=0 "./$executable_name" "$rounds" > run.log 2>&1) &
	fi
	runner_pid=$!
	last_round_count=0
	last_progress_seconds=$SECONDS
	result="OK"
	while kill -0 "$runner_pid" 2> /dev/null; do
		sleep "$poll_seconds"
		round_count=$(grep -c "^ROUND" "$target_dir/run.log" 2> /dev/null)
		if [ "$round_count" -ne "$last_round_count" ]; then
			last_round_count=$round_count
			last_progress_seconds=$SECONDS
		elif [ $((SECONDS - last_progress_seconds)) -ge "$stall_seconds" ]; then
			result="HANG"
			dump_threads "$target" "$target_dir" "$executable_name"
			pkill -f "$executable_name $rounds" 2> /dev/null
			kill "$runner_pid" 2> /dev/null
			break
		fi
	done
	wait "$runner_pid" 2> /dev/null
	rounds_done=$(grep -c "^ROUND" "$target_dir/run.log")
	if [ "$result" = "OK" ] && [ "$rounds_done" -ne "$rounds" ]; then
		result="EXIT-EARLY"
	fi
	if grep -q -E "Unhandled page fault|Unhandled exception|DEADLYSIGNAL|Segmentation fault" "$target_dir/run.log"; then
		result="CRASH"
	fi
	if grep -q -E "WARNING: ThreadSanitizer|Assertion failed|ASSERTION FAILED" "$target_dir/run.log"; then
		result="$result+REPORTS"
	fi
	elapsed_seconds=$((SECONDS - start_seconds))
	echo -e "$target\t$rounds\t$rounds_done\t$result\t$elapsed_seconds" >> "$results_file"
	echo "$target: $rounds_done/$rounds rounds, $result, ${elapsed_seconds}s"
done

fpl_test_stop_xvfb "$run_dir"
