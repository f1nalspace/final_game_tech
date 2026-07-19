# `demos/additions/` Audit — Failure Points, Leaks, Perf, Stability

Audit of the reusable single-header libraries in `demos/additions/`.
Data-only headers (`final_fonts.h`, `final_music.h`) were skipped.

Severity legend:
- **HIGH** — memory-safety / crash / use-after-free reachable with realistic (or attacker-supplied) input.
- **MED** — real bug: wrong results, resource issue, link/portability breakage, or a bounded overflow on an edge case.
- **LOW** — latent defect, minor correctness, or robustness gap.

## Summary

| # | File | Sev | One-line |
|---|------|-----|----------|
| 1 | final_waveloader.h | HIGH | Chunk sizes trusted; no bounds checks → heap over-read / div-by-zero on malformed WAV |
| 2 | final_audiosystem.h | HIGH | `AudioSystemStopOne` walks the play-item list without the lock → race / use-after-free with the audio thread |
| 3 | final_gameplatform.h | HIGH | Early-failure shutdown frees **uninitialized** `fmemMemoryBlock`s |
| 4 | final_xml.h | HIGH | Parser ignores `dataSize`, relies on NUL-termination; `assert()`-as-validation compiled out in release → OOB reads |
| 5 | final_assets.h | MED | `TextureDataAllocate` `w*h` overflows in 32-bit → under-allocation → heap overflow; wrong `components` after forced RGBA decode |
| 6 | final_buffer.h | MED | Unconditional `#define FINAL_BUFFER_IMPLEMENTATION` → multiple-definition link errors; `64 - UINTPTR_MAX` padding is wrong |
| 7 | final_render.h / final_opengl_render.h | MED | Partial command push on arena OOM desyncs the command executor → `remaining -= consumed` underflow → runaway/OOB |
| 8 | final_audioconversion.h | MED | SinC resampler can write `minOut+1` frames into a `minOut`-sized buffer (producer-side overflow) |
| 9 | final_audioconversion.h | MED | SinC calls `sinf()` per tap (~17 per output sample/ch); the precomputed table is never used → severe CPU cost |
| 10 | final_audio.h | MED | `DecibelToAmplitude` uses `pow(20, …)` instead of `pow(10, …)` → wrong gain |
| 11 | final_geometry.h | MED | `AABB3fContainsPoint` missing `abs()` → always reports "inside" on the negative side |
| 12 | final_gameplatform.h | MED | Custom keyboard mappings never populate `keyboardButtonStates->mapped[]` → custom mappings don't drive controller buttons |
| 13 | final_gameplatform.h | MED | Copy-paste guard: `if (audioSys == fpl_null)` after allocating `gamePlatformState` → null state not caught |
| 14 | final_audiosystem.h | MED | Missing null checks after `fplMemoryAllocate` (probe / file load) → crash on OOM |
| 15 | final_random.h | LOW | `RandomU8 % UINT8_MAX` never yields 255; xorshift seed 0 → all-zero output forever |
| 16 | final_debug.h | LOW | `RecordDebugEvent` has no release-mode bound; `InitDebug` doesn't null-check the alloc |
| 17 | final_opengl_render.h | LOW | Text path casts signed `char` to `uint32_t` without `uint8_t` → non-ASCII glyphs mis-render |
| 18 | final_graphics.h | LOW | No null checks on allocations; `DepthBufferClear` count is `int` (overflow on huge buffers) |
| 19 | final_math.h | LOW | `M4fInverse` has no `Determinant == 0` guard → inf/NaN for singular matrices |
| 20 | final_mp3loader.h / final_vorbisloader.h | LOW | No null check after decode-buffer alloc; div-by-zero if channel count is 0 |
| 21 | final_fontloader.h | LOW | `atlasWidth * atlasHeight` (uint32) can overflow; dead `characterRange` local |
| 22 | final_utils.h | LOW | `FormatSize`: `if (value < 0)` on a `size_t` is always false |
| 23 | final_log.h | LOW | `buffer[len] = '\n'` relies on the `fplStringFormatArgs` truncation contract — verify it returns the truncated length, not the would-be length |

---

## 1. final_waveloader.h — trusts chunk sizes, no bounds checks (HIGH)

