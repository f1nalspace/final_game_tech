/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImageViewer

Version:
	v0.6.0 (version.h)

Description:
	Very simple opengl based image viewer.
	Loads up pictures in multiple threads using a lock-free MPMC queue.
	Texture Allocate/Release is done in the main thread.
	It supports several image filters, such as Bilinear, Bicubic, Lanczos etc.

Requirements:
	- C++ Compiler :-( Just because to support R"()"
	- Final Platform Layer
	- Final Dynamic OpenGL
	- OpenGL 3.3 core profile
	- STB_image (the default loader, others are optional)

Author:
	Torsten Spaete

Changelog:
	## v0.6.0
	- New: --render-to=<file.pam> --window=<W>x<H> renders one picture offscreen into a framebuffer of exactly that size, writes it as PAM and exits
	- New: --zoom=fit|100|<percent> sets the start zoom (fit also upscales small pictures)
	- New: --window=<W>x<H> sets the initial window size, --no-preview hides the preview strip
	- New: --selftest checks the view math and the resample tap ranges without window and OpenGL
	- New: Correct downscaling: the GPU resample pipeline (resamplepipeline.h) scales in two separable passes in linear light with premultiplied alpha and widens the kernel by the reduction factor, the same math as ImageMagick -resize
	- New: Box and Mitchell kernels, separate filters for downscaling (default Mitchell) and upscaling (default Catmull-Rom), T and Shift+T step the filter of the direction in effect, the window title shows it
	- New: --down-filter=<key>, --up-filter=<key>, -f also takes a key and sets both directions (numbers keep their old meaning)
	- New: Background behind transparent pictures: checker board, black or gray, B or --background=checker|black|gray
	- New: The scaled picture is cached and only computed again when picture, filter or placement change, the GPU time of both passes is logged
	- Changed: The preview strip scales its pictures with the resample pipeline too, computed once per picture
	- New: Test image generator and scaling test runner in tests/
	- New: Pictures are read through exchangeable loaders (imageloader.h), each with a fixed 128-bit id: stb_image (default: JPEG, PNG, BMP), PNM/PAM (PGM, PPM, PAM) and a reference BMP loader behind stb_image
	- New: A loader is picked by the first bytes of the file (the signature beats the extension), a pinned loader goes first, Unsupported or Corrupt falls back to the next loader
	- New: --loader=<id|name>, --loader-for=<extension>:<id|name>, --loader-order=<id|name>,..., --no-loader-fallback, --list-loaders and --decode-all=<folder>
	- New: L reads the active picture again with the next loader that recognizes it, the window title shows the loader
	- New: The EXIF orientation of JPEG files is applied to the picture, its scaling and the preview strip, without copying pixels
	- Changed: The folder scan takes every extension of every loader
	- Changed: A load job that meets a cancel is dropped, the reload queues everything it needs again
	- Fixed: Two load threads could take the same picture slot
	- Changed: Requires an OpenGL 3.3 core profile, the legacy OpenGL path is removed and a missing context is reported on the console and in the log
	- Changed: Pictures are GL_TEXTURE_2D textures read with texelFetch, GL_TEXTURE_RECTANGLE and the 16x multisampling are gone
	- Changed: Drawing uses viewport pixels with y pointing down, the picture is placed at whole pixels by viewtransform.h
	- Changed: A frame is only drawn when something changed, otherwise the main loop sleeps
	- Changed: The preview strip keeps the aspect ratio of the pictures
	- Removed: Unfinished mipmap code and stb_image_resize
	- Fixed: Every filter except Nearest was shifted by half a texel, so 100% was blurred and scaled pictures were asymmetric
	- Fixed: Filters clamped at the picture edge, now taps outside the picture are left out and the weights renormalized
	- Fixed: Lanczos3 made transparent pixels opaque
	- Fixed: Dropping another folder onto the window leaked the textures of all loaded pictures
	- Fixed: The next picture was drawn outside of the window in every frame
	- Fixed: -p and -f parameters were never evaluated
	- Fixed: Unknown or malformed parameters are reported instead of silently ignored
	- Fixed: Preload count is rounded up to an even count before it is used and clamped to the view picture capacity
	- Fixed: Start index was not reset when a dropped file was not found in its folder
	- Fixed: The log kept a pointer to a temporary path buffer, so log lines could end up in a garbage named file in the working directory
	- Fixed: Load threads waited on their condition variable without holding its mutex
	- Fixed: Log timestamps showed the previous month, load threads shared one format buffer and platform log messages were used as format strings

	## v0.5.6
	- Changed multi sample count to 16, to improve quality for downscaled pictures
	- Changed default filter to bicubic triangular
	- Added -f parameter to control filter type
	- size_t for thread and preload count

	## v0.5.5
	- Reflect api changes in FPL 0.9.4
	- Fixed broken legacy opengl rendering

	## v0.5.4
	- Reflect api changes in FPL 0.9.3

	## v0.5.3
	- Reflect api changes in FPL 0.9.2
	- Preview is enabled by default

	## v0.5.2
	- Correction for api change in fplPlatformInit

	## v0.5.1
	- Changed: Moved progressbar to the top and made it smaller
	- Fixed: 16x16 icon was always loading 32x32

	## 2018-07-12 (v0.5)
	- Created icon
	- Created resource manifest (Win32)
	- Created version info
	- Load window icons
	- Changed default view picture capacity
	- Created inno setup script

	## 2018-07-11
	- Introduce view flags to control how a picture is displayed
	- Prepare to draw frames

	## 2018-07-10
	- Fixed crash when you trigger too many loading requests
	- Detect opengl version

	## 2018-07-09
	- Fixes for Linux/POSIX

	## 2018-07-08
	- Extreme refactoring to support modern OpenGL
	- Implemented custom GLSL filters to try-to-fix quality issues

	## 2018-07-03
	- Prepare for modern opengl
	- Use stbi_load_from_callbacks instead of stbi_load_from_file
	- Compute and show loading progress based on file position

	## 2018-06-30
	- Changed picture file to use a full path
	- Support recursively loading pictures
	- Parameter parsing and applying
	- Queue/View picture count is now based on thread count and rounded up to power of two

	## 2018-06-29
	- Changed to use new keyboard/mouse button state
	- No asserts for important functions anymore

	## 2018-06-28
	- Fixed crash for loading some weird image format constellations
	- Fixed stability problems
	- Detect core/queue count
	- Discard textures on the left/right side when the fileIndex is out of bounds

	## 2018-06-26
	- Multithreaded MPMC picture load

	## 2018-06-23
	- Initial version

Todo:
	- Text rendering display current file (Index/Count)
	- Fade in/out
	- Diashow

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define IMAGE_LOADER_IMPLEMENTATION
#include "imageloader.h"
#define IMAGE_LOADER_STB_IMPLEMENTATION
#include "imageloader_stb.h"
#define IMAGE_LOADER_PNM_IMPLEMENTATION
#include "imageloader_pnm.h"
#define IMAGE_LOADER_BMP_IMPLEMENTATION
#include "imageloader_bmp.h"

#include <string.h>

#include <final_math.h>

#include "shadersources.h"
#include "imageresources.h"
#include "version.h"

#define FLOG_IMPLEMENTATION
#include "logging.h"

#define VIEW_TRANSFORM_IMPLEMENTATION
#include "viewtransform.h"

#define RESAMPLE_PIPELINE_IMPLEMENTATION
#include "resamplepipeline.h"

#include "selftest.h"

char ToLowerCase(char ch) {
	if (ch >= 'A' && ch <= 'Z') {
		ch = 'a' + (ch - 'A');
	}
	return ch;
}

static int CompareStringIgnoreCase(const char* a, const char* b) {
	while (true) {
		if (!*a && !*b) {
			break;
		} else if (!*a || !*b) {
			return -1;
		}
		char ca = ToLowerCase(*a);
		char cb = ToLowerCase(*b);
		if (ca < cb || ca > cb) {
			return (int)ca - (int)cb;
		}
		++a;
		++b;
	}
	return(0);
}

typedef struct PictureFile {
	char filePath[FPL_MAX_PATH_LENGTH];
	// Loader entry the L key chose for this file, -1 lets the registry select
	int32_t forcedLoaderEntry;
} PictureFile;

typedef enum LoadedPictureStateType {
	LoadedPictureState_Error = -1,
	LoadedPictureState_Unloaded = 0,
	LoadedPictureState_LoadingData,
	LoadedPictureState_ToUpload,
	LoadedPictureState_Discard,
	LoadedPictureState_Ready,
} LoadedPictureStateType;

typedef int32_t LoadedPictureState;

// Always RGBA8, top row first, as stored (before the orientation)
typedef struct ImageData {
	// Decoded pixels until the upload, owned by the loader in loaderEntry
	ImagePixels pixels;
	int32_t loaderEntry;
	uint32_t width;
	uint32_t height;
	GLuint textureId;
} ImageData;

typedef struct ViewPicture {
	char filePath[FPL_MAX_PATH_LENGTH];
	ImageData image;
	PictureInfo info;
	// Scaled into its preview block, main thread only
	ResampleResult thumbnail;
	// Changes with every upload, the texture name alone may be reused by OpenGL
	uint64_t uploadSerial;
	float progress;
	size_t fileIndex;
	volatile LoadedPictureState state;
} ViewPicture;

typedef struct LoadPictureContext {
	ViewPicture* viewPic;
	volatile bool canceled;
} LoadPictureContext;

typedef struct PictureLoadThread {
	LoadPictureContext context;
	fplMutexHandle mutex;
	fplConditionVariable condition;
	struct ViewerState* state;
	fplThreadHandle* thread;
	volatile bool shutdown;
} PictureLoadThread;

#define MAX_LOAD_THREAD_COUNT 64
#define MAX_VIEW_PICTURE_COUNT MAX_LOAD_THREAD_COUNT * 4
#define MAX_LOAD_QUEUE_COUNT MAX_VIEW_PICTURE_COUNT * 2
#define PAGE_INCREMENT_COUNT 10
// Longest sleep of an idle load thread before it looks into the queue again, a new job wakes it up earlier
#define LOAD_THREAD_POLL_MILLISECONDS 50

typedef struct LoadQueueValue {
	int fileIndex;
	int pictureIndex;
} LoadQueueValue;

typedef struct LoadQueueEntry {
	LoadQueueValue value;
	volatile size_t seq;
} LoadQueueEntry;

// Queue based on: https://github.com/mstump/queues/blob/master/include/mpmc-bounded-queue.hpp
#if defined(FPL_CPU_64BIT)
#	define CACHE_LINE_SIZE 64
#else
#	define CACHE_LINE_SIZE 32
#endif
typedef char CacheLinePad[CACHE_LINE_SIZE];
typedef struct LoadQueue {
	CacheLinePad pad0;
	size_t size;
	size_t mask;
	LoadQueueEntry buffer[MAX_LOAD_QUEUE_COUNT];
	CacheLinePad pad1;
	volatile size_t headSeq;
	CacheLinePad pad2;
	volatile size_t tailSeq;
	CacheLinePad pad3;
	volatile int shutdown;
	CacheLinePad pad4;
} LoadQueue;

typedef struct ViewerParameters {
	const char* path;
	const char* renderToFilePath;
	size_t threadCount;
	size_t preloadCount;
	uint32_t windowWidth;
	uint32_t windowHeight;
	float zoomScale;
	ViewZoomMode zoomMode;
	ResampleKernel downKernel;
	ResampleKernel upKernel;
	ResampleBackground background;
	// Loader selection, resolved against the registry after parsing
	const char* loaderValue;
	const char* loaderOrderValue;
	const char* loaderForValues[IMAGE_LOADER_MAX_EXTENSION_PINS];
	uint32_t loaderForCount;
	const char* decodeAllPath;
	bool isLoaderFallbackDisabled;
	bool listLoaders;
	bool recursive;
	bool preview;
	bool runSelfTest;
} ViewerParameters;

// Exit codes of --render-to
typedef enum RenderToFileResult {
	RenderToFileResult_Success = 0,
	RenderToFileResult_InvalidParameters = 1,
	RenderToFileResult_NoPicture = 2,
	RenderToFileResult_LoadFailed = 3,
	RenderToFileResult_Timeout = 4,
	RenderToFileResult_GraphicsFailed = 5,
	RenderToFileResult_WriteFailed = 6,
	RenderToFileResult_Canceled = 7,
} RenderToFileResult;

// Longest time --render-to waits for the picture to be loaded and uploaded
#define RENDER_TO_FILE_TIMEOUT_MILLISECONDS 60000
// Sleep between two frames while --render-to waits for the picture
#define RENDER_TO_FILE_POLL_MILLISECONDS 1
// Size of the (never shown) window for --render-to, the picture is rendered into an offscreen framebuffer of --window size
#define RENDER_TO_FILE_WINDOW_SIZE 256
// Number of pictures preloaded on both sides of the active picture, when not set by -p
#define DEFAULT_PRELOAD_COUNT 16
// Sleep of the main loop when nothing needs to be drawn
#define IDLE_SLEEP_MILLISECONDS 5
// Oldest OpenGL version the viewer runs on, always as core profile
#define REQUIRED_OPENGL_MAJOR_VERSION 3
#define REQUIRED_OPENGL_MINOR_VERSION 3

// Filters of -f=<number> in the numbering before the Box and Mitchell kernels existed
static const ResampleKernel LegacyFilterNumberKernels[] = {
	ResampleKernel_Nearest,
	ResampleKernel_Triangle,
	ResampleKernel_Triangular,
	ResampleKernel_Bell,
	ResampleKernel_BSpline,
	ResampleKernel_CatmullRom,
	ResampleKernel_Lanczos3,
};

// Downscaling default decided with the comparison crops, the upscaling default stays a working hypothesis until iteration 5 (plan section 2.2)
#define DEFAULT_DOWN_KERNEL ResampleKernel_Mitchell
#define DEFAULT_UP_KERNEL ResampleKernel_CatmullRom
#define DEFAULT_BACKGROUND ResampleBackground_Checker

typedef struct ColorProgram {
	GLuint programId;
	GLint locationViewportSize;
	GLint locationRect;
	GLint locationColor;
} ColorProgram;

typedef enum PictureRequestType {
	PictureRequestType_None = 0,
	PictureRequestType_Change,
	PictureRequestType_Force,
} PictureRequestType;

typedef struct SupportedFeatures {
	bool srgbFrameBuffer;
} SupportedFeatures;

typedef struct ViewerState {
	char rootPath[FPL_MAX_PATH_LENGTH];
	SupportedFeatures features;
	PictureFile* pictureFiles;
	size_t pictureFileCapacity;
	size_t pictureFileCount;
	size_t folderCount;
	int activeFileIndex;

	ViewPicture viewPictures[MAX_VIEW_PICTURE_COUNT];
	size_t viewPicturesCapacity;
	int viewPictureIndex;
	bool doPictureReload;

	PictureLoadThread loadThreads[MAX_LOAD_THREAD_COUNT];
	size_t loadThreadCount;

	ViewerParameters params;
	ViewState view;

	LoadQueue loadQueue;
	size_t loadQueueCapacity;

	// Rectangles are generated from gl_VertexID, core profile still needs a bound vertex array
	GLuint vertexArray;
	ColorProgram colorProgram;

	ResamplePipeline pipeline;
	// The active picture scaled into the viewport
	ResampleResult viewResult;
	ResampleKernel downKernel;
	ResampleKernel upKernel;
	ResampleBackground background;
	uint64_t nextUploadSerial;

	// Every picture is read through these loaders, stb_image first
	ImageLoaderRegistry loaderRegistry;

	// Size of the window client area, updated every frame
	ViewSize viewportSize;
	char windowTitle[FPL_MAX_BUFFER_LENGTH];
} ViewerState;

static void InitQueue(LoadQueue* queue, const size_t queueCount) {
	fplClearStruct(queue);
	queue->size = queueCount;
	queue->mask = queue->size - 1;
	queue->headSeq = queue->tailSeq = 0;
	for (size_t i = 0; i < queue->size; ++i) {
		queue->buffer[i].seq = i;
	}
}

static void ShutdownQueue(LoadQueue* queue) {
	queue->shutdown = 1;
}

static bool TryQueueEnqueue(volatile LoadQueue* queue, const volatile LoadQueueValue value) {
	size_t headSeq = fplAtomicLoadSize(&queue->headSeq);
	while (!queue->shutdown) {
		size_t index = headSeq & queue->mask;
		volatile LoadQueueEntry* entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)headSeq;
		if (dif == 0) {
			if (fplAtomicIsCompareAndSwapSize(&queue->headSeq, headSeq, headSeq + 1)) {
				fplMemoryCopy((const void*)&value, sizeof(value), (void*)&entry->value);
				fplAtomicStoreSize(&entry->seq, headSeq + 1);
				return(true);
			}
		} else if (dif < 0) {
			return(false);
		} else {
			headSeq = fplAtomicLoadSize(&queue->headSeq);
		}
	}
	return(false);
}

