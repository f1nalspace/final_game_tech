/*
Plays many short sine wave streams with final_audiosystem.h and checks that the stream instances of ended items are freed on the caller side (AudioSystemPlaySource, AudioSystemStopAll, AudioSystemRemoveSource), never pile up and never leak.

Cases:
- sequential: 1000 short streams one after another on one thread, a few at once, about four times the pool of play items. After every play the allocated instances are exactly the playing items.
- remove-source: streams of one source end and no call frees them, then AudioSystemRemoveSource frees them before the source. A source that still plays is removed as well.
- threads: a mixing thread mixes all the time while two threads play 500 short streams each and stop some of them, so instances are freed while the mixer ends other items. Meant for ASan and TSan.

Build against the additions to test, with assertions (the mixer checks its own invariants):
gcc -std=c99 -O2 -DFPL_FORCE_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies stream_reclaim_test.c -o stream_reclaim_test -lm -ldl -lpthread
Usage: stream_reclaim_test
Output: one line per case "<name>: plays=<n> max_playing=<n> max_instances=<n> instances_left=<n> <ok|FAILED>", then "total: <n> failed checks".
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>

static const AudioHertz OutputSampleRate = 48000;
static const AudioChannelIndex StereoChannelCount = 2;
static const AudioFrameIndex CallFrameCount = 512;
static const double SineToneVolume = 0.25;
static const AudioHertz SineFrequency = 440;

// Stops a loop that waits for items that never end
static const uint32_t MaxCallCount = 100000;

typedef struct ReclaimTest {
	AudioSystem *audioSys;
	fplAudioFormat targetFormat;
	int16_t *callSamples;
	uint32_t playCount;
	uint32_t maxPlayingCount;
	uint32_t maxInstanceCount;
	volatile uint32_t failedCheckCount;
} ReclaimTest;

static void CheckTrue(ReclaimTest *test, const bool condition, const char *description) {
	if(!condition) {
		printf("  check failed: %s\n", description);
		fplAtomicIncrementU32(&test->failedCheckCount);
	}
}

static AudioSource *CreateSineStream(ReclaimTest *test, const AudioDuration seconds) {
	AudioSineWaveData sineWave = fplZeroInit;
	sineWave.duration = seconds;
	sineWave.toneVolume = SineToneVolume;
	sineWave.frequency = SineFrequency;
	AudioSource *source = AudioSystemCreateSineStreamSource(test->audioSys, StereoChannelCount, OutputSampleRate, &sineWave);
	CheckTrue(test, source != fpl_null, "create a sine wave stream");
	if(source == fpl_null) {
		return(fpl_null);
	}
	bool isAdded = AudioSystemAddSource(test->audioSys, source);
	CheckTrue(test, isAdded, "add a sine wave stream");
	return(source);
}

static AudioSystemStats GetStats(ReclaimTest *test) {
	AudioSystemStats result = fplZeroInit;
	AudioSystemGetStats(test->audioSys, &result);
	return(result);
}

static void MixCall(ReclaimTest *test) {
	AudioFrameIndex writtenFrameCount = AudioSystemWriteFrames(test->audioSys, test->callSamples, &test->targetFormat, CallFrameCount, true);
	CheckTrue(test, writtenFrameCount == CallFrameCount, "write all frames of a call");
}

static void MixUntilSilent(ReclaimTest *test) {
	uint32_t callIndex = 0;
	size_t itemCount = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
	while(itemCount > 0 && callIndex < MaxCallCount) {
		MixCall(test);
		itemCount = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
		++callIndex;
	}
	CheckTrue(test, itemCount == 0, "the streams end");
}

// The instances that are left at the end, after a check that AudioSystemStopAll frees all of them
static uint32_t FinishCase(ReclaimTest *test) {
	AudioSystemStopAll(test->audioSys);
	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.streamInstanceCount == 0 && stats.streamInstanceByteCount == 0, "AudioSystemStopAll frees every ended stream instance");
	CheckTrue(test, stats.failedStreamCount == 0, "no stream failed");
	return((uint32_t)stats.streamInstanceCount);
}

static void RunSequentialCase(ReclaimTest *test) {
	const uint32_t streamCount = 1000;
	// Two to three calls long, so a few streams play at once
	const AudioDuration streamSeconds = 0.025;
	AudioSource *source = CreateSineStream(test, streamSeconds);
	if(source == fpl_null) {
		return;
	}
	uint32_t instanceByteCount = 0;
	for(uint32_t streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play a stream");
		++test->playCount;

		// Every ended stream was freed by the play, so the instances are exactly the playing items
		AudioSystemStats stats = GetStats(test);
		CheckTrue(test, stats.streamInstanceCount == stats.activePlayItemCount, "the instances are the playing streams");
		test->maxPlayingCount = fplMax(test->maxPlayingCount, stats.activePlayItemCount);
		test->maxInstanceCount = fplMax(test->maxInstanceCount, (uint32_t)stats.streamInstanceCount);
		if(stats.streamInstanceCount == 1) {
			instanceByteCount = (uint32_t)stats.streamInstanceByteCount;
		}
		CheckTrue(test, stats.streamInstanceByteCount == stats.streamInstanceCount * instanceByteCount, "every instance has the same size");
		MixCall(test);
	}
	MixUntilSilent(test);
}

static void RunRemoveSourceCase(ReclaimTest *test) {
	const uint32_t endedStreamCount = 5;
	const uint32_t playingStreamCount = 2;
	const uint32_t playingCallCount = 3;
	const AudioDuration shortSeconds = 0.01;
	const AudioDuration longSeconds = 10.0;

	// Ended and not freed by any call, AudioSystemRemoveSource has to free them before the source
	AudioSource *shortSource = CreateSineStream(test, shortSeconds);
	AudioSource *longSource = CreateSineStream(test, longSeconds);
	if(shortSource == fpl_null || longSource == fpl_null) {
		return;
	}
	for(uint32_t streamIndex = 0; streamIndex < endedStreamCount; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, shortSource, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play a short stream");
		++test->playCount;
	}
	MixUntilSilent(test);
	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.streamInstanceCount == endedStreamCount, "the ended streams keep their instances");
	test->maxInstanceCount = fplMax(test->maxInstanceCount, (uint32_t)stats.streamInstanceCount);
	test->maxPlayingCount = fplMax(test->maxPlayingCount, endedStreamCount);
	bool isRemoved = AudioSystemRemoveSource(test->audioSys, shortSource);
	CheckTrue(test, isRemoved, "remove the short source");
	stats = GetStats(test);
	CheckTrue(test, stats.streamInstanceCount == 0, "AudioSystemRemoveSource frees the ended streams of its source");

	// Still playing, the removal stops them first
	for(uint32_t streamIndex = 0; streamIndex < playingStreamCount; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, longSource, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play a long stream");
		++test->playCount;
	}
	for(uint32_t callIndex = 0; callIndex < playingCallCount; ++callIndex) {
		MixCall(test);
	}
	isRemoved = AudioSystemRemoveSource(test->audioSys, longSource);
	CheckTrue(test, isRemoved, "remove the long source");
	stats = GetStats(test);
	CheckTrue(test, stats.activePlayItemCount == 0, "AudioSystemRemoveSource stops the playing streams");
	CheckTrue(test, stats.streamInstanceCount == 0, "AudioSystemRemoveSource frees the stopped streams");
	CheckTrue(test, stats.sourceCount == 0, "no source is left");
}

//
// Threads
//

#define PLAYER_THREAD_COUNT 2

static const uint32_t StreamsPerPlayerThread = 500;
static const AudioFrameIndex ThreadCallFrameCount = 64;
// Every few plays a thread stops the stream it just started
static const uint32_t StopEveryPlayCount = 7;
// A thread yields this often between two plays, which lets a few streams play at once
static const uint32_t YieldsBetweenPlays = 50;

typedef struct ReclaimThreadData {
	ReclaimTest *test;
	const AudioSource *source;
	int16_t *samples;
	volatile uint32_t *isMixing;
	uint32_t playCount;
	uint32_t maxPlayingCount;
	uint32_t maxInstanceCount;
} ReclaimThreadData;

static void MixingThread(const fplThreadHandle *thread, void *data) {
	(void)thread;
	ReclaimThreadData *threadData = (ReclaimThreadData *)data;
	ReclaimTest *test = threadData->test;
	while(fplAtomicLoadU32(threadData->isMixing) != 0) {
		AudioSystemWriteFrames(test->audioSys, threadData->samples, &test->targetFormat, ThreadCallFrameCount, true);
		fplThreadYield();
	}
}

static void PlayerThread(const fplThreadHandle *thread, void *data) {
	(void)thread;
	ReclaimThreadData *threadData = (ReclaimThreadData *)data;
	ReclaimTest *test = threadData->test;
	for(uint32_t streamIndex = 0; streamIndex < StreamsPerPlayerThread; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, threadData->source, false, 1.0f, AudioSystemDefaultPitch);
		if(playId.value != 0) {
			++threadData->playCount;
		}
		AudioSystemStats stats = fplZeroInit;
		AudioSystemGetStats(test->audioSys, &stats);
		threadData->maxPlayingCount = fplMax(threadData->maxPlayingCount, stats.activePlayItemCount);
		threadData->maxInstanceCount = fplMax(threadData->maxInstanceCount, (uint32_t)stats.streamInstanceCount);
		if((streamIndex % StopEveryPlayCount) == 0) {
			AudioSystemStopOne(test->audioSys, playId);
		}
		for(uint32_t yieldIndex = 0; yieldIndex < YieldsBetweenPlays; ++yieldIndex) {
			fplThreadYield();
		}
	}
}

static void RunThreadsCase(ReclaimTest *test) {
	const AudioDuration streamSeconds = 0.01;
	const uint32_t waitSleepMilliseconds = 1;
	const uint32_t maxWaitCount = 10000;
	AudioSource *source = CreateSineStream(test, streamSeconds);
	if(source == fpl_null) {
		return;
	}

	const size_t callSampleCount = (size_t)ThreadCallFrameCount * StereoChannelCount;
	volatile uint32_t isMixing = 1;
	ReclaimThreadData mixerData = fplZeroInit;
	mixerData.test = test;
	mixerData.samples = (int16_t *)fplMemoryAllocate(callSampleCount * sizeof(int16_t));
	mixerData.isMixing = &isMixing;
	fplThreadHandle *mixerThread = fplThreadCreate(MixingThread, &mixerData);

	ReclaimThreadData playerData[PLAYER_THREAD_COUNT];
	fplThreadHandle *playerThreads[PLAYER_THREAD_COUNT];
	for(uint32_t threadIndex = 0; threadIndex < PLAYER_THREAD_COUNT; ++threadIndex) {
		ReclaimThreadData *data = &playerData[threadIndex];
		fplClearStruct(data);
		data->test = test;
		data->source = source;
		playerThreads[threadIndex] = fplThreadCreate(PlayerThread, data);
	}
	for(uint32_t threadIndex = 0; threadIndex < PLAYER_THREAD_COUNT; ++threadIndex) {
		fplThreadWaitForOne(playerThreads[threadIndex], FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(playerThreads[threadIndex]);
		test->playCount += playerData[threadIndex].playCount;
		test->maxPlayingCount = fplMax(test->maxPlayingCount, playerData[threadIndex].maxPlayingCount);
		test->maxInstanceCount = fplMax(test->maxInstanceCount, playerData[threadIndex].maxInstanceCount);
	}

	// The mixer ends the last streams, then it stops
	uint32_t waitIndex = 0;
	size_t itemCount = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
	while(itemCount > 0 && waitIndex < maxWaitCount) {
		fplThreadSleep(waitSleepMilliseconds);
		itemCount = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
		++waitIndex;
	}
	CheckTrue(test, itemCount == 0, "the streams of both threads end");
	fplAtomicStoreU32(&isMixing, 0);
	fplThreadWaitForOne(mixerThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(mixerThread);
	fplMemoryFree(mixerData.samples);

	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.concurrentWriteCount == 0, "only the mixing thread mixed");
	// Two threads at once: each play frees what ended before it, so at most the playing streams plus the ones that ended since the last play of either thread
	CheckTrue(test, test->maxInstanceCount <= test->maxPlayingCount + PLAYER_THREAD_COUNT, "the instances do not pile up");
}

typedef void (ReclaimCaseFunction)(ReclaimTest *test);

typedef struct ReclaimCase {
	const char *name;
	ReclaimCaseFunction *run;
} ReclaimCase;

static const ReclaimCase ReclaimCases[] = {
	{ "sequential", RunSequentialCase },
	{ "remove-source", RunRemoveSourceCase },
	{ "threads", RunThreadsCase },
};

static uint32_t RunReclaimCase(const ReclaimCase *reclaimCase) {
	ReclaimTest test = fplZeroInit;
	test.targetFormat.type = fplAudioFormatType_S16;
	test.targetFormat.sampleRate = OutputSampleRate;
	test.targetFormat.channels = StereoChannelCount;

	const size_t callSampleCount = (size_t)CallFrameCount * StereoChannelCount;
	test.callSamples = (int16_t *)fplMemoryAllocate(callSampleCount * sizeof(int16_t));
	test.audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = false;
	if(test.callSamples != fpl_null && test.audioSys != fpl_null) {
		isInitialized = AudioSystemInit(test.audioSys, &test.targetFormat);
	}
	CheckTrue(&test, isInitialized, "initialize the audio system");

	uint32_t instancesLeftCount = 0;
	if(isInitialized) {
		reclaimCase->run(&test);
		instancesLeftCount = FinishCase(&test);
		AudioSystemShutdown(test.audioSys);
	}

	const uint32_t failedCheckCount = fplAtomicLoadU32(&test.failedCheckCount);
	printf("%s: plays=%u max_playing=%u max_instances=%u instances_left=%u %s\n", reclaimCase->name, test.playCount, test.maxPlayingCount, test.maxInstanceCount, instancesLeftCount, failedCheckCount == 0 ? "ok" : "FAILED");

	if(test.audioSys != fpl_null) {
		fplMemoryFree(test.audioSys);
	}
	if(test.callSamples != fpl_null) {
		fplMemoryFree(test.callSamples);
	}
	return(failedCheckCount);
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}

	uint32_t totalFailedCheckCount = 0;
	for(size_t caseIndex = 0; caseIndex < fplArrayCount(ReclaimCases); ++caseIndex) {
		totalFailedCheckCount += RunReclaimCase(&ReclaimCases[caseIndex]);
	}
	printf("total: %u failed checks\n", totalFailedCheckCount);

	fplPlatformRelease();
	return(totalFailedCheckCount > 0 ? 1 : 0);
}
