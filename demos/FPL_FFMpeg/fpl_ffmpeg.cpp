/*

Custom FFMPEG Media Player Example based on FPL
Written by Torsten Spaete

[x] Reads packets from stream and queues them up
[x] Decodes video and audio packets and queues them up as well
[x] FFMPEG functions are loaded dynamically
[ ] Pause/Resume/Restart
[ ] Fix: Audio starts very late
[ ] Handle PTS/DTS to syncronize audio/video
[ ] Seeking
[ ] OpenGL Video Rendering
[ ] Composite video rendering
	[ ] OSD
	[ ] Bitmap rect blitting
	[ ] Subtitle Decoding and Compositing
[ ] Image format conversion (YUY2, YUV > RGB24 etc.)
	[ ] Slow CPU implementation
	[ ] SSE2/AVX implementation
	[ ] GLSL
[ ] Audio format conversion (Downsampling, Upsamplíng, S16 > F32 etc.)
	[ ] Slow CPU implementation
	[ ] SSE2/AVX implementation
[ ] UI
	[ ] Current Time
	[ ] Buttons
	[ ] File dialog
	[ ] Seekbar
	[ ] Playlist

Docs:
- http://dranger.com/ffmpeg/tutorial01.html
- https://blogs.gentoo.org/lu_zero/2015/10/15/deprecating-avpicture/
- https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
- https://www.codeproject.com/tips/489450/creating-custom-ffmpeg-io-context

Requirements:
- Custom ffmpeg win64 build from https://ffmpeg.zeranoe.com/builds/

*/

#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

#include <assert.h> // assert

#include "utils.h"

//
// FFMPEG headers and function prototypes
//
extern "C" {
#	include <libavcodec/avcodec.h>
#	include <libavformat/avformat.h>
#	include <libavutil/avutil.h>
#	include <libavutil/imgutils.h>
#	include <libswscale\swscale.h>
#	include <libswresample\swresample.h>
#	include <libavutil/time.h>
}

//
// AVFormat
//

// av_register_all
#define FFMPEG_AV_REGISTER_ALL_FUNC(name) void name(void)
typedef FFMPEG_AV_REGISTER_ALL_FUNC(ffmpeg_av_register_all_func);
// avformat_close_input
#define FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(name) void name(AVFormatContext **s)
typedef FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(ffmpeg_avformat_close_input_func);
// avformat_open_input
#define FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(name) int name(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options)
typedef FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(ffmpeg_avformat_open_input_func);
// avformat_find_stream_info
#define FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(name) int name(AVFormatContext *ic, AVDictionary **options)
typedef FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(ffmpeg_avformat_find_stream_info_func);
// av_dump_format
#define FFMPEG_AV_DUMP_FORMAT_FUNC(name) void name(AVFormatContext *ic, int index, const char *url, int is_output)
typedef FFMPEG_AV_DUMP_FORMAT_FUNC(ffmpeg_av_dump_format_func);
// av_read_frame
#define FFMPEG_AV_READ_FRAME_FUNC(name) int name(AVFormatContext *s, AVPacket *pkt)
typedef FFMPEG_AV_READ_FRAME_FUNC(ffmpeg_av_read_frame_func);
// avformat_alloc_context
#define FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(name) AVFormatContext *name(void)
typedef FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(ffmpeg_avformat_alloc_context_func);
// avformat_seek_file
#define FFMPEG_AVFORMAT_SEEK_FILE_FUNC(name) int name(AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
typedef FFMPEG_AVFORMAT_SEEK_FILE_FUNC(ffmpeg_avformat_seek_file_func);
// avformat_match_stream_specifier
#define FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(name) int name(AVFormatContext *s, AVStream *st, const char *spec)
typedef FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(ffmpeg_avformat_match_stream_specifier_func);
// av_find_best_stream
#define FFMPEG_AV_FIND_BEST_STREAM_FUNC(name) int name(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags)
typedef FFMPEG_AV_FIND_BEST_STREAM_FUNC(ffmpeg_av_find_best_stream_func);
// av_guess_sample_aspect_ratio
#define FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(name) AVRational name(AVFormatContext *format, AVStream *stream, AVFrame *frame)
typedef FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(ffmpeg_av_guess_sample_aspect_ratio_func);
// av_guess_frame_rate
#define FFMPEG_AV_GUESS_FRAME_RATE_FUNC(name) AVRational name(AVFormatContext *ctx, AVStream *stream, AVFrame *frame)
typedef FFMPEG_AV_GUESS_FRAME_RATE_FUNC(ffmpeg_av_guess_frame_rate_func);

// av_read_pause
#define FFMPEG_AV_READ_PAUSE_FUNC(name) int name(AVFormatContext *s)
typedef FFMPEG_AV_READ_PAUSE_FUNC(ffmpeg_av_read_pause_func);
// av_read_play
#define FFMPEG_AV_READ_PLAY_FUNC(name) int name(AVFormatContext *s)
typedef FFMPEG_AV_READ_PLAY_FUNC(ffmpeg_av_read_play_func);
// avio_feof
#define FFMPEG_AVIO_FEOF_FUNC(name) int name(AVIOContext *s)
typedef FFMPEG_AVIO_FEOF_FUNC(ffmpeg_avio_feof_func);


//
// AVCodec
//

// avcodec_free_context
#define FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(name) void name(AVCodecContext **avctx)
typedef FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(ffmpeg_avcodec_free_context_func);
// avcodec_alloc_context3
#define FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(name) AVCodecContext *name(const AVCodec *codec)
typedef FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(ffmpeg_avcodec_alloc_context3_func);
// avcodec_parameters_to_context
#define FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(name) int name(AVCodecContext *codec, const AVCodecParameters *par)
typedef FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(ffmpeg_avcodec_parameters_to_context_func);
// avcodec_find_decoder
#define FFMPEG_AVCODEC_FIND_DECODER_FUNC(name) AVCodec *name(enum AVCodecID id)
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
// av_init_packet
#define FFMPEG_AV_INIT_PACKET_FUNC(name) void name(AVPacket *pkt)
typedef FFMPEG_AV_INIT_PACKET_FUNC(ffmpeg_av_init_packet_func);
// avcodec_find_decoder_by_name
#define FFMPEG_AVCODEC_FIND_DECODER_BY_NAME_FUNC(name) AVCodec *name(const char *cname)
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
// av_get_channel_layout_nb_channels
#define FFMPEG_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS_FUNC(name) int name(uint64_t channel_layout)
typedef FFMPEG_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS_FUNC(ffmpeg_av_get_channel_layout_nb_channels_func);
// av_gettime_relative
#define FFMPEG_AV_GETTIME_RELATIVE_FUNC(name) int64_t name(void)
typedef FFMPEG_AV_GETTIME_RELATIVE_FUNC(ffmpeg_av_gettime_relative_func);
// av_get_media_type_string
#define FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(name) const char *name(enum AVMediaType media_type)
typedef FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(ffmpeg_av_get_media_type_string_func);
// av_rescale_q
#define FFMPEG_AV_RESCALE_Q_FUNC(name) int64_t name(int64_t a, AVRational bq, AVRational cq) av_const
typedef FFMPEG_AV_RESCALE_Q_FUNC(ffmpeg_av_rescale_q_func);
// av_samples_get_buffer_size
#define FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(name) int name(int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
typedef FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(ffmpeg_av_samples_get_buffer_size_func);