static bool TryQueueDequeue(volatile LoadQueue* queue, volatile LoadQueueValue* value) {
	size_t tailSeq = fplAtomicLoadSize(&queue->tailSeq);
	while (!queue->shutdown) {
		size_t index = tailSeq & queue->mask;
		volatile LoadQueueEntry* entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)(tailSeq + 1);
		if (dif == 0) {
			if (fplAtomicIsCompareAndSwapSize(&queue->tailSeq, tailSeq, tailSeq + 1)) {
				fplMemoryCopy((const void*)&entry->value, sizeof(*value), (void*)value);
				fplAtomicStoreSize(&entry->seq, tailSeq + queue->mask + 1);
				return(true);
			}
		} else if (dif < 0) {
			return(false);
		} else {
			tailSeq = fplAtomicLoadSize(&queue->tailSeq);
		}
	}
	return(false);
}

// Every extension any registered loader lists, so a new loader brings its formats into the folder scan
static bool IsPictureFile(const ViewerState* state, const char* filePath) {
	const char* extension = fplExtractFileExtension(filePath);
	bool result = extension != fpl_null && ImageLoaderRegistryIsKnownExtension(&state->loaderRegistry, extension);
	return(result);
}

static void ClearPictureFiles(ViewerState* state) {
	if (state->pictureFiles != fpl_null) {
		free(state->pictureFiles);
		state->pictureFiles = fpl_null;
	}
	state->pictureFileCount = 0;
	state->pictureFileCapacity = 0;
	state->rootPath[0] = 0;
	state->folderCount = 0;
}

static void AddPictureFile(ViewerState* state, const char* filePath) {
	fplAssert(state->pictureFileCount <= state->pictureFileCapacity);
	if (state->pictureFileCapacity == 0) {
		state->pictureFileCapacity = 1;
		state->pictureFiles = (PictureFile*)malloc(sizeof(PictureFile) * state->pictureFileCapacity);
	} else if (state->pictureFileCount == state->pictureFileCapacity) {
		state->pictureFileCapacity *= 2;
		state->pictureFiles = (PictureFile*)realloc(state->pictureFiles, sizeof(PictureFile) * state->pictureFileCapacity);
	}
	PictureFile* pictureFile = &state->pictureFiles[state->pictureFileCount++];
	fplCopyString(filePath, pictureFile->filePath, fplArrayCount(pictureFile->filePath));
	pictureFile->forcedLoaderEntry = -1;
}

static void AddPicturesFromPath(ViewerState* state, const char* path, const bool recursive) {
	fplFileEntry entry;
	size_t addedPics = 0;
	for (bool hasEntry = fplDirectoryListBegin(path, "*", &entry); hasEntry; hasEntry = fplDirectoryListNext(&entry)) {
		if (!hasEntry) {
			break;
		}
		char fullPath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(fullPath, fplArrayCount(fullPath), 2, path, entry.name);
		if (entry.type == fplFileEntryType_File) {
			if (IsPictureFile(state, fullPath)) {
				AddPictureFile(state, fullPath);
				++addedPics;
			}
		} else if (recursive && entry.type == fplFileEntryType_Directory) {
			AddPicturesFromPath(state, fullPath, true);
		}
	}
	if (addedPics > 0) {
		++state->folderCount;
	}
}

static void ReleaseTexture(GLuint* target) {
	fplAssert(*target > 0);
	glDeleteTextures(1, target);
	*target = 0;
}

// RGBA8 picture as GL_TEXTURE_2D without mipmaps, sRGB when the framebuffer does the sRGB encoding, returns 0 on failure (e.g. larger than GL_MAX_TEXTURE_SIZE)
static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const uint32_t stride, const void* data, const bool supportsSRGB) {
	const uint32_t bytesPerPixel = 4;
	GLenum internalFormat = supportsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;

	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Rows may be longer than the pixels need
	GLint rowLength = (GLint)(stride / bytesPerPixel);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		flogWrite("Failed to allocate texture %u x %u, error 0x%x", width, height, error);
		glDeleteTextures(1, &handle);
		handle = 0;
	}
	return(handle);
}

// Frees the decoded pixels through the loader that owns them. Load threads pass noTextures, textures belong to the main thread.
static void ClearPictureData(ImageLoaderRegistry* registry, ViewPicture* viewPicture, bool noTextures) {
	ImageData* image = &viewPicture->image;
	if (image->pixels.pixels != fpl_null) {
		ImageLoaderRegistryReleasePixels(registry, image->loaderEntry, &image->pixels);
	}
	if (!noTextures) {
		if (image->textureId > 0) {
			ReleaseTexture(&image->textureId);
		}
	}
	fplClearStruct(image);
	image->loaderEntry = -1;
}

// Main thread only, releases the textures as well
static void ClearViewPictures(ViewerState* state) {
	for (size_t i = 0; i < state->viewPicturesCapacity; ++i) {
		state->viewPictures[i].state = LoadedPictureState_Unloaded;
		state->viewPictures[i].progress = 0.0f;
		ClearPictureData(&state->loaderRegistry, &state->viewPictures[i], false);
		ResampleResultRelease(&state->viewPictures[i].thumbnail);
	}
}

// Reads one file through the loader registry: the loader chosen with the L key alone, otherwise the selected candidates with fallback.
// Used by the load threads and by --decode-all.
static ImageLoadResult LoadPictureFile(ImageLoaderRegistry* registry, const char* filePath, const int32_t forcedLoaderEntry, volatile bool* cancelFlag, volatile float* progress, int32_t* outLoaderEntry, PictureInfo* outInfo, ImagePixels* outPixels, char* message, const size_t messageSize) {
	ImageFileSource fileSource;
	if (!ImageFileSourceOpen(&fileSource, filePath, cancelFlag, progress)) {
		fplStringFormat(message, messageSize, "file cannot be opened");
		return(ImageLoadResult_Corrupt);
	}
	const char* extension = fplExtractFileExtension(filePath);
	ImageLoadResult result = ImageLoaderRegistryLoad(registry, &fileSource.source, extension, forcedLoaderEntry, outLoaderEntry, outInfo, outPixels, message, messageSize);
	ImageFileSourceClose(&fileSource);
	return(result);
}

static void LoadPictureThreadProc(const fplThreadHandle* thread, void* data) {
	PictureLoadThread* loadThread = (PictureLoadThread*)data;
	ViewerState* state = loadThread->state;
	volatile LoadQueueValue valueToLoad = fplZeroInit;
	while (!loadThread->shutdown) {
		// The wait must hold the mutex, the timeout catches a signal that came before the wait
		fplMutexLock(&loadThread->mutex);
		fplConditionWait(&loadThread->condition, &loadThread->mutex, LOAD_THREAD_POLL_MILLISECONDS);
		fplMutexUnlock(&loadThread->mutex);
		if (loadThread->shutdown) {
			break;
		}

		// A job that meets a cancel is dropped, a reload queues everything it needs again
		if (!TryQueueDequeue(&state->loadQueue, &valueToLoad) || loadThread->context.canceled) {
			continue;
		}
		fplAssert(valueToLoad.fileIndex >= 0 && valueToLoad.fileIndex < (int)state->pictureFileCount);
		fplAssert(valueToLoad.pictureIndex >= 0 && valueToLoad.pictureIndex < (int)state->viewPicturesCapacity);
		ViewPicture* loadedPic = &state->viewPictures[valueToLoad.pictureIndex];
		const PictureFile* picFile = &state->pictureFiles[valueToLoad.fileIndex];

		// Exactly one thread takes a free slot
		bool isTakenFromUnloaded = fplAtomicIsCompareAndSwapS32(&loadedPic->state, LoadedPictureState_Unloaded, LoadedPictureState_LoadingData);
		bool isTaken = isTakenFromUnloaded || fplAtomicIsCompareAndSwapS32(&loadedPic->state, LoadedPictureState_Error, LoadedPictureState_LoadingData);
		if (!isTaken) {
			continue;
		}

		// @TODO(final): This should not be neccesary, but in case there are left-overs...
		ClearPictureData(&state->loaderRegistry, loadedPic, true);
		fplAssert(loadedPic->image.textureId == 0);
		loadedPic->progress = 0.0f;
		loadedPic->fileIndex = (size_t)valueToLoad.fileIndex;
		fplCopyString(picFile->filePath, loadedPic->filePath, fplArrayCount(loadedPic->filePath));
		fplClearStruct(&loadedPic->info);
		loadThread->context.viewPic = loadedPic;

		flogWrite("Load picture '%s' [%zu]", loadedPic->filePath, loadedPic->fileIndex);
		PictureInfo info = fplZeroInit;
		ImagePixels pixels = fplZeroInit;
		int32_t loaderEntry = -1;
		char message[IMAGE_LOADER_MESSAGE_SIZE] = fplZeroInit;
		ImageLoadResult result = LoadPictureFile(&state->loaderRegistry, loadedPic->filePath, picFile->forcedLoaderEntry, &loadThread->context.canceled, (volatile float*)&loadedPic->progress, &loaderEntry, &info, &pixels, message, sizeof(message));
		bool isCanceled = loadThread->shutdown || loadThread->context.canceled;
		if (result == ImageLoadResult_Success && isCanceled) {
			ImageLoaderRegistryReleasePixels(&state->loaderRegistry, loaderEntry, &pixels);
			result = ImageLoadResult_Canceled;
		}

		if (result == ImageLoadResult_Success) {
			const ImageLoader* loader = ImageLoaderRegistryGet(&state->loaderRegistry, loaderEntry);
			flogWrite("Loaded picture '%s' [%zu] with %s: %s %u x %u, %u bpp%s, orientation %d", loadedPic->filePath, loadedPic->fileIndex, loader->name, info.formatName, info.width, info.height, info.bitsPerPixel, (info.isPalette ? " palette" : ""), (int)info.orientation);
			ImageData* image = &loadedPic->image;
			image->pixels = pixels;
			image->loaderEntry = loaderEntry;
			image->width = pixels.width;
			image->height = pixels.height;
			loadedPic->info = info;
			loadedPic->progress = 0.75f;
			fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_ToUpload);
		} else {
			const char* resultName = ImageLoadResultGetName(result);
			flogWrite("%s picture '%s' [%zu]: %s (%s)", (isCanceled ? "Canceled" : "Failed"), loadedPic->filePath, loadedPic->fileIndex, resultName, message);
			loadedPic->progress = 1.0f;
			fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Error);
		}
	}
}

