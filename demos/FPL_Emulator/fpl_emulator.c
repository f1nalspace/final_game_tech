/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Emulator

Description:
	Fully working game boy DMG/CGB emulator with a simple debugger based on the final_game_box.h.
	
Features:

	- OpenGL Application with an immediate mode UI built on final_ui.h
	- Loading GamePak roms from either raw or zip files with drag & drop support
	- Emulator controls (Play, Pause, Stepping, etc.)
	- Visual Debugger with disassembly, breakpoints, various stepping modes
	- Rendering of internal states, such as CPU, PPU, APU, GamePak, etc.
	- Tilemap visualization
	- Full background map visualization with scroll area
	- Color palette rendering & swapping for DMG
	- Color palette rendering for CGB
	- Asyncrounous audio playback
	- Asyncrounous emulation using ring buffer for audio and image data
	- Information dialog with the key mapping, the feature list, the credits and the licenses

Key mapping:

	- A is mapped to the A-key of the keyboard or the A-button of your gamepad
	- B is mapped to the S-key of the keyboard or the B-button of your gamepad
	- Start is mapped to the Return-key of the keyboard or the Start-button of your gamepad
	- Select is mapped to the Backspace-key of the keyboard or the Select-button of your gamepad
	- Up is mapped to the Up-key of the keyboard or the DPad-Up of your gamepad
	- Down is mapped to the Down-key of the keyboard or the DPad-Down of your gamepad
	- Left is mapped to the Left-key of the keyboard or the DPad-Left of your gamepad
	- Right is mapped to the Right-key of the keyboard or the DPad-Right of your gamepad

Requirements:
	- C99 Compiler
	- Platform x64 or x86_64
	- Final Platform Layer
	- Final Dynamic OpenGL
	- Final Memory
	- Final Additions (Math)
	- Final UI
	- Final Game Box
	- STB Image
	- STB TrueType
	- MiniZ

Author:
	Torsten Spaete

Changelog:
	## 2026-08-26
	- Added an information dialog with an About, How to Use, Controls, Features and Libraries page
	- Added the information icon in the bottom left corner of both views, which is what opens that dialog
	- Added UITextView, UITabStrip, UIBevelBox and UIIconButton to ui.c/ui.h

	## 2026-08-24
	- Migrated the entire frontend UI to final_ui.h, replacing the hand rolled widget set in ui.c/ui.h
	- Switched the whole frontend to a top-left origin with y pointing down, matching final_ui.h
	- Text is baked by fui_font_stbtt.h into a body and a watermark atlas, replacing final_fontloader.h
	- Removed the widget era drawing helpers from the renderer, which now draws final_ui.h draw data

	## 2026-05-09
	- Shader support for gameboy display (Bilinear, HQ2x, HQ4x, Cat-Mull-Rom, Bicubic)

	## 2026-04-19
	- Support for CGB rom file extension
	- Support for render CGB palettes
	- Increased persistent and transient memory block
	- Fixed Disassembly loading was broken (misaligned instructions)
	- Fixed UIListbox highlight/scrolling was not handling resize
	- Fixed UIListbox computation issues
	- Fixed audio sample ring buffer was not drained, in pause mode
	- Improved assembly list by limit the updates to 0.1 secs

	## 2026-04-16
	- Emulation thread does not wait for OpenGL transfer anymore and buffers the pixels
	- Fixed audio playback was not in-sync with games
	- Improve texture upload performance by using glTexSubImage2D instead glTexImage2D

	## 2026-04-10
	- No more stall for texture uploads, emulator thread stores display/background-map/tile-map in ring buffer
	- Fixed audio sample playback was not in-sync with running game
	- Fixed strcmp() was used, even though no <string.h> was included. Now we have a macro FGB_STRCMP()

	## 2025-06-26
	- Initial version

Todo:
	- Unloading game button (very easy to do)
	- Show more CGB states
	- Show rom/ram bank indices in UI
	- OAM visualization (harder than it seems)
	- CGB sprite data visualization (harder than it seems)
	- Add option to select background tile area, because LCDC changes while a frame is rendered

License:
	Copyright (c) 2024-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Final Platform Layer
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

// Headers
#include "utils.h"

// Final Memory
#define FMEM_IMPLEMENTATION
#define FMEM_MALLOC(size) fplMemoryAllocate(size)
#define FMEM_FREE(ptr) fplMemoryFree(ptr)
#define FMEM_ASSERT(exp) fplAssert(exp)
#define FMEM_STATIC_ASSERT(exp) fplStaticAssert(exp)
#define FMEM_MEMSET(mem, value, size) fplMemorySet(mem, (uint8_t)(value), size)
#include <final_memory.h>

// MiniZ (Zip Compression Library)
#define MINIZ_NO_STDIO
#include <miniz/miniz.c>
#include <miniz/miniz_tinfl.c>
#include <miniz/miniz_tdef.c>
#include <miniz/miniz_zip.c>

// Final Gamebox
#define FGB_DISABLE_PLATFORM_DETECTION

#define FGB_STRLEN(str) fplGetStringLength(str)
#define FGB_STRINGFORMAT(buffer, bufferSize, format, ...) fplStringFormatArgs(buffer, bufferSize, format, __VA_ARGS__)
#define FGB_STRCMP(a, b) StringCompare(a, b)
#define FGB_MEMSET(ptr, value, size) fplMemorySet(ptr, value, size)
#define FGB_MEMCOPY(dst, src, size) fplMemoryCopy(src, size, dst)

#define FGB_ASSERT(exp) fplAssert(exp)
#define FGB_STATIC_ASSERT(exp) fplStaticAssert(exp)

#define FGB_INTERLOCKED_EXCHANGE_64(storage, value) fplAtomicExchangeS64((volatile int64_t *)(storage), (int64_t)(value))
#define FGB_INTERLOCKED_EXCHANGE_ADD_64(storage, addend) fplAtomicFetchAndAddS64((volatile int64_t *)(storage), (int64_t)(addend))
#define FGB_INTERLOCKED_LOAD_64(storage) fplAtomicLoadS64((volatile int64_t *)(storage))

#define FGB_CURRENT_TICKS() fplMillisecondsQuery()

#define FGB_IMPLEMENTATION
#include <final_game_box.h>

#define FGB_INPUT_SIMULATOR_IMPLEMENTATION
#include "fgb_input_simulator.h"

// Final Additions
#include <final_math.h>

// stb_truetype, emitted here because fui_font_stbtt.h deliberately includes only its interface
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// Final UI
#define FUI_TEXTURE_ID_TYPE uint32_t
#define FUI_ASSERT(exp) fplAssert(exp)
#define FUI_MALLOC(size) fplMemoryAllocate(size)
#define FUI_FREE(ptr) fplMemoryFree(ptr)
#define FUI_IMPLEMENTATION
#include <final_ui.h>

// Reference font provider for Final UI, baking the very same TrueType bytes fontdata.h already carries
#define FUI_STBTT_IMPLEMENTATION
#include <fui_font_stbtt.h>

// Local
#include "fontdata.h"
#include "imagedata.h"
#include "ui.c"
#include "about.c"
#include "render.c"
#include "utils.c"
#include "shaders.h"

// Is the debug UI enabled at startup
#define DEBUG_AT_START 1

// Do not draw any mouse cursors
#define NO_CURSOR 1

// How large the clickable information icon in the window's bottom left corner is drawn, in pixels
#define INFO_ICON_SIZE 100.0f

// How large the application icon overlaying the right panel is drawn, in pixels. Two thirds larger than the
// information icon, so the two corners read as a pair without the decoration outweighing the button
#define APP_ICON_SIZE (INFO_ICON_SIZE * (5.0f / 3.0f))

// How strongly that application icon shows over the panel it lies on. It is decoration and answers no click,
// so a click meant for what lies under it still gets there whatever this says
#define APP_ICON_OPACITY 0.75f

// Set from FGB!
#define MAX_STATE_SLOT_COUNT 6
fplStaticAssert(MAX_STATE_SLOT_COUNT % 2 == 0);

// Boot ROM
//#define NO_BOOTROM
#if !defined(NO_BOOTROM)
#include "bootrom.h"
#endif

typedef enum {
	ColorPaletteType_DMG = 0,
	ColorPaletteType_MGB,
	ColorPaletteType_SGB,
	ColorPaletteType_Blue,
} ColorPaletteType;

#define BLUE_COLOR_OFF {65, 112, 193}
#define BLUE_COLOR_ON {178, 206, 255}
#define BLUE_COLOR_00 {178, 206, 255}
#define BLUE_COLOR_01 {98, 134, 194}
#define BLUE_COLOR_10 {51, 69, 102}
#define BLUE_COLOR_11 {31, 43, 63}

static const char *fgb__externalRAMFileExtension = ".eram";

static const char *fgb__stateFileExtension = ".sav";

static fgbMonochromeColors BlueMonochromeColors = {
	.background = {BLUE_COLOR_00, BLUE_COLOR_01, BLUE_COLOR_10, BLUE_COLOR_11},
	.sprite0 = {BLUE_COLOR_00, BLUE_COLOR_01, BLUE_COLOR_10, BLUE_COLOR_11},
	.sprite1 = {BLUE_COLOR_00, BLUE_COLOR_01, BLUE_COLOR_10, BLUE_COLOR_11},
	.system = {BLUE_COLOR_OFF, BLUE_COLOR_ON}
};

static inline fmemMemoryBlock CreateTransientMemory(const size_t size) {
	fmemMemoryBlock memory = fmemCreate(fmemType_Fixed, size, 0);
	return memory;
}

static inline fmemMemoryBlock CreatePersistentMemory(const size_t size) {
	fmemMemoryBlock memory = fmemCreate(fmemType_Growable, size, 0);
	return memory;
}

typedef struct {
	fmemMemoryBlock base;
	fmemMemoryBlock temporary;
	volatile int64_t allocationCount;
	size_t padding[7];
} TransientMemory;

static FGB_CREATE_FILE_CALLBACK(frontend_FileCreate) {
	fplFileHandle *p = malloc(sizeof(fplFileHandle));
	if (!fplFileCreateBinary(filePath, p)) {
		return false;
	}
	*fileHandle = p;
	return true;
}

static FGB_OPEN_FILE_CALLBACK(frontend_FileOpen) {
	// TODO(final): Use temporary MemoryBlock instead!
	fplFileHandle *p = malloc(sizeof(fplFileHandle));

	if (!fplFileOpenBinary(filePath, p)) {
		return false;
	}

	*fileHandle = p;

	return true;
}

static FGB_CLOSE_FILE_CALLBACK(frontend_FileClose) {
	fplFileHandle *p = (fplFileHandle *)fileHandle;
	if (p == NULL) {
		return;
	}
	fplFileClose(p);
	free(p); // TODO(final): Use temporary MemoryBlock instead!
}

static FGB_READ_FILE_BUFFER_CALLBACK(frontend_FileRead) {
	fplFileHandle *p = (fplFileHandle *)fileHandle;
	if (p == NULL) {
		return 0;
	}
	return fplFileReadBlock(p, readSize, outBuffer, maxBufferLen);
}

static FGB_WRITE_FILE_BUFFER_CALLBACK(frontend_FileWrite) {
	fplFileHandle *p = (fplFileHandle *)fileHandle;
	if (p == NULL) {
		return 0;
	}
	return fplFileWriteBlock(p, (void *)inBuffer, writeSize);
}
static FGB_GET_FILE_SIZE_CALLBACK(frontend_GetFileSize) {
	fplFileHandle *p = (fplFileHandle *)fileHandle;
	if (p == NULL) {
		return 0;
	}
	return fplFileGetSizeFromHandle(p);
}

static FGB_FLUSH_FILE_CALLBACK(frontend_FlushFile) {
	fplFileHandle *p = (fplFileHandle *)fileHandle;
	if (p == NULL) {
		return;
	}
	fplFileFlush(p);
}

static FGB_BUILD_FILEPATH_CALLBACK(frontend_BuildFilePath) {
	char tmp[FPL_MAX_PATH_LENGTH] = fplZeroInit;

	char fileExt[16] = { 0 };

	switch (fileType) {
		case fgbFileType_ExternalRAM:
			fplCopyString(fgb__externalRAMFileExtension, fileExt, fplArrayCount(fileExt));
			break;
		case fgbFileType_Snapshot:
			fplStringFormat(fileExt, fplArrayCount(fileExt), "%s%u", fgb__stateFileExtension, slotIndex);
			break;
		default:
			return 0;
	}

	if (fplGetStringLength(folderPath) > 0) {
		const char *filename = fplExtractFileName(filePath);
		fplChangeFileExtension(filename, fileExt, tmp, fplArrayCount(tmp));
		fplPathCombine(outBuffer, maxBufferLen, 2, folderPath, tmp);
	} else {
		fplChangeFileExtension(filePath, fileExt, outBuffer, maxBufferLen);
	}

	if (fplIsStringEqual(outBuffer, filePath))
		return false;

	return true;
}

static FGB_ALLOCATE_MEMORY_CALLBACK(frontend_AllocateMemory) {
	TransientMemory *transientMem = (TransientMemory *)userData;
	if (transientMem == fpl_null || transientMem->base.size == 0) {
		FPL_LOG_ERROR("Frontend", "No base memory block set!");
		return fpl_null;
	}

	if (transientMem->temporary.size == 0) {
		if (!fmemBeginTemporary(&transientMem->base, &transientMem->temporary)) {
			FPL_LOG_ERROR("Frontend", "Failed to begin a temporary memory from base size '%zu'", transientMem->base.size);
			return fpl_null;
		}
	}

	uint8_t *result = fmemPush(&transientMem->temporary, size, fmemPushFlags_Clear);
	if (result == fpl_null) {
		FPL_LOG_ERROR("Frontend", "Not enough memory in transient block. Expect %zu bytes but got %zu bytes", size, remainingSize);
		return fpl_null;
	}

	fplAtomicIncrementS64(&transientMem->allocationCount);

	return result;
}

static FGB_FREE_MEMORY_CALLBACK(frontend_FreeMemory) {
	TransientMemory *transientMem = (TransientMemory *)userData;
	if (transientMem == fpl_null || transientMem->temporary.size == 0) {
		FPL_LOG_ERROR("Frontend", "No temporary memory block set!");
		return;
	}
	int64_t count = fplAtomicAddAndFetchS64(&transientMem->allocationCount, -1);
	if (count == 0) {
		fmemEndTemporary(&transientMem->temporary);
	}
}

static FGB_DATETIME_QUERY(frontend_DateTimeQuery) {
	fplDateTime dt = fplDateTimeQuery(fplDateTimeType_UTC);
	fgbDateTime result = { 0 };
	result.epoch = dt.epoch;
	result.milliseconds = dt.milliseconds;
	return result;
}

static TransientMemory globalTransientMemory = { 0 };

static const fgbCallbacks globalCallbacks = {
	.createFile = frontend_FileCreate,
	.openFile = frontend_FileOpen,
	.closeFile = frontend_FileClose,
	.readFile = frontend_FileRead,
	.writeFile = frontend_FileWrite,
	.getFileSize = frontend_GetFileSize,
	.flushFile = frontend_FlushFile,
	.buildFilePath = frontend_BuildFilePath,
	.allocateMemory = frontend_AllocateMemory,
	.freeMemory = frontend_FreeMemory,
	.dateTimeQuery = frontend_DateTimeQuery,
	.memoryAllocationUserData = &globalTransientMemory,
};

#define PERFORMANCE_COUNTER_DELTA_CAPACITY 25

typedef struct {
	fplSeconds deltas[PERFORMANCE_COUNTER_DELTA_CAPACITY];
	fplTimestamp start;
	fplTimestamp end;
	size_t count;
	fplSeconds minSecs;
	fplSeconds maxSecs;
} PerformanceCounter;

static inline double GetPerformanceCounterAvg(const PerformanceCounter *counter) {
	double total = 0.0;
	size_t count = fplMin(PERFORMANCE_COUNTER_DELTA_CAPACITY, counter->count);
	for (size_t i = 0; i < count; ++i) {
		total += counter->deltas[i];
	}
	return count > 0 ? total / (double)count : 0.0;
}

static void ResetPerformanceCounter(PerformanceCounter *counter) {
	fplClearStruct(counter);
}

static void BeginPerformanceCounter(PerformanceCounter *counter, const fplTimestamp start) {
	counter->start = counter->end = start;
}

static void EndPerformanceCounter(PerformanceCounter *counter, const fplTimestamp end) {
	counter->end = end;
	double duration = fplTimestampElapsed(counter->start, counter->end);
	size_t x = counter->count++;

	size_t deltaIndex = x % PERFORMANCE_COUNTER_DELTA_CAPACITY;
	counter->deltas[deltaIndex] = duration;

	counter->minSecs = counter->maxSecs = 0;

	size_t count = fplMin(PERFORMANCE_COUNTER_DELTA_CAPACITY, counter->count);

	for (size_t i = 0; i < count; ++i) {
		counter->minSecs = fplMin(counter->minSecs, counter->deltas[deltaIndex]);
		counter->maxSecs = fplMax(counter->maxSecs, counter->deltas[deltaIndex]);
	}
}

typedef union {
	struct {
		PerformanceCounter frameTime;
		PerformanceCounter texturesUpload;
		PerformanceCounter audioThread;
		PerformanceCounter audioReadSamples;
		PerformanceCounter audioOutputSamples;
		PerformanceCounter emulatorTick;
	};
	PerformanceCounter counters[6];
} PerformanceMetrics;

static void ResetPerformanceMetrics(PerformanceMetrics *metrics) {
	for (int i = 0; i < fplArrayCount(metrics->counters); ++i) {
		ResetPerformanceCounter(&metrics->counters[i]);
	}
}

typedef struct {
	fgbSnapshot snapshots[MAX_STATE_SLOT_COUNT];
	Texture textures[MAX_STATE_SLOT_COUNT];
	char labels[MAX_STATE_SLOT_COUNT][64];
} States;

// Lock-free SPSC frame queues carrying raw fgbColor snapshots from the
// emulator thread (producer) to the main thread (consumer). Same pattern as
// fgbAudioRingBuffer in final_game_box.h, power-of-two capacity with mask.
#define FRAME_QUEUE_CAPACITY 4u
#define FRAME_QUEUE_MASK     (FRAME_QUEUE_CAPACITY - 1u)
fplStaticAssert((FRAME_QUEUE_CAPACITY & FRAME_QUEUE_MASK) == 0);

typedef struct {
	fgbColor pixels[FGB_DISPLAY_WIDTH * FGB_DISPLAY_HEIGHT];
} FrameSnapshotDisplay;

typedef struct {
	fgbColor pixels[FGB_BACKGROUND_MAP_WIDTH * FGB_BACKGROUND_MAP_HEIGHT];
} FrameSnapshotBackgroundMap;

typedef struct {
	fgbColor pixels[FGB_TILEMAP_WIDTH * FGB_TILEMAP_HEIGHT];
} FrameSnapshotTilemap;

typedef struct {
	fgbCacheline cachelineHead;
	volatile int64_t head;
	fgbCacheline cachelineTail;
	volatile int64_t tail;
	fgbCacheline cachelineSlots;
	FrameSnapshotDisplay slots[FRAME_QUEUE_CAPACITY];
} DisplayFrameQueue;

typedef struct {
	fgbCacheline cachelineHead;
	volatile int64_t head;
	fgbCacheline cachelineTail;
	volatile int64_t tail;
	fgbCacheline cachelineSlots;
	FrameSnapshotBackgroundMap slots[FRAME_QUEUE_CAPACITY];
} BackgroundMapFrameQueue;

typedef struct {
	fgbCacheline cachelineHead;
	volatile int64_t head;
	fgbCacheline cachelineTail;
	volatile int64_t tail;
	fgbCacheline cachelineSlots;
	FrameSnapshotTilemap slots[FRAME_QUEUE_CAPACITY];
} TilemapFrameQueue;

static inline void DisplayFrameQueueInit(DisplayFrameQueue *q) {
	fgb__InterlockedExchange64(&q->head, 0);
	fgb__InterlockedExchange64(&q->tail, 0);
}

static inline void BackgroundMapFrameQueueInit(BackgroundMapFrameQueue *q) {
	fgb__InterlockedExchange64(&q->head, 0);
	fgb__InterlockedExchange64(&q->tail, 0);
}

static inline void TilemapFrameQueueInit(TilemapFrameQueue *q) {
	fgb__InterlockedExchange64(&q->head, 0);
	fgb__InterlockedExchange64(&q->tail, 0);
}

static inline bool DisplayFrameQueueTryPush(DisplayFrameQueue *q, const FrameSnapshotDisplay *src) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	const int64_t nextHead = (head + 1) & FRAME_QUEUE_MASK;
	if (nextHead == tail) {
		return false;
	}
	q->slots[(uint64_t)head] = *src;
	fgb__InterlockedExchange64(&q->head, nextHead);
	return true;
}

static inline bool BackgroundMapFrameQueueTryPush(BackgroundMapFrameQueue *q, const FrameSnapshotBackgroundMap *src) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	const int64_t nextHead = (head + 1) & FRAME_QUEUE_MASK;
	if (nextHead == tail) {
		return false;
	}
	q->slots[(uint64_t)head] = *src;
	fgb__InterlockedExchange64(&q->head, nextHead);
	return true;
}

static inline bool TilemapFrameQueueTryPush(TilemapFrameQueue *q, const FrameSnapshotTilemap *src) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	const int64_t nextHead = (head + 1) & FRAME_QUEUE_MASK;
	if (nextHead == tail) {
		return false;
	}
	q->slots[(uint64_t)head] = *src;
	fgb__InterlockedExchange64(&q->head, nextHead);
	return true;
}

static inline bool DisplayFrameQueuePopNewest(DisplayFrameQueue *q, FrameSnapshotDisplay *out) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	if (head == tail) {
		return false;
	}
	const int64_t newestIdx = (head - 1 + FRAME_QUEUE_CAPACITY) & FRAME_QUEUE_MASK;
	*out = q->slots[(uint64_t)newestIdx];
	fgb__InterlockedExchange64(&q->tail, head);
	return true;
}

static inline bool BackgroundMapFrameQueuePopNewest(BackgroundMapFrameQueue *q, FrameSnapshotBackgroundMap *out) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	if (head == tail) {
		return false;
	}
	const int64_t newestIdx = (head - 1 + FRAME_QUEUE_CAPACITY) & FRAME_QUEUE_MASK;
	*out = q->slots[(uint64_t)newestIdx];
	fgb__InterlockedExchange64(&q->tail, head);
	return true;
}

static inline bool TilemapFrameQueuePopNewest(TilemapFrameQueue *q, FrameSnapshotTilemap *out) {
	const int64_t head = fgb__InterlockedRead64(&q->head);
	const int64_t tail = fgb__InterlockedRead64(&q->tail);
	if (head == tail) {
		return false;
	}
	const int64_t newestIdx = (head - 1 + FRAME_QUEUE_CAPACITY) & FRAME_QUEUE_MASK;
	*out = q->slots[(uint64_t)newestIdx];
	fgb__InterlockedExchange64(&q->tail, head);
	return true;
}

// Wall-clock ns per PPU frame: 1e9 * 70368 / 4194304 = 16742706 (exact).
#define EMULATOR_FRAME_TIME_NS       ((uint64_t)16742706)
// Safety bound on the inner fgbTick loop (far exceeds ~70k real cycles / frame).
#define EMULATOR_INNER_SAFETY_CAP    200000u
// Spin the last ~1.5 ms of the wait for sub-ms accuracy; coarse sleep handles the bulk.
#define EMULATOR_SPIN_THRESHOLD_NS   ((int64_t)1500000)

