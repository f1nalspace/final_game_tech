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
	## 2026-10-06
	- New: Ray intersections for Plane3f, Sphere3f, AABB3f, Quad3f and OrientedBox3f
	- New: AABB3f empty, grow and half surface area functions
	- New: Quad3f and OrientedBox3f types

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

// Direction components smaller than this are clamped before the reciprocal
static const float Ray3fParallelEpsilon = 1e-20f;

// 1/value without infinities: avoids 0*inf = NaN in slab tests when the origin lies exactly on a slab plane
fpl_extern_inline float Ray3fSafeReciprocal(const float value) {
	float absoluteValue = F32Abs(value);
	float clampedValue = copysignf(Ray3fParallelEpsilon, value);
	float safeValue = (absoluteValue > Ray3fParallelEpsilon) ? value : clampedValue;
	float result = 1.0f / safeValue;
	return result;
}

fpl_extern_inline Vec3f Ray3fInverseDirection(const Vec3f direction) {
	float inverseX = Ray3fSafeReciprocal(direction.x);
	float inverseY = Ray3fSafeReciprocal(direction.y);
	float inverseZ = Ray3fSafeReciprocal(direction.z);
	Vec3f result = V3fInit(inverseX, inverseY, inverseZ);
	return result;
}

typedef struct HitResult3f {
	Vec3f contact;
	Vec3f normal;
	float t;
	bool isHit;
} HitResult3f;

//
// Ray intersections: *IntersectRay functions return true only for the nearest t with tMin < t < tMax (the !(t > tMin && t < tMax) form also rejects NaN)
//

//
// Plane3f
//
// All points p with dot(normal, p) == distance
typedef union Plane3f {
	struct {
		Vec3f normal;
		float distance;
	};
	Vec4f m;
} Plane3f;

fpl_extern_inline Plane3f Plane3fInitFromPoint(const Vec3f normal, const Vec3f point) {
	Plane3f result;
	result.normal = normal;
	result.distance = V3fDot(normal, point);
	return result;
}

fpl_extern_inline bool Plane3fIntersectRay(const Plane3f *plane, const Ray3f *ray, const float tMin, const float tMax, float *outT) {
	float denominator = V3fDot(plane->normal, ray->direction);
	if (denominator == 0.0f) {
		return false;
	}
	float originDot = V3fDot(plane->normal, ray->origin);
	float originDistance = plane->distance - originDot;
	float t = originDistance / denominator;
	if (!(t > tMin && t < tMax)) {
		return false;
	}
	*outT = t;
	return true;
}

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

