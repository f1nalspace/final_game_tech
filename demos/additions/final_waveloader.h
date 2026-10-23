/*
Name:
	Final Wave Loader

Description:
	Simple limited wave loader

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
	## 2026-10-22
	- Fixed: Chunks after a chunk of odd size were not found (pad byte)
	- Fixed: A huge chunk size could loop forever on 32-bit
	- New: The loop region from the first loop of the smpl chunk (PCMWaveFormat.loopStartFrame and loopEndFrame)

	## 2026-07-19
	- Fixed: Malformed WAV chunk sizes now bounds-checked (no more heap over-read / div-by-zero); allocation is null-checked
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
	WaveChunkId_Sampler = RIFF_ID('s', 'm', 'p', 'l'),
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

// Start of the smpl chunk, its sample loops follow right after it
typedef struct WaveSamplerHeader {
	uint32_t manufacturer;
	uint32_t product;
	uint32_t samplePeriod;
	uint32_t midiUnityNote;
	uint32_t midiPitchFraction;
	uint32_t smpteFormat;
	uint32_t smpteOffset;
	uint32_t sampleLoopCount;
	uint32_t samplerDataSize;
} WaveSamplerHeader;

typedef struct WaveSampleLoop {
	uint32_t identifier;
	uint32_t type;
	// First frame of the loop
	uint32_t start;
	// Last frame of the loop, it plays as well
	uint32_t end;
	uint32_t fraction;
	uint32_t playCount;
} WaveSampleLoop;
#pragma pack(pop)

typedef enum WaveSampleLoopType {
	WaveSampleLoopType_Forward = 0,
} WaveSampleLoopType;

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

static bool ConvertWaveFormatExToPCMWaveFormat(const WaveFormatEx *sourceFormat, const uint32_t dataSize, PCMWaveFormat *targetFormat) {
	uint16_t channelCount = sourceFormat->numberOfChannels;
	uint16_t bitsPerSample = sourceFormat->bitsPerSample;
	if ((channelCount == 0) || (bitsPerSample == 0) || (bitsPerSample % 8 != 0)) {
		return false;
	}
	uint32_t bytesPerSample = bitsPerSample / 8;
	uint32_t frameSize = channelCount * bytesPerSample;
	uint32_t frameCount = dataSize / frameSize;

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
		if (sourceFormat->formatTag == WaveFormatTags_PCM) {
			targetFormat->formatType = fplAudioFormatType_S32;
		} else {
			targetFormat->formatType = fplAudioFormatType_F32;
		}
	}
	return true;
}

// Where the chunk after a chunk body ends: every chunk of odd size is followed by a pad byte, which the last chunk of a file may lack
static size_t GetNextWaveChunkPosition(const size_t bodyPosition, const uint32_t chunkSize) {
	const uint32_t chunkAlignment = 2;
	const size_t padByteCount = chunkSize % chunkAlignment;
	const size_t result = bodyPosition + chunkSize + padByteCount;
	return(result);
}

// The first sample loop of a smpl chunk, false when the chunk has none
static bool ReadFirstWaveSampleLoop(const uint8_t *chunkBody, const uint32_t chunkSize, WaveSampleLoop *outLoop) {
	const size_t firstLoopEndOffset = sizeof(WaveSamplerHeader) + sizeof(WaveSampleLoop);
	if(chunkSize < firstLoopEndOffset) {
		return(false);
	}
	const WaveSamplerHeader *header = (const WaveSamplerHeader *)chunkBody;
	if(header->sampleLoopCount == 0) {
		return(false);
	}
	const WaveSampleLoop *firstLoop = (const WaveSampleLoop *)(chunkBody + sizeof(WaveSamplerHeader));
	*outLoop = *firstLoop;
	return(true);
}

// Sets the loop region of a forward sample loop that lies inside the frame count, otherwise the format keeps no region
static void SetWaveLoopRegion(const WaveSampleLoop *loop, PCMWaveFormat *targetFormat) {
	const bool isForward = (loop->type == WaveSampleLoopType_Forward);
	const bool isInsideFile = (loop->start <= loop->end) && (loop->end < targetFormat->frameCount);
	if(isForward && isInsideFile) {
		// The region ends after the last frame of the loop
		const uint32_t loopEndFrame = loop->end + 1;
		targetFormat->loopStartFrame = loop->start;
		targetFormat->loopEndFrame = loopEndFrame;
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

	// The smpl chunk may come before or after the data chunk
	WaveSampleLoop firstSampleLoop = fplZeroInit;
	bool hasSampleLoop = false;

	size_t bufferPosition = sizeof(*header);
	while(bufferPosition + sizeof(WaveChunk) <= bufferSize) {
		const WaveChunk *chunk = (const WaveChunk *)(buffer + bufferPosition);
		uint32_t chunkId = chunk->id;
		uint32_t chunkSize = chunk->size;
		bufferPosition += sizeof(WaveChunk);
		// Compared with what is left of the buffer, a sum of position and size overflows on 32-bit
		const size_t remainingBufferSize = bufferSize - bufferPosition;
		if(chunkSize > remainingBufferSize) {
			// Chunk body exceeds the buffer, malformed
			return false;
		}
		switch(chunkId) {
			case WaveChunkId_Format:
			{
				// Core PCM fmt chunk is 16 bytes, the optional cbSize field makes it 18
				const size_t minFormatChunkSize = 16;
				if(chunkSize < minFormatChunkSize || bufferPosition + sizeof(WaveFormatEx) > bufferSize) {
					return false;
				}
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
				uint32_t dataSize = chunkSize;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					case WaveFormatTags_IEEEFloat:
					{
						if(ConvertWaveFormatExToPCMWaveFormat(&waveFormat, dataSize, outFormat)) {
							result = true;
						}
					} break;
				}
			} break;

			case WaveChunkId_Sampler:
			{
				if(!hasSampleLoop) {
					hasSampleLoop = ReadFirstWaveSampleLoop(buffer + bufferPosition, chunkSize, &firstSampleLoop);
				}
			} break;
		}
		bufferPosition = GetNextWaveChunkPosition(bufferPosition, chunkSize);
	}
	if(result && hasSampleLoop) {
		SetWaveLoopRegion(&firstSampleLoop, outFormat);
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

	// The smpl chunk may come before or after the data chunk
	WaveSampleLoop firstSampleLoop = fplZeroInit;
	bool hasSampleLoop = false;

	size_t bufferPosition = sizeof(*header);
	while(bufferPosition + sizeof(WaveChunk) <= bufferSize) {
		const WaveChunk *chunk = (const WaveChunk *)(buffer + bufferPosition);
		uint32_t chunkId = chunk->id;
		uint32_t chunkSize = chunk->size;
		bufferPosition += sizeof(WaveChunk);
		// Compared with what is left of the buffer, a sum of position and size overflows on 32-bit
		const size_t remainingBufferSize = bufferSize - bufferPosition;
		if(chunkSize > remainingBufferSize) {
			// Chunk body exceeds the buffer, malformed
			PushWaveError(outWave, "Chunk body of size '%lu' exceeds the buffer!", (unsigned long)chunkSize);
			return false;
		}
		switch(chunkId) {
			case WaveChunkId_Format:
			{
				// Core PCM fmt chunk is 16 bytes, the optional cbSize field makes it 18
				const size_t minFormatChunkSize = 16;
				if(chunkSize < minFormatChunkSize || bufferPosition + sizeof(WaveFormatEx) > bufferSize) {
					PushWaveError(outWave, "Format chunk of size '%lu' is too small!", (unsigned long)chunkSize);
					return false;
				}
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
				uint32_t dataSize = chunkSize;
				const uint8_t *data = buffer + bufferPosition;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					case WaveFormatTags_IEEEFloat:
					{
						if(!ConvertWaveFormatExToPCMWaveFormat(&waveFormat, dataSize, &outWave->format)) {
							PushWaveError(outWave, "Invalid wave format (channels or bits-per-sample)!");
							return false;
						}
						size_t sampleMemorySize = outWave->format.bytesPerSample * outWave->format.channelCount * outWave->format.frameCount;
						// sampleMemorySize is derived from dataSize via integer division, so it can never exceed the verified in-buffer dataSize
						outWave->samplesSize = sampleMemorySize;
						outWave->isamples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
						if(outWave->isamples == fpl_null) {
							PushWaveError(outWave, "Failed allocating '%zu' bytes for wave samples!", sampleMemorySize);
							return false;
						}
						fplMemoryCopy(data, sampleMemorySize, outWave->isamples);
						outWave->isValid = true;
						result = true;
					} break;
				}
			} break;

			case WaveChunkId_Sampler:
			{
				if(!hasSampleLoop) {
					hasSampleLoop = ReadFirstWaveSampleLoop(buffer + bufferPosition, chunkSize, &firstSampleLoop);
				}
			} break;
		}
		bufferPosition = GetNextWaveChunkPosition(bufferPosition, chunkSize);
	}
	if(result && hasSampleLoop) {
		SetWaveLoopRegion(&firstSampleLoop, &outWave->format);
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