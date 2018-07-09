/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImageViewer

Description:
	Very simple opengl based image viewer.
	Loads up pictures in multiple threads using a lock-free MPMC queue.
	Texture Allocate/Release is done in the main thread.

Requirements:
	- C++ Compiler, just because to support R"()"
	- Final Dynamic OpenGL
	- Final Memory
	- STB_image

Author:
	Torsten Spaete

Changelog:
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
	- Migrate to modern opengl
	- Ansi-Aliasing (MSAA -> Requires change in FPL)
	- Text rendering display current file (Index/Count)
	- Use preload tile as preview
	- Fade in/out
	- Diashow
-------------------------------------------------------------------------------
*/

// 1 = Legacy
// 2 = Legacy, VBO, Shaders
// 3 = Modern, VBO, Shaders
#define USE_GLVERSION 3

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string.h>
#include <malloc.h>

#if defined(FPL_PLATFORM_WIN32)
#define CompareStringIgnoreCase _stricmp
#else
#define CompareStringIgnoreCase strcasecmp
#endif

typedef union Vec2f {
	struct {
		float x;
		float y;
	};
	float m[2];
} Vec2f;

inline Vec2f V2f(const float x, const float y) {
	Vec2f result = FPL_STRUCT_INIT(Vec2f, x, y);
	return(result);
}

typedef union Vec3f {
	struct {
		float x;
		float y;
		float z;
	};
	struct {
		Vec2f xy;
		float unused0;
	};
	struct {
		float unused1;
		Vec2f yz;
	};
	float m[3];
} Vec3f;

inline Vec3f V3f(const float x, const float y, const float z) {
	Vec3f result = FPL_STRUCT_INIT(Vec3f, x, y, z);
	return(result);
}

typedef union Vec4f {
	struct {
		float x;
		float y;
		float z;
		float w;
	};
	struct {
		Vec3f xyz;
		float unused0;
	};
	float m[4];
} Vec4f;

inline Vec4f V4f(const float x, const float y, const float z, const float w) {
	Vec4f result = FPL_STRUCT_INIT(Vec4f, x, y, z, w);
	return(result);
}

typedef union Mat4f {
	struct {
		Vec4f c[4];
	};
	struct {
		float r[4][4];
	};
	float m[16];
} Mat4f;

inline Mat4f M4f(const float value) {
	Mat4f result;
	result.c[0] = V4f(value, 0.0f, 0.0f, 0.0f);
	result.c[1] = V4f(0.0f, value, 0.0f, 0.0f);
	result.c[2] = V4f(0.0f, 0.0f, value, 0.0f);
	result.c[3] = V4f(0.0f, 0.0f, 0.0f, value);
	return(result);
}

inline void SetMat4fOrthoLH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar, Mat4f *outMat) {
	*outMat = M4f(1.0f);
	outMat->r[0][0] = 2.0f / (right - left);
	outMat->r[1][1] = 2.0f / (top - bottom);
	outMat->r[2][2] = 2.0f / (zFar - zNear);
	outMat->r[3][0] = -(right + left) / (right - left);
	outMat->r[3][1] = -(top + bottom) / (top - bottom);
	outMat->r[3][2] = -(zFar + zNear) / (zFar - zNear);
}

inline void SetMat4fTranslation(const float x, const float y, const float z, Mat4f *outMat) {
	*outMat = M4f(1.0f);
	outMat->c[3].xyz = V3f(x, y, z);
}

inline void SetMat4fScale(const float x, const float y, const float z, Mat4f *outMat) {
	*outMat = M4f(1.0f);
	outMat->c[0].x = x;
	outMat->c[1].y = y;
	outMat->c[2].z = z;
}

inline void MultMat4f(const Mat4f *a, const Mat4f *b, Mat4f *r) {
	for (int i = 0; i < 16; i += 4) {
		for (int j = 0; j < 4; ++j) {
			r->m[i + j] =
				(b->m[i + 0] * a->m[j + 0])
				+ (b->m[i + 1] * a->m[j + 4])
				+ (b->m[i + 2] * a->m[j + 8])
				+ (b->m[i + 3] * a->m[j + 12]);
		}
	}
}

typedef struct PictureFile {
	char filePath[FPL_MAX_PATH_LENGTH];
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

#define MAX_FILE_BUFFER_SIZE 4096
typedef struct StreamingFileBuffer {
	fplFileHandle handle;
	size_t size;
} StreamingFileBuffer;

typedef struct ViewPicture {
	StreamingFileBuffer fileStream;
	char filePath[FPL_MAX_PATH_LENGTH];
	uint8_t *data;
	float progress;
	size_t fileIndex;
	uint32_t width;
	uint32_t height;
	uint32_t components;
	GLuint textureId;
	volatile LoadedPictureState state;
} ViewPicture;

typedef struct PictureLoadThread {
	struct ViewerState *state;
	fplMutexHandle mutex;
	fplConditionVariable condition;
	volatile bool shutdown;
} PictureLoadThread;

#define MAX_LOAD_THREAD_COUNT 64
#define MAX_VIEW_PICTURE_COUNT MAX_LOAD_THREAD_COUNT * 4
#define MAX_LOAD_QUEUE_COUNT MAX_VIEW_PICTURE_COUNT * 2
#define PAGE_INCREMENT_COUNT 10

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
	const char *path;
	uint32_t threadCount;
	uint32_t preloadCount;
	bool recursive;
	bool preview;
} ViewerParameters;

typedef struct Vertex {
	Vec4f position;
	Vec2f texCoord;
} Vertex;

typedef struct LoadPictureContext {
	ViewPicture *viewPic;
	volatile bool *shutdown;
} LoadPictureContext;

typedef struct Filter {
	const char *name;
	GLuint programId;
} Filter;

typedef enum FilterType {
	FilterType_Nearest = 0,
	FilterType_Bilinear,
#if USE_GLVERSION >= 2
	FilterType_CubicTriangular,
	FilterType_CubicBell,
	FilterType_CubicBSpline,
	FilterType_CatMullRom,
	FilterType_Lanczos3,
#endif
	FilterType_Count,
} FilterType;

