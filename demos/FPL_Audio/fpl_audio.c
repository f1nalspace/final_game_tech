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
	Copyright (c) 2017-2020 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/
#define OPT_PLAYBACK_NONE 0
#define OPT_PLAYBACK_SINEWAVE_ONLY 1
#define OPT_PLAYBACK_AUDIOSYSTEM_ONLY 2

#define OPT_PLAYBACKMODE OPT_PLAYBACK_AUDIOSYSTEM_ONLY

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

#define FINAL_DEBUG_IMPLEMENTATION
#include <final_debug.h>

#include <final_audiodemo.h>

typedef enum WavePlotType {
	WavePlotType_None = 0,
	WavePlotType_WaveForm,
	WavePlotType_Lines,
	WavePlotType_Count,
} WavePlotType;

fpl_internal uint32_t NextPowerOfTwo(const uint32_t input) {
	uint32_t x = input;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return(x);
}
fpl_internal uint32_t PrevPowerOfTwo(const uint32_t input) {
	uint32_t result = NextPowerOfTwo(input) >> 1;
	return(result);
}

fpl_internal uint32_t RoundToPowerOfTwo(const uint32_t input) {
	if (fplIsPowerOfTwo(input))
		return(input);
	uint32_t result = NextPowerOfTwo(input);
	return(result);
}



typedef struct AudioDemo {
	AudioSystem audioSys;
	fplAudioFormatType sampleFormat;
	AudioSineWaveData sineWave;
	WavePlotType plotType;
	int32_t plotCount;
	bool enableFFT;
} AudioDemo;

// This code inside this disabled block are broken and are just there for ideas
#if 0

// https://github.com/miha53cevic/WitherMusic/blob/master/src/fft_extension/webAudioVisualiser.js
/*  We have N / 2 usable samples
	frequency = i * sampleRate / N
	where 0 <= i <= N/2
	- Note: the other samples are above 20KHz so they are useless for visualisation check this article
	- https://dsp.stackexchange.com/questions/38131/if-humans-can-only-hear-up-to-20-khz-frequency-sound-why-is-music-audio-sampled/38141

	48000 Hz / 2048 = 23,4375Hz per sample
	16 Band Table
	0  - 1  = 23.4375Hz  => 0Hz         - 23,4375Hz
	1  - 1  = 23.4375Hz  => 23.4375Hz   - 46.875Hz
	2  - 2  = 46.875Hz   => 46.875Hz    - 93.75Hz
	3  - 2  = 46.875Hz   => 93.75Hz     - 140.625Hz
	4  - 4  = 93.75Hz    => 140.625Hz   - 234.375Hz
	5  - 4  = 93.75Hz    => 234.375Hz   - 328.125Hz
	6  - 8  = 187.5Hz    => 328.125Hz   - 515.625Hz
	7  - 16 = 375Hz      => 515.625Hz   - 890.625Hz
	8  - 16 = 375Hz      => 890.625Hz   - 1265.625Hz
	9  - 32 = 750Hz      => 1265.625Hz  - 2015.625Hz
	10 - 32 = 750Hz      => 2015.625Hz  - 2765.625Hz
	11 - 32 = 750Hz      => 2765.625Hz  - 3515.625Hz
	12 - 64 = 1500Hz     => 3515.625Hz  - 5015.625Hz
	13 - 64 = 1500Hz     => 5015.625Hz  - 6515.625Hz
	14 - 128 = 3000Hz    => 6515.625Hz  - 9515.625Hz
	15 - 512 = 12000Hz   => 9515.625Hz  - 22031.25Hz
	918 samples used so we add the rest to the last band
*/

#define MAX_BAND_COUNT 32

typedef struct Spectrum {
	double frequencies[MAX_BAND_COUNT + 1];
	double currentPeaks[MAX_BAND_COUNT];
	double previousPeaks[MAX_BAND_COUNT];
	uint32_t count;
} Spectrum;

typedef struct AudioDemoBuffer {
	float *samples;
	size_t maxSizeBytes;
	AudioFrameIndex maxFrameCount;
	AudioFrameIndex writeFrameIndex;
	AudioFrameIndex readFrameIndex;
	volatile int32_t usedFrameCount;
} AudioDemoBuffer;

