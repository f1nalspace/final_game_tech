/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio
Description:
	This demo shows how to play a contiguous sine or square wave.
	Also it can play uncompressed PCM wave data with simple resampling support.
Requirements:
	- C-Runtime (sinf)
Author:
	Torsten Spaete
Changelog:
	## 2018-05-08
	- Simple Up/Down sampling
    - Add waveloader.c to makefiles
    - Use optional argument as wavefile to load
	## 2018-05-07
	- Introduce sample buffering
	- Written a simple wave file loader
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
#define MAX_AUDIOBUFFER_BYTES_PER_SAMPLE 4
#define MAX_AUDIOBUFFER_CHANNEL_COUNT 2
typedef struct AudioBuffer {
	// @NOTE(final): S16, max 2 channels, 4096 max samples
	uint8_t samples[MAX_AUDIOBUFFER_BYTES_PER_SAMPLE * MAX_AUDIOBUFFER_CHANNEL_COUNT * MAX_AUDIOBUFFER_SAMPLE_COUNT];
	uint32_t maxSampleCount;
	uint32_t framesRemaining;
	uint32_t sampleIndex;
} AudioBuffer;

typedef struct AudioData {
	LoadedWave loadedWave;
	AudioBuffer conversionBuffer;
	uint32_t toneHz;
	uint32_t toneVolume;
	uint32_t wavePeriod;
	bool useSquareWave;
	uint32_t sourceSampleIndex;
} AudioData;

static const float PI32 = 3.14159265359f;

static void FreeAudioData(AudioData *audioData) {
	FreeWave(&audioData->loadedWave);
}

static bool InitAudioData(const fplAudioDeviceFormat *nativeFormat, AudioData *audioData, const char *audioFilePath) {
    if (audioFilePath != fpl_null) {
        if (!LoadWaveFromFile(audioFilePath, &audioData->loadedWave)) {
            fplConsoleFormatError("Failed loading wave-file '%s': %s\n", audioFilePath,
                                  audioData->loadedWave.lastError);
        }
    }
	audioData->conversionBuffer.maxSampleCount = MAX_AUDIOBUFFER_SAMPLE_COUNT;
	return true;
}

static uint32_t WriteSamplesForFrame(int16_t *inSamples, uint32_t inChannels, int16_t *outSamples, uint32_t outChannels) {
	uint32_t writtenSampleCount = 0;
	if(inChannels > 0) {
		if(outChannels != inChannels) {
			int16_t sampleValue = inSamples[0];
			for(uint32_t i = 0; i < outChannels; ++i) {
				outSamples[i] = sampleValue;
				++writtenSampleCount;
			}
		} else {
			FPL_ASSERT(inChannels == outChannels);
			for(uint32_t i = 0; i < inChannels; ++i) {
				outSamples[i] = inSamples[i];
				++writtenSampleCount;
			}
		}
	}
	return(writtenSampleCount);
}

