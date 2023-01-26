#ifndef FMP_API
#define FMP_API

#include <final_platform_layer.h>

// CExtern
#ifdef __cplusplus
#define FMP_CEXTERN extern "C"
#define FMP_CEXTERN_START() extern "C" {
#define FMP_CEXTERN_END() }
#else
#define FMP_CEXTERN
#define FMP_CEXTERN_START()
#define FMP_CEXTERN_END()
#endif

// API keyword (extern or static)
#ifndef FMP_API
#ifdef FMP_PRIVATE
#define FMP_API FMP_CEXTERN static
#else
#define FMP_API FMP_CEXTERN extern
#endif
#endif // FMP_API

#ifdef FMP_PRIVATE
#define FFMPEG_PRIVATE
#endif
#include "ffmpeg_v2.h"

// Max number of frames in the queues
#define FMP_MAX_VIDEO_FRAME_QUEUE_COUNT 3
#define FMP_MAX_SUBTITLE_FRAME_QUEUE_COUNT 3
#define FMP_MAX_AUDIO_FRAME_QUEUE_COUNT 9
#define FMP_MAX_FRAME_QUEUE_COUNT fplMax(FMP_MAX_AUDIO_FRAME_QUEUE_COUNT, fplMax(FMP_MAX_VIDEO_FRAME_QUEUE_COUNT, FMP_MAX_SUBTITLE_FRAME_QUEUE_COUNT))

// Max number of streams in a media or url
#define FMP_MAX_STREAM_COUNT 8

typedef struct fmpString {
	char *data;
	size_t len;
} fmpString;

#define FMP_MEMORY_ALLOCATE_FUNC(name) void *name(const size_t size, void *user)
typedef FMP_MEMORY_ALLOCATE_FUNC(fmp_memory_allocate_func);
#define FMP_MEMORY_REALLOCATE_FUNC(name) void *name(void *ptr, const size_t size, void *user)
typedef FMP_MEMORY_REALLOCATE_FUNC(fmp_memory_reallocate_func);
#define FMP_MEMORY_FREE_FUNC(name) void name(void *ptr, void *user)
typedef FMP_MEMORY_FREE_FUNC(fmp_memory_free_func);

typedef struct fmpMemoryAllocator {
	fmp_memory_allocate_func *alloc;
	fmp_memory_reallocate_func *realloc;
	fmp_memory_free_func *free;
	void *user;
} fmpMemoryAllocator;

typedef struct fmpStreamIndex {
	uint32_t index;
} fmpStreamIndex;

typedef enum fmpStreamType {
	fmpStreamType_Unknown = 0,
	fmpStreamType_Video,
	fmpStreamType_Audio,
	fmpStreamType_Subtitle
} fmpStreamType;

typedef struct fmpPacket {
	AVPacket *pkt;
	int64_t pos;

	int64_t pts;
	int64_t dts;

	int32_t serial;
	fmpStreamType type;

	fmpStreamIndex index;
	fpl_b32 isValid;
} fmpPacket;

typedef struct fmpPacketQueue {
	fplMutexHandle mutex;
	fplConditionVariable cond;
	AVFifo *packetList;
	uint64_t duration;
	uint32_t packetCount;
	uint32_t size;
	int32_t serial;
	int32_t abortRequest;
	fpl_b32 isValid;
} fmpPacketQueue;

typedef enum fmpPacketQueueResult {
	fmpPacketQueueResult_WriteFailed = -20,
	fmpPacketQueueResult_AllocationFailed,
	fmpPacketQueueResult_Abort = -1,
	fmpPacketQueueResult_Full = 0,
	fmpPacketQueueResult_Success = 1,
} fmpPacketQueueResult;

typedef struct fmpVideoInfo {
	AVRational sampleAspectRatio;
	AVRational displayAspectRatio;
	AVRational frameRate;
	uint64_t bitrate;
	AVPixelFormat pixelFormat;
	int32_t width;
	int32_t height;
} fmpVideoInfo;

typedef struct fmpAudioInfo {
	uint64_t bitrate;
	AVSampleFormat sampleFormat;
	uint32_t sampleRate;
	uint32_t channels;
	uint32_t bitsPerSample;
} fmpAudioInfo;

typedef struct fmpSubtitleInfo {
	int32_t unused;
} fmpSubtitleInfo;

typedef struct fmpFrameInfo {
	fmpStreamType type;
	union {
		fmpVideoInfo video;
		fmpAudioInfo audio;
		fmpSubtitleInfo subtitle;
	};
} fmpFrameInfo;

typedef struct fmpFrame {
	AVSubtitle nativeSubtitle;
	AVFrame *nativeFrame;
	int64_t pos;
	int32_t serial;
	fpl_b32 isValid;
	
} fmpFrame;

typedef struct fmpFrameQueue {
	fmpFrame queue[FMP_MAX_FRAME_QUEUE_COUNT];
	fplMutexHandle mutex;
	fplConditionVariable cond;
	fmpPacketQueue *packetQueue;
	int32_t readIndex;
	int32_t writeIndex;
	int32_t size;
	int32_t maxSize;
	int32_t keepLast;
	int32_t readIndexShown;
	fpl_b32 isValid;
} fmpFrameQueue;

typedef struct fmpDecoder {
	const FFMPEGContext *ffmpeg;
	fplConditionVariable *emptyQueueCondition;
	fplThreadHandle *thread;
	AVPacket *pendingPacket;
	fmpPacketQueue *packetQueue;
	AVCodecContext *codecCtx;
	AVRational startPtsTimeBase;
	AVRational nextPtsTimeBase;
	int64_t startPts;
	int64_t nextPts;
	int32_t packetSerial;
	int32_t finishedSerial;
	int32_t isPacketPending;
	int32_t reorderPts;
	fpl_b32 isValid;
} fmpDecoder;

typedef struct fmpClock {
	double pts;
	double ptsDrift;
	double lastUpdated;
	double speed;
	int serial;
	int paused;
	int *queueSerial;
} fmpClock;

typedef enum fmpClockSyncType {
	fmpClockSyncType_AudioMaster = 0,
	fmpClockSyncType_VideoMaster,
	fmpClockSyncType_ExternalClock,
} fmpClockSyncType;

typedef struct fmpStream {
	fmpPacketQueue packetQueue;
	fmpFrameQueue frameQueue;
	fmpDecoder decoder;
	AVStream *stream;
	AVCodecContext *codecContext;
	fmpStreamType type;
	fmpStreamIndex index;
	fpl_b32 isValid;
} fmpStream;

typedef struct fmpAudioStream {
	fmpStream base;
} fmpAudioStream;

typedef struct fmpVideoStream {
	fmpStream base;
} fmpVideoStream;

typedef struct fmpSubtitleStream {
	fmpStream base;
} fmpSubtitleStream;

typedef struct fmpCodecInfo {
	fmpString name;
	char fourcc[4];
	uint64_t id;
} fmpCodecInfo;

typedef struct fmpAudioDeviceInfo {
	AVSampleFormat sampleFormat;
	uint32_t sampleRate;
	uint32_t channels;
	uint32_t bufferSizeInFrames;
	uint32_t bufferSizeInBytes;
} fmpAudioDeviceInfo;

typedef struct fmpLanguageInfo {
	fmpString name;
	char iso639_2[3];
} fmpLanguageInfo;

typedef struct fmpStreamInfo {
	union {
		fmpAudioInfo audio;
		fmpVideoInfo video;
		fmpSubtitleInfo subtitle;
	};
	fmpCodecInfo codec;
	fmpLanguageInfo language;
	fmpStreamType type;
	fmpStreamIndex index;
} fmpStreamInfo;

typedef struct fmpSeconds {
	double value;
} fmpSeconds;

typedef enum fmpMediaSourceType {
	fmpMediaSourceType_None = 0,
	fmpMediaSourceType_URL,
	fmpMediaSourceType_File,
	fmpMediaSourceType_Memory,
} fmpMediaSourceType;

typedef struct fmpURLMediaSource {
	const char *url;
} fmpURLMediaSource;

typedef struct fmpFileMediaSource {
	const char *filePath;
} fmpFileMediaSource;

typedef struct fmpMemoryMediaSource {
	const uint8_t *memory;
	size_t offset;
	size_t size;
} fmpMemoryMediaSource;

typedef struct fmpMediaSource {
	union {
		fmpURLMediaSource url;
		fmpFileMediaSource file;
		fmpMemoryMediaSource memory;
	};
	fmpMediaSourceType type;
} fmpMediaSource;

typedef struct fmpMediaInfo {
	fmpStreamInfo streams[FMP_MAX_STREAM_COUNT];
	fmpMediaSource source;
	fmpString title;
	fmpSeconds duration;
	uint32_t streamCount;
} fmpMediaInfo;

typedef struct fmpMediaOptions {
	fmpAudioDeviceInfo audioTargetFormat;

	fmpSeconds startTime;
	fmpSeconds duration;

	fpl_b32 isVideoDisabled;
	fpl_b32 isAudioDisabled;
	fpl_b32 isSubtitleDisabled;
} fmpMediaOptions;

typedef enum fmpMediaState {
	fmpMediaState_Error = -1,
	fmpMediaState_NotInitialized = 0,
	fmpMediaState_Stopped,
	fmpMediaState_Playing,
	fmpMediaState_Paused,
} fmpMediaState;

typedef struct fmpPacketReader {
	fplConditionVariable continueCondition;
} fmpPacketReader;

