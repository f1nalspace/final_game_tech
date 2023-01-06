#pragma once

#include <final_platform_layer.h>

#include "ffmpeg.h"
#include "packetqueue.h"

struct DecoderEx {
	fplConditionVariable emptyQueueCond;
	fplThreadHandle *thread;
	AVPacket *pendingPacket;
	PacketQueueEx *packetQueue;
	AVCodecContext *codecCtx;
	AVRational startPtsTimeBase;
	AVRational nextPtsTimeBase;
	int64_t startPts;
	int64_t nextPts;
	int32_t packetSerial;
	int32_t finished;
	int32_t packetPending;
};