//
// SWS
//

// sws_getContext
#define FFMPEG_SWS_GET_CONTEXT_FUNC(name) struct SwsContext *name(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CONTEXT_FUNC(ffmpeg_sws_getContext_func);
// sws_getCachedContext
#define FFMPEG_SWS_GET_CACHED_CONTEXT_FUNC(name) struct SwsContext *name(struct SwsContext *context, int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
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
#define FFMPEG_SWR_ALLOC_SET_OPTS(name) struct SwrContext *name(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate, int log_offset, void *log_ctx)
typedef FFMPEG_SWR_ALLOC_SET_OPTS(ffmpeg_swr_alloc_set_opts_func);
// swr_free
#define FFMPEG_SWR_FREE(name) void name(struct SwrContext **s)
typedef FFMPEG_SWR_FREE(ffmpeg_swr_free_func);
// swr_convert
#define FFMPEG_SWR_CONVERT(name) int name(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count)
typedef FFMPEG_SWR_CONVERT(ffmpeg_swr_convert_func);
// swr_init
#define FFMPEG_SWR_INIT(name) int name(struct SwrContext *s)
typedef FFMPEG_SWR_INIT(ffmpeg_swr_init_func);

#define FFMPEG_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)GetDynamicLibraryProc(libHandle, name); \
	if (target == nullptr) { \
		ConsoleFormatError("[FFMPEG] Failed getting '%s' from library '%s'!", name, libName); \
		return false; \
	}

struct FFMPEGContext {
	DynamicLibraryHandle avFormatLib;
	DynamicLibraryHandle avCodecLib;
	DynamicLibraryHandle avUtilLib;
	DynamicLibraryHandle swScaleLib;
	DynamicLibraryHandle swResampleLib;

	// Format
	ffmpeg_av_register_all_func *avRegisterAll;
	ffmpeg_avformat_close_input_func *avformatCloseInput;
	ffmpeg_avformat_open_input_func *avformatOpenInput;
	ffmpeg_avformat_find_stream_info_func *avformatFindStreamInfo;
	ffmpeg_av_dump_format_func *avDumpFormat;
	ffmpeg_av_read_frame_func *avReadFrame;
	ffmpeg_avformat_alloc_context_func *avformatAllocContext;
	ffmpeg_avformat_seek_file_func *avformatSeekFile;
	ffmpeg_avformat_match_stream_specifier_func *avformatMatchStreamSpecifier;
	ffmpeg_av_find_best_stream_func *avFindBestStream;
	ffmpeg_av_guess_sample_aspect_ratio_func *avGuessSampleAspectRatio;
	ffmpeg_av_guess_frame_rate_func *avGuessFrameRate;
	ffmpeg_av_read_pause_func *avReadPause;
	ffmpeg_av_read_play_func *avReadPlay;
	ffmpeg_avio_feof_func *avioFEOF;

	// Codec
	ffmpeg_avcodec_free_context_func *avcodecFreeContext;
	ffmpeg_avcodec_alloc_context3_func *avcodecAllocContext3;
	ffmpeg_avcodec_parameters_to_context_func *avcodecParametersToContext;
	ffmpeg_avcodec_find_decoder_func *avcodecFindDecoder;
	ffmpeg_avcodec_open2_func *avcodecOpen2;
	ffmpeg_av_packet_unref_func *avPacketUnref;
	ffmpeg_avcodec_receive_frame_func *avcodecReceiveFrame;
	ffmpeg_avcodec_send_packet_func *avcodecSendPacket;
	ffmpeg_av_packet_alloc_func *avPacketAlloc;
	ffmpeg_av_packet_free_func *avPacketFree;
	ffmpeg_av_init_packet_func *avInitPacket;
	ffmpeg_avsubtitle_free_func *avsubtitleFree;
	ffmpeg_avcodec_find_decoder_by_name_func *avcodecFindDecoderByName;
	ffmpeg_av_packet_move_ref_func *avPacketMoveRef;
	ffmpeg_avcodec_flush_buffers_func *avcodecFlushBuffers;
	ffmpeg_avcodec_decode_subtitle2_func *avcodecDecodeSubtitle2;
	ffmpeg_av_packet_ref_func *avPacketRef;

	// Util
	ffmpeg_av_frame_alloc_func *avFrameAlloc;
	ffmpeg_av_frame_free_func *avFrameFree;
	ffmpeg_av_frame_unref_func *avFrameUnref;
	ffmpeg_av_frame_move_ref_func *avFrameMoveRef;
	ffmpeg_av_image_get_buffer_size_func *avImageGetBufferSize;
	ffmpeg_av_image_get_linesize_func *avImageGetLineSize;
	ffmpeg_av_image_fill_arrays_func *avImageFillArrays;
	ffmpeg_av_get_channel_layout_nb_channels_func *avGetChannelLayoutNBChannels;
	ffmpeg_av_gettime_relative_func *avGetTimeRelative;
	ffmpeg_av_get_media_type_string_func *avGetMediaTypeString;
	ffmpeg_av_rescale_q_func *avRescaleQ;
	ffmpeg_av_samples_get_buffer_size_func *avSamplesGetBufferSize;

	// SWS
	ffmpeg_sws_getContext_func *swsGetContext;
	ffmpeg_sws_getCachedContext_func *swsGetCachedContext;
	ffmpeg_sws_scale_func *swsScale;
	ffmpeg_sws_freeContext_func *swsFreeContext;

	// SWR
	ffmpeg_swr_alloc_set_opts_func *swrAllocSetOpts;
	ffmpeg_swr_free_func *swrFree;
	ffmpeg_swr_convert_func *swrConvert;
	ffmpeg_swr_init_func *swrInit;
};

static void ReleaseFFMPEG(FFMPEGContext &ffmpeg) {
	DynamicLibraryUnload(ffmpeg.swResampleLib);
	DynamicLibraryUnload(ffmpeg.swScaleLib);
	DynamicLibraryUnload(ffmpeg.avUtilLib);
	DynamicLibraryUnload(ffmpeg.avCodecLib);
	DynamicLibraryUnload(ffmpeg.avFormatLib);
}