typedef struct fmpMediaContext {
	fmpMediaInfo info;

	fmpAudioStream audioStream;
	fmpVideoStream videoStream;
	fmpSubtitleStream subtitleStream;

	fmpClock audioClock;
	fmpClock videoClock;
	fmpClock subtitleClock;
	fmpClock externalClock;

	fmpMediaOptions options;

	fmpMediaSource source;

	fmpPacketReader reader;

	AVFormatContext *formatCtx;

	double maxFrameDuration;

	volatile fmpMediaState state;

	fmpClockSyncType syncType;

	fpl_b32 isEOF;
	fpl_b32 isRealTime;
	fpl_b32 isValid;
} fmpMediaContext;

typedef struct fmpContext {
	FFMPEGContext ffmpeg;
	fmpMemoryAllocator allocator;
	fpl_b32 isValid;
} fmpContext;

typedef enum fmpResult {
	fmpResult_UnknownError = INT32_MIN,

	// Backend errors
	fmpResult_Backend_NotInitialized = -1000,
	fmpResult_Backend_AllocationFailed,
	fmpResult_Backend_FailedInitialization,
	fmpResult_Backend_NoDecoderFound,
	fmpResult_Backend_NotSupported,
	fmpResult_Backend_NoStreamsFound,
	fmpResult_Backend_TooManyStreams,
	fmpResult_Backend_InvalidStream,
	fmpResult_Backend_PacketSendFailure,
	fmpResult_Backend_DecodeError,

	// Media errors
	fmpResult_Media_NotLoaded = -900,
	fmpResult_Media_StillLoaded,
	fmpResult_Media_EndOfStream,

	// Threading errors
	fmpResult_Thread_FailedCreation = -800,
	fmpResult_Mutex_FailedInitialization,
	fmpResult_Mutex_NotInitialized,
	fmpResult_ConditionVariable_FailedInitialization,
	fmpResult_ConditionVariable_NotInitialized,

	// Packet/Frame/Queue errors
	fmpResult_PacketQueue_NotInitialized = -700,
	fmpResult_PacketQueue_FailedInitialization,
	fmpResult_PacketQueue_Empty,

	fmpResult_Packet_AllocationFailed,
	fmpResult_Packet_NotInitialized,
	fmpResult_Packet_AlreadyInitialized,
	fmpResult_Packet_ReadFailure,

	fmpResult_FrameQueue_NotInitialized,
	fmpResult_FrameQueue_FailedInitialization,
	fmpResult_FrameQueue_Empty,

	fmpResult_Frame_AllocationFailed,
	fmpResult_Frame_NotInitialized,
	fmpResult_Frame_AlreadyInitialized,
	fmpResult_Frame_RequireMorePackets,

	// Argument errors
	fmpResult_Arguments_Invalid = -600,
	fmpResult_MediaSource_Invalid,
	fmpResult_MediaSource_Unsupported,
	fmpResult_MediaSource_FileOrPathNotFound,

	// System errors
	fmpResult_System_AllocationFailed = -500,

	// Context errors
	fmpResult_Context_AlreadyInitialized = -1,
	fmpResult_Context_NotInitialized = 0,

	fmpResult_Success = 1,
} fmpResult;

FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator);
FMP_API void fmpRelease(fmpContext *context);

FMP_API fmpResult fmpGetMediaInfo(const fmpContext *context, const fmpMediaSource *source, fmpMediaInfo *media);
FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media);

FMP_API fmpResult fmpLoadMedia(const fmpContext *context, const fmpMediaSource *source, const fmpMediaOptions *options, fmpMediaContext **outMedia);
FMP_API void fmpUnloadMedia(const fmpContext *context, fmpMediaContext **media);

FMP_API fmpResult fmpInitPacket(const fmpContext *context, fmpPacket *packet);
FMP_API void fmpReleasePacket(const fmpContext *context, fmpPacket *packet);
FMP_API fmpResult fmpReadPacket(const fmpContext *context, fmpMediaContext *media, fmpPacket *packet);
FMP_API void fmpClosePacket(const fmpContext *context, fmpPacket *packet);

FMP_API fmpResult fmpInitFrame(const fmpContext *context, fmpFrame *frame);
FMP_API void fmpReleaseFrame(const fmpContext *context, fmpFrame *frame);
FMP_API fmpResult fmpDecodeFrame(const fmpContext *context, fmpMediaContext *media, fmpFrame *frame, fmpPacket *packet);
FMP_API void fmpCloseFrame(const fmpContext *context, fmpFrame *frame);

#endif // FMP_API

#if (defined(FMP_IMPLEMENTATION) && !defined(FMP_IMPLEMENTED)) || defined(FPL_IS_IDE)
#define FMP_IMPLEMENTED

#define FFMPEG_IMPLEMENTATION
#include "ffmpeg_v2.h"

// No AV correction is done if too big error
#define __FMP_AV_NOSYNC_THRESHOLD 10.0

// ??
#define __FMP_EXTERNAL_CLOCK_MIN_FRAMES 2
#define __FMP_EXTERNAL_CLOCK_MAX_FRAMES 10

// external clock speed adjustment constants for realtime sources based on buffer fullness
#define __FMP_EXTERNAL_CLOCK_SPEED_MIN  0.900
#define __FMP_EXTERNAL_CLOCK_SPEED_MAX  1.010
#define __FMP_EXTERNAL_CLOCK_SPEED_STEP 0.001

//
// > Memory
//
typedef struct __fmpMemory {
	struct __fmpMemory *next;
	void *ptr;
	size_t size;
	size_t used;
} __fmpMemory;

static void *__fmpAllocateMemory(const fmpMemoryAllocator *allocator, const size_t size) {
	fplAssertPtr(allocator);
	fplAssert(size > 0);
	size_t totalSize = sizeof(__fmpMemory) + sizeof(uintptr_t) + size;
	void *base = allocator->alloc(totalSize, allocator->user);
	if (base == fpl_null) {
		return fpl_null;
	}
	__fmpMemory *mem = (__fmpMemory *)base;
	mem->next = fpl_null;
	mem->size = size;
	mem->used = size;
	mem->ptr = (uint8_t *)base + sizeof(__fmpMemory) + sizeof(uintptr_t);
	return mem->ptr;
}

static void *__fmpReallocateMemory(const fmpMemoryAllocator *allocator, void *ptr, const size_t newSize) {
	fplAssertPtr(allocator);
	fplAssert(newSize > 0);

	void *previousBase = fpl_null;
	size_t previousSize = 0;
	if (ptr != fpl_null) {
		previousBase = (uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(__fmpMemory));
		__fmpMemory *prevMem = (__fmpMemory *)previousBase;
		fplAssert(ptr == prevMem->ptr);
		previousSize = prevMem->used;
	}

	size_t totalSize = sizeof(__fmpMemory) + sizeof(uintptr_t) + newSize;
	void *newBase = allocator->realloc(previousBase, totalSize, allocator->user);
	if (newBase == fpl_null) {
		return fpl_null;
	}
	__fmpMemory *newMem = (__fmpMemory *)newBase;
	newMem->next = fpl_null;
	newMem->size = newSize;
	newMem->used = previousSize;
	newMem->ptr = (uint8_t *)newBase + sizeof(__fmpMemory) + sizeof(uintptr_t);

	return newMem->ptr;
}

static void __fmpFreeMemory(const fmpMemoryAllocator *allocator, void *ptr) {
	fplAssertPtr(allocator);
	fplAssertPtr(ptr);
	void *base = (uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(__fmpMemory));
	__fmpMemory *mem = (__fmpMemory *)base;
	fplAssert(ptr == mem->ptr);
	allocator->free(base, allocator->user);
}

//
// > Strings
//
static size_t __fmpCopyZeroTerminatedString(const char *source, const size_t sourceLen, char *target, const size_t maxTargetLen) {
	if (source == fpl_null || sourceLen == 0 || target == fpl_null || maxTargetLen == 0)
		return 0;
	if (maxTargetLen < sourceLen)
		return 0;
	size_t result = 0;
	while (result < sourceLen) {
		target[result] = source[result];
		++result;
	}
	if (result < maxTargetLen)
		target[result++] = '\0';
	return result;
}

static void __fmpFreeString(const fmpMemoryAllocator *allocator, fmpString *str) {
	fplAssertPtr(allocator);
	fplAssertPtr(str);
	if (str->data != fpl_null) {
		// TODO(final): This is very inefficient to always free a single string, use a block allocation scheme instead!
		__fmpFreeMemory(allocator, str->data);
	}
	fplClearStruct(str);
}

static fmpString __fmpCopyString(const fmpMemoryAllocator *allocator, const char *source, const size_t len) {
	fplAssertPtr(allocator);
	fmpString result = fplZeroInit;
	if (len > 0) {
		// TODO(final): This is very inefficient to always allocate a single string, use a block allocation scheme instead!
		size_t dataLen = len + 1;
		result.data = (char *)__fmpAllocateMemory(allocator, sizeof(char) * dataLen);
		result.len = len;
		size_t copied = __fmpCopyZeroTerminatedString(source, len, result.data, dataLen);
		fplAssert(copied == dataLen);
	}
	return result;
}

//
// Default allocator
//
static void *__fmpDefaultAllocate(const size_t size, void *user) {
	fplAssert(size > 0);
	void *result = fplMemoryAllocate(size);
	return result;
}

