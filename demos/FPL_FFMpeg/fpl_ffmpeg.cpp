/*

Custom FFMPEG Media Player Example based on FPL
Written by Torsten Spaete

[x] Reads packets from stream and queues them up
[x] Decodes video and audio packets and queues them up as well
[x] FFMPEG functions are loaded dynamically
[x] Linked list for packet queue
[x] Handle PTS/DTS to schedule video frame
[x] Syncronize video to audio
[x] Fix memory leak (There was no leak at all)
[ ] Rewrite Frame Queue to support Peek in Previous, Current and Next frame
[ ] Frame drop: Introduce serial as well
[ ] Syncronize audio to video
[ ] Pause/Resume
[ ] Restart
[ ] Seeking
[ ] Fix Bug when audio starts very late
[ ] Growing packet queue
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
#include "mpmc_queue.h"

#define PRINT_QUEUE_INFOS 0

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
// av_mallocz
#define FFMPEG_AV_MALLOCZ_FUNC(name) void *name(size_t size) av_malloc_attrib av_alloc_size(1)
typedef FFMPEG_AV_MALLOCZ_FUNC(ffmpeg_av_mallocz_func);
// av_freep
#define FFMPEG_AV_FREEP_FUNC(name) void name(void *ptr)
typedef FFMPEG_AV_FREEP_FUNC(ffmpeg_av_freep_func);

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
	if (target == fpl_null) { \
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
	ffmpeg_av_gettime_func *avGetTime;
	ffmpeg_av_get_media_type_string_func *avGetMediaTypeString;
	ffmpeg_av_rescale_q_func *avRescaleQ;
	ffmpeg_av_samples_get_buffer_size_func *avSamplesGetBufferSize;
	ffmpeg_av_mallocz_func *avMallocZ;
	ffmpeg_av_freep_func *avFreeP;

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
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetTime, ffmpeg_av_gettime_func, "av_gettime");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetMediaTypeString, ffmpeg_av_get_media_type_string_func, "av_get_media_type_string");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avRescaleQ, ffmpeg_av_rescale_q_func, "av_rescale_q");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avSamplesGetBufferSize, ffmpeg_av_samples_get_buffer_size_func, "av_samples_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avMallocZ, ffmpeg_av_mallocz_func, "av_mallocz");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFreeP, ffmpeg_av_freep_func, "av_freep");

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

static FFMPEGContext *globalFFMPEGFunctions = fpl_null;

//
// Stats
//
struct MemoryStats {
	volatile fpl_s32 allocatedPackets;
	volatile fpl_s32 referencedPackets;
	volatile fpl_s32 allocatedFrames;
	volatile fpl_s32 referencedFrames;
};

static MemoryStats globalMemStats = {};

inline void PrintMemStats() {
	ConsoleFormatOut("Packets: %d / %d, Frames: %d / %d\n", globalMemStats.allocatedPackets, globalMemStats.referencedPackets, globalMemStats.allocatedFrames, globalMemStats.referencedFrames);
}

//
// Packet Queue
//
struct PacketList {
	AVPacket packet;
	PacketList *next;
};

struct PacketQueue {
	ThreadMutex lock;
	ThreadSignal addedSignal;
	ThreadSignal freeSignal;
	PacketList *first;
	PacketList *last;
	uint64_t size;
	uint64_t duration;
	int32_t packetCount;
};

inline PacketList *AllocatePacket(PacketQueue &queue) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	PacketList *packet = (PacketList *)ffmpeg.avMallocZ(sizeof(PacketList));
	if (packet == fpl_null) {
		return fpl_null;
	}
	ffmpeg.avInitPacket(&packet->packet);
	AtomicAddS32(&globalMemStats.allocatedPackets, 1);
	return(packet);
}

inline void DestroyPacket(PacketQueue &queue, PacketList *packet) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ffmpeg.avPacketUnref(&packet->packet);
	AtomicAddS32(&globalMemStats.referencedPackets, -1);

	ffmpeg.avFreeP(packet);
	AtomicAddS32(&globalMemStats.allocatedPackets, -1);
}

static void DestroyPacketQueue(PacketQueue &queue) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	PacketList *p = queue.first;
	while (p != fpl_null) {
		PacketList *n = p->next;
		DestroyPacket(queue, p);
		p = n;
	}
	queue.first = queue.last = fpl_null;
	queue.packetCount = 0;
	queue.size = 0;

	if (queue.freeSignal.isValid) {
		SignalDestroy(queue.freeSignal);
	}
	if (queue.addedSignal.isValid) {
		SignalDestroy(queue.addedSignal);
	}
	MutexDestroy(queue.lock);
}

inline bool InitPacketQueue(PacketQueue &queue) {
	queue.lock = MutexCreate();
	if (!queue.lock.isValid) {
		return false;
	}
	queue.addedSignal = SignalCreate();
	if (!queue.addedSignal.isValid) {
		return false;
	}
	queue.freeSignal = SignalCreate();
	if (!queue.freeSignal.isValid) {
		return false;
	}
	return true;
}

inline void ReleasePacket(PacketQueue &queue, PacketList *packet) {
	// @TODO(final): Growing packet arena!
	DestroyPacket(queue, packet);
	SignalWakeUp(queue.freeSignal);
}

inline bool AquirePacket(PacketQueue &queue, PacketList *&packet) {
	// @TODO(final): Growing packet arena!
	bool result = false;
	packet = AllocatePacket(queue);
	if (packet != fpl_null) {
		result = true;
	}
	return(result);
}

inline void PushPacket(PacketQueue &queue, PacketList *packet) {
	MutexLock(queue.lock);
	{
		packet->next = fpl_null;
		if (queue.first == fpl_null) {
			queue.first = packet;
		}
		if (queue.last != fpl_null) {
			assert(queue.last->next == fpl_null);
			queue.last->next = packet;
		}
		queue.last = packet;
		queue.size += packet->packet.size + sizeof(*packet);
		queue.duration += packet->packet.duration;
		AtomicAddS32(&queue.packetCount, 1);
		SignalWakeUp(queue.addedSignal);
	}
	MutexUnlock(queue.lock);
}

inline bool PopPacket(PacketQueue &queue, PacketList *&packet) {
	bool result = false;
	MutexLock(queue.lock);
	{
		if (queue.first != fpl_null) {
			PacketList *p = queue.first;
			PacketList *n = p->next;
			queue.first = n;
			result = true;
			p->next = fpl_null;
			packet = p;
			queue.duration -= packet->packet.duration;
			queue.size -= packet->packet.size + sizeof(*packet);
		}
		if (queue.first == fpl_null) {
			queue.last = fpl_null;
		}
		AtomicAddS32(&queue.packetCount, -1);
	}
	MutexUnlock(queue.lock);
	return(result);
}

//
// Frame Queue
//
struct FrameInfo {
	double pts;
	double duration;
};
struct Frame {
	AVFrame *frame;
	FrameInfo infos;
};

struct FrameQueue {
	PacketList *pendingPacket;
	Frame *frames;
	MPMCBoundedQueue<Frame *> freeListQueue;
	MPMCBoundedQueue<Frame *> availableFramesQueue;
	uint32_t maxFrameCount;
	bool isValid;
	bool hasPendingPacket;
};

inline AVFrame *AllocateFrame() {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *result = ffmpeg.avFrameAlloc();
	AtomicAddS32(&globalMemStats.allocatedFrames, 1);
	return(result);
}

inline void FreeFrameData(Frame *frame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.avFrameUnref(frame->frame);
}

inline void FreeFrame(Frame *frame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	FreeFrameData(frame);
	ffmpeg.avFrameFree(&frame->frame);
}

static FrameQueue CreateFrameQueue(uint32_t capacity) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	FrameQueue result = {};
	result.maxFrameCount = (uint32_t)RoundToPowerOfTwo(capacity);
	result.frames = (Frame *)memory::MemoryAllocate(result.maxFrameCount * sizeof(Frame));
	result.freeListQueue = MPMCBoundedQueue<Frame *>::Create(result.maxFrameCount);
	result.availableFramesQueue = MPMCBoundedQueue<Frame *>::Create(result.maxFrameCount);
	for (uint32_t i = 0; i < result.maxFrameCount; ++i) {
		Frame *frame = result.frames + i;
		frame->frame = AllocateFrame();
		if (!Enqueue(result.freeListQueue, frame)) {
			assert(!"Not enough capacity in freelist, increase buffer size!");
		}
	}
	result.isValid = true;
	return(result);
}

static void DestroyFrameQueue(FrameQueue &queue) {
	if (queue.isValid) {
		MPMCBoundedQueue<Frame *>::Destroy(queue.availableFramesQueue);
		MPMCBoundedQueue<Frame *>::Destroy(queue.freeListQueue);
		for (uint32_t i = 0; i < queue.maxFrameCount; ++i) {
			Frame *frame = queue.frames + i;
			FreeFrame(frame);
		}
		memory::MemoryFree(queue.frames);
	}
}

struct MediaStream {
	AVStream *stream;
	AVCodecContext *codecContext;
	AVCodec *codec;
	int32_t streamIndex;
	bool isValid;
};

struct ReaderContext {
	PacketQueue packetQueue;
	ThreadSignal stopSignal;
	ThreadSignal resumeSignal;
	volatile uint32_t readPacketCount;
	volatile uint32_t stopRequest;
	volatile uint32_t isDone;
};

static bool InitReader(ReaderContext &outReader) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	outReader = {};
	outReader.stopSignal = SignalCreate();
	if (!outReader.stopSignal.isValid) {
		return false;
	}
	outReader.resumeSignal = SignalCreate();
	if (!outReader.resumeSignal.isValid) {
		return false;
	}
	if (!InitPacketQueue(outReader.packetQueue)) {
		return false;
	}
	return true;
}

static void DestroyReader(ReaderContext &reader) {
	DestroyPacketQueue(reader.packetQueue);
	SignalDestroy(reader.resumeSignal);
	SignalDestroy(reader.stopSignal);
}

static void StopReader(ReaderContext &reader) {
	reader.stopRequest = 1;
	SignalWakeUp(reader.stopSignal);
}

struct PlayerState;
struct Decoder {
	PacketQueue packetsQueue;
	FrameQueue frameQueue;
	ThreadSignal stopSignal;
	ThreadSignal resumeSignal;
	ThreadSignal freeFrameSignal;
	ThreadSignal decodedFrameSignal;
	PlayerState *state;
	ReaderContext *reader;
	MediaStream *stream;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	volatile uint32_t stopRequest;
	volatile uint32_t isDone;
	volatile uint32_t decodedFrameCount;
};

static bool InitDecoder(Decoder &outDecoder, PlayerState *state, ReaderContext *reader, MediaStream *stream, uint32_t frameCapacity) {
	outDecoder = {};
	outDecoder.stream = stream;
	outDecoder.reader = reader;
	outDecoder.state = state;
	outDecoder.stopSignal = SignalCreate();
	if (!outDecoder.stopSignal.isValid) {
		return false;
	}
	outDecoder.resumeSignal = SignalCreate();
	if (!outDecoder.resumeSignal.isValid) {
		return false;
	}
	outDecoder.decodedFrameSignal = SignalCreate();
	if (!outDecoder.decodedFrameSignal.isValid) {
		return false;
	}
	outDecoder.freeFrameSignal = SignalCreate();
	if (!outDecoder.freeFrameSignal.isValid) {
		return false;
	}
	if (!InitPacketQueue(outDecoder.packetsQueue)) {
		return false;
	}
	outDecoder.frameQueue = CreateFrameQueue(frameCapacity);

	outDecoder.start_pts = stream->stream->start_time;
	outDecoder.start_pts_tb = stream->stream->time_base;

	return true;
}

static void DestroyDecoder(Decoder &decoder) {
	DestroyFrameQueue(decoder.frameQueue);
	DestroyPacketQueue(decoder.packetsQueue);
	if (decoder.freeFrameSignal.isValid) {
		SignalDestroy(decoder.freeFrameSignal);
	}
	if (decoder.decodedFrameSignal.isValid) {
		SignalDestroy(decoder.decodedFrameSignal);
	}
	if (decoder.resumeSignal.isValid) {
		SignalDestroy(decoder.resumeSignal);
	}
	if (decoder.stopSignal.isValid) {
		SignalDestroy(decoder.stopSignal);
	}
}

static void StopDecoder(Decoder &decoder) {
	decoder.stopRequest = 1;
	if (decoder.stopSignal.isValid) {
		SignalWakeUp(decoder.stopSignal);
	}
}

static void AddPacketToDecoder(Decoder &decoder, PacketList *packet) {
	PushPacket(decoder.packetsQueue, packet);
}

static void PutFrameBackToDecoder(Decoder &decoder, Frame *frame) {
	FreeFrameData(frame);
	assert(Enqueue(decoder.frameQueue.freeListQueue, frame));
	SignalWakeUp(decoder.freeFrameSignal);
}

static void AddDecodedFrameToDecoder(Decoder &decoder, Frame *frame) {
	assert(Enqueue(decoder.frameQueue.availableFramesQueue, frame));
	SignalWakeUp(decoder.decodedFrameSignal);
}

//
// Clock
//
struct Clock {
	double pts;
	double ptsDrift;
	double lastUpdated;
	double speed;
	bool isPaused;
};
namespace AVSyncTypes {
	enum AVSyncTypeEnum {
		AudioMaster,
		VideoMaster,
		ExternalClock,
	};
};
typedef AVSyncTypes::AVSyncTypeEnum AVSyncType;

//
// Video
//
struct VideoContext {
	MediaStream stream;
	Decoder decoder;
	Clock clock;
	VideoBackBuffer *backBuffer;
	AVFrame *targetRGBFrame;
	uint8_t *targetRGBBuffer;
	SwsContext *softwareScaleCtx;
	Frame *waitingFrame;
	FrameInfo prevFrameInfo;
	FrameInfo nextFrameInfo;
};

//
// Audio
//
struct AudioContext {
	MediaStream stream;
	Decoder decoder;
	Clock clock;
	double audioClock;

	SwrContext *softwareResampleCtx;
	Frame *pendingAudioFrame;

	// @NOTE(final): Buffer holding some amount of samples in the format FPL expects, required for doing conversion using swr_convert().
	uint8_t *conversionAudioBuffer;
	uint32_t maxConversionAudioFrameCount;
	uint32_t maxConversionAudioBufferSize;
	uint32_t conversionAudioFramesRemaining;
	uint32_t conversionAudioFrameIndex;
};

struct PlayerPosition {
	bool isValid;
	int64_t value;
};

struct PlayerSettings {
	PlayerPosition startTime;
	PlayerPosition duration;
	bool useInfiniteBuffer;
};

inline void InitPlayerSettings(PlayerSettings &settings) {
	settings.startTime = {};
	settings.duration = {};
	settings.useInfiniteBuffer = false;
}

fpl_constant uint32_t MAX_STREAM_COUNT = 8;
struct PlayerState {
	ReaderContext reader;
	MediaStream stream[MAX_STREAM_COUNT];
	VideoContext video;
	AudioContext audio;
	PlayerSettings settings;
	Clock externalClock;
	AVFormatContext *formatCtx;
	double frameLastPTS;
	double frameLastDelay;
	double frameTimer;
	double maxFrameDuration;
	AVSyncType syncType;
	volatile uint32_t forceRefresh;
	bool isInfiniteBuffer;
	bool isRealTime;
	bool isPaused;
};

// Max number of frames in the queues
fpl_constant uint32_t MAX_VIDEO_FRAME_QUEUE_COUNT = 4;
fpl_constant uint32_t MAX_AUDIO_FRAME_QUEUE_COUNT = 8;

// Total size of data from all packet queues
fpl_constant uint64_t MAX_PACKET_QUEUE_SIZE = FPL_MEGABYTES(16);

// Min number of packet frames in a single queue
fpl_constant uint32_t MIN_PACKET_FRAMES = 25;

// External clock min/max frames
fpl_constant uint32_t EXTERNAL_CLOCK_MIN_FRAMES = 2;
fpl_constant uint32_t EXTERNAL_CLOCK_MAX_FRAMES = 10;

// External clock speed adjustment constants for realtime sources based on buffer fullness
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

// No AV sync correction is done if below the minimum AV sync threshold
fpl_constant double AV_SYNC_THRESHOLD_MIN = 0.04;
// No AV sync correction is done if above the maximum AV sync threshold
fpl_constant double AV_SYNC_THRESHOLD_MAX = 0.1;
// No AV correction is done if too big error
fpl_constant double AV_NOSYNC_THRESHOLD = 10.0;
// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
fpl_constant double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1;
// Default refresh rate of 1/sec
fpl_constant double DEFAULT_REFRESH_RATE = 0.01;

inline void PutPacketBackToReader(ReaderContext &reader, PacketList *packet) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ReleasePacket(reader.packetQueue, packet);
}

inline bool StreamHasEnoughPackets(const AVStream *stream, int streamIndex, const PacketQueue &queue) {
	bool result = (streamIndex < 0) ||
		(stream->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
		((queue.packetCount > MIN_PACKET_FRAMES) && (!queue.duration || (av_q2d(stream->time_base) * queue.duration) > 1.0));
	return (result);
}

inline AVSyncType GetMasterSyncType(PlayerState *state) {
	if (state->syncType == AVSyncType::VideoMaster) {
		if (state->video.stream.isValid) {
			return AVSyncType::VideoMaster;
		} else {
			return AVSyncType::AudioMaster;
		}
	} else if (state->syncType == AVSyncType::AudioMaster) {
		if (state->audio.stream.isValid) {
			return AVSyncType::AudioMaster;
		} else {
			return AVSyncType::ExternalClock;
		}
	} else {
		return AVSyncType::ExternalClock;
	}
}

inline double GetClock(Clock &clock) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	double result;
	if (clock.isPaused) {
		result = clock.pts;
	} else {
		double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
		result = clock.ptsDrift + time - (time - clock.lastUpdated) * (1.0 - clock.speed);
	}
	return(result);
}

inline void SetClockAt(Clock &clock, double pts, double time) {
	clock.pts = pts;
	clock.lastUpdated = time;
	clock.ptsDrift = clock.pts - clock.lastUpdated;
}

inline void SetClock(Clock &clock, double pts) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
	SetClockAt(clock, pts, time);
}

inline void SetClockSpeed(Clock &clock, double speed) {
	SetClock(clock, GetClock(clock));
	clock.speed = speed;
}

inline void InitClock(Clock &clock) {
	clock.speed = 1.0;
	clock.isPaused = false;
	SetClock(clock, NAN);
}

inline void SyncClockToSlave(Clock &c, Clock &slave) {
	double clock = GetClock(c);
	double slaveClock = GetClock(slave);
	if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > AV_NOSYNC_THRESHOLD)) {
		SetClock(c, slaveClock);
	}
}

inline double GetMasterClock(PlayerState *state) {
	double val;
	switch (GetMasterSyncType(state)) {
		case AVSyncType::VideoMaster:
			val = GetClock(state->video.clock);
			break;
		case AVSyncType::AudioMaster:
			val = GetClock(state->audio.clock);
			break;
		default:
			val = GetClock(state->externalClock);
			break;
	}
	return val;
}

static void UpdateExternalClockSpeed(PlayerState *state) {
	if ((state->video.stream.isValid && state->video.decoder.packetsQueue.packetCount <= EXTERNAL_CLOCK_MIN_FRAMES) ||
		(state->audio.stream.isValid && state->audio.decoder.packetsQueue.packetCount <= EXTERNAL_CLOCK_MIN_FRAMES)) {
		SetClockSpeed(state->externalClock, FPL_MAX(EXTERNAL_CLOCK_SPEED_MIN, state->externalClock.speed - EXTERNAL_CLOCK_SPEED_STEP));
	} else if ((!state->video.stream.isValid || (state->video.decoder.packetsQueue.packetCount > EXTERNAL_CLOCK_MAX_FRAMES)) &&
		(!state->audio.stream.isValid || (state->audio.decoder.packetsQueue.packetCount > EXTERNAL_CLOCK_MAX_FRAMES))) {
		SetClockSpeed(state->externalClock, FPL_MIN(EXTERNAL_CLOCK_SPEED_MAX, state->externalClock.speed + EXTERNAL_CLOCK_SPEED_STEP));
	} else {
		double speed = state->externalClock.speed;
		if (speed != 1.0) {
			SetClockSpeed(state->externalClock, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
		}
	}
}

namespace DecodeResults {
	enum DecodeResultEnum {
		Failed = -99,
		Stopped = -1,
		Success = 0,
		RequireMorePackets,
		EndOfStream,
		Skipped,
	};
};
typedef DecodeResults::DecodeResultEnum DecodeResult;

static DecodeResult DecodeFrame(ReaderContext &reader, Decoder &decoder, Frame *outFrame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	assert(decoder.stream != fpl_null);
	AVCodecContext *codecCtx = decoder.stream->codecContext;
	int ret = AVERROR(EAGAIN);
	PacketList *pkt;
	for (;;) {
		do {
			if (decoder.isDone) {
				return DecodeResult::Skipped;
			}
			if (decoder.stopRequest) {
				return DecodeResult::Stopped;
			}

			AVFrame *frame = outFrame->frame;

			switch (codecCtx->codec_type) {
				case AVMediaType::AVMEDIA_TYPE_VIDEO:
				{
					//avcodec_receive_frame
					ret = ffmpeg.avcodecReceiveFrame(codecCtx, frame);
					if (ret == 0) {
						frame->pts = frame->best_effort_timestamp;
					} else if (ret == AVERROR(EAGAIN)) {
						// This will continue sending packets until the frame is complete
						break;
					}
				} break;

				case AVMediaType::AVMEDIA_TYPE_AUDIO:
				{
					ret = ffmpeg.avcodecReceiveFrame(codecCtx, frame);
					if (ret == 0) {
						AVRational tb = { 1, frame->sample_rate };
						if (frame->pts != AV_NOPTS_VALUE) {
							frame->pts = ffmpeg.avRescaleQ(frame->pts, codecCtx->pkt_timebase, tb);
						} else if (decoder.next_pts != AV_NOPTS_VALUE) {
							frame->pts = ffmpeg.avRescaleQ(decoder.next_pts, decoder.next_pts_tb, tb);
						}
						if (frame->pts != AV_NOPTS_VALUE) {
							decoder.next_pts = frame->pts + frame->nb_samples;
							decoder.next_pts_tb = tb;
						}
					} else if (ret == AVERROR(EAGAIN)) {
						// This will continue sending packets until the frame is complete
						break;
					}
				} break;
			}
			if (ret == 0) {
				return DecodeResult::Success;
			} else if (ret == AVERROR_EOF) {
				ffmpeg.avcodecFlushBuffers(codecCtx);
				return DecodeResult::EndOfStream;
			} else if (ret == AVERROR(EAGAIN)) {
				// This will continue sending packets until the frame is complete
				break;
			} else {
				return DecodeResult::Failed;
			}
		} while (ret != AVERROR(EAGAIN));

		if (decoder.frameQueue.hasPendingPacket) {
			assert(decoder.frameQueue.pendingPacket != fpl_null);
			pkt = decoder.frameQueue.pendingPacket;
			decoder.frameQueue.hasPendingPacket = false;
		} else {
			pkt = fpl_null;
			if (PopPacket(decoder.packetsQueue, pkt)) {
				assert(pkt->packet.data != fpl_null);
			} else {
				// We cannot continue to decode, because the packet queue is empty
				return DecodeResult::RequireMorePackets;
			}
		}

		if (pkt != fpl_null) {
			if (ffmpeg.avcodecSendPacket(codecCtx, &pkt->packet) == AVERROR(EAGAIN)) {
				decoder.frameQueue.hasPendingPacket = true;
				decoder.frameQueue.pendingPacket = pkt;
			} else {
				PutPacketBackToReader(reader, pkt);
			}
		}
	}
}

static void VideoDecodingThreadProc(const ThreadContext &thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != fpl_null);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = *decoder->reader;

	MediaStream *stream = decoder->stream;
	assert(stream != fpl_null);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	PlayerState *state = decoder->state;

	ThreadSignal *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Got a free frame to decode in it
		&decoder->freeFrameSignal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	AVStream *videoStream = decoder->stream->stream;

	AVRational currentTimeBase = videoStream->time_base;
	AVRational currentFrameRate = ffmpeg.avGuessFrameRate(state->formatCtx, videoStream, fpl_null);

	bool aquireNewTargetFrame = true;
	Frame *targetFrame = fpl_null;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isDone) {
			continue;
		}

		// Get target frame from the free list if needed
		if (aquireNewTargetFrame) {
			targetFrame = fpl_null;
			if (!Dequeue(decoder->frameQueue.freeListQueue, targetFrame)) {
				continue;
			}
			aquireNewTargetFrame = false;
		}
		assert(targetFrame != fpl_null);
		assert(targetFrame->frame != fpl_null);
		assert(targetFrame->frame->pkt_size <= 0);
		assert(targetFrame->frame->width == 0);

		// Decode video frame
		DecodeResult decodeResult = DecodeFrame(reader, *decoder, targetFrame);
		if (decodeResult != DecodeResult::Success) {
			if (decodeResult != DecodeResult::RequireMorePackets) {
				FreeFrameData(targetFrame);
			}
			if (decodeResult == DecodeResult::EndOfStream) {
				decoder->isDone = 1;
				continue;
			} else if (decodeResult <= DecodeResult::Stopped) {
				break;
			}
		} else {
			double duration = (currentFrameRate.num && currentFrameRate.den ? av_q2d({ currentFrameRate.den, currentFrameRate.num }) : 0);
			double pts = (targetFrame->frame->pts == AV_NOPTS_VALUE) ? NAN : targetFrame->frame->pts * av_q2d(currentTimeBase);
			targetFrame->infos.pts = pts;
			targetFrame->infos.duration = duration;

			// Frame decoded, add it to the decoder
			aquireNewTargetFrame = true;
			AddDecodedFrameToDecoder(*decoder, targetFrame);
			uint32_t decodedAudioFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
#if PRINT_QUEUE_INFOS
			ConsoleFormatOut("Decoded video frame %lu\n", decodedAudioFrameIndex);
#endif

			// Stream finished and no packets left to decode, then are finished as well
			if (reader.isDone && (decoder->packetsQueue.first == fpl_null)) {
				decoder->isDone = 1;
			}
		}
	}
}

static void AudioDecodingThreadProc(const ThreadContext &thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != fpl_null);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = *decoder->reader;

	PlayerState *state = decoder->state;
	assert(state != fpl_null);

	MediaStream *stream = decoder->stream;
	assert(stream != fpl_null);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	ThreadSignal *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Got a free frame to decode in it
		&decoder->freeFrameSignal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	bool autoStarted = false;
	bool aquireNewTargetFrame = true;
	Frame *targetFrame = fpl_null;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isDone) {
			continue;
		}

		// Get target frame from the free list if needed
		if (aquireNewTargetFrame) {
			targetFrame = fpl_null;
			if (!Dequeue(decoder->frameQueue.freeListQueue, targetFrame)) {
				continue;
			}
			aquireNewTargetFrame = false;
		}
		assert(targetFrame != fpl_null);
		assert(targetFrame->frame != fpl_null);
		assert(targetFrame->frame->pkt_size <= 0);
		assert(targetFrame->frame->nb_samples == 0);

		// Decode audio frame
		DecodeResult decodeResult = DecodeFrame(reader, *decoder, targetFrame);
		if (decodeResult != DecodeResult::Success) {
			if (decodeResult != DecodeResult::RequireMorePackets) {
				FreeFrameData(targetFrame);
			}
			if (decodeResult == DecodeResult::EndOfStream) {
				decoder->isDone = 1;
				continue;
			} else if (decodeResult <= DecodeResult::Stopped) {
				break;
			}
		} else {
			// Update audio pts
			targetFrame->infos.pts = (double)targetFrame->frame->pts;

			// Update decoder audio clock
			if (!isnan(targetFrame->infos.pts)) {
				state->audio.audioClock = targetFrame->infos.pts + (double)targetFrame->frame->nb_samples / (double)targetFrame->frame->sample_rate;
			} else {
				state->audio.audioClock = NAN;
			}

			// Frame decoded, add it to the decoder
			aquireNewTargetFrame = true;
			AddDecodedFrameToDecoder(*decoder, targetFrame);
			uint32_t decodedAudioFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
#if PRINT_QUEUE_INFOS
			ConsoleFormatOut("Decoded audio frame %lu\n", decodedAudioFrameIndex);
#endif

			// Stream finished and no packets left to decode, then are finished as well
			if (reader.isDone && (decoder->packetsQueue.first == 0)) {
				decoder->isDone = 1;
			}
		}
	}
}

static uint32_t AudioReadCallback(const AudioDeviceFormat &nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	double audioCallbackTime = (double)ffmpeg.avGetTimeRelative();

	// Intermedite PCM
	// Sample0[Left], Sample0[Right], Sample1[Left], Sample1[Right],...
	// Frame0[Sample Left][Sample Right], Frame1[Sample Left][Sample Right], Frame2[Sample Left][Sample Right],...
	// Samples per Channel = Number of frames
	AudioContext *audio = (AudioContext *)userData;
	assert(audio != fpl_null);

	Decoder &decoder = audio->decoder;

	PlayerState *state = decoder.state;

	uint32_t result = 0;

	if (audio->stream.isValid) {
		uint8_t *conversionAudioBuffer = audio->conversionAudioBuffer;
		uint32_t maxConversionAudioBufferSize = audio->maxConversionAudioBufferSize;

		uint32_t outputSampleStride = nativeFormat.channels * audio::GetAudioSampleSizeInBytes(nativeFormat.type);
		uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

		uint32_t remainingFrameCount = frameCount;
		while (remainingFrameCount > 0) {
			// Consume audio in conversion buffer before we do anything else
			if ((audio->conversionAudioFramesRemaining) > 0) {
				uint32_t maxFramesToRead = audio->conversionAudioFramesRemaining;
				uint32_t framesToRead = FPL_MIN(remainingFrameCount, maxFramesToRead);
				size_t bytesToCopy = framesToRead * outputSampleStride;

				assert(audio->conversionAudioFrameIndex < audio->maxConversionAudioFrameCount);
				size_t sourcePosition = audio->conversionAudioFrameIndex * outputSampleStride;
				assert(sourcePosition < audio->maxConversionAudioBufferSize);

				size_t destPosition = (frameCount - remainingFrameCount) * outputSampleStride;
				assert(destPosition < maxOutputSampleBufferSize);

				memory::MemoryCopy(conversionAudioBuffer + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

				remainingFrameCount -= framesToRead;
				audio->conversionAudioFrameIndex += framesToRead;
				audio->conversionAudioFramesRemaining -= framesToRead;
				result += framesToRead;
			}

			// If we consumed all remaining audio frames, then we are done.
			if (remainingFrameCount == 0) {
				// @NOTE(final): Its highly possible that there are frames left in the conversion buffer, so dont clear anything here!
				break;
			}

			// Convert entire pending frame into conversion buffer
			if (audio->pendingAudioFrame != fpl_null) {
				assert(audio->conversionAudioFramesRemaining == 0);
				Frame *audioFrame = audio->pendingAudioFrame;
				assert(audioFrame->frame != fpl_null);

				audio->pendingAudioFrame = fpl_null;

				uint32_t sourceSampleCount = audioFrame->frame->nb_samples;
				uint32_t sourceChannels = audioFrame->frame->channels;
				uint32_t sourceFrameCount = sourceSampleCount;
				uint8_t **sourceSamples = audioFrame->frame->extended_data;

				// Conversion buffer needs to be big enough to hold the samples for the frame
				uint32_t maxConversionSampleCount = audio->maxConversionAudioFrameCount;
				assert(sourceSampleCount <= maxConversionSampleCount);

				int samplesPerChannel = ffmpeg.swrConvert(audio->softwareResampleCtx, (uint8_t **)&audio->conversionAudioBuffer, maxConversionSampleCount, (const uint8_t **)sourceSamples, sourceSampleCount);

				PutFrameBackToDecoder(decoder, audioFrame);

				if (samplesPerChannel <= 0) {
					break;
				}

				audio->conversionAudioFramesRemaining = samplesPerChannel;
				audio->conversionAudioFrameIndex = 0;
			}

			if ((audio->pendingAudioFrame == fpl_null) && (audio->conversionAudioFramesRemaining == 0)) {
				Frame *newAudioFrame;
				if (Dequeue(decoder.frameQueue.availableFramesQueue, newAudioFrame)) {
					audio->pendingAudioFrame = newAudioFrame;
					audio->conversionAudioFrameIndex = 0;
					audio->conversionAudioFramesRemaining = 0;
					continue;
				} else {
					// No audio frame available (Decoder is too slow).
					break;
				}
			}
		}

		// Update audio clock
		if (!isnan(audio->audioClock)) {
			uint32_t writtenSize = result * outputSampleStride;
			uint32_t bytesPerSample = nativeFormat.sampleRate * outputSampleStride;
			double diff = (double)(nativeFormat.periods * nativeFormat.bufferSizeInBytes + writtenSize) / (double)bytesPerSample;
			SetClockAt(audio->clock, audio->audioClock - diff, audioCallbackTime / (double)AV_TIME_BASE);
			SyncClockToSlave(state->externalClock, audio->clock);
		}

	}

	return(result);
}

static void PacketReadThreadProc(const ThreadContext &thread, void *userData) {
	PlayerState *state = (PlayerState *)userData;
	assert(state != fpl_null);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = state->reader;
	VideoContext &video = state->video;
	AudioContext &audio = state->audio;
	MediaStream *videoStream = video.decoder.stream;
	MediaStream *audioStream = audio.decoder.stream;
	AVFormatContext *formatCtx = state->formatCtx;
	assert(formatCtx != fpl_null);

	ThreadSignal *waitSignals[] = {
		// We got a free packet for use to read into
		&reader.packetQueue.freeSignal,
		// Reader should terminate
		&reader.stopSignal,
		// Reader can continue
		&reader.resumeSignal,
	};

	bool skipWait = true;
	for (;;) {
		// Wait for any signal or skip wait
		if (!skipWait) {
			SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));
		} else {
			skipWait = false;
		}

		// Stop reader
		if (reader.stopRequest) {
			break;
		}

		// Sleep 10 ms then wait until the reader wakes up in the next iteration when we are finished
		if (reader.isDone) {
			ThreadSleep(10);
			skipWait = false;
			continue;
		}

		// Limit the queue?
		if ((!state->isInfiniteBuffer &&
			(audio.decoder.packetsQueue.size + video.decoder.packetsQueue.size) > MAX_PACKET_QUEUE_SIZE) ||
			(StreamHasEnoughPackets(audio.stream.stream, audio.stream.streamIndex, audio.decoder.packetsQueue) &&
			StreamHasEnoughPackets(video.stream.stream, video.stream.streamIndex, video.decoder.packetsQueue))) {
			skipWait = true;
			ThreadSleep(10);
			continue;
		}

		// Try to get new packet from the freelist
		PacketList *packet = fpl_null;
		if (AquirePacket(reader.packetQueue, packet)) {
			assert(packet != fpl_null);

			// Read packet
			int res = ffmpeg.avReadFrame(formatCtx, &packet->packet);
			AtomicAddS32(&globalMemStats.referencedPackets, 1);
			if (res < 0) {
				// Error or stream done
				PutPacketBackToReader(reader, packet);
				reader.isDone = 1;
				continue;
			}

			uint32_t packetIndex = AtomicAddU32(&reader.readPacketCount, 1);
#if PRINT_QUEUE_INFOS
			ConsoleFormatOut("Read packet %lu\n", packetIndex);
#endif

			// Check if packet is in play range, then queue, otherwise discard
			int64_t streamStartTime = formatCtx->streams[packet->packet.stream_index]->start_time;
			int64_t pktTimeStamp = (packet->packet.pts == AV_NOPTS_VALUE) ? packet->packet.dts : packet->packet.pts;
			double timeInSeconds = (double)(pktTimeStamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0)) * av_q2d(formatCtx->streams[packet->packet.stream_index]->time_base);
			bool pktInPlayRange = (!state->settings.duration.isValid) ||
				((timeInSeconds / (double)AV_TIME_BASE) <= ((double)state->settings.duration.value / (double)AV_TIME_BASE));
			if ((videoStream != fpl_null) && (packet->packet.stream_index == videoStream->streamIndex) && pktInPlayRange) {
				AddPacketToDecoder(video.decoder, packet);
#if PRINT_QUEUE_INFOS
				ConsoleFormatOut("Queued video packet %lu\n", packetIndex);
#endif
			} else if ((audioStream != fpl_null) && (packet->packet.stream_index == audioStream->streamIndex) && pktInPlayRange) {
				AddPacketToDecoder(audio.decoder, packet);
#if PRINT_QUEUE_INFOS
				ConsoleFormatOut("Queued audio packet %lu\n", packetIndex);
#endif
			} else {
#if PRINT_QUEUE_INFOS
				ConsoleFormatOut("Dropped packet %lu\n", packetIndex);
#endif
				PutPacketBackToReader(reader, packet);
			}
			skipWait = true;
		} else {
			// Error: Packet cannot be allocated anymore!
		}
	}

	ConsoleOut("Reader thread stopped.\n");
}

static bool OpenStreamComponent(const char *mediaFilePath, AVStream *stream, MediaStream &outStream) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

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
	outStream.codecContext = ffmpeg.avcodecAllocContext3(fpl_null);
	if (ffmpeg.avcodecParametersToContext(outStream.codecContext, stream->codecpar) < 0) {
		ConsoleFormatError("Failed getting %s codec context from codec '%s' in media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @NOTE(finaL): Set packet time base to stream time base
	outStream.codecContext->pkt_timebase = stream->time_base;

	// Find decoder
	// @TODO(final): We could force the codec here if we want (avcodec_find_decoder_by_name).
	outStream.codec = ffmpeg.avcodecFindDecoder(stream->codecpar->codec_id);
	if (outStream.codec == fpl_null) {
		ConsoleFormatError("Unsupported %s codec '%s' in media file '%s' found!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// Open codec
	if (ffmpeg.avcodecOpen2(outStream.codecContext, outStream.codec, fpl_null) < 0) {
		ConsoleFormatError("Failed opening %s codec '%s' from media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @TODO(final): Why do we need to set the discard flag to default here?
	stream->discard = AVDISCARD_DEFAULT;

	outStream.isValid = true;
	outStream.stream = stream;

	return true;
}

static bool IsRealTime(AVFormatContext *s) {
	if (!strcmp(s->iformat->name, "rtp") ||
		!strcmp(s->iformat->name, "rtsp") ||
		!strcmp(s->iformat->name, "sdp")) {
		return true;
	}
	if (s->pb && (!strncmp(s->filename, "rtp:", 4) || !strncmp(s->filename, "udp:", 4))) {
		return true;
	}
	return false;
}

struct RefreshState {
	double remainingTime;
};

static void VideoDisplay(PlayerState *state, Frame *sourceNativeFrame) {
	assert(state != fpl_null);
	assert(sourceNativeFrame != fpl_null);
	VideoContext &video = state->video;
	AVCodecContext *videoCodecCtx = video.stream.codecContext;
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.swsScale(video.softwareScaleCtx, (uint8_t const * const *)sourceNativeFrame->frame->data, sourceNativeFrame->frame->linesize, 0, videoCodecCtx->height, video.targetRGBFrame->data, video.targetRGBFrame->linesize);
	ConvertRGB24ToBackBuffer(video.backBuffer, videoCodecCtx->width, videoCodecCtx->height, *video.targetRGBFrame->linesize, video.targetRGBBuffer);
	WindowFlip();
}

inline void UpdateVideoClock(PlayerState *state, double pts) {
	SetClock(state->video.clock, pts);
	SyncClockToSlave(state->externalClock, state->video.clock);
}

inline double GetFrameDuration(PlayerState *state, const FrameInfo &cur, const FrameInfo &next) {
	double duration = next.pts - cur.pts;
	if (isnan(duration) || duration <= 0 || duration > state->maxFrameDuration)
		return cur.duration;
	else
		return duration;
}

static double ComputeVideoDelay(PlayerState *state, double delay) {
	if (GetMasterSyncType(state) != AVSyncType::VideoMaster) {
		double diff = GetClock(state->video.clock) - GetMasterClock(state);
		double syncThreshold = FPL_MAX(AV_SYNC_THRESHOLD_MIN, FPL_MIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < state->maxFrameDuration) {
			if (diff <= -syncThreshold) {
				delay = FFMAX(0, delay + diff);
			} else if (diff >= syncThreshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
				delay = delay + diff;
			} else if (diff >= syncThreshold) {
				delay = 2 * delay;
			}
		}
	}
	return(delay);
}

static void VideoRefresh(PlayerState *state, double *remainingTime) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	if (!state->isPaused && GetMasterSyncType(state) == AVSyncType::ExternalClock && state->isRealTime) {
		UpdateExternalClockSpeed(state);
	}
	if (state->video.stream.isValid) {
	retry:
		Frame *frame = state->video.waitingFrame;
		if (state->video.waitingFrame == fpl_null) {
			frame = fpl_null;
			if (Dequeue(state->video.decoder.frameQueue.availableFramesQueue, frame)) {
				state->video.waitingFrame = frame;
			}
		}
		if (frame != fpl_null) {
			double delay = frame->infos.pts - state->frameLastPTS;
			if (delay <= 0 || delay >= 1) {
				delay = state->frameLastDelay;
			}
			state->frameLastPTS = frame->infos.pts;
			state->frameLastDelay = delay;

			delay = ComputeVideoDelay(state, delay);

			double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
			if (time < state->frameTimer + delay) {
				*remainingTime = FPL_MIN(state->frameTimer + delay - time, *remainingTime);
				return;
			}

			state->frameTimer += delay;
			if (delay > 0 && time - state->frameTimer > AV_SYNC_THRESHOLD_MAX) {
				state->frameTimer = time;
			}

			UpdateVideoClock(state, frame->infos.pts);

			if (time > state->frameTimer + frame->infos.duration) {
				PutFrameBackToDecoder(state->video.decoder, frame);
				frame = state->video.waitingFrame = fpl_null;
				goto retry;
			}

			VideoDisplay(state, frame);
			state->video.prevFrameInfo = frame->infos;
			PutFrameBackToDecoder(state->video.decoder, frame);
			state->video.waitingFrame = fpl_null;
		}
	}
}

static void ReleaseMedia(PlayerState &state) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	DestroyDecoder(state.audio.decoder);
	if (state.audio.conversionAudioBuffer != fpl_null) {
		memory::MemoryAlignedFree(state.audio.conversionAudioBuffer);
	}
	if (state.audio.softwareResampleCtx != fpl_null) {
		ffmpeg.swrFree(&state.audio.softwareResampleCtx);
	}
	if (state.audio.stream.codecContext != fpl_null) {
		ffmpeg.avcodecFreeContext(&state.audio.stream.codecContext);
	}

	DestroyDecoder(state.video.decoder);
	if (state.video.softwareScaleCtx != fpl_null) {
		ffmpeg.swsFreeContext(state.video.softwareScaleCtx);
	}
	if (state.video.targetRGBBuffer != fpl_null) {
		MemoryAlignedFree(state.video.targetRGBBuffer);
	}
	if (state.video.targetRGBFrame != fpl_null) {
		ffmpeg.avFrameFree(&state.video.targetRGBFrame);
	}
	if (state.video.stream.codecContext != fpl_null) {
		ffmpeg.avcodecFreeContext(&state.video.stream.codecContext);
	}

	DestroyReader(state.reader);
	if (state.formatCtx != fpl_null) {
		ffmpeg.avformatCloseInput(&state.formatCtx);
	}
}

static bool LoadMedia(PlayerState &state, const char *mediaFilePath, const AudioDeviceFormat &nativeAudioFormat, VideoBackBuffer *backBuffer) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	// @TODO(final): Custom IO!

	// Open media file
	if (ffmpeg.avformatOpenInput(&state.formatCtx, mediaFilePath, fpl_null, fpl_null) != 0) {
		ConsoleFormatError("Failed opening media file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Retrieve stream information
	if (ffmpeg.avformatFindStreamInfo(state.formatCtx, fpl_null) < 0) {
		ConsoleFormatError("Failed getting stream informations for media file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Dump information about file onto standard error
	ffmpeg.avDumpFormat(state.formatCtx, 0, mediaFilePath, 0);

	// Dont limit the queues when we are playing realtime based media, like internet streams, etc.
	state.isRealTime = IsRealTime(state.formatCtx);
	if (!state.isInfiniteBuffer && state.isRealTime) {
		state.isInfiniteBuffer = true;
	}

	// Find the first streams
	state.video.stream.streamIndex = -1;
	state.audio.stream.streamIndex = -1;
	for (uint32_t steamIndex = 0; steamIndex < state.formatCtx->nb_streams; steamIndex++) {
		AVStream *stream = state.formatCtx->streams[steamIndex];
		switch (stream->codecpar->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
			{
				if (state.video.stream.streamIndex == -1) {
					if (OpenStreamComponent(mediaFilePath, stream, state.video.stream)) {
						state.video.stream.streamIndex = steamIndex;
					}
				}
			} break;
			case AVMEDIA_TYPE_AUDIO:
			{
				if (state.audio.stream.streamIndex == -1) {
					if (OpenStreamComponent(mediaFilePath, stream, state.audio.stream)) {
						state.audio.stream.streamIndex = steamIndex;
					}
				}
			} break;
		}
	}

	// No streams found
	if ((!state.video.stream.isValid) && (!state.audio.stream.isValid)) {
		ConsoleFormatError("No video or audio stream in media file '%s' found!\n", mediaFilePath);
		goto release;
	}

	if (!InitReader(state.reader)) {
		ConsoleFormatError("Failed initializing reader file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Allocate audio related resources
	if (state.audio.stream.isValid) {
		AudioContext &audio = state.audio;
		AVCodecContext *audioCodexCtx = audio.stream.codecContext;

		// Init audio decoder
		if (!InitDecoder(audio.decoder, &state, &state.reader, &audio.stream, MAX_AUDIO_FRAME_QUEUE_COUNT)) {
			ConsoleFormatError("Failed initialize audio decoder for media file '%s'!\n", mediaFilePath);
			goto release;
		}

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
		audio.softwareResampleCtx = ffmpeg.swrAllocSetOpts(fpl_null,
														   targetChannelLayout,
														   targetSampleFormat,
														   targetSampleRate,
														   inputChannelLayout,
														   inputSampleFormat,
														   inputSampleRate,
														   0,
														   fpl_null);
		ffmpeg.swrInit(audio.softwareResampleCtx);

		// Allocate conversion buffer in native format, this must be big enough to hold one AVFrame worth of data.
		int lineSize;
		audio.maxConversionAudioBufferSize = ffmpeg.avSamplesGetBufferSize(&lineSize, targetChannelCount, targetSampleRate, targetSampleFormat, 1);
		audio.maxConversionAudioFrameCount = audio.maxConversionAudioBufferSize / audio::GetAudioSampleSizeInBytes(nativeAudioFormat.type) / targetChannelCount;
		audio.conversionAudioBuffer = (uint8_t *)memory::MemoryAlignedAllocate(audio.maxConversionAudioBufferSize, 16);
		audio.conversionAudioFrameIndex = 0;
		audio.conversionAudioFramesRemaining = 0;
	}

	// Allocate video related resources
	if (state.video.stream.isValid) {
		VideoContext &video = state.video;
		AVCodecContext *videoCodexCtx = video.stream.codecContext;

		// Init video decoder
		if (!InitDecoder(video.decoder, &state, &state.reader, &video.stream, MAX_VIDEO_FRAME_QUEUE_COUNT)) {
			ConsoleFormatError("Failed initialize video decoder for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB video frame
		video.targetRGBFrame = ffmpeg.avFrameAlloc();
		if (video.targetRGBFrame == fpl_null) {
			ConsoleFormatError("Failed allocating RGB video frame for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB buffer
		AVPixelFormat targetPixelFormat = AVPixelFormat::AV_PIX_FMT_BGR24;
		size_t rgbFrameSize = ffmpeg.avImageGetBufferSize(targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);
		video.targetRGBBuffer = (uint8_t *)MemoryAlignedAllocate(rgbFrameSize, 16);

		// Setup RGB video frame and give it access to the actual data
		ffmpeg.avImageFillArrays(video.targetRGBFrame->data, video.targetRGBFrame->linesize, video.targetRGBBuffer, targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);

		// Get software context
		video.softwareScaleCtx = ffmpeg.swsGetContext(
			videoCodexCtx->width,
			videoCodexCtx->height,
			videoCodexCtx->pix_fmt,
			videoCodexCtx->width,
			videoCodexCtx->height,
			targetPixelFormat,
			SWS_BILINEAR,
			fpl_null,
			fpl_null,
			fpl_null
		);
		if (video.softwareScaleCtx == fpl_null) {
			ConsoleFormatError("Failed getting software scale context with size (%d x %d) for file '%s'!\n", videoCodexCtx->width, videoCodexCtx->height, mediaFilePath);
			goto release;
		}

		// Resize backbuffer to fit the video size
		if (!ResizeVideoBackBuffer(videoCodexCtx->width, videoCodexCtx->height)) {
			ConsoleFormatError("Failed resizing video backbuffer to size (%d x %d) for file '%s'!\n", videoCodexCtx->width, videoCodexCtx->height, mediaFilePath);
			goto release;
		}

		state.video.backBuffer = backBuffer;
		state.frameTimer = 0.0;
		state.frameLastPTS = 0.0;
		state.frameLastDelay = 40e-3;
	}

	// Init timings
	state.maxFrameDuration = (state.formatCtx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	InitClock(state.video.clock);
	InitClock(state.audio.clock);
	InitClock(state.externalClock);

	return true;

release:
	ReleaseMedia(state);
	return false;
}

int main(int argc, char **argv) {
	int result = 0;

	if (argc < 2) {
		ConsoleError("Media file argument missing!");
		return -1;
	}

	const char *mediaFilePath = argv[1];

	Settings settings = DefaultSettings();
	CopyAnsiString("FPL FFmpeg Demo", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.driverType = VideoDriverType::Software;
	settings.video.isAutoSize = false;
	settings.video.isVSync = true;

	if (InitPlatform(InitFlags::All, settings)) {
		VideoBackBuffer *backBuffer = GetVideoBackBuffer();
		AudioDeviceFormat nativeAudioFormat = GetAudioHardwareFormat();

		globalFFMPEGFunctions = (FFMPEGContext *)MemoryAlignedAllocate(sizeof(FFMPEGContext), 16);
		FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

		PlayerState state = {};
		RefreshState refresh = {};
		uint32_t threadCount = 0;
		ThreadContext *threads[3] = {};

		//
		// Load ffmpeg libraries
		//
		if (!LoadFFMPEG(ffmpeg)) {
			goto release;
		}

		// Register all formats and codecs
		ffmpeg.avRegisterAll();

		//
		// Settings
		//
		InitPlayerSettings(state.settings);
		state.isInfiniteBuffer = state.settings.useInfiniteBuffer;

		// Load media
		if (!LoadMedia(state, mediaFilePath, nativeAudioFormat, backBuffer)) {
			goto release;
		}

		// Create threads
		threads[threadCount++] = ThreadCreate(PacketReadThreadProc, &state);
		if (state.video.stream.isValid) {
			threads[threadCount++] = ThreadCreate(VideoDecodingThreadProc, &state.video.decoder);
		}
		if (state.audio.stream.isValid) {
			threads[threadCount++] = ThreadCreate(AudioDecodingThreadProc, &state.audio.decoder);
		}

		// Start playing audio
		if (state.audio.stream.isValid) {
			SetAudioClientReadCallback(AudioReadCallback, &state.audio);
			PlayAudio();
		}

		//
		// App loop
		//
		double lastTime = GetHighResolutionTimeInSeconds();
		double remainingTime = 0.0;
		while (WindowUpdate()) {
			//
			// Handle events
			//
			Event ev = {};
			while (PollWindowEvent(ev)) {
			}

			// Refresh video?
			if (remainingTime <= 0.0) {
				remainingTime = DEFAULT_REFRESH_RATE;
				VideoRefresh(&state, &remainingTime);
			}

			// Update time
			double now = GetHighResolutionTimeInSeconds();
			double delta = now - lastTime;
			lastTime = now;
			remainingTime -= delta;
			PrintMemStats();
		}


	release:
		// Stop audio
		if (state.audio.stream.isValid) {
			StopAudio();
		}

		// Stop reader and decoders
		StopReader(state.reader);
		if (state.video.stream.isValid) {
			StopDecoder(state.video.decoder);
		}
		if (state.audio.stream.isValid) {
			StopDecoder(state.audio.decoder);
		}

		// Wait until all threads are finished running and release all threads
		ThreadWaitForAll(threads, threadCount);
		for (uint32_t threadIndex = threadCount - 1; threadIndex > 0; threadIndex--) {
			ThreadDestroy(threads[threadIndex]);
		}

		// Release media
		ReleaseMedia(state);

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