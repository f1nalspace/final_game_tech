/*
Name:
	FPL_ImageViewer | Reference BMP loader

Description:
	Reads only uncompressed (BI_RGB) BMP files with 24 or 32 bit per pixel, bottom-up and top-down, everything else is Unsupported.
	stb_image reads these files too: this loader proves pinning a loader (--loader-for=bmp:reference-bmp) and the fallback to the next one,
	and serves loader authors as a living example. Where both read a file, the pixels are byte-identical to stb_image,
	including its rule for 32 bit files whose alpha bytes are all zero (they become opaque).
	It is registered behind stb_image, so it changes nothing unless it is pinned.

Usage:
	Include after imageloader.h. Define IMAGE_LOADER_BMP_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_BMP_H
#define IMAGE_LOADER_BMP_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_BMP_ID_TEXT "3e5883ac-265c-4795-9bf0-de30a8a127c3"

extern const ImageLoader *ImageLoaderBmpGet(void);

#endif // IMAGE_LOADER_BMP_H

#if defined(IMAGE_LOADER_BMP_IMPLEMENTATION) && !defined(IMAGE_LOADER_BMP_IMPLEMENTED)
#define IMAGE_LOADER_BMP_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const ImageLoaderBmpExtensions[] = { ".bmp", NULL };

// File header (14 bytes) followed by at least a BITMAPINFOHEADER (40 bytes)
static const size_t ImageLoaderBmpFileHeaderSize = 14;
static const uint32_t ImageLoaderBmpMinimumInfoHeaderSize = 40;
static const size_t ImageLoaderBmpDataOffsetOffset = 10;
static const size_t ImageLoaderBmpInfoHeaderSizeOffset = 14;
static const size_t ImageLoaderBmpWidthOffset = 18;
static const size_t ImageLoaderBmpHeightOffset = 22;
static const size_t ImageLoaderBmpPlanesOffset = 26;
static const size_t ImageLoaderBmpBitCountOffset = 28;
static const size_t ImageLoaderBmpCompressionOffset = 30;
static const uint32_t ImageLoaderBmpCompressionNone = 0;
static const uint32_t ImageLoaderBmpBits24 = 24;
static const uint32_t ImageLoaderBmpBits32 = 32;
static const uint32_t ImageLoaderBmpRowAlignment = 4;
static const uint32_t ImageLoaderBmpOutputChannels = 4;
static const uint8_t ImageLoaderBmpOpaque = 255;
// Keeps a corrupt header from asking for absurd amounts of memory (1 GB of RGBA)
static const uint64_t ImageLoaderBmpMaximumPixels = 1ull << 28;

typedef struct ImageLoaderBmpHeader {
	uint32_t width;
	uint32_t height;
	uint32_t bitCount;
	uint32_t dataOffset;
	uint32_t rowSize;
	bool isTopDown;
} ImageLoaderBmpHeader;

static uint32_t ImageLoaderBmpU32(const uint8_t *p) {
	const int byteBits = 8;
	uint32_t result = (uint32_t)p[0] | ((uint32_t)p[1] << byteBits) | ((uint32_t)p[2] << (byteBits * 2)) | ((uint32_t)p[3] << (byteBits * 3));
	return(result);
}

static uint16_t ImageLoaderBmpU16(const uint8_t *p) {
	const int byteBits = 8;
	uint16_t result = (uint16_t)(p[0] | (p[1] << byteBits));
	return(result);
}

static bool ImageLoaderBmpHasSignature(const uint8_t *header, const size_t headerSize) {
	const size_t signatureSize = 2;
	bool result = headerSize >= signatureSize && header[0] == 'B' && header[1] == 'M';
	return(result);
}

static ImageLoadResult ImageLoaderBmpParseHeader(ImageSource *source, ImageLoaderBmpHeader *outHeader, char *message, const size_t messageSize) {
	const size_t requiredSize = ImageLoaderBmpFileHeaderSize + ImageLoaderBmpMinimumInfoHeaderSize;
	const uint16_t singlePlane = 1;
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	source->seek(source, 0);
	size_t headerSize = source->read(source, header, sizeof(header));
	if (!ImageLoaderBmpHasSignature(header, headerSize)) {
		snprintf(message, messageSize, "no BMP signature");
		return(ImageLoadResult_Unsupported);
	}
	if (headerSize < requiredSize) {
		snprintf(message, messageSize, "BMP header truncated");
		return(ImageLoadResult_Corrupt);
	}
	uint32_t infoHeaderSize = ImageLoaderBmpU32(header + ImageLoaderBmpInfoHeaderSizeOffset);
	int32_t width = (int32_t)ImageLoaderBmpU32(header + ImageLoaderBmpWidthOffset);
	int32_t height = (int32_t)ImageLoaderBmpU32(header + ImageLoaderBmpHeightOffset);
	uint16_t planes = ImageLoaderBmpU16(header + ImageLoaderBmpPlanesOffset);
	uint16_t bitCount = ImageLoaderBmpU16(header + ImageLoaderBmpBitCountOffset);
	uint32_t compression = ImageLoaderBmpU32(header + ImageLoaderBmpCompressionOffset);
	if (infoHeaderSize < ImageLoaderBmpMinimumInfoHeaderSize || planes != singlePlane) {
		snprintf(message, messageSize, "only BITMAPINFOHEADER or newer");
		return(ImageLoadResult_Unsupported);
	}
	if (compression != ImageLoaderBmpCompressionNone || (bitCount != ImageLoaderBmpBits24 && bitCount != ImageLoaderBmpBits32)) {
		snprintf(message, messageSize, "only uncompressed 24 and 32 bit, this is %u bit with compression %u", (unsigned)bitCount, (unsigned)compression);
		return(ImageLoadResult_Unsupported);
	}
	// A negative height means the first row is the top one; INT32_MIN has no positive counterpart
	if (width <= 0 || height == 0 || height == INT32_MIN) {
		snprintf(message, messageSize, "bad BMP size");
		return(ImageLoadResult_Corrupt);
	}
	ImageLoaderBmpHeader result;
	result.width = (uint32_t)width;
	result.isTopDown = height < 0;
	result.height = result.isTopDown ? (uint32_t)(-height) : (uint32_t)height;
	result.bitCount = bitCount;
	result.dataOffset = ImageLoaderBmpU32(header + ImageLoaderBmpDataOffsetOffset);
	uint64_t pixelCount = (uint64_t)result.width * result.height;
	if (pixelCount > ImageLoaderBmpMaximumPixels) {
		snprintf(message, messageSize, "picture too large");
		return(ImageLoadResult_Unsupported);
	}
	const uint32_t byteBits = 8;
	uint64_t rowBits = (uint64_t)result.width * bitCount;
	uint64_t alignmentBits = (uint64_t)ImageLoaderBmpRowAlignment * byteBits;
	uint64_t rowSize = ((rowBits + alignmentBits - 1) / alignmentBits) * ImageLoaderBmpRowAlignment;
	result.rowSize = (uint32_t)rowSize;
	uint64_t fileSize = source->size(source);
	if ((uint64_t)result.dataOffset + rowSize * result.height > fileSize) {
		snprintf(message, messageSize, "BMP pixels truncated");
		return(ImageLoadResult_Corrupt);
	}
	*outHeader = result;
	return(ImageLoadResult_Success);
}

static ImageLoaderMatch ImageLoaderBmpProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	if (ImageLoaderBmpHasSignature(header, headerSize)) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderBmpReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	const uint32_t bitsPerChannel = 8;
	ImageLoaderBmpHeader header;
	ImageLoadResult result = ImageLoaderBmpParseHeader(source, &header, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	outInfo->width = header.width;
	outInfo->height = header.height;
	outInfo->channelCount = header.bitCount / bitsPerChannel;
	outInfo->bitsPerPixel = header.bitCount;
	outInfo->formatName = "BMP";
	outInfo->orientation = ImageOrientation_Normal;
	outInfo->isPalette = false;
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderBmpDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	const size_t blueOffset = 0;
	const size_t greenOffset = 1;
	const size_t redOffset = 2;
	const size_t alphaOffset = 3;
	const uint32_t byteBits = 8;
	ImageLoaderBmpHeader header;
	ImageLoadResult result = ImageLoaderBmpParseHeader(source, &header, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	size_t outputStride = (size_t)header.width * ImageLoaderBmpOutputChannels;
	size_t bytesPerPixel = header.bitCount / byteBits;
	bool hasAlpha = header.bitCount == ImageLoaderBmpBits32;
	uint8_t *pixels = (uint8_t *)malloc(outputStride * header.height);
	uint8_t *row = (uint8_t *)malloc(header.rowSize);
	if (pixels == NULL || row == NULL) {
		free(pixels);
		free(row);
		snprintf(message, messageSize, "out of memory");
		return(ImageLoadResult_OutOfMemory);
	}
	source->seek(source, header.dataOffset);
	uint8_t combinedAlpha = 0;
	for (uint32_t fileRow = 0; fileRow < header.height; ++fileRow) {
		if (source->read(source, row, header.rowSize) != header.rowSize) {
			free(pixels);
			free(row);
			bool isCanceled = source->isCanceled(source);
			snprintf(message, messageSize, isCanceled ? "canceled" : "truncated");
			return(isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt);
		}
		uint32_t y = header.isTopDown ? fileRow : header.height - 1 - fileRow;
		uint8_t *target = pixels + (size_t)y * outputStride;
		for (uint32_t x = 0; x < header.width; ++x) {
			const uint8_t *source8 = row + (size_t)x * bytesPerPixel;
			uint8_t *pixel = target + (size_t)x * ImageLoaderBmpOutputChannels;
			pixel[0] = source8[redOffset];
			pixel[1] = source8[greenOffset];
			pixel[2] = source8[blueOffset];
			pixel[3] = hasAlpha ? source8[alphaOffset] : ImageLoaderBmpOpaque;
			combinedAlpha |= pixel[3];
		}
	}
	free(row);
	// Like stb_image: a 32 bit file whose alpha bytes are all zero never meant transparency
	if (combinedAlpha == 0) {
		size_t pixelCount = (size_t)header.width * header.height;
		for (size_t index = 0; index < pixelCount; ++index) {
			pixels[index * ImageLoaderBmpOutputChannels + alphaOffset] = ImageLoaderBmpOpaque;
		}
	}
	outPixels->pixels = pixels;
	outPixels->width = header.width;
	outPixels->height = header.height;
	outPixels->stride = (uint32_t)outputStride;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static void ImageLoaderBmpRelease(const ImageLoader *loader, ImagePixels *pixels) {
	free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderBmpGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		// 3e5883ac-265c-4795-9bf0-de30a8a127c3
		const ImageLoaderId id = { { 0x3e, 0x58, 0x83, 0xac, 0x26, 0x5c, 0x47, 0x95, 0x9b, 0xf0, 0xde, 0x30, 0xa8, 0xa1, 0x27, 0xc3 } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "reference-bmp";
		loader.version = "1.0";
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderBmpExtensions;
		loader.probe = ImageLoaderBmpProbe;
		loader.readInfo = ImageLoaderBmpReadInfo;
		loader.decode = ImageLoaderBmpDecode;
		loader.releasePixels = ImageLoaderBmpRelease;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_BMP_IMPLEMENTATION
