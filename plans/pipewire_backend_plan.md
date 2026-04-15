# PipeWire Audio Backend — Implementation Plan

## Goal
Add a second Linux-only audio backend next to the PulseAudio backend in
`final_platform_layer.h`. The new backend must:

- Be runtime-linked against `libpipewire-0.3.so.0` — no dev headers required
- Be completely optional (`FPL_NO_AUDIO_PIPEWIRE` must fully disable it)
- Stay compatible with the existing fplAudioBackendDescriptor system
- Mirror the pulse backend layout, style and error handling
- Perform no format conversion / resampling
- Be production-ready for the demos `FPL_SimpleAudio` and `FPL_AudioPlayer`

## Background — why this is non-trivial

PipeWire ships two kinds of API headers in `libpipewire-0.3`:

1. The normal ABI functions such as `pw_init`, `pw_thread_loop_new`,
   `pw_stream_new_simple`, etc. These are exported symbols in
   `libpipewire-0.3.so.0` and can be `dlsym`-loaded normally.
2. A second set of symbols declared `static inline` in the public headers
   (`pw_core_add_listener`, `pw_core_sync`, `pw_core_get_registry`,
   `pw_registry_add_listener`, `pw_registry_bind`, `pw_proxy_add_listener`,
   etc.). These wrap `spa_interface` vtable dispatch through the header.

   `nm -D /usr/lib/libpipewire-0.3.so.0` confirmed that libpipewire actually
   re-exports all of these as real symbols (the static inlines are compiled
   into the shared library with a non-inline `SPA_API_IMPL` override).
   → We can runtime-link them and we never need the vtable trampoline code.

Format negotiation needs a SPA POD. The SPA POD builder is header-only,
static inline, and there is no runtime library for it. We will build the
`SPA_TYPE_OBJECT_Format` POD for `SPA_PARAM_EnumFormat` manually into a fixed
byte buffer — the POD wire format is documented and stable in 0.3.

## Libraries / symbols we will runtime-link

From `libpipewire-0.3.so.0`:

- `pw_init`, `pw_deinit`, `pw_get_library_version`
- `pw_thread_loop_new`, `pw_thread_loop_destroy`,
  `pw_thread_loop_start`, `pw_thread_loop_stop`,
  `pw_thread_loop_lock`, `pw_thread_loop_unlock`,
  `pw_thread_loop_wait`, `pw_thread_loop_signal`,
  `pw_thread_loop_get_loop`
- `pw_context_new`, `pw_context_destroy`, `pw_context_connect`
- `pw_core_disconnect`, `pw_core_add_listener`, `pw_core_sync`,
  `pw_core_get_registry`
- `pw_registry_add_listener`, `pw_registry_destroy`
- `pw_proxy_destroy`
- `pw_properties_new`, `pw_properties_free`, `pw_properties_set`,
  `pw_properties_setf`
- `pw_stream_new_simple`, `pw_stream_destroy`,
  `pw_stream_connect`, `pw_stream_disconnect`, `pw_stream_set_active`,
  `pw_stream_get_state`, `pw_stream_dequeue_buffer`, `pw_stream_queue_buffer`,
  `pw_stream_get_time_n`

Library names to try:
`libpipewire-0.3.so.0`, `libpipewire-0.3.so`.

## Types we must mirror (all ABI-stable in 0.3)

Opaque handles (`typedef void`):
`pw_loop`, `pw_thread_loop`, `pw_context`, `pw_core`, `pw_registry`,
`pw_stream`, `pw_properties`, `pw_proxy`.

Layout-compatible structs:
- `spa_list` (prev/next)
- `spa_callbacks` (funcs/data)
- `spa_interface` (type/version/spa_callbacks)
- `spa_hook` (link/cb/removed/priv)
- `spa_dict_item`, `spa_dict`
- `spa_pod`, `spa_pod_object_body` (used only indirectly by the POD builder)
- `spa_chunk`, `spa_data`, `spa_buffer`
- `pw_buffer` (used in the realtime process callback)
- `pw_core_info` (only the leading fields we need: id, cookie, user_name,
  host_name, version, name, change_mask, props, params) — we only read
  pointer-address-stable fields at the start
- `pw_core_events` (version, info, done, ping, error, remove_id, bound_id,
  add_mem, remove_mem, bound_props) — we only implement `done` + `error`
- `pw_registry_events` (version, global, global_remove)
- `pw_stream_events` (version, destroy, state_changed, control_info,
  io_changed, param_changed, add_buffer, remove_buffer, process, drained,
  command, trigger_done)