static void *__fmpDefaultRealloc(void *ptr, const size_t size, void *user) {
	fplAssert(size > 0);
	void *newBase = fplMemoryAllocate(size);
	if (newBase == fpl_null) {
		return fpl_null;
	}
	if (ptr != fpl_null) {
		__fmpMemory *mem = (__fmpMemory *)ptr;
		uint8_t *address = (uint8_t *)mem->ptr - (sizeof(uintptr_t) + sizeof(__fmpMemory));
		fplAssert(address == (uint8_t *)mem);
		size_t blockSize = mem->size + sizeof(uintptr_t) + sizeof(__fmpMemory);
		size_t copySize = fplMin(size, blockSize);
		fplMemoryCopy(ptr, copySize, newBase);
		fplMemoryFree(ptr);
	}
	return newBase;
}

static void __fmpDefaultFree(void *ptr, void *user) {
	fplAssertPtr(ptr);
	fplMemoryFree(ptr);
}

static fmpMemoryAllocator __fmpCreateDefaultAllocator() {
	fmpMemoryAllocator result = fplZeroInit;
	result.alloc = __fmpDefaultAllocate;
	result.realloc = __fmpDefaultRealloc;
	result.free = __fmpDefaultFree;
	return result;
}

//
// Utils
//
fpl_force_inline void __fmpSetSafeMediaState(fmpMediaContext *context, const fmpMediaState state) {
	fplAssertPtr(context);
	fplAtomicExchangeS32((volatile int32_t *)&context->state, (int32_t)state);
}

fpl_force_inline fmpMediaState __fmpGetSafeMediaState(fmpMediaContext *context) {
	fplAssertPtr(context);
	return (fmpMediaState)fplAtomicLoadS32((volatile int32_t *)&context->state);
}

//
// Clock
//
static double __fmpGetClock(const FFMPEGContext *ffmpeg, const fmpClock *c) {
	if (*c->queueSerial != c->serial) {
		return NAN;
	}
	if (c->paused) {
		return c->pts;
	} else {
		double time = ffmpeg->av_gettime_relative() / 1000000.0;
		return c->ptsDrift + time - (time - c->lastUpdated) * (1.0 - c->speed);
	}
}

static void __fmpSetClockTime(fmpClock *c, const double pts, const int serial, const double time) {
	c->pts = pts;
	c->lastUpdated = time;
	c->ptsDrift = c->pts - time;
	c->serial = serial;
}

static void __fmpSetClock(const FFMPEGContext *ffmpeg, fmpClock *c, double pts, int serial) {
	double time = ffmpeg->av_gettime_relative() / 1000000.0;
	__fmpSetClockTime(c, pts, serial, time);
}

static void __fmpSetClockSpeed(const FFMPEGContext *ffmpeg, fmpClock *c, double speed) {
	double pts = __fmpGetClock(ffmpeg, c);
	__fmpSetClock(ffmpeg, c, pts, c->serial);
	c->speed = speed;
}

static void __fmpInitClock(const FFMPEGContext *ffmpeg, fmpClock *c, int *queue_serial) {
	c->speed = 1.0;
	c->paused = 0;
	c->queueSerial = queue_serial;
	__fmpSetClock(ffmpeg, c, NAN, -1);
}

static void __fmpSyncClockToSlave(const FFMPEGContext *ffmpeg, fmpClock *c, fmpClock *slave) {
	double clock = __fmpGetClock(ffmpeg, c);
	double slave_clock = __fmpGetClock(ffmpeg, slave);
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > __FMP_AV_NOSYNC_THRESHOLD)) {
		__fmpSetClock(ffmpeg, c, slave_clock, slave->serial);
	}
}

static fmpClockSyncType __fmpGetMasterSyncType(const fmpMediaContext *media) {
	if (media->syncType == fmpClockSyncType_VideoMaster) {
		if (media->videoStream.base.isValid)
			return fmpClockSyncType_VideoMaster;
		else
			return fmpClockSyncType_AudioMaster;
	} else if (media->syncType == fmpClockSyncType_AudioMaster && media->audioStream.base.isValid) {
		return fmpClockSyncType_AudioMaster;
	}
	return fmpClockSyncType_ExternalClock;
}

/* get the current master clock value */
static double __fmpGetMasterClock(const FFMPEGContext *ffmpeg, fmpMediaContext *media) {
	double val;
	fmpClockSyncType syncType = __fmpGetMasterSyncType(media);
	switch (syncType) {
		case fmpClockSyncType_VideoMaster:
			val = __fmpGetClock(ffmpeg, &media->videoClock);
			break;
		case fmpClockSyncType_AudioMaster:
			val = __fmpGetClock(ffmpeg, &media->audioClock);
			break;
		default:
			val = __fmpGetClock(ffmpeg, &media->externalClock);
			break;
	}
	return val;
}

static void __fmpCheckExternalClockSpeed(const FFMPEGContext *ffmpeg, fmpMediaContext *media) {
	fmpStream *videoStream = &media->videoStream.base;
	fmpStream *audioStream = &media->audioStream.base;
	if (videoStream->isValid && videoStream->packetQueue.packetCount <= __FMP_EXTERNAL_CLOCK_MIN_FRAMES ||
		audioStream->isValid && audioStream->packetQueue.packetCount <= __FMP_EXTERNAL_CLOCK_MIN_FRAMES) {
		__fmpSetClockSpeed(ffmpeg, &media->externalClock, FFMAX(__FMP_EXTERNAL_CLOCK_SPEED_MIN, media->externalClock.speed - __FMP_EXTERNAL_CLOCK_SPEED_STEP));
	} else if ((!videoStream->isValid || videoStream->packetQueue.packetCount > __FMP_EXTERNAL_CLOCK_MAX_FRAMES) &&
			   (!audioStream->isValid || audioStream->packetQueue.packetCount > __FMP_EXTERNAL_CLOCK_MAX_FRAMES)) {
		__fmpSetClockSpeed(ffmpeg, &media->externalClock, FFMIN(__FMP_EXTERNAL_CLOCK_SPEED_MAX, media->externalClock.speed + __FMP_EXTERNAL_CLOCK_SPEED_STEP));
	} else {
		double speed = media->externalClock.speed;
		if (speed != 1.0) {
			__fmpSetClockSpeed(ffmpeg, &media->externalClock, speed + __FMP_EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
		}
	}
}

//
// > Packet-Queue
//
static fmpPacketQueueResult __fmpPacketQueuePushLocal(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue, AVPacket *pkt) {
	if (queue->abortRequest) 
		return fmpPacketQueueResult_Abort;

	fmpPacket entry = {};
	entry.pkt = pkt;
	entry.serial = queue->serial;

	int result = ffmpeg->av_fifo_write(queue->packetList, &entry, 1);
	if (result < 0) {
		return fmpPacketQueueResult_WriteFailed;
	}

	queue->packetCount++;
	queue->size += entry.pkt->size + sizeof(entry);
	queue->duration += entry.pkt->duration;

	fplConditionSignal(&queue->cond);

	return fmpPacketQueueResult_Success;
}

static fmpPacketQueueResult __fmpPacketQueuePush(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue, AVPacket *pkt) {
	fplAssertPtr(ffmpeg); 
	fplAssertPtr(queue); 
	fplAssertPtr(pkt);

	AVPacket *newPacket = ffmpeg->av_packet_alloc();
	if (newPacket == fpl_null) {
		ffmpeg->av_packet_unref(pkt);
		return fmpPacketQueueResult_AllocationFailed;
	}
	ffmpeg->av_packet_move_ref(newPacket, pkt);

	fmpPacketQueueResult result;
	fplMutexLock(&queue->mutex);
	result = __fmpPacketQueuePushLocal(ffmpeg, queue, newPacket);
	fplMutexUnlock(&queue->mutex);

	if (result < 0) {
		ffmpeg->av_packet_free(&newPacket);
	}

	return result;
}

static fmpPacketQueueResult __fmpPacketQueuePushNullPacket(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue, AVPacket *pkt, int streamIndex) {
	fplAssertPtr(ffmpeg); 
	fplAssertPtr(queue); 
	fplAssertPtr(pkt);

	pkt->stream_index = streamIndex;
	fmpPacketQueueResult result = __fmpPacketQueuePush(ffmpeg, queue, pkt);
	return result;
}

static fmpPacketQueueResult __fmpPacketQueuePop(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue, AVPacket *pkt, int block, int *serial) {
	fplAssertPtr(ffmpeg); 
	fplAssertPtr(queue); 
	fplAssertPtr(pkt);

	fmpPacketQueueResult result;
	fplMutexLock(&queue->mutex);
	fmpPacket entry;
	for (;;) {
		if (queue->abortRequest) {
			result = fmpPacketQueueResult_Abort;
			break;
		}

		if (ffmpeg->av_fifo_read(queue->packetList, &entry, 1) >= 0) {
			queue->packetCount--;
			queue->size -= entry.pkt->size + sizeof(entry);
			queue->duration -= entry.pkt->duration;
			ffmpeg->av_packet_move_ref(pkt, entry.pkt);
			if (serial != fpl_null) {
				*serial = entry.serial;
			}
			ffmpeg->av_packet_free(&entry.pkt);
			result = fmpPacketQueueResult_Success;
			break;
		} else if (!block) {
			result = fmpPacketQueueResult_Full;
			break;
		} else {
			fplConditionWait(&queue->cond, &queue->mutex, FPL_TIMEOUT_INFINITE);
		}
	}
	fplMutexUnlock(&queue->mutex);
	return result;
}

static void __fmpPacketQueueFlush(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue) {
	fplAssertPtr(ffmpeg); 
	fplAssertPtr(queue);

	fmpPacket entry;
	fplMutexLock(&queue->mutex);
	while (ffmpeg->av_fifo_read(queue->packetList, &entry, 1) >= 0) {
		ffmpeg->av_packet_free(&entry.pkt);
	}
	queue->packetCount = 0;
	queue->size = 0;
	queue->duration = 0;
	queue->serial++;
	fplMutexUnlock(&queue->mutex);
}

static void __fmpPacketQueueDestroy(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue) {
	fplAssertPtr(ffmpeg); 
	fplAssertPtr(queue);

	if (queue->isValid) {
		__fmpPacketQueueFlush(ffmpeg, queue);
	}
	if (queue->cond.isValid) {
		fplConditionDestroy(&queue->cond);
	}
	if (queue->mutex.isValid) {
		fplMutexDestroy(&queue->mutex);
	}
	if (queue->packetList != fpl_null) {
		ffmpeg->av_fifo_freep2(&queue->packetList);
	}
	fplClearStruct(queue);
}

static fmpResult __fmpPacketQueueInit(const FFMPEGContext *ffmpeg, fmpPacketQueue *queue) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(queue);

	fmpResult errorRes = fmpResult_UnknownError;

	fplClearStruct(queue);
	queue->packetList = ffmpeg->av_fifo_alloc2(1, sizeof(fmpPacket), AV_FIFO_FLAG_AUTO_GROW);
	if (queue->packetList == fpl_null) {
		errorRes = fmpResult_Packet_AllocationFailed;
		goto failed;
	}
	if (!fplMutexInit(&queue->mutex)) {
		errorRes = fmpResult_Mutex_FailedInitialization;
		goto failed;
	}
	if (!fplConditionInit(&queue->cond)) {
		errorRes = fmpResult_ConditionVariable_FailedInitialization;
		goto failed;
	}

	queue->isValid = true;

	return fmpResult_Success;
failed:
	__fmpPacketQueueDestroy(ffmpeg, queue);
	fplAssert(errorRes != fmpResult_UnknownError);
	return errorRes;
}

