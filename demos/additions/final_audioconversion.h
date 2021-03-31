#ifndef FINAL_AUDIO_CONVERSION_H
#define FINAL_AUDIO_CONVERSION_H

#define __INT24_TYPE__

#include <final_audio.h>
#include <final_math.h>

typedef void(AudioSampleFormatConversionFunc)(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples);
typedef void(AudioSampleDeinterleaveFunc)(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void *inSamples, void **outSamples);
typedef void(AudioSampleInterleaveFunc)(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void **inSamples, void *outSamples);

typedef struct AudioSampleConversionFunctions {
	AudioSampleFormatConversionFunc *convU8ToF32;
	AudioSampleFormatConversionFunc *convS16ToF32;
	AudioSampleFormatConversionFunc *convS24ToF32;
	AudioSampleFormatConversionFunc *convS32ToF32;
	AudioSampleFormatConversionFunc *convF32ToU8;
	AudioSampleFormatConversionFunc *convF32ToS16;
	AudioSampleFormatConversionFunc *convF32ToS24;
	AudioSampleFormatConversionFunc *convF32ToS32;

	AudioSampleInterleaveFunc *interleaveU8;
	AudioSampleInterleaveFunc *interleaveS16;
	AudioSampleInterleaveFunc *interleaveS32;
	AudioSampleInterleaveFunc *interleaveF32;

	AudioSampleDeinterleaveFunc *deinterleaveU8;
	AudioSampleDeinterleaveFunc *deinterleaveS16;
	AudioSampleDeinterleaveFunc *deinterleaveS32;
	AudioSampleDeinterleaveFunc *deinterleaveF32;

	AudioSampleFormatConversionFunc *conversionTable[fplAudioFormatType_Last][fplAudioFormatType_Last];
	AudioSampleInterleaveFunc *interleaveTable[fplAudioFormatType_Last];
	AudioSampleDeinterleaveFunc *deinterleaveTable[fplAudioFormatType_Last];
} AudioSampleConversionFunctions;


extern AudioSampleConversionFunctions CreateAudioSamplesConversionFunctions();

extern bool AudioSamplesConvert(AudioSampleConversionFunctions *funcTable, const AudioSampleIndex numSamples, const fplAudioFormatType inFormat, const fplAudioFormatType outFormat, void *inSamples, void *outSamples);

extern bool AudioSamplesDeinterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, void *inSamples, void **outSamples);

extern bool AudioSamplesInterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, void **inSamples, void *outSamples);

extern bool IsAudioDeinterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void **a, const void **b);
extern bool IsAudioInterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void *a, const void *b);

extern void TestAudioSamplesSuite();

#endif // FINAL_AUDIO_CONVERSION_H

#if (defined(FINAL_AUDIO_CONVERSION_IMPLEMENTATION) || defined(FPL_IS_IDE)) && !defined(FINAL_AUDIO_CONVERSION_IMPLEMENTED)
#define FINAL_AUDIO_CONVERSION_IMPLEMENTED

// **********************************************************************************************************************
// Type-Conversion: U8 <-> F32 | S16 <-> F32 | S24 <-> F32 | S32 <-> F32
// TODO: U8 <-> S16 | S8 <-> S24 | S8 <-> S32
// TODO: S16 <-> S24 | S16 <-> S32
// TODO: S24 <-> S32
// **********************************************************************************************************************

const uint32_t AUDIO_INT24_MAX = 8388607; // Math is 608, due to signed support we use 607

//! Converts 8-bit unsigiend integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_U8ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const uint8_t *inU8 = (const uint8_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float invU8 = 1.0f / (float)UINT8_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		float x = (float)inU8[sampleIndex];
		x *= invU8;	// Scale into float range of 0.0 to 1.0
		x *= 2.0f;	// Scale into range of 0.0 to 2.0
		x -= 1.0f;	// Subtract one to get into range of -1.0 to 1.0
		outF32[sampleIndex] = x;
	}
}

//! Converts 32-bit floating point samples to 8-bit unsigned integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToU8_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	uint8_t *outU8 = (uint8_t *)outSamples;
	const float fm = (float)UINT8_MAX;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];
		x = fplMax(-1.0f, fplMin(1.0f, x)); // Clip to -1.0 and 1.0
		x += 1.0f;							// Get into range of 0.0 to 2.0
		x *= 0.5f;							// Divide by half to get into range 0.0 to 1.0
		x *= fm;							// Scale to 0 to 255
		outU8[i] = (uint8_t)x;
	}
}

