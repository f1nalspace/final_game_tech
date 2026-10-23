#!/bin/bash
# Builds FPL_Test in several variants and runs each one, native ones on the private X server, Windows ones under wine.
# Usage: fpltest.sh <label> [variant ...]
# Variants: gcc-debug clang-o2 asan tsan mingw64-o2 mingw32-o2 mingw64-o0 (default: all)
# Results: demos/immediates/fpl_test_runs/<label>/fpltest/results.tsv plus one build and one run log per variant.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

label=${1:?usage: fpltest.sh <label> [variant ...]}
shift
all_variants=(gcc-debug clang-o2 asan tsan mingw64-o2 mingw32-o2 mingw64-o0)
if [ $# -gt 0 ]; then
	variants=("$@")
else
	variants=("${all_variants[@]}")
fi

# A full run takes about a minute natively, sanitizers and wine make it several times slower
run_timeout_seconds=900
test_dir="$FPL_REPO_ROOT/demos/FPL_Test"
test_defines=(-DFPL_IMPLEMENTATION -DFPL_LOGGING -DFT_IMPLEMENTATION)
test_includes=(-I "$FPL_REPO_ROOT" -I "$FPL_REPO_ROOT/demos/additions" -I "$FPL_REPO_ROOT/demos/dependencies")
linux_libs=(-lm -ldl -lpthread)

run_dir=$(fpl_test_prepare_run "$label")
results_dir="$run_dir/fpltest"
mkdir -p "$results_dir"
results_file="$results_dir/results.tsv"
echo -e "variant\tbuild\texit\tfailed_asserts\tsanitizer_or_crash_reports\tseconds" > "$results_file"

build_variant() {
	local variant=$1
	local output=$2
	case "$variant" in
		gcc-debug) gcc -std=c99 -g -O0 "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		clang-o2) clang -std=c99 -g -O2 "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		asan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		tsan) gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=thread "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" "${linux_libs[@]}" ;;
		mingw64-o2) x86_64-w64-mingw32-gcc -std=c99 -g -O2 "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" ;;
		mingw32-o2) i686-w64-mingw32-gcc -std=c99 -g -O2 "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" ;;
		mingw64-o0) x86_64-w64-mingw32-gcc -std=c99 -g -O0 "${test_defines[@]}" "$test_dir/fpl_test.c" "${test_includes[@]}" -o "$output" ;;
		*) echo "unknown variant $variant" >&2; return 1 ;;
	esac
}

fpl_test_start_xvfb "$run_dir" || exit 1
fpl_test_prepare_wine

for variant in "${variants[@]}"; do
	variant_dir="$results_dir/$variant"
	rm -rf "$variant_dir"
	mkdir -p "$variant_dir"
	is_windows=0
	executable="$variant_dir/FPL_Test"
	if [[ "$variant" == mingw* ]]; then
		is_windows=1
		executable="$variant_dir/FPL_Test.exe"
	fi
	if ! build_variant "$variant" "$executable" > "$variant_dir/build.log" 2>&1; then
		echo -e "$variant\tFAILED\t-\t-\t-\t-" >> "$results_file"
		echo "$variant: build FAILED (see $variant_dir/build.log)"
		continue
	fi
	start_seconds=$SECONDS
	# Each variant runs in its own directory, the security tests write fixtures next to the executable
	if [ $is_windows -eq 1 ]; then
		(cd "$variant_dir" && timeout "$run_timeout_seconds" bash -c "source '$FPL_TEST_TOOLS_DIR/common.sh' && fpl_test_wine ./FPL_Test.exe" > run.log 2>&1)
	else
		(cd "$variant_dir" && DISPLAY=$FPL_TEST_DISPLAY ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 TSAN_OPTIONS=halt_on_error=0 timeout "$run_timeout_seconds" ./FPL_Test > run.log 2>&1)
	fi
	exit_code=$?
	elapsed_seconds=$((SECONDS - start_seconds))
	failed_asserts=$(grep -c -E "ASSERTION FAILED|Assertion failed|Assertion .* failed" "$variant_dir/run.log")
	sanitizer_reports=$(grep -c -E "ERROR: AddressSanitizer|runtime error:|WARNING: ThreadSanitizer|ERROR: ThreadSanitizer|ERROR: LeakSanitizer|Unhandled page fault|Unhandled exception" "$variant_dir/run.log")
	echo -e "$variant\tok\t$exit_code\t$failed_asserts\t$sanitizer_reports\t$elapsed_seconds" >> "$results_file"
	echo "$variant: exit $exit_code, failed asserts $failed_asserts, sanitizer reports $sanitizer_reports, ${elapsed_seconds}s"
done

fpl_test_stop_xvfb "$run_dir"