static void __fmpPacketQueueAbort(fmpPacketQueue *queue) {
	fplAssertPtr(queue);
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 1;
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static void __fmpPacketQueueStart(fmpPacketQueue *queue) {
	fplAssertPtr(queue);
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 0;
	queue->serial++;
	fplMutexUnlock(&queue->mutex);
}

//
// > Frame-Queue
//
static void __fmpFrameQueueUnref(const FFMPEGContext *ffmpeg, fmpFrame *frame) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(frame);
	ffmpeg->av_frame_unref(frame->nativeFrame);
	ffmpeg->avsubtitle_free(&frame->nativeSubtitle);
}

static void __fmpFrameQueueDestroy(const FFMPEGContext *ffmpeg, fmpFrameQueue *queue) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(queue);
	if (queue->isValid) {
		for (int32_t i = 0; i < queue->maxSize; ++i) {
			fmpFrame *frame = queue->queue + i;
			if (frame->nativeFrame != fpl_null) {
				__fmpFrameQueueUnref(ffmpeg, frame);
				ffmpeg->av_frame_free(&frame->nativeFrame);
			}
		}
	}
	fplConditionDestroy(&queue->cond);
	fplMutexDestroy(&queue->mutex);
	fplClearStruct(queue);
}

static fmpResult __fmpFrameQueueInit(const FFMPEGContext *ffmpeg, fmpFrameQueue *frameQueue, fmpPacketQueue *packetQueue, const int32_t maxSize, const int32_t keepLast) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(frameQueue);
	fplAssertPtr(packetQueue);

	if (maxSize <= 0)
		return fmpResult_Arguments_Invalid;

	fplClearStruct(frameQueue);

	fmpResult errorRes = fmpResult_UnknownError;

	if (!fplMutexInit(&frameQueue->mutex)) {
		errorRes = fmpResult_Mutex_FailedInitialization;
		goto failed;
	}
	if (!fplConditionInit(&frameQueue->cond)) {
		errorRes = fmpResult_ConditionVariable_FailedInitialization;
		goto failed;
	}

	frameQueue->maxSize = fplMin(maxSize, FMP_MAX_FRAME_QUEUE_COUNT);
	frameQueue->keepLast = !!keepLast;
	for (int32_t i = 0; i < frameQueue->maxSize; ++i) {
		AVFrame *frame = ffmpeg->av_frame_alloc();
		if (frame == fpl_null) {
			errorRes = fmpResult_Frame_AllocationFailed;
			goto failed;
		}
		frameQueue->queue[i].nativeFrame = frame;
	}

	frameQueue->packetQueue = packetQueue;
	frameQueue->isValid = true;

	return fmpResult_Success;
failed:
	__fmpFrameQueueDestroy(ffmpeg, frameQueue);
	fplAssert(errorRes != fmpResult_UnknownError);
	return errorRes;
}

static void __fmpFrameQueueSignal(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static fmpFrame *__fmpFrameQueuePeek(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown) % queue->maxSize;
	return &queue->queue[index];
}

static fmpFrame *__fmpFrameQueuePeekNext(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown + 1) % queue->maxSize;
	return &queue->queue[index];
}

static fmpFrame *__fmpFrameQueuePeekLast(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = queue->readIndex;
	return &queue->queue[index];
}

static fmpFrame *__fmpFrameQueuePeekWritable(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;

	fplMutexLock(&queue->mutex);
	while (queue->size >= queue->maxSize && !queue->packetQueue->abortRequest) {
		fplConditionWait(&queue->cond, &queue->mutex, FPL_TIMEOUT_INFINITE);
	}
	fplMutexUnlock(&queue->mutex);

	if (queue->packetQueue->abortRequest) {
		return fpl_null;
	}

	return &queue->queue[queue->writeIndex];
}

static fmpFrame *__fmpFrameQueuePeekReadable(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;

	fplMutexLock(&queue->mutex);
	while (queue->size - queue->readIndexShown <= 0 && !queue->packetQueue->abortRequest) {
		fplConditionWait(&queue->cond, &queue->mutex, FPL_TIMEOUT_INFINITE);
	}
	fplMutexUnlock(&queue->mutex);

	if (queue->packetQueue->abortRequest) {
		return fpl_null;
	}

	uint32_t index = (queue->readIndex + queue->readIndexShown) % queue->maxSize;

	return &queue->queue[index];
}

static void __fmpFrameQueuePush(const FFMPEGContext *ffmpeg, fmpFrameQueue *queue) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(queue);

	if (queue->keepLast && !queue->readIndexShown) {
		queue->readIndexShown = 1;
		return;
	}

	__fmpFrameQueueUnref(ffmpeg, &queue->queue[queue->readIndex]);
	if (++queue->readIndex == queue->maxSize) {
		queue->readIndex = 0;
	}

	fplMutexLock(&queue->mutex);
	queue->size--;
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static int32_t __fmpGetFrameQueueRemainingCount(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return 0;
	int32_t result = queue->size - queue->readIndexShown;
	return result;
}

static int64_t __fmpGetFrameQueueLastPos(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return -1;
	fmpFrame *frame = &queue->queue[queue->readIndex];
	if (queue->readIndexShown && frame->serial == queue->packetQueue->serial)
		return frame->pos;
	else
		return -1;
}

//
// > Decoder
//
static void __fmpDecoderDestroy(const FFMPEGContext *ffmpeg, fmpDecoder *decoder) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(decoder);

	if (decoder->pendingPacket != fpl_null) {
		ffmpeg->av_packet_free(&decoder->pendingPacket);
	}

	fplClearStruct(&decoder);
}

static fmpResult __fmpDecoderInit(const FFMPEGContext *ffmpeg, fmpDecoder *decoder, AVCodecContext *codecCtx, fmpPacketQueue *packetQueue, fplConditionVariable *emptyQueueCondition) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(decoder);
	fplAssertPtr(codecCtx);
	fplAssertPtr(packetQueue);
	fplAssertPtr(emptyQueueCondition);

	fmpResult errorResult = fmpResult_UnknownError;

	fplClearStruct(decoder);
	decoder->ffmpeg = ffmpeg;

	decoder->pendingPacket = ffmpeg->av_packet_alloc();
	if (decoder->pendingPacket == fpl_null) {
		errorResult = fmpResult_Packet_AllocationFailed;
		goto failed;
	}

	decoder->codecCtx = codecCtx;
	decoder->packetQueue = packetQueue;
	decoder->emptyQueueCondition = emptyQueueCondition;
	decoder->startPts = AV_NOPTS_VALUE;
	decoder->packetSerial = -1;
	decoder->reorderPts = -1;
	decoder->isValid = true;
	return fmpResult_Success;

failed:
	__fmpDecoderDestroy(ffmpeg, decoder);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

static void __fmpDecoderAbort(const FFMPEGContext *ffmpeg, fmpDecoder *decoder, fmpFrameQueue *frameQueue) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(decoder);
	fplAssertPtr(frameQueue);
	__fmpPacketQueueAbort(decoder->packetQueue);
	__fmpFrameQueueSignal(frameQueue);
	fplThreadWaitForOne(decoder->thread, FPL_TIMEOUT_INFINITE);
	decoder->thread = fpl_null;
	__fmpPacketQueueFlush(ffmpeg, decoder->packetQueue);
}

