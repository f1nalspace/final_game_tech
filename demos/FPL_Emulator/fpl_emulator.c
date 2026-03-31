/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Emulator

Description:
	Fully working game boy DMG emulator with a simple debugger based on the final_game_box.h
	
Features:

	- OpenGL Application with a custom immediate based UI
	- Loading GamePak roms from either raw or zip files
	- Emulator controls (Play, Pause, Stepping, etc.)
	- Visual Debugger with disassembly, breakpoints, various stepping modes
	- Rendering of internal states, such as CPU, PPU, APU, GamePak, etc.
	- Tilemap visualization
	- Color palette swapping
	- Full background map visualization with scroll area
	- Drag & Drop support for raw and zipped rom files 

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
	- Final Additions (Math, Fontloader)
	- Final Game Box
	- STB Image
	- STB TrueType
	- MiniZ

Author:
	Torsten Spaete

Changelog:
	## 2025-06-26
	- Initial version

Todo:

	- Unloading game button (very easy to do)
	- OAM Visualization (harder than it seems)
	- Add option to select background tile area, because relying on LCDC is not good

License:
	Copyright (c) 2024-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Final Platform Layer
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

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

// Final Additions
#include <final_math.h>

// Local
#include "fontdata.h"
#include "imagedata.h"
#include "ui.c"
#include "render.c"
#include "utils.c"

// Is the debug UI enabled at startup
#define DEBUG_AT_START 1

// Do not draw any mouse cursors
#define NO_CURSOR 1

// Set from FGB!
#define MAX_STATE_SLOT_COUNT 6
fplStaticAssert(MAX_STATE_SLOT_COUNT % 2 == 0);

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

	size_t remainingSize = fmemGetRemainingSize(&transientMem->temporary);

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

	int deltaIndex = x % PERFORMANCE_COUNTER_DELTA_CAPACITY;
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

typedef struct {
	States states;

	fgbSystem system;

	fgbConfiguration config;

	PerformanceMetrics performanceMetrics;

	fplMutexHandle mutex;
	fplConditionVariable waitCondition;
	fplConditionVariable microStepCondition;
	fplConditionVariable breakpointCondition;

	String pendingROMFilePath;

	fplThreadHandle *thread;

	fgbBreakpointType lastBreakpointType;

	ColorPaletteType paletteType;

	volatile uint32_t isActive;

	volatile uint32_t isROMFileRequested;

	volatile uint32_t isShutdown;

	volatile uint32_t isFrameFinished;
	volatile uint32_t isFrameStepActive;
	volatile uint32_t isMicroStepActive;
	volatile uint32_t isVRAMUpdated;

	float masterVolume;

	uint16_t currentROMBank;
} Emulator;

typedef struct {
	UICheckboxData checkboxes[FGB_BREAKPOINT_TYPE_COUNT];
} Breakpoints;

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
	UIDialogData dialog;
	UIButtonData closeButton;
	StatesDialogCellPos selectedSlotPos;
	DialogType type;
} StatesDialog;

typedef struct {
	UIContext uiCtx;

	char romsPath[1024];
	char defaultGameRomFilePath[1024];
		
	LoadedFont fontData;
	LoadedFont fontDataLarge;

	Mat4f projectionMat;
	Mat4f viewMat;
	Mat4f viewProjectionMat;

	Texture cursorTexture;
	Texture displayTexture;
	Texture backgroundMapTexture;
	Texture vramTexture;
	Texture fontTexture;
	Texture fontTextureLarge;
	Texture gbTexture;

	Viewport4i viewport;

	Vec2i windowSize;

	UIListboxData console;
	fmemMemoryBlock consoleMemory;

	UIListboxData disassemblyList;
	IndexHashtable disassemblyHashTable;
	fmemMemoryBlock disassemblyMemory;

	UIButtonData pauseOrResumeButton;
	UIButtonData resumeButton;
	UIButtonData frameStepButton;
	UIButtonData singleStepButton;
	UIButtonData microStepButton;
	UIButtonData resetButton;
	UIButtonData saveStateButton;
	UIButtonData restoreStateButton;

	UICheckboxData logEnabledCheckbox;
	UICheckboxData bootEnabledCheckbox;
	UICheckboxData initPauseCheckbox;

	UICheckboxData dmgPaletteCheckbox;
	UICheckboxData mgbPaletteCheckbox;
	UICheckboxData sgbPaletteCheckbox;
	UICheckboxData bluePaletteCheckbox;

	UICheckboxData voice1MuteCheckbox;
	UICheckboxData voice2MuteCheckbox;
	UICheckboxData voice3MuteCheckbox;
	UICheckboxData voice4MuteCheckbox;

	UICheckboxData ppuBackgroundCheckbox;
	UICheckboxData ppuWindowCheckbox;
	UICheckboxData ppuSpritesCheckbox;

	UITabControlData leftTabControl;
	UITabControlData rightTabControl;

	StatesDialog statesDialog;

	Breakpoints breakpoints;

	Emulator emulator;

	fpl_b32 isValid;

	bool isDebugEnabled;
} Application;

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
		emulator->states.textures[i] = AllocateTexture(mem, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	}

	emulator->isShutdown = false;
	emulator->isActive = false;
	emulator->masterVolume = 0.25f;

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
		ReleaseTexture(&emulator->states.textures[i]);

	fplClearStruct(emulator);
}

static Application *CreateApplication(fmemMemoryBlock *mem) {
	Application *app = fmemPushStruct(mem, Application, fmemPushFlags_Clear);
	if (app == fpl_null) {
		return fpl_null;
	}

	// TODO: Proper memory allocator
	MemoryAllocator *allocator = fpl_null;

	// Roms path
	fplGetExecutableFilePath(app->romsPath, fplArrayCount(app->romsPath));
	fplExtractFilePath(app->romsPath, app->romsPath, fplArrayCount(app->romsPath));
	fplPathCombine(app->romsPath, fplArrayCount(app->romsPath), 2, app->romsPath, "roms");

	// Load fonts
	float fontSizeSmall = 40.0f;
	float fontSizeLarge = 160.0f;
	if (!FontLoadFromMemory(allocator, ptr_fireCodeFont, sizeOf_fireCodeFont, 0, fontSizeSmall, 32, 126, 256, 256, false, &app->fontData)) {
		return fpl_null;
	}
	if (!FontLoadFromMemory(allocator, ptr_fireCodeFont, sizeOf_fireCodeFont, 0, fontSizeLarge, 32, 126, 1024, 1024, false, &app->fontDataLarge)) {
		return fpl_null;
	}	

	// Init console/disassembly/string memory
	size_t consoleBlockSize = fplMegaBytes(128);
	app->consoleMemory = CreatePersistentMemory(consoleBlockSize);
	app->console.values = StringListInit(&app->consoleMemory);

	size_t disassemblyBlockSize = fplMegaBytes(128);
	app->disassemblyMemory = CreatePersistentMemory(disassemblyBlockSize);
	app->disassemblyList.values = StringListInit(&app->disassemblyMemory);
	app->disassemblyHashTable = IndexHashtableInit(&app->disassemblyMemory);

	// Load/Allocate textures
	app->fontTexture = UploadTexture(app->fontData.atlasWidth, app->fontData.atlasHeight, TextureFormat_Alpha, TextureFilter_Linear, app->fontData.atlasAlphaBitmap);
	app->fontTextureLarge = UploadTexture(app->fontDataLarge.atlasWidth, app->fontDataLarge.atlasHeight, TextureFormat_Alpha, TextureFilter_Linear, app->fontDataLarge.atlasAlphaBitmap);
	app->displayTexture = AllocateTexture(mem, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->vramTexture = AllocateTexture(mem, FGB_TILEMAP_WIDTH, FGB_TILEMAP_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->backgroundMapTexture = AllocateTexture(mem, FGB_BACKGROUND_MAP_WIDTH, FGB_BACKGROUND_MAP_HEIGHT, TextureFormat_RGBA, TextureFilter_Nearest);
	app->cursorTexture = LoadTextureFromMemory(ptr_mouseCursor, sizeOf_mouseCursor, TextureFormat_Automatic, TextureFilter_Linear, 0, 0);
	app->gbTexture = LoadTextureFromMemory(ptr_gameboyImage, sizeOf_gameboyImage, TextureFormat_Automatic, TextureFilter_Linear, 619, 1024);

	// Init UI
	UIInitContext(&app->uiCtx, UITheme_Dark);

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

	// TODO: Proper memory allocator
	MemoryAllocator *allocator = fpl_null;

	Application *app = *appRef;

	ReleaseEmulator(&app->emulator);

	ReleaseTexture(&app->cursorTexture);
	ReleaseTexture(&app->vramTexture);
	ReleaseTexture(&app->displayTexture);
	ReleaseTexture(&app->backgroundMapTexture);
	ReleaseTexture(&app->fontTexture);
	ReleaseTexture(&app->fontTextureLarge);
	ReleaseTexture(&app->gbTexture);

	fmemFree(&app->disassemblyMemory);
	fmemFree(&app->consoleMemory);

	FontFree(allocator, &app->fontData);
	FontFree(allocator, &app->fontDataLarge);

	fplClearStruct(app);

	*appRef = fpl_null;
}

static void HighlightScrollDisassembly(Application *app) {
	Emulator *emu = &app->emulator;
	fgbSystem *system = &emu->system;
	uint64_t key = system->cpu.registers.pc;
	size_t index = 0;
	if (IndexHashtableGet(&app->disassemblyHashTable, key, &index)) {
		UIListboxHighlight(&app->disassemblyList, index + 1);
		UIListboxScrollTo(&app->disassemblyList, index);
	}
}

static char TextBuffer[256];

const float inv255 = 1.0f / 255.0f;

static Color4f FGBColorToLinearColor(const fgbColor color) {
	Color4f result = { color.r * inv255, color.g * inv255, color.b * inv255, 1.0f };
	return result;
}
	
static void DrawPanelLabel(const Application *app, UIContext *uiCtx, const float x, const float y, const float w, const float h, const char *text) {
	UIFont lastFont = UIGetFont(uiCtx);
	float largeFontHeight = lastFont.fontHeight * 4;
	float largeLineHeight = lastFont.lineHeight * 4;
	UISetFont(uiCtx, &app->fontDataLarge, app->fontTextureLarge.id, largeFontHeight, largeLineHeight);
	size_t labelLen = fplGetStringLength(text);
	Vec2f labelSize = UIGetStringSize(uiCtx, text, labelLen);
	float centerLabelX = x + (w - labelSize.w) * 0.5f;
	float centerLabelY = y + (h - labelSize.h) * 0.5f;
	UIString(uiCtx, centerLabelX, centerLabelY, ColorDarkGray, text, labelLen);
	UISetFont(uiCtx, lastFont.currentFont, lastFont.currentFontTextureID, lastFont.fontHeight, lastFont.lineHeight);
}

static void DrawDisplayState(Application *app, const fgbPPU *ppu, const float x, const float y, const float w, const float h, const float padding) {
	UIContext *uiCtx = &app->uiCtx;

	const float lineHeight = UIGetFontHeight(uiCtx);

	const Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	size_t maxLen = fplArrayCount(TextBuffer);

	float border = 1.0f;
	UIPanel(uiCtx, x, y, w, h, false);
	DrawPanelLabel(app, uiCtx, x, y, w, h, "PPU");

	float paddingX = padding;
	float paddingY = padding;

	float textX = x + paddingX;
	float textY = y + h - lineHeight - paddingY;

	const char *separator = ", ";

	const char *text;
	Vec2f textSize;
	Color4f bitColor;

	textX = x + paddingX;
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "LCDC: ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.w;

	// 7 = LCD Off/On
	text = (ppu->lcd.lcdc.lcdEnabled ? "On " : "Off");
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (ppu->lcd.lcdc.lcdEnabled)
		bitColor = ColorGreen;
	else
		bitColor = ColorRed;
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	// Separator
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), separator, text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.w;
#
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
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.w;

	textY -= lineHeight;

	textX = x + paddingX;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Sprites: %02u, Ticks: %04u", ppu->pipeline.sprites.count, ppu->state.lineTicks);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "LY: %02u, LYC: %02u, SCX: %02u, SCY: %02u", ppu->lcd.ly, ppu->lcd.lyc, ppu->lcd.scx, ppu->lcd.scy);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "WinX: %02u, WinY: %02u", ppu->lcd.wx, ppu->lcd.wy);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	textY -= lineHeight;

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
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "X Line/Fetch/Push/FIFO: %02u/%02u/%02u/%02u", ppu->pipeline.state.lineX, ppu->pipeline.fetch.currentX, ppu->pipeline.state.pushX, ppu->pipeline.state.fifoX);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "BG-Tile: %02u x %02u, ID: $%02X, Y-Offset: %02u", ppu->pipeline.tilePos.x, ppu->pipeline.tilePos.y, ppu->pipeline.fetch.tileID, ppu->pipeline.state.offsetY);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textY -= lineHeight * 2.0f;

	const uint8_t fifoCapacity = FGB_ARRAYCOUNT(ppu->pipeline.fifo.pixels);

	float fifoWidth = w - paddingX * 2.0f;
	float fifoHeight = lineHeight * 1.5f;
	float fifoCellWidth = fifoWidth / (float)fifoCapacity;

	float tmpX = x + paddingX;
	float tmpY = textY;

	for (int i = 0; i < ppu->pipeline.fifo.len; ++i) {
		int p = (ppu->pipeline.fifo.out + i) % fifoCapacity;

		fgbPixel fifoPixel = ppu->pipeline.fifo.pixels[p];

		Color4f color = FGBColorToLinearColor(fifoPixel.color);

		DrawFilledQuad(tmpX + i * fifoCellWidth, tmpY, fifoCellWidth, fifoHeight, color);
	}

	DrawStrokedQuad(tmpX, tmpY, fifoWidth, fifoHeight, 2.0f, ColorGray);
	for (int i = 1; i < fifoCapacity; ++i) {
		DrawLine(tmpX + i * fifoCellWidth, tmpY, tmpX + i * fifoCellWidth, tmpY + fifoHeight, 1.0f, ColorGray);
	}

	textY -= fifoHeight;

	const float switchesPanelPadding = 8.0f;

	float switchesPanelButtonY = textY - paddingY;

	tmpX = x + paddingX;

	Emulator *emu = &app->emulator;

	fgbSystem *system = &emu->system;

	bool isBackgroundChecked = system->ppu.state.isBackgroundEnabled;
	bool isBackgroundEnabled = emu->isActive;
	if (UICheckbox(uiCtx, &app->ppuBackgroundCheckbox, tmpX, switchesPanelButtonY, "Background", true, isBackgroundChecked, isBackgroundEnabled)) {
		system->ppu.state.isBackgroundEnabled = !system->ppu.state.isBackgroundEnabled;
	}

	tmpX += app->ppuBackgroundCheckbox.currentWidth + switchesPanelPadding;

	bool isWindowChecked = system->ppu.state.isWindowEnabled;
	bool isWindowEnabled = emu->isActive;
	if (UICheckbox(uiCtx, &app->ppuWindowCheckbox, tmpX, switchesPanelButtonY, "Window", true, isWindowChecked, isWindowEnabled)) {
		system->ppu.state.isWindowEnabled = !system->ppu.state.isWindowEnabled;
	}

	tmpX += app->ppuWindowCheckbox.currentWidth + switchesPanelPadding;

	bool isSpritesChecked = system->ppu.state.isSpritesEnabled;
	bool isSpritesEnabled = emu->isActive;
	if (UICheckbox(uiCtx, &app->ppuSpritesCheckbox, tmpX, switchesPanelButtonY, "Sprites", true, isSpritesChecked, isSpritesEnabled)) {
		system->ppu.state.isSpritesEnabled = !system->ppu.state.isSpritesEnabled;
	}
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

