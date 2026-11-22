/*
Name:
	FPL_ImageViewer | stb_image loader

Description:
	The default loader: JPEG, PNG, BMP and TGA through stb_image. A look at the file header adds what stb_image does not report:
	the bit depth and palette of PNG, BMP and TGA files and the EXIF orientation of JPEG files.
	TGA has no signature, it is picked by its extension.
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

static const char *const ImageLoaderStbExtensions[] = { ".jpg", ".jpeg", ".png", ".bmp", ".tga", NULL };
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
// TGA: the 18 byte header, the same fields stb_image tests before it reads a TGA
static const size_t ImageLoaderStbTgaHeaderSize = 18;
static const size_t ImageLoaderStbTgaColorMapTypeOffset = 1;
static const size_t ImageLoaderStbTgaImageTypeOffset = 2;
static const size_t ImageLoaderStbTgaPixelDepthOffset = 16;
static const uint8_t ImageLoaderStbTgaImageTypePalette = 1;
static const uint8_t ImageLoaderStbTgaImageTypeTrueColor = 2;
static const uint8_t ImageLoaderStbTgaImageTypeGray = 3;
// Run length encoded variants are the plain types plus this
static const uint8_t ImageLoaderStbTgaImageTypeRleOffset = 8;
static const uint8_t ImageLoaderStbTgaColorMapPresent = 1;

typedef enum ImageLoaderStbFormat {
	ImageLoaderStbFormat_Unknown = 0,
	ImageLoaderStbFormat_Jpeg,
	ImageLoaderStbFormat_Png,
	ImageLoaderStbFormat_Bmp,
	ImageLoaderStbFormat_Tga,
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

// The base image type of a TGA header (palette, true color or gray), 0 when the header is no TGA stb_image reads
static uint8_t ImageLoaderStbTgaBaseImageType(const uint8_t *header, const size_t headerSize) {
	if (headerSize < ImageLoaderStbTgaHeaderSize) {
		return(0);
	}
	uint8_t colorMapType = header[ImageLoaderStbTgaColorMapTypeOffset];
	uint8_t imageType = header[ImageLoaderStbTgaImageTypeOffset];
	uint8_t baseImageType = imageType > ImageLoaderStbTgaImageTypeRleOffset ? (uint8_t)(imageType - ImageLoaderStbTgaImageTypeRleOffset) : imageType;
	bool isKnownType = baseImageType == ImageLoaderStbTgaImageTypePalette || baseImageType == ImageLoaderStbTgaImageTypeTrueColor || baseImageType == ImageLoaderStbTgaImageTypeGray;
	bool isPaletteType = baseImageType == ImageLoaderStbTgaImageTypePalette;
	// A palette image needs a color map, the other types may only carry an unused one
	bool isColorMapValid = isPaletteType ? colorMapType == ImageLoaderStbTgaColorMapPresent : colorMapType <= ImageLoaderStbTgaColorMapPresent;
	bool result = isKnownType && isColorMapValid;
	return(result ? baseImageType : 0);
}

// Signatures first; without one, a TGA header counts only for a file stb_image was picked for by its extension
static ImageLoaderStbFormat ImageLoaderStbDetectWithTga(const uint8_t *header, const size_t headerSize) {
	ImageLoaderStbFormat format = ImageLoaderStbDetect(header, headerSize);
	if (format == ImageLoaderStbFormat_Unknown && ImageLoaderStbTgaBaseImageType(header, headerSize) != 0) {
		format = ImageLoaderStbFormat_Tga;
	}
	return(format);
}

// stb_image reads a whole PNG before it inflates it, so reading counts for this share of the progress and the rest is reported as indeterminate
static const float ImageLoaderStbPngReadShare = 0.25f;

// What the stb_image callbacks read from
typedef struct ImageLoaderStbStream {
	ImageSource *source;
	// Reports the PNG progress itself, see ImageLoaderStbPngReadShare
	bool isPngProgress;
} ImageLoaderStbStream;

static int ImageLoaderStbRead(void *user, char *data, int size) {
	ImageLoaderStbStream *stream = (ImageLoaderStbStream *)user;
	ImageSource *source = stream->source;
	size_t readSize = source->read(source, data, (size_t)size);
	if (stream->isPngProgress) {
		uint64_t position = source->tell(source);
		uint64_t sourceSize = source->size(source);
		float readFraction = sourceSize > 0 ? (float)((double)position / (double)sourceSize) : 1.0f;
		bool isEverythingRead = position >= sourceSize;
		source->reportProgress(source, readFraction * ImageLoaderStbPngReadShare, isEverythingRead);
	}
	return((int)readSize);
}

// stb_image skips forward and, with a negative count, back
static void ImageLoaderStbSkip(void *user, int count) {
	ImageLoaderStbStream *stream = (ImageLoaderStbStream *)user;
	ImageSource *source = stream->source;
	int64_t position = (int64_t)source->tell(source);
	int64_t size = (int64_t)source->size(source);
	int64_t target = position + count;
	int64_t clamped = target < 0 ? 0 : (target > size ? size : target);
	source->seek(source, (uint64_t)clamped);
}

static int ImageLoaderStbEof(void *user) {
	ImageLoaderStbStream *stream = (ImageLoaderStbStream *)user;
	ImageSource *source = stream->source;
	uint64_t position = source->tell(source);
	uint64_t size = source->size(source);
	bool isEnd = position >= size || source->isCanceled(source);
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
	ImageLoaderStbStream stream = { source, false };
	int width = 0;
	int height = 0;
	int components = 0;
	if (!stbi_info_from_callbacks(&callbacks, &stream, &width, &height, &components)) {
		ImageLoadResult failure = ImageLoaderStbFailure(source, message, messageSize);
		return(failure);
	}
	source->seek(source, 0);
	int is16Bit = stbi_is_16_bit_from_callbacks(&callbacks, &stream);
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
	ImageLoaderStbFormat format = ImageLoaderStbDetectWithTga(header, headerSize);
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
		case ImageLoaderStbFormat_Tga:
		{
			// The stored depth: 16 for 5-5-5-1 pixels, the index size for a palette image
			outInfo->formatName = "TGA";
			outInfo->bitsPerPixel = header[ImageLoaderStbTgaPixelDepthOffset];
			uint8_t baseImageType = ImageLoaderStbTgaBaseImageType(header, headerSize);
			outInfo->isPalette = baseImageType == ImageLoaderStbTgaImageTypePalette;
		} break;
		default:
		{
			outInfo->formatName = "Unknown";
		} break;
	}
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderStbDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	// JPEG decodes while reading, so the read position is a good progress, PNG does not
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	size_t headerSize = source->read(source, header, sizeof(header));
	source->seek(source, 0);
	ImageLoaderStbFormat format = ImageLoaderStbDetect(header, headerSize);
	stbi_io_callbacks callbacks = ImageLoaderStbCallbacks();
	ImageLoaderStbStream stream = { source, format == ImageLoaderStbFormat_Png };
	int width = 0;
	int height = 0;
	int components = 0;
	stbi_uc *pixels = stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &components, ImageLoaderStbRequestedComponents);
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
