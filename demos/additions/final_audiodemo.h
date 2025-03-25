#ifndef FINAL_AUDIODEMO_H
#define FINAL_AUDIODEMO_H

#include "../../final_platform_layer.h"

#include "final_audio.h"

#include "final_audiosystem.h"

#include "final_buffer.h"

typedef enum AudioTrackState {
	//! The audio track is invalid
	AudioTrackState_Failed = -1,

	//! The audio track is unloaded
	AudioTrackState_Unloaded = 0,

	//! Aquire the loading of the audio source
	AudioTrackState_AquireLoading,

	//! Actual loading of audio source
	AudioTrackState_Loading,

	//! The audio track is ready for streaming/playing
	AudioTrackState_Ready,

	//! The audio track is fully loaded
	AudioTrackState_Full,
} AudioTrackState;

typedef enum AudioTrackSourceType {
	AudioTrackSourceType_None = 0,
	AudioTrackSourceType_URL,
	AudioTrackSourceType_Data,
} AudioTrackSourceType;

typedef struct AudioTrackURLSource {
	char urlOrFilePath[1024];
} AudioTrackURLSource;

typedef struct AudioTrackDataSource {
	const uint8_t *data;
	size_t size;
} AudioTrackDataSource;

typedef struct AudioTrackSource {
	char name[256];
	AudioTrackSourceType type;
	union {
		AudioTrackDataSource data;
		AudioTrackURLSource url;
	};
} AudioTrackSource;

typedef struct AudioTrack {
	AudioTrackSource source;		// The source infos (file/url or data)
	char name[256];					// Name of the audio track
	AudioBuffer outputFullBuffer;	// Entire samples of the track, to have smoother spectrum visualization
	AudioSourceID sourceID;			// The audio source ID
	AudioPlayItemID playID;			// The play item ID
	volatile int32_t state;			// The AudioTrackState
	int loadingPercentage;			// The loading progress in range of 0-100
} AudioTrack;

#define MAX_AUDIO_TRACK_LIST_COUNT 8
typedef struct AudioTrackList {
	AudioTrack tracks[MAX_AUDIO_TRACK_LIST_COUNT];
	uint32_t count;
	int32_t currentIndex;
	int32_t lastIndex;
	fpl_b32 changedPending;
} AudioTrackList;

static bool HasAudioTrack(AudioTrackList *tracklist) {
	if(tracklist->changedPending)
		return(false);
	if (!(tracklist->currentIndex >= 0 && tracklist->currentIndex < (int32_t)tracklist->count))
		return(false);
	AudioTrackState state = (AudioTrackState)fplAtomicLoadS32(&tracklist->tracks[tracklist->currentIndex].state);
	bool result = (state == AudioTrackState_Full) || (state == AudioTrackState_Ready);
	return(result);
}

static void StopAllAudioTracks(AudioSystem *audioSys, AudioTrackList *tracklist) {
	AudioSystemStopAll(audioSys);
	tracklist->currentIndex = -1;
	tracklist->lastIndex = -1;
	tracklist->changedPending = false;
}

static bool PlayAudioTrack(AudioSystem *audioSys, AudioTrackList *tracklist, const uint32_t index) {
	if(index < tracklist->count) {
		AudioTrack *track = tracklist->tracks + index;
		AudioTrackState state = (AudioTrackState)fplAtomicLoadS32(&track->state);
		if(state == AudioTrackState_Unloaded) {
			// Audio file is loaded asynchronously
			StopAllAudioTracks(audioSys, tracklist);
			fplAssert(track->source.type != AudioTrackSourceType_None);
			fplAssert(track->sourceID.value == 0);
			fplAssert(track->playID.value == 0);
			tracklist->changedPending = true;
			tracklist->currentIndex = tracklist->lastIndex = index;
			fplAtomicStoreS32(&track->state, AudioTrackState_AquireLoading);
			return(true);
		} else if(state == AudioTrackState_Full || state == AudioTrackState_Ready) {
			// Audio file/stream is already loaded
			StopAllAudioTracks(audioSys, tracklist);
			fplAssert(track->sourceID.value > 0);
			AudioSource *source = AudioSystemGetSourceByID(audioSys, track->sourceID);
			if(source != fpl_null) {
				track->playID = AudioSystemPlaySource(audioSys, source, false, 1.0f);

				// We either have never initialized the full buffer or the buffer 
				bool isSameBuffer = AreAudioBuffersEqual(&track->outputFullBuffer, &source->buffer);
				fplAssert((track->outputFullBuffer.bufferSize == 0) || isSameBuffer);

				tracklist->currentIndex = tracklist->lastIndex = index;
			} else {
				// Source not available anymore?
				fplAlwaysAssert(!"Lost audio source???");
			}
		} else {
			// Invalid state
			FPL_NOT_IMPLEMENTED;
		}
		tracklist->changedPending = true;
		return(true);
	}
	return(false);
}

typedef enum LoadAudioTrackFlags {
	LoadAudioTrackFlags_None = 0,
	LoadAudioTrackFlags_AutoLoad = 1 << 0,
	LoadAudioTrackFlags_AutoPlay = 1 << 1,
} LoadAudioTrackFlags;

