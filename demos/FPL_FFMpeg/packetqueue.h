#pragma once

#include <final_platform_layer.h>

#include "ffmpeg.h"

struct PacketListEx {
	AVPacket *pkt;
	int32_t serial;
};

struct PacketQueueEx {
	fplMutexHandle mutex;
	fplConditionVariable cond;
	AVFifo *packetList;
	uint64_t duration;
	uint32_t packetCount;
	uint32_t size;
	int32_t serial;
	int32_t abortRequest;
	int32_t isValid;
};

static int _PacketQueuePushLocal(PacketQueueEx &queue, AVPacket *pkt) {
	if (queue.abortRequest) {
		return -1;
	}

	PacketListEx entry = {};
	entry.pkt = pkt;
	entry.serial = queue.serial;

	int result = ffmpeg.av_fifo_write(queue.packetList, &entry, 1);
	if (result < 0) {
		return result;
	}

	queue.packetCount++;
	queue.size += entry.pkt->size + sizeof(entry);
	queue.duration += entry.pkt->duration;

	fplConditionSignal(&queue.cond);

	return 0;
}

static int PacketQueuePush(PacketQueueEx &queue, AVPacket *pkt) {
	fplAssertPtr(pkt);
	AVPacket *newPacket = ffmpeg.av_packet_alloc();
	if (newPacket == fpl_null) {
		ffmpeg.av_packet_unref(pkt);
		return -1;
	}
	ffmpeg.av_packet_move_ref(newPacket, pkt);

	int result;
	fplMutexLock(&queue.mutex);
	result = _PacketQueuePushLocal(queue, newPacket);
	fplMutexUnlock(&queue.mutex);

	if (result < 0) {
		ffmpeg.av_packet_free(&newPacket);
	}

	return result;
}

static int PacketQueuePushNullPacket(PacketQueueEx &queue, AVPacket *pkt, int streamIndex) {
	fplAssertPtr(pkt);
	pkt->stream_index = streamIndex;
	int result = PacketQueuePush(queue, pkt);
	return result;
}

static int PacketQueuePop(PacketQueueEx &queue, AVPacket *pkt, int block, int *serial) {
	int result;
	fplMutexLock(&queue.mutex);
	PacketListEx entry;
	for (;;) {
		if (queue.abortRequest) {
			result = -1;
			break;
		}

		if (ffmpeg.av_fifo_read(queue.packetList, &entry, 1) >= 0) {
			queue.packetCount--;
			queue.size -= entry.pkt->size + sizeof(entry);
			queue.duration -= entry.pkt->duration;
			ffmpeg.av_packet_move_ref(pkt, entry.pkt);
			if (serial != fpl_null) {
				*serial = entry.serial;
			}
			ffmpeg.av_packet_free(&entry.pkt);
			result = 1;
			break;
		} else if (!block) {
			result = 0;
			break;
		} else {
			fplConditionWait(&queue.cond, &queue.mutex, FPL_TIMEOUT_INFINITE);
		}
	}
	fplMutexUnlock(&queue.mutex);
	return result;
}

static void PacketQueueFlush(PacketQueueEx &queue) {
	PacketListEx entry;
	fplMutexLock(&queue.mutex);
	while (ffmpeg.av_fifo_read(queue.packetList, &entry, 1) >= 0) {
		ffmpeg.av_packet_free(&entry.pkt);
	}
	queue.packetCount = 0;
	queue.size = 0;
	queue.duration = 0;
	queue.serial++;
	fplMutexUnlock(&queue.mutex);
}

static void PacketQueueDestroy(PacketQueueEx &queue) {
	if (queue.isValid) {
		PacketQueueFlush(queue);
	}
	if (queue.cond.isValid) {
		fplConditionDestroy(&queue.cond);
	}
	if (queue.mutex.isValid) {
		fplMutexDestroy(&queue.mutex);
	}
	if (queue.packetList != fpl_null) {
		ffmpeg.av_fifo_freep2(&queue.packetList);
	}
	fplClearStruct(&queue);
}

static bool PacketQueueInit(PacketQueueEx &queue) {
	fplClearStruct(&queue);
	queue.packetList = ffmpeg.av_fifo_alloc2(1, sizeof(PacketListEx), AV_FIFO_FLAG_AUTO_GROW);
	if (queue.packetList == fpl_null) {
		goto failed;
	}
	if (!fplMutexInit(&queue.mutex))
	{
		goto failed;
	}
	if (!fplConditionInit(&queue.cond)) {
		goto failed;
	}
	queue.isValid = 1;
	return true;
failed:
	PacketQueueDestroy(queue);
	return false;
}

static void PacketQueueAbort(PacketQueueEx &queue) {
	fplMutexLock(&queue.mutex);
	queue.abortRequest = 1;
	fplConditionSignal(&queue.cond);
	fplMutexUnlock(&queue.mutex);
}

static void PacketQueueStart(PacketQueueEx &queue) {
	fplMutexLock(&queue.mutex);
	queue.abortRequest = 0;
	queue.serial++;
	fplMutexUnlock(&queue.mutex);
}