Enumerations / constants:
- `spa_direction` (SPA_DIRECTION_INPUT/OUTPUT)
- `pw_stream_state` (ERROR/UNCONNECTED/CONNECTING/PAUSED/STREAMING)
- `pw_stream_flags` (AUTOCONNECT, MAP_BUFFERS, RT_PROCESS, INACTIVE, ...)
- `PW_ID_ANY`, `PW_ID_CORE`
- `PW_VERSION_CORE`, `PW_VERSION_REGISTRY`, `PW_VERSION_STREAM_EVENTS`,
  `PW_VERSION_REGISTRY_EVENTS`, `PW_VERSION_CORE_EVENTS`
- SPA POD: `SPA_TYPE_None/Bool/Id/Int/Object`, `SPA_TYPE_OBJECT_Format`
- SPA format keys: `SPA_FORMAT_mediaType`, `mediaSubtype`, `AUDIO_format`,
  `AUDIO_rate`, `AUDIO_channels`
- SPA audio format: `SPA_AUDIO_FORMAT_U8`, `S16_LE`, `S16_BE`, `S24_LE`,
  `S24_BE`, `S32_LE`, `S32_BE`, `F32_LE`, `F32_BE`
- SPA param types: `SPA_PARAM_EnumFormat`
- SPA media type/subtype: `SPA_MEDIA_TYPE_audio`, `SPA_MEDIA_SUBTYPE_raw`

All of these will go behind `FPL__ANONYMOUS_PIPEWIRE_HEADERS`. When
`FPL_PIPEWIRE_USE_REAL_HEADERS` is defined the block `#include
<pipewire/pipewire.h>` and pulls the real SPA format-utils header instead.

## Step-by-step task list

### Step 1 — Preprocessor setup
- In the audio preprocessor block (~lines 2626–2685):
  - Document `FPL_NO_AUDIO_PIPEWIRE` in the header comment
  - Define `FPL__SUPPORT_AUDIO_PIPEWIRE` if
    `!FPL_NO_AUDIO_PIPEWIRE && FPL_PLATFORM_LINUX` inside
    `FPL__SUPPORT_AUDIO`
  - Define `FPL__ENABLE_AUDIO_PIPEWIRE` if the support flag is on
- No header include (runtime-linked)

### Step 2 — fplAudioDeviceID
- Add a `uint32_t pipewire;` field guarded by `FPL__ENABLE_AUDIO_PIPEWIRE`
  (numeric node id, matches the PulseAudio `pulse` field choice)

### Step 3 — fplPipeWireAudioSettings
- Add struct with `applicationName[256]`, `streamName[256]`,
  `mediaRole[64]` (Movie/Music/Game etc., optional)
- Add `pipewire` field to `fplSpecificAudioSettings`

### Step 4 — Module + enum wiring
- `FPL__MODULE_AUDIO_PIPEWIRE "PipeWire"`
- Add `fplAudioBackendType_PipeWire` enum entry (after PulseAudio, before
  Custom)
- Add to `fpl__global_defaultAudioBackendTypes[]` (prefer PipeWire before
  PulseAudio — see rationale below)
- Add switch case in `fpl__GetAudioBackendDescriptors`
- Update `fplAudioBackendType_Last` to `Custom` stays, but the static
  assertion `fplAudioBackendType_Custom == fplAudioBackendType_Last` still
  holds

Probe order rationale: PipeWire is the modern default on most current Linux
distros. Putting it before PulseAudio means systems that have both
end up on PipeWire unless the user explicitly asks for PulseAudio. If
PipeWire fails to load we fall back to PulseAudio, then ALSA, then Custom.

### Step 5 — Backend block skeleton
- New `#if defined(FPL__ENABLE_AUDIO_PIPEWIRE)` block directly after the
  PulseAudio `#endif`
- Anonymous header block providing all types described above, guarded by
  `FPL__ANONYMOUS_PIPEWIRE_HEADERS` (default on unless
  `FPL_PIPEWIRE_USE_REAL_HEADERS` is set)
- `fpl__PipeWireAudioApi` struct with `libHandle` + one field per function
  typedef
- `fpl__PipeWireAudioBackend` struct with all runtime state:
  `api`, `threadLoop`, `context`, `core`, `registry`, `stream`,
  `coreListener`, `registryListener`, `streamListener`, `pendingSeq`,
  `isContextReady`, `isContextFailed`, `isStreamReady`, `isStreamFailed`,
  `isEnumerationDone`, `frameSize`, `channelCount`, `streamSampleFormat`,
  names (applicationName, streamName, mediaRole, targetNodeName),
  iteration context (mirroring the pulse `fpl__PulseDeviceIterationContext`)
