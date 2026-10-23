#!/bin/bash
# Renders the matrix with FPL_AudioPlayer --render and compares every hash with the baseline of the build variant.
# Usage: regress.sh <variant> [full|short] [--update-baseline] [--jobs <n>]   (default 4 jobs, test runs must stay light)
# Results: results/<variant>_<matrix>.txt (<case id> <hash> <frames> <mix_mean_us> <mix_p99_us> <mix_max_us> <vm_hwm_kb>)
# Baseline: baseline/<variant>_<matrix>.txt, written with --update-baseline. Only a case whose hash differs is rendered again with --render-wav into renders/<variant>/.
# Expected changes are explained in expected_changes.txt, by hand.

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$testbench_dir/../.." && pwd)"

export LC_ALL=C
export LANG=C

variant=${1:?usage: regress.sh <variant> [full|short] [--update-baseline] [--jobs <n>]}
shift
matrix_kind="short"
update_baseline=0
job_count=4
while [ $# -gt 0 ]; do
	case "$1" in
		full|short) matrix_kind=$1 ;;
		--update-baseline) update_baseline=1 ;;
		--jobs) job_count=$2; shift ;;
		*) echo "unknown argument $1" >&2; exit 2 ;;
	esac
	shift
done

executable="$testbench_dir/bin/$variant/FPL_AudioPlayer"
is_windows=0
if [[ "$variant" == mingw* ]]; then
	executable="$executable.exe"
	is_windows=1
fi
if [ ! -x "$executable" ] && [ ! -f "$executable" ]; then
	echo "missing $executable, run build.sh $variant first" >&2
	exit 1
fi

if [ $is_windows -eq 1 ]; then
	source "$repo_root/tools/fpl_test/common.sh"
	fpl_test_prepare_wine
fi

results_dir="$testbench_dir/results"
baseline_dir="$testbench_dir/baseline"
renders_dir="$testbench_dir/renders/$variant"
case_dir="$testbench_dir/results/cases_${variant}_${matrix_kind}"
mkdir -p "$results_dir" "$baseline_dir" "$renders_dir"
rm -rf "$case_dir"
mkdir -p "$case_dir"
results_file="$results_dir/${variant}_${matrix_kind}.txt"
baseline_file="$baseline_dir/${variant}_${matrix_kind}.txt"

# Renders one case and writes its result line, run in parallel by xargs
render_case() {
	local case_id=$1
	local arguments=$2
	local render_option=${3:---render}
	local log_file="$case_dir/$case_id.log"
	cd "$testbench_dir" || return 1
	if [ "$is_windows" -eq 1 ]; then
		fpl_test_wine "$executable" "$render_option" $arguments > "$log_file" 2>&1
	else
		"$executable" "$render_option" $arguments > "$log_file" 2>&1
	fi
	local render_line
	render_line=$(tr -d '\r' < "$log_file" | grep "^render: " | tail -n 1)
	if [ -z "$render_line" ]; then
		echo "$case_id FAILED - - - - -" > "$case_dir/$case_id.result"
		return 0
	fi
	# Fields are read by name, so later plans can add fields anywhere in the line
	local field_values
	field_values=$(echo "$render_line" | awk '{
		for (i = 2; i <= NF; ++i) { split($i, pair, "="); value[pair[1]] = pair[2] }
		printf "%s %s %s %s %s %s", value["hash"], value["frames"], value["mix_mean_us"], value["mix_p99_us"], value["mix_max_us"], value["vm_hwm_kb"]
	}')
	echo "$case_id $field_values" > "$case_dir/$case_id.result"
}
export -f render_case
export testbench_dir executable is_windows variant case_dir
if [ $is_windows -eq 1 ]; then
	export -f fpl_test_wine
	export FPL_TEST_DISPLAY FPL_WINE_PREFIX
fi

start_seconds=$SECONDS
"$testbench_dir/matrix.sh" "$matrix_kind" | while IFS=$'\t' read -r case_id arguments; do
	printf '%s\0%s\0' "$case_id" "$arguments"
done | xargs -0 -n 2 -P "$job_count" bash -c 'render_case "$0" "$1"'
elapsed_seconds=$((SECONDS - start_seconds))

cat "$case_dir"/*.result 2> /dev/null | sort > "$results_file"
case_count=$(wc -l < "$results_file")
failed_count=$(grep -c " FAILED " "$results_file")

if [ $update_baseline -eq 1 ]; then
	cp "$results_file" "$baseline_file"
	echo "$variant $matrix_kind: $case_count cases, $failed_count failed, baseline written, ${elapsed_seconds}s"
	exit 0
fi

if [ ! -f "$baseline_file" ]; then
	echo "$variant $matrix_kind: $case_count cases, $failed_count failed, no baseline yet (run with --update-baseline), ${elapsed_seconds}s"
	exit 1
fi

# Compares the hash column only, costs and memory may differ from run to run
same_count=0
different_count=0
missing_count=0
while read -r case_id hash rest; do
	baseline_hash=$(awk -v id="$case_id" '$1 == id { print $2 }' "$baseline_file")
	if [ -z "$baseline_hash" ]; then
		missing_count=$((missing_count + 1))
		echo "new case: $case_id"
	elif [ "$baseline_hash" == "$hash" ]; then
		same_count=$((same_count + 1))
	else
		different_count=$((different_count + 1))
		# Only a differing case is written as a wave file, to listen to it or compare it sample by sample
		case_arguments=$("$testbench_dir/matrix.sh" "$matrix_kind" | awk -F '\t' -v id="$case_id" '$1 == id { print $2 }')
		render_case "$case_id" "$case_arguments" "--render-wav=renders/$variant/$case_id.wav" > /dev/null
		echo "different: $case_id ($baseline_hash -> $hash), written to renders/$variant/$case_id.wav"
	fi
done < "$results_file"
echo "$variant $matrix_kind: $case_count cases, $same_count same, $different_count different, $missing_count new, $failed_count failed, ${elapsed_seconds}s"
if [ $different_count -gt 0 ] || [ $failed_count -gt 0 ]; then
	exit 1
fi
