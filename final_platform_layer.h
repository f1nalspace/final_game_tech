/*
final_platform_layer.h - FPL -  v0.1
Open-Source Single-File Header-Library by Torsten Spaete

This library is designed to abstract the underlying platform
to a very simple and easy to understand api
with requiring just built-in operatoring system dependencies only.

The main focus is game development, so the default settings
will create a window and setup a rendering context.

It is fully C-89 compatible and will compile in C++ as well.

HOW TO USE:

In one of your C or C++ translation unit define this:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

You must provide the typical main entry point:

In the main() entry point initialize the library and use whatever functionality you want and release the library when you are done.

When using opengl, you may need to link to the systems opengl library manually.

PREPROCESSOR OVERRIDES:

- FPL_API_AS_PRIVATE 0 or 1

- FPL_DEFAULT_WINDOW_WIDTH 1 or greater
- FPL_DEFAULT_WINDOW_HEIGHT 1 or greater

- FPL_ENABLE_ASSERTIONS 0 or 1
- FPL_ENABLE_CLIB_ASSERTIONS 0 or 1
- FPL_ENABLE_WINDOW 0 or 1
- FPL_ENABLE_OPENGL 0 or 1

FEATURES:

[p] Creating a fixed or resizeable window
[x] Handling window, keyboard, mouse events
[ ] Polling gamepad informations
[ ] Clipboard string reading and writing

[x] Creating a opengl rendering context
[ ] Support for loading opengl extensions
[ ] Modern opengl 3+ support

[ ] Audio playback using OS native libraries

[x] Memory allocation and de-allocation with custom alignment support
[x] Atomic operations
[ ] String manipulation functions
[ ] Path, file and directory functions
[ ] Thread, mutex, condition handling

SUPPORTED ARCHITECTURES:

[x] x86
[x] x86_64

SUPPORTED COMPILERS:

[X] Compiles in Visual Studio 2015+
[ ] Compiles in GCC
[ ] Compiles in Clang

SUPPORTED PLATFORMS:

[x] Win32
[ ] Linux (Planned)
[ ] Unix/Posix (Planned)
[ ] OSX (Not sure)

LICENSE:

MIT License

Copyright (c) 2017 Torsten Spaete
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

VERSION HISTORY:
v0.1 (2017-05-10) Initial version
*/

#ifndef FPL_INCLUDE_H
#define FPL_INCLUDE_H

// ****************************************************************************
//
// Header
//
// ****************************************************************************

//
// Platform detection
//
#if defined(_WIN32)
#	define FPL_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FPL_PLATFORM_LINUX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FPL_PLATFORM_UNIX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// Architecture detection (x86, x64)
// See: https://sourceforge.net/p/predef/wiki/Architectures/
//
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
#	define FPL_ARCH_X64	
#elif defined(_WIN32) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#else
#	error "This architecture/compiler is not supported!"
#endif

//
// Build configuration and compilers
//
// Based on:
// http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
#if defined(_MSC_VER)
#	define FPL_COMPILER_MSVC
#	if defined(_DEBUG) || (!defined(NDEBUG))
#		define FPL_DEBUG
#	else
#		define FPL_RELEASE
#	endif
#elif defined(__llvm__)
#	define FPL_COMPILER_LLVM
#elif defined(__clang__)
#	define FPL_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define FPL_COMPILER_GCC
#elif defined(__MINGW32__)
#	define FPL_COMPILER_MINGW
#elif defined(__INTEL_COMPILER) || defined(__ICC)
#	define FPL_COMPILER_INTEL
#else
#	define FPL_COMPILER_UNKNOWN
#endif

//
// Default preprocessor overrides
//
#if !defined(FPL_DEFAULT_WINDOW_WIDTH)
#	define FPL_DEFAULT_WINDOW_WIDTH 800
#endif
#if !defined(FPL_DEFAULT_WINDOW_HEIGHT)
#	define FPL_DEFAULT_WINDOW_HEIGHT 600
#endif
#if !defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_DEBUG)
#		define FPL_ENABLE_ASSERTIONS 1
#	else
#		define FPL_ENABLE_ASSERTIONS 0
#	endif
#endif
#if !defined(FPL_ENABLE_OPENGL)
#	define FPL_ENABLE_OPENGL 1
#endif
#if !defined(FPL_ENABLE_WINDOW)
#	define FPL_ENABLE_WINDOW 1
#endif

//
// Types
//
#include <stdint.h>
typedef int32_t bool32;

//
// Assertions
//
#if FPL_ENABLE_ASSERTIONS
#if FPL_ENABLE_CLIB_ASSERTIONS
#	include <assert.h>
#	define FPL_ASSERT(exp) assert(exp)
#	define FPL_STATICASSERT(exp) static_assert(exp)
#else
#	define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
#	define FPL_STATICASSERT_INTERNAL(exp, line) \
		int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#	define FPL_STATICASSERT(exp) \
		FPL_STATICASSERT_INTERNAL(exp, __LINE__)
#endif
#else
#	define FPL_ASSERT(exp)
#	define FPL_STATICASSERT(exp)
#endif

//
// Static/Inline/Extern
//
#define fpl_globalvar static
#define fpl_inline inline
#if FPL_API_AS_PRIVATE
#	define fpl_api static
#else
#	define fpl_api extern
#endif

