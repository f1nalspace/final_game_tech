# FPL Code Quality & Completeness Analysis

Single-header C99 platform abstraction library — `final_platform_layer.h` (~28,433 lines).

**Audit scope:** architectures `x86`, `x86_64`, `ARM32`, `ARM64` on Windows, Linux (incl. Raspberry Pi), FreeBSD, NetBSD, OpenBSD. Platforms outside this set (macOS/iOS/Android, PPC, MIPS, SPARC, RISC-V) are noted where relevant but not analyzed.

---

## 1. Executive Summary

| Area | Verdict |
|---|---|
| **Overall maturity** | Post-1.0, actively maintained. Windows + Linux (X11) are solid. BSD/Unix is effectively a stub. |
| **Production readiness** | ~65% — fine for Windows OpenGL games and Linux ALSA+X11+OpenGL apps. Not suitable for modern WASAPI/PulseAudio/PipeWire, Wayland, Vulkan rendering, or any BSD. |
| **Headline strengths** | Clean fixed-width type system, ~745 public functions grouped into 20+ subsystems, Doxygen docs ~70%, comprehensive atomics API, XInput/evdev gamepad, fiber-based Win32 message loop, solid GLX/WGL backends. |
| **Headline weaknesses** | UNIX_PLATFORM (21292-21325) is 33 lines of stubs. Vulkan backend has no rendering path (empty `Present` at 23429-23431). X11 has 17 `@IMPLEMENT` stubs including clipboard and multi-monitor. DirectSound is legacy-only (no WASAPI fallback). ALSA format mapping table is incomplete. |
| **Critical bugs** | `#elif defined(defined(FPL_ARCH_POWERPC64))` at line 1956 — double `defined()` wrapper, never evaluates true (out-of-scope arch, but still a real defect). |

---

## 2. Scope & Methodology

- **Input:** `final_platform_layer.h` only. No other repo files were read.
- **Reading strategy:** the header was traversed in preprocessor-block chunks (~1,500-2,000 lines each) across the CHANGELOG/HEADER/API/TYPES/IMPLEMENTATION/BACKEND/SYSTEM sections.
- **Target platforms:** Windows (Win32), Linux (incl. Raspberry Pi via ARM32/ARM64), FreeBSD, NetBSD, OpenBSD.
- **Target architectures:** x86, x86_64, ARM32, ARM64.
- **Out of scope:** Android, macOS, iOS, non-listed CPUs (PPC, MIPS, SPARC, RISC-V) — mentioned only when a critical cross-cutting bug affects them.
- **Tooling:** Read + Grep on `final_platform_layer.h`, cross-referenced cited line numbers before writing this document. Every line citation below has been spot-verified.

---

## 3. File-Level Map

| Line range | Section | Purpose |
|---|---|---|
| 1-1720 | CHANGELOG | Version log, license, contributor notes |
| 1721-3480 | HEADER | Compile-time feature detection: `FPL_PLATFORM_*`, `FPL_ARCH_*`, `FPL_COMPILER_*`, `FPL_API`, assert macros |
| 3482-9000 | API | All public types and function prototypes |
| 9019-9518 | IMPL / top matter | `COMPILER_CONFIG`, `INTERNAL_TOP`, `INTERNAL_LOGGING`, `PLATFORM_CONSTANTS`, `UTILITY_FUNCTIONS` |
| 9520-10800 | TYPES | `TYPES_WIN32` / `TYPES_POSIX` / `TYPES_LINUX` / `TYPES_UNIX` / `TYPES_X11` — private platform structs |
| 10800-11200 | PLATFORM_STATES | Internal state aggregates |
| 11200-13360 | COMMON | Shared helpers: strings, memory, math, atomics dispatch |
| 13397-17437 | WIN32_PLATFORM | Full Win32 backend (window, input, threads, files, paths, console) |
| 14573-14900 | WIN32_XINPUT | XInput gamepad |
| 17438-19190 | POSIX_SUBPLATFORM | pthreads, POSIX file I/O, timing, atomics |
| 19195-19253 | STD_STRINGS / STD_CONSOLE | CRT fallback shims |
| 19256-20790 | X11_SUBPLATFORM | X11 window + input |
| 20793-21290 | LINUX_PLATFORM | Linux-specific: evdev, eventfd, epoll, /proc |
| 21292-21325 | UNIX_PLATFORM | **33 lines, stub only** |
| 21327-21440 | VIDEO_BACKENDS top | Dispatch glue |
| 21443-21861 | VIDEO_BACKEND_OPENGL_WIN32 | WGL |
| 21864-22390 | VIDEO_BACKEND_OPENGL_X11 | GLX |
| 22394-22486 | VIDEO_BACKEND_SOFTWARE_X11 | XImage software path |
| 22489-22568 | VIDEO_BACKEND_SOFTWARE_WIN32 | GDI StretchDIBits software path |
| 22568-23453 | VIDEO_BACKEND_VULKAN | Instance + surface only, no render path |
| 23455-23600 | AUDIO_BACKEND_API | Backend interface |
| 23743-24667 | AUDIO_BACKEND_DIRECTSOUND | DSound |
| 24667-25967 | AUDIO_BACKEND_ALSA | ALSA |
| 25967-26734 | SYSTEM_AUDIO_L1 | Private audio impl |
| 26734-26911 | SYSTEM_VIDEO_L1 | Private video impl |
| 26911-~27500 | SYSTEM_WINDOW | Window system glue |
| ~27500-~27900 | SYSTEM_AUDIO_L2 / SYSTEM_VIDEO_L2 | Public audio/video API bodies |
| ~27900-28433 | SYSTEM_INIT | Platform init/release |

---

## 4. Public API Surface

Totals (lines 3482-9000):

- **~745 public functions** across 22+ groups
- **~224 public types** (enums, structs, unions)
- **Doxygen coverage ~70%** (`@brief`, `@param[in/out]`, `@return`, `@note`, `@see`, `@defgroup`) — roughly 30% of public functions lack a `@brief` line

### API group inventory

