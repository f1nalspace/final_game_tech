/*
Name:
	Final Math

Description:
	Math Library for defining/computing 2D/3D/4D Vectors, 2x2, 4x4 Matrices, etc.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2018 Torsten Spaete

Changelog
	## 2019-05-10:
	- Added Vec3f math operator overloaded functions
	- Renamed Mat4OrthoLH to Mat4OrthoRH
	- Added Mat4PerspectiveRH
	- Added Mat4LookAtRH
	- Added more overrides for Mat4Translation
	- Added SRGBToLinear, LinearToSRGB
	- Added default SRGB conversion to Linear <-> Pixel

	## 2018-07-05:
	- Added Mat4RotationX, Mat4RotationY, Mat4RotationZ
*/

#ifndef FINAL_MATH_H
#define FINAL_MATH_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include <stdint.h> // uint32_t, etc.
#include <stdlib.h> // rand, RAND_MAX

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

constexpr float Pi32 = (float)M_PI;
constexpr float Tau32 = (float)M_PI * 2.0f;
constexpr float Deg2Rad = (float)M_PI / 180.0f;
constexpr float Rad2Deg = 180.0f / (float)M_PI;
constexpr float Epsilon = FLT_EPSILON;

//
// Vector types
//

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

inline Vec4f V4f(const float x, const float y, const float z, const float w = 1.0f) {
	Vec4f result = { x, y, z, w };
	return(result);
}

