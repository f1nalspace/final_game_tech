/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio
Description:
	This demo shows how to play a contiguous sine or square wave
	in a fixed default format such as S16.
Requirements:
	- C-Runtime (sinf)
Author:
	Torsten Spaete
Changelog:
	## 2018-05-07
	- Introduce audio buffer
	- Simple wave file loader
	## 2018-05-05:
	- Fixed CMakeLists to compile properly
	- Fixed Makefile to compile properly
	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#include <final_platform_layer.h>
#include <math.h> // sinf

#include "waveloader.h"

#define MAX_AUDIOBUFFER_SAMPLE_COUNT 4096
#define MAX_AUDIOBUFFER_CHANNEL_COUNT 2
typedef struct AudioBuffer {
	// @NOTE(final): S16, max 2 channels, 4096 max samples
	int16_t samples[sizeof(int16_t) * MAX_AUDIOBUFFER_CHANNEL_COUNT * MAX_AUDIOBUFFER_SAMPLE_COUNT];
	uint32_t maxSampleCount;
	uint32_t samplesRemaining;
	uint32_t sampleIndex;
} AudioBuffer;

typedef struct AudioData {
	LoadedWave loadedWave;
	AudioBuffer conversionBuffer;
	uint32_t toneHz;
	uint32_t toneVolume;
	uint32_t wavePeriod;
	bool useSquareWave;
	uint32_t runningSampleIndex;
} AudioData;

static const float PI32 = 3.14159265359f;

static void FreeAudioData(AudioData *audioData) {
	FreeWave(&audioData->loadedWave);
}

static bool InitAudioData(const fplAudioDeviceFormat *nativeFormat, AudioData *audioData) {
	char filePath[1024];
	fplGetHomePath(filePath, FPL_ARRAYCOUNT(filePath));
	fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, filePath, "Music", "fla22k_02.wav");

	if (!LoadWaveFromFile(filePath, &audioData->loadedWave)) {
		fplConsoleFormatError("Failed loading wave-file '%s': %s\n", filePath, audioData->loadedWave.lastError);
	}

	audioData->conversionBuffer.maxSampleCount = MAX_AUDIOBUFFER_SAMPLE_COUNT;

	return true;
}

static bool FillConversionBuffer(AudioData *audioData, const fplAudioDeviceFormat *nativeFormat, const uint32_t samplesToRead) {
	audioData->conversionBuffer.samplesRemaining = 0;
	audioData->conversionBuffer.sampleIndex = 0;
	if(!audioData->loadedWave.isValid) {
		uint32_t halfWavePeriod = audioData->wavePeriod / 2;
		int16_t *outSamples = audioData->conversionBuffer.samples;
		for(size_t sampleIndex = 0; sampleIndex < samplesToRead; ++sampleIndex) {
			int16_t sampleValue;
			if(audioData->useSquareWave) {
				sampleValue = ((audioData->runningSampleIndex / halfWavePeriod) % 2) ? (int16_t)audioData->toneVolume : -(int16_t)audioData->toneVolume;
			} else {
				float t = 2.0f * PI32 * (float)audioData->runningSampleIndex / (float)audioData->wavePeriod;
				sampleValue = (int16_t)(sinf(t) * audioData->toneVolume);
			}
			for(uint32_t channelIndex = 0; channelIndex < nativeFormat->channels; ++channelIndex) {
				*outSamples++ = sampleValue;
			}
			++audioData->runningSampleIndex;
			++audioData->conversionBuffer.samplesRemaining;
		}
	} else {
		LoadedWave *wave = &audioData->loadedWave;
		if(wave->samplesPerSecond < nativeFormat->sampleRate) {
			// Upsampling
		} else if(wave->samplesPerSecond > nativeFormat->sampleRate) {
			// Downsampling
		} else {
			// Copy (S16 only for now)
			FPL_ASSERT(wave->bytesPerSample == 2);
		}
	}
	return audioData->conversionBuffer.samplesRemaining > 0;
}

