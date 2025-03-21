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
	AudioSampleFormatConversionFunc *convF32ToU8;
	AudioSampleFormatConversionFunc *convS16ToF32;
	AudioSampleFormatConversionFunc *convF32ToS16;
	AudioSampleFormatConversionFunc *convS24ToF32;
	AudioSampleFormatConversionFunc *convF32ToS24;
	AudioSampleFormatConversionFunc *convS32ToF32;
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


extern bool AudioSamplesConvert(AudioSampleConversionFunctions *funcTable, const AudioSampleIndex numSamples, const fplAudioFormatType inFormat, const fplAudioFormatType outFormat, const void *inSamples, void *outSamples);

extern bool AudioSamplesDeinterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, void *inSamples, void **outSamples);

extern bool AudioSamplesInterleave(AudioSampleConversionFunctions *funcTable, const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const fplAudioFormatType format, void **inSamples, void *outSamples);

extern bool IsAudioDeinterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void **a, const void **b);
extern bool IsAudioInterleavedSamplesEqual(const AudioFrameIndex numFrames, const AudioChannelIndex numChannels, const size_t formatSize, const void *a, const void *b);

#define AUDIO_RESAMPLE_BUFFER_FRAME_COUNT 512
#define AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT FPL_MAX_AUDIO_CHANNEL_COUNT
#define AUDIO_RESAMPLE_BUFFER_COUNT (AUDIO_RESAMPLE_BUFFER_FRAME_COUNT * AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT)

#define AUDIO_SINC_TABLE_SIZE 1024

typedef struct AudioSinCTable {
	float x[AUDIO_SINC_TABLE_SIZE];
	uint32_t lastIndex;
	uint32_t filterRadius;
} AudioSinCTable;

typedef struct AudioResamplingContext {
	//! The current input samples in de-interleaved format: LLLLLLLL, RRRRRRRR instead of LRLRLRLR
	float inBuffer[AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT][AUDIO_RESAMPLE_BUFFER_FRAME_COUNT];
	//! The current output samples in de-interleaved format: LLLLLLLL, RRRRRRRR instead of LRLRLRLR
	float outBuffer[AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT][AUDIO_RESAMPLE_BUFFER_FRAME_COUNT];
	//! The precomputed SinC table
	AudioSinCTable sincTable;
	//! The current processed input frame count
	AudioFrameIndex inFrameCount;
	//! The current processed output frame count
	AudioFrameIndex outFrameCount;
	//! The number of audio channels
	AudioChannelIndex channelCount;
} AudioResamplingContext;

extern AudioResampleResult AudioResampleInterleaved(const AudioChannelIndex numChannels, const AudioSampleIndex inSampleRate, const AudioSampleIndex outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float volume, const float *inSamples, float *outSamples);
extern AudioResampleResult AudioResampleDeinterleaved(const AudioChannelIndex numChannels, const AudioSampleIndex inSampleRate, const AudioSampleIndex outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float volume, const float **inSamples, float **outSamples);

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

const uint32_t AUDIO_INT24_MIN = -8388608;
const uint32_t AUDIO_INT24_MAX = 8388607;

inline float ClampF32(const float x, const float min, const float max) {
	return fplMax(min, fplMin(max, x));
}

inline float ClipF32(const float x) {
	return ClampF32(x, -1.0f, 1.0f);
}

//! Converts 8-bit unsigiend integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_U8ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const uint8_t *inU8 = (const uint8_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float halfU8 = (float)UINT8_MAX / 2.0f;
	const float invHalfU8 = 1.0f / halfU8;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		float x = (float)inU8[sampleIndex];	// Load value
		x *= invHalfU8;						// Scale into float range of 0.0 to 2.0
		x -= 1.0f;							// Subtract one to get into range of -1.0 to 1.0
		outF32[sampleIndex] = x;			// Output
	}
}

//! Converts 32-bit floating point samples to 8-bit unsigned integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToU8_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	uint8_t *outU8 = (uint8_t *)outSamples;
	const float halfU8 = (float)UINT8_MAX / 2.0f;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];				// Load value
		x = ClipF32(x);					// Clip to -1.0 and 1.0
		x += 1.0f;						// Scale into range of 0.0 to 2.0
		x *= halfU8;					// Scale into of 0 to 255
		x = roundf(x);					// Round to correct value
		uint8_t output = (uint8_t)x;	// Cast to U8
		outU8[i] = output;				// Output
	}
}

//! Converts 16-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S16ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const int16_t *inS16 = (const int16_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float invS16 = 1.0f / (float)INT16_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		float x = (float)inS16[sampleIndex];	// Load value
		x *= invS16;							// Scale into float range
		outF32[sampleIndex] = x;				// Output
	}
}