static int __fmpDecoderDecodeFrame(const FFMPEGContext *ffmpeg, fmpDecoder *decoder, AVFrame *frame, AVSubtitle *subtitle) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(decoder);
	int ret = AVERROR(EAGAIN);
	AVCodecContext *codecCtx = decoder->codecCtx;
	fmpPacketQueue *queue = decoder->packetQueue;
	AVPacket *packet = decoder->pendingPacket;
	for (;;) {
		if (queue->serial == decoder->packetSerial) {
			do {
				if (queue->abortRequest) {
					return -1;
				}
				switch (codecCtx->codec_type) {
					case AVMEDIA_TYPE_VIDEO:
						ret = ffmpeg->avcodec_receive_frame(codecCtx, frame);
						if (ret >= 0) {
							if (decoder->reorderPts == -1) {
								frame->pts = frame->best_effort_timestamp;
							} else if (!decoder->reorderPts) {
								frame->pts = frame->pkt_dts;
							}
						}
						break;
					case AVMEDIA_TYPE_AUDIO:
						ret = ffmpeg->avcodec_receive_frame(codecCtx, frame);
						if (ret >= 0) {
							AVRational tb = fplStructInit(AVRational, 1, frame->sample_rate);
							if (frame->pts != AV_NOPTS_VALUE)
								frame->pts = ffmpeg->av_rescale_q(frame->pts, codecCtx->pkt_timebase, tb);
							else if (decoder->nextPts != AV_NOPTS_VALUE)
								frame->pts = ffmpeg->av_rescale_q(decoder->nextPts, decoder->nextPtsTimeBase, tb);
							if (frame->pts != AV_NOPTS_VALUE) {
								decoder->nextPts = frame->pts + frame->nb_samples;
								decoder->nextPtsTimeBase = tb;
							}
						}
						break;
				}
				if (ret == AVERROR_EOF) {
					decoder->finishedSerial = decoder->packetSerial;
					ffmpeg->avcodec_flush_buffers(codecCtx);
					return 0;
				}
				if (ret >= 0) {
					return 1;
				}
			} while (ret != AVERROR(EAGAIN));
		}

		do {
			if (queue->packetCount == 0) {
				fplConditionSignal(decoder->emptyQueueCondition);
			}
			if (decoder->isPacketPending) {
				decoder->isPacketPending = 0;
			} else {
				int32_t old_serial = decoder->packetSerial;
				if (__fmpPacketQueuePop(ffmpeg, queue, packet, 1, &decoder->packetSerial) < 0) {
					return -1;
				}
				if (old_serial != decoder->packetSerial) {
					ffmpeg->avcodec_flush_buffers(codecCtx);
					decoder->finishedSerial = 0;
					decoder->nextPts = decoder->startPts;
					decoder->nextPtsTimeBase = decoder->startPtsTimeBase;
				}
			}
			if (queue->serial == decoder->packetSerial) {
				break;
			}
			ffmpeg->av_packet_unref(packet);
		} while (1);

		if (codecCtx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
			int gotFrame = 0;
			ret = ffmpeg->avcodec_decode_subtitle2(codecCtx, subtitle, &gotFrame, packet);
			if (ret < 0) {
				ret = AVERROR(EAGAIN);
			} else {
				if (gotFrame && !packet->data) {
					decoder->isPacketPending = 1;
				}
				ret = gotFrame ? 0 : (packet->data ? AVERROR(EAGAIN) : AVERROR_EOF);
			}
			ffmpeg->av_packet_unref(packet);
		} else {
			if (ffmpeg->avcodec_send_packet(codecCtx, packet) == AVERROR(EAGAIN)) {
				decoder->isPacketPending = 1;
			} else {
				ffmpeg->av_packet_unref(packet);
			}
		}
	}
}

//
// Media
//
static void __fmpFreeLanguageInfo(const fmpMemoryAllocator *allocator, fmpLanguageInfo *info) {
	__fmpFreeString(allocator, &info->name);
}

static fmpLanguageInfo __fmpGetLanguageInfo(const fmpMemoryAllocator *allocator, const FFMPEGContext *ffmpeg, AVDictionary *dict) {
	fmpLanguageInfo result = fplZeroInit;

	AVDictionaryEntry *lang = ffmpeg->av_dict_get(dict, "language", fpl_null, 0);
	size_t valueLen;
	if (lang != fpl_null && (valueLen = fplGetStringLength(lang->value)) > 0) {
		__fmpCopyZeroTerminatedString(lang->value, valueLen, result.iso639_2, fplArrayCount(result.iso639_2));
	}

	const char *nameTags[] = { "title", "description", "handler" };
	for (int i = 0; i < fplArrayCount(nameTags); ++i) {
		AVDictionaryEntry *entry = ffmpeg->av_dict_get(dict, nameTags[i], fpl_null, 0);
		size_t len;
		if (entry != fpl_null && (len = fplGetStringLength(entry->value)) > 0) {
			result.name = __fmpCopyString(allocator, entry->value, len);
			break;
		}
	}

	if (result.name.len == 0) {
		// @TODO(final): Translate ISO639-2 code to a language name
	}

	return result;
}

static void __fmpFreeCodecInfo(const fmpMemoryAllocator *allocator, fmpCodecInfo *info) {
	fplAssertPtr(allocator);
	fplAssertPtr(info);

	__fmpFreeString(allocator, &info->name);
	fplClearStruct(info);
}

static fmpCodecInfo __fmpGetCodecInfo(const fmpMemoryAllocator *allocator, const FFMPEGContext *ffmpeg, const AVCodecParameters *params) {
	fplAssertPtr(allocator);
	fplAssertPtr(ffmpeg);
	fplAssertPtr(params);

	fmpCodecInfo result = fplZeroInit;

	AVCodecID codecID = params->codec_id;
	uint32_t codecTag = params->codec_tag;

	const char *name = ffmpeg->avcodec_get_name(codecID);
	size_t len;
	if ((len = fplGetStringLength(name)) > 0) {
		result.name = __fmpCopyString(allocator, name, len);
	}

	if (fplIsLittleEndian()) {
		result.fourcc[0] = (char)((codecTag >> 0) & 0xFF);
		result.fourcc[1] = (char)((codecTag >> 8) & 0xFF);
		result.fourcc[2] = (char)((codecTag >> 16) & 0xFF);
		result.fourcc[3] = (char)((codecTag >> 24) & 0xFF);
	} else {
		result.fourcc[0] = (char)((codecTag >> 24) & 0xFF);
		result.fourcc[1] = (char)((codecTag >> 16) & 0xFF);
		result.fourcc[2] = (char)((codecTag >> 8) & 0xFF);
		result.fourcc[3] = (char)((codecTag >> 0) & 0xFF);
	}

	return result;
}

fpl_force_inline bool __fmpIsValidRational(const AVRational rational) {
	return rational.den != 0;
}

static void __fmpFillMediaInfo(const fmpMemoryAllocator *allocator, const FFMPEGContext *ffmpeg, const fmpMediaSource *source, AVFormatContext *formatCtx, fmpMediaInfo *media) {
	fplAssertPtr(allocator);
	fplAssertPtr(ffmpeg);
	fplAssertPtr(source);
	fplAssertPtr(formatCtx);
	fplAssertPtr(media);

	fplClearStruct(media);

	media->source = *source;

	AVDictionaryEntry *title = ffmpeg->av_dict_get(formatCtx->metadata, "title", fpl_null, 0);
	size_t titleLen;
	if (title != fpl_null && (titleLen = fplGetStringLength(title->value)) > 0) {
		media->title = __fmpCopyString(allocator, title->value, titleLen);
	}

	media->streamCount = formatCtx->nb_streams;

	if (formatCtx->duration > 0) {
		media->duration.value = formatCtx->duration / (double)AV_TIME_BASE;
	} else {
		media->duration.value = 0;
	}

	for (uint32_t streamIndex = 0; streamIndex < media->streamCount; ++streamIndex) {
		AVStream *st = formatCtx->streams[streamIndex];
		enum AVMediaType codecType = st->codecpar->codec_type;

		fplAssert(streamIndex == (uint32_t)st->index);

		fmpStreamInfo *info = media->streams + streamIndex;
		info->type = fmpStreamType_Unknown;
		info->index = fplStructInit(fmpStreamIndex, streamIndex);

#if 0
		if (st->metadata != fpl_null) {
			char *buffer = fpl_null;
			if (ffmpeg->av_dict_get_string(st->metadata, &buffer, '|', '\n') == 0) {
				fplDebugFormatOut("Stream[%lu]:\n%s\n", streamIndex, buffer);
				ffmpeg->av_freep(&buffer);
			}
		}
#endif

		info->language = __fmpGetLanguageInfo(allocator, ffmpeg, st->metadata);

		info->codec = __fmpGetCodecInfo(allocator, ffmpeg, st->codecpar);

		switch (codecType) {
			case AVMEDIA_TYPE_VIDEO:
			{
				info->type = fmpStreamType_Video;

				info->video.frameRate = st->r_frame_rate;
				if (!__fmpIsValidRational(info->video.frameRate)) {
					info->video.frameRate = ffmpeg->av_guess_frame_rate(formatCtx, st, fpl_null);
				}

				info->video.sampleAspectRatio = st->codecpar->sample_aspect_ratio;
				if (!__fmpIsValidRational(info->video.sampleAspectRatio)) {
					info->video.sampleAspectRatio = ffmpeg->av_guess_sample_aspect_ratio(formatCtx, st, fpl_null);
				}

				if (__fmpIsValidRational(info->video.sampleAspectRatio)) {
					ffmpeg->av_reduce(
						&info->video.displayAspectRatio.num,
						&info->video.displayAspectRatio.den,
						st->codecpar->width * (int64_t)info->video.sampleAspectRatio.num,
						st->codecpar->height * (int64_t)info->video.sampleAspectRatio.den,
						1024ULL * 1024ULL);
				}
				info->video.width = st->codecpar->width;
				info->video.height = st->codecpar->height;
				info->video.pixelFormat = (enum AVPixelFormat)st->codecpar->format;
				info->video.bitrate = st->codecpar->bit_rate;
			} break;

			case AVMEDIA_TYPE_AUDIO:
			{
				info->type = fmpStreamType_Audio;
				info->audio.channels = (uint32_t)st->codecpar->ch_layout.nb_channels;
				info->audio.sampleRate = (uint32_t)st->codecpar->sample_rate;
				info->audio.sampleFormat = (enum AVSampleFormat)st->codecpar->format;
				info->audio.bitsPerSample = (uint32_t)st->codecpar->bits_per_coded_sample;
				info->audio.bitrate = st->codecpar->bit_rate;
			} break;

			case AVMEDIA_TYPE_SUBTITLE:
			{
				info->type = fmpStreamType_Subtitle;
			} break;

			default:
				break;
		}
	}
}

