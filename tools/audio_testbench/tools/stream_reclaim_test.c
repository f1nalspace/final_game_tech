/*
Plays many short sine wave streams with final_audiosystem.h and checks that the stream instances of ended items are freed on the caller side (AudioSystemPlaySource, AudioSystemStopAll, AudioSystemRemoveSource, AudioSystemShutdown), never pile up and never leak.

Cases:
- sequential: 1000 short streams one after another on one thread, a few at once, about four times the pool of play items. After every play the allocated instances are exactly the playing items.
- remove-source: streams of one source end and no call frees them, then AudioSystemRemoveSource frees them before the source. A source that still plays is removed as well.
- threads: a mixing thread mixes all the time while two threads play 500 short streams each and stop some of them, so instances are freed while the mixer ends other items. Meant for ASan and TSan.
- vorbis (only with a file): 200 plays of a Vorbis stream that are stopped after two calls, then stopped streams that no call freed and a source whose data the caller keeps are removed. Each instance holds its own decoder.
- shutdown: a thread mixes once and ends, as a streaming thread does, then AudioSystemShutdown runs while streams play and one more play is still queued. It must not wait for the mixer, frees every instance, and a mix call afterwards writes silence.
- seek-drain: a sine wave stream (and the Vorbis stream with a file) is sought while no mixer runs, a waiting AudioSystemRemoveSource applies the queue. That only sets the position, the ring and the decoder stay untouched, the next mix call seeks the stream.

Build against the additions to test, with assertions (the mixer checks its own invariants):
gcc -std=c99 -O2 -DFPL_FORCE_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies stream_reclaim_test.c -o stream_reclaim_test -lm -ldl -lpthread
Usage: stream_reclaim_test [<ogg file>]
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

// The Ogg Vorbis file of the vorbis case, from the command line
static const char *VorbisFilePath = fpl_null;

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

//
// Vorbis
//

#define VORBIS_STOPPED_STREAM_COUNT 3

static void RunVorbisCase(ReclaimTest *test) {
	const uint32_t streamCount = 200;
	const uint32_t callsPerStream = 2;
	const uint32_t stoppedStreamCount = VORBIS_STOPPED_STREAM_COUNT;
	AudioPlayItemID stoppedPlayIds[VORBIS_STOPPED_STREAM_COUNT];
	AudioSource *fileSource = AudioSystemLoadFileStreamSource(test->audioSys, VorbisFilePath);
	CheckTrue(test, fileSource != fpl_null, "load the Vorbis file as a stream");
	if(fileSource == fpl_null) {
		return;
	}
	bool isAdded = AudioSystemAddSource(test->audioSys, fileSource);
	CheckTrue(test, isAdded, "add the Vorbis stream");

	// Stopped after two calls: the next play frees the instance, so the instances are exactly the playing items
	for(uint32_t streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, fileSource, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play the Vorbis stream");
		++test->playCount;
		AudioSystemStats stats = GetStats(test);
		CheckTrue(test, stats.streamInstanceCount == stats.activePlayItemCount, "the instances are the playing streams");
		test->maxPlayingCount = fplMax(test->maxPlayingCount, stats.activePlayItemCount);
		test->maxInstanceCount = fplMax(test->maxInstanceCount, (uint32_t)stats.streamInstanceCount);
		for(uint32_t callIndex = 0; callIndex < callsPerStream; ++callIndex) {
			MixCall(test);
		}
		AudioSystemStopOne(test->audioSys, playId);
	}
	// The mixer applies the last stop, the next play frees its instance
	MixCall(test);

	// Stopped and not freed by any call yet, AudioSystemRemoveSource frees them before the source and its data. A play would free them, so all of them start first
	for(uint32_t streamIndex = 0; streamIndex < stoppedStreamCount; ++streamIndex) {
		stoppedPlayIds[streamIndex] = AudioSystemPlaySource(test->audioSys, fileSource, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, stoppedPlayIds[streamIndex].value != 0, "play the Vorbis stream again");
		++test->playCount;
	}
	MixCall(test);
	for(uint32_t streamIndex = 0; streamIndex < stoppedStreamCount; ++streamIndex) {
		AudioSystemStopOne(test->audioSys, stoppedPlayIds[streamIndex]);
	}
	MixCall(test);
	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.activePlayItemCount == 0 && stats.streamInstanceCount == stoppedStreamCount, "the stopped streams keep their instances");
	CheckTrue(test, stats.compressedByteCount == fileSource->stream.dataSize, "the compressed file counts");
	bool isRemoved = AudioSystemRemoveSource(test->audioSys, fileSource);
	CheckTrue(test, isRemoved, "remove the Vorbis file source");
	stats = GetStats(test);
	CheckTrue(test, stats.streamInstanceCount == 0 && stats.compressedByteCount == 0, "AudioSystemRemoveSource frees the stopped streams and the compressed file");

	// The data stays with the caller, the source only points at it. It is removed while it plays and the data is freed afterwards
	fplFileHandle file;
	bool isOpen = fplFileOpenBinary(VorbisFilePath, &file);
	CheckTrue(test, isOpen, "open the Vorbis file");
	if(!isOpen) {
		return;
	}
	uint32_t fileSize = fplFileGetSizeFromHandle32(&file);
	uint8_t *fileData = (uint8_t *)fplMemoryAllocate(fileSize);
	uint32_t readSize = fplFileReadBlock32(&file, fileSize, fileData, fileSize);
	fplFileClose(&file);
	CheckTrue(test, readSize == fileSize, "read the Vorbis file");
	AudioSource *dataSource = AudioSystemLoadDataStreamSource(test->audioSys, fileSize, fileData, false);
	CheckTrue(test, dataSource != fpl_null && dataSource->stream.data == fileData, "a data stream without copy keeps the pointer");
	if(dataSource != fpl_null) {
		isAdded = AudioSystemAddSource(test->audioSys, dataSource);
		CheckTrue(test, isAdded, "add the Vorbis data stream");
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, dataSource, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play the Vorbis data stream");
		++test->playCount;
		MixCall(test);
		isRemoved = AudioSystemRemoveSource(test->audioSys, dataSource);
		CheckTrue(test, isRemoved, "remove the playing Vorbis data stream");
	}
	fplMemoryFree(fileData);
	stats = GetStats(test);
	CheckTrue(test, stats.activePlayItemCount == 0 && stats.streamInstanceCount == 0 && stats.sourceCount == 0, "nothing is left of the Vorbis streams");
}

// One mix call, then the thread ends like a streaming thread at the shutdown of an application
static void MixOnceThread(const fplThreadHandle *thread, void *data) {
	(void)thread;
	ReclaimTest *test = (ReclaimTest *)data;
	MixCall(test);
}

static void RunShutdownCase(ReclaimTest *test) {
	const uint32_t playingStreamCount = 3;
	const AudioDuration streamSeconds = 10.0;
	// The mixer counts as stopped only after 50 ms at least, a shutdown that waits for it takes longer than this
	const fplMilliseconds maxShutdownMilliseconds = 25;
	const int16_t leftoverSampleValue = 1234;
	AudioSource *source = CreateSineStream(test, streamSeconds);
	if(source == fpl_null) {
		return;
	}
	for(uint32_t streamIndex = 0; streamIndex < playingStreamCount; ++streamIndex) {
		AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, false, 1.0f, AudioSystemDefaultPitch);
		CheckTrue(test, playId.value != 0, "play a stream");
		++test->playCount;
	}
	// The thread that mixed last is gone, so the shutdown cannot ask it to apply anything
	fplThreadHandle *mixerThread = fplThreadCreate(MixOnceThread, test);
	CheckTrue(test, mixerThread != fpl_null, "start the mixing thread");
	if(mixerThread == fpl_null) {
		return;
	}
	fplThreadWaitForOne(mixerThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(mixerThread);
	// Queued only, the mixer never takes it
	AudioPlayItemID queuedPlayId = AudioSystemPlaySource(test->audioSys, source, false, 1.0f, AudioSystemDefaultPitch);
	CheckTrue(test, queuedPlayId.value != 0, "play a stream that stays queued");
	++test->playCount;
	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.streamInstanceCount == playingStreamCount + 1, "every play has its instance");
	test->maxPlayingCount = stats.activePlayItemCount;
	test->maxInstanceCount = (uint32_t)stats.streamInstanceCount;

	fplMilliseconds shutdownStart = fplMillisecondsQuery();
	AudioSystemShutdown(test->audioSys);
	fplMilliseconds shutdownMilliseconds = fplMillisecondsQuery() - shutdownStart;
	CheckTrue(test, shutdownMilliseconds < maxShutdownMilliseconds, "AudioSystemShutdown does not wait for the mixer");
	// AudioSystemGetStats answers no more after the shutdown
	uint32_t instanceCount = fplAtomicLoadU32(&test->audioSys->streamInstanceCount);
	uint32_t instanceByteCount = fplAtomicLoadU32(&test->audioSys->streamInstanceByteCount);
	CheckTrue(test, instanceCount == 0 && instanceByteCount == 0, "AudioSystemShutdown frees every stream instance, also of the queued play");

	const size_t callSampleCount = (size_t)CallFrameCount * StereoChannelCount;
	for(size_t sampleIndex = 0; sampleIndex < callSampleCount; ++sampleIndex) {
		test->callSamples[sampleIndex] = leftoverSampleValue;
	}
	MixCall(test);
	bool isSilent = true;
	for(size_t sampleIndex = 0; sampleIndex < callSampleCount; ++sampleIndex) {
		if(test->callSamples[sampleIndex] != 0) {
			isSilent = false;
			break;
		}
	}
	CheckTrue(test, isSilent, "a mix call after the shutdown writes silence");
}

// The play item of an id as AudioSystemGetPlayItems returns it, false when it is not there
static bool FindPlayItem(ReclaimTest *test, const AudioPlayItemID playId, AudioPlayItem *outItem) {
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

// The stream instance of a play item, read from its slot
static AudioStreamInstance *GetStreamInstance(ReclaimTest *test, const AudioPlayItemID playId) {
	uint32_t slotIndex = 0;
	uint32_t generation = 0;
	if(!DecodePlayItemID(playId, &slotIndex, &generation)) {
		return(fpl_null);
	}
	AudioPlayItemSlot *slot = &test->audioSys->playItems.slots[slotIndex];
	AudioStreamInstance *result = (AudioStreamInstance *)fplAtomicLoadPtr(&slot->streamInstance);
	return(result);
}

// Seeks a stream while no mixer runs: the thread that applies the queue only sets the position, the next mix seeks the stream
static void CheckSeekWithoutMixer(ReclaimTest *test, AudioSource *source) {
	const AudioDuration targetSeconds = 3.0;
	const AudioDuration otherSourceSeconds = 1.0;
	const AudioFrameIndex targetFrameIndex = (AudioFrameIndex)(targetSeconds * (AudioDuration)source->format.sampleRate);
	AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, false, 1.0f, AudioSystemDefaultPitch);
	CheckTrue(test, playId.value != 0, "play the stream to seek");
	if(playId.value == 0) {
		return;
	}
	++test->playCount;
	test->maxPlayingCount = fplMax(test->maxPlayingCount, 1);
	test->maxInstanceCount = fplMax(test->maxInstanceCount, 1);

	// The thread that mixed last is gone, so the next waiting function applies the queue itself
	fplThreadHandle *mixerThread = fplThreadCreate(MixOnceThread, test);
	CheckTrue(test, mixerThread != fpl_null, "start the mixing thread");
	if(mixerThread == fpl_null) {
		return;
	}
	fplThreadWaitForOne(mixerThread, FPL_TIMEOUT_INFINITE);
	fplThreadTerminate(mixerThread);

	AudioStreamInstance *instance = GetStreamInstance(test, playId);
	CheckTrue(test, instance != fpl_null, "the stream has its instance");
	if(instance == fpl_null) {
		return;
	}
	const AudioFrameIndex ringStartBefore = instance->ringStartFrameIndex;
	const AudioFrameIndex ringFrameCountBefore = instance->ringFrameCount;
	const int decoderOffsetBefore = (instance->vorbis != fpl_null) ? stb_vorbis_get_sample_offset(instance->vorbis) : 0;

	bool isSent = AudioSystemSetPlayItemPosition(test->audioSys, playId, targetFrameIndex);
	CheckTrue(test, isSent, "send the seek");
	// Removing a source waits for its command, with no mixer this thread applies the queue and the seek with it
	AudioSource *otherSource = CreateSineStream(test, otherSourceSeconds);
	if(otherSource == fpl_null) {
		return;
	}
	bool isRemoved = AudioSystemRemoveSource(test->audioSys, otherSource);
	CheckTrue(test, isRemoved, "remove a source while no mixer runs");

	AudioPlayItem item = fplZeroInit;
	bool hasItem = FindPlayItem(test, playId, &item);
	CheckTrue(test, hasItem && item.framesPlayed[0] == targetFrameIndex, "the seek is applied without a mixer");
	const int decoderOffsetAfter = (instance->vorbis != fpl_null) ? stb_vorbis_get_sample_offset(instance->vorbis) : 0;
	bool isRingUntouched = (instance->ringStartFrameIndex == ringStartBefore) && (instance->ringFrameCount == ringFrameCountBefore) && (decoderOffsetAfter == decoderOffsetBefore);
	CheckTrue(test, isRingUntouched, "applying the seek decodes nothing");

	// The mixer seeks the stream before its first chunk, the ring starts at the target then
	MixCall(test);
	hasItem = FindPlayItem(test, playId, &item);
	CheckTrue(test, hasItem && item.framesPlayed[0] > targetFrameIndex, "the stream plays on from the target");
	CheckTrue(test, instance->ringStartFrameIndex == targetFrameIndex, "the mixer seeks the stream to the target");
	AudioSystemStopOne(test->audioSys, playId);
	MixCall(test);
}

static void RunSeekDrainCase(ReclaimTest *test) {
	const AudioDuration sineSeconds = 10.0;
	AudioSource *sineSource = CreateSineStream(test, sineSeconds);
	if(sineSource != fpl_null) {
		CheckSeekWithoutMixer(test, sineSource);
	}
	if(VorbisFilePath != fpl_null) {
		AudioSource *fileSource = AudioSystemLoadFileStreamSource(test->audioSys, VorbisFilePath);
		CheckTrue(test, fileSource != fpl_null, "load the Vorbis file as a stream");
		if(fileSource != fpl_null) {
			bool isAdded = AudioSystemAddSource(test->audioSys, fileSource);
			CheckTrue(test, isAdded, "add the Vorbis stream");
			CheckSeekWithoutMixer(test, fileSource);
		}
	}
	AudioSystemStats stats = GetStats(test);
	CheckTrue(test, stats.failedStreamCount == 0, "no stream failed");
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
	{ "vorbis", RunVorbisCase },
	{ "shutdown", RunShutdownCase },
	{ "seek-drain", RunSeekDrainCase },
};

static uint32_t RunReclaimCase(const ReclaimCase *reclaimCase) {
	// The vorbis case needs a file
	const bool isVorbisCase = (reclaimCase->run == RunVorbisCase);
	if(isVorbisCase && VorbisFilePath == fpl_null) {
		printf("%s: skipped, no ogg file given\n", reclaimCase->name);
		return(0);
	}
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
	if(argc > 1) {
		VorbisFilePath = argv[1];
	}
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
