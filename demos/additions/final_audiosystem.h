/*
Name:
	Final Audio System

Description:
	Audio system for loading mixing/converting audio streams.

	This file is part of the final_framework.

How everything works:

Heart is the AudioSystemWriteFrames() function that generates X-samples worth of N-audio frames.
It's the function you call to get rendered audio output from all currently playing sounds.
Think of it as your "rendering engine" that combines all audio streams and produces the final mixed output.

The entire pipeline system uses 4 scratch buffers to process samples:
- DSP-In Buffer: Stores the source samples, but converted to F32
- DSP-Out Buffer: Stores the resampled samples as F32
- Mixing Buffer: Stores the samples from all audio sources mixed together as F32
- Conversion Buffer: Stores the samples in the target audio format (sample rate, type, channels)

The conversion buffer is special, because it will always be consumed first, before more samples are produced by the FillConversionBuffer() function.

There can be N audio sources, each audio source has a sample buffer that is based on either external or internal memory
There can be N play items, each play item tracks the current state of a audio source.

There are several API functions for adding/loading audio sources, start playback by adding play-items, change-volume etc.

How audio frame writing/generation works:

- The samples that are left in the conversion buffer are written first
- If the conversion buffer is empty, new samples are produced
- Samples are produced by looping over all play items and:
	- Convert them to float into the DSP-In buffer
	- Resample them to the target sample rate from the DSP-In buffer into the DSP-Out buffer
	- Apply the volume to each sample
	- Write the samples to the mixing buffer
- Write samples until no frames are left
- Clear remaining frames to zero

Diagram of audio frame generation:

┌─────────────────────────────────────────────────────────────────────────┐
│                     AudioSystemWriteFrames                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  1. CLEAR MIXING BUFFER                                                 │
│     ┌──────────────┐                                                    │
│     │ Mixing Buffer│                                                    │
│     │     [0]      │ ← All zeros                                        │
│     └──────────────┘                                                    │
│                                                                         │
│  2. PROCESS EACH PLAYING SOUND                                          │
│     ┌─────────────────────────────────────────────────────────────────┐ │
│     │ For each AudioPlayItem (sound)                                  │ │
│     │ ┌─────────────────────────────────────────────────────────────┐ │ │
│     │ │ SAMPLE RATE CONVERSION                                      │ │ │
│     │ │ Source Rate    →  Output Rate                               │ │ │
│     │ │ Example: 44100Hz → 48000Hz                                  │ │ │
│     │ └─────────────────────────────────────────────────────────────┘ │ │
│     │                                                                 │ │
│     │ ┌─────────────────────────────────────────────────────────────┐ │ │
│     │ │ FORMAT CONVERSION                                           │ │ │
│     │ │ int16 / int32  →  float32 (-1.0 to 1.0)                     │ │ │
│     │ └─────────────────────────────────────────────────────────────┘ │ │
│     │                                                                 │ │
│     │ ┌─────────────────────────────────────────────────────────────┐ │ │
│     │ │ MIXING ( += )                                               │ │ │
│     │ │ MixingBuffer += ConvertedSample * Volume                    │ │ │
│     │ └─────────────────────────────────────────────────────────────┘ │ │
│     └─────────────────────────────────────────────────────────────────┘ │
│                                                                         │
│  3. CLIPPING & FORMAT CONVERSION                                        │
│     ┌─────────────────────────────────────────────────────────────────┐ │
│     │ ┌─────────────────┐                                             │ │
│     │ │ Clip to [-1, 1] │ ──→ Target Format (S16/S32/F32)             │ │
│     │ └─────────────────┘                                             │ │
│     └─────────────────────────────────────────────────────────────────┘ │
│                                                                         │
│  4. WRITE TO OUTPUT BUFFER                                              │
│     ┌─────────────────────────────────────────────────────────────────┐ │
│     │ outSamples ← Final mixed and converted output                   │ │
│     └─────────────────────────────────────────────────────────────────┘ │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘

Data flow diagram:

┌──────────────┐
│ AudioSource  │ ───► Sample Buffer ───► Float Conversion ───►
└──────────────┘                                       Sample Stream
														│
														▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  AudioPlay   │───►│  Sample Rate │───►│   Channel    │
│    Item 1    │    │  Converter   │    │   Mixer      │
└──────────────┘    └──────────────┘    └──────────────┘
													│
													▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  AudioPlay   │───►│  Sample Rate │───►│   Channel    │
│    Item 2    │    │  Converter   │    │   Mixer      │
└──────────────┘    └──────────────┘    └──────────────┘
													│
													▼
┌─────────────────────────────────────────────────────────────────┐
│                    MIXING BUFFER                                │
│  All sources accumulate here: Buffer += Sample × Volume         │
└─────────────────────────────────────────────────────────────────┘
														  │
														  ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   MASTER     │───►│    CLIP      │───►│ FORMAT       │
│   VOLUME     │    │  [-1 to 1]   │    │ CONVERT      │
└──────────────┘    └──────────────┘    └──────────────┘
														  │
														  ▼
┌─────────────────────────────────────────────────────────────────┐
│                      OUTSAMPLES                                 │
│              Final output buffer filled with mixed audio        │
└─────────────────────────────────────────────────────────────────┘

Todo:
	- Performance is really bad, so we need to do a lot of things
		- Remove the need for mutexes (Lock-free!)
		- Dont allocate any memory
		- Dont do any file/network IO
		- Unroll loops (x4), but keep reference implementation
		- SIMD everything

	- Channel mapping

	- Do we need to deal with deinterleaved samples?
		Interleaved Samples         = LR|LR|LR|LR|LR|LR|LR

		Deinterleaved Left Samples  = L|L|L|L|L|L|L|L|L|L
		Deinterleaved Right Samples = R|R|R|R|R|R|R|R|R|R

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
	## 2026-08-07
	- Changed (API BREAKING): AudioSystemPlaySource() takes a fifth argument, `pitch`. Every existing call site must add it; pass 1.0f for the previous behaviour, which costs nothing (see AudioPitchNeutralTolerance). It is a parameter rather than a post-play setter because pitch is a property of THAT play - a setter would leave the first mixed buffer sounding at the wrong rate before it landed
	- New: AudioSystemSetPlayItemPitch() changes the pitch of a play item that is already playing (find-and-write under playItems.lock, by id, exactly like the volume setter) - so a pitch control can be dragged and HEARD while it moves, which is the only way to judge one on a sound effect that is over before the slider settles
	- New: AudioPlayItem.pitch - a varispeed playback rate (2 = an octave up and half as long, 0.5 = an octave down and twice as long). It needs no stage of its own: a rate conversion is decided by the RATIO alone, so ProcessSinglePlayItem simply tells the resampler the source has a different sample rate than it has, and the output still lands exactly on the device rate. A pitch within AudioPitchNeutralTolerance (0.1%) of 1 passes the source rate through UNCHANGED, so an unpitched clip keeps the memcpy passthrough and the even-ratio fast paths instead of falling onto the SinC branch over a slider's rounding error

	## 2026-08-06
	- Fixed: LoadMP3FormatFromBuffer (final_mp3loader.h) walked no headers at all -- it ran a FULL mp3dec_load_buf decode and read fileInfo.samples off the result, then freed it. It now hops frame header to frame header, which is what the format is actually made of. Measured over every mp3 in data/: identical frameCount to LoadMP3FromBuffer in all four, 174x-201x faster, no allocation (1.63 MB / 107 s track: 64.4 ms + 18 MB of PCM -> 0.37 ms + 0 bytes). AudioSystemLoadFileFormat is now cheap enough to ask per UI row, which is what a playlist needs to show a track's length
	- New: AudioSystemSetPlayItemVolume() changes the volume of a play item that is already playing (find-and-write under playItems.lock, by id so it cannot dangle)
	- New: AudioSystemRemoveSource() removes and frees ONE source, the counterpart to AudioSystemAddSource; play items referencing it are stopped first
	- Fixed: AudioSystemClearSources now resets sources.count to zero (it kept counting the sources it had just freed)
	- Fixed: AudioSystemClearSources now stops all play items first (clearing while something played left a play item pointing at freed samples)
	- New: AudioSystemGetTargetFormat() / AudioSystemGetSourceStats() report the device format and the resident sources + the PCM bytes they hold, so a diagnostic readout no longer has to reach into AudioSystem.targetFormat and walk sources itself
	- New: AudioSystemSetPlayItemPaused() pauses/resumes a play item that is already playing. The mixer skips a paused item without touching its framesPlayed, so resuming continues from the exact frame it stopped on - AudioSystemStopOne frees the item and forgets where it was, which is not the same thing
	- New: AudioSystemSetSuspended() / AudioSystemIsSuspended() pause the WHOLE mixer - AudioSystemWriteFrames returns silence and advances nothing, so everything resumes on the frame it stopped on (a pause, not a mute)

	## 2026-07-19
	- Fixed: AudioSystemStopOne now finds and removes the play-item entirely under playItems.lock (was traversing unlocked -> use-after-free race with the audio thread)
	- Fixed: Null-check probe/file-load allocations (PropeAudioFileFormat, AudioSystemLoadFileSource) so an out-of-memory condition bails cleanly instead of dereferencing null

	- Changed: ResampleChunk warning text updated — now references fplGetTargetAudioFrameCount as the authoritative formula; clamp kept defensively for arbitrary non-even ratios. 44100 <-> 48000 round-trip is exact.
*/

#ifndef FINAL_AUDIOSYSTEM_H
#define FINAL_AUDIOSYSTEM_H

#include <final_platform_layer.h>
#include <float.h>

#include "final_audio.h"
#include "final_audioconversion.h"

typedef struct AudioSourceID {
	uint64_t value;
} AudioSourceID;

typedef enum AudioSourceType {
	AudioSourceType_None = 0,
	AudioSourceType_Allocated,
	AudioSourceType_Stream,
	AudioSourceType_File,
} AudioSourceType;

typedef struct AudioSource {
	AudioBuffer buffer;
	AudioFormat format;
	AudioSourceType type;
	AudioSourceID id;
	struct AudioSource *next;
} AudioSource;

typedef struct AudioPlayItemID {
	uint64_t value;
} AudioPlayItemID;

