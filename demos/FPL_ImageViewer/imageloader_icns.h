/*
Name:
	FPL_ImageViewer | ICNS loader

Description:
	Reads Apple icon files (.icns) without any library and shows the largest icon the file holds.
	PNG icons go through stb_image, ARGB icons and the old 24 bit icons with their 8 bit masks (is32/s8mk, il32/l8mk, ih32/h8mk, it32/t8mk) are unpacked here.
	JPEG 2000 icons are skipped, the next smaller icon is shown instead. The old 1, 4 and 8 bit icons are not read.

Usage:
	Include after stb_image.h and imageloader.h. Define IMAGE_LOADER_ICNS_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_ICNS_H
#define IMAGE_LOADER_ICNS_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_ICNS_ID_TEXT "16217c30-79f2-4f1c-918b-948f60b36511"

extern const ImageLoader *ImageLoaderIcnsGet(void);

#endif // IMAGE_LOADER_ICNS_H

#if defined(IMAGE_LOADER_ICNS_IMPLEMENTATION) && !defined(IMAGE_LOADER_ICNS_IMPLEMENTED)
#define IMAGE_LOADER_ICNS_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const ImageLoaderIcnsExtensions[] = { ".icns", NULL };

// File: "icns" and the file length, then entries of a type, their length including this 8 byte header and the data
static const uint8_t ImageLoaderIcnsSignature[] = { 'i', 'c', 'n', 's' };
static const size_t ImageLoaderIcnsTypeSize = 4;
static const size_t ImageLoaderIcnsHeaderSize = 8;
// An icon file with the largest icons as PNG has a few MB, a larger file is no icon file
static const uint64_t ImageLoaderIcnsMaximumFileSize = 64 * 1024 * 1024;
static const uint8_t ImageLoaderIcnsPngSignature[] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
// PNG IHDR: width and height as big endian 32 bit values behind the signature, the chunk length and the chunk type
static const size_t ImageLoaderIcnsPngWidthOffset = 16;
static const size_t ImageLoaderIcnsPngHeightOffset = 20;
static const size_t ImageLoaderIcnsPngHeaderEnd = 24;
// ARGB icons: "ARGB", then the packed alpha, red, green and blue channels
static const uint8_t ImageLoaderIcnsArgbSignature[] = { 'A', 'R', 'G', 'B' };
// it32 data starts with four zero bytes before the packed channels
static const size_t ImageLoaderIcnsIt32PrefixSize = 4;
// Packed channels: a control byte below 0x80 copies the next control + 1 bytes, from 0x80 on the next byte repeats control - 0x80 + 3 times
static const uint8_t ImageLoaderIcnsRunFlag = 0x80;
static const uint32_t ImageLoaderIcnsLiteralBias = 1;
static const uint32_t ImageLoaderIcnsRunBias = 3;
static const uint32_t ImageLoaderIcnsChannels = 4;
static const uint32_t ImageLoaderIcnsRgbChannels = 3;
static const uint32_t ImageLoaderIcnsBitsPerChannel = 8;
static const uint32_t ImageLoaderIcnsAlphaChannelIndex = 3;
static const uint8_t ImageLoaderIcnsOpaque = 0xFF;

typedef enum ImageLoaderIcnsKind {
	// PNG, JPEG 2000 or ARGB, told apart by the data
	ImageLoaderIcnsKind_Embedded = 0,
	// Packed 24 bit RGB with an 8 bit mask in an entry of its own
	ImageLoaderIcnsKind_PackedRgb,
} ImageLoaderIcnsKind;

typedef struct ImageLoaderIcnsType {
	const char *type;
	uint32_t side;
	ImageLoaderIcnsKind kind;
	// Mask entry of a packed RGB icon
	const char *maskType;
} ImageLoaderIcnsType;

// Every icon type the loader reads; @2x types count with their pixel size
static const ImageLoaderIcnsType ImageLoaderIcnsTypes[] = {
	{ "ic10", 1024, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic09", 512, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic14", 512, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic08", 256, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic13", 256, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic07", 128, ImageLoaderIcnsKind_Embedded, NULL },
	{ "it32", 128, ImageLoaderIcnsKind_PackedRgb, "t8mk" },
	{ "icp6", 64, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic12", 64, ImageLoaderIcnsKind_Embedded, NULL },
	{ "SB24", 48, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ih32", 48, ImageLoaderIcnsKind_PackedRgb, "h8mk" },
	{ "icsB", 36, ImageLoaderIcnsKind_Embedded, NULL },
	{ "icp5", 32, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic05", 32, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic11", 32, ImageLoaderIcnsKind_Embedded, NULL },
	{ "il32", 32, ImageLoaderIcnsKind_PackedRgb, "l8mk" },
	{ "sb24", 24, ImageLoaderIcnsKind_Embedded, NULL },
	{ "icsb", 18, ImageLoaderIcnsKind_Embedded, NULL },
	{ "icp4", 16, ImageLoaderIcnsKind_Embedded, NULL },
	{ "ic04", 16, ImageLoaderIcnsKind_Embedded, NULL },
	{ "is32", 16, ImageLoaderIcnsKind_PackedRgb, "s8mk" },
};

typedef enum ImageLoaderIcnsEncoding {
	ImageLoaderIcnsEncoding_None = 0,
	ImageLoaderIcnsEncoding_Png,
	ImageLoaderIcnsEncoding_Argb,
	ImageLoaderIcnsEncoding_PackedRgb,
} ImageLoaderIcnsEncoding;

// The icon the loader shows: where its data lies in the file and how it is stored
typedef struct ImageLoaderIcnsIcon {
	const uint8_t *data;
	size_t dataSize;
	const ImageLoaderIcnsType *type;
	uint32_t width;
	uint32_t height;
	ImageLoaderIcnsEncoding encoding;
} ImageLoaderIcnsIcon;

static bool ImageLoaderIcnsStartsWith(const uint8_t *data, const size_t dataSize, const uint8_t *bytes, const size_t byteCount) {
	bool result = dataSize >= byteCount && memcmp(data, bytes, byteCount) == 0;
	return(result);
}

static uint32_t ImageLoaderIcnsReadBe32(const uint8_t *bytes) {
	const uint32_t byteBits = 8;
	const size_t valueSize = 4;
	uint32_t result = 0;
	for (size_t byteIndex = 0; byteIndex < valueSize; ++byteIndex) {
		result = (result << byteBits) | bytes[byteIndex];
	}
	return(result);
}

static const ImageLoaderIcnsType *ImageLoaderIcnsFindType(const uint8_t *type) {
	size_t typeCount = sizeof(ImageLoaderIcnsTypes) / sizeof(ImageLoaderIcnsTypes[0]);
	for (size_t typeIndex = 0; typeIndex < typeCount; ++typeIndex) {
		const ImageLoaderIcnsType *candidate = &ImageLoaderIcnsTypes[typeIndex];
		if (memcmp(type, candidate->type, ImageLoaderIcnsTypeSize) == 0) {
			return(candidate);
		}
	}
	return(NULL);
}

// How the data of an entry is stored and the size it decodes to; None for JPEG 2000 and anything unknown
static ImageLoaderIcnsEncoding ImageLoaderIcnsClassify(const ImageLoaderIcnsType *type, const uint8_t *data, const size_t dataSize, uint32_t *outWidth, uint32_t *outHeight) {
	*outWidth = type->side;
	*outHeight = type->side;
	if (type->kind == ImageLoaderIcnsKind_PackedRgb) {
		return(ImageLoaderIcnsEncoding_PackedRgb);
	}
	if (ImageLoaderIcnsStartsWith(data, dataSize, ImageLoaderIcnsPngSignature, sizeof(ImageLoaderIcnsPngSignature))) {
		if (dataSize < ImageLoaderIcnsPngHeaderEnd) {
			return(ImageLoaderIcnsEncoding_None);
		}
		*outWidth = ImageLoaderIcnsReadBe32(data + ImageLoaderIcnsPngWidthOffset);
		*outHeight = ImageLoaderIcnsReadBe32(data + ImageLoaderIcnsPngHeightOffset);
		return(ImageLoaderIcnsEncoding_Png);
	}
	if (ImageLoaderIcnsStartsWith(data, dataSize, ImageLoaderIcnsArgbSignature, sizeof(ImageLoaderIcnsArgbSignature))) {
		return(ImageLoaderIcnsEncoding_Argb);
	}
	return(ImageLoaderIcnsEncoding_None);
}

// The data of the first entry of this type, null when the file has none
static const uint8_t *ImageLoaderIcnsFindEntry(const uint8_t *file, const size_t fileSize, const char *type, size_t *outDataSize) {
	size_t offset = ImageLoaderIcnsHeaderSize;
	while (offset + ImageLoaderIcnsHeaderSize <= fileSize) {
		const uint8_t *entry = file + offset;
		uint32_t entrySize = ImageLoaderIcnsReadBe32(entry + ImageLoaderIcnsTypeSize);
		if (entrySize < ImageLoaderIcnsHeaderSize || entrySize > fileSize - offset) {
			return(NULL);
		}
		if (memcmp(entry, type, ImageLoaderIcnsTypeSize) == 0) {
			*outDataSize = entrySize - ImageLoaderIcnsHeaderSize;
			return(entry + ImageLoaderIcnsHeaderSize);
		}
		offset += entrySize;
	}
	return(NULL);
}

// The largest icon the loader can read; the entry list ends at the first entry that does not fit into the file
static bool ImageLoaderIcnsSelectIcon(const uint8_t *file, const size_t fileSize, ImageLoaderIcnsIcon *outIcon) {
	memset(outIcon, 0, sizeof(*outIcon));
	if (!ImageLoaderIcnsStartsWith(file, fileSize, ImageLoaderIcnsSignature, sizeof(ImageLoaderIcnsSignature)) || fileSize < ImageLoaderIcnsHeaderSize) {
		return(false);
	}
	uint32_t declaredSize = ImageLoaderIcnsReadBe32(file + ImageLoaderIcnsTypeSize);
	size_t entriesEnd = declaredSize < fileSize ? declaredSize : fileSize;
	uint64_t bestPixelCount = 0;
	size_t offset = ImageLoaderIcnsHeaderSize;
	while (offset + ImageLoaderIcnsHeaderSize <= entriesEnd) {
		const uint8_t *entry = file + offset;
		uint32_t entrySize = ImageLoaderIcnsReadBe32(entry + ImageLoaderIcnsTypeSize);
		if (entrySize < ImageLoaderIcnsHeaderSize || entrySize > entriesEnd - offset) {
			break;
		}
		const ImageLoaderIcnsType *type = ImageLoaderIcnsFindType(entry);
		if (type != NULL) {
			const uint8_t *data = entry + ImageLoaderIcnsHeaderSize;
			size_t dataSize = entrySize - ImageLoaderIcnsHeaderSize;
			uint32_t width = 0;
			uint32_t height = 0;
			ImageLoaderIcnsEncoding encoding = ImageLoaderIcnsClassify(type, data, dataSize, &width, &height);
			uint64_t pixelCount = (uint64_t)width * (uint64_t)height;
			if (encoding != ImageLoaderIcnsEncoding_None && pixelCount > bestPixelCount) {
				bestPixelCount = pixelCount;
				outIcon->data = data;
				outIcon->dataSize = dataSize;
				outIcon->type = type;
				outIcon->width = width;
				outIcon->height = height;
				outIcon->encoding = encoding;
			}
		}
		offset += entrySize;
	}
	bool result = bestPixelCount > 0;
	return(result);
}

// Unpacks one channel of pixelCount values into every fourth byte of pixels, starting at channelIndex
static bool ImageLoaderIcnsUnpackChannel(const uint8_t *data, const size_t dataSize, size_t *position, uint8_t *pixels, const uint32_t pixelCount, const uint32_t channelIndex) {
	uint32_t pixelIndex = 0;
	size_t readPosition = *position;
	while (pixelIndex < pixelCount) {
		if (readPosition >= dataSize) {
			return(false);
		}
		uint8_t control = data[readPosition++];
		bool isRun = control >= ImageLoaderIcnsRunFlag;
		uint32_t count = isRun ? (uint32_t)(control - ImageLoaderIcnsRunFlag) + ImageLoaderIcnsRunBias : (uint32_t)control + ImageLoaderIcnsLiteralBias;
		if (count > pixelCount - pixelIndex) {
			return(false);
		}
		if (isRun) {
			if (readPosition >= dataSize) {
				return(false);
			}
			uint8_t value = data[readPosition++];
			for (uint32_t index = 0; index < count; ++index) {
				pixels[(size_t)(pixelIndex + index) * ImageLoaderIcnsChannels + channelIndex] = value;
			}
		} else {
			if (count > dataSize - readPosition) {
				return(false);
			}
			for (uint32_t index = 0; index < count; ++index) {
				pixels[(size_t)(pixelIndex + index) * ImageLoaderIcnsChannels + channelIndex] = data[readPosition + index];
			}
			readPosition += count;
		}
		pixelIndex += count;
	}
	*position = readPosition;
	return(true);
}

// ARGB: alpha, red, green, blue, each channel packed on its own
static bool ImageLoaderIcnsUnpackArgb(const ImageLoaderIcnsIcon *icon, uint8_t *pixels) {
	const uint32_t argbChannelOrder[] = { ImageLoaderIcnsAlphaChannelIndex, 0, 1, 2 };
	uint32_t pixelCount = icon->width * icon->height;
	size_t position = sizeof(ImageLoaderIcnsArgbSignature);
	for (size_t channel = 0; channel < sizeof(argbChannelOrder) / sizeof(argbChannelOrder[0]); ++channel) {
		if (!ImageLoaderIcnsUnpackChannel(icon->data, icon->dataSize, &position, pixels, pixelCount, argbChannelOrder[channel])) {
			return(false);
		}
	}
	return(true);
}

// 24 bit RGB: red, green and blue packed on their own, or unpacked as four bytes per pixel with the first one unused; alpha from the mask entry or opaque
static bool ImageLoaderIcnsUnpackRgb(const ImageLoaderIcnsIcon *icon, const uint8_t *file, const size_t fileSize, uint8_t *pixels) {
	uint32_t pixelCount = icon->width * icon->height;
	size_t unpackedSize = (size_t)pixelCount * ImageLoaderIcnsChannels;
	bool isIt32 = memcmp(icon->type->type, "it32", ImageLoaderIcnsTypeSize) == 0;
	size_t position = isIt32 ? ImageLoaderIcnsIt32PrefixSize : 0;
	if (icon->dataSize == unpackedSize) {
		for (uint32_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
			const uint8_t *source = icon->data + (size_t)pixelIndex * ImageLoaderIcnsChannels;
			uint8_t *target = pixels + (size_t)pixelIndex * ImageLoaderIcnsChannels;
			// xRGB: the first byte is unused
			target[0] = source[1];
			target[1] = source[2];
			target[2] = source[3];
		}
	} else {
		for (uint32_t channel = 0; channel < ImageLoaderIcnsRgbChannels; ++channel) {
			if (!ImageLoaderIcnsUnpackChannel(icon->data, icon->dataSize, &position, pixels, pixelCount, channel)) {
				return(false);
			}
		}
	}
	size_t maskSize = 0;
	const uint8_t *mask = ImageLoaderIcnsFindEntry(file, fileSize, icon->type->maskType, &maskSize);
	bool hasMask = mask != NULL && maskSize == pixelCount;
	for (uint32_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
		uint8_t alpha = hasMask ? mask[pixelIndex] : ImageLoaderIcnsOpaque;
		pixels[(size_t)pixelIndex * ImageLoaderIcnsChannels + ImageLoaderIcnsAlphaChannelIndex] = alpha;
	}
	return(true);
}

// PNG through stb_image, copied so every icon is released the same way
static ImageLoadResult ImageLoaderIcnsDecodePng(const ImageLoaderIcnsIcon *icon, uint8_t **outPixels, uint32_t *outWidth, uint32_t *outHeight, char *message, size_t messageSize) {
	const int requestedComponents = (int)ImageLoaderIcnsChannels;
	if (icon->dataSize > (size_t)INT32_MAX) {
		snprintf(message, messageSize, "PNG icon of %zu bytes", icon->dataSize);
		return(ImageLoadResult_Corrupt);
	}
	int width = 0;
	int height = 0;
	int components = 0;
	stbi_uc *decoded = stbi_load_from_memory(icon->data, (int)icon->dataSize, &width, &height, &components, requestedComponents);
	if (decoded == NULL) {
		const char *reason = stbi_failure_reason();
		snprintf(message, messageSize, "PNG icon: %s", reason);
		return(ImageLoadResult_Corrupt);
	}
	size_t pixelsSize = (size_t)width * (size_t)height * ImageLoaderIcnsChannels;
	uint8_t *pixels = (uint8_t *)malloc(pixelsSize);
	if (pixels == NULL) {
		stbi_image_free(decoded);
		snprintf(message, messageSize, "no memory for %zu bytes", pixelsSize);
		return(ImageLoadResult_OutOfMemory);
	}
	memcpy(pixels, decoded, pixelsSize);
	stbi_image_free(decoded);
	*outPixels = pixels;
	*outWidth = (uint32_t)width;
	*outHeight = (uint32_t)height;
	return(ImageLoadResult_Success);
}

// The whole file, icon files are small and their entries point anywhere
static ImageLoadResult ImageLoaderIcnsReadAll(ImageSource *source, uint8_t **outData, size_t *outDataSize, char *message, size_t messageSize) {
	uint64_t sourceSize = source->size(source);
	if (sourceSize < ImageLoaderIcnsHeaderSize || sourceSize > ImageLoaderIcnsMaximumFileSize) {
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
		size_t readSize = source->read(source, data + position, dataSize - position);
		if (readSize == 0 || source->isCanceled(source)) {
			free(data);
			bool isCanceled = source->isCanceled(source);
			const char *reason = isCanceled ? "canceled" : "file ends early";
			snprintf(message, messageSize, "%s", reason);
			return(isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt);
		}
		position += readSize;
	}
	*outData = data;
	*outDataSize = dataSize;
	return(ImageLoadResult_Success);
}

static ImageLoaderMatch ImageLoaderIcnsProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	if (ImageLoaderIcnsStartsWith(header, headerSize, ImageLoaderIcnsSignature, sizeof(ImageLoaderIcnsSignature))) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderIcnsReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	uint8_t *file = NULL;
	size_t fileSize = 0;
	ImageLoadResult readResult = ImageLoaderIcnsReadAll(source, &file, &fileSize, message, messageSize);
	if (readResult != ImageLoadResult_Success) {
		return(readResult);
	}
	ImageLoaderIcnsIcon icon;
	bool hasIcon = ImageLoaderIcnsSelectIcon(file, fileSize, &icon);
	free(file);
	if (!hasIcon) {
		snprintf(message, messageSize, "no icon this loader reads");
		return(ImageLoadResult_Unsupported);
	}
	outInfo->width = icon.width;
	outInfo->height = icon.height;
	outInfo->channelCount = ImageLoaderIcnsChannels;
	outInfo->bitsPerPixel = ImageLoaderIcnsChannels * ImageLoaderIcnsBitsPerChannel;
	outInfo->formatName = "ICNS";
	outInfo->orientation = ImageOrientation_Normal;
	outInfo->isPalette = false;
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderIcnsDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	uint8_t *file = NULL;
	size_t fileSize = 0;
	ImageLoadResult readResult = ImageLoaderIcnsReadAll(source, &file, &fileSize, message, messageSize);
	if (readResult != ImageLoadResult_Success) {
		return(readResult);
	}
	ImageLoaderIcnsIcon icon;
	if (!ImageLoaderIcnsSelectIcon(file, fileSize, &icon)) {
		free(file);
		snprintf(message, messageSize, "no icon this loader reads");
		return(ImageLoadResult_Unsupported);
	}
	ImageLoadResult result = ImageLoadResult_Success;
	uint8_t *pixels = NULL;
	uint32_t width = icon.width;
	uint32_t height = icon.height;
	if (icon.encoding == ImageLoaderIcnsEncoding_Png) {
		result = ImageLoaderIcnsDecodePng(&icon, &pixels, &width, &height, message, messageSize);
	} else {
		size_t pixelsSize = (size_t)width * (size_t)height * ImageLoaderIcnsChannels;
		pixels = (uint8_t *)malloc(pixelsSize);
		if (pixels == NULL) {
			snprintf(message, messageSize, "no memory for %zu bytes", pixelsSize);
			result = ImageLoadResult_OutOfMemory;
		} else {
			bool isUnpacked = icon.encoding == ImageLoaderIcnsEncoding_Argb ? ImageLoaderIcnsUnpackArgb(&icon, pixels) : ImageLoaderIcnsUnpackRgb(&icon, file, fileSize, pixels);
			if (!isUnpacked) {
				free(pixels);
				pixels = NULL;
				snprintf(message, messageSize, "packed %.4s icon is broken", icon.type->type);
				result = ImageLoadResult_Corrupt;
			}
		}
	}
	free(file);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	outPixels->pixels = pixels;
	outPixels->width = width;
	outPixels->height = height;
	outPixels->stride = width * ImageLoaderIcnsChannels;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static void ImageLoaderIcnsRelease(const ImageLoader *loader, ImagePixels *pixels) {
	free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderIcnsGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		// 16217c30-79f2-4f1c-918b-948f60b36511
		const ImageLoaderId id = { { 0x16, 0x21, 0x7c, 0x30, 0x79, 0xf2, 0x4f, 0x1c, 0x91, 0x8b, 0x94, 0x8f, 0x60, 0xb3, 0x65, 0x11 } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "icns";
		loader.version = "1.0";
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderIcnsExtensions;
		loader.probe = ImageLoaderIcnsProbe;
		loader.readInfo = ImageLoaderIcnsReadInfo;
		loader.decode = ImageLoaderIcnsDecode;
		loader.releasePixels = ImageLoaderIcnsRelease;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_ICNS_IMPLEMENTATION