static void InitLoadThreads(ViewerState* state, const size_t threadCount) {
	state->loadThreadCount = threadCount;
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		fplMutexInit(&state->loadThreads[i].mutex);
		fplConditionInit(&state->loadThreads[i].condition);
		state->loadThreads[i].state = state;
		state->loadThreads[i].shutdown = false;
		state->loadThreads[i].context.canceled = false;
		state->loadThreads[i].context.viewPic = fpl_null;
		state->loadThreads[i].thread = fplThreadCreate(LoadPictureThreadProc, &state->loadThreads[i]);
	}
}

static void StopLoadingInThreads(ViewerState* state) {
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].context.canceled = true;
		fplConditionSignal(&state->loadThreads[i].condition);
	}
}

static void ShutdownLoadThreads(ViewerState* state) {
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].shutdown = true;
		state->loadThreads[i].context.canceled = true;
		fplConditionSignal(&state->loadThreads[i].condition);
	}

	// @FIXME(final): Passing an invalid stride should return a false, instead of hardly crashing or do we?
	//fplThreadWaitForAll(&state->loadThreads[0].thread, state->loadThreadCount, sizeof(fplThreadHandle *), FPL_TIMEOUT_INFINITE);

	fplThreadWaitForAll(&state->loadThreads[0].thread, state->loadThreadCount, sizeof(PictureLoadThread), FPL_TIMEOUT_INFINITE);

	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		fplConditionDestroy(&state->loadThreads[i].condition);
		fplMutexDestroy(&state->loadThreads[i].mutex);
	}
}

static void QueueUpPictures(ViewerState* state) {
	// Compute how many pictures we need to preload on the left/right side
	int capacity = (int)state->viewPicturesCapacity;
	int maxSidePreloadCount = capacity / 2;
	state->viewPictureIndex = maxSidePreloadCount;
	fplAssert(state->activeFileIndex >= 0 && state->activeFileIndex < (int)state->pictureFileCount);
	int preloadCountLeft;
	int preloadCountRight;
	if (state->activeFileIndex > 0) {
		preloadCountLeft = fplMin(state->activeFileIndex, maxSidePreloadCount);
	} else {
		preloadCountLeft = 0;
	}
	if (state->activeFileIndex < ((int)state->pictureFileCount - 1)) {
		int diff = (int)state->pictureFileCount - state->activeFileIndex;
		preloadCountRight = fplMax(fplMin(diff, maxSidePreloadCount), 0);
	} else {
		preloadCountRight = 0;
	}

	// First picture
	LoadQueueValue newValue;
	newValue.fileIndex = state->activeFileIndex;
	newValue.pictureIndex = state->viewPictureIndex;
	TryQueueEnqueue(&state->loadQueue, newValue);

	// Enqueu pictures from the left side
	for (int i = 1; i <= preloadCountLeft; ++i) {
		if ((state->activeFileIndex - i) >= 0) {
			newValue.fileIndex = state->activeFileIndex - i;
			newValue.pictureIndex = state->viewPictureIndex - i;
			TryQueueEnqueue(&state->loadQueue, newValue);
		}
	}

	// Enqueu pictures from the right side
	for (int i = 1; i <= preloadCountRight; ++i) {
		if ((state->activeFileIndex + i) < (int)state->pictureFileCount) {
			newValue.fileIndex = state->activeFileIndex + i;
			newValue.pictureIndex = state->viewPictureIndex + i;
			TryQueueEnqueue(&state->loadQueue, newValue);
		}
	}

	// Wakeup load threads
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].context.canceled = false;
		fplConditionSignal(&state->loadThreads[i].condition);
	}
}

// Size as displayed: turned by the EXIF orientation
static ViewSize GetDisplayedPictureSize(const ViewPicture* picture) {
	ViewSize storedSize = fplStructInit(ViewSize, picture->image.width, picture->image.height);
	ViewSize result = ComputeViewOrientedSize((uint32_t)picture->info.orientation, storedSize);
	return(result);
}

// Transform of the active picture in the current viewport, false while the picture is not ready
static bool GetActivePictureTransform(ViewerState* state, ViewTransform* outTransform) {
	bool hasActivePicture = state->pictureFileCount > 0 && state->viewPictureIndex > -1 && state->viewPictureIndex < (int)state->viewPicturesCapacity;
	if (!hasActivePicture) {
		return(false);
	}
	ViewPicture* activePicture = &state->viewPictures[state->viewPictureIndex];
	LoadedPictureState pictureState = fplAtomicLoadS32(&activePicture->state);
	if (pictureState != LoadedPictureState_Ready) {
		return(false);
	}
	ViewSize pictureSize = GetDisplayedPictureSize(activePicture);
	*outTransform = ComputeViewTransform(&state->view, pictureSize, state->viewportSize);
	return(true);
}

// Downscaling uses the down kernel, 1:1 and upscaling the up kernel
static bool IsDownscaling(const ViewTransform* transform) {
	bool result = transform->scale < 1.0f;
	return(result);
}

static ResampleKernel GetKernelForTransform(const ViewerState* state, const ViewTransform* transform) {
	bool isDownscaling = IsDownscaling(transform);
	ResampleKernel result = isDownscaling ? state->downKernel : state->upKernel;
	return(result);
}

// Sets the title only when it changed, so it can be called every frame
static void UpdateWindowTitle(ViewerState* state) {
	const char* downArrow = "\xE2\x86\x93";
	const char* upArrow = "\xE2\x86\x91";
	const ResampleKernelDefinition* downDefinition = ResampleGetKernelDefinition(state->downKernel);
	const ResampleKernelDefinition* upDefinition = ResampleGetKernelDefinition(state->upKernel);
	const ResampleBackgroundDefinition* backgroundDefinition = ResampleGetBackgroundDefinition(state->background);

	// The filter of the direction that is in effect, both while that is unknown
	char filterText[FPL_MAX_NAME_LENGTH];
	ViewTransform transform;
	if (GetActivePictureTransform(state, &transform)) {
		bool isDownscaling = IsDownscaling(&transform);
		const char* arrow = isDownscaling ? downArrow : upArrow;
		const char* kernelName = isDownscaling ? downDefinition->name : upDefinition->name;
		fplStringFormat(filterText, fplArrayCount(filterText), "%s %s", arrow, kernelName);
	} else {
		fplStringFormat(filterText, fplArrayCount(filterText), "%s %s %s %s", downArrow, downDefinition->name, upArrow, upDefinition->name);
	}

	// The loader that read the active picture, once it is shown
	char loaderText[FPL_MAX_NAME_LENGTH] = fplZeroInit;
	bool hasActivePicture = state->viewPictureIndex > -1 && state->viewPictureIndex < (int)state->viewPicturesCapacity;
	if (hasActivePicture) {
		ViewPicture* activePicture = &state->viewPictures[state->viewPictureIndex];
		LoadedPictureState pictureState = fplAtomicLoadS32(&activePicture->state);
		const ImageLoader* loader = ImageLoaderRegistryGet(&state->loaderRegistry, activePicture->image.loaderEntry);
		if (pictureState == LoadedPictureState_Ready && loader != fpl_null) {
			fplStringFormat(loaderText, fplArrayCount(loaderText), " | %s", loader->name);
		}
	}

	char titleBuffer[FPL_MAX_BUFFER_LENGTH];
	if (state->activeFileIndex > -1) {
		const char* picFilename = fplExtractFileName(state->pictureFiles[state->activeFileIndex].filePath);
		fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s v%s - %s [%d / %zu] {%s | %s%s}", VER_PRODUCTNAME_STR, VER_PRODUCTVERSION_STR, picFilename, (state->activeFileIndex + 1), state->pictureFileCount, filterText, backgroundDefinition->name, loaderText);
	} else {
		fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s v%s - No pictures found", VER_PRODUCTNAME_STR, VER_PRODUCTVERSION_STR);
	}
	if (!fplIsStringEqual(titleBuffer, state->windowTitle)) {
		fplCopyString(titleBuffer, state->windowTitle, fplArrayCount(state->windowTitle));
		fplSetWindowTitle(titleBuffer);
	}
}

static void ChangeViewPicture(ViewerState* state, const int offset, const bool forceReload) {
	if (state->pictureFileCount == 0) {
		fplAssert(state->viewPictureIndex == -1);
		fplAssert(state->activeFileIndex == -1);
		return;
	}
	int capacity = (int)state->viewPicturesCapacity;
	bool loadPictures = false;
	int viewIndex;
	if (state->viewPictureIndex == -1 || forceReload) {
		viewIndex = capacity / 2;
		loadPictures = true;
	} else {
		viewIndex = state->viewPictureIndex + offset;
		if (viewIndex < 0 || viewIndex >= capacity) {
			viewIndex = capacity / 2;
			loadPictures = true;
		}
	}
	state->viewPictureIndex = viewIndex;
	state->activeFileIndex = fplMax(fplMin(state->activeFileIndex + offset, (int)state->pictureFileCount - 1), 0);

	UpdateWindowTitle(state);

	if (loadPictures) {
		state->doPictureReload = true;
		ShutdownQueue(&state->loadQueue);
		StopLoadingInThreads(state);
	}
}

static uint32_t ParseNumber(const char **p) {
	uint32_t v = 0;
	while (isdigit(**p)) {
		v = v * 10 + (uint8_t)(**p - '0');
		++* p;
	}
	return(v);
}

// Parses a complete unsigned decimal number, trailing characters are an error
static bool ParseUnsignedValue(const char* text, uint32_t* outValue) {
	const char* p = text;
	uint32_t value = ParseNumber(&p);
	if (p == text || *p != 0) {
		return(false);
	}
	*outValue = value;
	return(true);
}

// Parses <W>x<H>, both must be greater than zero
static bool ParseSizeValue(const char* text, uint32_t* outWidth, uint32_t* outHeight) {
	const char* p = text;
	uint32_t width = ParseNumber(&p);
	if (p == text || (*p != 'x' && *p != 'X')) {
		return(false);
	}
	++p;
	const char* heightStart = p;
	uint32_t height = ParseNumber(&p);
	if (p == heightStart || *p != 0 || width == 0 || height == 0) {
		return(false);
	}
	*outWidth = width;
	*outHeight = height;
	return(true);
}

// Parses fit, 100 or any other percentage
static bool ParseZoomValue(const char* text, ViewZoomMode* outMode, float* outScale) {
	const double actualSizePercent = 100.0;
	if (CompareStringIgnoreCase(text, "fit") == 0) {
		*outMode = ViewZoomMode_Fit;
		*outScale = 1.0f;
		return(true);
	}
	char* end = fpl_null;
	double percent = strtod(text, &end);
	if (end == text || *end != 0 || percent <= 0.0) {
		return(false);
	}
	if (percent == actualSizePercent) {
		*outMode = ViewZoomMode_ActualSize;
		*outScale = 1.0f;
	} else {
		*outMode = ViewZoomMode_Custom;
		*outScale = (float)(percent / actualSizePercent);
	}
	return(true);
}

// A kernel key (e.g. mitchell) or, for -f, a number in the numbering before Box and Mitchell existed
static bool ParseFilterValue(const char* text, const bool allowLegacyNumber, ResampleKernel* outKernel) {
	uint32_t number = 0;
	if (allowLegacyNumber && ParseUnsignedValue(text, &number)) {
		const uint32_t firstLegacyNumber = 1;
		uint32_t legacyCount = fplArrayCount(LegacyFilterNumberKernels);
		if (number < firstLegacyNumber || number >= firstLegacyNumber + legacyCount) {
			return(false);
		}
		*outKernel = LegacyFilterNumberKernels[number - firstLegacyNumber];
		return(true);
	}
	bool result = ResampleFindKernel(text, outKernel);
	return(result);
}