typedef struct AudioPlayItem {
	AudioFrameIndex framesPlayed[2];	// 0 = Current, 1 = Saved
	fpl_b32 isFinished[2];				// 0 = Current, 1 = Saved
	const AudioSource *source;
	struct AudioPlayItem *next;
	struct AudioPlayItem *prev;
	AudioPlayItemID id;
	float volume;
	// Playback rate multiplier: 2 = an octave up and half as long, 0.5 = an octave down and twice as long.
	// VARISPEED, like changing a tape's speed -- pitch and duration move together, there is no time-stretch
	// anywhere in this mixer. Applied by telling the resampler the source has a different sample rate than it
	// really has, which is all a rate conversion ever needed: it is decided by the RATIO alone.
	// 1 (or anything within AudioPitchNeutralTolerance of it) is the file as recorded and costs nothing.
	float pitch;
	bool isRepeat;
	// Skipped by the mixer while set, WITHOUT touching framesPlayed - so resuming carries on from the exact frame it stopped on.
	// That is the whole difference between this and AudioSystemStopOne, which frees the item and forgets where it was
	bool isPaused;
} AudioPlayItem;

typedef struct AudioSources {
	volatile uint64_t idCounter;
	fplMutexHandle lock;
	AudioSource *first;
	AudioSource *last;
	size_t count;
} AudioSources;

typedef struct AudioPlayItems {
	fplMutexHandle lock;
	AudioPlayItem *first;
	AudioPlayItem *last;
	volatile uint64_t idCounter;
	size_t count;
} AudioPlayItems;

typedef struct AudioSineWaveData {
	AudioDuration duration;
	double toneVolume;
	AudioHertz frequency;
	AudioFrameIndex frameIndex;
} AudioSineWaveData;

typedef struct AudioMemory {
	int dummy;
} AudioMemory;

typedef struct AudioSystem {
	AudioStaticBuffer dspInBuffer;
	AudioStaticBuffer dspOutBuffer;
	AudioStaticBuffer mixingBuffer;
	AudioSampleConversionFunctions conversionFuncs;
	AudioStream conversionBuffer;
	AudioSineWaveData tempWaveData;
	AudioFormat targetFormat;
	AudioSources sources;
	AudioPlayItems playItems;
	AudioMemory memory;
	fplMutexHandle writeFramesLock;
	float masterVolume;
	bool isShutdown;
	// While set, AudioSystemWriteFrames hands the device SILENCE and advances nothing. Written from the
	// game thread, read on the audio thread, and deliberately a plain bool: the worst a torn read can do is
	// mix one more buffer, which is a few milliseconds of sound nobody was listening to anyway.
	bool isSuspended;
} AudioSystem;

fpl_extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioFormat *targetFormat);
fpl_extern void AudioSystemShutdown(AudioSystem *audioSys);

fpl_extern void AudioSystemSetMasterVolume(AudioSystem *audioSys, const float newMasterVolume);

fpl_extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const fplAudioFormatType type, const AudioFrameIndex frameCount);

fpl_extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath);
// Reads a file's FORMAT without decoding it -- headers only, in every supported format (wav: the header;
// vorbis: the seek table; mp3: a walk over the per-frame headers). A track's DURATION is this call:
// outFormat->frameCount / outFormat->samplesPerSecond.
fpl_extern bool AudioSystemLoadFileFormat(AudioSystem *audioSys, const char *filePath, PCMWaveFormat *outFormat);

fpl_extern AudioSource *AudioSystemLoadDataSource(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data);
fpl_extern bool AudioSystemLoadDataFormat(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data, PCMWaveFormat *outFormat);

fpl_extern bool AudioSystemAddSource(AudioSystem *audioSys, AudioSource *source);

// Remove ONE source and free it, the counterpart to AudioSystemAddSource. Any play item still referencing it is stopped first, so the caller never has to.
// Returns false when the source is not in the list (it is then left untouched).
fpl_extern bool AudioSystemRemoveSource(AudioSystem *audioSys, AudioSource *source);

fpl_extern AudioFrameIndex AudioSystemWriteFrames(AudioSystem *audioSys, void *outSamples, const fplAudioFormat *outFormat, const AudioFrameIndex frameCount, const bool advance);

// Suspend or resume the WHOLE mixer. While suspended the device is fed silence and no play item advances, so
// this is a PAUSE and not a mute: every position, every fade and the playlist all stand still, and what comes
// back is the exact frame that was playing when it stopped. What a game uses when its window loses focus --
// music playing on to an empty desktop is the thing this prevents.
fpl_extern void AudioSystemSetSuspended(AudioSystem *audioSys, const bool suspended);
fpl_extern bool AudioSystemIsSuspended(const AudioSystem *audioSys);

// How close to 1 a pitch has to be to count as UNPITCHED. Below this the source's own sample rate is passed to
// the resampler untouched, so a clip that matches the device still takes the memcpy passthrough and an even
// ratio still takes the cheap up/down path -- a slider quantizing to 0.9999999 must not silently push every
// sound in the game onto the SinC branch forever. 0.1% is about 0.017 semitones: inaudible by a wide margin.
#define AudioPitchNeutralTolerance 0.001f

// Play a source. `pitch` is the playback rate multiplier (see AudioPlayItem.pitch); pass 1 for the file as
// recorded, which costs nothing. Values <= 0 are treated as 1 rather than refused: silence would be the one
// outcome nobody could debug.
fpl_extern AudioPlayItemID AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume, const float pitch);
fpl_extern bool AudioSystemStopOne(AudioSystem *audioSys, const AudioPlayItemID playId);

// Change the volume of a play item that is ALREADY playing (fades, a volume slider). Takes an id and not a pointer on purpose:
// the audio thread frees a finished play item, so an id is the only handle that cannot dangle.
// Returns false when the item no longer exists, which is not an error - it just finished.
fpl_extern bool AudioSystemSetPlayItemVolume(AudioSystem *audioSys, const AudioPlayItemID playId, const float volume);

// Change the PITCH of a play item that is already playing, so a pitch control can be dragged and heard while
// it moves - which on a sound effect is the only way to judge one, most being over before a slider settles.
// Takes an id for the same reason the volume setter does. Values <= 0 are ignored rather than obeyed.
// The rate changes from the next mixed chunk; frames already written to the device keep the old one.
// Returns false when the item no longer exists, which is not an error - it just finished.
fpl_extern bool AudioSystemSetPlayItemPitch(AudioSystem *audioSys, const AudioPlayItemID playId, const float pitch);

// Pause or resume a play item that is already playing. The mixer skips it while paused and leaves its position alone, so resuming continues from the exact frame it stopped on -
// which is what separates this from AudioSystemStopOne, which frees the item and forgets where it was. Takes an id for the same reason the volume setter does.
// Returns false when the item no longer exists.
fpl_extern bool AudioSystemSetPlayItemPaused(AudioSystem *audioSys, const AudioPlayItemID playId, const bool paused);
fpl_extern void AudioSystemStopAll(AudioSystem *audioSys);
fpl_extern void AudioSystemClearSources(AudioSystem *audioSys);
fpl_extern size_t AudioSystemGetPlayItems(AudioSystem *audioSys, AudioPlayItem *dest, const size_t maxDestCount);

fpl_extern AudioSource *AudioSystemGetSourceByID(AudioSystem *audioSys, const AudioSourceID id);
fpl_extern size_t AudioSystemGetSources(AudioSystem *audioSys, AudioSource *dest, const size_t maxDestCount);

// The DEVICE format everything is mixed and converted to. Written once by AudioSystemInit and never again, so this needs no lock.
// For a diagnostic readout ("48000 Hz, 2 ch, S16"), which would otherwise have to reach into AudioSystem.targetFormat itself.
fpl_extern bool AudioSystemGetTargetFormat(const AudioSystem *audioSys, AudioFormat *outFormat);

// How many sources are resident, and (when outByteCount is given) how many bytes of decoded PCM they hold between them. Walks the list under sources.lock.
// The count on its own says nothing about the memory that actually matters - a fully decoded track is roughly 11x its size on disk.
fpl_extern size_t AudioSystemGetSourceStats(AudioSystem *audioSys, size_t *outByteCount);

// @TODO(final): Move to final_audiodemo.h, make a audio source more "Generative"
fpl_extern void AudioGenerateSineWave(AudioSineWaveData *waveData, void *outSamples, const fplAudioFormatType outFormat, const AudioHertz outSampleRate, const AudioChannelIndex channels, const AudioFrameIndex frameCount);

fpl_extern bool IsAudioSampleRateSupported(AudioSystem *audioSys, const AudioSampleIndex sampleRate);
#endif // FINAL_AUDIOSYSTEM_H

#if defined(FINAL_AUDIOSYSTEM_IMPLEMENTATION) && !defined(FINAL_AUDIOSYSTEM_IMPLEMENTED)
#define FINAL_AUDIOSYSTEM_IMPLEMENTED

#ifndef FINAL_WAVELOADER_IMPLEMENTATION
#	define FINAL_WAVELOADER_IMPLEMENTATION
#endif
#include "final_waveloader.h"

#ifndef FINAL_VORBISLOADER_IMPLEMENTATION
#	define FINAL_VORBISLOADER_IMPLEMENTATION
#endif
#include "final_vorbisloader.h"

#ifndef FINAL_MP3LOADER_IMPLEMENTATION
#	define FINAL_MP3LOADER_IMPLEMENTATION
#endif
#include "final_mp3loader.h"

#ifndef FINAL_AUDIO_CONVERSION_IMPLEMENTATION
#	define FINAL_AUDIO_CONVERSION_IMPLEMENTATION
#endif
#include "final_audioconversion.h"

#define FINAL_AUDIO_MAX_PROBE_BYTES_COUNT 128

static const float AudioPI32 = 3.14159265359f;

static void *AllocateAudioMemory(AudioMemory *audioSys, const size_t size) {
	// @TODO(final): Better memory management for audio system!
	void *result = fplMemoryAllocate(size);
	return(result);
}
static void FreeAudioMemory(AudioMemory *audioSys, void *ptr) {
	// @TODO(final): Better memory management for audio system!
	fplMemoryFree(ptr);
}

