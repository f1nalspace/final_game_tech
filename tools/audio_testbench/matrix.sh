#!/bin/bash
# Prints the render matrix of the test bench, one case per line: <case id><TAB><player arguments>.
# Usage: matrix.sh full|short
# full: every track in every format with pitch 0.5, 1 and 2 (passthrough, SinC and both even-ratio paths), plus a few single cases for chunk size, volume and repeat.
# Every case of full runs once loaded fully and once as a stream (--stream), both must give the same hash. A track that cannot stream yet loads fully then, its stream case is the same render.
# A single case may seek (--seek=<at>:<to>), its id ends on _s<at>-<to>. The cases without a seek keep the ids they had.
# Kept small on purpose (130 cases), test runs must stay light. More values go into the arrays below later.

media_dir="media"

print_case() {
	local track=$1 rate=$2 sample_format=$3 channels=$4 chunk=$5 volume=$6 pitch=$7 repeat=$8 load_mode=$9 seek=${10:--}
	local seek_suffix=""
	if [ "$seek" != "-" ]; then
		seek_suffix="_s${seek/:/-}"
	fi
	local case_id="${track}_${rate}-${sample_format}-${channels}_c${chunk}_v${volume}_p${pitch}_r${repeat}${seek_suffix}_${load_mode}"
	local arguments="--rate=$rate --format=$sample_format --channels=$channels --chunk=$chunk --volume=$volume --pitch=$pitch"
	if [ "$repeat" == "1" ]; then
		arguments="$arguments --repeat"
	fi
	if [ "$seek" != "-" ]; then
		arguments="$arguments --seek=$seek"
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
	# track rate format channels chunk volume pitch repeat [seek]
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
	read -r track rate sample_format channels chunk volume pitch repeat seek <<< "$single_case"
	for load_mode in "${load_modes[@]}"; do
		print_case "$track" "$rate" "$sample_format" "$channels" "$chunk" "$volume" "$pitch" "$repeat" "$load_mode" "${seek:--}"
	done
done
