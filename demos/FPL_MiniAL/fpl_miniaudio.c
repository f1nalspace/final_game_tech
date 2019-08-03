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
	## 2019-07-22
	- Changed: Migrated from mini_al to miniaudio (Rebrand, Api-Change)

	## 2018-09-30
	- Initial version

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_NO_WINDOW
#define FPL_IMPLEMENTATION
#define FPL_NO_UNDEF
#include <final_platform_layer.h>
#include <math.h> // sinf

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#define MINIAUDIO_IMPLEMENTATION
#define MA_USE_RUNTIME_LINKING_FOR_PTHREAD
#include <miniaudio/miniaudio.h>

static const float PI32 = 3.14159265359f;

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

static uint32_t AudioPlayback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	AudioSystem *audioSys = (AudioSystem *)pDevice->pUserData;
	fplAudioDeviceFormat outFormat = fplZeroInit;
	outFormat.channels = pDevice->playback.channels;
	outFormat.sampleRate = pDevice->sampleRate;
	outFormat.type = MapMALFormatToFPLFormat(pDevice->playback.format);
	outFormat.bufferSizeInFrames = pDevice->playback.internalBufferSizeInFrames;
	fplAssert(outFormat.type != fplAudioFormatType_None);
	AudioSampleIndex result = AudioSystemWriteSamples(audioSys, pOutput, &outFormat, frameCount);
	return(result);
}

static bool InitAudioData(const fplAudioDeviceFormat *targetFormat, AudioSystem *audioSys, const char *filePath, const bool generateSineWave) {
	if (!AudioSystemInit(audioSys, targetFormat)) {
		return false;
	}

	// Play audio file
	AudioSource *source = fpl_null;
	if (filePath != fpl_null) {
		source = AudioSystemLoadFileSource(audioSys, filePath);
		if (source != fpl_null) {
			AudioSystemPlaySource(audioSys, source, true, 0.25f);
		}
	}

	// Generate sine wave for some duration
	if (generateSineWave) {
		const double duration = 0.5f;
		const int toneHz = 256;
		const int toneVolume = INT16_MAX / 2;
		uint32_t sampleCount = (uint32_t)(audioSys->targetFormat.sampleRate * duration + 0.5);
		source = AudioSystemAllocateSource(audioSys, audioSys->targetFormat.channels, audioSys->targetFormat.sampleRate, fplAudioFormatType_S16, sampleCount);
		if (source != fpl_null) {
			int16_t *samples = (int16_t *)source->buffer.samples;
			int wavePeriod = source->format.sampleRate / toneHz;
			for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
				double t = 2.0f * PI32 * (double)sampleIndex / (float)wavePeriod;
				int16_t sampleValue = (int16_t)(sin(t) * toneVolume);
				for (uint32_t channelIndex = 0; channelIndex < source->format.channels; ++channelIndex) {
					*samples++ = sampleValue;
				}
			}
			AudioSystemPlaySource(audioSys, source, true, 1.0f);
		}
	}
	return(true);
}

int main(int argc, char **args) {
	const char *filePath = (argc == 2) ? args[1] : fpl_null;
	const bool generateSineWave = (filePath == fpl_null);

	// Use default audio format from FPL as target format
	fplAudioTargetFormat targetFormat;
	fplSetDefaultAudioTargetFormat(&targetFormat);
	targetFormat.channels = 2;
	targetFormat.type = fplAudioFormatType_S16;
	targetFormat.sampleRate = 44100;

	// Create empty audio system
	AudioSystem audioSys = fplZeroInit;

	// Init audio playback
	ma_format maFormat = MapFPLFormatToMALFormat(targetFormat.type);
	ma_device_config maDeviceConfig = ma_device_config_init(ma_device_type_playback);
	maDeviceConfig.playback.channels = targetFormat.channels;
	maDeviceConfig.playback.format = maFormat;
	maDeviceConfig.sampleRate = targetFormat.sampleRate;
	maDeviceConfig.dataCallback = AudioPlayback;
	maDeviceConfig.pUserData = &audioSys;

	ma_device maDevice;
	ma_context maContext;
	ma_result maResult;

#if 1
	ma_backend malBackends[] = {
		ma_backend_dsound,
		ma_backend_wasapi,
		ma_backend_winmm,
		ma_backend_alsa,
		ma_backend_pulseaudio,
	};
	ma_uint32 malBackendCount = fplArrayCount(malBackends);
#else
	ma_backend *malBackends = fpl_null;
	ma_uint32 malBackendCount = 0;
#endif

	maResult = ma_context_init(malBackends, malBackendCount, NULL, &maContext);
	if (maResult != MA_SUCCESS) {
		return -1;
	}
	maResult = ma_device_init(&maContext, &maDeviceConfig, &maDevice);
	if (maResult != MA_SUCCESS) {
		return -1;
	}

	// Init FPL
	fplSettings settings = fplMakeDefaultSettings();
	if (!fplPlatformInit(fplInitFlags_Console, &settings)) {
		ma_device_uninit(&maDevice);
		return -1;
	}

	// Init audio data
	fplAudioDeviceFormat targetDeviceFormat;
	fplConvertAudioTargetFormatToDeviceFormat(&targetFormat, &targetDeviceFormat);
	if (InitAudioData(&targetDeviceFormat, &audioSys, filePath, generateSineWave)) {
		// Start audio playback
		if (ma_device_start(&maDevice) == MA_SUCCESS) {
			// Print output infos
			const char *outDriver = ma_get_backend_name(maDevice.pContext->backend);
			const char *outFormat = fplGetAudioFormatString(targetFormat.type);
			fplAudioDeviceFormat deviceFormat = fplZeroInit;
			deviceFormat.channels = maDevice.playback.channels;
			deviceFormat.periods = maDevice.playback.internalPeriods;
			deviceFormat.sampleRate = maDevice.sampleRate;
			deviceFormat.type = MapMALFormatToFPLFormat(maDevice.playback.format);
			deviceFormat.bufferSizeInFrames = maDevice.playback.internalBufferSizeInFrames;
			deviceFormat.bufferSizeInBytes = fplGetAudioBufferSizeInBytes(deviceFormat.type, deviceFormat.channels, deviceFormat.bufferSizeInFrames);
			fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n", audioSys.playItems.count, outDriver, outFormat, deviceFormat.sampleRate, deviceFormat.channels);

			// Wait for any key presses
			fplConsoleFormatOut("Press any key to stop playback\n");
			fplConsoleWaitForCharInput();

			// Stop audio playback
			ma_device_stop(&maDevice);
		}
		// Release audio data
		AudioSystemShutdown(&audioSys);
	}

	// Release audio device
	ma_device_uninit(&maDevice);

	// Release the platform
	fplPlatformRelease();

	return 0;
}