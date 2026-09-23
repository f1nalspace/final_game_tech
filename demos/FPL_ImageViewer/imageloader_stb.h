/*
Name:
	FPL_ImageViewer | stb_image loader

Description:
	The default loader: JPEG, PNG and BMP through stb_image. A look at the file header adds what stb_image does not report:
	the bit depth and palette of PNG and BMP files and the EXIF orientation of JPEG files.
	stb_image keeps global switches, they are set once in ImageLoaderStbGet(). Its failure reason is thread-local (STBI_THREAD_LOCAL, stb_image 2.26 or newer),
	so the loader is thread-safe.

Usage:
	Include after stb_image.h and imageloader.h. Define IMAGE_LOADER_STB_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_STB_H
#define IMAGE_LOADER_STB_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_STB_ID_TEXT "854c10ca-79dc-4202-b064-586af81a96e0"

// The loader, filled on the first call, which also sets the global switches of stb_image
extern const ImageLoader *ImageLoaderStbGet(void);

#endif // IMAGE_LOADER_STB_H

#if defined(IMAGE_LOADER_STB_IMPLEMENTATION) && !defined(IMAGE_LOADER_STB_IMPLEMENTED)
#define IMAGE_LOADER_STB_IMPLEMENTED

#include <stdio.h>
#include <string.h>

static const char *const ImageLoaderStbExtensions[] = { ".jpg", ".jpeg", ".png", ".bmp", NULL };
static const uint8_t ImageLoaderStbJpegSignature[] = { 0xFF, 0xD8, 0xFF };
static const uint8_t ImageLoaderStbPngSignature[] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
static const uint8_t ImageLoaderStbBmpSignature[] = { 'B', 'M' };

// PNG: IHDR is the first chunk, bit depth and color type follow width and height
static const size_t ImageLoaderStbPngBitDepthOffset = 24;
static const size_t ImageLoaderStbPngColorTypeOffset = 25;
static const uint8_t ImageLoaderStbPngColorTypeGray = 0;
static const uint8_t ImageLoaderStbPngColorTypeRGB = 2;
static const uint8_t ImageLoaderStbPngColorTypePalette = 3;
static const uint8_t ImageLoaderStbPngColorTypeGrayAlpha = 4;
static const uint8_t ImageLoaderStbPngColorTypeRGBA = 6;
// BMP: bits per pixel in the info header
static const size_t ImageLoaderStbBmpBitCountOffset = 28;
static const uint32_t ImageLoaderStbBmpLargestPaletteBits = 8;
static const uint32_t ImageLoaderStbBitsPerChannel = 8;
static const uint32_t ImageLoaderStbBitsPerWideChannel = 16;
static const int ImageLoaderStbRequestedComponents = 4;

typedef enum ImageLoaderStbFormat {
	ImageLoaderStbFormat_Unknown = 0,
	ImageLoaderStbFormat_Jpeg,
	ImageLoaderStbFormat_Png,
	ImageLoaderStbFormat_Bmp,
} ImageLoaderStbFormat;

static bool ImageLoaderStbStartsWith(const uint8_t *header, const size_t headerSize, const uint8_t *signature, const size_t signatureSize) {
	bool result = headerSize >= signatureSize && memcmp(header, signature, signatureSize) == 0;
	return(result);
}

static ImageLoaderStbFormat ImageLoaderStbDetect(const uint8_t *header, const size_t headerSize) {
	if (ImageLoaderStbStartsWith(header, headerSize, ImageLoaderStbJpegSignature, sizeof(ImageLoaderStbJpegSignature))) {
		return(ImageLoaderStbFormat_Jpeg);
	}
	if (ImageLoaderStbStartsWith(header, headerSize, ImageLoaderStbPngSignature, sizeof(ImageLoaderStbPngSignature))) {
		return(ImageLoaderStbFormat_Png);
	}
	if (ImageLoaderStbStartsWith(header, headerSize, ImageLoaderStbBmpSignature, sizeof(ImageLoaderStbBmpSignature))) {
		return(ImageLoaderStbFormat_Bmp);
	}
	return(ImageLoaderStbFormat_Unknown);
}

static int ImageLoaderStbRead(void *user, char *data, int size) {
	ImageSource *source = (ImageSource *)user;
	size_t readSize = source->read(source, data, (size_t)size);
	return((int)readSize);
}

// stb_image skips forward and, with a negative count, back
static void ImageLoaderStbSkip(void *user, int count) {
	ImageSource *source = (ImageSource *)user;
	int64_t position = (int64_t)source->tell(source);
	int64_t size = (int64_t)source->size(source);
	int64_t target = position + count;
	int64_t clamped = target < 0 ? 0 : (target > size ? size : target);
	source->seek(source, (uint64_t)clamped);
}

static int ImageLoaderStbEof(void *user) {
	ImageSource *source = (ImageSource *)user;
	bool isEnd = source->tell(source) >= source->size(source) || source->isCanceled(source);
	return(isEnd ? 1 : 0);
}

static stbi_io_callbacks ImageLoaderStbCallbacks(void) {
	stbi_io_callbacks result;
	result.read = ImageLoaderStbRead;
	result.skip = ImageLoaderStbSkip;
	result.eof = ImageLoaderStbEof;
	return(result);
}

// Maps the reason stb_image gives to a result
static ImageLoadResult ImageLoaderStbFailure(ImageSource *source, char *message, const size_t messageSize) {
	if (source->isCanceled(source)) {
		snprintf(message, messageSize, "canceled");
		return(ImageLoadResult_Canceled);
	}
	const char *reason = stbi_failure_reason();
	if (reason == NULL) {
		reason = "unknown";
	}
	snprintf(message, messageSize, "%s", reason);
	if (strcmp(reason, "outofmem") == 0) {
		return(ImageLoadResult_OutOfMemory);
	}
	if (strcmp(reason, "unknown image type") == 0 || strcmp(reason, "BMP RLE") == 0) {
		return(ImageLoadResult_Unsupported);
	}
	return(ImageLoadResult_Corrupt);
}

static ImageLoaderMatch ImageLoaderStbProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	ImageLoaderStbFormat format = ImageLoaderStbDetect(header, headerSize);
	if (format != ImageLoaderStbFormat_Unknown) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

// Channels per pixel of a PNG color type, 0 for an unknown one
static uint32_t ImageLoaderStbPngChannels(const uint8_t colorType) {
	const uint32_t grayChannels = 1;
	const uint32_t grayAlphaChannels = 2;
	const uint32_t rgbChannels = 3;
	const uint32_t rgbaChannels = 4;
	if (colorType == ImageLoaderStbPngColorTypeGray || colorType == ImageLoaderStbPngColorTypePalette) {
		return(grayChannels);
	}
	if (colorType == ImageLoaderStbPngColorTypeGrayAlpha) {
		return(grayAlphaChannels);
	}
	if (colorType == ImageLoaderStbPngColorTypeRGB) {
		return(rgbChannels);
	}
	if (colorType == ImageLoaderStbPngColorTypeRGBA) {
		return(rgbaChannels);
	}
	return(0);
}

static ImageLoadResult ImageLoaderStbReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	stbi_io_callbacks callbacks = ImageLoaderStbCallbacks();
	int width = 0;
	int height = 0;
	int components = 0;
	if (!stbi_info_from_callbacks(&callbacks, source, &width, &height, &components)) {
		ImageLoadResult failure = ImageLoaderStbFailure(source, message, messageSize);
		return(failure);
	}
	source->seek(source, 0);
	int is16Bit = stbi_is_16_bit_from_callbacks(&callbacks, source);
	uint32_t bitsPerChannel = is16Bit ? ImageLoaderStbBitsPerWideChannel : ImageLoaderStbBitsPerChannel;

	// stbi_info reports a top-down BMP with its negative height, the decoder does not
	int absoluteHeight = height < 0 ? -height : height;
	outInfo->width = (uint32_t)width;
	outInfo->height = (uint32_t)absoluteHeight;
	outInfo->channelCount = (uint32_t)components;
	outInfo->bitsPerPixel = (uint32_t)components * bitsPerChannel;
	outInfo->orientation = ImageOrientation_Normal;

	// stb_image reports a palette PNG as 24 or 32 bpp, the header knows better
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	source->seek(source, 0);
	size_t headerSize = source->read(source, header, sizeof(header));
	ImageLoaderStbFormat format = ImageLoaderStbDetect(header, headerSize);
	switch (format) {
		case ImageLoaderStbFormat_Jpeg:
		{
			outInfo->formatName = "JPEG";
			outInfo->orientation = ImageJpegReadOrientation(source);
		} break;
		case ImageLoaderStbFormat_Png:
		{
			outInfo->formatName = "PNG";
			if (headerSize > ImageLoaderStbPngColorTypeOffset) {
				uint8_t bitDepth = header[ImageLoaderStbPngBitDepthOffset];
				uint8_t colorType = header[ImageLoaderStbPngColorTypeOffset];
				uint32_t channels = ImageLoaderStbPngChannels(colorType);
				if (channels > 0) {
					outInfo->bitsPerPixel = channels * bitDepth;
				}
				outInfo->isPalette = colorType == ImageLoaderStbPngColorTypePalette;
			}
		} break;
		case ImageLoaderStbFormat_Bmp:
		{
			const int byteBits = 8;
			outInfo->formatName = "BMP";
			if (headerSize > ImageLoaderStbBmpBitCountOffset + 1) {
				uint32_t bitCount = (uint32_t)header[ImageLoaderStbBmpBitCountOffset] | ((uint32_t)header[ImageLoaderStbBmpBitCountOffset + 1] << byteBits);
				outInfo->bitsPerPixel = bitCount;
				outInfo->isPalette = bitCount <= ImageLoaderStbBmpLargestPaletteBits;
			}
		} break;
		default:
		{
			outInfo->formatName = "Unknown";
		} break;
	}
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderStbDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	stbi_io_callbacks callbacks = ImageLoaderStbCallbacks();
	int width = 0;
	int height = 0;
	int components = 0;
	stbi_uc *pixels = stbi_load_from_callbacks(&callbacks, source, &width, &height, &components, ImageLoaderStbRequestedComponents);
	if (pixels == NULL) {
		ImageLoadResult failure = ImageLoaderStbFailure(source, message, messageSize);
		return(failure);
	}
	if (source->isCanceled(source)) {
		stbi_image_free(pixels);
		snprintf(message, messageSize, "canceled");
		return(ImageLoadResult_Canceled);
	}
	outPixels->pixels = pixels;
	outPixels->width = (uint32_t)width;
	outPixels->height = (uint32_t)height;
	outPixels->stride = (uint32_t)width * (uint32_t)ImageLoaderStbRequestedComponents;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static void ImageLoaderStbRelease(const ImageLoader *loader, ImagePixels *pixels) {
	stbi_image_free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderStbGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		// 854c10ca-79dc-4202-b064-586af81a96e0
		const ImageLoaderId id = { { 0x85, 0x4c, 0x10, 0xca, 0x79, 0xdc, 0x42, 0x02, 0xb0, 0x64, 0x58, 0x6a, 0xf8, 0x1a, 0x96, 0xe0 } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "stb_image";
		loader.version = "2.30";
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderStbExtensions;
		loader.probe = ImageLoaderStbProbe;
		loader.readInfo = ImageLoaderStbReadInfo;
		loader.decode = ImageLoaderStbDecode;
		loader.releasePixels = ImageLoaderStbRelease;

		// The global switches, set once here and nowhere else
		stbi_set_flip_vertically_on_load(0);
		stbi_set_unpremultiply_on_load(0);
		stbi_convert_iphone_png_to_rgb(0);
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_STB_IMPLEMENTATION
