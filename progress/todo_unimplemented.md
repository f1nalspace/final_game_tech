# FPL — Unimplemented / Partial TODO

Tracking missing or partial implementations in `final_platform_layer.h`.
Sourced from `@IMPLEMENT`, `@TODO`, `NotImplemented` returns, and Known Limitations section.

Last refresh: 2026-05-11

## Recently Implemented (no longer TODO)

- [x] `[X11] fplSetWindowCursorEnabled`
- [x] `[X11] fplIsWindowResizable` / `fplSetWindowResizeable`
- [x] `[X11] fplIsWindowFloating` / `fplSetWindowFloating`
- [x] `[X11] fplGetWindowState` / `fplSetWindowState`
- [x] `[X11] fplGetDisplayCount` / `fplGetDisplays` / `fplGetPrimaryDisplay`
- [x] `[X11] fplGetWindowDisplay` / `fplGetDisplayFromPosition` / `fplGetDisplayModes`
- [x] `[X11] fplSetWindowFullscreenRect`
- [x] `[X11] fplGetClipboardText` / `fplSetClipboardText`
- [x] `[X11] fplQueryCursorPosition` — now returns screen-relative coords (matches Win32 GetCursorPos), correctly guarded under FPL__ENABLE_WINDOW

## Platform / Architecture Coverage

- [ ] Arm32 — only partial support
- [ ] Arm64 — only partial support
- [ ] Unix/BSD — only partial support
- [ ] Raspberry Pi — only partial support
- [ ] MacOS / iOS — not supported (no hardware)
- [ ] x64 — untested
- [ ] Intel compiler — untested
- [ ] MingW32 / MingW64 — untested
- [ ] CC ARM — untested

## CPU

- [ ] `fplCPUGetCapabilities` for non-x86 architectures (line 13814)
- [ ] `fplCPUGetName` for non-x86 architectures (line 13819)
- [ ] Generic non-x86 CPU query path (Known Limitation)

## Window / Display (X11)

- [ ] X11 OEM1-OEM8 key mapping (line 22743)
- [ ] X11 display resolution + refresh rate switching (line 24584)

## Window (Win32)

- [x] ~~Multi-monitor origin assumption in `fpl__Win32EnterFullscreen` resolution-change path~~ — fixed: anchors to xpos/ypos or target monitor origin
- [x] ~~`fpl__Win32EnterFullscreen` non-resolution-change path wrote `ypos+h` into `.top` instead of `.bottom`~~ — fixed
- [ ] Read ACL for full permission detection (line 19115)

## Unix Platform

- [ ] `fplMemoryGetInfos` (line 25542) — Known Limitation: no unix memory query
- [ ] `fplGetSystemLocale` (line 25550)
- [ ] `fplGetUserLocale` (line 25555)
- [ ] `fplGetInputLocale` (line 25560)
- [ ] POSIX `st_atim` vs `st_atime` fallback detection for older POSIX (line 11087)
- [ ] `sched_getscheduler` POSIX standard coverage check (line 20624)
- [ ] LC_ALL restore in `fpl__LinuxReleasePlatform`

## Audio

- [x] ~~`[DirectSound] GetAudioDeviceInfo`~~ — done: lookup by GUID via DirectSoundEnumerateW
- [x] ~~`[ALSA] GetAudioDeviceInfo`~~ — done: lookup by name via snd_device_name_hint
- [x] ~~`[PulseAudio] GetAudioDeviceInfo`~~ — done: pa_context_get_sink_info_by_index
- [x] ~~`[PipeWire] GetAudioDeviceInfo`~~ — done: registry enum filtered by node id, shared helper with full enumeration
- [ ] Phase 2 (next): supported-formats list per backend — currently `supportedFormatCount = 0` placeholder
- [x] ~~`[ALSA]` Mapping table `fplAudioFormatType -> snd_pcm_format_t`~~ — done: indexed lookup, covers F64; S64 marked UNKNOWN (no ALSA equivalent)
- [x] ~~`[ALSA]` Mapping table `snd_pcm_format_t -> fplAudioFormatType`~~ — done: lookup table, handles native-endian aliases + FLOAT64
- [ ] `[ALSA]` Optional `dmix`/`hw` device id `:%d,%d` probe (line 29794, 30103)
- [ ] `[PipeWire]` Remove PipeWire include when runtime linking is enabled (line 31563)
- [ ] Audio backend descriptor from audio settings (line 32928)
- [ ] OSS backend — planned, not implemented (Known Limitation)
- [ ] Extended audio device infos do not contain format list yet (Known Limitation)
- [ ] Resampler edge case — fake-round extra frame workaround (line 33890)

## Input / Gamepad

- [ ] Unix gamepad via raw `fd` — planned, not implemented (Known Limitation)
- [ ] Linux evdev input backend (`FPL__SUPPORT_INPUT_LINUX_EVDEV` defined but not used)
- [ ] Linux udev input backend (`FPL__SUPPORT_INPUT_LINUX_UDEV` defined but not used)

## Video

- [ ] Vulkan message-type filtering in `fplVulkanSettings` (line 27373)
- [ ] Validate Vulkan video settings (line 27420)

## Internal

- [ ] Replace pointer mapping table with static offset table (line 24985)
- [ ] Crash path when feature not implemented is full-on crash (no graceful fallback)

## Source markers (as of 2026-05-11)

- `@IMPLEMENT` markers remaining: 6 (DirectSound device info, Unix memory/locale x3, CPU caps/name)
- `@TODO` markers remaining: 11
- `NotImplemented` audio device-info returns: 4 (DirectSound, ALSA, PulseAudio, PipeWire)
- All previously-listed `@IMPLEMENT(final/X11)` markers are now gone — entire X11 window/display/clipboard surface landed
