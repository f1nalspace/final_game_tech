#pragma once

#include "utils.h"

//
// FFMPEG headers and function prototypes
//
extern "C" {
#	include <libavutil/avutil.h>
#	include <libavutil/imgutils.h>
#	include <libavutil/time.h>
#	include <libavutil/version.h>
#	include <libavutil/fifo.h>
#	include <libavcodec/avcodec.h>
#	include <libavcodec/avfft.h>
#	include <libavformat/avformat.h>
#	include <libswscale/swscale.h>
#	include <libswresample/swresample.h>
}

//
// FFMPEG function macros for 5.1.2:
// 
// FFmpeg are based on modules, each module have its own _version() function.
// The signature are identical, so we define one function signature here and use it everywhere
//
#define FFMPEG_GET_LIB_VERSION_FUNC(name) unsigned name(void)
typedef FFMPEG_GET_LIB_VERSION_FUNC(ffmpeg_get_lib_version_func);

//
// AVFormat
//

// avformat_network_init
#define FFMPEG_AVFORMAT_NETWORK_INIT_FUNC(name) int name(void)
typedef FFMPEG_AVFORMAT_NETWORK_INIT_FUNC(ffmpeg_avformat_network_init_func);
// avformat_network_deinit
#define FFMPEG_AVFORMAT_NETWORK_DEINIT_FUNC(name) int name(void)
typedef FFMPEG_AVFORMAT_NETWORK_DEINIT_FUNC(ffmpeg_avformat_network_deinit_func);
// avformat_close_input
#define FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(name) void name(struct AVFormatContext **s)
typedef FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(ffmpeg_avformat_close_input_func);
// avformat_open_input
#define FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(name) int name(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options)
typedef FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(ffmpeg_avformat_open_input_func);
// avformat_find_stream_info
#define FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(name) int name(struct AVFormatContext *ic, struct AVDictionary **options)
typedef FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(ffmpeg_avformat_find_stream_info_func);
// av_dump_format
#define FFMPEG_AV_DUMP_FORMAT_FUNC(name) void name(struct AVFormatContext *ic, int index, const char *url, int is_output)
typedef FFMPEG_AV_DUMP_FORMAT_FUNC(ffmpeg_av_dump_format_func);
// av_read_frame
#define FFMPEG_AV_READ_FRAME_FUNC(name) int name(struct AVFormatContext *s, struct AVPacket *pkt)
typedef FFMPEG_AV_READ_FRAME_FUNC(ffmpeg_av_read_frame_func);
// avformat_alloc_context
#define FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(name) struct AVFormatContext *name(void)
typedef FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(ffmpeg_avformat_alloc_context_func);
// avformat_seek_file
#define FFMPEG_AVFORMAT_SEEK_FILE_FUNC(name) int name(struct AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
typedef FFMPEG_AVFORMAT_SEEK_FILE_FUNC(ffmpeg_avformat_seek_file_func);
// avformat_match_stream_specifier
#define FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(name) int name(struct AVFormatContext *s, struct AVStream *st, const char *spec)
typedef FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(ffmpeg_avformat_match_stream_specifier_func);
// av_find_best_stream
#define FFMPEG_AV_FIND_BEST_STREAM_FUNC(name) int name(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, const AVCodec **decoder_ret, int flags)
typedef FFMPEG_AV_FIND_BEST_STREAM_FUNC(ffmpeg_av_find_best_stream_func);
// av_guess_sample_aspect_ratio
#define FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(name) struct AVRational name(struct AVFormatContext *format, struct AVStream *stream, struct AVFrame *frame)
typedef FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(ffmpeg_av_guess_sample_aspect_ratio_func);
// av_guess_frame_rate
#define FFMPEG_AV_GUESS_FRAME_RATE_FUNC(name) struct AVRational name(struct AVFormatContext *ctx, struct AVStream *stream, struct AVFrame *frame)
typedef FFMPEG_AV_GUESS_FRAME_RATE_FUNC(ffmpeg_av_guess_frame_rate_func);
// av_read_pause
#define FFMPEG_AV_READ_PAUSE_FUNC(name) int name(struct AVFormatContext *s)
typedef FFMPEG_AV_READ_PAUSE_FUNC(ffmpeg_av_read_pause_func);
// av_read_play
#define FFMPEG_AV_READ_PLAY_FUNC(name) int name(struct AVFormatContext *s)
typedef FFMPEG_AV_READ_PLAY_FUNC(ffmpeg_av_read_play_func);
// avio_feof
#define FFMPEG_AVIO_FEOF_FUNC(name) int name(struct AVIOContext *s)
typedef FFMPEG_AVIO_FEOF_FUNC(ffmpeg_avio_feof_func);
// av_find_program_from_stream
#define FFMPEG_AV_FIND_PROGRAM_FROM_STREAM_FUNC(name) AVProgram *name(AVFormatContext *ic, AVProgram *last, int s)
typedef FFMPEG_AV_FIND_PROGRAM_FROM_STREAM_FUNC(ffmpeg_av_find_program_from_stream_func);
// av_format_inject_global_side_data
#define FFMPEG_AVFORMAT_INJECT_GLOBAL_SIDE_DATA_FUNC(name) void name(AVFormatContext *s)
typedef FFMPEG_AVFORMAT_INJECT_GLOBAL_SIDE_DATA_FUNC(ffmpeg_av_format_inject_global_side_data_func);
// avio_size
#define FFMPEG_AVIO_SIZE_FUNC(name) int64_t name(AVIOContext *s)
typedef FFMPEG_AVIO_SIZE_FUNC(ffmpeg_avio_size_func);
// avio_seek
#define FFMPEG_AVIO_SEEK_FUNC(name) int64_t name(AVIOContext *s, int64_t offset, int whence)
typedef FFMPEG_AVIO_SEEK_FUNC(ffmpeg_avio_seek_func);



//
// AVCodec
//

