// This example requires a custom ffmpeg win64 build from https://ffmpeg.zeranoe.com/builds/
// Additional resources:
// http://dranger.com/ffmpeg/tutorial01.html
// https://blogs.gentoo.org/lu_zero/2015/10/15/deprecating-avpicture/
// https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
// https://www.codeproject.com/tips/489450/creating-custom-ffmpeg-io-context

#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

#include <assert.h> // assert

struct BitmapInfoheader {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

#pragma pack(push,2)
struct BitmapFileHeader {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};
#pragma pack(pop)

constexpr uint32_t BITMAP_FORMAT_RGB = 0L;

//
// FFMPEG headers and function prototypes
//
extern "C" {
#	include <libavcodec/avcodec.h>
#	include <libavformat/avformat.h>
#	include <libavutil/avutil.h>
#	include <libavutil/imgutils.h>
#	include <libswscale\swscale.h>
}

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
#define FFMPEG_AV_PACKET_ALLOC(name) AVPacket *name(void)
typedef FFMPEG_AV_PACKET_ALLOC(ffmpeg_av_packet_alloc_func);
// av_packet_free
#define FFMPEG_AV_PACKET_FREE(name) void name(AVPacket **pkt)
typedef FFMPEG_AV_PACKET_FREE(ffmpeg_av_packet_free_func);

// av_frame_alloc
#define FFMPEG_AV_FRAME_ALLOC_FUNC(name) AVFrame *name(void)
typedef FFMPEG_AV_FRAME_ALLOC_FUNC(ffmpeg_av_frame_alloc_func);
// av_frame_free
#define FFMPEG_AV_FRAME_FREE_FUNC(name) void name(AVFrame **frame)
typedef FFMPEG_AV_FRAME_FREE_FUNC(ffmpeg_av_frame_free_func);
// av_image_get_buffer_size
#define FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(ffmpeg_av_image_get_buffer_size_func);
// av_image_get_linesize
#define FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int plane)
typedef FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(ffmpeg_av_image_get_linesize_func);
// av_image_fill_arrays
#define FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(name) int name(uint8_t *dst_data[4], int dst_linesize[4], const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(ffmpeg_av_image_fill_arrays_func);

// sws_getContext
#define FFMPEG_SWS_GET_CONTEXT_FUNC(name) struct SwsContext *name(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CONTEXT_FUNC(ffmpeg_sws_getContext_func);
// sws_scale
#define FFMPEG_SWS_SCALE_FUNC(name) int name(struct SwsContext *c, const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[])
typedef FFMPEG_SWS_SCALE_FUNC(ffmpeg_sws_scale_func);
// sws_freeContext
#define FFMPEG_SWS_FREE_CONTEXT_FUNC(name) void name(struct SwsContext *swsContext)
typedef FFMPEG_SWS_FREE_CONTEXT_FUNC(ffmpeg_sws_freeContext_func);

#define FFMPEG_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)GetDynamicLibraryProc(libHandle, name); \
	if (target == nullptr) { \
		ConsoleFormatError("[FFMPEG] Failed getting '%s' from library '%s'!", name, libName); \
		goto release; \
	}

struct ffmpegFunctions {
	// Format
	ffmpeg_av_register_all_func *avRegisterAll;
	ffmpeg_avformat_close_input_func *avFormatCloseInput;
	ffmpeg_avformat_open_input_func *avFormatOpenInput;
	ffmpeg_avformat_find_stream_info_func *avFormatFindStreamInfo;
	ffmpeg_av_dump_format_func *avDumpFormat;
	ffmpeg_av_read_frame_func *avReadFrame;

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

	// Util
	ffmpeg_av_frame_alloc_func *avFrameAlloc;
	ffmpeg_av_frame_free_func *avFrameFree;
	ffmpeg_av_image_get_buffer_size_func *avImageGetBufferSize;
	ffmpeg_av_image_get_linesize_func *avImageGetLineSize;
	ffmpeg_av_image_fill_arrays_func *avImageFillArrays;

