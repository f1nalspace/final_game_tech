/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio Player

Description:
	This demo shows how to play music, sounds using a custom audio system/mixer.
	It supports uncompressed PCM wave data, OGG Vorbis and MP3 Files.
	Resampling support is limited to only even sample rates.

	In addition all samples are cached in a lock-free ringbuffer and are played back properly, see AudioPlayback() for more details.

	The audio tracks are streamed in and use a slow/fast detection to only cache when it needs to, see AudioStreamingThread() for more details.

	To make it more appealing all audio samples are visualized with OpenGL and uses several algorythms, such FFT, Windowing, Smoothing, etc.
	This can be shown from the full audio buffer, or in realtime that is filled directly in the streaming thread.

	Everything together is very complex and requires a good understanding how digital sound is played back in a computer.

How the demo works:
	# Audio System

	To make playback of any sort of audio stream possible, there is a rather complex system under the hood that does the following:

	- Loading audio sources (Wave, Ogg, MP3) from either file or data streams into PCM format, that consists of infos such sample rate, number of channels, data type and the sample data itself
	- Separating of audio track and audio sources
	- Converting audio samples into floating point, that is used for the mixing and resampling
	- Resampling of audio samples by either (Simple up/down sampling or SinC based resampling)
	- Applying effects per audio track (Volume, Filters, etc.)
	- Mixing of multiple audio samples from input channels into output channels
	- Converting back the mixed samples into the target sample format
	- Caching of audio samples into chunks to increase performance

	# Audio playback thread

	The thread function AudioPlayback() is responsible for filling out N audio frames/samples of the audio device playback period buffer.
	This may be called thousands of times per second, therefore has a very tight time-frame and we can't use any I/O or locking operations there. 
	The samples are lock-free read from a ring-buffer that is continuosly filled by the streaming thread.

	# Audio streaming thread

	The thread function AudioStreamingThread() is filling a ring buffer that will be used in the audio playback thread, that eventually plays back the audio samples.

	It also detects new audio tracks that needs to be loaded and loads them automatically.

	There is a audio buffer that is fully loaded once, that is used for visualizing the samples.
	This may take a while, depending on the input/target sample rates and the length of the source tracks.

	In addition there is a smart algorythm, that detects the playback latency and adjusts the amount and duration of the ring-buffer that needs to be filled.
	This is important, because slow hardware requires more buffering than fast hardware - but a fixed buffer size is not sufficient enough.

	The samples ring-buffer are filled by the AudioSystemWriteFrames() function, which may advanced the audio system play cursor.

	# Main

	# Rendering

	All rendering is done using oldschool style OpenGL 1.x.

	The converted mono samples are visualized by either:
	- Fast Fourier Transformation
	- Pure sample drawing
	- Line drawing
	- Spectrum analysis (Incomplete)

	The ring-buffer is visualized as simple bars with a tail and head position.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Todo:
	- Smoother bars (Falling peaks, Max peak line -> https://www.youtube.com/watch?v=JU-uCytFvmI&list=UU56Qctnsu8wAyvzf4Yx6LIw&index=16)
	- Audio streaming from HTTP (requires networking in FPL!)
	- Multiple audio tracks

Changelog:
	## 2025-03-25
	- Added progressbar to indicate current position in the track
	- Fixed streamed/played frames was not reset on drag & drop

	## 2025-03-21
	- Compute the actual target frame count for the full audio buffer, so that we can up/down sample properly
	- Support for resampling from odd frequency ratios, such as 48000 <-> 41000 using SinC

	## 2025-03-15
	- Support for swapping out the audio track by drag & drop another file

	## 2025-03-09
	- Improved visualization a lot
	- Detect default audio device properly

	## 2025-03-08
	- Fixed crash when no audio track was loaded
	- Fixed sine wave streaming was totally broken

	## 2025-02-14
	- Get average sample value from stereo samples

	## 2023-05-28
	- Fixed crash/assert when audio visualization chunk is not ready yet

	## 2021-11-06
	- Proper FFT power spectrum and audio bin drawing

	## 2021-03-13
	- Samples and FFT visualization for realtime audio

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
	Copyright (c) 2017-2025 Torsten Spaete
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

#define APP_TITLE "FPL Demo | Audio Player"

#define OPT_PLAYBACK_NONE 0
#define OPT_PLAYBACK_SINEWAVE 1
#define OPT_PLAYBACK_AUDIOSYSTEM 2
#define OPT_PLAYBACK_STREAMBUFFER 3

#define OPT_PLAYBACK OPT_PLAYBACK_STREAMBUFFER

#define FPL_LOGGING
#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
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

#include <final_music.h>

#define MAX_AUDIO_FRAMES_CHUNK_FRAMES 256
typedef struct AudioFramesChunk {
	uint8_t samples[MAX_AUDIO_FRAMES_CHUNK_FRAMES * AUDIO_MAX_CHANNEL_COUNT * AUDIO_MAX_SAMPLESIZE];
	AudioFrameIndex index;
	AudioFrameIndex count;
} AudioFramesChunk;

static const int AudibleFrequencyRanges[] = {
	20,
	60,
	250,
	500,
	2000,
	4000,
	6000,
	20000
};

#define MAX_AUDIO_BIN_COUNT 32

typedef struct AudioVisualization {
	AudioFramesChunk videoAudioChunks[2]; // 0 = Render, 1 = New
	FFTDouble fftInput[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	FFTDouble fftOutput[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	float rawSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES * AUDIO_MAX_CHANNEL_COUNT];
	float monoSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double currentSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double lastSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double currentMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double lastMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double scaledMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double scaledSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double windowCoeffs[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double spectrum[MAX_AUDIO_BIN_COUNT];
	double bins[MAX_AUDIO_BIN_COUNT];
	volatile uint32_t hasVideoAudioChunk;
} AudioVisualization;

typedef struct AudioDemo {
	AudioVisualization visualization;
	AudioTrackList trackList;
	AudioSystem audioSys;

	LockFreeRingBuffer outputRingBuffer;	// The ring buffer for the audio output
	AudioBuffer outputTempBuffer;			// Used for decoding the audio samples into, before its pushed to the output ring buffer

	AudioSineWaveData sineWave;
	fplAudioFormat targetAudioFormat;
	fplThreadHandle *streamingThread;

	uint64_t lastVideoAudioChunkUpdateTime;
	volatile uint32_t numFramesPlayed;
	volatile uint32_t numFramesStreamed;
	volatile uint32_t maxPlaybackFrameLatency;

	volatile fpl_b32 isStreamingThreadStopped;
	fpl_b32 useRealTimeSamples;
} AudioDemo;

static void UpdateTitle(AudioDemo *demo, const char *audioTrackName, const bool isRealTime, const double fps) {
	char titleBuffer[256];
	const char *rtString = (isRealTime ? "RT" : "BUF");
	const fplSettings *settings = fplGetCurrentSettings();
	if (fplGetStringLength(audioTrackName) > 0)
        fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s (%s, %u Hz, %u ch) - %s [%.3f fps]", APP_TITLE, rtString, demo->targetAudioFormat.sampleRate, demo->targetAudioFormat.channels, audioTrackName, fps);
	else
        fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s (%s, %u Hz, %u ch) [%.3f fps]", APP_TITLE, rtString, demo->targetAudioFormat.sampleRate, demo->targetAudioFormat.channels, fps);
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
	RenderRectangle(pos.x, pos.y, pos.x + dim.w, pos.y + dim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

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

static void ClearVisualization(AudioDemo *demo) {
	fplAtomicExchangeU32(&demo->visualization.hasVideoAudioChunk, 0);
	fplClearStruct(&demo->visualization.videoAudioChunks);
	fplClearStruct(&demo->visualization.currentMagnitudes);
	fplClearStruct(&demo->visualization.rawSamples);
	fplClearStruct(&demo->visualization.monoSamples);
	fplClearStruct(&demo->visualization.lastMagnitudes);
	fplClearStruct(&demo->visualization.currentSamples);
	fplClearStruct(&demo->visualization.lastSamples);
	fplClearStruct(&demo->visualization.fftInput);
	fplClearStruct(&demo->visualization.fftOutput);
	fplClearStruct(&demo->visualization.scaledMagnitudes);
	fplClearStruct(&demo->visualization.scaledSamples);
}

static void Render(AudioDemo *demo, const int screenW, const int screenH, const double currentRenderTime) {
	float w = (float)screenW;
	float h = (float)screenH;

	glViewport(0, 0, screenW, screenH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Window coordinate system
	glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	// Draw center cross
	RenderLine(0.0f, h * 0.5f, w, h * 0.5f, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
	RenderLine(w * 0.5f, 0, w * 0.5f, h, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
#endif

#if OPT_PLAYBACK == OPT_PLAYBACK_STREAMBUFFER
	LockFreeRingBuffer *streamRingBuffer = &demo->outputRingBuffer;

	static AudioBuffer zeroFullAudioBufferStub = fplZeroInit;
	AudioBuffer *fullAudioBuffer = &zeroFullAudioBufferStub;

	AudioTrackList *trackList = &demo->trackList;

	if(HasAudioTrack(trackList)) {
		fplAssert(trackList->currentIndex < (int32_t)trackList->count);
		AudioTrack *track = trackList->tracks + trackList->currentIndex;
		fullAudioBuffer = &track->outputFullBuffer;
	}

	AudioVisualization *visualization = &demo->visualization;

	const float marginW = w * 0.05f;
	const float marginH = h * 0.05f;
	const float maxBufferW = w - marginW * 2.0f;
	const float maxBufferH = h * 0.1f;
	const float bufferBarH = maxBufferH * 0.9f;
	const float progressW = w - marginW * 2.0f;
	const float progressH = h * 0.05f;
	const float progressBarPadding = 5.0f;
	const float progressBarMaxWidth = progressW - progressBarPadding * 2.0f;
	const float progressBarMaxHeight = progressH - progressBarPadding * 2.0f;

	const float spectrumWidth = w - marginW * 2.0f;
	const float spectrumHeight = h - maxBufferH - progressH - marginH * 2.0f;

	Vec2f streamBufferDim = V2fInit(maxBufferW, maxBufferH);
	Vec2f streamBufferPos = V2fInit((w - streamBufferDim.w) * 0.5f, h - marginH - streamBufferDim.h);

	Vec2f spectrumDim = V2fInit(spectrumWidth, spectrumHeight);
	Vec2f spectrumPos = V2fInit((w - spectrumDim.w) * 0.5f, marginH + progressH);

	Vec2f progressDim = V2fInit(progressW, progressH);
	Vec2f progressPos = V2fInit((w - progressDim.w) * 0.5f, marginH);

	RenderRingBuffer(streamBufferPos, streamBufferDim, streamRingBuffer);

	RenderRectangle(spectrumPos.x, spectrumPos.y, spectrumPos.x + spectrumDim.w, spectrumPos.y + spectrumDim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

	fplAudioFormatType format = demo->targetAudioFormat.type;
	size_t sampleSize = fplGetAudioSampleSizeInBytes(format);
	AudioChannelIndex channelCount = demo->targetAudioFormat.channels;
	size_t frameSize = sampleSize * channelCount;
	AudioFrameIndex frameCount = visualization->videoAudioChunks[0].count;
	AudioFramesChunk *chunk = &visualization->videoAudioChunks[0];

	uint8_t *chunkSamples = chunk->samples;

	AudioFrameIndex framesPlayed;
	if(demo->useRealTimeSamples) {
		if(fplAtomicIsCompareAndSwapU32(&visualization->hasVideoAudioChunk, 2, 3)) {
			visualization->videoAudioChunks[0] = visualization->videoAudioChunks[1];
			fplAtomicExchangeU32(&visualization->hasVideoAudioChunk, 0);
		}

		framesPlayed = chunk->index;
	} else {

#if 1
		framesPlayed = fplAtomicLoadU32(&demo->numFramesPlayed);
#else
		double renderMsecs = currentRenderTime * 1000.0;
		framesPlayed = (AudioFrameIndex)(((double)demo->targetAudioFormat.sampleRate / 1000.0) * renderMsecs);
		if(framesPlayed > fullAudioBuffer->frameCount) {
			framesPlayed = fullAudioBuffer->frameCount;
		}
		fplAssert(framesPlayed <= fullAudioBuffer->frameCount);
#endif

		AudioFrameIndex remainingFramesToPlay = framesPlayed < fullAudioBuffer->frameCount ? fullAudioBuffer->frameCount - framesPlayed : 0;
		AudioFrameIndex remainingChunkFrames = fplMin(MAX_AUDIO_FRAMES_CHUNK_FRAMES, remainingFramesToPlay);
		if(remainingChunkFrames > 0) {
			size_t sourceFrameSize = fullAudioBuffer->bufferSize / fullAudioBuffer->frameCount;
			fplAssert(sourceFrameSize == frameSize);
			size_t totalSizeToCopy = remainingChunkFrames * frameSize;
			size_t chunkSamplesOffset = framesPlayed * frameSize;
			size_t endPos = chunkSamplesOffset + totalSizeToCopy;
			fplAssert(endPos <= fullAudioBuffer->bufferSize);
			const uint8_t *p = fullAudioBuffer->samples + chunkSamplesOffset;
			fplMemoryCopy(p, totalSizeToCopy, chunkSamples);
		}

		if(remainingChunkFrames < MAX_AUDIO_FRAMES_CHUNK_FRAMES && chunk->count >= MAX_AUDIO_FRAMES_CHUNK_FRAMES) {
			size_t totalSizeToClear = MAX_AUDIO_FRAMES_CHUNK_FRAMES * frameSize;
			fplMemoryClear(chunkSamples, totalSizeToClear);
		}

		frameCount = MAX_AUDIO_FRAMES_CHUNK_FRAMES;
	}

	RenderRectangle(progressPos.x, progressPos.y, progressPos.x + progressDim.w, progressPos.y + progressDim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

	float progressBarScale = 0.0f;
	if (fullAudioBuffer->frameCount > 0) {
		progressBarScale = fplMax(0.0f, fplMin(1.0f, framesPlayed / (float)fullAudioBuffer->frameCount));
	}

	float progressBarWidth = progressBarMaxWidth * progressBarScale;
	RenderQuad(progressPos.x + progressBarPadding, progressPos.y + progressBarPadding, progressPos.x + progressBarPadding + progressBarWidth, progressPos.y + progressBarPadding + progressBarMaxHeight, (Vec4f) { 1, 1, 0, 1.0f });

	if(frameCount > 0 && chunkSamples != fpl_null) {
		// Convert all samples to float
		bool convertRes = AudioSamplesConvert(&demo->audioSys.conversionFuncs, frameCount * channelCount, format, fplAudioFormatType_F32, chunkSamples, visualization->rawSamples);
		fplAssert(convertRes == true);

		// Convert samples to mono
		convertRes = AudioSamplesMonolize(channelCount, frameCount, visualization->rawSamples, visualization->monoSamples);
		fplAssert(convertRes == true);

		// Build FFT input samples from mono samples
		// Apply hanning window (Coefficients are precomputed)
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double sampleValue = visualization->monoSamples[frameIndex];
			double windowMultiplier = visualization->windowCoeffs[frameIndex];
			double adjustedSampleValue = sampleValue * windowMultiplier;
			double clampedSample = fplMax(-1.0, fplMin(adjustedSampleValue, 1.0));
			visualization->lastSamples[frameIndex] = visualization->currentSamples[frameIndex];
			visualization->currentSamples[frameIndex] = clampedSample;
			visualization->fftInput[frameIndex].real = clampedSample;
			visualization->fftInput[frameIndex].imag = 0;
		}

		// Smooth out samples (just for visualization)
		const double samplesSmooth = 0.35;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double lastSample = visualization->lastSamples[frameIndex];
			double currentSample = visualization->currentSamples[frameIndex];
			double newSample = lastSample * (1.0 - samplesSmooth) + currentSample * samplesSmooth;
			visualization->currentSamples[frameIndex] = newSample;
		}

		// Track min/max samples
		double minSamples = visualization->currentSamples[0];
		double maxSamples = visualization->currentSamples[0];
		for(uint32_t frameIndex = 1; frameIndex < frameCount; ++frameIndex) {
			double sample = visualization->currentSamples[frameIndex];
			if(sample > maxSamples) {
				maxSamples = sample;
			}
			if(sample < minSamples) {
				minSamples = sample;
			}
		}

		const bool sampleScaling = false;

		if (!sampleScaling) {
			// No sample scaling
			double scaleSamplesFitFactor = 2.0f;
			for (uint32_t frameIndex = 1; frameIndex < frameCount; ++frameIndex) {
				visualization->scaledSamples[frameIndex] = visualization->currentSamples[frameIndex] * scaleSamplesFitFactor;
			}
		} else {
			// Normalize samples to be in full range of -1.0 to 1.0, just for better visualization
			double scaleSamplesFitFactor = 0.75f;
			double rangeSample = maxSamples - minSamples;
			for (uint32_t frameIndex = 1; frameIndex < frameCount; ++frameIndex) {
				double sample = visualization->currentSamples[frameIndex];
				double scaledSample = ((sample - minSamples) / rangeSample) * scaleSamplesFitFactor;
				visualization->scaledSamples[frameIndex] = -1.0f + scaledSample * 2.0f;
			}
		}

		// Forward FFT using raw samples
        ForwardFFT(visualization->fftInput, frameCount, visualization->fftOutput);

		const uint32_t halfFFT = frameCount / 2;

		const bool useLogarythmBase = true;

		// Compute raw magnitudes (We do it for the entire FFT, not just the half because i want to see all of it)
		// Convert magnitudes into log() + Track last magnitudes for later use
		for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double re = visualization->fftOutput[frameIndex].real;
			double im = visualization->fftOutput[frameIndex].imag;
			double rawMagnitude = sqrt(re * re + im * im);
			double magnitude;
			if (useLogarythmBase)
				magnitude = log(1.0 + rawMagnitude);
			else
				magnitude = rawMagnitude;
			visualization->lastMagnitudes[frameIndex] = visualization->currentMagnitudes[frameIndex];
			visualization->currentMagnitudes[frameIndex] = magnitude;
		}

		// Smooth magnitudes
		const double magSmooth = 0.4;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double lastMagnitude = visualization->lastMagnitudes[frameIndex];
			double currentMagnitude = visualization->currentMagnitudes[frameIndex];
			double newMagnitude = lastMagnitude * (1.0 - magSmooth) + currentMagnitude * magSmooth;
			visualization->currentMagnitudes[frameIndex] = newMagnitude;
		}

		// Track min/max magnitudes
		double minMagnitude = visualization->currentMagnitudes[0];
		double maxMagnitude = visualization->currentMagnitudes[0];
		for(uint32_t frameIndex = 1; frameIndex < halfFFT; ++frameIndex) {
			double magnitude = visualization->currentMagnitudes[frameIndex];
			if(magnitude > maxMagnitude) {
				maxMagnitude = magnitude;
			}
			if(magnitude < minMagnitude) {
				minMagnitude = magnitude;
			}
		}

		

		// Normalize the magnitudes into range of 0.0 to 1.0
		const double rangeMagnitude = maxMagnitude - minMagnitude;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double magnitude = visualization->currentMagnitudes[frameIndex];
			double scaledMagnitude = (magnitude - minMagnitude) / rangeMagnitude;
			visualization->scaledMagnitudes[frameIndex] = scaledMagnitude;
		}

		// Reset and evaluate max peaks
		uint32_t binCount = MAX_AUDIO_BIN_COUNT;
		for(uint32_t binIndex = 0; binIndex < binCount - 1; ++binIndex) {
			visualization->spectrum[binIndex] = 0.0;
			double lowerFrequency = visualization->bins[binIndex];
			double upperFrequency = visualization->bins[binIndex + 1];
			for (uint32_t frameIndex = 0; frameIndex < halfFFT; ++frameIndex) {
				double frameFreq = (frameIndex * (double)demo->targetAudioFormat.sampleRate) / (double)frameCount;
				if (frameFreq >= lowerFrequency && frameFreq <= upperFrequency) {
					double scaledMagnitude = visualization->scaledMagnitudes[frameIndex];
					if (scaledMagnitude > visualization->spectrum[binIndex]) {
						visualization->spectrum[binIndex] = scaledMagnitude;
					}
				}
			}
		}

		// Spectrum deformations
		const double fitFactor = 1.0;
		for (uint32_t i = 0; i < binCount; ++i) {
			double value = visualization->spectrum[i];
			visualization->spectrum[i] = value * fitFactor;
		}

#if 1
		// Draw wave form
		{
			float lineX = spectrumPos.x;
			float lineY = spectrumPos.y + spectrumDim.h * 0.5f;
			float maxWaveFormHeight = spectrumDim.h * 0.5f;
			for(uint32_t frameIndex = 0; frameIndex < frameCount - 1; ++frameIndex) {
				double sampleValue1 = visualization->scaledSamples[frameIndex + 0];
				double sampleValue2 = visualization->scaledSamples[frameIndex + 1];
				float x1 = lineX + ((float)(frameIndex + 0) / (float)(frameCount - 1) * spectrumDim.w);
				float x2 = lineX + ((float)(frameIndex + 1) / (float)(frameCount - 1) * spectrumDim.w);
				float y1 = lineY + ((float)sampleValue1 * maxWaveFormHeight * 0.5f);
				float y2 = lineY + ((float)sampleValue2 * maxWaveFormHeight * 0.5f);
				RenderLine(x1, y1, x2, y2, (Vec4f) { 0.8f, 0.25f, 0.05f, 1.0f }, 4.0f);
			}
		}
#endif

#if 1
		// Draw samples
		{
			float spacing = 4.0f;
			float totalSpacing = spacing * (frameCount - 1);
			float barMaxWidth = spectrumDim.w / (float)frameCount;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)frameCount;
			float barMaxHeight = spectrumDim.h * 0.25f;
			float barStartX = spectrumPos.x;
			float barStartY = spectrumPos.y + spectrumDim.h - barMaxHeight;
			for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				double sampleValue = visualization->scaledSamples[frameIndex];
				float barHeight = (float)sampleValue * barMaxHeight;
				float barX = barStartX + frameIndex * barWidth + frameIndex * spacing;
				float barY = barStartY + barMaxHeight * 0.5f;
				RenderQuad(barX, barY + barHeight * 0.5f, barX + barWidth, barY - barHeight * 0.5f, (Vec4f) { 1, 1, 0, 1 });
			}
		}
#endif

#if 1
		// Draw FFT
		{
			float spacing = 4.0f;
			float totalSpacing = spacing * (halfFFT - 1);
			float barMaxWidth = spectrumDim.w / (float)halfFFT;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)halfFFT;
			float barStartX = spectrumPos.x;
			float barStartY = spectrumPos.y;
			for(uint32_t frameIndex = 0; frameIndex < halfFFT; ++frameIndex) {
				double scaledMagnitude = visualization->scaledMagnitudes[frameIndex];
				float barX = barStartX + frameIndex * barWidth + frameIndex * spacing;
				float barHeight = (float)scaledMagnitude * barMaxHeight;
				RenderQuad(barX, barStartY, barX + barWidth, barStartY + barHeight, (Vec4f) { 0.0f, 1.0, 0.1f, 0.25f });
			}
		}
#endif

#if 1
		// Draw spectrum
		{
			float spacing = 2.0f;
			float totalSpacing = spacing * (binCount - 1);
			float barMaxWidth = spectrumDim.w / (float)binCount;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)binCount;
			float barY = spectrumPos.y;
			for(uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
				double scaledMagnitude = visualization->spectrum[binIndex];
				float barX = spectrumPos.x + binIndex * barWidth + binIndex * spacing;
				float barHeight = (float)scaledMagnitude * barMaxHeight;
				RenderQuad(barX, barY, barX + barWidth, barY + barHeight, (Vec4f) { 0.0f, 0.1f, 1.0f, 0.5f });
			}
		}
#endif
	}  // chunkFrameCount > 0

//fplDebugFormatOut("Current render/play time: [%f, %f], played/streamed frames: [%lu, %lu], chunks: %zu\n", currentRenderTime, currentPlayTime, playedFrames, streamedFrames, numChunks);
#endif
}

/**
* @brief Called by the sound device automatically, requesting N audio frames/samples to be written to the output.
* This function has a very tight time-frame and will be called many thousand times per second, so keep implementation to be as small as possible.
* @param[in] outFormat The output audio format, @ref fplAudioFormat.
* @param[in] maxFrameCount The number of frames that needs to be played.
* @param[out] outputSamples The reference to the output samples that is defined by the output format.
* @param[in] userData The user data reference.
* @return Returns the number of audio frames that was written.
*/
static uint32_t AudioPlayback(const fplAudioFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	AudioDemo *demo = (AudioDemo *)userData;

	AudioFrameIndex result = 0;

#if OPT_PLAYBACK == OPT_PLAYBACK_SINEWAVE
	result = maxFrameCount;
	AudioGenerateSineWave(&demo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
#elif OPT_PLAYBACK == OPT_PLAYBACK_AUDIOSYSTEM
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	result = AudioSystemWriteFrames(&demo->audioSys, outputSamples, outFormat, maxFrameCount, true);
#elif OPT_PLAYBACK == OPT_PLAYBACK_STREAMBUFFER
	uint32_t currentPlaybackLatency = fplAtomicLoadU32(&demo->maxPlaybackFrameLatency);
	fplAtomicExchangeU32(&demo->maxPlaybackFrameLatency, fplMax(currentPlaybackLatency, maxFrameCount));

	size_t frameSize = fplGetAudioFrameSizeInBytes(outFormat->type, outFormat->channels);

	LockFreeRingBuffer *ringBuffer = &demo->outputRingBuffer;

	static AudioTrack zeroTrack = fplZeroInit;
	AudioTrack *track = &zeroTrack;

	AudioVisualization *visualization = &demo->visualization;

	if(HasAudioTrack(&demo->trackList)) {
		track = demo->trackList.tracks + demo->trackList.currentIndex;
	}

	size_t availableBytes = 0;
	bool hasData = LockFreeRingBufferCanRead(ringBuffer, &availableBytes);
	if(hasData && (availableBytes % frameSize) == 0) {
		AudioFrameIndex availableFrames = (AudioFrameIndex)fplMax(0, availableBytes / frameSize);

		AudioFrameIndex framesToCopy = fplMin(maxFrameCount, availableFrames);

		size_t totalCopySize = framesToCopy * frameSize;
		fplAssert((totalCopySize % frameSize) == 0);

		bool isRead = LockFreeRingBufferRead(ringBuffer, outputSamples, totalCopySize);
		fplAssert(isRead);

		result = framesToCopy;
		uint32_t numFramesPlayed = fplAtomicFetchAndAddU32(&demo->numFramesPlayed, result);

		if(demo->useRealTimeSamples) {
			const uint64_t updateInterval = 1000 / 60;
			if((framesToCopy >= MAX_AUDIO_FRAMES_CHUNK_FRAMES) && ((fplMillisecondsQuery() - demo->lastVideoAudioChunkUpdateTime) >= updateInterval)) {
				if(fplAtomicIsCompareAndSwapU32(&visualization->hasVideoAudioChunk, 0, 1)) {
					visualization->videoAudioChunks[1].index = numFramesPlayed;
					visualization->videoAudioChunks[1].count = MAX_AUDIO_FRAMES_CHUNK_FRAMES;
					size_t chunkSamplesSize = frameSize * MAX_AUDIO_FRAMES_CHUNK_FRAMES;
					fplMemoryCopy(outputSamples, chunkSamplesSize, visualization->videoAudioChunks[1].samples);
					fplAtomicExchangeU32(&visualization->hasVideoAudioChunk, 2);
				}
				demo->lastVideoAudioChunkUpdateTime = fplMillisecondsQuery();
			}
		}
	}

#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY

	return(result);
}

static bool WriteAudioToRingBuffer(const fplAudioFormat *format, const uint32_t maxFrameCount, AudioDemo *demo, uint64_t *outDuration) {
	if(format == fpl_null || demo == fpl_null) return(false);
	if(maxFrameCount == 0) return(false);

	LockFreeRingBuffer *streamRingBuffer = &demo->outputRingBuffer;

	AudioBuffer *tmpBuffer = &demo->outputTempBuffer;

	size_t frameSize = fplGetAudioFrameSizeInBytes(format->type, format->channels);

	size_t availableStreamSpace = 0;
	bool canStreamWrite = LockFreeRingBufferCanWrite(streamRingBuffer, &availableStreamSpace);

	if(canStreamWrite && (availableStreamSpace % frameSize) == 0) {
		uint64_t timeStart = fplMillisecondsQuery();

		AudioFrameIndex numOfAvailableFrames = (AudioFrameIndex)fplMax(0, availableStreamSpace / frameSize);

		// Disable this condition to always fill the entire buffer
		//if(numOfAvailableFrames < maxFrameCount)
		//	return(false);

		AudioFrameIndex framesToWrite = fplMin(maxFrameCount, numOfAvailableFrames);

		size_t totalFrameBytes = framesToWrite * frameSize;
		fplAssert(totalFrameBytes <= tmpBuffer->bufferSize);

		// The amount of actual written frames may be less than the frames we want to get written
		AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, tmpBuffer->samples, format, framesToWrite, true);
		fplAssert(writtenFrames == framesToWrite);
		totalFrameBytes = writtenFrames * frameSize;

		// Write samples from temporary buffer to stream ring buffer
		bool streamWritten = LockFreeRingBufferWrite(streamRingBuffer, tmpBuffer->samples, totalFrameBytes);
		fplAssert(streamWritten);

		uint32_t lastNumFramesStreamed = fplAtomicFetchAndAddU32(&demo->numFramesStreamed, writtenFrames);

		if(outDuration != fpl_null) {
			uint64_t delta = fplMillisecondsQuery() - timeStart;
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

	LockFreeRingBuffer *circularBuffer = &demo->outputRingBuffer;

	// Depending on the performance we can jump in table rows instead of dynamically computing stuff?

	const uint64_t totalBufferLength = circularBuffer->length;

	const float minBufferThreshold = 0.25f; // In percentage range of 0 to 1
	const float maxBufferThreshold = 0.75f; // In percentage range of 0 to 1

	// @TODO(final): Move the file stream loading code into the audio demo

	bool ignoreWait = false;
	uint64_t startTime = fplMillisecondsQuery();
	while(!demo->isStreamingThreadStopped) {
		// Load source and play it if needed
		if(demo->trackList.changedPending) {
			fplAssert(demo->trackList.currentIndex >= 0 && demo->trackList.currentIndex < (int32_t)demo->trackList.count);
			AudioTrack *track = demo->trackList.tracks + demo->trackList.currentIndex;
			AudioTrackState state = (AudioTrackState)fplAtomicLoadS32(&track->state);
			if (state == AudioTrackState_AquireLoading) {
				// Loading file source and play it
				fplAssert(track->outputFullBuffer.bufferSize == 0);
				fplAssert(!track->outputFullBuffer.isAllocated);
				fplAssert(track->source.type != AudioTrackSourceType_None);

				fplAtomicStoreS32(&track->state, AudioTrackState_Loading);

				AudioSource *source = fpl_null;
				switch (track->source.type) {
					case AudioTrackSourceType_URL:
					{
						size_t urlOrFileLen = fplGetStringLength(track->source.url.urlOrFilePath);
						fplAssert(urlOrFileLen > 0);
						source = AudioSystemLoadFileSource(&demo->audioSys, track->source.url.urlOrFilePath);
					} break;

					case AudioTrackSourceType_Data:
					{
						source = AudioSystemLoadDataSource(&demo->audioSys, track->source.data.size, track->source.data.data);
					} break;

					default:
						FPL_NOT_IMPLEMENTED;
						break;
				}

				if (source != fpl_null) {
					AudioBuffer *fullAudioBuffer = &track->outputFullBuffer;

					// Mark as playing
					track->sourceID = source->id;
					fplAtomicStoreS32(&track->state, AudioTrackState_Ready);
					track->playID = AudioSystemPlaySource(&demo->audioSys, source, false, 1.0f);

					// Allocate full audio buffer and fully load the 
					AudioFrameIndex sourceFrameCount = source->buffer.frameCount;
					AudioFrameIndex targetFrameCount = fplGetTargetAudioFrameCount(sourceFrameCount, source->format.sampleRate, demo->targetAudioFormat.sampleRate);
					AudioFormat fullAudioBufferFormat = fplZeroInit;
					fullAudioBufferFormat.channels = demo->targetAudioFormat.channels;
					fullAudioBufferFormat.format = demo->targetAudioFormat.type;
					fullAudioBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
					AllocateAudioBuffer(&demo->audioSys.memory, fullAudioBuffer, &fullAudioBufferFormat, targetFrameCount);

					// Write the entire audio track into the full audio buffer, that is used for rendering the audio samples.
					// The actual playback uses the audio system again, but we can enable realtime (F1) to use the actual played back samples.
					if (fullAudioBuffer->bufferSize > 0) {
						AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, fullAudioBuffer->samples, &demo->targetAudioFormat, targetFrameCount, false);
						fplAssert(writtenFrames == targetFrameCount);
					}
				} else {
					fplAtomicStoreS32(&track->state, AudioTrackState_Failed);
				}
				demo->trackList.changedPending = false;
				startTime = fplMillisecondsQuery();
			} else if (state == AudioTrackState_Full) {
				fplAssert(track->outputFullBuffer.bufferSize == 0);
				fplAssert(!track->outputFullBuffer.isAllocated);
				fplAssert(track->sourceID.value > 0);

				AudioBuffer *fullAudioBuffer = &track->outputFullBuffer;
				
				AudioSource *source = AudioSystemGetSourceByID(&demo->audioSys, track->sourceID);
				if (source == fpl_null) {
					fplAlwaysAssert(!"Audio Source not found!");
					continue;
				}
			
				fplAtomicStoreS32(&track->state, AudioTrackState_Ready);
				track->playID = AudioSystemPlaySource(&demo->audioSys, source, false, 1.0f);

				// Allocate full audio buffer
				AudioFrameIndex sourceFrameCount = source->buffer.frameCount;
				AudioFrameIndex targetFrameCount = fplGetTargetAudioFrameCount(sourceFrameCount, source->format.sampleRate, demo->targetAudioFormat.sampleRate);
				AudioFormat fullAudioBufferFormat = fplZeroInit;
				fullAudioBufferFormat.channels = demo->targetAudioFormat.channels;
				fullAudioBufferFormat.format = demo->targetAudioFormat.type;
				fullAudioBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
				AllocateAudioBuffer(&demo->audioSys.memory, fullAudioBuffer, &fullAudioBufferFormat, targetFrameCount);

				// Write the entire audio track into the full audio buffer, that is used for rendering the audio samples.
				// The actual playback uses the audio system again, but we can enable realtime (F1) to use the actual played back samples.
				if (fullAudioBuffer->bufferSize > 0) {
					AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, fullAudioBuffer->samples, &demo->targetAudioFormat, targetFrameCount, false);
					fplAssert(writtenFrames == targetFrameCount);
				}
				
				demo->trackList.changedPending = false;
				startTime = fplMillisecondsQuery();
			} else {
				fplAlwaysAssert(!"Invalid code path!");
			}
		}

		// No audio track?
		if (!HasAudioTrack(&demo->trackList)) {
			fplThreadSleep(100);
			continue;
		}

		// Wait if needed
		bool wait = !ignoreWait;
		if(wait || !currentEntry.canIgnoreWait) {
			uint64_t deltaTime = fplMillisecondsQuery() - startTime;
			if(deltaTime < currentEntry.delay) {
				fplThreadSleep(1);
				continue;
			}
			startTime = fplMillisecondsQuery();
		}

		if(ignoreWait) {
			// We just want to ignore waiting once
			ignoreWait = false;
		}

		bool tooSlow = false;
		bool tooFast = false;

		uint64_t streamDuration = 0;
		if(WriteAudioToRingBuffer(&demo->targetAudioFormat, currentEntry.frames, demo, &streamDuration)) {
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

static void ReleaseStreamBuffers(AudioDemo *demo) {
	FreeAudioBuffer(&demo->audioSys.memory, &demo->outputTempBuffer);
	LockFreeRingBufferRelease(&demo->outputRingBuffer);
}

static bool InitializeStreamBuffers(AudioDemo *demo, AudioFrameIndex *frameCount) {
	bool bufferInitRes;

	// Init streaming buffer and read some frames at the very start
	AudioFrameIndex streamBufferFrames = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, 10000);
	size_t streamBufferSize = fplGetAudioBufferSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels, streamBufferFrames);
	bufferInitRes = LockFreeRingBufferInit(&demo->outputRingBuffer, streamBufferSize, true);
	if(!bufferInitRes) {
		return(false);
	}

	// Allocate temporary stream buffer
	AudioFormat streamTempBufferFormat = fplZeroInit;
	streamTempBufferFormat.channels = demo->targetAudioFormat.channels;
	streamTempBufferFormat.format = demo->targetAudioFormat.type;
	streamTempBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
	bufferInitRes = AllocateAudioBuffer(&demo->audioSys.memory, &demo->outputTempBuffer, &streamTempBufferFormat, streamBufferFrames);
	if(!bufferInitRes) {
		return(false);
	}

	*frameCount = streamBufferFrames;

	return(true);
}

static void ReleaseVisualization(AudioDemo *demo) {

}

static void FillFrequencyBins(const uint32_t binCount, const uint32_t sampleRate, double *bins) {
	const double nyquist = sampleRate * 0.5;
	fplAssert(binCount == fplArrayCount(AudibleFrequencyRanges));
	for (uint32_t i = 0; i < binCount; i++) {
		bins[i] = fplMin(AudibleFrequencyRanges[i], nyquist);
	}
}

static void GenerateFrequencyBins(const uint32_t binCount, const uint32_t sampleRate, double *bins) {
	const double nyquist = sampleRate * 0.5;
	const double minHearableFreq = 400.0;
    const double maxHearableFreq = 20000.0;
	const double minFreq = minHearableFreq;
	const double maxFreq = fplMin(maxHearableFreq, nyquist);
	const uint32_t N = binCount - 1;
	bins[0] = 0;
    for (uint32_t i = 0; i < N; i++) {
        bins[i] = minFreq * pow(maxFreq / minFreq, (double)i / N);
    }
	bins[N] = maxFreq;
}

static bool InitializeVisualization(AudioDemo *demo) {
	// Initialize frequency bins
	//FillFrequencyBins(MAX_AUDIO_BIN_COUNT, demo->targetAudioFormat.sampleRate, demo->visualization.bins);
	GenerateFrequencyBins(MAX_AUDIO_BIN_COUNT, demo->targetAudioFormat.sampleRate, demo->visualization.bins);

	// Init window coefficients
	uint32_t N = fplArrayCount(demo->visualization.fftInput);
	HammingWindowFunction(demo->visualization.windowCoeffs, N);

	return(true);
}

static void TestAudioMath() {
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(0, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(22050, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(0, 1000) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(22050, 1000) == 22050);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(44100, 1000) == 44100);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(48000, 1000) == 48000);

	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(0, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(0, 22050) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(22050, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(22050, 22050) == 1000);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(44100, 44100) == 1000);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(48000, 48000) == 1000);
}

