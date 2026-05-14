# Plan: OSS Audio Backend for BSD

Status: Done — implemented and verified on FreeBSD across all FPL audio demos (2026-05-14).
Author: collaborative plan, 2026-05-14.
Reference backend: ALSA (`final_platform_layer.h` lines ~29309-30599).

## Goals

Add a native OSS (Open Sound System) audio backend so FPL audio works on BSD
platforms (FreeBSD, NetBSD, OpenBSD, DragonFly) without requiring PulseAudio
or PipeWire.

## Design Decisions (locked)

- **Target gate**: All BSDs via `FPL_SUBPLATFORM_BSD`. Not Linux (ALSA owns
  Linux). Linux legacy `/dev/dsp` is intentionally out of scope.
- **Linkage**: No runtime linking. OSS is direct `open/ioctl/write` against
  `/dev/dsp` via `<sys/soundcard.h>`. No `dlopen`, no API table.
- **Access mode**: Blocking `write()`. No mmap path (OSSv4 supports mmap but
  ALSA-style mmap on OSS is too platform-specific for the first pass).
- **Device enumeration**: Parse `/dev/sndstat` on FreeBSD for human-readable
  names; fall back to probing `/dev/dsp{0..7}` via `stat()` on Net/Open/Dragon.
- **OSSv4-only formats** (`AFMT_S32_NE`, `AFMT_FLOAT`, `AFMT_S24_NE`): guarded
  with `#ifdef AFMT_*` so the code still compiles on minimal NetBSD/OpenBSD
  OSS emulation headers.

## Phase Breakdown (one phase ≈ one commit)

### Phase 1 — Header surface and gating

File: `final_platform_layer.h`

1. Documentation block (line ~2806): add
   `// FPL_NO_AUDIO_OSS = Disable OSS audio backend`.
2. Support gate (after PipeWire block at ~line 2828):
   ```c
   #if !defined(FPL_NO_AUDIO_OSS) && defined(FPL_SUBPLATFORM_BSD)
   #   if fplHasInclude(<sys/soundcard.h>)
   #       define FPL__SUPPORT_AUDIO_OSS
   #   endif
   #endif
   ```
3. Enable gate (after PipeWire at ~line 2914):
   ```c
   #if defined(FPL__SUPPORT_AUDIO_OSS)
   #   define FPL__ENABLE_AUDIO_OSS
   #endif
   ```
4. Module name (line ~10054):
   `#define FPL__MODULE_AUDIO_OSS "OSS"`
5. Enum `fplAudioBackendType` (line ~5189): insert
   `fplAudioBackendType_OSS` **before** `_Custom`. Update doxygen, update
   `_Last`. The two `fplStaticAssert`s at ~34268 still hold since `_Last`
   continues to be `_Custom`.
6. `fplAudioDeviceID` union (line ~5471): add inside guard
   ```c
   #if defined(FPL__ENABLE_AUDIO_OSS)
       char oss[256];      // /dev/dspN path
   #endif
   ```
7. New struct (after `fplPipeWireAudioSettings`, ~line 5563):
   ```c
   #if defined(FPL__ENABLE_AUDIO_OSS)
   typedef struct fplOSSAudioSettings {
       // If non-zero, open the device blocking (default is non-blocking).
       fpl_b32 noNonBlocking;
       // Fragment exponent override (log2 of fragment bytes). 0 = let FPL pick.
       uint32_t fragmentExponent;
   } fplOSSAudioSettings;
   #endif
   ```
8. `fplSpecificAudioSettings` (line ~5569): add `fplOSSAudioSettings oss;`
   under guard.
9. Name table (line ~34270): insert `FPL__ENUM_NAME("OSS", fplAudioBackendType_OSS)`
   in matching order. Keep `fplStaticAssert(...COUNT...)` valid.
10. Default backend list (line ~33313): append `fplAudioBackendType_OSS` so
    auto-probe falls through to it on BSD when PulseAudio/PipeWire absent.
11. Descriptor dispatch (`fpl__GetAudioBackendDescriptors`, line ~33347):
    ```c
    case fplAudioBackendType_OSS:
    #if defined(FPL__ENABLE_AUDIO_OSS)
        desc = &fpl__global_audioBackendOSSDescriptor;
    #endif
        break;
    ```

Acceptance: builds on Linux (OSS path inactive) and on FreeBSD with
`<sys/soundcard.h>` present (descriptor symbol unresolved until Phase 2).

### Phase 2 — OSS backend skeleton

Insert a new section `> AUDIO_BACKEND_OSS` after the ALSA section closes
(`#endif // FPL__ENABLE_AUDIO_ALSA` at line ~30599) and before PulseAudio.

```c
#if defined(FPL__ENABLE_AUDIO_OSS)
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    int fd;
    void *intermediaryBuffer;
    uint32_t intermediaryBufferSize;
    volatile bool breakMainLoop;
} fpl__OssAudioBackend;
```