//! Converts 32-bit floating point samples to 16-bit integer samples by the number of samples specified
static void AudioSamples_Convert_F32ToS16_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const float *inF32 = (const float *)inSamples;
	int16_t *outS16 = (int16_t *)outSamples;
	const float maxS16 = (float)INT16_MAX;
	for(AudioSampleIndex i = 0; i < sampleCount; ++i) {
		float x = inF32[i];				// Load value
		x = ClipF32(x);					// Clip to -1.0 and 1.0
		x *= maxS16;					// Scale into int16_t range
		x = roundf(x);					// Round
		int16_t output = (int16_t)x;	// Cast to S16
		outS16[i] = output;				// Output
	}
}

//! Converts 24-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S24ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const uint8_t *inS24 = (const uint8_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float invMax24 = 1.0f / (float)AUDIO_INT24_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		// Load values
		uint8_t a = inS24[sampleIndex * 3 + 0];
		uint8_t b = inS24[sampleIndex * 3 + 1];
		uint8_t c = inS24[sampleIndex * 3 + 2];
		// Convert the three 8-bit samples to a 32-bit sample value
		int32_t value24 = (int32_t)(((uint32_t)a << 8) | ((uint32_t)b << 16) | ((uint32_t)b << 24));
		// Move 8-bit forward to leave the first 8-bits as zero
		value24 = value24 >> 8;
		// Cast to F32
		float x = (float)value24;
		// Scale to range of -1.0 to 1.0
		x *= invMax24;
		// Output
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
		x = ClipF32(x);					// Clip to -1.0 and 1.0
		x *= max24f;					// Scale to 24-bit
		int32_t value24 = (int32_t)x;	// Convert to int32

		// Extract the three 8-bits
		uint8_t a = (uint8_t)((value24 & 0x0000FF) >> 0);
		uint8_t b = (uint8_t)((value24 & 0x00FF00) >> 8);
		uint8_t c = (uint8_t)((value24 & 0xFF0000) >> 16);

		// Output
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
		x = ClipF32(x);					// Clip to -1.0 and 1.0
		x *= fm;						// Scale into int32_t range
		int32_t output = (int32_t)x;	// Cast to S32
		outS32[i] = output;
	}
}