static bool __fmpIsRealTime(const AVFormatContext *s) {
	if (!strcmp(s->iformat->name, "rtp") ||
		!strcmp(s->iformat->name, "rtsp") ||
		!strcmp(s->iformat->name, "sdp")) {
		return true;
	}
	if (s->pb && (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4))) {
		return true;
	}
	return false;
}

static void __fmpDestroyAudioStream(fmpMediaContext *media, fmpAudioStream *stream) {
	fplAssertPtr(media);
	fplAssertPtr(stream);
	fplClearStruct(stream);
}

static fmpResult __fmpOpenAudioStream(const fmpContext *context, fmpMediaContext *media, fmpAudioStream *stream, AVCodecContext *codecCtx) {
	fplAssertPtr(context);
	fplAssertPtr(media);
	fplAssertPtr(stream);
	fplAssertPtr(codecCtx);

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	AVChannelLayout channelLayout = fplZeroInit;

	fmpResult result = fmpResult_Success;

	int32_t sampleRate = codecCtx->sample_rate;

	int ret = ffmpeg->av_channel_layout_copy(&channelLayout, &codecCtx->ch_layout);
	if (ret < 0) {
		result = fmpResult_Backend_InvalidStream;
		goto failed;
	}

	goto out;

failed:
	fplAssert(result != fmpResult_Success);
	__fmpDestroyAudioStream(media, stream);
out:
	ffmpeg->av_channel_layout_uninit(&channelLayout);
	return result;
}

static void __fmpCloseStream(const FFMPEGContext *ffmpeg, fmpMediaContext *media, fmpStream *targetStream) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(media);
	fplAssertPtr(targetStream);

	if (targetStream->decoder.isValid) {
		__fmpDecoderAbort(ffmpeg, &targetStream->decoder, &targetStream->frameQueue);
	}
	__fmpDecoderDestroy(ffmpeg, &targetStream->decoder);

	__fmpFrameQueueDestroy(ffmpeg, &targetStream->frameQueue);

	__fmpPacketQueueDestroy(ffmpeg, &targetStream->packetQueue);

	if (targetStream->codecContext != fpl_null) {
		ffmpeg->avcodec_free_context(&targetStream->codecContext);
	}

	if (targetStream->stream != fpl_null) {
		targetStream->stream->discard = AVDISCARD_ALL;
	}

	switch (targetStream->type) {
		case fmpStreamType_Video: {
			fmpVideoStream *videoStream = (fmpVideoStream *)targetStream;
			fplClearStruct(videoStream);
		} break;
		case fmpStreamType_Audio: {
			fmpAudioStream *audioStream = (fmpAudioStream *)targetStream;
			__fmpDestroyAudioStream(media, audioStream);
		} break;
		case fmpStreamType_Subtitle: {
			fmpSubtitleStream *subtitleStream = (fmpSubtitleStream *)targetStream;
			fplClearStruct(subtitleStream);
		} break;
		default:
			fplClearStruct(targetStream);
			break;
	}
}

static fmpResult __fmpOpenStream(const fmpContext *context, fmpMediaContext *media, const fmpStreamIndex streamIndex, const char *codecName, fmpStream *targetStream) {
	fplAssertPtr(context);
	fplAssertPtr(media);
	fplAssertPtr(targetStream);

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	AVFormatContext *formatCtx = media->formatCtx;
	fplAssertPtr(formatCtx);

	if ((streamIndex.index == UINT32_MAX) || (streamIndex.index >= (int32_t)formatCtx->nb_streams)) {
		return fmpResult_Backend_InvalidStream;
	}

	AVStream *st = formatCtx->streams[streamIndex.index];

	AVCodecContext *codecCtx = fpl_null;
	const AVCodec *codec = fpl_null;
	uint32_t sampleRate = 0;

	fplClearStruct(targetStream);
	targetStream->stream = st;
	targetStream->index = streamIndex;

	fmpResult result = fmpResult_Success;

	int ret = AVERROR(EAGAIN);

	if (fplGetStringLength(codecName) > 0) {
		codec = ffmpeg->avcodec_find_decoder_by_name(codecName);
	} else {
		codec = ffmpeg->avcodec_find_decoder(st->codecpar->codec_id);
	}
	if (codec == fpl_null) {
		result = fmpResult_Backend_NoDecoderFound;
		goto failed;
	}

	codecCtx = ffmpeg->avcodec_alloc_context3(codec);
	if (codecCtx == fpl_null) {
		result = fmpResult_Backend_AllocationFailed;
		goto failed;
	}

	if ((ret = ffmpeg->avcodec_parameters_to_context(codecCtx, st->codecpar)) < 0) {
		result = fmpResult_Backend_InvalidStream; // @TODO(final): Better error code!
		goto failed;
	}

	if ((ret = ffmpeg->avcodec_open2(codecCtx, codec, fpl_null)) < 0) {
		result = fmpResult_Backend_InvalidStream; // @TODO(final): Better error code!
		goto failed;
	}

	codecCtx->pkt_timebase = st->time_base;

	codecCtx->codec_id = codec->id;

	st->discard = AVDISCARD_DEFAULT;

	int32_t queueCapcity = 0;
	int32_t keepLast = 0;
	switch (codecCtx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
		{
			fmpAudioStream *audioStream = (fmpAudioStream *)targetStream;
			fmpResult audioResult = __fmpOpenAudioStream(context, media, audioStream, codecCtx);
			if (audioResult != fmpResult_Success) {
				result = audioResult;
				goto failed;
			}
			queueCapcity = FMP_MAX_AUDIO_FRAME_QUEUE_COUNT;
		} break;

		case AVMEDIA_TYPE_VIDEO:
		{
			queueCapcity = FMP_MAX_VIDEO_FRAME_QUEUE_COUNT;
		} break;

		case AVMEDIA_TYPE_SUBTITLE:
		{
			queueCapcity = FMP_MAX_SUBTITLE_FRAME_QUEUE_COUNT;
		} break;
	}

	fmpResult frameQueueRes;
	if ((frameQueueRes = __fmpFrameQueueInit(ffmpeg, &targetStream->frameQueue, &targetStream->packetQueue, queueCapcity, keepLast)) != fmpResult_Success) {
		result = frameQueueRes;
		goto failed;
	}

	fmpResult packetQueueRes;
	if ((packetQueueRes = __fmpPacketQueueInit(ffmpeg, &targetStream->packetQueue)) != fmpResult_Success) {
		result = packetQueueRes;
		goto failed;
	}

	fmpResult decoderResult;
	if ((decoderResult = __fmpDecoderInit(ffmpeg, &targetStream->decoder, codecCtx, &targetStream->packetQueue, &media->reader.continueCondition)) != fmpResult_Success) {
		result = decoderResult;
		goto failed;
	}

	result = fmpResult_Success;
	goto out;

failed:
	if (codecCtx != fpl_null) {
		ffmpeg->avcodec_free_context(&codecCtx);
	}
	fplClearStruct(targetStream);
out:
	return result;
}

static void __fmpCloseInput(const FFMPEGContext *ffmpeg, AVFormatContext **formatCtx) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(formatCtx);
	if ((*formatCtx)->iformat != fpl_null) {
ffmpeg->avformat_close_input(formatCtx);
	}
	if (*formatCtx != fpl_null)
		ffmpeg->avformat_free_context(*formatCtx);
}

static fmpResult __fmpOpenInputFromSource(const FFMPEGContext *ffmpeg, AVFormatContext **inOutCtx, const fmpMediaSource *source) {
	// NOTE(final): Do not free any memory here, its just a helper function for loading the source into the AVFormatContext

	fplAssertPtr(ffmpeg);
	fplAssertPtr(inOutCtx);
	fplAssertPtr(source);

	const char *pathOrURL = fpl_null;
	switch (source->type) {
		case fmpMediaSourceType_File:
			pathOrURL = source->file.filePath;
			break;
		case fmpMediaSourceType_URL:
			pathOrURL = source->url.url;
			break;
		default:
			return fmpResult_MediaSource_Unsupported;
	}

	int openRes = ffmpeg->avformat_open_input(inOutCtx, pathOrURL, fpl_null, fpl_null);
	if (openRes < 0) {
		return fmpResult_Backend_NotSupported;
	}

	AVFormatContext *thisFormatCtx = *inOutCtx;

	fplAssertPtr(thisFormatCtx);

	int streamInfoRes = ffmpeg->avformat_find_stream_info(thisFormatCtx, fpl_null);
	if (streamInfoRes < 0) {
		return fmpResult_Backend_NoStreamsFound;
	}

	if (thisFormatCtx->nb_streams > FMP_MAX_STREAM_COUNT) {
		return fmpResult_Backend_TooManyStreams;
	}

	ffmpeg->av_format_inject_global_side_data(thisFormatCtx);

	return fmpResult_Success;
}