Implement as stubs that return success but do nothing useful:
- `fpl__AudioBackendOssInitialize` — no API loading required, return success.
- `fpl__AudioBackendOssRelease` — close fd if open, free intermediary buffer.
- `fpl__AudioBackendOssReleaseDevice` — close fd, free buffer, null pointers.
- `fpl__AudioBackendOssInitializeDevice` — placeholder returning
  `fplAudioResultType_NotImplemented`.
- `fpl__AudioBackendOssStartDevice` / `StopDevice` — noop returning success.
- `fpl__AudioBackendOssMainLoop` / `StopMainLoop` — set flag, no I/O.
- `fpl__AudioBackendOssGetAudioDevices` / `GetAudioDeviceInfo` — return 0 /
  `NotImplemented`.

Add the descriptor at end of section:
```c
fpl_globalvar fplAudioBackendDescriptor fpl__global_audioBackendOSSDescriptor = {
    { { /* idName guid+"OSS" */ }, fplAudioBackendType_OSS,
      sizeof(fpl__OssAudioBackend), false, true },
    { Initialize, Release, GetAudioDevices, GetAudioDeviceInfo,
      InitializeDevice, ReleaseDevice, StartDevice, StopDevice,
      MainLoop, StopMainLoop }
};
#endif // FPL__ENABLE_AUDIO_OSS
```

Pick a fresh GUID (any random 128-bit) for the idName.

Acceptance: FPL builds on FreeBSD, `fplAudioBackendType_OSS` shows up in
backend probe but device init fails with `NotImplemented` until Phase 3.

### Phase 3 — Device init and format mapping

Implement `fpl__AudioBackendOssInitializeDevice`:

1. Resolve device path:
   - If `targetDevice->oss` set, use it.
   - Else default to `/dev/dsp`.
2. `open(path, O_WRONLY | (noNonBlocking ? 0 : O_NONBLOCK))`.
   On failure return `fplAudioResultType_NoDeviceFound`.
3. Format mapping table (`fpl__MapAudioFormatToOSSFormat`):
   ```c
   static const int fmtMap[] = {
     /* None */ 0,
     /* U8   */ AFMT_U8,
     /* S16  */ AFMT_S16_NE,
     /* S24  */
   #ifdef AFMT_S24_NE
       AFMT_S24_NE,
   #else
       0,
   #endif
     /* S32  */
   #ifdef AFMT_S32_NE
       AFMT_S32_NE,
   #else
       0,
   #endif
     /* S64  */ 0,
     /* F32  */
   #ifdef AFMT_FLOAT
       AFMT_FLOAT,
   #else
       0,
   #endif
     /* F64  */ 0,
   };
   ```
4. `ioctl(fd, SNDCTL_DSP_GETFMTS, &mask)` → format mask. Walk fallback
   chain (requested → S16_NE → FLOAT → S32_NE → S24_NE → U8). Use first
   that is present in mask **and** has non-zero mapping.
5. `ioctl(fd, SNDCTL_DSP_SETFMT, &fmt)` — must echo back chosen fmt.
6. Reverse map `fpl__MapOSSFormatToAudioFormat(fmt)` for internalFormat.type.
7. Channels: `ioctl(fd, SNDCTL_DSP_CHANNELS, &channels)`.
8. Rate: `ioctl(fd, SNDCTL_DSP_SPEED, &rate)`.
9. Fragment size:
   - Pick `fragsize` = nearest power-of-two of period bytes
     (`bufferSizeInFrames / periods * channels * sampleSize`).
   - `frag = (periods << 16) | log2(fragsize)`.
   - `ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &frag)`.
10. Query actual buffer via `ioctl(fd, SNDCTL_DSP_GETOSPACE, &bi)`. Use
    `bi.fragments * bi.fragsize / frameSize` for `bufferSizeInFrames` and
    `bi.fragments` for `periods`.
11. Allocate `intermediaryBuffer` = `bi.fragsize` bytes.
12. Set channel map: factor `fpl__SetAudioDefaultChannelMapALSA` into a
    shared `fpl__SetAudioDefaultChannelMapPosix` helper or duplicate the
    body inline. (Recommend duplicate first time to keep ALSA section
    untouched; refactor later if a third user shows up.)

Acceptance: Demo `FPL_Audio.c` runs on FreeBSD and opens `/dev/dsp` with
the negotiated format reported via `fplGetAudioHardwareFormat`.

### Phase 4 — Main loop / write path