//! Converts 32-bit integer samples to 32-bit floating point samples by the number of samples specified
static void AudioSamples_Convert_S32ToF32_Default(const AudioSampleIndex sampleCount, const void *inSamples, void *outSamples) {
	const int32_t *inS32 = (const int32_t *)inSamples;
	float *outF32 = (float *)outSamples;
	const float invS32 = 1.0f / (float)INT32_MAX;
	for(AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		float x = (float)inS32[sampleIndex];	// Load value
		x *= invS32;							// Scale into float range
		outF32[sampleIndex] = x;
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
// Resamping
// **********************************************************************************************************************
const float AudioPi32 = (float)M_PI;

static void AudioSinCTableInitialize(AudioSinCTable *table, const uint32_t filterRadius) {
	fplClearStruct(table);
	table->lastIndex = AUDIO_SINC_TABLE_SIZE - 1;
	table->filterRadius = filterRadius;
	for (uint32_t i = 0; i < AUDIO_SINC_TABLE_SIZE; ++i) {
        float x = ((float)i / table->lastIndex) * (filterRadius * 2) - filterRadius;
        table->x[i] = (x == 0.0f ? 1.0f : sinf(AudioPi32 * x) / (AudioPi32 * x));
    }
}

static float GetSinCTableValue(const AudioSinCTable *table, const float f) {
	int index = (int)((f + table->filterRadius) / (table->filterRadius * 2) * table->lastIndex);
	if (index < 0) {
		index = 0;
	}
	if (index >= AUDIO_SINC_TABLE_SIZE) {
		index = table->lastIndex;
	}
    return table->x[index];
}

inline float AudioSinC(const float x) {
    if (x == 0.0f) {
        return 1.0f; // sinc(0) = 1
    }
    return sinf(AudioPi32 * x) / (AudioPi32 * x); // sinc(x) = sin(PI x) / (PI x)
}

/**
* @brief Resamples the specified interleaved source audio frames from its sample rate to the interleaved target frames to its target sample rate.
* Uses the SinC algorythm for best quality results.
* This is used for converting common sample rates, such as 44100 <-> 48000 or 44100 <-> 22050.
* @note The number of audio channels are both equal.
* @note The format is fixed floating point 32-bit for both source and target samples and each sample is in range of -1.0 to 1.0.
* @note The samples must be in interleaved format: Frame 0: L R, 1: L R, 2: L R, 3: L R 4: L R, ...
* @param[in] channelCount The number of audio channels for each audio frame.
* @param[in] sourceSampleRate The source sample rate in Hz.
* @param[in] targetSampleRate The target sample rate in Hz.
* @param[in] sourceFrameCount The number of source audio frames that is available.
* @param[in] targetFrameCount The number of target audio frames that is required.
* @param[in] filterRadius The filter radius (8 is a good compromise between speed and quality).
* @param[in] volume The volume in range of 0.0 to 1.0.
* @param[in] inSamples The interleaved source audio samples float buffer.
* @param[out] outSamples The interleaved target audio samples float buffer.
* @return Returns the number interleaved of input and output frames
*/
static AudioResampleResult Audio__ResamplingInterleaved(const uint16_t channelCount, const uint32_t sourceSampleRate, const uint32_t targetSampleRate, const uint32_t sourceFrameCount, const uint32_t targetFrameCount, const int filterRadius, const float volume, const float *inSamples, float *outSamples) {
    const float srcToTgtRatio = (float)targetSampleRate / (float)sourceSampleRate;
    const float tgtToSrcRatio = 1.0f / srcToTgtRatio;

	// Clear samples
	fplMemoryClear(outSamples, targetFrameCount * channelCount * sizeof(float));

    for (uint32_t tgtFrame = 0; tgtFrame < targetFrameCount; ++tgtFrame) {
        float srcFrame = tgtFrame * tgtToSrcRatio;
        int srcFrameInt = (int)srcFrame;
        float frac = srcFrame - srcFrameInt;
        for (uint16_t channel = 0; channel < channelCount; ++channel) {
            float sample = 0.0f;
			float weightSum = 0.0f;
            for (int r = -filterRadius; r <= filterRadius; ++r) {
                int srcIndex = srcFrameInt + r;

#if 0
				// Version without jumps
				float f = r - frac;
				float sincValue = AudioSinC(f);
				int mask = (srcIndex >= 0 && srcIndex < (int)sourceFrameCount) ? 1 : 0;
				float input = inSamples[(srcIndex * channelCount + channel) * mask];
				float value = input * sincValue;
				float output = value * mask;
				sample += output;
#else
				// Version with jumps
                if (srcIndex >= 0 && srcIndex < (int)sourceFrameCount) {
					float input = inSamples[srcIndex * channelCount + channel];
					float f = r - frac;
                    float sincValue = AudioSinC(f);
					float output = input * sincValue;
                    sample += output;
					weightSum += sincValue;
                }
#endif
            }

			// Normalize the output sample
            if (weightSum != 0.0f) {
				float x = sample / weightSum;
                outSamples[tgtFrame * channelCount + channel] = x * volume;
            } else {
                outSamples[tgtFrame * channelCount + channel] = 0.0f; // Handle edge case
            }
        }
    }
	AudioResampleResult result = fplZeroInit;
    result.inputCount = sourceFrameCount;
    result.outputCount = targetFrameCount;
    return result;
}

/**
* @brief Resamples the specified de-interleaved source audio frames from its sample rate to the de-interleaved target frames to its target sample rate.
* Uses the SinC algorythm for best quality results.
* This is used for converting non-even sample rates from 44100 to 48000 and vice versa.
* This implementation is much more efficient than the interleaved version, because each channel can be processed independently.
* @note The number of audio channels are both equal.
* @note The format is fixed floating point 32-bit for both source and target samples and each sample is in range of -1.0 to 1.0.
* @note The samples must be in de-interleaved format: Frame 0-(N-1): LLLLLLLL..., Frame 0-(N-1): RRRRRRRR...
* @param[in] channelCount The number of audio channels for each audio frame.
* @param[in] sourceSampleRate The source sample rate in Hz.
* @param[in] targetSampleRate The target sample rate in Hz.
* @param[in] sourceFrameCount The number of source audio frames that is available.
* @param[in] targetFrameCount The number of target audio frames that is required.
* @param[in] filterRadius The filter radius (8 is a good compromise between speed and quality).
* @param[in] volume The volume in range of 0.0 to 1.0.
* @param[in] inSamples The de-interleaved source audio samples float buffer.
* @param[out] outSamples The de-interleaved target audio samples float buffer.
* @return Returns the number de-interleaved of input and output frames
*/
static AudioResampleResult Audio__ResamplingDeinterleaved(const uint16_t channelCount, const uint32_t sourceSampleRate, const uint32_t targetSampleRate, const uint32_t sourceFrameCount, const uint32_t targetFrameCount, const int filterRadius, const float volume, const float **inSamples, float **outSamples) {
	AudioResampleResult result = fplZeroInit;

	const float srcToTgtRatio = (float)targetSampleRate / (float)sourceSampleRate;
    const float tgtToSrcRatio = 1.0f / srcToTgtRatio;

	for (uint16_t channel = 0; channel < channelCount; ++channel) {
		const float *channelInSamples = &inSamples[channel][sourceFrameCount];
		float *channelOutSamples = &outSamples[channel][targetFrameCount];

		fplMemoryClear(channelOutSamples, targetFrameCount * sizeof(float));

		for (uint32_t tgtFrame = 0; tgtFrame < targetFrameCount; ++tgtFrame) {
			float srcFrame = tgtFrame * tgtToSrcRatio;
            int srcFrameInt = (int)srcFrame;
            float frac = srcFrame - srcFrameInt;

			float sample = 0.0f;
			float weightSum = 0.0f;
            for (int r = -filterRadius; r <= filterRadius; ++r) {
                int srcIndex = srcFrameInt + r;
#if 0
				// Version without jumps
				float f = r - frac;
				float sincValue = AudioSinC(f);
				int mask = (srcIndex >= 0 && srcIndex < (int)sourceFrameCount) ? 1 : 0;
				float input = channelInSamples[srcIndex * mask];
				float value = input * sincValue;
				float output = value * mask;
				sample += output;
				weightSum += sincValue;
#else
				// Version with jumps
                if (srcIndex >= 0 && srcIndex < (int)sourceFrameCount) {
					float f = r - frac;
                    float sincValue = AudioSinC(f);
					float input = channelInSamples[srcIndex];
					float output = input * sincValue;
                    sample += output;
					weightSum += sincValue;
                }
#endif
            }

			// Normalize the output sample
            if (weightSum != 0.0f) {
                channelOutSamples[tgtFrame] = (sample / weightSum) * volume;
            } else {
                channelOutSamples[tgtFrame] = 0.0f; // Handle edge case
            }
		}
	}
    result.inputCount = sourceFrameCount;
    result.outputCount = targetFrameCount;
    return result;
}

static AudioResampleResult AudioWeightedSampleSumDownSampling(const uint16_t channelCount, const uint32_t sourceSampleRate, const uint32_t targetSampleRate, const uint32_t sourceFrameCount, const uint32_t targetFrameCount, const float volume, const float *inSamples, float *outSamples) {
    const float srcToTgtRatio = (float)sourceSampleRate / (float)targetSampleRate;

    // Clear output buffer
    memset(outSamples, 0, targetFrameCount * channelCount * sizeof(float));

    for (uint32_t tgtFrame = 0; tgtFrame < targetFrameCount; ++tgtFrame) {
        // Calculate the corresponding source frame
        float srcFrame = tgtFrame * srcToTgtRatio;
        int srcFrameInt = (int)srcFrame; // Integer part of the source frame
        int sourceFrameRange = fplMax(1, (int)roundf(srcToTgtRatio)); // Number of source frames to average

        for (uint16_t channel = 0; channel < channelCount; ++channel) {
            float sample = 0.0f;
            float weightSum = 0.0f;

            // Sum the samples from the source frames
            for (int i = 0; i < sourceFrameRange; ++i) {
                int currentSourceFrameIndex = srcFrameInt + i;
                if (currentSourceFrameIndex < (int)sourceFrameCount) {
                    sample += inSamples[currentSourceFrameIndex * channelCount + channel];
                    weightSum += 1.0f; // Increment weight for valid samples
                } else {
                    // If we exceed the source frame count, use the last sample
                    sample += inSamples[(sourceFrameCount - 1) * channelCount + channel];
                    weightSum += 1.0f; // Still count this as a valid sample
                }
            }

            // Normalize the output sample
            if (weightSum > 0.0f) {
                outSamples[tgtFrame * channelCount + channel] = sample / weightSum;
            } else {
                outSamples[tgtFrame * channelCount + channel] = 0.0f; // Handle edge case
            }
        }
    }

    AudioResampleResult result = {sourceFrameCount, targetFrameCount};
    return result;
}

extern AudioResampleResult AudioResampleInterleaved(const AudioChannelIndex numChannels, const AudioSampleIndex inSampleRate, const AudioSampleIndex outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float volume, const float *inSamples, float *outSamples) {
	if (numChannels == 0 || inSampleRate == 0 || outSampleRate == 0 || minOutputFrameCount == 0 || maxInputFrameCount == 0) {
		AudioResampleResult zero = fplZeroInit;
		return zero;
	}

	AudioFrameIndex inFrameCount;
	AudioFrameIndex outFrameCount;
	if (outSampleRate > inSampleRate) {
		double upSamplingFactor = outSampleRate / (double)inSampleRate;
		inFrameCount = fplMin((AudioFrameIndex)round(minOutputFrameCount / upSamplingFactor), maxInputFrameCount);
		outFrameCount = (AudioFrameIndex)round(inFrameCount * upSamplingFactor);
	} else {
		double downSamplingFactor = inSampleRate / (double)outSampleRate;
		inFrameCount = fplMin((AudioFrameIndex)round(minOutputFrameCount * downSamplingFactor), maxInputFrameCount);
		outFrameCount = (AudioFrameIndex)round(inFrameCount / downSamplingFactor);
	}

	// Return just the number of frames, when the buffers was null
	if (inSamples == fpl_null || outSamples == fpl_null) {
		AudioResampleResult result = fplZeroInit;
		result.inputCount = inFrameCount;
		result.outputCount = outFrameCount;
		return result;
	 }

	const int filterRadius = 8;
	AudioResampleResult res = Audio__ResamplingInterleaved(numChannels, inSampleRate, outSampleRate, inFrameCount, outFrameCount, filterRadius, volume, inSamples, outSamples);
	return res;
}

extern AudioResampleResult AudioResampleDeinterleaved(const AudioChannelIndex numChannels, const AudioSampleIndex inSampleRate, const AudioSampleIndex outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float volume, const float **inSamples, float **outSamples) {
	if (numChannels == 0 || inSampleRate == 0 || outSampleRate == 0 || minOutputFrameCount == 0 || maxInputFrameCount == 0) {
		AudioResampleResult zero = fplZeroInit;
		return zero;
	}

	AudioFrameIndex inFrameCount;
	AudioFrameIndex outFrameCount;
	if (outSampleRate > inSampleRate) {
		double upSamplingFactor = outSampleRate / (double)inSampleRate;
		inFrameCount = fplMin((AudioFrameIndex)round(minOutputFrameCount / upSamplingFactor), maxInputFrameCount);
		outFrameCount = (AudioFrameIndex)round(inFrameCount * upSamplingFactor);
	} else {
		double downSamplingFactor = inSampleRate / (double)outSampleRate;
		inFrameCount = fplMin((AudioFrameIndex)round(minOutputFrameCount * downSamplingFactor), maxInputFrameCount);
		outFrameCount = (AudioFrameIndex)round(inFrameCount / downSamplingFactor);
	}

	// Return just the number of frames, when the buffers was null
	if (inSamples == fpl_null || outSamples == fpl_null) {
		AudioResampleResult result = fplZeroInit;
		result.inputCount = inFrameCount;
		result.outputCount = outFrameCount;
		return result;
	 }

	const int filterRadius = 8;
	AudioResampleResult res = Audio__ResamplingDeinterleaved(numChannels, inSampleRate, outSampleRate, inFrameCount, outFrameCount, filterRadius, volume, inSamples, outSamples);
	return res;
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

#define F32_CMP(a, b, t) (bool)(((float)fabs((a) - (b)) <= (t)))

typedef struct SampleU8ToF32 {
	uint8_t u8;
	float f32;
} SampleU8ToF32;

static SampleU8ToF32 Test_Samples_Convert_U8_F32[] = {
	{ 0, -1.0f },       // Minimum value
    { 64, -0.5f },      // Mid negative value
    { 128, 0.0f },      // Zero value
    { 191, 0.5f },      // Mid positive value
    { 255, 1.0f },      // Maximum value
};

typedef struct SampleS16ToF32 {
	int16_t s16;
	float f32;
} SampleS16ToF32;

static SampleS16ToF32 Test_Samples_Convert_S16_F32[] = {
	{ -32767, -1.0f },   // Minimum value
    { -16384, -0.5f },   // Mid negative value
    {      0,  0.0f },   // Zero value
    {  16384,  0.5f },   // Mid positive value
    {  32767,  1.0f },   // Maximum value
};

typedef struct SampleS24 {
	uint8_t a;
	uint8_t b;
	uint8_t c;
} SampleS24;

typedef struct SampleS24ToF32 {
	SampleS24 s24;
	float f32;
} SampleS24ToF32;

static SampleS24ToF32 Test_Samples_Convert_S24_F32[] = {
	{{0x01, 0x00, 0x80}, -1.0f},       // Minimum value
	{{0x01, 0x00, 0xc0}, -0.5f},       // Negative mid range
    {{0x00, 0x00, 0x00}, 0.0f},        // Zero value
	{{0xff, 0xff, 0x3f}, 0.5f},       // Positive mid range
    {{0xFF, 0xFF, 0x7f}, 1.0f},        // Maximum value
};

const int32_t Test_4_Frames_Interleaved_S32_OneChannel[4] = {
	42,
	42,
	42,
	42,
};
const int32_t Test_4_Frames_Deinterleaved_S32_OneChannel[1][4] = {
	{42, 42, 42, 42},
};

const int32_t Test_4_Frames_Interleaved_S32_TwoChannels[8] = {
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
	-INT32_MAX, INT32_MAX,
};
const int32_t Test_4_Frames_Deinterleaved_S32_TwoChannels[2][4] = {
	{-INT32_MAX, -INT32_MAX, -INT32_MAX, -INT32_MAX},
	{INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}
};

const void *Test_4_Frames_Deinterleaved_S32_TwoChannelsP[2] = {
	&Test_4_Frames_Deinterleaved_S32_TwoChannels[0],
	&Test_4_Frames_Deinterleaved_S32_TwoChannels[1],
};

const int32_t Test_4_Frames_Interleaved_S32_FiveChannels[20] = {
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX,
	-INT32_MAX, -INT32_MAX / 2, 0, INT32_MAX / 2, INT32_MAX
};

const int32_t Test_4_Frames_Deinterleaved_S32_FiveChannels[5][4] = {
	{-INT32_MAX, -INT32_MAX, -INT32_MAX, -INT32_MAX},
	{-INT32_MAX / 2, -INT32_MAX / 2, -INT32_MAX / 2, -INT32_MAX / 2},
	{0, 0, 0, 0},
	{INT32_MAX / 2, INT32_MAX / 2, INT32_MAX / 2, INT32_MAX / 2},
	{INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}
};

const void *Test_4_Frames_Deinterleaved_S32_FiveChannelsP[5] = {
	&Test_4_Frames_Deinterleaved_S32_FiveChannels[0],
	&Test_4_Frames_Deinterleaved_S32_FiveChannels[1],
	&Test_4_Frames_Deinterleaved_S32_FiveChannels[2],
	&Test_4_Frames_Deinterleaved_S32_FiveChannels[3],
	&Test_4_Frames_Deinterleaved_S32_FiveChannels[4]
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

static void TestAudioSamplesConversion() {
	AudioSampleConversionFunctions funcTable = CreateAudioSamplesConversionFunctions();

	// U8 -> F32
	{
		const AudioSampleIndex sampleCount = fplArrayCount(Test_Samples_Convert_U8_F32);
		uint8_t expectedSamplesU8[8] = fplZeroInit;
		float expectedSamplesF32[8] = fplZeroInit;
		fplAlwaysAssert(fplArrayCount(expectedSamplesU8) >= sampleCount);
		fplAlwaysAssert(fplArrayCount(expectedSamplesF32) >= sampleCount);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			expectedSamplesU8[i] = Test_Samples_Convert_U8_F32[i].u8;
			expectedSamplesF32[i] = Test_Samples_Convert_U8_F32[i].f32;
		}

		float referenceSamplesF32[8] = fplZeroInit;
		AudioSamples_Convert_U8ToF32_Default(sampleCount, expectedSamplesU8, referenceSamplesF32);

		float actualSamplesF32[8] = fplZeroInit;
		bool res = AudioSamplesConvert(&funcTable, sampleCount, fplAudioFormatType_U8, fplAudioFormatType_F32, expectedSamplesU8, actualSamplesF32);
		fplAlwaysAssert(res);

		const float u8Tolerance = (1.0f / (float)UINT8_MAX) * 2.0f;
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			float referenceSampleF32 = referenceSamplesF32[i];
			float actualSampleF32 = actualSamplesF32[i];
			float expectedSampleF32 = expectedSamplesF32[i];
			float diff = (float)fabs(expectedSampleF32 - referenceSampleF32);
			bool equalsReference = F32_CMP(expectedSampleF32, referenceSampleF32, u8Tolerance);
			bool equalsActual = F32_CMP(referenceSampleF32, actualSampleF32, FLT_EPSILON);
			fplAlwaysAssert(equalsReference && equalsActual);
		}
	}

	// F32 -> U8
	{
		const AudioSampleIndex sampleCount = fplArrayCount(Test_Samples_Convert_U8_F32);
		uint8_t expectedSamplesU8[8] = fplZeroInit;
		float expectedSamplesF32[8] = fplZeroInit;
		fplAlwaysAssert(fplArrayCount(expectedSamplesU8) >= sampleCount);
		fplAlwaysAssert(fplArrayCount(expectedSamplesF32) >= sampleCount);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			expectedSamplesU8[i] = Test_Samples_Convert_U8_F32[i].u8;
			expectedSamplesF32[i] = Test_Samples_Convert_U8_F32[i].f32;
		}

		uint8_t referenceSamplesU8[8] = fplZeroInit;
		AudioSamples_Convert_F32ToU8_Default(sampleCount, expectedSamplesF32, referenceSamplesU8);

		uint8_t actualSamplesU8[8] = fplZeroInit;
		bool res = AudioSamplesConvert(&funcTable, sampleCount, fplAudioFormatType_F32, fplAudioFormatType_U8, expectedSamplesF32, actualSamplesU8);
		fplAlwaysAssert(res);

		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			uint8_t referenceSampleU8 = referenceSamplesU8[i];
			uint8_t actualSampleU8 = actualSamplesU8[i];
			uint8_t expectedSampleU8 = expectedSamplesU8[i];
			fplAlwaysAssert(expectedSampleU8 == referenceSampleU8);
			fplAlwaysAssert(actualSampleU8 == referenceSampleU8);
		}
	}

	// S16 -> F32
	{
		const AudioSampleIndex sampleCount = fplArrayCount(Test_Samples_Convert_S16_F32);
		int16_t expectedSamplesS16[8] = fplZeroInit;
		float expectedSamplesF32[8] = fplZeroInit;
		fplAlwaysAssert(fplArrayCount(expectedSamplesS16) >= sampleCount);
		fplAlwaysAssert(fplArrayCount(expectedSamplesF32) >= sampleCount);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			expectedSamplesS16[i] = Test_Samples_Convert_S16_F32[i].s16;
			expectedSamplesF32[i] = Test_Samples_Convert_S16_F32[i].f32;
		}

		float referenceSamplesF32[8] = fplZeroInit;
		AudioSamples_Convert_S16ToF32_Default(sampleCount, expectedSamplesS16, referenceSamplesF32);

		float actualSamplesF32[8] = fplZeroInit;
		bool res = AudioSamplesConvert(&funcTable, sampleCount, fplAudioFormatType_S16, fplAudioFormatType_F32, expectedSamplesS16, actualSamplesF32);
		fplAlwaysAssert(res);

		const float s16Tolerance = (1.0f / (float)INT16_MAX);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			float referenceSampleF32 = referenceSamplesF32[i];
			float actualSampleF32 = actualSamplesF32[i];
			float expectedSampleF32 = expectedSamplesF32[i];
			float diff = (float)fabs(expectedSampleF32 - referenceSampleF32);
			bool equalsReference = F32_CMP(expectedSampleF32, referenceSampleF32, s16Tolerance);
			bool equalsActual = F32_CMP(referenceSampleF32, actualSampleF32, FLT_EPSILON);
			fplAlwaysAssert(equalsReference && equalsActual);
		}
	}

	// F32 -> S16
	{
		const AudioSampleIndex sampleCount = fplArrayCount(Test_Samples_Convert_S16_F32);
		int16_t expectedSamplesS16[8] = fplZeroInit;
		float expectedSamplesF32[8] = fplZeroInit;
		fplAlwaysAssert(fplArrayCount(expectedSamplesS16) >= sampleCount);
		fplAlwaysAssert(fplArrayCount(expectedSamplesF32) >= sampleCount);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			expectedSamplesS16[i] = Test_Samples_Convert_S16_F32[i].s16;
			expectedSamplesF32[i] = Test_Samples_Convert_S16_F32[i].f32;
		}

		int16_t referenceSamplesS16[8] = fplZeroInit;
		AudioSamples_Convert_F32ToS16_Default(sampleCount, expectedSamplesF32, referenceSamplesS16);

		int16_t actualSamplesS16[8] = fplZeroInit;
		bool res = AudioSamplesConvert(&funcTable, sampleCount, fplAudioFormatType_F32, fplAudioFormatType_S16, expectedSamplesF32, actualSamplesS16);
		fplAlwaysAssert(res);

		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			int16_t referenceSampleS16 = referenceSamplesS16[i];
			int16_t actualSampleS16 = actualSamplesS16[i];
			int16_t expectedSampleS16 = expectedSamplesS16[i];
			fplAlwaysAssert(expectedSampleS16 == referenceSampleS16);
			fplAlwaysAssert(actualSampleS16 == referenceSampleS16);
		}
	}

	// S24 -> F32
	{
		const AudioSampleIndex sampleCount = fplArrayCount(Test_Samples_Convert_S24_F32);
		SampleS24 expectedSamplesS24[8] = fplZeroInit;
		float expectedSamplesF32[8] = fplZeroInit;
		fplAlwaysAssert(fplArrayCount(expectedSamplesS24) >= sampleCount);
		fplAlwaysAssert(fplArrayCount(expectedSamplesF32) >= sampleCount);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			expectedSamplesS24[i] = Test_Samples_Convert_S24_F32[i].s24;
			expectedSamplesF32[i] = Test_Samples_Convert_S24_F32[i].f32;
		}

		float referenceSamplesF32[8] = fplZeroInit;
		AudioSamples_Convert_S24ToF32_Default(sampleCount, expectedSamplesS24, referenceSamplesF32);

		float actualSamplesF32[8] = fplZeroInit;
		bool res = AudioSamplesConvert(&funcTable, sampleCount, fplAudioFormatType_S24, fplAudioFormatType_F32, expectedSamplesS24, actualSamplesF32);
		fplAlwaysAssert(res);

		const float s24Tolerance = (1.0f / (float)AUDIO_INT24_MAX);
		for (AudioSampleIndex i = 0; i < sampleCount; ++i) {
			float referenceSampleF32 = referenceSamplesF32[i];
			float actualSampleF32 = actualSamplesF32[i];
			float expectedSampleF32 = expectedSamplesF32[i];
			float diff = (float)fabs(expectedSampleF32 - referenceSampleF32);
			bool equalsReference = F32_CMP(expectedSampleF32, referenceSampleF32, s24Tolerance);
			bool equalsActual = F32_CMP(referenceSampleF32, actualSampleF32, FLT_EPSILON);
			fplAlwaysAssert(equalsReference && equalsActual);
		}
	}
}