static void DrawSoundState(Application *app, fgbSystem *system, const float x, const float y, const float w, const float h, const float padding) {
	const fgbAPU *apu = &system->apu;

	UIContext *uiCtx = &app->uiCtx;

	Emulator *emulator = &app->emulator;

	const float lineHeight = UIGetFontHeight(uiCtx);

	const Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	size_t maxLen = fplArrayCount(TextBuffer);

	float border = 1.0f;
	UIPanel(uiCtx, x, y, w, h, false);
	DrawPanelLabel(app, uiCtx, x, y, w, h, "APU");

	float paddingX = padding;
	float paddingY = padding;

	float textX = x + paddingX;
	float textY = y + h - lineHeight - paddingY;

	const char *text;
	Vec2f textSize;
	Color4f bitColor;
	float checkboxWidth;

	// Sound Control
	bool soundOn = fgbIsAudioPowered(system);

	textX = x + paddingX;
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Sound: ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.w;

	text = soundOn ? "On " : "Off";
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (soundOn)
		bitColor = ColorGreen;
	else
		bitColor = ColorRed;
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	textY -= lineHeight;

	// Volume
	float masterVolume = app->emulator.masterVolume;
	uint8_t masterVolumePercentage = (uint8_t)(masterVolume * 100.0);
	uint8_t leftVolumePercentage = (uint8_t)(fgbGetAudioSpeakerVolume(system, fgbSpeakerType_Left) * 100.0);
	uint8_t rightVolumePercentage = (uint8_t)(fgbGetAudioSpeakerVolume(system, fgbSpeakerType_Right) * 100.0);

	textX = x + paddingX;
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Master Volume: %u, LR: %u %u", masterVolumePercentage, leftVolumePercentage, rightVolumePercentage);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += textSize.w;

	textY -= lineHeight;

	// Channel 1: Sweep
	fgbVoiceState channel1State = fgbGetAudioVoiceState(system, fgbVoiceType_Sweep);
	uint8_t channel1Volume = (uint8_t)(fgbGetAudioVoiceVolume(system, fgbVoiceType_Sweep) * 100.0f);
	bool isVoice1Enabled = !fgbIsAudioVoiceMuted(system, fgbVoiceType_Sweep);
	
	textX = x + paddingX;
	if (UICheckbox(uiCtx, &app->voice1MuteCheckbox, textX, textY, "Voice 1 (Sweep): ", true, isVoice1Enabled, emulator->isActive)) {
		fgbSetAudioVoiceMute(system, fgbVoiceType_Sweep, !fgbIsAudioVoiceMuted(system, fgbVoiceType_Sweep));
	}
	checkboxWidth = app->voice1MuteCheckbox.currentWidth;
	textX += checkboxWidth;

	text = voiceStateLabelMap[channel1State];
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	bitColor = voiceStateColorMap[channel1State];
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), ", Vol: %u", channel1Volume);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textY -= lineHeight;

	// Channel 2: Tone
	fgbVoiceState channel2State = fgbGetAudioVoiceState(system, fgbVoiceType_Tone);
	uint8_t channel2Volume = (uint8_t)(fgbGetAudioVoiceVolume(system, fgbVoiceType_Tone) * 100.0f);
	bool isVoice2Enabled = !fgbIsAudioVoiceMuted(system, fgbVoiceType_Tone);

	textX = x + paddingX;
	if (UICheckbox(uiCtx, &app->voice2MuteCheckbox, textX, textY, "Voice 2 (Tone):  ", true, isVoice2Enabled, emulator->isActive)) {
		fgbSetAudioVoiceMute(system, fgbVoiceType_Tone, !fgbIsAudioVoiceMuted(system, fgbVoiceType_Tone));
	}
	checkboxWidth = app->voice2MuteCheckbox.currentWidth;
	textX += checkboxWidth;

	text = voiceStateLabelMap[channel2State];
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	bitColor = voiceStateColorMap[channel2State];
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), ", Vol: %u", channel2Volume);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textY -= lineHeight;

	// Channel 3: Wave
	fgbVoiceState channel3State = fgbGetAudioVoiceState(system, fgbVoiceType_Wave);
	uint8_t channel3Volume = (uint8_t)(fgbGetAudioVoiceVolume(system, fgbVoiceType_Wave) * 100.0f);
	bool isVoice3Enabled = !fgbIsAudioVoiceMuted(system, fgbVoiceType_Wave);

	textX = x + paddingX;
	if (UICheckbox(uiCtx, &app->voice3MuteCheckbox, textX, textY, "Voice 3 (Wave):  ", true, isVoice3Enabled, emulator->isActive)) {
		fgbSetAudioVoiceMute(system, fgbVoiceType_Wave, !fgbIsAudioVoiceMuted(system, fgbVoiceType_Wave));
	}
	checkboxWidth = app->voice3MuteCheckbox.currentWidth;
	textX += checkboxWidth;

	text = voiceStateLabelMap[channel3State];
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	bitColor = voiceStateColorMap[channel3State];
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), ", Vol: %u", channel3Volume);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textY -= lineHeight;

	// Channel 4: Noise
	fgbVoiceState channel4State = fgbGetAudioVoiceState(system, fgbVoiceType_Noise);
	uint8_t channel4Volume = (uint8_t)(fgbGetAudioVoiceVolume(system, fgbVoiceType_Noise) * 100.0f);
	bool isVoice4Enabled = !fgbIsAudioVoiceMuted(system, fgbVoiceType_Noise);

	textX = x + paddingX;
	if (UICheckbox(uiCtx, &app->voice4MuteCheckbox, textX, textY, "Voice 4 (Noise): ", true, isVoice4Enabled, emulator->isActive)) {
		fgbSetAudioVoiceMute(system, fgbVoiceType_Noise, !fgbIsAudioVoiceMuted(system, fgbVoiceType_Noise));
	}
	checkboxWidth = app->voice4MuteCheckbox.currentWidth;
	textX += checkboxWidth;

	text = voiceStateLabelMap[channel4State];
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "%s", text);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	bitColor = voiceStateColorMap[channel4State];
	UIString(uiCtx, textX, textY, bitColor, TextBuffer, 0);
	textX += textSize.w;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), ", Vol: %u", channel4Volume);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textY -= lineHeight;
}

