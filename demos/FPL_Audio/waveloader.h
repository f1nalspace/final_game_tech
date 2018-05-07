#ifndef WAVELOADER_H
#define WAVELOADER_H

#include <final_platform_layer.h>

typedef struct LoadedWave {
	//! Sample count per channel
	uint32_t sampleCount;
	uint32_t samplesPerSecond;
	uint32_t bytesPerSample;
	uint32_t channelCount;
	//! Interleaved samples
	uint8_t *samples;
	char lastError[1024];
	bool isValid;
} LoadedWave;

extern bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, LoadedWave *outWave);
extern bool LoadWaveFromFile(const char *filePath, LoadedWave *outWave);
extern void FreeWave(LoadedWave *wave);

#endif // WAVELOADER_H