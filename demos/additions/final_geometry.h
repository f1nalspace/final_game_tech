/*
Name:
	Final Geometry

Description:
	Contains types and functions for working with geometric types, that are mostly used for physics simulations.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
	## 2026-07-19
	- Fixed AABB3fContainsPoint missing abs(): it reported "inside" for points outside on the negative side
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

fpl_extern_inline Ray3f Ray3fInit(const Vec3f origin, const Vec3f direction) {
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

fpl_extern_inline Sphere3f Sphere3fInit(const Vec3f origin, const float radius) {
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

fpl_extern_inline AABB2f AABB2fInit(const Vec2f min, const Vec2f max) {
	AABB2f result = fplStructInit(AABB2f, min, max);
	return result;
}

fpl_extern_inline AABB2f AABB2fInitFromCenter(const Vec2f center, const Vec2f radius) {
	Vec2f min = V2fSub(center, radius);
	Vec2f max = V2fAdd(center, radius);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

fpl_extern_inline AABB2f AABB2fInitFromBottomLeft(const Vec2f bottomLeft, const Vec2f size) {
	Vec2f min = bottomLeft;
	Vec2f max = V2fAdd(bottomLeft, size);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

fpl_extern_inline Vec2f AABB2fGetRadius(const AABB2f *aabb) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f result = V2fMultScalar(size, 0.5f);
	return result;
}

fpl_extern_inline Vec2f AABB2fGetSize(const AABB2f *aabb) {
	Vec2f result = V2fSub(aabb->max, aabb->min);
	return result;
}

fpl_extern_inline Vec2f AABB2fGetCenter(const AABB2f *aabb) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f ext = V2fMultScalar(size, 0.5f);
	Vec2f result = V2fAdd(aabb->min, ext);
	return result;
}

fpl_extern_inline void AABB2fExtract(const AABB2f *aabb, Vec2f *outCenter, Vec2f *outRadius) {
	Vec2f size = V2fSub(aabb->max, aabb->min);
	Vec2f radius = V2fMultScalar(size, 0.5f);
	Vec2f center = V2fAdd(aabb->min, radius);
	*outCenter = center;
	*outRadius = radius;
}

fpl_extern_inline bool AABB2fIntersects(const AABB2f *a, const AABB2f *b) {
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

fpl_extern_inline bool AABB2fContainsPoint(const AABB2f *aabb, const Vec2f point) {
	bool result = (point.x >= aabb->min.x && point.x <= aabb->max.x && point.y >= aabb->min.y && point.y <= aabb->max.y);
	return result;
}

fpl_extern_inline void AABB2fExpand(AABB2f *aabb, const Vec2f expansion) {
    aabb->min = V2fSub(aabb->min, expansion);
    aabb->max = V2fAdd(aabb->max, expansion);
}

fpl_extern_inline void AABB2fExpandScalar(AABB2f *aabb, const float scalar) {
    Vec2f e = V2fInitScalar(scalar);
    AABB2fExpand(aabb, e);
}

fpl_extern_inline AABB2f AABB2fGetIntersection(const AABB2f *a, const AABB2f *b) {
	Vec2f min = V2fMax(a->min, b->min);
    Vec2f max = V2fMin(a->max, b->max);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

fpl_extern_inline int AABB2fSubtraction(const AABB2f *a, const AABB2f *b, AABB2f out[4]) {
    int count = 0;

    // First compute intersection
    AABB2f inter = AABB2fGetIntersection(a, b);
    if (inter.min.x >= inter.max.x || inter.min.y >= inter.max.y) {
        // no overlap, result is just 'a'
        out[0] = *a;
        return 1;
    }

    // Bottom strip
    if (inter.max.y < a->max.y) {
        out[count++] = fplStructInit(AABB2f, {a->min.x, inter.max.y}, {a->max.x, a->max.y});
    }
    // Top strip
    if (inter.min.y > a->min.y) {
        out[count++] = fplStructInit(AABB2f, {a->min.x, a->min.y}, {a->max.x, inter.min.y});
    }
    // Left strip
    if (inter.min.x > a->min.x) {
        out[count++] = fplStructInit(AABB2f, {a->min.x, inter.min.y}, {inter.min.x, inter.max.y});
    }
    // Right strip
    if (inter.max.x < a->max.x) {
        out[count++] = fplStructInit(AABB2f, {inter.max.x, inter.min.y}, {a->max.x, inter.max.y});
    }

    return count;
}

//
// AABB3f
//
typedef struct AABB3f {
	Vec3f min;
	Vec3f max;
} AABB3f;

fpl_extern_inline AABB3f AABB3fInit(const Vec3f min, const Vec3f max) {
	AABB3f result = fplStructInit(AABB3f, min, max);
	return result;
}

fpl_extern_inline AABB3f AABB3fInitFromCenter(const Vec3f center, const Vec3f radius) {
	Vec3f min = V3fSub(center, radius);
	Vec3f max = V3fAdd(center, radius);
	AABB3f result = AABB3fInit(min, max);
	return result;
}

fpl_extern_inline Vec3f AABB3fGetRadius(const AABB3f *aabb) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f result = V3fMultScalar(size, 0.5f);
	return result;
}

fpl_extern_inline Vec3f AABB3fGetSize(const AABB3f *aabb) {
	Vec3f result = V3fSub(aabb->max, aabb->min);
	return result;
}

fpl_extern_inline Vec3f AABB3fGetCenter(const AABB3f *aabb) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f ext = V3fMultScalar(size, 0.5f);
	Vec3f result = V3fAdd(aabb->min, ext);
	return result;
}

fpl_extern_inline void AABB3fExtract(const AABB3f *aabb, Vec3f *outCenter, Vec3f *outRadius) {
	Vec3f size = V3fSub(aabb->max, aabb->min);
	Vec3f radius = V3fMultScalar(size, 0.5f);
	Vec3f center = V3fAdd(aabb->min, radius);
	*outCenter = center;
	*outRadius = radius;
}

fpl_extern_inline bool AABB3fIsOverlap(const AABB3f *a, const AABB3f *b) {
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

fpl_extern_inline bool AABB3fContainsPoint(const AABB3f *aabb, const Vec3f point) {
	Vec3f center, radius;
	AABB3fExtract(aabb, &center, &radius);
	Vec3f d = V3fAbs(V3fSub(point, center));
	bool result = (d.x <= radius.x) && (d.y <= radius.y) && (d.z <= radius.z);
	return result;
}

//
// Circle2f
//
typedef struct Circle2f {
	Vec2f center;
	float radius;
} Circle2f;

fpl_extern_inline Circle2f Circle2fInit(const Vec2f center, const float radius) {
	Circle2f result = fplStructInit(Circle2f, center, radius);
	return result;
}

fpl_extern_inline bool Circle2fContainsPoint(const Circle2f *circle, const Vec2f point) {
	Vec2f delta = V2fSub(point, circle->center);
	const float distanceSq = V2fDot(delta, delta);
	const float radiusSq = circle->radius * circle->radius;
	bool result = distanceSq <= radiusSq;
	return result;
}

//
// Arc2f
//
typedef struct Arc2f {
	Vec2f center;
	float radius;
	float startAngle;
	float endAngle;
	uint32_t padding;
} Arc2f;

fpl_extern_inline Arc2f Arc2fInit(const Vec2f center, const float radius, const float startAngle, const float endAngle) {
	Arc2f result = fplStructInit(Arc2f, center, radius, startAngle, endAngle);
	return result;
}

fpl_extern bool Arc2fIsPointInside(const Arc2f *arc, const Vec2f point);


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

fpl_extern bool Arc2fIsPointInside(const Arc2f *arc, const Vec2f point) {
	if (arc == fpl_null) {
		return false;
	}
	const float s = F32AngleNormalize(arc->endAngle < arc->startAngle ? arc->endAngle : arc->startAngle);
	const float e = F32AngleNormalize(arc->endAngle < arc->startAngle ? arc->startAngle : arc->endAngle);
	float range = e - s;
	if (range <= 0.0f) {
		range += F32Tau;
	}
	const Vec2f d = V2fSub(point, arc->center);
	const float a = F32ArcTan2(d.y, d.x);
	const float angle = F32AngleNormalize(a - s);
	const float lenSquared = V2fDot(d, d);
	const float radiusSquared = arc->radius * arc->radius;
	const bool insideCircle = lenSquared <= radiusSquared;
	const bool insideSegment = angle <= range;
	const bool result = insideCircle && insideSegment;
	return result;
}

#endif // FINAL_GEOMETRY_IMPLEMENTATION