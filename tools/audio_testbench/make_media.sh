#!/bin/bash
# Creates the test media of the test bench in media/ from files of the repository, the long track converted to Vorbis twice, damaged copies of the crackout music and the long mp3, mp3 kinds and loop tagged files.
# Usage: make_media.sh   (needs ffmpeg with libvorbis, vorbiscomment from vorbis-tools and lame)

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

# Loop region of the loop tagged files in frames of the crackout music: 10 seconds plus one frame of intro, then an odd loop length, so the loop end is even
loop_start_frame=441001
loop_length_frames=2425501
loop_end_frame=$((loop_start_frame + loop_length_frames))

# Ogg Vorbis comments, once as LOOPSTART with LOOPLENGTH and once as LOOPSTART with LOOPEND (exclusive), the audio packets stay untouched
vorbiscomment -a -t "LOOPSTART=$loop_start_frame" -t "LOOPLENGTH=$loop_length_frames" "$media_dir/crackout_44k.ogg" "$media_dir/crackout_44k_loop.ogg" || exit 1
vorbiscomment -a -t "LOOPSTART=$loop_start_frame" -t "LOOPEND=$loop_end_frame" "$media_dir/crackout_44k.ogg" "$media_dir/crackout_44k_loopend.ogg" || exit 1

# Writes the 4 little-endian bytes of a 32-bit value
write_uint32_le() {
	local value=$1
	local escaped_bytes=""
	local byte_index
	for byte_index in 0 1 2 3; do
		local byte_value=$(((value >> (byte_index * 8)) & 0xff))
		local byte_hex
		byte_hex=$(printf '%02x' $byte_value)
		escaped_bytes="$escaped_bytes\\x$byte_hex"
	done
	printf "$escaped_bytes"
}

# Writes a smpl chunk with one forward loop, its end field is the last frame inside the loop
write_smpl_chunk() {
	local sample_rate=$1
	local loop_start=$2
	local loop_end=$3
	local smpl_header_size=36
	local smpl_loop_size=24
	local nanoseconds_per_second=1000000000
	local midi_unity_note=60
	local loop_count=1
	local loop_type_forward=0
	local sample_period=$((nanoseconds_per_second / sample_rate))
	local last_loop_frame=$((loop_end - 1))
	printf 'smpl'
	write_uint32_le $((smpl_header_size + smpl_loop_size))
	# Manufacturer, product
	write_uint32_le 0
	write_uint32_le 0
	write_uint32_le $sample_period
	write_uint32_le $midi_unity_note
	# MIDI pitch fraction, SMPTE format, SMPTE offset
	write_uint32_le 0
	write_uint32_le 0
	write_uint32_le 0
	write_uint32_le $loop_count
	# Size of the sampler specific data
	write_uint32_le 0
	# Loop: cue point id, type, start, end, fraction, play count (0 = endless)
	write_uint32_le 0
	write_uint32_le $loop_type_forward
	write_uint32_le $loop_start
	write_uint32_le $last_loop_frame
	write_uint32_le 0
	write_uint32_le 0
}

# Sets the RIFF size of a wave file to its file size minus the RIFF header
fix_riff_size() {
	local wave_file=$1
	local riff_header_size=8
	local riff_size_offset=4
	local wave_file_size
	wave_file_size=$(stat -c %s "$wave_file")
	write_uint32_le $((wave_file_size - riff_header_size)) | dd of="$wave_file" bs=1 seek=$riff_size_offset conv=notrunc status=none
}

# The crackout music as a plain wave file and twice with a smpl chunk: after the data chunk (most editors) and between the fmt and the data chunk
plain_wave="$media_dir/crackout_44k.wav"
ffmpeg -loglevel error -y -i "$media_dir/crackout_44k.ogg" -map_metadata -1 -bitexact -c:a pcm_s16le "$plain_wave" || exit 1
canonical_data_chunk_offset=36
data_chunk_id=$(dd if="$plain_wave" bs=1 skip=$canonical_data_chunk_offset count=4 status=none)
if [ "$data_chunk_id" != "data" ]; then
	echo "$plain_wave has no data chunk at byte $canonical_data_chunk_offset" >&2
	exit 1
fi
smpl_after_data_wave="$media_dir/crackout_44k_loop.wav"
{ cat "$plain_wave"; write_smpl_chunk $source_sample_rate $loop_start_frame $loop_end_frame; } > "$smpl_after_data_wave" || exit 1
fix_riff_size "$smpl_after_data_wave" || exit 1
smpl_before_data_wave="$media_dir/crackout_44k_loop_smpl_first.wav"
{ head -c $canonical_data_chunk_offset "$plain_wave"; write_smpl_chunk $source_sample_rate $loop_start_frame $loop_end_frame; tail -c +$((canonical_data_chunk_offset + 1)) "$plain_wave"; } > "$smpl_before_data_wave" || exit 1
fix_riff_size "$smpl_before_data_wave" || exit 1

# The whole crackout music as an mp3 without loop tags (mp3 has no common loop tag, its region comes from the program)
lame --quiet -b 192 "$plain_wave" "$media_dir/crackout_44k.mp3" || exit 1

ls -l "$media_dir"