static bool LoadFFMPEG(FFMPEGContext &ffmpeg) {
	const char *avFormatLibFile = "avformat-58.dll";
	const char *avCodecLibFile = "avcodec-58.dll";
	const char *avUtilLibFile = "avutil-56.dll";
	const char *swScaleLibFile = "swscale-5.dll";
	const char *swResampleLibFile = "swresample-3.dll";

	DynamicLibraryHandle avFormatLib = ffmpeg.avFormatLib = DynamicLibraryLoad(avFormatLibFile);
	DynamicLibraryHandle avCodecLib = ffmpeg.avCodecLib = DynamicLibraryLoad(avCodecLibFile);
	DynamicLibraryHandle avUtilLib = ffmpeg.avUtilLib = DynamicLibraryLoad(avUtilLibFile);
	DynamicLibraryHandle swScaleLib = ffmpeg.swScaleLib = DynamicLibraryLoad(swScaleLibFile);
	DynamicLibraryHandle swResampleLib = ffmpeg.swResampleLib = DynamicLibraryLoad(swResampleLibFile);

	//
	// AVFormat
	//
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avRegisterAll, ffmpeg_av_register_all_func, "av_register_all");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatCloseInput, ffmpeg_avformat_close_input_func, "avformat_close_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatOpenInput, ffmpeg_avformat_open_input_func, "avformat_open_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatFindStreamInfo, ffmpeg_avformat_find_stream_info_func, "avformat_find_stream_info");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avDumpFormat, ffmpeg_av_dump_format_func, "av_dump_format");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadFrame, ffmpeg_av_read_frame_func, "av_read_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatAllocContext, ffmpeg_avformat_alloc_context_func, "avformat_alloc_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatSeekFile, ffmpeg_avformat_seek_file_func, "avformat_seek_file");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatMatchStreamSpecifier, ffmpeg_avformat_match_stream_specifier_func, "avformat_match_stream_specifier");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avFindBestStream, ffmpeg_av_find_best_stream_func, "av_find_best_stream");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avGuessSampleAspectRatio, ffmpeg_av_guess_sample_aspect_ratio_func, "av_guess_sample_aspect_ratio");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avGuessFrameRate, ffmpeg_av_guess_frame_rate_func, "av_guess_frame_rate");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadPause, ffmpeg_av_read_pause_func, "av_read_pause");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadPlay, ffmpeg_av_read_play_func, "av_read_play");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avioFEOF, ffmpeg_avio_feof_func, "avio_feof");

	//
	// AVCodec
	//
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFreeContext, ffmpeg_avcodec_free_context_func, "avcodec_free_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecAllocContext3, ffmpeg_avcodec_alloc_context3_func, "avcodec_alloc_context3");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecParametersToContext, ffmpeg_avcodec_parameters_to_context_func, "avcodec_parameters_to_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFindDecoder, ffmpeg_avcodec_find_decoder_func, "avcodec_find_decoder");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecOpen2, ffmpeg_avcodec_open2_func, "avcodec_open2");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketUnref, ffmpeg_av_packet_unref_func, "av_packet_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecReceiveFrame, ffmpeg_avcodec_receive_frame_func, "avcodec_receive_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecSendPacket, ffmpeg_avcodec_send_packet_func, "avcodec_send_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketAlloc, ffmpeg_av_packet_alloc_func, "av_packet_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketFree, ffmpeg_av_packet_free_func, "av_packet_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avInitPacket, ffmpeg_av_init_packet_func, "av_init_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avsubtitleFree, ffmpeg_avsubtitle_free_func, "avsubtitle_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFindDecoderByName, ffmpeg_avcodec_find_decoder_by_name_func, "avcodec_find_decoder_by_name");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketMoveRef, ffmpeg_av_packet_move_ref_func, "av_packet_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFlushBuffers, ffmpeg_avcodec_flush_buffers_func, "avcodec_flush_buffers");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecDecodeSubtitle2, ffmpeg_avcodec_decode_subtitle2_func, "avcodec_decode_subtitle2");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketRef, ffmpeg_av_packet_ref_func, "av_packet_ref");

	//
	// AVUtil
	//
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameAlloc, ffmpeg_av_frame_alloc_func, "av_frame_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameFree, ffmpeg_av_frame_free_func, "av_frame_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameUnref, ffmpeg_av_frame_unref_func, "av_frame_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameMoveRef, ffmpeg_av_frame_move_ref_func, "av_frame_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetBufferSize, ffmpeg_av_image_get_buffer_size_func, "av_image_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetLineSize, ffmpeg_av_image_get_linesize_func, "av_image_get_linesize");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageFillArrays, ffmpeg_av_image_fill_arrays_func, "av_image_fill_arrays");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetChannelLayoutNBChannels, ffmpeg_av_get_channel_layout_nb_channels_func, "av_get_channel_layout_nb_channels");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetTimeRelative, ffmpeg_av_gettime_relative_func, "av_gettime_relative");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetMediaTypeString, ffmpeg_av_get_media_type_string_func, "av_get_media_type_string");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avRescaleQ, ffmpeg_av_rescale_q_func, "av_rescale_q");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avSamplesGetBufferSize, ffmpeg_av_samples_get_buffer_size_func, "av_samples_get_buffer_size");

	//
	// SWS
	//
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsGetContext, ffmpeg_sws_getContext_func, "sws_getContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsScale, ffmpeg_sws_scale_func, "sws_scale");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsFreeContext, ffmpeg_sws_freeContext_func, "sws_freeContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsGetCachedContext, ffmpeg_sws_getCachedContext_func, "sws_getCachedContext");

	//
	// SWR
	//
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrAllocSetOpts, ffmpeg_swr_alloc_set_opts_func, "swr_alloc_set_opts");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrFree, ffmpeg_swr_free_func, "swr_free");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrConvert, ffmpeg_swr_convert_func, "swr_convert");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrInit, ffmpeg_swr_init_func, "swr_init");

	return true;
}

static FFMPEGContext *globalFFMPEGFunctions = nullptr;

//
// Lock-Free Multiple Procuder Multiple Consumer Queue
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
//
constexpr size_t CACHE_LINE_SIZE = 64;
typedef char CacheLinePad[CACHE_LINE_SIZE];

template <typename T>
struct MPMCBoundedQueueCell {
	volatile uint64_t sequence;
	T data;
};

template <typename T>
struct MPMCBoundedQueue {
	CacheLinePad pad0;
	MPMCBoundedQueueCell<T> *buffer;
	size_t bufferMask;
	CacheLinePad pad1;
	volatile uint64_t enqueuePos;
	CacheLinePad pad2;
	volatile uint64_t dequeuePos;
	CacheLinePad pad3;

