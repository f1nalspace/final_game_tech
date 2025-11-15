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

typedef struct {
	Vec3f origin;
	Vec3f direction;
} Ray3f;

fpl_inline Ray3f MakeRay(const Vec3f origin, const Vec3f direction) {
	Ray3f result = { origin, direction };
	return(result);
}

typedef struct {
	Vec3f contact;
	Vec3f normal;
	float t;
	bool isHit;
} HitResult3f;

typedef union {
	struct {
		Vec3f normal;
		float distance;
	};
	Vec4f m;
} Plane3f;

typedef struct {
	Vec3f origin;
	float radius;
} Sphere3f;

typedef struct {
	Vec2f p1;
	Vec2f p2;
	float maxFraction;
} LineCastInput;

typedef struct {
	Vec2f normal;
	float fraction;
} LineCastOutput;

fpl_inline bool LineCastCircle(const LineCastInput *input, const Vec2f *center, const float radius, LineCastOutput *output) {
	if (input == fpl_null || center == fpl_null || output == fpl_null) {
		return false;
	}

	Vec2f s = input->p1 - *center;
	float b = V2fDot(s, s) - radius * radius;

	// Solve quadratic equation.
	Vec2f r = input->p2 - input->p1;
	float c = V2fDot(s, r);
	float rr = V2fDot(r, r);
	float sigma = c * c - rr * b;

	// Check for negative discriminant and short segment.
	if (sigma < 0.0f || rr < Epsilon) {
		return false;
	}

	// Find the point of intersection of the line with the circle.
	float a = -(c + SquareRoot(sigma));

	// Is the intersection point on the segment?
	if (0.0f <= a && a <= input->maxFraction * rr) {
		a /= rr;
		fplClearStruct(output);
		output->fraction = a;
		output->normal = V2fNormalize(s + a * r);
		return true;
	}

	return false;
}

#endif // FINAL_GEOMETRY_H