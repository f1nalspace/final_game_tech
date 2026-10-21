/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio Player

Description:
	This demo shows how to play music, sounds using a custom audio system/mixer.

	The audio tracks are streamed in and use a slow/fast detection to only cache when it needs to, see AudioStreamingThread() for more details.
	In addition all samples are cached in a lock-free ringbuffer and are played back properly, see AudioPlayback() for more details.

	To make it more appealing all audio samples are visualized with OpenGL and uses several algorythms, such FFT, Windowing, Smoothing, etc.
	This can be shown from the full audio buffer, or in realtime that is filled directly in the streaming thread.

	Resampling is fully supported for any sample rates

	The following formats are supports:
	- PCM Wave (uncompressed)
	- OGG Vorbis (S16 output)
	- MP3 (S16 output)

	The demo is very complex and requires a good understanding how digital sound is played back in a computer.

How the demo works:
	# Audio System

	To make playback of any sort of audio stream possible, there is a rather complex system under the hood that does the following:

	- Loading audio sources (Wave, Ogg, MP3) from either file or data streams into PCM format, that consists of infos such sample rate, number of channels, data type and the sample data itself
	- Separating of audio track and audio sources
	- Converting audio samples into floating point, that is used for the mixing and resampling
	- Resampling of audio samples by either (Simple up/down sampling or SinC based resampling)
	- Applying effects per audio track (Volume, Filters, etc.)
	- Mixing of multiple audio samples from input channels into output channels
	- Converting back the mixed samples into the target sample format
	- Caching of audio samples into chunks to increase performance

	# Audio playback thread

	The thread function AudioPlayback() is responsible for filling out N audio frames/samples of the audio device playback period buffer.
	This may be called thousands of times per second, therefore has a very tight time-frame and we can't use any I/O or locking operations there. 
	The samples are lock-free read from a ring-buffer that is continuosly filled by the streaming thread.

	# Audio streaming thread

	The thread function AudioStreamingThread() is filling a ring buffer that will be used in the audio playback thread, that eventually plays back the audio samples.

	It also detects new audio tracks that needs to be loaded and loads them automatically.
	It alone changes the track list: drag & drop only leaves a request, and at a track switch the device callback empties the output ring, so the ring keeps one producer and one consumer.

	There is a audio buffer that is fully loaded once, that is used for visualizing the samples.
	This may take a while, depending on the input/target sample rates and the length of the source tracks.

	In addition there is a smart algorythm, that detects the playback latency and adjusts the amount and duration of the ring-buffer that needs to be filled.
	This is important, because slow hardware requires more buffering than fast hardware - but a fixed buffer size is not sufficient enough.

	The samples ring-buffer are filled by the AudioSystemWriteFrames() function, which may advanced the audio system play cursor.

	# Rendering

	All rendering is done using oldschool style OpenGL 1.x.

	The converted mono samples are visualized by either:
	- Fast Fourier Transformation
	- Pure sample drawing
	- Line drawing
	- Spectrum analysis (Incomplete)

	The ring-buffer is visualized as simple bars with a tail and head position.

	# Command line

	FPL_AudioPlayer [options] [file ...]

	Every argument that starts with -- is an option, every other one is a file to play. --help prints all options.

	--mixer chooses where the audio system mixes:
	- thread (default): the streaming thread mixes into the output ring, the device callback only copies from it
	- callback: the device callback mixes itself, like most programs do. There is no output ring and no full-track visualization buffer then, the visualization always uses the real-time samples
	- device-sine: the device callback generates a sine wave, no audio system involved

	--device=<name> plays on the audio device with exactly this name and stops with an error when there is none, it never falls back to another device.
	Without it, no target device is given and the platform plays on the system default device.

	--sine plays only the generated sine wave, --sine-seconds sets its length.
	--volume, --pitch and --repeat apply to the play item of every track, the sine wave included.
	--play-mode chooses what follows a track that has played to its end: off (silence), list (the next track, silence after the last one, the default) or loop (after the last one the first again).
	A track ends when it is heard to its end, in thread mode that is after the latency of the output ring. A repeating track never ends.
	In thread mode every change is heard only after the latency of the output ring, --ring-ms makes the ring smaller.

	--render mixes the first track on the main thread without window and audio device, always --chunk frames per AudioSystemWriteFrames call, and prints one result line:
	render: file=<name> rate=<hz> format=<fmt> channels=<n> chunk=<n> frames=<n> hash=<fnv1a64> mix_mean_us=<f> mix_p99_us=<f> mix_max_us=<f> vm_hwm_kb=<n>
	The hash is FNV-1a 64 over the PCM bytes, the same options give the same hash on the same build. --render-wav=<file> writes the samples into a wave file as well.

	Keys in the window: N and P (and the media keys next/previous) switch to the next or previous track of the command line, Space (and media play/pause) pauses,
	R turns repeat on and off and Up/Down change the volume by 0.1, both also for the next tracks. M switches the play mode. In thread mode a key is heard only after the latency of the output ring.

	--stats prints a line every 5 seconds, the mixer values cover the calls since the line before and mix_tid is the kernel thread id of the mixing thread (for strace -p):
	stats: vm_hwm_kb=<n> vm_rss_kb=<n> sources=<n> source_bytes=<n> play_items=<n> ring_fill_pct=<f> mix_calls=<n> mix_mean_us=<f> mix_p99_us=<f> mix_max_us=<f> mix_tid=<n>

Known Issues / Limitations:
	- Visualization of FFT is incorrect
	- Displayed audio samples are not sync with actual samples that are output to the speaker, so its not syncronized

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Todo:
	- Smoother bars (Falling peaks, Max peak line -> https://www.youtube.com/watch?v=JU-uCytFvmI&list=UU56Qctnsu8wAyvzf4Yx6LIw&index=16)
	- Audio streaming from HTTP (requires networking in FPL!)
	- Multiple audio tracks

Changelog:
	## 2026-10-21
	- New: Command line options for the audio device, the mixer mode, the output ring size, the sine track and the play item, see --help
	- Changed: Without --device no target device is given, the platform plays on the system default device and follows it
	- Fixed: The generated sine track was played twice when it was started
	- Fixed: The temporary stream buffer was smaller than the output ring, which is rounded up to whole memory pages
	- New: Offline render mode (--render, --render-wav) that prints a hash of the mixed samples
	- New: --stats logs memory and mixer cost every 5 seconds
	- Changed: Drag & drop only leaves a request, the streaming thread switches the track and the device callback empties the output ring
	- Changed: --volume, --pitch and --repeat apply to the sine wave too
	- Fixed: Loaded tracks were never added to the audio system, so they were neither counted nor freed
	- Fixed: The full-track buffers of replaced tracks were never freed
	- New: Keys N/P and the media keys switch tracks, Space pauses, R toggles repeat, Up/Down change the volume
	- Fixed: The log line of a loaded file missed its sample rate
	- Changed: F1 logs the visualization, outside thread mode that it stays real-time
	- New: Play mode (--play-mode, key M), by default the next track follows until the last one
	- Fixed: The track position ran on during a pause
	- Fixed: Silence of up to a second after a track switch in thread mode
	- Changed: --stats takes its values from AudioSystemGetStats() and shows the play items
	- Changed: The end of a track is found by the id of its play item, other play items do not hold it up

	## 2026-08-23
	- Fixed: Every log line was written two or three times, because the log writer used the standard console, the error console and the debug output at the same time

	## 2026-08-07
	- Changed: Reflect for API change final_audiosystem.h (AudioSystemPlaySource takes a pitch, passing AudioSystemDefaultPitch)

	## 2026-05-14
	- Show audio backend name in title

	## 2026-04-03
	- Updated documentations

	## 2025-03-25
	- Added progressbar to indicate current position in the track
	- Fixed streamed/played frames was not reset on drag & drop

	## 2025-03-21
	- Compute the actual target frame count for the full audio buffer, so that we can up/down sample properly
	- Support for resampling from odd frequency ratios, such as 48000 <-> 41000 using SinC

	## 2025-03-15
	- Support for swapping out the audio track by drag & drop another file

	## 2025-03-09
	- Improved visualization a lot
	- Detect default audio device properly

	## 2025-03-08
	- Fixed crash when no audio track was loaded
	- Fixed sine wave streaming was totally broken

	## 2025-02-14
	- Get average sample value from stereo samples

	## 2023-05-28
	- Fixed crash/assert when audio visualization chunk is not ready yet

	## 2021-11-06
	- Proper FFT power spectrum and audio bin drawing

	## 2021-03-13
	- Samples and FFT visualization for realtime audio

	## 2021-03-04
	- Dynamic streaming delay based on latency and buffer percentage

	## 2021-03-02
	- Removed all broken sample rendering
	- Introduced audio buffering in preparation for real FFT

	## 2020-02-15
	- Basic FFT & spectrum rendering

	## 2019-10-31
	- Moved audio load code to the outside
	- Fixed release resources when exit

	## 2019-08-03
	- Print error when audio playback its too slow

	## 2019-07-19
	- Wave form visualization (Bars, Lines)
	- Moved sine wave generation out into final_audiosystem.h
	- Reflect api changes for FPL 0.9.4

	## 2019-05-22
	- Added support for playing multiple audio files from command line

	## 2018-12-06
	- Added basic support for loading IEEEFloat wave files

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2
	- Disable auto-play/stop of audio samples by default

	## 2018-06-06
	- Refactored files

	## 2018-05-13
	- Fixed Makefiles (Missing audiosystem.c)

	## 2018-05-09
	- Introduced audio system -> Prepare for mixing
	- Add support for audio format conversions

	## 2018-05-08
	- Simple Up/Down sampling
	- Add waveloader.c to makefiles
	- Use optional argument as wavefile to load

	## 2018-05-07
	- Introduce sample buffering
	- Written a simple wave file loader

	## 2018-05-05:
	- Fixed CMakeLists to compile properly
	- Fixed Makefile to compile properly

	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/


/*
Ideas:

Visualize the samples:

- Define a struct chunk which contains 128, 256, 512 samples + a frame counter, so we can convert it to a time
- Define a ringbuffer with a multiple of N-chunks with a total length of one second
- In the AudioStreamingThread convert the streamed samples to N-chunks and push it into the ring buffer
- In the rendering code we track the current time, regardless if audio is played or not or if window is visible or not
- Now for each frame, we look for one chunk which fits exactly in the current time, copy it and discard the chunk and all other chunks which are older (No need to keep chunks from previous frames)
- Use the copied chunk as FFT input and display the spectrum for it
*/

#define APP_TITLE "FPL Demo | Audio Player"

#ifndef FPL_LOGGING
#	define FPL_LOGGING
#endif
#ifndef FPL_IMPLEMENTATION
#	define FPL_IMPLEMENTATION
#endif
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define _USE_MATH_DEFINES
#include <math.h> // sinf, M_PI
#include <float.h>
#include <stddef.h> // offsetof
#include <stdlib.h> // strtod
#if defined(FPL_PLATFORM_LINUX)
#	include <unistd.h> // syscall
#	include <sys/syscall.h> // SYS_gettid
#endif

#ifndef FINAL_AUDIOSYSTEM_IMPLEMENTATION
#	define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#endif
#include <final_audiosystem.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <final_audiodemo.h>

#include <final_math.h>

#include <final_utils.h>

#define FINAL_BUFFER_IMPLEMENTATION
#include <final_buffer.h>

#include <final_music.h>

#define MAX_AUDIO_FRAMES_CHUNK_FRAMES 256
typedef struct AudioFramesChunk {
	uint8_t samples[MAX_AUDIO_FRAMES_CHUNK_FRAMES * AUDIO_MAX_CHANNEL_COUNT * AUDIO_MAX_SAMPLESIZE];
	AudioFrameIndex index;
	AudioFrameIndex count;
} AudioFramesChunk;

static const int AudibleFrequencyRanges[] = {
	20,
	60,
	250,
	500,
	2000,
	4000,
	6000,
	20000
};

#define MAX_AUDIO_BIN_COUNT 32

typedef enum SpectrumMode {
	SpectrumMode_Natural = 0,
	SpectrumMode_Exponential,
	SpectrumMode_MultiPeakScale,
	SpectrumMode_MaxPeakScale,
	SpectrumMode_Count,
} SpectrumMode;

typedef struct AudioVisualization {
	AudioFramesChunk videoAudioChunks[2]; // 0 = Render, 1 = New
	FFTDouble fftInput[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	FFTDouble fftOutput[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	float rawSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES * AUDIO_MAX_CHANNEL_COUNT];
	float monoSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double currentSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double lastSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double currentMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double lastMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double scaledMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double scaledSamples[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double windowCoeffs[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double rawMagnitudes[MAX_AUDIO_FRAMES_CHUNK_FRAMES];
	double spectrum[MAX_AUDIO_BIN_COUNT];
	double peakSpectrum[MAX_AUDIO_BIN_COUNT];
	double globalPeak;
	double bins[MAX_AUDIO_BIN_COUNT + 1];
	volatile uint32_t hasVideoAudioChunk;
} AudioVisualization;

// Where the audio system mixes, chosen with --mixer
typedef enum PlaybackMode {
	// The streaming thread mixes into the output ring, the device callback only copies from it
	PlaybackMode_Thread = 0,
	// The device callback mixes itself
	PlaybackMode_Callback,
	// The device callback generates a sine wave, no audio system involved
	PlaybackMode_DeviceSine,

	PlaybackMode_Count,
} PlaybackMode;

// What follows a track that has played to its end, chosen with --play-mode and switched with M. A repeating track never ends.
typedef enum PlayMode {
	// Silence after the track, only N/P switch
	PlayMode_Off = 0,
	// The next track of the command line, silence after the last one
	PlayMode_List,
	// The next track, after the last one the first again
	PlayMode_Loop,

	PlayMode_Count,
} PlayMode;

// One name of a choice option, also used for the log
typedef struct PlayerOptionChoice {
	const char *name;
	const char *description;
} PlayerOptionChoice;

// Same order as PlayMode
static const PlayerOptionChoice PlayModeChoices[] = {
	{ "off", "silence after every track, only N/P switch" },
	{ "list", "the next track of the command line, silence after the last one" },
	{ "loop", "the next track, after the last one the first again" },
};
fplStaticAssert(fplArrayCount(PlayModeChoices) == PlayMode_Count);

// Only the streaming thread: how far the current track is from its end, for the play mode
typedef enum TrackEndState {
	// The mixer still plays the current track
	TrackEndState_Playing = 0,
	// The mixer is done, the end is heard once numFramesPlayed reaches trackEndFrameCount
	TrackEndState_Mixed,
	// Heard to its end, and the play mode keeps the silence
	TrackEndState_Stopped,
} TrackEndState;

// Output sample format of the offline render, chosen with --format
typedef enum RenderFormat {
	RenderFormat_S16 = 0,
	RenderFormat_F32,

	RenderFormat_Count,
} RenderFormat;

// Same order as RenderFormat
static const fplAudioFormatType RenderFormatTypes[] = {
	fplAudioFormatType_S16,
	fplAudioFormatType_F32,
};

#define MAX_PLAYER_FILE_COUNT 8

typedef struct PlayerOptions {
	char deviceName[FPL_MAX_NAME_LENGTH];
	char renderWavePath[FPL_MAX_PATH_LENGTH];
	const char *files[MAX_PLAYER_FILE_COUNT];
	size_t fileCount;
	int32_t mixerMode;
	int32_t playMode;
	int32_t ringMilliseconds;
	int32_t renderChunkFrameCount;
	int32_t renderSampleRate;
	int32_t renderFormat;
	int32_t renderChannelCount;
	float renderSeconds;
	float sineSeconds;
	float volume;
	float pitch;
	bool isSine;
	bool isRepeat;
	bool isRender;
	bool isStats;
	bool isHelp;
} PlayerOptions;

// The sine wave of --sine and of the device sine
static AudioSineWaveData MakeSineWave(const float seconds) {
	const AudioHertz sineFrequency = 440;
	const double sineToneVolume = 0.25;
	AudioSineWaveData result = fplZeroInit;
	result.frequency = sineFrequency;
	result.toneVolume = sineToneVolume;
	result.duration = seconds;
	return(result);
}

// Cost of AudioSystemWriteFrames calls in 1 us steps up to 10 ms, the last bucket counts every call above
#define MIX_COST_BUCKET_COUNT 10000

typedef struct MixCostHistogram {
	uint32_t buckets[MIX_COST_BUCKET_COUNT + 1];
	uint64_t callCount;
	double totalMicroseconds;
	double maxMicroseconds;
} MixCostHistogram;

static const double MicrosecondsPerSecond = 1000000.0;

static void RecordMixCost(MixCostHistogram *histogram, const double microseconds) {
	size_t bucketIndex = MIX_COST_BUCKET_COUNT;
	if(microseconds < (double)MIX_COST_BUCKET_COUNT) {
		bucketIndex = (size_t)microseconds;
	}
	++histogram->buckets[bucketIndex];
	++histogram->callCount;
	histogram->totalMicroseconds += microseconds;
	histogram->maxMicroseconds = fplMax(histogram->maxMicroseconds, microseconds);
}

static double GetMixCostMean(const MixCostHistogram *histogram) {
	double result = 0.0;
	if(histogram->callCount > 0) {
		result = histogram->totalMicroseconds / (double)histogram->callCount;
	}
	return(result);
}

// Upper edge of the bucket that reaches the given share of all calls, never above the maximum
static double GetMixCostPercentile(const MixCostHistogram *histogram, const double share) {
	if(histogram->callCount == 0) {
		return(0.0);
	}
	double neededShareCount = ceil(share * (double)histogram->callCount);
	uint64_t neededCount = (uint64_t)neededShareCount;
	uint64_t count = 0;
	for(size_t bucketIndex = 0; bucketIndex < MIX_COST_BUCKET_COUNT; ++bucketIndex) {
		count += histogram->buckets[bucketIndex];
		if(count >= neededCount) {
			double bucketUpperEdge = (double)(bucketIndex + 1);
			return(fplMin(bucketUpperEdge, histogram->maxMicroseconds));
		}
	}
	return(histogram->maxMicroseconds);
}

// FNV-1a 64-bit, the hash of the rendered samples
static const uint64_t Fnv1a64OffsetBasis = 0xcbf29ce484222325ULL;
static const uint64_t Fnv1a64Prime = 0x100000001b3ULL;

static uint64_t HashFnv1a64(const uint64_t hash, const void *data, const size_t size) {
	const uint8_t *bytes = (const uint8_t *)data;
	uint64_t result = hash;
	for(size_t byteIndex = 0; byteIndex < size; ++byteIndex) {
		result ^= bytes[byteIndex];
		result *= Fnv1a64Prime;
	}
	return(result);
}

#define MIX_COST_HISTOGRAM_COUNT 2

typedef enum MixCostSwapState {
	MixCostSwapState_Idle = 0,
	// The main thread asks the mixing thread to swap the histograms
	MixCostSwapState_Requested,
	// The mixing thread swapped, the other histogram holds the calls since the swap before
	MixCostSwapState_Done,
} MixCostSwapState;

// The mixing thread records into one histogram while the main thread reads the other, they swap when the main thread asks for it
typedef struct MixCostStats {
	MixCostHistogram histograms[MIX_COST_HISTOGRAM_COUNT];
	volatile uint32_t writeIndex;
	volatile uint32_t swapState;
	// Kernel thread id of the mixing thread, set at every swap
	volatile uint32_t mixThreadId;
} MixCostStats;

// The id that strace -p and /proc/<pid>/task show, pthread_self is a different number
static uint32_t GetKernelThreadId(void) {
#if defined(FPL_PLATFORM_LINUX)
	long threadId = syscall(SYS_gettid);
	return((uint32_t)threadId);
#else
	uint64_t threadId = fplGetCurrentThreadId();
	return((uint32_t)threadId);
#endif
}

// Called by the mixing thread after every call, without locks
static void RecordMixCostStats(MixCostStats *stats, const double microseconds) {
	uint32_t swapState = fplAtomicLoadU32(&stats->swapState);
	if(swapState == MixCostSwapState_Requested) {
		uint32_t previousIndex = fplAtomicLoadU32(&stats->writeIndex);
		uint32_t nextIndex = (previousIndex + 1) % MIX_COST_HISTOGRAM_COUNT;
		uint32_t mixThreadId = GetKernelThreadId();
		fplAtomicStoreU32(&stats->writeIndex, nextIndex);
		fplAtomicStoreU32(&stats->mixThreadId, mixThreadId);
		fplAtomicStoreU32(&stats->swapState, MixCostSwapState_Done);
	}
	uint32_t writeIndex = fplAtomicLoadU32(&stats->writeIndex);
	RecordMixCost(&stats->histograms[writeIndex], microseconds);
}

typedef struct ProcessMemoryUsage {
	// Peak of the resident memory (VmHWM)
	uint64_t highWaterKilobytes;
	// Resident memory now (VmRSS)
	uint64_t residentKilobytes;
} ProcessMemoryUsage;

#if defined(FPL_PLATFORM_LINUX)
// The number of a "<key>: <number> kB" line of /proc/self/status, 0 when the key is missing
static uint64_t FindProcStatusKilobytes(const char *statusText, const char *key) {
	size_t keyLength = fplGetStringLength(key);
	const char *line = statusText;
	while(*line != 0) {
		size_t lineLength = 0;
		while(line[lineLength] != 0 && line[lineLength] != '\n') {
			++lineLength;
		}
		bool isKeyLine = (lineLength > keyLength) && fplIsStringEqualLen(line, keyLength, key, keyLength) && (line[keyLength] == ':');
		if(isKeyLine) {
			const char *digit = line + keyLength + 1;
			while(*digit == ' ' || *digit == '\t') {
				++digit;
			}
			const uint64_t decimalBase = 10;
			uint64_t result = 0;
			while(*digit >= '0' && *digit <= '9') {
				result = result * decimalBase + (uint64_t)(*digit - '0');
				++digit;
			}
			return(result);
		}
		line += lineLength;
		if(*line == '\n') {
			++line;
		}
	}
	return(0);
}
#endif

// Linux only, zero elsewhere
static ProcessMemoryUsage GetProcessMemoryUsage(void) {
	ProcessMemoryUsage result = fplZeroInit;
#if defined(FPL_PLATFORM_LINUX)
	fplFileHandle statusFile = fplZeroInit;
	if(fplFileOpenBinary("/proc/self/status", &statusFile)) {
		char statusText[8192];
		uint32_t textLength = 0;
		uint32_t maxTextLength = (uint32_t)fplArrayCount(statusText) - 1;
		while(textLength < maxTextLength) {
			uint32_t readLength = fplFileReadBlock32(&statusFile, maxTextLength - textLength, statusText + textLength, maxTextLength - textLength);
			if(readLength == 0) {
				break;
			}
			textLength += readLength;
		}
		statusText[textLength] = 0;
		fplFileClose(&statusFile);
		result.highWaterKilobytes = FindProcStatusKilobytes(statusText, "VmHWM");
		result.residentKilobytes = FindProcStatusKilobytes(statusText, "VmRSS");
	}
#endif
	return(result);
}

// Changed by the keys on the main thread and used for every track the streaming thread starts, guarded by AudioDemo.trackLock
typedef struct PlayerControls {
	float volume;
	PlayMode playMode;
	bool isRepeat;
} PlayerControls;

typedef struct AudioDemo {
	AudioVisualization visualization;
	AudioTrackList trackList;
	AudioSystem audioSys;
	PlayerOptions options;

	MixCostStats mixCostStats;				// Cost of the mixer calls for --stats

	LockFreeRingBuffer outputRingBuffer;	// The ring buffer for the audio output
	AudioBuffer outputTempBuffer;			// Used for decoding the audio samples into, before its pushed to the output ring buffer

	AudioSineWaveData sineWave;
	fplAudioFormat targetAudioFormat;
	fplThreadHandle *streamingThread;

	uint64_t lastVideoAudioChunkUpdateTime;
	volatile uint32_t numFramesPlayed;
	volatile uint32_t numFramesStreamed;
	volatile uint32_t maxPlaybackFrameLatency;

	// Length of the current track in output frames, for the progress bar when there is no full-track buffer
	volatile uint32_t currentTrackFrameCount;
	// A repeating track starts over after currentTrackFrameCount, so its position wraps around
	volatile uint32_t isCurrentTrackRepeating;

	// Only the streaming thread changes the track list, the main thread leaves requests and draws under trackLock.
	// The lock guards the track list against Render and the name of the current track, the device callback never takes it.
	fplMutexHandle trackLock;
	// Track switches the main thread asked for, +1 for the next and -1 for the previous track
	volatile int32_t pendingTrackStep;
	// 1 while droppedFilePath waits for the streaming thread
	volatile uint32_t hasDroppedFile;
	char droppedFilePath[FPL_MAX_PATH_LENGTH];
	// Set by the streaming thread at a track switch, the device callback empties the output ring, resets numFramesPlayed and clears it
	volatile uint32_t isPlaybackResetRequested;
	// The play item of the current track
	volatile uint64_t currentPlayItemId;
	// Counts the track starts, the main thread updates title and visualization when it changes
	volatile uint32_t trackChangeCount;
	// Name of the current track, guarded by trackLock
	char currentTrackName[256];
	// Guarded by trackLock
	PlayerControls controls;
	// 1 while paused, every track start clears it. Nothing is mixed then (by the device callback in callback mode, by the streaming thread in thread mode), so numFramesPlayed counts only what is heard.
	volatile uint32_t isPaused;
	// Only the streaming thread: how far the current track is from its end, and the numFramesPlayed at which its end is heard
	TrackEndState trackEndState;
	uint32_t trackEndFrameCount;
	// Only the streaming thread in thread mode: the numFramesStreamed at which the current track ends, UINT32_MAX while unknown (repeating)
	uint32_t streamedTrackEndFrameCount;

	volatile fpl_b32 isStreamingThreadStopped;
	fpl_b32 useRealTimeSamples;
	SpectrumMode spectrumMode;
	PlaybackMode playbackMode;
} AudioDemo;

// Adds the cost of the mixer call that started at mixStart, only with --stats
static void RecordMixCostSince(AudioDemo *demo, const fplTimestamp mixStart) {
	if(!demo->options.isStats) {
		return;
	}
	fplTimestamp mixEnd = fplTimestampQuery();
	fplSeconds mixSeconds = fplTimestampElapsed(mixStart, mixEnd);
	RecordMixCostStats(&demo->mixCostStats, mixSeconds * MicrosecondsPerSecond);
}

// Only thread mode has the full-track buffer, the other modes always visualize the samples that went to the device
static bool IsRealTimeVisualization(const AudioDemo *demo) {
	bool result = demo->useRealTimeSamples || (demo->playbackMode != PlaybackMode_Thread);
	return(result);
}

// The position in the current track, a repeating track starts over after its length
static AudioFrameIndex GetTrackPosition(AudioDemo *demo, const AudioFrameIndex framesPlayed) {
	AudioFrameIndex trackFrameCount = fplAtomicLoadU32(&demo->currentTrackFrameCount);
	uint32_t isTrackRepeating = fplAtomicLoadU32(&demo->isCurrentTrackRepeating);
	AudioFrameIndex result = framesPlayed;
	if(isTrackRepeating && trackFrameCount > 0) {
		result = framesPlayed % trackFrameCount;
	}
	return(result);
}

static void UpdateTitle(AudioDemo *demo, const char *audioTrackName, const double fps) {
	const bool isRealTime = IsRealTimeVisualization(demo);
	char titleBuffer[256];
	const char *rtString = (isRealTime ? "RT" : "BUF");
	const fplAudioBackendType backendType = fplGetAudioBackendType();
	const char *audioBackendName = fplGetAudioBackendName(backendType);
	const fplSettings *settings = fplGetCurrentSettings();
	if (fplGetStringLength(audioTrackName) > 0)
        fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s (%s, %s, %u Hz, %u ch) - %s [%.3f fps]", APP_TITLE, rtString, audioBackendName, demo->targetAudioFormat.sampleRate, demo->targetAudioFormat.channels, audioTrackName, fps);
	else
        fplStringFormat(titleBuffer, fplArrayCount(titleBuffer), "%s (%s, %s, %u Hz, %u ch) [%.3f fps]", APP_TITLE, rtString, audioBackendName, demo->targetAudioFormat.sampleRate, demo->targetAudioFormat.channels, fps);
	fplSetWindowTitle(titleBuffer);
}

static void RenderPushRectangle(const float x0, const float y0, const float x1, const float y1, const Vec4f color, const float lineWidth) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y0);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glEnd();
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 1);
}

static void RenderPushQuad(const float x0, const float y0, const float x1, const float y1, const Vec4f color) {
	glColor4fv(&color.m[0]);
	glBegin(GL_QUADS);
	glVertex2f(x1, y0);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(x1, y1);
	glEnd();
	glColor4f(1, 1, 1, 1);
}

static void RenderPushLine(const float x0, const float y0, const float x1, const float y1, const Vec4f color, const float lineWidth) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINES);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glEnd();
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 1);
}