static bool SetAudioTrackSourceFromFile(AudioSystem *audioSys, AudioTrackSource *track, const char *filePath) {
	PCMWaveFormat fileFormat = fplZeroInit;
	if (!AudioSystemLoadFileFormat(audioSys, filePath, &fileFormat)) {
		FPL_LOG_WARN("Demo", "Audio file '%s' is not unsupported!", filePath);
		return false;
	}
	if (!IsAudioSampleRateSupported(audioSys, fileFormat.samplesPerSecond)) {
		FPL_LOG_WARN("Demo", "Audio file '%s' cannot be converted from sample-rate '%u' to '%u'", filePath, fileFormat.samplesPerSecond, audioSys->targetFormat.sampleRate);
		return false;
	}
	const char *filename = fplExtractFileName(filePath);
	track->type = AudioTrackSourceType_URL;
	fplCopyString(filename, track->name, fplArrayCount(track->name));
	fplCopyString(filePath, track->url.urlOrFilePath, fplArrayCount(track->url.urlOrFilePath));
	FPL_LOG_INFO("Demo", "Audio file '%s' used with sample rate", filePath, fileFormat.samplesPerSecond);
	return true;
}

int main(int argc, char **args) {
	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char **files = (fileCount > 0) ? (const char **)args + 1 : fpl_null;
	bool forceSineWave = false;

    fplLogSettings logSettings = fplZeroInit;
    logSettings.maxLevel = fplLogLevel_All;
    logSettings.writers[0].flags = fplLogWriterFlags_DebugOut | fplLogWriterFlags_StandardConsole | fplLogWriterFlags_ErrorConsole;
    fplSetLogSettings(&logSettings);

	// Always sine-wave
	//fileCount = 0;
	//forceSineWave = true;

	AudioDemo *demo = (AudioDemo *)fplMemoryAllocate(sizeof(AudioDemo));
	demo->sineWave.frequency = 440;
	demo->sineWave.toneVolume = 0.25f;
	demo->sineWave.duration = 10.0;
	demo->useRealTimeSamples = true;

	int result = -1;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString(APP_TITLE, settings.window.title, fplArrayCount(settings.window.title));
	fplCopyString(APP_TITLE, settings.console.title, fplArrayCount(settings.console.title));

	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	settings.video.isVSync = true;

	// Set audio device format
	//settings.audio.targetFormat.type = fplAudioFormatType_S16;

	// Set number of channels
	settings.audio.targetFormat.channels = 2;
	settings.audio.targetFormat.channelLayout = fplAudioChannelLayout_Stereo;

	// Set samplerate in Hz
	//settings.audio.targetFormat.sampleRate = 11025;
	//settings.audio.targetFormat.sampleRate = 22050;
	settings.audio.targetFormat.sampleRate = 44100;
	//settings.audio.targetFormat.sampleRate = 48000;
	//settings.audio.targetFormat.sampleRate = 88200;

	// Optionally set buffer size in milliseconds or in frames
	//settings.audio.targetFormat.bufferSizeInMilliseconds = 16;
	//settings.audio.targetFormat.bufferSizeInFrames = 512;

	// Disable auto start/stop of audio playback
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	//
    // Setup default audio device
	//
	if(fplPlatformInit(fplInitFlags_Audio, &settings)) {
        uint32_t deviceCount = fplGetAudioDevices(0, 0, fpl_null);
        fplAudioDeviceInfo *audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * deviceCount);
        uint32_t loadedDeviceCount = fplGetAudioDevices(deviceCount, 0, audioDeviceInfos);
        fplAssert(loadedDeviceCount == deviceCount);
        if(loadedDeviceCount > 0) {
            const fplAudioDeviceInfo *defaultDeviceInfo = fpl_null;
            for(uint32_t deviceIndex = 0; deviceIndex < loadedDeviceCount; ++deviceIndex) {
                fplAudioDeviceInfo *audioDeviceInfo = audioDeviceInfos + deviceIndex;
                if (audioDeviceInfo->isDefault) {
                    FPL_LOG_INFO("Audio", "Found default audio device[%lu] %s\n", deviceIndex, audioDeviceInfo->name);
                }
            }
            if (defaultDeviceInfo != fpl_null)
                settings.audio.targetDevice = *defaultDeviceInfo;
        }
        fplMemoryFree(audioDeviceInfos);
		fplPlatformRelease();
	}

    AudioTrackSource audioTracks[8] = fplZeroInit;
    size_t audioTrackCount = 0;

    // Initialize the platform with audio enabled and the settings
	if(!fplPlatformInit(fplInitFlags_Video | fplInitFlags_Audio, &settings)) {
		goto done;
	}

    // Get hardware format
    fplGetAudioHardwareFormat(&demo->targetAudioFormat);

    // Overwrite the client read callback, so we can write samples to the sound device
    fplSetAudioClientReadCallback(AudioPlayback, demo);

	// Initialize audio system
	if(!AudioSystemInit(&demo->audioSys, &demo->targetAudioFormat)) {
		goto done;
	}

	// Initialze playback latency
	demo->maxPlaybackFrameLatency = demo->targetAudioFormat.bufferSizeInFrames / demo->targetAudioFormat.periods;

	// Load audio tracks
	// 
    // Only allow audio sources that have a sample rates that are even by the output sample rate!
    // Because we don't support non-even sample conversions, such as 48000 <-> 41000.
    if (fileCount > 0) {
        size_t maxTrackCount = fplArrayCount(audioTracks);
        for (size_t fileIndex = 0; fileIndex < fplMin(maxTrackCount, fileCount); ++fileIndex) {
			const char *filePath = files[fileIndex];
			AudioTrackSource *track = &audioTracks[audioTrackCount];
			if (SetAudioTrackSourceFromFile(&demo->audioSys, track, filePath)) {
				++audioTrackCount;
			}
        }
    } else if (IsAudioSampleRateSupported(&demo->audioSys, sampleRate_musicTavsControlArgofox)) {
        // Load default music (44100 Hz)
        AudioTrackSource *track = &audioTracks[audioTrackCount++];
        track->type = AudioTrackSourceType_Data;
        fplCopyString(name_musicTavsControlArgofox, track->name, fplArrayCount(track->name));
        track->data.size = sizeOf_musicTavsControlArgofox;
        track->data.data = ptr_musicTavsControlArgofox;
    }

	// Initialize OpenGL
	if(!fglLoadOpenGL(true))
		goto done;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH_HINT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	const fplSettings *currentSettings = fplGetCurrentSettings();