inline Vec4f V4f(const Vec3f &v, const float w = 1.0f) {
	Vec4f result = { v.x, v.y, v.z, w };
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

union Mat4f {
	struct {
		Vec4f col1;
		Vec4f col2;
		Vec4f col3;
		Vec4f col4;
	};
	struct {
		float r[4][4];
	};
	float m[16];
};

inline Mat4f M4f(const float value = 1.0f) {
	Mat4f result = {
		V4f(value, 0.0f, 0.0f, 0.0f),
		V4f(0.0f, value, 0.0f, 0.0f),
		V4f(0.0f, 0.0f, value, 0.0f),
		V4f(0.0f, 0.0f, 0.0f, value),
	};
	return(result);
}

inline Mat4f M4f(const Mat4f &other) {
	Mat4f result = { other.col1, other.col2, other.col3, other.col4 };
	return(result);
}

union Pixel {
	struct {
		uint8_t b, g, r, a;
	};
	uint32_t bgra;
	uint8_t m[4];
};

//
// Scalar
//

inline float Cosine(const float angle) {
	float result = cosf(angle);
	return(result);
}
inline float Sine(const float angle) {
	float result = sinf(angle);
	return(result);
}
inline float Tan(const float angle) {
	float result = tanf(angle);
	return(result);
}
inline float ArcTan2(const float y, const float x) {
	float result = atan2f(y, x);
	return(result);
}
inline float Abs(const float value) {
	float result = fabsf(value);
	return(result);
}
inline float Min(const float a, const float b) {
	float result = a < b ? a : b;
	return(result);
}
inline float Max(const float a, const float b) {
	float result = a > b ? a : b;
	return(result);
}
inline float SquareRoot(const float value) {
	float result = sqrtf(value);
	return(result);
}
inline float Degrees(const float radians) {
	float result = radians * Rad2Deg;
	return(result);
}
inline float Radians(const float degrees) {
	float result = degrees * Deg2Rad;
	return(result);
}
inline float Power(const float x, const float y) {
	float result = powf(x, y);
	return(result);
}

inline float ScalarLerp(float a, float t, float b) {
	float result = (1.0f - t) * a + t * b;
	return(result);
}

inline float GetBestAngleDistance(float a0, float a1) {
	float max = Pi32 * 2;
	float da = fmodf(a1 - a0, max);
	float result = fmodf(2.0f * da, max) - da;
	return(result);
}

inline float AngleLerp(float a, float t, float b) {
	float angleDistance = GetBestAngleDistance(a, b);
	float result = ScalarLerp(a, t, a + angleDistance);
	return(result);
}

inline uint8_t RoundF32ToU8(float value) {
	uint8_t result = (uint8_t)(value * 255.0f + 0.5f);
	return(result);
}

//
// Vec2f
//
inline Vec2f operator*(const Vec2f &a, float b) {
	Vec2f result = V2f(a.x * b, a.y * b);
	return(result);
}

inline Vec2f operator*(float b, const Vec2f &a) {
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
	a = a + b;
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
	if (l == 0) {
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

/* Returns the Z-rotation from two vectors */
inline float Vec2Cross(const Vec2f &a, const Vec2f &b) {
	return a.x * b.y - a.y * b.x;
}

inline float Vec2AxisToAngle(const Vec2f &axis) {
	float result = ArcTan2(axis.y, axis.x);
	return(result);
}

inline Vec2f Vec2AngleToAxis(const float angle) {
	Vec2f result = V2f(Cosine(angle), Sine(angle));
	return(result);
}

inline Vec2f Vec2RandomDirection() {
	float d = rand() / (float)RAND_MAX;
	float angle = d * ((float)M_PI * 2.0f);
	Vec2f result = V2f(Cosine(angle), Sine(angle));
	return(result);
}

inline Vec2f Vec2Lerp(const Vec2f &a, float t, const Vec2f &b) {
	Vec2f result;
	result.x = ScalarLerp(a.x, t, b.x);
	result.y = ScalarLerp(a.y, t, b.y);
	return(result);
}

//
// Vec2i
//
inline bool IsVec2Equals(const Vec2i &a, const Vec2i &b) {
	bool result = a.x == b.x && a.y == b.y;
	return(result);
}

//
// Vec3f
//
inline Vec3f operator*(float s, const Vec3f &v) {
	Vec3f result = V3f(s * v.x, s * v.y, s * v.z);
	return(result);
}

inline Vec3f operator*(const Vec3f &v, float s) {
	Vec3f result = s * v;
	return(result);
}

inline Vec3f& operator*=(Vec3f &v, float s) {
	v = s * v;
	return(v);
}

inline Vec3f operator+(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3f(a.x + b.x, a.y + b.y, a.z + b.z);
	return(result);
}

inline Vec3f& operator+=(Vec3f &a, const Vec3f &b) {
	a = a + b;
	return(a);
}

inline Vec3f operator-(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3f(a.x - b.x, a.y - b.y, a.z - b.z);
	return(result);
}

inline Vec3f operator-(const Vec3f &v) {
	Vec3f result = V3f(-v.x, -v.y, -v.z);
	return(result);
}

inline Vec3f& operator-=(Vec3f &a, const Vec3f &b) {
	a = a - b;
	return(a);
}

inline float Vec3Dot(const Vec3f &a, const Vec3f &b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return(result);
}

inline float Vec3DistanceSquared(const Vec3f &a, const Vec3f &b) {
	float f = (b.x - a.x) * (b.y - a.y) * (b.z - a.z);
	float result = f * f;
	return(result);
}

inline float Vec3Length(const Vec3f &v) {
	float result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return(result);
}

inline Vec3f Vec3Normalize(const Vec3f &v) {
	float l = Vec3Length(v);
	if (l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec3f result = V3f(v) * invL;
	return(result);
}

inline Vec3f Vec3Cross(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3f(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
	return(result);
}

inline Vec3f Vec3Lerp(const Vec3f &a, float t, const Vec3f &b) {
	Vec3f result;
	result.x = ScalarLerp(a.x, t, b.x);
	result.y = ScalarLerp(a.y, t, b.y);
	result.z = ScalarLerp(a.z, t, b.z);
	return(result);
}

inline Vec3f Vec3Hadamard(const Vec3f &a, const Vec3f &b) {
	Vec3f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return(result);
}

// Mat2f
//
inline Mat2f Mat2FromAngle(float angle) {
	float s = Sine(angle);
	float c = Cosine(angle);
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

//
// Mat4f
//
inline static Mat4f Mat4OrthoRH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	Mat4f result = M4f();
	result.r[0][0] = 2.0f / (right - left);
	result.r[1][1] = 2.0f / (top - bottom);
	result.r[2][2] = -2.0f / (zFar - zNear);
	result.r[3][0] = -(right + left) / (right - left);
	result.r[3][1] = -(top + bottom) / (top - bottom);
	result.r[3][2] = -(zFar + zNear) / (zFar - zNear);
	return (result);
}

inline static Mat4f Mat4PerspectiveRH(const float fov, const float aspect, const float zNear, const float zFar) {
	float tanHalfFov = Tan(fov / 2.0f);
	Mat4f result = M4f(0.0f);
	result.r[0][0] = 1.0f / (aspect * tanHalfFov);
	result.r[1][1] = 1.0f / (tanHalfFov);
	result.r[2][2] = -(zFar + zNear) / (zFar - zNear);
	result.r[2][3] = -1.0f;
	result.r[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	return (result);
}

inline static Mat4f Mat4LookAtRH(const Vec3f &eye, const Vec3f &center, const Vec3f &up) {
	// Forward/Side/Upward
	const Vec3f f = Vec3Normalize(center - eye);
	const Vec3f s = Vec3Normalize(Vec3Cross(f, up));
	const Vec3f u = Vec3Cross(s, f);

	Mat4f result = M4f();

	// X/Y/Z Rotation
	result.r[0][0] = s.x;
	result.r[1][0] = s.y;
	result.r[2][0] = s.z;

	result.r[0][1] = u.x;
	result.r[1][1] = u.y;
	result.r[2][1] = u.z;

	result.r[0][2] = -f.x;
	result.r[1][2] = -f.y;
	result.r[2][2] = -f.z;

	// Translation
	result.r[3][0] = -Vec3Dot(s, eye);
	result.r[3][1] = -Vec3Dot(u, eye);
	result.r[3][2] = Vec3Dot(f, eye);

	return (result);
}

inline static Mat4f Mat4Translation(const Vec4f &p) {
	Mat4f result = M4f();
	result.col4 = p;
	return (result);
}

inline static Mat4f Mat4Translation(const Vec3f &p) {
	Mat4f result = M4f();
	result.col4.xyz = p;
	result.col4.w = 1.0f;
	return (result);
}

inline static Mat4f Mat4Translation(const Vec2f &p) {
	Mat4f result = M4f();
	result.col4.xy = p;
	result.col4.z = 0.0f;
	result.col4.w = 1.0f;
	return (result);
}

inline static Mat4f Mat4Scale(const Vec2f &s) {
	Mat4f result = M4f();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = 0.0f;
	return (result);
}

inline static Mat4f Mat4Scale(const Vec3f &s) {
	Mat4f result = M4f();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	return (result);
}

inline static Mat4f Mat4Scale(const Vec4f &s) {
	Mat4f result = M4f();
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	result.col3.w = s.w;
	return (result);
}

inline static Mat4f Mat4RotationX(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4f(1.0f, 0.0f, 0.0f, 0.0f);
	result.col2 = V4f(0.0f, c, s, 0.0f);
	result.col3 = V4f(0.0f, -s, c, 0.0f);
	result.col4 = V4f(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline static Mat4f Mat4RotationY(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4f(c, 0.0f, s, 0.0f);
	result.col2 = V4f(0.0f, 1.0f, 0.0f, 0.0f);
	result.col3 = V4f(-s, 0.0f, c, 0.0f);
	result.col4 = V4f(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline static Mat4f Mat4RotationZ(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4f(c, s, 0.0f, 0.0f);
	result.col2 = V4f(-s, c, 0.0f, 0.0f);
	result.col3 = V4f(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4f(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline static Mat4f Mat4RotationZ(const Mat2f &m) {
	Mat4f result;
	result.col1 = V4f(m.col1.x, m.col1.y, 0.0f, 0.0f);
	result.col2 = V4f(-m.col1.y, m.col1.x, 0.0f, 0.0f);
	result.col3 = V4f(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4f(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	Mat4f result;
	for (int i = 0; i < 16; i += 4) {
		for (int j = 0; j < 4; ++j) {
			result.m[i + j] =
				(b.m[i + 0] * a.m[j + 0])
				+ (b.m[i + 1] * a.m[j + 4])
				+ (b.m[i + 2] * a.m[j + 8])
				+ (b.m[i + 3] * a.m[j + 12]);
		}
	}
	return(result);
}

//
// Pixel
//

const static Vec4f ColorWhite = V4f(1.0f, 1.0f, 1.0f, 1.0f);
const static Vec4f ColorRed = V4f(1.0f, 0.0f, 0.0f, 1.0f);
const static Vec4f ColorGreen = V4f(0.0f, 1.0f, 0.0f, 1.0f);
const static Vec4f ColorBlue = V4f(0.0f, 0.0f, 1.0f, 1.0f);
const static Vec4f ColorLightGray = V4f(0.3f, 0.3f, 0.3f, 1.0f);
const static Vec4f ColorDarkGray = V4f(0.2f, 0.2f, 0.2f, 1.0f);

inline Pixel MakePixel(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Pixel result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return(result);
}
inline Pixel MakePixel(const uint32_t rgba) {
	Pixel result;
	result.r = (uint8_t)((rgba >> 0) & 0xFF);
	result.g = (uint8_t)((rgba >> 8) & 0xFF);
	result.b = (uint8_t)((rgba >> 16) & 0xFF);
	result.a = (uint8_t)((rgba >> 24) & 0xFF);
	return(result);
}

inline uint32_t RGBA8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r << 0);
	return(result);
}
inline uint32_t RGBA8(const Pixel &pixel) {
	uint32_t result = RGBA8(pixel.r, pixel.g, pixel.b, pixel.a);
	return(result);
}

inline uint32_t BGRA8(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	uint32_t result = (a << 24) | (r << 16) | (g << 8) | (b << 0);
	return(result);
}
inline uint32_t BGRA8(const Pixel &pixel) {
	uint32_t result = pixel.bgra;
	return(result);
}

inline uint32_t RGBAPack4x8(const Vec4f &unpacked) {
	uint32_t result = (
		(RoundF32ToU8(unpacked.a) << 24) |
		(RoundF32ToU8(unpacked.b) << 16) |
		(RoundF32ToU8(unpacked.g) << 8) |
		(RoundF32ToU8(unpacked.r) << 0));
	return(result);
}
inline Vec4f RGBAUnpack4x8(const uint32_t packed) {
	Vec4f result;
	result.r = (float)((packed >> 0) & 0xFF);
	result.g = (float)((packed >> 8) & 0xFF);
	result.b = (float)((packed >> 16) & 0xFF);
	result.a = (float)((packed >> 24) & 0xFF);
	return(result);
}

inline uint32_t BGRAPack4x8(const Vec4f &unpacked) {
	uint32_t result = (
		(RoundF32ToU8(unpacked.a) << 24) |
		(RoundF32ToU8(unpacked.r) << 16) |
		(RoundF32ToU8(unpacked.g) << 8) |
		(RoundF32ToU8(unpacked.b) << 0));
	return(result);
}
inline Vec4f BGRAUnpack4x8(const uint32_t packed) {
	Vec4f result;
	result.b = (float)((packed >> 0) & 0xFF);
	result.g = (float)((packed >> 8) & 0xFF);
	result.r = (float)((packed >> 16) & 0xFF);
	result.a = (float)((packed >> 24) & 0xFF);
	return(result);
}

inline Pixel PixelPack(const Vec4f &unpacked) {
	Pixel result;
	result.r = RoundF32ToU8(unpacked.r);
	result.g = RoundF32ToU8(unpacked.g);
	result.b = RoundF32ToU8(unpacked.b);
	result.a = RoundF32ToU8(unpacked.a);
	return(result);
}
inline Vec4f PixelUnpack(const Pixel packed) {
	Vec4f result;
	result.r = (float)(packed.r & 0xFF);
	result.g = (float)(packed.g & 0xFF);
	result.b = (float)(packed.b & 0xFF);
	result.a = (float)(packed.a & 0xFF);
	return(result);
}

inline float SRGBToLinear(const float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.04045f)
		return x / 12.92f;
	else
		return Power((x + 0.055f) / 1.055f, 2.4f);
}

inline float LinearToSRGB(const float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.0031308f)
		return x * 12.92f;
	else
		return Power(x, 1.0f / 2.4f) * 1.055f - 0.055f;
}

inline Vec4f PixelToLinear(const Pixel &pixel, const bool fromSRGB = true) {
	Vec4f unpacked = RGBAUnpack4x8(pixel.bgra);
	Vec4f result;
	if (fromSRGB) {
		result = V4f(
			SRGBToLinear(unpacked.r),
			SRGBToLinear(unpacked.g),
			SRGBToLinear(unpacked.b),
			unpacked.a);
	} else {
		result = unpacked;
	}
	return(result);
}

inline Pixel LinearToPixel(const Vec4f &linear, const bool toSRGB = true) {
	float r = toSRGB ? LinearToSRGB(linear.r) : linear.r;
	float g = toSRGB ? LinearToSRGB(linear.g) : linear.g;
	float b = toSRGB ? LinearToSRGB(linear.b) : linear.b;
	float a = linear.a;
	Pixel result;
	result.bgra = RGBAPack4x8(V4f(r, g, b, a));
	return(result);
}

#if 0
inline Vec4f RGBA32ToLinear(const uint32_t rgba) {
	Pixel pixel = RGBA32ToPixel(rgba);
	Vec4f result = PixelToLinear(pixel);
	return(result);
}

inline uint32_t LinearToRGBA32(const Vec4f &linear) {
	Pixel pixel = LinearToPixel(linear);
	uint32_t result = RGBA32(pixel.r, pixel.g, pixel.b, pixel.a);
	return(result);
}

inline Vec4f AlphaToLinear(const uint8_t alpha) {
	float a = alpha * INV255;
	Vec4f result = V4f(1, 1, 1, a);
	return(result);
}
#endif

#endif // FINAL_MATH_H