#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H

#include <final_platform_layer.h>

#define MAX_AUDIOBUFFER_SAMPLE_COUNT 4096
#define MAX_AUDIOBUFFER_BYTES_PER_SAMPLE 4
#define MAX_AUDIOBUFFER_CHANNEL_COUNT 2
typedef struct AudioBuffer {
	uint8_t samples[MAX_AUDIOBUFFER_BYTES_PER_SAMPLE * MAX_AUDIOBUFFER_CHANNEL_COUNT * MAX_AUDIOBUFFER_SAMPLE_COUNT];
	uint32_t maxSampleCount;
	uint32_t framesRemaining;
	uint32_t sampleIndex;
} AudioBuffer;

typedef struct AudioSourceID {
	uint32_t value;
} AudioSourceID;

typedef struct AudioSource {
	uint8_t *samples;
	size_t samplesSize;
	AudioSourceID id;
	uint32_t sampleCount;
	uint32_t samplesPerSeconds;
	uint32_t channels;
	fplAudioFormatType format;
	struct AudioSource *next;
} AudioSource;

typedef struct AudioPlayItem {
	const AudioSource *source;
	struct AudioPlayItem *next;
	struct AudioPlayItem *prev;
	uint32_t samplesPlayed;
	bool isRepeat;
	bool isFinished;
} AudioPlayItem;

typedef struct AudioSources {
	volatile uint32_t idCounter;
	fplMutexHandle lock;
	AudioSource *first;
	AudioSource *last;
	uint32_t count;
} AudioSources;

typedef struct AudioPlayItems {
	fplMutexHandle lock;
	AudioPlayItem *first;
	AudioPlayItem *last;
	uint32_t count;
} AudioPlayItems;

typedef struct AudioSystem {
	AudioBuffer conversionBuffer;
	fplAudioDeviceFormat nativeFormat;
	AudioSources sources;
	AudioPlayItems playItems;
	bool isShutdown;
} AudioSystem;

extern bool AudioSystemInit(AudioSystem *audioSys);
extern void AudioSystemShutdown(AudioSystem *audioSys);
extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const uint32_t channels, const uint32_t sampleRate, const fplAudioFormatType type, const uint32_t sampleCount);
extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath);
extern uint32_t AudioSystemWriteSamples(AudioSystem *audioSys, const fplAudioDeviceFormat *outFormat, const uint32_t frameCount, uint8_t *outSamples);
extern bool AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat);

#endif // AUDIOSYSTEM_H