// avcodec_free_context
#define FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(name) void name(struct AVCodecContext **avctx)
typedef FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(ffmpeg_avcodec_free_context_func);
// avcodec_alloc_context3
#define FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(name) AVCodecContext *name(const struct AVCodec *codec)
typedef FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(ffmpeg_avcodec_alloc_context3_func);
// avcodec_parameters_to_context
#define FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(name) int name(struct AVCodecContext *codec, const struct AVCodecParameters *par)
typedef FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(ffmpeg_avcodec_parameters_to_context_func);
// avcodec_find_decoder
#define FFMPEG_AVCODEC_FIND_DECODER_FUNC(name) const AVCodec *name(enum AVCodecID id)
typedef FFMPEG_AVCODEC_FIND_DECODER_FUNC(ffmpeg_avcodec_find_decoder_func);
// avcodec_open2
#define FFMPEG_AVCODEC_OPEN2_FUNC(name) int name(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
typedef FFMPEG_AVCODEC_OPEN2_FUNC(ffmpeg_avcodec_open2_func);
// av_packet_unref
#define FFMPEG_AV_PACKET_UNREF_FUNC(name) void name(AVPacket *pkt)
typedef FFMPEG_AV_PACKET_UNREF_FUNC(ffmpeg_av_packet_unref_func);
// avcodec_receive_frame
#define FFMPEG_AVCODEC_RECEIVE_FRAME_FUNC(name) int name(AVCodecContext *avctx, AVFrame *frame)
typedef FFMPEG_AVCODEC_RECEIVE_FRAME_FUNC(ffmpeg_avcodec_receive_frame_func);
// avcodec_send_packet
#define FFMPEG_AVCODEC_SEND_PACKET_FUNC(name) int name(AVCodecContext *avctx, const AVPacket *avpkt)
typedef FFMPEG_AVCODEC_SEND_PACKET_FUNC(ffmpeg_avcodec_send_packet_func);
// av_packet_alloc
#define FFMPEG_AV_PACKET_ALLOC_FUNC(name) AVPacket *name(void)
typedef FFMPEG_AV_PACKET_ALLOC_FUNC(ffmpeg_av_packet_alloc_func);
// av_packet_free
#define FFMPEG_AV_PACKET_FREE_FUNC(name) void name(AVPacket **pkt)
typedef FFMPEG_AV_PACKET_FREE_FUNC(ffmpeg_av_packet_free_func);
// av_new_packet
#define FFMPEG_AV_NEW_PACKET_FUNC(name) int name(AVPacket *pkt, int size)
typedef FFMPEG_AV_NEW_PACKET_FUNC(ffmpeg_av_new_packet_func);
// avcodec_find_decoder_by_name
#define FFMPEG_AVCODEC_FIND_DECODER_BY_NAME_FUNC(name) const AVCodec *name(const char *cname)
typedef FFMPEG_AVCODEC_FIND_DECODER_BY_NAME_FUNC(ffmpeg_avcodec_find_decoder_by_name_func);
// avsubtitle_free
#define FFMPEG_AVSUBTITLE_FREE_FUNC(name) void name(AVSubtitle *sub)
typedef FFMPEG_AVSUBTITLE_FREE_FUNC(ffmpeg_avsubtitle_free_func);
// av_packet_move_ref
#define FFMPEG_AV_PACKET_MOVE_REF_FUNC(name) void name(AVPacket *dst, AVPacket *src)
typedef FFMPEG_AV_PACKET_MOVE_REF_FUNC(ffmpeg_av_packet_move_ref_func);
// av_packet_ref
#define FFMPEG_AV_PACKET_REF_FUNC(name) int name(AVPacket *dst, const AVPacket *src)
typedef FFMPEG_AV_PACKET_REF_FUNC(ffmpeg_av_packet_ref_func);
// avcodec_flush_buffers
#define FFMPEG_AVCODEC_FLUSH_BUFFERS_FUNC(name) void name(AVCodecContext *avctx)
typedef FFMPEG_AVCODEC_FLUSH_BUFFERS_FUNC(ffmpeg_avcodec_flush_buffers_func);
// avcodec_decode_subtitle2
#define FFMPEG_AVCODEC_DECODE_SUBTITLE2_FUNC(name) int name(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt)
typedef FFMPEG_AVCODEC_DECODE_SUBTITLE2_FUNC(ffmpeg_avcodec_decode_subtitle2_func);

// av_rdft_init
#define FFMPEG_AV_RDFT_INIT_FUNC(name) RDFTContext *name(int nbits, enum RDFTransformType trans)
typedef FFMPEG_AV_RDFT_INIT_FUNC(ffmpeg_av_rdft_init_func);
// av_rdft_calc
#define FFMPEG_AV_RDFT_CALC_FUNC(name) void name(RDFTContext *s, FFTSample *data)
typedef FFMPEG_AV_RDFT_CALC_FUNC(ffmpeg_av_rdft_calc_func);
// av_rdft_end
#define FFMPEG_AV_RDFT_END_FUNC(name) void name(RDFTContext *s)
typedef FFMPEG_AV_RDFT_END_FUNC(ffmpeg_av_rdft_end_func);

//
// AVUtil
//

