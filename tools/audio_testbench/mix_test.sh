#!/bin/bash
# Builds tools/mix_two_items_test.c for one variant, runs it and compares its output with the baseline of the variant.
# Then runs its --concurrent mode (two mixing threads at the same time) from a second build without assertions, which must report ok.
# Usage: mix_test.sh <variant> [--update-baseline]
# Variants: gcc-x64-o2 gcc-x86-o2 asan tsan mingw32-xp-o2 (tsan with tsan_suppressions.txt)
# Baseline: baseline/mix_two_items_test_<variant>.txt, written with --update-baseline. The hashes must stay the same before and after every change to the mixer.

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$testbench_dir/../.." && pwd)"
test_source="$testbench_dir/tools/mix_two_items_test.c"

export LC_ALL=C
export LANG=C

variant=${1:?usage: mix_test.sh <variant> [--update-baseline]}
update_baseline=0
if [ "$2" == "--update-baseline" ]; then
	update_baseline=1
fi

# Assertions on, the mixer checks its own invariants
defines=(-DFPL_FORCE_ASSERTIONS)
includes=(-I "$repo_root" -I "$repo_root/demos/additions" -I "$repo_root/demos/dependencies")
warnings=(-Wall -Wextra)
linux_libs=(-lm -ldl -lpthread)
xp_flags=(-march=prescott -mfpmath=sse -mcrtdll=msvcrt-os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D__USE_MINGW_ANSI_STDIO=1 -DSTB_VORBIS_NO_STDIO -static)

variant_dir="$testbench_dir/bin/$variant"
mkdir -p "$variant_dir"
executable_suffix=""
if [[ "$variant" == mingw* ]]; then
	executable_suffix=".exe"
fi
executable="$variant_dir/mix_two_items_test$executable_suffix"
concurrent_executable="$variant_dir/mix_concurrent_test$executable_suffix"
build_log="$variant_dir/mix_two_items_test_build.log"

# Builds the test with the given extra defines into the given file
build_test() {
	local output=$1
	shift
	case "$variant" in
		gcc-x64-o2) gcc -std=c99 -g -O2 "${warnings[@]}" "$@" "${includes[@]}" "$test_source" -o "$output" "${linux_libs[@]}" ;;
		gcc-x86-o2) gcc -m32 -std=c99 -g -O2 -march=prescott -mfpmath=sse "${warnings[@]}" "$@" "${includes[@]}" "$test_source" -o "$output" "${linux_libs[@]}" -latomic ;;
		asan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined "${warnings[@]}" "$@" "${includes[@]}" "$test_source" -o "$output" "${linux_libs[@]}" ;;
		tsan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=thread "${warnings[@]}" "$@" "${includes[@]}" "$test_source" -o "$output" "${linux_libs[@]}" ;;
		mingw32-xp-o2) i686-w64-mingw32-gcc -std=c99 -g -O2 "${xp_flags[@]}" "${warnings[@]}" "$@" "${includes[@]}" "$test_source" -o "$output" ;;
		*) echo "unknown variant $variant" >&2; return 2 ;;
	esac
}

build_test "$executable" "${defines[@]}" > "$build_log" 2>&1 && build_test "$concurrent_executable" -DFPL_NO_ASSERTIONS >> "$build_log" 2>&1
build_exit=$?
own_warning_count=$(grep -c "mix_two_items_test.c:[0-9]*:[0-9]*: warning:" "$build_log")
if [ $build_exit -ne 0 ]; then
	echo "$variant: build failed, see $build_log" >&2
	exit 1
fi
if [ "$variant" == "tsan" ]; then
	export TSAN_OPTIONS="suppressions=$testbench_dir/tsan_suppressions.txt"
fi

# Runs one of the two builds, its output goes to stdout
run_test() {
	if [[ "$variant" == mingw* ]]; then
		fpl_test_wine "$@" 2> /dev/null | tr -d '\r'
		return ${PIPESTATUS[0]}
	fi
	"$@"
}

results_file="$testbench_dir/results/mix_two_items_test_$variant.txt"
baseline_file="$testbench_dir/baseline/mix_two_items_test_$variant.txt"
mkdir -p "$testbench_dir/results" "$testbench_dir/baseline"
if [[ "$variant" == mingw* ]]; then
	source "$repo_root/tools/fpl_test/common.sh"
	fpl_test_prepare_wine
fi
run_test "$executable" > "$results_file"
test_exit=$?

# The number of rejected calls differs from run to run, only the verdict counts
concurrent_line=$(run_test "$concurrent_executable" --concurrent | grep "^concurrent: calls=")
concurrent_exit=$?
echo "$concurrent_line"
if [[ "$concurrent_line" != *" ok" ]]; then
	echo "$variant: concurrent mixers FAILED" >&2
	test_exit=1
fi

if [ $update_baseline -eq 1 ]; then
	cp "$results_file" "$baseline_file"
	echo "$variant: exit $test_exit, $own_warning_count own warnings, baseline written"
	cat "$results_file"
	exit $test_exit
fi
if [ ! -f "$baseline_file" ]; then
	echo "$variant: no baseline yet (run with --update-baseline)"
	exit 1
fi
if diff "$baseline_file" "$results_file" > /dev/null; then
	echo "$variant: exit $test_exit, $own_warning_count own warnings, same as the baseline"
else
	echo "$variant: exit $test_exit, $own_warning_count own warnings, DIFFERENT from the baseline:"
	diff "$baseline_file" "$results_file"
	exit 1
fi
exit $test_exit