static void InitAudioBuffer(AudioBuffer *audioBuffer, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	fplClearStruct(audioBuffer);
	audioBuffer->frameCount = frameCount;
	audioBuffer->bufferSize = fplGetAudioBufferSizeInBytes(audioFormat->format, audioFormat->channels, frameCount);
	audioBuffer->isAllocated = false;
}

static bool AllocateAudioBuffer(AudioMemory *memory, AudioBuffer *audioBuffer, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	InitAudioBuffer(audioBuffer, audioFormat, frameCount);
	audioBuffer->samples = (uint8_t *)AllocateAudioMemory(memory, audioBuffer->bufferSize);
	audioBuffer->isAllocated = audioBuffer->samples != fpl_null;
	return(audioBuffer->isAllocated);
}

static void FreeAudioBuffer(AudioMemory *memory, AudioBuffer *audioBuffer) {
	if(audioBuffer->isAllocated) {
		if(audioBuffer->samples != fpl_null) {
			FreeAudioMemory(memory, audioBuffer->samples);
		}
		fplClearStruct(audioBuffer);
	}
}

static void AllocateAudioStream(AudioMemory *memory, AudioStream *audioStream, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	fplClearStruct(audioStream);
	AllocateAudioBuffer(memory, &audioStream->buffer, audioFormat, frameCount);
}
static void FreeAudioStream(AudioMemory *memory, AudioStream *audioStream) {
	FreeAudioBuffer(memory, &audioStream->buffer);
	fplClearStruct(audioStream);
}

fpl_extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioFormat *targetFormat) {
	if(audioSys == fpl_null) {
		return false;
	}
	if(targetFormat == fpl_null) {
		return false;
	}
	fplClearStruct(audioSys);

	audioSys->masterVolume = 1.0f;

	audioSys->targetFormat.channels = targetFormat->channels;
	audioSys->targetFormat.sampleRate = targetFormat->sampleRate;
	audioSys->targetFormat.format = targetFormat->type;

	if(!fplMutexInit(&audioSys->sources.lock)) {
		return false;
	}
	if(!fplMutexInit(&audioSys->playItems.lock)) {
		return false;
	}
	if(!fplMutexInit(&audioSys->writeFramesLock)) {
		return false;
	}

	AllocateAudioStream(&audioSys->memory, &audioSys->conversionBuffer, &audioSys->targetFormat, MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT);
	audioSys->mixingBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;
	audioSys->dspInBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;
	audioSys->dspOutBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;

	audioSys->tempWaveData.frequency = 440;
	audioSys->tempWaveData.toneVolume = 0.25;
	audioSys->tempWaveData.duration = 0.5;

	audioSys->conversionFuncs = CreateAudioSamplesConversionFunctions();

	return(true);
}

fpl_extern void AudioSystemSetMasterVolume(AudioSystem *audioSys, const float newMasterVolume) {
	audioSys->masterVolume = newMasterVolume;
}

fpl_extern void AudioSystemSetSuspended(AudioSystem *audioSys, const bool suspended) {
	if (audioSys == fpl_null) {
		return; // no device: there is nothing to suspend, and no caller should have to check
	}
	audioSys->isSuspended = suspended;
}

fpl_extern bool AudioSystemIsSuspended(const AudioSystem *audioSys) {
	if (audioSys == fpl_null) {
		return false;
	}
	return audioSys->isSuspended;
}

fpl_extern bool AudioSystemAddSource(AudioSystem *audioSys, AudioSource *source) {
	if (audioSys == fpl_null || source == fpl_null) {
		return false;
	}

	if (source->id.value == 0) {
		fplAssert(!"Source has no id");
		return false;
	}

	if (AudioSystemGetSourceByID(audioSys, source->id)) {
		fplAssert(!"Source already exists");
		return false;
	}

	fplMutexLock(&audioSys->sources.lock);
	source->next = fpl_null;
	if(audioSys->sources.last == fpl_null) {
		audioSys->sources.first = audioSys->sources.last = source;
	} else {
		audioSys->sources.last->next = source;
		audioSys->sources.last = source;
	}
	++audioSys->sources.count;
	fplMutexUnlock(&audioSys->sources.lock);

	return true;
}

fpl_extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const fplAudioFormatType type, const AudioFrameIndex frameCount) {
	// Compute audio buffer
	AudioFormat audioFormat = fplZeroInit;
	audioFormat.channels = channels;
	audioFormat.sampleRate = sampleRate;
	audioFormat.format = type;

	AudioBuffer audioBuffer = fplZeroInit;
	InitAudioBuffer(&audioBuffer, &audioFormat, frameCount);

	// Allocate one memory block for source struct, some padding and the sample data
	size_t memSize = sizeof(AudioSource) + sizeof(size_t) + audioBuffer.bufferSize;
	void *mem = AllocateAudioMemory(&audioSys->memory, memSize);
	if(mem == fpl_null) {
		return fpl_null;
	}

	// Fill out source
	AudioSource *result = (AudioSource *)mem;
	result->type = AudioSourceType_Allocated;
	result->id.value = fplAtomicIncrementU64(&audioSys->sources.idCounter);
	result->format = audioFormat;
	result->buffer = audioBuffer;
	result->buffer.samples = (uint8_t *)mem + sizeof(AudioSource) + sizeof(size_t);
	result->buffer.isAllocated = false;

	return(result);
}

static AudioFileFormat PropeAudioFileFormat(AudioSystemStream *stream) {
	if (stream == fpl_null || stream->size == 0) {
		return AudioFileFormat_None;
	}

	fplAssert(stream->read != fpl_null);
	fplAssert(stream->seek != fpl_null);

	size_t streamSize = stream->size;

	AudioFileFormat result = AudioFileFormat_None;

	size_t initialBufferSize = fplMin(FINAL_AUDIO_MAX_PROBE_BYTES_COUNT, streamSize);
	uint8_t *probeBuffer = (uint8_t *)fplMemoryAllocate(initialBufferSize);
	if (probeBuffer == fpl_null) {
		return AudioFileFormat_None;
	}

	size_t currentBufferSize = initialBufferSize;
	bool requiresMoreData[2] = { 0 };
	do {
		bool seekStart = true;
		if (requiresMoreData[1]) {
			seekStart = false;
		}

		requiresMoreData[0] = false;
		requiresMoreData[1] = false;

		if (seekStart) {
			stream->seek(stream, 0);
		} else {
			stream->seek(stream, stream->size - currentBufferSize);
		}

		if(stream->read(stream, currentBufferSize, probeBuffer, currentBufferSize) == currentBufferSize) {
			if(TestWaveHeader(probeBuffer, currentBufferSize)) {
				result = AudioFileFormat_Wave;
				break;
			}

			if(TestVorbisHeader(probeBuffer, currentBufferSize)) {
				result = AudioFileFormat_Vorbis;
				break;
			}

			size_t mp3NewSize = 0;
			MP3HeaderTestStatus mp3Status = TestMP3Header(probeBuffer, currentBufferSize, &mp3NewSize);
			if(mp3Status == MP3HeaderTestStatus_Success) {
				result = AudioFileFormat_MP3;
				break;
			} else if(mp3Status == MP3HeaderTestStatus_RequireMoreDataBegin || mp3Status == MP3HeaderTestStatus_RequireMoreDataEnd) {
				if(mp3NewSize > 0 && mp3NewSize <= streamSize && mp3NewSize > currentBufferSize) {
					fplMemoryFree(probeBuffer);
					currentBufferSize = fplMax(currentBufferSize, mp3NewSize);
					probeBuffer = (uint8_t *)fplMemoryAllocate(currentBufferSize);
					if (probeBuffer == fpl_null) {
						return AudioFileFormat_None;
					}
					if (mp3Status == MP3HeaderTestStatus_RequireMoreDataBegin) {
						requiresMoreData[0] = true;
					} else {
						fplAssert(mp3Status == MP3HeaderTestStatus_RequireMoreDataEnd);
						requiresMoreData[1] = true;
					}
				}
			}
		}
	} while(requiresMoreData[0] || requiresMoreData[1]);

	fplMemoryFree(probeBuffer);

	return(result);
}

static AudioSource *CreateAudioSourceFromPCM(AudioSystem *audioSys, const PCMWaveData *pcm, const AudioSourceType type) {
	fplAssert(audioSys != fpl_null && pcm != fpl_null);

	// Allocate one memory block for source struct, some padding and the sample data
	AudioSource *source = AudioSystemAllocateSource(audioSys, pcm->format.channelCount, pcm->format.samplesPerSecond, pcm->format.formatType, pcm->format.frameCount);
	if(source == fpl_null) {
		return fpl_null;
	}

	source->type = type;

	// Copy samples to source
	fplAssert(source->buffer.bufferSize >= pcm->samplesSize);
	fplMemoryCopy(pcm->isamples, pcm->samplesSize, source->buffer.samples);

	return(source);
}

fpl_extern AudioSource *AudioSystemLoadDataSource(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data) {
	if (audioSys == fpl_null || dataSize == 0 || data == fpl_null) {
		return fpl_null;
	}

	AudioSystemStream stream = AudioStreamCreateFromData(dataSize, data);

	AudioFileFormat fileFormat = PropeAudioFileFormat(&stream);
	if(fileFormat == AudioFileFormat_None) {
		return fpl_null;
	}

	PCMWaveData loadedData = fplZeroInit;
	switch(fileFormat) {
		case AudioFileFormat_Wave:
		{
			if(!LoadWaveFromBuffer(data, dataSize, &loadedData)) {
				return fpl_null;
			}
		} break;

		case AudioFileFormat_Vorbis:
		{
			if(!LoadVorbisFromBuffer(data, dataSize, &loadedData)) {
				return fpl_null;
			}
		} break;

		case AudioFileFormat_MP3:
		{
			if(!LoadMP3FromBuffer(data, dataSize, &loadedData)) {
				return fpl_null;
			}
		} break;

		default:
			return fpl_null;
	}

	AudioSource *source = CreateAudioSourceFromPCM(audioSys, &loadedData, AudioSourceType_File);
	FreeWave(&loadedData);
	return(source);
}

