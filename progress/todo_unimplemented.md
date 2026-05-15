# FPL — Unimplemented / Partial TODO

Tracking missing or partial implementations in `final_platform_layer.h`.
Sourced from `@IMPLEMENT`, `@TODO`, `NotImplemented` returns, and Known Limitations section.

Last refresh: 2026-05-15

## Recently Implemented (since 2026-05-11)

- [x] Dropped `fplAudioDeviceInfoExtended` and `supportedFormats` — `fplGetAudioDeviceInfo` now returns plain `fplAudioDeviceInfo`; per-backend probe code removed (unreliable across resampling stacks)
- [x] `[Linux] fplMemoryGetUsage` — via `sysinfo()` + `/proc/meminfo` parsing (Cached + Buffers + SReclaimable for cache totals, swap for page counts)
- [x] `[Unix/BSD] fplMemoryGetUsage` — via sysctl (`hw.physmem`/`hw.realmem`, `vm.stats.vm.v_free_count`/`v_inactive_count`/`v_cache_count`, per-device `vm.swap_info.N` iteration with local `struct xswdev` accepting v1/v2 layouts)
- [x] `[OSS]` Full backend — enumeration via `/dev/sndstat`, blocking-write main loop
- [x] `[WASAPI]` Full backend — COM + IMMDeviceEnumerator init, IMMDeviceCollection enumeration, shared-mode event-driven main loop, exclusive-mode playback path
- [x] `[WASAPI]` Promoted to default Windows audio backend (DirectSound is now fallback)
- [x] `[Linux] fplGetSystemLocale` / `fplGetUserLocale` / `fplGetInputLocale` — via `setlocale` + `fpl__PosixLocaleToISO639`
- [x] `[Unix]` Locale init/release with LC_ALL save/restore in `fpl__UnixInitPlatform` / `fpl__UnixReleasePlatform`

## Earlier (since 2026-05-11 baseline)

- [x] `[X11] fplSetWindowCursorEnabled`
- [x] `[X11]` Resizable / Floating / WindowState / display surface (count, get, primary, from-position, modes, window-display, fullscreen-rect)
- [x] `[X11]` Clipboard get/set, screen-relative cursor position
- [x] `[Audio]` GetAudioDeviceInfo for DirectSound, ALSA, PulseAudio, PipeWire
- [x] `[ALSA]` Bidirectional fplAudioFormatType <-> snd_pcm_format_t mapping tables
- [x] Resampler rounding fix (44100 <-> 48000 round-trip safe)
- [x] `[Win32]` fpl__Win32EnterFullscreen multi-monitor + top/bottom rect fix

## Platform / Architecture Coverage

- [ ] Arm32 — only partial support
- [ ] Arm64 — only partial support
- [ ] Unix/BSD — only partial support
- [ ] Raspberry Pi — only partial support

## CPU

- [ ] `fplCPUGetCapabilities` for non-x86 architectures (line 13955)
- [ ] `fplCPUGetName` for non-x86 architectures (line 13960)
- [ ] Generic non-x86 CPU query path (Known Limitation)

## Window

- [ ] `[Win32]` Read ACL for full permission detection (line 19261)
- [ ] `[X11]` Map OEM1–OEM8 key (line 22963)
- [ ] `[X11]` Display resolution + refresh-rate change (line 24804)

## Unix / Linux Platform

- [x] ~~`[Linux] fplMemoryGetInfos`~~ — renamed to `fplMemoryGetUsage`, implemented via `sysinfo()` + `/proc/meminfo`
- [x] ~~`[Unix] fplMemoryGetInfos`~~ — renamed to `fplMemoryGetUsage`, implemented via sysctl on FreeBSD (xswdev iteration for swap)
- [ ] `[Unix] fplPollGamepadStates` (line 26043) — Known Limitation: no unix gamepad support (fd backend planned)

## Audio

- [x] ~~Supported-formats list~~ — dropped: `fplAudioDeviceInfoExtended` removed, `fplGetAudioDeviceInfo` returns plain `fplAudioDeviceInfo`. Native format already available via `fplGetAudioHardwareFormat`.
- [ ] `[ALSA]` Optional `dmix`/`hw` device id `:%d,%d` probe (line 31369)
- [ ] `[PipeWire]` Remove PipeWire include when runtime linking is enabled (line 33949)
- [ ] Audio backend descriptor from audio settings (line 35362)
- [x] ~~OSS backend — planned, not implemented~~ — done, available on Linux and BSD/Unix
- [x] ~~WASAPI backend~~ — done, default on Windows

## Video

- [ ] Vulkan message-type filtering in `fplVulkanSettings` (line 27881)
- [ ] Validate Vulkan video settings (line 27928)

## Internal

- [ ] Replace pointer mapping table with static offset table (line 25215)
- [ ] Crash path when feature not implemented is full-on crash (no graceful fallback)

## Source markers (as of 2026-05-15)

- `@IMPLEMENT` markers remaining: 3
  - CPU caps / CPU name for non-x86 (13955, 13960)
  - fplPollGamepadStates Unix (26043)
- `@TODO` markers remaining: 9
  - Win32 Read ACL (19261)
  - X11 OEM1-8 keys (22963)
  - X11 resolution/refresh change (24804)
  - Pointer-table → offset-table (25215)
  - Vulkan message filter (27881)
  - Vulkan settings validation (27928)
  - ALSA dmix/hw probe (31369)
  - PipeWire include under runtime linking (33949)
  - Audio backend descriptor from settings (35362)
- `NotImplemented` audio device-info returns: 0 (all backends implement GetAudioDeviceInfo)