static void RenderRingBuffer(const Vec2f pos, const Vec2f dim, LockFreeRingBuffer *buffer) {
	RenderPushRectangle(pos.x, pos.y, pos.x + dim.w, pos.y + dim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

	uint64_t bufferLen = buffer->length;

	float bufferScale = dim.w / (float)bufferLen;

	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);

	uint64_t remaining = bufferLen - fillCount;

	uint64_t tail = buffer->tail;
	uint64_t head = buffer->head;

	float tailPos = pos.x + (float)tail * bufferScale;
	float headPos = pos.x + (float)head * bufferScale;

	bool tailWouldWrap = (tail + fillCount) > bufferLen;
	if(tailWouldWrap) {
		// Double
		RenderPushQuad(tailPos, pos.y, pos.x + dim.w, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
		uint64_t wrapPos = (tail + fillCount) % bufferLen;
		float fillEnd = wrapPos * bufferScale;
		RenderPushQuad(pos.x, pos.y, pos.x + fillEnd, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	} else {
		// Single
		float fillOffset = fillCount * bufferScale;
		RenderPushQuad(tailPos, pos.y, tailPos + fillOffset, pos.y + dim.h, V4fInit(1.0f, 1.0f, 1.0f, 0.5f));
	}

	RenderPushLine(headPos, pos.y - dim.h * 0.5f, headPos, pos.y + dim.h * 1.5f, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
	RenderPushLine(tailPos, pos.y - dim.h * 0.5f, tailPos, pos.y + dim.h * 1.5f, V4fInit(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);
}

static void ClearVisualization(AudioDemo *demo) {
	fplAtomicExchangeU32(&demo->visualization.hasVideoAudioChunk, 0);
	fplClearStruct(&demo->visualization.videoAudioChunks);
	fplClearStruct(&demo->visualization.currentMagnitudes);
	fplClearStruct(&demo->visualization.rawSamples);
	fplClearStruct(&demo->visualization.monoSamples);
	fplClearStruct(&demo->visualization.lastMagnitudes);
	fplClearStruct(&demo->visualization.currentSamples);
	fplClearStruct(&demo->visualization.lastSamples);
	fplClearStruct(&demo->visualization.fftInput);
	fplClearStruct(&demo->visualization.fftOutput);
	fplClearStruct(&demo->visualization.scaledMagnitudes);
	fplClearStruct(&demo->visualization.scaledSamples);
	fplClearStruct(&demo->visualization.rawMagnitudes);
	fplClearStruct(&demo->visualization.peakSpectrum);
	demo->visualization.globalPeak = 0.0;
	demo->spectrumMode = SpectrumMode_Natural;
}

static void Render(AudioDemo *demo, const int screenW, const int screenH, const double currentRenderTime) {
	float w = (float)screenW;
	float h = (float)screenH;

	glViewport(0, 0, screenW, screenH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Window coordinate system
	glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
	// Draw center cross
	RenderLine(0.0f, h * 0.5f, w, h * 0.5f, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
	RenderLine(w * 0.5f, 0, w * 0.5f, h, V4fInit(1.0f, 1.0f, 1.0f, 0.25f), 1.0f);
#endif

	LockFreeRingBuffer *streamRingBuffer = &demo->outputRingBuffer;

	static AudioBuffer zeroFullAudioBufferStub = fplZeroInit;
	AudioBuffer *fullAudioBuffer = &zeroFullAudioBufferStub;

	AudioTrackList *trackList = &demo->trackList;

	if(HasAudioTrack(trackList)) {
		fplAssert(trackList->currentIndex < (int32_t)trackList->count);
		AudioTrack *track = trackList->tracks + trackList->currentIndex;
		fullAudioBuffer = &track->outputFullBuffer;
	}

	AudioVisualization *visualization = &demo->visualization;

	const float marginW = w * 0.05f;
	const float marginH = h * 0.05f;
	const float maxBufferW = w - marginW * 2.0f;
	const float maxBufferH = h * 0.1f;
	const float bufferBarH = maxBufferH * 0.9f;
	const float progressW = w - marginW * 2.0f;
	const float progressH = h * 0.05f;
	const float progressBarPadding = 5.0f;
	const float progressBarMaxWidth = progressW - progressBarPadding * 2.0f;
	const float progressBarMaxHeight = progressH - progressBarPadding * 2.0f;

	const float spectrumWidth = w - marginW * 2.0f;
	const float spectrumHeight = h - maxBufferH - progressH - marginH * 2.0f;

	Vec2f streamBufferDim = V2fInit(maxBufferW, maxBufferH);
	Vec2f streamBufferPos = V2fInit((w - streamBufferDim.w) * 0.5f, h - marginH - streamBufferDim.h);

	Vec2f spectrumDim = V2fInit(spectrumWidth, spectrumHeight);
	Vec2f spectrumPos = V2fInit((w - spectrumDim.w) * 0.5f, marginH + progressH);

	Vec2f progressDim = V2fInit(progressW, progressH);
	Vec2f progressPos = V2fInit((w - progressDim.w) * 0.5f, marginH);

	// Only thread mode has an output ring
	if(demo->playbackMode == PlaybackMode_Thread) {
		RenderRingBuffer(streamBufferPos, streamBufferDim, streamRingBuffer);
	}

	RenderPushRectangle(spectrumPos.x, spectrumPos.y, spectrumPos.x + spectrumDim.w, spectrumPos.y + spectrumDim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

	fplAudioFormatType format = demo->targetAudioFormat.type;
	size_t sampleSize = fplGetAudioSampleSizeInBytes(format);
	AudioChannelIndex channelCount = demo->targetAudioFormat.channels;
	size_t frameSize = sampleSize * channelCount;
	AudioFrameIndex frameCount = visualization->videoAudioChunks[0].count;
	AudioFramesChunk *chunk = &visualization->videoAudioChunks[0];

	uint8_t *chunkSamples = chunk->samples;

	const AudioFrameIndex trackFrameCount = fplAtomicLoadU32(&demo->currentTrackFrameCount);
	const bool isRealTime = IsRealTimeVisualization(demo);

	AudioFrameIndex framesPlayed;
	if(isRealTime) {
		if(fplAtomicIsCompareAndSwapU32(&visualization->hasVideoAudioChunk, 2, 3)) {
			visualization->videoAudioChunks[0] = visualization->videoAudioChunks[1];
			fplAtomicExchangeU32(&visualization->hasVideoAudioChunk, 0);
		}

		framesPlayed = GetTrackPosition(demo, chunk->index);
	} else {

#if 1
		AudioFrameIndex totalFramesPlayed = fplAtomicLoadU32(&demo->numFramesPlayed);
		framesPlayed = GetTrackPosition(demo, totalFramesPlayed);
#else
		double renderMsecs = currentRenderTime * 1000.0;
		framesPlayed = (AudioFrameIndex)(((double)demo->targetAudioFormat.sampleRate / 1000.0) * renderMsecs);
		if(framesPlayed > fullAudioBuffer->frameCount) {
			framesPlayed = fullAudioBuffer->frameCount;
		}
		fplAssert(framesPlayed <= fullAudioBuffer->frameCount);
#endif

		AudioFrameIndex remainingFramesToPlay = framesPlayed < fullAudioBuffer->frameCount ? fullAudioBuffer->frameCount - framesPlayed : 0;
		AudioFrameIndex remainingChunkFrames = fplMin(MAX_AUDIO_FRAMES_CHUNK_FRAMES, remainingFramesToPlay);
		if(remainingChunkFrames > 0) {
			size_t sourceFrameSize = fullAudioBuffer->bufferSize / fullAudioBuffer->frameCount;
			fplAssert(sourceFrameSize == frameSize);
			size_t totalSizeToCopy = remainingChunkFrames * frameSize;
			size_t chunkSamplesOffset = framesPlayed * frameSize;
			size_t endPos = chunkSamplesOffset + totalSizeToCopy;
			fplAssert(endPos <= fullAudioBuffer->bufferSize);
			const uint8_t *p = fullAudioBuffer->samples + chunkSamplesOffset;
			fplMemoryCopy(p, totalSizeToCopy, chunkSamples);
		}

		if(remainingChunkFrames < MAX_AUDIO_FRAMES_CHUNK_FRAMES && chunk->count >= MAX_AUDIO_FRAMES_CHUNK_FRAMES) {
			size_t totalSizeToClear = MAX_AUDIO_FRAMES_CHUNK_FRAMES * frameSize;
			fplMemoryClear(chunkSamples, totalSizeToClear);
		}

		frameCount = MAX_AUDIO_FRAMES_CHUNK_FRAMES;
	}

	RenderPushRectangle(progressPos.x, progressPos.y, progressPos.x + progressDim.w, progressPos.y + progressDim.h, (Vec4f) { 1, 1, 1, 0.5f }, 1.0f);

	float progressBarScale = 0.0f;
	if (trackFrameCount > 0) {
		float trackProgress = framesPlayed / (float)trackFrameCount;
		progressBarScale = fplMax(0.0f, fplMin(1.0f, trackProgress));
	}

	float progressBarWidth = progressBarMaxWidth * progressBarScale;
	RenderPushQuad(progressPos.x + progressBarPadding, progressPos.y + progressBarPadding, progressPos.x + progressBarPadding + progressBarWidth, progressPos.y + progressBarPadding + progressBarMaxHeight, (Vec4f) { 1, 1, 0, 1.0f });

	if(frameCount > 0 && chunkSamples != fpl_null) {
		// Convert all samples to float
		bool convertRes = AudioSamplesConvert(&demo->audioSys.conversionFuncs, frameCount * channelCount, format, fplAudioFormatType_F32, chunkSamples, visualization->rawSamples);
		fplAssert(convertRes == true);

		// Convert samples to mono
		convertRes = AudioSamplesMonolize(channelCount, frameCount, visualization->rawSamples, visualization->monoSamples);
		fplAssert(convertRes == true);

		// Build FFT input samples from mono samples
		// Apply hanning window (Coefficients are precomputed)
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double sampleValue = visualization->monoSamples[frameIndex];
			double windowMultiplier = visualization->windowCoeffs[frameIndex];
			double adjustedSampleValue = sampleValue * windowMultiplier;
			double clampedSample = fplMax(-1.0, fplMin(adjustedSampleValue, 1.0));
			visualization->lastSamples[frameIndex] = visualization->currentSamples[frameIndex];
			visualization->currentSamples[frameIndex] = clampedSample;
			visualization->fftInput[frameIndex].real = clampedSample;
			visualization->fftInput[frameIndex].imag = 0;
		}

		// Smooth out samples (just for visualization)
		const double samplesSmooth = 0.35;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double lastSample = visualization->lastSamples[frameIndex];
			double currentSample = visualization->currentSamples[frameIndex];
			double newSample = lastSample * (1.0 - samplesSmooth) + currentSample * samplesSmooth;
			visualization->currentSamples[frameIndex] = newSample;
		}

		// Track min/max samples
		double minSamples = visualization->currentSamples[0];
		double maxSamples = visualization->currentSamples[0];
		for(uint32_t frameIndex = 1; frameIndex < frameCount; ++frameIndex) {
			double sample = visualization->currentSamples[frameIndex];
			if(sample > maxSamples) {
				maxSamples = sample;
			}
			if(sample < minSamples) {
				minSamples = sample;
			}
		}

		const bool sampleScaling = false;

		if (!sampleScaling) {
			// No sample scaling
			double scaleSamplesFitFactor = 2.0f;
			for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				visualization->scaledSamples[frameIndex] = visualization->currentSamples[frameIndex] * scaleSamplesFitFactor;
			}
		} else {
			// Normalize samples to be in full range of -1.0 to 1.0, just for better visualization
			double scaleSamplesFitFactor = 0.75f;
			double rangeSample = maxSamples - minSamples;
			for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				double sample = visualization->currentSamples[frameIndex];
				double scaledSample = ((sample - minSamples) / rangeSample) * scaleSamplesFitFactor;
				visualization->scaledSamples[frameIndex] = -1.0f + scaledSample * 2.0f;
			}
		}

		// Forward FFT using raw samples
        ForwardFFT(visualization->fftInput, frameCount, visualization->fftOutput);

		const uint32_t halfFFT = frameCount / 2;

		// Compute magnitudes in dB with fixed reference scale [-120, 0] dB
		// Factor of 2 for single-sided spectrum (all bins except DC and Nyquist)
		const double minDb = -60.0;
		const double maxDb = 0.0;
		const double dbRange = maxDb - minDb;
		for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double re = visualization->fftOutput[frameIndex].real;
			double im = visualization->fftOutput[frameIndex].imag;
			double rawMagnitude = sqrt(re * re + im * im);
			visualization->rawMagnitudes[frameIndex] = rawMagnitude;
			double amplitude = (frameIndex > 0 && frameIndex < halfFFT) ? rawMagnitude * 2.0 : rawMagnitude;
			double dB = 20.0 * log10(amplitude + 1e-10);
			double scaledMagnitude = fplMax(0.0, fplMin((dB - minDb) / dbRange, 1.0));
			visualization->lastMagnitudes[frameIndex] = visualization->currentMagnitudes[frameIndex];
			visualization->currentMagnitudes[frameIndex] = scaledMagnitude;
		}

		// Smooth magnitudes with fast attack / slow decay
		const double magAttack = 0.8;
		const double magDecay = 0.05;
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			double last = visualization->lastMagnitudes[frameIndex];
			double current = visualization->currentMagnitudes[frameIndex];
			double factor = (current > last) ? magAttack : magDecay;
			visualization->currentMagnitudes[frameIndex] = last * (1.0 - factor) + current * factor;
		}

		// Copy smoothed magnitudes to scaledMagnitudes for rendering
		for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			visualization->scaledMagnitudes[frameIndex] = visualization->currentMagnitudes[frameIndex];
		}

		// Fill spectrum bins with max raw magnitude per frequency band (video algorithm step 5)
		uint32_t binCount = MAX_AUDIO_BIN_COUNT;
		double newSpectrum[MAX_AUDIO_BIN_COUNT] = {0};
		for (uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
			double lowerFrequency = visualization->bins[binIndex];
			double upperFrequency = visualization->bins[binIndex + 1];
			for (uint32_t frameIndex = 0; frameIndex < halfFFT; ++frameIndex) {
				double frameFreq = (frameIndex * (double)demo->targetAudioFormat.sampleRate) / (double)frameCount;
				if (frameFreq >= lowerFrequency && frameFreq <= upperFrequency) {
					double mag = visualization->rawMagnitudes[frameIndex];
					if (mag > newSpectrum[binIndex])
						newSpectrum[binIndex] = mag;
				}
			}
		}

		// Apply spectrum deformation (video algorithm step 6)
		// After NormalizeFFT (÷N=256) + Hamming window, full-scale sine → rawMagnitude ≈ 0.27
		const double peakDecay = 0.99;
		switch (demo->spectrumMode) {
			case SpectrumMode_Natural: {
				// fitFactor = 1/0.27 ≈ 3.7 so a full-scale sine reaches ~100% bar height
				const double fitFactor = 4.0;
				for (uint32_t i = 0; i < binCount; ++i)
					newSpectrum[i] = fplMin(newSpectrum[i] * fitFactor, 1.0);
			} break;
			case SpectrumMode_Exponential: {
				// fitFactor2=1000 sets noise floor at x=0.001 (log(0.001*1000)=log(1)=0)
				// fitFactor=0.16 so full-scale (x=0.27): log(270)*0.16 ≈ 0.90
				const double fitFactor = 0.16;
				const double fitFactor2 = 1000.0;
				for (uint32_t i = 0; i < binCount; ++i)
					newSpectrum[i] = fplMin(fplMax(log(newSpectrum[i] * fitFactor2) * fitFactor, 0.0), 1.0);
			} break;
			case SpectrumMode_MultiPeakScale: {
				const double fitFactor = 1.0;
				for (uint32_t i = 0; i < binCount; ++i) {
					if (newSpectrum[i] > visualization->peakSpectrum[i])
						visualization->peakSpectrum[i] = newSpectrum[i];
					else
						visualization->peakSpectrum[i] *= peakDecay;
					double peak = visualization->peakSpectrum[i];
					newSpectrum[i] = (peak > 1e-10) ? fplMin(newSpectrum[i] / peak * fitFactor, 1.0) : 0.0;
				}
			} break;
			case SpectrumMode_MaxPeakScale: {
				const double fitFactor = 1.0;
				double maxVal = 0.0;
				for (uint32_t i = 0; i < binCount; ++i)
					if (newSpectrum[i] > maxVal) maxVal = newSpectrum[i];
				if (maxVal > visualization->globalPeak)
					visualization->globalPeak = maxVal;
				else
					visualization->globalPeak *= peakDecay;
				double gPeak = visualization->globalPeak;
				for (uint32_t i = 0; i < binCount; ++i)
					newSpectrum[i] = (gPeak > 1e-10) ? fplMin(newSpectrum[i] / gPeak * fitFactor, 1.0) : 0.0;
			} break;
			default: break;
		}

		// Temporal smoothing with fast attack / slow decay (~1s fall from peak at 60fps)
		const double specAttack = 0.7;
		const double specDecay  = 0.04;
		for (uint32_t i = 0; i < binCount; ++i) {
			double last    = visualization->spectrum[i];
			double current = newSpectrum[i];
			double factor  = (current > last) ? specAttack : specDecay;
			visualization->spectrum[i] = last * (1.0 - factor) + current * factor;
		}

#if 1
		// Draw wave form
		{
			float lineX = spectrumPos.x;
			float lineY = spectrumPos.y + spectrumDim.h * 0.5f;
			float maxWaveFormHeight = spectrumDim.h * 0.5f;
			for(uint32_t frameIndex = 0; frameIndex < frameCount - 1; ++frameIndex) {
				double sampleValue1 = visualization->scaledSamples[frameIndex + 0];
				double sampleValue2 = visualization->scaledSamples[frameIndex + 1];
				float x1 = lineX + ((float)(frameIndex + 0) / (float)(frameCount - 1) * spectrumDim.w);
				float x2 = lineX + ((float)(frameIndex + 1) / (float)(frameCount - 1) * spectrumDim.w);
				float y1 = lineY + ((float)sampleValue1 * maxWaveFormHeight * 0.5f);
				float y2 = lineY + ((float)sampleValue2 * maxWaveFormHeight * 0.5f);
				RenderPushLine(x1, y1, x2, y2, (Vec4f) { 0.8f, 0.25f, 0.05f, 1.0f }, 4.0f);
			}
		}
#endif

#if 1
		// Draw samples
		{
			float spacing = 4.0f;
			float totalSpacing = spacing * (frameCount - 1);
			float barMaxWidth = spectrumDim.w / (float)frameCount;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)frameCount;
			float barMaxHeight = spectrumDim.h * 0.25f;
			float barStartX = spectrumPos.x;
			float barStartY = spectrumPos.y + spectrumDim.h - barMaxHeight;
			for(uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				double sampleValue = visualization->scaledSamples[frameIndex];
				float barHeight = (float)sampleValue * barMaxHeight;
				float barX = barStartX + frameIndex * barWidth + frameIndex * spacing;
				float barY = barStartY + barMaxHeight * 0.5f;
				RenderPushQuad(barX, barY + barHeight * 0.5f, barX + barWidth, barY - barHeight * 0.5f, (Vec4f) { 1, 1, 0, 1 });
			}
		}
#endif

#if 1
		// Draw FFT
		{
			float spacing = 4.0f;
			float totalSpacing = spacing * (halfFFT - 1);
			float barMaxWidth = spectrumDim.w / (float)halfFFT;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)halfFFT;
			float barStartX = spectrumPos.x;
			float barStartY = spectrumPos.y;
			for(uint32_t frameIndex = 0; frameIndex < halfFFT; ++frameIndex) {
				double scaledMagnitude = visualization->scaledMagnitudes[frameIndex];
				float barX = barStartX + frameIndex * barWidth + frameIndex * spacing;
				float barHeight = (float)scaledMagnitude * barMaxHeight;
				RenderPushQuad(barX, barStartY, barX + barWidth, barStartY + barHeight, (Vec4f) { 0.0f, 1.0, 0.1f, 0.25f });
			}
		}
