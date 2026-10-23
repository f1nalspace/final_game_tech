/*
Checks the loop regions of final_audiosystem.h frame by frame.

A source in memory holds its own frame index in every sample (F32, (index + 1) / 2^24, exact on the passthrough path at 48 kHz), so every output frame tells which source frame the mixer played.
A model of the semantics plays the same item frame by frame, the mixer must give exactly its frames for every call size: intro once, then the region, the outro after a release or a seek, the end of the source.

Cases (each with the call sizes 512, 441, 37 and 6000):
- intro-once, no-repeat, whole-file, region-to-end, one-frame, tiny-region: regions alone, with and without repeat.
- release, release-in-intro, repeat-again: AudioSystemSetPlayItemRepeat in the region, in the intro and again in the outro.
- seek-outro, seek-past-end, seek-past-end-no-repeat, seek-intro: AudioSystemSetPlayItemPosition with a region.
- change-region, clear-region, invalid-region: AudioSystemSetPlayItemLoop while the item plays, a region past the source is dropped and the setter refuses a start that is not below the end.
- source-default, source-invalid: the region of the AudioSource is the default of every play, one past the source is left out.
- preview: a preview call (advance = false) in between moves nothing.
- pitch-2, pitch-0.5: frame skipping and frame duplication with a region, every frame of a pass in order and every jump from the loop end to the loop start.
- sine-stream: a sine wave in memory and the same sine wave as a stream give byte-identical output with a region, a tiny region (the ring is used again), a release and pitches 1, 0.5, 2 and 0.75 (SinC). An endless stream drops every region.

Build against the additions to test, with assertions (the mixer checks its own invariants):
gcc -std=c99 -O2 -DFPL_FORCE_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies loop_region_test.c -o loop_region_test -lm -ldl -lpthread
Usage: loop_region_test
Output: one line per case "<name>: frames=<n> <ok|FAILED>" (sine-stream without frames), then "total: <n> failed checks".
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>
#include <string.h>

static const AudioHertz OutputSampleRate = 48000;
static const AudioChannelIndex StereoChannelCount = 2;

// A sample of the index source is (frame index + 1) / 2^24, exact in a float for every index below 2^24 - 1, 0 is silence
static const float IndexSampleScale = 16777216.0f;
static const int64_t SilentFrameIndex = -1;

// The index source of the cases: intro [0, 3001), region [3001, 7002), outro [7002, 20000)
static const AudioFrameIndex IndexSourceFrameCount = 20000;
static const AudioFrameIndex DefaultLoopStartFrame = 3001;
static const AudioFrameIndex DefaultLoopEndFrame = 7002;

// The call sizes every case runs with: a power of two, an odd one, a tiny one and one above the conversion buffer
static const AudioFrameIndex CallFrameCounts[] = { 512, 441, 37, 6000 };

// Stops printing after this many wrong frames of a case
static const uint32_t MaxReportedFrameErrorCount = 4;

typedef struct LoopTest {
	AudioSystem *audioSys;
	fplAudioFormat targetFormat;
	uint32_t failedCheckCount;
	uint32_t reportedFrameErrorCount;
} LoopTest;

static void CheckTrue(LoopTest *test, const bool condition, const char *description) {
	if(!condition) {
		printf("  check failed: %s\n", description);
		++test->failedCheckCount;
	}
}

// The model of a play item with a loop region, it plays one source frame per step
typedef struct LoopModel {
	AudioFrameIndex position;
	AudioFrameIndex frameCount;
	AudioFrameIndex loopStartFrame;
	AudioFrameIndex loopEndFrame;
	bool isRepeat;
	bool isFinished;
} LoopModel;

// The source frame the model plays next, -1 once it has ended
static int64_t ModelNextFrame(LoopModel *model) {
	if(model->isFinished) {
		return(SilentFrameIndex);
	}
	const int64_t result = model->position;
	const bool isInsideLoopRegion = model->isRepeat && (model->loopEndFrame > 0) && (model->position < model->loopEndFrame);
	const AudioFrameIndex endFrameIndex = isInsideLoopRegion ? model->loopEndFrame : model->frameCount;
	++model->position;
	if(model->position == endFrameIndex) {
		if(model->isRepeat) {
			model->position = model->loopStartFrame;
		} else {
			model->isFinished = true;
		}
	}
	return(result);
}

static void ModelSetPosition(LoopModel *model, const AudioFrameIndex frameIndex) {
	if(model->isFinished) {
		return;
	}
	if(frameIndex >= model->frameCount) {
		if(model->isRepeat) {
			model->position = model->loopStartFrame;
		} else {
			model->isFinished = true;
		}
		return;
	}
	model->position = frameIndex;
}

static void ModelSetLoop(LoopModel *model, const AudioFrameIndex loopStartFrame, const AudioFrameIndex loopEndFrame) {
	if(loopEndFrame == 0) {
		model->loopStartFrame = 0;
		model->loopEndFrame = 0;
		return;
	}
	const bool isValid = (loopStartFrame < loopEndFrame) && (loopEndFrame <= model->frameCount);
	if(isValid) {
		model->loopStartFrame = loopStartFrame;
		model->loopEndFrame = loopEndFrame;
	}
}

typedef enum LoopActionKind {
	LoopActionKind_None = 0,
	LoopActionKind_SetRepeat,
	LoopActionKind_SetPosition,
	LoopActionKind_SetLoop,
	LoopActionKind_Preview,
} LoopActionKind;

// Something a case does before the call that starts at an output frame
typedef struct LoopAction {
	AudioFrameIndex outputFrameIndex;
	LoopActionKind kind;
	AudioFrameIndex firstValue;
	AudioFrameIndex secondValue;
	bool flag;
	// SetLoop only: what the setter must return
	bool isAccepted;
} LoopAction;

#define MAX_LOOP_ACTION_COUNT 4

typedef struct LoopCase {
	const char *name;
	AudioFrameIndex outputFrameCount;
	// The region of the source, the default of the play
	AudioFrameIndex sourceLoopStartFrame;
	AudioFrameIndex sourceLoopEndFrame;
	// The region the item gets by the setter right after the play, none when the end is 0
	AudioFrameIndex itemLoopStartFrame;
	AudioFrameIndex itemLoopEndFrame;
	bool isRepeat;
	LoopAction actions[MAX_LOOP_ACTION_COUNT];
	size_t actionCount;
} LoopCase;

static AudioSource *CreateIndexSource(LoopTest *test) {
	AudioSource *source = AudioSystemAllocateSource(test->audioSys, StereoChannelCount, OutputSampleRate, fplAudioFormatType_F32, IndexSourceFrameCount);
	CheckTrue(test, source != fpl_null, "allocate the index source");
	if(source == fpl_null) {
		return(fpl_null);
	}
	float *samples = (float *)source->buffer.samples;
	for(AudioFrameIndex frameIndex = 0; frameIndex < IndexSourceFrameCount; ++frameIndex) {
		const float indexValue = (float)(frameIndex + 1) / IndexSampleScale;
		samples[frameIndex * StereoChannelCount + 0] = indexValue;
		samples[frameIndex * StereoChannelCount + 1] = -indexValue;
	}
	bool isAdded = AudioSystemAddSource(test->audioSys, source);
	CheckTrue(test, isAdded, "add the index source");
	return(source);
}

// The source frame an output frame of the index source holds, -1 for silence, -2 when the frame is no frame of the source
static int64_t ReadOutputFrameIndex(const float *outputSamples, const AudioFrameIndex outputFrameIndex) {
	const int64_t invalidFrameIndex = -2;
	const float leftValue = outputSamples[outputFrameIndex * StereoChannelCount + 0];
	const float rightValue = outputSamples[outputFrameIndex * StereoChannelCount + 1];
	if(leftValue == 0.0f && rightValue == 0.0f) {
		return(SilentFrameIndex);
	}
	const float scaledValue = leftValue * IndexSampleScale;
	const int64_t indexPlusOne = (int64_t)scaledValue;
	const bool isExact = ((float)indexPlusOne == scaledValue) && (rightValue == -leftValue) && (indexPlusOne >= 1);
	if(!isExact) {
		return(invalidFrameIndex);
	}
	return(indexPlusOne - 1);
}

// The play item of an id as AudioSystemGetPlayItems returns it, false when it is not there
static bool FindPlayItem(LoopTest *test, const AudioPlayItemID playId, AudioPlayItem *outItem) {
	AudioPlayItem items[FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT];
	size_t itemCount = AudioSystemGetPlayItems(test->audioSys, items, FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT);
	for(size_t itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
		if(items[itemIndex].id.value == playId.value) {
			*outItem = items[itemIndex];
			return(true);
		}
	}
	return(false);
}

// Mixes frameCount frames in calls of at most callFrameCount frames into outputSamples, F32 stereo
static void MixFrames(LoopTest *test, float *outputSamples, const AudioFrameIndex frameCount, const AudioFrameIndex callFrameCount, const bool advance) {
	AudioFrameIndex mixedFrameCount = 0;
	while(mixedFrameCount < frameCount) {
		const AudioFrameIndex remainingFrameCount = frameCount - mixedFrameCount;
		const AudioFrameIndex framesToMix = fplMin(callFrameCount, remainingFrameCount);
		float *callSamples = outputSamples + (size_t)mixedFrameCount * StereoChannelCount;
		AudioFrameIndex writtenFrameCount = AudioSystemWriteFrames(test->audioSys, callSamples, &test->targetFormat, framesToMix, advance);
		CheckTrue(test, writtenFrameCount == framesToMix, "the mixer writes every frame");
		mixedFrameCount += framesToMix;
	}
}

// Applies an action to the play item and to the model
static void ApplyLoopAction(LoopTest *test, const LoopAction *action, const AudioPlayItemID playId, LoopModel *model, const AudioFrameIndex callFrameCount) {
	switch(action->kind) {
		case LoopActionKind_SetRepeat:
		{
			bool isSent = AudioSystemSetPlayItemRepeat(test->audioSys, playId, action->flag);
			CheckTrue(test, isSent || model->isFinished, "send the repeat");
			model->isRepeat = action->flag;
		} break;

		case LoopActionKind_SetPosition:
		{
			bool isSent = AudioSystemSetPlayItemPosition(test->audioSys, playId, action->firstValue);
			CheckTrue(test, isSent || model->isFinished, "send the position");
			ModelSetPosition(model, action->firstValue);
		} break;

		case LoopActionKind_SetLoop:
		{
			bool isSent = AudioSystemSetPlayItemLoop(test->audioSys, playId, action->firstValue, action->secondValue);
			CheckTrue(test, isSent == action->isAccepted, "the loop setter accepts or refuses the region");
			if(isSent) {
				ModelSetLoop(model, action->firstValue, action->secondValue);
			}
		} break;

		case LoopActionKind_Preview:
		{
			// A preview leaves every position as it was, the model does not move
			const AudioFrameIndex previewFrameCount = action->firstValue;
			float *previewSamples = (float *)fplMemoryAllocate((size_t)previewFrameCount * StereoChannelCount * sizeof(float));
			if(previewSamples != fpl_null) {
				MixFrames(test, previewSamples, previewFrameCount, callFrameCount, false);
				fplMemoryFree(previewSamples);
			}
		} break;

		default:
			break;
	}
}

// Plays the index source as the case says with one call size and compares every output frame with the model
static void RunIndexCase(LoopTest *test, AudioSource *source, const LoopCase *loopCase, const AudioFrameIndex callFrameCount) {
	source->loopStartFrame = loopCase->sourceLoopStartFrame;
	source->loopEndFrame = loopCase->sourceLoopEndFrame;
	AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, loopCase->isRepeat, 1.0f, AudioSystemDefaultPitch);
	CheckTrue(test, playId.value != 0, "play the index source");
	if(playId.value == 0) {
		return;
	}

	LoopModel model = fplZeroInit;
	model.frameCount = IndexSourceFrameCount;
	model.isRepeat = loopCase->isRepeat;
	ModelSetLoop(&model, loopCase->sourceLoopStartFrame, loopCase->sourceLoopEndFrame);

	// The item has the region of the source at once, before the mixer took it
	AudioPlayItem item = fplZeroInit;
	bool hasItem = FindPlayItem(test, playId, &item);
	CheckTrue(test, hasItem && item.loopStartFrame == model.loopStartFrame && item.loopEndFrame == model.loopEndFrame, "the play gets the region of the source");

	if(loopCase->itemLoopEndFrame > 0) {
		bool isSent = AudioSystemSetPlayItemLoop(test->audioSys, playId, loopCase->itemLoopStartFrame, loopCase->itemLoopEndFrame);
		CheckTrue(test, isSent, "send the region of the item");
		ModelSetLoop(&model, loopCase->itemLoopStartFrame, loopCase->itemLoopEndFrame);
	}

	const AudioFrameIndex outputFrameCount = loopCase->outputFrameCount;
	float *outputSamples = (float *)fplMemoryAllocate((size_t)outputFrameCount * StereoChannelCount * sizeof(float));
	CheckTrue(test, outputSamples != fpl_null, "allocate the output");
	if(outputSamples == fpl_null) {
		return;
	}

	uint32_t frameErrorCount = 0;
	size_t nextActionIndex = 0;
	AudioFrameIndex outputFrameIndex = 0;
	while(outputFrameIndex < outputFrameCount) {
		// Every action lands on the first frame of a call
		while(nextActionIndex < loopCase->actionCount && loopCase->actions[nextActionIndex].outputFrameIndex == outputFrameIndex) {
			ApplyLoopAction(test, &loopCase->actions[nextActionIndex], playId, &model, callFrameCount);
			++nextActionIndex;
		}
		AudioFrameIndex segmentEndFrameIndex = outputFrameCount;
		if(nextActionIndex < loopCase->actionCount) {
			segmentEndFrameIndex = loopCase->actions[nextActionIndex].outputFrameIndex;
		}
		const AudioFrameIndex segmentFrameCount = segmentEndFrameIndex - outputFrameIndex;
		float *segmentSamples = outputSamples + (size_t)outputFrameIndex * StereoChannelCount;
		MixFrames(test, segmentSamples, segmentFrameCount, callFrameCount, true);

		// The region the item has after the commands of this segment, the model has applied them as well
		if(!model.isFinished) {
			hasItem = FindPlayItem(test, playId, &item);
			CheckTrue(test, !hasItem || (item.loopStartFrame == model.loopStartFrame && item.loopEndFrame == model.loopEndFrame), "the published region is the one of the model");
		}

		for(AudioFrameIndex segmentFrameIndex = 0; segmentFrameIndex < segmentFrameCount; ++segmentFrameIndex) {
			const AudioFrameIndex checkedFrameIndex = outputFrameIndex + segmentFrameIndex;
			const int64_t expectedFrameIndex = ModelNextFrame(&model);
			const int64_t actualFrameIndex = ReadOutputFrameIndex(outputSamples, checkedFrameIndex);
			if(actualFrameIndex != expectedFrameIndex) {
				if(test->reportedFrameErrorCount < MaxReportedFrameErrorCount) {
					printf("  %s, call %u: output frame %u is source frame %lld, the model expects %lld\n", loopCase->name, callFrameCount, checkedFrameIndex, (long long)actualFrameIndex, (long long)expectedFrameIndex);
					++test->reportedFrameErrorCount;
				}
				++frameErrorCount;
			}
		}
		outputFrameIndex = segmentEndFrameIndex;
	}
	CheckTrue(test, frameErrorCount == 0, "every output frame is the frame of the model");

	// An item that has ended is gone, one that plays on is stopped for the next run
	hasItem = FindPlayItem(test, playId, &item);
	CheckTrue(test, hasItem == !model.isFinished, "the item has ended exactly when the model has");
	if(hasItem) {
		AudioSystemStopOne(test->audioSys, playId);
		float stopSamples[2];
		const AudioFrameIndex stopFrameCount = 1;
		MixFrames(test, stopSamples, stopFrameCount, stopFrameCount, true);
	}
	fplMemoryFree(outputSamples);
}

// The cases of the index source, see the comment at the top
static const LoopCase IndexLoopCases[] = {
	{ "intro-once", 60000, 0, 0, 3001, 7002, true, { { 0 } }, 0 },
	{ "no-repeat", 30000, 0, 0, 3001, 7002, false, { { 0 } }, 0 },
	{ "whole-file", 50000, 0, 0, 0, 20000, true, { { 0 } }, 0 },
	{ "region-to-end", 50000, 0, 0, 3001, 20000, true, { { 0 } }, 0 },
	{ "one-frame", 6000, 0, 0, 5000, 5001, true, { { 0 } }, 0 },
	{ "tiny-region", 20000, 0, 0, 5000, 5003, true, { { 0 } }, 0 },
	{ "release", 40000, 0, 0, 3001, 7002, true, { { 10000, LoopActionKind_SetRepeat, 0, 0, false, false } }, 1 },
	{ "release-in-intro", 30000, 0, 0, 3001, 7002, true, { { 1000, LoopActionKind_SetRepeat, 0, 0, false, false } }, 1 },
	{ "repeat-again", 50000, 0, 0, 3001, 7002, true, { { 10000, LoopActionKind_SetRepeat, 0, 0, false, false }, { 16000, LoopActionKind_SetRepeat, 0, 0, true, false } }, 2 },
	{ "seek-outro", 40000, 0, 0, 3001, 7002, true, { { 5000, LoopActionKind_SetPosition, 15000, 0, false, false } }, 1 },
	{ "seek-past-end", 30000, 0, 0, 3001, 7002, true, { { 5000, LoopActionKind_SetPosition, 25000, 0, false, false } }, 1 },
	{ "seek-past-end-no-repeat", 20000, 0, 0, 3001, 7002, false, { { 5000, LoopActionKind_SetPosition, 25000, 0, false, false } }, 1 },
	{ "seek-intro", 40000, 0, 0, 3001, 7002, true, { { 12000, LoopActionKind_SetPosition, 100, 0, false, false } }, 1 },
	{ "change-region", 50000, 0, 0, 3001, 7002, true, { { 8000, LoopActionKind_SetLoop, 5000, 9000, false, true }, { 30000, LoopActionKind_SetLoop, 11000, 13000, false, true } }, 2 },
	{ "clear-region", 60000, 0, 0, 3001, 7002, true, { { 8000, LoopActionKind_SetLoop, 0, 0, false, true } }, 1 },
	{ "invalid-region", 40000, 0, 0, 3001, 7002, true, { { 8000, LoopActionKind_SetLoop, 5000, 20001, false, true }, { 9000, LoopActionKind_SetLoop, 7000, 7000, false, false }, { 10000, LoopActionKind_SetLoop, 9000, 8000, false, false } }, 3 },
	{ "source-default", 60000, 3001, 7002, 0, 0, true, { { 0 } }, 0 },
	{ "source-invalid", 50000, 3001, 20001, 0, 0, true, { { 0 } }, 0 },
	{ "preview", 40000, 0, 0, 3001, 7002, true, { { 6000, LoopActionKind_Preview, 3000, 0, false, false }, { 20000, LoopActionKind_Preview, 9000, 0, false, false } }, 2 },
};

static void RunIndexCases(LoopTest *test, AudioSource *source) {
	for(size_t caseIndex = 0; caseIndex < fplArrayCount(IndexLoopCases); ++caseIndex) {
		const LoopCase *loopCase = &IndexLoopCases[caseIndex];
		const uint32_t failedBefore = test->failedCheckCount;
		test->reportedFrameErrorCount = 0;
		for(size_t callIndex = 0; callIndex < fplArrayCount(CallFrameCounts); ++callIndex) {
			RunIndexCase(test, source, loopCase, CallFrameCounts[callIndex]);
		}
		const bool isCaseOk = (test->failedCheckCount == failedBefore);
		printf("%s: frames=%u %s\n", loopCase->name, loopCase->outputFrameCount, isCaseOk ? "ok" : "FAILED");
	}
}

/**
* Plays the index source at a pitch that skips or duplicates frames (an even ratio) and checks every pass of the region:
* within a pass the source frames follow each other in steps of the skip factor or come in runs of the copy factor, and every jump goes from the last frames before the loop end to the loop start.
*/
static void RunEvenPitchCase(LoopTest *test, AudioSource *source, const char *name, const float pitch, const AudioFrameIndex skipFactor, const AudioFrameIndex copyFactor) {
	const AudioFrameIndex outputFrameCount = 60000;
	const AudioFrameIndex minPassCount = 3;
	const uint32_t failedBefore = test->failedCheckCount;
	test->reportedFrameErrorCount = 0;
	float *outputSamples = (float *)fplMemoryAllocate((size_t)outputFrameCount * StereoChannelCount * sizeof(float));
	CheckTrue(test, outputSamples != fpl_null, "allocate the output");
	if(outputSamples == fpl_null) {
		return;
	}
	source->loopStartFrame = 0;
	source->loopEndFrame = 0;
	for(size_t callIndex = 0; callIndex < fplArrayCount(CallFrameCounts); ++callIndex) {
		const AudioFrameIndex callFrameCount = CallFrameCounts[callIndex];
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, true, 1.0f, pitch);
		bool isSent = AudioSystemSetPlayItemLoop(test->audioSys, playId, DefaultLoopStartFrame, DefaultLoopEndFrame);
		CheckTrue(test, playId.value != 0 && isSent, "play the index source with a region");
		MixFrames(test, outputSamples, outputFrameCount, callFrameCount, true);
		AudioSystemStopOne(test->audioSys, playId);
		float stopSamples[2];
		const AudioFrameIndex stopFrameCount = 1;
		MixFrames(test, stopSamples, stopFrameCount, stopFrameCount, true);

		int64_t previousFrameIndex = ReadOutputFrameIndex(outputSamples, 0);
		CheckTrue(test, previousFrameIndex == 0, "the first output frame is the first source frame");
		uint32_t runLength = 1;
		uint32_t jumpCount = 0;
		uint32_t errorCount = 0;
		for(AudioFrameIndex outputFrameIndex = 1; outputFrameIndex < outputFrameCount; ++outputFrameIndex) {
			const int64_t frameIndex = ReadOutputFrameIndex(outputSamples, outputFrameIndex);
			const bool isSameFrame = (frameIndex == previousFrameIndex);
			const bool isNextFrame = (frameIndex == previousFrameIndex + (int64_t)skipFactor);
			const bool isJump = (frameIndex == (int64_t)DefaultLoopStartFrame) && (previousFrameIndex >= (int64_t)(DefaultLoopEndFrame - skipFactor)) && (previousFrameIndex < (int64_t)DefaultLoopEndFrame);
			bool isValid = false;
			if(isSameFrame) {
				++runLength;
				isValid = (runLength <= copyFactor);
			} else if(isNextFrame || isJump) {
				// A run before the step is complete, also the one before a jump
				isValid = (runLength == copyFactor);
				runLength = 1;
				if(isJump) {
					++jumpCount;
				}
			}
			if(!isValid) {
				if(test->reportedFrameErrorCount < MaxReportedFrameErrorCount) {
					printf("  %s, call %u: output frame %u is source frame %lld after %lld (run %u)\n", name, callFrameCount, outputFrameIndex, (long long)frameIndex, (long long)previousFrameIndex, runLength);
					++test->reportedFrameErrorCount;
				}
				++errorCount;
			}
			previousFrameIndex = frameIndex;
		}
		CheckTrue(test, errorCount == 0, "every pass plays the region in order and jumps from its end to its start");
		CheckTrue(test, jumpCount >= minPassCount, "the region loops several times");
	}
	fplMemoryFree(outputSamples);
	const bool isCaseOk = (test->failedCheckCount == failedBefore);
	printf("%s: frames=%u %s\n", name, outputFrameCount, isCaseOk ? "ok" : "FAILED");
}