// Returns the text after "<name>=" or an empty string for "<name>" alone, null when the argument is a different parameter
static const char* MatchLongParameter(const char* argument, const char* name) {
	size_t nameLength = fplGetStringLength(name);
	if (strncmp(argument, name, nameLength) != 0) {
		return(fpl_null);
	}
	const char* rest = argument + nameLength;
	if (*rest == '=') {
		return(rest + 1);
	}
	if (*rest == 0) {
		return(rest);
	}
	return(fpl_null);
}

static bool ParseParameters(ViewerParameters *params, const ViewerParameters *defaultParams, const int argc, char** argv) {
	fplClearStruct(params);
	*params = *defaultParams;
	params->path = fpl_null;
	for (int i = 0; i < argc; ++i) {
		const char* argument = argv[i];
		bool isValid = true;
		if (argument[0] == '-' && argument[1] == '-') {
			const char* renderToValue = MatchLongParameter(argument, "--render-to");
			const char* windowValue = MatchLongParameter(argument, "--window");
			const char* zoomValue = MatchLongParameter(argument, "--zoom");
			const char* noPreviewValue = MatchLongParameter(argument, "--no-preview");
			const char* selfTestValue = MatchLongParameter(argument, "--selftest");
			const char* downFilterValue = MatchLongParameter(argument, "--down-filter");
			const char* upFilterValue = MatchLongParameter(argument, "--up-filter");
			const char* backgroundValue = MatchLongParameter(argument, "--background");
			const char* loaderValue = MatchLongParameter(argument, "--loader");
			const char* loaderForValue = MatchLongParameter(argument, "--loader-for");
			const char* loaderOrderValue = MatchLongParameter(argument, "--loader-order");
			const char* noLoaderFallbackValue = MatchLongParameter(argument, "--no-loader-fallback");
			const char* listLoadersValue = MatchLongParameter(argument, "--list-loaders");
			const char* decodeAllValue = MatchLongParameter(argument, "--decode-all");
			if (renderToValue != fpl_null) {
				params->renderToFilePath = renderToValue;
				isValid = *renderToValue != 0;
			} else if (windowValue != fpl_null) {
				isValid = ParseSizeValue(windowValue, &params->windowWidth, &params->windowHeight);
			} else if (zoomValue != fpl_null) {
				isValid = ParseZoomValue(zoomValue, &params->zoomMode, &params->zoomScale);
			} else if (noPreviewValue != fpl_null) {
				params->preview = false;
				isValid = *noPreviewValue == 0;
			} else if (selfTestValue != fpl_null) {
				params->runSelfTest = true;
				isValid = *selfTestValue == 0;
			} else if (downFilterValue != fpl_null) {
				isValid = ParseFilterValue(downFilterValue, false, &params->downKernel);
			} else if (upFilterValue != fpl_null) {
				isValid = ParseFilterValue(upFilterValue, false, &params->upKernel);
			} else if (backgroundValue != fpl_null) {
				isValid = ResampleFindBackground(backgroundValue, &params->background);
			} else if (loaderValue != fpl_null) {
				params->loaderValue = loaderValue;
				isValid = *loaderValue != 0;
			} else if (loaderForValue != fpl_null) {
				isValid = *loaderForValue != 0 && params->loaderForCount < fplArrayCount(params->loaderForValues);
				if (isValid) {
					params->loaderForValues[params->loaderForCount++] = loaderForValue;
				}
			} else if (loaderOrderValue != fpl_null) {
				params->loaderOrderValue = loaderOrderValue;
				isValid = *loaderOrderValue != 0;
			} else if (noLoaderFallbackValue != fpl_null) {
				params->isLoaderFallbackDisabled = true;
				isValid = *noLoaderFallbackValue == 0;
			} else if (listLoadersValue != fpl_null) {
				params->listLoaders = true;
				isValid = *listLoadersValue == 0;
			} else if (decodeAllValue != fpl_null) {
				params->decodeAllPath = decodeAllValue;
				isValid = *decodeAllValue != 0;
			} else {
				isValid = false;
			}
		} else if (argument[0] == '-') {
			// Short forms: -r, -t=<threads>, -p=<preload count>, -f=<filter number or key> for both directions
			const char shortName = argument[1];
			const bool hasValue = shortName != 0 && argument[2] == '=';
			const char* shortValue = hasValue ? argument + 3 : fpl_null;
			uint32_t number = 0;
			if (shortName == 'r' && argument[2] == 0) {
				params->recursive = true;
			} else if (shortName == 't' && hasValue && ParseUnsignedValue(shortValue, &number)) {
				params->threadCount = number;
			} else if (shortName == 'p' && hasValue && ParseUnsignedValue(shortValue, &number)) {
				params->preloadCount = number;
			} else if (shortName == 'f' && hasValue && ParseFilterValue(shortValue, true, &params->downKernel)) {
				params->upKernel = params->downKernel;
			} else {
				isValid = false;
			}
		} else {
			params->path = argument;
		}
		if (!isValid) {
			fplConsoleFormatError("Invalid parameter '%s'\n", argument);
			flogWrite("Invalid parameter '%s'", argument);
			return(false);
		}
	}
	if (params->renderToFilePath != fpl_null && (params->windowWidth == 0 || params->path == fpl_null)) {
		fplConsoleFormatError("--render-to requires --window=<W>x<H> and a picture path\n");
		flogWrite("--render-to requires --window=<W>x<H> and a picture path");
		return(false);
	}
	return(true);
}

static void LogLoaderMessage(const char* message) {
	flogWrite("%s", message);
}

// stb_image first, it is the default; the reference BMP loader stays behind it and changes nothing unless it is pinned
static bool RegisterLoaders(ImageLoaderRegistry* registry) {
	const ImageLoader* loaders[] = { ImageLoaderStbGet(), ImageLoaderPnmGet(), ImageLoaderBmpGet() };
	ImageLoaderRegistryInit(registry);
	registry->log = LogLoaderMessage;
	for (size_t loaderIndex = 0; loaderIndex < fplArrayCount(loaders); ++loaderIndex) {
		char message[IMAGE_LOADER_MESSAGE_SIZE];
		if (!ImageLoaderRegistryAdd(registry, loaders[loaderIndex], message, sizeof(message))) {
			fplConsoleFormatError("Failed to register loader '%s': %s\n", loaders[loaderIndex]->name, message);
			return(false);
		}
	}
	return(true);
}

static void ReportLoaderParameterError(const char* parameter, const char* message) {
	fplConsoleFormatError("Invalid parameter '%s': %s\n", parameter, message);
	flogWrite("Invalid parameter '%s': %s", parameter, message);
}

// Applies --loader-order, --loader, --loader-for and --no-loader-fallback; an unknown id or an ambiguous name is an error
static bool ApplyLoaderParameters(ImageLoaderRegistry* registry, const ViewerParameters* params) {
	char message[IMAGE_LOADER_MESSAGE_SIZE];
	if (params->loaderOrderValue != fpl_null) {
		char buffer[FPL_MAX_BUFFER_LENGTH];
		fplCopyString(params->loaderOrderValue, buffer, fplArrayCount(buffer));
		int32_t entries[IMAGE_LOADER_MAX_COUNT];
		uint32_t entryCount = 0;
		for (char* token = strtok(buffer, ","); token != fpl_null; token = strtok(fpl_null, ",")) {
			int32_t entry = ImageLoaderRegistryFind(registry, token, message, sizeof(message));
			if (entry < 0 || entryCount >= fplArrayCount(entries)) {
				ReportLoaderParameterError(params->loaderOrderValue, message);
				return(false);
			}
			entries[entryCount++] = entry;
		}
		if (!ImageLoaderRegistrySetOrder(registry, entries, entryCount)) {
			ReportLoaderParameterError(params->loaderOrderValue, "a loader is named twice");
			return(false);
		}
	}
	if (params->loaderValue != fpl_null) {
		int32_t entry = ImageLoaderRegistryFind(registry, params->loaderValue, message, sizeof(message));
		if (entry < 0) {
			ReportLoaderParameterError(params->loaderValue, message);
			return(false);
		}
		ImageLoaderRegistryPinGlobal(registry, entry);
	}
	for (uint32_t pinIndex = 0; pinIndex < params->loaderForCount; ++pinIndex) {
		const char* value = params->loaderForValues[pinIndex];
		const char* separator = strchr(value, ':');
		if (separator == fpl_null || separator == value) {
			ReportLoaderParameterError(value, "expected <extension>:<loader id or name>");
			return(false);
		}
		char extension[IMAGE_LOADER_MAX_EXTENSION_LENGTH];
		size_t extensionLength = (size_t)(separator - value);
		if (extensionLength >= fplArrayCount(extension)) {
			ReportLoaderParameterError(value, "extension too long");
			return(false);
		}
		fplCopyStringLen(value, extensionLength, extension, fplArrayCount(extension));
		int32_t entry = ImageLoaderRegistryFind(registry, separator + 1, message, sizeof(message));
		if (entry < 0 || !ImageLoaderRegistryPinExtension(registry, extension, entry)) {
			ReportLoaderParameterError(value, entry < 0 ? message : "too many extension pins");
			return(false);
		}
	}
	registry->isFallbackEnabled = !params->isLoaderFallbackDisabled;
	return(true);
}

// --list-loaders: one line per loader in selection order, then the pins
static void PrintLoaders(const ImageLoaderRegistry* registry) {
	fplConsoleFormatOut("Loaders in selection order, the first one is the default:\n");
	for (uint32_t orderIndex = 0; orderIndex < registry->count; ++orderIndex) {
		int32_t entry = registry->order[orderIndex];
		const ImageLoader* loader = ImageLoaderRegistryGet(registry, entry);
		char idText[IMAGE_LOADER_ID_TEXT_SIZE];
		ImageLoaderIdFormat(&loader->id, idText, sizeof(idText));
		char extensions[FPL_MAX_NAME_LENGTH] = fplZeroInit;
		for (const char* const* extension = loader->fileExtensions; *extension != fpl_null; ++extension) {
			fplStringAppend(*extension, extensions, fplArrayCount(extensions));
			fplStringAppend(" ", extensions, fplArrayCount(extensions));
		}
		const char* threadText = (loader->flags & ImageLoaderFlags_NotThreadSafe) ? "serialized" : "thread-safe";
		fplConsoleFormatOut("  %u. %-14s %-5s %s  %-11s %s\n", orderIndex + 1, loader->name, loader->version, idText, threadText, extensions);
	}
	if (registry->globalPinIndex >= 0) {
		const ImageLoader* loader = ImageLoaderRegistryGet(registry, registry->globalPinIndex);
		fplConsoleFormatOut("Pinned for all files: %s\n", loader->name);
	}
	for (uint32_t pinIndex = 0; pinIndex < registry->extensionPinCount; ++pinIndex) {
		const ImageLoader* loader = ImageLoaderRegistryGet(registry, registry->extensionPins[pinIndex].entryIndex);
		fplConsoleFormatOut("Pinned for %s: %s\n", registry->extensionPins[pinIndex].extension, loader->name);
	}
	fplConsoleFormatOut("Fallback to the next loader: %s\n", registry->isFallbackEnabled ? "on" : "off");
}

typedef struct DecodeAllResult {
	ImageLoadResult result;
	int32_t loaderEntry;
	double milliseconds;
	char message[IMAGE_LOADER_MESSAGE_SIZE];
} DecodeAllResult;

typedef struct DecodeAllJob {
	ViewerState* state;
	DecodeAllResult* results;
	volatile int32_t nextFileIndex;
} DecodeAllJob;

static void DecodeAllThreadProc(const fplThreadHandle* thread, void* data) {
	const double millisecondsPerSecond = 1000.0;
	DecodeAllJob* job = (DecodeAllJob*)data;
	ViewerState* state = job->state;
	while (true) {
		int32_t fileIndex = fplAtomicFetchAndAddS32(&job->nextFileIndex, 1);
		if (fileIndex >= (int32_t)state->pictureFileCount) {
			break;
		}
		DecodeAllResult* result = &job->results[fileIndex];
		PictureInfo info;
		ImagePixels pixels = fplZeroInit;
		fplTimestamp start = fplTimestampQuery();
		result->loaderEntry = -1;
		result->result = LoadPictureFile(&state->loaderRegistry, state->pictureFiles[fileIndex].filePath, -1, fpl_null, fpl_null, &result->loaderEntry, &info, &pixels, result->message, sizeof(result->message));
		fplTimestamp finish = fplTimestampQuery();
		result->milliseconds = fplTimestampElapsed(start, finish) * millisecondsPerSecond;
		if (result->result == ImageLoadResult_Success) {
			ImageLoaderRegistryReleasePixels(&state->loaderRegistry, result->loaderEntry, &pixels);
		}
	}
}