static void DrawCPUState(Application *app, const fgbEmulationState state, const fgbCPU *cpu, const float x, const float y, const float w, const float h, const float padding) {
	const fgbCPURegisters *r = &cpu->registers;

	Emulator *emulator = &app->emulator;

	UIContext *uiCtx = &app->uiCtx;

	const float lineHeight = UIGetLineHeight(uiCtx);

	UIFont lastFont = UIGetFont(uiCtx);

	UISetFont(uiCtx, &app->fontData, app->fontTexture.id, 24.0f, 1.0f);

	size_t maxLen = fplArrayCount(TextBuffer);

	const Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	float border = 1.0f;
	UIPanel(uiCtx, x, y, w, h, false);
	DrawPanelLabel(app, uiCtx, x, y, w, h, "CPU");

	float paddingX = padding;
	float paddingY = padding;

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

	float textX = x + paddingX;
	float textY = y + h - lineHeight - paddingY;

	float tmpX;

	Vec2f textSize;
	Color4f flagColor;

	if (state == fgbEmulationState_Breakpoint) {
		fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "State: %s, %s", stateText, fgbGetBreakpointTypeLabel(emulator->lastBreakpointType));
	} else {
		fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "State: %s", stateText);
	}
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "M-Cycles: %llu, T-Cycles: %llu", cpu->state.currentMemoryCycles, cpu->state.totalTickCycles);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textY -= lineHeight;

	textY -= lineHeight;

	// Flags Label
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Flags: ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	// Z Flag
	tmpX = x + textSize.w;
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "Z ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (!r->f.zeroFlag)
		flagColor = ColorRed;
	else
		flagColor = ColorGreen;
	UIString(uiCtx, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.w;

	// N Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "N ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (!r->f.negativeFlag)
		flagColor = ColorRed;
	else
		flagColor = ColorGreen;
	UIString(uiCtx, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.w;

	// H Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "H ");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (!r->f.halfCarryFlag)
		flagColor = ColorRed;
	else
		flagColor = ColorGreen;
	UIString(uiCtx, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.w;

	// C Flag
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "C");
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	if (!r->f.fullCarryFlag)
		flagColor = ColorRed;
	else
		flagColor = ColorGreen;
	UIString(uiCtx, tmpX, textY, flagColor, TextBuffer, 0);
	tmpX += textSize.w;

	textY -= lineHeight;

	textY -= lineHeight;

	// Registers

	const char *spaceForNextRegisterLabel = "AF: $%02X $%02X";
	textSize = UIGetStringSize(uiCtx, spaceForNextRegisterLabel, 0);
	float spaceToNextRegister = textSize.w;

	float startTextX = textX;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "AF: $%02X $%02X", r->a, r->f.flags);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "BC: $%02X $%02X", r->b, r->c);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textX = startTextX;
	textY -= lineHeight;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "DE: $%02X $%02X", r->d, r->e);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "HL: $%02X $%02X", r->h, r->l);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textX = startTextX;
	textY -= lineHeight;
	
	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "SP: $%04X", r->sp);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);
	textX += spaceToNextRegister;

	fplStringFormat(TextBuffer, fplArrayCount(TextBuffer), "PC: $%04X", r->pc);
	textSize = UIGetStringSize(uiCtx, TextBuffer, 0);
	UIString(uiCtx, textX, textY, foregroundColor, TextBuffer, 0);

	textX = startTextX;
	textY -= lineHeight;

	UIResetFont(uiCtx, &lastFont);
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

static void UpdateDisplayTexture(Application *app, const fgbPPU *ppu) {
	TransferPixelsToTexture(ppu->display, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, &app->displayTexture);
	UpdateTexture(&app->displayTexture);
}

static void UpdateBackgroundMapTexture(Application *app, const fgbPPU *ppu) {
	TransferPixelsToTexture(ppu->backgroundMap.colors, FGB_BACKGROUND_MAP_WIDTH, FGB_BACKGROUND_MAP_HEIGHT, &app->backgroundMapTexture);
	UpdateTexture(&app->backgroundMapTexture);
}

static void UpdateVRAMTexture(Application *app, const fgbPPU *ppu) {
	TransferPixelsToTexture(ppu->tilemap, FGB_TILEMAP_WIDTH, FGB_TILEMAP_HEIGHT, &app->vramTexture);
	UpdateTexture(&app->vramTexture);
}

static void DrawDisplay(const Application *app, const float x, const float y, const float w, const float h, const float aspect) {
	const UIContext *uiCtx = &app->uiCtx;

	const Texture *tex = &app->displayTexture;
	float uMin = 0.0f;
	float uMax = tex->uScale;
	float vMin = tex->vScale;
	float vMax = 0.0f;
	float border = 1.0f;

	float fontHeight = UIGetFontHeight(uiCtx);

	Vec2f screenSize = V2fInit(w, h);

	Viewport4f displayView = VP4fComputeByAspect(screenSize, aspect);

	float boyWidth = displayView.w;
	float boyHeight = displayView.h;
	float boyX = x + displayView.x;
	float boyY = y + displayView.y;

	if (app->isDebugEnabled) {
		UIPanel(uiCtx, x, y, w, h, false);
		boyX += border * 2.0f;
		boyY += border * 2.0f;
		boyWidth -= border * 4.0f;
		boyHeight -= border * 4.0f;
	}

	if (app->emulator.isActive) {
		DrawTexturedQuad(tex->id, boyX, boyY, boyWidth, boyHeight, ColorWhite, uMin, vMin, uMax, vMax);
	} else {
		DrawFilledQuad(x + border * 2.0f, y + border * 2.0f, w - border * 4.0f, h - border * 4.0f, ColorBlack);

		const char *insertGameText = "No Game Pak loaded";
		size_t textLen = fplGetStringLength(insertGameText);
		Vec2f textSize = FontGetTextSize(&app->fontData, insertGameText, textLen, fontHeight * 2.0f);

		DrawString(&app->fontData, app->fontTexture.id, insertGameText, textLen, x + (w - textSize.w) * 0.5f, y + (h - textSize.h) * 0.5f - fontHeight, fontHeight * 2.0f, ColorWhite);
	}
}

static void DrawBackgroundMap(UIContext *uiCtx, const Application *app, const float x, const float y, const float w, const float h) {
	const Emulator *emulator = &app->emulator;

	const Texture *tex = &app->backgroundMapTexture;
	float uMin = 0.0f;
	float uMax = tex->uScale;
	float vMin = tex->vScale;
	float vMax = 0.0f;

	UIPanel(uiCtx, x, y, w, h, true);

	const uint8_t gridCountX = 32;
	const uint8_t gridCountY = 32;

	float insideMargin = 4;
	float insideX = x + insideMargin;
	float insideY = y + insideMargin;
	float insideWidth = w - insideMargin * 2.0f;
	float insideHeight = h - insideMargin * 2.0f;

	//DrawFilledQuad(insideX, insideY, insideWidth, insideHeight, ColorWhite);

	float tileSize = insideWidth / (float)gridCountX;

	float totalTilesWidth = gridCountX * tileSize;
	float totalTilesHeight = gridCountY * tileSize;

	float tilesX = insideX;
	float tilesY = insideY + insideHeight;
	if (totalTilesHeight <= insideHeight) {
		int a = 42;
	} else {
		// Need to do some aspect ratio stuff?
		int b = 42;
	}

	float texY = insideY + insideHeight - totalTilesHeight;
	DrawTexturedQuad(tex->id, insideX, texY, insideWidth, totalTilesHeight, ColorWhite, uMin, vMin, uMax, vMax);

	Color4f gridLineColor = { 0.1f, 0.1f, 0.1f, 0.25f };
	for (uint8_t i = 0; i <= gridCountX; ++i) {
		float gridLineX0 = tilesX + i * tileSize;
		float gridLineY0 = tilesY;
		float gridLineX1 = tilesX + i * tileSize;
		float gridLineY1 = tilesY - totalTilesHeight;
		DrawLine(gridLineX0, gridLineY0, gridLineX1, gridLineY1, 1.0f, gridLineColor);
	}
	for (uint8_t i = 0; i <= gridCountY; ++i) {
		float gridLineX0 = tilesX;
		float gridLineY0 = tilesY - i * tileSize;
		float gridLineX1 = tilesX + totalTilesWidth;
		float gridLineY1 = tilesY - i * tileSize;
		DrawLine(gridLineX0, gridLineY0, gridLineX1, gridLineY1, 1.0f, gridLineColor);
	}

	float pixelsPerTile = tileSize / (float)8.0f;

	uint8_t scx = emulator->system.ppu.backgroundMap.scrollX;
	uint8_t scy = emulator->system.ppu.backgroundMap.scrollY;

	float scrollWidth = pixelsPerTile * (float)FGB_DISPLAY_WIDTH;
	float scrollHeight = pixelsPerTile * (float)FGB_DISPLAY_HEIGHT;
	float scrollOffsetX = (float)scx * pixelsPerTile;
	float scrollOffsetY = (float)scy * pixelsPerTile;
	float scrollX = scrollOffsetX;
	float scrollY = scrollOffsetY;

	const float minX = 0.0f;
	const float minY = 0.0f;
	const float maxX = totalTilesWidth;
	const float maxY = totalTilesHeight;

	bool isHorizontalWrap = (scrollX + scrollWidth) > maxX;
	bool isVerticalWrap = (scrollY + scrollHeight) > maxY;

	float xDepth = (scrollX + scrollWidth) - maxX;
	float yDepth = (scrollY + scrollHeight) - maxY;
	float xRemaining = scrollWidth - xDepth;
	float yRemaining = scrollHeight - yDepth;

	if (isHorizontalWrap && isVerticalWrap) {
		// Bottom Right
		DrawStrokedQuad(tilesX + scrollX, tilesY - scrollY - yRemaining, xRemaining, yRemaining, 2.0f, ColorRed);
		// Bottom Left
		DrawStrokedQuad(tilesX, tilesY - scrollY - yRemaining, xDepth, yRemaining, 2.0f, ColorRed);
		// Top Right
		DrawStrokedQuad(tilesX + scrollX, tilesY - yDepth, xRemaining, yDepth, 2.0f, ColorRed);
		// Top Left
		DrawStrokedQuad(tilesX, tilesY - yDepth, xDepth, yDepth, 2.0f, ColorRed);
	} else if (isHorizontalWrap && !isVerticalWrap) {
		// Right
		DrawStrokedQuad(tilesX + scrollX, tilesY - scrollY - scrollHeight, xRemaining, scrollHeight, 2.0f, ColorRed);
		// Left
		DrawStrokedQuad(tilesX, tilesY - scrollY - scrollHeight, xDepth, scrollHeight, 2.0f, ColorRed);
	} else if (isVerticalWrap && !isHorizontalWrap) {
		// Bottom
		DrawStrokedQuad(tilesX + scrollX, tilesY - scrollY - yRemaining, scrollWidth, yRemaining, 2.0f, ColorRed);
		// Top
		DrawStrokedQuad(tilesX + scrollX, tilesY - yDepth, scrollWidth, yDepth, 2.0f, ColorRed);
	} else {
		DrawStrokedQuad(tilesX + scrollX, tilesY - scrollY - scrollHeight, scrollWidth, scrollHeight, 2.0f, ColorRed);
	}
}

static void DrawBackground(const Application *app, const float x, const float y, const float w, const float h) {
	const Texture *tex = &app->gbTexture;
	float uMin = 0.0f;
	float uMax = tex->uScale;
	float vMin = tex->vScale;
	float vMax = 0.0f;
	float border = 1.0f;
	DrawFilledQuad(x, y, w, h, ColorBlack);
	DrawStrokedQuad(x + border * 0.5f, y + border * 0.5f, w - border, h - border, border, ColorWhite);
	DrawTexturedQuad(tex->id, x + border * 2.0f, y + border * 2.0f, w - border * 4.0f, h - border * 4.0f, ColorWhite, uMin, vMin, uMax, vMax);
}