#ifdef __cplusplus
extern
"C" {
#endif
	//
	// API
	//
	// @NOTE: Based on MS Virtual-Key-Codes, mostly directly mappable to US-ASCII
	enum {
		fpl_Key_None = 0,

		// 0x07: Undefined

		fpl_Key_Backspace = 0x08,
		fpl_Key_Tab = 0x09,

		// 0x0A-0x0B: Reserved

		fpl_Key_Clear = 0x0C,
		fpl_Key_Enter = 0x0D,

		// 0x0E-0x0F: Undefined

		fpl_Key_Shift = 0x10,
		fpl_Key_Control = 0x11,
		fpl_Key_Alt = 0x12,
		fpl_Key_Pause = 0x13,
		fpl_Key_CapsLock = 0x14,

		// 0x15: IME-Keys
		// 0x16: Undefined
		// 0x17-0x19 IME-Keys
		// 0x1A: Undefined

		fpl_Key_Escape = 0x1B,

		// 0x1C - 0x1F: IME-Keys

		fpl_Key_Space = 0x20,
		fpl_Key_PageUp = 0x21,
		fpl_Key_PageDown = 0x22,
		fpl_Key_End = 0x23,
		fpl_Key_Home = 0x24,
		fpl_Key_Left = 0x25,
		fpl_Key_Up = 0x26,
		fpl_Key_Right = 0x27,
		fpl_Key_Down = 0x28,
		fpl_Key_Select = 0x29,
		fpl_Key_Print = 0x2A,
		fpl_Key_Execute = 0x2B,
		fpl_Key_Snapshot = 0x2C,
		fpl_Key_Insert = 0x2D,
		fpl_Key_Delete = 0x2E,
		fpl_Key_Help = 0x2F,

		fpl_Key_0 = 0x30,
		fpl_Key_1 = 0x31,
		fpl_Key_2 = 0x32,
		fpl_Key_3 = 0x33,
		fpl_Key_4 = 0x34,
		fpl_Key_5 = 0x35,
		fpl_Key_6 = 0x36,
		fpl_Key_7 = 0x37,
		fpl_Key_8 = 0x38,
		fpl_Key_9 = 0x39,

		// 0x3A-0x40: Undefined

		fpl_Key_A = 0x41,
		fpl_Key_B = 0x42,
		fpl_Key_C = 0x43,
		fpl_Key_D = 0x44,
		fpl_Key_E = 0x45,
		fpl_Key_F = 0x46,
		fpl_Key_G = 0x47,
		fpl_Key_H = 0x48,
		fpl_Key_I = 0x49,
		fpl_Key_J = 0x4A,
		fpl_Key_K = 0x4B,
		fpl_Key_L = 0x4C,
		fpl_Key_M = 0x4D,
		fpl_Key_N = 0x4E,
		fpl_Key_O = 0x4F,
		fpl_Key_P = 0x50,
		fpl_Key_Q = 0x51,
		fpl_Key_R = 0x52,
		fpl_Key_S = 0x53,
		fpl_Key_T = 0x54,
		fpl_Key_U = 0x55,
		fpl_Key_V = 0x56,
		fpl_Key_W = 0x57,
		fpl_Key_X = 0x58,
		fpl_Key_Y = 0x59,
		fpl_Key_Z = 0x5A,

		fpl_Key_LeftWin = 0x5B,
		fpl_Key_RightWin = 0x5C,
		fpl_Key_Apps = 0x5D,

		// 0x5E: Reserved

		fpl_Key_Sleep = 0x5F,
		fpl_Key_NumPad0 = 0x60,
		fpl_Key_NumPad1 = 0x61,
		fpl_Key_NumPad2 = 0x62,
		fpl_Key_NumPad3 = 0x63,
		fpl_Key_NumPad4 = 0x64,
		fpl_Key_NumPad5 = 0x65,
		fpl_Key_NumPad6 = 0x66,
		fpl_Key_NumPad7 = 0x67,
		fpl_Key_NumPad8 = 0x68,
		fpl_Key_NumPad9 = 0x69,
		fpl_Key_Multiply = 0x6A,
		fpl_Key_Add = 0x6B,
		fpl_Key_Separator = 0x6C,
		fpl_Key_Substract = 0x6D,
		fpl_Key_Decimal = 0x6E,
		fpl_Key_Divide = 0x6F,
		fpl_Key_F1 = 0x70,
		fpl_Key_F2 = 0x71,
		fpl_Key_F3 = 0x72,
		fpl_Key_F4 = 0x73,
		fpl_Key_F5 = 0x74,
		fpl_Key_F6 = 0x75,
		fpl_Key_F7 = 0x76,
		fpl_Key_F8 = 0x77,
		fpl_Key_F9 = 0x78,
		fpl_Key_F10 = 0x79,
		fpl_Key_F11 = 0x7A,
		fpl_Key_F12 = 0x7B,
		fpl_Key_F13 = 0x7C,
		fpl_Key_F14 = 0x7D,
		fpl_Key_F15 = 0x7E,
		fpl_Key_F16 = 0x7F,
		fpl_Key_F17 = 0x80,
		fpl_Key_F18 = 0x81,
		fpl_Key_F19 = 0x82,
		fpl_Key_F20 = 0x83,
		fpl_Key_F21 = 0x84,
		fpl_Key_F22 = 0x85,
		fpl_Key_F23 = 0x86,
		fpl_Key_F24 = 0x87,

		// 0x88-8F: Unassigned

		fpl_Key_NumLock = 0x90,
		fpl_Key_Scroll = 0x91,

		// 0x92-9x96: OEM specific
		// 0x97-0x9F: Unassigned

		fpl_Key_LeftShift = 0xA0,
		fpl_Key_RightShift = 0xA1,
		fpl_Key_LeftControl = 0xA2,
		fpl_Key_RightControl = 0xA3,
		fpl_Key_LeftAlt = 0xA4,
		fpl_Key_RightAlt = 0xA5,

		// 0xA6-0xFE: Dont care
	};

	typedef uint64_t fpl_Key;

	typedef struct fpl_WindowConfiguration {
		char windowTitle[128];
		uint32_t windowWidth;
		uint32_t windowHeight;
		bool32 vsyncEnabled;
	} fpl_WindowConfiguration;

	typedef enum fpl_WindowEventType {
		fpl_WindowEventType_Resize = 1,
	} fpl_WindowEventType;

	typedef struct fpl_WindowEvent {
		fpl_WindowEventType type;
		int32_t width;
		int32_t height;
	} fpl_WindowEvent;

	typedef enum fpl_KeyboardEventType {
		fpl_KeyboardEventType_KeyDown = 1,
		fpl_KeyboardEventType_KeyUp,
		fpl_KeyboardEventType_Char,
	} fpl_KeyboardEventType;

	enum {
		fpl_KeyboardModifierType_None = 0,
		fpl_KeyboardModifierType_Alt = 1 << 0,
		fpl_KeyboardModifierType_Ctrl = 1 << 1,
		fpl_KeyboardModifierType_Shift = 1 << 2,
		fpl_KeyboardModifierType_Super = 1 << 3,
	};
	typedef uint32_t fpl_KeyboardModifierType;

	typedef struct fpl_KeyboardEvent {
		fpl_KeyboardEventType type;
		uint64_t keyCode;
		fpl_Key mappedKey;
		fpl_KeyboardModifierType modifiers;
	} fpl_KeyboardEvent;

	typedef enum fpl_MouseEventType {
		fpl_MouseEventType_Move = 1,
		fpl_MouseEventType_ButtonDown,
		fpl_MouseEventType_ButtonUp,
		fpl_MouseEventType_Wheel,
	} fpl_MouseEventType;

	typedef enum fpl_MouseButtonType {
		fpl_MouseButtonType_None = -1,
		fpl_MouseButtonType_Left = 0,
		fpl_MouseButtonType_Right,
		fpl_MouseButtonType_Middle,
	} fpl_MouseButtonType;

	typedef struct fpl_MouseEvent {
		fpl_MouseEventType type;
		fpl_MouseButtonType mouseButton;
		int32_t mouseX;
		int32_t mouseY;
		float wheelDelta;
		int32_t _padding;
	} fpl_MouseEvent;

	typedef enum fpl_EventType {
		fpl_EventType_Window = 1,
		fpl_EventType_Keyboard,
		fpl_EventType_Mouse,
	} fpl_EventType;

	typedef struct fpl_Event {
		fpl_EventType type;
		union {
			fpl_WindowEvent window;
			fpl_KeyboardEvent keyboard;
			fpl_MouseEvent mouse;
		};
	} fpl_Event;

	enum {
		fpl_InitFlag_None = 0,
		fpl_InitFlag_Window = 1 << 0,
		fpl_InitFlag_VideoOpenGL = 1 << 1,
	};
	typedef uint32_t fpl_InitFlag;

#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(FPL_PLATFORM_WINDOWS)
	// @NOTE(final): Required for access "main" from the actual win32 entry point
	fpl_api int main(int argc, char **args);
#endif

	// Atomics
	fpl_api uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, uint32_t value);
	fpl_api uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, uint64_t value);
	fpl_api uint32_t fpl_AtomicAddU32(volatile uint32_t *value, uint32_t addend);
	fpl_api uint64_t fpl_AtomicAddU64(volatile uint64_t *value, uint64_t addend);
	fpl_api uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, uint32_t exchange, uint32_t comparand);
	fpl_api uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, uint64_t exchange, uint64_t comparand);

	// Core
	fpl_api bool32 fpl_Init(fpl_InitFlag initFlags);
	fpl_api void fpl_Release();

	// Window
	fpl_api bool32 fpl_IsWindowRunning();
	fpl_api bool32 fpl_WindowUpdate();
	fpl_api void fpl_WindowFlip();
	fpl_api int32_t fpl_GetWindowWidth();
	fpl_api int32_t fpl_GetWindowHeight();

	// Events
	fpl_api bool32 fpl_PollEvent(fpl_Event *event);

	// Memory
	fpl_api void fpl_ClearMemory(void *mem, size_t size);
	fpl_api void *fpl_AllocateMemory(size_t size);
	fpl_api void fpl_FreeMemory(void *ptr);
	fpl_api void *fpl_AllocateAlignedMemory(size_t size, size_t alignment);
	fpl_api void fpl_FreeAlignedMemory(void *ptr);

	// Timing
	fpl_api double fpl_GetHighResolutionTimeInSeconds();

	// Strings
	fpl_api uint32_t fpl_GetStringLength(const char *str);
	fpl_api uint32_t fpl_GetWideStringLength(const wchar_t *str);
	fpl_api void fpl_CopyString(const char *source, uint32_t sourceLen, char *dest, uint32_t maxDestLen);
	fpl_api void fpl_CopyWideString(const wchar_t *source, uint32_t sourceLen, wchar_t *dest, uint32_t maxDestLen);
	fpl_api void fpl_WideStringToOEMString(const wchar_t *wideSource, uint32_t maxWideSourceLen, char *oemDest, uint32_t maxOemDestLen);
	fpl_api void fpl_WideStringToUTF8String(const wchar_t *wideSource, uint32_t maxWideSourceLen, char *utf8Dest, uint32_t maxUtf8DestLen);
	fpl_api void fpl_OEMStringToWideString(const char *oemSource, uint32_t oemSourceLen, wchar_t *wideDest, uint32_t maxWideDestLen);
	fpl_api void fpl_UTF8StringToWideString(const char *utf8Source, uint32_t utf8SourceLen, wchar_t *wideDest, uint32_t maxWideDestLen);

	// Files, Directories and Paths
	fpl_api void fpl_GetExecutableFilePath(char *dest, uint32_t maxDestLen);
	fpl_api char *fpl_ExtractFilePath(const char *sourcePath, char *destPath, uint32_t maxDestLen);
	fpl_api void fpl_CombinePath(char *destPath, uint32_t maxDestPathLen, const char *basePath, ...);