	// SWS
	ffmpeg_sws_getContext_func *swsGetContext;
	ffmpeg_sws_scale_func *swsScale;
	ffmpeg_sws_freeContext_func *swsFreeContext;
};

static ffmpegFunctions *globalFFMPEGFunctions = nullptr;

static int DecodeVideoPacket(struct AVCodecContext *avctx, struct AVFrame *frame, struct AVPacket *pkt, bool *got_frame) {
	ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;

	int ret;

	*got_frame = false;

	if (pkt) {
		// @NOTE(final): Bad naming convention in ffmpeg "send" is more like "push"
		ret = ffmpeg.avcodecSendPacket(avctx, pkt);
		if (ret < 0) {
			return ret == AVERROR_EOF ? 0 : ret;
		}
	}

	// @NOTE(final): We "pushed" all required packets and are able to decode the actual frame
	ret = ffmpeg.avcodecReceiveFrame(avctx, frame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
		return ret;
	}

	if (ret >= 0) {
		*got_frame = true;
	}

	return 0;
}

static void SaveBitmapRGB24(uint8_t *source, uint32_t width, uint32_t height, uint32_t scanline, const char *targetFilePath) {
	assert(scanline == (width * 3));

	av_packet_alloc();

	BitmapInfoheader bih = {};
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biCompression = BITMAP_FORMAT_RGB;
	bih.biHeight = -(int32_t)height;
	bih.biWidth = width;
	bih.biPlanes = 1;
	bih.biSizeImage = scanline * height;
	bih.biSize = sizeof(BitmapInfoheader);

	BitmapFileHeader bfh = {};
	bfh.bfType = ((uint16_t)('M' << 8) | 'B');
	bfh.bfSize = (uint32_t)(sizeof(BitmapFileHeader) + bih.biSize + bih.biSizeImage);
	bfh.bfOffBits = (uint32_t)(sizeof(BitmapFileHeader) + bih.biSize);

	FileHandle handle = CreateBinaryFile(targetFilePath);
	if (handle.isValid) {
		WriteFileBlock32(handle, &bfh, sizeof(BitmapFileHeader));
		WriteFileBlock32(handle, &bih, sizeof(BitmapInfoheader));
		WriteFileBlock32(handle, source, bih.biSizeImage);
		CloseFile(handle);
	}
}

// http://nullprogram.com/blog/2014/09/02/

template <typename T>
struct ListItem {
	ListItem<T> *next;
	ListItem<T> *prev;
	T value;
};

template <typename T>
struct List {
	ThreadMutex lock;
	ThreadSignal availableSignal;
	ListItem<T> *first;
	ListItem<T> *last;
};

template <typename T>
inline void	InitList(List<T> &list, ListItem<T> *first = nullptr, ListItem<T> *last = nullptr) {
	list.lock = MutexCreate();
	list.availableSignal = SignalCreate();
	list.first = first;
	list.last = last;
}

template <typename T>
inline void DestroyList(List<T> &list) {
	SignalDestroy(list.availableSignal);
	MutexDestroy(list.lock);
	list = {};
}

template <typename T>
static bool TryGetFromList(List<T> &list, ListItem<T> **outValue) {
	bool result = false;
	if (list.first != nullptr) {
		MutexLock(list.lock);
		{
			// We may was in waiting for lock releasing state, so the first could be null.
			if (list.first != nullptr) {
				// Get entry from list
				ListItem<T> *item = list.first;
				ListItem<T> *next = item->next;
				list.first = next;
				if (list.first == nullptr) {
					list.last = nullptr;
				}
				item->next = item->prev = nullptr;
				*outValue = item;
				result = true;
			}
		}
		MutexUnlock(list.lock);
	}
	return(result);
}

template <typename T>
static void AddToList(List<T> &list, ListItem<T> *item) {
	assert(item != nullptr);
	item->next = item->prev = nullptr;
	MutexLock(list.lock);
	{
		// Add entry to the list
		if (list.last == nullptr) {
			list.first = list.last = item;
		} else {
			list.last->next = item;
			item->prev = list.last;
			list.last = item;
		}
	}
	MutexUnlock(list.lock);
}

template <typename T>
struct AvailableFreeQueue {
	typedef T(CreateListItemValueFunction)();
	typedef void(FreeListItemValueFunction)(T *value);

