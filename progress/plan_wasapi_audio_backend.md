# Plan: WASAPI Audio Backend for FPL

Status: Draft. Branch: `wasapi-audio-backend`.

Each phase below is a self-contained unit of work that ends in a compiling tree with the existing audio demos still functional. Commit at the end of every phase.

## Goal

Add a WASAPI playback backend to `final_platform_layer.h`, alongside the existing DirectSound backend, with:

- Pure C (C99). No C++. No exception handling.
- Runtime-linked. All WASAPI/COM/MMDevice functions wrapped in an API container, mirroring `fpl__DirectSoundApi`.
- Default to enabled on Windows. Disable via `FPL_NO_AUDIO_WASAPI`.
- Shared mode first; exclusive mode added in a dedicated later phase.
- Render (playback) only for v1. Capture is out of scope.
- Selectable through `fplAudioBackendType_WASAPI`. Auto-probe order after phase 8: WASAPI first, then DirectSound.

## Reference Material

- DirectSound backend: `final_platform_layer.h:28480-29342` (`fpl__DirectSoundApi`, `fpl__AudioBackendDirectSound`, `fpl__global_audioBackendDirectShowDescriptor`).
- OSS backend (most recent addition, similar in style): `final_platform_layer.h:30640-31390`.
- Audio backend function-table API: `final_platform_layer.h:28182-28322`.
- Backend probe/registry: `final_platform_layer.h:34125-34218` (`fpl__global_defaultAudioBackendTypes`, `fpl__GetAudioBackendDescriptors`).
- miniaudio WASAPI reference: `~/Downloads/miniaudio.h:21705+` (`ma_device_init__wasapi`, IID tables, IMMNotificationClient handling, buffer-alignment recovery).

## Non-Goals (v1)

- Capture (microphone) support.
- IMMNotificationClient (device hot-plug / default change rerouting).
- UWP code path (`ActivateAudioInterfaceAsync`). Desktop only.
- Loopback capture.
- Hardware offloading.

---

## Phase Overview

| # | Phase | Outcome | Auto-select WASAPI? |
|---|-------|---------|---------------------|
| 1 | Scaffolding + stubs | Backend visible to probe, all funcs return `NotImplemented` | No |
| 2 | API loader + COM types | Loader works in isolation; still stubs | No |
| 3 | Initialize + Release | COM + enumerator alive; no devices yet | No |
| 4 | GetAudioDevices | Enumeration works | No |
| 5 | GetAudioDeviceInfo | Format probe works | No |
| 6 | InitializeDevice + ReleaseDevice | Device opens; no audio yet | No |
| 7 | Start + Stop + MainLoop | Playback works when explicitly requested | No |
| 8 | Promote in probe order | WASAPI default on Windows | Yes |
| 9 | Exclusive mode | Opt-in exclusive playback path | Yes |
| 10 | Cleanup + changelog | Style sweep, changelog entry | Yes |

Every phase ends with: tree compiles on Windows (MSVC + MinGW), existing audio demos run on DirectSound. Phases 1-7 must not alter DirectSound default selection.

---

## Phase 1 — Scaffolding + Stubs

Goal: WASAPI backend descriptor is visible to `fpl__GetAudioBackendDescriptors`; all function-table entries are stub functions that return `fplAudioResultType_NotImplemented` (or no-op for void returns). Existing builds unaffected because WASAPI is not in the default probe order yet.

Changes:

1. Feature gates near `FPL__SUPPORT_AUDIO_DIRECTSOUND` (~`final_platform_layer.h:2818`):
   ```c
   // FPL_NO_AUDIO_WASAPI = Disable WASAPI audio backend
   #if !defined(FPL_NO_AUDIO_WASAPI) && defined(FPL_PLATFORM_WINDOWS)
   #   define FPL__SUPPORT_AUDIO_WASAPI
   #endif
   ```
   And in the enable block (~line 2914):
   ```c
   #if defined(FPL__SUPPORT_AUDIO_WASAPI)
   #   define FPL__ENABLE_AUDIO_WASAPI
   #endif
   ```
