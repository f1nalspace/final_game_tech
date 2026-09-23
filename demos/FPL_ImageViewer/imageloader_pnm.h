/*
Name:
	FPL_ImageViewer | PNM/PAM loader

Description:
	Reads binary PGM (P5), PPM (P6) and PAM (P7) with 8 or 16 bit samples, gray or RGB, with or without alpha.
	A format stb_image is not asked for in this viewer, so this loader proves that a loader brings new formats into the folder scan.
	It also shows the renders of --render-to (PAM) directly in the viewer.

Usage:
	Include after imageloader.h. Define IMAGE_LOADER_PNM_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_PNM_H
#define IMAGE_LOADER_PNM_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_PNM_ID_TEXT "f66f6759-5f30-42bc-a046-0d758209a92a"

extern const ImageLoader *ImageLoaderPnmGet(void);

#endif // IMAGE_LOADER_PNM_H

#if defined(IMAGE_LOADER_PNM_IMPLEMENTATION) && !defined(IMAGE_LOADER_PNM_IMPLEMENTED)
#define IMAGE_LOADER_PNM_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const ImageLoaderPnmExtensions[] = { ".pgm", ".ppm", ".pam", ".pnm", NULL };
// Headers longer than this are rejected, a real one has well below 100 bytes
#define IMAGE_LOADER_PNM_MAX_HEADER_SIZE 4096
#define IMAGE_LOADER_PNM_MAX_TOKEN_LENGTH 32
// Samples per pixel at most: RGB + alpha
#define IMAGE_LOADER_PNM_MAX_DEPTH 4
// Keeps a corrupt header from asking for absurd amounts of memory (1 GB of RGBA)
static const uint64_t ImageLoaderPnmMaximumPixels = 1ull << 28;
static const uint32_t ImageLoaderPnmLargestByteSample = 255;
static const uint32_t ImageLoaderPnmLargestSample = 65535;
static const uint32_t ImageLoaderPnmOutputChannels = 4;
static const uint8_t ImageLoaderPnmOpaque = 255;

typedef enum ImageLoaderPnmKind {
	ImageLoaderPnmKind_None = 0,
	ImageLoaderPnmKind_Gray,
	ImageLoaderPnmKind_Pixmap,
	ImageLoaderPnmKind_Arbitrary,
} ImageLoaderPnmKind;

typedef struct ImageLoaderPnmHeader {
	uint32_t width;
	uint32_t height;
	// Samples per pixel: 1 gray, 2 gray + alpha, 3 RGB, 4 RGB + alpha
	uint32_t depth;
	uint32_t maximumValue;
	uint64_t dataOffset;
	const char *formatName;
} ImageLoaderPnmHeader;

typedef struct ImageLoaderPnmCursor {
	const uint8_t *data;
	size_t size;
	size_t position;
} ImageLoaderPnmCursor;

static bool ImageLoaderPnmIsSpace(const uint8_t c) {
	bool result = c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
	return(result);
}

// P5, P6 or P7 followed by whitespace
static ImageLoaderPnmKind ImageLoaderPnmDetect(const uint8_t *header, const size_t headerSize) {
	const size_t magicSize = 3;
	if (headerSize < magicSize || header[0] != 'P' || !ImageLoaderPnmIsSpace(header[2])) {
		return(ImageLoaderPnmKind_None);
	}
	switch (header[1]) {
		case '5':
			return(ImageLoaderPnmKind_Gray);
		case '6':
			return(ImageLoaderPnmKind_Pixmap);
		case '7':
			return(ImageLoaderPnmKind_Arbitrary);
		default:
			return(ImageLoaderPnmKind_None);
	}
}

static void ImageLoaderPnmSkipSpaceAndComments(ImageLoaderPnmCursor *cursor) {
	while (cursor->position < cursor->size) {
		uint8_t c = cursor->data[cursor->position];
		if (c == '#') {
			while (cursor->position < cursor->size && cursor->data[cursor->position] != '\n') {
				++cursor->position;
			}
		} else if (ImageLoaderPnmIsSpace(c)) {
			++cursor->position;
		} else {
			break;
		}
	}
}

static bool ImageLoaderPnmReadNumber(ImageLoaderPnmCursor *cursor, uint32_t *outValue) {
	const uint32_t decimalBase = 10;
	const uint64_t largestValue = 0xFFFFFFFFull;
	ImageLoaderPnmSkipSpaceAndComments(cursor);
	uint64_t value = 0;
	size_t start = cursor->position;
	while (cursor->position < cursor->size && cursor->data[cursor->position] >= '0' && cursor->data[cursor->position] <= '9') {
		uint32_t digit = (uint32_t)(cursor->data[cursor->position] - '0');
		value = value * decimalBase + digit;
		if (value > largestValue) {
			return(false);
		}
		++cursor->position;
	}
	if (cursor->position == start) {
		return(false);
	}
	*outValue = (uint32_t)value;
	return(true);
}

static bool ImageLoaderPnmReadToken(ImageLoaderPnmCursor *cursor, char *token, const size_t tokenSize) {
	ImageLoaderPnmSkipSpaceAndComments(cursor);
	size_t length = 0;
	while (cursor->position < cursor->size && !ImageLoaderPnmIsSpace(cursor->data[cursor->position])) {
		if (length + 1 >= tokenSize) {
			return(false);
		}
		token[length++] = (char)cursor->data[cursor->position++];
	}
	token[length] = 0;
	return(length > 0);
}

static void ImageLoaderPnmSkipLine(ImageLoaderPnmCursor *cursor) {
	while (cursor->position < cursor->size && cursor->data[cursor->position] != '\n') {
		++cursor->position;
	}
	if (cursor->position < cursor->size) {
		++cursor->position;
	}
}

static ImageLoadResult ImageLoaderPnmParseHeader(ImageSource *source, ImageLoaderPnmHeader *outHeader, char *message, const size_t messageSize) {
	uint8_t buffer[IMAGE_LOADER_PNM_MAX_HEADER_SIZE];
	source->seek(source, 0);
	size_t bufferSize = source->read(source, buffer, sizeof(buffer));
	ImageLoaderPnmKind kind = ImageLoaderPnmDetect(buffer, bufferSize);
	if (kind == ImageLoaderPnmKind_None) {
		snprintf(message, messageSize, "no binary PGM, PPM or PAM signature");
		return(ImageLoadResult_Unsupported);
	}
	const size_t magicLength = 2;
	ImageLoaderPnmCursor cursor = { buffer, bufferSize, magicLength };
	ImageLoaderPnmHeader header;
	memset(&header, 0, sizeof(header));

	if (kind == ImageLoaderPnmKind_Arbitrary) {
		const uint32_t missingDepth = 0;
		header.formatName = "PAM";
		header.depth = missingDepth;
		bool hasEnd = false;
		char token[IMAGE_LOADER_PNM_MAX_TOKEN_LENGTH];
		while (!hasEnd && ImageLoaderPnmReadToken(&cursor, token, sizeof(token))) {
			bool isValid = true;
			if (strcmp(token, "ENDHDR") == 0) {
				ImageLoaderPnmSkipLine(&cursor);
				hasEnd = true;
			} else if (strcmp(token, "WIDTH") == 0) {
				isValid = ImageLoaderPnmReadNumber(&cursor, &header.width);
			} else if (strcmp(token, "HEIGHT") == 0) {
				isValid = ImageLoaderPnmReadNumber(&cursor, &header.height);
			} else if (strcmp(token, "DEPTH") == 0) {
				isValid = ImageLoaderPnmReadNumber(&cursor, &header.depth);
			} else if (strcmp(token, "MAXVAL") == 0) {
				isValid = ImageLoaderPnmReadNumber(&cursor, &header.maximumValue);
			} else {
				// TUPLTYPE and anything unknown: the depth says enough
				ImageLoaderPnmSkipLine(&cursor);
			}
			if (!isValid) {
				snprintf(message, messageSize, "bad value for %s", token);
				return(ImageLoadResult_Corrupt);
			}
		}
		if (!hasEnd) {
			snprintf(message, messageSize, "PAM header without ENDHDR");
			return(ImageLoadResult_Corrupt);
		}
	} else {
		const uint32_t grayDepth = 1;
		const uint32_t pixmapDepth = 3;
		bool isGray = kind == ImageLoaderPnmKind_Gray;
		header.formatName = isGray ? "PGM" : "PPM";
		header.depth = isGray ? grayDepth : pixmapDepth;
		bool hasNumbers = ImageLoaderPnmReadNumber(&cursor, &header.width) && ImageLoaderPnmReadNumber(&cursor, &header.height) && ImageLoaderPnmReadNumber(&cursor, &header.maximumValue);
		// Exactly one whitespace character separates the header from the samples
		if (!hasNumbers || cursor.position >= cursor.size || !ImageLoaderPnmIsSpace(cursor.data[cursor.position])) {
			snprintf(message, messageSize, "bad PNM header");
			return(ImageLoadResult_Corrupt);
		}
		++cursor.position;
	}

	if (header.width == 0 || header.height == 0 || header.depth == 0 || header.depth > IMAGE_LOADER_PNM_MAX_DEPTH || header.maximumValue == 0 || header.maximumValue > ImageLoaderPnmLargestSample) {
		snprintf(message, messageSize, "unsupported size, depth or sample range");
		return(ImageLoadResult_Unsupported);
	}
	uint64_t pixelCount = (uint64_t)header.width * header.height;
	if (pixelCount > ImageLoaderPnmMaximumPixels) {
		snprintf(message, messageSize, "picture too large");
		return(ImageLoadResult_Unsupported);
	}
	header.dataOffset = cursor.position;
	uint64_t bytesPerSample = header.maximumValue > ImageLoaderPnmLargestByteSample ? 2 : 1;
	uint64_t dataSize = pixelCount * header.depth * bytesPerSample;
	uint64_t fileSize = source->size(source);
	if (header.dataOffset + dataSize > fileSize) {
		snprintf(message, messageSize, "truncated, %llu of %llu sample bytes", (unsigned long long)(fileSize - header.dataOffset), (unsigned long long)dataSize);
		return(ImageLoadResult_Corrupt);
	}
	*outHeader = header;
	return(ImageLoadResult_Success);
}

static ImageLoaderMatch ImageLoaderPnmProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	ImageLoaderPnmKind kind = ImageLoaderPnmDetect(header, headerSize);
	if (kind != ImageLoaderPnmKind_None) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderPnmReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	const uint32_t byteSampleBits = 8;
	const uint32_t wideSampleBits = 16;
	ImageLoaderPnmHeader header;
	ImageLoadResult result = ImageLoaderPnmParseHeader(source, &header, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	uint32_t sampleBits = header.maximumValue > ImageLoaderPnmLargestByteSample ? wideSampleBits : byteSampleBits;
	outInfo->width = header.width;
	outInfo->height = header.height;
	outInfo->channelCount = header.depth;
	outInfo->bitsPerPixel = header.depth * sampleBits;
	outInfo->formatName = header.formatName;
	outInfo->orientation = ImageOrientation_Normal;
	outInfo->isPalette = false;
	return(ImageLoadResult_Success);
}

// Sample scaled to 0..255 with rounding
static uint8_t ImageLoaderPnmScale(const uint32_t value, const uint32_t maximumValue) {
	uint32_t clamped = value > maximumValue ? maximumValue : value;
	uint32_t result = (clamped * ImageLoaderPnmLargestByteSample + maximumValue / 2) / maximumValue;
	return((uint8_t)result);
}

static ImageLoadResult ImageLoaderPnmDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	const int byteBits = 8;
	const uint32_t grayAlphaDepth = 2;
	const uint32_t rgbDepth = 3;
	ImageLoaderPnmHeader header;
	ImageLoadResult result = ImageLoaderPnmParseHeader(source, &header, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	bool isWide = header.maximumValue > ImageLoaderPnmLargestByteSample;
	size_t bytesPerSample = isWide ? 2 : 1;
	size_t rowSize = (size_t)header.width * header.depth * bytesPerSample;
	size_t outputStride = (size_t)header.width * ImageLoaderPnmOutputChannels;
	uint8_t *pixels = (uint8_t *)malloc(outputStride * header.height);
	uint8_t *row = (uint8_t *)malloc(rowSize);
	if (pixels == NULL || row == NULL) {
		free(pixels);
		free(row);
		snprintf(message, messageSize, "out of memory");
		return(ImageLoadResult_OutOfMemory);
	}
	source->seek(source, header.dataOffset);
	for (uint32_t y = 0; y < header.height; ++y) {
		if (source->read(source, row, rowSize) != rowSize) {
			free(pixels);
			free(row);
			bool isCanceled = source->isCanceled(source);
			snprintf(message, messageSize, isCanceled ? "canceled" : "truncated");
			return(isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt);
		}
		uint8_t *target = pixels + (size_t)y * outputStride;
		for (uint32_t x = 0; x < header.width; ++x) {
			uint8_t samples[IMAGE_LOADER_PNM_MAX_DEPTH];
			for (uint32_t sampleIndex = 0; sampleIndex < header.depth; ++sampleIndex) {
				size_t offset = ((size_t)x * header.depth + sampleIndex) * bytesPerSample;
				// 16 bit samples are big endian
				uint32_t value = isWide ? (((uint32_t)row[offset] << byteBits) | row[offset + 1]) : row[offset];
				samples[sampleIndex] = ImageLoaderPnmScale(value, header.maximumValue);
			}
			uint8_t *pixel = target + (size_t)x * ImageLoaderPnmOutputChannels;
			bool isGray = header.depth < rgbDepth;
			pixel[0] = samples[0];
			pixel[1] = isGray ? samples[0] : samples[1];
			pixel[2] = isGray ? samples[0] : samples[2];
			if (header.depth == grayAlphaDepth) {
				pixel[3] = samples[1];
			} else if (header.depth > rgbDepth) {
				pixel[3] = samples[3];
			} else {
				pixel[3] = ImageLoaderPnmOpaque;
			}
		}
	}
	free(row);
	outPixels->pixels = pixels;
	outPixels->width = header.width;
	outPixels->height = header.height;
	outPixels->stride = (uint32_t)outputStride;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static void ImageLoaderPnmRelease(const ImageLoader *loader, ImagePixels *pixels) {
	free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderPnmGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		// f66f6759-5f30-42bc-a046-0d758209a92a
		const ImageLoaderId id = { { 0xf6, 0x6f, 0x67, 0x59, 0x5f, 0x30, 0x42, 0xbc, 0xa0, 0x46, 0x0d, 0x75, 0x82, 0x09, 0xa9, 0x2a } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "pnm";
		loader.version = "1.0";
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderPnmExtensions;
		loader.probe = ImageLoaderPnmProbe;
		loader.readInfo = ImageLoaderPnmReadInfo;
		loader.decode = ImageLoaderPnmDecode;
		loader.releasePixels = ImageLoaderPnmRelease;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_PNM_IMPLEMENTATION
