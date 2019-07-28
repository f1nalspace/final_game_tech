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
	- Wave form visualization (Bars, Lines)
	- Moved sine wave generation out into final_audiosystem.h
	- Reflect api changes for FPL 0.9.4

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

#define OPT_PLAYBACKMODE OPT_PLAYBACK_AUDIOSYSTEM_ONLY

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define _USE_MATH_DEFINES
#include <math.h> // sinf, M_PI

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#define FINAL_GRAPHICS_IMPLEMENTATION
#include <final_graphics.h>

typedef enum WavePlotType {
	WavePlotType_None = 0,
	WavePlotType_Bars,
	WavePlotType_Lines,
	WavePlotType_Count,
} WavePlotType;

typedef struct AudioDemo {
	AudioSystem *audioSys;
	fplAudioFormatType sampleFormat;
	AudioFrameIndex maxFrameCount;
	size_t maxSizeBytes;
	AudioFrameIndex writeFrameIndex;
	AudioFrameIndex readFrameIndex;
	float *samples;
	AudioSineWaveData sineWave;
	WavePlotType plotType;
	int32_t plotCount;
} AudioDemo;

static void AudioRenderSamples(AudioDemo *audioDemo, const void *inSamples, const fplAudioDeviceFormat *inFormat, const AudioFrameIndex frameCount) {
	AudioSystem *audioSys = audioDemo->audioSys;

	const AudioChannelIndex outChannels = audioSys->targetFormat.channels;
	const size_t inSampleSize = fplGetAudioSampleSizeInBytes(inFormat->type);

	AudioFrameIndex inputFrameIndex = 0;
	AudioFrameIndex remainingFrameCount = frameCount;
	fplAssert(frameCount <= audioDemo->maxFrameCount);
	while (remainingFrameCount > 0) {
		AudioFrameIndex remainingFramesInDemo = fplMax(0, (int)(audioDemo->maxFrameCount - audioDemo->writeFrameIndex));
		AudioFrameIndex framesToCopy = remainingFrameCount;
		if (framesToCopy >= remainingFramesInDemo) {
			framesToCopy = remainingFramesInDemo;
		}
		AudioSampleIndex outputSampleIndex = audioDemo->writeFrameIndex * outChannels;
		AudioSampleIndex inputSampleIndex = inputFrameIndex * inFormat->channels;
		for (AudioFrameIndex frameIndex = 0; frameIndex < framesToCopy; ++frameIndex) {
			for (AudioChannelIndex channelIndex = 0; channelIndex < inFormat->channels; ++channelIndex) {
				audioDemo->samples[outputSampleIndex++] = ConvertToF32((uint8_t *)inSamples + inputSampleIndex * inSampleSize, channelIndex, inFormat->type);
			}
			inputSampleIndex += inFormat->channels;
		}

		inputFrameIndex += framesToCopy;
		remainingFrameCount -= framesToCopy;
		audioDemo->writeFrameIndex = (audioDemo->writeFrameIndex + framesToCopy) % audioDemo->maxFrameCount;
	}
}


static uint32_t AudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	AudioDemo *audioDemo = (AudioDemo *)userData;

	AudioFrameIndex result = 0;

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_SINEWAVE_ONLY

#if 0
	// 100% Clean sine wave (Contigously)
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


#if OPT_PLAYBACKMODE == OPT_PLAYBACK_AUDIOSYSTEM_ONLY
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	AudioFrameIndex frameCount = AudioSystemWriteSamples(audioDemo->audioSys, outputSamples, outFormat, maxFrameCount);
	result = frameCount;
#endif // OPT_OUTPUT_SAMPLES_SYSTEM_ONLY

#if OPT_AUTO_RENDER_SAMPLES
	AudioRenderSamples(audioDemo, outputSamples, outFormat, result);
#endif

	return(result);
}

