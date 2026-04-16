# Video System Architecture in Final Platform Layer (FPL)

## Overview

The video system in FPL is designed to be modular and extensible, supporting multiple backends such as Software rendering, OpenGL (legacy and modern), and Vulkan. It abstracts platform-specific details and provides a unified interface for video output, window surface management, and frame presentation.

## Architecture

### Backend Abstraction

- Each video backend implements a set of function pointers encapsulated in a `fpl__VideoContext` structure. These functions include:
  - `loadFunc`: Load backend resources and APIs.
  - `unloadFunc`: Unload and clean up backend resources.
  - `initializeFunc`: Initialize the backend with platform and window state.
  - `shutdownFunc`: Shutdown the backend.
  - `prepareWindowFunc`: Prepare the window before video initialization.
  - `finalizeWindowFunc`: Finalize window setup after video initialization.
  - `destroyedWindowFunc`: Handle window destruction.
  - `presentFunc`: Present the rendered frame.
  - `getProcedureFunc`: Retrieve function pointers for backend-specific procedures.
  - `getRequirementsFunc`: Query backend-specific requirements.

- The active backend is stored in a union `fpl__ActiveVideoBackend` that holds backend-specific data structures.

### Platform Integration

- The video system integrates with platform-specific windowing systems (Win32 on Windows, X11 on Linux/Unix).
- Video surfaces hold platform-specific handles such as device contexts, rendering contexts, Vulkan instances, or X11 windows.

### Video Backbuffer

- For software rendering, a backbuffer is allocated and managed, with pixel data accessible for direct manipulation.
- For hardware-accelerated backends (OpenGL, Vulkan), the rendering context and swap buffers are managed internally.

## Adding a New Video Backend: Tutorial Example

This tutorial demonstrates how to add a new video backend to FPL. The example backend will be a simple stub backend named `fplVideoBackendType_Example`.

### Step 1: Preprocessor Setup

#### Naming scheme

Always ensure that the same naming scheme that matches the other backends are used for all defines.

The name of the backend always comes last.

The postfix "BACKEND" is never used in the defines!

FPL_NO_VIDEO_{NAME OF THE NEW BACKEND IN UPPERCASE}
FPL__SUPPORT_VIDEO_{NAME OF THE NEW BACKEND IN UPPERCASE}
FPL__ENABLE_VIDEO_{NAME OF THE NEW BACKEND IN UPPERCASE}

#### Add support define for new audio backend

Search for "// Video preprocessor setup" and include a preprocessor block for the new video backend:

```c
//
// Video preprocessor setup
//
// ...
// FPL_NO_VIDEO_EXAMPLE = Disable example video backend
// ...
#if defined(FPL__SUPPORT_VIDEO)
//  ...
#	if !defined(FPL_NO_VIDEO_EXAMPLE)
#		define FPL__SUPPORT_VIDEO_EXAMPLE
#	endif
//  ...
#endif // FPL__SUPPORT_VIDEO
```

If the new video backend requires includes, that are not guaranteed to be always present, please use `fplHasInclude()` and only set `FPL__SUPPORT_VIDEO_EXAMPLE` on true!

#### Add condition that will remove support define, when windowing support is disabled:

//
// Remove video support when the Window is disabled
//
#if !defined(FPL__SUPPORT_WINDOW)
//  ...
#	if defined(FPL__SUPPORT_VIDEO_EXAMPLE)
#		undef FPL__SUPPORT_VIDEO_EXAMPLE
#	endif
//  ...
#endif // !FPL__SUPPORT_WINDOW

#### Activate enable define, when backend is supported:

Add preprocessor guards to support and enable the backend, allowing disabling via `FPL_NO_XAUDIO`:

```c
//
// Enable supports (FPL uses _ENABLE_ internally only)
//
#if defined(FPL__SUPPORT_VIDEO)
//  ...
#   if defined(FPL__SUPPORT_VIDEO_EXAMPLE)
#		define FPL__ENABLE_VIDEO_EXAMPLE
#	endif
//  ...
#endif // FPL__SUPPORT_VIDEO
```

### Step 2: Define Public Backend Type in enumeration

Add a new enum value in `fplVideoBackendType` as the last actual backend type, but before `fplVideoBackendType_First`.

Change `fplVideoBackendType_Last` to point to `fplVideoBackendType_Example` now.

```c
typedef enum fplVideoBackendType {
    // ...
    fplVideoBackendType_Example,
    // ...
    fplVideoBackendType_Last = fplVideoBackendType_Example,
} fplVideoBackendType;
```

### Step 3: Define Private Backend Data Structures

#### Preprocessor Guard

The private implementation of the entire API is always defined inside the block `FPL__VIDEO_BACKENDS_IMPLEMENTED`

and has its own preprocessor guard that has the following conditions:

