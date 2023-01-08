#pragma once

#include <final_platform_layer.h>

#include "ffmpeg.h"
#include "packetqueue.h"
#include "framequeue.h"

struct DecoderEx {
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
};

static void DecoderDestroy(DecoderEx &decoder) {
    ffmpeg.av_packet_free(&decoder.pendingPacket);
    ffmpeg.avcodec_free_context(&decoder.codecCtx);
    fplClearStruct(&decoder);
}

static bool DecoderInit(DecoderEx &decoder, AVCodecContext *codecCtx, PacketQueueEx *packetQueue, fplConditionVariable *emptyQueueCondition) {
	fplAssertPtr(codecCtx);
	fplAssertPtr(packetQueue);
	fplAssertPtr(emptyQueueCondition);
	fplClearStruct(&decoder);
	decoder.pendingPacket = ffmpeg.av_packet_alloc();
    if (decoder.pendingPacket == fpl_null) {
        return false;
    }
	decoder.codecCtx = codecCtx;
	decoder.packetQueue = packetQueue;
	decoder.emptyQueueCondition = emptyQueueCondition;
	decoder.startPts = AV_NOPTS_VALUE;
	decoder.packetSerial = -1;
    decoder.reorderPts = -1;
	return true;
}

static void DecoderAbort(DecoderEx &decoder, FrameQueueEx &frameQueue) {
    PacketQueueAbort(*decoder.packetQueue);
    FrameQueueSignal(frameQueue);
    fplThreadWaitForOne(decoder.thread, FPL_TIMEOUT_INFINITE);
    decoder.thread = fpl_null;
    PacketQueueFlush(*decoder.packetQueue);
}

static int DecoderDecodeFrame(DecoderEx &decoder, AVFrame *frame, AVSubtitle *subtitle) {
	int ret = AVERROR(EAGAIN);
    AVCodecContext *codecCtx = decoder.codecCtx;
    PacketQueueEx *queue = decoder.packetQueue;
    AVPacket *packet = decoder.pendingPacket;
    for (;;) {
        if (queue->serial == decoder.packetSerial) {
            do {
                if (queue->abortRequest) {
                    return -1;
                }
                switch (codecCtx->codec_type) {
                    case AVMEDIA_TYPE_VIDEO:
                        ret = ffmpeg.avcodec_receive_frame(codecCtx, frame);
                        if (ret >= 0) {
                            if (decoder.reorderPts == -1) {
                                frame->pts = frame->best_effort_timestamp;
                            } else if (!decoder.reorderPts) {
                                frame->pts = frame->pkt_dts;
                            }
                        }
                        break;
                    case AVMEDIA_TYPE_AUDIO:
                        ret = ffmpeg.avcodec_receive_frame(codecCtx, frame);
                        if (ret >= 0) {
                            AVRational tb = fplStructInit(AVRational, 1, frame->sample_rate);
                            if (frame->pts != AV_NOPTS_VALUE)
                                frame->pts = av_rescale_q(frame->pts, codecCtx->pkt_timebase, tb);
                            else if (decoder.nextPts != AV_NOPTS_VALUE)
                                frame->pts = av_rescale_q(decoder.nextPts, decoder.nextPtsTimeBase, tb);
                            if (frame->pts != AV_NOPTS_VALUE) {
                                decoder.nextPts = frame->pts + frame->nb_samples;
                                decoder.nextPtsTimeBase = tb;
                            }
                        }
                        break;
                }
                if (ret == AVERROR_EOF) {
                    decoder.finishedSerial = decoder.packetSerial;
                    ffmpeg.avcodec_flush_buffers(codecCtx);
                    return 0;
                }
                if (ret >= 0) {
                    return 1;
                }
            } while (ret != AVERROR(EAGAIN));
        }

        do {
            if (queue->packetCount == 0) {
                fplConditionSignal(decoder.emptyQueueCondition);
            }
            if (decoder.isPacketPending) {
                decoder.isPacketPending = 0;
            } else {
                int32_t old_serial = decoder.packetSerial;
                if (PacketQueuePop(*queue, packet, 1, &decoder.packetSerial) < 0) {
                    return -1;
                }
                if (old_serial != decoder.packetSerial) {
                    ffmpeg.avcodec_flush_buffers(codecCtx);
                    decoder.finishedSerial = 0;
                    decoder.nextPts = decoder.startPts;
                    decoder.nextPtsTimeBase = decoder.startPtsTimeBase;
                }
            }
            if (queue->serial == decoder.packetSerial) {
                break;
            }
            av_packet_unref(packet);
        } while (1);

        if (codecCtx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            int gotFrame = 0;
            ret = ffmpeg.avcodec_decode_subtitle2(codecCtx, subtitle, &gotFrame, packet);
            if (ret < 0) {
                ret = AVERROR(EAGAIN);
            } else {
                if (gotFrame && !packet->data) {
                    decoder.isPacketPending = 1;
                }
                ret = gotFrame ? 0 : (packet->data ? AVERROR(EAGAIN) : AVERROR_EOF);
            }
            ffmpeg.av_packet_unref(packet);
        } else {
            if (ffmpeg.avcodec_send_packet(codecCtx, packet) == AVERROR(EAGAIN)) {
                decoder.isPacketPending = 1;
            } else {
                ffmpeg.av_packet_unref(packet);
            }
        }
    }
}