//! Converts 16-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S16ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const int32_t *inS32 = (const int32_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const double invS32 = 1.0f / (double)INT32_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		double x = (double)inS32[sampleIndex];
		x *= invS32; // Scale into float range
		outF32[sampleIndex] = (float)x;
	}
}

//! Converts 32-bit floating point samples to 16-bit integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToS16_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	int32_t *outS32 = (int32_t *)outSamples;
	const float fm = (float)INT32_MAX;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];
		x = fplMax(-1.0f, fplMin(1.0f, x)); // Clip to -1.0 and 1.0
		x *= fm;							// Scale into int32_t range
		outS32[i] = (int32_t)x;
	}
}

//! Converts 24-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S24ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const uint8_t *inS24 = (const uint8_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float invMax24 = 1.0f / (float)AUDIO_INT24_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		// Read three 8-bit input samples
		uint8_t a = inS24[sampleIndex * 3 + 0];
		uint8_t b = inS24[sampleIndex * 3 + 1];
		uint8_t c = inS24[sampleIndex * 3 + 2];

		// Convert the three 8-bit samples to a 32-bit sample value
		int32_t value24 = (int32_t)(((uint32_t)a << 8) | ((uint32_t)b << 16) | ((uint32_t)b << 24));

		// Move 8-bit forward to leave the first 8-bits as zero
		value24 = value24 >> 8;

		// Scale 24-bit integer to float space
		float x = (float)value24 * invMax24;

		outF32[sampleIndex] = x;
	}
}


//! Converts 32-bit floating point samples to 24-bit integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToS24_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	uint8_t *outS24 = (uint8_t *)outSamples;
	const float max24f = (float)AUDIO_INT24_MAX;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];
		x = fplMax(-1.0f, fplMin(1.0f, x)); // Clip to -1.0 and 1.0
		x *= max24f;						// Scale to 24-bit
		int32_t value24 = (int32_t)x;		// Convert to int32

		// Extract the three 8-bits
		uint8_t a = (uint8_t)((value24 & 0x0000FF) >> 0);
		uint8_t b = (uint8_t)((value24 & 0x00FF00) >> 8);
		uint8_t c = (uint8_t)((value24 & 0xFF0000) >> 16);

		outS24[i * 3 + 0] = a;
		outS24[i * 3 + 1] = b;
		outS24[i * 3 + 2] = c;
	}
}

//! Converts 32-bit floating point samples to 32-bit integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToS32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	int32_t *outS32 = (int32_t *)outSamples;
	const float fm = (float)INT32_MAX;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];
		x = fplMax(-1.0f, fplMin(1.0f, x)); // Clip to -1.0 and 1.0
		x *= fm;							// Scale into int32_t range
		outS32[i] = (int32_t)x;
	}
}

//! Converts 32-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S32ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const int32_t *inS32 = (const int32_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const double invS32 = 1.0f / (double)INT32_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		double x = (double)inS32[sampleIndex];
		x *= invS32; // Scale into float range
		outF32[sampleIndex] = (float)x;
	}
}

// **********************************************************************************************************************
// Interleave <-> Deinterleave samples conversion: U8 - S16 - S32 - F32
//
// Interleaved (One-dimensional):
// Samples = [LL][RR] [LL][RR] [LL][RR] [LL][RR] [LL][RR]
//
// De-Interleaved (Two dimensional):
// Samples[0] = [LL] [LL] [LL] [LL] [LL]
// Samples[1] = [RR] [RR] [RR] [RR] [RR]
// ...
//
// We require interleaved samples to process samples for each channel separately, this is much more cache friendly,
// also this makes conversion much easier
//
// **********************************************************************************************************************