	ListItem<T> *items;
	FreeListItemValueFunction *freeCallback;
	List<T> freeList;
	List<T> availableList;
	ThreadSignal stoppedSignal;
	volatile uint32_t isStopped;
	uint32_t capacity;

	static AvailableFreeQueue<T> Create(uint32_t capacity, CreateListItemValueFunction *createCallback, FreeListItemValueFunction freeCallback) {
		AvailableFreeQueue<T> result = {};
		result.capacity = capacity;
		result.items = (ListItem<T> *)memory::MemoryAllocate(sizeof(ListItem<T>) * capacity);
		result.stoppedSignal = SignalCreate();
		result.freeCallback = freeCallback;
		InitList<T>(result.freeList, result.items, result.items + (capacity - 1));
		InitList<T>(result.availableList);
		for (uint32_t i = 0; i < result.capacity; ++i) {
			ListItem<T> *item = result.items + i;
			item->value = createCallback();
			if (i < result.capacity - 1) {
				item->next = result.items + (i + 1);
			} else {
				item->next = nullptr;
			}
			if (i > 0) {
				item->prev = result.items + (i - 1);
			} else {
				item->prev = nullptr;
			}
		}
		return(result);
	}

	static void Destroy(AvailableFreeQueue<T> &result) {
		for (uint32_t i = result.capacity - 1; i > 0; i--) {
			ListItem<T> *item = result.items + i;
			result.freeCallback(&item->value);
		}
		DestroyList<T>(result.availableList);
		DestroyList<T>(result.freeList);
		SignalDestroy(result.stoppedSignal);
		memory::MemoryFree(result.items);
		result = {};
	}
};

//
// Packet Queue
//

constexpr uint32_t MAX_PACKET_QUEUE_COUNT = 16;

typedef AvailableFreeQueue<AVPacket *> PacketQueue;
typedef ListItem<AVPacket *> PacketItem;

inline AVPacket *AllocatePacket() {
	ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;
	AVPacket *result = ffmpeg.avPacketAlloc();
	return(result);
}

inline void FreePacket(AVPacket **packet) {
	ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;
	AVPacket *p = *packet;
	if (p->data != nullptr) {
		ffmpeg.avPacketUnref(p);
	}
	ffmpeg.avPacketFree(packet);
}

static PacketQueue CreatePacketQueue(uint32_t capacity) {
	PacketQueue result = AvailableFreeQueue<AVPacket *>::Create(capacity, AllocatePacket, FreePacket);
	return(result);
}

static void DestroyPacketQueue(PacketQueue &queue) {
	AvailableFreeQueue<AVPacket *>::Destroy(queue);
}

//
// Frame Queue
//

constexpr uint32_t MAX_FRAME_QUEUE_COUNT = 8;

typedef AvailableFreeQueue<AVFrame *> FrameQueue;
typedef ListItem<AVFrame *> FrameItem;

inline AVFrame *AllocateFrame() {
	ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *result = ffmpeg.avFrameAlloc();
	return(result);
}

inline void FreeFrame(AVFrame **frame) {
	ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *p = *frame;
	ffmpeg.avFrameFree(frame);
}

static FrameQueue CreateFrameQueue(uint32_t capacity) {
	FrameQueue result = AvailableFreeQueue<AVFrame *>::Create(capacity, AllocateFrame, FreeFrame);
	return(result);
}

static void DestroyFrameQueue(FrameQueue &queue) {
	AvailableFreeQueue<AVFrame *>::Destroy(queue);
}

struct FFMPEGState {
	PacketQueue packetQueue;
	FrameQueue frameQueue;
	ffmpegFunctions *functions;
	AVFormatContext *formatCtx;
	AVCodecContext *videoCtx;
	AVCodec *videoCodec;
	int32_t videoStreamIndex;
	volatile uint32_t readPackets;
	volatile uint32_t decodedVideoFrames;
	AVFrame *sourceNativeFrame;
	AVFrame *targetRGBFrame;
	uint8_t *targetRGBBuffer;
	SwsContext *softwareCtx;
};

static void ConvertRGB24ToBackBuffer(VideoBackBuffer *backbuffer, int width, int height, int sourceScanLine, uint8_t *sourceData) {
	for (int y = 0; y < height; ++y) {
		uint8_t *src = sourceData + y * sourceScanLine;
		int invertY = height - 1 - y;
		uint32_t *dst = (uint32_t *)((uint8_t *)backbuffer->pixels + invertY * backbuffer->stride);
		for (int x = 0; x < width; ++x) {
			uint8_t r = *src++;
			uint8_t g = *src++;
			uint8_t b = *src++;
			uint8_t a = 255;
			*dst++ = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}
}

static void VideoDecodingThreadProc(const ThreadContext &thread, void *userData) {
	FFMPEGState *state = (FFMPEGState *)userData;
	ffmpegFunctions &ffmpeg = *state->functions;

	ThreadSignal *waitSignals[] = {
		&state->packetQueue.availableList.availableSignal,
		&state->frameQueue.freeList.availableSignal,
		&state->frameQueue.stoppedSignal,
	};

	bool frameWasDone = true;
	FrameItem *frameItem = nullptr;
	for (;;) {
		// Wait for either a available signal from the list or an stopped signal from the queue
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		if (state->frameQueue.isStopped) {
			// Queue is stopped, exit
			break;
		}

		// Get native frame if needed

		if (frameWasDone) {
			frameItem = nullptr;
			if (!TryGetFromList(state->frameQueue.freeList, &frameItem)) {
				continue;
			}
			frameWasDone = false;
		}
		assert(frameItem != nullptr);
		AVFrame *sourceNativeFrame = frameItem->value;

		// Try to get new packet
		PacketItem *packetItem;
		if (TryGetFromList(state->packetQueue.availableList, &packetItem)) {
			AVPacket *packet = packetItem->value;
			if (packet->stream_index == state->videoStreamIndex) {
				// Decode video packet
				bool frameDecoded = false;
				DecodeVideoPacket(state->videoCtx, sourceNativeFrame, packet, &frameDecoded);

				// Put packet back to the freelist (Processed)
				ffmpeg.avPacketUnref(packet);
				AddToList(state->packetQueue.freeList, packetItem);
				SignalWakeUp(state->packetQueue.freeList.availableSignal);

				if (frameDecoded) {
					frameWasDone = true;
					uint32_t decodedVideoFrameIndex = AtomicAddU32(&state->decodedVideoFrames, 1);
					ConsoleFormatOut("Decoded video frame %lu\n", decodedVideoFrameIndex);
					AddToList(state->frameQueue.availableList, frameItem);
					SignalWakeUp(state->frameQueue.availableList.availableSignal);
				}
			} else {
				// Put packet back to the freelist (Dropped)
				ffmpeg.avPacketUnref(packet);
				AddToList(state->packetQueue.freeList, packetItem);
				SignalWakeUp(state->packetQueue.freeList.availableSignal);
			}
		}
	}
}

static void PacketReadThreadProc(const ThreadContext &thread, void *userData) {
	FFMPEGState *state = (FFMPEGState *)userData;
	ffmpegFunctions &ffmpeg = *state->functions;

	bool skipFirstWait = state->packetQueue.freeList.first != nullptr;

	ThreadSignal *waitSignals[2] = {
		&state->packetQueue.freeList.availableSignal,
		&state->packetQueue.stoppedSignal,
	};

	for (;;) {
		if (skipFirstWait) {
			skipFirstWait = false;
		} else {
			// Wait for either a available signal from the list or an stopped signal from the queue
			SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));
		}

		if (state->packetQueue.isStopped) {
			// Queue is stopped, exit
			break;
		}

		PacketItem *packetItem;
		if (TryGetFromList(state->packetQueue.freeList, &packetItem)) {
			uint32_t packetIndex = AtomicAddU32(&state->readPackets, 1);
			assert(packetItem != nullptr);
			ConsoleFormatOut("Read packet frame %lu\n", packetIndex);
			int res = ffmpeg.avReadFrame(state->formatCtx, packetItem->value);
			if (res >= 0) {
				ConsoleFormatOut("Added packet %lu\n", packetIndex);
				AddToList(state->packetQueue.availableList, packetItem);
				SignalWakeUp(state->packetQueue.availableList.availableSignal);
			} else {
				// Error or stream is done, exit
				break;
			}
		} else {
			// Queue is full, we wait on the start of the next iteration for next available signal
		}
	}
}