typedef struct ViewerState {
	char rootPath[FPL_MAX_PATH_LENGTH];
	PictureFile *pictureFiles;
	size_t pictureFileCapacity;
	size_t pictureFileCount;
	size_t folderCount;
	int activeFileIndex;

	ViewPicture viewPictures[MAX_VIEW_PICTURE_COUNT];
	size_t viewPicturesCapacity;
	int viewPictureIndex;
	bool doPictureReload;

	PictureLoadThread loadThreadData[MAX_LOAD_THREAD_COUNT];
	fplThreadHandle *loadThreads[MAX_LOAD_THREAD_COUNT];
	size_t loadThreadCount;

	ViewerParameters params;

	LoadQueue loadQueue;
	size_t loadQueueCapacity;

	GLuint vertexArray;
	GLuint colorShaderProgram;

	Filter filters[FilterType_Count];
	size_t activeFilter;

	GLuint quadVBO;
	GLuint quadIBO;
	Vertex quadVertices[4];
	GLushort quadIndices[6];
} ViewerState;

#include "shadersources.h"

static void InitQueue(LoadQueue *queue, const size_t queueCount) {
	FPL_CLEAR_STRUCT(queue);
	queue->size = queueCount;
	queue->mask = queue->size - 1;
	queue->headSeq = queue->tailSeq = 0;
	for (size_t i = 0; i < queue->size; ++i) {
		queue->buffer[i].seq = i;
	}
}

static void ShutdownQueue(LoadQueue *queue) {
	queue->shutdown = 1;
}