`LoadWaveFromBuffer` / `LoadWaveFormatFromBuffer` (lines ~177–249, ~114–175) and `ConvertWaveFormatExToPCMWaveFormat` (line ~88).

Problems:
- `TestWaveHeader` only checks `bufferSize >= sizeof(WaveHeader)` (12 bytes). The chunk loop then reads `chunk->id` / `chunk->size` (8 bytes) at `buffer + 12` **without** verifying `bufferPosition + sizeof(WaveChunk) <= bufferSize`.
- The `Format` case reads a full `WaveFormatEx` (18 bytes) at `buffer + bufferPosition` with no length check.
- The `Data` case takes `dataSize = chunk->size` (fully attacker-controlled, up to 4 GB), then:
  ```c
  size_t sampleMemorySize = bytesPerSample * channelCount * frameCount;
  fplAssert(sampleMemorySize == dataSize);           // assert only — gone in release
  outWave->isamples = fplMemoryAllocate(sampleMemorySize);
  fplMemoryCopy(data, sampleMemorySize, outWave->isamples);  // reads dataSize bytes from the buffer
  ```
  There is **no check that `bufferPosition + dataSize <= bufferSize`**. A truncated or hostile WAV with a large `data` chunk size causes a massive out-of-bounds read (crash / info leak). The end-of-loop test guards only the *next* chunk pointer; the *current* chunk body is already read.
- `ConvertWaveFormatExToPCMWaveFormat`: `frameCount = dataSize / (channelCount * bytesPerSample)` divides by zero if a format chunk has `numberOfChannels == 0` or `bitsPerSample == 0` (the `% 8` and `> 0` checks are `fplAssert`, i.e. gone in release).
- `fplMemoryAllocate` return value is not null-checked before `fplMemoryCopy`.

Fix:
- Before reading any chunk header, require `bufferPosition + sizeof(WaveChunk) <= bufferSize`; before reading a chunk body, require `bufferPosition + chunk->size <= bufferSize`. Bail out (return false) otherwise.
- Clamp the copied size to what actually remains in the buffer instead of trusting `chunk->size`; drop the `fplAssert(sampleMemorySize == dataSize)` in favor of a real runtime check.
- Reject `channelCount == 0` and `bitsPerSample == 0` / non-multiple-of-8 with a runtime `return false`, not an assert.
- Null-check the allocation.

## 2. final_audiosystem.h — `AudioSystemStopOne` unlocked traversal (HIGH)

`AudioSystemStopOne` (lines ~827–844) walks the linked list to find the play item **before** taking `playItems.lock`:
```c
AudioPlayItem *playItem = audioSys->playItems.first;   // no lock held
while (playItem != fpl_null) { ... playItem = playItem->next; }
if (foundPlayItem != fpl_null) {
    fplMutexLock(&audioSys->playItems.lock);
    RemovePlayItem(...);                                // frees it
    fplMutexUnlock(&audioSys->playItems.lock);
}
```
The audio device callback runs `AudioSystemWriteFrames → WritePlayItemsToMixer2`, which holds `playItems.lock` and can `RemovePlayItem` (free nodes) concurrently. The unlocked traversal can therefore follow a `next` pointer into a freed node → use-after-free / crash. It can also return a pointer that another thread frees before `RemovePlayItem` re-finds it.

Fix: take `playItems.lock` around the whole find-and-remove, exactly like `AudioSystemStopAll` does. (Related: `AudioSystemGetSources`/`GetPlayItems` read `count` outside the lock, then copy under it — a benign but real TOCTOU; snapshot `count` under the lock too.)

Shutdown ordering note: `AudioSystemShutdown` destroys the mutexes and frees the conversion buffer with no guarantee the device callback has stopped. `GameMain` happens to call `fplStopAudio()` first (good), but the library itself gives no protection — document that the audio device must be stopped before `AudioSystemShutdown`.

## 3. final_gameplatform.h — frees uninitialized memory blocks on early failure (HIGH)

`gameMemoryBlock` / `renderMemoryBlock` are declared uninitialized (lines ~684–685). Several early-exit paths call `GameMainShutdown(config, …, &gameMemoryBlock, &renderMemoryBlock)` **before** `fmemInit` has run:
- `fplPlatformInit` failure (line ~691–696)
- `fglLoadOpenGL` failure (line ~713–717)

