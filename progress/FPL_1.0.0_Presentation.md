# Final Platform Layer (FPL) — 1.0.0 Release Presentation

> A C99 Single-Header-File Platform Abstraction Library for game, media, and simulation development.
>
> **Author:** Torsten Spaete (Finalspace) — 30+ years of programming experience
> **License:** MIT
> **Repository:** https://github.com/f1nalspace/final_game_tech
> **Release:** 1.0.0 — First stable release (2026)

This document is structured as a video/presentation script. Each chapter is a scene with a clear hook, talking points, on-screen text, and B-roll suggestions. A video generator or a human narrator can read it top-to-bottom.

---

## Scene 1 — Title / Hook (00:00 – 00:20)

**On-screen title:**
> **Final Platform Layer 1.0.0**
> *One Header. Every Platform. No Dependencies.*

**Narration:**
> "What if you could build a cross-platform game, emulator, or audio app — with windowing, OpenGL, Vulkan, multi-backend audio, gamepads, threading, and file IO — and ship it as **one single C header file**, with **zero third-party dependencies**?
>
> That's Final Platform Layer. After nine years of development, version 1.0.0 is finally here."

**B-roll:** FPL logo, fast-cut montage of all the demos (AudioPlayer, FFMpeg, Emulator, NBody, Crackout, Towadev, ImGui, Raytracer).

---

## Scene 2 — What is FPL? (00:20 – 01:00)

**On-screen bullets:**
- Single-header C99 / C++11 library
- Cross-platform: Windows, Linux, Unix/BSD, Raspberry Pi
- Architectures: x86, x64, ARM32, ARM64
- Focus: games, multimedia, simulation
- MIT licensed

**Narration:**
> "FPL is a single-header platform abstraction library. You drop **`final_platform_layer.h`** into your project, define `FPL_IMPLEMENTATION` in one translation unit, and you instantly get low-level access to a window, an OpenGL or Vulkan rendering context, audio playback, input devices, file IO, threads, memory, and hardware queries.
>
> It is written in C99 for maximum portability — and it is 100% C++ compatible. The **only** dependencies are the built-in operating system libraries and a C99-compliant compiler. No external libraries. No build system. No package manager."

---

## Scene 3 — The Problem It Solves (01:00 – 01:45)

**On-screen comparison table:**

| Concern                   | Typical Stack (SDL/GLFW/etc.) | FPL                          |
|---------------------------|-------------------------------|------------------------------|
| Files to integrate        | Library + headers + binaries  | **1 header file**            |
| Build system              | CMake / Meson / vcpkg         | **None required**            |
| Linking                   | Static or dynamic libs        | **Runtime-linked by default**|
| Third-party deps          | Many                          | **Zero**                     |
| Compile time              | Slow                          | **Blazingly fast**           |
| Hidden internal state     | Yes                           | **Nothing hidden**           |

**Narration:**
> "Traditional cross-platform libraries pull you into a dependency rabbit hole. SDL needs SDL. GLFW needs GLFW. Both need a build system, linker flags, and shipped binaries.
>
> FPL is different. It runtime-links every platform library — Win32, X11, ALSA, PulseAudio, PipeWire, OpenGL, Vulkan — at startup. No `.lib`. No `.so`. No CMake. You just compile your single C file and ship the executable."

---

## Scene 4 — Core Feature Set (01:45 – 02:45)

**On-screen feature grid:**

**Windowing & Display**
- Single-window creation and management
- Fullscreen, resizable, borderless, floating
- Multi-monitor / display enumeration
- Clipboard, cursor, window state queries
- Headless / no-window mode for input-only apps

**Video Backends**
- Software (raw pixel framebuffer)
- OpenGL — Legacy & Modern Core profile
- Vulkan
- Planned: EGL

**Audio Backends**
- Windows: DirectSound, WASAPI (XAudio planned)
- Linux: ALSA, PulseAudio, PipeWire
- Unix/BSD: OSS
- Async streaming with callback-driven mixing

**Input**
- Keyboard, mouse — polling **or** event-based
- Gamepads: XInput, DirectInput, Linux Joystick (`/dev/input/jsX`)
- SDL-compatible gamepad mapping system with resolver callbacks
- Multi-backend polling and merging