#endif

#if 1
		// Draw spectrum
		{
			float spacing = 2.0f;
			float totalSpacing = spacing * (binCount - 1);
			float barMaxWidth = spectrumDim.w / (float)binCount;
			float barMaxHeight = spectrumDim.h;
			float barWidth = (spectrumDim.w - totalSpacing) / (float)binCount;
			float barY = spectrumPos.y;
			for(uint32_t binIndex = 0; binIndex < binCount; ++binIndex) {
				double scaledMagnitude = visualization->spectrum[binIndex];
				float barX = spectrumPos.x + binIndex * barWidth + binIndex * spacing;
				float barHeight = (float)scaledMagnitude * barMaxHeight;
				RenderPushQuad(barX, barY, barX + barWidth, barY + barHeight, (Vec4f) { 0.0f, 0.1f, 1.0f, 0.5f });
			}
		}
#endif
	}  // chunkFrameCount > 0

//fplDebugFormatOut("Current render/play time: [%f, %f], played/streamed frames: [%lu, %lu], chunks: %zu\n", currentRenderTime, currentPlayTime, playedFrames, streamedFrames, numChunks);
}

/**
* @brief Copies the first chunk of the samples just written to the device for the real-time visualization, about 60 times per second.
* @param[in] demo The audio demo.
* @param[in] outFormat The output audio format, @ref fplAudioFormat.
* @param[in] outputSamples The samples just written to the device.
* @param[in] writtenFrameCount The number of frames written to the device.
* @param[in] firstFrameIndex The number of frames played before the written ones.
*/
static void CaptureRealTimeChunk(AudioDemo *demo, const fplAudioFormat *outFormat, const void *outputSamples, const AudioFrameIndex writtenFrameCount, const AudioFrameIndex firstFrameIndex) {
	AudioVisualization *visualization = &demo->visualization;
	size_t frameSize = fplGetAudioFrameSizeInBytes(outFormat->type, outFormat->channels);
	const uint64_t updateInterval = 1000 / 60;
	if((writtenFrameCount >= MAX_AUDIO_FRAMES_CHUNK_FRAMES) && ((fplMillisecondsQuery() - demo->lastVideoAudioChunkUpdateTime) >= updateInterval)) {
		if(fplAtomicIsCompareAndSwapU32(&visualization->hasVideoAudioChunk, 0, 1)) {
			visualization->videoAudioChunks[1].index = firstFrameIndex;
			visualization->videoAudioChunks[1].count = MAX_AUDIO_FRAMES_CHUNK_FRAMES;
			size_t chunkSamplesSize = frameSize * MAX_AUDIO_FRAMES_CHUNK_FRAMES;
			fplMemoryCopy(outputSamples, chunkSamplesSize, visualization->videoAudioChunks[1].samples);
			fplAtomicExchangeU32(&visualization->hasVideoAudioChunk, 2);
		}
		demo->lastVideoAudioChunkUpdateTime = fplMillisecondsQuery();
	}
}

/**
* @brief Copies up to N frames from the output ring that the streaming thread fills, used in thread mode.
* @param[in] demo The audio demo.
* @param[in] outFormat The output audio format, @ref fplAudioFormat.
* @param[in] maxFrameCount The number of frames that needs to be played.
* @param[out] outputSamples The reference to the output samples that is defined by the output format.
* @return Returns the number of audio frames that was copied, less than requested when the ring runs empty.
*/
static AudioFrameIndex ReadAudioFromRingBuffer(AudioDemo *demo, const fplAudioFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples) {
	AudioFrameIndex result = 0;

	uint32_t currentPlaybackLatency = fplAtomicLoadU32(&demo->maxPlaybackFrameLatency);
	fplAtomicExchangeU32(&demo->maxPlaybackFrameLatency, fplMax(currentPlaybackLatency, maxFrameCount));

	size_t frameSize = fplGetAudioFrameSizeInBytes(outFormat->type, outFormat->channels);

	LockFreeRingBuffer *ringBuffer = &demo->outputRingBuffer;

	size_t availableBytes = 0;
	bool hasData = LockFreeRingBufferCanRead(ringBuffer, &availableBytes);
	if(hasData && (availableBytes % frameSize) == 0) {
		AudioFrameIndex availableFrames = (AudioFrameIndex)fplMax(0, availableBytes / frameSize);

		AudioFrameIndex framesToCopy = fplMin(maxFrameCount, availableFrames);

		size_t totalCopySize = framesToCopy * frameSize;
		fplAssert((totalCopySize % frameSize) == 0);

		bool isRead = LockFreeRingBufferRead(ringBuffer, outputSamples, totalCopySize);
		fplAssert(isRead);

		result = framesToCopy;
	}

	return(result);
}

