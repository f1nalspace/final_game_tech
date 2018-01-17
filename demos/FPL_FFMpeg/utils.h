#pragma once

#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

struct BitmapInfoheader {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

#pragma pack(push,2)
struct BitmapFileHeader {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};
#pragma pack(pop)

fpl_constant uint32_t BITMAP_FORMAT_RGB = 0L;

static void SaveBitmapRGB24(uint8_t *source, uint32_t width, uint32_t height, uint32_t scanline, const char *targetFilePath) {
	assert(scanline == (width * 3));

	BitmapInfoheader bih = {};
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biCompression = BITMAP_FORMAT_RGB;
	bih.biHeight = -(int32_t)height;
	bih.biWidth = width;
	bih.biPlanes = 1;
	bih.biSizeImage = scanline * height;
	bih.biSize = sizeof(BitmapInfoheader);

	BitmapFileHeader bfh = {};
	bfh.bfType = ((uint16_t)('M' << 8) | 'B');
	bfh.bfSize = (uint32_t)(sizeof(BitmapFileHeader) + bih.biSize + bih.biSizeImage);
	bfh.bfOffBits = (uint32_t)(sizeof(BitmapFileHeader) + bih.biSize);

	FileHandle handle = CreateBinaryFile(targetFilePath);
	if (handle.isValid) {
		WriteFileBlock32(handle, &bfh, sizeof(BitmapFileHeader));
		WriteFileBlock32(handle, &bih, sizeof(BitmapInfoheader));
		WriteFileBlock32(handle, source, bih.biSizeImage);
		CloseFile(handle);
	}
}

static void FillRGB32TestColor(uint8_t *destData, int32_t destScanline, int32_t width, int32_t height) {
	for (int32_t y = 0; y < height; ++y) {
		int32_t invertY = height - 1 - y;
		uint32_t *dst = (uint32_t *)((uint8_t *)destData + invertY * destScanline);
		for (int32_t x = 0; x < width; ++x) {
			uint8_t r, g, b;
			if (y < (height / 2)) {
				r = x < (width / 2) ? 255 : 0;
				g = 0;
				b = x > (width / 2) ? 255 : 0;
			} else {
				r = x > (width / 2) ? 255 : 0;
				g = x < (width / 2) ? 255 : 0;
				b = x > (width / 2) ? 255 : 0;
			}
			uint8_t a = 255;
			*dst++ = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}
}

static void ConvertRGB24ToRGB32(uint8_t *destData, int32_t destScanline, int32_t width, int32_t height, int32_t sourceScanLine, uint8_t *sourceData, bool flipY, bool isBGRA) {
	uint32_t rindex = 0;
	uint32_t gindex = 1;
	uint32_t bindex = 2;
	if (isBGRA) {
		rindex = 2;
		bindex = 0;
	}
	for (int32_t y = 0; y < height; ++y) {
		uint8_t *src = sourceData + y * sourceScanLine;
		int32_t yDst = height - 1 - y;
		uint32_t *dst = (uint32_t *)((uint8_t *)destData + yDst * destScanline);
		for (int32_t x = 0; x < width; ++x) {
			uint8_t r = *(src + rindex);
			uint8_t g = *(src + gindex);
			uint8_t b = *(src + bindex);
			uint8_t a = 255;
			*dst++ = (a << 24) | (b << 16) | (g << 8) | r;
			src += 3;
		}
	}
}

// See: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline size_t GetNextPowerOfTwo(size_t x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
#if defined(FPL_ARCH_X64)
	x |= x >> 32;
#endif
	x++;
	return x;
}

static inline size_t GetPrevPowerOfTwo(size_t x) {
	return GetNextPowerOfTwo(x) >> 1;
}
static inline size_t RoundToPowerOfTwo(size_t x) {
	size_t prev = GetPrevPowerOfTwo(x);
	size_t next = GetNextPowerOfTwo(x);
	if ((next - x) > (x - prev)) {
		return prev;
	} else {
		return next;
	}
}