static bool TryQueueEnqueue(volatile LoadQueue *queue, const volatile LoadQueueValue value) {
	size_t headSeq = fplAtomicLoadSize(&queue->headSeq);
	while (!queue->shutdown) {
		size_t index = headSeq & queue->mask;
		volatile LoadQueueEntry *entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)headSeq;
		if (dif == 0) {
			if (fplIsAtomicCompareAndExchangeSize(&queue->headSeq, headSeq, headSeq + 1)) {
				fplMemoryCopy((const void *)&value, sizeof(value), (void *)&entry->value);
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

static bool TryQueueDequeue(volatile LoadQueue *queue, volatile LoadQueueValue *value) {
	size_t tailSeq = fplAtomicLoadSize(&queue->tailSeq);
	while (!queue->shutdown) {
		size_t index = tailSeq & queue->mask;
		volatile LoadQueueEntry *entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)(tailSeq + 1);
		if (dif == 0) {
			if (fplIsAtomicCompareAndExchangeSize(&queue->tailSeq, tailSeq, tailSeq + 1)) {
				fplMemoryCopy((const void *)&entry->value, sizeof(*value), (void *)value);
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

static bool IsPictureFile(const char *filePath) {
	const char *ext = fplExtractFileExtension(filePath);
	bool result;
	if (ext != fpl_null) {
		result = (CompareStringIgnoreCase(ext, ".jpg") == 0) || (CompareStringIgnoreCase(ext, ".jpeg") == 0) || (CompareStringIgnoreCase(ext, ".png") == 0) || (CompareStringIgnoreCase(ext, ".bmp") == 0);
	} else {
		result = false;
	}
	return(result);
}

static void ClearPictureFiles(ViewerState *state) {
	if (state->pictureFiles != fpl_null) {
		free(state->pictureFiles);
		state->pictureFiles = fpl_null;
	}
	state->pictureFileCount = 0;
	state->pictureFileCapacity = 0;
	state->rootPath[0] = 0;
	state->folderCount = 0;
}

static void AddPictureFile(ViewerState *state, const char *filePath) {
	FPL_ASSERT(state->pictureFileCount <= state->pictureFileCapacity);
	if (state->pictureFileCapacity == 0) {
		state->pictureFileCapacity = 1;
		state->pictureFiles = (PictureFile *)malloc(sizeof(PictureFile) * state->pictureFileCapacity);
	} else if (state->pictureFileCount == state->pictureFileCapacity) {
		state->pictureFileCapacity *= 2;
		state->pictureFiles = (PictureFile *)realloc(state->pictureFiles, sizeof(PictureFile) * state->pictureFileCapacity);
	}
	PictureFile *pictureFile = &state->pictureFiles[state->pictureFileCount++];
	fplCopyAnsiString(filePath, pictureFile->filePath, FPL_ARRAYCOUNT(pictureFile->filePath));
}

static void AddPicturesFromPath(ViewerState *state, const char *path, const bool recursive) {
	fplFileEntry entry;
	size_t addedPics = 0;
	for (bool hasEntry = fplListDirBegin(path, "*", &entry); hasEntry; hasEntry = fplListDirNext(&entry)) {
		if (!hasEntry) {
			break;
		}
		if (entry.type == fplFileEntryType_File) {
			if (IsPictureFile(entry.fullPath)) {
				AddPictureFile(state, entry.fullPath);
				++addedPics;
			}
		} else if (recursive && entry.type == fplFileEntryType_Directory) {
			AddPicturesFromPath(state, entry.fullPath, true);
		}
	}
	if (addedPics > 0) {
		++state->folderCount;
	}
}

static bool LoadPicturesPath(ViewerState *state, const char *path, const bool recursive) {
	ClearPictureFiles(state);
	FPL_LOG_INFO("ImageViewer", "Loading pictures from path '%s'", path);
	if (fplDirectoryExists(path)) {
		fplCopyAnsiString(path, state->rootPath, FPL_ARRAYCOUNT(state->rootPath));
		AddPicturesFromPath(state, state->rootPath, recursive);
	} else if (fplFileExists(path)) {
		if (IsPictureFile(path)) {
			fplExtractFilePath(path, state->rootPath, FPL_ARRAYCOUNT(state->rootPath));
			state->folderCount = 1;
			AddPictureFile(state, path);
		}
	}
	bool result = (state->pictureFileCount > 0);
	return(result);
}

static void ReleaseTexture(GLuint *target) {
	FPL_ASSERT(*target > 0);
	glDeleteTextures(1, target);
	*target = 0;
}

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const uint8_t components, const void *data, const bool repeatable, const GLint filter) {
	static int internalFormatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA8,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB8,
		/* 4 = RGBA */ GL_RGBA8,
	};
	static int formatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB,
		/* 4 = RGBA */ GL_RGBA,
	};
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_RECTANGLE, handle);
	GLuint internalFormat = internalFormatMapping[components];
	GLenum format = formatMapping[components];
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	return(handle);
}

static void ClearViewPictures(ViewerState *state) {
	for (int i = 0; i < state->viewPicturesCapacity; ++i) {
		state->viewPictures[i].state = LoadedPictureState_Unloaded;
		state->viewPictures[i].progress = 0.0f;
		if (state->viewPictures[i].textureId > 0) {
			ReleaseTexture(&state->viewPictures[i].textureId);
		}
		if (state->viewPictures[i].data != fpl_null) {
			stbi_image_free(state->viewPictures[i].data);
		}
	}
	state->viewPicturesCapacity = 0;
}

static void UpdateStreamProgress(ViewPicture *pic) {
	size_t pos = fplGetFilePosition32(&pic->fileStream.handle);
	if (pic->fileStream.size > 0) {
		pic->progress = pos / (float)pic->fileStream.size;
	}
}

int ReadPictureStreamCallback(void *user, char *data, int size) {
	// fill 'data' with 'size' bytes.  return number of bytes actually read
	LoadPictureContext *ctx = (LoadPictureContext *)user;
	ViewPicture *pic = ctx->viewPic;
	if (*ctx->shutdown) {
		return -1;
	}
	FPL_ASSERT(size >= 0);
	uint32_t readBytes = fplReadFileBlock32(&pic->fileStream.handle, (uint32_t)size, (void *)data, (uint32_t)size);
	UpdateStreamProgress(pic);
	return (int)readBytes;
}
void SkipPictureStreamCallback(void *user, int n) {
	// skip the next 'n' bytes, or 'unget' the last -n bytes if negative
	LoadPictureContext *ctx = (LoadPictureContext *)user;
	if (*ctx->shutdown) {
		return;
	}
	ViewPicture *pic = ctx->viewPic;
	fplSetFilePosition32(&pic->fileStream.handle, n, fplFilePositionMode_Current);
	UpdateStreamProgress(pic);
}
int EofPictureStreamCallback(void *user) {
	// returns nonzero if we are at end of file/data
	LoadPictureContext *ctx = (LoadPictureContext *)user;
	ViewPicture *pic = ctx->viewPic;
	if (*ctx->shutdown) {
		return 1;
	}
	int res = 0;
	size_t pos = fplGetFilePosition32(&pic->fileStream.handle);
	if (pic->fileStream.size == 0 || pos == pic->fileStream.size) {
		res = 1;
	}
	return(res);
}

static void LoadPictureThreadProc(const fplThreadHandle *thread, void *data) {
	PictureLoadThread *loadThread = (PictureLoadThread *)data;
	ViewerState *state = loadThread->state;
	volatile LoadQueueValue valueToLoad = FPL_ZERO_INIT;
	volatile bool hasValue = false;
	while (!loadThread->shutdown) {
		fplConditionWait(&loadThread->condition, &loadThread->mutex, 50);
		if (loadThread->shutdown) {
			break;
		}

		if (!hasValue) {
			if (TryQueueDequeue(&state->loadQueue, &valueToLoad)) {
				hasValue = true;
			}
		}

		if (hasValue) {
			FPL_ASSERT(valueToLoad.fileIndex >= 0 && valueToLoad.fileIndex < state->pictureFileCount);
			FPL_ASSERT(valueToLoad.pictureIndex >= 0 && valueToLoad.pictureIndex < state->viewPicturesCapacity);
			ViewPicture *loadedPic = &state->viewPictures[valueToLoad.pictureIndex];
			const PictureFile *picFile = &state->pictureFiles[valueToLoad.fileIndex];
			if (fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_Discard) {
				continue;
			}
			if (fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_Unloaded) {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_LoadingData);

				loadedPic->progress = 0.0f;
				loadedPic->fileStream.size = 0;

				loadedPic->fileIndex = (size_t)valueToLoad.fileIndex;
				FPL_ASSERT(picFile->filePath != fpl_null);
				fplCopyAnsiString(picFile->filePath, loadedPic->filePath, FPL_ARRAYCOUNT(loadedPic->filePath));
				loadedPic->width = loadedPic->height = 0;
				loadedPic->data = fpl_null;
				loadedPic->components = 0;

				int w = 0, h = 0, comp = 0;
				uint8_t *imageData = fpl_null;

                FPL_LOG_INFO("ImageViewer", "Load picture stream '%s'[%d]", loadedPic->filePath, loadedPic->fileIndex);
				if (fplOpenAnsiBinaryFile(loadedPic->filePath, &loadedPic->fileStream.handle)) {
					loadedPic->fileStream.size = fplGetFileSizeFromHandle32(&loadedPic->fileStream.handle);
					stbi_io_callbacks callbacks;
					callbacks.read = ReadPictureStreamCallback;
					callbacks.skip = SkipPictureStreamCallback;
					callbacks.eof = EofPictureStreamCallback;
					stbi_set_flip_vertically_on_load(0);
					LoadPictureContext loadCtx;
					loadCtx.shutdown = &loadThread->shutdown;
					loadCtx.viewPic = loadedPic;
                    imageData = stbi_load_from_callbacks(&callbacks, &loadCtx, &w, &h, &comp, 4);
					fplCloseFile(&loadedPic->fileStream.handle);
				}
				if (imageData != fpl_null) {
					loadedPic->progress = 0.75f;
					loadedPic->width = (uint32_t)w;
					loadedPic->height = (uint32_t)h;
					loadedPic->components = 4;
					loadedPic->data = imageData;
					fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_ToUpload);
				} else {
					loadedPic->progress = 1.0f;
					fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Error);
				}
			}
			hasValue = false;
		}
	}
}

static void InitLoadThreads(ViewerState *state, const size_t threadCount) {
	state->loadThreadCount = threadCount;
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		fplMutexInit(&state->loadThreadData[i].mutex);
		fplConditionInit(&state->loadThreadData[i].condition);
		state->loadThreadData[i].state = state;
		state->loadThreadData[i].shutdown = false;
		state->loadThreads[i] = fplThreadCreate(LoadPictureThreadProc, &state->loadThreadData[i]);
	}
}

