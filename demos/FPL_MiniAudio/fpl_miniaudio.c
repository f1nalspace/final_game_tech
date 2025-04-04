/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Mini Audio

Description:
	This demo shows how to use the miniaudio library with FPL.

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- miniaudio 0.9.5+

Author:
	Torsten Spaete

Changelog:
	## 2025-03-28
	- Fixed warnings for int vs size_t

	## 2025-02-24
	- Upgraded to latest miniaudio v0.11.22

	## 2025-02-13
	- Fixed compile errors due to changes in the audio system

	## 2020-10-11
	- Use mini audio by default

	## 2020-07-10
	- Upgraded to latest miniaudio.h (No more access to internal fields required)
	- Better playback format detection
	- Small changes to console output

	## 2019-10-31
	- Use final_audiodemo.h to prevent code duplication from FPL_Audio demo

	## 2019-10-26
	- New: Support for switching between miniaudio.h and FPL internal audio system
	- Fixed: Sine wave generation was generating invalid samples (Noises, Clicking)

	## 2019-07-22
	- Changed: Migrated from mini_al to miniaudio (Rebrand, Api-Change)

	## 2018-09-30
	- Initial version

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Set this to one to use miniaudio.h
// Set this to zero to use FPL internal audio
#define OPT_USE_MINIAUDIO 1

#define FPL_NO_WINDOW
#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_UNDEF
#include <final_platform_layer.h>
#include <math.h> // sinf

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#include <final_audiodemo.h>

#if OPT_USE_MINIAUDIO
#	define MINIAUDIO_IMPLEMENTATION
#	define MA_COPY_MEMORY(dst, src, size) fplMemoryCopy(src, size, dst)
#	define MA_ZERO_MEMORY(ptr, size) fplMemoryClear(ptr, size)
#	define MA_USE_RUNTIME_LINKING_FOR_PTHREAD
#	include <miniaudio/miniaudio.h>
#endif

#include <final_music.h>

typedef struct PlaybackAudioFormat {
	char backendName[FPL_MAX_NAME_LENGTH];
    char deviceName[FPL_MAX_NAME_LENGTH];
	fplAudioFormat deviceFormat;
} PlaybackAudioFormat;

typedef struct AudioContext {
#if OPT_USE_MINIAUDIO
	ma_format maTargtFormat;
	ma_device_config maDeviceConfig;
	ma_device maDevice;
	ma_context maContext;
#endif
	AudioSystem system;
	AudioSineWaveData sineWave;
	PlaybackAudioFormat playbackFormat;
} AudioContext;

static const float PI32 = 3.14159265359f;

#if OPT_USE_MINIAUDIO
static fplAudioFormatType MapMALFormatToFPLFormat(const ma_format mformat) {
	switch (mformat) {
		case ma_format_f32:
			return fplAudioFormatType_F32;
		case ma_format_s32:
			return fplAudioFormatType_S32;
		case ma_format_s24:
			return fplAudioFormatType_S24;
		case ma_format_s16:
			return fplAudioFormatType_S16;
		case ma_format_u8:
			return fplAudioFormatType_U8;
		default:
			return fplAudioFormatType_None;
	}
}

static ma_format MapFPLFormatToMALFormat(const fplAudioFormatType format) {
	switch (format) {
		case fplAudioFormatType_F32:
			return ma_format_f32;
		case fplAudioFormatType_S32:
			return ma_format_s32;
		case fplAudioFormatType_S24:
			return ma_format_s24;
		case fplAudioFormatType_S16:
			return ma_format_s16;
		case fplAudioFormatType_U8:
			return ma_format_u8;
		default:
			return ma_format_unknown;
	}
}

static fplAudioLatencyType MapMALPerformanceProfileToFPLLatencyType(const ma_performance_profile profile) {
    switch (profile) {
		case ma_performance_profile_low_latency:
			return fplAudioLatencyType_Low;
		case ma_performance_profile_conservative:
		default:
			return fplAudioLatencyType_Conservative;
    }
}

static fplAudioShareMode MapMAPShareModeToFPLShareMode(const ma_share_mode shareMode) {
    switch (shareMode) {
		case ma_share_mode_exclusive:
			return fplAudioShareMode_Exclusive;
		case ma_share_mode_shared:
		default:
			return fplAudioShareMode_Shared;
    }
}

static void AudioPlayback_MiniAudio(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	AudioContext *audioCtx = (AudioContext *)pDevice->pUserData;
	AudioSystem *audioSys = &audioCtx->system;
	AudioSystemWriteFrames(audioSys, pOutput, &audioCtx->playbackFormat.deviceFormat, frameCount, true);
}
#else
static uint32_t AudioPlayback_FPL(const fplAudioFormat *deviceFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	fplAssert(userData != fpl_null);
	AudioContext *audioCtx = (AudioContext *)userData;
	AudioSystem *audioSys = &audioCtx->system;
	AudioFrameIndex result = AudioSystemWriteFrames(audioSys, outputSamples, deviceFormat, maxFrameCount, true);
	return(result);
}
#endif