#ifdef __cplusplus
}
#endif

#endif // FPL_INCLUDE_H

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#		define FPL_IMPLEMENTED
void *fpl_AllocateAlignedMemory(size_t size, size_t alignment) {
	FPL_ASSERT(size > 0);
	FPL_ASSERT((alignment > 0) && !(alignment & (alignment - 1)));

	// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
	size_t newSize = sizeof(void *) + size + (alignment << 1);
	void *basePtr = fpl_AllocateMemory(newSize);
	fpl_ClearMemory(basePtr, newSize);

	// The resulting address starts after the stored base pointer
	void *alignedPtr = (void *)((uintptr_t)basePtr + sizeof(void *));

	// Move the resulting address to a aligned one when not aligned
	// @TODO(final): This seems to do nothing on a typical i7 machine, regardless of the target - test with other architectures!
	uintptr_t mask = alignment - 1;
	if ((alignment > 1) && (((uintptr_t)alignedPtr & mask) != 0)) {
		*(uintptr_t *)alignedPtr += ((uintptr_t)alignment - ((uintptr_t)alignedPtr & mask));
	}

	// Write the base pointer before the alignment pointer
	*(void **)((void *)((uintptr_t)alignedPtr - sizeof(void *))) = basePtr;

	return(alignedPtr);
}