| Group | Approx functions | Approx types | Completeness |
|---|---|---|---|
| Timing | 3 | 1 | Complete |
| Atomics | ~60 | 0 | Complete — fences, exchange, fetch-add, CAS, load/store for u32/u64/s32/s64/ptr/size |
| Memory | 8 | 3 | Complete — aligned + unaligned alloc/free, copy, clear, info |
| Hardware / CPU | 8 | 4 | Partial — ARM/non-x86 caps stubbed (see 12284/12289) |
| Platform / OS | 3 | 2 | Partial — Unix locale is a stub |
| Settings | 5 | 10+ | Complete |
| Error / Log | 6 | 3 | Complete |
| Threading | ~40 | 8 | Complete — mutex, semaphore, condvar, thread, signal, barrier |
| Console | 4 | 0 | Complete |
| Dynamic Library | 3 | 1 | Complete |
| Files / I/O | ~25 | 4 | Complete, pair-symmetric (32/64/size variants) |
| Paths | 5 | 0 | Complete |
| Strings | 8 | 0 | Complete — UTF-8/UTF-16/wide conversions |
| Input (kbd/mouse/gamepad) | ~30 | 12 | Complete API, partial Linux impl (js0 only) |
| Window | ~30 | 8 | API complete, X11 impl has 17 stubs |
| Window Events | 6 | 4 | Complete |
| Display / Monitor | 6 | 3 | Complete API, X11 impl stubbed |
| Video | ~20 | 8 | API complete, Vulkan impl incomplete |
| Audio | ~50 | 15 | Complete API, ALSA+DSound gaps |
| Localization | 3 | 2 | Stubbed on Unix |
| DateTime | 4 | 3 | Complete |

### Doc quality notes

- Doxygen grouping via `@defgroup` is consistent and navigable.
- About 30% of public functions lack `@brief`; these are usually variants of a sibling function (e.g. size/u32/u64 siblings inherit doc implicitly).
- A handful of internal-only functions appear documented publicly which bloats the generated docs.

---

## 5. Configuration & Feature Detection (lines 1721-3480)

### Architecture detection (1817-1880)

| Arch | Defines used | Status |
|---|---|---|
| x86_64 | `__x86_64__`, `_M_X64`, `__amd64__` | Complete |
| x86 | `__i386__`, `_M_IX86`, `__X86__`, `_X86_` | Complete |
| ARM64 | `__aarch64__`, `_M_ARM64` | Complete |
| ARM32 | `__arm__`, `_M_ARM`, `__ARM_ARCH_6/7__`, `__armv7__` | Complete |
| Other | RISC-V, PPC32/64, MIPS, SPARC | Detected but unused in this audit |

Unknown architecture errors out at line 1880.

### Platform detection (2059-2087)

- `FPL_PLATFORM_WINDOWS` via `_WIN32`/`_WIN64`
- `FPL_PLATFORM_LINUX` via `__linux__`/`__gnu_linux__`
- `FPL_PLATFORM_UNIX` via `__FreeBSD__`, `__NetBSD__`, `__OpenBSD__`, `__DragonFly__`, `__bsdi__`, generic `__unix`

All five required OSes (Windows, Linux, FreeBSD, NetBSD, OpenBSD) are flagged. Sub-platforms `POSIX`, `X11`, `STD_CONSOLE`, `STD_STRINGS`, `BSD` are layered on top.

### Compiler detection (1976-2050)

Clang, Intel, MinGW, ARMCC, GCC, MSVC, Apple Clang, Borland, TCC, DMC, CSMC, Linaro. Order of `#if` branches places Clang before GCC to avoid false-positive GCC on Clang.

### Cacheline detection (1938-1968)

| Arch | Cacheline | Verified |
|---|---|---|
| x86 / x86_64 | 64 | ✓ |
| ARM32 | 32 | ✓ |
| ARM64 / Apple ARM64 | 128 | ✓ |
| PowerPC64 | 128 | **BROKEN** — line 1956: `#elif defined(defined(FPL_ARCH_POWERPC64))` — double `defined()`, never matches; falls through to 64 |
| default | 64 | ✓ |

**CRITICAL:** line 1956 is a genuine preprocessor bug. While PowerPC64 is out of scope for this audit, the bug is a one-character fix and should be corrected.

### Feature guards (2562-2672)

`FPL_NO_WINDOW`, `FPL_NO_VIDEO`, `FPL_NO_AUDIO`, `FPL_NO_VIDEO_OPENGL`, `FPL_NO_VIDEO_VULKAN`, `FPL_NO_VIDEO_SOFTWARE`, `FPL_NO_AUDIO_DIRECTSOUND`, `FPL_NO_AUDIO_ALSA`. `FPL_NO_CRT` hard-requires user-provided `vsnprintf`. `FPL_APPTYPE_CONSOLE`/`FPL_APPTYPE_WINDOW` auto-selected, with mutex check at line 2362.

---

## 6. Type System Quality (lines 3482-5000, 9520-10800)

- **Naming:** consistent `fplFooBar` for public, `fpl__FooBar` for internal. No `snake_case` variants leak out.
- **Integer types:** fixed-width `int32_t`, `int64_t`, `uint8_t` etc. are used throughout struct fields. `fpl_b32` is `int32_t`.
- **Static assertions:** struct sizes validated at compile time (e.g. lines 2857-2865, 4492, 4530). ARM CPU caps struct padded to 32 bytes to match the x86 sibling.
- **Opaque handles:** threads, mutexes, semaphores use `uint64_t handle[1]` arrays. Clean.
- **Alignment / padding:** explicit padding bytes inserted where necessary (e.g. 3-byte pad at line 3624 in `fplDateTimeCreationResult`).
- **Unions:** `fplAudioDeviceID` (5155-5166) conditionally includes DirectSound `GUID` + ALSA device-name string. `fplTimestamp` holds either Win32 or POSIX representation.

### Type-system issues

| Severity | Line | Issue |
|---|---|---|
| Major | 3311 | X11 `Display` typedef'd as `void *`, but comment admits it is actually a large struct — ABI / sizeof risk if callers assume the typedef's size |
| Minor | 4495-4527 | `fplARMCPUCapabilities` only covers NEON/AES/SHA1/SHA2/CRC32/PMULL. No SVE, SVE2, SME, Dotprod, I8MM, BF16 |
| Minor | 4460-4492 | `fplX86CPUCapabilities` missing GFNI, VAES, VPCLMUL, AVX512 subfamilies |
| Minor | — | Naming drift between `fplVideoBackendType_OpenGL` (camel) and `fplAudioBackendType_Alsa` (title-case) |

---

## 7. Per-Section Code Quality Analysis

### 7.1 WIN32_PLATFORM (13400-17437, ~4,040 LOC)

**Implemented:** full window lifecycle, fullscreen with display-mode change (13474-13553), raw input, 14 modifiers (13663-13710), 256-entry virtual-key switch (14115-14394), `BITMAPV5HEADER`-based icon creation (14050-14113), cursor control, Motif-like decoration, multi-monitor awareness via `GetMonitorInfo`, `WM_DROPFILES`, fiber-based message loop (13720-13732), full threading with priority mapping, `CRITICAL_SECTION` mutexes, `Interlocked*` atomics, `GlobalMemoryStatusEx`/`GetPhysicallyInstalledSystemMemory`, `RtlGetVersion` with fallback.