/**
* @brief Called by the sound device automatically, requesting N audio frames/samples to be written to the output.
* This function has a very tight time-frame and will be called many thousand times per second, so keep implementation to be as small as possible.
* @param[in] outFormat The output audio format, @ref fplAudioFormat.
* @param[in] maxFrameCount The number of frames that needs to be played.
* @param[out] outputSamples The reference to the output samples that is defined by the output format.
* @param[in] userData The user data reference.
* @return Returns the number of audio frames that was written.
*/
static uint32_t AudioPlayback(const fplAudioFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	AudioDemo *demo = (AudioDemo *)userData;

	AudioFrameIndex result = 0;

	// A track switch: only the consumer may drop what is left of the track before in the ring, the streaming thread waits for this
	if(fplAtomicLoadU32(&demo->isPlaybackResetRequested) != 0) {
		if(demo->playbackMode == PlaybackMode_Thread) {
			size_t availableBytes = 0;
			if(LockFreeRingBufferCanRead(&demo->outputRingBuffer, &availableBytes)) {
				LockFreeRingBufferSkip(&demo->outputRingBuffer, availableBytes);
			}
		}
		fplAtomicStoreU32(&demo->numFramesPlayed, 0);
		fplAtomicStoreU32(&demo->isPlaybackResetRequested, 0);
	}

	switch(demo->playbackMode) {
		case PlaybackMode_DeviceSine:
		{
			fplTimestamp mixStart = fplTimestampQuery();
			result = maxFrameCount;
			AudioGenerateSineWave(&demo->sineWave, outputSamples, outFormat->type, outFormat->sampleRate, outFormat->channels, maxFrameCount);
			RecordMixCostSince(demo, mixStart);
		} break;

		case PlaybackMode_Callback:
		{
			// Nothing while paused: the play item stays where it is, and FPL fills the device buffer with silence
			if(fplAtomicLoadU32(&demo->isPaused) != 0) {
				break;
			}
			// @FIXME(final): Fix hearable error, when the audio stream has finished playing and will repeat
			fplTimestamp mixStart = fplTimestampQuery();
			result = AudioSystemWriteFrames(&demo->audioSys, outputSamples, outFormat, maxFrameCount, true);
			RecordMixCostSince(demo, mixStart);
		} break;

		case PlaybackMode_Thread:
		{
			result = ReadAudioFromRingBuffer(demo, outFormat, maxFrameCount, outputSamples);
		} break;

		default:
			break;
	}

	if(result > 0) {
		uint32_t numFramesPlayed = fplAtomicFetchAndAddU32(&demo->numFramesPlayed, result);
		if(IsRealTimeVisualization(demo)) {
			CaptureRealTimeChunk(demo, outFormat, outputSamples, result, numFramesPlayed);
		}
	}

	return(result);
}

static bool WriteAudioToRingBuffer(const fplAudioFormat *format, const uint32_t maxFrameCount, AudioDemo *demo, uint64_t *outDuration) {
	if(format == fpl_null || demo == fpl_null) return(false);
	if(maxFrameCount == 0) return(false);

	LockFreeRingBuffer *streamRingBuffer = &demo->outputRingBuffer;

	AudioBuffer *tmpBuffer = &demo->outputTempBuffer;

	size_t frameSize = fplGetAudioFrameSizeInBytes(format->type, format->channels);

	size_t availableStreamSpace = 0;
	bool canStreamWrite = LockFreeRingBufferCanWrite(streamRingBuffer, &availableStreamSpace);

	if(canStreamWrite && (availableStreamSpace % frameSize) == 0) {
		uint64_t timeStart = fplMillisecondsQuery();

		AudioFrameIndex numOfAvailableFrames = (AudioFrameIndex)fplMax(0, availableStreamSpace / frameSize);

		// Disable this condition to always fill the entire buffer
		//if(numOfAvailableFrames < maxFrameCount)
		//	return(false);

		AudioFrameIndex framesToWrite = fplMin(maxFrameCount, numOfAvailableFrames);

		size_t totalFrameBytes = framesToWrite * frameSize;
		fplAssert(totalFrameBytes <= tmpBuffer->bufferSize);

		// The amount of actual written frames may be less than the frames we want to get written
		fplTimestamp mixStart = fplTimestampQuery();
		AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, tmpBuffer->samples, format, framesToWrite, true);
		RecordMixCostSince(demo, mixStart);
		fplAssert(writtenFrames == framesToWrite);
		totalFrameBytes = writtenFrames * frameSize;

		// Write samples from temporary buffer to stream ring buffer
		bool streamWritten = LockFreeRingBufferWrite(streamRingBuffer, tmpBuffer->samples, totalFrameBytes);
		fplAssert(streamWritten);

		uint32_t lastNumFramesStreamed = fplAtomicFetchAndAddU32(&demo->numFramesStreamed, writtenFrames);

		if(outDuration != fpl_null) {
			uint64_t delta = fplMillisecondsQuery() - timeStart;
			*outDuration = (uint32_t)delta;
		}

		return(true);
	}

	return(false);
}

// The output frames that sourceFrameCount frames of the source become. Pitch is varispeed, the mixer plays a pitched source as if it had its sample rate times the pitch (without the neutral tolerance of the mixer).
static AudioFrameIndex ConvertToPlayedFrameCount(const AudioSource *source, const AudioFrameIndex sourceFrameCount, const float pitch, const AudioHertz targetSampleRate) {
	const float roundingOffset = 0.5f;
	float pitchedSampleRate = (float)source->format.sampleRate * pitch + roundingOffset;
	AudioHertz playedSampleRate = (AudioHertz)pitchedSampleRate;
	AudioFrameIndex result = fplGetTargetAudioFrameCount(sourceFrameCount, playedSampleRate, targetSampleRate);
	return(result);
}

static AudioFrameIndex GetPlayedFrameCount(const AudioSource *source, const float pitch, const AudioHertz targetSampleRate) {
	AudioFrameIndex result = ConvertToPlayedFrameCount(source, source->buffer.frameCount, pitch, targetSampleRate);
	return(result);
}

// Copies the play item of the current track, false when it has ended or there is none. Other play items, like the tones of --stress, do not count
static bool GetCurrentPlayItem(AudioDemo *demo, AudioPlayItem *outPlayItem) {
	uint64_t currentPlayItemId = fplAtomicLoadU64(&demo->currentPlayItemId);
	if(currentPlayItemId == 0) {
		return(false);
	}
	// Room for the whole pool, so the copy never fails for lack of space
	AudioPlayItem playItems[FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT];
	size_t playItemCount = AudioSystemGetPlayItems(&demo->audioSys, playItems, FINAL_AUDIOSYSTEM_MAX_PLAY_ITEM_COUNT);
	for(size_t itemIndex = 0; itemIndex < playItemCount; ++itemIndex) {
		if(playItems[itemIndex].id.value == currentPlayItemId) {
			*outPlayItem = playItems[itemIndex];
			return(true);
		}
	}
	return(false);
}

// Output frames left in the current pass of the play item of the current track, false when it is gone or repeats.
// Only in thread mode, where the streaming thread is the only one that mixes.
static bool GetRemainingPlayItemFrameCount(AudioDemo *demo, AudioFrameIndex *outFrameCount) {
	AudioPlayItem playItem = fplZeroInit;
	bool hasPlayItem = GetCurrentPlayItem(demo, &playItem);
	if(!hasPlayItem || playItem.isRepeat || playItem.isFinished[0]) {
		return(false);
	}
	AudioFrameIndex sourceFrameCount = playItem.source->buffer.frameCount;
	AudioFrameIndex playedSourceFrameCount = playItem.framesPlayed[0];
	AudioFrameIndex remainingSourceFrameCount = (playedSourceFrameCount < sourceFrameCount) ? (sourceFrameCount - playedSourceFrameCount) : 0;
	*outFrameCount = ConvertToPlayedFrameCount(playItem.source, remainingSourceFrameCount, playItem.pitch, demo->targetAudioFormat.sampleRate);
	return(true);
}

static bool SetAudioTrackSourceFromFile(AudioSystem *audioSys, AudioTrackSource *track, const char *filePath) {
	PCMWaveFormat fileFormat = fplZeroInit;
	if (!AudioSystemLoadFileFormat(audioSys, filePath, &fileFormat)) {
		FPL_LOG_WARN("Demo", "Audio file '%s' is missing or not supported!", filePath);
		return false;
	}
	if (!IsAudioSampleRateSupported(audioSys, fileFormat.samplesPerSecond)) {
		FPL_LOG_WARN("Demo", "Audio file '%s' cannot be converted from sample-rate '%u' to '%u'", filePath, fileFormat.samplesPerSecond, audioSys->targetFormat.sampleRate);
		return false;
	}
	const char *filename = fplExtractFileName(filePath);
	track->type = AudioTrackSourceType_URL;
	fplCopyString(filename, track->name, fplArrayCount(track->name));
	fplCopyString(filePath, track->url.urlOrFilePath, fplArrayCount(track->url.urlOrFilePath));
	FPL_LOG_INFO("Demo", "Audio file '%s' used with sample rate %u Hz", filePath, fileFormat.samplesPerSecond);
	return true;
}

// The sine wave track has no file or data source, LoadAudioTrackList generates it
static bool IsSineTrack(const AudioTrack *track) {
	bool result = (track->source.type == AudioTrackSourceType_None);
	return(result);
}

// Lets the device callback empty the output ring and reset numFramesPlayed, only the consumer may drop what is in the ring. Waits at most one second for its confirmation.
static void ResetPlayback(AudioDemo *demo) {
	const fplMilliseconds confirmTimeoutMilliseconds = 1000;
	const uint32_t confirmPollMilliseconds = 1;
	fplAtomicStoreU32(&demo->isPlaybackResetRequested, 1);
	fplMilliseconds startTime = fplMillisecondsQuery();
	while(fplAtomicLoadU32(&demo->isPlaybackResetRequested) != 0) {
		fplMilliseconds waitedMilliseconds = fplMillisecondsQuery() - startTime;
		if(waitedMilliseconds >= confirmTimeoutMilliseconds) {
			FPL_LOG_WARN("Track", "The device callback did not confirm the playback reset within %llu ms", (unsigned long long)confirmTimeoutMilliseconds);
			break;
		}
		fplThreadSleep(confirmPollMilliseconds);
	}
	fplAtomicStoreU32(&demo->numFramesStreamed, 0);
}

// Stops every play item, once AudioSystemStopAll returns the mixer mixes nothing of them anymore
static void StopCurrentTrack(AudioDemo *demo) {
	AudioSystemStopAll(&demo->audioSys);
	for(uint32_t trackIndex = 0; trackIndex < demo->trackList.count; ++trackIndex) {
		demo->trackList.tracks[trackIndex].playID.value = 0;
	}
	fplAtomicStoreU64(&demo->currentPlayItemId, 0);
	fplAtomicStoreU32(&demo->currentTrackFrameCount, 0);
	fplAtomicStoreU32(&demo->isCurrentTrackRepeating, 0);
	ResetPlayback(demo);
}

// Loads the samples of a file or data track and adds them to the audio system, which counts and frees them from then on
static AudioSource *LoadTrackSource(AudioDemo *demo, const AudioTrack *track) {
	AudioSource *source = fpl_null;
	switch(track->source.type) {
		case AudioTrackSourceType_URL:
		{
			source = AudioSystemLoadFileSource(&demo->audioSys, track->source.url.urlOrFilePath);
		} break;

		case AudioTrackSourceType_Data:
		{
			source = AudioSystemLoadDataSource(&demo->audioSys, track->source.data.size, track->source.data.data);
		} break;

		default:
			break;
	}
	if(source == fpl_null) {
		return(fpl_null);
	}
	if(!AudioSystemAddSource(&demo->audioSys, source)) {
		FPL_LOG_ERROR("Track", "Failed to add the source of '%s'", track->name);
		return(fpl_null);
	}
	return(source);
}

/**
* @brief Stops the current track and starts the track at the given index, only the streaming thread calls it.
* @param[in] demo The audio demo.
* @param[in] trackIndex Index into the track list.
* @note A track loads on its first start and keeps its source and its full-track buffer while the track list lives.
* @note Only thread mode renders the whole track for the visualization: it takes long and holds the mixer, which would block the device callback in callback mode.
*/
static void StartTrack(AudioDemo *demo, const int32_t trackIndex) {
	AudioTrackList *trackList = &demo->trackList;
	AudioTrack *track = &trackList->tracks[trackIndex];
	const PlayerOptions *options = &demo->options;
	const char *trackName = IsSineTrack(track) ? "Sine wave" : track->name;

	if(trackList->currentIndex >= 0) {
		StopCurrentTrack(demo);
	}

	// A new track starts unpaused
	fplAtomicStoreU32(&demo->isPaused, 0);
	demo->trackEndState = TrackEndState_Playing;
	demo->streamedTrackEndFrameCount = UINT32_MAX;

	fplMutexLock(&demo->trackLock);
	PlayerControls controls = demo->controls;
	fplMutexUnlock(&demo->trackLock);

	AudioSource *source = fpl_null;
	AudioTrackState state = (AudioTrackState)fplAtomicLoadS32(&track->state);
	if(state == AudioTrackState_Unloaded) {
		fplAtomicStoreS32(&track->state, AudioTrackState_Loading);
		source = LoadTrackSource(demo, track);
		if(source != fpl_null) {
			track->sourceID = source->id;
		}
	} else if(state == AudioTrackState_Ready || state == AudioTrackState_Full) {
		source = AudioSystemGetSourceByID(&demo->audioSys, track->sourceID);
	}

	if(source == fpl_null) {
		fplAtomicStoreS32(&track->state, AudioTrackState_Failed);
		fplMutexLock(&demo->trackLock);
		trackList->currentIndex = trackIndex;
		fplMutexUnlock(&demo->trackLock);
		FPL_LOG_ERROR("Track", "Track %d of %u cannot be played: %s", trackIndex + 1, trackList->count, trackName);
		return;
	}

	track->playID = AudioSystemPlaySource(&demo->audioSys, source, controls.isRepeat, controls.volume, options->pitch);
	fplAtomicStoreU64(&demo->currentPlayItemId, track->playID.value);

	AudioFrameIndex trackFrameCount = GetPlayedFrameCount(source, options->pitch, demo->targetAudioFormat.sampleRate);
	fplAtomicStoreU32(&demo->currentTrackFrameCount, trackFrameCount);
	fplAtomicStoreU32(&demo->isCurrentTrackRepeating, controls.isRepeat ? 1 : 0);

	bool needsPreview = (demo->playbackMode == PlaybackMode_Thread) && !track->outputFullBuffer.isAllocated;

	fplMutexLock(&demo->trackLock);
	if(needsPreview) {
		AudioFormat fullAudioBufferFormat = fplZeroInit;
		fullAudioBufferFormat.channels = demo->targetAudioFormat.channels;
		fullAudioBufferFormat.format = demo->targetAudioFormat.type;
		fullAudioBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
		AllocateAudioBuffer(&demo->audioSys.memory, &track->outputFullBuffer, &fullAudioBufferFormat, trackFrameCount);
	}
	fplAtomicStoreS32(&track->state, AudioTrackState_Ready);
	trackList->currentIndex = trackIndex;
	trackList->changedPending = false;
	fplCopyString(trackName, demo->currentTrackName, fplArrayCount(demo->currentTrackName));
	fplMutexUnlock(&demo->trackLock);

	fplAtomicFetchAndAddU32(&demo->trackChangeCount, 1);
	FPL_LOG_INFO("Track", "Track %d of %u: %s", trackIndex + 1, trackList->count, trackName);

	// Write the entire audio track into the full audio buffer, that is used for rendering the audio samples.
	// The actual playback uses the audio system again, but we can enable realtime (F1) to use the actual played back samples.
	if(needsPreview && track->outputFullBuffer.bufferSize > 0) {
		AudioFrameIndex writtenFrames = AudioSystemWriteFrames(&demo->audioSys, track->outputFullBuffer.samples, &demo->targetAudioFormat, trackFrameCount, false);
		fplAssert(writtenFrames == trackFrameCount);
	}
}

// Replaces the track list with the dropped file (and the sine wave with --sine), the streaming thread starts its first track next
static void ReplaceTrackList(AudioDemo *demo, const char *filePath) {
	AudioTrackSource trackSource = fplZeroInit;
	if(!SetAudioTrackSourceFromFile(&demo->audioSys, &trackSource, filePath)) {
		FPL_LOG_WARN("Track", "The dropped file '%s' cannot be played, the track list stays", filePath);
		return;
	}

	if(demo->trackList.currentIndex >= 0) {
		StopCurrentTrack(demo);
	}

	fplMutexLock(&demo->trackLock);
	for(uint32_t trackIndex = 0; trackIndex < fplArrayCount(demo->trackList.tracks); ++trackIndex) {
		AudioTrack *track = &demo->trackList.tracks[trackIndex];
		FreeAudioBuffer(&demo->audioSys.memory, &track->outputFullBuffer);
		fplClearStruct(track);
	}
	demo->trackList.count = 0;
	demo->trackList.currentIndex = -1;
	// No play item is left after StopCurrentTrack, so the sources can go
	AudioSystemClearSources(&demo->audioSys);
	bool isLoaded = LoadAudioTrackList(&demo->audioSys, &trackSource, 1, demo->options.isSine, &demo->sineWave, LoadAudioTrackFlags_None, &demo->trackList);
	fplMutexUnlock(&demo->trackLock);

	if(!isLoaded) {
		FPL_LOG_ERROR("Track", "Failed to build the track list for the dropped file '%s'", filePath);
	}
}

