#ifndef FINAL_AUDIO_H
#define FINAL_AUDIO_H

#include <final_platform_layer.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h> // sin, cos, M_PI
#include <float.h> // EPSILON

/// The number of audio frames or index (32-bit)
typedef uint32_t AudioFrameIndex;

/// The number of audio samples or index (32-bit)
typedef uint32_t AudioSampleIndex;

/// The number of audio channels or index (16-bit)
typedef uint16_t AudioChannelIndex;

/// The audio frequency in Hertz (Hz) (32-bit)
typedef uint32_t AudioHertz;

/// The number of milliseconds for audio (32-bit)
typedef uint32_t AudioMilliseconds;

/// The duration in seconds for audio (64-bit)
typedef double AudioDuration;

/// The size of a audio buffer in bytes (32-bit or 64-bit)
typedef size_t AudioBufferSize; // The size in bytes

//! Stores the result of a audio resampling process.
typedef struct AudioResampleResult {
	//! The number of source frames that was processed.
	AudioFrameIndex inputCount;
	//! The number of output frames that was processed.
	AudioFrameIndex outputCount;
} AudioResampleResult;

typedef struct AudioStreamData {
	size_t size;
	const uint8_t *data;
} AudioStreamData;

#define AUDIO_STREAM_SEEK_ABSOLUTE_FUNC(name) size_t name(void *opaque, const intptr_t offset)
typedef AUDIO_STREAM_SEEK_ABSOLUTE_FUNC(AudioStreamSeekAbsoluteFunc);

#define AUDIO_STREAM_READ_FUNC(name) size_t name(void *opaque, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize)
typedef AUDIO_STREAM_READ_FUNC(AudioStreamReadFunc);

#define AUDIO_STREAM_GET_DATA_FUNC(name) AudioStreamData name(void *opaque)
typedef AUDIO_STREAM_GET_DATA_FUNC(AudioStreamGetDataFunc);

typedef struct AudioSystemStream {
	AudioStreamSeekAbsoluteFunc *seek;
	AudioStreamReadFunc *read;
	AudioStreamGetDataFunc *getData;
	size_t size;
	size_t pos;
	void *opaque;
} AudioSystemStream;

fpl_force_inline size_t AudioSystemStreamSeek(AudioSystemStream *stream, const intptr_t offset) {
	size_t pos = stream->seek(stream, offset);
	stream->pos = pos;
	return pos;
}

fpl_force_inline size_t AudioSystemStreamRead(AudioSystemStream *stream, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize) {
	size_t read = stream->read(stream, sizeToRead, targetBuffer, maxTargetBufferSize);
	stream->pos += read;
	return read;
}

fpl_force_inline AudioStreamData AudioSystemStreamGetData(AudioSystemStream *stream) {
	AudioStreamData result = stream->getData(stream);
	return result;
}

static AUDIO_STREAM_SEEK_ABSOLUTE_FUNC(AudioStreamFileSeekAbsolute) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	fplFileHandle *handle = (fplFileHandle *)stream->opaque;
	return fplFileSetPosition(handle, offset, fplFilePositionMode_Beginning);
}

static AUDIO_STREAM_READ_FUNC(AudioStreamFileRead) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	fplFileHandle *handle = (fplFileHandle *)stream->opaque;
	return fplFileReadBlock(handle, sizeToRead, targetBuffer, maxTargetBufferSize);
}

static AUDIO_STREAM_GET_DATA_FUNC(AudioStreamFileGetData) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	AudioStreamData result = fplZeroInit;
	return result;
}

static AudioSystemStream AudioStreamCreateFromFileHandle(fplFileHandle *file, const size_t size) {
	AudioSystemStream stream = fplZeroInit;
	stream.size = size;
	stream.pos = 0;
	stream.seek = AudioStreamFileSeekAbsolute;
	stream.read = AudioStreamFileRead;
	stream.getData = AudioStreamFileGetData;
	stream.opaque = file;
	return stream;
}

static AUDIO_STREAM_SEEK_ABSOLUTE_FUNC(AudioStreamDataSeekAbsolute) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	const uint8_t *data = (const uint8_t *)stream->opaque;
	if (offset >= 0 && (size_t)offset < stream->size) {
		// Nothing todo, stream pos is already set in the API function
		return offset;
	}
	return 0;
}

static AUDIO_STREAM_READ_FUNC(AudioStreamDataRead) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	const uint8_t *data = (const uint8_t *)stream->opaque;
	if (targetBuffer == fpl_null || maxTargetBufferSize < sizeToRead) {
		return 0;
	}
	if (stream->pos + sizeToRead > stream->size) {
		return 0;
	}
	const uint8_t *src = data + stream->pos;
	fplMemoryCopy(src, sizeToRead, targetBuffer);
	return sizeToRead;
}

static AUDIO_STREAM_GET_DATA_FUNC(AudioStreamDataGetData) {
	AudioSystemStream *stream = (AudioSystemStream *)opaque;
	AudioStreamData result = fplZeroInit;
	result.data = (const uint8_t *)stream->opaque;
	result.size = stream->size;
	return result;
}