static void __fmpReleasePacketReader(fmpMediaContext *media, fmpPacketReader *reader) {
	fplAssertPtr(media);
	fplAssertPtr(reader);
	if (reader->continueCondition.isValid) {
		fplConditionDestroy(&reader->continueCondition);
	}
	fplClearStruct(reader);
}

static fmpResult __fmpInitPacketReader(fmpMediaContext *media, fmpPacketReader *reader) {
	fplAssertPtr(media);
	fplAssertPtr(reader);

	fmpResult errorResult = fmpResult_UnknownError;

	if (!fplConditionInit(&reader->continueCondition)) {
		errorResult = fmpResult_ConditionVariable_FailedInitialization;
		goto failed;
	}

	return fmpResult_Success;

failed:
	__fmpReleasePacketReader(media, reader);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

static void __fmpUnloadMediaContext(const FFMPEGContext *ffmpeg, fmpMediaContext *media) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(media);
	
	__fmpCloseStream(ffmpeg, media, &media->videoStream.base);
	__fmpCloseStream(ffmpeg, media, &media->audioStream.base);
	__fmpCloseStream(ffmpeg, media, &media->subtitleStream.base);

	if (media->formatCtx != fpl_null) {
		__fmpCloseInput(ffmpeg, &media->formatCtx);
	}

	__fmpReleasePacketReader(media, &media->reader);

	fplClearStruct(media);
}