static void ConvertRenderSamples(AudioDemo *audioDemo, const void *inSamples, const fplAudioDeviceFormat *inFormat, const AudioFrameIndex frameCount) {
	AudioSystem *audioSys = &audioDemo->audioSys;

	const AudioChannelIndex outChannels = audioSys->targetFormat.channels;
	const size_t inSampleSize = fplGetAudioSampleSizeInBytes(inFormat->type);
	AudioHertz sampleRate = inFormat->sampleRate;

	AudioDemoBuffer *buffer = &audioDemo->buffer;

	uint32_t availableBufferFrames = fplMax(0, (int32_t)buffer->maxFrameCount - buffer->usedFrameCount);
	if (availableBufferFrames > 0) {
		float *samples = buffer->samples;
		AudioFrameIndex minFrameCount = fplMin(frameCount, availableBufferFrames);
		for (AudioFrameIndex frameIndex = 0; frameIndex < minFrameCount; ++frameIndex) {
			AudioFrameIndex outFrameIndex = (buffer->writeFrameIndex + frameIndex) % buffer->maxFrameCount;
			for (AudioChannelIndex channelIndex = 0; channelIndex < inFormat->channels; ++channelIndex) {
				AudioSampleIndex inSampleIndex = frameIndex * inFormat->channels;
				AudioSampleIndex outSampleIndex = outFrameIndex * outChannels + channelIndex;
				samples[outSampleIndex] = ConvertToF32((uint8_t *)inSamples + inSampleIndex * inSampleSize, channelIndex, inFormat->type);
			}
		}
		buffer->writeFrameIndex = (buffer->writeFrameIndex + minFrameCount) % buffer->maxFrameCount;
		fplAtomicAddAndFetchS32(&buffer->usedFrameCount, minFrameCount);
	}

#if 0
	AudioFrameIndex inputFrameIndex = 0;
	AudioFrameIndex remainingFrameCount = frameCount;
	while (remainingFrameCount > 0) {
		AudioFrameIndex remainingFramesInDemo = fplMax(0, (int32_t)(buffer->maxFrameCount - buffer->writeFrameIndex));
		AudioFrameIndex framesToCopy = remainingFrameCount;
		if (framesToCopy >= remainingFramesInDemo) {
			framesToCopy = remainingFramesInDemo;
		}
		AudioSampleIndex outputSampleIndex = buffer->writeFrameIndex * outChannels;
		AudioSampleIndex inputSampleIndex = inputFrameIndex * inFormat->channels;
		for (AudioFrameIndex frameIndex = 0; frameIndex < framesToCopy; ++frameIndex) {
			for (AudioChannelIndex channelIndex = 0; channelIndex < inFormat->channels; ++channelIndex) {
				buffer->samples[outputSampleIndex++] = ConvertToF32((uint8_t *)inSamples + inputSampleIndex * inSampleSize, channelIndex, inFormat->type);
			}
			inputSampleIndex += inFormat->channels;
		}
		inputFrameIndex += framesToCopy;
		remainingFrameCount -= framesToCopy;
		buffer->writeFrameIndex = (buffer->writeFrameIndex + framesToCopy) % buffer->maxFrameCount;
	}
	fplAtomicAddAndFetchS32(&buffer->usedFrameCount, frameCount);
#endif

	if (audioDemo->enableFFT) {
		uint32_t actualFrameCount = buffer->usedFrameCount;
		audioDemo->fft.size = RoundToPowerOfTwo(actualFrameCount);
		fplAssert(audioDemo->fft.size >= actualFrameCount);
		fplAssert(audioDemo->fft.size <= audioDemo->fft.capacity);

		// FFT
		for (AudioFrameIndex i = 0; i < actualFrameCount; ++i) {
			AudioFrameIndex frameIndex = (buffer->readFrameIndex + i) % buffer->maxFrameCount;
			AudioSampleIndex sampleIndex = frameIndex;
			float sampleFloat = buffer->samples[frameIndex * outChannels + 0];
			//float window = 0.54f - 0.46f * cosf(2.0f * (float)M_PI * frameIndex / (float)(N - 1)); // Hanning window
			float window = 1.0f;
			float amplitude = sampleFloat * sampleRate;
			audioDemo->fft.in[sampleIndex].real = amplitude * window;
			audioDemo->fft.in[sampleIndex].imag = 0.0f;
		}
		size_t zeroCount = audioDemo->fft.size - actualFrameCount;
		if (zeroCount > 0) {
			fplMemoryClear(&audioDemo->fft.in[actualFrameCount], sizeof(FFTDouble) * zeroCount);
		}
		ForwardFFT(audioDemo->fft.in, audioDemo->fft.size, audioDemo->fft.out);


#if 1
		for (uint32_t bandIndex = 0; bandIndex < (MAX_BAND_COUNT + 1); ++bandIndex) {
			audioDemo->spectrum.frequencies[bandIndex] = (float)bandIndex * (float)(sampleRate / 2) / (float)MAX_BAND_COUNT;
		}
		audioDemo->spectrum.count = MAX_BAND_COUNT;
#else
		// Compute frequency bins/bands
		// Sub-bass (20 Hz – 60 Hz)
		// Bass (60 Hz – 250 Hz)
		// Low midrange (250 Hz – 500 Hz)
		// Midrange (500 Hz – 2 000 Hz)
		// Upper midrange (2 000 Hz – 4 000 Hz)
		// Lower treble (4 000 Hz – 6 000 Hz)
		// Highs (6 000 Hz – 20 000 Hz)
		uint32_t bandNum = 0;
		audioDemo->spectrum.count = 0;
		audioDemo->spectrum.frequencies[bandNum++] = 20.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 60.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 100.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 150.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 200.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 250.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 500.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 2000.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 4000.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 6000.0f;
		audioDemo->spectrum.frequencies[bandNum++] = 20000.0f;
		audioDemo->spectrum.frequencies[bandNum++] = (float)sampleRate * 2.0f;
		audioDemo->spectrum.count = bandNum - 1;
#endif



		// Compute peak magnitudes
		uint32_t halfFFT = audioDemo->fft.size / 2;
		uint32_t bandCount = audioDemo->spectrum.count;
		for (uint32_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
			audioDemo->spectrum.currentPeaks[bandIndex] = FLT_MIN;
		}
		for (AudioFrameIndex fftIndex = 0; fftIndex < halfFFT; ++fftIndex) {
			double real = audioDemo->fft.out[fftIndex].real;
			double imag = audioDemo->fft.out[fftIndex].imag;
			double magnitude = sqrt(real * real + imag * imag);
			float freq = fftIndex * (float)sampleRate / (float)audioDemo->fft.size;
			for (uint32_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
				if ((freq > audioDemo->spectrum.frequencies[bandIndex]) && (freq <= audioDemo->spectrum.frequencies[bandIndex + 1])) {
					if (magnitude > audioDemo->spectrum.currentPeaks[bandIndex]) {
						audioDemo->spectrum.currentPeaks[bandIndex] = magnitude;
					}
				}
			}
		}

		// Interpolate peaks
		const float peakInterpolation = 1.0f;
		for (uint32_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
			double prev = audioDemo->spectrum.previousPeaks[bandIndex];
			double cur = audioDemo->spectrum.currentPeaks[bandIndex];
			audioDemo->spectrum.currentPeaks[bandIndex] = prev * (1.0 - peakInterpolation) + cur * peakInterpolation;
			audioDemo->spectrum.previousPeaks[bandIndex] = audioDemo->spectrum.currentPeaks[bandIndex];
		}
	}
}