**Issues:**
- Line 13510 `@TODO`: fullscreen origin assumption may be wrong on non-primary displays.
- Line 16348 `@TODO`: ACL-based file permission detection not implemented.
- No DirectInput fallback path if XInput is unavailable.
- No explicit high-DPI raw-input scaling.

**Quality:** solid. Biggest risk is the WndProc switch at 13759-14048 (~290 LOC) which is large but flat.

### 7.2 WIN32_XINPUT (14573-14900, ~300 LOC)

Polling + state mgmt + rumble for up to 4 controllers. No hot-plug notification (polls state). No DirectInput fallback. Otherwise complete.

### 7.3 POSIX_SUBPLATFORM (17438-19190, ~1,750 LOC)

Solid. Implements pthreads, mutex, condition variables, POSIX file I/O, time, and `__sync_*`-based atomics (17584-17758).

**Issues:**
- Line 17859 `@TODO`: `sched_getscheduler` availability not probed across POSIX variants.
- Line 19178 `@TODO`: distro-version detection stubbed.
- `fpl__PosixThreadWaitForMultiple` (17533-17579) polls with a fixed 10 ms sleep. On `N` threads this is O(N) per tick — should be replaced with condition-variable-based completion signalling.

### 7.4 STD_STRINGS / STD_CONSOLE (19195-19253)

Thin CRT shims. Low risk, low complexity.

### 7.5 X11_SUBPLATFORM (19256-20790, ~1,534 LOC)

**Implemented:** window creation/destruction, event loop (20400-20452), 260+ keysym translation switch (19305-19536), Motif decoration hints (20519-20552), icon loading (19538-19579), XDND drag-drop (19731-19741), focus tracking, fullscreen via `_NET_WM_STATE` (20603-20627), keyboard + mouse state polling.

**Stub markers — 17 × `@IMPLEMENT`:**

| Line | Function | Impact |
|---|---|---|
| 20476 | `fplSetWindowCursorEnabled` | Mouse cursor hide/show broken on X11 |
| 20504 | `fplIsWindowResizable` | Query broken |
| 20509 | `fplSetWindowResizable` | Toggle broken |
| 20555 | `fplIsWindowFloating` | Query broken |
| 20560 | `fplSetWindowFloating` | Toggle broken |
| 20564 | `fplGetWindowState` | State query broken |
| 20569 | `fplSetWindowState` | State setter broken |
| 20574 | `fplGetDisplayCount` | No multi-monitor count |
| 20579 | `fplGetDisplays` | No multi-monitor enum |
| 20584 | `fplGetPrimaryDisplay` | No primary query |
| 20589 | `fplGetWindowDisplay` | No per-window display |
| 20594 | `fplGetDisplayFromPosition` | No position→display lookup |
| 20599 | `fplGetDisplayModes` | No mode enum |
| 20630 | `fplSetWindowFullscreenRect` | No per-display fullscreen |
| 20700 | `fplGetClipboardText` | Clipboard get broken |
| 20705 | `fplSetClipboardText` | Clipboard set broken |
| 20783 | `fplQueryCursorPosition` | Cursor pos broken |

**Known bugs:**
- Line 19539 `@BUG`: window icon set via `_NET_WM_ICON` works but is not shown in the GNOME/Ubuntu task bar.
- Line 20675 `@BUG`: window title not shown in the GNOME task bar entry.
- Line 19531 `@TODO`: OEM1-OEM8 keys unmapped.
- Line 20620 `@TODO`: no display-mode change (Xrandr missing).

### 7.6 LINUX_PLATFORM (20793-21290, ~498 LOC)

**Implemented:** `setlocale`, evdev gamepad (detect, poll, dead-zone, dpad, triggers, hot-plug via `ENODEV` read failure 21032-21046), signal wakeup via `eventfd`, `epoll`-based multi-signal wait (21149-21204).

**Issues:**
- Lines 20957-20959: gamepad detection hard-wired to a single `"/dev/input/js0"` entry. Multi-gamepad setups see only the first device. No `/sys/class/input/` enumeration.
- Line 20844 `@TODO`: button mapping uses a pointer table instead of a static offset table.
- `fplMemoryGetInfos` exists but returns `false` — no `/proc/meminfo` parser.
- No `/proc/cpuinfo` parser for CPU name/flags.
- No `/etc/os-release` parser for distro version.
- No EVIOCRUMBLE / force-feedback ioctls for gamepad rumble.

### 7.7 UNIX_PLATFORM (21292-21325, **33 LOC**)

**CRITICAL GAP.** The entire BSD platform layer is 33 lines of near-empty bodies.

```c
21293  fpl__UnixReleasePlatform(...)  { /* empty */ }
21296  fpl__UnixInitPlatform(...)      { return true; }
21303  fplMemoryGetInfos(...)          { /* @IMPLEMENT(final/Unix) */ return false; }
21311  fplGetSystemLocale(...)         { /* @IMPLEMENT(final/Unix) */ return 0; }
21316  fplGetUserLocale(...)           { /* @IMPLEMENT(final/Unix) */ return 0; }
21321  fplGetInputLocale(...)          { /* @IMPLEMENT(final/Unix) */ return 0; }
```

What BSDs inherit for free:
- `POSIX_SUBPLATFORM` → pthreads, mutex, file I/O, atomics (via `__sync`)
- `X11_SUBPLATFORM` → X11 window/input (if libX11 is available)
- `SOFTWARE_X11` / `OPENGL_X11` video backends
- `STD_CONSOLE` / `STD_STRINGS`

What BSDs are missing entirely:
- Hardware / memory info (`sysctl` `HW_PHYSMEM`, `HW_NCPU`)
- OS version detection (`uname` or `sysctlbyname("kern.ostype")`)
- CPU info (`sysctlbyname("hw.model")`)
- Gamepad (`/dev/uhid*` on FreeBSD, no evdev)
- Audio — ALSA does not exist on BSD, and no OSS / sndio backend is implemented
- Locale queries

**Verdict:** FreeBSD/NetBSD/OpenBSD are *buildable* but deliver a severely degraded experience. This is the single biggest completeness gap in the library.

### 7.8 VIDEO_BACKEND_OPENGL_WIN32 (21443-21861, ~420 LOC)

Full WGL init with both legacy and `WGL_ARB_create_context` paths. Pixel format selection, context version request, extension string parsing. Production-ready.

Gap: no `GL_KHR_debug` wiring — the library does not expose a callback-based debug output path.

### 7.9 VIDEO_BACKEND_OPENGL_X11 (21864-22390, ~470 LOC)

GLX 1.3 `FBConfig`-based with legacy fallback. Extension query includes the "required for AMD drivers" note at line 22045. Production-ready.