// --decode-all: decodes every picture of the folder with the loader threads, without window and GL, and reports errors and time per loader
static int DecodeAllPictures(ViewerState* state, const char* folderPath) {
	const double millisecondsPerSecond = 1000.0;
	const uint32_t largestListedFailures = 20;
	if (!fplDirectoryExists(folderPath)) {
		fplConsoleFormatError("Folder '%s' does not exist\n", folderPath);
		return(1);
	}
	AddPicturesFromPath(state, folderPath, state->params.recursive);
	size_t fileCount = state->pictureFileCount;
	DecodeAllJob job = fplZeroInit;
	job.state = state;
	job.results = (DecodeAllResult*)calloc(fileCount > 0 ? fileCount : 1, sizeof(DecodeAllResult));
	size_t threadCount = fplMax(fplMin(state->params.threadCount, (size_t)MAX_LOAD_THREAD_COUNT), (size_t)1);
	fplThreadHandle* threads[MAX_LOAD_THREAD_COUNT];
	fplTimestamp start = fplTimestampQuery();
	for (size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
		threads[threadIndex] = fplThreadCreate(DecodeAllThreadProc, &job);
	}
	fplThreadWaitForAll(threads, threadCount, sizeof(fplThreadHandle*), FPL_TIMEOUT_INFINITE);
	fplTimestamp finish = fplTimestampQuery();
	double wallMilliseconds = fplTimestampElapsed(start, finish) * millisecondsPerSecond;

	uint32_t failureCount = 0;
	uint32_t loaderFileCounts[IMAGE_LOADER_MAX_COUNT] = fplZeroInit;
	double loaderMilliseconds[IMAGE_LOADER_MAX_COUNT] = fplZeroInit;
	for (size_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		const DecodeAllResult* result = &job.results[fileIndex];
		if (result->result == ImageLoadResult_Success) {
			++loaderFileCounts[result->loaderEntry];
			loaderMilliseconds[result->loaderEntry] += result->milliseconds;
		} else {
			if (failureCount < largestListedFailures) {
				const char* resultName = ImageLoadResultGetName(result->result);
				fplConsoleFormatOut("FAILED %s: %s (%s)\n", state->pictureFiles[fileIndex].filePath, resultName, result->message);
			}
			++failureCount;
		}
	}
	fplConsoleFormatOut("Decoded %zu files with %zu threads in %.0f ms, %u failed\n", fileCount, threadCount, wallMilliseconds, failureCount);
	for (uint32_t entry = 0; entry < state->loaderRegistry.count; ++entry) {
		if (loaderFileCounts[entry] == 0) {
			continue;
		}
		const ImageLoader* loader = ImageLoaderRegistryGet(&state->loaderRegistry, entry);
		double averageMilliseconds = loaderMilliseconds[entry] / (double)loaderFileCounts[entry];
		fplConsoleFormatOut("  %-14s %u files, %.1f ms per file on average\n", loader->name, loaderFileCounts[entry], averageMilliseconds);
	}
	flogWrite("Decode all '%s': %zu files, %u failed, %.0f ms", folderPath, fileCount, failureCount, wallMilliseconds);
	free(job.results);
	int result = failureCount == 0 ? 0 : 1;
	return(result);
}

size_t RoundToPowerOfTwo(size_t v) {
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
#if defined(FPL_CPU_64BIT)
	if (sizeof(size_t) == 8) {
		v |= v >> 32;
	}
#endif
	++v;
	return(v);
}

static GLuint CreateShaderType(GLenum type, const char* name, const char* source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);

	char info[1024 * 10] = fplZeroInit;

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		glGetShaderInfoLog(shaderId, (GLsizei)fplArrayCount(info), fpl_null, info);
		flogWrite("Failed compiling '%s' %s shader: %s", name, (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), info);
		glDeleteShader(shaderId);
		shaderId = 0;
	}

	return(shaderId);
}

static GLuint CreateShaderProgram(const char* name, const char* vertexSource, const char* fragmentSource) {
	GLuint programId = glCreateProgram();

	GLuint vertexShader = CreateShaderType(GL_VERTEX_SHADER, name, vertexSource);
	GLuint fragmentShader = CreateShaderType(GL_FRAGMENT_SHADER, name, fragmentSource);
	if (vertexShader == 0 || fragmentShader == 0) {
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
		glDeleteProgram(programId);
		return(0);
	}

	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glLinkProgram(programId);
	glValidateProgram(programId);

	char info[1024 * 10] = fplZeroInit;

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		glGetProgramInfoLog(programId, (GLsizei)fplArrayCount(info), fpl_null, info);
		flogWrite("Failed linking '%s' shader: %s", name, info);
		glDeleteProgram(programId);
		programId = 0;
	}

	return(programId);
}

static ColorProgram CreateColorProgram() {
	ColorProgram result = fplZeroInit;
	result.programId = CreateShaderProgram("Color", RectangleVertexSource, ColorFragmentSource);
	if (result.programId > 0) {
		result.locationViewportSize = glGetUniformLocation(result.programId, "uniViewportSize");
		result.locationRect = glGetUniformLocation(result.programId, "uniRect");
		result.locationColor = glGetUniformLocation(result.programId, "uniColor");
	}
	return(result);
}

static void CheckGLError(const char* stmt, const char* fname, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		flogWrite("Error: OpenGL check %08x, at %s:%i - for %s", err, fname, line, stmt);
		fplAssert(!"OpenGL Error!");
	}
}

#define CheckGL(stmt) do { \
	(stmt); \
	CheckGLError(#stmt, __FILE__, __LINE__); \
	} while (0)

static void Kill(ViewerState* state) {
	ShutdownQueue(&state->loadQueue);
	ShutdownLoadThreads(state);
	ClearPictureFiles(state);
	ClearViewPictures(state);
}

static void Clear(ViewerState* state) {
	ShutdownQueue(&state->loadQueue);
	StopLoadingInThreads(state);
	ClearPictureFiles(state);
	ClearViewPictures(state);
}

static bool FindPictureIndexByPath(ViewerState* state, const char* path, size_t* outIndex) {
	for (size_t i = 0; i < state->pictureFileCount; ++i) {
		if (fplIsStringEqual(path, state->pictureFiles[i].filePath)) {
			*outIndex = i;
			return(true);
		}
	}
	return(false);
}

static bool LoadPicturesPath(ViewerState* state, const char* path, const bool recursive, size_t* startIndex) {
	bool result = false;
	Clear(state);
	flogWrite("Loading pictures from path '%s'", path);
	if (fplDirectoryExists(path)) {
		fplCopyString(path, state->rootPath, fplArrayCount(state->rootPath));
		AddPicturesFromPath(state, state->rootPath, recursive);
		result = state->pictureFileCount > 0;
		*startIndex = 0;
	} else if (fplFileExists(path)) {
		if (IsPictureFile(state, path)) {
			fplExtractFilePath(path, state->rootPath, fplArrayCount(state->rootPath));
			AddPicturesFromPath(state, state->rootPath, recursive);
			if (!FindPictureIndexByPath(state, path, startIndex)) {
				*startIndex = 0;
			}
			result = true;
		}
	}
	return(result);
}

static bool Init(ViewerState* state) {
	GLint majorVersion = 0;
	GLint minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	bool isMajorTooOld = majorVersion < REQUIRED_OPENGL_MAJOR_VERSION;
	bool isMinorTooOld = majorVersion == REQUIRED_OPENGL_MAJOR_VERSION && minorVersion < REQUIRED_OPENGL_MINOR_VERSION;
	if (isMajorTooOld || isMinorTooOld) {
		fplConsoleFormatError("OpenGL %d.%d core profile is required, but got %d.%d\n", REQUIRED_OPENGL_MAJOR_VERSION, REQUIRED_OPENGL_MINOR_VERSION, majorVersion, minorVersion);
		flogWrite("OpenGL %d.%d core profile is required, but got %d.%d", REQUIRED_OPENGL_MAJOR_VERSION, REQUIRED_OPENGL_MINOR_VERSION, majorVersion, minorVersion);
		return(false);
	}

	state->features.srgbFrameBuffer = false;
	GLint extensionCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	for (int i = 0; i < extensionCount; ++i) {
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (CompareStringIgnoreCase("GL_ARB_framebuffer_sRGB", extension) == 0) {
			state->features.srgbFrameBuffer = true;
		}
	}

	glClearColor(0, 0, 0, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (state->features.srgbFrameBuffer) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	glGenVertexArrays(1, &state->vertexArray);
	glBindVertexArray(state->vertexArray);

	state->colorProgram = CreateColorProgram();
	bool hasAllPrograms = state->colorProgram.programId > 0;

	if (!ResamplePipelineInit(&state->pipeline)) {
		hasAllPrograms = false;
	}
	if (!hasAllPrograms) {
		fplConsoleFormatError("Failed to create the shader programs, see the log for details\n");
		return(false);
	}

	state->downKernel = state->params.downKernel;
	state->upKernel = state->params.upKernel;
	state->background = state->params.background;

	CheckGLError("Init", __FILE__, __LINE__);

	state->viewPictureIndex = -1;
	state->activeFileIndex = -1;
	state->doPictureReload = false;

	// Allocate and startup load threads
	size_t threadCount;
	if (state->params.threadCount > 0) {
		threadCount = fplMax(fplMin(state->params.threadCount, MAX_LOAD_THREAD_COUNT), 1);
	} else {
		threadCount = fplMax(fplMin(fplCPUGetCoreCount(), MAX_LOAD_THREAD_COUNT), 1);
	}
	InitLoadThreads(state, threadCount);

	// Even count, so both sides of the active picture get the same number of preloaded pictures, plus one slot for the active picture
	const size_t maxPreloadCapacity = MAX_VIEW_PICTURE_COUNT - 2;
	size_t preloadCapacity;
	if (state->params.renderToFilePath != fpl_null) {
		preloadCapacity = 0;
	} else if (state->params.preloadCount > 0) {
		size_t evenPreloadCount = state->params.preloadCount + (state->params.preloadCount % 2);
		preloadCapacity = fplMin(evenPreloadCount, maxPreloadCapacity);
	} else {
		preloadCapacity = DEFAULT_PRELOAD_COUNT;
	}
	state->params.preloadCount = preloadCapacity;
	size_t queueCapacity = RoundToPowerOfTwo((preloadCapacity + 1) * 2);
	state->viewPicturesCapacity = preloadCapacity + 1;
	state->loadQueueCapacity = queueCapacity;

	fplAssert(fplIsPowerOfTwo(queueCapacity));
	InitQueue(&state->loadQueue, queueCapacity);

	// Load initial pictures from parameters
	if (fplGetStringLength(state->params.path) > 0) {
		size_t startPicIndex = 0;
		if (LoadPicturesPath(state, state->params.path, state->params.recursive, &startPicIndex)) {
			state->activeFileIndex = (int)startPicIndex;
			ChangeViewPicture(state, 0, true);
		}
	}

	state->view.zoomMode = state->params.zoomMode;
	state->view.customScale = state->params.zoomScale;

	UpdateWindowTitle(state);

	return(true);
}

// All programs use RectangleVertexSource, a triangle strip with 4 corners
static const GLsizei RectangleVertexCount = 4;

static void SetRectangleUniforms(const GLint locationViewportSize, const GLint locationRect, const ViewSize viewportSize, const ViewRect rect) {
	glUniform2f(locationViewportSize, (float)viewportSize.width, (float)viewportSize.height);
	glUniform4f(locationRect, rect.left, rect.top, rect.width, rect.height);
}

static void DrawSolidRectangle(const ViewerState* state, const ViewSize viewportSize, const ViewRect rect, const Vec4f color) {
	const ColorProgram* program = &state->colorProgram;
	glUseProgram(program->programId);
	SetRectangleUniforms(program->locationViewportSize, program->locationRect, viewportSize, rect);
	glUniform4fv(program->locationColor, 1, &color.m[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RectangleVertexCount);
	glUseProgram(0);
}

// Outline of the rectangle, the lines are centered on its edges and do not overlap, so a translucent color stays even
static void DrawLinedRectangle(const ViewerState* state, const ViewSize viewportSize, const ViewRect rect, const Vec4f color, const float lineWidth) {
	float halfLineWidth = lineWidth * 0.5f;
	float left = rect.left - halfLineWidth;
	float top = rect.top - halfLineWidth;
	float right = rect.left + rect.width - halfLineWidth;
	float bottom = rect.top + rect.height - halfLineWidth;
	float lineLength = rect.width + lineWidth;
	float sideTop = top + lineWidth;
	float sideLength = rect.height - lineWidth;
	ViewRect topLine = fplStructInit(ViewRect, left, top, lineLength, lineWidth);
	ViewRect bottomLine = fplStructInit(ViewRect, left, bottom, lineLength, lineWidth);
	ViewRect leftLine = fplStructInit(ViewRect, left, sideTop, lineWidth, sideLength);
	ViewRect rightLine = fplStructInit(ViewRect, right, sideTop, lineWidth, sideLength);
	DrawSolidRectangle(state, viewportSize, topLine, color);
	DrawSolidRectangle(state, viewportSize, bottomLine, color);
	DrawSolidRectangle(state, viewportSize, leftLine, color);
	DrawSolidRectangle(state, viewportSize, rightLine, color);
}

// Discards, uploads and queues pictures. Returns true when a picture changed or is still loading, so the next frame has to be drawn.
static bool UpdatePictures(ViewerState* state) {
	bool isChanging = false;

	// Discard textures on the left/right side when the fileIndex is out of bounds
	if (state->viewPictureIndex != -1) {
		ViewPicture* currentPic = &state->viewPictures[state->viewPictureIndex];
		if (currentPic->fileIndex == 0) {
			for (int i = 0; i < state->viewPictureIndex; ++i) {
				ViewPicture* sidePic = &state->viewPictures[i];
				if (sidePic->state == LoadedPictureState_Ready) {
					fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
				}
			}
		} else if (currentPic->fileIndex == state->pictureFileCount - 1) {
			for (int i = state->viewPictureIndex + 1; i < (int)state->viewPicturesCapacity; ++i) {
				ViewPicture* sidePic = &state->viewPictures[i];
				if (sidePic->state == LoadedPictureState_Ready) {
					fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
				}
			}
		}
	}

	// Discard or upload textures
	for (size_t i = 0; i < state->viewPicturesCapacity; ++i) {
		ViewPicture* loadedPic = &state->viewPictures[i];
		ImageData* image = &loadedPic->image;
		LoadedPictureState loadState = fplAtomicLoadS32(&loadedPic->state);
		if (loadState == LoadedPictureState_Discard) {
			if (image->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%zu]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&image->textureId);
			}
			ResampleResultRelease(&loadedPic->thumbnail);
			fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Unloaded);
			isChanging = true;
		} else if (loadState == LoadedPictureState_ToUpload) {
			if (image->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%zu]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&image->textureId);
			}
			ResampleResultRelease(&loadedPic->thumbnail);
			fplAssert(image->pixels.pixels != fpl_null);
			fplAssert(image->width > 0 && image->height > 0);

			fplDebugFormatOut("Allocate texture '%s'[%zu]\n", loadedPic->filePath, loadedPic->fileIndex);
			image->textureId = AllocateTexture(image->width, image->height, image->pixels.stride, image->pixels.pixels, state->features.srgbFrameBuffer);
			ImageLoaderRegistryReleasePixels(&state->loaderRegistry, image->loaderEntry, &image->pixels);
			loadedPic->uploadSerial = ++state->nextUploadSerial;

			LoadedPictureState uploadedState = image->textureId > 0 ? LoadedPictureState_Ready : LoadedPictureState_Error;
			fplAtomicStoreS32(&loadedPic->state, uploadedState);
			loadedPic->progress = 1.0f;
			isChanging = true;
		} else if (loadState == LoadedPictureState_LoadingData) {
			// The progress bars move
			isChanging = true;
		}
	}

	// Start to queue up pictures to load
	if (state->doPictureReload) {
		size_t notUnloadedCount = 0;
		for (size_t i = 0; i < state->viewPicturesCapacity; ++i) {
			ViewPicture* viewPic = &state->viewPictures[i];
			LoadedPictureState loadState = fplAtomicLoadS32(&viewPic->state);
			if (loadState != LoadedPictureState_Unloaded) {
				if (loadState != LoadedPictureState_Error) {
					fplAtomicStoreS32(&viewPic->state, LoadedPictureState_Discard);
					++notUnloadedCount;
				}
			}
		}
		if (notUnloadedCount == 0) {
			InitQueue(&state->loadQueue, state->loadQueueCapacity);
			QueueUpPictures(state);
			state->doPictureReload = false;
		}
		isChanging = true;
	}

	CheckGLError("UpdatePictures", __FILE__, __LINE__);

	return(isChanging);
}