static void ShutdownLoadThreads(ViewerState *state) {
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreadData[i].shutdown = true;
		fplConditionSignal(&state->loadThreadData[i].condition);
	}
	fplThreadWaitForAll(state->loadThreads, state->loadThreadCount, FPL_TIMEOUT_INFINITE);
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		fplConditionDestroy(&state->loadThreadData[i].condition);
		fplMutexDestroy(&state->loadThreadData[i].mutex);
	}
}

static void DiscardAll(ViewerState *state) {
	for (int i = 0; i < state->viewPicturesCapacity; ++i) {
		fplAtomicStoreS32(&state->viewPictures[i].state, LoadedPictureState_Discard);
	}
}

static void QueueUpPictures(ViewerState *state) {
	int capacity = (int)state->viewPicturesCapacity;
	int maxSidePreloadCount = capacity / 2;
	state->viewPictureIndex = maxSidePreloadCount;
	FPL_ASSERT(state->activeFileIndex >= 0 && state->activeFileIndex < (int)state->pictureFileCount);
	int preloadCountLeft;
	int preloadCountRight;
	if (state->activeFileIndex > 0) {
		preloadCountLeft = FPL_MIN(state->activeFileIndex, maxSidePreloadCount);
	} else {
		preloadCountLeft = 0;
	}
	if (state->activeFileIndex < ((int)state->pictureFileCount - 1)) {
		int diff = (int)state->pictureFileCount - state->activeFileIndex;
		preloadCountRight = FPL_MAX(FPL_MIN(diff, maxSidePreloadCount), 0);
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
		if ((state->activeFileIndex + i) < state->pictureFileCount) {
			newValue.fileIndex = state->activeFileIndex + i;
			newValue.pictureIndex = state->viewPictureIndex + i;
			TryQueueEnqueue(&state->loadQueue, newValue);
		}
	}

	// Wakeup load threads
	for (size_t i = 0; i < state->loadThreadCount; ++i) {
		fplConditionSignal(&state->loadThreadData[i].condition);
	}
}

static void UpdateWindowTitle(ViewerState *state) {
	if (state->activeFileIndex > -1) {
		char titleBuffer[256];
		const char *filterName = state->filters[state->activeFilter].name;
		const char *picFilename = fplExtractFileName(state->pictureFiles[state->activeFileIndex].filePath);
		fplFormatAnsiString(titleBuffer, FPL_ARRAYCOUNT(titleBuffer), "FPL ImageViewer - %s [%s]", picFilename, filterName);
		fplSetWindowAnsiTitle(titleBuffer);
	} else {
		fplSetWindowAnsiTitle("FPL ImageViewer - No pictures found");
	}
}

