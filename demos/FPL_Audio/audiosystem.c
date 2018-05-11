#include "audiosystem.h"

#include "waveloader.h"

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
	FPL_ASSERT(source->samplesSize >= loadedWave.samplesSize);
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

extern bool AudioSystemPlaySource(AudioSystem *audioSys, const AudioSource *source, const bool repeat) {
	if((audioSys == fpl_null) || (source == fpl_null)) {
		return false;
	}

	AudioPlayItem *playItem = AllocateAudioMemory(audioSys, sizeof(AudioPlayItem));
	if(playItem == fpl_null) {
		return false;
	}

	playItem->next = playItem->prev = fpl_null;
	playItem->samplesPlayed = 0;
	playItem->source = source;
	playItem->isFinished = false;
	playItem->isRepeat = repeat;

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

static uint32_t WriteSamples(const float *inSamples, const uint32_t inChannels, uint8_t *outSamples, const uint32_t outChannels, const fplAudioFormatType outFormat) {
	uint32_t writtenSampleCount = 0;
	if(inChannels > 0) {
		if(outChannels != inChannels) {
			float sampleValue = inSamples[0];
			for(uint32_t i = 0; i < outChannels; ++i) {
				ConvertFromF32(sampleValue, outSamples, i, outFormat);
				++writtenSampleCount;
			}
		} else {
			FPL_ASSERT(inChannels == outChannels);
			for(uint32_t i = 0; i < inChannels; ++i) {
				float sampleValue = inSamples[i];
				ConvertFromF32(sampleValue, outSamples, i, outFormat);
				++writtenSampleCount;
			}
		}
	}
	return(writtenSampleCount);
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

static bool FillConversionBuffer(AudioSystem *audioSys, const uint32_t outSampleCount) {
	audioSys->conversionBuffer.framesRemaining = 0;
	audioSys->conversionBuffer.sampleIndex = 0;
	uint8_t *outSamples = audioSys->conversionBuffer.samples;
	uint32_t outBytesPerSample = fplGetAudioSampleSizeInBytes(audioSys->nativeFormat.type);
	uint32_t outChannelCount = audioSys->nativeFormat.channels;
	uint32_t outSampleRate = audioSys->nativeFormat.sampleRate;
	fplAudioFormatType outFormat = audioSys->nativeFormat.type;

	// @TODO(final): Support for mixing play-items!

	AudioPlayItem *item = audioSys->playItems.first;
	if(item == fpl_null) {
		return false;
	}
	FPL_ASSERT(!item->isFinished);

	const AudioSource *source = item->source;
	FPL_ASSERT(item->samplesPlayed < source->sampleCount);

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
				FPL_ASSERT(inSampleRate > outSampleRate);
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

	// Remove or repeat play items
	fplMutexLock(&audioSys->playItems.lock);
	AudioPlayItem *playItem = audioSys->playItems.first;
	while(playItem != fpl_null) {
		AudioPlayItem *next = playItem->next;
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

	return audioSys->conversionBuffer.framesRemaining > 0;
}

extern uint32_t AudioSystemWriteSamples(AudioSystem *audioSys, const fplAudioDeviceFormat *outFormat, const uint32_t frameCount, uint8_t *outSamples) {
	FPL_ASSERT(audioSys != NULL);
	FPL_ASSERT(audioSys->nativeFormat.sampleRate == outFormat->sampleRate);
	FPL_ASSERT(audioSys->nativeFormat.type == outFormat->type);
	FPL_ASSERT(audioSys->nativeFormat.channels == outFormat->channels);
	FPL_ASSERT(audioSys->nativeFormat.channels <= 2);

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
			FPL_ASSERT(sourcePosition < maxConversionAudioBufferSize);

			size_t destPosition = (frameCount - remainingFrames) * outputSampleStride;
			FPL_ASSERT(destPosition < maxOutputSampleBufferSize);

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
	FPL_ASSERT(playItems != fpl_null);
	AudioPlayItem *item = playItems->first;
	while(item != fpl_null) {
		AudioPlayItem *next = item->next;
		FreeAudioMemory(item);
		item = next;
	}
	playItems->first = playItems->last = fpl_null;
}

static void ReleaseSources(AudioSources *sources) {
	FPL_ASSERT(sources != fpl_null);
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