static Vec4f GetPreviewBlockColor(const LoadedPictureState loadState) {
	Vec4f result;
	switch (loadState) {
		case LoadedPictureState_LoadingData:
			result = V4fInit(0.0f, 0.0f, 1.0f, 0.5f);
			break;
		case LoadedPictureState_Ready:
			result = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		case LoadedPictureState_ToUpload:
			result = V4fInit(0.0f, 0.5f, 0.5f, 0.5f);
			break;
		case LoadedPictureState_Discard:
			result = V4fInit(0.75f, 0.25f, 0.0f, 0.5f);
			break;
		case LoadedPictureState_Error:
			result = V4fInit(1.0f, 0.0f, 0.0f, 0.5f);
			break;
		default:
			fplAssert(!"Invalid loaded picture state!");
			result = V4fInit(0.0f, 0.0f, 0.0f, 0.0f);
			break;
	}
	return(result);
}

// Part of a whole pixel rectangle that lies inside the viewport
static ViewRect IntersectWithViewport(const ViewRect rect, const ViewSize viewportSize) {
	float left = fplMax(rect.left, 0.0f);
	float top = fplMax(rect.top, 0.0f);
	float right = fplMin(rect.left + rect.width, (float)viewportSize.width);
	float bottom = fplMin(rect.top + rect.height, (float)viewportSize.height);
	float width = fplMax(right - left, 0.0f);
	float height = fplMax(bottom - top, 0.0f);
	ViewRect result = fplStructInit(ViewRect, left, top, width, height);
	return(result);
}

// Scales the picture for the transform into the result, but only the part in outputRect (viewport pixels). Returns true when it had to be computed.
static bool UpdatePictureResult(ViewerState* state, const ViewPicture* picture, const ViewTransform* transform, const ViewRect outputRect, ResampleResult* result, const char* timerLabel) {
	ResampleRequest request = fplZeroInit;
	request.sourceTexture = picture->image.textureId;
	request.sourceSerial = picture->uploadSerial;
	ViewSize storedSize = fplStructInit(ViewSize, picture->image.width, picture->image.height);
	ViewSize displayedSize = GetDisplayedPictureSize(picture);
	ViewOrientationMapping mapping = ComputeViewOrientationMapping((uint32_t)picture->info.orientation, storedSize);
	request.sourceWidth = displayedSize.width;
	request.sourceHeight = displayedSize.height;
	request.axisMapping.originX = mapping.originX;
	request.axisMapping.originY = mapping.originY;
	request.axisMapping.stepUX = mapping.stepUX;
	request.axisMapping.stepUY = mapping.stepUY;
	request.axisMapping.stepVX = mapping.stepVX;
	request.axisMapping.stepVY = mapping.stepVY;
	request.kernel = GetKernelForTransform(state, transform);
	request.scaleX = transform->scaleX;
	request.scaleY = transform->scaleY;
	request.originX = transform->imageRect.left - outputRect.left;
	request.originY = transform->imageRect.top - outputRect.top;
	request.outputWidth = (uint32_t)outputRect.width;
	request.outputHeight = (uint32_t)outputRect.height;
	bool isComputed = ResampleUpdate(&state->pipeline, &request, result, timerLabel);
	return(isComputed);
}

// One block per view picture slot at the bottom, the active slot is outlined green
static void RenderPreviewStrip(ViewerState* state, const ViewSize viewportSize) {
	const float stripWidthFactor = 0.75f;
	const float blockPadding = 4.0f;
	const float activeBlockLineWidth = 2.0f;
	const float blockLineWidth = 1.0f;
	const float minimumPictureBlockSize = 1.0f;
	const Vec4f activeBlockLineColor = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
	const Vec4f unloadedBlockLineColor = V4fInit(1.0f, 1.0f, 1.0f, 0.2f);
	const Vec4f loadedBlockLineColor = V4fInit(1.0f, 1.0f, 1.0f, 0.5f);

	float viewportWidth = (float)viewportSize.width;
	float viewportHeight = (float)viewportSize.height;
	int blockCount = (int)state->viewPicturesCapacity;
	float shorterViewportSide = fplMin(viewportWidth, viewportHeight);
	float stripWidth = shorterViewportSide * stripWidthFactor;
	float paddingSum = (float)(blockCount - 1) * blockPadding;
	float blockSize = (stripWidth - paddingSum) / (float)blockCount;
	float stripLeft = (viewportWidth - stripWidth) * 0.5f;
	float stripTop = floorf(viewportHeight - blockPadding - blockSize);
	ViewState fitView = fplStructInit(ViewState, ViewZoomMode_Fit, 0.0f);
	ViewSize blockPixelSize = fplStructInit(ViewSize, (uint32_t)blockSize, (uint32_t)blockSize);

	for (int i = 0; i < blockCount; ++i) {
		ViewPicture* picture = &state->viewPictures[i];
		float blockLeft = floorf(stripLeft + (float)i * (blockSize + blockPadding));
		ViewRect blockRect = fplStructInit(ViewRect, blockLeft, stripTop, blockSize, blockSize);

		LoadedPictureState loadState = fplAtomicLoadS32(&picture->state);
		if (loadState == LoadedPictureState_Ready) {
			if (blockSize >= minimumPictureBlockSize) {
				// Fitted into the block with the resample pipeline, computed once per picture and block size
				ViewSize pictureSize = GetDisplayedPictureSize(picture);
				ViewTransform transform = ComputeViewTransform(&fitView, pictureSize, blockPixelSize);
				UpdatePictureResult(state, picture, &transform, transform.imageRect, &picture->thumbnail, fpl_null);
				ResampleCompositeParameters composite = fplZeroInit;
				composite.viewportWidth = viewportSize.width;
				composite.viewportHeight = viewportSize.height;
				composite.left = blockLeft + transform.imageRect.left;
				composite.top = stripTop + transform.imageRect.top;
				composite.background = state->background;
				composite.checkerOriginX = composite.left;
				composite.checkerOriginY = composite.top;
				ResampleComposite(&state->pipeline, &picture->thumbnail, &composite);
			}
		} else if (loadState != LoadedPictureState_Unloaded) {
			// Grows from the center with the progress
			Vec4f color = GetPreviewBlockColor(loadState);
			float progressSize = blockSize * picture->progress;
			float progressOffset = (blockSize - progressSize) * 0.5f;
			ViewRect progressRect = fplStructInit(ViewRect, blockLeft + progressOffset, stripTop + progressOffset, progressSize, progressSize);
			DrawSolidRectangle(state, viewportSize, progressRect, color);
		}

		if (i == state->viewPictureIndex) {
			DrawLinedRectangle(state, viewportSize, blockRect, activeBlockLineColor, activeBlockLineWidth);
		} else if (loadState == LoadedPictureState_Unloaded) {
			DrawLinedRectangle(state, viewportSize, blockRect, unloadedBlockLineColor, blockLineWidth);
		} else {
			DrawLinedRectangle(state, viewportSize, blockRect, loadedBlockLineColor, blockLineWidth);
		}
	}
}

// Draws into the bound framebuffer, the resample passes run before and keep that binding
static void RenderFrame(ViewerState* state, const ViewSize viewportSize) {
	state->viewportSize = viewportSize;

	// Scale the active picture, only when its placement, filter or content changed
	ViewTransform transform;
	bool isActivePictureReady = GetActivePictureTransform(state, &transform);
	ViewRect visibleRect = fplZeroInit;
	if (isActivePictureReady) {
		ViewPicture* activePicture = &state->viewPictures[state->viewPictureIndex];
		ResampleKernel kernel = GetKernelForTransform(state, &transform);
		const ResampleKernelDefinition* kernelDefinition = ResampleGetKernelDefinition(kernel);
		const char* pictureFileName = fplExtractFileName(activePicture->filePath);
		visibleRect = IntersectWithViewport(transform.imageRect, viewportSize);
		char timerLabel[FPL_MAX_BUFFER_LENGTH];
		fplStringFormat(timerLabel, fplArrayCount(timerLabel), "'%s' %u x %u -> %u x %u of %u x %u (%s)", pictureFileName, activePicture->image.width, activePicture->image.height, (uint32_t)visibleRect.width, (uint32_t)visibleRect.height, (uint32_t)transform.imageRect.width, (uint32_t)transform.imageRect.height, kernelDefinition->name);
		UpdatePictureResult(state, activePicture, &transform, visibleRect, &state->viewResult, timerLabel);
	}

	glViewport(0, 0, (GLsizei)viewportSize.width, (GLsizei)viewportSize.height);
	glClear(GL_COLOR_BUFFER_BIT);

	float viewportWidth = (float)viewportSize.width;
	bool hasActivePicture = state->pictureFileCount > 0 && state->viewPictureIndex > -1 && state->viewPictureIndex < (int)state->viewPicturesCapacity;
	if (isActivePictureReady) {
		ResampleCompositeParameters composite = fplZeroInit;
		composite.viewportWidth = viewportSize.width;
		composite.viewportHeight = viewportSize.height;
		composite.left = visibleRect.left;
		composite.top = visibleRect.top;
		composite.background = state->background;
		composite.checkerOriginX = transform.imageRect.left;
		composite.checkerOriginY = transform.imageRect.top;
		ResampleComposite(&state->pipeline, &state->viewResult, &composite);
	} else if (hasActivePicture) {
		ViewPicture* activePicture = &state->viewPictures[state->viewPictureIndex];
		LoadedPictureState pictureState = fplAtomicLoadS32(&activePicture->state);
		if (pictureState == LoadedPictureState_LoadingData) {
			// Progress bar centered at the top
			const float progressPadding = 4.0f;
			const float progressWidthFactor = 0.5f;
			const float progressAspectRatio = 400.0f / 10.0f;
			const float progressBorderWidth = 2.0f;
			const Vec4f progressFillColor = V4fInit(0.25f, 0.25f, 0.25f, 1.0f);
			const Vec4f progressBorderColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
			float progressWidth = viewportWidth * progressWidthFactor;
			float progressHeight = progressWidth / progressAspectRatio;
			float progressLeft = (viewportWidth - progressWidth) * 0.5f;
			float filledWidth = progressWidth * activePicture->progress;
			ViewRect filledRect = fplStructInit(ViewRect, progressLeft, progressPadding, filledWidth, progressHeight);
			ViewRect borderRect = fplStructInit(ViewRect, progressLeft, progressPadding, progressWidth, progressHeight);
			DrawSolidRectangle(state, viewportSize, filledRect, progressFillColor);
			DrawLinedRectangle(state, viewportSize, borderRect, progressBorderColor, progressBorderWidth);
		}
	}

	if (state->params.preview && state->viewPicturesCapacity > 1 && state->pictureFileCount > 0) {
		RenderPreviewStrip(state, viewportSize);
	}

	CheckGLError("RenderFrame", __FILE__, __LINE__);
}

