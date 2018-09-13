#pragma once

#include <final_platform_layer.h>

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

constexpr uint32_t BITMAP_FORMAT_RGB = 0L;

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

	fplFileHandle handle;
	if (fplCreateBinaryFile(targetFilePath, &handle)) {
		fplWriteFileBlock32(&handle, &bfh, sizeof(BitmapFileHeader));
		fplWriteFileBlock32(&handle, &bih, sizeof(BitmapInfoheader));
		fplWriteFileBlock32(&handle, source, bih.biSizeImage);
		fplCloseFile(&handle);
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

inline uint8_t ClipByte(float v) {
	if (v < 0)
		return 0;
	if (v > 255)
		return 255;
	return (uint8_t)v;
}

inline uint32_t YUVToRGB32(const uint8_t y, const uint8_t u, const uint8_t v, const bool isBGRA) {
	float r = (1.164f * (float)(y - 16)) + (2.018f * (float)(v - 128));
	float g = (1.164f * (float)(y - 16)) - (0.813f * (float)(u - 128)) - (0.391f * (float)(v - 128));
	float b = (1.164f * (float)(y - 16)) + (1.596f * (float)(u - 128));
	uint32_t result;
	if (isBGRA) {
		result = ((uint8_t)255 << 24) | (ClipByte(b) << 16) | (ClipByte(g) << 8) | ClipByte(r);
	} else {
		result = ((uint8_t)255 << 24) | (ClipByte(r) << 16) | (ClipByte(g) << 8) | ClipByte(b);
	}
	return(result);
}

enum class ConversionFlags : uint32_t {
	None = 0,
	DstBGRA = 1 << 0,
};
FPL_ENUM_AS_FLAGS_OPERATORS(ConversionFlags);

static void ConvertYUV420PToRGB32(uint8_t *destData[8], int32_t destLineSize[8], int32_t width, int32_t height, uint8_t *sourceData[8], int32_t sourceLineSize[8], const ConversionFlags flags) {
	// Planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
	constexpr uint32_t YPLANE = 0;
	constexpr uint32_t UPLANE = 1;
	constexpr uint32_t VPLANE = 2;
	const bool dstBGRA = (flags & ConversionFlags::DstBGRA) == ConversionFlags::DstBGRA;
	for (int32_t y = 0; y < height; ++y) {
		uint32_t *dst32 = (uint32_t *)((uint8_t *)destData[0] + y * destLineSize[0]);
		uint8_t *srcY = sourceData[YPLANE] + y * sourceLineSize[YPLANE];
		uint8_t *srcU = sourceData[UPLANE] + (y / 2) * sourceLineSize[UPLANE];
		uint8_t *srcV = sourceData[VPLANE] + (y / 2) * sourceLineSize[VPLANE];
		for (int32_t x = 0; x < width; ++x) {
			uint8_t yComponent = *(srcY + x * sizeof(uint8_t));
			uint8_t uComponent = *(srcU + (x / 2) * sizeof(uint8_t));
			uint8_t vComponent = *(srcV + (x / 2) * sizeof(uint8_t));
			*dst32++ = YUVToRGB32(yComponent, uComponent, vComponent, dstBGRA);
		}
	}
}

static void ConvertRGB24ToRGB32(uint8_t *destData, int32_t destScanline, int32_t width, int32_t height, int32_t sourceScanLine, uint8_t *sourceData) {
	for (int32_t y = 0; y < height; ++y) {
		uint8_t *src = sourceData + y * sourceScanLine;
		uint32_t *dst = (uint32_t *)((uint8_t *)destData + y * destScanline);
		for (int32_t x = 0; x < width; ++x) {
			uint8_t r = *(src + 0);
			uint8_t g = *(src + 1);
			uint8_t b = *(src + 2);
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