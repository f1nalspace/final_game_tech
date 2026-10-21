#!/bin/bash
# Measures the cost of AudioSystemWriteFrames with FPL_AudioPlayer --render on one core: the empty mixer, one item at 48 kHz (passthrough), one item at 44.1 kHz (SinC).
# Usage: measure_mixer.sh <variant> [runs]   (default 3 runs per case, run build.sh <variant> first)
# Prints every run and per case the median of mix_mean_us over the runs, 48 kHz stereo S16 output with 512 frames per call (the player defaults).
# The empty mixer is the short bounce sound followed by silence up to 10 seconds, its few SinC calls add about half a microsecond to the mean.

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$testbench_dir/../.." && pwd)"

export LC_ALL=C
export LANG=C

variant=${1:?usage: measure_mixer.sh <variant> [runs]}
run_count=${2:-3}
measure_core=0

executable="$testbench_dir/bin/$variant/FPL_AudioPlayer"
is_windows=0
if [[ "$variant" == mingw* ]]; then
	executable="$executable.exe"
	is_windows=1
	source "$repo_root/tools/fpl_test/common.sh"
	fpl_test_prepare_wine
fi
if [ ! -f "$executable" ]; then
	echo "missing $executable, run build.sh $variant first" >&2
	exit 1
fi

# <case name>|<player arguments>
cases=(
	"empty|--seconds=10 media/bounce_44100hz.wav"
	"48k|media/long_48k.ogg"
	"44k|media/crackout_44k.ogg"
)

cd "$testbench_dir" || exit 1
for entry in "${cases[@]}"; do
	case_name=${entry%%|*}
	arguments=${entry#*|}
	means=()
	for ((run_index = 0; run_index < run_count; ++run_index)); do
		if [ $is_windows -eq 1 ]; then
			render_line=$(DISPLAY=$FPL_TEST_DISPLAY WINEPREFIX=$FPL_WINE_PREFIX WINEDEBUG=-all taskset -c $measure_core wine "$executable" --render $arguments 2> /dev/null | tr -d '\r' | grep '^render: ')
		else
			render_line=$(taskset -c $measure_core "$executable" --render $arguments 2> /dev/null | grep '^render: ')
		fi
		mean=$(echo "$render_line" | sed -n 's/.* mix_mean_us=\([0-9.]*\).*/\1/p')
		percentile=$(echo "$render_line" | sed -n 's/.* mix_p99_us=\([0-9.]*\).*/\1/p')
		hash=$(echo "$render_line" | sed -n 's/.* hash=\([0-9a-f]*\).*/\1/p')
		echo "$variant $case_name run $run_index: mix_mean_us=$mean mix_p99_us=$percentile hash=$hash"
		means+=("$mean")
	done
	median_index=$(((run_count + 1) / 2))
	median=$(printf '%s\n' "${means[@]}" | sort -g | sed -n "${median_index}p")
	echo "$variant $case_name: median mix_mean_us=$median"
done
