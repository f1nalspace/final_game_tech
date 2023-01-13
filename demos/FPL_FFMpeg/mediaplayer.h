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

typedef struct fmpPacket {
	AVPacket *pkt;
	int32_t serial;
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

typedef struct fmpFrame {
	AVFrame *frame;
	AVSubtitle subtitle;
	AVRational sar;
	double pts;
	double duration;
	int64_t pos;
	int32_t serial;
	int32_t width;
	int32_t height;
	int32_t format;
	int32_t uploaded;
	int32_t flipV;
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

typedef enum fmpStreamType {
	fmpStreamType_Unknown = 0,
	fmpStreamType_Video,
	fmpStreamType_Audio,
	fmpStreamType_Subtitle
} fmpStreamType;

typedef struct fmpStream {
	fmpPacketQueue packetQueue;
	fmpFrameQueue frameQueue;
	fmpDecoder decoder;
	AVStream *stream;
	AVCodecContext *codecContext;
	fmpStreamType type;
	int32_t index;
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
	};
	fmpCodecInfo codec;
	fmpLanguageInfo language;
	fmpStreamType type;
	uint32_t index;
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

	fplConditionVariable continueReadCondition;

	struct fmpContext *context;

	AVFormatContext *formatCtx;
	AVPacket *readPacket;

	double maxFrameDuration;

	fmpClockSyncType syncType;

	fpl_b32 isEOF;
	fpl_b32 isRealTime;
	fpl_b32 isValid;
} fmpMediaContext;

typedef enum fmpMediaState {
	fmpMediaState_Error = -1,
	fmpMediaState_NotInitialized = 0,
	fmpMediaState_Processing,
	fmpMediaState_Unloaded,
	fmpMediaState_Loaded,
	fmpMediaState_Playing,
	fmpMediaState_Paused,
} fmpMediaState;

typedef struct fmpContext {
	FFMPEGContext ffmpeg;
	fmpMediaContext media;
	fmpMemoryAllocator allocator;
	fplMutexHandle processMediaLock;
	volatile fmpMediaState state;
	fpl_b32 isValid;
} fmpContext;

typedef enum fmpResult {
	fmpResult_UnknownError = INT32_MIN,

	// Backend errors
	fmpResult_BackendNotInitialized = -1000,
	fmpResult_BackendMemoryAllocationFailed,
	fmpResult_BackendFailedInitialization,
	fmpResult_MediaNotSupported,
	fmpResult_MediaNotLoaded,
	fmpResult_NoStreamsFound,
	fmpResult_TooManyStreams,
	fmpResult_InvalidStream,
	fmpResult_NoDecoderFound,
	fmpResult_NoAudioDeviceFound,

	// Threading errors
	fmpResult_ThreadFailedCreation = -900,
	fmpResult_MutexFailedInitialization,
	fmpResult_MutexNotInitialized,
	fmpResult_ConditionVariableFailedInitialization,
	fmpResult_ConditionVariableNotInitialized,

	// Packet/Frame errors
	fmpResult_PacketQueueNotInitialized = -800,
	fmpResult_PacketQueueFailedInitialization,
	fmpResult_PacketAllocationFailed,
	fmpResult_PacketQueueEmpty,
	fmpResult_FrameQueueNotInitialized,
	fmpResult_FrameQueueFailedInitialization,
	fmpResult_FrameQueueEmpty,
	fmpResult_FrameAllocationFailed,

	// Argument errors
	fmpResult_InvalidArguments = -700,
	fmpResult_InvalidMediaSource,
	fmpResult_UnsupportedMediaSource,
	fmpResult_FileOrPathNotFound,

	// Memory errors
	fmpResult_SystemMemoryAllocationFailed = -600,

	// Context errors
	fmpResult_ContextAlreadyInitialized = -1,
	fmpResult_ContextNotInitialized = 0,

	fmpResult_Success = 1,
} fmpResult;

FMP_API fmpMediaState fmpGetMediaState(fmpContext *context);

FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator);
FMP_API void fmpRelease(fmpContext *context);

FMP_API fmpResult fmpGetMediaInfo(const fmpContext *context, const fmpMediaSource *source, fmpMediaInfo *media);
FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media);

FMP_API fmpResult fmpLoadMedia(fmpContext *context, const fmpMediaSource *source, const fmpMediaOptions *options);
FMP_API void fmpUnloadMedia(fmpContext *context);

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
fpl_force_inline void __fmpSetSafeMediaState(fmpContext *context, const fmpMediaState state) {
	fplAtomicExchangeS32((volatile int32_t *)&context->state, (int32_t)state);
}