**Threading**
- Threads, atomics, mutexes, semaphores, signals, condition variables
- Lock-free error reporting and event queue

**File / Path / IO**
- Files, directories, paths, append-binary, normalization
- Safe size queries, safe string-to-int parsing
- UTF-8 ↔ wide-string conversion

**System Queries**
- CPU info, core count, cache-line size
- Memory usage (sysinfo, /proc/meminfo, sysctl)
- Date/time with UTC + local, thread-safe
- OS version info, locale (system / user / input)

**Narration:**
> "FPL is not minimal in **what** it does — it is minimal in **what it depends on**. The 1.0 feature set covers everything a game or media application typically needs from the operating system."

---

## Scene 5 — Hello World (02:45 – 03:15)

**On-screen code:**
```c
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
    if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
        fplConsoleOut("Hello World!");
        fplPlatformRelease();
        return 0;
    }
    return -1;
}
```

**Narration:**
> "This is a fully cross-platform program. Compiles on Windows with MSVC, on Linux with GCC, on FreeBSD with Clang, on a Raspberry Pi. No project file. No dependencies. Just `cc hello.c`."

---

## Scene 6 — OpenGL Application (03:15 – 04:15)

**On-screen code:**
```c
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
    fplSettings settings = fplMakeDefaultSettings();
    settings.video.backend = fplVideoBackendType_OpenGL;

    // Modern OpenGL 3.3 Core
    settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Core;
    settings.video.graphics.opengl.majorVersion = 3;
    settings.video.graphics.opengl.minorVersion = 3;

    if (fplPlatformInit(fplInitFlags_Video, &settings)) {
        while (fplWindowUpdate()) {
            fplEvent ev;
            while (fplPollEvent(&ev)) {
                // handle input
            }

            // your render code here

            fplVideoFlip();
        }
        fplPlatformRelease();
        return 0;
    }
    return -1;
}
```

**Narration:**
> "That is the entire boilerplate for an OpenGL window, with input polling and swap-buffer presentation. Switch `fplVideoBackendType_OpenGL` to `fplVideoBackendType_Vulkan` and you get a Vulkan surface instead. Switch to `fplVideoBackendType_Software` and you get a raw pixel framebuffer.
>
> The settings struct gives you fine-grained control of every subsystem at startup — but the defaults are sensible enough that you usually never touch them."

---

## Scene 7 — Advantages (04:15 – 05:15)

**On-screen bullets, one at a time:**

1. **One file.** Drop `final_platform_layer.h` in, define `FPL_IMPLEMENTATION`, compile. Done.
2. **Zero third-party dependencies.** Only the OS libraries that already exist on the user's machine.
3. **Runtime linking by default.** No import libraries. No DLL hell. FPL loads what it needs, when it needs it.
4. **Blazingly fast compile times.** A small, focused, header-only design — measured in seconds, not minutes.
5. **Fixed, small memory footprint.** No hidden growing buffers, no surprise allocations.
6. **Detailed startup configuration.** A single `fplSettings` struct controls every subsystem.
7. **Hides nothing.** Every internal handle is accessible — opaque types still expose their real underlying handle (HWND, Display*, pthread_t).
8. **Integration on your terms.** Header-only, statically linked, dynamically linked, or with full source — your choice.
9. **MIT licensed.** Ship it in commercial, closed-source, or open-source projects.
10. **Pure C99.** Works in C and C++ projects without wrappers.

**Narration:**
> "These are not marketing points. They are the design constraints FPL was built under. Every release breaks against them, not against feature wishlists."

---

## Scene 8 — Disadvantages & Honest Trade-offs (05:15 – 06:15)

**On-screen bullets:**

- **No macOS support.** Apple's deprecation of OpenGL and the cost of a separate Objective-C / Cocoa backend put it out of scope. Linux and BSD are first-class instead.
- **Single window only.** FPL deliberately does not support multiple windows. If your application needs multi-window, FPL is not the right tool.
- **No high-level rendering.** FPL gives you a context — not a sprite batcher, not a scene graph. You bring your own renderer.
- **No asset pipeline.** No image loader, no model loader, no font rasterizer. The repo ships optional helpers (in `demos/additions/`), but the core library is platform-only.
- **No networking.** Sockets are out of scope.
- **Wayland not yet supported** — X11 only on Linux for now (planned).
- **Some backends still partial:** XAudio, RawInput, EGL, EvDev/udev are planned but not yet implemented.
- **Single-author project.** Maintained primarily by one person — though AI-assisted development has accelerated coverage of new backends (PulseAudio, PipeWire) and major bug fixes.

