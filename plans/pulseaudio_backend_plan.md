# PulseAudio Backend Implementation Plan

## Goal
Add PulseAudio audio backend to `final_platform_layer.h`, modeled after ALSA but running
as an **async backend** (isAsync=true) using PulseAudio's own threaded mainloop. No own
audio thread is spawned; audio data is pulled from the client in PulseAudio's stream
write callback.

## Targets
- Only active on `FPL_PLATFORM_LINUX`
- New macros: `FPL_NO_AUDIO_PULSEAUDIO`, `FPL__SUPPORT_AUDIO_PULSEAUDIO`, `FPL__ENABLE_AUDIO_PULSEAUDIO`
- Runtime dynamic load of `libpulse.so.0` (no pulse dev headers required)
- Anonymous types under `FPL__ANONYMOUS_PULSEAUDIO_HEADERS` (default on, since we do not
  require dev headers)
- No format conversion, no resampling — PulseAudio may pick a native format
- Does not break existing code; fully compilable out

## Tasks

### 1. Preprocessor setup (line ~2626)
- Update comment block to mention `FPL_NO_AUDIO_PULSEAUDIO`
- Add `FPL__SUPPORT_AUDIO_PULSEAUDIO` definition under `FPL__SUPPORT_AUDIO` block,
  guarded by `FPL_PLATFORM_LINUX` and `!FPL_NO_AUDIO_PULSEAUDIO`
  - No `fplHasInclude` check — we runtime-load, not header-require
- Add `FPL__ENABLE_AUDIO_PULSEAUDIO` mirror inside the enable block

### 2. Enum `fplAudioBackendType` (line ~4880)
- Add `fplAudioBackendType_PulseAudio` between `Alsa` and `Custom`
- Keep `Custom` as `_Last`
- Add name table entry at line ~27070
- Add to `fpl__global_defaultAudioBackendTypes` at line ~26128
- Add case in `fpl__GetAudioBackendDescriptors` at line ~26170

### 3. Device ID union (line ~5160)
- Add `uint32_t pulse;` field (PulseAudio uses numeric stream/sink indexes — prefer
  numeric per user request). Actual sink selection uses sink index; name-based lookup
  can still resolve from that index at query time.
- Guarded by `FPL__ENABLE_AUDIO_PULSEAUDIO`

### 4. Specific settings (line ~5205)
- Add `fplPulseAudioSettings` struct with:
  - `char serverName[256]` — optional pulse server
  - `char appName[256]` — client app name shown in pavucontrol
  - `char streamName[256]` — stream description
- Add `fplPulseAudioSettings pulse;` field in `fplSpecificAudioSettings` union

### 5. Create `FPL__MODULE_AUDIO_PULSEAUDIO` macro (near ALSA module at ~9167)

### 6. PulseAudio backend preprocessor block (directly after ALSA, line ~26000)
All wrapped in `#if defined(FPL__ENABLE_AUDIO_PULSEAUDIO)`:

#### 6a. Channel map helper
- `fpl__SetAudioDefaultChannelMapPulseAudio()` — same structure as ALSA helper

#### 6b. Anonymous types under `FPL__ANONYMOUS_PULSEAUDIO_HEADERS`
Prefix custom types `pa_*` matching pulse ABI:
- `typedef void pa_mainloop_api;`
- `typedef void pa_threaded_mainloop;`
- `typedef void pa_context;`
- `typedef void pa_stream;`
- `typedef void pa_operation;`
- `typedef void pa_proplist;`
- Enums:
  - `pa_context_state_t` (UNCONNECTED, CONNECTING, AUTHORIZING, SETTING_NAME, READY, FAILED, TERMINATED)
  - `pa_stream_state_t` (UNCONNECTED, CREATING, READY, FAILED, TERMINATED)
  - `pa_sample_format_t` (U8, ALAW, ULAW, S16LE, S16BE, FLOAT32LE, FLOAT32BE, S32LE, S32BE, S24LE, S24BE, S24_32LE, S24_32BE, INVALID=-1)
  - `pa_operation_state_t` (RUNNING, DONE, CANCELLED)
  - `pa_channel_position_t` (long enum)