static fmpResult __fmpLoadMediaIntoContext(const fmpContext *context, const fmpMediaSource *source, const fmpMediaOptions *options, fmpMediaContext *media) {
	fplAssertPtr(context);
	fplAssertPtr(source);
	fplAssertPtr(options);
	fplAssertPtr(media);

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	fmpResult errorResult = fmpResult_UnknownError;

	fplClearStruct(media);
	media->source = *source;
	if (options != fpl_null) {
		media->options = *options;
	}

	//
	// Init Reader
	//
	fmpResult readerRes = __fmpInitPacketReader(media, &media->reader);
	if (readerRes != fmpResult_Success) {
		errorResult = readerRes;
		goto failed;
	}

	//
	// Load path or url into a AVFormatContext
	//
	media->formatCtx = ffmpeg->avformat_alloc_context();
	if (media->formatCtx == fpl_null) {
		errorResult = fmpResult_Backend_AllocationFailed;
		goto failed;
	}

	const AVInputFormat *inputFormat = fpl_null;

	fmpResult openRes = __fmpOpenInputFromSource(ffmpeg, &media->formatCtx, source);
	if (openRes != fmpResult_Success) {
		errorResult = openRes;
		goto failed;
	}

	//
	// Fill media info
	//
	AVFormatContext *ic = media->formatCtx;
	__fmpFillMediaInfo(allocator, ffmpeg, source, ic, &media->info);

	//
	// Defaults
	//
	media->maxFrameDuration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	media->isRealTime = __fmpIsRealTime(ic);

	//
	// Initialize streams and find the best ones
	//
	const char *streamSpecs[AVMEDIA_TYPE_NB] = fplZeroInit; // TODO(final): User option!

	int32_t streamIndices[AVMEDIA_TYPE_NB]; // TODO(final): User option!
	memset(streamIndices, -1, sizeof(streamIndices));

	for (uint32_t streamIndex = 0; streamIndex < ic->nb_streams; ++streamIndex) {
		AVStream *st = ic->streams[streamIndex];

		st->discard = AVDISCARD_ALL; // By default we discard all streams, this flag will be removed when the stream is activated

		enum AVMediaType type = st->codecpar->codec_type;
		const char *streamSpec = streamSpecs[type];
		if (type >= 0 && fplGetStringLength(streamSpec) > 0 && streamIndices[type] == -1) {
			if (ffmpeg->avformat_match_stream_specifier(ic, st, streamSpec) > 0) {
				streamIndices[type] = streamIndex;
			}
		}
	}

	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	int subtitleStreamIndex = -1;
	if (!media->options.isVideoDisabled) {
		videoStreamIndex = ffmpeg->av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, streamIndices[AVMEDIA_TYPE_VIDEO], -1, fpl_null, 0);
	}
	if (!media->options.isAudioDisabled) {
		audioStreamIndex = ffmpeg->av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, streamIndices[AVMEDIA_TYPE_AUDIO], -1, fpl_null, 0);
	}
	if (!media->options.isVideoDisabled && !media->options.isSubtitleDisabled) {
		int relatedStream = audioStreamIndex >= 0 ? audioStreamIndex : videoStreamIndex;
		subtitleStreamIndex = ffmpeg->av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, streamIndices[AVMEDIA_TYPE_SUBTITLE], relatedStream, fpl_null, 0);
	}

	if (videoStreamIndex < 0 && audioStreamIndex < 0) {
		errorResult = fmpResult_Backend_NoStreamsFound;
		goto failed;
	}

	if (videoStreamIndex >= 0) {
		fmpStreamIndex vsi = fplStructInit(fmpStreamIndex, (uint32_t)videoStreamIndex);
		fmpResult streamRes = __fmpOpenStream(context, media, vsi, fpl_null, &media->videoStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}
	if (audioStreamIndex >= 0) {
		fmpStreamIndex asi = fplStructInit(fmpStreamIndex, (uint32_t)audioStreamIndex);
		fmpResult streamRes = __fmpOpenStream(context, media, asi, fpl_null, &media->audioStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}
	if (subtitleStreamIndex >= 0) {
		fmpStreamIndex ssi = fplStructInit(fmpStreamIndex, (uint32_t)audioStreamIndex);
		fmpResult streamRes = __fmpOpenStream(context, media, ssi, fpl_null, &media->subtitleStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}

	return fmpResult_Success;

failed:
	__fmpUnloadMediaContext(ffmpeg, media);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

//
// Context
//
static bool __fmpIsContextInitialized(const fmpContext *context) {
	if (context == fpl_null || !context->isValid)
		return false;
	if (!context->ffmpeg.isValid)
		return false;
	return true;
}

static void __fmpReleaseContext(fmpContext *context) {
	fplAssertPtr(context);
	if (context->ffmpeg.isValid)
		FFMPEGRelease(&context->ffmpeg);
	fplClearStruct(context);
}

FMP_API void fmpRelease(fmpContext *context) {
	if (context == fpl_null)
		return;
	__fmpReleaseContext(context);
}

FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator) {
	if (context == fpl_null)
		return fmpResult_Arguments_Invalid;

	if (__fmpIsContextInitialized(context))
		return fmpResult_Context_AlreadyInitialized;

	fplClearStruct(context);

	fmpResult errorResult = fmpResult_UnknownError;

	if (!FFMPEGInit(&context->ffmpeg)) {
		errorResult = fmpResult_Backend_FailedInitialization;
		goto failed;
	}

	if (allocator != fpl_null && allocator->alloc != fpl_null && allocator->realloc != fpl_null && allocator->free != fpl_null)
		context->allocator = *allocator;
	else
		context->allocator = __fmpCreateDefaultAllocator();

	context->isValid = true;

	return fmpResult_Success;

failed:
	__fmpReleaseContext(context);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

//
// Media info
//
FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media) {
	if (context == fpl_null || media == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;

	const fmpMemoryAllocator *allocator = &context->allocator;
	for (uint32_t i = 0; i < media->streamCount; ++i) {
		__fmpFreeLanguageInfo(allocator, &media->streams[i].language);
		__fmpFreeCodecInfo(allocator, &media->streams[i].codec);
	}
	__fmpFreeString(allocator, &media->title);
	fplClearStruct(media);
}

FMP_API fmpResult fmpGetMediaInfo(const fmpContext *context, const fmpMediaSource *source, fmpMediaInfo *media) {
	if (context == fpl_null || source == fpl_null || media == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!__fmpIsContextInitialized(context))
		return fmpResult_Context_NotInitialized;

	fmpResult result = fmpResult_UnknownError;

	const fmpMemoryAllocator *allocator = &context->allocator;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	AVFormatContext *formatCtx;

	formatCtx = ffmpeg->avformat_alloc_context();
	if (formatCtx == fpl_null) {
		result = fmpResult_Backend_AllocationFailed;
		goto release;
	}

	fmpResult openRes = __fmpOpenInputFromSource(ffmpeg, &formatCtx, source);
	if (openRes != fmpResult_Success) {
		result = openRes;
		goto release;
	}

	__fmpFillMediaInfo(allocator, ffmpeg, source, formatCtx, media);

	result = fmpResult_Success;

release:
	if (formatCtx != fpl_null) {
		__fmpCloseInput(ffmpeg, &formatCtx);
	}
	if (result != fmpResult_Success) {
		fmpReleaseMediaInfo(context, media);
	}
	return result;
}

//
// Media/Packet/Frame Helper
//
static bool __fmpIsMediaLoaded(const fmpMediaContext *media) {
	if (media == fpl_null || !media->isValid)
		return false;
	if (media->formatCtx == fpl_null)
		return false;
	return true;
}

static bool __fmpIsPacketInitialized(const fmpPacket *packet) {
	if (packet == fpl_null || !packet->isValid)
		return false;
	if (packet->pkt == fpl_null)
		return false;
	return true;
}

static bool __fmpIsFrameInitialized(const fmpFrame *frame) {
	if (frame == fpl_null || !frame->isValid)
		return false;
	if (frame->nativeFrame == fpl_null)
		return false;
	return true;
}

//
// Load / Unload
//
FMP_API void fmpUnloadMedia(const fmpContext *context, fmpMediaContext **media) {
	if (context == fpl_null || media == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;
	fmpMediaContext *mediaReference = *media;
	if (mediaReference != fpl_null) {
		const fmpMemoryAllocator *allocator = &context->allocator;
		const FFMPEGContext *ffmpeg = &context->ffmpeg;
		__fmpUnloadMediaContext(ffmpeg, mediaReference);
		__fmpFreeMemory(allocator, mediaReference);
	}
	*media = fpl_null;
}

FMP_API fmpResult fmpLoadMedia(const fmpContext *context, const fmpMediaSource *source, const fmpMediaOptions *options, fmpMediaContext **outMedia) {
	if (context == fpl_null || source == fpl_null || outMedia == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!context->isValid)
		return fmpResult_Context_NotInitialized;

	fmpMediaContext *mediaReference = *outMedia;
	if ((mediaReference != fpl_null) && (__fmpGetSafeMediaState(mediaReference) != fmpMediaState_NotInitialized))
		return fmpResult_Media_StillLoaded;

	switch (source->type) {
		case fmpMediaSourceType_File: {
			if (fplGetStringLength(source->file.filePath) == 0)
				return fmpResult_MediaSource_Invalid;
			if (!fplFileExists(source->file.filePath))
				return fmpResult_MediaSource_FileOrPathNotFound;
		} break;

		case fmpMediaSourceType_URL: {
			if (fplGetStringLength(source->url.url) == 0)
				return fmpResult_MediaSource_Invalid;
		} break;

		default:
			return fmpResult_MediaSource_Unsupported;
	}

	const fmpMemoryAllocator *allocator = &context->allocator;

	fmpResult errorResult = fmpResult_UnknownError;

	fmpMediaContext *newMedia = (fmpMediaContext *)__fmpAllocateMemory(allocator, sizeof(fmpMediaContext));
	if (newMedia == fpl_null) {
		errorResult = fmpResult_System_AllocationFailed;
		goto failed;
	}

	fmpResult mediaRes = __fmpLoadMediaIntoContext(context, source, options, newMedia);
	if (mediaRes != fmpResult_Success) {
		errorResult = mediaRes;
		goto failed;
	}

	__fmpSetSafeMediaState(newMedia, fmpMediaState_Stopped);

	newMedia->isValid = true;

	*outMedia = newMedia;

	return fmpResult_Success;

failed:
	if (newMedia != fpl_null)
		__fmpFreeMemory(allocator, newMedia);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

//
// Packet
//
FMP_API fmpResult fmpInitPacket(const fmpContext *context, fmpPacket *packet) {
	if (context == fpl_null || packet == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!__fmpIsContextInitialized(context))
		return fmpResult_Context_NotInitialized;
	if (__fmpIsPacketInitialized(packet))
		return fmpResult_Packet_AlreadyInitialized;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	AVPacket *nativePacket = ffmpeg->av_packet_alloc();
	if (nativePacket == fpl_null)
		return fmpResult_Packet_AllocationFailed;

	fplClearStruct(packet);
	packet->pkt = nativePacket;
	packet->isValid = true;

	return fmpResult_Success;
}

FMP_API void fmpReleasePacket(const fmpContext *context, fmpPacket *packet) {
	if (context == fpl_null || packet == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	if (packet->pkt != fpl_null)
		ffmpeg->av_packet_free(&packet->pkt);
	fplClearStruct(packet);
}

FMP_API fmpResult fmpReadPacket(const fmpContext *context, fmpMediaContext *media, fmpPacket *packet) {
	if (context == fpl_null || media == fpl_null || packet == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!__fmpIsContextInitialized(context))
		return fmpResult_Context_NotInitialized;
	if (!__fmpIsMediaLoaded(media))
		return fmpResult_Media_NotLoaded;
	if (!__fmpIsPacketInitialized(packet))
		return fmpResult_Packet_NotInitialized;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	AVFormatContext *ic = media->formatCtx;

	fmpResult errorResult = fmpResult_UnknownError;

	AVPacket *nativePacket = packet->pkt;

	int ret = ffmpeg->av_read_frame(media->formatCtx, nativePacket);
	if (ret < 0) {
		if ((ret == AVERROR_EOF || ffmpeg->avio_feof(ic->pb)) && !media->isEOF) {
			media->isEOF = true;
		}
		if (ic->pb != fpl_null && ic->pb->error) {
			errorResult = fmpResult_Packet_ReadFailure;
			goto failed;
		}
	} else {
		media->isEOF = false;
	}

	int streamIndex = nativePacket->stream_index;
	fplAssert(streamIndex >= 0);
	packet->index.index = (uint32_t)streamIndex;

	if (packet->index.index == media->videoStream.base.index.index) {
		packet->type = fmpStreamType_Video;
	} else if (packet->index.index == media->audioStream.base.index.index) {
		packet->type = fmpStreamType_Audio;
	} else if (packet->index.index == media->subtitleStream.base.index.index) {
		packet->type = fmpStreamType_Subtitle;
	} else {
		packet->type = fmpStreamType_Unknown;
	}

	packet->pos = nativePacket->pos;
	packet->pts = (nativePacket->pts != AV_NOPTS_VALUE) ? nativePacket->pts : 0;
	packet->dts = (nativePacket->dts != AV_NOPTS_VALUE) ? nativePacket->dts : 0;

	if (media->isEOF)
		return fmpResult_Media_EndOfStream;
	else
		return fmpResult_Success;

failed:
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

FMP_API void fmpClosePacket(const fmpContext *context, fmpPacket *packet) {
	if (context == fpl_null || packet == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	if (packet->pkt != fpl_null) {
		ffmpeg->av_packet_unref(packet->pkt);
	}
}

//
// Frame
//
FMP_API fmpResult fmpInitFrame(const fmpContext *context, fmpFrame *frame) {
	if (context == fpl_null || frame == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!__fmpIsContextInitialized(context))
		return fmpResult_Context_NotInitialized;
	if (__fmpIsFrameInitialized(frame))
		return fmpResult_Frame_AlreadyInitialized;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	AVFrame *nativeFrame = ffmpeg->av_frame_alloc();
	if (nativeFrame == fpl_null)
		return fmpResult_Frame_AllocationFailed;

	fplClearStruct(frame);
	frame->nativeFrame = nativeFrame;
	frame->isValid = true;

	return fmpResult_Success;
}

FMP_API void fmpReleaseFrame(const fmpContext *context, fmpFrame *frame) {
	if (context == fpl_null || frame == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	if (frame->nativeFrame != fpl_null) {
		ffmpeg->av_frame_free(&frame->nativeFrame);
	}
	fplClearStruct(frame);
}

FMP_API fmpResult fmpDecodeFrame(const fmpContext *context, fmpMediaContext *media, fmpFrame *frame, fmpPacket *packet) {
	if (context == fpl_null || media == fpl_null || frame == fpl_null || packet == fpl_null)
		return fmpResult_Arguments_Invalid;
	if (!__fmpIsContextInitialized(context))
		return fmpResult_Context_NotInitialized;
	if (!__fmpIsMediaLoaded(media))
		return fmpResult_Media_NotLoaded;
	if (!__fmpIsFrameInitialized(frame))
		return fmpResult_Frame_NotInitialized;
	if (!__fmpIsPacketInitialized(packet))
		return fmpResult_Packet_NotInitialized;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	fmpDecoder *decoder = fpl_null;
	switch (packet->type) {
		case fmpStreamType_Video:
			if (packet->pkt->stream_index != (int)media->videoStream.base.index.index)
				return fmpResult_Backend_NoStreamsFound;
			decoder = &media->videoStream.base.decoder;
			break;
		case fmpStreamType_Audio:
			if (packet->pkt->stream_index != (int)media->audioStream.base.index.index)
				return fmpResult_Backend_NoStreamsFound;
			decoder = &media->audioStream.base.decoder;
			break;
		case fmpStreamType_Subtitle:
			if (packet->pkt->stream_index != (int)media->subtitleStream.base.index.index)
				return fmpResult_Backend_NoStreamsFound;
			decoder = &media->subtitleStream.base.decoder;
			break;
		default:
			return fmpResult_Backend_NoDecoderFound;
	}

	AVCodecContext *codecCtx = decoder->codecCtx;
	fplAssertPtr(codecCtx);

	bool packetSent = false;

	for (;;) {
		int receiveRet = ffmpeg->avcodec_receive_frame(codecCtx, frame->nativeFrame);
		if (receiveRet >= 0) {
			break; // Success
		} else if (receiveRet == AVERROR_EOF) {
			ffmpeg->avcodec_flush_buffers(codecCtx);
			return fmpResult_Media_EndOfStream;
		} else if (receiveRet == AVERROR(EAGAIN)) {
			if (!packetSent) {
				packetSent = true;
				int sentRet = ffmpeg->avcodec_send_packet(codecCtx, packet->pkt);
				if (sentRet < 0) {
					return fmpResult_Backend_DecodeError;
				}
			} else {
				return fmpResult_Frame_RequireMorePackets;
			}
		} else {
			return fmpResult_Backend_DecodeError;
		}
	}

	// We dont need pos and serial, when we manually decode a frame
	frame->pos = 0;
	frame->serial = 0;

	return fmpResult_Success;
}

FMP_API void fmpCloseFrame(const fmpContext *context, fmpFrame *frame) {
	if (context == fpl_null || frame == fpl_null)
		return;
	if (!__fmpIsContextInitialized(context))
		return;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	if (frame->nativeFrame != fpl_null)
		ffmpeg->av_frame_unref(frame->nativeFrame);
}


#endif // FMP_IMPLEMENTATION