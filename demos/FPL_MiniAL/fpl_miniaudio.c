/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | miniaudio

Description:
	This demo shows how to use the miniaudio library with FPL.

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- miniaudio 0.9.5+

Author:
	Torsten Spaete

Changelog:
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
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Set this to one to use miniaudio.h
// Set this to zero to use FPL internal audio
#define OPT_USE_MINIAUDIO 0

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
#	define MA_USE_RUNTIME_LINKING_FOR_PTHREAD
#	include <miniaudio/miniaudio.h>
#endif

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

static void AudioPlayback_MiniAudio(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	AudioSystem *audioSys = (AudioSystem *)pDevice->pUserData;
	fplAudioDeviceFormat outFormat = fplZeroInit;
	outFormat.channels = pDevice->playback.channels;
	outFormat.sampleRate = pDevice->sampleRate;
	outFormat.type = MapMALFormatToFPLFormat(pDevice->playback.format);
	outFormat.bufferSizeInFrames = pDevice->playback.internalBufferSizeInFrames;
	fplAssert(outFormat.type != fplAudioFormatType_None);
	AudioSystemWriteSamples(audioSys, pOutput, &outFormat, frameCount);
}
#else
static uint32_t AudioPlayback_FPL(const fplAudioDeviceFormat *outFormat, const uint32_t maxFrameCount, void *outputSamples, void *userData) {
	AudioSystem *audioSys = (AudioSystem *)userData;
	AudioSampleIndex result = AudioSystemWriteSamples(audioSys, outputSamples, outFormat, maxFrameCount);
	return(result);
}
#endif

typedef struct AudioContext {
#if OPT_USE_MINIAUDIO
	ma_format maTargtFormat;
	ma_device_config maDeviceConfig;
	ma_device maDevice;
	ma_context maContext;
#else
#endif
	AudioSystem system;
	AudioSineWaveData sineWave;
} AudioContext;

typedef struct PlaybackAudioFormat {
	char driverName[FPL_MAX_NAME_LENGTH];
	fplAudioDeviceFormat deviceFormat;
} PlaybackAudioFormat;

static void ReleaseAudioContext(AudioContext *context) {
	fplAssert(context != fpl_null);
#if OPT_USE_MINIAUDIO
	ma_device_uninit(&context->maDevice);
#endif // #if OPT_USE_MINIAUDIO	
}

static bool InitAudioContext(AudioContext *context, const fplAudioTargetFormat targetFormat) {
	fplAssert(context != fpl_null);
	context->sineWave.frequency = 440;
	context->sineWave.toneVolume = 0.25f;
	context->sineWave.duration = 0.5;
	
#if OPT_USE_MINIAUDIO
	// Init audio playback (MiniAudio)
	context->maTargtFormat = MapFPLFormatToMALFormat(targetFormat.type);
	context->maDeviceConfig = ma_device_config_init(ma_device_type_playback);
	context->maDeviceConfig.playback.channels = targetFormat.channels;
	context->maDeviceConfig.playback.format = context->maTargtFormat;
	context->maDeviceConfig.sampleRate = targetFormat.sampleRate;
	context->maDeviceConfig.dataCallback = AudioPlayback_MiniAudio;
	context->maDeviceConfig.pUserData = &context->system;
	
	ma_backend malBackends[] = {
		ma_backend_dsound,
		ma_backend_wasapi,
		ma_backend_winmm,
		ma_backend_alsa,
		ma_backend_pulseaudio,
	};
	ma_uint32 malBackendCount = fplArrayCount(malBackends);
	
	ma_result maResult;

	maResult = ma_context_init(malBackends, malBackendCount, NULL, &context->maContext);
	if (maResult != MA_SUCCESS) {
		return false;
	}
	
	maResult = ma_device_init(&context->maContext, &context->maDeviceConfig, &context->maDevice);
	if (maResult != MA_SUCCESS) {
		ma_context_uninit(&context->maContext);
		return false;
	}
#else
	// No need to initialize anything here for FPL
#endif // OPT_USE_MINIAUDIO
	
	return(true);
}

