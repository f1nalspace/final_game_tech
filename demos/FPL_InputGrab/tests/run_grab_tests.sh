#!/bin/bash
# Drives FPL_InputGrab with xdotool and checks its event log (--log-events), see plans/fpl_mouse_control_plan.md section 4.2.
#
# WARNING: The tests open small windows, move the real mouse pointer and send key presses to the focused window, including Alt+Tab.
# Tests that grab the keyboard or the mouse hold the real input for a few seconds. Every demo runs under "timeout -s KILL",
# and an X client that is killed loses all its grabs. Do not run this in a loop without telling the person at the desk.
#
# Usage: run_grab_tests.sh [--demo=<path>] [--tests=<name,...>] [--list]
#   --demo   FPL_InputGrab executable (default: Release build, then Debug build under demos/build/FPL_InputGrab)
#   --tests  only these tests, see --list
#   --list   print the test names and quit
# Logs and report.md go to demos/build/FPL_InputGrab/tests, which is ignored by git.
# Exit code is 0 when every test passed, 1 when at least one failed, 2 on usage or setup errors.

set -uo pipefail

scriptDirectory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
demosDirectory="$(cd "$scriptDirectory/../.." && pwd)"
outputDirectory="$demosDirectory/build/FPL_InputGrab/tests"
reportFile="$outputDirectory/report.md"

# --- Timing ------------------------------------------------------------------------------------------------------------------

# A demo quits by itself after this time (--timeout), no test needs longer
demoTimeoutSeconds=20
# A hanging demo is killed this long after its own timeout
killGraceSeconds=5
# Longest wait for an expected log line or window
waitSeconds=5
# Poll interval while waiting for a log line
pollIntervalSeconds=0.05
# Time the demo gets to pump an input before the log is checked for something that must NOT be there
settleSeconds=0.5
# A demo that dies of an X error is started again, see the X window id reuse race in plans/fpl_imageviewer_plan.md section 8
maximumTestAttempts=3

# --- Window placement (top left corner of the screen, out of the way) --------------------------------------------------------

firstWindowX=40
firstWindowY=60
secondWindowX=720
secondWindowY=60
windowSize="640x400"

# --- Tests -------------------------------------------------------------------------------------------------------------------

allTests=(mouse_move key_press_release focus_switch alt_tab_without_grab_switches no_stuck_alt_after_alt_tab)

# --- Arguments ---------------------------------------------------------------------------------------------------------------

demo=""
selectedTests="${allTests[*]}"
for argument in "$@"; do
	case "$argument" in
		--demo=*) demo="${argument#--demo=}" ;;
		--tests=*) selectedTests="${argument#--tests=}"; selectedTests="${selectedTests//,/ }" ;;
		--list) printf '%s\n' "${allTests[@]}"; exit 0 ;;
		*) echo "Unknown argument '$argument'" >&2; exit 2 ;;
	esac
done
if [ -z "$demo" ]; then
	for candidate in "$demosDirectory/build/FPL_InputGrab/Linux-x64-Release/FPL_InputGrab" "$demosDirectory/build/FPL_InputGrab/Linux-x64-Debug/FPL_InputGrab"; do
		if [ -x "$candidate" ]; then
			demo="$candidate"
			break
		fi
	done
fi
if [ -z "$demo" ] || [ ! -x "$demo" ]; then
	echo "FPL_InputGrab executable not found, build it or pass --demo=<path>" >&2
	exit 2
fi
for testName in $selectedTests; do
	if [[ " ${allTests[*]} " != *" $testName "* ]]; then
		echo "Unknown test '$testName', see --list" >&2
		exit 2
	fi
done
if [ -z "${DISPLAY:-}" ]; then
	echo "No X display (DISPLAY is not set)" >&2
	exit 2
fi
if ! command -v xdotool > /dev/null; then
	echo "xdotool is missing" >&2
	exit 2
fi
mkdir -p "$outputDirectory"

# --- Demo handling -----------------------------------------------------------------------------------------------------------

# Process ids of every "timeout" wrapper that was started, cleaned up on exit
startedWrapperPids=()
currentTest=""
failureReason=""
testNotes=""

KillAllDemos() {
	local wrapperPid
	for wrapperPid in "${startedWrapperPids[@]}"; do
		pkill -KILL -P "$wrapperPid" 2> /dev/null
		kill -KILL "$wrapperPid" 2> /dev/null
		# Reaps the job, so bash does not report it as killed
		wait "$wrapperPid" 2> /dev/null
	done
	startedWrapperPids=()
}
trap KillAllDemos EXIT

Fail() {
	failureReason="$1"
	return 1
}

Note() {
	if [ -n "$testNotes" ]; then
		testNotes="$testNotes; $1"
	else
		testNotes="$1"
	fi
}

LogLineCount() {
	wc -l < "$1"
}

