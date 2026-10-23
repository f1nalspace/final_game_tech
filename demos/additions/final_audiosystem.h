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
There can be up to FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT play items in a fixed pool, each play item tracks the current state of a audio source.

A stream source (AudioSourceType_Stream) has no samples in memory, the mixer generates or decodes them while it mixes:
a generated sine wave (AudioSystemCreateSineStreamSource), or an Ogg Vorbis or mp3 file that stays compressed in memory (AudioSystemLoadDataStreamSource, AudioSystemLoadFileStreamSource).
Every play of a stream source gets its own stream instance: a ring of 4096 frames (S16) and its own decoder, allocated by AudioSystemPlaySource, never by the mixer.
Right before the mixer needs the frames from a position on, it lets the decoder fill the ring with exactly them, then reads the ring like samples in memory.
So a stream gives the same samples as the same file decoded in full, bit for bit, also with pitch, after a seek and across a repeat.
An mp3 loop is not seamless (delay and padding of the encoder, about 42 ms), a Vorbis loop is.
A 4:40 track at 44.1 kHz as a stream holds its compressed file (about 6 MB) plus about 30 KB per play for mp3 and about 220 KB for Vorbis, decoded in full it would be about 50 MB.

Loop regions:

A track can have an intro, a loop region from the loop start to the loop end and an outro up to its end, music of games often has them:

|--- intro ---|=========== loop region ===========|--- outro ---|
0          loop start                          loop end      end

- A repeating play item plays the intro once and then only the region, without a gap. Without repeat it plays the whole track, the region changes nothing then.
- Turning repeat off (AudioSystemSetPlayItemRepeat) is the release: the item plays the rest of the region and the outro, then it ends. Turned on again in the outro, it plays to the end and goes on at the loop start.
- The loop start is the first frame of the region and the loop end the frame after its last one, in frames of the source like AudioSystemSetPlayItemPosition(). A seek past the end of a repeating item goes on at the loop start.
- A play item gets the region of its source (AudioSource::loopStartFrame and loopEndFrame) when it starts, AudioSystemSetPlayItemLoop() gives it another one or clears it while it plays. A region that ends past the source is dropped, an endless stream has none.
- A source of a file has the region from the tags of the file: the comments LOOPSTART with LOOPLENGTH or LOOPEND of Ogg Vorbis (LOOPSTART alone loops up to the end) and the first loop of the smpl chunk of a wave file when it loops forward.
- mp3 has no tags. Its frames include the delay of the encoder (1411 frames on the test track at 44.1 kHz), so a region taken from the original audio comes that much later in the mp3.
- While it loops the mixer never reads past the loop end, a stream jumps back with an ordinary seek of its decoder. A stream gives the same samples as the track in memory over any number of passes. At the jump the SinC resampler has the edge of a chunk border, like at the end of a track.

There are several API functions for adding/loading audio sources, start playback by adding play-items, change-volume etc.

How audio frame writing/generation works:

- The samples that are left in the conversion buffer are written first
- If the conversion buffer is empty, new samples are produced
- Samples are produced by looping over all play items and:
	- A stream item lets its decoder fill its ring with the frames from its position on, the steps below read the ring
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
│     │ │ STREAM FILL (stream items only)                             │ │ │
│     │ │ Generator / decoder of the play  →  ring (4096 frames, S16) │ │ │
│     │ └─────────────────────────────────────────────────────────────┘ │ │
│     │                                                                 │ │
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
┌──────────────┐
│ AudioSource  │ ───► Decoder of the play ───► Ring ───► Float Conversion
│   (stream)   │      (in the mixer)          (4096 frames)
└──────────────┘
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

Threading:

- One thread at a time mixes: the one that calls AudioSystemWriteFrames() with advance, usually the audio device callback or a streaming thread. The library starts no thread itself, the mixing thread may change between calls.
- The mixer takes no lock, allocates nothing, does no file IO and never waits for another thread.
- Any number of other threads may play, stop and change play items. AudioSystemPlaySource() takes a slot of the fixed pool, each of these calls sends a command, and the mixer applies the commands at the start of its next call in the order they were sent.
- A play item id is valid as soon as AudioSystemPlaySource() returns. The id of an ended item matches nothing, the setters then return false.
- AudioSystemGetPlayItems() reads without a lock and never holds up the mixer. It returns every item that is started and not finished, also one the mixer has not taken yet. A change by a command shows up after the next mix call.
- AudioSystemRemoveSource(), AudioSystemClearSources() and AudioSystemStopAll() free what the mixer reads, so they return only after their command is applied. When the caller is the mixing thread, or no mixer ran for two periods (at least 50 ms, a stopped device for example), they apply the queue themselves and return at once.
- AudioSystemShutdown() does not wait for the mixer: it takes it at once (only a mix call that runs right then is waited for) and applies the queue itself.
- While one of these functions or a preview (advance = false) holds the mixer, a mixing call writes silence and advances nothing. A second thread that mixes at the same time gets silence as well, it is counted in AudioSystemStats::concurrentWriteCount and a debug build asserts.
- Playing, stopping one item and the setters never block. With a full pool AudioSystemPlaySource() returns the id 0, with a full command queue (FINAL_AUDIOSYSTEM_COMMAND_CAPACITY) they return false or the id 0 and count AudioSystemStats::droppedCommandCount.
- The master volume and the suspend flag are atomic, the mixer reads them once per call.
- The source list has its own lock (sources.lock), the mixer never touches it.
- A stream item is generated or decoded by the mixer in its own call, again with no lock, no allocation and no file IO: the compressed file is in memory, and every play has its decoder memory in its stream instance. A preview (advance = false) leaves stream items out.
- The mixer frees nothing. The stream instance of an ended item stays at its slot until a function on the caller side frees it: AudioSystemPlaySource, AudioSystemStopAll, AudioSystemRemoveSource, AudioSystemClearSources or AudioSystemShutdown. AudioSystemStats::streamInstanceCount includes such instances.
- AudioSystemSetPlayItemPosition() only moves the position when a thread other than the mixer applies the queue, the mixer seeks the decoder at its next call.

Where to mix, the two usual places:
- In the device callback: the lowest latency, every change is heard with the next callback and nothing needs a buffer of its own. The decoding runs in the real-time thread of the device, a Vorbis or mp3 stream adds about 0.1 to 0.2 % of a core to it (48 kHz, measured on x64 and on a Windows XP build).
- In a thread of its own that mixes ahead into a ring, which the device callback only copies (FPL_AudioPlayer --mixer=thread): the work leaves the real-time thread, but every change is heard only after the latency of the ring. That thread has a normal priority, on one busy core it can starve and the ring runs dry (Windows XP lifts a starving thread only after about 4 seconds), a larger ring bridges more.

Todo:
	- Performance
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
	## 2026-10-22
	- New: Stream sources, which generate or decode their frames while they are mixed, starting with a generated sine wave (AudioSystemCreateSineStreamSource())
	- New: AudioSystemStats.streamSourceCount, streamInstanceCount, streamInstanceByteCount and failedStreamCount
	- Changed: A preview (AudioSystemWriteFrames() without advance) leaves stream items out
	- New: Vorbis stream sources, decoded while they are mixed (AudioSystemLoadDataStreamSource(), AudioSystemLoadFileStreamSource())
	- New: AudioSystemStats.compressedByteCount
	- Fixed: AudioSystemShutdown() waited up to two mix periods for a mixer that no longer runs
	- New: AudioSystemSetPlayItemPosition() seeks memory and stream items
	- New: MP3 stream sources (AudioSystemLoadDataStreamSource(), AudioSystemLoadFileStreamSource())
	- New: Documentation of stream sources and where to mix
	- New: Loop regions, a repeating play item plays its intro once and then loops between loop start and loop end (AudioSystemSetPlayItemLoop(), loopStartFrame and loopEndFrame in AudioSource and AudioPlayItem)
	- New: Sources of Ogg Vorbis and wave files get the loop region from the tags of the file
	- New: Documentation of loop regions

	## 2026-10-21
	- Fixed: Even downsampling played past the end of a source whose frame count is not a multiple of the factor
	- Fixed: Even upsampling left the end of every mix call silent when its frame count was no multiple of the factor
	- Fixed: AudioSystemGetPlayItems() and AudioSystemGetSources() could write past the destination when an entry was added between the size check and the copy
	- Changed: The mixer clears only the used part of the mixing buffer and no longer the DSP buffers
	- New: AudioSystemGetStats() returns the sources, their PCM bytes and the playing items in one call
	- Changed: Play items live in a fixed pool of FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT (256) slots, starting or ending one allocates and frees nothing
	- Changed: AudioSystemGetPlayItems() reads without a lock and returns the items in slot order
	- Changed (API BREAKING): AudioSystem.playItems has no list and no count anymore, use AudioSystemGetStats() or AudioSystemGetPlayItems()
	- Changed: Play, stop and the play item setters are commands that the mixer applies at the start of its next call, AudioSystemWriteFrames() takes no lock anymore
	- Changed: AudioSystemStopAll(), AudioSystemRemoveSource(), AudioSystemClearSources() and AudioSystemShutdown() wait for the mixer, or end the play items themselves when no mixer runs
	- New: AudioSystemStats.droppedCommandCount and concurrentWriteCount, a second thread that mixes at the same time gets silence
	- Fixed: AudioSystemShutdown(), AudioSystemStopAll() and AudioSystemClearSources() return at once on a never initialized audio system
	- Changed (API BREAKING): The master volume and the suspend flag are atomic, AudioSystem.masterVolume became masterVolumeBits
	- New: Threading section at the top of this header

	## 2026-08-07
	- New: AudioSystemDefaultPitch - the file as recorded (1.0f), what every call site that does not care about pitch passes. A macro, so it is a constant expression in C99, C++ and MSVC alike
	- Changed (API BREAKING): AudioSystemPlaySource() takes a fifth argument, `pitch`. Every existing call site must add it; pass AudioSystemDefaultPitch for the previous behaviour, which costs nothing (see AudioPitchNeutralTolerance). It is a parameter rather than a post-play setter because pitch is a property of THAT play - a setter would leave the first mixed buffer sounding at the wrong rate before it landed
	- New: AudioSystemSetPlayItemRepeat() turns looping on or off on a play item that is already playing. Clearing it lets the current pass FINISH and the item free itself normally, which is what a "stop looping" control has to mean - AudioSystemStopOne would cut the sound off mid-note instead
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

#ifndef FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT
//! Number of play items that can be started and not finished at the same time, the size of the fixed pool in @ref AudioSystem (at most 65535)
#	define FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT 256
#endif

#ifndef FINAL_AUDIOSYSTEM_COMMAND_CAPACITY
//! Number of commands that can wait for the mixer, a power of two
#	define FINAL_AUDIOSYSTEM_COMMAND_CAPACITY 1024
#endif

typedef struct AudioSourceID {
	uint64_t value;
} AudioSourceID;

typedef enum AudioSourceType {
	AudioSourceType_None = 0,
	AudioSourceType_Allocated,
	//! A stream source, see @ref AudioSource::stream
	AudioSourceType_Stream,
	AudioSourceType_File,
} AudioSourceType;

typedef struct AudioSineWaveData {
	AudioDuration duration;
	double toneVolume;
	AudioHertz frequency;
	AudioFrameIndex frameIndex;
} AudioSineWaveData;

//! Kinds of stream sources, see @ref AudioSourceStream
typedef enum AudioStreamKind {
	//! Not a stream
	AudioStreamKind_None = 0,
	//! A generated sine wave, see @ref AudioSystemCreateSineStreamSource
	AudioStreamKind_Sine,
	//! An Ogg Vorbis file in memory, decoded while it is mixed, see @ref AudioSystemLoadDataStreamSource
	AudioStreamKind_Vorbis,
	//! An mp3 file in memory, decoded while it is mixed, see @ref AudioSystemLoadDataStreamSource
	AudioStreamKind_MP3,
} AudioStreamKind;

//! What a stream source generates or decodes while it is mixed, see @ref AudioSource::stream
typedef struct AudioSourceStream {
	//! The sine wave of @ref AudioStreamKind_Sine, its frameIndex is the frame the stream starts at
	AudioSineWaveData sineWave;
	//! The compressed file of @ref AudioStreamKind_Vorbis and @ref AudioStreamKind_MP3. It lies in the memory block of the source when the source copied it or read it from a file, otherwise it belongs to the caller
	const uint8_t *data;
	//! Size of data in bytes
	size_t dataSize;
	//! Size of the memory block every play gives its decoder, so the decoder never allocates while it is mixed
	size_t decoderMemorySize;
	//! @ref AudioStreamKind_MP3 only: the byte offset of every frame in data, so a seek finds its frame at once. The source owns it
	const uint32_t *frameOffsets;
	//! Number of frameOffsets
	uint32_t frameOffsetCount;
	//! @ref AudioStreamKind_MP3 only: the samples per channel every frame decodes to
	uint32_t frameSampleCount;
	//! @ref AudioStreamKind
	AudioStreamKind kind;
	//! The stream never ends, its @ref AudioSource::buffer has a frame count of 0
	bool isEndless;
} AudioSourceStream;

typedef struct AudioSource {
	//! The samples. A stream source has none, only its frame count is set: the length of the stream, 0 when it is endless
	AudioBuffer buffer;
	//! The format of the samples, for a stream source the format it generates or decodes into (S16)
	AudioFormat format;
	//! Set when the type is @ref AudioSourceType_Stream: every play of the source generates or decodes its frames while it is mixed
	AudioSourceStream stream;
	//! First frame of the loop region, the default of every play (see @ref AudioPlayItem::loopStartFrame). A source of a file has the region from the tags of the file. The program may change it while no item of the source plays
	AudioFrameIndex loopStartFrame;
	//! Frame after the last one of the loop region, 0 when the source has none
	AudioFrameIndex loopEndFrame;
	AudioSourceType type;
	AudioSourceID id;
	struct AudioSource *next;
} AudioSource;

typedef struct AudioPlayItemID {
	uint64_t value;
} AudioPlayItemID;

typedef struct AudioPlayItem {
	AudioFrameIndex framesPlayed[2];	//!< 0 = Current, 1 = Saved
	fpl_b32 isFinished[2];				//!< 0 = Current, 1 = Saved
	uint32_t copiesPlayed[2];			//!< Frame duplication (even upsampling): copies of the frame at framesPlayed that are mixed already, 0 = Current, 1 = Saved
	const AudioSource *source;
	//! Unused, null in every copy
	struct AudioPlayItem *next;
	//! Unused, null in every copy
	struct AudioPlayItem *prev;
	AudioPlayItemID id;
	float volume;
	//! Playback rate multiplier: 2 = an octave up and half as long, 0.5 = an octave down and twice as long.
	//! VARISPEED, like changing a tape's speed -- pitch and duration move together, there is no time-stretch anywhere in this mixer.
	//! Applied by telling the resampler the source has a different sample rate than it really has, which is all a rate conversion ever needed: it is decided by the RATIO alone.
	//! 1 (or anything within @ref AudioPitchNeutralTolerance of it) is the file as recorded and costs nothing.
	float pitch;
	//! First frame of the loop region in frames of the source. While isRepeat is set, the item plays up to loopEndFrame and goes on here, so the frames before it (the intro) play once
	AudioFrameIndex loopStartFrame;
	//! Frame after the last one of the loop region, 0 when the item has none and repeats the whole source. From a position at or past it (the outro) the item plays to the end of the source, then it goes on at loopStartFrame
	AudioFrameIndex loopEndFrame;
	bool isRepeat;
	//! Skipped by the mixer while set, WITHOUT touching framesPlayed - so resuming carries on from the exact frame it stopped on.
	//! That is the whole difference between this and @ref AudioSystemStopOne, which frees the item and forgets where it was.
	bool isPaused;
} AudioPlayItem;

typedef struct AudioSources {
	volatile uint64_t idCounter;
	fplMutexHandle lock;
	AudioSource *first;
	AudioSource *last;
	size_t count;
} AudioSources;

//! State of a slot in the play item pool, see @ref AudioPlayItemSlot
typedef enum AudioPlayItemSlotState {
	//! Never used
	AudioPlayItemSlotState_Free = 0,
	//! Taken by @ref AudioSystemPlaySource, which fills it and starts it
	AudioPlayItemSlotState_Reserved,
	//! Mixed by the mixer
	AudioPlayItemSlotState_Playing,
	//! Ended or stopped, the mixer reads it no more and it can be taken again
	AudioPlayItemSlotState_Finished,
} AudioPlayItemSlotState;

//! Flags of @ref AudioPublishedPlayItem::flags
typedef enum AudioPublishedPlayItemFlags {
	AudioPublishedPlayItemFlags_None = 0,
	//! Started and not finished, @ref AudioSystemGetPlayItems returns it
	AudioPublishedPlayItemFlags_Live = 1 << 0,
	AudioPublishedPlayItemFlags_Finished = 1 << 1,
	AudioPublishedPlayItemFlags_Repeat = 1 << 2,
	AudioPublishedPlayItemFlags_Paused = 1 << 3,
} AudioPublishedPlayItemFlags;

/**
* @brief The state of a play item that @ref AudioSystemGetPlayItems reads without a lock.
* @note One thread at a time writes it, between an odd and the next even sequence. A reader repeats its copy when the sequence was odd or has changed, so it never returns a torn copy and the writer never waits.
* @note Every field is read and written atomically. The generation is part of it, so a copy never pairs the id of one item with the values of another.
*/
typedef struct AudioPublishedPlayItem {
	volatile void *source;
	volatile uint32_t sequence;
	volatile uint32_t generation;
	volatile uint32_t framesPlayed;
	volatile uint32_t copiesPlayed;
	volatile uint32_t volumeBits;
	volatile uint32_t pitchBits;
	volatile uint32_t loopStartFrame;
	volatile uint32_t loopEndFrame;
	//! @ref AudioPublishedPlayItemFlags
	volatile uint32_t flags;
} AudioPublishedPlayItem;

