/*
Name:
	Final Random

Description:
	Pseudo random number generator and utils.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2019 Torsten Spaete
*/

#ifndef FINAL_RANDOM_H
#define FINAL_RANDOM_H

#include "final_math.h"

// https://arvid.io/2018/07/02/better-cxx-prng/
#define RANDOMTYPE_SPLITMIX 1
#define RANDOMTYPE_XORSHIFT 2
#define RANDOMTYPE_CRT 3

#define RANDOMTYPE RANDOMTYPE_XORSHIFT

struct RandomSeries {
	uint64_t seed;
};

inline RandomSeries RandomSeed(const uint64_t seed) {
	RandomSeries result = {};
	result.seed = seed;
#if RANDOMTYPE == RANDOMTYPE_CRT
	srand(seed);
#endif
	return(result);
}

#if RANDOMTYPE == RANDOMTYPE_SPLITMIX
inline uint32_t RandomU32(RandomSeries *series) {
	uint64_t z = (series->seed += UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return uint32_t((z ^ (z >> 31)) >> 31);
}
#endif

#if RANDOMTYPE == RANDOMTYPE_XORSHIFT
inline uint32_t RandomU32(RandomSeries *series) {
	uint64_t result = (series->seed * UINT64_C(0xd989bcacc137dcd5));
	series->seed ^= series->seed >> 11;
	series->seed ^= series->seed << 31;
	series->seed ^= series->seed >> 18;
	return uint32_t(result >> 32ull);
}
#endif

#if RANDOMTYPE == RANDOMTYPE_CRT
inline uint32_t RandomU32(RandomSeries *series) {
	srand(series->seed++);
	uint32_t result = (uint32_t)(rand() / (double)RAND_MAX * (double)UINT32_MAX);
	return(result);
}
#endif

inline uint8_t RandomU8(RandomSeries *series) {
	uint8_t result = RandomU32(series) % UINT8_MAX;
	return(result);
}

// -1.0 to +1.0
inline float RandomBilateral(RandomSeries *series) {
	uint32_t s = RandomU32(series);
	float result = -1.0f + (s / (float)UINT32_MAX) * 2.0f;
	return(result);
}

// 0.0 to 1.0
inline float RandomUnilateral(RandomSeries *series) {
	uint32_t u = RandomU32(series);
	float result = u / (float)UINT32_MAX;
	return(result);
}

inline Vec3f RandomV3f(RandomSeries *series) {
	float x = RandomBilateral(series);
	float y = RandomBilateral(series);
	float z = RandomBilateral(series);
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

inline Vec3f RandomDirection(RandomSeries *series) {
	Vec3f result = V3fNormalize(RandomV3f(series));
	return(result);
}

// http://www.rorydriscoll.com/2009/01/07/better-sampling/
inline Vec3f UniformSampleHemisphere(const float u1, const float u2) {
	const float r = SquareRoot(1.0f - u1 * u1);
	const float phi = 2.0f * Pi32 * u2;
	const float x = Cosine(phi) * r;
	const float y = Sine(phi) * r;
	const float z = u1;
	Vec3f result = V3fNormalize(V3fInit(x, y, z));
	return(result);
}
inline Vec3f CosineSampleHemisphere(const float u1, const float u2) {
	const float r = SquareRoot(u1);
	const float theta = 2.0f * Pi32 * u2;
	const float x = r * Cosine(theta);
	const float y = r * Sine(theta);
	const float z = SquareRoot(Max(0.0f, 1.0f - u1));
	const Vec3f result = V3fNormalize(V3fInit(x, y, z));
	return(result);
}

// Returns a random direction inside the universal unit hemisphere
inline Vec3f RandomUnitHemisphere(RandomSeries *series) {
	float u1 = RandomUnilateral(series);
	float u2 = RandomUnilateral(series);
	Vec3f result = CosineSampleHemisphere(u1, u2);
	return(result);
}

#endif // FINAL_RANDOM_H