static void TestAudioSamplesDeinterleave() {
	AudioSampleConversionFunctions funcTable = CreateAudioSamplesConversionFunctions();

	{
		// Deinterleave S32, 2 channels, 4 Frames
		int32_t outSamplesTyped[2][4] = fplZeroInit;
		void *outSamples[2] = { &outSamplesTyped[0], &outSamplesTyped[1] };

		AudioSamples_Deinterleave_S32_Default(4, 2, Test_4_Frames_Interleaved_S32_TwoChannels, outSamples);
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 2, sizeof(int32_t), (const void **)outSamples, (const void **)Test_4_Frames_Deinterleaved_S32_TwoChannelsP));

		fplMemoryClear(outSamplesTyped, sizeof(outSamplesTyped));
		fplAlwaysAssert(AudioSamplesDeinterleave(&funcTable, 4, 2, fplAudioFormatType_S32, Test_4_Frames_Interleaved_S32_TwoChannels, outSamples));
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 2, sizeof(int32_t), (const void **)outSamples, (const void **)Test_4_Frames_Deinterleaved_S32_TwoChannelsP));
	}
	{
		// Deinterleave S32, 5 channels, 4 Frames
		int32_t outSamplesTyped[5][4] = fplZeroInit;
		void *outSamples[5] = { &outSamplesTyped[0], &outSamplesTyped[1], &outSamplesTyped[2], &outSamplesTyped[3], &outSamplesTyped[4] };

		AudioSamples_Deinterleave_S32_Default(4, 5, Test_4_Frames_Interleaved_S32_FiveChannels, outSamples);
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 5, sizeof(int32_t), (const void **)outSamples, (const void **)Test_4_Frames_Deinterleaved_S32_FiveChannelsP));

		fplMemoryClear(outSamplesTyped, sizeof(outSamplesTyped));
		fplAlwaysAssert(AudioSamplesDeinterleave(&funcTable, 4, 5, fplAudioFormatType_S32, Test_4_Frames_Interleaved_S32_FiveChannels, outSamples));
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(4, 5, sizeof(int32_t), (const void **)outSamples, (const void **)Test_4_Frames_Deinterleaved_S32_FiveChannelsP));
	}
}