	static MPMCBoundedQueue<T> Create(size_t capacity) {
		size_t bufferCount = RoundToPowerOfTwo(capacity);
		MPMCBoundedQueue<T> result = {};
		result.buffer = (MPMCBoundedQueueCell<T> *)memory::MemoryAlignedAllocate(sizeof(MPMCBoundedQueueCell<T>) * bufferCount, CACHE_LINE_SIZE);
		result.bufferMask = bufferCount - 1;
		assert((bufferCount >= 2) && ((bufferCount & (bufferCount - 1)) == 0));
		for (size_t i = 0; i < bufferCount; i += 1) {
			result.buffer[i].sequence = i;
		}
		result.enqueuePos = 0;
		result.dequeuePos = 0;
		return(result);
	}

	static void Destroy(MPMCBoundedQueue<T> &queue) {
		memory::MemoryAlignedFree(queue.buffer);
		queue = {};
	}
};

template <typename T>
static bool Enqueue(MPMCBoundedQueue<T> &queue, const T &data) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.enqueuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)pos;
		if (dif == 0) {
			if (IsAtomicCompareAndExchangeU64(&queue.enqueuePos, pos, pos + 1)) {
				break;
			}
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.enqueuePos);
	}
	cell->data = data;
	AtomicStoreU64(&cell->sequence, pos + 1);
	return true;
}

template <typename T>
static bool Dequeue(MPMCBoundedQueue<T> &queue, T &data) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.dequeuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
		if (dif == 0) {
			if (IsAtomicCompareAndExchangeU64(&queue.dequeuePos, pos, pos + 1)) {
				break;
			}
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.dequeuePos);
	}
	data = cell->data;
	AtomicStoreU64(&cell->sequence, pos + queue.bufferMask + 1);
	return true;
}

//
// Packet Queue
//

struct PacketQueue {
	AVPacket **packets;
	MPMCBoundedQueue<AVPacket *> freeListQueue;
	MPMCBoundedQueue<AVPacket *> availableVideoPacketsQueue;
	MPMCBoundedQueue<AVPacket *> availableAudioPacketsQueue;
	ThreadSignal freePacketSignal;
	ThreadSignal videoPacketsSignal;
	ThreadSignal audioPacketsSignal;
	ThreadSignal stoppedSignal;
	uint32_t maxPacketCount;
	volatile uint32_t isStopped;
};

inline AVPacket *AllocatePacket() {
	FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVPacket *result = ffmpeg.avPacketAlloc();
	return(result);
}

inline void FreePacket(AVPacket **packet) {
	FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVPacket *p = *packet;
	if (p->data != nullptr) {
		ffmpeg.avPacketUnref(p);
	}
	ffmpeg.avPacketFree(packet);
}

static PacketQueue CreatePacketQueue(uint32_t capacity) {
	PacketQueue result = {};
	result.maxPacketCount = capacity;
	result.packets = (AVPacket **)memory::MemoryAllocate(result.maxPacketCount * sizeof(AVPacket *));

	result.freePacketSignal = SignalCreate();
	result.videoPacketsSignal = SignalCreate();
	result.audioPacketsSignal = SignalCreate();
	result.stoppedSignal = SignalCreate();

	result.freeListQueue = MPMCBoundedQueue<AVPacket *>::Create(capacity);
	for (uint32_t i = 0; i < capacity; ++i) {
		result.packets[i] = AllocatePacket();
		if (!Enqueue(result.freeListQueue, result.packets[i])) {
			assert(!"Not enough capacity in freelist, increase buffer size!");
		}
	}
	result.availableVideoPacketsQueue = MPMCBoundedQueue<AVPacket *>::Create(capacity);
	result.availableAudioPacketsQueue = MPMCBoundedQueue<AVPacket *>::Create(capacity);
	return(result);
}

static void DestroyPacketQueue(PacketQueue &queue) {
	MPMCBoundedQueue<AVPacket *>::Destroy(queue.availableAudioPacketsQueue);
	MPMCBoundedQueue<AVPacket *>::Destroy(queue.availableVideoPacketsQueue);
	MPMCBoundedQueue<AVPacket *>::Destroy(queue.freeListQueue);

	SignalDestroy(queue.stoppedSignal);
	SignalDestroy(queue.audioPacketsSignal);
	SignalDestroy(queue.videoPacketsSignal);
	SignalDestroy(queue.freePacketSignal);

	for (uint32_t i = 0; i < queue.maxPacketCount; ++i) {
		AVPacket *packet = queue.packets[i];
		FreePacket(&packet);
	}
	memory::MemoryFree(queue.packets);
}

//
// Frame Queue
//

struct FrameQueue {
	AVPacket *pendingPacket;
	AVFrame **frames;
	MPMCBoundedQueue<AVFrame *> freeListQueue;
	MPMCBoundedQueue<AVFrame *> availableFramesQueue;
	ThreadSignal freeSignal;
	ThreadSignal availableFrameSignal;
	ThreadSignal stoppedSignal;
	uint32_t maxFrameCount;
	volatile uint32_t isStopped;
	bool isValid;
	bool hasPendingPacket;
};

inline AVFrame *AllocateFrame() {
	FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *result = ffmpeg.avFrameAlloc();
	return(result);
}

inline void FreeFrame(AVFrame **frame) {
	FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *p = *frame;
	ffmpeg.avFrameFree(frame);
}

static FrameQueue CreateFrameQueue(uint32_t capacity) {
	FrameQueue result = {};
	result.maxFrameCount = capacity;
	result.frames = (AVFrame **)memory::MemoryAllocate(result.maxFrameCount * sizeof(AVFrame *));

	result.freeSignal = SignalCreate();
	result.availableFrameSignal = SignalCreate();
	result.stoppedSignal = SignalCreate();

	result.freeListQueue = MPMCBoundedQueue<AVFrame *>::Create(capacity);
	result.availableFramesQueue = MPMCBoundedQueue<AVFrame *>::Create(capacity);
	for (uint32_t i = 0; i < capacity; ++i) {
		result.frames[i] = AllocateFrame();
		if (!Enqueue(result.freeListQueue, result.frames[i])) {
			assert(!"Not enough capacity in freelist, increase buffer size!");
		}
	}
	result.isValid = true;
	return(result);
}

static void DestroyFrameQueue(FrameQueue &queue) {
	if (queue.isValid) {
		MPMCBoundedQueue<AVFrame *>::Destroy(queue.availableFramesQueue);
		MPMCBoundedQueue<AVFrame *>::Destroy(queue.freeListQueue);

		SignalDestroy(queue.stoppedSignal);
		SignalDestroy(queue.availableFrameSignal);
		SignalDestroy(queue.freeSignal);

		for (uint32_t i = 0; i < queue.maxFrameCount; ++i) {
			AVFrame *frame = queue.frames[i];
			FreeFrame(&frame);
		}
		memory::MemoryFree(queue.frames);
	}
}

