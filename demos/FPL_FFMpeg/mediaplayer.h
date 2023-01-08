#ifndef FMP_API
#define FMP_API

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
#if defined(FMP_PRIVATE)
#define FMP_API FMP_CEXTERN static
#else
#define FMP_API FMP_CEXTERN extern
#endif

#if defined(FMP_PRIVATE)
#define FFMPEG_PRIVATE
#endif
#include "ffmpeg_v2.h"

// Remove this!
#include "constants.h"

typedef struct PacketListEx {
	AVPacket *pkt;
	int32_t serial;
} PacketListEx;

typedef struct PacketQueueEx {
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
} PacketQueueEx;

typedef enum PacketQueueResult {
	PacketQueueResult_Error = -1,
	PacketQueueResult_Abort = -1,
	PacketQueueResult_Full = 0,
	PacketQueueResult_Success = 1,
} PacketQueueResult;

typedef struct FrameEx {
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
} FrameEx;

typedef struct FrameQueueEx {
	FrameEx queue[MAX_FRAME_QUEUE_COUNT];
	fplMutexHandle mutex;
	fplConditionVariable cond;
	FFMPEGContext *ffmpeg;
	PacketQueueEx *packetQueue;
	int32_t readIndex;
	int32_t writeIndex;
	int32_t size;
	int32_t maxSize;
	int32_t keepLast;
	int32_t readIndexShown;
	fpl_b32 isValid;
} FrameQueueEx;

typedef struct DecoderEx {
	FFMPEGContext *ffmpeg;
	fplConditionVariable *emptyQueueCondition;
	fplThreadHandle *thread;
	AVPacket *pendingPacket;
	PacketQueueEx *packetQueue;
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
} DecoderEx;

#endif // FMP_API

#if (defined(FMP_IMPLEMENTATION) && !defined(FMP_IMPLEMENTED)) || defined(FPL_IS_IDE)
#define FMP_IMPLEMENTED

#define FFMPEG_IMPLEMENTATION
#include "ffmpeg_v2.h"

//
// > Packet-Queue
//
static PacketQueueResult _PacketQueuePushLocal(PacketQueueEx *queue, AVPacket *pkt) {
	if (queue == fpl_null || !queue->isValid) return PacketQueueResult_Error;

	if (queue->abortRequest) return PacketQueueResult_Abort;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	PacketListEx entry = {};
	entry.pkt = pkt;
	entry.serial = queue->serial;

	int result = ffmpeg->av_fifo_write(queue->packetList, &entry, 1);
	if (result < 0) {
		return PacketQueueResult_Error;
	}

	queue->packetCount++;
	queue->size += entry.pkt->size + sizeof(entry);
	queue->duration += entry.pkt->duration;

	fplConditionSignal(&queue->cond);

	return PacketQueueResult_Success;
}

static PacketQueueResult PacketQueuePush(PacketQueueEx *queue, AVPacket *pkt) {
	if (queue == fpl_null || !queue->isValid) return PacketQueueResult_Error;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	fplAssertPtr(pkt);
	AVPacket *newPacket = ffmpeg->av_packet_alloc();
	if (newPacket == fpl_null) {
		ffmpeg->av_packet_unref(pkt);
		return PacketQueueResult_Error;
	}
	ffmpeg->av_packet_move_ref(newPacket, pkt);

	PacketQueueResult result;
	fplMutexLock(&queue->mutex);
	result = _PacketQueuePushLocal(queue, newPacket);
	fplMutexUnlock(&queue->mutex);

	if (result < 0) {
		ffmpeg->av_packet_free(&newPacket);
	}

	return result;
}

static PacketQueueResult PacketQueuePushNullPacket(PacketQueueEx *queue, AVPacket *pkt, int streamIndex) {
	if (queue == fpl_null || !queue->isValid) return PacketQueueResult_Error;
	fplAssertPtr(pkt);
	pkt->stream_index = streamIndex;
	PacketQueueResult result = PacketQueuePush(queue, pkt);
	return result;
}

static PacketQueueResult PacketQueuePop(PacketQueueEx *queue, AVPacket *pkt, int block, int *serial) {
	if (queue == fpl_null || !queue->isValid) return PacketQueueResult_Error;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	PacketQueueResult result;
	fplMutexLock(&queue->mutex);
	PacketListEx entry;
	for (;;) {
		if (queue->abortRequest) {
			result = PacketQueueResult_Abort;
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
			result = PacketQueueResult_Success;
			break;
		} else if (!block) {
			result = PacketQueueResult_Full;
			break;
		} else {
			fplConditionWait(&queue->cond, &queue->mutex, FPL_TIMEOUT_INFINITE);
		}
	}
	fplMutexUnlock(&queue->mutex);
	return result;
}