static void ReleaseAudioVisualization() {
	if (demo->buffer.samples)
		fplMemoryAlignedFree(demo->buffer.samples);
	if (demo->fft.in)
		fplMemoryFree(demo->fft.in);
	if (demo->fft.out)
		fplMemoryFree(demo->fft.out);
	if (demo->spectrum.frequencies)
		fplMemoryFree(demo->spectrum.frequencies);
}

static void InitAudioVisualization() {
	const AudioFrameIndex audioFramesForVideoFrame = fplGetAudioBufferSizeInFrames(targetAudioFormat.sampleRate, targetFrameRateMs);

	// Allocate render samples
	uint32_t channelCount = demo->audioSys.targetFormat.channels;
	demo->sampleFormat = fplAudioFormatType_F32;
	demo->buffer.maxFrameCount = targetAudioFormat.bufferSizeInFrames * 2; // We hold frames for two render-frames -> one for current and one for next
	demo->buffer.maxSizeBytes = fplGetAudioBufferSizeInBytes(demo->sampleFormat, channelCount, demo->buffer.maxFrameCount);
	demo->buffer.samples = fplMemoryAlignedAllocate(demo->buffer.maxSizeBytes, 16);
	demo->buffer.readFrameIndex = 0;
	demo->buffer.writeFrameIndex = 0;

	demo->fft.capacity = RoundToPowerOfTwo(demo->buffer.maxFrameCount);
	demo->fft.in = fplMemoryAllocate(sizeof(*demo->fft.in) * demo->fft.capacity);
	demo->fft.out = fplMemoryAllocate(sizeof(*demo->fft.out) * demo->fft.capacity);
}