// The track that follows the current one by the play mode, tracks that failed to load are skipped. -1 when the play mode stops here.
static int32_t FindNextTrackIndex(AudioTrackList *trackList, const PlayMode playMode) {
	if(playMode == PlayMode_Off) {
		return(-1);
	}
	int32_t trackCount = (int32_t)trackList->count;
	for(int32_t trackStep = 1; trackStep <= trackCount; ++trackStep) {
		int32_t steppedIndex = trackList->currentIndex + trackStep;
		if(steppedIndex >= trackCount && playMode == PlayMode_List) {
			return(-1);
		}
		int32_t trackIndex = steppedIndex % trackCount;
		AudioTrackState state = (AudioTrackState)fplAtomicLoadS32(&trackList->tracks[trackIndex].state);
		if(state != AudioTrackState_Failed) {
			return(trackIndex);
		}
	}
	return(-1);
}

// Starts the track that follows by the play mode once the current one is heard to its end, not while paused
static void AdvanceAtTrackEnd(AudioDemo *demo) {
	AudioTrackList *trackList = &demo->trackList;
	if(fplAtomicLoadU32(&demo->isPaused) != 0) {
		return;
	}

	if(demo->trackEndState == TrackEndState_Playing) {
		// The mixer ends a play item at its end, a repeating one never ends. A track that failed to load has none.
		AudioTrack *currentTrack = &trackList->tracks[trackList->currentIndex];
		AudioTrackState currentState = (AudioTrackState)fplAtomicLoadS32(&currentTrack->state);
		AudioPlayItem currentPlayItem = fplZeroInit;
		bool hasCurrentPlayItem = GetCurrentPlayItem(demo, &currentPlayItem);
		if(currentState != AudioTrackState_Failed && hasCurrentPlayItem) {
			return;
		}
		// In thread mode the end is still in the output ring, it is heard once the device callback has played the stream up to it. Without a known end, up to everything streamed so far.
		uint32_t streamedFrameCount = fplAtomicLoadU32(&demo->numFramesStreamed);
		uint32_t streamedTrackEndFrameCount = fplMin(streamedFrameCount, demo->streamedTrackEndFrameCount);
		uint32_t playedFrameCount = fplAtomicLoadU32(&demo->numFramesPlayed);
		demo->trackEndFrameCount = (demo->playbackMode == PlaybackMode_Thread) ? streamedTrackEndFrameCount : playedFrameCount;
		demo->trackEndState = TrackEndState_Mixed;
	}

	uint32_t playedFrameCount = fplAtomicLoadU32(&demo->numFramesPlayed);
	if(playedFrameCount < demo->trackEndFrameCount) {
		return;
	}

	// Asked again after a stop, so switching the play mode in the silence goes on at once
	fplMutexLock(&demo->trackLock);
	PlayMode playMode = demo->controls.playMode;
	fplMutexUnlock(&demo->trackLock);
	int32_t nextIndex = FindNextTrackIndex(trackList, playMode);
	if(nextIndex < 0) {
		if(demo->trackEndState != TrackEndState_Stopped) {
			demo->trackEndState = TrackEndState_Stopped;
			FPL_LOG_INFO("Track", "Track %d of %u has ended, play mode %s", trackList->currentIndex + 1, trackList->count, PlayModeChoices[playMode].name);
		}
		return;
	}
	StartTrack(demo, nextIndex);
}

// Executes the requests of the main thread, called at the start of every loop of the streaming thread
static void ProcessTrackRequests(AudioDemo *demo) {
	if(fplAtomicLoadU32(&demo->hasDroppedFile) != 0) {
		char filePath[FPL_MAX_PATH_LENGTH];
		fplCopyString(demo->droppedFilePath, filePath, fplArrayCount(filePath));
		fplAtomicStoreU32(&demo->hasDroppedFile, 0);
		ReplaceTrackList(demo, filePath);
	}

	int32_t trackStep = fplAtomicExchangeS32(&demo->pendingTrackStep, 0);
	int32_t trackCount = (int32_t)demo->trackList.count;
	if(trackCount == 0) {
		return;
	}

	// Nothing started yet, at the start and after a drop
	int32_t currentIndex = demo->trackList.currentIndex;
	if(currentIndex < 0) {
		StartTrack(demo, 0);
		return;
	}

	if(trackStep != 0) {
		int32_t steppedIndex = (currentIndex + trackStep) % trackCount;
		int32_t nextIndex = (steppedIndex + trackCount) % trackCount;
		StartTrack(demo, nextIndex);
		return;
	}

	AdvanceAtTrackEnd(demo);
}

typedef struct {
	AudioFrameIndex frames;
	AudioMilliseconds delay;
	bool canIgnoreWait;
} AudioFrameDelayEntry;

static void AudioStreamingThread(const fplThreadHandle *thread, void *rawData) {
	AudioDemo *demo = (AudioDemo *)rawData;

	// This thing has a few issues on slow machines:
	// - Too much frames per loop is too much to handle on my linux machine (8192 frames seems to be just fine)
	// - Delay is bad when streaming is too slow, so we need stop it entirely -> Sleep seems to be very expensive on linux (Scheduler granularity?)

	// On fast machines we want:
	// - High delay when we are too fast
	// - Increase frames to stream in more data per loop

	// Audio characteristics table
	// Number of frames to stream | Delay | Ignore wait
	// ------------------------------------------------
	//   2048                     | 1     | YES
	//   2048                     | 1     | YES
	//   4192                     | 1     | YES
	//   4192                     | 2     | YES
	//   8192                     | 2     | YES
	//   8192                     | 4     | YES
	//  16384                     | 4     | NO
	//  16384                     | 6     | NO
	//  32768                     | 6     | NO
	//  32768                     | 8     | NO
	//  65536                     | 8     | NO
	//  65536                     | 10    | NO
	// 131072                     | 10    | NO
	// 131072                     | 15    | NO
	// 262144                     | 15    | NO
	// 262144                     | 20    | NO

	const AudioFrameDelayEntry entries[] = {
		fplStructInit(AudioFrameDelayEntry, 2048, 1, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 2, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 4, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 6, true),
		fplStructInit(AudioFrameDelayEntry, 2048, 8, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 1, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 2, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 4, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 6, true),
		fplStructInit(AudioFrameDelayEntry, 4192, 8, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 2, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 4, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 6, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 8, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 10, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 12, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 15, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 20, true),
		fplStructInit(AudioFrameDelayEntry, 8192, 25, true),
		fplStructInit(AudioFrameDelayEntry, 16384, 4, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 6, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 8, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 10, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 12, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 15, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 20, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 25, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 50, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 75, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 100, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 150, false),
		fplStructInit(AudioFrameDelayEntry, 16384, 200, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 4, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 6, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 8, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 10, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 12, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 15, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 20, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 25, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 50, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 75, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 100, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 150, false),
		fplStructInit(AudioFrameDelayEntry, 32768, 200, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 6, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 8, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 10, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 12, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 15, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 20, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 25, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 50, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 75, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 100, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 150, false),
		fplStructInit(AudioFrameDelayEntry, 65536, 200, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 10, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 15, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 20, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 25, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 50, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 75, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 100, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 150, false),
		fplStructInit(AudioFrameDelayEntry, 131072, 200, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 15, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 20, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 25, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 50, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 100, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 150, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 200, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 300, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 400, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 500, false),
		fplStructInit(AudioFrameDelayEntry, 262144, 1000, false),
	};

	const uint32_t intitialIndex = 0;

	uint32_t entryIndex = intitialIndex;
	AudioFrameDelayEntry currentEntry = entries[entryIndex];

	LockFreeRingBuffer *circularBuffer = &demo->outputRingBuffer;

	// Depending on the performance we can jump in table rows instead of dynamically computing stuff?

	const uint64_t totalBufferLength = circularBuffer->length;

	const float minBufferThreshold = 0.25f; // In percentage range of 0 to 1
	const float maxBufferThreshold = 0.75f; // In percentage range of 0 to 1

	// How often the thread looks for a new track when it does not mix
	const uint32_t trackLoadPollMilliseconds = 10;

	// @TODO(final): Move the file stream loading code into the audio demo

	bool ignoreWait = false;
	uint64_t startTime = fplMillisecondsQuery();
	while(!demo->isStreamingThreadStopped) {
		uint32_t trackChangeCount = fplAtomicLoadU32(&demo->trackChangeCount);
		ProcessTrackRequests(demo);
		// The output ring is empty after a track switch, it fills from the first entry on without waiting
		if(fplAtomicLoadU32(&demo->trackChangeCount) != trackChangeCount) {
			startTime = fplMillisecondsQuery();
			entryIndex = intitialIndex;
			currentEntry = entries[entryIndex];
			ignoreWait = true;
		}

		// No audio track?
		if (!HasAudioTrack(&demo->trackList)) {
			fplThreadSleep(100);
			continue;
		}

		// Outside thread mode the device callback mixes, this thread only loads the tracks
		if (demo->playbackMode != PlaybackMode_Thread) {
			fplThreadSleep(trackLoadPollMilliseconds);
			continue;
		}

		// Nothing is mixed while paused, the output ring runs empty and the device callback counts no frames
		if (fplAtomicLoadU32(&demo->isPaused) != 0) {
			fplThreadSleep(trackLoadPollMilliseconds);
			continue;
		}

		// Wait if needed
		bool wait = !ignoreWait;
		if(wait || !currentEntry.canIgnoreWait) {
			uint64_t deltaTime = fplMillisecondsQuery() - startTime;
			if(deltaTime < currentEntry.delay) {
				fplThreadSleep(1);
				continue;
			}
			startTime = fplMillisecondsQuery();
		}

		if(ignoreWait) {
			// We just want to ignore waiting once
			ignoreWait = false;
		}

		bool tooSlow = false;
		bool tooFast = false;

		// Where the current track ends in the stream, for the play mode: the write that mixes its end fills the rest with silence, up to the whole chunk
		AudioFrameIndex remainingTrackFrameCount = 0;
		if(GetRemainingPlayItemFrameCount(demo, &remainingTrackFrameCount)) {
			uint32_t streamedFrameCount = fplAtomicLoadU32(&demo->numFramesStreamed);
			demo->streamedTrackEndFrameCount = streamedFrameCount + remainingTrackFrameCount;
		} else {
			demo->streamedTrackEndFrameCount = UINT32_MAX;
		}

		uint64_t streamDuration = 0;
		if(WriteAudioToRingBuffer(&demo->targetAudioFormat, currentEntry.frames, demo, &streamDuration)) {
			if(streamDuration > currentEntry.delay) {
				// We are taking too slow to stream in new audio samples
				tooSlow = true;
			}
		}

		uint64_t fillCount = (uint64_t)fplAtomicLoadS64(&circularBuffer->fillCount);
		float percentageFilled = (1.0f / (float)totalBufferLength) * (float)fillCount;
		if(percentageFilled < minBufferThreshold) {
			// We are not filling the buffer fast enough, maybe due to streaming slowness
			tooSlow = true;
		} else if(percentageFilled > maxBufferThreshold) {
			// We are too fast
			tooFast = true;
		}

		if(tooSlow) {
			// Go back one characteristics entry
			if(entryIndex > 0) {
				currentEntry = entries[--entryIndex];
				if(currentEntry.canIgnoreWait) {
					ignoreWait = true;
				}
			} else {
				ignoreWait = true; // We are the worst entry, ignore any waiting
			}
		} else if(tooFast) {
			// Go forward one characteristics entry
			if(entryIndex < (fplArrayCount(entries) - 1)) {
				currentEntry = entries[++entryIndex];
				if(!currentEntry.canIgnoreWait) {
					ignoreWait = false;
				}
			} else {
				ignoreWait = false; // We are the max entry, never ignore waiting
			}
		}
	}
}

static void ReleaseStreamBuffers(AudioDemo *demo) {
	FreeAudioBuffer(&demo->audioSys.memory, &demo->outputTempBuffer);
	LockFreeRingBufferRelease(&demo->outputRingBuffer);
}

static bool InitializeStreamBuffers(AudioDemo *demo, AudioFrameIndex *frameCount) {
	bool bufferInitRes;

	// Init streaming buffer and read some frames at the very start
	AudioFrameIndex streamBufferFrames = fplGetAudioBufferSizeInFrames(demo->targetAudioFormat.sampleRate, demo->options.ringMilliseconds);
	size_t streamBufferSize = fplGetAudioBufferSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels, streamBufferFrames);
	bufferInitRes = LockFreeRingBufferInit(&demo->outputRingBuffer, streamBufferSize, true);
	if(!bufferInitRes) {
		return(false);
	}

	// Allocate temporary stream buffer, as large as the ring really is: the mirrored ring rounds its size up to whole memory pages and offers that much space
	size_t frameSize = fplGetAudioFrameSizeInBytes(demo->targetAudioFormat.type, demo->targetAudioFormat.channels);
	AudioFrameIndex ringFrameCount = (AudioFrameIndex)(demo->outputRingBuffer.length / frameSize);
	AudioFormat streamTempBufferFormat = fplZeroInit;
	streamTempBufferFormat.channels = demo->targetAudioFormat.channels;
	streamTempBufferFormat.format = demo->targetAudioFormat.type;
	streamTempBufferFormat.sampleRate = demo->targetAudioFormat.sampleRate;
	bufferInitRes = AllocateAudioBuffer(&demo->audioSys.memory, &demo->outputTempBuffer, &streamTempBufferFormat, ringFrameCount);
	if(!bufferInitRes) {
		return(false);
	}

	*frameCount = streamBufferFrames;

	return(true);
}

static void ReleaseVisualization(AudioDemo *demo) {

}

static void FillFrequencyBins(const uint32_t binCount, const uint32_t sampleRate, double *bins) {
	const double nyquist = sampleRate * 0.5;
	fplAssert(binCount == fplArrayCount(AudibleFrequencyRanges));
	for (uint32_t i = 0; i < binCount; i++) {
		bins[i] = fplMin(AudibleFrequencyRanges[i], nyquist);
	}
}

static void GenerateFrequencyBins(const uint32_t binCount, const uint32_t sampleRate, double *bins) {
	const double nyquist = sampleRate * 0.5;
	// Linear distribution: bin i covers [i*nyquist/N, (i+1)*nyquist/N)
	// This ensures every bin maps to the same number of FFT bins and a sweep
	// from 0 to Nyquist visually moves left to right across all bars.
	for (uint32_t i = 0; i <= binCount; i++) {
		bins[i] = (double)i / (double)binCount * nyquist;
	}
}

static bool InitializeVisualization(AudioDemo *demo) {
	// Initialize frequency bins
	//FillFrequencyBins(MAX_AUDIO_BIN_COUNT, demo->targetAudioFormat.sampleRate, demo->visualization.bins);
	GenerateFrequencyBins(MAX_AUDIO_BIN_COUNT, demo->targetAudioFormat.sampleRate, demo->visualization.bins);

	// Init window coefficients
	uint32_t N = fplArrayCount(demo->visualization.fftInput);
	HammingWindowFunction(demo->visualization.windowCoeffs, N);

	return(true);
}

static void TestAudioMath() {
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(0, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(22050, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(0, 1000) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(22050, 1000) == 22050);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(44100, 1000) == 44100);
	fplAlwaysAssert(fplGetAudioBufferSizeInFrames(48000, 1000) == 48000);

	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(0, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(0, 22050) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(22050, 0) == 0);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(22050, 22050) == 1000);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(44100, 44100) == 1000);
	fplAlwaysAssert(fplGetAudioBufferSizeInMilliseconds(48000, 48000) == 1000);
}

static void SetupAudioBackendAndFormat(fplAudioSettings *audio) {
	// Overwrite number of channels and/or layout
	audio->targetFormat.channelLayout = fplAudioChannelLayout_Stereo;

	// Overwrite audio device format / sample format
	//audio->targetFormat.type = fplAudioFormatType_U8;
	//audio->targetFormat.type = fplAudioFormatType_S16;
	//audio->targetFormat.type = fplAudioFormatType_S24;
	//audio->targetFormat.type = fplAudioFormatType_S32;
	//audio->targetFormat.type = fplAudioFormatType_S64;
	//audio->targetFormat.type = fplAudioFormatType_F32;
	//audio->targetFormat.type = fplAudioFormatType_F64;

	// Overwrite samplerate in Hz
	//audio->targetFormat.sampleRate = 11025;
	//audio->targetFormat.sampleRate = 22050;
	//audio->targetFormat.sampleRate = 44100;
	//audio->targetFormat.sampleRate = 48000;
	//audio->targetFormat.sampleRate = 88200;
	//audio->targetFormat.sampleRate = 96000;

	// Overwrite buffer size in milliseconds or in frames
	//audio->targetFormat.bufferSizeInMilliseconds = 16;
	//audio->targetFormat.bufferSizeInFrames = 512;

	// Overwrite audio backend to a specific type
	//audio->backend = fplAudioBackendType_WASAPI;
	//audio->backend = fplAudioBackendType_DirectSound;
	//audio->backend = fplAudioBackendType_PipeWire;
	//audio->backend = fplAudioBackendType_PulseAudio;
	//audio->backend = fplAudioBackendType_Alsa;
	//audio->backend = fplAudioBackendType_OSS;
}

//
// Keys, the main thread only leaves requests or calls the thread-safe setters of the audio system, every action logs its new value
//

static void RequestTrackStep(AudioDemo *demo, const int32_t trackStep) {
	if(demo->playbackMode == PlaybackMode_DeviceSine) {
		FPL_LOG_INFO("Keys", "There are no tracks in device-sine mode");
		return;
	}
	fplAtomicFetchAndAddS32(&demo->pendingTrackStep, trackStep);
	FPL_LOG_INFO("Keys", "%s track", (trackStep > 0) ? "Next" : "Previous");
}