typedef struct {
	States states;

	fgbSystem system;

	fgbConfiguration config;

	PerformanceMetrics performanceMetrics;

	fplMutexHandle mutex;
	fplConditionVariable waitCondition;
	fplConditionVariable microStepCondition;
	fplConditionVariable breakpointCondition;

	DisplayFrameQueue displayQueue;
	BackgroundMapFrameQueue backgroundMapQueue;
	TilemapFrameQueue tilemapQueue;

	String pendingROMFilePath;

	fplThreadHandle *thread;

	fgbBreakpointType lastBreakpointType;

	ColorPaletteType paletteType;

	volatile uint32_t isActive;

	volatile uint32_t isROMFileRequested;

	volatile uint32_t isShutdown;

	volatile uint32_t isFrameStepActive;
	volatile uint32_t isMicroStepActive;

	// Sticky audio-rescue flag: set when ring drops below LOW_WATER,
	// cleared when ring refills to HIGH_WATER. While set, pacing skips
	// the wall-clock sleep so the ring can be rebuilt to full headroom
	// instead of stabilizing at the low-water level.
	volatile uint32_t audioRescueActive;

	float masterVolume;

	// Frontend mute. The emulated APU's own power bit is driven by the game and cannot be written from
	// outside, so switching sound off happens on the way to the audio device rather than in the hardware
	volatile uint32_t isSoundEnabled;

	uint16_t currentROMBank;

	// Scripted joypad input for automated testing (see INPUT_SIMULATOR.md).
	// While a script is loaded and still has pending events, it owns the joypad and real input is ignored.
	fgbInputSimulator inputSim;
	// The emulated frame index the input simulator has been applied up to (only touched by the emulator thread)
	uint32_t inputSimFrameIndex;
} Emulator;

typedef enum {
	DialogType_None = 0,
	DialogType_SaveState,
	DialogType_RestoreState,
} DialogType;

typedef struct {
	uint32_t row;
	uint32_t column;
} StatesDialogCellPos;

typedef struct {
	StatesDialogCellPos selectedSlotPos;
	DialogType type;
	bool isShown;
} StatesDialog;

typedef struct {
	const char *romFilePath;
	const char *inputScriptFilePath;
	bool isTraceEnabled;
} EmulatorParameters;

typedef struct {
	ShaderProgram program;
	ShaderError error;
	int32_t textureSamplerLocation;
	int32_t textureSizeLocation;
	int32_t imageSizeLocation;
	bool isValid;
} AppShader;

typedef enum {
	AppShaderType_None = 0,
	AppShaderType_Bilinear,
	AppShaderType_HQ2X,
	AppShaderType_HQ4X,
	AppShaderType_BicubicHermite,
	AppShaderType_BicubicLagrange,
	AppShaderType_CatmullRom4,
} AppShaderType;

typedef struct {
	fuiContext ui;
	fuiInput uiInput;
	UIFont uiFont;

	char romsPath[1024];
	char defaultGameRomFilePath[1024];

	Mat4f projectionMat;
	Mat4f viewMat;
	Mat4f viewProjectionMat;

	Texture cursorTexture;
	Texture aboutIconTexture;
	Texture aboutIconSmallTexture;
	Texture appIconTexture;
	Texture appIconSmallTexture;
	Texture displayTexture;
	Texture backgroundMapTexture;
	Texture tileMapTexture;
	Texture gbTexture;

	Viewport4i viewport;

	AppShader nearestShader;
	AppShader bilinearShader;
	AppShader catmullRom4Shader;
	AppShader hq2xShader;
	AppShader hq4xShader;
	AppShader bicubicLagrangeShader;
	AppShader bicubicHermiteShader;

	Vec2i windowSize;

	StringList console;
	fmemMemoryBlock consoleMemory;
	UISourceListState consoleList;

	StringList disassembly;
	IndexHashtable disassemblyHashTable;
	fmemMemoryBlock disassemblyMemory;
	UISourceListState disassemblyList;
	int32_t disassemblyHighlightIndex;
	bool disassemblyScrollRequested;
	bool consoleScrollPending;

	// Width of the two side columns, owned by their splitters. Zero until the first layout seeds them
	float leftPanelWidth;
	float rightPanelWidth;

	StatesDialog statesDialog;
	AboutDialog aboutDialog;

	Emulator emulator;

	fpl_b32 isValid;

	AppShaderType activeShaderType;

	bool isDebugEnabled;
	bool isShaderSupported;

	fplTimestamp lastDisassemblyScrollTime;
	uint64_t lastDisassemblyScrollPC;
} Application;

//
// Emulator input buttons
//
// These track the joypad and the debug keys rather than the interface, which is why they stay here
// and are not the fuiButtonState the user interface is fed.
//

typedef struct {
	int32_t halfTransitionCount;
	uint32_t endedDown;
} UIButtonState;

fpl_extern_inline bool UIWasPressed(const UIButtonState *state) {
	bool result = ((state->halfTransitionCount > 1) || ((state->halfTransitionCount == 1) && (!state->endedDown)));
	return(result);
}

fpl_extern_inline bool UIIsDown(const UIButtonState *state) {
	bool result = state->endedDown != 0;
	return(result);
}

static inline void UpdateKeyboardButtonState(UIButtonState *newState, const bool isDown) {
	newState->endedDown = isDown;
	++newState->halfTransitionCount;
}

static inline bool UpdateDigitalButtonState(const UIButtonState *oldState, UIButtonState *newState, const bool isDown) {
	newState->endedDown = isDown;
	newState->halfTransitionCount = ((newState->endedDown == oldState->endedDown) ? 0 : 1);
	return(newState->endedDown == 1);
}

typedef struct {
	union {
		struct {
			UIButtonState left;
			UIButtonState middle;
			UIButtonState right;
		};
		UIButtonState buttons[3];
	};
	Vec2i screenPos;
	Vec2f worldPos;
	float wheelDelta;
} MouseInput;

typedef struct {
	union {
		struct {
			UIButtonState singleStep;
			UIButtonState frameStep;
			UIButtonState toggleDebug;
		};
		UIButtonState buttons[3];
	};
	bool isEnabled;
} DebugInput;

typedef enum {
	ControllerState_Disconnected = 0,
	ControllerState_Connected,
} ControllerState;

#define KEYBOARD_CONTROLLER_INDEX 0

#define CONTROLLER_BUTTON_START 0
#define CONTROLLER_BUTTON_SELECT 1
#define CONTROLLER_BUTTON_A 2
#define CONTROLLER_BUTTON_B 3
#define CONTROLLER_BUTTON_DPAD_UP 4
#define CONTROLLER_BUTTON_DPAD_DOWN 5
#define CONTROLLER_BUTTON_DPAD_LEFT 6
#define CONTROLLER_BUTTON_DPAD_RIGHT 7

typedef struct {
	union {
		struct {
			UIButtonState start;
			UIButtonState select;
			UIButtonState actionA;
			UIButtonState actionB;
			UIButtonState dpadUp;
			UIButtonState dpadDown;
			UIButtonState dpadLeft;
			UIButtonState dpadRight;
		};
		UIButtonState buttons[8];
	};
	ControllerState state;
} ControllerInput;

typedef struct {
	union {
		struct {
			ControllerInput keyboardController;
			ControllerInput gamepads[4];
		};
		ControllerInput controllers[5];
	};
	DebugInput debug;
	MouseInput mouse;
	int32_t activeControllerIndex;
	double frameRate;
} InputState;

static void EmulatorThreadProc(const fplThreadHandle *thread, void *data);

static void WakeupEmulatorThread(Emulator *emulator) {
	fplConditionSignal(&emulator->waitCondition);
	fplConditionSignal(&emulator->microStepCondition);
	fplConditionSignal(&emulator->breakpointCondition);
}

static bool InitEmulator(fmemMemoryBlock *mem, Emulator *emulator) {
	fplClearStruct(emulator);

	if (!fplConditionInit(&emulator->waitCondition)) {
		return false;
	}
	if (!fplConditionInit(&emulator->microStepCondition)) {
		return false;
	}
	if (!fplConditionInit(&emulator->breakpointCondition)) {
		return false;
	}

	if (!fplMutexInit(&emulator->mutex)) {
		return false;
	}

	for (int i = 0; i < fplArrayCount(emulator->states.textures); ++i) {
		emulator->states.textures[i] = RendererTextureAllocate(mem, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	}

	DisplayFrameQueueInit(&emulator->displayQueue);
	BackgroundMapFrameQueueInit(&emulator->backgroundMapQueue);
	TilemapFrameQueueInit(&emulator->tilemapQueue);

	emulator->isShutdown = false;
	emulator->isActive = false;
	emulator->masterVolume = 0.25f;
	emulator->isSoundEnabled = true;

	emulator->thread = fplThreadCreate(EmulatorThreadProc, emulator);
	if (emulator->thread == fpl_null) {
		return false;
	}

	return true;
}

static void ReleaseEmulator(Emulator *emulator) {
	if (emulator == fpl_null) {
		return;
	}

	emulator->isShutdown = true;

	if (emulator->thread != fpl_null) {
		WakeupEmulatorThread(emulator);
		fplThreadWaitForOne(emulator->thread, FPL_TIMEOUT_INFINITE);
	}

	if (emulator->isActive) {
		fgbShutdown(&emulator->system);
		emulator->isActive = false;
	}

	if (emulator->mutex.isValid) {
		fplMutexDestroy(&emulator->mutex);
	}

	if (emulator->breakpointCondition.isValid) {
		fplConditionDestroy(&emulator->breakpointCondition);
	}
	if (emulator->microStepCondition.isValid) {
		fplConditionDestroy(&emulator->microStepCondition);
	}
	if (emulator->waitCondition.isValid) {
		fplConditionDestroy(&emulator->waitCondition);
	}

	for (int i = 0; i < fplArrayCount(emulator->states.textures); ++i)
		RendererTextureRelease(&emulator->states.textures[i]);

	fplClearStruct(emulator);
}

static void UpdateAppShaderLocations(AppShader *appShader) {
	appShader->textureSamplerLocation = RendererShaderGetUniformLocation(&appShader->program, "textureSampler");
	appShader->textureSizeLocation = RendererShaderGetUniformLocation(&appShader->program, "textureSize");
	appShader->imageSizeLocation = RendererShaderGetUniformLocation(&appShader->program, "imageSize");
}

static void LoadAppShader(AppShader *shader, const char *vertexSource, const char *fragmentSource) {
	if (RendererShaderCreate(vertexSource, fragmentSource, &shader->program, &shader->error)) {
		UpdateAppShaderLocations(shader);
		shader->isValid = true;
	}
}

static Application *CreateApplication(fmemMemoryBlock *mem, const EmulatorParameters *parameters, const RendererSupport *rendererSupport) {
	Application *app = fmemPushStruct(mem, Application, fmemPushFlags_Clear);
	if (app == fpl_null) {
		return fpl_null;
	}

	// Roms path
	fplGetExecutableFilePath(app->romsPath, fplArrayCount(app->romsPath));
	fplExtractFilePath(app->romsPath, app->romsPath, fplArrayCount(app->romsPath));
	fplPathCombine(app->romsPath, fplArrayCount(app->romsPath), 2, app->romsPath, "roms");

	// Bake the body and the watermark atlases the interface draws with
	if (!UIFontCreate(&app->uiFont, ptr_fireCodeFont)) {
		return fpl_null;
	}

	// Init console/disassembly/string memory
	size_t consoleBlockSize = fplMegaBytes(128);
	app->consoleMemory = CreatePersistentMemory(consoleBlockSize);
	app->console = StringListInit(&app->consoleMemory);

	size_t disassemblyBlockSize = fplMegaBytes(128);
	app->disassemblyMemory = CreatePersistentMemory(disassemblyBlockSize);
	app->disassembly = StringListInit(&app->disassemblyMemory);
	app->disassemblyHashTable = IndexHashtableInit(&app->disassemblyMemory);

	app->consoleList.selectedIndex = -1;
	app->disassemblyList.selectedIndex = -1;
	app->disassemblyHighlightIndex = -1;

	// Load/Allocate textures
	app->displayTexture = RendererTextureAllocate(mem, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->tileMapTexture = RendererTextureAllocate(mem, FGB_TILEMAP_WIDTH, FGB_TILEMAP_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->backgroundMapTexture = RendererTextureAllocate(mem, FGB_BACKGROUND_MAP_WIDTH, FGB_BACKGROUND_MAP_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->cursorTexture = RendererTextureLoadFromMemory(ptr_mouseCursor, sizeOf_mouseCursor, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);
	app->gbTexture = RendererTextureLoadFromMemory(ptr_gameboyImage, sizeOf_gameboyImage, TextureFormat_Automatic, TextureFilter_Linear, 619, 1024);
	app->aboutIconTexture = RendererTextureLoadFromMemory(ptr_aboutIcon, sizeOf_aboutIcon, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);
	// Two bakes of each picture rather than one: a texture shrunk to a fifth of its size by the sampler is a
	// smear, and this renderer builds no mip chain to shrink it with
	app->aboutIconSmallTexture = RendererTextureLoadFromMemory(ptr_aboutIconSmall, sizeOf_aboutIconSmall, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);
	app->appIconTexture = RendererTextureLoadFromMemory(ptr_appIcon, sizeOf_appIcon, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);
	app->appIconSmallTexture = RendererTextureLoadFromMemory(ptr_appIconSmall, sizeOf_appIconSmall, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);

	// Load shaders
	if (rendererSupport->hasGLSL) {
		LoadAppShader(&app->nearestShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureColor);
		LoadAppShader(&app->bilinearShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureBilinear);
		LoadAppShader(&app->catmullRom4Shader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureCatmullRom);
		LoadAppShader(&app->hq2xShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureHQ2X);
		LoadAppShader(&app->hq4xShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureHQ4X);
		LoadAppShader(&app->bicubicLagrangeShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureBicubicLagrange);
		LoadAppShader(&app->bicubicHermiteShader, g_shaderVertexPosTexcoord, g_shaderFragmentTextureBicubicHermite);
		app->isShaderSupported = app->bilinearShader.isValid;
		app->activeShaderType = AppShaderType_None;
	} else {
		app->isShaderSupported = false;
		app->activeShaderType = AppShaderType_None;
	}

	// Init UI
	if (!fuiInit(&app->ui, &app->uiFont.body.font, fui_null)) {
		return fpl_null;
	}
	const float uiFontHeight = 20.0f;
	const float uiLineScale = 1.15f;
	UIApplyDarkTheme(&app->ui, uiFontHeight, uiFontHeight * uiLineScale);
	UIInstallPlatform(&app->ui);
	app->uiInput = fuiZeroInput();

	// Init emulator
	if (!InitEmulator(mem, &app->emulator)) {
		return false;
	}

#if DEBUG_AT_START
	app->isDebugEnabled = true;
#else
	app->isDebugEnabled = false;
#endif

	return app;
}

static void ReleaseApplication(Application **appRef) {
	if (appRef == fpl_null) {
		return;
	}

	Application *app = *appRef;

	ReleaseEmulator(&app->emulator);

	RendererShaderRelease(&app->nearestShader.program);
	RendererShaderRelease(&app->bilinearShader.program);
	RendererShaderRelease(&app->catmullRom4Shader.program);
	RendererShaderRelease(&app->hq2xShader.program);
	RendererShaderRelease(&app->hq4xShader.program);
	RendererShaderRelease(&app->bicubicLagrangeShader.program);
	RendererShaderRelease(&app->bicubicHermiteShader.program);

	RendererTextureRelease(&app->cursorTexture);
	RendererTextureRelease(&app->aboutIconTexture);
	RendererTextureRelease(&app->aboutIconSmallTexture);
	RendererTextureRelease(&app->appIconTexture);
	RendererTextureRelease(&app->appIconSmallTexture);
	RendererTextureRelease(&app->tileMapTexture);
	RendererTextureRelease(&app->displayTexture);
	RendererTextureRelease(&app->backgroundMapTexture);
	RendererTextureRelease(&app->gbTexture);

	fmemFree(&app->disassemblyMemory);
	fmemFree(&app->consoleMemory);

	fuiRelease(&app->ui);
	UIFontRelease(&app->uiFont);

	fplClearStruct(app);

	*appRef = fpl_null;
}

static void HighlightScrollDisassembly(Application *app) {
	Emulator *emu = &app->emulator;
	fgbSystem *system = &emu->system;
	uint64_t key = system->cpu.registers.pc;
	size_t index = 0;
	if (IndexHashtableGet(&app->disassemblyHashTable, key, &index)) {
		app->disassemblyHighlightIndex = (int32_t)index;
		app->disassemblyScrollRequested = true;
	}
}

static char TextBuffer[256];

const float inv255 = 1.0f / 255.0f;

static Color4f FGBColorToLinearColor(const fgbColor color) {
	Color4f result = { (float)color.r * inv255, (float)color.g * inv255, (float)color.b * inv255, 1.0f };
	return result;
}
	
// One inset for every panel in the debugger. Sharing a single value is what puts the text of one panel and
// the checkboxes of the next on the same column instead of each starting wherever its own padding landed
static const float DebugPanelPadding = 8.0f;

//
// Panel content metrics
//
// The PPU and APU panels are sized from what they actually draw rather than from a guessed line count, so
// neither ends in a band of empty space. Each height function counts the same rows its draw code walks.
//

// Nine rows reach the pixel FIFO: four readouts, a blank, three more readouts, then a double gap
static const float DisplayStateTextRowCount = 9.0f;

// How many rows tall the pixel FIFO strip is drawn
static const float DisplayStateFifoRowCount = 1.5f;

static float DisplayStatePanelHeight(const float lineHeight) {
	const float layerSwitchRowCount = 1.0f;
	const float contentRowCount = DisplayStateTextRowCount + DisplayStateFifoRowCount + layerSwitchRowCount;
	// Three insets: above the text, between the FIFO strip and the switches, and below them
	const float result = contentRowCount * lineHeight + DebugPanelPadding * 3.0f;
	return result;
}

// Shown in place of the emulated APU's own power state while the user has muted the output themselves
static const fuiColor SoundStateMutedTextColor = { 1.0f, 0.55f, 0.1f, 1.0f };

// How wide the master volume slider is drawn. A slider only has to be wide enough to aim at, and the row
// it shares with the stereo readout reads better when it does not stretch across the whole panel
static const float SoundStateVolumeSliderWidth = 90.0f;

// Gap between the master volume slider and the stereo readout beside it
static const float SoundStateVolumeRowSpacing = 10.0f;

// How much air is left above the first voice, so it does not sit against the master volume slider
static const float SoundStateVoiceGapRowCount = 0.5f;

// How much air is left between two voice rows, ON TOP of the full row each one already occupies. A hair is
// all it takes to keep them from reading as one block - a third of a row on top of a whole one had the four
// of them drifting apart into four separate things
static const float SoundStateVoiceSpacingRowCount = 0.1f;

static float SoundStatePanelHeight(const float lineHeight) {
	const float soundToggleRowCount = 1.0f;
	const float masterVolumeRowCount = 1.0f;
	const float voiceCount = 4.0f;
	const float voiceGapCount = voiceCount - 1.0f;
	const float contentRowCount = soundToggleRowCount + masterVolumeRowCount + SoundStateVoiceGapRowCount + voiceCount + voiceGapCount * SoundStateVoiceSpacingRowCount;
	const float result = contentRowCount * lineHeight + DebugPanelPadding * 2.0f;
	return result;
}

static void DrawDisplayState(Application *app, const fgbPPU *ppu, const fuiRect area, const float padding) {
	fuiContext *ui = &app->ui;

	const fuiTheme *theme = fuiGetTheme(ui);
	// The same row height the layout sized this panel with, or the content and its box disagree
	const float lineHeight = theme->menuItemHeight;
	const fuiColor foregroundColor = theme->textColor;

	UIPanel(ui, area, false);
	UIWatermark(ui, &app->uiFont, area, "PPU");

	// Clipped to itself, so a line too long for the panel is cut at its border rather than run into the next one
	fuiPushClip(ui, area);

	const float paddingX = padding;
	const float paddingY = padding;

	float textX = area.x + paddingX;
	float textY = area.y + paddingY;

	const char *separator = ", ";

	const char *text;
	fuiVec2 textSize;
	fuiColor bitColor;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "LCDC: ");
	textSize = UITextSize(ui, TextBuffer, 0);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.x;

	// 7 = LCD Off/On
	text = (ppu->lcd.lcdc.lcdEnabled ? "On " : "Off");
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UITextSize(ui, TextBuffer, 0);
	if (ppu->lcd.lcdc.lcdEnabled)
		bitColor = UIColorFrom4f(ColorGreen);
	else
		bitColor = UIColorFrom4f(ColorRed);
	UIText(ui, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.x;

	// Separator
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), separator, text);
	textSize = UITextSize(ui, TextBuffer, 0);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.x;

	// Mode
	switch (ppu->lcd.stat.lcdMode) {
		case fgbPPUMode_OAMSearch:
			text = "OAM Search";
			break;
		case fgbPPUMode_PixelTransfer:
			text = "Pixel Transfer";
			break;
		case fgbPPUMode_HBlank:
			text = "HBlank";
			break;
		case fgbPPUMode_VBlank:
			text = "VBlank";
			break;
		default:
			text = "Unknown";
			break;

	}

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Mode: %s", text);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	textY += lineHeight;

	textX = area.x + paddingX;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Sprites: %02u, Ticks: %04u", ppu->pipeline.sprites.count, ppu->state.lineTicks);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "LY: %02u, LYC: %02u, SCX: %02u, SCY: %02u", ppu->lcd.ly, ppu->lcd.lyc, ppu->lcd.scx, ppu->lcd.scy);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "WinX: %02u, WinY: %02u", ppu->lcd.wx, ppu->lcd.wy);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	textY += lineHeight;

	switch (ppu->pipeline.fetch.state) {
		case fgbPPUFetchState_Tile:
			text = "Tile";
			break;
		case fgbPPUFetchState_Data0:
			text = "Data0";
			break;
		case fgbPPUFetchState_Data1:
			text = "Data1";
			break;
		case fgbPPUFetchState_Waiting:
			text = "Waiting";
			break;
		case fgbPPUFetchState_Push:
			text = "Push";
			break;
		default:
			text = "Unknown";
			break;
	}

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Pipeline: %-7s, Fifo: %02u/%02u, I/O: %02u/%02u", text, ppu->pipeline.fifo.len, FGB_ARRAYCOUNT(ppu->pipeline.fifo.pixels), ppu->pipeline.fifo.in, ppu->pipeline.fifo.out);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "X Line/Fetch/Push/FIFO: %02u/%02u/%02u/%02u", ppu->pipeline.state.lineX, ppu->pipeline.fetch.currentX, ppu->pipeline.state.pushX, ppu->pipeline.state.fifoX);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "BG-Tile: %02u x %02u, ID: $%02X, Y-Offset: %02u", ppu->pipeline.tilePos.x, ppu->pipeline.tilePos.y, ppu->pipeline.fetch.tileID, ppu->pipeline.state.offsetY);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	textY += lineHeight * 2.0f;

	const uint8_t fifoCapacity = FGB_ARRAYCOUNT(ppu->pipeline.fifo.pixels);

	const float fifoWidth = area.w - paddingX * 2.0f;
	const float fifoHeight = lineHeight * 1.5f;
	const float fifoCellWidth = fifoWidth / (float)fifoCapacity;

	float tmpX = area.x + paddingX;
	float tmpY = textY;

	for (int i = 0; i < ppu->pipeline.fifo.len; ++i) {
		int p = (ppu->pipeline.fifo.out + i) % fifoCapacity;
		fgbPixel fifoPixel = ppu->pipeline.fifo.pixels[p];
		fuiColor color = UIColorFrom4f(FGBColorToLinearColor(fifoPixel.color));
		fuiDrawRect(ui, fuiRectMake(tmpX + (float)i * fifoCellWidth, tmpY, fifoCellWidth, fifoHeight), color);
	}

	const fuiColor fifoFrameColor = UIColorFrom4f(ColorGray);
	fuiDrawRectOutline(ui, fuiRectMake(tmpX, tmpY, fifoWidth, fifoHeight), fifoFrameColor, 2.0f);
	for (int i = 1; i < fifoCapacity; ++i) {
		float cellX = tmpX + (float)i * fifoCellWidth;
		fuiDrawLine(ui, fuiV2(cellX, tmpY), fuiV2(cellX, tmpY + fifoHeight), fifoFrameColor, 1.0f);
	}

	textY += fifoHeight;

	const float switchRowHeight = lineHeight;

	float switchesRowY = textY + paddingY;

	tmpX = UICheckboxRowX(ui, area.x + paddingX);

	Emulator *emu = &app->emulator;

	fgbSystem *system = &emu->system;

	const bool areLayerSwitchesEnabled = emu->isActive;

	bool isBackgroundEnabled = system->ppu.state.isBackgroundEnabled;
	float backgroundWidth = UICheckboxWidth(ui, "Background");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchesRowY, backgroundWidth, switchRowHeight), "Background", &isBackgroundEnabled, areLayerSwitchesEnabled)) {
		system->ppu.state.isBackgroundEnabled = isBackgroundEnabled;
	}
	tmpX += backgroundWidth;

	bool isWindowEnabled = system->ppu.state.isWindowEnabled;
	float windowWidth = UICheckboxWidth(ui, "Window");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchesRowY, windowWidth, switchRowHeight), "Window", &isWindowEnabled, areLayerSwitchesEnabled)) {
		system->ppu.state.isWindowEnabled = isWindowEnabled;
	}
	tmpX += windowWidth;

	bool isSpritesEnabled = system->ppu.state.isSpritesEnabled;
	float spritesWidth = UICheckboxWidth(ui, "Sprites");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchesRowY, spritesWidth, switchRowHeight), "Sprites", &isSpritesEnabled, areLayerSwitchesEnabled)) {
		system->ppu.state.isSpritesEnabled = isSpritesEnabled;
	}

	fuiPopClip(ui);
}
const char *voiceStateLabelMap[] = {
	[fgbVoiceState_Off] = "Off",
	[fgbVoiceState_Powered] = "Pow",
	[fgbVoiceState_Active] = "Act",
	[fgbVoiceState_Muted] = "Sil",
};

