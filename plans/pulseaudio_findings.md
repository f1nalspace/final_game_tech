# PulseAudio Backend - Findings & Remaining Work

## User-reported issues (still open)

1. **`fpl__AudioBackendPulseAudioGetAudioDevices` returns 0** — no device iteration implemented.
2. **Neither FPL_SimpleAudio nor FPL_AudioPlayer plays sound** — write callback fires only once instead of continuously.
3. **`InitializeDevice` ignores `targetDevice`** — user cannot select a sink via `fplAudioDeviceInfo`.

## Root cause analysis

### Issue 2 — write callback fires once

Bug is in `fpl__PulseAudio_StreamWriteCallback` at final_platform_layer.h:26638.

`fpl__ReadAudioFramesFromClient` **returns `samplesRead` (= frames × channels), NOT frames** — confirmed at final_platform_layer.h:23722. The function name is misleading. All other backends (ALSA, DirectSound) simply discard the return value and rely on the function's built-in zero-padding to fill the full requested buffer.

In our write callback we currently:
```c
uint32_t producedFrames = fpl__ReadAudioFramesFromClient(backend, requestedFrames, destinationBuffer);  // WRONG: samples, not frames
size_t producedBytes = producedFrames * frameSize;     // over-computed by factor of channel count
...
if (producedFrames < requestedFrames) break;           // almost always breaks after 1st iter
remainingBytes -= producedBytes;                       // underflows
```

**Effect**: first call writes huge bogus byte count (channels× too much) into the stream, then we `break` out of the loop. PulseAudio is now stuffed with bytes and doesn't fire the write callback again for a long time → "callback fires once" symptom.

**Fix**: always write the full clamped chunk; do not use the return value; clamp `chunkBytes` down to a frame multiple.

#### Note from user

We well fix the return value of `fpl__ReadAudioFramesFromClient`, but you should know that this function always writes samples for all audio frames (frameCount).
If there is not enough samples from the user, the remaining samples are filled with silence always.

### Issue 1 — GetAudioDevices

`pa_context_get_sink_info_list` requires a running mainloop + connected context. Our backend currently has none at this entry point (we only load the api in `initialize`). Fix requires spinning up a temporary threaded mainloop + context just for enumeration, then tearing it down.

Needs:
- Dedicated enumeration session struct (separate from the persistent `fpl__PulseAudioBackend`)
- Dedicated context-state + sink-info-list callbacks that signal via the enumeration session (not via the persistent backend)
- `pa_sink_info` fields used: `name` (internal sink name), `index` (→ `id.pulse`), `description` (→ `fplAudioDeviceInfo.name`)

#### Note from user
fplGetAudioDevices() can only be used when fplPlatformInit() has been called, so audio initialization is complete at this point.
Therefore you can simply add fields that are required for device enumeration into the fpl__PulseAudioBackend struct and use it in the getAudioDevices function of the descriptor table.
I would recommend adding a sub struct for that is contained in the fpl__PulseAudioBackend struct, maybe call this struct fpl__PulseDeviceIterationContext or something. I am bad at names.

### Issue 3 — targetDevice ignored

`pa_stream_connect_playback` takes a sink **name string**, not an index. `id.pulse` holds a numeric index (per user preference). Resolution must happen at connect time:

1. If `targetDevice != NULL` and `fplGetStringLength(targetDevice->name) > 0`, call `pa_context_get_sink_info_by_index(targetDevice->id.pulse)` with a name-lookup callback.
2. Store resolved internal sink name in a local buffer.
3. Pass that buffer to `pa_stream_connect_playback` instead of `NULL`.

Mirrors ALSA's pattern at final_platform_layer.h:25569-25607 (check pointer, then check empty string, else use default).

## Async play/stop bug (fixed by user)

User added `result = fpl__StartAudioDevice(audioState);` in `fplPlayAudio` async path at final_platform_layer.h:28420 so backend start failures now propagate. Previously the result was silently ignored.

## Plan for next commits

1. Fix `fpl__ReadAudioFramesFromClient` - to return number of audio frames instead of audio samples, but not change the behavior! Note that the return value was never used anywhere. See my notes in `Issue 1`.
2. Change `fpl__PulseAudio_StreamWriteCallback` to simply call `fpl__ReadAudioFramesFromClient` and expect that all frames/samples are written (remaining samples are always filled with empty ones)
3. Add `fpl__PulseAudio_EnumerationSession` struct + dedicated enumeration callbacks.
4. Implement `fpl__AudioBackendPulseAudioGetAudioDevices` with its own temporary mainloop+context, then teardown.
5. Add `fpl__PulseAudio_ResolveSinkNameByIndex` helper invoked from `InitializeDevice` to translate `targetDevice->id.pulse` into a sink name for `pa_stream_connect_playback`.
6. Rebuild both demos.
