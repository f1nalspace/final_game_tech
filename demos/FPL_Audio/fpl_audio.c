/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio

Description:
	This demo shows how to play music, sounds using a custom mixer.
	It supports uncompressed PCM wave data, OGG Vorbis and MP3 Files.
	Resampling support is limited.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2020-02-15
	- Basic FFT & spectrum rendering

	## 2019-10-31
	- Moved audio load code to the outside
	- Fixed release resources when exit

	## 2019-08-03
	- Print error when audio playback its too slow

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

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/
#define OPT_PLAYBACK_NONE 0
#define OPT_PLAYBACK_SINEWAVE_ONLY 1
#define OPT_PLAYBACK_AUDIOSYSTEM_ONLY 2
#define OPT_PLAYBACK_DECODEBUFFER_ONLY 3

#define OPT_PLAYBACKMODE OPT_PLAYBACK_DECODEBUFFER_ONLY

#define FPL_LOGGING
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define _USE_MATH_DEFINES
#include <math.h> // sinf, M_PI
#include <float.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <final_audiodemo.h>

#include <final_math.h>

#include <final_utils.h>

#include <circularbuffer/TPCircularBuffer.h>

typedef struct AudioDemo {
	AudioSystem audioSys;
	AudioSineWaveData sineWave;
	AudioBuffer tempBuffer;
	TPCircularBuffer decodeBuffer;
	fplThreadHandle* decodeThread;
	fplAudioDeviceFormat targetAudioFormat;
	volatile fpl_b32 isDecodeThreadStopped;
} AudioDemo;

static uint32_t AudioPlayback(const fplAudioDeviceFormat* outFormat, const uint32_t maxFrameCount, void* outputSamples, void* userData) {
	fplDebugFormatOut("Requested %lu frames\n", maxFrameCount);

	AudioDemo* audioDemo = (AudioDemo*)userData;

	AudioFrameIndex result = 0;

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_SINEWAVE_ONLY
	result = maxFrameCount;
	AudioGenerateSineWave(&audioDemo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
#elif OPT_PLAYBACKMODE == OPT_PLAYBACK_AUDIOSYSTEM_ONLY
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	result = AudioSystemWriteFrames(&audioDemo->audioSys, outputSamples, outFormat, maxFrameCount);
#elif OPT_PLAYBACKMODE == OPT_PLAYBACK_DECODEBUFFER_ONLY

	uint32_t frameSize = fplGetAudioFrameSizeInBytes(outFormat->type, outFormat->channels);

	uint32_t availableBytes = 0;
	TPCircularBufferData tail = TPCircularBufferTail(&audioDemo->decodeBuffer, &availableBytes);
	if((availableBytes % frameSize) == 0) {
		uint32_t availableFrames = fplMax(0, availableBytes / frameSize);
		fplAssert(availableFrames >= maxFrameCount);

		uint32_t framesToCopy = maxFrameCount;

		uint32_t totalCopySize = framesToCopy * frameSize;
		fplAssert((totalCopySize % frameSize) == 0);

		TPCircularBufferRead(&audioDemo->decodeBuffer, outputSamples, totalCopySize);

		result = framesToCopy;
	}

#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY

	return(result);
}

static bool DecodeAudio(const uint32_t frameCount, const fplAudioDeviceFormat* format, AudioDemo* demo) {
	if(frameCount == 0) return(false);

	TPCircularBuffer* circularBuffer = &demo->decodeBuffer;

	uint32_t availableSpace = 0;
	TPCircularBufferData bufferHead = TPCircularBufferHead(circularBuffer, &availableSpace);

	AudioBuffer* tmpBuffer = &demo->tempBuffer;

	if(availableSpace > 0) {
		uint32_t frameSize = fplGetAudioFrameSizeInBytes(format->type, format->channels);

		uint32_t numOfAvailableFrames = fplMax(0, availableSpace / frameSize);

		uint32_t framesToWrite = fplMin(numOfAvailableFrames, frameCount);

		uint32_t totalFrameBytes = framesToWrite * frameSize;
		fplAssert(totalFrameBytes <= tmpBuffer->bufferSize);

		uint32_t writtenFrames = AudioSystemWriteFrames(&demo->audioSys, tmpBuffer->samples, format, framesToWrite);

		// The amount of actual written frames may be less than the frames we want to get written
		totalFrameBytes = writtenFrames * frameSize;

		bool written = TPCircularBufferWrite(circularBuffer, tmpBuffer->samples, totalFrameBytes);
		assert(written);

		return(true);
	}

	return(false);
}

static void AudioDecodeThread(const fplThreadHandle* thread, void* rawData) {
	AudioDemo* demo = (AudioDemo*)rawData;

	uint64_t pollDelay = 10;

	AudioFrameIndex framesToDecode = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 100);

	uint64_t startTime = fplGetTimeInMillisecondsLP();
	while(!demo->isDecodeThreadStopped) {
		uint64_t deltaTime = fplGetTimeInMillisecondsLP() - startTime;
		if(deltaTime >= pollDelay) {
			startTime = fplGetTimeInMillisecondsLP();
			DecodeAudio(1024, &demo->targetAudioFormat, demo);
		} else {
			fplThreadSleep(10);
		}
	}
}

