/*
Name:
	Final Random

Description:
	Pseudo random number generator and utils.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2019-2026 Torsten Spaete

Changelog:
	## 2026-10-20
	- Ported to C99, compiles as C99, C17 and C++11
	- Fixed RandomU8() never returned 255
*/

#ifndef FINAL_RANDOM_H
#define FINAL_RANDOM_H

// Also brings in final_platform_layer.h (fpl_extern_inline, fplZeroInit) and stdlib.h (srand, rand, RAND_MAX)
#include "final_math.h"

// https://arvid.io/2018/07/02/better-cxx-prng/

//! Generator type: SplitMix64, bits 31 to 62 of each 64-bit output.
#define RANDOMTYPE_SPLITMIX 1
//! Generator type: Xorshift64 with a multiplied output (default).
#define RANDOMTYPE_XORSHIFT 2
//! Generator type: srand()/rand() of the C runtime, not reproducible across compilers and platforms.
#define RANDOMTYPE_CRT 3

//! The selected generator type, only the two integer generators give the same sequence everywhere.
#define RANDOMTYPE RANDOMTYPE_XORSHIFT

/**
* @struct RandomSeries
* @brief The state of one pseudo random sequence.
*/
typedef struct RandomSeries {
	//! The current generator state.
	uint64_t seed;
} RandomSeries;

/**
* @brief Creates a random series from a seed.
* @param[in] seed The start state, the same seed always gives the same sequence.
* @return The initialized RandomSeries.
* @note With RANDOMTYPE_XORSHIFT a seed of zero stays zero forever and the series returns only zeros, map zero to another seed first.
*/
fpl_extern_inline RandomSeries RandomSeed(const uint64_t seed) {
	RandomSeries result = fplZeroInit;
	result.seed = seed;
#if RANDOMTYPE == RANDOMTYPE_CRT
	srand((unsigned int)seed);
#endif
	return(result);
}