const Color4f voiceStateColorMap[] = {
	[fgbVoiceState_Off] = {.r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
	[fgbVoiceState_Powered] = {.r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
	[fgbVoiceState_Active] = {.r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
	[fgbVoiceState_Muted] = {.r = 1.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
};
static void DrawSoundVoice(fuiContext *ui, fgbSystem *system, const bool isActive, const float x, const float y, const float rowHeight, const char *label, const fgbVoiceType voiceType) {
	const fuiTheme *theme = fuiGetTheme(ui);
	const fuiColor foregroundColor = theme->textColor;

	const fgbVoiceState voiceState = fgbGetAudioVoiceState(system, voiceType);
	const uint8_t voiceVolume = (uint8_t)(fgbGetAudioVoiceVolume(system, voiceType) * 100.0f);

	bool isVoiceAudible = !fgbIsAudioVoiceMuted(system, voiceType);
	const float checkboxWidth = UICheckboxWidth(ui, label);
	if (UICheckboxEx(ui, fuiRectMake(x, y, checkboxWidth, rowHeight), label, &isVoiceAudible, isActive)) {
		fgbSetAudioVoiceMute(system, voiceType, !isVoiceAudible);
	}

	// The state word and the volume sit on the checkbox's own baseline, which is the middle of its row
	fuiVec2 stateSize = UITextSize(ui, voiceStateLabelMap[voiceState], 0);
	float textY = y + (rowHeight - stateSize.y) * 0.5f;
	float textX = x + checkboxWidth;


	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", voiceStateLabelMap[voiceState]);
	UIText(ui, textX, textY, UIColorFrom4f(voiceStateColorMap[voiceState]), TextBuffer, 0);
	textX += stateSize.x;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), ", Vol: %u", voiceVolume);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
}

static void DrawSoundState(Application *app, fgbSystem *system, const fuiRect area, const float padding) {
	fuiContext *ui = &app->ui;

	Emulator *emulator = &app->emulator;

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;
	const fuiColor foregroundColor = theme->textColor;

	UIPanel(ui, area, false);
	UIWatermark(ui, &app->uiFont, area, "APU");

	fuiPushClip(ui, area);

	const float paddingX = padding;
	const float paddingY = padding;

	const float contentX = area.x + paddingX;
	const float contentWidth = area.w - paddingX * 2.0f;
	const float rowX = UICheckboxRowX(ui, contentX);

	float rowY = area.y + paddingY;

	// Sound toggle. The label reports what the emulated APU powered up as, the checkbox owns whether any of
	// it reaches the speakers, which are two different things and are shown as such
	const bool isAudioPoweredByTheGame = fgbIsAudioPowered(system);

	bool isSoundEnabled = emulator->isSoundEnabled != 0;
	const char *soundToggleLabel = "Sound";
	const float soundToggleWidth = UICheckboxWidth(ui, soundToggleLabel);
	if (UICheckboxEx(ui, fuiRectMake(rowX, rowY, soundToggleWidth, lineHeight), soundToggleLabel, &isSoundEnabled, true)) {
		emulator->isSoundEnabled = isSoundEnabled;
	}

	// A mute the user asked for outranks whatever the emulated APU is doing, because that is the state they
	// are looking for an explanation of. The hardware's own power state comes back the moment they unmute
	const char *soundStateText = isAudioPoweredByTheGame ? "On " : "Off";
	fuiColor soundStateColor = isAudioPoweredByTheGame ? UIColorFrom4f(ColorGreen) : UIColorFrom4f(ColorRed);
	if (!isSoundEnabled) {
		soundStateText = "Mute";
		soundStateColor = SoundStateMutedTextColor;
	}

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", soundStateText);
	fuiVec2 soundStateTextSize = UITextSize(ui, TextBuffer, 0);
	UIText(ui, contentX + soundToggleWidth, rowY + (lineHeight - soundStateTextSize.y) * 0.5f, soundStateColor, TextBuffer, 0);

	rowY += lineHeight;

	// Master volume, which is the frontend's own output gain rather than anything the emulated APU holds.
	// The slider is only as wide as it needs to be to aim at, and the stereo readout follows it on the row
	const uint8_t masterVolumePercentage = (uint8_t)(emulator->masterVolume * 100.0f);
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%u%%", masterVolumePercentage);

	const fuiRect masterVolumeRow = fuiRectMake(contentX, rowY, contentWidth, lineHeight);
	fuiBeginStackAt(ui, "Master-Volume-Row", fuiAxis_Horizontal, masterVolumeRow, SoundStateVolumeRowSpacing);

	const char *masterVolumeCaption = "Master Volume:";
	const fuiVec2 masterVolumeCaptionSize = UITextSize(ui, masterVolumeCaption, 0);
	const fuiRect masterVolumeCaptionRect = fuiLayoutSlot(ui, masterVolumeCaptionSize.x);
	UIText(ui, masterVolumeCaptionRect.x, masterVolumeCaptionRect.y + (masterVolumeCaptionRect.h - masterVolumeCaptionSize.y) * 0.5f, foregroundColor, masterVolumeCaption, 0);

	const fuiRect masterVolumeSliderRect = fuiLayoutSlot(ui, SoundStateVolumeSliderWidth);

	const float masterVolumeMinimum = 0.0f;
	const float masterVolumeMaximum = 1.0f;
	const float masterVolumeStep = 0.01f;
	const bool masterVolumeUpdatesLive = true;
	bool *noBeginReport = fpl_null;
	bool *noEndReport = fpl_null;
	float masterVolume = emulator->masterVolume;
	if (fuiSliderFloatEx(ui, masterVolumeSliderRect, "Master-Volume", &masterVolume, masterVolumeMinimum, masterVolumeMaximum, masterVolumeStep, masterVolumeUpdatesLive, true, TextBuffer, noBeginReport, noEndReport)) {
		emulator->masterVolume = masterVolume;
	}

	const uint8_t leftVolumePercentage = (uint8_t)(fgbGetAudioSpeakerVolume(system, fgbSpeakerType_Left) * 100.0);
	const uint8_t rightVolumePercentage = (uint8_t)(fgbGetAudioSpeakerVolume(system, fgbSpeakerType_Right) * 100.0);
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "LR: %u %u", leftVolumePercentage, rightVolumePercentage);

	const fuiVec2 stereoTextSize = UITextSize(ui, TextBuffer, 0);
	const fuiRect stereoTextRect = fuiLayoutRemaining(ui);
	UIText(ui, stereoTextRect.x, stereoTextRect.y + (stereoTextRect.h - stereoTextSize.y) * 0.5f, foregroundColor, TextBuffer, 0);

	fuiEndStack(ui);

	rowY += lineHeight;

	// Air between the slider and the voices, so the first one does not read as part of the row above it
	rowY += lineHeight * SoundStateVoiceGapRowCount;

	const float voiceRowSpacing = lineHeight * SoundStateVoiceSpacingRowCount;

	DrawSoundVoice(ui, system, emulator->isActive, rowX, rowY, lineHeight, "Voice 1 (Sweep): ", fgbVoiceType_Sweep);
	rowY += lineHeight + voiceRowSpacing;

	DrawSoundVoice(ui, system, emulator->isActive, rowX, rowY, lineHeight, "Voice 2 (Tone):  ", fgbVoiceType_Tone);
	rowY += lineHeight + voiceRowSpacing;

	DrawSoundVoice(ui, system, emulator->isActive, rowX, rowY, lineHeight, "Voice 3 (Wave):  ", fgbVoiceType_Wave);
	rowY += lineHeight + voiceRowSpacing;

	DrawSoundVoice(ui, system, emulator->isActive, rowX, rowY, lineHeight, "Voice 4 (Noise): ", fgbVoiceType_Noise);

	fuiPopClip(ui);
}

static void DrawCPUState(Application *app, fgbSystem *system, const fuiRect area, const float padding) {
	Emulator *emulator = &app->emulator;

	const fgbCPU *cpu = &system->cpu;
	const fgbCPURegisters *r = &cpu->registers;
	const fgbPPU *ppu = &system->ppu;
	const fgbEmulationState state = system->state;

	fuiContext *ui = &app->ui;

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;
	const fuiColor foregroundColor = theme->textColor;

	UIPanel(ui, area, false);
	UIWatermark(ui, &app->uiFont, area, "CPU");

	fuiPushClip(ui, area);

	const float paddingX = padding;
	const float paddingY = padding;

	const char *stateText;
	if (state == fgbEmulationState_Step)
		stateText = "Step";
	else if (state == fgbEmulationState_Paused)
		stateText = "Paused";
	else if (state == fgbEmulationState_Running)
		stateText = "Running";
	else if (state == fgbEmulationState_Error)
		stateText = "Error";
	else if (state == fgbEmulationState_MicroStep)
		stateText = "MicroStep";
	else if (state == fgbEmulationState_Breakpoint)
		stateText = "Breakpoint";
	else
		stateText = "Unknown";

	float textX = area.x + paddingX;
	float textY = area.y + paddingY;

	float tmpX;

	fuiVec2 textSize;
	fuiColor flagColor;

	if (state == fgbEmulationState_Breakpoint) {
		fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "State: %s, %s", stateText, fgbGetBreakpointTypeLabel(emulator->lastBreakpointType));
	} else {
		fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "State: %s", stateText);
	}
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	const char *gameboyTypeName = fgbGetCoreTypeName(emulator->system.coreType);
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", gameboyTypeName);
	textSize = UITextSize(ui, TextBuffer, 0);
	float gbtX = area.x + area.w - textSize.x - paddingX;
	UIText(ui, gbtX, textY, foregroundColor, TextBuffer, 0);

	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "M-Cycles: %03llu, T-Cycles: %llu, Frames: %llu", cpu->state.currentMemoryCycles, cpu->state.totalTickCycles, ppu->state.frameCount);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textY += lineHeight;

	textY += lineHeight;

	// Flags Label
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Flags: ");
	textSize = UITextSize(ui, TextBuffer, 0);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	// Z Flag
	tmpX = textX + textSize.x;
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Z ");
	textSize = UITextSize(ui, TextBuffer, 0);
	if (!r->f.zeroFlag)
		flagColor = UIColorFrom4f(ColorRed);
	else
		flagColor = UIColorFrom4f(ColorGreen);
	UIText(ui, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.x;

	// N Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "N ");
	textSize = UITextSize(ui, TextBuffer, 0);
	if (!r->f.negativeFlag)
		flagColor = UIColorFrom4f(ColorRed);
	else
		flagColor = UIColorFrom4f(ColorGreen);
	UIText(ui, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.x;

	// H Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "H ");
	textSize = UITextSize(ui, TextBuffer, 0);
	if (!r->f.halfCarryFlag)
		flagColor = UIColorFrom4f(ColorRed);
	else
		flagColor = UIColorFrom4f(ColorGreen);
	UIText(ui, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.x;

	// C Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "C");
	textSize = UITextSize(ui, TextBuffer, 0);
	if (!r->f.fullCarryFlag)
		flagColor = UIColorFrom4f(ColorRed);
	else
		flagColor = UIColorFrom4f(ColorGreen);
	UIText(ui, tmpX, textY, flagColor, TextBuffer, 0);

	textY += lineHeight;

	textY += lineHeight;

	// Registers

	const char *spaceForNextRegisterLabel = "AF: $%02X $%02X";
	textSize = UITextSize(ui, spaceForNextRegisterLabel, 0);
	const float spaceToNextRegister = textSize.x;

	const float startTextX = textX;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "AF: $%02X $%02X", r->a, r->f.flags);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "BC: $%02X $%02X", r->b, r->c);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	textX = startTextX;
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "DE: $%02X $%02X", r->d, r->e);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "HL: $%02X $%02X", r->h, r->l);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	textX = startTextX;
	textY += lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "SP: $%04X", r->sp);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "PC: $%04X", r->pc);
	UIText(ui, textX, textY, foregroundColor, TextBuffer, 0);

	fuiPopClip(ui);
}
typedef union {
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};
	uint32_t rgba;
} RGBAPixel;

static inline uint32_t GameboyPixelToRGBA(const fgbColor *pixel) {
	RGBAPixel result = { 0 };
	result.r = pixel->r;
	result.g = pixel->g;
	result.b = pixel->b;
	result.a = 255;
	return result.rgba;
}

static void ClearPixelsTexture(Texture *texture) {
	uint32_t sourceLineWidth = texture->width * 4;
	uint32_t targetLineWidth = texture->width * 4;
	fplMemoryClear(texture->pixels, targetLineWidth * texture->height);
	texture->hasPixels = false;
}

static void TransferPixelsToTexture(const fgbColor *sourcePixels, const uint32_t sourceWidth, const uint32_t sourceHeight, Texture *texture) {
	fplAssert(sourceWidth <= texture->width);
	fplAssert(sourceHeight <= texture->height);
	uint32_t sourceLineWidth = sourceWidth * 4;
	uint32_t targetLineWidth = texture->width * 4;
	fplMemoryClear(texture->pixels, targetLineWidth * texture->height);
	for (uint32_t y = 0; y < sourceHeight; ++y) {
		uint32_t *targetScanline = texture->pixels + y * texture->width;
		for (uint32_t x = 0; x < sourceWidth; ++x) {
			uint32_t *targetPixel = targetScanline + x;
			const fgbColor *sourcePixel = sourcePixels + (y * sourceWidth + x);
			*targetPixel = GameboyPixelToRGBA(sourcePixel);
		}
	}
	texture->hasPixels = true;
}

static const AppShader* GetActiveAppShader(const Application *app) {
	switch (app->activeShaderType) {
		case AppShaderType_Bilinear:
			return &app->bilinearShader;
		case AppShaderType_HQ2X:
			return &app->hq2xShader;
		case AppShaderType_HQ4X:
			return &app->hq4xShader;
		case AppShaderType_BicubicHermite:
			return &app->bicubicHermiteShader;
		case AppShaderType_BicubicLagrange:
			return &app->bicubicLagrangeShader;
		case AppShaderType_CatmullRom4:
			return &app->catmullRom4Shader;
		default:
			return &app->nearestShader;
	}
}
// The frame around the emulator display. It deliberately draws NO fill while a game is running, because
// the display itself was already rendered straight to the framebuffer before the interface was built and
// a panel background would paint straight over it.
static void DrawDisplayFrame(Application *app, const fuiRect area) {
	fuiContext *ui = &app->ui;

	const fuiTheme *theme = fuiGetTheme(ui);
	const float border = 1.0f;

	if (app->emulator.isActive) {
		if (app->isDebugEnabled) {
			fuiDrawRectOutline(ui, area, theme->panelBorderColor, theme->panelBorderThickness);
		}
		return;
	}

	if (app->isDebugEnabled) {
		UIPanel(ui, area, false);
	}

	fuiRect inner = fuiRectMake(area.x + border * 2.0f, area.y + border * 2.0f, area.w - border * 4.0f, area.h - border * 4.0f);
	fuiDrawRect(ui, inner, UIColorFrom4f(ColorBlack));

	const char *insertGameText = "No Game Pak loaded";
	const float placeholderHeight = theme->fontHeight * 2.0f;
	fuiVec2 textSize = fuiMeasureText(ui, insertGameText, 0, placeholderHeight);
	float textX = area.x + (area.w - textSize.x) * 0.5f;
	float textY = area.y + (area.h - textSize.y) * 0.5f;
	fuiDrawText(ui, insertGameText, 0, fuiV2(textX, textY), placeholderHeight, UIColorFrom4f(ColorWhite));
}

// The emulator display is drawn by a shader and therefore cannot go through the user interface geometry.
// It is rendered straight to the framebuffer BEFORE the interface is, so the panel around it draws no fill
// and anything the interface puts on top of it - a modal backdrop above all - still covers it.
static void RenderDisplayTexture(const Application *app, const fuiRect area, const float aspect) {
	if (!app->emulator.isActive) {
		return;
	}

	const Texture *tex = &app->displayTexture;
	const float uMin = 0.0f;
	const float uMax = tex->uScale;
	const float vMin = 0.0f;
	const float vMax = tex->vScale;
	const float border = 1.0f;

	// The letterbox bars would otherwise show the window clear colour straight through
	RendererDrawFilledQuad(area.x, area.y, area.w, area.h, ColorBlack);

	const Vec2f screenSize = V2fInit(area.w, area.h);
	const Viewport4f displayView = VP4fComputeByAspect(screenSize, aspect);

	float boyWidth = displayView.w;
	float boyHeight = displayView.h;
	float boyX = area.x + displayView.x;
	float boyY = area.y + displayView.y;

	if (app->isDebugEnabled) {
		boyX += border * 2.0f;
		boyY += border * 2.0f;
		boyWidth -= border * 4.0f;
		boyHeight -= border * 4.0f;
	}

	const AppShader *appShader;
	if (app->isShaderSupported && app->activeShaderType != AppShaderType_None && (appShader = GetActiveAppShader(app)) != fpl_null) {
		const ShaderProgram *shaderProgram = &appShader->program;

		const int textureSamplerLocation = appShader->textureSamplerLocation;
		const int textureSizeLocation = appShader->textureSizeLocation;
		const int imageSizeLocation = appShader->imageSizeLocation;

		const Vec2f textureSize = V2fInit((float)tex->width, (float)tex->height);
		const Vec2f imageSize = V2fInit((float)FGB_DISPLAY_WIDTH, (float)FGB_DISPLAY_HEIGHT);

		RendererShaderBind(shaderProgram);

		RendererShaderUniform1i(shaderProgram, textureSamplerLocation, 0);
		RendererShaderUniformVec2f(shaderProgram, textureSizeLocation, textureSize);
		RendererShaderUniformVec2f(shaderProgram, imageSizeLocation, imageSize);

		// The shaders take the image relative UV in 0..1 and rescale it into the padded atlas themselves, so the top of the quad is v 0 and the bottom is v 1
		const float shaderImageUMin = 0.0f;
		const float shaderImageVMin = 0.0f;
		const float shaderImageUMax = 1.0f;
		const float shaderImageVMax = 1.0f;
		RendererDrawTexturedQuad(tex->id, boyX, boyY, boyWidth, boyHeight, ColorWhite, shaderImageUMin, shaderImageVMin, shaderImageUMax, shaderImageVMax);

		RendererShaderUnbind(shaderProgram);
	} else {
		RendererDrawTexturedQuad(tex->id, boyX, boyY, boyWidth, boyHeight, ColorWhite, uMin, vMin, uMax, vMax);
	}
}

