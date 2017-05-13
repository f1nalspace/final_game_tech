/*
final_platform_layer.h - FPL -  v0.1
Public Domain Single-File Library by Torsten Spaete

This library is designed to abstract the underlying platform
to a very simple and easy to understand api
without requiring any external dependencies.

HOW TO USE:

In one of your C or C++ translation unit define this:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

In the main() entry point initialize the library and use whatever functionality you want and
release the library when you are done.

PREPROCESSOR OVERRIDES:

- FPL_PLATFORM_INIT
- FPL_PLATFORM_DESTROY
- FPL_PLATFORM_UPDATEFRAME

- FPL_DEFAULT_WINDOW_WIDTH
- FPL_DEFAULT_WINDOW_HEIGHT

- FPL_USE_ASSERTIONS 0 or 1
- FPL_USE_CLIB_ASSERTIONS 0 or 1

FEATURES:

[p] Creating a fixed or resizeable window
[x] Handling window, keyboard, mouse events
[ ] Polling gamepad informations

[x] Creating a opengl rendering context
[ ] Support for loading opengl extensions
[ ] Modern opengl 3+ support

[x] Memory allocation and de-allocation with custom alignment support
[x] Atomic operations
[ ] Simple file handling
[ ] Thread, mutex, condition handling

SUPPORTED ARCHITECTURES:

[x] x86
[x] x86_64

SUPPORTED PLATFORMS:

[x] Win32
[ ] Linux (Planned)
[ ] Unix/Posix (Planned)

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
#if !defined(FPL_USE_ASSERTIONS)
#	if defined(FPL_DEBUG)
#		define FPL_USE_ASSERTIONS 1
#	else
#		define FPL_USE_ASSERTIONS 0
#	endif
#endif

//
// Platform includes and compiler hints
//
#if defined(FPL_PLATFORM_WINDOWS)
#	include <windows.h>
#	include <windowsx.h>
#	include <intrin.h>
#	include <gl\gl.h>
#	pragma comment( linker, "/subsystem:windows" )
#	pragma comment( lib, "opengl32.lib" )
#endif

//
// Types
//
#include <stdint.h>
typedef int32_t fpl_b32;

//
// Assertions
//
#if FPL_USE_ASSERTIONS
#if FPL_USE_CLIB_ASSERTIONS
#	include <assert.h>
#	define FPL_Assert(exp) assert(exp)
#	define FPL_StaticAssert(exp) static_assert(exp)
#else
#	define FPL_Assert(exp) if(!(exp)) {*(int *)0 = 0;}
#	define FPL_StaticAssert_(exp, line) \
		int __static_assert_##line(int static_assert_failed[(exp)?1:-1])
#	define FPL_StaticAssert(exp) \
		FPL_StaticAssert_(exp, __LINE__)
#endif
#else
#	define FPL_Assert(exp)
#	define FPL_StaticAssert(exp)
#endif

//
// Static/Extern
//
#define fpl_internal static
#define fpl_global static
#define fpl_inline inline
#if FPL_EXTERN_AS_PRIVATE
#	define fpl_extern fpl_internal
#else
#	define fpl_extern extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

	//
	// API
	//
	typedef struct {
		char windowTitle[128];
		uint32_t windowWidth;
		uint32_t windowHeight;
		fpl_b32 vsyncEnabled;
	} fpl_WindowConfiguration;

	typedef enum {
		fpl_WindowEventType_Resize = 1,
	} fpl_WindowEventType;

	typedef struct {
		fpl_WindowEventType type;
		int32_t width;
		int32_t height;
	} fpl_WindowEvent;

	typedef enum {
		fpl_KeyboardEventType_KeyDown = 1,
		fpl_KeyboardEventType_KeyUp,
	} fpl_KeyboardEventType;

	typedef struct {
		fpl_KeyboardEventType type;
		uint64_t keyCode;
	} fpl_KeyboardEvent;

	typedef enum {
		fpl_MouseEventType_Move = 1,
		fpl_MouseEventType_ButtonDown,
		fpl_MouseEventType_ButtonUp,
		fpl_MouseEventType_Wheel,
	} fpl_MouseEventType;

	typedef enum {
		fpl_MouseButtonType_Left = 1,
		fpl_MouseButtonType_Right,
		fpl_MouseButtonType_Middle,
	} fpl_MouseButtonType;

	typedef struct {
		fpl_MouseEventType type;
		fpl_MouseButtonType mouseButton;
		int32_t mouseX;
		int32_t mouseY;
		float wheelDelta;
		int32_t _padding;
	} fpl_MouseEvent;

	typedef enum {
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

	typedef enum {
		fpl_InitFlag_VideoOpenGL = 1 << 0,
		fpl_InitFlag_All = fpl_InitFlag_VideoOpenGL,
	} fpl_InitFlag;

#define FPL_PLATFORM_INIT_FUNCTION(name) fpl_b32 name(fpl_WindowConfiguration *windowConfiguration)
#define FPL_PLATFORM_DESTROY_FUNCTION(name) void name()
#define FPL_PLATFORM_UPDATEFRAME_FUNCTION(name) void name()

#if defined(_MSC_VER)
	// Inline Atomics for Visual C Compiler
	fpl_inline uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, uint32_t value) {
		uint32_t result = _InterlockedExchange((volatile long *)target, value);
		return (result);
	}
	fpl_inline uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, uint64_t value) {
		uint64_t result = InterlockedExchange64((volatile long long *)target, value);
		return (result);
	}
	fpl_inline uint32_t fpl_AtomicAddU32(volatile uint32_t *value, uint32_t addend) {
		uint32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
		return (result);
	}
	fpl_inline uint64_t fpl_AtomicAddU64(volatile uint64_t *value, uint64_t addend) {
		uint64_t result = InterlockedExchangeAdd64((volatile long long *)value, addend);
		return (result);
	}
	fpl_inline uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, uint32_t exchange, uint32_t comparand) {
		uint32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
		return (result);
	}
	fpl_inline uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, uint64_t exchange, uint64_t comparand) {
		uint64_t result = InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
		return (result);
	}
#endif

#define FPL_ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(FPL_PLATFORM_WINDOWS)
	// @NOTE(final): Required for access "main" from the actual win32 entry point
	fpl_extern int main(int argc, char **args);
#endif

	// Core
	fpl_extern fpl_b32 fpl_Init(fpl_InitFlag initFlags);
	fpl_extern void fpl_Release();
	fpl_extern fpl_b32 fpl_IsRunning();
	fpl_extern fpl_b32 fpl_Update();
	fpl_extern fpl_b32 fpl_PollEvent(struct fpl_Event *event);
	fpl_extern void fpl_Flip();

	// Memory
	fpl_extern void fpl_ClearMemory(void *mem, size_t size);
	fpl_extern void *fpl_AllocateMemory(size_t size);
	fpl_extern void fpl_FreeMemory(void *ptr);
	fpl_extern void *fpl_AllocateAlignedMemory(size_t size, size_t alignment);
	fpl_extern void fpl_FreeAlignedMemory(void *ptr);

	// Timing
	fpl_extern double fpl_GetHighResolutionTimeInSeconds();

	// Strings
	fpl_extern uint32_t __fpl_GetStringLength(const char *str);
	fpl_extern uint32_t __fpl_GetWideStringLength(const wchar_t *str);
	fpl_extern void fpl_CopyString(const char *source, char *dest, uint32_t maxDestLen);
	fpl_extern void fpl_CopyWideString(const wchar_t *source, wchar_t *dest, uint32_t maxDestLen);

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

fpl_extern void *fpl_AllocateAlignedMemory(size_t size, size_t alignment) {
	FPL_Assert(size > 0);
	FPL_Assert((alignment > 0) && !(alignment & (alignment - 1)));

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

fpl_extern void fpl_FreeAlignedMemory(void *ptr) {
	FPL_Assert(ptr != NULL);

	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
	fpl_FreeMemory(basePtr);
}

#define __FPL_MemoryBlockClear(type, mem, size, shift, mask) do { \
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

fpl_extern void fpl_ClearMemory(void *mem, size_t size) {
	// @TODO(final): This can be more efficient: Clear by 8 than 4 than 2 then the rest
	if (size % 8 == 0) {
		__FPL_MemoryBlockClear(uint64_t, mem, size, 3, 0x00000007);
	} else if (size % 4 == 0) {
		__FPL_MemoryBlockClear(uint32_t, mem, size, 2, 0x00000003);
	} else if (size % 2 == 0) {
		__FPL_MemoryBlockClear(uint16_t, mem, size, 1, 0x00000001);
	} else {
		uint8_t *data8 = (uint8_t *)mem;
		uint8_t *data8End = data8 + size;
		while (data8 != data8End) {
			*data8++ = 0;
		}
	}
}

#define FPL_MAX_EVENT_COUNT 4096
typedef struct fpl_EventQueue {
	fpl_Event events[FPL_MAX_EVENT_COUNT];
	volatile uint32_t index;
	volatile uint32_t count;
} fpl_EventQueue;

fpl_global fpl_EventQueue *__fpl_GlobalEventQueue = 0;

fpl_extern fpl_b32 fpl_PollEvent(struct fpl_Event *event) {
	fpl_EventQueue *eventQueue = __fpl_GlobalEventQueue;
	FPL_Assert(eventQueue != NULL);
	if (eventQueue->count > 0 && (eventQueue->index < eventQueue->count)) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->index, 1);
		*event = eventQueue->events[eventIndex];
		return 1;
	} else if (__fpl_GlobalEventQueue->count > 0) {
		fpl_AtomicExchangeU32(&__fpl_GlobalEventQueue->index, 0);
		fpl_AtomicExchangeU32(&__fpl_GlobalEventQueue->count, 0);
	}
	return 0;
}

fpl_internal void __fpl_PushEvent(struct fpl_EventQueue *eventQueue, struct fpl_Event *event) {
	FPL_Assert(eventQueue != NULL);
	uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->count, 1);
	FPL_Assert(eventIndex < FPL_MAX_EVENT_COUNT);
	eventQueue->events[eventIndex] = *event;
}

uint32_t __fpl_GetStringLength(const char *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

uint32_t __fpl_GetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

void fpl_CopyString(const char *source, char *dest, uint32_t maxDestLen) {
	FPL_Assert(source && dest);
	uint32_t sourceLen = __fpl_GetStringLength(source);
	FPL_Assert((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	dest[sourceLen] = 0;
}

void fpl_CopyWideString(const wchar_t *source, wchar_t *dest, uint32_t maxDestLen) {
	FPL_Assert(source && dest);
	uint32_t sourceLen = __fpl_GetWideStringLength(source);
	FPL_Assert((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	dest[sourceLen] = 0;
}

#if defined(FPL_PLATFORM_WINDOWS)

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
	HWND windowHandle;
	fpl_win32_char windowClass[256];
	HDC deviceContext;
	HGLRC renderingContext;
	fpl_b32 isRunning;
} fpl_Win32State;

fpl_global fpl_Win32State __fpl_GlobalWin32State = { 0 };

#define WGL_SWAP_INTERVAL_FUNCTION(name) BOOL WINAPI name(int value)
typedef WGL_SWAP_INTERVAL_FUNCTION(wgl_swap_interval);
fpl_global wgl_swap_interval *wglSwapIntervalEXT = NULL;

#undef WNDCLASSEX
#undef RegisterClassEx
#undef UnregisterClass
#undef CreateWindowEx
#undef DefWindowProc
#undef GetWindowLongPtr
#undef SetWindowLongPtr
#undef DispatchMessage

#if defined(UNICODE)
#define WIN32_CLASSNAME L"FPLWindowClass"
#define WIN32_UNNAMED_WINDOW L"Unnamed FPL Window"
#define WNDCLASSEX WNDCLASSEXW
#define RegisterClassEx RegisterClassExW
#define UnregisterClass UnregisterClassW
#define CreateWindowEx CreateWindowExW
#define DefWindowProc DefWindowProcW
#define GetWindowLongPtr GetWindowLongPtrW
#define SetWindowLongPtr SetWindowLongPtrW
#define PeekMessage PeekMessageW
#define DispatchMessage DispatchMessageW
#define fpl_Win32StringCopy fpl_CopyWideString
#else
#define WIN32_CLASSNAME "FPLWindowClass"
#define WIN32_UNNAMED_WINDOW "Unnamed FPL Window"
#define WNDCLASSEX WNDCLASSEXA
#define RegisterClassEx RegisterClassExA
#define UnregisterClass UnregisterClassA
#define CreateWindowEx CreateWindowExA
#define DefWindowProc DefWindowProcA
#define GetWindowLongPtr GetWindowLongPtrA
#define SetWindowLongPtr SetWindowLongPtrA
#define PeekMessage PeekMessageA
#define DispatchMessage DispatchMessageA
#define fpl_Win32StringCopy fpl_CopyString
#endif // defined(UNICODE)

fpl_inline LARGE_INTEGER __fpl_Win32GetWallClock() {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return(result);
}

void *fpl_AllocateMemory(size_t size) {
	FPL_Assert(size > 0);
	void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return(result);
}

void fpl_FreeMemory(void *ptr) {
	FPL_Assert(ptr != NULL);
	VirtualFree(ptr, 0, MEM_FREE);
}

double fpl_GetHighResolutionTimeInSeconds() {
	LARGE_INTEGER clock = __fpl_Win32GetWallClock();
	double result = clock.QuadPart / (double)__fpl_GlobalWin32State.performanceFrequency.QuadPart;
	return(result);
}

void fpl_Flip() {
	SwapBuffers(__fpl_GlobalWin32State.deviceContext);
}

LRESULT CALLBACK __fpl_Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	fpl_Win32State *win32State = &__fpl_GlobalWin32State;
	FPL_Assert(win32State != NULL);
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
			win32State->isRunning = 0;
			break;
		case WM_SIZE:
		{
			fpl_Event newEvent = { 0 };
			newEvent.type = fpl_EventType_Window;
			newEvent.window.type = fpl_WindowEventType_Resize;
			newEvent.window.width = LOWORD(lParam);
			newEvent.window.height = HIWORD(lParam);
			__fpl_PushEvent(__fpl_GlobalEventQueue, &newEvent);
		}; break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

fpl_internal void __fpl_Win32PushMouseEvent(fpl_MouseEventType mouseEventType, fpl_MouseButtonType mouseButton, LPARAM lParam, WPARAM wParam) {
	fpl_Event newEvent = { 0 };
	newEvent.type = fpl_EventType_Mouse;
	newEvent.mouse.type = mouseEventType;
	newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
	newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
	if (mouseEventType == fpl_MouseEventType_Wheel) {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
	}
	__fpl_PushEvent(__fpl_GlobalEventQueue, &newEvent);
}

fpl_internal void __fpl_Win32PushKeyboardEvent(fpl_KeyboardEventType keyboardEventType, uint64_t keyCode, fpl_b32 isDown) {
	fpl_Event newEvent = { 0 };
	newEvent.type = fpl_EventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.type = isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp;
	__fpl_PushEvent(__fpl_GlobalEventQueue, &newEvent);
}

fpl_extern fpl_b32 fpl_Update() {
	fpl_Win32State *win32State = &__fpl_GlobalWin32State;

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
			case WM_QUIT:
			case WM_CLOSE:
			{
				win32State->isRunning = 0;
			}; break;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				uint64_t keyCode = msg.wParam;
				fpl_b32 altKeyWasDown = (msg.lParam & (1 << 29));
				fpl_b32 wasDown = ((msg.lParam & (1 << 30)) != 0);
				fpl_b32 isDown = ((msg.lParam & (1 << 31)) == 0);

				__fpl_Win32PushKeyboardEvent(isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp, keyCode, isDown);

				if (wasDown != isDown) {
					if (isDown) {
						if (keyCode == VK_F4 && altKeyWasDown) {
							win32State->isRunning = 0;
						}
					}
				}
			}; break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				fpl_MouseEventType mouseEventType;
				if (msg.message == WM_LBUTTONDOWN) {
					mouseEventType = fpl_MouseEventType_ButtonDown;
				} else {
					mouseEventType = fpl_MouseEventType_ButtonUp;
				}
				__fpl_Win32PushMouseEvent(mouseEventType, fpl_MouseButtonType_Left, msg.lParam, msg.wParam);
			};
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			{
				fpl_MouseEventType mouseEventType;
				if (msg.message == WM_RBUTTONDOWN) {
					mouseEventType = fpl_MouseEventType_ButtonDown;
				} else {
					mouseEventType = fpl_MouseEventType_ButtonUp;
				}
				__fpl_Win32PushMouseEvent(mouseEventType, fpl_MouseButtonType_Right, msg.lParam, msg.wParam);
			};
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				fpl_MouseEventType mouseEventType;
				if (msg.message == WM_MBUTTONDOWN) {
					mouseEventType = fpl_MouseEventType_ButtonDown;
				} else {
					mouseEventType = fpl_MouseEventType_ButtonUp;
				}
				__fpl_Win32PushMouseEvent(mouseEventType, fpl_MouseButtonType_Middle, msg.lParam, msg.wParam);
			};
			case WM_MOUSEMOVE:
			{
				__fpl_Win32PushMouseEvent(fpl_MouseEventType_Move, 0, msg.lParam, msg.wParam);
			}; break;
			case WM_MOUSEWHEEL:
			{
				__fpl_Win32PushMouseEvent(fpl_MouseEventType_Wheel, 0, msg.lParam, msg.wParam);
			}; break;
			default:
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}; break;
		}
	}

	return win32State->isRunning;
}

fpl_extern fpl_b32 fpl_IsRunning() {
	fpl_b32 result = __fpl_GlobalWin32State.isRunning;
	return(result);
}

fpl_internal fpl_b32 __fpl_Win32CreateOpenGL() {
	fpl_Win32State *win32State = &__fpl_GlobalWin32State;

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

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	return 1;
}

fpl_extern fpl_b32 fpl_Init(fpl_InitFlag initFlags) {
	fpl_Win32State *win32State = &__fpl_GlobalWin32State;
	FPL_Assert(win32State != NULL);

	// Register window class
	WNDCLASSEX windowClass = { 0 };
	windowClass.hInstance = __fpl_GlobalWin32State.appInstance;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = WIN32_CLASSNAME;
	windowClass.lpfnWndProc = __fpl_Win32MessageProc;
	if (RegisterClassEx(&windowClass) == 0) {
		// @TODO: Log error
		return -1;
	}
	fpl_Win32StringCopy(windowClass.lpszClassName, win32State->windowClass, FPL_ArrayCount(win32State->windowClass));

	// Create window
	win32State->windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, windowClass.lpszClassName, WIN32_UNNAMED_WINDOW, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FPL_DEFAULT_WINDOW_WIDTH, FPL_DEFAULT_WINDOW_HEIGHT, NULL, NULL, windowClass.hInstance, NULL);
	if (win32State->windowHandle == NULL) {
		// @TODO: Log error
		return -1;
	}

	// Get device context so we can swap the back and front buffer
	win32State->deviceContext = GetDC(win32State->windowHandle);
	if (win32State->deviceContext == NULL) {
		// @TODO: Log error
		return -1;
	}

	// Create opengl rendering context if required
	if (initFlags & fpl_InitFlag_VideoOpenGL) {
		fpl_b32 openglResult = __fpl_Win32CreateOpenGL();
		if (!openglResult) {
			// @TODO: Log error
			return -1;
		}
	}

	void *eventQueueMemory = fpl_AllocateAlignedMemory(sizeof(fpl_EventQueue), 16);
	__fpl_GlobalEventQueue = (fpl_EventQueue *)eventQueueMemory;


	// Show window
	ShowWindow(win32State->windowHandle, SW_SHOW);
	UpdateWindow(win32State->windowHandle);

	win32State->isRunning = 1;

	return 1;
}

fpl_extern void fpl_Release() {
	fpl_Win32State *win32State = &__fpl_GlobalWin32State;
	FPL_Assert(win32State != NULL);

	fpl_FreeAlignedMemory(__fpl_GlobalEventQueue);

#if FPL_ENABLE_OPENGL
	if (win32State->renderingContext) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(win32State->renderingContext);
		win32State->renderingContext = NULL;

	}
#endif

	if (win32State->deviceContext != NULL) {
		ReleaseDC(win32State->windowHandle, win32State->deviceContext);
		win32State->deviceContext = NULL;
	}

	if (win32State->windowHandle != NULL) {
		DestroyWindow(win32State->windowHandle);
		win32State->windowHandle = NULL;
		UnregisterClass(win32State->windowClass, win32State->appInstance);
	}
}

#if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
#else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
#endif

	__fpl_GlobalWin32State.appInstance = appInstance;
	QueryPerformanceFrequency(&__fpl_GlobalWin32State.performanceFrequency);

	// @TODO: Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#elif defined(FPL_PLATFORM_LINUX) // FPL_PLATFORM_WINDOWS
int main(int argc, char **args) {

}
//
// Linux platform implementation
//
#	error "Please define at least the entry point for the linux platform!"
#elif defined(FPL_PLATFORM_UNIX) // FPL_PLATFORM_LINUX
int main(int argc, char **args) {

}
//
// Unix platform implementation
//
#	error "Please define at least the entry point for the unix platform!"
#else // defined(FPL_PLATFORM_UNIX)
#	error "Unsupported Platform!"
#endif // defined(FPL_PLATFORM_xxx)

#endif // defined(FPL_IMPLEMENTED)

#endif // defined(FPL_IMPLEMENTATION)