static void PacketQueueFlush(PacketQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return;

	FFMPEGContext *ffmpeg = queue->ffmpeg;

	PacketListEx entry;
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

static void PacketQueueDestroy(PacketQueueEx *queue) {
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

static bool PacketQueueInit(PacketQueueEx *queue, FFMPEGContext *ffmpeg) {
	if (queue == fpl_null || ffmpeg == fpl_null || !ffmpeg->isValid) return false;

	fplClearStruct(queue);

	queue->ffmpeg = ffmpeg;

	queue->packetList = ffmpeg->av_fifo_alloc2(1, sizeof(PacketListEx), AV_FIFO_FLAG_AUTO_GROW);
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

static void PacketQueueAbort(PacketQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 1;
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static void PacketQueueStart(PacketQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	queue->abortRequest = 0;
	queue->serial++;
	fplMutexUnlock(&queue->mutex);
}

//
// > Frame-Queue
//

static void FrameQueueUnref(FFMPEGContext *ffmpeg, FrameEx *frame) {
	fplAssertPtr(ffmpeg);
	fplAssertPtr(frame);
	ffmpeg->av_frame_unref(frame->frame);
	ffmpeg->avsubtitle_free(&frame->subtitle);
}

static void FrameQueueDestroy(FrameQueueEx *queue) {
	if (queue == fpl_null) return;
	FFMPEGContext *ffmpeg = queue->ffmpeg;
	if (queue->isValid) {
		for (int32_t i = 0; i < queue->maxSize; ++i) {
			FrameEx *frame = queue->queue + i;
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

static bool FrameQueueInit(FFMPEGContext *ffmpeg, FrameQueueEx *frameQueue, PacketQueueEx *packetQueue, int32_t maxSize, int32_t keepLast) {
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

	frameQueue->maxSize = fplMin(maxSize, MAX_FRAME_QUEUE_COUNT);
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

static void FrameQueueSignal(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return;
	fplMutexLock(&queue->mutex);
	fplConditionSignal(&queue->cond);
	fplMutexUnlock(&queue->mutex);
}

static FrameEx *FrameQueuePeek(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown) % queue->maxSize;
	return &queue->queue[index];
}

static FrameEx *FrameQueuePeekNext(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = (queue->readIndex + queue->readIndexShown + 1) % queue->maxSize;
	return &queue->queue[index];
}

static FrameEx *FrameQueuePeekLast(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return fpl_null;
	uint32_t index = queue->readIndex;
	return &queue->queue[index];
}

static FrameEx *FrameQueuePeekWritable(FrameQueueEx *queue) {
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

static FrameEx *FrameQueuePeekReadable(FrameQueueEx *queue) {
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

static void FrameQueuePush(FrameQueueEx *queue) {
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

static int32_t GetFrameQueueRemainingCount(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return 0;
	int32_t result = queue->size - queue->readIndexShown;
	return result;
}

static int64_t GetFrameQueueLastPos(FrameQueueEx *queue) {
	if (queue == fpl_null || !queue->isValid) return -1;
	FrameEx *frame = &queue->queue[queue->readIndex];
	if (queue->readIndexShown && frame->serial == queue->packetQueue->serial)
		return frame->pos;
	else
		return -1;
}

//
// > Decoder
//

static void DecoderDestroy(DecoderEx *decoder) {
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

static bool DecoderInit(FFMPEGContext *ffmpeg, DecoderEx *decoder, AVCodecContext *codecCtx, PacketQueueEx *packetQueue, fplConditionVariable *emptyQueueCondition) {
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

static void DecoderAbort(DecoderEx *decoder, FrameQueueEx *frameQueue) {
	if (decoder == fpl_null || !decoder->isValid) return;
	PacketQueueAbort(decoder->packetQueue);
	FrameQueueSignal(frameQueue);
	fplThreadWaitForOne(decoder->thread, FPL_TIMEOUT_INFINITE);
	decoder->thread = fpl_null;
	PacketQueueFlush(decoder->packetQueue);
}

static int DecoderDecodeFrame(DecoderEx *decoder, AVFrame *frame, AVSubtitle *subtitle) {
	if (decoder == fpl_null || !decoder->isValid) return -1;
	int ret = AVERROR(EAGAIN);
	FFMPEGContext *ffmpeg = decoder->ffmpeg;
	AVCodecContext *codecCtx = decoder->codecCtx;
	PacketQueueEx *queue = decoder->packetQueue;
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

#endif // FMP_IMPLEMENTATION