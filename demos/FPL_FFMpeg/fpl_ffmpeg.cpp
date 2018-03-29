/*

Custom FFMPEG Media Player Demo based on FPL and ffplay.c
Written by Torsten Spaete

[x] Reads packets from stream and queues them up
[x] Decodes video and audio packets and queues them up as well
[x] FFMPEG functions are loaded dynamically
[x] Linked list for packet queue
[x] Handle PTS/DTS to schedule video frame
[x] Syncronize video to audio
[x] Fix memory leak (There was no leak at all)
[x] Support for FFMPEG static linking
[x] Rewrite Frame Queue to support Peek in Previous, Current and Next frame
[x] Introduce serials
[x] Introduce null and flush packet
[x] Restart
[x] Frame dropping using prev/next frame
[x] Pause/Resume
[x] OpenGL Video Rendering
[x] Syncronize audio to video
[x] Use same ffmpeg name from every dynamic function
[x] Fix bug for WMV always dropping nearly every frame (TimeBase was wrong all the time)
[x] Aspect ratio calculation
[x] Fullscreen toggling
[x] Modern OpenGL 3.3
[ ] OSD
[ ] Seeking (+/- 5 secs)
[ ] Frame by frame stepping
[ ] Support for audio format change while playing
[ ] Support for video format change while playing
[x] Image format conversion (YUY2, YUV > RGB24 etc.)
	[x] GLSL (YUV420P for now)
	[x] Slow CPU implementation (YUV420P for now)
[ ] Audio format conversion (Downsampling, Upsampling, S16 > F32 etc.)
	[ ] Slow CPU implementation
	[ ] Planar (Non-interleaved) samples to (Interleaved) Non-planar samples conversion
[ ] Composite video rendering
	[ ] Bitmap rect blitting
	[ ] Subtitle Decoding and Compositing
[ ] UI
	[ ] Current Time
	[ ] Buttons
	[ ] File dialog
	[ ] Seekbar
	[ ] Playlist
	[ ] Audio visualizer (FFT)

Docs:
- http://dranger.com/ffmpeg/tutorial01.html
- https://blogs.gentoo.org/lu_zero/2015/10/15/deprecating-avpicture/
- https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
- https://www.codeproject.com/tips/489450/creating-custom-ffmpeg-io-context

Requirements:
- Custom ffmpeg win64 build from https://ffmpeg.zeranoe.com/builds/

*/

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#include <assert.h> // assert

#include "defines.h"
#include "utils.h"
#include "maths.h"
#include "ffmpeg.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#if USE_HARDWARE_RENDERING
#	define FDYNGL_IMPLEMENTATION
#	include <final_dynamic_opengl.hpp>
#	include "shaders.h"
#endif

static FFMPEGContext ffmpeg = {};


static char glErrorCodeBuffer[16];
static const char *GetGLErrorString(const GLenum err) {
	switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			if (_itoa_s(err, glErrorCodeBuffer, FPL_ARRAYCOUNT(glErrorCodeBuffer), 10) == 0)
				return (const char *)glErrorCodeBuffer;
			else
				return "";
	}
}

static void CheckGLError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		const char *msg = GetGLErrorString(err);
		assert(!msg);
	}
}

//
// Stats
//
struct MemoryStats {
	volatile int32_t allocatedPackets;
	volatile int32_t usedPackets;
	volatile int32_t allocatedFrames;
	volatile int32_t usedFrames;
};

static MemoryStats globalMemStats = {};

inline void PrintMemStats() {
	int32_t allocatedPackets = fplAtomicLoadS32(&globalMemStats.allocatedPackets);
	int32_t usedPackets = fplAtomicLoadS32(&globalMemStats.usedPackets);
	int32_t allocatedFrames = fplAtomicLoadS32(&globalMemStats.allocatedFrames);
	int32_t usedFrames = fplAtomicLoadS32(&globalMemStats.usedFrames);
	fplConsoleFormatOut("Packets: %d / %d, Frames: %d / %d\n", allocatedPackets, usedPackets, allocatedFrames, usedFrames);
}

//
// Constants
//
// Max number of frames in the queues
constexpr uint32_t MAX_VIDEO_FRAME_QUEUE_COUNT = 4;
constexpr uint32_t MAX_AUDIO_FRAME_QUEUE_COUNT = 8;
constexpr uint32_t MAX_FRAME_QUEUE_COUNT = FPL_MAX(MAX_AUDIO_FRAME_QUEUE_COUNT, MAX_VIDEO_FRAME_QUEUE_COUNT);

// Total size of data from all packet queues
constexpr uint64_t MAX_PACKET_QUEUE_SIZE = FPL_MEGABYTES(16);

// Min number of packet frames in a single queue
constexpr uint32_t MIN_PACKET_FRAMES = 25;

// External clock min/max frames
constexpr uint32_t EXTERNAL_CLOCK_MIN_FRAMES = 2;
constexpr uint32_t EXTERNAL_CLOCK_MAX_FRAMES = 10;

// External clock speed adjustment constants for realtime sources based on buffer fullness
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

// No AV sync correction is done if below the minimum AV sync threshold
constexpr double AV_SYNC_THRESHOLD_MIN = 0.04;
// No AV sync correction is done if above the maximum AV sync threshold
constexpr double AV_SYNC_THRESHOLD_MAX = 0.1;
// No AV correction is done if too big error
constexpr double AV_NOSYNC_THRESHOLD = 10.0;
// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
constexpr double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1;
// Default refresh rate of 1/sec
constexpr double DEFAULT_REFRESH_RATE = 0.01;
// Number of audio measurements required to make an average.
constexpr int AV_AUDIO_DIFF_AVG_NB = 20;
// Maximum audio speed change to get correct sync
constexpr int AV_SAMPLE_CORRECTION_PERCENT_MAX = 10;
//
// Packet Queue
//
static AVPacket globalFlushPacket = {};

struct PacketList {
	AVPacket packet;
	PacketList *next;
	int32_t serial;
};

struct PacketQueue {
	fplMutexHandle lock;
	fplSignalHandle addedSignal;
	fplSignalHandle freeSignal;
	PacketList *first;
	PacketList *last;
	uint64_t size;
	uint64_t duration;
	int32_t packetCount;
	int32_t serial;
};

inline bool IsFlushPacket(PacketList *packet) {
	assert(packet != nullptr);
	bool result = (packet->packet.data == (uint8_t *)&globalFlushPacket);
	return(result);
}

inline PacketList *AllocatePacket(PacketQueue &queue) {
	PacketList *packet = (PacketList *)ffmpeg.av_mallocz(sizeof(PacketList));
	if (packet == nullptr) {
		return nullptr;
	}
	fplAtomicAddS32(&globalMemStats.allocatedPackets, 1);
	return(packet);
}

inline void DestroyPacket(PacketQueue &queue, PacketList *packet) {
	ffmpeg.av_freep(packet);
	fplAtomicAddS32(&globalMemStats.allocatedPackets, -1);
}

inline void ReleasePacketData(PacketList *packet) {
	if (!IsFlushPacket(packet)) {
		ffmpeg.av_packet_unref(&packet->packet);
	}
}

inline void ReleasePacket(PacketQueue &queue, PacketList *packet) {
	ReleasePacketData(packet);
	DestroyPacket(queue, packet);
	fplSignalSet(&queue.freeSignal);
}

inline bool AquirePacket(PacketQueue &queue, PacketList *&packet) {
	bool result = false;
	packet = AllocatePacket(queue);
	if (packet != nullptr) {
		result = true;
	}
	return(result);
}

static void FlushPacketQueue(PacketQueue &queue) {
	fplMutexLock(&queue.lock, UINT32_MAX);
	PacketList *p = queue.first;
	while (p != nullptr) {
		PacketList *n = p->next;
		ReleasePacketData(p);
		DestroyPacket(queue, p);
		p = n;
	}
	queue.first = queue.last = nullptr;
	queue.packetCount = 0;
	queue.size = 0;
	queue.duration = 0;
	fplMutexUnlock(&queue.lock);
}

static void DestroyPacketQueue(PacketQueue &queue) {
	FlushPacketQueue(queue);
	fplSignalDestroy(&queue.freeSignal);
	fplSignalDestroy(&queue.addedSignal);
	fplMutexDestroy(&queue.lock);
}

inline bool InitPacketQueue(PacketQueue &queue) {
	queue.lock = fplMutexCreate();
	if (!queue.lock.isValid) {
		return false;
	}
	queue.addedSignal = fplSignalCreate();
	if (!queue.addedSignal.isValid) {
		return false;
	}
	queue.freeSignal = fplSignalCreate();
	if (!queue.freeSignal.isValid) {
		return false;
	}
	return true;
}

inline void PushPacket(PacketQueue &queue, PacketList *packet) {
	fplMutexLock(&queue.lock, UINT32_MAX);
	{
		packet->next = nullptr;
		if (IsFlushPacket(packet)) {
			queue.serial++;
		}
		packet->serial = queue.serial;
		if (queue.first == nullptr) {
			queue.first = packet;
		}
		if (queue.last != nullptr) {
			assert(queue.last->next == nullptr);
			queue.last->next = packet;
		}
		queue.last = packet;
		queue.size += packet->packet.size + sizeof(*packet);
		queue.duration += packet->packet.duration;
		fplAtomicAddS32(&queue.packetCount, 1);
		fplAtomicAddS32(&globalMemStats.usedPackets, 1);
		fplSignalSet(&queue.addedSignal);
	}
	fplMutexUnlock(&queue.lock);
}

inline bool PopPacket(PacketQueue &queue, PacketList *&packet) {
	bool result = false;
	fplMutexLock(&queue.lock, UINT32_MAX);
	{
		if (queue.first != nullptr) {
			PacketList *p = queue.first;
			PacketList *n = p->next;
			queue.first = n;
			p->next = nullptr;
			packet = p;
			queue.duration -= packet->packet.duration;
			queue.size -= packet->packet.size + sizeof(*packet);
			if (queue.first == nullptr) {
				queue.last = nullptr;
			}
			fplAtomicAddS32(&queue.packetCount, -1);
			fplAtomicAddS32(&globalMemStats.usedPackets, -1);
			result = true;
		}
	}
	fplMutexUnlock(&queue.lock);
	return(result);
}

inline bool PushNullPacket(PacketQueue &queue, int streamIndex) {
	bool result = false;
	PacketList *packet = nullptr;
	if (AquirePacket(queue, packet)) {
		ffmpeg.av_init_packet(&packet->packet);
		packet->packet.data = nullptr;
		packet->packet.size = 0;
		packet->packet.stream_index = streamIndex;
		PushPacket(queue, packet);
		result = true;
	}
	return(result);
}

inline bool PushFlushPacket(PacketQueue &queue) {
	bool result = false;
	PacketList *packet = nullptr;
	if (AquirePacket(queue, packet)) {
		packet->packet = globalFlushPacket;
		PushPacket(queue, packet);
		result = true;
	}
	return(result);
}

inline void StartPacketQueue(PacketQueue &queue) {
	fplMutexLock(&queue.lock, UINT32_MAX);
	assert(PushFlushPacket(queue));
	fplMutexUnlock(&queue.lock);
}

//
// Frame Queue
//
struct Frame {
	AVRational sar;
	AVFrame *frame;
	double pts;
	double duration;
	int64_t pos;
	int32_t serial;
	int32_t width;
	int32_t height;
	bool isUploaded;
};

// @NOTE(final): This is a single producer single consumer fast ringbuffer queue.
// The read position can never pass the write position and vice versa.
inline AVFrame *AllocateFrame() {
	AVFrame *result = ffmpeg.av_frame_alloc();
	fplAtomicAddS32(&globalMemStats.allocatedFrames, 1);
	return(result);
}