static bool LoadAudioTrackList(AudioSystem *audioSys, const AudioTrackSource *sources, const size_t sourceCount, const bool forceSineWave, const AudioSineWaveData *sineWave, const LoadAudioTrackFlags flags, AudioTrackList *tracklist) {
	// Stop any audio tracks
	StopAllAudioTracks(audioSys, tracklist);

	// @TODO(final): Remove all audio sources
	fplAssert(audioSys->sources.count == 0);

	bool autoLoad = flags & LoadAudioTrackFlags_AutoLoad;
	bool autoPlay = flags & LoadAudioTrackFlags_AutoPlay;

	// Add to track list (Optionally start playing)
	uint32_t maxTrackCount = fplArrayCount(tracklist->tracks);

	tracklist->count = 0;

	bool hadSources = false;
	for(size_t sourceIndex = 0; sourceIndex < sourceCount; ++sourceIndex) {
		const AudioTrackSource *trackSource = &sources[sourceIndex];
		if(trackSource != fpl_null && trackSource->type != AudioTrackSourceType_None) {
			if(tracklist->count < maxTrackCount) {
				uint32_t trackIndex = tracklist->count++;
				AudioTrack *track = &tracklist->tracks[trackIndex];
				fplClearStruct(track);

				track->source.type = trackSource->type;

				fplCopyString(trackSource->name, track->name, fplArrayCount(track->name));

				switch (trackSource->type) {
					case AudioTrackSourceType_URL:
						fplCopyString(trackSource->url.urlOrFilePath, track->source.url.urlOrFilePath, fplArrayCount(track->source.url.urlOrFilePath));
						break;
					case AudioTrackSourceType_Data:
						track->source.data.data = trackSource->data.data;
						track->source.data.size = trackSource->data.size;
						break;
					default:
						FPL_NOT_IMPLEMENTED;
				}

				if(autoLoad) {
					fplConsoleFormatOut("Loading audio track '%s\n", trackSource->name);

					AudioSource *source = fpl_null;				
					
					switch (trackSource->type) {
						case AudioTrackSourceType_URL:
							source = AudioSystemLoadFileSource(audioSys, trackSource->url.urlOrFilePath);
							break;
						case AudioTrackSourceType_Data:
							source = AudioSystemLoadDataSource(audioSys, trackSource->data.size, trackSource->data.data);
							break;
						default:
							FPL_NOT_IMPLEMENTED;
					}

					if (source == fpl_null) {
						fplConsoleFormatError("Can't load audio source '%s'!\n", trackSource->name);
						continue;
					}

					if (!AudioSystemAddSource(audioSys, source)) {
						fplConsoleFormatError("Failed to add audio track '%s' with source id '%zu'!\n", trackSource->name, source->id.value);
						continue;
					}

					track->sourceID = source->id;
					if(autoPlay) {
						AudioPlayItemID playID = AudioSystemPlaySource(audioSys, source, false, 1.0f);
						track->playID = playID;
					}
				}

				hadSources = true;
			} else {
				fplConsoleFormatError("Track capacity of '%lu' reached! Cannot play audio track '%s'!\n", tracklist->count, trackSource->name);
				break;
			}
		}
	}

	// Generate sine wave for some duration when no files was loaded
	if(!hadSources || forceSineWave) {
		if(tracklist->count < maxTrackCount) {
			AudioSineWaveData waveData = *sineWave;
			AudioHertz sampleRate = audioSys->targetFormat.sampleRate;
			AudioChannelIndex channels = audioSys->targetFormat.channels;
			AudioFrameIndex frameCount = (AudioFrameIndex)(sampleRate * waveData.duration + 0.5);
			AudioSource *source = AudioSystemAllocateSource(audioSys, audioSys->targetFormat.channels, audioSys->targetFormat.sampleRate, fplAudioFormatType_S16, frameCount);
			if(source != fpl_null) {
				fplAssert(source->type == AudioSourceType_Allocated);

				if (!AudioSystemAddSource(audioSys, source)) {
					fplConsoleFormatError("Failed to add sine wave audio source id '%zu'!\n", source->id.value);
					return false;
				}

				// Generate sine wave
				source->type = AudioSourceType_Stream;
				AudioGenerateSineWave(&waveData, source->buffer.samples, source->format.format, source->format.sampleRate, source->format.channels, source->buffer.frameCount);

				uint32_t trackIndex = tracklist->count++;
				AudioTrack *track = &tracklist->tracks[trackIndex];
				track->sourceID = source->id;
				track->state = AudioTrackState_Full;

				if(autoPlay) {
					AudioPlayItemID playID = AudioSystemPlaySource(audioSys, source, true, 1.0f);
					track->playID = playID;
				}
			}
		} else {
			fplConsoleFormatError("Track capacity of '%lu' reached! Cannot add sine wave.\n", tracklist->count);
		}
	}

	return(true);
}

#endif // FINAL_AUDIODEMO_H