void fpl_FreeAlignedMemory(void *ptr) {
	FPL_ASSERT(ptr != NULL);

	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
	fpl_FreeMemory(basePtr);
}

#define FPL_MemoryBlockClear_Internal(type, mem, size, shift, mask) do { \
	type *dataBlock = (type *)mem; \
	type *dataBlockEnd = (type *)dataBlock + (size >> shift); \
	uint8_t *data8 = (uint8_t *)dataBlockEnd; \
	uint8_t *data8End = data8 + (size & mask); \
	while (dataBlock != dataBlockEnd) { \
		*dataBlock++ = 0; \
	} \
	while (data8 != data8End) { \
		*data8++ = 0; \
	} \
} while (0)

void fpl_ClearMemory(void *mem, size_t size) {
	// @TODO(final): This can be more efficient: Clear by 8 than 4 than 2 then the rest
	if (size % 8 == 0) {
		FPL_MemoryBlockClear_Internal(uint64_t, mem, size, 3, 0x00000007);
	} else if (size % 4 == 0) {
		FPL_MemoryBlockClear_Internal(uint32_t, mem, size, 2, 0x00000003);
	} else if (size % 2 == 0) {
		FPL_MemoryBlockClear_Internal(uint16_t, mem, size, 1, 0x00000001);
	} else {
		uint8_t *data8 = (uint8_t *)mem;
		uint8_t *data8End = data8 + size;
		while (data8 != data8End) {
			*data8++ = 0;
		}
	}
}

#define FPL_MAX_EVENT_COUNT 32768
typedef struct fpl_EventQueue {
	fpl_Event events[FPL_MAX_EVENT_COUNT];
	volatile uint32_t index;
	volatile uint32_t count;
} fpl_EventQueue;

fpl_globalvar fpl_EventQueue *fpl_GlobalEventQueue_Internal = 0;

bool32 fpl_PollEvent(fpl_Event *event) {
	bool32 result = 0;
	fpl_EventQueue *eventQueue = fpl_GlobalEventQueue_Internal;
	FPL_ASSERT(eventQueue != NULL);
	if (eventQueue->count > 0 && (eventQueue->index < eventQueue->count)) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->index, 1);
		*event = eventQueue->events[eventIndex];
		result = 1;
	} else if (fpl_GlobalEventQueue_Internal->count > 0) {
		fpl_AtomicExchangeU32(&fpl_GlobalEventQueue_Internal->index, 0);
		fpl_AtomicExchangeU32(&fpl_GlobalEventQueue_Internal->count, 0);
	}
	return result;
}

fpl_inline void fpl_PushEvent_Internal(fpl_EventQueue *eventQueue, fpl_Event *event) {
	FPL_ASSERT(eventQueue != NULL);
	uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->count, 1);
	FPL_ASSERT(eventIndex < FPL_MAX_EVENT_COUNT);
	eventQueue->events[eventIndex] = *event;
}

uint32_t fpl_GetStringLength(const char *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

uint32_t fpl_GetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

void fpl_CopyString(const char *source, uint32_t sourceLen, char *dest, uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	*dest = 0;
}

void fpl_CopyWideString(const wchar_t *source, uint32_t sourceLen, wchar_t *dest, uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	dest[sourceLen] = 0;
}

#define FPL_CLEARSTRUCT(value) \
	fpl_ClearMemory(value, sizeof(*value))

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WINDOWS)

#	include <intrin.h>
	// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break std::min/max so we have to tell the header we dont want this!
#	define NOMINMAX
#	include <windows.h>
#	include <windowsx.h>
#	if FPL_ENABLE_WINDOW
#		pragma comment(linker, "/subsystem:windows")
#		if FPL_ENABLE_OPENGL
#			include <gl\gl.h>
#			pragma comment( lib, "opengl32.lib" )
#		endif // FPL_ENABLE_OPENGL
#	endif // FPL_ENABLE_WINDOW

#define FPL_WIN32_PATH_SEPARATOR '\\'

// Intrinsics
#if defined(FPL_COMPILER_MSVC)
uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, uint32_t value) {
	uint32_t result = _InterlockedExchange((volatile long *)target, value);
	return (result);
}
uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, uint64_t value) {
	uint64_t result = InterlockedExchange64((volatile long long *)target, value);
	return (result);
}
uint32_t fpl_AtomicAddU32(volatile uint32_t *value, uint32_t addend) {
	uint32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
	return (result);
}
uint64_t fpl_AtomicAddU64(volatile uint64_t *value, uint64_t addend) {
	uint64_t result = InterlockedExchangeAdd64((volatile long long *)value, addend);
	return (result);
}
uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, uint32_t exchange, uint32_t comparand) {
	uint32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
	return (result);
}
uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, uint64_t exchange, uint64_t comparand) {
	uint64_t result = InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
	return (result);
}
#endif // defined(FPL_COMPILER_MSVC)

