#ifndef WAVELOADER_H
#define WAVELOADER_H

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

#endif // WAVELOADER_H