inline void FreeFrameData(Frame *frame) {
	ffmpeg.av_frame_unref(frame->frame);
}

inline void FreeFrame(Frame *frame) {
	FreeFrameData(frame);
	ffmpeg.av_frame_free(&frame->frame);
}

struct FrameQueue {
	Frame frames[MAX_FRAME_QUEUE_COUNT];
	fplMutexHandle lock;
	fplSignalHandle signal;
	PacketList *pendingPacket;
	volatile uint32_t *stopped;
	int32_t readIndex;
	int32_t writeIndex;
	int32_t count;
	int32_t capacity;
	int32_t keepLast;
	int32_t readIndexShown;
	bool isValid;
	bool hasPendingPacket;
};

static bool InitFrameQueue(FrameQueue &queue, int32_t capacity, volatile uint32_t *stopped, int32_t keepLast) {
	queue = {};
	queue.capacity = FPL_MIN(capacity, MAX_FRAME_QUEUE_COUNT);
	for (int32_t i = 0; i < queue.capacity; ++i) {
		Frame *frame = queue.frames + i;
		frame->frame = AllocateFrame();
		if (frame->frame == nullptr) {
			return false;
		}
	}

	queue.keepLast = !!keepLast;
	queue.stopped = stopped;

	queue.lock = fplMutexCreate();
	if (!queue.lock.isValid) {
		return false;
	}

	queue.signal = fplSignalCreate();
	if (!queue.signal.isValid) {
		return false;
	}

	queue.isValid = true;
	return true;
}

static void DestroyFrameQueue(FrameQueue &queue) {
	fplSignalDestroy(&queue.signal);
	fplMutexDestroy(&queue.lock);
	for (int64_t i = 0; i < queue.capacity; ++i) {
		Frame *frame = queue.frames + i;
		FreeFrame(frame);
	}
}

static Frame *PeekFrameQueue(FrameQueue &queue) {
	return &queue.frames[(queue.readIndex + queue.readIndexShown) % queue.capacity];
}

static Frame *PeekFrameQueueNext(FrameQueue &queue) {
	return &queue.frames[(queue.readIndex + queue.readIndexShown + 1) % queue.capacity];
}

static Frame *PeekFrameQueueLast(FrameQueue &queue) {
	return &queue.frames[queue.readIndex];
}

static bool PeekWritableFromFrameQueue(FrameQueue &queue, Frame *&frame) {
	fplMutexLock(&queue.lock, UINT32_MAX);
	if (queue.count >= queue.capacity || *queue.stopped) {
		fplMutexUnlock(&queue.lock);
		return false;
	}
	fplMutexUnlock(&queue.lock);

	if (*queue.stopped) {
		return false;
	}

	frame = &queue.frames[queue.writeIndex];
	return true;
}

static bool PeekReadableFromFrameQueue(FrameQueue &queue, Frame *&frame) {
	fplMutexLock(&queue.lock, UINT32_MAX);
	if ((queue.count - queue.readIndexShown) <= 0 || *queue.stopped) {
		fplMutexUnlock(&queue.lock);
		return false;
	}
	fplMutexUnlock(&queue.lock);

	if (*queue.stopped) {
		return false;
	}

	frame = &queue.frames[(queue.readIndex + queue.readIndexShown) % queue.capacity];
	return true;
}

static void NextWritable(FrameQueue &queue) {
	queue.writeIndex = (queue.writeIndex + 1) % queue.capacity;

	fplMutexLock(&queue.lock, UINT32_MAX);
	queue.count++;
	fplSignalSet(&queue.signal);
	fplMutexUnlock(&queue.lock);
}

static void NextReadable(FrameQueue &queue) {
	if (queue.keepLast && !queue.readIndexShown) {
		queue.readIndexShown = 1;
		return;
	}

	FreeFrameData(&queue.frames[queue.readIndex]);
	queue.readIndex = (queue.readIndex + 1) % queue.capacity;

	fplMutexLock(&queue.lock, UINT32_MAX);
	queue.count--;
	fplSignalSet(&queue.signal);
	fplMutexUnlock(&queue.lock);
}

static int32_t GetFrameQueueRemainingCount(const FrameQueue &queue) {
	return queue.count - queue.readIndexShown;
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
	fplMutexHandle lock;
	fplSignalHandle stopSignal;
	fplSignalHandle resumeSignal;
	fplThreadHandle *thread;
	volatile uint32_t readPacketCount;
	volatile uint32_t stopRequest;
	bool isEOF;
};

static bool InitReader(ReaderContext &outReader) {
	outReader = {};
	outReader.lock = fplMutexCreate();
	if (!outReader.lock.isValid) {
		return false;
	}
	outReader.stopSignal = fplSignalCreate();
	if (!outReader.stopSignal.isValid) {
		return false;
	}
	outReader.resumeSignal = fplSignalCreate();
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
	fplSignalDestroy(&reader.resumeSignal);
	fplSignalDestroy(&reader.stopSignal);
	fplMutexDestroy(&reader.lock);
}

static void StopReader(ReaderContext &reader) {
	reader.stopRequest = 1;
	fplSignalSet(&reader.stopSignal);
	fplThreadWaitForOne(reader.thread, UINT32_MAX);
	fplThreadDestroy(reader.thread);
	reader.thread = nullptr;
}

static void StartReader(ReaderContext &reader, fpl_run_thread_function *readerThreadFunc, void *state) {
	reader.stopRequest = 0;
	assert(reader.thread == nullptr);
	reader.thread = fplThreadCreate(readerThreadFunc, state);
}

struct PlayerState;
struct Decoder {
	PacketQueue packetsQueue;
	FrameQueue frameQueue;
	fplMutexHandle lock;
	fplSignalHandle stopSignal;
	fplSignalHandle resumeSignal;
	fplThreadHandle *thread;
	PlayerState *state;
	ReaderContext *reader;
	MediaStream *stream;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	volatile uint32_t stopRequest;
	volatile uint32_t isEOF;
	volatile uint32_t decodedFrameCount;
	int32_t pktSerial;
	int32_t finishedSerial;
};

static bool InitDecoder(Decoder &outDecoder, PlayerState *state, ReaderContext *reader, MediaStream *stream, uint32_t frameCapacity, int32_t keepLast) {
	outDecoder = {};
	outDecoder.stream = stream;
	outDecoder.reader = reader;
	outDecoder.state = state;
	outDecoder.pktSerial = -1;
	outDecoder.start_pts = AV_NOPTS_VALUE;
	outDecoder.lock = fplMutexCreate();
	if (!outDecoder.lock.isValid) {
		return false;
	}
	outDecoder.stopSignal = fplSignalCreate();
	if (!outDecoder.stopSignal.isValid) {
		return false;
	}
	outDecoder.resumeSignal = fplSignalCreate();
	if (!outDecoder.resumeSignal.isValid) {
		return false;
	}
	if (!InitPacketQueue(outDecoder.packetsQueue)) {
		return false;
	}
	if (!InitFrameQueue(outDecoder.frameQueue, frameCapacity, &outDecoder.stopRequest, keepLast)) {
		return false;
	}

	return true;
}

static void DestroyDecoder(Decoder &decoder) {
	DestroyFrameQueue(decoder.frameQueue);
	DestroyPacketQueue(decoder.packetsQueue);
	fplSignalDestroy(&decoder.resumeSignal);
	fplSignalDestroy(&decoder.stopSignal);
	fplMutexDestroy(&decoder.lock);
}

static void StartDecoder(Decoder &decoder, fpl_run_thread_function *decoderThreadFunc) {
	StartPacketQueue(decoder.packetsQueue);
	assert(decoder.thread == nullptr);
	decoder.thread = fplThreadCreate(decoderThreadFunc, &decoder);
}

static void StopDecoder(Decoder &decoder) {
	decoder.stopRequest = 1;
	fplSignalSet(&decoder.stopSignal);
	fplThreadWaitForOne(decoder.thread, UINT32_MAX);
	fplThreadDestroy(decoder.thread);
	decoder.thread = nullptr;
	FlushPacketQueue(decoder.packetsQueue);
}

static void AddPacketToDecoder(Decoder &decoder, PacketList *targetPacket, AVPacket *sourcePacket) {
	targetPacket->packet = *sourcePacket;
	PushPacket(decoder.packetsQueue, targetPacket);
}

//
// Clock
//
struct Clock {
	double pts;
	double ptsDrift;
	double lastUpdated;
	double speed;
	int32_t *queueSerial;
	int32_t serial;
	bool isPaused;
};
enum class AVSyncType {
	AudioMaster,
	VideoMaster,
	ExternalClock,
};

//
// Video
//
struct Texture {
#if USE_HARDWARE_RENDERING
	GLuint id;
	GLuint pboId;
	GLuint target;
	GLint internalFormat;
	GLenum format;
#	if !USE_GL_PBO
	uint8_t *data;
#	endif
#else
	uint32_t id;
#endif
	uint32_t width;
	uint32_t height;
	uint32_t pixelSize;
	int32_t rowSize;
	uint32_t colorBits;
};

