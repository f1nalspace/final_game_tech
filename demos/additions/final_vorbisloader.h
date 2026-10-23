/*
Name:
	Final Wave Loader

Description:
	Simple limited wave loader

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
	## 2026-10-22
	- New: The loop region from the comments LOOPSTART with LOOPLENGTH or LOOPEND (PCMWaveFormat.loopStartFrame and loopEndFrame)

	## 2026-10-21
	- Fixed: Stereo files loaded only half their length
*/

#ifndef FINAL_VORBISLOADER_H
#define FINAL_VORBISLOADER_H

#include <final_platform_layer.h>

#include "final_audio.h"

extern bool TestVorbisHeader(const uint8_t *buffer, const size_t bufferSize);
extern bool LoadVorbisFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave);
extern bool LoadVorbisFromFile(const char *filePath, PCMWaveData *outWave);

extern bool LoadVorbisFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat);

#endif // FINAL_VORBISLOADER_H

#if defined(FINAL_VORBISLOADER_IMPLEMENTATION) && !defined(FINAL_VORBISLOADER_IMPLEMENTED)
#define FINAL_VORBISLOADER_IMPLEMENTED

#include <stb/stb_vorbis.c>

#define OGG_MAGIC FOURCC32('O', 'g', 'g', 'S')

#pragma pack(push, 1)
typedef struct OggFileHeader {
	uint32 capturePattern;
} OggFileHeader;
#pragma pack(pop)

// Names of the comments with the loop region, the values count samples per channel (frames). LOOPEND is the frame after the last one of the region
static const char *VorbisLoopStartFieldName = "LOOPSTART";
static const char *VorbisLoopLengthFieldName = "LOOPLENGTH";
static const char *VorbisLoopEndFieldName = "LOOPEND";

typedef struct VorbisLoopField {
	uint32_t value;
	bool isFound;
	bool isNumber;
} VorbisLoopField;

static char GetUpperAsciiChar(const char value) {
	const bool isLower = (value >= 'a') && (value <= 'z');
	const char result = isLower ? (char)(value - 'a' + 'A') : value;
	return(result);
}

// The value of a comment "<field>=<value>" when its field is the given upper case name, the case of field names does not matter
static const char *GetVorbisCommentValue(const char *comment, const char *upperFieldName) {
	size_t charIndex = 0;
	while(upperFieldName[charIndex] != 0) {
		const char commentChar = GetUpperAsciiChar(comment[charIndex]);
		if(commentChar != upperFieldName[charIndex]) {
			return(fpl_null);
		}
		++charIndex;
	}
	if(comment[charIndex] != '=') {
		return(fpl_null);
	}
	const size_t valueIndex = charIndex + 1;
	const char *result = comment + valueIndex;
	return(result);
}

// A frame index of decimal digits only, false when the text is empty, has any other char or does not fit in 32 bits
static bool ParseVorbisCommentFrameIndex(const char *text, uint32_t *outValue) {
	const uint64_t decimalBase = 10;
	uint64_t value = 0;
	size_t charIndex = 0;
	while(text[charIndex] != 0) {
		const char digitChar = text[charIndex];
		const bool isDigit = (digitChar >= '0') && (digitChar <= '9');
		if(!isDigit) {
			return(false);
		}
		const uint64_t digit = (uint64_t)(digitChar - '0');
		value = value * decimalBase + digit;
		if(value > UINT32_MAX) {
			return(false);
		}
		++charIndex;
	}
	if(charIndex == 0) {
		return(false);
	}
	*outValue = (uint32_t)value;
	return(true);
}

// Reads the field from the comment, only the first comment of a field counts
static void ReadVorbisLoopField(const char *comment, const char *upperFieldName, VorbisLoopField *field) {
	if(field->isFound) {
		return;
	}
	const char *value = GetVorbisCommentValue(comment, upperFieldName);
	if(value == fpl_null) {
		return;
	}
	field->isFound = true;
	field->isNumber = ParseVorbisCommentFrameIndex(value, &field->value);
}

// Reads the loop region from the comments LOOPSTART with LOOPLENGTH or LOOPEND, LOOPSTART alone loops up to the end of the file. Both are 0 without LOOPSTART, when a value is no number or when the region does not fit in the frame count
static void ReadVorbisLoopFromComments(const stb_vorbis_comment *comments, const uint32_t frameCount, uint32_t *outLoopStartFrame, uint32_t *outLoopEndFrame) {
	*outLoopStartFrame = 0;
	*outLoopEndFrame = 0;

	VorbisLoopField startField = fplZeroInit;
	VorbisLoopField lengthField = fplZeroInit;
	VorbisLoopField endField = fplZeroInit;
	for(int commentIndex = 0; commentIndex < comments->comment_list_length; ++commentIndex) {
		const char *comment = comments->comment_list[commentIndex];
		if(comment == fpl_null) {
			continue;
		}
		ReadVorbisLoopField(comment, VorbisLoopStartFieldName, &startField);
		ReadVorbisLoopField(comment, VorbisLoopLengthFieldName, &lengthField);
		ReadVorbisLoopField(comment, VorbisLoopEndFieldName, &endField);
	}
	if(!startField.isFound || !startField.isNumber) {
		return;
	}

	// 64-bit, start plus length may exceed 32 bits
	uint64_t loopEndFrame = frameCount;
	if(lengthField.isFound) {
		if(!lengthField.isNumber) {
			return;
		}
		loopEndFrame = (uint64_t)startField.value + lengthField.value;
	} else if(endField.isFound) {
		if(!endField.isNumber) {
			return;
		}
		loopEndFrame = endField.value;
	}
	const bool isInsideFile = (startField.value < loopEndFrame) && (loopEndFrame <= frameCount);
	if(isInsideFile) {
		*outLoopStartFrame = startField.value;
		*outLoopEndFrame = (uint32_t)loopEndFrame;
	}
}

