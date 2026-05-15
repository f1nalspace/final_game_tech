# Plan — Robust Audio Backend / Format Probing

Status: planned. Owner: `fpl__InitAudio` and helpers in `final_platform_layer.h`.

## Goal

Make backend selection in `fpl__InitAudio` deterministic and predictable: the user states a target `(sampleRate, channels/layout, type)` in `fplAudioSettings.targetFormat`, and FPL picks the **best** backend that can serve that configuration without any FPL-side conversion.

Sample rate is the first-class field — relaxed only at the very last tier. Channels and type are relaxed in stages.

## Current state (post fix-1, fix-2)

- WASAPI shared mode is strict (`IsFormatSupported == S_OK` or fail with `UnsuportedDeviceFormat`).
- `fpl__InitAudio` probe loop iterates fallback-outer, backend-inner.
- Fallback rounds come from `fpl__global_AudioFormat_FallbackFields[]` — a mixed bag of `_Type`, `_SampleRate`, `_Type|_SampleRate`, `_Channels`, ... no fixed semantics about which field is "sacred" at which tier.
- `fpl__PopulateFallbackAudioFormats` expands a mask into a cartesian product of the relaxed axes using `FallbackTypes / FallbackSampleRates / FallbackChannels`.

This works for the WASAPI-vs-DSound case but is not specified — the order of fallback rounds in `FallbackFields[]` accidentally controls behavior.

## Target probing spec

For every tier below, the inner loop is **backends** — every registered backend gets a shot at the current candidate format before we move on to the next candidate. First success wins and short-circuits the whole probe.

The `currentTargetFormat` constructed for each attempt always keeps the user's original `targetFormat` fields except where the tier explicitly substitutes from a fallback table.

### Tier 1 — Perfect match

Candidate set: `{ (user.sampleRate, user.channels, user.type) }` (one candidate).

For each backend, try the exact requested format. First match wins.

### Tier 2 — Sample rate + channels match (type relaxed)

Candidate set: `{ (user.sampleRate, user.channels, T) | T ∈ fpl__global_AudioFormat_FallbackTypes }`.

For each candidate (in fallback-table order), probe every backend. First match wins. Result: rate and channel count are guaranteed to match the user; type may have shifted to whatever the first willing backend serves.

### Tier 3 — Sample rate + type match (channels relaxed)

Candidate set: `{ (user.sampleRate, C, user.type) | C ∈ fpl__global_AudioFormat_FallbackChannels }`.

For each candidate, probe every backend. First match wins. Rate and type are guaranteed; channels may shift.

### Tier 4 — Sample rate match only (channels and type relaxed)

Candidate set: cartesian product `{ (user.sampleRate, C, T) | C ∈ FallbackChannels, T ∈ FallbackTypes }`.

For each candidate, probe every backend. First match wins. Only rate is guaranteed.

### Tier 5 — Total fallback

Candidate set: full cartesian product `{ (R, C, T) | R ∈ FallbackSampleRates, C ∈ FallbackChannels, T ∈ FallbackTypes }`.

For each candidate, probe every backend. First match wins. The user gets *something*, but the sample rate is no longer guaranteed.

If Tier 5 produces no match, `fpl__InitAudio` returns `fplAudioResultType_NoBackendsFound` / `UnsuportedDeviceFormat` as today.

## Notes on the spec

- **Why sample rate is sacred until Tier 5**: changing rate forces resampling on the caller, which is the hardest conversion to write correctly for game/audio code. Channel mixing (mono↔stereo) and type conversion (S16↔F32) are trivial in comparison.
- **Backend order inside each tier**: preserved from the descriptor list returned by `fpl__GetAudioBackendDescriptors`. On Windows that puts WASAPI before DirectSound; if WASAPI rejects, DSound gets the same candidate before either backend moves to the next candidate.
- **No backend silently substitutes**: every backend must already reject formats it cannot natively serve (WASAPI does this post-fix-1). If a backend still negotiates internally, the tier logic above can't enforce the user's preference for it.

## Required code changes

### Constants / tables

`fpl__global_AudioFormat_FallbackFields[]` should be replaced (or repurposed) with an enum or array describing the **tiers** rather than ad-hoc masks. Proposed:

```c
typedef enum fpl__AudioProbeTier {
    fpl__AudioProbeTier_Exact = 0,           // user.rate + user.channels + user.type
    fpl__AudioProbeTier_RateAndChannels,     // user.rate + user.channels + FallbackTypes
    fpl__AudioProbeTier_RateAndType,         // user.rate + FallbackChannels + user.type
    fpl__AudioProbeTier_RateOnly,            // user.rate + FallbackChannels × FallbackTypes
    fpl__AudioProbeTier_Anything,            // FallbackSampleRates × FallbackChannels × FallbackTypes
    fpl__AudioProbeTier_Count,
} fpl__AudioProbeTier;
```

The existing `fpl__global_AudioFormat_FallbackTypes / FallbackChannels / FallbackSampleRates` arrays stay as-is and become the substitution sources per tier.

### Candidate-set generator

`fpl__PopulateFallbackAudioFormats` (mask-driven cartesian product) is too generic for the new spec. Replace with a tier-aware builder, e.g.:

```c
fpl_internal size_t fpl__PopulateProbeCandidates(
    fpl__AudioProbeTier tier,
    const fplAudioFormat *userTarget,
    size_t maxOut,
    fplAudioFormatU64 *outCandidates);
```

It writes the candidate list for the given tier and returns the count. Each tier has a deterministic order matching the spec above.

The `fplAudioDefaultFields` flag-mask machinery used by the populator can stay for the public API (`audioSettings->targetFormat.defaultFields` still has its meaning at the caller level) — but the probe internals no longer need it.

### Probe loop

Rewrite the loops in `fpl__InitAudio` as:

```c
for (tier = 0; tier < fpl__AudioProbeTier_Count && !probeSucceeded; ++tier) {
    size_t candidateCount = fpl__PopulateProbeCandidates(tier, &audioSettings->targetFormat, ..., candidates);
    for (i = 0; i < candidateCount && !probeSucceeded; ++i) {
        fplAudioFormat currentTarget = audioSettings->targetFormat;
        decode candidate into currentTarget;
        for (b = 0; b < audioBackendCount && !probeSucceeded; ++b) {
            reset backend memory
            initialize(backend)
            initializeDevice(backend, &currentTarget, ...)
            if success: probeSucceeded = true; commit
            else: releaseDevice + release
        }
    }
}
```

Backend init/release per attempt is the cost of correctness — backends share one memory chunk, we cannot hold more than one open at a time. WASAPI's `initialize()` does a `CoCreateInstance(IMMDeviceEnumerator)` (cheap, ms-scale); DSound's is a `LoadLibrary` (also cheap). With ~5 tiers × small candidate counts × 2 backends on Windows, total cost is bounded and only paid once at platform init.

### Deletions / cleanups

- `fplWasapiAudioSettings.autoConvertSampleRate` — removed (WASAPI is strict-only after this; the spec already covers the negotiation in the tier loop).
- The empty `fplWasapiAudioSettings` struct can be kept as a forward-compat placeholder, or removed along with its union member.
- `fpl__global_AudioFormat_FallbackFields[]` — removed (replaced by tier enum).
- `fpl__PopulateFallbackAudioFormats` — replaced by tier-aware `fpl__PopulateProbeCandidates`.

### Documentation

- Update the changelog under "Fixed" with a single short line, e.g.: `Fixed: Audio backend probing is now tier-based — perfect match → rate+channels → rate+type → rate → full fallback — keeping sample rate sacred until the last tier`.
- Update the doxygen comment block on `fplAudioSettings` / `targetFormat` to describe the tier order so callers know exactly which fields the platform will honor before degrading.

## Test plan

After the rewrite, the SimpleAudio / WaveAudio demos (which hardcode S16/44100 stereo) should:

- **Windows with WASAPI mix format = 48000/F32**:
  - Tier 1 (44100/S16/2ch) on WASAPI → reject.
  - Tier 1 on DirectSound → accept (DS supports 44100/S16 natively).
  - Demo plays.
- **Windows with no DSound (FPL_NO_AUDIO_DIRECTSOUND)**:
  - Tier 1 / 2 / 3 / 4 on WASAPI for 44100 → all reject (WASAPI shared can only do its mix rate natively).
  - Tier 5 on WASAPI at 48000/F32/2ch → accept.
  - Demo callback receives 48000/F32 and bails on `type != S16` (expected — design says caller does conversion).
- **Linux with PulseAudio / ALSA / PipeWire**:
  - Tier 1 typically accepts directly. No regression.

Manual test matrix: the three demos × {WASAPI default, DSound forced, ALSA, Pulse, PipeWire, OSS}.
