# Plan: Fix FFT Visualization and Spectrum Analyzer

## Context

Audio player (`demos/FPL_AudioPlayer/fpl_audioplayer.c`) visualizes mono audio chunks via FFT bars and a spectrum analyzer. Both are broken:
- FFT bars flicker due to per-frame dynamic min/max normalization — scale collapses differently every frame
- Spectrum analyzer misses all frequencies below 400 Hz (sine sweep never appears in low bands)
- Last spectrum bar (index 31) is always 0 because bin boundary array is one entry short
- Window function uses periodic form (divide by N) instead of symmetric (divide by N−1)

---

## Task 1: Fix FFT Visualization

**Files:** `demos/FPL_AudioPlayer/fpl_audioplayer.c`, `demos/additions/final_audio.h`

### 1a. Fix window function denominator (final_audio.h line 523)
```c
// Before
double k = 2.0 * M_PI * index / (double)N;
// After (symmetric/analysis window)
double k = 2.0 * M_PI * index / (double)(N - 1);
```

### 1b. Fix ForwardFFT normalization (final_audio.h lines 440-443)
Change `HalfNormalizeFFT` → `NormalizeFFT` so output divides by N (not sqrt(N)).
This gives amplitude-correct output: DC signal of amplitude A → magnitude A.
```c
static void ForwardFFT(const FFTDouble* in, const size_t size, FFTDouble* out) {
    FFTCore(in, size, 1, out, FFTDirection_Forward);
    NormalizeFFT(out, size);   // was HalfNormalizeFFT
}
```

### 1c. Replace dynamic min/max with fixed dB scaling (fpl_audioplayer.c lines 595-638)

Remove the `log(1 + magnitude)` heuristic and the per-frame min/max tracking entirely.

New approach:
```c
const double minDb = -120.0;
const double maxDb = 0.0;
const double dbRange = maxDb - minDb;
for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    double re = visualization->fftOutput[frameIndex].real;
    double im = visualization->fftOutput[frameIndex].imag;
    double rawMagnitude = sqrt(re * re + im * im);
    // Factor of 2 for single-sided representation (skip for DC bin 0 and Nyquist bin N/2)
    double amplitude = (frameIndex > 0 && frameIndex < halfFFT) ? rawMagnitude * 2.0 : rawMagnitude;
    double dB = 20.0 * log10(amplitude + 1e-10);
    double scaledMagnitude = fplClamp((dB - minDb) / dbRange, 0.0, 1.0);
    visualization->lastMagnitudes[frameIndex] = visualization->currentMagnitudes[frameIndex];
    visualization->currentMagnitudes[frameIndex] = scaledMagnitude;
}
```
Remove lines 617-638 (min/max tracking + linear normalization loop) entirely.
`scaledMagnitudes[i]` can then just be copied from `currentMagnitudes[i]` after smoothing.

### 1d. Replace flat smoothing with asymmetric fast-attack/slow-decay (fpl_audioplayer.c lines 608-615)
```c
const double attackSmooth = 0.8;   // fast rise
const double decaySmooth  = 0.05;  // slow fall
for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
    double last    = visualization->lastMagnitudes[frameIndex];
    double current = visualization->currentMagnitudes[frameIndex];
    double factor  = (current > last) ? attackSmooth : decaySmooth;
    visualization->currentMagnitudes[frameIndex] = last * (1.0 - factor) + current * factor;
}
```

### 1e. Fix off-by-one in scaledSamples loops (fpl_audioplayer.c lines 572, 579)
```c
// Both loops: change from frameIndex = 1 to frameIndex = 0
for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
```

---

## Task 2: Fix Spectrum Analyzer

**File:** `demos/FPL_AudioPlayer/fpl_audioplayer.c`

### 2a. Add bins+1 boundary array to AudioVisualization struct (~line 275)
```c
// Before
double bins[MAX_AUDIO_BIN_COUNT];
// After (need N+1 boundary points for N bands)
double bins[MAX_AUDIO_BIN_COUNT + 1];
```

### 2b. Add lastSpectrum array to AudioVisualization struct
```c
double lastSpectrum[MAX_AUDIO_BIN_COUNT];
```

### 2c. Fix GenerateFrequencyBins (fpl_audioplayer.c line 1211-1223)
- Change `minHearableFreq` from 400.0 → 20.0 (covers full audible range)
- Accept `binCount + 1` entries and fill N+1 boundary points:
```c
static void GenerateFrequencyBins(const uint32_t binCount, const uint32_t sampleRate, double *bins) {
    const double nyquist = sampleRate * 0.5;
    const double minFreq = 20.0;
    const double maxFreq = fplMin(20000.0, nyquist);
    const uint32_t N = binCount;  // N+1 entries: bins[0]..bins[N]
    for (uint32_t i = 0; i <= N; i++) {
        bins[i] = minFreq * pow(maxFreq / minFreq, (double)i / N);
    }
}
```

### 2d. Fix InitializeVisualization call (line 1228)
No change needed to call — function now fills `binCount + 1` = 33 entries for 32 bands.
Pass the same `MAX_AUDIO_BIN_COUNT` — the function now interprets this as the band count and fills `bins[0..32]`.

### 2e. Fix spectrum loop (fpl_audioplayer.c lines 641-655)
Change `binIndex < binCount - 1` → `binIndex < binCount` (all 32 bars).
Use `currentMagnitudes` (already fixed dB-scaled) directly:
```c
for (uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
    visualization->lastSpectrum[binIndex] = visualization->spectrum[binIndex];
    visualization->spectrum[binIndex] = 0.0;
    double lowerFrequency = visualization->bins[binIndex];
    double upperFrequency = visualization->bins[binIndex + 1];  // safe: bins[binCount] exists
    for (uint32_t frameIndex = 0; frameIndex < halfFFT; ++frameIndex) {
        double frameFreq = (frameIndex * (double)demo->targetAudioFormat.sampleRate) / (double)frameCount;
        if (frameFreq >= lowerFrequency && frameFreq < upperFrequency) {
            double mag = visualization->currentMagnitudes[frameIndex];
            if (mag > visualization->spectrum[binIndex])
                visualization->spectrum[binIndex] = mag;
        }
    }
}
```

### 2f. Add asymmetric smoothing for spectrum bars (after the loop above)
```c
const double specAttack = 0.7;
const double specDecay  = 0.08;
for (uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
    double last    = visualization->lastSpectrum[binIndex];
    double current = visualization->spectrum[binIndex];
    double factor  = (current > last) ? specAttack : specDecay;
    visualization->spectrum[binIndex] = last * (1.0 - factor) + current * factor;
}
```

---

## Critical Files
- `demos/FPL_AudioPlayer/fpl_audioplayer.c` — Render(), GenerateFrequencyBins(), InitializeVisualization(), AudioVisualization struct
- `demos/additions/final_audio.h` — ForwardFFT(), WindowFunctionCore()

## Verification
1. Build and run FPL_AudioPlayer
2. Play a pure sine wave at a known frequency (e.g., 440 Hz) — FFT should show a single stable peak at bin corresponding to 440 Hz, no flickering
3. Sweep sine from 20 Hz to 20 kHz — spectrum bars should light up from left to right in order
4. Play silence — all bars should smoothly fall to zero
5. Play music — FFT bars should be stable and proportional without flickering or collapsing