//! One slot of the play item pool in @ref AudioPlayItems
typedef struct AudioPlayItemSlot {
	//! The item the mixer plays, read and written only by the owner of the mixer (@ref AudioSystem::mixerOwner) once the slot is playing
	AudioPlayItem item;
	//! What @ref AudioSystemGetPlayItems copies
	AudioPublishedPlayItem published;
	//! @ref AudioPlayItemSlotState, taken with a compare-and-swap
	volatile uint32_t state;
	//! Part of the id of the item in this slot, counted up when the item ends, so the id of an ended item matches nothing
	volatile uint32_t generation;
	//! The ring and the state of a stream item, null for any other item. Read and written atomically, it belongs to whoever owns the slot: the caller of @ref AudioSystemPlaySource while it is reserved, the mixer while it plays. Once the item has ended, a caller side function frees it
	volatile void *streamInstance;
} AudioPlayItemSlot;

//! The play item pool, starting or ending a play item allocates or frees nothing
typedef struct AudioPlayItems {
	AudioPlayItemSlot slots[FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT];
	//! The slots the mixer plays in the order they were started, which is the order they are mixed in. Only the owner of the mixer reads and writes them
	uint32_t activeSlotIndices[FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT];
	uint32_t activeCount;
	//! Where the search for a free slot starts, so a finished slot is not taken again at once
	volatile uint32_t nextSlotIndex;
} AudioPlayItems;

//! Kinds of @ref AudioCommand
typedef enum AudioCommandKind {
	AudioCommandKind_None = 0,
	//! Starts the reserved slot of playId
	AudioCommandKind_Play,
	//! Ends the item of playId
	AudioCommandKind_Stop,
	//! Ends every item that plays value.source
	AudioCommandKind_StopSource,
	//! Ends every item
	AudioCommandKind_StopAll,
	AudioCommandKind_SetVolume,
	AudioCommandKind_SetPitch,
	AudioCommandKind_SetRepeat,
	AudioCommandKind_SetPaused,
	//! Moves the item of playId to value.frameIndex
	AudioCommandKind_SetPosition,
	//! Sets the loop region of the item of playId to value.loop
	AudioCommandKind_SetLoop,
} AudioCommandKind;

//! The loop region of @ref AudioCommandKind_SetLoop
typedef struct AudioCommandLoop {
	AudioFrameIndex startFrame;
	AudioFrameIndex endFrame;
} AudioCommandLoop;

//! A change of the play items, the mixer applies it at the start of its next @ref AudioSystemWriteFrames call
typedef struct AudioCommand {
	AudioPlayItemID playId;
	union {
		const AudioSource *source;
		float volume;
		float pitch;
		bool isRepeat;
		bool isPaused;
		AudioFrameIndex frameIndex;
		AudioCommandLoop loop;
	} value;
	//! @ref AudioCommandKind
	uint32_t kind;
} AudioCommand;

/**
* @brief The commands of every thread to the mixer, a ring with a single reader.
* @note Producers take producerLock, the mixer never does: it applies everything up to writeIndex and then moves readIndex, which acknowledges the commands to a waiting producer.
*/
typedef struct AudioCommandQueue {
	AudioCommand commands[FINAL_AUDIOSYSTEM_COMMAND_CAPACITY];
	fplMutexHandle producerLock;
	//! Commands sent so far, wraps around
	volatile uint32_t writeIndex;
	//! Commands applied so far, wraps around
	volatile uint32_t readIndex;
	//! Commands refused because the queue was full
	volatile uint32_t droppedCount;
} AudioCommandQueue;

//! Who holds the mixer, see @ref AudioSystem::mixerOwner
typedef enum AudioMixerOwner {
	AudioMixerOwner_None = 0,
	//! @ref AudioSystemWriteFrames with advance
	AudioMixerOwner_Mixing,
	//! @ref AudioSystemWriteFrames without advance
	AudioMixerOwner_Preview,
	//! A function that waits for its commands and applies them itself, because no mixer answers
	AudioMixerOwner_Drain,
	//! @ref AudioSystemShutdown, held until the audio system is initialized again
	AudioMixerOwner_Shutdown,
} AudioMixerOwner;

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
	AudioCommandQueue commandQueue;
	AudioMemory memory;
	//! The thread of the last @ref AudioSystemWriteFrames call with advance (fplGetCurrentThreadId), null before the first one
	volatile void *lastMixThread;
	//! @ref AudioMixerOwner, taken with a compare-and-swap. Only its holder reads and writes the active play items and applies commands, nobody ever waits for it inside the mixer
	volatile uint32_t mixerOwner;
	//! Low 32 bits of fplMillisecondsQuery at the last call with advance
	volatile uint32_t lastMixMilliseconds;
	//! Length of the last call with advance in milliseconds, rounded up
	volatile uint32_t lastMixPeriodMilliseconds;
	//! Calls with advance that found another one mixing, see @ref AudioSystemStats::concurrentWriteCount
	volatile uint32_t concurrentWriteCount;
	//! Bits of the float master volume, any thread stores it, the mixer loads it once per call
	volatile uint32_t masterVolumeBits;
	//! While not 0, @ref AudioSystemWriteFrames hands the device SILENCE and advances nothing. Any thread stores it, the mixer loads it once per call
	volatile uint32_t isSuspended;
	//! Stream instances that are allocated and not freed yet, see @ref AudioSystemStats::streamInstanceCount
	volatile uint32_t streamInstanceCount;
	//! Their size in bytes
	volatile uint32_t streamInstanceByteCount;
	//! Stream items that ended because their stream failed, see @ref AudioSystemStats::failedStreamCount
	volatile uint32_t failedStreamCount;
	bool isInitialized;
	bool isShutdown;
} AudioSystem;

/**
* @brief Initializes the audio system for the given device format.
* @param[out] audioSys Reference to the audio system @ref AudioSystem to initialize.
* @param[in] targetFormat Reference to the device format @ref fplAudioFormat everything is mixed and converted to.
* @return Returns true when the audio system is ready to use, false when an argument is null or a lock could not be created.
* @note The audio system must be released with @ref AudioSystemShutdown.
* @see @ref AudioSystemShutdown
*/
fpl_extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioFormat *targetFormat);

/**
* @brief Stops all play items, frees all sources and releases every resource the audio system owns.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem, may be null or never initialized.
* @note The audio system is marked as shut down and has to be initialized again with @ref AudioSystemInit before it can be used.
* @note Does not wait for the mixer to apply a command: it takes the mixer at once, waiting only for a mix call that runs right then, and holds it until @ref AudioSystemInit. A mix call after that writes silence.
* @see @ref AudioSystemInit
*/
fpl_extern void AudioSystemShutdown(AudioSystem *audioSys);

/**
* @brief Changes the master volume that is applied on top of the volume of every play item.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] newMasterVolume The new master volume, where 1.0 is unchanged (the default) and 0.0 is silent.
* @note Any thread may call it, the mixer uses the new value from its next call on.
*/
fpl_extern void AudioSystemSetMasterVolume(AudioSystem *audioSys, const float newMasterVolume);

/**
* @brief Allocates a source with a silent sample buffer for the given format.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] channels Number of interleaved channels.
* @param[in] sampleRate Sample rate in Hz.
* @param[in] type Sample format @ref fplAudioFormatType.
* @param[in] frameCount Number of frames the sample buffer holds.
* @return Returns the new source, or null when the memory could not be allocated.
* @note The source and its samples live in one memory block. The source gets a unique id but is not added, call @ref AudioSystemAddSource for that.
*/
fpl_extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const fplAudioFormatType type, const AudioFrameIndex frameCount);

/**
* @brief Creates a stream source that generates a sine wave while it is mixed, it has no sample buffer.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] channels Number of interleaved channels, all of them get the same sample.
* @param[in] sampleRate Sample rate in Hz.
* @param[in] sineWave Reference to the sine wave @ref AudioSineWaveData: frequency, volume and duration in seconds, its frameIndex is the frame the stream starts at (usually 0).
* @return Returns the new source, or null when an argument is null or zero, the duration is shorter than one frame or longer than 2^32 frames, or the memory could not be allocated.
* @note A duration above zero gives a stream of (sampleRate * duration + 0.5) frames, which ends or repeats like any other source.
* @note A duration of zero or below gives an endless stream: its frame count is 0, it never ends and repeat does nothing. Its position wraps after 2^32 frames (at 48 kHz after 24.8 hours), the phase jumps once there.
* @note The S16 frames are bit-identical to a sine wave that @ref AudioGenerateSineWave writes into a source of @ref AudioSystemAllocateSource, which holds every frame in memory.
* @note The source is not added, call @ref AudioSystemAddSource for that.
* @see @ref AudioSystemPlaySource
*/
fpl_extern AudioSource *AudioSystemCreateSineStreamSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const AudioSineWaveData *sineWave);

/**
* @brief Creates a stream source for an Ogg Vorbis file that is already in memory, every play decodes it while it is mixed.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] dataSize Size of the data in bytes.
* @param[in] data The complete contents of an Ogg Vorbis or mp3 file.
* @param[in] copyData True copies the data into the memory block of the source. False keeps the pointer, the data must then stay valid and unchanged until the source is freed.
* @return Returns the new source, or null when the data is no Vorbis or mp3 file (a wave file cannot stream), its headers are invalid, it has no length or the memory could not be allocated.
* @note Vorbis: the decoder is opened once here, it reads channels, sample rate, length and the size of the memory block it needs. Every play allocates such a block with its stream instance and opens its own decoder in it, so the decoder never allocates while it is mixed.
* @note Vorbis: the length comes from the last page of the file (stb_vorbis_stream_length_in_samples). A damaged file can decode fewer frames, its play item then ends early and counts in @ref AudioSystemStats::failedStreamCount. Frames past the length are left out.
* @note mp3: a walk over the frame headers reads format and length without decoding and writes the offset of every frame into a frame index (4 bytes per frame), which the source allocates. Every frame must decode to the same number of samples. A seek decodes a few frames before its target and drops them, so the decoder has its bit reservoir again.
* @note mp3: a loop is not seamless. minimp3 plays the delay and the padding of the encoder and an Info or Xing frame as silence, on the test track (44.1 kHz) 1411 frames at the start and 448 at the end, 42 ms in all. A loop that must have no gap needs Vorbis.
* @note The frames are bit-identical to the source that @ref AudioSystemLoadDataSource decodes in full, as long as the file is intact, also after a seek.
* @note The source gets the loop region from the tags of the file (@ref AudioSource::loopStartFrame): the comments LOOPSTART with LOOPLENGTH or LOOPEND of Ogg Vorbis, LOOPSTART alone loops up to the end, and the first loop of the smpl chunk of a wave file when it loops forward. mp3 has no tags. A region that does not fit in the length is left out.
* @note The source is not added, call @ref AudioSystemAddSource for that.
* @see @ref AudioSystemPlaySource
*/
fpl_extern AudioSource *AudioSystemLoadDataStreamSource(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data, const bool copyData);

/**
* @brief Reads an Ogg Vorbis or mp3 file into memory, still compressed, and creates a stream source for it like @ref AudioSystemLoadDataStreamSource.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] filePath Path to an Ogg Vorbis or mp3 file.
* @return Returns the new source, or null when the file could not be read, is no Vorbis or mp3 file (a wave file cannot stream) or @ref AudioSystemLoadDataStreamSource fails for its contents.
* @note The file is read once and closed again, the source holds its compressed contents in its memory block.
* @note The source is not added, call @ref AudioSystemAddSource for that.
* @see @ref AudioSystemLoadFileSource
*/
fpl_extern AudioSource *AudioSystemLoadFileStreamSource(AudioSystem *audioSys, const char *filePath);

/**
* @brief Loads and fully decodes an audio file into a new source.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] filePath Path to a wave, vorbis or mp3 file.
* @return Returns the new source, or null when the file could not be opened, detected or decoded.
* @note The source gets the loop region from the tags of the file (@ref AudioSource::loopStartFrame): the comments LOOPSTART with LOOPLENGTH or LOOPEND of Ogg Vorbis, LOOPSTART alone loops up to the end, and the first loop of the smpl chunk of a wave file when it loops forward. mp3 has no tags. A region that does not fit in the length is left out.
* @note The source is not added, call @ref AudioSystemAddSource for that.
* @see @ref AudioSystemLoadFileFormat
*/
fpl_extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath);

/**
* @brief Reads the format of an audio file without decoding it.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] filePath Path to a wave, vorbis or mp3 file.
* @param[out] outFormat Reference to the format @ref PCMWaveFormat to fill.
* @return Returns true when the format was read, false when the file could not be opened or detected or its headers are invalid.
* @note Headers only, in every supported format (wav: the header; vorbis: the seek table; mp3: a walk over the per-frame headers).
* @note A track's duration is outFormat->frameCount / outFormat->samplesPerSecond.
* @note outFormat->loopStartFrame and loopEndFrame are the loop region from the tags of the file, which a source of the file gets (see @ref AudioSystemLoadFileSource).
* @see @ref AudioSystemLoadFileSource
*/
fpl_extern bool AudioSystemLoadFileFormat(AudioSystem *audioSys, const char *filePath, PCMWaveFormat *outFormat);

/**
* @brief Decodes an audio file that is already in memory into a new source.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] dataSize Size of the data in bytes.
* @param[in] data The complete contents of a wave, vorbis or mp3 file.
* @return Returns the new source, or null when the data could not be detected or decoded.
* @note The samples are copied, so the data can be released after this call. The source is not added, call @ref AudioSystemAddSource for that.
* @note The source gets the loop region from the tags of the file (@ref AudioSource::loopStartFrame): the comments LOOPSTART with LOOPLENGTH or LOOPEND of Ogg Vorbis, LOOPSTART alone loops up to the end, and the first loop of the smpl chunk of a wave file when it loops forward. mp3 has no tags. A region that does not fit in the length is left out.
* @see @ref AudioSystemLoadDataFormat
*/
fpl_extern AudioSource *AudioSystemLoadDataSource(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data);

/**
* @brief Reads the format of an audio file that is already in memory without decoding it.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] dataSize Size of the data in bytes.
* @param[in] data The complete contents of a wave, vorbis or mp3 file.
* @param[out] outFormat Reference to the format @ref PCMWaveFormat to fill.
* @return Returns true when the format was read, false when the data could not be detected or its headers are invalid.
* @see @ref AudioSystemLoadFileFormat
*/
fpl_extern bool AudioSystemLoadDataFormat(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data, PCMWaveFormat *outFormat);

/**
* @brief Adds a source to the audio system, which takes ownership of it.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in,out] source Reference to the source @ref AudioSource, from @ref AudioSystemAllocateSource or one of the load functions.
* @return Returns true when the source was added, false when an argument is null, the source has no id or a source with the same id already exists.
* @note An added source is freed by @ref AudioSystemRemoveSource, @ref AudioSystemClearSources or @ref AudioSystemShutdown.
*/
fpl_extern bool AudioSystemAddSource(AudioSystem *audioSys, AudioSource *source);

/**
* @brief Removes ONE source and frees it, the counterpart to @ref AudioSystemAddSource.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] source Reference to the source @ref AudioSource to remove, invalid after a successful call.
* @return Returns true when the source was removed, false when it is not in the list (it is then not freed).
* @note Any play item still referencing the source is stopped first, so the caller never has to.
* @note The stream instances of ended stream items are freed as well, see @ref AudioSystemPlaySource.
*/
fpl_extern bool AudioSystemRemoveSource(AudioSystem *audioSys, AudioSource *source);

/**
* @brief Mixes all playing sources and writes the given number of frames in the device format.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[out] outSamples Destination buffer for frameCount interleaved frames in the device format.
* @param[in] outFormat Reference to the output format @ref fplAudioFormat, must match the format passed to @ref AudioSystemInit.
* @param[in] frameCount Number of frames to write.
* @param[in] advance Pass true to advance the play items and remove the finished ones, false to render a preview that leaves every play item unchanged.
* @return Returns the number of frames written, which is always frameCount.
* @note This is the heart of the audio system, usually called from the audio device callback. Frames not covered by any play item are filled with silence.
* @note It takes no lock and allocates nothing. It first applies the commands that other threads sent (play, stop, the play item setters).
* @note With advance it generates or decodes the frames of the stream items. A preview leaves them out and does not move them, rewinding a stream would cost a seek.
* @note Only one thread at a time may call it with advance. A second one at the same time gets silence, advances nothing and is counted in @ref AudioSystemStats::concurrentWriteCount, a debug build asserts. A call while a preview runs or a function applies the commands itself gets silence as well.
* @note While the mixer is suspended this only writes silence, see @ref AudioSystemSetSuspended.
*/
fpl_extern AudioFrameIndex AudioSystemWriteFrames(AudioSystem *audioSys, void *outSamples, const fplAudioFormat *outFormat, const AudioFrameIndex frameCount, const bool advance);

/**
* @brief Suspends or resumes the WHOLE mixer.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem, may be null.
* @param[in] suspended Pass true to suspend, false to resume.
* @note While suspended the device is fed silence and no play item advances, so this is a PAUSE and not a mute: every position, every fade and the playlist all stand still, and what comes back is the exact frame that was playing when it stopped.
* @note What a game uses when its window loses focus -- music playing on to an empty desktop is the thing this prevents.
* @note Any thread may call it, the mixer uses the new state from its next call on.
* @see @ref AudioSystemIsSuspended
*/
fpl_extern void AudioSystemSetSuspended(AudioSystem *audioSys, const bool suspended);

/**
* @brief Returns whether the whole mixer is suspended.
* @param[in] audioSys Reference to the audio system @ref AudioSystem, may be null.
* @return Returns true when the mixer is suspended, false when it is not or the audio system is null.
* @see @ref AudioSystemSetSuspended
*/
fpl_extern bool AudioSystemIsSuspended(const AudioSystem *audioSys);