static void ReleaseAudioContext(AudioContext *context) {
	fplAssert(context != fpl_null);
#if OPT_USE_MINIAUDIO
	ma_device_uninit(&context->maDevice);
#endif // #if OPT_USE_MINIAUDIO	
}

static bool InitAudioContext(AudioContext *context, const fplAudioFormat *inFormat, PlaybackAudioFormat *playbackFormat) {
	fplAssert(context != fpl_null && inFormat != fpl_null && playbackFormat != fpl_null);

    fplClearStruct(playbackFormat);

	context->sineWave.frequency = 440;
	context->sineWave.toneVolume = 0.25f;
	context->sineWave.duration = 0.5;
	
#if OPT_USE_MINIAUDIO
	// Init audio playback (MiniAudio)
	context->maTargtFormat = MapFPLFormatToMALFormat(inFormat->type);
	context->maDeviceConfig = ma_device_config_init(ma_device_type_playback);
	context->maDeviceConfig.playback.channels = inFormat->channels;
	context->maDeviceConfig.playback.format = context->maTargtFormat;
	context->maDeviceConfig.sampleRate = inFormat->sampleRate;
	context->maDeviceConfig.dataCallback = AudioPlayback_MiniAudio;
	context->maDeviceConfig.pUserData = context;
	
	ma_result maResult;

#if 0
	// Only allow certain backends
	ma_backend malBackends[] = {
		ma_backend_alsa,
        ma_backend_pulseaudio,
        ma_backend_jack,
        ma_backend_dsound,
        ma_backend_wasapi,
    };
	maResult = ma_context_init(malBackends, fplArrayCount(malBackends), NULL, &context->maContext);
#else
    maResult = ma_context_init(NULL, 0, NULL, &context->maContext);
#endif

	if (maResult != MA_SUCCESS) {
		return false;
	}
	
	maResult = ma_device_init(&context->maContext, &context->maDeviceConfig, &context->maDevice);
	if (maResult != MA_SUCCESS) {
		ma_context_uninit(&context->maContext);
		return false;
	}

	fplAudioLatencyType latencyMode = MapMALPerformanceProfileToFPLLatencyType(context->maDeviceConfig.performanceProfile);
	fplAudioShareMode shareMode = MapMAPShareModeToFPLShareMode(context->maDevice.playback.shareMode);

	playbackFormat->deviceFormat.sampleRate = context->maDevice.sampleRate;
    playbackFormat->deviceFormat.channels = context->maDevice.playback.channels;
    playbackFormat->deviceFormat.channelLayout = fplGetDefaultAudioChannelLayoutFromChannels(context->maDevice.playback.channels);
    playbackFormat->deviceFormat.periods = context->maDevice.playback.internalPeriods;
    playbackFormat->deviceFormat.bufferSizeInFrames = context->maDevice.playback.internalPeriodSizeInFrames * context->maDevice.playback.internalPeriods;
    playbackFormat->deviceFormat.bufferSizeInMilliseconds = fplGetAudioBufferSizeInMilliseconds(context->maDevice.playback.internalSampleRate, playbackFormat->deviceFormat.bufferSizeInFrames);
    playbackFormat->deviceFormat.mode = fplCreateAudioMode(latencyMode, shareMode);
    playbackFormat->deviceFormat.defaultFields = fplAudioDefaultFields_None;
    playbackFormat->deviceFormat.type = MapMALFormatToFPLFormat(context->maDevice.playback.format);

	const char *backendName = ma_get_backend_name(context->maDevice.pContext->backend);
    fplCopyString(backendName, playbackFormat->backendName, fplArrayCount(playbackFormat->backendName));

	const char *deviceName = context->maDevice.playback.name;
    fplCopyString(deviceName, playbackFormat->deviceName, fplArrayCount(playbackFormat->deviceName));
#else
	fplGetAudioHardwareFormat(&playbackFormat->deviceFormat);

    fplAudioBackendType backendType = fplGetAudioBackendType();
	const char *backendName = fplGetAudioBackendName(backendType);
	fplCopyString(backendName, playbackFormat->backendName, fplArrayCount(playbackFormat->backendName));

	const char *deviceName = fplGetAudioHardwareDeviceName();
	fplCopyString(deviceName, playbackFormat->deviceName, fplArrayCount(playbackFormat->deviceName));
#endif // OPT_USE_MINIAUDIO
	
	return(true);
}

static bool StartPlayback(AudioContext *context) {
	fplAssert(context != fpl_null);

#if OPT_USE_MINIAUDIO
    if (ma_device_start(&context->maDevice) != MA_SUCCESS) {
		return(false);
	}
#else
	fplAudioResultType audioRes = fplPlayAudio();
	if (audioRes != fplAudioResultType_Success) {
		return(false);
	}
#endif
	
	return(true);
}

static void StopPlayback(AudioContext *context) {
	fplAssert(context != fpl_null);
#if OPT_USE_MINIAUDIO
	ma_device_stop(&context->maDevice);
#else
	fplStopAudio();
#endif
}