struct MediaStream {
	AVCodecContext *codecContext;
	AVCodec *codec;
	int32_t streamIndex;
	bool isValid;
};

// @TODO(final): Bad field layout!
struct MediaState {
	FFMPEGContext *ffmpeg;

	PacketQueue packetQueue;
	AVFormatContext *formatCtx;
	volatile uint32_t readPackets;

	//
	// Video
	//
	MediaStream videoStream;
	FrameQueue videoQueue;
	AVFrame *targetRGBFrame;
	uint8_t *targetRGBBuffer;
	SwsContext *softwareScaleCtx;
	volatile uint32_t decodedVideoFrames;

	//
	// Audio
	//
	MediaStream audioStream;
	FrameQueue audioQueue;
	SwrContext *softwareResampleCtx;
	AVFrame *pendingAudioFrame;

	// @NOTE(final): Buffer holding some amount of samples in the format FPL expects, required for doing conversion using swr_convert().
	uint8_t *conversionAudioBuffer;
	uint32_t maxConversionAudioFrameCount;
	uint32_t maxConversionAudioBufferSize;
	uint32_t conversionAudioFramesRemaining;
	uint32_t conversionAudioFrameIndex;
	volatile uint32_t decodedAudioFrames;
};

// Max number of video frames in the queues
constexpr uint32_t MAX_VIDEO_QUEUE_COUNT = 4;
// Max number of audio frames in the queues
constexpr uint32_t MAX_AUDIO_QUEUE_COUNT = 8;
// Max number of packets in the queues
constexpr uint32_t MAX_PACKET_QUEUE_COUNT = 16;

static int DecodeFrame(MediaState *state, MPMCBoundedQueue<AVPacket *> &availablePacketQueue, FrameQueue &frameQueue, AVCodecContext *avctx, AVFrame *frame, bool *completedFrame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	int ret = AVERROR(EAGAIN);
	AVPacket *pkt;
	*completedFrame = false;
	for (;;) {
		do {
			if (frameQueue.isStopped) {
				return -1;
			}

			switch (avctx->codec_type) {
				case AVMediaType::AVMEDIA_TYPE_VIDEO:
				case AVMediaType::AVMEDIA_TYPE_AUDIO:
				{
					ret = ffmpeg.avcodecReceiveFrame(avctx, frame);
				} break;
			}

			if (ret == AVERROR_EOF) {
				ffmpeg.avcodecFlushBuffers(avctx);
				return AVERROR_EOF;
			}
			if (ret >= 0) {
				*completedFrame = true;
				return 0;
			}
		} while (ret != AVERROR(EAGAIN));

		if (frameQueue.hasPendingPacket) {
			assert(frameQueue.pendingPacket != nullptr);
			pkt = frameQueue.pendingPacket;
			frameQueue.hasPendingPacket = false;
		} else {
			pkt = nullptr;
			if (Dequeue(availablePacketQueue, pkt)) {
				assert(pkt->data != nullptr);
			}
		}

		if (pkt != nullptr) {
			if (ffmpeg.avcodecSendPacket(avctx, pkt) == AVERROR(EAGAIN)) {
				frameQueue.hasPendingPacket = true;
				frameQueue.pendingPacket = pkt;
			} else {
				// Put packet back into the freelist of the packet queue
				ffmpeg.avPacketUnref(pkt);
				assert(Enqueue(state->packetQueue.freeListQueue, pkt));
				SignalWakeUp(state->packetQueue.freePacketSignal);
			}
		}
	}
}

static void VideoDecodingThreadProc(const ThreadContext &thread, void *userData) {
	MediaState *state = (MediaState *)userData;
	FFMPEGContext &ffmpeg = *state->ffmpeg;

	MediaStream &videoStream = state->videoStream;
	assert(videoStream.isValid);
	assert(videoStream.streamIndex > -1);

	ThreadSignal *waitSignals[] = {
		&state->packetQueue.videoPacketsSignal,
		&state->videoQueue.freeSignal,
		&state->videoQueue.stoppedSignal,
	};

	bool aquireNewTargetFrame = true;
	AVFrame *targetFrame = nullptr;
	for (;;) {
		// Wait for either a available video packet signal from the packet queue or a signal from the freelist or a stopped signal from the frame queue
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// When frame queue is stopped exit
		if (state->videoQueue.isStopped) {
			break;
		}

		// Get target frame from the free list if needed
		if (aquireNewTargetFrame) {
			targetFrame = nullptr;
			if (!Dequeue(state->videoQueue.freeListQueue, targetFrame)) {
				continue;
			}
			aquireNewTargetFrame = false;
		}
		assert(targetFrame != nullptr);

		// Decode video frame
		bool frameDecoded = false;
		int decodeResult = DecodeFrame(state, state->packetQueue.availableVideoPacketsQueue, state->videoQueue, videoStream.codecContext, targetFrame, &frameDecoded);
		if (decodeResult < 0) {
			ConsoleFormatError("Video decoder error: %d!\n", decodeResult);
			break;
		}

		if (frameDecoded) {
			// We need a new frame for the next iteration
			aquireNewTargetFrame = true;

			uint32_t decodedVideoFrameIndex = AtomicAddU32(&state->decodedVideoFrames, 1);
			ConsoleFormatOut("Decoded video frame %lu\n", decodedVideoFrameIndex);

			// Put decoded video frame into the available frame queue
			assert(Enqueue(state->videoQueue.availableFramesQueue, targetFrame));
			SignalWakeUp(state->videoQueue.availableFrameSignal);
		}
	}
}