//! Converts 8-bit unsigned integer interleaved samples to deinterleaved 8-bit unsigned integer samples by the specified frame count and channel count
static void AudioSamples_Deinterleave_U8_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void *inSamples, void **outSamples) {
	const uint8_t *inU8 = (const uint8_t *)inSamples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
			uint8_t *outU8 = (uint8_t *)outSamples[channelIndex];
			outU8[frameIndex] = inU8[frameIndex * channelCount + channelIndex];
		}
	}
}
//! Converts 8-bit unsigned integer deinterleaved samples to interleaved 8-bit unsigned integer samples by the specified frame count and channel count
static void AudioSamples_Interleave_U8_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void **inSamples, void *outSamples) {
	uint8_t *outU8 = (uint8_t *)outSamples;
	for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
		const uint8_t *inU8 = (const uint8_t *)inSamples[channelIndex];
		for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			outU8[frameIndex * channelCount + channelIndex] = inU8[frameIndex];
		}
	}
}

//! Converts 16-bit integer interleaved samples to deinterleaved 16-bit integer samples by the specified frame count and channel count
static void AudioSamples_Deinterleave_S16_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void *inSamples, void **outSamples) {
	const int16_t *inS16 = (const int16_t *)inSamples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
			int16_t *outS16 = (int16_t *)outSamples[channelIndex];
			outS16[frameIndex] = inS16[frameIndex * channelCount + channelIndex];
		}
	}
}
//! Converts 16-bit integer deinterleaved samples to interleaved 16-bit integer samples by the specified frame count and channel count
static void AudioSamples_Interleave_S16_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void **inSamples, void *outSamples) {
	int16_t *outS16 = (int16_t *)outSamples;
	for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
		const int16_t *inS16 = (const int16_t *)inSamples[channelIndex];
		for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			outS16[frameIndex * channelCount + channelIndex] = inS16[frameIndex];
		}
	}
}

//! Converts 32-bit integer interleaved samples to deinterleaved 32-bit integer samples by the specified frame count and channel count
static void AudioSamples_Deinterleave_S32_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void *inSamples, void **outSamples) {
	const int32_t *inS32 = (const int32_t *)inSamples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
			int32_t *outS32 = (int32_t *)outSamples[channelIndex];
			outS32[frameIndex] = inS32[frameIndex * channelCount + channelIndex];
		}
	}
}
//! Converts 32-bit integer deinterleaved samples to interleaved 32-bit integer samples by the specified frame count and channel count
static void AudioSamples_Interleave_S32_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void **inSamples, void *outSamples) {
	int32_t *outS32 = (int32_t *)outSamples;
	for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
		const int32_t *inS32 = (const int32_t *)inSamples[channelIndex];
		for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			outS32[frameIndex * channelCount + channelIndex] = inS32[frameIndex];
		}
	}
}

//! Converts 32-bit float point interleaved samples to deinterleaved 32-bit float point samples by the specified frame count and channel count
static void AudioSamples_Deinterleave_F32_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void *inSamples, void **outSamples) {
	const float *inF32 = (const float *)inSamples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
			float *outF32 = (float *)outSamples[channelIndex];
			outF32[frameIndex] = inF32[frameIndex * channelCount + channelIndex];
		}
	}
}
//! Converts 32-bit floating point deinterleaved samples to interleaved 32-bit floating point samples by the specified frame count and channel count
static void AudioSamples_Interleave_F32_Default(const AudioFrameIndex frameCount, const AudioChannelIndex channelCount, const void **inSamples, void *outSamples) {
	float *outF32 = (float *)outSamples;
	for(AudioChannelIndex channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
		const float *inF32 = (const float *)inSamples[channelIndex];
		for(AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			outF32[frameIndex * channelCount + channelIndex] = inF32[frameIndex];
		}
	}
}

// **********************************************************************************************************************
// Function tables
// **********************************************************************************************************************