// Writes the GPU time of the last timed resample into the log once it is available
static void LogResampleTimer(ViewerState* state) {
	const char* label = fpl_null;
	double horizontalMilliseconds = 0.0;
	double verticalMilliseconds = 0.0;
	if (ResamplePollTimer(&state->pipeline, &label, &horizontalMilliseconds, &verticalMilliseconds)) {
		flogWrite("Resample %s: horizontal %.3f ms, vertical %.3f ms", label, horizontalMilliseconds, verticalMilliseconds);
	}
}

// Reads a view picture again, main thread only: the texture goes at once and the slot is queued like on the first load
static void ReloadViewPicture(ViewerState* state, const int pictureIndex) {
	ViewPicture* picture = &state->viewPictures[pictureIndex];
	ClearPictureData(&state->loaderRegistry, picture, false);
	ResampleResultRelease(&picture->thumbnail);
	picture->progress = 0.0f;
	fplAtomicStoreS32(&picture->state, LoadedPictureState_Unloaded);
	LoadQueueValue value;
	value.fileIndex = (int)picture->fileIndex;
	value.pictureIndex = pictureIndex;
	TryQueueEnqueue(&state->loadQueue, value);
	for (size_t threadIndex = 0; threadIndex < state->loadThreadCount; ++threadIndex) {
		fplConditionSignal(&state->loadThreads[threadIndex].condition);
	}
}

// L: reads the active picture again with the next loader that recognizes the file, so two loaders can be compared on the same file
static void SwitchActivePictureLoader(ViewerState* state) {
	bool hasActivePicture = state->activeFileIndex > -1 && state->viewPictureIndex > -1 && state->viewPictureIndex < (int)state->viewPicturesCapacity;
	if (!hasActivePicture) {
		return;
	}
	ViewPicture* picture = &state->viewPictures[state->viewPictureIndex];
	LoadedPictureState pictureState = fplAtomicLoadS32(&picture->state);
	if (pictureState != LoadedPictureState_Ready && pictureState != LoadedPictureState_Error) {
		return;
	}
	PictureFile* file = &state->pictureFiles[picture->fileIndex];

	int32_t candidates[IMAGE_LOADER_MAX_COUNT];
	uint32_t candidateCount = 0;
	ImageFileSource fileSource;
	if (ImageFileSourceOpen(&fileSource, file->filePath, fpl_null, fpl_null)) {
		const char* extension = fplExtractFileExtension(file->filePath);
		candidateCount = ImageLoaderRegistrySelectForSource(&state->loaderRegistry, &fileSource.source, extension, true, candidates, IMAGE_LOADER_MAX_COUNT);
		ImageFileSourceClose(&fileSource);
	}
	if (candidateCount < 2) {
		flogWrite("No other loader recognizes '%s'", file->filePath);
		return;
	}

	int32_t currentEntry = pictureState == LoadedPictureState_Ready ? picture->image.loaderEntry : file->forcedLoaderEntry;
	uint32_t nextIndex = 0;
	for (uint32_t candidateIndex = 0; candidateIndex < candidateCount; ++candidateIndex) {
		if (candidates[candidateIndex] == currentEntry) {
			nextIndex = (candidateIndex + 1) % candidateCount;
		}
	}
	file->forcedLoaderEntry = candidates[nextIndex];
	const ImageLoader* nextLoader = ImageLoaderRegistryGet(&state->loaderRegistry, file->forcedLoaderEntry);
	flogWrite("Read '%s' again with loader %s", file->filePath, nextLoader->name);
	ReloadViewPicture(state, state->viewPictureIndex);
}

// Steps the kernel of the direction that is in effect, the down kernel while no picture is shown
static void CycleActiveKernel(ViewerState* state, const int step) {
	ViewTransform transform;
	bool isDownscaling = true;
	if (GetActivePictureTransform(state, &transform)) {
		isDownscaling = IsDownscaling(&transform);
	}
	ResampleKernel* kernel = isDownscaling ? &state->downKernel : &state->upKernel;
	int kernelCount = (int)ResampleKernel_Count;
	int next = ((int)*kernel + step + kernelCount) % kernelCount;
	*kernel = (ResampleKernel)next;
}

typedef struct OffscreenTarget {
	GLuint frameBufferId;
	GLuint colorTextureId;
	uint32_t width;
	uint32_t height;
} OffscreenTarget;

static void DestroyOffscreenTarget(OffscreenTarget* target) {
	if (target->frameBufferId > 0) {
		glDeleteFramebuffers(1, &target->frameBufferId);
	}
	if (target->colorTextureId > 0) {
		glDeleteTextures(1, &target->colorTextureId);
	}
	fplClearStruct(target);
}

// Color attachment is sRGB when the default framebuffer is, so GL_FRAMEBUFFER_SRGB encodes exactly as on screen
static bool CreateOffscreenTarget(OffscreenTarget* target, const uint32_t width, const uint32_t height, const bool supportsSRGB) {
	fplClearStruct(target);
	target->width = width;
	target->height = height;

	GLenum internalFormat = supportsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	glGenTextures(1, &target->colorTextureId);
	glBindTexture(GL_TEXTURE_2D, target->colorTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fpl_null);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &target->frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, target->frameBufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->colorTextureId, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLenum error = glGetError();
	if (status != GL_FRAMEBUFFER_COMPLETE || error != GL_NO_ERROR) {
		flogWrite("Failed to create offscreen framebuffer %u x %u, status 0x%x, error 0x%x", width, height, status, error);
		DestroyOffscreenTarget(target);
		return(false);
	}
	return(true);
}

// Writes bottom-up RGBA rows (as glReadPixels returns them) as a top-down PAM with RGB tuples, alpha is dropped because the screen shows no alpha either
static bool WritePortableArbitraryMapRGB(const char* filePath, const uint8_t* bottomUpPixelsRGBA, const uint32_t width, const uint32_t height) {
	const uint32_t sourceComponents = 4;
	const uint32_t targetComponents = 3;
	fplFileHandle file;
	if (!fplFileCreateBinary(filePath, &file)) {
		return(false);
	}
	char header[256];
	size_t headerLength = fplStringFormat(header, fplArrayCount(header), "P7\nWIDTH %u\nHEIGHT %u\nDEPTH %u\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n", width, height, targetComponents);
	uint32_t writtenHeaderBytes = fplFileWriteBlock32(&file, header, (uint32_t)headerLength);
	bool result = writtenHeaderBytes == (uint32_t)headerLength;

	uint32_t targetRowSize = width * targetComponents;
	uint8_t* targetRow = (uint8_t*)malloc(targetRowSize);
	for (uint32_t row = 0; result && row < height; ++row) {
		uint32_t sourceRow = height - 1 - row;
		const uint8_t* source = bottomUpPixelsRGBA + (size_t)sourceRow * width * sourceComponents;
		for (uint32_t x = 0; x < width; ++x) {
			targetRow[x * targetComponents + 0] = source[x * sourceComponents + 0];
			targetRow[x * targetComponents + 1] = source[x * sourceComponents + 1];
			targetRow[x * targetComponents + 2] = source[x * sourceComponents + 2];
		}
		uint32_t writtenRowBytes = fplFileWriteBlock32(&file, targetRow, targetRowSize);
		result = writtenRowBytes == targetRowSize;
	}
	free(targetRow);
	fplFileClose(&file);
	return(result);
}

static RenderToFileResult ReadOffscreenTargetToFile(const OffscreenTarget* target, const char* filePath) {
	const size_t bytesPerPixel = 4;
	size_t pixelsSize = (size_t)target->width * target->height * bytesPerPixel;
	uint8_t* pixels = (uint8_t*)malloc(pixelsSize);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, target->frameBufferId);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, (GLsizei)target->width, (GLsizei)target->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	GLenum error = glGetError();
	RenderToFileResult result;
	if (error != GL_NO_ERROR) {
		flogWrite("Failed to read offscreen framebuffer, error 0x%x", error);
		result = RenderToFileResult_GraphicsFailed;
	} else if (!WritePortableArbitraryMapRGB(filePath, pixels, target->width, target->height)) {
		flogWrite("Failed to write '%s'", filePath);
		result = RenderToFileResult_WriteFailed;
	} else {
		result = RenderToFileResult_Success;
	}
	free(pixels);
	return(result);
}

// Renders the active picture into an offscreen framebuffer of --window size, as soon as it is loaded, and writes it to --render-to
static RenderToFileResult RenderPictureToFile(ViewerState* state) {
	const char* filePath = state->params.renderToFilePath;
	const uint32_t width = state->params.windowWidth;
	const uint32_t height = state->params.windowHeight;
	const ViewSize targetSize = fplStructInit(ViewSize, width, height);
	if (state->pictureFileCount == 0) {
		fplConsoleFormatError("No picture found in '%s'\n", state->params.path);
		return(RenderToFileResult_NoPicture);
	}

	OffscreenTarget target;
	if (!CreateOffscreenTarget(&target, width, height, state->features.srgbFrameBuffer)) {
		fplConsoleFormatError("Failed to create offscreen framebuffer %u x %u\n", width, height);
		return(RenderToFileResult_GraphicsFailed);
	}

	RenderToFileResult result = RenderToFileResult_Canceled;
	fplMilliseconds startTime = fplMillisecondsQuery();
	while (fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
		}

		UpdatePictures(state);
		glBindFramebuffer(GL_FRAMEBUFFER, target.frameBufferId);
		RenderFrame(state, targetSize);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// The frame that uploads the picture already draws it
		if (!state->doPictureReload && state->viewPictureIndex > -1) {
			ViewPicture* activePicture = &state->viewPictures[state->viewPictureIndex];
			LoadedPictureState pictureState = fplAtomicLoadS32(&activePicture->state);
			if (pictureState == LoadedPictureState_Ready) {
				result = ReadOffscreenTargetToFile(&target, filePath);
				glFinish();
				LogResampleTimer(state);
				break;
			} else if (pictureState == LoadedPictureState_Error) {
				fplConsoleFormatError("Failed to load picture '%s'\n", activePicture->filePath);
				result = RenderToFileResult_LoadFailed;
				break;
			}
		}

		fplMilliseconds elapsed = fplMillisecondsQuery() - startTime;
		if (elapsed > RENDER_TO_FILE_TIMEOUT_MILLISECONDS) {
			fplConsoleFormatError("Timeout while waiting for picture '%s'\n", state->params.path);
			result = RenderToFileResult_Timeout;
			break;
		}
		fplThreadSleep(RENDER_TO_FILE_POLL_MILLISECONDS);
	}

	DestroyOffscreenTarget(&target);

	if (result == RenderToFileResult_Success) {
		flogWrite("Rendered '%s' into '%s' (%u x %u)", state->params.path, filePath, width, height);
	} else if (result == RenderToFileResult_GraphicsFailed || result == RenderToFileResult_WriteFailed) {
		fplConsoleFormatError("Failed to write '%s'\n", filePath);
	}
	return(result);
}

static void LogCallbackFunc(const char* funcName, const int lineNumber, fplLogLevel level, const char* message) {
	flogWrite("%s", message);
}