fpl_force_inline fmpMediaState __fmpGetSafeMediaState(fmpContext *context) {
	return (fmpMediaState)fplAtomicLoadS32((volatile int32_t *)&context->state);
}

//
// Clock
//
static double __fmpGetClock(const FFMPEGContext *ffmpeg, fmpClock *c) {
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

static void __fmpSetClockTime(fmpClock *c, double pts, int serial, double time) {
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

static fmpClockSyncType __fmpGetMasterSyncType(fmpMediaContext *ctx) {
	if (ctx->syncType == fmpClockSyncType_VideoMaster) {
		if (ctx->videoStream.base.isValid)
			return fmpClockSyncType_VideoMaster;
		else
			return fmpClockSyncType_AudioMaster;
	} else if (ctx->syncType == fmpClockSyncType_AudioMaster && ctx->audioStream.base.isValid) {
		return fmpClockSyncType_AudioMaster;
	}
	return fmpClockSyncType_ExternalClock;
}

/* get the current master clock value */
static double __fmpGetMasterClock(fmpMediaContext *ctx) {
	double val;
	const FFMPEGContext *ffmpeg = &ctx->context->ffmpeg;
	fmpClockSyncType syncType = __fmpGetMasterSyncType(ctx);
	switch (syncType) {
		case fmpClockSyncType_VideoMaster:
			val = __fmpGetClock(ffmpeg, &ctx->videoClock);
			break;
		case fmpClockSyncType_AudioMaster:
			val = __fmpGetClock(ffmpeg, &ctx->audioClock);
			break;
		default:
			val = __fmpGetClock(ffmpeg, &ctx->externalClock);
			break;
	}
	return val;
}

static void __fmpCheckExternalClockSpeed(fmpMediaContext *ctx) {
	const FFMPEGContext *ffmpeg = &ctx->context->ffmpeg;
	fmpStream *videoStream = &ctx->videoStream.base;
	fmpStream *audioStream = &ctx->audioStream.base;
	if (videoStream->isValid && videoStream->packetQueue.packetCount <= __FMP_EXTERNAL_CLOCK_MIN_FRAMES ||
		audioStream->isValid && audioStream->packetQueue.packetCount <= __FMP_EXTERNAL_CLOCK_MIN_FRAMES) {
		__fmpSetClockSpeed(ffmpeg, &ctx->externalClock, FFMAX(__FMP_EXTERNAL_CLOCK_SPEED_MIN, ctx->externalClock.speed - __FMP_EXTERNAL_CLOCK_SPEED_STEP));
	} else if ((!videoStream->isValid || videoStream->packetQueue.packetCount > __FMP_EXTERNAL_CLOCK_MAX_FRAMES) &&
			   (!audioStream->isValid || audioStream->packetQueue.packetCount > __FMP_EXTERNAL_CLOCK_MAX_FRAMES)) {
		__fmpSetClockSpeed(ffmpeg, &ctx->externalClock, FFMIN(__FMP_EXTERNAL_CLOCK_SPEED_MAX, ctx->externalClock.speed + __FMP_EXTERNAL_CLOCK_SPEED_STEP));
	} else {
		double speed = ctx->externalClock.speed;
		if (speed != 1.0) {
			__fmpSetClockSpeed(ffmpeg, &ctx->externalClock, speed + __FMP_EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
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
		errorRes = fmpResult_PacketAllocationFailed;
		goto failed;
	}
	if (!fplMutexInit(&queue->mutex)) {
		errorRes = fmpResult_MutexFailedInitialization;
		goto failed;
	}
	if (!fplConditionInit(&queue->cond)) {
		errorRes = fmpResult_ConditionVariableFailedInitialization;
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
	ffmpeg->av_frame_unref(frame->frame);
	ffmpeg->avsubtitle_free(&frame->subtitle);
}

static void __fmpFrameQueueDestroy(const FFMPEGContext *ffmpeg, fmpFrameQueue *queue) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(queue);
	if (queue->isValid) {
		for (int32_t i = 0; i < queue->maxSize; ++i) {
			fmpFrame *frame = queue->queue + i;
			if (frame->frame != fpl_null) {
				__fmpFrameQueueUnref(ffmpeg, frame);
				ffmpeg->av_frame_free(&frame->frame);
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
		return fmpResult_InvalidArguments;

	fplClearStruct(frameQueue);

	fmpResult errorRes = fmpResult_UnknownError;

	if (!fplMutexInit(&frameQueue->mutex)) {
		errorRes = fmpResult_MutexFailedInitialization;
		goto failed;
	}
	if (!fplConditionInit(&frameQueue->cond)) {
		errorRes = fmpResult_ConditionVariableFailedInitialization;
		goto failed;
	}

	frameQueue->maxSize = fplMin(maxSize, FMP_MAX_FRAME_QUEUE_COUNT);
	frameQueue->keepLast = !!keepLast;
	for (int32_t i = 0; i < frameQueue->maxSize; ++i) {
		if (!(frameQueue->queue[i].frame = ffmpeg->av_frame_alloc())) {
			errorRes = fmpResult_FrameAllocationFailed;
			goto failed;
		}
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
		errorResult = fmpResult_PacketAllocationFailed;
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
								frame->pts = av_rescale_q(frame->pts, codecCtx->pkt_timebase, tb);
							else if (decoder->nextPts != AV_NOPTS_VALUE)
								frame->pts = av_rescale_q(decoder->nextPts, decoder->nextPtsTimeBase, tb);
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
			av_packet_unref(packet);
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
	__fmpFreeString(allocator, &info->name);
}

static fmpCodecInfo __fmpGetCodecInfo(const fmpMemoryAllocator *allocator, const FFMPEGContext *ffmpeg, const AVCodecParameters *params) {
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

	result.fourcc[4] = '\0';

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

		fmpStreamInfo *info = media->streams + streamIndex;
		info->type = fmpStreamType_Unknown;
		info->index = st->index;

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

static fmpResult __fmpOpenAudioStream(fmpMediaContext *media, fmpAudioStream *stream, AVCodecContext *codecCtx) {
	fplAssertPtr(media);
	fplAssertPtr(stream);
	fplAssertPtr(codecCtx);
	fmpContext *context = media->context;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	AVChannelLayout channelLayout = fplZeroInit;

	fmpResult result = fmpResult_Success;

	int32_t sampleRate = codecCtx->sample_rate;

	int ret = ffmpeg->av_channel_layout_copy(&channelLayout, &codecCtx->ch_layout);
	if (ret < 0) {
		result = fmpResult_InvalidStream;
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

static void __fmpCloseStream(fmpMediaContext *media, fmpStream *targetStream) {
	fplAssertPtr(media);
	fplAssertPtr(targetStream);

	fmpContext *context = media->context;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;

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

static fmpResult __fmpOpenStream(fmpMediaContext *media, const int32_t streamIndex, const char *codecName, fmpStream *targetStream) {
	fplAssertPtr(media);
	fplAssertPtr(targetStream);

	AVFormatContext *formatCtx = media->formatCtx;
	fplAssertPtr(formatCtx);

	if (streamIndex < 0 || streamIndex >= (int32_t)formatCtx->nb_streams) {
		return fmpResult_InvalidStream;
	}

	fmpContext *context = media->context;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	AVStream *st = formatCtx->streams[streamIndex];

	AVCodecContext *codecCtx = fpl_null;
	const AVCodec *codec = fpl_null;
	uint32_t sampleRate = 0;

	fplClearStruct(targetStream);
	targetStream->stream = st;
	targetStream->index = streamIndex;

	fmpResult result = fmpResult_Success;

	codecCtx = ffmpeg->avcodec_alloc_context3(fpl_null);
	if (codecCtx == fpl_null) {
		result = fmpResult_BackendMemoryAllocationFailed;
		goto failed;
	}

	int ret = ffmpeg->avcodec_parameters_to_context(codecCtx, st->codecpar);
	if (ret < 0) {
		result = fmpResult_InvalidStream;
		goto failed;
	}

	codecCtx->pkt_timebase = st->time_base;

	if (fplGetStringLength(codecName) > 0) {
		codec = ffmpeg->avcodec_find_decoder_by_name(codecName);
	} else {
		codec = ffmpeg->avcodec_find_decoder(codecCtx->codec_id);
	}

	if (codec == fpl_null) {
		result = fmpResult_NoDecoderFound;
		goto failed;
	}

	codecCtx->codec_id = codec->id;

	st->discard = AVDISCARD_DEFAULT;

	int32_t queueCapcity = 0;
	int32_t keepLast = 0;
	switch (codecCtx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
		{
			fmpAudioStream *audioStream = (fmpAudioStream *)targetStream;
			fmpResult audioResult = __fmpOpenAudioStream(media, audioStream, codecCtx);
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
	if ((decoderResult = __fmpDecoderInit(ffmpeg, &targetStream->decoder, codecCtx, &targetStream->packetQueue, &media->continueReadCondition)) != fmpResult_Success) {
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
			return fmpResult_UnsupportedMediaSource;
	}

	int openRes = ffmpeg->avformat_open_input(inOutCtx, pathOrURL, fpl_null, fpl_null);
	if (openRes < 0) {
		return fmpResult_MediaNotSupported;
	}

	AVFormatContext *thisFormatCtx = *inOutCtx;

	fplAssertPtr(thisFormatCtx);

	int streamInfoRes = ffmpeg->avformat_find_stream_info(thisFormatCtx, fpl_null);
	if (streamInfoRes < 0) {
		return fmpResult_NoStreamsFound;
	}

	if (thisFormatCtx->nb_streams > FMP_MAX_STREAM_COUNT) {
		return fmpResult_TooManyStreams;
	}

	ffmpeg->av_format_inject_global_side_data(thisFormatCtx);

	return fmpResult_Success;
}

static void __fmpUnloadMediaContext(fmpContext *context, fmpMediaContext *media) {
	fplAssertPtr(context);
	fplAssertPtr(media);

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	__fmpCloseStream(media, &media->videoStream.base);
	__fmpCloseStream(media, &media->audioStream.base);
	__fmpCloseStream(media, &media->subtitleStream.base);

	if (media->formatCtx != fpl_null) {
		__fmpCloseInput(ffmpeg, &media->formatCtx);
	}

	if (media->readPacket != fpl_null) {
		ffmpeg->av_packet_free(&media->readPacket);
	}

	if (media->continueReadCondition.isValid) {
		fplConditionDestroy(&media->continueReadCondition);
	}

	fplClearStruct(media);
}

static fmpResult __fmpLoadMediaIntoContext(fmpContext *context, fmpMediaContext *media, const fmpMediaSource *source, const fmpMediaOptions *options) {
	fplAssertPtr(context);
	fplAssertPtr(media);
	fplAssertPtr(source);
	fplAssertPtr(options);

	const FFMPEGContext *ffmpeg = &context->ffmpeg;
	const fmpMemoryAllocator *allocator = &context->allocator;

	fplClearStruct(media);
	media->context = context;
	if (options != fpl_null) {
		media->options = *options;
	}

	fmpResult errorResult = fmpResult_Success;

	media->formatCtx = fpl_null;

	//
	// Condition
	//
	if (!fplConditionInit(&media->continueReadCondition)) {
		errorResult = fmpResult_ConditionVariableFailedInitialization;
		goto failed;
	}

	//
	// Allocate read packet
	//
	if ((media->readPacket = ffmpeg->av_packet_alloc()) == fpl_null) {
		errorResult = fmpResult_PacketAllocationFailed;
		goto failed;
	}

	//
	// Load path or url into a AVFormatContext
	//
	media->formatCtx = ffmpeg->avformat_alloc_context();
	if (media->formatCtx == fpl_null) {
		errorResult = fmpResult_BackendMemoryAllocationFailed;
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

	int32_t videoStreamIndex = -1;
	int32_t audioStreamIndex = -1;
	int32_t subtitleStreamIndex = -1;
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
		errorResult = fmpResult_NoStreamsFound;
		goto failed;
	}

	if (videoStreamIndex >= 0) {
		fmpResult streamRes = __fmpOpenStream(media, videoStreamIndex, fpl_null, &media->videoStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}
	if (audioStreamIndex >= 0) {
		fmpResult streamRes = __fmpOpenStream(media, audioStreamIndex, fpl_null, &media->audioStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}
	if (subtitleStreamIndex >= 0) {
		fmpResult streamRes = __fmpOpenStream(media, subtitleStreamIndex, fpl_null, &media->subtitleStream.base);
		if (streamRes != fmpResult_Success) {
			errorResult = streamRes;
			goto failed;
		}
	}

	return fmpResult_Success;

failed:
	if (media->formatCtx != fpl_null)
		__fmpCloseInput(ffmpeg, &media->formatCtx);
	__fmpUnloadMediaContext(context, media);
	fplAssert(errorResult != fmpResult_Success);
	return errorResult;
}

//
// Context
//
FMP_API fmpMediaState fmpGetMediaState(fmpContext *context) {
	if (context == fpl_null || !context->isValid)
		return fmpMediaState_NotInitialized;
	fmpMediaState result = __fmpGetSafeMediaState(context);
	return result;
}

FMP_API void fmpRelease(fmpContext *context) {
	if (context == fpl_null || !context->isValid)
		return;

	fmpMediaState state = __fmpGetSafeMediaState(context);
	if (state > fmpMediaState_Unloaded) {
		__fmpUnloadMediaContext(context, &context->media);
	}

	if (context->ffmpeg.isValid) {
		FFMPEGRelease(&context->ffmpeg);
	}

	if (context->processMediaLock.isValid) {
		fplMutexDestroy(&context->processMediaLock);
	}

	fplClearStruct(context);
}

FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator) {
	if (context == fpl_null)
		return fmpResult_InvalidArguments;
	if (context->isValid)
		return fmpResult_ContextAlreadyInitialized;

	fplClearStruct(context);

	fmpResult errorResult = fmpResult_UnknownError;

	if (!fplMutexInit(&context->processMediaLock)) {
		errorResult = fmpResult_MutexFailedInitialization;
		goto failed;
	}

	if (!FFMPEGInit(&context->ffmpeg)) {
		errorResult = fmpResult_BackendFailedInitialization;
		goto failed;
	}

	if (allocator != fpl_null && allocator->alloc != fpl_null && allocator->realloc != fpl_null && allocator->free != fpl_null)
		context->allocator = *allocator;
	else
		context->allocator = __fmpCreateDefaultAllocator();

	context->isValid = true;

	__fmpSetSafeMediaState(context, fmpMediaState_Unloaded);

	return fmpResult_Success;

failed:
	fmpRelease(context);
	fplAssert(errorResult != fmpResult_UnknownError);
	return errorResult;
}

//
// Media info
//
FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media) {
	if (context == fpl_null || !context->isValid || media == fpl_null)
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
		return fmpResult_InvalidArguments;
	if (!context->isValid)
		return fmpResult_ContextNotInitialized;

	fmpResult result = fmpResult_UnknownError;

	const fmpMemoryAllocator *allocator = &context->allocator;
	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	AVFormatContext *formatCtx;

	formatCtx = ffmpeg->avformat_alloc_context();
	if (formatCtx == fpl_null) {
		result = fmpResult_BackendMemoryAllocationFailed;
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
// Load / Unload
//
static void __fmpSafeUnloadMedia(fmpContext *context) {
	fplAssertPtr(context);
	if (__fmpGetSafeMediaState(context) >= fmpMediaState_Loaded) {
		__fmpSetSafeMediaState(context, fmpMediaState_Processing);
		fplMutexLock(&context->processMediaLock);
		{
			__fmpUnloadMediaContext(context, &context->media);
		}
		fplMutexUnlock(&context->processMediaLock);
	}
	__fmpSetSafeMediaState(context, fmpMediaState_Unloaded);
}

FMP_API void fmpUnloadMedia(fmpContext *context) {
	if (context == fpl_null || !context->isValid)
		return;
	__fmpSafeUnloadMedia(context);
}

FMP_API fmpResult fmpLoadMedia(fmpContext *context, const fmpMediaSource *source, const fmpMediaOptions *options) {
	if (context == fpl_null || source == fpl_null)
		return fmpResult_InvalidArguments;
	if (!context->isValid)
		return fmpResult_ContextNotInitialized;

	switch (source->type) {
		case fmpMediaSourceType_File: {
			if (fplGetStringLength(source->file.filePath) == 0)
				return fmpResult_InvalidMediaSource;
			if (!fplFileExists(source->file.filePath))
				return fmpResult_FileOrPathNotFound;
		} break;

		case fmpMediaSourceType_URL: {
			if (fplGetStringLength(source->url.url) == 0)
				return fmpResult_InvalidMediaSource;
		} break;

		default:
			return fmpResult_UnsupportedMediaSource;
	}

	// Unload if needed
	__fmpSafeUnloadMedia(context);

	// Load
	fmpResult result;
	fplMutexLock(&context->processMediaLock);
	{
		__fmpSetSafeMediaState(context, fmpMediaState_Processing);
		result = __fmpLoadMediaIntoContext(context, &context->media, source, options);
		if (result == fmpResult_Success)
			__fmpSetSafeMediaState(context, fmpMediaState_Loaded);
		else
			__fmpSetSafeMediaState(context, fmpMediaState_Error);
	}
	fplMutexUnlock(&context->processMediaLock);

	return result;
}

#endif // FMP_IMPLEMENTATION