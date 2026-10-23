#!/bin/bash
# Shared paths and helpers of the FPL test bench, sourced by the other scripts.
# Every result goes below demos/immediates/fpl_test_runs/<label>/, which git ignores.

FPL_TEST_TOOLS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FPL_REPO_ROOT="$(cd "$FPL_TEST_TOOLS_DIR/../.." && pwd)"
FPL_RUNS_DIR="$FPL_REPO_ROOT/demos/immediates/fpl_test_runs"
FPL_HEADER="$FPL_REPO_ROOT/final_platform_layer.h"

# Display of the private X server, so no test window ever shows up on the desktop or steals keystrokes
FPL_TEST_DISPLAY=":77"
FPL_TEST_SCREEN="1600x1000x24"

# English compiler messages, so logs of different machines and runs compare as text
export LC_ALL=C
export LANG=C

# Creates the run directory of a label and records which state of the repository it tested
fpl_test_prepare_run() {
	local label=$1
	local run_dir="$FPL_RUNS_DIR/$label"
	mkdir -p "$run_dir"
	{
		echo "label: $label"
		echo "date: $(date -Iseconds)"
		echo "commit: $(git -C "$FPL_REPO_ROOT" rev-parse HEAD)"
		echo "branch: $(git -C "$FPL_REPO_ROOT" rev-parse --abbrev-ref HEAD)"
		echo "fpl changes against HEAD:"
		git -C "$FPL_REPO_ROOT" diff --stat HEAD -- final_platform_layer.h demos/FPL_Test
	} > "$run_dir/meta.txt"
	# The header is kept with the results, the warning comparison maps line numbers to source text through it
	cp "$FPL_HEADER" "$run_dir/final_platform_layer.h"
	echo "$run_dir"
}

# Starts the private X server unless it already runs, remembers its pid in the run directory
fpl_test_start_xvfb() {
	local run_dir=$1
	if xdpyinfo -display "$FPL_TEST_DISPLAY" > /dev/null 2>&1; then
		return 0
	fi
	Xvfb "$FPL_TEST_DISPLAY" -screen 0 "$FPL_TEST_SCREEN" +extension GLX > /dev/null 2>&1 &
	echo $! > "$run_dir/xvfb.pid"
	local wait_steps=50
	for ((step = 0; step < wait_steps; ++step)); do
		if xdpyinfo -display "$FPL_TEST_DISPLAY" > /dev/null 2>&1; then
			return 0
		fi
		sleep 0.1
	done
	echo "Xvfb did not come up" >&2
	return 1
}

# Stops the private X server, but only the one this run started
fpl_test_stop_xvfb() {
	local run_dir=$1
	if [ -f "$run_dir/xvfb.pid" ]; then
		kill "$(cat "$run_dir/xvfb.pid")" 2> /dev/null
		rm -f "$run_dir/xvfb.pid"
	fi
}

# Own wine prefix of the test bench: the crash dialog is off, so a crash writes its backtrace into the log instead of opening a window
FPL_WINE_PREFIX="$FPL_RUNS_DIR/wineprefix"

fpl_test_prepare_wine() {
	if [ -f "$FPL_WINE_PREFIX/.fpl_test_ready" ]; then
		return 0
	fi
	mkdir -p "$FPL_WINE_PREFIX"
	DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all wineboot -i > /dev/null 2>&1
	DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all wine reg add "HKCU\\Software\\Wine\\WineDbg" /v ShowCrashDialog /t REG_DWORD /d 0 /f > /dev/null 2>&1
	DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all wineserver -w
	touch "$FPL_WINE_PREFIX/.fpl_test_ready"
}

# Runs a Windows program on the private X server inside the own prefix, never on the desktop
fpl_test_wine() {
	DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all wine "$@"
}

# Lists the CMake projects below demos/ and apps/ that include final_platform_layer.h, one relative path per line
fpl_test_list_projects() {
	local project_dir
	for project_dir in "$FPL_REPO_ROOT"/demos/*/ "$FPL_REPO_ROOT"/apps/*/; do
		project_dir=${project_dir%/}
		if [ ! -f "$project_dir/CMakeLists.txt" ]; then
			continue
		fi
		if grep -rqs --include="*.c" --include="*.cpp" --include="*.h" --exclude-dir=build --exclude-dir=immediates --exclude-dir="cmake-build-*" --exclude-dir="linux-x64-*" -l "final_platform_layer.h" "$project_dir"; then
			echo "${project_dir#$FPL_REPO_ROOT/}"
		fi
	done
}
