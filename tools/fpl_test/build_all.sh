#!/bin/bash
# Builds every CMake project that uses FPL with -Wall -Wextra and records per project the result and the warnings.
# Usage: build_all.sh <label> [parallel projects]
# Compare two runs with compare_builds.py <baseline label> <label>.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

label=${1:?usage: build_all.sh <label> [parallel projects]}
parallel_projects=${2:-8}
jobs_per_project=4

run_dir=$(fpl_test_prepare_run "$label")
build_root="$run_dir/cmake"
log_root="$run_dir/build_logs"
rm -rf "$build_root" "$log_root"
mkdir -p "$build_root" "$log_root"
fpl_test_list_projects > "$run_dir/projects.txt"

build_one() {
	local project=$1
	local name
	name=$(basename "$project")
	local log="$log_root/$name.log"
	{
		cmake -S "$FPL_REPO_ROOT/$project" -B "$build_root/$name" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-Wall -Wextra" -DCMAKE_CXX_FLAGS="-Wall -Wextra" 2>&1
		echo "### CONFIGURE EXIT $?"
		cmake --build "$build_root/$name" -j "$jobs_per_project" 2>&1
		echo "### BUILD EXIT $?"
	} > "$log" 2>&1
}
export -f build_one
export build_root log_root jobs_per_project FPL_REPO_ROOT

xargs -P "$parallel_projects" -I{} bash -c 'build_one "$@"' _ {} < "$run_dir/projects.txt"

python3 "$FPL_TEST_TOOLS_DIR/compare_builds.py" --summary "$label"