#if defined(UNICODE)
typedef wchar_t fpl_win32_char;
#else
typedef char fpl_win32_char;
#endif // defined(UNICODE)

typedef struct fpl_Win32State {
	HINSTANCE appInstance;
	LARGE_INTEGER performanceFrequency;
#if FPL_ENABLE_WINDOW
	HWND windowHandle;
	fpl_win32_char windowClass[256];
	HDC deviceContext;
#	if FPL_ENABLE_OPENGL
	HGLRC renderingContext;
#	endif
#endif // FPL_ENABLE_WINDOW
	bool32 isRunning;
} fpl_Win32State;

fpl_globalvar fpl_Win32State fpl_GlobalWin32State_Internal = { 0 };

// @TODO: Remove this entirely
#if FPL_ENABLE_OPENGL && FPL_ENABLE_WINDOW
#	define WGL_SWAP_INTERVAL_FUNCTION(name) BOOL WINAPI name(int value)
typedef WGL_SWAP_INTERVAL_FUNCTION(wgl_swap_interval);
fpl_globalvar wgl_swap_interval *wglSwapIntervalEXT = NULL;
#endif // FPL_ENABLE_OPENGL && FPL_ENABLE_WINDOW

// @TODO: Dont like this overwritting the defines like that
#undef WNDCLASSEX
#undef RegisterClassEx
#undef UnregisterClass
#undef CreateWindowEx
#undef DefWindowProc
#undef GetWindowLongPtr
#undef SetWindowLongPtr
#undef DispatchMessage
#undef GetModuleFileName
#if defined(UNICODE)
#	define WIN32_CLASSNAME L"FPLWindowClass"
#	define WIN32_UNNAMED_WINDOW L"Unnamed FPL Window"
#	define WNDCLASSEX WNDCLASSEXW
#	define RegisterClassEx RegisterClassExW
#	define UnregisterClass UnregisterClassW
#	define CreateWindowEx CreateWindowExW
#	define DefWindowProc DefWindowProcW
#	define GetWindowLongPtr GetWindowLongPtrW
#	define SetWindowLongPtr SetWindowLongPtrW
#	define PeekMessage PeekMessageW
#	define DispatchMessage DispatchMessageW
#	define fpl_Win32StringCopy fpl_CopyWideString
#	define fpl_Win32GetStringLength fpl_GetWideStringLength
#	define MapVirtualKey MapVirtualKeyW
#else
#	define WIN32_CLASSNAME "FPLWindowClass"
#	define WIN32_UNNAMED_WINDOW "Unnamed FPL Window"
#	define WNDCLASSEX WNDCLASSEXA
#	define RegisterClassEx RegisterClassExA
#	define UnregisterClass UnregisterClassA
#	define CreateWindowEx CreateWindowExA
#	define DefWindowProc DefWindowProcA
#	define GetWindowLongPtr GetWindowLongPtrA
#	define SetWindowLongPtr SetWindowLongPtrA
#	define PeekMessage PeekMessageA
#	define DispatchMessage DispatchMessageA
#	define fpl_Win32StringCopy fpl_CopyString
#	define fpl_Win32GetStringLength fpl_GetStringLength
#	define MapVirtualKey MapVirtualKeyA
#endif // defined(UNICODE)

//
// Memory
//
void *fpl_AllocateMemory(size_t size) {
	FPL_ASSERT(size > 0);
	void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return(result);
}

void fpl_FreeMemory(void *ptr) {
	FPL_ASSERT(ptr != NULL);
	VirtualFree(ptr, 0, MEM_FREE);
}

//
// File/Path IO
//
void fpl_GetExecutableFilePath(char *dest, uint32_t maxDestLen) {
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
#if defined(UNICODE)
	wchar_t modulePath[MAX_PATH + 1];
	GetModuleFileNameW(NULL, modulePath, MAX_PATH + 1);
	fpl_WideStringToOEMString(modulePath, fpl_GetWideStringLength(modulePath), dest, maxDestLen);
#else
	char modulePath[MAX_PATH + 1];
	GetModuleFileNameA(NULL, modulePath, MAX_PATH + 1);
	fpl_CopyString(modulePath, fpl_GetStringLength(modulePath), dest, maxDestLen);
#endif
}

void fpl_CombinePath(char *destPath, uint32_t maxDestPathLen, const char *basePath, ...) {
	// @TODO: Implement this!
}

char *fpl_ExtractFilePath(const char *sourcePath, char *destPath, uint32_t maxDestLen) {
	char *result = 0;
	if (sourcePath) {
		int copyLen = 0;
		char *chPtr = (char *)sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL_WIN32_PATH_SEPARATOR) {
				copyLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		if (copyLen) {
			fpl_CopyString(sourcePath, copyLen, destPath, maxDestLen);
		}
	}
	return(result);
}

//
// Timing
//
fpl_inline LARGE_INTEGER fpl_Win32GetWallClock_Internal() {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return(result);
}
double fpl_GetHighResolutionTimeInSeconds() {
	LARGE_INTEGER clock = fpl_Win32GetWallClock_Internal();
	double result = clock.QuadPart / (double)fpl_GlobalWin32State_Internal.performanceFrequency.QuadPart;
	return(result);
}

