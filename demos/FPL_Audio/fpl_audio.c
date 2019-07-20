/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio

Description:
	This demo shows how to play a contiguous sine or square wave.
	Also it can play uncompressed PCM wave data with simple resampling support.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2019-07-19
	- Wave form history visualization
	- Moved sine wave generation out into final_audiosystem.h

	## 2019-05-22
	- Added support for playing multiple audio files from command line

	## 2018-12-06
	- Added basic support for loading IEEEFloat wave files

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2
	- Disable auto-play/stop of audio samples by default

	## 2018-06-06
	- Refactored files

	## 2018-05-13
	- Fixed Makefiles (Missing audiosystem.c)

	## 2018-05-09
	- Introduced audio system -> Prepare for mixing
	- Add support for audio format conversions

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
#define OPT_AUTO_RENDER_SAMPLES 1

#define OPT_PLAYBACK_NONE 0
#define OPT_PLAYBACK_SINEWAVE_ONLY 1
#define OPT_PLAYBACK_AUDIOSYSTEM_ONLY 2

#define OPT_PLAYBACK_MODE OPT_PLAYBACK_AUDIOSYSTEM_ONLY

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>
#define _USE_MATH_DEFINES
#include <math.h> // sinf, M_PI

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

typedef struct AudioDemo {
	AudioSystem *audioSys;
	fplAudioFormatType sampleFormat;
	AudioFrameCount maxFrameCount;
	size_t maxSizeBytes;
	AudioFrameCount writeFrameIndex;
	AudioFrameCount readFrameIndex;
	float *samples;
	uint32_t *periods;
	uint32_t periodCounter;
	AudioSineWaveData sineWave;
} AudioDemo;

static void AudioRenderSamples(AudioDemo *audioDemo, const void *inSamples, const fplAudioDeviceFormat *inFormat, const AudioFrameCount frameCount) {
	AudioSystem *audioSys = audioDemo->audioSys;

	const AudioChannelCount outChannels = audioSys->targetFormat.channels;
	const size_t inSampleSize = fplGetAudioSampleSizeInBytes(inFormat->type);

	AudioFrameCount inputFrameIndex = 0;
	AudioFrameCount remainingFrameCount = frameCount;
	fplAssert(frameCount <= audioDemo->maxFrameCount);
	while (remainingFrameCount > 0) {
		AudioFrameCount remainingFramesInDemo = fplMax(0, (int)(audioDemo->maxFrameCount - audioDemo->writeFrameIndex));
		AudioFrameCount framesToCopy = remainingFrameCount;
		if (framesToCopy >= remainingFramesInDemo) {
			framesToCopy = remainingFramesInDemo;
		}
		AudioSampleCount outputSampleIndex = audioDemo->writeFrameIndex * outChannels;
		AudioSampleCount inputSampleIndex = inputFrameIndex * inFormat->channels;
		for (AudioFrameCount frameIndex = 0; frameIndex < framesToCopy; ++frameIndex) {
			for (AudioChannelCount channelIndex = 0; channelIndex < inFormat->channels; ++channelIndex) {
				audioDemo->samples[outputSampleIndex++] = ConvertToF32((uint8_t *)inSamples + inputSampleIndex * inSampleSize, channelIndex, inFormat->type);
			}
			inputSampleIndex += inFormat->channels;
		}

		fplMemorySet(&audioDemo->periods[audioDemo->writeFrameIndex], (uint8_t)audioDemo->periodCounter, framesToCopy * sizeof(uint32_t));

		inputFrameIndex += framesToCopy;
		remainingFrameCount -= framesToCopy;
		audioDemo->writeFrameIndex = (audioDemo->writeFrameIndex + framesToCopy) % audioDemo->maxFrameCount;
		audioDemo->periodCounter++;
	}
}


static uint32_t AudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	AudioDemo *audioDemo = (AudioDemo *)userData;

	AudioFrameCount result = 0;

#if OPT_PLAYBACK_MODE == OPT_PLAYBACK_SINEWAVE_ONLY

#if 0
	// 100% Clean sine wave
	int16_t *outSamples = (int16_t *)outputSamples;
	uint32_t wavePeriod = outFormat->sampleRate / audioDemo->sineWave.toneHz;
	for (uint32_t frameIndex = 0; frameIndex < maxFrameCount; ++frameIndex) {
		float t = 2.0f * (float)M_PI * (float)audioDemo->sineWave.frameIndex++ / (float)wavePeriod;
		int16_t sampleValue = (int16_t)(sinf(t) * audioDemo->sineWave.toneVolume * (float)INT16_MAX);
		for (uint32_t channelIndex = 0; channelIndex < outFormat->channels; ++channelIndex) {
			*outSamples++ = sampleValue;
		}
		++result;
	}