int main(int argc, char **args) {
	size_t fileCount = argc >= 2 ? argc - 1 : 0;
	const char** files = (fileCount > 0) ? (const char**)args + 1 : fpl_null;
	bool forceSineWave = false;

	AudioTrackSource audioTracks[8] = fplZeroInit;
	size_t audioTrackCount = 0;
	if (fileCount > 0) {
		size_t maxTrackCount = fplArrayCount(audioTracks);
		for (size_t i = 0; i < fplMin(maxTrackCount, fileCount); ++i) {
			AudioTrackSource *track = &audioTracks[audioTrackCount++];
			const char *filename = fplExtractFileName(files[i]);
			track->type = AudioTrackSourceType_URL;
			fplCopyString(filename, track->name, fplArrayCount(track->name));
			fplCopyString(files[i], track->url.urlOrFilePath, fplArrayCount(track->url.urlOrFilePath));
		}
	} else {
		AudioTrackSource *track = &audioTracks[audioTrackCount++];
		track->type = AudioTrackSourceType_Data;
		fplCopyString(name_musicTavsControlArgofox, track->name, fplArrayCount(track->name));
		track->data.size = sizeOf_musicTavsControlArgofox;
		track->data.data = ptr_musicTavsControlArgofox;
	}

	// Use default audio format from FPL as target format
	fplAudioFormat targetFormat = fplZeroInit;

	// NOTE(final): Our test audio file is encoded in 44100 Hz, Stereo and our audio system does not support up/down sample with non-even sample rates yet
	targetFormat.sampleRate = 44100;
	targetFormat.channels = 2;
	targetFormat.channelLayout = fplAudioChannelLayout_Stereo;

	int result = -1;

	// Allocate audio context
	AudioContext *audioContext = (AudioContext*)fplMemoryAllocate(sizeof(AudioContext));
	if (audioContext == fpl_null) {
		fplConsoleFormatError("Failed allocating memory for audio context!\n");
		goto releaseResources;
	}

	// Init FPL
	fplInitFlags initFlags = fplInitFlags_Console;
	fplSettings settings = fplMakeDefaultSettings();

#if !OPT_USE_MINIAUDIO
	settings.audio.targetFormat = targetFormat;
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;
	settings.audio.clientUserData = audioContext;
	settings.audio.clientReadCallback = AudioPlayback_FPL;
	initFlags |= fplInitFlags_Audio;
#else
	initFlags &= ~fplInitFlags_Audio;
#endif

	if (!fplPlatformInit(initFlags, &settings)) {
		fplConsoleFormatError("Failed initializing FPL with flags %d!\n", initFlags);
		goto releaseResources;
	}

    if (!InitAudioContext(audioContext, &targetFormat, &audioContext->playbackFormat)) {
		fplConsoleFormatError("Failed initializing audio context!\n");
		goto releaseResources;
	}
	
	AudioTrackList tracklist = fplZeroInit;

    if(!AudioSystemInit(&audioContext->system, &audioContext->playbackFormat.deviceFormat)) {
		fplConsoleFormatError("Failed initializing audio system!\n");
		goto releaseResources;
	}

	if(!LoadAudioTrackList(&audioContext->system, audioTracks, audioTrackCount, forceSineWave, &audioContext->sineWave, LoadAudioTrackFlags_AutoLoad | LoadAudioTrackFlags_AutoPlay, &tracklist)) {
		fplConsoleFormatError("Failed loading tracklist for %zu files!\n", fileCount);
		goto releaseResources;
	}

	// Start audio playback
	if (!StartPlayback(audioContext)) {
		fplConsoleFormatError("Failed starting audio playback!\n");
		goto releaseResources;
	}
	
	// Print output infos
	PlaybackAudioFormat *playbackFormat = &audioContext->playbackFormat;
	const char *formatName = fplGetAudioFormatName(playbackFormat->deviceFormat.type);
    const char *backendName = playbackFormat->backendName;
    const char *deviceName = playbackFormat->deviceName;

#if OPT_USE_MINIAUDIO
    const char *systemName = "MiniAudio";
#else
    const char *systemName = "FPL";
#endif

    fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %s, %s, %lu Hz, %lu channels, %lu frames, %lu periods)\n", audioContext->system.playItems.count, systemName, backendName, deviceName, formatName, playbackFormat->deviceFormat.sampleRate, playbackFormat->deviceFormat.channels, playbackFormat->deviceFormat.bufferSizeInFrames, playbackFormat->deviceFormat.periods);

	// Wait for any key presses
	fplConsoleFormatOut("Press any key to stop playback\n");
	fplConsoleWaitForCharInput();

	// Stop audio playback
	StopPlayback(audioContext);
		
	// Release audio data
	StopAllAudioTracks(&audioContext->system, &tracklist);
	AudioSystemShutdown(&audioContext->system);

	result = 0;

releaseResources:
	// Release audio device
	if (audioContext != fpl_null) {
		ReleaseAudioContext(audioContext);
	}

	// Release the platform
	if (fplIsPlatformInitialized())
		fplPlatformRelease();

	// Free audio context
	if (audioContext != fpl_null) {
		fplMemoryFree(audioContext);
	}

	return(result);
}
