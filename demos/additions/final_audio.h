#ifndef FINAL_AUDIO_H
#define FINAL_AUDIO_H

#include <final_platform_layer.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h> // sin, cos, M_PI

typedef uint32_t AudioFrameIndex; // The number or index of frames
typedef uint32_t AudioSampleIndex; // The number or index of samples
typedef uint32_t AudioChannelIndex; // The number or index of channels
typedef uint32_t AudioHertz; // The number of index of Hz
typedef size_t AudioSize; // The size in bytes

typedef struct PCMWaveData {
	//! Total frame count
	AudioFrameIndex frameCount;
	//! Samples per second (Frequency in Hz)
	AudioSampleIndex samplesPerSecond;
	//! Bytes per sample
	AudioSize bytesPerSample;
	//! Format type
	fplAudioFormatType formatType;
	//! Number of channels
	AudioChannelIndex channelCount;
	//! Size of samples in bytes
	AudioSize samplesSize;
	//! Interleaved samples (Max of 2 channels)
	void *samples;
	//! Last error string
	char lastError[1024];
	//! Is valid boolean flag
	bool isValid;
} PCMWaveData;

static void FreeWaveData(PCMWaveData *wave) {
	if (wave != fpl_null) {
		if (wave->samples != fpl_null) {
			fplMemoryFree(wave->samples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

static void PushWaveError(PCMWaveData *outWave, const char *format, ...) {

	outWave->lastError[0] = 0;
	va_list argList;
	va_start(argList, format);
	fplFormatStringArgs(outWave->lastError, fplArrayCount(outWave->lastError), format, argList);
	va_end(argList);
}

#define FOURCC32(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

//
// Forward and Backward FFT
//
typedef struct FFTDouble {
	double real;
	double imag;
} FFTDouble;

typedef struct FFT {
	FFTDouble *in;
	FFTDouble *out;
	AudioSampleIndex capacity;
	AudioSampleIndex size;
} FFT;

static void FFTCore(const FFTDouble *in, const size_t size, const size_t gap, FFTDouble *out, const bool isForward) {
	if (size == 1) {
		out[0] = in[0];
	} else {
		// This algorithm works by extending the concept of how two-bin DFTs (discrete fourier transform) work, in order to correlate decimated DFTs, recursively.
		// No, I'm not your guy if you want a proof of why it works, but it does.
		FFTCore(in, size / 2, gap * 2, out, isForward);
		FFTCore(&in[gap], size / 2, gap * 2, &out[size / 2], isForward);
		// non-combed decimated output to non-combed correlated output
		for (size_t i = 0; i < size / 2; ++i) {
			double a_real = out[i].real;
			double a_imag = out[i].imag;
			double b_real = out[i + size / 2].real;
			double b_imag = out[i + size / 2].imag;

			double twiddle_real = cos(2 * M_PI * i / size);
			double twiddle_imag = sin(2 * M_PI * i / size) * (isForward ? -1 : 1);
			// complex multiplication (vector angle summing and length multiplication)
			double bias_real = b_real * twiddle_real - b_imag * twiddle_imag;
			double bias_imag = b_imag * twiddle_real + b_real * twiddle_imag;
			// real output (sum of real parts)
			out[i].real = a_real + bias_real;
			out[i + size / 2].real = a_real - bias_real;
			// imag output (sum of imaginary parts)
			out[i].imag = a_imag + bias_imag;
			out[i + size / 2].imag = a_imag - bias_imag;
		}
	}
}

static void NormalizeFFT(FFTDouble *values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / (double)size;
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void HalfNormalizeFFT(FFTDouble *values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / sqrt((double)size);
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void ForwardFFT(const FFTDouble *in, const size_t size, const bool normalized, FFTDouble *out) {
	FFTCore(in, size, 1, out, true);
	if (normalized)
		HalfNormalizeFFT(out, size);
}

static void BackwardFFT(const FFTDouble *in, const size_t size, const bool normalized, FFTDouble *out) {
	FFTCore(in, size, 1, out, false);
	if (normalized)
		HalfNormalizeFFT(out, size);
}

static void FFTTest() {
	// This forward FFT without any normalization must return the following values from the input (1,1,1,1,0,0,0,0) with imaginary of zero
	// See https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B for more details.
	// 0 = {real=4.0000000000000000 imag=0.00000000000000000 }
	// 1 = {real=1.0000000000000000 imag=-2.4142135623730949 }
	// 2 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 3 = {real=1.0000000000000000 imag=-0.41421356237309492 }
	// 4 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 5 = {real=1.0000000000000000 imag=0.41421356237309515 }
	// 6 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 7 = {real=0.99999999999999967 imag=2.4142135623730949 }

	double data[8] = { 1,1,1,1,0,0,0,0 };

	FFTDouble dataIn[8];
	for (int i = 0; i < 8; ++i)
		dataIn[i] = fplStructInit(FFTDouble, data[i], 0.0);
	FFTDouble dataOut[8] = { 0 };
	FFTCore(dataIn, 8, 1, dataOut, true); // Forward
}

#endif // FINAL_AUDIO_H