// av_frame_alloc
#define FFMPEG_AV_FRAME_ALLOC_FUNC(name) AVFrame *name(void)
typedef FFMPEG_AV_FRAME_ALLOC_FUNC(ffmpeg_av_frame_alloc_func);
// av_frame_free
#define FFMPEG_AV_FRAME_FREE_FUNC(name) void name(AVFrame **frame)
typedef FFMPEG_AV_FRAME_FREE_FUNC(ffmpeg_av_frame_free_func);
// av_frame_unref
#define FFMPEG_AV_FRAME_UNREF_FUNC(name) void name(AVFrame *frame)
typedef FFMPEG_AV_FRAME_UNREF_FUNC(ffmpeg_av_frame_unref_func);
// av_frame_move_ref
#define FFMPEG_AV_FRAME_MOVE_REF_FUNC(name) void name(AVFrame *dst, AVFrame *src)
typedef FFMPEG_AV_FRAME_MOVE_REF_FUNC(ffmpeg_av_frame_move_ref_func);
// av_image_get_buffer_size
#define FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(ffmpeg_av_image_get_buffer_size_func);
// av_image_get_linesize
#define FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int plane)
typedef FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(ffmpeg_av_image_get_linesize_func);
// av_image_fill_arrays
#define FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(name) int name(uint8_t *dst_data[4], int dst_linesize[4], const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(ffmpeg_av_image_fill_arrays_func);
// av_gettime_relative
#define FFMPEG_AV_GETTIME_RELATIVE_FUNC(name) int64_t name(void)
typedef FFMPEG_AV_GETTIME_RELATIVE_FUNC(ffmpeg_av_gettime_relative_func);
// av_gettime
#define FFMPEG_AV_GETTIME_FUNC(name) int64_t name(void)
typedef FFMPEG_AV_GETTIME_FUNC(ffmpeg_av_gettime_func);
// av_get_media_type_string
#define FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(name) const char *name(enum AVMediaType media_type)
typedef FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(ffmpeg_av_get_media_type_string_func);
// av_rescale_q
#define FFMPEG_AV_RESCALE_Q_FUNC(name) int64_t name(int64_t a, AVRational bq, AVRational cq) av_const
typedef FFMPEG_AV_RESCALE_Q_FUNC(ffmpeg_av_rescale_q_func);
// av_samples_get_buffer_size
#define FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(name) int name(int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
typedef FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(ffmpeg_av_samples_get_buffer_size_func);
// av_malloc
#define FFMPEG_AV_MALLOC_FUNC(name) void *name(size_t size) av_malloc_attrib av_alloc_size(1)
typedef FFMPEG_AV_MALLOC_FUNC(ffmpeg_av_malloc_func);
// av_mallocz
#define FFMPEG_AV_MALLOCZ_FUNC(name) void *name(size_t size) av_malloc_attrib av_alloc_size(1)
typedef FFMPEG_AV_MALLOCZ_FUNC(ffmpeg_av_mallocz_func);
// av_fast_malloc
#define FFMPEG_AV_FAST_MALLOC_FUNC(name) void name(void *ptr, unsigned int *size, size_t min_size)
typedef FFMPEG_AV_FAST_MALLOC_FUNC(ffmpeg_av_fast_malloc_func);
// av_free
#define FFMPEG_AV_FREE_FUNC(name) void name(void *ptr)
typedef FFMPEG_AV_FREE_FUNC(ffmpeg_av_free_func);
// av_freep
#define FFMPEG_AV_FREEP_FUNC(name) void name(void *ptr)
typedef FFMPEG_AV_FREEP_FUNC(ffmpeg_av_freep_func);
// av_get_packed_sample_fmt
#define FFMPEG_AV_GET_PACKED_SAMPLE_FMT_FUNC(name) enum AVSampleFormat name(enum AVSampleFormat sample_fmt)
typedef FFMPEG_AV_GET_PACKED_SAMPLE_FMT_FUNC(ffmpeg_av_get_packed_sample_fmt_func);
// av_channel_layout_default
#define FFMPEG_AV_CHANNEL_LAYOUT_DEFAULT_FUNC(name) void name(AVChannelLayout *ch_layout, int nb_channels)
typedef FFMPEG_AV_CHANNEL_LAYOUT_DEFAULT_FUNC(ffmpeg_av_channel_layout_default_func);
// av_usleep
#define FFMPEG_AV_USLEEP_FUNC(name) int name(unsigned usec)
typedef FFMPEG_AV_USLEEP_FUNC(ffmpeg_av_usleep_func);
// av_strdup
#define FFMPEG_AV_STRDUP_FUNC(name) char *name(const char *s)
typedef FFMPEG_AV_STRDUP_FUNC(ffmpeg_av_strdup_func);
// av_log2
#define FFMPEG_AV_LOG2_FUNC(name) av_const int name(unsigned v)
typedef FFMPEG_AV_LOG2_FUNC(ffmpeg_av_log2_func);
// av_compare_ts
#define FFMPEG_AV_COMPARE_TS_FUNC(name) int name(int64_t ts_a, AVRational tb_a, int64_t ts_b, AVRational tb_b)
typedef FFMPEG_AV_COMPARE_TS_FUNC(ffmpeg_av_compare_ts_func);
// av_get_bytes_per_sample
#define FFMPEG_AV_GET_BYTES_PER_SAMPLE_FUNC(name) int name(enum AVSampleFormat sample_fmt)
typedef FFMPEG_AV_GET_BYTES_PER_SAMPLE_FUNC(ffmpeg_av_get_bytes_per_sample_func);
// av_get_sample_fmt_name
#define FFMPEG_AV_GET_SAMPLE_FMT_NAME_FUNC(name) const char *name(enum AVSampleFormat sample_fmt)
typedef FFMPEG_AV_GET_SAMPLE_FMT_NAME_FUNC(ffmpeg_av_get_sample_fmt_name_func);
// av_log_set_flags
#define FFMPEG_AV_LOG_SET_FLAGS_FUNC(name) void name(int arg)
typedef FFMPEG_AV_LOG_SET_FLAGS_FUNC(ffmpeg_av_log_set_flags_func);
// av_log
#define FFMPEG_AV_LOG_FUNC(name) void name(void *avcl, int level, const char *fmt, ...)
typedef FFMPEG_AV_LOG_FUNC(ffmpeg_av_log_func);
// av_get_pix_fmt_string
#define FFMPEG_AV_GET_PIX_FMT_STRING_FUNC(name) char* name(char* buf, int buf_size, enum AVPixelFormat pix_fmt)
typedef FFMPEG_AV_GET_PIX_FMT_STRING_FUNC(ffmpeg_av_get_pix_fmt_string_func);
// av_get_pix_fmt_name
#define FFMPEG_AV_GET_PIX_FMT_NAME_FUNC(name) const char* name(enum AVPixelFormat pix_fmt)
typedef FFMPEG_AV_GET_PIX_FMT_NAME_FUNC(ffmpeg_av_get_pix_fmt_name_func);
// av_fifo_write
#define FFMPEG_AV_FIFO_WRITE_FUNC(name) int name(AVFifo *f, const void *buf, size_t nb_elems)
typedef FFMPEG_AV_FIFO_WRITE_FUNC(ffmpeg_av_fifo_write_func);
// av_fifo_alloc2
#define FFMPEG_AV_FIFO_ALLOC2_FUNC(name) AVFifo *name(size_t elems, size_t elem_size, unsigned int flags)
typedef FFMPEG_AV_FIFO_ALLOC2_FUNC(ffmpeg_av_fifo_alloc2_func);
// av_fifo_read
#define FFMPEG_AV_FIFO_READ_FUNC(name) int name(AVFifo *f, void *buf, size_t nb_elems)
typedef FFMPEG_AV_FIFO_READ_FUNC(ffmpeg_av_fifo_read_func);
// av_fifo_freep2
#define FFMPEG_AV_FIFO_FREEP2_FUNC(name) void name(AVFifo **f)
typedef FFMPEG_AV_FIFO_FREEP2_FUNC(ffmpeg_av_fifo_freep2_func);

