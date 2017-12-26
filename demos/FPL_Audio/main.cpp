/*FPL_AUTO_NAMESPACE
*******************************************************************************
FPL Simple Audio Playback Demo
*******************************************************************************
This demo plays a contiguous sine or square wave in the expected S16 format.

- You fill out the audio settings you want or leave the default
- You fill out the audio client read callback and user data pointer
- You initialize the platform with InitFlags::Audio and pass your settings as second parameter
- You start playing the audio using PlayAudio()
- When you are done you stop it using StopAudio()
- You release the platform
- Done
*/

#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#define FPL_NO_WINDOW
#include <final_platform_layer.hpp>
#include <math.h> // sinf
#include <stdio.h> // getchar

struct AudioTest {
	fpl_u32 toneHz;
	fpl_u32 toneVolume;
	fpl_u32 runningSampleIndex;
	fpl_u32 wavePeriod;
	bool useSquareWave;
};

static const float PI32 = 3.14159265359f;

static fpl_u32 FillAudioBuffer(const AudioDeviceFormat &nativeFormat, const fpl_u32 frameCount, void *outputSamples, void *userData) {
	AudioTest *audioTest = (AudioTest *)userData;
	FPL_ASSERT(audioTest != fpl_null);
	FPL_ASSERT(nativeFormat.type == AudioFormatType::S16);
	fpl_u32 result = 0;
	fpl_s16 *outSamples = (fpl_s16 *)outputSamples;
	fpl_u32 halfWavePeriod = audioTest->wavePeriod / 2;
	for (fpl_u32 frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		fpl_s16 sampleValue;
		if (audioTest->useSquareWave) {
			sampleValue = ((audioTest->runningSampleIndex++ / halfWavePeriod) % 2) ? (fpl_s16)audioTest->toneVolume : -(fpl_s16)audioTest->toneVolume;
		} else {
			float t = 2.0f * PI32 * (float)audioTest->runningSampleIndex++ / (float)audioTest->wavePeriod;
			sampleValue = (fpl_s16)(sinf(t) * audioTest->toneVolume);
		}
		for (fpl_u32 channelIndex = 0; channelIndex < nativeFormat.channels; ++channelIndex) {
			*outSamples++ = sampleValue;
			++result;
		}
	}
	return result;
}

int main(int argc, char **args) {
	int result = -1;

	// Initialize to default settings which is 48 KHz and 2 Channels
	Settings settings = DefaultSettings();

	// Optionally overwrite audio settings if needed

	// Setup some state for the sine/square wave generation
	AudioTest audioTest = {};
	audioTest.toneHz = 256;
	audioTest.toneVolume = 1000;
	audioTest.wavePeriod = settings.audio.desiredFormat.sampleRate / audioTest.toneHz;
	audioTest.useSquareWave = false;

	// Provide client read callback and optionally user data
	settings.audio.clientReadCallback = FillAudioBuffer;
	settings.audio.userData = &audioTest;

	// Initialize the platform with audio enabled and the settings
	if (InitPlatform(InitFlags::Audio, settings)) {
		// You can overwrite the client read callback and user data if you want to
		SetAudioClientReadCallback(FillAudioBuffer, &audioTest);
		// Start audio playback (This will start calling clientReadCallback regulary)
		if (PlayAudio() == AudioResult::Success) {
			// Print out the native audio format
			AudioDeviceFormat nativeFormat = GetAudioHardwareFormat();
			ConsoleFormatOut("Audio with %lu KHz and %lu channels is playing, press any key to stop playback...\n", nativeFormat.sampleRate, nativeFormat.channels);
			// Wait for any key presses
			getchar();
			// Stop audio playback
			StopAudio();
		}
		// Release the platform
		ReleasePlatform();
		result = 0;
	}
	return(result);
}