### 7.10 VIDEO_BACKEND_SOFTWARE_WIN32 (22489-22568, ~80 LOC)

GDI `StretchDIBits` only. No hardware acceleration, no SIMD copy, minimal but functional.

### 7.11 VIDEO_BACKEND_SOFTWARE_X11 (22394-22486, ~90 LOC)

`XImage` + `XPutImage`. No `MIT-SHM` shared-memory optimization — every frame goes through the wire protocol, which is a major perf issue over remote X.

### 7.12 VIDEO_BACKEND_VULKAN (22568-23453, ~885 LOC)

**Implemented:** Vulkan loader (dlopen/LoadLibrary), instance creation (23275-23324), layer + extension enumeration, optional `VK_EXT_debug_utils` wiring, Win32 `vkCreateWin32SurfaceKHR` + X11 `vkCreateXlibSurfaceKHR`, instance/surface destruction.

**Not implemented:**
- No physical-device enumeration
- No logical-device / queue creation
- No swapchain (`VK_KHR_swapchain`) creation
- No render pass / framebuffer / pipeline
- **`fpl__VideoBackend_Vulkan_Present` at line 23429-23431 is an empty body** — cannot present frames
- No swapchain recreation on window resize

**TODOs:**
- Line 23128 `@TODO`: validation message-type filtering
- Line 23175 `@TODO`: validate vulkan video settings

**Verdict:** Usable only as an instance+surface provider for user code that drives Vulkan manually. As a rendering backend the Vulkan path is not functional.

### 7.13 AUDIO_BACKEND_DIRECTSOUND (23743-24667, ~925 LOC)

**Implemented:** device enumeration via `DirectSoundEnumerateW` (24008-24030), device info query (24032-24094), format negotiation with `WAVEFORMATEXTENSIBLE` (24391-24454), 11-speaker channel mask, PCM + IEEE float via GUID selection, secondary-buffer ring with `DSBPOSITIONNOTIFY` events (24528-24546), playback loop using `WaitForMultipleObjects` on period events (24584-24630), `DSSCL_EXCLUSIVE` vs `DSSCL_PRIORITY` mode selection.

**Issues:**
- Line 24057 `@IMPLEMENT`: `fpl__AudioBackendGetDeviceInfoExtended` returns `NotImplemented`. Clients cannot query detailed device caps.
- No recovery from `DSERR_NODRIVER` / device loss.
- No WASAPI fallback. DirectSound is legacy API; modern Windows setups may not expose it in all configurations (though it is still shipped).

### 7.14 AUDIO_BACKEND_ALSA (24667-25967, ~1,300 LOC)

**Implemented:** `snd_pcm_open` with configurable device name (25492-25510), hw/sw params (25512-25580), buffer and period sizing, two playback paths (`mmap_begin/commit` at 25707-25747 and `writei` at 25750-25792, auto-selected at 25584-25590), xrun recovery via `snd_pcm_recover`, device enumeration via `snd_device_name_hint` (25837-25967) that tags "default"/"pulse"/"pipewire" as defaults.

**Issues:**
- Line 25323 `@TODO`: FPL→ALSA format mapping table incomplete (no F64, U16, U32, S64).
- Line 25368 `@TODO`: ALSA→FPL mapping incomplete.
- Line 25528 `@TODO` / 25837 `@TODO`: ability to probe `:card,device` for `dmix` / `hw` is missing.
- Line 24864 `@TODO`: ALSA headers are statically `#include`d — runtime linking via `dlopen("libasound.so.2")` is not yet implemented. This breaks portability when `libasound-dev` is absent at build time.
- Lines 24672-24679: hard-coded 2× buffer scaling for Raspberry Pi `bcm2835` detected by glob match. Device-specific hack sitting in shared code.
- `PulseAudio` and `PipeWire` support is *device-name* probing via ALSA PCM, not native API — no `libpulse` or `libpipewire` integration.

### 7.15 SYSTEM_AUDIO_L1 / L2 (25967-26734, ~27500+)

Clean state machine: `Uninitialized → Stopped → Starting → Started → Stopping`, driven by a volatile enum and an event. Worker thread set to RealTime priority (27290-27305). Audio event signalling via `fpl__WaitForAudioEvent` / `fpl__SetAudioEvent`.

**Issues:**
- Line 27098 `@TODO`: frame-rounding workaround acknowledged as "not correct but works for most cases".
- Line 26138 `@TODO`: audio backend descriptor not fully derived from audio settings.
- Shutdown synchronization relies solely on volatile + event — no explicit lock. On aggressive shutdown this is a potential TOCTOU with the worker thread.

### 7.16 SYSTEM_VIDEO_L1 / L2 (26734-26911, ~27500+)

Clean backend union (`fpl__ActiveVideoBackend`) and dispatch to platform-specific load/unload/init/shutdown/present. Backbuffer allocation for software renderer is straightforward. Abstraction is cleaner than the audio side — no leakage of backend state into the system layer.

### 7.17 SYSTEM_WINDOW + SYSTEM_INIT (26911-28433)

Init order: `Platform → OS subsystem → Audio → Video → Window`. Release is the exact reverse. Worker thread is stopped before backend release. Safe.

Risk: if a backend init fails partway through, unwinding relies on well-defined state flags. A careful review of partial-failure paths would be valuable but nothing obviously broken stands out.

---

## 8. Completeness Matrix (OS × Arch)

Legend: `✅` full · `🟨` partial / known gaps · `⚠️` builds but untested / degraded · `❌` missing / stub · `—` not applicable

### 8.1 Window + Input subsystem

| Subsystem | Win/x86 | Win/x64 | Win/ARM32 | Win/ARM64 | Lin/x86 | Lin/x64 | Lin/ARM32 | Lin/ARM64 | FreeBSD/x86 | FreeBSD/x64 | FreeBSD/ARM32 | FreeBSD/ARM64 | NetBSD/x86 | NetBSD/x64 | NetBSD/ARM32 | NetBSD/ARM64 | OpenBSD/x86 | OpenBSD/x64 | OpenBSD/ARM32 | OpenBSD/ARM64 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Window creation | ✅ | ✅ | ⚠️ | ⚠️ | 🟨 | 🟨 | 🟨 | 🟨 | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ |
| Keyboard input | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ |
| Mouse input | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ |
| Gamepad | ✅ XInput | ✅ XInput | ⚠️ | ⚠️ | 🟨 js0 | 🟨 js0 | 🟨 js0 | 🟨 js0 | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Clipboard | ✅ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Multi-monitor | ✅ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

