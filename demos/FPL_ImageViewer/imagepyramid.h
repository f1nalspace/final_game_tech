/*
Name:
	FPL_ImageViewer | Image pyramid

Description:
	Builds the level of detail chain of a picture on the CPU, in the load thread. Every level halves the previous one, rounded up, until the longer side is at most 32 pixels.
	Pixel i of level L covers the picture pixels [i * 2^L, (i + 1) * 2^L) on each axis, so any level can stand in for the picture in the resample pipeline.
	Rounding up keeps the last column and row of an odd size inside the level, the part beyond the picture is filled the way the picture edge is filtered.

	One 2:1 reduction (ImagePyramidReduceHalf) filters in linear light with premultiplied alpha, in fixed point:
		1. Decode: sRGB8 to linear u15 (0..32767) through a 256 entry table, alpha is premultiplied, the rows are planar (R, G, B, A).
		2. Filter: the separable 2:1 kernel with 8 taps per axis and Q14 weights that sum to exactly 16384, first vertically over a ring of 8 decoded rows, then horizontally.
		   Output pixel x is centered at source position 2x + 1 (pixel edges at integers) and reads the taps 2x - 3 ... 2x + 4.
		   Taps outside the picture are left out and the remaining weights renormalized, the same edge handling as ImageMagick -resize.
		   Each pass rounds, shifts by 14 and saturates to int16, so the negative lobes survive between the passes.
		3. Encode: clamp to 0 <= rgb <= alpha, undo the premultiplication, linear u15 to sRGB8 through a 32768 entry table.
	The next level is reduced from the RGBA8 result of the previous one with the same function.

	Everything is integer arithmetic, so every SIMD level computes bit for bit what the scalar reference computes, --selftest checks exactly that.
	The architecture free driver calls the few hot row loops through a table (ImagePyramidRowFunctions) with one entry per SIMD level.
	The SIMD level is picked from fplCPUGetCapabilities() alone, and the only place that asks the FPL architecture macros is the IMAGE_PYRAMID_ARCH_* switch below.
	x86 row functions live in imagepyramid_x86.h, ARM (NEON) gets imagepyramid_arm.h later: four row functions, one table entry, one line in the level check.

Usage:
	Needs final_platform_layer.h (CPU capabilities). Define IMAGE_PYRAMID_IMPLEMENTATION in exactly one translation unit before including this header.
	Call ImagePyramidInitialize() once before any other function, it fills the shared lookup tables.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_PYRAMID_H
#define IMAGE_PYRAMID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <final_platform_layer.h>

// The one architecture switch: Apple Silicon only sets FPL_ARCH_APPLE_ARM64 and not FPL_ARCH_ARM64, everything else asks IMAGE_PYRAMID_ARCH_*
#if defined(FPL_ARCH_X64) || defined(FPL_ARCH_X86)
#	define IMAGE_PYRAMID_ARCH_X86
#	if defined(FPL_ARCH_X64)
		// Every x64 CPU has SSE2
#		define IMAGE_PYRAMID_ARCH_X86_SSE2_BASELINE
#	endif
#elif defined(FPL_ARCH_ARM64) || defined(FPL_ARCH_APPLE_ARM64) || defined(FPL_ARCH_ARM32)
#	define IMAGE_PYRAMID_ARCH_ARM
#endif

// Enough for a longer side of 32 * 2^23 pixels
#define IMAGE_PYRAMID_MAX_LEVELS 24
// Levels are made until the longer side is at most this
#define IMAGE_PYRAMID_SMALLEST_LONG_SIDE 32
// Taps per axis of the 2:1 kernel (radius 2, widened by 2)
#define IMAGE_PYRAMID_TAP_COUNT 8
// Planar rows: R, G, B, A
#define IMAGE_PYRAMID_CHANNEL_COUNT 4
// Entries of the linear u15 to sRGB8 table
#define IMAGE_PYRAMID_LINEAR_LEVELS 32768
// Entries of the sRGB8 to linear u15 table
#define IMAGE_PYRAMID_SRGB_LEVELS 256

// SIMD levels over all architectures, higher is faster within an architecture.
// AVX-512 has no row functions (yet): with AVX512F alone it was not faster than AVX2, requesting it falls back to AVX2.
typedef enum SimdLevel {
	SimdLevel_Scalar = 0,
	SimdLevel_X86_SSE2,
	SimdLevel_X86_AVX2,
	SimdLevel_X86_AVX512,
	SimdLevel_ARM_NEON,
	SimdLevel_Count,
	// Request only: the highest level that is compiled and supported by the CPU
	SimdLevel_Best = SimdLevel_Count,
} SimdLevel;

// Kernel of the 2:1 reduction, both have radius 2 and are widened by 2
typedef enum ImagePyramidKernel {
	ImagePyramidKernel_Mitchell = 0,
	ImagePyramidKernel_Lanczos2,
	ImagePyramidKernel_Count,
} ImagePyramidKernel;

typedef struct ImagePyramidKernelDefinition {
	const char *name;
	// Used on the command line
	const char *key;
} ImagePyramidKernelDefinition;

// RGBA8, sRGB color, straight alpha, top row first
typedef struct ImagePyramidLevel {
	uint8_t *pixels;
	uint32_t width;
	uint32_t height;
	// Bytes per row
	uint32_t stride;
} ImagePyramidLevel;

typedef struct ImagePyramid {
	// Level 0 is the picture itself and belongs to the caller, the other levels live in memory
	ImagePyramidLevel levels[IMAGE_PYRAMID_MAX_LEVELS];
	uint32_t levelCount;
	void *memory;
} ImagePyramid;

typedef struct ImagePyramidTables {
	// sRGB8 to linear u15
	int16_t srgbToLinear[IMAGE_PYRAMID_SRGB_LEVELS];
	// Alpha8 to u15, alpha is linear
	int16_t alphaToLinear[IMAGE_PYRAMID_SRGB_LEVELS];
	// Linear u15 to sRGB8
	uint8_t linearToSRGB[IMAGE_PYRAMID_LINEAR_LEVELS];
	// Q14 weights of an output pixel whose taps all lie inside the picture
	int16_t interiorWeights[ImagePyramidKernel_Count][IMAGE_PYRAMID_TAP_COUNT];
} ImagePyramidTables;

// Planar rows hold the channels R, G, B, A one after another, each planeStride values apart

// sRGB8 RGBA pixels to planar linear u15, premultiplied
typedef void ImagePyramidDecodeRowFunction(const uint8_t *rgba, const uint32_t width, int16_t *planes, const size_t planeStride, const ImagePyramidTables *tables);
// out[i] = saturate16((sum of weights[t] * rows[t][i] + 2^13) >> 14) for i in [0, length)
typedef void ImagePyramidFilterVerticalFunction(const int16_t *const *rows, const int16_t *weights, const uint32_t tapCount, const uint32_t length, int16_t *out);
// out[x] = saturate16((sum of weights[k] * row[2x - 3 + k] + 2^13) >> 14) for x in [firstOutput, endOutput), the caller guarantees that every tap lies inside the row
typedef void ImagePyramidFilterHorizontalHalfFunction(const int16_t *row, const int16_t *weights, const uint32_t firstOutput, const uint32_t endOutput, int16_t *out);
// Planar premultiplied linear u15 to sRGB8 RGBA pixels with straight alpha
typedef void ImagePyramidEncodeRowFunction(const int16_t *planes, const size_t planeStride, const uint32_t width, uint8_t *rgba, const ImagePyramidTables *tables);

typedef struct ImagePyramidRowFunctions {
	// "scalar", "sse2", "avx2", "avx512", "neon"
	const char *name;
	SimdLevel level;
	ImagePyramidDecodeRowFunction *decodeRowToLinear;
	ImagePyramidFilterVerticalFunction *filterRowsVertical;
	ImagePyramidFilterHorizontalHalfFunction *filterRowHorizontalHalf;
	ImagePyramidEncodeRowFunction *encodeRowToSRGB;
} ImagePyramidRowFunctions;

typedef void ImagePyramidProgressFunction(void *userData, const float fraction);

typedef struct ImagePyramidSettings {
	const ImagePyramidRowFunctions *functions;
	ImagePyramidKernel kernel;
	// Optional: the build stops between two blocks of rows once it turns true
	volatile bool *cancelFlag;
	// Optional: fraction 0..1 of the whole build, counted in output pixels
	ImagePyramidProgressFunction *progress;
	void *progressUserData;
} ImagePyramidSettings;

typedef enum ImagePyramidResult {
	ImagePyramidResult_Success = 0,
	ImagePyramidResult_Canceled,
	ImagePyramidResult_OutOfMemory,
} ImagePyramidResult;

// Fills the lookup tables, once before anything else and before any thread uses the pyramid
extern void ImagePyramidInitialize(void);
extern const ImagePyramidTables *ImagePyramidGetTables(void);

extern const ImagePyramidKernelDefinition *ImagePyramidGetKernelDefinition(const ImagePyramidKernel kernel);
extern bool ImagePyramidFindKernel(const char *key, ImagePyramidKernel *outKernel);

extern const char *ImagePyramidGetSimdLevelName(const SimdLevel level);
extern bool ImagePyramidFindSimdLevel(const char *name, SimdLevel *outLevel);
// Row functions of a level, null when the level is not compiled into this build
extern const ImagePyramidRowFunctions *ImagePyramidGetRowFunctions(const SimdLevel level);
// True when the level is compiled and the CPU runs it
extern bool ImagePyramidIsSimdLevelAvailable(const fplCPUCapabilities *capabilities, const SimdLevel level);
// The requested level when it is available, otherwise the next lower one that is, SimdLevel_Best takes the highest available level
extern const ImagePyramidRowFunctions *ImagePyramidSelectRowFunctions(const fplCPUCapabilities *capabilities, const SimdLevel requestedLevel);

// Number of levels including level 0: halving until the longer side is at most IMAGE_PYRAMID_SMALLEST_LONG_SIDE
extern uint32_t ImagePyramidComputeLevelCount(const uint32_t width, const uint32_t height);
// Size of one level, each level is the previous one halved and rounded up
extern void ImagePyramidComputeLevelSize(const uint32_t width, const uint32_t height, const uint32_t level, uint32_t *outWidth, uint32_t *outHeight);

// Q14 weights of the taps of one output pixel of a 2:1 reduction of length source pixels, returns the tap count; the first tap is written to outFirstTap
extern uint32_t ImagePyramidComputeTaps(const ImagePyramidKernel kernel, const int32_t output, const int32_t length, int32_t *outFirstTap, int16_t *outWeights);

// One 2:1 reduction into a target of ((width + 1) / 2) x ((height + 1) / 2) pixels, the target pixels must be allocated by the caller
extern ImagePyramidResult ImagePyramidReduceHalf(const ImagePyramidLevel *source, const ImagePyramidLevel *target, const ImagePyramidSettings *settings);

// All levels of the picture in baseLevel, which stays level 0 and is not copied. On failure only level 0 remains.
extern ImagePyramidResult ImagePyramidBuild(const ImagePyramidLevel *baseLevel, const ImagePyramidSettings *settings, ImagePyramid *outPyramid);
extern void ImagePyramidRelease(ImagePyramid *pyramid);

#endif // IMAGE_PYRAMID_H

#if defined(IMAGE_PYRAMID_IMPLEMENTATION) && !defined(IMAGE_PYRAMID_IMPLEMENTED)
#define IMAGE_PYRAMID_IMPLEMENTED

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Q14 weights, every set of weights sums to exactly ImagePyramidWeightOne
static const int32_t ImagePyramidWeightBits = 14;
static const int32_t ImagePyramidWeightOne = 1 << 14;
static const int32_t ImagePyramidWeightHalf = 1 << 13;
// Linear values are u15
static const int32_t ImagePyramidLinearMaximum = 32767;
static const int32_t ImagePyramidLinearBits = 15;
static const int32_t ImagePyramidPremultiplyHalf = 1 << 14;
// alpha + (alpha >> 14) turns u15 alpha into a factor of 0..32768 that is exactly 32768 for opaque pixels, so they keep their color
static const int32_t ImagePyramidAlphaFactorShift = 14;
static const int32_t ImagePyramidByteMaximum = 255;
static const int32_t ImagePyramidSaturateMinimum = -32768;
static const int32_t ImagePyramidSaturateMaximum = 32767;
// Output pixel x reads source taps from 2x + ImagePyramidFirstTapOffset on
static const int32_t ImagePyramidFirstTapOffset = -(IMAGE_PYRAMID_TAP_COUNT / 2 - 1);
// Center taps of a complete set, the rounding residue of a symmetric set is split between them
static const int32_t ImagePyramidLeftCenterTap = IMAGE_PYRAMID_TAP_COUNT / 2 - 1;
static const int32_t ImagePyramidRightCenterTap = IMAGE_PYRAMID_TAP_COUNT / 2;
static const double ImagePyramidKernelRadius = 2.0;
static const double ImagePyramidReductionFactor = 2.0;
// Cancel and progress are checked after this many output rows
static const uint32_t ImagePyramidRowsPerBlock = 16;
// Bytes per RGBA8 pixel
static const uint32_t ImagePyramidBytesPerPixel = 4;

static const ImagePyramidKernelDefinition ImagePyramidKernelDefinitions[ImagePyramidKernel_Count] = {
	{ "Mitchell", "mitchell" },
	{ "Lanczos2", "lanczos2" },
};

static const char *ImagePyramidSimdLevelNames[SimdLevel_Count] = {
	"scalar",
	"sse2",
	"avx2",
	"avx512",
	"neon",
};

static ImagePyramidTables ImagePyramidSharedTables;
static bool ImagePyramidIsInitialized = false;

// --- Shared scalar helpers, the SIMD functions finish their row tails with them -------------------------------------------

static int16_t ImagePyramidSaturate(const int32_t value) {
	int32_t clamped = value < ImagePyramidSaturateMinimum ? ImagePyramidSaturateMinimum : (value > ImagePyramidSaturateMaximum ? ImagePyramidSaturateMaximum : value);
	return((int16_t)clamped);
}

// Rounds a Q14 sum to the sample scale, the shift of a negative sum is arithmetic on every supported compiler
static int16_t ImagePyramidRoundWeightedSum(const int32_t sum) {
	int32_t shifted = (sum + ImagePyramidWeightHalf) >> ImagePyramidWeightBits;
	int16_t result = ImagePyramidSaturate(shifted);
	return(result);
}

static void ImagePyramidFilterVerticalRange(const int16_t *const *rows, const int16_t *weights, const uint32_t tapCount, const uint32_t first, const uint32_t end, int16_t *out) {
	if (tapCount == IMAGE_PYRAMID_TAP_COUNT) {
		// Every interior row: rows and weights in locals, so the loop needs no indirection per tap
		const int16_t *row0 = rows[0];
		const int16_t *row1 = rows[1];
		const int16_t *row2 = rows[2];
		const int16_t *row3 = rows[3];
		const int16_t *row4 = rows[4];
		const int16_t *row5 = rows[5];
		const int16_t *row6 = rows[6];
		const int16_t *row7 = rows[7];
		const int32_t weight0 = weights[0];
		const int32_t weight1 = weights[1];
		const int32_t weight2 = weights[2];
		const int32_t weight3 = weights[3];
		const int32_t weight4 = weights[4];
		const int32_t weight5 = weights[5];
		const int32_t weight6 = weights[6];
		const int32_t weight7 = weights[7];
		for (uint32_t index = first; index < end; ++index) {
			int32_t sum = weight0 * row0[index] + weight1 * row1[index] + weight2 * row2[index] + weight3 * row3[index] + weight4 * row4[index] + weight5 * row5[index] + weight6 * row6[index] + weight7 * row7[index];
			out[index] = ImagePyramidRoundWeightedSum(sum);
		}
		return;
	}
	for (uint32_t index = first; index < end; ++index) {
		int32_t sum = 0;
		for (uint32_t tap = 0; tap < tapCount; ++tap) {
			sum += (int32_t)weights[tap] * (int32_t)rows[tap][index];
		}
		out[index] = ImagePyramidRoundWeightedSum(sum);
	}
}

static void ImagePyramidFilterHorizontalHalfRange(const int16_t *row, const int16_t *weights, const uint32_t firstOutput, const uint32_t endOutput, int16_t *out) {
	const int32_t weight0 = weights[0];
	const int32_t weight1 = weights[1];
	const int32_t weight2 = weights[2];
	const int32_t weight3 = weights[3];
	const int32_t weight4 = weights[4];
	const int32_t weight5 = weights[5];
	const int32_t weight6 = weights[6];
	const int32_t weight7 = weights[7];
	for (uint32_t x = firstOutput; x < endOutput; ++x) {
		const int16_t *taps = row + 2 * (int32_t)x + ImagePyramidFirstTapOffset;
		int32_t sum = weight0 * taps[0] + weight1 * taps[1] + weight2 * taps[2] + weight3 * taps[3] + weight4 * taps[4] + weight5 * taps[5] + weight6 * taps[6] + weight7 * taps[7];
		out[x] = ImagePyramidRoundWeightedSum(sum);
	}
}

// --- Scalar reference row functions ------------------------------------------------------------------------------------------

static void ImagePyramidDecodeRangeScalar(const uint8_t *rgba, const uint32_t first, const uint32_t end, int16_t *planes, const size_t planeStride, const ImagePyramidTables *tables) {
	const uint8_t opaqueAlpha = (uint8_t)ImagePyramidByteMaximum;
	int16_t *red = planes;
	int16_t *green = planes + planeStride;
	int16_t *blue = planes + planeStride * 2;
	int16_t *alpha = planes + planeStride * 3;
	for (uint32_t x = first; x < end; ++x) {
		const uint8_t *pixel = rgba + (size_t)x * ImagePyramidBytesPerPixel;
		int16_t linearRed = tables->srgbToLinear[pixel[0]];
		int16_t linearGreen = tables->srgbToLinear[pixel[1]];
		int16_t linearBlue = tables->srgbToLinear[pixel[2]];
		if (pixel[3] == opaqueAlpha) {
			// The alpha factor is exactly 32768, premultiplying keeps the color
			red[x] = linearRed;
			green[x] = linearGreen;
			blue[x] = linearBlue;
			alpha[x] = (int16_t)ImagePyramidLinearMaximum;
			continue;
		}
		int32_t linearAlpha = tables->alphaToLinear[pixel[3]];
		int32_t alphaFactor = linearAlpha + (linearAlpha >> ImagePyramidAlphaFactorShift);
		red[x] = (int16_t)((linearRed * alphaFactor + ImagePyramidPremultiplyHalf) >> ImagePyramidLinearBits);
		green[x] = (int16_t)((linearGreen * alphaFactor + ImagePyramidPremultiplyHalf) >> ImagePyramidLinearBits);
		blue[x] = (int16_t)((linearBlue * alphaFactor + ImagePyramidPremultiplyHalf) >> ImagePyramidLinearBits);
		alpha[x] = (int16_t)linearAlpha;
	}
}

static void ImagePyramidDecodeRowScalar(const uint8_t *rgba, const uint32_t width, int16_t *planes, const size_t planeStride, const ImagePyramidTables *tables) {
	ImagePyramidDecodeRangeScalar(rgba, 0, width, planes, planeStride, tables);
}

static void ImagePyramidFilterVerticalScalar(const int16_t *const *rows, const int16_t *weights, const uint32_t tapCount, const uint32_t length, int16_t *out) {
	ImagePyramidFilterVerticalRange(rows, weights, tapCount, 0, length, out);
}

static void ImagePyramidFilterHorizontalHalfScalar(const int16_t *row, const int16_t *weights, const uint32_t firstOutput, const uint32_t endOutput, int16_t *out) {
	ImagePyramidFilterHorizontalHalfRange(row, weights, firstOutput, endOutput, out);
}

static int32_t ImagePyramidClampInt(const int32_t value, const int32_t minimum, const int32_t maximum) {
	int32_t result = value < minimum ? minimum : (value > maximum ? maximum : value);
	return(result);
}

// Straight linear color of a premultiplied one, color already clamped to [0, alpha]
static int32_t ImagePyramidUnpremultiply(const int32_t color, const int32_t alpha) {
	if (alpha == ImagePyramidLinearMaximum) {
		return(color);
	}
	if (alpha == 0) {
		return(0);
	}
	int32_t result = (color * ImagePyramidLinearMaximum + alpha / 2) / alpha;
	return(result);
}

static void ImagePyramidEncodeRangeScalar(const int16_t *planes, const size_t planeStride, const uint32_t first, const uint32_t end, uint8_t *rgba, const ImagePyramidTables *tables) {
	const int16_t *red = planes;
	const int16_t *green = planes + planeStride;
	const int16_t *blue = planes + planeStride * 2;
	const int16_t *alpha = planes + planeStride * 3;
	const int32_t alphaRoundingHalf = ImagePyramidLinearMaximum / 2;
	for (uint32_t x = first; x < end; ++x) {
		int32_t clampedAlpha = ImagePyramidClampInt(alpha[x], 0, ImagePyramidLinearMaximum);
		int32_t clampedRed = ImagePyramidClampInt(red[x], 0, clampedAlpha);
		int32_t clampedGreen = ImagePyramidClampInt(green[x], 0, clampedAlpha);
		int32_t clampedBlue = ImagePyramidClampInt(blue[x], 0, clampedAlpha);
		int32_t straightRed = ImagePyramidUnpremultiply(clampedRed, clampedAlpha);
		int32_t straightGreen = ImagePyramidUnpremultiply(clampedGreen, clampedAlpha);
		int32_t straightBlue = ImagePyramidUnpremultiply(clampedBlue, clampedAlpha);
		uint8_t *pixel = rgba + (size_t)x * ImagePyramidBytesPerPixel;
		pixel[0] = tables->linearToSRGB[straightRed];
		pixel[1] = tables->linearToSRGB[straightGreen];
		pixel[2] = tables->linearToSRGB[straightBlue];
		pixel[3] = (uint8_t)((clampedAlpha * ImagePyramidByteMaximum + alphaRoundingHalf) / ImagePyramidLinearMaximum);
	}
}

static void ImagePyramidEncodeRowScalar(const int16_t *planes, const size_t planeStride, const uint32_t width, uint8_t *rgba, const ImagePyramidTables *tables) {
	ImagePyramidEncodeRangeScalar(planes, planeStride, 0, width, rgba, tables);
}

static const ImagePyramidRowFunctions ImagePyramidScalarRowFunctions = {
	"scalar",
	SimdLevel_Scalar,
	ImagePyramidDecodeRowScalar,
	ImagePyramidFilterVerticalScalar,
	ImagePyramidFilterHorizontalHalfScalar,
	ImagePyramidEncodeRowScalar,
};

#if defined(IMAGE_PYRAMID_ARCH_X86)
#	include "imagepyramid_x86.h"
#endif

// --- Kernels and tables ------------------------------------------------------------------------------------------------------

// Mitchell-Netravali with B = C = 1/3, radius 2
static double ImagePyramidKernelMitchell(const double x) {
	const double B = 1.0 / 3.0;
	const double C = 1.0 / 3.0;
	const double denominator = 6.0;
	double a = fabs(x);
	if (a < 1.0) {
		double cubic = (12.0 - 9.0 * B - 6.0 * C) * a * a * a;
		double quadratic = (-18.0 + 12.0 * B + 6.0 * C) * a * a;
		double constant = 6.0 - 2.0 * B;
		double result = (cubic + quadratic + constant) / denominator;
		return(result);
	}
	if (a < ImagePyramidKernelRadius) {
		double cubic = (-B - 6.0 * C) * a * a * a;
		double quadratic = (6.0 * B + 30.0 * C) * a * a;
		double linear = (-12.0 * B - 48.0 * C) * a;
		double constant = 8.0 * B + 24.0 * C;
		double result = (cubic + quadratic + linear + constant) / denominator;
		return(result);
	}
	return(0.0);
}

static double ImagePyramidSinc(const double x) {
	const double pi = 3.14159265358979323846;
	if (x == 0.0) {
		return(1.0);
	}
	double t = pi * x;
	double sine = sin(t);
	double result = sine / t;
	return(result);
}

// Lanczos with 2 lobes
static double ImagePyramidKernelLanczos2(const double x) {
	double a = fabs(x);
	if (a >= ImagePyramidKernelRadius) {
		return(0.0);
	}
	double window = ImagePyramidSinc(a / ImagePyramidKernelRadius);
	double sinc = ImagePyramidSinc(a);
	double result = sinc * window;
	return(result);
}

static double ImagePyramidKernelWeight(const ImagePyramidKernel kernel, const double x) {
	double result;
	if (kernel == ImagePyramidKernel_Lanczos2) {
		result = ImagePyramidKernelLanczos2(x);
	} else {
		result = ImagePyramidKernelMitchell(x);
	}
	return(result);
}

extern uint32_t ImagePyramidComputeTaps(const ImagePyramidKernel kernel, const int32_t output, const int32_t length, int32_t *outFirstTap, int16_t *outWeights) {
	const double half = 0.5;
	int32_t firstTap = 2 * output + ImagePyramidFirstTapOffset;
	int32_t endTap = firstTap + IMAGE_PYRAMID_TAP_COUNT;
	int32_t clampedFirst = firstTap < 0 ? 0 : firstTap;
	int32_t clampedEnd = endTap > length ? length : endTap;
	uint32_t count = clampedEnd > clampedFirst ? (uint32_t)(clampedEnd - clampedFirst) : 0;
	*outFirstTap = clampedFirst;
	if (count == 0) {
		return(0);
	}

	// Output center 2 * output + 1, tap center tap + 0.5, the kernel is widened by the reduction factor
	double center = 2.0 * (double)output + 1.0;
	double weights[IMAGE_PYRAMID_TAP_COUNT];
	double weightSum = 0.0;
	for (uint32_t index = 0; index < count; ++index) {
		double tapCenter = (double)(clampedFirst + (int32_t)index) + half;
		double distance = (tapCenter - center) / ImagePyramidReductionFactor;
		weights[index] = ImagePyramidKernelWeight(kernel, distance);
		weightSum += weights[index];
	}

	int32_t roundedSum = 0;
	uint32_t largestIndex = 0;
	for (uint32_t index = 0; index < count; ++index) {
		double normalized = weights[index] / weightSum * (double)ImagePyramidWeightOne;
		double rounded = floor(normalized + half);
		outWeights[index] = (int16_t)rounded;
		roundedSum += outWeights[index];
		if (fabs(weights[index]) > fabs(weights[largestIndex])) {
			largestIndex = index;
		}
	}

	// The residue of a complete symmetric set is even and split between both center taps, so the set stays symmetric
	int32_t residue = ImagePyramidWeightOne - roundedSum;
	bool isComplete = count == IMAGE_PYRAMID_TAP_COUNT;
	if (isComplete && (residue % 2) == 0) {
		outWeights[ImagePyramidLeftCenterTap] = (int16_t)(outWeights[ImagePyramidLeftCenterTap] + residue / 2);
		outWeights[ImagePyramidRightCenterTap] = (int16_t)(outWeights[ImagePyramidRightCenterTap] + residue / 2);
	} else {
		outWeights[largestIndex] = (int16_t)(outWeights[largestIndex] + residue);
	}
	return(count);
}

static double ImagePyramidSRGBToLinear(const double value) {
	const double linearLimit = 0.04045;
	const double linearSlope = 12.92;
	const double offset = 0.055;
	const double gamma = 2.4;
	if (value <= linearLimit) {
		return(value / linearSlope);
	}
	double base = (value + offset) / (1.0 + offset);
	double result = pow(base, gamma);
	return(result);
}

static double ImagePyramidLinearToSRGB(const double value) {
	const double linearLimit = 0.0031308;
	const double linearSlope = 12.92;
	const double offset = 0.055;
	const double inverseGamma = 1.0 / 2.4;
	if (value <= linearLimit) {
		return(value * linearSlope);
	}
	double power = pow(value, inverseGamma);
	double result = (1.0 + offset) * power - offset;
	return(result);
}

extern void ImagePyramidInitialize(void) {
	const double half = 0.5;
	const int32_t interiorOutput = 2;
	const int32_t interiorLength = interiorOutput * 2 + IMAGE_PYRAMID_TAP_COUNT;
	if (ImagePyramidIsInitialized) {
		return;
	}
	ImagePyramidTables *tables = &ImagePyramidSharedTables;
	for (int32_t value = 0; value < IMAGE_PYRAMID_SRGB_LEVELS; ++value) {
		double normalized = (double)value / (double)ImagePyramidByteMaximum;
		double linear = ImagePyramidSRGBToLinear(normalized);
		double scaledLinear = floor(linear * (double)ImagePyramidLinearMaximum + half);
		double scaledAlpha = floor(normalized * (double)ImagePyramidLinearMaximum + half);
		tables->srgbToLinear[value] = (int16_t)scaledLinear;
		tables->alphaToLinear[value] = (int16_t)scaledAlpha;
	}
	for (int32_t value = 0; value < IMAGE_PYRAMID_LINEAR_LEVELS; ++value) {
		double normalized = (double)value / (double)ImagePyramidLinearMaximum;
		double srgb = ImagePyramidLinearToSRGB(normalized);
		double scaled = floor(srgb * (double)ImagePyramidByteMaximum + half);
		tables->linearToSRGB[value] = (uint8_t)scaled;
	}
	for (int32_t kernel = 0; kernel < ImagePyramidKernel_Count; ++kernel) {
		int32_t firstTap = 0;
		ImagePyramidComputeTaps((ImagePyramidKernel)kernel, interiorOutput, interiorLength, &firstTap, tables->interiorWeights[kernel]);
	}
	ImagePyramidIsInitialized = true;
}

extern const ImagePyramidTables *ImagePyramidGetTables(void) {
	fplAssert(ImagePyramidIsInitialized);
	return(&ImagePyramidSharedTables);
}

static bool ImagePyramidIsKeyEqual(const char *a, const char *b) {
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

extern const ImagePyramidKernelDefinition *ImagePyramidGetKernelDefinition(const ImagePyramidKernel kernel) {
	int index = (kernel >= 0 && kernel < ImagePyramidKernel_Count) ? (int)kernel : (int)ImagePyramidKernel_Mitchell;
	const ImagePyramidKernelDefinition *result = &ImagePyramidKernelDefinitions[index];
	return(result);
}

extern bool ImagePyramidFindKernel(const char *key, ImagePyramidKernel *outKernel) {
	for (int index = 0; index < ImagePyramidKernel_Count; ++index) {
		if (ImagePyramidIsKeyEqual(key, ImagePyramidKernelDefinitions[index].key)) {
			*outKernel = (ImagePyramidKernel)index;
			return(true);
		}
	}
	return(false);
}

// --- SIMD levels ---------------------------------------------------------------------------------------------------------------

extern const char *ImagePyramidGetSimdLevelName(const SimdLevel level) {
	if (level < SimdLevel_Scalar || level >= SimdLevel_Count) {
		return("best");
	}
	return(ImagePyramidSimdLevelNames[level]);
}

extern bool ImagePyramidFindSimdLevel(const char *name, SimdLevel *outLevel) {
	for (int index = 0; index < SimdLevel_Count; ++index) {
		if (ImagePyramidIsKeyEqual(name, ImagePyramidSimdLevelNames[index])) {
			*outLevel = (SimdLevel)index;
			return(true);
		}
	}
	return(false);
}

extern const ImagePyramidRowFunctions *ImagePyramidGetRowFunctions(const SimdLevel level) {
	switch (level) {
		case SimdLevel_Scalar:
			return(&ImagePyramidScalarRowFunctions);
#if defined(IMAGE_PYRAMID_ARCH_X86)
		case SimdLevel_X86_SSE2:
			return(&ImagePyramidSse2RowFunctions);
#	if !defined(IMAGE_PYRAMID_NO_AVX2)
		case SimdLevel_X86_AVX2:
			return(&ImagePyramidAvx2RowFunctions);
#	endif
#endif
		default:
			return(NULL);
	}
}

extern bool ImagePyramidIsSimdLevelAvailable(const fplCPUCapabilities *capabilities, const SimdLevel level) {
	const ImagePyramidRowFunctions *functions = ImagePyramidGetRowFunctions(level);
	if (functions == NULL) {
		return(false);
	}
	bool isX86 = capabilities != NULL && capabilities->type == fplCPUCapabilitiesType_X86;
	switch (level) {
		case SimdLevel_Scalar:
			return(true);
		case SimdLevel_X86_SSE2:
		{
#if defined(IMAGE_PYRAMID_ARCH_X86_SSE2_BASELINE)
			return(true);
#else
			bool result = isX86 && capabilities->x86.hasSSE2;
			return(result);
#endif
		}
		case SimdLevel_X86_AVX2:
		{
			bool result = isX86 && capabilities->x86.hasAVX2;
			return(result);
		}
		default:
			return(false);
	}
}

extern const ImagePyramidRowFunctions *ImagePyramidSelectRowFunctions(const fplCPUCapabilities *capabilities, const SimdLevel requestedLevel) {
	int32_t highestLevel = (int32_t)SimdLevel_Count - 1;
	int32_t startLevel = (requestedLevel >= SimdLevel_Scalar && requestedLevel < SimdLevel_Count) ? (int32_t)requestedLevel : highestLevel;
	for (int32_t level = startLevel; level > (int32_t)SimdLevel_Scalar; --level) {
		if (ImagePyramidIsSimdLevelAvailable(capabilities, (SimdLevel)level)) {
			const ImagePyramidRowFunctions *result = ImagePyramidGetRowFunctions((SimdLevel)level);
			return(result);
		}
	}
	return(&ImagePyramidScalarRowFunctions);
}

// --- Levels ----------------------------------------------------------------------------------------------------------------------

static uint32_t ImagePyramidHalveRoundedUp(const uint32_t length) {
	uint32_t result = length / 2 + length % 2;
	return(result);
}

extern uint32_t ImagePyramidComputeLevelCount(const uint32_t width, const uint32_t height) {
	uint32_t levelWidth = width;
	uint32_t levelHeight = height;
	uint32_t result = 1;
	while (result < IMAGE_PYRAMID_MAX_LEVELS) {
		uint32_t longSide = levelWidth > levelHeight ? levelWidth : levelHeight;
		if (longSide <= IMAGE_PYRAMID_SMALLEST_LONG_SIDE) {
			break;
		}
		levelWidth = ImagePyramidHalveRoundedUp(levelWidth);
		levelHeight = ImagePyramidHalveRoundedUp(levelHeight);
		++result;
	}
	return(result);
}

extern void ImagePyramidComputeLevelSize(const uint32_t width, const uint32_t height, const uint32_t level, uint32_t *outWidth, uint32_t *outHeight) {
	uint32_t levelWidth = width;
	uint32_t levelHeight = height;
	for (uint32_t index = 0; index < level; ++index) {
		levelWidth = ImagePyramidHalveRoundedUp(levelWidth);
		levelHeight = ImagePyramidHalveRoundedUp(levelHeight);
	}
	*outWidth = levelWidth;
	*outHeight = levelHeight;
}

// --- Driver ----------------------------------------------------------------------------------------------------------------------

// Taps of an output pixel near either end of an axis, where some taps fall outside the picture; the pixels in between use the interior weights
typedef struct ImagePyramidEdgeTaps {
	uint32_t output;
	int32_t firstTap;
	uint32_t tapCount;
	int16_t weights[IMAGE_PYRAMID_TAP_COUNT];
} ImagePyramidEdgeTaps;

// Two output pixels at each end of an axis read taps outside the picture, on a short axis all of them do, which are at most four as well
#define IMAGE_PYRAMID_MAX_EDGE_OUTPUTS 4

// Output pixels [firstInterior, endInterior) of an axis read only taps inside the picture: 2x - 3 >= 0 and 2x + 4 <= sourceLength - 1
static void ImagePyramidComputeInteriorRange(const uint32_t sourceLength, const uint32_t targetLength, uint32_t *outFirstInterior, uint32_t *outEndInterior) {
	const int32_t lastTapOffset = ImagePyramidFirstTapOffset + IMAGE_PYRAMID_TAP_COUNT - 1;
	const int32_t firstInteriorOutput = (-ImagePyramidFirstTapOffset + 1) / 2;
	int32_t lastInteriorCenterRoom = (int32_t)sourceLength - 1 - lastTapOffset;
	int32_t endInteriorOutput = lastInteriorCenterRoom >= 0 ? lastInteriorCenterRoom / 2 + 1 : 0;
	int32_t length = (int32_t)targetLength;
	int32_t firstInterior = firstInteriorOutput < length ? firstInteriorOutput : length;
	int32_t endInterior = endInteriorOutput < length ? endInteriorOutput : length;
	if (endInterior < firstInterior) {
		endInterior = firstInterior;
	}
	*outFirstInterior = (uint32_t)firstInterior;
	*outEndInterior = (uint32_t)endInterior;
}

// Taps of every output pixel outside [firstInterior, endInterior), returns their count
static uint32_t ImagePyramidComputeEdgeTaps(const ImagePyramidKernel kernel, const uint32_t sourceLength, const uint32_t targetLength, const uint32_t firstInterior, const uint32_t endInterior, ImagePyramidEdgeTaps *outEdges) {
	uint32_t count = 0;
	for (uint32_t output = 0; output < targetLength; ++output) {
		bool isInterior = output >= firstInterior && output < endInterior;
		if (isInterior) {
			continue;
		}
		fplAssert(count < IMAGE_PYRAMID_MAX_EDGE_OUTPUTS);
		ImagePyramidEdgeTaps *edge = &outEdges[count];
		edge->output = output;
		edge->tapCount = ImagePyramidComputeTaps(kernel, (int32_t)output, (int32_t)sourceLength, &edge->firstTap, edge->weights);
		++count;
	}
	return(count);
}

static void ImagePyramidReportProgress(const ImagePyramidSettings *settings, const double start, const double span, const double fraction) {
	if (settings->progress == NULL) {
		return;
	}
	double value = start + span * fraction;
	settings->progress(settings->progressUserData, (float)value);
}

static bool ImagePyramidIsCanceled(const ImagePyramidSettings *settings) {
	bool result = settings->cancelFlag != NULL && *settings->cancelFlag;
	return(result);
}

static ImagePyramidResult ImagePyramidReduceHalfWithProgress(const ImagePyramidLevel *source, const ImagePyramidLevel *target, const ImagePyramidSettings *settings, const double progressStart, const double progressSpan) {
	const ImagePyramidTables *tables = ImagePyramidGetTables();
	const ImagePyramidRowFunctions *functions = settings->functions != NULL ? settings->functions : &ImagePyramidScalarRowFunctions;
	const int16_t *interiorWeights = tables->interiorWeights[settings->kernel];
	const uint32_t ringRowCount = IMAGE_PYRAMID_TAP_COUNT;
	const uint32_t sourceWidth = source->width;
	const uint32_t sourceHeight = source->height;
	const uint32_t targetWidth = target->width;
	const uint32_t targetHeight = target->height;
	const uint32_t expectedTargetWidth = ImagePyramidHalveRoundedUp(sourceWidth);
	const uint32_t expectedTargetHeight = ImagePyramidHalveRoundedUp(sourceHeight);
	fplAssert(targetWidth == expectedTargetWidth && targetHeight == expectedTargetHeight);

	// Ring of decoded source rows, the vertically filtered row and the horizontally filtered row, all planar
	size_t sourceRowValues = (size_t)sourceWidth * IMAGE_PYRAMID_CHANNEL_COUNT;
	size_t targetRowValues = (size_t)targetWidth * IMAGE_PYRAMID_CHANNEL_COUNT;
	size_t scratchValues = sourceRowValues * (ringRowCount + 1) + targetRowValues;
	int16_t *scratch = (int16_t *)malloc(scratchValues * sizeof(int16_t));
	if (scratch == NULL) {
		return(ImagePyramidResult_OutOfMemory);
	}
	int16_t *ringRows[IMAGE_PYRAMID_TAP_COUNT];
	for (uint32_t ringIndex = 0; ringIndex < ringRowCount; ++ringIndex) {
		ringRows[ringIndex] = scratch + sourceRowValues * ringIndex;
	}
	int16_t *verticalRow = scratch + sourceRowValues * ringRowCount;
	int16_t *horizontalRow = verticalRow + sourceRowValues;

	// Horizontal edge taps are the same for every row
	uint32_t firstInteriorColumn = 0;
	uint32_t endInteriorColumn = 0;
	ImagePyramidComputeInteriorRange(sourceWidth, targetWidth, &firstInteriorColumn, &endInteriorColumn);
	ImagePyramidEdgeTaps edgeColumns[IMAGE_PYRAMID_MAX_EDGE_OUTPUTS];
	uint32_t edgeColumnCount = ImagePyramidComputeEdgeTaps(settings->kernel, sourceWidth, targetWidth, firstInteriorColumn, endInteriorColumn, edgeColumns);

	uint32_t firstInteriorRow = 0;
	uint32_t endInteriorRow = 0;
	ImagePyramidComputeInteriorRange(sourceHeight, targetHeight, &firstInteriorRow, &endInteriorRow);

	ImagePyramidResult result = ImagePyramidResult_Success;
	uint32_t nextDecodedRow = 0;
	for (uint32_t y = 0; y < targetHeight; ++y) {
		if ((y % ImagePyramidRowsPerBlock) == 0) {
			if (ImagePyramidIsCanceled(settings)) {
				result = ImagePyramidResult_Canceled;
				break;
			}
			double rowFraction = (double)y / (double)targetHeight;
			ImagePyramidReportProgress(settings, progressStart, progressSpan, rowFraction);
		}

		// Vertical taps of this output row
		ImagePyramidEdgeTaps rowTaps;
		bool isInteriorRow = y >= firstInteriorRow && y < endInteriorRow;
		if (isInteriorRow) {
			rowTaps.firstTap = 2 * (int32_t)y + ImagePyramidFirstTapOffset;
			rowTaps.tapCount = IMAGE_PYRAMID_TAP_COUNT;
			memcpy(rowTaps.weights, interiorWeights, sizeof(rowTaps.weights));
		} else {
			rowTaps.tapCount = ImagePyramidComputeTaps(settings->kernel, (int32_t)y, (int32_t)sourceHeight, &rowTaps.firstTap, rowTaps.weights);
		}

		// Decode the source rows this output row needs, each row once into its ring slot
		uint32_t endTap = (uint32_t)rowTaps.firstTap + rowTaps.tapCount;
		while (nextDecodedRow < endTap) {
			const uint8_t *sourceRow = source->pixels + (size_t)nextDecodedRow * source->stride;
			int16_t *ringRow = ringRows[nextDecodedRow % ringRowCount];
			functions->decodeRowToLinear(sourceRow, sourceWidth, ringRow, sourceWidth, tables);
			++nextDecodedRow;
		}

		// Vertical over all four planes at once, they lie one after another
		const int16_t *tapRows[IMAGE_PYRAMID_TAP_COUNT];
		for (uint32_t tap = 0; tap < rowTaps.tapCount; ++tap) {
			uint32_t sourceRowIndex = (uint32_t)rowTaps.firstTap + tap;
			tapRows[tap] = ringRows[sourceRowIndex % ringRowCount];
		}
		functions->filterRowsVertical(tapRows, rowTaps.weights, rowTaps.tapCount, (uint32_t)sourceRowValues, verticalRow);

		// Horizontal per plane: the SIMD function takes the interior, the edge columns are computed here
		for (uint32_t channel = 0; channel < IMAGE_PYRAMID_CHANNEL_COUNT; ++channel) {
			const int16_t *verticalPlane = verticalRow + (size_t)sourceWidth * channel;
			int16_t *horizontalPlane = horizontalRow + (size_t)targetWidth * channel;
			functions->filterRowHorizontalHalf(verticalPlane, interiorWeights, firstInteriorColumn, endInteriorColumn, horizontalPlane);
			for (uint32_t edge = 0; edge < edgeColumnCount; ++edge) {
				const ImagePyramidEdgeTaps *edgeTaps = &edgeColumns[edge];
				const int16_t *taps = verticalPlane + edgeTaps->firstTap;
				int32_t sum = 0;
				for (uint32_t tap = 0; tap < edgeTaps->tapCount; ++tap) {
					sum += (int32_t)edgeTaps->weights[tap] * (int32_t)taps[tap];
				}
				horizontalPlane[edgeTaps->output] = ImagePyramidRoundWeightedSum(sum);
			}
		}

		uint8_t *targetRow = target->pixels + (size_t)y * target->stride;
		functions->encodeRowToSRGB(horizontalRow, targetWidth, targetWidth, targetRow, tables);
	}

	free(scratch);
	return(result);
}

extern ImagePyramidResult ImagePyramidReduceHalf(const ImagePyramidLevel *source, const ImagePyramidLevel *target, const ImagePyramidSettings *settings) {
	const double wholeProgressStart = 0.0;
	const double wholeProgressSpan = 1.0;
	ImagePyramidResult result = ImagePyramidReduceHalfWithProgress(source, target, settings, wholeProgressStart, wholeProgressSpan);
	return(result);
}

extern ImagePyramidResult ImagePyramidBuild(const ImagePyramidLevel *baseLevel, const ImagePyramidSettings *settings, ImagePyramid *outPyramid) {
	memset(outPyramid, 0, sizeof(*outPyramid));
	outPyramid->levels[0] = *baseLevel;
	outPyramid->levelCount = 1;
	uint32_t levelCount = ImagePyramidComputeLevelCount(baseLevel->width, baseLevel->height);
	if (levelCount <= 1) {
		return(ImagePyramidResult_Success);
	}

	// One block for all levels, rows without padding
	size_t totalBytes = 0;
	uint64_t totalPixels = 0;
	for (uint32_t level = 1; level < levelCount; ++level) {
		ImagePyramidLevel *pyramidLevel = &outPyramid->levels[level];
		ImagePyramidComputeLevelSize(baseLevel->width, baseLevel->height, level, &pyramidLevel->width, &pyramidLevel->height);
		pyramidLevel->stride = pyramidLevel->width * ImagePyramidBytesPerPixel;
		totalBytes += (size_t)pyramidLevel->stride * pyramidLevel->height;
		totalPixels += (uint64_t)pyramidLevel->width * pyramidLevel->height;
	}
	uint8_t *memory = (uint8_t *)malloc(totalBytes);
	if (memory == NULL) {
		return(ImagePyramidResult_OutOfMemory);
	}
	size_t offset = 0;
	for (uint32_t level = 1; level < levelCount; ++level) {
		ImagePyramidLevel *pyramidLevel = &outPyramid->levels[level];
		pyramidLevel->pixels = memory + offset;
		offset += (size_t)pyramidLevel->stride * pyramidLevel->height;
	}
	outPyramid->memory = memory;

	uint64_t donePixels = 0;
	for (uint32_t level = 1; level < levelCount; ++level) {
		const ImagePyramidLevel *sourceLevel = &outPyramid->levels[level - 1];
		const ImagePyramidLevel *targetLevel = &outPyramid->levels[level];
		uint64_t levelPixels = (uint64_t)targetLevel->width * targetLevel->height;
		double progressStart = (double)donePixels / (double)totalPixels;
		double progressSpan = (double)levelPixels / (double)totalPixels;
		ImagePyramidResult result = ImagePyramidReduceHalfWithProgress(sourceLevel, targetLevel, settings, progressStart, progressSpan);
		if (result != ImagePyramidResult_Success) {
			ImagePyramidRelease(outPyramid);
			outPyramid->levels[0] = *baseLevel;
			outPyramid->levelCount = 1;
			return(result);
		}
		donePixels += levelPixels;
	}
	const double wholeProgressStart = 0.0;
	const double wholeProgressSpan = 1.0;
	const double completeFraction = 1.0;
	outPyramid->levelCount = levelCount;
	ImagePyramidReportProgress(settings, wholeProgressStart, wholeProgressSpan, completeFraction);
	return(ImagePyramidResult_Success);
}

extern void ImagePyramidRelease(ImagePyramid *pyramid) {
	free(pyramid->memory);
	memset(pyramid, 0, sizeof(*pyramid));
}

#endif // IMAGE_PYRAMID_IMPLEMENTATION