static bool AudioSystem__LoadWaveFormat(AudioFileFormat fileFormat, const uint8_t *data, const size_t size, PCMWaveFormat *outFormat) {
	fplClearStruct(outFormat);
	switch(fileFormat) {
		case AudioFileFormat_Wave:
			return LoadWaveFormatFromBuffer(data, size, outFormat);
		case AudioFileFormat_Vorbis:
			return LoadVorbisFormatFromBuffer(data, size, outFormat);
		case AudioFileFormat_MP3:
			return LoadMP3FormatFromBuffer(data, size, outFormat);
		default:
			return false;
	}
}

fpl_extern bool AudioSystemLoadDataFormat(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data, PCMWaveFormat *outFormat) {
	if (audioSys == fpl_null || dataSize == 0 || data == fpl_null || outFormat == fpl_null) {
		return false;
	}

	AudioSystemStream stream = AudioStreamCreateFromData(dataSize, data);

	AudioFileFormat fileFormat = PropeAudioFileFormat(&stream);
	if(fileFormat == AudioFileFormat_None) {
		return false;
	}

	if (!AudioSystem__LoadWaveFormat(fileFormat, data, dataSize, outFormat)) {
		return false;
	}

	return true;
}

fpl_extern bool AudioSystemLoadFileFormat(AudioSystem *audioSys, const char *filePath, PCMWaveFormat *outFormat) {
	if (audioSys == fpl_null || fplGetStringLength(filePath) == 0 || outFormat == fpl_null) {
		return false;
	}

	fplFileHandle file;
	if (!fplFileOpenBinary(filePath, &file)) {
		return false;
	}

	bool result = false;

	uint8_t *data = fpl_null;

	size_t fileSize = 0;
	size_t seek = 0;
	size_t read = 0;
	AudioFileFormat fileFormat = AudioFileFormat_None;
	AudioSystemStream stream = fplZeroInit;

	fileSize = fplFileGetSizeFromHandle32(&file);
	if (fileSize == 0) {
		goto done;
	}

	stream = AudioStreamCreateFromFileHandle(&file, fileSize);

	fileFormat = PropeAudioFileFormat(&stream);
	if(fileFormat == AudioFileFormat_None) {
		goto done;
	}

	data = (uint8_t *)fplMemoryAllocate(fileSize);
	if (data == fpl_null) {
		goto done;
	}

	seek = AudioSystemStreamSeek(&stream, 0);
	if (seek != 0) {
		goto done;
	}

	read = AudioSystemStreamRead(&stream, fileSize, data, fileSize);
	if (read != fileSize) {
		goto done;
	}

	if (!AudioSystem__LoadWaveFormat(fileFormat, data, fileSize, outFormat)) {
		goto done;
	}

	result = true;

done:
	if (data != fpl_null) {
		fplMemoryFree(data);
	}

	fplFileClose(&file);

	return result;
}

fpl_extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath) {
	if (audioSys == fpl_null || fplGetStringLength(filePath) == 0) {
		return fpl_null;
	}

	fplFileHandle file;
	if (!fplFileOpenBinary(filePath, &file)) {
		return fpl_null;
	}

	size_t fileSize = fplFileGetSizeFromHandle32(&file);
	if (fileSize == 0) {
		fplFileClose(&file);
		return fpl_null;
	}

	AudioSystemStream stream = AudioStreamCreateFromFileHandle(&file, fileSize);

	AudioFileFormat fileFormat = PropeAudioFileFormat(&stream);
	if(fileFormat == AudioFileFormat_None) {
		fplFileClose(&file);
		return fpl_null;
	}

	uint8_t *buffer = (uint8_t *)fplMemoryAllocate(fileSize);
	if (buffer == fpl_null) {
		fplFileClose(&file);
		return fpl_null;
	}

	AudioSystemStreamSeek(&stream, 0);

	size_t read = AudioSystemStreamRead(&stream, fileSize, buffer, fileSize);
	if (read != fileSize) {
		fplMemoryFree(buffer);
		fplFileClose(&file);
		return fpl_null;
	}

	fplFileClose(&file);

	PCMWaveData loadedData = fplZeroInit;
	switch(fileFormat) {
		case AudioFileFormat_Wave:
		{
			if(!LoadWaveFromBuffer(buffer, fileSize, &loadedData)) {
				fplMemoryFree(buffer);
				return fpl_null;
			}
		} break;

		case AudioFileFormat_Vorbis:
		{
			if(!LoadVorbisFromBuffer(buffer, fileSize, &loadedData)) {
				fplMemoryFree(buffer);
				return fpl_null;
			}
		} break;

		case AudioFileFormat_MP3:
		{
			if(!LoadMP3FromBuffer(buffer, fileSize, &loadedData)) {
				fplMemoryFree(buffer);
				return fpl_null;
			}
		} break;

		default:
		{
			fplMemoryFree(buffer);
			return fpl_null;
		} break;
	}

	fplMemoryFree(buffer);

	AudioSource *source = CreateAudioSourceFromPCM(audioSys, &loadedData, AudioSourceType_File);
	FreeWave(&loadedData);
	return(source);
}

static void RemovePlayItem(AudioMemory *memory, AudioPlayItems *playItems, AudioPlayItem *playItem) {
	if(playItems->first == playItems->last) {
		// Remove single item
		playItems->first = playItems->last = fpl_null;
	} else 	if(playItem == playItems->last) {
		// Remove at end
		if(playItems->first == playItems->last) {
			playItems->first = playItems->last = fpl_null;
		} else {
			playItems->last = playItem->prev;
			playItems->last->next = fpl_null;
		}
	} else if(playItem == playItems->first) {
		// Remove at start
		if(playItems->first == playItems->last) {
			playItems->first = playItems->last = fpl_null;
		} else {
			playItems->first = playItem->next;
			playItems->first->prev = fpl_null;
		}
	} else {
		// Remove in the middle
		AudioPlayItem *cur = playItems->first;
		while(cur != playItem) {
			cur = cur->next;
		}
		cur->prev->next = cur->next;
		if(cur->next != fpl_null) {
			cur->next->prev = cur->prev;
		}
	}
	FreeAudioMemory(memory, playItem);
	--playItems->count;
}

fpl_extern AudioSource *AudioSystemGetSourceByID(AudioSystem *audioSys, const AudioSourceID id) {
	fplMutexLock(&audioSys->sources.lock);
	AudioSource *src = audioSys->sources.first;
	AudioSource *result = fpl_null;
	while(src != fpl_null) {
		if(src->id.value == id.value) {
			result = src;
			break;
		}
		src = src->next;
	}
	fplMutexUnlock(&audioSys->sources.lock);
	return(result);
}

fpl_extern size_t AudioSystemGetSources(AudioSystem *audioSys, AudioSource *dest, const size_t maxDestCount) {
	size_t count = audioSys->sources.count;
	if(dest != fpl_null) {
		if(count > maxDestCount) {
			return(0); // Error, destination array to small
		}
		fplMutexLock(&audioSys->sources.lock);
		const AudioSource *src = audioSys->sources.first;
		AudioSource *dst = dest;
		while(src != fpl_null) {
			*dst = *src;
			src = src->next;
			++dst;
		}
		fplMutexUnlock(&audioSys->sources.lock);
	}
	return(count);
}

fpl_extern size_t AudioSystemGetPlayItems(AudioSystem *audioSys, AudioPlayItem *dest, const size_t maxDestCount) {
	size_t count = audioSys->playItems.count;
	if(dest != fpl_null) {
		if(count > maxDestCount) {
			return(0); // Error, destination array to small
		}
		fplMutexLock(&audioSys->playItems.lock);
		const AudioPlayItem *src = audioSys->playItems.first;
		AudioPlayItem *dst = dest;
		while(src != fpl_null) {
			*dst = *src;
			src = src->next;
			++dst;
		}
		fplMutexUnlock(&audioSys->playItems.lock);
	}
	return(count);
}

