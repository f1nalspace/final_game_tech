/*
Name:
	FPL_ImageViewer | HEIF loader

Description:
	Reads HEIF files, HEIC (HEVC), AVIF (AV1) and the other HEIF brands, through the libheif of the system.
	libheif is loaded at runtime when the loader is first requested, so the viewer builds and runs without it: the loader stays registered
	and reports every file as unsupported, with the missing library as the reason.
	libheif reads through the image source and only the parts of the file it needs. The primary image is decoded with its transformations
	(rotation, mirroring, cropping) applied, so the orientation is always normal, and higher bit depths come down to 8 bit per channel.
	Which codecs work depends on the plugins of the installed libheif (e.g. libde265 or ffmpeg for HEIC, dav1d or aom for AVIF),
	a missing one is reported as unsupported with the reason libheif gives. Color profiles are not applied, the pixels count as sRGB.

Usage:
	Include after imageloader.h. Define IMAGE_LOADER_HEIF_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_HEIF_H
#define IMAGE_LOADER_HEIF_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_HEIF_ID_TEXT "8fe6fddd-5246-435d-8665-64dbbdc8101f"

// The loader, filled on the first call, which also tries to load libheif
extern const ImageLoader *ImageLoaderHeifGet(void);

#endif // IMAGE_LOADER_HEIF_H

#if defined(IMAGE_LOADER_HEIF_IMPLEMENTATION) && !defined(IMAGE_LOADER_HEIF_IMPLEMENTED)
#define IMAGE_LOADER_HEIF_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const ImageLoaderHeifExtensions[] = { ".heic", ".heif", ".avif", NULL };

// The library names tried in this order, the versioned one first
#if defined(FPL_PLATFORM_WINDOWS)
static const char *const ImageLoaderHeifLibraryNames[] = { "libheif.dll", "heif.dll", NULL };
#else
static const char *const ImageLoaderHeifLibraryNames[] = { "libheif.so.1", "libheif.so", NULL };
#endif

// ISO base media file: the first box is ftyp with its size, the major brand, a minor version and the compatible brands
static const uint8_t ImageLoaderHeifFileTypeBox[] = { 'f', 't', 'y', 'p' };
static const size_t ImageLoaderHeifBoxSizeSize = 4;
static const size_t ImageLoaderHeifMajorBrandOffset = 8;
static const size_t ImageLoaderHeifCompatibleBrandsOffset = 16;
static const size_t ImageLoaderHeifBrandSize = 4;
static const char *const ImageLoaderHeifAvifBrands[] = { "avif", "avis", NULL };
static const char *const ImageLoaderHeifHeicBrands[] = { "heic", "heix", "heim", "heis", "hevc", "hevx", "hevm", "hevs", NULL };
static const char *const ImageLoaderHeifGenericBrands[] = { "mif1", "msf1", "mif2", NULL };

// Values of the libheif enums and error codes the loader uses, from libheif/heif_*.h
static const int ImageLoaderHeifColorspaceRgb = 1;
static const int ImageLoaderHeifChromaInterleavedRgba = 11;
static const int ImageLoaderHeifChannelInterleaved = 10;
static const int ImageLoaderHeifCompressionHevc = 1;
static const int ImageLoaderHeifCompressionAv1 = 4;
static const int ImageLoaderHeifErrorOk = 0;
static const int ImageLoaderHeifErrorUnsupportedFiletype = 3;
static const int ImageLoaderHeifErrorUnsupportedFeature = 4;
static const int ImageLoaderHeifErrorMemoryAllocation = 6;
static const int ImageLoaderHeifErrorPluginLoading = 11;
static const int ImageLoaderHeifErrorCanceled = 12;
static const int ImageLoaderHeifReaderApiVersion = 1;
static const int ImageLoaderHeifReaderSuccess = 0;
static const int ImageLoaderHeifReaderFailure = 1;
static const int ImageLoaderHeifGrowSizeReached = 0;
static const int ImageLoaderHeifGrowSizeBeyondEnd = 2;

static const uint32_t ImageLoaderHeifRgbChannels = 3;
static const uint32_t ImageLoaderHeifRgbaChannels = 4;
static const uint32_t ImageLoaderHeifBitsPerChannel = 8;

typedef enum ImageLoaderHeifKind {
	ImageLoaderHeifKind_None = 0,
	ImageLoaderHeifKind_Heif,
	ImageLoaderHeifKind_Heic,
	ImageLoaderHeifKind_Avif,
} ImageLoaderHeifKind;

// The types of libheif the loader uses, declared here so no libheif header is needed; the handles stay opaque
typedef struct ImageLoaderHeifContext ImageLoaderHeifContext;
typedef struct ImageLoaderHeifImageHandle ImageLoaderHeifImageHandle;
typedef struct ImageLoaderHeifImage ImageLoaderHeifImage;

// heif_error: code, subcode and a message owned by libheif, valid until the next call on the same object
typedef struct ImageLoaderHeifError {
	int code;
	int subcode;
	const char *message;
} ImageLoaderHeifError;

// heif_reader in version 1: read and seek return 0 on success, wait_for_file_size returns a heif_reader_grow_status
typedef struct ImageLoaderHeifReader {
	int readerApiVersion;
	int64_t (*getPosition)(void *userData);
	int (*read)(void *data, size_t size, void *userData);
	int (*seek)(int64_t position, void *userData);
	int (*waitForFileSize)(int64_t targetSize, void *userData);
} ImageLoaderHeifReader;

typedef const char *ImageLoaderHeifGetVersionFunction(void);
typedef ImageLoaderHeifError ImageLoaderHeifInitFunction(void *params);
typedef int ImageLoaderHeifHaveDecoderFunction(int compressionFormat);
typedef ImageLoaderHeifContext *ImageLoaderHeifContextAllocFunction(void);
typedef void ImageLoaderHeifContextFreeFunction(ImageLoaderHeifContext *context);
typedef ImageLoaderHeifError ImageLoaderHeifReadFromReaderFunction(ImageLoaderHeifContext *context, const ImageLoaderHeifReader *reader, void *userData, const void *options);
typedef ImageLoaderHeifError ImageLoaderHeifGetPrimaryHandleFunction(ImageLoaderHeifContext *context, ImageLoaderHeifImageHandle **outHandle);
typedef void ImageLoaderHeifHandleReleaseFunction(const ImageLoaderHeifImageHandle *handle);
typedef int ImageLoaderHeifHandleGetValueFunction(const ImageLoaderHeifImageHandle *handle);
typedef ImageLoaderHeifError ImageLoaderHeifDecodeImageFunction(const ImageLoaderHeifImageHandle *handle, ImageLoaderHeifImage **outImage, int colorspace, int chroma, const void *options);
typedef int ImageLoaderHeifImageGetSizeFunction(const ImageLoaderHeifImage *image, int channel);
typedef const uint8_t *ImageLoaderHeifGetPlaneFunction(const ImageLoaderHeifImage *image, int channel, int *outStride);
typedef const uint8_t *ImageLoaderHeifGetPlane2Function(const ImageLoaderHeifImage *image, int channel, size_t *outStride);
typedef void ImageLoaderHeifImageReleaseFunction(const ImageLoaderHeifImage *image);

typedef struct ImageLoaderHeifLibrary {
	fplDynamicLibraryHandle handle;
	ImageLoaderHeifGetVersionFunction *getVersion;
	// Optional, libheif 1.13 or newer: loads the codec plugins
	ImageLoaderHeifInitFunction *init;
	ImageLoaderHeifHaveDecoderFunction *haveDecoder;
	ImageLoaderHeifContextAllocFunction *contextAlloc;
	ImageLoaderHeifContextFreeFunction *contextFree;
	ImageLoaderHeifReadFromReaderFunction *readFromReader;
	ImageLoaderHeifGetPrimaryHandleFunction *getPrimaryHandle;
	ImageLoaderHeifHandleReleaseFunction *handleRelease;
	ImageLoaderHeifHandleGetValueFunction *handleGetWidth;
	ImageLoaderHeifHandleGetValueFunction *handleGetHeight;
	ImageLoaderHeifHandleGetValueFunction *handleHasAlpha;
	ImageLoaderHeifHandleGetValueFunction *handleGetLumaBits;
	ImageLoaderHeifDecodeImageFunction *decodeImage;
	ImageLoaderHeifImageGetSizeFunction *imageGetWidth;
	ImageLoaderHeifImageGetSizeFunction *imageGetHeight;
	// heif_image_get_plane_readonly2 (libheif 1.20) when present, the older one otherwise
	ImageLoaderHeifGetPlane2Function *getPlane2;
	ImageLoaderHeifGetPlaneFunction *getPlane;
	ImageLoaderHeifImageReleaseFunction *imageRelease;
	// Reads through the image source given as user data; libheif keeps a pointer to it as long as the context lives
	ImageLoaderHeifReader sourceReader;
	// The library file that was loaded, or the first name when none was found
	const char *libraryName;
	// "1.23.6 HEVC AV1": heif_get_version and the decoders found, "missing" without the library
	char versionText[64];
	bool isLoaded;
} ImageLoaderHeifLibrary;

// Loaded once in ImageLoaderHeifGet() and kept until the process ends, separate libheif contexts may decode in parallel
static ImageLoaderHeifLibrary imageLoaderHeif__library;

static int64_t ImageLoaderHeifReaderGetPosition(void *userData);
static int ImageLoaderHeifReaderRead(void *data, size_t size, void *userData);
static int ImageLoaderHeifReaderSeek(int64_t position, void *userData);
static int ImageLoaderHeifReaderWaitForFileSize(int64_t targetSize, void *userData);

static bool ImageLoaderHeifIsBrandInList(const uint8_t *brand, const char *const *brands) {
	for (size_t brandIndex = 0; brands[brandIndex] != NULL; ++brandIndex) {
		if (memcmp(brand, brands[brandIndex], ImageLoaderHeifBrandSize) == 0) {
			return(true);
		}
	}
	return(false);
}

// The kind from the brands of the ftyp box: any AVIF brand makes it AVIF, then any HEIC brand, then a generic HEIF brand
static ImageLoaderHeifKind ImageLoaderHeifParseKind(const uint8_t *header, const size_t headerSize) {
	bool hasFileType = headerSize >= ImageLoaderHeifCompatibleBrandsOffset && memcmp(header + ImageLoaderHeifBoxSizeSize, ImageLoaderHeifFileTypeBox, sizeof(ImageLoaderHeifFileTypeBox)) == 0;
	if (!hasFileType) {
		return(ImageLoaderHeifKind_None);
	}
	const uint32_t byteBits = 8;
	uint32_t boxSize = 0;
	for (size_t byteIndex = 0; byteIndex < ImageLoaderHeifBoxSizeSize; ++byteIndex) {
		boxSize = (boxSize << byteBits) | header[byteIndex];
	}
	size_t brandsEnd = boxSize < headerSize ? boxSize : headerSize;
	bool hasAvif = false;
	bool hasHeic = false;
	bool hasGeneric = false;
	// The major brand, then the compatible brands behind the minor version
	size_t offset = ImageLoaderHeifMajorBrandOffset;
	while (offset + ImageLoaderHeifBrandSize <= brandsEnd) {
		const uint8_t *brand = header + offset;
		hasAvif = hasAvif || ImageLoaderHeifIsBrandInList(brand, ImageLoaderHeifAvifBrands);
		hasHeic = hasHeic || ImageLoaderHeifIsBrandInList(brand, ImageLoaderHeifHeicBrands);
		hasGeneric = hasGeneric || ImageLoaderHeifIsBrandInList(brand, ImageLoaderHeifGenericBrands);
		offset = offset == ImageLoaderHeifMajorBrandOffset ? ImageLoaderHeifCompatibleBrandsOffset : offset + ImageLoaderHeifBrandSize;
	}
	if (hasAvif) {
		return(ImageLoaderHeifKind_Avif);
	}
	if (hasHeic) {
		return(ImageLoaderHeifKind_Heic);
	}
	if (hasGeneric) {
		return(ImageLoaderHeifKind_Heif);
	}
	return(ImageLoaderHeifKind_None);
}

static ImageLoaderHeifKind ImageLoaderHeifReadKind(ImageSource *source) {
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	source->seek(source, 0);
	size_t headerSize = source->read(source, header, sizeof(header));
	ImageLoaderHeifKind result = ImageLoaderHeifParseKind(header, headerSize);
	return(result);
}

static const char *ImageLoaderHeifGetKindName(const ImageLoaderHeifKind kind) {
	switch (kind) {
		case ImageLoaderHeifKind_Avif:
			return("AVIF");
		case ImageLoaderHeifKind_Heic:
			return("HEIC");
		default:
			return("HEIF");
	}
}

// --- Reader over the image source ------------------------------------------------------------------------------------------

static int64_t ImageLoaderHeifReaderGetPosition(void *userData) {
	ImageSource *source = (ImageSource *)userData;
	uint64_t position = source->tell(source);
	return((int64_t)position);
}

// A cancel fails the read, libheif then returns an error and the loader reports the cancel
static int ImageLoaderHeifReaderRead(void *data, size_t size, void *userData) {
	ImageSource *source = (ImageSource *)userData;
	uint8_t *target = (uint8_t *)data;
	size_t position = 0;
	while (position < size) {
		if (source->isCanceled(source)) {
			return(ImageLoaderHeifReaderFailure);
		}
		size_t readSize = source->read(source, target + position, size - position);
		if (readSize == 0) {
			return(ImageLoaderHeifReaderFailure);
		}
		position += readSize;
	}
	return(ImageLoaderHeifReaderSuccess);
}

static int ImageLoaderHeifReaderSeek(int64_t position, void *userData) {
	ImageSource *source = (ImageSource *)userData;
	if (position < 0 || !source->seek(source, (uint64_t)position)) {
		return(ImageLoaderHeifReaderFailure);
	}
	return(ImageLoaderHeifReaderSuccess);
}

// The file never grows: everything up to its size is there
static int ImageLoaderHeifReaderWaitForFileSize(int64_t targetSize, void *userData) {
	ImageSource *source = (ImageSource *)userData;
	uint64_t sourceSize = source->size(source);
	bool isReached = targetSize >= 0 && (uint64_t)targetSize <= sourceSize;
	int result = isReached ? ImageLoaderHeifGrowSizeReached : ImageLoaderHeifGrowSizeBeyondEnd;
	return(result);
}

// --- Library ---------------------------------------------------------------------------------------------------------------

static void *ImageLoaderHeifGetProc(ImageLoaderHeifLibrary *library, const char *name) {
	void *result = fplGetDynamicLibraryProc(&library->handle, name);
	return(result);
}

static void ImageLoaderHeifLoadLibrary(ImageLoaderHeifLibrary *library) {
	memset(library, 0, sizeof(*library));
	library->sourceReader.readerApiVersion = ImageLoaderHeifReaderApiVersion;
	library->sourceReader.getPosition = ImageLoaderHeifReaderGetPosition;
	library->sourceReader.read = ImageLoaderHeifReaderRead;
	library->sourceReader.seek = ImageLoaderHeifReaderSeek;
	library->sourceReader.waitForFileSize = ImageLoaderHeifReaderWaitForFileSize;
	library->libraryName = ImageLoaderHeifLibraryNames[0];
	for (size_t nameIndex = 0; ImageLoaderHeifLibraryNames[nameIndex] != NULL; ++nameIndex) {
		const char *libraryName = ImageLoaderHeifLibraryNames[nameIndex];
		if (!fplDynamicLibraryLoad(libraryName, &library->handle)) {
			continue;
		}
		library->getVersion = (ImageLoaderHeifGetVersionFunction *)ImageLoaderHeifGetProc(library, "heif_get_version");
		library->init = (ImageLoaderHeifInitFunction *)ImageLoaderHeifGetProc(library, "heif_init");
		library->haveDecoder = (ImageLoaderHeifHaveDecoderFunction *)ImageLoaderHeifGetProc(library, "heif_have_decoder_for_format");
		library->contextAlloc = (ImageLoaderHeifContextAllocFunction *)ImageLoaderHeifGetProc(library, "heif_context_alloc");
		library->contextFree = (ImageLoaderHeifContextFreeFunction *)ImageLoaderHeifGetProc(library, "heif_context_free");
		library->readFromReader = (ImageLoaderHeifReadFromReaderFunction *)ImageLoaderHeifGetProc(library, "heif_context_read_from_reader");
		library->getPrimaryHandle = (ImageLoaderHeifGetPrimaryHandleFunction *)ImageLoaderHeifGetProc(library, "heif_context_get_primary_image_handle");
		library->handleRelease = (ImageLoaderHeifHandleReleaseFunction *)ImageLoaderHeifGetProc(library, "heif_image_handle_release");
		library->handleGetWidth = (ImageLoaderHeifHandleGetValueFunction *)ImageLoaderHeifGetProc(library, "heif_image_handle_get_width");
		library->handleGetHeight = (ImageLoaderHeifHandleGetValueFunction *)ImageLoaderHeifGetProc(library, "heif_image_handle_get_height");
		library->handleHasAlpha = (ImageLoaderHeifHandleGetValueFunction *)ImageLoaderHeifGetProc(library, "heif_image_handle_has_alpha_channel");
		library->handleGetLumaBits = (ImageLoaderHeifHandleGetValueFunction *)ImageLoaderHeifGetProc(library, "heif_image_handle_get_luma_bits_per_pixel");
		library->decodeImage = (ImageLoaderHeifDecodeImageFunction *)ImageLoaderHeifGetProc(library, "heif_decode_image");
		library->imageGetWidth = (ImageLoaderHeifImageGetSizeFunction *)ImageLoaderHeifGetProc(library, "heif_image_get_width");
		library->imageGetHeight = (ImageLoaderHeifImageGetSizeFunction *)ImageLoaderHeifGetProc(library, "heif_image_get_height");
		library->getPlane2 = (ImageLoaderHeifGetPlane2Function *)ImageLoaderHeifGetProc(library, "heif_image_get_plane_readonly2");
		library->getPlane = (ImageLoaderHeifGetPlaneFunction *)ImageLoaderHeifGetProc(library, "heif_image_get_plane_readonly");
		library->imageRelease = (ImageLoaderHeifImageReleaseFunction *)ImageLoaderHeifGetProc(library, "heif_image_release");
		bool hasPlaneFunction = library->getPlane2 != NULL || library->getPlane != NULL;
		bool hasContextFunctions = library->contextAlloc != NULL && library->contextFree != NULL && library->readFromReader != NULL && library->getPrimaryHandle != NULL;
		bool hasHandleFunctions = library->handleRelease != NULL && library->handleGetWidth != NULL && library->handleGetHeight != NULL && library->handleHasAlpha != NULL && library->handleGetLumaBits != NULL;
		bool hasImageFunctions = library->decodeImage != NULL && library->imageGetWidth != NULL && library->imageGetHeight != NULL && library->imageRelease != NULL && hasPlaneFunction;
		bool hasAllFunctions = library->getVersion != NULL && library->haveDecoder != NULL && hasContextFunctions && hasHandleFunctions && hasImageFunctions;
		if (!hasAllFunctions) {
			fplDynamicLibraryUnload(&library->handle);
			continue;
		}
		// The codec plugins are only loaded by heif_init, which stays in effect until the process ends
		if (library->init != NULL) {
			library->init(NULL);
		}
		const char *version = library->getVersion();
		bool hasHevc = library->haveDecoder(ImageLoaderHeifCompressionHevc) != 0;
		bool hasAv1 = library->haveDecoder(ImageLoaderHeifCompressionAv1) != 0;
		const char *hevcText = hasHevc ? " HEVC" : "";
		const char *av1Text = hasAv1 ? " AV1" : "";
		const char *noDecoderText = !hasHevc && !hasAv1 ? " no decoder" : "";
		snprintf(library->versionText, sizeof(library->versionText), "%s%s%s%s", version, hevcText, av1Text, noDecoderText);
		library->libraryName = libraryName;
		library->isLoaded = true;
		return;
	}
	snprintf(library->versionText, sizeof(library->versionText), "missing");
}

// --- Loader ----------------------------------------------------------------------------------------------------------------

static ImageLoadResult ImageLoaderHeifGetErrorResult(const ImageLoaderHeifError error) {
	if (error.code == ImageLoaderHeifErrorUnsupportedFiletype || error.code == ImageLoaderHeifErrorUnsupportedFeature || error.code == ImageLoaderHeifErrorPluginLoading) {
		return(ImageLoadResult_Unsupported);
	}
	if (error.code == ImageLoaderHeifErrorMemoryAllocation) {
		return(ImageLoadResult_OutOfMemory);
	}
	if (error.code == ImageLoaderHeifErrorCanceled) {
		return(ImageLoadResult_Canceled);
	}
	return(ImageLoadResult_Corrupt);
}

// A failed read caused by a cancel counts as cancel, everything else keeps the reason of libheif
static ImageLoadResult ImageLoaderHeifReportError(ImageSource *source, const ImageLoaderHeifError error, char *message, size_t messageSize) {
	if (source->isCanceled(source)) {
		snprintf(message, messageSize, "canceled");
		return(ImageLoadResult_Canceled);
	}
	snprintf(message, messageSize, "libheif: %s", error.message);
	ImageLoadResult result = ImageLoaderHeifGetErrorResult(error);
	return(result);
}

typedef struct ImageLoaderHeifFile {
	ImageLoaderHeifContext *context;
	ImageLoaderHeifImageHandle *handle;
} ImageLoaderHeifFile;

static void ImageLoaderHeifClose(const ImageLoaderHeifLibrary *library, ImageLoaderHeifFile *file) {
	if (file->handle != NULL) {
		library->handleRelease(file->handle);
	}
	if (file->context != NULL) {
		library->contextFree(file->context);
	}
	memset(file, 0, sizeof(*file));
}

// Reads the boxes of the file and opens its primary image
static ImageLoadResult ImageLoaderHeifOpen(const ImageLoaderHeifLibrary *library, ImageSource *source, ImageLoaderHeifFile *outFile, char *message, size_t messageSize) {
	memset(outFile, 0, sizeof(*outFile));
	if (!library->isLoaded) {
		snprintf(message, messageSize, "%s not found", library->libraryName);
		return(ImageLoadResult_Unsupported);
	}
	outFile->context = library->contextAlloc();
	if (outFile->context == NULL) {
		snprintf(message, messageSize, "no memory for a libheif context");
		return(ImageLoadResult_OutOfMemory);
	}
	source->seek(source, 0);
	ImageLoaderHeifError readError = library->readFromReader(outFile->context, &library->sourceReader, source, NULL);
	if (readError.code != ImageLoaderHeifErrorOk) {
		ImageLoadResult result = ImageLoaderHeifReportError(source, readError, message, messageSize);
		ImageLoaderHeifClose(library, outFile);
		return(result);
	}
	ImageLoaderHeifError handleError = library->getPrimaryHandle(outFile->context, &outFile->handle);
	if (handleError.code != ImageLoaderHeifErrorOk) {
		ImageLoadResult result = ImageLoaderHeifReportError(source, handleError, message, messageSize);
		ImageLoaderHeifClose(library, outFile);
		return(result);
	}
	return(ImageLoadResult_Success);
}

static ImageLoaderMatch ImageLoaderHeifProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	ImageLoaderHeifKind kind = ImageLoaderHeifParseKind(header, headerSize);
	if (kind != ImageLoaderHeifKind_None) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderHeifReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	const ImageLoaderHeifLibrary *library = &imageLoaderHeif__library;
	ImageLoaderHeifKind kind = ImageLoaderHeifReadKind(source);
	if (kind == ImageLoaderHeifKind_None) {
		snprintf(message, messageSize, "no HEIF file type box");
		return(ImageLoadResult_Corrupt);
	}
	ImageLoaderHeifFile file;
	ImageLoadResult openResult = ImageLoaderHeifOpen(library, source, &file, message, messageSize);
	if (openResult != ImageLoadResult_Success) {
		return(openResult);
	}
	// The size as signalled with the transformations applied, the decoded size is what counts for the pixels
	int width = library->handleGetWidth(file.handle);
	int height = library->handleGetHeight(file.handle);
	bool hasAlpha = library->handleHasAlpha(file.handle) != 0;
	int lumaBits = library->handleGetLumaBits(file.handle);
	ImageLoaderHeifClose(library, &file);
	if (width <= 0 || height <= 0) {
		snprintf(message, messageSize, "image size %d x %d", width, height);
		return(ImageLoadResult_Corrupt);
	}
	uint32_t channelCount = hasAlpha ? ImageLoaderHeifRgbaChannels : ImageLoaderHeifRgbChannels;
	uint32_t bitsPerChannel = lumaBits > 0 ? (uint32_t)lumaBits : ImageLoaderHeifBitsPerChannel;
	outInfo->width = (uint32_t)width;
	outInfo->height = (uint32_t)height;
	outInfo->channelCount = channelCount;
	outInfo->bitsPerPixel = channelCount * bitsPerChannel;
	outInfo->formatName = ImageLoaderHeifGetKindName(kind);
	outInfo->orientation = ImageOrientation_Normal;
	outInfo->isPalette = false;
	return(ImageLoadResult_Success);
}

// Copies the interleaved RGBA plane into rows without padding, owned by the loader
static ImageLoadResult ImageLoaderHeifCopyPixels(const ImageLoaderHeifLibrary *library, const ImageLoaderHeifImage *image, ImagePixels *outPixels, char *message, size_t messageSize) {
	int width = library->imageGetWidth(image, ImageLoaderHeifChannelInterleaved);
	int height = library->imageGetHeight(image, ImageLoaderHeifChannelInterleaved);
	const uint8_t *plane = NULL;
	size_t planeStride = 0;
	if (library->getPlane2 != NULL) {
		plane = library->getPlane2(image, ImageLoaderHeifChannelInterleaved, &planeStride);
	} else {
		int stride = 0;
		plane = library->getPlane(image, ImageLoaderHeifChannelInterleaved, &stride);
		planeStride = stride > 0 ? (size_t)stride : 0;
	}
	size_t rowSize = (size_t)width * ImageLoaderHeifRgbaChannels;
	if (plane == NULL || width <= 0 || height <= 0 || planeStride < rowSize) {
		snprintf(message, messageSize, "libheif returned no RGBA plane");
		return(ImageLoadResult_Corrupt);
	}
	size_t pixelsSize = rowSize * (size_t)height;
	uint8_t *pixels = (uint8_t *)malloc(pixelsSize);
	if (pixels == NULL) {
		snprintf(message, messageSize, "no memory for %zu bytes", pixelsSize);
		return(ImageLoadResult_OutOfMemory);
	}
	for (int row = 0; row < height; ++row) {
		const uint8_t *sourceRow = plane + (size_t)row * planeStride;
		uint8_t *targetRow = pixels + (size_t)row * rowSize;
		memcpy(targetRow, sourceRow, rowSize);
	}
	outPixels->pixels = pixels;
	outPixels->width = (uint32_t)width;
	outPixels->height = (uint32_t)height;
	outPixels->stride = (uint32_t)rowSize;
	outPixels->format = ImagePixelFormat_RGBA8_SRGB;
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderHeifDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	const ImageLoaderHeifLibrary *library = &imageLoaderHeif__library;
	ImageLoaderHeifFile file;
	ImageLoadResult openResult = ImageLoaderHeifOpen(library, source, &file, message, messageSize);
	if (openResult != ImageLoadResult_Success) {
		return(openResult);
	}
	// Default options: transformations applied, straight alpha, 8 bit per channel
	ImageLoaderHeifImage *image = NULL;
	ImageLoaderHeifError decodeError = library->decodeImage(file.handle, &image, ImageLoaderHeifColorspaceRgb, ImageLoaderHeifChromaInterleavedRgba, NULL);
	if (decodeError.code != ImageLoaderHeifErrorOk) {
		ImageLoadResult result = ImageLoaderHeifReportError(source, decodeError, message, messageSize);
		if (image != NULL) {
			library->imageRelease(image);
		}
		ImageLoaderHeifClose(library, &file);
		return(result);
	}
	ImageLoadResult result = ImageLoadResult_Canceled;
	if (source->isCanceled(source)) {
		snprintf(message, messageSize, "canceled");
	} else {
		result = ImageLoaderHeifCopyPixels(library, image, outPixels, message, messageSize);
	}
	library->imageRelease(image);
	ImageLoaderHeifClose(library, &file);
	return(result);
}

static void ImageLoaderHeifRelease(const ImageLoader *loader, ImagePixels *pixels) {
	free(pixels->pixels);
}

extern const ImageLoader *ImageLoaderHeifGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		ImageLoaderHeifLoadLibrary(&imageLoaderHeif__library);
		// 8fe6fddd-5246-435d-8665-64dbbdc8101f
		const ImageLoaderId id = { { 0x8f, 0xe6, 0xfd, 0xdd, 0x52, 0x46, 0x43, 0x5d, 0x86, 0x65, 0x64, 0xdb, 0xbd, 0xc8, 0x10, 0x1f } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "libheif";
		loader.version = imageLoaderHeif__library.versionText;
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderHeifExtensions;
		loader.probe = ImageLoaderHeifProbe;
		loader.readInfo = ImageLoaderHeifReadInfo;
		loader.decode = ImageLoaderHeifDecode;
		loader.releasePixels = ImageLoaderHeifRelease;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_HEIF_IMPLEMENTATION
