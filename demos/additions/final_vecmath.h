/*
Name:
	Final Vector Math
Description:
	Vector Math Library for defining/computing 2D/3D/4D Vectors, 2x2, 4x4 Matrices, etc.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_VECMATH_H
#define FINAL_VECMATH_H

#if !defined(__cplusplus)
#error "C++ is required for this math library!"
#endif

#include <stdint.h>
#include <stdlib.h> // rand, RAND_MAX

#define _USE_MATH_DEFINES
#include <math.h>

union Vec2i {
	struct {
		int x, y;
	};
	struct {
		int w, h;
	};
	int m[2];
};

inline Vec2i V2i() {
	Vec2i result = {};
	return(result);
}
inline Vec2i V2i(const Vec2i &v) {
	Vec2i result = { v.x, v.y };
	return(result);
}
inline Vec2i V2i(const int x, const int y) {
	Vec2i result = { x, y };
	return(result);
}

union Vec2f {
	struct {
		float x, y;
	};
	struct {
		float w, h;
	};
	float m[2];
};

inline Vec2f V2f() {
	Vec2f result = {};
	return(result);
}
inline Vec2f V2f(const Vec2f &v) {
	Vec2f result = { v.x, v.y };
	return(result);
}
inline Vec2f V2f(const float value) {
	Vec2f result = { value, value };
	return(result);
}
inline Vec2f V2f(const float x, const float y) {
	Vec2f result = { x, y };
	return(result);
}

union Mat2f {
	struct {
		Vec2f col1;
		Vec2f col2;
	};
	float m[4];
};

inline Mat2f M2f() {
	Mat2f result = { V2f(1, 0), V2f(0, 1) };
	return(result);
}

inline Mat2f M2f(const Mat2f &other) {
	Mat2f result = { other.col1, other.col2 };
	return(result);
}

union Vec3f {
	struct {
		float x, y, z;
	};
	struct {
		float u, v, w;
	};
	struct {
		float r, g, b;
	};
	struct {
		Vec2f xy;
		float ignored0;
	};
	struct {
		float ignored1;
		Vec2f yz;
	};
	struct {
		Vec2f uv;
		float ignored2;
	};
	struct {
		float ignored3;
		Vec2f vw;
	};
	float m[3];
};

inline Vec3f V3f() {
	Vec3f result = {};
	return(result);
}

inline Vec3f V3f(const float scalar) {
	Vec3f result = { scalar, scalar, scalar };
	return(result);
}

inline Vec3f V3f(const Vec3f &other) {
	Vec3f result = { other.x, other.y, other.z };
	return(result);
}

inline Vec3f V3f(const float x, const float y, const float z) {
	Vec3f result = { x, y, z };
	return(result);
}

union Vec4f {
	struct {
		union {
			Vec3f xyz;
			struct {
				float x, y, z;
			};
		};
		float w;
	};
	struct {
		union {
			Vec3f rgb;
			struct {
				float r, g, b;
			};
		};
		float a;
	};
	struct {
		Vec2f xy;
		float ignored0;
		float ignored1;
	};
	struct {
		float ignored2;
		Vec2f yz;
		float ignored3;
	};
	struct {
		float ignored4;
		float ignored5;
		Vec2f zw;
	};
	float m[4];
};

inline Vec4f V4f() {
	Vec4f result = { 0, 0, 0, 1 };
	return(result);
}
inline Vec4f V4f(const Vec4f &other) {
	Vec4f result = { other.x, other.y, other.z, other.w };
	return(result);
}
inline Vec4f V4f(const float x, const float y, const float z, const float w) {
	Vec4f result = { x, y, z, w };
	return(result);
}

union Mat4f {
	struct {
		Vec4f col1;
		Vec4f col2;
		Vec4f col3;
		Vec4f col4;
	};
	float m[16];
};

inline Mat4f M4f() {
	Mat4f result = {
		V4f(1.0f, 0.0f, 0.0f, 0.0f), 
		V4f(0.0f, 1.0f, 0.0f, 0.0f),
		V4f(0.0f, 0.0f, 1.0f, 0.0f),
		V4f(0.0f, 0.0f, 0.0f, 1.0f),
	};
	return(result);
}

inline Mat4f M4f(const Mat4f &other) {
	Mat4f result = { other.col1, other.col2, other.col3, other.col4 };
	return(result);
}

inline static Mat4f TransformationFromVec2(const Vec2f &p) {
	Mat4f result = Mat4f();
	result.col4.x = p.x;
	result.col4.y = p.y;
	result.col4.z = 0.0f;
	return (result);
}

inline static Mat4f ScaleFromVec2(const Vec2f &s) {
	Mat4f result = Mat4f();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = 0.0f;
	return (result);
}

union Pixel {
	struct {
		uint8_t r, g, b, a;
	};
	uint8_t m[4];
};

inline float ScalarLerp(float a, float t, float b) {
	float result = (1.0f - t) * a + t * b;
	return(result);
}

inline Vec2f operator*(const Vec2f &a, float b) {
	Vec2f result = V2f(a.x * b, a.y * b);
	return(result);
}
inline Vec2f& operator*=(Vec2f &a, float value) {
	a = a * value;
	return(a);
}
inline Vec2f operator-(const Vec2f &a) {
	Vec2f result = V2f(-a.x, -a.y);
	return(result);
}
inline Vec2f operator+(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2f(a.x + b.x, a.y + b.y);
	return(result);
}
inline Vec2f& operator+=(Vec2f &a, const Vec2f &b) {
	a = b + a;
	return(a);
}
inline Vec2f operator-(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2f(a.x - b.x, a.y - b.y);
	return(result);
}
inline Vec2f& operator-=(Vec2f &a, const Vec2f &b) {
	a = a - b;
	return(a);
}

inline float Vec2Dot(const Vec2f &a, const Vec2f &b) {
	float result = a.x * b.x + a.y * b.y;
	return(result);
}

inline float Vec2Length(const Vec2f &v) {
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return(result);
}

inline Vec2f Vec2Normalize(const Vec2f &v) {
	float l = Vec2Length(v);
	if(l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec2f result = Vec2f(v) * invL;
	return(result);
}

inline Vec2f Vec2Hadamard(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2f(a.x * b.x, a.y * b.y);
	return(result);
}

inline Vec2f Vec2MultMat2(const Mat2f &A, const Vec2f &v) {
	Vec2f result = V2f(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
	return(result);
}

inline float Vec2DistanceSquared(const Vec2f &a, const Vec2f &b) {
	float f = (b.x - a.x) * (b.y - a.y);
	float result = f * f;
	return(result);
}

/* Returns the right perpendicular vector */
inline Vec2f Vec2Cross(const Vec2f &a, float s) {
	return V2f(s * a.y, -s * a.x);
}