2. Module-name constant `FPL__MODULE_AUDIO_WASAPI "WASAPI"` near `FPL__MODULE_AUDIO_DIRECTSOUND`.
3. Public enum: add `fplAudioBackendType_WASAPI` after `fplAudioBackendType_DirectSound` (~`5208`). Keep `_Last = _Custom`.
4. `fplAudioDeviceID` union (~`5486`): add `wchar_t wasapi[128];` under `#if defined(FPL__ENABLE_AUDIO_WASAPI)`. Confirm with `fplStaticAssert(sizeof(...wasapi) <= 256)`.
5. `fplWasapiAudioSettings` struct and `wasapi` field in `fplSpecificAudioSettings`. `useExclusiveMode` is declared now but ignored until phase 9 so the public ABI does not change later:
   ```c
   typedef struct fplWasapiAudioSettings {
       fpl_b32 useExclusiveMode;           // honored from phase 9 onward
       fpl_b32 noAutoConvertSampleRate;
   } fplWasapiAudioSettings;
   ```
6. New backend section `> AUDIO_BACKEND_WASAPI` between DirectSound and ALSA (~line 29344), wrapped in `#if defined(FPL__ENABLE_AUDIO_WASAPI)`. Contents for this phase only:
   - Empty `fpl__AudioBackendWasapi { int placeholder; }` struct.
   - Stub functions matching every signature in `fplAudioBackendFunctionTable`. Stubs:
     - `Initialize` / `InitializeDevice` / `StartDevice` → return `fplAudioResultType_NotImplemented`.
     - `Release` / `ReleaseDevice` / `StopDevice` → return `true`.
     - `GetAudioDevices` → return `0`.
     - `GetAudioDeviceInfo` → return `fplAudioResultType_NotImplemented`.
     - `MainLoop` / `StopMainLoop` → no-op.
   - `fpl__global_audioBackendWasapiDescriptor` with a freshly generated GUID. `isValid = true`.
7. Probe switch in `fpl__GetAudioBackendDescriptors` (~line 34160): add the `fplAudioBackendType_WASAPI` case wired to the descriptor. **Do not** add WASAPI to `fpl__global_defaultAudioBackendTypes` yet.
8. Name table (~line 35092): `FPL__ENUM_NAME("WASAPI", fplAudioBackendType_WASAPI)`.

Done when:
- Windows MSVC + MinGW builds clean.
- `fplGetAudioBackendName(fplAudioBackendType_WASAPI)` returns `"WASAPI"`.
- Forcing `audioSettings.backend = fplAudioBackendType_WASAPI` causes init to fail cleanly with `NotImplemented` and fall through to DirectSound.

Commit: `WASAPI: Add backend scaffolding (stubs, enum, gates, descriptor)`.

---

## Phase 2 — API Loader + COM Types

Goal: All Win32/COM symbols we will use are resolvable at runtime. Minimal COM vtables compile. No behavioral changes.

Changes (inside the WASAPI backend block only):

1. Add `<mmreg.h>` / `<mmsystem.h>` includes (already used by DirectSound, may already be visible). Do **not** include `<audioclient.h>` or `<mmdeviceapi.h>`.
2. Declare `fpl__WasapiApi`:
   ```c
   typedef struct {
       HMODULE oleLibrary;
       HRESULT (WINAPI *CoInitializeEx)(LPVOID, DWORD);
       void    (WINAPI *CoUninitialize)(void);
       HRESULT (WINAPI *CoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
       void    (WINAPI *CoTaskMemFree)(LPVOID);
       HRESULT (WINAPI *PropVariantClear)(PROPVARIANT*);
   } fpl__WasapiApi;
   ```
3. Implement `fpl__LoadWasapiApi` / `fpl__UnloadWasapiApi` using `FPL__WIN32_LOAD_LIBRARY` and `FPL__WIN32_GET_FUNCTION_ADDRESS` (same pattern as DirectSound).
4. Declare minimal COM interface types (vtables + opaque struct shells) for:
   - `fpl__IMMDeviceEnumerator`, `fpl__IMMDeviceCollection`, `fpl__IMMDevice`, `fpl__IPropertyStore`
   - `fpl__IAudioClient`, `fpl__IAudioRenderClient`

   Each as `struct X { struct XVtbl *lpVtbl; };` with the method pointers we will call. Copy carefully from miniaudio.