int main(int argc, char** argv) {
	// Initialize logging
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
		char logFilePath[FPL_MAX_PATH_LENGTH];
		fplGetHomePath(logFilePath, fplArrayCount(logFilePath));
		fplPathCombine(logFilePath, fplArrayCount(logFilePath), 2, logFilePath, VER_INTERNALNAME_STR);
		fplDirectoriesCreate(logFilePath);
		fplPathCombine(logFilePath, fplArrayCount(logFilePath), 2, logFilePath, "log.txt");
		flogInit(logFilePath);
		fplPlatformRelease();
	}

	fplLogSettings logSettings = fplZeroInit;
	logSettings.maxLevel = fplLogLevel_All;
	logSettings.writers[0].flags = fplLogWriterFlags_Custom;
	logSettings.writers[0].custom.callback = LogCallbackFunc;
	fplSetLogSettings(&logSettings);

	flogWrite("Startup %s", VER_PRODUCTNAME_STR);

	ViewerState* state = (ViewerState*)fplMemoryAllocate(sizeof(ViewerState));
	ViewerParameters defaultParams = fplZeroInit;
	defaultParams.preview = true;
	defaultParams.downKernel = DEFAULT_DOWN_KERNEL;
	defaultParams.upKernel = DEFAULT_UP_KERNEL;
	defaultParams.background = DEFAULT_BACKGROUND;
	defaultParams.threadCount = fplMax(fplMin(fplCPUGetCoreCount(), MAX_LOAD_THREAD_COUNT), 1);
	state->params = defaultParams;
	if (argc >= 2) {
		if (!ParseParameters(&state->params, &defaultParams, argc - 1, argv + 1)) {
			fplMemoryFree(state);
			return(RenderToFileResult_InvalidParameters);
		}
	}

	if (state->params.runSelfTest) {
		int selfTestResult = RunSelfTest();
		fplMemoryFree(state);
		return(selfTestResult);
	}

	if (!RegisterLoaders(&state->loaderRegistry) || !ApplyLoaderParameters(&state->loaderRegistry, &state->params)) {
		ImageLoaderRegistryRelease(&state->loaderRegistry);
		fplMemoryFree(state);
		return(RenderToFileResult_InvalidParameters);
	}

	// Modes without window
	if (state->params.listLoaders) {
		PrintLoaders(&state->loaderRegistry);
		ImageLoaderRegistryRelease(&state->loaderRegistry);
		fplMemoryFree(state);
		return(0);
	}
	if (state->params.decodeAllPath != fpl_null) {
		int decodeResult = 1;
		if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
			decodeResult = DecodeAllPictures(state, state->params.decodeAllPath);
			ClearPictureFiles(state);
			fplPlatformRelease();
		}
		ImageLoaderRegistryRelease(&state->loaderRegistry);
		fplMemoryFree(state);
		return(decodeResult);
	}

	const bool isRenderToFile = state->params.renderToFilePath != fpl_null;
	if (isRenderToFile) {
		// Nothing but the picture itself goes into the file
		state->params.preview = false;
	}

	flogWrite("Initial Parameters:");
	flogWrite("Path: %s", state->params.path);
	flogWrite("Preload count: %zu", state->params.preloadCount);
	flogWrite("Thread count: %zu", state->params.threadCount);
	flogWrite("Preview enabled: %s", (state->params.preview ? "yes" : "no"));
	flogWrite("Recursive enabled: %s", (state->params.recursive ? "yes" : "no"));
	flogWrite("Window size: %u x %u", state->params.windowWidth, state->params.windowHeight);
	flogWrite("Zoom mode: %d, scale: %f", (int)state->params.zoomMode, state->params.zoomScale);
	const ResampleKernelDefinition* downKernelDefinition = ResampleGetKernelDefinition(state->params.downKernel);
	const ResampleKernelDefinition* upKernelDefinition = ResampleGetKernelDefinition(state->params.upKernel);
	const ResampleBackgroundDefinition* backgroundDefinition = ResampleGetBackgroundDefinition(state->params.background);
	flogWrite("Filters: down %s, up %s, background %s", downKernelDefinition->name, upKernelDefinition->name, backgroundDefinition->name);
	if (isRenderToFile) {
		flogWrite("Render to: %s", state->params.renderToFilePath);
	}

	int returnCode = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	settings.video.isVSync = !isRenderToFile;
	settings.video.backend = fplVideoBackendType_OpenGL;
	// No multisampling: it only smooths geometry edges, a full screen picture has none and the filters compute every pixel themselves
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = REQUIRED_OPENGL_MAJOR_VERSION;
	settings.video.graphics.opengl.minorVersion = REQUIRED_OPENGL_MINOR_VERSION;
	settings.video.graphics.opengl.multiSamplingCount = 0;
	fplCopyString("FPL Demo - Image Viewer", settings.window.title, fplArrayCount(settings.window.title));
	if (isRenderToFile) {
		settings.window.windowSize.width = RENDER_TO_FILE_WINDOW_SIZE;
		settings.window.windowSize.height = RENDER_TO_FILE_WINDOW_SIZE;
	} else if (state->params.windowWidth > 0) {
		settings.window.windowSize.width = state->params.windowWidth;
		settings.window.windowSize.height = state->params.windowHeight;
	}

	// Load icons (Memory are released on shutdown)
	int iconW, iconH, iconC;
	uint8_t* icon16Data = stbi_load_from_memory(icon16DataArray, icon16DataArraySize, &iconW, &iconH, &iconC, 4);
	if (icon16Data != fpl_null) {
		settings.window.icons[0].data = icon16Data;
		settings.window.icons[0].width = iconW;
		settings.window.icons[0].height = iconH;
		settings.window.icons[0].type = fplImageType_RGBA;
	}
	iconW = iconH = iconC = 0;
	uint8_t* icon32Data = stbi_load_from_memory(icon32DataArray, icon32DataArraySize, &iconW, &iconH, &iconC, 4);
	if (icon32Data != fpl_null) {
		settings.window.icons[1].data = icon32Data;
		settings.window.icons[1].width = iconW;
		settings.window.icons[1].height = iconH;
		settings.window.icons[1].type = fplImageType_RGBA;
	}

	bool isPlatformInitialized = fplPlatformInit(fplInitFlags_Video, &settings);
	if (!isPlatformInitialized) {
		const char* platformError = fplGetLastError();
		fplConsoleFormatError("Failed to create a window with an OpenGL %d.%d core profile context: %s\n", REQUIRED_OPENGL_MAJOR_VERSION, REQUIRED_OPENGL_MINOR_VERSION, platformError);
		flogWrite("Failed to create a window with an OpenGL %d.%d core profile context: %s", REQUIRED_OPENGL_MAJOR_VERSION, REQUIRED_OPENGL_MINOR_VERSION, platformError);
		returnCode = -1;
	}
	bool isOpenGLLoaded = isPlatformInitialized && fglLoadOpenGL(true);
	if (isPlatformInitialized && !isOpenGLLoaded) {
		fplConsoleFormatError("Failed to load the OpenGL functions\n");
		flogWrite("Failed to load the OpenGL functions");
		returnCode = -1;
	}
	if (isOpenGLLoaded) {
		if (Init(state)) {
			fplKey activeKey = fplKey_None;
			uint64_t activeKeyStart = 0;
			const int ActiveKeyThreshold = 150;
			if (isRenderToFile) {
				returnCode = RenderPictureToFile(state);
			}
			// The first frame is always drawn
			bool hasDrawnFrame = false;
			ViewSize lastViewportSize = fplZeroInit;
			while (!isRenderToFile && fplWindowUpdate()) {
				// Events
				bool hasEvents = false;
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					hasEvents = true;
					switch (ev.type) {
						case fplEventType_Window:
						{
							if (ev.window.type == fplWindowEventType_DroppedFiles) {
								size_t startPicIndex = 0;
								for (size_t fileIndex = 0; fileIndex < ev.window.dropFiles.fileCount; ++fileIndex) {
									const char* filePath = ev.window.dropFiles.files[fileIndex];
									// @TODO(final): LoadPicturesPath clears the picture files always, so we basically can only load one folder at a time
									if (LoadPicturesPath(state, filePath, false, &startPicIndex)) {
										state->activeFileIndex = (int)startPicIndex;
										ChangeViewPicture(state, 0, true);
									}
								}
							}
						} break;

						case fplEventType_Keyboard:
						{
							if (ev.keyboard.type == fplKeyboardEventType_Button) {
								if (ev.keyboard.buttonState >= fplButtonState_Press) {
									bool isActiveKeyRepeat;
									if (activeKey != ev.keyboard.mappedKey) {
										activeKey = ev.keyboard.mappedKey;
										activeKeyStart = fplMillisecondsQuery();
										isActiveKeyRepeat = false;
									} else {
										isActiveKeyRepeat = (fplMillisecondsQuery() - activeKeyStart) >= ActiveKeyThreshold;
									}
									if (ev.keyboard.mappedKey == fplKey_Left) {
										if (activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if (state->activeFileIndex > 0) {
												ChangeViewPicture(state, -1, false);
											}
										}
									} else if (ev.keyboard.mappedKey == fplKey_Right) {
										if (activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if (state->activeFileIndex < ((int)state->pictureFileCount - 1)) {
												ChangeViewPicture(state, +1, false);
											}
										}
									} else if (ev.keyboard.mappedKey == fplKey_T && ev.keyboard.buttonState == fplButtonState_Press) {
										// Filter of the direction in effect, backwards with shift. On press, because shift is often let go before the key.
										int shiftFlags = (int)fplKeyboardModifierFlags_LShift | (int)fplKeyboardModifierFlags_RShift;
										bool isShiftDown = ((int)ev.keyboard.modifiers & shiftFlags) != 0;
										int step = isShiftDown ? -1 : 1;
										CycleActiveKernel(state, step);
									}
								} else {
									fplAssert(ev.keyboard.buttonState == fplButtonState_Release);
									activeKey = fplKey_None;
									activeKeyStart = 0;
									if (ev.keyboard.mappedKey == fplKey_Left) {
										if (state->activeFileIndex > 0) {
											ChangeViewPicture(state, -1, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_Right) {
										if (state->activeFileIndex < ((int)state->pictureFileCount - 1)) {
											ChangeViewPicture(state, +1, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_PageDown) {
										if (state->activeFileIndex < ((int)state->pictureFileCount - PAGE_INCREMENT_COUNT)) {
											ChangeViewPicture(state, PAGE_INCREMENT_COUNT, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_PageUp) {
										if (state->activeFileIndex > (PAGE_INCREMENT_COUNT - 1)) {
											ChangeViewPicture(state, -PAGE_INCREMENT_COUNT, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_Home) {
										int delta = 0 - (int)state->activeFileIndex;
										ChangeViewPicture(state, delta, true);
									} else if (ev.keyboard.mappedKey == fplKey_End) {
										int delta = (int)state->pictureFileCount - state->activeFileIndex;
										ChangeViewPicture(state, delta, true);
									} else if (ev.keyboard.mappedKey == fplKey_F) {
										fplSetWindowFullscreenSize(!fplIsWindowFullscreen(), 0, 0, 0);
									} else if (ev.keyboard.mappedKey == fplKey_P) {
										state->params.preview = !state->params.preview;
									} else if (ev.keyboard.mappedKey == fplKey_R) {
										ChangeViewPicture(state, 0, true);
									} else if (ev.keyboard.mappedKey == fplKey_L) {
										SwitchActivePictureLoader(state);
									} else if (ev.keyboard.mappedKey == fplKey_B) {
										int backgroundCount = (int)ResampleBackground_Count;
										int nextBackground = ((int)state->background + 1) % backgroundCount;
										state->background = (ResampleBackground)nextBackground;
									}
								}
							}
						} break;

						default:
							break;
					}
				}

				fplWindowSize windowSize = fplZeroInit;
				fplGetWindowSize(&windowSize);
				ViewSize viewportSize = fplStructInit(ViewSize, windowSize.width, windowSize.height);
				bool isViewportChanged = viewportSize.width != lastViewportSize.width || viewportSize.height != lastViewportSize.height;
				lastViewportSize = viewportSize;

				state->viewportSize = viewportSize;
				bool arePicturesChanging = UpdatePictures(state);
				UpdateWindowTitle(state);
				LogResampleTimer(state);

				// Draw only when something changed, otherwise idle, and never into an empty (minimized) viewport
				bool isViewportEmpty = viewportSize.width == 0 || viewportSize.height == 0;
				bool needsFrame = !hasDrawnFrame || hasEvents || isViewportChanged || arePicturesChanging;
				if (needsFrame && !isViewportEmpty) {
					RenderFrame(state, viewportSize);
					fplVideoFlip();
					hasDrawnFrame = true;
				} else {
					fplThreadSleep(IDLE_SLEEP_MILLISECONDS);
				}
			}

			Kill(state);
		} else {
			returnCode = -1;
		}
		for (size_t pictureIndex = 0; pictureIndex < fplArrayCount(state->viewPictures); ++pictureIndex) {
			ResampleResultRelease(&state->viewPictures[pictureIndex].thumbnail);
		}
		ResampleResultRelease(&state->viewResult);
		ResamplePipelineRelease(&state->pipeline);
		fglUnloadOpenGL();
	}
	if (isPlatformInitialized) {
		fplPlatformRelease();
	}

	ImageLoaderRegistryRelease(&state->loaderRegistry);
	fplMemoryFree(state);

	if (icon16Data != fpl_null) {
		stbi_image_free(icon16Data);
	}
	if (icon32Data != fpl_null) {
		stbi_image_free(icon32Data);
	}

	flogWrite("Shutdown %s with code %d", VER_PRODUCTNAME_STR, returnCode);

	return(returnCode);
}