- Structs:
  - `pa_sample_spec { pa_sample_format_t format; uint32_t rate; uint8_t channels; }`
  - `pa_channel_map { uint8_t channels; pa_channel_position_t map[32]; }`
  - `pa_buffer_attr { uint32_t maxlength, tlength, prebuf, minreq, fragsize; }`
  - `pa_cvolume { uint8_t channels; uint32_t values[32]; }`
  - `pa_sink_info { const char *name; uint32_t index; const char *description; pa_sample_spec sample_spec; pa_channel_map channel_map; ... }` (only need pointers)
- Stream flag constants: `PA_STREAM_START_CORKED`, `PA_STREAM_FIX_FORMAT`, `PA_STREAM_FIX_RATE`, `PA_STREAM_FIX_CHANNELS`, `PA_STREAM_ADJUST_LATENCY`, `PA_STREAM_AUTO_TIMING_UPDATE`
- Context flag constants: `PA_CONTEXT_NOFLAGS`
- Seek constant: `PA_SEEK_RELATIVE = 0`

#### 6c. Function typedefs (fpl__pa_func_* and macros FPL__PULSEAUDIO_FUNC_*)
Subset needed:
- `pa_threaded_mainloop_new`, `_free`, `_start`, `_stop`, `_lock`, `_unlock`, `_wait`, `_signal`, `_get_api`
- `pa_context_new`, `_unref`, `_set_state_callback`, `_connect`, `_disconnect`, `_get_state`, `_get_sink_info_list`, `_get_sink_info_by_index`, `_get_sink_info_by_name`
- `pa_stream_new`, `_unref`, `_set_state_callback`, `_set_write_callback`, `_connect_playback`, `_disconnect`, `_get_state`, `_begin_write`, `_write`, `_cancel_write`, `_cork`, `_flush`, `_is_corked`, `_writable_size`, `_get_sample_spec`, `_get_channel_map`, `_get_device_name`, `_get_buffer_attr`
- `pa_operation_unref`, `_get_state`
- `pa_channel_map_init_auto`, `pa_channel_map_init_stereo`
- `pa_strerror`
- `pa_frame_size`, `pa_bytes_per_second` (helpers; may be macro in real header → runtime load of function forms)
- `pa_proplist_new`, `_free`, `_sets` (optional)

#### 6d. `fpl__PulseAudioApi` struct + `fpl__LoadPulseAudioApi()` / `fpl__UnloadPulseAudioApi()`
- Library name: `libpulse.so.0`, `libpulse.so`

#### 6e. `fpl__PulseAudioBackend` struct
Fields:
- `fpl__PulseAudioApi api;`
- `pa_threaded_mainloop *mainloop;`
- `pa_context *context;`
- `pa_stream *stream;`
- `uint32_t sinkIndex;`
- `bool isContextReady;`
- `bool isStreamReady;`
- `char appName[256];` / `streamName[256];`
- `volatile int32_t breakMainLoop;`

#### 6f. Stubs (all 10 backend table funcs)
- initialize / release
- getAudioDevices / getAudioDeviceInfo
- initializeDevice / releaseDevice
- startDevice / stopDevice
- mainLoop / stopMainLoop

Plus internal helpers (all use full descriptive names with `fpl__PulseAudio_` prefix):
- `fpl__PulseAudio_ContextStateCallback()`
- `fpl__PulseAudio_StreamStateCallback()`
- `fpl__PulseAudio_StreamWriteCallback()` (pulls from client via `fpl__ReadAudioFramesFromClient`)
- `fpl__PulseAudio_StreamSuccessCallback()`
- `fpl__PulseAudio_SinkInfoListCallback()` (device enumeration)
- `fpl__PulseAudio_SinkInfoByIndexCallback()`
- `fpl__PulseAudio_WaitForOperation()`
- `fpl__PulseAudio_MapAudioFormatTypeToSampleFormat()`
- `fpl__PulseAudio_MapSampleFormatToAudioFormatType()`
- `fpl__PulseAudio_SetAudioDefaultChannelMap()`

Naming rules:
- No short names anywhere. Never `writeCb`, `ctxCb`, `ml`, etc.
- Always `fpl__` internal prefix.
- Always backend prefix `PulseAudio_` after `fpl__`.
- Descriptive suffix describing the role (e.g. `StreamWriteCallback`, `ContextStateCallback`).
- Local variables inside functions also avoid short cryptic names — use `mainloop`, `context`, `stream`, `sampleSpec`, `channelMap`, `bufferAttr`, `pulseApi` (not `ml`, `ctx`, `s`, `ss`, `cm`, `ba`, `api`).

