#!/bin/bash
# Drives FPL_InputGrab with xdotool and checks its event log (--log-events), see plans/fpl_mouse_control_plan.md section 4.2.
#
# WARNING: The tests open small windows, move the real mouse pointer and send key presses to the focused window, including Alt+Tab.
# Tests that grab the keyboard or the mouse hold the real input for a few seconds. Every demo runs under "timeout -s KILL",
# and an X client that is killed loses all its grabs. Do not run this in a loop without telling the person at the desk.
# The mouse grab tests lock the real pointer inside a demo window, and the retry tests let a small python-xlib client hold the pointer
# or the keyboard for two seconds, clicks and keys go nowhere then. The keyboard grab tests send Alt+Tab, Alt+F4 and Super,
# a keyboard grab that does not work lets KWin switch or close windows or open its launcher.
#
# Usage: run_grab_tests.sh [--demo=<path>] [--fallback-demo=<path>] [--tests=<name,...>] [--list]
#   --demo            FPL_InputGrab executable (default: Release build, then Debug build under demos/build/FPL_InputGrab)
#   --fallback-demo   FPL_InputGrab_NoXInput2 executable for the relative_fallback_* tests (default: next to the demo)
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
# The self test starts one second after the focus and waits up to two seconds per target
selfTestWaitSeconds=15
# A position to the right of the window, in window coordinates, for leaving the window
outsideOffsetX=700
# A demo that dies of an X error is started again, see the X window id reuse race in plans/fpl_imageviewer_plan.md section 8
maximumTestAttempts=3
# Distance of pointer targets outside of the client area, far enough to leave it, close enough to stay away from the screen edges and their corner actions
outsideDistance=30
# Another client holds the pointer or the keyboard this long in the retry tests, the demo must get the focus meanwhile
inputHoldSeconds=2
# Relative moves of the relative mode tests and their sums, small enough to never reach an edge of the window from its center
relativeMoves=("10 0" "0 7" "-4 -3" "25 12" "-6 20")
relativeSumX=25
relativeSumY=36
relativeLargestStep=25
# A position inside the window where the relative mode starts in the tests that end it again
startInsideX=100
startInsideY=50

# --- Window placement (top left corner of the screen, out of the way) --------------------------------------------------------

firstWindowX=40
firstWindowY=60
secondWindowX=720
secondWindowY=60
windowSize="640x400"
# The window size the resize test sets, smaller than the start size, so a cage that does not follow would let the pointer out
resizedWidth=400
resizedHeight=300

# --- Tests -------------------------------------------------------------------------------------------------------------------

allTests=(mouse_move move_deltas no_delta_after_reenter warp_selftest key_press_release focus_switch focus_loss_releases_keys alt_tab_without_grab_switches no_stuck_alt_after_alt_tab mouse_grab_confines mouse_grab_released_on_focus_loss mouse_grab_restored_on_focus_gain mouse_grab_follows_resize mouse_grab_retries_when_already_grabbed warp_limited_by_mouse_grab relative_delta_sum relative_hides_and_keeps_pointer relative_returns_to_start relative_warp_moves_start relative_released_on_focus_loss relative_fallback_delta_sum relative_fallback_hides_and_keeps_pointer relative_fallback_returns_to_start keyboard_grab_keeps_alt_tab keyboard_grab_keeps_alt_f4 keyboard_grab_text_input keyboard_grab_released_on_focus_loss keyboard_grab_retries_when_already_grabbed keyboard_grab_keeps_super)

# --- Arguments ---------------------------------------------------------------------------------------------------------------

demo=""
fallbackDemo=""
selectedTests="${allTests[*]}"
for argument in "$@"; do
	case "$argument" in
		--demo=*) demo="${argument#--demo=}" ;;
		--fallback-demo=*) fallbackDemo="${argument#--fallback-demo=}" ;;
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
if [ -z "$fallbackDemo" ]; then
	fallbackDemo="$(dirname "$demo")/FPL_InputGrab_NoXInput2"
fi
if [[ " $selectedTests " == *" relative_fallback_"* ]] && [ ! -x "$fallbackDemo" ]; then
	echo "FPL_InputGrab_NoXInput2 executable not found, build it or pass --fallback-demo=<path>" >&2
	exit 2
fi
# The executable StartDemo runs, the fallback tests set it to the build without XInput2
demoBinary="$demo"
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

