/*
Name:
	Final Math

Description:
	Math Library for defining/computing 2D/3D/4D Vectors, 2x2, 3x3, 4x4 Matrices, etc.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog
	## 2026-06-02
	- Standalone math constants (no float.h, no math.h constants); math.h kept only for functions
	- Fixed V2fDistanceSquared() / V3fDistanceSquared() was not implemented correctly
	- Fixed V4fZero() was not initializing all components to zero
	- Fixed M4fRotationY() had the sine sign swapped (was the transposed rotation)
	- Fixed M4fInverse() was not transposing the resulting matrix
	- Fixed V4fMultM4f() was not computing in column-major order
	- Fixed QuatToMat4() was not computing in column-major order
	- Fixed V2iProject() was not computing NDC correctly
	- Added M4fTranspose() that transposes a Mat4f
	- Added struct Mat3f
	- Added functions M3fInit, M3fIdentity, M3fCopy and M3f() C++ variants
	- Added functions M3fFromMat4, V3fMultM3f, M3fMult, M3fTranspose, M3fTranslationV2, M3fScaleV2, M3fRotationZ
	- Added functions F32NaN() and F32Infinity()
	- Removed usage of _USE_MATH_DEFINES

	## 2025-12-31
	- Added struct Vec2u
	- Added function V2iProject() that reverses V2fUnproject()
	- Changed V4fDivideScalar to use a ratio multiplier instead of using division directly

	## 2025-11-29
	- Added macros for creating types as a constant

	## 2025-11-24
	- Added function V2fClamp()

	## 2025-11-21
	- Added function V2iAdd()
	- Added function V2iSub()
	- Added function V2iMult()
	- Added function V2iDiv()
	- Renamed function F32ScalarLerp to F32Lerp

	## 2025-11-16
	- Added function V2fInitV2i()
	- Added function V2fAbs()
	- Added function V2fPerp()
	- Added function V2fMajorAxis()
	- Added function F32IsNaN()
	- Added function F32Sign()
	- Fixed some structs not initialized

	## 2025-03-09
	- More Vec2f functions (Add, Sub)

	## 2020-05-15:
	- Added optional c++ methods with overloads, to make life a bit easier
	- Added experimental quaternion struct

	## 2020-05-09:
	- Changed to be 100% C99 complaint
	- Added optional c++ overator over, to make life a bit easier

	## 2019-05-10:
	- Added Vec3f math operator overloaded functions
	- Renamed Mat4OrthoLH to M4fOrthoRH
	- Added M4fPerspectiveRH
	- Added M4fLookAtRH
	- Added more overrides for M4fTranslation
	- Added SRGBToLinear, LinearToSRGB
	- Added default SRGB conversion to Linear <-> Pixel

	## 2018-07-05:
	- Added M4fRotation, M4fRotationY, M4fRotationZ
*/

#ifndef FINAL_MATH_H
#define FINAL_MATH_H

#include <final_platform_layer.h>

#include <stdlib.h> // rand, RAND_MAX

// Only math functions (sinf, cosf, sqrtf, ...) are taken from math.h.
// All constants are defined ourselves below, so no math.h/float.h constants
// and no need for _USE_MATH_DEFINES.
#include <math.h>

//! Largest finite 32-bit float (FLT_MAX).
const float F32MaxValue = 3.402823466e+38f;
//! Smallest normal positive 32-bit float (FLT_MIN).
const float F32MinValue = 1.175494351e-38f;
//! Ratio of a circle's circumference to its diameter (Pi).
const float F32Pi = 3.14159265358979323846f;
//! Full turn in radians (2*Pi).
const float F32Tau = 6.28318530717958647692f;
//! Factor to convert degrees to radians.
const float F32Deg2Rad = 3.14159265358979323846f / 180.0f;
//! Factor to convert radians to degrees.
const float F32Rad2Deg = 180.0f / 3.14159265358979323846f;
//! Smallest difference between two representable floats (FLT_EPSILON).
const float F32Epsilon = 1.192092896e-07f;
//! Reciprocal of 255, to map a byte 0..255 into 0..1.
const float F32InvByte = 1.0f / 255.0f;

//
// Ratio64 type
//

/**
* @struct Ratio64
* @brief A double-precision fraction (numerator over denominator).
*/
typedef struct Ratio64 {
	//! The dividend of the ratio.
	double numerator;
	//! The divisor of the ratio.
	double denominator;
} Ratio64;

/**
* @brief Creates a Ratio64 from a numerator and denominator.
* @param[in] numerator The dividend.
* @param[in] denominator The divisor.
* @return The initialized Ratio64.
*/
fpl_force_inline Ratio64 Ratio64Init(double numerator, double denominator) {
	Ratio64 result = fplStructInit(Ratio64, numerator, denominator);
	return(result);
}

/**
* @brief Computes the value of the ratio (numerator / denominator).
* @param[in] ratio The ratio to evaluate.
* @return The quotient as a double. Asserts when the denominator is zero.
*/
fpl_force_inline double Ratio64Compute(const Ratio64 ratio) {
	fplAssert(ratio.denominator != 0);
	double result = ratio.numerator / ratio.denominator;
	return(result);
}

//
// Vector types
//

/**
* @union Vec2i
* @brief A 2D vector of signed integers, accessible as x/y, w/h or by index.
*/
typedef union Vec2i {
	struct {
		//! The first component (x or width).
		int x;
		//! The second component (y or height).
		int y;
	};
	struct {
		int w, h;
	};
	//! The components as an indexable array.
	int m[2];
} Vec2i;

//! Brace-initializer list for a Vec2i literal.
#define V2I(x, y) {x, y}
//! Casts a brace-initializer list to a Vec2i value.
#define V2IArg(v) (Vec2i)v

/**
* @brief Creates a zero Vec2i.
* @return A Vec2i with both components set to zero.
*/
fpl_force_inline Vec2i V2iZero() {
	Vec2i result = fplZeroInit;
	return(result);
}

/**
* @brief Creates a copy of a Vec2i.
* @param[in] v The vector to copy.
* @return A copy of the input vector.
*/
fpl_force_inline Vec2i V2iCopy(const Vec2i v) {
	Vec2i result = fplStructInit(Vec2i, v.x, v.y);
	return(result);
}

/**
* @brief Creates a Vec2i from two components.
* @param[in] x The x component.
* @param[in] y The y component.
* @return The initialized vector.
*/
fpl_force_inline Vec2i V2iInit(const int x, const int y) {
	Vec2i result = fplStructInit(Vec2i, x, y);
	return(result);
}

