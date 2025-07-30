# Final Platform Layer (FPL) Library Overview

## Introduction

Final Platform Layer (FPL) is a single-header-file, cross-platform C99/C++11 development library designed to abstract the underlying platform to a simple and easy-to-use API. It provides low-level access to operating system and hardware devices, focusing on rapid game, media, and simulation development. By default, it creates a window, sets up an OpenGL rendering context, and initializes audio playback on supported platforms.

Supported platforms include Windows, Linux, and Unix for architectures x86, x64, and ARM. FPL has minimal dependencies, relying only on built-in OS libraries and a C99 or C++11 compliant compiler. It is released under the MIT License, allowing free use in any software.

## Core Design and Architecture

- **Single-header-file library**: Combines API declarations and implementation in one file, controlled by compiler conditions.
- **Platform and architecture detection**: Uses preprocessor macros to detect platform (Windows, Linux, Unix), subplatforms (POSIX, X11), CPU architecture (x86, x64, ARM), and compiler (MSVC, GCC, Clang, etc.).
- **Subsystems**:
  - Window creation and management
  - Video output with multiple backends (OpenGL legacy/modern, Vulkan, Software)
  - Audio playback with multiple backends (DirectSound, ALSA, Custom)
  - Input handling (keyboard, mouse, gamepad)
  - File and path I/O
  - Threading and synchronization (threads, mutexes, signals, condition variables, semaphores)
  - Memory management (aligned and dynamic allocation)
  - Logging and error handling
  - Hardware and OS information querying
  - String conversions and locale handling

## Platform Abstraction

- **Opaque handles**: Platform-specific handles (e.g., Windows HANDLE, POSIX file descriptors) are abstracted as opaque types.
- **Platform-specific includes and APIs**: Conditional inclusion of platform headers and dynamic loading of system APIs.
- **Subplatforms**: POSIX and X11 subplatforms allow code reuse across Unix-like systems.
- **Memory management**: Uses OS-specific allocation functions (VirtualAlloc on Windows, mmap on POSIX).

## Backend Abstraction

### Video Backends

- Abstracted via a common interface with function pointers for load, unload, initialize, shutdown, prepare window, finalize window, present frame, and get procedure.
- Supported backends:
  - OpenGL (legacy and modern) on Windows and X11
  - Vulkan on Windows and X11
  - Software rendering on Windows and X11
- Video surface structures store backend-specific handles (e.g., OpenGL context, Vulkan instance and surface).

### Audio Backends

- Abstracted via function tables defining initialization, release, device management, playback control, and main loop functions.
- Supported backends:
  - DirectSound (Windows)
  - ALSA (Linux)
  - Custom backends
- Audio formats and devices are represented with detailed structures including channel layouts and formats.
- Audio data is provided via client callbacks for flexible streaming.

## Initialization and Usage

- Initialize platform with `fplPlatformInit()` using flags to specify subsystems (console, window, video, audio, game controllers).
- Configure settings via `fplSettings` structure, with defaults available via `fplSetDefaultSettings()` or `fplMakeDefaultSettings()`.
- Use `fplWindowUpdate()` and `fplPollEvent()` to process window events and input.
- Use `fplVideoFlip()` to present video frames.
- Use `fplPlayAudio()` and `fplStopAudio()` to control audio playback.
- Release resources with `fplPlatformRelease()`.

## Key Features

- **Threading and synchronization**: Cross-platform threads, mutexes, signals, condition variables, semaphores, and atomic operations.
- **Memory management**: Aligned and dynamic memory allocation with optional custom allocators.
- **Logging and error handling**: Configurable logging system with multiple log levels and error tracking.
- **Input handling**: Event-based and polling support for keyboard, mouse, and gamepads.
- **File and path utilities**: Cross-platform file I/O, directory traversal, path manipulation.
- **Hardware and OS info**: CPU capabilities, core count, memory info, OS version, and user session info.
- **Localization**: Locale querying and string conversions (UTF-8 and wide strings).
- **Console support**: Standard input/output and formatted output.
- **Debugging**: Debug break and debug output support.

## Summary

FPL is a lightweight, portable, and comprehensive platform abstraction library tailored for game and media development. Its single-header-file design and minimal dependencies make it easy to integrate and use. It provides unified APIs for essential platform services, multiple backend support for video and audio, and detailed control over system resources.

## License

FPL is licensed under the MIT License, allowing free use in private or commercial software.

---

*Author: Torsten Spaete (Finalspace)*  
*Version: 1.0.0*