# Prints the line number of the first line after <mark> that matches the extended regular expression, nothing when there is none
LineNumberOf() {
	local logFile="$1"
	local mark="$2"
	local pattern="$3"
	local relativeLine
	relativeLine=$(tail -n +"$((mark + 1))" "$logFile" | grep -n -m1 -E -- "$pattern" | cut -d: -f1)
	if [ -n "$relativeLine" ]; then
		echo "$((mark + relativeLine))"
	fi
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
	timeout -s KILL "$((demoTimeoutSeconds + killGraceSeconds))" "$demoBinary" --log-events --timeout="$demoTimeoutSeconds" --window="$windowSize" --title="$title" "$@" > "$demoLog" 2> "$demoErrorLog" &
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

# --- Pointer helpers ---------------------------------------------------------------------------------------------------------

# Reads the client area of a window in screen coordinates into areaLeft, areaTop, areaWidth and areaHeight.
# xdotool translates the origin of a reparented window to the root, which is the origin of the client area.
ReadClientArea() {
	local window="$1"
	local WINDOW X Y WIDTH HEIGHT SCREEN
	eval "$(xdotool getwindowgeometry --shell "$window")"
	areaLeft="$X"
	areaTop="$Y"
	areaWidth="$WIDTH"
	areaHeight="$HEIGHT"
}

# Reads the pointer position in screen coordinates into pointerX and pointerY
ReadPointer() {
	local X Y SCREEN WINDOW
	eval "$(xdotool getmouselocation --shell)"
	pointerX="$X"
	pointerY="$Y"
}

IsPointerInsideArea() {
	local areaRight=$((areaLeft + areaWidth))
	local areaBottom=$((areaTop + areaHeight))
	[ "$pointerX" -ge "$areaLeft" ] && [ "$pointerX" -lt "$areaRight" ] && [ "$pointerY" -ge "$areaTop" ] && [ "$pointerY" -lt "$areaBottom" ]
}

# Moves the pointer to a screen position outside of the client area read last, until the grab keeps it inside. Tried several times, the grab follows the focus asynchronously.
ExpectPointerConfined() {
	local targetX="$1"
	local targetY="$2"
	local deadline=$((SECONDS + waitSeconds))
	while [ "$SECONDS" -le "$deadline" ]; do
		xdotool mousemove "$targetX" "$targetY"
		ReadPointer
		if IsPointerInsideArea; then
			return 0
		fi
		sleep "$pollIntervalSeconds"
	done
	Fail "pointer at $pointerX,$pointerY after moving to $targetX,$targetY, outside of the client area $areaLeft,$areaTop ${areaWidth}x$areaHeight"
	return 1
}

# Moves the pointer to a screen position until it arrives there, no grab holds it back anymore
ExpectPointerFree() {
	local targetX="$1"
	local targetY="$2"
	local deadline=$((SECONDS + waitSeconds))
	while [ "$SECONDS" -le "$deadline" ]; do
		xdotool mousemove "$targetX" "$targetY"
		ReadPointer
		if [ "$pointerX" -eq "$targetX" ] && [ "$pointerY" -eq "$targetY" ]; then
			return 0
		fi
		sleep "$pollIntervalSeconds"
	done
	Fail "pointer stays at $pointerX,$pointerY instead of $targetX,$targetY, the grab was not released"
	return 1
}

# Holds an active grab of another client on the root window for the given time, "pointer" or "keyboard". Sets holderLog, which gets "grab <status>" and "released".
StartInputHolder() {
	local device="$1"
	local holdSeconds="$2"
	holderLog="$outputDirectory/$currentTest-holder.log"
	timeout -s KILL "$((holdSeconds + killGraceSeconds))" python3 - "$device" "$holdSeconds" > "$holderLog" 2>&1 << 'PYTHON_END' &
import sys
import time
from Xlib import X, display
device = sys.argv[1]
holdSeconds = float(sys.argv[2])
xDisplay = display.Display()
root = xDisplay.screen().root
if device == "keyboard":
    status = root.grab_keyboard(False, X.GrabModeAsync, X.GrabModeAsync, X.CurrentTime)
else:
    eventMask = X.ButtonPressMask | X.ButtonReleaseMask | X.PointerMotionMask
    status = root.grab_pointer(False, eventMask, X.GrabModeAsync, X.GrabModeAsync, X.NONE, X.NONE, X.CurrentTime)
xDisplay.sync()
print("grab %d" % status, flush=True)
time.sleep(holdSeconds)
if device == "keyboard":
    xDisplay.ungrab_keyboard(X.CurrentTime)
else:
    xDisplay.ungrab_pointer(X.CurrentTime)
xDisplay.sync()
print("released", flush=True)
PYTHON_END
	startedWrapperPids+=("$!")
	holderPid="$!"
}

# Prints "invisible" when the current cursor image has no visible pixel (XFixes), "visible" otherwise
ReadCursorVisibility() {
	python3 - << 'PYTHON_END'
from Xlib import display
xDisplay = display.Display()
xDisplay.xfixes_query_version()
cursorImage = xDisplay.xfixes_get_cursor_image(xDisplay.screen().root)
alphaShift = 24
isVisible = any((pixel >> alphaShift) != 0 for pixel in cursorImage.cursor_image)
print("visible" if isVisible else "invisible")
PYTHON_END
}

# Sends the relative moves of the relative mode tests through XTEST
SendRelativeMoves() {
	local move
	for move in "${relativeMoves[@]}"; do
		# shellcheck disable=SC2086
		xdotool mousemove_relative -- $move
	done
}

# Waits until the move events after <mark> add up to the given sums. Sets movedSumX, movedSumY and movedPositions (the distinct x,y of those events).
ExpectMoveSum() {
	local logFile="$1"
	local mark="$2"
	local expectedX="$3"
	local expectedY="$4"
	local deadline=$((SECONDS + waitSeconds))
	local sums
	while :; do
		sums=$(tail -n +"$((mark + 1))" "$logFile" | awk '/ mouse move / { for (i = 1; i <= NF; ++i) { split($i, pair, "="); if (pair[1] == "dx") { sumX += pair[2] } else if (pair[1] == "dy") { sumY += pair[2] } else if (pair[1] == "x") { x = pair[2] } else if (pair[1] == "y") { position[x "," pair[2]] = 1 } } } END { list = ""; for (p in position) { list = list " " p }; printf "%d %d%s", sumX, sumY, list }')
		read -r movedSumX movedSumY movedPositions <<< "$sums"
		if [ "$movedSumX" -eq "$expectedX" ] && [ "$movedSumY" -eq "$expectedY" ]; then
			return 0
		fi
		if [ "$SECONDS" -gt "$deadline" ]; then
			Fail "move deltas add up to $movedSumX,$movedSumY instead of $expectedX,$expectedY"
			return 1
		fi
		sleep "$pollIntervalSeconds"
	done
}

# Fails when a move event after the first gotfocus has a step larger than the given one, like a jump when the relative mode starts
ExpectNoJumpAfterFocus() {
	local logFile="$1"
	local largestStep="$2"
	local focusLine largestSeen
	focusLine=$(LineNumberOf "$logFile" 0 "window gotfocus")
	if [ -z "$focusLine" ]; then
		Fail "no gotfocus in $(basename "$logFile")"
		return 1
	fi
	largestSeen=$(tail -n +"$focusLine" "$logFile" | awk '/ mouse move / { for (i = 1; i <= NF; ++i) { split($i, pair, "="); if (pair[1] == "dx" || pair[1] == "dy") { step = pair[2] < 0 ? -pair[2] : pair[2]; if (step > largest) { largest = step } } } } END { printf "%d", largest }')
	if [ "$largestSeen" -gt "$largestStep" ]; then
		Fail "a move event after gotfocus has a step of $largestSeen, the largest move sent was $largestStep"
		return 1
	fi
	return 0
}

# --- Tests -------------------------------------------------------------------------------------------------------------------

# Mouse positions arrive in window coordinates
Test_mouse_move() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 100 50
	ExpectLogLine "$logFile" "$mark" "mouse move x=100 y=50 " || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 300 200
	ExpectLogLine "$logFile" "$mark" "mouse move x=300 y=200 " || return 1
	StopDemo "$processId"
	return 0
}

