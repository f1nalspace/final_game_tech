# FPL — Unimplemented / Partial TODO

Tracking missing or partial implementations in `final_platform_layer.h`.
Sourced from `@IMPLEMENT`, `@TODO`, `NotImplemented` returns, and Known Limitations section.

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

- [ ] `fplCPUGetCapabilities` for non-x86 architectures (line 13544)
- [ ] `fplCPUGetName` for non-x86 architectures (line 13549)
- [ ] Generic non-x86 CPU query path (Known Limitation)

## Window / Display (X11)

- [ ] `fplSetWindowCursorEnabled` (line 23595)
- [ ] `fplIsWindowResizable` (line 23623)
- [ ] `fplSetWindowResizeable` (line 23628)
- [ ] `fplIsWindowFloating` (line 23674)
- [ ] `fplSetWindowFloating` (line 23679)
- [ ] `fplGetWindowState` (line 23683)
- [ ] `fplSetWindowState` (line 23688)
- [ ] `fplGetDisplayCount` (line 23693)
- [ ] `fplGetDisplays` (line 23698)
- [ ] `fplGetPrimaryDisplay` (line 23703)
- [ ] `fplGetWindowDisplay` (line 23708)
- [ ] `fplGetDisplayFromPosition` (line 23713)
- [ ] `fplGetDisplayModes` (line 23718)
- [ ] `fplSetWindowFullscreenRect` (line 23749)
- [ ] `fplGetClipboardText` (line 23819)
- [ ] `fplSetClipboardText` (line 23824)
- [ ] `fplQueryCursorPosition` (line 23852)
- [ ] Display resolution + refresh rate switching in X11 (line 23739)
- [ ] X11 map OEM1-OEM8 key (line 22461)

## Window (Win32)

- [ ] Multi-monitor origin assumption may be wrong — `fplGetWindowDisplay` uses 0,0 (line 15174)
- [ ] Read ACL for full permission detection (line 18845)

## Unix Platform

- [ ] `fplMemoryGetInfos` (line 24557) — Known Limitation: no unix memory query
- [ ] `fplGetSystemLocale` (line 24565)
- [ ] `fplGetUserLocale` (line 24570)
- [ ] `fplGetInputLocale` (line 24575)
- [ ] POSIX `st_atim` vs `st_atime` fallback detection for older POSIX (line 11083)
- [ ] `sched_getscheduler` POSIX standard coverage check (line 20355)
- [ ] LC_ALL restore in `fpl__LinuxReleasePlatform` (line 23873)

## Audio

- [ ] `[DirectSound] fpl__AudioBackendDirectSoundGetAudioDeviceInfo` — returns `NotImplemented` (line 27329-27331)
- [ ] `[ALSA] fpl__AudioBackendAlsaGetAudioDeviceInfo` — returns `NotImplemented` (line 29131-29133)
- [ ] `[PulseAudio] fpl__AudioBackendPulseAudioGetAudioDeviceInfo` — returns `NotImplemented` (line 30018-30023)
- [ ] `[PipeWire] fpl__AudioBackendPipeWireGetAudioDeviceInfo` — returns `NotImplemented` (line 31472-31477)
- [ ] `[ALSA]` Mapping table `fplAudioFormatType -> snd_pcm_format_t` (line 28605)
- [ ] `[ALSA]` Mapping table `snd_pcm_format_t -> fplAudioFormatType` (line 28650)
- [ ] `[ALSA]` Optional `dmix`/`hw` device id `:%d,%d` probe (line 28810, 29119)
- [ ] `[PipeWire]` Remove PipeWire include when runtime linking is enabled (line 30579)
- [ ] Audio backend descriptor from audio settings (line 31944)
- [ ] OSS backend — planned, not implemented (Known Limitation)
- [ ] Extended audio device infos do not contain format list yet (Known Limitation)
- [ ] Resampler edge case — fake-round extra frame workaround (line 32906)

## Input / Gamepad

- [ ] Unix gamepad via raw `fd` — planned, not implemented (Known Limitation)

## Video

- [ ] Vulkan message-type filtering in `fplVulkanSettings` (line 26389)
- [ ] Validate Vulkan video settings (line 26436)

## Internal

- [ ] Replace pointer mapping table with static offset table (line 24001)
- [ ] Crash path when feature not implemented is full-on crash (line 3109)

## Source markers

Total `@IMPLEMENT` / `@TODO` / `FIXME` markers: 17 (as of 2026-05-11)
Total `NotImplemented` audio returns: 4
