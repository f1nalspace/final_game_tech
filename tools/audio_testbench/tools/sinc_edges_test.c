/*
Checks the two edges where the SinC path of the mixer in final_audiosystem.h got zero frames from AudioResampleInterleaved.
Every source holds the constant 0.5 in both channels, which SinC keeps (its weights are normalized), so an output frame near zero inside the track is a gap.

- down-end: 96 kHz played with pitch 1.7 (163.2 kHz into 48 kHz, a ratio below 0.5), 5 * 1741 + 1 frames, so exactly one source frame is left at the end. The item has to end.
- up-rest: 22.05 kHz into 48 kHz (above 2x) with 1024-frame calls, the PipeWire quantum. A call whose last chunk asks for less than half a source frame has to get a sample, not silence.

Build against the additions to test:
gcc -std=c99 -O2 -I <repo> -I <additions> -I <repo>/demos/dependencies sinc_edges_test.c -o sinc_edges_test -lm -ldl -lpthread
Usage: sinc_edges_test
Output: one line per case "<name>: ... ok|FAILED", the exit code is 1 when a case failed.
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>

static const AudioHertz OutputSampleRate = 48000;
static const AudioChannelIndex ChannelCount = 2;
static const float SourceValue = 0.5f;
// Anything below this inside the track is a gap, SinC keeps the constant far closer than that
static const float GapThreshold = 0.25f;

typedef struct EdgeTest {
	AudioSystem *audioSys;
	fplAudioFormat targetFormat;
	float *callSamples;
} EdgeTest;

static bool InitEdgeTest(EdgeTest *test, const AudioFrameIndex maxCallFrameCount) {
	test->targetFormat.type = fplAudioFormatType_F32;
	test->targetFormat.sampleRate = OutputSampleRate;
	test->targetFormat.channels = ChannelCount;
	test->audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	size_t callSampleCount = (size_t)maxCallFrameCount * ChannelCount;
	test->callSamples = (float *)fplMemoryAllocate(callSampleCount * sizeof(float));
	if(test->audioSys == fpl_null || test->callSamples == fpl_null) {
		return(false);
	}
	bool result = AudioSystemInit(test->audioSys, &test->targetFormat);
	return(result);
}

static void ReleaseEdgeTest(EdgeTest *test) {
	if(test->audioSys != fpl_null) {
		AudioSystemShutdown(test->audioSys);
		fplMemoryFree(test->audioSys);
	}
	if(test->callSamples != fpl_null) {
		fplMemoryFree(test->callSamples);
	}
}

static AudioSource *PlayConstantSource(EdgeTest *test, const AudioHertz sampleRate, const AudioFrameIndex frameCount, const float pitch) {
	AudioSource *source = AudioSystemAllocateSource(test->audioSys, ChannelCount, sampleRate, fplAudioFormatType_F32, frameCount);
	if(source == fpl_null) {
		return(fpl_null);
	}
	float *samples = (float *)source->buffer.samples;
	size_t sampleCount = (size_t)frameCount * ChannelCount;
	for(size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		samples[sampleIndex] = SourceValue;
	}
	AudioSystemAddSource(test->audioSys, source);
	AudioSystemPlaySource(test->audioSys, source, false, 1.0f, pitch);
	return(source);
}

static size_t GetPlayItemCount(EdgeTest *test) {
	size_t result = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
	return(result);
}

static bool RunDownsamplingEndCase(void) {
	const AudioHertz sourceRate = 96000;
	const float pitch = 1.7f;
	// 512 output frames take 1741 source frames at 163.2 kHz, the last call gets the one frame that is left
	const AudioFrameIndex callFrameCount = 512;
	const AudioFrameIndex sourceFramesPerCall = 1741;
	const AudioFrameIndex fullCallCount = 5;
	const AudioFrameIndex leftSourceFrameCount = 1;
	const AudioFrameIndex sourceFrameCount = fullCallCount * sourceFramesPerCall + leftSourceFrameCount;
	// The track lasts 6 calls, every call more means the item hangs at its end
	const uint32_t maxCallCount = 20;

	EdgeTest test = fplZeroInit;
	bool result = false;
	uint32_t callCount = 0;
	if(InitEdgeTest(&test, callFrameCount)) {
		AudioSource *source = PlayConstantSource(&test, sourceRate, sourceFrameCount, pitch);
		while(source != fpl_null && callCount < maxCallCount) {
			AudioSystemWriteFrames(test.audioSys, test.callSamples, &test.targetFormat, callFrameCount, true);
			++callCount;
			size_t itemCount = GetPlayItemCount(&test);
			if(itemCount == 0) {
				result = true;
				break;
			}
		}
	}
	printf("down-end: item ended after %u calls %s\n", callCount, result ? "ok" : "FAILED");
	ReleaseEdgeTest(&test);
	return(result);
}

static bool RunUpsamplingRestCase(void) {
	const AudioHertz sourceRate = 22050;
	const AudioFrameIndex sourceSeconds = 2;
	const AudioFrameIndex sourceFrameCount = sourceRate * sourceSeconds;
	const AudioFrameIndex callFrameCount = 1024;

	EdgeTest test = fplZeroInit;
	uint32_t gapCount = 0;
	uint32_t callCount = 0;
	bool hasEnded = false;
	if(InitEdgeTest(&test, callFrameCount)) {
		AudioSource *source = PlayConstantSource(&test, sourceRate, sourceFrameCount, AudioSystemDefaultPitch);
		while(source != fpl_null && !hasEnded) {
			AudioSystemWriteFrames(test.audioSys, test.callSamples, &test.targetFormat, callFrameCount, true);
			++callCount;
			size_t itemCount = GetPlayItemCount(&test);
			hasEnded = (itemCount == 0);
			// The call that mixes the end of the track is silent after it
			if(hasEnded) {
				break;
			}
			for(AudioFrameIndex frameIndex = 0; frameIndex < callFrameCount; ++frameIndex) {
				float left = test.callSamples[frameIndex * ChannelCount];
				if(left < GapThreshold) {
					++gapCount;
				}
			}
		}
	}
	bool result = hasEnded && (gapCount == 0);
	printf("up-rest: %u silent frames inside the track in %u calls %s\n", gapCount, callCount, result ? "ok" : "FAILED");
	ReleaseEdgeTest(&test);
	return(result);
}

int main(void) {
	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}
	bool isDownsamplingEndOk = RunDownsamplingEndCase();
	bool isUpsamplingRestOk = RunUpsamplingRestCase();
	fplPlatformRelease();
	return((isDownsamplingEndOk && isUpsamplingRestOk) ? 0 : 1);
}