// Only thread mode has the full-track buffer, the other modes always visualize the real-time samples
static void ToggleRealTimeVisualization(AudioDemo *demo) {
	if(demo->playbackMode != PlaybackMode_Thread) {
		FPL_LOG_INFO("Keys", "Visualization: real-time only, the full-track buffer exists in thread mode only");
		return;
	}
	demo->useRealTimeSamples = !demo->useRealTimeSamples;
	FPL_LOG_INFO("Keys", "Visualization: %s", demo->useRealTimeSamples ? "real-time samples" : "full-track buffer");
}

static AudioPlayItemID GetCurrentPlayItemId(AudioDemo *demo) {
	AudioPlayItemID result = fplZeroInit;
	result.value = fplAtomicLoadU64(&demo->currentPlayItemId);
	return(result);
}

// Every track start resumes
static void TogglePause(AudioDemo *demo) {
	AudioPlayItemID playID = GetCurrentPlayItemId(demo);
	if(playID.value == 0) {
		FPL_LOG_INFO("Keys", "Pause: no track plays");
		return;
	}
	uint32_t isPaused = fplAtomicLoadU32(&demo->isPaused);
	uint32_t newPaused = (isPaused != 0) ? 0 : 1;
	fplAtomicStoreU32(&demo->isPaused, newPaused);
	FPL_LOG_INFO("Keys", "Pause: %s", (newPaused != 0) ? "on" : "off");
}

// For the current track and every track after it
static void CyclePlayMode(AudioDemo *demo) {
	fplMutexLock(&demo->trackLock);
	PlayMode playMode = (PlayMode)((demo->controls.playMode + 1) % PlayMode_Count);
	demo->controls.playMode = playMode;
	fplMutexUnlock(&demo->trackLock);
	FPL_LOG_INFO("Keys", "Play mode: %s", PlayModeChoices[playMode].name);
}

// For the current track and every track after it
static void ToggleRepeat(AudioDemo *demo) {
	fplMutexLock(&demo->trackLock);
	demo->controls.isRepeat = !demo->controls.isRepeat;
	bool isRepeat = demo->controls.isRepeat;
	fplMutexUnlock(&demo->trackLock);

	AudioPlayItemID playID = GetCurrentPlayItemId(demo);
	bool isApplied = (playID.value != 0) && AudioSystemSetPlayItemRepeat(&demo->audioSys, playID, isRepeat);
	if(isApplied) {
		fplAtomicStoreU32(&demo->isCurrentTrackRepeating, isRepeat ? 1 : 0);
	}
	FPL_LOG_INFO("Keys", "Repeat: %s%s", isRepeat ? "on" : "off", isApplied ? "" : " (from the next track on)");
}

// For the current track and every track after it, in steps of 0.1 between 0 and 2
static void ChangeVolume(AudioDemo *demo, const float volumeStep) {
	const float maxVolume = 2.0f;
	const float stepsPerVolume = 10.0f;

	fplMutexLock(&demo->trackLock);
	float changedVolume = demo->controls.volume + volumeStep;
	float roundedVolume = roundf(changedVolume * stepsPerVolume) / stepsPerVolume;
	float limitedVolume = fplMin(roundedVolume, maxVolume);
	float volume = fplMax(0.0f, limitedVolume);
	demo->controls.volume = volume;
	fplMutexUnlock(&demo->trackLock);

	AudioPlayItemID playID = GetCurrentPlayItemId(demo);
	bool isApplied = (playID.value != 0) && AudioSystemSetPlayItemVolume(&demo->audioSys, playID, volume);
	FPL_LOG_INFO("Keys", "Volume: %.1f%s", volume, isApplied ? "" : " (from the next track on)");
}

typedef struct PlayerKeyHelp {
	const char *keys;
	const char *description;
} PlayerKeyHelp;

static const PlayerKeyHelp PlayerKeyHelps[] = {
	{ "N, P, media next/prev", "Next or previous track of the command line" },
	{ "Space, media play/pause", "Pause and resume" },
	{ "R", "Repeat on and off, for the next tracks too" },
	{ "M", "Play mode off, list or loop, what follows a track that has played to its end" },
	{ "Up, Down", "Volume +0.1 and -0.1, for the next tracks too" },
	{ "F, F1, F2", "Fullscreen, real-time visualization (thread mode only), spectrum mode" },
};

//
// Command line
//

typedef enum PlayerOptionType {
	// No value, sets a bool
	PlayerOptionType_Flag = 0,
	// Copies the value into a char array
	PlayerOptionType_String,
	// Stores an int32_t
	PlayerOptionType_Int,
	// Stores a float
	PlayerOptionType_Float,
	// One name out of a list, stores its index as int32_t
	PlayerOptionType_Choice,
	// No option, a heading line in the help (the description)
	PlayerOptionType_Heading,
} PlayerOptionType;

// Same order as PlaybackMode
static const PlayerOptionChoice MixerModeChoices[] = {
	{ "thread", "the streaming thread mixes into the output ring, the device callback only copies" },
	{ "callback", "the device callback mixes itself" },
	{ "device-sine", "the device callback generates a sine wave, no audio system involved" },
};
fplStaticAssert(fplArrayCount(MixerModeChoices) == PlaybackMode_Count);

// Same order as RenderFormat
static const PlayerOptionChoice RenderFormatChoices[] = {
	{ "s16", "16-bit signed integer" },
	{ "f32", "32-bit float" },
};
fplStaticAssert(fplArrayCount(RenderFormatChoices) == RenderFormat_Count);
fplStaticAssert(fplArrayCount(RenderFormatTypes) == RenderFormat_Count);

typedef struct PlayerOptionDefinition {
	PlayerOptionType type;
	const char *name;
	// Shown in the help as --name=<valueName>, a choice shows its names instead
	const char *valueName;
	size_t fieldOffset;
	size_t fieldSize;
	const PlayerOptionChoice *choices;
	size_t choiceCount;
	const char *description;
} PlayerOptionDefinition;

#define PLAYER_OPTION_FIELD(field) offsetof(PlayerOptions, field), sizeof(((PlayerOptions *)0)->field)

// A new option is one line here and a field in PlayerOptions, the help and the log are generated from this table
static const PlayerOptionDefinition PlayerOptionDefinitions[] = {
	{ PlayerOptionType_Flag, "help", fpl_null, PLAYER_OPTION_FIELD(isHelp), fpl_null, 0, "Print this help and exit" },
	{ PlayerOptionType_String, "device", "<name>", PLAYER_OPTION_FIELD(deviceName), fpl_null, 0, "Play on the audio device with exactly this name, stop with an error when there is none" },
	{ PlayerOptionType_Flag, "sine", fpl_null, PLAYER_OPTION_FIELD(isSine), fpl_null, 0, "Play only the generated sine wave, no files and no built-in music" },
	{ PlayerOptionType_Float, "sine-seconds", "<s>", PLAYER_OPTION_FIELD(sineSeconds), fpl_null, 0, "Length of the sine wave in seconds" },
	{ PlayerOptionType_Choice, "mixer", fpl_null, PLAYER_OPTION_FIELD(mixerMode), MixerModeChoices, fplArrayCount(MixerModeChoices), "Where the audio system mixes:" },
	{ PlayerOptionType_Int, "ring-ms", "<ms>", PLAYER_OPTION_FIELD(ringMilliseconds), fpl_null, 0, "Size of the output ring in thread mode, every change is heard only after its latency" },
	{ PlayerOptionType_Float, "volume", "<v>", PLAYER_OPTION_FIELD(volume), fpl_null, 0, "Volume of the play item" },
	{ PlayerOptionType_Float, "pitch", "<p>", PLAYER_OPTION_FIELD(pitch), fpl_null, 0, "Pitch of the play item, 2 is an octave up and half as long" },
	{ PlayerOptionType_Flag, "repeat", fpl_null, PLAYER_OPTION_FIELD(isRepeat), fpl_null, 0, "Loop the track" },
	{ PlayerOptionType_Choice, "play-mode", fpl_null, PLAYER_OPTION_FIELD(playMode), PlayModeChoices, fplArrayCount(PlayModeChoices), "What follows a track that has played to its end, a repeating track never ends:" },
	{ PlayerOptionType_Flag, "stats", fpl_null, PLAYER_OPTION_FIELD(isStats), fpl_null, 0, "Log memory and mixer cost every 5 seconds" },
	{ PlayerOptionType_Heading, fpl_null, fpl_null, 0, 0, fpl_null, 0, "Offline render (no window, no audio device):" },
	{ PlayerOptionType_Flag, "render", fpl_null, PLAYER_OPTION_FIELD(isRender), fpl_null, 0, "Mix the first track in fixed chunks and print a hash of the samples" },
	{ PlayerOptionType_String, "render-wav", "<file.wav>", PLAYER_OPTION_FIELD(renderWavePath), fpl_null, 0, "Like --render, and also write the samples into a wave file" },
	{ PlayerOptionType_Int, "chunk", "<frames>", PLAYER_OPTION_FIELD(renderChunkFrameCount), fpl_null, 0, "Frames per AudioSystemWriteFrames call" },
	{ PlayerOptionType_Int, "rate", "<hz>", PLAYER_OPTION_FIELD(renderSampleRate), fpl_null, 0, "Output sample rate" },
	{ PlayerOptionType_Choice, "format", fpl_null, PLAYER_OPTION_FIELD(renderFormat), RenderFormatChoices, fplArrayCount(RenderFormatChoices), "Output sample format:" },
	{ PlayerOptionType_Int, "channels", "<n>", PLAYER_OPTION_FIELD(renderChannelCount), fpl_null, 0, "Output channel count" },
	{ PlayerOptionType_Float, "seconds", "<s>", PLAYER_OPTION_FIELD(renderSeconds), fpl_null, 0, "Length to render, 0 is the track length, with --repeat twice that" },
};

static const char PlayerOptionPrefix[] = "--";

static PlayerOptions MakeDefaultPlayerOptions(void) {
	const int32_t defaultRingMilliseconds = 10000;
	const float defaultSineSeconds = 10.0f;
	const float defaultVolume = 1.0f;
	const int32_t defaultRenderChunkFrameCount = 512;
	const int32_t defaultRenderSampleRate = 48000;
	const int32_t defaultRenderChannelCount = 2;
	PlayerOptions result = fplZeroInit;
	result.mixerMode = PlaybackMode_Thread;
	result.playMode = PlayMode_List;
	result.ringMilliseconds = defaultRingMilliseconds;
	result.sineSeconds = defaultSineSeconds;
	result.volume = defaultVolume;
	result.pitch = AudioSystemDefaultPitch;
	result.renderChunkFrameCount = defaultRenderChunkFrameCount;
	result.renderSampleRate = defaultRenderSampleRate;
	result.renderFormat = RenderFormat_S16;
	result.renderChannelCount = defaultRenderChannelCount;
	return(result);
}

static void FormatPlayerOptionValue(const PlayerOptionDefinition *definition, const PlayerOptions *options, char *buffer, const size_t bufferSize) {
	const uint8_t *field = (const uint8_t *)options + definition->fieldOffset;
	switch(definition->type) {
		case PlayerOptionType_Flag:
		{
			const bool *value = (const bool *)field;
			fplCopyString(*value ? "on" : "off", buffer, bufferSize);
		} break;

		case PlayerOptionType_String:
		{
			fplCopyString((const char *)field, buffer, bufferSize);
		} break;

		case PlayerOptionType_Int:
		{
			const int32_t *value = (const int32_t *)field;
			fplStringFormat(buffer, bufferSize, "%d", *value);
		} break;

		case PlayerOptionType_Float:
		{
			const float *value = (const float *)field;
			fplStringFormat(buffer, bufferSize, "%g", (double)*value);
		} break;

		case PlayerOptionType_Choice:
		{
			const int32_t *choiceIndex = (const int32_t *)field;
			fplAssert(*choiceIndex >= 0 && *choiceIndex < (int32_t)definition->choiceCount);
			fplCopyString(definition->choices[*choiceIndex].name, buffer, bufferSize);
		} break;

		default:
			FPL_NOT_IMPLEMENTED;
			break;
	}
}

// --name=<value>, or --name=a|b|c for a choice
static void FormatPlayerOptionSyntax(const PlayerOptionDefinition *definition, char *buffer, const size_t bufferSize) {
	size_t length = fplStringFormat(buffer, bufferSize, "%s%s", PlayerOptionPrefix, definition->name);
	if(definition->valueName != fpl_null) {
		fplStringFormat(buffer + length, bufferSize - length, "=%s", definition->valueName);
	}
	for(size_t choiceIndex = 0; choiceIndex < definition->choiceCount; ++choiceIndex) {
		const char *separator = (choiceIndex == 0) ? "=" : "|";
		length += fplStringFormat(buffer + length, bufferSize - length, "%s%s", separator, definition->choices[choiceIndex].name);
	}
}

static void PrintPlayerHelp(void) {
	const int descriptionColumn = 28;
	const PlayerOptions defaults = MakeDefaultPlayerOptions();
	fplConsoleFormatOut("FPL_AudioPlayer [options] [file ...]\n\n");
	for(size_t definitionIndex = 0; definitionIndex < fplArrayCount(PlayerOptionDefinitions); ++definitionIndex) {
		const PlayerOptionDefinition *definition = &PlayerOptionDefinitions[definitionIndex];
		if(definition->type == PlayerOptionType_Heading) {
			fplConsoleFormatOut("\n%s\n", definition->description);
			continue;
		}

		char syntax[128];
		FormatPlayerOptionSyntax(definition, syntax, fplArrayCount(syntax));

		// Numbers show their default, a choice marks its default line instead
		char defaultText[64] = fplZeroInit;
		if(definition->type == PlayerOptionType_Int || definition->type == PlayerOptionType_Float) {
			char defaultValue[32];
			FormatPlayerOptionValue(definition, &defaults, defaultValue, fplArrayCount(defaultValue));
			fplStringFormat(defaultText, fplArrayCount(defaultText), " (default %s)", defaultValue);
		}

		// A long syntax gets a line of its own, the description starts below it
		size_t syntaxLength = fplGetStringLength(syntax);
		if(syntaxLength < (size_t)descriptionColumn) {
			fplConsoleFormatOut("  %-*s%s%s\n", descriptionColumn, syntax, definition->description, defaultText);
		} else {
			fplConsoleFormatOut("  %s\n", syntax);
			fplConsoleFormatOut("  %-*s%s%s\n", descriptionColumn, "", definition->description, defaultText);
		}

		int choiceNameWidth = 0;
		for(size_t choiceIndex = 0; choiceIndex < definition->choiceCount; ++choiceIndex) {
			size_t choiceNameLength = fplGetStringLength(definition->choices[choiceIndex].name);
			choiceNameWidth = fplMax(choiceNameWidth, (int)choiceNameLength);
		}
		for(size_t choiceIndex = 0; choiceIndex < definition->choiceCount; ++choiceIndex) {
			const PlayerOptionChoice *choice = &definition->choices[choiceIndex];
			const int32_t *defaultChoiceIndex = (const int32_t *)((const uint8_t *)&defaults + definition->fieldOffset);
			const char *defaultMark = (*defaultChoiceIndex == (int32_t)choiceIndex) ? " (default)" : "";
			size_t choiceNameLength = fplGetStringLength(choice->name);
			int choiceNamePadding = choiceNameWidth - (int)choiceNameLength;
			fplConsoleFormatOut("  %-*s  %s:%-*s %s%s\n", descriptionColumn, "", choice->name, choiceNamePadding, "", choice->description, defaultMark);
		}
	}
	fplConsoleFormatOut("\nEvery other argument is a file to play, without one the built-in music plays.\n");

	fplConsoleFormatOut("\nKeys in the window:\n");
	for(size_t keyIndex = 0; keyIndex < fplArrayCount(PlayerKeyHelps); ++keyIndex) {
		const PlayerKeyHelp *keyHelp = &PlayerKeyHelps[keyIndex];
		fplConsoleFormatOut("  %-*s%s\n", descriptionColumn, keyHelp->keys, keyHelp->description);
	}
	fplConsoleFormatOut("In thread mode a key is heard only after the latency of the output ring, --mixer=callback or a small --ring-ms hear it at once.\n");
}

static const PlayerOptionDefinition *FindPlayerOptionDefinition(const char *name, const size_t nameLength) {
	for(size_t definitionIndex = 0; definitionIndex < fplArrayCount(PlayerOptionDefinitions); ++definitionIndex) {
		const PlayerOptionDefinition *definition = &PlayerOptionDefinitions[definitionIndex];
		if(definition->type == PlayerOptionType_Heading) {
			continue;
		}
		size_t definitionNameLength = fplGetStringLength(definition->name);
		if(fplIsStringEqualLen(name, nameLength, definition->name, definitionNameLength)) {
			return(definition);
		}
	}
	return(fpl_null);
}