/**
* @brief The pitch that plays a source as recorded, which is what a caller that does not care about pitch passes.
* @note A macro and not a `const float` so it works unchanged in C99, C++ and MSVC, and can be used where a constant expression is required.
*/
#define AudioSystemDefaultPitch 1.0f

/**
* @brief How close to 1 a pitch has to be to count as UNPITCHED.
* @note Below this the source's own sample rate is passed to the resampler untouched, so a clip that matches the device still takes the memcpy passthrough and an even ratio still takes the cheap up/down path.
* @note A slider quantizing to 0.9999999 must not silently push every sound in the game onto the SinC branch forever. 0.1% is about 0.017 semitones: inaudible by a wide margin.
*/
#define AudioPitchNeutralTolerance 0.001f

/**
* @brief Plays a source by adding a new play item.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] source Reference to the source @ref AudioSource to play, it must stay alive while the play item exists.
* @param[in] repeat Pass true to loop the source until the play item is stopped.
* @param[in] volume The volume of the play item, where 1.0 is unchanged.
* @param[in] pitch The playback rate multiplier (see @ref AudioPlayItem::pitch), pass @ref AudioSystemDefaultPitch for the file as recorded, which costs nothing.
* @return Returns the id @ref AudioPlayItemID of the new play item, or an id with a value of zero when an argument is null, all @ref FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT play items are in use or the command queue is full.
* @note A pitch of zero or below is treated as 1 rather than refused: silence would be the one outcome nobody could debug.
* @note The play item gets the loop region of the source (@ref AudioSource::loopStartFrame), unless it ends past the source or the source is an endless stream.
* @note The play item takes a slot of the fixed pool. The id is valid at once and @ref AudioSystemGetPlayItems returns the item at once, the mixer plays it from its next call.
* @note For a source with samples it allocates nothing. For a stream source it allocates the stream instance of this play, a ring of 4096 frames and the state of the stream, which the mixer fills while it mixes. A Vorbis stream gets the memory of its decoder in the same block (about 200 KB) and opens its decoder here.
* @note The stream instance of an ended item is freed by the next call of this function, @ref AudioSystemStopAll, @ref AudioSystemRemoveSource, @ref AudioSystemClearSources or @ref AudioSystemShutdown, never by the mixer.
*/
fpl_extern AudioPlayItemID AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume, const float pitch);

/**
* @brief Stops one play item immediately and frees its slot.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @return Returns true when the stop was sent to a live play item, false when it no longer exists, an argument is null or the command queue is full.
* @note The mixer ends it at the start of its next call, until then @ref AudioSystemGetPlayItems still returns it.
* @note The play position is lost. Use @ref AudioSystemSetPlayItemPaused to pause, or @ref AudioSystemSetPlayItemRepeat to let a looping item finish its current pass.
*/
fpl_extern bool AudioSystemStopOne(AudioSystem *audioSys, const AudioPlayItemID playId);

/**
* @brief Changes the volume of a play item that is ALREADY playing (fades, a volume slider).
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] volume The new volume of the play item, where 1.0 is unchanged.
* @return Returns true when the volume was sent to a live play item, false when the item no longer exists, which is not an error - it just finished, or the command queue is full.
* @note The mixer applies it at the start of its next call, @ref AudioSystemGetPlayItems shows it from then on. An item that ends before that ignores it. The same holds for every play item setter.
* @note Takes an id and not a pointer on purpose: the audio thread ends a finished play item and its slot is used again, the id of an ended item matches nothing.
*/
fpl_extern bool AudioSystemSetPlayItemVolume(AudioSystem *audioSys, const AudioPlayItemID playId, const float volume);

/**
* @brief Changes the PITCH of a play item that is already playing.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] pitch The new playback rate multiplier (see @ref AudioPlayItem::pitch), values <= 0 are ignored rather than obeyed.
* @return Returns true when the pitch was sent to a live play item, false when the pitch is <= 0, the item no longer exists, which is not an error - it just finished, or the command queue is full.
* @note Lets a pitch control be dragged and heard while it moves - which on a sound effect is the only way to judge one, most being over before a slider settles.
* @note The rate changes from the next mixed chunk; frames already written to the device keep the old one.
*/
fpl_extern bool AudioSystemSetPlayItemPitch(AudioSystem *audioSys, const AudioPlayItemID playId, const float pitch);

/**
* @brief Changes whether a play item that is already playing REPEATS.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] repeat Pass true to keep looping, false to let the current pass finish.
* @return Returns true when the repeat state was sent to a live play item, false when the item no longer exists, which is not an error - it just ended, or the command queue is full.
* @note Clearing it lets the current pass finish and the item free itself normally, which is what "stop looping" should mean - @ref AudioSystemStopOne would cut it off mid-sound instead.
* @note With a loop region (@ref AudioSystemSetPlayItemLoop) clearing it is the release: the item plays the rest of the region and the frames after it to the end of the source.
* @note Setting it makes an item that has not ended yet carry on.
*/
fpl_extern bool AudioSystemSetPlayItemRepeat(AudioSystem *audioSys, const AudioPlayItemID playId, const bool repeat);

/**
* @brief Pauses or resumes a play item that is already playing.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] paused Pass true to pause, false to resume.
* @return Returns true when the pause state was sent to a live play item, false when the item no longer exists or the command queue is full.
* @note The mixer skips it while paused and leaves its position alone, so resuming continues from the exact frame it stopped on.
* @note That is what separates this from @ref AudioSystemStopOne, which frees the item and forgets where it was.
*/
fpl_extern bool AudioSystemSetPlayItemPaused(AudioSystem *audioSys, const AudioPlayItemID playId, const bool paused);

/**
* @brief Moves a play item that is already playing to another frame of its source (seeking), memory and stream items alike.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] frameIndex The source frame to play next, counted at the sample rate of the source like @ref AudioPlayItem::framesPlayed.
* @return Returns true when the position was sent to a live play item, false when the item no longer exists or the command queue is full.
* @note A position at or past the end of the source ends the item as its end would, a repeating item goes on at its loop start instead (frame 0 without a loop region). An endless stream takes any position.
* @note A stream item seeks its decoder in the mixer at its next call, a thread that applies the commands itself only sets the position and never decodes. From there on the stream gives the same frames as the same source in memory.
* @note Frames that were mixed already keep playing, the jump is heard from the next mixed chunk on.
*/
fpl_extern bool AudioSystemSetPlayItemPosition(AudioSystem *audioSys, const AudioPlayItemID playId, const AudioFrameIndex frameIndex);

/**
* @brief Sets the loop region of a play item that is already playing, see @ref AudioPlayItem::loopStartFrame.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] playId The id @ref AudioPlayItemID returned by @ref AudioSystemPlaySource.
* @param[in] loopStartFrame First frame of the loop region, counted at the sample rate of the source like @ref AudioPlayItem::framesPlayed.
* @param[in] loopEndFrame Frame after the last one of the loop region, at most the frame count of the source. 0 clears the region, the item repeats the whole source again.
* @return Returns true when the region was sent to a live play item, false when loopStartFrame is not below loopEndFrame (and loopEndFrame is not 0), the item no longer exists or the command queue is full.
* @note The region counts only while the item repeats: it plays the frames before the loop start once, then the region again and again. @ref AudioSystemSetPlayItemRepeat with false releases it, the item then plays the rest of the region and the frames after it, and ends at the end of the source.
* @note The mixer checks the region against the source when it applies it: a region that ends past the source, or any region of an endless stream, is dropped and the item keeps the one it had.
* @note A stream item seeks its decoder at every jump to the loop start, the frames stay the same as the same source in memory.
*/
fpl_extern bool AudioSystemSetPlayItemLoop(AudioSystem *audioSys, const AudioPlayItemID playId, const AudioFrameIndex loopStartFrame, const AudioFrameIndex loopEndFrame);

/**
* @brief Stops and frees all play items.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem, may be null or never initialized.
* @note The sources stay loaded, use @ref AudioSystemClearSources to free them as well. The stream instances of the ended items are freed.
* @note Returns once the mixer has ended them. When the calling thread is the one that mixes, or no mixer ran for two of its periods (at least 50 ms), it ends them itself at once, otherwise it waits at most that long for the mixer. The same holds for @ref AudioSystemRemoveSource and @ref AudioSystemClearSources.
*/
fpl_extern void AudioSystemStopAll(AudioSystem *audioSys);

/**
* @brief Stops all play items, then removes and frees all sources.
* @param[in,out] audioSys Reference to the audio system @ref AudioSystem, may be null or never initialized.
* @note Nothing is left playing, because a play item outliving the source it points at would read freed samples on the next mix.
*/
fpl_extern void AudioSystemClearSources(AudioSystem *audioSys);

/**
* @brief Copies all current play items into an array.
* @param[in] audioSys Reference to the audio system @ref AudioSystem, may be null.
* @param[out] dest Destination array of @ref AudioPlayItem, pass null to query the count only.
* @param[in] maxDestCount Number of elements the destination array can hold.
* @return Returns the number of play items, or zero when the destination array is too small to hold all of them.
* @note Reads without a lock and never holds up the mixer. Each copy is consistent in itself, a play item started by @ref AudioSystemPlaySource is included at once.
* @note The copies come in the order of the pool slots, which is not always the order the items were started in. Their next and prev pointers are null.
*/
fpl_extern size_t AudioSystemGetPlayItems(AudioSystem *audioSys, AudioPlayItem *dest, const size_t maxDestCount);

/**
* @brief Finds an added source by its id.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] id The id @ref AudioSourceID of the source.
* @return Returns the source, or null when no added source has this id.
*/
fpl_extern AudioSource *AudioSystemGetSourceByID(AudioSystem *audioSys, const AudioSourceID id);

/**
* @brief Copies all added sources into an array.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[out] dest Destination array of @ref AudioSource, pass null to query the count only.
* @param[in] maxDestCount Number of elements the destination array can hold.
* @return Returns the number of sources, or zero when the destination array is too small to hold all of them.
* @note The copies are snapshots that share the sample memory of the originals, their next pointers must not be followed.
*/
fpl_extern size_t AudioSystemGetSources(AudioSystem *audioSys, AudioSource *dest, const size_t maxDestCount);

/**
* @brief Returns the DEVICE format everything is mixed and converted to.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[out] outFormat Reference to the format @ref AudioFormat to fill.
* @return Returns true when the format was written, false when an argument is null.
* @note Written once by @ref AudioSystemInit and never again, so this needs no lock.
* @note For a diagnostic readout ("48000 Hz, 2 ch, S16"), which would otherwise have to reach into AudioSystem.targetFormat itself.
*/
fpl_extern bool AudioSystemGetTargetFormat(const AudioSystem *audioSys, AudioFormat *outFormat);

/**
* @brief Returns how many sources are resident, and how many bytes of decoded PCM they hold between them.
* @param[in] audioSys Reference to the audio system @ref AudioSystem, may be null.
* @param[out] outByteCount Receives the size of all sample buffers in bytes, may be null.
* @return Returns the number of added sources, or zero when the audio system is null or shut down.
* @note Walks the list under sources.lock.
* @note The count on its own says nothing about the memory that actually matters - a fully decoded track is roughly 11x its size on disk.
*/
fpl_extern size_t AudioSystemGetSourceStats(AudioSystem *audioSys, size_t *outByteCount);

/**
* @brief Runtime values of the audio system for a diagnostic readout, see @ref AudioSystemGetStats.
*/
typedef struct AudioSystemStats {
	//! Number of added sources
	size_t sourceCount;
	//! Size of the sample buffers of all added sources in bytes
	size_t pcmByteCount;
	//! Play items that are started and not finished, the same ones @ref AudioSystemGetPlayItems returns
	uint32_t activePlayItemCount;
	//! Size of the play item pool, @ref FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT
	uint32_t maxPlayItemCount;
	//! Commands refused because the command queue (@ref FINAL_AUDIOSYSTEM_COMMAND_CAPACITY) was full
	uint32_t droppedCommandCount;
	//! Calls of @ref AudioSystemWriteFrames with advance that found another thread mixing, a programming error
	uint32_t concurrentWriteCount;
	//! Number of added stream sources, they are part of sourceCount
	size_t streamSourceCount;
	//! Size of the compressed files of the added stream sources in bytes, also the ones whose data the caller keeps
	size_t compressedByteCount;
	//! Allocated stream instances, one per play of a stream source: the playing ones and the ended ones that are not freed yet (see @ref AudioSystemPlaySource)
	size_t streamInstanceCount;
	//! Size of the allocated stream instances in bytes
	size_t streamInstanceByteCount;
	//! Stream items that ended early because their stream could not deliver frames
	uint32_t failedStreamCount;
} AudioSystemStats;

/**
* @brief Returns the runtime values of the audio system in one call.
* @param[in] audioSys Reference to the audio system @ref AudioSystem, may be null.
* @param[out] outStats Reference to the stats @ref AudioSystemStats to fill, cleared when the call fails.
* @return Returns true when the stats were written, false when an argument is null or the audio system is shut down.
*/
fpl_extern bool AudioSystemGetStats(AudioSystem *audioSys, AudioSystemStats *outStats);

// @TODO(final): Move to final_audiodemo.h, make a audio source more "Generative"
/**
* @brief Generates a sine wave tone into a sample buffer.
* @param[in,out] waveData Reference to the sine wave state @ref AudioSineWaveData, its frame index is advanced by frameCount.
* @param[out] outSamples Destination buffer for frameCount interleaved frames.
* @param[in] outFormat Sample format @ref fplAudioFormatType of the destination, only S16, S32 and F32 are written.
* @param[in] outSampleRate Sample rate of the destination in Hz.
* @param[in] channels Number of interleaved channels, all of them get the same sample.
* @param[in] frameCount Number of frames to generate.
*/
fpl_extern void AudioGenerateSineWave(AudioSineWaveData *waveData, void *outSamples, const fplAudioFormatType outFormat, const AudioHertz outSampleRate, const AudioChannelIndex channels, const AudioFrameIndex frameCount);

/**
* @brief Returns whether a source with the given sample rate can be played.
* @param[in] audioSys Reference to the audio system @ref AudioSystem.
* @param[in] sampleRate The sample rate of the source in Hz.
* @return Returns true for every sample rate above zero, because the resampler handles arbitrary ratios, false when the audio system is null or not initialized.
*/
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

// FPL has no float atomics, a float is stored and loaded as its bits
static uint32_t AudioFloatToBits(const float value) {
	uint32_t result;
	fplMemoryCopy(&value, sizeof(result), &result);
	return(result);
}

static float AudioBitsToFloat(const uint32_t bits) {
	float result;
	fplMemoryCopy(&bits, sizeof(result), &result);
	return(result);
}