```c
fpl_internal bool fpl__GetAudioFramesFromClientOss(...) {
    while (!impl->breakMainLoop) {
        uint32_t frameCount = impl->intermediaryBufferSize /
                              fplGetAudioFrameSizeInBytes(...);
        uint32_t framesRead = fpl__ReadAudioFramesFromClient(
            backend, frameCount, impl->intermediaryBuffer);
        fplAssert(framesRead == frameCount);

        uint8_t *p = impl->intermediaryBuffer;
        size_t remaining = framesRead * frameSize;
        while (remaining > 0 && !impl->breakMainLoop) {
            ssize_t n = write(impl->fd, p, remaining);
            if (n < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN) {
                    // wait writable
                    struct pollfd pfd = { impl->fd, POLLOUT, 0 };
                    poll(&pfd, 1, 100);
                    continue;
                }
                FPL__ERROR(FPL__MODULE_AUDIO_OSS,
                           "OSS write failed: %d", errno);
                return false;
            }
            p += n;
            remaining -= n;
        }
    }
    return true;
}
```

- `StartDevice` does a single priming call to `GetFromClient` then returns.
- `StopDevice`: try `ioctl(fd, SNDCTL_DSP_HALT, NULL)`; on `ENOTTY`/`EINVAL`
  fall back to `ioctl(fd, SNDCTL_DSP_RESET, NULL)`.
- `MainLoop` runs `while (!breakMainLoop && GetFromClient(...)) {}`.
- `StopMainLoop` just flips the flag.

Acceptance: Continuous sine demo plays cleanly without underruns/overruns
on FreeBSD `/dev/dsp`. No memory leak across init/release cycles.

### Phase 5 — Device enumeration

`fpl__AudioBackendOssGetAudioDevices`:

1. Try `open("/dev/sndstat", O_RDONLY)`:
   - If success, read into a stack buffer. Lines look like:
     ```
     pcm0: <Realtek ALC1220> (play/rec) default
     pcm1: <HDMI> (play)
     ```
   - Parse `pcmN:` prefix → device path `/dev/dspN`. Description = text
     between `<` and `>`. `default` keyword → `isDefault = true`.
2. Else fall back to probing `/dev/dsp` and `/dev/dsp{0..7}` via `stat()`.
   First found becomes default. Name = `"OSS Device N"`.

`fpl__AudioBackendOssGetAudioDeviceInfo`:

1. Validate `targetDevice->oss` non-empty.
2. `stat()` the path. If missing, return `DeviceByIdNotFound`.
3. Copy id/name back. Optionally open + `SNDCTL_DSP_GETFMTS` to populate
   `supportedFormats[]` (encode via `fplAudioFormatU64`).

Acceptance: `fplGetAudioDevices()` returns at least one device on FreeBSD
and reasonable names from `/dev/sndstat` when present.

### Phase 6 — Changelog and progress tracking

1. `final_platform_layer.h` v1.0.0 block:
   - `- New: [OSS] Added native OSS audio backend for BSD platforms (FreeBSD/NetBSD/OpenBSD/DragonFly)`
   - `- New: [OSS] Implemented device enumeration via /dev/sndstat with stat() fallback`
   - `- New: [OSS] Added fplOSSAudioSettings (noNonBlocking, fragmentExponent)`
2. Update `progress/todo_unimplemented.md` — remove BSD from "partial" if
   audio was the gating concern.
3. Delete this plan file or mark `Status: Done`.

## Commit Message Style

Per memory `[[feedback_commit_style]]`: short, no `Co-Authored-By` trailer.

Suggested subjects:
1. `OSS: Add backend gating, enum, and settings surface`
2. `OSS: Add backend skeleton (no audio yet)`
3. `OSS: Implement device init and format mapping`
4. `OSS: Implement blocking write main loop`
5. `OSS: Implement device enumeration via /dev/sndstat`
6. `OSS: Update changelog for new backend`

## Risks / Open Items

- **Channel map duplication** — Phase 3 duplicates `fpl__SetAudioDefaultChannelMapALSA`. Decide on rename to `fpl__SetAudioDefaultChannelMapPosix` if a future backend also wants it.
- **NetBSD/OpenBSD OSS emulation** — Limited format support. Test with real hardware before claiming Net/Open support in changelog. If unverified, mark as "FreeBSD/DragonFly tested, Net/Open expected to work".
- **mmap path** — Deliberately skipped. Could be added in a v2 pass mirroring ALSA's `isUsingMMap` branch using `SNDCTL_DSP_MAPINBUF`/`MAPOUTBUF`. Not blocking initial release.
- **`fplStaticAssert` on enum** — Inserting `_OSS` before `_Custom` keeps `_Last == _Custom` so existing asserts hold. Verify after edit.
- **Demo coverage** — No demo edits required; existing `FPL_Audio.c` should pick up the new backend automatically via auto-probe. Verify manually.

## Out of Scope (explicit)

- macOS CoreAudio backend (separate effort).
- Sndio backend on OpenBSD (would be parallel third BSD option, not OSS).
- OSS capture (record) support — playback only first pass.