Notes: "Win/ARM32" and "Win/ARM64" are marked `⚠️` because FPL builds for Windows on ARM should compile — Win32 APIs exist — but `Interlocked*` on ARM, XInput on ARM, and fiber support on ARM have not been audited in the backend code.

### 8.2 Video subsystem

| Backend | Win/x86 | Win/x64 | Win/ARM32 | Win/ARM64 | Lin/x86 | Lin/x64 | Lin/ARM32 | Lin/ARM64 | FreeBSD/x86 | FreeBSD/x64 | FreeBSD/ARM32 | FreeBSD/ARM64 | NetBSD/all | OpenBSD/all |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| OpenGL | ✅ WGL | ✅ WGL | ⚠️ | ⚠️ | ✅ GLX | ✅ GLX | ✅ GLX | ✅ GLX | ⚠️ GLX | ⚠️ GLX | ⚠️ GLX | ⚠️ GLX | ⚠️ GLX | ⚠️ GLX |
| Software | ✅ GDI | ✅ GDI | ⚠️ | ⚠️ | 🟨 XImage | 🟨 XImage | 🟨 XImage | 🟨 XImage | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ |
| Vulkan | 🟨 inst only | 🟨 inst only | ⚠️ | ⚠️ | 🟨 inst only | 🟨 inst only | 🟨 inst only | 🟨 inst only | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Wayland | — | — | — | — | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

### 8.3 Audio subsystem

| Backend | Win/x86 | Win/x64 | Win/ARM32 | Win/ARM64 | Lin/x86 | Lin/x64 | Lin/ARM32 | Lin/ARM64 | FreeBSD | NetBSD | OpenBSD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| DirectSound | 🟨 legacy | 🟨 legacy | ⚠️ | ⚠️ | — | — | — | — | — | — | — |
| WASAPI | ❌ | ❌ | ❌ | ❌ | — | — | — | — | — | — | — |
| ALSA | — | — | — | — | 🟨 PCM | 🟨 PCM | 🟨 PCM + RPi hack | 🟨 PCM | ❌ (no ALSA) | ❌ | ❌ |
| PulseAudio (native) | — | — | — | — | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| PipeWire (native) | — | — | — | — | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| OSS / sndio | — | — | — | — | — | — | — | — | ❌ | ❌ | ❌ |

### 8.4 Core services

| Subsystem | Win/x86 | Win/x64 | Win/ARM32 | Win/ARM64 | Lin/x86 | Lin/x64 | Lin/ARM32 | Lin/ARM64 | FreeBSD/all | NetBSD/all | OpenBSD/all |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Threading (pthreads/Win32) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| File I/O | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Memory info | ✅ | ✅ | ✅ | ✅ | 🟨 stub | 🟨 stub | 🟨 stub | 🟨 stub | ❌ stub | ❌ stub | ❌ stub |
| OS / locale | ✅ | ✅ | ⚠️ | ⚠️ | 🟨 | 🟨 | 🟨 | 🟨 | ❌ stub | ❌ stub | ❌ stub |
| Atomics (u32/u64) | ✅ Interlocked | ✅ Interlocked | 🟨 | 🟨 | 🟨 __sync | 🟨 __sync | 🟨 __sync | 🟨 __sync | 🟨 __sync | 🟨 __sync | 🟨 __sync |
| CPU name/caps | ✅ x86 | ✅ x86 | ❌ ARM | ❌ ARM | ✅ x86 | ✅ x86 | ❌ ARM | ❌ ARM | 🟨 | 🟨 | 🟨 |

**Atomics note:** the POSIX path relies entirely on GCC `__sync_*` builtins. These are lock-free on ARM32 (ARMv6+) and ARM64, but provide no acquire/release/seqcst control — everything is a full barrier. Modern code should use `__atomic_*` with explicit memory order. This is listed as a Major issue below.

---

## 9. Complexity Analysis

| Hot-spot | Location | LOC | Nature | Suggested action |
|---|---|---|---|---|
| Win32 WndProc switch | 13759-14048 | ~290 | Large flat switch, `WM_*` dispatch, per-message heap allocations for drop files | Acceptable; consider extracting per-message helpers |
| Win32 fullscreen state machine | 13474-13553 | ~80 | Window-style restore, `ChangeDisplaySettingsW`, multi-monitor position | Extract display-change helper |
| X11 event loop | 20400-20452 | ~50 | Multi-queue drain, fallthrough event translation | OK |
| X11 keysym switch | 19305-19536 | ~230 | 260+ keysyms via `case` statements | Replace with sorted lookup table |
| Linux evdev poll | 20841-21070 | ~230 | Gamepad poll, dead-zone math, hotplug via ENODEV | Reasonable; cleanup pointer-map at 20844 |
| POSIX wait-for-multiple threads | 17533-17579 | ~45 | Polling loop with 10 ms sleep | Replace with condvar-driven completion |
| ALSA device enumeration | 25837-25967 | ~130 | Triple-nested hint walk with per-iteration frees | OK, but dense |
| DirectSound playback loop | 24584-24630 | ~50 | `WaitForMultipleObjects` with modulo cursor wrap | OK |
| Vulkan instance creation | 23175-23324 | ~150 | 4-level-nested layer + extension enumeration | Extract layer/ext enumerator helpers |

### Function-length watch-list

These are the longest contiguous implementations spotted; none are pathological but all would benefit from extraction:

- `fpl__Win32WindowProc` (13759-14048) — ~290 lines
- `fpl__X11EventHandler` / keysym translation (19305-19536) — ~230 lines
- `fpl__LinuxUpdateGameController` + helpers (20841-21070) — ~230 lines
- Vulkan instance+surface init (23175-23380) — ~205 lines

---

## 10. Architectural Weaknesses