static void DrawBackgroundMap(fuiContext *ui, const Application *app, const fuiRect area) {
	const Emulator *emulator = &app->emulator;

	const Texture *tex = &app->backgroundMapTexture;

	UIPanel(ui, area, true);

	const uint8_t gridCountX = 32;
	const uint8_t gridCountY = 32;

	const float insideMargin = 4.0f;
	const float insideX = area.x + insideMargin;
	const float insideY = area.y + insideMargin;
	const float insideWidth = area.w - insideMargin * 2.0f;

	const float tileSize = insideWidth / (float)gridCountX;

	const float totalTilesWidth = (float)gridCountX * tileSize;
	const float totalTilesHeight = (float)gridCountY * tileSize;

	const float tilesX = insideX;
	const float tilesY = insideY;

	fuiImageDesc mapImage = fplZeroInit;
	mapImage.texture = (fuiTextureId)tex->id;
	mapImage.textureSize = fuiV2((float)tex->width, (float)tex->height);
	mapImage.uvMin = fuiV2(0.0f, 0.0f);
	mapImage.uvMax = fuiV2(tex->uScale, tex->vScale);
	mapImage.scaleMode = fuiImageScaleMode_Stretch;
	fuiImage(ui, fuiRectMake(tilesX, tilesY, totalTilesWidth, totalTilesHeight), &mapImage);

	const fuiColor gridLineColor = fuiColorRGBA(0.1f, 0.1f, 0.1f, 0.25f);
	for (uint8_t i = 0; i <= gridCountX; ++i) {
		float gridLineX = tilesX + (float)i * tileSize;
		fuiDrawLine(ui, fuiV2(gridLineX, tilesY), fuiV2(gridLineX, tilesY + totalTilesHeight), gridLineColor, 1.0f);
	}
	for (uint8_t i = 0; i <= gridCountY; ++i) {
		float gridLineY = tilesY + (float)i * tileSize;
		fuiDrawLine(ui, fuiV2(tilesX, gridLineY), fuiV2(tilesX + totalTilesWidth, gridLineY), gridLineColor, 1.0f);
	}

	const float pixelsPerTile = tileSize / 8.0f;

	const uint8_t scx = emulator->system.ppu.backgroundMap.scrollX;
	const uint8_t scy = emulator->system.ppu.backgroundMap.scrollY;

	const float scrollWidth = pixelsPerTile * (float)FGB_DISPLAY_WIDTH;
	const float scrollHeight = pixelsPerTile * (float)FGB_DISPLAY_HEIGHT;
	const float scrollX = (float)scx * pixelsPerTile;
	const float scrollY = (float)scy * pixelsPerTile;

	const float maxX = totalTilesWidth;
	const float maxY = totalTilesHeight;

	const bool isHorizontalWrap = (scrollX + scrollWidth) > maxX;
	const bool isVerticalWrap = (scrollY + scrollHeight) > maxY;

	const float xDepth = (scrollX + scrollWidth) - maxX;
	const float yDepth = (scrollY + scrollHeight) - maxY;
	const float xRemaining = scrollWidth - xDepth;
	const float yRemaining = scrollHeight - yDepth;

	const fuiColor scrollBoxColor = UIColorFrom4f(ColorRed);
	const float scrollBoxThickness = 2.0f;

	if (isHorizontalWrap && isVerticalWrap) {
		// Bottom Right
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY + scrollY, xRemaining, yRemaining), scrollBoxColor, scrollBoxThickness);
		// Bottom Left
		fuiDrawRectOutline(ui, fuiRectMake(tilesX, tilesY + scrollY, xDepth, yRemaining), scrollBoxColor, scrollBoxThickness);
		// Top Right
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY, xRemaining, yDepth), scrollBoxColor, scrollBoxThickness);
		// Top Left
		fuiDrawRectOutline(ui, fuiRectMake(tilesX, tilesY, xDepth, yDepth), scrollBoxColor, scrollBoxThickness);
	} else if (isHorizontalWrap && !isVerticalWrap) {
		// Right
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY + scrollY, xRemaining, scrollHeight), scrollBoxColor, scrollBoxThickness);
		// Left
		fuiDrawRectOutline(ui, fuiRectMake(tilesX, tilesY + scrollY, xDepth, scrollHeight), scrollBoxColor, scrollBoxThickness);
	} else if (isVerticalWrap && !isHorizontalWrap) {
		// Bottom
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY + scrollY, scrollWidth, yRemaining), scrollBoxColor, scrollBoxThickness);
		// Top
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY, scrollWidth, yDepth), scrollBoxColor, scrollBoxThickness);
	} else {
		fuiDrawRectOutline(ui, fuiRectMake(tilesX + scrollX, tilesY + scrollY, scrollWidth, scrollHeight), scrollBoxColor, scrollBoxThickness);
	}
}

static void DrawTiles(fuiContext *ui, const Texture *tex, const fuiRect area, const float aspect) {
	const float border = 1.0f;

	const Vec2f size = V2fInit(area.w - border * 4.0f, area.h - border * 4.0f);

	const Viewport4f vp = VP4fComputeByAspect(size, aspect);

	const float rx = area.x + border * 2.0f + vp.x;
	const float ry = area.y + border * 2.0f + vp.y;
	const float rw = vp.w;

	const uint8_t gridCountX = 16;
	const uint8_t gridCountY = 24;
	const float tileSize = rw / (float)gridCountX;

	const float totalTilesWidth = (float)gridCountX * tileSize;
	const float totalTilesHeight = (float)gridCountY * tileSize;

	const fuiColor gridLineColor = fuiColorRGBA(0.1f, 0.1f, 0.1f, 0.25f);

	UIPanel(ui, area, true);

	fuiImageDesc tileImage = fplZeroInit;
	tileImage.texture = (fuiTextureId)tex->id;
	tileImage.textureSize = fuiV2((float)tex->width, (float)tex->height);
	tileImage.uvMin = fuiV2(0.0f, 0.0f);
	tileImage.uvMax = fuiV2(tex->uScale, tex->vScale);
	tileImage.scaleMode = fuiImageScaleMode_Stretch;
	fuiImage(ui, fuiRectMake(rx, ry, totalTilesWidth, totalTilesHeight), &tileImage);

	for (uint8_t i = 0; i <= gridCountX; ++i) {
		float gridLineX = rx + (float)i * tileSize;
		fuiDrawLine(ui, fuiV2(gridLineX, ry), fuiV2(gridLineX, ry + totalTilesHeight), gridLineColor, 1.0f);
	}
	for (uint8_t i = 0; i <= gridCountY; ++i) {
		float gridLineY = ry + (float)i * tileSize;
		fuiDrawLine(ui, fuiV2(rx, gridLineY), fuiV2(rx + totalTilesWidth, gridLineY), gridLineColor, 1.0f);
	}
}

static void DrawBreakpoints(Application *app, const fuiRect area) {
	fuiContext *ui = &app->ui;

	Emulator *emulator = &app->emulator;

	fgbSystem *system = &emulator->system;

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;
	const float checkboxHeight = lineHeight;

	fuiPushClip(ui, area);

	const float checkboxX = UICheckboxRowX(ui, area.x + DebugPanelPadding);
	float currentY = area.y + DebugPanelPadding;

	const uint32_t checkboxCount = fplArrayCount(emulator->config.debug.breakpoints.filter);

	for (uint32_t checkboxIndex = 0; checkboxIndex < checkboxCount; ++checkboxIndex) {
		fgbBreakpointType type = fgbBreakpointType_First + checkboxIndex;
		const char *label = fgbGetBreakpointTypeLabel(type);
		bool isChecked = emulator->config.debug.breakpoints.filter[type];
		float checkboxWidth = UICheckboxWidth(ui, label);
		if (UICheckboxEx(ui, fuiRectMake(checkboxX, currentY, checkboxWidth, checkboxHeight), label, &isChecked, emulator->isActive)) {
			emulator->config.debug.breakpoints.filter[type] = isChecked;
			fgbBreakpointEnable(system, type, isChecked);
		}
		currentY += checkboxHeight;
	}

	fuiPopClip(ui);
}

static void DrawPalette(fuiContext *ui, const float x, const float y, const float cellWidth, const float cellHeight, const Color4f *colors, const uint8_t colorCount) {
	const float totalWidth = cellWidth * (float)colorCount;
	const float totalHeight = cellHeight;

	const float border = 1.0f;

	const float colW = cellWidth - border * 2.0f;
	const float colH = cellHeight - border * 2.0f;

	const fuiColor frameColor = UIColorFrom4f(ColorGray);

	fuiDrawRectOutline(ui, fuiRectMake(x + border * 0.5f, y + border * 0.5f, totalWidth - border, totalHeight - border), frameColor, 1.0f);

	for (uint8_t colorIndex = 1; colorIndex < colorCount; ++colorIndex) {
		float lineX = x + (float)colorIndex * cellWidth;
		fuiDrawLine(ui, fuiV2(lineX, y), fuiV2(lineX, y + cellHeight), frameColor, 1.0f);
	}
	for (uint8_t colorIndex = 0; colorIndex < colorCount; ++colorIndex) {
		float colX = x + (float)colorIndex * cellWidth + border;
		float colY = y + border;
		fuiDrawRect(ui, fuiRectMake(colX, colY, colW, colH), UIColorFrom4f(colors[colorIndex]));
	}
}
static void DrawPalettes(Application *app, const fuiRect area) {
	fuiContext *ui = &app->ui;

	Emulator *emulator = &app->emulator;

	fgbSystem *system = &emulator->system;

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;

	const fuiColor foregroundColor = theme->textColor;

	const float paletteHeight = lineHeight * 1.5f;

	const float cellWidth = area.w / 18.0f;
	const float cellHeight = paletteHeight;

	const float paletteTypeSpacing = 10.0f;

	const float spacing = lineHeight * 0.5f;

	const float px = area.x;
	float py = area.y;

	fuiPushClip(ui, area);

	const fuiVec2 maxLabelSize = UITextSize(ui, "Palette: ", 5);

	size_t textLen;
	const char *text;
	float textX, textY;
	float palX, palY, palWidth;

	static Color4f paletteColorsSys[2];
	static Color4f paletteColorsBg[4];
	static Color4f paletteColorsObj0[4];
	static Color4f paletteColorsObj1[4];

	static Color4f sysColors[4];
	static Color4f bgColors[4];
	static Color4f obj0Colors[4];
	static Color4f obj1Colors[4];

	static Color4f cgbBGColors[8][4];
	static Color4f cgbObjColors[8][4];

	for (uint8_t colorIndex = 0; colorIndex < 4; ++colorIndex) {
		sysColors[colorIndex] = FGBColorToLinearColor(system->ppu.currentMonochromeColors.system[colorIndex]);
		bgColors[colorIndex] = FGBColorToLinearColor(system->ppu.currentMonochromeColors.background[colorIndex]);
		obj0Colors[colorIndex] = FGBColorToLinearColor(system->ppu.currentMonochromeColors.sprite0[colorIndex]);
		obj1Colors[colorIndex] = FGBColorToLinearColor(system->ppu.currentMonochromeColors.sprite1[colorIndex]);
	}

	for (uint8_t colorIndex = 0; colorIndex < 2; ++colorIndex) {
		paletteColorsSys[colorIndex] = FGBColorToLinearColor(system->systemMonochromeColors.system[colorIndex]);
	}
	for (uint8_t colorIndex = 0; colorIndex < 4; ++colorIndex) {
		paletteColorsBg[colorIndex] = FGBColorToLinearColor(system->systemMonochromeColors.background[colorIndex]);
		paletteColorsObj0[colorIndex] = FGBColorToLinearColor(system->systemMonochromeColors.sprite0[colorIndex]);
		paletteColorsObj1[colorIndex] = FGBColorToLinearColor(system->systemMonochromeColors.sprite1[colorIndex]);
	}

	for (uint8_t lineIndex = 0; lineIndex < 8; ++lineIndex) {
		for (uint8_t colorIndex = 0; colorIndex < 4; ++colorIndex) {
			cgbBGColors[lineIndex][colorIndex] = FGBColorToLinearColor(system->cgbState.currentPalette.bg.grid[lineIndex][colorIndex]);
			cgbObjColors[lineIndex][colorIndex] = FGBColorToLinearColor(system->cgbState.currentPalette.obj.grid[lineIndex][colorIndex]);
		}
	}

	// Palette Label
	text = "Palette: ";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = py + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);

	// The four monochrome palettes are one choice out of four, which is what a radio group is
	const float checkboxHeight = lineHeight;
	const bool isPaletteChoiceEnabled = emulator->isActive;
	float checkboxX = UICheckboxRowX(ui, px + maxLabelSize.x);
	const float checkboxY = py;
	int32_t paletteChoice = (int32_t)emulator->paletteType;

	float dmgWidth = UICheckboxWidth(ui, "DMG");
	if (UIRadioEx(ui, fuiRectMake(checkboxX, checkboxY, dmgWidth, checkboxHeight), "DMG", &paletteChoice, ColorPaletteType_DMG, isPaletteChoiceEnabled)) {
		emulator->paletteType = ColorPaletteType_DMG;
		fgbSetColorPalette(system, &FGB_DEFAULT_DMG_COLORS);
	}
	checkboxX += dmgWidth;

	float mgbWidth = UICheckboxWidth(ui, "MGB");
	if (UIRadioEx(ui, fuiRectMake(checkboxX, checkboxY, mgbWidth, checkboxHeight), "MGB", &paletteChoice, ColorPaletteType_MGB, isPaletteChoiceEnabled)) {
		emulator->paletteType = ColorPaletteType_MGB;
		fgbSetColorPalette(system, &FGB_DEFAULT_MGB_COLORS);
	}
	checkboxX += mgbWidth;

	float sgbWidth = UICheckboxWidth(ui, "SGB");
	if (UIRadioEx(ui, fuiRectMake(checkboxX, checkboxY, sgbWidth, checkboxHeight), "SGB", &paletteChoice, ColorPaletteType_SGB, isPaletteChoiceEnabled)) {
		emulator->paletteType = ColorPaletteType_SGB;
		fgbSetColorPalette(system, &FGB_DEFAULT_SGB_COLORS);
	}
	checkboxX += sgbWidth;

	float blueWidth = UICheckboxWidth(ui, "Blue");
	if (UIRadioEx(ui, fuiRectMake(checkboxX, checkboxY, blueWidth, checkboxHeight), "Blue", &paletteChoice, ColorPaletteType_Blue, isPaletteChoiceEnabled)) {
		emulator->paletteType = ColorPaletteType_Blue;
		fgbSetColorPalette(system, &BlueMonochromeColors);
	}

	py += (paletteHeight + spacing);

	// Active Palette
	palX = px + maxLabelSize.x;
	palY = py;
	palWidth = cellWidth * 2;
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, paletteColorsSys, 2);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, paletteColorsBg, 4);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, paletteColorsObj0, 4);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, paletteColorsObj1, 4);

	py += paletteHeight + spacing;
	py += paletteHeight + spacing;

	// System Palette
	palX = px + maxLabelSize.x;
	palY = py;
	text = "Sys";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, sysColors, 2);

	py += (paletteHeight + spacing);

	// Background Palette
	palX = px + maxLabelSize.x;
	palY = py;
	text = "BG";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, bgColors, 4);

	py += (paletteHeight + spacing);

	// Obj0 Palette
	palX = px + maxLabelSize.x;
	palY = py;
	text = "OBJ-0";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, obj0Colors, 4);

	py += (paletteHeight + spacing);

	// Obj1 Palette
	palX = px + maxLabelSize.x;
	palY = py;
	text = "OBJ-1";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	DrawPalette(ui, palX, palY, cellWidth, cellHeight, obj1Colors, 4);

	py += (paletteHeight + spacing);

	// CGB Lines/Colums Palettes
	palX = px + maxLabelSize.x;
	palY = py;
	text = "CGB-BG";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	for (uint8_t lineIndex = 0; lineIndex < 8; ++lineIndex) {
		DrawPalette(ui, palX, palY, cellWidth, cellHeight, cgbBGColors[lineIndex], 4);
		palY += (paletteHeight + spacing);
	}

	const float blockWidth = cellWidth * 4 + paletteTypeSpacing;

	palX = px + maxLabelSize.x + blockWidth + maxLabelSize.x;
	palY = py;
	text = "CGB-OBJ";
	textLen = fplGetStringLength(text);
	textX = px + maxLabelSize.x + blockWidth;
	textY = palY + (cellHeight - maxLabelSize.y) * 0.5f;
	UIText(ui, textX, textY, foregroundColor, text, textLen);
	for (uint8_t lineIndex = 0; lineIndex < 8; ++lineIndex) {
		DrawPalette(ui, palX, palY, cellWidth, cellHeight, cgbObjColors[lineIndex], 4);
		palY += (paletteHeight + spacing);
	}

	fuiPopClip(ui);
}

static char performanceLabelBuffer[1024] = { 0 };

static void DrawPerformanceCounter(fuiContext *ui, const float x, const float y, const char *name, const fuiColor foregroundColor, const PerformanceCounter *counter) {
	double avgTimeMs = GetPerformanceCounterAvg(counter) * 1000.0;
	fplStringFormat(performanceLabelBuffer, fplArrayCount(performanceLabelBuffer), "%s: %.5f / %.5f / %.5f ms [%zu]", name, counter->minSecs * 1000.0, counter->maxSecs * 1000.0, avgTimeMs, counter->count);

	size_t textLen = fplGetStringLength(performanceLabelBuffer);
	UIText(ui, x, y, foregroundColor, performanceLabelBuffer, textLen);
}
static void ResumeGameboy(Application *app, fgbSystem *system) {
	Emulator *emulator = &app->emulator;
	fgbResume(system);
	emulator->isFrameStepActive = false;
	emulator->isMicroStepActive = false;
	HighlightScrollDisassembly(app);
	WakeupEmulatorThread(emulator);
}
static void DrawPerformanceMetrics(fuiContext *ui, const Application *app, const fuiRect area) {
	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;

	const PerformanceMetrics *metrics = &app->emulator.performanceMetrics;

	const fuiColor foregroundColor = theme->textColor;

	UIPanel(ui, area, false);
	fuiPushClip(ui, area);

	const float padding = 8.0f;

	const float textX = area.x + padding;
	float textY = area.y + padding;

	DrawPerformanceCounter(ui, textX, textY, "Frame", foregroundColor, &metrics->frameTime);
	textY += lineHeight;
	DrawPerformanceCounter(ui, textX, textY, "Textures Upload", foregroundColor, &metrics->texturesUpload);
	textY += lineHeight;
	DrawPerformanceCounter(ui, textX, textY, "Audio Thread", foregroundColor, &metrics->audioThread);
	textY += lineHeight;
	DrawPerformanceCounter(ui, textX, textY, "Audio Read", foregroundColor, &metrics->audioReadSamples);
	textY += lineHeight;
	DrawPerformanceCounter(ui, textX, textY, "Audio Output", foregroundColor, &metrics->audioOutputSamples);
	textY += lineHeight;
	DrawPerformanceCounter(ui, textX, textY, "Emulator Tick", foregroundColor, &metrics->emulatorTick);

	fuiPopClip(ui);
}

static char dateTimeFormatBuffer[32];

//
// Debug layout
//
// Every rectangle of the debugger, worked out before anything is drawn. The emulator display has to be
// rendered before the interface is built, so where it goes cannot be a by-product of building it.
//

typedef struct {
	fuiRect leftTabHeader;
	fuiRect leftTabContent;

	fuiRect cartInfo;
	fuiRect displayState;
	fuiRect soundState;
	fuiRect shaderControl;
	fuiRect display;
	fuiRect userButtons;

	fuiRect actions;
	fuiRect cpuState;
	fuiRect switches;
	fuiRect rightTabHeader;
	fuiRect rightTabContent;

	fuiRect leftSplitterGrip;
	fuiRect rightSplitterGrip;

	float actionButtonWidth;
	float actionButtonHeight;
	float actionsPadding;
	float actionsSpacing;

	float userButtonWidth;
	float userButtonHeight;
	float userButtonsPadding;
	float userButtonSpacing;

	bool isShaderControlVisible;
} DebugLayout;

// Narrowest either side column may be dragged to
static const float DebugSideColumnMinWidth = 260.0f;

// Narrowest the middle column may become, so the display and its readouts keep room
static const float DebugMiddleColumnMinWidth = 320.0f;

// How wide the grabbable strip on a column boundary is
static const float DebugSplitterGripWidth = 6.0f;

// How thick the accent line drawn on a hovered or dragged grip is
static const float DebugSplitterHighlightThickness = 2.0f;

// Added on top of the right column's seeded width, so the action buttons start out wide enough for their captions
// rather than only after the splitter has been dragged out by hand
static const float DebugRightColumnExtraSeedWidth = 75.0f;

// Seeds the column widths the first time round and keeps them inside what the window can hold, which is
// what stops a window dragged narrow from collapsing the middle column or overlapping the two sides
static void ClampDebugColumnWidths(Application *app, const float w) {
	if (app->leftPanelWidth <= 0.0f) {
		app->leftPanelWidth = fplMax(w * 0.325f, 300.0f);
	}
	if (app->rightPanelWidth <= 0.0f) {
		const float seededRightPanelWidth = fplMax(w * 0.35f, 300.0f);
		app->rightPanelWidth = seededRightPanelWidth + DebugRightColumnExtraSeedWidth;
	}

	// A window too narrow to honour both minimums AND the middle cannot be satisfied, so the sides keep
	// their minimum and the middle is the one that gives, rather than the columns overlapping each other
	float availableForSides = w - DebugMiddleColumnMinWidth;
	const float smallestBothSidesFitIn = DebugSideColumnMinWidth * 2.0f;
	if (availableForSides < smallestBothSidesFitIn) {
		availableForSides = smallestBothSidesFitIn;
	}

	const float widestLeftMayBe = availableForSides - DebugSideColumnMinWidth;
	app->leftPanelWidth = fuiClampF(app->leftPanelWidth, DebugSideColumnMinWidth, widestLeftMayBe);

	const float widestRightMayBe = availableForSides - app->leftPanelWidth;
	app->rightPanelWidth = fuiClampF(app->rightPanelWidth, DebugSideColumnMinWidth, widestRightMayBe);
}

static DebugLayout ComputeDebugLayout(const Application *app, const float w, const float h, const float lineHeight) {
	DebugLayout layout = fplZeroInit;

	const float borderThickness = 1.5f;

	const float leftSideWidth = app->leftPanelWidth;
	const float rightSideWidth = app->rightPanelWidth;
	const float middleWidth = w - (leftSideWidth + rightSideWidth);

	const float leftSideX = 0.0f;
	const float middleX = leftSideWidth;
	const float rightSideX = w - rightSideWidth;

	const float tabHeaderHeight = lineHeight * 1.5f;

	// Left column, one tab control over the full window height
	layout.leftTabHeader = fuiRectMake(leftSideX, 0.0f, leftSideWidth, tabHeaderHeight);
	layout.leftTabContent = fuiRectMake(leftSideX, tabHeaderHeight, leftSideWidth, h - tabHeaderHeight);

	// Right column, stacked from the top down
	const uint8_t actionAreaButtonCount = 5;
	layout.actionsPadding = 1.0f;
	layout.actionsSpacing = 2.0f;
	const float actionsHeight = 1.5f * lineHeight;
	layout.actions = fuiRectMake(rightSideX, 0.0f, rightSideWidth, actionsHeight);
	layout.actionButtonWidth = ((rightSideWidth - (layout.actionsPadding * 2.0f) - layout.actionsSpacing * (float)(actionAreaButtonCount - 1)) / (float)actionAreaButtonCount);
	layout.actionButtonHeight = actionsHeight - (layout.actionsPadding * 2.0f);

	const float cpuStateLineCount = 8.0f;
	const float cpuStateHeight = cpuStateLineCount * lineHeight + DebugPanelPadding * 2.0f;
	layout.cpuState = fuiRectMake(rightSideX, actionsHeight, rightSideWidth, cpuStateHeight);

	const float switchesHeight = lineHeight + DebugPanelPadding * 2.0f + borderThickness * 2.0f;
	layout.switches = fuiRectMake(rightSideX, actionsHeight + cpuStateHeight, rightSideWidth, switchesHeight);

	const float rightTabY = actionsHeight + cpuStateHeight + switchesHeight;
	const float rightTabHeight = h - rightTabY;
	layout.rightTabHeader = fuiRectMake(rightSideX, rightTabY, rightSideWidth, tabHeaderHeight);
	layout.rightTabContent = fuiRectMake(rightSideX, rightTabY + tabHeaderHeight, rightSideWidth, rightTabHeight - tabHeaderHeight);

	// Middle column, stacked from the top down
	const int cartInfoLineCount = 3;
	const float cartInfoHeight = (float)cartInfoLineCount * lineHeight + DebugPanelPadding * 2.0f;
	layout.cartInfo = fuiRectMake(middleX, 0.0f, middleWidth, cartInfoHeight);

	const float displayStateHeight = DisplayStatePanelHeight(lineHeight);
	layout.displayState = fuiRectMake(middleX, cartInfoHeight, middleWidth, displayStateHeight);

	const float soundStateHeight = SoundStatePanelHeight(lineHeight);
	layout.soundState = fuiRectMake(middleX, cartInfoHeight + displayStateHeight, middleWidth, soundStateHeight);

	layout.isShaderControlVisible = app->isShaderSupported;
	const float shaderControlHeight = layout.isShaderControlVisible ? 1.5f * lineHeight : 0.0f;
	const float shaderControlY = cartInfoHeight + displayStateHeight + soundStateHeight;
	layout.shaderControl = fuiRectMake(middleX, shaderControlY, middleWidth, shaderControlHeight);

	const uint8_t userButtonCount = 2;
	layout.userButtonSpacing = 2.0f;
	layout.userButtonsPadding = 4.0f;
	const float userButtonsHeight = 1.5f * lineHeight;
	layout.userButtons = fuiRectMake(middleX, h - userButtonsHeight, middleWidth, userButtonsHeight);
	layout.userButtonWidth = ((middleWidth - (layout.userButtonsPadding * 2.0f) - layout.userButtonSpacing * (float)(userButtonCount - 1)) / (float)userButtonCount);
	layout.userButtonHeight = userButtonsHeight - (layout.userButtonsPadding * 2.0f);

	const float displayY = shaderControlY + shaderControlHeight;
	const float displayHeight = h - displayY - userButtonsHeight;
	layout.display = fuiRectMake(middleX, displayY, middleWidth, displayHeight);

	// Straddling the boundary rather than sitting beside it, so the grab target is the seam the eye reads
	const float gripHalfWidth = DebugSplitterGripWidth * 0.5f;
	layout.leftSplitterGrip = fuiRectMake(middleX - gripHalfWidth, 0.0f, DebugSplitterGripWidth, h);
	layout.rightSplitterGrip = fuiRectMake(rightSideX - gripHalfWidth, 0.0f, DebugSplitterGripWidth, h);

	return layout;
}