static void TestAudioSamplesInterleave() {
	AudioSampleConversionFunctions funcTable = CreateAudioSamplesConversionFunctions();

	{
		// Interleave S32, 2 channels, 4 Frames
		int32_t outSamples[8] = fplZeroInit;

		AudioSamples_Interleave_S32_Default(4, 2, Test_4_Frames_Deinterleaved_S32_TwoChannelsP, (void *)outSamples);
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 2, sizeof(int32_t), (void *)outSamples, (void *)Test_4_Frames_Interleaved_S32_TwoChannels));

		fplMemoryClear(outSamples, sizeof(outSamples));
		fplAlwaysAssert(AudioSamplesInterleave(&funcTable, 4, 2, fplAudioFormatType_S32, Test_4_Frames_Deinterleaved_S32_TwoChannelsP, (void *)outSamples));
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 2, sizeof(int32_t), (void *)outSamples, (void *)Test_4_Frames_Interleaved_S32_TwoChannels));
	}
	{
		// Interleave S32, 5 channels, 4 Frames
		int32_t outSamples[20] = fplZeroInit;

		AudioSamples_Interleave_S32_Default(4, 5, Test_4_Frames_Deinterleaved_S32_FiveChannelsP, (void *)outSamples);
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 5, sizeof(int32_t), (void *)outSamples, (void *)Test_4_Frames_Interleaved_S32_FiveChannels));

		fplMemoryClear(outSamples, sizeof(outSamples));
		fplAlwaysAssert(AudioSamplesInterleave(&funcTable, 4, 5, fplAudioFormatType_S32, Test_4_Frames_Deinterleaved_S32_FiveChannelsP, (void *)outSamples));
		fplAlwaysAssert(IsAudioInterleavedSamplesEqual(4, 5, sizeof(int32_t), (void *)outSamples, (void *)Test_4_Frames_Interleaved_S32_FiveChannels));
	}
}

extern void TestAudioSamplesSuite() {
	TestAudioSamplesConversion();
	TestAudioSamplesDeinterleave();
	TestAudioSamplesInterleave();
}

#endif // FINAL_AUDIO_CONVERSION_H