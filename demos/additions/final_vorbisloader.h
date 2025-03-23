/*
Name:
	Final Wave Loader

Description:
	Simple limited wave loader

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_VORBISLOADER_H
#define FINAL_VORBISLOADER_H

#include <final_platform_layer.h>

#include "final_audio.h"

extern bool TestVorbisHeader(const uint8_t *buffer, const size_t bufferSize);
extern bool LoadVorbisFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
extern bool LoadVorbisFromFile(const char *filePath, PCMWaveData *outWave);

extern bool LoadVorbisFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);

#endif // FINAL_VORBISLOADER_H

#if defined(FINAL_VORBISLOADER_IMPLEMENTATION) && !defined(FINAL_VORBISLOADER_IMPLEMENTED)
#define FINAL_VORBISLOADER_IMPLEMENTED

#include <stb/stb_vorbis.c>

#define OGG_MAGIC FOURCC32('O', 'g', 'g', 'S')

#pragma pack(push, 1)
typedef struct OggFileHeader {
	uint32 capturePattern;
} OggFileHeader;
#pragma pack(pop)

extern bool TestVorbisHeader(const uint8_t *buffer, const size_t bufferSize) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(false);
	}
	if (bufferSize < sizeof(OggFileHeader)) {
		return(false);
	}
	OggFileHeader *header = (OggFileHeader *)buffer;
	if (header->capturePattern != OGG_MAGIC) {
		return(false);
	}
	return(true);
}

extern bool LoadVorbisFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat) {
	if((buffer == fpl_null) || (bufferSize == 0) || outFormat == fpl_null) {
		return(false);
	}

	int openErr = 0;
	stb_vorbis *vorbis = stb_vorbis_open_memory(buffer, (int)bufferSize, &openErr, fpl_null);
	if (vorbis == fpl_null) {
		return false;
	}

	uint32_t sampleCount = stb_vorbis_stream_length_in_samples(vorbis);

	fplClearStruct(outFormat);

	if (sampleCount > 0) {
		outFormat->formatType = fplAudioFormatType_S16;
		outFormat->bytesPerSample = fplGetAudioFrameSizeInBytes(outFormat->formatType, 1);
		outFormat->channelCount = (uint16_t)fplMax(0, vorbis->channels);
		outFormat->samplesPerSecond = (uint32_t)fplMax(0, vorbis->sample_rate);
		outFormat->frameCount = fplMax(0, sampleCount / outFormat->channelCount);
	}

	stb_vorbis_close(vorbis);

	return true;
}

extern bool LoadVorbisFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave) {
	if((buffer == fpl_null) || (bufferSize == 0) || outWave == fpl_null) {
		return(false);
	}

	int channels = 0;
	int sampleRate = 0;
	short *output = fpl_null;
	int sampleCount = stb_vorbis_decode_memory(buffer, (int)bufferSize, &channels, &sampleRate, &output);
	if (sampleCount <= 0) {
		return(false);
	}

	outWave->format.formatType = fplAudioFormatType_S16;
	outWave->format.bytesPerSample = fplGetAudioFrameSizeInBytes(outWave->format.formatType, 1);
	outWave->format.samplesPerSecond = (uint32_t)fplMax(0, sampleRate);
	outWave->format.channelCount = (uint16_t)fplMax(0, channels);
	outWave->format.frameCount = fplMax(0, sampleCount / outWave->format.channelCount);

	size_t sampleMemorySize = outWave->format.bytesPerSample * outWave->format.channelCount * outWave->format.frameCount;
	outWave->samplesSize = sampleMemorySize;
	outWave->isamples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
	fplMemoryCopy(output, sampleMemorySize, outWave->isamples);
	outWave->isValid = true;

	free(output);

	return(true);
}

extern bool LoadVorbisFromFile(const char *filePath, PCMWaveData *outWave) {
	bool result = false;
	fplFileHandle file;
	if(fplFileOpenBinary(filePath, &file)) {
		size_t length = fplFileGetSizeFromHandle32(&file);
		uint8_t *contents = (uint8_t *)fplMemoryAllocate(length);
		if(contents != fpl_null) {
			if(fplFileReadBlock32(&file, (uint32_t)length, contents, (uint32_t)length) == length) {
				result = LoadVorbisFromBuffer(contents, length, outWave);
			}
			fplMemoryFree(contents);
		}
		fplFileClose(&file);
	}
	return(result);
}

#endif // FINAL_VORBISLOADER_IMPLEMENTATION