// The value is null when the argument has no =
static bool SetPlayerOptionValue(const PlayerOptionDefinition *definition, const char *value, PlayerOptions *options) {
	uint8_t *field = (uint8_t *)options + definition->fieldOffset;

	if(definition->type == PlayerOptionType_Flag) {
		if(value != fpl_null) {
			fplConsoleFormatError("%s%s takes no value\n", PlayerOptionPrefix, definition->name);
			return(false);
		}
		bool *flag = (bool *)field;
		*flag = true;
		return(true);
	}

	size_t valueLength = fplGetStringLength(value);
	if(valueLength == 0) {
		char syntax[128];
		FormatPlayerOptionSyntax(definition, syntax, fplArrayCount(syntax));
		fplConsoleFormatError("%s%s needs a value: %s\n", PlayerOptionPrefix, definition->name, syntax);
		return(false);
	}

	switch(definition->type) {
		case PlayerOptionType_String:
		{
			if(valueLength >= definition->fieldSize) {
				fplConsoleFormatError("%s%s is longer than %zu characters\n", PlayerOptionPrefix, definition->name, definition->fieldSize - 1);
				return(false);
			}
			fplCopyString(value, (char *)field, definition->fieldSize);
		} break;

		case PlayerOptionType_Int:
		{
			int32_t *number = (int32_t *)field;
			if(!fplTryStringToS32(value, number)) {
				fplConsoleFormatError("%s%s needs a whole number, not '%s'\n", PlayerOptionPrefix, definition->name, value);
				return(false);
			}
		} break;

		case PlayerOptionType_Float:
		{
			char *numberEnd = fpl_null;
			double number = strtod(value, &numberEnd);
			if(numberEnd == value || *numberEnd != 0 || !isfinite(number)) {
				fplConsoleFormatError("%s%s needs a number, not '%s'\n", PlayerOptionPrefix, definition->name, value);
				return(false);
			}
			float *floatField = (float *)field;
			*floatField = (float)number;
		} break;

		case PlayerOptionType_Choice:
		{
			int32_t *choiceIndexField = (int32_t *)field;
			for(size_t choiceIndex = 0; choiceIndex < definition->choiceCount; ++choiceIndex) {
				if(fplIsStringEqual(value, definition->choices[choiceIndex].name)) {
					*choiceIndexField = (int32_t)choiceIndex;
					return(true);
				}
			}
			char syntax[128];
			FormatPlayerOptionSyntax(definition, syntax, fplArrayCount(syntax));
			fplConsoleFormatError("%s%s does not know '%s': %s\n", PlayerOptionPrefix, definition->name, value, syntax);
			return(false);
		} break;

		default:
			FPL_NOT_IMPLEMENTED;
			break;
	}
	return(true);
}

/**
* @brief Parses the options and files from the command line.
* @param[in] argc The argument count of main.
* @param[in] args The arguments of main.
* @param[in, out] options The options, filled with the defaults before.
* @return Returns false when an argument is wrong, the error is printed already.
*/
static bool ParsePlayerOptions(const int argc, char **args, PlayerOptions *options) {
	const size_t prefixLength = fplGetStringLength(PlayerOptionPrefix);
	for(int argIndex = 1; argIndex < argc; ++argIndex) {
		const char *argument = args[argIndex];
		size_t argumentLength = fplGetStringLength(argument);

		bool isOption = (argumentLength > prefixLength) && fplIsStringEqualLen(argument, prefixLength, PlayerOptionPrefix, prefixLength);
		if(!isOption) {
			if(options->fileCount < fplArrayCount(options->files)) {
				options->files[options->fileCount++] = argument;
			} else {
				FPL_LOG_WARN("Options", "More than %zu files, '%s' is ignored", fplArrayCount(options->files), argument);
			}
			continue;
		}

		const char *name = argument + prefixLength;
		size_t nameLength = 0;
		while(name[nameLength] != 0 && name[nameLength] != '=') {
			++nameLength;
		}
		const char *value = (name[nameLength] == '=') ? (name + nameLength + 1) : fpl_null;

		const PlayerOptionDefinition *definition = FindPlayerOptionDefinition(name, nameLength);
		if(definition == fpl_null) {
			fplConsoleFormatError("Unknown option '%s'\n\n", argument);
			PrintPlayerHelp();
			return(false);
		}
		if(!SetPlayerOptionValue(definition, value, options)) {
			fplConsoleFormatError("See %shelp for all options\n", PlayerOptionPrefix);
			return(false);
		}
	}
	return(true);
}

static bool ValidatePlayerOptions(const PlayerOptions *options) {
	if(options->sineSeconds <= 0.0f) {
		fplConsoleFormatError("--sine-seconds must be above 0, an endless sine wave (0) comes with the streaming source (--stream), which does not exist yet\n");
		return(false);
	}
	if(options->ringMilliseconds <= 0) {
		fplConsoleFormatError("--ring-ms must be above 0\n");
		return(false);
	}
	if(options->volume < 0.0f) {
		fplConsoleFormatError("--volume must not be negative\n");
		return(false);
	}
	if(options->pitch <= 0.0f) {
		fplConsoleFormatError("--pitch must be above 0\n");
		return(false);
	}
	if(options->isSine && options->fileCount > 0) {
		fplConsoleFormatError("--sine plays only the sine wave, it takes no files\n");
		return(false);
	}
	if(options->isRender) {
		if(options->renderChunkFrameCount <= 0) {
			fplConsoleFormatError("--chunk must be above 0\n");
			return(false);
		}
		if(options->renderSampleRate <= 0) {
			fplConsoleFormatError("--rate must be above 0\n");
			return(false);
		}
		if(options->renderChannelCount <= 0 || options->renderChannelCount > (int32_t)MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT) {
			fplConsoleFormatError("--channels must be between 1 and %d\n", (int)MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT);
			return(false);
		}
		if(options->renderSeconds < 0.0f) {
			fplConsoleFormatError("--seconds must not be negative\n");
			return(false);
		}
	}
	return(true);
}

static void LogPlayerOptions(const PlayerOptions *options) {
	for(size_t definitionIndex = 0; definitionIndex < fplArrayCount(PlayerOptionDefinitions); ++definitionIndex) {
		const PlayerOptionDefinition *definition = &PlayerOptionDefinitions[definitionIndex];
		if(definition->type == PlayerOptionType_Heading || definition->fieldOffset == offsetof(PlayerOptions, isHelp)) {
			continue;
		}
		char value[FPL_MAX_NAME_LENGTH];
		FormatPlayerOptionValue(definition, options, value, fplArrayCount(value));
		FPL_LOG_INFO("Options", "%s%s=%s", PlayerOptionPrefix, definition->name, value);
	}
	for(size_t fileIndex = 0; fileIndex < options->fileCount; ++fileIndex) {
		FPL_LOG_INFO("Options", "file[%zu]=%s", fileIndex, options->files[fileIndex]);
	}
}

/**
* @brief Sets the audio device with exactly the given name as target device.
* @param[in] deviceName The device name from --device, empty for no target device: the platform then plays on the system default device.
* @param[in, out] settings The settings, used for a short audio-only initialization that lists the devices.
* @return Returns false when there is no device with this name, it never falls back to another device.
*/
static bool SelectAudioDevice(const char *deviceName, fplSettings *settings) {
	size_t deviceNameLength = fplGetStringLength(deviceName);
	if(deviceNameLength == 0) {
		return(true);
	}

	if(!fplPlatformInit(fplInitFlags_Audio, settings)) {
		FPL_LOG_ERROR("Audio", "Failed to initialize the audio to find the device '%s'", deviceName);
		return(false);
	}

	bool isFound = false;
	uint32_t deviceCount = fplGetAudioDevices(0, 0, fpl_null);
	if(deviceCount > 0) {
		fplAudioDeviceInfo *audioDeviceInfos = fplMemoryAllocate(sizeof(fplAudioDeviceInfo) * deviceCount);
		uint32_t loadedDeviceCount = fplGetAudioDevices(deviceCount, 0, audioDeviceInfos);
		fplAssert(loadedDeviceCount == deviceCount);
		for(uint32_t deviceIndex = 0; deviceIndex < loadedDeviceCount; ++deviceIndex) {
			const fplAudioDeviceInfo *audioDeviceInfo = audioDeviceInfos + deviceIndex;
			if(fplIsStringEqual(audioDeviceInfo->name, deviceName)) {
				FPL_LOG_INFO("Audio", "Using audio device[%u] %s", deviceIndex, audioDeviceInfo->name);
				settings->audio.targetDevice = *audioDeviceInfo;
				isFound = true;
				break;
			}
		}
		if(!isFound) {
			FPL_LOG_ERROR("Audio", "There is no audio device '%s', the %u devices are:", deviceName, loadedDeviceCount);
			for(uint32_t deviceIndex = 0; deviceIndex < loadedDeviceCount; ++deviceIndex) {
				const fplAudioDeviceInfo *audioDeviceInfo = audioDeviceInfos + deviceIndex;
				FPL_LOG_ERROR("Audio", "  [%u] %s", deviceIndex, audioDeviceInfo->name);
			}
		}
		fplMemoryFree(audioDeviceInfos);
	} else {
		FPL_LOG_ERROR("Audio", "There is no audio device '%s', there are no audio devices at all", deviceName);
	}

	fplPlatformRelease();
	return(isFound);
}

//
// Stats
//

// The histogram is null when the mixer did not run since the last line
static void LogStatsLine(AudioDemo *demo, const MixCostHistogram *histogram) {
	const double percentileShare = 0.99;
	const double percentScale = 100.0;

	ProcessMemoryUsage memoryUsage = GetProcessMemoryUsage();

	AudioSystemStats audioStats = fplZeroInit;
	AudioSystemGetStats(&demo->audioSys, &audioStats);

	// Only thread mode has an output ring
	char ringFillText[32] = "-";
	if(demo->playbackMode == PlaybackMode_Thread && demo->outputRingBuffer.length > 0) {
		int64_t fillCount = fplAtomicLoadS64(&demo->outputRingBuffer.fillCount);
		double ringFillPercent = (double)fillCount / (double)demo->outputRingBuffer.length * percentScale;
		fplStringFormat(ringFillText, fplArrayCount(ringFillText), "%.1f", ringFillPercent);
	}

	uint64_t callCount = 0;
	double meanMicroseconds = 0.0;
	double percentileMicroseconds = 0.0;
	double maxMicroseconds = 0.0;
	if(histogram != fpl_null) {
		callCount = histogram->callCount;
		meanMicroseconds = GetMixCostMean(histogram);
		percentileMicroseconds = GetMixCostPercentile(histogram, percentileShare);
		maxMicroseconds = histogram->maxMicroseconds;
	}
	uint32_t mixThreadId = fplAtomicLoadU32(&demo->mixCostStats.mixThreadId);

	fplConsoleFormatOut("stats: vm_hwm_kb=%llu vm_rss_kb=%llu sources=%zu source_bytes=%zu play_items=%u ring_fill_pct=%s mix_calls=%llu mix_mean_us=%.3f mix_p99_us=%.3f mix_max_us=%.3f mix_tid=%u\n", (unsigned long long)memoryUsage.highWaterKilobytes, (unsigned long long)memoryUsage.residentKilobytes, audioStats.sourceCount, audioStats.pcmByteCount, audioStats.activePlayItemCount, ringFillText, (unsigned long long)callCount, meanMicroseconds, percentileMicroseconds, maxMicroseconds, mixThreadId);
}

// Called every frame with --stats: asks the mixing thread for its histogram every 5 seconds and logs it as soon as the mixing thread swapped
static void UpdateStats(AudioDemo *demo, fplMilliseconds *lastRequestTime) {
	const fplMilliseconds statsIntervalMilliseconds = 5000;
	MixCostStats *stats = &demo->mixCostStats;

	uint32_t swapState = fplAtomicLoadU32(&stats->swapState);
	if(swapState == MixCostSwapState_Done) {
		uint32_t writeIndex = fplAtomicLoadU32(&stats->writeIndex);
		uint32_t readIndex = (writeIndex + 1) % MIX_COST_HISTOGRAM_COUNT;
		MixCostHistogram *histogram = &stats->histograms[readIndex];
		LogStatsLine(demo, histogram);
		fplClearStruct(histogram);
		fplAtomicStoreU32(&stats->swapState, MixCostSwapState_Idle);
		return;
	}

	fplMilliseconds now = fplMillisecondsQuery();
	if(now - *lastRequestTime < statsIntervalMilliseconds) {
		return;
	}
	*lastRequestTime = now;
	if(swapState == MixCostSwapState_Requested) {
		// The mixer did not run since the last request
		LogStatsLine(demo, fpl_null);
	} else {
		fplAtomicStoreU32(&stats->swapState, MixCostSwapState_Requested);
	}
}

//
// Offline render
//

// RIFF header with one fmt chunk (without cbSize) and one data chunk, the samples follow it
static bool WriteWaveFileHeader(const fplFileHandle *file, const fplAudioFormat *format, const AudioFrameIndex frameCount) {
	const uint32_t formatChunkSize = (uint32_t)(sizeof(WaveFormatEx) - sizeof(uint16_t));
	const uint32_t bitsPerByte = 8;
	uint32_t frameSize = (uint32_t)fplGetAudioFrameSizeInBytes(format->type, format->channels);
	uint32_t sampleSize = (uint32_t)fplGetAudioSampleSizeInBytes(format->type);
	uint32_t dataSize = frameSize * frameCount;

	WaveHeader header = fplZeroInit;
	header.chunkId = WaveChunkId_RIFF;
	header.chunkSize = (uint32_t)(sizeof(header.formatId) + sizeof(WaveChunk) + formatChunkSize + sizeof(WaveChunk)) + dataSize;
	header.formatId = WaveChunkId_WAVE;

	WaveChunk formatChunk = fplZeroInit;
	formatChunk.id = WaveChunkId_Format;
	formatChunk.size = formatChunkSize;

	WaveFormatEx waveFormat = fplZeroInit;
	waveFormat.formatTag = (format->type == fplAudioFormatType_F32) ? WaveFormatTags_IEEEFloat : WaveFormatTags_PCM;
	waveFormat.numberOfChannels = format->channels;
	waveFormat.samplesPerSecond = format->sampleRate;
	waveFormat.avgBytesPerSample = format->sampleRate * frameSize;
	waveFormat.blockAlign = (uint16_t)frameSize;
	waveFormat.bitsPerSample = (uint16_t)(sampleSize * bitsPerByte);

	WaveChunk dataChunk = fplZeroInit;
	dataChunk.id = WaveChunkId_Data;
	dataChunk.size = dataSize;

	bool result = (fplFileWriteBlock32(file, &header, sizeof(header)) == sizeof(header));
	result = result && (fplFileWriteBlock32(file, &formatChunk, sizeof(formatChunk)) == sizeof(formatChunk));
	result = result && (fplFileWriteBlock32(file, &waveFormat, formatChunkSize) == formatChunkSize);
	result = result && (fplFileWriteBlock32(file, &dataChunk, sizeof(dataChunk)) == sizeof(dataChunk));
	return(result);
}

