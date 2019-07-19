#ifndef FINAL_AUDIO_H
#define FINAL_AUDIO_H

#include <final_platform_layer.h>

typedef struct PCMWaveData {
	//! Total frame count
	uint32_t frameCount;
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
} PCMWaveData;

inline void FreeWaveData(PCMWaveData *wave) {
	if (wave != fpl_null) {
		if (wave->samples != fpl_null) {
			fplMemoryFree(wave->samples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

inline void PushWaveError(PCMWaveData *outWave, const char *format, ...) {

	outWave->lastError[0] = 0;
	va_list argList;
	va_start(argList, format);
	fplFormatStringArgs(outWave->lastError, fplArrayCount(outWave->lastError), format, argList);
	va_end(argList);
}

#define FOURCC32(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#endif // FINAL_AUDIO_H