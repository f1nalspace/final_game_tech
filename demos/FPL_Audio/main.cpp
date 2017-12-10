#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#define FPL_NO_WINDOW
#include <final_platform_layer.hpp>
#include <math.h>
#include <stdio.h>

struct AudioTest {
	uint32_t toneHz;
	uint32_t toneVolume;
	uint32_t runningSampleIndex;
	uint32_t wavePeriod;
	bool useSquareWave;
};

static const float PI32 = 3.14159265359f;

static uint32_t FillAudioBuffer(const AudioDeviceFormat &nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioTest *audioTest = (AudioTest *)userData;
	FPL_ASSERT(audioTest != nullptr);
	FPL_ASSERT(nativeFormat.type == AudioFormatType::S16);
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
		for (uint32_t channelIndex = 0; channelIndex < nativeFormat.channels; ++channelIndex) {
			*outSamples++ = sampleValue;
			++result;
		}
	}
	return result;
}

int main(int argc, char **args) {
	int result = -1;

	Settings settings = DefaultSettings();

	AudioTest audioTest = {};
	audioTest.toneHz = 256;
	audioTest.toneVolume = 1000;
	audioTest.wavePeriod = settings.audio.desiredFormat.sampleRate / audioTest.toneHz;
	audioTest.useSquareWave = false;

	settings.audio.clientReadCallback = FillAudioBuffer;
	settings.audio.userData = &audioTest;

	if (InitPlatform(InitFlags::Audio, settings)) {
		if (PlayAudio() == AudioResult::Success) {
			ConsoleOut("Audio is playing, press any key to stop playback...\n");
			getchar();
			StopAudio();
		}
		ReleasePlatform();
	}
	return(result);
}