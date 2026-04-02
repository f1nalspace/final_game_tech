# Final Audio Libraries — Architecture Overview

> **Scope:** `demos/additions/final_audio.h`, `final_audiosystem.h`, `final_audioconversion.h`,
> `final_audiodemo.h`, `final_buffer.h`, `final_waveloader.h`, `final_vorbisloader.h`, `final_mp3loader.h`
>
> Third-party libraries (`minimp3`, `stb_vorbis`) are treated as **black-box dependencies** — their internals are excluded.

---

## Table of Contents

1. [System Layers & Responsibilities](#1-system-layers--responsibilities)
2. [Dependency Graph](#2-dependency-graph)
3. [Core Types & Primitives — `final_audio.h`](#3-core-types--primitives--final_audioh)
4. [Format Conversion & Resampling — `final_audioconversion.h`](#4-format-conversion--resampling--final_audioconversionh)
5. [Format Loaders](#5-format-loaders)
6. [Audio System — `final_audiosystem.h`](#6-audio-system--final_audiosystemh)
7. [Lock-Free Ring Buffer — `final_buffer.h`](#7-lock-free-ring-buffer--final_bufferh)
8. [Track Management / Demo Layer — `final_audiodemo.h`](#8-track-management--demo-layer--final_audiodemoh)
9. [Data Flow Diagrams](#9-data-flow-diagrams)
10. [Weak Points, Issues & Critical Bugs](#10-weak-points-issues--critical-bugs)
11. [Stable & Working Areas](#11-stable--working-areas)

---

## 1. System Layers & Responsibilities

The audio stack is organized in four horizontal layers:

| Layer | File(s) | Role |
|-------|---------|------|
| **Demo / Application** | `final_audiodemo.h` | Track list management, UI state, loading orchestration |
| **Audio System** | `final_audiosystem.h` | Source registry, playback scheduling, mixing, output |
| **Processing** | `final_audioconversion.h`, `final_audio.h` | Format conversion, resampling, DSP, buffer utilities |
| **I/O / Loaders** | `final_waveloader.h`, `final_vorbisloader.h`, `final_mp3loader.h`, `final_buffer.h` | Decode audio files into PCM; lock-free ring buffer |

Each layer only depends on layers below it. The demo layer is optional — the audio system can be used directly.

---

## 2. Dependency Graph

```
final_audiodemo.h
  ├── final_audio.h            (core types)
  ├── final_audiosystem.h      (playback system)
  │     ├── final_audio.h
  │     ├── final_audioconversion.h
  │     ├── final_waveloader.h
  │     ├── final_vorbisloader.h  ──> [stb_vorbis — black box]
  │     └── final_mp3loader.h     ──> [minimp3    — black box]
  └── final_buffer.h           (lock-free ring buffer, currently unused by audiodemo)

final_platform_layer.h         (FPL — all files depend on this)
```

> `final_buffer.h` is included by `final_audiodemo.h` but not actively used by it or by `final_audiosystem.h`. It exists as a standalone utility.

---

## 3. Core Types & Primitives — `final_audio.h`

This is the **shared foundation** header. All other audio files include it.

### 3.1 Primitive Type Aliases

```c
AudioFrameIndex   = uint32_t   // Number of audio frames (one sample per channel)
AudioSampleIndex  = uint32_t   // Number of individual samples (frames × channels)
AudioChannelIndex = uint16_t   // Channel count
AudioHertz        = uint32_t   // Sample rate in Hz (e.g. 44100, 48000)
AudioMilliseconds = uint32_t   // Duration in milliseconds
AudioDuration     = double     // Duration in seconds (high precision)
AudioBufferSize   = size_t     // Buffer size in bytes
```

### 3.2 AudioFormat

```c
typedef struct AudioFormat {
    AudioHertz        sampleRate;   // e.g. 44100
    AudioChannelIndex channels;     // e.g. 2 (stereo)
    fplAudioFormatType format;      // U8 | S16 | S24 | S32 | F32
    uint8_t           padding;      // alignment to 16 bytes (static assertion enforced)
} AudioFormat;
```

### 3.3 AudioBuffer

```c
typedef struct AudioBuffer {
    uint8_t       *samples;     // Pointer to interleaved PCM data
    AudioBufferSize bufferSize; // Total bytes allocated
    AudioFrameIndex frameCount; // Number of frames
    fpl_b32         isAllocated; // True = buffer owns memory; False = borrowed pointer
} AudioBuffer;
```

- `isAllocated = true` → caller must free via `FreeAudioBuffer` / `FreeWaveData`
- `isAllocated = false` → shallow reference; do NOT free independently

### 3.4 AudioStream

```c
typedef struct AudioStream {
    AudioBuffer     buffer;           // Underlying PCM buffer
    AudioFrameIndex readFrameIndex;   // Current read cursor
    AudioFrameIndex framesRemaining;  // Frames left to read
} AudioStream;
```

Used internally by the audio system to stage output before final format conversion.

### 3.5 AudioStaticBuffer

```c
#define MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT  FPL_MAX_AUDIO_CHANNEL_COUNT
#define MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT    4096
#define MAX_AUDIO_STATIC_BUFFER_MAX_TYPE_SIZE  4   // sizeof(float) or sizeof(int32_t)

typedef struct AudioStaticBuffer {
    uint8_t         samples[MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT *
                            MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT *
                            MAX_AUDIO_STATIC_BUFFER_MAX_TYPE_SIZE];
    AudioFrameIndex maxFrameCount;
} AudioStaticBuffer;
```

Stack-allocated DSP scratch buffer. Three instances live inside `AudioSystem` (dspIn, dspOut, mixing). Total footprint per system ≈ 192 KB (assuming 16 channels, 4096 frames, 4 bytes/sample).

### 3.6 AudioBufferPointer (Iterator/View)

```c
typedef struct AudioBufferPointer {
    uint8_t        *samples;         // Pointer to current frame
    AudioFrameIndex totalFrameCount; // Total frames in buffer
    AudioFrameIndex usedFrameCount;  // Frames consumed so far
    AudioChannelIndex channels;
    uint16_t        frameStride;     // Bytes per frame = channels × sample size
} AudioBufferPointer;
```

Inline helper functions:

| Function | Purpose |
|----------|---------|
| `AudioBufferPointerCreateFromStaticBuffer()` | Wrap a static buffer |
| `AudioBufferPointerCreateFromAudioBuffer()` | Wrap a heap buffer |
| `AudioBufferPointerIsValid()` | Non-null and non-zero frame count |
| `AudioBufferPointerIsEOF()` | `usedFrameCount >= totalFrameCount` |
| `AudioBufferPointerGetRemainingFrameCount()` | Remaining frames |
| `AudioBufferPointerGetDataF32()` | Typed pointer to current F32 sample |
| `AudioBufferPointerGetDataU8()` | Typed pointer to current U8 sample |
| `AudioBufferPointerAdvance(n)` | Move cursor forward by `n` frames |
| `AudioBufferPointerSeek(i)` | Seek to absolute frame index |
| `AudioBufferPointerClearSamples()` | Zero the entire buffer |

### 3.7 PCMWaveFormat / PCMWaveData

Shared output structure used by all three loaders:

```c
typedef struct PCMWaveFormat {
    uint32_t           frameCount;
    uint32_t           samplesPerSecond;
    uint32_t           bytesPerSample;
    fplAudioFormatType formatType;      // U8 | S16 | S24 | S32 | F32
    uint16_t           channelCount;
} PCMWaveFormat;

typedef struct PCMWaveData {
    PCMWaveFormat  format;
    AudioBufferSize samplesSize;       // Bytes allocated for isamples
    void          *isamples;           // Interleaved PCM samples (heap-allocated)
    char           lastError[976];     // Human-readable error string
    bool           isValid;
} PCMWaveData;
```

All loaders write into `PCMWaveData`. The caller is responsible for calling `FreeWave()` / `FreeWaveData()` when done.

### 3.8 Abstract Stream Interface

```c
typedef struct AudioSystemStream {
    AudioStreamSeekAbsoluteFunc *seek;
    AudioStreamReadFunc         *read;
    AudioStreamGetDataFunc      *getData;
    size_t  size;  // Total stream size in bytes
    size_t  pos;   // Current byte position
    void   *opaque; // Backend-specific state
} AudioSystemStream;
```

Two concrete implementations (both `static`):

| Factory | Backend |
|---------|---------|
| `AudioStreamCreateFromFileHandle()` | FPL file handle |
| `AudioStreamCreateFromData()` | In-memory byte buffer |

### 3.9 DSP: FFT & Window Functions

FFT implementation uses the recursive Cooley-Tukey algorithm.

```c
typedef struct FFT {
    FFTDouble      *in;       // Complex input array
    FFTDouble      *out;      // Complex output array
    AudioSampleIndex capacity;
    AudioSampleIndex size;
} FFT;
```

**Constraint:** `size` must be a power of two. Non-power-of-two inputs are unsupported.

| Function | Description |
|----------|-------------|
| `ForwardFFT(fft)` | Time domain → frequency domain |
| `BackwardFFT(fft)` | Frequency domain → time domain |
| `NormalizeFFT(fft)` | Divide by N |
| `HalfNormalizeFFT(fft)` | Divide by sqrt(N) |
| `FFTTest()` | Self-test (assertions disabled in current code) |

Window functions (for pre-processing before FFT):

| Function | Parameters |
|----------|-----------|
| `UniformWindowFunction(n, N)` | a0=1.0 |
| `HannWindowFunction(n, N)` | a0=0.5, a1=0.5 |
| `HammingWindowFunction(n, N)` | a0=0.53836, a1=0.46164 |
| `BlackmanWindowFunction(n, N)` | a0=0.42, a1=0.50, a2=0.08 |

Amplitude / decibel utilities:

```c
AmplitudeToDecibel(a)  = 20 * log10(a)
DecibelToAmplitude(dB) = pow(10, dB/20)
DecibelToPower(dB)     = maps dB range to [0.0, 1.0]
```

---

## 4. Format Conversion & Resampling — `final_audioconversion.h`

### 4.1 Dispatch Table

```c
typedef struct AudioSampleConversionFunctions {
    // Direct named pointers (convenience)
    AudioSampleFormatConversionFunc *convU8ToF32, *convF32ToU8;
    AudioSampleFormatConversionFunc *convS16ToF32, *convF32ToS16;
    AudioSampleFormatConversionFunc *convS24ToF32, *convF32ToS24;
    AudioSampleFormatConversionFunc *convS32ToF32, *convF32ToS32;

    // Interleave/deinterleave by format
    AudioSampleInterleaveFunc   *interleaveU8, *interleaveS16, *interleaveS32, *interleaveF32;
    AudioSampleDeinterleaveFunc *deinterleaveU8, *deinterleaveS16, *deinterleaveS32, *deinterleaveF32;

    // 2D lookup tables
    AudioSampleFormatConversionFunc *conversionTable[fplAudioFormatType_Last][fplAudioFormatType_Last];
    AudioSampleInterleaveFunc       *interleaveTable[fplAudioFormatType_Last];
    AudioSampleDeinterleaveFunc     *deinterleaveTable[fplAudioFormatType_Last];
} AudioSampleConversionFunctions;
```

Initialized by:
```c
AudioSampleConversionFunctions CreateAudioSamplesConversionFunctions();
```

All function pointers are populated with scalar (non-SIMD) implementations. SIMD dispatch is a TODO (see §10).

### 4.2 Supported Format Conversions

Only F32 is used as the pivot/intermediate format. Conversions between two non-F32 formats require chaining (not built in).

```
Implemented:
  U8  <-> F32
  S16 <-> F32
  S24 <-> F32
  S32 <-> F32

Missing (TODO lines 92-94):
  U8  <-> S16
  S8  <-> S24, S8  <-> S32
  S16 <-> S24, S16 <-> S32
  S24 <-> S32
```

Format conversion math summary:

| Conversion | Method |
|------------|--------|
| U8 → F32 | `(x / 127.5) - 1.0` → range `[-1.0, 1.0]` |
| F32 → U8 | clip + `(x + 1.0) * 127.5`, round |
| S16 → F32 | `x / 32767.0` |
| F32 → S16 | clip + `x * 32767`, round |
| S24 → F32 | 3-byte LE to int32 sign-extend, `/ 8388607.0` |
| F32 → S24 | clip + `x * 8388607`, pack 3 bytes LE |
| S32 → F32 | `x / INT32_MAX` |
| F32 → S32 | clip + `x * INT32_MAX`, round |

### 4.3 Public Conversion API

```c
// Convert a flat sample array from inFormat to outFormat
bool AudioSamplesConvert(AudioSampleConversionFunctions *funcTable,
    const AudioSampleIndex numSamples,
    const fplAudioFormatType inFormat, const fplAudioFormatType outFormat,
    const void *inSamples, void *outSamples);

// Interleaved LRLRLR -> deinterleaved LLLL|RRRR
bool AudioSamplesDeinterleave(AudioSampleConversionFunctions *funcTable,
    const AudioFrameIndex numFrames, const AudioChannelIndex numChannels,
    const fplAudioFormatType format,
    const void *inSamples, void **outSamples);

// Deinterleaved -> interleaved
bool AudioSamplesInterleave(AudioSampleConversionFunctions *funcTable,
    const AudioFrameIndex numFrames, const AudioChannelIndex numChannels,
    const fplAudioFormatType format,
    const void **inSamples, void *outSamples);

// Mix all channels into mono (F32 only)
bool AudioSamplesMonolize(const AudioChannelIndex inChannels,
    const AudioFrameIndex frameCount,
    const float *inSamples, float *outSamples);
```

Behavior notes:
- `AudioSamplesConvert` early-exits to `memcpy` when `inFormat == outFormat`
- Interleave/deinterleave fall back to a generic byte-copy loop for unknown formats
- None of these functions allocate memory — callers provide buffers

### 4.4 SinC Resampling

```c
#define AUDIO_SINC_TABLE_SIZE              1024
#define AUDIO_RESAMPLE_BUFFER_FRAME_COUNT  512
#define AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT FPL_MAX_AUDIO_CHANNEL_COUNT

typedef struct AudioSinCTable {
    float    x[AUDIO_SINC_TABLE_SIZE]; // Precomputed SinC values
    uint32_t lastIndex;                // AUDIO_SINC_TABLE_SIZE - 1
    uint32_t filterRadius;             // Recommended: 8
} AudioSinCTable;

typedef struct AudioResamplingContext {
    float inBuffer [AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT][AUDIO_RESAMPLE_BUFFER_FRAME_COUNT];
    float outBuffer[AUDIO_RESAMPLE_BUFFER_CHANNEL_COUNT][AUDIO_RESAMPLE_BUFFER_FRAME_COUNT];
    AudioSinCTable  sincTable;
    AudioFrameIndex inFrameCount;
    AudioFrameIndex outFrameCount;
    AudioChannelIndex channelCount;
} AudioResamplingContext;
```

SinC function: `sin(π·x) / (π·x)` (returns 1.0 when x == 0)

Two resampling variants:

| Function | Layout | Notes |
|----------|--------|-------|
| `AudioResampleInterleaved()` | LRLRLR... | Working |
| `AudioResampleDeinterleaved()` | LLLL\|RRRR | **CRITICAL BUG — see §10.1** |

Both accept `null` output buffers to query required frame counts. Filter radius is fixed at 8.

Public wrappers auto-calculate output frame count from sample rate ratio:
```c
AudioResampleResult AudioResampleInterleaved(
    channelCount, inSampleRate, outSampleRate,
    minOutputFrameCount, maxInputFrameCount,
    inSamples, outSamples);

AudioResampleResult AudioResampleDeinterleaved(/* same params, deinterleaved ptrs */);
```

---

## 5. Format Loaders

All three loaders share the same output contract: they populate a `PCMWaveData` struct and return `bool`. The caller owns the resulting `isamples` memory and must call `FreeWave()` or `FreeWaveData()` when done.

### 5.1 WAVE Loader — `final_waveloader.h`

**Dependency:** FPL only. No third-party library.

**Supported input formats:**

| WAVE formatTag | Bit depths | Maps to |
|---------------|-----------|---------|
| PCM (1) | 8-bit | `fplAudioFormatType_U8` |
| PCM (1) | 16-bit | `fplAudioFormatType_S16` |
| PCM (1) | 24-bit | `fplAudioFormatType_S24` |
| PCM (1) | 32-bit | `fplAudioFormatType_S32` |
| IEEE Float (3) | 32-bit | `fplAudioFormatType_F32` |

**Internal structure:**

```
RIFF chunk (4 bytes "RIFF" + 4 bytes size + 4 bytes "WAVE")
  fmt  chunk (WaveFormatEx: channels, sample rate, bits/sample, ...)
  data chunk (raw PCM samples)
```

**API:**

```c
bool TestWaveHeader(const uint8_t *buffer, const size_t bufferSize);
bool LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
bool LoadWaveFromFile(const char *filePath, PCMWaveData *outWave);
bool LoadWaveFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);
void FreeWave(PCMWaveData *wave);
```

**Limitations:**
- ADPCM and other compressed WAVE variants not supported
- Error details written to `outWave->lastError` (976-byte buffer)
- No streaming; entire file loaded into memory

### 5.2 Ogg Vorbis Loader — `final_vorbisloader.h`

**Dependency:** `stb_vorbis` (black box — not documented here)

**Output format:** Always `S16` (signed 16-bit), regardless of source bit depth.

**API:**

```c
bool TestVorbisHeader(const uint8_t *buffer, const size_t bufferSize);
// Detection: checks for "OggS" magic bytes (FOURCC = 0x5367674F)

bool LoadVorbisFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
bool LoadVorbisFromFile(const char *filePath, PCMWaveData *outWave);
bool LoadVorbisFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);
// Format query uses stb_vorbis_open_memory + get_info — does not decode all samples
```

**Limitations:**
- No streaming; full decode to memory
- Output always S16, no format selection
- Minimal error reporting (no error string in PCMWaveData)

### 5.3 MP3 Loader — `final_mp3loader.h`

**Dependency:** `minimp3` (black box — not documented here)

**Output format:** Always `S16` (signed 16-bit).

**Header detection** is more complex than WAVE/Vorbis due to MPEG's variable-length frame structure:

```c
typedef enum MP3HeaderTestStatus {
    MP3HeaderTestStatus_Success = 0,
    MP3HeaderTestStatus_InvalidBuffer,
    MP3HeaderTestStatus_RequireMoreDataBegin, // Need more bytes at start
    MP3HeaderTestStatus_RequireMoreDataEnd,   // Need more bytes at end (ID3v1)
    MP3HeaderTestStatus_NoMP3,
} MP3HeaderTestStatus;

MP3HeaderTestStatus TestMP3Header(const uint8_t *buffer,
    const size_t bufferSize, size_t *requiredBufferSize);
```

Detection strategy:
1. Check for `ID3v2` tag (`"ID3"` at start)
2. Check for valid MPEG frame sync bytes
3. Check for `ID3v1` tag (`"TAG"` at `bufferSize - 128`) or `ID3v1+` (`"TAG+"` at `bufferSize - 227`)

**ID3v1 quirk:** The loader may require the *entire file* to be in the buffer to detect an ID3v1 tag at the end. `requiredBufferSize` is set when more data is needed, enabling incremental loading.

**API:**

```c
bool LoadMP3FromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
bool LoadMP3FromFile(const char *filePath, PCMWaveData *outWave);
bool LoadMP3FormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);
// Format query decodes internally and frees the result — memory-efficient probe
```

**Limitations:**
- No streaming; full decode to memory
- Output always S16, no format selection
- ID3v1 detection requires entire file buffer in worst case

---

## 6. Audio System — `final_audiosystem.h`

This is the central runtime. It manages loaded audio sources, schedules playback, and mixes samples into the output buffer on demand.

### 6.1 Key Data Structures

#### AudioSource (singly-linked list node)

```c
typedef struct AudioSource {
    AudioBuffer    buffer;  // PCM sample data (may be owned or referenced)
    AudioFormat    format;  // Sample rate, channels, format type
    AudioSourceType type;   // Allocated | Stream | File
    AudioSourceID  id;      // Unique opaque uint64_t handle
    struct AudioSource *next; // Singly-linked list
} AudioSource;
```

`AudioSourceType` enum:
- `AudioSourceType_Allocated` — source owns its sample buffer
- `AudioSourceType_Stream` — borrows a pointer from elsewhere
- `AudioSourceType_File` — loaded from disk

#### AudioPlayItem (doubly-linked list node)

```c
typedef struct AudioPlayItem {
    AudioFrameIndex framesPlayed[2]; // [0]=current, [1]=saved for preview
    fpl_b32         isFinished[2];   // [0]=current, [1]=saved for preview
    const AudioSource *source;       // Non-owning reference
    struct AudioPlayItem *next, *prev;
    AudioPlayItemID id;
    float           volume;    // Per-item volume multiplier [0.0, 1.0]
    bool            isRepeat;
} AudioPlayItem;
```

The dual-state `[2]` arrays support the `advance=false` preview mode (see §6.4).

#### AudioSystem

```c
typedef struct AudioSystem {
    AudioStaticBuffer              dspInBuffer;      // Scratch: source samples
    AudioStaticBuffer              dspOutBuffer;     // Scratch: resampled samples
    AudioStaticBuffer              mixingBuffer;     // Accumulated mix (F32)
    AudioSampleConversionFunctions conversionFuncs;  // Format conversion dispatch table
    AudioStream                    conversionBuffer; // Output staging buffer
    AudioSineWaveData              tempWaveData;     // Procedural tone generator state
    AudioFormat                    targetFormat;     // Output device format
    AudioSources                   sources;          // Registered source list + mutex
    AudioPlayItems                 playItems;        // Active play item list + mutex
    AudioMemory                    memory;           // Allocator context (currently stub)
    fplMutexHandle                 writeFramesLock;  // Guards mixing path
    float                          masterVolume;     // Global volume [0.0, 1.0]
    bool                           isShutdown;
} AudioSystem;
```

### 6.2 Threading Model

Three mutexes protect shared state:

| Mutex | Protects | Contention |
|-------|---------|------------|
| `sources.lock` | `AudioSources` linked list | Low (source load/unload) |
| `playItems.lock` | `AudioPlayItems` linked list | Medium (play/stop calls) |
| `writeFramesLock` | Entire mixing path | High (called from audio callback) |

Atomic counters:
- `sources.idCounter` — volatile, incremented with FPL atomic ops to generate unique `AudioSourceID`
- `playItems.idCounter` — same, for `AudioPlayItemID`

**Not thread-safe:**
- Direct mutation of `AudioBuffer` sample data while a source is playing
- Concurrent calls to `AudioSystemWriteFrames` (protected by `writeFramesLock`, but two callers would serialize)

### 6.3 Public API

**Lifecycle:**
```c
bool AudioSystemInit(AudioSystem *audioSys, const fplAudioFormat *targetFormat);
void AudioSystemShutdown(AudioSystem *audioSys);
void AudioSystemSetMasterVolume(AudioSystem *audioSys, float newMasterVolume);
```

**Source management:**
```c
// Allocate an empty source (caller fills samples manually)
AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys,
    AudioChannelIndex channels, AudioHertz sampleRate,
    fplAudioFormatType type, AudioFrameIndex frameCount);

// Load from disk (auto-detects WAVE / Vorbis / MP3)
AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath);
bool         AudioSystemLoadFileFormat(AudioSystem *audioSys, const char *filePath,
                                       PCMWaveFormat *outFormat);

// Load from memory buffer (auto-detects format)
AudioSource *AudioSystemLoadDataSource(AudioSystem *audioSys,
    size_t dataSize, const uint8_t *data);
bool         AudioSystemLoadDataFormat(AudioSystem *audioSys,
    size_t dataSize, const uint8_t *data, PCMWaveFormat *outFormat);

// Register a pre-built source; query sources
bool          AudioSystemAddSource(AudioSystem *audioSys, AudioSource *source);
AudioSource  *AudioSystemGetSourceByID(AudioSystem *audioSys, AudioSourceID id);
size_t        AudioSystemGetSources(AudioSystem *audioSys,
                                    AudioSource *dest, size_t maxDestCount);
void          AudioSystemClearSources(AudioSystem *audioSys);
```

**Playback control:**
```c
AudioPlayItemID AudioSystemPlaySource(AudioSystem *audioSys,
    const AudioSource *source, bool repeat, float volume);
// Returns ID with value=0 on failure

bool  AudioSystemStopOne(AudioSystem *audioSys, AudioPlayItemID playId);
void  AudioSystemStopAll(AudioSystem *audioSys);
size_t AudioSystemGetPlayItems(AudioSystem *audioSys,
    AudioPlayItem *dest, size_t maxDestCount);
```

**Frame generation (called from audio callback):**
```c
AudioFrameIndex AudioSystemWriteFrames(AudioSystem *audioSys,
    void *outSamples, const fplAudioFormat *outFormat,
    AudioFrameIndex frameCount, bool advance);
```

**Procedural generation:**
```c
void AudioGenerateSineWave(AudioSineWaveData *waveData,
    void *outSamples, fplAudioFormatType outFormat,
    AudioHertz outSampleRate, AudioChannelIndex channels,
    AudioFrameIndex frameCount);
// NOTE: TODO at line 160 — should be moved to final_audiodemo.h

bool IsAudioSampleRateSupported(AudioSystem *audioSys, AudioSampleIndex sampleRate);
// WARNING: Always returns true (see §10)
```

### 6.4 Mixing Pipeline — `AudioSystemWriteFrames`

`advance=true` is the normal audio callback path. `advance=false` is a preview/peek mode.

```
AudioSystemWriteFrames(audioSys, outSamples, outFormat, frameCount, advance)
  │
  ├── [advance=false] SavePlayStates()   // Save framesPlayed[0] → [1], isFinished[0] → [1]
  │
  ├── FillConversionBuffer(audioSys, frameCount, advance)
  │     │
  │     └── WritePlayItemsToMixer(audioSys, frameCount, advance)
  │           │
  │           ├── Clear mixingBuffer to zero (F32)
  │           │
  │           └── For each AudioPlayItem:
  │                 ├── 1. ConvertToF32(source sample, channel, format)
  │                 │      Supports: S16, S32, F32  [U8/S24 fall to 0.0 — see §10]
  │                 ├── 2. Sample rate conversion (if needed)
  │                 │      ratio = sourceRate / targetRate
  │                 │      ratio > 1: AudioSimpleUpSampling   (frame duplication)
  │                 │      ratio < 1: AudioSimpleDownSampling (frame skipping)
  │                 │      ratio = 1: pass through
  │                 ├── 3. Apply per-item volume
  │                 ├── 4. MixSamples() — accumulate (+=) into mixingBuffer
  │                 │      Handles: mono↔stereo channel mapping
  │                 │      Missing: arbitrary channel count mapping (TODO)
  │                 └── 5. If finished and !repeat: RemovePlayItem()
  │
  ├── Convert mixingBuffer (F32) → outFormat via ConvertFromF32
  │      Supports: S16, S32, F32  [other formats silently produce 0 — see §10]
  │
  ├── Copy conversionBuffer → outSamples
  │
  └── [advance=false] RestorePlayStates()  // Restore [1] → [0]
```

**Resampling note:** The simple up/down sampling (frame duplication/skipping) is only correct for integer ratios (e.g. 2x, 4x). Fractional ratios produce audible artifacts. A proper SinC or linear interpolation resampler is listed as a TODO (see §10).

### 6.5 Format Detection — `PropeAudioFileFormat`

```c
static AudioFileFormat PropeAudioFileFormat(AudioSystemStream *stream);
```

Note: **Typo in function name** — "Prope" should be "Probe". Tests WAVE, Vorbis, and MP3 headers in that order and returns the first match.

---

## 7. Lock-Free Ring Buffer — `final_buffer.h`

### 7.1 Mirrored Memory

An optimization for ring buffers: map the same physical memory twice in virtual address space so that a contiguous read/write always succeeds even at the wrap-around point.

```c
typedef struct MirroredMemory {
    HANDLE *fileHandle; // Windows: memory-mapped file object
    void   *buffer;     // Virtual base pointer (length * count bytes)
    size_t  length;     // Length of one mirror
    size_t  count;      // Number of mirrors (2 for a double-mirror)
    fpl_b32 isValid;
} MirroredMemory;

bool InitMemoryMirror(MirroredMemory *mem, size_t length, size_t count);
void ReleaseMemoryMirror(MirroredMemory *mem);
```

**Platform support:**
- **Windows:** Uses `VirtualAlloc3` + `MapViewOfFile3` (modern Windows) with a fallback to `VirtualAlloc` + manual retry
- **Other platforms:** Always returns `false` — no mmap implementation (TODO)

Length is rounded up to system allocation granularity.

### 7.2 LockFreeRingBuffer

```c
typedef struct LockFreeRingBuffer {
    // Each field is padded to 64 bytes to prevent false-sharing
    HANDLE          *fileHandle;      // Windows only
    void            *buffer;
    uint64_t         length;          // Total capacity in bytes
    uint64_t         tail;            // Read cursor (consumer advances)
    uint64_t         head;            // Write cursor (producer advances)
    volatile int64_t fillCount;       // Available bytes for reading (atomic)
    fpl_b32          isMirror;        // True if using mirrored memory
} LockFreeRingBuffer;
```

All fields padded to 64-byte cache lines to prevent false-sharing between producer and consumer threads.

**API:**

```c
bool LockFreeRingBufferInit(LockFreeRingBuffer *buffer, size_t length, bool allowMirror);
void LockFreeRingBufferRelease(LockFreeRingBuffer *buffer);

bool LockFreeRingBufferCanRead(LockFreeRingBuffer *buffer, size_t *availableBytes);
bool LockFreeRingBufferRead(LockFreeRingBuffer *buffer, void *dst, size_t len);
bool LockFreeRingBufferSkip(LockFreeRingBuffer *buffer, size_t length);
bool LockFreeRingBufferPeek(LockFreeRingBuffer *buffer, void *dst, size_t offset, size_t len);

bool LockFreeRingBufferCanWrite(LockFreeRingBuffer *buffer, size_t *availableBytes);
bool LockFreeRingBufferWrite(LockFreeRingBuffer *buffer, const void *src, size_t len);

void LockFreeRingBufferClear(LockFreeRingBuffer *buffer);

void LockFreeRingBufferUnitTest(); // DISABLED — see §10.2
```

All read/write/peek operations return `false` on failure (underflow / overflow). No exceptions or error strings.

### 7.3 Current Usage

`final_buffer.h` is included by `final_audiodemo.h` but **not actively used** anywhere in the audio stack. It exists as a utility for audio streaming/buffering but no streaming loader has been implemented yet.

---

## 8. Track Management / Demo Layer — `final_audiodemo.h`

This is an application-level helper for managing a playlist of up to 8 audio tracks.

### 8.1 Key Types

```c
typedef enum AudioTrackState {
    AudioTrackState_Failed        = -1,
    AudioTrackState_Unloaded      =  0,
    AudioTrackState_AquireLoading =  1,  // BUG: Typo — should be "AcquireLoading"
    AudioTrackState_Loading       =  2,
    AudioTrackState_Ready         =  3,  // Buffered enough to start playing
    AudioTrackState_Full          =  4,  // Fully loaded
} AudioTrackState;
```

```c
typedef struct AudioTrack {
    AudioTrackSource source;          // File path or in-memory data
    char             name[256];
    AudioBuffer      outputFullBuffer; // Full decoded samples (for spectrum visualization)
    AudioSourceID    sourceID;
    AudioPlayItemID  playID;
    volatile int32_t state;           // Atomic: AudioTrackState
    int              loadingPercentage; // 0–100
} AudioTrack;

#define MAX_AUDIO_TRACK_LIST_COUNT 8

typedef struct AudioTrackList {
    AudioTrack tracks[MAX_AUDIO_TRACK_LIST_COUNT];
    uint32_t   count;
    int32_t    currentIndex;  // -1 = none
    int32_t    lastIndex;
    fpl_b32    changedPending; // Set when track change needs UI sync
} AudioTrackList;
```

### 8.2 Public API

```c
// Check if current track is ready (state >= Ready)
bool HasAudioTrack(AudioTrackList *tracklist);

// Stop all tracks and reset list
void StopAllAudioTracks(AudioSystem *audioSys, AudioTrackList *tracklist);

// Start playing track at index
bool PlayAudioTrack(AudioSystem *audioSys, AudioTrackList *tracklist, uint32_t index);

// Load multiple sources into tracklist
bool LoadAudioTrackList(AudioSystem *audioSys,
    const AudioTrackSource *sources, size_t sourceCount,
    bool forceSineWave, const AudioSineWaveData *sineWave,
    LoadAudioTrackFlags flags, AudioTrackList *tracklist);
```

`LoadAudioTrackFlags`:
- `LoadAudioTrackFlags_AutoLoad` — begin loading immediately
- `LoadAudioTrackFlags_AutoPlay` — also start playback after load

### 8.3 Track Source Types

```c
typedef struct AudioTrackSource {
    char                name[256];
    AudioTrackSourceType type;       // URL or Data
    union {
        AudioTrackDataSource data;   // { const uint8_t *data; size_t size; }
        AudioTrackURLSource  url;    // { char urlOrFilePath[1024]; }
    };
} AudioTrackSource;
```

---

## 9. Data Flow Diagrams

### 9.1 Audio Loading Pipeline

```
[File on disk]                         [In-memory buffer]
      │                                       │
      ▼                                       ▼
 fplFileOpen()                     (raw bytes already in memory)
      │                                       │
      └──────────────┬────────────────────────┘
                     │
                     ▼
          PropeAudioFileFormat()     ← probes stream header
                     │
          ┌──────────┼──────────┐
          ▼          ▼          ▼
     LoadWave()  LoadVorbis()  LoadMP3()
          │          │            │
          └──────────┴────────────┘
                     │
                     ▼
              PCMWaveData
              { isamples, format, samplesSize }
                     │
                     ▼
        AudioSystemLoadDataSource()
          ├── AllocateAudioBuffer()       ← copy samples to AudioBuffer
          ├── Creates AudioSource
          └── AudioSystemAddSource()      ← registers in sources list
                     │
                     ▼
              AudioSource  ←──── AudioSourceID (opaque handle)
```

### 9.2 Mixing / Playback Pipeline

```
[Audio callback: AudioSystemWriteFrames(advance=true)]
                     │
                     ▼
         FillConversionBuffer()
                     │
                     ▼
         WritePlayItemsToMixer()
                     │
          ┌──────────┴────────────────────────────┐
          │  For each AudioPlayItem               │
          │                                       │
          │  Source PCM samples (S16/S32/F32)     │
          │    │                                  │
          │    ▼                                  │
          │  ConvertToF32()                       │
          │  [per-sample, supports S16/S32/F32]   │
          │    │                                  │
          │    ▼                                  │
          │  AudioSimpleUpSampling /              │
          │  AudioSimpleDownSampling              │
          │  [frame duplication / skipping]       │
          │    │                                  │
          │    ▼                                  │
          │  Apply volume (item × master)         │
          │    │                                  │
          │    ▼                                  │
          │  MixSamples() ─── += mixingBuffer     │
          │                                       │
          └───────────────────────────────────────┘
                     │
                     ▼
            ConvertFromF32()
            [F32 → target format: S16/S32/F32]
                     │
                     ▼
           Copy to outSamples buffer
                     │
                     ▼
             [hardware / FPL audio callback]
```

### 9.3 Ring Buffer Layout (when used)

```
Physical memory:  [AAAAAAAAAAAAAAAA]
Virtual layout:   [AAAAAAAAAAAAAAAA][AAAAAAAAAAAAAAAA]
                   ^                ^
                   mirror 0         mirror 1 (same pages)

Ring buffer:
  tail ──▶ [........RRRRRRRRRWWWWWWWW........] ◀── head
            consumed  readable  writable  free

Wrap-around read (no copy needed with mirror):
  [XXXXXXXXWWWWWWWW][RRRRRRRRRXXXXXXX]
                     ^─ contiguous with mirror ─^
```

---

## 10. Weak Points, Issues & Critical Bugs

### 10.1 CRITICAL: Deinterleaved Resampling Out-of-Bounds Access

**File:** `final_audioconversion.h` lines 491–492

```c
// WRONG:
const float *channelInSamples  = &inSamples[channel][sourceFrameCount];
float       *channelOutSamples = &outSamples[channel][targetFrameCount];

// Correct:
const float *channelInSamples  = inSamples[channel];
float       *channelOutSamples = outSamples[channel];
```

`sourceFrameCount` and `targetFrameCount` are the *sizes* of the arrays, not offsets into them. The current code immediately reads and writes one-past-end, producing undefined behavior (likely a crash or garbage output) whenever `AudioResampleDeinterleaved` is called.

### 10.2 CRITICAL: Ring Buffer Unit Test Disabled and Broken

**File:** `final_buffer.h` line 486

```c
extern void LockFreeRingBufferUnitTest() {
    // @FIXME(tspaete): This is totally broken right now and needs to be fixed,
    // due to change from page to allocation granularity size
#if 0
    ...
#endif
}
```

The entire test body is `#if 0`'d out. The ring buffer has never been validated since a refactor changed it from page-aligned to allocation-granularity-aligned allocation. Correctness of the lock-free implementation is unverified.

### 10.3 CRITICAL: Mirror Buffer Windows-Only

**File:** `final_buffer.h`

`InitMemoryMirror` returns `false` on every non-Windows platform. There is no `mmap`-based fallback. On Linux/macOS, `LockFreeRingBuffer` always allocates via `malloc` and handles wrap-around in software, losing the zero-copy benefit of mirrored memory.

### 10.4 MAJOR: ConvertToF32 / ConvertFromF32 Silent Failures

**File:** `final_audiosystem.h` lines 791–842

`ConvertToF32` only handles `S16`, `S32`, `F32`. For all other types (including `U8`, `S24`) it falls through the `default:` branch and **silently returns 0.0**. Similarly, `ConvertFromF32` only handles `S16`, `S32`, `F32` and silently writes nothing for other formats. WAVE files with `U8` or `S24` samples will produce silent output when played back through the audio system, with no error reported.

These are internal functions used by the mixing loop, separate from the full conversion dispatch table in `final_audioconversion.h` (which does support U8/S24). The mixing loop uses the simpler inline versions.

### 10.5 MAJOR: Naive Sample Rate Conversion

**File:** `final_audiosystem.h`

The mixing pipeline uses frame duplication (up) and frame skipping (down) for sample rate conversion. This is only accurate for integer multiples (2x, 3x, 4x, etc.). Fractional ratios (e.g. 44100→48000) produce audible pitch/timing errors and aliasing. The `final_audioconversion.h` SinC resampler exists but is **not used by the mixing loop**.

> TODO comment in `final_audiosystem.h` line 27: *"Separate sample rate conversion from mixing (Doing the sample rate conversion inside the mixing is stupid)"*

### 10.6 MAJOR: No SIMD Optimizations

**File:** `final_audioconversion.h` (CreateAudioSamplesConversionFunctions)

The SIMD dispatch hook exists as a comment (TODO after function pointer table initialization). All format conversions, interleave/deinterleave operations, and SinC resampling are entirely scalar. This limits throughput for high channel counts or high sample rates.

### 10.7 MAJOR: Per-Sample Operations in Hot Path

**File:** `final_audiosystem.h` (WritePlayItemsToMixer)

Format conversion (`ConvertToF32`) and volume application happen **one sample at a time** inside the innermost loop. The TODO list explicitly calls this out:

> *"Do format conversion <-> float for multiple frames, not just one sample"*
> *"Unroll loops (x4)"*
> *"SIMD everything"*

### 10.8 MAJOR: No Streaming Decoders

All three loaders (`LoadWaveFromFile`, `LoadVorbisFromFile`, `LoadMP3FromFile`) decode the entire file into memory before returning. For long audio files (music tracks, ambient audio) this can consume significant heap memory. A ring-buffer-based streaming path exists conceptually (`LockFreeRingBuffer` + `AudioStream`) but no streaming decoder is implemented.

### 10.9 MAJOR: Mutexes in Audio Callback

**File:** `final_audiosystem.h`

`AudioSystemWriteFrames` acquires `writeFramesLock`, `playItems.lock`, and potentially `sources.lock` on the audio callback thread. Mutex contention on the audio thread causes glitches and timing jitter. The TODO explicitly calls for a lock-free design.

### 10.10 MINOR: `IsAudioSampleRateSupported` Always Returns `true`

**File:** `final_audiosystem.h` lines 1331–1341

```c
fpl_extern bool IsAudioSampleRateSupported(AudioSystem *audioSys, AudioSampleIndex sampleRate) {
    ...
#if 1
    return true;  // Correct check disabled
#else
    return AreSampleRatesEven(sampleRate, audioSys->targetFormat.sampleRate);
#endif
}
```

The actual validation (`AreSampleRatesEven`) is commented out. Callers cannot rely on this function to reject incompatible sample rates.

### 10.11 MINOR: Typo — `PropeAudioFileFormat`

**File:** `final_audiosystem.h` line 341

Function name is `Prope...` instead of `Probe...`. This is an internal function so it does not affect the public API, but it is confusing.

### 10.12 MINOR: Typo — `AudioTrackState_AquireLoading`

**File:** `final_audiodemo.h` line 20

`AquireLoading` should be `AcquireLoading`. This is a public enum value and would require a breaking change to fix.

### 10.13 MINOR: `AudioGenerateSineWave` in Wrong File

**File:** `final_audiosystem.h` line 160

```c
// @TODO(final): Move to final_audiodemo.h, make a audio source more "Generative"
fpl_extern void AudioGenerateSineWave(...);
```

A procedural test-tone generator is a demo/debugging utility, not a core system concern.

### 10.14 MINOR: No Smooth Volume Fade

**File:** `final_audiosystem.h`

Volume changes take effect immediately (step function), causing audible clicks when volume is changed while audio is playing. A per-frame linear fade is noted as a TODO.

### 10.15 MINOR: Fixed Track List Size

**File:** `final_audiodemo.h`

`MAX_AUDIO_TRACK_LIST_COUNT = 8` is a compile-time constant with no dynamic growth. Acceptable for a demo layer but would need to change for production use.

### 10.16 MINOR: `AudioMemory` is a Stub

**File:** `final_audiosystem.h`

```c
typedef struct AudioMemory {
    // @TODO(final): Better memory management for audio system
} AudioMemory;
```

The struct is empty. All allocations currently go directly to `fplMemoryAllocate` / `fplMemoryFree`. A custom pool allocator (to avoid heap fragmentation on the audio thread) is listed as a future goal.

---

## 11. Stable & Working Areas

The following components are functional and produce correct output within their documented constraints:

| Component | Status | Notes |
|-----------|--------|-------|
| WAVE Loader | Stable | All 5 PCM+float formats; good error reporting |
| Vorbis Loader | Stable | Decodes correctly; always outputs S16 |
| MP3 Loader | Stable | ID3v2/ID3v1 detection; always outputs S16 |
| `AudioSamplesConvert` | Stable | All 8 F32↔integer pairs; correct math |
| `AudioSamplesInterleave` | Stable | U8/S16/S32/F32 with generic fallback |
| `AudioSamplesDeinterleave` | Stable | Same |
| `AudioResampleInterleaved` | Stable | SinC correct; use fixed radius=8 |
| FFT / Window Functions | Stable | Power-of-2 only; disabled self-test |
| `AudioBufferPointer` | Stable | All iterator functions correct |
| AudioSystem init/shutdown | Stable | Source and playback lifecycle correct |
| `AudioSystemWriteFrames` (S16/S32/F32 sources) | Functional | Correct with caveats on SRC quality |
| Preview mode (`advance=false`) | Functional | Dual-state save/restore works correctly |
| `LockFreeRingBuffer` I/O | Likely correct | **Unit test disabled** — not formally verified |