**Narration:**
> "FPL is not trying to replace Unreal, Unity, or even SDL. It is the platform layer underneath your engine — the part you would otherwise write yourself in a few thousand lines per OS. By being deliberately small, it stays maintainable, fast, and honest."

---

## Scene 9 — Supported Matrix (06:15 – 06:45)

**On-screen matrix:**

**Operating Systems**
- ✅ Windows (Win32)
- ✅ Linux
- ✅ Raspberry Pi
- ✅ Unix / BSD (FreeBSD, NetBSD, OpenBSD, DragonFly)
- 📝 Wayland (planned)
- ❌ macOS

**Architectures**
- ✅ x86 (32-bit)
- ✅ x64 (64-bit)
- ✔️ ARM32
- ✔️ ARM64

**Compilers**
- ✅ MSVC
- ✅ GCC
- ✅ Clang
- ✔️ MinGW
- ✔️ Intel

**Video Backends**
- ✅ Software (Win32 / X11)
- ✅ OpenGL (Win32 / X11)
- ✅ Vulkan (Win32 / X11)
- 📝 EGL (X11)

**Audio Backends**
- ✅ DirectSound (Win32)
- ✅ WASAPI (Win32)
- ✅ ALSA (Linux)
- ✅ PulseAudio (Linux)
- ✅ PipeWire (Linux)
- ✅ OSS (Linux / Unix)
- 📝 XAudio (Win32)

**Input Backends**
- ✅ Win32 API (Keyboard / Mouse)
- ✅ XLib (Keyboard / Mouse)
- ✅ XInput (Gamepad)
- ✅ DirectInput (Gamepad)
- ✅ Linux Joystick `/dev/input/jsX` (Gamepad)
- 📝 RawInput (Keyboard / Mouse)
- 📝 EvDev / udev

**Legend:** ✅ Fully supported · ✔️ Partial or untested · 📝 Planned · ❌ Not supported

---

## Scene 10 — Use Cases (06:45 – 07:45)

**On-screen, with B-roll of each matching demo:**

### Games
- **FPL_Crackout** — Breakout-like with 2D rigid-body physics (Box2D single-header conversion), random level generation, music + audio streaming, seamless keyboard/gamepad input.
- **FPL_Towadev** — Tower-defense game with projectile prediction and time scaling.

### Simulation / Emulation
- **FPL_Emulator** — Game Boy DMG/CGB emulator with debugger, background-map + tile-map visualization, palette swap, ZIP/ROM drag-and-drop, custom immediate UI. Built on `final_game_box.h`.
- **FPL_NBodySimulation** — 2D N-body fluid simulation with multithreaded kernel, loadable scenarios, static collision, fluid emitter, built-in benchmark.
- **FPL_Raytracer** — Multithreaded CPU ray-tracer with software rendering.

### Multimedia
- **FPL_AudioPlayer** — Visual audio player with MP3 / OGG / WAV ring-buffer streaming, resampling, format conversion, channel up/down mixing, waveform + FFT + spectrum visualization.
- **FPL_FFMpeg** — Full FFmpeg video player (C++), multithreaded packet/frame decoding, color-conversion shaders, pause/seek, OSD — using runtime-linked FFmpeg.
- **FPL_ImageViewer** — Image viewer with GLSL scaling/filtering and parallel image loading (STB_Image backend).

### Examples & Integrations
- **FPL_OpenGL** — Legacy and modern OpenGL, runtime linking, GLSL shaders.
- **FPL_Software** — Pure software rendering with backbuffer rescaling.
- **FPL_ImGui** — Full Dear ImGui integration with device/display enumeration.
- **FTT_TileTracingDemo** — Tile-map contour tracing using `final_tiletrace.hpp`.