static bool FillConversionBuffer(AudioData *audioData, const fplAudioDeviceFormat *nativeFormat, const uint32_t outSampleCount) {
	audioData->conversionBuffer.framesRemaining = 0;
	audioData->conversionBuffer.sampleIndex = 0;
	int16_t *outSamples = (int16_t *)audioData->conversionBuffer.samples;
	uint32_t outBytesPerSample = fplGetAudioSampleSizeInBytes(nativeFormat->type);
	uint32_t outChannelCount = nativeFormat->channels;
	FPL_ASSERT(nativeFormat->type == fplAudioFormatType_S16);
	if(!audioData->loadedWave.isValid) {
		// @NOTE(final): Compute samples from sine wave or square wave
		uint32_t halfWavePeriod = audioData->wavePeriod / 2;
		for(size_t sampleIndex = 0; sampleIndex < outSampleCount; ++sampleIndex) {
			int16_t inSampleValue;
			if(audioData->useSquareWave) {
				inSampleValue = ((audioData->sourceSampleIndex / halfWavePeriod) % 2) ? (int16_t)audioData->toneVolume : -(int16_t)audioData->toneVolume;
			} else {
				float t = 2.0f * PI32 * (float)audioData->sourceSampleIndex / (float)audioData->wavePeriod;
				inSampleValue = (int16_t)(sinf(t) * audioData->toneVolume);
			}
			int16_t inSampleValues[2];
			FPL_ASSERT(nativeFormat->channels <= FPL_ARRAYCOUNT(inSampleValues));
			for(uint32_t channelIndex = 0; channelIndex < outChannelCount; ++channelIndex) {
				inSampleValues[channelIndex] = inSampleValue;
			}
			++audioData->sourceSampleIndex;

			uint32_t writtenSamples = WriteSamplesForFrame(inSampleValues, outChannelCount, outSamples, outChannelCount);
			outSamples += writtenSamples;
			++audioData->conversionBuffer.framesRemaining;
		}
	} else {
		LoadedWave *wave = &audioData->loadedWave;

		uint32_t inSampleRate = wave->samplesPerSecond;
		uint32_t inTotalSampleCount = wave->sampleCount;
		uint32_t inChannelCount = wave->channelCount;
		uint32_t inBytesPerSample = wave->bytesPerSample;
		FPL_ASSERT(audioData->sourceSampleIndex < wave->sampleCount);
		FPL_ASSERT(wave->bytesPerSample == 2);
		int16_t *inSamples = (int16_t *)(wave->samples + audioData->sourceSampleIndex * (inChannelCount * inBytesPerSample));
		uint32_t inRemainingSampleCount = inTotalSampleCount - audioData->sourceSampleIndex;

		uint32_t outSampleRate = nativeFormat->sampleRate;

		if(inSampleRate == outSampleRate) {
			// Sample rates are equal, just write out the samples
			int inSampleCount = FPL_MIN(outSampleCount, inRemainingSampleCount);
			for(int i = 0; i < inSampleCount; ++i) {
				int16_t inSampleValues[2];
				for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
					int16_t inSampleValue = *inSamples;
					inSampleValues[inChannelIndex] = inSampleValue;
					++inSamples;
				}
				++audioData->sourceSampleIndex;

				uint32_t writtenSamples = WriteSamplesForFrame(inSampleValues, inChannelCount, outSamples, outChannelCount);
				outSamples += writtenSamples;
				++audioData->conversionBuffer.framesRemaining;
			}
		} else if(outSampleRate > 0 && inSampleRate > 0 && inTotalSampleCount > 0) {
			bool isEven = (outSampleRate > inSampleRate) ? ((outSampleRate % inSampleRate) == 0) : ((inSampleRate % outSampleRate) == 0);
			if(isEven) {
				if(outSampleRate > inSampleRate) {
					// @NOTE(final): Simple Upsampling (2x, 4x, 6x, 8x, etc.)
					int upsamplingFactor = outSampleRate / inSampleRate;
					int inSampleCount = FPL_MIN(outSampleCount / upsamplingFactor, inRemainingSampleCount);
					for(int i = 0; i < inSampleCount; ++i) {
						int16_t inSampleValues[2];
						for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							int16_t inSampleValue = *inSamples;
							inSampleValues[inChannelIndex] = inSampleValue;
							++inSamples;
						}
						++audioData->sourceSampleIndex;

						for(int f = 0; f < upsamplingFactor; ++f) {
							uint32_t writtenSamples = WriteSamplesForFrame(inSampleValues, inChannelCount, outSamples, outChannelCount);
							outSamples += writtenSamples;
							++audioData->conversionBuffer.framesRemaining;
						}
					}
				} else {
					// @TODO(final): Simple Downsampling (1/2, 1/4, 1/6, 1/8, etc.)
					FPL_ASSERT(inSampleRate > outSampleRate);
					int downsamplingCount = inSampleRate / outSampleRate;
					int inSampleCount = FPL_MIN(outSampleCount * downsamplingCount, inRemainingSampleCount);
					for(int i = 0; i < inSampleCount; i += downsamplingCount) {
						int16_t inSampleValues[2];
						for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							int16_t inSampleValue = *(inSamples + (inChannelCount * i));
							inSampleValues[inChannelIndex] = inSampleValue;
						}
						audioData->sourceSampleIndex += downsamplingCount;

						uint32_t writtenSamples = WriteSamplesForFrame(inSampleValues, inChannelCount, outSamples, outChannelCount);
						outSamples += writtenSamples;
						++audioData->conversionBuffer.framesRemaining;
					}
				}
			} else {
				// @TODO(final): Convert from odd freqencies such as 22050 Hz to 48000 Hz, etc.
				// This requires real DSP!
			}

			// @NOTE(final): Loop wave when it is finished
			if(audioData->sourceSampleIndex == inTotalSampleCount) {
				audioData->sourceSampleIndex = 0;
			}
		}
	}
	return audioData->conversionBuffer.framesRemaining > 0;
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
		if(convBuffer->framesRemaining > 0) {
			uint32_t maxFramesToRead = convBuffer->framesRemaining;
			uint32_t framesToRead = FPL_MIN(remainingFrames, maxFramesToRead);
			size_t bytesToCopy = framesToRead * outputSampleStride;

			size_t sourcePosition = convBuffer->sampleIndex * outputSampleStride;
			FPL_ASSERT(sourcePosition < maxConversionAudioBufferSize);

			size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
			FPL_ASSERT(destPosition < maxOutputSampleBufferSize);

			fplMemoryCopy((uint8_t *)convBuffer->samples + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

			remainingFrames -= framesToRead;
			audioData->conversionBuffer.sampleIndex += framesToRead;
			audioData->conversionBuffer.framesRemaining -= framesToRead;
			result += framesToRead;
		}

		if(remainingFrames == 0) {
			// Done
			break;
		}

		// Conversion buffer is empty, fill it with new data
		if(audioData->conversionBuffer.framesRemaining == 0) {
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
	return result;
}

int main(int argc, char **args) {
	const char *filePath = (argc == 2) ? args[1] : fpl_null;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	settings.audio.deviceFormat.type = fplAudioFormatType_S16;
	settings.audio.deviceFormat.channels = 2;

	//settings.audio.deviceFormat.sampleRate = 11025;
	//settings.audio.deviceFormat.sampleRate = 22050;
	settings.audio.deviceFormat.sampleRate = 44100;
	//settings.audio.deviceFormat.sampleRate = 48000;

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
		if(InitAudioData(&nativeFormat, &audioData, filePath)) {
			// Start audio playback (This will start calling clientReadCallback regulary)
			if(fplPlayAudio() == fplAudioResult_Success) {
				const char *outFormat = fplGetAudioFormatString(nativeFormat.type);
				uint32_t outSampleRate = nativeFormat.sampleRate;
				uint32_t outChannels = nativeFormat.channels;
				if(audioData.loadedWave.isValid) {
					const char *inFormat = fplGetAudioFormatString(audioData.loadedWave.formatType);
					uint32_t inSampleRate = audioData.loadedWave.samplesPerSecond;
					uint32_t inChannels = audioData.loadedWave.channelCount;
					fplConsoleFormatOut("Playing samples (%s, %lu Hz, %lu channels) -> (%s, %lu Hz, %lu channels)\n", inFormat, inSampleRate, inChannels, outFormat, outSampleRate, outChannels);
				} else {
					fplConsoleFormatOut("Generating samples (%s, %lu Hz and %lu channels)\n", outFormat, outSampleRate, outChannels);
				}
				// Wait for any key presses
				fplConsoleFormatOut("Press any key to stop playback\n", outFormat, outSampleRate, outChannels);
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