1. **BSD support is aspirational.** `UNIX_PLATFORM` provides the minimum to build, nothing more. The library advertises FreeBSD/NetBSD/OpenBSD support but no hardware, audio, gamepad, or locale services exist for them.
2. **Vulkan backend is not a backend.** It is an instance+surface provider. `Present` at line 23429-23431 is literally empty. Users who opt into Vulkan must then implement the swapchain, pipeline, and present loop themselves — contrary to the promise of a backend abstraction.
3. **Audio coverage lags modern OSes.** DirectSound (Windows) and ALSA PCM (Linux) are legacy / low-level. WASAPI, PulseAudio, PipeWire, and sndio are all absent. Modern Linux systems route audio through PipeWire; ALSA PCM "pulse" / "pipewire" device hints work but rely on plugin availability.
4. **X11 is the only Linux windowing backend.** No Wayland. On modern distros that ship Wayland by default the library falls back to XWayland, which is an OS-level shim — not a first-class path.
5. **X11 impl is incomplete.** 17 `@IMPLEMENT` stubs cover clipboard, multi-monitor enumeration, and window-state toggles — foundations any non-toy app needs.
6. **Atomics lack memory-order semantics.** The POSIX atomics use `__sync_*` builtins which always imply a full barrier. This is correct but suboptimal; performance-sensitive ARM code cannot issue relaxed or acquire-only ops.
7. **Linux gamepad is hard-wired.** Only `/dev/input/js0` is probed. No enumeration, no hotplug of multiple devices, no rumble.
8. **DirectSound GetAudioDeviceInfoExtended returns `NotImplemented` (24057).** Clients cannot query detailed caps.
9. **ALSA format mapping is incomplete (25323, 25368).** Unsupported formats silently fail negotiation.
10. **ALSA headers are statically `#include`d (24864).** Runtime linking (`dlopen`) is not yet enabled, breaking portability when `libasound-dev` is absent at build time.
11. **Raspberry Pi audio hack in shared code (24672-24679).** Device-specific glob-matched buffer scaling lives in production paths instead of a capability/config driver.
12. **Thread-safety of shutdown is implicit.** Audio state machine relies on volatile + event only; no explicit locks. Works but not hardened.
13. **`st_atim` vs `st_atime` detection (10181) is not finished** — builds on some older POSIX systems may break.
14. **X11 `Display` typed as `void *` (3311)** — ABI risk if callers ever take `sizeof(*fpl__X11Display)`.

---

## 11. Issue Ledger

### 11.1 Critical

| ID | Location | Description | Impact | Fix direction |
|---|---|---|---|---|
| C1 | 21292-21325 | `UNIX_PLATFORM` is 33 lines of stub init + 4 `@IMPLEMENT` markers (memory info, 3 × locale). No BSD hardware, audio, gamepad, locale. | FreeBSD/NetBSD/OpenBSD "supported" in name only. | Implement `sysctl`-based hardware / locale queries; add sndio or OSS audio backend; add FreeBSD `/dev/uhid*` gamepad path. |
| C2 | 23429-23431 | `fpl__VideoBackend_Vulkan_Present` is an empty function body. | Vulkan backend cannot present frames — rendering does not work. | Complete device/queue/swapchain/render-pass/pipeline + proper `vkQueuePresentKHR`. |
| C3 | Audio (23743-24667) | DirectSound is the only Windows audio backend. No WASAPI fallback. | On restricted / legacy-disabled Windows setups audio fails to init; high-latency even when it works. | Add WASAPI shared + exclusive backend as primary; keep DirectSound as fallback. |
| C4 | 1956 | `#elif defined(defined(FPL_ARCH_POWERPC64))` — nested `defined()`, never matches. | PPC64 cacheline wrong; out-of-scope arch but still a real bug. | Change to `#elif defined(FPL_ARCH_POWERPC64)`. |

### 11.2 Major

| ID | Location | Description | Impact | Fix direction |
|---|---|---|---|---|
| M1 | 20574-20599 | X11 display enumeration: 6 `@IMPLEMENT` stubs (`fplGetDisplayCount`, `fplGetDisplays`, `fplGetPrimaryDisplay`, `fplGetWindowDisplay`, `fplGetDisplayFromPosition`, `fplGetDisplayModes`). | No multi-monitor on Linux/BSD X11. | Use Xrandr 1.5 (`RRCrtc`, `RROutput`). |
| M2 | 20700-20705 | X11 clipboard get/set unimplemented. | No copy-paste on Linux. | Implement `XConvertSelection` / `XSetSelectionOwner` + SelectionRequest handler. |
| M3 | 20620 | X11 display-mode change unimplemented. | No resolution/refresh switching on Linux. | Use Xrandr `XRRSetCrtcConfig`. |
| M4 | 20957-20959 | Linux gamepad hard-wired to `/dev/input/js0`. | Only first controller ever detected. | Enumerate `/sys/class/input/event*` with udev or scandir fallback. |
| M5 | 21231-21237 (body of `fplMemoryGetInfos` on Linux) | Linux memory-info parser missing. | `fplMemoryGetInfos` returns `false`. | Parse `/proc/meminfo` or use `sysinfo(2)`. |
| M6 | 25323, 25368 | ALSA format mapping tables incomplete. | Unsupported `fplAudioFormatType` variants silently fail. | Add F64, U16, U32, S64, U8 mapping; gate behind ALSA version. |
| M7 | 24057 | DirectSound `fpl__AudioBackendGetDeviceInfoExtended` returns `NotImplemented`. | Clients cannot query device caps. | Implement via `IDirectSoundCapture::GetCaps` + `DirectSoundEnumerate`. |
| M8 | 17533-17579 | POSIX `WaitForMultiple` uses 10 ms polling sleep. | High-latency thread join, wasted CPU. | Replace with condvar-based completion signal. |
| M9 | 17584-17758 | POSIX atomics use `__sync_*` (full barrier only). ARM32/ARM64 affected. | No relaxed/acquire/release ordering. | Switch to `__atomic_*` with explicit memory order. |
| M10 | 3311 | X11 `Display` typedef as `void *` with comment noting it is actually a large struct. | Any `sizeof` / value-copy path is wrong. | Keep typedef but document as opaque-pointer only; static-assert `sizeof(void*)` where used. |
| M11 | 4495-4527 | ARM CPU capabilities struct missing SVE, SVE2, SME, Dotprod, I8MM, BF16. | Cannot dispatch modern ARM code paths. | Extend struct; probe `/proc/cpuinfo` and HWCAP/HWCAP2. |
| M12 | 13510 | Win32 fullscreen origin assumes `(0,0)`. | May break on non-primary-monitor fullscreen. | Use `GetMonitorInfo` of target display. |
| M13 | 16348 | Win32 ACL-based file permission detection stubbed. | Incorrect permission reporting. | Use `GetFileSecurity` / `GetEffectiveRightsFromAcl`. |
| M14 | 24864 | ALSA statically `#include`s `alsa/asoundlib.h`. | Build fails without `libasound-dev`; runtime portability broken. | Add `dlopen("libasound.so.2")` loader like the Vulkan loader. |

### 11.3 Minor