static uint32_t FillAudioBuffer(const fplAudioDeviceFormat *nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioData *audioData = (AudioData *)userData;
	FPL_ASSERT(audioData != NULL);
	FPL_ASSERT(nativeFormat->type == fplAudioFormatType_S16);
	FPL_ASSERT(nativeFormat->channels <= 2);
	uint32_t result = 0;

	uint32_t outputSampleStride = fplGetAudioFrameSizeInBytes(nativeFormat->type, nativeFormat->channels);
	uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

	AudioBuffer *convBuffer = &audioData->conversionBuffer;
	size_t maxConversionAudioBufferSize = fplGetAudioBufferSizeInBytes(nativeFormat->type, nativeFormat->channels, convBuffer->maxSampleCount);

	uint32_t remainingFrames = frameCount;
	while(remainingFrames > 0) {
		// Consume remaining samples in conversion buffer first
		if(convBuffer->samplesRemaining > 0) {
			uint32_t maxFramesToRead = convBuffer->samplesRemaining;
			uint32_t framesToRead = FPL_MIN(remainingFrames, maxFramesToRead);
			size_t bytesToCopy = framesToRead * outputSampleStride;

			size_t sourcePosition = convBuffer->sampleIndex * outputSampleStride;
			FPL_ASSERT(sourcePosition < maxConversionAudioBufferSize);

			size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
			FPL_ASSERT(destPosition < maxOutputSampleBufferSize);

			fplMemoryCopy((uint8_t *)convBuffer->samples + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

			remainingFrames -= framesToRead;
			audioData->conversionBuffer.sampleIndex += framesToRead;
			audioData->conversionBuffer.samplesRemaining -= framesToRead;
			result += framesToRead;
		}

		if(remainingFrames == 0) {
			// Done
			break;
		}

		// Conversion buffer is empty, fill it with new data
		if(audioData->conversionBuffer.samplesRemaining == 0) {
			if(!FillConversionBuffer(audioData, nativeFormat, remainingFrames)) {
				// @NOTE(final): No data available, clear remaining samples to zero (Silent)
				uint32_t samplesToClear = remainingFrames;
				size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
				size_t clearSize = remainingFrames * outputSampleStride;
				fplMemoryClear((uint8_t *)outputSamples + destPosition, clearSize);
				remainingFrames -= samplesToClear;
				result += samplesToClear;
			}
		}
	}

#if 0
	if(!audioData->loadedWave.isValid) {
		uint32_t halfWavePeriod = audioData->wavePeriod / 2;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			int16_t sampleValue;
			if(audioData->useSquareWave) {
				sampleValue = ((audioData->runningSampleIndex / halfWavePeriod) % 2) ? (int16_t)audioData->toneVolume : -(int16_t)audioData->toneVolume;
			} else {
				float t = 2.0f * PI32 * (float)audioData->runningSampleIndex / (float)audioData->wavePeriod;
				sampleValue = (int16_t)(sinf(t) * audioData->toneVolume);
			}
			for(uint32_t channelIndex = 0; channelIndex < nativeFormat->channels; ++channelIndex) {
				*outSamples++ = sampleValue;
			}
			++audioData->runningSampleIndex;
			++result;
		}
	} else {
		FPL_ASSERT(audioData->loadedWave.bytesPerSample == 2);
		uint32_t sourceChannelCount = audioData->loadedWave.channelCount;
		uint32_t targetChannelCount = nativeFormat->channels;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			uint32_t sampleIndex = audioData->runningSampleIndex;
			for(uint32_t channelIndex = 0; channelIndex < audioData->loadedWave.channelCount; ++channelIndex) {
				int16_t sampleValue = 0;
				*outSamples++ = sampleValue;
			}
			audioData->runningSampleIndex = (audioData->runningSampleIndex + 1) % audioData->loadedWave.sampleCount;
			++result;
		}
	}
#endif

	return result;
}

int main(int argc, char **args) {
	int result = -1;

	//
	// Create default settings with 48 KHz, 2 Channels, S16-Format
	//
	fplSettings settings = fplMakeDefaultSettings();
	settings.audio.deviceFormat.type = fplAudioFormatType_S16;
	settings.audio.deviceFormat.channels = 2;
	settings.audio.deviceFormat.sampleRate = 48000;

	// Optionally overwrite audio settings if needed

	// Setup some state for the sine/square wave generation
	AudioData audioData = FPL_ZERO_INIT;
	audioData.toneHz = 256;
	audioData.toneVolume = 1000;
	audioData.wavePeriod = settings.audio.deviceFormat.sampleRate / audioData.toneHz;
	audioData.useSquareWave = false;

	// Provide client read callback and optionally user data
	settings.audio.clientReadCallback = FillAudioBuffer;
	settings.audio.userData = &audioData;

	// Find audio device
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		return -1;
	}

	fplAudioDeviceInfo audioDeviceInfos[16] = FPL_ZERO_INIT;
	uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, FPL_ARRAYCOUNT(audioDeviceInfos));
	if(deviceCount > 0) {
		settings.audio.deviceInfo = audioDeviceInfos[0];
		fplConsoleFormatOut("Using audio device: %s\n", settings.audio.deviceInfo.name);
	}
	fplPlatformRelease();

	// Initialize the platform with audio enabled and the settings
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		return -1;
	}

	// Print out the native audio format
	fplAudioDeviceFormat nativeFormat;
	if(fplGetAudioHardwareFormat(&nativeFormat)) {
		// You can overwrite the client read callback and user data if you want to
		fplSetAudioClientReadCallback(FillAudioBuffer, &audioData);
		// Init audio data
		if(InitAudioData(&nativeFormat, &audioData)) {
			// Start audio playback (This will start calling clientReadCallback regulary)
			if(fplPlayAudio() == fplAudioResult_Success) {
				fplConsoleFormatOut("Audio with %lu KHz and %lu channels is playing, press any key to stop playback...\n", nativeFormat.sampleRate, nativeFormat.channels);
				// Wait for any key presses
				fplConsoleWaitForCharInput();
				// Stop audio playback
				fplStopAudio();
			}
			// Release audio data
			FreeAudioData(&audioData);
		}
	}

	// Release the platform
	fplPlatformRelease();

	return 0;
}