fpl_extern bool AudioSystemStopOne(AudioSystem *audioSys, const AudioPlayItemID playId) {
	// The audio device callback can RemovePlayItem concurrently, so the whole find-and-remove must run under the lock
	bool result = false;
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		if(playItem->id.value == playId.value) {
			RemovePlayItem(&audioSys->memory, &audioSys->playItems, playItem);
			result = true;
			break;
		}
		playItem = playItem->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemVolume(AudioSystem *audioSys, const AudioPlayItemID playId, const float volume) {
	if(audioSys == fpl_null) {
		return false;
	}
	// The audio device callback reads item->volume under this same lock (ProcessSinglePlayItem) and can RemovePlayItem
	// concurrently, so the whole find-and-write must run under it
	bool result = false;
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		if(playItem->id.value == playId.value) {
			playItem->volume = volume;
			result = true;
			break;
		}
		playItem = playItem->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemPitch(AudioSystem *audioSys, const AudioPlayItemID playId, const float pitch) {
	if(audioSys == fpl_null || pitch <= 0.0f) {
		return false;
	}
	// Same find-and-write under the same lock as the volume setter, and for the same reason: the audio device
	// callback reads item->pitch (ProcessSinglePlayItem) and can RemovePlayItem concurrently
	bool result = false;
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		if(playItem->id.value == playId.value) {
			playItem->pitch = pitch;
			result = true;
			break;
		}
		playItem = playItem->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemPaused(AudioSystem *audioSys, const AudioPlayItemID playId, const bool paused) {
	if(audioSys == fpl_null) {
		return false;
	}
	// Same reasoning as the volume setter: the device callback reads this flag under this lock and can free the item concurrently
	bool result = false;
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		if(playItem->id.value == playId.value) {
			playItem->isPaused = paused;
			result = true;
			break;
		}
		playItem = playItem->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
	return(result);
}

fpl_extern AudioPlayItemID AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume, const float pitch) {
	if((audioSys == fpl_null) || (source == fpl_null)) {
		AudioPlayItemID empty = fplZeroInit;
		return(empty);
	}

	AudioPlayItem *playItem = (AudioPlayItem *)AllocateAudioMemory(&audioSys->memory, sizeof(AudioPlayItem));
	if(playItem == fpl_null) {
		AudioPlayItemID empty = fplZeroInit;
		return(empty);
	}

	playItem->id.value = fplAtomicIncrementU64(&audioSys->playItems.idCounter);
	playItem->next = playItem->prev = fpl_null;
	playItem->framesPlayed[0] = playItem->framesPlayed[1] = 0;
	playItem->isFinished[0] = playItem->isFinished[1] = false;
	playItem->source = source;
	playItem->isRepeat = repeat;
	playItem->volume = volume;
	// A pitch of zero or below would consume no input and sound like nothing at all -- the one failure nobody
	// could debug from the outside. Treat it as the file as recorded.
	playItem->pitch = (pitch > 0.0f) ? pitch : 1.0f;
	playItem->isPaused = false;

	fplMutexLock(&audioSys->playItems.lock);
	if(audioSys->playItems.last == fpl_null) {
		audioSys->playItems.first = audioSys->playItems.last = playItem;
	} else {
		playItem->prev = audioSys->playItems.last;
		audioSys->playItems.last->next = playItem;
		audioSys->playItems.last = playItem;
	}
	++audioSys->playItems.count;
	fplMutexUnlock(&audioSys->playItems.lock);

	return(playItem->id);
}

fpl_force_inline float AudioClipF32(const float value) {
	float result = fplMax(-1.0f, fplMin(value, 1.0f));
	return(result);
}

fpl_extern float ConvertToF32(const void *inSamples, const AudioChannelIndex inChannel, const fplAudioFormatType inFormat) {
	// @TODO(final): Convert from other audio formats to F32
	switch(inFormat) {
		case fplAudioFormatType_S16:
		{
			int16_t sampleValue = *((const int16_t *)inSamples + inChannel);
			return sampleValue / (float)INT16_MAX;
		} break;

		case fplAudioFormatType_S32:
		{
			int32_t sampleValue = *((const int32_t *)inSamples + inChannel);
			return sampleValue / (float)INT32_MAX;
		} break;

		case fplAudioFormatType_F32:
		{
			float sampleValueF32 = *((const float *)inSamples + inChannel);
			return(sampleValueF32);
		} break;

		default:
			return 0.0;
	}
}

fpl_extern void ConvertFromF32(void *outSamples, const float inSampleValue, const AudioChannelIndex outChannel, const fplAudioFormatType outFormat) {
	// @TODO(final): Convert to other audio formats
	float x = AudioClipF32(inSampleValue);
	switch(outFormat) {
		case fplAudioFormatType_S16:
		{
			int16_t *sampleValuePtr = (int16_t *)outSamples + outChannel;
			*sampleValuePtr = (int16_t)(x * INT16_MAX);
		} break;

		case fplAudioFormatType_S32:
		{
			int32_t *sampleValue = (int32_t *)outSamples + outChannel;
			*sampleValue = (int32_t)(x * INT32_MAX);
		} break;

		case fplAudioFormatType_F32:
		{
			float *sampleValue = (float *)outSamples + outChannel;
			*sampleValue = x;
		} break;

		default:
			break;
	}
}

fpl_extern void AudioGenerateSineWave(AudioSineWaveData *waveData, void *outSamples, const fplAudioFormatType outFormat, const AudioHertz outSampleRate, const AudioChannelIndex channels, const AudioFrameIndex frameCount) {
	uint8_t *samples = (uint8_t *)outSamples;
	size_t sampleStride = (size_t)fplGetAudioSampleSizeInBytes(outFormat) * channels;
	for(AudioFrameIndex i = 0; i < frameCount; ++i) {
		AudioFrameIndex f = i + waveData->frameIndex;
		double t = sin((2.0 * M_PI * waveData->frequency) / outSampleRate * f);
		float sampleValue = (float)(t * waveData->toneVolume);
		for(AudioChannelIndex channelIndex = 0; channelIndex < channels; ++channelIndex) {
			ConvertFromF32(samples, sampleValue, channelIndex, outFormat);
		}
		samples += sampleStride;
	}
	waveData->frameIndex += frameCount;
}

static AudioResampleResult AudioSimpleUpSampling(const AudioChannelIndex inChannelCount, const AudioHertz inSampleRate, const AudioHertz outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float *inSamples, float *outSamples) {
	// Simple Upsampling (2x, 4x, 6x, 8x etc.) -> Duplicate frames
	fplAssert(outSampleRate > inSampleRate);
	fplAssert((outSampleRate % inSampleRate) == 0);
	const uint32_t upsamplingFactor = outSampleRate / inSampleRate;
	const AudioFrameIndex inFrameCount = fplMin(minOutputFrameCount / upsamplingFactor, maxInputFrameCount);
	const float *inSamplesF32 = (const float *)inSamples;
	const size_t inSampleStride = inChannelCount;
	AudioResampleResult result = fplZeroInit;
	for(AudioFrameIndex i = 0; i < inFrameCount; ++i) {
		for(uint32_t f = 0; f < upsamplingFactor; ++f) {
			for(AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
				*outSamples++ = inSamplesF32[inChannelIndex];
			}
			++result.outputCount;
		}
		inSamplesF32 += inSampleStride;
		++result.inputCount;
	}
	return(result);
}

static AudioResampleResult AudioSimpleDownSampling(const AudioChannelIndex inChannelCount, const AudioHertz inSampleRate, const AudioHertz outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float *inSamples, float *outSamples) {
	// Simple Downsampling (1/2, 1/4, 1/6, 1/8, etc.) -> Skipping frames
	fplAssert(inSampleRate > outSampleRate);
	fplAssert((inSampleRate % outSampleRate) == 0);
	const uint32_t downsamplingFactor = inSampleRate / outSampleRate;
	const AudioFrameIndex inFrameCount = fplMin(minOutputFrameCount * downsamplingFactor, maxInputFrameCount);
	AudioResampleResult result = fplZeroInit;
	for(AudioFrameIndex i = 0; i < inFrameCount; i += downsamplingFactor) {
		for(AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
			AudioFrameIndex sourceIndex = i * inChannelCount + inChannelIndex;
			*outSamples++ = inSamples[sourceIndex];
		}
		result.inputCount += downsamplingFactor;
		result.outputCount++;
	}
	return(result);
}

static void ApplyVolumeToSamples(const AudioChannelIndex inChannelCount, const AudioFrameIndex frameCount, const float volume, float *samples) {
	AudioSampleIndex sampleCount = frameCount * inChannelCount;
	for (AudioSampleIndex sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		samples[sampleIndex] *= volume;
	}
}

static void SavePlayStates(AudioSystem *audioSys) {
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *item = audioSys->playItems.first;
	while(item != fpl_null) {
		item->framesPlayed[1] = item->framesPlayed[0];
		item->isFinished[1] = item->isFinished[0];
		item = item->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
}

static void RestorePlayStates(AudioSystem *audioSys) {
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *item = audioSys->playItems.first;
	while(item != fpl_null) {
		item->framesPlayed[0] = item->framesPlayed[1];
		item->isFinished[0] = item->isFinished[1];
		item = item->next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);
}

// ============================================================================
// Play Items Mixing Pipeline
// ============================================================================
//
// Pipeline per play item (repeated in chunks until targetFrameCount is filled):
//
//   Source PCM (any format)
//       |
//       v
//   ConvertSourceChunkToF32()     -- bulk conversion via dispatch table
//       |                            supports U8, S16, S24, S32, F32
//       v
//   ResampleChunk()               -- passthrough / even up-down / SinC
//       |                            output clamped to buffer capacity
//       v
//   ApplyVolumeToSamples()        -- flat loop: sample *= volume
//       |
//       v
//   MixSamplesIntoBuffer()        -- accumulate (+= ) into mixing buffer
//                                    handles channel up/down mixing
//
// ============================================================================

/**
* @brief Accumulates interleaved F32 samples from inSamples into outSamples (+=).
*
* @details Handles channel count differences:
*   - Same count:   flat bulk add over frameCount * channels samples.
*   - Mono in:      broadcast the mono sample to every output channel.
*   - Mono out:     average all input channels into the single output channel.
*   - Stereo in, N out (N >= 2): add L/R to first two output channels,
*     add a mono downmix (0.5 * (L+R)) to remaining channels.
*   - General case: average all input channels, add to every output channel.
*
* @param[in] frameCount  Number of audio frames to mix.
* @param[in] inChannels  Channel count of inSamples (samples per frame).
* @param[in] outChannels Channel count of outSamples (samples per frame).
* @param[in] inSamples   Source interleaved F32 samples (frameCount * inChannels elements).
* @param[in] outSamples  Destination interleaved F32 samples (frameCount * outChannels elements).
*                        Existing values are preserved and added to (+=).
*
* @returns Total number of output samples touched (frameCount * outChannels).
*
* @note Buffer safety: Caller must ensure both buffers are large enough.
*       No internal allocation.
*/
static AudioSampleIndex MixSamplesIntoBuffer(const AudioFrameIndex frameCount, const AudioChannelIndex inChannels, const AudioChannelIndex outChannels, const float *inSamples, float *outSamples) {
	if (frameCount == 0 || inChannels == 0 || outChannels == 0 || inSamples == fpl_null || outSamples == fpl_null) {
		return 0;
	}

	const AudioSampleIndex outSampleCount = (AudioSampleIndex)frameCount * outChannels;

	if (inChannels == outChannels) {
		// Bulk add — single flat loop, no per-frame/per-channel indexing needed
		const AudioSampleIndex totalSamples = (AudioSampleIndex)frameCount * outChannels;
		for (AudioSampleIndex i = 0; i < totalSamples; ++i) {
			outSamples[i] += inSamples[i];
		}
	} else if (inChannels == 1) {
		// Mono → N channels: broadcast each mono sample to all output channels
		for (AudioFrameIndex f = 0; f < frameCount; ++f) {
			const float sample = inSamples[f];
			float *outFrame = outSamples + f * outChannels;
			for (AudioChannelIndex ch = 0; ch < outChannels; ++ch) {
				outFrame[ch] += sample;
			}
		}
	} else if (outChannels == 1) {
		// N channels → mono: average all input channels per frame
		const float invInChannels = 1.0f / (float)inChannels;
		for (AudioFrameIndex f = 0; f < frameCount; ++f) {
			const float *inFrame = inSamples + f * inChannels;
			float sum = 0.0f;
			for (AudioChannelIndex ch = 0; ch < inChannels; ++ch) {
				sum += inFrame[ch];
			}
			outSamples[f] += sum * invInChannels;
		}
	} else if (inChannels == 2 && outChannels >= 2) {
		// Stereo → N channels (N >= 2): L/R to first two, mono downmix to rest
		for (AudioFrameIndex f = 0; f < frameCount; ++f) {
			const float *inFrame = inSamples + f * inChannels;
			float *outFrame = outSamples + f * outChannels;
			const float left = inFrame[0];
			const float right = inFrame[1];
			outFrame[0] += left;   // += not = (fixes stereo overwrite bug)
			outFrame[1] += right;  // += not = (fixes stereo overwrite bug)
			const float mono = 0.5f * (left + right);
			for (AudioChannelIndex ch = 2; ch < outChannels; ++ch) {
				outFrame[ch] += mono;
			}
		}
	} else {
		// General case: average all input channels, add to every output channel
		const float invInChannels = 1.0f / (float)inChannels;
		for (AudioFrameIndex f = 0; f < frameCount; ++f) {
			const float *inFrame = inSamples + f * inChannels;
			float *outFrame = outSamples + f * outChannels;
			float sum = 0.0f;
			for (AudioChannelIndex ch = 0; ch < inChannels; ++ch) {
				sum += inFrame[ch];
			}
			const float mono = sum * invInChannels;
			for (AudioChannelIndex ch = 0; ch < outChannels; ++ch) {
				outFrame[ch] += mono;
			}
		}
	}

	return outSampleCount;
}

/**
* @brief Converts a chunk of source PCM samples to interleaved F32 in the DSP-in buffer.
*
* @details Uses the full AudioSamplesConvert dispatch table.
*          Supports the format types: U8, S16, S24, S32, F32.
*          Clamps the number of frames to the buffer capacity.
*
* @param convFuncs          Format conversion dispatch table.
* @param sourceData         Pointer to the first byte of source samples to convert.
* @param sourceFrameCount   Number of source frames available starting at sourceData.
* @param sourceChannels     Channel count of the source audio.
* @param sourceFormat       Sample format of the source audio (U8, S16, S24, S32, F32).
* @param dspInBuffer        Destination buffer for interleaved F32 output.
* @param dspInMaxFrameCount Maximum frames the destination buffer can hold.
*
* @return Returns the number of frames actually converted (always <= min(sourceFrameCount, dspInMaxFrameCount)).
*
* @note Buffer safety:
*       Output is clamped to dspInMaxFrameCount. Caller must ensure sourceData has at least sourceFrameCount * sourceChannels * bytesPerSample bytes available.
*/
static AudioFrameIndex ConvertSourceChunkToF32(AudioSampleConversionFunctions *convFuncs, const uint8_t *sourceData, const AudioFrameIndex sourceFrameCount, const AudioChannelIndex sourceChannels, const fplAudioFormatType sourceFormat, float *dspInBuffer, const AudioFrameIndex dspInMaxFrameCount) {
	const AudioFrameIndex framesToConvert = fplMin(sourceFrameCount, dspInMaxFrameCount);
	if (framesToConvert == 0) {
		return 0;
	}
	const AudioSampleIndex sampleCount = (AudioSampleIndex)framesToConvert * sourceChannels;
	bool ok = AudioSamplesConvert(convFuncs, sampleCount, sourceFormat, fplAudioFormatType_F32, sourceData, dspInBuffer);
	fplAssert(ok);
	(void)ok;
	return framesToConvert;
}

/**
* @brief Resamples interleaved F32 audio from one sample rate to another.
*
* @details Three paths:
*   1. Same rate:      Memory copy passthrough — no processing.
*   2. Even ratio:     Frame duplication (up) or frame skipping (down).
*                      Only used for exact integer multiples (2x, 4x, etc.).
*   3. Non-even ratio: SinC interpolation via AudioResampleInterleaved.
*                      Handles arbitrary ratios (e.g. 44100 <-> 48000).
*
* @param channels        Number of interleaved channels.
* @param inRate          Input sample rate in Hz.
* @param outRate         Output sample rate in Hz.
* @param maxOutputFrames Maximum number of output frames the dspOut buffer can hold.
*                        Also limits how many frames we request from the resampler.
* @param inputFrameCount Number of input frames available in dspIn.
* @param dspIn           Source interleaved F32 samples.
* @param dspOut          Destination interleaved F32 samples.
*
* @return Returns an AudioResampleResult with .inputCount (frames consumed) and .outputCount (frames produced).
*         Both are zero if rates are invalid or no frames could be produced.
*
* @note Buffer safety:
*       For upsampling, input frames are pre-clamped so that (input * factor) <= maxOutputFrames.
*       For downsampling and SinC, maxOutputFrames is passed as the output limit.
*       Passthrough is clamped to min(inputFrameCount, maxOutputFrames).
*
* @warning AudioResampleInterleaved derives its outputCount via fplGetTargetAudioFrameCount(inFrameCount, inRate, outRate).
*          For 44100 <-> 48000 the round-trip converges exactly, but for arbitrary non-even ratios the result may still exceed minOutputFrameCount by 1 frame.
*          Callers MUST clamp the returned outputCount before using it to index into fixed-size buffers or subtract from unsigned frame counters.
*/
static AudioResampleResult ResampleChunk(const AudioChannelIndex channels, const AudioHertz inRate, const AudioHertz outRate, const AudioFrameIndex maxOutputFrames, const AudioFrameIndex inputFrameCount, const float *dspIn, float *dspOut) {
	AudioResampleResult result = fplZeroInit;

	if (channels == 0 || inRate == 0 || outRate == 0 || inputFrameCount == 0 || maxOutputFrames == 0) {
		return result;
	}

	if (inRate == outRate) {
		// Passthrough — same sample rate, just copy
		const AudioFrameIndex framesToCopy = fplMin(inputFrameCount, maxOutputFrames);
		const size_t bytesToCopy = (size_t)framesToCopy * channels * sizeof(float);
		fplMemoryCopy(dspIn, bytesToCopy, dspOut);
		result.inputCount = framesToCopy;
		result.outputCount = framesToCopy;
	} else {
		const bool isEven = (outRate > inRate) ? ((outRate % inRate) == 0) : ((inRate % outRate) == 0);

		if (isEven) {
			if (outRate > inRate) {
				// Even upsampling (2x, 4x, etc.)
				// Clamp input so that (input * factor) does not exceed maxOutputFrames
				result = AudioSimpleUpSampling(channels, inRate, outRate, maxOutputFrames, inputFrameCount, dspIn, dspOut);
			} else {
				// Even downsampling (1/2, 1/4, etc.)
				result = AudioSimpleDownSampling(channels, inRate, outRate, maxOutputFrames, inputFrameCount, dspIn, dspOut);
			}
		} else {
			// Non-even ratio — SinC interpolation (e.g. 44100 <-> 48000)
			result = AudioResampleInterleaved(channels, inRate, outRate, maxOutputFrames, inputFrameCount, dspIn, dspOut);
		}
	}

	return result;
}

/**
* @brief Processes a single AudioPlayItem through the full mixing pipeline.
*
* @details For one play item, this function loops in chunks (bounded by DSP buffer sizes) until
* either targetFrameCount output frames have been produced or the source is exhausted.
*
* Each chunk iteration:
*   1. Convert source PCM → interleaved F32            (ConvertSourceChunkToF32)
*   2. Resample to output sample rate                  (ResampleChunk)
*   3. Apply per-item and master volume                (ApplyVolumeToSamples)
*   4. Accumulate into the mixing buffer               (MixSamplesIntoBuffer)
*
* DSP scratch buffers (dspInBuffer, dspOutBuffer) are used from the start of the buffer
* each iteration — they are temporary workspace, never accumulated across iterations.
*
* The function updates item->framesPlayed[0] and item->isFinished[0]. When the item
* reaches the end of its source and isRepeat is true, framesPlayed resets to zero so
* the loop continues.
*
* @param audioSys         Audio system (provides target format, DSP buffers, conversion table).
* @param item             Play item to process. Modified in place (framesPlayed, isFinished).
* @param targetFrameCount Number of output frames to fill in the mixing buffer.
* @param mixingBuffer     Pointer to the start of the F32 mixing buffer for this item.
*                         Must have at least targetFrameCount * outChannels floats.
*
* @return Number of output frames actually produced and mixed into the buffer.
*
* @warning Buffer safety:
*   - dspInBuffer:  ConvertSourceChunkToF32 clamps to dspInBuffer.maxFrameCount.
*   - dspOutBuffer: ResampleChunk clamps output to min(outRemainingFrameCount, dspOutBuffer.maxFrameCount).
*   - mixingBuffer: outRemainingFrameCount counts down from targetFrameCount, preventing overflow.
*   - Source data:  Bounded by inTotalFrameCount - framesPlayed[0].
*/
// The sample rate the resampler is TOLD the source has, which is the only thing pitch changes. A rate
// conversion is decided by the RATIO alone -- claiming a 44100 Hz source is 47187 Hz while the device stays at
// 48000 consumes the clip 7% faster and still lands exactly on the device rate, so pitch needs no stage of its
// own and the output rate never moves.
//
// A neutral pitch returns the source rate UNCHANGED (not merely something close to it), which is what keeps an
// unpitched clip on the memcpy passthrough or the even-ratio path instead of the SinC branch.
static AudioHertz ResolvePitchedSampleRate(const AudioHertz sourceSampleRate, const float pitch) {
	if (pitch <= 0.0f) {
		return sourceSampleRate;
	}
	float distanceFromNeutral = pitch - 1.0f;
	if (distanceFromNeutral < 0.0f) {
		distanceFromNeutral = -distanceFromNeutral;
	}
	if (distanceFromNeutral <= AudioPitchNeutralTolerance) {
		return sourceSampleRate;
	}
	float pitchedRate = (float)sourceSampleRate * pitch + 0.5f;
	if (pitchedRate < 1.0f) {
		return sourceSampleRate; // an absurd pitch would round the rate to zero, which the resampler refuses
	}
	return (AudioHertz)pitchedRate;
}

static AudioFrameIndex ProcessSinglePlayItem(AudioSystem *audioSys, AudioPlayItem *item, const AudioFrameIndex targetFrameCount, float *mixingBuffer) {
	const AudioHertz outSampleRate = audioSys->targetFormat.sampleRate;
	const AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;

	const AudioSource *source = item->source;
	const AudioFormat *srcFormat = &source->format;
	const AudioBuffer *srcBuffer = &source->buffer;

	// The PITCHED rate, which is the source's own whenever the item is not pitched -- see ResolvePitchedSampleRate.
	const AudioHertz inSampleRate = ResolvePitchedSampleRate(srcFormat->sampleRate, item->pitch);
	const AudioFrameIndex inTotalFrameCount = srcBuffer->frameCount;
	const AudioChannelIndex inChannelCount = srcFormat->channels;
	const fplAudioFormatType inFormat = srcFormat->format;
	const size_t inBytesPerSample = fplGetAudioSampleSizeInBytes(inFormat);
	const size_t inBytesPerFrame = (size_t)inChannelCount * inBytesPerSample;

	const float volume = item->volume * audioSys->masterVolume;

	// Maximum frames the DSP output buffer can hold (for the source channel count)
	const AudioFrameIndex dspOutMaxFrames = audioSys->dspOutBuffer.maxFrameCount;

	AudioFrameIndex outRemainingFrameCount = targetFrameCount;
	AudioFrameIndex totalOutputFrameCount = 0;

	while (outRemainingFrameCount > 0) {
		// If this item finished on a previous chunk (e.g. repeat looped and then finished), stop
		if (item->isFinished[0]) {
			break;
		}

		const AudioFrameIndex inStartFrameIndex = item->framesPlayed[0];
		fplAssert(inStartFrameIndex < inTotalFrameCount);

		const AudioFrameIndex inRemainingFrameCount = inTotalFrameCount - inStartFrameIndex;
		const uint8_t *inSourceData = srcBuffer->samples + (size_t)inStartFrameIndex * inBytesPerFrame;

		// ---- Step 1: Convert source samples to interleaved F32 (DSP-In) ----
		// Always write to the start of the DSP-in buffer (scratch space, reused each iteration)
		float *dspIn = (float *)audioSys->dspInBuffer.samples;
		const AudioFrameIndex convertedFrameCount = ConvertSourceChunkToF32(
			&audioSys->conversionFuncs,
			inSourceData,
			inRemainingFrameCount,
			inChannelCount,
			inFormat,
			dspIn,
			audioSys->dspInBuffer.maxFrameCount
		);

		if (convertedFrameCount == 0) {
			break;
		}

		// ---- Step 2: Resample to output sample rate (DSP-Out) ----
		// Always write to the start of the DSP-out buffer (scratch space, reused each iteration)
		float *dspOut = (float *)audioSys->dspOutBuffer.samples;

		// Clamp output to both the remaining target frames and the DSP-out buffer capacity
		const AudioFrameIndex maxOutputForThisChunk = fplMin(outRemainingFrameCount, dspOutMaxFrames);

		const AudioResampleResult resampleResult = ResampleChunk(
			inChannelCount,
			inSampleRate,
			outSampleRate,
			maxOutputForThisChunk,
			convertedFrameCount,
			dspIn,
			dspOut
		);

		const AudioFrameIndex playedFrameCount = resampleResult.inputCount;

		// Clamp output frame count to what we actually need. AudioResampleInterleaved
		// can produce 1 extra frame due to rounding when computing outFrameCount from
		// inFrameCount * (outRate/inRate). Without this clamp, the unsigned subtraction
		// outRemainingFrameCount -= outputFrameCount would underflow, causing the loop
		// to run far past the mixing buffer boundary.
		const AudioFrameIndex outputFrameCount = fplMin(resampleResult.outputCount, outRemainingFrameCount);

		// If resampling could not produce any frames (e.g. not enough input for SinC), stop
		if (outputFrameCount == 0 || playedFrameCount == 0) {
			break;
		}

		// ---- Step 3: Apply volume ----
		// Note: we apply volume to the full resampler output (resampleResult.outputCount),
		// not the clamped count, because dspOut contains that many valid samples.
		// However, only outputFrameCount frames will be mixed into the output.
		ApplyVolumeToSamples(inChannelCount, outputFrameCount, volume, dspOut);

		// ---- Step 4: Mix into the mixing buffer with channel up/down conversion ----
		float *mixDest = mixingBuffer + (size_t)totalOutputFrameCount * outChannelCount;
		MixSamplesIntoBuffer(outputFrameCount, inChannelCount, outChannelCount, dspOut, mixDest);

		// ---- Update play position ----
		item->framesPlayed[0] += playedFrameCount;
		fplAssert(item->framesPlayed[0] <= inTotalFrameCount);

		if (item->framesPlayed[0] == inTotalFrameCount) {
			if (item->isRepeat) {
				item->isFinished[0] = false;
				item->framesPlayed[0] = 0; // Reset for next loop iteration
			} else {
				item->isFinished[0] = true;
			}
		}

		totalOutputFrameCount += outputFrameCount;
		outRemainingFrameCount -= outputFrameCount;
	}

	return totalOutputFrameCount;
}

/**
* @brief Mixes all active play items into the F32 mixing buffer.
*
* @details Produces up to targetFrameCount output frames at the system's target sample rate and channel count.
*
* This is the top-level mixing function called from FillConversionBuffer.
* After this returns, the mixing buffer contains additive F32 samples ready
* for final format conversion to the output device format.
*
* Pipeline overview:
*
*   For each active AudioPlayItem:
* ┌──────────────────────────────────────────────────────────────────┐
* │  Source PCM ──► F32 convert ──► Resample ──► Volume ──► Mix +=   │
* └──────────────────────────────────────────────────────────────────┘
* Multiple items accumulate into the same mixing buffer via +=.
*
* @param audioSys         Audio system state (buffers, format, play items list).
* @param targetFrameCount Number of output frames to produce. Must be <= mixingBuffer.maxFrameCount.
* @param advance          If true, finished (non-repeating) play items are removed from the list.
*                         If false, play items are left in place (preview mode).
* @return                 The maximum number of output frames produced by any single play item.
*                         This represents the valid range of the mixing buffer — frames beyond this
*                         are zero (the buffer is cleared at the start).
* @note                   Thread safety: Acquires playItems.lock for the duration of the call. Must not be called concurrently (guarded externally by writeFramesLock).
*/
static AudioFrameIndex WritePlayItemsToMixer2(AudioSystem *audioSys, const AudioFrameIndex targetFrameCount, const bool advance) {
	// The requested frame count must fit in the mixing buffer
	fplAssert(targetFrameCount <= audioSys->mixingBuffer.maxFrameCount);

	// Clear all three static scratch buffers to zero
	fplMemoryClear(audioSys->dspInBuffer.samples, fplArrayCount(audioSys->dspInBuffer.samples));
	fplMemoryClear(audioSys->dspOutBuffer.samples, fplArrayCount(audioSys->dspOutBuffer.samples));
	fplMemoryClear(audioSys->mixingBuffer.samples, fplArrayCount(audioSys->mixingBuffer.samples));

#define GENSINEWAVE 0

#if GENSINEWAVE == 1
	AudioGenerateSineWave(&audioSys->tempWaveData, audioSys->mixingBuffer.samples, fplAudioFormatType_F32, outSampleRate, outChannelCount, targetFrameCount);
	AudioFrameIndex result = targetFrameCount;
	return result;
#else
	float *mixingBuffer = (float *)audioSys->mixingBuffer.samples;
	AudioFrameIndex maxOutputFrameCount = 0;

	fplMutexLock(&audioSys->playItems.lock);

	AudioPlayItem *item = audioSys->playItems.first;
	while (item != fpl_null) {
		// Skip items that were already finished (can happen in advance=false mode)
		if (item->isFinished[0]) {
			item = item->next;
			continue;
		}

		// A PAUSED item is skipped the same way, but it is never removed and its framesPlayed is left alone, so unpausing continues from where it stopped
		if (item->isPaused) {
			item = item->next;
			continue;
		}

		// Process this play item through the full pipeline
		const AudioFrameIndex itemOutputFrames = ProcessSinglePlayItem(audioSys, item, targetFrameCount, mixingBuffer);
		if (itemOutputFrames > maxOutputFrameCount) {
			maxOutputFrameCount = itemOutputFrames;
		}

		// Save next pointer before potential removal
		AudioPlayItem *next = item->next;

		if (item->isFinished[0]) {
			item->framesPlayed[0] = 0;
			if (advance) {
				RemovePlayItem(&audioSys->memory, &audioSys->playItems, item);
			}
		}

		item = next;
	}

	fplMutexUnlock(&audioSys->playItems.lock);

	return maxOutputFrameCount;
#endif
}

static void ClearConversionBuffer(AudioSystem *audioSys) {
	audioSys->conversionBuffer.framesRemaining = 0;
	audioSys->conversionBuffer.readFrameIndex = 0;
}

/**
 * @brief Fills the conversion buffer with mixed/converted samples from the audio sources of the play items.
 *
 * @param audioSys Audio system state (buffers, format, play items list).
 * @param maxFrameCount Maximim number of output frames that can be produced.
 * @param advance If true, finished (non-repeating) play items are removed from the list.
 *                If false, play items are left in place (preview mode).
 * @return Returns the the maximum number of audio frames always, because remaining frames are filled with zero.
 */
static AudioFrameIndex FillConversionBuffer(AudioSystem *audioSys, const AudioFrameIndex maxFrameCount, const bool advance) {
	audioSys->conversionBuffer.framesRemaining = 0;
	audioSys->conversionBuffer.readFrameIndex = 0;
	uint8_t *outSamples = audioSys->conversionBuffer.buffer.samples;
	size_t outBytesPerSample = fplGetAudioSampleSizeInBytes(audioSys->targetFormat.format);
	AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;
	AudioHertz outSampleRate = audioSys->targetFormat.sampleRate;
	fplAudioFormatType outFormat = audioSys->targetFormat.format;

	//
	// This "little" function does all the magic, type-conversion, resampling and the mixing
	//
	AudioFrameIndex mixedFrameCount = WritePlayItemsToMixer2(audioSys, maxFrameCount, advance);

	// Convert mixed samples to final output
	AudioSampleIndex samplesToConvert = mixedFrameCount * outChannelCount;
	AudioSamplesConvert(&audioSys->conversionFuncs, samplesToConvert, fplAudioFormatType_F32, outFormat, audioSys->mixingBuffer.samples, outSamples);
	audioSys->conversionBuffer.framesRemaining += mixedFrameCount;

	// Clear remaining mixing samples
	if (mixedFrameCount < maxFrameCount) {
		AudioFrameIndex zeroFrameCount = maxFrameCount - mixedFrameCount;
		uint8_t *startMixedOutSamples = outSamples + (outBytesPerSample * outChannelCount * mixedFrameCount);
		size_t zeroSize = outBytesPerSample * outChannelCount * zeroFrameCount;
		fplMemoryClear(startMixedOutSamples, zeroSize);
		audioSys->conversionBuffer.framesRemaining += zeroFrameCount;
	}

	fplAssert(audioSys->conversionBuffer.framesRemaining == maxFrameCount);

	return maxFrameCount;
}

fpl_extern AudioFrameIndex AudioSystemWriteFrames(AudioSystem *audioSys, void *outSamples, const fplAudioFormat *outFormat, const AudioFrameIndex frameCount, const bool advance) {
	fplAssert(audioSys != NULL);
	fplAssert(audioSys->targetFormat.sampleRate == outFormat->sampleRate);
	fplAssert(audioSys->targetFormat.format == outFormat->type);
	fplAssert(audioSys->targetFormat.channels == outFormat->channels);
	fplAssert(audioSys->targetFormat.channels <= MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT);

	if(audioSys->isSuspended) {
		// Silence, and NOTHING else touched -- no play item advanced, no source consumed, no lock taken.
		// That is what makes this a pause: the mixer picks up exactly where it left off.
		size_t suspendedFrameStride = fplGetAudioFrameSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels);
		fplMemoryClear(outSamples, suspendedFrameStride * frameCount);
		return(frameCount);
	}

	fplMutexLock(&audioSys->writeFramesLock);

	if(!advance) {
		SavePlayStates(audioSys);
	}

	AudioFrameIndex result = 0;

	size_t outputSampleStride = fplGetAudioFrameSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels);
	size_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

	AudioStream *convBuffer = &audioSys->conversionBuffer;
	size_t maxConversionAudioBufferSize = fplGetAudioBufferSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels, convBuffer->buffer.frameCount);

	// Expect the conversion buffer to be empty at start
	fplAssert(convBuffer->framesRemaining == 0);

	AudioFrameIndex remainingFrames = frameCount;
	while(remainingFrames > 0) {
		// Consume remaining samples in conversion buffer first
		if(convBuffer->framesRemaining > 0) {
			AudioFrameIndex maxFramesToRead = convBuffer->framesRemaining;
			AudioFrameIndex framesToRead = fplMin(remainingFrames, maxFramesToRead);
			size_t bytesToCopy = framesToRead * outputSampleStride;

			size_t sourcePosition = convBuffer->readFrameIndex * outputSampleStride;
			fplAssert(sourcePosition < maxConversionAudioBufferSize);

			size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
			fplAssert(destPosition < maxOutputSampleBufferSize);

			fplMemoryCopy((uint8_t *)convBuffer->buffer.samples + sourcePosition, bytesToCopy, (uint8_t *)outSamples + destPosition);

			remainingFrames -= framesToRead;
			audioSys->conversionBuffer.readFrameIndex += framesToRead;
			audioSys->conversionBuffer.framesRemaining -= framesToRead;
			result += framesToRead;
		}

		if(remainingFrames == 0) {
			// Done
			break;
		}

		// Conversion buffer is empty, fill it with new data
		if(audioSys->conversionBuffer.framesRemaining == 0) {
			AudioFrameIndex framesToFill = fplMin(audioSys->conversionBuffer.buffer.frameCount, remainingFrames);
			AudioFrameIndex convertedFrameCount = FillConversionBuffer(audioSys, framesToFill, advance);
			fplAssert(convertedFrameCount == framesToFill);
			fplAssert(audioSys->conversionBuffer.framesRemaining == framesToFill);
		}
	}

	if(!advance) {
		// Restore saved play states
		RestorePlayStates(audioSys);
	}

	fplMutexUnlock(&audioSys->writeFramesLock);

	return result;
}

