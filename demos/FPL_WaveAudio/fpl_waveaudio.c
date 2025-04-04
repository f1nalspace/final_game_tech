/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Wave Audio

Description:
	This sample shows how to play a wave audio file with a fixed sample rate of 44100 Hz.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2025-04-01
	- Initial version

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Include FPL
#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO
#define FPL_NO_WINDOW
#include <final_platform_layer.h>

// We need a little bit of math
#define _USE_MATH_DEFINES
#include <math.h> // sinf, M_PI

// Example wave data
#include <final_music.h>

// Represents the loaded samples with meta informations
typedef struct {
	// Total size of the audio samples
	size_t dataSize;
	// Interleaved audio samples
	uint8_t *data;
	// Sample rate in Hz
	uint32_t sampleRate;
	// Number of audio frames
	uint32_t frameCount;
	// Sample format (S16, F32, etc.)
	fplAudioFormatType format;
	// Number of channels per frame
	uint16_t channels;
	// Padding to align to 32 bytes
	uint16_t padding;
} LoadedWaveData;

#define WAVE_RIFF_ID(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#pragma pack(push, 1)
typedef struct {
	uint32_t chunkId;
	uint32_t chunkSize;
	uint32_t formatId;
} WaveHeader;

typedef struct {
	uint32_t id;
	uint32_t size;
} WaveChunk;

typedef enum {
	WaveChunkId_RIFF = WAVE_RIFF_ID('R', 'I', 'F', 'F'),
	WaveChunkId_WAVE = WAVE_RIFF_ID('W', 'A', 'V', 'E'),
	WaveChunkId_Format = WAVE_RIFF_ID('f', 'm', 't', ' '),
	WaveChunkId_Data = WAVE_RIFF_ID('d', 'a', 't', 'a'),
} WaveChunkID;

typedef enum WaveFormatTags {
	WaveFormatTags_None = 0,
	WaveFormatTags_PCM = 1,
	WaveFormatTags_IEEEFloat = 3,
} WaveFormatTags;

typedef struct WaveFormatEx {
	uint16_t formatTag;
	uint16_t numberOfChannels;
	uint32_t samplesPerSecond;
	uint32_t avgBytesPerSample;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint16_t cbSize;
} WaveFormatEx;
#pragma pack(pop)

typedef enum {
	WaveLoadResult_Success = 0,
	WaveLoadResult_OutOfMemory,
	WaveLoadResult_InvalidArguments,
	WaveLoadResult_FileNotFound,
	WaveLoadResult_IOError,
	WaveLoadResult_BufferTooSmall,
	WaveLoadResult_NotSupported,
	WaveLoadResult_NoFormatFound,
	WaveLoadResult_NoDataFound,
	WaveLoadResult_UnsupportedFormatTag,
} WaveLoadResult;

