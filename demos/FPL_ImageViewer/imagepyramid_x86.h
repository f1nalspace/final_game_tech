/*
Name:
	FPL_ImageViewer | Image pyramid, x86 row functions

Description:
	SSE2 and AVX2 versions of the row loops of imagepyramid.h.
	Every function computes bit for bit what the scalar reference computes: the same integer products and sums, the same rounding shift,
	the same saturation (packs saturates exactly like ImagePyramidSaturate). The row tails go through the scalar helpers.

	The filters multiply-add pairs of int16 with pmaddwd. The horizontal 2:1 pass needs no even/odd split:
	a load at 2x - 3 + 2k holds the tap pair k of the outputs x, x + 1, ... in consecutive 32-bit lanes, so four loads cover all eight taps.

	Decode and encode are table lookups, which stay scalar loads: gathers were not faster on Zen 4 for the decode and are slow on Intel CPUs with the
	Gather Data Sampling mitigation (plan section 1.2). SIMD finds groups of opaque pixels, which need no premultiplication, and does their alpha,
	every other group goes through the scalar code, including the division that undoes the premultiplication. Both need SSE2 only and serve SSE2 and AVX2.

	There is no AVX-512 level: with the AVX512F instructions FPL reports, the filters (vpmulld instead of pmaddwd) were slower than AVX2 and the
	table lookups do not get faster with wider registers (measured in iteration 4).

	No global -mavx2 or -march: each function carries its own target attribute (GCC, Clang), MSVC needs none for intrinsics.
	So one binary runs on every x86 CPU and picks its level at runtime.

Usage:
	Included by imagepyramid.h inside its implementation, only under IMAGE_PYRAMID_ARCH_X86.
	IMAGE_PYRAMID_NO_AVX2 leaves that level out, for compilers without its intrinsics.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef IMAGE_PYRAMID_X86_H
#define IMAGE_PYRAMID_X86_H

#if !defined(IMAGE_PYRAMID_ARCH_X86)
#	error "imagepyramid_x86.h is for x86 and x64 only"
#endif

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#	define IMAGE_PYRAMID_TARGET_SSE2 __attribute__((target("sse2")))
#	define IMAGE_PYRAMID_TARGET_AVX2 __attribute__((target("avx2")))
#else
#	define IMAGE_PYRAMID_TARGET_SSE2
#	define IMAGE_PYRAMID_TARGET_AVX2
#endif

// Tap pairs of the 2:1 kernel
#define IMAGE_PYRAMID_TAP_PAIR_COUNT (IMAGE_PYRAMID_TAP_COUNT / 2)

// Two int16 weights as one 32-bit lane for pmaddwd: the first weight multiplies the lower int16 of each lane
static int32_t ImagePyramidPackWeightPair(const int16_t first, const int16_t second) {
	const uint32_t halfBits = 16;
	uint32_t low = (uint32_t)(uint16_t)first;
	uint32_t high = (uint32_t)(uint16_t)second << halfBits;
	int32_t result = (int32_t)(low | high);
	return(result);
}

// Bits of the sRGB8 channels inside a little endian RGBA pixel, x86 is always little endian
#define IMAGE_PYRAMID_GREEN_SHIFT 8
#define IMAGE_PYRAMID_BLUE_SHIFT 16
#define IMAGE_PYRAMID_ALPHA_SHIFT 24
#define IMAGE_PYRAMID_BYTE_MASK 0xFF
// Pixels per opaque check: one 128-bit load of RGBA8 pixels, one 128-bit load of int16 alpha
#define IMAGE_PYRAMID_DECODE_GROUP 4
#define IMAGE_PYRAMID_ENCODE_GROUP 8

// Decode with SIMD opaque check: a group of four opaque pixels only needs the table lookups, the alpha plane is filled with SIMD
IMAGE_PYRAMID_TARGET_SSE2 static void ImagePyramidDecodeRowSse2(const uint8_t *rgba, const uint32_t width, int16_t *planes, const size_t planeStride, const ImagePyramidTables *tables) {
	const int allBytesSet = 0xFFFF;
	int16_t *red = planes;
	int16_t *green = planes + planeStride;
	int16_t *blue = planes + planeStride * 2;
	int16_t *alpha = planes + planeStride * 3;
	const int16_t *linearTable = tables->srgbToLinear;
	const __m128i opaqueAlphaByte = _mm_set1_epi32(IMAGE_PYRAMID_BYTE_MASK);
	const __m128i opaqueLinearAlpha = _mm_set1_epi16((int16_t)ImagePyramidLinearMaximum);

	uint32_t x = 0;
	for (; x + IMAGE_PYRAMID_DECODE_GROUP <= width; x += IMAGE_PYRAMID_DECODE_GROUP) {
		const uint8_t *groupPixels = rgba + (size_t)x * ImagePyramidBytesPerPixel;
		__m128i pixels = _mm_loadu_si128((const __m128i *)groupPixels);
		__m128i alphaBytes = _mm_srli_epi32(pixels, IMAGE_PYRAMID_ALPHA_SHIFT);
		__m128i opaqueLanes = _mm_cmpeq_epi32(alphaBytes, opaqueAlphaByte);
		int opaqueMask = _mm_movemask_epi8(opaqueLanes);
		if (opaqueMask != allBytesSet) {
			ImagePyramidDecodeRangeScalar(rgba, x, x + IMAGE_PYRAMID_DECODE_GROUP, planes, planeStride, tables);
			continue;
		}
		for (uint32_t index = 0; index < IMAGE_PYRAMID_DECODE_GROUP; ++index) {
			uint32_t pixel;
			memcpy(&pixel, groupPixels + index * ImagePyramidBytesPerPixel, sizeof(pixel));
			uint32_t redByte = pixel & IMAGE_PYRAMID_BYTE_MASK;
			uint32_t greenByte = (pixel >> IMAGE_PYRAMID_GREEN_SHIFT) & IMAGE_PYRAMID_BYTE_MASK;
			uint32_t blueByte = (pixel >> IMAGE_PYRAMID_BLUE_SHIFT) & IMAGE_PYRAMID_BYTE_MASK;
			red[x + index] = linearTable[redByte];
			green[x + index] = linearTable[greenByte];
			blue[x + index] = linearTable[blueByte];
		}
		_mm_storel_epi64((__m128i *)(alpha + x), opaqueLinearAlpha);
	}
	ImagePyramidDecodeRangeScalar(rgba, x, width, planes, planeStride, tables);
}

// Encode with SIMD opaque check: a group of eight opaque pixels only needs 0 <= color, nothing to undo, and alpha8 is 255
IMAGE_PYRAMID_TARGET_SSE2 static void ImagePyramidEncodeRowSse2(const int16_t *planes, const size_t planeStride, const uint32_t width, uint8_t *rgba, const ImagePyramidTables *tables) {
	const int allBytesSet = 0xFFFF;
	const uint32_t opaqueAlphaByte = (uint32_t)IMAGE_PYRAMID_BYTE_MASK << IMAGE_PYRAMID_ALPHA_SHIFT;
	const int16_t *red = planes;
	const int16_t *green = planes + planeStride;
	const int16_t *blue = planes + planeStride * 2;
	const int16_t *alpha = planes + planeStride * 3;
	const uint8_t *srgbTable = tables->linearToSRGB;
	const __m128i opaqueAlpha = _mm_set1_epi16((int16_t)ImagePyramidLinearMaximum);
	const __m128i zero = _mm_setzero_si128();

	uint32_t x = 0;
	for (; x + IMAGE_PYRAMID_ENCODE_GROUP <= width; x += IMAGE_PYRAMID_ENCODE_GROUP) {
		__m128i alphaValues = _mm_loadu_si128((const __m128i *)(alpha + x));
		__m128i opaqueLanes = _mm_cmpeq_epi16(alphaValues, opaqueAlpha);
		int opaqueMask = _mm_movemask_epi8(opaqueLanes);
		if (opaqueMask != allBytesSet) {
			ImagePyramidEncodeRangeScalar(planes, planeStride, x, x + IMAGE_PYRAMID_ENCODE_GROUP, rgba, tables);
			continue;
		}
		__m128i redValues = _mm_loadu_si128((const __m128i *)(red + x));
		__m128i greenValues = _mm_loadu_si128((const __m128i *)(green + x));
		__m128i blueValues = _mm_loadu_si128((const __m128i *)(blue + x));
		__m128i redClamped = _mm_max_epi16(redValues, zero);
		__m128i greenClamped = _mm_max_epi16(greenValues, zero);
		__m128i blueClamped = _mm_max_epi16(blueValues, zero);
		uint16_t redIndices[IMAGE_PYRAMID_ENCODE_GROUP];
		uint16_t greenIndices[IMAGE_PYRAMID_ENCODE_GROUP];
		uint16_t blueIndices[IMAGE_PYRAMID_ENCODE_GROUP];
		_mm_storeu_si128((__m128i *)redIndices, redClamped);
		_mm_storeu_si128((__m128i *)greenIndices, greenClamped);
		_mm_storeu_si128((__m128i *)blueIndices, blueClamped);
		uint8_t *groupPixels = rgba + (size_t)x * ImagePyramidBytesPerPixel;
		for (uint32_t index = 0; index < IMAGE_PYRAMID_ENCODE_GROUP; ++index) {
			uint32_t redByte = srgbTable[redIndices[index]];
			uint32_t greenByte = (uint32_t)srgbTable[greenIndices[index]] << IMAGE_PYRAMID_GREEN_SHIFT;
			uint32_t blueByte = (uint32_t)srgbTable[blueIndices[index]] << IMAGE_PYRAMID_BLUE_SHIFT;
			uint32_t pixel = redByte | greenByte | blueByte | opaqueAlphaByte;
			memcpy(groupPixels + index * ImagePyramidBytesPerPixel, &pixel, sizeof(pixel));
		}
	}
	ImagePyramidEncodeRangeScalar(planes, planeStride, x, width, rgba, tables);
}

// --- SSE2 ------------------------------------------------------------------------------------------------------------------------

IMAGE_PYRAMID_TARGET_SSE2 static void ImagePyramidFilterVerticalSse2(const int16_t *const *rows, const int16_t *weights, const uint32_t tapCount, const uint32_t length, int16_t *out) {
	const uint32_t lanes = 8;
	uint32_t pairCount = tapCount / 2;
	bool hasSingleTap = (tapCount % 2) != 0;
	int16_t lastWeight = hasSingleTap ? weights[tapCount - 1] : 0;
	int32_t singlePair = ImagePyramidPackWeightPair(lastWeight, 0);
	__m128i pairWeights[IMAGE_PYRAMID_TAP_PAIR_COUNT];
	for (uint32_t pair = 0; pair < pairCount; ++pair) {
		int32_t packed = ImagePyramidPackWeightPair(weights[pair * 2], weights[pair * 2 + 1]);
		pairWeights[pair] = _mm_set1_epi32(packed);
	}
	const __m128i singleWeights = _mm_set1_epi32(singlePair);
	const __m128i rounding = _mm_set1_epi32(ImagePyramidWeightHalf);
	const __m128i zero = _mm_setzero_si128();

	uint32_t index = 0;
	for (; index + lanes <= length; index += lanes) {
		__m128i sumLow = zero;
		__m128i sumHigh = zero;
		for (uint32_t pair = 0; pair < pairCount; ++pair) {
			__m128i first = _mm_loadu_si128((const __m128i *)(rows[pair * 2] + index));
			__m128i second = _mm_loadu_si128((const __m128i *)(rows[pair * 2 + 1] + index));
			__m128i interleavedLow = _mm_unpacklo_epi16(first, second);
			__m128i interleavedHigh = _mm_unpackhi_epi16(first, second);
			__m128i productLow = _mm_madd_epi16(interleavedLow, pairWeights[pair]);
			__m128i productHigh = _mm_madd_epi16(interleavedHigh, pairWeights[pair]);
			sumLow = _mm_add_epi32(sumLow, productLow);
			sumHigh = _mm_add_epi32(sumHigh, productHigh);
		}
		if (hasSingleTap) {
			__m128i last = _mm_loadu_si128((const __m128i *)(rows[tapCount - 1] + index));
			__m128i interleavedLow = _mm_unpacklo_epi16(last, zero);
			__m128i interleavedHigh = _mm_unpackhi_epi16(last, zero);
			__m128i productLow = _mm_madd_epi16(interleavedLow, singleWeights);
			__m128i productHigh = _mm_madd_epi16(interleavedHigh, singleWeights);
			sumLow = _mm_add_epi32(sumLow, productLow);
			sumHigh = _mm_add_epi32(sumHigh, productHigh);
		}
		__m128i roundedLow = _mm_add_epi32(sumLow, rounding);
		__m128i roundedHigh = _mm_add_epi32(sumHigh, rounding);
		__m128i shiftedLow = _mm_srai_epi32(roundedLow, ImagePyramidWeightBits);
		__m128i shiftedHigh = _mm_srai_epi32(roundedHigh, ImagePyramidWeightBits);
		__m128i packed = _mm_packs_epi32(shiftedLow, shiftedHigh);
		_mm_storeu_si128((__m128i *)(out + index), packed);
	}
	ImagePyramidFilterVerticalRange(rows, weights, tapCount, index, length, out);
}

IMAGE_PYRAMID_TARGET_SSE2 static void ImagePyramidFilterHorizontalHalfSse2(const int16_t *row, const int16_t *weights, const uint32_t firstOutput, const uint32_t endOutput, int16_t *out) {
	// Two groups of four outputs, each group from four loads of eight int16
	const uint32_t outputsPerGroup = 4;
	const uint32_t outputsPerStep = outputsPerGroup * 2;
	__m128i pairWeights[IMAGE_PYRAMID_TAP_PAIR_COUNT];
	for (uint32_t pair = 0; pair < IMAGE_PYRAMID_TAP_PAIR_COUNT; ++pair) {
		int32_t packed = ImagePyramidPackWeightPair(weights[pair * 2], weights[pair * 2 + 1]);
		pairWeights[pair] = _mm_set1_epi32(packed);
	}
	const __m128i rounding = _mm_set1_epi32(ImagePyramidWeightHalf);

	uint32_t x = firstOutput;
	for (; x + outputsPerStep <= endOutput; x += outputsPerStep) {
		const int16_t *firstGroupTaps = row + 2 * (int32_t)x + ImagePyramidFirstTapOffset;
		const int16_t *secondGroupTaps = firstGroupTaps + 2 * outputsPerGroup;
		__m128i firstSum = _mm_setzero_si128();
		__m128i secondSum = _mm_setzero_si128();
		for (uint32_t pair = 0; pair < IMAGE_PYRAMID_TAP_PAIR_COUNT; ++pair) {
			__m128i firstTaps = _mm_loadu_si128((const __m128i *)(firstGroupTaps + pair * 2));
			__m128i secondTaps = _mm_loadu_si128((const __m128i *)(secondGroupTaps + pair * 2));
			__m128i firstProduct = _mm_madd_epi16(firstTaps, pairWeights[pair]);
			__m128i secondProduct = _mm_madd_epi16(secondTaps, pairWeights[pair]);
			firstSum = _mm_add_epi32(firstSum, firstProduct);
			secondSum = _mm_add_epi32(secondSum, secondProduct);
		}
		__m128i firstRounded = _mm_add_epi32(firstSum, rounding);
		__m128i secondRounded = _mm_add_epi32(secondSum, rounding);
		__m128i firstShifted = _mm_srai_epi32(firstRounded, ImagePyramidWeightBits);
		__m128i secondShifted = _mm_srai_epi32(secondRounded, ImagePyramidWeightBits);
		__m128i packed = _mm_packs_epi32(firstShifted, secondShifted);
		_mm_storeu_si128((__m128i *)(out + x), packed);
	}
	ImagePyramidFilterHorizontalHalfRange(row, weights, x, endOutput, out);
}

static const ImagePyramidRowFunctions ImagePyramidSse2RowFunctions = {
	"sse2",
	SimdLevel_X86_SSE2,
	ImagePyramidDecodeRowSse2,
	ImagePyramidFilterVerticalSse2,
	ImagePyramidFilterHorizontalHalfSse2,
	ImagePyramidEncodeRowSse2,
};

// --- AVX2 ------------------------------------------------------------------------------------------------------------------------

#if !defined(IMAGE_PYRAMID_NO_AVX2)

IMAGE_PYRAMID_TARGET_AVX2 static void ImagePyramidFilterVerticalAvx2(const int16_t *const *rows, const int16_t *weights, const uint32_t tapCount, const uint32_t length, int16_t *out) {
	const uint32_t lanes = 16;
	uint32_t pairCount = tapCount / 2;
	bool hasSingleTap = (tapCount % 2) != 0;
	int16_t lastWeight = hasSingleTap ? weights[tapCount - 1] : 0;
	int32_t singlePair = ImagePyramidPackWeightPair(lastWeight, 0);
	__m256i pairWeights[IMAGE_PYRAMID_TAP_PAIR_COUNT];
	for (uint32_t pair = 0; pair < pairCount; ++pair) {
		int32_t packed = ImagePyramidPackWeightPair(weights[pair * 2], weights[pair * 2 + 1]);
		pairWeights[pair] = _mm256_set1_epi32(packed);
	}
	const __m256i singleWeights = _mm256_set1_epi32(singlePair);
	const __m256i rounding = _mm256_set1_epi32(ImagePyramidWeightHalf);
	const __m256i zero = _mm256_setzero_si256();

	// unpack, madd and packs all work within 128-bit halves, so the packed result comes out in memory order again
	uint32_t index = 0;
	for (; index + lanes <= length; index += lanes) {
		__m256i sumLow = zero;
		__m256i sumHigh = zero;
		for (uint32_t pair = 0; pair < pairCount; ++pair) {
			__m256i first = _mm256_loadu_si256((const __m256i *)(rows[pair * 2] + index));
			__m256i second = _mm256_loadu_si256((const __m256i *)(rows[pair * 2 + 1] + index));
			__m256i interleavedLow = _mm256_unpacklo_epi16(first, second);
			__m256i interleavedHigh = _mm256_unpackhi_epi16(first, second);
			__m256i productLow = _mm256_madd_epi16(interleavedLow, pairWeights[pair]);
			__m256i productHigh = _mm256_madd_epi16(interleavedHigh, pairWeights[pair]);
			sumLow = _mm256_add_epi32(sumLow, productLow);
			sumHigh = _mm256_add_epi32(sumHigh, productHigh);
		}
		if (hasSingleTap) {
			__m256i last = _mm256_loadu_si256((const __m256i *)(rows[tapCount - 1] + index));
			__m256i interleavedLow = _mm256_unpacklo_epi16(last, zero);
			__m256i interleavedHigh = _mm256_unpackhi_epi16(last, zero);
			__m256i productLow = _mm256_madd_epi16(interleavedLow, singleWeights);
			__m256i productHigh = _mm256_madd_epi16(interleavedHigh, singleWeights);
			sumLow = _mm256_add_epi32(sumLow, productLow);
			sumHigh = _mm256_add_epi32(sumHigh, productHigh);
		}
		__m256i roundedLow = _mm256_add_epi32(sumLow, rounding);
		__m256i roundedHigh = _mm256_add_epi32(sumHigh, rounding);
		__m256i shiftedLow = _mm256_srai_epi32(roundedLow, ImagePyramidWeightBits);
		__m256i shiftedHigh = _mm256_srai_epi32(roundedHigh, ImagePyramidWeightBits);
		__m256i packed = _mm256_packs_epi32(shiftedLow, shiftedHigh);
		_mm256_storeu_si256((__m256i *)(out + index), packed);
	}
	ImagePyramidFilterVerticalRange(rows, weights, tapCount, index, length, out);
}

IMAGE_PYRAMID_TARGET_AVX2 static void ImagePyramidFilterHorizontalHalfAvx2(const int16_t *row, const int16_t *weights, const uint32_t firstOutput, const uint32_t endOutput, int16_t *out) {
	// Two groups of eight outputs, each group from four loads of sixteen int16
	const uint32_t outputsPerGroup = 8;
	const uint32_t outputsPerStep = outputsPerGroup * 2;
	// packs interleaves the 128-bit halves of both groups, this puts the four 64-bit parts back into memory order
	const int memoryOrderPermutation = 0xD8;
	__m256i pairWeights[IMAGE_PYRAMID_TAP_PAIR_COUNT];
	for (uint32_t pair = 0; pair < IMAGE_PYRAMID_TAP_PAIR_COUNT; ++pair) {
		int32_t packed = ImagePyramidPackWeightPair(weights[pair * 2], weights[pair * 2 + 1]);
		pairWeights[pair] = _mm256_set1_epi32(packed);
	}
	const __m256i rounding = _mm256_set1_epi32(ImagePyramidWeightHalf);

	uint32_t x = firstOutput;
	for (; x + outputsPerStep <= endOutput; x += outputsPerStep) {
		const int16_t *firstGroupTaps = row + 2 * (int32_t)x + ImagePyramidFirstTapOffset;
		const int16_t *secondGroupTaps = firstGroupTaps + 2 * outputsPerGroup;
		__m256i firstSum = _mm256_setzero_si256();
		__m256i secondSum = _mm256_setzero_si256();
		for (uint32_t pair = 0; pair < IMAGE_PYRAMID_TAP_PAIR_COUNT; ++pair) {
			__m256i firstTaps = _mm256_loadu_si256((const __m256i *)(firstGroupTaps + pair * 2));
			__m256i secondTaps = _mm256_loadu_si256((const __m256i *)(secondGroupTaps + pair * 2));
			__m256i firstProduct = _mm256_madd_epi16(firstTaps, pairWeights[pair]);
			__m256i secondProduct = _mm256_madd_epi16(secondTaps, pairWeights[pair]);
			firstSum = _mm256_add_epi32(firstSum, firstProduct);
			secondSum = _mm256_add_epi32(secondSum, secondProduct);
		}
		__m256i firstRounded = _mm256_add_epi32(firstSum, rounding);
		__m256i secondRounded = _mm256_add_epi32(secondSum, rounding);
		__m256i firstShifted = _mm256_srai_epi32(firstRounded, ImagePyramidWeightBits);
		__m256i secondShifted = _mm256_srai_epi32(secondRounded, ImagePyramidWeightBits);
		__m256i packed = _mm256_packs_epi32(firstShifted, secondShifted);
		__m256i ordered = _mm256_permute4x64_epi64(packed, memoryOrderPermutation);
		_mm256_storeu_si256((__m256i *)(out + x), ordered);
	}
	ImagePyramidFilterHorizontalHalfRange(row, weights, x, endOutput, out);
}

static const ImagePyramidRowFunctions ImagePyramidAvx2RowFunctions = {
	"avx2",
	SimdLevel_X86_AVX2,
	ImagePyramidDecodeRowSse2,
	ImagePyramidFilterVerticalAvx2,
	ImagePyramidFilterHorizontalHalfAvx2,
	ImagePyramidEncodeRowSse2,
};

#endif // IMAGE_PYRAMID_NO_AVX2


#endif // IMAGE_PYRAMID_X86_H