static AudioSystemStream AudioStreamCreateFromData(const size_t size, const uint8_t *data) {
	AudioSystemStream stream = fplZeroInit;
	stream.size = size;
	stream.pos = 0;
	stream.seek = AudioStreamDataSeekAbsolute;
	stream.read = AudioStreamDataRead;
	stream.getData = AudioStreamDataGetData;
	stream.opaque = (void *)data;
	return stream;
}

typedef enum AudioFileFormat {
	AudioFileFormat_None = 0,
	AudioFileFormat_Wave,
	AudioFileFormat_Vorbis,
	AudioFileFormat_MP3,
} AudioFileFormat;

typedef struct AudioFormat {
	AudioHertz sampleRate;
	AudioChannelIndex channels;
	fplAudioFormatType format;
	uint8_t padding;
} AudioFormat;
fplStaticAssert(sizeof(AudioFormat) % 16 == 0);

typedef struct AudioBuffer {
	uint8_t *samples;
	AudioBufferSize bufferSize;
	AudioFrameIndex frameCount;
	fpl_b32 isAllocated;
} AudioBuffer;

typedef struct AudioStream {
	AudioBuffer buffer;
	AudioFrameIndex readFrameIndex;
	AudioFrameIndex framesRemaining;
} AudioStream;

#define MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT (AudioChannelIndex)FPL_MAX_AUDIO_CHANNEL_COUNT
#define MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT (AudioFrameIndex)4096
#define MAX_AUDIO_STATIC_BUFFER_MAX_TYPE_SIZE (size_t)4
typedef struct AudioStaticBuffer {
	uint8_t samples[MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT * MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT * MAX_AUDIO_STATIC_BUFFER_MAX_TYPE_SIZE];
	AudioFrameIndex maxFrameCount;
} AudioStaticBuffer;

typedef struct PCMWaveFormat {
	//! Total frame count
	uint32_t frameCount;
	//! Samples per second
	uint32_t samplesPerSecond;
	//! Bytes per sample
	uint32_t bytesPerSample;
	//! Format type
	fplAudioFormatType formatType;
	//! Number of channels
	uint16_t channelCount;
	// Padding 0
	uint16_t padding0;
} PCMWaveFormat;

typedef struct PCMWaveData {
	//! Format
	PCMWaveFormat format;
	//! Size of samples in bytes
	AudioBufferSize samplesSize;
	//! Samples (Interleaved)
	void* isamples;
	//! Last error string
	char lastError[976];
	//! Is valid boolean flag
	bool isValid;
} PCMWaveData;

#define AUDIO_MAX_CHANNEL_COUNT (AudioChannelIndex)16
#define AUDIO_MAX_SAMPLESIZE 4

static bool AreAudioBuffersEqual(AudioBuffer *a, AudioBuffer *b) {
	if(a == fpl_null || b == fpl_null)
		return(false);
	if(a->bufferSize != b->bufferSize)
		return(false);
	if(a->frameCount != b->frameCount)
		return(false);
	if(a->isAllocated != b->isAllocated)
		return(false);
	if(a->samples == fpl_null || b->samples == fpl_null)
		return(false);
	return(true);
}

