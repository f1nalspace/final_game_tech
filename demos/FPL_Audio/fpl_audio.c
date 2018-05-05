/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Audio
Description:
	This demo shows how to play a contiguous sine or square wave
	in a fixed default format such as S16.
Requirements:
	- C-Runtime (sinf)
Author:
	Torsten Spaete
Changelog:
 	## 2018-05-5:
 	- Fixed CMakeLists to compile properly
 	- Fixed Makefile to compile properly
	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#include <final_platform_layer.h>
#include <math.h> // sinf

typedef struct AudioTest {
	uint32_t toneHz;
	uint32_t toneVolume;
	uint32_t runningSampleIndex;
	uint32_t wavePeriod;
	bool useSquareWave;
} AudioTest;

static const float PI32 = 3.14159265359f;

static uint32_t FillAudioBuffer(const fplAudioDeviceFormat *nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioTest *audioTest = (AudioTest *)userData;
	FPL_ASSERT(audioTest != NULL);
	FPL_ASSERT(nativeFormat->type == fplAudioFormatType_S16);
	uint32_t result = 0;
	int16_t *outSamples = (int16_t *)outputSamples;
	uint32_t halfWavePeriod = audioTest->wavePeriod / 2;
	for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		int16_t sampleValue;
		if (audioTest->useSquareWave) {
			sampleValue = ((audioTest->runningSampleIndex++ / halfWavePeriod) % 2) ? (int16_t)audioTest->toneVolume : -(int16_t)audioTest->toneVolume;
		} else {
			float t = 2.0f * PI32 * (float)audioTest->runningSampleIndex++ / (float)audioTest->wavePeriod;
			sampleValue = (int16_t)(sinf(t) * audioTest->toneVolume);
		}
		for (uint32_t channelIndex = 0; channelIndex < nativeFormat->channels; ++channelIndex) {
			*outSamples++ = sampleValue;
			++result;
		}
	}
	return result;
}

int main(int argc, char **args) {
	int result = -1;

	//
	// Create default settings with 48 KHz, 2 Channels, S16-Format
	//
	fplSettings settings = fplMakeDefaultSettings();

	// Optionally overwrite audio settings if needed

	// Setup some state for the sine/square wave generation
	AudioTest audioTest = FPL_ZERO_INIT;
	audioTest.toneHz = 256;
	audioTest.toneVolume = 1000;
	audioTest.wavePeriod = settings.audio.deviceFormat.sampleRate / audioTest.toneHz;
	audioTest.useSquareWave = false;

	// Provide client read callback and optionally user data
	settings.audio.clientReadCallback = FillAudioBuffer;
	settings.audio.userData = &audioTest;
	settings.audio.deviceFormat.type = fplAudioFormatType_S16;
	settings.audio.deviceFormat.channels = 2;
	settings.audio.deviceFormat.sampleRate = 48000;

	// Find audio device
	if (fplPlatformInit(fplInitFlags_Audio, &settings)) {
		fplAudioDeviceInfo audioDeviceInfos[16] = FPL_ZERO_INIT;
		uint32_t deviceCount = fplGetAudioDevices(audioDeviceInfos, FPL_ARRAYCOUNT(audioDeviceInfos));
		if (deviceCount > 0) {
			settings.audio.deviceInfo = audioDeviceInfos[0];
			fplConsoleFormatOut("Using audio device: %s\n", settings.audio.deviceInfo.name);
		}
		fplPlatformRelease();
	}

	// Initialize the platform with audio enabled and the settings
	if (fplPlatformInit(fplInitFlags_Audio, &settings)) {
		// Print out the native audio format
		fplAudioDeviceFormat nativeFormat;
		if(fplGetAudioHardwareFormat(&nativeFormat)) {
			// You can overwrite the client read callback and user data if you want to
			fplSetAudioClientReadCallback(FillAudioBuffer, &audioTest);
			// Start audio playback (This will start calling clientReadCallback regulary)
			if(fplPlayAudio() == fplAudioResult_Success) {
				fplConsoleFormatOut("Audio with %lu KHz and %lu channels is playing, press any key to stop playback...\n", nativeFormat.sampleRate, nativeFormat.channels);
				// Wait for any key presses
				fplConsoleWaitForCharInput();
				// Stop audio playback
				fplStopAudio();
			}
		}
		// Release the platform
		fplPlatformRelease();
		result = 0;
	}
	return(result);
}