fpl_extern void AudioSystemStopAll(AudioSystem *audioSys) {
	if(audioSys == fpl_null || audioSys->isShutdown)
		return;

	AudioMemory *memory = &audioSys->memory;
	AudioPlayItems *playItems = &audioSys->playItems;

	fplMutexLock(&playItems->lock);
	AudioPlayItem *item = playItems->first;
	while(item != fpl_null) {
		AudioPlayItem *next = item->next;
		FreeAudioMemory(memory, item);
		item = next;
	}
	playItems->first = playItems->last = fpl_null;
	playItems->count = 0;
	fplMutexUnlock(&playItems->lock);

	fplMutexLock(&audioSys->writeFramesLock);
	ClearConversionBuffer(audioSys);
	fplMemoryClear(audioSys->mixingBuffer.samples, fplArrayCount(audioSys->mixingBuffer.samples));
	fplMemoryClear(audioSys->dspInBuffer.samples, fplArrayCount(audioSys->dspInBuffer.samples));
	fplMemoryClear(audioSys->dspOutBuffer.samples, fplArrayCount(audioSys->dspOutBuffer.samples));
	fplMutexUnlock(&audioSys->writeFramesLock);
}

fpl_extern bool AudioSystemRemoveSource(AudioSystem *audioSys, AudioSource *source) {
	if(audioSys == fpl_null || source == fpl_null || audioSys->isShutdown) {
		return false;
	}

	AudioMemory *memory = &audioSys->memory;
	AudioSources *sources = &audioSys->sources;

	// Play items first: one still pointing at this source would read freed samples on the very next mix.
	// Its own lock section - the audio thread only ever takes playItems.lock, so there is no lock order to invert here
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		AudioPlayItem *nextPlayItem = playItem->next;
		if(playItem->source == source) {
			RemovePlayItem(memory, &audioSys->playItems, playItem);
		}
		playItem = nextPlayItem;
	}
	fplMutexUnlock(&audioSys->playItems.lock);

	// Singly linked, so the predecessor is found by walking (AudioSource has no prev pointer)
	bool result = false;
	fplMutexLock(&sources->lock);
	AudioSource *previous = fpl_null;
	AudioSource *current = sources->first;
	while(current != fpl_null) {
		if(current == source) {
			if(previous == fpl_null) {
				sources->first = current->next;
			} else {
				previous->next = current->next;
			}
			if(sources->last == current) {
				sources->last = previous;
			}
			--sources->count;
			result = true;
			break;
		}
		previous = current;
		current = current->next;
	}
	fplMutexUnlock(&sources->lock);

	// Freed outside the lock: it is unreachable now, and nothing else in here allocates
	if(result) {
		source->next = fpl_null;
		FreeAudioBuffer(memory, &source->buffer);
		FreeAudioMemory(memory, source);
	}

	return(result);
}

