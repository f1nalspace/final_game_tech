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

#include <final_platform_layer.h>

#include <stdlib.h> // rand, RAND_MAX

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h>
#include <float.h>

const float Pi32 = (float)M_PI;
const float Tau32 = (float)M_PI * 2.0f;
const float Deg2Rad = (float)M_PI / 180.0f;
const float Rad2Deg = 180.0f / (float)M_PI;
const float Epsilon = FLT_EPSILON;
const float InvByte = 1.0f / 255.0f;

//
// Ratio type
//
typedef struct Ratio {
	double numerator;
	double denominator;
} Ratio;

fpl_force_inline Ratio MakeRatio(double numerator, double denominator) {
	Ratio result = fplStructInit(Ratio, numerator, denominator);
	return(result);
}

fpl_force_inline double ComputeRatio(const Ratio ratio) {
	fplAssert(ratio.denominator != 0);
	double result = ratio.numerator / ratio.denominator;
	return(result);
}

//
// Vector types
//

typedef union Vec2i {
	struct {
		int x, y;
	};
	struct {
		int w, h;
	};
	int m[2];
} Vec2i;

fpl_force_inline Vec2i V2iZero() {
	Vec2i result = fplZeroInit;
	return(result);
}

fpl_force_inline Vec2i V2iCopy(const Vec2i v) {
	Vec2i result = fplStructInit(Vec2i, v.x, v.y);
	return(result);
}

fpl_force_inline Vec2i V2iInit(const int x, const int y) {
	Vec2i result = fplStructInit(Vec2i, x, y);
	return(result);
}

