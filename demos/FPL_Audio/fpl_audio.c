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


/*
Ideas:

Visualize the samples:

- Define a struct chunk which contains 128, 256, 512 samples + a frame counter, so we can convert it to a time
- Define a ringbuffer with a multiple of N-chunks with a total length of one second
- In the AudioStreamingThread convert the streamed samples to N-chunks and push it into the ring buffer
- In the rendering code we track the current time, regardless if audio is played or not or if window is visible or not
- Now for each frame, we look for one chunk which fits exactly in the current time, copy it and discard the chunk and all other chunks which are older (No need to keep chunks from previous frames)
- Use the copied chunk as FFT input and display the spectrum for it
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

#define MAX_AUDIO_FRAMES_CHUNK_FRAMES 128
typedef struct AudioFramesChunk {
	uint8_t samples[MAX_AUDIO_FRAMES_CHUNK_FRAMES * AUDIO_MAX_CHANNEL_COUNT * AUDIO_MAX_SAMPLESIZE];
	AudioFrameIndex index;
	AudioFrameIndex count;
} AudioFramesChunk;

#define MAX_AUDIO_BIN_COUNT 128

typedef struct AudioSpectrum {
	FFTDouble input[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	FFTDouble output[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double magnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double windowCoeffs[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double maxPeaks[MAX_AUDIO_BIN_COUNT];
	double bins[MAX_AUDIO_BIN_COUNT + 1];
} AudioSpectrum;

typedef struct AudioDemo {
	AudioSystem audioSys;
	AudioSineWaveData sineWave;
	AudioBuffer streamTempBuffer;
	LockFreeRingBuffer streamRingBuffer;
	LockFreeRingBuffer chunkRingBuffer;
	AudioFramesChunk videoAudioChunks[2]; // 0 = Render, 1 = New
	AudioSpectrum spectrum;
	void *chunkTempBuffer;
	fplThreadHandle *streamingThread;
	fplAudioDeviceFormat targetAudioFormat;
	uint64_t lastVideoAudioChunkUpdateTime;
	volatile uint32_t hasVideoAudioChunk;
	volatile uint32_t numFramesPlayed;
	volatile uint32_t numFramesStreamed;
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

static void RenderRingBuffer(const Vec2f pos, const Vec2f dim, LockFreeRingBuffer *buffer) {
	RenderRectangle(pos.x, pos.y, pos.x + dim.w, pos.y + dim.h, (Vec4f) { 1, 0, 0, 1 }, 1.0f);

	uint64_t bufferLen = buffer->length;

	float bufferScale = dim.w / (float)bufferLen;

	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);

	uint64_t remaining = bufferLen - fillCount;

	uint64_t tail = buffer->tail;
	uint64_t head = buffer->head;

	float tailPos = pos.x + (float)tail * bufferScale;
	float headPos = pos.x + (float)head * bufferScale;

	bool tailWouldWrap = (tail + fillCount) > bufferLen;
	if(tailWouldWrap) {
		// Double
		RenderQuad(tailPos, pos.y, pos.x + dim.w, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
		uint64_t wrapPos = (tail + fillCount) % bufferLen;
		float fillEnd = wrapPos * bufferScale;
		RenderQuad(pos.x, pos.y, pos.x + fillEnd, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	} else {
		// Single
		float fillOffset = fillCount * bufferScale;
		RenderQuad(tailPos, pos.y, tailPos + fillOffset, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	}

	RenderLine(headPos, pos.y - dim.h * 0.5f, headPos, pos.y + dim.h * 1.5f, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
	RenderLine(tailPos, pos.y - dim.h * 0.5f, tailPos, pos.y + dim.h * 1.5f, V4fInit(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);
}

static void Render(AudioDemo *demo, const int screenW, const int screenH, const double currentRenderTime) {
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

	RenderLine(0.0f, h * 0.5f, w, h * 0.5f, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
	RenderLine(w * 0.5f, 0, w * 0.5f, h, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
	Vec2f streamBufferDim = V2fInit(w * 0.9f, h * 0.1f);
	Vec2f streamBufferPos = V2fInit((w - streamBufferDim.w) * 0.5f, h * 0.1f);
	RenderRingBuffer(streamBufferPos, streamBufferDim, &demo->streamRingBuffer);

#if 0
	Vec2f chunkBufferDim = V2fInit(w * 0.9f, h * 0.1f);
	Vec2f chunkBufferPos = V2fInit((w - chunkBufferDim.w) * 0.5f, (h - chunkBufferDim.h) * 0.9f);
	RenderRingBuffer(chunkBufferPos, chunkBufferDim, &demo->chunkRingBuffer);
#endif

	Vec2f spectrumDim = V2fInit(w * 0.9f, h * 0.6f);
	Vec2f spectrumPos = V2fInit((w - spectrumDim.w) * 0.5f, h * 0.3f);
	RenderRectangle(spectrumPos.x, spectrumPos.y, spectrumPos.x + spectrumDim.w, spectrumPos.y + spectrumDim.h, (Vec4f) { 1, 1, 1, 1 }, 1.0f);

	if(fplIsAtomicCompareAndSwapU32(&demo->hasVideoAudioChunk, 2, 3)) {
		demo->videoAudioChunks[0] = demo->videoAudioChunks[1];
		fplAtomicExchangeU32(&demo->hasVideoAudioChunk, 0);
	}

	uint32_t frameCount = demo->videoAudioChunks[0].count;
	if(frameCount > 0) {
		//
		// Compute FFT
		//
		const uint32_t channel = 0;
		fplAudioFormatType format = demo->targetAudioFormat.type;
		size_t sampleSize = fplGetAudioSampleSizeInBytes(format);
		size_t frameSize = sampleSize * demo->targetAudioFormat.channels;
		for(uint32_t i = 0; i < frameCount; ++i) {
			double sampleValue = 0.0;
			switch(format) {
				case fplAudioFormatType_F32:
				{
					float *pF32 = (float *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
					float sampleF32 = *pF32;
					sampleValue = (double)sampleF32;
				} break;

				case fplAudioFormatType_S32:
				{
					int32_t *pS32 = (int32_t *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
					int32_t sampleS32 = *pS32;
					sampleValue = (double)sampleS32 / (double)INT32_MAX;
				} break;

				case fplAudioFormatType_S16:
				{
					int16_t *pS16 = (int16_t *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
					int16_t sampleS16 = *pS16;
					sampleValue = (double)sampleS16 / (double)INT16_MAX;
				} break;
			}

			// Window multiplier (Hamming for smoother visualization)
			double windowMultiplier = demo->spectrum.windowCoeffs[i];

			demo->spectrum.input[i].real = sampleValue * windowMultiplier;
			demo->spectrum.input[i].imag = 0;
		}

		ForwardFFT(demo->spectrum.input, frameCount, false, demo->spectrum.output);

		// Save magnitudes for FFT output
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double re = fabs(demo->spectrum.output[frameIndex].real);
			double im = fabs(demo->spectrum.output[frameIndex].imag);
			double magnitude = sqrt(re * re + im * im) * 2.0 / (double)frameCount;
			demo->spectrum.magnitudes[frameIndex] = magnitude;
		}

		// Reset and evaluate max peaks
		uint32_t binCount = MAX_AUDIO_BIN_COUNT;
		for(uint32_t i = 0; i < binCount; ++i) {
			demo->spectrum.maxPeaks[i] = -INFINITY;
		}
		double sampleRatePerFrame = (double)demo->targetAudioFormat.sampleRate / (double)frameCount;
		for(uint32_t frameIndex = 0; frameIndex < frameCount / 2; ++frameIndex) {
			double magnitude = demo->spectrum.magnitudes[frameIndex];
			double freq = frameIndex * sampleRatePerFrame;
			for(uint32_t binIndex = 0; binIndex < (binCount + 1); ++binIndex) {
				if((freq > demo->spectrum.bins[binIndex]) && (freq <= demo->spectrum.bins[binIndex + 1])) {
					if(magnitude > demo->spectrum.maxPeaks[binIndex]) {
						demo->spectrum.maxPeaks[binIndex] = magnitude;
					}
				}
			}
		}


#if 1
		// Draw samples
		{
			float spacing = 3.0f;
			float totalSpacing = spacing * (frameCount - 1);
			float barMaxWidth = spectrumDim.w / (float)frameCount;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)frameCount;

			for(uint32_t i = 0; i < frameCount; ++i) {
				float sampleValue = 0.0;
				switch(format) {
					case fplAudioFormatType_F32:
					{
						float *pF32 = (float *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
						float sampleF32 = *pF32;
						sampleValue = sampleF32;
					} break;

					case fplAudioFormatType_S32:
					{
						int32_t *pS32 = (int32_t *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
						int32_t sampleS32 = *pS32;
						sampleValue = (float)sampleS32 / (float)INT32_MAX;
					} break;

					case fplAudioFormatType_S16:
					{
						int16_t *pS16 = (int16_t *)(demo->videoAudioChunks[0].samples + i * frameSize + channel * sampleSize);
						int16_t sampleS16 = *pS16;
						sampleValue = (float)sampleS16 / (float)INT16_MAX;
					} break;
				}

				// Normalize sample value
				sampleValue = sqrtf(sampleValue * sampleValue);

				float barMaxHeight = spectrumDim.h * 0.5f;
				float barHeight = sampleValue * barMaxHeight;
				float barX = spectrumPos.x + i * barWidth + i * spacing;
				float barY = spectrumPos.y + spectrumDim.h * 0.5f;
				RenderRectangle(barX, barY + barHeight * 0.5f, barX + barWidth, barY - barHeight * 0.5f, (Vec4f) { 1, 1, 0, 1 }, 1.0f);
			}
		}
#endif


#if 0
		// Draw FFT bins/peaks (Is incorrect)
		{
			float fitFactor = 1.0f;
			float spacing = 4.0f;
			float totalSpacing = spacing * (binCount - 1);
			float barMaxWidth = spectrumDim.w / (float)binCount;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)binCount;
			float barY = spectrumPos.y + barMaxHeight;

			for(uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
				double maxPeak = demo->spectrum.maxPeaks[binIndex];
				double db = AmplitudeToDecibel(maxPeak);
				double minDB = -90.0;
				if(db < minDB) db = minDB;
				float sampleValue = (float)(db / minDB);

				// https://dsp.stackexchange.com/questions/32076/fft-to-spectrum-in-decibel

				float barX = spectrumPos.x + binIndex * barWidth + binIndex * spacing;
				float barHeight = sampleValue * barMaxHeight;
				RenderQuad(barX, barY, barX + barWidth, barY - barHeight, (Vec4f) { 0, 1, 0, 0.1f });
			}
		}
#endif

#if 1
		// Draw raw FFT
		{
			uint32_t fftCount = frameCount;
			float spacing = 6.0f;
			float totalSpacing = spacing * (fftCount - 1);
			float barMaxWidth = spectrumDim.w / (float)fftCount;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)fftCount;
			float barY = spectrumPos.y + barMaxHeight;

			float magScale = 4.0f;

			// https://dsp.stackexchange.com/questions/32076/fft-to-spectrum-in-decibel

			for(uint32_t frameIndex = 0; frameIndex < fftCount; ++frameIndex) {
				double magnitude = demo->spectrum.magnitudes[frameIndex];
				float sampleValue = (float)magnitude * magScale;
				float barX = spectrumPos.x + frameIndex * barWidth + frameIndex * spacing;
				float barHeight = sampleValue * barMaxHeight;
				RenderQuad(barX, barY, barX + barWidth, barY - barHeight, (Vec4f) { 0, 1, 0, 1 });
			}
		}
#endif
	}  // chunkFrameCount > 0

//fplDebugFormatOut("Current render/play time: [%f, %f], played/streamed frames: [%lu, %lu], chunks: %zu\n", currentRenderTime, currentPlayTime, playedFrames, streamedFrames, numChunks);
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
		uint32_t numFramesPlayed = fplAtomicFetchAndAddU32(&demo->numFramesPlayed, result);

		const uint32_t updateInterval = 1000 / 60;

		if((framesToCopy >= MAX_AUDIO_FRAMES_CHUNK_FRAMES) && ((fplGetTimeInMillisecondsLP() - demo->lastVideoAudioChunkUpdateTime) >= updateInterval)) {
			if(fplIsAtomicCompareAndSwapU32(&demo->hasVideoAudioChunk, 0, 1)) {
				demo->videoAudioChunks[1].index = numFramesPlayed;
				demo->videoAudioChunks[1].count = MAX_AUDIO_FRAMES_CHUNK_FRAMES;
				size_t chunkSamplesSize = frameSize * MAX_AUDIO_FRAMES_CHUNK_FRAMES;
				fplMemoryCopy(outputSamples, chunkSamplesSize, demo->videoAudioChunks[1].samples);
				fplAtomicExchangeU32(&demo->hasVideoAudioChunk, 2);
			}
			demo->lastVideoAudioChunkUpdateTime = fplGetTimeInMillisecondsLP();
		}
	}

#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY

	return(result);
}

static bool StreamAudio(const fplAudioDeviceFormat *format, const uint32_t maxFrameCount, AudioDemo *demo, uint64_t *outDuration) {
	if(format == fpl_null || demo == fpl_null) return(false);
	if(maxFrameCount == 0) return(false);

	LockFreeRingBuffer *streamRingBuffer = &demo->streamRingBuffer;
	LockFreeRingBuffer *chunkRingBuffer = &demo->chunkRingBuffer;

	AudioBuffer *tmpBuffer = &demo->streamTempBuffer;

	size_t frameSize = fplGetAudioFrameSizeInBytes(format->type, format->channels);

	size_t availableStreamSpace = 0;
	bool canStreamWrite = LockFreeRingBufferCanWrite(streamRingBuffer, &availableStreamSpace);

	if(canStreamWrite && (availableStreamSpace % frameSize) == 0) {
		uint64_t timeStart = fplGetTimeInMillisecondsLP();

		AudioFrameIndex numOfAvailableFrames = (AudioFrameIndex)fplMax(0, availableStreamSpace / frameSize);

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

		// Write samples from temporary buffer to stream ring buffer
		bool streamWritten = LockFreeRingBufferWrite(streamRingBuffer, tmpBuffer->samples, totalFrameBytes);
		fplAssert(streamWritten);

		uint32_t lastNumFramesStreamed = fplAtomicFetchAndAddU32(&demo->numFramesStreamed, writtenFrames);

		//
		// Fill chunks for visualization
		//
#if 0
		size_t chunkSize = sizeof(AudioFramesChunk);
		size_t availableChunkSpace = 0;
		bool canChunkWrite = LockFreeRingBufferCanWrite(chunkRingBuffer, &availableChunkSpace);
		if(canChunkWrite && availableChunkSpace >= chunkSize && totalFrameBytes >= chunkSize) {
			AudioFrameIndex remainingChunkFrames = writtenFrames;

			size_t nbytes = fplMin(availableChunkSpace, totalFrameBytes);

			size_t numChunks = nbytes / chunkSize;

			size_t totalChunksSize = numChunks * chunkSize;

			AudioFramesChunk tempChunk;
			AudioFrameIndex maxFramesPerChunk = MAX_AUDIO_FRAMES_CHUNK_FRAMES;

			AudioFrameIndex chunkFrameStartTime = lastNumFramesStreamed;

			void *chunkTempMemory = demo->chunkTempBuffer;

			size_t sourceOffset = 0;
			for(size_t chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex) {
				AudioFrameIndex framesForChunk = fplMin(maxFramesPerChunk, remainingChunkFrames);
				size_t chunkSamplesSize = framesForChunk * frameSize;

				AudioFramesChunk *chunk = (AudioFramesChunk *)((uint8_t *)chunkTempMemory + (chunkSize * chunkIndex));

				double chunkTime = fplGetAudioBufferSizeInMilliseconds(demo->targetAudioFormat.sampleRate, chunkFrameStartTime);

				fplClearStruct(chunk);
				chunk->index = chunkFrameStartTime;
				chunk->count = framesForChunk;
				fplAssert(chunkSamplesSize <= sizeof(chunk->samples));
				fplMemoryCopy(tmpBuffer->samples + sourceOffset, chunkSamplesSize, chunk->samples);

				remainingChunkFrames -= framesForChunk;
				chunkFrameStartTime += framesForChunk;
				sourceOffset += chunkSamplesSize;
			}

			bool chunksWritten = LockFreeRingBufferWrite(chunkRingBuffer, chunkTempMemory, totalChunksSize);
			fplAssert(chunksWritten);
		}
#endif

		if(outDuration != fpl_null) {
			uint64_t delta = fplGetTimeInMilliseconds() - timeStart;
			*outDuration = (uint32_t)delta;
		}

		return(true);
	}

	return(false);
}

typedef struct {
	AudioFrameIndex frames;
	AudioMilliseconds delay;
	bool canIgnoreWait;
} AudioFrameDelayEntry;

static void AudioStreamingThread(const fplThreadHandle *thread, void *rawData) {
	AudioDemo *demo = (AudioDemo *)rawData;

	// This thing has a few issues on slow machines:
	// - Too much frames per loop is too much to handle on my linux machine (8192 frames seems to be just fine)
	// - Delay is bad when streaming is too slow, so we need stop it entirely -> Sleep seems to be very expensive on linux (Scheduler granularity?)

	// On fast machines we want:
	// - High delay when we are too fast
	// - Increase frames to stream in more data per loop

	// Audio characteristics table
	// Number of frames to stream | Delay | Ignore wait
	// ------------------------------------------------
	//   2048                     | 1     | YES
	//   2048                     | 1     | YES
	//   4192                     | 1     | YES
	//   4192                     | 2     | YES
	//   8192                     | 2     | YES
	//   8192                     | 4     | YES
	//  16384                     | 4     | NO
	//  16384                     | 6     | NO
	//  32768                     | 6     | NO
	//  32768                     | 8     | NO
	//  65536                     | 8     | NO
	//  65536                     | 10    | NO
	// 131072                     | 10    | NO
	// 131072                     | 15    | NO
	// 262144                     | 15    | NO
	// 262144                     | 20    | NO

	const AudioFrameDelayEntry entries[] = {
		fplStructInit(AudioFrameDelayEntry, 2048, 1, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 2, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 4, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 6, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 8, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 1, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 2, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 4, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 6, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 8, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 2, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 4, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 6, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 8, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 10, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 12, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 15, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 20, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 25, true),
		fplStructInit(AudioFrameDelayEntry, 16384, 4, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 6, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 8, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 10, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 12, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 15, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 20, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 25, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 50, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 75, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 100, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 150, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 200, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 4, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 6, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 8, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 10, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 12, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 15, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 20, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 25, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 50, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 75, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 100, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 150, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 200, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 6, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 8, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 10, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 12, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 15, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 20, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 25, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 50, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 75, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 100, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 150, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 200, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 10, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 15, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 20, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 25, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 50, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 75, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 100, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 150, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 200, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 15, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 20, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 25, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 50, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 100, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 150, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 200, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 300, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 400, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 500, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 1000, false),
	};

	const uint32_t intitialIndex = 0;

	uint32_t entryIndex = intitialIndex;
	AudioFrameDelayEntry currentEntry = entries[entryIndex];

	// Depending on the performance we can jump in table rows instead of dynamically computing stuff?
	LockFreeRingBuffer *circularBuffer = &demo->streamRingBuffer;

	const AudioFrameIndex bufferFrameCount = demo->streamTempBuffer.frameCount;
	const uint64_t totalBufferLength = circularBuffer->length;

	const float minBufferThreshold = 0.25f; // In percentage range of 0 to 1
	const float maxBufferThreshold = 0.75f; // In percentage range of 0 to 1

	bool ignoreWait = false;
	uint64_t startTime = fplGetTimeInMillisecondsLP();
	while(!demo->isStreamingThreadStopped) {
		// Wait if needed
		bool wait = !ignoreWait;
		if(wait || !currentEntry.canIgnoreWait) {
			uint64_t deltaTime = fplGetTimeInMillisecondsLP() - startTime;
			if(deltaTime < currentEntry.delay) {
				fplThreadSleep(1);
				continue;
			}
			startTime = fplGetTimeInMillisecondsLP();
		}

		if(ignoreWait) {
			// We just want to ignore waiting once
			ignoreWait = true;
		}

		bool tooSlow = false;
		bool tooFast = false;

		uint64_t streamDuration = 0;
		if(StreamAudio(&demo->targetAudioFormat, currentEntry.frames, demo, &streamDuration)) {
			if(streamDuration > currentEntry.delay) {
				// We are taking too slow to stream in new audio samples
				tooSlow = true;
			}
		}

		uint64_t fillCount = (uint64_t)fplAtomicLoadS64(&circularBuffer->fillCount);
		float percentageFilled = (1.0f / (float)totalBufferLength) * (float)fillCount;
		if(percentageFilled < minBufferThreshold) {
			// We are not filling the buffer fast enough, maybe due to streaming slowness
			tooSlow = true;
		} else if(percentageFilled > maxBufferThreshold) {
			// We are too fast
			tooFast = true;
		}

		if(tooSlow) {
			// Go back one characteristics entry
			if(entryIndex > 0) {
				currentEntry = entries[--entryIndex];
				if(currentEntry.canIgnoreWait) {
					ignoreWait = true;
				}
			} else {
				ignoreWait = true; // We are the worst entry, ignore any waiting
			}
		} else if(tooFast) {
			// Go forward one characteristics entry
			if(entryIndex < (fplArrayCount(entries) - 1)) {
				currentEntry = entries[++entryIndex];
				if(!currentEntry.canIgnoreWait) {
					ignoreWait = false;
				}
			} else {
				ignoreWait = false; // We are the max entry, never ignore waiting
			}
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

	// Set number of channels
	settings.audio.targetFormat.channels = 2;

	// Set samplerate in Hz
	//settings.audio.targetFormat.sampleRate = 11025;
	//settings.audio.targetFormat.sampleRate = 22050;
	settings.audio.targetFormat.sampleRate = 44100;
	//settings.audio.targetFormat.sampleRate = 48000;

	// Optionally set buffer size in milliseconds or in frames
	//settings.audio.targetFormat.bufferSizeInMilliseconds = 16;
	//settings.audio.targetFormat.bufferSizeInFrames = 512;

	// Disable auto start/stop of audio playback
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	//
	// Find audio device
	//
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		goto done;
	}

	// Get number of audio devices
	uint32_t deviceCount = fplGetAudioDevices(fpl_null, 0);

	// Allocate memory for audio devices and fill it
	fplAudioDeviceInfo *audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * deviceCount);
	uint32_t loadedDeviceCount = fplGetAudioDevices(audioDeviceInfos, deviceCount);
	fplAssert(loadedDeviceCount == deviceCount);
	// Use first audio device
	if(loadedDeviceCount > 0) {
		settings.audio.targetDevice = audioDeviceInfos[0];
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
		// Initialze playback latency
		demo->maxPlaybackFrameLatency = demo->targetAudioFormat.bufferSizeInFrames / demo->targetAudioFormat.periods;

		size_t maxPlayItemsFrameCount = 0;
		size_t playitemCount = AudioSystemGetPlayItems(&demo->audioSys, fpl_null, 0);
		AudioPlayItem *playItems = fplMemoryAllocate(sizeof(AudioPlayItem) * playitemCount);
		AudioSystemGetPlayItems(&demo->audioSys, playItems, playitemCount);
		for(size_t audioSourceIndex = 0; audioSourceIndex < playitemCount; ++audioSourceIndex) {
			const AudioPlayItem *playItem = playItems + audioSourceIndex;
			const AudioSource *audioSource = playItem->source;
			AudioFrameIndex sourceFrameCount = audioSource->buffer.frameCount;
			maxPlayItemsFrameCount = fplMax(maxPlayItemsFrameCount, sourceFrameCount);
		}

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_STREAMBUFFER_ONLY
		AudioFrameIndex streamBufferFrames = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 10000);

		// Init streaming buffer and read some frames at the very start
		size_t streamBufferSize = fplGetAudioBufferSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels, streamBufferFrames);
		bool bufferInitRes = LockFreeRingBufferInit(&demo->streamRingBuffer, streamBufferSize, true);
		if(!bufferInitRes) {
			goto done;
		}

		// Init chunk buffer
		size_t numChunks = fplMax(1, streamBufferSize / sizeof(AudioFramesChunk));
		size_t chunkBufferSize = numChunks * sizeof(AudioFramesChunk);
		bufferInitRes = LockFreeRingBufferInit(&demo->chunkRingBuffer, chunkBufferSize, true);
		if(!bufferInitRes) {
			goto done;
		}

		// Allocate temporary stream buffer
		AudioFormat streamTempBufferFormat = fplZeroInit;
		streamTempBufferFormat.channels = demo->targetAudioFormat.channels;
		streamTempBufferFormat.format = demo->targetAudioFormat.type;
		streamTempBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
		bufferInitRes = AllocateAudioBuffer(&demo->audioSys, &demo->streamTempBuffer, &streamTempBufferFormat, streamBufferFrames);
		if(!bufferInitRes) {
			goto done;
		}

		// Allocate temporary chunk buffer
		size_t tempChunkTempBufferSize = (numChunks + 2) * sizeof(AudioFramesChunk);
		demo->chunkTempBuffer = fplMemoryAlignedAllocate(tempChunkTempBufferSize, 16);

		// Initialize frequency bins
		uint32_t frequencyBinCount = fplArrayCount(demo->spectrum.bins);
		for(uint32_t i = 0; i < frequencyBinCount; ++i) {
			double freq = i * (double)demo->targetAudioFormat.sampleRate / (double)frequencyBinCount;
			demo->spectrum.maxPeaks[i] = 0;
			demo->spectrum.bins[i] = freq;
		}

		// Init window coefficients
		HammingWindowFunction(demo->spectrum.windowCoeffs, fplArrayCount(demo->spectrum.input));

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
			const char *outDriver = fplGetAudioDriverName(currentSettings->audio.driver);
			const char *outFormat = fplGetAudioFormatTypeName(demo->audioSys.targetFormat.format);
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
			double totalTime = 0.0;
			fplWallClock lastTime = fplGetWallClock();
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
				Render(demo, winSize.width, winSize.height, totalTime);
				fplVideoFlip();

				fplWallClock curTime = fplGetWallClock();
				double frameTime = fplGetWallDelta(lastTime, curTime);
				totalTime += frameTime;
				lastTime = curTime;
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
	fplMemoryAlignedFree(demo->chunkTempBuffer);
	FreeAudioBuffer(&demo->audioSys, &demo->streamTempBuffer);
	LockFreeRingBufferRelease(&demo->chunkRingBuffer);
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