5. Declare GUIDs/IIDs/CLSIDs and `PKEY_Device_FriendlyName` as `static const`.
6. Smoke-test loader: in `fpl__AudiobackendWasapiInitialize`, call `fpl__LoadWasapiApi`. On success, immediately unload and still return `NotImplemented`. Confirm no link errors on either toolchain.

Done when: build clean on both toolchains, no behavior change.

Commit: `WASAPI: Add runtime API loader and minimal COM type declarations`.

---

## Phase 3 — Initialize + Release

Goal: `fpl__AudiobackendWasapiInitialize` brings COM and `IMMDeviceEnumerator` up; `Release` tears them down. Still no device support.

Changes:

1. Backend struct gains real fields:
   ```c
   typedef struct {
       fpl__WasapiApi api;
       fpl__IMMDeviceEnumerator *enumerator;
       bool comInitialized;
   } fpl__AudioBackendWasapi;
   ```
2. `Initialize`:
   - Load API.
   - `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)`. `S_OK` → we own; `S_FALSE` or `RPC_E_CHANGED_MODE` → already initialized in this thread, do not pair with `CoUninitialize`.
   - `CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, &enumerator)`.
   - Return `fplAudioResultType_Success`.
3. `Release`:
   - `enumerator->lpVtbl->Release(enumerator)` if set.
   - `CoUninitialize()` if we owned the init.
   - `fpl__UnloadWasapiApi`.

Done when: forcing `backend = fplAudioBackendType_WASAPI` initializes and releases without error, then proceeds to fail on the next stage (`InitializeDevice` still stub `NotImplemented`).

Commit: `WASAPI: Implement Initialize/Release with COM + IMMDeviceEnumerator`.

---

## Phase 4 — GetAudioDevices

Goal: `fplGetAudioDevices` returns real WASAPI render endpoints.

Changes:

1. Implement `fpl__AudiobackendWasapiGetAudioDevices`:
   - `enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection)`.
   - `collection->GetCount(&count)`.
   - For each: `Item(i, &dev)`, `GetId(&pwszId)`, `OpenPropertyStore(STGM_READ, &props)`, `props->GetValue(PKEY_Device_FriendlyName, &pv)`.
   - Convert friendly name UTF-16 → UTF-8 (reuse existing FPL wide→ANSI helper if present; otherwise inline `WideCharToMultiByte`).
   - Determine default: `enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defDev)`; `defDev->GetId(&defId)`; `wcscmp`.
   - Copy `pwszId` into `fplAudioDeviceID.wasapi`. Truncate gracefully if longer than 127 wchars (log a warning).
   - Free with `CoTaskMemFree`; release every COM ref.

2. Add a `static inline` helper `fpl__WasapiCopyDeviceID(wchar_t *dst, size_t dstCap, const wchar_t *src)` to centralize the truncation and null-termination.

Done when: a demo that lists audio devices shows all WASAPI endpoints with correct names and the default flag set.

Commit: `WASAPI: Implement GetAudioDevices via IMMDeviceCollection`.

---

## Phase 5 — GetAudioDeviceInfo

Goal: `fplGetAudioDeviceInfo` returns a `fplAudioDeviceInfoExtended` with supported formats for a given WASAPI device id.

Changes:

1. Implement `fpl__AudiobackendWasapiGetAudioDeviceInfo`:
   - Resolve device: `enumerator->GetDevice(targetDevice->wasapi, &dev)` (or default endpoint if id empty).
   - `dev->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, &client)`.
   - `client->GetMixFormat(&mixFmt)` — always record this as one supported format.
   - Iterate the same rate × channels × sample-type matrix the other backends use (compare DirectSound / OSS for the canonical lists). For each candidate build a `WAVEFORMATEXTENSIBLE` and call `client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &wfx, &closest)`. Accept `S_OK` exactly; ignore `S_FALSE` for the supported-format list (closest is a hint, not support).
   - Encode each accepted entry via `fplAudioFormatU64`. Cap at 63 entries.
   - `CoTaskMemFree(mixFmt)`, release `client`, release `dev`.