static void ChangeViewPicture(ViewerState *state, const int offset, const bool forceReload) {
	if (state->pictureFileCount == 0) {
		FPL_ASSERT(state->viewPictureIndex == -1);
		FPL_ASSERT(state->activeFileIndex == -1);
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
	state->activeFileIndex = FPL_MAX(FPL_MIN(state->activeFileIndex + offset, (int)state->pictureFileCount - 1), 0);

	UpdateWindowTitle(state);

	if (loadPictures) {
		DiscardAll(state);
		ShutdownQueue(&state->loadQueue);

		// @TODO(final): Wait for threads to stop loading, but dont shutdown the threads!

		InitQueue(&state->loadQueue, state->loadQueueCapacity);
		state->doPictureReload = true;
	}
}

static uint32_t ParseNumber(const char **p) {
	uint32_t v = 0;
	while (isdigit(**p)) {
		v = v * 10 + (uint8_t)(**p - '0');
		++*p;
	}
	return(v);
}

static void ParseParameters(ViewerParameters *params, const int argc, char **argv) {
	FPL_CLEAR_STRUCT(params);
	params->path = fpl_null;
	for (int i = 0; i < argc; ++i) {
		const char *p = argv[i];
		if (p[0] == '-') {
			++p;
			if (!isalpha(*p)) {
				continue;
			}
			const char param = p[0];
			switch (param) {
				case 'p':
					params->preview = true;
					break;
				case 'r':
					params->recursive = true;
					break;
				case 't':
					params->threadCount = 0;
					break;
				default:
					continue;
			}
			if (param == 't') {
				++p;
				if (p[0] == '=') {
					++p;
					params->threadCount = ParseNumber(&p);
				} else {
					continue;
				}
			} else if (param == 'p') {
				++p;
				if (p[0] == '=') {
					++p;
					params->preloadCount = ParseNumber(&p);
				} else {
					continue;
				}
			}
		} else {
			params->path = p;
		}
	}
}

size_t RoundToPowerOfTwo(size_t v) {
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	if (sizeof(size_t) == 8) {
		v |= v >> 32;
	}
	++v;
	return(v);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLuint CreateShaderType(GLenum type, const char *name, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);

	char info[1024 * 10] = FPL_ZERO_INIT;

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		FPL_ASSERT(infoLen <= FPL_ARRAYCOUNT(info));
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		fplDebugFormatOut("Failed compiling '%s' %s shader!\n", name, (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		fplDebugFormatOut("%s\n", info);
		glDeleteShader(shaderId);
		shaderId = 0;
	}

	FPL_ASSERT(shaderId > 0);

	return(shaderId);
}

static GLuint CreateShaderProgram(const char *name, const char *vertexSource, const char *fragmentSource) {
	GLuint programId = glCreateProgram();

	GLuint vertexShader = CreateShaderType(GL_VERTEX_SHADER, name, vertexSource);
	GLuint fragmentShader = CreateShaderType(GL_FRAGMENT_SHADER, name, fragmentSource);

	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glLinkProgram(programId);
	glValidateProgram(programId);

	char info[1024 * 10] = FPL_ZERO_INIT;

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
		FPL_ASSERT(infoLen <= FPL_ARRAYCOUNT(info));
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		fplDebugFormatOut("Failed linking '%s' shader!\n", name);
		fplDebugFormatOut("%s\n", info);
		glDeleteProgram(programId);
		programId = 0;
	}
	FPL_ASSERT(programId > 0);

	return(programId);
}

#define CheckGL(func) ((func), FPL_ASSERT(glGetError() == GL_NO_ERROR))

static void Init(ViewerState *state) {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if USE_GLVERSION < 2
	glClientActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_MODELVIEW);
#endif

	glGenVertexArrays(1, &state->vertexArray);
	glBindVertexArray(state->vertexArray);

	state->colorShaderProgram = CreateShaderProgram("Color", ColorVertexSource, ColorFragmentSource);

	state->activeFilter = FilterType_Nearest;
	state->filters[FilterType_Nearest] = FPL_STRUCT_INIT(Filter, "Nearest", CreateShaderProgram("Nearest", FilterVertexSource, NoFilterFragmentSource));
	state->filters[FilterType_Bilinear] = FPL_STRUCT_INIT(Filter, "Bilinear", CreateShaderProgram("Bilinear", FilterVertexSource, BilinearFilterFragmentSource));
#if USE_GLVERSION >= 2
	state->filters[FilterType_CubicTriangular] = FPL_STRUCT_INIT(Filter, "Bicubic (Triangular)", CreateShaderProgram("Bicubic (Triangular)", FilterVertexSource, BicubicTriangularFilterFragmentSource));
	state->filters[FilterType_CubicBell] = FPL_STRUCT_INIT(Filter, "Bicubic (Bell)", CreateShaderProgram("Bicubic (Bell)", FilterVertexSource, BicubicBellFilterFragmentSource));
	state->filters[FilterType_CubicBSpline] = FPL_STRUCT_INIT(Filter, "Bicubic (B-Spline)", CreateShaderProgram("Bicubic (B-Spline)", FilterVertexSource, BicubicBSplineFilterFragmentSource));
	state->filters[FilterType_CatMullRom] = FPL_STRUCT_INIT(Filter, "Bicubic (CatMull-Rom)", CreateShaderProgram("Bicubic (CatMull-Rom)", FilterVertexSource, BicubicCatMullRowFilterFragmentSource));
	state->filters[FilterType_Lanczos3] = FPL_STRUCT_INIT(Filter, "Lanczos3", CreateShaderProgram("Lanczos3", FilterVertexSource, Lanczos3FilterFragmentSource));
#endif

	state->quadVertices[0] = FPL_STRUCT_INIT(Vertex, V4f(1.0f, 1.0f, 0.0f, 1.0f), V2f(1.0f, 1.0f));
	state->quadVertices[1] = FPL_STRUCT_INIT(Vertex, V4f(-1.0f, 1.0f, 0.0f, 1.0f), V2f(0.0f, 1.0f));
	state->quadVertices[2] = FPL_STRUCT_INIT(Vertex, V4f(-1.0f, -1.0f, 0.0f, 1.0f), V2f(0.0f, 0.0f));
	state->quadVertices[3] = FPL_STRUCT_INIT(Vertex, V4f(1.0f, -1.0f, 0.0f, 1.0f), V2f(1.0f, 0.0f));

	state->quadIndices[0] = 0;
	state->quadIndices[1] = 1;
	state->quadIndices[2] = 2;
	state->quadIndices[3] = 2;
	state->quadIndices[4] = 3;
	state->quadIndices[5] = 0;

	glGenBuffers(1, &state->quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, &state->quadVertices[0].position.x, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &state->quadIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quadIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, &state->quadIndices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	FPL_ASSERT(glGetError() == GL_NO_ERROR);

	state->viewPictureIndex = -1;
	state->activeFileIndex = -1;
	state->doPictureReload = false;

	// Allocate and startup load threads
	size_t threadCount;
	if (state->params.threadCount > 0) {
		threadCount = FPL_MAX(FPL_MIN(state->params.threadCount, MAX_LOAD_THREAD_COUNT), 1);
	} else {
		threadCount = FPL_MAX(FPL_MIN(fplGetProcessorCoreCount(), MAX_LOAD_THREAD_COUNT), 1);
	}
	InitLoadThreads(state, threadCount);

	size_t queueCapacity;
	size_t preloadCapacity;
	if (FPL_IS_POWEROFTWO(threadCount)) {
		queueCapacity = threadCount * 2;
		preloadCapacity = threadCount;
	} else {
		queueCapacity = RoundToPowerOfTwo(threadCount) * 2;
		preloadCapacity = RoundToPowerOfTwo(threadCount);
	}
	state->viewPicturesCapacity = preloadCapacity + 1;
	state->loadQueueCapacity = queueCapacity;

	FPL_ASSERT(FPL_IS_POWEROFTWO(queueCapacity));
	InitQueue(&state->loadQueue, queueCapacity);

	// Load initial pictures from parameters
	if (fplGetAnsiStringLength(state->params.path) > 0) {
		if (LoadPicturesPath(state, state->params.path, state->params.recursive)) {
			state->activeFileIndex = 0;
			ChangeViewPicture(state, 0, true);
		}
	}

	UpdateWindowTitle(state);
}

inline void BuildModelMat(const float centerX, const float centerY, const float scaleX, const float scaleY, Mat4f *modelView) {
	Mat4f modelScale;
	Mat4f modelTranslation;
	SetMat4fScale(scaleX, scaleY, 1.0f, &modelScale);
	SetMat4fTranslation(centerX, centerY, 0.0f, &modelTranslation);
	MultMat4f(&modelTranslation, &modelScale, modelView);
}

static void DrawLinedRectangle(ViewerState *state, const Mat4f *vpMat, const Vec2f pos, const Vec2f ext, const Vec4f color, const float lineWidth) {
#if USE_GLVERSION >= 2
	GLint locVP = glGetUniformLocation(state->colorShaderProgram, "uniVP");
	GLint locModel = glGetUniformLocation(state->colorShaderProgram, "uniModel");
	GLint locColor = glGetUniformLocation(state->colorShaderProgram, "uniColor");

	glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

	glUseProgram(state->colorShaderProgram);

	Mat4f modelMat;

	glUniformMatrix4fv(locVP, 1, GL_FALSE, &vpMat->m[0]);
	glUniform4fv(locColor, 1, &color.m[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quadIBO);

	// Top
	BuildModelMat(pos.x, pos.y + ext.y, ext.x, lineWidth * 0.5f, &modelMat);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat.m[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	// Left
	BuildModelMat(pos.x - ext.x, pos.y, lineWidth * 0.5f, ext.y, &modelMat);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat.m[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	// Bottom
	BuildModelMat(pos.x, pos.y - ext.y, ext.x, lineWidth * 0.5f, &modelMat);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat.m[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	// Right
	BuildModelMat(pos.x + ext.x, pos.y, lineWidth * 0.5f, ext.y, &modelMat);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat.m[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
	glLoadMatrixf(&vpMat->m[0]);
	glColor4fv(&color.m[0]);
	glLineWidth(lineWidth);
	glBegin(GL_LINE_LOOP);
	glVertex2f(pos.x + ext.x, pos.y + ext.y);
	glVertex2f(pos.x - ext.x, pos.y + ext.y);
	glVertex2f(pos.x - ext.x, pos.y - ext.y);
	glVertex2f(pos.x + ext.x, pos.y - ext.y);
	glEnd();
#endif
}

static void DrawSolidRectangle(ViewerState *state, const Mat4f *vpMat, const Mat4f *modelMat, const Vec4f color) {
#if USE_GLVERSION >= 2
	GLint locVP = glGetUniformLocation(state->colorShaderProgram, "uniVP");
	GLint locModel = glGetUniformLocation(state->colorShaderProgram, "uniModel");
	GLint locColor = glGetUniformLocation(state->colorShaderProgram, "uniColor");

	glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

	glUseProgram(state->colorShaderProgram);

	glUniformMatrix4fv(locVP, 1, GL_FALSE, &vpMat->m[0]);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat->m[0]);
	glUniform4fv(locColor, 1, &color.m[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quadIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
	Mat4f mvp;
	MultMat4f(vpMat, modelMat, &mvp);
	glLoadMatrixf(&mvp.m[0]);
	glColor4fv(&color.m[0]);
	glBegin(GL_QUADS);
	glVertex2f(state->quadVertices[0].position.x, state->quadVertices[0].position.y);
	glVertex2f(state->quadVertices[1].position.x, state->quadVertices[1].position.y);
	glVertex2f(state->quadVertices[2].position.x, state->quadVertices[2].position.y);
	glVertex2f(state->quadVertices[3].position.x, state->quadVertices[3].position.y);
	glEnd();
#endif
}

static void DrawTexturedRectangle(ViewerState *state, const GLuint textureId, const GLuint programId, const Mat4f *vpMat, const Mat4f *modelMat, const Vec4f color, const Vec2f texSize) {
#if USE_GLVERSION >= 2
	GLint locVP = glGetUniformLocation(programId, "uniVP");
	GLint locModel = glGetUniformLocation(programId, "uniModel");
	GLint locImage = glGetUniformLocation(programId, "uniImage");
	GLint locColor = glGetUniformLocation(programId, "uniColor");
	GLint locTexSize = glGetUniformLocation(programId, "uniTexSize");

	glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(Vec4f)));

	glBindTexture(GL_TEXTURE_RECTANGLE, textureId);

	glUseProgram(programId);

	glUniformMatrix4fv(locVP, 1, GL_FALSE, &vpMat->m[0]);
	glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat->m[0]);
	glUniform4fv(locColor, 1, &color.m[0]);
	glUniform2fv(locTexSize, 1, &texSize.m[0]);
	glUniform1i(locImage, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quadIBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
	Mat4f mvp;
	MultMat4f(vpMat, modelMat, &mvp);
	glLoadMatrixf(&mvp.m[0]);
	glColor4fv(&color.m[0]);
	glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(state->quadVertices[0].texCoord.x * texSize.x, (1.0f - state->quadVertices[0].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[0].position.x, state->quadVertices[0].position.y);
	glTexCoord2f(state->quadVertices[1].texCoord.x * texSize.x, (1.0f - state->quadVertices[1].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[1].position.x, state->quadVertices[1].position.y);
	glTexCoord2f(state->quadVertices[2].texCoord.x * texSize.x, (1.0f - state->quadVertices[2].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[2].position.x, state->quadVertices[2].position.y);
	glTexCoord2f(state->quadVertices[2].texCoord.x * texSize.x, (1.0f - state->quadVertices[2].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[2].position.x, state->quadVertices[2].position.y);
	glTexCoord2f(state->quadVertices[3].texCoord.x * texSize.x, (1.0f - state->quadVertices[3].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[3].position.x, state->quadVertices[3].position.y);
	glTexCoord2f(state->quadVertices[0].texCoord.x * texSize.x, (1.0f - state->quadVertices[0].texCoord.y) * texSize.y); glVertex2f(state->quadVertices[0].position.x, state->quadVertices[0].position.y);
	glEnd();
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
#endif
}

static void UpdateAndRender(ViewerState *state) {
	// Discard textures on the left/right side when the fileIndex is out of bounds
	if (state->viewPictureIndex != -1) {
		ViewPicture *currentPic = &state->viewPictures[state->viewPictureIndex];
		if (currentPic->fileIndex == 0) {
			for (int i = 0; i < state->viewPictureIndex; ++i) {
				ViewPicture *sidePic = &state->viewPictures[i];
				fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
			}
		} else if (currentPic->fileIndex == state->pictureFileCount - 1) {
			for (int i = state->viewPictureIndex + 1; i < state->viewPicturesCapacity; ++i) {
				ViewPicture *sidePic = &state->viewPictures[i];
				fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
			}
		}
	}

	// Discard or upload textures
	for (int i = 0; i < state->viewPicturesCapacity; ++i) {
		ViewPicture *loadedPic = &state->viewPictures[i];
		if (fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_Discard) {
			if (loadedPic->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&loadedPic->textureId);
			}
			fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Unloaded);
		} else if (fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_ToUpload) {
			if (loadedPic->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&loadedPic->textureId);
			}
			FPL_ASSERT(loadedPic->textureId == 0);
			FPL_ASSERT(loadedPic->data != fpl_null);
			FPL_ASSERT(loadedPic->width > 0 && loadedPic->height > 0);
			FPL_ASSERT(loadedPic->components > 0);
			fplDebugFormatOut("Allocate texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
			GLint texFilter = GL_NEAREST;
			if (state->activeFilter == FilterType_Bilinear) {
#if USE_GLVERSION < 2
				texFilter = GL_LINEAR;
#endif
			}

			loadedPic->textureId = AllocateTexture(loadedPic->width, loadedPic->height, (uint8_t)loadedPic->components, loadedPic->data, false, texFilter);

			stbi_image_free(loadedPic->data);
			loadedPic->data = fpl_null;
			if (loadedPic->textureId > 0) {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Ready);
			} else {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Error);
			}
			loadedPic->progress = 1.0f;
		}
	}

	// Start to queue up pictures to load
	if (state->doPictureReload) {
		QueueUpPictures(state);
		state->doPictureReload = false;
	}

	int w, h;
	fplWindowSize winSize;
	if (fplGetWindowArea(&winSize)) {
		w = winSize.width;
		h = winSize.height;
	} else {
		w = 0;
		h = 0;
	}

	float screenLeft = -(float)w * 0.5f;
	float screenRight = (float)w * 0.5f;
	float screenBottom = -(float)h * 0.5f;
	float screenTop = (float)h * 0.5f;
	float screenW = (float)w;
	float screenH = (float)h;

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, w, h);

	Mat4f proj;
	Mat4f view;
	Mat4f viewProjection;
	SetMat4fOrthoLH(screenLeft, screenRight, screenBottom, screenTop, 0.0f, 1.0f, &proj);
	SetMat4fTranslation(0.0f, 0.0f, 0.0f, &view);
	MultMat4f(&proj, &view, &viewProjection);

	if (state->viewPictureIndex > -1) {
		FPL_ASSERT(state->viewPictureIndex < FPL_ARRAYCOUNT(state->viewPictures));
		ViewPicture *loadedPic = &state->viewPictures[state->viewPictureIndex];
		LoadedPictureState pictureState = fplAtomicLoadS32(&loadedPic->state);
		if (pictureState == LoadedPictureState_Ready) {
			float texW = (float)loadedPic->width;
			float texH = (float)loadedPic->height;
			float aspect = texH > 0 ? texW / texH : 1;
			FPL_ASSERT(aspect != 0);
			float viewWidth;
			float viewHeight;
			float viewX;
			float viewY;
			if (texW > screenW || texH > screenH) {
				float targetHeight = screenW / aspect;
				if (targetHeight > screenH) {
					viewHeight = screenH;
					viewWidth = screenH * aspect;
					viewX = screenLeft + (screenW - viewWidth) * 0.5f;
					viewY = screenBottom;
				} else {
					viewWidth = screenW;
					viewHeight = screenW / aspect;
					viewX = screenLeft;
					viewY = screenBottom + (screenH - viewHeight) * 0.5f;
				}
			} else {
				viewWidth = texW;
				viewHeight = texH;
				viewX = screenLeft + (screenW - viewWidth) * 0.5f;
				viewY = screenBottom + (screenH - viewHeight) * 0.5f;
			}
			float viewLeft = viewX;
			float viewRight = viewX + viewWidth;
			float viewBottom = viewY;
			float viewTop = viewY + viewHeight;
			Mat4f modelMat;
			BuildModelMat(viewLeft + viewWidth * 0.5f, viewBottom + viewHeight * 0.5f, viewWidth * 0.5f, viewHeight * 0.5f, &modelMat);
			Vec4f texColor = V4f(1, 1, 1, 1);
			Vec2f texScale = V2f(texW, texH);
			GLuint filterProgramId = state->filters[state->activeFilter].programId;
			DrawTexturedRectangle(state, loadedPic->textureId, filterProgramId, &viewProjection, &modelMat, texColor, texScale);
		} else if (pictureState == LoadedPictureState_LoadingData) {
			float progressW = screenW * 0.5f;
			float progressH = progressW * 0.1f;
			float progressLeft = screenLeft + (screenW - progressW) * 0.5f;
			float progressRight = progressLeft + progressW;
			float progressTop = screenTop - (screenH - progressH) * 0.5f;
			float progressBottom = progressTop - progressH;
			float percentage = loadedPic->progress;

			Vec2f progressExt = V2f(progressW * 0.5f, progressH * 0.5f);
			Mat4f progressModelMat;
			Mat4f borderModelMat;
			BuildModelMat(progressLeft + progressExt.x * percentage, progressBottom + progressExt.y, progressExt.x * percentage, progressExt.y, &progressModelMat);
			BuildModelMat(progressLeft + progressExt.x, progressBottom + progressExt.y, progressExt.x, progressExt.y, &borderModelMat);
			Vec4f progressColor = V4f(0.25f, 0.25f, 0.25f, 1);
			Vec4f borderColor = V4f(1, 1, 1, 1);
			Vec2f progressCenter = V2f(progressLeft + progressExt.x, progressBottom + progressExt.y);
			float borderLineWidth = 2.0f;
			DrawSolidRectangle(state, &viewProjection, &progressModelMat, progressColor);
			DrawLinedRectangle(state, &viewProjection, progressCenter, progressExt, borderColor, borderLineWidth);
		}
	}

	if (state->params.preview) {
		int blockCount = (int)state->viewPicturesCapacity;
		float maxBlockW = ((FPL_MIN(screenW, screenH)) * 0.5f);
		float blockPadding = (maxBlockW / (float)blockCount) * 0.1f;
		float blockW = ((maxBlockW - ((float)(blockCount - 1) * blockPadding)) / (float)blockCount);
		float blockH = blockW;
		float blocksLeft = -maxBlockW * 0.5f;
		float blocksBottom = (-screenH * 0.5f + blockPadding);
		Vec2f blockExt = V2f(blockW * 0.5f, blockH * 0.5f);
		for (int i = 0; i < blockCount; ++i) {
			float bx = blocksLeft + (float)i * blockW + ((float)i * blockPadding);
			float by = blocksBottom;
			Vec2f blockPos = V2f(bx + blockW * 0.5f, by + blockH * 0.5f);
			Mat4f blockModelMat;
			BuildModelMat(blockPos.x, blockPos.y, blockExt.x, blockExt.y, &blockModelMat);
			ViewPicture *loadedPic = &state->viewPictures[i];
			LoadedPictureState loadState = fplAtomicLoadS32(&loadedPic->state);
			if (loadState != LoadedPictureState_Unloaded) {
				Vec4f color = V4f(0, 0, 0, 0);
				switch (loadState) {
					case LoadedPictureState_LoadingData:
						color = V4f(0, 0, 1, 0.5f);
						break;
					case LoadedPictureState_Ready:
						color = V4f(1, 1, 1, 1);
						break;
					case LoadedPictureState_ToUpload:
						color = V4f(0, 0.5f, 0.5f, 0.5f);
						break;
					case LoadedPictureState_Discard:
						color = V4f(0.75f, 0.25f, 0.0f, 0.5f);
						break;
					case LoadedPictureState_Error:
						color = V4f(1.0, 0.0f, 0.0f, 0.5f);
						break;
					default:
						FPL_ASSERT(!"Invalid loaded picture state!");
						break;
				}

				DrawSolidRectangle(state, &viewProjection, &blockModelMat, color);
				if (loadState == LoadedPictureState_Ready) {
					float texW = (float)loadedPic->width;
					float texH = (float)loadedPic->height;
					Vec2f texScale = V2f(texW, texH);
					GLuint filterProgramId = state->filters[state->activeFilter].programId;
					DrawTexturedRectangle(state, loadedPic->textureId, filterProgramId, &viewProjection, &blockModelMat, color, texScale);
				}
			}

			Vec4f blockColor;
			float blockLineWidth;
			if (i == state->viewPictureIndex) {
				blockLineWidth = 3;
				blockColor = V4f(0, 1, 0, 1);
			} else {
				blockLineWidth = 2;
				if (loadedPic->state == LoadedPictureState_Unloaded) {
					blockColor = V4f(1, 1, 1, 0.2f);
				} else {
					blockColor = V4f(1, 1, 1, 0.5f);
				}
			}
			DrawLinedRectangle(state, &viewProjection, blockPos, blockExt, blockColor, blockLineWidth);
		}
	}

	FPL_ASSERT(glGetError() == GL_NO_ERROR);
}

int main(int argc, char **argv) {
	int returnCode = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	settings.video.isVSync = true;
	settings.video.driver = fplVideoDriverType_OpenGL;
#if USE_GLVERSION >= 3
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
	settings.video.graphics.opengl.multiSamplingCount = 0;
#else
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#endif
	fplCopyAnsiString("FPL Demo - Image Viewer", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));

    fplSetMaxLogLevel(fplLogLevel_All);

	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		if (fglLoadOpenGL(true)) {

			ViewerState state = FPL_ZERO_INIT;
			if (argc >= 2) {
				ParseParameters(&state.params, argc - 1, argv + 1);
			}
			Init(&state);

			fplKey activeKey = fplKey_None;
			uint64_t activeKeyStart = 0;
			const int ActiveKeyThreshold = 150;
			while (fplWindowUpdate()) {
				// Events
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					switch (ev.type) {
						case fplEventType_Keyboard:
						{
							if (ev.keyboard.type == fplKeyboardEventType_Button) {
								if (ev.keyboard.buttonState >= fplButtonState_Press) {
									bool isActiveKeyRepeat;
									if (activeKey != ev.keyboard.mappedKey) {
										activeKey = ev.keyboard.mappedKey;
										activeKeyStart = fplGetTimeInMillisecondsLP();
										isActiveKeyRepeat = false;
									} else {
										isActiveKeyRepeat = (fplGetTimeInMillisecondsLP() - activeKeyStart) >= ActiveKeyThreshold;
									}
									if (ev.keyboard.mappedKey == fplKey_Left) {
										if (activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if (state.activeFileIndex > 0) {
												ChangeViewPicture(&state, -1, false);
											}
										}
									} else if (ev.keyboard.mappedKey == fplKey_Right) {
										if (activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if (state.activeFileIndex < ((int)state.pictureFileCount - 1)) {
												ChangeViewPicture(&state, +1, false);
											}
										}
									}
								} else  {
								    FPL_ASSERT(ev.keyboard.buttonState == fplButtonState_Release);
									activeKey = fplKey_None;
									activeKeyStart = 0;
									if (ev.keyboard.mappedKey == fplKey_Left) {
										if (state.activeFileIndex > 0) {
											ChangeViewPicture(&state, -1, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_Right) {
										if (state.activeFileIndex < ((int)state.pictureFileCount - 1)) {
											ChangeViewPicture(&state, +1, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_PageDown) {
										if (state.activeFileIndex < ((int)state.pictureFileCount - PAGE_INCREMENT_COUNT)) {
											ChangeViewPicture(&state, PAGE_INCREMENT_COUNT, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_PageUp) {
										if (state.activeFileIndex > (PAGE_INCREMENT_COUNT - 1)) {
											ChangeViewPicture(&state, -PAGE_INCREMENT_COUNT, false);
										}
									} else if (ev.keyboard.mappedKey == fplKey_F) {
                                        fplSetWindowFullscreen(!fplIsWindowFullscreen(), 0, 0, 0);
									} else if (ev.keyboard.mappedKey == fplKey_P) {
										state.params.preview = !state.params.preview;
									} else if (ev.keyboard.mappedKey == fplKey_R) {
										ChangeViewPicture(&state, 0, true);
									} else if (ev.keyboard.mappedKey == fplKey_T) {
										state.activeFilter = (state.activeFilter + 1) % FilterType_Count;
										UpdateWindowTitle(&state);
									}
								}
							}
						} break;

						default:
						    break;
					}
				}

				UpdateAndRender(&state);

				fplVideoFlip();
			}

			ShutdownQueue(&state.loadQueue);
			ShutdownLoadThreads(&state);
			ClearPictureFiles(&state);
			ClearViewPictures(&state);

			fglUnloadOpenGL();
		} else {
			returnCode = -1;
		}

		fplPlatformRelease();
	} else {
		returnCode = -1;
	}
	return(returnCode);
}