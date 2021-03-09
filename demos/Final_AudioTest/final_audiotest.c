#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_UNDEF
#include <final_platform_layer.h>

#define FINAL_AUDIO_CONVERSION_IMPLEMENTATION
#include <final_audioconversion.h>

#define COMPARE_WITH_MINIAUDIO

#if defined(COMPARE_WITH_MINIAUDIO)
#	define MINIAUDIO_IMPLEMENTATION
#	include <miniaudio/miniaudio.h>
#endif

typedef struct InterleavedSamples {
	void *samples;
} InterleavedSamples;

static InterleavedSamples AllocateInterleavedSamples(fplAudioFormatType type, AudioChannelIndex numChannels, AudioFrameIndex frameCount) {
	AudioBufferSize size = fplGetAudioBufferSizeInBytes(type, numChannels, frameCount);
	void *base = fplMemoryAlignedAllocate(size, 16);
	InterleavedSamples result;
	result.samples = base;
	return(result);
}

static void ReleaseInterleavedSamples(InterleavedSamples *buffer) {
	fplMemoryAlignedFree(buffer->samples);
}

typedef struct DeinterleavedSamples {
	void *samples[AUDIO_MAX_CHANNEL_COUNT];
	void *base;
} DeinterleavedSamples;

static DeinterleavedSamples AllocateDeinterleavedSamples(fplAudioFormatType type, AudioChannelIndex numChannels, AudioFrameIndex frameCount) {
	DeinterleavedSamples result = fplZeroInit;

	AudioBufferSize sampleSize = fplGetAudioSampleSizeInBytes(type);
	AudioBufferSize stride = sampleSize * frameCount;
	AudioBufferSize totalSize = stride * numChannels;
	result.base = fplMemoryAlignedAllocate(totalSize, 16);

	for(AudioChannelIndex channelIndex = 0; channelIndex < numChannels; ++channelIndex) {
		void *samples = (uint8_t *)result.base + channelIndex * stride;
		result.samples[channelIndex] = samples;
	}

	return(result);
}

static void ReleaseDeinterleavedSamples(DeinterleavedSamples *buffer) {
	fplMemoryAlignedFree(buffer->samples);
}

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_Console, fpl_null)) {
		TestAudioSamplesSuite();

		AudioMilliseconds duration = 1000;

		AudioChannelIndex inChannels = 2;
		AudioHertz inSampleRate = 44100;
		fplAudioFormatType inFormat = fplAudioFormatType_F32;
		AudioFrameIndex inNumFrames = (AudioFrameIndex)(((float)inSampleRate / 1000.0) * (float)duration);
		AudioBufferSize inSampleSize = fplGetAudioSampleSizeInBytes(inFormat);

		AudioChannelIndex outChannels = 2;
		AudioHertz outSampleRate = 44100;
		fplAudioFormatType outFormat = fplAudioFormatType_S24;
		AudioFrameIndex outNumFrames = (AudioFrameIndex)(((float)outSampleRate / 1000.0) * (float)duration);
		AudioBufferSize outSampleSize = fplGetAudioSampleSizeInBytes(outFormat);

		// Build de-interleaved input samples (F32)
		InterleavedSamples inSamples = AllocateInterleavedSamples(inFormat, inChannels, inNumFrames);
		float *inSamplesF32 = (float *)inSamples.samples;
		for(AudioFrameIndex frameIndex = 0; frameIndex < inNumFrames; ++frameIndex) {
			float t = frameIndex / (float)inNumFrames;
			float sampleF32 = -1.0f + (2.0f * t);
			sampleF32 = -1.0f;
			for(AudioChannelIndex channelIndex = 0; channelIndex < inChannels; ++channelIndex) {
				inSamplesF32[frameIndex * inChannels + channelIndex] = sampleF32;
			}
		}

		fplAlwaysAssert(inSampleRate == outSampleRate);
		fplAlwaysAssert(inChannels == outChannels);
		fplAlwaysAssert(inNumFrames == outNumFrames);

		AudioSampleConversionFunctions conversionFuncs = CreateAudioSamplesConversionFunctions();

		AudioFrameIndex frameCount = inNumFrames;

		InterleavedSamples outSamples = AllocateInterleavedSamples(outFormat, outChannels, frameCount);

		DeinterleavedSamples inDeinterleaveSamples = AllocateDeinterleavedSamples(inFormat, inChannels, frameCount);
		DeinterleavedSamples outDeinterleaveSamples = AllocateDeinterleavedSamples(outFormat, outChannels, frameCount);

#if defined(COMPARE_WITH_MINIAUDIO)
		DeinterleavedSamples inDeinterleaveSamplesMA = AllocateDeinterleavedSamples(inFormat, inChannels, frameCount);
		DeinterleavedSamples outDeinterleaveSamplesMA = AllocateDeinterleavedSamples(outFormat, outChannels, frameCount);
		InterleavedSamples outInterleaveSamplesMA = AllocateInterleavedSamples(outFormat, outChannels, frameCount);
#endif

		bool res;

		// Deinterleave input samples (LLRRLLRR -> LLLLRRRR)
		res = AudioSamplesDeinterleave(&conversionFuncs, frameCount, inChannels, inFormat, inSamples.samples, inDeinterleaveSamples.samples);
		fplAlwaysAssert(res);

#if defined(COMPARE_WITH_MINIAUDIO)
		if(inFormat == fplAudioFormatType_F32) {
			ma_pcm_deinterleave_f32(inDeinterleaveSamplesMA.samples, inSamples.samples, frameCount, inChannels);
		} else {
			fplAlwaysAssert(!"Unsupported format!");
		}
		fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(frameCount, inChannels, inSampleSize, inDeinterleaveSamplesMA.samples, inDeinterleaveSamples.samples));
