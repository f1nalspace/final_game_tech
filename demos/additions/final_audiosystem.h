/*
Name:
	Final Audio System

Description:
	Audio system for loading mixing/converting audio streams.

	This file is part of the final_framework.

How the mixer works:
	- Clear out the mixer buffers to zero
	- Loop over all playing sounds, for each sound
		- Start at the beginning of the mixing buffer
		- Do sample rate conversion for sound samples -> More samples, less samples, equal samples
		- Converted samples are already in float space, or convert raw samples to float space
		- Mix the samples (+=)
		- Clip and convert mixed samples into target format

Todo:
	- Performance is really bad, so we need to do a lot of things
		- Needs to be called from a realtime thread (fplThreadPriority_RealTime)
		- Remove the need for mutexes (Lock-free!)
		- Dont allocate any memory
		- Dont do any file/network IO
		- Dont call code non-deterministic functions (external api)
		- Do format conversion <-> float for multiple frames, not just one sample
		- Separate format conversion into its own functions and use a dispatch table
		- Separate sample rate conversion from mixing (Doing the sample rate conversion inside the mixing is stupid)
		- Unroll loops (x4), but keep reference implementation
		- SIMD alignment
		- SIMD everything

	- Proper sample rate conversion
		- Linear interpolation
		- SinC

	- Channel mapping -> Requires Channel mapping for FPL as well

	- Do we need to deal with deinterleaved samples?
		Interleaved Samples         = LR|LR|LR|LR|LR|LR|LR

		Deinterleaved Left Samples  = L|L|L|L|L|L|L|L|L|L
		Deinterleaved Right Samples = R|R|R|R|R|R|R|R|R|R

License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_AUDIOSYSTEM_H
#define FINAL_AUDIOSYSTEM_H

#include <final_platform_layer.h>
#include <float.h>

typedef uint32_t AudioFrameIndex;
typedef uint32_t AudioHertz;
typedef uint32_t AudioChannelIndex;
typedef uint32_t AudioSampleIndex;

#define MAX_AUDIO_PROBE_BYTES_COUNT 128
typedef enum AudioFileFormat {
	AudioFileFormat_None = 0,
	AudioFileFormat_Wave,
	AudioFileFormat_Vorbis,
	AudioFileFormat_MP3,
} AudioFileFormat;

typedef struct AudioFormat {
	AudioHertz sampleRate;
	AudioChannelIndex channels;
	fplAudioFormatType format;
} AudioFormat;

typedef struct AudioBuffer {
	uint8_t *samples;
	size_t bufferSize;
	AudioFrameIndex frameCount;
	fpl_b32 isAllocated;
} AudioBuffer;

typedef struct AudioStream {
	AudioBuffer buffer;
	AudioFrameIndex readFrameIndex;
	AudioFrameIndex framesRemaining;
} AudioStream;

#define MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT 2
#define MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT 4096
typedef struct AudioStaticBuffer {
	float samples[MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT * MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT];
	AudioFrameIndex maxFrameCount;
} AudioStaticBuffer;

typedef struct AudioSource {
	AudioBuffer buffer;
	AudioFormat format;
	struct AudioSource *next;
	uint64_t id;
} AudioSource;

typedef struct AudioPlayItem {
	const AudioSource *source;
	struct AudioPlayItem *next;
	struct AudioPlayItem *prev;
	uint64_t id;
	float volume;
	AudioFrameIndex framesPlayed;
	bool isRepeat;
	bool isFinished;
} AudioPlayItem;

typedef struct AudioSources {
	volatile size_t idCounter;
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
	AudioFrameIndex frameIndex;
	AudioHertz toneHz;
	float toneVolume;
} AudioSineWaveData;

typedef struct AudioSystem {
	AudioFormat targetFormat;
	AudioStream conversionBuffer;
	AudioStaticBuffer dspInBuffer;
	AudioStaticBuffer dspOutBuffer;
	AudioStaticBuffer mixingBuffer;
	AudioSineWaveData tempWaveData;
	AudioSources sources;
	AudioPlayItems playItems;
	bool isShutdown;
} AudioSystem;

extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioDeviceFormat *targetFormat);
extern void AudioSystemShutdown(AudioSystem *audioSys);
extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const fplAudioFormatType type, const AudioFrameIndex frameCount);
extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath);
extern AudioSampleIndex AudioSystemWriteSamples(AudioSystem *audioSys, void *outSamples, const fplAudioDeviceFormat *outFormat, const AudioFrameIndex frameCount);
extern uint64_t AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume);
extern void AudioSystemStopSource(AudioSystem *audioSys, const uint64_t playId);
extern void AudioGenerateSineWave(AudioSineWaveData *waveData, void *outSamples, const fplAudioFormatType outFormat, const AudioHertz outSampleRate, const AudioChannelIndex channels, const AudioFrameIndex frameCount);
extern float ConvertToF32(const void *inSamples, const AudioChannelIndex inChannel, const fplAudioFormatType inFormat);
extern void ConvertFromF32(void *outSamples, const float inSampleValue, const AudioChannelIndex outChannel, const fplAudioFormatType outFormat);
#endif // FINAL_AUDIOSYSTEM_H

#if defined(FINAL_AUDIOSYSTEM_IMPLEMENTATION) && !defined(FINAL_AUDIOSYSTEM_IMPLEMENTED)
#define FINAL_AUDIOSYSTEM_IMPLEMENTED

#define FINAL_WAVELOADER_IMPLEMENTATION
#include "final_waveloader.h"

#define FINAL_VORBISLOADER_IMPLEMENTATION
#include "final_vorbisloader.h"

#define FINAL_MP3LOADER_IMPLEMENTATION
#include "final_mp3loader.h"

static const float AudioPI32 = 3.14159265359f;

static void *AllocateAudioMemory(AudioSystem *audioSys, size_t size) {
	// @TODO(final): Better memory management for audio system!
	void *result = fplMemoryAllocate(size);
	return(result);
}
static void FreeAudioMemory(void *ptr) {
	// @TODO(final): Better memory management for audio system!
	fplMemoryFree(ptr);
}

static void InitAudioBuffer(AudioBuffer *audioBuffer, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	fplClearStruct(audioBuffer);
	audioBuffer->frameCount = frameCount;
	audioBuffer->bufferSize = fplGetAudioBufferSizeInBytes(audioFormat->format, audioFormat->channels, frameCount);
	audioBuffer->isAllocated = false;
}
static bool AllocateAudioBuffer(AudioSystem *audioSys, AudioBuffer *audioBuffer, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	InitAudioBuffer(audioBuffer, audioFormat, frameCount);
	audioBuffer->samples = (uint8_t *)AllocateAudioMemory(audioSys, audioBuffer->bufferSize);
	audioBuffer->isAllocated = audioBuffer->samples != fpl_null;
	return(audioBuffer->isAllocated);
}
static void FreeAudioBuffer(AudioSystem *audioSys, AudioBuffer *audioBuffer) {
	if (audioBuffer->isAllocated) {
		if (audioBuffer->samples != fpl_null) {
			FreeAudioMemory(audioBuffer->samples);
		}
		fplClearStruct(audioBuffer);
	}
}

static void AllocateAudioStream(AudioSystem *audioSys, AudioStream *audioStream, const AudioFormat *audioFormat, const AudioFrameIndex frameCount) {
	fplClearStruct(audioStream);
	AllocateAudioBuffer(audioSys, &audioStream->buffer, audioFormat, frameCount);
}
static void FreeAudioStream(AudioSystem *audioSys, AudioStream *audioStream) {
	FreeAudioBuffer(audioSys, &audioStream->buffer);
	fplClearStruct(audioStream);
}

extern bool AudioSystemInit(AudioSystem *audioSys, const fplAudioDeviceFormat *targetFormat) {
	if (audioSys == fpl_null) {
		return false;
	}
	fplClearStruct(audioSys);
	audioSys->targetFormat.channels = targetFormat->channels;
	audioSys->targetFormat.format = targetFormat->type;
	audioSys->targetFormat.sampleRate = targetFormat->sampleRate;
	if (!fplMutexInit(&audioSys->sources.lock)) {
		return false;
	}
	if (!fplMutexInit(&audioSys->playItems.lock)) {
		return false;
	}
	AllocateAudioStream(audioSys, &audioSys->conversionBuffer, &audioSys->targetFormat, MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT);
	audioSys->mixingBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;
	audioSys->dspInBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;
	audioSys->dspOutBuffer.maxFrameCount = MAX_AUDIO_STATIC_BUFFER_FRAME_COUNT;
	audioSys->tempWaveData.toneHz = 256;
	audioSys->tempWaveData.toneVolume = 0.5f;
	return(true);
}

extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const AudioChannelIndex channels, const AudioHertz sampleRate, const fplAudioFormatType type, const AudioFrameIndex frameCount) {
	// Compute audio buffer
	AudioFormat audioFormat = fplZeroInit;
	audioFormat.channels = channels;
	audioFormat.sampleRate = sampleRate;
	audioFormat.format = type;

	AudioBuffer audioBuffer = fplZeroInit;
	InitAudioBuffer(&audioBuffer, &audioFormat, frameCount);

	// Allocate one memory block for source struct, some padding and the sample data
	size_t memSize = sizeof(AudioSource) + sizeof(size_t) + audioBuffer.bufferSize;
	void *mem = AllocateAudioMemory(audioSys, memSize);
	if (mem == fpl_null) {
		return fpl_null;
	}

	AudioSource *result = (AudioSource *)mem;
	result->format = audioFormat;
	result->buffer = audioBuffer;
	result->buffer.samples = (uint8_t *)mem + sizeof(AudioSource) + sizeof(size_t);
	result->buffer.isAllocated = false;

	return(result);
}

static AudioFileFormat PropeAudioFileFormat(const char *filePath) {
	AudioFileFormat result = AudioFileFormat_None;
	if (filePath != fpl_null) {
		fplFileHandle file;
		if (fplOpenBinaryFile(filePath, &file)) {
			size_t length = fplGetFileSizeFromHandle32(&file);
			size_t sizeToRead = fplMin(MAX_AUDIO_PROBE_BYTES_COUNT, length);
			uint8_t *probeBuffer = (uint8_t *)fplMemoryAllocate(sizeToRead);
			if (probeBuffer != fpl_null) {
				if (fplReadFileBlock32(&file, (uint32_t)sizeToRead, probeBuffer, (uint32_t)sizeToRead) == sizeToRead) {
					if (TestWaveHeader(probeBuffer, sizeToRead)) {
						result = AudioFileFormat_Wave;
					} else if (TestVorbisHeader(probeBuffer, sizeToRead)) {
						result = AudioFileFormat_Vorbis;
					} else if (TestMP3Header(probeBuffer, sizeToRead)) {
						result = AudioFileFormat_MP3;
					}
				}
				fplMemoryFree(probeBuffer);
			}
			fplCloseFile(&file);
		}
	}
	return(result);
}

extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath) {
	AudioFileFormat fileFormat = PropeAudioFileFormat(filePath);
	if (fileFormat == AudioFileFormat_None) {
		return fpl_null;
	}

	PCMWaveData loadedData = fplZeroInit;
	switch (fileFormat) {
		case AudioFileFormat_Wave:
		{
			if (!LoadWaveFromFile(filePath, &loadedData)) {
				return fpl_null;
			}
		} break;

		case AudioFileFormat_Vorbis:
		{
			if (!LoadVorbisFromFile(filePath, &loadedData)) {
				return fpl_null;
			}
		} break;

		case AudioFileFormat_MP3:
		{
			if (!LoadMP3FromFile(filePath, &loadedData)) {
				return fpl_null;
			}
		} break;

		default:
			// Unsupported file format
			return fpl_null;
			break;
	}

	// Allocate one memory block for source struct, some padding and the sample data
	AudioSource *source = AudioSystemAllocateSource(audioSys, loadedData.channelCount, loadedData.samplesPerSecond, loadedData.formatType, loadedData.frameCount);
	if (source == fpl_null) {
		return fpl_null;
	}
	fplAssert(source->buffer.bufferSize >= loadedData.samplesSize);
	fplMemoryCopy(loadedData.samples, loadedData.samplesSize, source->buffer.samples);
	source->id = fplAtomicAddAndFetchSize(&audioSys->sources.idCounter);

	FreeWave(&loadedData);

	fplMutexLock(&audioSys->sources.lock);
	source->next = fpl_null;
	if (audioSys->sources.last == fpl_null) {
		audioSys->sources.first = audioSys->sources.last = source;
	} else {
		audioSys->sources.last->next = source;
		audioSys->sources.last = source;
	}
	++audioSys->sources.count;
	fplMutexUnlock(&audioSys->sources.lock);

	return(source);
}

static void RemovePlayItem(AudioPlayItems *playItems, AudioPlayItem *playItem) {
	if (playItems->first == playItems->last) {
		// Remove single item
		playItems->first = playItems->last = fpl_null;
	} else 	if (playItem == playItems->last) {
		// Remove at end
		if (playItems->first == playItems->last) {
			playItems->first = playItems->last = fpl_null;
		} else {
			playItems->last = playItem->prev;
			playItems->last->next = fpl_null;
		}
	} else if (playItem == playItems->first) {
		// Remove at start
		if (playItems->first == playItems->last) {
			playItems->first = playItems->last = fpl_null;
		} else {
			playItems->first = playItem->next;
			playItems->first->prev = fpl_null;
		}
	} else {
		// Remove in the middle
		AudioPlayItem *cur = playItems->first;
		while (cur != playItem) {
			cur = cur->next;
		}
		cur->prev->next = cur->next;
		if (cur->next != fpl_null) {
			cur->next->prev = cur->prev;
		}
	}
	FreeAudioMemory(playItem);
	--playItems->count;
}

extern void AudioSystemStopSource(AudioSystem *audioSys, const uint64_t playId) {
	AudioPlayItem *playItem = audioSys->playItems.first;
	AudioPlayItem *foundPlayItem = fpl_null;
	while (playItem != fpl_null) {
		if (playItem->id == playId) {
			foundPlayItem = playItem;
			break;
		}
		playItem = playItem->next;
	}
	if (foundPlayItem != fpl_null) {
		fplMutexLock(&audioSys->playItems.lock);
		RemovePlayItem(&audioSys->playItems, foundPlayItem);
		fplMutexUnlock(&audioSys->playItems.lock);
	}
}

extern uint64_t AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume) {
	if ((audioSys == fpl_null) || (source == fpl_null)) {
		return(0);
	}

	AudioPlayItem *playItem = (AudioPlayItem *)AllocateAudioMemory(audioSys, sizeof(AudioPlayItem));
	if (playItem == fpl_null) {
		return(0);
	}

	playItem->id = fplAtomicAddAndFetchU64(&audioSys->playItems.idCounter);
	playItem->next = playItem->prev = fpl_null;
	playItem->framesPlayed = 0;
	playItem->source = source;
	playItem->isFinished = false;
	playItem->isRepeat = repeat;
	playItem->volume = volume;

	fplMutexLock(&audioSys->playItems.lock);
	if (audioSys->playItems.last == fpl_null) {
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

// @TODO(final): Use array instead of single samples for conversion to F32
extern float ConvertToF32(const void *inSamples, const AudioChannelIndex inChannel, const fplAudioFormatType inFormat) {
	// @TODO(final): Convert from other audio formats to F32
	switch (inFormat) {
		case fplAudioFormatType_S16:
		{
			int16_t sampleValue = *((const int16_t *)inSamples + inChannel);
			if (sampleValue < 0) {
				return sampleValue / (float)(INT16_MAX - 1);
			} else {
				return sampleValue / (float)INT16_MAX;
			}
		} break;

		case fplAudioFormatType_S32:
		{
			int32_t sampleValue = *((const int32_t *)inSamples + inChannel);
			if (sampleValue < 0) {
				return sampleValue / (float)(INT32_MAX - 1);
			} else {
				return sampleValue / (float)INT32_MAX;
			}
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

inline float ClipF32(const float value) {
	float result = fplMax(-1.0f, fplMin(value, 1.0f));
	return(result);
}

// @TODO(final): Use array instead of single samples for conversion from F32
extern void ConvertFromF32(void *outSamples, const float inSampleValue, const AudioChannelIndex outChannel, const fplAudioFormatType outFormat) {
	// @TODO(final): Convert to other audio formats
	float x = ClipF32(inSampleValue);
	switch (outFormat) {
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

static AudioSampleIndex MixSamples(float *outSamples, const AudioChannelIndex outChannels, const float *inSamples, const AudioChannelIndex inChannels, const AudioFrameIndex frameCount) {
	AudioSampleIndex mixedSampleCount = 0;

	if (inChannels > 0 && outChannels > 0) {
		float *outP = outSamples;
		const float *inP = inSamples;
		if (inChannels != outChannels) {
			for (AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				float sampleValue = inP[0]; // Just use first channel
				for (AudioChannelIndex channelIndex = 0; channelIndex < outChannels; ++channelIndex) {
					outP[channelIndex] += sampleValue;
					++mixedSampleCount;
				}
				outP += outChannels;
				inP += inChannels;
			}
		} else if (inChannels == outChannels) {
			for (AudioFrameIndex frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
				for (AudioChannelIndex channelIndex = 0; channelIndex < outChannels; ++channelIndex) {
					float sampleValue = *inP++;
					outP[channelIndex] += sampleValue;
					++mixedSampleCount;
				}
				outP += outChannels;
			}
		}
	}
	return(mixedSampleCount);
}

extern void AudioGenerateSineWave(AudioSineWaveData *waveData, void *outSamples, const fplAudioFormatType outFormat, const AudioHertz outSampleRate, const AudioChannelIndex channels, const AudioFrameIndex frameCount) {
	uint8_t *samples = (uint8_t *)outSamples;
	int wavePeriod = outSampleRate / waveData->toneHz;
	size_t sampleStride = fplGetAudioSampleSizeInBytes(outFormat) * channels;
	for (AudioFrameIndex i = 0; i < frameCount; ++i) {
		float t = 2.0f * AudioPI32 * (float)waveData->frameIndex++ / (float)wavePeriod;
		float sampleValue = (float)sin(t) * waveData->toneVolume;
		for (AudioChannelIndex channelIndex = 0; channelIndex < channels; ++channelIndex) {
			ConvertFromF32(samples, sampleValue, channelIndex, outFormat);
		}
		samples += sampleStride;
	}
}

static AudioFrameIndex MixPlayItems(AudioSystem *audioSys, const AudioFrameIndex maxFrameCount) {
	const AudioHertz outSampleRate = audioSys->targetFormat.sampleRate;
	const AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;

	fplMemoryClear(audioSys->mixingBuffer.samples, fplArrayCount(audioSys->mixingBuffer.samples));

	AudioFrameIndex result = 0;

#define GENSINEWAVE 0

#if GENSINEWAVE == 1
	AudioGenerateSineWave(&audioSys->tempWaveData, audioSys->mixingBuffer.samples, fplAudioFormatType_F32, outSampleRate, outChannelCount, maxFrameCount);
	result = maxFrameCount;
#else

	fplMutexLock(&audioSys->playItems.lock);
	AudioSampleIndex maxOutSampleCount = 0;
	AudioPlayItem *item = audioSys->playItems.first;
	while (item != fpl_null) {
		fplAssert(!item->isFinished);

		float *dspOutSamples = audioSys->dspOutBuffer.samples;

		float *outSamples = audioSys->mixingBuffer.samples;

		const AudioSource *source = item->source;
		const AudioFormat *format = &item->source->format;
		const AudioBuffer *buffer = &item->source->buffer;
		fplAssert(item->framesPlayed < buffer->frameCount);

		AudioHertz inSampleRate = format->sampleRate;
		AudioFrameIndex inTotalFrameCount = buffer->frameCount;
		AudioChannelIndex inChannelCount = format->channels;
		fplAudioFormatType inFormat = format->format;
		size_t inBytesPerSample = fplGetAudioSampleSizeInBytes(inFormat);

		uint8_t *inSamples = source->buffer.samples + item->framesPlayed * (inChannelCount * inBytesPerSample);
		AudioFrameIndex inRemainingFrameCount = inTotalFrameCount - item->framesPlayed;

		if (inSampleRate == outSampleRate) {
			// Sample rates are equal, just write out the samples
			const AudioFrameIndex inFrameCount = fplMin(maxFrameCount, inRemainingFrameCount);
			for (AudioFrameIndex i = 0; i < inFrameCount; ++i) {
				for (AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
					*dspOutSamples++ = ConvertToF32(inSamples, inChannelIndex, inFormat) * item->volume;
				}
				inSamples += inBytesPerSample * inChannelCount;
				++item->framesPlayed;
			}
		} else if (outSampleRate > 0 && inSampleRate > 0 && inTotalFrameCount > 0) {
			bool isEven = (outSampleRate > inSampleRate) ? ((outSampleRate % inSampleRate) == 0) : ((inSampleRate % outSampleRate) == 0);
			if (isEven) {
				if (outSampleRate > inSampleRate) {
					// Simple Upsampling (2x, 4x, 6x, 8x etc.)
					const int upsamplingFactor = outSampleRate / inSampleRate;
					const AudioFrameIndex inFrameCount = fplMin(maxFrameCount / upsamplingFactor, inRemainingFrameCount);
					for (AudioFrameIndex i = 0; i < inFrameCount; ++i) {
						float tempSamples[MAX_AUDIO_STATIC_BUFFER_CHANNEL_COUNT];
						for (AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							tempSamples[inChannelIndex] = ConvertToF32(inSamples, inChannelIndex, inFormat) * item->volume;
						}
						for (int f = 0; f < upsamplingFactor; ++f) {
							for (AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
								*dspOutSamples++ = tempSamples[inChannelIndex];
							}
						}
						inSamples += inBytesPerSample * inChannelCount;
						++item->framesPlayed;
					}
				} else {
					// Simple Downsampling (1/2, 1/4, 1/6, 1/8, etc.)
					fplAssert(inSampleRate > outSampleRate);
					const int downsamplingCount = inSampleRate / outSampleRate;
					const AudioFrameIndex inFrameCount = fplMin(maxFrameCount * downsamplingCount, inRemainingFrameCount);
					for (AudioFrameIndex i = 0; i < inFrameCount; i += downsamplingCount) {
						uint8_t *inSamplesForIndex = inSamples + (i * inBytesPerSample * inChannelCount);
						for (AudioChannelIndex inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							*dspOutSamples++ = ConvertToF32(inSamplesForIndex, inChannelIndex, inFormat) * item->volume;
						}
						item->framesPlayed += downsamplingCount;
					}
				}
			} else {
				if (inSampleRate > outSampleRate) {
					// @TODO(final): Linear-Downsampling (Example: 48000 to 41000)
				} else if (inSampleRate < outSampleRate) {
					// @TODO(final): Linear-Upsampling (Example: 41000 to 48000)
				}
			}
		}

		AudioSampleIndex dspOutFrameCount = (AudioSampleIndex)(dspOutSamples - audioSys->dspOutBuffer.samples) / inChannelCount;

		AudioSampleIndex writtenSampleCount = MixSamples(outSamples, outChannelCount, audioSys->dspOutBuffer.samples, inChannelCount, dspOutFrameCount);
		outSamples += writtenSampleCount;

		AudioSampleIndex outSampleCount = (AudioSampleIndex)(outSamples - audioSys->mixingBuffer.samples);
		maxOutSampleCount = fplMax(maxOutSampleCount, outSampleCount);

		// Remove item when it is finished, or restart it for the next run.
		AudioPlayItem *next = item->next;
		if (!item->isFinished) {
			if (item->framesPlayed >= item->source->buffer.frameCount) {
				item->isFinished = true;
			}
		}
		if (item->isFinished) {
			if (!item->isRepeat) {
				RemovePlayItem(&audioSys->playItems, item);
			} else {
				item->isFinished = false;
				item->framesPlayed = 0;
			}
		}
		item = next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);

	result = maxOutSampleCount / outChannelCount;
#endif

	return(result);
}

static AudioSampleIndex ConvertSamplesFromF32(const float *inSamples, const AudioChannelIndex inChannels, uint8_t *outSamples, const AudioChannelIndex outChannels, const fplAudioFormatType outFormat) {
	AudioSampleIndex result = 0;
	if (inChannels > 0 && outChannels > 0) {
		if (outChannels != inChannels) {
			float sampleValue = inSamples[0];
			for (AudioChannelIndex i = 0; i < outChannels; ++i) {
				ConvertFromF32(outSamples, sampleValue, i, outFormat);
				++result;
			}
		} else {
			fplAssert(inChannels == outChannels);
			for (AudioChannelIndex i = 0; i < inChannels; ++i) {
				float sampleValue = inSamples[i];
				ConvertFromF32(outSamples, sampleValue, i, outFormat);
				++result;
			}
		}
	}
	return(result);
}

static bool FillConversionBuffer(AudioSystem *audioSys, const AudioFrameIndex maxFrameCount) {
	audioSys->conversionBuffer.framesRemaining = 0;
	audioSys->conversionBuffer.readFrameIndex = 0;
	uint8_t *outSamples = audioSys->conversionBuffer.buffer.samples;
	size_t outBytesPerSample = fplGetAudioSampleSizeInBytes(audioSys->targetFormat.format);
	AudioChannelIndex outChannelCount = audioSys->targetFormat.channels;
	AudioHertz outSampleRate = audioSys->targetFormat.sampleRate;
	fplAudioFormatType outFormat = audioSys->targetFormat.format;

	AudioFrameIndex mixFrameCount = MixPlayItems(audioSys, maxFrameCount);

	for (AudioFrameIndex i = 0; i < mixFrameCount; ++i) {
		float *inMixingSamples = audioSys->mixingBuffer.samples + i * outChannelCount;
		AudioSampleIndex writtenSamples = ConvertSamplesFromF32(inMixingSamples, outChannelCount, outSamples, outChannelCount, outFormat);
		outSamples += writtenSamples * outBytesPerSample;
		audioSys->conversionBuffer.framesRemaining += writtenSamples / outChannelCount;
	}

	return audioSys->conversionBuffer.framesRemaining > 0;
}

extern AudioSampleIndex AudioSystemWriteSamples(AudioSystem *audioSys, void *outSamples, const fplAudioDeviceFormat *outFormat, const AudioFrameIndex frameCount) {
	fplAssert(audioSys != NULL);
	fplAssert(audioSys->targetFormat.sampleRate == outFormat->sampleRate);
	fplAssert(audioSys->targetFormat.format == outFormat->type);
	fplAssert(audioSys->targetFormat.channels == outFormat->channels);
	fplAssert(audioSys->targetFormat.channels <= 2);

	AudioSampleIndex result = 0;

	size_t outputSampleStride = fplGetAudioFrameSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels);
	size_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

	AudioStream *convBuffer = &audioSys->conversionBuffer;
	size_t maxConversionAudioBufferSize = fplGetAudioBufferSizeInBytes(audioSys->targetFormat.format, audioSys->targetFormat.channels, convBuffer->buffer.frameCount);

	AudioFrameIndex remainingFrames = frameCount;
	while (remainingFrames > 0) {
		// Consume remaining samples in conversion buffer first
		if (convBuffer->framesRemaining > 0) {
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

		if (remainingFrames == 0) {
			// Done
			break;
		}

		// Conversion buffer is empty, fill it with new data
		if (audioSys->conversionBuffer.framesRemaining == 0) {
			if (!FillConversionBuffer(audioSys, remainingFrames)) {
				// @NOTE(final): No data available, clear remaining samples to zero (Silent)
				AudioFrameIndex framesToClear = remainingFrames;
				size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
				size_t clearSize = remainingFrames * outputSampleStride;
				fplMemoryClear((uint8_t *)outSamples + destPosition, clearSize);
				remainingFrames -= framesToClear;
				result += framesToClear;
			}
		}
	}
	return result;
}

static void ClearPlayItems(AudioPlayItems *playItems) {
	fplAssert(playItems != fpl_null);
	AudioPlayItem *item = playItems->first;
	while (item != fpl_null) {
		AudioPlayItem *next = item->next;
		FreeAudioMemory(item);
		item = next;
	}
	playItems->first = playItems->last = fpl_null;
}

static void ReleaseSources(AudioSources *sources) {
	fplAssert(sources != fpl_null);
	AudioSource *source = sources->first;
	while (source != fpl_null) {
		AudioSource *next = source->next;
		// @NOTE(final): Sample memory is included in the memory block
		FreeAudioMemory(source);
		source = next;
	}
	sources->first = sources->last = fpl_null;

}

extern void AudioSystemShutdown(AudioSystem *audioSys) {
	if (audioSys != fpl_null) {
		audioSys->isShutdown = true;

		ClearPlayItems(&audioSys->playItems);
		ReleaseSources(&audioSys->sources);

		FreeAudioStream(audioSys, &audioSys->conversionBuffer);

		fplMutexDestroy(&audioSys->playItems.lock);
		fplMutexDestroy(&audioSys->sources.lock);
	}
}

#endif // FINAL_AUDIOSYSTEM_IMPLEMENTATION