/*
Name:
	FPL_ImageViewer | GIF loader

Description:
	Reads GIF files (87a and 89a) without any library: LZW, interlaced rows, global and local color tables, transparency and every disposal method.
	decode composes the first frame. An animation keeps the file in memory and composes one frame after the other on a canvas of its own,
	so only one frame is decoded at a time, however many frames the file has.

	Like the browsers: the canvas starts transparent and the background color is ignored, disposal to the background clears to transparent,
	delays of 10 ms and less last 100 ms, and a NETSCAPE2.0 loop count of n plays the animation n + 1 times (0 forever, a file without it plays once).
	A frame whose data ends early keeps the pixels decoded so far. A frame cut off by the end of the file only shows as the first frame,
	an animation ends with its last complete frame.

Usage:
	Include after imageloader.h. Define IMAGE_LOADER_GIF_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_LOADER_GIF_H
#define IMAGE_LOADER_GIF_H

#include "imageloader.h"

// The id in text form, the self test checks it against the bytes in the loader
#define IMAGE_LOADER_GIF_ID_TEXT "37544c19-2b92-4663-8357-0c5825c596f8"

extern const ImageLoader *ImageLoaderGifGet(void);

#endif // IMAGE_LOADER_GIF_H

#if defined(IMAGE_LOADER_GIF_IMPLEMENTATION) && !defined(IMAGE_LOADER_GIF_IMPLEMENTED)
#define IMAGE_LOADER_GIF_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LZW codes have at most 12 bits, so the string table has 4096 entries
#define IMAGE_LOADER_GIF_CODE_LIMIT 4096
// A color table has at most 256 entries, kept as RGBA
#define IMAGE_LOADER_GIF_PALETTE_CAPACITY 256
#define IMAGE_LOADER_GIF_CHANNELS 4
#define IMAGE_LOADER_GIF_PALETTE_BYTES (IMAGE_LOADER_GIF_PALETTE_CAPACITY * IMAGE_LOADER_GIF_CHANNELS)
// The file stores 3 bytes (red, green, blue) per color table entry
#define IMAGE_LOADER_GIF_STORED_PALETTE_BYTES (IMAGE_LOADER_GIF_PALETTE_CAPACITY * 3)
#define IMAGE_LOADER_GIF_SIGNATURE_SIZE 6
// Logical screen descriptor: width, height, flags, background color index, pixel aspect ratio
#define IMAGE_LOADER_GIF_SCREEN_DESCRIPTOR_SIZE 7
// Image descriptor behind its separator: left, top, width, height, flags
#define IMAGE_LOADER_GIF_IMAGE_DESCRIPTOR_SIZE 9
// A sub-block holds at most 255 bytes behind its size byte
#define IMAGE_LOADER_GIF_SUB_BLOCK_CAPACITY 255
// Interlaced frames store their rows in four passes, each with its first row and its step
#define IMAGE_LOADER_GIF_INTERLACE_PASS_COUNT 4

static const char *const ImageLoaderGifExtensions[] = { ".gif", NULL };

static const uint8_t ImageLoaderGifSignature87[IMAGE_LOADER_GIF_SIGNATURE_SIZE] = { 'G', 'I', 'F', '8', '7', 'a' };
static const uint8_t ImageLoaderGifSignature89[IMAGE_LOADER_GIF_SIGNATURE_SIZE] = { 'G', 'I', 'F', '8', '9', 'a' };
static const size_t ImageLoaderGifScreenFlagsOffset = 4;
static const size_t ImageLoaderGifImageFlagsOffset = 8;
static const size_t ImageLoaderGifWidthOffset = 4;
static const size_t ImageLoaderGifHeightOffset = 6;
static const size_t ImageLoaderGifScreenHeightOffset = 2;
static const size_t ImageLoaderGifTopOffset = 2;
// Flags of both descriptors: a color table of 2 << size entries follows
static const uint8_t ImageLoaderGifColorTableFlag = 0x80;
static const uint8_t ImageLoaderGifColorTableSizeMask = 0x07;
static const uint8_t ImageLoaderGifInterlaceFlag = 0x40;
static const uint32_t ImageLoaderGifStoredEntrySize = 3;
static const uint8_t ImageLoaderGifImageSeparator = 0x2C;
static const uint8_t ImageLoaderGifExtensionIntroducer = 0x21;
static const uint8_t ImageLoaderGifGraphicControlLabel = 0xF9;
static const uint8_t ImageLoaderGifApplicationLabel = 0xFF;
// Graphic control extension: flags, delay in 1/100 s, transparent color index
static const size_t ImageLoaderGifGraphicControlSize = 4;
static const size_t ImageLoaderGifDelayOffset = 1;
static const size_t ImageLoaderGifTransparentIndexOffset = 3;
static const uint8_t ImageLoaderGifTransparencyFlag = 0x01;
static const uint32_t ImageLoaderGifDisposalShift = 2;
static const uint8_t ImageLoaderGifDisposalMask = 0x07;
// Application extension: 8 byte identifier and 3 byte authentication code, then a sub-block with id 1 and the loop count
static const size_t ImageLoaderGifApplicationIdSize = 11;
static const char ImageLoaderGifNetscapeId[] = "NETSCAPE2.0";
static const char ImageLoaderGifAnimextsId[] = "ANIMEXTS1.0";
static const size_t ImageLoaderGifLoopSubBlockSize = 3;
static const uint8_t ImageLoaderGifLoopSubBlockId = 1;
static const size_t ImageLoaderGifLoopCountOffset = 1;
// LZW: the minimum code size of a frame is the bit count of its color indices, the codes grow from one bit more up to 12 bits
static const uint32_t ImageLoaderGifLowestMinimumCodeSize = 1;
static const uint32_t ImageLoaderGifHighestMinimumCodeSize = 8;
static const uint32_t ImageLoaderGifLargestCodeSize = 12;
static const uint32_t ImageLoaderGifBitsPerByte = 8;
static const uint8_t ImageLoaderGifOpaque = 0xFF;
static const uint32_t ImageLoaderGifAlphaChannelIndex = 3;
static const uint32_t ImageLoaderGifRgbChannels = 3;
// Delays below 2/100 s are shown for 100 ms, like the browsers do
static const uint32_t ImageLoaderGifDelayUnitMilliseconds = 10;
static const uint32_t ImageLoaderGifShortestDelay = 2;
static const uint32_t ImageLoaderGifDefaultDurationMilliseconds = 100;
static const uint32_t ImageLoaderGifInterlaceFirstRows[IMAGE_LOADER_GIF_INTERLACE_PASS_COUNT] = { 0, 4, 2, 1 };
static const uint32_t ImageLoaderGifInterlaceRowSteps[IMAGE_LOADER_GIF_INTERLACE_PASS_COUNT] = { 8, 8, 4, 2 };
// A larger canvas is no picture anyone made, it would only exhaust the memory
static const uint64_t ImageLoaderGifMaximumPixelCount = (uint64_t)1 << 28;
// An animation keeps its whole file in memory
static const uint64_t ImageLoaderGifMaximumAnimationFileSize = (uint64_t)256 * 1024 * 1024;
static const size_t ImageLoaderGifReadBufferSize = 64 * 1024;
// Bits per pixel the info shows for a frame without any color table
static const uint32_t ImageLoaderGifDefaultPaletteBits = 8;

typedef enum ImageLoaderGifDisposal {
	// Unspecified and the reserved values 4..7 leave the frame on the canvas
	ImageLoaderGifDisposal_Unspecified = 0,
	ImageLoaderGifDisposal_Keep = 1,
	// Clears the frame area to transparent
	ImageLoaderGifDisposal_Background = 2,
	// Restores the frame area to what was there before the frame
	ImageLoaderGifDisposal_Previous = 3,
} ImageLoaderGifDisposal;

// Area of a frame on the canvas, cut to the canvas
typedef struct ImageLoaderGifRect {
	uint32_t left;
	uint32_t top;
	uint32_t width;
	uint32_t height;
} ImageLoaderGifRect;

// What a frame declares in its graphic control extension and its image descriptor
typedef struct ImageLoaderGifFrameHeader {
	uint32_t left;
	uint32_t top;
	uint32_t width;
	uint32_t height;
	uint32_t durationMilliseconds;
	// Bits per color index of the color table the frame uses, 0 when there is none
	uint32_t paletteBits;
	// -1 when no index is transparent
	int32_t transparentIndex;
	ImageLoaderGifDisposal disposal;
	bool isInterlaced;
	bool hasLocalPalette;
} ImageLoaderGifFrameHeader;

typedef enum ImageLoaderGifFrameResult {
	// The file has no further frame
	ImageLoaderGifFrameResult_None = 0,
	ImageLoaderGifFrameResult_Complete,
	// The file ends inside the frame data, the pixels decoded so far are drawn
	ImageLoaderGifFrameResult_CutOff,
	ImageLoaderGifFrameResult_OutOfMemory,
} ImageLoaderGifFrameResult;

// Bytes one after the other, from a source through a buffer or straight from a file in memory
typedef struct ImageLoaderGifReader {
	// Null when data is the whole file
	ImageSource *source;
	uint8_t *buffer;
	const uint8_t *data;
	size_t dataSize;
	size_t position;
	bool isEnd;
} ImageLoaderGifReader;

typedef struct ImageLoaderGifDecoder {
	ImageLoaderGifReader reader;
	// Canvas size: the logical screen, enlarged by the first frame when that reaches beyond it
	uint32_t width;
	uint32_t height;
	bool isCanvasSized;
	uint32_t globalPaletteBits;
	// Opaque black behind the stored entries, so every index has a color
	uint8_t globalPalette[IMAGE_LOADER_GIF_PALETTE_BYTES];
	uint8_t localPalette[IMAGE_LOADER_GIF_PALETTE_BYTES];
	// The frames are drawn here, RGBA, transparent at the start
	uint8_t *canvas;
	// The canvas under the frame drawn last, kept for its disposal to previous
	uint8_t *previousCanvas;
	ImageLoaderGifDisposal lastDisposal;
	ImageLoaderGifRect lastRect;
	// NETSCAPE2.0 loop count, wherever the file has it
	uint32_t loopCount;
	bool hasLoopCount;
	// LZW string table: every code behind the end code is a shorter string (its prefix code) plus one byte
	uint16_t prefixCodes[IMAGE_LOADER_GIF_CODE_LIMIT];
	uint8_t suffixBytes[IMAGE_LOADER_GIF_CODE_LIMIT];
	// A string comes out backwards and is reversed here, one byte more for a code that is not in the table yet
	uint8_t stringStack[IMAGE_LOADER_GIF_CODE_LIMIT + 1];
} ImageLoaderGifDecoder;

// Places the color indices of a frame on the canvas in the order the LZW data delivers them
typedef struct ImageLoaderGifPixelWriter {
	uint8_t *canvas;
	const uint8_t *palette;
	size_t canvasStride;
	uint32_t canvasWidth;
	uint32_t canvasHeight;
	uint32_t frameLeft;
	uint32_t frameTop;
	uint32_t frameWidth;
	uint32_t frameHeight;
	int32_t transparentIndex;
	uint32_t column;
	uint32_t row;
	uint32_t interlacePass;
	bool isInterlaced;
	// Pixels of the frame that are still to come
	uint64_t remainingPixelCount;
} ImageLoaderGifPixelWriter;

// Everything an animation needs between two frames
typedef struct ImageLoaderGifAnimation {
	ImageLoaderGifDecoder *decoder;
	uint8_t *file;
	size_t fileSize;
	// Behind the screen descriptor and the global color table, where the first frame starts
	size_t firstBlockOffset;
	uint32_t frameCount;
	// Frames composed since the first one, the last of them is on the canvas
	uint32_t composedFrameCount;
	uint32_t lastDurationMilliseconds;
} ImageLoaderGifAnimation;

// --- Reading ---------------------------------------------------------------------------------------------------------------

// False at the end of the data
static bool ImageLoaderGifReaderFill(ImageLoaderGifReader *reader) {
	if (reader->position < reader->dataSize) {
		return(true);
	}
	if (reader->source == NULL || reader->isEnd) {
		reader->isEnd = true;
		return(false);
	}
	size_t readSize = reader->source->read(reader->source, reader->buffer, ImageLoaderGifReadBufferSize);
	if (readSize == 0) {
		reader->isEnd = true;
		return(false);
	}
	reader->data = reader->buffer;
	reader->dataSize = readSize;
	reader->position = 0;
	return(true);
}

static bool ImageLoaderGifReadByte(ImageLoaderGifReader *reader, uint8_t *outByte) {
	if (!ImageLoaderGifReaderFill(reader)) {
		return(false);
	}
	*outByte = reader->data[reader->position];
	++reader->position;
	return(true);
}

static bool ImageLoaderGifReadBytes(ImageLoaderGifReader *reader, uint8_t *target, const size_t count) {
	size_t doneCount = 0;
	while (doneCount < count) {
		if (!ImageLoaderGifReaderFill(reader)) {
			return(false);
		}
		size_t availableCount = reader->dataSize - reader->position;
		size_t remainingCount = count - doneCount;
		size_t chunkSize = availableCount < remainingCount ? availableCount : remainingCount;
		memcpy(target + doneCount, reader->data + reader->position, chunkSize);
		reader->position += chunkSize;
		doneCount += chunkSize;
	}
	return(true);
}

static bool ImageLoaderGifSkipBytes(ImageLoaderGifReader *reader, const size_t count) {
	size_t doneCount = 0;
	while (doneCount < count) {
		if (!ImageLoaderGifReaderFill(reader)) {
			return(false);
		}
		size_t availableCount = reader->dataSize - reader->position;
		size_t remainingCount = count - doneCount;
		size_t chunkSize = availableCount < remainingCount ? availableCount : remainingCount;
		reader->position += chunkSize;
		doneCount += chunkSize;
	}
	return(true);
}

// Skips sub-blocks up to and including the empty one that ends them, false when the file ends before
static bool ImageLoaderGifSkipSubBlocks(ImageLoaderGifReader *reader) {
	for (;;) {
		uint8_t subBlockSize = 0;
		if (!ImageLoaderGifReadByte(reader, &subBlockSize)) {
			return(false);
		}
		if (subBlockSize == 0) {
			return(true);
		}
		if (!ImageLoaderGifSkipBytes(reader, subBlockSize)) {
			return(false);
		}
	}
}

static uint32_t ImageLoaderGifReadLe16(const uint8_t *bytes) {
	uint32_t result = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << ImageLoaderGifBitsPerByte);
	return(result);
}

static bool ImageLoaderGifIsSignature(const uint8_t *bytes, const size_t byteCount) {
	if (byteCount < IMAGE_LOADER_GIF_SIGNATURE_SIZE) {
		return(false);
	}
	bool result = memcmp(bytes, ImageLoaderGifSignature87, IMAGE_LOADER_GIF_SIGNATURE_SIZE) == 0 || memcmp(bytes, ImageLoaderGifSignature89, IMAGE_LOADER_GIF_SIGNATURE_SIZE) == 0;
	return(result);
}

// Every entry opaque black, the stored entries are read over them
static void ImageLoaderGifClearPalette(uint8_t *palette) {
	memset(palette, 0, IMAGE_LOADER_GIF_PALETTE_BYTES);
	for (uint32_t entryIndex = 0; entryIndex < IMAGE_LOADER_GIF_PALETTE_CAPACITY; ++entryIndex) {
		palette[entryIndex * IMAGE_LOADER_GIF_CHANNELS + ImageLoaderGifAlphaChannelIndex] = ImageLoaderGifOpaque;
	}
}

// A color table of 2^paletteBits entries
static bool ImageLoaderGifReadPalette(ImageLoaderGifReader *reader, const uint32_t paletteBits, uint8_t *palette) {
	uint8_t stored[IMAGE_LOADER_GIF_STORED_PALETTE_BYTES];
	uint32_t entryCount = 1u << paletteBits;
	size_t storedSize = (size_t)entryCount * ImageLoaderGifStoredEntrySize;
	if (!ImageLoaderGifReadBytes(reader, stored, storedSize)) {
		return(false);
	}
	ImageLoaderGifClearPalette(palette);
	for (uint32_t entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
		const uint8_t *source = stored + (size_t)entryIndex * ImageLoaderGifStoredEntrySize;
		uint8_t *target = palette + (size_t)entryIndex * IMAGE_LOADER_GIF_CHANNELS;
		memcpy(target, source, ImageLoaderGifRgbChannels);
	}
	return(true);
}

// Signature, logical screen descriptor and the global color table
static ImageLoadResult ImageLoaderGifReadScreen(ImageLoaderGifDecoder *decoder, char *message, size_t messageSize) {
	ImageLoaderGifReader *reader = &decoder->reader;
	uint8_t signature[IMAGE_LOADER_GIF_SIGNATURE_SIZE];
	bool hasSignature = ImageLoaderGifReadBytes(reader, signature, sizeof(signature)) && ImageLoaderGifIsSignature(signature, sizeof(signature));
	if (!hasSignature) {
		snprintf(message, messageSize, "no GIF signature");
		return(ImageLoadResult_Unsupported);
	}
	uint8_t screen[IMAGE_LOADER_GIF_SCREEN_DESCRIPTOR_SIZE];
	if (!ImageLoaderGifReadBytes(reader, screen, sizeof(screen))) {
		snprintf(message, messageSize, "file ends in the screen descriptor");
		return(ImageLoadResult_Corrupt);
	}
	decoder->width = ImageLoaderGifReadLe16(screen);
	decoder->height = ImageLoaderGifReadLe16(screen + ImageLoaderGifScreenHeightOffset);
	uint8_t flags = screen[ImageLoaderGifScreenFlagsOffset];
	ImageLoaderGifClearPalette(decoder->globalPalette);
	decoder->globalPaletteBits = 0;
	if ((flags & ImageLoaderGifColorTableFlag) != 0) {
		uint32_t paletteBits = (uint32_t)(flags & ImageLoaderGifColorTableSizeMask) + 1;
		if (!ImageLoaderGifReadPalette(reader, paletteBits, decoder->globalPalette)) {
			snprintf(message, messageSize, "file ends in the global color table");
			return(ImageLoadResult_Corrupt);
		}
		decoder->globalPaletteBits = paletteBits;
	}
	return(ImageLoadResult_Success);
}

// Takes the graphic control extension and the loop count from the sub-blocks of an extension, up to its empty sub-block
static bool ImageLoaderGifReadExtension(ImageLoaderGifDecoder *decoder, const uint8_t label, ImageLoaderGifFrameHeader *header) {
	ImageLoaderGifReader *reader = &decoder->reader;
	uint8_t subBlock[IMAGE_LOADER_GIF_SUB_BLOCK_CAPACITY];
	bool isLoopExtension = false;
	for (uint32_t subBlockIndex = 0;; ++subBlockIndex) {
		uint8_t subBlockSize = 0;
		if (!ImageLoaderGifReadByte(reader, &subBlockSize)) {
			return(false);
		}
		if (subBlockSize == 0) {
			return(true);
		}
		if (!ImageLoaderGifReadBytes(reader, subBlock, subBlockSize)) {
			return(false);
		}
		bool isFirstSubBlock = subBlockIndex == 0;
		if (isFirstSubBlock && label == ImageLoaderGifGraphicControlLabel && subBlockSize >= ImageLoaderGifGraphicControlSize) {
			uint8_t flags = subBlock[0];
			uint32_t delay = ImageLoaderGifReadLe16(subBlock + ImageLoaderGifDelayOffset);
			uint32_t disposal = (flags >> ImageLoaderGifDisposalShift) & ImageLoaderGifDisposalMask;
			bool isTransparent = (flags & ImageLoaderGifTransparencyFlag) != 0;
			header->disposal = disposal <= ImageLoaderGifDisposal_Previous ? (ImageLoaderGifDisposal)disposal : ImageLoaderGifDisposal_Unspecified;
			header->transparentIndex = isTransparent ? (int32_t)subBlock[ImageLoaderGifTransparentIndexOffset] : -1;
			header->durationMilliseconds = delay < ImageLoaderGifShortestDelay ? ImageLoaderGifDefaultDurationMilliseconds : delay * ImageLoaderGifDelayUnitMilliseconds;
		} else if (isFirstSubBlock && label == ImageLoaderGifApplicationLabel && subBlockSize == ImageLoaderGifApplicationIdSize) {
			isLoopExtension = memcmp(subBlock, ImageLoaderGifNetscapeId, ImageLoaderGifApplicationIdSize) == 0 || memcmp(subBlock, ImageLoaderGifAnimextsId, ImageLoaderGifApplicationIdSize) == 0;
		} else if (isLoopExtension && subBlockSize >= ImageLoaderGifLoopSubBlockSize && subBlock[0] == ImageLoaderGifLoopSubBlockId) {
			decoder->loopCount = ImageLoaderGifReadLe16(subBlock + ImageLoaderGifLoopCountOffset);
			decoder->hasLoopCount = true;
		}
	}
}

// Walks the blocks up to the next image descriptor and reads it with its color table, the extensions before it are taken on the way.
// The first frame enlarges the canvas when it reaches beyond the logical screen. False at the trailer, at anything that is no GIF block and at the end of the file.
static bool ImageLoaderGifReadFrameHeader(ImageLoaderGifDecoder *decoder, ImageLoaderGifFrameHeader *outHeader) {
	ImageLoaderGifReader *reader = &decoder->reader;
	memset(outHeader, 0, sizeof(*outHeader));
	outHeader->transparentIndex = -1;
	outHeader->disposal = ImageLoaderGifDisposal_Unspecified;
	outHeader->durationMilliseconds = ImageLoaderGifDefaultDurationMilliseconds;
	for (;;) {
		uint8_t introducer = 0;
		if (!ImageLoaderGifReadByte(reader, &introducer)) {
			return(false);
		}
		if (introducer == ImageLoaderGifImageSeparator) {
			break;
		}
		// The trailer or a broken file, either way no further frame
		if (introducer != ImageLoaderGifExtensionIntroducer) {
			return(false);
		}
		uint8_t label = 0;
		if (!ImageLoaderGifReadByte(reader, &label) || !ImageLoaderGifReadExtension(decoder, label, outHeader)) {
			return(false);
		}
	}
	uint8_t descriptor[IMAGE_LOADER_GIF_IMAGE_DESCRIPTOR_SIZE];
	if (!ImageLoaderGifReadBytes(reader, descriptor, sizeof(descriptor))) {
		return(false);
	}
	outHeader->left = ImageLoaderGifReadLe16(descriptor);
	outHeader->top = ImageLoaderGifReadLe16(descriptor + ImageLoaderGifTopOffset);
	outHeader->width = ImageLoaderGifReadLe16(descriptor + ImageLoaderGifWidthOffset);
	outHeader->height = ImageLoaderGifReadLe16(descriptor + ImageLoaderGifHeightOffset);
	uint8_t flags = descriptor[ImageLoaderGifImageFlagsOffset];
	outHeader->isInterlaced = (flags & ImageLoaderGifInterlaceFlag) != 0;
	outHeader->paletteBits = decoder->globalPaletteBits;
	if ((flags & ImageLoaderGifColorTableFlag) != 0) {
		uint32_t paletteBits = (uint32_t)(flags & ImageLoaderGifColorTableSizeMask) + 1;
		if (!ImageLoaderGifReadPalette(reader, paletteBits, decoder->localPalette)) {
			return(false);
		}
		outHeader->hasLocalPalette = true;
		outHeader->paletteBits = paletteBits;
	}
	if (!decoder->isCanvasSized) {
		uint32_t frameRight = outHeader->left + outHeader->width;
		uint32_t frameBottom = outHeader->top + outHeader->height;
		decoder->width = frameRight > decoder->width ? frameRight : decoder->width;
		decoder->height = frameBottom > decoder->height ? frameBottom : decoder->height;
		decoder->isCanvasSized = true;
	}
	return(true);
}

// Skips the LZW data of a frame, false when the file ends inside it
static bool ImageLoaderGifSkipImageData(ImageLoaderGifReader *reader) {
	uint8_t minimumCodeSize = 0;
	bool result = ImageLoaderGifReadByte(reader, &minimumCodeSize) && ImageLoaderGifSkipSubBlocks(reader);
	return(result);
}

// --- Decoding --------------------------------------------------------------------------------------------------------------

static void ImageLoaderGifWritePixel(ImageLoaderGifPixelWriter *writer, const uint8_t colorIndex) {
	uint32_t x = writer->frameLeft + writer->column;
	uint32_t y = writer->frameTop + writer->row;
	bool isOnCanvas = writer->row < writer->frameHeight && x < writer->canvasWidth && y < writer->canvasHeight;
	bool isTransparent = (int32_t)colorIndex == writer->transparentIndex;
	if (isOnCanvas && !isTransparent) {
		uint8_t *target = writer->canvas + (size_t)y * writer->canvasStride + (size_t)x * IMAGE_LOADER_GIF_CHANNELS;
		const uint8_t *color = writer->palette + (size_t)colorIndex * IMAGE_LOADER_GIF_CHANNELS;
		memcpy(target, color, IMAGE_LOADER_GIF_CHANNELS);
	}
	--writer->remainingPixelCount;
	++writer->column;
	if (writer->column < writer->frameWidth) {
		return;
	}
	writer->column = 0;
	if (!writer->isInterlaced) {
		++writer->row;
		return;
	}
	writer->row += ImageLoaderGifInterlaceRowSteps[writer->interlacePass];
	while (writer->row >= writer->frameHeight && writer->interlacePass + 1 < IMAGE_LOADER_GIF_INTERLACE_PASS_COUNT) {
		++writer->interlacePass;
		writer->row = ImageLoaderGifInterlaceFirstRows[writer->interlacePass];
	}
}

// Decodes the LZW data of a frame into the writer. False when the file ends inside the data, the pixels decoded so far stay.
// Broken codes end the frame early, the rest of its data is skipped.
static bool ImageLoaderGifDecodeImageData(ImageLoaderGifDecoder *decoder, ImageLoaderGifPixelWriter *writer) {
	ImageLoaderGifReader *reader = &decoder->reader;
	uint8_t minimumCodeSize = 0;
	if (!ImageLoaderGifReadByte(reader, &minimumCodeSize)) {
		return(false);
	}
	bool isValidCodeSize = minimumCodeSize >= ImageLoaderGifLowestMinimumCodeSize && minimumCodeSize <= ImageLoaderGifHighestMinimumCodeSize;
	if (!isValidCodeSize) {
		bool isSkipped = ImageLoaderGifSkipSubBlocks(reader);
		return(isSkipped);
	}
	const uint32_t noCode = IMAGE_LOADER_GIF_CODE_LIMIT;
	uint32_t clearCode = 1u << minimumCodeSize;
	uint32_t endCode = clearCode + 1;
	uint32_t firstFreeCode = clearCode + 2;
	uint32_t initialCodeSize = (uint32_t)minimumCodeSize + 1;
	uint32_t codeSize = initialCodeSize;
	uint32_t nextCode = firstFreeCode;
	uint32_t previousCode = noCode;
	uint8_t previousFirstByte = 0;
	uint32_t bitBuffer = 0;
	uint32_t bitCount = 0;
	size_t subBlockRemaining = 0;
	bool isTerminated = false;
	bool isCutOff = false;
	while (writer->remainingPixelCount > 0) {
		// The bits of the next code, sub-block after sub-block
		while (bitCount < codeSize && !isTerminated && !isCutOff) {
			if (subBlockRemaining == 0) {
				uint8_t subBlockSize = 0;
				if (!ImageLoaderGifReadByte(reader, &subBlockSize)) {
					isCutOff = true;
				} else if (subBlockSize == 0) {
					isTerminated = true;
				}
				subBlockRemaining = subBlockSize;
				continue;
			}
			uint8_t dataByte = 0;
			if (!ImageLoaderGifReadByte(reader, &dataByte)) {
				isCutOff = true;
				continue;
			}
			--subBlockRemaining;
			bitBuffer |= (uint32_t)dataByte << bitCount;
			bitCount += ImageLoaderGifBitsPerByte;
		}
		if (bitCount < codeSize) {
			break;
		}
		uint32_t codeMask = (1u << codeSize) - 1;
		uint32_t code = bitBuffer & codeMask;
		bitBuffer >>= codeSize;
		bitCount -= codeSize;
		if (code == clearCode) {
			codeSize = initialCodeSize;
			nextCode = firstFreeCode;
			previousCode = noCode;
			continue;
		}
		if (code == endCode) {
			break;
		}
		if (previousCode == noCode) {
			// The first code after a clear is a single color index
			if (code >= clearCode) {
				break;
			}
			ImageLoaderGifWritePixel(writer, (uint8_t)code);
			previousCode = code;
			previousFirstByte = (uint8_t)code;
			continue;
		}
		uint32_t stackSize = 0;
		uint32_t stringCode = code;
		if (code == nextCode) {
			// The string this very code adds: the previous string plus its own first byte
			decoder->stringStack[stackSize++] = previousFirstByte;
			stringCode = previousCode;
		} else if (code > nextCode) {
			break;
		}
		while (stringCode >= firstFreeCode) {
			decoder->stringStack[stackSize++] = decoder->suffixBytes[stringCode];
			stringCode = decoder->prefixCodes[stringCode];
		}
		uint8_t firstByte = (uint8_t)stringCode;
		decoder->stringStack[stackSize++] = firstByte;
		while (stackSize > 0 && writer->remainingPixelCount > 0) {
			--stackSize;
			ImageLoaderGifWritePixel(writer, decoder->stringStack[stackSize]);
		}
		if (nextCode < IMAGE_LOADER_GIF_CODE_LIMIT) {
			decoder->prefixCodes[nextCode] = (uint16_t)previousCode;
			decoder->suffixBytes[nextCode] = firstByte;
			++nextCode;
			uint32_t codeSizeLimit = 1u << codeSize;
			if (nextCode == codeSizeLimit && codeSize < ImageLoaderGifLargestCodeSize) {
				++codeSize;
			}
		}
		previousCode = code;
		previousFirstByte = firstByte;
	}
	if (isCutOff) {
		return(false);
	}
	if (isTerminated) {
		return(true);
	}
	// The end code, a broken code or all pixels: the rest of the data up to its empty sub-block is skipped
	bool isSkipped = ImageLoaderGifSkipBytes(reader, subBlockRemaining) && ImageLoaderGifSkipSubBlocks(reader);
	return(isSkipped);
}

// The frame area on the canvas, empty when the frame lies outside
static ImageLoaderGifRect ImageLoaderGifClipFrame(const ImageLoaderGifDecoder *decoder, const ImageLoaderGifFrameHeader *header) {
	ImageLoaderGifRect result;
	memset(&result, 0, sizeof(result));
	if (header->left < decoder->width && header->top < decoder->height) {
		uint32_t availableWidth = decoder->width - header->left;
		uint32_t availableHeight = decoder->height - header->top;
		result.left = header->left;
		result.top = header->top;
		result.width = header->width < availableWidth ? header->width : availableWidth;
		result.height = header->height < availableHeight ? header->height : availableHeight;
	}
	return(result);
}

static size_t ImageLoaderGifRectOffset(const ImageLoaderGifDecoder *decoder, const ImageLoaderGifRect *rect, const uint32_t row) {
	size_t canvasStride = (size_t)decoder->width * IMAGE_LOADER_GIF_CHANNELS;
	size_t result = (size_t)(rect->top + row) * canvasStride + (size_t)rect->left * IMAGE_LOADER_GIF_CHANNELS;
	return(result);
}

// The disposal of the frame drawn last, before the next frame is drawn
static void ImageLoaderGifDisposeLastFrame(ImageLoaderGifDecoder *decoder) {
	const ImageLoaderGifRect *rect = &decoder->lastRect;
	size_t rowSize = (size_t)rect->width * IMAGE_LOADER_GIF_CHANNELS;
	if (decoder->lastDisposal == ImageLoaderGifDisposal_Background) {
		for (uint32_t row = 0; row < rect->height; ++row) {
			size_t offset = ImageLoaderGifRectOffset(decoder, rect, row);
			memset(decoder->canvas + offset, 0, rowSize);
		}
	} else if (decoder->lastDisposal == ImageLoaderGifDisposal_Previous && decoder->previousCanvas != NULL) {
		for (uint32_t row = 0; row < rect->height; ++row) {
			size_t offset = ImageLoaderGifRectOffset(decoder, rect, row);
			memcpy(decoder->canvas + offset, decoder->previousCanvas + offset, rowSize);
		}
	}
	decoder->lastDisposal = ImageLoaderGifDisposal_Unspecified;
	memset(&decoder->lastRect, 0, sizeof(decoder->lastRect));
}

static size_t ImageLoaderGifCanvasSize(const ImageLoaderGifDecoder *decoder) {
	size_t result = (size_t)decoder->width * (size_t)decoder->height * IMAGE_LOADER_GIF_CHANNELS;
	return(result);
}

// Draws the frame whose header was just read onto the canvas, after the disposal of the frame before
static ImageLoaderGifFrameResult ImageLoaderGifDrawFrame(ImageLoaderGifDecoder *decoder, const ImageLoaderGifFrameHeader *header) {
	size_t canvasSize = ImageLoaderGifCanvasSize(decoder);
	if (decoder->canvas == NULL) {
		decoder->canvas = (uint8_t *)calloc(1, canvasSize);
		if (decoder->canvas == NULL) {
			return(ImageLoaderGifFrameResult_OutOfMemory);
		}
	}
	ImageLoaderGifDisposeLastFrame(decoder);
	ImageLoaderGifRect rect = ImageLoaderGifClipFrame(decoder, header);
	if (header->disposal == ImageLoaderGifDisposal_Previous) {
		if (decoder->previousCanvas == NULL) {
			decoder->previousCanvas = (uint8_t *)malloc(canvasSize);
			if (decoder->previousCanvas == NULL) {
				return(ImageLoaderGifFrameResult_OutOfMemory);
			}
		}
		size_t rowSize = (size_t)rect.width * IMAGE_LOADER_GIF_CHANNELS;
		for (uint32_t row = 0; row < rect.height; ++row) {
			size_t offset = ImageLoaderGifRectOffset(decoder, &rect, row);
			memcpy(decoder->previousCanvas + offset, decoder->canvas + offset, rowSize);
		}
	}
	ImageLoaderGifPixelWriter writer;
	memset(&writer, 0, sizeof(writer));
	writer.canvas = decoder->canvas;
	writer.palette = header->hasLocalPalette ? decoder->localPalette : decoder->globalPalette;
	writer.canvasStride = (size_t)decoder->width * IMAGE_LOADER_GIF_CHANNELS;
	writer.canvasWidth = decoder->width;
	writer.canvasHeight = decoder->height;
	writer.frameLeft = header->left;
	writer.frameTop = header->top;
	writer.frameWidth = header->width;
	writer.frameHeight = header->height;
	writer.transparentIndex = header->transparentIndex;
	writer.isInterlaced = header->isInterlaced;
	writer.remainingPixelCount = (uint64_t)header->width * (uint64_t)header->height;
	bool isComplete = ImageLoaderGifDecodeImageData(decoder, &writer);
	decoder->lastDisposal = header->disposal;
	decoder->lastRect = rect;
	ImageLoaderGifFrameResult result = isComplete ? ImageLoaderGifFrameResult_Complete : ImageLoaderGifFrameResult_CutOff;
	return(result);
}

static ImageLoaderGifFrameResult ImageLoaderGifComposeNextFrame(ImageLoaderGifDecoder *decoder, ImageLoaderGifFrameHeader *outHeader) {
	if (!ImageLoaderGifReadFrameHeader(decoder, outHeader)) {
		return(ImageLoaderGifFrameResult_None);
	}
	ImageLoaderGifFrameResult result = ImageLoaderGifDrawFrame(decoder, outHeader);
	return(result);
}

// --- Decoder ---------------------------------------------------------------------------------------------------------------

static void ImageLoaderGifDestroyDecoder(ImageLoaderGifDecoder *decoder) {
	if (decoder == NULL) {
		return;
	}
	free(decoder->canvas);
	free(decoder->previousCanvas);
	free(decoder->reader.buffer);
	free(decoder);
}

// A decoder that reads through the source, or from the file in memory when source is null
static ImageLoadResult ImageLoaderGifCreateDecoder(ImageSource *source, const uint8_t *file, const size_t fileSize, ImageLoaderGifDecoder **outDecoder, char *message, size_t messageSize) {
	*outDecoder = NULL;
	ImageLoaderGifDecoder *decoder = (ImageLoaderGifDecoder *)calloc(1, sizeof(ImageLoaderGifDecoder));
	if (decoder == NULL) {
		snprintf(message, messageSize, "no memory for the decoder");
		return(ImageLoadResult_OutOfMemory);
	}
	if (source != NULL) {
		decoder->reader.buffer = (uint8_t *)malloc(ImageLoaderGifReadBufferSize);
		if (decoder->reader.buffer == NULL) {
			free(decoder);
			snprintf(message, messageSize, "no memory for the read buffer");
			return(ImageLoadResult_OutOfMemory);
		}
		decoder->reader.source = source;
		decoder->reader.data = decoder->reader.buffer;
	} else {
		decoder->reader.data = file;
		decoder->reader.dataSize = fileSize;
	}
	*outDecoder = decoder;
	return(ImageLoadResult_Success);
}

// Why the file ended: the source was canceled or the file is broken
static ImageLoadResult ImageLoaderGifEndResult(const ImageLoaderGifDecoder *decoder) {
	ImageSource *source = decoder->reader.source;
	bool isCanceled = source != NULL && source->isCanceled(source);
	ImageLoadResult result = isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt;
	return(result);
}

// The header of the first frame, which also fixes the canvas size
static ImageLoadResult ImageLoaderGifReadFirstFrameHeader(ImageLoaderGifDecoder *decoder, ImageLoaderGifFrameHeader *outHeader, char *message, size_t messageSize) {
	if (!ImageLoaderGifReadFrameHeader(decoder, outHeader)) {
		snprintf(message, messageSize, "no picture in the file");
		ImageLoadResult endResult = ImageLoaderGifEndResult(decoder);
		return(endResult);
	}
	if (decoder->width == 0 || decoder->height == 0) {
		snprintf(message, messageSize, "empty canvas");
		return(ImageLoadResult_Corrupt);
	}
	uint64_t pixelCount = (uint64_t)decoder->width * (uint64_t)decoder->height;
	if (pixelCount > ImageLoaderGifMaximumPixelCount) {
		snprintf(message, messageSize, "canvas of %u x %u is too large", decoder->width, decoder->height);
		return(ImageLoadResult_Unsupported);
	}
	return(ImageLoadResult_Success);
}

// Back to the first frame on a transparent canvas
static void ImageLoaderGifRestart(ImageLoaderGifDecoder *decoder, const size_t firstBlockOffset) {
	decoder->reader.position = firstBlockOffset;
	decoder->reader.isEnd = false;
	if (decoder->canvas != NULL) {
		size_t canvasSize = ImageLoaderGifCanvasSize(decoder);
		memset(decoder->canvas, 0, canvasSize);
	}
	decoder->lastDisposal = ImageLoaderGifDisposal_Unspecified;
	memset(&decoder->lastRect, 0, sizeof(decoder->lastRect));
}

// The whole file, for an animation that composes its frames long after the source is closed
static ImageLoadResult ImageLoaderGifReadFile(ImageSource *source, uint8_t **outFile, size_t *outFileSize, char *message, size_t messageSize) {
	uint64_t sourceSize = source->size(source);
	if (sourceSize > ImageLoaderGifMaximumAnimationFileSize) {
		snprintf(message, messageSize, "file of %llu bytes is too large for an animation", (unsigned long long)sourceSize);
		return(ImageLoadResult_Unsupported);
	}
	size_t fileSize = (size_t)sourceSize;
	uint8_t *file = (uint8_t *)malloc(fileSize > 0 ? fileSize : 1);
	if (file == NULL) {
		snprintf(message, messageSize, "no memory for %zu bytes", fileSize);
		return(ImageLoadResult_OutOfMemory);
	}
	source->seek(source, 0);
	size_t position = 0;
	while (position < fileSize) {
		size_t readSize = source->read(source, file + position, fileSize - position);
		if (readSize == 0 || source->isCanceled(source)) {
			free(file);
			bool isCanceled = source->isCanceled(source);
			const char *reason = isCanceled ? "canceled" : "file ends early";
			snprintf(message, messageSize, "%s", reason);
			return(isCanceled ? ImageLoadResult_Canceled : ImageLoadResult_Corrupt);
		}
		position += readSize;
	}
	*outFile = file;
	*outFileSize = fileSize;
	return(ImageLoadResult_Success);
}

// --- Loader ----------------------------------------------------------------------------------------------------------------

static ImageLoaderMatch ImageLoaderGifProbe(const ImageLoader *loader, const uint8_t *header, size_t headerSize, const char *fileExtension) {
	if (ImageLoaderGifIsSignature(header, headerSize)) {
		return(ImageLoaderMatch_Signature);
	}
	if (ImageLoaderListsExtension(loader, fileExtension)) {
		return(ImageLoaderMatch_Extension);
	}
	return(ImageLoaderMatch_None);
}

static ImageLoadResult ImageLoaderGifReadInfo(const ImageLoader *loader, ImageSource *source, PictureInfo *outInfo, char *message, size_t messageSize) {
	ImageLoaderGifDecoder *decoder = NULL;
	ImageLoadResult result = ImageLoaderGifCreateDecoder(source, NULL, 0, &decoder, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	ImageLoaderGifFrameHeader header;
	result = ImageLoaderGifReadScreen(decoder, message, messageSize);
	if (result == ImageLoadResult_Success) {
		result = ImageLoaderGifReadFirstFrameHeader(decoder, &header, message, messageSize);
	}
	if (result == ImageLoadResult_Success) {
		// Transparent where the first frame says so or where it leaves the canvas empty
		bool isCoveringCanvas = header.left == 0 && header.top == 0 && header.width >= decoder->width && header.height >= decoder->height;
		bool hasAlpha = header.transparentIndex >= 0 || !isCoveringCanvas;
		outInfo->width = decoder->width;
		outInfo->height = decoder->height;
		outInfo->channelCount = hasAlpha ? IMAGE_LOADER_GIF_CHANNELS : ImageLoaderGifRgbChannels;
		outInfo->bitsPerPixel = header.paletteBits > 0 ? header.paletteBits : ImageLoaderGifDefaultPaletteBits;
		outInfo->formatName = "GIF";
		outInfo->orientation = ImageOrientation_Normal;
		outInfo->isPalette = true;
	}
	ImageLoaderGifDestroyDecoder(decoder);
	return(result);
}

static ImageLoadResult ImageLoaderGifDecode(const ImageLoader *loader, ImageSource *source, ImagePixels *outPixels, char *message, size_t messageSize) {
	ImageLoaderGifDecoder *decoder = NULL;
	ImageLoadResult result = ImageLoaderGifCreateDecoder(source, NULL, 0, &decoder, message, messageSize);
	if (result != ImageLoadResult_Success) {
		return(result);
	}
	ImageLoaderGifFrameHeader header;
	result = ImageLoaderGifReadScreen(decoder, message, messageSize);
	if (result == ImageLoadResult_Success) {
		result = ImageLoaderGifReadFirstFrameHeader(decoder, &header, message, messageSize);
	}
	if (result == ImageLoadResult_Success) {
		// A first frame cut off by the end of the file shows what it has
		ImageLoaderGifFrameResult frameResult = ImageLoaderGifDrawFrame(decoder, &header);
		bool isCanceled = source->isCanceled(source);
		if (frameResult == ImageLoaderGifFrameResult_OutOfMemory) {
			snprintf(message, messageSize, "no memory for a canvas of %u x %u", decoder->width, decoder->height);
			result = ImageLoadResult_OutOfMemory;
		} else if (isCanceled) {
			snprintf(message, messageSize, "canceled");
			result = ImageLoadResult_Canceled;
		}
	}
	if (result == ImageLoadResult_Success) {
		outPixels->pixels = decoder->canvas;
		outPixels->width = decoder->width;
		outPixels->height = decoder->height;
		outPixels->stride = decoder->width * IMAGE_LOADER_GIF_CHANNELS;
		outPixels->format = ImagePixelFormat_RGBA8_SRGB;
		decoder->canvas = NULL;
	}
	ImageLoaderGifDestroyDecoder(decoder);
	return(result);
}

static void ImageLoaderGifRelease(const ImageLoader *loader, ImagePixels *pixels) {
	free(pixels->pixels);
}

static void ImageLoaderGifCloseAnimation(const ImageLoader *loader, void *animationPointer) {
	ImageLoaderGifAnimation *animation = (ImageLoaderGifAnimation *)animationPointer;
	if (animation == NULL) {
		return;
	}
	ImageLoaderGifDestroyDecoder(animation->decoder);
	free(animation->file);
	free(animation);
}

// Counts the complete frames from the first one, whose header was just read
static uint32_t ImageLoaderGifCountFrames(ImageLoaderGifDecoder *decoder) {
	uint32_t result = 0;
	ImageLoaderGifFrameHeader header;
	do {
		if (!ImageLoaderGifSkipImageData(&decoder->reader)) {
			break;
		}
		++result;
	} while (ImageLoaderGifReadFrameHeader(decoder, &header));
	return(result);
}

static ImageLoadResult ImageLoaderGifOpenAnimation(const ImageLoader *loader, ImageSource *source, void **outAnimation, ImageAnimationInfo *outInfo, char *message, size_t messageSize) {
	ImageLoaderGifAnimation *animation = (ImageLoaderGifAnimation *)calloc(1, sizeof(ImageLoaderGifAnimation));
	if (animation == NULL) {
		snprintf(message, messageSize, "no memory for the animation");
		return(ImageLoadResult_OutOfMemory);
	}
	ImageLoadResult result = ImageLoaderGifReadFile(source, &animation->file, &animation->fileSize, message, messageSize);
	if (result == ImageLoadResult_Success) {
		result = ImageLoaderGifCreateDecoder(NULL, animation->file, animation->fileSize, &animation->decoder, message, messageSize);
	}
	if (result == ImageLoadResult_Success) {
		result = ImageLoaderGifReadScreen(animation->decoder, message, messageSize);
	}
	ImageLoaderGifFrameHeader header;
	if (result == ImageLoadResult_Success) {
		animation->firstBlockOffset = animation->decoder->reader.position;
		result = ImageLoaderGifReadFirstFrameHeader(animation->decoder, &header, message, messageSize);
	}
	if (result == ImageLoadResult_Success) {
		animation->frameCount = ImageLoaderGifCountFrames(animation->decoder);
		if (animation->frameCount < 2) {
			snprintf(message, messageSize, "a single frame");
			result = ImageLoadResult_Unsupported;
		}
	}
	if (result != ImageLoadResult_Success) {
		ImageLoaderGifCloseAnimation(loader, animation);
		return(result);
	}
	ImageLoaderGifRestart(animation->decoder, animation->firstBlockOffset);
	const uint32_t playOnce = 1;
	const uint32_t playForever = 0;
	uint32_t playCount = playOnce;
	if (animation->decoder->hasLoopCount) {
		// The loop count repeats the animation after its first play, 0 repeats it forever
		uint32_t loopCount = animation->decoder->loopCount;
		playCount = loopCount == 0 ? playForever : loopCount + 1;
	}
	outInfo->frameCount = animation->frameCount;
	outInfo->playCount = playCount;
	*outAnimation = animation;
	return(ImageLoadResult_Success);
}

static ImageLoadResult ImageLoaderGifDecodeFrame(const ImageLoader *loader, void *animationPointer, uint32_t frameIndex, ImageFrame *outFrame, char *message, size_t messageSize) {
	ImageLoaderGifAnimation *animation = (ImageLoaderGifAnimation *)animationPointer;
	ImageLoaderGifDecoder *decoder = animation->decoder;
	if (frameIndex >= animation->frameCount) {
		snprintf(message, messageSize, "no frame %u in %u frames", frameIndex, animation->frameCount);
		return(ImageLoadResult_Unsupported);
	}
	bool isOnCanvas = animation->composedFrameCount == frameIndex + 1;
	if (!isOnCanvas) {
		bool isBehind = frameIndex < animation->composedFrameCount || animation->composedFrameCount == 0;
		if (isBehind) {
			ImageLoaderGifRestart(decoder, animation->firstBlockOffset);
			animation->composedFrameCount = 0;
		}
		while (animation->composedFrameCount <= frameIndex) {
			ImageLoaderGifFrameHeader header;
			ImageLoaderGifFrameResult frameResult = ImageLoaderGifComposeNextFrame(decoder, &header);
			if (frameResult != ImageLoaderGifFrameResult_Complete) {
				// The next call starts again from the first frame
				uint32_t failedFrameIndex = animation->composedFrameCount;
				animation->composedFrameCount = 0;
				bool isOutOfMemory = frameResult == ImageLoaderGifFrameResult_OutOfMemory;
				snprintf(message, messageSize, "frame %u cannot be composed", failedFrameIndex);
				return(isOutOfMemory ? ImageLoadResult_OutOfMemory : ImageLoadResult_Corrupt);
			}
			++animation->composedFrameCount;
			animation->lastDurationMilliseconds = header.durationMilliseconds;
		}
	}
	outFrame->pixels.pixels = decoder->canvas;
	outFrame->pixels.width = decoder->width;
	outFrame->pixels.height = decoder->height;
	outFrame->pixels.stride = decoder->width * IMAGE_LOADER_GIF_CHANNELS;
	outFrame->pixels.format = ImagePixelFormat_RGBA8_SRGB;
	outFrame->frameIndex = frameIndex;
	outFrame->durationMilliseconds = animation->lastDurationMilliseconds;
	return(ImageLoadResult_Success);
}

extern const ImageLoader *ImageLoaderGifGet(void) {
	static ImageLoader loader;
	static bool isInitialized = false;
	if (!isInitialized) {
		// 37544c19-2b92-4663-8357-0c5825c596f8
		const ImageLoaderId id = { { 0x37, 0x54, 0x4c, 0x19, 0x2b, 0x92, 0x46, 0x63, 0x83, 0x57, 0x0c, 0x58, 0x25, 0xc5, 0x96, 0xf8 } };
		memset(&loader, 0, sizeof(loader));
		loader.interfaceVersion = IMAGE_LOADER_INTERFACE_VERSION;
		loader.structSize = sizeof(ImageLoader);
		loader.id = id;
		loader.name = "gif";
		loader.version = "1.0";
		loader.flags = ImageLoaderFlags_None;
		loader.fileExtensions = ImageLoaderGifExtensions;
		loader.probe = ImageLoaderGifProbe;
		loader.readInfo = ImageLoaderGifReadInfo;
		loader.decode = ImageLoaderGifDecode;
		loader.releasePixels = ImageLoaderGifRelease;
		loader.openAnimation = ImageLoaderGifOpenAnimation;
		loader.decodeFrame = ImageLoaderGifDecodeFrame;
		loader.closeAnimation = ImageLoaderGifCloseAnimation;
		isInitialized = true;
	}
	return(&loader);
}

#endif // IMAGE_LOADER_GIF_IMPLEMENTATION