#### 6g. `fpl__global_audioBackendPulseAudioDescriptor`
- GUID: `{ 0x5f3c8e2a, 0x4b1d, 0x4f9e, { 0xa8, 0x72, 0x91, 0xd5, 0xcc, 0x63, 0x17, 0xe8 } }`
- name = `"PulseAudio"`
- type = `fplAudioBackendType_PulseAudio`
- isAsync = **true**
- Populate all 10 table entries

### 7. Real implementation of stubs
Async flow:
- `initialize`: load API. No global pulse state created yet.
- `initializeDevice`:
  1. Create threaded mainloop, start it
  2. Create context (with proplist app name), connect, wait until READY (via state cb + signal)
  3. Build `pa_sample_spec` from targetFormat
  4. Build `pa_channel_map` using `pa_channel_map_init_auto`
  5. Create stream with proplist, set state + write callbacks
  6. Build `pa_buffer_attr` from targetFormat buffer/period (tlength = periods × periodFrames × frameSize)
  7. `pa_stream_connect_playback` with `PA_STREAM_START_CORKED | ADJUST_LATENCY`
  8. Wait stream READY
  9. Read back actual sample spec + channel map from stream → fill `outputFormat` / `outputChannelMap`
  10. Fill `outputDevice` from sink info
- `startDevice`:
  - Uncork stream (`pa_stream_cork(stream, 0, ...)`). Since `isAsync`, `fplPlayAudio`
    calls this and sets state to `Started` directly — no worker thread.
- `stopDevice`:
  - Cork stream (`pa_stream_cork(stream, 1, ...)`).
- `mainLoop` / `stopMainLoop`: **NOP** for async backend. The caller's code paths
  with `isAsyncBackend==true` skip calling these except via the worker thread, which
  is never created. Still provide no-op stubs so the function pointers are non-null.
- `releaseDevice`:
  - Disconnect stream, unref, disconnect context, unref, stop + free mainloop.
- `release`:
  - `fpl__AudioBackendPulseAudioReleaseDevice()` then unload API.

Write callback (async heart) — sketch with full descriptive naming:
```
static void fpl__PulseAudio_StreamWriteCallback(pa_stream *stream, size_t requestedBytes, void *userData) {
    fpl__PulseAudioBackend *pulseAudioBackend = (fpl__PulseAudioBackend *)userData;
    const fpl__PulseAudioApi *pulseApi = &pulseAudioBackend->api;
    fplAudioBackend *audioBackend = pulseAudioBackend->owningBackend;
    const uint32_t frameSize = audioBackend->internalFormat.channels * fplGetAudioSampleSizeInBytes(audioBackend->internalFormat.type);
    size_t remainingBytes = requestedBytes;
    while (remainingBytes >= frameSize) {
        void *destinationBuffer = fpl_null;
        size_t chunkSize = remainingBytes;
        if (pulseApi->pa_stream_begin_write(stream, &destinationBuffer, &chunkSize) < 0) break;
        uint32_t requestedFrames = (uint32_t)(chunkSize / frameSize);
        uint32_t producedFrames = fpl__ReadAudioFramesFromClient(audioBackend, requestedFrames, destinationBuffer);
        size_t producedBytes = producedFrames * frameSize;
        pulseApi->pa_stream_write(stream, destinationBuffer, producedBytes, fpl_null, 0, PA_SEEK_RELATIVE);
        if (producedBytes == 0 || producedFrames < requestedFrames) break;
        remainingBytes -= producedBytes;
    }
}
```

### 8. Build validation
- Compile `demos/FPL_SimpleAudio` (gcc/clang)
- Compile `demos/FPL_AudioPlayer`
- Both must compile cleanly with and without PulseAudio enabled (default enabled on Linux)

## Manual testing (user-driven)
- FPL_SimpleAudio (sine wave)
- FPL_AudioPlayer
- FPL_WavePlayer
- FPL_FFMpeg
- FPL_Crackout

## Confirmation checkpoints
Each task is confirmed by user before moving to the next, per user instruction.