2. Extract `fpl__BuildWaveFormatExtensible(targetFormat, channelMap, &wfx)` from the existing `fpl__SetupWaveFormatDirectSound` (~line 28821) so DirectSound and WASAPI share the same helper. Keep DirectSound calling sites intact.

Done when: a probe demo prints sane format lists for at least the default endpoint.

Commit: `WASAPI: Implement GetAudioDeviceInfo (format probing)`.

---

## Phase 6 — InitializeDevice + ReleaseDevice

Goal: WASAPI device opens with the requested format (or a negotiated equivalent). `Start/Stop/MainLoop` still stubs — no audio plays yet. Backend reports the *output* format and channel map correctly so the rest of FPL can wire up the resampler.

Changes to `fpl__AudioBackendWasapi`:

```c
fpl__IMMDevice          *device;
fpl__IAudioClient       *audioClient;
fpl__IAudioRenderClient *renderClient;
HANDLE bufferEvent;
HANDLE stopEvent;
uint32_t actualBufferSizeInFrames;
volatile bool breakMainLoop;
```

`InitializeDevice` recipe (shared mode):

1. Resolve device: empty id → `GetDefaultAudioEndpoint(eRender, eConsole)`; else `GetDevice(id)`.
2. `dev->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, &audioClient)`.
3. Build initial `WAVEFORMATEXTENSIBLE` via the shared helper.
4. `audioClient->IsFormatSupported(SHARED, &wfx, &closest)`:
   - `S_OK` → use `wfx`.
   - `S_FALSE` and `closest != NULL` → adopt closest, copy locally, `CoTaskMemFree(closest)`.
   - Failure → fall back to `GetMixFormat`.
5. Pick `hnsBufferDuration` from `targetFormat.bufferSizeInFrames` (convert frames to 100-ns units against the final sample rate). Default `periods` from `targetFormat.periods` (clamp 2..4).
6. Build `flags`:
   - Always `AUDCLNT_STREAMFLAGS_EVENTCALLBACK`.
   - If `!audioSettings->wasapi.noAutoConvertSampleRate`: also `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY`.
7. `audioClient->Initialize(SHARED, flags, hnsBufferDuration, 0, &finalWfx, NULL)`. Handle `AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED`:
   - `audioClient->GetBufferSize(&alignedFrames)`.
   - Release `audioClient`. `dev->Activate` again. Recompute `hnsBufferDuration` from `alignedFrames`. Re-Initialize.
   - Cap retries to 2.
8. `bufferEvent = CreateEventW(NULL, FALSE, FALSE, NULL)`; `audioClient->SetEventHandle(bufferEvent)`.
9. `audioClient->GetBufferSize(&actualBufferSizeInFrames)`.
10. `audioClient->GetService(IID_IAudioRenderClient, &renderClient)`.
11. Translate `finalWfx` back to `fplAudioFormat` for `outputFormat`. Fill `outputDevice` and `outputChannelMap` using `fpl__SetAudioDefaultChannelMapWin32` (line 28394).
12. `stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL)`.

`ReleaseDevice`: release renderClient, audioClient, device; close events; clear fields.

Done when: opening + closing the device succeeds repeatedly and prints the negotiated format. No audio yet because `Start` is still a stub.

Commit: `WASAPI: Implement InitializeDevice/ReleaseDevice (shared mode)`.

---

## Phase 7 — Start + Stop + MainLoop

Goal: Playback works when WASAPI is explicitly selected (`fplAudioBackendType_WASAPI`).

Changes:

1. `StartDevice`:
   - Pre-fill: `renderClient->GetBuffer(actualBufferSizeInFrames, &dst)`; `fpl__ReadAudioFramesFromClient(backend, actualBufferSizeInFrames, dst)`; `renderClient->ReleaseBuffer(actualBufferSizeInFrames, 0)`.
   - `audioClient->Start()`.

2. `StopDevice`:
   - `audioClient->Stop()`.
   - `audioClient->Reset()`.