# A move inside the window carries the difference to the previous move
Test_move_deltas() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 100 50
	ExpectLogLine "$logFile" "$mark" "mouse move x=100 y=50 " || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 130 70
	ExpectLogLine "$logFile" "$mark" "mouse move x=130 y=70 dx=30 dy=20$" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 110 90
	ExpectLogLine "$logFile" "$mark" "mouse move x=110 y=90 dx=-20 dy=20$" || return 1
	StopDemo "$processId"
	return 0
}

# The first move after the cursor came back into the window has no delta, the way outside is not movement inside the window
Test_no_delta_after_reenter() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 100 50
	ExpectLogLine "$logFile" "$mark" "mouse move x=100 y=50 " || return 1
	# Outside of the window, to the right of it
	xdotool mousemove --window "$window" "$((outsideOffsetX))" 50
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" 300 200
	ExpectLogLine "$logFile" "$mark" "mouse move x=300 y=200 dx=0 dy=0$" || return 1
	StopDemo "$processId"
	return 0
}

# Runs the self test of the demo with the given extra parameters and checks its result
RunSelfTest() {
	StartDemo A "$firstWindowX" "$firstWindowY" --selftest "$@" || return 1
	local window="$demoWindow" logFile="$demoLog" resultLine
	# A new window usually has the focus already, and the self test may be over before an activation could be confirmed, so the activation is not checked here
	xdotool windowactivate "$window" > /dev/null 2>&1
	resultLine=$(waitSeconds=$selfTestWaitSeconds WaitForLogLine "$logFile" 0 "selftest (pass|fail)")
	if [ -z "$resultLine" ]; then
		Fail "no self test result"
		return 1
	fi
	if [[ "$resultLine" != *"selftest pass"* ]]; then
		Fail "$(grep -m1 "result=fail" "$logFile" | cut -d' ' -f2-)"
		return 1
	fi
	return 0
}