#if OPT_PLAYBACK == OPT_PLAYBACK_STREAMBUFFER
	AudioFrameIndex streamBufferFrames = 0;
	if(!InitializeStreamBuffers(demo, &streamBufferFrames)) {
		goto done;
	}

	if(!InitializeVisualization(demo)) {
		goto done;
	}

	if(!LoadAudioTrackList(&demo->audioSys, audioTracks, audioTrackCount, forceSineWave, &demo->sineWave, LoadAudioTrackFlags_None, &demo->trackList)) {
		goto done;
	}

	if(demo->trackList.count == 0) {
		goto done;
	}

	PlayAudioTrack(&demo->audioSys, &demo->trackList, 0);

#if 0
	// Stream in initial frames
	AudioFrameIndex initialFrameStreamCount = streamBufferFrames / 4;
	fplAssert(streamBufferFrames >= initialFrameStreamCount);
	WriteAudioToRingBuffer(&demo->targetAudioFormat, initialFrameStreamCount, demo, fpl_null);
#endif

	// Start streaming thread
	demo->streamingThread = fplThreadCreate(AudioStreamingThread, demo);
#endif

	// Start audio playback (This will start calling clientReadCallback regularly)
	if(fplPlayAudio() == fplAudioResultType_Success) {
		// Print output infos
		const char *outBackendName = fplGetAudioBackendName(currentSettings->audio.backend);
		const char *outFormatName = fplGetAudioFormatName(demo->audioSys.targetFormat.format);
		uint32_t outSampleRate = demo->audioSys.targetFormat.sampleRate;
		uint32_t outChannels = demo->audioSys.targetFormat.channels;
		fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n",
			demo->audioSys.playItems.count,
			outBackendName,
			outFormatName,
			outSampleRate,
			outChannels);

		const char *audioTrackName = audioTrackCount > 0 ? audioTracks[0].name : fpl_null;
        UpdateTitle(demo, audioTrackName, demo->useRealTimeSamples, 0.0);

		// Loop
		const double targetFrameFps = 1.0 / 60.0;
		double totalTime = 0.0;
		fplTimestamp lastTime = fplTimestampQuery();
        fplMilliseconds lastFpsTime = fplMillisecondsQuery();
        double currentFrameTime = 0.0;
        double currentFps = 0.0;
        uint64_t frameCount = 0;
		while(fplWindowUpdate()) {
			fplEvent ev;
			while(fplPollEvent(&ev)) {
				switch (ev.type) {
					case fplEventType_Keyboard:
					{
						if(ev.keyboard.type == fplKeyboardEventType_Button) {
							if(ev.keyboard.buttonState == fplButtonState_Release) {
								fplKey key = ev.keyboard.mappedKey;
								if(key == fplKey_F) {
									if(!fplIsWindowFullscreen())
										fplEnableWindowFullscreen();
									else
										fplDisableWindowFullscreen();
								} else if(key == fplKey_F1) {
									demo->useRealTimeSamples = !demo->useRealTimeSamples;
								}
                                UpdateTitle(demo, audioTrackName, demo->useRealTimeSamples, currentFps);
							}
						}
					} break;

					case fplEventType_Window:
					{
						if (ev.window.type == fplWindowEventType_DroppedFiles) {
							if (ev.window.dropFiles.fileCount > 0) {
								const char *newMediaTrack = ev.window.dropFiles.files[0];
								AudioTrackSource newTrack = fplZeroInit;

								StopAllAudioTracks(&demo->audioSys, &demo->trackList);

								AudioSystemClearSources(&demo->audioSys);

								LockFreeRingBufferClear(&demo->outputRingBuffer);
								fplMemoryClear(demo->outputTempBuffer.samples, demo->outputTempBuffer.bufferSize);

								fplAtomicExchangeU32(&demo->numFramesPlayed, 0);
								fplAtomicExchangeU32(&demo->numFramesStreamed, 0);

								ClearVisualization(demo);

								if (SetAudioTrackSourceFromFile(&demo->audioSys, &newTrack, newMediaTrack)) {
									audioTrackCount = 1;
									audioTracks[0] = newTrack;
									audioTrackName = audioTracks[0].name;
                                    UpdateTitle(demo, audioTrackName, demo->useRealTimeSamples, currentFps);
									if (LoadAudioTrackList(&demo->audioSys, audioTracks, audioTrackCount, forceSineWave, &demo->sineWave, LoadAudioTrackFlags_None, &demo->trackList)) {
										PlayAudioTrack(&demo->audioSys, &demo->trackList, 0);
									}
								}
							}
						}
					} break;

					default:
						break;
				}
			}

            fplWindowSize winSize = fplZeroInit;
            fplGetWindowSize(&winSize);

            Render(demo, winSize.width, winSize.height, totalTime);
			fplVideoFlip();

			fplTimestamp curTime = fplTimestampQuery();
            double frameTime = fplTimestampElapsed(lastTime, curTime);
            ++frameCount;
			totalTime += frameTime;

#if 0
			// Delay to limit the frame rate to 60 Hz
			if (frameTime < targetFrameFps) {
				double delta = targetFrameFps - frameTime;
				int ms = (int)(delta * 1000.0);
				if (ms > 0) {
					fplThreadSleep(ms);
				}
			}
#endif

            currentFps = (double)frameCount / totalTime;
            if( currentFps > 1000 ){
                currentFps = 0;
            }

            if (fplMillisecondsQuery() - lastFpsTime >= 1000) {
                UpdateTitle(demo, audioTrackName, demo->useRealTimeSamples, currentFps);
                lastFpsTime = fplMillisecondsQuery();
            }

			lastTime = fplTimestampQuery();
		}

		// Stop audio playback
		fplStopAudio();
	}

	result = 0;

done:
#if OPT_PLAYBACK == OPT_PLAYBACK_STREAMBUFFER
	// Wait for decoding thread to stop
	demo->isStreamingThreadStopped = 1;
	fplThreadWaitForOne(demo->streamingThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(demo->streamingThread);

	// Free and streaming buffers
	ReleaseStreamBuffers(demo);
#endif

    // Release visualization
	ReleaseVisualization(demo);

    // Release audio system
    StopAllAudioTracks(&demo->audioSys, &demo->trackList);
    AudioSystemShutdown(&demo->audioSys);

    // Shutdown OpenGL
	fglUnloadOpenGL();

	// Free demo memory
	if(demo) {
		fplMemoryFree(demo);
	}

	fplPlatformRelease();

	return(result);
}
