/*
Name:
	Final Wave Loader

Description:
	Simple limited wave loader

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_WAVELOADER_H
#define FINAL_WAVELOADER_H

#include <final_platform_layer.h>

typedef struct LoadedWave {
	//! Total frame count
	uint32_t sampleCount;
	//! Samples per second (Frequency in Hz)
	uint32_t samplesPerSecond;
	//! Bytes per sample
	uint32_t bytesPerSample;
	//! Format type
	fplAudioFormatType formatType;
	//! Number of channels
	uint32_t channelCount;
	//! Size of samples in bytes
	size_t samplesSize;
	//! Interleaved samples (Max of 2 channels)
	uint8_t *samples;
	//! Last error string
	char lastError[1024];
	//! Is valid boolean flag
	bool isValid;
} LoadedWave;

extern bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, LoadedWave *outWave);
extern bool LoadWaveFromFile(const char *filePath, LoadedWave *outWave);
extern void FreeWave(LoadedWave *wave);

#endif // FINAL_WAVELOADER_H

#if defined(FINAL_WAVELOADER_IMPLEMENTATION) && !defined(FINAL_WAVELOADER_IMPLEMENTED)
#define FINAL_WAVELOADER_IMPLEMENTED

#define RIFF_ID(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#pragma pack(push, 1)
typedef struct WaveHeader {
	uint32_t chunkId;
	uint32_t chunkSize;
	uint32_t formatId;
} WaveHeader;

typedef struct WaveChunk {
	uint32_t id;
	uint32_t size;
} WaveChunk;

typedef enum WaveChunkId {
	WaveChunkId_RIFF = RIFF_ID('R', 'I', 'F', 'F'),
	WaveChunkId_WAVE = RIFF_ID('W', 'A', 'V', 'E'),
	WaveChunkId_Format = RIFF_ID('f', 'm', 't', ' '),
	WaveChunkId_Data = RIFF_ID('d', 'a', 't', 'a'),
} WaveChunkID;

typedef enum WaveFormatTags {
	WaveFormatTags_None = 0,
	WaveFormatTags_PCM = 1,
	WaveFormatTags_IEEEFloat = 3,
} WaveFormatTags;

typedef struct WaveFormatEx {
	uint16_t formatTag;
	uint16_t numberOfChannels;
	uint32_t samplesPerSecond;
	uint32_t avgBytesPerSample;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint16_t cbSize;
} WaveFormatEx;
#pragma pack(pop)

static void WaveError(LoadedWave *outWave, const char *format, ...) {

	outWave->lastError[0] = 0;
	va_list argList;
	va_start(argList, format);
	fplFormatAnsiStringArgs(outWave->lastError, FPL_ARRAYCOUNT(outWave->lastError), format, argList);
	va_end(argList);
}

extern bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, LoadedWave *outWave) {
	if((buffer == fpl_null) || (bufferSize == 0)) {
		return false;
	}
	if(outWave == fpl_null) {
		return false;
	}
	if(bufferSize < sizeof(WaveHeader)) {
		WaveError(outWave, "File is not a wave-file!");
		return false;
	}
	WaveHeader *header = (WaveHeader *)buffer;
	if(header->chunkId != WaveChunkId_RIFF || header->formatId != WaveChunkId_WAVE) {
		WaveError(outWave, "File is not a wave-file, wrong chunk or format id!");
		return false;
	}

	bool result = false;
	fplMemoryClear(outWave, sizeof(*outWave));

	WaveFormatEx waveFormat = FPL_ZERO_INIT;

	size_t bufferPosition = sizeof(*header);
	const WaveChunk *chunk = (const WaveChunk *)(buffer + bufferPosition);
	while(chunk != fpl_null) {
		bufferPosition += sizeof(WaveChunk);
		size_t nextChunkOffset = chunk->size;
		switch(chunk->id) {
			case WaveChunkId_Format:
			{
				const WaveFormatEx *format = (const WaveFormatEx *)(buffer + bufferPosition);
				if(format->formatTag == WaveFormatTags_PCM) {
					waveFormat = *format;
				} else if(format->formatTag == WaveFormatTags_IEEEFloat) {
					waveFormat = *format;
				} else {
					// Unsupported format
					WaveError(outWave, "Unsupported wave format tag '%d'", format->formatTag);
					return false;
				}
			} break;

			case WaveChunkId_Data:
			{
				uint32_t dataSize = chunk->size;
				const uint8_t *data = buffer + bufferPosition;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					{
						FPL_ASSERT((waveFormat.bitsPerSample > 0) && (waveFormat.bitsPerSample % 8 == 0));
						uint32_t channelCount = waveFormat.numberOfChannels;
						uint32_t bytesPerSample = waveFormat.bitsPerSample / 8;
						uint32_t sampleCount = dataSize / (channelCount * bytesPerSample);
						FPL_ASSERT(waveFormat.numberOfChannels <= 2);
						outWave->channelCount = channelCount;
						outWave->samplesPerSecond = waveFormat.samplesPerSecond;
						outWave->sampleCount = sampleCount;
						outWave->bytesPerSample = bytesPerSample;

						outWave->formatType = fplAudioFormatType_None;
						if(bytesPerSample == 1) {
							outWave->formatType = fplAudioFormatType_U8;
						} else if (bytesPerSample == 2) {
							outWave->formatType = fplAudioFormatType_S16;
						} else if (bytesPerSample == 3) {
							outWave->formatType = fplAudioFormatType_S24;
						} else if (bytesPerSample == 4) {
							outWave->formatType = fplAudioFormatType_S32;
						}

						size_t sampleMemorySize = bytesPerSample * channelCount * sampleCount;
						FPL_ASSERT(sampleMemorySize == dataSize);
						outWave->samplesSize = sampleMemorySize;
						outWave->samples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
						fplMemoryCopy(data, sampleMemorySize, outWave->samples);
						outWave->isValid = true;
						result = true;
					} break;

					case WaveFormatTags_IEEEFloat:
					{
						WaveError(outWave, "IEEE Float Wave Format is not supported yet!");
						return false;
					} break;
				}
			} break;
		}

		if((bufferPosition + nextChunkOffset) <= bufferSize) {
			bufferPosition += nextChunkOffset;
			chunk = (const WaveChunk *)(buffer + bufferPosition);
		} else {
			chunk = fpl_null;
			break;
		}
	}
	return(result);
}

extern bool LoadWaveFromFile(const char *filePath, LoadedWave *outWave) {
	bool result = false;
	fplFileHandle file;
	if(fplOpenAnsiBinaryFile(filePath, &file)) {
		size_t length = fplGetFileSizeFromHandle32(&file);
		uint8_t *contents = (uint8_t *)fplMemoryAllocate(length);
		if(contents != fpl_null) {
			if(fplReadFileBlock32(&file, (uint32_t)length, contents, (uint32_t)length) == length) {
				result = LoadWaveFromBuffer(contents, length, outWave);
			}
			fplMemoryFree(contents);
		}
		fplCloseFile(&file);
	}
	return(result);
}

extern void FreeWave(LoadedWave *wave) {
	if(wave != fpl_null) {
		if(wave->samples != fpl_null) {
			fplMemoryFree(wave->samples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

#endif // FINAL_WAVELOADER_IMPLEMENTATION