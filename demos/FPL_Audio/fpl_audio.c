/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio
Description:
	This demo shows how to play a contiguous sine or square wave.
	Also it can play uncompressed PCM wave data with simple resampling support.
Requirements:
	- C-Runtime (sinf)
Author:
	Torsten Spaete
Changelog:
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
-------------------------------------------------------------------------------
*/

#define FPL_NO_WINDOW
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>
#include <math.h> // sinf

#include "audiosystem.h"

static const float PI32 = 3.14159265359f;

static uint32_t AudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioSystem *audioSys = (AudioSystem *)userData;
	uint32_t result = AudioSystemWriteSamples(audioSys, outFormat, frameCount, outputSamples);
	return(result);
}

static bool InitAudioData(AudioSystem *audioSys, const char *filePath) {
	if(!AudioSystemInit(audioSys)) {
		return false;
	}
	AudioSource *source = fpl_null;
	if(filePath != fpl_null) {
		source = AudioSystemLoadFileSource(audioSys, filePath);
		if(source != fpl_null) {
			AudioSystemPlaySource(audioSys, source, true);
		}
	}

	// Generate sine wave for some duration
	const double duration = 5.0f;
	const int toneHz = 256;
	const int toneVolume = 1000;
	uint32_t sampleCount = (uint32_t)(audioSys->nativeFormat.sampleRate * duration + 0.5);
	source = AudioSystemAllocateSource(audioSys, audioSys->nativeFormat.channels, audioSys->nativeFormat.sampleRate, fplAudioFormatType_S16, sampleCount);
	if(source != fpl_null) {
		int16_t *samples = (int16_t *)source->samples;
		int wavePeriod = source->samplesPerSeconds / toneHz;
		for(uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
			double t = 2.0f * PI32 * (double)sampleIndex / (float)wavePeriod;
			int16_t sampleValue = (int16_t)(sin(t) * toneVolume);
			for(uint32_t channelIndex = 0; channelIndex < source->channels; ++channelIndex) {
				*samples++ = sampleValue;
			}
		}
		AudioSystemPlaySource(audioSys, source, true);
	}
	return(true);
}

int main(int argc, char **args) {
	const char *filePath = (argc == 2) ? args[1] : fpl_null;

	AudioSystem audioSys = FPL_ZERO_INIT;

	//
	// Settings
	//
	fplSettings settings = fplMakeDefaultSettings();
	settings.audio.deviceFormat.type = fplAudioFormatType_S16;
	settings.audio.deviceFormat.channels = 2;

	//settings.audio.deviceFormat.sampleRate = 11025;
	//settings.audio.deviceFormat.sampleRate = 22050;
	settings.audio.deviceFormat.sampleRate = 44100;
	//settings.audio.deviceFormat.sampleRate = 48000;

	// Find audio device
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		return -1;
	}

	fplAudioDeviceInfo audioDeviceInfos[64] = FPL_ZERO_INIT;
	uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, FPL_ARRAYCOUNT(audioDeviceInfos));
	if(deviceCount > 0) {
		settings.audio.deviceInfo = audioDeviceInfos[0];
		fplConsoleFormatOut("Using audio device: %s\n", settings.audio.deviceInfo.name);
	}
	fplPlatformRelease();

	// Initialize the platform with audio enabled and the settings
	if(!fplPlatformInit(fplInitFlags_Audio, &settings)) {
		return -1;
	}

	// You can overwrite the client read callback and user data if you want to
	fplSetAudioClientReadCallback(AudioPlayback, &audioSys);

	// Init audio data
	if(InitAudioData(&audioSys, filePath)) {
		// Start audio playback (This will start calling clientReadCallback regulary)
		if(fplPlayAudio() == fplAudioResult_Success) {
			// Print output infos
			const char *outFormat = fplGetAudioFormatString(audioSys.nativeFormat.type);
			uint32_t outSampleRate = audioSys.nativeFormat.sampleRate;
			uint32_t outChannels = audioSys.nativeFormat.channels;
			fplConsoleFormatOut("Playing %lu audio sources (%s, %lu Hz, %lu channels)\n", audioSys.playItems.count, outFormat, outSampleRate, outChannels);

			// Wait for any key presses
			fplConsoleFormatOut("Press any key to stop playback\n", outFormat, outSampleRate, outChannels);
			fplConsoleWaitForCharInput();

			// Stop audio playback
			fplStopAudio();
		}
		// Release audio data
		AudioSystemShutdown(&audioSys);
	}

	// Release the platform
	fplPlatformRelease();

	return 0;
}