//
// SWS
//

// sws_getContext
#define FFMPEG_SWS_GET_CONTEXT_FUNC(name) struct SwsContext *name(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, struct SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CONTEXT_FUNC(ffmpeg_sws_getContext_func);
// sws_getCachedContext
#define FFMPEG_SWS_GET_CACHED_CONTEXT_FUNC(name) struct SwsContext *name(struct SwsContext *context, int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, struct SwsFilter *srcFilter, struct SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CACHED_CONTEXT_FUNC(ffmpeg_sws_getCachedContext_func);

// sws_scale
#define FFMPEG_SWS_SCALE_FUNC(name) int name(struct SwsContext *c, const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[])
typedef FFMPEG_SWS_SCALE_FUNC(ffmpeg_sws_scale_func);
// sws_freeContext
#define FFMPEG_SWS_FREE_CONTEXT_FUNC(name) void name(struct SwsContext *swsContext)
typedef FFMPEG_SWS_FREE_CONTEXT_FUNC(ffmpeg_sws_freeContext_func);

//
// SWR
//

// swr_alloc_set_opts_func
#define FFMPEG_SWR_ALLOC_SET_OPTS2(name) int name(struct SwrContext **ps, AVChannelLayout *out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, AVChannelLayout *in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate, int log_offset, void *log_ctx)
typedef FFMPEG_SWR_ALLOC_SET_OPTS2(ffmpeg_swr_alloc_set_opts2_func);
// swr_free
#define FFMPEG_SWR_FREE(name) void name(struct SwrContext **s)
typedef FFMPEG_SWR_FREE(ffmpeg_swr_free_func);
// swr_convert
#define FFMPEG_SWR_CONVERT(name) int name(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count)
typedef FFMPEG_SWR_CONVERT(ffmpeg_swr_convert_func);
// swr_init
#define FFMPEG_SWR_INIT(name) int name(struct SwrContext *s)
typedef FFMPEG_SWR_INIT(ffmpeg_swr_init_func);
// swr_set_compensation
#define FFMPEG_SWR_SET_COMPENSATION(name) int name(struct SwrContext *s, int sample_delta, int compensation_distance)
typedef FFMPEG_SWR_SET_COMPENSATION(ffmpeg_swr_set_compensation_func);

#define FFMPEG_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)fplGetDynamicLibraryProc(&libHandle, name); \
	if (target == nullptr) { \
		FPL_LOG_ERROR("FFMPEG", "Failed getting procedure '%s' from library '%s'!", name, libName); \
		return false; \
	}

struct FFMPEGContext {
	fplDynamicLibraryHandle avFormatLib;
	fplDynamicLibraryHandle avCodecLib;
	fplDynamicLibraryHandle avUtilLib;
	fplDynamicLibraryHandle swScaleLib;
	fplDynamicLibraryHandle swResampleLib;

	// Format
	ffmpeg_get_lib_version_func* avformat_version;
	ffmpeg_avformat_network_init_func* avformat_network_init;
	ffmpeg_avformat_network_deinit_func* avformat_network_deinit;
	ffmpeg_avformat_close_input_func* avformat_close_input;
	ffmpeg_avformat_open_input_func* avformat_open_input;
	ffmpeg_avformat_find_stream_info_func* avformat_find_stream_info;
	ffmpeg_av_dump_format_func* av_dump_format;
	ffmpeg_av_read_frame_func* av_read_frame;
	ffmpeg_avformat_alloc_context_func* avformat_alloc_context;
	ffmpeg_avformat_seek_file_func* avformat_seek_file;
	ffmpeg_avformat_match_stream_specifier_func* avformat_match_stream_specifier;
	ffmpeg_av_find_best_stream_func* av_find_best_stream;
	ffmpeg_av_guess_sample_aspect_ratio_func* av_guess_sample_aspect_ratio;
	ffmpeg_av_guess_frame_rate_func* av_guess_frame_rate;
	ffmpeg_av_read_pause_func* av_read_pause;
	ffmpeg_av_read_play_func* av_read_play;
	ffmpeg_avio_feof_func* avio_feof;
	ffmpeg_av_find_program_from_stream_func* av_find_program_from_stream;
	ffmpeg_av_format_inject_global_side_data_func* av_format_inject_global_side_data;
	ffmpeg_avio_size_func* avio_size;
	ffmpeg_avio_seek_func* avio_seek;