static bool InitAudioData(const fplAudioDeviceFormat *targetFormat, AudioSystem *audioSys, const char **files, const size_t fileCount, const bool forceSineWave, const AudioSineWaveData *sineWave) {
	if (!AudioSystemInit(audioSys, targetFormat)) {
		return false;
	}

	AudioSystemSetMasterVolume(audioSys, 0.5f);

	// Play audio files
	bool hadFiles = false;
	for (size_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		const char *filePath = files[fileIndex];
		if (filePath != fpl_null) {
			fplConsoleFormatOut("Loading audio file '%s\n", filePath);
			AudioSource *source = AudioSystemLoadFileSource(audioSys, filePath);
			if (source != fpl_null) {
				AudioSystemPlaySource(audioSys, source, true, 1.0f);
				hadFiles = true;
			}
		}
	}

	// Generate sine wave for some duration when no files was loaded
	if (!hadFiles || forceSineWave) {
		// @FIXME(final): If wave duration is smaller than actual audio buffer, we will hear a bad click
		const double waveDuration = 10.0f;
		AudioSineWaveData waveData = *sineWave;
		AudioHertz sampleRate = audioSys->targetFormat.sampleRate;
		AudioChannelIndex channels = audioSys->targetFormat.channels;
		AudioFrameIndex frameCount = (AudioFrameIndex)(sampleRate * waveDuration + 0.5);
		AudioSource *source = AudioSystemAllocateSource(audioSys, audioSys->targetFormat.channels, audioSys->targetFormat.sampleRate, fplAudioFormatType_S16, frameCount);
		if (source != fpl_null) {
			AudioGenerateSineWave(&waveData, source->buffer.samples, source->format.format, source->format.sampleRate, source->format.channels, source->buffer.frameCount);
			AudioSystemPlaySource(audioSys, source, true, 1.0f);
		}
	}
	return(true);
}

static const char *MapPlotTypeToString(WavePlotType plotType) {
	switch (plotType) {
		case WavePlotType_Bars:
			return "Bars";
		case WavePlotType_Lines:
			return "Lines";
		default:
			return "None";
	}
}

static void UpdateTitle(AudioDemo *demo) {
	char titleBuffer[256];
	fplFormatString(titleBuffer, fplArrayCount(titleBuffer), "FPL Demo | Audio [Plot: %s, Points: %d]", MapPlotTypeToString(demo->plotType), demo->plotCount);
	fplSetWindowTitle(titleBuffer);
}