// Whether a caption still fits a button of the given width. Measured with the font in use rather than decided at a
// hardcoded column width, so a bigger font gives up on the full wording sooner, and it leaves the caption the same
// room fuiButton does when it draws, so what is decided here is what the drawing then honours.
static bool DoesButtonCaptionFit(fuiContext *ui, const char *caption, const float buttonWidth) {
	const fuiTheme *theme = fuiGetTheme(ui);
	const float widthAvailableForCaption = buttonWidth - theme->widgetPaddingX * 2.0f;
	const fuiVec2 captionSize = fuiMeasureText(ui, caption, 0, theme->fontHeight);
	return captionSize.x <= widthAvailableForCaption;
}

// Identifies the states dialog to the library, and is what fuiOpenDialog and fuiBeginModal agree on
static const char *StatesDialogId = "States-Dialog";

//
// Information icon
//
// The one way into everything a first time user cannot find out by looking: the key mapping, what the
// application can do, and what it was built out of. It is a bare picture rather than a button with a
// caption, and it floats over the bottom left corner of BOTH views so it never moves out from under the
// hand that reached for it.
//

// How far either corner icon sits from the two edges of the corner it floats over
static const float InfoIconMargin = 12.0f;

// How faint it is drawn while the cursor is somewhere else. Faint enough not to compete with whatever it
// covers, solid enough that somebody who has never seen the application still notices it is there
static const float InfoIconIdleOpacity = 0.3f;

// How far around the icon the cursor has to come for it to fade fully in, as a multiple of its own size
static const float InfoIconRevealFactor = 1.0f;

static fuiRect ComputeInfoIconRect(const float windowHeight) {
	fuiRect result = fuiRectMake(InfoIconMargin, windowHeight - INFO_ICON_SIZE - InfoIconMargin, INFO_ICON_SIZE, INFO_ICON_SIZE);
	return result;
}

// The application's own icon, mirroring the information icon across the window into the bottom right corner,
// which is the foot of the right panel. Decoration only: it answers no click and swallows none either.
static fuiRect ComputeAppIconRect(const float windowWidth, const float windowHeight) {
	fuiRect result = fuiRectMake(windowWidth - APP_ICON_SIZE - InfoIconMargin, windowHeight - APP_ICON_SIZE - InfoIconMargin, APP_ICON_SIZE, APP_ICON_SIZE);
	return result;
}

static void DrawAppIconOverlay(fuiContext *ui, const Texture *icon, const fuiRect rect) {
	if (icon == fpl_null || !icon->isValid) {
		return;
	}
	fuiImageDesc iconImage = fplZeroInit;
	iconImage.texture = (fuiTextureId)icon->id;
	iconImage.textureSize = fuiV2((float)icon->width, (float)icon->height);
	iconImage.uvMin = fuiV2(0.0f, 0.0f);
	iconImage.uvMax = fuiV2(icon->uScale, icon->vScale);
	iconImage.tint = fuiColorRGBA(1.0f, 1.0f, 1.0f, APP_ICON_OPACITY);
	iconImage.scaleMode = fuiImageScaleMode_Letterbox;
	// The image loader flips every texture on the way in, so it is turned back over here
	iconImage.flags = fuiImageFlags_FlipV;
	fuiImage(ui, rect, &iconImage);
}

static float ComputeInfoIconOpacity(fuiContext *ui, const fuiRect iconRect) {
	const fuiRect revealArea = fuiRectInflate(iconRect, iconRect.h * InfoIconRevealFactor);
	const fuiVec2 cursor = fuiGetMousePosition(ui);
	const bool cursorIsNear = (cursor.x >= revealArea.x) && (cursor.x < (revealArea.x + revealArea.w)) && (cursor.y >= revealArea.y) && (cursor.y < (revealArea.y + revealArea.h));
	float result = cursorIsNear ? 1.0f : InfoIconIdleOpacity;
	return result;
}

static void BuildStatesDialog(Application *app, const InputState *input) {
	fuiContext *ui = &app->ui;

	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;
	StatesDialog *statesDlg = &app->statesDialog;

	const uint32_t statesDialogNumSlots = MAX_STATE_SLOT_COUNT;
	const uint32_t statesDialogGridNumRows = 2;
	const uint32_t statesDialogGridNumColumns = statesDialogNumSlots / statesDialogGridNumRows;

	const float statesDialogGridColumnSpacing = 20.0f;
	const float statesDialogGridRowSpacing = 20.0f;
	const float statesDialogGridLabelMargin = 5.0f;

	const float windowWidth = (float)app->windowSize.w;
	const float windowHeight = (float)app->windowSize.h;
	const float statesDialogWidth = windowWidth * 0.75f;
	const float statesDialogHeight = windowHeight * 0.75f;

	const char *title = "";
	switch (statesDlg->type) {
		case DialogType_SaveState:
			title = "Save State";
			break;
		case DialogType_RestoreState:
			title = "Restore State";
			break;
		default:
			break;
	}

	if (!fuiBeginModal(ui, StatesDialogId, title, statesDialogWidth, statesDialogHeight)) {
		fuiEndModal(ui);
		return;
	}

	const fuiTheme *theme = fuiGetTheme(ui);
	const float lineHeight = theme->menuItemHeight;

	const fuiRect content = fuiLayoutRemaining(ui);

	const float closeButtonHeight = lineHeight * 1.5f;
	const float closeButtonWidth = 120.0f;

	const float gridHeight = content.h - closeButtonHeight - theme->widgetSpacing;

	const float statesGridCellWidth = (content.w - statesDialogGridColumnSpacing * (float)(statesDialogGridNumColumns - 1)) / (float)statesDialogGridNumColumns;
	const float statesGridCellHeight = (gridHeight - statesDialogGridRowSpacing * (float)(statesDialogGridNumRows - 1)) / (float)statesDialogGridNumRows;

	const fuiColor gameLabelBackground = fuiColorRGBA(0.0f, 0.0f, 0.0f, 0.5f);
	const fuiColor labelShadowColor = fuiColorRGBA(0.0f, 0.0f, 0.0f, 1.0f);
	const fuiColor labelColor = fuiColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);

	float gridY = content.y;
	for (uint32_t row = 0; row < statesDialogGridNumRows; ++row) {
		float gridX = content.x;
		for (uint32_t column = 0; column < statesDialogGridNumColumns; ++column) {
			uint32_t slotIndex = row * statesDialogGridNumColumns + column;

			bool isSlotSelected = (statesDlg->selectedSlotPos.row == row) && (statesDlg->selectedSlotPos.column == column);

			const Texture *texture = emulator->states.textures + slotIndex;

			if (isSlotSelected && statesDlg->type == DialogType_SaveState) {
				texture = &app->displayTexture;
			}

			const fgbSnapshot *snapshot = emulator->states.snapshots + slotIndex;

			char *labelBuffer = (char *)&emulator->states.labels[slotIndex];
			size_t labelBufferCapacity = fplArrayCount(emulator->states.labels[slotIndex]);

			fuiRect cellRect = fuiRectMake(gridX, gridY, statesGridCellWidth, statesGridCellHeight);

			UIPanel(ui, cellRect, isSlotSelected);

			float textureAlpha = isSlotSelected ? 1.0f : 0.5f;

			fuiImageDesc slotImage = fplZeroInit;
			slotImage.texture = (fuiTextureId)texture->id;
			slotImage.textureSize = fuiV2((float)texture->width, (float)texture->height);
			slotImage.uvMin = fuiV2(0.0f, 0.0f);
			slotImage.uvMax = fuiV2(texture->uScale, texture->vScale);
			slotImage.tint = fuiColorRGBA(1.0f, 1.0f, 1.0f, textureAlpha);
			slotImage.scaleMode = fuiImageScaleMode_Letterbox;
			fuiImage(ui, cellRect, &slotImage);

			if (isSlotSelected) {
				fuiDrawRectOutline(ui, cellRect, theme->accentColor, 2.0f);
			}

			// Date time + Game title
			const char *gameTitle = snapshot->gameInfo.title.text;
			if (fplGetStringLength(gameTitle) == 0) {
				if (isSlotSelected && statesDlg->type == DialogType_SaveState) {
					gameTitle = system->gamePak.info.title.text;
					fplStringFormat(labelBuffer, labelBufferCapacity, "%s *", gameTitle);
				} else {
					fplCopyString("None", labelBuffer, labelBufferCapacity);
				}
			} else {
				fplDateTime dt = fplZeroInit;
				dt.epoch = snapshot->dateTime.epoch;
				dt.milliseconds = snapshot->dateTime.milliseconds & 0xFFFF;
				fplDateTimeResult dtRes = fplFormatDateTime(dt, fplDateTimeType_Local);
				fplStringFormat(dateTimeFormatBuffer, fplArrayCount(dateTimeFormatBuffer), "%.4u-%.2u-%.2u %.2u:%.2u:%.2u", dtRes.year, dtRes.month, dtRes.day, dtRes.hour, dtRes.minute, dtRes.second);
				if (isSlotSelected && statesDlg->type == DialogType_SaveState) {
					fplStringFormat(labelBuffer, labelBufferCapacity, "%s - %s *", dateTimeFormatBuffer, gameTitle);
				} else {
					fplStringFormat(labelBuffer, labelBufferCapacity, "%s - %s", dateTimeFormatBuffer, gameTitle);
				}
			}

			float labelHeight = lineHeight + statesDialogGridLabelMargin + 4.0f;
			fuiDrawRect(ui, fuiRectMake(gridX + 2.0f, gridY + 2.0f, statesGridCellWidth - 4.0f, labelHeight), gameLabelBackground);

			const char *label = labelBuffer;
			size_t labelLen = fplGetStringLength(label);
			UIText(ui, gridX + statesDialogGridLabelMargin, gridY + statesDialogGridLabelMargin, labelShadowColor, label, labelLen);
			UIText(ui, gridX + statesDialogGridLabelMargin + 2, gridY + statesDialogGridLabelMargin + 2, labelColor, label, labelLen);

			gridX += statesGridCellWidth + statesDialogGridColumnSpacing;
		}
		gridY += statesGridCellHeight + statesDialogGridRowSpacing;
	}

	bool shouldClose = false;

	// Handle keyboard/controller input
	if (input->activeControllerIndex >= 0) {
		// "Start" was pressed?
		const ControllerInput *controller = &input->controllers[input->activeControllerIndex];
		if (UIWasPressed(&controller->start)) {
			const uint32_t slotIndex = statesDlg->selectedSlotPos.row * statesDialogGridNumColumns + statesDlg->selectedSlotPos.column;

			FGB_ASSERT(slotIndex < fplArrayCount(emulator->states.snapshots));
			FGB_ASSERT(slotIndex < fplArrayCount(emulator->states.textures));

			fgbSnapshot *snapshot = emulator->states.snapshots + slotIndex;

			Texture *stateTexture = emulator->states.textures + slotIndex;

			switch (statesDlg->type) {
				case DialogType_SaveState:
					if (fgbSnapshotExport(system, snapshot)) {
						if (fgbSnapshotSaveToFile(system, system->gamePak.filePath, slotIndex, snapshot)) {
							stateTexture->state = TextureState_Update;
						}
					}
					break;
				case DialogType_RestoreState:
					if (fgbSnapshotLoadFromFile(system, system->gamePak.filePath, slotIndex, snapshot)) {
						if (fgbSnapshotImport(system, snapshot)) {
							stateTexture->state = TextureState_Update;
						}
					}
					break;
				default:
					break;
			}

			shouldClose = true;
		} else if (UIWasPressed(&controller->dpadLeft)) {
			if (statesDlg->selectedSlotPos.column > 0) {
				--statesDlg->selectedSlotPos.column;
			}
		} else if (UIWasPressed(&controller->dpadRight)) {
			if (statesDlg->selectedSlotPos.column < statesDialogGridNumColumns - 1) {
				statesDlg->selectedSlotPos.column++;
			}
		} else if (UIWasPressed(&controller->dpadUp)) {
			if (statesDlg->selectedSlotPos.row > 0) {
				--statesDlg->selectedSlotPos.row;
			}
		} else if (UIWasPressed(&controller->dpadDown)) {
			if (statesDlg->selectedSlotPos.row < statesDialogGridNumRows - 1) {
				statesDlg->selectedSlotPos.row++;
			}
		}
	}

	// Close button
	float closeButtonX = content.x + (content.w - closeButtonWidth) * 0.5f;
	float closeButtonY = content.y + content.h - closeButtonHeight;
	if (fuiButton(ui, fuiRectMake(closeButtonX, closeButtonY, closeButtonWidth, closeButtonHeight), "Close")) {
		shouldClose = true;
	}

	// Escape is taken through the library, so a dialog stacked on another one does not close both on one press
	if (fuiDialogTakeKey(ui, fuiKey_Escape)) {
		shouldClose = true;
	}

	fuiEndModal(ui);

	if (shouldClose) {
		statesDlg->type = DialogType_None;
		statesDlg->isShown = false;
		fuiCloseDialog(ui, StatesDialogId);
		ResumeGameboy(app, system);
	}
}

static void RenderDebugFrame(Application *app, const InputState *input) {
	fuiContext *ui = &app->ui;

	fuiTheme *theme = fuiGetTheme(ui);

	const float lineHeight = theme->menuItemHeight;

	static char tmpText[256];

	const float w = (float)app->viewport.w;
	const float h = (float)app->viewport.h;

	const float vramAspect = FGB_TILEMAP_WIDTH / (float)FGB_TILEMAP_HEIGHT;
	const float boyAspect = FGB_DISPLAY_WIDTH / (float)FGB_DISPLAY_HEIGHT;

	ClampDebugColumnWidths(app, w);

	const DebugLayout layout = ComputeDebugLayout(app, w, h, lineHeight);

	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;
	fgbGamePak *gamePak = &system->gamePak;

	RendererSetViewport(app->viewport.x, app->viewport.y, app->viewport.w, app->viewport.h);

	RendererClear(0.1f, 0.3f, 0.7f, 1.0f);

	RendererSetModelViewProjectionMatrix(&app->viewProjectionMat.m[0]);

	// The shader filtered display goes down FIRST, so everything the interface draws lands on top of it
	RenderDisplayTexture(app, layout.display, boyAspect);

	const fuiColor foregroundColor = theme->textColor;

	fuiBeginFrame(ui, &app->uiInput, fuiPass_Both);

	//
	// GamePak info
	//
	UIPanel(ui, layout.cartInfo, false);
	UIWatermark(ui, &app->uiFont, layout.cartInfo, "Game Pak");
	fuiPushClip(ui, layout.cartInfo);

	const char *gamePakTitle = gamePak->isValid ? gamePak->info.title.text : "[Unloaded]";
	const char *gamePakTypeName = fgbGetGamePakTypeName(gamePak->info.gamePakType);
	const char *coreName = fgbGetCoreTypeName(gamePak->info.coreType);

	float tmpX = layout.cartInfo.x + DebugPanelPadding;
	float tmpY = layout.cartInfo.y + DebugPanelPadding;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "GamePak: %s", gamePakTitle);
	UIText(ui, tmpX, tmpY, foregroundColor, tmpText, 0);
	tmpY += lineHeight;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "Core: %s Type: %s", coreName, gamePakTypeName);
	UIText(ui, tmpX, tmpY, foregroundColor, tmpText, 0);
	tmpY += lineHeight;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "ROM/RAM Banks: %u/%u [%zu bytes]", gamePak->info.romBankCount, gamePak->info.ramBankCount, gamePak->rom.length);
	UIText(ui, tmpX, tmpY, foregroundColor, tmpText, 0);

	fuiPopClip(ui);

	//
	// CPU
	//
	DrawCPUState(app, system, layout.cpuState, DebugPanelPadding);

	//
	// Display Registers
	//
	DrawDisplayState(app, &system->ppu, layout.displayState, DebugPanelPadding);

	//
	// Sound Registers
	//
	DrawSoundState(app, system, layout.soundState, DebugPanelPadding);

	//
	// Shader Control
	//
	if (layout.isShaderControlVisible) {
		UIPanel(ui, layout.shaderControl, false);

		const float shaderRowHeight = lineHeight;
		const float shaderRowY = layout.shaderControl.y + (layout.shaderControl.h - shaderRowHeight) * 0.5f;
		float shaderX = UICheckboxRowX(ui, layout.shaderControl.x + DebugPanelPadding);

		int32_t shaderChoice = (int32_t)app->activeShaderType;

		const struct {
			const char *label;
			AppShaderType type;
			bool isSupported;
		} shaderChoices[] = {
			{ "Nearest", AppShaderType_None, app->nearestShader.isValid },
			{ "Bilinear", AppShaderType_Bilinear, app->bilinearShader.isValid },
			{ "HQ2x", AppShaderType_HQ2X, app->hq2xShader.isValid },
			{ "HQ4x", AppShaderType_HQ4X, app->hq4xShader.isValid },
			{ "Bicubic-H", AppShaderType_BicubicHermite, app->bicubicHermiteShader.isValid },
			{ "Bicubic-L", AppShaderType_BicubicLagrange, app->bicubicLagrangeShader.isValid },
			{ "CatMullRom", AppShaderType_CatmullRom4, app->catmullRom4Shader.isValid },
		};

		for (uint32_t choiceIndex = 0; choiceIndex < fplArrayCount(shaderChoices); ++choiceIndex) {
			const char *label = shaderChoices[choiceIndex].label;
			float choiceWidth = UICheckboxWidth(ui, label);
			if (UIRadioEx(ui, fuiRectMake(shaderX, shaderRowY, choiceWidth, shaderRowHeight), label, &shaderChoice, (int32_t)shaderChoices[choiceIndex].type, shaderChoices[choiceIndex].isSupported)) {
				app->activeShaderType = shaderChoices[choiceIndex].type;
			}
			shaderX += choiceWidth;
		}
	}

	//
	// Actions
	//
	UIPanel(ui, layout.actions, false);

	const char *pauseOrResumeButtonName = system->state == fgbEmulationState_Running ? "Pause" : "Resume";

	// The three step buttons all get the same width and sit side by side, so they are decided together against the
	// longest of their captions. A row where only the middle one has shortened reads as a mistake rather than as a fit.
	const bool stepCaptionsFitInFull = DoesButtonCaptionFit(ui, "Single Step", layout.actionButtonWidth);
	const char *frameStepCaption = stepCaptionsFitInFull ? "Frame Step" : "Step/F";
	const char *singleStepCaption = stepCaptionsFitInFull ? "Single Step" : "Step/S";
	const char *microStepCaption = stepCaptionsFitInFull ? "Micro Step" : "Step/M";

	tmpX = layout.actions.x + layout.actionsPadding;
	const float actionButtonY = layout.actions.y + layout.actionsPadding;

	bool pauseOrResumeEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (fuiButtonEx(ui, fuiRectMake(tmpX, actionButtonY, layout.actionButtonWidth, layout.actionButtonHeight), pauseOrResumeButtonName, pauseOrResumeEnabled)) {
		if (system->state == fgbEmulationState_Running) {
			fgbPause(system);
		} else {
			fgbResume(system);
		}
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = false;
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += layout.actionButtonWidth + layout.actionsSpacing;

	bool frameStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error && !emulator->isFrameStepActive;
	if (fuiButtonEx(ui, fuiRectMake(tmpX, actionButtonY, layout.actionButtonWidth, layout.actionButtonHeight), frameStepCaption, frameStepEnabled)) {
		emulator->isFrameStepActive = true;
		emulator->isMicroStepActive = false;
		fgbResume(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += layout.actionButtonWidth + layout.actionsSpacing;

	bool singleStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (fuiButtonEx(ui, fuiRectMake(tmpX, actionButtonY, layout.actionButtonWidth, layout.actionButtonHeight), singleStepCaption, singleStepEnabled)) {
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = false;
		fgbStep(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += layout.actionButtonWidth + layout.actionsSpacing;

	bool microStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (fuiButtonEx(ui, fuiRectMake(tmpX, actionButtonY, layout.actionButtonWidth, layout.actionButtonHeight), microStepCaption, microStepEnabled)) {
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = true;
		fgbMicroStep(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += layout.actionButtonWidth + layout.actionsSpacing;

	bool resetEnabled = emulator->isActive;
	if (fuiButtonEx(ui, fuiRectMake(tmpX, actionButtonY, layout.actionButtonWidth, layout.actionButtonHeight), "Reset", resetEnabled)) {
		StringListClear(&app->console);
		app->consoleList.selectedIndex = -1;
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = false;
		fgbReset(system, app->emulator.config.paused);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}

	//
	// Switches
	//
	UIPanel(ui, layout.switches, false);

	const float switchRowHeight = lineHeight;
	const float switchRowY = layout.switches.y + (layout.switches.h - switchRowHeight) * 0.5f;

	tmpX = UICheckboxRowX(ui, layout.switches.x + DebugPanelPadding);

	bool isLoggingChecked = system->log.isEnabled;
	float logWidth = UICheckboxWidth(ui, "Log");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchRowY, logWidth, switchRowHeight), "Log", &isLoggingChecked, true)) {
		emulator->config.log.isEnabled = isLoggingChecked;
		if (emulator->isActive) {
			system->log.isEnabled = emulator->config.log.isEnabled;
		}
	}
	tmpX += logWidth;

	bool isTraceChecked = system->debug.isInstructionTraceEnabled;
	float traceWidth = UICheckboxWidth(ui, "Trace");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchRowY, traceWidth, switchRowHeight), "Trace", &isTraceChecked, true)) {
		emulator->config.debug.isInstructionTraceEnabled = isTraceChecked;
		if (emulator->isActive) {
			system->debug.isInstructionTraceEnabled = emulator->config.debug.isInstructionTraceEnabled;
		}
	}
	tmpX += traceWidth;

	bool isBootChecked = system->boot.rom.isEnabled;
	float bootWidth = UICheckboxWidth(ui, "Boot");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchRowY, bootWidth, switchRowHeight), "Boot", &isBootChecked, true)) {
		emulator->config.bootROM.isEnabled = isBootChecked;
		if (emulator->isActive) {
			system->boot.rom.isEnabled = emulator->config.bootROM.isEnabled;
		}
	}
	tmpX += bootWidth;

	bool isInitPauseChecked = app->emulator.config.paused;
	float initPauseWidth = UICheckboxWidth(ui, "IR-Pause");
	if (UICheckboxEx(ui, fuiRectMake(tmpX, switchRowY, initPauseWidth, switchRowHeight), "IR-Pause", &isInitPauseChecked, true)) {
		app->emulator.config.paused = isInitPauseChecked;
	}

	//
	// Display frame
	//
	DrawDisplayFrame(app, layout.display);

	//
	// Right Tab Control
	//
	static const char *rightTabs[] = { "Tiles", "Palettes", "Breakpoints" };

	// The tab control draws its headers and nothing behind them, so the strip they sit on is filled first
	fuiDrawRect(ui, layout.rightTabHeader, theme->panelBackgroundColor);
	UIPanel(ui, layout.rightTabContent, false);
	// Clipped to the strip, so headers wider than a narrowed column are cut at its edge and cannot be clicked there either
	fuiPushClip(ui, layout.rightTabHeader);
	int32_t rightTabIndex = fuiTabControl(ui, layout.rightTabHeader, "Right-TabControl", rightTabs, (int32_t)fplArrayCount(rightTabs));
	fuiPopClip(ui);
	if (rightTabIndex == 0) {
		DrawTiles(ui, &app->tileMapTexture, layout.rightTabContent, vramAspect);
	} else if (rightTabIndex == 1) {
		DrawPalettes(app, layout.rightTabContent);
	} else if (rightTabIndex == 2) {
		DrawBreakpoints(app, layout.rightTabContent);
	}

	//
	// Main Tab Control
	//
	static const char *leftTabs[] = { "Log", "Performance", "Disassembly", "BG-Map" };

	// Disassembly and the background map need a running emulator, so without one the list stops at two tabs
	int32_t leftTabCount = (int32_t)fplArrayCount(leftTabs);
	if (!app->emulator.isActive) {
		leftTabCount = 2;
	}

	fuiDrawRect(ui, layout.leftTabHeader, theme->panelBackgroundColor);
	UIPanel(ui, layout.leftTabContent, false);
	fuiPushClip(ui, layout.leftTabHeader);
	int32_t leftTabIndex = fuiTabControl(ui, layout.leftTabHeader, "Left-TabControl", leftTabs, leftTabCount);
	fuiPopClip(ui);
	if (leftTabIndex == 0) {
		// The log follows its own tail, the way it did when every new line scrolled the list down
		int32_t consoleScrollTo = -1;
		if (app->consoleScrollPending) {
			consoleScrollTo = (int32_t)app->console.count - 1;
			app->consoleScrollPending = false;
		}
		UISourceList(ui, layout.leftTabContent, "Console", &app->console, &app->consoleList, -1, consoleScrollTo);
	} else if (leftTabIndex == 1) {
		DrawPerformanceMetrics(ui, app, layout.leftTabContent);
	} else if (leftTabIndex == 2) {
		int32_t disassemblyScrollTo = app->disassemblyScrollRequested ? app->disassemblyHighlightIndex : -1;
		app->disassemblyScrollRequested = false;
		UISourceList(ui, layout.leftTabContent, "Disassembly", &app->disassembly, &app->disassemblyList, app->disassemblyHighlightIndex, disassemblyScrollTo);
	} else if (leftTabIndex == 3) {
		DrawBackgroundMap(ui, app, layout.leftTabContent);
	}

	//
	// User Buttons
	//
	UIPanel(ui, layout.userButtons, false);

	StatesDialog *statesDlg = &app->statesDialog;

	tmpX = layout.userButtons.x + layout.userButtonsPadding;
	const float userButtonY = layout.userButtons.y + layout.userButtonsPadding;

	bool saveStateButtonEnabled = emulator->isActive && fgbAreSnapshotsSupported(system);
	if (fuiButtonEx(ui, fuiRectMake(tmpX, userButtonY, layout.userButtonWidth, layout.userButtonHeight), "Save State", saveStateButtonEnabled)) {
		fgbPause(system);
		statesDlg->type = DialogType_SaveState;
		statesDlg->isShown = true;
		fuiOpenDialog(ui, StatesDialogId);
	}
	tmpX += layout.userButtonWidth + layout.userButtonSpacing;

	bool restoreStateButtonEnabled = emulator->isActive && fgbAreSnapshotsSupported(system);
	if (fuiButtonEx(ui, fuiRectMake(tmpX, userButtonY, layout.userButtonWidth, layout.userButtonHeight), "Restore State", restoreStateButtonEnabled)) {
		fgbPause(system);
		statesDlg->type = DialogType_RestoreState;
		statesDlg->isShown = true;
		fuiOpenDialog(ui, StatesDialogId);
	}

	//
	// Column splitters
	//
	// Built last so they take the cursor from whatever panel lies under the seam. They resize for the NEXT
	// frame, which is the ordinary immediate mode bargain and is what lets the layout be settled up front.
	const float widestLeftMayBe = (w - app->rightPanelWidth) - DebugMiddleColumnMinWidth;
	if (widestLeftMayBe > DebugSideColumnMinWidth) {
		fuiVerticalSplitter(ui, "Left-Splitter", layout.leftSplitterGrip, &app->leftPanelWidth, DebugSideColumnMinWidth, widestLeftMayBe, DebugSplitterHighlightThickness);
	}

	// Driven by where its LEFT edge sits rather than by its width, because the splitter grows what it is
	// given as the cursor moves right and this column has to shrink instead
	const float leftmostRightEdgeMayBe = app->leftPanelWidth + DebugMiddleColumnMinWidth;
	const float rightmostRightEdgeMayBe = w - DebugSideColumnMinWidth;
	if (rightmostRightEdgeMayBe > leftmostRightEdgeMayBe) {
		float rightColumnEdge = w - app->rightPanelWidth;
		if (fuiVerticalSplitter(ui, "Right-Splitter", layout.rightSplitterGrip, &rightColumnEdge, leftmostRightEdgeMayBe, rightmostRightEdgeMayBe, DebugSplitterHighlightThickness)) {
			app->rightPanelWidth = w - rightColumnEdge;
		}
	}

	//
	// Corner icons
	//
	// The information button on the left and the application's own icon on the right, both built after every
	// panel so they lie over the one they share a corner with rather than under it.
	//
	DrawAppIconOverlay(ui, &app->appIconSmallTexture, ComputeAppIconRect(w, h));

	const fuiRect infoIconRect = ComputeInfoIconRect(h);
	const float infoIconOpacity = ComputeInfoIconOpacity(ui, infoIconRect);
	if (UIIconButton(ui, infoIconRect, "Debug-Info-Icon", &app->aboutIconTexture, infoIconOpacity)) {
		AboutDialogOpen(ui, &app->aboutDialog, AboutPage_About);
	}
	fuiTooltip(ui, infoIconRect, "Key mapping, features, credits and licenses");

	//
	// Dialogs
	//
	BuildStatesDialog(app, input);
	AboutDialogBuild(ui, &app->aboutDialog, &app->aboutIconSmallTexture, &app->appIconTexture, w, h);

	fuiEndFrame(ui);

	const fuiDrawData *uiDrawData = fuiGetDrawData(ui);
	RendererDrawUIDrawData(uiDrawData);
}