// A region with an action, played once by the sine wave in memory and once by the sine wave stream
typedef struct SineLoopRun {
	float pitch;
	AudioFrameIndex callFrameCount;
	AudioFrameIndex loopStartFrame;
	AudioFrameIndex loopEndFrame;
	// The output frame of the release, 0 for none
	AudioFrameIndex releaseOutputFrameIndex;
} SineLoopRun;

// Plays one run and writes its S16 output, the release lands on the first frame of a call
static void MixSineRun(LoopTest *test, const AudioSource *source, const SineLoopRun *run, int16_t *outputSamples, const AudioFrameIndex outputFrameCount) {
	AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, true, 1.0f, run->pitch);
	bool isSent = AudioSystemSetPlayItemLoop(test->audioSys, playId, run->loopStartFrame, run->loopEndFrame);
	CheckTrue(test, playId.value != 0 && isSent, "play the sine wave with a region");
	AudioFrameIndex mixedFrameCount = 0;
	while(mixedFrameCount < outputFrameCount) {
		if(run->releaseOutputFrameIndex > 0 && mixedFrameCount == run->releaseOutputFrameIndex) {
			AudioSystemSetPlayItemRepeat(test->audioSys, playId, false);
		}
		AudioFrameIndex callEndFrameIndex = fplMin(mixedFrameCount + run->callFrameCount, outputFrameCount);
		if(run->releaseOutputFrameIndex > mixedFrameCount && run->releaseOutputFrameIndex < callEndFrameIndex) {
			callEndFrameIndex = run->releaseOutputFrameIndex;
		}
		const AudioFrameIndex framesToMix = callEndFrameIndex - mixedFrameCount;
		int16_t *callSamples = outputSamples + (size_t)mixedFrameCount * StereoChannelCount;
		AudioSystemWriteFrames(test->audioSys, callSamples, &test->targetFormat, framesToMix, true);
		mixedFrameCount = callEndFrameIndex;
	}
	AudioSystemStopOne(test->audioSys, playId);
	int16_t stopSamples[2];
	AudioSystemWriteFrames(test->audioSys, stopSamples, &test->targetFormat, 1, true);
}