extern AudioSampleConversionFunctions CreateAudioSamplesConversionFunctions() {
	AudioSampleConversionFunctions result = fplZeroInit;

	// @TODO(final): SIMD detection and simply change the function pointer to right one with best SIMD implementation

	// Conversion functions
	result.conversionTable[fplAudioFormatType_U8][fplAudioFormatType_F32] = result.convU8ToF32 = AudioSamples_Convert_U8ToF32_Default;
	result.conversionTable[fplAudioFormatType_S16][fplAudioFormatType_F32] = result.convS16ToF32 = AudioSamples_Convert_S16ToF32_Default;
	result.conversionTable[fplAudioFormatType_S24][fplAudioFormatType_F32] = result.convS24ToF32 = AudioSamples_Convert_S24ToF32_Default;
	result.conversionTable[fplAudioFormatType_S32][fplAudioFormatType_F32] = result.convS32ToF32 = AudioSamples_Convert_S32ToF32_Default;

	result.conversionTable[fplAudioFormatType_F32][fplAudioFormatType_U8] = result.convF32ToU8 = AudioSamples_Convert_F32ToU8_Default;
	result.conversionTable[fplAudioFormatType_F32][fplAudioFormatType_S16] = result.convF32ToS16 = AudioSamples_Convert_F32ToS16_Default;
	result.conversionTable[fplAudioFormatType_F32][fplAudioFormatType_S24] = result.convF32ToS24 = AudioSamples_Convert_F32ToS24_Default;
	result.conversionTable[fplAudioFormatType_F32][fplAudioFormatType_S32] = result.convF32ToS32 = AudioSamples_Convert_F32ToS32_Default;

	// Interleave functions
	result.interleaveTable[fplAudioFormatType_U8] = result.interleaveU8 = AudioSamples_Interleave_U8_Default;
	result.interleaveTable[fplAudioFormatType_S16] = result.interleaveS16 = AudioSamples_Interleave_S16_Default;
	result.interleaveTable[fplAudioFormatType_S32] = result.interleaveS32 = AudioSamples_Interleave_S32_Default;
	result.interleaveTable[fplAudioFormatType_F32] = result.interleaveF32 = AudioSamples_Interleave_F32_Default;

	// Deinterleave functions
	result.deinterleaveTable[fplAudioFormatType_U8] = result.deinterleaveU8 = AudioSamples_Deinterleave_U8_Default;
	result.deinterleaveTable[fplAudioFormatType_S16] = result.deinterleaveS16 = AudioSamples_Deinterleave_S16_Default;
	result.deinterleaveTable[fplAudioFormatType_S32] = result.deinterleaveS32 = AudioSamples_Deinterleave_S32_Default;
	result.deinterleaveTable[fplAudioFormatType_F32] = result.deinterleaveF32 = AudioSamples_Deinterleave_F32_Default;

	return(result);
}

extern bool AudioSamplesConvert(AudioSampleConversionFunctions *funcTable, const AudioSampleIndex numSamples, const fplAudioFormatType inFormat, const fplAudioFormatType outFormat, const void *inSamples, void *outSamples) {
	if(funcTable == fpl_null || inSamples == fpl_null || outSamples == fpl_null) return(false);
	AudioSampleFormatConversionFunc *convFunc = funcTable->conversionTable[inFormat][outFormat];
	if(convFunc == fpl_null) {
		return(false); // Not supported
	}
	convFunc(numSamples, inSamples, outSamples);
	return(true);
}

extern bool AudioSamplesDeinterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, const void *inSamples, void **outSamples) {
	if(funcTable == fpl_null || inSamples == fpl_null || outSamples == fpl_null) return(false);
	AudioSampleDeinterleaveFunc *deinterleaveFunc = funcTable->deinterleaveTable[format];
	if(deinterleaveFunc != fpl_null) {
		deinterleaveFunc(numFrames, numChannels, inSamples, outSamples);
		return(true);
	}

	// No function found, we deinterleave in the slowest possible way -> Memory copy by size
	AudioBufferSize sampleSize = fplGetAudioSampleSizeInBytes(format);
	for(AudioChannelIndex channelIndex = 0; channelIndex < numChannels; ++channelIndex) {
		uint8_t *outS8 = (uint8_t *)outSamples[channelIndex];
		for(AudioFrameIndex frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
			uint8_t *sourceMem = (uint8_t *)inSamples + (frameIndex * numChannels * sampleSize + channelIndex * sampleSize);
			uint8_t *targetMem = outS8 + (frameIndex * sampleSize);
			fplMemoryCopy(sourceMem, sampleSize, targetMem);
		}
	}

	return(true);
}

