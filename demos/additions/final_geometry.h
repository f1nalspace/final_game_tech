/*
Name:
	Final Geometry

Description:
	Contains types and functions for working with geometric types, that are mostly used for physics simulations.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_GEOMETRY_H
#define FINAL_GEOMETRY_H

#include <final_math.h>

//
// Ray3f
//
typedef struct Ray3f {
	Vec3f origin;
	Vec3f direction;
} Ray3f;

fpl_inline Ray3f Ray3fInit(const Vec3f origin, const Vec3f direction) {
	Ray3f result = { origin, direction };
	return(result);
}

typedef struct HitResult3f {
	Vec3f contact;
	Vec3f normal;
	float t;
	bool isHit;
} HitResult3f;

//
// Plane3f
//
typedef union Plane3f {
	struct {
		Vec3f normal;
		float distance;
	};
	Vec4f m;
} Plane3f;

//
// Sphere3f
//
typedef struct Sphere3f {
	Vec3f origin;
	float radius;
} Sphere3f;

fpl_inline Sphere3f Sphere3fInit(const Vec3f origin, const float radius) {
	Sphere3f result = fplStructInit(Sphere3f,origin,radius);
	return result;
}

//
// LineCast2f
//
typedef struct LineCastInput2f {
	Vec2f p1;
	Vec2f p2;
	float maxFraction;
} LineCastInput2f;

typedef struct LineCastOutput2f {
	Vec2f normal;
	float fraction;
} LineCastOutput2f;

fpl_extern bool LineCast2fAgainstCircle(const LineCastInput2f *input, const Vec2f *center, const float radius, LineCastOutput2f *output);

//
// AABB2f
//
typedef struct AABB2f {
	Vec2f min;
	Vec2f max;
} AABB2f;

fpl_inline AABB2f AABB2fInit(const Vec2f min, const Vec2f max) {
	AABB2f result = fplStructInit(AABB2f, min, max);
	return result;
}

fpl_inline AABB2f AABB2fInitFromCenter(const Vec2f center, const Vec2f radius) {
	Vec2f min = V2fSub(center, radius);
	Vec2f max = V2fAdd(center, radius);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

fpl_inline AABB2f AABB2fInitFromBottomLeft(const Vec2f bottomLeft, const Vec2f size) {
	Vec2f min = bottomLeft;
	Vec2f max = V2fAdd(bottomLeft, size);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

fpl_inline Vec2f AABB2fGetRadius(const AABB2f *aabb) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f result = V2fMultScalar(size, 0.5f);
	return result;
}

fpl_inline Vec2f AABB2fGetCenter(const AABB2f *aabb) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f ext = V2fMultScalar(size, 0.5f);
	Vec2f result = V2fAdd(aabb->min, ext);
	return result;
}

fpl_inline Vec2f AABB2fGetSize(const AABB2f *aabb) {
	Vec2f result = V2fSub(aabb->max, aabb->min);
	return result;
}

fpl_inline void AABB2fExtract(const AABB2f *aabb, Vec2f *outCenter, Vec2f *outRadius) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f radius = V2fMultScalar(size, 0.5f);
	Vec2f center = V2fAdd(aabb->min, radius);
	*outCenter = center;
	*outRadius = radius;
}

fpl_inline bool AABB2fIsOverlap(const AABB2f *a, const AABB2f *b) {
	Vec2f centerA, centerB;
	Vec2f radiusA, radiusB;
	AABB2fExtract(a, &centerA, &radiusA);
	AABB2fExtract(b, &centerB, &radiusB);
	Vec2f centerDiff = V2fAbs(V2fSub(centerB, centerA));
	Vec2f minkowskiSum = V2fAdd(radiusA, radiusB);
	Vec2f delta = V2fSub(centerDiff, minkowskiSum);
	bool result = (delta.x < 0.0f) && (delta.y < 0.0f);
	return result;
}

fpl_inline bool AABB2fContainsPoint(const AABB2f *aabb, const Vec2f point) {
	Vec2f center, radius;
	AABB2fExtract(aabb, &center, &radius);
	Vec2f d = V2fSub(point, center);
	bool result = (d.x < radius.x) && (d.y < radius.y);
	return result;
}

fpl_inline void AABB2fExpand(AABB2f *aabb, const Vec2f expansion) {
    aabb->min = V2fSub(aabb->min, expansion);
    aabb->max = V2fAdd(aabb->max, expansion);
}

fpl_inline void AABB2fExpandScalar(AABB2f *aabb, const float scalar) {
    Vec2f e = V2fInitScalar(scalar);
    AABB2fExpand(aabb, e);
}

//
// AABB3f
//
typedef struct AABB3f {
	Vec3f min;
	Vec3f max;
} AABB3f;

fpl_inline AABB3f AABB3fInit(const Vec3f min, const Vec3f max) {
	AABB3f result = fplStructInit(AABB3f, min, max);
	return result;
}

fpl_inline AABB3f AABB3fInitFromCenter(const Vec3f center, const Vec3f radius) {
	Vec3f min = V3fSub(center, radius);
	Vec3f max = V3fAdd(center, radius);
	AABB3f result = AABB3fInit(min, max);
	return result;
}

fpl_inline Vec3f AABB3fGetRadius(const AABB3f *aabb) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f result = V3fMultScalar(size, 0.5f);
	return result;
}

fpl_inline Vec3f AABB3fGetCenter(const AABB3f *aabb) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f ext = V3fMultScalar(size, 0.5f);
	Vec3f result = V3fAdd(aabb->min, ext);
	return result;
}

fpl_inline void AABB3fExtract(const AABB3f *aabb, Vec3f *outCenter, Vec3f *outRadius) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f radius = V3fMultScalar(size, 0.5f);
	Vec3f center = V3fAdd(aabb->min, radius);
	*outCenter = center;
	*outRadius = radius;
}

fpl_inline bool AABB3fIsOverlap(const AABB3f *a, const AABB3f *b) {
	Vec3f centerA, centerB;
	Vec3f radiusA, radiusB;
	AABB3fExtract(a, &centerA, &radiusA);
	AABB3fExtract(b, &centerB, &radiusB);
	Vec3f centerDiff = V3fAbs(V3fSub(centerB, centerA));
	Vec3f minkowskiSum = V3fAdd(radiusA, radiusB);
	Vec3f delta = V3fSub(centerDiff, minkowskiSum);
	bool result = (delta.x < 0.0f) && (delta.y < 0.0f) && (delta.z < 0.0f);
	return result;
}

fpl_inline bool AABB3fContainsPoint(const AABB3f *aabb, const Vec3f point) {
	Vec3f center, radius;
	AABB3fExtract(aabb, &center, &radius);
	Vec3f d = V3fSub(point, center);
	bool result = (d.x < radius.x) && (d.y < radius.y) && (d.z < radius.z);
	return result;
}

#endif // FINAL_GEOMETRY_H

#if defined(FINAL_GEOMETRY_IMPLEMENTATION) && !defined(FINAL_GEOMETRY_IMPLEMENTED)
#define FINAL_GEOMETRY_IMPLEMENTED

fpl_extern bool LineCast2fAgainstCircle(const LineCastInput2f *input, const Vec2f *center, const float radius, LineCastOutput2f *output) {
	if (input == fpl_null || center == fpl_null || output == fpl_null) {
		return false;
	}

	Vec2f s = V2fSub(input->p1, *center);
	float b = V2fDot(s, s) - radius * radius;

	// Solve quadratic equation.
	Vec2f r = V2fSub(input->p2, input->p1);
	float c = V2fDot(s, r);
	float rr = V2fDot(r, r);
	float sigma = c * c - rr * b;

	// Check for negative discriminant and short segment.
	if (sigma < 0.0f || rr < F32Epsilon) {
		return false;
	}

	// Find the point of intersection of the line with the circle.
	float a = -(c + F32SquareRoot(sigma));

	// Is the intersection point on the segment?
	if (0.0f <= a && a <= input->maxFraction * rr) {
		a /= rr;
		fplClearStruct(output);
		output->fraction = a;
		output->normal = V2fNormalize(V2fAddMultScalar(s, r, a));
		return true;
	}

	return false;
}

#endif // FINAL_GEOMETRY_IMPLEMENTATION