	// Codec
	ffmpeg_get_lib_version_func* avcodec_version;
	ffmpeg_avcodec_free_context_func* avcodec_free_context;
	ffmpeg_avcodec_alloc_context3_func* avcodec_alloc_context3;
	ffmpeg_avcodec_parameters_to_context_func* avcodec_parameters_to_context;
	ffmpeg_avcodec_find_decoder_func* avcodec_find_decoder;
	ffmpeg_avcodec_open2_func* avcodec_open2;
	ffmpeg_avcodec_receive_frame_func* avcodec_receive_frame;
	ffmpeg_avcodec_send_packet_func* avcodec_send_packet;
	ffmpeg_av_packet_alloc_func* av_packet_alloc;
	ffmpeg_av_packet_free_func* av_packet_free;
	ffmpeg_av_packet_ref_func* av_packet_ref;
	ffmpeg_av_packet_unref_func* av_packet_unref;
	ffmpeg_av_new_packet_func* av_new_packet;
	ffmpeg_avsubtitle_free_func* avsubtitle_free;
	ffmpeg_avcodec_find_decoder_by_name_func* avcodec_find_decoder_by_name;
	ffmpeg_av_packet_move_ref_func* av_packet_move_ref;
	ffmpeg_avcodec_flush_buffers_func* avcodec_flush_buffers;
	ffmpeg_avcodec_decode_subtitle2_func* avcodec_decode_subtitle2;
	ffmpeg_av_rdft_init_func* av_rdft_init;
	ffmpeg_av_rdft_calc_func* av_rdft_calc;
	ffmpeg_av_rdft_end_func* av_rdft_end;

	// Util
	ffmpeg_get_lib_version_func* avutil_version;
	ffmpeg_av_frame_alloc_func* av_frame_alloc;
	ffmpeg_av_frame_free_func* av_frame_free;
	ffmpeg_av_frame_unref_func* av_frame_unref;
	ffmpeg_av_frame_move_ref_func* av_frame_move_ref;
	ffmpeg_av_image_get_buffer_size_func* av_image_get_buffer_size;
	ffmpeg_av_image_get_linesize_func* av_image_get_linesize;
	ffmpeg_av_image_fill_arrays_func* av_image_fill_arrays;
	ffmpeg_av_gettime_relative_func* av_gettime_relative;
	ffmpeg_av_gettime_func* av_gettime;
	ffmpeg_av_get_media_type_string_func* av_get_media_type_string;
	ffmpeg_av_rescale_q_func* av_rescale_q;
	ffmpeg_av_samples_get_buffer_size_func* av_samples_get_buffer_size;
	ffmpeg_av_malloc_func* av_malloc;
	ffmpeg_av_mallocz_func* av_mallocz;
	ffmpeg_av_fast_malloc_func* av_fast_malloc;
	ffmpeg_av_free_func* av_free;
	ffmpeg_av_freep_func* av_freep;
	ffmpeg_av_get_packed_sample_fmt_func* av_get_packed_sample_fmt;
	ffmpeg_av_channel_layout_default_func* av_channel_layout_default;
	ffmpeg_av_usleep_func* av_usleep;
	ffmpeg_av_strdup_func* av_strdup;
	ffmpeg_av_log2_func* av_log2;
	ffmpeg_av_compare_ts_func* av_compare_ts;
	ffmpeg_av_get_bytes_per_sample_func* av_get_bytes_per_sample;
	ffmpeg_av_get_sample_fmt_name_func* av_get_sample_fmt_name;
	ffmpeg_av_log_set_flags_func* av_log_set_flags;
	ffmpeg_av_log_func* av_log;
	ffmpeg_av_get_pix_fmt_string_func* av_get_pix_fmt_string;
	ffmpeg_av_get_pix_fmt_name_func* av_get_pix_fmt_name;
	ffmpeg_av_fifo_write_func *av_fifo_write;
	ffmpeg_av_fifo_alloc2_func *av_fifo_alloc2;
	ffmpeg_av_fifo_read_func *av_fifo_read;
	ffmpeg_av_fifo_freep2_func *av_fifo_freep2;

	// SWS
	ffmpeg_get_lib_version_func* swscale_version;
	ffmpeg_sws_getContext_func* sws_getContext;
	ffmpeg_sws_getCachedContext_func* sws_getCachedContext;
	ffmpeg_sws_scale_func* sws_scale;
	ffmpeg_sws_freeContext_func* sws_freeContext;

	// SWR
	ffmpeg_get_lib_version_func* swresample_version;
	ffmpeg_swr_alloc_set_opts2_func* swr_alloc_set_opts2;
	ffmpeg_swr_free_func* swr_free;
	ffmpeg_swr_convert_func* swr_convert;
	ffmpeg_swr_init_func* swr_init;
	ffmpeg_swr_set_compensation_func* swr_set_compensation;
};

static void ReleaseFFMPEG(FFMPEGContext& ffmpeg) {
#if !USE_FFMPEG_STATIC_LINKING
	fplDynamicLibraryUnload(&ffmpeg.swResampleLib);
	fplDynamicLibraryUnload(&ffmpeg.swScaleLib);
	fplDynamicLibraryUnload(&ffmpeg.avUtilLib);
	fplDynamicLibraryUnload(&ffmpeg.avCodecLib);
	fplDynamicLibraryUnload(&ffmpeg.avFormatLib);
#endif
}

inline fplDynamicLibraryHandle LoadFFMPEGLibrary(const char* filePath) {
	fplDynamicLibraryHandle result = {};
	fplDynamicLibraryLoad(filePath, &result);
	return(result);
}

static bool IsFFMPEGVersionEqual(int a, int b) {
	int amajor = AV_VERSION_MAJOR(a);
	int aminor = AV_VERSION_MINOR(a);

	int bmajor = AV_VERSION_MAJOR(b);
	int bminor = AV_VERSION_MINOR(b);

	bool result = (amajor == bmajor) && (aminor == bminor);
	return(result);
}

#define FFMPEG_CHECK_VERSION(libName, libFileName, versionFunc, headerVersion) \
	do { \
		int dllVersion = versionFunc(); \
		if (!IsFFMPEGVersionEqual(dllVersion, headerVersion)) { \
			FPL_LOG_ERROR("FFMPEG", "%s library '%s' version (%d.%d) does not match header version (%d.%d)!", libName, libFileName, AV_VERSION_MAJOR(dllVersion), AV_VERSION_MINOR(dllVersion), AV_VERSION_MAJOR(headerVersion), AV_VERSION_MINOR(headerVersion)); \
			return false; \
		} \
	} while (0);