/**
* @brief Mixes the first track on the main thread without window and audio device, always --chunk frames per AudioSystemWriteFrames call, and prints the result line with the hash of the samples.
* @param[in] options The options, --volume, --pitch and --repeat apply to the sine wave too.
* @return Returns the exit code of the player, 0 when the whole track was rendered.
*/
static int RenderOffline(const PlayerOptions *options) {
	const int renderFailedExitCode = 1;
	const double roundingOffset = 0.5;
	const double percentileShare = 0.99;
	const AudioFrameIndex repeatPassCount = 2;

	int result = renderFailedExitCode;

	fplSettings settings = fplMakeDefaultSettings();
	if(!fplPlatformInit(fplInitFlags_None, &settings)) {
		FPL_LOG_ERROR("Render", "Failed to initialize the platform");
		return(renderFailedExitCode);
	}

	AudioSystem *audioSys = (AudioSystem *)fplMemoryAllocate(sizeof(AudioSystem));
	AudioTrackList *trackList = (AudioTrackList *)fplMemoryAllocate(sizeof(AudioTrackList));
	MixCostHistogram *histogram = (MixCostHistogram *)fplMemoryAllocate(sizeof(MixCostHistogram));
	uint8_t *chunkSamples = fpl_null;
	fplFileHandle waveFile = fplZeroInit;
	bool isWaveFileOpen = false;
	bool isAudioSystemInitialized = false;
	if(audioSys == fpl_null || trackList == fpl_null || histogram == fpl_null) {
		goto done;
	}

	fplAudioFormat targetFormat = fplZeroInit;
	targetFormat.type = RenderFormatTypes[options->renderFormat];
	targetFormat.sampleRate = (uint32_t)options->renderSampleRate;
	targetFormat.channels = (uint16_t)options->renderChannelCount;
	if(!AudioSystemInit(audioSys, &targetFormat)) {
		FPL_LOG_ERROR("Render", "Failed to initialize the audio system");
		goto done;
	}
	isAudioSystemInitialized = true;

	// The track the player would play first: the sine wave, the first file or the built-in music
	AudioTrackSource trackSource = fplZeroInit;
	size_t trackSourceCount = 0;
	if(options->fileCount > 0) {
		if(!SetAudioTrackSourceFromFile(audioSys, &trackSource, options->files[0])) {
			goto done;
		}
		trackSourceCount = 1;
	} else if(!options->isSine) {
		if(!IsAudioSampleRateSupported(audioSys, sampleRate_musicTavsControlArgofox)) {
			FPL_LOG_ERROR("Render", "The built-in music cannot be converted to %u Hz", targetFormat.sampleRate);
			goto done;
		}
		trackSource.type = AudioTrackSourceType_Data;
		fplCopyString(name_musicTavsControlArgofox, trackSource.name, fplArrayCount(trackSource.name));
		trackSource.data.size = sizeOf_musicTavsControlArgofox;
		trackSource.data.data = ptr_musicTavsControlArgofox;
		trackSourceCount = 1;
	}

	AudioSineWaveData sineWave = MakeSineWave(options->sineSeconds);
	if(!LoadAudioTrackList(audioSys, &trackSource, trackSourceCount, options->isSine, &sineWave, LoadAudioTrackFlags_AutoLoad, trackList)) {
		goto done;
	}
	AudioSource *source = fpl_null;
	if(trackList->count > 0) {
		source = AudioSystemGetSourceByID(audioSys, trackList->tracks[0].sourceID);
	}
	if(source == fpl_null) {
		FPL_LOG_ERROR("Render", "Failed to load the track");
		goto done;
	}

	AudioPlayItemID playID = AudioSystemPlaySource(audioSys, source, options->isRepeat, options->volume, options->pitch);
	if(playID.value == 0) {
		FPL_LOG_ERROR("Render", "Failed to play the track");
		goto done;
	}

	AudioFrameIndex totalFrameCount;
	if(options->renderSeconds > 0.0f) {
		double requestedFrameCount = (double)options->renderSeconds * (double)targetFormat.sampleRate + roundingOffset;
		totalFrameCount = (AudioFrameIndex)requestedFrameCount;
	} else {
		AudioFrameIndex trackFrameCount = GetPlayedFrameCount(source, options->pitch, targetFormat.sampleRate);
		totalFrameCount = options->isRepeat ? trackFrameCount * repeatPassCount : trackFrameCount;
	}

	size_t frameSize = fplGetAudioFrameSizeInBytes(targetFormat.type, targetFormat.channels);
	AudioFrameIndex chunkFrameCount = (AudioFrameIndex)options->renderChunkFrameCount;
	chunkSamples = (uint8_t *)fplMemoryAllocate(frameSize * chunkFrameCount);
	if(chunkSamples == fpl_null) {
		goto done;
	}

	size_t wavePathLength = fplGetStringLength(options->renderWavePath);
	if(wavePathLength > 0) {
		if(!fplFileCreateBinary(options->renderWavePath, &waveFile)) {
			FPL_LOG_ERROR("Render", "Failed to create the wave file '%s'", options->renderWavePath);
			goto done;
		}
		isWaveFileOpen = true;
		if(!WriteWaveFileHeader(&waveFile, &targetFormat, totalFrameCount)) {
			FPL_LOG_ERROR("Render", "Failed to write the wave file '%s'", options->renderWavePath);
			goto done;
		}
	}

	uint64_t hash = Fnv1a64OffsetBasis;
	AudioFrameIndex renderedFrameCount = 0;
	while(renderedFrameCount < totalFrameCount) {
		AudioFrameIndex remainingFrameCount = totalFrameCount - renderedFrameCount;
		AudioFrameIndex framesToWrite = fplMin(chunkFrameCount, remainingFrameCount);

		fplTimestamp mixStart = fplTimestampQuery();
		AudioFrameIndex writtenFrameCount = AudioSystemWriteFrames(audioSys, chunkSamples, &targetFormat, framesToWrite, true);
		fplTimestamp mixEnd = fplTimestampQuery();
		fplSeconds mixSeconds = fplTimestampElapsed(mixStart, mixEnd);
		RecordMixCost(histogram, mixSeconds * MicrosecondsPerSecond);

		if(writtenFrameCount != framesToWrite) {
			FPL_LOG_ERROR("Render", "The mixer wrote %u instead of %u frames", writtenFrameCount, framesToWrite);
			goto done;
		}

		size_t chunkByteCount = writtenFrameCount * frameSize;
		hash = HashFnv1a64(hash, chunkSamples, chunkByteCount);

		if(isWaveFileOpen) {
			uint32_t writtenByteCount = fplFileWriteBlock32(&waveFile, chunkSamples, (uint32_t)chunkByteCount);
			if(writtenByteCount != chunkByteCount) {
				FPL_LOG_ERROR("Render", "Failed to write the wave file '%s'", options->renderWavePath);
				goto done;
			}
		}

		renderedFrameCount += writtenFrameCount;
	}

	// One space separated name=value line, the scripts read the fields by name
	char fileName[256];
	fplCopyString(options->isSine ? "sine" : trackSource.name, fileName, fplArrayCount(fileName));
	for(char *nameChar = fileName; *nameChar != 0; ++nameChar) {
		if(*nameChar == ' ') {
			*nameChar = '_';
		}
	}
	ProcessMemoryUsage memoryUsage = GetProcessMemoryUsage();
	double mixMeanMicroseconds = GetMixCostMean(histogram);
	double mixPercentileMicroseconds = GetMixCostPercentile(histogram, percentileShare);
	const char *formatName = RenderFormatChoices[options->renderFormat].name;
	fplConsoleFormatOut("render: file=%s rate=%u format=%s channels=%u chunk=%u frames=%u hash=%016llx mix_mean_us=%.3f mix_p99_us=%.3f mix_max_us=%.3f vm_hwm_kb=%llu\n", fileName, targetFormat.sampleRate, formatName, (uint32_t)targetFormat.channels, chunkFrameCount, renderedFrameCount, (unsigned long long)hash, mixMeanMicroseconds, mixPercentileMicroseconds, histogram->maxMicroseconds, (unsigned long long)memoryUsage.highWaterKilobytes);

	result = 0;

done:
	if(isWaveFileOpen) {
		fplFileClose(&waveFile);
	}
	if(chunkSamples != fpl_null) {
		fplMemoryFree(chunkSamples);
	}
	if(isAudioSystemInitialized) {
		AudioSystemShutdown(audioSys);
	}
	if(histogram != fpl_null) {
		fplMemoryFree(histogram);
	}
	if(trackList != fpl_null) {
		fplMemoryFree(trackList);
	}
	if(audioSys != fpl_null) {
		fplMemoryFree(audioSys);
	}
	fplPlatformRelease();
	return(result);
}

int main(int argc, char **args) {
    // Log everything into the standard console. Adding the error console would write every line a second
    // time, and the debug output is a real one on Windows only - anywhere else it is that same console again.
    fplLogSettings logSettings = fplZeroInit;
    logSettings.maxLevel = fplLogLevel_All;
#if defined(FPL_PLATFORM_WINDOWS)
    logSettings.writers[0].flags = fplLogWriterFlags_StandardConsole | fplLogWriterFlags_DebugOut;
#else
    logSettings.writers[0].flags = fplLogWriterFlags_StandardConsole;
#endif
    fplSetLogSettings(&logSettings);

	const int invalidArgumentsExitCode = 1;

	PlayerOptions options = MakeDefaultPlayerOptions();
	if(!ParsePlayerOptions(argc, args, &options)) {
		return(invalidArgumentsExitCode);
	}
	if(options.isHelp) {
		PrintPlayerHelp();
		return(0);
	}
	// --render-wav renders as well
	size_t renderWavePathLength = fplGetStringLength(options.renderWavePath);
	if(renderWavePathLength > 0) {
		options.isRender = true;
	}
	if(!ValidatePlayerOptions(&options)) {
		return(invalidArgumentsExitCode);
	}
	LogPlayerOptions(&options);

	if(options.isRender) {
		return(RenderOffline(&options));
	}

	AudioDemo *demo = (AudioDemo *)fplMemoryAllocate(sizeof(AudioDemo));
	demo->options = options;
	demo->playbackMode = (PlaybackMode)options.mixerMode;
	demo->sineWave = MakeSineWave(options.sineSeconds);
	demo->useRealTimeSamples = false;

	int result = -1;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString(APP_TITLE, settings.window.title, fplArrayCount(settings.window.title));
	fplCopyString(APP_TITLE, settings.console.title, fplArrayCount(settings.console.title));

	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings.video.isVSync = true;

	SetupAudioBackendAndFormat(&settings.audio);

	// Disable auto start/stop of audio playback
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	//
	// Audio device from --device, before anything plays
	//
	if(!SelectAudioDevice(options.deviceName, &settings)) {
		fplMemoryFree(demo);
		return(invalidArgumentsExitCode);
	}

    AudioTrackSource audioTracks[8] = fplZeroInit;
    size_t audioTrackCount = 0;

    // Initialize the platform with audio enabled and the settings
	if(!fplPlatformInit(fplInitFlags_Video | fplInitFlags_Audio, &settings)) {
		goto done;
	}

    // Get hardware format
    fplGetAudioHardwareFormat(&demo->targetAudioFormat);

    // Overwrite the client read callback, so we can write samples to the sound device
    fplSetAudioClientReadCallback(AudioPlayback, demo);

	// Initialize audio system
	if(!AudioSystemInit(&demo->audioSys, &demo->targetAudioFormat)) {
		goto done;
	}

	// Initialze playback latency
	demo->maxPlaybackFrameLatency = demo->targetAudioFormat.bufferSizeInFrames / demo->targetAudioFormat.periods;

	// Load audio tracks
	// 
    // Only allow audio sources that have a sample rates that are even by the output sample rate!
    // Because we don't support non-even sample conversions, such as 48000 <-> 41000.
    if (options.fileCount > 0) {
        size_t maxTrackCount = fplArrayCount(audioTracks);
        for (size_t fileIndex = 0; fileIndex < fplMin(maxTrackCount, options.fileCount); ++fileIndex) {
			const char *filePath = options.files[fileIndex];
			AudioTrackSource *track = &audioTracks[audioTrackCount];
			if (SetAudioTrackSourceFromFile(&demo->audioSys, track, filePath)) {
				++audioTrackCount;
			}
        }
    } else if (!options.isSine && IsAudioSampleRateSupported(&demo->audioSys, sampleRate_musicTavsControlArgofox)) {
        // Load default music (44100 Hz), --sine plays only the sine wave that LoadAudioTrackList adds
        AudioTrackSource *track = &audioTracks[audioTrackCount++];
        track->type = AudioTrackSourceType_Data;
        fplCopyString(name_musicTavsControlArgofox, track->name, fplArrayCount(track->name));
        track->data.size = sizeOf_musicTavsControlArgofox;
        track->data.data = ptr_musicTavsControlArgofox;
    }

	// Initialize OpenGL
	if(!fglLoadOpenGL(true))
		goto done;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH_HINT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	const fplSettings *currentSettings = fplGetCurrentSettings();

	// Only thread mode mixes into an output ring
	AudioFrameIndex streamBufferFrames = 0;
	if(demo->playbackMode == PlaybackMode_Thread) {
		if(!InitializeStreamBuffers(demo, &streamBufferFrames)) {
			goto done;
		}
	}

	if(!InitializeVisualization(demo)) {
		goto done;
	}

	// The device sine plays without the audio system, so it needs no tracks and no streaming thread
	if(demo->playbackMode != PlaybackMode_DeviceSine) {
		if(!LoadAudioTrackList(&demo->audioSys, audioTracks, audioTrackCount, options.isSine, &demo->sineWave, LoadAudioTrackFlags_None, &demo->trackList)) {
			goto done;
		}

		if(demo->trackList.count == 0) {
			goto done;
		}

		// The streaming thread starts the first track itself
		if(!fplMutexInit(&demo->trackLock)) {
			goto done;
		}
		demo->controls.volume = options.volume;
		demo->controls.playMode = (PlayMode)options.playMode;
		demo->controls.isRepeat = options.isRepeat;

#if 0
		// Stream in initial frames
		AudioFrameIndex initialFrameStreamCount = streamBufferFrames / 4;
		fplAssert(streamBufferFrames >= initialFrameStreamCount);
		WriteAudioToRingBuffer(&demo->targetAudioFormat, initialFrameStreamCount, demo, fpl_null);
#endif

		// Start streaming thread
		demo->streamingThread = fplThreadCreate(AudioStreamingThread, demo);
	}

	// Start audio playback (This will start calling clientReadCallback regularly)
	if(fplPlayAudio() == fplAudioResultType_Success) {
		// Print output infos
		const char *outBackendName = fplGetAudioBackendName(currentSettings->audio.backend);
		const char *outFormatName = fplGetAudioFormatName(demo->audioSys.targetFormat.format);
		uint32_t outSampleRate = demo->audioSys.targetFormat.sampleRate;
		uint32_t outChannels = demo->audioSys.targetFormat.channels;
		AudioSystemStats audioStats = fplZeroInit;
		AudioSystemGetStats(&demo->audioSys, &audioStats);
		fplConsoleFormatOut("Playing %u audio sources (%s, %s, %lu Hz, %lu channels)\n",
			audioStats.activePlayItemCount,
			outBackendName,
			outFormatName,
			outSampleRate,
			outChannels);

		// The streaming thread names the track it starts, the device sine plays none
		char audioTrackName[256] = fplZeroInit;
		uint32_t lastTrackChangeCount = 0;
		const float volumeKeyStep = 0.1f;
        UpdateTitle(demo, audioTrackName, 0.0);

		// Loop
		const double targetFrameFps = 1.0 / 60.0;
		double totalTime = 0.0;
		fplTimestamp lastTime = fplTimestampQuery();
        fplMilliseconds lastFpsTime = fplMillisecondsQuery();
        double currentFrameTime = 0.0;
        double currentFps = 0.0;
        uint64_t frameCount = 0;
		fplMilliseconds lastStatsRequestTime = fplMillisecondsQuery();
		while(fplWindowUpdate()) {
			fplEvent ev;
			while(fplPollEvent(&ev)) {
				switch (ev.type) {
					case fplEventType_Keyboard:
					{
						if(ev.keyboard.type == fplKeyboardEventType_Button) {
							if(ev.keyboard.buttonState == fplButtonState_Release) {
								fplKey key = ev.keyboard.mappedKey;
								if(key == fplKey_F) {
									if(!fplIsWindowFullscreen())
										fplEnableWindowFullscreen();
									else
										fplDisableWindowFullscreen();
								} else if(key == fplKey_F1) {
									ToggleRealTimeVisualization(demo);
								} else if(key == fplKey_F2) {
									demo->spectrumMode = (SpectrumMode)((demo->spectrumMode + 1) % SpectrumMode_Count);
								} else if(key == fplKey_N || key == fplKey_MediaNextTrack) {
									RequestTrackStep(demo, 1);
								} else if(key == fplKey_P || key == fplKey_MediaPrevTrack) {
									RequestTrackStep(demo, -1);
								} else if(key == fplKey_Space || key == fplKey_MediaPlayPause) {
									TogglePause(demo);
								} else if(key == fplKey_R) {
									ToggleRepeat(demo);
								} else if(key == fplKey_M) {
									CyclePlayMode(demo);
								} else if(key == fplKey_Up) {
									ChangeVolume(demo, volumeKeyStep);
								} else if(key == fplKey_Down) {
									ChangeVolume(demo, -volumeKeyStep);
								}
                                UpdateTitle(demo, audioTrackName, currentFps);
							}
						}
					} break;

					case fplEventType_Window:
					{
						if (ev.window.type == fplWindowEventType_DroppedFiles) {
							// The device sine plays without the audio system, there is no track to swap
							if (ev.window.dropFiles.fileCount > 0 && demo->playbackMode != PlaybackMode_DeviceSine) {
								// Only a request, the streaming thread replaces the track list
								const char *droppedFilePath = ev.window.dropFiles.files[0];
								if (fplAtomicLoadU32(&demo->hasDroppedFile) == 0) {
									fplCopyString(droppedFilePath, demo->droppedFilePath, fplArrayCount(demo->droppedFilePath));
									fplAtomicStoreU32(&demo->hasDroppedFile, 1);
								} else {
									FPL_LOG_WARN("Track", "The dropped file '%s' is ignored, the file dropped before is not loaded yet", droppedFilePath);
								}
							}
						}
					} break;

					default:
						break;
				}
			}

			// A new track: its name for the title, a fresh visualization
			uint32_t trackChangeCount = fplAtomicLoadU32(&demo->trackChangeCount);
			if(trackChangeCount != lastTrackChangeCount) {
				lastTrackChangeCount = trackChangeCount;
				fplMutexLock(&demo->trackLock);
				fplCopyString(demo->currentTrackName, audioTrackName, fplArrayCount(audioTrackName));
				fplMutexUnlock(&demo->trackLock);
				ClearVisualization(demo);
				UpdateTitle(demo, audioTrackName, currentFps);
			}

            fplWindowSize winSize = fplZeroInit;
            fplGetWindowSize(&winSize);

			// The streaming thread changes the track list only under this lock
			bool hasTrackLock = (demo->playbackMode != PlaybackMode_DeviceSine);
			if(hasTrackLock) {
				fplMutexLock(&demo->trackLock);
			}
            Render(demo, winSize.width, winSize.height, totalTime);
			if(hasTrackLock) {
				fplMutexUnlock(&demo->trackLock);
			}
			fplVideoFlip();

			if(demo->options.isStats) {
				UpdateStats(demo, &lastStatsRequestTime);
			}

			fplTimestamp curTime = fplTimestampQuery();
            double frameTime = fplTimestampElapsed(lastTime, curTime);
            ++frameCount;
			totalTime += frameTime;

#if 0
			// Delay to limit the frame rate to 60 Hz
			if (frameTime < targetFrameFps) {
				double delta = targetFrameFps - frameTime;
				int ms = (int)(delta * 1000.0);
				if (ms > 0) {
					fplThreadSleep(ms);
				}
			}
#endif

            currentFps = (double)frameCount / totalTime;
            if( currentFps > 1000 ){
                currentFps = 0;
            }

            if (fplMillisecondsQuery() - lastFpsTime >= 1000) {
                UpdateTitle(demo, audioTrackName, currentFps);
                lastFpsTime = fplMillisecondsQuery();
            }

			lastTime = fplTimestampQuery();
		}

		// Stop audio playback
		fplStopAudio();
	}

	result = 0;

done:
	// Wait for decoding thread to stop
	if(demo->streamingThread != fpl_null) {
		demo->isStreamingThreadStopped = 1;
		fplThreadWaitForOne(demo->streamingThread, FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(demo->streamingThread);
	}

	// Free and streaming buffers
	if(demo->playbackMode == PlaybackMode_Thread) {
		ReleaseStreamBuffers(demo);
	}

    // Release visualization
	ReleaseVisualization(demo);

    // Release audio system
    StopAllAudioTracks(&demo->audioSys, &demo->trackList);
	for(uint32_t trackIndex = 0; trackIndex < fplArrayCount(demo->trackList.tracks); ++trackIndex) {
		FreeAudioBuffer(&demo->audioSys.memory, &demo->trackList.tracks[trackIndex].outputFullBuffer);
	}
    AudioSystemShutdown(&demo->audioSys);
	fplMutexDestroy(&demo->trackLock);

    // Shutdown OpenGL
	fglUnloadOpenGL();

	// Free demo memory
	if(demo) {
		fplMemoryFree(demo);
	}

	fplPlatformRelease();

	return(result);
}