3. `MainLoop`:
   ```c
   HANDLE waits[2] = { impl->bufferEvent, impl->stopEvent };
   ResetEvent(impl->stopEvent);
   impl->breakMainLoop = false;
   while (!impl->breakMainLoop) {
       DWORD r = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
       if (r != WAIT_OBJECT_0) { break; }

       uint32_t padding = 0;
       if (FAILED(audioClient->lpVtbl->GetCurrentPadding(audioClient, &padding))) { break; }
       uint32_t framesAvailable = impl->actualBufferSizeInFrames - padding;
       if (framesAvailable == 0) { continue; }

       BYTE *dst = NULL;
       if (FAILED(renderClient->lpVtbl->GetBuffer(renderClient, framesAvailable, &dst))) { break; }
       uint32_t framesRead = fpl__ReadAudioFramesFromClient(backend, framesAvailable, dst);
       fplAssert(framesRead == framesAvailable);
       renderClient->lpVtbl->ReleaseBuffer(renderClient, framesAvailable, 0);
   }
   ```
   Honor the FPL contract: `fpl__ReadAudioFramesFromClient` always fills `frameCount`; never gate on its return.

4. `StopMainLoop`: `impl->breakMainLoop = true; SetEvent(impl->stopEvent);`

Done when:
- Audio demos play correctly at 44.1k and 48k stereo when forcing `backend = fplAudioBackendType_WASAPI`.
- DirectSound demos still work unchanged.
- 60-second soak: no underruns.

Commit: `WASAPI: Implement Start/Stop/MainLoop (event-driven shared mode)`.

---

## Phase 8 — Promote WASAPI in Probe Order

Goal: WASAPI becomes the auto-selected backend on Windows; DirectSound stays as fallback.

Changes:

1. `fpl__global_defaultAudioBackendTypes` (~line 34125): prepend `fplAudioBackendType_WASAPI` before `fplAudioBackendType_DirectSound`.
2. Run every audio demo with default settings; confirm WASAPI is selected via `fplGetAudioBackendType()`. Confirm fall-through to DirectSound when WASAPI is disabled at compile time.

Commit: `WASAPI: Promote to default Windows audio backend (DSound fallback)`.

---

## Phase 9 — Exclusive Mode

Goal: Honor `fplWasapiAudioSettings.useExclusiveMode`. When set, open the endpoint in `AUDCLNT_SHAREMODE_EXCLUSIVE` for lower latency and bit-exact playback. Shared mode stays the default.

Background: exclusive mode bypasses the Windows audio engine. The driver dictates supported formats; `AUTOCONVERTPCM` and `SRC_DEFAULT_QUALITY` are not allowed. `Initialize` returns `AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED` more aggressively, and the period alignment rules from `GetDevicePeriod` must be respected. Reference: miniaudio `ma_device_init__wasapi` exclusive branch.

Changes:

1. `fpl__AudioBackendWasapi`: track `bool isExclusive`.
2. `InitializeDevice` — split the existing shared-mode path into a small dispatcher:
   - If `audioSettings->wasapi.useExclusiveMode` is set, call `fpl__WasapiInitDeviceExclusive`. On failure, log a warning and fall back to shared mode (do not fail the whole init unless the user passed a "strict" flag — for v1 always fall back).
   - Otherwise call `fpl__WasapiInitDeviceShared` (the existing phase-6 logic, refactored into its own function).
