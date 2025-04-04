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

#ifndef FINAL_WAVELOADER_H
#define FINAL_WAVELOADER_H

#include <final_platform_layer.h>

#include "final_audio.h"

extern bool TestWaveHeader(const uint8_t *buffer, const size_t bufferSize);

extern bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
extern bool LoadWaveFromFile(const char *filePath, PCMWaveData *outWave);

extern bool LoadWaveFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);

extern void FreeWave(PCMWaveData *wave);

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

extern bool TestWaveHeader(const uint8_t *buffer, const size_t bufferSize) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(false);
	}
	if (bufferSize < sizeof(WaveHeader)) {
		return(false);
	}
	WaveHeader *header = (WaveHeader *)buffer;
	if (header->chunkId != WaveChunkId_RIFF || header->formatId != WaveChunkId_WAVE) {
		return(false);
	}
	return(true);
}

static void ConvertWaveFormatExToPCMWaveFormat(const WaveFormatEx *sourceFormat, const uint32_t dataSize, PCMWaveFormat *targetFormat) {
	fplAssert((sourceFormat->bitsPerSample > 0) && (sourceFormat->bitsPerSample % 8 == 0));
	uint16_t channelCount = sourceFormat->numberOfChannels;
	uint32_t bytesPerSample = sourceFormat->bitsPerSample / 8;
	uint32_t frameCount = dataSize / (channelCount * bytesPerSample);
	
	targetFormat->channelCount = channelCount;
	targetFormat->samplesPerSecond = sourceFormat->samplesPerSecond;
	targetFormat->frameCount = frameCount;
	targetFormat->bytesPerSample = bytesPerSample;

	targetFormat->formatType = fplAudioFormatType_None;
	if(bytesPerSample == 1) {
		targetFormat->formatType = fplAudioFormatType_U8;
	} else if (bytesPerSample == 2) {
		targetFormat->formatType = fplAudioFormatType_S16;
	} else if (bytesPerSample == 3) {
		targetFormat->formatType = fplAudioFormatType_S24;
	} else if (bytesPerSample == 4) {
		if (sourceFormat->formatTag == WaveFormatTags_PCM)
			targetFormat->formatType = fplAudioFormatType_S32;
		else
			targetFormat->formatType = fplAudioFormatType_F32;
	}
}

extern bool LoadWaveFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat) {
	if((buffer == fpl_null) || (bufferSize == 0) || outFormat == fpl_null) {
		return false;
	}
	if(bufferSize < sizeof(WaveHeader)) {
		return false;
	}
	WaveHeader *header = (WaveHeader *)buffer;
	if(header->chunkId != WaveChunkId_RIFF || header->formatId != WaveChunkId_WAVE) {
		return false;
	}

	bool result = false;

	fplClearStruct(outFormat);

	WaveFormatEx waveFormat = fplZeroInit;

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
					return false;
				}
			} break;

			case WaveChunkId_Data:
			{
				uint32_t dataSize = chunk->size;
				const uint8_t *data = buffer + bufferPosition;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					case WaveFormatTags_IEEEFloat:
					{
						ConvertWaveFormatExToPCMWaveFormat(&waveFormat, dataSize, outFormat);
						result = true;
					} break;
				}
			} break;
		}

		if((bufferPosition + nextChunkOffset + sizeof(WaveChunk)) <= bufferSize) {
			bufferPosition += nextChunkOffset;
			chunk = (const WaveChunk *)(buffer + bufferPosition);
		} else {
			chunk = fpl_null;
			break;
		}
	}
	return(result);
}

extern bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave) {
	if((buffer == fpl_null) || (bufferSize == 0)) {
		return false;
	}
	if(outWave == fpl_null) {
		return false;
	}
	if(bufferSize < sizeof(WaveHeader)) {
		PushWaveError(outWave, "File is not a wave-file!");
		return false;
	}
	WaveHeader *header = (WaveHeader *)buffer;
	if(header->chunkId != WaveChunkId_RIFF || header->formatId != WaveChunkId_WAVE) {
		PushWaveError(outWave, "File is not a wave-file, wrong chunk or format id!");
		return false;
	}

	bool result = false;
	fplMemoryClear(outWave, sizeof(*outWave));

	WaveFormatEx waveFormat = fplZeroInit;

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
					PushWaveError(outWave, "Unsupported wave format tag '%d'", format->formatTag);
					return false;
				}
			} break;

			case WaveChunkId_Data:
			{
				uint32_t dataSize = chunk->size;
				const uint8_t *data = buffer + bufferPosition;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					case WaveFormatTags_IEEEFloat:
					{
						ConvertWaveFormatExToPCMWaveFormat(&waveFormat, dataSize, &outWave->format);
						size_t sampleMemorySize = outWave->format.bytesPerSample * outWave->format.channelCount * outWave->format.frameCount;
						fplAssert(sampleMemorySize == dataSize);
						outWave->samplesSize = sampleMemorySize;
						outWave->isamples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
						fplMemoryCopy(data, sampleMemorySize, outWave->isamples);
						outWave->isValid = true;
						result = true;
					} break;
				}
			} break;
		}

		if((bufferPosition + nextChunkOffset + sizeof(WaveChunk)) <= bufferSize) {
			bufferPosition += nextChunkOffset;
			chunk = (const WaveChunk *)(buffer + bufferPosition);
		} else {
			chunk = fpl_null;
			break;
		}
	}
	return(result);
}

extern bool LoadWaveFromFile(const char *filePath, PCMWaveData *outWave) {
	bool result = false;
	fplFileHandle file;
	if(fplFileOpenBinary(filePath, &file)) {
		size_t length = fplFileGetSizeFromHandle32(&file);
		uint8_t *contents = (uint8_t *)fplMemoryAllocate(length);
		if(contents != fpl_null) {
			if(fplFileReadBlock32(&file, (uint32_t)length, contents, (uint32_t)length) == length) {
				result = LoadWaveFromBuffer(contents, length, outWave);
			}
			fplMemoryFree(contents);
		}
		fplFileClose(&file);
	}
	return(result);
}

extern void FreeWave(PCMWaveData *wave) {
	if(wave != fpl_null) {
		if(wave->isamples != fpl_null) {
			fplMemoryFree(wave->isamples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

#endif // FINAL_WAVELOADER_IMPLEMENTATION