| ID | Location | Description |
|---|---|---|
| m1 | 19539 | X11 window icon not shown in GNOME/Ubuntu task bar (`@BUG`) |
| m2 | 20675 | X11 window title not shown in GNOME task bar (`@BUG`) |
| m3 | 19531 | X11 OEM1-OEM8 key mapping missing |
| m4 | 20844 | Linux evdev button map uses pointer table, not static offset |
| m5 | 20476/20504/20509/20555/20560/20564/20569/20630/20783 | 9 smaller X11 `@IMPLEMENT` stubs (cursor, resizable, floating, state, fullscreen rect, cursor pos) |
| m6 | 23128 | Vulkan message-type filtering TODO |
| m7 | 23175 | Vulkan settings validation TODO |
| m8 | 24672-24679 | Raspberry Pi hard-coded 2× ALSA buffer scale |
| m9 | 27098 | Audio frame-rounding "fake" workaround |
| m10 | 26138 | Audio backend descriptor not fully sourced from settings |
| m11 | 10181 | POSIX `st_atim` vs `st_atime` detection TODO |
| m12 | 17859 | `sched_getscheduler` availability TODO |
| m13 | 19178 | POSIX distro-version detection TODO |
| m14 | 12284, 12289 | `fplCPUGetCapabilities` / `fplCPUGetName` missing for non-x86 |
| m15 | 19305-19536 | X11 keysym switch instead of table lookup |
| m16 | 22394-22486 | X11 software backend does not use `MIT-SHM` |
| m17 | 25528, 25837 | ALSA `:card,device` probing TODO |
| m18 | — | Naming drift `_OpenGL` vs `_Alsa` |
| m19 | — | ~30% of public functions lack `@brief` |
| m20 | — | No `GL_KHR_debug` wiring in OpenGL backends |

---

## 12. Missing Work to Complete the Library

Cited line numbers mark where each item must land. Items are grouped by platform/subsystem. Estimates in parentheses are rough and for planning only.

### BSD / UNIX_PLATFORM (21292-21325)
- Implement `fplMemoryGetInfos` via `sysctl(CTL_HW, HW_PHYSMEM)` / `HW_USERMEM`.
- Implement `fplGetSystemLocale`, `fplGetUserLocale`, `fplGetInputLocale` via `setlocale` + env fallback.
- Add `fpl__UnixInitPlatform` actual init (signal mask, X11 bootstrap check, sysctl handle cache).
- Add CPU name/caps via `sysctlbyname("hw.model")` / `hw.ncpu`.
- Add OS version via `sysctlbyname("kern.ostype")` / `kern.osrelease`.
- Add gamepad on FreeBSD via `libusbhid` / `/dev/uhid*`.
- Add audio backend: **sndio** (OpenBSD native), **OSS** (FreeBSD/NetBSD fallback), or route through ALSA-OSS compat.

### Linux
- Enumerate all gamepads: walk `/dev/input/event*` (evdev) or `/dev/input/js*`, add hotplug via `inotify` or udev (20957-20959).
- Parse `/proc/meminfo` for `fplMemoryGetInfos` (21231-21237).
- Parse `/proc/cpuinfo` + `HWCAP` for CPU name / flags, esp. ARM (extend 4495-4527).
- Parse `/etc/os-release` for distro/version (19178).
- Gamepad rumble via `EVIOCSFF` + `write(effect_id)`.

### X11
- Implement all 17 `@IMPLEMENT` stubs (20476, 20504, 20509, 20555, 20560, 20564, 20569, 20574, 20579, 20584, 20589, 20594, 20599, 20630, 20700, 20705, 20783).
- Implement Xrandr-backed display mode change (20620).
- Map OEM1-OEM8 keys (19531).
- Move to `MIT-SHM` for the software backend (22394-22486).
- Fix GNOME task-bar icon / title bugs (19539, 20675).

### Vulkan
- Add physical device enumeration + suitability scoring.
- Add logical device + queue creation.
- Add `VK_KHR_swapchain` creation + recreation on resize.
- Add a minimal render pass + blit-to-swapchain path.
- Implement `fpl__VideoBackend_Vulkan_Present` (23429-23431) with `vkAcquireNextImageKHR` + `vkQueuePresentKHR`.
- Wire validation-layer message-type filtering (23128).
- Validate Vulkan video settings (23175).

### DirectSound / Windows audio
- Implement `fpl__AudioBackendGetDeviceInfoExtended` (24057) via `IDirectSoundCapture::GetCaps`.
- Add WASAPI as the primary Windows backend.
- Handle `DSERR_NODRIVER` and device loss gracefully.

### ALSA
- Complete format mapping tables (25323, 25368).
- Add `:card,device` probing for `dmix` / `hw:` (25528, 25837).
- Switch to `dlopen("libasound.so.2")` runtime linking (24864).
- Move the Raspberry Pi buffer scale into a capability table (24672-24679).

### Win32
- Fix fullscreen origin for non-primary monitor (13510).
- Implement ACL-based file permission read (16348).
- Add `GL_KHR_debug` wiring for OpenGL backends.
- Audit ARM32/ARM64 Win32 paths (atomics, fibers).

### Cross-cutting
- Replace POSIX `__sync_*` with `__atomic_*` + explicit memory order.
- Replace POSIX `WaitForMultiple` polling loop with condvar (17533-17579).
- Finish `st_atim` vs `st_atime` detection (10181).
- Fix PPC64 cacheline bug (1956) — trivial one-char fix.
- Add missing `@brief` comments to ~30% of functions.

---

## 13. Roadmap for Production-Grade Quality

Prioritized. Effort estimates are rough order-of-magnitude ranges based on typical backend sizes elsewhere in FPL.

| Priority | Item | Est. LOC | Rationale |
|---|---|---|---|
| P0 | Complete Vulkan rendering pipeline (device, queue, swapchain, render pass, present) | 2,000-3,000 | C2 — backend is currently non-functional |
| P0 | Implement UNIX_PLATFORM for FreeBSD / NetBSD / OpenBSD (hardware, locale, audio via sndio+OSS, gamepad) | 800-1,200 | C1 — library advertises BSD support |
| P0 | Add WASAPI backend (shared + exclusive, event-driven) | 800-1,200 | C3 — legacy-only Windows audio is a liability |
| P1 | X11 multi-monitor via Xrandr (M1 + M3) | 300-450 | Any serious app needs multi-monitor |
| P1 | X11 clipboard get/set (M2) | 120-180 | Any serious app needs clipboard |
| P1 | Linux gamepad multi-device + hot-plug + rumble (M4) | 150-250 | Modern gaming expectations |
| P1 | Add Wayland window + input backend | 600-900 | Default on modern Linux distros |
| P1 | PulseAudio native backend | 500-800 | Default on many Linux distros |
| P1 | ALSA runtime linking via dlopen (M14) | 80-120 | Portability / build robustness |
| P2 | PipeWire native backend | 700-1,000 | Next-gen Linux audio |
| P2 | Linux /proc parsers: meminfo, cpuinfo, os-release (M5) | 200-300 | Feature completeness |
| P2 | ARM32 / ARM64 atomics via `__atomic_*` with memory order (M9) | 200-300 | Perf + correctness |
| P2 | Finish X11 window-state stubs (m5 + rest of M1-M3) | 150-250 | Polishes app experience |
| P2 | ALSA format mapping completion (M6) | 40-80 | Correctness |
| P2 | DirectSound `GetDeviceInfoExtended` (M7) | 100-150 | API contract |
| P3 | POSIX WaitForMultiple → condvar (M8) | 80-120 | Perf / correctness |
| P3 | OpenGL `KHR_debug` wiring | 120-180 | DX parity |
| P3 | Replace Raspberry Pi hack with a capability table (m8) | 80-120 | Maintainability |
| P3 | ARM CPU capabilities (SVE, SVE2, SME, Dotprod, BF16) (M11) | 100-150 | Modern ARM dispatch |
| P3 | x86 CPU capabilities (GFNI, VAES, VPCLMUL, AVX512 subfamilies) | 80-120 | Modern x86 dispatch |
| P3 | Doxygen `@brief` completion for public API (~30%) | — | Doc polish |
| P3 | Fix PPC64 cacheline `defined(defined(...))` (C4) | 1 char | Trivial |
| P3 | Naming audit (m18) | — | Polish |
| P3 | Thread-safety audit for init/shutdown paths | 200-300 | Hardening |