fpl_force_inline Vec2i V2iInitScalar(const int value) {
	Vec2i result = fplStructInit(Vec2i, value, value);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec2i V2i() {
	return V2iZero();
}
fpl_force_inline Vec2i V2i(const int value) {
	return V2iInitScalar(value);
}
fpl_force_inline Vec2i V2i(const int x, const int y) {
	return V2iInit(x, y);
}
#endif

typedef union Vec2f {
	struct {
		float x, y;
	};
	struct {
		float w, h;
	};
	float m[2];
} Vec2f;

fpl_force_inline Vec2f V2fZero() {
	Vec2f result = fplZeroInit;
	return(result);
}

fpl_force_inline Vec2f V2fCopy(const Vec2f v) {
	Vec2f result = fplStructInit(Vec2f, v.x, v.y);
	return(result);
}

fpl_force_inline Vec2f V2fInit(const float x, const float y) {
	Vec2f result = fplStructInit(Vec2f, x, y);
	return(result);
}

fpl_force_inline Vec2f V2fInitScalar(const float value) {
	Vec2f result = fplStructInit(Vec2f, value, value);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec2f V2f() {
	return V2fZero();
}
fpl_force_inline Vec2f V2f(const float value) {
	return V2fInitScalar(value);
}
fpl_force_inline Vec2f V2f(const float x, const float y) {
	return V2fInit(x, y);
}
#endif

//
// Rect2f type
//
typedef struct Rect2f {
	Vec2f pos;
	Vec2f size;
} Rect2f;

fpl_force_inline Rect2f R2fInit(const Vec2f pos, const Vec2f size) {
	Rect2f result = fplStructInit(Rect2f, pos, size);
	return(result);
}

typedef union Vec3f {
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
} Vec3f;

fpl_force_inline Vec3f V3fZero() {
	Vec3f result = fplZeroInit;
	return(result);
}

fpl_force_inline Vec3f V3fInitScalar(const float scalar) {
	Vec3f result = fplStructInit(Vec3f, scalar, scalar, scalar);
	return(result);
}

fpl_force_inline Vec3f V3fInitXY(const Vec2f other, const float z) {
	Vec3f result = fplStructInit(Vec3f, other.x, other.y, z);
	return(result);
}

fpl_force_inline Vec3f V3fCopy(const Vec3f other) {
	Vec3f result = fplStructInit(Vec3f, other.x, other.y, other.z);
	return(result);
}

fpl_force_inline Vec3f V3fInit(const float x, const float y, const float z) {
	Vec3f result = fplStructInit(Vec3f, x, y, z);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec3f V3f() {
	return V3fZero();
}
fpl_force_inline Vec3f V3f(const Vec3f &other) {
	return V3fCopy(other);
}
fpl_force_inline Vec3f V3f(const float value) {
	return V3fInitScalar(value);
}
fpl_force_inline Vec3f V3f(const float x, const float y) {
	return V3fInit(x, y, 0.0f);
}
fpl_force_inline Vec3f V3f(const Vec2f &v, const float z) {
	return V3fInitXY(v, z);
}
fpl_force_inline Vec3f V3f(const float x, const float y, const float z) {
	return V3fInit(x, y, z);
}
#endif

typedef union Vec4f {
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
} Vec4f;

fpl_force_inline Vec4f V4fZero() {
	Vec4f result = fplStructInit(Vec4f, 0, 0, 0, 1);
	return(result);
}

fpl_force_inline Vec4f V4fCopy(const Vec4f other) {
	Vec4f result = fplStructInit(Vec4f, other.x, other.y, other.z, other.w);
	return(result);
}

fpl_force_inline Vec4f V4fInit(const float x, const float y, const float z, const float w) {
	Vec4f result = fplStructInit(Vec4f, x, y, z, w);
	return(result);
}

fpl_force_inline Vec4f V4fInitXYZ(const Vec3f v, const float w) {
	Vec4f result = fplStructInit(Vec4f, v.x, v.y, v.z, w);
	return(result);
}

fpl_force_inline Vec4f V4fInitXY(const Vec2f v, const float z, const float w) {
	Vec4f result = fplStructInit(Vec4f, v.x, v.y, z, w);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec4f V4f() {
	return V4fZero();
}
fpl_force_inline Vec4f V4f(const Vec4f &other) {
	return V4fCopy(other);
}
fpl_force_inline Vec4f V4f(const Vec2f &v, const float z, const float w) {
	return V4fInitXY(v, z, w);
}
fpl_force_inline Vec4f V4f(const Vec3f &v, const float w) {
	return V4fInitXYZ(v, w);
}
fpl_force_inline Vec4f V4f(const float x, const float y, const float z) {
	return V4fInit(x, y, z, 1.0f);
}
fpl_force_inline Vec4f V4f(const float x, const float y, const float z, const float w) {
	return V4fInit(x, y, z, w);
}
#endif

typedef union Mat2f {
	struct {
		Vec2f col1;
		Vec2f col2;
	};
	float m[4];
} Mat2f;

fpl_force_inline Mat2f M2fDefault() {
	Mat2f result = fplStructInit(Mat2f, V2fInit(1, 0), V2fInit(0, 1));
	return(result);
}

fpl_force_inline Mat2f M2fCopy(const Mat2f other) {
	Mat2f result = fplStructInit(Mat2f, other.col1, other.col2);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Mat2f M2f() {
	return M2fDefault();
}
fpl_force_inline Mat2f M2f(const Mat2f &other) {
	return M2fCopy(other);
}
#endif

typedef union Mat4f {
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
} Mat4f;

fpl_force_inline Mat4f M4fInit(const float value) {
	Mat4f result = {
		V4fInit(value, 0.0f, 0.0f, 0.0f),
		V4fInit(0.0f, value, 0.0f, 0.0f),
		V4fInit(0.0f, 0.0f, value, 0.0f),
		V4fInit(0.0f, 0.0f, 0.0f, value),
	};
	return(result);
}

fpl_force_inline Mat4f M4fDefault() {
	Mat4f result = M4fInit(1.0f);
	return(result);
}

fpl_force_inline Mat4f M4fCopy(const Mat4f other) {
	Mat4f result = fplStructInit(Mat4f, other.col1, other.col2, other.col3, other.col4);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Mat4f M4f(const float value = 1.0f) {
	return M4fInit(value);
}
fpl_force_inline Mat4f M4f(const Mat4f &other) {
	return M4fCopy(other);
}
#endif

typedef union Pixel {
	struct {
		uint8_t b, g, r, a;
	};
	uint32_t bgra;
	uint8_t m[4];
} Pixel;

//
// Scalar
//

fpl_force_inline float Cosine(const float angle) {
	float result = cosf(angle);
	return(result);
}
fpl_force_inline float Sine(const float angle) {
	float result = sinf(angle);
	return(result);
}
fpl_force_inline float Tan(const float angle) {
	float result = tanf(angle);
	return(result);
}
fpl_force_inline float ArcTan2(const float y, const float x) {
	float result = atan2f(y, x);
	return(result);
}
fpl_force_inline float Abs(const float value) {
	float result = fabsf(value);
	return(result);
}
fpl_force_inline float Power(const float x, const float y) {
	float result = powf(x, y);
	return(result);
}
fpl_force_inline float Min(const float a, const float b) {
	float result = a < b ? a : b;
	return(result);
}
fpl_force_inline float Max(const float a, const float b) {
	float result = a > b ? a : b;
	return(result);
}
fpl_force_inline float SquareRoot(const float value) {
	float result = sqrtf(value);
	return(result);
}
fpl_force_inline float RadiansToDegrees(const float radians) {
	float result = radians * Rad2Deg;
	return(result);
}
fpl_force_inline float DegreesToRadians(const float degrees) {
	float result = degrees * Deg2Rad;
	return(result);
}

fpl_force_inline float ScalarLerp(float a, float t, float b) {
	float result = (1.0f - t) * a + t * b;
	return(result);
}

fpl_force_inline float ScalarAvg(float oldValue, float t, float newValue) {
	float result = t * newValue + (1.0f - t) * oldValue;
	return(result);
}

fpl_force_inline float GetBestAngleDistance(float a0, float a1) {
	float max = Pi32 * 2;
	float da = fmodf(a1 - a0, max);
	float result = fmodf(2.0f * da, max) - da;
	return(result);
}

fpl_force_inline float AngleLerp(float a, float t, float b) {
	float angleDistance = GetBestAngleDistance(a, b);
	float result = ScalarLerp(a, t, a + angleDistance);
	return(result);
}

fpl_force_inline uint8_t RoundF32ToU8(float value) {
	uint8_t result = (uint8_t)(value * 255.0f + 0.5f);
	return(result);
}

fpl_force_inline float RoundU8ToF32(uint8_t value) {
	float result = value * InvByte;
	return(result);
}

//
// Vec2f
//
fpl_force_inline Vec2f V2fMultScalar(const Vec2f v, const float s) {
	Vec2f result = V2fInit(v.x * s, v.y * s);
	return(result);
}

fpl_force_inline Vec2f V2fAddMultScalar(const Vec2f a, const Vec2f b, const float s) {
	Vec2f result = V2fInit(a.x + b.x * s, a.y + b.y * s);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec2f operator*(const Vec2f &v, float s) {
	Vec2f result = V2fMultScalar(v, s);
	return(result);
}

fpl_force_inline Vec2f operator*(float s, const Vec2f &v) {
	Vec2f result = V2fMultScalar(v, s);
	return(result);
}

fpl_force_inline Vec2f &operator*=(Vec2f &v, float s) {
	v = v * s;
	return(v);
}

fpl_force_inline Vec2f operator-(const Vec2f &v) {
	Vec2f result = V2fInit(-v.x, -v.y);
	return(result);
}

fpl_force_inline Vec2f operator+(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2fInit(a.x + b.x, a.y + b.y);
	return(result);
}

fpl_force_inline Vec2f &operator+=(Vec2f &a, const Vec2f &b) {
	a = a + b;
	return(a);
}

fpl_force_inline Vec2f operator-(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2fInit(a.x - b.x, a.y - b.y);
	return(result);
}

fpl_force_inline Vec2f &operator-=(Vec2f &a, const Vec2f &b) {
	a = a - b;
	return(a);
}
#endif // __cplusplus

fpl_force_inline float V2fDot(const Vec2f a, const Vec2f b) {
	float result = a.x * b.x + a.y * b.y;
	return(result);
}

fpl_force_inline float V2fLength(const Vec2f v) {
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return(result);
}

fpl_force_inline Vec2f V2fNormalize(const Vec2f v) {
	float l = V2fLength(v);
	if (l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec2f result = V2fMultScalar(v, invL);
	return(result);
}

fpl_force_inline Vec2f V2fHadamard(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(a.x * b.x, a.y * b.y);
	return(result);
}

fpl_force_inline Vec2f V2fMultMat2(const Mat2f A, const Vec2f v) {
	Vec2f result = V2fInit(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
	return(result);
}

fpl_force_inline float V2fDistanceSquared(const Vec2f a, const Vec2f b) {
	float f = (b.x - a.x) * (b.y - a.y);
	float result = f * f;
	return(result);
}

/* Returns the right perpendicular vector */
fpl_force_inline Vec2f V2fCrossR(const Vec2f a, float s) {
	return V2fInit(s * a.y, -s * a.x);
}

/* Returns the left perpendicular vector */
fpl_force_inline Vec2f V2fCrossL(float s, const Vec2f a) {
	return V2fInit(-s * a.y, s * a.x);
}

/* Returns the Z-rotation from two vectors */
fpl_force_inline float V2fCrossZ(const Vec2f a, const Vec2f b) {
	return a.x * b.y - a.y * b.x;
}

fpl_force_inline float V2fAngleFromAxis(const Vec2f axis) {
	float result = ArcTan2(axis.y, axis.x);
	return(result);
}

fpl_force_inline Vec2f V2fAxisFromAngle(const float angle) {
	Vec2f result = V2fInit(Cosine(angle), Sine(angle));
	return(result);
}

fpl_force_inline Vec2f V2fRandomDirection() {
	float d = rand() / (float)RAND_MAX;
	float angle = d * ((float)M_PI * 2.0f);
	Vec2f result = V2fInit(Cosine(angle), Sine(angle));
	return(result);
}

fpl_force_inline Vec2f V2fLerp(const Vec2f a, const float t, const Vec2f b) {
	Vec2f result;
	result.x = ScalarLerp(a.x, t, b.x);
	result.y = ScalarLerp(a.y, t, b.y);
	return(result);
}

fpl_force_inline Vec2f V2fMin(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(Min(a.x, b.x), Min(a.y, b.y));
	return(result);
}

fpl_force_inline Vec2f V2fMax(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(Max(a.x, b.x), Max(a.y, b.y));
	return(result);
}

//
// Vec2i
//
fpl_force_inline bool V2iEquals(const Vec2i a, const Vec2i b) {
	bool result = a.x == b.x && a.y == b.y;
	return(result);
}

//
// Vec3f
//
fpl_force_inline Vec3f V3fMultScalar(const Vec3f v, const float s) {
	Vec3f result = V3fInit(v.x * s, v.y * s, v.z * s);
	return(result);
}

fpl_force_inline Vec3f V3fSub(const Vec3f a, const Vec3f b) {
	Vec3f result = V3fInit(a.x - b.x, a.y - b.y, a.z - b.z);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Vec3f operator*(float s, const Vec3f &v) {
	Vec3f result = V3fMultScalar(v, s);
	return(result);
}

fpl_force_inline Vec3f operator*(const Vec3f &v, float s) {
	Vec3f result = V3fMultScalar(v, s);
	return(result);
}

fpl_force_inline Vec3f &operator*=(Vec3f &v, float s) {
	v = s * v;
	return(v);
}

fpl_force_inline Vec3f operator+(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3fInit(a.x + b.x, a.y + b.y, a.z + b.z);
	return(result);
}

fpl_force_inline Vec3f &operator+=(Vec3f &a, const Vec3f &b) {
	a = a + b;
	return(a);
}

fpl_force_inline Vec3f operator-(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3fInit(a.x - b.x, a.y - b.y, a.z - b.z);
	return(result);
}

fpl_force_inline Vec3f operator-(const Vec3f &v) {
	Vec3f result = V3fInit(-v.x, -v.y, -v.z);
	return(result);
}

fpl_force_inline Vec3f &operator-=(Vec3f &a, const Vec3f &b) {
	a = a - b;
	return(a);
}
#endif // __cplusplus

fpl_force_inline float V3fDot(const Vec3f a, const Vec3f b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return(result);
}

fpl_force_inline float V3fDistanceSquared(const Vec3f a, const Vec3f b) {
	float f = (b.x - a.x) * (b.y - a.y) * (b.z - a.z);
	float result = f * f;
	return(result);
}

fpl_force_inline float V3fLength(const Vec3f v) {
	float result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return(result);
}

fpl_force_inline Vec3f V3fNormalize(const Vec3f v) {
	float l = V3fLength(v);
	if (l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec3f result = V3fMultScalar(v, invL);
	return(result);
}

fpl_force_inline Vec3f V3fCross(const Vec3f a, const Vec3f b) {
	Vec3f result = V3fInit(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
	return(result);
}

fpl_force_inline Vec3f V3fLerp(const Vec3f a, float t, const Vec3f b) {
	Vec3f result;
	result.x = ScalarLerp(a.x, t, b.x);
	result.y = ScalarLerp(a.y, t, b.y);
	result.z = ScalarLerp(a.z, t, b.z);
	return(result);
}

fpl_force_inline Vec3f V3fHadamard(const Vec3f a, const Vec3f b) {
	Vec3f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return(result);
}

//
// Mat2f
//
fpl_force_inline Mat2f Mat2FromAngle(float angle) {
	float s = Sine(angle);
	float c = Cosine(angle);
	Mat2f result;
	result.col1 = V2fInit(c, s);
	result.col2 = V2fInit(-s, c);
	return(result);
}

fpl_force_inline Mat2f Mat2FromAxis(const Vec2f axis) {
	Mat2f result;
	result.col1 = axis;
	result.col2 = V2fCrossL(1.0f, axis);
	return(result);
}

fpl_force_inline Mat2f Mat2Transpose(const Mat2f m) {
	Mat2f result;
	result.col1 = V2fInit(m.col1.x, m.col2.x);
	result.col2 = V2fInit(m.col1.y, m.col2.y);
	return(result);
}

fpl_force_inline Mat2f Mat2Mult(const Mat2f a, const Mat2f b) {
	Mat2f result;
	result.col1 = V2fMultMat2(a, b.col1);
	result.col2 = V2fMultMat2(a, b.col2);
	return(result);
}

fpl_force_inline float Mat2ToAngle(const Mat2f mat) {
	float result = V2fAngleFromAxis(mat.col1);
	return(result);
}

/* Generates a 2x2 matrix for doing B to A conversion */
fpl_force_inline Mat2f Mat2MultTranspose(const Mat2f a, const Mat2f b) {
	Mat2f result;
	result.col1 = V2fInit(V2fDot(a.col1, b.col1), V2fDot(a.col2, b.col1));
	result.col2 = V2fInit(V2fDot(a.col1, b.col2), V2fDot(a.col2, b.col2));
	return(result);
}

//
// Mat4f
//
fpl_force_inline static Mat4f Mat4OrthoRH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	Mat4f result = M4fInit(1.0f);
	result.r[0][0] = 2.0f / (right - left);
	result.r[1][1] = 2.0f / (top - bottom);
	result.r[2][2] = -2.0f / (zFar - zNear);
	result.r[3][0] = -(right + left) / (right - left);
	result.r[3][1] = -(top + bottom) / (top - bottom);
	result.r[3][2] = -(zFar + zNear) / (zFar - zNear);
	return (result);
}

fpl_force_inline static Mat4f Mat4PerspectiveRH(const float fov, const float aspect, const float zNear, const float zFar) {
	float tanHalfFov = Tan(fov * 0.5f);
	Mat4f result = M4fInit(0.0f);
	result.r[0][0] = 1.0f / (aspect * tanHalfFov);
	result.r[1][1] = 1.0f / (tanHalfFov);
	result.r[2][2] = -(zFar + zNear) / (zFar - zNear);
	result.r[2][3] = -1.0f;
	result.r[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	return (result);
}

fpl_force_inline static Mat4f Mat4LookAtRH(const Vec3f eye, const Vec3f center, const Vec3f up) {
	// Forward/Side/Upward
	const Vec3f f = V3fNormalize(V3fSub(center, eye));
	const Vec3f s = V3fNormalize(V3fCross(f, up));
	const Vec3f u = V3fCross(s, f);

	Mat4f result = M4fInit(1.0f);

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
	result.r[3][0] = -V3fDot(s, eye);
	result.r[3][1] = -V3fDot(u, eye);
	result.r[3][2] = V3fDot(f, eye);

	return (result);
}

fpl_force_inline static Mat4f Mat4TranslationV2(const Vec2f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4.xy = p;
	result.col4.z = 0.0f;
	result.col4.w = 1.0f;
	return (result);
}

fpl_force_inline static Mat4f Mat4TranslationV3(const Vec3f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4.xyz = p;
	result.col4.w = 1.0f;
	return (result);
}

fpl_force_inline static Mat4f Mat4TranslationV4(const Vec4f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4 = p;
	return (result);
}

fpl_force_inline static Mat4f Mat4ScaleV2(const Vec2f s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = 0.0f;
	return (result);
}

fpl_force_inline static Mat4f Mat4ScaleV3(const Vec3f s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	return (result);
}

fpl_force_inline static Mat4f Mat4ScaleV4(const Vec4f s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	result.col3.w = s.w;
	return (result);
}

fpl_force_inline static Mat4f Mat4RotationX(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4fInit(1.0f, 0.0f, 0.0f, 0.0f);
	result.col2 = V4fInit(0.0f, c, s, 0.0f);
	result.col3 = V4fInit(0.0f, -s, c, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f Mat4RotationY(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4fInit(c, 0.0f, s, 0.0f);
	result.col2 = V4fInit(0.0f, 1.0f, 0.0f, 0.0f);
	result.col3 = V4fInit(-s, 0.0f, c, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f Mat4RotationZFromAngle(const float angle) {
	float c = Cosine(angle);
	float s = Sine(angle);
	Mat4f result;
	result.col1 = V4fInit(c, s, 0.0f, 0.0f);
	result.col2 = V4fInit(-s, c, 0.0f, 0.0f);
	result.col3 = V4fInit(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f Mat4RotationZFromM2f(const Mat2f m) {
	Mat4f result;
	result.col1 = V4fInit(m.col1.x, m.col1.y, 0.0f, 0.0f);
	result.col2 = V4fInit(-m.col1.y, m.col1.x, 0.0f, 0.0f);
	result.col3 = V4fInit(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline Mat4f Mat4Mult(const Mat4f a, const Mat4f b) {
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

#if defined(__cplusplus)
fpl_force_inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	Mat4f result = Mat4Mult(a, b);
	return(result);
}
#endif // __cplusplus

fpl_force_inline Vec4f Vec4MultMat4(const Mat4f mat, const Vec4f v) {
	Vec4f result;
	result.x = mat.r[0][0] * v.m[0] + mat.r[0][1] * v.m[1] + mat.r[0][2] * v.m[2] + mat.r[0][3] * v.m[3];
	result.y = mat.r[1][0] * v.m[0] + mat.r[1][1] * v.m[1] + mat.r[1][2] * v.m[2] + mat.r[1][3] * v.m[3];
	result.z = mat.r[2][0] * v.m[0] + mat.r[2][1] * v.m[1] + mat.r[2][2] * v.m[2] + mat.r[2][3] * v.m[3];
	result.w = mat.r[3][0] * v.m[0] + mat.r[3][1] * v.m[1] + mat.r[3][2] * v.m[2] + mat.r[3][3] * v.m[3];
	return(result);
}

//
// Pixel
//
#if 0
static const Vec4f ColorWhite = V4fInit(.0f, 1.0f, 1.0f, 1.0f);
static const Vec4f ColorRed = V4fInit(1.0f, 0.0f, 0.0f, 1.0f);
static const Vec4f ColorGreen = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
static const Vec4f ColorBlue = V4fInit(0.0f, 0.0f, 1.0f, 1.0f);
static const Vec4f ColorLightGray = V4fInit(0.3f, 0.3f, 0.3f, 1.0f);
static const Vec4f ColorDarkGray = V4fInit(0.2f, 0.2f, 0.2f, 1.0f);
#endif

fpl_force_inline Pixel MakePixelFromRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Pixel result;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return(result);
}
fpl_force_inline Pixel MakePixelFromU32(const uint32_t rgba) {
	Pixel result;
	result.r = (uint8_t)((rgba >> 0) & 0xFF);
	result.g = (uint8_t)((rgba >> 8) & 0xFF);
	result.b = (uint8_t)((rgba >> 16) & 0xFF);
	result.a = (uint8_t)((rgba >> 24) & 0xFF);
	return(result);
}

fpl_force_inline uint32_t RGBA8FromRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r << 0);
	return(result);
}
fpl_force_inline uint32_t RGBA8FromPixel(const Pixel pixel) {
	uint32_t result = RGBA8FromRGBA(pixel.r, pixel.g, pixel.b, pixel.a);
	return(result);
}

fpl_force_inline uint32_t BGRA8FromRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	uint32_t result = (a << 24) | (r << 16) | (g << 8) | (b << 0);
	return(result);
}
fpl_force_inline uint32_t BGRA8FromPixel(const Pixel pixel) {
	uint32_t result = pixel.bgra;
	return(result);
}

fpl_force_inline uint32_t BGRAPack4x8(const Vec4f unpacked) {
	uint32_t result = (
		(RoundF32ToU8(unpacked.a) << 24) |
		(RoundF32ToU8(unpacked.r) << 16) |
		(RoundF32ToU8(unpacked.g) << 8) |
		(RoundF32ToU8(unpacked.b) << 0));
	return(result);
}

fpl_force_inline Vec4f BGRAUnpack4x8(const uint32_t packed) {
	Vec4f result;
	result.b = RoundU8ToF32((packed >> 0) & 0xFF);
	result.g = RoundU8ToF32((packed >> 8) & 0xFF);
	result.r = RoundU8ToF32((packed >> 16) & 0xFF);
	result.a = RoundU8ToF32((packed >> 24) & 0xFF);
	return(result);
}

fpl_force_inline Pixel PixelPack(const Vec4f unpacked) {
	Pixel result;
	result.r = RoundF32ToU8(unpacked.r);
	result.g = RoundF32ToU8(unpacked.g);
	result.b = RoundF32ToU8(unpacked.b);
	result.a = RoundF32ToU8(unpacked.a);
	return(result);
}

fpl_force_inline Vec4f PixelUnpack(const Pixel packed) {
	Vec4f result;
	result.r = RoundU8ToF32(packed.r & 0xFF);
	result.g = RoundU8ToF32(packed.g & 0xFF);
	result.b = RoundU8ToF32(packed.b & 0xFF);
	result.a = RoundU8ToF32(packed.a & 0xFF);
	return(result);
}

fpl_force_inline float SRGBToLinear(const float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.04045f)
		return x / 12.92f;
	else
		return Power((x + 0.055f) / 1.055f, 2.4f);
}

fpl_force_inline float LinearToSRGB(const float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.0031308f)
		return x * 12.92f;
	else
		return Power(x, 1.0f / 2.4f) * 1.055f - 0.055f;
}

fpl_force_inline Vec4f PixelToLinearRaw(const Pixel pixel) {
	Vec4f result = BGRAUnpack4x8(pixel.bgra);
	return(result);
}

fpl_force_inline Vec4f PixelToLinearSRGB(const Pixel pixel) {
	Vec4f unpacked = BGRAUnpack4x8(pixel.bgra);
	Vec4f result =
		V4fInit(
			SRGBToLinear(unpacked.r),
			SRGBToLinear(unpacked.g),
			SRGBToLinear(unpacked.b),
			unpacked.a);
	return(result);
}

fpl_force_inline Vec4f RGBAToLinearRaw(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Pixel pixel = MakePixelFromRGBA(r, g, b, a);
	Vec4f result = PixelToLinearRaw(pixel);
	return(result);
}

fpl_force_inline Vec4f RGBAToLinearSRGB(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	Pixel pixel = MakePixelFromRGBA(r, g, b, a);
	Vec4f result = PixelToLinearSRGB(pixel);
	return(result);
}

fpl_force_inline Pixel LinearToPixelRaw(const Vec4f linear) {
	float r = linear.r;
	float g = linear.g;
	float b = linear.b;
	float a = linear.a;
	Pixel result;
	result.bgra = BGRAPack4x8(V4fInit(r, g, b, a));
	return(result);
}

fpl_force_inline Pixel LinearToPixelSRGB(const Vec4f linear) {
	float r = LinearToSRGB(linear.r);
	float g = LinearToSRGB(linear.g);
	float b = LinearToSRGB(linear.b);
	float a = linear.a;
	Pixel result;
	result.bgra = BGRAPack4x8(V4fInit(r, g, b, a));
	return(result);
}

#endif // FINAL_MATH_H