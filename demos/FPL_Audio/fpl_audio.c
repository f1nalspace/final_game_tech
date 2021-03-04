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
	## 2021-03-04
	- Dynamic streaming delay based on latency and buffer percentage

	## 2021-03-02
	- Removed all broken sample rendering
	- Introduced audio buffering in preparation for real FFT

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
#define OPT_PLAYBACK_STREAMBUFFER_ONLY 3

#define OPT_PLAYBACKMODE OPT_PLAYBACK_STREAMBUFFER_ONLY

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

#define FINAL_BUFFER_IMPLEMENTATION
#include <final_buffer.h>

typedef struct AudioDemo {
	AudioSystem audioSys;
	AudioSineWaveData sineWave;
	AudioBuffer streamTempBuffer;
	LockFreeRingBuffer streamRingBuffer;
	fplThreadHandle *streamingThread;
	fplAudioDeviceFormat targetAudioFormat;
	volatile uint32_t maxPlaybackFrameLatency;
	volatile fpl_b32 isStreamingThreadStopped;
} AudioDemo;



static void UpdateTitle(AudioDemo *demo) {
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

static void RenderQuad(const float x0, const float y0, const float x1, const float y1, const Vec4f color) {
	glColor4fv(&color.m[0]);
	glBegin(GL_QUADS);
	glVertex2f(x1, y0);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glEnd();
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

static void Render(AudioDemo *demo, const int screenW, const int screenH) {
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

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
	float bufferScale = areaDim.w / (float)demo->streamRingBuffer.length;

	uint64_t fillCount = fplAtomicLoadS64(&demo->streamRingBuffer.fillCount);

	uint64_t remaining = demo->streamRingBuffer.length - fillCount;

	float tailPos = areaPos.x + (float)demo->streamRingBuffer.tail * bufferScale;
	float headPos = areaPos.x + (float)demo->streamRingBuffer.head * bufferScale;

	bool tailWouldWrap = (demo->streamRingBuffer.tail + fillCount) > demo->streamRingBuffer.length;
	if(tailWouldWrap) {
		// Double
		RenderQuad(tailPos, areaPos.y, areaPos.x + areaDim.w, areaPos.y + areaDim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
		uint64_t wrapPos = (demo->streamRingBuffer.tail + fillCount) % demo->streamRingBuffer.length;
		float fillEnd = wrapPos * bufferScale;
		RenderQuad(areaPos.x, areaPos.y, areaPos.x + fillEnd, areaPos.y + areaDim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	} else {
		// Single
		float fillOffset = fillCount * bufferScale;
		RenderQuad(tailPos, areaPos.y, tailPos + fillOffset, areaPos.y + areaDim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	}

	RenderLine(headPos, areaPos.y - areaDim.h * 0.5f, headPos, areaPos.y + areaDim.h * 1.5f, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
	RenderLine(tailPos, areaPos.y - areaDim.h * 0.5f, tailPos, areaPos.y + areaDim.h * 1.5f, V4fInit(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);
#endif
}

static uint32_t AudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	//fplDebugFormatOut("Requested %lu frames\n", maxFrameCount);

	AudioDemo *demo = (AudioDemo *)userData;

	AudioFrameIndex result = 0;

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_SINEWAVE_ONLY
	result = maxFrameCount;
	AudioGenerateSineWave(&demo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
#elif OPT_PLAYBACKMODE == OPT_PLAYBACK_AUDIOSYSTEM_ONLY
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	result = AudioSystemWriteFrames(&demo->audioSys, outputSamples, outFormat, maxFrameCount);
#elif OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
	uint32_t currentPlaybackLatency = fplAtomicLoadU32(&demo->maxPlaybackFrameLatency);
	fplAtomicExchangeU32(&demo->maxPlaybackFrameLatency, fplMax(currentPlaybackLatency, maxFrameCount));

	size_t frameSize = fplGetAudioFrameSizeInBytes(outFormat->type, outFormat->channels);

	size_t availableBytes = 0;
	bool hasData = LockFreeRingBufferCanRead(&demo->streamRingBuffer, &availableBytes);
	if(hasData && (availableBytes % frameSize) == 0) {
		AudioFrameIndex availableFrames = (AudioFrameIndex)fplMax(0, availableBytes / frameSize);

		// Enable this assert to fire when the decoding is too slow
		//fplAssert(availableFrames >= maxFrameCount);

		AudioFrameIndex framesToCopy = fplMin(maxFrameCount, availableFrames);

		size_t totalCopySize = framesToCopy * frameSize;
		fplAssert((totalCopySize % frameSize) == 0);

		bool isRead = LockFreeRingBufferRead(&demo->streamRingBuffer, outputSamples, totalCopySize);
		fplAssert(isRead);

		result = framesToCopy;
	}

#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY

	return(result);
}

static bool StreamAudio(const fplAudioDeviceFormat *format, const uint32_t maxFrameCount, AudioDemo *demo, uint64_t *outDuration) {
	if(format == fpl_null || demo == fpl_null) return(false);
	if(maxFrameCount == 0) return(false);

	LockFreeRingBuffer *circularBuffer = &demo->streamRingBuffer;

	AudioBuffer *tmpBuffer = &demo->streamTempBuffer;

	size_t frameSize = fplGetAudioFrameSizeInBytes(format->type, format->channels);

	size_t availableSpace = 0;
	bool canWrite = LockFreeRingBufferCanWrite(circularBuffer, &availableSpace);

	if(canWrite && (availableSpace % frameSize) == 0) {
		uint64_t timeStart = fplGetTimeInMillisecondsLP();

		AudioFrameIndex numOfAvailableFrames = (AudioFrameIndex)fplMax(0, availableSpace / frameSize);

		// Disable this condition to always 100% fill the buffer
		//if(numOfAvailableFrames < maxFrameCount)
		//	return(false);

		AudioFrameIndex framesToWrite = fplMin(maxFrameCount, numOfAvailableFrames);

		size_t totalFrameBytes = framesToWrite * frameSize;
		fplAssert(totalFrameBytes <= tmpBuffer->bufferSize);

		// The amount of actual written frames may be less than the frames we want to get written
		AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, tmpBuffer->samples, format, framesToWrite);
		fplAssert(writtenFrames == framesToWrite);
		totalFrameBytes = writtenFrames * frameSize;

		bool written = LockFreeRingBufferWrite(circularBuffer, tmpBuffer->samples, totalFrameBytes);
		fplAssert(written);

		if(outDuration != fpl_null) {
			*outDuration = fplGetTimeInMillisecondsLP() - timeStart;
		}

		return(true);
	}

	return(false);
}

static void AudioStreamingThread(const fplThreadHandle *thread, void *rawData) {
	AudioDemo *demo = (AudioDemo *)rawData;

	AudioFrameIndex framesToStream = 256 * 60;

	AudioMilliseconds maxDelay = fplGetAudioBufferSizeInMilliseconds(demo->targetAudioFormat.sampleRate, framesToStream);
	maxDelay = (maxDelay / 100 * 100);

	AudioMilliseconds currentDelay = maxDelay;

	LockFreeRingBuffer *circularBuffer = &demo->streamRingBuffer;

	float minBufferPercentage = 0.25f;
	float maxBufferPercentage = 0.75f;

	size_t frameSize = fplGetAudioFrameSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels);

	uint64_t startTime = fplGetTimeInMillisecondsLP();
	while(!demo->isStreamingThreadStopped) {
		// Wait if needed
		uint64_t deltaTime = fplGetTimeInMillisecondsLP() - startTime;
		if(deltaTime < currentDelay) {
			fplThreadSleep(1);
			continue;
		}
		startTime = fplGetTimeInMillisecondsLP();

		// Stream in audio to a ring buffer
		uint64_t streamDuration = 0;
		if(StreamAudio(&demo->targetAudioFormat, framesToStream, demo, &streamDuration)) {
			if(streamDuration > currentDelay) {
				// @TODO(final): We are taking too long to stream, do we want to adjust the delay (slow platform/device)
			}
		}

		// Get playback latency (Max of frames the audio card has requested so far)
		AudioFrameIndex latencyInFrames = fplAtomicLoadU32(&demo->maxPlaybackFrameLatency);
		AudioMilliseconds latencyInMs = fplGetAudioBufferSizeInMilliseconds(demo->targetAudioFormat.sampleRate, latencyInFrames);

		// Minimum delay is either latency in ms or 10 ms
		AudioMilliseconds minDelay = fplMax(10, latencyInMs); 

		// Adjust delay when we are playing back too fast (buffering too slow, buffer too small -> results in lower wait delay)
		// Adjust delay when we are stream too fast (buffering too fast -> good case -> results in higher wait delay)
		uint64_t fillCount = fplAtomicLoadS64(&circularBuffer->fillCount);
		AudioFrameIndex filledFrames = (AudioFrameIndex)(fillCount / frameSize);
		float percentageFilled = (1.0f / (float)circularBuffer->length) * (float)fillCount;
		if(percentageFilled < minBufferPercentage) {
			float x = 1.0f / minBufferPercentage * percentageFilled;
			currentDelay = fplMax(minDelay, fplMin((uint32_t)(maxDelay * x), maxDelay));
		} else if(percentageFilled > maxBufferPercentage) {
			currentDelay = (uint32_t)((float)currentDelay * 1.25f);
		}
	}
}

int main(int argc, char **args) {
	//LockFreeRingBufferUnitTest();

	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char **files = (fileCount > 0) ? (const char **)args + 1 : fpl_null;
	bool forceSineWave = false;

	// Always sine-wave
	//fileCount = 0;
	//forceSineWave = true;

	AudioDemo *demo = (AudioDemo *)fplMemoryAllocate(sizeof(AudioDemo));
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
	fplAudioDeviceInfo *audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * maxAudioDeviceCount);
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

	const fplSettings *currentSettings = fplGetCurrentSettings();

	// Init audio data
	if(InitAudioData(&demo->targetAudioFormat, &demo->audioSys, files, fileCount, forceSineWave, &demo->sineWave)) {
		demo->maxPlaybackFrameLatency = demo->targetAudioFormat.bufferSizeInFrames / demo->targetAudioFormat.periods;

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
		AudioFrameIndex streamBufferFrames = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 5000);

		// Init streaming buffer and read some frames at the very start
		size_t streamBufferSize = fplGetAudioBufferSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels, streamBufferFrames);
		bool bufferInitRes = LockFreeRingBufferInit(&demo->streamRingBuffer, streamBufferSize, true);
		if(!bufferInitRes) {
			goto done;
		}

		// Allocate temporary audio buffer
		AudioFormat streamTempBufferFormat = fplZeroInit;
		streamTempBufferFormat.channels = demo->targetAudioFormat.channels;
		streamTempBufferFormat.format = demo->targetAudioFormat.type;
		streamTempBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
		bufferInitRes = AllocateAudioBuffer(&demo->audioSys, &demo->streamTempBuffer, &streamTempBufferFormat, streamBufferFrames);
		if(!bufferInitRes) {
			goto done;
		}

		// Stream in initial frames
		AudioFrameIndex initialFrameStreamCount = streamBufferFrames / 4;
		fplAssert(streamBufferFrames >= initialFrameStreamCount);
		StreamAudio(&demo->targetAudioFormat, initialFrameStreamCount, demo, fpl_null);

		// Start streaming thread
		demo->streamingThread = fplThreadCreate(AudioStreamingThread, demo);
#endif

		// Start audio playback (This will start calling clientReadCallback regulary)
		if(fplPlayAudio() == fplAudioResultType_Success) {
			// Print output infos
			const char *outDriver = fplGetAudioDriverString(currentSettings->audio.driver);
			const char *outFormat = fplGetAudioFormatTypeString(demo->audioSys.targetFormat.format);
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
#if OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
	// Wait for decoding thread to stop
	demo->isStreamingThreadStopped = 1;
	fplThreadWaitForOne(demo->streamingThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(demo->streamingThread);

	// Release audio buffers
	FreeAudioBuffer(&demo->audioSys, &demo->streamTempBuffer);
	LockFreeRingBufferRelease(&demo->streamRingBuffer);
#endif

	// Release audio system
	AudioSystemShutdown(&demo->audioSys);

	fglUnloadOpenGL();

	fplPlatformRelease();

	if(demo)
		fplMemoryFree(demo);

	return(result);
}