- FPL__ENABLE_VIDEO_{OPTIONAL PLATFORM PREFIX}_{BACKEND NAME IN UPPERCASE}
- When it is platform -specific, it must also have a condition of the platform or subplatform define
- It is documented in the example format below
- Endif has a simple comment to indicate the end of the implementation block, because implementation can be rather large

Example:

```c
// ############################################################################
//
// > VIDEO_BACKEND_{OPTIONAL PLATFORM PREFIX}_{BACKEND NAME IN UPPERCASE}
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_OPENGL) && defined(FPL_PLATFORM_WINDOWS)

// Private video backend code goes here

#endif // FPL__ENABLE_VIDEO_OPENGL && FPL_PLATFORM_WINDOWS
```

#### Naming Scheme

All struct/enum types always starts with `fpl__VideoBackend{Optional Platform Prefix in Pascal Case}Name of the Backend in Pascal Case}`.

All preprocessor defines start with `FPL__VIDEO_{OPTIONAL PLATFORM PREFIX}_{NAME OF THE BACKEND IN UPPERCASE}`.

All function pointer defines start with `FPL__FUNC_{OPTIONAL PLATFORM PREFIX}_{NAME OF THE BACKEND IN UPPERCASE}_{Original Function Name}`.

When the backend is only available for a certain platform, e.g. Win32, POSIX, Linux, etc. the Platform prefix must be added before the name of the backend:
`fpl__VideoBackend{Platform Prefix in Pascal Case}{Name of the Backend in Pascal Case}`, e.g. `fpl__VideoBackendWin32XAudio`.

Function pointer defines for the backend must be named accordingly, e.g. `FPL__FUNC_{OPTIONAL PLATFORM PREFIX}_{NAME OF THE BACKEND IN UPPERCASE}_{FUNCTION NAME UPPERCASE}.
The typedef for the function pointer always is based on the function pointer define and is named as lowercase!

#### Add includes, if needed

If the includes are always present on the platform, you can safely add the includes without any trouble.

In case the includes are not always present, it is a better approach to not use any includes at all and use opaque types/pointers instead.

If this is not possible, use fplHasInclude() in the @Preprocessor Setup at the very beginning, then you safely add the includes.

#### Create requires types and defines

Create all required types and defines, if needed - using the naming schemes above.

Opaque/mirrored types are preferred over the real ones!

#### Create a private API structure and function pointer types

Create a backend-specific API structure that stores the handle to the library and the function pointers that are required.

#define FPL__FUNC_WIN32_EXAMPLE_Initialize(name) HRESULT name(uint32_t defaultDevice)
typedef FPL__FUNC_WIN32_EXAMPLE_Initialize(fpl__func_win32_example_initialize;

```c
typedef struct fpl__VideoBackendExampleAPI {
    fplDynamicLibraryHandle handle;
    fpl__func_win32_example_initialize *func_Initialize;
    // ...
} fpl__VideoBackendExampleAPI;
```

#### Loading the API library

Create a private load and unload function, that loads and unloads the dynamic library.

If the backend is platform indendent, use fplDynamicLibraryHandle as a handle to the library and fplGetDynamicLibraryProc to get the address of the procedure.

To make things easier, there are *nasty* macros that can do this for you:

- FPL__AUTO_LOAD_LIBRARY
- FPL__AUTO_UNLOAD_LIBRARY
- FPL__AUTO_GET_FUNCTION_ADDRESS

or, if the library does not allow linking by .lib files or runtime linking is preferred, then use:

- fplDynamicLibraryLoad
- fplDynamicLibraryUnload
- FPL__AUTO_GET_FUNCTION_ADDRESS_CONTINUE
- FPL__AUTO_GET_FUNCTION_ADDRESS_BREAK

In case the library is platform bound, use either:

- FPL__POSIX_LOAD_LIBRARY_BREAK
- FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK
- FPL__POSIX_GET_FUNCTION_ADDRESS_OPTIONAL
- FPL__POSIX_GET_FUNCTION_ADDRESS

or

- FPL__WIN32_LOAD_LIBRARY
- FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK
- FPL__WIN32_LOAD_LIBRARY_BREAK

It is intended to use this inside a loop for testing multiple libraries, if the first one does not work.

#### Create a private fpl__VideoBackend based structure

Create a backend-specific structure extending `fpl__VideoBackend`:

```c
typedef struct fpl__VideoBackendWin32Example {
    // Base backend
    fpl__VideoBackend base;
    // Additional fields must come after fpl__VideoBackend!
    fpl__VideoBackendExampleAPI api;
} fpl__VideoBackendWin32Example;
```

### Step 4: Implement Backend Functions

Implement the required backend functions matching the signatures:

```c
// Very first call that expects to load the video backend API or allocate common resources
#define FPL__FUNC_VIDEO_BACKEND_LOAD(name) bool name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
// Very last call that expects to unload the video backend API or clean common resources
#define FPL__FUNC_VIDEO_BACKEND_UNLOAD(name) void name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
// Initializes the video backend for the specified window and create resources
#define FPL__FUNC_VIDEO_BACKEND_INITIALIZE(name) bool name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, const fplVideoSettings *videoSettings, const fpl__VideoData *data, struct fpl__VideoBackend *backend)
// Prepare the video backend for the window that is going to be created
#define FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(name) bool name(const fpl__PlatformAppState *appState, const fplVideoSettings *videoSettings, fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
// Window is now created, adjust if needed for the video backend, such as creating a rendering context
#define FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(name) bool name(const fpl__PlatformAppState *appState, const fplVideoSettings *videoSettings, fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
// Window is destroyed, if needed you can release additional resources
#define FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(name) void name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
// Shutdowns the video backend, that may release created resources or destroys a rendering context
#define FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(name) void name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
// Displays the current frame to the window, e.g. swap buffers, swap chain present, bit blit, etc.
#define FPL__FUNC_VIDEO_BACKEND_PRESENT(name) void name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, const fpl__VideoData *data, const struct fpl__VideoBackend *backend)
// Gets a procedure address from the backend, if supported
#define FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(name) const void *name(const struct fpl__VideoBackend *backend, const char *procName)
// Gets and fill out the requirements of the backend, if supported
#define FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(name) bool name(fplVideoRequirements *requirements)
```

#### Loading and unloading of the API

```c
fpl_internal fpl__UnloadExampleVideoBackendAPI(fpl__VideoBackendExampleAPI *api) {
    // Unload the module, clear the api struct
}