static bool StartPlayback(AudioContext *context, PlaybackAudioFormat *playbackFormat) {
	fplAssert(context != fpl_null);
	fplAssert(playbackFormat != fpl_null);
	
	fplClearStruct(playbackFormat);

#if OPT_USE_MINIAUDIO
	if (ma_device_start(&context->maDevice) != MA_SUCCESS) {
		return(false);
	}
	
	fplAudioDeviceFormat *actualDeviceFormat = &playbackFormat->deviceFormat;

	const char *outDriver = ma_get_backend_name(context->maDevice.pContext->backend);
	fplCopyString(outDriver, playbackFormat->driverName, fplArrayCount(playbackFormat->driverName));
	actualDeviceFormat->channels = context->maDevice.playback.channels;
	actualDeviceFormat->periods = context->maDevice.playback.internalPeriods;
	actualDeviceFormat->sampleRate = context->maDevice.sampleRate;
	actualDeviceFormat->type = MapMALFormatToFPLFormat(context->maDevice.playback.format);
	actualDeviceFormat->bufferSizeInFrames = context->maDevice.playback.internalBufferSizeInFrames;
	actualDeviceFormat->bufferSizeInBytes = fplGetAudioBufferSizeInBytes(actualDeviceFormat->type, actualDeviceFormat->channels, actualDeviceFormat->bufferSizeInFrames);
#else
	fplAudioResultType audioRes = fplPlayAudio();
	if (audioRes != fplAudioResultType_Success) {
		return(false);
	}

	fplGetAudioHardwareFormat(&playbackFormat->deviceFormat);
	const fplSettings *settings = fplGetCurrentSettings();
	const char *outDriver = fplGetAudioDriverString(settings->audio.driver);
	fplCopyString(outDriver, playbackFormat->driverName, fplArrayCount(playbackFormat->driverName));
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

	// Use default audio format from FPL as target format
	fplAudioTargetFormat targetFormat;
	fplSetDefaultAudioTargetFormat(&targetFormat);
	targetFormat.channels = 2;
	targetFormat.type = fplAudioFormatType_S16;
	targetFormat.sampleRate = 44100;

	int result = -1;

	// Init audio context
	AudioContext *audioContext = (AudioContext*)fplMemoryAllocate(sizeof(AudioContext));
	if (!InitAudioContext(audioContext, targetFormat)) {
		fplConsoleFormatError("Failed initializing audio context!\n");
		goto releaseResources;
	}
	
	// Init FPL
	fplInitFlags initFlags = fplInitFlags_Console;
	fplSettings settings = fplMakeDefaultSettings();
	
#if !OPT_USE_MINIAUDIO
	settings.audio.targetFormat = targetFormat;
	settings.audio.startAuto = false;
	settings.audio.stopAuto = false;
	settings.audio.userData = &audioContext->system;
	settings.audio.clientReadCallback = AudioPlayback_FPL;
	initFlags |= fplInitFlags_Audio;
#endif
	
	if (!fplPlatformInit(initFlags, &settings)) {
		fplConsoleFormatError("Failed initializing FPL with flags %d!\n", initFlags);
		goto releaseResources;
	}

	// Init audio data
	fplAudioDeviceFormat targetDeviceFormat;
	fplConvertAudioTargetFormatToDeviceFormat(&targetFormat, &targetDeviceFormat);
	
	if (!InitAudioData(&targetDeviceFormat, &audioContext->system, files, fileCount, forceSineWave, &audioContext->sineWave)){
		fplConsoleFormatError("Failed initializing audio system!\n");
		goto releaseResources;
	}
	
	// Start audio playback
	PlaybackAudioFormat playbackFormat = fplZeroInit;
	if (!StartPlayback(audioContext, &playbackFormat)) {
		fplConsoleFormatError("Failed starting audio playback!\n");
		goto releaseResources;
	}
	
	// Print output infos
	const char *formatName = fplGetAudioFormatTypeString(playbackFormat.deviceFormat.type);
	fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n", audioContext->system.playItems.count, playbackFormat.driverName, formatName, playbackFormat.deviceFormat.sampleRate, playbackFormat.deviceFormat.channels);

	// Wait for any key presses
	fplConsoleFormatOut("Press any key to stop playback\n");
	fplConsoleWaitForCharInput();

	// Stop audio playback
	StopPlayback(audioContext);
		
	// Release audio data
	AudioSystemShutdown(&audioContext->system);

	result = 0;

releaseResources:
	// Release audio device
	ReleaseAudioContext(audioContext);

	// Release the platform
	if (fplIsPlatformInitialized())
		fplPlatformRelease();

	// Release memory
	fplMemoryFree(audioContext);

	return(result);
}