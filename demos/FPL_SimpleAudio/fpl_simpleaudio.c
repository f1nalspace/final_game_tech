/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Simple Audio

Description:
	This sample shows how to play audio samples in the simplest way.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2025-03-25
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

// Sine wave generator state
static uint32_t globalWaveGeneratorToneHz = 256;
static uint32_t globalWaveGeneratorToneVolume = 1000;
static uint32_t globalWaveGeneratorCurrentSampleIndex = 0;
static uint32_t globalWaveGeneratorPeriod = 0;

// Thread that is executed when our audio device was started, that is called thousands of times per seconds
// Do not use any locking mechanism, such as spinlocks and don't use any I/O operations here
// This function has a very tight time budget of rougly ~2-3 ms (depends on the buffer size, the number of channels and the sample rate)
static uint32_t AudioPlaybackThread(const fplAudioFormat *nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	// To make this demo easier, we don't do any sample format conversion and force the output to S16
	// If the sound device does not support S16, then we can't play any audio samples/frames
	if (nativeFormat->type != fplAudioFormatType_S16) {
		return 0;
	}

	// Initialize sine wave generator once
	// If you want to experiment with the tone hz, keep in mind that you have to re-initialize the period when you change the tone hz!
	if (globalWaveGeneratorPeriod == 0) {
		globalWaveGeneratorPeriod = nativeFormat->sampleRate / globalWaveGeneratorToneHz;
	}

	// Output format is expected to be S16
	fplAssert(nativeFormat->type == fplAudioFormatType_S16);

	// Our outputs samples are opaque, but we know from the assertion above that we are S16, so we can safely cast it to a int16_t pointer
	int16_t *outSamples = (int16_t *)outputSamples;

	// Number of frames that is written to the sound device
	uint32_t numFramesOut = 0;

	for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
		uint32_t index = globalWaveGeneratorCurrentSampleIndex++;
		float t = sinf((2.0f * (float)M_PI * globalWaveGeneratorPeriod) / (float)nativeFormat->sampleRate * index);
		int16_t monoSampleValue = (int16_t)(t * globalWaveGeneratorToneVolume);
		for (uint32_t channelIndex = 0; channelIndex < nativeFormat->channels; ++channelIndex) {
			*outSamples++ = monoSampleValue;
		}
		++numFramesOut;
	}

	// Return the generated frame count - do not confuse that with samples!
	fplAssert(numFramesOut == frameCount);
	return numFramesOut;
}

int main(int argc, char **argv) {
	// Setup FPL and force the audio format to S16, 44100 Hz, Stereo
	// Note that, there is no guarantee that every sound device supports this!
	// Modern sound devices with 6+ channels may require 48 KHz or even 96 KHz
	// Real audio applications must do sample rate conversion from any sample rate configurations, such as 44100 to 48000 or 22050 to 44100, etc.
	fplSettings settings = fplZeroInit;
	fplSetDefaultSettings(&settings);
	settings.audio.clientReadCallback = AudioPlaybackThread;
	
	// Try to force stereo channel layout
	settings.audio.targetFormat.channelLayout = fplAudioChannelLayout_Stereo;

	// Try to force S16 as format
	settings.audio.targetFormat.type = fplAudioFormatType_S16;

	// Try to force to 44100 Hz, which is the most common used sample rate
	settings.audio.targetFormat.sampleRate = 44100;

	// Always start and stop the playback automatically
	settings.audio.startAuto = true;
	settings.audio.stopAuto = true;

	// Initialize FPL with console and audio only
	// We don't need any video or window configuration
	if (!fplPlatformInit(fplInitFlags_Console | fplInitFlags_Audio, &settings)) {
		return -1;
	}

	// Get actual audio hardware format, which may be different that our "target" format.
	fplAudioFormat hardwareFormat = fplZeroInit;
	fplGetAudioHardwareFormat(&hardwareFormat);

	// Get name of the audio format (S16, F32, etc.)
	const char *audioFormatName = fplGetAudioFormatName(hardwareFormat.type);

	// Print out some infos and wait for a key to be pressed
	// While we are waiting for a key press, new audio samples will be generated continuesly
	fplConsoleFormatOut("Playing sine wave with %u Hz, %u channels, %s\n", hardwareFormat.sampleRate, hardwareFormat.channels, audioFormatName);
	fplConsoleOut("Press any key to exit\n");
	fplConsoleWaitForCharInput();

	// Stop audio playback, shutdown the audio device and release any platform resources
	fplPlatformRelease();

	// We are done
	return 0;
}