fpl_internal fpl__LoadExampleVideoBackendAPI(fpl__VideoBackendExampleAPI *api) {
    // Clear the api struct, Load the module, get proc addresses, etc.
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_Example_Load) {
    fpl__VideoBackendExample *nativeBackend = (fpl__VideoBackendExample *)backend;

    // Always clear the full backend struct
    fplClearStruct(nativeBackend);

    // Important: Always set the magic of the video backend, but don't change it to something else
    nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;

    // Load API
    if (!fpl__LoadExampleVideoBackendAPI(&nativeBackend->api)){
        return false;
    }

    return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_Example_Unload) {
    fpl__VideoBackendExample *nativeBackend = (fpl__VideoBackendExample *)backend;

    // Unload API
    fpl__UnloadExampleVideoBackendAPI(&nativeBackend->api);

    // Always clear the full backend struct on unload
    fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_Example_Initialize) {
    // Create resources or a rendering context
    return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_Example_PrepareWindow) {
    // Create additional resources, that does not require any window handle
    return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__VideoBackend_Example_FinalizeWindow) {
    // Window is now created, you can use this to create additional resources or modify the window, if needed
    return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(fpl__VideoBackend_Example_DestroyedWindow) {
    // Window is already destroyed, you can use this to clean up resources - that won't require any window handle
    return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_Example_Shutdown) {
    // Shutdown rendering context or release resources
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_Example_Present) {
    // Present the frame (swap buffers or blit)
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_Example_GetProcedure) {
    // Return function pointers for backend-specific extensions or procedures, when it is supported
    return NULL;
}
```

### Step 4: Construct Video Context

Provide a constructor function returning the `fpl__VideoContext` with function pointers set:

```c
fpl_internal fpl__VideoContext fpl__VideoBackend_Example_Construct(void) {
    fpl__VideoContext result = fpl__StubVideoContext();
    result.loadFunc = fpl__VideoBackend_Example_Load;
    result.unloadFunc = fpl__VideoBackend_Example_Unload;
    result.initializeFunc = fpl__VideoBackend_Example_Initialize;
    result.shutdownFunc = fpl__VideoBackend_Example_Shutdown;
    result.presentFunc = fpl__VideoBackend_Example_Present;
    result.prepareWindowFunc = fpl__VideoBackend_Example_PrepareWindow;
    result.finalizeWindowFunc = fpl__VideoBackend_Example_FinalizeWindow;
    result.destroyedWindowFunc = fpl__VideoBackend_Example_DestroyedWindow;
    return result;
}
```

### Step 5: Integrate Backend Selection

Modify the function `fpl__ConstructVideoContext`, to include a switch-case that calls the new construction function:

```c
fpl_internal fpl__VideoContext fpl__ConstructVideoContext(const fplVideoBackendType backendType) {
    switch (backendType) {
        // Existing backends...
        case fplVideoBackendType_Example:
            return fpl__VideoBackend_Example_Construct();
        // ...
    }
    // Fallback stub
    return fpl__StubVideoContext();
}
```

This tutorial provides a clear path to add a new video backend consistent with existing backends in FPL.