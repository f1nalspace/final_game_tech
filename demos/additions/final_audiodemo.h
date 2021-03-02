#ifndef FINAL_AUDIODEMO_H
#define FINAL_AUDIODEMO_H

#include "../../final_platform_layer.h"
#include "final_audio.h"
#include "final_audiosystem.h"

static bool InitAudioData(const fplAudioDeviceFormat *targetFormat, AudioSystem *audioSys, const char **files, const size_t fileCount, const bool forceSineWave, const AudioSineWaveData *sineWave) {
	if (!AudioSystemInit(audioSys, targetFormat)) {
		return false;
	}

	// Default volume
	AudioSystemSetMasterVolume(audioSys, 0.5f);

	// Play audio files
	bool hadFiles = false;
	for (size_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		const char *filePath = files[fileIndex];
		if (filePath != fpl_null) {
			fplConsoleFormatOut("Loading audio file '%s\n", filePath);
			AudioSource *source = AudioSystemLoadFileSource(audioSys, filePath);
			if (source != fpl_null) {
				AudioSystemPlaySource(audioSys, source, true, 1.0f);
				hadFiles = true;
			}
		}
	}

	// Generate sine wave for some duration when no files was loaded
	if (!hadFiles || forceSineWave) {
		AudioSineWaveData waveData = *sineWave;
		AudioHertz sampleRate = audioSys->targetFormat.sampleRate;
		AudioChannelIndex channels = audioSys->targetFormat.channels;
		AudioFrameIndex frameCount = (AudioFrameIndex)(sampleRate * waveData.duration + 0.5);
		AudioSource *source = AudioSystemAllocateSource(audioSys, audioSys->targetFormat.channels, audioSys->targetFormat.sampleRate, fplAudioFormatType_S16, frameCount);
		if (source != fpl_null) {
			AudioGenerateSineWave(&waveData, source->buffer.samples, source->format.format, source->format.sampleRate, source->format.channels, source->buffer.frameCount);
			AudioSystemPlaySource(audioSys, source, true, 1.0f);
		}
	}
	return(true);
}

#endif // FINAL_AUDIODEMO_H