static bool InitTexture(Texture &texture, const uint32_t w, const uint32_t h, const uint32_t colorBits) {
	texture.width = w;
	texture.height = h;
	texture.colorBits = colorBits;

	int colorComponents = colorBits / 8;

	texture.pixelSize = colorComponents * sizeof(uint8_t);
	texture.rowSize = w * texture.pixelSize;

#if USE_HARDWARE_RENDERING
	size_t dataSize = texture.rowSize * texture.height;

#	if USE_GL_PBO
	glGenBuffers(1, &texture.pboId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, texture.pboId);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, dataSize, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#	else
	texture.data = (uint8_t *)MemoryAllocate(dataSize);
#	endif

#if USE_GL_RECTANGLE_TEXTURES
	texture.target = GL_TEXTURE_RECTANGLE;
#else
	texture.target = GL_TEXTURE_2D;
#endif

	texture.internalFormat = GL_RGBA8;
	texture.format = GL_RGBA;
	if (colorComponents == 1) {
		texture.internalFormat = GL_R8;
		texture.format = GL_RED;
	}

	glGenTextures(1, &texture.id);
	glBindTexture(texture.target, texture.id);
	glTexImage2D(texture.target, 0, texture.internalFormat, w, h, 0, texture.format, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(texture.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(texture.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(texture.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texture.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(texture.target, 0);
	CheckGLError();
#else
	texture.id = 1;
	ResizeVideoBackBuffer(w, h);
#endif

	return true;
}

inline uint8_t *LockTexture(Texture &texture) {
	uint8_t *result;

#if USE_HARDWARE_RENDERING
#	if USE_GL_PBO
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, texture.pboId);
	result = (uint8_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	CheckGLError();
#	else
	result = texture.data;
#	endif
#else
	VideoBackBuffer *backBuffer = GetVideoBackBuffer();
	result = (uint8_t *)backBuffer->pixels;
#endif

	return(result);
}

inline void UnlockTexture(Texture &texture) {
#if USE_HARDWARE_RENDERING
#	if USE_GL_PBO
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindTexture(texture.target, texture.id);
	glTexSubImage2D(texture.target, 0, 0, 0, texture.width, texture.height, texture.format, GL_UNSIGNED_BYTE, (void *)0);
	glBindTexture(texture.target, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	CheckGLError();
#	else
	glBindTexture(texture.target, texture.id);
	glTexSubImage2D(texture.target, 0, 0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, texture.data);
	glBindTexture(texture.target, 0);
#	endif
#endif
}

inline void DestroyTexture(Texture &texture) {
#if USE_HARDWARE_RENDERING
#	if !USE_GL_PBO
	MemoryFree(texture.data);
#	endif
	glDeleteTextures(1, &texture.id);
#	if USE_GL_PBO
	glDeleteBuffers(1, &texture.pboId);
#	endif
#endif

	texture = {};
}

struct VideoShader {
	GLuint programId;
	GLuint uniform_uniProjMat;
	GLuint uniform_uniTextures;
	GLuint uniform_uniTextureScaleY;
	GLuint uniform_uniTextureOffsetY;
};

constexpr uint32_t MAX_TARGET_TEXTURE_COUNT = 4;
struct VideoContext {
	MediaStream stream;
	Decoder decoder;
	Clock clock;
	Texture targetTextures[MAX_TARGET_TEXTURE_COUNT];
#if USE_HARDWARE_RENDERING
	VideoShader basicShader;
	VideoShader yuv420pShader;
	GLuint vao;
	GLuint vertexBufferId;
	GLuint indexBufferId;
	VideoShader *activeShader;
#endif
	SwsContext *softwareScaleCtx;
	uint32_t targetTextureCount;
};

inline void FlipSourcePicture(uint8_t *srcData[8], int srcLineSize[8], int height) {
	int h0 = srcLineSize[0];
	for (int i = 0; i < 8; ++i) {
		int hi = srcLineSize[i];
		if (hi == 0) {
			break;
		}
		int h;
		if (hi != h0) {
			int div = h0 / hi;
			h = (height / div) - 1;
		} else {
			h = height - 1;
		}
		srcData[i] = srcData[i] + srcLineSize[i] * h;
		srcLineSize[i] = -srcLineSize[i];
	}

}

static void UploadTexture(VideoContext &video, const AVFrame *sourceNativeFrame) {
	AVCodecContext *videoCodecCtx = video.stream.codecContext;
#if USE_HARDWARE_RENDERING && USE_HARDWARE_IMAGE_FORMAT_DECODING
	switch (sourceNativeFrame->format) {
		case AVPixelFormat::AV_PIX_FMT_YUV420P:
			assert(video.targetTextureCount == 3);
			for (uint32_t textureIndex = 0; textureIndex < video.targetTextureCount; ++textureIndex) {
				Texture &targetTexture = video.targetTextures[textureIndex];
				uint8_t *data = LockTexture(targetTexture);
				assert(data != nullptr);
				uint32_t h = (textureIndex == 0) ? sourceNativeFrame->height : sourceNativeFrame->height / 2;
				fplMemoryCopy(sourceNativeFrame->data[textureIndex], sourceNativeFrame->linesize[textureIndex] * h, data);
				UnlockTexture(targetTexture);
			}
			break;
		default:
			break;
	}
#else
	assert(video.targetTextureCount == 1);
	Texture &targetTexture = video.targetTextures[0];
	assert(targetTexture.width == sourceNativeFrame->width);
	assert(targetTexture.height == sourceNativeFrame->height);

	uint8_t *data = LockTexture(targetTexture);
	assert(data != nullptr);

	int32_t dstLineSize[8] = { targetTexture.rowSize, 0 };
	uint8_t *dstData[8] = { data, nullptr };
	uint8_t *srcData[8];
	int srcLineSize[8];
	for (int i = 0; i < 8; ++i) {
		srcData[i] = sourceNativeFrame->data[i];
		srcLineSize[i] = sourceNativeFrame->linesize[i];
	}

#if USE_FFMPEG_SOFTWARE_CONVERSION
	ffmpeg.sws_scale(video.softwareScaleCtx, (uint8_t const * const *)srcData, srcLineSize, 0, videoCodecCtx->height, dstData, dstLineSize);
#else
	ConversionFlags flags = ConversionFlags::None;
#	if USE_HARDWARE_RENDERING
	flags |= ConversionFlags::DstBGRA;
#	endif
	switch (sourceNativeFrame->format) {
		case AVPixelFormat::AV_PIX_FMT_YUV420P:
			ConvertYUV420PToRGB32(dstData, dstLineSize, targetTexture.width, targetTexture.height, srcData, srcLineSize, flags);
			break;
		default:
			ffmpeg.sws_scale(video.softwareScaleCtx, (uint8_t const * const *)srcData, srcLineSize, 0, videoCodecCtx->height, dstData, dstLineSize);
			break;
	}
#endif
	UnlockTexture(targetTexture);
#endif
}

//
// Audio
//
struct AudioContext {
	MediaStream stream;
	Decoder decoder;
	fplAudioDeviceFormat audioSource;
	fplAudioDeviceFormat audioTarget;
	Clock clock;
	double audioClock;
	int32_t audioClockSerial;
	int32_t audioDiffAvgCount;
	double audioDiffCum;
	double audioDiffAbgCoef;
	double audioDiffThreshold;

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
	int32_t frameDrop;
	int32_t reorderDecoderPTS;
	bool isInfiniteBuffer;
	bool isLoop;
	bool isVideoDisabled;
	bool isAudioDisabled;
};

inline void InitPlayerSettings(PlayerSettings &settings) {
	settings.startTime = {};
	settings.duration = {};
	settings.frameDrop = 1;
	settings.isInfiniteBuffer = false;
	settings.isLoop = false;
	settings.reorderDecoderPTS = -1;
}

struct SeekState {
	int64_t pos;
	int64_t rel;
	int seekFlags;
	bool isRequired;
};

constexpr uint32_t MAX_STREAM_COUNT = 8;
struct PlayerState {
	ReaderContext reader;
	MediaStream stream[MAX_STREAM_COUNT];
	VideoContext video;
	AudioContext audio;
	PlayerSettings settings;
	Clock externalClock;
	SeekState seek;
	AVFormatContext *formatCtx;
	fplWindowSize viewport;
	double frameLastPTS;
	double frameLastDelay;
	double frameTimer;
	double maxFrameDuration;
	AVSyncType syncType;
	volatile uint32_t forceRefresh;
	int loop;
	int readPauseReturn;
	int step;
	int frame_drops_early;
	int frame_drops_late;
	bool isInfiniteBuffer;
	bool isRealTime;
	bool isPaused;
	bool lastPaused;
	bool isFullscreen;
};

inline void PutPacketBackToReader(ReaderContext &reader, PacketList *packet) {
	ReleasePacket(reader.packetQueue, packet);
}

inline bool StreamHasEnoughPackets(const AVStream *stream, const int streamIndex, const PacketQueue &queue) {
	bool result = (streamIndex < 0) ||
		(stream->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
		((queue.packetCount > MIN_PACKET_FRAMES) && (!queue.duration || (av_q2d(stream->time_base) * queue.duration) > 1.0));
	return (result);
}

inline AVSampleFormat MapAudioFormatType(const fplAudioFormatType format) {
	// @TODO(final): Support planar formats as well
	switch (format) {
		case fplAudioFormatType_U8:
			return AVSampleFormat::AV_SAMPLE_FMT_U8;
		case fplAudioFormatType_S16:
			return AVSampleFormat::AV_SAMPLE_FMT_S16;
		case fplAudioFormatType_S32:
			return AVSampleFormat::AV_SAMPLE_FMT_S32;
		case fplAudioFormatType_F32:
			return AVSampleFormat::AV_SAMPLE_FMT_FLT;
		case fplAudioFormatType_F64:
			return AVSampleFormat::AV_SAMPLE_FMT_DBL;
		default:
			return AVSampleFormat::AV_SAMPLE_FMT_NONE;
	}
}

inline fplAudioFormatType MapAVSampleFormat(const AVSampleFormat format) {
	switch (format) {
		case AV_SAMPLE_FMT_U8:
		case AV_SAMPLE_FMT_U8P:
			return fplAudioFormatType_U8;
		case AV_SAMPLE_FMT_S16:
		case AV_SAMPLE_FMT_S16P:
			return fplAudioFormatType_S16;
		case AV_SAMPLE_FMT_S32:
		case AV_SAMPLE_FMT_S32P:
			return fplAudioFormatType_S32;
		case AV_SAMPLE_FMT_S64:
		case AV_SAMPLE_FMT_S64P:
			return fplAudioFormatType_S64;
		case AV_SAMPLE_FMT_FLT:
		case AV_SAMPLE_FMT_FLTP:
			return fplAudioFormatType_F32;
		case AV_SAMPLE_FMT_DBL:
		case AV_SAMPLE_FMT_DBLP:
			return fplAudioFormatType_F64;
		default:
			return fplAudioFormatType_None;
	}
}

inline bool IsPlanarAVSampleFormat(const AVSampleFormat format) {
	switch (format) {
		case AV_SAMPLE_FMT_U8P:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_S64P:
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_DBLP:
			return true;
		default:
			return false;
	}
}

inline AVSyncType GetMasterSyncType(const PlayerState *state) {
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

inline double GetClock(const Clock &clock) {
	if (*clock.queueSerial != clock.serial) {
		return NAN;
	}
	double result;
	if (clock.isPaused) {
		result = clock.pts;
	} else {
		double time = ffmpeg.av_gettime_relative() / (double)AV_TIME_BASE;
		result = clock.ptsDrift + time - (time - clock.lastUpdated) * (1.0 - clock.speed);
	}
	return(result);
}

inline void SetClockAt(Clock &clock, const double pts, const int32_t serial, const double time) {
	clock.pts = pts;
	clock.lastUpdated = time;
	clock.ptsDrift = clock.pts - time;
	clock.serial = serial;
}

inline void SetClock(Clock &clock, const double pts, const int32_t serial) {
	double time = ffmpeg.av_gettime_relative() / (double)AV_TIME_BASE;
	SetClockAt(clock, pts, serial, time);
}

inline void SetClockSpeed(Clock &clock, const double speed) {
	SetClock(clock, GetClock(clock), clock.serial);
	clock.speed = speed;
}

inline void InitClock(Clock &clock, int32_t *queueSerial) {
	clock.speed = 1.0;
	clock.isPaused = false;
	clock.queueSerial = queueSerial;
	SetClock(clock, NAN, -1);
}

inline void SyncClockToSlave(Clock &c, const Clock &slave) {
	double clock = GetClock(c);
	double slaveClock = GetClock(slave);
	if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > AV_NOSYNC_THRESHOLD)) {
		SetClock(c, slaveClock, slave.serial);
	}
}

inline double GetMasterClock(const PlayerState *state) {
	double result = 0;
	switch (GetMasterSyncType(state)) {
		case AVSyncType::VideoMaster:
			result = GetClock(state->video.clock);
			break;
		case AVSyncType::AudioMaster:
			result = GetClock(state->audio.clock);
			break;
		case AVSyncType::ExternalClock:
			result = GetClock(state->externalClock);
			break;
	}
	return result;
}

inline void UpdateExternalClockSpeed(PlayerState *state) {
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

inline void AddFrameToDecoder(Decoder &decoder, Frame *frame, AVFrame *srcFrame) {
	ffmpeg.av_frame_move_ref(frame->frame, srcFrame);
	NextWritable(decoder.frameQueue);
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

static DecodeResult DecodeFrame(ReaderContext &reader, Decoder &decoder, AVFrame *frame) {
	assert(decoder.stream != nullptr);
	AVCodecContext *codecCtx = decoder.stream->codecContext;
	int ret = AVERROR(EAGAIN);
	PacketList *pkt;
	for (;;) {
		if (decoder.packetsQueue.serial == decoder.pktSerial) {
			do {
				if (decoder.isEOF) {
					return DecodeResult::Skipped;
				}
				if (decoder.stopRequest) {
					return DecodeResult::Stopped;
				}

				switch (codecCtx->codec_type) {
					case AVMediaType::AVMEDIA_TYPE_VIDEO:
					{
						ret = ffmpeg.avcodec_receive_frame(codecCtx, frame);
						if (ret >= 0) {
							if (decoder.state->settings.reorderDecoderPTS == -1) {
								frame->pts = frame->best_effort_timestamp;
							} else if (!decoder.state->settings.reorderDecoderPTS) {
								frame->pts = frame->pkt_dts;
							}
						}
					} break;

					case AVMediaType::AVMEDIA_TYPE_AUDIO:
					{
						ret = ffmpeg.avcodec_receive_frame(codecCtx, frame);
						if (ret >= 0) {
							AVRational tb = { 1, frame->sample_rate };
							if (frame->pts != AV_NOPTS_VALUE) {
								frame->pts = ffmpeg.av_rescale_q(frame->pts, codecCtx->pkt_timebase, tb);
							} else if (decoder.next_pts != AV_NOPTS_VALUE) {
								frame->pts = ffmpeg.av_rescale_q(decoder.next_pts, decoder.next_pts_tb, tb);
							}
							if (frame->pts != AV_NOPTS_VALUE) {
								decoder.next_pts = frame->pts + frame->nb_samples;
								decoder.next_pts_tb = tb;
							}
						}
					} break;
				}
				if (ret >= 0) {
					return DecodeResult::Success;
				} else if (ret == AVERROR_EOF) {
					decoder.finishedSerial = decoder.pktSerial;
					ffmpeg.avcodec_flush_buffers(codecCtx);
					return DecodeResult::EndOfStream;
				} else if (ret == AVERROR(EAGAIN)) {
					// This will continue sending packets until the frame is complete
					break;
				} else {
					return DecodeResult::Failed;
				}
			} while (ret != AVERROR(EAGAIN));
		}

		do {
			if (decoder.frameQueue.hasPendingPacket) {
				assert(decoder.frameQueue.pendingPacket != nullptr);
				pkt = decoder.frameQueue.pendingPacket;
				decoder.frameQueue.hasPendingPacket = false;
			} else {
				pkt = nullptr;
				if (PopPacket(decoder.packetsQueue, pkt)) {
					decoder.pktSerial = pkt->serial;
				} else {
					// We cannot continue to decode, because the packet queue is empty
					return DecodeResult::RequireMorePackets;
				}
			}
		} while (decoder.packetsQueue.serial != decoder.pktSerial);

		if (pkt != nullptr) {
			if (IsFlushPacket(pkt)) {
				ffmpeg.avcodec_flush_buffers(decoder.stream->codecContext);
				decoder.finishedSerial = 0;
				decoder.next_pts = decoder.start_pts;
				decoder.next_pts_tb = decoder.start_pts_tb;
				PutPacketBackToReader(reader, pkt);
			} else {
				if (ffmpeg.avcodec_send_packet(codecCtx, &pkt->packet) == AVERROR(EAGAIN)) {
					decoder.frameQueue.hasPendingPacket = true;
					decoder.frameQueue.pendingPacket = pkt;
				} else {
					PutPacketBackToReader(reader, pkt);
				}
			}
		}
	}
}

static void QueuePicture(Decoder &decoder, AVFrame *sourceFrame, Frame *targetFrame, const int32_t serial) {
	assert(targetFrame != nullptr);
	assert(targetFrame->frame != nullptr);
	assert(targetFrame->frame->pkt_size <= 0);
	assert(targetFrame->frame->width == 0);

	AVStream *videoStream = decoder.stream->stream;

	AVRational currentTimeBase = videoStream->time_base;
	AVRational currentFrameRate = ffmpeg.av_guess_frame_rate(decoder.state->formatCtx, videoStream, nullptr);

	targetFrame->pos = sourceFrame->pkt_pos;
	targetFrame->pts = (sourceFrame->pts == AV_NOPTS_VALUE) ? NAN : sourceFrame->pts * av_q2d(currentTimeBase);
	targetFrame->duration = (currentFrameRate.num && currentFrameRate.den ? av_q2d({ currentFrameRate.den, currentFrameRate.num }) : 0);
	targetFrame->serial = serial;
	targetFrame->isUploaded = false;
	targetFrame->sar = sourceFrame->sample_aspect_ratio;
	targetFrame->width = sourceFrame->width;
	targetFrame->height = sourceFrame->height;

#if PRINT_PTS
	ConsoleFormatOut("PTS V: %7.2f, Next: %7.2f\n", targetFrame->pts, decoder.next_pts);
#endif

	AddFrameToDecoder(decoder, targetFrame, sourceFrame);
}

static void VideoDecodingThreadProc(const fplThreadHandle *thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != nullptr);

	ReaderContext &reader = *decoder->reader;

	MediaStream *stream = decoder->stream;
	assert(stream != nullptr);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	PlayerState *state = decoder->state;

	fplSignalHandle *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Frame queue changed
		&decoder->frameQueue.signal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	AVStream *videoStream = decoder->stream->stream;

	AVFrame *sourceFrame = ffmpeg.av_frame_alloc();
	bool hasDecodedFrame = false;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		fplSignalWaitForAny(&decoder->lock, waitSignals, FPL_ARRAYCOUNT(waitSignals), UINT32_MAX);

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isEOF) {
			fplThreadSleep(10);
			continue;
		}

		if (!hasDecodedFrame) {
			// Decode video frame
			DecodeResult decodeResult = DecodeFrame(reader, *decoder, sourceFrame);
			if (decodeResult != DecodeResult::Success) {
				if (decodeResult != DecodeResult::RequireMorePackets) {
					ffmpeg.av_frame_unref(sourceFrame);
				}
				if (decodeResult == DecodeResult::EndOfStream) {
					decoder->isEOF = 1;
					continue;
				} else if (decodeResult <= DecodeResult::Stopped) {
					break;
				}

				// Stream finished and no packets left to decode, then are finished as well
				if (reader.isEOF && (decoder->packetsQueue.packetCount == 0)) {
					decoder->isEOF = 1;
				}
			} else {
#if PRINT_QUEUE_INFOS
				uint32_t decodedVideoFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
				ConsoleFormatOut("Decoded video frame %lu\n", decodedVideoFrameIndex);
#endif
				hasDecodedFrame = true;

				if (state->settings.frameDrop > 0 || (state->settings.frameDrop && GetMasterSyncType(state) != AVSyncType::VideoMaster)) {
					double dpts = NAN;
					if (sourceFrame->pts != AV_NOPTS_VALUE) {
						dpts = av_q2d(stream->stream->time_base) * sourceFrame->pts;
					}
					if (!isnan(dpts)) {
						double diff = dpts - GetMasterClock(state);
						if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
							diff < 0 &&
							decoder->pktSerial == state->video.clock.serial &&
							decoder->packetsQueue.packetCount) {
							state->frame_drops_early++;
							ffmpeg.av_frame_unref(sourceFrame);
							hasDecodedFrame = false;
#if PRINT_FRAME_DROPS
							fplConsoleFormatError("Frame drops: %d/%d\n", state->frame_drops_early, state->frame_drops_late);
#endif
						}
					}
				}
			}
		}

		if (hasDecodedFrame) {
			Frame *targetFrame = nullptr;
			if (PeekWritableFromFrameQueue(decoder->frameQueue, targetFrame)) {
				QueuePicture(*decoder, sourceFrame, targetFrame, decoder->pktSerial);
				ffmpeg.av_frame_unref(sourceFrame);
				hasDecodedFrame = false;
			}
		}

	}
	ffmpeg.av_frame_free(&sourceFrame);
}

static void QueueSamples(Decoder &decoder, AVFrame *sourceFrame, Frame *targetFrame, int32_t serial) {
	assert(targetFrame != nullptr);
	assert(targetFrame->frame != nullptr);
	assert(targetFrame->frame->pkt_size <= 0);
	assert(targetFrame->frame->nb_samples == 0);

	AVStream *audioStream = decoder.stream->stream;
	AVRational currentTimeBase = { 1, sourceFrame->sample_rate };

	targetFrame->pos = sourceFrame->pkt_pos;
	targetFrame->pts = (sourceFrame->pts == AV_NOPTS_VALUE) ? NAN : sourceFrame->pts * av_q2d(currentTimeBase);
	targetFrame->duration = av_q2d({ sourceFrame->nb_samples, sourceFrame->sample_rate });
	targetFrame->serial = serial;

#if PRINT_PTS
	ConsoleFormatOut("PTS A: %7.2f, Next: %7.2f\n", targetFrame->pts, decoder.next_pts);
#endif

	AddFrameToDecoder(decoder, targetFrame, sourceFrame);
}

static int SyncronizeAudio(PlayerState *state, const uint32_t sampleCount) {
	int result = sampleCount;
	if (GetMasterSyncType(state) != AVSyncType::AudioMaster) {
		double diff = GetClock(state->audio.clock) - GetMasterClock(state);
		if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
			state->audio.audioDiffCum = diff + state->audio.audioDiffAbgCoef * state->audio.audioDiffCum;
			if (state->audio.audioDiffAvgCount < AV_AUDIO_DIFF_AVG_NB) {
				// Not enough measures to have a correct estimate
				state->audio.audioDiffAvgCount++;
			} else {
				// Estimate the A-V difference 
				double avgDiff = state->audio.audioDiffCum * (1.0 - state->audio.audioDiffAbgCoef);
				if (fabs(avgDiff) >= state->audio.audioDiffThreshold) {
					result = sampleCount + (int)(diff * state->audio.audioSource.sampleRate);
					int min_nb_samples = ((sampleCount * (100 - AV_SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					int max_nb_samples = ((sampleCount * (100 + AV_SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					result = av_clip(result, min_nb_samples, max_nb_samples);
				}
			}
		} else {
			// Too big difference : may be initial PTS errors, so reset A-V filter
			state->audio.audioDiffAvgCount = 0;
			state->audio.audioDiffCum = 0;
		}
	}
	return(result);
}

static void AudioDecodingThreadProc(const fplThreadHandle *thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != nullptr);

	ReaderContext &reader = *decoder->reader;

	PlayerState *state = decoder->state;
	assert(state != nullptr);

	MediaStream *stream = decoder->stream;
	assert(stream != nullptr);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	fplSignalHandle *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Frame queue changed
		&decoder->frameQueue.signal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	AVFrame *sourceFrame = ffmpeg.av_frame_alloc();
	bool hasDecodedFrame = false;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		fplSignalWaitForAny(&decoder->lock, waitSignals, FPL_ARRAYCOUNT(waitSignals), UINT32_MAX);

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isEOF) {
			continue;
		}

		if (!hasDecodedFrame) {
			// Decode video frame
			DecodeResult decodeResult = DecodeFrame(reader, *decoder, sourceFrame);
			if (decodeResult != DecodeResult::Success) {
				if (decodeResult != DecodeResult::RequireMorePackets) {
					ffmpeg.av_frame_unref(sourceFrame);
				}
				if (decodeResult == DecodeResult::EndOfStream) {
					decoder->isEOF = 1;
					continue;
				} else if (decodeResult <= DecodeResult::Stopped) {
					break;
				}

				// Stream finished and no packets left to decode, then are finished as well
				if (reader.isEOF && (decoder->packetsQueue.packetCount == 0)) {
					decoder->isEOF = 1;
				}
			} else {
#if PRINT_QUEUE_INFOS
				uint32_t decodedAudioFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
				ConsoleFormatOut("Decoded audio frame %lu\n", decodedAudioFrameIndex);
#endif
				hasDecodedFrame = true;
			}
		}

		if (hasDecodedFrame) {
			Frame *targetFrame = nullptr;
			if (PeekWritableFromFrameQueue(decoder->frameQueue, targetFrame)) {
				QueueSamples(*decoder, sourceFrame, targetFrame, decoder->pktSerial);
				ffmpeg.av_frame_unref(sourceFrame);
				hasDecodedFrame = false;
			}
		}
	}
	ffmpeg.av_frame_free(&sourceFrame);
}

static void WriteSilenceSamples(AudioContext *audio, uint32_t remainingFrameCount, uint32_t outputSampleStride, uint8_t *conversionAudioBuffer) {
	audio->conversionAudioFramesRemaining = remainingFrameCount;
	audio->conversionAudioFrameIndex = 0;
	size_t bytesToClear = remainingFrameCount * outputSampleStride;
	fplMemoryClear(conversionAudioBuffer, bytesToClear);

}

static uint32_t AudioReadCallback(const fplAudioDeviceFormat *nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	double audioCallbackTime = (double)ffmpeg.av_gettime_relative();

	// FPL Audio Frame = Audio Sample = [Left S16][Right S16]
	// FPL Output frame are interleaved only!
	// FFMPEG Planar audio: (Each channel a plane -> AVFrame->data[channel])
	// FFMPEG Non-planar audio (One plane, Interleaved samples -> AVFrame->extended_data) [Left][Right],[Left][Right],..
	AudioContext *audio = (AudioContext *)userData;
	assert(audio != nullptr);

	Decoder &decoder = audio->decoder;

	PlayerState *state = decoder.state;

	uint32_t result = 0;

	if (audio->stream.isValid) {
		uint8_t *conversionAudioBuffer = audio->conversionAudioBuffer;
		uint32_t maxConversionAudioBufferSize = audio->maxConversionAudioBufferSize;

		uint32_t outputSampleStride = fplGetAudioFrameSizeInBytes(nativeFormat->type, nativeFormat->channels);
		uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

		uint32_t remainingFrameCount = frameCount;
		while (remainingFrameCount > 0) {
			if (state->isPaused) {
				WriteSilenceSamples(audio, remainingFrameCount, outputSampleStride, conversionAudioBuffer);
			}

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

				fplMemoryCopy(conversionAudioBuffer + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

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
			if (audio->pendingAudioFrame != nullptr) {
				assert(audio->conversionAudioFramesRemaining == 0);
				Frame *audioFrame = audio->pendingAudioFrame;
				assert(audioFrame->frame != nullptr);
				audio->pendingAudioFrame = nullptr;

				// Get conversion sample count
				const uint32_t maxConversionSampleCount = audio->maxConversionAudioFrameCount;
				int wantedSampleCount = SyncronizeAudio(state, audioFrame->frame->nb_samples);
				int conversionSampleCount = wantedSampleCount * nativeFormat->sampleRate / audioFrame->frame->sample_rate + 256;

				// @TODO(final): Handle audio format change here!

				//
				// Convert samples
				//
				const uint32_t sourceSampleCount = audioFrame->frame->nb_samples;
				const uint32_t sourceChannels = audioFrame->frame->channels;
				const uint32_t sourceFrameCount = sourceSampleCount;
				uint8_t **sourceSamples = audioFrame->frame->extended_data;
				// @TODO(final): Support for converting planar audio samples
				uint8_t *targetSamples[8] = {};
				targetSamples[0] = audio->conversionAudioBuffer;

				// @NOTE(final): Conversion buffer needs to be big enough to hold the samples for the frame
				assert(conversionSampleCount <= (int)maxConversionSampleCount);
				int samplesPerChannel = ffmpeg.swr_convert(audio->softwareResampleCtx, (uint8_t **)targetSamples, conversionSampleCount, (const uint8_t **)sourceSamples, sourceSampleCount);

				// We are done with this audio frame, release it
				NextReadable(decoder.frameQueue);

				// Update audio clock
				if (!isnan(audioFrame->pts)) {
					state->audio.audioClock = audioFrame->pts + (double)audioFrame->frame->nb_samples / (double)audioFrame->frame->sample_rate;
				} else {
					state->audio.audioClock = NAN;
				}
				state->audio.audioClockSerial = audioFrame->serial;

				if (samplesPerChannel <= 0) {
					break;
				}

				audio->conversionAudioFramesRemaining = samplesPerChannel;
				audio->conversionAudioFrameIndex = 0;
			}

			if ((audio->pendingAudioFrame == nullptr) && (audio->conversionAudioFramesRemaining == 0)) {
				Frame *newAudioFrame;
				if (!state->isPaused && PeekReadableFromFrameQueue(decoder.frameQueue, newAudioFrame)) {
					if (newAudioFrame->serial != decoder.packetsQueue.serial) {
						NextReadable(decoder.frameQueue);
						continue;
					}
					audio->pendingAudioFrame = newAudioFrame;
					audio->conversionAudioFrameIndex = 0;
					audio->conversionAudioFramesRemaining = 0;
					continue;
				} else {
					// No audio frame available, write silence for the remaining frames
					if (remainingFrameCount > 0) {
						WriteSilenceSamples(audio, remainingFrameCount, outputSampleStride, conversionAudioBuffer);
					} else {
						break;
					}
				}
			}
		}

		// Update audio clock
		if (!isnan(audio->audioClock)) {
			uint32_t writtenSize = result * outputSampleStride;
			double pts = audio->audioClock - (double)(nativeFormat->periods * nativeFormat->bufferSizeInBytes + writtenSize) / state->audio.audioTarget.bufferSizeInBytes;
			SetClockAt(audio->clock, pts, audio->audioClockSerial, audioCallbackTime / (double)AV_TIME_BASE);
			SyncClockToSlave(state->externalClock, audio->clock);
		}

	}

	return(result);
}

static void StreamTogglePause(PlayerState *state) {
	if (state->isPaused) {
		state->frameTimer += (ffmpeg.av_gettime_relative() / (double)AV_TIME_BASE) - state->video.clock.lastUpdated;
		if (state->readPauseReturn != AVERROR(ENOSYS)) {
			state->video.clock.isPaused = false;
		}
		SetClock(state->video.clock, GetClock(state->video.clock), state->video.clock.serial);
	}
	SetClock(state->externalClock, GetClock(state->externalClock), state->externalClock.serial);
	state->isPaused = state->audio.clock.isPaused = state->video.clock.isPaused = state->externalClock.isPaused = !state->isPaused;
}

static void SeekStream(SeekState *state, int64_t pos, int64_t rel, bool seekInBytes) {
	if (!state->isRequired) {
		state->pos = pos;
		state->rel = rel;
		state->seekFlags &= ~AVSEEK_FLAG_BYTE;
		if (seekInBytes)
			state->seekFlags |= AVSEEK_FLAG_BYTE;
		state->isRequired = 1;
	}
}

static void ToggleFullscreen(PlayerState *state) {
	if (state->isFullscreen) {
		fplSetWindowFullscreen(false, 0, 0, 0);
		state->isFullscreen = false;
	} else {
		state->isFullscreen = fplSetWindowFullscreen(true, 0, 0, 0);
	}
}

static void TogglePause(PlayerState *state) {
	StreamTogglePause(state);
	state->step = false;
}

static void StepToNextFrame(PlayerState *state) {
	if (state->isPaused) {
		StreamTogglePause(state);
	}
	state->step = 1;
}

static void PacketReadThreadProc(const fplThreadHandle *thread, void *userData) {
	PlayerState *state = (PlayerState *)userData;
	assert(state != nullptr);

	ReaderContext &reader = state->reader;
	VideoContext &video = state->video;
	AudioContext &audio = state->audio;
	MediaStream *videoStream = video.decoder.stream;
	MediaStream *audioStream = audio.decoder.stream;
	AVFormatContext *formatCtx = state->formatCtx;
	assert(formatCtx != nullptr);

	fplSignalHandle *waitSignals[] = {
		// We got a free packet for use to read into
		&reader.packetQueue.freeSignal,
		// Reader should terminate
		&reader.stopSignal,
		// Reader can continue
		&reader.resumeSignal,
	};

	bool skipWait = true;
	AVPacket srcPacket;
	bool hasPendingPacket = false;
	for (;;) {
		// Wait for any signal or skip wait
		if (!skipWait) {
			fplSignalWaitForAny(&reader.lock, waitSignals, FPL_ARRAYCOUNT(waitSignals), UINT32_MAX);
		} else {
			skipWait = false;
		}

		// Stop reader
		if (reader.stopRequest) {
			break;
		}

		// Pause
		if (state->isPaused != state->lastPaused) {
			state->lastPaused = state->isPaused;
			if (state->isPaused) {
				state->readPauseReturn = ffmpeg.av_read_pause(formatCtx);
			} else {
				ffmpeg.av_read_play(formatCtx);
			}
		}

		// Seeking
		if (state->seek.isRequired) {
			int64_t seekTarget = state->seek.pos;
			int64_t seekMin = state->seek.rel > 0 ? seekTarget - state->seek.rel + 2 : INT64_MIN;
			int64_t seekMax = state->seek.rel < 0 ? seekTarget - state->seek.rel - 2 : INT64_MAX;
			int seekResult = ffmpeg.avformat_seek_file(formatCtx, -1, seekMin, seekTarget, seekMax, state->seek.seekFlags);
			if (seekResult < 0) {
				// @TODO(final): Log seek error
			} else {
				if (state->seek.seekFlags & AVSEEK_FLAG_BYTE) {
					SetClock(state->externalClock, NAN, 0);
				} else {
					SetClock(state->externalClock, seekTarget / (double)AV_TIME_BASE, 0);
				}
				if (state->audio.stream.isValid) {
					FlushPacketQueue(state->audio.decoder.packetsQueue);
					PushFlushPacket(state->audio.decoder.packetsQueue);

					state->audio.decoder.isEOF = false;
					fplSignalSet(&state->audio.decoder.resumeSignal);
				}
				if (state->video.stream.isValid) {
					FlushPacketQueue(state->video.decoder.packetsQueue);
					PushFlushPacket(state->video.decoder.packetsQueue);

					state->video.decoder.isEOF = false;
					fplSignalSet(&state->video.decoder.resumeSignal);
				}
			}
			state->seek.isRequired = false;
			reader.isEOF = false;
			if (state->isPaused) {
				StepToNextFrame(state);
			}
		}

		// @TODO(final): Handle attached pictures

		// Limit the queue?
		if ((!state->isInfiniteBuffer &&
			(audio.decoder.packetsQueue.size + video.decoder.packetsQueue.size) > MAX_PACKET_QUEUE_SIZE) ||
			(StreamHasEnoughPackets(audio.stream.stream, audio.stream.streamIndex, audio.decoder.packetsQueue) &&
			StreamHasEnoughPackets(video.stream.stream, video.stream.streamIndex, video.decoder.packetsQueue))) {
			skipWait = true;
			fplThreadSleep(10);
			continue;
		}

		//
		// Seek to the beginning when everything is done
		//
		// @TODO(final): Make this configurable
		bool autoExit = true;
		int64_t startTime = AV_NOPTS_VALUE;

		if (!state->isPaused &&
			(!state->audio.stream.isValid || (state->audio.decoder.finishedSerial == state->audio.decoder.packetsQueue.serial && GetFrameQueueRemainingCount(state->audio.decoder.frameQueue) == 0)) &&
			(!state->video.stream.isValid || (state->video.decoder.finishedSerial == state->video.decoder.packetsQueue.serial && GetFrameQueueRemainingCount(state->video.decoder.frameQueue) == 0))) {
			if ((state->loop == -1) || (state->loop > 0)) {
				if (state->loop > 0) {
					--state->loop;
				}
				SeekStream(&state->seek, startTime != AV_NOPTS_VALUE ? startTime : 0, 0, 0);
			} else {
				if (autoExit) {
					break;
				}
			}
		}

		// Read packet
		if (!hasPendingPacket) {
			int res = ffmpeg.av_read_frame(formatCtx, &srcPacket);
			if (res < 0) {
				if ((res == AVERROR_EOF || ffmpeg.avio_feof(formatCtx->pb)) && !reader.isEOF) {
					if (video.stream.isValid) {
						PushNullPacket(video.decoder.packetsQueue, video.stream.streamIndex);
					}
					if (audio.stream.isValid) {
						PushNullPacket(audio.decoder.packetsQueue, audio.stream.streamIndex);
					}
					reader.isEOF = true;
				}
				if (formatCtx->pb != nullptr && formatCtx->pb->error) {
					// @TODO(final): Handle error
					break;
				}

				// Wait for a few milliseconds
				fplThreadSleep(10);
				skipWait = true;
				continue;
			} else {
				hasPendingPacket = true;
				reader.isEOF = false;
			}
		}

		if (hasPendingPacket) {
			// Try to get new packet from the freelist
			PacketList *targetPacket = nullptr;
			if (AquirePacket(reader.packetQueue, targetPacket)) {
				assert(targetPacket != nullptr);

#if PRINT_QUEUE_INFOS
				uint32_t packetIndex = AtomicAddU32(&reader.readPacketCount, 1);
				ConsoleFormatOut("Read packet %lu\n", packetIndex);
#endif

				// Check if packet is in play range, then queue, otherwise discard
				int64_t streamStartTime = formatCtx->streams[srcPacket.stream_index]->start_time;
				int64_t pktTimeStamp = (srcPacket.pts == AV_NOPTS_VALUE) ? srcPacket.dts : srcPacket.pts;
				double timeInSeconds = (double)(pktTimeStamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0)) * av_q2d(formatCtx->streams[srcPacket.stream_index]->time_base);
				bool pktInPlayRange = (!state->settings.duration.isValid) ||
					((timeInSeconds / (double)AV_TIME_BASE) <= ((double)state->settings.duration.value / (double)AV_TIME_BASE));

				if ((videoStream != nullptr) && (srcPacket.stream_index == videoStream->streamIndex) && pktInPlayRange) {
					AddPacketToDecoder(video.decoder, targetPacket, &srcPacket);
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Queued video packet %lu\n", packetIndex);
#endif
				} else if ((audioStream != nullptr) && (srcPacket.stream_index == audioStream->streamIndex) && pktInPlayRange) {
					AddPacketToDecoder(audio.decoder, targetPacket, &srcPacket);
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Queued audio packet %lu\n", packetIndex);
#endif
				} else {
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Dropped packet %lu\n", packetIndex);
#endif
					ffmpeg.av_packet_unref(&srcPacket);
				}
				hasPendingPacket = false;
			}
			skipWait = true;
		}
	}

	fplConsoleOut("Reader thread stopped.\n");
}

static bool OpenStreamComponent(const char *mediaFilePath, const int32_t streamIndex, AVStream *stream, MediaStream &outStream) {
	// Get codec name
	char codecName[5] = {};
	fplMemoryCopy(&stream->codecpar->codec_tag, 4, codecName);

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
			typeName = "";
			assert(!"Unsupported stream type!");
	}

	// Create codec context
	outStream.codecContext = ffmpeg.avcodec_alloc_context3(nullptr);
	if (ffmpeg.avcodec_parameters_to_context(outStream.codecContext, stream->codecpar) < 0) {
		fplConsoleFormatError("Failed getting %s codec context from codec '%s' in media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @NOTE(finaL): Set packet time base to stream time base
	outStream.codecContext->pkt_timebase = stream->time_base;

	// Find decoder
	// @TODO(final): We could force the codec here if we want (avcodec_find_decoder_by_name).
	outStream.codec = ffmpeg.avcodec_find_decoder(stream->codecpar->codec_id);
	if (outStream.codec == nullptr) {
		fplConsoleFormatError("Unsupported %s codec '%s' in media file '%s' found!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// Open codec
	if (ffmpeg.avcodec_open2(outStream.codecContext, outStream.codec, nullptr) < 0) {
		fplConsoleFormatError("Failed opening %s codec '%s' from media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @TODO(final): Why do we need to set the discard flag to default here?
	stream->discard = AVDISCARD_DEFAULT;

	outStream.isValid = true;
	outStream.stream = stream;
	outStream.streamIndex = streamIndex;

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

struct DisplayRect {
	int left;
	int top;
	int right;
	int bottom;
};

static DisplayRect CalculateDisplayRect(const int screenLeft, const int screenTop, const int screenWidth, const int screenHeight, const int pictureWidth, const int pictureHeight, const AVRational pictureSAR) {
	double aspect_ratio;
	if (pictureSAR.num == 0.0) {
		aspect_ratio = 0.0;
	} else {
		aspect_ratio = av_q2d(pictureSAR);
	}
	if (aspect_ratio <= 0.0) {
		aspect_ratio = 1.0;
	}
	aspect_ratio *= (float)pictureWidth / (float)pictureHeight;

	int height = screenHeight;
	int width = lrint(height * aspect_ratio) & ~1;
	if (width > screenWidth) {
		width = screenWidth;
		height = lrint(width / aspect_ratio) & ~1;
	}
	int x = (screenWidth - width) / 2;
	int y = (screenHeight - height) / 2;
	DisplayRect result;
	result.left = screenLeft + x;
	result.top = screenTop + y;
	result.right = result.left + FFMAX(width, 1);
	result.bottom = result.top + FFMAX(height, 1);
	return(result);
}

static void DisplayVideoFrame(PlayerState *state) {
	assert(state != nullptr);
	int readIndex = state->video.decoder.frameQueue.readIndex;
	Frame *vp = PeekFrameQueueLast(state->video.decoder.frameQueue);
	VideoContext &video = state->video;
	bool wasUploaded = false;
	if (!vp->isUploaded) {
		UploadTexture(video, vp->frame);
		vp->isUploaded = true;
		wasUploaded = true;
	}

	// Calculate display rect (Top-Down)
	int w = state->viewport.width;
	int h = state->viewport.height;
	DisplayRect rect = CalculateDisplayRect(0, 0, w, h, vp->width, vp->height, vp->sar);

#if USE_HARDWARE_RENDERING
	Mat4f proj = Mat4f::CreateOrthoRH(0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f);

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);

	float uMin = 0.0f;
	float vMin = 0.0f;
#if USE_GL_RECTANGLE_TEXTURES
	float uMax = (float)vp->width;
	float vMax = (float)vp->height;
#else
	float uMax = 1.0f;
	float vMax = 1.0f;
#endif

	float left = (float)rect.left;
	float right = (float)rect.right;
	float top = (float)rect.bottom;
	float bottom = (float)rect.top;

	float vertexData[4 * 4] = {
		// Top right
		right, top, uMax, vMax,
		// Bottom right
		right, bottom, uMax, vMin,
		// Bottom left
		left, bottom, uMin, vMin,
		// Top left
		left, top, uMin, vMax,
	};

	// Enable vertex array buffer
	glBindVertexArray(state->video.vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->video.vertexBufferId);
	// Update vertex array buffer with new rectangle
	glBufferData(GL_ARRAY_BUFFER, FPL_ARRAYCOUNT(vertexData) * sizeof(float), vertexData, GL_STREAM_DRAW);
	CheckGLError();

	// Setup vertex attributes for vertex shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)(sizeof(float) * 2));
	CheckGLError();

	// Enable textures
	int textureIndices[MAX_TARGET_TEXTURE_COUNT] = {};
	for (uint32_t textureIndex = 0; textureIndex < video.targetTextureCount; ++textureIndex) {
		const Texture &targetTexture = video.targetTextures[textureIndex];
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		glBindTexture(targetTexture.target, targetTexture.id);
		textureIndices[textureIndex] = textureIndex;
	}

	// Enable shader
	const VideoShader *shader = state->video.activeShader;
	glUseProgram(shader->programId);
	glUniformMatrix4fv(shader->uniform_uniProjMat, 1, GL_FALSE, proj.m);
	glUniform1iv(shader->uniform_uniTextures, (GLsizei)MAX_TARGET_TEXTURE_COUNT, textureIndices);
	glUniform1f(shader->uniform_uniTextureOffsetY, vMax);
	glUniform1f(shader->uniform_uniTextureScaleY, -1.0f);
	CheckGLError();

	// Draw quad
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->video.indexBufferId);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CheckGLError();

	// Disable shader
	glUseProgram(0);

	// Disable textures
	for (int textureIndex = (int)video.targetTextureCount - 1; textureIndex >= 0; textureIndex--) {
		const Texture &targetTexture = video.targetTextures[textureIndex];
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		glBindTexture(targetTexture.target, 0);
	}

	// Disable vertex array buffer
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	CheckGLError();

#else
	VideoBackBuffer *backBuffer = GetVideoBackBuffer();
	backBuffer->outputRect = CreateVideoRectFromLTRB(rect.left, rect.top, rect.right, rect.bottom);
	backBuffer->useOutputRect = true;
#endif

	fplVideoFlip();

#if PRINT_FRAME_UPLOAD_INFOS
	ConsoleFormatOut("Displayed frame: %d(%s)\n", readIndex, (wasUploaded ? " (New)" : ""));
#endif
}

inline void UpdateVideoClock(PlayerState *state, const double pts, const int32_t serial) {
	SetClock(state->video.clock, pts, serial);
	SyncClockToSlave(state->externalClock, state->video.clock);
}

inline double GetFrameDuration(const PlayerState *state, const Frame *cur, const Frame *next) {
	if (cur->serial == next->serial) {
		double duration = next->pts - cur->pts;
		if (isnan(duration) || duration <= 0 || duration > state->maxFrameDuration)
			return cur->duration;
		else
			return duration;
	} else {
		return 0.0;
	}
}

static double ComputeVideoDelay(const PlayerState *state, const double delay) {
	double result = delay;
	double diff = 0.0;
	double videoClock = 0.0;
	double masterClock = 0.0;
	double syncThreshold = 0.0;
	if (GetMasterSyncType(state) != AVSyncType::VideoMaster) {
		videoClock = GetClock(state->video.clock);
		masterClock = GetMasterClock(state);
		diff = videoClock - masterClock;
		syncThreshold = FPL_MAX(AV_SYNC_THRESHOLD_MIN, FPL_MIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < state->maxFrameDuration) {
			if (diff <= -syncThreshold) {
				result = FFMAX(0, delay + diff);
			} else if (diff >= syncThreshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
				result = delay + diff;
			} else if (diff >= syncThreshold) {
				result = 2 * delay;
			}
		}
	}

#if PRINT_VIDEO_DELAY
	ConsoleFormatOut("video: delay=%0.3f A-V=%f\n", delay, -diff);
#endif

	return(result);
}

static void VideoRefresh(PlayerState *state, double &remainingTime, int &displayCount) {
	if (!state->isPaused && GetMasterSyncType(state) == AVSyncType::ExternalClock && state->isRealTime) {
		UpdateExternalClockSpeed(state);
	}
	if (state->video.stream.isValid) {
	retry:
		if (GetFrameQueueRemainingCount(state->video.decoder.frameQueue) > 0) {
			// Dequeue the current and the last picture
			Frame *lastvp = PeekFrameQueueLast(state->video.decoder.frameQueue);
			Frame *vp = PeekFrameQueue(state->video.decoder.frameQueue);

			// Serials from frame and packet queue must match
			if (vp->serial != state->video.decoder.packetsQueue.serial) {
				NextReadable(state->video.decoder.frameQueue);
				goto retry;
			}

			// Reset frame timer when serial from current and last frame are different
			if (lastvp->serial != vp->serial) {
				state->frameTimer = ffmpeg.av_gettime_relative() / (double)AV_TIME_BASE;
			}

			// Just display the last shown frame
			if (state->isPaused) {
				goto display;
			}

			// Compute duration and delay
			double lastDuration = GetFrameDuration(state, lastvp, vp);
			double delay = ComputeVideoDelay(state, lastDuration);

			// Compute remaining time if have time left to display the frame
			double time = ffmpeg.av_gettime_relative() / (double)AV_TIME_BASE;
			if (time < state->frameTimer + delay) {
				remainingTime = FPL_MIN(state->frameTimer + delay - time, remainingTime);
				goto display;
			}

			// Accumulate frame timer by the computed delay
			state->frameTimer += delay;

			// Reset frame timer when frametimer is out-of-sync
			if (delay > 0 && time - state->frameTimer > AV_SYNC_THRESHOLD_MAX) {
				state->frameTimer = time;
			}

			// @TODO(final): Why do we need to lock the frame queue here?
			fplMutexLock(&state->video.decoder.frameQueue.lock, UINT32_MAX);
			if (!isnan(vp->pts)) {
				UpdateVideoClock(state, vp->pts, vp->serial);
			}
			fplMutexUnlock(&state->video.decoder.frameQueue.lock);

			// When we got more than one frame we may drop this frame entirely
			if (GetFrameQueueRemainingCount(state->video.decoder.frameQueue) > 1) {
				Frame *nextvp = PeekFrameQueueNext(state->video.decoder.frameQueue);
				double duration = GetFrameDuration(state, vp, nextvp);
				if (!state->step && (state->settings.frameDrop > 0 || (state->settings.frameDrop && GetMasterSyncType(state) != AVSyncType::VideoMaster)) && time > state->frameTimer + duration) {
					state->frame_drops_late++;
					NextReadable(state->video.decoder.frameQueue);
#if PRINT_FRAME_DROPS
					fplConsoleFormatError("Frame drops: %d/%d\n", state->frame_drops_early, state->frame_drops_late);
#endif
					goto retry;
				}
			}

			// Get to next readable frame in the queue and force the refresh of the frame
			NextReadable(state->video.decoder.frameQueue);
			state->forceRefresh = 1;

			if (state->step && !state->isPaused) {
				StreamTogglePause(state);
			}
		}

	display:
		if (!state->settings.isVideoDisabled && state->forceRefresh && state->video.decoder.frameQueue.readIndexShown) {
			DisplayVideoFrame(state);
			displayCount++;
		} else {
			if (state->video.decoder.frameQueue.count < state->video.decoder.frameQueue.capacity) {
				// @TODO(final): This is not great, but a fix to not wait forever in the video decoding thread
				fplSignalSet(&state->video.decoder.frameQueue.signal);
			}
		}
	}
	state->forceRefresh = 0;

#if PRINT_CLOCKS
	double masterClock = GetMasterClock(state);
	double audioClock = GetClock(state->audio.clock);
	double videoClock = GetClock(state->video.clock);
	double extClock = GetClock(state->externalClock);
	ConsoleFormatOut("M: %7.2f, A: %7.2f, V: %7.2f, E: %7.2f\n", masterClock, audioClock, videoClock, extClock);
#endif
}

static int DecodeInterruptCallback(void *opaque) {
	PlayerState *state = (PlayerState *)opaque;
	int result = state->reader.stopRequest;
	return(result);
}

static void ReleaseVideoContext(VideoContext &video) {
#if USE_HARDWARE_RENDERING
	glDeleteProgram(video.basicShader.programId);
	video.basicShader.programId = 0;
	glDeleteBuffers(1, &video.indexBufferId);
	video.indexBufferId = 0;
	glDeleteBuffers(1, &video.vertexBufferId);
	video.vertexBufferId = 0;
#endif

	for (uint32_t textureIndex = 0; textureIndex < video.targetTextureCount; ++textureIndex) {
		if (video.targetTextures[textureIndex].id) {
			DestroyTexture(video.targetTextures[textureIndex]);
		}
	}
	video.targetTextureCount = 0;

	if (video.softwareScaleCtx != nullptr) {
		ffmpeg.sws_freeContext(video.softwareScaleCtx);
	}
	if (video.stream.codecContext != nullptr) {
		ffmpeg.avcodec_free_context(&video.stream.codecContext);
	}
}

static GLuint CompileShader(GLuint type, const char *source, const char *name) {
	GLuint result = glCreateShader(type);
	glShaderSource(result, 1, &source, nullptr);
	glCompileShader(result);
	int compileStatus;
	glGetShaderiv(result, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		int length;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &length);
		char *message = (char *)FPL_STACKALLOCATE(length * sizeof(char));
		glGetShaderInfoLog(result, length, &length, message);
		fplConsoleFormatError("Failed to compile %s shader '%s':\n%s\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), name, message);
		glDeleteShader(result);
		return 0;
	}
	return(result);
}

static GLuint CreateShader(const char *vertexShaderSource, const char *fragmentShaderSource, const char *name) {
	GLuint result = glCreateProgram();
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource, name);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource, name);
	if (vs == 0 || fs == 0) {
		glDeleteProgram(result);
		return 0;
	}
	glAttachShader(result, vs);
	glAttachShader(result, fs);
	glDeleteShader(fs);
	glDeleteShader(vs);
	glLinkProgram(result);

	int linkStatus;
	glGetProgramiv(result, GL_LINK_STATUS, &linkStatus);
	if (GL_LINK_STATUS == GL_FALSE) {
		int length;
		glGetProgramiv(result, GL_INFO_LOG_LENGTH, &length);
		char *message = (char *)FPL_STACKALLOCATE(length * sizeof(char));
		glGetProgramInfoLog(result, length, &length, message);
		fplConsoleFormatError("Failed to link %s shader program:\n%s\n", name, message);
		glDeleteProgram(result);
		return 0;
	}

	glValidateProgram(result);
	return(result);
}

static bool LoadVideoShader(VideoShader &shader, const char *vertexSource, const char *fragSource, const char *name) {
	shader.programId = CreateShader(vertexSource, fragSource, "Basic");
	shader.uniform_uniProjMat = glGetUniformLocation(shader.programId, "uniProjMat");
	shader.uniform_uniTextures = glGetUniformLocation(shader.programId, "uniTextures");
	shader.uniform_uniTextureScaleY = glGetUniformLocation(shader.programId, "uniTextureScaleY");
	shader.uniform_uniTextureOffsetY = glGetUniformLocation(shader.programId, "uniTextureOffsetY");
	return true;
}

static bool InitializeVideo(PlayerState &state, const char *mediaFilePath) {
	VideoContext &video = state.video;
	AVCodecContext *videoCodexCtx = video.stream.codecContext;

	// Init video decoder
	if (!InitDecoder(video.decoder, &state, &state.reader, &video.stream, MAX_VIDEO_FRAME_QUEUE_COUNT, 1)) {
		fplConsoleFormatError("Failed initialize video decoder for media file '%s'!\n", mediaFilePath);
		return false;
	}

	AVPixelFormat targetPixelFormat;
#if USE_HARDWARE_RENDERING
	targetPixelFormat = AVPixelFormat::AV_PIX_FMT_RGBA;
#else
	targetPixelFormat = AVPixelFormat::AV_PIX_FMT_BGRA;
#endif

	// Get software context
	video.softwareScaleCtx = ffmpeg.sws_getContext(
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
	if (video.softwareScaleCtx == nullptr) {
		fplConsoleFormatError("Failed getting software scale context with size (%d x %d) for file '%s'!\n", videoCodexCtx->width, videoCodexCtx->height, mediaFilePath);
		return false;
	}

#if USE_HARDWARE_RENDERING && USE_HARDWARE_IMAGE_FORMAT_DECODING
	switch (videoCodexCtx->pix_fmt) {
		case AVPixelFormat::AV_PIX_FMT_YUV420P:
		{
			state.video.activeShader = &state.video.yuv420pShader;
			state.video.targetTextureCount = 3;
			if (!InitTexture(state.video.targetTextures[0], videoCodexCtx->width, videoCodexCtx->height, 8)) {
				return false;
			}
			if (!InitTexture(state.video.targetTextures[1], videoCodexCtx->width / 2, videoCodexCtx->height / 2, 8)) {
				return false;
			}
			if (!InitTexture(state.video.targetTextures[2], videoCodexCtx->width / 2, videoCodexCtx->height / 2, 8)) {
				return false;
			}
		} break;
		default:
		{
			state.video.activeShader = &state.video.basicShader;
			state.video.targetTextureCount = 1;
			if (!InitTexture(state.video.targetTextures[0], videoCodexCtx->width, videoCodexCtx->height, 32)) {
				return false;
			}
		} break;
	}
#else
#	if USE_HARDWARE_RENDERING
	state.video.activeShader = &state.video.basicShader;
#	endif
	state.video.targetTextureCount = 1;
	if (!InitTexture(state.video.targetTextures[0], videoCodexCtx->width, videoCodexCtx->height, 32)) {
		return false;
	}
#endif

#if USE_HARDWARE_RENDERING
	glGenVertexArrays(1, &state.video.vao);
	glBindVertexArray(state.video.vao);
	CheckGLError();

	glGenBuffers(1, &state.video.vertexBufferId);
	glGenBuffers(1, &state.video.indexBufferId);
	CheckGLError();

	glBindBuffer(GL_ARRAY_BUFFER, state.video.vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 4, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CheckGLError();

	// Topright, Bottomright, Bottomleft, Topleft
	uint32_t indices[6] = {
		0, 1, 2,
		2, 3, 0,
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.video.indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, FPL_ARRAYCOUNT(indices) * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CheckGLError();

	glBindVertexArray(0);

	if (!LoadVideoShader(state.video.basicShader, BasicShaderSource::Vertex, BasicShaderSource::Fragment, BasicShaderSource::Name)) {
		return false;
	}
	if (!LoadVideoShader(state.video.yuv420pShader, YUV420PShaderSource::Vertex, YUV420PShaderSource::Fragment, YUV420PShaderSource::Name)) {
		return false;
	}

	CheckGLError();
#endif

	state.frameTimer = 0.0;
	state.frameLastPTS = 0.0;
	state.frameLastDelay = 40e-3;

	return true;
}

static void ReleaseAudio(AudioContext &audio) {
	if (audio.conversionAudioBuffer != nullptr) {
		fplMemoryAlignedFree(audio.conversionAudioBuffer);
		audio.conversionAudioBuffer = nullptr;
	}
	if (audio.softwareResampleCtx != nullptr) {
		ffmpeg.swr_free(&audio.softwareResampleCtx);
	}
	if (audio.stream.codecContext != nullptr) {
		ffmpeg.avcodec_free_context(&audio.stream.codecContext);
	}
}

static bool InitializeAudio(PlayerState &state, const char *mediaFilePath, const fplAudioDeviceFormat &nativeAudioFormat) {
	AudioContext &audio = state.audio;
	AVCodecContext *audioCodexCtx = audio.stream.codecContext;

	// Init audio decoder
	if (!InitDecoder(audio.decoder, &state, &state.reader, &audio.stream, MAX_AUDIO_FRAME_QUEUE_COUNT, 1)) {
		fplConsoleFormatError("Failed initialize audio decoder for media file '%s'!\n", mediaFilePath);
		return false;
	}

	if ((state.formatCtx->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !state.formatCtx->iformat->read_seek) {
		audio.decoder.start_pts = audio.stream.stream->start_time;
		audio.decoder.start_pts_tb = audio.stream.stream->time_base;
	}

	AVSampleFormat targetSampleFormat = MapAudioFormatType(nativeAudioFormat.type);
	// @TODO(final): Map target audio channels to channel layout
	int targetChannelCount = nativeAudioFormat.channels;
	uint64_t targetChannelLayout = AV_CH_LAYOUT_STEREO;
	assert(targetChannelCount == 2);
	int targetSampleRate = nativeAudioFormat.sampleRate;
	audio.audioTarget = {};
	audio.audioTarget.periods = nativeAudioFormat.periods;
	audio.audioTarget.channels = targetChannelCount;
	audio.audioTarget.sampleRate = targetSampleRate;
	audio.audioTarget.type = nativeAudioFormat.type;
	audio.audioTarget.bufferSizeInFrames = ffmpeg.av_samples_get_buffer_size(nullptr, audio.audioTarget.channels, 1, targetSampleFormat, 1);
	audio.audioTarget.bufferSizeInBytes = ffmpeg.av_samples_get_buffer_size(nullptr, audio.audioTarget.channels, audio.audioTarget.sampleRate, targetSampleFormat, 1);

	AVSampleFormat inputSampleFormat = audioCodexCtx->sample_fmt;
	int inputChannelCount = audioCodexCtx->channels;
	// @TODO(final): Map input audio channels to channel layout
	uint64_t inputChannelLayout = AV_CH_LAYOUT_STEREO;
	int inputSampleRate = audioCodexCtx->sample_rate;
	assert(inputChannelCount == 2);
	audio.audioSource = {};
	audio.audioSource.channels = inputChannelCount;
	audio.audioSource.sampleRate = inputSampleRate;
	audio.audioSource.type = MapAVSampleFormat(inputSampleFormat);
	audio.audioSource.periods = nativeAudioFormat.periods;
	audio.audioSource.bufferSizeInBytes = ffmpeg.av_samples_get_buffer_size(nullptr, inputChannelCount, inputSampleRate, inputSampleFormat, 1);
	audio.audioSource.bufferSizeInFrames = ffmpeg.av_samples_get_buffer_size(nullptr, inputChannelCount, 1, inputSampleFormat, 1);

	// Compute AVSync audio threshold
	audio.audioDiffAbgCoef = exp(log(0.01) / AV_AUDIO_DIFF_AVG_NB);
	audio.audioDiffAvgCount = 0;
	audio.audioDiffThreshold = nativeAudioFormat.bufferSizeInBytes / (double)audio.audioTarget.bufferSizeInBytes;

	// Create software resample context and initialize
	audio.softwareResampleCtx = ffmpeg.swr_alloc_set_opts(nullptr,
														  targetChannelLayout,
														  targetSampleFormat,
														  targetSampleRate,
														  inputChannelLayout,
														  inputSampleFormat,
														  inputSampleRate,
														  0,
														  nullptr);
	ffmpeg.swr_init(audio.softwareResampleCtx);

	// Allocate conversion buffer in native format, this must be big enough to hold one AVFrame worth of data.
	int lineSize;
	audio.maxConversionAudioBufferSize = ffmpeg.av_samples_get_buffer_size(&lineSize, targetChannelCount, targetSampleRate, targetSampleFormat, 1);
	audio.maxConversionAudioFrameCount = audio.maxConversionAudioBufferSize / fplGetAudioSampleSizeInBytes(nativeAudioFormat.type) / targetChannelCount;
	audio.conversionAudioBuffer = (uint8_t *)fplMemoryAlignedAllocate(audio.maxConversionAudioBufferSize, 16);
	audio.conversionAudioFrameIndex = 0;
	audio.conversionAudioFramesRemaining = 0;

	return true;
}

static void ReleaseMedia(PlayerState &state) {
	DestroyDecoder(state.audio.decoder);
	ReleaseAudio(state.audio);
	DestroyDecoder(state.video.decoder);
	ReleaseVideoContext(state.video);
	DestroyReader(state.reader);
	if (state.formatCtx != nullptr) {
		ffmpeg.avformat_close_input(&state.formatCtx);
	}
}

static bool LoadMedia(PlayerState &state, const char *mediaFilePath, const fplAudioDeviceFormat &nativeAudioFormat) {
	// @TODO(final): Custom IO!

	// Open media file
	if (ffmpeg.avformat_open_input(&state.formatCtx, mediaFilePath, nullptr, nullptr) != 0) {
		fplConsoleFormatError("Failed opening media file '%s'!\n", mediaFilePath);
		goto release;
	}

	state.formatCtx->interrupt_callback.callback = DecodeInterruptCallback;
	state.formatCtx->interrupt_callback.opaque = &state;

	// Retrieve stream information
	if (ffmpeg.avformat_find_stream_info(state.formatCtx, nullptr) < 0) {
		fplConsoleFormatError("Failed getting stream informations for media file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Dump information about file into standard error
	ffmpeg.av_dump_format(state.formatCtx, 0, mediaFilePath, 0);

	// Dont limit the queues when we are playing realtime based media, like internet streams, etc.
	state.isRealTime = IsRealTime(state.formatCtx);
	if (!state.isInfiniteBuffer && state.isRealTime) {
		state.isInfiniteBuffer = true;
	}

	// Find the first streams
	state.video.stream.streamIndex = -1;
	state.audio.stream.streamIndex = -1;
	for (uint32_t streamIndex = 0; streamIndex < state.formatCtx->nb_streams; streamIndex++) {
		AVStream *stream = state.formatCtx->streams[streamIndex];
		switch (stream->codecpar->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
			{
				if (state.video.stream.streamIndex == -1 && !state.settings.isVideoDisabled) {
					OpenStreamComponent(mediaFilePath, streamIndex, stream, state.video.stream);
				}
			} break;
			case AVMEDIA_TYPE_AUDIO:
			{
				if (state.audio.stream.streamIndex == -1 && !state.settings.isAudioDisabled) {
					OpenStreamComponent(mediaFilePath, streamIndex, stream, state.audio.stream);
				}
			} break;
		}
	}

	// No streams found
	if ((!state.video.stream.isValid) && (!state.audio.stream.isValid)) {
		fplConsoleFormatError("No video or audio stream in media file '%s' found!\n", mediaFilePath);
		goto release;
	}

	// We need to initialize the reader first before we allocate stream specific stuff
	if (!InitReader(state.reader)) {
		fplConsoleFormatError("Failed initializing reader file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Allocate audio related resources
	if (state.audio.stream.isValid) {
		if (!InitializeAudio(state, mediaFilePath, nativeAudioFormat)) {
			goto release;
		}
	}

	// Allocate video related resources
	if (state.video.stream.isValid) {
		if (!InitializeVideo(state, mediaFilePath)) {
			goto release;
		}
	}

	// Init timings
	state.maxFrameDuration = (state.formatCtx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	InitClock(state.video.clock, &state.video.decoder.packetsQueue.serial);
	InitClock(state.audio.clock, &state.audio.decoder.packetsQueue.serial);
	InitClock(state.externalClock, &state.externalClock.serial);
	state.audio.audioClockSerial = -1;

	return true;

release:
	ReleaseMedia(state);
	return false;
}

int main(int argc, char **argv) {
	int result = 0;

	if (argc < 2) {
		fplConsoleError("Media file argument missing!");
		return -1;
	}

	const char *mediaFilePath = argv[1];

	fplSettings settings;
	fplSetDefaultSettings(&settings);
	fplCopyAnsiString("FPL FFmpeg Demo", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
#if USE_HARDWARE_RENDERING
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
#else
	settings.video.driver = fplVideoDriverType_Software;
#endif
	settings.video.isAutoSize = false;
	settings.video.isVSync = false;

	if (!fplPlatformInit(fplInitFlags_All, &settings)) {
		return -1;
	}

#if USE_HARDWARE_RENDERING
	if (!fdyngl::LoadOpenGL()) {
		fplPlatformRelease();
		return -1;
	}
#endif

	fplAudioDeviceFormat nativeAudioFormat = fplGetAudioHardwareFormat();

	PlayerState state = {};

	//
	// Load ffmpeg libraries
	//
	if (!LoadFFMPEG(ffmpeg)) {
		goto release;
	}

	// Register all formats and codecs
	ffmpeg.av_register_all();

	// Init flush packet
	ffmpeg.av_init_packet(&globalFlushPacket);
	globalFlushPacket.data = (uint8_t *)&globalFlushPacket;

	//
	// Settings
	//
	InitPlayerSettings(state.settings);
	state.isInfiniteBuffer = state.settings.isInfiniteBuffer;
	state.loop = state.settings.isLoop ? 1 : 0;

	FPL_ASSERT(fplGetWindowArea(&state.viewport));

	// Load media
	if (!LoadMedia(state, mediaFilePath, nativeAudioFormat)) {
		goto release;
	}

	// Start decoder and reader
	if (state.video.stream.isValid) {
		StartDecoder(state.video.decoder, VideoDecodingThreadProc);
	}
	if (state.audio.stream.isValid) {
		StartDecoder(state.audio.decoder, AudioDecodingThreadProc);
	}
	StartReader(state.reader, PacketReadThreadProc, &state);

	// Start playing audio
	if (state.audio.stream.isValid) {
		fplSetAudioClientReadCallback(AudioReadCallback, &state.audio);
		fplPlayAudio();
	}

	//
	// App loop
	//
	double lastTime = fplGetTimeInSeconds();
	double remainingTime = 0.0;
	double lastRefreshTime = fplGetTimeInSeconds();
	int refreshCount = 0;
	while (fplWindowUpdate()) {
		//
		// Handle events
		//
		fplEvent ev = {};
		while (fplPollEvent(&ev)) {
			switch (ev.type) {
				case fplEventType_Keyboard:
				{
					if (ev.keyboard.type == fplKeyboardEventType_KeyUp) {
						switch (ev.keyboard.mappedKey) {
							case fplKey_Space:
							{
								TogglePause(&state);
							} break;
							case fplKey_F:
							{
								ToggleFullscreen(&state);
							} break;
						}
					}
				} break;
				case fplEventType_Window:
				{
					if (ev.window.type == fplWindowEventType_Resized) {
						state.viewport.width = ev.window.width;
						state.viewport.height = ev.window.height;
						state.forceRefresh = 1;
					}
				} break;
			}
		}

		//
		// Refresh video
		//
		if (remainingTime > 0.0) {
			uint32_t msToSleep = (uint32_t)(remainingTime * 1000.0);
			fplThreadSleep(msToSleep);
		}
		remainingTime = DEFAULT_REFRESH_RATE;
		if (!state.isPaused || state.forceRefresh) {
			VideoRefresh(&state, remainingTime, refreshCount);
#if PRINT_VIDEO_REFRESH
			ConsoleFormatOut("Video refresh: %d\n", ++refreshCount);
#endif
		}

		// Update time
		double now = fplGetTimeInSeconds();
		double refreshDelta = now - lastRefreshTime;
		if (refreshDelta >= 1.0) {
			lastRefreshTime = now;
#if PRINT_FPS
			ConsoleFormatOut("FPS: %d\n", refreshCount);
#endif
			refreshCount = 0;
		}
		double delta = now - lastTime;
		lastTime = now;
#if PRINT_MEMORY_STATS
		PrintMemStats();
#endif
	}


release:
	// Stop audio
	if (state.audio.stream.isValid) {
		fplStopAudio();
	}

	// Stop reader and decoders
	StopReader(state.reader);
	if (state.video.stream.isValid) {
		StopDecoder(state.video.decoder);
	}
	if (state.audio.stream.isValid) {
		StopDecoder(state.audio.decoder);
	}

	// Release media
	ReleaseMedia(state);

	//
	// Release FFMPEG
	//
	ReleaseFFMPEG(ffmpeg);

	// Release platform
#if USE_HARDWARE_RENDERING
	fdyngl::UnloadOpenGL();
#endif
	fplPlatformRelease();

	return(0);
}