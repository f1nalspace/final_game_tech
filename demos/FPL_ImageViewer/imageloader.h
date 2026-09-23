/*
Name:
	FPL_ImageViewer | Image loaders

Description:
	Pictures are read by exchangeable loaders. Every loader carries a fixed 128-bit id (a random UUID v4, generated once and never changed),
	name and version are for display only. Settings, command line and log refer to the id.

	A loader never opens a file itself, it reads from an ImageSource (read, seek, tell, size, cancel, progress),
	so progress and cancel work the same for every loader and other sources (memory, archives) need no loader change.

	The registry keeps the loaders in order, the first one is the default. For a file it asks every loader to probe the first bytes
	(signature beats extension, ties go by order), tries a pinned loader first, falls back to the next candidate when a loader
	returns Unsupported or Corrupt, and serializes loaders that are not thread-safe.

	The interface is plain C with interfaceVersion and structSize, so a loader can later come from a shared library without changes.

	Also here: a bounds checked EXIF reader for the orientation tag of JPEG files, shared by all JPEG loaders.

Usage:
	Needs final_platform_layer.h (mutex, files). Define IMAGE_LOADER_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <final_platform_layer.h>

// An unknown version is rejected at registration
#define IMAGE_LOADER_INTERFACE_VERSION 1
// Bytes of the file start every loader gets to probe
#define IMAGE_LOADER_PROBE_SIZE 64
#define IMAGE_LOADER_ID_SIZE 16
// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx plus terminator
#define IMAGE_LOADER_ID_TEXT_SIZE 37
#define IMAGE_LOADER_MAX_COUNT 16
#define IMAGE_LOADER_MAX_EXTENSION_PINS 16
#define IMAGE_LOADER_MAX_EXTENSION_LENGTH 16
// Size of the message buffers loaders write their reasons into
#define IMAGE_LOADER_MESSAGE_SIZE 256

// Byte order is the order of the text form (RFC 4122), not the Windows GUID layout
typedef struct ImageLoaderId {
	uint8_t bytes[IMAGE_LOADER_ID_SIZE];
} ImageLoaderId;

typedef enum ImageLoaderFlags {
	ImageLoaderFlags_None = 0,
	// Calls of this loader are serialized by the registry
	ImageLoaderFlags_NotThreadSafe = 1 << 0,
} ImageLoaderFlags;

typedef enum ImageLoaderMatch {
	ImageLoaderMatch_None = 0,
	// Only the file extension fits
	ImageLoaderMatch_Extension,
	// The magic bytes fit, regardless of the extension
	ImageLoaderMatch_Signature,
} ImageLoaderMatch;

typedef enum ImageLoadResult {
	ImageLoadResult_Success = 0,
	// This loader does not read this file or this variant of the format
	ImageLoadResult_Unsupported,
	ImageLoadResult_Corrupt,
	ImageLoadResult_OutOfMemory,
	ImageLoadResult_Canceled,
} ImageLoadResult;

typedef enum ImagePixelFormat {
	// 8 bit per channel, sRGB color, straight alpha, top row first. RGBA16 and RGBA32F are reserved for later.
	ImagePixelFormat_RGBA8_SRGB = 0,
} ImagePixelFormat;

// EXIF orientation: how the stored pixels must be turned to be upright
typedef enum ImageOrientation {
	ImageOrientation_Normal = 1,
	ImageOrientation_MirrorHorizontal = 2,
	ImageOrientation_Rotate180 = 3,
	ImageOrientation_MirrorVertical = 4,
	ImageOrientation_Transpose = 5,
	ImageOrientation_Rotate90 = 6,
	ImageOrientation_Transverse = 7,
	ImageOrientation_Rotate270 = 8,
} ImageOrientation;

typedef struct PictureInfo {
	// As stored, before the orientation is applied
	uint32_t width;
	uint32_t height;
	uint32_t channelCount;
	uint32_t bitsPerPixel;
	// Static text, e.g. "JPEG"
	const char *formatName;
	ImageOrientation orientation;
	bool isPalette;
} PictureInfo;

// Owned by the loader that decoded it, handed back through its releasePixels
typedef struct ImagePixels {
	void *pixels;
	uint32_t width;
	uint32_t height;
	// Bytes per row
	uint32_t stride;
	ImagePixelFormat format;
} ImagePixels;

typedef struct ImageSource ImageSource;
struct ImageSource {
	// Reads up to size bytes and returns how many were read, 0 at the end or on errors
	size_t (*read)(ImageSource *source, void *buffer, size_t size);
	bool (*seek)(ImageSource *source, uint64_t position);
	uint64_t (*tell)(ImageSource *source);
	uint64_t (*size)(ImageSource *source);
	bool (*isCanceled)(ImageSource *source);
	// For a loader that knows better than the read position: fraction 0..1 of reading and decoding, from the first report on the read position no longer counts.
	// isIndeterminate: the loader keeps working past fraction without knowing how far it is, e.g. inflating a PNG after the whole file is read.
	void (*reportProgress)(ImageSource *source, float fraction, bool isIndeterminate);
	void *userData;
};

// Gets the progress of a source: the read position, or what the loader reports
typedef void ImageSourceProgressFunction(void *userData, const float fraction, const bool isIndeterminate);

typedef struct ImageLoader ImageLoader;
typedef ImageLoaderMatch ImageLoaderProbeFunction(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension);
typedef ImageLoadResult ImageLoaderReadInfoFunction(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize);
typedef ImageLoadResult ImageLoaderDecodeFunction(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize);
typedef void ImageLoaderReleaseFunction(const ImageLoader *loader, ImagePixels *pixels);

struct ImageLoader {
	// IMAGE_LOADER_INTERFACE_VERSION the loader was built against
	uint32_t interfaceVersion;
	// sizeof(ImageLoader) as the loader was compiled with
	uint32_t structSize;
	// The signature of the loader, never changes
	ImageLoaderId id;
	// Display only, e.g. "stb_image"
	const char *name;
	// Display only, e.g. "2.30"
	const char *version;
	ImageLoaderFlags flags;
	// Null terminated list, lower case with dot, e.g. ".jpg"
	const char *const *fileExtensions;
	ImageLoaderProbeFunction *probe;
	ImageLoaderReadInfoFunction *readInfo;
	ImageLoaderDecodeFunction *decode;
	ImageLoaderReleaseFunction *releasePixels;
	void *userData;
};

typedef void ImageLoaderLogFunction(const char *message);

typedef struct ImageLoaderEntry {
	const ImageLoader *loader;
	fplMutexHandle mutex;
	bool hasMutex;
} ImageLoaderEntry;

typedef struct ImageLoaderExtensionPin {
	char extension[IMAGE_LOADER_MAX_EXTENSION_LENGTH];
	int32_t entryIndex;
} ImageLoaderExtensionPin;

// Entries keep their index from registration on (the index names a loader in this process), order[] is the order of the selection
typedef struct ImageLoaderRegistry {
	ImageLoaderEntry entries[IMAGE_LOADER_MAX_COUNT];
	int32_t order[IMAGE_LOADER_MAX_COUNT];
	uint32_t count;
	// Tried first for every file it probes, -1 when none
	int32_t globalPinIndex;
	ImageLoaderExtensionPin extensionPins[IMAGE_LOADER_MAX_EXTENSION_PINS];
	uint32_t extensionPinCount;
	bool isFallbackEnabled;
	// Optional, gets one line per failed attempt
	ImageLoaderLogFunction *log;
} ImageLoaderRegistry;

// File backed source, the first member makes it usable as ImageSource
typedef struct ImageFileSource {
	ImageSource source;
	fplFileHandle file;
	uint64_t fileSize;
	// Optional: canceled when it turns true
	volatile bool *cancelFlag;
	// Optional: gets the read position until the loader reports its own progress
	ImageSourceProgressFunction *progress;
	void *progressUserData;
	bool hasLoaderProgress;
} ImageFileSource;

// Memory backed source for tests and embedded data
typedef struct ImageMemorySource {
	ImageSource source;
	const uint8_t *data;
	uint64_t dataSize;
	uint64_t position;
} ImageMemorySource;

extern bool ImageLoaderIdParse(const char *text, ImageLoaderId *outId);
// Lower case text form, bufferSize must be at least IMAGE_LOADER_ID_TEXT_SIZE
extern void ImageLoaderIdFormat(const ImageLoaderId *id, char *buffer, const size_t bufferSize);
extern bool ImageLoaderIdIsEqual(const ImageLoaderId *a, const ImageLoaderId *b);

extern const char *ImageLoadResultGetName(const ImageLoadResult result);
// True when the loader lists the extension (lower case with dot, as probe gets it)
extern bool ImageLoaderListsExtension(const ImageLoader *loader, const char *normalizedExtension);

// Orientation tag (0x0112) of a TIFF block as found in an EXIF APP1 segment, Normal when missing or broken; never reads outside the block
extern ImageOrientation ImageExifReadOrientation(const uint8_t *tiff, const size_t tiffSize);
// Walks the JPEG segments up to the first scan and reads the orientation of the first EXIF APP1 segment, Normal otherwise
extern ImageOrientation ImageJpegReadOrientation(ImageSource *source);

extern bool ImageFileSourceOpen(ImageFileSource *fileSource, const char *filePath, volatile bool *cancelFlag, ImageSourceProgressFunction *progress, void *progressUserData);
extern void ImageFileSourceClose(ImageFileSource *fileSource);
extern void ImageMemorySourceInit(ImageMemorySource *memorySource, const void *data, const size_t dataSize);

extern void ImageLoaderRegistryInit(ImageLoaderRegistry *registry);
extern void ImageLoaderRegistryRelease(ImageLoaderRegistry *registry);
// Rejects an unknown interface version, a smaller struct and a duplicate id, appends the loader to the order
extern bool ImageLoaderRegistryAdd(ImageLoaderRegistry *registry, const ImageLoader *loader, char *message, const size_t messageSize);
// Entry index of the loader with this id (text form) or this unique name (ignoring case), -1 with the reason in message otherwise
extern int32_t ImageLoaderRegistryFind(const ImageLoaderRegistry *registry, const char *idOrName, char *message, const size_t messageSize);
extern const ImageLoader *ImageLoaderRegistryGet(const ImageLoaderRegistry *registry, const int32_t entryIndex);
// Moves these entries to the front in this order, all others keep their order behind them
extern bool ImageLoaderRegistrySetOrder(ImageLoaderRegistry *registry, const int32_t *entryIndices, const uint32_t count);
extern void ImageLoaderRegistryPinGlobal(ImageLoaderRegistry *registry, const int32_t entryIndex);
// Extension with or without dot, any case
extern bool ImageLoaderRegistryPinExtension(ImageLoaderRegistry *registry, const char *extension, const int32_t entryIndex);
// True when any loader lists this extension (with dot, any case)
extern bool ImageLoaderRegistryIsKnownExtension(const ImageLoaderRegistry *registry, const char *extension);
// Candidates for a file, best first: a pinned loader, then signature matches, then extension matches, each group in registry order.
// Without fallback only the first candidate is returned. Loaders that probe None are never candidates.
extern uint32_t ImageLoaderRegistrySelect(const ImageLoaderRegistry *registry, const uint8_t *header, const size_t headerSize, const char *fileExtension, int32_t *outEntries, const uint32_t maxEntries);
// Reads the first bytes of the source and selects the candidates for it; with ignoreFallbackSetting every candidate is returned even without fallback
extern uint32_t ImageLoaderRegistrySelectForSource(const ImageLoaderRegistry *registry, ImageSource *source, const char *fileExtension, const bool ignoreFallbackSetting, int32_t *outEntries, const uint32_t maxEntries);

extern ImageLoadResult ImageLoaderRegistryReadInfo(ImageLoaderRegistry *registry, const int32_t entryIndex, ImageSource *source, PictureInfo *outInfo, char *message, const size_t messageSize);
extern ImageLoadResult ImageLoaderRegistryDecode(ImageLoaderRegistry *registry, const int32_t entryIndex, ImageSource *source, ImagePixels *outPixels, char *message, const size_t messageSize);
extern void ImageLoaderRegistryReleasePixels(ImageLoaderRegistry *registry, const int32_t entryIndex, ImagePixels *pixels);

// Selects the candidates and tries them in turn (info, then pixels) until one succeeds.
// A forced entry (>= 0) is tried alone, without fallback. On success outEntry names the loader that owns the pixels.
extern ImageLoadResult ImageLoaderRegistryLoad(ImageLoaderRegistry *registry, ImageSource *source, const char *fileExtension, const int32_t forcedEntry, int32_t *outEntry, PictureInfo *outInfo, ImagePixels *outPixels, char *message, const size_t messageSize);

#endif // IMAGE_LOADER_H

#if defined(IMAGE_LOADER_IMPLEMENTATION) && !defined(IMAGE_LOADER_IMPLEMENTED)
#define IMAGE_LOADER_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Text form: groups of 8-4-4-4-12 hex digits, dashes after these byte counts
static const int ImageLoaderIdDashAfterBytes[] = { 4, 6, 8, 10 };
static const char ImageLoaderHexDigits[] = "0123456789abcdef";
static const int ImageLoaderBitsPerNibble = 4;
static const uint8_t ImageLoaderNibbleMask = 0x0F;

// JPEG and EXIF constants
static const uint8_t ImageJpegMarkerPrefix = 0xFF;
static const uint8_t ImageJpegStartOfImage = 0xD8;
static const uint8_t ImageJpegEndOfImage = 0xD9;
static const uint8_t ImageJpegStartOfScan = 0xDA;
static const uint8_t ImageJpegApp1 = 0xE1;
static const uint8_t ImageJpegTemporary = 0x01;
static const uint8_t ImageJpegFirstRestart = 0xD0;
static const uint8_t ImageJpegLastRestart = 0xD7;
// A JPEG header rarely has more segments than this before the first scan
static const int ImageJpegMaximumSegments = 256;
static const size_t ImageJpegSegmentLengthSize = 2;
static const size_t ImageExifSignatureSize = 6;
static const char ImageExifSignature[] = "Exif\0\0";
static const uint16_t ImageTiffMagic = 42;
static const uint16_t ImageTiffTagOrientation = 0x0112;
static const uint16_t ImageTiffTypeShort = 3;
static const size_t ImageTiffHeaderSize = 8;
static const size_t ImageTiffEntrySize = 12;
static const size_t ImageTiffEntryCountSize = 2;

// --- Ids -------------------------------------------------------------------------------------------------------------------

static int ImageLoaderHexValue(const char c) {
	const int letterOffset = 10;
	if (c >= '0' && c <= '9') {
		return(c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return(c - 'a' + letterOffset);
	}
	if (c >= 'A' && c <= 'F') {
		return(c - 'A' + letterOffset);
	}
	return(-1);
}

static bool ImageLoaderIsDashPosition(const int byteIndex) {
	for (size_t index = 0; index < sizeof(ImageLoaderIdDashAfterBytes) / sizeof(ImageLoaderIdDashAfterBytes[0]); ++index) {
		if (ImageLoaderIdDashAfterBytes[index] == byteIndex) {
			return(true);
		}
	}
	return(false);
}

extern bool ImageLoaderIdParse(const char *text, ImageLoaderId *outId) {
	if (text == NULL) {
		return(false);
	}
	const char *p = text;
	bool hasBraces = *p == '{';
	if (hasBraces) {
		++p;
	}
	ImageLoaderId id;
	for (int byteIndex = 0; byteIndex < IMAGE_LOADER_ID_SIZE; ++byteIndex) {
		if (ImageLoaderIsDashPosition(byteIndex)) {
			if (*p != '-') {
				return(false);
			}
			++p;
		}
		int high = ImageLoaderHexValue(p[0]);
		int low = high >= 0 ? ImageLoaderHexValue(p[1]) : -1;
		if (high < 0 || low < 0) {
			return(false);
		}
		id.bytes[byteIndex] = (uint8_t)((high << ImageLoaderBitsPerNibble) | low);
		p += 2;
	}
	if (hasBraces) {
		if (*p != '}') {
			return(false);
		}
		++p;
	}
	if (*p != 0) {
		return(false);
	}
	*outId = id;
	return(true);
}

extern void ImageLoaderIdFormat(const ImageLoaderId *id, char *buffer, const size_t bufferSize) {
	if (bufferSize < IMAGE_LOADER_ID_TEXT_SIZE) {
		if (bufferSize > 0) {
			buffer[0] = 0;
		}
		return;
	}
	char *p = buffer;
	for (int byteIndex = 0; byteIndex < IMAGE_LOADER_ID_SIZE; ++byteIndex) {
		if (ImageLoaderIsDashPosition(byteIndex)) {
			*p++ = '-';
		}
		uint8_t value = id->bytes[byteIndex];
		*p++ = ImageLoaderHexDigits[(value >> ImageLoaderBitsPerNibble) & ImageLoaderNibbleMask];
		*p++ = ImageLoaderHexDigits[value & ImageLoaderNibbleMask];
	}
	*p = 0;
}

extern bool ImageLoaderIdIsEqual(const ImageLoaderId *a, const ImageLoaderId *b) {
	bool result = memcmp(a->bytes, b->bytes, IMAGE_LOADER_ID_SIZE) == 0;
	return(result);
}

extern const char *ImageLoadResultGetName(const ImageLoadResult result) {
	switch (result) {
		case ImageLoadResult_Success:
			return("Success");
		case ImageLoadResult_Unsupported:
			return("Unsupported");
		case ImageLoadResult_Corrupt:
			return("Corrupt");
		case ImageLoadResult_OutOfMemory:
			return("OutOfMemory");
		case ImageLoadResult_Canceled:
			return("Canceled");
		default:
			return("Unknown");
	}
}

// --- EXIF ------------------------------------------------------------------------------------------------------------------

static uint16_t ImageReadU16(const uint8_t *p, const bool isBigEndian) {
	const int byteBits = 8;
	uint16_t result = isBigEndian ? (uint16_t)((p[0] << byteBits) | p[1]) : (uint16_t)((p[1] << byteBits) | p[0]);
	return(result);
}

static uint32_t ImageReadU32(const uint8_t *p, const bool isBigEndian) {
	const int wordBits = 16;
	const size_t wordSize = 2;
	uint32_t first = ImageReadU16(p, isBigEndian);
	uint32_t second = ImageReadU16(p + wordSize, isBigEndian);
	uint32_t result = isBigEndian ? ((first << wordBits) | second) : ((second << wordBits) | first);
	return(result);
}

extern ImageOrientation ImageExifReadOrientation(const uint8_t *tiff, const size_t tiffSize) {
	if (tiff == NULL || tiffSize < ImageTiffHeaderSize) {
		return(ImageOrientation_Normal);
	}
	bool isBigEndian;
	if (tiff[0] == 'M' && tiff[1] == 'M') {
		isBigEndian = true;
	} else if (tiff[0] == 'I' && tiff[1] == 'I') {
		isBigEndian = false;
	} else {
		return(ImageOrientation_Normal);
	}
	const size_t magicOffset = 2;
	const size_t firstIfdOffsetOffset = 4;
	uint16_t magic = ImageReadU16(tiff + magicOffset, isBigEndian);
	if (magic != ImageTiffMagic) {
		return(ImageOrientation_Normal);
	}
	// All offsets are checked in 64 bit, so no sum can wrap around
	uint64_t ifdOffset = ImageReadU32(tiff + firstIfdOffsetOffset, isBigEndian);
	if (ifdOffset + ImageTiffEntryCountSize > tiffSize) {
		return(ImageOrientation_Normal);
	}
	uint16_t entryCount = ImageReadU16(tiff + ifdOffset, isBigEndian);
	uint64_t firstEntry = ifdOffset + ImageTiffEntryCountSize;
	const size_t typeOffset = 2;
	const size_t countOffset = 4;
	const size_t valueOffset = 8;
	for (uint32_t entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		uint64_t entryOffset = firstEntry + (uint64_t)entryIndex * ImageTiffEntrySize;
		if (entryOffset + ImageTiffEntrySize > tiffSize) {
			break;
		}
		const uint8_t *entry = tiff + entryOffset;
		uint16_t tag = ImageReadU16(entry, isBigEndian);
		if (tag != ImageTiffTagOrientation) {
			continue;
		}
		uint16_t type = ImageReadU16(entry + typeOffset, isBigEndian);
		uint32_t count = ImageReadU32(entry + countOffset, isBigEndian);
		if (type != ImageTiffTypeShort || count != 1) {
			return(ImageOrientation_Normal);
		}
		uint16_t value = ImageReadU16(entry + valueOffset, isBigEndian);
		if (value < ImageOrientation_Normal || value > ImageOrientation_Rotate270) {
			return(ImageOrientation_Normal);
		}
		return((ImageOrientation)value);
	}
	return(ImageOrientation_Normal);
}

static bool ImageSourceReadExact(ImageSource *source, void *buffer, const size_t size) {
	size_t readSize = source->read(source, buffer, size);
	bool result = readSize == size;
	return(result);
}

extern ImageOrientation ImageJpegReadOrientation(ImageSource *source) {
	const size_t markerSize = 2;
	uint8_t marker[2];
	if (!source->seek(source, 0) || !ImageSourceReadExact(source, marker, markerSize)) {
		return(ImageOrientation_Normal);
	}
	if (marker[0] != ImageJpegMarkerPrefix || marker[1] != ImageJpegStartOfImage) {
		return(ImageOrientation_Normal);
	}
	for (int segmentIndex = 0; segmentIndex < ImageJpegMaximumSegments; ++segmentIndex) {
		// Markers may be padded with any number of 0xFF bytes
		uint8_t byte = 0;
		if (!ImageSourceReadExact(source, &byte, 1) || byte != ImageJpegMarkerPrefix) {
			return(ImageOrientation_Normal);
		}
		do {
			if (!ImageSourceReadExact(source, &byte, 1)) {
				return(ImageOrientation_Normal);
			}
		} while (byte == ImageJpegMarkerPrefix);
		uint8_t type = byte;
		if (type == ImageJpegStartOfScan || type == ImageJpegEndOfImage) {
			return(ImageOrientation_Normal);
		}
		bool isStandalone = type == ImageJpegTemporary || (type >= ImageJpegFirstRestart && type <= ImageJpegLastRestart);
		if (isStandalone) {
			continue;
		}
		uint8_t lengthBytes[2];
		if (!ImageSourceReadExact(source, lengthBytes, ImageJpegSegmentLengthSize)) {
			return(ImageOrientation_Normal);
		}
		size_t segmentLength = ImageReadU16(lengthBytes, true);
		if (segmentLength < ImageJpegSegmentLengthSize) {
			return(ImageOrientation_Normal);
		}
		size_t payloadSize = segmentLength - ImageJpegSegmentLengthSize;
		uint64_t payloadStart = source->tell(source);
		if (type == ImageJpegApp1 && payloadSize > ImageExifSignatureSize) {
			// The length field has 16 bits, so a payload is below 64 KB; on the heap, load threads may have small stacks
			uint8_t *payload = (uint8_t *)malloc(payloadSize);
			if (payload == NULL) {
				return(ImageOrientation_Normal);
			}
			bool isRead = ImageSourceReadExact(source, payload, payloadSize);
			bool isExif = isRead && memcmp(payload, ImageExifSignature, ImageExifSignatureSize) == 0;
			ImageOrientation result = ImageOrientation_Normal;
			if (isExif) {
				result = ImageExifReadOrientation(payload + ImageExifSignatureSize, payloadSize - ImageExifSignatureSize);
			}
			free(payload);
			if (!isRead || isExif) {
				return(result);
			}
		}
		if (!source->seek(source, payloadStart + payloadSize)) {
			return(ImageOrientation_Normal);
		}
	}
	return(ImageOrientation_Normal);
}

// --- Sources ---------------------------------------------------------------------------------------------------------------

static void ImageFileSourceUpdateProgress(ImageFileSource *fileSource) {
	if (fileSource->progress == NULL || fileSource->fileSize == 0 || fileSource->hasLoaderProgress) {
		return;
	}
	uint64_t position = fplFileGetPosition64(&fileSource->file);
	float fraction = (float)((double)position / (double)fileSource->fileSize);
	fileSource->progress(fileSource->progressUserData, fraction, false);
}

static size_t ImageFileSourceRead(ImageSource *source, void *buffer, size_t size) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	if (source->isCanceled(source)) {
		return(0);
	}
	uint64_t readSize = fplFileReadBlock64(&fileSource->file, size, buffer, size);
	ImageFileSourceUpdateProgress(fileSource);
	return((size_t)readSize);
}

static bool ImageFileSourceSeek(ImageSource *source, uint64_t position) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	if (position > fileSource->fileSize) {
		return(false);
	}
	uint64_t newPosition = fplFileSetPosition64(&fileSource->file, (int64_t)position, fplFilePositionMode_Beginning);
	ImageFileSourceUpdateProgress(fileSource);
	bool result = newPosition == position;
	return(result);
}

static uint64_t ImageFileSourceTell(ImageSource *source) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	uint64_t result = fplFileGetPosition64(&fileSource->file);
	return(result);
}

static uint64_t ImageFileSourceSize(ImageSource *source) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	return(fileSource->fileSize);
}

static bool ImageFileSourceIsCanceled(ImageSource *source) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	bool result = fileSource->cancelFlag != NULL && *fileSource->cancelFlag;
	return(result);
}

static void ImageFileSourceReportProgress(ImageSource *source, float fraction, bool isIndeterminate) {
	ImageFileSource *fileSource = (ImageFileSource *)source;
	fileSource->hasLoaderProgress = true;
	if (fileSource->progress != NULL) {
		fileSource->progress(fileSource->progressUserData, fraction, isIndeterminate);
	}
}

extern bool ImageFileSourceOpen(ImageFileSource *fileSource, const char *filePath, volatile bool *cancelFlag, ImageSourceProgressFunction *progress, void *progressUserData) {
	memset(fileSource, 0, sizeof(*fileSource));
	if (!fplFileOpenBinary(filePath, &fileSource->file)) {
		return(false);
	}
	fileSource->fileSize = fplFileGetSizeFromHandle64(&fileSource->file);
	fileSource->cancelFlag = cancelFlag;
	fileSource->progress = progress;
	fileSource->progressUserData = progressUserData;
	fileSource->source.read = ImageFileSourceRead;
	fileSource->source.seek = ImageFileSourceSeek;
	fileSource->source.tell = ImageFileSourceTell;
	fileSource->source.size = ImageFileSourceSize;
	fileSource->source.isCanceled = ImageFileSourceIsCanceled;
	fileSource->source.reportProgress = ImageFileSourceReportProgress;
	return(true);
}

extern void ImageFileSourceClose(ImageFileSource *fileSource) {
	fplFileClose(&fileSource->file);
}

static size_t ImageMemorySourceRead(ImageSource *source, void *buffer, size_t size) {
	ImageMemorySource *memorySource = (ImageMemorySource *)source;
	uint64_t remaining = memorySource->dataSize - memorySource->position;
	size_t readSize = (uint64_t)size < remaining ? size : (size_t)remaining;
	memcpy(buffer, memorySource->data + memorySource->position, readSize);
	memorySource->position += readSize;
	return(readSize);
}

static bool ImageMemorySourceSeek(ImageSource *source, uint64_t position) {
	ImageMemorySource *memorySource = (ImageMemorySource *)source;
	if (position > memorySource->dataSize) {
		return(false);
	}
	memorySource->position = position;
	return(true);
}

static uint64_t ImageMemorySourceTell(ImageSource *source) {
	ImageMemorySource *memorySource = (ImageMemorySource *)source;
	return(memorySource->position);
}

static uint64_t ImageMemorySourceSize(ImageSource *source) {
	ImageMemorySource *memorySource = (ImageMemorySource *)source;
	return(memorySource->dataSize);
}

static bool ImageMemorySourceIsCanceled(ImageSource *source) {
	return(false);
}

static void ImageMemorySourceReportProgress(ImageSource *source, float fraction, bool isIndeterminate) {
}

extern void ImageMemorySourceInit(ImageMemorySource *memorySource, const void *data, const size_t dataSize) {
	memset(memorySource, 0, sizeof(*memorySource));
	memorySource->data = (const uint8_t *)data;
	memorySource->dataSize = dataSize;
	memorySource->source.read = ImageMemorySourceRead;
	memorySource->source.seek = ImageMemorySourceSeek;
	memorySource->source.tell = ImageMemorySourceTell;
	memorySource->source.size = ImageMemorySourceSize;
	memorySource->source.isCanceled = ImageMemorySourceIsCanceled;
	memorySource->source.reportProgress = ImageMemorySourceReportProgress;
}

// --- Registry --------------------------------------------------------------------------------------------------------------

static void ImageLoaderSetMessage(char *message, const size_t messageSize, const char *text) {
	if (message != NULL && messageSize > 0) {
		snprintf(message, messageSize, "%s", text);
	}
}

// Lower case with a leading dot, false when it does not fit
static bool ImageLoaderNormalizeExtension(const char *extension, char *buffer, const size_t bufferSize) {
	const char caseOffset = 'a' - 'A';
	if (extension == NULL || bufferSize == 0) {
		return(false);
	}
	size_t length = 0;
	if (*extension != '.') {
		buffer[length++] = '.';
	}
	for (const char *p = extension; *p; ++p) {
		if (length + 1 >= bufferSize) {
			return(false);
		}
		char c = *p;
		buffer[length++] = (c >= 'A' && c <= 'Z') ? (char)(c + caseOffset) : c;
	}
	buffer[length] = 0;
	return(true);
}

static bool ImageLoaderIsNameEqual(const char *a, const char *b) {
	const char caseOffset = 'a' - 'A';
	while (*a && *b) {
		char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + caseOffset) : *a;
		char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b + caseOffset) : *b;
		if (ca != cb) {
			return(false);
		}
		++a;
		++b;
	}
	bool result = *a == 0 && *b == 0;
	return(result);
}

extern bool ImageLoaderListsExtension(const ImageLoader *loader, const char *normalizedExtension) {
	if (loader->fileExtensions == NULL) {
		return(false);
	}
	for (const char *const *extension = loader->fileExtensions; *extension != NULL; ++extension) {
		if (ImageLoaderIsNameEqual(*extension, normalizedExtension)) {
			return(true);
		}
	}
	return(false);
}

static bool ImageLoaderIsValidEntry(const ImageLoaderRegistry *registry, const int32_t entryIndex) {
	bool result = entryIndex >= 0 && (uint32_t)entryIndex < registry->count;
	return(result);
}

extern void ImageLoaderRegistryInit(ImageLoaderRegistry *registry) {
	memset(registry, 0, sizeof(*registry));
	registry->globalPinIndex = -1;
	registry->isFallbackEnabled = true;
}

extern void ImageLoaderRegistryRelease(ImageLoaderRegistry *registry) {
	for (uint32_t index = 0; index < registry->count; ++index) {
		if (registry->entries[index].hasMutex) {
			fplMutexDestroy(&registry->entries[index].mutex);
		}
	}
	memset(registry, 0, sizeof(*registry));
}

extern bool ImageLoaderRegistryAdd(ImageLoaderRegistry *registry, const ImageLoader *loader, char *message, const size_t messageSize) {
	if (loader->interfaceVersion != IMAGE_LOADER_INTERFACE_VERSION) {
		ImageLoaderSetMessage(message, messageSize, "unknown interface version");
		return(false);
	}
	if (loader->structSize < sizeof(ImageLoader)) {
		ImageLoaderSetMessage(message, messageSize, "loader struct is too small");
		return(false);
	}
	if (loader->probe == NULL || loader->readInfo == NULL || loader->decode == NULL || loader->releasePixels == NULL) {
		ImageLoaderSetMessage(message, messageSize, "loader function missing");
		return(false);
	}
	if (registry->count >= IMAGE_LOADER_MAX_COUNT) {
		ImageLoaderSetMessage(message, messageSize, "too many loaders");
		return(false);
	}
	for (uint32_t index = 0; index < registry->count; ++index) {
		if (ImageLoaderIdIsEqual(&registry->entries[index].loader->id, &loader->id)) {
			ImageLoaderSetMessage(message, messageSize, "a loader with this id is already registered");
			return(false);
		}
	}
	ImageLoaderEntry *entry = &registry->entries[registry->count];
	entry->loader = loader;
	entry->hasMutex = false;
	if ((loader->flags & ImageLoaderFlags_NotThreadSafe) != 0) {
		entry->hasMutex = fplMutexInit(&entry->mutex);
		if (!entry->hasMutex) {
			ImageLoaderSetMessage(message, messageSize, "mutex for a loader that is not thread-safe failed");
			return(false);
		}
	}
	registry->order[registry->count] = (int32_t)registry->count;
	++registry->count;
	return(true);
}

extern int32_t ImageLoaderRegistryFind(const ImageLoaderRegistry *registry, const char *idOrName, char *message, const size_t messageSize) {
	ImageLoaderId id;
	if (ImageLoaderIdParse(idOrName, &id)) {
		for (uint32_t index = 0; index < registry->count; ++index) {
			if (ImageLoaderIdIsEqual(&registry->entries[index].loader->id, &id)) {
				return((int32_t)index);
			}
		}
		if (message != NULL && messageSize > 0) {
			snprintf(message, messageSize, "no loader with id '%s'", idOrName);
		}
		return(-1);
	}
	int32_t found = -1;
	uint32_t matchCount = 0;
	for (uint32_t index = 0; index < registry->count; ++index) {
		if (ImageLoaderIsNameEqual(registry->entries[index].loader->name, idOrName)) {
			found = (int32_t)index;
			++matchCount;
		}
	}
	if (matchCount == 1) {
		return(found);
	}
	if (message != NULL && messageSize > 0) {
		if (matchCount == 0) {
			snprintf(message, messageSize, "no loader with id or name '%s'", idOrName);
		} else {
			// Ambiguous: list the ids of all loaders with this name
			int written = snprintf(message, messageSize, "name '%s' is ambiguous, use one of the ids:", idOrName);
			for (uint32_t index = 0; index < registry->count && written >= 0 && (size_t)written < messageSize; ++index) {
				if (ImageLoaderIsNameEqual(registry->entries[index].loader->name, idOrName)) {
					char idText[IMAGE_LOADER_ID_TEXT_SIZE];
					ImageLoaderIdFormat(&registry->entries[index].loader->id, idText, sizeof(idText));
					written += snprintf(message + written, messageSize - (size_t)written, " %s", idText);
				}
			}
		}
	}
	return(-1);
}

extern const ImageLoader *ImageLoaderRegistryGet(const ImageLoaderRegistry *registry, const int32_t entryIndex) {
	if (!ImageLoaderIsValidEntry(registry, entryIndex)) {
		return(NULL);
	}
	return(registry->entries[entryIndex].loader);
}

extern bool ImageLoaderRegistrySetOrder(ImageLoaderRegistry *registry, const int32_t *entryIndices, const uint32_t count) {
	int32_t newOrder[IMAGE_LOADER_MAX_COUNT];
	bool isPlaced[IMAGE_LOADER_MAX_COUNT] = { false };
	uint32_t placedCount = 0;
	for (uint32_t index = 0; index < count; ++index) {
		int32_t entryIndex = entryIndices[index];
		if (!ImageLoaderIsValidEntry(registry, entryIndex) || isPlaced[entryIndex]) {
			return(false);
		}
		newOrder[placedCount++] = entryIndex;
		isPlaced[entryIndex] = true;
	}
	for (uint32_t index = 0; index < registry->count; ++index) {
		int32_t entryIndex = registry->order[index];
		if (!isPlaced[entryIndex]) {
			newOrder[placedCount++] = entryIndex;
		}
	}
	memcpy(registry->order, newOrder, sizeof(int32_t) * registry->count);
	return(true);
}

extern void ImageLoaderRegistryPinGlobal(ImageLoaderRegistry *registry, const int32_t entryIndex) {
	registry->globalPinIndex = ImageLoaderIsValidEntry(registry, entryIndex) ? entryIndex : -1;
}

extern bool ImageLoaderRegistryPinExtension(ImageLoaderRegistry *registry, const char *extension, const int32_t entryIndex) {
	char normalized[IMAGE_LOADER_MAX_EXTENSION_LENGTH];
	if (!ImageLoaderIsValidEntry(registry, entryIndex) || !ImageLoaderNormalizeExtension(extension, normalized, sizeof(normalized))) {
		return(false);
	}
	for (uint32_t index = 0; index < registry->extensionPinCount; ++index) {
		if (strcmp(registry->extensionPins[index].extension, normalized) == 0) {
			registry->extensionPins[index].entryIndex = entryIndex;
			return(true);
		}
	}
	if (registry->extensionPinCount >= IMAGE_LOADER_MAX_EXTENSION_PINS) {
		return(false);
	}
	ImageLoaderExtensionPin *pin = &registry->extensionPins[registry->extensionPinCount++];
	memcpy(pin->extension, normalized, sizeof(pin->extension));
	pin->entryIndex = entryIndex;
	return(true);
}

extern bool ImageLoaderRegistryIsKnownExtension(const ImageLoaderRegistry *registry, const char *extension) {
	char normalized[IMAGE_LOADER_MAX_EXTENSION_LENGTH];
	if (!ImageLoaderNormalizeExtension(extension, normalized, sizeof(normalized))) {
		return(false);
	}
	for (uint32_t index = 0; index < registry->count; ++index) {
		if (ImageLoaderListsExtension(registry->entries[index].loader, normalized)) {
			return(true);
		}
	}
	return(false);
}

static uint32_t ImageLoaderRegistrySelectCandidates(const ImageLoaderRegistry *registry, const uint8_t *header, const size_t headerSize, const char *fileExtension, const bool ignoreFallbackSetting, int32_t *outEntries, const uint32_t maxEntries) {
	char normalized[IMAGE_LOADER_MAX_EXTENSION_LENGTH] = { 0 };
	const char *extension = ImageLoaderNormalizeExtension(fileExtension, normalized, sizeof(normalized)) ? normalized : "";

	ImageLoaderMatch matches[IMAGE_LOADER_MAX_COUNT];
	for (uint32_t index = 0; index < registry->count; ++index) {
		const ImageLoader *loader = registry->entries[index].loader;
		matches[index] = loader->probe(loader, header, headerSize, extension);
	}

	// An extension pin is more specific than the global pin
	int32_t pinIndex = registry->globalPinIndex;
	for (uint32_t index = 0; index < registry->extensionPinCount; ++index) {
		if (strcmp(registry->extensionPins[index].extension, extension) == 0) {
			pinIndex = registry->extensionPins[index].entryIndex;
		}
	}

	uint32_t count = 0;
	bool isTaken[IMAGE_LOADER_MAX_COUNT] = { false };
	if (ImageLoaderIsValidEntry(registry, pinIndex) && matches[pinIndex] != ImageLoaderMatch_None && count < maxEntries) {
		outEntries[count++] = pinIndex;
		isTaken[pinIndex] = true;
	}
	const ImageLoaderMatch levels[] = { ImageLoaderMatch_Signature, ImageLoaderMatch_Extension };
	for (size_t levelIndex = 0; levelIndex < sizeof(levels) / sizeof(levels[0]); ++levelIndex) {
		for (uint32_t orderIndex = 0; orderIndex < registry->count && count < maxEntries; ++orderIndex) {
			int32_t entryIndex = registry->order[orderIndex];
			if (!isTaken[entryIndex] && matches[entryIndex] == levels[levelIndex]) {
				outEntries[count++] = entryIndex;
				isTaken[entryIndex] = true;
			}
		}
	}
	if (!registry->isFallbackEnabled && !ignoreFallbackSetting && count > 1) {
		count = 1;
	}
	return(count);
}

extern uint32_t ImageLoaderRegistrySelect(const ImageLoaderRegistry *registry, const uint8_t *header, const size_t headerSize, const char *fileExtension, int32_t *outEntries, const uint32_t maxEntries) {
	uint32_t result = ImageLoaderRegistrySelectCandidates(registry, header, headerSize, fileExtension, false, outEntries, maxEntries);
	return(result);
}

extern uint32_t ImageLoaderRegistrySelectForSource(const ImageLoaderRegistry *registry, ImageSource *source, const char *fileExtension, const bool ignoreFallbackSetting, int32_t *outEntries, const uint32_t maxEntries) {
	uint8_t header[IMAGE_LOADER_PROBE_SIZE];
	size_t headerSize = 0;
	if (source->seek(source, 0)) {
		headerSize = source->read(source, header, sizeof(header));
	}
	uint32_t result = ImageLoaderRegistrySelectCandidates(registry, header, headerSize, fileExtension, ignoreFallbackSetting, outEntries, maxEntries);
	return(result);
}

static void ImageLoaderLock(ImageLoaderRegistry *registry, const int32_t entryIndex) {
	if (registry->entries[entryIndex].hasMutex) {
		fplMutexLock(&registry->entries[entryIndex].mutex);
	}
}

static void ImageLoaderUnlock(ImageLoaderRegistry *registry, const int32_t entryIndex) {
	if (registry->entries[entryIndex].hasMutex) {
		fplMutexUnlock(&registry->entries[entryIndex].mutex);
	}
}

extern ImageLoadResult ImageLoaderRegistryReadInfo(ImageLoaderRegistry *registry, const int32_t entryIndex, ImageSource *source, PictureInfo *outInfo, char *message, const size_t messageSize) {
	if (!ImageLoaderIsValidEntry(registry, entryIndex) || !source->seek(source, 0)) {
		ImageLoaderSetMessage(message, messageSize, "invalid loader or source");
		return(ImageLoadResult_Unsupported);
	}
	const ImageLoader *loader = registry->entries[entryIndex].loader;
	memset(outInfo, 0, sizeof(*outInfo));
	outInfo->orientation = ImageOrientation_Normal;
	ImageLoaderLock(registry, entryIndex);
	ImageLoadResult result = loader->readInfo(loader, source, outInfo, message, messageSize);
	ImageLoaderUnlock(registry, entryIndex);
	return(result);
}

extern ImageLoadResult ImageLoaderRegistryDecode(ImageLoaderRegistry *registry, const int32_t entryIndex, ImageSource *source, ImagePixels *outPixels, char *message, const size_t messageSize) {
	if (!ImageLoaderIsValidEntry(registry, entryIndex) || !source->seek(source, 0)) {
		ImageLoaderSetMessage(message, messageSize, "invalid loader or source");
		return(ImageLoadResult_Unsupported);
	}
	const ImageLoader *loader = registry->entries[entryIndex].loader;
	memset(outPixels, 0, sizeof(*outPixels));
	ImageLoaderLock(registry, entryIndex);
	ImageLoadResult result = loader->decode(loader, source, outPixels, message, messageSize);
	ImageLoaderUnlock(registry, entryIndex);
	return(result);
}

extern void ImageLoaderRegistryReleasePixels(ImageLoaderRegistry *registry, const int32_t entryIndex, ImagePixels *pixels) {
	if (!ImageLoaderIsValidEntry(registry, entryIndex) || pixels->pixels == NULL) {
		return;
	}
	const ImageLoader *loader = registry->entries[entryIndex].loader;
	ImageLoaderLock(registry, entryIndex);
	loader->releasePixels(loader, pixels);
	ImageLoaderUnlock(registry, entryIndex);
	memset(pixels, 0, sizeof(*pixels));
}

static void ImageLoaderLogAttempt(const ImageLoaderRegistry *registry, const int32_t entryIndex, const char *step, const ImageLoadResult result, const char *reason) {
	if (registry->log == NULL) {
		return;
	}
	char line[IMAGE_LOADER_MESSAGE_SIZE * 2];
	const ImageLoader *loader = registry->entries[entryIndex].loader;
	const char *resultName = ImageLoadResultGetName(result);
	snprintf(line, sizeof(line), "Loader '%s' %s: %s (%s)", loader->name, step, resultName, reason);
	registry->log(line);
}

extern ImageLoadResult ImageLoaderRegistryLoad(ImageLoaderRegistry *registry, ImageSource *source, const char *fileExtension, const int32_t forcedEntry, int32_t *outEntry, PictureInfo *outInfo, ImagePixels *outPixels, char *message, const size_t messageSize) {
	int32_t candidates[IMAGE_LOADER_MAX_COUNT];
	uint32_t candidateCount;
	if (forcedEntry >= 0) {
		if (!ImageLoaderIsValidEntry(registry, forcedEntry)) {
			ImageLoaderSetMessage(message, messageSize, "forced loader does not exist");
			return(ImageLoadResult_Unsupported);
		}
		candidates[0] = forcedEntry;
		candidateCount = 1;
	} else {
		candidateCount = ImageLoaderRegistrySelectForSource(registry, source, fileExtension, false, candidates, IMAGE_LOADER_MAX_COUNT);
	}
	if (candidateCount == 0) {
		ImageLoaderSetMessage(message, messageSize, "no loader recognizes this file");
		return(ImageLoadResult_Unsupported);
	}

	ImageLoadResult result = ImageLoadResult_Unsupported;
	for (uint32_t candidateIndex = 0; candidateIndex < candidateCount; ++candidateIndex) {
		int32_t entryIndex = candidates[candidateIndex];
		char reason[IMAGE_LOADER_MESSAGE_SIZE] = { 0 };
		result = ImageLoaderRegistryReadInfo(registry, entryIndex, source, outInfo, reason, sizeof(reason));
		if (result == ImageLoadResult_Success) {
			result = ImageLoaderRegistryDecode(registry, entryIndex, source, outPixels, reason, sizeof(reason));
			if (result == ImageLoadResult_Success) {
				*outEntry = entryIndex;
				ImageLoaderSetMessage(message, messageSize, "");
				return(result);
			}
			ImageLoaderLogAttempt(registry, entryIndex, "decode", result, reason);
		} else {
			ImageLoaderLogAttempt(registry, entryIndex, "info", result, reason);
		}
		ImageLoaderSetMessage(message, messageSize, reason);
		bool canFallBack = result == ImageLoadResult_Unsupported || result == ImageLoadResult_Corrupt;
		if (!canFallBack) {
			break;
		}
	}
	return(result);
}

#endif // IMAGE_LOADER_IMPLEMENTATION