static void DrawTiles(UIContext *uiCtx, const Application *app, const float x, const float y, const float w, const float h, const float aspect) {
	const Texture *tex = &app->vramTexture;
	float uMin = 0.0f;
	float uMax = tex->uScale;
	float vMin = tex->vScale;
	float vMax = 0.0f;
	float border = 1.0f;

	Vec2f size = V2fInit(w - border * 4.0f, h - border * 4.0f);

	Viewport4f vp = VP4fComputeByAspect(size, aspect);

	float rx = x + border * 2.0f + vp.x;
	float ry = y + border * 2.0f + vp.y;
	float rw = vp.w;
	float rh = vp.h;

	uint8_t gridCountX = 16;
	uint8_t gridCountY = 24;
	float tileSize = rw / (float)gridCountX;

	float totalTilesWidth = gridCountX * tileSize;
	float totalTilesHeight = gridCountY * tileSize;

	UIPanel(uiCtx, x, y, w, h, true);

	DrawTexturedQuad(tex->id, rx, ry, rw, rh, ColorWhite, uMin, vMin, uMax, vMax);

	Color4f gridLineColor = { 0.1f, 0.1f, 0.1f, 0.25f };
	for (uint8_t i = 0; i <= gridCountX; ++i) {
		float gridLineX0 = rx + i * tileSize;
		float gridLineY0 = ry;
		float gridLineX1 = rx + i * tileSize;
		float gridLineY1 = ry + totalTilesHeight;
		DrawLine(gridLineX0, gridLineY0, gridLineX1, gridLineY1, 1.0f, gridLineColor);
	}
	for (uint8_t i = 0; i <= gridCountY; ++i) {
		float gridLineX0 = rx;
		float gridLineY0 = ry + i * tileSize;
		float gridLineX1 = rx + totalTilesWidth;
		float gridLineY1 = ry + i * tileSize;
		DrawLine(gridLineX0, gridLineY0, gridLineX1, gridLineY1, 1.0f, gridLineColor);
	}
}

static void DrawBreakpoints(Application *app, const float x, const float y, const float w, const float h) {
	UIContext *uiCtx = &app->uiCtx;

	Emulator *emulator = &app->emulator;

	fgbSystem *system = &emulator->system;

	const float charHeight = UIGetFontHeight(uiCtx);
	const float lineHeight = UIGetLineHeight(uiCtx);

	const float checkboxHeight = lineHeight * 1.4f;

	Breakpoints *bp = &app->breakpoints;

	float currentX = x;
	float currentY = y + h - lineHeight;

	const uint32_t checkboxCount = fplArrayCount(emulator->config.debug.breakpoints.filter);

	// Struct sizes and field alignment is very important!!!
	const size_t checkboxDataSize = sizeof(UICheckboxData);

	fplAssert(fplArrayCount(bp->checkboxes) == checkboxCount);

	for (uint32_t checkboxIndex = 0; checkboxIndex < checkboxCount; ++checkboxIndex) {
		fgbBreakpointType type = fgbBreakpointType_First + checkboxIndex;
		bool isChecked = emulator->config.debug.breakpoints.filter[type];
		const char *label = fgbGetBreakpointTypeLabel(type);
		UICheckboxData *checkbox = &bp->checkboxes[checkboxIndex];
		if (UICheckbox(uiCtx, checkbox, currentX, currentY, label, true, isChecked, emulator->isActive)) {
			bool isEnabled = (emulator->config.debug.breakpoints.filter[type] = !emulator->config.debug.breakpoints.filter[type]);
			fgbBreakpointEnable(system, type, isEnabled);
		}
		currentY -= checkboxHeight;
	}
}

static void DrawPalette(const float x, const float y, const float cellWidth, const float cellHeight, const Color4f *colors, const uint8_t colorCount) {
	float totalWidth = cellWidth * colorCount;
	float totalHeight = cellHeight;

	float border = 1.0f;

	float colW = cellWidth - border * 2.0f;
	float colH = cellHeight - border * 2.0f;

	DrawStrokedQuad(x + border * 0.5f, y + border * 0.5f, totalWidth - border, totalHeight - border, 1.0f, ColorGray);

	for (uint8_t colorIndex = 1; colorIndex < colorCount; ++colorIndex) {
		DrawLine(x + colorIndex * cellWidth, y, x + colorIndex * cellWidth, y + cellHeight, 1.0f, ColorGray);
	}
	for (uint8_t colorIndex = 0; colorIndex < colorCount; ++colorIndex) {
		float colX = x + colorIndex * cellWidth + border;
		float colY = y + border;
		DrawFilledQuad(colX, colY, colW, colH, colors[colorIndex]);
	}
}

static void DrawPalettes(Application *app, const float x, const float y, const float w, const float h) {
	UIContext *uiCtx = &app->uiCtx;

	Emulator *emulator = &app->emulator;

	fgbSystem *system = &emulator->system;

	const float charHeight = UIGetFontHeight(uiCtx);
	const float lineHeight = UIGetLineHeight(uiCtx);

	Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	float paletteHeight = lineHeight * 1.5f;

	float cellWidth = w / 18.0f;
	float cellHeight = paletteHeight;

	float checkboxSpacing = 10.0f;

	float paletteTypeSpacing = 10.0f;

	float spacing = lineHeight * 0.5f;

	float px = x;
	float py = y + h - lineHeight * 2.0f;

	Vec2f maxLabelSize = UIGetStringSize(uiCtx, "Palette: ", 5);

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

	// Palette Label
	text = "Palette: ";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = py + (cellHeight - maxLabelSize.h) * 0.5f - lineHeight * 0.25f;
	UIString(uiCtx, textX, textY, foregroundColor, text, textLen);

	// Checkboxes
	float checkboxX = px + maxLabelSize.w;
	float checkboxY = py;

	bool isDMGPaletteEnabled = emulator->isActive;
	bool isDMGPaletteChecked = emulator->paletteType == ColorPaletteType_DMG;
	if (UICheckbox(uiCtx, &app->dmgPaletteCheckbox, checkboxX, checkboxY, "DMG", true, isDMGPaletteChecked, isDMGPaletteEnabled)) {
		emulator->paletteType = ColorPaletteType_DMG;
		fgbSetColorPalette(system, &FGB_DEFAULT_DMG_COLORS);
	}
	checkboxX += app->dmgPaletteCheckbox.currentWidth + checkboxSpacing;

	bool isMGBPaletteEnabled = emulator->isActive;
	bool isMGBPaletteChecked = emulator->paletteType == ColorPaletteType_MGB;
	if (UICheckbox(uiCtx, &app->mgbPaletteCheckbox, checkboxX, checkboxY, "MGB", true, isMGBPaletteChecked, isMGBPaletteEnabled)) {
		emulator->paletteType = ColorPaletteType_MGB;
		fgbSetColorPalette(system, &FGB_DEFAULT_MGB_COLORS);
	}
	checkboxX += app->mgbPaletteCheckbox.currentWidth + checkboxSpacing;

	bool isSGBPaletteEnabled = emulator->isActive;
	bool isSGBPaletteChecked = emulator->paletteType == ColorPaletteType_SGB;
	if (UICheckbox(uiCtx, &app->sgbPaletteCheckbox, checkboxX, checkboxY, "SGB", true, isSGBPaletteChecked, isSGBPaletteEnabled)) {
		emulator->paletteType = ColorPaletteType_SGB;
		fgbSetColorPalette(system, &FGB_DEFAULT_SGB_COLORS);
	}
	checkboxX += app->sgbPaletteCheckbox.currentWidth + checkboxSpacing;

	bool isBluePaletteEnabled = emulator->isActive;
	bool isBluePaletteChecked = emulator->paletteType == ColorPaletteType_Blue;
	if (UICheckbox(uiCtx, &app->bluePaletteCheckbox, checkboxX, checkboxY, "Blue", true, isBluePaletteChecked, isBluePaletteEnabled)) {
		emulator->paletteType = ColorPaletteType_Blue;
		fgbSetColorPalette(system, &BlueMonochromeColors);
	}
	checkboxX += app->bluePaletteCheckbox.currentWidth + checkboxSpacing;

	py -= (paletteHeight + spacing);

	// Active Palette
	palX = px + maxLabelSize.w;
	palY = py;
	palWidth = cellWidth * 2;
	DrawPalette(palX, palY, cellWidth, cellHeight, paletteColorsSys, 2);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(palX, palY, cellWidth, cellHeight, paletteColorsBg, 4);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(palX, palY, cellWidth, cellHeight, paletteColorsObj0, 4);
	palX += palWidth + paletteTypeSpacing;

	palWidth = cellWidth * 4;
	DrawPalette(palX, palY, cellWidth, cellHeight, paletteColorsObj1, 4);
	palX += palWidth + paletteTypeSpacing;

	py -= paletteHeight + spacing;
	py -= paletteHeight + spacing;

	// System Palette
	palX = px + maxLabelSize.w;
	palY = py;
	text = "Sys";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.h) * 0.5f - lineHeight * 0.25f;
	UIString(uiCtx, textX, textY, foregroundColor, text, textLen);
	DrawPalette(palX, palY, cellWidth, cellHeight, sysColors, 2);

	py -= (paletteHeight + spacing);

	// Background Palette
	palX = px + maxLabelSize.w;
	palY = py;
	text = "BG";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.h) * 0.5f - lineHeight * 0.25f;
	UIString(uiCtx, textX, textY, foregroundColor, text, textLen);
	DrawPalette(palX, palY, cellWidth, cellHeight, bgColors, 4);

	py -= (paletteHeight + spacing);

	// Obj0 Palette
	palX = px + maxLabelSize.w;
	palY = py;
	text = "OBJ-0";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.h) * 0.5f - lineHeight * 0.25f;
	UIString(uiCtx, textX, textY, foregroundColor, text, textLen);
	DrawPalette(palX, palY, cellWidth, cellHeight, obj0Colors, 4);

	py -= (paletteHeight + spacing);

	// Obj1 Palette
	palX = px + maxLabelSize.w;
	palY = py;
	text = "OBJ-1";
	textLen = fplGetStringLength(text);
	textX = px;
	textY = palY + (cellHeight - maxLabelSize.h) * 0.5f - lineHeight * 0.25f;
	UIString(uiCtx, textX, textY, foregroundColor, text, textLen);
	DrawPalette(palX, palY, cellWidth, cellHeight, obj1Colors, 4);
}

static char performanceLabelBuffer[1024] = { 0 };

static void DrawPerformanceCounter(UIContext *uiCtx, const float x, const float y, const char *name, const Color4f foregroundColor, const PerformanceCounter *counter) {
	double avgTimeMs = GetPerformanceCounterAvg(counter) * 1000.0;
	fplStringFormat(performanceLabelBuffer, fplArrayCount(performanceLabelBuffer), "%s: %.5f / %.5f / %.5f ms [%zu]", name, counter->minSecs * 1000.0, counter->maxSecs * 1000.0, avgTimeMs, counter->count);

	size_t textLen = fplGetStringLength(performanceLabelBuffer);
	UIString(uiCtx, x, y, foregroundColor, performanceLabelBuffer, textLen);
}

static void PauseGameboy(Application *app, fgbSystem *system) {
	fgbPause(system);
}

static void ResumeGameboy(Application *app, fgbSystem *system) {
	Emulator *emulator = &app->emulator;
	fgbResume(system);
	emulator->isFrameStepActive = false;
	emulator->isMicroStepActive = false;
	HighlightScrollDisassembly(app);
	WakeupEmulatorThread(emulator);
}