- `fpl__LoadPipeWireAudioApi()` / `fpl__UnloadPipeWireAudioApi()` that mirror
  the pulse loader (same `FPL__POSIX_LOAD_LIBRARY` +
  `FPL__POSIX_GET_FUNCTION_ADDRESS` macros, fail if any symbol is missing)
- Forward declarations for all 10 backend functions using the
  `FPL_AUDIO_BACKEND_*_FUNC` macros

### Step 6 — Audio format helpers
- `fpl__PipeWire_MapAudioFormatTypeToSampleFormat(fplAudioFormatType)` →
  `SPA_AUDIO_FORMAT_*`
- `fpl__PipeWire_MapSampleFormatToAudioFormatType(uint32_t)` →
  `fplAudioFormatType`
- `fpl__PipeWire_SetAudioDefaultChannelMap(channels, layout, outChannelMap)`
  — copy of the pulse helper, reused because the logical channel positions
  are identical from FPL's perspective.

### Step 7 — SPA POD format builder
- `fpl__PipeWire_BuildAudioFormatPod(uint8_t *buffer, size_t bufferSize,
  uint32_t spaAudioFormat, uint32_t sampleRate, uint32_t channels)` returns
  the total POD size in bytes.
- Layout: one `SPA_TYPE_Object` pod holding five Id/Int props
  (mediaType, mediaSubtype, AUDIO_format, AUDIO_rate, AUDIO_channels). Total
  size ≈ 136 bytes, the buffer is sized at 256 bytes for safety.
- Self-contained, no SPA headers required.

### Step 8 — Core + registry listeners
- Static `pw_core_events` struct pointing at
  `fpl__PipeWire_CoreInfoCallback` (no-op),
  `fpl__PipeWire_CoreDoneCallback`, `fpl__PipeWire_CoreErrorCallback`
  and the other required function pointers set to trampoline stubs
  (`fpl_null` is unsafe — pipewire may invoke them; we provide empty
  functions)
- Static `pw_registry_events` struct with `fpl__PipeWire_RegistryGlobal` +
  `fpl__PipeWire_RegistryGlobalRemove` stubs
- Done callback signals the thread loop, tracks a `pendingSeq` that is
  matched against `pw_core_sync` return value.

### Step 9 — Stream event callbacks
- `fpl__PipeWire_StreamStateChanged` → state transitions
  (PAUSED/STREAMING/ERROR); signals the thread loop and sets
  `isStreamReady` / `isStreamFailed`
- `fpl__PipeWire_StreamProcess` → dequeue buffer, read buffer info, call
  `fpl__ReadAudioFramesFromClient`, queue the buffer back. This is the
  realtime path.
- Stub `destroy`, `io_changed`, `param_changed`, `add_buffer`,
  `remove_buffer`, `drained`, `command`, `trigger_done`, `control_info` —
  all safe no-ops (we must still set the `version` field so pipewire knows
  which callbacks exist)

### Step 10 — fpl__AudioBackendPipeWireInitialize / Release
- `Initialize`: load the API, call `pw_init(NULL, NULL)`. Log library
  version on success.
- `Release`: disconnect + tear down device, call `pw_deinit()`, unload API.

### Step 11 — fpl__AudioBackendPipeWireGetAudioDevices
- Create a one-shot thread loop + context + core + registry flow.
- Lock, start thread loop (but start BEFORE lock so callbacks can fire),
  attach core listener, attach registry listener, call `pw_core_sync` with
  seq=1, wait until the done callback reports back seq==1.
- In `global` callback, filter on `type == "PipeWire:Interface:Node"` and
  `media.class` containing `Audio/Sink`. Extract `node.description` or
  `node.nick` or `node.name` for the display name, and `object.id` as the
  numeric id that goes into `fplAudioDeviceID.pipewire`.
- On done, stop loop, destroy registry, core, context, thread loop, unlock.
- Return number of sinks collected (with overflow logging like pulse).
- **Note:** this flow creates a temporary thread loop per call. The
  initializeDevice path uses its own thread loop separately because it
  needs to stay alive for the stream.

### Step 12 — fpl__AudioBackendPipeWireGetAudioDeviceInfo
- Stub returning `fplAudioResultType_NotImplemented`, same as the pulse
  backend.

### Step 13 — fpl__AudioBackendPipeWireInitializeDevice
- Resolve application / stream / media role names from audioSettings
  (mirrors the pulse code)