static bool LoadFFMPEG(FFMPEGContext& ffmpeg) {
#define FFMPEG_CONCAT_SO(name, version) name STR(version)
#define FFMPEG_CONCAT_DLL(name, version, ext) name STR(version) ext

#if !USE_FFMPEG_STATIC_LINKING
	
#	if defined(FPL_PLATFORM_WINDOWS)
	const char* avFormatLibFile = FFMPEG_CONCAT_DLL("avformat-", LIBAVFORMAT_VERSION_MAJOR, ".dll");
	const char* avCodecLibFile = FFMPEG_CONCAT_DLL("avcodec-", LIBAVCODEC_VERSION_MAJOR, ".dll");
	const char* avUtilLibFile = FFMPEG_CONCAT_DLL("avutil-", LIBAVUTIL_VERSION_MAJOR, ".dll");
	const char* swScaleLibFile = FFMPEG_CONCAT_DLL("swscale-", LIBSWSCALE_VERSION_MAJOR, ".dll");
	const char* swResampleLibFile = FFMPEG_CONCAT_DLL("swresample-", LIBSWRESAMPLE_VERSION_MAJOR, ".dll");
#	else
	const char *avFormatLibFile = FFMPEG_CONCAT_SO("libavformat.so.", LIBAVFORMAT_VERSION_MAJOR);
	const char *avCodecLibFile = FFMPEG_CONCAT_SO("libavcodec.so.", LIBAVCODEC_VERSION_MAJOR);
	const char *avUtilLibFile = FFMPEG_CONCAT_SO("libavutil.so.", LIBAVUTIL_VERSION_MAJOR);
	const char *swScaleLibFile = FFMPEG_CONCAT_SO("libswscale.so.", LIBSWSCALE_VERSION_MAJOR);
	const char *swResampleLibFile = FFMPEG_CONCAT_SO("libswresample.so.", LIBSWRESAMPLE_VERSION_MAJOR);
#	endif // FPL_PLATFORM_WINDOWS

	fplDynamicLibraryHandle avFormatLib = ffmpeg.avFormatLib = LoadFFMPEGLibrary(avFormatLibFile);
	fplDynamicLibraryHandle avCodecLib = ffmpeg.avCodecLib = LoadFFMPEGLibrary(avCodecLibFile);
	fplDynamicLibraryHandle avUtilLib = ffmpeg.avUtilLib = LoadFFMPEGLibrary(avUtilLibFile);
	fplDynamicLibraryHandle swScaleLib = ffmpeg.swScaleLib = LoadFFMPEGLibrary(swScaleLibFile);
	fplDynamicLibraryHandle swResampleLib = ffmpeg.swResampleLib = LoadFFMPEGLibrary(swResampleLibFile);
	
#else
	
#	if defined(FPL_PLATFORM_WINDOWS)
	const char* avFormatLibFile = "avformat.lib";
	const char* avCodecLibFile = "avcodec.lib";
	const char* avUtilLibFile = "avutil.lib";
	const char* swScaleLibFile = "swscale.lib";
	const char* swResampleLibFile = "swresample.lib";
#	else
	const char* avFormatLibFile = "avformat";
	const char* avCodecLibFile = "avcodec";
	const char* avUtilLibFile = "avutil";
	const char* swScaleLibFile = "swscale";
	const char* swResampleLibFile = "swresample";
#	endif // FPL_PLATFORM_WINDOWS
	
#endif // !USE_FFMPEG_STATIC_LINKING
	
#undef FFMPEG_CONCAT_DLL
#undef FFMPEG_CONCAT_SO

//
// AVFormat
//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_version, ffmpeg_get_lib_version_func, "avformat_version");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_network_init, ffmpeg_avformat_network_init_func, "avformat_network_init");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_network_deinit, ffmpeg_avformat_network_deinit_func, "avformat_network_deinit");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_close_input, ffmpeg_avformat_close_input_func, "avformat_close_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_open_input, ffmpeg_avformat_open_input_func, "avformat_open_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_find_stream_info, ffmpeg_avformat_find_stream_info_func, "avformat_find_stream_info");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_dump_format, ffmpeg_av_dump_format_func, "av_dump_format");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_read_frame, ffmpeg_av_read_frame_func, "av_read_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_alloc_context, ffmpeg_avformat_alloc_context_func, "avformat_alloc_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_seek_file, ffmpeg_avformat_seek_file_func, "avformat_seek_file");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformat_match_stream_specifier, ffmpeg_avformat_match_stream_specifier_func, "avformat_match_stream_specifier");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_find_best_stream, ffmpeg_av_find_best_stream_func, "av_find_best_stream");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_guess_sample_aspect_ratio, ffmpeg_av_guess_sample_aspect_ratio_func, "av_guess_sample_aspect_ratio");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_guess_frame_rate, ffmpeg_av_guess_frame_rate_func, "av_guess_frame_rate");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_read_pause, ffmpeg_av_read_pause_func, "av_read_pause");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_read_play, ffmpeg_av_read_play_func, "av_read_play");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avio_feof, ffmpeg_avio_feof_func, "avio_feof");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_find_program_from_stream, ffmpeg_av_find_program_from_stream_func, "av_find_program_from_stream");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.av_format_inject_global_side_data, ffmpeg_av_format_inject_global_side_data_func, "av_format_inject_global_side_data");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avio_size, ffmpeg_avio_size_func, "avio_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avio_seek, ffmpeg_avio_seek_func, "avio_seek");
#else
	ffmpeg.avformat_version = avformat_version;
	ffmpeg.avformat_network_init = avformat_network_init;
	ffmpeg.avformat_network_deinit = avformat_network_deinit;
	ffmpeg.avformat_close_input = avformat_close_input;
	ffmpeg.avformat_open_input = avformat_open_input;
	ffmpeg.avformat_find_stream_info = avformat_find_stream_info;
	ffmpeg.av_dump_format = av_dump_format;
	ffmpeg.av_read_frame = av_read_frame;
	ffmpeg.avformat_alloc_context = avformat_alloc_context;
	ffmpeg.avformat_seek_file = avformat_seek_file;
	ffmpeg.avformat_match_stream_specifier = avformat_match_stream_specifier;
	ffmpeg.av_find_best_stream = av_find_best_stream;
	ffmpeg.av_guess_sample_aspect_ratio = av_guess_sample_aspect_ratio;
	ffmpeg.av_guess_frame_rate = av_guess_frame_rate;
	ffmpeg.av_read_pause = av_read_pause;
	ffmpeg.av_read_play = av_read_play;
	ffmpeg.avio_feof = avio_feof;
	ffmpeg.av_find_program_from_stream = av_find_program_from_stream;
	ffmpeg.av_format_inject_global_side_data = av_format_inject_global_side_data;
	ffmpeg.avio_size = avio_size;
	ffmpeg.avio_seek = avio_seek;
#endif

	//
	// AVCodec
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_version, ffmpeg_get_lib_version_func, "avcodec_version");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_free_context, ffmpeg_avcodec_free_context_func, "avcodec_free_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_alloc_context3, ffmpeg_avcodec_alloc_context3_func, "avcodec_alloc_context3");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_parameters_to_context, ffmpeg_avcodec_parameters_to_context_func, "avcodec_parameters_to_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_find_decoder, ffmpeg_avcodec_find_decoder_func, "avcodec_find_decoder");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_open2, ffmpeg_avcodec_open2_func, "avcodec_open2");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_receive_frame, ffmpeg_avcodec_receive_frame_func, "avcodec_receive_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_send_packet, ffmpeg_avcodec_send_packet_func, "avcodec_send_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_packet_alloc, ffmpeg_av_packet_alloc_func, "av_packet_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_packet_free, ffmpeg_av_packet_free_func, "av_packet_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_packet_ref, ffmpeg_av_packet_ref_func, "av_packet_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_packet_unref, ffmpeg_av_packet_unref_func, "av_packet_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_new_packet, ffmpeg_av_new_packet_func, "av_new_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avsubtitle_free, ffmpeg_avsubtitle_free_func, "avsubtitle_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_find_decoder_by_name, ffmpeg_avcodec_find_decoder_by_name_func, "avcodec_find_decoder_by_name");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_packet_move_ref, ffmpeg_av_packet_move_ref_func, "av_packet_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_flush_buffers, ffmpeg_avcodec_flush_buffers_func, "avcodec_flush_buffers");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodec_decode_subtitle2, ffmpeg_avcodec_decode_subtitle2_func, "avcodec_decode_subtitle2");

	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_rdft_init, ffmpeg_av_rdft_init_func, "av_rdft_init");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_rdft_calc, ffmpeg_av_rdft_calc_func, "av_rdft_calc");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.av_rdft_end, ffmpeg_av_rdft_end_func, "av_rdft_end");