`GameMainShutdown` unconditionally calls `fmemFree(gameMemoryBlock)` / `fmemFree(renderMemoryBlock)` on those garbage structs → frees a wild pointer (UB / crash). It also calls `fplStopAudio()` / `fglUnloadOpenGL()` / `fplPlatformRelease()` when those subsystems may not be initialized.

Fix: zero-init both blocks at declaration (`fmemMemoryBlock gameMemoryBlock = fplZeroInit;`) and make `fmemFree` a no-op on a zeroed block, or track how far initialization got and only tear down what was set up (staged cleanup / goto-ladder).

## 4. final_xml.h — no size bounds, asserts-as-validation (HIGH)

- The parser stores `context->size` in `fxmlInitFromMemory` but **never uses it**. All scanning is driven by `*context->ptr` (NUL checks). If the input buffer is not NUL-terminated (a memory-mapped file, an exact-size byte buffer), every scan loop (`fxml__ParseIdent`, `fxml__ParseInnerText`, attribute value scan, …) reads past the end → OOB read / crash.
- Structural validation is done with `assert()` (`fxml__ParseAttribute`, `fxml__ParseTag`, `fxml__ParseDeclaration`): `assert(*context->ptr == '=')`, `assert(*ptr == '\"')`, `assert(ptr[0] == '>')`, etc. In release builds asserts are compiled out, so malformed XML walks the pointer to wherever and reads/allocates garbage — again an OOB read on hostile input.
- `fxml__ParseTag` sets `outResult->tag = fgl_null;` (line ~345) — **`fgl_null` is the OpenGL loader's macro**, not `fxml_null`. This only compiles if `final_dynamic_opengl.h` happens to be included first; the header is not self-contained.
- `fxml__AllocMemory` doesn't null-check `FXML_MALLOC` before `FXML_MEMSET(blockBase, 0, totalSize)` (line ~153–154).

Fix:
- Add an end pointer (`data + dataSize`) and bound every scan loop against it; never rely on NUL-termination.
- Replace the structural `assert`s with real error returns (make `fxmlParse` return `false` on malformed input).
- `fgl_null` → `fxml_null`.
- Null-check the malloc.

## 5. final_assets.h — texture allocation overflow + wrong component count (MED)

`TextureDataAllocate` (line ~117):
```c
size_t size = w * h * sizeof(uint8_t) * components;  // w, h, components are uint32_t
```
`w * h` is evaluated in 32-bit `unsigned` and can overflow (e.g. 100000×100000) **before** the promotion to `size_t`, producing a small `size`, an under-allocation, and a heap overflow when the caller fills the texture.
Fix: `size_t size = (size_t)w * h * components;`.

