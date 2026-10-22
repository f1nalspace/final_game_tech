/*
Checks the loop regions that the loaders of final_audiosystem.h read from the tags of files.

Vorbis: lists of comments go straight into ReadVorbisLoopFromComments of final_vorbisloader.h: LOOPSTART with LOOPLENGTH or LOOPEND, LOOPSTART alone, the case of field names, values that are no number, regions outside the file, start plus length past 32 bits.
Wave: files built in memory load with LoadWaveFormatFromBuffer, LoadWaveFromBuffer, AudioSystemLoadDataFormat and AudioSystemLoadDataSource, all must give the same frame count and region.
The smpl chunk after or before data and fmt and after an odd chunk, forward loops only, the first loop counts, loops past the file, broken smpl chunks, the pad byte of odd chunks and chunk sizes that overflow a 32-bit position.
Files: every <file> <loop start> <loop end> given loads with AudioSystemLoadFileFormat, AudioSystemLoadFileSource and, for Ogg Vorbis and mp3, AudioSystemLoadFileStreamSource.
All must have exactly that region (0 0 for none), and so must a repeating play item of every source.

Build against the additions to test, with assertions:
gcc -std=c99 -O2 -DFPL_FORCE_ASSERTIONS -I <repo> -I <additions> -I <repo>/demos/dependencies loop_tag_test.c -o loop_tag_test -lm -ldl -lpthread
Usage: loop_tag_test [<file> <loop start> <loop end>]...
Output: one line per case "<name>: <ok|FAILED>" (files with the regions they got), then "total: <n> failed checks".
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#include <final_platform_layer.h>

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const AudioHertz OutputSampleRate = 48000;
static const AudioChannelIndex StereoChannelCount = 2;

// Frames that a play item mixes before its region is read back, the samples are F32 stereo
#define PLAY_CHECK_FRAME_COUNT 256
#define PLAY_CHECK_SAMPLE_COUNT (PLAY_CHECK_FRAME_COUNT * 2)
// Room for more play items than the one expected, so a play item left over shows as a wrong count
#define MAX_PLAY_CHECK_ITEM_COUNT 4

typedef struct TagTest {
	AudioSystem *audioSys;
	fplAudioFormat targetFormat;
	uint32_t failedCheckCount;
} TagTest;

static bool CheckTrue(TagTest *test, const bool condition, const char *description) {
	if(!condition) {
		printf("  check failed: %s\n", description);
		++test->failedCheckCount;
	}
	return(condition);
}

static bool CheckRegion(TagTest *test, const char *what, const uint32_t loopStartFrame, const uint32_t loopEndFrame, const uint32_t expectedLoopStartFrame, const uint32_t expectedLoopEndFrame) {
	const bool isSame = (loopStartFrame == expectedLoopStartFrame) && (loopEndFrame == expectedLoopEndFrame);
	if(!isSame) {
		printf("  check failed: %s has region %u-%u, expected %u-%u\n", what, loopStartFrame, loopEndFrame, expectedLoopStartFrame, expectedLoopEndFrame);
		++test->failedCheckCount;
	}
	return(isSame);
}

static void PrintCaseResult(const char *name, const uint32_t failedCountBefore, const uint32_t failedCountAfter) {
	const bool isOk = (failedCountAfter == failedCountBefore);
	printf("%s: %s\n", name, isOk ? "ok" : "FAILED");
}

//
// Vorbis comments
//
#define MAX_VORBIS_TEST_COMMENT_COUNT 4

typedef struct VorbisCommentCase {
	const char *name;
	const char *comments[MAX_VORBIS_TEST_COMMENT_COUNT];
	uint32_t frameCount;
	uint32_t expectedLoopStartFrame;
	uint32_t expectedLoopEndFrame;
} VorbisCommentCase;

#define VORBIS_TEST_FRAME_COUNT 100000

static const VorbisCommentCase VorbisCommentCases[] = {
	{ "vorbis-start-length", { "LOOPSTART=1000", "LOOPLENGTH=5000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-start-end", { "LOOPSTART=1000", "LOOPEND=6000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-other-comments-between", { "LOOPLENGTH=5000", "TITLE=Crackout", "LOOPSTART=1000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-length-before-end", { "LOOPSTART=1000", "LOOPEND=9000", "LOOPLENGTH=5000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-start-alone", { "LOOPSTART=1000" }, VORBIS_TEST_FRAME_COUNT, 1000, VORBIS_TEST_FRAME_COUNT },
	{ "vorbis-start-zero-alone", { "LOOPSTART=0" }, VORBIS_TEST_FRAME_COUNT, 0, VORBIS_TEST_FRAME_COUNT },
	{ "vorbis-lower-case", { "loopstart=1000", "LoopLength=5000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-leading-zeros", { "LOOPSTART=0000001000", "LOOPEND=00000000000006000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-end-at-file-end", { "LOOPSTART=1000", "LOOPEND=100000" }, VORBIS_TEST_FRAME_COUNT, 1000, VORBIS_TEST_FRAME_COUNT },
	{ "vorbis-length-to-file-end", { "LOOPSTART=1000", "LOOPLENGTH=99000" }, VORBIS_TEST_FRAME_COUNT, 1000, VORBIS_TEST_FRAME_COUNT },
	{ "vorbis-one-frame", { "LOOPSTART=99999", "LOOPLENGTH=1" }, VORBIS_TEST_FRAME_COUNT, 99999, VORBIS_TEST_FRAME_COUNT },
	{ "vorbis-first-start-counts", { "LOOPSTART=1000", "LOOPSTART=2000", "LOOPLENGTH=5000" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-bad-end-after-length", { "LOOPSTART=1000", "LOOPLENGTH=5000", "LOOPEND=abc" }, VORBIS_TEST_FRAME_COUNT, 1000, 6000 },
	{ "vorbis-largest-start", { "LOOPSTART=4294967294" }, UINT32_MAX, 4294967294u, UINT32_MAX },
	{ "vorbis-no-start", { "LOOPLENGTH=5000", "LOOPEND=6000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-no-tags", { "TITLE=Crackout", "ARTIST=Final" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-no-comments", { fpl_null }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-end-past-file", { "LOOPSTART=1000", "LOOPEND=100001" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-length-past-file", { "LOOPSTART=1000", "LOOPLENGTH=99001" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-length-zero", { "LOOPSTART=1000", "LOOPLENGTH=0" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-end-before-start", { "LOOPSTART=6000", "LOOPEND=1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-end-at-start", { "LOOPSTART=6000", "LOOPEND=6000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-start-at-file-end", { "LOOPSTART=100000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-empty-file", { "LOOPSTART=0" }, 0, 0, 0 },
	{ "vorbis-length-past-32-bit", { "LOOPSTART=4294967295", "LOOPLENGTH=2" }, UINT32_MAX, 0, 0 },
	{ "vorbis-start-past-32-bit", { "LOOPSTART=4294967296" }, UINT32_MAX, 0, 0 },
	{ "vorbis-start-many-digits", { "LOOPSTART=99999999999999999999999" }, UINT32_MAX, 0, 0 },
	{ "vorbis-space-before-value", { "LOOPSTART= 1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-space-after-value", { "LOOPSTART=1000 " }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-plus-sign", { "LOOPSTART=+1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-minus-sign", { "LOOPSTART=-1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-hex-value", { "LOOPSTART=0x10" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-empty-value", { "LOOPSTART=" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-bad-length", { "LOOPSTART=1000", "LOOPLENGTH=abc" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-bad-end", { "LOOPSTART=1000", "LOOPEND=6000s" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-longer-field-name", { "LOOPSTARTX=1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-shorter-field-name", { "LOOPSTAR=1000" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
	{ "vorbis-field-without-value", { "LOOPSTART" }, VORBIS_TEST_FRAME_COUNT, 0, 0 },
};

static void RunVorbisCommentCase(TagTest *test, const VorbisCommentCase *testCase) {
	const uint32_t failedCountBefore = test->failedCheckCount;

	// stb_vorbis hands out writable strings, the cases are literals
	char commentTexts[MAX_VORBIS_TEST_COMMENT_COUNT][64];
	char *commentList[MAX_VORBIS_TEST_COMMENT_COUNT];
	int commentCount = 0;
	for(uint32_t commentIndex = 0; commentIndex < MAX_VORBIS_TEST_COMMENT_COUNT; ++commentIndex) {
		const char *comment = testCase->comments[commentIndex];
		if(comment == fpl_null) {
			break;
		}
		fplCopyString(comment, commentTexts[commentCount], sizeof(commentTexts[commentCount]));
		commentList[commentCount] = commentTexts[commentCount];
		++commentCount;
	}
	char vendor[] = "loop_tag_test";
	stb_vorbis_comment comments = fplZeroInit;
	comments.vendor = vendor;
	comments.comment_list_length = commentCount;
	comments.comment_list = (commentCount > 0) ? commentList : fpl_null;

	// Garbage before the call, the parser must write both values
	const uint32_t garbageFrameIndex = 0xCDCDCDCD;
	uint32_t loopStartFrame = garbageFrameIndex;
	uint32_t loopEndFrame = garbageFrameIndex;
	ReadVorbisLoopFromComments(&comments, testCase->frameCount, &loopStartFrame, &loopEndFrame);
	CheckRegion(test, "the comments", loopStartFrame, loopEndFrame, testCase->expectedLoopStartFrame, testCase->expectedLoopEndFrame);

	PrintCaseResult(testCase->name, failedCountBefore, test->failedCheckCount);
}

//
// Wave files built in memory
//
#define MAX_WAVE_TEST_FILE_SIZE 8192
#define MAX_WAVE_TEST_LOOP_COUNT 2
#define MAX_WAVE_TEST_CHUNK_COUNT 5

// 16-bit stereo at 44.1 kHz, its data chunk is even
static const uint32_t WaveTestFrameCount = 1000;
static const uint16_t WaveTestChannelCount = 2;
static const uint32_t WaveTestSampleRate = 44100;
static const uint16_t WaveTestBitsPerSample = 16;
// 8-bit mono at 8 kHz, its data chunk is odd
static const uint32_t OddWaveTestFrameCount = 101;
static const uint16_t OddWaveTestChannelCount = 1;
static const uint32_t OddWaveTestSampleRate = 8000;
static const uint16_t OddWaveTestBitsPerSample = 8;

typedef struct WaveFileBuilder {
	uint8_t bytes[MAX_WAVE_TEST_FILE_SIZE];
	size_t size;
} WaveFileBuilder;

typedef enum WaveTestChunk {
	WaveTestChunk_None = 0,
	// fmt for 16-bit stereo at 44.1 kHz
	WaveTestChunk_Format,
	// fmt for 8-bit mono at 8 kHz
	WaveTestChunk_OddFormat,
	// WaveTestFrameCount frames of 16-bit stereo
	WaveTestChunk_Data,
	// OddWaveTestFrameCount frames of 8-bit mono with its pad byte
	WaveTestChunk_OddData,
	// The same without the pad byte, as the last chunk of the file
	WaveTestChunk_OddDataWithoutPad,
	// smpl from the first sampler of the case
	WaveTestChunk_Sampler,
	// smpl from the second sampler of the case
	WaveTestChunk_SecondSampler,
	// LIST of 17 bytes with its pad byte
	WaveTestChunk_OddList,
	// JUNK header whose size wraps a 32-bit position back to the fmt chunk, its body is missing
	WaveTestChunk_HugeJunk,
	// data header whose size wraps a 32-bit position, its body is missing
	WaveTestChunk_HugeData,
} WaveTestChunk;

typedef struct WaveTestSampler {
	WaveSampleLoop loops[MAX_WAVE_TEST_LOOP_COUNT];
	uint32_t loopCount;
	// The loop count in the header, the loops that follow are loopCount
	uint32_t declaredLoopCount;
	// Bytes cut from the end of the chunk
	uint32_t cutByteCount;
} WaveTestSampler;

typedef struct WaveTestCase {
	const char *name;
	WaveTestChunk chunks[MAX_WAVE_TEST_CHUNK_COUNT];
	WaveTestSampler samplers[2];
	bool expectedIsLoaded;
	uint32_t expectedFrameCount;
	uint32_t expectedLoopStartFrame;
	uint32_t expectedLoopEndFrame;
} WaveTestCase;

// A sample loop of the case tables: identifier, type, start, end (last frame of the loop), fraction, play count
#define FORWARD_LOOP(start, end) { 0, WaveSampleLoopType_Forward, start, end, 0, 0 }
#define TYPED_LOOP(type, start, end) { 0, type, start, end, 0, 0 }
#define ONE_LOOP(loop) { { loop }, 1, 1, 0 }
#define NO_SAMPLER { { { 0 } }, 0, 0, 0 }

static const uint32_t PingPongLoopType = 1;
static const uint32_t BackwardLoopType = 2;

static const WaveTestCase WaveTestCases[] = {
	{ "wave-smpl-after-data", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(100, 899)), NO_SAMPLER }, true, 1000, 100, 900 },
	{ "wave-smpl-before-data", { WaveTestChunk_Format, WaveTestChunk_Sampler, WaveTestChunk_Data }, { ONE_LOOP(FORWARD_LOOP(100, 899)), NO_SAMPLER }, true, 1000, 100, 900 },
	{ "wave-smpl-before-fmt", { WaveTestChunk_Sampler, WaveTestChunk_Format, WaveTestChunk_Data }, { ONE_LOOP(FORWARD_LOOP(100, 899)), NO_SAMPLER }, true, 1000, 100, 900 },
	{ "wave-smpl-after-odd-list", { WaveTestChunk_Format, WaveTestChunk_OddList, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(100, 899)), NO_SAMPLER }, true, 1000, 100, 900 },
	{ "wave-smpl-after-odd-data", { WaveTestChunk_OddFormat, WaveTestChunk_OddData, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(10, 99)), NO_SAMPLER }, true, 101, 10, 100 },
	{ "wave-one-frame-loop", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(500, 500)), NO_SAMPLER }, true, 1000, 500, 501 },
	{ "wave-whole-file-loop", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(0, 999)), NO_SAMPLER }, true, 1000, 0, 1000 },
	{ "wave-last-frame-loop", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(999, 999)), NO_SAMPLER }, true, 1000, 999, 1000 },
	{ "wave-first-smpl-counts", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler, WaveTestChunk_SecondSampler }, { ONE_LOOP(FORWARD_LOOP(100, 899)), ONE_LOOP(FORWARD_LOOP(300, 699)) }, true, 1000, 100, 900 },
	{ "wave-second-smpl-after-empty", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler, WaveTestChunk_SecondSampler }, { NO_SAMPLER, ONE_LOOP(FORWARD_LOOP(300, 699)) }, true, 1000, 300, 700 },
	{ "wave-first-loop-counts", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { { { FORWARD_LOOP(100, 899), FORWARD_LOOP(300, 699) }, 2, 2, 0 }, NO_SAMPLER }, true, 1000, 100, 900 },
	{ "wave-no-smpl", { WaveTestChunk_Format, WaveTestChunk_Data }, { NO_SAMPLER, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-end-past-file", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(100, 1000)), NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-end-max", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(100, 0xFFFFFFFF)), NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-start-after-end", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(FORWARD_LOOP(600, 500)), NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-ping-pong", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(TYPED_LOOP(PingPongLoopType, 100, 899)), NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-backward", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { ONE_LOOP(TYPED_LOOP(BackwardLoopType, 100, 899)), NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-ping-pong-then-forward", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { { { TYPED_LOOP(PingPongLoopType, 100, 899), FORWARD_LOOP(300, 699) }, 2, 2, 0 }, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-smpl-without-loops", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { NO_SAMPLER, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-loop-count-without-loop", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { { { { 0 } }, 0, 1, 0 }, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-loop-count-zero", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { { { FORWARD_LOOP(100, 899) }, 1, 0, 0 }, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-truncated-loop", { WaveTestChunk_Format, WaveTestChunk_Data, WaveTestChunk_Sampler }, { { { FORWARD_LOOP(100, 899) }, 1, 1, 4 }, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-odd-list-before-data", { WaveTestChunk_Format, WaveTestChunk_OddList, WaveTestChunk_Data }, { NO_SAMPLER, NO_SAMPLER }, true, 1000, 0, 0 },
	{ "wave-odd-data-then-list", { WaveTestChunk_OddFormat, WaveTestChunk_OddData, WaveTestChunk_OddList }, { NO_SAMPLER, NO_SAMPLER }, true, 101, 0, 0 },
	{ "wave-odd-data-last-without-pad", { WaveTestChunk_OddFormat, WaveTestChunk_OddDataWithoutPad }, { NO_SAMPLER, NO_SAMPLER }, true, 101, 0, 0 },
	{ "wave-huge-chunk-size", { WaveTestChunk_Format, WaveTestChunk_HugeJunk, WaveTestChunk_Data }, { NO_SAMPLER, NO_SAMPLER }, false, 0, 0, 0 },
	{ "wave-huge-data-size", { WaveTestChunk_Format, WaveTestChunk_HugeData }, { NO_SAMPLER, NO_SAMPLER }, false, 0, 0, 0 },
};

static void AppendBytes(WaveFileBuilder *builder, const void *data, const size_t size) {
	fplAssert(builder->size + size <= MAX_WAVE_TEST_FILE_SIZE);
	if(size > 0) {
		fplMemoryCopy(data, size, builder->bytes + builder->size);
	}
	builder->size += size;
}

static void AppendUInt32(WaveFileBuilder *builder, const uint32_t value) {
	AppendBytes(builder, &value, sizeof(value));
}

static void AppendChunkHeader(WaveFileBuilder *builder, const uint32_t chunkId, const uint32_t chunkSize) {
	AppendUInt32(builder, chunkId);
	AppendUInt32(builder, chunkSize);
}

static void AppendChunk(WaveFileBuilder *builder, const uint32_t chunkId, const void *body, const uint32_t bodySize, const bool withPadByte) {
	AppendChunkHeader(builder, chunkId, bodySize);
	AppendBytes(builder, body, bodySize);
	const bool isOdd = (bodySize % 2) != 0;
	if(isOdd && withPadByte) {
		const uint8_t padByte = 0;
		AppendBytes(builder, &padByte, sizeof(padByte));
	}
}

static void AppendFormatChunk(WaveFileBuilder *builder, const uint16_t channelCount, const uint32_t sampleRate, const uint16_t bitsPerSample) {
	const uint16_t bitsPerByte = 8;
	const uint16_t blockAlign = channelCount * (bitsPerSample / bitsPerByte);
	WaveFormatEx format = fplZeroInit;
	format.formatTag = WaveFormatTags_PCM;
	format.numberOfChannels = channelCount;
	format.samplesPerSecond = sampleRate;
	format.avgBytesPerSample = sampleRate * blockAlign;
	format.blockAlign = blockAlign;
	format.bitsPerSample = bitsPerSample;
	// The PCM fmt chunk ends before cbSize
	const uint32_t formatChunkSize = 16;
	AppendChunk(builder, WaveChunkId_Format, &format, formatChunkSize, true);
}

static void AppendSamplerChunk(WaveFileBuilder *builder, const WaveTestSampler *sampler) {
	uint8_t body[sizeof(WaveSamplerHeader) + sizeof(WaveSampleLoop) * MAX_WAVE_TEST_LOOP_COUNT];
	// One sample at 44.1 kHz and middle C, like the test media
	const uint32_t samplePeriodNanoseconds = 22675;
	const uint32_t middleCMidiNote = 60;
	WaveSamplerHeader header = fplZeroInit;
	header.samplePeriod = samplePeriodNanoseconds;
	header.midiUnityNote = middleCMidiNote;
	header.sampleLoopCount = sampler->declaredLoopCount;
	fplMemoryCopy(&header, sizeof(header), body);
	const size_t loopsSize = sizeof(WaveSampleLoop) * sampler->loopCount;
	if(loopsSize > 0) {
		fplMemoryCopy(sampler->loops, loopsSize, body + sizeof(header));
	}
	const size_t fullSize = sizeof(header) + loopsSize;
	fplAssert(sampler->cutByteCount <= fullSize);
	const uint32_t bodySize = (uint32_t)(fullSize - sampler->cutByteCount);
	AppendChunk(builder, WaveChunkId_Sampler, body, bodySize, true);
}

static void BuildWaveTestFile(const WaveTestCase *testCase, WaveFileBuilder *builder) {
	// The RIFF size counts everything after its chunk header
	const uint32_t riffSizeOffset = 4;
	const uint32_t riffChunkHeaderSize = 8;
	const uint32_t junkChunkId = RIFF_ID('J', 'U', 'N', 'K');
	const uint32_t listChunkId = RIFF_ID('L', 'I', 'S', 'T');
	// INFO list with one ISFT entry of 5 bytes, 17 bytes in all
	const uint8_t oddListBody[] = { 'I', 'N', 'F', 'O', 'I', 'S', 'F', 'T', 5, 0, 0, 0, 'a', 'b', 'c', 'd', 0 };
	// Body position of a huge chunk right after fmt (12 + 24 + 8), a size of 2^32 - 32 wraps it to 12
	const uint32_t wrappingChunkSize = 0xFFFFFFE0;
	uint8_t samples[MAX_WAVE_TEST_FILE_SIZE / 2];
	fplMemorySet(samples, 0x11, sizeof(samples));
	const uint32_t bitsPerByte = 8;
	const uint32_t evenFrameSize = WaveTestChannelCount * (WaveTestBitsPerSample / bitsPerByte);
	const uint32_t oddFrameSize = OddWaveTestChannelCount * (OddWaveTestBitsPerSample / bitsPerByte);
	const uint32_t evenDataSize = WaveTestFrameCount * evenFrameSize;
	const uint32_t oddDataSize = OddWaveTestFrameCount * oddFrameSize;
	fplAssert(evenDataSize <= sizeof(samples));

	builder->size = 0;
	AppendUInt32(builder, WaveChunkId_RIFF);
	AppendUInt32(builder, 0);
	AppendUInt32(builder, WaveChunkId_WAVE);
	for(uint32_t chunkIndex = 0; chunkIndex < MAX_WAVE_TEST_CHUNK_COUNT; ++chunkIndex) {
		const WaveTestChunk chunk = testCase->chunks[chunkIndex];
		switch(chunk) {
			case WaveTestChunk_Format:
				AppendFormatChunk(builder, WaveTestChannelCount, WaveTestSampleRate, WaveTestBitsPerSample);
				break;
			case WaveTestChunk_OddFormat:
				AppendFormatChunk(builder, OddWaveTestChannelCount, OddWaveTestSampleRate, OddWaveTestBitsPerSample);
				break;
			case WaveTestChunk_Data:
				AppendChunk(builder, WaveChunkId_Data, samples, evenDataSize, true);
				break;
			case WaveTestChunk_OddData:
				AppendChunk(builder, WaveChunkId_Data, samples, oddDataSize, true);
				break;
			case WaveTestChunk_OddDataWithoutPad:
				AppendChunk(builder, WaveChunkId_Data, samples, oddDataSize, false);
				break;
			case WaveTestChunk_Sampler:
			case WaveTestChunk_SecondSampler:
			{
				const uint32_t samplerSlot = (chunk == WaveTestChunk_Sampler) ? 0 : 1;
				AppendSamplerChunk(builder, &testCase->samplers[samplerSlot]);
			} break;
			case WaveTestChunk_OddList:
				AppendChunk(builder, listChunkId, oddListBody, sizeof(oddListBody), true);
				break;
			case WaveTestChunk_HugeJunk:
				AppendChunkHeader(builder, junkChunkId, wrappingChunkSize);
				break;
			case WaveTestChunk_HugeData:
				AppendChunkHeader(builder, WaveChunkId_Data, wrappingChunkSize);
				break;
			case WaveTestChunk_None:
			default:
				break;
		}
	}
	const uint32_t riffSize = (uint32_t)(builder->size - riffChunkHeaderSize);
	fplMemoryCopy(&riffSize, sizeof(riffSize), builder->bytes + riffSizeOffset);
}

// A repeating play item of the source must get the region of the source
static void CheckPlayItemRegion(TagTest *test, AudioSource *source, const char *what, const uint32_t expectedLoopStartFrame, const uint32_t expectedLoopEndFrame) {
	const bool repeat = true;
	const float volume = 1.0f;
	const float pitch = 1.0f;
	const AudioPlayItemID playId = AudioSystemPlaySource(test->audioSys, source, repeat, volume, pitch);
	if(!CheckTrue(test, playId.value != 0, "play the source")) {
		return;
	}
	float samples[PLAY_CHECK_SAMPLE_COUNT];
	const bool advance = true;
	AudioSystemWriteFrames(test->audioSys, samples, &test->targetFormat, PLAY_CHECK_FRAME_COUNT, advance);
	AudioPlayItem playItems[MAX_PLAY_CHECK_ITEM_COUNT];
	const size_t playItemCount = AudioSystemGetPlayItems(test->audioSys, playItems, fplArrayCount(playItems));
	if(CheckTrue(test, playItemCount == 1, "exactly one play item")) {
		char description[128];
		fplStringFormat(description, sizeof(description), "the play item of %s", what);
		CheckRegion(test, description, playItems[0].loopStartFrame, playItems[0].loopEndFrame, expectedLoopStartFrame, expectedLoopEndFrame);
	}
	AudioSystemStopAll(test->audioSys);
	AudioSystemWriteFrames(test->audioSys, samples, &test->targetFormat, PLAY_CHECK_FRAME_COUNT, advance);
}

// Adds the source, checks its region and the one of a play item, then removes it again
static void CheckSourceRegion(TagTest *test, AudioSource *source, const char *what, const uint32_t expectedLoopStartFrame, const uint32_t expectedLoopEndFrame) {
	CheckRegion(test, what, source->loopStartFrame, source->loopEndFrame, expectedLoopStartFrame, expectedLoopEndFrame);
	const bool isAdded = AudioSystemAddSource(test->audioSys, source);
	if(!CheckTrue(test, isAdded, "add the source")) {
		return;
	}
	CheckPlayItemRegion(test, source, what, expectedLoopStartFrame, expectedLoopEndFrame);
	AudioSystemRemoveSource(test->audioSys, source);
}

static void RunWaveTestCase(TagTest *test, const WaveTestCase *testCase) {
	const uint32_t failedCountBefore = test->failedCheckCount;
	static WaveFileBuilder builder;
	BuildWaveTestFile(testCase, &builder);

	PCMWaveFormat format;
	fplMemorySet(&format, 0xCD, sizeof(format));
	const bool isFormatLoaded = LoadWaveFormatFromBuffer(builder.bytes, builder.size, &format);
	CheckTrue(test, isFormatLoaded == testCase->expectedIsLoaded, "LoadWaveFormatFromBuffer loads exactly the valid files");

	PCMWaveData wave;
	fplMemorySet(&wave, 0xCD, sizeof(wave));
	const bool isWaveLoaded = LoadWaveFromBuffer(builder.bytes, builder.size, &wave);
	CheckTrue(test, isWaveLoaded == testCase->expectedIsLoaded, "LoadWaveFromBuffer loads exactly the valid files");

	PCMWaveFormat systemFormat = fplZeroInit;
	const bool isSystemFormatLoaded = AudioSystemLoadDataFormat(test->audioSys, builder.size, builder.bytes, &systemFormat);
	CheckTrue(test, isSystemFormatLoaded == testCase->expectedIsLoaded, "AudioSystemLoadDataFormat loads exactly the valid files");

	AudioSource *source = AudioSystemLoadDataSource(test->audioSys, builder.size, builder.bytes);
	const bool isSourceLoaded = (source != fpl_null);
	CheckTrue(test, isSourceLoaded == testCase->expectedIsLoaded, "AudioSystemLoadDataSource loads exactly the valid files");

	if(isFormatLoaded) {
		CheckTrue(test, format.frameCount == testCase->expectedFrameCount, "frame count of LoadWaveFormatFromBuffer");
		CheckRegion(test, "LoadWaveFormatFromBuffer", format.loopStartFrame, format.loopEndFrame, testCase->expectedLoopStartFrame, testCase->expectedLoopEndFrame);
	}
	if(isWaveLoaded) {
		CheckTrue(test, wave.format.frameCount == testCase->expectedFrameCount, "frame count of LoadWaveFromBuffer");
		CheckRegion(test, "LoadWaveFromBuffer", wave.format.loopStartFrame, wave.format.loopEndFrame, testCase->expectedLoopStartFrame, testCase->expectedLoopEndFrame);
		FreeWave(&wave);
	}
	if(isSystemFormatLoaded) {
		CheckRegion(test, "AudioSystemLoadDataFormat", systemFormat.loopStartFrame, systemFormat.loopEndFrame, testCase->expectedLoopStartFrame, testCase->expectedLoopEndFrame);
	}
	if(isSourceLoaded) {
		CheckTrue(test, source->buffer.frameCount == testCase->expectedFrameCount, "frame count of AudioSystemLoadDataSource");
		CheckSourceRegion(test, source, "AudioSystemLoadDataSource", testCase->expectedLoopStartFrame, testCase->expectedLoopEndFrame);
	}

	PrintCaseResult(testCase->name, failedCountBefore, test->failedCheckCount);
}

//
// Files from the command line
//
static bool ParseFrameIndexArgument(const char *text, uint32_t *outValue) {
	char *end = fpl_null;
	const int decimalBase = 10;
	const unsigned long long value = strtoull(text, &end, decimalBase);
	const bool isNumber = (end != text) && (*end == 0) && (value <= UINT32_MAX);
	if(isNumber) {
		*outValue = (uint32_t)value;
	}
	return(isNumber);
}

static void RunFileCase(TagTest *test, const char *filePath, const uint32_t expectedLoopStartFrame, const uint32_t expectedLoopEndFrame) {
	const uint32_t failedCountBefore = test->failedCheckCount;
	char formatRegion[32] = "-";
	char fullRegion[32] = "-";
	char streamRegion[32] = "-";

	PCMWaveFormat format = fplZeroInit;
	const bool isFormatLoaded = AudioSystemLoadFileFormat(test->audioSys, filePath, &format);
	if(CheckTrue(test, isFormatLoaded, "AudioSystemLoadFileFormat")) {
		fplStringFormat(formatRegion, sizeof(formatRegion), "%u-%u", format.loopStartFrame, format.loopEndFrame);
		CheckRegion(test, "AudioSystemLoadFileFormat", format.loopStartFrame, format.loopEndFrame, expectedLoopStartFrame, expectedLoopEndFrame);
	}

	AudioSource *fullSource = AudioSystemLoadFileSource(test->audioSys, filePath);
	if(CheckTrue(test, fullSource != fpl_null, "AudioSystemLoadFileSource")) {
		fplStringFormat(fullRegion, sizeof(fullRegion), "%u-%u", fullSource->loopStartFrame, fullSource->loopEndFrame);
		CheckSourceRegion(test, fullSource, "AudioSystemLoadFileSource", expectedLoopStartFrame, expectedLoopEndFrame);
	}

	// A wave file cannot stream
	AudioSource *streamSource = AudioSystemLoadFileStreamSource(test->audioSys, filePath);
	if(streamSource != fpl_null) {
		fplStringFormat(streamRegion, sizeof(streamRegion), "%u-%u", streamSource->loopStartFrame, streamSource->loopEndFrame);
		CheckSourceRegion(test, streamSource, "AudioSystemLoadFileStreamSource", expectedLoopStartFrame, expectedLoopEndFrame);
	}

	const bool isOk = (test->failedCheckCount == failedCountBefore);
	printf("%s: format %s, full %s, stream %s, %s\n", filePath, formatRegion, fullRegion, streamRegion, isOk ? "ok" : "FAILED");
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		fprintf(stderr, "platform init failed\n");
		return(1);
	}

	TagTest test = fplZeroInit;
	test.targetFormat.type = fplAudioFormatType_F32;
	test.targetFormat.sampleRate = OutputSampleRate;
	test.targetFormat.channels = StereoChannelCount;
	test.audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	bool isInitialized = (test.audioSys != fpl_null) && AudioSystemInit(test.audioSys, &test.targetFormat);
	CheckTrue(&test, isInitialized, "initialize the audio system");

	if(isInitialized) {
		for(size_t caseIndex = 0; caseIndex < fplArrayCount(VorbisCommentCases); ++caseIndex) {
			RunVorbisCommentCase(&test, &VorbisCommentCases[caseIndex]);
		}
		for(size_t caseIndex = 0; caseIndex < fplArrayCount(WaveTestCases); ++caseIndex) {
			RunWaveTestCase(&test, &WaveTestCases[caseIndex]);
		}
		const int argumentsPerFile = 3;
		for(int argIndex = 1; argIndex + argumentsPerFile - 1 < argc; argIndex += argumentsPerFile) {
			const char *filePath = argv[argIndex];
			uint32_t expectedLoopStartFrame = 0;
			uint32_t expectedLoopEndFrame = 0;
			const bool hasStart = ParseFrameIndexArgument(argv[argIndex + 1], &expectedLoopStartFrame);
			const bool hasEnd = ParseFrameIndexArgument(argv[argIndex + 2], &expectedLoopEndFrame);
			if(CheckTrue(&test, hasStart && hasEnd, "loop start and loop end of a file are numbers")) {
				RunFileCase(&test, filePath, expectedLoopStartFrame, expectedLoopEndFrame);
			}
		}
		const bool hasLeftOverArguments = ((argc - 1) % argumentsPerFile) != 0;
		CheckTrue(&test, !hasLeftOverArguments, "every file has a loop start and a loop end");
		AudioSystemShutdown(test.audioSys);
	}
	if(test.audioSys != fpl_null) {
		fplMemoryFree(test.audioSys);
	}

	printf("total: %u failed checks\n", test.failedCheckCount);
	fplPlatformRelease();
	return(test.failedCheckCount > 0 ? 1 : 0);
}
