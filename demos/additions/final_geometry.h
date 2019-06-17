#ifndef FINAL_GEOMETRY_H
#define FINAL_GEOMETRY_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include "final_math.h"

struct Ray3f {
	Vec3f origin;
	Vec3f direction;
};

inline Ray3f MakeRay(const Vec3f &origin, const Vec3f &direction) {
	Ray3f result = { origin, direction };
	return(result);
}

struct HitResult3f {
	Vec3f contact;
	Vec3f normal;
	float t;
	bool isHit;
};

union Plane3f {
	struct {
		Vec3f normal;
		float distance;
	};
	Vec4f m;
};

struct Sphere3f {
	Vec3f origin;
	float radius;
};

struct LineCastInput {
	Vec2f p1;
	Vec2f p2;
	float maxFraction;
};

struct LineCastOutput {
	Vec2f normal;
	float fraction;
};

bool LineCastCircle(const LineCastInput &input, const Vec2f &center, const float radius, LineCastOutput &output) {
	Vec2f s = input.p1 - center;
	float b = Vec2Dot(s, s) - radius * radius;

	// Solve quadratic equation.
	Vec2f r = input.p2 - input.p1;
	float c = Vec2Dot(s, r);
	float rr = Vec2Dot(r, r);
	float sigma = c * c - rr * b;

	// Check for negative discriminant and short segment.
	if (sigma < 0.0f || rr < Epsilon) {
		return false;
	}

	// Find the point of intersection of the line with the circle.
	float a = -(c + SquareRoot(sigma));

	// Is the intersection point on the segment?
	if (0.0f <= a && a <= input.maxFraction * rr) {
		a /= rr;
		output.fraction = a;
		output.normal = Vec2Normalize(s + a * r);
		return true;
	}

	return false;
}

#endif // FINAL_GEOMETRY_H