**Narration:**
> "Every one of these demos lives in the same repository — and every single one is a single translation unit pulling in `final_platform_layer.h`. No build system. No dependency manager. Just `cc demo.c`."

---

## Scene 11 — What's New in 1.0.0 (07:45 – 08:45)

**On-screen, grouped:**

### New Subsystems
- New **input backend system** decoupled from windowing, with multi-backend polling and merging
- New **SDL-compatible gamepad mapping** system with resolver callbacks and device enumeration
- New **DirectInput** backend on Windows; improved `/dev/input/jsX` backend on Linux
- New **PulseAudio**, **PipeWire**, and **OSS** audio backends (OSS covers FreeBSD / NetBSD / OpenBSD / DragonFly)
- New **WASAPI** audio backend
- New **date/time API** with UTC / local support and thread-safe queries
- New **headless / no-window** event pump for input-only applications

### Reliability
- **Thread-safe error reporting** (lock-free)
- **Lock-free window event queue**
- **Compile-time size checks** for opaque X11 / Win32 / POSIX handles
- Numerous ABI, runtime-linking, and platform-specific bugfixes across X11, ALSA, OpenGL, Vulkan

### File / Path / String
- New `fplFileAppendBinary()` for log files
- New `fplPathNormalize()` for absolute-path normalization
- New `fplTryStringToS32` / `fplTryStringToS32Len` for safe parsing
- Unified buffer contract: query-mode, NUL-termination, required-size return

### X11 / POSIX
- Implemented missing X11 surface APIs (display, clipboard, fullscreen rect, window state, resizable/floating, cursor query)
- Better BSD / macOS version detection, thread-safe date/time, separator-tolerant directory creation

### Breaking Changes (highlights)
- Several string / path / window-title APIs now **return the required size** instead of a pointer
- `fplInitFlags_GameController` → `fplInitFlags_Gamepad`
- `fplMemoryGetInfos` → `fplMemoryGetUsage`
- `fplGetCurrentThreadId()` and `fplThreadHandle.id` widened to `uint64_t` to portably hold `pthread_t`
- Removed `fplAudioDeviceInfoExtended` (rolled into `fplAudioDeviceInfo`)
- Removed `fplAudioBackendType_Custom`

**Narration:**
> "1.0 is not a sprint to the finish line. It is the version where every API, every backend, every buffer contract has been normalized — and committed to. The breaking changes happened so that there will not be more of them."

---

## Scene 12 — The Wider Family of Libraries (08:45 – 09:30)

**On-screen table:**

| Library                  | Description                                | Language | Status   |
|--------------------------|--------------------------------------------|----------|----------|
| `final_platform_layer.h` | Cross-platform abstraction (this)          | C99      | 1.0.0    |
| `final_dynamic_opengl.h` | Single-file OpenGL loader                  | C99      | 1.0.1    |
| `final_game_box.h`       | Game Boy DMG / CGB emulator                | C99      | 1.3.0    |
| `final_memory.h`         | Custom heap allocator                      | C99      | 1.0.1    |
| `final_xml.h`            | XML parser                                 | C99      | 0.3.1-α  |
| `final_tiletrace.hpp`    | Tile-map contour tracing                   | C++11    | 1.02     |

**Narration:**
> "FPL is the centerpiece of a family of focused single-header libraries — an OpenGL loader, a Game Boy emulator, a memory allocator, an XML parser, a tile-trace utility. They follow the same philosophy: small, single-file, no dependencies, MIT licensed."

---

## Scene 13 — Getting Started (09:30 – 10:00)

**On-screen, step-by-step:**

1. **Download** the latest `final_platform_layer.h` from the repository.
2. **Drop** it into your C or C++ project.
3. In **exactly one** translation unit, write:
   ```c
   #define FPL_IMPLEMENTATION
   #include <final_platform_layer.h>
   ```
4. Anywhere else, just `#include <final_platform_layer.h>`.
5. Call `fplPlatformInit()` at startup, `fplPlatformRelease()` at shutdown.

**Compile commands:**
- Linux: `gcc main.c -o app -ldl -lm`
- Windows (MSVC): `cl main.c`
- FreeBSD: `cc main.c -o app -ldl -lm -lpthread`