static void RenderGameFrame(Application *app, const InputState *input) {
	RendererSetViewport(app->viewport.x, app->viewport.y, app->viewport.w, app->viewport.h);

	RendererClear(0.1f, 0.3f, 0.7f, 1.0f);

	RendererSetModelViewProjectionMatrix(&app->viewProjectionMat.m[0]);

	const float w = (float)app->viewport.w;
	const float h = (float)app->viewport.h;

	const float displayAspect = FGB_DISPLAY_WIDTH / (float)FGB_DISPLAY_HEIGHT;

	const fuiRect displayArea = fuiRectMake(0.0f, 0.0f, w, h);

	RendererDrawFilledQuad(0, 0, w, h, ColorBlack);

	RenderDisplayTexture(app, displayArea, displayAspect);

	// The placeholder, the information button and its dialog are built here, and they go through the same
	// path the debugger does so the display is covered by exactly one convention
	fuiContext *ui = &app->ui;
	fuiBeginFrame(ui, &app->uiInput, fuiPass_Both);

	DrawDisplayFrame(app, displayArea);

	//
	// Information icon
	//
	// The same corner and the same icon the debugger carries, except here it lies over the game. Nothing is
	// being covered while no game is loaded, and that is exactly the moment somebody needs to be told how to
	// load one, so it stands at full strength until there is something to stay out of the way of.
	//
	const fuiRect infoIconRect = ComputeInfoIconRect(h);
	const float infoIconOpacity = app->emulator.isActive ? ComputeInfoIconOpacity(ui, infoIconRect) : 1.0f;

	if (UIIconButton(ui, infoIconRect, "Player-Info-Icon", &app->aboutIconTexture, infoIconOpacity)) {
		AboutDialogOpen(ui, &app->aboutDialog, AboutPage_HowToUse);
	}
	fuiTooltip(ui, infoIconRect, "Key mapping, features, credits and licenses");

	AboutDialogBuild(ui, &app->aboutDialog, &app->aboutIconSmallTexture, &app->appIconTexture, w, h);

	fuiEndFrame(ui);

	const fuiDrawData *uiDrawData = fuiGetDrawData(ui);
	RendererDrawUIDrawData(uiDrawData);
}

static void RenderFrame(Application *app, const InputState *input) {
	if (app->isDebugEnabled) {
		RenderDebugFrame(app, input);
	} else {
		RenderGameFrame(app, input);
	}
}

static void GameboxLog(void *userData, const fgbLogLevel level, const char *system, const char *message) {
	char logText[255] = { 0 };
	if (fplGetStringLength(message) > 0) {
		fplStringFormat(logText, fplArrayCount(logText), "[%s] %s", system, message);
	} else {
		logText[0] = '\0';
	}

	fplDebugFormatOut("%s\n", message);

	if (level < fgbLogLevel_Trace) {
		Application *app = (Application *)userData;
		StringListAdd(&app->console, logText);
		app->consoleScrollPending = true;
	}
}

typedef enum {
	ExitCode_Success = 0,
	ExitCode_InvalidArguments,
	ExitCode_MissingGamePakArgument,
	ExitCode_OutOfMemory,
	ExitCode_FailedInitializePlatform,
	ExitCode_FailedInitializeRenderer,
	ExitCode_FailedLoadingShaders,
	ExitCode_FailedLoadingGamePak,
	ExitCode_FailedInitializeGamebox,
	ExitCode_FailedStartingThread,
} ExitCode;

static void *CustomMemoryDynamicAllocate(void *userData, const size_t size, const size_t alignment) {
	fmemMemoryBlock *memory = (fmemMemoryBlock *)userData;
	return (void *)fmemPushAligned(memory, size, alignment, fmemPushFlags_Clear);
}

static void CustomMemoryDynamicRelease(void *userData, void *ptr) {
	// TODO(final): fmemFreeBlock(), that requires matching the ptr to the base-pointer of a block, that is in locked mode (push permission denied)
}

static void TestQuirkTileAddresses() {
	uint16_t startAddress = 0x8000;
	uint16_t address = startAddress;
	while (address < 0x97FF) {
		uint16_t normalizedAddress = address & 0xFFFE;

		uint16_t normalizedIndex = normalizedAddress - 0x8000;

		uint32_t tileIndex = normalizedIndex / FGB_TILE_SIZE;

		uint32_t tileRow = tileIndex / FGB_TILEMAP_HORIZONTAL_COUNT;
		uint32_t tileColumn = tileIndex % FGB_TILEMAP_HORIZONTAL_COUNT;

		uint32_t lineY = (normalizedIndex/2) % FGB_TILE_HEIGHT;

		uint32_t pixelX = tileColumn * FGB_TILE_WIDTH;
		uint32_t pixelY = tileRow * FGB_TILE_HEIGHT + lineY;

		address++;
	}
}

static void TestQuirkFMEM() {
	fmemMemoryBlock mainBlock = fmemCreate(fmemType_Growable, 0, fplMegaBytes(128));

	fmemPush(&mainBlock, 16, fmemPushFlags_Clear);

	for (uint32_t i = 0; i < 10000000; ++i) {
		fmemPush(&mainBlock, fplKiloBytes(2), fmemPushFlags_Clear);
	}
}

static void FillWithRandomPixels(fgbColor *colors, const uint32_t width, const uint32_t height) {
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			uint8_t r = rand() % UINT8_MAX;
			uint8_t g = rand() % UINT8_MAX;
			uint8_t b = rand() % UINT8_MAX;
			fgbColor color = { .r = r, .g = g, .b = b };
			colors[y * width + x] = color;
		}
	}
}

static void GameboxMicroStepHandler(struct fgbSystem *gb, void *userData, const fgbMicroStepType type, const fgbTickCycles cycles) {
	Emulator *emulator = (Emulator *)userData;
	if (!emulator->isShutdown && emulator->isActive && emulator->isMicroStepActive) {
		fplConditionWait(&emulator->microStepCondition, &emulator->mutex, 1000 * 60);
	}
}

static void GameboxBreakpointHandler(struct fgbSystem *gb, void *userData, const fgbBreakpointType type) {
	Emulator *emulator = (Emulator *)userData;
	fgbSystem *system = (fgbSystem *)gb;
	if (!emulator->isShutdown && emulator->isActive && system->state == fgbEmulationState_Breakpoint) {
		emulator->lastBreakpointType = type;
		fplConditionWait(&emulator->breakpointCondition, &emulator->mutex, 1000 * 60);
	}
}

static void EmulatorThreadProc(const fplThreadHandle *thread, void *data) {
	Emulator *emulator = (Emulator *)data;

	PerformanceMetrics *metrics = &emulator->performanceMetrics;

	fgbSystem *system = &emulator->system;

	emulator->isActive = false;
	emulator->isShutdown = false;

	fplTimestamp epoch = fplTimestampQuery();
	uint64_t framesDone = 0;

	static FrameSnapshotDisplay prodDisplay;
	static FrameSnapshotBackgroundMap prodBgMap;
	static FrameSnapshotTilemap prodTilemap;

	while (!emulator->isShutdown) {
		const fgbEmulationState state = fgbGetState(system);

		// Park when not running. Push a snapshot first so the debugger view stays live.
		if (!emulator->isActive || state == fgbEmulationState_Paused || state == fgbEmulationState_Error) {
			if (emulator->isActive) {
				fplMemoryCopy(system->ppu.display, sizeof(prodDisplay.pixels), prodDisplay.pixels);
				fplMemoryCopy(system->ppu.backgroundMap.colors, sizeof(prodBgMap.pixels), prodBgMap.pixels);
				fplMemoryCopy(system->ppu.tilemap, sizeof(prodTilemap.pixels), prodTilemap.pixels);
				DisplayFrameQueueTryPush(&emulator->displayQueue, &prodDisplay);
				BackgroundMapFrameQueueTryPush(&emulator->backgroundMapQueue, &prodBgMap);
				TilemapFrameQueueTryPush(&emulator->tilemapQueue, &prodTilemap);
			}
			fplConditionWait(&emulator->waitCondition, &emulator->mutex, 1000 * 60);
			// Resync pacing on wakeup so we don't try to "catch up" on sleep time.
			epoch = fplTimestampQuery();
			framesDone = 0;
			continue;
		}

		// Scripted input: apply all events scheduled up to the frame that is about to run
		fgbInputSimApply(&emulator->inputSim, system, emulator->inputSimFrameIndex);

		// Run one full PPU frame (or until a halt point is hit).
		BeginPerformanceCounter(&metrics->emulatorTick, fplTimestampQuery());
		bool frameDone = false;
		bool vramTouched = false;
		bool haltRequested = false;
		uint32_t safety = 0;
		while (safety++ < EMULATOR_INNER_SAFETY_CAP) {
			if (!fgbTick(system)) {
				break;
			}
			if (fgbIsVRAMUpdated(system)) {
				vramTouched = true;
			}
			if (fgbIsFrameUpdated(system)) {
				frameDone = true;
				break;
			}
			// MicroStep / Breakpoint handlers condwait inside fgbTick; when they return
			// the state may have changed and fgbIsFrameUpdated will not become true for
			// the rest of this frame. Exit so the outer loop can pause / park.
			if (emulator->isMicroStepActive) {
				haltRequested = true;
				break;
			}
			const fgbEmulationState innerState = fgbGetState(system);
			if (innerState == fgbEmulationState_Paused || innerState == fgbEmulationState_Error) {
				haltRequested = true;
				break;
			}
		}
		EndPerformanceCounter(&metrics->emulatorTick, fplTimestampQuery());

		// Snapshot display + bgmap on a full frame, and whenever an inner halt ran (so
		// debugger step / breakpoint paths always land a visible frame).
		if (frameDone || haltRequested) {
			fplMemoryCopy(system->ppu.display, sizeof(prodDisplay.pixels), prodDisplay.pixels);
			fplMemoryCopy(system->ppu.backgroundMap.colors, sizeof(prodBgMap.pixels), prodBgMap.pixels);
			DisplayFrameQueueTryPush(&emulator->displayQueue, &prodDisplay);
			BackgroundMapFrameQueueTryPush(&emulator->backgroundMapQueue, &prodBgMap);
		}
		if (vramTouched || haltRequested) {
			fplMemoryCopy(system->ppu.tilemap, sizeof(prodTilemap.pixels), prodTilemap.pixels);
			TilemapFrameQueueTryPush(&emulator->tilemapQueue, &prodTilemap);
		}

		// Frame-step halt: pause after the full frame completed.
		if (frameDone && emulator->isFrameStepActive) {
			emulator->isFrameStepActive = false;
			fgbPause(system);
			continue;
		}
		// Micro-step halt: pause after the micro-step handler unblocked.
		if (emulator->isMicroStepActive) {
			emulator->isMicroStepActive = false;
			fgbPause(system);
			continue;
		}
		// Other mid-frame halts (breakpoint handler, error): let the next iteration park.
		if (haltRequested) {
			continue;
		}

		// Pacing: epoch-anchored absolute target. Each completed frame advances the
		// schedule by exactly EMULATOR_FRAME_TIME_NS so the CPU clock averages exactly
		// 4.194304 MHz over the long run, regardless of per-iteration jitter.
		++framesDone;
		++emulator->inputSimFrameIndex;

		// Audio-rescue hysteresis: once ring drops below LOW_WATER, stay in
		// rescue mode (run flat-out, no wallclock wait) until ring refills
		// to PRIME (HIGH_WATER). Prevents the ring from stabilizing at
		// LOW_WATER and leaving no headroom for the next scheduler hiccup.
		const uint32_t ringFill = fgbGetAudioRingBufferFillFrames(system);
		if (!emulator->audioRescueActive) {
			if (ringFill < FGB_APU_RING_BUFFER_LOW_WATER_FRAMES) {
				emulator->audioRescueActive = 1;
			}
		} else {
			if (ringFill >= FGB_APU_RING_BUFFER_PRIME_FRAMES) {
				emulator->audioRescueActive = 0;
			}
		}

		if (emulator->audioRescueActive) {
			// Skip pacing entirely; re-anchor so accumulated wallclock debt
			// doesn't cause an endless catch-up burst after rescue ends.
			epoch = fplTimestampQuery();
			framesDone = 0;
			continue;
		}

		const uint64_t targetNs = framesDone * EMULATOR_FRAME_TIME_NS;
		for (;;) {
			const int64_t nowNs = (int64_t)(fplTimestampElapsed(epoch, fplTimestampQuery()) * 1e9);
			const int64_t remainingNs = (int64_t)targetNs - nowNs;
			if (remainingNs <= 0) {
				break;
			}
			// Enter rescue mid-sleep if ring drops below LOW_WATER.
			if (fgbGetAudioRingBufferFillFrames(system) < FGB_APU_RING_BUFFER_LOW_WATER_FRAMES) {
				emulator->audioRescueActive = 1;
				epoch = fplTimestampQuery();
				framesDone = 0;
				break;
			}
			if (remainingNs > EMULATOR_SPIN_THRESHOLD_NS) {
				const uint32_t coarseMs = (uint32_t)((remainingNs - EMULATOR_SPIN_THRESHOLD_NS) / 1000000);
				if (coarseMs > 0) {
					fplThreadSleep(coarseMs);
				} else {
					fplThreadYield();
				}
			} else {
				fplThreadYield();
			}
		}
	}
}

static void UpdateActiveController(InputState *input, const int newIndex) {
	if (newIndex != -1) {
		input->activeControllerIndex = newIndex;
		if (newIndex == 0) {
			// Enable keyboard controller, if a keyboard button was pressed
			input->controllers[0].state = ControllerState_Connected;
		} else {
			// Disable keyboard controller, if a gamepad was connected
			input->controllers[0].state = ControllerState_Disconnected;
		}
	} else {
		input->activeControllerIndex = -1;	
		for (uint32_t i = fplArrayCount(input->controllers) - 1; i > 0; i--) {
			if (input->controllers[i].state == ControllerState_Connected) {
				input->activeControllerIndex = (int32_t)i;
				break;
			}
		}
	}
}

static void ProcessControllerButton(const InputState *oldInput, InputState *newInput, const uint32_t buttonIndex, const uint32_t controllerIndex, const bool isDown) {
	const ControllerInput *oldController = oldInput->controllers + controllerIndex;
	ControllerInput *newController = newInput->controllers + controllerIndex;

	const UIButtonState *oldButton = oldController->buttons + buttonIndex;
	UIButtonState *newButton = newController->buttons + buttonIndex;
	
	const bool isKeyboard = controllerIndex == 0;
	
	if (isKeyboard) {
		UpdateActiveController(newInput, (int32_t)controllerIndex);
	}

	if (isKeyboard)
		UpdateKeyboardButtonState(newButton, isDown);
	else
		UpdateDigitalButtonState(oldButton, newButton, isDown);
}