fpl_extern void AudioSystemClearSources(AudioSystem *audioSys) {
	if(audioSys == fpl_null || audioSys->isShutdown)
		return;

	// A play item outliving the source it points at would read freed samples on the next mix, so nothing is left playing.
	// AudioSystemShutdown already did this before calling here; a second pass over an empty list costs nothing
	AudioSystemStopAll(audioSys);

	AudioMemory *memory = &audioSys->memory;
	AudioSources *sources = &audioSys->sources;

	fplMutexLock(&sources->lock);
	AudioSource *source = sources->first;
	while(source != fpl_null) {
		AudioSource *next = source->next;
		FreeAudioBuffer(memory, &source->buffer);
		FreeAudioMemory(memory, source);
		source = next;
	}
	sources->first = sources->last = fpl_null;
	// The count was left standing here, so every source query after a clear reported the sources that were just freed
	sources->count = 0;
	fplMutexUnlock(&sources->lock);
}

fpl_extern bool AudioSystemGetTargetFormat(const AudioSystem *audioSys, AudioFormat *outFormat) {
	if(audioSys == fpl_null || outFormat == fpl_null)
		return(false);
	*outFormat = audioSys->targetFormat;
	return(true);
}

fpl_extern size_t AudioSystemGetSourceStats(AudioSystem *audioSys, size_t *outByteCount) {
	if(outByteCount != fpl_null)
		*outByteCount = 0;
	if(audioSys == fpl_null || audioSys->isShutdown)
		return(0);

	AudioSources *sources = &audioSys->sources;
	fplMutexLock(&sources->lock);
	size_t sourceCount = 0;
	size_t byteCount = 0;
	AudioSource *source = sources->first;
	while(source != fpl_null) {
		++sourceCount;
		byteCount += source->buffer.bufferSize;
		source = source->next;
	}
	fplMutexUnlock(&sources->lock);

	if(outByteCount != fpl_null)
		*outByteCount = byteCount;
	return(sourceCount);
}

fpl_extern void AudioSystemShutdown(AudioSystem *audioSys) {
	if(audioSys != fpl_null) {
		AudioSystemStopAll(audioSys);
		AudioSystemClearSources(audioSys);

		FreeAudioStream(&audioSys->memory, &audioSys->conversionBuffer);

		fplMutexDestroy(&audioSys->writeFramesLock);
		fplMutexDestroy(&audioSys->playItems.lock);
		fplMutexDestroy(&audioSys->sources.lock);

		audioSys->isShutdown = true;
	}
}

static inline bool AreSampleRatesEven(const uint32_t rateA, const uint32_t rateB) {
	return ((rateA % rateB) == 0) || ((rateB % rateA) == 0);
}

fpl_extern bool IsAudioSampleRateSupported(AudioSystem *audioSys, const AudioSampleIndex sampleRate) {
	if (audioSys == fpl_null || audioSys->targetFormat.sampleRate == 0 || sampleRate == 0) {
		return false;
	}
#if 1
	return true;
#else
	bool result = AreSampleRatesEven(sampleRate, audioSys->targetFormat.sampleRate);
	return result;
#endif
}

#endif // FINAL_AUDIOSYSTEM_IMPLEMENTATION