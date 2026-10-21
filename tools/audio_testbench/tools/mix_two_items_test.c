/*
Mixes several play items at the same time with final_audiosystem.h and prints one FNV-1a 64 hash per case, because the render matrix of the test bench plays only one item per case.
The hashes must stay the same before and after every change to the mixer on the same build, F32 output so that no bit of a difference is lost.
The sources are integer noise, the same on every build, so only the mixer itself can make two builds differ (SinC differs between x64 and the XP build).

Cases:
- pass-sinc: 48 kHz stereo passes through, 44.1 kHz mono goes through SinC and ends inside a call while the other one goes on. Previews, a volume change and a pause in between.
- down2-up2: frame skipping and frame duplication with 6 output channels, a repeating item that wraps inside calls, a pitch change that switches its path while it plays.
- four-items: four items at once, the second one ends first. The other three must be mixed in the same order as before, a different order changes the float sums. Then a stale id, a new play and a stop from the middle.
- stale-ids: 600 short tones one after another, a few at once, more than twice the default pool of play items. A setter and a stop on the id of the first tone must change nothing.
- stream-cancel: a sine wave in memory and the same sine wave as a stream with the negated volume, so every mixed sample must be exactly 0 while both play: through pitch changes (SinC, frame skipping), a pause, previews (which leave the stream out) and repeats.

--concurrent instead runs two threads that call AudioSystemWriteFrames with advance at the same time, which is a programming error the mixer must survive:
the second one gets silence and is counted in concurrentWriteCount, so the item moves on by exactly the frames of the calls that got the mixer.
It runs once with a source in memory and once with a sine wave stream, which only the thread that holds the mixer may move on.
A debug build asserts on it, so this mode is built without assertions.

Build against the additions to test, with assertions (the mixer checks its own invariants), and once more with -DFPL_NO_ASSERTIONS for --concurrent:
gcc -std=c99 -O2 -DFPL_FORCE_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies mix_two_items_test.c -o mix_two_items_test -lm -ldl -lpthread
Usage: mix_two_items_test [-v | --concurrent]   (-v prints every mix call)
Output: one line per case "<name>: calls=<n> frames=<n> items_left=<n> hash=<fnv1a64>", then "total: <n> cases, <n> failed checks, hash=<fnv1a64>".
With --concurrent: "concurrent: calls=<n> rejected=<n> frames_played=<n> expected=<n> <ok|FAILED>", then the same line for the stream as "concurrent-stream: ...".
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>
#include <string.h>

static const uint64_t Fnv1a64OffsetBasis = 0xcbf29ce484222325ULL;
static const uint64_t Fnv1a64Prime = 0x100000001b3ULL;

static uint64_t HashFnv1a64(const uint64_t hash, const void *data, const size_t size) {
	const uint8_t *bytes = (const uint8_t *)data;
	uint64_t result = hash;
	for(size_t byteIndex = 0; byteIndex < size; ++byteIndex) {
		result ^= bytes[byteIndex];
		result *= Fnv1a64Prime;
	}
	return(result);
}

static const AudioHertz OutputSampleRate = 48000;
// No even ratio to the output, goes through SinC
static const AudioHertz SincSampleRate = 44100;
// Frame duplication with factor 2
static const AudioHertz HalfOutputSampleRate = 24000;

static const AudioChannelIndex MonoChannelCount = 1;
static const AudioChannelIndex StereoChannelCount = 2;

// Output channel counts of the cases, an enum so that the case table can use them
typedef enum MixOutputChannels {
	MixOutputChannels_Stereo = 2,
	MixOutputChannels_Surround = 6,
} MixOutputChannels;

// Frames per AudioSystemWriteFrames call, used in turn. 5000 needs two fills of the 4096-frame conversion buffer, 1, 333 and 1001 end inside a duplicated frame
static const AudioFrameIndex CallFrameCounts[] = { 512, 1001, 4096, 5000, 64, 1, 333 };
static const AudioFrameIndex MaxCallFrameCount = 5000;
static const AudioChannelIndex MaxOutputChannelCount = MixOutputChannels_Surround;

// Silent calls after the last item ended
static const uint32_t TailCallCount = 3;

// Stops a case whose items never end
static const uint32_t MaxCallCount = 1000;

// Noise peak, a quarter of full scale, so four items at once stay far below clipping
static const int32_t NoiseAmplitude = 8192;

// Numerical Recipes LCG, integer only so the sources are the same on every build
static const uint32_t NoiseMultiplier = 1664525u;
static const uint32_t NoiseIncrement = 1013904223u;
static const uint32_t NoiseValueShift = 16;
static const int32_t NoiseValueHalfRange = 32768;

typedef struct MixTest {
	AudioSystem *audioSys;
	fplAudioFormat targetFormat;
	float *callSamples;
	uint64_t hash;
	// Frames mixed with advance, previews are not counted
	size_t frameCount;
	uint32_t callCount;
	uint32_t failedCheckCount;
	bool isVerbose;
} MixTest;

static void CheckTrue(MixTest *test, const bool condition, const char *description) {
	if(!condition) {
		printf("  check failed: %s\n", description);
		++test->failedCheckCount;
	}
}

static size_t GetPlayItemCount(MixTest *test) {
	size_t result = AudioSystemGetPlayItems(test->audioSys, fpl_null, 0);
	return(result);
}

static AudioSource *CreateNoiseSource(MixTest *test, const AudioChannelIndex channelCount, const AudioHertz sampleRate, const AudioFrameIndex frameCount, const uint32_t seed) {
	AudioSource *source = AudioSystemAllocateSource(test->audioSys, channelCount, sampleRate, fplAudioFormatType_S16, frameCount);
	CheckTrue(test, source != fpl_null, "allocate a source");
	if(source == fpl_null) {
		return(fpl_null);
	}
	int16_t *samples = (int16_t *)source->buffer.samples;
	const size_t sampleCount = (size_t)frameCount * channelCount;
	uint32_t state = seed;
	for(size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		state = state * NoiseMultiplier + NoiseIncrement;
		int32_t rawValue = (int32_t)(state >> NoiseValueShift);
		int32_t centeredValue = rawValue - NoiseValueHalfRange;
		int32_t scaledValue = centeredValue * NoiseAmplitude / NoiseValueHalfRange;
		samples[sampleIndex] = (int16_t)scaledValue;
	}
	bool isAdded = AudioSystemAddSource(test->audioSys, source);
	CheckTrue(test, isAdded, "add a source");
	return(source);
}

static AudioPlayItemID PlayNoiseSource(MixTest *test, const AudioSource *source, const bool repeat, const float volume, const float pitch) {
	AudioPlayItemID result = AudioSystemPlaySource(test->audioSys, source, repeat, volume, pitch);
	CheckTrue(test, result.value != 0, "play a source");
	return(result);
}

static void MixCall(MixTest *test, const AudioFrameIndex frameCount, const bool advance) {
	AudioFrameIndex writtenFrameCount = AudioSystemWriteFrames(test->audioSys, test->callSamples, &test->targetFormat, frameCount, advance);
	CheckTrue(test, writtenFrameCount == frameCount, "write all frames of a call");
	size_t frameSize = fplGetAudioFrameSizeInBytes(test->targetFormat.type, test->targetFormat.channels);
	size_t byteCount = frameSize * frameCount;
	test->hash = HashFnv1a64(test->hash, test->callSamples, byteCount);
	if(advance) {
		test->frameCount += frameCount;
		++test->callCount;
	}
	if(test->isVerbose) {
		size_t itemCount = GetPlayItemCount(test);
		const char *callKind = advance ? "call" : "preview after call";
		printf("  %s %u: %u frames, %zu items after, %zu frames mixed\n", callKind, test->callCount, frameCount, itemCount, test->frameCount);
	}
}

static void MixCalls(MixTest *test, const uint32_t count) {
	for(uint32_t callIndex = 0; callIndex < count; ++callIndex) {
		uint32_t cycleIndex = (uint32_t)(test->callCount % fplArrayCount(CallFrameCounts));
		MixCall(test, CallFrameCounts[cycleIndex], true);
	}
}

// Renders ahead without moving any item, AudioSystemWriteFrames with advance = false
static void MixPreview(MixTest *test, const AudioFrameIndex frameCount) {
	MixCall(test, frameCount, false);
}

// Mixes until no more than the given number of items is left, a case whose items do not end counts as a failed check
static void MixUntilItemCount(MixTest *test, const size_t maxItemCount) {
	uint32_t callIndex = 0;
	size_t itemCount = GetPlayItemCount(test);
	while(itemCount > maxItemCount && callIndex < MaxCallCount) {
		MixCalls(test, 1);
		itemCount = GetPlayItemCount(test);
		++callIndex;
	}
	CheckTrue(test, itemCount <= maxItemCount, "items end");
}

static void RunPassthroughAndSincCase(MixTest *test) {
	const AudioFrameIndex musicFrameCount = 60000;
	const AudioFrameIndex effectFrameCount = 20000;
	const uint32_t musicSeed = 1;
	const uint32_t effectSeed = 2;
	const float musicVolume = 0.6f;
	const float effectVolume = 0.5f;
	const float changedEffectVolume = 0.8f;
	const AudioFrameIndex firstPreviewFrameCount = 5000;
	const AudioFrameIndex secondPreviewFrameCount = 1001;
	const uint32_t callsBeforeVolumeChange = 3;
	const uint32_t callsBeforePause = 3;
	const uint32_t pausedCallCount = 2;

	AudioSource *music = CreateNoiseSource(test, StereoChannelCount, OutputSampleRate, musicFrameCount, musicSeed);
	AudioSource *effect = CreateNoiseSource(test, MonoChannelCount, SincSampleRate, effectFrameCount, effectSeed);
	if(music == fpl_null || effect == fpl_null) {
		return;
	}
	AudioPlayItemID musicId = PlayNoiseSource(test, music, false, musicVolume, AudioSystemDefaultPitch);
	AudioPlayItemID effectId = PlayNoiseSource(test, effect, false, effectVolume, AudioSystemDefaultPitch);

	// Like the player, which renders a preview right after it started a track
	MixPreview(test, firstPreviewFrameCount);
	MixCalls(test, callsBeforeVolumeChange);

	bool isVolumeChanged = AudioSystemSetPlayItemVolume(test->audioSys, effectId, changedEffectVolume);
	CheckTrue(test, isVolumeChanged, "change the volume of a playing item");
	// A preview right after a change must already hear it
	MixPreview(test, secondPreviewFrameCount);
	MixCalls(test, callsBeforePause);

	bool isPaused = AudioSystemSetPlayItemPaused(test->audioSys, musicId, true);
	CheckTrue(test, isPaused, "pause a playing item");
	MixCalls(test, pausedCallCount);
	bool isResumed = AudioSystemSetPlayItemPaused(test->audioSys, musicId, false);
	CheckTrue(test, isResumed, "resume a paused item");

	MixUntilItemCount(test, 0);
	MixCalls(test, TailCallCount);
}

static void RunEvenRatiosCase(MixTest *test) {
	const AudioFrameIndex loopFrameCount = 3001;
	const AudioFrameIndex fastFrameCount = 15001;
	const uint32_t loopSeed = 3;
	const uint32_t fastSeed = 4;
	const float loopVolume = 0.5f;
	const float fastVolume = 0.6f;
	const float duplicationPitch = 0.5f;
	const float skippingPitch = 2.0f;
	const uint32_t callsBeforePitchChange = 2;
	const uint32_t callsWithChangedPitch = 1;
	const uint32_t callsAfterFastEnded = 4;
	const size_t loopOnlyItemCount = 1;

	// Mono into 6 channels is broadcast, stereo into 6 channels gets a mono downmix in the other four
	AudioSource *loop = CreateNoiseSource(test, MonoChannelCount, OutputSampleRate, loopFrameCount, loopSeed);
	AudioSource *fast = CreateNoiseSource(test, StereoChannelCount, OutputSampleRate, fastFrameCount, fastSeed);
	if(loop == fpl_null || fast == fpl_null) {
		return;
	}
	AudioPlayItemID loopId = PlayNoiseSource(test, loop, true, loopVolume, duplicationPitch);
	PlayNoiseSource(test, fast, false, fastVolume, skippingPitch);
	MixCalls(test, callsBeforePitchChange);

	// Switches the loop from frame duplication to frame skipping and back while both play, inside a duplicated frame. The loop wraps in both paths
	bool isSkippingPitchSet = AudioSystemSetPlayItemPitch(test->audioSys, loopId, skippingPitch);
	CheckTrue(test, isSkippingPitchSet, "change the pitch of a playing item");
	MixCalls(test, callsWithChangedPitch);
	bool isDuplicationPitchSet = AudioSystemSetPlayItemPitch(test->audioSys, loopId, duplicationPitch);
	CheckTrue(test, isDuplicationPitchSet, "change the pitch back");

	// The fast item ends inside a call while the loop goes on
	MixUntilItemCount(test, loopOnlyItemCount);
	MixCalls(test, callsAfterFastEnded);

	bool isLoopStopped = AudioSystemStopOne(test->audioSys, loopId);
	CheckTrue(test, isLoopStopped, "stop a repeating item");
	MixCalls(test, TailCallCount);
}

static void RunFourItemsCase(MixTest *test) {
	const AudioFrameIndex firstFrameCount = 30000;
	const AudioFrameIndex shortFrameCount = 9001;
	const AudioFrameIndex monoFrameCount = 40000;
	const AudioFrameIndex halfRateFrameCount = 20000;
	const uint32_t firstSeed = 5;
	const uint32_t shortSeed = 6;
	const uint32_t monoSeed = 7;
	const uint32_t halfRateSeed = 8;
	const float itemVolume = 0.3f;
	const float staleVolume = 0.9f;
	const size_t itemCountAfterShortEnded = 3;
	const uint32_t callsBeforeStop = 4;

	AudioSource *first = CreateNoiseSource(test, StereoChannelCount, OutputSampleRate, firstFrameCount, firstSeed);
	AudioSource *shortSource = CreateNoiseSource(test, StereoChannelCount, SincSampleRate, shortFrameCount, shortSeed);
	AudioSource *mono = CreateNoiseSource(test, MonoChannelCount, OutputSampleRate, monoFrameCount, monoSeed);
	AudioSource *halfRate = CreateNoiseSource(test, MonoChannelCount, HalfOutputSampleRate, halfRateFrameCount, halfRateSeed);
	if(first == fpl_null || shortSource == fpl_null || mono == fpl_null || halfRate == fpl_null) {
		return;
	}
	PlayNoiseSource(test, first, false, itemVolume, AudioSystemDefaultPitch);
	AudioPlayItemID shortId = PlayNoiseSource(test, shortSource, false, itemVolume, AudioSystemDefaultPitch);
	AudioPlayItemID monoId = PlayNoiseSource(test, mono, false, itemVolume, AudioSystemDefaultPitch);
	PlayNoiseSource(test, halfRate, false, itemVolume, AudioSystemDefaultPitch);

	// The second item ends inside a call, the other three go on in their order
	MixUntilItemCount(test, itemCountAfterShortEnded);

	bool isStaleVolumeChanged = AudioSystemSetPlayItemVolume(test->audioSys, shortId, staleVolume);
	CheckTrue(test, !isStaleVolumeChanged, "a setter on an ended item returns false");
	bool isStaleStopped = AudioSystemStopOne(test->audioSys, shortId);
	CheckTrue(test, !isStaleStopped, "a stop on an ended item returns false");

	// Plays the ended source again, it comes last
	PlayNoiseSource(test, shortSource, false, itemVolume, AudioSystemDefaultPitch);
	MixCalls(test, callsBeforeStop);

	// Stops the second of four items
	bool isMonoStopped = AudioSystemStopOne(test->audioSys, monoId);
	CheckTrue(test, isMonoStopped, "stop an item in the middle");

	MixUntilItemCount(test, 0);
	MixCalls(test, TailCallCount);
}

// The stale-ids case plays a few tones at once, the copies have room to spare
#define STALE_ID_COPY_CAPACITY 16

static void RunStaleIdsCase(MixTest *test) {
	// More than twice the default pool of 256 play items, so every slot plays several tones
	const uint32_t toneCount = 600;
	// Longer than most calls, so a few tones play at once and end inside calls
	const AudioFrameIndex toneFrameCount = 1200;
	const uint32_t toneSeed = 9;
	const float toneVolume = 0.25f;
	const float staleVolume = 0.95f;

	AudioSource *tone = CreateNoiseSource(test, MonoChannelCount, OutputSampleRate, toneFrameCount, toneSeed);
	if(tone == fpl_null) {
		return;
	}
	AudioPlayItemID firstId = PlayNoiseSource(test, tone, false, toneVolume, AudioSystemDefaultPitch);
	for(uint32_t toneIndex = 1; toneIndex < toneCount; ++toneIndex) {
		MixCalls(test, 1);
		PlayNoiseSource(test, tone, false, toneVolume, AudioSystemDefaultPitch);
	}

	// The first tone ended long ago, its slot has played other tones since
	bool isStaleVolumeChanged = AudioSystemSetPlayItemVolume(test->audioSys, firstId, staleVolume);
	CheckTrue(test, !isStaleVolumeChanged, "a setter on the id of a reused slot returns false");
	size_t itemCountBeforeStop = GetPlayItemCount(test);
	bool isStaleStopped = AudioSystemStopOne(test->audioSys, firstId);
	CheckTrue(test, !isStaleStopped, "a stop on the id of a reused slot returns false");
	size_t itemCountAfterStop = GetPlayItemCount(test);
	CheckTrue(test, itemCountAfterStop == itemCountBeforeStop, "a stop on the id of a reused slot stops nothing");

	AudioPlayItem playItemCopies[STALE_ID_COPY_CAPACITY];
	size_t copyCount = AudioSystemGetPlayItems(test->audioSys, playItemCopies, STALE_ID_COPY_CAPACITY);
	CheckTrue(test, copyCount == itemCountAfterStop, "copy the playing tones");
	for(size_t copyIndex = 0; copyIndex < copyCount; ++copyIndex) {
		CheckTrue(test, playItemCopies[copyIndex].volume == toneVolume, "a setter on the id of a reused slot changes no other tone");
	}

	MixUntilItemCount(test, 0);
	MixCalls(test, TailCallCount);
}

// One mix call with advance, every sample of it must be 0 because the two sine waves cancel each other
static void MixCancellingCall(MixTest *test) {
	const uint32_t cycleIndex = (uint32_t)(test->callCount % fplArrayCount(CallFrameCounts));
	const AudioFrameIndex frameCount = CallFrameCounts[cycleIndex];
	MixCall(test, frameCount, true);
	const size_t sampleCount = (size_t)frameCount * test->targetFormat.channels;
	bool isSilent = true;
	for(size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		if(test->callSamples[sampleIndex] != 0.0f) {
			isSilent = false;
			break;
		}
	}
	CheckTrue(test, isSilent, "the stream cancels the sine wave in memory");
}

static void MixCancellingCalls(MixTest *test, const uint32_t count) {
	for(uint32_t callIndex = 0; callIndex < count; ++callIndex) {
		MixCancellingCall(test);
	}
}

static void RunStreamCancelCase(MixTest *test) {
	const AudioDuration sineSeconds = 1.5;
	const AudioHertz sineFrequency = 997;
	const double sineToneVolume = 0.5;
	const float memoryVolume = 0.75f;
	const float streamVolume = -0.75f;
	// Through SinC, then frame skipping with factor 2, which steps past the end of the ring
	const float sincPitch = 1.37f;
	const float skippingPitch = 2.0f;
	const AudioFrameIndex previewFrameCount = 1024;
	const uint32_t callsPerStep = 9;

	AudioSineWaveData sineWave = fplZeroInit;
	sineWave.duration = sineSeconds;
	sineWave.toneVolume = sineToneVolume;
	sineWave.frequency = sineFrequency;

	const double roundingOffset = 0.5;
	const double frameCountValue = (double)OutputSampleRate * sineSeconds + roundingOffset;
	const AudioFrameIndex frameCount = (AudioFrameIndex)frameCountValue;
	AudioSource *memorySine = AudioSystemAllocateSource(test->audioSys, StereoChannelCount, OutputSampleRate, fplAudioFormatType_S16, frameCount);
	AudioSource *streamSine = AudioSystemCreateSineStreamSource(test->audioSys, StereoChannelCount, OutputSampleRate, &sineWave);
	CheckTrue(test, memorySine != fpl_null && streamSine != fpl_null, "create both sine waves");
	if(memorySine == fpl_null || streamSine == fpl_null) {
		return;
	}
	CheckTrue(test, streamSine->buffer.frameCount == frameCount, "the stream is as long as the sine wave in memory");
	AudioSineWaveData memoryWave = sineWave;
	AudioGenerateSineWave(&memoryWave, memorySine->buffer.samples, memorySine->format.format, memorySine->format.sampleRate, memorySine->format.channels, frameCount);
	bool isMemoryAdded = AudioSystemAddSource(test->audioSys, memorySine);
	bool isStreamAdded = AudioSystemAddSource(test->audioSys, streamSine);
	CheckTrue(test, isMemoryAdded && isStreamAdded, "add both sine waves");

	AudioPlayItemID memoryId = PlayNoiseSource(test, memorySine, true, memoryVolume, AudioSystemDefaultPitch);
	AudioPlayItemID streamId = PlayNoiseSource(test, streamSine, true, streamVolume, AudioSystemDefaultPitch);
	MixCancellingCalls(test, callsPerStep);

	// A preview mixes the sine wave in memory only and moves nothing
	MixPreview(test, previewFrameCount);
	MixCancellingCalls(test, callsPerStep);

	AudioSystemSetPlayItemPitch(test->audioSys, memoryId, sincPitch);
	AudioSystemSetPlayItemPitch(test->audioSys, streamId, sincPitch);
	MixCancellingCalls(test, callsPerStep);
	MixPreview(test, previewFrameCount);

	AudioSystemSetPlayItemPitch(test->audioSys, memoryId, skippingPitch);
	AudioSystemSetPlayItemPitch(test->audioSys, streamId, skippingPitch);
	MixCancellingCalls(test, callsPerStep);

	AudioSystemSetPlayItemPaused(test->audioSys, memoryId, true);
	AudioSystemSetPlayItemPaused(test->audioSys, streamId, true);
	MixCancellingCalls(test, callsPerStep);
	AudioSystemSetPlayItemPaused(test->audioSys, memoryId, false);
	AudioSystemSetPlayItemPaused(test->audioSys, streamId, false);
	MixCancellingCalls(test, callsPerStep);

	// Both wrap around more than once at pitch 2, then they end in the same call
	AudioSystemSetPlayItemRepeat(test->audioSys, memoryId, false);
	AudioSystemSetPlayItemRepeat(test->audioSys, streamId, false);
	uint32_t callIndex = 0;
	size_t itemCount = GetPlayItemCount(test);
	bool isOneItemLeft = false;
	while(itemCount > 0 && callIndex < MaxCallCount) {
		MixCancellingCall(test);
		itemCount = GetPlayItemCount(test);
		if(itemCount == 1) {
			isOneItemLeft = true;
		}
		++callIndex;
	}
	CheckTrue(test, itemCount == 0, "both sine waves end");
	CheckTrue(test, !isOneItemLeft, "both sine waves end in the same call");
	MixCancellingCalls(test, TailCallCount);

	// The ended stream keeps its instance until a caller side function frees it
	AudioSystemStats stats = fplZeroInit;
	AudioSystemGetStats(test->audioSys, &stats);
	CheckTrue(test, stats.streamSourceCount == 1, "one stream source");
	CheckTrue(test, stats.streamInstanceCount == 1, "the ended stream keeps its instance");
	CheckTrue(test, stats.failedStreamCount == 0, "the stream never failed");
	AudioSystemStopAll(test->audioSys);
	AudioSystemGetStats(test->audioSys, &stats);
	CheckTrue(test, stats.streamInstanceCount == 0 && stats.streamInstanceByteCount == 0, "AudioSystemStopAll frees the ended stream instance");
}

typedef void (MixCaseFunction)(MixTest *test);

typedef struct MixCase {
	const char *name;
	AudioChannelIndex outputChannelCount;
	MixCaseFunction *run;
} MixCase;

static const MixCase MixCases[] = {
	{ "pass-sinc", MixOutputChannels_Stereo, RunPassthroughAndSincCase },
	{ "down2-up2", MixOutputChannels_Surround, RunEvenRatiosCase },
	{ "four-items", MixOutputChannels_Stereo, RunFourItemsCase },
	{ "stale-ids", MixOutputChannels_Stereo, RunStaleIdsCase },
	{ "stream-cancel", MixOutputChannels_Stereo, RunStreamCancelCase },
};

static uint64_t RunMixCase(const MixCase *mixCase, const bool isVerbose, uint32_t *outFailedCheckCount) {
	MixTest test = fplZeroInit;
	test.targetFormat.type = fplAudioFormatType_F32;
	test.targetFormat.sampleRate = OutputSampleRate;
	test.targetFormat.channels = mixCase->outputChannelCount;
	test.hash = Fnv1a64OffsetBasis;
	test.isVerbose = isVerbose;

	size_t callSampleCount = (size_t)MaxCallFrameCount * MaxOutputChannelCount;
	test.callSamples = (float *)fplMemoryAllocate(callSampleCount * sizeof(float));
	test.audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = false;
	if(test.callSamples != fpl_null && test.audioSys != fpl_null) {
		isInitialized = AudioSystemInit(test.audioSys, &test.targetFormat);
	}
	CheckTrue(&test, isInitialized, "initialize the audio system");

	size_t itemsLeftCount = 0;
	if(isInitialized) {
		mixCase->run(&test);
		itemsLeftCount = GetPlayItemCount(&test);
		CheckTrue(&test, itemsLeftCount == 0, "no item is left at the end");
		AudioSystemShutdown(test.audioSys);
	}

	printf("%s: calls=%u frames=%zu items_left=%zu hash=%016llx\n", mixCase->name, test.callCount, test.frameCount, itemsLeftCount, (unsigned long long)test.hash);

	if(test.audioSys != fpl_null) {
		fplMemoryFree(test.audioSys);
	}
	if(test.callSamples != fpl_null) {
		fplMemoryFree(test.callSamples);
	}
	*outFailedCheckCount = test.failedCheckCount;
	return(test.hash);
}

//
// Two mixing threads at the same time (--concurrent)
//

#define CONCURRENT_MIXER_COUNT 2

static const uint32_t ConcurrentCallCount = 2000;
static const AudioFrameIndex ConcurrentCallFrameCount = 64;

typedef struct ConcurrentMixer {
	AudioSystem *audioSys;
	const fplAudioFormat *targetFormat;
	float *samples;
	volatile uint32_t *readyCount;
	volatile uint32_t *isStarted;
} ConcurrentMixer;

static void ConcurrentMixThread(const fplThreadHandle *thread, void *data) {
	(void)thread;
	ConcurrentMixer *mixer = (ConcurrentMixer *)data;
	// Both threads start mixing at the same moment, so their calls overlap
	fplAtomicIncrementU32(mixer->readyCount);
	while(fplAtomicLoadU32(mixer->isStarted) == 0) {
		fplThreadYield();
	}
	for(uint32_t callIndex = 0; callIndex < ConcurrentCallCount; ++callIndex) {
		AudioSystemWriteFrames(mixer->audioSys, mixer->samples, mixer->targetFormat, ConcurrentCallFrameCount, true);
	}
}

// Two threads mix one item at the same time, a source in memory or a sine wave stream. Returns 0 when the item moved on by exactly the frames of the calls that got the mixer
static int RunConcurrentMixers(const bool isStream) {
	const int failedExitCode = 1;
	const char *lineName = isStream ? "concurrent-stream" : "concurrent";
	const double sineToneVolume = 0.5;
	const AudioHertz sineFrequency = 440;
	fplAudioFormat targetFormat = fplZeroInit;
	targetFormat.type = fplAudioFormatType_F32;
	targetFormat.sampleRate = OutputSampleRate;
	targetFormat.channels = StereoChannelCount;

	AudioSystem *audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = false;
	if(audioSys != fpl_null) {
		isInitialized = AudioSystemInit(audioSys, &targetFormat);
	}
	if(!isInitialized) {
		fprintf(stderr, "audio system init failed\n");
		return(failedExitCode);
	}

	// Passthrough and longer than all calls together, so the position counts every mixed frame
	const uint32_t totalCallCount = ConcurrentCallCount * CONCURRENT_MIXER_COUNT;
	const AudioFrameIndex sourceFrameCount = totalCallCount * ConcurrentCallFrameCount + ConcurrentCallFrameCount;
	AudioSource *source;
	if(isStream) {
		AudioSineWaveData sineWave = fplZeroInit;
		sineWave.duration = (AudioDuration)sourceFrameCount / (AudioDuration)OutputSampleRate;
		sineWave.toneVolume = sineToneVolume;
		sineWave.frequency = sineFrequency;
		source = AudioSystemCreateSineStreamSource(audioSys, StereoChannelCount, OutputSampleRate, &sineWave);
	} else {
		source = AudioSystemAllocateSource(audioSys, StereoChannelCount, OutputSampleRate, fplAudioFormatType_S16, sourceFrameCount);
	}
	bool isAdded = false;
	if(source != fpl_null) {
		isAdded = AudioSystemAddSource(audioSys, source);
	}
	AudioPlayItemID playId = fplZeroInit;
	if(isAdded) {
		playId = AudioSystemPlaySource(audioSys, source, false, 1.0f, AudioSystemDefaultPitch);
	}

	volatile uint32_t readyCount = 0;
	volatile uint32_t isStarted = 0;
	ConcurrentMixer mixers[CONCURRENT_MIXER_COUNT];
	fplThreadHandle *threads[CONCURRENT_MIXER_COUNT];
	const size_t callSampleCount = (size_t)ConcurrentCallFrameCount * StereoChannelCount;
	for(uint32_t mixerIndex = 0; mixerIndex < CONCURRENT_MIXER_COUNT; ++mixerIndex) {
		ConcurrentMixer *mixer = &mixers[mixerIndex];
		mixer->audioSys = audioSys;
		mixer->targetFormat = &targetFormat;
		mixer->samples = (float *)fplMemoryAllocate(callSampleCount * sizeof(float));
		mixer->readyCount = &readyCount;
		mixer->isStarted = &isStarted;
		threads[mixerIndex] = fplThreadCreate(ConcurrentMixThread, mixer);
	}
	while(fplAtomicLoadU32(&readyCount) < CONCURRENT_MIXER_COUNT) {
		fplThreadYield();
	}
	fplAtomicStoreU32(&isStarted, 1);
	for(uint32_t mixerIndex = 0; mixerIndex < CONCURRENT_MIXER_COUNT; ++mixerIndex) {
		fplThreadWaitForOne(threads[mixerIndex], FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(threads[mixerIndex]);
		fplMemoryFree(mixers[mixerIndex].samples);
	}

	AudioSystemStats stats = fplZeroInit;
	AudioSystemGetStats(audioSys, &stats);
	AudioPlayItem playItem = fplZeroInit;
	size_t playItemCount = AudioSystemGetPlayItems(audioSys, &playItem, 1);
	const bool isPlaying = (playId.value != 0) && (playItemCount == 1) && (playItem.id.value == playId.value);
	const uint32_t acceptedCallCount = totalCallCount - stats.concurrentWriteCount;
	const AudioFrameIndex expectedFrameCount = acceptedCallCount * ConcurrentCallFrameCount;
	const bool isExact = isPlaying && (playItem.framesPlayed[0] == expectedFrameCount) && (stats.failedStreamCount == 0);
	printf("%s: calls=%u rejected=%u frames_played=%u expected=%u %s\n", lineName, totalCallCount, stats.concurrentWriteCount, playItem.framesPlayed[0], expectedFrameCount, isExact ? "ok" : "FAILED");
	if(stats.concurrentWriteCount == 0) {
		printf("%s: the calls never overlapped, the check above is weak\n", lineName);
	}

	AudioSystemShutdown(audioSys);
	fplMemoryFree(audioSys);
	return(isExact ? 0 : failedExitCode);
}

int main(int argc, char **argv) {
	bool isVerbose = (argc > 1) && (strcmp(argv[1], "-v") == 0);
	bool isConcurrent = (argc > 1) && (strcmp(argv[1], "--concurrent") == 0);

	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}

	if(isConcurrent) {
		int memoryResult = RunConcurrentMixers(false);
		int streamResult = RunConcurrentMixers(true);
		fplPlatformRelease();
		int concurrentResult = (memoryResult != 0) ? memoryResult : streamResult;
		return(concurrentResult);
	}

	// The total hash runs over the hashes of the cases, so one line tells whether anything changed
	uint64_t totalHash = Fnv1a64OffsetBasis;
	uint32_t totalFailedCheckCount = 0;
	for(size_t caseIndex = 0; caseIndex < fplArrayCount(MixCases); ++caseIndex) {
		uint32_t failedCheckCount = 0;
		uint64_t caseHash = RunMixCase(&MixCases[caseIndex], isVerbose, &failedCheckCount);
		totalHash = HashFnv1a64(totalHash, &caseHash, sizeof(caseHash));
		totalFailedCheckCount += failedCheckCount;
	}
	printf("total: %zu cases, %u failed checks, hash=%016llx\n", fplArrayCount(MixCases), totalFailedCheckCount, (unsigned long long)totalHash);

	fplPlatformRelease();
	return(totalFailedCheckCount > 0 ? 1 : 0);
}