#else
	result = maxFrameCount;
	AudioGenerateSineWave(&audioDemo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
#endif

#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY


#if OPT_PLAYBACK_MODE == OPT_PLAYBACK_AUDIOSYSTEM_ONLY
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	AudioFrameCount frameCount = AudioSystemWriteSamples(audioDemo->audioSys, outputSamples, outFormat, maxFrameCount);
	result = frameCount;
#endif // OPT_OUTPUT_SAMPLES_SYSTEM_ONLY

#if OPT_AUTO_RENDER_SAMPLES
	AudioRenderSamples(audioDemo, outputSamples, outFormat, result);
#endif

	return(result);
}

static bool InitAudioData(const fplAudioDeviceFormat *targetFormat, AudioSystem *audioSys, const char **files, const size_t fileCount, const bool generateSineWave, const AudioSineWaveData *sineWave) {
	if (!AudioSystemInit(audioSys, targetFormat)) {
		return false;
	}

	// Play audio files
	for (size_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		const char *filePath = files[fileIndex];
		if (filePath != fpl_null) {
			fplConsoleFormatOut("Loading audio file '%s\n", filePath);
			AudioSource *source = AudioSystemLoadFileSource(audioSys, filePath);
			if (source != fpl_null) {
				AudioSystemPlaySource(audioSys, source, true, 1.0f);
			}
		}
	}

	// Generate sine wave for some duration
	if (generateSineWave) {
		// @FIXME(final): If wave duration is smaller than actual audio buffer, we will hear a bad click
		const double waveDuration = 0.1f;
		AudioSineWaveData waveData = *sineWave;
		AudioHertz sampleRate = audioSys->targetFormat.sampleRate;
		AudioChannelCount channels = audioSys->targetFormat.channels;
		AudioFrameCount frameCount = (AudioFrameCount)(sampleRate * waveDuration + 0.5);
		AudioSource *source = AudioSystemAllocateSource(audioSys, audioSys->targetFormat.channels, audioSys->targetFormat.sampleRate, fplAudioFormatType_S16, frameCount);
		if (source != fpl_null) {
			AudioGenerateSineWave(&waveData, source->samples, source->format, source->samplesPerSeconds, source->channels, source->frameCount);
			AudioSystemPlaySource(audioSys, source, true, 1.0f);
		}
	}
	return(true);
}

inline int ClampInt(int value, int min, int max) {
	int result = value;
	if (result < min) result = min;
	if (result > max) result = max;
	return(result);
}

static void FillBackRect(fplVideoBackBuffer *backBuffer, float x0, float y0, float x1, float y1, int color) {
	int minX = (int)(x0 + 0.5f);
	int minY = (int)(y0 + 0.5f);
	int maxX = (int)(x1 + 0.5f);
	int maxY = (int)(y1 + 0.5f);
	if (minX > maxX) {
		minX = (int)(x1 + 0.5f);
		maxX = (int)(x0 + 0.5f);
	}
	if (minY > maxY) {
		minY = (int)(y1 + 0.5f);
		maxY = (int)(y0 + 0.5f);
	}
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	minX = ClampInt(minX, 0, w - 1);
	maxX = ClampInt(maxX, 0, w - 1);
	minY = ClampInt(minY, 0, h - 1);
	maxY = ClampInt(maxY, 0, h - 1);
	for (int yp = minY; yp <= maxY; ++yp) {
		uint32_t *pixel = backBuffer->pixels + yp * backBuffer->width + minX;
		for (int xp = minX; xp <= maxX; ++xp) {
			*pixel++ = color;
		}
	}
}