static WaveLoadResult LoadWaveFromBuffer(const uint8_t *buffer, const size_t bufferSize, LoadedWaveData *outputData) {
	if(buffer == fpl_null || bufferSize == 0 || outputData == fpl_null) {
		return WaveLoadResult_InvalidArguments;
	}

	// Read wave header
	if(bufferSize < sizeof(WaveHeader)) {
		return WaveLoadResult_BufferTooSmall;
	}
	const WaveHeader *header = (const WaveHeader *)buffer;
	if(header->chunkId != WaveChunkId_RIFF || header->formatId != WaveChunkId_WAVE) {
		return WaveLoadResult_NotSupported;
	}

	WaveLoadResult result = WaveLoadResult_NotSupported;

	fplClearStruct(outputData);

	WaveFormatEx waveFormat = fplZeroInit;

	// Search for format and data chunk
	size_t bufferPosition = sizeof(*header);
	const WaveChunk *chunk = (const WaveChunk *)(buffer + bufferPosition);
	while(chunk != fpl_null) {
		bufferPosition += sizeof(WaveChunk);
		size_t nextChunkOffset = chunk->size;
		switch(chunk->id) {
			case WaveChunkId_Format:
			{
				// Found format: Copy WaveFormatEx structure
				const WaveFormatEx *format = (const WaveFormatEx *)(buffer + bufferPosition);
				if(format->formatTag == WaveFormatTags_PCM) {
					waveFormat = *format;
				} else if(format->formatTag == WaveFormatTags_IEEEFloat) {
					waveFormat = *format;
				} else {
					return WaveLoadResult_UnsupportedFormatTag;
				}
			} break;

			case WaveChunkId_Data:
			{
				// Found data: Fill output data
				uint32_t dataSize = chunk->size;
				const uint8_t *data = buffer + bufferPosition;
				switch(waveFormat.formatTag) {
					case WaveFormatTags_PCM:
					case WaveFormatTags_IEEEFloat:
					{
						fplAssert((waveFormat.bitsPerSample > 0) && (waveFormat.bitsPerSample % 8 == 0));
						uint16_t channelCount = waveFormat.numberOfChannels;
						uint32_t bytesPerSample = waveFormat.bitsPerSample / 8;
						uint32_t frameCount = dataSize / (channelCount * bytesPerSample);
						outputData->channels = channelCount;
						outputData->sampleRate = waveFormat.samplesPerSecond;
						outputData->frameCount = frameCount;
						outputData->format = fplAudioFormatType_None;
						if(bytesPerSample == 1) {
							outputData->format = fplAudioFormatType_U8;
						} else if (bytesPerSample == 2) {
							outputData->format = fplAudioFormatType_S16;
						} else if (bytesPerSample == 3) {
							outputData->format = fplAudioFormatType_S24;
						} else if (bytesPerSample == 4) {
							if (waveFormat.formatTag == WaveFormatTags_PCM)
								outputData->format = fplAudioFormatType_S32;
							else
								outputData->format = fplAudioFormatType_F32;
						}
						outputData->dataSize = bytesPerSample * channelCount * frameCount;
						outputData->data = (uint8_t *)fplMemoryAllocate(outputData->dataSize);
						if (outputData->data == fpl_null) {
							return WaveLoadResult_OutOfMemory;
						}
						fplMemoryCopy(data, outputData->dataSize, outputData->data);
						return WaveLoadResult_Success;
					} break;

					default:
						return WaveLoadResult_NoFormatFound;
				}
			} break;
		}

		if((bufferPosition + nextChunkOffset + sizeof(WaveChunk)) <= bufferSize) {
			bufferPosition += nextChunkOffset;
			chunk = (const WaveChunk *)(buffer + bufferPosition);
		} else {
			chunk = fpl_null;
			break;
		}
	}

	return WaveLoadResult_NoDataFound;
}

static WaveLoadResult LoadWaveFromFile(const char* filePath, LoadedWaveData *loadedData) {
	if(filePath == fpl_null || fplGetStringLength(filePath) == 0) {
		return WaveLoadResult_InvalidArguments;
	}
	if (!fplFileExists(filePath)) {
		return WaveLoadResult_FileNotFound;
	}

	fplFileHandle file;
	if (!fplFileOpenBinary(filePath, &file)) {
		return WaveLoadResult_FileNotFound;
	}

	size_t fileSize = fplFileGetSizeFromHandle(&file);

	WaveLoadResult result = WaveLoadResult_NotSupported;

	uint8_t *buffer = fpl_null;

	size_t read = 0;

	if (fileSize == 0) {
		result = WaveLoadResult_BufferTooSmall;
		goto cleanup;
	}

	buffer = fplMemoryAllocate(fileSize);
	if (buffer == fpl_null) {
		result = WaveLoadResult_OutOfMemory;
		goto cleanup;
	}

	read = fplFileReadBlock(&file, fileSize, buffer, fileSize);
	if (read != fileSize) {
		result = WaveLoadResult_IOError;
		goto cleanup;
	}

	result = LoadWaveFromBuffer(buffer, fileSize, loadedData);

cleanup:
	if (buffer != fpl_null) {
		fplMemoryFree(buffer);
	}
	fplFileClose(&file);
}

typedef struct {
	LoadedWaveData waveData;
	uint32_t playedFrames;
} AudioPlaybackState;

static uint32_t AudioPlaybackThread(const fplAudioFormat *nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioPlaybackState *playState = (AudioPlaybackState *)userData;

	// Only S16 is supported for now, a real world application would do a format conversion
	if (nativeFormat->type != fplAudioFormatType_S16) {
		return 0;
	}

	// Wave format must match native format, a real world application would do resampling, format conversion and channel mixing
	if (nativeFormat->type != playState->waveData.format || 
		nativeFormat->sampleRate != playState->waveData.sampleRate || 
		playState->waveData.frameCount == 0)
	{
		return 0;
	}

	// Get size of each sample in bytes
	size_t sampleSize = fplGetAudioSampleSizeInBytes(nativeFormat->type);

	// Number of audio channels
	uint16_t inChannels = playState->waveData.channels;
	uint16_t outChannels = nativeFormat->channels;

	// Loop through all output frames and simply copy the looped wave samples to the output
	int16_t *outS16 = (int16_t *)outputSamples;
	for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		uint32_t sourceFrameIndex = playState->playedFrames;
		int16_t *sourceSamples = (int16_t *)(playState->waveData.data + sourceFrameIndex * inChannels * sampleSize);
		for (uint16_t outChannelIndex = 0; outChannelIndex < outChannels; ++outChannelIndex) {
			int16_t sample = outChannels == inChannels ? sourceSamples[outChannelIndex] : sourceSamples[0];
			outS16[frameIndex * outChannels + outChannelIndex] = sample;
		}
		playState->playedFrames = (playState->playedFrames + 1) % playState->waveData.frameCount;
	}

	// We always return the output frame count, because we are looping the wave samples (Current + 1) % Total
	return frameCount;
}