extern bool AudioSamplesInterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, const void **inSamples, void *outSamples) {
	if(funcTable == fpl_null || inSamples == fpl_null || outSamples == fpl_null) return(false);
	AudioSampleInterleaveFunc *interleaveFunc = funcTable->interleaveTable[format];
	if(interleaveFunc != fpl_null) {
		interleaveFunc(numFrames, numChannels, inSamples, outSamples);
		return(true);
	}

	// No function found, we interleave in the slowest possible way -> Memory copy by size
	AudioBufferSize sampleSize = fplGetAudioSampleSizeInBytes(format);
	for(AudioFrameIndex frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		for(AudioChannelIndex channelIndex = 0; channelIndex < numChannels; ++channelIndex) {
			const void *x = inSamples[channelIndex];
			const uint8_t *sourceMem = (const uint8_t *)x;
			uint8_t *targetMem = (uint8_t *)outSamples + ((frameIndex * numChannels + channelIndex) * sampleSize);
			fplMemoryCopy(sourceMem, sampleSize, targetMem);
		}
	}
	return(true);
}

// **********************************************************************************************************************
// Test Suite
// **********************************************************************************************************************

#define S24_FROM_S32(value32) ((uint8_t)((value32 & 0x0000FF) >> 0)), ((uint8_t)((value32 & 0x00FF00) >> 8)), ((uint8_t)((value32 & 0xFF0000) >> 16))

static int16_t Test5Samples_Convert_S16[] = {
	-INT16_MAX,
	-INT16_MAX/2,
	0,
	INT16_MAX/2,
	INT16_MAX,
};

static uint8_t Test5Samples_Convert_S24[] = {
	S24_FROM_S32(0xAAAAAA),
	S24_FROM_S32(0xBBBBBB),
	S24_FROM_S32(0xCCCCCC),
	S24_FROM_S32(0xDDDDDD),
	S24_FROM_S32(0xEEEEEE),
};

static int32_t Test5Samples_Convert_S32[] = {
	-INT32_MAX,
	-INT32_MAX / 2,
	0,
	INT32_MAX / 2,
	INT32_MAX,
};

const int32_t Test4Frames_Interleaved_S32_OneChannel[4] = {
	42,
	42,
	42,
	42,
};
const int32_t Test4Frames_Deinterleaved_S32_OneChannel[1][4] = {
	{42, 42, 42, 42},
};

const int32_t Test4Frames_Interleaved_S32_TwoChannels[8] = {
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
};
const int32_t Test4Frames_Deinterleaved_S32_TwoChannels[2][4] = {
	{-INT32_MAX, -INT32_MAX, -INT32_MAX, -INT32_MAX},
	{INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}
};
const void *Test4Frames_Deinterleaved_S32_TwoChannelsP[2] = {
	&Test4Frames_Deinterleaved_S32_TwoChannels[0],
	&Test4Frames_Deinterleaved_S32_TwoChannels[1],
};

const int32_t Test4Frames_Interleaved_S32_FiveChannels[20] = {
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX
};
const int32_t Test4Frames_Deinterleaved_S32_FiveChannels[5][4] = {
	{-INT32_MAX, -INT32_MAX, -INT32_MAX, -INT32_MAX},
	{-INT32_MAX / 2, -INT32_MAX / 2, -INT32_MAX / 2, -INT32_MAX / 2},
	{0, 0, 0, 0},
	{INT32_MAX / 2, INT32_MAX / 2, INT32_MAX / 2, INT32_MAX / 2},
	{INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}
};
const void *Test4Frames_Deinterleaved_S32_FiveChannelsP[5] = {
	&Test4Frames_Deinterleaved_S32_FiveChannels[0],
	&Test4Frames_Deinterleaved_S32_FiveChannels[1],
	&Test4Frames_Deinterleaved_S32_FiveChannels[2],
	&Test4Frames_Deinterleaved_S32_FiveChannels[3],
	&Test4Frames_Deinterleaved_S32_FiveChannels[4]
};

extern bool IsAudioDeinterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void **a, const void **b) {
	size_t lineWidth = numFrames * formatSize;
	for(AudioChannelIndex channelIndex = 0; channelIndex < numChannels; ++channelIndex) {
		const void *aLine = a[channelIndex];
		const void *bLine = b[channelIndex];
		int r = memcmp(aLine, bLine, lineWidth);
		if(r != 0) {
			return(false);
		}
	}
	return(true);
}