//
// String
//
void fpl_WideStringToOEMString(const wchar_t *wideSource, uint32_t maxWideSourceLen, char *oemDest, uint32_t maxOemDestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxOemDestLen >= (requiredLen + 1));
	WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, oemDest, maxOemDestLen, NULL, NULL);
	oemDest[requiredLen] = 0;
}
void fpl_WideStringToUTF8String(const wchar_t *wideSource, uint32_t maxWideSourceLen, char *utf8Dest, uint32_t maxUtf8DestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxUtf8DestLen >= (requiredSize + 1));
	WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, NULL, NULL);
	utf8Dest[requiredLen] = 0;
}
void fpl_OEMStringToWideString(const char *oemSource, uint32_t oemSourceLen, wchar_t *wideDest, uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_ACP, 0, oemSource, oemSourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_ACP, 0, oemSource, oemSourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}
void fpl_UTF8StringToWideString(const char *utf8Source, uint32_t utf8SourceLen, wchar_t *wideDest, uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}

//
// Window
//
#if FPL_ENABLE_WINDOW
void fpl_WindowFlip() {
	SwapBuffers(fpl_GlobalWin32State_Internal.deviceContext);
}

int32_t fpl_GetWindowWidth() {
	int32_t result = 0;
	RECT windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.windowHandle, &windowRect)) {
		result = (windowRect.right - windowRect.left) + 1;
	}
	return(result);
}
int32_t fpl_GetWindowHeight() {
	int32_t result = 0;
	RECT windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.windowHandle, &windowRect)) {
		result = (windowRect.bottom - windowRect.top) + 1;
	}
	return(result);
}

static void fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType mouseEventType, fpl_MouseButtonType mouseButton, LPARAM lParam, WPARAM wParam) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT(&newEvent);
	newEvent.type = fpl_EventType_Mouse;
	newEvent.mouse.type = mouseEventType;
	newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
	newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
	if (mouseEventType == fpl_MouseEventType_Wheel) {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
	}
	fpl_PushEvent_Internal(fpl_GlobalEventQueue_Internal, &newEvent);
}

static fpl_Key fpl_Win32MapVirtualKey_Internal(uint64_t keyCode) {
	switch (keyCode) {
		case VK_BACK:
			return fpl_Key_Backspace;
		case VK_TAB:
			return fpl_Key_Tab;

		case VK_CLEAR:
			return fpl_Key_Clear;
		case VK_RETURN:
			return fpl_Key_Enter;

		case VK_SHIFT:
			return fpl_Key_Shift;
		case VK_CONTROL:
			return fpl_Key_Control;
		case VK_MENU:
			return fpl_Key_Alt;
		case VK_PAUSE:
			return fpl_Key_Pause;
		case VK_CAPITAL:
			return fpl_Key_CapsLock;

		case VK_ESCAPE:
			return fpl_Key_Escape;
		case VK_SPACE:
			return fpl_Key_Space;
		case VK_PRIOR:
			return fpl_Key_PageUp;
		case VK_NEXT:
			return fpl_Key_PageDown;
		case VK_END:
			return fpl_Key_End;
		case VK_HOME:
			return fpl_Key_Home;
		case VK_LEFT:
			return fpl_Key_Left;
		case VK_UP:
			return fpl_Key_Up;
		case VK_RIGHT:
			return fpl_Key_Right;
		case VK_DOWN:
			return fpl_Key_Down;
		case VK_SELECT:
			return fpl_Key_Select;
		case VK_PRINT:
			return fpl_Key_Print;
		case VK_EXECUTE:
			return fpl_Key_Execute;
		case VK_SNAPSHOT:
			return fpl_Key_Snapshot;
		case VK_INSERT:
			return fpl_Key_Insert;
		case VK_DELETE:
			return fpl_Key_Delete;
		case VK_HELP:
			return fpl_Key_Help;

		case 0x30:
			return fpl_Key_0;
		case 0x31:
			return fpl_Key_1;
		case 0x32:
			return fpl_Key_2;
		case 0x33:
			return fpl_Key_3;
		case 0x34:
			return fpl_Key_4;
		case 0x35:
			return fpl_Key_5;
		case 0x36:
			return fpl_Key_6;
		case 0x37:
			return fpl_Key_7;
		case 0x38:
			return fpl_Key_8;
		case 0x39:
			return fpl_Key_9;

		case 0x41:
			return fpl_Key_A;
		case 0x42:
			return fpl_Key_B;
		case 0x43:
			return fpl_Key_C;
		case 0x44:
			return fpl_Key_D;
		case 0x45:
			return fpl_Key_E;
		case 0x46:
			return fpl_Key_F;
		case 0x47:
			return fpl_Key_G;
		case 0x48:
			return fpl_Key_H;
		case 0x49:
			return fpl_Key_I;
		case 0x4A:
			return fpl_Key_J;
		case 0x4B:
			return fpl_Key_K;
		case 0x4C:
			return fpl_Key_L;
		case 0x4D:
			return fpl_Key_M;
		case 0x4E:
			return fpl_Key_N;
		case 0x4F:
			return fpl_Key_O;
		case 0x50:
			return fpl_Key_P;
		case 0x51:
			return fpl_Key_Q;
		case 0x52:
			return fpl_Key_R;
		case 0x53:
			return fpl_Key_S;
		case 0x54:
			return fpl_Key_T;
		case 0x55:
			return fpl_Key_U;
		case 0x56:
			return fpl_Key_V;
		case 0x57:
			return fpl_Key_W;
		case 0x58:
			return fpl_Key_X;
		case 0x59:
			return fpl_Key_Y;
		case 0x5A:
			return fpl_Key_Z;

		case VK_LWIN:
			return fpl_Key_LeftWin;
		case VK_RWIN:
			return fpl_Key_RightWin;
		case VK_APPS:
			return fpl_Key_Apps;

		case VK_SLEEP:
			return fpl_Key_Sleep;
		case VK_NUMPAD0:
			return fpl_Key_NumPad0;
		case VK_NUMPAD1:
			return fpl_Key_NumPad1;
		case VK_NUMPAD2:
			return fpl_Key_NumPad2;
		case VK_NUMPAD3:
			return fpl_Key_NumPad3;
		case VK_NUMPAD4:
			return fpl_Key_NumPad4;
		case VK_NUMPAD5:
			return fpl_Key_NumPad5;
		case VK_NUMPAD6:
			return fpl_Key_NumPad6;
		case VK_NUMPAD7:
			return fpl_Key_NumPad7;
		case VK_NUMPAD8:
			return fpl_Key_NumPad8;
		case VK_NUMPAD9:
			return fpl_Key_NumPad9;
		case VK_MULTIPLY:
			return fpl_Key_Multiply;
		case VK_ADD:
			return fpl_Key_Add;
		case VK_SEPARATOR:
			return fpl_Key_Separator;
		case VK_SUBTRACT:
			return fpl_Key_Substract;
		case VK_DECIMAL:
			return fpl_Key_Decimal;
		case VK_DIVIDE:
			return fpl_Key_Divide;
		case VK_F1:
			return fpl_Key_F1;
		case VK_F2:
			return fpl_Key_F2;
		case VK_F3:
			return fpl_Key_F3;
		case VK_F4:
			return fpl_Key_F4;
		case VK_F5:
			return fpl_Key_F5;
		case VK_F6:
			return fpl_Key_F6;
		case VK_F7:
			return fpl_Key_F7;
		case VK_F8:
			return fpl_Key_F8;
		case VK_F9:
			return fpl_Key_F9;
		case VK_F10:
			return fpl_Key_F10;
		case VK_F11:
			return fpl_Key_F11;
		case VK_F12:
			return fpl_Key_F12;
		case VK_F13:
			return fpl_Key_F13;
		case VK_F14:
			return fpl_Key_F14;
		case VK_F15:
			return fpl_Key_F15;
		case VK_F16:
			return fpl_Key_F16;
		case VK_F17:
			return fpl_Key_F17;
		case VK_F18:
			return fpl_Key_F18;
		case VK_F19:
			return fpl_Key_F19;
		case VK_F20:
			return fpl_Key_F20;
		case VK_F21:
			return fpl_Key_F21;
		case VK_F22:
			return fpl_Key_F22;
		case VK_F23:
			return fpl_Key_F23;
		case VK_F24:
			return fpl_Key_F24;

		case VK_LSHIFT:
			return fpl_Key_LeftShift;
		case VK_RSHIFT:
			return fpl_Key_RightShift;
		case VK_LCONTROL:
			return fpl_Key_LeftControl;
		case VK_RCONTROL:
			return fpl_Key_RightControl;
		case VK_LMENU:
			return fpl_Key_LeftAlt;
		case VK_RMENU:
			return fpl_Key_RightAlt;

		default:
			return fpl_Key_None;
	}
}

