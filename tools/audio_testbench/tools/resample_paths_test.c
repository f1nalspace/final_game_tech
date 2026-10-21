/*
Validates the passthrough and the even-ratio paths of the mixer in final_audiosystem.h (frame skipping, frame duplication) sample by sample.
Every source frame k holds (k % 8191 + 1) / 16384 in the left channel and its negative in the right channel, F32 in and out at volume 1,
so the source frame behind every output frame is known exactly. After every AudioSystemWriteFrames call the play position must not be past the end of the source.

Build against the additions to test (assertions off, the test checks the invariants itself):
gcc -std=c99 -O2 -DFPL_NO_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies resample_paths_test.c -o resample_paths_test -lm -ldl -lpthread
Usage: resample_paths_test [-v]   (-v prints every failing case)
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RatePair {
	const char *name;
	AudioHertz sourceRate;
	AudioHertz targetRate;
	float pitch;
} RatePair;

static const RatePair RatePairs[] = {
	{ "passthrough", 48000, 48000, 1.0f },
	{ "down2", 96000, 48000, 1.0f },
	{ "down2-pitch", 48000, 48000, 2.0f },
	{ "down2-pitch-44k", 44100, 44100, 2.0f },
	{ "down3", 144000, 48000, 1.0f },
	{ "down3-pitch", 48000, 48000, 3.0f },
	{ "down4", 192000, 48000, 1.0f },
	{ "down6", 48000, 8000, 1.0f },
	{ "up2", 24000, 48000, 1.0f },
	{ "up2-pitch", 48000, 48000, 0.5f },
	{ "up2-44k", 22050, 44100, 1.0f },
	{ "up3", 16000, 48000, 1.0f },
	{ "up4", 12000, 48000, 1.0f },
	{ "up6", 8000, 48000, 1.0f },
};

// One case per path on purpose, test runs must stay light.
// 8191 is no multiple of 2, 3, 4 or 6 and crosses the 4096-frame chunks of the mixer (the end of frame skipping, the skipping grid at a chunk border)
static const AudioFrameIndex SourceFrameCounts[] = { 8191 };

// No multiple of 2, 3, 4 or 6 (every mix call of frame duplication ends inside a frame)
static const AudioFrameIndex ChunkFrameCounts[] = { 1001 };

// With repeat the end of the source and the start of the next pass are checked as well
static const bool RepeatModes[] = { true };

static const AudioChannelIndex ChannelCount = 2;
static const uint32_t ValuePeriod = 8191;
static const float ValueScale = 16384.0f;

static float SourceValue(const AudioFrameIndex frameIndex) {
	float result = (float)((frameIndex % ValuePeriod) + 1) / ValueScale;
	return(result);
}

typedef enum FailureKind {
	FailureKind_None = 0,
	// The play position went past the end of the source
	FailureKind_PositionPastEnd,
	// A frame inside the track came out silent
	FailureKind_SilentGap,
	// A frame came from the wrong source frame or carried a wrong value
	FailureKind_WrongFrame,
	// The track went on after its end, or a repeat pass was too short or too long
	FailureKind_WrongLength,
	// A play item without repeat did not finish
	FailureKind_NotFinished,
	FailureKind_Count,
} FailureKind;

static const char *FailureKindNames[] = { "none", "position past end", "silent gap", "wrong frame", "wrong length", "not finished" };

typedef struct CaseResult {
	FailureKind kind;
	AudioFrameIndex firstBadOutputFrame;
	int64_t expectedSourceFrame;
	float gotLeft;
} CaseResult;

// The source frame behind every output frame of one pass, the ratio decides the path exactly as ResampleChunk does
static size_t BuildExpectedSourceFrames(const AudioHertz playedRate, const AudioHertz targetRate, const AudioFrameIndex sourceFrameCount, int64_t *outSourceFrames) {
	size_t count = 0;
	if(playedRate == targetRate) {
		for(AudioFrameIndex frameIndex = 0; frameIndex < sourceFrameCount; ++frameIndex) {
			outSourceFrames[count++] = frameIndex;
		}
	} else if(playedRate > targetRate) {
		uint32_t factor = playedRate / targetRate;
		for(AudioFrameIndex frameIndex = 0; frameIndex < sourceFrameCount; frameIndex += factor) {
			outSourceFrames[count++] = frameIndex;
		}
	} else {
		uint32_t factor = targetRate / playedRate;
		for(AudioFrameIndex frameIndex = 0; frameIndex < sourceFrameCount; ++frameIndex) {
			for(uint32_t copyIndex = 0; copyIndex < factor; ++copyIndex) {
				outSourceFrames[count++] = frameIndex;
			}
		}
	}
	return(count);
}

static CaseResult RunCase(const RatePair *pair, const AudioFrameIndex sourceFrameCount, const AudioFrameIndex chunkFrameCount, const bool repeat) {
	CaseResult result = fplZeroInit;

	fplAudioFormat targetFormat = fplZeroInit;
	targetFormat.type = fplAudioFormatType_F32;
	targetFormat.sampleRate = pair->targetRate;
	targetFormat.channels = ChannelCount;

	AudioSystem *audioSys = (AudioSystem *)calloc(1, sizeof(AudioSystem));
	AudioSystemInit(audioSys, &targetFormat);

	AudioSource *source = AudioSystemAllocateSource(audioSys, ChannelCount, pair->sourceRate, fplAudioFormatType_F32, sourceFrameCount);
	float *sourceSamples = (float *)source->buffer.samples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < sourceFrameCount; ++frameIndex) {
		float value = SourceValue(frameIndex);
		sourceSamples[frameIndex * ChannelCount + 0] = value;
		sourceSamples[frameIndex * ChannelCount + 1] = -value;
	}
	AudioSystemAddSource(audioSys, source);
	AudioSystemPlaySource(audioSys, source, repeat, 1.0f, pair->pitch);

	AudioHertz playedRate = ResolvePitchedSampleRate(pair->sourceRate, pair->pitch);
	uint32_t maxFactor = (playedRate > pair->targetRate) ? 1 : (pair->targetRate / playedRate);
	int64_t *passSourceFrames = (int64_t *)malloc(sizeof(int64_t) * ((size_t)sourceFrameCount * maxFactor + 1));
	size_t passFrameCount = BuildExpectedSourceFrames(playedRate, pair->targetRate, sourceFrameCount, passSourceFrames);

	// Two passes with repeat, one pass and some silence without
	const size_t passCount = repeat ? 2 : 1;
	const size_t tailFrameCount = (size_t)chunkFrameCount * 2 + 64;
	size_t totalFrameCount = passFrameCount * passCount + tailFrameCount;
	float *output = (float *)calloc(totalFrameCount * ChannelCount, sizeof(float));
	float *chunk = (float *)malloc(sizeof(float) * chunkFrameCount * ChannelCount);

	size_t mixedFrameCount = 0;
	while(mixedFrameCount < totalFrameCount && result.kind == FailureKind_None) {
		size_t remainingFrameCount = totalFrameCount - mixedFrameCount;
		AudioFrameIndex framesToMix = (AudioFrameIndex)fplMin((size_t)chunkFrameCount, remainingFrameCount);
		AudioSystemWriteFrames(audioSys, chunk, &targetFormat, framesToMix, true);
		memcpy(output + mixedFrameCount * ChannelCount, chunk, sizeof(float) * framesToMix * ChannelCount);
		mixedFrameCount += framesToMix;

		for(AudioPlayItem *item = audioSys->playItems.first; item != fpl_null; item = item->next) {
			if(item->framesPlayed[0] > sourceFrameCount) {
				result.kind = FailureKind_PositionPastEnd;
				result.firstBadOutputFrame = (AudioFrameIndex)mixedFrameCount;
				result.expectedSourceFrame = item->framesPlayed[0];
			}
		}
	}

	for(size_t frameIndex = 0; frameIndex < totalFrameCount && result.kind == FailureKind_None; ++frameIndex) {
		float gotLeft = output[frameIndex * ChannelCount + 0];
		float gotRight = output[frameIndex * ChannelCount + 1];
		size_t passIndex = frameIndex / passFrameCount;
		int64_t expectedSourceFrame = -1;
		if(passIndex < passCount) {
			expectedSourceFrame = passSourceFrames[frameIndex % passFrameCount];
		}
		float expectedLeft = (expectedSourceFrame >= 0) ? SourceValue((AudioFrameIndex)expectedSourceFrame) : 0.0f;
		bool isSame = (gotLeft == expectedLeft) && (gotRight == -expectedLeft);
		if(!isSame) {
			if(expectedSourceFrame < 0) {
				result.kind = repeat ? FailureKind_None : FailureKind_WrongLength;
				if(repeat) {
					// A third pass may start here, only the first two passes are checked
					break;
				}
			} else if(gotLeft == 0.0f && gotRight == 0.0f) {
				result.kind = FailureKind_SilentGap;
			} else {
				result.kind = FailureKind_WrongFrame;
			}
			result.firstBadOutputFrame = (AudioFrameIndex)frameIndex;
			result.expectedSourceFrame = expectedSourceFrame;
			result.gotLeft = gotLeft;
		}
	}

	if(result.kind == FailureKind_None && !repeat && audioSys->playItems.count > 0) {
		result.kind = FailureKind_NotFinished;
	}

	free(chunk);
	free(output);
	free(passSourceFrames);
	AudioSystemShutdown(audioSys);
	free(audioSys);
	return(result);
}

int main(int argc, char **argv) {
	bool isVerbose = (argc > 1) && (strcmp(argv[1], "-v") == 0);

	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}

	size_t totalCaseCount = 0;
	size_t totalFailedCount = 0;
	printf("%-16s %6s %6s", "path", "cases", "failed");
	for(int kindIndex = FailureKind_PositionPastEnd; kindIndex < FailureKind_Count; ++kindIndex) {
		printf(" | %s", FailureKindNames[kindIndex]);
	}
	printf("\n");

	for(size_t pairIndex = 0; pairIndex < fplArrayCount(RatePairs); ++pairIndex) {
		const RatePair *pair = &RatePairs[pairIndex];
		size_t kindCounts[FailureKind_Count] = { 0 };
		size_t caseCount = 0;
		for(size_t sourceIndex = 0; sourceIndex < fplArrayCount(SourceFrameCounts); ++sourceIndex) {
			for(size_t chunkIndex = 0; chunkIndex < fplArrayCount(ChunkFrameCounts); ++chunkIndex) {
				for(size_t repeatIndex = 0; repeatIndex < fplArrayCount(RepeatModes); ++repeatIndex) {
					bool repeat = RepeatModes[repeatIndex];
					CaseResult caseResult = RunCase(pair, SourceFrameCounts[sourceIndex], ChunkFrameCounts[chunkIndex], repeat);
					++caseCount;
					++kindCounts[caseResult.kind];
					if(caseResult.kind != FailureKind_None && isVerbose) {
						printf("  FAIL %s source=%u chunk=%u repeat=%d: %s at output frame %u (expected source frame %lld, got left %.8f)\n", pair->name, SourceFrameCounts[sourceIndex], ChunkFrameCounts[chunkIndex], repeat, FailureKindNames[caseResult.kind], caseResult.firstBadOutputFrame, (long long)caseResult.expectedSourceFrame, caseResult.gotLeft);
					}
				}
			}
		}
		size_t failedCount = caseCount - kindCounts[FailureKind_None];
		printf("%-16s %6zu %6zu", pair->name, caseCount, failedCount);
		for(int kindIndex = FailureKind_PositionPastEnd; kindIndex < FailureKind_Count; ++kindIndex) {
			printf(" | %zu", kindCounts[kindIndex]);
		}
		printf("\n");
		totalCaseCount += caseCount;
		totalFailedCount += failedCount;
	}
	printf("total: %zu cases, %zu failed\n", totalCaseCount, totalFailedCount);

	fplPlatformRelease();
	return(totalFailedCount > 0 ? 1 : 0);
}
