#pragma once

#include <final_platform_layer.h>

#include "ffmpeg.h"
#include "packetqueue.h"
#include "constants.h"

struct FrameEx {
	AVFrame *frame;
	AVSubtitle sub;
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