# Waits until a line matching the extended regular expression shows up after line <mark>; prints the line
WaitForLogLine() {
	local logFile="$1"
	local mark="$2"
	local pattern="$3"
	local deadline=$((SECONDS + waitSeconds))
	local matchingLine
	while [ "$SECONDS" -le "$deadline" ]; do
		matchingLine=$(tail -n +"$((mark + 1))" "$logFile" | grep -m1 -E -- "$pattern")
		if [ -n "$matchingLine" ]; then
			echo "$matchingLine"
			return 0
		fi
		sleep "$pollIntervalSeconds"
	done
	return 1
}

ExpectLogLine() {
	local logFile="$1"
	local mark="$2"
	local pattern="$3"
	if ! WaitForLogLine "$logFile" "$mark" "$pattern" > /dev/null; then
		Fail "missing '$pattern' in $(basename "$logFile")"
		return 1
	fi
	return 0
}

# Starts a demo instance and places its window. Sets demoPid, demoWindow, demoLog and demoErrorLog.
StartDemo() {
	local instanceName="$1"
	local windowX="$2"
	local windowY="$3"
	shift 3
	local title="FPLInputGrabTest-$currentTest-$instanceName-$$"
	demoLog="$outputDirectory/$currentTest-$instanceName.log"
	demoErrorLog="$outputDirectory/$currentTest-$instanceName.err"
	timeout -s KILL "$((demoTimeoutSeconds + killGraceSeconds))" "$demo" --log-events --timeout="$demoTimeoutSeconds" --window="$windowSize" --title="$title" "$@" > "$demoLog" 2> "$demoErrorLog" &
	local wrapperPid=$!
	startedWrapperPids+=("$wrapperPid")
	if ! WaitForLogLine "$demoLog" 0 "demo ready" > /dev/null; then
		Fail "demo $instanceName did not start"
		return 1
	fi
	# FPL sets only _NET_WM_NAME and no WM_NAME, which "xdotool search --name" does not find, so the window is found by the process id
	demoPid=$(pgrep -P "$wrapperPid" | head -n 1)
	if [ -z "$demoPid" ]; then
		Fail "process of demo $instanceName not found"
		return 1
	fi
	demoWindow=$(timeout "$waitSeconds" xdotool search --sync --onlyvisible --pid "$demoPid" 2> /dev/null | head -n 1)
	if [ -z "$demoWindow" ]; then
		Fail "window of demo $instanceName not found"
		return 1
	fi
	xdotool windowmove --sync "$demoWindow" "$windowX" "$windowY" > /dev/null 2>&1
	return 0
}

# Activates a window and waits for the gotfocus line in its log
ActivateDemo() {
	local window="$1"
	local logFile="$2"
	local mark
	mark=$(LogLineCount "$logFile")
	xdotool windowactivate --sync "$window" > /dev/null 2>&1
	if ! WaitForLogLine "$logFile" "$mark" "window gotfocus" > /dev/null; then
		# Already focused windows do not get the event again
		local activeWindow
		activeWindow=$(xdotool getactivewindow 2> /dev/null)
		if [ "$activeWindow" != "$window" ]; then
			Fail "window $window did not get the focus"
			return 1
		fi
	fi
	return 0
}

# Ends a demo with a signal. Never with its Ctrl+Alt+Q hotkey from xdotool: xdotool sends the keys through XTEST into the focused window,
# the demo quits on the Q press, xdotool fails on the destroyed window and never releases Ctrl, Alt and Q, which then stay pressed in the X server.
StopDemo() {
	local processId="$1"
	kill -TERM "$processId" 2> /dev/null
}

# Lists the keycodes the X server holds as pressed, a key that stays pressed after a test would leak into the desktop session
ListPressedKeycodes() {
	python3 - << 'PYTHON_END'
from Xlib import display
xDisplay = display.Display()
keymap = xDisplay.query_keymap()
bitsPerByte = 8
pressedKeycodes = []
for byteIndex, byteValue in enumerate(keymap):
    for bitIndex in range(bitsPerByte):
        if byteValue & (1 << bitIndex):
            pressedKeycodes.append(str(byteIndex * bitsPerByte + bitIndex))
print(" ".join(pressedKeycodes))
PYTHON_END
}

# Releases the given keycodes through XTEST
ReleaseKeycodes() {
	python3 - "$@" << 'PYTHON_END'
import sys
from Xlib import X, display
from Xlib.ext import xtest
xDisplay = display.Display()
for keycodeText in sys.argv[1:]:
    xtest.fake_input(xDisplay, X.KeyRelease, int(keycodeText))
xDisplay.sync()
PYTHON_END
}

HadXError() {
	grep -q "X Error" "$outputDirectory/$currentTest"-*.err 2> /dev/null
}

# --- Tests -------------------------------------------------------------------------------------------------------------------