static void DrawPerformanceMetrics(UIContext *uiCtx, const Application *app, const float x, const float y, const float w, const float h) {
	const float charHeight = UIGetFontHeight(uiCtx);
	const float lineHeight = UIGetLineHeight(uiCtx);

	const PerformanceMetrics *metrics = &app->emulator.performanceMetrics;

	Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	UIPanel(uiCtx, x, y, w, h, false);

	float padding = 8.0f;

	float textX = x + padding;
	float textY = y + h - lineHeight - padding;

	DrawPerformanceCounter(uiCtx, textX, textY, "Frame", foregroundColor, &metrics->frameTime);
	textY -= lineHeight;
	DrawPerformanceCounter(uiCtx, textX, textY, "Textures Upload", foregroundColor, &metrics->texturesUpload);
	textY -= lineHeight;
	DrawPerformanceCounter(uiCtx, textX, textY, "Audio Thread", foregroundColor, &metrics->audioThread);
	textY -= lineHeight;
	DrawPerformanceCounter(uiCtx, textX, textY, "Audio Read", foregroundColor, &metrics->audioReadSamples);
	textY -= lineHeight;
	DrawPerformanceCounter(uiCtx, textX, textY, "Audio Output", foregroundColor, &metrics->audioOutputSamples);
	textY -= lineHeight;
	DrawPerformanceCounter(uiCtx, textX, textY, "Emulator Tick", foregroundColor, &metrics->emulatorTick);
	textY -= lineHeight;
}

static char dateTimeFormatBuffer[32];