#if RANDOMTYPE == RANDOMTYPE_SPLITMIX
/**
* @brief Returns the next random 32-bit value and advances the series.
* @param[in,out] series The random series.
* @return A value in the full range 0 to UINT32_MAX.
*/
fpl_extern_inline uint32_t RandomU32(RandomSeries *series) {
	uint64_t z = (series->seed += UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	uint64_t mixed = z ^ (z >> 31);
	// Bits 31 to 62 of the output, like the reference implementation
	const uint32_t outputShift = 31;
	uint32_t result = (uint32_t)(mixed >> outputShift);
	return(result);
}
#endif

#if RANDOMTYPE == RANDOMTYPE_XORSHIFT
/**
* @brief Returns the next random 32-bit value and advances the series.
* @param[in,out] series The random series.
* @return A value in the full range 0 to UINT32_MAX.
*/
fpl_extern_inline uint32_t RandomU32(RandomSeries *series) {
	uint64_t result = (series->seed * UINT64_C(0xd989bcacc137dcd5));
	series->seed ^= series->seed >> 11;
	series->seed ^= series->seed << 31;
	series->seed ^= series->seed >> 18;
	const uint32_t upperHalfShift = 32;
	uint32_t upperHalf = (uint32_t)(result >> upperHalfShift);
	return(upperHalf);
}
#endif

#if RANDOMTYPE == RANDOMTYPE_CRT
/**
* @brief Returns the next random 32-bit value and advances the series.
* @param[in,out] series The random series.
* @return A value in the range 0 to UINT32_MAX, from rand() of the C runtime.
*/
fpl_extern_inline uint32_t RandomU32(RandomSeries *series) {
	srand((unsigned int)series->seed++);
	uint32_t result = (uint32_t)(rand() / (double)RAND_MAX * (double)UINT32_MAX);
	return(result);
}
#endif

/**
* @brief Returns the next random 8-bit value and advances the series.
* @param[in,out] series The random series.
* @return A value in the full range 0 to 255, the upper 8 bits of RandomU32().
*/
fpl_extern_inline uint8_t RandomU8(RandomSeries *series) {
	uint32_t value = RandomU32(series);
	const uint32_t upperByteShift = 24;
	uint8_t result = (uint8_t)(value >> upperByteShift);
	return(result);
}

/**
* @brief Returns the next random float between -1 and +1 and advances the series.
* @param[in,out] series The random series.
* @return A value in the range -1.0 to +1.0, both inclusive.
*/
fpl_extern_inline float RandomBilateral(RandomSeries *series) {
	uint32_t s = RandomU32(series);
	float sample = (float)s;
	float result = -1.0f + (sample / (float)UINT32_MAX) * 2.0f;
	return(result);
}

/**
* @brief Returns the next random float between 0 and 1 and advances the series.
* @param[in,out] series The random series.
* @return A value in the range 0.0 to 1.0, both inclusive.
*/
fpl_extern_inline float RandomUnilateral(RandomSeries *series) {
	uint32_t u = RandomU32(series);
	float sample = (float)u;
	float result = sample / (float)UINT32_MAX;
	return(result);
}

/**
* @brief Returns a vector with three random components between -1 and +1 and advances the series.
* @param[in,out] series The random series.
* @return A Vec3f inside the cube from -1 to +1, the components are drawn in the order x, y, z.
*/
fpl_extern_inline Vec3f RandomV3f(RandomSeries *series) {
	float x = RandomBilateral(series);
	float y = RandomBilateral(series);
	float z = RandomBilateral(series);
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

/**
* @brief Returns a random unit direction and advances the series.
* @param[in,out] series The random series.
* @return The normalized RandomV3f(), not exactly uniform on the sphere because the cube corners are denser.
*/
fpl_extern_inline Vec3f RandomDirection(RandomSeries *series) {
	Vec3f result = V3fNormalize(RandomV3f(series));
	return(result);
}

// http://www.rorydriscoll.com/2009/01/07/better-sampling/

/**
* @brief Maps two uniform values onto the unit hemisphere around +Z with a uniform distribution.
* @param[in] u1 The first uniform value in 0 to 1, becomes the z component.
* @param[in] u2 The second uniform value in 0 to 1, the angle around +Z.
* @return A unit vector on the hemisphere with z >= 0.
*/
fpl_extern_inline Vec3f UniformSampleHemisphere(const float u1, const float u2) {
	const float r = F32SquareRoot(1.0f - u1 * u1);
	const float phi = 2.0f * F32Pi * u2;
	const float x = F32Cos(phi) * r;
	const float y = F32Sin(phi) * r;
	const float z = u1;
	Vec3f result = V3fNormalize(V3fInit(x, y, z));
	return(result);
}

/**
* @brief Maps two uniform values onto the unit hemisphere around +Z with a cosine weighted distribution.
* @param[in] u1 The first uniform value in 0 to 1, the squared distance from the +Z axis.
* @param[in] u2 The second uniform value in 0 to 1, the angle around +Z.
* @return A unit vector on the hemisphere with z >= 0, directions near +Z are more likely.
*/
fpl_extern_inline Vec3f CosineSampleHemisphere(const float u1, const float u2) {
	const float r = F32SquareRoot(u1);
	const float theta = 2.0f * F32Pi * u2;
	const float x = r * F32Cos(theta);
	const float y = r * F32Sin(theta);
	const float z = F32SquareRoot(F32Max(0.0f, 1.0f - u1));
	const Vec3f result = V3fNormalize(V3fInit(x, y, z));
	return(result);
}

/**
* @brief Returns a random cosine weighted direction on the unit hemisphere around +Z and advances the series.
* @param[in,out] series The random series.
* @return A unit vector with z >= 0, see CosineSampleHemisphere().
*/
fpl_extern_inline Vec3f RandomUnitHemisphere(RandomSeries *series) {
	float u1 = RandomUnilateral(series);
	float u2 = RandomUnilateral(series);
	Vec3f result = CosineSampleHemisphere(u1, u2);
	return(result);
}

#endif // FINAL_RANDOM_H