// The sine waves of the case, in an audio system with S16 output like the sine wave itself
static void RunSineStreamRuns(LoopTest *test) {
	const double sineSeconds = 3.0;
	const double sineToneVolume = 0.25;
	const AudioHertz sineFrequency = 440;
	const double roundingOffset = 0.5;
	const AudioFrameIndex outputFrameCount = 400000;

	AudioSineWaveData sineWave = fplZeroInit;
	sineWave.duration = sineSeconds;
	sineWave.toneVolume = sineToneVolume;
	sineWave.frequency = sineFrequency;
	const double frameCountValue = (double)OutputSampleRate * sineSeconds + roundingOffset;
	const AudioFrameIndex frameCount = (AudioFrameIndex)frameCountValue;
	AudioSource *memorySine = AudioSystemAllocateSource(test->audioSys, StereoChannelCount, OutputSampleRate, fplAudioFormatType_S16, frameCount);
	AudioSource *streamSine = AudioSystemCreateSineStreamSource(test->audioSys, StereoChannelCount, OutputSampleRate, &sineWave);
	CheckTrue(test, memorySine != fpl_null && streamSine != fpl_null, "create both sine waves");
	if(memorySine == fpl_null || streamSine == fpl_null) {
		return;
	}
	AudioSineWaveData memoryWave = sineWave;
	AudioGenerateSineWave(&memoryWave, memorySine->buffer.samples, memorySine->format.format, memorySine->format.sampleRate, memorySine->format.channels, frameCount);
	bool isMemoryAdded = AudioSystemAddSource(test->audioSys, memorySine);
	bool isStreamAdded = AudioSystemAddSource(test->audioSys, streamSine);
	CheckTrue(test, isMemoryAdded && isStreamAdded, "add both sine waves");

	const size_t outputByteCount = (size_t)outputFrameCount * StereoChannelCount * sizeof(int16_t);
	int16_t *memoryOutput = (int16_t *)fplMemoryAllocate(outputByteCount);
	int16_t *streamOutput = (int16_t *)fplMemoryAllocate(outputByteCount);
	int16_t *plainOutput = (int16_t *)fplMemoryAllocate(outputByteCount);
	CheckTrue(test, memoryOutput != fpl_null && streamOutput != fpl_null && plainOutput != fpl_null, "allocate the outputs");
	if(memoryOutput == fpl_null || streamOutput == fpl_null || plainOutput == fpl_null) {
		return;
	}

	// Loop points off the sine period, a tiny region shorter than one chunk, and a release in the third pass
	const SineLoopRun runs[] = {
		{ 1.0f, 512, 12345, 67891, 0 },
		{ 1.0f, 441, 12345, 67891, 250000 },
		{ 1.0f, 512, 50000, 50100, 0 },
		{ 0.5f, 512, 12345, 67891, 300000 },
		{ 2.0f, 441, 12345, 67891, 0 },
		{ 2.0f, 512, 23457, 23561, 0 },
		{ 0.75f, 512, 12345, 67891, 200000 },
	};
	for(size_t runIndex = 0; runIndex < fplArrayCount(runs); ++runIndex) {
		const SineLoopRun *run = &runs[runIndex];
		MixSineRun(test, memorySine, run, memoryOutput, outputFrameCount);
		MixSineRun(test, streamSine, run, streamOutput, outputFrameCount);
		bool isSame = (memcmp(memoryOutput, streamOutput, outputByteCount) == 0);
		if(!isSame) {
			printf("  sine-stream: pitch %.2f, call %u, region %u-%u: the stream differs from the sine wave in memory\n", run->pitch, run->callFrameCount, run->loopStartFrame, run->loopEndFrame);
		}
		CheckTrue(test, isSame, "the stream gives the same output as the sine wave in memory");

		// The region changes the output, the plain repeat gives another one
		SineLoopRun plainRun = *run;
		plainRun.loopStartFrame = 0;
		plainRun.loopEndFrame = frameCount;
		MixSineRun(test, memorySine, &plainRun, plainOutput, outputFrameCount);
		bool isDifferent = (memcmp(memoryOutput, plainOutput, outputByteCount) != 0);
		CheckTrue(test, isDifferent, "the region changes the output");
	}

	// An endless stream has no end to loop before, it drops every region
	AudioSineWaveData endlessWave = sineWave;
	endlessWave.duration = 0.0;
	AudioSource *endlessSine = AudioSystemCreateSineStreamSource(test->audioSys, StereoChannelCount, OutputSampleRate, &endlessWave);
	CheckTrue(test, endlessSine != fpl_null, "create the endless sine wave");
	if(endlessSine != fpl_null) {
		bool isEndlessAdded = AudioSystemAddSource(test->audioSys, endlessSine);
		CheckTrue(test, isEndlessAdded, "add the endless sine wave");
		endlessSine->loopStartFrame = 100;
		endlessSine->loopEndFrame = 200;
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, endlessSine, true, 1.0f, AudioSystemDefaultPitch);
		AudioPlayItem item = fplZeroInit;
		bool hasItem = FindPlayItem(test, playId, &item);
		CheckTrue(test, hasItem && item.loopEndFrame == 0, "an endless stream gets no region from its source");
		bool isSent = AudioSystemSetPlayItemLoop(test->audioSys, playId, 100, 200);
		CheckTrue(test, isSent, "send a region to the endless stream");
		AudioSystemWriteFrames(test->audioSys, memoryOutput, &test->targetFormat, 512, true);
		hasItem = FindPlayItem(test, playId, &item);
		CheckTrue(test, hasItem && item.loopEndFrame == 0 && item.framesPlayed[0] == 512, "the endless stream drops the region and plays on");
		AudioSystemStopOne(test->audioSys, playId);
		AudioSystemWriteFrames(test->audioSys, memoryOutput, &test->targetFormat, 1, true);
	}

	AudioSystemStats stats = fplZeroInit;
	AudioSystemGetStats(test->audioSys, &stats);
	CheckTrue(test, stats.failedStreamCount == 0, "no stream failed");

	fplMemoryFree(plainOutput);
	fplMemoryFree(streamOutput);
	fplMemoryFree(memoryOutput);
}