static void VisualizeAudio() {
	AudioDemoBuffer *buffer = &demo->buffer;
	if (buffer->usedFrameCount > 0) {
		//float *samples = buffer->samples + buffer->readFrameIndex * channelCount;
		float *samples = buffer->samples;

		AudioFrameIndex frameCount = buffer->usedFrameCount;

		const float paddingX = 20.0f;
		const float paddingY = 20.0f;
		const float spaceBetweenBars = 0.0f;
		const float maxRangeH = h - paddingY * 2.0f;
		const float maxRangeW = w - paddingX * 2.0f;
		const float halfH = h * 0.5f;

		AudioChannelIndex channelIndex = 0;

#if 0
		float rahmenWidth = maxRangeW;
		float rahmenHeight = h * 0.25f;
		float rahmenX = paddingX;
		float rahmenY = paddingY;
		float rahmenSectionWidth = rahmenWidth / (float)buffer->maxFrameCount;

		RenderRectangle(rahmenX, rahmenY, rahmenX + rahmenWidth, rahmenX + rahmenHeight, (Color4f) { 1, 1, 1, 1 }, 1);

		for (AudioFrameIndex frameIndex = 0; frameIndex < buffer->maxFrameCount; ++frameIndex) {
			float x = rahmenX + frameIndex * rahmenWidth / (float)buffer->maxFrameCount;
			float y = rahmenY;
			RenderLine(x, y, x, y + rahmenHeight, (Color4f) { 0.1f, 0.1f, 0.1f, 1 }, 0.5f);
		}
		{
			float x = rahmenX + buffer->writeFrameIndex * rahmenWidth / (float)buffer->maxFrameCount;
			float y = rahmenY;
			RenderLine(x, y, x, y + rahmenHeight, (Color4f) { 1, 0, 0, 1 }, 5);

			x = rahmenX + buffer->readFrameIndex * rahmenWidth / (float)buffer->maxFrameCount;
			y = rahmenY;
			RenderLine(x, y, x, y + rahmenHeight, (Color4f) { 0, 0, 1, 1 }, 5);
		}
#endif

		if (demo->plotType == WavePlotType_WaveForm) {
			const uint32_t barCount = demo->plotCount;
			const float barW = (maxRangeW - spaceBetweenBars * (float)(barCount - 1)) / (float)barCount;
			const float maxBarH = h;
			Color4f color = (Color4f) { 0.5,0.5,0.5,1 };
			for (uint32_t barIndex = 0; barIndex < barCount; ++barIndex) {
				float t = barIndex / (float)barCount;
				AudioFrameIndex frameIndex = ((AudioFrameIndex)(t * frameCount + 0.5f) + buffer->readFrameIndex) % buffer->maxFrameCount;
				AudioSampleIndex sampleIndex = frameIndex * channelCount;
				float sampleValue = samples[sampleIndex + channelIndex];
				float barH = maxBarH * sampleValue;
				float posX = paddingX + barIndex * barW + barIndex * spaceBetweenBars;
				float posY = halfH - barH * 0.5f;
				RenderRectangle(posX, posY, posX + barW, posY + barH, color, 1);
			}
		} else if (demo->plotType == WavePlotType_Lines) {
			const uint32_t pointCount = demo->plotCount;
			const float lineWidth = maxRangeW / (float)(pointCount - 1);
			float firstSampleValue = samples[0 * channelCount + 0];
			float startX = paddingX;
			float startY = halfH + (firstSampleValue * halfH);
			Color4f color = (Color4f) { 0.5,0.5,0.5,1 };
			for (uint32_t pointIndex = 1; pointIndex < pointCount; ++pointIndex) {
				float t = pointIndex / (float)pointCount;
				AudioFrameIndex frameIndex = ((AudioFrameIndex)(t * (float)frameCount + 0.5f) + buffer->readFrameIndex) % buffer->maxFrameCount;
				AudioSampleIndex sampleIndex = frameIndex * channelCount;
				float sampleValue = samples[sampleIndex + channelIndex];
				float targetX = startX + lineWidth;
				float targetY = halfH + (sampleValue * halfH);
				RenderLine(startX, startY, targetX, targetY, color, 1.0f);
				startX = targetX;
				startY = targetY;
			}
		}

		buffer->readFrameIndex = (buffer->readFrameIndex + frameCount) % buffer->maxFrameCount;
		fplAtomicAddAndFetchS32(&buffer->usedFrameCount, -(int32_t)frameCount);
		fplAssert(buffer->usedFrameCount >= 0);

		if (demo->enableFFT) {
			//
			// Build spectrum bands
			//
			const float bandSpacing = 5; // 5-pixels between each band
			int N = demo->fft.size;
			uint32_t halfFFT = N / 2;
#if 1
			float bandDimensionW = w - paddingX * 2.0f;
			float bandDimensionH = h * 0.5f - paddingY;
			const uint32_t barCount = demo->plotCount;
			//const float barW = (maxRangeW - spaceBetweenBars * (float)(barCount - 1)) / (float)barCount;
			const Color4f barColor = (Color4f) { 0,1,0,1 };
			float barW = maxRangeW / (float)halfFFT;
			for (uint32_t fftIndex = 0; fftIndex < halfFFT; ++fftIndex) {
				double real = demo->fft.out[fftIndex].real;
				double imag = demo->fft.out[fftIndex].imag;
				double mag = sqrt(real * real + imag * imag);
				float barH = (float)mag * bandDimensionH;
				float posX = paddingX + fftIndex * barW;
				float posY = h - paddingY;
				RenderRectangle(posX, posY, posX + barW, posY - barH, barColor, 1);
			}
#else
			uint32_t bandCount = demo->spectrum.count;
			float bandSpacingDimension = bandSpacing * (float)(bandCount - 1);
			float bandDimensionW = w - paddingX * 2.0f;
			float bandDimensionH = h * 0.5f - paddingY;
			float bandStepWidth = bandDimensionW / (float)bandCount;
			float bandWidth = (bandDimensionW - bandSpacingDimension) / (float)bandCount;
			for (uint32_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
				float mag = demo->spectrum.currentPeaks[bandIndex];
				float bandH = (float)mag / (float)outSampleRate * bandDimensionH;
				float bandX = paddingX + (float)bandIndex * bandStepWidth;
				float bandY = h - paddingY;
				int color = 0xFF00FF00;
				BackbufferDrawRect(backBuffer, bandX, bandY, bandX + bandWidth, bandY - bandH, color);
			}
#endif
		}
	}
}

