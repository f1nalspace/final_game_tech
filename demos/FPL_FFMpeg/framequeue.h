#pragma once

#include <final_platform_layer.h>

#include "ffmpeg.h"
#include "packetqueue.h"
#include "constants.h"

struct FrameEx {
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
};

struct FrameQueueEx {
	FrameEx queue[MAX_FRAME_QUEUE_COUNT];
	fplMutexHandle mutex;
	fplConditionVariable cond;
	PacketQueueEx *packetQueue;
	int32_t readIndex;
	int32_t writeIndex;
	int32_t size;
	int32_t maxSize;
	int32_t keepLast;
	int32_t readIndexShown;
};

static void FrameQueueUnref(FrameEx &frame) {
	ffmpeg.av_frame_unref(frame.frame);
	ffmpeg.avsubtitle_free(&frame.subtitle);
}

static void FrameQueueDestroy(FrameQueueEx &frameQueue) {
	for (int32_t i = 0; i < frameQueue.maxSize; ++i) {
		FrameEx *frame = frameQueue.queue + i;
		if (frame->frame != fpl_null) {
			FrameQueueUnref(*frame);
			ffmpeg.av_frame_free(&frame->frame);
		}
	}
	fplConditionDestroy(&frameQueue.cond);
	fplMutexDestroy(&frameQueue.mutex);
	fplClearStruct(&frameQueue);
}

static bool FrameQueueInit(FrameQueueEx &frameQueue, PacketQueueEx *packetQueue, int32_t maxSize, int32_t keepLast) {
	fplClearStruct(&frameQueue);
	if (!fplMutexInit(&frameQueue.mutex)) {
		goto failed;
	}
	if (!fplConditionInit(&frameQueue.cond)) {
		goto failed;
	}
	frameQueue.packetQueue = packetQueue;
	frameQueue.maxSize = fplMin(maxSize, MAX_FRAME_QUEUE_COUNT);
	frameQueue.keepLast = !!keepLast;
	for (int32_t i = 0; i < frameQueue.maxSize; ++i) {
		if (!(frameQueue.queue[i].frame = ffmpeg.av_frame_alloc())) {
			goto failed;
		}
	}
	return true;
failed:
	FrameQueueDestroy(frameQueue);
	return false;
}

static void FrameQueueSignal(FrameQueueEx &frameQueue) {
	fplMutexLock(&frameQueue.mutex);
	fplConditionSignal(&frameQueue.cond);
	fplMutexUnlock(&frameQueue.mutex);
}

static FrameEx *FrameQueuePeek(FrameQueueEx &frameQueue) {
	return &frameQueue.queue[(frameQueue.readIndex + frameQueue.readIndexShown) % frameQueue.maxSize];
}

static FrameEx *FrameQueuePeekNext(FrameQueueEx &frameQueue) {
	return &frameQueue.queue[(frameQueue.readIndex + frameQueue.readIndexShown + 1) % frameQueue.maxSize];
}

static FrameEx *FrameQueuePeekLast(FrameQueueEx &frameQueue) {
	return &frameQueue.queue[frameQueue.readIndex];
}

static FrameEx *FrameQueuePeekWritable(FrameQueueEx &frameQueue) {
	fplMutexLock(&frameQueue.mutex);
	while (frameQueue.size >= frameQueue.maxSize && !frameQueue.packetQueue->abortRequest) {
		fplConditionWait(&frameQueue.cond, &frameQueue.mutex, FPL_TIMEOUT_INFINITE);
	}
	fplMutexUnlock(&frameQueue.mutex);

	if (frameQueue.packetQueue->abortRequest) {
		return fpl_null;
	}

	return &frameQueue.queue[frameQueue.writeIndex];
}

static FrameEx *FrameQueuePeekReadable(FrameQueueEx &frameQueue) {
	fplMutexLock(&frameQueue.mutex);
	while (frameQueue.size - frameQueue.readIndexShown <= 0 && !frameQueue.packetQueue->abortRequest) {
		fplConditionWait(&frameQueue.cond, &frameQueue.mutex, FPL_TIMEOUT_INFINITE);
	}
	fplMutexUnlock(&frameQueue.mutex);

	if (frameQueue.packetQueue->abortRequest) {
		return fpl_null;
	}

	return &frameQueue.queue[(frameQueue.readIndex + frameQueue.readIndexShown) % frameQueue.maxSize];
}

static void FrameQueuePush(FrameQueueEx &frameQueue) {
	if (frameQueue.keepLast && !frameQueue.readIndexShown) {
        frameQueue.readIndexShown = 1;
        return;
    }

    FrameQueueUnref(frameQueue.queue[frameQueue.readIndex]);
	if (++frameQueue.readIndex == frameQueue.maxSize) {
		frameQueue.readIndex = 0;
	}

    fplMutexLock(&frameQueue.mutex);
    frameQueue.size--;
    fplConditionSignal(&frameQueue.cond);
    fplMutexUnlock(&frameQueue.mutex);
}

static int GetFrameQueueRemainingCount(FrameQueueEx &frameQueue) {
	return frameQueue.size - frameQueue.readIndexShown;
}

static int64_t GetFrameQueueLastPos(FrameQueueEx &frameQueue) {
	FrameEx *frame = &frameQueue.queue[frameQueue.readIndex];
    if (frameQueue.readIndexShown && frame->serial == frameQueue.packetQueue->serial)
        return frame->pos;
    else
        return -1;
}