/* Returns the left perpendicular vector */
inline Vec2f Vec2Cross(float s, const Vec2f &a) {
	return V2f(-s * a.y, s * a.x);
}

inline float Vec2Cross(const Vec2f &a, const Vec2f &b) {
	return a.x * b.y - a.y * b.x;
}

inline float Vec2AxisToAngle(const Vec2f &axis) {
	float result = atan2f(axis.y, axis.x);
	return(result);
}

inline Vec2f Vec2RandomDirection() {
	float d = rand() / (float)RAND_MAX;
	float angle = d * ((float)M_PI * 2.0f);
	Vec2f result = V2f(cosf(angle), sinf(angle));
	return(result);
}

inline Vec2f Vec2Lerp(const Vec2f &a, float t, const Vec2f &b) {
	Vec2f result;
	result.x = ScalarLerp(a.x, t, b.x);
	result.y = ScalarLerp(a.y, t, b.y);
	return(result);
}

//
// Vec3f
//
inline Vec3f operator*(float a, const Vec3f &b) {
	Vec3f result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return(result);
}
inline Vec3f operator*(const Vec3f &a, float b) {
	Vec3f result = b * a;
	return(result);
}
inline Vec3f& operator*=(Vec3f &a, float value) {
	a = value * a;
	return(a);
}

//
// Mat2f
//
inline Mat2f Mat2Identity() {
	Mat2f result = M2f();
	return (result);
}