#endif

static uint32_t AudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	double timeStart = fplGetTimeInMillisecondsHP();

	AudioDemo *audioDemo = (AudioDemo *)userData;

	AudioFrameIndex result = 0;

#if OPT_PLAYBACKMODE == OPT_PLAYBACK_SINEWAVE_ONLY
	result = maxFrameCount;
	AudioGenerateSineWave(&audioDemo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
#endif // OPT_OUTPUT_SAMPLES_SINEWAVE_ONLY


#if OPT_PLAYBACKMODE == OPT_PLAYBACK_AUDIOSYSTEM_ONLY
	// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
	AudioFrameIndex frameCount = AudioSystemWriteSamples(&audioDemo->audioSys, outputSamples, outFormat, maxFrameCount);
	result = frameCount;
#endif // OPT_OUTPUT_SAMPLES_SYSTEM_ONLY

	double timeEnd = fplGetTimeInMillisecondsHP();
	double actualTime = timeEnd - timeStart;

	// Print error when its too slow
	uint64_t frameDelay = 50;
	uint64_t minFrames = outFormat->bufferSizeInFrames / outFormat->periods;
	uint64_t requiredFrames = minFrames - frameDelay;
	double maxTime = 1.0 / (double)requiredFrames;
	double missedTime = fplMax(0.0, actualTime - maxTime);

	if (missedTime > 0) {
		double missRate = missedTime / maxTime * 100.0;
		fplDebugFormatOut("ERROR: Audio playback too slow, available time: %.6f, actual time: %.6f, missed time: %.6f, missed rate: %.2f %s\n", maxTime, actualTime, missedTime, missRate, "%");
	}

	return(result);
}