**Narration:**
> "That's it. Five lines of integration. Two function calls. One platform layer."

---

## Scene 14 — Who Is It For? (10:00 – 10:30)

**On-screen audience cards:**

- **Hobby game developers** who hate fighting build systems.
- **Engine writers** who want a clean, hideable platform abstraction layer.
- **Emulator and tool authors** who need windowing, audio, and gamepad support without dragging in SDL.
- **Demoscene** and creative-coding folks who want one C file and a window.
- **Teachers and students** learning low-level graphics or audio — FPL is short enough to read end-to-end.
- **Embedded / Pi developers** who need ARM + OpenGL or ARM + Vulkan without dependency hell.

---

## Scene 15 — Roadmap / Future Work (10:30 – 11:00)

**On-screen bullets:**

- **Wayland** windowing backend on Linux
- **EGL** video backend on X11 / Wayland
- **XAudio** audio backend on Windows
- **RawInput** keyboard / mouse on Windows
- **EvDev / udev** input on Linux
- Continued small libraries graduating out of alpha (`final_xml.h`)

**Narration:**
> "1.0 is the foundation, not the ceiling. The roadmap is intentionally narrow — only items that fit the *zero-dependency, single-header, OS-built-in-only* philosophy."

---

## Scene 16 — Call to Action / Outro (11:00 – 11:30)

**On-screen:**
> **Final Platform Layer 1.0.0**
> https://github.com/f1nalspace/final_game_tech
> MIT Licensed — Free for any use
>
> ⭐ Star the repo · 🐛 File issues · 🔧 Open pull requests · 📣 Spread the word

**Narration:**
> "If you build games, emulators, demos, or any C/C++ program that needs a window, sound, and input — give FPL a try. One header. Every platform. No dependencies. That is the promise of version 1.0."

**End card:** FPL logo · Author: Torsten Spaete (Finalspace) · Copyright © 2017–2026

---

## Appendix A — Suggested B-roll & Visuals

| Scene | Visual |
|-------|--------|
| 1     | Logo reveal, demo montage cuts |
| 2     | Code editor zooming into `#define FPL_IMPLEMENTATION` |
| 3     | Split screen: pile of SDL/GLFW deps vs. a single `.h` file |
| 4     | Animated feature wheel of subsystems |
| 5–6   | Side-by-side: code on the left, running window on the right |
| 7     | Each advantage as a stamp/badge animation |
| 8     | Honest "not supported" cards with crossed-out logos for macOS |
| 9     | Platform/backend grid with checkmarks lighting up |
| 10    | Carousel of demo screenshots (`screenshots/FPL_*.png`) |
| 11    | Changelog scroll with highlights popping out |
| 12    | Bookshelf metaphor — each library as a single book spine |
| 13    | Live recording: dragging the header into a project, compiling, running |
| 14    | Audience personas as illustrations |
| 15    | Roadmap timeline |
| 16    | GitHub star animation, final outro card |

## Appendix B — Talking-Point Cheat Sheet (one-liners)

- "One header. Every platform. No dependencies."
- "Drop it in. Define `FPL_IMPLEMENTATION`. Compile. Ship."
- "Runtime-linked by default — no `.lib`, no `.so`, no DLL hell."
- "Pure C99 — works in C **and** C++."
- "Zero third-party dependencies. Only built-in OS libraries."
- "Windows, Linux, Unix, BSD, Raspberry Pi — x86, x64, ARM."
- "OpenGL, Vulkan, or raw pixels — your choice."
- "DirectSound, WASAPI, ALSA, PulseAudio, PipeWire, OSS."
- "Keyboard, mouse, and gamepads — polling or event-based."
- "MIT licensed. Use it anywhere, including commercial."
- "Nine years of work. Today: 1.0."

## Appendix C — Stats & Facts

- **Library size:** ~1200 KB (single header, including the full implementation)
- **Languages:** C99, C17, C++11+
- **License:** MIT
- **First commit:** 2017
- **1.0 release year:** 2026
- **Maintainer:** Torsten Spaete (Finalspace)
- **AI-assisted development since:** 2026 — reviewed and tested by humans

---

*End of presentation.*
