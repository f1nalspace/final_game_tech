/*
Name:
	FPL_ImageViewer | WebP loader

Description:
	Reads lossy and lossless WebP files, with or without alpha, through the libwebp of the system.
	libwebp is loaded at runtime when the loader is first requested, so the viewer builds and runs without it: the loader stays registered,
	reads the picture info from the file header itself and reports every decode as unsupported, with the missing library as the reason.
	Animated WebP files are reported as unsupported as well, the simple decoding API of libwebp does not read them.

Usage:
	Include after imageloader.h. Define IMAGE_LOADER_WEBP_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_WEBP_H
#define IMAGE_LOADER_WEBP_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_WEBP_ID_TEXT "19aa1ca9-5c5b-427f-90ab-b85fc4a8c377"

// The loader, filled on the first call, which also tries to load libwebp
extern const ImageLoader *ImageLoaderWebpGet(void);

#endif // IMAGE_LOADER_WEBP_H

#if defined(IMAGE_LOADER_WEBP_IMPLEMENTATION) && !defined(IMAGE_LOADER_WEBP_IMPLEMENTED)
#define IMAGE_LOADER_WEBP_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const ImageLoaderWebpExtensions[] = { ".webp", NULL };

// The library names tried in this order, the versioned one first
#if defined(FPL_PLATFORM_WINDOWS)
static const char *const ImageLoaderWebpLibraryNames[] = { "libwebp.dll", "webp.dll", NULL };
#else
static const char *const ImageLoaderWebpLibraryNames[] = { "libwebp.so.7", "libwebp.so", NULL };
#endif

// RIFF container: "RIFF", the size, "WEBP", then the first chunk with its FourCC and size
static const uint8_t ImageLoaderWebpRiffSignature[] = { 'R', 'I', 'F', 'F' };
static const uint8_t ImageLoaderWebpFormSignature[] = { 'W', 'E', 'B', 'P' };
static const uint8_t ImageLoaderWebpChunkLossy[] = { 'V', 'P', '8', ' ' };
static const uint8_t ImageLoaderWebpChunkLossless[] = { 'V', 'P', '8', 'L' };
static const uint8_t ImageLoaderWebpChunkExtended[] = { 'V', 'P', '8', 'X' };
static const size_t ImageLoaderWebpFormOffset = 8;
static const size_t ImageLoaderWebpChunkOffset = 12;
static const size_t ImageLoaderWebpPayloadOffset = 20;
// VP8X: flags, three reserved bytes, then canvas width - 1 and height - 1 as 24 bit values
static const uint8_t ImageLoaderWebpExtendedFlagAnimation = 0x02;
static const uint8_t ImageLoaderWebpExtendedFlagAlpha = 0x10;
static const size_t ImageLoaderWebpExtendedWidthOffset = 24;
static const size_t ImageLoaderWebpExtendedHeightOffset = 27;
static const size_t ImageLoaderWebpExtendedHeaderEnd = 30;
// VP8L: signature byte, then 14 bits width - 1, 14 bits height - 1 and the alpha bit
static const uint8_t ImageLoaderWebpLosslessSignature = 0x2F;
static const size_t ImageLoaderWebpLosslessBitsOffset = 21;
static const size_t ImageLoaderWebpLosslessHeaderEnd = 25;
static const uint32_t ImageLoaderWebpLosslessSizeBits = 14;
static const uint32_t ImageLoaderWebpLosslessSizeMask = 0x3FFF;
static const uint32_t ImageLoaderWebpLosslessAlphaBit = 28;
// VP8 key frame: three bytes frame tag, the start code 9D 01 2A, then 14 bits width and 14 bits height, each with two scale bits on top
static const uint8_t ImageLoaderWebpLossyStartCode[] = { 0x9D, 0x01, 0x2A };
static const size_t ImageLoaderWebpLossyStartCodeOffset = 23;
static const size_t ImageLoaderWebpLossyWidthOffset = 26;
static const size_t ImageLoaderWebpLossyHeightOffset = 28;
static const size_t ImageLoaderWebpLossyHeaderEnd = 30;
static const uint32_t ImageLoaderWebpLossySizeMask = 0x3FFF;

static const uint32_t ImageLoaderWebpRgbChannels = 3;
static const uint32_t ImageLoaderWebpRgbaChannels = 4;
static const uint32_t ImageLoaderWebpBitsPerChannel = 8;
// Reading is this share of the progress, decoding the rest without a progress of its own
static const float ImageLoaderWebpReadShare = 0.5f;
static const size_t ImageLoaderWebpReadChunkSize = 1024 * 1024;

// The functions of libwebp the loader uses, declared here so no libwebp header is needed
typedef int ImageLoaderWebpGetDecoderVersionFunction(void);
typedef uint8_t *ImageLoaderWebpDecodeRgbaFunction(const uint8_t *data, size_t dataSize, int *width, int *height);
typedef void ImageLoaderWebpFreeFunction(void *ptr);

typedef struct ImageLoaderWebpLibrary {
	fplDynamicLibraryHandle handle;
	ImageLoaderWebpGetDecoderVersionFunction *getDecoderVersion;
	ImageLoaderWebpDecodeRgbaFunction *decodeRgba;
	ImageLoaderWebpFreeFunction *free;
	// The library file that was loaded, or the first name when none was found
	const char *libraryName;
	// "1.6.0" from WebPGetDecoderVersion, "missing" without the library
	char versionText[64];
	bool isLoaded;
} ImageLoaderWebpLibrary;

// Loaded once in ImageLoaderWebpGet() and kept until the process ends, the decode functions of libwebp are thread-safe
static ImageLoaderWebpLibrary imageLoaderWebp__library;

typedef struct ImageLoaderWebpHeader {
	uint32_t width;
	uint32_t height;
	const char *formatName;
	bool hasAlpha;
	bool isAnimated;
} ImageLoaderWebpHeader;

static bool ImageLoaderWebpMatchesAt(const uint8_t *header, const size_t headerSize, const size_t offset, const uint8_t *bytes, const size_t byteCount) {
	bool result = headerSize >= offset + byteCount && memcmp(header + offset, bytes, byteCount) == 0;
	return(result);
}

static bool ImageLoaderWebpIsSignature(const uint8_t *header, const size_t headerSize) {
	bool isRiff = ImageLoaderWebpMatchesAt(header, headerSize, 0, ImageLoaderWebpRiffSignature, sizeof(ImageLoaderWebpRiffSignature));
	bool isWebp = ImageLoaderWebpMatchesAt(header, headerSize, ImageLoaderWebpFormOffset, ImageLoaderWebpFormSignature, sizeof(ImageLoaderWebpFormSignature));
	bool result = isRiff && isWebp;
	return(result);
}

static uint32_t ImageLoaderWebpReadLe16(const uint8_t *bytes) {
	const uint32_t byteBits = 8;
	uint32_t result = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << byteBits);
	return(result);
}

static uint32_t ImageLoaderWebpReadLe24(const uint8_t *bytes) {
	const uint32_t byteBits = 8;
	const uint32_t twoByteBits = 16;
	uint32_t result = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << byteBits) | ((uint32_t)bytes[2] << twoByteBits);
	return(result);
}

static uint32_t ImageLoaderWebpReadLe32(const uint8_t *bytes) {
	const uint32_t twoByteBits = 16;
	const size_t highHalfOffset = 2;
	uint32_t low = ImageLoaderWebpReadLe16(bytes);
	uint32_t high = ImageLoaderWebpReadLe16(bytes + highHalfOffset);
	uint32_t result = low | (high << twoByteBits);
	return(result);
}

// Size, alpha and animation from the first chunk, without libwebp
static bool ImageLoaderWebpParseHeader(const uint8_t *header, const size_t headerSize, ImageLoaderWebpHeader *outHeader) {
	if (!ImageLoaderWebpIsSignature(header, headerSize)) {
		return(false);
	}
	memset(outHeader, 0, sizeof(*outHeader));
	if (ImageLoaderWebpMatchesAt(header, headerSize, ImageLoaderWebpChunkOffset, ImageLoaderWebpChunkExtended, sizeof(ImageLoaderWebpChunkExtended))) {
		if (headerSize < ImageLoaderWebpExtendedHeaderEnd) {
			return(false);
		}
		uint8_t flags = header[ImageLoaderWebpPayloadOffset];
		outHeader->width = ImageLoaderWebpReadLe24(header + ImageLoaderWebpExtendedWidthOffset) + 1;
		outHeader->height = ImageLoaderWebpReadLe24(header + ImageLoaderWebpExtendedHeightOffset) + 1;
		outHeader->hasAlpha = (flags & ImageLoaderWebpExtendedFlagAlpha) != 0;
		outHeader->isAnimated = (flags & ImageLoaderWebpExtendedFlagAnimation) != 0;
		outHeader->formatName = outHeader->isAnimated ? "WebP animated" : "WebP";
		return(true);
	}
	if (ImageLoaderWebpMatchesAt(header, headerSize, ImageLoaderWebpChunkOffset, ImageLoaderWebpChunkLossless, sizeof(ImageLoaderWebpChunkLossless))) {
		if (headerSize < ImageLoaderWebpLosslessHeaderEnd || header[ImageLoaderWebpPayloadOffset] != ImageLoaderWebpLosslessSignature) {
			return(false);
		}
		uint32_t bits = ImageLoaderWebpReadLe32(header + ImageLoaderWebpLosslessBitsOffset);
		outHeader->width = (bits & ImageLoaderWebpLosslessSizeMask) + 1;
		outHeader->height = ((bits >> ImageLoaderWebpLosslessSizeBits) & ImageLoaderWebpLosslessSizeMask) + 1;
		outHeader->hasAlpha = ((bits >> ImageLoaderWebpLosslessAlphaBit) & 1) != 0;
		outHeader->formatName = "WebP lossless";
		return(true);
	}
	if (ImageLoaderWebpMatchesAt(header, headerSize, ImageLoaderWebpChunkOffset, ImageLoaderWebpChunkLossy, sizeof(ImageLoaderWebpChunkLossy))) {
		bool hasStartCode = ImageLoaderWebpMatchesAt(header, headerSize, ImageLoaderWebpLossyStartCodeOffset, ImageLoaderWebpLossyStartCode, sizeof(ImageLoaderWebpLossyStartCode));
		if (headerSize < ImageLoaderWebpLossyHeaderEnd || !hasStartCode) {
			return(false);
		}
		outHeader->width = ImageLoaderWebpReadLe16(header + ImageLoaderWebpLossyWidthOffset) & ImageLoaderWebpLossySizeMask;
		outHeader->height = ImageLoaderWebpReadLe16(header + ImageLoaderWebpLossyHeightOffset) & ImageLoaderWebpLossySizeMask;
		outHeader->formatName = "WebP lossy";
		return(true);
	}
	return(false);
}

static bool ImageLoaderWebpReadHeader(ImageSource *source, ImageLoaderWebpHeader *outHeader) {
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	source->seek(source, 0);
	size_t headerSize = source->read(source, header, sizeof(header));
	bool result = ImageLoaderWebpParseHeader(header, headerSize, outHeader);
	return(result);
}

static void ImageLoaderWebpLoadLibrary(ImageLoaderWebpLibrary *library) {
	memset(library, 0, sizeof(*library));
	library->libraryName = ImageLoaderWebpLibraryNames[0];
	for (size_t nameIndex = 0; ImageLoaderWebpLibraryNames[nameIndex] != NULL; ++nameIndex) {
		const char *libraryName = ImageLoaderWebpLibraryNames[nameIndex];
		if (!fplDynamicLibraryLoad(libraryName, &library->handle)) {
			continue;
		}
		library->getDecoderVersion = (ImageLoaderWebpGetDecoderVersionFunction *)fplGetDynamicLibraryProc(&library->handle, "WebPGetDecoderVersion");
		library->decodeRgba = (ImageLoaderWebpDecodeRgbaFunction *)fplGetDynamicLibraryProc(&library->handle, "WebPDecodeRGBA");
		library->free = (ImageLoaderWebpFreeFunction *)fplGetDynamicLibraryProc(&library->handle, "WebPFree");
		bool hasAllFunctions = library->getDecoderVersion != NULL && library->decodeRgba != NULL && library->free != NULL;
		if (!hasAllFunctions) {
			// WebPFree came with libwebp 1.0, an older library is skipped
			fplDynamicLibraryUnload(&library->handle);
			continue;
		}
		const int byteBits = 8;
		const int twoByteBits = 16;
		const int byteMask = 0xFF;
		int version = library->getDecoderVersion();
		int major = (version >> twoByteBits) & byteMask;
		int minor = (version >> byteBits) & byteMask;
		int revision = version & byteMask;
		snprintf(library->versionText, sizeof(library->versionText), "%d.%d.%d", major, minor, revision);
		library->libraryName = libraryName;
		library->isLoaded = true;
		return;
	}
	snprintf(library->versionText, sizeof(library->versionText), "missing");
}

static ImageLoaderMatch ImageLoaderWebpProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	if (ImageLoaderWebpIsSignature(header, headerSize)) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderWebpReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	ImageLoaderWebpHeader header;
	if (!ImageLoaderWebpReadHeader(source, &header)) {
		snprintf(message, messageSize, "no WebP header");
		return(ImageLoadResult_Corrupt);
	}
	uint32_t channelCount = header.hasAlpha ? ImageLoaderWebpRgbaChannels : ImageLoaderWebpRgbChannels;
	outInfo->width = header.width;
	outInfo->height = header.height;
	outInfo->channelCount = channelCount;
	outInfo->bitsPerPixel = channelCount * ImageLoaderWebpBitsPerChannel;
	outInfo->formatName = header.formatName;
	outInfo->orientation = ImageOrientation_Normal;
	outInfo->isPalette = false;
	return(ImageLoadResult_Success);
}

// The whole file into memory, the simple API of libwebp decodes from one buffer; the read position is the progress up to ImageLoaderWebpReadShare
static ImageLoadResult ImageLoaderWebpReadAll(ImageSource *source, uint8_t **outData, size_t *outDataSize, char *message, size_t messageSize) {
	uint64_t sourceSize = source->size(source);
	if (sourceSize == 0 || sourceSize > (uint64_t)SIZE_MAX) {
		snprintf(message, messageSize, "file size %llu", (unsigned long long)sourceSize);
		return(ImageLoadResult_Corrupt);
	}
	size_t dataSize = (size_t)sourceSize;
	uint8_t *data = (uint8_t *)malloc(dataSize);
	if (data == NULL) {
		snprintf(message, messageSize, "no memory for %zu bytes", dataSize);
		return(ImageLoadResult_OutOfMemory);
	}
	source->seek(source, 0);
	size_t position = 0;
	while (position < dataSize) {
		size_t chunkSize = dataSize - position < ImageLoaderWebpReadChunkSize ? dataSize - position : ImageLoaderWebpReadChunkSize;
		size_t readSize = source->read(source, data + position, chunkSize);
		if (readSize == 0 || source->isCanceled(source)) {
			free(data);
			bool isCanceled = source->isCanceled(source);
			const char *reason = isCanceled ? "canceled" : "file ends early";
			snprintf(message, messageSize, "%s", reason);
			return(isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt);
		}
		position += readSize;
		float readFraction = (float)((double)position / (double)dataSize);
		source->reportProgress(source, readFraction * ImageLoaderWebpReadShare, false);
	}
	*outData = data;
	*outDataSize = dataSize;
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderWebpDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	const ImageLoaderWebpLibrary *library = &imageLoaderWebp__library;
	if (!library->isLoaded) {
		snprintf(message, messageSize, "%s not found", library->libraryName);
		return(ImageLoadResult_Unsupported);
	}
	ImageLoaderWebpHeader header;
	if (!ImageLoaderWebpReadHeader(source, &header)) {
		snprintf(message, messageSize, "no WebP header");
		return(ImageLoadResult_Corrupt);
	}
	if (header.isAnimated) {
		snprintf(message, messageSize, "animated WebP is not supported");
		return(ImageLoadResult_Unsupported);
	}

	uint8_t *data = NULL;
	size_t dataSize = 0;
	ImageLoadResult readResult = ImageLoaderWebpReadAll(source, &data, &dataSize, message, messageSize);
	if (readResult != ImageLoadResult_Success) {
		return(readResult);
	}
	// Decoding has no progress of its own
	source->reportProgress(source, ImageLoaderWebpReadShare, true);
	int width = 0;
	int height = 0;
	uint8_t *pixels = library->decodeRgba(data, dataSize, &width, &height);
	free(data);
	if (pixels == NULL) {
		snprintf(message, messageSize, "libwebp cannot decode it");
		return(ImageLoadResult_Corrupt);
	}
	if (source->isCanceled(source)) {
		library->free(pixels);
		snprintf(message, messageSize, "canceled");
		return(ImageLoadResult_Canceled);
	}
	// WebPDecodeRGBA gives straight alpha, top row first, 4 bytes per pixel without padding
	outPixels->pixels = pixels;
	outPixels->width = (uint32_t)width;
	outPixels->height = (uint32_t)height;
	outPixels->stride = (uint32_t)width * ImageLoaderWebpRgbaChannels;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static void ImageLoaderWebpRelease(const ImageLoader *loader, ImagePixels *pixels) {
	const ImageLoaderWebpLibrary *library = &imageLoaderWebp__library;
	library->free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderWebpGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		ImageLoaderWebpLoadLibrary(&imageLoaderWebp__library);
		// 19aa1ca9-5c5b-427f-90ab-b85fc4a8c377
		const ImageLoaderId id = { { 0x19, 0xaa, 0x1c, 0xa9, 0x5c, 0x5b, 0x42, 0x7f, 0x90, 0xab, 0xb8, 0x5f, 0xc4, 0xa8, 0xc3, 0x77 } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "webp";
		loader.version = imageLoaderWebp__library.versionText;
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderWebpExtensions;
		loader.probe = ImageLoaderWebpProbe;
		loader.readInfo = ImageLoaderWebpReadInfo;
		loader.decode = ImageLoaderWebpDecode;
		loader.releasePixels = ImageLoaderWebpRelease;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_WEBP_IMPLEMENTATION
