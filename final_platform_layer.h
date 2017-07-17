/*
final_platform_layer.h - FPL -  v0.1
Open-Source Single-File Header-Library by Torsten Spaete

This library is designed to abstract the underlying platform
to a very simple and easy to understand api
without requiring any external dependencies.

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
// Build configuration
//
#if defined(_MSC_VER)
#	if defined(_DEBUG) || (!defined(NDEBUG))
#		define FPL_DEBUG
#	else
#		define FPL_RELEASE
#	endif
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

#ifndef __cplusplus
#	define FPL_STRUCT_TYPE(name) typedef struct name name;
#	define FPL_ENUM_TYPE(name) typedef enum name name;
#else
#	define FPL_STRUCT_TYPE(name)
#	define FPL_ENUM_TYPE(name)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	//
	// API
	//
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

	typedef struct fpl_KeyboardModifierType {
		uint32_t value;
	} fpl_KeyboardModifierType;

	const uint32_t fpl_KeyboardModifierType_Alt = 1 << 0;
	const uint32_t fpl_KeyboardModifierType_Ctrl = 1 << 1;
	const uint32_t fpl_KeyboardModifierType_Shift = 1 << 2;
	const uint32_t fpl_KeyboardModifierType_Super = 1 << 3;

	typedef struct fpl_KeyboardEvent {
		fpl_KeyboardEventType type;
		uint64_t keyCode;
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

	typedef struct fpl_InitFlag {
		uint32_t value;
	} fpl_InitFlag;

	const uint32_t fpl_InitFlag_Window = 1 << 0;
	const uint32_t fpl_InitFlag_VideoOpenGL = 1 << 1;

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
	fpl_api bool32 fpl_Init(struct fpl_InitFlag initFlags);
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
#if defined(FPL_IMPLEMENTATION)

#	if defined(FPL_IMPLEMENTED)
#		error "FPL_IMPLEMENTATION can only be defined once!"
#	else
#		define FPL_IMPLEMENTED

#	if defined(_MSC_VER)
#		include <intrin.h>
#		include <windows.h>
// Inline Atomics for Visual C Compiler
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
#	else
#		error "Implement intrinsics for other platforms/compilers!"
#	endif // defined(_MSC_VER)

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

#define FPL_CLEARSTRUCT(value) \
	fpl_ClearMemory(value, sizeof(*value))

#include <stdio.h>

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WINDOWS)

#	include <windows.h>
#	include <windowsx.h>
#	if FPL_ENABLE_WINDOW
#		pragma comment(linker, "/subsystem:windows")
#		if FPL_ENABLE_OPENGL
#			include <gl\gl.h>
#			pragma comment( lib, "opengl32.lib" )
#		endif
#	endif

#define FPL_WIN32_PATH_SEPARATOR '\\'

//
// Windows platform implementation
//
#if defined(UNICODE)
typedef wchar_t fpl_win32_char;
#else
typedef char fpl_win32_char;
#endif

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

#if FPL_ENABLE_OPENGL && FPL_ENABLE_WINDOW
#	define WGL_SWAP_INTERVAL_FUNCTION(name) BOOL WINAPI name(int value)
typedef WGL_SWAP_INTERVAL_FUNCTION(wgl_swap_interval);
fpl_globalvar wgl_swap_interval *wglSwapIntervalEXT = NULL;
#endif // FPL_ENABLE_OPENGL && FPL_ENABLE_WINDOW

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

void *fpl_AllocateMemory(size_t size) {
	FPL_ASSERT(size > 0);
	void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return(result);
}

void fpl_FreeMemory(void *ptr) {
	FPL_ASSERT(ptr != NULL);
	VirtualFree(ptr, 0, MEM_FREE);
}

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

static void fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType keyboardEventType, uint64_t keyCode, fpl_KeyboardModifierType modifiers, bool32 isDown) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT(&newEvent);
	newEvent.type = fpl_EventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl_PushEvent_Internal(fpl_GlobalEventQueue_Internal, &newEvent);
}

static bool32 fpl_Win32IsKeyDown(uint64_t keyCode) {
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

			bool32 altKeyWasDown = fpl_Win32IsKeyDown(VK_MENU);
			bool32 shiftKeyWasDown = fpl_Win32IsKeyDown(VK_LSHIFT);
			bool32 ctrlKeyWasDown = fpl_Win32IsKeyDown(VK_LCONTROL);
			bool32 superKeyWasDown = fpl_Win32IsKeyDown(VK_LMENU);

			fpl_KeyboardEventType keyEventType = isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp;
			fpl_KeyboardModifierType modifiers = { 0 };
			if (altKeyWasDown) {
				modifiers.value |= fpl_KeyboardModifierType_Alt;
			}
			if (shiftKeyWasDown) {
				modifiers.value |= fpl_KeyboardModifierType_Shift;
			}
			if (ctrlKeyWasDown) {
				modifiers.value |= fpl_KeyboardModifierType_Ctrl;
			}
			if (superKeyWasDown) {
				modifiers.value |= fpl_KeyboardModifierType_Super;
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
			// @TODO: Map keycode to fpl owns keycode table - but only for special keys like return and such.
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

bool32 fpl_Init(struct fpl_InitFlag initFlags) {
	if (initFlags.value & fpl_InitFlag_VideoOpenGL) {
		initFlags.value |= fpl_InitFlag_Window;
	}

	fpl_Win32State *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);

	// Timing
	QueryPerformanceFrequency(&win32State->performanceFrequency);

#	if FPL_ENABLE_WINDOW
	if (initFlags.value & fpl_InitFlag_Window) {
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
		if (initFlags.value & fpl_InitFlag_VideoOpenGL) {
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
#endif // defined(FPL_PLATFORM_xxx)

#endif // defined(FPL_IMPLEMENTED)

#endif // defined(FPL_IMPLEMENTATION)