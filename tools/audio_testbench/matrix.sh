#!/bin/bash
# Prints the render matrix of the test bench, one case per line: <case id><TAB><player arguments>.
# Usage: matrix.sh full|short
# full: every track in every format with pitch 0.5, 1 and 2 (passthrough, SinC and both even-ratio paths), plus a few single cases for chunk size, volume and repeat.
# Every case of full runs once loaded fully and once as a stream (--stream), both must give the same hash. A track that cannot stream yet loads fully then, its stream case is the same render.
# A single case may seek (--seek=<at>:<to>), its id ends on _s<at>-<to>. The cases without a seek keep the ids they had.
# A single case may have a loop region (--loop=<start>:<end>, seek - for none), its id ends on _l<start>-<end>.
# Kept small on purpose (168 cases), test runs must stay light. More values go into the arrays below later.

media_dir="media"

print_case() {
	local track=$1 rate=$2 sample_format=$3 channels=$4 chunk=$5 volume=$6 pitch=$7 repeat=$8 load_mode=$9 seek=${10:--} loop=${11:--}
	local seek_suffix=""
	if [ "$seek" != "-" ]; then
		seek_suffix="_s${seek/:/-}"
	fi
	local loop_suffix=""
	if [ "$loop" != "-" ]; then
		loop_suffix="_l${loop/:/-}"
	fi
	local case_id="${track}_${rate}-${sample_format}-${channels}_c${chunk}_v${volume}_p${pitch}_r${repeat}${seek_suffix}${loop_suffix}_${load_mode}"
	local arguments="--rate=$rate --format=$sample_format --channels=$channels --chunk=$chunk --volume=$volume --pitch=$pitch"
	if [ "$repeat" == "1" ]; then
		arguments="$arguments --repeat"
	fi
	if [ "$seek" != "-" ]; then
		arguments="$arguments --seek=$seek"
	fi
	if [ "$loop" != "-" ]; then
		arguments="$arguments --loop=$loop"
	fi
	if [ "$load_mode" == "stream" ]; then
		arguments="$arguments --stream"
	fi
	if [ "$track" == "sine" ]; then
		arguments="$arguments --sine"
	else
		arguments="$arguments $media_dir/$track"
	fi
	printf '%s\t%s\n' "$case_id" "$arguments"
}

set_full_matrix() {
	tracks=(sine crackout_44k.ogg long_48k.ogg long_44k.ogg bounce_44100hz.wav long_44k.mp3)
	formats=("48000 s16 2" "48000 f32 2" "44100 s16 2")
	chunks=(512)
	volumes=(1.0)
	pitches=(1.0 0.5 2.0)
	repeats=(0)
	load_modes=(full stream)
	# track rate format channels chunk volume pitch repeat [seek|-] [loop]
	single_cases=(
		"crackout_44k.ogg 48000 s16 2 256 1.0 1.0 0"
		"crackout_44k.ogg 48000 s16 2 2048 1.0 1.0 0"
		"crackout_44k.ogg 48000 s16 2 512 0.5 1.0 0"
		"crackout_44k.ogg 48000 s16 2 512 1.0 1.0 1"
		"sine 48000 s16 2 512 1.0 0.5 1"
		"long_48k.ogg 48000 s16 2 512 1.0 2.0 1"
		"sine 48000 s16 2 512 1.0 1.0 0 1:5"
		"crackout_44k.ogg 48000 s16 2 512 1.0 1.0 0 2:60"
		"crackout_44k.ogg 48000 f32 2 512 1.0 2.0 0 3:0.5"
		"long_48k.ogg 48000 s16 2 512 1.0 2.0 1 5:275"
		"long_44k.mp3 48000 s16 2 512 1.0 1.0 0 2:270"
		# Loop regions: passthrough, SinC, frame skipping and duplication at odd frames, a region shorter than a chunk, seeks into the outro and past the end, no repeat
		"sine 48000 s16 2 512 1.0 1.0 1 - 1.2345:4.321"
		"sine 48000 s16 2 441 1.0 0.5 1 - 1.2345:4.321"
		"sine 48000 s16 2 512 1.0 1.0 1 - 2.5:2.505"
		"crackout_44k.ogg 48000 s16 2 512 1.0 1.0 1 - 10:65"
		"crackout_44k.ogg 48000 s16 2 512 1.0 1.0 0 - 10:65"
		"crackout_44k.ogg 44100 f32 2 512 1.0 1.0 1 - 10.00002:65.00005"
		"crackout_44k.ogg 44100 s16 2 512 1.0 2.0 1 - 10.00002:65.00005"
		"crackout_44k.ogg 44100 s16 2 512 1.0 0.5 1 - 10.00002:65.00005"
		"crackout_44k.ogg 48000 s16 2 512 1.0 1.0 1 3:90 10:65"
		"long_48k.ogg 48000 s16 2 512 1.0 1.0 1 5:275 20:250"
		"long_44k.mp3 48000 s16 2 512 1.0 1.0 1 - 30:200"
		"long_44k.mp3 44100 s16 2 512 1.0 2.0 1 - 30.00003:200.00007"
		"bounce_44100hz.wav 44100 s16 2 512 1.0 1.0 1 - 0.02:0.09"
		# Loop regions from the tags of the file: Vorbis LOOPSTART with LOOPLENGTH and with LOOPEND, wave smpl after and before the data, the tags without repeat, --loop over the tags
		"crackout_44k_loop.ogg 48000 s16 2 512 1.0 1.0 1"
		"crackout_44k_loopend.ogg 44100 s16 2 512 1.0 2.0 1"
		"crackout_44k_loop.wav 48000 s16 2 512 1.0 1.0 1"
		"crackout_44k_loop_smpl_first.wav 44100 s16 2 512 1.0 0.5 1"
		"crackout_44k_loop.ogg 44100 s16 2 512 1.0 1.0 0"
		"crackout_44k_loop.ogg 44100 f32 2 512 1.0 1.0 1 - 10:65"
	)
}

set_short_matrix() {
	tracks=(sine crackout_44k.ogg bounce_44100hz.wav)
	formats=("48000 s16 2")
	chunks=(512)
	volumes=(1.0)
	pitches=(1.0 2.0)
	repeats=(0)
	load_modes=(full)
	single_cases=()
}

case "${1:-short}" in
	full) set_full_matrix ;;
	short) set_short_matrix ;;
	*) echo "usage: matrix.sh full|short" >&2; exit 2 ;;
esac

for track in "${tracks[@]}"; do
	for format in "${formats[@]}"; do
		read -r rate sample_format channels <<< "$format"
		for chunk in "${chunks[@]}"; do
			for volume in "${volumes[@]}"; do
				for pitch in "${pitches[@]}"; do
					for repeat in "${repeats[@]}"; do
						for load_mode in "${load_modes[@]}"; do
							print_case "$track" "$rate" "$sample_format" "$channels" "$chunk" "$volume" "$pitch" "$repeat" "$load_mode"
						done
					done
				done
			done
		done
	done
done

for single_case in "${single_cases[@]}"; do
	read -r track rate sample_format channels chunk volume pitch repeat seek loop <<< "$single_case"
	for load_mode in "${load_modes[@]}"; do
		print_case "$track" "$rate" "$sample_format" "$channels" "$chunk" "$volume" "$pitch" "$repeat" "$load_mode" "${seek:--}" "${loop:--}"
	done
done
