#!/bin/bash
# Creates the test media of the test bench in media/ from files of the repository, the long track converted to Vorbis twice.
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
ls -l "$media_dir"