static uint32_t AudioReadCallback(const AudioDeviceFormat &nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	// Intermedite PCM
	// Sample0[Left], Sample0[Right], Sample1[Left], Sample1[Right],...
	// Frame0[Sample Left][Sample Right], Frame1[Sample Left][Sample Right], Frame2[Sample Left][Sample Right],...
	MediaState *state = (MediaState *)userData;
	assert(state != nullptr);
	FFMPEGContext &ffmpeg = *state->ffmpeg;

	uint32_t result = 0;
	if (state->audioStream.isValid) {
		uint8_t *conversionAudioBuffer = state->conversionAudioBuffer;
		uint32_t maxConversionAudioBufferSize = state->maxConversionAudioBufferSize;

		uint32_t outputSampleStride = nativeFormat.channels * audio::GetAudioSampleSizeInBytes(nativeFormat.type);
		uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

		uint32_t remainingFrameCount = frameCount;
		while (remainingFrameCount > 0) {
			// Consume audio in conversion buffer before we do anything else
			if ((state->conversionAudioFramesRemaining) > 0) {
				uint32_t maxFramesToRead = state->conversionAudioFramesRemaining;
				uint32_t framesToRead = FPL_MIN(remainingFrameCount, maxFramesToRead);
				size_t bytesToCopy = framesToRead * outputSampleStride;

				assert(state->conversionAudioFrameIndex < state->maxConversionAudioFrameCount);
				size_t sourcePosition = state->conversionAudioFrameIndex * outputSampleStride;
				assert(sourcePosition < state->maxConversionAudioBufferSize);

				size_t destPosition = (frameCount - remainingFrameCount) * outputSampleStride;
				assert(destPosition < maxOutputSampleBufferSize);

				memory::MemoryCopy(conversionAudioBuffer + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

				remainingFrameCount -= framesToRead;
				state->conversionAudioFrameIndex += framesToRead;
				state->conversionAudioFramesRemaining -= framesToRead;
				result += framesToRead;
			}

			// If we consumed all remaining, so filling out all required audio frames then we are done.
			if (remainingFrameCount == 0) {
				// @NOTE(final): Its highly possible that there are frames left in the conversion buffer, so dont clear anything here!
				break;
			}

			// Convert entire pending frame (Conversion buffer must be empty!)
			if (state->pendingAudioFrame != nullptr) {
				assert(state->conversionAudioFramesRemaining == 0);

				AVFrame *audioFrame = state->pendingAudioFrame;
				uint32_t sourceSampleCount = state->pendingAudioFrame->nb_samples;
				uint32_t sourceChannels = state->pendingAudioFrame->channels;
				uint32_t sourceFrameCount = sourceSampleCount;
				uint8_t **sourceSamples = audioFrame->extended_data;

				// Conversion buffer needs to be big enough to hold the samples from the frame
				uint32_t maxConversionSampleCount = state->maxConversionAudioFrameCount;
				assert(sourceSampleCount <= maxConversionSampleCount);

				int samplesPerChannel = ffmpeg.swrConvert(state->softwareResampleCtx, (uint8_t **)&state->conversionAudioBuffer, maxConversionSampleCount, (const uint8_t **)sourceSamples, sourceSampleCount);

				// Put frame back into the freelist of the frame queue
				state->pendingAudioFrame = nullptr;
				assert(Enqueue(state->audioQueue.freeListQueue, audioFrame));
				SignalWakeUp(state->audioQueue.freeSignal);

				if (samplesPerChannel < 0) {
					// Sample conversion failed, exit audio callback.
					break;
				}

				state->conversionAudioFramesRemaining = samplesPerChannel;
				state->conversionAudioFrameIndex = 0;
			}

			if ((state->pendingAudioFrame == nullptr) && (state->conversionAudioFramesRemaining == 0)) {
				AVFrame *newAudioFrame;
				if (Dequeue(state->audioQueue.availableFramesQueue, newAudioFrame)) {
					state->pendingAudioFrame = newAudioFrame;
					state->conversionAudioFrameIndex = 0;
					state->conversionAudioFramesRemaining = 0;
					continue;
				}
			}
		}
	}
	return(result);
}

static void AudioDecodingThreadProc(const ThreadContext &thread, void *userData) {
	MediaState *state = (MediaState *)userData;
	FFMPEGContext &ffmpeg = *state->ffmpeg;

	MediaStream &audioStream = state->audioStream;
	assert(audioStream.isValid);
	assert(audioStream.streamIndex > -1);

	ThreadSignal *waitSignals[] = {
		&state->packetQueue.audioPacketsSignal,
		&state->audioQueue.freeSignal,
		&state->audioQueue.stoppedSignal,
	};

	bool autoStarted = false;
	bool aquireNewTargetFrame = true;
	AVFrame *targetFrame = nullptr;
	for (;;) {
		// Wait for either a available audio packet signal from the packet queue or a signal from the freelist or a stopped signal from the frame queue
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// When frame queue is stopped exit
		if (state->audioQueue.isStopped) {
			break;
		}

		// Get target frame from the free list if needed
		if (aquireNewTargetFrame) {
			targetFrame = nullptr;
			if (!Dequeue(state->audioQueue.freeListQueue, targetFrame)) {
				continue;
			}
			aquireNewTargetFrame = false;
		}
		assert(targetFrame != nullptr);

		// Decode audio frame
		bool frameDecoded = false;
		int decodeResult = DecodeFrame(state, state->packetQueue.availableAudioPacketsQueue, state->audioQueue, audioStream.codecContext, targetFrame, &frameDecoded);
		if (decodeResult < 0) {
			ConsoleFormatError("Video decoder error: %d!\n", decodeResult);
			break;
		}

		if (frameDecoded) {
			// We need a new frame for the next iteration
			aquireNewTargetFrame = true;

			uint32_t decodedAudioFrameIndex = AtomicAddU32(&state->decodedAudioFrames, 1);
			ConsoleFormatOut("Decoded audio frame %lu\n", decodedAudioFrameIndex);

			// Put decoded audio frame into the available frame queue
			assert(Enqueue(state->audioQueue.availableFramesQueue, targetFrame));
			SignalWakeUp(state->audioQueue.availableFrameSignal);

			// Start audio
			if (!autoStarted) {
				autoStarted = true;
				SetAudioClientReadCallback(AudioReadCallback, state);
				PlayAudio();
			}
		}
	}
}

static void PacketReadThreadProc(const ThreadContext &thread, void *userData) {
	MediaState *state = (MediaState *)userData;
	FFMPEGContext &ffmpeg = *state->ffmpeg;

	MediaStream &videoStream = state->videoStream;
	MediaStream &audioStream = state->audioStream;

	ThreadSignal *waitSignals[] = {
		&state->packetQueue.freePacketSignal,
		&state->packetQueue.stoppedSignal,
	};

	bool skipWait = true;
	for (;;) {
		if (skipWait) {
			skipWait = false;
		} else {
			// Wait for either a signal from the free list or an stopped signal from the packet queue
			SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));
		}

		// When packet queue is stopped exit
		if (state->packetQueue.isStopped) {
			break;
		}

		// Try to get new packet from the freelist
		AVPacket *packet = nullptr;
		if (Dequeue(state->packetQueue.freeListQueue, packet)) {
			assert(packet != nullptr);

			uint32_t packetIndex = AtomicAddU32(&state->readPackets, 1);
			ConsoleFormatOut("Read packet %lu\n", packetIndex);

			// Read packet
			int res = ffmpeg.avReadFrame(state->formatCtx, packet);
			if (res >= 0) {
				if (videoStream.isValid && (packet->stream_index == videoStream.streamIndex)) {
					ConsoleFormatOut("Added video packet %lu\n", packetIndex);
					// Put video packet into the available video packet queue
					assert(Enqueue(state->packetQueue.availableVideoPacketsQueue, packet));
					SignalWakeUp(state->packetQueue.videoPacketsSignal);
				} else if (audioStream.isValid && (packet->stream_index == audioStream.streamIndex)) {
					ConsoleFormatOut("Added audio packet %lu\n", packetIndex);
					// Put audio packet into the available audio packet queue
					assert(Enqueue(state->packetQueue.availableAudioPacketsQueue, packet));
					SignalWakeUp(state->packetQueue.audioPacketsSignal);
				} else {
					// Drop packet
					ConsoleFormatOut("Dropped packet %lu\n", packetIndex);

					// Put packet back into the freelist of the packet queue
					ffmpeg.avPacketUnref(packet);
					assert(Enqueue(state->packetQueue.freeListQueue, packet));
					SignalWakeUp(state->packetQueue.freePacketSignal);
				}

				// Skip next wait, because there might be more packets in the freelist
				skipWait = true;
			} else {
				// Error or stream is done, put packet back into free list, but then exit.
				ConsoleFormatOut("Error or stream is done for packet %lu\n", packetIndex);

				// Put packet back into the freelist of the packet queue
				assert(Enqueue(state->packetQueue.freeListQueue, packet));
				SignalWakeUp(state->packetQueue.freePacketSignal);

				// We exit the loop, we are done.
				break;
			}
		}
	}
}