# The demo warps the cursor to the corners and the center and checks the move events and the screen positions itself
Test_warp_selftest() {
	RunSelfTest
}

# With the mouse grab the self test also warps outside of the window, the cursor must end at the nearest corner of the client area
Test_warp_limited_by_mouse_grab() {
	RunSelfTest --mouse-grab
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

# A key that is still held when the window loses the focus gets its release before the lostfocus event
Test_focus_loss_releases_keys() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark releaseLine lostFocusLine
	ActivateDemo "$windowA" "$logA" || return 1
	mark=$(LogLineCount "$logA")
	xdotool keydown a
	ExpectLogLine "$logA" "$mark" "key button state=press .* key=A " || { xdotool keyup a; return 1; }
	ActivateDemo "$windowB" "$logB" || { xdotool keyup a; return 1; }
	xdotool keyup a
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	releaseLine=$(LineNumberOf "$logA" "$mark" "key button state=release .* key=A ")
	lostFocusLine=$(LineNumberOf "$logA" "$mark" "window lostfocus")
	if [ -z "$releaseLine" ]; then
		Fail "no release of A in demo A"
		return 1
	fi
	if [ "$releaseLine" -gt "$lostFocusLine" ]; then
		Fail "the release of A came after lostfocus"
		return 1
	fi
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
	local releaseLine lostFocusLine
	releaseLine=$(LineNumberOf "$logA" "$mark" "key button state=release .* key=LeftAlt ")
	lostFocusLine=$(LineNumberOf "$logA" "$mark" "window lostfocus")
	if [ -z "$releaseLine" ] || [ "$releaseLine" -gt "$lostFocusLine" ]; then
		Fail "Alt was not released before lostfocus"
		return 1
	fi
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

# The mouse grab keeps the pointer inside the client area, above left and below right of it
Test_mouse_grab_confines() {
	StartDemo A "$firstWindowX" "$firstWindowY" --mouse-grab || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid"
	ActivateDemo "$window" "$logFile" || return 1
	ReadClientArea "$window"
	ExpectPointerConfined "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	ExpectPointerConfined "$((areaLeft + areaWidth + outsideDistance))" "$((areaTop + areaHeight + outsideDistance))" || return 1
	StopDemo "$processId"
	return 0
}

# Another window gets the focus: the grab is released and the pointer leaves the window, the request stays
Test_mouse_grab_released_on_focus_loss() {
	StartDemo A "$firstWindowX" "$firstWindowY" --mouse-grab || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowA" "$logA" || return 1
	ReadClientArea "$windowA"
	ExpectPointerConfined "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowB" "$logB" || return 1
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	ExpectPointerFree "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# The grab comes back when the window gets the focus again
Test_mouse_grab_restored_on_focus_gain() {
	StartDemo A "$firstWindowX" "$firstWindowY" --mouse-grab || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid"
	ActivateDemo "$windowB" "$logB" || return 1
	ReadClientArea "$windowA"
	ExpectPointerFree "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	ActivateDemo "$windowA" "$logA" || return 1
	ExpectPointerConfined "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# The cage follows the client area when the window gets smaller
Test_mouse_grab_follows_resize() {
	StartDemo A "$firstWindowX" "$firstWindowY" --mouse-grab || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	mark=$(LogLineCount "$logFile")
	xdotool windowsize "$window" "$resizedWidth" "$resizedHeight"
	ExpectLogLine "$logFile" "$mark" "window resized w=$resizedWidth h=$resizedHeight$" || return 1
	ReadClientArea "$window"
	if [ "$areaWidth" -ne "$resizedWidth" ] || [ "$areaHeight" -ne "$resizedHeight" ]; then
		Fail "client area is ${areaWidth}x$areaHeight after the resize, expected ${resizedWidth}x$resizedHeight"
		return 1
	fi
	ExpectPointerConfined "$((areaLeft + areaWidth + outsideDistance))" "$((areaTop + areaHeight + outsideDistance))" || return 1
	StopDemo "$processId"
	return 0
}

# Another client holds the pointer when the window gets the focus: XGrabPointer fails with AlreadyGrabbed, FPL tries again and gets the grab once the other client lets go
Test_mouse_grab_retries_when_already_grabbed() {
	StartDemo A "$firstWindowX" "$firstWindowY" --mouse-grab --log-fpl || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowB" "$logB" || return 1
	ReadClientArea "$windowA"
	StartInputHolder pointer "$inputHoldSeconds"
	ExpectLogLine "$holderLog" 0 "^grab 0$" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowA" "$logA" || return 1
	ExpectLogLine "$logA" "$mark" "XGrabPointer failed with AlreadyGrabbed" || return 1
	ExpectLogLine "$holderLog" 0 "^released$" || return 1
	ExpectPointerConfined "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# The relative mode reports the raw movement: a known series of XTEST moves adds up to the same sum, and every move event keeps the position where the mode started
Test_relative_delta_sum() {
	StartDemo A "$firstWindowX" "$firstWindowY" --relative-mouse || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	SendRelativeMoves
	ExpectMoveSum "$logFile" "$mark" "$relativeSumX" "$relativeSumY" || return 1
	if [ "$(wc -w <<< "$movedPositions")" -ne 1 ]; then
		Fail "the move events carry several positions ($movedPositions), expected the one where the mode started"
		return 1
	fi
	ExpectNoJumpAfterFocus "$logFile" "$relativeLargestStep" || return 1
	StopDemo "$processId"
	return 0
}

# The cursor is invisible and can not leave the window
Test_relative_hides_and_keeps_pointer() {
	StartDemo A "$firstWindowX" "$firstWindowY" --relative-mouse || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" visibility
	ActivateDemo "$window" "$logFile" || return 1
	ReadClientArea "$window"
	ExpectPointerConfined "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	ExpectPointerConfined "$((areaLeft + areaWidth + outsideDistance))" "$((areaTop + areaHeight + outsideDistance))" || return 1
	visibility=$(ReadCursorVisibility)
	if [ "$visibility" != "invisible" ]; then
		Fail "the cursor is $visibility in the relative mode"
		return 1
	fi
	StopDemo "$processId"
	return 0
}

# Ending the relative mode puts the cursor back where the mode started, and it is visible again
Test_relative_returns_to_start() {
	StartDemo A "$firstWindowX" "$firstWindowY" || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark visibility
	ActivateDemo "$window" "$logFile" || return 1
	ReadClientArea "$window"
	mark=$(LogLineCount "$logFile")
	xdotool mousemove --window "$window" "$startInsideX" "$startInsideY"
	ExpectLogLine "$logFile" "$mark" "mouse move x=$startInsideX y=$startInsideY " || return 1
	mark=$(LogLineCount "$logFile")
	xdotool key ctrl+alt+r
	ExpectLogLine "$logFile" "$mark" "grab relative requested=on result=1" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	SendRelativeMoves
	ExpectMoveSum "$logFile" "$mark" "$relativeSumX" "$relativeSumY" || return 1
	if [ "$movedPositions" != "$startInsideX,$startInsideY" ]; then
		Fail "the move events carry $movedPositions, expected the start position $startInsideX,$startInsideY"
		return 1
	fi
	mark=$(LogLineCount "$logFile")
	xdotool key ctrl+alt+r
	ExpectLogLine "$logFile" "$mark" "grab relative requested=off result=1" || return 1
	sleep "$settleSeconds"
	ReadPointer
	if [ "$pointerX" -ne "$((areaLeft + startInsideX))" ] || [ "$pointerY" -ne "$((areaTop + startInsideY))" ]; then
		Fail "the pointer is at $pointerX,$pointerY after the relative mode, expected $((areaLeft + startInsideX)),$((areaTop + startInsideY))"
		return 1
	fi
	visibility=$(ReadCursorVisibility)
	if [ "$visibility" != "visible" ]; then
		Fail "the cursor is $visibility after the relative mode"
		return 1
	fi
	StopDemo "$processId"
	return 0
}

# A warp in the relative mode moves the position the events carry and where the cursor appears afterwards, not the hidden cursor
Test_relative_warp_moves_start() {
	StartDemo A "$firstWindowX" "$firstWindowY" --relative-mouse || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark warpLine centerX centerY
	ActivateDemo "$window" "$logFile" || return 1
	ReadClientArea "$window"
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	xdotool key ctrl+alt+w
	warpLine=$(WaitForLogLine "$logFile" "$mark" "warp x=[0-9]+ y=[0-9]+ result=1")
	if [ -z "$warpLine" ]; then
		Fail "no warp in the log"
		return 1
	fi
	centerX=$(sed -E 's/.* warp x=([0-9]+) .*/\1/' <<< "$warpLine")
	centerY=$(sed -E 's/.* y=([0-9]+) .*/\1/' <<< "$warpLine")
	mark=$(LogLineCount "$logFile")
	SendRelativeMoves
	ExpectMoveSum "$logFile" "$mark" "$relativeSumX" "$relativeSumY" || return 1
	if [ "$movedPositions" != "$centerX,$centerY" ]; then
		Fail "the move events carry $movedPositions after the warp, expected $centerX,$centerY"
		return 1
	fi
	mark=$(LogLineCount "$logFile")
	xdotool key ctrl+alt+r
	ExpectLogLine "$logFile" "$mark" "grab relative requested=off result=1" || return 1
	sleep "$settleSeconds"
	ReadPointer
	if [ "$pointerX" -ne "$((areaLeft + centerX))" ] || [ "$pointerY" -ne "$((areaTop + centerY))" ]; then
		Fail "the pointer is at $pointerX,$pointerY after the relative mode, expected the warp target $((areaLeft + centerX)),$((areaTop + centerY))"
		return 1
	fi
	StopDemo "$processId"
	return 0
}

# Another window gets the focus: no more raw movement, and the pointer is free
Test_relative_released_on_focus_loss() {
	StartDemo A "$firstWindowX" "$firstWindowY" --relative-mouse || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowA" "$logA" || return 1
	ReadClientArea "$windowA"
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logA")
	SendRelativeMoves
	ExpectMoveSum "$logA" "$mark" "$relativeSumX" "$relativeSumY" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowB" "$logB" || return 1
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	ExpectPointerFree "$((areaLeft - outsideDistance))" "$((areaTop - outsideDistance))" || return 1
	mark=$(LogLineCount "$logA")
	SendRelativeMoves
	sleep "$settleSeconds"
	if grep -q " mouse move " <(tail -n +"$((mark + 1))" "$logA"); then
		Fail "demo A still got move events without the focus"
		return 1
	fi
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# The same with the build without XInput2, which warps the cursor back to the window center after every motion
Test_relative_fallback_delta_sum() {
	local demoBinary="$fallbackDemo"
	Test_relative_delta_sum
}

Test_relative_fallback_hides_and_keeps_pointer() {
	local demoBinary="$fallbackDemo"
	Test_relative_hides_and_keeps_pointer
}

Test_relative_fallback_returns_to_start() {
	local demoBinary="$fallbackDemo"
	Test_relative_returns_to_start
}

# Fails when the active window is not the given one. A window that shows up in front instead (KWin switcher or launcher) is closed with Escape.
ExpectActiveWindow() {
	local window="$1"
	local what="$2"
	local activeWindow
	activeWindow=$(xdotool getactivewindow 2> /dev/null)
	if [ "$activeWindow" != "$window" ]; then
		xdotool key Escape
		Fail "$what, the active window is $activeWindow instead of $window"
		return 1
	fi
	return 0
}

# With the keyboard grab Alt+Tab goes to the window, the window manager does not switch. The grab comes back with the focus, demo A lost it to B at the start.
Test_keyboard_grab_keeps_alt_tab() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowB" "$logB" || return 1
	ActivateDemo "$windowA" "$logA" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logA")
	xdotool key alt+Tab
	sleep "$settleSeconds"
	ExpectActiveWindow "$windowA" "Alt+Tab switched the window" || return 1
	ExpectLogLine "$logA" "$mark" "key button state=press .* key=Tab " || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# With the keyboard grab Alt+F4 goes to the window, the window manager does not close it
Test_keyboard_grab_keeps_alt_f4() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	xdotool key alt+F4
	sleep "$settleSeconds"
	if ! kill -0 "$processId" 2> /dev/null || grep -q "window closed" <(tail -n +"$((mark + 1))" "$logFile"); then
		Fail "Alt+F4 closed the demo"
		return 1
	fi
	ExpectLogLine "$logFile" "$mark" "key button state=press .* key=F4 " || return 1
	StopDemo "$processId"
	return 0
}

# The text input keeps working while the keyboard is grabbed
Test_keyboard_grab_text_input() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	xdotool key a
	ExpectLogLine "$logFile" "$mark" "key input code=97$" || return 1
	StopDemo "$processId"
	return 0
}

# Another window gets the focus: the grab is released, so Alt+Tab from there switches back to demo A
Test_keyboard_grab_released_on_focus_loss() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowA" "$logA" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowB" "$logB" || return 1
	ExpectLogLine "$logA" "$mark" "window lostfocus" || return 1
	sleep "$settleSeconds"
	xdotool key alt+Tab
	sleep "$settleSeconds"
	ExpectActiveWindow "$windowA" "Alt+Tab from demo B did not switch back to demo A" || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# Another client holds the keyboard when the window gets the focus: XGrabKeyboard fails with AlreadyGrabbed, FPL tries again and gets the grab once the other client lets go
Test_keyboard_grab_retries_when_already_grabbed() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab --log-fpl || return 1
	local windowA="$demoWindow" logA="$demoLog" processA="$demoPid"
	StartDemo B "$secondWindowX" "$secondWindowY" || return 1
	local windowB="$demoWindow" logB="$demoLog" processB="$demoPid" mark
	ActivateDemo "$windowB" "$logB" || return 1
	StartInputHolder keyboard "$inputHoldSeconds"
	ExpectLogLine "$holderLog" 0 "^grab 0$" || return 1
	mark=$(LogLineCount "$logA")
	ActivateDemo "$windowA" "$logA" || return 1
	ExpectLogLine "$logA" "$mark" "XGrabKeyboard failed with AlreadyGrabbed" || return 1
	ExpectLogLine "$holderLog" 0 "^released$" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logA")
	xdotool key alt+Tab
	sleep "$settleSeconds"
	ExpectActiveWindow "$windowA" "Alt+Tab switched the window after the other client let go" || return 1
	ExpectLogLine "$logA" "$mark" "key button state=press .* key=Tab " || return 1
	StopDemo "$processA"
	StopDemo "$processB"
	return 0
}

# With the keyboard grab Super goes to the window, the launcher of the desktop does not open
Test_keyboard_grab_keeps_super() {
	StartDemo A "$firstWindowX" "$firstWindowY" --keyboard-grab || return 1
	local window="$demoWindow" logFile="$demoLog" processId="$demoPid" mark
	ActivateDemo "$window" "$logFile" || return 1
	sleep "$settleSeconds"
	mark=$(LogLineCount "$logFile")
	xdotool key super
	sleep "$settleSeconds"
	ExpectActiveWindow "$window" "Super opened something" || return 1
	ExpectLogLine "$logFile" "$mark" "key button state=press .* key=LeftSuper " || return 1
	StopDemo "$processId"
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