static const char *MapPlotTypeToString(WavePlotType plotType) {
	switch (plotType) {
		case WavePlotType_WaveForm:
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

typedef union Color4f {
	struct {
		float r;
		float g;
		float b;
		float a;
	};
	float m[4];
} Color4f;

static void RenderRectangle(const float x0, const float y0, const float x1, const float y1, const Color4f color, const float lineWidth) {
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

static void RenderLine(const float x0, const float y0, const float x1, const float y1, const Color4f color, const float lineWidth) {
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
}

int main(int argc, char **args) {


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
	demo->plotType = WavePlotType_WaveForm;
	demo->plotCount = 512;
	demo->enableFFT = true;

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
	if (!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		goto done;
	}

	const uint32_t targetFrameRateMs = (uint32_t)ceil(1000.0 / 60.0);

	const uint32_t maxAudioDeviceCount = 64;
	fplAudioDeviceInfo *audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * maxAudioDeviceCount);
	uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, maxAudioDeviceCount);
	if (deviceCount > 0) {
		// Use first audio device in settings
		settings.audio.targetDevice = audioDeviceInfos[0];
		// @TODO(final): Fix weird space after line break
		fplConsoleFormatOut("Using audio device: '%s'\n", settings.audio.targetDevice.name);
	}
	fplMemoryFree(audioDeviceInfos);
	fplPlatformRelease();

	// Initialize the platform with audio enabled and the settings
	if (!fplPlatformInit(fplInitFlags_All, &settings)) {
		goto done;
	}

	if (!fglLoadOpenGL(true))
		goto done;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH_HINT);

	fplAudioDeviceFormat targetAudioFormat = fplZeroInit;
	fplGetAudioHardwareFormat(&targetAudioFormat);

	// You can overwrite the client read callback and user data if you want to
	fplSetAudioClientReadCallback(AudioPlayback, demo);

	const fplSettings *currentSettings = fplGetCurrentSettings();

	// Init audio data
	if (InitAudioData(&targetAudioFormat, &demo->audioSys, files, fileCount, forceSineWave, &demo->sineWave)) {

#if 0

#endif

		// Start audio playback (This will start calling clientReadCallback regulary)
		if (fplPlayAudio() == fplAudioResultType_Success) {
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
									demo->plotType = (demo->plotType + 1) % WavePlotType_Count;
								} else if (key == fplKey_F) {
									demo->enableFFT = !demo->enableFFT;
								} else if (key == fplKey_Add) {
									demo->plotCount *= 2;
									if (demo->plotCount > 2048) {
										demo->plotCount = 2048;
									}
								} else if (key == fplKey_Substract) {
									demo->plotCount /= 2;
									if (demo->plotCount < 8) {
										demo->plotCount = 8;
									}
								}
								UpdateTitle(demo);
							}
						}
					}
				}

				// Audio
				if (nextSamples) {
#if 0
					AudioFrameIndex framesToRender = targetAudioFormat.bufferSizeInFrames / targetAudioFormat.periods;
					// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
					AudioFrameIndex writtenFrames = AudioSystemWriteSamples(&audioSys, tempAudioSamples, &targetAudioFormat, framesToRender);
					AudioRenderSamples(&demo, tempAudioSamples, &targetAudioFormat, writtenFrames);
#endif
					nextSamples = false;
				}

				fplWindowSize winSize = fplZeroInit;
				fplGetWindowSize(&winSize);
				Render(demo, winSize.width, winSize.height);
				fplVideoFlip();
			}

			// Stop audio playback
			fplStopAudio();
		}

		// Release audio data
		AudioSystemShutdown(&demo->audioSys);
	}

	ReleaseDebug();

	result = 0;

done:
	fglUnloadOpenGL();

	fplPlatformRelease();

	if (demo)
		fplMemoryFree(demo);

	return(result);
}