static void FreeWaveData(PCMWaveData* wave) {
	if (wave != fpl_null) {
		if (wave->isamples != fpl_null) {
			fplMemoryFree(wave->isamples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

static void PushWaveError(PCMWaveData* outWave, const char* format, ...) {
	outWave->lastError[0] = 0;
	va_list argList;
	va_start(argList, format);
	fplStringFormatArgs(outWave->lastError, fplArrayCount(outWave->lastError), format, argList);
	va_end(argList);
}

#define FOURCC32(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

//
// Forward and Backward FFT
// Ported from: https://github.com/wareya/fft/blob/master/fft.hpp
// Original source cannot handle structs as in/out, so we had to port it (Why wasn`t there a stride).
//
typedef enum FFTDirection {
	FFTDirection_Forward = 0,
	FFTDirection_Backward = 1,
} FFTDirection;

typedef struct FFTDouble {
	double real;
	double imag;
} FFTDouble;

typedef struct FFT {
	FFTDouble* in;
	FFTDouble* out;
	AudioSampleIndex capacity;
	AudioSampleIndex size;
} FFT;

static void FFTCore(const FFTDouble* in, const size_t size, const size_t gap, FFTDouble* out, const FFTDirection direction) {
	if (size == 1) {
		out[0] = in[0];
	}
	else {
		FFTCore(in, size / 2, gap * 2, out, direction);
		FFTCore(&in[gap], size / 2, gap * 2, &out[size / 2], direction);
		double imagScale = (direction == FFTDirection_Forward) ? -1 : 1;
		for (size_t i = 0; i < size / 2; ++i) {
			// Terms
			double a_real = out[i].real;
			double a_imag = out[i].imag;
			double b_real = out[i + size / 2].real;
			double b_imag = out[i + size / 2].imag;
			double twiddle_real = cos(2 * M_PI * i / size);
			double twiddle_imag = sin(2 * M_PI * i / size) * imagScale;
			// Complex multiplication (vector angle summing and length multiplication)
			double bias_real = b_real * twiddle_real - b_imag * twiddle_imag;
			double bias_imag = b_imag * twiddle_real + b_real * twiddle_imag;
			// Real output (sum of real parts)
			out[i].real = a_real + bias_real;
			out[i + size / 2].real = a_real - bias_real;
			// Imaginary output (sum of imaginary parts)
			out[i].imag = a_imag + bias_imag;
			out[i + size / 2].imag = a_imag - bias_imag;
		}
	}
}

static void NormalizeFFT(FFTDouble* values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / (double)size;
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void HalfNormalizeFFT(FFTDouble* values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / sqrt((double)size);
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void ForwardFFT(const FFTDouble* in, const size_t size, FFTDouble* out) {
	FFTCore(in, size, 1, out, FFTDirection_Forward);
	HalfNormalizeFFT(out, size);
}

static void BackwardFFT(const FFTDouble* in, const size_t size, FFTDouble* out) {
	FFTCore(in, size, 1, out, FFTDirection_Backward);
	HalfNormalizeFFT(out, size);
}

inline bool FFTScalarEquals(const double a, const double b) {
	static const double FFT_EPSILON = 0.00001;
	return fabs(a - b) < FFT_EPSILON;
}

inline bool FFTDoubleEquals(const double expectedReal, const double expectedImag, const double actualReal, const double actualImag) {
	bool result = FFTScalarEquals(expectedReal, actualReal) && FFTScalarEquals(expectedImag, actualImag);
	return(result);
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
	FFTCore(dataIn, 8, 1, dataOut, FFTDirection_Forward);

#if 0
	fplAssert(FFTDoubleEquals(4.0, 0.0, dataOut[0].real, dataOut[0].imag));
	fplAssert(FFTDoubleEquals(1.0, -2.41421, dataOut[1].real, dataOut[1].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[2].real, dataOut[2].imag));
	fplAssert(FFTDoubleEquals(1.0, -0.414214, dataOut[3].real, dataOut[3].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[4].real, dataOut[4].imag));
	fplAssert(FFTDoubleEquals(1.0, 0.414214, dataOut[5].real, dataOut[5].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[6].real, dataOut[6].imag));
	fplAssert(FFTDoubleEquals(1.0, 2.41421, dataOut[7].real, dataOut[7].imag));
#endif

}

fpl_force_inline double AmplitudeToDecibel(const double amplitude) {
	return 20.0 * log10(amplitude);
}

fpl_force_inline double DecibelToAmplitude(const double dB) {
	return pow(20.0, dB / 20.0);
}

fpl_force_inline double DecibelToPower(const double dB, const double min, const double max) {
	// https://stackoverflow.com/a/9812267
	double range = max - min;
	double result;
	if(dB < min) {
		result = 0.0;
	} else {
		result = (dB - min) / range;
	}
	fplAssert(result >= 0.0 && result <= 1.0);
	return result;
}

static void WindowFunctionCore(double* output, const size_t length, const double a0, const double a1, const double a2, const double a3, const double a4) {
	if (output == fpl_null || length == 0) return;
	size_t N = length;
	if (N == 1) {
		output[0] = 1.0;
		return;
	}
	for (size_t index = 0; index <= N - 1; index++)
	{
		double k = 2.0 * M_PI * index / (double)N;
		output[index] = a0 - a1 * cos(k) + a2 * cos(2.0 * k) - a3 * cos(3.0 * k) + a4 * cos(4.0 * k);
	}
}

static void UniformWindowFunction(double* output, const size_t length) {
	double a0 = 1.0;
	double a1 = 0.0;
	double a2 = 0.0;
	double a3 = 0.0;
	double a4 = 0.0;
	WindowFunctionCore(output, length, a0, a1, a2, a3, a4);
}

static void HannWindowFunction(double* output, const size_t length) {
	double a0 = 0.5;
	double a1 = 0.5;
	double a2 = 0.0;
	double a3 = 0.0;
	double a4 = 0.0;
	WindowFunctionCore(output, length, a0, a1, a2, a3, a4);
}

static void HammingWindowFunction(double* output, const size_t length) {
	double a0 = 0.53836; // 25 / 46
	double a1 = 0.46164; // a1 = 1 - a0 = 21 / 46
	double a2 = 0.0;
	double a3 = 0.0;
	double a4 = 0.0;
	WindowFunctionCore(output, length, a0, a1, a2, a3, a4);
}

static void BlackmanWindowFunction(double* output, const size_t length) {
	double a0 = 0.42; // 21 / 50
	double a1 = 0.50; // 25 / 50
	double a2 = 0.08; //  4 / 50
	double a3 = 0.0;
	double a4 = 0.0;
	WindowFunctionCore(output, length, a0, a1, a2, a3, a4);
}

#endif // FINAL_AUDIO_H