// The loop region from the comments of an open decoder, see ReadVorbisLoopFromComments()
static void GetVorbisLoopFromDecoder(stb_vorbis *vorbis, const uint32_t frameCount, uint32_t *outLoopStartFrame, uint32_t *outLoopEndFrame) {
	const stb_vorbis_comment comments = stb_vorbis_get_comment(vorbis);
	ReadVorbisLoopFromComments(&comments, frameCount, outLoopStartFrame, outLoopEndFrame);
}

// stb_vorbis_decode_memory() returns no comments, so a second decoder opens for them (only the headers). Both are 0 when it does not open
static void LoadVorbisLoopFromBuffer(const uint8_t *buffer, const size_t bufferSize, const uint32_t frameCount, uint32_t *outLoopStartFrame, uint32_t *outLoopEndFrame) {
	*outLoopStartFrame = 0;
	*outLoopEndFrame = 0;
	int openError = 0;
	stb_vorbis *vorbis = stb_vorbis_open_memory(buffer, (int)bufferSize, &openError, fpl_null);
	if(vorbis == fpl_null) {
		return;
	}
	GetVorbisLoopFromDecoder(vorbis, frameCount, outLoopStartFrame, outLoopEndFrame);
	stb_vorbis_close(vorbis);
}

extern bool TestVorbisHeader(const uint8_t *buffer, const size_t bufferSize) {
	if ((buffer == fpl_null) || (bufferSize == 0)) {
		return(false);
	}
	if (bufferSize < sizeof(OggFileHeader)) {
		return(false);
	}
	OggFileHeader *header = (OggFileHeader *)buffer;
	if (header->capturePattern != OGG_MAGIC) {
		return(false);
	}
	return(true);
}

extern bool LoadVorbisFormatFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveFormat *outFormat) {
	if((buffer == fpl_null) || (bufferSize == 0) || outFormat == fpl_null) {
		return(false);
	}

	int openErr = 0;
	stb_vorbis *vorbis = stb_vorbis_open_memory(buffer, (int)bufferSize, &openErr, fpl_null);
	if (vorbis == fpl_null) {
		return false;
	}

	// stb_vorbis counts samples per channel, so this is already the frame count
	uint32_t frameCount = stb_vorbis_stream_length_in_samples(vorbis);

	fplClearStruct(outFormat);

	if (frameCount > 0) {
		outFormat->formatType = fplAudioFormatType_S16;
		outFormat->bytesPerSample = fplGetAudioFrameSizeInBytes(outFormat->formatType, 1);
		outFormat->channelCount = (uint16_t)fplMax(0, vorbis->channels);
		outFormat->samplesPerSecond = (uint32_t)fplMax(0, vorbis->sample_rate);
		outFormat->frameCount = frameCount;
		GetVorbisLoopFromDecoder(vorbis, frameCount, &outFormat->loopStartFrame, &outFormat->loopEndFrame);
	}

	stb_vorbis_close(vorbis);

	return true;
}

extern bool LoadVorbisFromBuffer(const uint8_t *buffer, const size_t bufferSize, PCMWaveData *outWave) {
	if((buffer == fpl_null) || (bufferSize == 0) || outWave == fpl_null) {
		return(false);
	}

	int channels = 0;
	int sampleRate = 0;
	short *output = fpl_null;
	// stb_vorbis counts samples per channel, so this is already the frame count
	int frameCount = stb_vorbis_decode_memory(buffer, (int)bufferSize, &channels, &sampleRate, &output);
	if (frameCount <= 0) {
		return(false);
	}

	outWave->format.formatType = fplAudioFormatType_S16;
	outWave->format.bytesPerSample = fplGetAudioFrameSizeInBytes(outWave->format.formatType, 1);
	outWave->format.samplesPerSecond = (uint32_t)fplMax(0, sampleRate);
	outWave->format.channelCount = (uint16_t)fplMax(0, channels);
	outWave->format.frameCount = (uint32_t)frameCount;
	LoadVorbisLoopFromBuffer(buffer, bufferSize, outWave->format.frameCount, &outWave->format.loopStartFrame, &outWave->format.loopEndFrame);

	size_t sampleMemorySize = outWave->format.bytesPerSample * outWave->format.channelCount * outWave->format.frameCount;
	outWave->samplesSize = sampleMemorySize;
	outWave->isamples = (uint8_t *)fplMemoryAllocate(sampleMemorySize);
	fplMemoryCopy(output, sampleMemorySize, outWave->isamples);
	outWave->isValid = true;

	free(output);

	return(true);
}

extern bool LoadVorbisFromFile(const char *filePath, PCMWaveData *outWave) {
	bool result = false;
	fplFileHandle file;
	if(fplFileOpenBinary(filePath, &file)) {
		size_t length = fplFileGetSizeFromHandle32(&file);
		uint8_t *contents = (uint8_t *)fplMemoryAllocate(length);
		if(contents != fpl_null) {
			if(fplFileReadBlock32(&file, (uint32_t)length, contents, (uint32_t)length) == length) {
				result = LoadVorbisFromBuffer(contents, length, outWave);
			}
			fplMemoryFree(contents);
		}
		fplFileClose(&file);
	}
	return(result);
}

#endif // FINAL_VORBISLOADER_IMPLEMENTATION