#else
	ffmpeg.avcodec_version = avcodec_version;
	ffmpeg.avcodec_free_context = avcodec_free_context;
	ffmpeg.avcodec_alloc_context3 = avcodec_alloc_context3;
	ffmpeg.avcodec_parameters_to_context = avcodec_parameters_to_context;
	ffmpeg.avcodec_find_decoder = avcodec_find_decoder;
	ffmpeg.avcodec_open2 = avcodec_open2;
	ffmpeg.avcodec_receive_frame = avcodec_receive_frame;
	ffmpeg.avcodec_send_packet = avcodec_send_packet;
	ffmpeg.av_packet_alloc = av_packet_alloc;
	ffmpeg.av_packet_free = av_packet_free;
	ffmpeg.av_packet_ref = av_packet_ref;
	ffmpeg.av_packet_unref = av_packet_unref;
	ffmpeg.av_new_packet = av_new_packet;
	ffmpeg.avsubtitle_free = avsubtitle_free;
	ffmpeg.avcodec_find_decoder_by_name = avcodec_find_decoder_by_name;
	ffmpeg.av_packet_move_ref = av_packet_move_ref;
	ffmpeg.avcodec_flush_buffers = avcodec_flush_buffers;
	ffmpeg.avcodec_decode_subtitle2 = avcodec_decode_subtitle2;
	ffmpeg.av_rdft_init = av_rdft_init;
	ffmpeg.av_rdft_calc = av_rdft_calc;
	ffmpeg.av_rdft_end = av_rdft_end;