static void RunSineStreamCase(LoopTest *test) {
	LoopTest sineTest = fplZeroInit;
	sineTest.targetFormat = test->targetFormat;
	sineTest.targetFormat.type = fplAudioFormatType_S16;
	sineTest.audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = (sineTest.audioSys != fpl_null) && AudioSystemInit(sineTest.audioSys, &sineTest.targetFormat);
	CheckTrue(&sineTest, isInitialized, "initialize the audio system with S16 output");
	if(isInitialized) {
		RunSineStreamRuns(&sineTest);
		AudioSystemShutdown(sineTest.audioSys);
	}
	if(sineTest.audioSys != fpl_null) {
		fplMemoryFree(sineTest.audioSys);
	}
	test->failedCheckCount += sineTest.failedCheckCount;
	printf("sine-stream: %s\n", sineTest.failedCheckCount == 0 ? "ok" : "FAILED");
}

int main(int argc, char **argv) {
	// No arguments, the signature is the one FPL expects on Windows
	(void)argc;
	(void)argv;
	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}

	LoopTest test = fplZeroInit;
	test.targetFormat.type = fplAudioFormatType_F32;
	test.targetFormat.sampleRate = OutputSampleRate;
	test.targetFormat.channels = StereoChannelCount;
	test.audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = (test.audioSys != fpl_null) && AudioSystemInit(test.audioSys, &test.targetFormat);
	CheckTrue(&test, isInitialized, "initialize the audio system");

	AudioSource *indexSource = fpl_null;
	if(isInitialized) {
		indexSource = CreateIndexSource(&test);
	}
	if(indexSource != fpl_null) {
		const float doublePitch = 2.0f;
		const float halfPitch = 0.5f;
		const AudioFrameIndex evenFactor = 2;
		const AudioFrameIndex noFactor = 1;
		RunIndexCases(&test, indexSource);
		RunEvenPitchCase(&test, indexSource, "pitch-2", doublePitch, evenFactor, noFactor);
		RunEvenPitchCase(&test, indexSource, "pitch-0.5", halfPitch, noFactor, evenFactor);
	}
	RunSineStreamCase(&test);
	if(isInitialized) {
		AudioSystemShutdown(test.audioSys);
	}
	if(test.audioSys != fpl_null) {
		fplMemoryFree(test.audioSys);
	}

	printf("total: %u failed checks\n", test.failedCheckCount);
	fplPlatformRelease();
	return(test.failedCheckCount > 0 ? 1 : 0);
}