extern bool IsAudioInterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void *a, const void *b) {
	size_t totalWidth = numFrames * numChannels * formatSize;
	int r = memcmp(a, b, totalWidth);
	if(r != 0) {
		return(false);
	}
	return(true);
}

static void TestAudioSamplesDeinterleave() {
	AudioSampleConversionFunctions funcTable = CreateAudioSamplesConversionFunctions();

	{
		// Deinterleave S32, 2 channels, 4 Frames
		int32_t outSamplesTyped[2][4] = fplZeroInit;
		void *outSamples[2] = { &outSamplesTyped[0], &outSamplesTyped[1] };

		AudioSamples_Deinterleave_S32_Default(4, 2, Test4Frames_Interleaved_S32_TwoChannels, outSamples);
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 2, sizeof(int32_t), outSamples, Test4Frames_Deinterleaved_S32_TwoChannelsP));

		fplMemoryClear(outSamplesTyped, sizeof(outSamplesTyped));
		fplAlwaysAssert(AudioSamplesDeinterleave(&funcTable, 4, 2, fplAudioFormatType_S32, Test4Frames_Interleaved_S32_TwoChannels, outSamples));
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 2, sizeof(int32_t), outSamples, Test4Frames_Deinterleaved_S32_TwoChannelsP));
	}
	{
		// Deinterleave S32, 5 channels, 4 Frames
		int32_t outSamplesTyped[5][4] = fplZeroInit;
		void *outSamples[5] = { &outSamplesTyped[0], &outSamplesTyped[1], &outSamplesTyped[2], &outSamplesTyped[3], &outSamplesTyped[4] };

		AudioSamples_Deinterleave_S32_Default(4, 5, Test4Frames_Interleaved_S32_FiveChannels, outSamples);
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 5, sizeof(int32_t), (const void **)outSamples, (const void **)Test4Frames_Deinterleaved_S32_FiveChannelsP));

		fplMemoryClear(outSamplesTyped, sizeof(outSamplesTyped));
		fplAlwaysAssert(AudioSamplesDeinterleave(&funcTable, 4, 5, fplAudioFormatType_S32, Test4Frames_Interleaved_S32_FiveChannels, outSamples));
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 5, sizeof(int32_t), (const void **)outSamples, (const void **)Test4Frames_Deinterleaved_S32_FiveChannelsP));
	}
}

static void TestAudioSamplesInterleave() {
	AudioSampleConversionFunctions funcTable = CreateAudioSamplesConversionFunctions();

	{
		// Interleave S32, 2 channels, 4 Frames
		int32_t outSamples[8] = fplZeroInit;

		AudioSamples_Interleave_S32_Default(4, 2, Test4Frames_Deinterleaved_S32_TwoChannelsP, (void *)outSamples);
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 2, sizeof(int32_t), (void *)outSamples, (void *)Test4Frames_Interleaved_S32_TwoChannels));

		fplMemoryClear(outSamples, sizeof(outSamples));
		fplAlwaysAssert(AudioSamplesInterleave(&funcTable, 4, 2, fplAudioFormatType_S32, Test4Frames_Deinterleaved_S32_TwoChannelsP, (void *)outSamples));
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 2, sizeof(int32_t), (void *)outSamples, (void *)Test4Frames_Interleaved_S32_TwoChannels));
	}
	{
		// Interleave S32, 5 channels, 4 Frames
		int32_t outSamples[20] = fplZeroInit;

		AudioSamples_Interleave_S32_Default(4, 5, Test4Frames_Deinterleaved_S32_FiveChannelsP, (void *)outSamples);
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 5, sizeof(int32_t), (void *)outSamples, (void *)Test4Frames_Interleaved_S32_FiveChannels));

		fplMemoryClear(outSamples, sizeof(outSamples));
		fplAlwaysAssert(AudioSamplesInterleave(&funcTable, 4, 5, fplAudioFormatType_S32, Test4Frames_Deinterleaved_S32_FiveChannelsP, (void *)outSamples));
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 5, sizeof(int32_t), (void *)outSamples, (void *)Test4Frames_Interleaved_S32_FiveChannels));
	}
}

extern void TestAudioSamplesSuite() {
	TestAudioSamplesDeinterleave();
	TestAudioSamplesInterleave();
}

#endif // FINAL_AUDIO_CONVERSION_H