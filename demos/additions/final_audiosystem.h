/*
Name:
	Final Audio System

Description:
	Audio system for loading mixing/converting audio streams.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_AUDIOSYSTEM_H
#define FINAL_AUDIOSYSTEM_H

#include <final_platform_layer.h>
#include <float.h>

#define MAX_AUDIOBUFFER_SAMPLE_COUNT 4096
#define MAX_AUDIOBUFFER_BYTES_PER_SAMPLE 4
#define MAX_AUDIOBUFFER_CHANNEL_COUNT 2
typedef struct AudioBuffer {
	uint8_t samples[MAX_AUDIOBUFFER_BYTES_PER_SAMPLE * MAX_AUDIOBUFFER_CHANNEL_COUNT * MAX_AUDIOBUFFER_SAMPLE_COUNT];
	uint32_t maxSampleCount;
	uint32_t framesRemaining;
	uint32_t sampleIndex;
} AudioBuffer;

typedef struct AudioMixingBuffer {
	float samples[MAX_AUDIOBUFFER_CHANNEL_COUNT * MAX_AUDIOBUFFER_SAMPLE_COUNT];
	uint32_t maxSampleCount;
	uint32_t samplesUsed;
} AudioMixingBuffer;

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
	float volume;
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
	AudioMixingBuffer mixingBuffer;
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
extern bool AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume);

#endif // FINAL_AUDIOSYSTEM_H

#if defined(FINAL_AUDIOSYSTEM_IMPLEMENTATION) && !defined(FINAL_AUDIOSYSTEM_IMPLEMENTED)
#define FINAL_AUDIOSYSTEM_IMPLEMENTED

#define FINAL_WAVELOADER_IMPLEMENTATION
#include "final_waveloader.h"

static void *AllocateAudioMemory(AudioSystem *audioSys, size_t size) {
	// @TODO(final): Better memory management for audio system!
	void *result = fplMemoryAllocate(size);
	return(result);
}
static void FreeAudioMemory(void *ptr) {
	// @TODO(final): Better memory management for audio system!
	fplMemoryFree(ptr);
}

extern bool AudioSystemInit(AudioSystem *audioSys) {
	if(audioSys == fpl_null) {
		return false;
	}
	FPL_CLEAR_STRUCT(audioSys);
	if(!fplGetAudioHardwareFormat(&audioSys->nativeFormat)) {
		return false;
	}
	if(!fplMutexInit(&audioSys->sources.lock)) {
		return false;
	}
	if(!fplMutexInit(&audioSys->playItems.lock)) {
		return false;
	}
	audioSys->conversionBuffer.maxSampleCount = MAX_AUDIOBUFFER_SAMPLE_COUNT;
	audioSys->mixingBuffer.maxSampleCount = MAX_AUDIOBUFFER_SAMPLE_COUNT;
	return(true);
}

extern AudioSource *AudioSystemAllocateSource(AudioSystem *audioSys, const uint32_t channels, const uint32_t sampleRate, const fplAudioFormatType type, const uint32_t sampleCount) {
	// Allocate one memory block for source struct, some padding and the sample data
	size_t samplesSize = fplGetAudioBufferSizeInBytes(type, channels, sampleCount);
	size_t memSize = sizeof(AudioSource) + sizeof(size_t) + samplesSize;
	void *mem = AllocateAudioMemory(audioSys, memSize);
	if(mem == fpl_null) {
		return fpl_null;
	}

	AudioSource *result = (AudioSource *)mem;
	result->samplesSize = samplesSize;
	result->samples = (uint8_t *)mem + sizeof(AudioSource) + sizeof(size_t);
	result->sampleCount = sampleCount;
	result->channels = channels;
	result->format = type;
	result->samplesPerSeconds = sampleRate;
	return(result);
}

extern AudioSource *AudioSystemLoadFileSource(AudioSystem *audioSys, const char *filePath) {
	// @NOTE(final): For now we only support wave files
	LoadedWave loadedWave = FPL_ZERO_INIT;
	if(!LoadWaveFromFile(filePath, &loadedWave)) {
		return fpl_null;
	}

	// Allocate one memory block for source struct, some padding and the sample data
	AudioSource *source = AudioSystemAllocateSource(audioSys, loadedWave.channelCount, loadedWave.samplesPerSecond, loadedWave.formatType, loadedWave.sampleCount);
	if(source == fpl_null) {
		return fpl_null;
	}
	fplAssert(source->samplesSize >= loadedWave.samplesSize);
	fplMemoryCopy(loadedWave.samples, loadedWave.samplesSize, source->samples);
	source->id.value = fplAtomicAddU32(&audioSys->sources.idCounter, 1) + 1;

	FreeWave(&loadedWave);

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

	return(source);
}

extern bool AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat, const float volume) {
	if((audioSys == fpl_null) || (source == fpl_null)) {
		return false;
	}

	AudioPlayItem *playItem = (AudioPlayItem *)AllocateAudioMemory(audioSys, sizeof(AudioPlayItem));
	if(playItem == fpl_null) {
		return false;
	}

	playItem->next = playItem->prev = fpl_null;
	playItem->samplesPlayed = 0;
	playItem->source = source;
	playItem->isFinished = false;
	playItem->isRepeat = repeat;
	playItem->volume = volume;

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

	return true;
}

static float ConvertToF32(uint8_t *inSamples, uint32_t inChannel, fplAudioFormatType inFormat) {
	// @TODO(final): Convert from other audio formats to F32
	switch(inFormat) {
		case fplAudioFormatType_S16:
		{
			int16_t sampleValue = *(int16_t *)inSamples + inChannel;
			if(sampleValue < 0) {
				return sampleValue / (float)(INT16_MAX - 1);
			} else {
				return sampleValue / (float)INT16_MAX;
			}
		} break;

		case fplAudioFormatType_S32:
		{
			int32_t sampleValue = *(int32_t *)inSamples + inChannel;
			if(sampleValue < 0) {
				return sampleValue / (float)(INT32_MAX - 1);
			} else {
				return sampleValue / (float)INT32_MAX;
			}
		} break;

		case fplAudioFormatType_F32:
		{
			float sampleValueF32 = *(float *)inSamples + inChannel;
			return(sampleValueF32);
		} break;

		default:
			return 0.0;
	}
}

static void ConvertFromF32(const float inSampleValue, uint8_t *outSamples, const uint32_t outChannel, const fplAudioFormatType outFormat) {
	// @TODO(final): Convert to other audio formats
	switch(outFormat) {
		case fplAudioFormatType_S16:
		{
			int16_t *sampleValue = (int16_t *)outSamples + outChannel;
			*sampleValue = (int16_t)(inSampleValue * INT16_MAX);
		} break;

		case fplAudioFormatType_S32:
		{
			int32_t *sampleValue = (int32_t *)outSamples + outChannel;
			*sampleValue = (int32_t)(inSampleValue * INT32_MAX);
		} break;

		case fplAudioFormatType_F32:
		{
			float *sampleValue = (float *)outSamples + outChannel;
			*sampleValue = inSampleValue;
		} break;

		default:
			break;
	}
}




static void RemovePlayItem(AudioPlayItems *playItems, AudioPlayItem *playItem) {
	if(playItem->prev != fpl_null) {
		playItem->prev->next = playItem->next;
		if(playItem->next != fpl_null) {
			playItem->next->prev = playItem->prev;
		}
	}
	if(playItem->next != fpl_null) {
		playItem->next->prev = playItem->prev;
		if(playItem->prev != fpl_null) {
			playItem->prev->next = playItem->next;
		}
	}
	if(playItem == playItems->last) {
		playItems->first = fpl_null;
	} else if(playItem == playItems->first) {
		playItems->first = playItem->next;
	}
	if(playItems->first == fpl_null) {
		playItems->last = fpl_null;
	}
	FreeAudioMemory(playItem);
	--playItems->count;
}

static uint32_t MixSamples(const float *inSamples, const uint32_t inChannels, float *outSamples, const uint32_t outChannels) {
	uint32_t mixedSampleCount = 0;
	if(inChannels > 0 && outChannels > 0) {
		if(inChannels != outChannels) {
			float sampleValue = inSamples[0];
			for(uint32_t i = 0; i < outChannels; ++i) {
				outSamples[i] += sampleValue;
				++mixedSampleCount;
			}
		} else if(inChannels == outChannels) {
			for(uint32_t i = 0; i < inChannels; ++i) {
				float sampleValue = inSamples[i];
				outSamples[i] += sampleValue;
				++mixedSampleCount;
			}
		}
	}
	return(mixedSampleCount);
}

static uint32_t MixPlayItems(AudioSystem *audioSys, const uint32_t maxSampleCount) {
	const uint32_t outSampleRate = audioSys->nativeFormat.sampleRate;
	const uint32_t outChannelCount = audioSys->nativeFormat.channels;

	audioSys->mixingBuffer.samplesUsed = 0;
	fplMemoryClear(audioSys->mixingBuffer.samples, FPL_ARRAYCOUNT(audioSys->mixingBuffer.samples));

	uint32_t maxOutSampleCount = 0;

	AudioPlayItem *item = audioSys->playItems.first;
	while(item != fpl_null) {
		fplAssert(!item->isFinished);

		float *outSamples = audioSys->mixingBuffer.samples;

		const AudioSource *source = item->source;
		fplAssert(item->samplesPlayed < source->sampleCount);

		uint32_t inSampleRate = source->samplesPerSeconds;
		uint32_t inTotalSampleCount = source->sampleCount;
		uint32_t inChannelCount = source->channels;
		fplAudioFormatType inFormat = source->format;
		uint32_t inBytesPerSample = fplGetAudioSampleSizeInBytes(source->format);

		uint8_t *inSamples = source->samples + item->samplesPlayed * (inChannelCount * inBytesPerSample);
		uint32_t inRemainingSampleCount = inTotalSampleCount - item->samplesPlayed;

		if(inSampleRate == outSampleRate) {
			// Sample rates are equal, just write out the samples
			int inSampleCount = FPL_MIN(maxSampleCount, inRemainingSampleCount);
			for(int i = 0; i < inSampleCount; ++i) {
				float inSampleValues[MAX_AUDIOBUFFER_CHANNEL_COUNT];
				for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
					inSampleValues[inChannelIndex] = ConvertToF32(inSamples, inChannelIndex, inFormat) * item->volume;
				}
				inSamples += inBytesPerSample * inChannelCount;
				++item->samplesPlayed;

				uint32_t writtenSamples = MixSamples(inSampleValues, inChannelCount, outSamples, outChannelCount);
				outSamples += writtenSamples;
			}
		} else if(outSampleRate > 0 && inSampleRate > 0 && inTotalSampleCount > 0) {
			bool isEven = (outSampleRate > inSampleRate) ? ((outSampleRate % inSampleRate) == 0) : ((inSampleRate % outSampleRate) == 0);
			if(isEven) {
				if(outSampleRate > inSampleRate) {
					// Simple Upsampling (2x, 4x, 6x, 8x, etc.)
					int upsamplingFactor = outSampleRate / inSampleRate;
					int inSampleCount = FPL_MIN(maxSampleCount / upsamplingFactor, inRemainingSampleCount);
					for(int i = 0; i < inSampleCount; ++i) {
						float inSampleValues[MAX_AUDIOBUFFER_CHANNEL_COUNT];
						for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							inSampleValues[inChannelIndex] = ConvertToF32(inSamples, inChannelIndex, inFormat) * item->volume;
						}
						inSamples += inBytesPerSample * inChannelCount;
						++item->samplesPlayed;

						for(int f = 0; f < upsamplingFactor; ++f) {
							uint32_t writtenSamples = MixSamples(inSampleValues, inChannelCount, outSamples, outChannelCount);
							outSamples += writtenSamples;
						}
					}
				} else {
					// Simple Downsampling (1/2, 1/4, 1/6, 1/8, etc.)
					fplAssert(inSampleRate > outSampleRate);
					int downsamplingCount = inSampleRate / outSampleRate;
					int inSampleCount = FPL_MIN(maxSampleCount * downsamplingCount, inRemainingSampleCount);
					for(int i = 0; i < inSampleCount; i += downsamplingCount) {
						float inSampleValues[MAX_AUDIOBUFFER_CHANNEL_COUNT];
						uint8_t *inSamplesForIndex = inSamples + (i * inBytesPerSample * inChannelCount);
						for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
							inSampleValues[inChannelIndex] = ConvertToF32(inSamplesForIndex, inChannelIndex, inFormat) * item->volume;
						}
						item->samplesPlayed += downsamplingCount;

						uint32_t writtenSamples = MixSamples(inSampleValues, inChannelCount, outSamples, outChannelCount);
						outSamples += writtenSamples;
					}
				}
			} else {
				// @TODO(final): Convert from odd freqencies such as 22050 Hz to 48000 Hz, etc.
				// This requires real DSP!
			}
		}

		uint32_t outSampleCount = (uint32_t)(outSamples - audioSys->mixingBuffer.samples);
		maxOutSampleCount = FPL_MAX(maxOutSampleCount, outSampleCount);

		item = item->next;
	}

	// Remove or repeat play items
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		AudioPlayItem *next = playItem->next;
		if(!playItem->isFinished) {
			if(playItem->samplesPlayed == playItem->source->sampleCount) {
				playItem->isFinished = true;
			}
		}
		if(playItem->isFinished) {
			if(!playItem->isRepeat) {
				RemovePlayItem(&audioSys->playItems, playItem);
			} else {
				playItem->isFinished = false;
				playItem->samplesPlayed = 0;
			}
		}
		playItem = next;
	}
	fplMutexUnlock(&audioSys->playItems.lock);

	uint32_t result = maxOutSampleCount / outChannelCount;
	return(result);
}

static uint32_t WriteSamples(const float *inSamples, const uint32_t inChannels, uint8_t *outSamples, const uint32_t outChannels, const fplAudioFormatType outFormat) {
	uint32_t writtenSampleCount = 0;
	if(inChannels > 0 && outChannels > 0) {
		if(outChannels != inChannels) {
			float sampleValue = inSamples[0];
			for(uint32_t i = 0; i < outChannels; ++i) {
				ConvertFromF32(sampleValue, outSamples, i, outFormat);
				++writtenSampleCount;
			}
		} else {
			fplAssert(inChannels == outChannels);
			for(uint32_t i = 0; i < inChannels; ++i) {
				float sampleValue = inSamples[i];
				ConvertFromF32(sampleValue, outSamples, i, outFormat);
				++writtenSampleCount;
			}
		}
	}
	return(writtenSampleCount);
}

static bool FillConversionBuffer(AudioSystem *audioSys, const uint32_t maxSampleCount) {
	audioSys->conversionBuffer.framesRemaining = 0;
	audioSys->conversionBuffer.sampleIndex = 0;
	uint8_t *outSamples = audioSys->conversionBuffer.samples;
	uint32_t outBytesPerSample = fplGetAudioSampleSizeInBytes(audioSys->nativeFormat.type);
	uint32_t outChannelCount = audioSys->nativeFormat.channels;
	uint32_t outSampleRate = audioSys->nativeFormat.sampleRate;
	fplAudioFormatType outFormat = audioSys->nativeFormat.type;

	uint32_t mixSampleCount = MixPlayItems(audioSys, maxSampleCount);

	for(uint32_t i = 0; i < mixSampleCount; ++i) {
		float *inMixingSamples = audioSys->mixingBuffer.samples + i * outChannelCount;
		uint32_t writtenSamples = WriteSamples(inMixingSamples, outChannelCount, outSamples, outChannelCount, outFormat);
		outSamples += writtenSamples * outBytesPerSample;
		++audioSys->conversionBuffer.framesRemaining;
	}

#if 0
	AudioPlayItem *item = audioSys->playItems.first;
	if(item == fpl_null) {
		return false;
	}
	fplAssert(!item->isFinished);

	const AudioSource *source = item->source;
	fplAssert(item->samplesPlayed < source->sampleCount);

	uint32_t inSampleRate = source->samplesPerSeconds;
	uint32_t inTotalSampleCount = source->sampleCount;
	uint32_t inChannelCount = source->channels;
	fplAudioFormatType inFormat = source->format;
	uint32_t inBytesPerSample = fplGetAudioSampleSizeInBytes(source->format);

	uint8_t *inSamples = source->samples + item->samplesPlayed * (inChannelCount * inBytesPerSample);
	uint32_t inRemainingSampleCount = inTotalSampleCount - item->samplesPlayed;

	if(inSampleRate == outSampleRate) {
		// Sample rates are equal, just write out the samples
		int inSampleCount = FPL_MIN(outSampleCount, inRemainingSampleCount);
		for(int i = 0; i < inSampleCount; ++i) {
			float inSampleValues[2];
			for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
				inSampleValues[inChannelIndex] = ConvertToF32(inSamples, inChannelIndex, inFormat);
			}
			inSamples += inBytesPerSample * inChannelCount;
			++item->samplesPlayed;

			uint32_t writtenSamples = WriteSamples(inSampleValues, inChannelCount, outSamples, outChannelCount, outFormat);
			outSamples += writtenSamples * outBytesPerSample;
			++audioSys->conversionBuffer.framesRemaining;
		}
	} else if(outSampleRate > 0 && inSampleRate > 0 && inTotalSampleCount > 0) {
		bool isEven = (outSampleRate > inSampleRate) ? ((outSampleRate % inSampleRate) == 0) : ((inSampleRate % outSampleRate) == 0);
		if(isEven) {
			if(outSampleRate > inSampleRate) {
				// @NOTE(final): Simple Upsampling (2x, 4x, 6x, 8x, etc.)
				int upsamplingFactor = outSampleRate / inSampleRate;
				int inSampleCount = FPL_MIN(outSampleCount / upsamplingFactor, inRemainingSampleCount);
				for(int i = 0; i < inSampleCount; ++i) {
					float inSampleValues[2];
					for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
						inSampleValues[inChannelIndex] = ConvertToF32(inSamples, inChannelIndex, inFormat);
					}
					inSamples += inBytesPerSample * inChannelCount;
					++item->samplesPlayed;

					for(int f = 0; f < upsamplingFactor; ++f) {
						uint32_t writtenSamples = WriteSamples(inSampleValues, inChannelCount, outSamples, outChannelCount, outFormat);
						outSamples += writtenSamples * outBytesPerSample;
						++audioSys->conversionBuffer.framesRemaining;
					}
				}
			} else {
				// @NOTE(final): Simple Downsampling (1/2, 1/4, 1/6, 1/8, etc.)
				fplAssert(inSampleRate > outSampleRate);
				int downsamplingCount = inSampleRate / outSampleRate;
				int inSampleCount = FPL_MIN(outSampleCount * downsamplingCount, inRemainingSampleCount);
				for(int i = 0; i < inSampleCount; i += downsamplingCount) {
					float inSampleValues[2];
					uint8_t *inSamplesForIndex = inSamples + (i * inBytesPerSample * inChannelCount);
					for(uint32_t inChannelIndex = 0; inChannelIndex < inChannelCount; ++inChannelIndex) {
						inSampleValues[inChannelIndex] = ConvertToF32(inSamplesForIndex, inChannelIndex, inFormat);
					}
					item->samplesPlayed += downsamplingCount;

					uint32_t writtenSamples = WriteSamples(inSampleValues, inChannelCount, outSamples, outChannelCount, outFormat);
					outSamples += writtenSamples * outBytesPerSample;
					++audioSys->conversionBuffer.framesRemaining;
				}
			}
		} else {
			// @TODO(final): Convert from odd freqencies such as 22050 Hz to 48000 Hz, etc.
			// This requires real DSP!
		}
	}

	// @NOTE(final): Loop wave when it is finished
	if(item->samplesPlayed == inTotalSampleCount) {
		item->isFinished = true;
	}
#endif

	return audioSys->conversionBuffer.framesRemaining > 0;
	}

extern uint32_t AudioSystemWriteSamples(AudioSystem *audioSys, const fplAudioDeviceFormat *outFormat, const uint32_t frameCount, uint8_t *outSamples) {
	fplAssert(audioSys != NULL);
	fplAssert(audioSys->nativeFormat.sampleRate == outFormat->sampleRate);
	fplAssert(audioSys->nativeFormat.type == outFormat->type);
	fplAssert(audioSys->nativeFormat.channels == outFormat->channels);
	fplAssert(audioSys->nativeFormat.channels <= 2);

	uint32_t result = 0;

	uint32_t outputSampleStride = fplGetAudioFrameSizeInBytes(audioSys->nativeFormat.type, audioSys->nativeFormat.channels);
	uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

	AudioBuffer *convBuffer = &audioSys->conversionBuffer;
	size_t maxConversionAudioBufferSize = fplGetAudioBufferSizeInBytes(audioSys->nativeFormat.type, audioSys->nativeFormat.channels, convBuffer->maxSampleCount);

	uint32_t remainingFrames = frameCount;
	while(remainingFrames > 0) {
		// Consume remaining samples in conversion buffer first
		if(convBuffer->framesRemaining > 0) {
			uint32_t maxFramesToRead = convBuffer->framesRemaining;
			uint32_t framesToRead = FPL_MIN(remainingFrames, maxFramesToRead);
			size_t bytesToCopy = framesToRead * outputSampleStride;

			size_t sourcePosition = convBuffer->sampleIndex * outputSampleStride;
			fplAssert(sourcePosition < maxConversionAudioBufferSize);

			size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
			fplAssert(destPosition < maxOutputSampleBufferSize);

			fplMemoryCopy((uint8_t *)convBuffer->samples + sourcePosition, bytesToCopy, (uint8_t *)outSamples + destPosition);

			remainingFrames -= framesToRead;
			audioSys->conversionBuffer.sampleIndex += framesToRead;
			audioSys->conversionBuffer.framesRemaining -= framesToRead;
			result += framesToRead;
		}

		if(remainingFrames == 0) {
			// Done
			break;
		}

		// Conversion buffer is empty, fill it with new data
		if(audioSys->conversionBuffer.framesRemaining == 0) {
			if(!FillConversionBuffer(audioSys, remainingFrames)) {
				// @NOTE(final): No data available, clear remaining samples to zero (Silent)
				uint32_t samplesToClear = remainingFrames;
				size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
				size_t clearSize = remainingFrames * outputSampleStride;
				fplMemoryClear((uint8_t *)outSamples + destPosition, clearSize);
				remainingFrames -= samplesToClear;
				result += samplesToClear;
			}
		}
	}
	return result;
}

static void ClearPlayItems(AudioPlayItems *playItems) {
	fplAssert(playItems != fpl_null);
	AudioPlayItem *item = playItems->first;
	while(item != fpl_null) {
		AudioPlayItem *next = item->next;
		FreeAudioMemory(item);
		item = next;
	}
	playItems->first = playItems->last = fpl_null;
}

static void ReleaseSources(AudioSources *sources) {
	fplAssert(sources != fpl_null);
	AudioSource *source = sources->first;
	while(source != fpl_null) {
		AudioSource *next = source->next;
		// @NOTE(final): Sample memory is included in the memory block
		FreeAudioMemory(source);
		source = next;
	}
	sources->first = sources->last = fpl_null;

}

extern void AudioSystemShutdown(AudioSystem *audioSys) {
	if(audioSys != fpl_null) {
		audioSys->isShutdown = true;

		ClearPlayItems(&audioSys->playItems);
		ReleaseSources(&audioSys->sources);

		fplMutexDestroy(&audioSys->playItems.lock);
		fplMutexDestroy(&audioSys->sources.lock);
	}
}

#endif // FINAL_AUDIOSYSTEM_IMPLEMENTATION