int main(int argc, char **args) {
	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char **files = fileCount > 0 ? args + 1 : fpl_null;
	const bool generateSineWave = fileCount == 0;

	AudioSystem audioSys = fplZeroInit;
	AudioDemo demo = fplZeroInit;
	demo.audioSys = &audioSys;
	demo.sineWave.toneHz = 256;
	demo.sineWave.toneVolume = 0.5f;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo | Audio", settings.window.title, fplArrayCount(settings.window.title));

	settings.video.driver = fplVideoDriverType_Software;
	settings.video.isAutoSize = true;
	settings.video.isVSync = true;

	// Set audio device format
	settings.audio.targetFormat.type = fplAudioFormatType_S16;
	settings.audio.targetFormat.channels = 2;
	//settings.audio.deviceFormat.sampleRate = 11025;
	//settings.audio.deviceFormat.sampleRate = 22050;
	settings.audio.targetFormat.sampleRate = 44100 / 10;
	//settings.audio.deviceFormat.sampleRate = 48000;

	// Disable start/stop of audio playback
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	// Find audio device
	if (!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		return -1;
	}
	fplAudioDeviceInfo audioDeviceInfos[64] = fplZeroInit;
	uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, fplArrayCount(audioDeviceInfos));
	if (deviceCount > 0) {
		// Use first audio device in settings
		settings.audio.targetDevice = audioDeviceInfos[0];
		// @TODO(final): Fix weird space after line break
		fplConsoleFormatOut("Using audio device: '%s'\n", settings.audio.targetDevice.name);
	}
	fplPlatformRelease();

	// Initialize the platform with audio enabled and the settings
	if (!fplPlatformInit(fplInitFlags_All, &settings)) {
		return -1;
	}

	fplAudioDeviceFormat targetAudioFormat = fplZeroInit;
	fplGetAudioHardwareFormat(&targetAudioFormat);

	// You can overwrite the client read callback and user data if you want to
	fplSetAudioClientReadCallback(AudioPlayback, &demo);

	const fplSettings *currentSettings = fplGetCurrentSettings();

	uint32_t framesPerSeconds = 1000 / 60;
	uint32_t audioDelay = 10;
	uint32_t audioDurationInMs = framesPerSeconds + audioDelay;
	uint32_t audioFrameCount = fplGetAudioBufferSizeInFrames(targetAudioFormat.sampleRate, audioDurationInMs);

	void *tempAudioSamples = fplMemoryAllocate(targetAudioFormat.bufferSizeInBytes);

	// Init audio data
	if (InitAudioData(&targetAudioFormat, &audioSys, files, fileCount, generateSineWave, &demo.sineWave)) {
		// Allocate render samples
		demo.maxFrameCount = targetAudioFormat.bufferSizeInFrames * 2;
		uint32_t channelCount = audioSys.targetFormat.channels;
		demo.sampleFormat = fplAudioFormatType_F32;
		demo.maxSizeBytes = fplGetAudioBufferSizeInBytes(demo.sampleFormat, channelCount, demo.maxFrameCount);
		demo.samples = fplMemoryAllocate(demo.maxSizeBytes);
		demo.periods = fplMemoryAllocate(demo.maxFrameCount * sizeof(int32_t));
		demo.readFrameIndex = 0;
		demo.writeFrameIndex = 0;

		// Start audio playback (This will start calling clientReadCallback regulary)
		if (fplPlayAudio() == fplAudioResult_Success) {
			// Print output infos
			const char *outDriver = fplGetAudioDriverString(currentSettings->audio.driver);
			const char *outFormat = fplGetAudioFormatString(audioSys.targetFormat.type);
			uint32_t outSampleRate = audioSys.targetFormat.sampleRate;
			uint32_t outChannels = audioSys.targetFormat.channels;
			fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n", audioSys.playItems.count, outDriver, outFormat, outSampleRate, outChannels);

			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();

			// Loop
			bool nextSamples = false;
			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					if (ev.type == fplEventType_Keyboard) {
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release && ev.keyboard.mappedKey == fplKey_Space) {
								nextSamples = true;
							}
						}
					}
				}

				// Audio
				if (nextSamples) {
#if !OPT_AUTO_RENDER_SAMPLES
					AudioFrameCount framesToRender = targetAudioFormat.bufferSizeInFrames / targetAudioFormat.periods;
					// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
					AudioFrameCount writtenFrames = AudioSystemWriteSamples(&audioSys, tempAudioSamples, &targetAudioFormat, framesToRender);
					AudioRenderSamples(&demo, tempAudioSamples, &targetAudioFormat, writtenFrames);
#endif
					nextSamples = false;
				}

				// Render
				const float w = (float)backBuffer->width;
				const float h = (float)backBuffer->height;
				FillBackRect(backBuffer, 0, 0, w, h, 0xFF000000);

				const AudioFrameCount frameCount = demo.maxFrameCount;
				const uint32_t maxBarCount = frameCount;
				const float paddingX = 0.0f;
				const float spaceBetweenBars = 0.0f;

				const float maxRangeW = w - paddingX * 2.0f;
				const float barW = (maxRangeW - spaceBetweenBars * (maxBarCount - 1)) / maxBarCount;
				const float maxBarH = h;

				for (AudioFrameCount frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
					AudioSampleCount sampleIndex = frameIndex * channelCount;
					int32_t period = demo.periods[frameIndex];
					float sampleValue = demo.samples[sampleIndex + 0];
					float t = frameIndex / (float)frameCount;
					uint32_t barIndex = (uint32_t)(t * maxBarCount + 0.5f);
					float barH = maxBarH * sampleValue;
					float posX = paddingX + barIndex * barW + barIndex * spaceBetweenBars;
					float posY = h * 0.5f - barH * 0.5f;
					int color = period % 2 == 0 ? 0xFFFF0000 : 0xFF00FF00;
					FillBackRect(backBuffer, posX, posY, posX + barW, posY + barH, color);
				}

				fplVideoFlip();
			}

			// Stop audio playback
			fplStopAudio();
		}
		// Release audio data
		AudioSystemShutdown(&audioSys);
	}

	fplMemoryFree(tempAudioSamples);
	fplMemoryFree(demo.periods);
	fplMemoryFree(demo.samples);

	// Release the platform
	fplPlatformRelease();

	return 0;
}