- `pw_thread_loop_new` → `pw_context_new` (with the loop) →
  `pw_context_connect` (returns core) → attach core listener →
  build a `pw_properties` for the stream with:
  - PW_KEY_MEDIA_TYPE = "Audio"
  - PW_KEY_MEDIA_CATEGORY = "Playback"
  - PW_KEY_MEDIA_ROLE = settings role or "Game"
  - PW_KEY_APP_NAME = applicationName
  - PW_KEY_NODE_NAME = streamName
  - PW_KEY_NODE_LATENCY = "<bufferSizeInFrames>/<sampleRate>"
  - PW_KEY_AUDIO_CHANNELS = <channels>
  - PW_KEY_AUDIO_RATE = <sampleRate>
  - PW_KEY_TARGET_OBJECT = <targetDevice->name> (if provided)
- `pw_stream_new_simple` with the built properties, stream events, and the
  backend pointer as user data
- Manually build the EnumFormat POD with our requested format
- `pw_stream_connect(stream, SPA_DIRECTION_OUTPUT, PW_ID_ANY,
  AUTOCONNECT | MAP_BUFFERS | RT_PROCESS | INACTIVE, &pod, 1)`
- Wait for stream state → PAUSED (or STREAMING when not INACTIVE) /
  ERROR using a `state_changed` callback + `pw_thread_loop_wait`
- Compute frame size from the negotiated channels/format (we use what we
  requested — PipeWire does NOT tell us via callback unless we parse the
  param_changed POD; skip that since the user already opted out of format
  conversion)
- Fill outputFormat / outputDevice / outputChannelMap, return success.

### Step 14 — fpl__AudioBackendPipeWireReleaseDevice
- Lock thread loop
- If stream: `pw_stream_disconnect` + `pw_stream_destroy`
- If core: spa_hook_remove inline (we unlink manually because
  `spa_hook_remove` itself is not exported from libpipewire — its list ops
  are trivial)
- If core: `pw_core_disconnect`
- If context: `pw_context_destroy`
- Unlock, `pw_thread_loop_stop`, `pw_thread_loop_destroy`
- Zero the flags.

Actually, check one more time: `spa_hook_remove` — if not exported, call
the listener's stored `removed()` function pointer plus inline unlink.
Simpler alternative: re-run with zero hook state after disconnect since
`pw_core_disconnect` already removes the proxy's emitters. We'll rely on
`pw_stream_destroy` / `pw_core_disconnect` / `pw_registry_destroy` doing
the cleanup internally, which matches the pipewire tutorials.

### Step 15 — start/stop/mainLoop
- `StartDevice`: lock, `pw_stream_set_active(stream, true)`, unlock
- `StopDevice`: lock, `pw_stream_set_active(stream, false)`, unlock
- `MainLoop`/`StopMainLoop`: empty — async backend.

### Step 16 — Descriptor
- `fpl__global_audioBackendPipeWireDescriptor` with:
  - GUID: unique, pick a new one (`0x7d2f9b14, 0xae05, 0x4b7e, …`)
  - Name: "PipeWire"
  - Type: `fplAudioBackendType_PipeWire`
  - `backendSize = sizeof(fpl__PipeWireAudioBackend)`
  - `isAsync = true`
  - `isValid = true`
- All ten function pointers wired to the `fpl__AudioBackendPipeWire*`
  implementations.

### Step 17 — Build validation
- Compile `demos/FPL_SimpleAudio` and `demos/FPL_AudioPlayer` with the
  standard Linux build (GCC). The existing makefile / build script is in
  each demo directory.
- No runtime testing here — manual testing is the user's job.

## Coding style notes
- All helpers use `fpl_internal`
- Follow pulse naming: `fpl__PipeWire_*` for helpers,
  `fpl__AudioBackendPipeWire*` for the 10 backend entry points
- Use `FPL__MODULE_AUDIO_PIPEWIRE` in all log calls
- Use `FPL__ERROR`, `FPL_LOG_WARN`, `FPL_LOG_DEBUG` for tracing the same
  spots the pulse code traces
- No new dependencies, no new headers (SPA included only with
  `FPL_PIPEWIRE_USE_REAL_HEADERS`)

## Open risks
- POD byte layout — we own a test that hand-builds it. The format is
  documented and pipewire has been ABI-stable on 0.3 since 2019.
- `spa_hook_remove` cleanup — we'll rely on proxy destruction to release
  listeners rather than unlinking them ourselves.
- `pw_stream_connect` requires a non-null events struct with a `process`
  callback — we provide one.
- Format negotiation is implicit — we only ever set the format we want and
  do not parse `param_changed`. The user has already accepted that
  PulseAudio / PipeWire may return a different format.