static void RenderDebugFrame(Application *app, const InputState *input) {
	const LoadedFont *font = &app->fontData;

	TextureID fontTextureId = app->fontTexture.id;

	UIContext *uiCtx = &app->uiCtx;

	const float charHeight = UIGetFontHeight(uiCtx);
	const float lineHeight = UIGetLineHeight(uiCtx);

	static char tmpText[256];

	const float w = (float)app->viewport.w;
	const float h = (float)app->viewport.h;

	const float borderThickness = 1.5f;

	const float leftSideWidth = fplMax(w * 0.325f, 300);
	const float leftSideHeight = h;

	const float rightSideWidth = fplMax(w * 0.35f, 300);
	const float rightSideHeight = h;

	const uint8_t actionAreaButtonCount = 5;
	const float actionsAreaPadding = 1.0f;
	const float actionsAreaButtonSpacing = 2.0f;
	const float actionsAreaHeight = 1.5f * lineHeight;
	const float actionsAreaWidth = rightSideWidth;
	const float actionsAreaX = w - rightSideWidth;
	const float actionsAreaY = h - actionsAreaHeight;
	const float actionButtonWidth = ((actionsAreaWidth - (actionsAreaPadding * 2.0f) - actionsAreaButtonSpacing * (float)(actionAreaButtonCount - 1)) / (float)actionAreaButtonCount);
	const float actionButtonHeight = actionsAreaHeight - (actionsAreaPadding * 2.0f);

	const float cpuStatePadding = 4.0f;
	const float cpuStateLineCount = 8.0f;
	const float cpuStateWidth = rightSideWidth;
	const float cpuStateHeight = cpuStateLineCount * lineHeight + cpuStatePadding * 2.0f;
	const float cpuStateX = w - cpuStateWidth;
	const float cpuStateY = h - actionsAreaHeight - cpuStateHeight;

	const float switchesPanelPadding = 8.0f;
	const float switchesPanelWidth = rightSideWidth;
	const float switchesPanelHeight = lineHeight + switchesPanelPadding * 2.0f + borderThickness * 2.0f;
	const float switchesPanelX = w - switchesPanelWidth;
	const float switchesPanelY = h - actionsAreaHeight - cpuStateHeight - switchesPanelHeight;

	const float switchesPanelButtonY = switchesPanelY + switchesPanelPadding;

	const float vramAspect = FGB_TILEMAP_WIDTH / (float)FGB_TILEMAP_HEIGHT;

	const float leftTabControlWidth = leftSideWidth;
	const float leftTabControlHeight = h;
	const float leftTabControlX = 0;
	const float leftTabControlY = 0;

	const float rightTabControlWidth = rightSideWidth;
	const float rightTabControlHeight = h - (actionsAreaHeight + cpuStateHeight + switchesPanelHeight);
	const float rightTabControlX = w - rightSideWidth;
	const float rightTabControlY = 0;

	const float middleWidth = w - (leftSideWidth + rightSideWidth);

	const int cartInfoLineCount = 3;
	const float cartInfoPadding = 6.0f;
	const float cartInfoWidth = middleWidth;
	const float cartInfoHeight = cartInfoLineCount * lineHeight + cartInfoPadding * 2.0f;
	const float cartInfoX = leftSideWidth;
	const float cartInfoY = h - cartInfoHeight;

	const float displayStatePadding = 4.0f;
	const float displayStateLineCount = 12.0f;
	const float displayStateWidth = middleWidth;
	const float displayStateHeight = displayStateLineCount * lineHeight + displayStatePadding * 2.0f;
	const float displayStateX = leftSideWidth;
	const float displayStateY = h - cartInfoHeight - displayStateHeight;

	const float soundStatePadding = 4.0f;
	const float soundStateLineCount = 6.0f;
	const float soundStateWidth = middleWidth;
	const float soundStateHeight = soundStateLineCount * lineHeight + soundStatePadding * 2.0f;
	const float soundStateX = leftSideWidth;
	const float soundStateY = h - cartInfoHeight - displayStateHeight - soundStateHeight;

	const uint8_t userButtonCount = 2;
	const float userButtonSpacing = 2.0f;
	const float userButtonsPadding = 4.0f;
	const float userButtonsWidth = middleWidth;
	const float userButtonsHeight = 1.5f * lineHeight;
	const float userButtonsX = leftSideWidth;
	const float userButtonsY = 0;
	const float userButtonWidth = ((userButtonsWidth - (userButtonsPadding * 2.0f) - userButtonSpacing * (float)(userButtonCount - 1)) / (float)userButtonCount);
	const float userButtonHeight = userButtonsHeight - (userButtonsPadding * 2.0f);

	const float boyAspect = FGB_DISPLAY_WIDTH / (float)FGB_DISPLAY_HEIGHT;
	const float boyWidth = middleWidth;
	const float boyHeight = h - (cartInfoHeight + displayStateHeight + soundStateHeight + userButtonsHeight);
	const float boyX = leftSideWidth;
	const float boyY = userButtonsHeight;

	SetViewport(app->viewport.x, app->viewport.y, app->viewport.w, app->viewport.h);

	Clear(0.1f, 0.3f, 0.7f, 1.0f);

	SetModelViewProjectionMatrix(&app->viewProjectionMat.m[0]);

	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;
	fgbGamePak *gamePak = &system->gamePak;

	float tmpX;
	float tmpY;

	Color4f foregroundColor = UIGetForegroundColor(uiCtx);

	UIBegin(uiCtx);

	//
	// GamePak info
	//
	UIPanel(uiCtx, cartInfoX, cartInfoY, cartInfoWidth, cartInfoHeight, false);
	DrawPanelLabel(app, uiCtx, cartInfoX, cartInfoY, cartInfoWidth, cartInfoHeight, "Game Pak");

	const char *gamePakTitle = gamePak->isValid ? gamePak->info.title.text : "[Unloaded]";
	const char *gamePakTypeName = fgbGetGamePakTypeName(gamePak->info.gamePakType);
	const char *coreName = fgbGetCoreTypeName(gamePak->info.coreType);

	tmpX = cartInfoX + cartInfoPadding;
	tmpY = cartInfoY + cartInfoHeight - lineHeight - cartInfoPadding;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "GamePak: %s", gamePakTitle);
	UIString(uiCtx, tmpX, tmpY, foregroundColor, tmpText, 0);
	tmpY -= lineHeight;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "Core: %s Type: %s", coreName, gamePakTypeName);
	UIString(uiCtx, tmpX, tmpY, foregroundColor, tmpText, 0);
	tmpY -= lineHeight;

	fplStringFormat(tmpText, fplArrayCount(tmpText), "ROM/RAM Banks: %u/%u [%zu bytes]", gamePak->info.romBankCount, gamePak->info.ramBankCount, gamePak->rom.length);
	UIString(uiCtx, tmpX, tmpY, foregroundColor, tmpText, 0);
	tmpY -= lineHeight;

	//
	// Display Registers
	//
	DrawDisplayState(app, &system->ppu, displayStateX, displayStateY, displayStateWidth, displayStateHeight, displayStatePadding);

	//
	// Sound Registers
	//
	DrawSoundState(app, system, soundStateX, soundStateY, soundStateWidth, soundStateHeight, soundStatePadding);

	//
	// CPU
	//
	DrawCPUState(app, system->state, &system->cpu, cpuStateX, cpuStateY, cpuStateWidth, cpuStateHeight, cpuStatePadding);

	//
	// Actions
	//
	UIPanel(uiCtx, actionsAreaX, actionsAreaY, actionsAreaWidth, actionsAreaHeight, false);

	const char *pauseOrResumeButtonName = system->state == fgbEmulationState_Running ? "Pause" : "Resume";
	const char *frameStepButtonName = "Frame Step";
	const char *singleStepButtonName = "Single Step";
	const char *microStepButtonName = "Micro Step";
	const char *resetButtonName = "Reset";

	size_t pauseOrResumeButtonNameLen = fplGetStringLength(pauseOrResumeButtonName);
	size_t frameStepButtonNameLen = fplGetStringLength(frameStepButtonName);
	size_t singleStepButtonNameLen = fplGetStringLength(singleStepButtonName);
	size_t microStepButtonNameLen = fplGetStringLength(microStepButtonName);
	size_t resetButtonNameLen = fplGetStringLength(resetButtonName);

	tmpX = actionsAreaX + actionsAreaPadding;

	bool pauseOrResumeEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (UIButton(uiCtx, &app->pauseOrResumeButton, tmpX, actionsAreaY + actionsAreaPadding, actionButtonWidth, actionButtonHeight, pauseOrResumeButtonName, pauseOrResumeButtonNameLen, pauseOrResumeEnabled)) {
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
	tmpX += actionButtonWidth + actionsAreaButtonSpacing;

	bool frameStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error && !emulator->isFrameStepActive;
	if (UIButton(uiCtx, &app->frameStepButton, tmpX, actionsAreaY + actionsAreaPadding, actionButtonWidth, actionButtonHeight, frameStepButtonName, frameStepButtonNameLen, frameStepEnabled)) {
		emulator->isFrameStepActive = true;
		emulator->isMicroStepActive = false;
		fgbResume(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += actionButtonWidth + actionsAreaButtonSpacing;

	bool singleStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (UIButton(uiCtx, &app->singleStepButton, tmpX, actionsAreaY + actionsAreaPadding, actionButtonWidth, actionButtonHeight, singleStepButtonName, singleStepButtonNameLen, singleStepEnabled)) {
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = false;
		fgbStep(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += actionButtonWidth + actionsAreaButtonSpacing;

	bool microStepEnabled = emulator->isActive && system->state != fgbEmulationState_Error;
	if (UIButton(uiCtx, &app->microStepButton, tmpX, actionsAreaY + actionsAreaPadding, actionButtonWidth, actionButtonHeight, microStepButtonName, microStepButtonNameLen, microStepEnabled)) {
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = true;
		fgbMicroStep(system);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += actionButtonWidth + actionsAreaButtonSpacing;

	bool resetEnabled = emulator->isActive;
	if (UIButton(uiCtx, &app->resetButton, tmpX, actionsAreaY + actionsAreaPadding, actionButtonWidth, actionButtonHeight, resetButtonName, resetButtonNameLen, resetEnabled)) {
		StringListClear(&app->console.values);
		emulator->isFrameStepActive = false;
		emulator->isMicroStepActive = false;
		fgbReset(system, app->emulator.config.paused);
		HighlightScrollDisassembly(app);
		WakeupEmulatorThread(emulator);
	}
	tmpX += actionButtonWidth + actionsAreaButtonSpacing;

	//
	// Switches
	//
	UIPanel(uiCtx, switchesPanelX, switchesPanelY, switchesPanelWidth, switchesPanelHeight, false);

	tmpX = switchesPanelX + switchesPanelPadding;

	bool isLoggingChecked = system->log.isEnabled;
	bool isLoggingEnabled = true;
	if (UICheckbox(uiCtx, &app->logEnabledCheckbox, tmpX, switchesPanelButtonY, "Log", true, isLoggingChecked, isLoggingEnabled)) {
		emulator->config.log.isEnabled = !emulator->config.log.isEnabled;
		if (emulator->isActive) {
			system->log.isEnabled = emulator->config.log.isEnabled;
		}
	}

	tmpX += app->logEnabledCheckbox.currentWidth + switchesPanelPadding;

	bool isBootChecked = system->boot.rom.isEnabled;
	bool isBootEnabled = true;
	if (UICheckbox(uiCtx, &app->bootEnabledCheckbox, tmpX, switchesPanelButtonY, "Boot", true, isBootChecked, isBootEnabled)) {
		emulator->config.bootROM.isEnabled = !emulator->config.bootROM.isEnabled;
		if (emulator->isActive) {
			system->boot.rom.isEnabled = emulator->config.bootROM.isEnabled;
		}
	}

	tmpX += app->bootEnabledCheckbox.currentWidth + switchesPanelPadding;

	bool isInitPauseChecked = app->emulator.config.paused;
	bool isInitPauseEnabled = true;
	if (UICheckbox(uiCtx, &app->initPauseCheckbox, tmpX, switchesPanelButtonY, "IR-Pause", true, isInitPauseChecked, isInitPauseEnabled)) {
		app->emulator.config.paused = !app->emulator.config.paused;
	}

	DrawDisplay(app, boyX, boyY, boyWidth, boyHeight, boyAspect);

	//
	// Right Tab Control
	//
	const char *tabTilesId = "Tiles";
	const char *tabPalettesId = "Palettes";
	const char *tabBreakpointsId = "Breakpoints";

	UITab tabTiles = UICreateTab(UIPtrToID(tabTilesId), tabTilesId);
	UITab tabPalettes = UICreateTab(UIPtrToID(tabPalettesId), tabPalettesId);
	UITab tabBreakpoints = UICreateTab(UIPtrToID(tabBreakpointsId), tabBreakpointsId);

	const UITab *rightTabs[] = {
		&tabTiles,
		&tabPalettes,
		&tabBreakpoints,
	};
	uint8_t rightTabCount = fplArrayCount(rightTabs);
	if (app->rightTabControl.activeTab == NULL) {
		app->rightTabControl.activeTab = &tabTiles;
	}
	UITabContent rightTabContent = UITabControl(uiCtx, &app->rightTabControl, rightTabControlX, rightTabControlY, rightTabControlWidth, rightTabControlHeight, "Right-TabControl", rightTabs, rightTabCount);
	if (rightTabContent.activeTab == &tabTiles) {
		DrawTiles(uiCtx, app, rightTabContent.area.x, rightTabContent.area.y, rightTabContent.area.w, rightTabContent.area.h, vramAspect);
	} else if (rightTabContent.activeTab == &tabPalettes) {
		DrawPalettes(app, rightTabContent.area.x, rightTabContent.area.y, rightTabContent.area.w, rightTabContent.area.h);
	} else if (rightTabContent.activeTab == &tabBreakpoints) {
		DrawBreakpoints(app, rightTabContent.area.x, rightTabContent.area.y, rightTabContent.area.w, rightTabContent.area.h);
	}

	//
	// Main Tab Control
	//
	const char *logTabID = "Log";
	const char *performanceTabID = "Performance";
	const char *disassemblyTabID = "Disassembly";
	const char *backgroundMapTabID = "BG-Map";

	UITab logTab = UICreateTab(UIPtrToID(logTabID), logTabID);
	UITab performanceTab = UICreateTab(UIPtrToID(performanceTabID), performanceTabID);
	UITab disassemblyTab = UICreateTab(UIPtrToID(disassemblyTabID), disassemblyTabID);
	UITab backgroundMapTab = UICreateTab(UIPtrToID(backgroundMapTabID), backgroundMapTabID);

	const UITab *leftTabs[] = {
		&logTab,
		&performanceTab,
		&disassemblyTab,
		&backgroundMapTab,
	};
	uint8_t leftTabCount = fplArrayCount(leftTabs);
	if (!app->emulator.isActive) {
		leftTabCount = 2;
	}

	//
	// Tab Control
	//
	if (app->leftTabControl.activeTab != NULL && !emulator->isActive) {
		if (!(app->leftTabControl.activeTab == &logTab || app->leftTabControl.activeTab == &performanceTab)) {
			app->leftTabControl.activeTab = NULL;
		}
	}
	if (app->leftTabControl.activeTab == NULL) {
		app->leftTabControl.activeTab = &logTab;
	}
	UITabContent mainTabContent = UITabControl(uiCtx, &app->leftTabControl, leftTabControlX, leftTabControlY, leftTabControlWidth, leftTabControlHeight, "Left-TabControl", leftTabs, leftTabCount);

	//
	// Console
	//
	const float tabContentWidth = mainTabContent.area.w;
	const float tabContentHeight = mainTabContent.area.h;
	const float tabContentX = mainTabContent.area.x;
	const float tabContentY = mainTabContent.area.y;
	if (mainTabContent.activeTab == &logTab) {
		UIListbox(uiCtx, &app->console, tabContentX, tabContentY, tabContentWidth, tabContentHeight, "Console");
	} else if (mainTabContent.activeTab == &performanceTab) {
		DrawPerformanceMetrics(uiCtx, app, tabContentX, tabContentY, tabContentWidth, tabContentHeight);
	} else if (mainTabContent.activeTab == &disassemblyTab) {
		UIListbox(uiCtx, &app->disassemblyList, tabContentX, tabContentY, tabContentWidth, tabContentHeight, "Disassembly");
	} else if (mainTabContent.activeTab == &backgroundMapTab) {
		DrawBackgroundMap(uiCtx, app, tabContentX, tabContentY, tabContentWidth, tabContentHeight);
	}

	//
	// User Buttons
	//
	UIPanel(uiCtx, userButtonsX, userButtonsY, userButtonsWidth, userButtonsHeight, false);

	//
	// States Dialog
	//
	StatesDialog *statesDlg = &app->statesDialog;
	const char *saveStateButtonName = "Save State";
	const char *restoreStateButtonName = "Restore State";

	size_t saveStateButtonNameLen = fplGetStringLength(saveStateButtonName);
	size_t restoreStateButtonNameLen = fplGetStringLength(restoreStateButtonName);

	tmpX = userButtonsX + userButtonsPadding;

	bool saveStateButtonEnabled = emulator->isActive && fgbAreSnapshotsSupported(system);
	if (UIButton(uiCtx, &app->saveStateButton, tmpX, userButtonsY + userButtonsPadding, userButtonWidth, userButtonHeight, saveStateButtonName, saveStateButtonNameLen, saveStateButtonEnabled)) {
		fgbPause(system);
		statesDlg->type = DialogType_SaveState;
		statesDlg->dialog.isShown = true;
	}
	tmpX += userButtonWidth + userButtonSpacing;

	bool restoreStateButtonEnabled = emulator->isActive && fgbAreSnapshotsSupported(system);
	if (UIButton(uiCtx, &app->restoreStateButton, tmpX, userButtonsY + userButtonsPadding, userButtonWidth, userButtonHeight, restoreStateButtonName, restoreStateButtonNameLen, restoreStateButtonEnabled)) {
		fgbPause(system);
		statesDlg->type = DialogType_RestoreState;
		statesDlg->dialog.isShown = true;
	}
	tmpX += userButtonWidth + userButtonSpacing;

	//
	// States Dialog
	//
	const uint32_t statesDialogNumSlots = MAX_STATE_SLOT_COUNT;

	const float statesDialogWidth = w * 0.75f;
	const float statesDialogHeight = h * 0.75f;
	const float statesDialogMargin = 40.0f;

	const float statesDialogGridLabelHeight = 40;
	const float statesDialogGridLabelMargin = 5;

	const float statesDialogGridColumnSpacing = 20.0f;
	const float statesDialogGridRowSpacing = 20.0f;
	const uint32_t statesDialogGridNumRows = 2;
	const uint32_t statesDialogGridNumColumns = statesDialogNumSlots / statesDialogGridNumRows;

	const float statesDialogCloseButtonPadding = 5;
	const float statesDialogCloseButtonWidth = 30;
	const float statesDialogCloseButtonHeight = 30;

	const float statesDialogTitleMargin = 10;

	UIFont lastFont = UIGetFont(uiCtx);
	float lastFontHeight = UIGetFontHeight(uiCtx);

	const Color4f gameLabelBackground = { 0.0f, 0.0f, 0.0f, 0.5f };

	if (UIBeginDialog(uiCtx, &app->statesDialog.dialog, statesDialogWidth, statesDialogHeight, "States-Dialog")) {

		const float statesDialogContentWidth = statesDlg->dialog.window.size.w - statesDialogMargin * 2.0f;
		const float statesDialogContentHeight = statesDlg->dialog.window.size.h - statesDialogMargin * 2.0f;
		const float statesDialogContentX = statesDlg->dialog.window.pos.x + statesDialogMargin;
		const float statesDialogContentY = statesDlg->dialog.window.pos.y + statesDialogMargin;

		const float statesGridCellWidth = (statesDialogContentWidth - statesDialogGridColumnSpacing * (statesDialogGridNumColumns - 1)) / (float)statesDialogGridNumColumns;
		const float statesGridCellHeight = (statesDialogContentHeight - statesDialogGridRowSpacing * (statesDialogGridNumRows - 1)) / (float)statesDialogGridNumRows;

		Color4f labelColor0 = { 0.0f, 0.0f, 0.0f, 1.0f };
		Color4f labelColor1 = { 1.0f, 1.0f, 1.0f, 1.0f };
		Color4f titleColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		Vec2f screenSize = V2fInit(statesGridCellWidth, statesGridCellHeight);

		const Texture *firstTexture = &app->displayTexture;

		float stateTextureAspect = firstTexture->width / (float)firstTexture->height;

		Viewport4f stateTextureView = VP4fComputeByAspect(screenSize, stateTextureAspect);

		// Draw grid
		float gridY = statesDialogContentY + statesDialogContentHeight - statesGridCellHeight;
		for (uint32_t row = 0; row < statesDialogGridNumRows; ++row) {
			float gridX = statesDialogContentX;
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

				float uMin = 0.0f;
				float uMax = texture->uScale;
				float vMin = texture->vScale;
				float vMax = 0.0f;

				float textureX = gridX + stateTextureView.x;
				float textureY = gridY + stateTextureView.y;
				float textureW = stateTextureView.w;
				float textureH = stateTextureView.h;

				UIPanel(uiCtx, gridX, gridY, statesGridCellWidth, statesGridCellHeight, isSlotSelected);

				float textureAlpha = isSlotSelected ? 1.0f : 0.5f;
				Color4f textureColor = { 1.0f, 1.0f, 1.0f, textureAlpha };
				DrawTexturedQuad(texture->id, textureX, textureY, textureW, textureH, textureColor, uMin, vMin, uMax, vMax);

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
				DrawFilledQuad(gridX + 2.0f, gridY + 2.0f, statesGridCellWidth - 4.0f, labelHeight, gameLabelBackground);

				const char *label = labelBuffer;
				size_t labelLen = fplGetStringLength(label);
				UIString(uiCtx, gridX + statesDialogGridLabelMargin, gridY + statesDialogGridLabelMargin, labelColor0, label, labelLen);
				UIString(uiCtx, gridX + statesDialogGridLabelMargin + 2, gridY + statesDialogGridLabelMargin + 2, labelColor1, label, labelLen);

				gridX += statesGridCellWidth + statesDialogGridColumnSpacing;
			}
			gridY -= statesGridCellHeight;
			gridY -= statesDialogGridRowSpacing;
		}

		// Handle keyboard/controller input
		if (statesDlg->dialog.isShown && input->activeControllerIndex >= 0) {
			// "Start" was pressed?
			const ControllerInput *controller = &input->controllers[input->activeControllerIndex];
			if (UIWasPressed(&controller->start)) {
				int slotIndex = statesDlg->selectedSlotPos.row * statesDialogGridNumColumns + statesDlg->selectedSlotPos.column;

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
				}

				// Close dialog
				statesDlg->type = DialogType_None;
				statesDlg->dialog.isShown = false;
				ResumeGameboy(app, system);
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

		// Title
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
		size_t titleLen = fplGetStringLength(title);
		UISetFont(uiCtx, &app->fontData, app->fontTexture.id, 24.0f, 1.0f);
		Vec2f titleSize = UIGetStringSize(uiCtx, title, titleLen);
		float tempX = statesDlg->dialog.window.pos.x + (statesDlg->dialog.window.size.w - titleSize.w) * 0.5f;
		float tempY = statesDlg->dialog.window.pos.y + statesDlg->dialog.window.size.h - titleSize.h - statesDialogTitleMargin;
		UIString(uiCtx, tempX, tempY, titleColor, title, titleLen);
		UIResetFont(uiCtx, &lastFont);

		// Close button
		tempX = statesDlg->dialog.window.pos.x + statesDlg->dialog.window.size.w - statesDialogCloseButtonWidth - statesDialogCloseButtonPadding;
		tempY = statesDlg->dialog.window.pos.y + statesDlg->dialog.window.size.h - statesDialogCloseButtonHeight - statesDialogCloseButtonPadding;
		if (UIButton(uiCtx, &statesDlg->closeButton, tempX, tempY, statesDialogCloseButtonWidth, statesDialogCloseButtonHeight, "x", 1, true)) {
			statesDlg->type = DialogType_None;
			statesDlg->dialog.isShown = false;
			ResumeGameboy(app, system);
		}

		UIEndDialog(uiCtx, &app->statesDialog.dialog);
	}

	UIEnd(uiCtx);

#if !NO_CURSOR
	//
	// Button
	//
	float mouseCursorWidth = 32.0f;
	float mouseCursorHeight = 32.0f;
	Vec2f mousePos = input->mouse.worldPos;
	DrawTexturedQuad(app->cursorTexture.id, mousePos.x, mousePos.y - mouseCursorHeight, mouseCursorWidth, mouseCursorHeight, ColorWhite, 0.0f, 0.0f, 1.0f, 1.0f);
#endif
}

static void RenderGameFrame(Application *app, const InputState *input) {
	SetViewport(app->viewport.x, app->viewport.y, app->viewport.w, app->viewport.h);

	Clear(0.1f, 0.3f, 0.7f, 1.0f);

	SetModelViewProjectionMatrix(&app->viewProjectionMat.m[0]);

	const Emulator *emulator = &app->emulator;

	const fgbSystem *system = &emulator->system;

	const float w = (float)app->viewport.w;
	const float h = (float)app->viewport.h;

	float displayAspect = FGB_DISPLAY_WIDTH / (float)FGB_DISPLAY_HEIGHT;

	DrawFilledQuad(0, 0, w, h, ColorBlack);
	DrawDisplay(app, 0, 0, w, h, displayAspect);
}

static void RenderFrame(Application *app, const InputState *input) {
	if (app->isDebugEnabled) {
		RenderDebugFrame(app, input);
	} else {
		RenderGameFrame(app, input);
	}
}

static void GameboxLog(void *userData, fgbLogLevel level, const char *system, const char *message) {
	char logText[255] = { 0 };
	if (fplGetStringLength(message) > 0) {
		fplStringFormat(logText, fplArrayCount(logText), "[%s] %s", system, message);
	} else {
		logText[0] = '\0';
	}

	fplDebugFormatOut("%s\n", message);

	UIListboxData *list = (UIListboxData *)userData;
	StringListAdd(&list->values, logText);
	UIListboxScrollTo(list, list->values.count - 1);
}

typedef enum {
	ExitCode_Success = 0,
	ExitCode_MissingGamePakArgument,
	ExitCode_OutOfMemory,
	ExitCode_FailedInitializePlatform,
	ExitCode_FailedInitializeRenderer,
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
			//uint32_t color = 0xFFFF0000;
			uint8_t r = rand() % UINT8_MAX;
			uint8_t g = rand() % UINT8_MAX;
			uint8_t b = rand() % UINT8_MAX;
			//uint8_t r = 0;
			//uint8_t g = 0;
			//uint8_t b = 0;
			uint8_t a = 255;
			//uint32_t color = (r << 0) | (g << 8) | (b << 16) | (a << 24);
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

	emulator->isFrameFinished = false;
	emulator->isVRAMUpdated = false;
	emulator->isActive = false;

	emulator->isShutdown = false;

	while (!emulator->isShutdown) {
		// Wait a while, when a frame is finished and continue loop
		if (emulator->isFrameFinished) {
			fplThreadSleep(0);
			continue;
		}

		// Possible wait until the emulator wakes by the condition variable
		fgbEmulationState state = fgbGetState(system);
		if (!emulator->isActive || state == fgbEmulationState_Paused || state == fgbEmulationState_Error) {
			emulator->isFrameFinished = true;
			emulator->isVRAMUpdated = true;
			fplConditionWait(&emulator->waitCondition, &emulator->mutex, 1000 * 60);
			continue;
		}

		// Process a single CPU tick (4 or more memory ticks)
		BeginPerformanceCounter(&metrics->emulatorTick, fplTimestampQuery());
		bool tickResult = fgbTick(system);
		EndPerformanceCounter(&metrics->emulatorTick, fplTimestampQuery());

		if (tickResult) {

			// When a frame was updated, notify the emulator that is finished
			if (fgbIsFrameUpdated(system)) {
				emulator->isFrameFinished = true;

				// Halt game boy when frame stepping is active
				if (emulator->isFrameStepActive) {
					emulator->isFrameStepActive = false;
					fgbPause(system);
				}
			}

			// When the video ram was updated, notify the emulator that is finished
			if (fgbIsVRAMUpdated(system)) {
				emulator->isVRAMUpdated = true;
			}

			// Halt game boy when micro stepping is active
			if (emulator->isMicroStepActive) {
				emulator->isMicroStepActive = false;
				fgbPause(system);
			}

			// In case the system is paused, always update the textures of the video frame and the VRAM
			if (fgbGetState(system) == fgbEmulationState_Paused) {
				emulator->isFrameFinished = true;
				emulator->isVRAMUpdated = true;
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
				input->activeControllerIndex = i;
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
	
	bool isKeyboard = controllerIndex == 0;
	
	if (isKeyboard)
		UpdateActiveController(newInput, controllerIndex);

	if (isKeyboard)
		UpdateKeyboardButtonState(newButton, isDown);
	else
		UpdateDigitalButtonState(oldButton, newButton, isDown);
}

static void ProcessEvents(Application *app, const InputState *oldInput, InputState *newInput) {
	Emulator *emulator = &app->emulator;

	const Mat4f mvp = app->viewProjectionMat;
	const Viewport4i viewport = app->viewport;

	fplEvent ev;
	while (fplPollEvent(&ev)) {
		switch (ev.type) {
			case fplEventType_Keyboard:
			{
				if (ev.keyboard.type == fplKeyboardEventType_Button) {
					bool isDown = ev.keyboard.buttonState >= fplButtonState_Press;
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
							UpdateActiveController(newInput, controllerIndex);
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
								UpdateActiveController(newInput, controllerIndex);
							}
						}
					} break;
				}
			}
			break;

			case fplEventType_Mouse:
			{
				switch (ev.mouse.type) {
					case fplMouseEventType_Move:
					{
						Vec2i screenMousePos = V2iInit(ev.mouse.mouseX, app->windowSize.h - 1 - ev.mouse.mouseY);
						Vec2f worldMousePos = V2fUnproject(screenMousePos, mvp, viewport);
						newInput->mouse.screenPos = screenMousePos;
						newInput->mouse.worldPos = worldMousePos;
					} break;

					case fplMouseEventType_Button:
					{
						bool isDown = ev.mouse.buttonState >= fplButtonState_Press;
						switch (ev.mouse.mouseButton) {
							case fplMouseButtonType_Left:
								UpdateKeyboardButtonState(&newInput->mouse.left, isDown);
								break;
							case fplMouseButtonType_Right:
								UpdateKeyboardButtonState(&newInput->mouse.right, isDown);
								break;
							case fplMouseButtonType_Middle:
								UpdateKeyboardButtonState(&newInput->mouse.middle, isDown);
								break;
						}
					} break;

					case fplMouseEventType_Wheel:
					{
						newInput->mouse.wheelDelta = ev.mouse.wheelDelta;
					} break;
				}
			}
			break;

			case fplEventType_Window:
			{
				switch (ev.window.type) {
					case fplWindowEventType_DroppedFiles:
					{
						if (ev.window.dropFiles.fileCount != 1) {
							return;
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
				}
			} break;
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

		if (StringCompareIgnoreCase(itemFileExt, ".gb")) {
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
	float *audioSamples;
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
		result = 0;
	} else {
		int16_t *out16 = (int16_t *)outputSamples;

		BeginPerformanceCounter(&metrics->audioReadSamples, fplTimestampQuery());
		result = fgbFetchAudioSamples(system, frameCount, AudioTempSampels);
		EndPerformanceCounter(&metrics->audioReadSamples, fplTimestampQuery());

		BeginPerformanceCounter(&metrics->audioOutputSamples, fplTimestampQuery());
		for (uint32_t frameIndex = 0; frameIndex < result; ++frameIndex) {
			for (uint8_t channelIndex = 0; channelIndex < 2; ++channelIndex) {
				uint8_t rawSample = AudioTempSampels[frameIndex * 2 + channelIndex];
				float sampleF32 = rawSample / 255.0f;
				uint8_t sampleU8 = (uint8_t)((sampleF32 * emulator->masterVolume) * 255.0f);
				int16_t sampleS16 = (sampleU8 << 8) - INT16_MIN;
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

static void ClearDisassembly(UIListboxData *listbox, IndexHashtable *hashtable) {
	StringListClear(&listbox->values);
	IndexHashtableClear(hashtable);
}

static void LoadDisassembly(fgbSystem *system, UIListboxData *listbox, IndexHashtable *hashtable) {
	StringListClear(&listbox->values);
	IndexHashtableClear(hashtable);

	uint16_t addressRange = system->boot.state.isActive ? 0xFF : 0xFFFF;

	for (uint16_t address = 0; address < addressRange; ++address) {
		uint8_t value = fgbBusRead8(system, address);
		disassemblyMemory[address] = value;
	}

	fgbMemory rom = { 0 };
	rom.data = disassemblyMemory;
	rom.length = addressRange;

	fgbDecodedInstruction decoded = { 0 };

	uint16_t pos = 0;
	while (pos < rom.length) {
		fplClearStruct(&decoded);
		if (!fgbDecodeInstruction(&rom, pos, &decoded)) {
			decoded.length = 1;
		}
		fplAssert(decoded.length > 0 && decoded.length <= 3);

		disassemblyLineBuffer[0] = 0;

		fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "%04X | ", pos);
		fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));

		int8_t m = 3 - decoded.length;
		if (m > 0) {
			for (uint8_t x = 0; x < m; x++) {
				fplStringAppend("   ", disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
			}
		}

		for (uint8_t x = 0; x < decoded.length; x++) {
			fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "%02X ", rom.data[pos + x]);
			fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		}

		fplStringFormat(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), "| ");
		fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));

		if (pos >= FGB__GAMEPAK_HEADER_POSITION && pos <= FGB__GAMEPAK_HEADER_POSITION + sizeof(fgb__GamePakHeader)) {
			fplStringAppend("GAMEPAK_HEADER", disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		} else {
			fgbFormatInstruction(disassemblyTempBuffer, fplArrayCount(disassemblyTempBuffer), &decoded);
			fplStringAppend(disassemblyTempBuffer, disassemblyLineBuffer, fplArrayCount(disassemblyLineBuffer));
		}

		size_t listIndex = StringListAdd(&listbox->values, disassemblyLineBuffer);

		IndexHashtableAdd(hashtable, pos, listIndex);

		pos += decoded.length;
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
	for (uint8_t i = 0; i < fplArrayCount(emulator->states.snapshots); ++i) {
		fgbSnapshot *snapshot = emulator->states.snapshots + i;
		fplClearStruct(snapshot);

		Texture *texture = emulator->states.textures + i;
		ClearPixelsTexture(texture);
		UpdateTexture(texture);
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
	for (uint8_t i = 0; i < fplArrayCount(emulator->states.snapshots); ++i) {
		fgbSnapshot *snapshot = emulator->states.snapshots + i;
		Texture *texture = emulator->states.textures + i;

		fplClearStruct(snapshot);

		if (fgbSnapshotLoadFromFile(&emulator->system, filePath, i, snapshot)) {
			TransferPixelsToTexture(snapshot->ppu.display, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, texture);
			UpdateTexture(texture);
		} else {
			ClearPixelsTexture(texture);
			UpdateTexture(texture);
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
	settings->video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	settings->video.isVSync = false;

	settings->audio.targetFormat.sampleRate = 48000;
	settings->audio.targetFormat.channels = 2;
	settings->audio.targetFormat.type = fplAudioFormatType_S16;
	settings->audio.startAuto = false;
	settings->audio.stopAuto = false;
}

static void SetupGamebox(fgbConfiguration *config, Application *app, const uint32_t sampleRate) {
	// Callbacks
	config->callbacks = globalCallbacks;

	// Logging
	config->log.userData = &app->console;
	config->log.callback = GameboxLog;
	config->log.isEnabled = true;

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

	// Disable boot rom
	fplClearStruct(&config->bootROM);
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

static void SetupGameboxInput(InputState *newInput, fgbSystem *system) {
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
		ClearDisassembly(&app->disassemblyList, &app->disassemblyHashTable);
		StringListClear(&app->console.values);
	}

	if (EmulatorLoadGame(emulator, romFilePath->text)) {
		LoadDisassembly(system, &app->disassemblyList, &app->disassemblyHashTable);
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

static void InitializeUI(Application *app) {
	UIContext *uiCtx = &app->uiCtx;
	const float charHeight = 20.0f;
	const float lineScale = 1.15f;
	const float lineHeight = charHeight * lineScale;
	UISetFont(uiCtx, &app->fontData, app->fontTexture.id, charHeight, lineHeight);
}

static void PrepareInputUI(Application *app, InputState *newInput) {
	UIContext *uiCtx = &app->uiCtx;
	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;

	UIInputState uiInputState = fplZeroInit;
	uiInputState.leftMouse = newInput->mouse.left;
	uiInputState.rightMouse = newInput->mouse.right;
	uiInputState.middleMouse = newInput->mouse.middle;
	uiInputState.escapeButton = newInput->keyboardController.select;
	uiInputState.mousePos = newInput->mouse.worldPos;
	uiInputState.projectionMat = app->projectionMat;
	uiInputState.viewMat = app->viewMat;
	uiInputState.viewport = app->viewport;
	uiInputState.mouseWheelDelta = newInput->mouse.wheelDelta;
	UIContextSetInput(uiCtx, &uiInputState);
}

static void HandleDefaultInput(Application *app, InputState *newInput) {
	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;

	if (UIWasPressed(&newInput->debug.toggleDebug)) {
		app->isDebugEnabled = !app->isDebugEnabled;
	}

	if (emulator->isActive && system->state != fgbEmulationState_Error) {
		HighlightScrollDisassembly(app);
	}
}

static bool HasGameboxVideoChanges(const Emulator *emulator) {
	if (!emulator->isActive) {
		return false;
	}
	if (emulator->isFrameFinished || emulator->isVRAMUpdated) {
		return true;
	}
	return false;
}

static void UploadStateTextures(States *states) {
	for (uint8_t i = 0; i < fplArrayCount(states->snapshots); ++i) {
		fgbSnapshot *snapshot = states->snapshots + i;
		Texture *texture = states->textures + i;
		if (texture->state == TextureState_Update) {
			TransferPixelsToTexture(snapshot->ppu.display, FGB_DISPLAY_WIDTH, FGB_DISPLAY_HEIGHT, texture);
			UpdateTexture(texture);
			texture->state = TextureState_None;
		} else if (texture->state == TextureState_Clear) {
			ClearPixelsTexture(texture);
			ClearTexture(texture);
		}
	}
}

static void UploadGameboxTextures(Application *app) {
	Emulator *emulator = &app->emulator;
	fgbSystem *system = &emulator->system;

	if (!emulator->isActive) {
		return;
	}

	if (emulator->isFrameFinished) {
		UpdateBackgroundMapTexture(app, &system->ppu);
		UpdateDisplayTexture(app, &system->ppu);
		emulator->isFrameFinished = false;
	}

	if (emulator->isVRAMUpdated) {
		UpdateVRAMTexture(app, &system->ppu);
		emulator->isVRAMUpdated = false;
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

	app->windowSize = V2iInit(size.width, size.height);
	app->viewMat = M4fMult(M4fScaleScalar(scale), M4fTranslationV2(V2fInit(translationX, translationY)));
	app->projectionMat = M4fOrthoRH(0.0f, (float)size.width, 0.0f, (float)size.height, 0.0f, 1.0f);
	app->viewProjectionMat = M4fMult(app->projectionMat, app->viewMat);
	app->viewport = VP4iInit(0, 0, size.width, size.height);
}

static void UpdateWindowTitle(Emulator *emulator, const double frameRate) {
	static char windowTitleBuffer[512];

	const char *gamePakTitle = fplExtractFileName(emulator->system.gamePak.filePath);
	if (fplGetStringLength(gamePakTitle) == 0) {
		gamePakTitle = "No game pak loaded";
	}
	fplStringFormat(windowTitleBuffer, fplArrayCount(windowTitleBuffer), "FPL Emulator - %s - (Fps: %.2f)\n", gamePakTitle, frameRate);
	fplSetWindowTitle(windowTitleBuffer);
}

// Main Entry Point (No need for WinMain or anything like that, due to FPL)
int main(int argc, char **argv) {
	int exitCode = 0;

	const char *romFilePath = argc >= 2 ? argv[1] : fpl_null;

	// Initalize random seed
	srand(42U);

	Application *app = fpl_null;
	RendererContext *renderer = fpl_null;

	// Allocate transient memory (Used for rom file and external ram file loading)
	globalTransientMemory.base = CreateTransientMemory(fplMegaBytes(8));

	// Allocate main memory
	fmemMemoryBlock mainMemory = CreatePersistentMemory(fplMegaBytes(32));
	if (mainMemory.base == fpl_null) {
		return ExitCode_OutOfMemory;
	}

	// Setup & Initialize Platform
	fplSettings *settings = fmemPushStruct(&mainMemory, fplSettings, fmemPushFlags_Clear);

	SetupPlatformSettings(&mainMemory, settings);

	if (!fplPlatformInit(fplInitFlags_All & ~fplInitFlags_Console, settings)) {
		return ExitCode_FailedInitializePlatform;
	}

	// Initialize the renderer (OpenGL)
	renderer = RendererCreate(&mainMemory);
	if (renderer == fpl_null) {
		exitCode = ExitCode_FailedInitializeRenderer;
		goto shutdown;
	}

	// Create Application & Emulator resources and start the threads
	app = CreateApplication(&mainMemory);
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
	SetupGamebox(&emulator->config, app, hardwareAudioFormat.sampleRate);

	// Set audio callback and its data
	AudioThreadState audioThreadState = { 0 };
	audioThreadState.system = system;
	audioThreadState.emulator = emulator;
	fplSetAudioClientReadCallback(AudioThreadCallback, &audioThreadState);

	// Default game rom (Retroid from Jonas Fischbach)
	fplPathCombine(app->defaultGameRomFilePath, fplArrayCount(app->defaultGameRomFilePath), 2, app->romsPath, "Retroid.zip");
	if (fplGetStringLength(romFilePath) == 0) {
		romFilePath = app->defaultGameRomFilePath;
	}

	// Auto load initial rom file from arguments
	if (fplGetStringLength(romFilePath) > 0) {
		if (EmulatorLoadGame(emulator, romFilePath)) {
			LoadDisassembly(&emulator->system, &app->disassemblyList, &app->disassemblyHashTable);
		}
	}

	// Initialize UI and Input
	InitializeUI(app);

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

		// Process events and handle events from the window (Keyboard, Mouse, Gamepad, etc.)
		ProcessEvents(app, oldInput, newInput);

		// Setup gamebox input from new input
		SetupGameboxInput(newInput, system);

		// Show various informations in the window title bar
		UpdateWindowTitle(emulator, timing.frameRate);

		// Load a rom file, if requested by e.g. a drag & drop operation
		if (HasRequestedROMFile(app)) {
			LoadRequestedROMFile(app, &emulator->pendingROMFilePath);
		}

		// Setup input for UI
		PrepareInputUI(app, newInput);

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