static void fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType keyboardEventType, uint64_t keyCode, fpl_KeyboardModifierType modifiers, bool32 isDown) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT(&newEvent);
	newEvent.type = fpl_EventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = fpl_Win32MapVirtualKey_Internal(keyCode);
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl_PushEvent_Internal(fpl_GlobalEventQueue_Internal, &newEvent);
}

static bool32 fpl_Win32IsKeyDown_Internal(uint64_t keyCode) {
	bool32 result = GetAsyncKeyState((int)keyCode) & 0x8000;
	return(result);
}

LRESULT CALLBACK fpl_Win32MessageProc_Internal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			win32State->isRunning = 0;
		} break;

		case WM_SIZE:
		{
			fpl_Event newEvent;
			FPL_CLEARSTRUCT(&newEvent);
			newEvent.type = fpl_EventType_Window;
			newEvent.window.type = fpl_WindowEventType_Resize;
			newEvent.window.width = LOWORD(lParam);
			newEvent.window.height = HIWORD(lParam);
			fpl_PushEvent_Internal(fpl_GlobalEventQueue_Internal, &newEvent);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint64_t keyCode = wParam;
			bool32 wasDown = ((lParam & (1 << 30)) != 0);
			bool32 isDown = ((lParam & (1 << 31)) == 0);

			bool32 altKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_MENU);
			bool32 shiftKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LSHIFT);
			bool32 ctrlKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LCONTROL);
			bool32 superKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LMENU);

			fpl_KeyboardEventType keyEventType = isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp;
			fpl_KeyboardModifierType modifiers = fpl_KeyboardModifierType_None;
			if (altKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Alt;
			}
			if (shiftKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Shift;
			}
			if (ctrlKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Ctrl;
		}
			if (superKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Super;
			}
			fpl_Win32PushKeyboardEvent_Internal(keyEventType, keyCode, modifiers, isDown);

			if (wasDown != isDown) {
				if (isDown) {
					if (keyCode == VK_F4 && altKeyWasDown) {
						win32State->isRunning = 0;
					}
				}
			}
			result = 1;
	} break;

		case WM_CHAR:
		{
#if 0
			UINT virtualKey = wParam;
			unsigned char keyboardState[256];
			GetKeyboardState(keyboardState);
			WORD resultingChars;
			int resultingCharCount = ToAscii(virtualKey, MapVirtualKeyA(virtualKey, 0), keyboardState, &resultingChars, 0);
			if (resultingCharCount == 1) {
				fpl_KeyboardModifierType modifiers = { 0 };
				uint64_t keyCode = (uint64_t)(resultingChars & 0xFF);
				fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType_Char, keyCode, modifiers, 0);
			}
#else
			// @TODO: Map keycode to fpl owns keycode table - but only for special keys like return and such. So we can map keys platform-indenpendent for libraries like imGUI!
			uint64_t keyCode = wParam;
			fpl_KeyboardModifierType modifiers = { 0 };
			fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType_Char, keyCode, modifiers, 0);