/**
* @brief Creates a Vec2i with both components set to the same scalar.
* @param[in] value The value for both components.
* @return The initialized vector.
*/
fpl_force_inline Vec2i V2iInitScalar(const int value) {
	Vec2i result = fplStructInit(Vec2i, value, value);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a zero Vec2i.
fpl_force_inline Vec2i V2i() {
	return V2iZero();
}
//! C++ overload constructing a Vec2i from a single scalar.
fpl_force_inline Vec2i V2i(const int value) {
	return V2iInitScalar(value);
}
//! C++ overload constructing a Vec2i from two components.
fpl_force_inline Vec2i V2i(const int x, const int y) {
	return V2iInit(x, y);
}
#endif

/**
* @union Vec2u
* @brief A 2D vector of unsigned integers, accessible as x/y, w/h or by index.
*/
typedef union Vec2u {
	struct {
		//! The first component (x or width).
		uint32_t x;
		//! The second component (y or height).
		uint32_t y;
	};
	struct {
		uint32_t w, h;
	};
	//! The components as an indexable array.
	int m[2];
} Vec2u;

//! Brace-initializer list for a Vec2u literal.
#define V2U(x, y) {x, y}
//! Casts a brace-initializer list to a Vec2u value.
#define V2UArg(v) (Vec2u)v

/**
* @brief Creates a zero Vec2u.
* @return A Vec2u with both components set to zero.
*/
fpl_force_inline Vec2u V2uZero() {
	Vec2u result = fplZeroInit;
	return(result);
}

/**
* @brief Creates a copy of a Vec2u.
* @param[in] v The vector to copy.
* @return A copy of the input vector.
*/
fpl_force_inline Vec2u V2uCopy(const Vec2u v) {
	Vec2u result = fplStructInit(Vec2u, v.x, v.y);
	return(result);
}

/**
* @brief Creates a Vec2u from two components.
* @param[in] x The x component.
* @param[in] y The y component.
* @return The initialized vector.
*/
fpl_force_inline Vec2u V2uInit(const uint32_t x, const uint32_t y) {
	Vec2u result = fplStructInit(Vec2u, x, y);
	return(result);
}

/**
* @brief Creates a Vec2u with both components set to the same scalar.
* @param[in] value The value for both components.
* @return The initialized vector.
*/
fpl_force_inline Vec2u V2uInitScalar(const uint32_t value) {
	Vec2u result = fplStructInit(Vec2u, value, value);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a zero Vec2u.
fpl_force_inline Vec2u V2u() {
	return V2uZero();
}
//! C++ overload constructing a Vec2u from a single scalar.
fpl_force_inline Vec2u V2u(const uint32_t value) {
	return V2uInitScalar(value);
}
//! C++ overload constructing a Vec2u from two components.
fpl_force_inline Vec2u V2u(const uint32_t x, const uint32_t y) {
	return V2uInit(x, y);
}
#endif

/**
* @union Vec2f
* @brief A 2D vector of 32-bit floats, accessible as x/y, w/h or by index.
*/
typedef union Vec2f {
	struct {
		//! The first component (x or width).
		float x;
		//! The second component (y or height).
		float y;
	};
	struct {
		float w, h;
	};
	//! The components as an indexable array.
	float m[2];
} Vec2f;

//! Brace-initializer list for a Vec2f literal.
#define V2F(x, y) {x, y}
//! Casts a brace-initializer list to a Vec2f value.
#define V2FArg(v) (Vec2f)v

/**
* @brief Creates a zero Vec2f.
* @return A Vec2f with both components set to zero.
*/
fpl_force_inline Vec2f V2fZero() {
	Vec2f result = fplZeroInit;
	return(result);
}

/**
* @brief Creates a copy of a Vec2f.
* @param[in] v The vector to copy.
* @return A copy of the input vector.
*/
fpl_force_inline Vec2f V2fCopy(const Vec2f v) {
	Vec2f result = fplStructInit(Vec2f, v.x, v.y);
	return(result);
}

/**
* @brief Creates a Vec2f from two components.
* @param[in] x The x component.
* @param[in] y The y component.
* @return The initialized vector.
*/
fpl_force_inline Vec2f V2fInit(const float x, const float y) {
	Vec2f result = fplStructInit(Vec2f, x, y);
	return(result);
}

/**
* @brief Creates a Vec2f with both components set to the same scalar.
* @param[in] value The value for both components.
* @return The initialized vector.
*/
fpl_force_inline Vec2f V2fInitScalar(const float value) {
	Vec2f result = fplStructInit(Vec2f, value, value);
	return(result);
}

/**
* @brief Creates a Vec2f from a Vec2i, converting each component to float.
* @param[in] v The integer vector to convert.
* @return The converted float vector.
*/
fpl_force_inline Vec2f V2fInitV2i(const Vec2i v) {
	Vec2f result = fplStructInit(Vec2f, (float)v.x, (float)v.y);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a zero Vec2f.
fpl_force_inline Vec2f V2f() {
	return V2fZero();
}
//! C++ overload constructing a Vec2f from a single scalar.
fpl_force_inline Vec2f V2f(const float value) {
	return V2fInitScalar(value);
}
//! C++ overload constructing a Vec2f from two components.
fpl_force_inline Vec2f V2f(const float x, const float y) {
	return V2fInit(x, y);
}
#endif

/**
* @brief Creates a Vec2f from a Vec2u, converting each component to float.
* @param[in] v The unsigned integer vector to convert.
* @return The converted float vector.
*/
fpl_force_inline Vec2f V2fInitV2u(const Vec2u v) {
	Vec2f result = fplStructInit(Vec2f, (float)v.x, (float)v.y);
	return(result);
}

//
// Rect2f type
//

/**
* @struct Rect2f
* @brief An axis-aligned 2D rectangle defined by a position and a size.
*/
typedef struct Rect2f {
	//! The top-left position of the rectangle.
	Vec2f pos;
	//! The width and height of the rectangle.
	Vec2f size;
} Rect2f;

//! Brace-initializer list for a Rect2f literal.
#define R2F(p, s) {p, s}
//! Casts a brace-initializer list to a Rect2f value.
#define R2FArg(v) (Rect2f)v

/**
* @brief Creates a Rect2f from a position and a size.
* @param[in] pos The top-left position.
* @param[in] size The width and height.
* @return The initialized rectangle.
*/
fpl_force_inline Rect2f R2fInit(const Vec2f pos, const Vec2f size) {
	Rect2f result = fplStructInit(Rect2f, pos, size);
	return(result);
}

/**
* @union Vec3f
* @brief A 3D vector of 32-bit floats, accessible as x/y/z, u/v/w, r/g/b, 2D swizzles or by index.
*/
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

//! Brace-initializer list for a Vec3f literal.
#define V3F(x, y, z) {x, y, z}
//! Casts a brace-initializer list to a Vec3f value.
#define V3FArg(v) (Vec3f)v

/**
* @brief Creates a zero Vec3f.
* @return A Vec3f with all components set to zero.
*/
fpl_force_inline Vec3f V3fZero() {
	Vec3f result = fplZeroInit;
	return(result);
}

/**
* @brief Creates a Vec3f with all components set to the same scalar.
* @param[in] scalar The value for all three components.
* @return The initialized vector.
*/
fpl_force_inline Vec3f V3fInitScalar(const float scalar) {
	Vec3f result = fplStructInit(Vec3f, scalar, scalar, scalar);
	return(result);
}

/**
* @brief Creates a Vec3f from a Vec2f and a z component.
* @param[in] other The x/y components.
* @param[in] z The z component.
* @return The initialized vector.
*/
fpl_force_inline Vec3f V3fInitXY(const Vec2f other, const float z) {
	Vec3f result = fplStructInit(Vec3f, other.x, other.y, z);
	return(result);
}

/**
* @brief Creates a copy of a Vec3f.
* @param[in] other The vector to copy.
* @return A copy of the input vector.
*/
fpl_force_inline Vec3f V3fCopy(const Vec3f other) {
	Vec3f result = fplStructInit(Vec3f, other.x, other.y, other.z);
	return(result);
}

/**
* @brief Creates a Vec3f from three components.
* @param[in] x The x component.
* @param[in] y The y component.
* @param[in] z The z component.
* @return The initialized vector.
*/
fpl_force_inline Vec3f V3fInit(const float x, const float y, const float z) {
	Vec3f result = fplStructInit(Vec3f, x, y, z);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a zero Vec3f.
fpl_force_inline Vec3f V3f() {
	return V3fZero();
}
//! C++ overload constructing a Vec3f from another Vec3f.
fpl_force_inline Vec3f V3f(const Vec3f &other) {
	return V3fCopy(other);
}
//! C++ overload constructing a Vec3f from a single scalar.
fpl_force_inline Vec3f V3f(const float value) {
	return V3fInitScalar(value);
}
//! C++ overload constructing a Vec3f from x/y with z set to zero.
fpl_force_inline Vec3f V3f(const float x, const float y) {
	return V3fInit(x, y, 0.0f);
}
//! C++ overload constructing a Vec3f from a Vec2f and a z component.
fpl_force_inline Vec3f V3f(const Vec2f &v, const float z) {
	return V3fInitXY(v, z);
}
//! C++ overload constructing a Vec3f from three components.
fpl_force_inline Vec3f V3f(const float x, const float y, const float z) {
	return V3fInit(x, y, z);
}
#endif

/**
* @union Vec4f
* @brief A 4D vector of 32-bit floats, accessible as x/y/z/w, r/g/b/a, sub-vectors or by index.
*/
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

//! Brace-initializer list for a Vec4f literal.
#define V4F(x, y, z, w) {x, y, z, w}
//! Casts a brace-initializer list to a Vec4f value.
#define V4FArg(v) (Vec4f)v

/**
* @brief Creates a zero Vec4f.
* @return A Vec4f with all components set to zero.
*/
fpl_force_inline Vec4f V4fZero() {
	Vec4f result = fplStructInit(Vec4f, 0, 0, 0, 0);
	return(result);
}

/**
* @brief Creates a copy of a Vec4f.
* @param[in] other The vector to copy.
* @return A copy of the input vector.
*/
fpl_force_inline Vec4f V4fCopy(const Vec4f other) {
	Vec4f result = fplStructInit(Vec4f, other.x, other.y, other.z, other.w);
	return(result);
}

/**
* @brief Creates a Vec4f from four components.
* @param[in] x The x component.
* @param[in] y The y component.
* @param[in] z The z component.
* @param[in] w The w component.
* @return The initialized vector.
*/
fpl_force_inline Vec4f V4fInit(const float x, const float y, const float z, const float w) {
	Vec4f result = fplStructInit(Vec4f, x, y, z, w);
	return(result);
}

/**
* @brief Creates a Vec4f from a Vec3f and a w component.
* @param[in] v The x/y/z components.
* @param[in] w The w component.
* @return The initialized vector.
*/
fpl_force_inline Vec4f V4fInitXYZ(const Vec3f v, const float w) {
	Vec4f result = fplStructInit(Vec4f, v.x, v.y, v.z, w);
	return(result);
}

/**
* @brief Creates a Vec4f from a Vec2f plus z and w components.
* @param[in] v The x/y components.
* @param[in] z The z component.
* @param[in] w The w component.
* @return The initialized vector.
*/
fpl_force_inline Vec4f V4fInitXY(const Vec2f v, const float z, const float w) {
	Vec4f result = fplStructInit(Vec4f, v.x, v.y, z, w);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a zero Vec4f.
fpl_force_inline Vec4f V4f() {
	return V4fZero();
}
//! C++ overload constructing a Vec4f from another Vec4f.
fpl_force_inline Vec4f V4f(const Vec4f &other) {
	return V4fCopy(other);
}
//! C++ overload constructing a Vec4f from a Vec2f plus z and w.
fpl_force_inline Vec4f V4f(const Vec2f &v, const float z, const float w) {
	return V4fInitXY(v, z, w);
}
//! C++ overload constructing a Vec4f from a Vec3f and a w component.
fpl_force_inline Vec4f V4f(const Vec3f &v, const float w) {
	return V4fInitXYZ(v, w);
}
//! C++ overload constructing a Vec4f from x/y/z with w set to one.
fpl_force_inline Vec4f V4f(const float x, const float y, const float z) {
	return V4fInit(x, y, z, 1.0f);
}
//! C++ overload constructing a Vec4f from four components.
fpl_force_inline Vec4f V4f(const float x, const float y, const float z, const float w) {
	return V4fInit(x, y, z, w);
}
#endif

/**
* @union Mat2f
* @brief A 2x2 column-major matrix of 32-bit floats, accessible by column or by index.
*/
typedef union Mat2f {
	struct {
		//! The first column.
		Vec2f col1;
		//! The second column.
		Vec2f col2;
	};
	//! The components as an indexable array (column-major).
	float m[4];
} Mat2f;

//! Brace-initializer list for a Mat2f literal from two columns.
#define M2F(c1, c2) {c1, c2}
//! Casts a brace-initializer list to a Mat2f value.
#define M2FArg(m) (Mat2f)m

/**
* @brief Creates a 2x2 identity matrix.
* @return The identity matrix.
*/
fpl_force_inline Mat2f M2fDefault() {
	Mat2f result = fplStructInit(Mat2f, V2fInit(1, 0), V2fInit(0, 1));
	return(result);
}

/**
* @brief Creates a copy of a Mat2f.
* @param[in] other The matrix to copy.
* @return A copy of the input matrix.
*/
fpl_force_inline Mat2f M2fCopy(const Mat2f other) {
	Mat2f result = fplStructInit(Mat2f, other.col1, other.col2);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a 2x2 identity matrix.
fpl_force_inline Mat2f M2f() {
	return M2fDefault();
}
//! C++ overload constructing a Mat2f from another Mat2f.
fpl_force_inline Mat2f M2f(const Mat2f &other) {
	return M2fCopy(other);
}
#endif

/**
* @union Mat4f
* @brief A 4x4 column-major matrix of 32-bit floats, accessible by column, as r[col][row] or by index.
* @note Storage is column-major (GL-compatible): r[i] is column i and the transform is M * v.
*/
typedef union Mat4f {
	struct {
		//! The first column.
		Vec4f col1;
		//! The second column.
		Vec4f col2;
		//! The third column.
		Vec4f col3;
		//! The fourth column (typically the translation).
		Vec4f col4;
	};
	struct {
		//! The components as r[col][row] (column-major).
		float r[4][4];
	};
	//! The components as an indexable array (column-major).
	float m[16];
} Mat4f;

//! Brace-initializer list for a Mat4f literal from four columns.
#define M4F(c1, c2, c3, c4) {c1, c2, c3, c4}
//! Casts a brace-initializer list to a Mat4f value.
#define M4FArg(m) (Mat4f)m

/**
* @brief Creates a diagonal 4x4 matrix with the given value on the main diagonal.
* @param[in] value The value placed on the main diagonal.
* @return The diagonal matrix.
*/
fpl_force_inline Mat4f M4fInit(const float value) {
	Mat4f result = {
		V4fInit(value, 0.0f, 0.0f, 0.0f),
		V4fInit(0.0f, value, 0.0f, 0.0f),
		V4fInit(0.0f, 0.0f, value, 0.0f),
		V4fInit(0.0f, 0.0f, 0.0f, value),
	};
	return(result);
}

/**
* @brief Creates a 4x4 identity matrix.
* @return The identity matrix.
*/
fpl_force_inline Mat4f M4fIdentity() {
	Mat4f result = M4fInit(1.0f);
	return(result);
}

/**
* @brief Creates a copy of a Mat4f.
* @param[in] other The matrix to copy.
* @return A copy of the input matrix.
*/
fpl_force_inline Mat4f M4fCopy(const Mat4f other) {
	Mat4f result = fplStructInit(Mat4f, other.col1, other.col2, other.col3, other.col4);
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing a diagonal Mat4f (identity by default).
fpl_force_inline Mat4f M4f(const float value = 1.0f) {
	return M4fInit(value);
}
//! C++ overload constructing a Mat4f from another Mat4f.
fpl_force_inline Mat4f M4f(const Mat4f &other) {
	return M4fCopy(other);
}
#endif

//
// Quaternion
//

/**
* @union Quaternion
* @brief A quaternion stored as a scalar w and a vector part (x, y, z).
*/
typedef union Quaternion {
	struct {
		//! The scalar (real) part, same as w.
		float s;
		//! The vector (imaginary) part, same as x/y/z.
		Vec3f n;
	};
	struct {
		//! The scalar (real) part.
		float w;
		//! The imaginary x component.
		float x;
		//! The imaginary y component.
		float y;
		//! The imaginary z component.
		float z;
	};
} Quaternion;

//! Brace-initializer list for a Quaternion literal.
#define QU(w, x, y, z) {w, x, y, z}
//! Casts a brace-initializer list to a Quaternion value.
#define QUArg(v) (Quaternion)v

/**
* @brief Creates a quaternion from its scalar and imaginary components.
* @param[in] w The scalar (real) part.
* @param[in] x The imaginary x component.
* @param[in] y The imaginary y component.
* @param[in] z The imaginary z component.
* @return The initialized quaternion.
*/
fpl_force_inline Quaternion QuatInit(const float w, const float x, const float y, const float z) {
	Quaternion result;
	result.w = w;
	result.x = x;
	result.y = y;
	result.z = z;
	return(result);
}

/**
* @brief Creates the identity quaternion (no rotation).
* @return The identity quaternion (w=1, x=y=z=0).
*/
fpl_force_inline Quaternion QuatIdentity() {
	Quaternion result = QuatInit(1.0f, 0.0f, 0.0f, 0.0f);
	return(result);
}

/**
* @brief Creates a quaternion from a scalar and a vector part.
* @param[in] s The scalar (real) part.
* @param[in] axis The vector (imaginary) part.
* @return The initialized quaternion.
*/
fpl_force_inline Quaternion QuatInitSXYZ(const float s, const Vec3f axis) {
	Quaternion result;
	result.s = s;
	result.n = axis;
	return(result);
}

#if defined(__cplusplus)
//! C++ overload constructing the identity quaternion.
fpl_force_inline Quaternion Quat() {
	return QuatIdentity();
}
//! C++ overload constructing a quaternion from its four components.
fpl_force_inline Quaternion Quat(const float w, const float x, const float y, const float z) {
	return QuatInit(w, x, y, z);
}
//! C++ overload constructing a quaternion from a scalar and a vector part.
fpl_force_inline Quaternion Quat(const float s, const Vec3f &axis) {
	return QuatInitSXYZ(s, axis);
}
#endif

//
// Color & Pixel
//

/**
* @union Pixel
* @brief A 32-bit BGRA color, accessible per channel, as a packed uint or by index.
*/
typedef union Pixel {
	struct {
		//! The blue channel.
		uint8_t b;
		//! The green channel.
		uint8_t g;
		//! The red channel.
		uint8_t r;
		//! The alpha channel.
		uint8_t a;
	};
	//! The channels packed as a single uint32_t in BGRA order.
	uint32_t bgra;
	//! The channels as an indexable array.
	uint8_t m[4];
} Pixel;

//! Brace-initializer list for a Pixel literal in BGRA order.
#define PX(b, g, r, a) {b, g, r, a}
//! Casts a brace-initializer list to a Pixel value.
#define PXArg(p) (Pixel)p

/**
* @struct Viewport4i
* @brief An integer viewport rectangle (x, y, width, height).
*/
typedef struct Viewport4i {
	//! The left edge in pixels.
	int32_t x;
	//! The bottom/top edge in pixels.
	int32_t y;
	//! The width in pixels.
	int32_t w;
	//! The height in pixels.
	int32_t h;
} Viewport4i;

//! Brace-initializer list for a Viewport4i literal.
#define VP4I(x, y, w, h) {x, y, w, h}
//! Casts a brace-initializer list to a Viewport4i value.
#define VP4IArg(vp) (Viewport4i)vp

/**
* @brief Creates a Viewport4i from position and size.
* @param[in] x The left edge.
* @param[in] y The bottom/top edge.
* @param[in] w The width.
* @param[in] h The height.
* @return The initialized viewport.
*/
fpl_force_inline Viewport4i VP4iInit(const int32_t x, const int32_t y, const int32_t w, const int32_t h) {
	return fplStructInit(Viewport4i, x, y, w, h);
}

/**
* @struct Viewport4f
* @brief A floating-point viewport rectangle (x, y, width, height).
*/
typedef struct Viewport4f {
	//! The left edge.
	float x;
	//! The bottom/top edge.
	float y;
	//! The width.
	float w;
	//! The height.
	float h;
} Viewport4f;

//! Brace-initializer list for a Viewport4f literal.
#define VP4F(x, y, w, h) {x, y, w, h}
//! Casts a brace-initializer list to a Viewport4f value.
#define VP4FArg(vp) (Viewport4f)vp

/**
* @brief Creates a Viewport4f from position and size.
* @param[in] x The left edge.
* @param[in] y The bottom/top edge.
* @param[in] w The width.
* @param[in] h The height.
* @return The initialized viewport.
*/
fpl_force_inline Viewport4f VP4fInit(const float x, const float y, const float w, const float h) {
	return fplStructInit(Viewport4f, x, y, w, h);
}

//
// Scalar
//
/**
* @brief Returns a quiet NaN without depending on math.h NAN.
* @return A 32-bit quiet NaN.
*/
fpl_force_inline float F32NaN() {
	union { uint32_t u; float f; } bits;
	bits.u = 0x7FC00000u;
	return bits.f;
}
/**
* @brief Returns positive infinity without depending on math.h INFINITY.
* @return A 32-bit positive infinity.
*/
fpl_force_inline float F32Infinity() {
	union { uint32_t u; float f; } bits;
	bits.u = 0x7F800000u;
	return bits.f;
}
/**
* @brief Tests whether a float is NaN.
* @param[in] x The value to test.
* @return True when the value is NaN, false otherwise.
*/
fpl_force_inline bool F32IsNaN(const float x) {
	// NaN is the only value not equal to itself (no math.h dependency).
	return x != x;
}
/**
* @brief Returns the sign of a float.
* @param[in] x The value to test.
* @return -1 for negative, +1 for positive, 0 for zero, and NaN when the input is NaN.
*/
fpl_force_inline float F32Sign(const float x) {
	if(F32IsNaN(x)) {
		return F32NaN();
	}
	if(x == 0.0) {
		return 0.0;
	}
	int b = (x > 0.0f) - (x < 0.0f);
	return (float)b;
}
/**
* @brief Computes the cosine of an angle.
* @param[in] angle The angle in radians.
* @return The cosine of the angle.
*/
fpl_force_inline float F32Cos(const float angle) {
	float result = cosf(angle);
	return(result);
}
/**
* @brief Computes the sine of an angle.
* @param[in] angle The angle in radians.
* @return The sine of the angle.
*/
fpl_force_inline float F32Sin(const float angle) {
	float result = sinf(angle);
	return(result);
}
/**
* @brief Computes the tangent of an angle.
* @param[in] angle The angle in radians.
* @return The tangent of the angle.
*/
fpl_force_inline float F32Tan(const float angle) {
	float result = tanf(angle);
	return(result);
}
/**
* @brief Computes the arc cosine.
* @param[in] x The value, expected in the range [-1, 1].
* @return The angle in radians.
*/
fpl_force_inline float F32ArcCos(const float x) {
	float result = acosf(x);
	return(result);
}
/**
* @brief Computes the arc sine.
* @param[in] x The value, expected in the range [-1, 1].
* @return The angle in radians.
*/
fpl_force_inline float F32ArcSin(const float x) {
	float result = asinf(x);
	return(result);
}
/**
* @brief Computes the arc tangent.
* @param[in] x The value.
* @return The angle in radians.
*/
fpl_force_inline float F32ArcTan(const float x) {
	float result = atanf(x);
	return(result);
}
/**
* @brief Computes the arc tangent of y/x using the signs of both arguments to pick the quadrant.
* @param[in] y The y coordinate.
* @param[in] x The x coordinate.
* @return The angle in radians in the range (-Pi, Pi].
*/
fpl_force_inline float F32ArcTan2(const float y, const float x) {
	float result = atan2f(y, x);
	return(result);
}
/**
* @brief Computes the absolute value.
* @param[in] value The input value.
* @return The absolute value.
*/
fpl_force_inline float F32Abs(const float value) {
	float result = fabsf(value);
	return(result);
}
/**
* @brief Raises a base to a power.
* @param[in] x The base.
* @param[in] y The exponent.
* @return x raised to the power of y.
*/
fpl_force_inline float F32Power(const float x, const float y) {
	float result = powf(x, y);
	return(result);
}
/**
* @brief Computes the floating-point remainder of x/y.
* @param[in] x The dividend.
* @param[in] y The divisor.
* @return The remainder of x divided by y.
*/
fpl_force_inline float F32Modulate(const float x, const float y) {
	float result = fmodf(x, y);
	return result;
}
/**
* @brief Returns the smaller of two values.
* @param[in] a The first value.
* @param[in] b The second value.
* @return The minimum of a and b.
*/
fpl_force_inline float F32Min(const float a, const float b) {
	float result = a < b ? a : b;
	return(result);
}
/**
* @brief Returns the larger of two values.
* @param[in] a The first value.
* @param[in] b The second value.
* @return The maximum of a and b.
*/
fpl_force_inline float F32Max(const float a, const float b) {
	float result = a > b ? a : b;
	return(result);
}
/**
* @brief Computes the square root.
* @param[in] value The input value, expected to be non-negative.
* @return The square root of the value.
*/
fpl_force_inline float F32SquareRoot(const float value) {
	float result = sqrtf(value);
	return(result);
}
/**
* @brief Converts radians to degrees.
* @param[in] radians The angle in radians.
* @return The angle in degrees.
*/
fpl_force_inline float F32RadiansToDegrees(const float radians) {
	float result = radians * F32Rad2Deg;
	return(result);
}
/**
* @brief Converts degrees to radians.
* @param[in] degrees The angle in degrees.
* @return The angle in radians.
*/
fpl_force_inline float F32DegreesToRadians(const float degrees) {
	float result = degrees * F32Deg2Rad;
	return(result);
}

/**
* @brief Linearly interpolates between two values.
* @param[in] a The start value (at t=0).
* @param[in] t The interpolation factor, typically in [0, 1].
* @param[in] b The end value (at t=1).
* @return The interpolated value.
*/
fpl_force_inline float F32Lerp(float a, float t, float b) {
	float result = (1.0f - t) * a + t * b;
	return(result);
}

/**
* @brief Blends an old value toward a new value by a weight.
* @param[in] oldValue The previous value (at t=0).
* @param[in] t The weight of the new value, typically in [0, 1].
* @param[in] newValue The new value (at t=1).
* @return The blended value.
*/
fpl_force_inline float F32Avg(float oldValue, float t, float newValue) {
	float result = t * newValue + (1.0f - t) * oldValue;
	return(result);
}

/**
* @brief Clamps a value into an inclusive range.
* @param[in] value The value to clamp.
* @param[in] min The lower bound.
* @param[in] max The upper bound.
* @return The clamped value.
*/
fpl_force_inline float F32Clamp(float value, float min, float max) {
	float result = fplMin(fplMax(value, min), max);
	return(result);
}

/**
* @brief Computes the shortest signed angular distance between two angles.
* @param[in] a0 The start angle in radians.
* @param[in] a1 The end angle in radians.
* @return The shortest signed distance in radians.
*/
fpl_force_inline float F32GetBestAngleDistance(float a0, float a1) {
	float max = F32Pi * 2;
	float da = fmodf(a1 - a0, max);
	float result = fmodf(2.0f * da, max) - da;
	return(result);
}

/**
* @brief Interpolates between two angles along the shortest path.
* @param[in] a The start angle in radians (at t=0).
* @param[in] t The interpolation factor, typically in [0, 1].
* @param[in] b The end angle in radians (at t=1).
* @return The interpolated angle in radians.
*/
fpl_force_inline float F32AngleLerp(float a, float t, float b) {
	float angleDistance = F32GetBestAngleDistance(a, b);
	float result = F32Lerp(a, t, a + angleDistance);
	return(result);
}

/**
* @brief Normalizes an angle into the range [0, 2*Pi).
* @param[in] angle The angle in radians.
* @return The normalized angle in radians.
*/
fpl_force_inline float F32AngleNormalize(const float angle) {
	float x = F32Modulate(angle, F32Tau);
	if (x < 0.0f) x += F32Tau;
	return x;
}

/**
* @brief Rounds a normalized float (0..1) to a byte (0..255).
* @param[in] value The value in the range [0, 1].
* @return The rounded byte value.
*/
fpl_force_inline uint8_t RoundF32ToU8(float value) {
	uint8_t result = (uint8_t)(value * 255.0f + 0.5f);
	return(result);
}

/**
* @brief Converts a byte (0..255) to a normalized float (0..1).
* @param[in] value The byte value.
* @return The value mapped into the range [0, 1].
*/
fpl_force_inline float RoundU8ToF32(uint8_t value) {
	float result = value * F32InvByte;
	return(result);
}

//
// Vec2f
//
/**
* @brief Adds two vectors component-wise.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The sum a + b.
*/
fpl_force_inline Vec2f V2fAdd(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(a.x + b.x, a.y + b.y);
	return(result);
}

/**
* @brief Subtracts two vectors component-wise.
* @param[in] a The vector to subtract from.
* @param[in] b The vector to subtract.
* @return The difference a - b.
*/
fpl_force_inline Vec2f V2fSub(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(a.x - b.x, a.y - b.y);
	return(result);
}

/**
* @brief Multiplies a vector by a scalar.
* @param[in] v The vector.
* @param[in] s The scalar.
* @return The scaled vector.
*/
fpl_force_inline Vec2f V2fMultScalar(const Vec2f v, const float s) {
	Vec2f result = V2fInit(v.x * s, v.y * s);
	return(result);
}

/**
* @brief Adds a scaled vector to another vector (a + b * s).
* @param[in] a The base vector.
* @param[in] b The vector to scale and add.
* @param[in] s The scalar applied to b.
* @return The result a + b * s.
*/
fpl_force_inline Vec2f V2fAddMultScalar(const Vec2f a, const Vec2f b, const float s) {
	Vec2f result = V2fInit(a.x + b.x * s, a.y + b.y * s);
	return(result);
}

#if defined(__cplusplus)
//! C++ operator multiplying a Vec2f by a scalar.
fpl_force_inline Vec2f operator*(const Vec2f &v, float s) {
	Vec2f result = V2fMultScalar(v, s);
	return(result);
}

//! C++ operator multiplying a scalar by a Vec2f.
fpl_force_inline Vec2f operator*(float s, const Vec2f &v) {
	Vec2f result = V2fMultScalar(v, s);
	return(result);
}

//! C++ compound operator scaling a Vec2f in place.
fpl_force_inline Vec2f &operator*=(Vec2f &v, float s) {
	v = V2fMultScalar(v, s);
	return(v);
}

//! C++ unary negation of a Vec2f.
fpl_force_inline Vec2f operator-(const Vec2f &v) {
	Vec2f result = V2fInit(-v.x, -v.y);
	return(result);
}

//! C++ operator adding two Vec2f vectors.
fpl_force_inline Vec2f operator+(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2fAdd(a, b);
	return(result);
}

//! C++ compound operator adding a Vec2f in place.
fpl_force_inline Vec2f &operator+=(Vec2f &a, const Vec2f &b) {
	a = V2fAdd(a, b);
	return(a);
}

//! C++ operator subtracting two Vec2f vectors.
fpl_force_inline Vec2f operator-(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2fSub(a, b);
	return(result);
}

//! C++ compound operator subtracting a Vec2f in place.
fpl_force_inline Vec2f &operator-=(Vec2f &a, const Vec2f &b) {
	a = V2fSub(a, b);
	return(a);
}
#endif // __cplusplus

/**
* @brief Computes the component-wise absolute value.
* @param[in] v The input vector.
* @return The vector with absolute components.
*/
fpl_force_inline Vec2f V2fAbs(const Vec2f v) {
	return V2fInit(F32Abs(v.x), F32Abs(v.y));
}

/**
* @brief Negates a vector.
* @param[in] v The input vector.
* @return The negated vector.
*/
fpl_force_inline Vec2f V2fNegate(const Vec2f v) {
	return V2fInit(-v.x, -v.y);
}

/**
* @brief Computes the dot product of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The dot product.
*/
fpl_force_inline float V2fDot(const Vec2f a, const Vec2f b) {
	float result = a.x * b.x + a.y * b.y;
	return(result);
}

/**
* @brief Computes the length (magnitude) of a vector.
* @param[in] v The input vector.
* @return The Euclidean length.
*/
fpl_force_inline float V2fLength(const Vec2f v) {
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return(result);
}

/**
* @brief Normalizes a vector to unit length.
* @param[in] v The input vector.
* @return The unit-length vector, or the original vector when its length is zero.
*/
fpl_force_inline Vec2f V2fNormalize(const Vec2f v) {
	float l = V2fLength(v);
	if (l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec2f result = V2fMultScalar(v, invL);
	return(result);
}

/**
* @brief Computes the component-wise (Hadamard) product of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The component-wise product.
*/
fpl_force_inline Vec2f V2fHadamard(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(a.x * b.x, a.y * b.y);
	return(result);
}

/**
* @brief Transforms a vector by a 2x2 matrix (A * v).
* @param[in] A The column-major 2x2 matrix.
* @param[in] v The vector to transform.
* @return The transformed vector.
*/
fpl_force_inline Vec2f V2fMultMat2(const Mat2f A, const Vec2f v) {
	Vec2f result = V2fInit(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
	return(result);
}

/**
* @brief Computes the squared distance between two points.
* @param[in] a The first point.
* @param[in] b The second point.
* @return The squared distance.
*/
fpl_force_inline float V2fDistanceSquared(const Vec2f a, const Vec2f b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float result = dx * dx + dy * dy;
	return(result);
}

/**
* @brief Returns the left perpendicular vector (rotated 90 degrees CCW).
* @param[in] v The input vector.
* @return The perpendicular vector (-y, x).
*/
fpl_force_inline Vec2f V2fPerp(const Vec2f v) {
	return V2fInit(-v.y, v.x);
}

/**
* @brief Returns the right perpendicular vector (vector-scalar cross product).
* @param[in] a The input vector.
* @param[in] s The scalar.
* @return The right perpendicular vector (s*y, -s*x).
*/
fpl_force_inline Vec2f V2fCrossR(const Vec2f a, float s) {
	return V2fInit(s * a.y, -s * a.x);
}

/**
* @brief Returns the left perpendicular vector (scalar-vector cross product).
* @param[in] s The scalar.
* @param[in] a The input vector.
* @return The left perpendicular vector (-s*y, s*x).
*/
fpl_force_inline Vec2f V2fCrossL(float s, const Vec2f a) {
	return V2fInit(-s * a.y, s * a.x);
}

/**
* @brief Returns the scalar (z) component of the 2D cross product.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The signed area of the parallelogram (a.x*b.y - a.y*b.x).
*/
fpl_force_inline float V2fCrossZ(const Vec2f a, const Vec2f b) {
	return a.x * b.y - a.y * b.x;
}

/**
* @brief Computes the angle of an axis vector.
* @param[in] axis The direction vector.
* @return The angle in radians measured from the positive x axis.
*/
fpl_force_inline float V2fAngleFromAxis(const Vec2f axis) {
	float result = F32ArcTan2(axis.y, axis.x);
	return(result);
}

/**
* @brief Computes a unit direction vector from an angle.
* @param[in] angle The angle in radians.
* @return The unit vector (cos, sin).
*/
fpl_force_inline Vec2f V2fAxisFromAngle(const float angle) {
	Vec2f result = V2fInit(F32Cos(angle), F32Sin(angle));
	return(result);
}

/**
* @brief Returns the dominant axis of a vector as a signed unit axis.
* @param[in] v The input vector.
* @return A unit vector along the axis with the larger magnitude, signed to match the input.
*/
fpl_force_inline Vec2f V2fMajorAxis(const Vec2f v) {
	float x = F32Abs(v.x);
	float y = F32Abs(v.y);
	if (x > y) {
		return V2fInit(F32Sign(v.x), 0.0f);
	} else {
		return V2fInit(0.0f, F32Sign(v.y));
	}
}

/**
* @brief Generates a random unit direction vector.
* @return A unit vector pointing in a uniformly random direction.
*/
fpl_force_inline Vec2f V2fRandomDirection() {
	float d = rand() / (float)RAND_MAX;
	float angle = d * F32Tau;
	Vec2f result = V2fInit(F32Cos(angle), F32Sin(angle));
	return(result);
}

/**
* @brief Linearly interpolates between two vectors.
* @param[in] a The start vector (at t=0).
* @param[in] t The interpolation factor, typically in [0, 1].
* @param[in] b The end vector (at t=1).
* @return The interpolated vector.
*/
fpl_force_inline Vec2f V2fLerp(const Vec2f a, const float t, const Vec2f b) {
	Vec2f result;
	result.x = F32Lerp(a.x, t, b.x);
	result.y = F32Lerp(a.y, t, b.y);
	return(result);
}

/**
* @brief Computes the component-wise minimum of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The component-wise minimum.
*/
fpl_force_inline Vec2f V2fMin(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(F32Min(a.x, b.x), F32Min(a.y, b.y));
	return(result);
}

/**
* @brief Computes the component-wise maximum of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The component-wise maximum.
*/
fpl_force_inline Vec2f V2fMax(const Vec2f a, const Vec2f b) {
	Vec2f result = V2fInit(F32Max(a.x, b.x), F32Max(a.y, b.y));
	return(result);
}

/**
* @brief Clamps a vector component-wise into an inclusive range.
* @param[in] value The vector to clamp.
* @param[in] min The component-wise lower bound.
* @param[in] max The component-wise upper bound.
* @return The clamped vector.
*/
fpl_force_inline Vec2f V2fClamp(const Vec2f value, const Vec2f min, const Vec2f max) {
	Vec2f result = V2fMin(V2fMax(value, min), max);
	return(result);
}

//
// Vec2i
//
/**
* @brief Adds two integer vectors component-wise.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The sum a + b.
*/
fpl_force_inline Vec2i V2iAdd(const Vec2i a, const Vec2i b) {
	Vec2i result = V2iInit(a.x + b.x, a.y + b.y);
	return(result);
}

/**
* @brief Subtracts two integer vectors component-wise.
* @param[in] a The vector to subtract from.
* @param[in] b The vector to subtract.
* @return The difference a - b.
*/
fpl_force_inline Vec2i V2iSub(const Vec2i a, const Vec2i b) {
	Vec2i result = V2iInit(a.x - b.x, a.y - b.y);
	return(result);
}

/**
* @brief Multiplies two integer vectors component-wise.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The component-wise product.
*/
fpl_force_inline Vec2i V2iMult(const Vec2i a, const Vec2i b) {
	Vec2i result = V2iInit(a.x * b.x, a.y * b.y);
	return(result);
}

/**
* @brief Divides two integer vectors component-wise.
* @param[in] a The dividend vector.
* @param[in] b The divisor vector.
* @return The component-wise quotient.
*/
fpl_force_inline Vec2i V2iDiv(const Vec2i a, const Vec2i b) {
	Vec2i result = V2iInit(a.x / b.x, a.y / b.y);
	return(result);
}

/**
* @brief Tests two integer vectors for equality.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return True when both components are equal.
*/
fpl_force_inline bool V2iEquals(const Vec2i a, const Vec2i b) {
	bool result = a.x == b.x && a.y == b.y;
	return(result);
}

//
// Vec3f
//
/**
* @brief Negates a vector.
* @param[in] v The input vector.
* @return The negated vector.
*/
fpl_force_inline Vec3f V3fNegate(const Vec3f v) {
	Vec3f result = V3fInit(-v.x, -v.y, -v.z);
	return(result);
}

/**
* @brief Computes the component-wise absolute value.
* @param[in] v The input vector.
* @return The vector with absolute components.
*/
fpl_force_inline Vec3f V3fAbs(const Vec3f v) {
	Vec3f result = V3fInit(F32Abs(v.x), F32Abs(v.y), F32Abs(v.z));
	return(result);
}

/**
* @brief Multiplies a vector by a scalar.
* @param[in] v The vector.
* @param[in] s The scalar.
* @return The scaled vector.
*/
fpl_force_inline Vec3f V3fMultScalar(const Vec3f v, const float s) {
	Vec3f result = V3fInit(v.x * s, v.y * s, v.z * s);
	return(result);
}

/**
* @brief Adds two vectors component-wise.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The sum a + b.
*/
fpl_force_inline Vec3f V3fAdd(const Vec3f a, const Vec3f b) {
	Vec3f result = V3fInit(a.x + b.x, a.y + b.y, a.z + b.z);
	return(result);
}

/**
* @brief Subtracts two vectors component-wise.
* @param[in] a The vector to subtract from.
* @param[in] b The vector to subtract.
* @return The difference a - b.
*/
fpl_force_inline Vec3f V3fSub(const Vec3f a, const Vec3f b) {
	Vec3f result = V3fInit(a.x - b.x, a.y - b.y, a.z - b.z);
	return(result);
}

/**
* @brief Computes the dot product of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The dot product.
*/
fpl_force_inline float V3fDot(const Vec3f a, const Vec3f b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return(result);
}

/**
* @brief Computes the squared distance between two points.
* @param[in] a The first point.
* @param[in] b The second point.
* @return The squared distance.
*/
fpl_force_inline float V3fDistanceSquared(const Vec3f a, const Vec3f b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;
	float result = dx * dx + dy * dy + dz * dz;
	return(result);
}

/**
* @brief Computes the squared length (magnitude) of a vector.
* @param[in] v The input vector.
* @return The squared Euclidean length.
*/
fpl_force_inline float V3fLength2(const Vec3f v) {
	float result = V3fDot(v, v);
	return(result);
}

/**
* @brief Computes the length (magnitude) of a vector.
* @param[in] v The input vector.
* @return The Euclidean length.
*/
fpl_force_inline float V3fLength(const Vec3f v) {
	float result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return(result);
}

/**
* @brief Normalizes a vector to unit length.
* @param[in] v The input vector.
* @return The unit-length vector, or the original vector when its length is zero.
*/
fpl_force_inline Vec3f V3fNormalize(const Vec3f v) {
	float l = V3fLength(v);
	if (l == 0) {
		l = 1;
	}
	float invL = 1.0f / l;
	Vec3f result = V3fMultScalar(v, invL);
	return(result);
}

/**
* @brief Computes the cross product of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The cross product a x b, perpendicular to both inputs.
*/
fpl_force_inline Vec3f V3fCross(const Vec3f a, const Vec3f b) {
	Vec3f result = V3fInit(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
	return(result);
}

/**
* @brief Linearly interpolates between two vectors.
* @param[in] a The start vector (at t=0).
* @param[in] t The interpolation factor, typically in [0, 1].
* @param[in] b The end vector (at t=1).
* @return The interpolated vector.
*/
fpl_force_inline Vec3f V3fLerp(const Vec3f a, float t, const Vec3f b) {
	Vec3f result;
	result.x = F32Lerp(a.x, t, b.x);
	result.y = F32Lerp(a.y, t, b.y);
	result.z = F32Lerp(a.z, t, b.z);
	return(result);
}

/**
* @brief Computes the component-wise (Hadamard) product of two vectors.
* @param[in] a The first vector.
* @param[in] b The second vector.
* @return The component-wise product.
*/
fpl_force_inline Vec3f V3fHadamard(const Vec3f a, const Vec3f b) {
	Vec3f result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return(result);
}

#if defined(__cplusplus)
//! C++ operator multiplying a scalar by a Vec3f.
fpl_force_inline Vec3f operator*(float s, const Vec3f &v) {
	Vec3f result = V3fMultScalar(v, s);
	return(result);
}

//! C++ operator multiplying a Vec3f by a scalar.
fpl_force_inline Vec3f operator*(const Vec3f &v, float s) {
	Vec3f result = V3fMultScalar(v, s);
	return(result);
}

//! C++ compound operator scaling a Vec3f in place.
fpl_force_inline Vec3f &operator*=(Vec3f &v, float s) {
	v = s * v;
	return(v);
}

//! C++ operator adding two Vec3f vectors.
fpl_force_inline Vec3f operator+(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3fAdd(a, b);
	return(result);
}

//! C++ compound operator adding a Vec3f in place.
fpl_force_inline Vec3f &operator+=(Vec3f &a, const Vec3f &b) {
	a = a + b;
	return(a);
}

//! C++ operator subtracting two Vec3f vectors.
fpl_force_inline Vec3f operator-(const Vec3f &a, const Vec3f &b) {
	Vec3f result = V3fSub(a, b);
	return(result);
}

//! C++ unary negation of a Vec3f.
fpl_force_inline Vec3f operator-(const Vec3f &v) {
	Vec3f result = V3fNegate(v);
	return(result);
}

//! C++ compound operator subtracting a Vec3f in place.
fpl_force_inline Vec3f &operator-=(Vec3f &a, const Vec3f &b) {
	a = a - b;
	return(a);
}
#endif // __cplusplus

//
// Mat2f
//
/**
* @brief Creates a 2x2 rotation matrix from an angle.
* @param[in] angle The rotation angle in radians.
* @return The rotation matrix.
*/
fpl_force_inline Mat2f M2fFromAngle(float angle) {
	float s = F32Sin(angle);
	float c = F32Cos(angle);
	Mat2f result;
	result.col1 = V2fInit(c, s);
	result.col2 = V2fInit(-s, c);
	return(result);
}

/**
* @brief Creates a 2x2 rotation matrix from an axis (first column).
* @param[in] axis The direction used as the first column; the second column is its left perpendicular.
* @return The rotation matrix.
*/
fpl_force_inline Mat2f M2fFromAxis(const Vec2f axis) {
	Mat2f result;
	result.col1 = axis;
	result.col2 = V2fCrossL(1.0f, axis);
	return(result);
}

/**
* @brief Transposes a 2x2 matrix.
* @param[in] m The input matrix.
* @return The transposed matrix.
*/
fpl_force_inline Mat2f M2fTranspose(const Mat2f m) {
	Mat2f result;
	result.col1 = V2fInit(m.col1.x, m.col2.x);
	result.col2 = V2fInit(m.col1.y, m.col2.y);
	return(result);
}

/**
* @brief Multiplies two 2x2 matrices (a * b).
* @param[in] a The left-hand matrix.
* @param[in] b The right-hand matrix.
* @return The matrix product a * b.
*/
fpl_force_inline Mat2f M2fMult(const Mat2f a, const Mat2f b) {
	Mat2f result;
	result.col1 = V2fMultMat2(a, b.col1);
	result.col2 = V2fMultMat2(a, b.col2);
	return(result);
}

/**
* @brief Extracts the rotation angle from a 2x2 rotation matrix.
* @param[in] mat The rotation matrix.
* @return The angle in radians.
*/
fpl_force_inline float M2fToAngle(const Mat2f mat) {
	float result = V2fAngleFromAxis(mat.col1);
	return(result);
}

/**
* @brief Multiplies the transpose of a by b, transforming from frame B to frame A.
* @param[in] a The matrix whose transpose is used on the left.
* @param[in] b The right-hand matrix.
* @return The product transpose(a) * b.
*/
fpl_force_inline Mat2f M2fMultTranspose(const Mat2f a, const Mat2f b) {
	Mat2f result;
	result.col1 = V2fInit(V2fDot(a.col1, b.col1), V2fDot(a.col2, b.col1));
	result.col2 = V2fInit(V2fDot(a.col1, b.col2), V2fDot(a.col2, b.col2));
	return(result);
}

//
// Mat4f
//
fpl_force_inline static Mat4f M4fOrthoRH(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar) {
	Mat4f result = M4fInit(1.0f);
	result.r[0][0] = 2.0f / (right - left);
	result.r[1][1] = 2.0f / (top - bottom);
	result.r[2][2] = -2.0f / (zFar - zNear);
	result.r[3][0] = -(right + left) / (right - left);
	result.r[3][1] = -(top + bottom) / (top - bottom);
	result.r[3][2] = -(zFar + zNear) / (zFar - zNear);
	return (result);
}

fpl_force_inline static Mat4f M4fPerspectiveRH(const float fov, const float aspect, const float zNear, const float zFar) {
	float tanHalfFov = F32Tan(fov * 0.5f);
	Mat4f result = M4fInit(0.0f);
	result.r[0][0] = 1.0f / (aspect * tanHalfFov);
	result.r[1][1] = 1.0f / (tanHalfFov);
	result.r[2][2] = -(zFar + zNear) / (zFar - zNear);
	result.r[2][3] = -1.0f;
	result.r[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	return (result);
}

fpl_force_inline static Mat4f M4fLookAtRH(const Vec3f eye, const Vec3f center, const Vec3f up) {
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

fpl_force_inline static Mat4f M4fTranslationV2(const Vec2f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4.xy = p;
	result.col4.z = 0.0f;
	result.col4.w = 1.0f;
	return (result);
}

fpl_force_inline static Mat4f M4fTranslationV3(const Vec3f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4.xyz = p;
	result.col4.w = 1.0f;
	return (result);
}

fpl_force_inline static Mat4f M4fTranslationV4(const Vec4f p) {
	Mat4f result = M4fInit(1.0f);
	result.col4 = p;
	return (result);
}

#if defined(__cplusplus)
fpl_force_inline static Mat4f M4fTranslation(const Vec2f p) {
	return M4fTranslationV2(p);
}
fpl_force_inline static Mat4f M4fTranslation(const Vec3f p) {
	return M4fTranslationV3(p);
}
fpl_force_inline static Mat4f M4fTranslation(const Vec4f p) {
	return M4fTranslationV4(p);
}
#endif // __cplusplus

fpl_force_inline static Mat4f M4fScaleScalar(const float s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s;
	result.col2.y = s;
	result.col3.z = s;
	return (result);
}

fpl_force_inline static Mat4f M4fScaleV2(const Vec2f s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = 1.0f;
	return (result);
}

fpl_force_inline static Mat4f M4fScaleV3(const Vec3f s) {
	Mat4f result = M4fInit(1.0f);
	result.col1.x = s.x;
	result.col2.y = s.y;
	result.col3.z = s.z;
	return (result);
}

#if defined(__cplusplus)
fpl_force_inline static Mat4f M4fScale(const float s) {
	return M4fScaleScalar(s);
}
fpl_force_inline static Mat4f M4fScale(const Vec2f p) {
	return M4fScaleV2(p);
}
fpl_force_inline static Mat4f M4fScale(const Vec3f p) {
	return M4fScaleV3(p);
}
#endif // __cplusplus

fpl_force_inline static Mat4f M4fRotationX(const float angle) {
	float c = F32Cos(angle);
	float s = F32Sin(angle);
	Mat4f result;
	result.col1 = V4fInit(1.0f, 0.0f, 0.0f, 0.0f);
	result.col2 = V4fInit(0.0f, c, s, 0.0f);
	result.col3 = V4fInit(0.0f, -s, c, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f M4fRotationY(const float angle) {
	float c = F32Cos(angle);
	float s = F32Sin(angle);
	Mat4f result;
	result.col1 = V4fInit(c, 0.0f, -s, 0.0f);
	result.col2 = V4fInit(0.0f, 1.0f, 0.0f, 0.0f);
	result.col3 = V4fInit(s, 0.0f, c, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f M4fRotationZ(const float angle) {
	float c = F32Cos(angle);
	float s = F32Sin(angle);
	Mat4f result;
	result.col1 = V4fInit(c, s, 0.0f, 0.0f);
	result.col2 = V4fInit(-s, c, 0.0f, 0.0f);
	result.col3 = V4fInit(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline static Mat4f M4fRotationZFromM2f(const Mat2f m) {
	Mat4f result;
	result.col1 = V4fInit(m.col1.x, m.col1.y, 0.0f, 0.0f);
	result.col2 = V4fInit(-m.col1.y, m.col1.x, 0.0f, 0.0f);
	result.col3 = V4fInit(0.0f, 0.0f, 1.0f, 0.0f);
	result.col4 = V4fInit(0.0f, 0.0f, 0.0f, 1.0f);
	return (result);
}

fpl_force_inline Mat4f M4fMult(const Mat4f a, const Mat4f b) {
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

fpl_force_inline Mat4f M4fTranspose(const Mat4f m) {
	Mat4f result;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.r[col][row] = m.r[row][col];
		}
	}
	return(result);
}

fpl_force_inline Mat4f M4fInverse(const Mat4f mat) {
	float SubFactor00 = mat.r[2][2] * mat.r[3][3] - mat.r[3][2] * mat.r[2][3];
	float SubFactor01 = mat.r[2][1] * mat.r[3][3] - mat.r[3][1] * mat.r[2][3];
	float SubFactor02 = mat.r[2][1] * mat.r[3][2] - mat.r[3][1] * mat.r[2][2];
	float SubFactor03 = mat.r[2][0] * mat.r[3][3] - mat.r[3][0] * mat.r[2][3];
	float SubFactor04 = mat.r[2][0] * mat.r[3][2] - mat.r[3][0] * mat.r[2][2];
	float SubFactor05 = mat.r[2][0] * mat.r[3][1] - mat.r[3][0] * mat.r[2][1];
	float SubFactor06 = mat.r[1][2] * mat.r[3][3] - mat.r[3][2] * mat.r[1][3];
	float SubFactor07 = mat.r[1][1] * mat.r[3][3] - mat.r[3][1] * mat.r[1][3];
	float SubFactor08 = mat.r[1][1] * mat.r[3][2] - mat.r[3][1] * mat.r[1][2];
	float SubFactor09 = mat.r[1][0] * mat.r[3][3] - mat.r[3][0] * mat.r[1][3];
	float SubFactor10 = mat.r[1][0] * mat.r[3][2] - mat.r[3][0] * mat.r[1][2];
	float SubFactor11 = mat.r[1][1] * mat.r[3][3] - mat.r[3][1] * mat.r[1][3];
	float SubFactor12 = mat.r[1][0] * mat.r[3][1] - mat.r[3][0] * mat.r[1][1];
	float SubFactor13 = mat.r[1][2] * mat.r[2][3] - mat.r[2][2] * mat.r[1][3];
	float SubFactor14 = mat.r[1][1] * mat.r[2][3] - mat.r[2][1] * mat.r[1][3];
	float SubFactor15 = mat.r[1][1] * mat.r[2][2] - mat.r[2][1] * mat.r[1][2];
	float SubFactor16 = mat.r[1][0] * mat.r[2][3] - mat.r[2][0] * mat.r[1][3];
	float SubFactor17 = mat.r[1][0] * mat.r[2][2] - mat.r[2][0] * mat.r[1][2];
	float SubFactor18 = mat.r[1][0] * mat.r[2][1] - mat.r[2][0] * mat.r[1][1];

	Mat4f Inverse = fplZeroInit;

	Inverse.r[0][0] = +(mat.r[1][1] * SubFactor00 - mat.r[1][2] * SubFactor01 + mat.r[1][3] * SubFactor02);
	Inverse.r[0][1] = -(mat.r[1][0] * SubFactor00 - mat.r[1][2] * SubFactor03 + mat.r[1][3] * SubFactor04);
	Inverse.r[0][2] = +(mat.r[1][0] * SubFactor01 - mat.r[1][1] * SubFactor03 + mat.r[1][3] * SubFactor05);
	Inverse.r[0][3] = -(mat.r[1][0] * SubFactor02 - mat.r[1][1] * SubFactor04 + mat.r[1][2] * SubFactor05);

	Inverse.r[1][0] = -(mat.r[0][1] * SubFactor00 - mat.r[0][2] * SubFactor01 + mat.r[0][3] * SubFactor02);
	Inverse.r[1][1] = +(mat.r[0][0] * SubFactor00 - mat.r[0][2] * SubFactor03 + mat.r[0][3] * SubFactor04);
	Inverse.r[1][2] = -(mat.r[0][0] * SubFactor01 - mat.r[0][1] * SubFactor03 + mat.r[0][3] * SubFactor05);
	Inverse.r[1][3] = +(mat.r[0][0] * SubFactor02 - mat.r[0][1] * SubFactor04 + mat.r[0][2] * SubFactor05);

	Inverse.r[2][0] = +(mat.r[0][1] * SubFactor06 - mat.r[0][2] * SubFactor07 + mat.r[0][3] * SubFactor08);
	Inverse.r[2][1] = -(mat.r[0][0] * SubFactor06 - mat.r[0][2] * SubFactor09 + mat.r[0][3] * SubFactor10);
	Inverse.r[2][2] = +(mat.r[0][0] * SubFactor11 - mat.r[0][1] * SubFactor09 + mat.r[0][3] * SubFactor12);
	Inverse.r[2][3] = -(mat.r[0][0] * SubFactor08 - mat.r[0][1] * SubFactor10 + mat.r[0][2] * SubFactor12);

	Inverse.r[3][0] = -(mat.r[0][1] * SubFactor13 - mat.r[0][2] * SubFactor14 + mat.r[0][3] * SubFactor15);
	Inverse.r[3][1] = +(mat.r[0][0] * SubFactor13 - mat.r[0][2] * SubFactor16 + mat.r[0][3] * SubFactor17);
	Inverse.r[3][2] = -(mat.r[0][0] * SubFactor14 - mat.r[0][1] * SubFactor16 + mat.r[0][3] * SubFactor18);
	Inverse.r[3][3] = +(mat.r[0][0] * SubFactor15 - mat.r[0][1] * SubFactor17 + mat.r[0][2] * SubFactor18);

	float Determinant =
		+ mat.r[0][0] * Inverse.r[0][0]
		+ mat.r[0][1] * Inverse.r[0][1]
		+ mat.r[0][2] * Inverse.r[0][2]
		+ mat.r[0][3] * Inverse.r[0][3];

	float inverseDet = 1.0f / Determinant;

	for (int i = 0; i < 16; ++i) {
		Inverse.m[i] *= inverseDet;
	}

	// The cofactor expansion above produces the inverse in row-major order;
	// transpose it back to the column-major storage used everywhere else so
	// M4fMult(m, M4fInverse(m)) == identity.
	return M4fTranspose(Inverse);
}

#if defined(__cplusplus)
fpl_force_inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
	Mat4f result = M4fMult(a, b);
	return(result);
}
#endif // __cplusplus

fpl_force_inline Vec4f V4fMultM4f(const Mat4f mat, const Vec4f v) {
	// Column-major storage: r[col][row], so element (row i, col j) is r[j][i].
	// Computes M * v, matches OpenGL matrix layout.
	Vec4f result;
	result.x = mat.r[0][0] * v.x + mat.r[1][0] * v.y + mat.r[2][0] * v.z + mat.r[3][0] * v.w;
	result.y = mat.r[0][1] * v.x + mat.r[1][1] * v.y + mat.r[2][1] * v.z + mat.r[3][1] * v.w;
	result.z = mat.r[0][2] * v.x + mat.r[1][2] * v.y + mat.r[2][2] * v.z + mat.r[3][2] * v.w;
	result.w = mat.r[0][3] * v.x + mat.r[1][3] * v.y + mat.r[2][3] * v.z + mat.r[3][3] * v.w;
	return(result);
}

//
// Quaternion
//
fpl_force_inline float QuatDot(const Quaternion a, const Quaternion b) {
	Vec4f tmp = V4fInit(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
	float result = (tmp.x + tmp.y) + (tmp.z + tmp.w);
	return(result);
}

fpl_force_inline Mat4f QuatToMat4(const Quaternion q) {
	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

	float sqw = q.w * q.w;
	float sqx = q.x * q.x;
	float sqy = q.y * q.y;
	float sqz = q.z * q.z;

	float tmp1, tmp2;

	// invs (inverse square length) is only required if quaternion is not already normalised
	float invs = 1.0f / (sqx + sqy + sqz + sqw);

	Mat4f result = M4fIdentity();

	result.r[0][0] = (sqx - sqy - sqz + sqw) * invs; // since sqw + sqx + sqy + sqz =1/invs*invs
	result.r[1][1] = (-sqx + sqy - sqz + sqw) * invs;
	result.r[2][2] = (-sqx - sqy + sqz + sqw) * invs;

	// Off-diagonals stored column-major (r[col][row]) to match M4fMult /
	// V4fMultM4f / the GL-uploaded layout, so the matrix rotates the same
	// direction as QuatMultV3f.
	tmp1 = q.x * q.y;
	tmp2 = q.z * q.w;
	result.r[0][1] = 2.0f * (tmp1 + tmp2) * invs;
	result.r[1][0] = 2.0f * (tmp1 - tmp2) * invs;

	tmp1 = q.x * q.z;
	tmp2 = q.y * q.w;
	result.r[0][2] = 2.0f * (tmp1 - tmp2) * invs;
	result.r[2][0] = 2.0f * (tmp1 + tmp2) * invs;

	tmp1 = q.y * q.z;
	tmp2 = q.x * q.w;
	result.r[1][2] = 2.0f * (tmp1 + tmp2) * invs;
	result.r[2][1] = 2.0f * (tmp1 - tmp2) * invs;

	return(result);
}

fpl_force_inline float QuatLength(const Quaternion q) {
	float result = F32SquareRoot(QuatDot(q, q));
	return(result);
}

fpl_force_inline Quaternion QuatNormalize(const Quaternion q) {
	float len = QuatLength(q);
	if (len <= 0.0f) {
		return QuatInit(1.0f, 0.0f, 0.0f, 0.0f);
	}
	float oneOverLen = 1.0f / len;
	Quaternion result = QuatInit(q.w * oneOverLen, q.x * oneOverLen, q.y * oneOverLen, q.z * oneOverLen);
	return(result);
}

fpl_force_inline Quaternion QuatAdd(const Quaternion a, const Quaternion b) {
	Quaternion result = QuatInit(a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z);
	return(result);
}

fpl_force_inline Quaternion QuatSub(const Quaternion a, const Quaternion b) {
	Quaternion result = QuatInit(a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z);
	return(result);
}

fpl_force_inline Quaternion QuatMultScalar(const Quaternion q, const float s) {
	Quaternion result = QuatInit(q.w * s, q.x * s, q.y * s, q.z * s);
	return(result);
}

fpl_force_inline Vec3f QuatMultV3f(const Quaternion q, const Vec3f v) {
	Vec3f quatVector = V3fInit(q.x, q.y, q.z);
	Vec3f uv = V3fCross(quatVector, v);
	Vec3f uuv = V3fCross(quatVector, uv);
	Vec3f result;
	result.x = v.x + ((uv.x * q.w) + uuv.x) * 2.0f;
	result.y = v.y + ((uv.y * q.w) + uuv.y) * 2.0f;
	result.z = v.z + ((uv.z * q.w) + uuv.z) * 2.0f;
	return(result);
}

fpl_force_inline Vec4f QuatMultV4f(const Quaternion q, const Vec4f v) {
	Vec3f quatVector = V3fInit(q.x, q.y, q.z);
	Vec3f uv = V3fCross(quatVector, v.xyz);
	Vec3f uuv = V3fCross(quatVector, uv);
	Vec4f result;
	result.x = v.x + ((uv.x * q.w) + uuv.x) * 2.0f;
	result.y = v.y + ((uv.y * q.w) + uuv.y) * 2.0f;
	result.z = v.z + ((uv.z * q.w) + uuv.z) * 2.0f;
	result.w = v.w; // TODO(tspaete): Not sure about this?
	return(result);
}

fpl_force_inline Quaternion QuatConjugate(const Quaternion quat) {
	float s = quat.s;
	Vec3f n = V3fMultScalar(quat.n, -1);
	Quaternion result = QuatInitSXYZ(s, n);
	return(result);
}

fpl_force_inline Quaternion QuatInverse(const Quaternion q) {
	Quaternion con = QuatConjugate(q);
	float d = QuatDot(q, q);
	float f = 1.0f / d;
	Quaternion result = QuatMultScalar(con, f);
	return(result);
}

fpl_force_inline Vec3f QuatAxis(const Quaternion q) {
	float tmp1 = 1.0f - q.w * q.w;
	if (tmp1 <= 0.0f) {
		return V3fInit(0, 0, 1);
	}
	float tmp2 = 1.0f / F32SquareRoot(tmp1);
	Vec3f result = V3fInit(q.x * tmp2, q.y * tmp2, q.z * tmp2);
	return(result);
}

fpl_force_inline Quaternion QuatCross(const Quaternion a, const Quaternion b) {
	float w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	float x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	float y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	float z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	Quaternion result = QuatInit(w, x, y, z);
	return(result);
}

fpl_force_inline Quaternion QuatLerp(const Quaternion a, const float t, const Quaternion b) {
	Quaternion tmp1 = QuatMultScalar(a, 1.0f - t);
	Quaternion tmp2 = QuatMultScalar(b, t);
	Quaternion result = QuatAdd(tmp1, tmp2);
	return(result);
}

fpl_force_inline float QuatAngle(const Quaternion q) {
	float result = F32ArcCos(q.w) * 2.0f;
	return(result);
}

fpl_force_inline Quaternion QuatFromAngleAxis(const float angle, const Vec3f axis) {
	Quaternion result;
	float const a = angle;
	float const s = F32Sin(a * 0.5f);
	result.w = F32Cos(a * 0.5f);
	result.x = axis.x * s;
	result.y = axis.y * s;
	result.z = axis.z * s;
	return(result);
}

fpl_force_inline float QuatRoll(const Quaternion q) {
	float result = F32ArcTan2(2.0f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);
	return(result);
}

fpl_force_inline float QuatPitch(const Quaternion q) {
	const float y = 2.0f * (q.y * q.z + q.w * q.x);
	const float x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
	float result;
	if (x == 0.0f && y == 0.0f) {
		result = 2.0f * F32ArcTan2(q.x, q.w);
	} else {
		result = F32ArcTan2(y, x);
	}
	return(result);
}

fpl_force_inline float QuatYaw(const Quaternion q) {
	float result = F32ArcSin(F32Clamp(-2.0f * (q.x * q.z - q.w * q.y), 1.0f, 1.0f));
	return(result);
}

fpl_force_inline Quaternion QuatRotation(const Vec3f orig, const Vec3f dest) {
	float cosTheta = V3fDot(orig, dest);
	Vec3f rotationAxis;

	if (cosTheta >= 1.0f - F32Epsilon) {
		// orig and dest point in the same direction
		return QuatIdentity();
	}

	if (cosTheta < -1.0f + F32Epsilon) {
		// special case when vectors in opposite directions :
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		// This implementation favors a rotation around the Up axis (Y),
		// since it's often what you want to do.
		rotationAxis = V3fCross(V3fInit(0, 0, 1), orig);
		if (V3fLength2(rotationAxis) < F32Epsilon) {
			// bad luck, they were parallel, try again!
			rotationAxis = V3fCross(V3fInit(1, 0, 0), orig);
		}

		rotationAxis = V3fNormalize(rotationAxis);
		return QuatFromAngleAxis(F32Pi, rotationAxis);
	}

	// Implementation from Stan Melax's Game Programming Gems 1 article
	rotationAxis = V3fCross(orig, dest);

	float s = F32SquareRoot((1.0f + cosTheta) * 2.0f);
	float invs = 1.0f / s;

	Quaternion result = QuatInit(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);

	return(result);
}

#if defined(__cplusplus)
#endif // __cplusplus

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
	Pixel result = fplZeroInit;
	result.r = r;
	result.g = g;
	result.b = b;
	result.a = a;
	return(result);
}
fpl_force_inline Pixel MakePixelFromU32(const uint32_t rgba) {
	Pixel result = fplZeroInit;
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
	Vec4f result = fplZeroInit;
	result.b = RoundU8ToF32((packed >> 0) & 0xFF);
	result.g = RoundU8ToF32((packed >> 8) & 0xFF);
	result.r = RoundU8ToF32((packed >> 16) & 0xFF);
	result.a = RoundU8ToF32((packed >> 24) & 0xFF);
	return(result);
}

fpl_force_inline Pixel PixelPack(const Vec4f unpacked) {
	Pixel result = fplZeroInit;
	result.r = RoundF32ToU8(unpacked.r);
	result.g = RoundF32ToU8(unpacked.g);
	result.b = RoundF32ToU8(unpacked.b);
	result.a = RoundF32ToU8(unpacked.a);
	return(result);
}

fpl_force_inline Vec4f PixelUnpack(const Pixel packed) {
	Vec4f result = fplZeroInit;
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
		return F32Power((x + 0.055f) / 1.055f, 2.4f);
}

fpl_force_inline float LinearToSRGB(const float x) {
	if (x <= 0.0f)
		return 0.0f;
	else if (x >= 1.0f)
		return 1.0f;
	else if (x < 0.0031308f)
		return x * 12.92f;
	else
		return F32Power(x, 1.0f / 2.4f) * 1.055f - 0.055f;
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

fpl_force_inline Vec4f RGBAToLinearHex24(const uint32_t hexValue24) {
	uint8_t r = (hexValue24 >> 16) & 0xFF;
	uint8_t g = (hexValue24 >> 8) & 0xFF;
	uint8_t b = (hexValue24 >> 0) & 0xFF;
	uint8_t a = 255;
	Pixel pixel = MakePixelFromRGBA(r, g, b, a);
	Vec4f result = PixelToLinearRaw(pixel);
	return(result);
}

fpl_force_inline Pixel LinearToPixelRaw(const Vec4f linear) {
	float r = linear.r;
	float g = linear.g;
	float b = linear.b;
	float a = linear.a;
	Pixel result = fplZeroInit;
	result.bgra = BGRAPack4x8(V4fInit(r, g, b, a));
	return(result);
}

fpl_force_inline Pixel LinearToPixelSRGB(const Vec4f linear) {
	float r = LinearToSRGB(linear.r);
	float g = LinearToSRGB(linear.g);
	float b = LinearToSRGB(linear.b);
	float a = linear.a;
	Pixel result = fplZeroInit;
	result.bgra = BGRAPack4x8(V4fInit(r, g, b, a));
	return(result);
}



fpl_force_inline Vec4f V4fDivideScalar(const Vec4f vec, const float divisor) {
	float ratio = 1.0f / divisor;
	Vec4f result = V4fInit(vec.x * ratio, vec.y * ratio, vec.z * ratio, vec.w * ratio);
	return result;
}

fpl_force_inline Vec2f V2fUnproject(const Vec2i screenPos, const Mat4f mvp, const Viewport4i viewport) {
	Mat4f inverse = M4fInverse(mvp);

	float x = (float)screenPos.x;
	float y = (float)screenPos.y;

	Vec4f tmp = V4fInit(x, y, 0.0f, 1.0f);

	// Convert to linear range (0.0 to 1.0)
	tmp.x = (tmp.x - viewport.x) / viewport.w;
	tmp.y = (tmp.y - viewport.y) / viewport.h;

	// Convert to clip range (-1.0 to 1.0)
	tmp.x = tmp.x * 2.0f - 1.0f;
	tmp.y = tmp.y * 2.0f - 1.0f;

	// Convert to world coordinates
	Vec4f obj = V4fMultM4f(inverse, tmp);
	obj = V4fDivideScalar(obj, obj.w);

	Vec2f result = V2fInit(obj.x, obj.y);
	return result;
}

fpl_force_inline Vec2i V2iProject(const Vec2f worldPos, const Mat4f mvp, const Viewport4i viewport) {
	// Convert world position to homogeneous clip space
	Vec4f world4 = V4fInit(worldPos.x, worldPos.y, 0.0f, 1.0f);
	Vec4f clip = V4fMultM4f(mvp, world4);

	// Perspective divide -> NDC (-1 to +1)
	Vec4f ndc = V4fDivideScalar(clip, clip.w);

	// Convert NDC -> linear viewport space (0 to 1): ndc * 0.5 + 0.5
	Vec2f linear = V2fInit(ndc.x * 0.5f + 0.5f, ndc.y * 0.5f + 0.5f);

	// Convert linear -> screen coordinates inside viewport (rounded to nearest pixel)
	int screenX = (int)(viewport.x + linear.x * viewport.w + 0.5f);
	int screenY = (int)(viewport.y + linear.y * viewport.h + 0.5f);

	return V2iInit(screenX, screenY);
}

fpl_force_inline Viewport4i VP4iComputeByAspect(const Vec2i screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2iInit(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2iInit(0, 0);
	if (targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport4i result = VP4iInit(viewOffset.x, viewOffset.y, viewSize.w, viewSize.h);
	return(result);
}

fpl_force_inline Viewport4f VP4fComputeByAspect(const Vec2f screenSize, const float targetAspect) {
	float targetHeight = screenSize.w / targetAspect;
	Vec2f viewSize = V2fInit(screenSize.w, screenSize.h);
	Vec2f viewOffset = V2fInit(0, 0);
	if (targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = screenSize.h * targetAspect;
		viewOffset.x = (screenSize.w - viewSize.w) * 0.5f;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = screenSize.w / targetAspect;
		viewOffset.y = (screenSize.h - viewSize.h) * 0.5f;
	}
	Viewport4f result = VP4fInit(viewOffset.x, viewOffset.y, viewSize.w, viewSize.h);
	return(result);
}

//
// Mat3f (column-major, r[col][row])
//
typedef union Mat3f {
	struct {
		Vec3f col1;
		Vec3f col2;
		Vec3f col3;
	};
	struct {
		float r[3][3];
	};
	float m[9];
} Mat3f;

#define M3F(c1, c2, c3) {c1, c2, c3}
#define M3FArg(m) (Mat3f)m

fpl_force_inline Mat3f M3fInit(const float value) {
	Mat3f result;
	result.col1 = V3fInit(value, 0.0f, 0.0f);
	result.col2 = V3fInit(0.0f, value, 0.0f);
	result.col3 = V3fInit(0.0f, 0.0f, value);
	return(result);
}

fpl_force_inline Mat3f M3fIdentity() {
	return M3fInit(1.0f);
}

fpl_force_inline Mat3f M3fCopy(const Mat3f other) {
	Mat3f result = fplStructInit(Mat3f, other.col1, other.col2, other.col3);
	return(result);
}

#if defined(__cplusplus)
fpl_force_inline Mat3f M3f() {
	return M3fIdentity();
}
fpl_force_inline Mat3f M3f(const float value) {
	return M3fInit(value);
}
fpl_force_inline Mat3f M3f(const Mat3f &other) {
	return M3fCopy(other);
}
#endif

// Upper-left 3x3 of a Mat4f (drops translation row/column).
fpl_force_inline Mat3f M3fFromMat4(const Mat4f m) {
	Mat3f result;
	result.col1 = V3fInit(m.r[0][0], m.r[0][1], m.r[0][2]);
	result.col2 = V3fInit(m.r[1][0], m.r[1][1], m.r[1][2]);
	result.col3 = V3fInit(m.r[2][0], m.r[2][1], m.r[2][2]);
	return(result);
}

// Transforms a vector: result = M * v (column-major).
fpl_force_inline Vec3f V3fMultM3f(const Mat3f m, const Vec3f v) {
	Vec3f result;
	result.x = m.r[0][0] * v.x + m.r[1][0] * v.y + m.r[2][0] * v.z;
	result.y = m.r[0][1] * v.x + m.r[1][1] * v.y + m.r[2][1] * v.z;
	result.z = m.r[0][2] * v.x + m.r[1][2] * v.y + m.r[2][2] * v.z;
	return(result);
}

fpl_force_inline Mat3f M3fMult(const Mat3f a, const Mat3f b) {
	Mat3f result;
	result.col1 = V3fMultM3f(a, b.col1);
	result.col2 = V3fMultM3f(a, b.col2);
	result.col3 = V3fMultM3f(a, b.col3);
	return(result);
}

fpl_force_inline Mat3f M3fTranspose(const Mat3f m) {
	Mat3f result;
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 3; ++col) {
			result.r[col][row] = m.r[row][col];
		}
	}
	return(result);
}

// 2D affine helpers (translation in column 3).
fpl_force_inline Mat3f M3fTranslationV2(const Vec2f p) {
	Mat3f result = M3fIdentity();
	result.r[2][0] = p.x;
	result.r[2][1] = p.y;
	return(result);
}

fpl_force_inline Mat3f M3fScaleV2(const Vec2f s) {
	Mat3f result = M3fIdentity();
	result.r[0][0] = s.x;
	result.r[1][1] = s.y;
	return(result);
}

fpl_force_inline Mat3f M3fRotationZ(const float angle) {
	float c = F32Cos(angle);
	float s = F32Sin(angle);
	Mat3f result;
	result.col1 = V3fInit(c, s, 0.0f);
	result.col2 = V3fInit(-s, c, 0.0f);
	result.col3 = V3fInit(0.0f, 0.0f, 1.0f);
	return(result);
}

#endif // FINAL_MATH_H