3. `fpl__WasapiInitDeviceExclusive` recipe:
   - Resolve device + activate `IAudioClient` exactly as in shared mode.
   - Build `WAVEFORMATEXTENSIBLE` from the requested format.
   - `IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &wfx, NULL)` — exclusive mode passes `NULL` for closest; on `S_FALSE`/failure, retry by walking candidate formats (44.1k, 48k, 96k × 16-bit, 24-bit-in-32, 32-bit-float × current channel count). First one that returns `S_OK` wins. If nothing matches, return `fplAudioResultType_UnsuportedDeviceFormat`.
   - `GetDevicePeriod(&defaultPeriod, &minimumPeriod)`. Use `defaultPeriod` unless the caller asked for a smaller `bufferSizeInFrames` (compute requested period as `(bufferSizeInFrames * 10000000) / sampleRate / periods`; clamp to `minimumPeriod`).
   - Flags: `AUDCLNT_STREAMFLAGS_EVENTCALLBACK` only. **No** `AUTOCONVERTPCM`, **no** `SRC_DEFAULT_QUALITY`.
   - `audioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, flags, period, period, &wfx, NULL)`. (In exclusive event-driven mode the two duration args must be equal.)
   - On `AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED`:
     - `GetBufferSize(&alignedFrames)`.
     - Compute aligned `period = ((10000000 * alignedFrames) / sampleRate) + 0.5` (rounded). Use 64-bit math; do not introduce float.
     - Release `audioClient`, re-`Activate`, re-`Initialize` with the aligned period. Cap retries at 2 (same as the shared-mode recovery).
   - Continue with `SetEventHandle`, `GetBufferSize`, `GetService(IID_IAudioRenderClient)`, output-format/channel-map translation, `stopEvent` creation — identical to the shared-mode path.
   - Set `isExclusive = true`.
4. `MainLoop` is unchanged — the event-driven render loop already works for exclusive mode.
5. `StartDevice` / `StopDevice` are unchanged.
6. Documentation in the settings struct: update the `useExclusiveMode` comment to describe the fallback behavior.

Done when:
- With `useExclusiveMode = 0`: behavior identical to phase 7/8.
- With `useExclusiveMode = 1` on a device that supports exclusive: playback works, latency measurably lower than shared mode. `fplGetAudioDeviceInfo` of the running device reports the exact requested format.
- With `useExclusiveMode = 1` on a device that does **not** support the requested format in exclusive mode: warning logged, falls back to shared mode without failing init.
- 60-second soak in exclusive mode at 44.1 kHz and 48 kHz stereo: no underruns.

Commit: `WASAPI: Implement exclusive-mode playback path`.

---

## Phase 10 — Cleanup + Changelog

Goal: Conform to repo style and document the addition.

Changes:

1. Style sweep on the new code: braces on all conditionals (`feedback_block_style`); single-line comments, minimal (`feedback_comment_style`); never branch on `fpl__ReadAudioFramesFromClient`'s return (`feedback_read_audio_frames_contract`).
2. Top-of-file changelog block: add an entry describing the new backend, the new enum value, the new device-id field, the new settings struct, the new gate.
3. Update `README.txt` if it lists supported backends.

Commit: `WASAPI: Update changelog and finalize style sweep`.

---

## Testing Matrix (apply at each phase end)

- MSVC build (Debug + Release).
- MinGW-w64 build.
- Audio demo (`demos/FPL_Audio` or equivalent) with:
  - Default settings (DirectSound until phase 8, WASAPI after).
  - `--backend wasapi` (explicit) — phases 3+ to whatever is implemented.
  - `--backend dsound` — must always work.
- For phase 7+: 60-second soak at 44.1 kHz and 48 kHz stereo. Watch for underruns and for the SinC resampler rounding interaction (`feedback_sinc_resampler_rounding`).

## Risks / Open Questions

- **COM thread model.** Initialize COM on the audio worker thread, not on the caller's thread, to match DirectSound's behavior.
- **`IsFormatSupported` quirks.** Some drivers return `S_FALSE` with no `closestMatch`; fall back to `GetMixFormat`.
- **Double SRC.** `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM` does WASAPI-side SRC. Combined with FPL's internal resampler this is double-SRC; the toggle (`noAutoConvertSampleRate`) lets users disable WASAPI's side.
- **MinGW headers.** `PROPERTYKEY` / `PROPVARIANT` may already be defined — guard with `#ifndef`.
- **No SDK headers.** Hand-rolled COM types must stay binary-compatible with the real interfaces; copy IIDs and method orders exactly from miniaudio.

## Out-of-Scope Follow-ups

- `IAudioClient3` ultra-low-latency shared mode (`InitializeSharedAudioStream`).
- Capture endpoints (`eCapture`) and loopback.
- IMMNotificationClient for device-change events.
- UWP `ActivateAudioInterfaceAsync` path.