fpl_extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioFormat *targetFormat) {
	if(audioSys == fpl_null) {
		return false;
	}
	if(targetFormat == fpl_null) {
		return false;
	}
	fplClearStruct(audioSys);

	const float defaultMasterVolume = 1.0f;
	const uint32_t defaultMasterVolumeBits = AudioFloatToBits(defaultMasterVolume);
	fplAtomicStoreU32(&audioSys->masterVolumeBits, defaultMasterVolumeBits);

	audioSys->targetFormat.channels = targetFormat->channels;
	audioSys->targetFormat.sampleRate = targetFormat->sampleRate;
	audioSys->targetFormat.format = targetFormat->type;

	if(!fplMutexInit(&audioSys->sources.lock)) {
		return false;
	}
	if(!fplMutexInit(&audioSys->commandQueue.producerLock)) {
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

	audioSys->isInitialized = true;

	return(true);
}

fpl_extern void AudioSystemSetMasterVolume(AudioSystem *audioSys, const float newMasterVolume) {
	const uint32_t masterVolumeBits = AudioFloatToBits(newMasterVolume);
	fplAtomicStoreU32(&audioSys->masterVolumeBits, masterVolumeBits);
}

fpl_extern void AudioSystemSetSuspended(AudioSystem *audioSys, const bool suspended) {
	if (audioSys == fpl_null) {
		return; // no device: there is nothing to suspend, and no caller should have to check
	}
	const uint32_t isSuspended = suspended ? 1 : 0;
	fplAtomicStoreU32(&audioSys->isSuspended, isSuspended);
}

fpl_extern bool AudioSystemIsSuspended(const AudioSystem *audioSys) {
	if (audioSys == fpl_null) {
		return false;
	}
	// The FPL atomics take no const pointer, a load changes nothing
	const uint32_t isSuspended = fplAtomicLoadU32((volatile uint32_t *)&audioSys->isSuspended);
	return(isSuspended != 0);
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

// Every stream generates or decodes into S16, a ring of this many frames per play, the size of one DSP chunk
static const fplAudioFormatType AudioStreamRingFormat = fplAudioFormatType_S16;
static const AudioFrameIndex AudioStreamRingFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;

fpl_extern AudioSource *AudioSystemCreateSineStreamSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const AudioSineWaveData *sineWave) {
	if(audioSys == fpl_null || sineWave == fpl_null || channels == 0 || sampleRate == 0) {
		return fpl_null;
	}

	// The same rounding as a sine wave that is generated into a source with samples
	const double roundingOffset = 0.5;
	const double maxFrameCount = (double)UINT32_MAX;
	const bool isEndless = (sineWave->duration <= 0.0);
	AudioFrameIndex frameCount = 0;
	if(!isEndless) {
		const double frameCountValue = (double)sampleRate * sineWave->duration + roundingOffset;
		if(frameCountValue > maxFrameCount) {
			return fpl_null;
		}
		frameCount = (AudioFrameIndex)frameCountValue;
		if(frameCount == 0) {
			return fpl_null;
		}
	}

	AudioSource *result = (AudioSource *)AllocateAudioMemory(&audioSys->memory, sizeof(AudioSource));
	if(result == fpl_null) {
		return fpl_null;
	}
	fplClearStruct(result);
	result->type = AudioSourceType_Stream;
	result->id.value = fplAtomicIncrementU64(&audioSys->sources.idCounter);
	result->format.channels = channels;
	result->format.sampleRate = sampleRate;
	result->format.format = AudioStreamRingFormat;
	result->buffer.frameCount = frameCount;
	result->stream.kind = AudioStreamKind_Sine;
	result->stream.sineWave = *sineWave;
	result->stream.isEndless = isEndless;
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
	source->loopStartFrame = pcm->format.loopStartFrame;
	source->loopEndFrame = pcm->format.loopEndFrame;

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

// Every play opens its decoder in the memory block of its stream instance, aligned like a heap allocation
static const size_t AudioStreamDecoderAlignment = 16;

// The decoder takes the size of the file and of its memory block as an int
static const size_t AudioStreamMaxDataSize = (size_t)INT32_MAX;

// Reads format, length, loop region and the decoder memory of every play from an Ogg Vorbis file in memory. The decoder is opened once and allocates for itself here, which is never in the mixer
static bool ProbeVorbisStream(const uint8_t *data, const size_t dataSize, AudioFormat *outFormat, AudioFrameIndex *outFrameCount, AudioFrameIndex *outLoopStartFrame, AudioFrameIndex *outLoopEndFrame, size_t *outDecoderMemorySize) {
	if(dataSize > AudioStreamMaxDataSize) {
		return(false);
	}
	int openError = 0;
	stb_vorbis *vorbis = stb_vorbis_open_memory(data, (int)dataSize, &openError, fpl_null);
	if(vorbis == fpl_null) {
		return(false);
	}
	const stb_vorbis_info info = stb_vorbis_get_info(vorbis);
	// Read from the last page, 0 when it has no position
	const unsigned int frameCount = stb_vorbis_stream_length_in_samples(vorbis);
	AudioFrameIndex loopStartFrame = 0;
	AudioFrameIndex loopEndFrame = 0;
	GetVorbisLoopFromDecoder(vorbis, frameCount, &loopStartFrame, &loopEndFrame);
	stb_vorbis_close(vorbis);

	// The setup memory grows from the start of the block, the temporary memory of the setup and of every packet from its end
	const size_t tempMemorySize = fplMax(info.setup_temp_memory_required, info.temp_memory_required);
	const size_t decoderMemorySize = (size_t)info.setup_memory_required + tempMemorySize;
	const bool hasChannels = (info.channels > 0) && (info.channels <= (int)MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT);
	if(!hasChannels || info.sample_rate == 0 || frameCount == 0 || decoderMemorySize > AudioStreamMaxDataSize) {
		return(false);
	}
	outFormat->channels = (AudioChannelIndex)info.channels;
	outFormat->sampleRate = info.sample_rate;
	outFormat->format = AudioStreamRingFormat;
	*outFrameCount = frameCount;
	*outLoopStartFrame = loopStartFrame;
	*outLoopEndFrame = loopEndFrame;
	*outDecoderMemorySize = decoderMemorySize;
	return(true);
}

// Allocates a stream source with room for dataCapacity bytes of file data in its memory block, like AudioSystemAllocateSource. Kind and format are still unset
static AudioSource *AllocateStreamSourceBlock(AudioSystem *audioSys, const size_t dataCapacity, uint8_t **outData) {
	const size_t dataOffset = sizeof(AudioSource) + sizeof(size_t);
	const size_t memorySize = dataOffset + dataCapacity;
	void *memory = AllocateAudioMemory(&audioSys->memory, memorySize);
	if(memory == fpl_null) {
		return fpl_null;
	}
	AudioSource *result = (AudioSource *)memory;
	fplClearStruct(result);
	result->type = AudioSourceType_Stream;
	result->id.value = fplAtomicIncrementU64(&audioSys->sources.idCounter);
	*outData = (dataCapacity > 0) ? ((uint8_t *)memory + dataOffset) : fpl_null;
	return(result);
}

// Makes a source of AllocateStreamSourceBlock a Vorbis stream over the data, false when the data is no valid Vorbis file
static bool SetupVorbisStreamSource(AudioSource *source, const uint8_t *data, const size_t dataSize) {
	AudioFormat format = fplZeroInit;
	AudioFrameIndex frameCount = 0;
	AudioFrameIndex loopStartFrame = 0;
	AudioFrameIndex loopEndFrame = 0;
	size_t decoderMemorySize = 0;
	const bool isProbed = ProbeVorbisStream(data, dataSize, &format, &frameCount, &loopStartFrame, &loopEndFrame, &decoderMemorySize);
	if(!isProbed) {
		return(false);
	}
	source->format = format;
	source->buffer.frameCount = frameCount;
	source->loopStartFrame = loopStartFrame;
	source->loopEndFrame = loopEndFrame;
	source->stream.kind = AudioStreamKind_Vorbis;
	source->stream.data = data;
	source->stream.dataSize = dataSize;
	source->stream.decoderMemorySize = decoderMemorySize;
	return(true);
}

// The ring of a stream holds S16, which minimp3 decodes to unless MINIMP3_FLOAT_OUTPUT is set
fplStaticAssert(sizeof(mp3d_sample_t) == sizeof(int16_t));

// The decoder of one play of an mp3 stream and the frame it decoded last, in the memory block of its stream instance
typedef struct AudioStreamMP3 {
	mp3dec_t decoder;
	// The samples of the frame decoded last, frameReadIndex of its frameSampleCount frames are delivered already
	mp3d_sample_t frameSamples[MINIMP3_MAX_SAMPLES_PER_FRAME];
	// Where the decoder reads its next frame in the data of the source
	size_t bytePosition;
	AudioFrameIndex frameSampleCount;
	AudioFrameIndex frameReadIndex;
} AudioStreamMP3;

// Reads format, length and the number of frames from an mp3 in memory with walks over its frame headers, no decoding. A stream needs the same number of samples in every frame to find a frame by its position
static bool ProbeMP3Stream(const uint8_t *data, const size_t dataSize, AudioFormat *outFormat, AudioFrameIndex *outFrameCount, uint32_t *outFrameOffsetCount, uint32_t *outFrameSampleCount) {
	if(dataSize > AudioStreamMaxDataSize) {
		return(false);
	}
	PCMWaveFormat fileFormat = fplZeroInit;
	if(!LoadMP3FormatFromBuffer(data, dataSize, &fileFormat)) {
		return(false);
	}
	const uint32_t frameOffsetCount = LoadMP3FrameOffsetsFromBuffer(data, dataSize, fpl_null, 0);
	const bool hasChannels = (fileFormat.channelCount > 0) && (fileFormat.channelCount <= MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT);
	if(!hasChannels || fileFormat.samplesPerSecond == 0 || frameOffsetCount == 0) {
		return(false);
	}
	const uint32_t frameSampleCount = fileFormat.frameCount / frameOffsetCount;
	const bool isEveryFrameSameLength = (frameSampleCount * frameOffsetCount == fileFormat.frameCount);
	const bool fitsFrameSamples = ((size_t)frameSampleCount * fileFormat.channelCount <= MINIMP3_MAX_SAMPLES_PER_FRAME);
	if(!isEveryFrameSameLength || !fitsFrameSamples) {
		return(false);
	}
	outFormat->channels = (AudioChannelIndex)fileFormat.channelCount;
	outFormat->sampleRate = fileFormat.samplesPerSecond;
	outFormat->format = AudioStreamRingFormat;
	*outFrameCount = fileFormat.frameCount;
	*outFrameOffsetCount = frameOffsetCount;
	*outFrameSampleCount = frameSampleCount;
	return(true);
}

// Makes a source of AllocateStreamSourceBlock an mp3 stream over the data and allocates its frame index, false when the data is no mp3 that can stream or the index could not be allocated
static bool SetupMP3StreamSource(AudioSystem *audioSys, AudioSource *source, const uint8_t *data, const size_t dataSize) {
	AudioFormat format = fplZeroInit;
	AudioFrameIndex frameCount = 0;
	uint32_t frameOffsetCount = 0;
	uint32_t frameSampleCount = 0;
	const bool isProbed = ProbeMP3Stream(data, dataSize, &format, &frameCount, &frameOffsetCount, &frameSampleCount);
	if(!isProbed) {
		return(false);
	}
	const size_t frameOffsetsSize = sizeof(uint32_t) * frameOffsetCount;
	uint32_t *frameOffsets = (uint32_t *)AllocateAudioMemory(&audioSys->memory, frameOffsetsSize);
	if(frameOffsets == fpl_null) {
		return(false);
	}
	const uint32_t writtenOffsetCount = LoadMP3FrameOffsetsFromBuffer(data, dataSize, frameOffsets, frameOffsetCount);
	if(writtenOffsetCount != frameOffsetCount) {
		FreeAudioMemory(&audioSys->memory, frameOffsets);
		return(false);
	}
	source->format = format;
	source->buffer.frameCount = frameCount;
	source->stream.kind = AudioStreamKind_MP3;
	source->stream.data = data;
	source->stream.dataSize = dataSize;
	source->stream.decoderMemorySize = sizeof(AudioStreamMP3);
	source->stream.frameOffsets = frameOffsets;
	source->stream.frameOffsetCount = frameOffsetCount;
	source->stream.frameSampleCount = frameSampleCount;
	return(true);
}

static bool IsStreamFileFormat(const AudioFileFormat fileFormat) {
	const bool result = (fileFormat == AudioFileFormat_Vorbis) || (fileFormat == AudioFileFormat_MP3);
	return(result);
}

// Makes a source of AllocateStreamSourceBlock a stream of the file format over the data, false when the data is no valid file of a format that can stream
static bool SetupFileStreamSource(AudioSystem *audioSys, AudioSource *source, const AudioFileFormat fileFormat, const uint8_t *data, const size_t dataSize) {
	bool result = false;
	switch(fileFormat) {
		case AudioFileFormat_Vorbis:
		{
			result = SetupVorbisStreamSource(source, data, dataSize);
		} break;

		case AudioFileFormat_MP3:
		{
			result = SetupMP3StreamSource(audioSys, source, data, dataSize);
		} break;

		default:
			break;
	}
	return(result);
}

fpl_extern AudioSource *AudioSystemLoadDataStreamSource(AudioSystem *audioSys, const size_t dataSize, const uint8_t *data, const bool copyData) {
	if(audioSys == fpl_null || dataSize == 0 || data == fpl_null || dataSize > AudioStreamMaxDataSize) {
		return fpl_null;
	}

	AudioSystemStream stream = AudioStreamCreateFromData(dataSize, data);
	const AudioFileFormat fileFormat = PropeAudioFileFormat(&stream);
	if(!IsStreamFileFormat(fileFormat)) {
		return fpl_null;
	}

	const size_t dataCapacity = copyData ? dataSize : 0;
	uint8_t *copiedData = fpl_null;
	AudioSource *source = AllocateStreamSourceBlock(audioSys, dataCapacity, &copiedData);
	if(source == fpl_null) {
		return fpl_null;
	}
	const uint8_t *streamData = data;
	if(copyData) {
		fplMemoryCopy(data, dataSize, copiedData);
		streamData = copiedData;
	}
	const bool isSetUp = SetupFileStreamSource(audioSys, source, fileFormat, streamData, dataSize);
	if(!isSetUp) {
		FreeAudioMemory(&audioSys->memory, source);
		return fpl_null;
	}
	return(source);
}

fpl_extern AudioSource *AudioSystemLoadFileStreamSource(AudioSystem *audioSys, const char *filePath) {
	if(audioSys == fpl_null || fplGetStringLength(filePath) == 0) {
		return fpl_null;
	}

	fplFileHandle file;
	if(!fplFileOpenBinary(filePath, &file)) {
		return fpl_null;
	}

	AudioSource *result = fpl_null;
	const size_t fileSize = fplFileGetSizeFromHandle32(&file);
	if(fileSize > 0 && fileSize <= AudioStreamMaxDataSize) {
		AudioSystemStream stream = AudioStreamCreateFromFileHandle(&file, fileSize);
		const AudioFileFormat fileFormat = PropeAudioFileFormat(&stream);
		if(IsStreamFileFormat(fileFormat)) {
			// Read straight into the memory block of the source, the file is never in memory twice
			uint8_t *data = fpl_null;
			AudioSource *source = AllocateStreamSourceBlock(audioSys, fileSize, &data);
			if(source != fpl_null) {
				AudioSystemStreamSeek(&stream, 0);
				const size_t readSize = AudioSystemStreamRead(&stream, fileSize, data, fileSize);
				const bool isRead = (readSize == fileSize);
				if(isRead && SetupFileStreamSource(audioSys, source, fileFormat, data, fileSize)) {
					result = source;
				} else {
					FreeAudioMemory(&audioSys->memory, source);
				}
			}
		}
	}

	fplFileClose(&file);
	return(result);
}

// A play item id holds the slot index + 1 in its low bits and the generation of the slot above them, so 0 stays invalid
static const uint32_t AudioPlayItemIDSlotBitCount = 16;
static const uint64_t AudioPlayItemIDSlotMask = 0xFFFF;

// The slot number has to fit into the low bits of an id
fplStaticAssert(FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT > 0 && FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT <= 0xFFFF);

static AudioPlayItemID MakePlayItemID(const uint32_t slotIndex, const uint32_t generation) {
	AudioPlayItemID result;
	result.value = ((uint64_t)generation << AudioPlayItemIDSlotBitCount) | (uint64_t)(slotIndex + 1);
	return(result);
}

// Splits an id into its slot and generation, false for an id that no slot can have
static bool DecodePlayItemID(const AudioPlayItemID playId, uint32_t *outSlotIndex, uint32_t *outGeneration) {
	const uint64_t slotNumber = playId.value & AudioPlayItemIDSlotMask;
	const uint64_t idGeneration = playId.value >> AudioPlayItemIDSlotBitCount;
	if(slotNumber == 0 || slotNumber > FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT || idGeneration > UINT32_MAX) {
		return(false);
	}
	*outSlotIndex = (uint32_t)(slotNumber - 1);
	*outGeneration = (uint32_t)idGeneration;
	return(true);
}

// Returns the slot of an id while its item plays, null for the id of an ended item, whose slot may play another one by now. Only by the owner of the mixer
static AudioPlayItemSlot *FindPlayingSlot(AudioPlayItems *playItems, const AudioPlayItemID playId, uint32_t *outSlotIndex) {
	uint32_t slotIndex = 0;
	uint32_t idGeneration = 0;
	const bool isValid = DecodePlayItemID(playId, &slotIndex, &idGeneration);
	if(!isValid) {
		return(fpl_null);
	}
	AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
	const uint32_t state = fplAtomicLoadU32(&slot->state);
	const uint32_t generation = fplAtomicLoadU32(&slot->generation);
	if(state != AudioPlayItemSlotState_Playing || generation != idGeneration) {
		return(fpl_null);
	}
	*outSlotIndex = slotIndex;
	return(slot);
}

// Finds the position of a playing item in the active slots. Only by the owner of the mixer
static bool FindActivePlayItem(AudioPlayItems *playItems, const AudioPlayItemID playId, uint32_t *outActiveIndex) {
	uint32_t slotIndex = 0;
	AudioPlayItemSlot *slot = FindPlayingSlot(playItems, playId, &slotIndex);
	if(slot == fpl_null) {
		return(false);
	}
	for(uint32_t activeIndex = 0; activeIndex < playItems->activeCount; ++activeIndex) {
		if(playItems->activeSlotIndices[activeIndex] == slotIndex) {
			*outActiveIndex = activeIndex;
			return(true);
		}
	}
	return(false);
}

// Writes the item of a slot into its published state, see AudioPublishedPlayItem. One thread at a time: the owner of the mixer, or the caller of AudioSystemPlaySource while the slot is reserved
static void PublishPlayItem(AudioPlayItemSlot *slot, const bool isLive) {
	const AudioPlayItem *item = &slot->item;
	AudioPublishedPlayItem *published = &slot->published;
	uint32_t flags = AudioPublishedPlayItemFlags_None;
	if(isLive) {
		flags |= AudioPublishedPlayItemFlags_Live;
	}
	if(item->isFinished[0]) {
		flags |= AudioPublishedPlayItemFlags_Finished;
	}
	if(item->isRepeat) {
		flags |= AudioPublishedPlayItemFlags_Repeat;
	}
	if(item->isPaused) {
		flags |= AudioPublishedPlayItemFlags_Paused;
	}
	const uint32_t generation = fplAtomicLoadU32(&slot->generation);
	const uint32_t volumeBits = AudioFloatToBits(item->volume);
	const uint32_t pitchBits = AudioFloatToBits(item->pitch);

	// Odd while the fields are written, a reader that sees it or a change of it copies again
	const uint32_t sequence = fplAtomicLoadU32(&published->sequence);
	fplAtomicStoreU32(&published->sequence, sequence + 1);
	fplAtomicStorePtr(&published->source, item->source);
	fplAtomicStoreU32(&published->generation, generation);
	fplAtomicStoreU32(&published->framesPlayed, item->framesPlayed[0]);
	fplAtomicStoreU32(&published->copiesPlayed, item->copiesPlayed[0]);
	fplAtomicStoreU32(&published->volumeBits, volumeBits);
	fplAtomicStoreU32(&published->pitchBits, pitchBits);
	fplAtomicStoreU32(&published->loopStartFrame, item->loopStartFrame);
	fplAtomicStoreU32(&published->loopEndFrame, item->loopEndFrame);
	fplAtomicStoreU32(&published->flags, flags);
	fplAtomicStoreU32(&published->sequence, sequence + 2);
}

// Copies the published state of a slot, false when it holds no started and unfinished item. Never makes the writer wait, it repeats the copy instead
static bool CopyPublishedPlayItem(AudioPlayItemSlot *slot, const uint32_t slotIndex, AudioPlayItem *outItem) {
	AudioPublishedPlayItem *published = &slot->published;
	for(;;) {
		const uint32_t sequenceBefore = fplAtomicLoadU32(&published->sequence);
		if((sequenceBefore & 1) != 0) {
			// The writer is in the middle of it and done in a moment
			fplThreadYield();
			continue;
		}
		void *source = fplAtomicLoadPtr(&published->source);
		const uint32_t generation = fplAtomicLoadU32(&published->generation);
		const uint32_t framesPlayed = fplAtomicLoadU32(&published->framesPlayed);
		const uint32_t copiesPlayed = fplAtomicLoadU32(&published->copiesPlayed);
		const uint32_t volumeBits = fplAtomicLoadU32(&published->volumeBits);
		const uint32_t pitchBits = fplAtomicLoadU32(&published->pitchBits);
		const uint32_t loopStartFrame = fplAtomicLoadU32(&published->loopStartFrame);
		const uint32_t loopEndFrame = fplAtomicLoadU32(&published->loopEndFrame);
		const uint32_t flags = fplAtomicLoadU32(&published->flags);
		const uint32_t sequenceAfter = fplAtomicLoadU32(&published->sequence);
		if(sequenceAfter != sequenceBefore) {
			continue;
		}
		if((flags & AudioPublishedPlayItemFlags_Live) == 0) {
			return(false);
		}
		fplClearStruct(outItem);
		outItem->id = MakePlayItemID(slotIndex, generation);
		outItem->source = (const AudioSource *)source;
		outItem->framesPlayed[0] = framesPlayed;
		outItem->copiesPlayed[0] = copiesPlayed;
		outItem->isFinished[0] = (flags & AudioPublishedPlayItemFlags_Finished) != 0;
		outItem->volume = AudioBitsToFloat(volumeBits);
		outItem->pitch = AudioBitsToFloat(pitchBits);
		outItem->loopStartFrame = loopStartFrame;
		outItem->loopEndFrame = loopEndFrame;
		outItem->isRepeat = (flags & AudioPublishedPlayItemFlags_Repeat) != 0;
		outItem->isPaused = (flags & AudioPublishedPlayItemFlags_Paused) != 0;
		return(true);
	}
}

// Takes a free or finished slot for a new item, null when every slot is in use
static AudioPlayItemSlot *ReservePlayItemSlot(AudioPlayItems *playItems, uint32_t *outSlotIndex) {
	const uint32_t slotCount = FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT;
	const uint32_t startIndex = fplAtomicLoadU32(&playItems->nextSlotIndex);
	for(uint32_t offset = 0; offset < slotCount; ++offset) {
		const uint32_t slotIndex = (startIndex + offset) % slotCount;
		AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
		const uint32_t state = fplAtomicLoadU32(&slot->state);
		if(state != AudioPlayItemSlotState_Free && state != AudioPlayItemSlotState_Finished) {
			continue;
		}
		// Another thread may take the same slot in between, only one compare-and-swap succeeds
		const bool isReserved = fplAtomicIsCompareAndSwapU32(&slot->state, state, AudioPlayItemSlotState_Reserved);
		if(isReserved) {
			const uint32_t nextSlotIndex = (slotIndex + 1) % slotCount;
			fplAtomicStoreU32(&playItems->nextSlotIndex, nextSlotIndex);
			*outSlotIndex = slotIndex;
			return(slot);
		}
	}
	return(fpl_null);
}

// Ends the item of an active slot: AudioSystemGetPlayItems no longer returns it, its id matches nothing and the slot can be taken again. Only by the owner of the mixer
static void FinishActivePlayItem(AudioPlayItems *playItems, const uint32_t activeIndex) {
	fplAssert(activeIndex < playItems->activeCount);
	const uint32_t slotIndex = playItems->activeSlotIndices[activeIndex];
	AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
	PublishPlayItem(slot, false);
	fplAtomicIncrementU32(&slot->generation);
	fplAtomicStoreU32(&slot->state, AudioPlayItemSlotState_Finished);

	// The others move up instead of the last one taking this place: the order is the mix order, and a different order changes the float sums
	for(uint32_t moveIndex = activeIndex + 1; moveIndex < playItems->activeCount; ++moveIndex) {
		playItems->activeSlotIndices[moveIndex - 1] = playItems->activeSlotIndices[moveIndex];
	}
	--playItems->activeCount;
}

// The state of one play of a stream source, see AudioPlayItemSlot::streamInstance. One memory block: this struct, the ring, then the memory of the decoder.
// The ring holds ringFrameCount frames of the source from ringStartFrameIndex on, the stream delivers its next frame right after them.
typedef struct AudioStreamInstance {
	uint8_t *ringSamples;
	// The decoder of AudioStreamKind_Vorbis, opened in the memory block after the ring
	stb_vorbis *vorbis;
	// The decoder of AudioStreamKind_MP3, in the memory block after the ring
	AudioStreamMP3 *mp3;
	// Size of the memory block in bytes
	uint32_t memorySize;
	AudioFrameIndex ringStartFrameIndex;
	AudioFrameIndex ringFrameCount;
	// The sine wave of AudioStreamKind_Sine, its frameIndex is the next frame it generates
	AudioSineWaveData sineWave;
	AudioStreamKind kind;
} AudioStreamInstance;

// Whether the mixer generates or decodes the frames of a source. A source that only carries the type, which an older final_audiodemo.h set for its sine wave, plays its samples
static bool IsStreamSource(const AudioSource *source) {
	const bool result = (source->type == AudioSourceType_Stream) && (source->stream.kind != AudioStreamKind_None);
	return(result);
}

// Whether a loop region fits the source: its start before its end, its end at most the frame count. An endless stream has a frame count of 0 and takes none
static bool IsLoopRegionValid(const AudioSource *source, const AudioFrameIndex loopStartFrame, const AudioFrameIndex loopEndFrame) {
	const bool result = (loopStartFrame < loopEndFrame) && (loopEndFrame <= source->buffer.frameCount);
	return(result);
}

// Allocates the stream instance for one play of a stream source, null when the memory could not be allocated. Not by the mixer
static AudioStreamInstance *CreateStreamInstance(AudioSystem *audioSys, const AudioSource *source) {
	fplAssert(source->format.format == AudioStreamRingFormat);
	const size_t ringSize = fplGetAudioBufferSizeInBytes(source->format.format, source->format.channels, AudioStreamRingFrameCount);
	const size_t ringEndOffset = sizeof(AudioStreamInstance) + ringSize;
	// The decoder gets the end of the block, with room to align its start
	const bool hasDecoder = (source->stream.kind == AudioStreamKind_Vorbis) || (source->stream.kind == AudioStreamKind_MP3);
	const size_t decoderBlockSize = hasDecoder ? (source->stream.decoderMemorySize + AudioStreamDecoderAlignment) : 0;
	const size_t memorySize = ringEndOffset + decoderBlockSize;
	void *memory = AllocateAudioMemory(&audioSys->memory, memorySize);
	if(memory == fpl_null) {
		return(fpl_null);
	}
	AudioStreamInstance *result = (AudioStreamInstance *)memory;
	fplClearStruct(result);
	result->ringSamples = (uint8_t *)memory + sizeof(AudioStreamInstance);
	result->memorySize = (uint32_t)memorySize;
	result->kind = source->stream.kind;
	const uintptr_t ringEndAddress = (uintptr_t)memory + ringEndOffset;
	const uintptr_t alignmentOffset = fplGetAlignmentOffset(ringEndAddress, (uintptr_t)AudioStreamDecoderAlignment);
	void *decoderMemoryStart = (void *)(ringEndAddress + alignmentOffset);
	switch(result->kind) {
		case AudioStreamKind_Sine:
		{
			result->sineWave = source->stream.sineWave;
		} break;

		case AudioStreamKind_Vorbis:
		{
			// Its own decoder in its own memory, opened from the compressed file of the source. It never allocates, also not later in the mixer
			stb_vorbis_alloc decoderMemory = fplZeroInit;
			decoderMemory.alloc_buffer = (char *)decoderMemoryStart;
			decoderMemory.alloc_buffer_length_in_bytes = (int)source->stream.decoderMemorySize;
			int openError = 0;
			result->vorbis = stb_vorbis_open_memory(source->stream.data, (int)source->stream.dataSize, &openError, &decoderMemory);
			if(result->vorbis == fpl_null) {
				FreeAudioMemory(&audioSys->memory, memory);
				return(fpl_null);
			}
		} break;

		case AudioStreamKind_MP3:
		{
			// Its own decoder, which starts at the first frame like LoadMP3FromBuffer
			AudioStreamMP3 *mp3 = (AudioStreamMP3 *)decoderMemoryStart;
			fplClearStruct(mp3);
			mp3dec_init(&mp3->decoder);
			mp3->bytePosition = source->stream.frameOffsets[0];
			result->mp3 = mp3;
		} break;

		default:
		{
			fplAssert(!"Unknown stream kind");
		} break;
	}
	fplAtomicIncrementU32(&audioSys->streamInstanceCount);
	fplAtomicFetchAndAddU32(&audioSys->streamInstanceByteCount, result->memorySize);
	return(result);
}

// Closes the stream and frees its instance. Not by the mixer
static void FreeStreamInstance(AudioSystem *audioSys, AudioStreamInstance *instance) {
	// Its memory is part of the instance, closing frees nothing
	if(instance->vorbis != fpl_null) {
		stb_vorbis_close(instance->vorbis);
		instance->vorbis = fpl_null;
	}
	// Added as unsigned values that wrap around, which subtracts them
	const uint32_t countDecrement = UINT32_MAX;
	const uint32_t byteCountDecrement = 0u - instance->memorySize;
	fplAtomicFetchAndAddU32(&audioSys->streamInstanceCount, countDecrement);
	fplAtomicFetchAndAddU32(&audioSys->streamInstanceByteCount, byteCountDecrement);
	FreeAudioMemory(&audioSys->memory, instance);
}

// Frees the stream instances of all ended items. Never by the mixer, which leaves an ended instance at its slot: AudioSystemPlaySource, AudioSystemStopAll, AudioSystemRemoveSource, AudioSystemClearSources and AudioSystemShutdown call it
static void ReclaimFinishedStreamInstances(AudioSystem *audioSys) {
	AudioPlayItems *playItems = &audioSys->playItems;
	for(uint32_t slotIndex = 0; slotIndex < FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT; ++slotIndex) {
		AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
		const uint32_t state = fplAtomicLoadU32(&slot->state);
		if(state != AudioPlayItemSlotState_Finished) {
			continue;
		}
		void *finishedInstance = fplAtomicLoadPtr(&slot->streamInstance);
		if(finishedInstance == fpl_null) {
			continue;
		}
		// Reserved while it is freed, so no other thread takes the slot meanwhile. The generation stays, the id of the ended item still matches nothing
		const bool isReserved = fplAtomicIsCompareAndSwapU32(&slot->state, AudioPlayItemSlotState_Finished, AudioPlayItemSlotState_Reserved);
		if(!isReserved) {
			continue;
		}
		// Loaded again, another thread may have freed it before the slot was reserved
		AudioStreamInstance *instance = (AudioStreamInstance *)fplAtomicLoadPtr(&slot->streamInstance);
		if(instance != fpl_null) {
			fplAtomicStorePtr(&slot->streamInstance, fpl_null);
			FreeStreamInstance(audioSys, instance);
		}
		fplAtomicStoreU32(&slot->state, AudioPlayItemSlotState_Finished);
	}
}

static const uint32_t AudioMillisecondsPerSecond = 1000;

// A mixer that did not call AudioSystemWriteFrames for this many of its periods counts as stopped, but never before the minimum
static const uint32_t AudioMixerIdlePeriodCount = 2;
static const uint32_t AudioMixerIdleMinimumMilliseconds = 50;

// A waiting function yields this many times before it sleeps between its checks
static const uint32_t AudioCommandWaitYieldCount = 64;
static const uint32_t AudioCommandWaitSleepMilliseconds = 1;

// The indices of the command queue wrap around at 2^32, which only works for a power of two
fplStaticAssert(FINAL_AUDIOSYSTEM_COMMAND_CAPACITY > 0 && (FINAL_AUDIOSYSTEM_COMMAND_CAPACITY & (FINAL_AUDIOSYSTEM_COMMAND_CAPACITY - 1)) == 0);

// Whether an id belongs to an item that is started and not finished, from any thread
static bool IsPlayItemLive(AudioPlayItems *playItems, const AudioPlayItemID playId) {
	uint32_t slotIndex = 0;
	uint32_t idGeneration = 0;
	const bool isValid = DecodePlayItemID(playId, &slotIndex, &idGeneration);
	if(!isValid) {
		return(false);
	}
	AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
	const uint32_t state = fplAtomicLoadU32(&slot->state);
	const uint32_t generation = fplAtomicLoadU32(&slot->generation);
	const bool isStarted = (state == AudioPlayItemSlotState_Reserved) || (state == AudioPlayItemSlotState_Playing);
	const bool result = isStarted && (generation == idGeneration);
	return(result);
}

// Puts a reserved slot into the active slots, it is mixed from now on. Only by the owner of the mixer
static void StartReservedPlayItem(AudioPlayItems *playItems, const AudioPlayItemID playId) {
	uint32_t slotIndex = 0;
	uint32_t idGeneration = 0;
	const bool isValid = DecodePlayItemID(playId, &slotIndex, &idGeneration);
	fplAssert(isValid);
	if(!isValid) {
		return;
	}
	AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
	const uint32_t state = fplAtomicLoadU32(&slot->state);
	fplAssert(state == AudioPlayItemSlotState_Reserved);
	fplAssert(playItems->activeCount < FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT);
	if(state != AudioPlayItemSlotState_Reserved || playItems->activeCount >= FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT) {
		return;
	}
	playItems->activeSlotIndices[playItems->activeCount] = slotIndex;
	++playItems->activeCount;
	fplAtomicStoreU32(&slot->state, AudioPlayItemSlotState_Playing);
}

// Sends a command to the mixer, false when the queue is full. The sequence is the write index after it, a waiting function waits until the read index reaches it
static bool PushAudioCommand(AudioCommandQueue *queue, const AudioCommand *command, uint32_t *outSequence) {
	bool result = false;
	fplMutexLock(&queue->producerLock);
	const uint32_t writeIndex = fplAtomicLoadU32(&queue->writeIndex);
	const uint32_t readIndex = fplAtomicLoadU32(&queue->readIndex);
	const uint32_t queuedCount = writeIndex - readIndex;
	if(queuedCount < FINAL_AUDIOSYSTEM_COMMAND_CAPACITY) {
		const uint32_t commandIndex = writeIndex % FINAL_AUDIOSYSTEM_COMMAND_CAPACITY;
		queue->commands[commandIndex] = *command;
		const uint32_t nextWriteIndex = writeIndex + 1;
		// The command is visible to the mixer once the index moves on
		fplAtomicStoreU32(&queue->writeIndex, nextWriteIndex);
		if(outSequence != fpl_null) {
			*outSequence = nextWriteIndex;
		}
		result = true;
	}
	fplMutexUnlock(&queue->producerLock);
	return(result);
}

// Applies one command to the play items. Only by the owner of the mixer
static void ApplyAudioCommand(AudioPlayItems *playItems, const AudioCommand *command) {
	uint32_t slotIndex = 0;
	switch(command->kind) {
		case AudioCommandKind_Play:
		{
			StartReservedPlayItem(playItems, command->playId);
		} break;

		case AudioCommandKind_Stop:
		{
			uint32_t activeIndex = 0;
			const bool isActive = FindActivePlayItem(playItems, command->playId, &activeIndex);
			if(isActive) {
				FinishActivePlayItem(playItems, activeIndex);
			}
		} break;

		case AudioCommandKind_StopSource:
		{
			uint32_t activeIndex = 0;
			while(activeIndex < playItems->activeCount) {
				const uint32_t activeSlotIndex = playItems->activeSlotIndices[activeIndex];
				const AudioPlayItem *playItem = &playItems->slots[activeSlotIndex].item;
				if(playItem->source == command->value.source) {
					// The next item moves up to this index
					FinishActivePlayItem(playItems, activeIndex);
					continue;
				}
				++activeIndex;
			}
		} break;

		case AudioCommandKind_StopAll:
		{
			// From the end, so nothing has to move up
			while(playItems->activeCount > 0) {
				FinishActivePlayItem(playItems, playItems->activeCount - 1);
			}
		} break;

		// A setter whose item ended before the command arrived changes nothing
		case AudioCommandKind_SetVolume:
		{
			AudioPlayItemSlot *slot = FindPlayingSlot(playItems, command->playId, &slotIndex);
			if(slot != fpl_null) {
				slot->item.volume = command->value.volume;
				PublishPlayItem(slot, true);
			}
		} break;

		case AudioCommandKind_SetPitch:
		{
			AudioPlayItemSlot *slot = FindPlayingSlot(playItems, command->playId, &slotIndex);
			if(slot != fpl_null) {
				slot->item.pitch = command->value.pitch;
				PublishPlayItem(slot, true);
			}
		} break;

		case AudioCommandKind_SetRepeat:
		{
			AudioPlayItemSlot *slot = FindPlayingSlot(playItems, command->playId, &slotIndex);
			if(slot != fpl_null) {
				slot->item.isRepeat = command->value.isRepeat;
				PublishPlayItem(slot, true);
			}
		} break;

		case AudioCommandKind_SetPaused:
		{
			AudioPlayItemSlot *slot = FindPlayingSlot(playItems, command->playId, &slotIndex);
			if(slot != fpl_null) {
				slot->item.isPaused = command->value.isPaused;
				PublishPlayItem(slot, true);
			}
		} break;

		case AudioCommandKind_SetPosition:
		{
			uint32_t activeIndex = 0;
			const bool isActive = FindActivePlayItem(playItems, command->playId, &activeIndex);
			if(!isActive) {
				break;
			}
			const uint32_t activeSlotIndex = playItems->activeSlotIndices[activeIndex];
			AudioPlayItemSlot *slot = &playItems->slots[activeSlotIndex];
			AudioPlayItem *item = &slot->item;
			const AudioSource *source = item->source;
			const bool isEndless = IsStreamSource(source) && source->stream.isEndless;
			const bool isPastEnd = !isEndless && (command->value.frameIndex >= source->buffer.frameCount);
			if(isPastEnd && !item->isRepeat) {
				// As at the end of the source
				FinishActivePlayItem(playItems, activeIndex);
				break;
			}
			// The ring of a stream no longer holds the position, so the stream seeks at the next mix. Whoever applies the queue only sets it.
			// Past the end a repeating item goes on as at the end, at its loop start, which is 0 without a loop region
			item->framesPlayed[0] = isPastEnd ? item->loopStartFrame : command->value.frameIndex;
			item->copiesPlayed[0] = 0;
			PublishPlayItem(slot, true);
		} break;

		case AudioCommandKind_SetLoop:
		{
			AudioPlayItemSlot *slot = FindPlayingSlot(playItems, command->playId, &slotIndex);
			if(slot == fpl_null) {
				break;
			}
			AudioPlayItem *item = &slot->item;
			const AudioFrameIndex loopStartFrame = command->value.loop.startFrame;
			const AudioFrameIndex loopEndFrame = command->value.loop.endFrame;
			const bool isCleared = (loopEndFrame == 0);
			const bool isValid = IsLoopRegionValid(item->source, loopStartFrame, loopEndFrame);
			if(!isCleared && !isValid) {
				// Ends past the source, or the source is an endless stream: the item keeps the region it had
				break;
			}
			// Without a region the loop start stays 0, where a repeat goes on
			item->loopStartFrame = isCleared ? 0 : loopStartFrame;
			item->loopEndFrame = loopEndFrame;
			PublishPlayItem(slot, true);
		} break;

		default:
		{
			fplAssert(!"Unknown audio command");
		} break;
	}
}

// Applies every queued command in the order it was sent, then moves the read index, which acknowledges them. Only by the owner of the mixer
static void DrainAudioCommands(AudioSystem *audioSys) {
	AudioCommandQueue *queue = &audioSys->commandQueue;
	const uint32_t writeIndex = fplAtomicLoadU32(&queue->writeIndex);
	uint32_t readIndex = fplAtomicLoadU32(&queue->readIndex);
	while(readIndex != writeIndex) {
		const uint32_t commandIndex = readIndex % FINAL_AUDIOSYSTEM_COMMAND_CAPACITY;
		ApplyAudioCommand(&audioSys->playItems, &queue->commands[commandIndex]);
		++readIndex;
	}
	fplAtomicStoreU32(&queue->readIndex, readIndex);
}

// Takes the mixer for the given owner, returns the owner it had before: AudioMixerOwner_None when it was taken
static uint32_t TryAcquireMixerOwner(AudioSystem *audioSys, const AudioMixerOwner owner) {
	const uint32_t result = fplAtomicCompareAndSwapU32(&audioSys->mixerOwner, AudioMixerOwner_None, owner);
	return(result);
}

static void ReleaseMixerOwner(AudioSystem *audioSys) {
	fplAtomicStoreU32(&audioSys->mixerOwner, AudioMixerOwner_None);
}

static void WaitForAudioCommandStep(const uint32_t attemptIndex) {
	if(attemptIndex < AudioCommandWaitYieldCount) {
		fplThreadYield();
	} else {
		fplThreadSleep(AudioCommandWaitSleepMilliseconds);
	}
}

// Returns once the commands up to the sequence are applied. Applies the queue itself when the calling thread is the one that mixes, when no mixer runs, or when the mixer did not answer in time
static void WaitForAudioCommands(AudioSystem *audioSys, const uint32_t sequence) {
	AudioCommandQueue *queue = &audioSys->commandQueue;
	const fplMilliseconds startMilliseconds = fplMillisecondsQuery();
	const uint64_t currentThreadId = fplGetCurrentThreadId();
	for(uint32_t attemptIndex = 0;; ++attemptIndex) {
		const uint32_t readIndex = fplAtomicLoadU32(&queue->readIndex);
		const int32_t missingCount = (int32_t)(sequence - readIndex);
		if(missingCount <= 0) {
			return;
		}

		void *lastMixThread = fplAtomicLoadPtr(&audioSys->lastMixThread);
		const bool hasMixed = (lastMixThread != fpl_null);
		const bool isMixerThread = hasMixed && ((uintptr_t)lastMixThread == (uintptr_t)currentThreadId);

		const uint32_t periodMilliseconds = fplAtomicLoadU32(&audioSys->lastMixPeriodMilliseconds);
		const uint32_t idleMilliseconds = fplMax(periodMilliseconds * AudioMixerIdlePeriodCount, AudioMixerIdleMinimumMilliseconds);
		const fplMilliseconds nowMilliseconds = fplMillisecondsQuery();
		const uint32_t lastMixMilliseconds = fplAtomicLoadU32(&audioSys->lastMixMilliseconds);
		const uint32_t sinceLastMixMilliseconds = (uint32_t)nowMilliseconds - lastMixMilliseconds;
		const bool isMixerIdle = !hasMixed || (sinceLastMixMilliseconds > idleMilliseconds);
		const uint64_t waitedMilliseconds = nowMilliseconds - startMilliseconds;
		const bool hasWaitedTooLong = (waitedMilliseconds > idleMilliseconds);

		if(isMixerThread || isMixerIdle || hasWaitedTooLong) {
			const uint32_t previousOwner = TryAcquireMixerOwner(audioSys, AudioMixerOwner_Drain);
			if(previousOwner == AudioMixerOwner_None) {
				DrainAudioCommands(audioSys);
				ReleaseMixerOwner(audioSys);
				continue;
			}
		}
		WaitForAudioCommandStep(attemptIndex);
	}
}

// Sends a command that a function waits for before it frees memory, so it is never dropped: with a full queue it first waits for room
static void SendAudioCommandAndWait(AudioSystem *audioSys, const AudioCommand *command) {
	AudioCommandQueue *queue = &audioSys->commandQueue;
	uint32_t sequence = 0;
	for(;;) {
		const bool isSent = PushAudioCommand(queue, command, &sequence);
		if(isSent) {
			break;
		}
		const uint32_t queuedSequence = fplAtomicLoadU32(&queue->writeIndex);
		WaitForAudioCommands(audioSys, queuedSequence);
	}
	WaitForAudioCommands(audioSys, sequence);
}

// Sends a change of one play item when its id is live, false when it has ended or the queue is full (counted)
static bool SendPlayItemCommand(AudioSystem *audioSys, const AudioCommand *command) {
	const bool isLive = IsPlayItemLive(&audioSys->playItems, command->playId);
	if(!isLive) {
		return(false);
	}
	AudioCommandQueue *queue = &audioSys->commandQueue;
	const bool result = PushAudioCommand(queue, command, fpl_null);
	if(!result) {
		fplAtomicIncrementU32(&queue->droppedCount);
	}
	return(result);
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
	// Counted and copied under the same lock, another thread may add or remove a source in between
	fplMutexLock(&audioSys->sources.lock);
	size_t count = audioSys->sources.count;
	size_t result = count;
	if(dest != fpl_null) {
		if(count > maxDestCount) {
			result = 0; // Error, destination array to small
		} else {
			const AudioSource *src = audioSys->sources.first;
			AudioSource *dst = dest;
			while(src != fpl_null) {
				*dst = *src;
				src = src->next;
				++dst;
			}
		}
	}
	fplMutexUnlock(&audioSys->sources.lock);
	return(result);
}

fpl_extern size_t AudioSystemGetPlayItems(AudioSystem *audioSys, AudioPlayItem *dest, const size_t maxDestCount) {
	if(audioSys == fpl_null) {
		return(0);
	}
	// No lock: the count and the copies come from one pass over the published states, which are written without waiting for a reader
	AudioPlayItems *playItems = &audioSys->playItems;
	size_t result = 0;
	for(uint32_t slotIndex = 0; slotIndex < FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT; ++slotIndex) {
		AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
		const uint32_t flags = fplAtomicLoadU32(&slot->published.flags);
		if((flags & AudioPublishedPlayItemFlags_Live) == 0) {
			continue;
		}
		if(dest == fpl_null) {
			++result;
			continue;
		}
		AudioPlayItem copy;
		const bool isLive = CopyPublishedPlayItem(slot, slotIndex, &copy);
		if(!isLive) {
			// Ended between the two reads
			continue;
		}
		if(result >= maxDestCount) {
			return(0); // Error, destination array to small
		}
		dest[result] = copy;
		++result;
	}
	return(result);
}

fpl_extern bool AudioSystemStopOne(AudioSystem *audioSys, const AudioPlayItemID playId) {
	if(audioSys == fpl_null) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_Stop;
	command.playId = playId;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemVolume(AudioSystem *audioSys, const AudioPlayItemID playId, const float volume) {
	if(audioSys == fpl_null) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetVolume;
	command.playId = playId;
	command.value.volume = volume;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemPitch(AudioSystem *audioSys, const AudioPlayItemID playId, const float pitch) {
	if(audioSys == fpl_null || pitch <= 0.0f) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetPitch;
	command.playId = playId;
	command.value.pitch = pitch;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemRepeat(AudioSystem *audioSys, const AudioPlayItemID playId, const bool repeat) {
	if(audioSys == fpl_null) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetRepeat;
	command.playId = playId;
	command.value.isRepeat = repeat;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemPaused(AudioSystem *audioSys, const AudioPlayItemID playId, const bool paused) {
	if(audioSys == fpl_null) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetPaused;
	command.playId = playId;
	command.value.isPaused = paused;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemPosition(AudioSystem *audioSys, const AudioPlayItemID playId, const AudioFrameIndex frameIndex) {
	if(audioSys == fpl_null) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetPosition;
	command.playId = playId;
	command.value.frameIndex = frameIndex;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern bool AudioSystemSetPlayItemLoop(AudioSystem *audioSys, const AudioPlayItemID playId, const AudioFrameIndex loopStartFrame, const AudioFrameIndex loopEndFrame) {
	if(audioSys == fpl_null) {
		return false;
	}
	// The frame count of the source is checked by the mixer
	const bool isCleared = (loopEndFrame == 0);
	if(!isCleared && loopStartFrame >= loopEndFrame) {
		return false;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_SetLoop;
	command.playId = playId;
	command.value.loop.startFrame = loopStartFrame;
	command.value.loop.endFrame = loopEndFrame;
	const bool result = SendPlayItemCommand(audioSys, &command);
	return(result);
}

fpl_extern AudioPlayItemID AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume, const float pitch) {
	if((audioSys == fpl_null) || (source == fpl_null)) {
		AudioPlayItemID empty = fplZeroInit;
		return(empty);
	}

	// The streams that ended since the last call give their memory back first
	ReclaimFinishedStreamInstances(audioSys);

	AudioPlayItems *playItems = &audioSys->playItems;
	uint32_t slotIndex = 0;
	AudioPlayItemSlot *slot = ReservePlayItemSlot(playItems, &slotIndex);
	if(slot == fpl_null) {
		AudioPlayItemID empty = fplZeroInit;
		return(empty);
	}

	// The slot is reserved, nobody else reads or writes its item until it is playing. A stream that ended in it after the reclaim above is freed now
	AudioStreamInstance *previousInstance = (AudioStreamInstance *)fplAtomicLoadPtr(&slot->streamInstance);
	if(previousInstance != fpl_null) {
		fplAtomicStorePtr(&slot->streamInstance, fpl_null);
		FreeStreamInstance(audioSys, previousInstance);
	}
	const bool isStream = IsStreamSource(source);
	if(isStream) {
		AudioStreamInstance *instance = CreateStreamInstance(audioSys, source);
		if(instance == fpl_null) {
			// Given back unused, its generation stays
			fplAtomicStoreU32(&slot->state, AudioPlayItemSlotState_Finished);
			AudioPlayItemID empty = fplZeroInit;
			return(empty);
		}
		fplAtomicStorePtr(&slot->streamInstance, instance);
	}

	const uint32_t generation = fplAtomicLoadU32(&slot->generation);
	const AudioPlayItemID result = MakePlayItemID(slotIndex, generation);
	AudioPlayItem *playItem = &slot->item;
	fplClearStruct(playItem);
	playItem->id = result;
	playItem->source = source;
	playItem->isRepeat = repeat;
	playItem->volume = volume;
	// A pitch of zero or below would consume no input and sound like nothing at all -- the one failure nobody
	// could debug from the outside. Treat it as the file as recorded.
	playItem->pitch = (pitch > 0.0f) ? pitch : 1.0f;
	playItem->isPaused = false;
	// The loop region of the source is the default of every play, one that does not fit the source is left out
	const bool hasLoopRegion = IsLoopRegionValid(source, source->loopStartFrame, source->loopEndFrame);
	if(hasLoopRegion) {
		playItem->loopStartFrame = source->loopStartFrame;
		playItem->loopEndFrame = source->loopEndFrame;
	}

	// AudioSystemGetPlayItems sees it from here on, before it is mixed
	PublishPlayItem(slot, true);

	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_Play;
	command.playId = result;
	AudioCommandQueue *queue = &audioSys->commandQueue;
	const bool isSent = PushAudioCommand(queue, &command, fpl_null);
	if(!isSent) {
		// The queue is full, the slot is given back. Its id never left this function, so the generation stays
		fplAtomicIncrementU32(&queue->droppedCount);
		PublishPlayItem(slot, false);
		AudioStreamInstance *unusedInstance = (AudioStreamInstance *)fplAtomicLoadPtr(&slot->streamInstance);
		if(unusedInstance != fpl_null) {
			fplAtomicStorePtr(&slot->streamInstance, fpl_null);
			FreeStreamInstance(audioSys, unusedInstance);
		}
		fplAtomicStoreU32(&slot->state, AudioPlayItemSlotState_Finished);
		AudioPlayItemID empty = fplZeroInit;
		return(empty);
	}

	return(result);
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

// Fills up to minOutputFrameCount frames, also when that is no multiple of the factor.
// copiesPlayed brings the copies of the first input frame that earlier calls mixed already and returns the copies of the frame the output stopped in.
// inputCount counts only the input frames whose copies are all out.
static AudioResampleResult AudioSimpleUpSampling(const AudioChannelIndex inChannelCount, const AudioHertz inSampleRate, const AudioHertz outSampleRate, const AudioFrameIndex minOutputFrameCount, const AudioFrameIndex maxInputFrameCount, const float *inSamples, float *outSamples, uint32_t *copiesPlayed) {
	// Simple Upsampling (2x, 4x, 6x, 8x etc.) -> Duplicate frames
	fplAssert(outSampleRate > inSampleRate);
	fplAssert((outSampleRate % inSampleRate) == 0);
	const uint32_t upsamplingFactor = outSampleRate / inSampleRate;
	const float *inSamplesF32 = inSamples;
	uint32_t copyIndex = *copiesPlayed;
	AudioResampleResult result = fplZeroInit;
	while(result.inputCount < maxInputFrameCount && result.outputCount < minOutputFrameCount) {
		// A pitch change can lower the factor below the copies a frame has already
		if(copyIndex < upsamplingFactor) {
			for(AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
				*outSamples++ = inSamplesF32[inChannelIndex];
			}
			++result.outputCount;
			++copyIndex;
		}
		if(copyIndex >= upsamplingFactor) {
			inSamplesF32 += inChannelCount;
			++result.inputCount;
			copyIndex = 0;
		}
	}
	*copiesPlayed = copyIndex;
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

// Only by the owner of the mixer, like RestorePlayStates
static void SavePlayStates(AudioSystem *audioSys) {
	AudioPlayItems *playItems = &audioSys->playItems;
	for(uint32_t activeIndex = 0; activeIndex < playItems->activeCount; ++activeIndex) {
		const uint32_t slotIndex = playItems->activeSlotIndices[activeIndex];
		AudioPlayItem *item = &playItems->slots[slotIndex].item;
		item->framesPlayed[1] = item->framesPlayed[0];
		item->isFinished[1] = item->isFinished[0];
		item->copiesPlayed[1] = item->copiesPlayed[0];
	}
}

static void RestorePlayStates(AudioSystem *audioSys) {
	AudioPlayItems *playItems = &audioSys->playItems;
	for(uint32_t activeIndex = 0; activeIndex < playItems->activeCount; ++activeIndex) {
		const uint32_t slotIndex = playItems->activeSlotIndices[activeIndex];
		AudioPlayItem *item = &playItems->slots[slotIndex].item;
		item->framesPlayed[0] = item->framesPlayed[1];
		item->isFinished[0] = item->isFinished[1];
		item->copiesPlayed[0] = item->copiesPlayed[1];
	}
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
* @param[in,out] outSamples  Destination interleaved F32 samples (frameCount * outChannels elements).
*                        Existing values are preserved and added to (+=).
*
* @return Returns the total number of output samples touched (frameCount * outChannels).
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
* @param[in] convFuncs          Format conversion dispatch table.
* @param[in] sourceData         Pointer to the first byte of source samples to convert.
* @param[in] sourceFrameCount   Number of source frames available starting at sourceData.
* @param[in] sourceChannels     Channel count of the source audio.
* @param[in] sourceFormat       Sample format of the source audio (U8, S16, S24, S32, F32).
* @param[out] dspInBuffer       Destination buffer for interleaved F32 output.
* @param[in] dspInMaxFrameCount Maximum frames the destination buffer can hold.
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
* @param[in] channels        Number of interleaved channels.
* @param[in] inRate          Input sample rate in Hz.
* @param[in] outRate         Output sample rate in Hz.
* @param[in] maxOutputFrames Maximum number of output frames the dspOut buffer can hold.
*                            Also limits how many frames we request from the resampler.
* @param[in] inputFrameCount Number of input frames available in dspIn.
* @param[in] dspIn           Source interleaved F32 samples.
* @param[out] dspOut         Destination interleaved F32 samples.
* @param[in,out] copiesPlayed Copies of the first input frame that were mixed already (frame duplication only), set to 0 by every other path.
*
* @return Returns an AudioResampleResult with .inputCount (frames consumed) and .outputCount (frames produced).
*         Both are zero if rates are invalid or no frames could be produced.
*
* @note Buffer safety:
*       For upsampling, the output stops at maxOutputFrames, inside an input frame when needed (copiesPlayed continues it).
*       For downsampling and SinC, maxOutputFrames is passed as the output limit.
*       Passthrough is clamped to min(inputFrameCount, maxOutputFrames).
*
* @warning AudioResampleInterleaved derives its outputCount via fplGetTargetAudioFrameCount(inFrameCount, inRate, outRate).
*          For 44100 <-> 48000 the round-trip converges exactly, but for arbitrary non-even ratios the result may still exceed minOutputFrameCount by 1 frame.
*          Callers MUST clamp the returned outputCount before using it to index into fixed-size buffers or subtract from unsigned frame counters.
*/
static AudioResampleResult ResampleChunk(const AudioChannelIndex channels, const AudioHertz inRate, const AudioHertz outRate, const AudioFrameIndex maxOutputFrames, const AudioFrameIndex inputFrameCount, const float *dspIn, float *dspOut, uint32_t *copiesPlayed) {
	AudioResampleResult result = fplZeroInit;

	if (channels == 0 || inRate == 0 || outRate == 0 || inputFrameCount == 0 || maxOutputFrames == 0) {
		return result;
	}

	// Only frame duplication continues inside a frame, a pitch change to another path starts the frame again
	const uint32_t copiesPlayedBefore = *copiesPlayed;
	*copiesPlayed = 0;

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
				*copiesPlayed = copiesPlayedBefore;
				result = AudioSimpleUpSampling(channels, inRate, outRate, maxOutputFrames, inputFrameCount, dspIn, dspOut, copiesPlayed);
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
* @brief Returns the sample rate the resampler is TOLD the source has, which is the only thing pitch changes.
* @param[in] sourceSampleRate The real sample rate of the source in Hz.
* @param[in] pitch The playback rate multiplier of the play item, see @ref AudioPlayItem::pitch.
* @return Returns the pitched sample rate, or the source rate UNCHANGED when the pitch is neutral, <= 0 or would round the rate to zero.
* @note A rate conversion is decided by the RATIO alone -- claiming a 44100 Hz source is 47187 Hz while the device stays at 48000 consumes the clip 7% faster and still lands exactly on the device rate, so pitch needs no stage of its own and the output rate never moves.
* @note A neutral pitch returns the source rate UNCHANGED (not merely something close to it), which is what keeps an unpitched clip on the memcpy passthrough or the even-ratio path instead of the SinC branch.
*/
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

// A seek of an mp3 stream lets the decoder start again this many bytes of main data before the frame in front of its target: the bit reservoir of MPEG 1 Layer III, up to 511 bytes back
static const size_t AudioStreamMP3PreRollMainDataSize = MAX_BITRESERVOIR_BYTES;
// What a frame has besides its main data at most: 4 bytes header, 32 bytes side information and 2 bytes CRC
static const size_t AudioStreamMP3FrameOverheadSize = 38;

// Decodes the next frame that has samples, like LoadMP3FromBuffer that skips a frame without samples and ends at a frame in another format. False at the end of the data. Only by the owner of the mixer
static bool DecodeNextMP3Frame(AudioStreamMP3 *mp3, const AudioSource *source) {
	const uint8_t *data = source->stream.data;
	const size_t dataSize = source->stream.dataSize;
	while(mp3->bytePosition < dataSize) {
		mp3dec_frame_info_t frameInfo;
		fplClearStruct(&frameInfo);
		const uint8_t *frameStart = data + mp3->bytePosition;
		const int remainingSize = (int)(dataSize - mp3->bytePosition);
		const int sampleCount = mp3dec_decode_frame(&mp3->decoder, frameStart, remainingSize, mp3->frameSamples, &frameInfo);
		if(frameInfo.frame_bytes == 0) {
			break;
		}
		mp3->bytePosition += (size_t)frameInfo.frame_bytes;
		if(sampleCount == 0) {
			continue;
		}
		const bool isSameFormat = (frameInfo.hz == (int)source->format.sampleRate) && (frameInfo.channels == (int)source->format.channels);
		if(!isSameFormat) {
			// The file ends here, as for LoadMP3FromBuffer
			mp3->bytePosition = dataSize;
			break;
		}
		mp3->frameSampleCount = (AudioFrameIndex)sampleCount;
		mp3->frameReadIndex = 0;
		return(true);
	}
	mp3->frameSampleCount = 0;
	mp3->frameReadIndex = 0;
	return(false);
}

// Copies the frames the decoder has decoded and decodes the next one whenever they are used up, fewer only at the end of the data. Only by the owner of the mixer
static AudioFrameIndex DeliverMP3Frames(AudioStreamMP3 *mp3, const AudioSource *source, int16_t *outSamples, const AudioFrameIndex frameCount) {
	const AudioChannelIndex channelCount = source->format.channels;
	AudioFrameIndex result = 0;
	while(result < frameCount) {
		if(mp3->frameReadIndex == mp3->frameSampleCount) {
			const bool isDecoded = DecodeNextMP3Frame(mp3, source);
			if(!isDecoded) {
				break;
			}
		}
		const AudioFrameIndex availableFrameCount = mp3->frameSampleCount - mp3->frameReadIndex;
		const AudioFrameIndex missingFrameCount = frameCount - result;
		const AudioFrameIndex copyFrameCount = fplMin(missingFrameCount, availableFrameCount);
		const mp3d_sample_t *copyStart = mp3->frameSamples + (size_t)mp3->frameReadIndex * channelCount;
		int16_t *copyTarget = outSamples + (size_t)result * channelCount;
		const size_t copySize = (size_t)copyFrameCount * channelCount * sizeof(int16_t);
		fplMemoryCopy(copyStart, copySize, copyTarget);
		mp3->frameReadIndex += copyFrameCount;
		result += copyFrameCount;
	}
	return(result);
}

/**
* @brief Lets an mp3 stream deliver the given source frame next.
* @return Returns false when the frame lies past the frames of the file or the decoder failed.
* @note A fresh decoder at a frame lacks the bit reservoir of the frames before and the overlap of the frame before. So it starts at frames before the target that hold the reservoir, decodes them and the frame in front of the target and drops them. From the target on it decodes as a decoder from the start of the file, bit for bit.
* @note Frame 0, a repeat, needs nothing before it: the decoder starts there as after its creation.
* @note Only by the owner of the mixer.
*/
static bool SeekMP3Stream(AudioStreamMP3 *mp3, const AudioSource *source, const AudioFrameIndex frameIndex) {
	const AudioSourceStream *stream = &source->stream;
	const uint32_t targetFrame = frameIndex / stream->frameSampleCount;
	if(targetFrame >= stream->frameOffsetCount) {
		return(false);
	}
	uint32_t startFrame = targetFrame;
	if(startFrame > 0) {
		--startFrame;
	}
	size_t preRollMainDataSize = 0;
	while(startFrame > 0 && preRollMainDataSize < AudioStreamMP3PreRollMainDataSize) {
		--startFrame;
		const size_t frameSize = stream->frameOffsets[startFrame + 1] - stream->frameOffsets[startFrame];
		preRollMainDataSize += (frameSize > AudioStreamMP3FrameOverheadSize) ? (frameSize - AudioStreamMP3FrameOverheadSize) : 0;
	}

	mp3dec_init(&mp3->decoder);
	mp3->bytePosition = stream->frameOffsets[startFrame];
	mp3->frameSampleCount = 0;
	mp3->frameReadIndex = 0;
	// One frame per decode, a frame that fails for its missing reservoir still keeps its main data for the next one
	for(uint32_t preRollFrame = startFrame; preRollFrame < targetFrame; ++preRollFrame) {
		mp3dec_frame_info_t frameInfo;
		fplClearStruct(&frameInfo);
		const uint8_t *frameStart = stream->data + mp3->bytePosition;
		const int remainingSize = (int)(stream->dataSize - mp3->bytePosition);
		mp3dec_decode_frame(&mp3->decoder, frameStart, remainingSize, mp3->frameSamples, &frameInfo);
		if(frameInfo.frame_bytes == 0) {
			return(false);
		}
		mp3->bytePosition += (size_t)frameInfo.frame_bytes;
	}

	const bool isDecoded = DecodeNextMP3Frame(mp3, source);
	const AudioFrameIndex skipFrameCount = frameIndex - targetFrame * stream->frameSampleCount;
	if(!isDecoded || skipFrameCount >= mp3->frameSampleCount) {
		return(false);
	}
	mp3->frameReadIndex = skipFrameCount;
	return(true);
}

// Lets the stream deliver its next frames, returns how many it delivered, 0 when it failed. Only by the owner of the mixer
static AudioFrameIndex DeliverStreamFrames(AudioStreamInstance *instance, const AudioSource *source, uint8_t *outSamples, const AudioFrameIndex frameCount) {
	AudioFrameIndex result = 0;
	switch(instance->kind) {
		case AudioStreamKind_Sine:
		{
			AudioGenerateSineWave(&instance->sineWave, outSamples, source->format.format, source->format.sampleRate, source->format.channels, frameCount);
			result = frameCount;
		} break;

		case AudioStreamKind_Vorbis:
		{
			// Fewer frames only at the end of the file, the decoder counts frames per channel as samples
			const int channelCount = (int)source->format.channels;
			const int sampleCount = (int)frameCount * channelCount;
			const int decodedFrameCount = stb_vorbis_get_samples_short_interleaved(instance->vorbis, channelCount, (short *)outSamples, sampleCount);
			result = (AudioFrameIndex)fplMax(0, decodedFrameCount);
		} break;

		case AudioStreamKind_MP3:
		{
			result = DeliverMP3Frames(instance->mp3, source, (int16_t *)outSamples, frameCount);
		} break;

		default:
			break;
	}
	return(result);
}

// Lets the stream deliver the given source frame next, false when it failed. Only by the owner of the mixer
static bool SeekStream(AudioStreamInstance *instance, const AudioSource *source, const AudioFrameIndex frameIndex) {
	bool result = false;
	switch(instance->kind) {
		case AudioStreamKind_Sine:
		{
			instance->sineWave.frameIndex = source->stream.sineWave.frameIndex + frameIndex;
			result = true;
		} break;

		case AudioStreamKind_Vorbis:
		{
			// Back to the start, a repeat for example, is cheap and leaves the decoder as right after opening
			int isSought = 0;
			if(frameIndex == 0) {
				isSought = stb_vorbis_seek_start(instance->vorbis);
			} else {
				isSought = stb_vorbis_seek(instance->vorbis, frameIndex);
			}
			result = (isSought != 0);
		} break;

		case AudioStreamKind_MP3:
		{
			result = SeekMP3Stream(instance->mp3, source, frameIndex);
		} break;

		default:
			break;
	}
	return(result);
}

// Moves samples to a lower address in the same ring, the ranges may overlap, which fplMemoryCopy does not allow
static void MoveRingSamplesDown(int16_t *target, const int16_t *source, const size_t sampleCount) {
	fplAssert(target <= source);
	for(size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
		target[sampleIndex] = source[sampleIndex];
	}
}

/**
* @brief Makes the ring of a stream hold the source frames [startFrameIndex, startFrameIndex + frameCount), what a source with samples has in memory.
* @param[in,out] instance The stream instance of the play item.
* @param[in] source The stream source.
* @param[in] startFrameIndex The source frame of the next chunk, the position of the play item.
* @param[in] frameCount Number of frames the chunk needs, the rest of the source but at most the ring.
* @return Returns the number of frames the ring holds from startFrameIndex on: frameCount, fewer when the stream ended before its length (a damaged file), 0 when it failed.
* @note Frames from startFrameIndex on that the ring holds already move to its front. A position behind the ring, where even downsampling steps up to factor - 1 frames past its input, lets the stream drop the frames in between. Any other position, a repeat for example, makes the stream seek.
* @note Only by the owner of the mixer.
*/
static AudioFrameIndex FillStreamRing(AudioStreamInstance *instance, const AudioSource *source, const AudioFrameIndex startFrameIndex, const AudioFrameIndex frameCount) {
	fplAssert(frameCount <= AudioStreamRingFrameCount);
	const AudioChannelIndex channelCount = source->format.channels;
	const size_t frameSize = fplGetAudioFrameSizeInBytes(source->format.format, channelCount);
	const AudioFrameIndex ringEndFrameIndex = instance->ringStartFrameIndex + instance->ringFrameCount;
	const bool isInRing = (startFrameIndex >= instance->ringStartFrameIndex) && (startFrameIndex <= ringEndFrameIndex);
	const bool isBehindRing = (startFrameIndex > ringEndFrameIndex) && ((startFrameIndex - ringEndFrameIndex) < AudioStreamRingFrameCount);
	if(isInRing) {
		const AudioFrameIndex consumedFrameCount = startFrameIndex - instance->ringStartFrameIndex;
		const AudioFrameIndex keptFrameCount = ringEndFrameIndex - startFrameIndex;
		if(consumedFrameCount > 0 && keptFrameCount > 0) {
			int16_t *ringStart = (int16_t *)instance->ringSamples;
			const int16_t *keptStart = ringStart + (size_t)consumedFrameCount * channelCount;
			const size_t keptSampleCount = (size_t)keptFrameCount * channelCount;
			MoveRingSamplesDown(ringStart, keptStart, keptSampleCount);
		}
		instance->ringFrameCount = keptFrameCount;
	} else if(isBehindRing) {
		// Delivered into the ring and dropped
		const AudioFrameIndex skipFrameCount = startFrameIndex - ringEndFrameIndex;
		const AudioFrameIndex skippedFrameCount = DeliverStreamFrames(instance, source, instance->ringSamples, skipFrameCount);
		instance->ringFrameCount = 0;
		if(skippedFrameCount != skipFrameCount) {
			return(0);
		}
	} else {
		instance->ringFrameCount = 0;
		const bool isSought = SeekStream(instance, source, startFrameIndex);
		if(!isSought) {
			return(0);
		}
	}
	instance->ringStartFrameIndex = startFrameIndex;

	while(instance->ringFrameCount < frameCount) {
		uint8_t *deliverTarget = instance->ringSamples + (size_t)instance->ringFrameCount * frameSize;
		const AudioFrameIndex missingFrameCount = frameCount - instance->ringFrameCount;
		const AudioFrameIndex deliveredFrameCount = DeliverStreamFrames(instance, source, deliverTarget, missingFrameCount);
		if(deliveredFrameCount == 0) {
			break;
		}
		instance->ringFrameCount += deliveredFrameCount;
	}
	const AudioFrameIndex result = fplMin(instance->ringFrameCount, frameCount);
	return(result);
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
* the loop continues. With a loop region a repeating item ends each chunk at the loop end
* at the latest and goes on at the loop start, after the frames past the loop end it does so at the end of the source.
*
* A stream item reads its chunk from its ring instead of the samples of the source, FillStreamRing gives it the same frames.
*
* @param[in,out] audioSys     Audio system (provides target format, DSP buffers, conversion table).
* @param[in,out] item         Play item to process. Modified in place (framesPlayed, isFinished).
* @param[in,out] streamInstance The stream instance of a stream item, null for any other item.
* @param[in] targetFrameCount Number of output frames to fill in the mixing buffer.
* @param[in,out] mixingBuffer Pointer to the start of the F32 mixing buffer for this item.
*                             Must have at least targetFrameCount * outChannels floats.
* @param[in] masterVolume     Master volume of this mix call, applied on top of the item volume.
*
* @return Returns the number of output frames actually produced and mixed into the buffer.
*
* @warning Buffer safety:
*   - dspInBuffer:  ConvertSourceChunkToF32 clamps to dspInBuffer.maxFrameCount.
*   - dspOutBuffer: ResampleChunk clamps output to min(outRemainingFrameCount, dspOutBuffer.maxFrameCount).
*   - mixingBuffer: outRemainingFrameCount counts down from targetFrameCount, preventing overflow.
*   - Source data:  Bounded by inTotalFrameCount - framesPlayed[0].
*/
static AudioFrameIndex ProcessSinglePlayItem(AudioSystem *audioSys, AudioPlayItem *item, AudioStreamInstance *streamInstance, const AudioFrameIndex targetFrameCount, float *mixingBuffer, const float masterVolume) {
	const AudioHertz outSampleRate = audioSys->targetFormat.sampleRate;
	const AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;

	const AudioSource *source = item->source;
	const AudioFormat *srcFormat = &source->format;
	const AudioBuffer *srcBuffer = &source->buffer;

	// An endless stream has no rest: every chunk can take the whole ring, nothing clamps its position and it never ends
	const bool isStream = (streamInstance != fpl_null);
	const bool isEndless = isStream && source->stream.isEndless;
	const AudioFrameIndex endlessRemainingFrameCount = UINT32_MAX;

	// The PITCHED rate, which is the source's own whenever the item is not pitched -- see ResolvePitchedSampleRate.
	const AudioHertz inSampleRate = ResolvePitchedSampleRate(srcFormat->sampleRate, item->pitch);
	const AudioFrameIndex inTotalFrameCount = srcBuffer->frameCount;
	const AudioChannelIndex inChannelCount = srcFormat->channels;
	const fplAudioFormatType inFormat = srcFormat->format;
	const size_t inBytesPerSample = fplGetAudioSampleSizeInBytes(inFormat);
	const size_t inBytesPerFrame = (size_t)inChannelCount * inBytesPerSample;

	const float volume = item->volume * masterVolume;

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
		fplAssert(isEndless || (inStartFrameIndex < inTotalFrameCount));
		fplAssert(!isEndless || (item->loopEndFrame == 0));

		// A repeating item inside its loop region plays up to the loop end, never into the frames after it. From the loop end on it plays to the end of the source
		const bool isInsideLoopRegion = item->isRepeat && (item->loopEndFrame > 0) && (inStartFrameIndex < item->loopEndFrame);
		const AudioFrameIndex inEndFrameIndex = isInsideLoopRegion ? item->loopEndFrame : inTotalFrameCount;

		const AudioFrameIndex inRemainingFrameCount = isEndless ? endlessRemainingFrameCount : (inEndFrameIndex - inStartFrameIndex);

		// The frames from the position on: in memory, or in the ring of a stream
		const uint8_t *inSourceData;
		AudioFrameIndex inAvailableFrameCount;
		if (isStream) {
			const AudioFrameIndex chunkFrameCount = fplMin(inRemainingFrameCount, AudioStreamRingFrameCount);
			inAvailableFrameCount = FillStreamRing(streamInstance, source, inStartFrameIndex, chunkFrameCount);
			if (inAvailableFrameCount == 0) {
				// The stream failed, the item ends as at the end of its source
				fplAtomicIncrementU32(&audioSys->failedStreamCount);
				item->isFinished[0] = true;
				break;
			}
			inSourceData = streamInstance->ringSamples;
		} else {
			inAvailableFrameCount = inRemainingFrameCount;
			inSourceData = srcBuffer->samples + (size_t)inStartFrameIndex * inBytesPerFrame;
		}

		// ---- Step 1: Convert source samples to interleaved F32 (DSP-In) ----
		// Always write to the start of the DSP-in buffer (scratch space, reused each iteration)
		float *dspIn = (float *)audioSys->dspInBuffer.samples;
		const AudioFrameIndex convertedFrameCount = ConvertSourceChunkToF32(
			&audioSys->conversionFuncs,
			inSourceData,
			inAvailableFrameCount,
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

		const uint32_t copiesPlayedBefore = item->copiesPlayed[0];
		const AudioResampleResult resampleResult = ResampleChunk(
			inChannelCount,
			inSampleRate,
			outSampleRate,
			maxOutputForThisChunk,
			convertedFrameCount,
			dspIn,
			dspOut,
			&item->copiesPlayed[0]
		);

		// Frame skipping (AudioSimpleDownSampling) steps up to factor-1 frames past its input, which keeps the skipping grid across chunks, but never past the end of the source or the loop end
		const AudioFrameIndex playedFrameCount = fplMin(resampleResult.inputCount, inRemainingFrameCount);

		// Clamp output frame count to what we actually need. AudioResampleInterleaved
		// can produce 1 extra frame due to rounding when computing outFrameCount from
		// inFrameCount * (outRate/inRate). Without this clamp, the unsigned subtraction
		// outRemainingFrameCount -= outputFrameCount would underflow, causing the loop
		// to run far past the mixing buffer boundary.
		const AudioFrameIndex outputFrameCount = fplMin(resampleResult.outputCount, outRemainingFrameCount);

		// If resampling could not produce any frames (e.g. not enough input for SinC), stop. Frame duplication can output copies without finishing an input frame, which still moves on
		const bool hasCopiesMoved = (item->copiesPlayed[0] != copiesPlayedBefore);
		if (outputFrameCount == 0 || (playedFrameCount == 0 && !hasCopiesMoved)) {
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
		// The position of an endless stream wraps around after 2^32 frames
		item->framesPlayed[0] += playedFrameCount;
		fplAssert(isEndless || (item->framesPlayed[0] <= inEndFrameIndex));

		if (!isEndless && (item->framesPlayed[0] == inEndFrameIndex)) {
			if (item->isRepeat) {
				item->isFinished[0] = false;
				// The next pass starts at the loop start, which is 0 without a loop region
				item->framesPlayed[0] = item->loopStartFrame;
				item->copiesPlayed[0] = 0;
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
* @param[in,out] audioSys     Audio system state (buffers, format, play items list).
* @param[in] targetFrameCount Number of output frames to produce. Must be <= mixingBuffer.maxFrameCount.
* @param[in] advance          If true, finished (non-repeating) play items are ended and their slots freed.
*                             If false, play items are left in place (preview mode).
* @return Returns the maximum number of output frames produced by any single play item.
*                         This represents the valid range of the mixing buffer — frames beyond this
*                         are zero (the buffer is cleared at the start).
* @note                   Thread safety: Only by the owner of the mixer (AudioSystem::mixerOwner), which AudioSystemWriteFrames takes. No lock.
*/
static AudioFrameIndex WritePlayItemsToMixer2(AudioSystem *audioSys, const AudioFrameIndex targetFrameCount, const bool advance) {
	// The requested frame count must fit in the mixing buffer
	fplAssert(targetFrameCount <= audioSys->mixingBuffer.maxFrameCount);

	// Every item adds into the mixing buffer from its first frame on, so only the frames of this call need zeros.
	// The DSP buffers need none: each step reads only what the step before wrote for the same item.
	const AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;
	const size_t mixingByteCount = (size_t)targetFrameCount * outChannelCount * sizeof(float);
	fplMemoryClear(audioSys->mixingBuffer.samples, mixingByteCount);

#define GENSINEWAVE 0

#if GENSINEWAVE == 1
	AudioGenerateSineWave(&audioSys->tempWaveData, audioSys->mixingBuffer.samples, fplAudioFormatType_F32, outSampleRate, outChannelCount, targetFrameCount);
	AudioFrameIndex result = targetFrameCount;
	return result;
#else
	float *mixingBuffer = (float *)audioSys->mixingBuffer.samples;
	AudioFrameIndex maxOutputFrameCount = 0;

	// Every item of this call gets the same master volume, even when another thread changes it meanwhile
	const uint32_t masterVolumeBits = fplAtomicLoadU32(&audioSys->masterVolumeBits);
	const float masterVolume = AudioBitsToFloat(masterVolumeBits);

	AudioPlayItems *playItems = &audioSys->playItems;

	uint32_t activeIndex = 0;
	while (activeIndex < playItems->activeCount) {
		const uint32_t slotIndex = playItems->activeSlotIndices[activeIndex];
		AudioPlayItemSlot *slot = &playItems->slots[slotIndex];
		AudioPlayItem *item = &slot->item;
		AudioStreamInstance *streamInstance = (AudioStreamInstance *)fplAtomicLoadPtr(&slot->streamInstance);

		// Skip items that were already finished (can happen in advance=false mode)
		if (item->isFinished[0]) {
			++activeIndex;
			continue;
		}

		// A PAUSED item is skipped the same way, but it is never ended and its framesPlayed is left alone, so unpausing continues from where it stopped
		if (item->isPaused) {
			++activeIndex;
			continue;
		}

		// A preview leaves a stream alone, rewinding it afterwards would cost a seek
		if (!advance && (streamInstance != fpl_null)) {
			++activeIndex;
			continue;
		}

		// Process this play item through the full pipeline
		const AudioFrameIndex itemOutputFrames = ProcessSinglePlayItem(audioSys, item, streamInstance, targetFrameCount, mixingBuffer, masterVolume);
		if (itemOutputFrames > maxOutputFrameCount) {
			maxOutputFrameCount = itemOutputFrames;
		}

		if (item->isFinished[0]) {
			item->framesPlayed[0] = 0;
			item->copiesPlayed[0] = 0;
			if (advance) {
				// The next item moves up to this index
				FinishActivePlayItem(playItems, activeIndex);
				continue;
			}
		}

		++activeIndex;
	}

	// The new positions become visible to AudioSystemGetPlayItems, a preview leaves everything as it was
	if (advance) {
		for (uint32_t publishIndex = 0; publishIndex < playItems->activeCount; ++publishIndex) {
			const uint32_t slotIndex = playItems->activeSlotIndices[publishIndex];
			PublishPlayItem(&playItems->slots[slotIndex], true);
		}
	}

	return maxOutputFrameCount;
#endif
}

/**
* @brief Fills the conversion buffer with mixed/converted samples from the audio sources of the play items.
*
* @param[in,out] audioSys Audio system state (buffers, format, play items list).
* @param[in] maxFrameCount Maximum number of output frames that can be produced.
* @param[in] advance If true, finished (non-repeating) play items are removed from the list.
*                    If false, play items are left in place (preview mode).
* @return Returns always maxFrameCount, because the remaining frames are filled with zero.
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

	const size_t outputFrameSize = fplGetAudioFrameSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels);
	const AudioMixerOwner requestedOwner = advance ? AudioMixerOwner_Mixing : AudioMixerOwner_Preview;
	const uint32_t previousOwner = TryAcquireMixerOwner(audioSys, requestedOwner);
	if(previousOwner != AudioMixerOwner_None) {
		// Someone else mixes or applies commands: silence, nothing advances and nobody waits
		if(advance && previousOwner == AudioMixerOwner_Mixing) {
			fplAtomicIncrementU32(&audioSys->concurrentWriteCount);
			fplAssert(!"AudioSystemWriteFrames was called with advance from two threads at the same time");
		}
		fplMemoryClear(outSamples, outputFrameSize * frameCount);
		return(frameCount);
	}

	if(advance) {
		// Tells a waiting function which thread mixes and whether it still runs
		const uint64_t threadId = fplGetCurrentThreadId();
		const fplMilliseconds nowMilliseconds = fplMillisecondsQuery();
		const uint64_t sampleRate = audioSys->targetFormat.sampleRate;
		const uint64_t periodMilliseconds = ((uint64_t)frameCount * AudioMillisecondsPerSecond + sampleRate - 1) / sampleRate;
		fplAtomicStorePtr(&audioSys->lastMixThread, (void *)(uintptr_t)threadId);
		fplAtomicStoreU32(&audioSys->lastMixMilliseconds, (uint32_t)nowMilliseconds);
		fplAtomicStoreU32(&audioSys->lastMixPeriodMilliseconds, (uint32_t)periodMilliseconds);
	}

	// What the other threads sent takes effect from this call on
	DrainAudioCommands(audioSys);

	const uint32_t isSuspended = fplAtomicLoadU32(&audioSys->isSuspended);
	if(isSuspended != 0) {
		// Silence, and nothing else touched -- no play item advanced, no source consumed.
		// That is what makes this a pause: the mixer picks up exactly where it left off.
		fplMemoryClear(outSamples, outputFrameSize * frameCount);
		ReleaseMixerOwner(audioSys);
		return(frameCount);
	}

	if(!advance) {
		SavePlayStates(audioSys);
	}

	AudioFrameIndex result = 0;

	size_t outputSampleStride = outputFrameSize;
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

	ReleaseMixerOwner(audioSys);

	return result;
}

fpl_extern void AudioSystemStopAll(AudioSystem *audioSys) {
	if(audioSys == fpl_null || !audioSys->isInitialized) {
		return;
	}
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_StopAll;
	SendAudioCommandAndWait(audioSys, &command);
	ReclaimFinishedStreamInstances(audioSys);
}

// Frees a source with its samples, and with the frame index of an mp3 stream
static void FreeAudioSourceMemory(AudioMemory *memory, AudioSource *source) {
	FreeAudioBuffer(memory, &source->buffer);
	if(source->stream.frameOffsets != fpl_null) {
		FreeAudioMemory(memory, (void *)source->stream.frameOffsets);
		source->stream.frameOffsets = fpl_null;
	}
	FreeAudioMemory(memory, source);
}

fpl_extern bool AudioSystemRemoveSource(AudioSystem *audioSys, AudioSource *source) {
	if(audioSys == fpl_null || source == fpl_null || !audioSys->isInitialized) {
		return false;
	}

	AudioMemory *memory = &audioSys->memory;
	AudioSources *sources = &audioSys->sources;

	// Play items first: one still pointing at this source would read freed samples on the very next mix
	AudioCommand command = fplZeroInit;
	command.kind = AudioCommandKind_StopSource;
	command.value.source = source;
	SendAudioCommandAndWait(audioSys, &command);

	// The streams of the stopped items are freed before the source they play
	ReclaimFinishedStreamInstances(audioSys);

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
		FreeAudioSourceMemory(memory, source);
	}

	return(result);
}

// Frees every source, only once no play item is left that could read one
static void FreeAllAudioSources(AudioSystem *audioSys) {
	AudioMemory *memory = &audioSys->memory;
	AudioSources *sources = &audioSys->sources;

	fplMutexLock(&sources->lock);
	AudioSource *source = sources->first;
	while(source != fpl_null) {
		AudioSource *next = source->next;
		FreeAudioSourceMemory(memory, source);
		source = next;
	}
	sources->first = sources->last = fpl_null;
	// The count was left standing here, so every source query after a clear reported the sources that were just freed
	sources->count = 0;
	fplMutexUnlock(&sources->lock);
}

fpl_extern void AudioSystemClearSources(AudioSystem *audioSys) {
	if(audioSys == fpl_null || !audioSys->isInitialized) {
		return;
	}

	// A play item outliving the source it points at would read freed samples on the next mix, so nothing is left playing
	AudioSystemStopAll(audioSys);
	FreeAllAudioSources(audioSys);
}

fpl_extern bool AudioSystemGetTargetFormat(const AudioSystem *audioSys, AudioFormat *outFormat) {
	if(audioSys == fpl_null || outFormat == fpl_null)
		return(false);
	*outFormat = audioSys->targetFormat;
	return(true);
}

// Counts the added sources, the bytes of their samples, the stream sources and their compressed bytes under sources.lock
static size_t CountAudioSources(AudioSystem *audioSys, size_t *outByteCount, size_t *outStreamSourceCount, size_t *outCompressedByteCount) {
	AudioSources *sources = &audioSys->sources;
	fplMutexLock(&sources->lock);
	size_t sourceCount = 0;
	size_t byteCount = 0;
	size_t streamSourceCount = 0;
	size_t compressedByteCount = 0;
	AudioSource *source = sources->first;
	while(source != fpl_null) {
		++sourceCount;
		byteCount += source->buffer.bufferSize;
		if(IsStreamSource(source)) {
			++streamSourceCount;
			compressedByteCount += source->stream.dataSize;
		}
		source = source->next;
	}
	fplMutexUnlock(&sources->lock);
	*outByteCount = byteCount;
	*outStreamSourceCount = streamSourceCount;
	*outCompressedByteCount = compressedByteCount;
	return(sourceCount);
}

fpl_extern size_t AudioSystemGetSourceStats(AudioSystem *audioSys, size_t *outByteCount) {
	if(outByteCount != fpl_null)
		*outByteCount = 0;
	if(audioSys == fpl_null || audioSys->isShutdown)
		return(0);

	size_t byteCount = 0;
	size_t streamSourceCount = 0;
	size_t compressedByteCount = 0;
	size_t sourceCount = CountAudioSources(audioSys, &byteCount, &streamSourceCount, &compressedByteCount);

	if(outByteCount != fpl_null)
		*outByteCount = byteCount;
	return(sourceCount);
}

fpl_extern bool AudioSystemGetStats(AudioSystem *audioSys, AudioSystemStats *outStats) {
	if(outStats == fpl_null) {
		return(false);
	}
	fplClearStruct(outStats);
	if(audioSys == fpl_null || audioSys->isShutdown) {
		return(false);
	}
	size_t pcmByteCount = 0;
	size_t streamSourceCount = 0;
	size_t compressedByteCount = 0;
	size_t sourceCount = CountAudioSources(audioSys, &pcmByteCount, &streamSourceCount, &compressedByteCount);
	size_t playItemCount = AudioSystemGetPlayItems(audioSys, fpl_null, 0);
	outStats->sourceCount = sourceCount;
	outStats->pcmByteCount = pcmByteCount;
	outStats->activePlayItemCount = (uint32_t)playItemCount;
	outStats->maxPlayItemCount = FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT;
	outStats->droppedCommandCount = fplAtomicLoadU32(&audioSys->commandQueue.droppedCount);
	outStats->concurrentWriteCount = fplAtomicLoadU32(&audioSys->concurrentWriteCount);
	outStats->streamSourceCount = streamSourceCount;
	outStats->compressedByteCount = compressedByteCount;
	outStats->streamInstanceCount = fplAtomicLoadU32(&audioSys->streamInstanceCount);
	outStats->streamInstanceByteCount = fplAtomicLoadU32(&audioSys->streamInstanceByteCount);
	outStats->failedStreamCount = fplAtomicLoadU32(&audioSys->failedStreamCount);
	return(true);
}

fpl_extern void AudioSystemShutdown(AudioSystem *audioSys) {
	if(audioSys == fpl_null || !audioSys->isInitialized) {
		return;
	}
	// Taken first and held until the next AudioSystemInit: a mixer that still runs gets silence and touches nothing that is freed here, so no command has to wait for it
	for(uint32_t attemptIndex = 0;; ++attemptIndex) {
		const uint32_t previousOwner = TryAcquireMixerOwner(audioSys, AudioMixerOwner_Shutdown);
		if(previousOwner == AudioMixerOwner_None) {
			break;
		}
		WaitForAudioCommandStep(attemptIndex);
	}

	// The commands sent before come first, so an item that was just started ends as well. AudioSystemStopAll and AudioSystemClearSources would wait for the mixer that is held here
	DrainAudioCommands(audioSys);
	AudioCommand stopAllCommand = fplZeroInit;
	stopAllCommand.kind = AudioCommandKind_StopAll;
	ApplyAudioCommand(&audioSys->playItems, &stopAllCommand);
	ReclaimFinishedStreamInstances(audioSys);
	FreeAllAudioSources(audioSys);

	FreeAudioStream(&audioSys->memory, &audioSys->conversionBuffer);

	fplMutexDestroy(&audioSys->commandQueue.producerLock);
	fplMutexDestroy(&audioSys->sources.lock);

	audioSys->isInitialized = false;
	audioSys->isShutdown = true;
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