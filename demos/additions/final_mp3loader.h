/*
Name:
	Final MP3 Loader

Description:
	Simple limited mp3 loader based on minimp3.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_MP3LOADER_H
#define FINAL_MP3LOADER_H

#include <final_platform_layer.h>

#include "final_audio.h"

typedef enum MP3HeaderTestStatus {
	MP3HeaderTestStatus_Success = 0,
	MP3HeaderTestStatus_InvalidBuffer,
	MP3HeaderTestStatus_RequireMoreDataBegin,
	MP3HeaderTestStatus_RequireMoreDataEnd,
	MP3HeaderTestStatus_NoMP3,
} MP3HeaderTestStatus;

extern MP3HeaderTestStatus TestMP3Header(const uint8_t *buffer, const size_t bufferSize, size_t* requiredBufferSize);

extern bool LoadMP3FromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
extern bool LoadMP3FromFile(const char *filePath, PCMWaveData *outWave);

extern bool LoadMP3FormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);

#endif // FINAL_MP3LOADER_H

#if defined(FINAL_MP3LOADER_IMPLEMENTATION) && !defined(FINAL_MP3LOADER_IMPLEMENTED)
#define FINAL_MP3LOADER_IMPLEMENTED

#ifndef MINIMP3_IMPLEMENTATION
#	define MINIMP3_IMPLEMENTATION
#endif
#include <minimp3/minimp3_ex.h>

extern MP3HeaderTestStatus TestMP3Header(const uint8_t *buffer, const size_t bufferSize, size_t *requiredBufferSize) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(MP3HeaderTestStatus_InvalidBuffer);
	}

	mp3dec_t dec = fplZeroInit;
	mp3dec_init(&dec);

	if (bufferSize < 4) {
		*requiredBufferSize = 4;
		return MP3HeaderTestStatus_RequireMoreDataBegin;
	}

	if (strncmp((char *)buffer, "ID3", 3) == 0) {
		// ID3v2 Tag Header Detected
		return MP3HeaderTestStatus_Success;
	} else if (hdr_valid(buffer)) {
		// Audio Frame Header Detected
		return MP3HeaderTestStatus_Success;
	} else {
		// ID3v1 Tag Search
		if (bufferSize <= 227) {
			*requiredBufferSize = (227 + 1);
			return MP3HeaderTestStatus_RequireMoreDataEnd;
		}
		if (strncmp((char *)(buffer + bufferSize - 227), "TAG+", 4) == 0) {
			// ID3v1.1 Tag Detected
			return MP3HeaderTestStatus_Success;
		} else if (strncmp((char *)(buffer + bufferSize - 128), "TAG", 3) == 0) {
			// ID3v1.0 Tag Detected
			return MP3HeaderTestStatus_Success;
		}
	}

	return(MP3HeaderTestStatus_NoMP3);
}

// Bytes of ID3v2 tag at the front of the buffer, which sit BEFORE the first MPEG frame and must be
// stepped over. The tag's size is a "syncsafe" integer: four bytes carrying seven bits each, so the byte
// stream can never contain a false frame sync. Zero when there is no tag.
//
// (minimp3_ex.h has its own mp3dec_skip_id3v2, and it is NOT used here on purpose: line 214 of that file
// says `uint32` where it means `uint32_t`, and the only reason the game compiles it at all is that
// stb_vorbis.c -- pulled in by final_vorbisloader.h, which lands in the same TU EARLIER -- happens to
// `typedef unsigned int uint32`. Swap those two includes and the build breaks in a vendored third-party
// file. This is ten lines of a documented format, so it is written here rather than resting on that.)
static size_t MP3SkipID3v2Tag(const uint8_t *buffer, const size_t bufferSize) {
	const size_t id3v2HeaderSize = 10;
	if (bufferSize < id3v2HeaderSize) {
		return 0;
	}
	if (!(buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3')) {
		return 0;
	}
	// Any of the size bytes with its top bit set means this is not a syncsafe integer, i.e. not a tag.
	if ((buffer[6] | buffer[7] | buffer[8] | buffer[9]) & 0x80) {
		return 0;
	}
	size_t tagSize = ((size_t)(buffer[6] & 0x7F) << 21) | ((size_t)(buffer[7] & 0x7F) << 14) | ((size_t)(buffer[8] & 0x7F) << 7) | (size_t)(buffer[9] & 0x7F);
	size_t totalSize = id3v2HeaderSize + tagSize;
	const uint8_t footerPresentFlag = 0x10;
	if (buffer[5] & footerPresentFlag) {
		totalSize += id3v2HeaderSize;
	}
	return (totalSize <= bufferSize) ? totalSize : bufferSize;
}

// Read an mp3's FORMAT without decoding it.
//
// This used to call mp3dec_load_buf and read fileInfo.samples off the result -- i.e. it decoded the whole
// file, allocated ~11x its size in PCM, and threw all of it away to learn one number. Measured on a 1.63 MB
// / 107 s track: 58.3 ms and 18 MB, against 0.51 ms and no allocation for the frame walk below, which
// arrives at exactly the same sample count. An mp3 has no single global header, but it has one per FRAME,
// so the length is the frames counted by hopping header to header -- exact for VBR as well as CBR, and
// unlike trusting a Xing tag's frame count it stays right when that tag is missing or lying.
//
// EVERY frame is counted, including an encoder's leading Xing/Info/VBRI tag frame. That frame carries no
// music -- media players discard it -- but LoadMP3FromBuffer DECODES it (to ~26 ms of silence), and this
// function's contract is to describe what that decode will produce. Skipping it here made the probe report
// 1152 frames fewer than the decoder for every VBR file, which is a buffer under-allocation waiting to
// happen for anyone who sizes from the probe.
extern bool LoadMP3FormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat) {
	if ((buffer == fpl_null) || (bufferSize == 0) || outFormat == fpl_null) {
		return(false);
	}

	fplClearStruct(outFormat);

	size_t id3v2Size = MP3SkipID3v2Tag(buffer, bufferSize);
	const uint8_t *scan = buffer + id3v2Size;
	size_t remaining = bufferSize - id3v2Size;

	uint64_t totalFrameSamples = 0;
	unsigned sampleRate = 0;
	unsigned channelCount = 0;
	bool isFirstFrame = true;
	int freeFormatBytes = 0;

	// Mirrors mp3dec_iterate_buf's loop, minus the decoding and the callback.
	while (remaining > 0) {
		int frameSize = 0;
		int skippedBytes = mp3d_find_frame(scan, (int)remaining, &freeFormatBytes, &frameSize);
		scan += skippedBytes;
		remaining -= (size_t)skippedBytes;
		if (skippedBytes > 0 && frameSize == 0) {
			continue; // junk between frames -- mp3d_find_frame already stepped past it
		}
		if (frameSize == 0) {
			break; // no further frame in what is left
		}
		const uint8_t *frameHeader = scan;
		if (isFirstFrame) {
			sampleRate = hdr_sample_rate_hz(frameHeader);
			channelCount = HDR_IS_MONO(frameHeader) ? 1 : 2;
		}
		totalFrameSamples += hdr_frame_samples(frameHeader);
		isFirstFrame = false;
		scan += frameSize;
		remaining -= (size_t)frameSize;
	}

	if (totalFrameSamples == 0 || sampleRate == 0 || channelCount == 0) {
		return false;
	}

	outFormat->channelCount = (uint16_t)channelCount;
	outFormat->samplesPerSecond = sampleRate;
	outFormat->formatType = fplAudioFormatType_S16; // what LoadMP3FromBuffer decodes to
	outFormat->bytesPerSample = fplGetAudioSampleSizeInBytes(outFormat->formatType);
	outFormat->frameCount = (uint32_t)totalFrameSamples;
	return true;
}

extern bool LoadMP3FromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(false);
	}
	if (outWave == fpl_null) {
		return(false);
	}
	mp3dec_t dec = fplZeroInit;
	mp3dec_file_info_t fileInfo = fplZeroInit;
	mp3dec_load_buf(&dec, buffer, bufferSize, &fileInfo, fpl_null, fpl_null);

	bool result = false;
	if (fileInfo.samples > 0) {
		outWave->format.channelCount = fileInfo.channels;
		outWave->format.samplesPerSecond = fileInfo.hz;
		outWave->format.formatType = fplAudioFormatType_S16;
		outWave->format.bytesPerSample = fplGetAudioSampleSizeInBytes(outWave->format.formatType);
		outWave->format.frameCount = (uint32_t)(fileInfo.samples / fileInfo.channels);

		size_t sampleMemorySize = outWave->format.bytesPerSample * outWave->format.channelCount * outWave->format.frameCount;
		outWave->samplesSize = sampleMemorySize;
		outWave->isamples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
		fplMemoryCopy(fileInfo.buffer, sampleMemorySize, outWave->isamples);
		outWave->isValid = true;
		result = true;
	}
	if (fileInfo.buffer != fpl_null) {
		free(fileInfo.buffer);
	}
	return(result);
}

extern bool LoadMP3FromFile(const char *filePath, PCMWaveData *outWave) {
	bool result = false;
	fplFileHandle file;
	if (fplFileOpenBinary(filePath, &file)) {
		size_t length = fplFileGetSizeFromHandle32(&file);
		uint8_t *contents = (uint8_t *)fplMemoryAllocate(length);
		if (contents != fpl_null) {
			if (fplFileReadBlock32(&file, (uint32_t)length, contents, (uint32_t)length) == length) {
				result = LoadMP3FromBuffer(contents, length, outWave);
			}
			fplMemoryFree(contents);
		}
		fplFileClose(&file);
	}
	return(result);
}

#endif // FINAL_MP3LOADER_IMPLEMENTATION