#endif

	//
	// AVUtil
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avutil_version, ffmpeg_get_lib_version_func, "avutil_version");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_frame_alloc, ffmpeg_av_frame_alloc_func, "av_frame_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_frame_free, ffmpeg_av_frame_free_func, "av_frame_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_frame_unref, ffmpeg_av_frame_unref_func, "av_frame_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_frame_move_ref, ffmpeg_av_frame_move_ref_func, "av_frame_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_image_get_buffer_size, ffmpeg_av_image_get_buffer_size_func, "av_image_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_image_get_linesize, ffmpeg_av_image_get_linesize_func, "av_image_get_linesize");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_image_fill_arrays, ffmpeg_av_image_fill_arrays_func, "av_image_fill_arrays");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_gettime_relative, ffmpeg_av_gettime_relative_func, "av_gettime_relative");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_gettime, ffmpeg_av_gettime_func, "av_gettime");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_media_type_string, ffmpeg_av_get_media_type_string_func, "av_get_media_type_string");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_rescale_q, ffmpeg_av_rescale_q_func, "av_rescale_q");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_samples_get_buffer_size, ffmpeg_av_samples_get_buffer_size_func, "av_samples_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_malloc, ffmpeg_av_malloc_func, "av_malloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_mallocz, ffmpeg_av_mallocz_func, "av_mallocz");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_fast_malloc, ffmpeg_av_fast_malloc_func, "av_fast_malloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_free, ffmpeg_av_freep_func, "av_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_freep, ffmpeg_av_freep_func, "av_freep");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_packed_sample_fmt, ffmpeg_av_get_packed_sample_fmt_func, "av_get_packed_sample_fmt");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_channel_layout_default, ffmpeg_av_channel_layout_default_func, "av_channel_layout_default");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_usleep, ffmpeg_av_usleep_func, "av_usleep");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_strdup, ffmpeg_av_strdup_func, "av_strdup");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_log2, ffmpeg_av_log2_func, "av_log2");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_compare_ts, ffmpeg_av_compare_ts_func, "av_compare_ts");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_bytes_per_sample, ffmpeg_av_get_bytes_per_sample_func, "av_get_bytes_per_sample");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_sample_fmt_name, ffmpeg_av_get_sample_fmt_name_func, "av_get_sample_fmt_name");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_log_set_flags, ffmpeg_av_log_set_flags_func, "av_log_set_flags");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_log, ffmpeg_av_log_func, "av_log");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_pix_fmt_string, ffmpeg_av_get_pix_fmt_string_func, "av_get_pix_fmt_string");	
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_get_pix_fmt_name, ffmpeg_av_get_pix_fmt_name_func, "av_get_pix_fmt_name");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_fifo_write, ffmpeg_av_fifo_write_func, "av_fifo_write");	
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_fifo_alloc2, ffmpeg_av_fifo_alloc2_func, "av_fifo_alloc2");	
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_fifo_read, ffmpeg_av_fifo_read_func, "av_fifo_read");	
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.av_fifo_freep2, ffmpeg_av_fifo_freep2_func, "av_fifo_freep2");	
#else	
	ffmpeg.avutil_version = avutil_version;
	ffmpeg.av_frame_alloc = av_frame_alloc;
	ffmpeg.av_frame_free = av_frame_free;
	ffmpeg.av_frame_unref = av_frame_unref;
	ffmpeg.av_frame_move_ref = av_frame_move_ref;
	ffmpeg.av_image_get_buffer_size = av_image_get_buffer_size;
	ffmpeg.av_image_get_linesize = av_image_get_linesize;
	ffmpeg.av_image_fill_arrays = av_image_fill_arrays;
	ffmpeg.av_gettime_relative = av_gettime_relative;
	ffmpeg.av_gettime = av_gettime;
	ffmpeg.av_get_media_type_string = av_get_media_type_string;
	ffmpeg.av_rescale_q = av_rescale_q;
	ffmpeg.av_samples_get_buffer_size = av_samples_get_buffer_size;
	ffmpeg.av_malloc = av_malloc;
	ffmpeg.av_mallocz = av_mallocz;
	ffmpeg.av_fast_malloc = av_fast_malloc;
	ffmpeg.av_free = av_free;
	ffmpeg.av_freep = av_freep;
	ffmpeg.av_get_packed_sample_fmt = av_get_packed_sample_fmt;
	ffmpeg.av_channel_layout_default = av_channel_layout_default;
	ffmpeg.av_usleep = av_usleep;
	ffmpeg.av_strdup = av_strdup;
	ffmpeg.av_log2 = av_log2;
	ffmpeg.av_compare_ts = av_compare_ts;
	ffmpeg.av_get_bytes_per_sample = av_get_bytes_per_sample;
	ffmpeg.av_get_sample_fmt_name = av_get_sample_fmt_name;
	ffmpeg.av_log_set_flags = av_log_set_flags;
	ffmpeg.av_log = av_log;
	ffmpeg.av_get_pix_fmt_string = av_get_pix_fmt_string;
	ffmpeg.av_get_pix_fmt_name = av_get_pix_fmt_name;
	ffmpeg.av_fifo_write = av_fifo_write;
	ffmpeg.av_fifo_alloc2 = av_fifo_alloc2;
	ffmpeg.av_fifo_read = av_fifo_read;
	ffmpeg.av_fifo_freep2 = av_fifo_freep2;
#endif

	//
	// SWS
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swscale_version, ffmpeg_get_lib_version_func, "swscale_version");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.sws_getContext, ffmpeg_sws_getContext_func, "sws_getContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.sws_scale, ffmpeg_sws_scale_func, "sws_scale");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.sws_freeContext, ffmpeg_sws_freeContext_func, "sws_freeContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.sws_getCachedContext, ffmpeg_sws_getCachedContext_func, "sws_getCachedContext");
#else
	ffmpeg.swscale_version = swscale_version;
	ffmpeg.sws_getContext = sws_getContext;
	ffmpeg.sws_scale = sws_scale;
	ffmpeg.sws_freeContext = sws_freeContext;
	ffmpeg.sws_getCachedContext = sws_getCachedContext;
#endif

	//
	// SWResamle
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swresample_version, ffmpeg_get_lib_version_func, "swresample_version");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swr_alloc_set_opts2, ffmpeg_swr_alloc_set_opts2_func, "swr_alloc_set_opts2");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swr_free, ffmpeg_swr_free_func, "swr_free");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swr_convert, ffmpeg_swr_convert_func, "swr_convert");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swr_init, ffmpeg_swr_init_func, "swr_init");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swr_set_compensation, ffmpeg_swr_set_compensation_func, "swr_set_compensation");
#else
	ffmpeg.swresample_version = swresample_version;
	ffmpeg.swr_alloc_set_opts2 = swr_alloc_set_opts2;
	ffmpeg.swr_free = swr_free;
	ffmpeg.swr_convert = swr_convert;
	ffmpeg.swr_init = swr_init;
	ffmpeg.swr_set_compensation = swr_set_compensation;
#endif

	// Check versions
	FFMPEG_CHECK_VERSION("AVformat", avFormatLibFile, ffmpeg.avformat_version, LIBAVFORMAT_VERSION_INT);
	FFMPEG_CHECK_VERSION("AVcodec", avCodecLibFile, ffmpeg.avcodec_version, LIBAVCODEC_VERSION_INT);
	FFMPEG_CHECK_VERSION("AVutil", avUtilLibFile, ffmpeg.avutil_version, LIBAVUTIL_VERSION_INT);
	FFMPEG_CHECK_VERSION("SWscale", swScaleLibFile, ffmpeg.swscale_version, LIBSWSCALE_VERSION_INT);
	FFMPEG_CHECK_VERSION("SWresample", swResampleLibFile, ffmpeg.swresample_version, LIBSWRESAMPLE_VERSION_INT);
	
	return true;
}