// Numerically stable form of Ray Tracing Gems ch. 7, the ray direction must be unit length
fpl_extern_inline bool Sphere3fIntersectRay(const Sphere3f *sphere, const Ray3f *ray, const float tMin, const float tMax, float *outT) {
	Vec3f centerToOrigin = V3fSub(ray->origin, sphere->origin);
	float halfB = V3fDot(centerToOrigin, ray->direction);
	Vec3f alongDirection = V3fMultScalar(ray->direction, halfB);
	Vec3f perpendicular = V3fSub(centerToOrigin, alongDirection);
	float radiusSquared = sphere->radius * sphere->radius;
	float perpendicularSquared = V3fDot(perpendicular, perpendicular);
	float discriminant = radiusSquared - perpendicularSquared;
	if (discriminant < 0.0f) {
		return false;
	}
	float originDistanceSquared = V3fDot(centerToOrigin, centerToOrigin);
	float c = originDistanceSquared - radiusSquared;
	float discriminantRoot = F32SquareRoot(discriminant);
	float signedRoot = copysignf(discriminantRoot, halfB);
	float q = -(halfB + signedRoot);
	float rootFromC = c / q; // q == 0 only for a tangent ray starting on the surface: NaN, rejected below
	float rootFromQ = q;
	float tNear = F32Min(rootFromC, rootFromQ);
	float tFar = F32Max(rootFromC, rootFromQ);
	if (tNear > tMin && tNear < tMax) {
		*outT = tNear;
		return true;
	}
	if (tFar > tMin && tFar < tMax) {
		*outT = tFar;
		return true;
	}
	return false;
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

// Inverted box, the starting point for AABB3fGrowPoint and AABB3fGrowBox
fpl_extern_inline AABB3f AABB3fInitEmpty(void) {
	Vec3f min = V3fInitScalar(F32MaxValue);
	Vec3f max = V3fInitScalar(-F32MaxValue);
	AABB3f result = AABB3fInit(min, max);
	return result;
}

fpl_extern_inline void AABB3fGrowPoint(AABB3f *aabb, const Vec3f point) {
	for (uint32_t axis = 0; axis < 3; ++axis) {
		aabb->min.m[axis] = F32Min(aabb->min.m[axis], point.m[axis]);
		aabb->max.m[axis] = F32Max(aabb->max.m[axis], point.m[axis]);
	}
}

// Combines min with min and max with max, growing by the corners of an empty box would push the maximum to +F32MaxValue
fpl_extern_inline void AABB3fGrowBox(AABB3f *aabb, const AABB3f *other) {
	for (uint32_t axis = 0; axis < 3; ++axis) {
		aabb->min.m[axis] = F32Min(aabb->min.m[axis], other->min.m[axis]);
		aabb->max.m[axis] = F32Max(aabb->max.m[axis], other->max.m[axis]);
	}
}

// Half of the surface area, enough for surface area heuristics
fpl_extern_inline float AABB3fGetHalfSurfaceArea(const AABB3f *aabb) {
	Vec3f extent = V3fSub(aabb->max, aabb->min);
	float result = extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
	return result;
}

// 1 + 2*gamma(3) (Ize 2013), makes the slab test conservative
static const float AABB3fRobustSlabScale = 1.0000004f;

// Slab test with a precomputed Ray3fInverseDirection, conservative so grazing hits are never culled; returns the entry distance or F32MaxValue on a miss
fpl_extern_inline float AABB3fIntersectRayInverse(const AABB3f *aabb, const Vec3f origin, const Vec3f inverseDirection, const float tMin, const float tMax) {
	float tx0 = (aabb->min.x - origin.x) * inverseDirection.x;
	float tx1 = (aabb->max.x - origin.x) * inverseDirection.x;
	float ty0 = (aabb->min.y - origin.y) * inverseDirection.y;
	float ty1 = (aabb->max.y - origin.y) * inverseDirection.y;
	float tz0 = (aabb->min.z - origin.z) * inverseDirection.z;
	float tz1 = (aabb->max.z - origin.z) * inverseDirection.z;
	float nearX = F32Min(tx0, tx1);
	float farX = F32Max(tx0, tx1);
	float nearY = F32Min(ty0, ty1);
	float farY = F32Max(ty0, ty1);
	float nearZ = F32Min(tz0, tz1);
	float farZ = F32Max(tz0, tz1);
	float nearXY = F32Max(nearX, nearY);
	float nearZClamped = F32Max(nearZ, tMin);
	float tNear = F32Max(nearXY, nearZClamped);
	float farXY = F32Min(farX, farY);
	float farZClamped = F32Min(farZ, tMax);
	float tFarUnscaled = F32Min(farXY, farZClamped);
	float tFar = tFarUnscaled * AABB3fRobustSlabScale;
	float result = (tNear <= tFar) ? tNear : F32MaxValue;
	return result;
}

//
// Quad3f
//
// Parallelogram corner + u * edgeU + v * edgeV with u,v in [0,1], normal = normalize(cross(edgeU, edgeV))
typedef struct Quad3f {
	Vec3f corner;
	Vec3f edgeU;
	Vec3f edgeV;
	Vec3f normal;
	Vec3f dualU;    // coordinate along edgeU = dot(point - corner, dualU)
	Vec3f dualV;    // coordinate along edgeV = dot(point - corner, dualV)
	float distance; // dot(normal, corner), same plane convention as Plane3f
	float area;
} Quad3f;

fpl_extern_inline Quad3f Quad3fInit(const Vec3f corner, const Vec3f edgeU, const Vec3f edgeV) {
	Vec3f crossUV = V3fCross(edgeU, edgeV);
	float crossLengthSquared = V3fDot(crossUV, crossUV);
	fplAssert(crossLengthSquared > 0.0f);
	float crossLength = F32SquareRoot(crossLengthSquared);
	float inverseCrossLengthSquared = 1.0f / crossLengthSquared;
	float inverseCrossLength = 1.0f / crossLength;
	Vec3f planeW = V3fMultScalar(crossUV, inverseCrossLengthSquared);
	Quad3f result;
	result.corner = corner;
	result.edgeU = edgeU;
	result.edgeV = edgeV;
	result.normal = V3fMultScalar(crossUV, inverseCrossLength);
	result.dualU = V3fCross(edgeV, planeW);
	result.dualV = V3fCross(planeW, edgeU);
	result.distance = V3fDot(result.normal, corner);
	result.area = crossLength;
	return result;
}

// Plane test, then the coordinates inside the parallelogram; inclusive so adjacent quads overlap on shared edges
fpl_extern_inline bool Quad3fIntersectRay(const Quad3f *quad, const Ray3f *ray, const float tMin, const float tMax, float *outT) {
	float denominator = V3fDot(quad->normal, ray->direction);
	if (denominator == 0.0f) {
		return false;
	}
	float originDot = V3fDot(quad->normal, ray->origin);
	float originDistance = quad->distance - originDot;
	float t = originDistance / denominator;
	if (!(t > tMin && t < tMax)) {
		return false;
	}
	Vec3f alongDirection = V3fMultScalar(ray->direction, t);
	Vec3f hitPoint = V3fAdd(ray->origin, alongDirection);
	Vec3f planarHit = V3fSub(hitPoint, quad->corner);
	float coordinateU = V3fDot(planarHit, quad->dualU);
	float coordinateV = V3fDot(planarHit, quad->dualV);
	if (coordinateU < 0.0f || coordinateU > 1.0f || coordinateV < 0.0f || coordinateV > 1.0f) {
		return false;
	}
	*outT = t;
	return true;
}

fpl_extern_inline AABB3f Quad3fGetBounds(const Quad3f *quad) {
	Vec3f cornerU = V3fAdd(quad->corner, quad->edgeU);
	Vec3f cornerV = V3fAdd(quad->corner, quad->edgeV);
	Vec3f cornerUV = V3fAdd(cornerU, quad->edgeV);
	AABB3f result = AABB3fInitEmpty();
	AABB3fGrowPoint(&result, quad->corner);
	AABB3fGrowPoint(&result, cornerU);
	AABB3fGrowPoint(&result, cornerV);
	AABB3fGrowPoint(&result, cornerUV);
	return result;
}

//
// OrientedBox3f
//
// The axes are orthonormal (rotation only), the size lives in halfExtents
typedef struct OrientedBox3f {
	Vec3f center;
	Vec3f halfExtents;
	Vec3f axisX;
	Vec3f axisY;
	Vec3f axisZ;
} OrientedBox3f;

fpl_extern_inline OrientedBox3f OrientedBox3fInit(const Vec3f center, const Vec3f halfExtents, const Vec3f axisX, const Vec3f axisY, const Vec3f axisZ) {
	OrientedBox3f result = fplStructInit(OrientedBox3f, center, halfExtents, axisX, axisY, axisZ);
	return result;
}

// Slab test in box space; a pure rotation keeps the local direction unit length, so t is the same in both spaces
fpl_extern_inline bool OrientedBox3fIntersectRay(const OrientedBox3f *box, const Ray3f *ray, const float tMin, const float tMax, float *outT) {
	Vec3f relativeOrigin = V3fSub(ray->origin, box->center);
	float localOriginX = V3fDot(relativeOrigin, box->axisX);
	float localOriginY = V3fDot(relativeOrigin, box->axisY);
	float localOriginZ = V3fDot(relativeOrigin, box->axisZ);
	float localDirectionX = V3fDot(ray->direction, box->axisX);
	float localDirectionY = V3fDot(ray->direction, box->axisY);
	float localDirectionZ = V3fDot(ray->direction, box->axisZ);
	Vec3f localOrigin = V3fInit(localOriginX, localOriginY, localOriginZ);
	Vec3f localDirection = V3fInit(localDirectionX, localDirectionY, localDirectionZ);
	Vec3f inverseDirection = Ray3fInverseDirection(localDirection);
	float tEnter = -F32MaxValue;
	float tExit = F32MaxValue;
	for (uint32_t axis = 0; axis < 3; ++axis) {
		float halfExtent = box->halfExtents.m[axis];
		float slabNear = (-halfExtent - localOrigin.m[axis]) * inverseDirection.m[axis];
		float slabFar = (halfExtent - localOrigin.m[axis]) * inverseDirection.m[axis];
		if (slabNear > slabFar) {
			float swapTemp = slabNear;
			slabNear = slabFar;
			slabFar = swapTemp;
		}
		tEnter = F32Max(slabNear, tEnter);
		tExit = F32Min(slabFar, tExit);
	}
	if (tEnter > tExit) {
		return false;
	}
	if (tEnter > tMin && tEnter < tMax) {
		*outT = tEnter;
		return true;
	}
	if (tExit > tMin && tExit < tMax) {
		*outT = tExit;
		return true;
	}
	return false;
}

fpl_extern_inline AABB3f OrientedBox3fGetBounds(const OrientedBox3f *box) {
	Vec3f extent;
	for (uint32_t axis = 0; axis < 3; ++axis) {
		float extentFromX = F32Abs(box->axisX.m[axis]) * box->halfExtents.x;
		float extentFromY = F32Abs(box->axisY.m[axis]) * box->halfExtents.y;
		float extentFromZ = F32Abs(box->axisZ.m[axis]) * box->halfExtents.z;
		extent.m[axis] = extentFromX + extentFromY + extentFromZ;
	}
	AABB3f result = AABB3fInitFromCenter(box->center, extent);
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