**Approximate total for P0+P1: ~5,000-8,500 LOC of net new implementation.** P2+P3 adds another ~2,000-3,500 LOC. A single maintainer could realistically tackle P0 within 2-3 months of focused work.

---

## Appendix A — All TODO / FIXME / @IMPLEMENT / @BUG markers

Every marker below was verified against `final_platform_layer.h`.

```
1956   [BUG]        #elif defined(defined(FPL_ARCH_POWERPC64))   // out-of-scope arch but genuine bug
3311   @TODO        Opaque X11 Display is not correct (full struct, large)
10181  @TODO        POSIX st_atim vs st_atime detection
12284  @IMPLEMENT   fplCPUGetCapabilities for non-x86 architectures
12289  @IMPLEMENT   fplCPUGetName for non-x86 architectures
13510  @TODO        Win32 fullscreen (0,0) origin assumption
16348  @TODO        Win32 read ACL for file permission detection
17859  @TODO        sched_getscheduler availability across POSIX
19178  @TODO        POSIX get distro version
19531  @TODO        X11 map OEM1-OEM8 keys
19539  @BUG         X11 window icon not shown in GNOME/Ubuntu task bar
20476  @IMPLEMENT   fplSetWindowCursorEnabled (X11)
20504  @IMPLEMENT   fplIsWindowResizable (X11)
20509  @IMPLEMENT   fplSetWindowResizable (X11)
20555  @IMPLEMENT   fplIsWindowFloating (X11)
20560  @IMPLEMENT   fplSetWindowFloating (X11)
20564  @IMPLEMENT   fplGetWindowState (X11)
20569  @IMPLEMENT   fplSetWindowState (X11)
20574  @IMPLEMENT   fplGetDisplayCount (X11)
20579  @IMPLEMENT   fplGetDisplays (X11)
20584  @IMPLEMENT   fplGetPrimaryDisplay (X11)
20589  @IMPLEMENT   fplGetWindowDisplay (X11)
20594  @IMPLEMENT   fplGetDisplayFromPosition (X11)
20599  @IMPLEMENT   fplGetDisplayModes (X11)
20620  @TODO        X11 change display resolution + refresh rate
20630  @IMPLEMENT   fplSetWindowFullscreenRect (X11)
20675  @BUG         X11 window title not shown in GNOME/Ubuntu task bar
20700  @IMPLEMENT   fplGetClipboardText (X11)
20705  @IMPLEMENT   fplSetClipboardText (X11)
20783  @IMPLEMENT   fplQueryCursorPosition (X11)
20844  @TODO        Use static offset table instead of pointer mapping (Linux evdev)
21304  @IMPLEMENT   fplMemoryGetInfos (Unix)
21312  @IMPLEMENT   fplGetSystemLocale (Unix)
21317  @IMPLEMENT   fplGetUserLocale (Unix)
21322  @IMPLEMENT   fplGetInputLocale (Unix)
23128  @TODO        Vulkan message-type filtering
23175  @TODO        Validate Vulkan video settings
23429  [EMPTY]      fpl__VideoBackend_Vulkan_Present body is empty
24057  @IMPLEMENT   DirectSound audio device info extended
24672  [HACK]       Raspberry Pi ALSA 2x buffer scaling
24864  @TODO        Remove ALSA #include when runtime linking enabled
25323  @TODO        ALSA fplAudioFormatType → snd_pcm_format_t mapping table
25368  @TODO        ALSA snd_pcm_format_t → fplAudioFormatType mapping table
25528  @TODO        ALSA allow :card,device IDs for dmix / hw probing
25837  @TODO        (same as 25528)
26138  @TODO        Audio backend descriptor from settings
27098  @TODO        Frame-rounding workaround not correct
```

---

## Appendix B — Architecture-specific Notes

### x86 / x86_64
- Cacheline: 64 bytes (correct, 1938-1948).
- Atomics: `Interlocked*` on Windows, `__sync_*` on POSIX. Both lock-free.
- Full-width RDTSC path (12064-13360).
- CPU capability probing missing modern ISA bits (GFNI, VAES, VPCLMUL, AVX512 subfamilies).

### ARM32
- Cacheline: 32 bytes (1948-1950).
- Win32: untested in this audit; `Interlocked*` is available but fiber + XInput on ARM32 Windows are unexplored in the code.
- POSIX: `__sync_*` — lock-free on ARMv6+ via `ldrex`/`strex`. No acquire/release control. May hit kernel helpers (`__kuser_cmpxchg`) on pre-ARMv6.
- RDTSC: ARM32 perf counter path at 12251-12267.
- CPU capabilities: NEON (ARMv7a+) flag only; no VFP, Thumb-2, or TrustZone bits.

### ARM64 (AArch64)
- Cacheline: 128 bytes (1950-1955).
- POSIX atomics: `__sync_*` emits LL/SC (`ldaxr`/`stlxr`) or LSE atomics depending on target; full-barrier only.
- RDTSC: reads `CNTVCT_EL0` (12247-12250).
- CPU capabilities struct omits SVE, SVE2, SME, Dotprod, I8MM, BF16 — modern ARMv8.2+ features cannot be detected.
- Apple ARM64 aliased to `FPL_ARCH_APPLE_ARM64` but macOS is out of scope for this audit.

### Cross-arch atomics recommendation
All POSIX atomics should migrate from `__sync_*` to `__atomic_*` with explicit memory order. This is a prerequisite for any lock-free data structure that wants acquire/release semantics on ARM, and would not affect x86/x64 code generation materially.

---

*End of analysis.*