int main(int argc, char **argv) {
	// We may got a wave file from the arguments
	const char *waveFilePath = fpl_null;
	if (argc >= 2) {
		waveFilePath = argv[1];
	}


	// Setup FPL and force the audio format to S16, 44100 Hz, Stereo
	// Note that, there is no guarantee that every sound device supports this!
	// Modern sound devices with 6+ channels may require 48 KHz or even 96 KHz
	// Real audio applications must do sample rate conversion from any sample rate configurations, such as 44100 to 48000 or 22050 to 44100, etc.
	fplSettings settings = fplZeroInit;
	fplCopyString("FPL Demo | WaveAudio", settings.console.title, fplArrayCount(settings.console.title));

	fplSetDefaultSettings(&settings);
	
	// Try to force stereo channel layout
	settings.audio.targetFormat.channelLayout = fplAudioChannelLayout_Stereo;

	// Try to force S16 as format
	settings.audio.targetFormat.type = fplAudioFormatType_S16;

	// Try to force to 44100 Hz, which is the most common used sample rate
	settings.audio.targetFormat.sampleRate = 44100;

	// Do not start and stop the playback automatically
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;

	// Initialize FPL with console and audio only
	// We don't need any video or window configuration
	if (!fplPlatformInit(fplInitFlags_Console | fplInitFlags_Audio, &settings)) {
		return -1;
	}

	int result = 0;

	WaveLoadResult res = WaveLoadResult_NotSupported;

	AudioPlaybackState playState = fplZeroInit;

	fplAudioFormat hardwareFormat = fplZeroInit;

	const char *audioFormatName = fpl_null;

	// Either load wave file from argument or use a example wave stream stored as binary
	LoadedWaveData loadedWave = fplZeroInit;
	if (fplGetStringLength(waveFilePath) > 0 && fplFileExists(waveFilePath)) {
		fplConsoleFormatOut("Loading wave file '%s'\n", waveFilePath);
		res = LoadWaveFromFile(waveFilePath, &loadedWave);
		if (res != WaveLoadResult_Success) {
			fplConsoleFormatError("Failed to load wave file '%s', result code: %d\n", waveFilePath, res);
			result = -1;
			goto done;
		}
	} else {
		waveFilePath = name_waveFileExample;
		fplConsoleFormatOut("Loading wave stream '%s' with size %zu\n", name_waveFileExample, sizeOf_waveFileExample);
		res = LoadWaveFromBuffer(ptr_waveFileExample, sizeOf_waveFileExample, &loadedWave);
		if (res != WaveLoadResult_Success) {
			fplConsoleFormatError("Failed to load wave example stream '%s', result code: %d\n", name_waveFileExample, res);
			result = -1;
			goto done;
		}
	}

	// Setup audio callback and user data
	playState.waveData = loadedWave;
	playState.playedFrames = 0;
	fplSetAudioClientReadCallback(AudioPlaybackThread, &playState);

	// Start audio playback
	fplPlayAudio();

	// Get actual audio hardware format, which may be different that our "target" format.
	fplGetAudioHardwareFormat(&hardwareFormat);

	// Get name of the audio format (S16, F32, etc.)
	audioFormatName = fplGetAudioFormatName(hardwareFormat.type);

	// Print out some infos and wait for a key to be pressed
	// While we are waiting for a key press, new audio samples will be generated continuesly
	fplConsoleFormatOut("Playing wave file '%s' with %u frames, %u Hz, %u channels, %s\n", waveFilePath, loadedWave.frameCount, hardwareFormat.sampleRate, hardwareFormat.channels, audioFormatName);
	fplConsoleOut("Press any key to exit\n");
	fplConsoleWaitForCharInput();

	// Stop any audio playback, so we can free the wave samples
	fplStopAudio();

	// Free wave samples
	if (playState.waveData.data != fpl_null) {
		fplMemoryFree(playState.waveData.data);
	}

	result = 0;

done:
	// Stop audio playback, shutdown the audio device and release any platform resources
	fplPlatformRelease();

	// We are done
	return result;
}