static bool LoadStream(const char *mediaFilePath, AVStream *stream, MediaStream &outStream) {
	FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	// Get codec name
	char codecName[5] = {};
	MemoryCopy(&stream->codecpar->codec_tag, 4, codecName);

	// Determine codec type name
	const char *typeName;
	switch (stream->codecpar->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			typeName = "Video";
			break;
		case AVMEDIA_TYPE_AUDIO:
			typeName = "Audio";
			break;
		default:
			assert(!"Unsupported stream type!");
	}

	// Create codec context
	outStream.codecContext = ffmpeg.avcodecAllocContext3(nullptr);
	if (ffmpeg.avcodecParametersToContext(outStream.codecContext, stream->codecpar) < 0) {
		ConsoleFormatError("Failed getting %s codec context from codec '%s' in media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// Find decoder
	// @TODO(final): We could force the codec here if we want.
	outStream.codec = ffmpeg.avcodecFindDecoder(stream->codecpar->codec_id);
	if (outStream.codec == nullptr) {
		ConsoleFormatError("Unsupported %s codec '%s' in media file '%s' found!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// Open codec
	if (ffmpeg.avcodecOpen2(outStream.codecContext, outStream.codec, nullptr) < 0) {
		ConsoleFormatError("Failed opening %s codec '%s' from media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	outStream.isValid = true;

	return true;
}

int main(int argc, char **argv) {
	int result = 0;

	if (argc < 2) {
		ConsoleError("Media file argument missing!");
		return -1;
	}

	const char *mediaFilePath = argv[1];

	Settings settings = DefaultSettings();
	settings.video.driverType = VideoDriverType::Software;
	settings.video.isAutoSize = false;

	if (InitPlatform(InitFlags::All, settings)) {
		globalFFMPEGFunctions = (FFMPEGContext *)MemoryAlignedAllocate(sizeof(FFMPEGContext), 16);
		FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

		MediaState state = {};
		state.ffmpeg = &ffmpeg;

		VideoBackBuffer *backBuffer = GetVideoBackBuffer();
		AudioDeviceFormat nativeAudioFormat = GetAudioNativeFormat();

		//
		// Load ffmpeg libraries
		//
		if (!LoadFFMPEG(ffmpeg)) {
			goto release;
		}

		// Register all formats and codecs
		ffmpeg.avRegisterAll();

		// @TODO(final): Custom IO!

		// Open media file
		if (ffmpeg.avformatOpenInput(&state.formatCtx, mediaFilePath, nullptr, nullptr) != 0) {
			ConsoleFormatError("Failed opening media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Retrieve stream information
		if (ffmpeg.avformatFindStreamInfo(state.formatCtx, nullptr) < 0) {
			ConsoleFormatError("Failed getting stream informations for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Dump information about file onto standard error
		ffmpeg.avDumpFormat(state.formatCtx, 0, mediaFilePath, 0);

		// Find the first streams
		state.videoStream.streamIndex = -1;
		state.audioStream.streamIndex = -1;
		for (uint32_t steamIndex = 0; steamIndex < state.formatCtx->nb_streams; steamIndex++) {
			AVStream *stream = state.formatCtx->streams[steamIndex];
			switch (stream->codecpar->codec_type) {
				case AVMEDIA_TYPE_VIDEO:
				{
					if (state.videoStream.streamIndex == -1) {
						if (LoadStream(mediaFilePath, stream, state.videoStream)) {
							state.videoStream.streamIndex = steamIndex;
						}
					}
				} break;
				case AVMEDIA_TYPE_AUDIO:
				{
					if (state.audioStream.streamIndex == -1) {
						if (LoadStream(mediaFilePath, stream, state.audioStream)) {
							state.audioStream.streamIndex = steamIndex;
						}
					}
				} break;
			}
		}

		// No streams found
		if ((!state.videoStream.isValid) && (!state.audioStream.isValid)) {
			ConsoleFormatError("No video or audio stream in media file '%s' found!\n", mediaFilePath);
			goto release;
		}

		// Allocate audio related resources
		if (state.audioStream.isValid) {
			AVCodecContext *audioCodexCtx = state.audioStream.codecContext;

			// @TODO(final): Map target audio format to FFMPEG
			assert(nativeAudioFormat.type == AudioFormatType::S16);
			AVSampleFormat targetSampleFormat = AV_SAMPLE_FMT_S16;
			int targetChannelCount = nativeAudioFormat.channels;
			// @TODO(final): Map target audio channels to channel layout
			uint64_t targetChannelLayout = AV_CH_LAYOUT_STEREO;
			assert(targetChannelCount == 2);
			int targetSampleRate = nativeAudioFormat.sampleRate;

			AVSampleFormat inputSampleFormat = audioCodexCtx->sample_fmt;
			int inputChannelCount = audioCodexCtx->channels;
			// @TODO(final): Map input audio channels to channel layout
			uint64_t inputChannelLayout = AV_CH_LAYOUT_STEREO;
			int inputSampleRate = audioCodexCtx->sample_rate;
			assert(inputChannelCount == 2);

			// Create software resample context and initialize
			state.softwareResampleCtx = ffmpeg.swrAllocSetOpts(nullptr,
															   targetChannelLayout,
															   targetSampleFormat,
															   targetSampleRate,
															   inputChannelLayout,
															   inputSampleFormat,
															   inputSampleRate,
															   0,
															   nullptr);
			ffmpeg.swrInit(state.softwareResampleCtx);

			// Allocate conversion buffer in native format, this must be big enough to hold one AVFrame worth of data.
			int lineSize;
			state.maxConversionAudioBufferSize = ffmpeg.avSamplesGetBufferSize(&lineSize, targetChannelCount, targetSampleRate, targetSampleFormat, 1);
			state.maxConversionAudioFrameCount = state.maxConversionAudioBufferSize / audio::GetAudioSampleSizeInBytes(nativeAudioFormat.type) / targetChannelCount;
			state.conversionAudioBuffer = (uint8_t *)memory::MemoryAlignedAllocate(state.maxConversionAudioBufferSize, 16);
			state.conversionAudioFrameIndex = 0;
			state.conversionAudioFramesRemaining = 0;
		}

		// Allocate video related resources
		if (state.videoStream.isValid) {
			AVCodecContext *videoCodexCtx = state.videoStream.codecContext;

			// Allocate RGB video frame
			state.targetRGBFrame = ffmpeg.avFrameAlloc();
			if (state.targetRGBFrame == nullptr) {
				ConsoleFormatError("Failed allocating RGB video frame for media file '%s'!\n", mediaFilePath);
				goto release;
			}

			// Allocate RGB buffer
			AVPixelFormat targetPixelFormat = AVPixelFormat::AV_PIX_FMT_BGR24;
			size_t rgbFrameSize = ffmpeg.avImageGetBufferSize(targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);
			state.targetRGBBuffer = (uint8_t *)MemoryAlignedAllocate(rgbFrameSize, 16);

			// Setup RGB video frame and give it access to the actual data
			ffmpeg.avImageFillArrays(state.targetRGBFrame->data, state.targetRGBFrame->linesize, state.targetRGBBuffer, targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);

			// Get software context
			state.softwareScaleCtx = ffmpeg.swsGetContext(
				videoCodexCtx->width,
				videoCodexCtx->height,
				videoCodexCtx->pix_fmt,
				videoCodexCtx->width,
				videoCodexCtx->height,
				targetPixelFormat,
				SWS_BILINEAR,
				nullptr,
				nullptr,
				nullptr
			);

			// Resize backbuffer to fit the video size
			ResizeVideoBackBuffer(videoCodexCtx->width, videoCodexCtx->height);
		}

		// Create queues
		state.packetQueue = CreatePacketQueue(MAX_PACKET_QUEUE_COUNT);
		if (state.videoStream.isValid) {
			state.videoQueue = CreateFrameQueue(MAX_VIDEO_QUEUE_COUNT);
		}
		if (state.audioStream.isValid) {
			state.audioQueue = CreateFrameQueue(MAX_AUDIO_QUEUE_COUNT);
		}

		// Create threads
		uint32_t threadCount = 0;
		ThreadContext *threads[3] = {};
		threads[threadCount++] = ThreadCreate(PacketReadThreadProc, &state);
		if (state.videoQueue.isValid) {
			threads[threadCount++] = ThreadCreate(VideoDecodingThreadProc, &state);
		}
		if (state.audioQueue.isValid) {
			threads[threadCount++] = ThreadCreate(AudioDecodingThreadProc, &state);
		}

		//
		// App loop
		//
		while (WindowUpdate()) {
			if (state.videoStream.isValid) {
				// Get available frame in native format from the queue
				AVCodecContext *videoCodecCtx = state.videoStream.codecContext;
				AVFrame *sourceNativeFrame = nullptr;
				if (Dequeue(state.videoQueue.availableFramesQueue, sourceNativeFrame)) {
					assert(sourceNativeFrame != nullptr);

					// @TODO(final): Decode picture format directly into the backbuffer, without the software scaling!

					// Convert native frame to target RGB24 frame
					ffmpeg.swsScale(state.softwareScaleCtx, (uint8_t const * const *)sourceNativeFrame->data, sourceNativeFrame->linesize, 0, videoCodecCtx->height, state.targetRGBFrame->data, state.targetRGBFrame->linesize);

					// Put native frame back into the freelist of the frame queue
					assert(Enqueue(state.videoQueue.freeListQueue, sourceNativeFrame));
					SignalWakeUp(state.videoQueue.freeSignal);

					// Convert RGB24 frame to RGB32 backbuffer
					ConvertRGB24ToBackBuffer(backBuffer, videoCodecCtx->width, videoCodecCtx->height, *state.targetRGBFrame->linesize, state.targetRGBBuffer);
				}
			}

			// Present frame
			WindowFlip();
		}

		// Stop audio
		if (state.audioStream.isValid) {
			StopAudio();
		}

		// Stop queues
		state.packetQueue.isStopped = 1;
		SignalWakeUp(state.packetQueue.stoppedSignal);
		if (state.videoQueue.isValid) {
			state.videoQueue.isStopped = 1;
			SignalWakeUp(state.videoQueue.stoppedSignal);
		}
		if (state.audioQueue.isValid) {
			state.audioQueue.isStopped = 1;
			SignalWakeUp(state.audioQueue.stoppedSignal);
		}

		// Wait until all threads are finished running and release all threads
		ThreadWaitForAll(threads, threadCount);
		for (uint32_t threadIndex = threadCount - 1; threadIndex > 0; threadIndex--) {
			ThreadDestroy(threads[threadIndex]);
		}

		// Release queues
		DestroyFrameQueue(state.audioQueue);
		DestroyFrameQueue(state.videoQueue);
		DestroyPacketQueue(state.packetQueue);

	release:
		// Release media
		if (state.conversionAudioBuffer != nullptr) {
			memory::MemoryAlignedFree(state.conversionAudioBuffer);
		}

		if (state.softwareResampleCtx != nullptr) {
			ffmpeg.swrFree(&state.softwareResampleCtx);
		}

		if (state.softwareScaleCtx != nullptr) {
			ffmpeg.swsFreeContext(state.softwareScaleCtx);
		}
		if (state.targetRGBBuffer != nullptr) {
			MemoryAlignedFree(state.targetRGBBuffer);
		}
		if (state.targetRGBFrame != nullptr) {
			ffmpeg.avFrameFree(&state.targetRGBFrame);
		}

		if (state.audioStream.codecContext != nullptr) {
			ffmpeg.avcodecFreeContext(&state.audioStream.codecContext);
		}
		if (state.videoStream.codecContext != nullptr) {
			ffmpeg.avcodecFreeContext(&state.videoStream.codecContext);
		}

		if (state.formatCtx != nullptr) {
			ffmpeg.avformatCloseInput(&state.formatCtx);
		}

		//
		// Release FFMPEG
		//
		ReleaseFFMPEG(ffmpeg);
		MemoryAlignedFree(globalFFMPEGFunctions);

		// Release platform
		ReleasePlatform();

		result = 0;
	} else {
		result = -1;
	}

	return(result);
}