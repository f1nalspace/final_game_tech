#pragma once

#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

struct BitmapInfoheader {
	fpl_u32 biSize;
	fpl_s32 biWidth;
	fpl_s32 biHeight;
	fpl_u16 biPlanes;
	fpl_u16 biBitCount;
	fpl_u32 biCompression;
	fpl_u32 biSizeImage;
	fpl_s32 biXPelsPerMeter;
	fpl_s32 biYPelsPerMeter;
	fpl_u32 biClrUsed;
	fpl_u32 biClrImportant;
};

#pragma pack(push,2)
struct BitmapFileHeader {
	fpl_u16 bfType;
	fpl_u32 bfSize;
	fpl_u16 bfReserved1;
	fpl_u16 bfReserved2;
	fpl_u32 bfOffBits;
};
#pragma pack(pop)

constexpr fpl_u32 BITMAP_FORMAT_RGB = 0L;

static void SaveBitmapRGB24(fpl_u8 *source, fpl_u32 width, fpl_u32 height, fpl_u32 scanline, const char *targetFilePath) {
	assert(scanline == (width * 3));

	BitmapInfoheader bih = {};
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biCompression = BITMAP_FORMAT_RGB;
	bih.biHeight = -(fpl_s32)height;
	bih.biWidth = width;
	bih.biPlanes = 1;
	bih.biSizeImage = scanline * height;
	bih.biSize = sizeof(BitmapInfoheader);

	BitmapFileHeader bfh = {};
	bfh.bfType = ((fpl_u16)('M' << 8) | 'B');
	bfh.bfSize = (fpl_u32)(sizeof(BitmapFileHeader) + bih.biSize + bih.biSizeImage);
	bfh.bfOffBits = (fpl_u32)(sizeof(BitmapFileHeader) + bih.biSize);

	FileHandle handle = CreateBinaryFile(targetFilePath);
	if (handle.isValid) {
		WriteFileBlock32(handle, &bfh, sizeof(BitmapFileHeader));
		WriteFileBlock32(handle, &bih, sizeof(BitmapInfoheader));
		WriteFileBlock32(handle, source, bih.biSizeImage);
		CloseFile(handle);
	}
}

static void ConvertRGB24ToBackBuffer(VideoBackBuffer *backbuffer, int width, int height, int sourceScanLine, fpl_u8 *sourceData) {
	for (int y = 0; y < height; ++y) {
		fpl_u8 *src = sourceData + y * sourceScanLine;
		int invertY = height - 1 - y;
		fpl_u32 *dst = (fpl_u32 *)((fpl_u8 *)backbuffer->pixels + invertY * backbuffer->stride);
		for (int x = 0; x < width; ++x) {
			fpl_u8 r = *src++;
			fpl_u8 g = *src++;
			fpl_u8 b = *src++;
			fpl_u8 a = 255;
			*dst++ = (a << 24) | (b << 16) | (g << 8) | r;
		}
	}
}

// See: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline fpl_size GetNextPowerOfTwo(fpl_size x) {
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

static inline fpl_size GetPrevPowerOfTwo(fpl_size x) {
	return GetNextPowerOfTwo(x) >> 1;
}
static inline fpl_size RoundToPowerOfTwo(fpl_size x) {
	fpl_size prev = GetPrevPowerOfTwo(x);
	fpl_size next = GetNextPowerOfTwo(x);
	if ((next - x) > (x - prev)) {
		return prev;
	} else {
		return next;
	}
}