int main(int argc, char **argv) {
	int result = 0;

	if (argc < 2) {
		ConsoleError("Video file argument missing!");
		return -1;
	}

	const char *mediaFilePath = argv[1];

	Settings settings = DefaultSettings();
	settings.video.driverType = VideoDriverType::Software;
	settings.video.isAutoSize = false;
	if (InitPlatform(InitFlags::All, settings)) {
		globalFFMPEGFunctions = (ffmpegFunctions *)MemoryAlignedAllocate(sizeof(ffmpegFunctions), 16);

		ffmpegFunctions &ffmpeg = *globalFFMPEGFunctions;

		FFMPEGState state = {};
		state.functions = &ffmpeg;

		//
		// Load ffmpeg libraries
		//
		const char *avFormatLibFile = "avformat-58.dll";
		const char *avCodecLibFile = "avcodec-58.dll";
		const char *avUtilLibFile = "avutil-56.dll";
		const char *swsScaleLibFile = "swscale-5.dll";
		DynamicLibraryHandle avFormatLib = DynamicLibraryLoad(avFormatLibFile);
		DynamicLibraryHandle avCodecLib = DynamicLibraryLoad(avCodecLibFile);
		DynamicLibraryHandle avUtilLib = DynamicLibraryLoad(avUtilLibFile);
		DynamicLibraryHandle swsScaleLib = DynamicLibraryLoad(swsScaleLibFile);
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avRegisterAll, ffmpeg_av_register_all_func, "av_register_all");
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avFormatCloseInput, ffmpeg_avformat_close_input_func, "avformat_close_input");
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avFormatOpenInput, ffmpeg_avformat_open_input_func, "avformat_open_input");
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avFormatFindStreamInfo, ffmpeg_avformat_find_stream_info_func, "avformat_find_stream_info");
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avDumpFormat, ffmpeg_av_dump_format_func, "av_dump_format");
		FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadFrame, ffmpeg_av_read_frame_func, "av_read_frame");

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

		FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameAlloc, ffmpeg_av_frame_alloc_func, "av_frame_alloc");
		FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameFree, ffmpeg_av_frame_free_func, "av_frame_free");
		FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetBufferSize, ffmpeg_av_image_get_buffer_size_func, "av_image_get_buffer_size");
		FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetLineSize, ffmpeg_av_image_get_linesize_func, "av_image_get_linesize");
		FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageFillArrays, ffmpeg_av_image_fill_arrays_func, "av_image_fill_arrays");

		FFMPEG_GET_FUNCTION_ADDRESS(swsScaleLib, swsScaleLibFile, ffmpeg.swsGetContext, ffmpeg_sws_getContext_func, "sws_getContext");
		FFMPEG_GET_FUNCTION_ADDRESS(swsScaleLib, swsScaleLibFile, ffmpeg.swsScale, ffmpeg_sws_scale_func, "sws_scale");
		FFMPEG_GET_FUNCTION_ADDRESS(swsScaleLib, swsScaleLibFile, ffmpeg.swsFreeContext, ffmpeg_sws_freeContext_func, "sws_freeContext");

		// Register all formats and codecs
		ffmpeg.avRegisterAll();

		// @TODO(final): Custom IO!

		// Open video file
		if (ffmpeg.avFormatOpenInput(&state.formatCtx, mediaFilePath, nullptr, nullptr) != 0) {
			ConsoleFormatError("Failed opening media file '%s'!\n", mediaFilePath);
			goto release;
		}
		// Retrieve stream information
		if (ffmpeg.avFormatFindStreamInfo(state.formatCtx, nullptr) < 0) {
			ConsoleFormatError("Failed getting stream informations for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Dump information about file onto standard error
		ffmpeg.avDumpFormat(state.formatCtx, 0, mediaFilePath, 0);

		// Find the first video stream
		int videoStream = -1;
		for (uint32_t steamIndex = 0; steamIndex < state.formatCtx->nb_streams; steamIndex++) {
			if (state.formatCtx->streams[steamIndex]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoStream = steamIndex;
				break;
			}
		}
		if (videoStream == -1) {
			ConsoleFormatError("No video stream in media file '%s' found!\n", mediaFilePath);
			goto release;
		}
		state.videoStreamIndex = videoStream;

		// Get a pointer to the video stream
		AVStream *pVideoStream = state.formatCtx->streams[videoStream];
		assert(pVideoStream->codecpar != nullptr);

		// Get codec name
		char codecName[5] = {};
		MemoryCopy(&pVideoStream->codecpar->codec_tag, 4, codecName);

		// Create video context
		state.videoCtx = ffmpeg.avcodecAllocContext3(nullptr);
		if (ffmpeg.avcodecParametersToContext(state.videoCtx, pVideoStream->codecpar) < 0) {
			ConsoleFormatError("Failed getting video context from codec '%s' in media file '%s'!\n", codecName, mediaFilePath);
			goto release;
		}

		// Find video decoder
		state.videoCodec = ffmpeg.avcodecFindDecoder(pVideoStream->codecpar->codec_id);
		if (state.videoCodec == nullptr) {
			ConsoleFormatError("Unsupported video codec '%s' in media file '%s' found!\n", codecName, mediaFilePath);
			goto release;
		}

		// Open codec
		if (ffmpeg.avcodecOpen2(state.videoCtx, state.videoCodec, nullptr) < 0) {
			ConsoleFormatError("Failed opening video codec '%s' from media file '%s'!\n", codecName, mediaFilePath);
			goto release;
		}

		// Allocate native video frame
		state.sourceNativeFrame = ffmpeg.avFrameAlloc();
		if (state.sourceNativeFrame == nullptr) {
			ConsoleFormatError("Failed allocating native video frame for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB video frame
		state.targetRGBFrame = ffmpeg.avFrameAlloc();
		if (state.targetRGBFrame == nullptr) {
			ConsoleFormatError("Failed allocating RGB video frame for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB buffer
		AVPixelFormat targetPixelFormat = AVPixelFormat::AV_PIX_FMT_BGR24;
		size_t rgbFrameSize = ffmpeg.avImageGetBufferSize(targetPixelFormat, state.videoCtx->width, state.videoCtx->height, 1);
		state.targetRGBBuffer = (uint8_t *)MemoryAlignedAllocate(rgbFrameSize, 16);

		// Setup RGB video frame and give it access to the actual data
		ffmpeg.avImageFillArrays(state.targetRGBFrame->data, state.targetRGBFrame->linesize, state.targetRGBBuffer, targetPixelFormat, state.videoCtx->width, state.videoCtx->height, 1);

		// Get software context
		state.softwareCtx = ffmpeg.swsGetContext(
			state.videoCtx->width,
			state.videoCtx->height,
			state.videoCtx->pix_fmt,
			state.videoCtx->width,
			state.videoCtx->height,
			targetPixelFormat,
			SWS_BILINEAR,
			nullptr,
			nullptr,
			nullptr
		);

		// Resize backbuffer to fit the video size
		ResizeVideoBackBuffer(state.videoCtx->width, state.videoCtx->height);
		VideoBackBuffer *backBuffer = GetVideoBackBuffer();

		// Create queues
		state.packetQueue = CreatePacketQueue(MAX_PACKET_QUEUE_COUNT);
		state.frameQueue = CreateFrameQueue(MAX_FRAME_QUEUE_COUNT);

		// Create threads
		ThreadContext *threads[] = {
			ThreadCreate(PacketReadThreadProc, &state),
			ThreadCreate(VideoDecodingThreadProc, &state),
		};

		//
		// App loop
		//
		while (WindowUpdate()) {
			// Get available frame in native format from the queue
			FrameItem *frameItem;
			if (TryGetFromList(state.frameQueue.availableList, &frameItem)) {
				AVFrame *sourceNativeFrame = frameItem->value;

				// @TODO(final): Decode picture format directly into the backbuffer, without the software scaling!

				// Convert native frame to target RGB24 frame
				ffmpeg.swsScale(state.softwareCtx, (uint8_t const * const *)sourceNativeFrame->data, sourceNativeFrame->linesize, 0, state.videoCtx->height, state.targetRGBFrame->data, state.targetRGBFrame->linesize);

				// Put native frame back into the freelist of the queue
				AddToList(state.frameQueue.freeList, frameItem);
				SignalWakeUp(state.frameQueue.freeList.availableSignal);

				// Convert RGB24 frame to RGB32 backbuffer
				ConvertRGB24ToBackBuffer(backBuffer, state.videoCtx->width, state.videoCtx->height, *state.targetRGBFrame->linesize, state.targetRGBBuffer);
			}


			// Present frame
			WindowFlip();
		}

		// Stop queues and wait until all threads are finished running
		state.packetQueue.isStopped = 1;
		SignalWakeUp(state.packetQueue.stoppedSignal);
		state.frameQueue.isStopped = 1;
		SignalWakeUp(state.frameQueue.stoppedSignal);
		ThreadWaitForAll(threads, FPL_ARRAYCOUNT(threads));

		// Release all threads and queues
		ThreadDestroy(threads[1]);
		ThreadDestroy(threads[0]);
		DestroyFrameQueue(state.frameQueue);
		DestroyPacketQueue(state.packetQueue);

	release:
		// Release media
		if (state.softwareCtx != nullptr) {
			ffmpeg.swsFreeContext(state.softwareCtx);
		}
		if (state.targetRGBBuffer != nullptr) {
			MemoryAlignedFree(state.targetRGBBuffer);
		}
		if (state.targetRGBFrame != nullptr) {
			ffmpeg.avFrameFree(&state.targetRGBFrame);
		}
		if (state.sourceNativeFrame != nullptr) {
			ffmpeg.avFrameFree(&state.sourceNativeFrame);
		}
		if (state.videoCtx != nullptr) {
			ffmpeg.avcodecFreeContext(&state.videoCtx);
		}
		if (state.formatCtx != nullptr) {
			ffmpeg.avFormatCloseInput(&state.formatCtx);
		}

		// Release FFMPEG
		DynamicLibraryUnload(avUtilLib);
		DynamicLibraryUnload(avCodecLib);
		DynamicLibraryUnload(avFormatLib);
		MemoryAlignedFree(globalFFMPEGFunctions);

		// Release platform
		ReleasePlatform();

		result = 0;
	} else {
		result = -1;
	}

	return(result);
}