#endif
			result = 1;
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_LBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Left, lParam, wParam);
			result = 1;
		} break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_RBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Right, lParam, wParam);
			result = 1;
		} break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_MBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Middle, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEMOVE:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Move, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEWHEEL:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Wheel, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
}
	return (result);
}

bool32 fpl_WindowUpdate() {
	bool32 result = 0;
#if FPL_ENABLE_WINDOW
	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;
	if (win32State->windowHandle != 0) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			switch (msg.message) {
				case WM_QUIT:
				case WM_CLOSE:
				{
					win32State->isRunning = 0;
				} break;
				default:
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				} break;
			}
		}
		result = win32State->isRunning;
	}
#endif
	return(result);
}

bool32 fpl_IsWindowRunning() {
	bool32 result = fpl_GlobalWin32State_Internal.isRunning;
	return(result);
}

#	if FPL_ENABLE_OPENGL
static bool32 fpl_Win32CreateOpenGL_Internal() {
	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(win32State->deviceContext, &pfd);
	if (pixelFormat == 0) {
		// @TODO: Log error
		return -1;
	}
	if (!SetPixelFormat(win32State->deviceContext, pixelFormat, &pfd)) {
		// @TODO: Log error
		return -1;
	}

	win32State->renderingContext = wglCreateContext(win32State->deviceContext);
	if (!win32State->renderingContext) {
		// @TODO: Log error
		return -1;
	}

	if (!wglMakeCurrent(win32State->deviceContext, win32State->renderingContext)) {
		// @TODO: Log error
		return -1;
	}

	wglSwapIntervalEXT = (wgl_swap_interval *)wglGetProcAddress("wglSwapIntervalEXT");

	return 1;
}
#	endif // FPL_ENABLE_OPENGL

#endif // FPL_ENABLE_WINDOW

bool32 fpl_Init(fpl_InitFlag initFlags) {
	if (initFlags & fpl_InitFlag_VideoOpenGL) {
		initFlags |= fpl_InitFlag_Window;
	}

	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);

	// Timing
	QueryPerformanceFrequency(&win32State->performanceFrequency);

#	if FPL_ENABLE_WINDOW
	if (initFlags & fpl_InitFlag_Window) {
		// Register window class
		WNDCLASSEX windowClass = { 0 };
		windowClass.hInstance = win32State->appInstance;
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.lpszClassName = WIN32_CLASSNAME;
		windowClass.lpfnWndProc = fpl_Win32MessageProc_Internal;
		if (RegisterClassEx(&windowClass) == 0) {
			// @TODO: Log error
			return 0;
		}
		fpl_Win32StringCopy(windowClass.lpszClassName, fpl_Win32GetStringLength(windowClass.lpszClassName), win32State->windowClass, FPL_ARRAYCOUNT(win32State->windowClass));

		// Create window
		win32State->windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, windowClass.lpszClassName, WIN32_UNNAMED_WINDOW, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FPL_DEFAULT_WINDOW_WIDTH, FPL_DEFAULT_WINDOW_HEIGHT, NULL, NULL, windowClass.hInstance, NULL);
		if (win32State->windowHandle == NULL) {
			// @TODO: Log error
			return 0;
		}

		// Get device context so we can swap the back and front buffer
		win32State->deviceContext = GetDC(win32State->windowHandle);
		if (win32State->deviceContext == NULL) {
			// @TODO: Log error
			return 0;
		}

#	if FPL_ENABLE_OPENGL
		// Create opengl rendering context if required
		if (initFlags & fpl_InitFlag_VideoOpenGL) {
			bool32 openglResult = fpl_Win32CreateOpenGL_Internal();
			if (!openglResult) {
				// @TODO: Log error
				return 0;
			}
		}
#	endif // FPL_ENABLE_OPENGL

		void *eventQueueMemory = fpl_AllocateAlignedMemory(sizeof(fpl_EventQueue), 16);
		fpl_GlobalEventQueue_Internal = (fpl_EventQueue *)eventQueueMemory;

		// Show window
		ShowWindow(win32State->windowHandle, SW_SHOW);
		UpdateWindow(win32State->windowHandle);
	}

	win32State->isRunning = 1;
#endif // FPL_ENABLE_WINDOW

	return 1;
}

void fpl_Release() {
	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);

#	if FPL_ENABLE_WINDOW

#		if FPL_ENABLE_OPENGL
	if (win32State->renderingContext) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(win32State->renderingContext);
		win32State->renderingContext = NULL;
	}
#		endif // FPL_ENABLE_OPENGL

	if (win32State->deviceContext != NULL) {
		ReleaseDC(win32State->windowHandle, win32State->deviceContext);
		win32State->deviceContext = NULL;
	}

	if (win32State->windowHandle != NULL) {
		DestroyWindow(win32State->windowHandle);
		win32State->windowHandle = NULL;
		UnregisterClass(win32State->windowClass, win32State->appInstance);
	}

	fpl_FreeAlignedMemory(fpl_GlobalEventQueue_Internal);
#	endif // FPL_ENABLE_WINDOW
}

//
// Entry-Point
//
#if FPL_ENABLE_WINDOW
#if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
#else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
#endif // defined(UNICODE)
	fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO: Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#else
// The main() entry point is used directly
#endif // FPL_ENABLE_WINDOW

#elif defined(FPL_PLATFORM_LINUX) // FPL_PLATFORM_WINDOWS
//
// Linux platform implementation
//
#	error "Please define at least the entry point for the linux platform!"
#elif defined(FPL_PLATFORM_UNIX) // FPL_PLATFORM_LINUX
//
// Unix platform implementation
//
#	error "Please define at least the entry point for the unix platform!"
#else // defined(FPL_PLATFORM_UNIX)
#	error "Unsupported Platform!"
#endif // !defined(FPL_PLATFORM_UNIX)

#endif // defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)