static void UpdateTitle(AudioDemo* demo) {
	char titleBuffer[256];
	fplFormatString(titleBuffer, fplArrayCount(titleBuffer), "FPL Demo | Audio");
	fplSetWindowTitle(titleBuffer);
}

static void RenderRectangle(const float x0, const float y0, const float x1, const float y1, const Vec4f color, const float lineWidth) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y0);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glEnd();
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 1);
}

static void RenderLine(const float x0, const float y0, const float x1, const float y1, const Vec4f color, const float lineWidth) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINES);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glEnd();
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 1);
}

static void Render(AudioDemo* demo, const int screenW, const int screenH) {
	uint32_t channelCount = demo->audioSys.targetFormat.channels;

	float w = (float)screenW;
	float h = (float)screenH;

	glViewport(0, 0, screenW, screenH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Vec2f areaDim = V2fInit(w * 0.9f, h * 0.25f);
	Vec2f areaPos = V2fInit((w - areaDim.w) * 0.5f, (h - areaDim.h) * 0.5f);

	RenderLine(0.0f, h * 0.5f, w, h * 0.5f, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
	RenderLine(w * 0.5f, 0, w * 0.5f, h, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);

	RenderRectangle(areaPos.x, areaPos.y, areaPos.x + areaDim.w, areaPos.y + areaDim.h, (Vec4f) { 1, 0, 0, 1 }, 1.0f);
}

int main(int argc, char** args) {
	TPCircularBufferUnitTest();




	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char** files = (fileCount > 0) ? (const char**)args + 1 : fpl_null;
	bool forceSineWave = false;

	// Always sine-wave
	//fileCount = 0;
	//forceSineWave = true;

	AudioDemo* demo = (AudioDemo*)fplMemoryAllocate(sizeof(AudioDemo));
	demo->sineWave.frequency = 440;
	demo->sineWave.toneVolume = 0.25f;
	demo->sineWave.duration = 0.5;

	int result = -1;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo | Audio", settings.window.title, fplArrayCount(settings.window.title));

	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	settings.video.isVSync = true;

	// Set audio device format
	settings.audio.targetFormat.type = fplAudioFormatType_S16;
	settings.audio.targetFormat.channels = 2;

	//settings.audio.targetFormat.sampleRate = 11025;
	//settings.audio.targetFormat.sampleRate = 22050;
	settings.audio.targetFormat.sampleRate = 44100;
	//settings.audio.targetFormat.bufferSizeInMilliseconds = 16;
	//settings.audio.targetFormat.sampleRate = 48000;

	// Disable start/stop of audio playback
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	// Find audio device
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		goto done;
	}

	const uint32_t targetFrameRateMs = (uint32_t)ceil(1000.0 / 60.0);

	const uint32_t maxAudioDeviceCount = 64;
	fplAudioDeviceInfo* audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * maxAudioDeviceCount);
	uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, maxAudioDeviceCount);
	if(deviceCount > 0) {
		// Use first audio device in settings
		settings.audio.targetDevice = audioDeviceInfos[0];
		// @TODO(final): Fix weird space after line break
		fplConsoleFormatOut("Using audio device: '%s'\n", settings.audio.targetDevice.name);
	}
	fplMemoryFree(audioDeviceInfos);
	fplPlatformRelease();

	// Initialize the platform with audio enabled and the settings
	if(!fplPlatformInit(fplInitFlags_All, &settings)) {
		goto done;
	}

	// Initialize OpenGL
	if(!fglLoadOpenGL(true))
		goto done;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH_HINT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fplGetAudioHardwareFormat(&demo->targetAudioFormat);

	// You can overwrite the client read callback and user data if you want to
	fplSetAudioClientReadCallback(AudioPlayback, demo);

	const fplSettings* currentSettings = fplGetCurrentSettings();

	AudioFrameIndex decodeBufferFrames = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 2000);

	// Init audio data
	if(InitAudioData(&demo->targetAudioFormat, decodeBufferFrames, &demo->audioSys, files, fileCount, forceSineWave, &demo->sineWave)) {
		// Init decode buffer and read some frames at the very start
		uint32_t decodeBufferSize = fplGetAudioBufferSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels, decodeBufferFrames);
		bool bufferInitRes = TPCircularBufferInit(&demo->decodeBuffer, decodeBufferSize, true);
		if(!bufferInitRes) {
			goto done;
		}

		// Allocate temporary audio buffer
		AudioFormat tempBufferFormat = fplZeroInit;
		tempBufferFormat.channels = demo->targetAudioFormat.channels;
		tempBufferFormat.format = demo->targetAudioFormat.type;
		tempBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
		bufferInitRes = AllocateAudioBuffer(&demo->audioSys, &demo->tempBuffer, &tempBufferFormat, decodeBufferFrames);
		if(!bufferInitRes) {
			goto done;
		}

		// Decode one buffer worth of frames at the very start
		AudioFrameIndex initialFrameDecodeCount = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 1000);
		fplAssert(decodeBufferFrames >= initialFrameDecodeCount);
		DecodeAudio(initialFrameDecodeCount, &demo->targetAudioFormat, demo);

		// Start decoding thread
		demo->decodeThread = fplThreadCreate(AudioDecodeThread, demo);

		// Start audio playback (This will start calling clientReadCallback regulary)
		if(fplPlayAudio() == fplAudioResultType_Success) {
			// Print output infos
			const char* outDriver = fplGetAudioDriverString(currentSettings->audio.driver);
			const char* outFormat = fplGetAudioFormatTypeString(demo->audioSys.targetFormat.format);
			uint32_t outSampleRate = demo->audioSys.targetFormat.sampleRate;
			uint32_t outChannels = demo->audioSys.targetFormat.channels;
			fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n",
				demo->audioSys.playItems.count,
				outDriver,
				outFormat,
				outSampleRate,
				outChannels);

			UpdateTitle(demo);

			// Loop
			double lastTime = fplGetTimeInSecondsHP();
			while(fplWindowUpdate()) {
				fplEvent ev;
				while(fplPollEvent(&ev)) {
					if(ev.type == fplEventType_Keyboard) {
						if(ev.keyboard.type == fplKeyboardEventType_Button) {
							if(ev.keyboard.buttonState == fplButtonState_Release) {
								fplKey key = ev.keyboard.mappedKey;

#if 0
								if(key == fplKey_P) {
									demo->plotType = (demo->plotType + 1) % WavePlotType_Count;
								} else if(key == fplKey_Add) {
									demo->plotCount *= 2;
									if(demo->plotCount > 2048) {
										demo->plotCount = 2048;
									}
								} else if(key == fplKey_Substract) {
									demo->plotCount /= 2;
									if(demo->plotCount < 8) {
										demo->plotCount = 8;
									}
								}
#endif

								UpdateTitle(demo);
							}
						}
					}
				}

				fplWindowSize winSize = fplZeroInit;
				fplGetWindowSize(&winSize);
				Render(demo, winSize.width, winSize.height);
				fplVideoFlip();

				double newTime = fplGetTimeInSecondsHP();
				double frameTime = newTime - lastTime;
				//AudioFrameIndex videoFrameCount = (AudioFrameIndex)(targetAudioFormat.sampleRate * frameTime);
				//demo->currentVideoFrameIndex = (demo->currentVideoFrameIndex + videoFrameCount) % demo->totalBufferFrameCount;
				lastTime = newTime;
			}

			// Stop audio playback
			fplStopAudio();
					}
				}

	result = 0;

done:
	// Wait for decoding thread to stop
	demo->isDecodeThreadStopped = 1;
	fplThreadWaitForOne(demo->decodeThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(demo->decodeThread);

	// Release audio buffers
	FreeAudioBuffer(&demo->audioSys, &demo->tempBuffer);
	TPCircularBufferCleanup(&demo->decodeBuffer);

	// Release audio system
	AudioSystemShutdown(&demo->audioSys);

	fglUnloadOpenGL();

	fplPlatformRelease();

	if(demo)
		fplMemoryFree(demo);

	return(result);
			}