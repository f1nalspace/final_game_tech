#!/bin/bash
# Creates the test media of the test bench in media/ from files of the repository, the long track converted to Vorbis twice, damaged copies of the crackout music and the long mp3 and mp3 kinds.
# Usage: make_media.sh   (needs ffmpeg with libvorbis and lame)

testbench_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$testbench_dir/../.." && pwd)"
media_dir="$testbench_dir/media"

long_track="$repo_root/demos/resources/Tavs_Control_[Argofox Release].mp3"
vorbis_quality=5
output_sample_rate=48000
source_sample_rate=44100

mkdir -p "$media_dir"
cp "$repo_root/demos/FPL_Crackout/data/music_44100hz.ogg" "$media_dir/crackout_44k.ogg" || exit 1
cp "$repo_root/demos/FPL_Crackout/data/bounce_44100hz.wav" "$media_dir/bounce_44100hz.wav" || exit 1
cp "$long_track" "$media_dir/long_44k.mp3" || exit 1
# Once at the output sample rate (no resampling in the mixer) and once at the sample rate of the mp3
ffmpeg -loglevel error -y -i "$media_dir/long_44k.mp3" -c:a libvorbis -q:a $vorbis_quality -ar $output_sample_rate "$media_dir/long_48k.ogg" || exit 1
ffmpeg -loglevel error -y -i "$media_dir/long_44k.mp3" -c:a libvorbis -q:a $vorbis_quality -ar $source_sample_rate "$media_dir/long_44k.ogg" || exit 1

# Damaged files the player must survive: one cut off at half its size, one overwritten at a quarter, half and three quarters of its size
make_damaged_copies() {
	local damaged_source=$1
	local truncated_file=$2
	local overwritten_file=$3
	local damaged_source_size
	damaged_source_size=$(stat -c %s "$damaged_source")
	local truncated_size=$((damaged_source_size / 2))
	head -c $truncated_size "$damaged_source" > "$truncated_file" || return 1
	cp "$damaged_source" "$overwritten_file" || return 1
	local overwrite_length=64
	local overwrite_parts=4
	local overwrite_index
	for overwrite_index in 1 2 3; do
		local overwrite_offset=$((damaged_source_size * overwrite_index / overwrite_parts))
		printf '\xa5%.0s' $(seq $overwrite_length) | dd of="$overwritten_file" bs=1 seek=$overwrite_offset conv=notrunc status=none || return 1
	done
}
make_damaged_copies "$media_dir/crackout_44k.ogg" "$media_dir/crackout_44k_truncated.ogg" "$media_dir/crackout_44k_overwritten.ogg" || exit 1
make_damaged_copies "$media_dir/long_44k.mp3" "$media_dir/long_44k_truncated.mp3" "$media_dir/long_44k_overwritten.mp3" || exit 1

# The mp3 kinds the stream must handle (MPEG-2, mono, low and high bit rates, VBR), made from the first 40 seconds of the crackout music
mp3_kinds_seconds=40
mp3_kinds_source="$media_dir/crackout_40s.wav"
ffmpeg -loglevel error -y -i "$media_dir/crackout_44k.ogg" -t $mp3_kinds_seconds "$mp3_kinds_source" || exit 1
lame --quiet -b 8 -m m --resample 8 "$mp3_kinds_source" "$media_dir/mp3_low_8k_mono.mp3" || exit 1
lame --quiet -b 32 -m m --resample 22.05 "$mp3_kinds_source" "$media_dir/mp3_mpeg2_22k_mono.mp3" || exit 1
lame --quiet -V 2 "$mp3_kinds_source" "$media_dir/mp3_vbr_44k.mp3" || exit 1
lame --quiet -b 320 --resample 48 "$mp3_kinds_source" "$media_dir/mp3_cbr320_48k.mp3" || exit 1
rm -f "$mp3_kinds_source"

ls -l "$media_dir"