#endif

		// Convert to output format
		for(AudioChannelIndex channelIndex = 0; channelIndex < outChannels; ++channelIndex) {
			res = AudioSamplesConvert(&conversionFuncs, frameCount, inFormat, outFormat, inDeinterleaveSamples.samples[channelIndex], outDeinterleaveSamples.samples[channelIndex]);
			fplAlwaysAssert(res);

#if defined(COMPARE_WITH_MINIAUDIO)
			if(inFormat == fplAudioFormatType_F32 && outFormat == fplAudioFormatType_S24) {
				ma_pcm_f32_to_s24(outDeinterleaveSamplesMA.samples[channelIndex], inDeinterleaveSamples.samples[channelIndex], frameCount, ma_dither_mode_none);
			} else {
				fplAlwaysAssert(!"Unsupported format!");
			}
#endif
		}

#if defined(COMPARE_WITH_MINIAUDIO)
		if(outFormat == fplAudioFormatType_S24) {
			fplAlwaysAssert(IsAudioDeinterleavedSamplesEqual(frameCount, inChannels, outSampleSize, outDeinterleaveSamplesMA.samples, outDeinterleaveSamples.samples));
		} else {
			fplAlwaysAssert(!"Unsupported format!");
		}
#endif

		// Interleave converted samples (LLLLRRRR -> LRLRLRLR)
		res = AudioSamplesInterleave(&conversionFuncs, frameCount, outChannels, outFormat, outDeinterleaveSamples.samples, outSamples.samples);
		fplAlwaysAssert(res);

#if defined(COMPARE_WITH_MINIAUDIO)
		if(outFormat == fplAudioFormatType_S24) {
			ma_pcm_interleave_s24(outInterleaveSamplesMA.samples, outDeinterleaveSamplesMA.samples, frameCount, inChannels);
			fplAlwaysAssert(IsAudioInterleavedSamplesEqual(frameCount, inChannels, outSampleSize, outInterleaveSamplesMA.samples, outSamples.samples));
		} else {
			fplAlwaysAssert(!"Unsupported format!");
		}
#endif

#if defined(COMPARE_WITH_MINIAUDIO)
		ReleaseDeinterleavedSamples(&inDeinterleaveSamplesMA);
		ReleaseDeinterleavedSamples(&outDeinterleaveSamplesMA);
		ReleaseInterleavedSamples(&outInterleaveSamplesMA);
#endif
		ReleaseDeinterleavedSamples(&outDeinterleaveSamples);
		ReleaseDeinterleavedSamples(&inDeinterleaveSamples);

		ReleaseInterleavedSamples(&outSamples);
		ReleaseInterleavedSamples(&inSamples);

		fplConsoleOut("Press any key to exit");
		fplConsoleWaitForCharInput();

		fplPlatformRelease();
	}
	return(0);
}