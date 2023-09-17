/*
Name:
	Final MP3 Loader

Description:
	Simple limited mp3 loader based on minimp3.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2023 Torsten Spaete
*/

#ifndef FINAL_MP3LOADER_H
#define FINAL_MP3LOADER_H

#include <final_platform_layer.h>

#include "final_audio.h"

typedef enum MP3HeaderTestStatus {
	MP3HeaderTestStatus_Success = 0,
	MP3HeaderTestStatus_InvalidBuffer,
	MP3HeaderTestStatus_RequireMoreData,
	MP3HeaderTestStatus_NoMP3,
} MP3HeaderTestStatus;

extern MP3HeaderTestStatus TestMP3Header(const uint8_t *buffer, const size_t bufferSize, size_t* requiredBufferSize);
extern bool LoadMP3FromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
extern bool LoadMP3FromFile(const char *filePath, PCMWaveData *outWave);

#endif // FINAL_MP3LOADER_H

#if defined(FINAL_MP3LOADER_IMPLEMENTATION) && !defined(FINAL_MP3LOADER_IMPLEMENTED)
#define FINAL_MP3LOADER_IMPLEMENTED

#define MINIMP3_IMPLEMENTATION
#include <minimp3/minimp3_ex.h>

extern MP3HeaderTestStatus TestMP3Header(const uint8_t *buffer, const size_t bufferSize, size_t *requiredBufferSize) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(MP3HeaderTestStatus_InvalidBuffer);
	}
	mp3dec_t dec = fplZeroInit;
	mp3dec_init(&dec);

	if (bufferSize >= 4) {
		size_t id3v2size = mp3dec_skip_id3v2(buffer, bufferSize);
		if (bufferSize < id3v2size) {
			*requiredBufferSize = id3v2size + 4;
			return(MP3HeaderTestStatus_RequireMoreData);
		}

		if (id3v2size > 0) {
			buffer += id3v2size;
		}
		
		dec.header[0] = buffer[0];
		dec.header[1] = buffer[1];
		dec.header[2] = buffer[2];
		dec.header[3] = buffer[3];

		if (dec.header[0] == 0xff && hdr_compare(dec.header, buffer)) {
			return(MP3HeaderTestStatus_Success);
		}
	}

	return(MP3HeaderTestStatus_NoMP3);
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
		outWave->channelCount = fileInfo.channels;
		outWave->samplesPerSecond = fileInfo.hz;
		outWave->formatType = fplAudioFormatType_S16;
		outWave->bytesPerSample = fplGetAudioSampleSizeInBytes(outWave->formatType);
		outWave->frameCount = (uint32_t)(fileInfo.samples / fileInfo.channels);

		size_t sampleMemorySize = outWave->bytesPerSample * outWave->channelCount * outWave->frameCount;
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