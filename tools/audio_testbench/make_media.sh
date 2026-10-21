#!/bin/bash
# Creates the test media of the test bench in media/ from files of the repository, the long track converted to Vorbis twice and two damaged copies of the crackout music.
# Usage: make_media.sh   (needs ffmpeg with libvorbis)

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
damaged_source="$media_dir/crackout_44k.ogg"
damaged_source_size=$(stat -c %s "$damaged_source")
truncated_size=$((damaged_source_size / 2))
head -c $truncated_size "$damaged_source" > "$media_dir/crackout_44k_truncated.ogg" || exit 1
overwritten_file="$media_dir/crackout_44k_overwritten.ogg"
cp "$damaged_source" "$overwritten_file" || exit 1
overwrite_length=64
overwrite_parts=4
for overwrite_index in 1 2 3; do
	overwrite_offset=$((damaged_source_size * overwrite_index / overwrite_parts))
	printf '\xa5%.0s' $(seq $overwrite_length) | dd of="$overwritten_file" bs=1 seek=$overwrite_offset conv=notrunc status=none || exit 1
done
ls -l "$media_dir"
