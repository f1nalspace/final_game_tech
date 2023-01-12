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
#define FMP_MAX_VIDEO_FRAME_QUEUE_COUNT 4
#define FMP_MAX_AUDIO_FRAME_QUEUE_COUNT 8
#define FMP_MAX_FRAME_QUEUE_COUNT 8 // Must be the max of all frame queue counts

// Max number of streams in a media or url
#define FMP_MAX_STREAM_COUNT 8

typedef struct fmpString {
	char *data;
	size_t len;
} fmpString;


typedef struct fmpMemory {
	struct fmpMemory *next;
	void *base;
	size_t size;
	size_t offset;
} fmpMemory;

#define FMP_MEMORY_ALLOCATE_FUNC(name) void *name(const size_t size, void *user)
typedef FMP_MEMORY_ALLOCATE_FUNC(fmp_memory_allocate_func);
#define FMP_MEMORY_REALLOCATE_FUNC(name) void *name(void *base, const size_t size, void *user)
typedef FMP_MEMORY_REALLOCATE_FUNC(fmp_memory_reallocate_func);
#define FMP_MEMORY_FREE_FUNC(name) void name(void *mem, void *user)
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
	FFMPEGContext *ffmpeg;
	AVFifo *packetList;
	uint64_t duration;
	uint32_t packetCount;
	uint32_t size;
	int32_t serial;
	int32_t abortRequest;
	fpl_b32 isValid;
} fmpPacketQueue;

typedef enum fmpPacketQueueResult {
	fmpPacketQueueResult_Error = -1,
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
	FFMPEGContext *ffmpeg;
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
	FFMPEGContext *ffmpeg;
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
	int32_t index;
	fpl_b32 isValid;
} fmpStream;

typedef enum fmpStreamType {
	fmpStreamType_Unknown = 0,
	fmpStreamType_Video,
	fmpStreamType_Audio,
	fmpStreamType_Subtitle
} fmpStreamType;

typedef struct fmpCodecInfo {
	fmpString name;
	char fourcc[4];
	uint64_t id;
} fmpCodecInfo;

typedef struct fmpVideoInfo {
	AVRational sampleAspectRatio;
	AVRational displayAspectRatio;
	AVRational frameRate;
	int32_t width;
	int32_t height;
} fmpVideoInfo;

typedef struct fmpAudioInfo {
	AVSampleFormat sampleFormat;
	uint32_t sampleRate;
	uint32_t channels;
	uint32_t bitsPerSample;
	uint32_t bitrate;
} fmpAudioInfo;

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

typedef struct fmpMediaInfo {
	fmpStreamInfo streams[FMP_MAX_STREAM_COUNT];
	fmpString url;
	uint32_t streamCount;
} fmpMediaInfo;

typedef struct fmpMediaContext {
	fmpMediaInfo info;

	fmpStream audioStream;
	fmpStream videoStream;
	fmpStream subtitleStream;

	fmpClock audioClock;
	fmpClock videoClock;
	fmpClock subtitleClock;
	fmpClock externalClock;

	struct fmpContext *context;

	AVFormatContext *formatCtx;

	fmpClockSyncType syncType;

	fpl_b32 isValid;
} fmpMediaContext;

typedef enum fmpMediaState {
	fmpMediaState_Error = -1,
	fmpMediaState_NotInitialized = 0,
	fmpMediaState_Unloaded,
	fmpMediaState_Loading,
	fmpMediaState_Loaded,
	fmpMediaState_Playing,
	fmpMediaState_Paused,
} fmpMediaState;

typedef struct fmpContext {
	FFMPEGContext ffmpeg;
	fmpMediaContext media;
	fmpMemoryAllocator allocator;
	volatile fmpMediaState state;
	fpl_b32 isValid;
} fmpContext;

typedef enum fmpResult {
	fmpResult_UnknownError = INT32_MIN,

	fmpResult_FailedToInitialize = -100,
	fmpResult_MediaNotSupported,
	fmpResult_TooManyStreams,
	fmpResult_FileNotFound,

	fmpResult_NotEnoughMemory = -50,

	fmpResult_InvalidArguments = -20,

	fmpResult_ContextAlreadyInitialized = -1,

	fmpResult_ContextNotInitialized = 0,

	fmpResult_Success = 1,
} fmpResult;

FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator);
FMP_API void fmpRelease(fmpContext *context);

FMP_API fmpResult fmpGetMediaInfo(const fmpContext *context, const char *url, fmpMediaInfo *media);
FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media);

FMP_API fmpResult fmpLoadMediaByFile(fmpContext *context, const char *filePath);
FMP_API fmpResult fmpLoadMediaByURL(fmpContext *context, const char *url);
FMP_API void fmpUnloadMedia(fmpContext *context);

FMP_API fmpMediaState fmpGetMediaState(fmpContext *context);

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
static void *__fmpAllocateMemory(const fmpMemoryAllocator *allocator, const size_t size) {
	return allocator->alloc(size, allocator->user);
}
static void __fmpFreeMemory(const fmpMemoryAllocator *allocator, void *ptr) {
	allocator->free(ptr, allocator->user);
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
static void *__fmpAVAllocate(const size_t size, void *user) {
	const FFMPEGContext *ffmpeg = (const FFMPEGContext *)user;
	void *result = ffmpeg->av_mallocz(size);
	return result;
}

static void *__fmpAVRealloc(void *base, const size_t size, void *user) {
	const FFMPEGContext *ffmpeg = (const FFMPEGContext *)user;
	void *result = ffmpeg->av_realloc(base, size);
	return result;
}

static void __fmpAVFree(void *ptr, void *user) {
	const FFMPEGContext *ffmpeg = (const FFMPEGContext *)user;
	ffmpeg->av_freep(ptr);
}

static fmpMemoryAllocator __fmpCreateAVAllocator(FFMPEGContext *ffmpeg) {
	fmpMemoryAllocator result = fplZeroInit;
	result.user = ffmpeg;
	result.alloc = __fmpAVAllocate;
	result.realloc = __fmpAVRealloc;
	result.free = __fmpAVFree;
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

static void __fmpSyncClockToSlave(const FFMPEGContext *ffmpeg, fmpClock *c, fmpClock *slave)
{
	double clock = __fmpGetClock(ffmpeg, c);
	double slave_clock = __fmpGetClock(ffmpeg, slave);
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > __FMP_AV_NOSYNC_THRESHOLD)) {
		__fmpSetClock(ffmpeg, c, slave_clock, slave->serial);
	}
}

