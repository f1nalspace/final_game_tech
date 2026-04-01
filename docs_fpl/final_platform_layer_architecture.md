# Final Platform Layer (FPL) — Architecture Documentation

**File:** `final_platform_layer.h`  
**Version:** 1.0.0  
**Language:** C99 (with optional C++ helpers)  
**Lines:** ~28,400  

This document describes the internal architecture of `final_platform_layer.h` so that AI agents and contributors can understand the codebase well enough to add features or fix bugs without reading the entire file.

---

## Table of Contents

1. [Overview & Design Philosophy](#1-overview--design-philosophy)
2. [File Layout](#2-file-layout)
3. [Platform & Feature Detection](#3-platform--feature-detection)
4. [Core Types & Macros](#4-core-types--macros)
5. [Initialization & Shutdown](#5-initialization--shutdown)
6. [Global State](#6-global-state)
7. [Window System](#7-window-system)
8. [Event System](#8-event-system)
9. [Input System](#9-input-system)
10. [Video System & Backends](#10-video-system--backends)
11. [Audio System & Backends](#11-audio-system--backends)
12. [Threading & Synchronization](#12-threading--synchronization)
13. [File I/O & Paths](#13-file-io--paths)
14. [Memory Management](#14-memory-management)
15. [Atomic Operations](#15-atomic-operations)
16. [CPU & Hardware Queries](#16-cpu--hardware-queries)
17. [Timing & Date/Time](#17-timing--datetime)
18. [Logging](#18-logging)
19. [String Utilities](#19-string-utilities)
20. [How to Add a New Video Backend](#20-how-to-add-a-new-video-backend)
21. [How to Add a New Audio Backend](#21-how-to-add-a-new-audio-backend)
22. [Platform-Specific Code Organization](#22-platform-specific-code-organization)

---

## 1. Overview & Design Philosophy

FPL is a **single-header, cross-platform abstraction library** written in C99. It follows the common single-header pattern: include the header normally for declarations; define `FPL_IMPLEMENTATION` once in exactly one translation unit to get the implementation compiled in.

```c
// In one .c/.cpp file only:
#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

// In all other files:
#include "final_platform_layer.h"
```

Key design rules:
- **No global C runtime dependency** (CRT can be disabled with `FPL_NO_CRT`).
- **No platform headers leak into user code** (unless `FPL_NO_PLATFORM_HEADERS_PROTECTION` is set).
- **Strategy/Bridge pattern** for backends: the public API dispatches through function-pointer tables to platform-specific implementations.
- **Single global app-state singleton** (`fpl__global__AppState`) stores all runtime state.
- **Conditional compilation** everywhere — features are guarded by `FPL__ENABLE_*` macros derived from user-supplied and auto-detected defines.

---

## 2. File Layout

The file is divided into well-commented sections in this order:

| Line Range | Section |
|------------|---------|
| 1–1718 | Header comment, ASCII logo, license, getting-started guide, changelog |
| 1719–2450 | **Platform & compiler detection macros** |
| 2451–3480 | **Internal type helpers**, storage class macros, DLL export macros, API-call macros, opaque platform handle typedefs |
| 3481–8673 | **Public API** — all types, enums, structs, and function declarations visible to users |
| 8674–9343 | **Public audio API** (guarded by `FPL__ENABLE_AUDIO`) |
| 9344–10899 | **Internal implementation helpers** — common utilities, platform-agnostic logic |
| 10900–10936 | **`fpl__PlatformAppState`** definition — the global singleton struct |
| 10937–21335 | **Platform implementations**: POSIX, Linux, X11, Win32 (window, input, video, audio for each) |
| 21336–21435 | **Video backend interface** — function-pointer table definitions and stub implementations |
| 21436–23455 | **Video backend implementations**: Win32 OpenGL, Win32 Software, Win32 Vulkan, X11 OpenGL, X11 Software, X11 Vulkan |
| 23456–23597 | **Audio backend interface** — function-pointer table definitions |
| 23598–25960 | **Audio backend implementations**: DirectSound (Win32), ALSA (Linux) |
| 25961–26400 | **Common audio engine** — device state machine, thread management, format utilities |
| 26401–28172 | **`fplPlatformInit` / `fplPlatformRelease`** orchestration and entry-point macros |
| 28173–28432 | **Entry point helpers** (`FPL_ENTRYPOINT`, `WinMain`, `main` wrappers) |

---

## 3. Platform & Feature Detection

### 3.1 Platform Macros (auto-detected)

| Macro | Meaning |
|-------|---------|
| `FPL_PLATFORM_WINDOWS` | Target is Windows |
| `FPL_PLATFORM_LINUX` | Target is Linux |
| `FPL_PLATFORM_UNIX` | Target is generic Unix |
| `FPL_SUBPLATFORM_POSIX` | POSIX API available (Linux + Unix) |
| `FPL_SUBPLATFORM_X11` | X11 windowing available (Linux) |

### 3.2 Architecture Macros (auto-detected)

`FPL_ARCH_X64`, `FPL_ARCH_X86`, `FPL_ARCH_ARM64`, `FPL_ARCH_ARM32`, `FPL_ARCH_RISCV`, `FPL_ARCH_POWERPC64`, `FPL_ARCH_POWERPC32`, `FPL_ARCH_MIPS`, `FPL_ARCH_SPARC`

`FPL_CPU_64BIT` / `FPL_CPU_32BIT` for pointer width.  
`FPL_CACHELINE_SIZE` — 64 (x86), 128 (ARM64/Apple/PowerPC), 32 (ARM32).

### 3.3 Feature Enable Macros (internal)

These are derived from platform detection + user overrides. They guard all subsystem code:

| Macro | Subsystem |
|-------|-----------|
| `FPL__ENABLE_WINDOW` | Window creation |
| `FPL__ENABLE_VIDEO` | Any video backend |
| `FPL__ENABLE_VIDEO_SOFTWARE` | Software pixel buffer |
| `FPL__ENABLE_VIDEO_OPENGL` | OpenGL context |
| `FPL__ENABLE_VIDEO_VULKAN` | Vulkan surface |
| `FPL__ENABLE_AUDIO` | Any audio backend |
| `FPL__ENABLE_AUDIO_DIRECTSOUND` | DirectSound (Win32) |
| `FPL__ENABLE_AUDIO_ALSA` | ALSA (Linux) |

Users can force-disable features by defining e.g. `FPL_NO_VIDEO_OPENGL` before the include.

---

## 4. Core Types & Macros

### 4.1 Primitive Types

| Type | Meaning |
|------|---------|
| `fpl_b32` | 32-bit boolean (uint32_t under the hood) |
| `fplSeconds` | `double` representing time in seconds |
| `fplMilliseconds` | `uint64_t` representing time in milliseconds |
| `fplTimeoutValue` | `uint32_t` timeout; use `FPL_TIMEOUT_INFINITE` (UINT32_MAX) for blocking |

### 4.2 Key Constants

| Constant | Value / Meaning |
|----------|-----------------|
| `FPL_MAX_NAME_LENGTH` | Maximum string length for names/titles |
| `FPL_MAX_FILENAME_LENGTH` | Maximum file path length |
| `FPL_MAX_KEYBOARD_STATE_COUNT` | 256 — max key states tracked |
| `FPL_MAX_GAMEPAD_STATE_COUNT` | 4 — max simultaneous gamepads |
| `FPL_MAX_AUDIO_CHANNEL_COUNT` | 32 — max audio channels |
| `FPL__MAX_EVENT_COUNT` | 32768 — internal event ring buffer size |

### 4.3 Important Macros

| Macro | Purpose |
|-------|---------|
| `fpl_internal` | `static` — marks a function as translation-unit-private |
| `fpl_global` / `fpl_globalvar` | Marks global variables |
| `fpl_inline` | Inline hint |
| `fpl_null` | Null pointer constant (0 or nullptr in C++) |
| `fplArrayCount(arr)` | Static array element count |
| `fplZeroInit` | Zero-initializer (`{0}`) |
| `fplClearStruct(p)` | `fplMemoryClear(p, sizeof(*p))` |
| `fplStructField(type, field, value)` | Designated initializer helper |
| `FPL_ENUM_AS_FLAGS_OPERATORS(e)` | Adds `\|`, `&`, `~` C++ operators for a flag enum |
| `FPL_ENTRYPOINT` | Provides a `main`/`WinMain` entry point that calls user-defined `fplMain()` |
| `fpl_common_api` | Tags a function that is platform-independent |
| `fpl_platform_api` | Tags a function with a platform-specific implementation |

---

## 5. Initialization & Shutdown

### 5.1 Settings Hierarchy

```
fplSettings
├── fplWindowSettings    — title, size, fullscreen, resizable, icons, callbacks
├── fplVideoSettings     — backend type, vsync, auto-resize, backend-specific config
│   └── fplGraphicsApiSettings (union)
│       ├── fplOpenGLSettings   — version, profile, MSAA, custom driver
│       └── fplVulkanSettings   — API version, validation layers, instance handle
├── fplAudioSettings     — backend type, target format, device, callback, auto-start/stop
│   ├── fplAudioFormat          — sample rate, channels, format type, buffer size
│   └── fplSpecificAudioSettings (union)
│       └── fplAlsaAudioSettings
├── fplInputSettings     — controller detection/update frequency, disable events
├── fplConsoleSettings   — console title
└── fplMemorySettings    — custom allocator callbacks
    ├── fplMemoryAllocationSettings dynamic
    └── fplMemoryAllocationSettings temporary
```

### 5.2 Init Flags

```c
typedef enum fplInitFlags {
    fplInitFlags_None           = 0,
    fplInitFlags_Console        = 1 << 0,   // Console I/O subsystem
    fplInitFlags_Window         = 1 << 1,   // Window creation
    fplInitFlags_Video          = 1 << 2,   // Video backend (implies Window)
    fplInitFlags_Audio          = 1 << 3,   // Audio backend
    fplInitFlags_GameController = 1 << 4,   // Gamepad detection
    fplInitFlags_All            = ...
} fplInitFlags;
```

### 5.3 Initialization Flow (`fplPlatformInit`)

Located near line 26401. The sequence is:

1. Validate not already initialized.
2. Allocate and zero the global `fpl__PlatformAppState` using the memory settings (custom or OS).
3. Initialize the memory subsystem.
4. Initialize the POSIX subplatform (`pthread` library load) if on Linux/Unix.
5. Initialize the X11 subplatform (load `libX11.so`, open display) if windowing on Linux.
6. Initialize console if `fplInitFlags_Console` set.
7. **Initialize window** if `fplInitFlags_Window` or `fplInitFlags_Video` set:
   - Select and prepare the video backend (calls `PrepareWindow` which modifies window creation params).
   - Create the OS window (Win32: `CreateWindowEx`; X11: `XCreateWindow`).
   - Finalize the video backend (calls `FinalizeWindow` to attach rendering context to the window).
8. **Initialize audio** if `fplInitFlags_Audio` set: load and initialize the selected audio backend.
9. **Initialize game controllers** if `fplInitFlags_GameController` set.
10. Store `initSettings`, `initFlags`, set `fpl__global__AppState`.

### 5.4 Shutdown Flow (`fplPlatformRelease`)

Reverse order: stop audio → release audio backend → destroy window (calls `DestroyedWindow` on video backend, then `Shutdown` on video backend) → unload X11 → unload POSIX → free app state memory.

### 5.5 Result Type

```c
typedef enum fplPlatformResultType {
    fplPlatformResultType_Success       =  1,
    fplPlatformResultType_NotInitialized = 0,
    fplPlatformResultType_AlreadyInitialized = -1,
    fplPlatformResultType_OutOfMemory   = -2,
    fplPlatformResultType_FailedPlatform = -3,
    fplPlatformResultType_FailedVideo   = -4,
    fplPlatformResultType_FailedAudio   = -5,
    fplPlatformResultType_FailedWindow  = -6,
} fplPlatformResultType;
```

Retrieve with `fplGetPlatformResult()`. Last error string via `fplGetLastError()`.

---

## 6. Global State

There is a **single global pointer** to the entire runtime state:

```c
fpl_globalvar struct fpl__PlatformAppState *fpl__global__AppState = fpl_null;  // Line 9344
```

`fpl__PlatformAppState` (defined at line 10901) contains:

```c
struct fpl__PlatformAppState {
    // Subplatform state (POSIX, X11)
    fpl__PosixAppState   posix;      // only if FPL_SUBPLATFORM_POSIX
    fpl__X11SubplatformState x11;   // only if FPL_SUBPLATFORM_X11

    // Subsystem state
    fpl__PlatformWindowState  window;  // event queue, key map, window handles
    fpl__PlatformBackendState video;   // opaque memory block for video backend
    fpl__PlatformBackendState audio;   // opaque memory block for audio backend

    // Configuration
    fplSettings initSettings;
    fplSettings currentSettings;
    fplInitFlags initFlags;

    // Platform-specific union
    union {
        fpl__Win32AppState   win32;
        fpl__LinuxAppState   plinux;
        fpl__UnixAppState    plinux;
    };
};
```

`fpl__PlatformBackendState` is a generic memory-block descriptor:
```c
typedef struct {
    void    *mem;              // Heap allocation for the backend struct
    size_t   memSize;          // Total size of that allocation
    size_t   maxBackendSize;   // Size of the largest possible backend (for sizing)
    uintptr_t offsetToBackend; // Byte offset from mem to the actual backend struct
} fpl__PlatformBackendState;
```

This allows backends of varying sizes to be stored in a single opaque allocation without knowing the concrete type at the call site.

---

## 7. Window System

### 7.1 Window Settings

```c
typedef struct fplWindowSettings {
    char title[FPL_MAX_NAME_LENGTH];
    fplImageSource icons[2];           // [0]=small, [1]=large
    fplWindowCallbacks callbacks;      // event + expose callbacks + user data
    fplColor32 background;             // BGRA clear color
    fplWindowSize windowSize;          // { width, height }
    fplWindowSize fullscreenSize;
    uint32_t fullscreenRefreshRate;
    fpl_b32 isResizable;
    fpl_b32 isDecorated;
    fpl_b32 isFloating;
    fpl_b32 isFullscreen;
    fpl_b32 isScreenSaverPrevented;
    fpl_b32 isMonitorPowerPrevented;
} fplWindowSettings;
```

Optional callbacks:
```c
typedef void(fpl_window_event_callback)(const fplEvent *ev, void *userData);
typedef void(fpl_window_exposed_callback)(void *userData);
```

### 7.2 Window Lifecycle

```c
// Typical main loop:
while (fplWindowUpdate()) {        // Returns false when window close is requested
    fplEvent ev;
    while (fplPollEvent(&ev)) {    // Drain the event queue
        switch (ev.type) { ... }
    }
    // Render frame ...
    fplVideoFlip();                // Present the rendered frame
}
```

`fplWindowUpdate()` (platform-specific):
- Processes pending OS messages (Win32: `PeekMessage` loop; X11: `XPending`/`XNextEvent` loop).
- Returns `false` if a close/quit signal was received.

### 7.3 Window State Queries and Mutations

| Function | Description |
|----------|-------------|
| `fplIsWindowRunning()` | Returns true while the window is open |
| `fplWindowShutdown()` | Requests window closure |
| `fplGetWindowSize(out)` | Current client area dimensions |
| `fplSetWindowSize(w, h)` | Resize window |
| `fplGetWindowState()` | `Normal`, `Iconify`, `Maximize`, `Fullscreen` |
| `fplSetWindowState(state)` | Change window state |
| `fplSetWindowFullscreenSize(...)` | Enter fullscreen with specific mode |
| `fplEnableWindowFullscreen()` / `fplDisableWindowFullscreen()` | Toggle fullscreen |
| `fplGetWindowPosition(out)` | Top-left screen position |
| `fplSetWindowPosition(x, y)` | Move window |
| `fplSetWindowTitle(title)` | Change title bar text |
| `fplGetClipboardText(dst, maxLen)` | Read clipboard |
| `fplSetClipboardText(text)` | Write clipboard |
| `fplSetWindowCursorEnabled(bool)` | Show/hide cursor |

### 7.4 Display Management

```c
fplGetDisplayCount()                          → size_t
fplGetDisplays(displays, maxCount)            → size_t
fplGetPrimaryDisplay(outDisplay)              → bool
fplGetWindowDisplay(outDisplay)               → bool
fplGetDisplayFromPosition(x, y, outDisplay)   → bool
fplGetDisplayModes(id, modes, maxCount)        → size_t
```

`fplDisplayInfo` contains: name, display ID, `isPrimary`, virtual rect (x, y, w, h), physical size (mm).  
`fplDisplayMode` contains: width, height, refreshRate, colorFormat.

### 7.5 Internal Window State (`fpl__PlatformWindowState`)

Located near line 10880:
```c
typedef struct {
    fpl__EventQueue eventQueue;           // Ring buffer (32768 events)
    fplKeyboardState keyboardState;       // Current keyboard state snapshot
    uint64_t keyMap[512];                 // Raw keycode → fplKey mapping
    fpl_b32 isRunning;
} fpl__PlatformWindowState;
```

The `fpl__EventQueue` is a lock-free SPSC ring buffer using atomic `pushIndex`/`popIndex`. Platform code pushes events; `fplPollEvent` pops them.

---

## 8. Event System

### 8.1 Event Types

```c
typedef enum fplEventType {
    fplEventType_None     = 0,
    fplEventType_Window,     // Resize, move, close, expose, drop files
    fplEventType_Keyboard,   // Key press/release/repeat, text input
    fplEventType_Mouse,      // Move, button, wheel
    fplEventType_Gamepad,    // Connect, disconnect, state change
} fplEventType;
```

### 8.2 Unified Event Struct

```c
typedef struct fplEvent {
    fplEventType type;
    union {
        fplWindowEvent   window;
        fplKeyboardEvent keyboard;
        fplMouseEvent    mouse;
        fplGamepadEvent  gamepad;
    };
} fplEvent;
```

**`fplWindowEvent`** — type (`fplWindowEventType`): `Resized`, `PositionChanged`, `GotFocus`, `LostFocus`, `Iconified`, `Maximized`, `Restored`, `DropFiles`, `Exposed`; payload union: `fplWindowSize size`, `fplWindowPosition position`, `fplWindowDropFiles dropFiles`.

**`fplKeyboardEvent`** — type (`Button` or `Input`), `keyCode` (raw platform code), `mappedKey` (`fplKey` enum), `modifiers` (`fplKeyboardModifierFlags`), `buttonState` (`Press`/`Release`/`Repeat`).

**`fplMouseEvent`** — type (`Move`, `Button`, `Wheel`), `mouseButton`, `buttonState`, `mouseX`, `mouseY`, `wheelDelta`.

**`fplGamepadEvent`** — type (`Connected`, `Disconnected`, `StateChanged`), `deviceIndex`, `deviceName`, full `fplGamepadState`.

### 8.3 Event Polling

```c
bool fplPollEvent(fplEvent *ev);  // Get next event; returns false when queue empty
void fplPollEvents(void);         // Process all pending OS events and discard them
```

`fplPollEvent` first drains the internal ring buffer. If the ring buffer is empty it may trigger one more OS pump. Use `fplPollEvents` only when you don't need to inspect events.

### 8.4 Internal Event Push Helpers

These are called from platform code (X11 event handler, Win32 `WndProc`, Linux input):

```c
fpl__HandleKeyboardButtonEvent(windowState, time, keyCode, modifiers, state, force)
fpl__HandleKeyboardInputEvent(windowState, keyCode, textCode)
fpl__HandleMouseButtonEvent(windowState, x, y, button, state)
fpl__HandleMouseMoveEvent(windowState, x, y)
fpl__HandleMouseWheelEvent(windowState, x, y, delta)
// + window events: resize, move, focus, drop, etc.
```

---

## 9. Input System

### 9.1 Keyboard

```c
typedef struct fplKeyboardState {
    fplKeyboardModifierFlags modifiers;                            // Current Shift/Ctrl/Alt/Super
    fpl_b32 keyStatesRaw[FPL_MAX_KEYBOARD_STATE_COUNT];           // Raw platform bools
    fplButtonState buttonStatesMapped[FPL_MAX_KEYBOARD_STATE_COUNT]; // Press/Release/Repeat
} fplKeyboardState;

bool fplPollKeyboardState(fplKeyboardState *outState);
```

`fplKey` is a 256-entry enum based on USB HID / Windows VK codes. Key index maps raw OS codes to `fplKey` values via `windowState->keyMap[]`.

### 9.2 Mouse

```c
typedef struct fplMouseState {
    fplButtonState buttonStates[fplMouseButtonType_MaxCount];  // Left, Right, Middle
    int32_t x, y;                                              // Client-area pixel coords
} fplMouseState;

bool fplPollMouseState(fplMouseState *outState);
bool fplQueryCursorPosition(int32_t *outX, int32_t *outY);
```

### 9.3 Gamepad

Up to 4 simultaneous gamepads (`FPL_MAX_GAMEPAD_STATE_COUNT`).

```c
typedef struct fplGamepadState {
    fplGamepadButton buttons[14];          // DPad, ABXY, Start/Back, Shoulders, Thumbs
    float leftStickX, leftStickY;          // -1.0 to +1.0
    float rightStickX, rightStickY;        // -1.0 to +1.0
    float leftTrigger, rightTrigger;       // 0.0 to 1.0
    const char *deviceName;
    fpl_b32 isConnected;
    fpl_b32 isActive;
} fplGamepadState;

bool fplPollGamepadStates(fplGamepadStates *outStates);
```

Gamepad connection events also arrive in `fplEventType_Gamepad`. Platform backends:
- **Win32**: XInput (via runtime-loaded `xinput.dll`).
- **Linux**: `/dev/input/js*` and `/dev/input/event*` via `fpl__LinuxGameControllersState`.

### 9.4 Input Settings

```c
typedef struct fplInputSettings {
    fplGameControllersSettings gameControllers;  // detectionFrequency (ms), updateFrequency (ms)
    fpl_b32 disabledEvents;                      // Stop all input events from being queued
} fplInputSettings;
```

---

## 10. Video System & Backends

### 10.1 Available Backends

```c
typedef enum fplVideoBackendType {
    fplVideoBackendType_None     = 0,
    fplVideoBackendType_Software,   // CPU pixel buffer, no GPU
    fplVideoBackendType_OpenGL,     // OpenGL rendering context
    fplVideoBackendType_Vulkan,     // Vulkan surface
} fplVideoBackendType;
```

Select via `settings.video.backend` before calling `fplPlatformInit`.

### 10.2 Video Settings

```c
typedef struct fplVideoSettings {
    fplGraphicsApiSettings graphics;  // union { fplOpenGLSettings opengl; fplVulkanSettings vulkan; }
    fplVideoBackendType backend;
    fpl_b32 isVSync;
    fpl_b32 isAutoSize;               // Auto-resize backbuffer on window resize
} fplVideoSettings;
```

**OpenGL settings** (`settings.video.graphics.opengl`):
```c
typedef struct fplOpenGLSettings {
    const char *libraryFile;                      // NULL = system default
    fplOpenGLCompabilityFlags compabilityFlags;   // Legacy, Core, Compability, Forward
    uint32_t majorVersion, minorVersion;
    uint8_t multiSamplingCount;                   // MSAA sample count
} fplOpenGLSettings;
```

**Vulkan settings** (`settings.video.graphics.vulkan`):
```c
typedef struct fplVulkanSettings {
    fplVersionInfo appVersion, engineVersion, apiVersion;
    const char *libraryFile;       // NULL = system default
    const char *appName, *engineName;
    void *instanceHandle;          // Pre-existing VkInstance or NULL
    const void *allocator;         // VkAllocationCallbacks or NULL
    fplVulkanValidationLayerCallback *validationLayerCallback;
    void *userData;
    fplVulkanValidationLayerMode validationLayerMode;   // Disabled, Optional, Required
    fplVulkanValidationSeverity validationSeverity;
} fplVulkanSettings;
```

### 10.3 Public Video API

| Function | Description |
|----------|-------------|
| `fplGetVideoBackendType()` | Returns active `fplVideoBackendType` |
| `fplGetVideoBackendName(type)` | Returns display name string |
| `fplVideoFlip()` | Present current frame (swap buffers / BitBlt / present) |
| `fplGetVideoBackBuffer()` | Returns `fplVideoBackBuffer*` (Software backend only) |
| `fplResizeVideoBackBuffer(w, h)` | Resize software back buffer |
| `fplGetVideoProcedure(name)` | `wglGetProcAddress` / `glXGetProcAddress` equivalent |
| `fplGetVideoSurface()` | Returns `const fplVideoSurface*` with native handles |
| `fplGetVideoRequirements(type, out)` | Query instance extensions needed (Vulkan) |

### 10.4 Software Back Buffer

```c
typedef struct fplVideoBackBuffer {
    uint32_t *pixels;       // 32-bit ARGB, top-down scanline order
    uint32_t width, height;
    size_t pixelStride;     // Always 4 (bytes per pixel)
    size_t lineWidth;       // width * pixelStride
    fplVideoRect outputRect;
    fpl_b32 useOutputRect;
} fplVideoBackBuffer;
```

Write pixels directly, then call `fplVideoFlip()`.

### 10.5 Video Surface (native handles)

```c
typedef struct fplVideoSurface {
    union {
        fplVideoSurfaceOpenGL opengl;   // { void *renderingContext }   (HGLRC / GLXContext)
        fplVideoSurfaceVulkan vulkan;   // { void *instance, *surfaceKHR }
    };
    union {
        fplVideoWindowWin32 win32;      // { HWND, HDC }
        fplVideoWindowX11  x11;         // { Window, Display*, Visual*, int screen }
    };
    fplVideoBackendType backendType;
} fplVideoSurface;
```

Access via `fplGetVideoSurface()` when you need to pass native handles to a third-party library.

### 10.6 Internal Video Backend Interface (lines 21339–21435)

Each video backend is implemented as a struct extending `fpl__VideoBackend` (the "base") and registered via a `fpl__VideoContext` function-pointer table:

```c
typedef struct fpl__VideoContext {
    fpl__func_VideoBackendLoad          *loadFunc;          // Load dynamic library / alloc resources
    fpl__func_VideoBackendUnload        *unloadFunc;        // Unload dynamic library / free resources
    fpl__func_VideoBackendPrepareWindow *prepareWindowFunc; // Modify window params before window creation
    fpl__func_VideoBackendFinalizeWindow *finalizeWindowFunc; // Attach context after window is created
    fpl__func_VideoBackendDestroyedWindow *destroyedWindowFunc; // Called when window is destroyed
    fpl__func_VideoBackendInitialize    *initializeFunc;    // Full backend initialization
    fpl__func_VideoBackendShutdown      *shutdownFunc;      // Teardown
    fpl__func_VideoBackendPresent       *presentFunc;       // Swap/present/blit
    fpl__func_VideoBackendGetProcedure  *getProcedureFunc;  // Get function pointer by name
    fpl__func_VideoBackendGetRequirements *getRequirementsFunc; // Query Vulkan extensions etc.
    fpl_b32 recreateOnResize;                               // Should context be recreated on resize?
} fpl__VideoContext;
```

The concrete backend struct starts with the base:
```c
typedef struct fpl__VideoBackend {
    uint64_t magic;          // FPL__VIDEOBACKEND_MAGIC = 0x564944454f535953
    fplVideoSurface surface; // Filled in during FinalizeWindow
} fpl__VideoBackend;

// Example — Win32 OpenGL backend:
typedef struct fpl__VideoBackendWin32OpenGL {
    fpl__VideoBackend base;        // Must be first
    fpl__Win32OpenGLApi openglApi; // Loaded WGL function pointers
    // ... platform-specific fields
} fpl__VideoBackendWin32OpenGL;
```

---

## 11. Audio System & Backends

### 11.1 Available Backends

```c
typedef enum fplAudioBackendType {
    fplAudioBackendType_None        = 0,
    fplAudioBackendType_Auto,           // Auto-select based on platform
    fplAudioBackendType_DirectSound,    // Win32 DirectSound (dsound.dll)
    fplAudioBackendType_Alsa,           // Linux ALSA (libasound.so)
    fplAudioBackendType_Custom,         // User-supplied backend
} fplAudioBackendType;
```

### 11.2 Audio Format

```c
typedef enum fplAudioFormatType {
    fplAudioFormatType_None = 0,
    fplAudioFormatType_U8,    // Unsigned 8-bit PCM
    fplAudioFormatType_S16,   // Signed 16-bit PCM
    fplAudioFormatType_S24,   // Signed 24-bit PCM
    fplAudioFormatType_S32,   // Signed 32-bit PCM
    fplAudioFormatType_S64,   // Signed 64-bit PCM
    fplAudioFormatType_F32,   // IEEE 754 32-bit float
    fplAudioFormatType_F64,   // IEEE 754 64-bit float
} fplAudioFormatType;

typedef struct fplAudioFormat {
    uint32_t sampleRate;                  // Hz; 0 = device default
    uint32_t bufferSizeInFrames;          // 0 = compute from milliseconds
    uint32_t bufferSizeInMilliseconds;    // 0 = device default
    uint16_t channels;                    // 0 = device default
    uint16_t periods;                     // 0 = device default (number of buffer periods)
    fplAudioDefaultFields defaultFields;  // Flag: which fields are defaulted
    fplAudioFormatType type;
    fplAudioChannelLayout channelLayout;  // Mono, Stereo, 5.1, 7.1, etc.
    fplAudioMode mode;                    // Shared_Conservative (default), Exclusive_LowLatency, etc.
} fplAudioFormat;
```

### 11.3 Audio Callback (The Core Pull Model)

FPL uses a **pull / callback model**: the audio backend calls user code to fill buffers.

```c
// Signature:
typedef uint32_t(fpl_audio_client_read_callback)(
    const fplAudioFormat *deviceFormat,  // Actual hardware format negotiated
    const uint32_t frameCount,           // Frames requested
    void *outputSamples,                 // Buffer to fill (interleaved PCM)
    void *userData                       // User-supplied pointer
);

// Registration:
settings.audio.clientReadCallback = MyAudioCallback;
settings.audio.clientUserData     = &myData;

// Or after init:
fplSetAudioClientReadCallback(MyAudioCallback, &myData);
```

The callback runs on a dedicated audio thread (not the main thread). It must not block or allocate.

`deviceFormat->channels` and `deviceFormat->type` describe the interleaved layout the hardware expects. FPL does **not** perform format conversion; the callback must produce samples in the hardware format.

Unused frames at the end of the buffer are automatically zeroed by `fpl__ReadAudioFramesFromClient`.

### 11.4 Audio Settings

```c
typedef struct fplAudioSettings {
    fplAudioFormat targetFormat;          // Requested format; hardware may adjust
    fplAudioDeviceInfo targetDevice;      // Leave zeroed to use default device
    fplSpecificAudioSettings specific;    // Backend-specific (e.g. fplAlsaAudioSettings)
    fpl_audio_client_read_callback *clientReadCallback;
    void *clientUserData;
    fplAudioBackendType backend;
    fpl_b32 startAuto;   // true = start playback immediately on fplPlatformInit
    fpl_b32 stopAuto;    // true = stop playback automatically on fplPlatformRelease
    fpl_b32 manualLoad;  // true = do not auto-init audio; call fplAudioInit() manually
} fplAudioSettings;
```

ALSA-specific:
```c
typedef struct fplAlsaAudioSettings {
    fpl_b32 noMMap;   // Disable memory-mapped I/O (fallback for some drivers)
} fplAlsaAudioSettings;
// Access via: settings.audio.specific.alsa.noMMap
```

### 11.5 Public Audio API

| Function | Description |
|----------|-------------|
| `fplGetAudioBackendType()` | Returns active backend type |
| `fplPlayAudio()` | Start audio playback; returns `fplAudioResultType` |
| `fplStopAudio()` | Stop audio playback |
| `fplAudioInit(settings)` | Manual init (only when `manualLoad = true`) |
| `fplAudioRelease()` | Manual release |
| `fplGetAudioHardwareFormat(out)` | Get negotiated hardware format |
| `fplGetAudioHardwareDevice(out)` | Get active device info |
| `fplGetAudioHardwareDeviceName()` | Get device name string |
| `fplGetAudioChannelMap(out)` | Get speaker channel mapping |
| `fplSetAudioClientReadCallback(cb, ud)` | Replace callback at runtime |
| `fplGetAudioDevices(max, stride, out)` | Enumerate available devices |
| `fplGetAudioDeviceInfo(id, out)` | Get extended info for a device |
| `fplGetAudioBufferSizeInFrames(sr, ms)` | Convert ms to frames |
| `fplGetAudioBufferSizeInMilliseconds(sr, frames)` | Convert frames to ms |
| `fplGetTargetAudioFrameCount(frames, inSR, outSR)` | Resample frame count |
| `fplEncodeAudioFormatU64(sr, ch, type)` | Encode format as uint64 |
| `fplDecodeAudioFormatU64(u64, ...)` | Decode format from uint64 |

### 11.6 Internal Audio Backend Interface (lines 23460–23597)

All audio backends share this function-pointer table:

```c
typedef struct fplAudioBackendFunctionTable {
    fpl_audio_backend_initialize_func        *initialize;        // Load library, alloc backend
    fpl_audio_backend_release_func           *release;           // Unload/free
    fpl_audio_backend_get_audio_devices_func *getAudioDevices;   // Enumerate devices
    fpl_audio_backend_get_audio_device_info_func *getAudioDeviceInfo; // Extended device info
    fpl_audio_backend_initialize_device_func *initializeDevice;  // Open device, negotiate format
    fpl_audio_backend_release_device_func    *releaseDevice;     // Close device
    fpl_audio_backend_start_device_func      *startDevice;       // Begin playback thread
    fpl_audio_backend_stop_device_func       *stopDevice;        // Stop playback thread
    fpl_audio_backend_main_loop_func         *mainLoop;          // Audio thread loop body
    fpl_audio_backend_stop_main_loop_func    *stopMainLoop;      // Signal loop to exit
} fplAudioBackendFunctionTable;
```

This table is registered via a `fplAudioBackendDescriptor`:
```c
typedef struct fplAudioBackendDescriptor {
    fplAudioBackendDescriptorHeader header;   // { idName, type, backendSize, isAsync, isValid }
    fplAudioBackendFunctionTable    table;
} fplAudioBackendDescriptor;
```

The concrete backend data is stored at a fixed offset after the base `fplAudioBackend` struct in the heap allocation, accessed with:
```c
#define FPL_GET_AUDIO_BACKEND_IMPL(backend, type)  \
    (type *)(((uint8_t *)(backend)) + FPL_AUDIO_BACKEND_DATA_OFFSET)
```

### 11.7 Device State Machine

```c
typedef enum fpl__AudioDeviceState {
    fpl__AudioDeviceState_Uninitialized = 0,
    fpl__AudioDeviceState_Stopped,
    fpl__AudioDeviceState_Started,
    fpl__AudioDeviceState_Starting,
    fpl__AudioDeviceState_Stopping,
} fpl__AudioDeviceState;
```

Stored in `fplAudioContext.state` (atomic). Transitions:
- `Uninitialized` → `Stopped` (after `initializeDevice`)
- `Stopped` → `Starting` → `Started` (on `fplPlayAudio`)
- `Started` → `Stopping` → `Stopped` (on `fplStopAudio`)

---

## 12. Threading & Synchronization

### 12.1 Threads

```c
// Create:
fplThreadHandle *fplThreadCreate(fpl_run_thread_callback *runFunc, void *data);
fplThreadHandle *fplThreadCreateWithParameters(const fplThreadParameters *params);

// fplThreadParameters allows specifying stack size and priority
typedef struct fplThreadParameters {
    fpl_run_thread_callback *runFunc;
    void *data;
    size_t stackSize;           // 0 = OS default
    fplThreadPriority priority; // Idle, Lowest, Low, Normal, High, Highest, Realtime
} fplThreadParameters;

// Wait:
bool fplThreadWaitForOne(fplThreadHandle *thread, fplTimeoutValue timeout);
bool fplThreadWaitForAll(fplThreadHandle **threads, size_t count, size_t stride, fplTimeoutValue timeout);
bool fplThreadWaitForAny(fplThreadHandle **threads, size_t count, size_t stride, fplTimeoutValue timeout);

// Misc:
uint32_t fplGetCurrentThreadId();
void     fplThreadSleep(uint32_t milliseconds);
bool     fplThreadYield();
bool     fplThreadTerminate(fplThreadHandle *thread);
```

Implementation: Win32 uses `CreateThread`; POSIX uses `pthread_create`.  
Thread handles are allocated from a fixed pool inside `fpl__PlatformAppState`. `fplGetAvailableThreadCount()` and `fplGetUsedThreadCount()` query pool usage.

### 12.2 Mutex

```c
fplMutexHandle mutex;
fplMutexInit(&mutex);
fplMutexLock(&mutex);
fplMutexTryLock(&mutex);   // Non-blocking attempt
fplMutexUnlock(&mutex);
fplMutexDestroy(&mutex);
```

Win32: `CRITICAL_SECTION`; POSIX: `pthread_mutex_t`.

### 12.3 Signal (Manual-Reset Event)

```c
fplSignalHandle signal;
fplSignalInit(&signal, fplSignalValue_Unsignaled);
fplSignalSet(&signal);                                         // Set to signaled
fplSignalReset(&signal);                                       // Set to unsignaled
fplSignalWaitForOne(&signal, timeout);
fplSignalWaitForAll(signals, count, stride, timeout);
fplSignalWaitForAny(signals, count, stride, timeout);
fplSignalDestroy(&signal);
```

Win32: `CreateEvent(manual=TRUE)`; POSIX: condition variable + mutex pair.

### 12.4 Semaphore

```c
fplSemaphoreHandle sem;
fplSemaphoreInit(&sem, initialValue);
fplSemaphoreWait(&sem, timeout);
fplSemaphoreTryWait(&sem);
int32_t fplSemaphoreValue(&sem);
fplSemaphoreRelease(&sem);
fplSemaphoreDestroy(&sem);
```

Win32: `CreateSemaphore`; POSIX: `sem_t`.

### 12.5 Condition Variable

```c
fplConditionVariable cond;
fplConditionInit(&cond);
fplConditionWait(&cond, &mutex, timeout);
fplConditionSignal(&cond);
fplConditionBroadcast(&cond);
fplConditionDestroy(&cond);
```

Win32: `CONDITION_VARIABLE`; POSIX: `pthread_cond_t`.

---

## 13. File I/O & Paths

### 13.1 File Operations

```c
// Open / Create
bool fplFileOpenBinary(const char *path, fplFileHandle *out);
bool fplFileCreateBinary(const char *path, fplFileHandle *out);
bool fplFileAppendBinary(const char *path, fplFileHandle *out);

// Read / Write (variants for 32-bit, 64-bit, and size_t returns)
size_t   fplFileReadBlock(fplFileHandle *h, size_t size, void *buf, size_t maxBufSize);
size_t   fplFileWriteBlock(fplFileHandle *h, const void *buf, size_t size);

// Seek / Tell
uint64_t fplFileSetPosition64(fplFileHandle *h, int64_t pos, fplFilePositionMode mode);
uint64_t fplFileGetPosition64(fplFileHandle *h);
// fplFilePositionMode: Beginning, Current, End

// Misc
bool     fplFileFlush(fplFileHandle *h);
void     fplFileClose(fplFileHandle *h);
uint64_t fplFileGetSizeFromPath64(const char *path);
uint64_t fplFileGetSizeFromHandle64(fplFileHandle *h);
bool     fplFileGetTimestampsFromPath(const char *path, fplFileTimeStamps *out);
bool     fplFileExists(const char *path);
bool     fplFileCopy(const char *src, const char *dst, bool overwrite);
bool     fplFileMove(const char *src, const char *dst);
bool     fplFileDelete(const char *path);
```

### 13.2 Directory Operations

```c
bool fplDirectoryExists(const char *path);
bool fplDirectoriesCreate(const char *path);    // Creates intermediate directories
bool fplRemoveDirectory(const char *path);

// Directory listing iterator:
fplFileEntry entry;
if (fplDirectoryListBegin(path, filter, &entry)) {
    do {
        // entry.name, entry.type, entry.size, entry.timeStamps, entry.permissions
    } while (fplDirectoryListNext(&entry));
    fplDirectoryListEnd(&entry);
}
```

### 13.3 Path Helpers

```c
size_t fplGetExecutableFilePath(char *dest, size_t maxLen);
size_t fplGetHomePath(char *dest, size_t maxLen);
size_t fplPathNormalize(const char *src, char *dest, size_t maxLen);
size_t fplExtractFilePath(const char *src, char *dest, size_t maxLen);
size_t fplChangeFileExtension(const char *path, const char *ext, char *dest, size_t maxLen);
size_t fplPathCombine(const char *p1, const char *p2, char *dest, size_t maxLen);
```

Path separator is platform-appropriate. Win32: `\`; POSIX: `/`. `fplPathNormalize` normalizes separators.

---

## 14. Memory Management

### 14.1 Standard Allocation

```c
void *fplMemoryAllocate(size_t size);                          // Heap alloc, zero-initialized
void  fplMemoryFree(void *ptr);
void *fplMemoryAlignedAllocate(size_t size, size_t alignment); // Aligned heap alloc
void  fplMemoryAlignedFree(void *ptr);
```

Win32: `HeapAlloc` / `VirtualAlloc`; POSIX: `mmap` or `malloc`.

### 14.2 Memory Utilities

```c
void fplMemoryClear(void *mem, size_t size);                   // memset to 0
void fplMemorySet(void *mem, uint8_t value, size_t size);      // memset to value
void fplMemoryCopy(const void *src, size_t size, void *dst);   // memcpy
bool fplMemoryGetInfos(fplMemoryInfos *out);                   // OS memory stats
```

### 14.3 Custom Allocator

Override the default OS allocator by setting `settings.memory.dynamic` before calling `fplPlatformInit`:

```c
typedef void *(fpl_memory_allocate_callback)(void *userData, size_t size, size_t alignment);
typedef void  (fpl_memory_release_callback)(void *userData, void *ptr);

fplMemoryAllocationSettings s;
s.mode = fplMemoryAllocationMode_Custom;
s.allocateCallback = MyAlloc;
s.releaseCallback  = MyFree;
s.userData         = &myHeap;
settings.memory.dynamic = s;
```

---

## 15. Atomic Operations

Full set of lock-free primitives using compiler intrinsics:

```c
// Fences
fplAtomicReadFence();
fplAtomicWriteFence();
fplAtomicReadWriteFence();

// Exchange (returns old value)
T fplAtomicExchange{U32,U64,S32,S64,Ptr,Size}(volatile T *target, T value);

// Fetch-and-add / Add-and-fetch
T fplAtomicFetchAndAdd{U32,U64,S32,S64,Size,Ptr}(volatile T *dest, T addend);
T fplAtomicAddAndFetch{U32,U64,S32,S64,Size,Ptr}(volatile T *dest, T addend);
T fplAtomicIncrement{U32,U64,S32,S64,Size,Ptr}(volatile T *dest);

// Compare-and-swap (returns old value)
T    fplAtomicCompareAndSwap{U32,U64,S32,S64,Size,Ptr}(volatile T *dest, T cmp, T exchange);
bool fplAtomicIsCompareAndSwap{...}(volatile T *dest, T cmp, T exchange);

// Load / Store
T    fplAtomicLoad{U32,U64,S32,S64,Size,Ptr}(const volatile T *src);
void fplAtomicStore{U32,U64,S32,S64,Size,Ptr}(volatile T *dst, T value);
```

MSVC uses `_Interlocked*` intrinsics; GCC/Clang use `__atomic_*` builtins.

---

## 16. CPU & Hardware Queries

```c
size_t        fplCPUGetCoreCount();
size_t        fplCPUGetName(char *buf, size_t maxLen);   // Returns chars written
bool          fplCPUGetCapabilities(fplCPUCapabilities *out);
fplCPUArchType fplCPUGetArchitecture();
bool          fplCPUID(uint32_t functionId, fplCPUIDLeaf *out);
uint64_t      fplCPUXCR0();
uint64_t      fplCPURDTSC();
```

`fplCPUCapabilities` is a union of `fplX86CPUCapabilities` (SSE, SSE2, AVX, AVX2, AVX512, etc.) and `fplARMCPUCapabilities` (NEON, etc.).

---

## 17. Timing & Date/Time

```c
// High-resolution timer (platform-specific opaque struct)
fplTimestamp ts = fplTimestampQuery();
fplSeconds   elapsed = fplTimestampElapsed(start, finish);  // finish - start in seconds
fplMilliseconds ms = fplMillisecondsQuery();                // Epoch milliseconds

// Date/time
fplDateTime dt = fplDateTimeQuery(fplDateTimeType_Local);   // or fplDateTimeType_UTC
fplDateTimeResult r = fplFormatDateTime(&dt, fplDateTimeType_Local);
// r.year, r.month, r.day, r.hour, r.minute, r.second, r.millisecond
```

Win32: `QueryPerformanceCounter`; POSIX: `clock_gettime(CLOCK_MONOTONIC)`.

---

## 18. Logging

```c
typedef enum fplLogLevel {
    fplLogLevel_Critical = 0,
    fplLogLevel_Error,
    fplLogLevel_Warning,
    fplLogLevel_Info,
    fplLogLevel_Verbose,
    fplLogLevel_Debug,
    fplLogLevel_Trace,
    fplLogLevel_All,
} fplLogLevel;

// Set what level gets written:
void fplSetLogLevel(fplLogLevel level);
fplLogLevel fplGetLogLevel();

// Internal log macros (used inside FPL source):
// FPL_LOG_CRITICAL, FPL_LOG_ERROR, FPL_LOG_WARN, FPL_LOG_INFO, FPL_LOG_DEBUG, FPL_LOG_VERBOSE
```

Custom log writers can be registered via `fplLogWriter`:
```c
typedef struct fplLogWriter {
    void (*writeCallback)(fplLogLevel level, const char *message, void *userData);
    void *userData;
} fplLogWriter;
```

---

## 19. String Utilities

```c
// Comparison / Copy
bool   fplIsStringEqual(const char *a, const char *b);
size_t fplCopyString(const char *src, char *dst, size_t maxDst);
size_t fplCopyStringLen(const char *src, size_t srcLen, char *dst, size_t maxDst);

// Formatting
size_t fplStringFormat(char *dst, size_t maxDst, const char *fmt, ...);
size_t fplStringFormatArgs(char *dst, size_t maxDst, const char *fmt, va_list args);

// Conversion
int64_t fplStringToS64(const char *str, char **endPtr, int base);
// + wide string ↔ UTF-8 conversions for Win32 use
```

---

## 20. How to Add a New Video Backend

Video backends are purely internal (inside `#if defined(FPL_IMPLEMENTATION)`). Adding one requires changes only within `final_platform_layer.h`.

### Step 1 — Add the backend type enum value

At line ~4706, add to `fplVideoBackendType`:
```c
fplVideoBackendType_MyBackend,
// Update fplVideoBackendType_Last if needed
```

### Step 2 — Add settings struct (if needed)

In the `fplGraphicsApiSettings` union (around line 4853), add a new member:
```c
typedef struct fplMyBackendSettings {
    // Backend-specific configuration fields
} fplMyBackendSettings;

// In fplGraphicsApiSettings:
union {
    fplOpenGLSettings opengl;
    fplVulkanSettings vulkan;
    fplMyBackendSettings myBackend;   // <-- new
};
```

Add a corresponding `fplSetDefaultMyBackendSettings()` helper.

### Step 3 — Add a feature guard macro

Near the feature detection block, add:
```c
#if !defined(FPL_NO_VIDEO_MYBACKEND)
#  define FPL__ENABLE_VIDEO_MYBACKEND
#endif
```

### Step 4 — Define the concrete backend struct

Inside `#if defined(FPL__ENABLE_VIDEO_MYBACKEND)`:
```c
typedef struct fpl__VideoBackendMyBackend {
    fpl__VideoBackend base;   // MUST be first — magic + fplVideoSurface
    // ... your platform-specific handles ...
} fpl__VideoBackendMyBackend;
```

### Step 5 — Implement all required functions

Use the macros to define each hook (see the Win32 OpenGL backend at line ~21532 as a reference):

```c
// Load dynamic library / initialize global resources
fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_MyBackend_Load) {
    fpl__VideoBackendMyBackend *impl = (fpl__VideoBackendMyBackend *)backend;
    // Load library, fill impl->...
    return true;
}

// Free dynamic library / global resources
fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_MyBackend_Unload) {
    fpl__VideoBackendMyBackend *impl = (fpl__VideoBackendMyBackend *)backend;
    // Unload library
}

// Called BEFORE window creation — modify windowState to set required pixel format, etc.
fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_MyBackend_PrepareWindow) {
    return true;
}

// Called AFTER window creation — create rendering context and attach to window
fpl_internal FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__VideoBackend_MyBackend_FinalizeWindow) {
    fpl__VideoBackendMyBackend *impl = (fpl__VideoBackendMyBackend *)backend;
    // Create context, fill backend->base.surface.myBackend + backend->base.surface.win32/x11
    return true;
}

// Called when window is being destroyed — release context
fpl_internal FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(fpl__VideoBackend_MyBackend_DestroyedWindow) { }

// Full initialization (may call PrepareWindow + FinalizeWindow internally, or stand-alone)
fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_MyBackend_Initialize) {
    return true;
}

// Full teardown
fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_MyBackend_Shutdown) { }

// Present a frame (swap buffers, bit-blit, etc.)
fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_MyBackend_Present) {
    // Swap/present
}

// Get a procedure by name (like wglGetProcAddress)
fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_MyBackend_GetProcedure) {
    return fpl_null;
}

// Fill out fplVideoRequirements (e.g. required Vulkan extensions)
fpl_internal FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(fpl__VideoBackend_MyBackend_GetRequirements) {
    return false;
}
```

### Step 6 — Register the backend with a Construct function

```c
fpl_internal fpl__VideoContext fpl__VideoBackend_MyBackend_Construct(void) {
    fpl__VideoContext result = fplZeroInit;
    result.loadFunc             = fpl__VideoBackend_MyBackend_Load;
    result.unloadFunc           = fpl__VideoBackend_MyBackend_Unload;
    result.prepareWindowFunc    = fpl__VideoBackend_MyBackend_PrepareWindow;
    result.finalizeWindowFunc   = fpl__VideoBackend_MyBackend_FinalizeWindow;
    result.destroyedWindowFunc  = fpl__VideoBackend_MyBackend_DestroyedWindow;
    result.initializeFunc       = fpl__VideoBackend_MyBackend_Initialize;
    result.shutdownFunc         = fpl__VideoBackend_MyBackend_Shutdown;
    result.presentFunc          = fpl__VideoBackend_MyBackend_Present;
    result.getProcedureFunc     = fpl__VideoBackend_MyBackend_GetProcedure;
    result.getRequirementsFunc  = fpl__VideoBackend_MyBackend_GetRequirements;
    result.recreateOnResize     = false;
    return result;
}
```

### Step 7 — Wire into the backend selection switch

Find the function that selects a video context (search for `fplVideoBackendType_OpenGL` or `fplVideoBackendType_Software` in the init path, around line 26401+). Add a case for your backend:

```c
case fplVideoBackendType_MyBackend:
    *outContext = fpl__VideoBackend_MyBackend_Construct();
    *outBackendSize = sizeof(fpl__VideoBackendMyBackend);
    break;
```

### Step 8 — Add surface type (optional)

If the backend exposes native handles, add a struct to `fplVideoSurface`:
```c
typedef struct fplVideoSurfaceMyBackend {
    void *myHandle;
} fplVideoSurfaceMyBackend;

// In fplVideoSurface union:
fplVideoSurfaceMyBackend myBackend;
```

---

## 21. How to Add a New Audio Backend

Audio backends use the `fplAudioBackendDescriptor` / `fplAudioBackendFunctionTable` pattern. See the ALSA backend (line ~25243) as the canonical reference.

### Step 1 — Add the backend type enum value

At line ~4876, add to `fplAudioBackendType`:
```c
fplAudioBackendType_MyAudioBackend,
// Update fplAudioBackendType_Last if needed
```

### Step 2 — Add a feature guard macro

```c
#if !defined(FPL_NO_AUDIO_MYBACKEND)
#  define FPL__ENABLE_AUDIO_MYBACKEND
#endif
```

Platform-gate it if needed (e.g. `&& defined(FPL_PLATFORM_LINUX)`).

### Step 3 — Add backend-specific settings (if needed)

In `fplSpecificAudioSettings` union and `fplAudioSettings`, add:
```c
typedef struct fplMyAudioBackendSettings {
    // ...
} fplMyAudioBackendSettings;

// In fplSpecificAudioSettings:
union {
    fplAlsaAudioSettings alsa;
    fplMyAudioBackendSettings myBackend;   // <-- new
};
```

### Step 4 — Define the concrete backend data struct

```c
typedef struct fpl__MyAudioBackend {
    // API function pointers (loaded at runtime if using a dynamic library)
    // Device handles, buffer pointers, etc.
    volatile bool stopRequest;
} fpl__MyAudioBackend;
```

Note: The struct is stored at `FPL_AUDIO_BACKEND_DATA_OFFSET` bytes past the `fplAudioBackend` base. Access with:
```c
fpl__MyAudioBackend *impl = FPL_GET_AUDIO_BACKEND_IMPL(backend, fpl__MyAudioBackend);
```

### Step 5 — Implement all required functions

```c
// Load the audio library (e.g. dlopen("libmyaudio.so"))
fpl_internal FPL_AUDIO_BACKEND_INITIALIZE_FUNC(fpl__MyAudioBackendInitialize) {
    // Load library, fill context
    return fplAudioResultType_Success;
}

// Unload the audio library
fpl_internal FPL_AUDIO_BACKEND_RELEASE_FUNC(fpl__MyAudioBackendRelease) {
    fpl__MyAudioBackendReleaseDevice(context, backend);
    // Unload library
    return true;
}

// Enumerate available audio output devices
fpl_internal FPL_AUDIO_BACKEND_GET_AUDIO_DEVICES_FUNC(fpl__MyAudioBackendGetAudioDevices) {
    // Fill deviceInfos[0..min(count,maxDeviceCount)-1]
    return deviceCount;
}

// Get extended info for a specific device ID
fpl_internal FPL_AUDIO_BACKEND_GET_AUDIO_DEVICE_INFO_FUNC(fpl__MyAudioBackendGetAudioDeviceInfo) {
    return fplAudioResultType_Success;
}

// Open device, negotiate format, fill outputFormat / outputDevice / outputChannelMap
fpl_internal FPL_AUDIO_BACKEND_INITIALIZE_DEVICE_FUNC(fpl__MyAudioBackendInitializeDevice) {
    fpl__MyAudioBackend *impl = FPL_GET_AUDIO_BACKEND_IMPL(backend, fpl__MyAudioBackend);
    // Open device, set hardware parameters
    // Copy negotiated format to outputFormat
    // Copy device info to outputDevice
    return fplAudioResultType_Success;
}

// Close the device
fpl_internal FPL_AUDIO_BACKEND_RELEASE_DEVICE_FUNC(fpl__MyAudioBackendReleaseDevice) {
    return true;
}

// Called to start the audio playback thread / DMA
fpl_internal FPL_AUDIO_BACKEND_START_DEVICE_FUNC(fpl__MyAudioBackendStartDevice) {
    return fplAudioResultType_Success;
}

// Called to stop the audio playback thread / DMA
fpl_internal FPL_AUDIO_BACKEND_STOP_DEVICE_FUNC(fpl__MyAudioBackendStopDevice) {
    return true;
}

// The audio main loop — runs on a dedicated audio thread.
// Must call fpl__ReadAudioFramesFromClient(backend, frameCount, buffer) to fill buffers.
fpl_internal FPL_AUDIO_BACKEND_MAIN_LOOP_FUNC(fpl__MyAudioBackendMainLoop) {
    fpl__MyAudioBackend *impl = FPL_GET_AUDIO_BACKEND_IMPL(backend, fpl__MyAudioBackend);
    while (!impl->stopRequest) {
        // Wait for hardware buffer to need filling
        // Determine available frame count
        uint32_t frameCount = ...;
        void *buffer = ...;
        fpl__ReadAudioFramesFromClient(backend, frameCount, buffer);
        // Commit buffer to hardware
    }
}

// Signal the main loop to exit
fpl_internal FPL_AUDIO_BACKEND_STOP_MAIN_LOOP_FUNC(fpl__MyAudioBackendStopMainLoop) {
    fpl__MyAudioBackend *impl = FPL_GET_AUDIO_BACKEND_IMPL(backend, fpl__MyAudioBackend);
    impl->stopRequest = true;
}
```

### Step 6 — Define the backend descriptor

```c
#if defined(FPL__ENABLE_AUDIO_MYBACKEND)
fpl_globalvar fplAudioBackendDescriptor fpl__MyAudioBackendDescriptor = {
    fplStructField(fplAudioBackendDescriptorHeader, idName, {
        fplStructField(fplAudioBackendDescriptorIDName, id, { /* GUID */ }),
        fplStructField(fplAudioBackendDescriptorIDName, name, "MyAudioBackend"),
    }),
    fplStructField(fplAudioBackendDescriptorHeader, type,        fplAudioBackendType_MyAudioBackend),
    fplStructField(fplAudioBackendDescriptorHeader, backendSize, sizeof(fpl__MyAudioBackend)),
    fplStructField(fplAudioBackendDescriptorHeader, isAsync,     false),
    fplStructField(fplAudioBackendDescriptorHeader, isValid,     true),
    // Function table:
    fplStructField(fplAudioBackendFunctionTable, initialize,        fpl__MyAudioBackendInitialize),
    fplStructField(fplAudioBackendFunctionTable, release,           fpl__MyAudioBackendRelease),
    fplStructField(fplAudioBackendFunctionTable, getAudioDevices,   fpl__MyAudioBackendGetAudioDevices),
    fplStructField(fplAudioBackendFunctionTable, getAudioDeviceInfo,fpl__MyAudioBackendGetAudioDeviceInfo),
    fplStructField(fplAudioBackendFunctionTable, initializeDevice,  fpl__MyAudioBackendInitializeDevice),
    fplStructField(fplAudioBackendFunctionTable, releaseDevice,     fpl__MyAudioBackendReleaseDevice),
    fplStructField(fplAudioBackendFunctionTable, startDevice,       fpl__MyAudioBackendStartDevice),
    fplStructField(fplAudioBackendFunctionTable, stopDevice,        fpl__MyAudioBackendStopDevice),
    fplStructField(fplAudioBackendFunctionTable, mainLoop,          fpl__MyAudioBackendMainLoop),
    fplStructField(fplAudioBackendFunctionTable, stopMainLoop,      fpl__MyAudioBackendStopMainLoop),
};
#endif
```

### Step 7 — Register in the backend list

Find the array of `fplAudioBackendDescriptor*` that is iterated during audio init (search for `fpl__DirectSoundDescriptor` or `fpl__AlsaAudioDescriptor`). Add a conditional pointer to your descriptor:

```c
#if defined(FPL__ENABLE_AUDIO_MYBACKEND)
    &fpl__MyAudioBackendDescriptor,
#endif
```

The audio init code iterates this list, matching on `header.type` (or auto-selecting the first valid one when `fplAudioBackendType_Auto` is requested).

---

## 22. Platform-Specific Code Organization

All platform-specific code lives inside `#if defined(FPL_IMPLEMENTATION)` blocks, further gated by platform macros:

```
FPL_PLATFORM_WINDOWS
    fpl__Win32Api          — loaded WinAPI function pointers (User32, GDI, Shell, etc.)
    fpl__Win32AppState     — per-app Win32 state
    fpl__Win32WindowState  — per-window state (HWND, DC, atoms)
    fpl__Win32XInputApi / fpl__Win32XInputState  — gamepad via XInput

FPL_SUBPLATFORM_POSIX
    fpl__PThreadApi        — loaded pthread function pointers
    fpl__PosixAppState     — POSIX thread pool, etc.

FPL_PLATFORM_LINUX
    fpl__LinuxGameController / fpl__LinuxGameControllersState
                           — /dev/input gamepad polling

FPL_SUBPLATFORM_X11
    fpl__X11Api            — loaded X11 function pointers (XOpenDisplay, XCreateWindow, etc.)
    fpl__X11SubplatformState — display, screen, root window
    fpl__X11WindowState    — per-window X11 state + Xdnd atoms
```

### Dynamic Library Loading Pattern

FPL never links directly against platform libraries at link time (where avoidable). Instead it loads them at runtime via `fplDynamicLibraryLoad` / `fplGetDynamicLibraryProc`, storing function pointers in API structs (e.g. `fpl__Win32Api`, `fpl__X11Api`, `fpl__AlsaAudioApi`). This allows:
- Running on systems that may not have certain libraries installed.
- Specifying a custom library path via settings (e.g. `opengl.libraryFile`).

### Adding Support for a New Platform

1. Add detection macro (e.g. `FPL_PLATFORM_MYOS`).
2. Define opaque handle types for the new platform under `#if defined(FPL_PLATFORM_MYOS)`.
3. Add a `fpl__MyOsAppState` struct and add it to the union in `fpl__PlatformAppState`.
4. Add a `fpl__MyOsWindowState` and include it in `fpl__PlatformWindowState` (or in the union).
5. Implement `fplWindowUpdate`, `fplPollEvent`, `fplPollEvents`, all window management functions, and the gamepad polling loop for the new platform.
6. Add video and audio backends for the new platform following Sections 20 and 21.
7. Wire platform init/release into `fplPlatformInit` / `fplPlatformRelease`.
