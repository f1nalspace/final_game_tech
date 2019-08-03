/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImageViewer

Version:
	v0.5.5 (version.h)

Description:
	Very simple opengl based image viewer.
	Loads up pictures in multiple threads using a lock-free MPMC queue.
	Texture Allocate/Release is done in the main thread.
	It supports several image filters, such as Bilinear, Bicubic, Lanczos etc.

Requirements:
	- C++ Compiler :-( Just because to support R"()"
	- Final Platform Layer
	- Final Dynamic OpenGL
	- STB_image

Author:
	Torsten Spaete

Changelog:
	## v0.5.5
	- Reflect api changes in FPL 0.9.4

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
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string.h>

#include "shadersources.h"
#include "imageresources.h"
#include "version.h"

#define FLOG_IMPLEMENTATION
#include "logging.h"

char ToLowerCase(char ch) {
	if(ch >= 'A' && ch <= 'Z') {
		ch = 'a' + (ch - 'A');
	}
	return ch;
}

static int CompareStringIgnoreCase(const char *a, const char *b) {
	while(true) {
		if(!*a && !*b) {
			break;
		} else if(!*a || !*b) {
			return -1;
		}
		char ca = ToLowerCase(*a);
		char cb = ToLowerCase(*b);
		if(ca < cb || ca > cb) {
			return (int)ca - (int)cb;
		}
		++a;
		++b;
	}
	return(0);
}

static int CompareStringLengthIgnoreCase(const char *a, const char *b, const size_t length) {
	size_t count = 0;
	while(true) {
		if(count == length) {
			break;
		} else if(!*a || !*b) {
			return -1;
		}
		char ca = ToLowerCase(*a);
		char cb = ToLowerCase(*b);
		if(ca < cb || ca > cb) {
			return (int)ca - (int)cb;
		}
		++a;
		++b;
		++count;
	}
	return(0);
}

typedef union Vec2f {
	struct {
		float x;
		float y;
	};
	float m[2];
} Vec2f;

inline Vec2f V2f(const float x, const float y) {
	Vec2f result = fplStructInit(Vec2f, x, y);
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
	Vec3f result = fplStructInit(Vec3f, x, y, z);
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
	Vec4f result = fplStructInit(Vec4f, x, y, z, w);
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
	for(int i = 0; i < 16; i += 4) {
		for(int j = 0; j < 4; ++j) {
			r->m[i + j] =
				(b->m[i + 0] * a->m[j + 0])
				+ (b->m[i + 1] * a->m[j + 4])
				+ (b->m[i + 2] * a->m[j + 8])
				+ (b->m[i + 3] * a->m[j + 12]);
		}
	}
}

inline float ScalarLerp(float a, float t, float b) {
	float result = (1.0f - t) * a + t * b;
	return(result);
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

typedef struct LoadPictureContext {
	ViewPicture *viewPic;
	volatile bool canceled;
} LoadPictureContext;

typedef struct PictureLoadThread {
	LoadPictureContext context;
	fplMutexHandle mutex;
	fplConditionVariable condition;
	struct ViewerState *state;
	fplThreadHandle *thread;
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
	bool border;
} ViewerParameters;

typedef struct Vertex {
	Vec4f position;
	Vec2f texCoord;
} Vertex;

typedef struct Filter {
	const char *name;
	GLuint programId;
} Filter;

typedef enum FilterType {
	FilterType_Nearest = 0,
	FilterType_Bilinear,
	FilterType_CubicTriangular,
	FilterType_CubicBell,
	FilterType_CubicBSpline,
	FilterType_CatMullRom,
	FilterType_Lanczos3,
	FilterType_Count,
} FilterType;

typedef enum PictureRequestType {
	PictureRequestType_None = 0,
	PictureRequestType_Change,
	PictureRequestType_Force,
} PictureRequestType;

typedef enum PictureViewFlags {
	PictureViewFlags_None = 0,
	PictureViewFlags_KeepAspectRatio = 1 << 1,
	PictureViewFlags_Upscale = 1 << 2,
} PictureViewFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(PictureViewFlags);

typedef struct SupportedFeatures {
	int openGLMajor;
	bool rectangleTextures;
	bool srgbFrameBuffer;
} SupportedFeatures;

typedef struct ViewerState {
	char rootPath[FPL_MAX_PATH_LENGTH];
	SupportedFeatures features;
	PictureFile *pictureFiles;
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
	PictureViewFlags viewFlags;

	LoadQueue loadQueue;
	size_t loadQueueCapacity;

	GLenum textureTarget;
	GLuint vertexArray;
	GLuint colorShaderProgram;

	Filter filters[FilterType_Count];
	size_t activeFilter;
	size_t filterCount;

	GLuint quadVBO;
	GLuint quadIBO;
	Vertex quadVertices[4];
	GLushort quadIndices[6];
} ViewerState;

static void InitQueue(LoadQueue *queue, const size_t queueCount) {
	fplClearStruct(queue);
	queue->size = queueCount;
	queue->mask = queue->size - 1;
	queue->headSeq = queue->tailSeq = 0;
	for(size_t i = 0; i < queue->size; ++i) {
		queue->buffer[i].seq = i;
	}
}

static void ShutdownQueue(LoadQueue *queue) {
	queue->shutdown = 1;
}

static bool TryQueueEnqueue(volatile LoadQueue *queue, const volatile LoadQueueValue value) {
	size_t headSeq = fplAtomicLoadSize(&queue->headSeq);
	while(!queue->shutdown) {
		size_t index = headSeq & queue->mask;
		volatile LoadQueueEntry *entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)headSeq;
		if(dif == 0) {
			if(fplIsAtomicCompareAndSwapSize(&queue->headSeq, headSeq, headSeq + 1)) {
				fplMemoryCopy((const void *)&value, sizeof(value), (void *)&entry->value);
				fplAtomicStoreSize(&entry->seq, headSeq + 1);
				return(true);
			}
		} else if(dif < 0) {
			return(false);
		} else {
			headSeq = fplAtomicLoadSize(&queue->headSeq);
		}
	}
	return(false);
}

static bool TryQueueDequeue(volatile LoadQueue *queue, volatile LoadQueueValue *value) {
	size_t tailSeq = fplAtomicLoadSize(&queue->tailSeq);
	while(!queue->shutdown) {
		size_t index = tailSeq & queue->mask;
		volatile LoadQueueEntry *entry = &queue->buffer[index];
		size_t entrySeq = fplAtomicLoadSize(&entry->seq);
		intptr_t dif = (intptr_t)entrySeq - (intptr_t)(tailSeq + 1);
		if(dif == 0) {
			if(fplIsAtomicCompareAndSwapSize(&queue->tailSeq, tailSeq, tailSeq + 1)) {
				fplMemoryCopy((const void *)&entry->value, sizeof(*value), (void *)value);
				fplAtomicStoreSize(&entry->seq, tailSeq + queue->mask + 1);
				return(true);
			}
		} else if(dif < 0) {
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
	if(ext != fpl_null) {
		result = (CompareStringIgnoreCase(ext, ".jpg") == 0) || (CompareStringIgnoreCase(ext, ".jpeg") == 0) || (CompareStringIgnoreCase(ext, ".png") == 0) || (CompareStringIgnoreCase(ext, ".bmp") == 0);
	} else {
		result = false;
	}
	return(result);
}

static void ClearPictureFiles(ViewerState *state) {
	if(state->pictureFiles != fpl_null) {
		free(state->pictureFiles);
		state->pictureFiles = fpl_null;
	}
	state->pictureFileCount = 0;
	state->pictureFileCapacity = 0;
	state->rootPath[0] = 0;
	state->folderCount = 0;
}

static void AddPictureFile(ViewerState *state, const char *filePath) {
	fplAssert(state->pictureFileCount <= state->pictureFileCapacity);
	if(state->pictureFileCapacity == 0) {
		state->pictureFileCapacity = 1;
		state->pictureFiles = (PictureFile *)malloc(sizeof(PictureFile) * state->pictureFileCapacity);
	} else if(state->pictureFileCount == state->pictureFileCapacity) {
		state->pictureFileCapacity *= 2;
		state->pictureFiles = (PictureFile *)realloc(state->pictureFiles, sizeof(PictureFile) * state->pictureFileCapacity);
	}
	PictureFile *pictureFile = &state->pictureFiles[state->pictureFileCount++];
	fplCopyString(filePath, pictureFile->filePath, fplArrayCount(pictureFile->filePath));
}

static void AddPicturesFromPath(ViewerState *state, const char *path, const bool recursive) {
	fplFileEntry entry;
	size_t addedPics = 0;
	for(bool hasEntry = fplListDirBegin(path, "*", &entry); hasEntry; hasEntry = fplListDirNext(&entry)) {
		if(!hasEntry) {
			break;
		}
		char fullPath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(fullPath, fplArrayCount(fullPath), 2, path, entry.name);
		if(entry.type == fplFileEntryType_File) {
			if(IsPictureFile(fullPath)) {
				AddPictureFile(state, fullPath);
				++addedPics;
			}
		} else if(recursive && entry.type == fplFileEntryType_Directory) {
			AddPicturesFromPath(state, fullPath, true);
		}
	}
	if(addedPics > 0) {
		++state->folderCount;
	}
}

static void ReleaseTexture(GLuint *target) {
	fplAssert(*target > 0);
	glDeleteTextures(1, target);
	*target = 0;
}

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const uint8_t components, const void *data, const bool repeatable, const GLenum textureTarget, const bool supportsSRGB) {
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
	int sizedInternalFormatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA8,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB8,
		/* 4 = RGBA */ GL_RGBA8,
	};
	int baseInternalFormatMapping[] = {
		/* 0 = */ 0,
		/* 1 = R */ GL_ALPHA,
		/* 2 = RG */ 0,
		/* 3 = RGB */ GL_RGB,
		/* 4 = RGBA */ GL_RGBA,
	};

	if(supportsSRGB) {
		sizedInternalFormatMapping[4] = GL_SRGB8_ALPHA8;
	}

	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(textureTarget, handle);
	GLuint baseInternalFormat = baseInternalFormatMapping[components];
	GLenum sizedInternalFormat = sizedInternalFormatMapping[components];

	glTexImage2D(textureTarget, 0, sizedInternalFormat, width, height, 0, baseInternalFormat, GL_UNSIGNED_BYTE, data);

	glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(textureTarget, 0);

	return(handle);
}

static void ClearViewPictures(ViewerState *state) {
	for(size_t i = 0; i < state->viewPicturesCapacity; ++i) {
		state->viewPictures[i].state = LoadedPictureState_Unloaded;
		state->viewPictures[i].progress = 0.0f;
		if(state->viewPictures[i].textureId > 0) {
			ReleaseTexture(&state->viewPictures[i].textureId);
		}
		if(state->viewPictures[i].data != fpl_null) {
			stbi_image_free(state->viewPictures[i].data);
		}
	}
}

static void UpdateStreamProgress(ViewPicture *pic) {
	size_t pos = fplGetFilePosition32(&pic->fileStream.handle);
	if(pic->fileStream.size > 0) {
		pic->progress = pos / (float)pic->fileStream.size;
	}
}

int ReadPictureStreamCallback(void *user, char *data, int size) {
	// fill 'data' with 'size' bytes.  return number of bytes actually read
	LoadPictureContext *ctx = (LoadPictureContext *)user;
	ViewPicture *pic = ctx->viewPic;
	if(ctx->canceled) {
		return -1;
	}
	fplAssert(size >= 0);
	uint32_t readBytes = fplReadFileBlock32(&pic->fileStream.handle, (uint32_t)size, (void *)data, (uint32_t)size);
	UpdateStreamProgress(pic);
	return (int)readBytes;
}
void SkipPictureStreamCallback(void *user, int n) {
	// skip the next 'n' bytes, or 'unget' the last -n bytes if negative
	LoadPictureContext *ctx = (LoadPictureContext *)user;
	if(ctx->canceled) {
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
	if(ctx->canceled) {
		return 1;
	}
	int res = 0;
	size_t pos = fplGetFilePosition32(&pic->fileStream.handle);
	if(pic->fileStream.size == 0 || pos == pic->fileStream.size) {
		res = 1;
	}
	return(res);
}

static void LoadPictureThreadProc(const fplThreadHandle *thread, void *data) {
	PictureLoadThread *loadThread = (PictureLoadThread *)data;
	ViewerState *state = loadThread->state;
	volatile LoadQueueValue valueToLoad = fplZeroInit;
	volatile bool hasValue = false;
	while(!loadThread->shutdown) {
		fplConditionWait(&loadThread->condition, &loadThread->mutex, 50);
		if(loadThread->shutdown) {
			break;
		}

		if(!hasValue) {
			if(TryQueueDequeue(&state->loadQueue, &valueToLoad)) {
				hasValue = true;
			}
		}

		if(hasValue) {
			fplAssert(valueToLoad.fileIndex >= 0 && valueToLoad.fileIndex < (int)state->pictureFileCount);
			fplAssert(valueToLoad.pictureIndex >= 0 && valueToLoad.pictureIndex < (int)state->viewPicturesCapacity);
			ViewPicture *loadedPic = &state->viewPictures[valueToLoad.pictureIndex];
			const PictureFile *picFile = &state->pictureFiles[valueToLoad.fileIndex];
			LoadedPictureState loadState = fplAtomicLoadS32(&loadedPic->state);
			if(loadState == LoadedPictureState_Discard || loadThread->context.canceled || loadedPic->fileStream.handle.isValid) {
				continue;
			}
			if(loadState == LoadedPictureState_Unloaded || loadState == LoadedPictureState_Error) {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_LoadingData);

				if(loadedPic->data != fpl_null) {
					stbi_image_free(loadedPic->data);
					loadedPic->data = fpl_null;
				}

				fplAssert(!loadedPic->fileStream.handle.isValid);
				fplAssert(loadedPic->data == fpl_null);
				fplAssert(loadedPic->textureId == 0);
				fplAssert(picFile->filePath != fpl_null);

				loadedPic->progress = 0.0f;
				loadedPic->fileStream.size = 0;
				loadedPic->fileIndex = (size_t)valueToLoad.fileIndex;
				fplCopyString(picFile->filePath, loadedPic->filePath, fplArrayCount(loadedPic->filePath));
				loadedPic->width = loadedPic->height = 0;
				loadedPic->components = 0;
				loadThread->context.viewPic = loadedPic;

				int w = 0, h = 0, comp = 0;
				uint8_t *imageData = fpl_null;

				flogWrite("Load picture stream '%s' [%d]", loadedPic->filePath, loadedPic->fileIndex);
				if(fplOpenBinaryFile(loadedPic->filePath, &loadedPic->fileStream.handle)) {
					loadedPic->fileStream.size = fplGetFileSizeFromHandle32(&loadedPic->fileStream.handle);
					stbi_io_callbacks callbacks;
					callbacks.read = ReadPictureStreamCallback;
					callbacks.skip = SkipPictureStreamCallback;
					callbacks.eof = EofPictureStreamCallback;
					stbi_set_flip_vertically_on_load(0);
					imageData = stbi_load_from_callbacks(&callbacks, &loadThread->context, &w, &h, &comp, 4);
					fplCloseFile(&loadedPic->fileStream.handle);
				}

				if(loadThread->shutdown || loadThread->context.canceled) {
					// Loading is canceled
					if(imageData != fpl_null) {
						stbi_image_free(imageData);
						imageData = fpl_null;
					}
				}
				if(imageData != fpl_null) {
					// Loading was successful, mark it as ToUpload
					flogWrite("Successfully loaded picture stream '%s' [%d], Size (%d x %d)", loadedPic->filePath, loadedPic->fileIndex, w, h);
					loadedPic->progress = 0.75f;
					loadedPic->width = (uint32_t)w;
					loadedPic->height = (uint32_t)h;
					loadedPic->components = 4;
					loadedPic->data = imageData;
					fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_ToUpload);
				} else {
					// Failed or canceled loading
					bool isFailed = !(loadThread->shutdown || loadThread->context.canceled);
					flogWrite("%s loaded picture stream '%s' [%d], Size (%d x %d)", (isFailed ? "Failed" : "Canceled"), loadedPic->filePath, loadedPic->fileIndex, w, h);
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
	for(size_t i = 0; i < state->loadThreadCount; ++i) {
		fplMutexInit(&state->loadThreads[i].mutex);
		fplConditionInit(&state->loadThreads[i].condition);
		state->loadThreads[i].state = state;
		state->loadThreads[i].shutdown = false;
		state->loadThreads[i].context.canceled = false;
		state->loadThreads[i].context.viewPic = fpl_null;
		state->loadThreads[i].thread = fplThreadCreate(LoadPictureThreadProc, &state->loadThreads[i]);
	}
}

static void StopLoadingInThreads(ViewerState *state) {
	for(size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].context.canceled = true;
		fplConditionSignal(&state->loadThreads[i].condition);
	}
}

static void ShutdownLoadThreads(ViewerState *state) {
	for(size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].shutdown = true;
		state->loadThreads[i].context.canceled = true;
		fplConditionSignal(&state->loadThreads[i].condition);
	}

	// @FIXME(final): Passing an invalid stride should return a false, instead of hardly crashing or do we?
	//fplThreadWaitForAll(&state->loadThreads[0].thread, state->loadThreadCount, sizeof(fplThreadHandle *), FPL_TIMEOUT_INFINITE);
	
	fplThreadWaitForAll(&state->loadThreads[0].thread, state->loadThreadCount, sizeof(PictureLoadThread), FPL_TIMEOUT_INFINITE);

	for(size_t i = 0; i < state->loadThreadCount; ++i) {
		fplConditionDestroy(&state->loadThreads[i].condition);
		fplMutexDestroy(&state->loadThreads[i].mutex);
	}
}

static void QueueUpPictures(ViewerState *state) {
	// Compute how many pictures we need to preload on the left/right side
	int capacity = (int)state->viewPicturesCapacity;
	int maxSidePreloadCount = capacity / 2;
	state->viewPictureIndex = maxSidePreloadCount;
	fplAssert(state->activeFileIndex >= 0 && state->activeFileIndex < (int)state->pictureFileCount);
	int preloadCountLeft;
	int preloadCountRight;
	if(state->activeFileIndex > 0) {
		preloadCountLeft = fplMin(state->activeFileIndex, maxSidePreloadCount);
	} else {
		preloadCountLeft = 0;
	}
	if(state->activeFileIndex < ((int)state->pictureFileCount - 1)) {
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
	for(int i = 1; i <= preloadCountLeft; ++i) {
		if((state->activeFileIndex - i) >= 0) {
			newValue.fileIndex = state->activeFileIndex - i;
			newValue.pictureIndex = state->viewPictureIndex - i;
			TryQueueEnqueue(&state->loadQueue, newValue);
		}
	}

	// Enqueu pictures from the right side
	for(int i = 1; i <= preloadCountRight; ++i) {
		if((state->activeFileIndex + i) < (int)state->pictureFileCount) {
			newValue.fileIndex = state->activeFileIndex + i;
			newValue.pictureIndex = state->viewPictureIndex + i;
			TryQueueEnqueue(&state->loadQueue, newValue);
		}
	}

	// Wakeup load threads
	for(size_t i = 0; i < state->loadThreadCount; ++i) {
		state->loadThreads[i].context.canceled = false;
		fplConditionSignal(&state->loadThreads[i].condition);
	}
}

static void UpdateWindowTitle(ViewerState *state) {
	char titleBuffer[256];
	if(state->activeFileIndex > -1) {
		const char *filterName = state->filters[state->activeFilter].name;
		const char *picFilename = fplExtractFileName(state->pictureFiles[state->activeFileIndex].filePath);
		fplFormatString(titleBuffer, fplArrayCount(titleBuffer), "%s v%s - %s [%d / %zu] {%s}", VER_PRODUCTNAME_STR, VER_PRODUCTVERSION_STR, picFilename, (state->activeFileIndex + 1), state->pictureFileCount, filterName);
	} else {
		fplFormatString(titleBuffer, fplArrayCount(titleBuffer), "%s v%s - No pictures found", VER_PRODUCTNAME_STR, VER_PRODUCTVERSION_STR);
	}
	fplSetWindowTitle(titleBuffer);
}

static void ChangeViewPicture(ViewerState *state, const int offset, const bool forceReload) {
	if(state->pictureFileCount == 0) {
		fplAssert(state->viewPictureIndex == -1);
		fplAssert(state->activeFileIndex == -1);
		return;
	}
	int capacity = (int)state->viewPicturesCapacity;
	bool loadPictures = false;
	int viewIndex;
	if(state->viewPictureIndex == -1 || forceReload) {
		viewIndex = capacity / 2;
		loadPictures = true;
	} else {
		viewIndex = state->viewPictureIndex + offset;
		if(viewIndex < 0 || viewIndex >= capacity) {
			viewIndex = capacity / 2;
			loadPictures = true;
		}
	}
	state->viewPictureIndex = viewIndex;
	state->activeFileIndex = fplMax(fplMin(state->activeFileIndex + offset, (int)state->pictureFileCount - 1), 0);

	UpdateWindowTitle(state);

	if(loadPictures) {
		state->doPictureReload = true;
		ShutdownQueue(&state->loadQueue);
		StopLoadingInThreads(state);
	}
}

static uint32_t ParseNumber(const char **p) {
	uint32_t v = 0;
	while(isdigit(**p)) {
		v = v * 10 + (uint8_t)(**p - '0');
		++*p;
	}
	return(v);
}

static void ParseParameters(ViewerParameters *params, const int argc, char **argv) {
	fplClearStruct(params);
	params->path = fpl_null;
	for(int i = 0; i < argc; ++i) {
		const char *p = argv[i];
		if(p[0] == '-') {
			++p;
			if(!isalpha(*p)) {
				continue;
			}
			const char param = p[0];
			switch(param) {
				case 'r':
					params->recursive = true;
					break;
				case 't':
					params->threadCount = 0;
					break;
				default:
					continue;
			}
			if(param == 't') {
				++p;
				if(p[0] == '=') {
					++p;
					params->threadCount = ParseNumber(&p);
				} else {
					continue;
				}
			} else if(param == 'p') {
				++p;
				if(p[0] == '=') {
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
#if defined(FPL_CPU_64BIT)
	if(sizeof(size_t) == 8) {
		v |= v >> 32;
	}
#endif
	++v;
	return(v);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLuint CreateShaderType(GLenum type, const char *name, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);

	char info[1024 * 10] = fplZeroInit;

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if(!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		fplAssert(infoLen <= fplArrayCount(info));
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		fplDebugFormatOut("Failed compiling '%s' %s shader!\n", name, (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		fplDebugFormatOut("%s\n", info);
		glDeleteShader(shaderId);
		shaderId = 0;
	}

	fplAssert(shaderId > 0);

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

	char info[1024 * 10] = fplZeroInit;

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if(!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
		fplAssert(infoLen <= fplArrayCount(info));
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		fplDebugFormatOut("Failed linking '%s' shader!\n", name);
		fplDebugFormatOut("%s\n", info);
		glDeleteProgram(programId);
		programId = 0;
	}
	fplAssert(programId > 0);

	return(programId);
}

static void CheckGLError(const char* stmt, const char* fname, int line) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		flogWrite("Error: OpenGL check %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		fplAssert(!"OpenGL Error!");
	}
}

#define CheckGL(stmt) do { \
	(stmt); \
	CheckGLError(#stmt, __FILE__, __LINE__); \
	} while (0)

static void Kill(ViewerState *state) {
	ShutdownQueue(&state->loadQueue);
	ShutdownLoadThreads(state);
	ClearPictureFiles(state);
	ClearViewPictures(state);
}

static void Clear(ViewerState *state) {
	ShutdownQueue(&state->loadQueue);
	StopLoadingInThreads(state);
	ClearPictureFiles(state);
	ClearViewPictures(state);
}

static bool FindPictureIndexByPath(ViewerState *state, const char *path, size_t *outIndex) {
	for(size_t i = 0; i < state->pictureFileCount; ++i) {
		if(fplIsStringEqual(path, state->pictureFiles[i].filePath)) {
			*outIndex = i;
			return(true);
		}
	}
	return(false);
}

static bool LoadPicturesPath(ViewerState *state, const char *path, const bool recursive, size_t *startIndex) {
	bool result = false;
	Clear(state);
	flogWrite("Loading pictures from path '%s'", path);
	if(fplDirectoryExists(path)) {
		fplCopyString(path, state->rootPath, fplArrayCount(state->rootPath));
		AddPicturesFromPath(state, state->rootPath, recursive);
		result = state->pictureFileCount > 0;
		*startIndex = 0;
	} else if(fplFileExists(path)) {
		if(IsPictureFile(path)) {
			fplExtractFilePath(path, state->rootPath, fplArrayCount(state->rootPath));
			AddPicturesFromPath(state, state->rootPath, recursive);
			if(!FindPictureIndexByPath(state, path, startIndex)) {
				startIndex = 0;
			}
			result = true;
		}
	}
	return(result);
}

static bool Init(ViewerState *state) {
	// Query GL version
	state->features.openGLMajor = 1;
	const char *versionStr = (const char *)glGetString(GL_VERSION);
	int versions[2] = fplZeroInit;
	if(versionStr != fpl_null) {
		const char *p = versionStr;
		for(int i = 0; i < 2; ++i) {
			const char *digitStart = p;
			int value = 0;
			while(isdigit(*p)) {
				int part = (int)(*p - '0');
				value = value * 10 + part;
				++p;
			}
			versions[i] = value;
			if(*p != '.' && *p != '-') break;
			++p;
		}
		state->features.openGLMajor = versions[0];
	}

	state->features.rectangleTextures = false;
	state->features.srgbFrameBuffer = false;

	if(state->features.openGLMajor >= 3) {
		GLint extensionCount = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
		for(int i = 0; i < extensionCount; ++i) {
			const char *extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
			if(CompareStringIgnoreCase("GL_ARB_framebuffer_sRGB", extension) == 0) {
				state->features.srgbFrameBuffer = true;
			} else if(CompareStringIgnoreCase("GL_ARB_texture_rectangle", extension) == 0) {
				state->features.rectangleTextures = true;
			}
		}
	} else {
		const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
		const char *p = extensions;
		const char *start = p;
		while(true) {
			if(*p == ' ' || *p == 0) {
				if(CompareStringLengthIgnoreCase(start, "GL_ARB_framebuffer_sRGB", fplGetStringLength("GL_ARB_framebuffer_sRGB")) == 0) {
					state->features.srgbFrameBuffer = true;
				} else if(CompareStringLengthIgnoreCase(start, "GL_ARB_texture_rectangle", fplGetStringLength("GL_ARB_texture_rectangle")) == 0) {
					state->features.rectangleTextures = true;
				}
				start = p + 1;
			}
			if(*p == 0) {
				break;
			}
			++p;
		}
	}

	if(state->features.rectangleTextures) {
		state->textureTarget = GL_TEXTURE_RECTANGLE;
	} else {
		state->textureTarget = GL_TEXTURE_2D;
	}

	glClearColor(0, 0, 0, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const char *samplerType = state->textureTarget == GL_TEXTURE_2D ? "sampler2D" : "sampler2DRect";

	if(state->features.openGLMajor < 2) {
		glMatrixMode(GL_MODELVIEW);
		state->filterCount = 0;
		state->filters[state->filterCount++] = fplStructInit(Filter, "Nearest", 0);
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bilinear", 0);
		state->activeFilter = state->filterCount - 1;
	} else {
		if(state->features.srgbFrameBuffer) {
			glEnable(GL_FRAMEBUFFER_SRGB);
		}

		glGenVertexArrays(1, &state->vertexArray);
		glBindVertexArray(state->vertexArray);

		state->colorShaderProgram = CreateShaderProgram("Color", ColorVertexSource, ColorFragmentSource);

		state->filterCount = 0;
		state->filters[state->filterCount++] = fplStructInit(Filter, "Nearest", CreateShaderProgram("Nearest", FilterVertexSource, NoFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bilinear", CreateShaderProgram("Bilinear", FilterVertexSource, BilinearFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bicubic (Triangular)", CreateShaderProgram("Bicubic (Triangular)", FilterVertexSource, BicubicTriangularFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bicubic (Bell)", CreateShaderProgram("Bicubic (Bell)", FilterVertexSource, BicubicBellFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bicubic (B-Spline)", CreateShaderProgram("Bicubic (B-Spline)", FilterVertexSource, BicubicBSplineFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Bicubic (CatMull-Rom)", CreateShaderProgram("Bicubic (CatMull-Rom)", FilterVertexSource, BicubicCatMullRowFilterFragmentSource(samplerType).c_str()));
		state->filters[state->filterCount++] = fplStructInit(Filter, "Lanczos3", CreateShaderProgram("Lanczos3", FilterVertexSource, Lanczos3FilterFragmentSource(samplerType).c_str()));
		state->activeFilter = state->filterCount - 1;

		CheckGL(true);

		state->quadVertices[0] = fplStructInit(Vertex, V4f(1.0f, 1.0f, 0.0f, 1.0f), V2f(1.0f, 1.0f));
		state->quadVertices[1] = fplStructInit(Vertex, V4f(-1.0f, 1.0f, 0.0f, 1.0f), V2f(0.0f, 1.0f));
		state->quadVertices[2] = fplStructInit(Vertex, V4f(-1.0f, -1.0f, 0.0f, 1.0f), V2f(0.0f, 0.0f));
		state->quadVertices[3] = fplStructInit(Vertex, V4f(1.0f, -1.0f, 0.0f, 1.0f), V2f(1.0f, 0.0f));

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

		CheckGL(0);
	}

	state->viewPictureIndex = -1;
	state->activeFileIndex = -1;
	state->doPictureReload = false;

	// Allocate and startup load threads
	size_t threadCount;
	if(state->params.threadCount > 0) {
		threadCount = fplMax(fplMin(state->params.threadCount, MAX_LOAD_THREAD_COUNT), 1);
	} else {
		threadCount = fplMax(fplMin(fplGetProcessorCoreCount(), MAX_LOAD_THREAD_COUNT), 1);
	}
	InitLoadThreads(state, threadCount);

	size_t preloadCapacity;
	if(state->params.preloadCount > 0) {
		preloadCapacity = state->params.preloadCount;
		if((state->params.preloadCount % 2) != 0) {
			state->params.preloadCount++;
		}
	} else {
		preloadCapacity = 16;
	}
	size_t queueCapacity = RoundToPowerOfTwo((preloadCapacity + 1) * 2);
	state->viewPicturesCapacity = preloadCapacity + 1;
	state->loadQueueCapacity = queueCapacity;

	fplAssert(fplIsPowerOfTwo(queueCapacity));
	InitQueue(&state->loadQueue, queueCapacity);

	// Load initial pictures from parameters
	if(fplGetStringLength(state->params.path) > 0) {
		size_t startPicIndex = 0;
		if(LoadPicturesPath(state, state->params.path, state->params.recursive, &startPicIndex)) {
			state->activeFileIndex = (int)startPicIndex;
			ChangeViewPicture(state, 0, true);
		}
	}

	state->viewFlags = PictureViewFlags_KeepAspectRatio;

	UpdateWindowTitle(state);

	return(true);
}

inline void BuildModelMat(const float centerX, const float centerY, const float scaleX, const float scaleY, Mat4f *modelView) {
	Mat4f modelScale;
	Mat4f modelTranslation;
	SetMat4fScale(scaleX, scaleY, 1.0f, &modelScale);
	SetMat4fTranslation(centerX, centerY, 0.0f, &modelTranslation);
	MultMat4f(&modelTranslation, &modelScale, modelView);
}

static void DrawLinedRectangle(ViewerState *state, const Mat4f *vpMat, const Vec2f pos, const Vec2f ext, const Vec4f color, const float lineWidth) {
	if(state->features.openGLMajor >= 2) {
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
	} else {
		glLoadMatrixf(&vpMat->m[0]);
		glColor4fv(&color.m[0]);
		glLineWidth(lineWidth);
		glBegin(GL_LINE_LOOP);
		glVertex2f(pos.x + ext.x, pos.y + ext.y);
		glVertex2f(pos.x - ext.x, pos.y + ext.y);
		glVertex2f(pos.x - ext.x, pos.y - ext.y);
		glVertex2f(pos.x + ext.x, pos.y - ext.y);
		glEnd();
	}
}

static void DrawSolidRectangle(ViewerState *state, const Mat4f *vpMat, const Mat4f *modelMat, const Vec4f color) {
	if(state->features.openGLMajor >= 2) {
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
	} else {
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
	}
}

static void DrawTexturedRectangle(ViewerState *state, const GLuint textureId, const GLenum textureTarget, const GLuint programId, const Mat4f *vpMat, const Mat4f *modelMat, const Vec4f color, const Vec2f texSize, const Vec2f texScale) {
	if(state->features.openGLMajor >= 2) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(textureTarget, textureId);
		glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		GLint locVP = glGetUniformLocation(programId, "uniVP");
		GLint locModel = glGetUniformLocation(programId, "uniModel");
		GLint locImage = glGetUniformLocation(programId, "uniImage");
		GLint locColor = glGetUniformLocation(programId, "uniColor");
		GLint locTexSize = glGetUniformLocation(programId, "uniTexSize");
		GLint locTexScale = glGetUniformLocation(programId, "uniTexScale");

		glBindBuffer(GL_ARRAY_BUFFER, state->quadVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(Vec4f)));

		glUseProgram(programId);

		glUniformMatrix4fv(locVP, 1, GL_FALSE, &vpMat->m[0]);
		glUniformMatrix4fv(locModel, 1, GL_FALSE, &modelMat->m[0]);
		glUniform4fv(locColor, 1, &color.m[0]);
		glUniform2fv(locTexSize, 1, &texSize.m[0]);
		glUniform2fv(locTexScale, 1, &texScale.m[0]);
		glUniform1i(locImage, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quadIBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glUseProgram(0);

		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindTexture(textureTarget, 0);
	} else {
		glEnable(textureTarget);
		glBindTexture(textureTarget, textureId);
		if(state->activeFilter == FilterType_Nearest) {
			glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		Mat4f mvp;
		MultMat4f(vpMat, modelMat, &mvp);
		glLoadMatrixf(&mvp.m[0]);
		glColor4fv(&color.m[0]);
		glBegin(GL_TRIANGLES);
		glTexCoord2f(state->quadVertices[0].texCoord.x * texScale.x, (1.0f - state->quadVertices[0].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[0].position.x, state->quadVertices[0].position.y);
		glTexCoord2f(state->quadVertices[1].texCoord.x * texScale.x, (1.0f - state->quadVertices[1].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[1].position.x, state->quadVertices[1].position.y);
		glTexCoord2f(state->quadVertices[2].texCoord.x * texScale.x, (1.0f - state->quadVertices[2].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[2].position.x, state->quadVertices[2].position.y);
		glTexCoord2f(state->quadVertices[2].texCoord.x * texScale.x, (1.0f - state->quadVertices[2].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[2].position.x, state->quadVertices[2].position.y);
		glTexCoord2f(state->quadVertices[3].texCoord.x * texScale.x, (1.0f - state->quadVertices[3].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[3].position.x, state->quadVertices[3].position.y);
		glTexCoord2f(state->quadVertices[0].texCoord.x * texScale.x, (1.0f - state->quadVertices[0].texCoord.y) * texScale.y); glVertex2f(state->quadVertices[0].position.x, state->quadVertices[0].position.y);
		glEnd();
		glBindTexture(textureTarget, 0);
		glDisable(textureTarget);
	}

	fplAssert(glGetError() == GL_NO_ERROR);
}

static void UpdateAndRender(ViewerState *state, const float deltaTime) {
	// Discard textures on the left/right side when the fileIndex is out of bounds
	if(state->viewPictureIndex != -1) {
		ViewPicture *currentPic = &state->viewPictures[state->viewPictureIndex];
		if(currentPic->fileIndex == 0) {
			for(int i = 0; i < state->viewPictureIndex; ++i) {
				ViewPicture *sidePic = &state->viewPictures[i];
				if(sidePic->state == LoadedPictureState_Ready) {
					fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
				}
			}
		} else if(currentPic->fileIndex == state->pictureFileCount - 1) {
			for(int i = state->viewPictureIndex + 1; i < (int)state->viewPicturesCapacity; ++i) {
				ViewPicture *sidePic = &state->viewPictures[i];
				if(sidePic->state == LoadedPictureState_Ready) {
					fplAtomicStoreS32(&sidePic->state, LoadedPictureState_Discard);
				}
			}
		}
	}

	// Discard or upload textures
	for(size_t i = 0; i < state->viewPicturesCapacity; ++i) {
		ViewPicture *loadedPic = &state->viewPictures[i];
		if(fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_Discard) {
			if(loadedPic->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&loadedPic->textureId);
			}
			fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Unloaded);
		} else if(fplAtomicLoadS32(&loadedPic->state) == LoadedPictureState_ToUpload) {
			if(loadedPic->textureId > 0) {
				fplDebugFormatOut("Release texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
				ReleaseTexture(&loadedPic->textureId);
			}
			fplAssert(loadedPic->textureId == 0);
			fplAssert(loadedPic->data != fpl_null);
			fplAssert(loadedPic->width > 0 && loadedPic->height > 0);
			fplAssert(loadedPic->components > 0);
			fplDebugFormatOut("Allocate texture '%s'[%d]\n", loadedPic->filePath, loadedPic->fileIndex);
			loadedPic->textureId = AllocateTexture(loadedPic->width, loadedPic->height, (uint8_t)loadedPic->components, loadedPic->data, false, state->textureTarget, state->features.srgbFrameBuffer);
			stbi_image_free(loadedPic->data);
			loadedPic->data = fpl_null;
			if(loadedPic->textureId > 0) {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Ready);
			} else {
				fplAtomicStoreS32(&loadedPic->state, LoadedPictureState_Error);
			}
			loadedPic->progress = 1.0f;
		}
	}
	fplAssert(glGetError() == GL_NO_ERROR);

	// Start to queue up pictures to load
	if(state->doPictureReload) {
		size_t notUnloadedCount = 0;
		for(size_t i = 0; i < state->viewPicturesCapacity; ++i) {
			ViewPicture *viewPic = &state->viewPictures[i];
			LoadedPictureState loadState = fplAtomicLoadS32(&viewPic->state);
			if(loadState != LoadedPictureState_Unloaded) {
				if(loadState != LoadedPictureState_Error) {
					fplAtomicStoreS32(&viewPic->state, LoadedPictureState_Discard);
					++notUnloadedCount;
				}
			}
		}
		if(notUnloadedCount == 0) {
			InitQueue(&state->loadQueue, state->loadQueueCapacity);
			QueueUpPictures(state);
			state->doPictureReload = false;
		}
	}

	int w, h;
	fplWindowSize winSize;
	if(fplGetWindowSize(&winSize)) {
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

	Mat4f view;
	BuildModelMat(0.0f, 0.0f, 1.0f, 1.0f, &view);
	Mat4f proj;
	SetMat4fOrthoLH(screenLeft, screenRight, screenBottom, screenTop, 0.0f, 1.0f, &proj);
	Mat4f viewProjection;
	MultMat4f(&proj, &view, &viewProjection);

	float pictureScale = 1.0f;
	int pictureFrameSideCount = 0;

	float targetRectWidth = screenW * pictureScale;
	float targetRectHeight = screenH * pictureScale;
	float targetRectLeft = screenLeft + (screenW - targetRectWidth) * 0.5f;
	float targetRectBottom = screenBottom + (screenH - targetRectHeight) * 0.5f;
	float pictureFrameSpacing = 10;

	if(state->pictureFileCount > 0 && state->viewPictureIndex > -1 && state->viewPictureIndex < (int)state->viewPicturesCapacity) {
		int framePictureStart = fplMin(-pictureFrameSideCount, 0);
		int framePictureEnd = (framePictureStart + 1 + pictureFrameSideCount);
		for(int framePictureOffset = framePictureStart; framePictureOffset < (framePictureEnd + 1); ++framePictureOffset) {
			if((state->viewPictureIndex + framePictureOffset) < 0 || (state->viewPictureIndex + framePictureOffset) > ((int)state->viewPicturesCapacity - 1)) {
				continue;
			}
			ViewPicture *loadedPic = &state->viewPictures[state->viewPictureIndex + framePictureOffset];
			LoadedPictureState pictureState = fplAtomicLoadS32(&loadedPic->state);
			if(pictureState == LoadedPictureState_Unloaded) {
				continue;
			}

			float targetOpacity = 1.0f;
			float targetRectX = targetRectLeft + (targetRectWidth * (float)framePictureOffset) + (pictureFrameSpacing * (float)framePictureOffset);
			float targetRectY = targetRectBottom;

			if(pictureState == LoadedPictureState_Ready) {
				float texW = (float)loadedPic->width;
				float texH = (float)loadedPic->height;
				float viewWidth;
				float viewHeight;
				float viewX;
				float viewY;
				if((state->viewFlags & PictureViewFlags_KeepAspectRatio) == PictureViewFlags_KeepAspectRatio) {
					float aspect = texH > 0 ? texW / texH : 1;
					fplAssert(aspect != 0);
					float targetHeight = targetRectWidth / aspect;
					if((texW > targetRectWidth || texH > targetRectHeight) || ((state->viewFlags & PictureViewFlags_Upscale) == PictureViewFlags_Upscale)) {
						// Upscaling
						if(targetHeight > targetRectHeight) {
							viewHeight = targetRectHeight;
							viewWidth = targetRectHeight * aspect;
							viewX = targetRectX + (targetRectWidth - viewWidth) * 0.5f;
							viewY = targetRectY;
						} else {
							viewWidth = targetRectWidth;
							viewHeight = targetRectWidth / aspect;
							viewX = targetRectX;
							viewY = targetRectY + (targetRectHeight - viewHeight) * 0.5f;
						}
					} else {
						// Downscaling
						viewWidth = texW;
						viewHeight = texH;
						viewX = targetRectX + (targetRectWidth - viewWidth) * 0.5f;
						viewY = targetRectY + (targetRectHeight - viewHeight) * 0.5f;
					}
				} else {
					viewWidth = targetRectWidth;
					viewHeight = targetRectHeight;
					viewX = targetRectX;
					viewY = targetRectY;
				}
				float viewLeft = viewX;
				float viewRight = viewX + viewWidth;
				float viewBottom = viewY;
				float viewTop = viewY + viewHeight;
				Mat4f modelMat;
				BuildModelMat(viewLeft + viewWidth * 0.5f, viewBottom + viewHeight * 0.5f, viewWidth * 0.5f, viewHeight * 0.5f, &modelMat);
				Vec4f texColor = V4f(1, 1, 1, targetOpacity);
				Vec2f texSize = V2f(texW, texH);
				Vec2f texScale = state->features.rectangleTextures ? texSize : V2f(1.0f, 1.0f);
				GLuint filterProgramId = state->filters[state->activeFilter].programId;
				DrawTexturedRectangle(state, loadedPic->textureId, state->textureTarget, filterProgramId, &viewProjection, &modelMat, texColor, texSize, texScale);

			} else if(pictureState == LoadedPictureState_LoadingData) {
				float progressPadding = 4;
				float progressAspect = 400.0f / 10.0f;
				float progressW = targetRectWidth * 0.5f;
				float progressH = progressW / progressAspect;
				float progressLeft = targetRectX + (targetRectWidth - progressW) * 0.5f;
				float progressBottom = targetRectY + targetRectHeight - progressH - progressPadding;
				float percentage = loadedPic->progress;
				Vec2f progressExt = V2f(progressW * 0.5f, progressH * 0.5f);
				Mat4f progressModelMat;
				Mat4f borderModelMat;
				BuildModelMat(progressLeft + progressExt.x * percentage, progressBottom + progressExt.y, progressExt.x * percentage, progressExt.y, &progressModelMat);
				BuildModelMat(progressLeft + progressExt.x, progressBottom + progressExt.y, progressExt.x, progressExt.y, &borderModelMat);
				Vec4f progressColor = V4f(0.25f, 0.25f, 0.25f, targetOpacity);
				Vec4f borderColor = V4f(1, 1, 1, targetOpacity);
				Vec2f progressCenter = V2f(progressLeft + progressExt.x, progressBottom + progressExt.y);
				float borderLineWidth = 2.0f;
				DrawSolidRectangle(state, &viewProjection, &progressModelMat, progressColor);
				DrawLinedRectangle(state, &viewProjection, progressCenter, progressExt, borderColor, borderLineWidth);
			}
			if(state->params.border) {
				Vec4f frameBorderColor = V4f(1.0f, 1.0f, 1.0f, targetOpacity);
				float frameBorderWidth = 1.0f;
				Vec2f targetRectExt = V2f(targetRectWidth * 0.5f + frameBorderWidth * 2.0f, targetRectHeight * 0.5f + frameBorderWidth * 2.0f);
				DrawLinedRectangle(state, &viewProjection, V2f(targetRectX + targetRectWidth * 0.5f, targetRectY + targetRectHeight * 0.5f), targetRectExt, frameBorderColor, frameBorderWidth);
			}
		}
	}

	if(state->params.preview && state->viewPicturesCapacity > 1 && state->pictureFileCount) {
		int blockCount = (int)state->viewPicturesCapacity;
		float maxBlockW = ((fplMin(screenW, screenH)) * 0.75f);
		float blockPadding = 4;
		float blockW = ((maxBlockW - ((float)(blockCount - 1) * blockPadding)) / (float)blockCount);
		float blockH = blockW;
		float blocksLeft = -maxBlockW * 0.5f;
		float blocksBottom = (-screenH * 0.5f + blockPadding);
		Vec2f blockExt = V2f(blockW * 0.5f, blockH * 0.5f);
		for(int i = 0; i < blockCount; ++i) {
			ViewPicture *loadedPic = &state->viewPictures[i];
			float bx = blocksLeft + (float)i * blockW + ((float)i * blockPadding);
			float by = blocksBottom;
			Vec2f blockPos = V2f(bx + blockW * 0.5f, by + blockH * 0.5f);
			Mat4f blockModelMat;
			BuildModelMat(blockPos.x, blockPos.y, blockExt.x * loadedPic->progress, blockExt.y * loadedPic->progress, &blockModelMat);
			LoadedPictureState loadState = fplAtomicLoadS32(&loadedPic->state);
			if(loadState != LoadedPictureState_Unloaded) {
				Vec4f color = V4f(0, 0, 0, 0);
				switch(loadState) {
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
						fplAssert(!"Invalid loaded picture state!");
						break;
				}

				DrawSolidRectangle(state, &viewProjection, &blockModelMat, color);
				if(loadState == LoadedPictureState_Ready) {
					float texW = (float)loadedPic->width;
					float texH = (float)loadedPic->height;
					Vec2f texSize = V2f(texW, texH);
					Vec2f texScale = state->features.rectangleTextures ? texSize : V2f(1.0f, 1.0f);
					GLuint filterProgramId = state->filters[state->activeFilter].programId;
					DrawTexturedRectangle(state, loadedPic->textureId, state->textureTarget, filterProgramId, &viewProjection, &blockModelMat, color, texSize, texScale);
				}
			}

			Vec4f blockColor;
			float blockLineWidth;
			if(i == state->viewPictureIndex) {
				blockLineWidth = 2;
				blockColor = V4f(0, 1, 0, 1);
			} else {
				blockLineWidth = 1;
				if(loadedPic->state == LoadedPictureState_Unloaded) {
					blockColor = V4f(1, 1, 1, 0.2f);
				} else {
					blockColor = V4f(1, 1, 1, 0.5f);
				}
			}
			DrawLinedRectangle(state, &viewProjection, blockPos, blockExt, blockColor, blockLineWidth);
		}
	}

	fplAssert(glGetError() == GL_NO_ERROR);
}

static void LogCallbackFunc(const char *funcName, const int lineNumber, fplLogLevel level, const char *message) {
	flogWrite(message);
}

int main(int argc, char **argv) {
	// Initialize logging
	if(fplPlatformInit(fplInitFlags_None, fpl_null)) {
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

	ViewerState state = fplZeroInit;
	state.params.preview = true;
	if(argc >= 2) {
		ParseParameters(&state.params, argc - 1, argv + 1);
	}

	flogWrite("Initial Parameters:");
	flogWrite("Path: %s", state.params.path);
	flogWrite("Preload count: %lu", state.params.preloadCount);
	flogWrite("Thread count: %lu", state.params.threadCount);
	flogWrite("Preview enabled: %s", (state.params.preview ? "yes" : "no"));
	flogWrite("Recursive enabled: %s", (state.params.recursive ? "yes" : "no"));

	int returnCode = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	settings.video.isVSync = true;
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
	settings.video.graphics.opengl.multiSamplingCount = 4;
	fplCopyString("FPL Demo - Image Viewer", settings.window.title, fplArrayCount(settings.window.title));

	// Load icons (Memory are released on shutdown)
	int iconW, iconH, iconC;
	uint8_t *icon16Data = stbi_load_from_memory(icon16DataArray, icon16DataArraySize, &iconW, &iconH, &iconC, 4);
	if(icon16Data != fpl_null) {
		settings.window.icons[0].data = icon16Data;
		settings.window.icons[0].width = iconW;
		settings.window.icons[0].height = iconH;
		settings.window.icons[0].type = fplImageType_RGBA;
	}
	iconW = iconH = iconC = 0;
	uint8_t *icon32Data = stbi_load_from_memory(icon32DataArray, icon32DataArraySize, &iconW, &iconH, &iconC, 4);
	if(icon32Data != fpl_null) {
		settings.window.icons[1].data = icon32Data;
		settings.window.icons[1].width = iconW;
		settings.window.icons[1].height = iconH;
		settings.window.icons[1].type = fplImageType_RGBA;
	}

	if(fplPlatformInit(fplInitFlags_Video, &settings)) {
		if(fglLoadOpenGL(true) && Init(&state)) {
			fplKey activeKey = fplKey_None;
			uint64_t activeKeyStart = 0;
			const int ActiveKeyThreshold = 150;
			const float deltaTime = 1.0f / 60.0f;
			while(fplWindowUpdate()) {
				// Events
				fplEvent ev;
				while(fplPollEvent(&ev)) {
					switch(ev.type) {
						case fplEventType_Window:
						{
							if(ev.window.type == fplWindowEventType_DroppedFiles) {
								size_t startPicIndex = 0;
								for (size_t fileIndex = 0; fileIndex < ev.window.dropFiles.fileCount; ++fileIndex) {
									const char *filePath = ev.window.dropFiles.files[fileIndex];
									// @TODO(final): LoadPicturesPath clears the picture files always, so we basically can only load one folder at a time
									if (LoadPicturesPath(&state, filePath, false, &startPicIndex)) {
										state.activeFileIndex = (int)startPicIndex;
										ChangeViewPicture(&state, 0, true);
									}
								}
							}
						} break;

						case fplEventType_Keyboard:
						{
							if(ev.keyboard.type == fplKeyboardEventType_Button) {
								if(ev.keyboard.buttonState >= fplButtonState_Press) {
									bool isActiveKeyRepeat;
									if(activeKey != ev.keyboard.mappedKey) {
										activeKey = ev.keyboard.mappedKey;
										activeKeyStart = fplGetTimeInMillisecondsLP();
										isActiveKeyRepeat = false;
									} else {
										isActiveKeyRepeat = (fplGetTimeInMillisecondsLP() - activeKeyStart) >= ActiveKeyThreshold;
									}
									if(ev.keyboard.mappedKey == fplKey_Left) {
										if(activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if(state.activeFileIndex > 0) {
												ChangeViewPicture(&state, -1, false);
											}
										}
									} else if(ev.keyboard.mappedKey == fplKey_Right) {
										if(activeKey == ev.keyboard.mappedKey && isActiveKeyRepeat) {
											if(state.activeFileIndex < ((int)state.pictureFileCount - 1)) {
												ChangeViewPicture(&state, +1, false);
											}
										}
									}
								} else {
									fplAssert(ev.keyboard.buttonState == fplButtonState_Release);
									activeKey = fplKey_None;
									activeKeyStart = 0;
									if(ev.keyboard.mappedKey == fplKey_Left) {
										if(state.activeFileIndex > 0) {
											ChangeViewPicture(&state, -1, false);
										}
									} else if(ev.keyboard.mappedKey == fplKey_Right) {
										if(state.activeFileIndex < ((int)state.pictureFileCount - 1)) {
											ChangeViewPicture(&state, +1, false);
										}
									} else if(ev.keyboard.mappedKey == fplKey_PageDown) {
										if(state.activeFileIndex < ((int)state.pictureFileCount - PAGE_INCREMENT_COUNT)) {
											ChangeViewPicture(&state, PAGE_INCREMENT_COUNT, false);
										}
									} else if(ev.keyboard.mappedKey == fplKey_PageUp) {
										if(state.activeFileIndex > (PAGE_INCREMENT_COUNT - 1)) {
											ChangeViewPicture(&state, -PAGE_INCREMENT_COUNT, false);
										}
									} else if(ev.keyboard.mappedKey == fplKey_Home) {
										int delta = 0 - (int)state.activeFileIndex;
										ChangeViewPicture(&state, delta, true);
									} else if(ev.keyboard.mappedKey == fplKey_End) {
										int delta = (int)state.pictureFileCount - state.activeFileIndex;
										ChangeViewPicture(&state, delta, true);
									} else if(ev.keyboard.mappedKey == fplKey_F) {
										fplSetWindowFullscreenSize(!fplIsWindowFullscreen(), 0, 0, 0);
									} else if(ev.keyboard.mappedKey == fplKey_P) {
										state.params.preview = !state.params.preview;
									} else if(ev.keyboard.mappedKey == fplKey_R) {
										ChangeViewPicture(&state, 0, true);
									} else if(ev.keyboard.mappedKey == fplKey_T) {
										state.activeFilter = (state.activeFilter + 1) % state.filterCount;
										UpdateWindowTitle(&state);
									}
								}
							}
						} break;

						default:
							break;
					}
				}

				UpdateAndRender(&state, deltaTime);

				fplVideoFlip();
			}

			Kill(&state);

			fglUnloadOpenGL();
		} else {
			returnCode = -1;
		}

		fplPlatformRelease();
	} else {
		returnCode = -1;
	}

	if(icon16Data != fpl_null) {
		stbi_image_free(icon16Data);
	}
	if(icon32Data != fpl_null) {
		stbi_image_free(icon32Data);
	}

	flogWrite("Shutdown %s with code %d", VER_PRODUCTNAME_STR, returnCode);

	return(returnCode);
}