int main(int argc, char **args) {
	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char **files = fileCount > 0 ? args + 1 : fpl_null;
	const bool forceSineWave = false;

	AudioSystem audioSys = fplZeroInit;
	AudioDemo demo = fplZeroInit;
	demo.audioSys = &audioSys;
	demo.sineWave.toneHz = 256;
	demo.sineWave.toneVolume = 0.5f;
	demo.plotType = WavePlotType_Bars;
	demo.plotCount = 512;

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
	settings.audio.targetFormat.sampleRate = 44100;
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

	// Init audio data
	if (InitAudioData(&targetAudioFormat, &audioSys, files, fileCount, forceSineWave, &demo.sineWave)) {
		// Allocate render samples
		demo.maxFrameCount = targetAudioFormat.bufferSizeInFrames * 2;
		uint32_t channelCount = audioSys.targetFormat.channels;
		demo.sampleFormat = fplAudioFormatType_F32;
		demo.maxSizeBytes = fplGetAudioBufferSizeInBytes(demo.sampleFormat, channelCount, demo.maxFrameCount);
		demo.samples = fplMemoryAllocate(demo.maxSizeBytes);
		demo.readFrameIndex = 0;
		demo.writeFrameIndex = 0;

		// Start audio playback (This will start calling clientReadCallback regulary)
		if (fplPlayAudio() == fplAudioResult_Success) {
			// Print output infos
			const char *outDriver = fplGetAudioDriverString(currentSettings->audio.driver);
			const char *outFormat = fplGetAudioFormatString(audioSys.targetFormat.format);
			uint32_t outSampleRate = audioSys.targetFormat.sampleRate;
			uint32_t outChannels = audioSys.targetFormat.channels;
			fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n", audioSys.playItems.count, outDriver, outFormat, outSampleRate, outChannels);

			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();

			UpdateTitle(&demo);

			// Loop
			bool nextSamples = false;
			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					if (ev.type == fplEventType_Keyboard) {
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release) {
								fplKey key = ev.keyboard.mappedKey;
								if (key == fplKey_Space) {
									nextSamples = true;
								} else if (key == fplKey_P) {
									demo.plotType = (demo.plotType + 1) % WavePlotType_Count;
								} else if (key == fplKey_Add) {
									demo.plotCount *= 2;
									if (demo.plotCount > 2048) {
										demo.plotCount = 2048;
									}
								} else if (key == fplKey_Substract) {
									demo.plotCount /= 2;
									if (demo.plotCount < 8) {
										demo.plotCount = 8;
									}
								}
								UpdateTitle(&demo);
							}
						}
					}
				}

				// Audio
				if (nextSamples) {
#if !OPT_AUTO_RENDER_SAMPLES
					AudioFrameIndex framesToRender = targetAudioFormat.bufferSizeInFrames / targetAudioFormat.periods;
					// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
					AudioFrameIndex writtenFrames = AudioSystemWriteSamples(&audioSys, tempAudioSamples, &targetAudioFormat, framesToRender);
					AudioRenderSamples(&demo, tempAudioSamples, &targetAudioFormat, writtenFrames);
#endif
					nextSamples = false;
				}

				// Render
				const float w = (float)backBuffer->width;
				const float h = (float)backBuffer->height;
				BackbufferDrawRect(backBuffer, 0, 0, w, h, 0xFF000000);

				const AudioFrameIndex frameCount = demo.maxFrameCount;

				const float paddingX = 20.0f;
				const float spaceBetweenBars = 0.0f;
				const float maxRangeW = w - paddingX * 2.0f;
				const float halfH = h * 0.5f;

				if (demo.plotType == WavePlotType_Bars) {
					const uint32_t barCount = demo.plotCount;
					const float barW = (maxRangeW - spaceBetweenBars * (float)(barCount - 1)) / (float)barCount;
					const float maxBarH = h;
					for (uint32_t barIndex = 0; barIndex < barCount; ++barIndex) {
						float t = barIndex / (float)barCount;
						AudioFrameIndex frameIndex = (AudioFrameIndex)(t * frameCount + 0.5f);
						AudioSampleIndex sampleIndex = frameIndex * channelCount;
						float sampleValue = demo.samples[sampleIndex + 0];
						float barH = maxBarH * sampleValue;
						float posX = paddingX + barIndex * barW + barIndex * spaceBetweenBars;
						float posY = halfH - barH * 0.5f;
						int color = 0xFFAAAAAA;
						BackbufferDrawRect(backBuffer, posX, posY, posX + barW, posY + barH, color);
					}
				} else if (demo.plotType == WavePlotType_Lines) {
					const uint32_t pointCount = demo.plotCount;
					const float lineWidth = maxRangeW / (float)(pointCount - 1);

					float firstSampleValue = demo.samples[0 * channelCount + 0];
					float startX = paddingX;
					float startY = halfH + (firstSampleValue * halfH);
					for (uint32_t pointIndex = 1; pointIndex < pointCount; ++pointIndex) {
						float t = pointIndex / (float)pointCount;
						AudioFrameIndex frameIndex = (AudioFrameIndex)(t * (float)frameCount + 0.5f);
						AudioSampleIndex sampleIndex = frameIndex * channelCount;
						float sampleValue = demo.samples[sampleIndex + 0];
						float targetX = startX + lineWidth;
						float targetY = halfH + (sampleValue * halfH);
						BackbufferDrawLine(backBuffer, startX, startY, targetX, targetY, 0xFFAAAAAA);
						startX = targetX;
						startY = targetY;
					}
				}

				fplVideoFlip();
			}

			// Stop audio playback
			fplStopAudio();
		}
		// Release audio data
		AudioSystemShutdown(&audioSys);
	}

	fplMemoryFree(demo.samples);

	// Release the platform
	fplPlatformRelease();

	return 0;
}