static void ProcessEvents(Application *app, const InputState *oldInput, InputState *newInput) {
	Emulator *emulator = &app->emulator;

	fplEvent ev;
	while (fplPollEvent(&ev)) {
		switch (ev.type) {
			case fplEventType_Keyboard:
			{
				if (ev.keyboard.type == fplKeyboardEventType_Input) {
					UIInputAddText(&app->uiInput, ev.keyboard.keyCode);
				}

				if (ev.keyboard.type == fplKeyboardEventType_Button) {
					bool isDown = ev.keyboard.buttonState >= fplButtonState_Press;

					// The very same press also reaches the interface, which is what makes its own keyboard navigation work
					fuiKey uiKey = UIKeyFromPlatformKey(ev.keyboard.mappedKey);
					if (uiKey != fuiKey_None) {
						UIInputSetButton(&app->uiInput.keys[uiKey], isDown);
					}

					switch (ev.keyboard.mappedKey) {
						case fplKey_Up:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_DPAD_UP, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_Down:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_DPAD_DOWN, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_Left:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_DPAD_LEFT, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_Right:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_DPAD_RIGHT, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_Return:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_START, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_Backspace:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_SELECT, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_A:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_A, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;
						case fplKey_S:
							ProcessControllerButton(oldInput, newInput, CONTROLLER_BUTTON_B, KEYBOARD_CONTROLLER_INDEX, isDown);
							break;

						// Debug keyboard mapping
						case fplKey_Space:
							if (newInput->debug.isEnabled)
								UpdateKeyboardButtonState(&newInput->debug.singleStep, isDown);
							break;
						case fplKey_F:
							if (newInput->debug.isEnabled)
								UpdateKeyboardButtonState(&newInput->debug.frameStep, isDown);
							break;
						case fplKey_F1:
							UpdateKeyboardButtonState(&newInput->debug.toggleDebug, isDown);
							break;

						default:
							break;
					}
				}
			}
			break;

			case fplEventType_Gamepad:
			{
				uint32_t controllerIndex = 1 + ev.gamepad.deviceIndex;
				fplAssert(controllerIndex < fplArrayCount(newInput->controllers));
				ControllerInput *newController = &newInput->controllers[controllerIndex];
				const ControllerInput *oldController = &oldInput->controllers[controllerIndex];
				switch (ev.gamepad.type) {
					case fplGamepadEventType_Connected:
					{
						// Only connect a controller when at least one mapped button is pressed
							newController->state = ControllerState_Connected;
						if (ev.gamepad.state.isActive) {
							UpdateActiveController(newInput, (int32_t)controllerIndex);
						}
					} break;

					case fplGamepadEventType_Disconnected:
					{
						newController->state = ControllerState_Disconnected;
						UpdateActiveController(newInput, -1);
					} break;

					case fplGamepadEventType_StateChanged:
					{
						fplGamepadState *padstate = &ev.gamepad.state;
						if (newController->state == ControllerState_Connected) {
							bool changed = false;

							changed |= UpdateDigitalButtonState(&oldController->actionA, &newController->actionA, padstate->actionA.isDown);
							changed |= UpdateDigitalButtonState(&oldController->actionB, &newController->actionB, padstate->actionB.isDown);
							changed |= UpdateDigitalButtonState(&oldController->select, &newController->select, padstate->back.isDown);
							changed |= UpdateDigitalButtonState(&oldController->start, &newController->start, padstate->start.isDown);

							changed |= UpdateDigitalButtonState(&oldController->dpadDown, &newController->dpadDown, padstate->dpadDown.isDown);
							changed |= UpdateDigitalButtonState(&oldController->dpadUp, &newController->dpadUp, padstate->dpadUp.isDown);
							changed |= UpdateDigitalButtonState(&oldController->dpadLeft, &newController->dpadLeft, padstate->dpadLeft.isDown);
							changed |= UpdateDigitalButtonState(&oldController->dpadRight, &newController->dpadRight, padstate->dpadRight.isDown);

							if (padstate->leftStickY < 0) {
								changed |= UpdateDigitalButtonState(&oldController->dpadDown, &newController->dpadDown, true);
							} else if (padstate->leftStickY > 0) {
								changed |= UpdateDigitalButtonState(&oldController->dpadUp, &newController->dpadUp, true);
							}
							if (padstate->leftStickX < 0) {
								changed |= UpdateDigitalButtonState(&oldController->dpadLeft, &newController->dpadLeft, true);
							} else if (padstate->leftStickX > 0) {
								changed |= UpdateDigitalButtonState(&oldController->dpadRight, &newController->dpadRight, true);
							}

							if (changed) {
								UpdateActiveController(newInput, (int32_t)controllerIndex);
							}
						}
					} break;

					default:
						break;
				}
			}
			break;

			case fplEventType_Mouse:
			{
				switch (ev.mouse.type) {
					case fplMouseEventType_Move:
					{
						// The window is rendered with a top-left origin and y pointing down, which is exactly what the platform reports, so the position needs no unprojection at all
						Vec2i screenMousePos = V2iInit(ev.mouse.mouseX, ev.mouse.mouseY);
						Vec2f worldMousePos = V2fInit((float)ev.mouse.mouseX, (float)ev.mouse.mouseY);
						newInput->mouse.screenPos = screenMousePos;
						newInput->mouse.worldPos = worldMousePos;
					} break;

					case fplMouseEventType_Button:
					{
						bool isDown = ev.mouse.buttonState >= fplButtonState_Press;
						switch (ev.mouse.mouseButton) {
							case fplMouseButtonType_Left:
								UpdateKeyboardButtonState(&newInput->mouse.left, isDown);
								UIInputSetButton(&app->uiInput.mouseButtons[FUI_MOUSE_LEFT], isDown);
								break;
							case fplMouseButtonType_Right:
								UpdateKeyboardButtonState(&newInput->mouse.right, isDown);
								UIInputSetButton(&app->uiInput.mouseButtons[FUI_MOUSE_RIGHT], isDown);
								break;
							case fplMouseButtonType_Middle:
								UpdateKeyboardButtonState(&newInput->mouse.middle, isDown);
								UIInputSetButton(&app->uiInput.mouseButtons[FUI_MOUSE_MIDDLE], isDown);
								break;
							default:
								break;
						}
					} break;

					case fplMouseEventType_Wheel:
					{
						newInput->mouse.wheelDelta = ev.mouse.wheelDelta;
					} break;

					default:
						break;
				}
			}
			break;

			case fplEventType_Window:
			{
				switch (ev.window.type) {
					case fplWindowEventType_DroppedFiles:
					{
						if (ev.window.dropFiles.fileCount != 1) {
							break;
						}

						const char *filePath = ev.window.dropFiles.files[0];

						// After the next fplWindowUpdate() the memory for dropFiles are freed automatically, so we have to copy it
						if (globalTransientMemory.temporary.type == fmemType_Temporary || fmemBeginTemporary(&globalTransientMemory.base, &globalTransientMemory.temporary)) {
							emulator->pendingROMFilePath = StringCreateFromSource(&globalTransientMemory.temporary, filePath);
							emulator->isROMFileRequested = true;
							fplAtomicIncrementS64(&globalTransientMemory.allocationCount);
						} else {
							// Handle error!
							fplAlwaysAssert(!"Failed beginnning a temorary memory");
						}
					} break;

					default:
						break;
				}
			} break;

			default:
				break;
		}
	}
}

static fgbGamePakLoadResultType LoadGamePakFromZipFile(const fgbCallbacks *callbacks, const char *zipFilePath, fgbGamePak *outGamePak) {
	mz_zip_archive archive = { 0 };
	uint8_t *zipMemory = NULL;
	uint8_t *itemMemory = NULL;
	size_t zipFileSize = 0;
	fgbGamePakLoadResultType result = fgbGamePakLoadResultType_FileError;

	fplFileHandle zipFile = { 0 };
	if (!fplFileOpenBinary(zipFilePath, &zipFile)) {
		return fgbGamePakLoadResultType_FileNotFound;
	}

	zipFileSize = fplFileGetSizeFromHandle(&zipFile);
	if (zipFileSize == 0) {
		fplFileClose(&zipFile);
		return fgbGamePakLoadResultType_NotEnoughData;
	}

	zipMemory = callbacks->allocateMemory(zipFileSize, callbacks->memoryAllocationUserData);
	if (zipMemory == NULL) {
		fplFileClose(&zipFile);
		return fgbGamePakLoadResultType_MemoryErrorFile;
	}

	fplFileReadBlock(&zipFile, zipFileSize, zipMemory, zipFileSize);
	fplFileClose(&zipFile);

	mz_zip_zero_struct(&archive);
	if (!mz_zip_reader_init_mem(&archive, zipMemory, zipFileSize, 0)) {
		result = fgbGamePakLoadResultType_FileError;
		goto cleanup;
	}

	uint32_t fileCount = mz_zip_reader_get_num_files(&archive);
	if (fileCount == 0) {
		mz_zip_reader_end(&archive);
		result = fgbGamePakLoadResultType_UnsupportedFormat;
		goto cleanup;
	}

	char itemFilename[100];

	uint32_t romFileIndex = UINT32_MAX;

	for (uint32_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		if (mz_zip_reader_is_file_a_directory(&archive, fileIndex)) {
			continue;
		}

		mz_zip_reader_get_filename(&archive, fileIndex, itemFilename, fplArrayCount(itemFilename));

		const char *itemFileExt = fplExtractFileExtension(itemFilename);

		if (StringCompareIgnoreCase(itemFileExt, ".gb") || StringCompareIgnoreCase(itemFileExt, ".gbc")) {
			romFileIndex = fileIndex;
			break;
		}
	}

	if (romFileIndex == UINT32_MAX) {
		// No ROM file found
		result = fgbGamePakLoadResultType_NotEnoughData;
		goto failed;
	}

	mz_zip_archive_file_stat itemStat = { 0 };
	if (!mz_zip_reader_file_stat(&archive, romFileIndex, &itemStat)) {
		// No size for entry?
		result = fgbGamePakLoadResultType_FileError;
		goto failed;
	}

	size_t itemFileSize = (size_t)itemStat.m_uncomp_size;

	itemMemory = (uint8_t *)callbacks->allocateMemory(itemFileSize, callbacks->memoryAllocationUserData);
	if (itemMemory == NULL) {
		// Allocation failed
		result = fgbGamePakLoadResultType_MemoryErrorFile;
		goto failed;
	}

	if (!mz_zip_reader_extract_to_mem(&archive, romFileIndex, itemMemory, itemFileSize, 0)) {
		// Extract failed
		callbacks->freeMemory(itemMemory, callbacks->memoryAllocationUserData);
		result = fgbGamePakLoadResultType_FileError;
		goto failed;
	}

	fgbGamePakLoadResultType loadRes = fgbGamePakLoadFromMemory(itemMemory, itemFileSize, outGamePak);

	callbacks->freeMemory(itemMemory, callbacks->memoryAllocationUserData);

	if (loadRes != fgbGamePakLoadResultType_Success) {
		// GamePak loading failed
		result = loadRes;
		goto failed;
	}

	fplCopyString(zipFilePath, outGamePak->filePath, fplArrayCount(outGamePak->filePath));	

	result = fgbGamePakLoadResultType_Success;
	goto cleanup;

failed:
	result = false;
	goto cleanup;

cleanup:
	if (archive.m_zip_type != MZ_ZIP_TYPE_INVALID) {
		mz_zip_reader_end(&archive);
	}

	if (zipMemory != NULL) {
		callbacks->freeMemory(zipMemory, callbacks->memoryAllocationUserData);
	}
	return result;
}

typedef struct {
	uint32_t frequency;
	uint32_t frameIndex;
	float toneVolume;
} AudioSineWaveData;

static AudioSineWaveData g_sinewave = { 0 };

static float AudioClipF32(const float value) {
	return fplMax(-1.0f, fplMin(1.0f, value));
}

typedef struct {
	fgbSystem *system;
	Emulator *emulator;
} AudioThreadState;

static uint32_t AudioThreadCallback(const fplAudioFormat *deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	static uint8_t AudioTempSampels[FGB_APU_MAX_SAMPLE_COUNT] = { 0 };

	AudioThreadState *threadState = (AudioThreadState *)userData;

	fgbSystem *system = threadState->system;

	Emulator *emulator = threadState->emulator;

	PerformanceMetrics *metrics = &emulator->performanceMetrics;

	BeginPerformanceCounter(&metrics->audioThread, fplTimestampQuery());

	uint32_t result;

	fgbEmulationState state = fgbGetState(system);
	if (!emulator->isActive || state == fgbEmulationState_Paused || state == fgbEmulationState_Error) {
		// Drain the APU ring buffer so accumulated samples from stepping don't
		// play back as a burst when execution resumes.
		fgbFetchAudioSamples(system, frameCount, AudioTempSampels);
		fplMemorySet(outputSamples, 0, frameCount * deviceFormat->channels * sizeof(int16_t));
		result = frameCount;
	} else {
		int16_t *out16 = (int16_t *)outputSamples;

		BeginPerformanceCounter(&metrics->audioReadSamples, fplTimestampQuery());
		// fgbFetchAudioSamples zero-fills (with silence = 128) any shortfall
		// inside AudioTempSampels, but still returns the actual frame count so
		// the underrun can be logged for diagnostics.
		const uint32_t popped = fgbFetchAudioSamples(system, frameCount, AudioTempSampels);
		EndPerformanceCounter(&metrics->audioReadSamples, fplTimestampQuery());

		if (popped == 0) {
			// No audio frames, return silence
			fplMemorySet(outputSamples, 0, frameCount * deviceFormat->channels * sizeof(int16_t));
		} else if (popped < frameCount) {
			const uint32_t missing = frameCount - popped;
			fplDebugFormatOut("APU Audio buffer underrun: %u frames are missing\n", missing);
		}

		// Always convert the full frameCount — the silence-padded tail keeps
		// the audio device from playing stale samples from the previous callback.
		result = frameCount;

		BeginPerformanceCounter(&metrics->audioOutputSamples, fplTimestampQuery());
		// Muting is a gain of zero rather than an early out, so the device still receives the silence it
		// expects for this callback and never replays the previous buffer
		const float outputGain = emulator->isSoundEnabled ? emulator->masterVolume : 0.0f;
		for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			for (uint8_t channelIndex = 0; channelIndex < 2; ++channelIndex) {
				uint8_t rawSample = AudioTempSampels[frameIndex * 2 + channelIndex];
				float sampleF32 = (float)rawSample / 255.0f;
				uint8_t sampleU8 = (uint8_t)((sampleF32 * outputGain) * 255.0f);
				int16_t sampleS16 = (int16_t)((sampleU8 << 8) - INT16_MIN);
				out16[frameIndex * 2 + channelIndex] = sampleS16;
			}
		}
		EndPerformanceCounter(&metrics->audioOutputSamples, fplTimestampQuery());
	}

	// Pure sine wave
#if 0
	uint32_t result = 0;

	if (g_sinewave.frequency == 0) {
		g_sinewave.frequency = 440;
		g_sinewave.toneVolume = 0.25;
		g_sinewave.frameIndex = 0;
	}

	for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		uint32_t f = frameIndex + g_sinewave.frameIndex;
		double t = sin((2.0 * M_PI * g_sinewave.frequency) / deviceFormat->sampleRate * f);
		float sampleValue = (float)(t * g_sinewave.toneVolume);
		float clip = AudioClipF32(sampleValue);
		out16[frameIndex * 2 + 0] = (int16_t)(clip * INT16_MAX);
		out16[frameIndex * 2 + 1] = (int16_t)(clip * INT16_MAX);
	}
	g_sinewave.frameIndex += frameCount;

	result += frameCount;
#endif

	EndPerformanceCounter(&metrics->audioThread, fplTimestampQuery());

	return result;
}

static char disassemblyTempBuffer[256] = { 0 };
static char disassemblyLineBuffer[1024] = { 0 };

static uint8_t disassemblyMemory[0xFFFF] = { 0 };
static uint8_t disassemblyVisited[65536] = { 0 };
static uint8_t disassemblyQueued[65536] = { 0 };
static uint16_t disassemblyBFSQueue[65536] = { 0 };

static void ClearDisassembly(StringList *lines, IndexHashtable *hashtable) {
	StringListClear(lines);
	IndexHashtableClear(hashtable);
}

static void LoadDisassembly(fgbSystem *system, StringList *lines, IndexHashtable *hashtable) {
	StringListClear(lines);
	IndexHashtableClear(hashtable);

	// TODO(final): Address not correct for GBC when GBC boot rom is active!
	const uint16_t addressRange = system->boot.state.isActive ? 0xFF : 0xFFFF;

	// Read all values from the entire address range into a memory array
	for (uint16_t address = 0; address < addressRange; ++address) {
		disassemblyMemory[address] = fgbBusRead8(system, address);
	}

	// Stup to use fgbDecodeInstruction
	fgbMemory rom = { 0 };
	rom.data = disassemblyMemory;
	rom.length = addressRange;

	// BFS from known entry points to discover real instruction starts.
	// This avoids misalignment from overlapping instruction sequences.
	fplMemorySet(disassemblyVisited, 0, sizeof(disassemblyVisited));
	fplMemorySet(disassemblyQueued, 0, sizeof(disassemblyQueued));

	static const uint16_t bfsEntryPoints[] = {
		// RST vectors
		0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038,
		// Interrupt vectors
		0x0040, 0x0048, 0x0050, 0x0058, 0x0060,
		// Game entry point
		0x0100,
	};

	uint32_t queueHead = 0, queueTail = 0;
	for (uint32_t i = 0; i < fplArrayCount(bfsEntryPoints); ++i) {
		const uint16_t ep = bfsEntryPoints[i];
		if (ep < addressRange && !disassemblyQueued[ep]) {
			disassemblyQueued[ep] = 1;
			disassemblyBFSQueue[queueTail++] = ep;
		}
	}

	fgbDecodedInstruction decoded = fplZeroInit;

	while (queueHead < queueTail) {
		uint16_t pos = disassemblyBFSQueue[queueHead++];

		if (disassemblyVisited[pos]) {
			continue;
		}
		disassemblyVisited[pos] = 1;

		fplClearStruct(&decoded);
		if (!fgbDecodeInstruction(&rom, pos, &decoded)) {
			continue;
		}

		const uint16_t nextSeq = (uint16_t)(pos + decoded.length);

		// Unconditional transfers have no sequential fall-through.
		const bool noFallthrough =
			decoded.opCode == 0xC3 ||  // JP a16
			decoded.opCode == 0xE9 ||  // JP HL (indirect)
			decoded.opCode == 0x18 ||  // JR e8
			decoded.opCode == 0xC9 ||  // RET
			decoded.opCode == 0xD9;    // RETI

		if (!noFallthrough && nextSeq < addressRange && !disassemblyQueued[nextSeq]) {
			disassemblyQueued[nextSeq] = 1;
			disassemblyBFSQueue[queueTail++] = nextSeq;
		}

		// Extract and queue static jump targets.
		if ((decoded.type == fgbInstructionType_JP || decoded.type == fgbInstructionType_CALL) &&
		    decoded.mode == fgbAddressingMode_U16 && decoded.operandCount > 0) {
			const uint16_t target = decoded.operands[0].immediate.u16;
			if (target < addressRange && !disassemblyQueued[target]) {
				disassemblyQueued[target] = 1;
				disassemblyBFSQueue[queueTail++] = target;
			}
		} else if (decoded.type == fgbInstructionType_JR &&
		           decoded.mode == fgbAddressingMode_I8 && decoded.operandCount > 0) {
			const int32_t target = (int32_t)nextSeq + (int32_t)decoded.operands[0].immediate.slow;
			if (target >= 0 && (uint32_t)target < addressRange && !disassemblyQueued[(uint16_t)target]) {
				disassemblyQueued[(uint16_t)target] = 1;
				disassemblyBFSQueue[queueTail++] = (uint16_t)target;
			}
		} else if (decoded.type == fgbInstructionType_RST &&
		           decoded.mode == fgbAddressingMode_Constant && decoded.operandCount > 0) {
			const uint16_t target = decoded.operands[0].constant;
			if (target < addressRange && !disassemblyQueued[target]) {
				disassemblyQueued[target] = 1;
				disassemblyBFSQueue[queueTail++] = target;
			}
		}
	}

	// Emit disassembly in address order.
	// - Header bytes (0x0100-0x014F) are always labeled GAMEPAK_HEADER.
	// - BFS-confirmed addresses are decoded as guaranteed-correct instructions.
	// - Other bytes use best-effort linear decode, but never cross a BFS boundary
	// - If a multi-byte decode would consume a confirmed address, fall back to DB.
	uint16_t pos = 0;
	while (pos < rom.length) {
		fplClearStruct(&decoded);
		disassemblyLineBuffer[0] = 0;

		const bool inHeader = (pos >= FGB__GAMEPAK_HEADER_POSITION) && (pos < (uint16_t)(FGB__GAMEPAK_HEADER_POSITION + sizeof(fgb__GamePakHeader)));
		const bool isConfirmed = disassemblyVisited[pos] != 0;

		uint32_t instrLen = 1;
		bool showAsHeader = false;
		bool showAsCode = false;

		if (inHeader) {
			showAsHeader = true;
		} else if (isConfirmed) {
			if (fgbDecodeInstruction(&rom, pos, &decoded)) {
				instrLen = decoded.length;
				showAsCode = true;
			}
		} else {
			// Best-effort linear decode; abort if it would swallow a BFS boundary.
			if (fgbDecodeInstruction(&rom, pos, &decoded)) {
				const uint32_t tentLen = decoded.length;
				bool wouldSplit = false;
				for (uint32_t k = 1; k < tentLen; ++k) {
					if ((pos + k) < addressRange && disassemblyVisited[pos + k]) {
						wouldSplit = true;
						break;
					}
				}
				if (!wouldSplit) {
					instrLen = tentLen;
					showAsCode = true;
				}
			}
		}

		fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "%04X | ", pos);
		fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));

		const uint8_t MaxInstructionLength = 3;

		const int m = MaxInstructionLength - (int8_t)instrLen;
		if (m > 0) {
			for (int x = 0; x < m; x++) {
				fplStringAppend("   ", disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
			}
		}
		for (uint32_t x = 0; x < instrLen; x++) {
			fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "%02X ", rom.data[pos + x]);
			fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		}

		fplStringAppend("| ", disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));

		if (showAsHeader) {
			fplStringAppend("GAMEPAK_HEADER", disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		} else if (showAsCode) {
			fgbFormatInstruction(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), &decoded);
			fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		} else {
			fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "DB $%02X", rom.data[pos]);
			fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		}

		const size_t listIndex = StringListAdd(lines, disassemblyLineBuffer);
		IndexHashtableAdd(hashtable, pos, listIndex);

		pos += instrLen;
	}
}

static bool IsEmulatorGameLoaded(const Emulator *emulator) {
	return emulator->isActive;
}

static void EmulatorUnloadGame(Emulator *emulator) {
	if (emulator->isActive) {
		emulator->isActive = false;
		emulator->currentROMBank = 0;
		WakeupEmulatorThread(emulator);
		fgbShutdown(&emulator->system);
	}
	for (size_t i = 0; i < fplArrayCount(emulator->states.snapshots); ++i) {
		fgbSnapshot *snapshot = emulator->states.snapshots + i;
		fplClearStruct(snapshot);

		Texture *texture = emulator->states.textures + i;
		ClearPixelsTexture(texture);
		RendererTextureUpdate(texture);
	}
}

static bool EmulatorLoadGame(Emulator *emulator, const char *filePath) {
	if (fplGetStringLength(filePath) == 0 || !fplFileExists(filePath)) {
		return false;
	}

	const char *fileExt = fplExtractFileExtension(filePath);

	fgbGamePak *gamepak = (fgbGamePak *)fgb__AllocateMemory(&globalCallbacks, sizeof(fgbGamePak));
	if (gamepak == NULL) {
		return false;
	}

	fgbGamePakLoadResultType loadRes;
	if (StringCompareIgnoreCase(".zip", fileExt)) {
		loadRes = LoadGamePakFromZipFile(&globalCallbacks, filePath, gamepak);
	} else {
		loadRes = fgbGamePakLoadFromFile(&globalCallbacks, filePath, gamepak);
	}

	if (loadRes != fgbGamePakLoadResultType_Success) {
		fgb__FreeMemory(&globalCallbacks, gamepak);
		return false;
	}

	ResetPerformanceMetrics(&emulator->performanceMetrics);

	// Replay any loaded input script from the beginning for the freshly loaded game
	fgbInputSimRestart(&emulator->inputSim);
	emulator->inputSimFrameIndex = 0;

	fgbInitResult initRes = fgbInit(&emulator->system, &emulator->config, gamepak);

	fgb__FreeMemory(&globalCallbacks, gamepak);

	if (initRes != fgbInitResult_Success) {
		return false;
	}

	switch (emulator->paletteType) {
		case ColorPaletteType_SGB:
			fgbSetColorPalette(&emulator->system, &FGB_DEFAULT_SGB_COLORS);
			break;

		case ColorPaletteType_MGB:
			fgbSetColorPalette(&emulator->system, &FGB_DEFAULT_MGB_COLORS);
			break;

		case ColorPaletteType_Blue:
			fgbSetColorPalette(&emulator->system, &BlueMonochromeColors);
			break;

		case ColorPaletteType_DMG:
		default:
			fgbSetColorPalette(&emulator->system, &FGB_DEFAULT_DMG_COLORS);
			break;
	}

	// Load states from files
	for (size_t i = 0; i < fplArrayCount(emulator->states.snapshots); ++i) {
		fgbSnapshot *snapshot = emulator->states.snapshots + i;
		Texture *texture = emulator->states.textures + i;

		fplClearStruct(snapshot);

		if (fgbSnapshotLoadFromFile(&emulator->system, filePath, i, snapshot)) {
			TransferPixelsToTexture(snapshot->ppu.display, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, texture);
			RendererTextureUpdate(texture);
		} else {
			ClearPixelsTexture(texture);
			RendererTextureUpdate(texture);
		}
	}

	emulator->isActive = true;
	WakeupEmulatorThread(emulator);

	return true;
}

