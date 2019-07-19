/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | mini_al

Description:
	This demo shows how to use the mini-al library with FPL.

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- mini_al

Author:
	Torsten Spaete

Changelog:
	## 2018-09-30
	- Initial version
-------------------------------------------------------------------------------
*/

#define FPL_NO_WINDOW
#define FPL_IMPLEMENTATION
#define FPL_NO_UNDEF
#include <final_platform_layer.h>
#include <math.h> // sinf

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include <final_audiosystem.h>

#define MINI_AL_IMPLEMENTATION
#define MAL_USE_RUNTIME_LINKING_FOR_PTHREAD
#include <minial/mini_al.h>

static const float PI32 = 3.14159265359f;

static fplAudioFormatType MapMALFormatToFPLFormat(const mal_format mformat) {
	switch (mformat) {
		case mal_format_f32:
			return fplAudioFormatType_F32;
		case mal_format_s32:
			return fplAudioFormatType_S32;
		case mal_format_s24:
			return fplAudioFormatType_S24;
		case mal_format_s16:
			return fplAudioFormatType_S16;
		case mal_format_u8:
			return fplAudioFormatType_U8;
		default:
			return fplAudioFormatType_None;
	}
}

static mal_format MapFPLFormatToMALFormat(const fplAudioFormatType format) {
	switch (format) {
		case fplAudioFormatType_F32:
			return mal_format_f32;
		case fplAudioFormatType_S32:
			return mal_format_s32;
		case fplAudioFormatType_S24:
			return mal_format_s24;
		case fplAudioFormatType_S16:
			return mal_format_s16;
		case fplAudioFormatType_U8:
			return mal_format_u8;
		default:
			return mal_format_unknown;
	}
}

static uint32_t AudioPlayback(mal_device* pDevice, mal_uint32 frameCount, void* pSamples) {
	AudioSystem *audioSys = (AudioSystem *)pDevice->pUserData;
	fplAudioDeviceFormat outFormat = fplZeroInit;
	outFormat.channels = pDevice->channels;
	outFormat.sampleRate = pDevice->sampleRate;
	outFormat.type = MapMALFormatToFPLFormat(pDevice->format);
	outFormat.bufferSizeInFrames = pDevice->bufferSizeInFrames;
	fplAssert(outFormat.type != fplAudioFormatType_None);
	uint32_t result = AudioSystemWriteSamples(audioSys, &outFormat, frameCount, (uint8_t *)pSamples);
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
			int16_t *samples = (int16_t *)source->samples;
			int wavePeriod = source->samplesPerSeconds / toneHz;
			for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
				double t = 2.0f * PI32 * (double)sampleIndex / (float)wavePeriod;
				int16_t sampleValue = (int16_t)(sin(t) * toneVolume);
				for (uint32_t channelIndex = 0; channelIndex < source->channels; ++channelIndex) {
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
	const bool generateSineWave = true;

	// Use default audio format from FPL as target format
	fplAudioTargetFormat targetFormat;
	fplSetDefaultAudioTargetFormat(&targetFormat);
	targetFormat.channels = 2;
	targetFormat.type = fplAudioFormatType_S16;
	targetFormat.sampleRate = 44100;

	// Create empty audio system
	AudioSystem audioSys = fplZeroInit;

	// Init audio playback
	mal_format malFormat = MapFPLFormatToMALFormat(targetFormat.type);
	mal_device_config malDeviceConfig = mal_device_config_init_playback(malFormat, targetFormat.channels, targetFormat.sampleRate, AudioPlayback);
	mal_device malDevice;
	mal_context malContext;
	mal_result malResult;

#if 1
	mal_backend malBackends[] = {
		mal_backend_dsound,
		mal_backend_wasapi,
		mal_backend_winmm,
		mal_backend_alsa,
		mal_backend_pulseaudio,
	};
	mal_uint32 malBackendCount = fplArrayCount(malBackends);
#else
	mal_backend *malBackends = fpl_null;
	mal_uint32 malBackendCount = 0;
#endif

	malResult = mal_context_init(malBackends, malBackendCount, NULL, &malContext);
	if (malResult != MAL_SUCCESS) {
		return -1;
	}
	malResult = mal_device_init(&malContext, mal_device_type_playback, NULL, &malDeviceConfig, &audioSys, &malDevice);
	if (malResult != MAL_SUCCESS) {
		return -1;
	}

	// Init FPL
	fplSettings settings = fplMakeDefaultSettings();
	if (!fplPlatformInit(fplInitFlags_Console, &settings)) {
		mal_device_uninit(&malDevice);
		return -1;
	}

	// Init audio data
	fplAudioDeviceFormat targetDeviceFormat;
	fplConvertAudioTargetFormatToDeviceFormat(&targetFormat, &targetDeviceFormat);
	if (InitAudioData(&targetDeviceFormat, &audioSys, filePath, generateSineWave)) {
		// Start audio playback
		if (mal_device_start(&malDevice) == MAL_SUCCESS) {
			// Print output infos
			const char *outDriver = mal_get_backend_name(malDevice.pContext->backend);
			const char *outFormat = fplGetAudioFormatString(targetFormat.type);
			fplAudioDeviceFormat deviceFormat = fplZeroInit;
			deviceFormat.channels = malDevice.channels;
			deviceFormat.periods = malDevice.periods;
			deviceFormat.sampleRate = malDevice.sampleRate;
			deviceFormat.type = MapMALFormatToFPLFormat(malDevice.format);
			deviceFormat.bufferSizeInFrames = malDevice.bufferSizeInFrames;
			deviceFormat.bufferSizeInBytes = fplGetAudioBufferSizeInBytes(deviceFormat.type, deviceFormat.channels, deviceFormat.bufferSizeInFrames);
			fplConsoleFormatOut("Playing %lu audio sources (%s, %s, %lu Hz, %lu channels)\n", audioSys.playItems.count, outDriver, outFormat, deviceFormat.sampleRate, deviceFormat.channels);

			// Wait for any key presses
			fplConsoleFormatOut("Press any key to stop playback\n");
			fplConsoleWaitForCharInput();

			// Stop audio playback
			mal_device_stop(&malDevice);
		}
		// Release audio data
		AudioSystemShutdown(&audioSys);
	}

	// Release audio device
	mal_device_uninit(&malDevice);

	// Release the platform
	fplPlatformRelease();

	return 0;
}