inline Mat2f Mat2FromAngle(float angle) {
	float s = sinf(angle);
	float c = cosf(angle);
	Mat2f result;
	result.col1 = V2f(c, s);
	result.col2 = V2f(-s, c);
	return(result);
}

inline Mat2f Mat2FromAxis(const Vec2f &axis) {
	Mat2f result;
	result.col1 = axis;
	result.col2 = Vec2Cross(1.0f, axis);
	return(result);
}

inline Mat2f Mat2Transpose(const Mat2f &m) {
	Mat2f result;
	result.col1 = V2f(m.col1.x, m.col2.x);
	result.col2 = V2f(m.col1.y, m.col2.y);
	return(result);
}

inline Mat2f Mat2Mult(const Mat2f &a, const Mat2f &b) {
	Mat2f result;
	result.col1 = Vec2MultMat2(a, b.col1);
	result.col2 = Vec2MultMat2(a, b.col2);
	return(result);
}

inline float Mat2ToAngle(const Mat2f &mat) {
	float result = Vec2AxisToAngle(mat.col1);
	return(result);
}

/* Generates a 2x2 matrix for doing B to A conversion */
inline Mat2f Mat2MultTranspose(const Mat2f &a, const Mat2f &b) {
	Mat2f result;
	result.col1 = V2f(Vec2Dot(a.col1, b.col1), Vec2Dot(a.col2, b.col1));
	result.col2 = V2f(Vec2Dot(a.col1, b.col2), Vec2Dot(a.col2, b.col2));
	return(result);
}



inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	Mat4f result;
	for(int i = 0; i < 16; i += 4) {
		for(int j = 0; j < 4; ++j) {
			result.m[i + j] =
				(b.m[i + 0] * a.m[j + 0])
				+ (b.m[i + 1] * a.m[j + 4])
				+ (b.m[i + 2] * a.m[j + 8])
				+ (b.m[i + 3] * a.m[j + 12]);
		}
	}
	return(result);
}

const static Vec4f ColorWhite = V4f(1.0f, 1.0f, 1.0f, 1.0f);
const static Vec4f ColorRed = V4f(1.0f, 0.0f, 0.0f, 1.0f);
const static Vec4f ColorGreen = V4f(0.0f, 1.0f, 0.0f, 1.0f);
const static Vec4f ColorBlue = V4f(0.0f, 0.0f, 1.0f, 1.0f);
const static Vec4f ColorLightGray = V4f(0.3f, 0.3f, 0.3f, 1.0f);
const static Vec4f ColorDarkGray = V4f(0.2f, 0.2f, 0.2f, 1.0f);

inline Pixel RGBA32ToPixel(const uint32_t rgba) {
	Pixel result = { (uint8_t)((rgba >> 0) & 0xFF), (uint8_t)((rgba >> 8) & 0xFF), (uint8_t)((rgba >> 16) & 0xFF), (uint8_t)((rgba >> 24) & 0xFF) };
	return(result);
}

inline uint32_t RGBA32(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r << 0);
	return(result);
}

const static float INV255 = 1.0f / 255.0f;

inline Vec4f PixelToLinear(const Pixel &pixel) {
	Vec4f result = V4f(pixel.r * INV255, pixel.g * INV255, pixel.b * INV255, pixel.a * INV255);
	return(result);
}

inline Vec4f RGBA32ToLinear(const uint32_t rgba) {
	Pixel pixel = RGBA32ToPixel(rgba);
	Vec4f result = PixelToLinear(pixel);
	return(result);
}

inline Vec4f AlphaToLinear(const uint8_t alpha) {
	float a = alpha * INV255;
	Vec4f result = V4f(1, 1, 1, a);
	return(result);
}

inline uint32_t LinearToRGBA32(const Vec4f &linear) {
	uint8_t r = (uint8_t)((linear.x * 255.0f) + 0.5f);
	uint8_t g = (uint8_t)((linear.y * 255.0f) + 0.5f);
	uint8_t b = (uint8_t)((linear.z * 255.0f) + 0.5f);
	uint8_t a = (uint8_t)((linear.w * 255.0f) + 0.5f);
	uint32_t result = RGBA32(r, g, b, a);
	return(result);
}

#endif // FINAL_VECMATH_H