static fmpClockSyncType __fmpGetMasterSyncType(fmpMediaContext *ctx) {
	if (ctx->syncType == fmpClockSyncType_VideoMaster) {
		if (ctx->videoStream.isValid)
			return fmpClockSyncType_VideoMaster;
		else
			return fmpClockSyncType_AudioMaster;
	} else if (ctx->syncType == fmpClockSyncType_AudioMaster && ctx->audioStream.isValid) {
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
	fmpStream *videoStream = &ctx->videoStream;
	fmpStream *audioStream = &ctx->audioStream;
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
static fmpPacketQueueResult _PacketQueuePushLocal(fmpPacketQueue *queue, AVPacket *pkt) {
	if (queue == fpl_null || !queue->isValid) return fmpPacketQueueResult_Error;

	if (queue->abortRequest) return fmpPacketQueueResult_Abort;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	fmpPacket entry = {};
	entry.pkt = pkt;
	entry.serial = queue->serial;

	int result = ffmpeg->av_fifo_write(queue->packetList, &entry, 1);
	if (result < 0) {
		return fmpPacketQueueResult_Error;
	}

	queue->packetCount++;
	queue->size += entry.pkt->size + sizeof(entry);
	queue->duration += entry.pkt->duration;

	fplConditionSignal(&queue->cond);

	return fmpPacketQueueResult_Success;
}

static fmpPacketQueueResult PacketQueuePush(fmpPacketQueue *queue, AVPacket *pkt) {
	if (queue == fpl_null || !queue->isValid) return fmpPacketQueueResult_Error;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	fplAssertPtr(pkt);
	AVPacket *newPacket = ffmpeg->av_packet_alloc();
	if (newPacket == fpl_null) {
		ffmpeg->av_packet_unref(pkt);
		return fmpPacketQueueResult_Error;
	}
	ffmpeg->av_packet_move_ref(newPacket, pkt);

	fmpPacketQueueResult result;
	fplMutexLock(&queue->mutex);
	result = _PacketQueuePushLocal(queue, newPacket);
	fplMutexUnlock(&queue->mutex);

	if (result < 0) {
		ffmpeg->av_packet_free(&newPacket);
	}

	return result;
}

static fmpPacketQueueResult PacketQueuePushNullPacket(fmpPacketQueue *queue, AVPacket *pkt, int streamIndex) {
	if (queue == fpl_null || !queue->isValid) return fmpPacketQueueResult_Error;
	fplAssertPtr(pkt);
	pkt->stream_index = streamIndex;
	fmpPacketQueueResult result = PacketQueuePush(queue, pkt);
	return result;
}

static fmpPacketQueueResult PacketQueuePop(fmpPacketQueue *queue, AVPacket *pkt, int block, int *serial) {
	if (queue == fpl_null || !queue->isValid) return fmpPacketQueueResult_Error;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

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

static void PacketQueueFlush(fmpPacketQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

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

static void PacketQueueDestroy(fmpPacketQueue *queue) {
	if (queue == fpl_null) return;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	if (queue->isValid) {
		PacketQueueFlush(queue);
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

static bool PacketQueueInit(fmpPacketQueue *queue, FFMPEGContext *ffmpeg) {
	if (queue == fpl_null || ffmpeg == fpl_null || !ffmpeg->isValid) return false;

	fplClearStruct(queue);

	queue->ffmpeg = ffmpeg;

	queue->packetList = ffmpeg->av_fifo_alloc2(1, sizeof(fmpPacket), AV_FIFO_FLAG_AUTO_GROW);
	if (queue->packetList == fpl_null) {
		goto failed;
	}
	if (!fplMutexInit(&queue->mutex))
	{
		goto failed;
	}
	if (!fplConditionInit(&queue->cond)) {
		goto failed;
	}

	queue->isValid = true;

	return true;
failed:
	PacketQueueDestroy(queue);
	return false;
}

static void PacketQueueAbort(fmpPacketQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 1;
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static void PacketQueueStart(fmpPacketQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 0;
	queue->serial++;
	fplMutexUnlock(&queue->mutex);
}

//
// > Frame-Queue
//
static void FrameQueueUnref(FFMPEGContext *ffmpeg, fmpFrame *frame) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(frame);
	ffmpeg->av_frame_unref(frame->frame);
	ffmpeg->avsubtitle_free(&frame->subtitle);
}

static void FrameQueueDestroy(fmpFrameQueue *queue) {
	if (queue == fpl_null) return;
	FFMPEGContext *ffmpeg = queue->ffmpeg;
	if (queue->isValid) {
		for (int32_t i = 0; i < queue->maxSize; ++i) {
			fmpFrame *frame = queue->queue + i;
			if (frame->frame != fpl_null) {
				FrameQueueUnref(queue->ffmpeg, frame);
				ffmpeg->av_frame_free(&frame->frame);
			}
		}
	}
	fplConditionDestroy(&queue->cond);
	fplMutexDestroy(&queue->mutex);
	fplClearStruct(queue);
}

static bool FrameQueueInit(FFMPEGContext *ffmpeg, fmpFrameQueue *frameQueue, fmpPacketQueue *packetQueue, int32_t maxSize, int32_t keepLast) {
	if (ffmpeg == fpl_null || !ffmpeg->isValid) return false;
	if (frameQueue == fpl_null || packetQueue == fpl_null) return false;

	fplClearStruct(frameQueue);
	frameQueue->ffmpeg = ffmpeg;
	frameQueue->packetQueue = packetQueue;

	if (!fplMutexInit(&frameQueue->mutex)) {
		goto failed;
	}
	if (!fplConditionInit(&frameQueue->cond)) {
		goto failed;
	}

	frameQueue->maxSize = fplMin(maxSize, FMP_MAX_FRAME_QUEUE_COUNT);
	frameQueue->keepLast = !!keepLast;
	for (int32_t i = 0; i < frameQueue->maxSize; ++i) {
		if (!(frameQueue->queue[i].frame = ffmpeg->av_frame_alloc())) {
			goto failed;
		}
	}

	frameQueue->isValid = true;
	return true;
failed:
	FrameQueueDestroy(frameQueue);
	return false;
}

static void FrameQueueSignal(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static fmpFrame *FrameQueuePeek(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown) % queue->maxSize;
	return &queue->queue[index];
}

static fmpFrame *FrameQueuePeekNext(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown + 1) % queue->maxSize;
	return &queue->queue[index];
}

static fmpFrame *FrameQueuePeekLast(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = queue->readIndex;
	return &queue->queue[index];
}

static fmpFrame *FrameQueuePeekWritable(fmpFrameQueue *queue) {
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

static fmpFrame *FrameQueuePeekReadable(fmpFrameQueue *queue) {
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

static void FrameQueuePush(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return;

	if (queue->keepLast && !queue->readIndexShown) {
		queue->readIndexShown = 1;
		return;
	}

	FrameQueueUnref(queue->ffmpeg, &queue->queue[queue->readIndex]);
	if (++queue->readIndex == queue->maxSize) {
		queue->readIndex = 0;
	}

	fplMutexLock(&queue->mutex);
	queue->size--;
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static int32_t GetFrameQueueRemainingCount(fmpFrameQueue *queue) {
	if (queue == fpl_null || !queue->isValid) return 0;
	int32_t result = queue->size - queue->readIndexShown;
	return result;
}

static int64_t GetFrameQueueLastPos(fmpFrameQueue *queue) {
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
static void DecoderDestroy(fmpDecoder *decoder) {
	if (decoder == fpl_null) return;
	FFMPEGContext *ffmpeg = decoder->ffmpeg;
	if (ffmpeg != fpl_null) {
		if (decoder->pendingPacket != fpl_null) {
			ffmpeg->av_packet_free(&decoder->pendingPacket);
		}
		if (decoder->codecCtx != fpl_null) {
			ffmpeg->avcodec_free_context(&decoder->codecCtx);
		}
	}
	fplClearStruct(&decoder);
}

static bool DecoderInit(FFMPEGContext *ffmpeg, fmpDecoder *decoder, AVCodecContext *codecCtx, fmpPacketQueue *packetQueue, fplConditionVariable *emptyQueueCondition) {
	if (ffmpeg == fpl_null || !ffmpeg->isValid) return false;
	if (decoder == fpl_null || codecCtx == fpl_null) return false;
	if (packetQueue == fpl_null || !packetQueue->isValid) return false;
	if (emptyQueueCondition == fpl_null || !emptyQueueCondition->isValid) return false;

	fplClearStruct(decoder);
	decoder->ffmpeg = ffmpeg;

	decoder->pendingPacket = ffmpeg->av_packet_alloc();
	if (decoder->pendingPacket == fpl_null) {
		goto failed;
	}
	decoder->codecCtx = codecCtx;
	decoder->packetQueue = packetQueue;
	decoder->emptyQueueCondition = emptyQueueCondition;
	decoder->startPts = AV_NOPTS_VALUE;
	decoder->packetSerial = -1;
	decoder->reorderPts = -1;
	decoder->isValid = true;
	return true;

failed:
	DecoderDestroy(decoder);
	return false;
}

static void DecoderAbort(fmpDecoder *decoder, fmpFrameQueue *frameQueue) {
	if (decoder == fpl_null || !decoder->isValid) return;
	PacketQueueAbort(decoder->packetQueue);
	FrameQueueSignal(frameQueue);
	fplThreadWaitForOne(decoder->thread, FPL_TIMEOUT_INFINITE);
	decoder->thread = fpl_null;
	PacketQueueFlush(decoder->packetQueue);
}

static int DecoderDecodeFrame(fmpDecoder *decoder, AVFrame *frame, AVSubtitle *subtitle) {
	if (decoder == fpl_null || !decoder->isValid) return -1;
	int ret = AVERROR(EAGAIN);
	FFMPEGContext *ffmpeg = decoder->ffmpeg;
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
				if (PacketQueuePop(queue, packet, 1, &decoder->packetSerial) < 0) {
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
// Context
//
FMP_API fmpResult fmpInit(fmpContext *context, const fmpMemoryAllocator *allocator) {
	if (context == fpl_null) 
		return fmpResult_InvalidArguments;
	if (context->isValid) 
		return fmpResult_ContextAlreadyInitialized;

	fplClearStruct(context);

	if (!FFMPEGInit(&context->ffmpeg)) {
		return fmpResult_FailedToInitialize;
	}

	if (allocator != fpl_null && allocator->alloc != fpl_null && allocator->realloc != fpl_null && allocator->free != fpl_null)
		context->allocator = *allocator;
	else
		context->allocator = __fmpCreateAVAllocator(&context->ffmpeg);

	context->isValid = true;

	return fmpResult_Success;
}

FMP_API void fmpRelease(fmpContext *context) {
	if (context == fpl_null)
		return;
	FFMPEGRelease(&context->ffmpeg);
	fplClearStruct(context);
}

static void __fmpFreeLanguageInfo(const fmpMemoryAllocator *allocator, fmpLanguageInfo *info) {
	__fmpFreeString(allocator, &info->name);
}

static fmpLanguageInfo __fmpGetLanguageInfo(const fmpMemoryAllocator *allocator, const FFMPEGContext *ffmpeg, AVDictionary *dict) {
	fmpLanguageInfo result = fplZeroInit;

	AVDictionaryEntry *lang = ffmpeg->av_dict_get(dict, "language", fpl_null, 0);
	size_t valueLen;
	if (lang != fpl_null && (valueLen = fplGetStringLength(lang->value)) > 0) {
		strncpy_s(result.iso639_2, fplMin(3, valueLen), lang->value, fplArrayCount(result.iso639_2));
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

FMP_API void fmpReleaseMediaInfo(const fmpContext *context, fmpMediaInfo *media) {
	if (context == fpl_null || !context->isValid || media == fpl_null)
		return;
	for (uint32_t i = 0; i < media->streamCount; ++i) {
		__fmpFreeLanguageInfo(&context->allocator, &media->streams[i].language);
		__fmpFreeCodecInfo(&context->allocator, &media->streams[i].codec);
	}
	__fmpFreeString(&context->allocator, &media->url);
	fplClearStruct(media);
}

FMP_API fmpResult fmpGetMediaInfo(const fmpContext *context, const char *url, fmpMediaInfo *media) {
	if (context == fpl_null || fplGetStringLength(url) == 0 || media == fpl_null)
		return fmpResult_InvalidArguments;
	if (!context->isValid)
		return fmpResult_ContextNotInitialized;

	fmpResult result = fmpResult_UnknownError;

	const fmpMemoryAllocator *allocator = &context->allocator;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	AVFormatContext *formatCtx;

	int openRes = -1;

	formatCtx = ffmpeg->avformat_alloc_context();
	if (formatCtx == fpl_null) {
		result = fmpResult_NotEnoughMemory;
		goto release;
	}

	openRes = ffmpeg->avformat_open_input(&formatCtx, url, fpl_null, fpl_null);
	if (openRes < 0) {
		result = fmpResult_MediaNotSupported;
		goto release;
	}

	int streamInfoRes = ffmpeg->avformat_find_stream_info(formatCtx, fpl_null);
	if (streamInfoRes < 0) {
		result = fmpResult_MediaNotSupported;
		goto release;
	}

	if (formatCtx->nb_streams > FMP_MAX_STREAM_COUNT) {
		result = fmpResult_TooManyStreams;
		goto release;
	}

	fplClearStruct(media);

	size_t urlLen = fplGetStringLength(url);
	media->url = __fmpCopyString(&context->allocator, url, urlLen);

	media->streamCount = formatCtx->nb_streams;

	for (uint32_t streamIndex = 0; streamIndex < media->streamCount; ++streamIndex) {
		const AVStream *st = formatCtx->streams[streamIndex];
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
				info->video.sampleAspectRatio = st->codecpar->sample_aspect_ratio;
				if (info->video.sampleAspectRatio.num) {
					ffmpeg->av_reduce(
						&info->video.displayAspectRatio.num,
						&info->video.displayAspectRatio.den,
						st->codecpar->width * (int64_t)info->video.sampleAspectRatio.num,
						st->codecpar->height * (int64_t)info->video.sampleAspectRatio.den,
						1024ULL * 1024ULL);
				}
				info->video.width = st->codecpar->width;
				info->video.height = st->codecpar->height;
			} break;

			case AVMEDIA_TYPE_AUDIO:
			{
				info->type = fmpStreamType_Audio;
				info->audio.channels = (uint32_t)st->codecpar->ch_layout.nb_channels;
				info->audio.sampleRate = (uint32_t)st->codecpar->sample_rate;
				info->audio.sampleFormat = (enum AVSampleFormat)st->codecpar->format;
				info->audio.bitrate = (enum AVSampleFormat)st->codecpar->bits_per_coded_sample;
			} break;

			case AVMEDIA_TYPE_SUBTITLE:
			{
				info->type = fmpStreamType_Subtitle;
			} break;

			default:
				break;
		}
}

	result = fmpResult_Success;

release:
	if (formatCtx != fpl_null) {
		if (openRes >= 0) {
			ffmpeg->avformat_close_input(&formatCtx);
		}
		ffmpeg->avformat_free_context(formatCtx);
	}
	if (!result) {
		fmpReleaseMediaInfo(context, media);
	}
	return result;
}

static void __fmpUnloadMediaContext(fmpMediaContext *media) {
	// @TODO(final): Proper unload code
}

static fmpResult __fmpLoadMediaIntoContext(fmpMediaContext *media, const char *pathOrURL) {

	fmpContext *context = media->context;

	// Unload media if needed
	fmpMediaState state = __fmpGetSafeMediaState(context);
	if (state > fmpMediaState_Unloaded) {
		__fmpUnloadMediaContext(media);
	}

	fplAssert(__fmpGetSafeMediaState(context) == fmpMediaState_Unloaded);

	fplClearStruct(media);
	media->context = context;

	__fmpSetSafeMediaState(context, fmpMediaState_Loading);

	fmpResult loadResult = fmpResult_Success;

	// @TODO(final): Proper load code

	if (loadResult == fmpResult_Success) {
		__fmpSetSafeMediaState(context, fmpMediaState_Loaded);
		return fmpResult_Success;
	} else {
		__fmpUnloadMediaContext(media);
		__fmpSetSafeMediaState(context, fmpMediaState_Error);
		return loadResult;
	}
}

FMP_API void fmpUnloadMedia(fmpContext *context) {
	if (context == fpl_null || !context->isValid) 
		return;

	fmpMediaState state = __fmpGetSafeMediaState(context);
	if (state <= fmpMediaState_Unloaded) 
		return;

	const FFMPEGContext *ffmpeg = &context->ffmpeg;

	fmpMediaContext *media = &context->media;

	__fmpUnloadMediaContext(media);

	fplClearStruct(media);
}

FMP_API fmpResult fmpLoadMediaByURL(fmpContext *context, const char *url) {
	if (context == fpl_null || fplGetStringLength(url) == 0) 
		return fmpResult_InvalidArguments;
	if (!context->isValid) 
		return fmpResult_ContextNotInitialized;
	fmpMediaContext *media = &context->media;
	fmpResult result = __fmpLoadMediaIntoContext(media, url);
	return result;
}

FMP_API fmpResult fmpLoadMediaByFile(fmpContext *context, const char *filePath) {
	if (context == fpl_null || fplGetStringLength(filePath) == 0) 
		return fmpResult_InvalidArguments;
	if (!context->isValid) 
		return fmpResult_ContextNotInitialized;
	if (!fplFileExists(filePath))
		return fmpResult_FileNotFound;
	fmpResult result = __fmpLoadMediaIntoContext(&context->media, filePath);
	return result;
}

FMP_API fmpMediaState fmpGetMediaState(fmpContext *context) {
	if (context == fpl_null || !context->isValid)
		return fmpMediaState_NotInitialized;
	fmpMediaState result = __fmpGetSafeMediaState(context);
	return result;
}

#endif // FMP_IMPLEMENTATION