# Mouse positions arrive in window coordinates
Test_mouse_move() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 100 50
	ExpectLogLine "$logFile" "$mark" "mouse move x=100 y=50$" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 300 200
	ExpectLogLine "$logFile" "$mark" "mouse move x=300 y=200$" || return 1
	StopDemo "$processId"
	return 0
}

# A key press gives a press, a text input and a release event
Test_key_press_release() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool key a
	ExpectLogLine "$logFile" "$mark" "key button state=press .* key=A " || return 1
	ExpectLogLine "$logFile" "$mark" "key input code=97$" || return 1
	ExpectLogLine "$logFile" "$mark" "key button state=release .* key=A " || return 1
	StopDemo "$processId"
	return 0
}

# Activating another window gives lostfocus, activating it again gotfocus
Test_focus_switch() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowA" "$logA" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowB" "$logB" || return 1
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowA" "$logA" || return 1
	ExpectLogLine "$logA" "$mark" "window gotfocus" || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# Without a keyboard grab the window manager handles Alt+Tab and switches to the previously active window
Test_alt_tab_without_grab_switches() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark activeWindow
	ActivateDemo "$windowB" "$logB" || return 1
	ActivateDemo "$windowA" "$logA" || return 1
	mark=$(LogLineCount "$logA")
	xdotool key alt+Tab
	sleep "$settleSeconds"
	activeWindow=$(xdotool getactivewindow 2> /dev/null)
	if grep -q -E "key button state=press .* key=Tab " <(tail -n +"$((mark + 1))" "$logA"); then
		Note "the demo saw Tab"
	fi
	if [ "$activeWindow" != "$windowB" ]; then
		Fail "the active window is $activeWindow, expected demo B ($windowB)"
		return 1
	fi
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# After Alt+Tab away and back the next Alt press must be a press, not a repeat: the Alt release went to the window manager
Test_no_stuck_alt_after_alt_tab() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark altLine
	ActivateDemo "$windowB" "$logB" || return 1
	ActivateDemo "$windowA" "$logA" || return 1
	mark=$(LogLineCount "$logA")
	xdotool key alt+Tab
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	ActivateDemo "$windowA" "$logA" || return 1
	mark=$(LogLineCount "$logA")
	xdotool key alt
	altLine=$(WaitForLogLine "$logA" "$mark" "key button state=[a-z]+ .* key=[A-Za-z]*Alt[A-Za-z]* ")
	if [ -z "$altLine" ]; then
		Fail "no Alt key event after returning"
		return 1
	fi
	if [[ "$altLine" != *"state=press"* ]]; then
		Fail "first Alt after returning: '${altLine#t=* }'"
		return 1
	fi
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# --- Runner ------------------------------------------------------------------------------------------------------------------

passedCount=0
failedCount=0
reportRows=()

echo "FPL_InputGrab tests with $demo"
echo "The tests move the mouse pointer and send keys (including Alt+Tab) for a moment."
for testName in $selectedTests; do
	currentTest="$testName"
	attempt=1
	while :; do
		failureReason=""
		testNotes=""
		rm -f "$outputDirectory/$testName"-*.log "$outputDirectory/$testName"-*.err
		"Test_$testName"
		testResult=$?
		KillAllDemos
		stuckKeycodes=$(ListPressedKeycodes)
		if [ -n "$stuckKeycodes" ]; then
			ReleaseKeycodes $stuckKeycodes
			testResult=1
			failureReason="keycodes $stuckKeycodes stayed pressed after the test and were released"
		fi
		if [ "$testResult" -ne 0 ] && HadXError && [ "$attempt" -lt "$maximumTestAttempts" ]; then
			attempt=$((attempt + 1))
			continue
		fi
		break
	done
	detail="$testNotes"
	if [ "$testResult" -eq 0 ]; then
		result="PASS"
		passedCount=$((passedCount + 1))
	else
		result="FAIL"
		failedCount=$((failedCount + 1))
		if [ -n "$detail" ]; then
			detail="$failureReason; $detail"
		else
			detail="$failureReason"
		fi
	fi
	if [ "$attempt" -gt 1 ]; then
		detail="$detail (attempt $attempt)"
	fi
	printf '%-36s %s  %s\n' "$testName" "$result" "$detail"
	reportRows+=("| $testName | $result | $detail |")
done

{
	echo "# FPL_InputGrab test report"
	echo
	echo "Demo: \`$demo\`"
	echo
	echo "| Test | Result | Detail |"
	echo "|---|---|---|"
	printf '%s\n' "${reportRows[@]}"
	echo
	echo "Passed: $passedCount, failed: $failedCount"
} > "$reportFile"

echo "Passed: $passedCount, failed: $failedCount (report: $reportFile)"
if [ "$failedCount" -gt 0 ]; then
	exit 1
fi
exit 0