static void SetupPlatformSettings(fmemMemoryBlock *mainMemory, fplSettings *settings) {
	fplSetDefaultSettings(settings);

	settings->memory.dynamic.allocateCallback = CustomMemoryDynamicAllocate;
	settings->memory.dynamic.releaseCallback = CustomMemoryDynamicRelease;
	settings->memory.dynamic.mode = fplMemoryAllocationMode_Custom;
	settings->memory.dynamic.userData = mainMemory;

	fplCopyString("FGB - Final Gamebox", settings->window.title, fplArrayCount(settings->window.title));

	settings->video.backend = fplVideoBackendType_OpenGL;
	settings->video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings->video.isVSync = false;

	settings->audio.targetFormat.sampleRate = 48000;
	settings->audio.targetFormat.channels = 2;
	settings->audio.targetFormat.type = fplAudioFormatType_S16;
	settings->audio.startAuto = false;
	settings->audio.stopAuto = false;
}

static void SetupGamebox(fgbConfiguration *config, Application *app, const uint32_t sampleRate, const EmulatorParameters *parameters) {
	// Callbacks
	config->callbacks = globalCallbacks;

	// Logging
	config->log.userData = app;
	config->log.callback = GameboxLog;
	config->log.isEnabled = true;
	config->debug.isInstructionTraceEnabled = parameters->isTraceEnabled;

	// Audio
	config->targetSampleRate = sampleRate;

	// Micro stepping
	config->debug.microStepping.filter[fgbMicroStepType_CPUTick] = true;
	config->debug.microStepping.filter[fgbMicroStepType_HardwareTick] = true;
	config->debug.microStepping.userData = &app->emulator;
	config->debug.microStepping.callback = GameboxMicroStepHandler;
	config->debug.microStepping.isEnabled = true;

	// Break points
	config->debug.breakpoints.filter[fgbBreakpointType_LCDControlPower] = false;
	config->debug.breakpoints.filter[fgbBreakpointType_LCDControlMode] = false;
	config->debug.breakpoints.userData = &app->emulator;
	config->debug.breakpoints.callback = GameboxBreakpointHandler;
	config->debug.breakpoints.isEnabled = true;

	// Run rom immediately
	config->paused = false;

	fplClearStruct(&config->bootROM);
#if !defined(NO_BOOTROM)
	fplMemoryCopy(ptr_bootROM_DMG, 0x100, config->bootROM.data);
#endif
}

static void SetupInput(InputState *oldInput, InputState *newInput, const double frameRate, const bool isDebug) {
	newInput->frameRate = frameRate;
	newInput->debug.isEnabled = isDebug;
	newInput->activeControllerIndex = oldInput->activeControllerIndex;

	// Keyboard controller is always connected
	ControllerInput *newKeyboardController = &newInput->keyboardController;
	ControllerInput *oldKeyboardController = &oldInput->keyboardController;
	fgbClearStruct(newKeyboardController);
	newKeyboardController->state = ControllerState_Connected;
	for (uint32_t buttonIndex = 0; buttonIndex < fplArrayCount(newKeyboardController->buttons); ++buttonIndex) {
		newKeyboardController->buttons[buttonIndex].endedDown = oldKeyboardController->buttons[buttonIndex].endedDown;
	}

	// Remember debug input
	DebugInput *newDebugInput = &newInput->debug;
	DebugInput *oldDebugInput = &oldInput->debug;
	fplClearStruct(newDebugInput);
	for (uint32_t buttonIndex = 0; buttonIndex < fplArrayCount(newDebugInput->buttons); ++buttonIndex) {
		newDebugInput->buttons[buttonIndex].endedDown = oldDebugInput->buttons[buttonIndex].endedDown;
	}

	// Remember mouse input
	MouseInput *newMouse = &newInput->mouse;
	MouseInput *oldMouse = &oldInput->mouse;
	fplClearStruct(newMouse);
	for (uint32_t buttonIndex = 0; buttonIndex < fplArrayCount(newMouse->buttons); ++buttonIndex) {
		newMouse->buttons[buttonIndex].endedDown = oldMouse->buttons[buttonIndex].endedDown;
	}
	newInput->mouse.worldPos = oldInput->mouse.worldPos;

	// Remember previous gamepad connected states
	for (uint32_t controllerIndex = 1; controllerIndex < fplArrayCount(newInput->controllers); ++controllerIndex) {
		ControllerInput *newGamepadController = &newInput->controllers[controllerIndex];
		ControllerInput *oldGamepadController = &oldInput->controllers[controllerIndex];
		fplClearStruct(newGamepadController);
		newGamepadController->state = oldGamepadController->state;
		for (uint32_t buttonIndex = 0; buttonIndex < fplArrayCount(newGamepadController->buttons); ++buttonIndex) {
			newGamepadController->buttons[buttonIndex].endedDown = oldGamepadController->buttons[buttonIndex].endedDown;
		}
	}
}

static void SwapInput(InputState **oldInput, InputState **newInput) {
	InputState *tmp = *newInput;
	*newInput = *oldInput;
	*oldInput = tmp;
}

// True while a loaded input script still has pending events -> the simulator owns the joypad
static bool EmulatorInputSimIsActive(const Emulator *emulator) {
	return emulator->inputSim.isEnabled && emulator->inputSim.appliedCount < emulator->inputSim.eventCount;
}

static void SetupGameboxInput(const InputState *newInput, fgbSystem *system) {
	// Translate controller states to game boy buttons
	if (newInput->activeControllerIndex > -1) {
		int index = newInput->activeControllerIndex;
		const ControllerInput *newController = &newInput->controllers[index];

		// ControllerInput button mapping is identical of FGB
		for (uint8_t buttonIndex = 0; buttonIndex < 8; ++buttonIndex) {
			if (UIIsDown(&newController->buttons[buttonIndex])) {
				fgbSetButtonState(system, fgbButtonType_Start + buttonIndex, true);
			} else {
				fgbSetButtonState(system, fgbButtonType_Start + buttonIndex, false);
			}
		}
	} else {
		fgbClearButtons(system);
	}
}

static bool HasRequestedROMFile(const Application *app) {
	const Emulator *emulator = &app->emulator;
	return emulator->isROMFileRequested;
}

static void LoadRequestedROMFile(Application *app, String *romFilePath) {
	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;

	if (globalTransientMemory.temporary.type != fmemType_Temporary) {
		fplAlwaysAssert(!"No transient memory block active!");
		return;
	}

	if (IsEmulatorGameLoaded(emulator)) {
		EmulatorUnloadGame(emulator);
		ClearDisassembly(&app->disassembly, &app->disassemblyHashTable);
		StringListClear(&app->console);
		app->consoleList.selectedIndex = -1;
		app->disassemblyList.selectedIndex = -1;
		app->disassemblyHighlightIndex = -1;
	}

	if (EmulatorLoadGame(emulator, romFilePath->text)) {
		LoadDisassembly(system, &app->disassembly, &app->disassemblyHashTable);
	}

	if (globalTransientMemory.temporary.type == fmemType_Temporary) {
		int64_t count = fplAtomicAddAndFetchS64(&globalTransientMemory.allocationCount, -1);
		if (count == 0) {
			fmemEndTemporary(&globalTransientMemory.temporary);
		}
	}

	fplClearStruct(romFilePath);
	emulator->isROMFileRequested = false;
}

static void PrepareInputUI(Application *app, const InputState *newInput, const float deltaTime) {
	// The keys and the typed text were already accumulated into app->uiInput by ProcessEvents, so only
	// what is sampled once per frame is filled in here
	fuiInput *uiInput = &app->uiInput;

	uiInput->mousePosition = fuiV2(newInput->mouse.worldPos.x, newInput->mouse.worldPos.y);
	uiInput->mouseWheelDelta = newInput->mouse.wheelDelta;
	uiInput->windowSize = fuiV2i(app->windowSize.w, app->windowSize.h);
	uiInput->deltaTime = deltaTime;
	uiInput->isActive = true;
}

static void HandleDefaultInput(Application *app, const InputState *newInput) {
	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;

	if (UIWasPressed(&newInput->debug.toggleDebug)) {
		app->isDebugEnabled = !app->isDebugEnabled;
	}

	if (emulator->isActive && system->state != fgbEmulationState_Error) {
		uint64_t currentPC = (uint64_t)system->cpu.registers.pc;
		if (currentPC != app->lastDisassemblyScrollPC) {
			bool isPaused = system->state != fgbEmulationState_Running;
			fplTimestamp now = fplTimestampQuery();
			double elapsed = fplTimestampElapsed(app->lastDisassemblyScrollTime, now);
			if (isPaused || elapsed >= 0.1) {
				HighlightScrollDisassembly(app);
				app->lastDisassemblyScrollTime = now;
				app->lastDisassemblyScrollPC = currentPC;
			}
		}
	}
}

static bool HasGameboxVideoChanges(const Emulator *emulator) {
	return emulator->isActive;
}

static void UploadStateTextures(States *states) {
	for (size_t i = 0; i < fplArrayCount(states->snapshots); ++i) {
		fgbSnapshot *snapshot = states->snapshots + i;
		Texture *texture = states->textures + i;
		if (texture->state == TextureState_Update) {
			TransferPixelsToTexture(snapshot->ppu.display, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, texture);
			RendererTextureUpdate(texture);
			texture->state = TextureState_None;
		} else if (texture->state == TextureState_Clear) {
			ClearPixelsTexture(texture);
			RendererTextureClear(texture);
		}
	}
}

static void UploadGameboxTextures(Application *app) {
	Emulator *emulator = &app->emulator;

	if (!emulator->isActive) {
		return;
	}

	static FrameSnapshotDisplay scratchDisplay;
	static FrameSnapshotBackgroundMap scratchBgMap;
	static FrameSnapshotTilemap scratchTilemap;

	if (DisplayFrameQueuePopNewest(&emulator->displayQueue, &scratchDisplay)) {
		TransferPixelsToTexture(scratchDisplay.pixels, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, &app->displayTexture);
		RendererTextureUpdate(&app->displayTexture);
	}
	if (BackgroundMapFrameQueuePopNewest(&emulator->backgroundMapQueue, &scratchBgMap)) {
		TransferPixelsToTexture(scratchBgMap.pixels, FGB_BACKGROUND_MAP_WIDTH, FGB_BACKGROUND_MAP_HEIGHT, &app->backgroundMapTexture);
		RendererTextureUpdate(&app->backgroundMapTexture);
	}
	if (TilemapFrameQueuePopNewest(&emulator->tilemapQueue, &scratchTilemap)) {
		TransferPixelsToTexture(scratchTilemap.pixels, FGB_TILEMAP_WIDTH, FGB_TILEMAP_HEIGHT, &app->tileMapTexture);
		RendererTextureUpdate(&app->tileMapTexture);
	}
}

typedef struct {
	fplTimestamp frameStart;
	fplTimestamp frameEnd;

	uint32_t frameCount;

	double elapsedTime;
	double overSleepDuration;
	double frameRate;
	double fpsAccumulator;
} FrameTiming;

static void InitializeFrameTiming(FrameTiming *frameTiming) {
	fplClearStruct(&frameTiming->frameStart);
	fplClearStruct(&frameTiming->frameEnd);

	frameTiming->frameCount = 0;

	frameTiming->elapsedTime = 0;
	frameTiming->overSleepDuration = 0;
	frameTiming->frameRate = (double)FGB_DISPLAY_REFRESH_RATE;
	frameTiming->fpsAccumulator = 0;

	frameTiming->frameStart = fplTimestampQuery();
}

static void ComputeFrameTiming(FrameTiming *frameTiming, const double targetFrameTime) {
	frameTiming->frameEnd = fplTimestampQuery();
	double elapsedTime = fplTimestampElapsed(frameTiming->frameStart, frameTiming->frameEnd);

	// We are a bit too fast, wait for a while
	while (elapsedTime < targetFrameTime) {
		if ((elapsedTime + frameTiming->overSleepDuration) >= targetFrameTime) {
			frameTiming->overSleepDuration -= targetFrameTime - elapsedTime;
		}

		fplThreadYield();

		frameTiming->frameEnd = fplTimestampQuery();
        elapsedTime = fplTimestampElapsed(frameTiming->frameStart, frameTiming->frameEnd);

		if (elapsedTime > targetFrameTime) {
			frameTiming->overSleepDuration += elapsedTime - targetFrameTime;
		}
	}

	frameTiming->frameEnd = fplTimestampQuery();
	elapsedTime = fplTimestampElapsed(frameTiming->frameStart, frameTiming->frameEnd);
	frameTiming->fpsAccumulator += elapsedTime;
    frameTiming->frameCount += 1;

	if (frameTiming->fpsAccumulator >= 1.0)
    {
        frameTiming->frameRate = (double)frameTiming->frameCount * 0.5 + frameTiming->frameRate * 0.5;
        frameTiming->fpsAccumulator = 0;
        frameTiming->frameCount = 0;
    }

	frameTiming->frameStart = fplTimestampQuery();
}

static void PrepareFrame(Application *app, const fplWindowSize size) {
	float w = (float)size.width;
	float h = (float)size.height;

	float scale = 1.0f;

	float translationX = w * (1.0f - scale) * 0.5f;
	float translationY = h * (1.0f - scale) * 0.5f;

	app->windowSize = V2iInit((int)size.width, (int)size.height);
	app->viewMat = M4fMult(M4fScaleScalar(scale), M4fTranslationV2(V2fInit(translationX, translationY)));
	// Top-left origin with y pointing down, which is the convention final_ui.h emits its geometry in
	app->projectionMat = M4fOrthoRH(0.0f, (float)size.width, (float)size.height, 0.0f, 0.0f, 1.0f);
	app->viewProjectionMat = M4fMult(app->projectionMat, app->viewMat);
	app->viewport = VP4iInit(0, 0, (int)size.width, (int)size.height);
}

static void UpdateWindowTitle(const Emulator *emulator, const double frameRate) {
	static char windowTitleBuffer[512];

	const char *gamePakTitle = fplExtractFileName(emulator->system.gamePak.filePath);
	if (fplGetStringLength(gamePakTitle) == 0) {
		gamePakTitle = "No game pak loaded";
	}
	fplStringFormat(windowTitleBuffer, fplArrayCount(windowTitleBuffer), "FPL Emulator - %s - (Fps: %.2f)\n", gamePakTitle, frameRate);
	fplSetWindowTitle(windowTitleBuffer);
}

static bool CharIsAlpha(const char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static EmulatorParameters ParseEmulatorParameters(const int argc, char **argv) {
	EmulatorParameters result = fplZeroInit;
	if (argc >= 2) {
		int argIndex = 1;
		while (argIndex < argc) {
			const char *arg = argv[argIndex];
			if (strlen(arg) >= 2) {
				if (arg[0] == '-') {
					const char c = arg[1];
					if (CharIsAlpha(c)) {
						// Single char argument
						if (c == 't') {
							result.isTraceEnabled = true;
						} else {
							// Not supported argument
						}
					} else if (c == '-' && CharIsAlpha(arg[2])) {
						// Long key argument
						const char *key = arg + 2;
						if (StringCompareIgnoreCase("trace", key) == 0) {
							result.isTraceEnabled = true;
						} else if (StringCompareIgnoreCase("input", key) == 0 && (argIndex + 1) < argc) {
							result.inputScriptFilePath = argv[argIndex + 1];
							argIndex++;
						} else {
							// Not supported argument
						}
					}
					argIndex++;
					continue;
				}
			}
			if (argIndex == argc - 1) {
				result.romFilePath = argv[argIndex];
			}
			++argIndex;
		}
	}
	return result;
}

// Main Entry Point (No need for WinMain or anything like that, due to FPL)
int main(int argc, char **argv) {
	int exitCode = 0;

	const EmulatorParameters parameters = ParseEmulatorParameters(argc, argv);

	const char *romFilePath = parameters.romFilePath;

	Application *app = fpl_null;
	RendererContext *renderer = fpl_null;

	// Allocate transient memory (Used for rom file and external ram file loading)
	globalTransientMemory.base = CreateTransientMemory(fplMegaBytes(16));

	// Allocate main memory
	fmemMemoryBlock mainMemory = CreatePersistentMemory(fplMegaBytes(64));
	if (mainMemory.base == fpl_null) {
		return ExitCode_OutOfMemory;
	}

	// Initialize random seed
	srand(42U);

	// Setup & Initialize Platform
	fplSettings *settings = fmemPushStruct(&mainMemory, fplSettings, fmemPushFlags_Clear);

	SetupPlatformSettings(&mainMemory, settings);

	if (!fplPlatformInit(fplInitFlags_All & ~fplInitFlags_Console, settings)) {
		return ExitCode_FailedInitializePlatform;
	}

	// Initialize the renderer (OpenGL)
	RendererSupport rendererSupport = fplZeroInit;
	renderer = RendererCreate(&mainMemory, &rendererSupport);
	if (renderer == fpl_null) {
		exitCode = ExitCode_FailedInitializeRenderer;
		goto shutdown;
	}



	// Create Application & Emulator resources and start the threads
	app = CreateApplication(&mainMemory, &parameters, &rendererSupport);
	if (app == fpl_null) {
		exitCode = ExitCode_OutOfMemory;
		goto shutdown;
	}

	Emulator *emulator = &app->emulator;

	fgbSystem *system = &emulator->system;

	// Get audio hardware format, so we can fill out the configuration properly
	fplAudioFormat hardwareAudioFormat = { 0 };
	fplGetAudioHardwareFormat(&hardwareAudioFormat);

	// Gamebox configuration
	SetupGamebox(&emulator->config, app, hardwareAudioFormat.sampleRate, &parameters);

	// Set audio callback and its data
	AudioThreadState audioThreadState = { 0 };
	audioThreadState.system = system;
	audioThreadState.emulator = emulator;
	fplSetAudioClientReadCallback(AudioThreadCallback, &audioThreadState);

	// Load an input script for automated testing, if requested (see INPUT_SIMULATOR.md)
	if (fplGetStringLength(parameters.inputScriptFilePath) > 0) {
		if (!fgbInputSimLoadFromFile(&emulator->inputSim, parameters.inputScriptFilePath)) {
			fplConsoleFormatError("Failed loading input script '%s'!\n", parameters.inputScriptFilePath);
		}
	}

	// Default game rom (Retroid from Jonas Fischbach)
	fplPathCombine(app->defaultGameRomFilePath, fplArrayCount(app->defaultGameRomFilePath), 2, app->romsPath, "Retroid.zip");
	if (fplGetStringLength(romFilePath) == 0) {
		romFilePath = app->defaultGameRomFilePath;
	}

	// Auto load initial rom file from arguments
	if (fplGetStringLength(romFilePath) > 0) {
		if (EmulatorLoadGame(emulator, romFilePath)) {
			LoadDisassembly(&emulator->system, &app->disassembly, &app->disassemblyHashTable);
		}
	}

	// Initialize Input
	InputState inputs[2] = fplZeroInit;
	InputState *newInput = &inputs[0];
	InputState *oldInput = &inputs[1];

#if !NO_CURSOR
	fplSetWindowCursorEnabled(false);
#endif

	// Initialize Timing States
	const double TARGET_FRAME_TIME = 1.0 / (double)FGB_DISPLAY_REFRESH_RATE;
	FrameTiming timing = { 0 };
	InitializeFrameTiming(&timing);

	// Start playback of audio samples
	fplPlayAudio();

	// Run main loop
	while (fplWindowUpdate()) {
		BeginPerformanceCounter(&emulator->performanceMetrics.frameTime, fplTimestampQuery());

		// Compute the matrices based on the window size
		fplWindowSize windowSize = fplZeroInit;
		fplGetWindowSize(&windowSize);
		PrepareFrame(app, windowSize);

		// Setup input for this frame, but ensure that previous button states are preserved
		SetupInput(oldInput, newInput, timing.frameRate, app->isDebugEnabled);

		// Half transitions and typed text are only true for one frame, while held keys carry over
		UIInputBeginFrame(&app->uiInput);

		// Process events and handle events from the window (Keyboard, Mouse, Gamepad, etc.)
		ProcessEvents(app, oldInput, newInput);

		// Setup gamebox input from new input, unless a scripted input replay currently owns the joypad
		if (!EmulatorInputSimIsActive(emulator)) {
			SetupGameboxInput(newInput, system);
		}

		// Show various informations in the window title bar
		UpdateWindowTitle(emulator, timing.frameRate);

		// Load a rom file, if requested by e.g. a drag & drop operation
		if (HasRequestedROMFile(app)) {
			LoadRequestedROMFile(app, &emulator->pendingROMFilePath);
		}

		// Setup input for UI
		PrepareInputUI(app, newInput, (float)TARGET_FRAME_TIME);

		// Default input handling, like switching to debug / play mode
		HandleDefaultInput(app, newInput);

		// Upload game textures, when there was any changes in the video system
		if (HasGameboxVideoChanges(emulator)) {
			BeginPerformanceCounter(&emulator->performanceMetrics.texturesUpload, fplTimestampQuery());
			UploadGameboxTextures(app);
			EndPerformanceCounter(&emulator->performanceMetrics.texturesUpload, fplTimestampQuery());
		}

		// Upload state textures
		UploadStateTextures(&emulator->states);

		// Render the entire frame
		RenderFrame(app, newInput);

		// Swap back and front buffer, showing the current frame the user
		fplVideoFlip();

		// Swap inputs and compute frame timings
		SwapInput(&oldInput, &newInput);
		ComputeFrameTiming(&timing, TARGET_FRAME_TIME);

		EndPerformanceCounter(&emulator->performanceMetrics.frameTime, fplTimestampQuery());
	}

shutdown:
	// We are done with the application, releases resources
	if (app != fpl_null) {
		ReleaseApplication(&app);
	}
	if (renderer != fpl_null) {
		RendererDestroy(&renderer);
	}

#if !NO_CURSOR
	fplSetWindowCursorEnabled(true);
#endif

	// Stop audio playback and release any platform resources
	fplStopAudio();
	fplPlatformRelease();

	// Release the entire main memory block
	fmemFree(&mainMemory);

	// Release transient memory block
	if (globalTransientMemory.allocationCount > 0) {
		fplAlwaysAssert("There are transient memory allocations left, indicating that we screwed up!");
		return -1;
	}

	fmemFree(&globalTransientMemory.base);

	return exitCode;
}