`TextureDataLoadFromFile` (line ~233–245): `stbi_load_from_memory(..., 4)` forces a 4-channel decode, but then sets `target->components = imageComponents` (the file's *original* channel count, e.g. 3). Any consumer that computes a stride/row size from `components` will be wrong (the data is always 4-channel here).
Fix: set `target->components = 4;` (the requested component count), or thread the real decoded layout through consistently.

## 6. final_buffer.h — forced implementation + wrong cacheline padding (MED)

- Line 104: `#define FINAL_BUFFER_IMPLEMENTATION` is emitted **unconditionally**, right after the include guard closes. Every translation unit that includes `final_buffer.h` therefore compiles the implementation (the `extern` functions `InitMemoryMirror`, `LockFreeRingBuffer*`, …). Include it in two `.c`/`.cpp` files and the link fails with duplicate symbols. The single-header convention is that the *user* defines the `*_IMPLEMENTATION` macro in exactly one TU.
  Fix: delete line 104; require callers to `#define FINAL_BUFFER_IMPLEMENTATION` in one TU like the other headers.
- Lines 62, 63, 69: `uint8_t filePadding[64 - UINTPTR_MAX];` / `bufferPadding[64 - UINTPTR_MAX]`. `UINTPTR_MAX` is `2^64-1`, so `64 - UINTPTR_MAX` wraps to `65` (not the intended `64 - sizeof(void*) = 56`). The false-sharing padding is the wrong size, so the "one field per cache line" layout is broken (defeats the purpose of the lock-free ring buffer's padding). The Linux branch uses `sizeof(int)` correctly — the Windows branch should use `sizeof(HANDLE*)` / `sizeof(void*)`.
  Fix: `64 - sizeof(void *)` (and fix the `HANDLE *fileHandle` field, which is declared as a pointer-to-HANDLE but used as a HANDLE).
- `f_InitMemoryMirrorWin32`: after `virtualAlloc2` fails, `blockAddress == NULL` closes the handle but does **not** `break`/`continue`; execution falls into the mapping loop with `blockAddress == NULL`. Add the missing early-out.

## 7. final_render.h + final_opengl_render.h — command-buffer desync on OOM (MED)

`_RenderPushHeader` commits a `CommandHeader` into the arena, then the pusher calls `_RenderPushTypeAs` for the body. If the arena (`fmemPush`) returns null mid-command (growable block exhausted), the pusher `return`s, leaving a **header with the type set but `dataSize == 0` and no body** in the stream (e.g. `RenderPushMatrix`, `RenderAllocateVertices`).

`RenderWithOpenGL` walks the stream purely by `header->dataSize`:
```c
mem += dataSize;
size_t consumed = (size_t)(mem - startMem);
remaining -= consumed;      // size_t
```
A truncated/mis-sized command makes the executor read the following bytes as the wrong struct, and if `consumed > remaining`, `remaining -= consumed` underflows to a huge value → the `while (remaining > 0)` loop runs off the end of the buffer (OOB reads, likely crash). The `default: fplAssert(!"Invalid default case!")` is compiled out in release.

Fix:
- In the pushers, reserve header **and** body as a single unit and roll back the header (or don't emit it) if the body allocation fails, so a partial command is never left in the stream.
- Defensively, have `RenderWithOpenGL` stop when `sizeof(CommandHeader) + dataSize > remaining` instead of trusting `dataSize`.

## 8. final_audioconversion.h — SinC producer-side overflow (MED)

`AudioResampleInterleaved` computes `outFrameCount = fplGetTargetAudioFrameCount(inFrameCount, inRate, outRate)`, which can round to `minOutputFrameCount + 1`. `Audio__ResamplingInterleaved` then clears and writes `outFrameCount * channels` floats into `outSamples`. The consumer (`ProcessSinglePlayItem`) clamps the *returned* count (line ~1342), but the resampler has **already written** the extra frame into `dspOut`. When `minOutputFrameCount == dspOutBuffer.maxFrameCount` and `channels == FPL_MAX_AUDIO_CHANNEL_COUNT`, that extra frame overruns the DSP-out static buffer.

This is the producer side of the known "SinC +1 frame" issue (the consumer side was already patched). Fix: clamp `outFrameCount` to the caller-provided `minOutputFrameCount` inside `AudioResampleInterleaved`/`AudioResampleDeinterleaved` before calling the core, so the write can never exceed the output buffer.

Also note: the SinC filter has no history across chunk boundaries — `srcIndex < 0` samples are skipped at every chunk start. In the streaming pipeline (`ResampleChunk` per chunk) this produces small discontinuities at chunk edges; single-call tests won't catch it.

## 9. final_audioconversion.h — SinC recomputes `sinf` per tap (MED, performance)

`Audio__ResamplingInterleaved` / `Audio__ResamplingDeinterleaved` call `AudioSinC(f)` (→ `sinf`) for every filter tap: `2*filterRadius+1 = 17` `sinf` calls per output sample per channel. At 48 kHz stereo that is ~1.6M `sinf`/s just for resampling. The file already defines a precomputed `AudioSinCTable` + `GetSinCTableValue` and an `AudioResamplingContext` (a ~128 KB struct of `inBuffer`/`outBuffer`) — **all unused** by the actual resamplers.

Fix: build the SinC table once and sample it (`GetSinCTableValue`) in the inner loop instead of calling `sinf`, or drop the dead table/context if the design changed. This is the single biggest CPU win in the audio path (matches the "Performance is really bad" TODO at the top of `final_audiosystem.h`).

Related perf/stability: the mixing path takes `fplMutexLock` inside the realtime audio callback (`WritePlayItemsToMixer2`, `StopOne`, etc.). Locking a mutex from the device callback risks priority inversion / dropouts; a lock-free or try-lock handoff would be safer (also called out in the TODO).

## 10. final_audio.h — `DecibelToAmplitude` wrong base (MED)

```c
double DecibelToAmplitude(const double dB) { return pow(20.0, dB / 20.0); }
```
Amplitude from dB is `10^(dB/20)`, not `20^(dB/20)`. `AmplitudeToDecibel` correctly uses `20*log10(amp)`, so the round-trip is broken.
Fix: `return pow(10.0, dB / 20.0);`.
Minor, same file: `DecibelToPower` only clamps `dB < min`; for `dB > max` it returns `> 1.0` and trips its own `fplAssert(result <= 1.0)`. Clamp the upper end too.

## 11. final_geometry.h — `AABB3fContainsPoint` missing abs (MED)

```c
Vec3f d = V3fSub(point, center);
bool result = (d.x < radius.x) && (d.y < radius.y) && (d.z < radius.z);
```
For a point far on the negative side, `d.x` is very negative and `< radius.x` is trivially true → the function reports "inside" for points that are outside on the -X/-Y/-Z side. Compare `AABB2fContainsPoint`, which does proper min/max bounds.
Fix: `Vec3f a = V3fAbs(d); result = (a.x <= radius.x) && (a.y <= radius.y) && (a.z <= radius.z);`.

## 12. final_gameplatform.h — custom keyboard mappings don't work (MED)

When `config->keyboardMappings->isCustom` is set (line ~865):
```c
fplMemoryCopy(config->keyboardMappings, sizeof(KeyboardButtonMappings), keyboardMappings);
```
This copies the mapping list but never sets `keyboardButtonStates->mapped[...]`. The per-frame polled-key loop (line ~907) does `if (!keyboardButtonStates->mapped[buttonTypeIndex]) continue;`, so with custom mappings **every controller button is skipped** — custom keyboard bindings never drive the controller. Only the default path (`InternalGamePlatformAddDefaultKeyboardMappings`, which calls `...AddKeyboardControllerButtonMapping` and sets `mapped`) works.
Fix: after copying custom mappings, rebuild `keyboardButtonStates->mapped[]` by walking the copied entries (set `mapped[type - First] = true` for each valid `type`).

## 13. final_gameplatform.h — copy-paste null check (MED)

After allocating the platform state (line ~757):
```c
gamePlatformState = fmemPushStruct(&gameMemoryBlock, GamePlatformState, fmemPushFlags_Clear);
if (audioSys == fpl_null) {          // should be: if (gamePlatformState == fpl_null)
```
`audioSys` was already validated non-null just above, so this check never fires. If the `GamePlatformState` push fails (arena exhausted), `gamePlatformState` stays null and is dereferenced later (`&gamePlatformState->gameMemory`, `&gamePlatformState->inputs[0]`, …) → null deref.
Fix: check `gamePlatformState == fpl_null`.

## 14. final_audiosystem.h — missing allocation null checks (MED/LOW)

- `PropeAudioFileFormat` (line ~448): `probeBuffer = fplMemoryAllocate(initialBufferSize)` and the MP3 realloc (line ~487) are not null-checked before being passed to `stream->read`.
- `AudioSystemLoadFileSource` (line ~686): `buffer = fplMemoryAllocate(fileSize)` is not null-checked before `AudioSystemStreamRead(&stream, fileSize, buffer, fileSize)`.
Fix: null-check each allocation and bail cleanly (closing the file / freeing the probe buffer).

Also: `InternalGamePlatformProcessEvents` gamepad handling (`final_gameplatform.h`, line ~268) computes `controllerIndex = 1 + event.gamepad.deviceIndex` and only `fplAssert`s it is in range; a 5th gamepad (`deviceIndex >= 4`) writes past `controllers[5]` in release. Clamp/ignore out-of-range device indices.

## 15. final_random.h — RandomU8 range + xorshift seed 0 (LOW)

- `RandomU8`: `RandomU32(series) % UINT8_MAX` (`% 255`) never returns 255 and biases the distribution. Use `& 0xFF` (or `% 256`).
- Xorshift `RandomU32` derives the result from `seed` *before* advancing, and seed `0` is a fixed point of xorshift — `RandomSeed(0)` produces all zeros forever. Reject/replace a zero seed (e.g. seed with a non-zero constant).

## 16. final_debug.h — unbounded event recording, unchecked alloc (LOW)

- `RecordDebugEvent`: `eventIndex` is only range-checked with `fplAssert`; in release, if more than `MAX_DEBUG_EVENT_COUNT` events are recorded before a frame swap, it writes past `events[...][]`. Add a real bound (drop events when full).
- `InitDebug`: `fplMemoryAllocate(totalSize)` isn't null-checked before the struct pointers are written. Note the table is large (`DebugEvent events[2][16*65536]` ≈ tens of MB); worth documenting.

## 17. final_opengl_render.h — signed char in text path (LOW)

`CommandType_Text` (and `DrawTextFont`): `char at = text[textPos]; (uint32_t)at`. With signed `char`, bytes ≥ 128 sign-extend to huge values, fail the glyph range test, and render as spaces. `final_fontloader.h` correctly uses `(uint8_t)text[textPos]`. Cast through `uint8_t` here too.

## 18. final_graphics.h — unchecked allocs, int overflow (LOW)

- `TextureLoadFromFile` and `DepthBufferReset` don't null-check `fplMemoryAllocate`; a large image / buffer that fails to allocate leads to a null write.
- `DepthBufferClear`: `int count = depth->width * depth->height;` overflows `int` for very large buffers (fine for screen sizes, but the pattern is fragile — use `size_t`).
- `BackbufferDrawLine` uses a flat index with only a `0 <= index < size` guard; a line crossing the left/right edge wraps onto the adjacent row (visual artifact, not a crash).

## 19. final_math.h — M4fInverse singular matrix (LOW)

`M4fInverse` computes `inverseDet = 1.0f / Determinant` with no `Determinant == 0` guard (line ~2287). A singular input yields inf/NaN throughout the result. `V2fNormalize`/`V3fNormalize`/`QuatNormalize` already guard zero-length, so this is the odd one out. Consider returning identity (or a success flag) when `|Determinant| < epsilon`.

## 20. final_mp3loader.h / final_vorbisloader.h — decode edge cases (LOW)

- Both compute `frameCount = sampleCount / channels` — if a decoder ever reports `channels == 0` with `samples > 0`, that's a divide-by-zero. Guard `channels > 0`.
- `LoadMP3FromBuffer` / `LoadVorbisFromBuffer` allocate `isamples` and `fplMemoryCopy` into it with no null check on the allocation.

## 21. final_fontloader.h — atlas size overflow, dead local (LOW)

- `FontLoadFromMemoryEx`: `atlasAlphaBitmap = MemoryAllocatorAlloc(allocator, atlasWidth * atlasHeight)` — `uint32 * uint32` can overflow for a very large atlas. Use `(size_t)atlasWidth * atlasHeight`.
- The `stbtt_pack_range characterRange` local (lines ~493–497) is fully populated but never used — `stbtt_PackFontRange` is called with explicit arguments instead. Dead code (harmless, but confusing).

## 22. final_utils.h — dead sign check (LOW)

`FormatSize`: `if (value < 0) { p++; }` where `value` is `size_t` (unsigned) — always false. Either drop it or take a signed parameter if negative sizes are meant to be formatted.

## 23. final_log.h — verify the format-truncation contract (LOW / to verify)

`LogWriteArgs` / `LogWriteRaw` do `buffer[len] = '\n'; buffer[len+1] = '\0';` where `len = fplStringFormatArgs(buffer, capacity-1, …)`. This is safe **only if** `fplStringFormatArgs` returns the *truncated* number of bytes written (≤ `capacity-2`). If it instead returns the would-be length (like `snprintf`), those two stores are out of bounds on overflow. The code comment asserts the former ("empties the buffer on overflow"); confirm against the FPL implementation, since the 1 MB line buffer makes overflow unlikely but not impossible (the OpenGL extension dump is deliberately large).

---

## Notes / non-issues checked

- `final_core.h` allocator wrapper — clean.
- `final_game.h` — declarations/inlines only; `ButtonWasPressed` firing on the release edge is intentional (documented, `ButtonWentDown` added for the press edge).
- `final_gamecontroller.h` SDL-mapping parser — careful bounds handling throughout; no issues found.
- `final_audiosystem.h` allocated-source memory model (struct + samples in one block, `isAllocated == false`) is consistent with `AudioSystemClearSources` / `FreeAudioBuffer`, so there is no double-free there.
- `final_math.h` vector normalizers guard zero length (per the earlier hardening pass).
