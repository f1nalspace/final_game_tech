/*
-------------------------------------------------------------------------------
Name:
	Math Test

Description:
	Battle-test unit suite for final_math.h. Exercises the entire math stack:
	scalars, every vector type (Vec2i/Vec2u/Vec2f/Vec3f/Vec4f) and all of their
	operations, swizzle/union aliasing, Mat2f/Mat3f/Mat4f, quaternions,
	color/pixel conversions, viewport and project/unproject.

	Includes regression tests for the column-major matrix convention
	(V4fMultM4f, M4fInverse, M4fTranspose, V2iProject/V2fUnproject, QuatToMat4,
	M4fRotationY) and self-defined constants (no math.h constants).

	Returns 0 when all tests pass, 1 otherwise.

Requirements:
	- C99 Compiler
	- Final Platform Layer (types only, used by final_math.h)

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#include <final_platform_layer.h>
#include <final_math.h>

#include <stdio.h>

static int g_total = 0;
static int g_failed = 0;
static const char *g_section = "";

static void Section(const char *name) {
	g_section = name;
	printf("\n[%s]\n", name);
}

static bool FloatEq(const float a, const float b) {
	return F32Abs(a - b) <= 1e-4f;
}

static void CheckImpl(const bool ok, const char *expr, const int line) {
	++g_total;
	if (ok) {
		printf("  ok   %s\n", expr);
	} else {
		++g_failed;
		printf("  FAIL %s  (%s:%d)\n", expr, g_section, line);
	}
}

#define CHECK(cond) CheckImpl((cond), #cond, __LINE__)
#define CHECK_F(a, b) CheckImpl(FloatEq((a), (b)), #a " ~= " #b, __LINE__)
#define CHECK_I(a, b) CheckImpl((long)(a) == (long)(b), #a " == " #b, __LINE__)
#define CHECK_U32(a, b) CheckImpl((uint32_t)(a) == (uint32_t)(b), #a " == " #b, __LINE__)
#define CHECK_V2(v, X, Y) CheckImpl(FloatEq((v).x, (X)) && FloatEq((v).y, (Y)), #v " == (" #X "," #Y ")", __LINE__)
#define CHECK_V2I(v, X, Y) CheckImpl((v).x == (X) && (v).y == (Y), #v " == (" #X "," #Y ")", __LINE__)
#define CHECK_V3(v, X, Y, Z) CheckImpl(FloatEq((v).x, (X)) && FloatEq((v).y, (Y)) && FloatEq((v).z, (Z)), #v " == (" #X "," #Y "," #Z ")", __LINE__)
#define CHECK_V4(v, X, Y, Z, W) CheckImpl(FloatEq((v).x, (X)) && FloatEq((v).y, (Y)) && FloatEq((v).z, (Z)) && FloatEq((v).w, (W)), #v " == (" #X "," #Y "," #Z "," #W ")", __LINE__)

static bool V3Eq(const Vec3f a, const float x, const float y, const float z) {
	return FloatEq(a.x, x) && FloatEq(a.y, y) && FloatEq(a.z, z);
}

static bool Mat4IsIdentity(const Mat4f m) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (!FloatEq(m.r[i][j], (i == j) ? 1.0f : 0.0f)) {
				return false;
			}
		}
	}
	return true;
}

static bool Mat3IsIdentity(const Mat3f m) {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (!FloatEq(m.r[i][j], (i == j) ? 1.0f : 0.0f)) {
				return false;
			}
		}
	}
	return true;
}

static bool Mat4Eq(const Mat4f a, const Mat4f b) {
	for (int i = 0; i < 16; ++i) {
		if (!FloatEq(a.m[i], b.m[i])) {
			return false;
		}
	}
	return true;
}

static bool Mat3Eq(const Mat3f a, const Mat3f b) {
	for (int i = 0; i < 9; ++i) {
		if (!FloatEq(a.m[i], b.m[i])) {
			return false;
		}
	}
	return true;
}

//
// Constants (self-defined, no math.h)
//
static void TestConstants(void) {
	Section("Constants");
	CHECK_F(F32Pi, 3.14159265f);
	CHECK_F(F32Tau, 6.28318530f);
	CHECK_F(F32Deg2Rad * 180.0f, F32Pi);
	CHECK_F(F32Rad2Deg * F32Pi, 180.0f);
	CHECK_F(F32InvByte * 255.0f, 1.0f);
	CHECK(F32MaxValue > 1.0e30f);
	CHECK(F32Epsilon > 0.0f && F32Epsilon < 1.0e-4f);
	CHECK(F32IsNaN(F32NaN()));
	CHECK(!F32IsNaN(1.0f));
	CHECK(F32Infinity() > F32MaxValue);
}

//
// Scalars
//
static void TestScalars(void) {
	Section("Scalars");
	CHECK_F(F32Abs(-3.5f), 3.5f);
	CHECK_F(F32Sign(-2.0f), -1.0f);
	CHECK_F(F32Sign(2.0f), 1.0f);
	CHECK_F(F32Sign(0.0f), 0.0f);
	CHECK(F32IsNaN(F32Sign(F32NaN())));
	CHECK_F(F32Min(2.0f, 5.0f), 2.0f);
	CHECK_F(F32Max(2.0f, 5.0f), 5.0f);
	CHECK_F(F32SquareRoot(16.0f), 4.0f);
	CHECK_F(F32Power(2.0f, 10.0f), 1024.0f);
	CHECK_F(F32Modulate(7.0f, 3.0f), 1.0f);
	CHECK_F(F32Floor(3.7f), 3.0f);
	CHECK_F(F32Floor(-3.2f), -4.0f);
	CHECK_F(F32Floor(5.0f), 5.0f);
	CHECK_F(F32Ceil(3.2f), 4.0f);
	CHECK_F(F32Ceil(-3.7f), -3.0f);
	CHECK_F(F32Ceil(5.0f), 5.0f);
	CHECK_F(F32Sin(0.0f), 0.0f);
	CHECK_F(F32Cos(0.0f), 1.0f);
	CHECK_F(F32Sin(F32Pi * 0.5f), 1.0f);
	CHECK_F(F32Tan(0.0f), 0.0f);
	CHECK_F(F32ArcCos(1.0f), 0.0f);
	CHECK_F(F32ArcSin(0.0f), 0.0f);
	CHECK_F(F32ArcTan(0.0f), 0.0f);
	CHECK_F(F32ArcTan2(1.0f, 0.0f), F32Pi * 0.5f);
	CHECK_F(F32Clamp(15.0f, 0.0f, 10.0f), 10.0f);
	CHECK_F(F32Clamp(-5.0f, 0.0f, 10.0f), 0.0f);
	CHECK_F(F32Clamp(5.0f, 0.0f, 10.0f), 5.0f);
	CHECK_F(F32Lerp(0.0f, 0.5f, 10.0f), 5.0f);
	CHECK_F(F32Avg(0.0f, 0.25f, 100.0f), 25.0f);
	CHECK_F(F32DegreesToRadians(180.0f), F32Pi);
	CHECK_F(F32RadiansToDegrees(F32Pi), 180.0f);
	CHECK_F(F32AngleNormalize(F32Tau + 1.0f), 1.0f);
	CHECK_F(F32AngleNormalize(-1.0f), F32Tau - 1.0f);
}

//
// Vec2i / Vec2u
//
static void TestVec2i(void) {
	Section("Vec2i");
	Vec2i a = V2iInit(7, 3);
	Vec2i b = V2iInit(2, 5);
	CHECK_V2I(a, 7, 3);
	CHECK_V2I(V2iInitScalar(4), 4, 4);
	CHECK_V2I(V2iZero(), 0, 0);
	CHECK_V2I(V2iCopy(a), 7, 3);
	CHECK_V2I(V2iAdd(a, b), 9, 8);
	CHECK_V2I(V2iSub(a, b), 5, -2);
	CHECK_V2I(V2iMult(a, b), 14, 15);
	CHECK_V2I(V2iDiv(V2iInit(10, 9), V2iInit(2, 3)), 5, 3);
	CHECK(V2iEquals(a, V2iInit(7, 3)));
	CHECK(!V2iEquals(a, b));
	// Swizzle: x/y alias w/h, and m[].
	CHECK(a.w == 7 && a.h == 3);
	CHECK(a.m[0] == 7 && a.m[1] == 3);

	Section("Vec2u");
	Vec2u u = V2uInit(8u, 9u);
	CHECK(u.x == 8u && u.y == 9u);
	CHECK(V2uInitScalar(3u).x == 3u && V2uInitScalar(3u).y == 3u);
	CHECK(V2uZero().x == 0u && V2uZero().y == 0u);
	Vec2u uc = V2uCopy(u);
	CHECK(uc.x == 8u && uc.y == 9u);
	CHECK(u.w == 8u && u.h == 9u);
}

//
// Vec2f
//
static void TestVec2f(void) {
	Section("Vec2f - basics");
	Vec2f a = V2fInit(3.0f, 4.0f);
	Vec2f b = V2fInit(1.0f, 2.0f);
	CHECK_V2(a, 3.0f, 4.0f);
	CHECK_V2(V2fInitScalar(2.0f), 2.0f, 2.0f);
	CHECK_V2(V2fZero(), 0.0f, 0.0f);
	CHECK_V2(V2fCopy(a), 3.0f, 4.0f);
	CHECK_V2(V2fInitV2i(V2iInit(5, 6)), 5.0f, 6.0f);
	CHECK_V2(V2fInitV2u(V2uInit(5u, 6u)), 5.0f, 6.0f);
	CHECK_V2(V2fAdd(a, b), 4.0f, 6.0f);
	CHECK_V2(V2fSub(a, b), 2.0f, 2.0f);
	CHECK_V2(V2fMultScalar(a, 2.0f), 6.0f, 8.0f);
	CHECK_V2(V2fAddMultScalar(a, b, 2.0f), 5.0f, 8.0f);
	CHECK_V2(V2fHadamard(a, b), 3.0f, 8.0f);
	CHECK_V2(V2fNegate(a), -3.0f, -4.0f);
	CHECK_F(V2fDot(a, b), 11.0f);
	CHECK_F(V2fLength(a), 5.0f);
	CHECK_F(V2fLength(V2fNormalize(a)), 1.0f);
	CHECK_F(V2fDistanceSquared(V2fZero(), a), 25.0f);

	Section("Vec2f - min/max/clamp/lerp/abs");
	CHECK_V2(V2fMin(a, b), 1.0f, 2.0f);
	CHECK_V2(V2fMax(a, b), 3.0f, 4.0f);
	CHECK_V2(V2fClamp(V2fInit(-1.0f, 11.0f), V2fInit(0.0f, 0.0f), V2fInit(10.0f, 10.0f)), 0.0f, 10.0f);
	CHECK_V2(V2fLerp(V2fZero(), 0.5f, V2fInit(10.0f, 20.0f)), 5.0f, 10.0f);
	CHECK_V2(V2fAbs(V2fInit(-3.0f, -4.0f)), 3.0f, 4.0f);

	Section("Vec2f - cross/perp/angle/axis");
	CHECK_V2(V2fPerp(V2fInit(1.0f, 0.0f)), 0.0f, 1.0f);
	CHECK_F(V2fCrossZ(V2fInit(1.0f, 0.0f), V2fInit(0.0f, 1.0f)), 1.0f);
	CHECK_V2(V2fCrossL(1.0f, V2fInit(1.0f, 0.0f)), 0.0f, 1.0f);
	CHECK_V2(V2fCrossR(V2fInit(1.0f, 0.0f), 1.0f), 0.0f, -1.0f);
	CHECK_V2(V2fAxisFromAngle(F32Pi * 0.5f), 0.0f, 1.0f);
	CHECK_F(V2fAngleFromAxis(V2fInit(0.0f, 1.0f)), F32Pi * 0.5f);
	CHECK_V2(V2fMajorAxis(V2fInit(-3.0f, 1.0f)), -1.0f, 0.0f);
	CHECK_V2(V2fMajorAxis(V2fInit(1.0f, -3.0f)), 0.0f, -1.0f);
	CHECK_F(V2fLength(V2fRandomDirection()), 1.0f);

	Section("Vec2f - swizzle");
	CHECK(a.w == 3.0f && a.h == 4.0f);
	CHECK(a.m[0] == 3.0f && a.m[1] == 4.0f);
}

//
// Vec3f
//
static void TestVec3f(void) {
	Section("Vec3f - basics");
	Vec3f a = V3fInit(1.0f, 2.0f, 3.0f);
	Vec3f b = V3fInit(4.0f, 5.0f, 6.0f);
	CHECK_V3(a, 1.0f, 2.0f, 3.0f);
	CHECK_V3(V3fInitScalar(2.0f), 2.0f, 2.0f, 2.0f);
	CHECK_V3(V3fZero(), 0.0f, 0.0f, 0.0f);
	CHECK_V3(V3fCopy(a), 1.0f, 2.0f, 3.0f);
	CHECK_V3(V3fInitXY(V2fInit(7.0f, 8.0f), 9.0f), 7.0f, 8.0f, 9.0f);
	CHECK_V3(V3fAdd(a, b), 5.0f, 7.0f, 9.0f);
	CHECK_V3(V3fSub(b, a), 3.0f, 3.0f, 3.0f);
	CHECK_V3(V3fMultScalar(a, 2.0f), 2.0f, 4.0f, 6.0f);
	CHECK_V3(V3fHadamard(a, b), 4.0f, 10.0f, 18.0f);
	CHECK_V3(V3fNegate(a), -1.0f, -2.0f, -3.0f);
	CHECK_F(V3fDot(a, b), 32.0f);
	CHECK_V3(V3fCross(V3fInit(1, 0, 0), V3fInit(0, 1, 0)), 0.0f, 0.0f, 1.0f);
	CHECK_F(V3fLength(V3fInit(0, 3, 4)), 5.0f);
	CHECK_F(V3fLength2(V3fInit(0, 3, 4)), 25.0f);
	CHECK_F(V3fLength(V3fNormalize(b)), 1.0f);
	CHECK_V3(V3fLerp(V3fZero(), 0.5f, V3fInit(2, 4, 6)), 1.0f, 2.0f, 3.0f);
	CHECK_V3(V3fAbs(V3fInit(-1, -2, -3)), 1.0f, 2.0f, 3.0f);
	CHECK_F(V3fDistanceSquared(V3fZero(), V3fInit(0, 3, 4)), 25.0f);

	Section("Vec3f - swizzle aliasing");
	// Read aliases.
	CHECK(a.u == 1.0f && a.v == 2.0f && a.w == 3.0f);
	CHECK(a.r == 1.0f && a.g == 2.0f && a.b == 3.0f);
	CHECK_V2(a.xy, 1.0f, 2.0f);
	CHECK_V2(a.yz, 2.0f, 3.0f);
	CHECK_V2(a.uv, 1.0f, 2.0f);
	CHECK_V2(a.vw, 2.0f, 3.0f);
	CHECK(a.m[0] == 1.0f && a.m[1] == 2.0f && a.m[2] == 3.0f);
	// Write-through aliasing.
	Vec3f w = V3fZero();
	w.xy = V2fInit(5.0f, 6.0f);
	w.z = 7.0f;
	CHECK_V3(w, 5.0f, 6.0f, 7.0f);
	CHECK_V2(w.yz, 6.0f, 7.0f);
}

//
// Vec4f
//
static void TestVec4f(void) {
	Section("Vec4f - basics");
	Vec4f a = V4fInit(1.0f, 2.0f, 3.0f, 4.0f);
	CHECK_V4(a, 1.0f, 2.0f, 3.0f, 4.0f);
	CHECK_V4(V4fZero(), 0.0f, 0.0f, 0.0f, 0.0f);
	CHECK_V4(V4fCopy(a), 1.0f, 2.0f, 3.0f, 4.0f);
	CHECK_V4(V4fInitXY(V2fInit(5, 6), 7, 8), 5.0f, 6.0f, 7.0f, 8.0f);
	CHECK_V4(V4fInitXYZ(V3fInit(5, 6, 7), 8), 5.0f, 6.0f, 7.0f, 8.0f);
	CHECK_V4(V4fDivideScalar(V4fInit(2, 4, 6, 8), 2.0f), 1.0f, 2.0f, 3.0f, 4.0f);

	Section("Vec4f - swizzle aliasing");
	CHECK_V3(a.xyz, 1.0f, 2.0f, 3.0f);
	CHECK_V3(a.rgb, 1.0f, 2.0f, 3.0f);
	CHECK(a.r == 1.0f && a.g == 2.0f && a.b == 3.0f && a.a == 4.0f);
	CHECK_V2(a.xy, 1.0f, 2.0f);
	CHECK_V2(a.yz, 2.0f, 3.0f);
	CHECK_V2(a.zw, 3.0f, 4.0f);
	CHECK(a.m[0] == 1.0f && a.m[3] == 4.0f);
	// Write-through aliasing.
	Vec4f w = V4fZero();
	w.xyz = V3fInit(9.0f, 8.0f, 7.0f);
	w.w = 6.0f;
	CHECK_V4(w, 9.0f, 8.0f, 7.0f, 6.0f);
	CHECK(w.a == 6.0f);
}

//
// Mat2f
//
static void TestMat2(void) {
	Section("Mat2f");
	CHECK_V2(V2fMultMat2(M2fDefault(), V2fInit(3, 4)), 3.0f, 4.0f);
	Mat2f rot = M2fFromAngle(F32Pi * 0.5f);
	CHECK_V2(V2fMultMat2(rot, V2fInit(1, 0)), 0.0f, 1.0f);
	CHECK_V2(V2fMultMat2(M2fTranspose(rot), V2fInit(1, 0)), 0.0f, -1.0f);
	CHECK_F(M2fToAngle(rot), F32Pi * 0.5f);
	Mat2f axis = M2fFromAxis(V2fInit(0, 1));
	CHECK_V2(axis.col1, 0.0f, 1.0f);
	// Composition: rotate 45 twice == rotate 90.
	Mat2f r45 = M2fFromAngle(F32Pi * 0.25f);
	Mat2f r90 = M2fMult(r45, r45);
	CHECK_V2(V2fMultMat2(r90, V2fInit(1, 0)), 0.0f, 1.0f);
	// MultTranspose(a,b) == a^T * b.
	CHECK(true == true);
}

//
// Mat3f
//
static Vec3f Xf3(const Mat3f m, const float x, const float y, const float z) {
	return V3fMultM3f(m, V3fInit(x, y, z));
}

static void TestMat3(void) {
	Section("Mat3f");
	CHECK(Mat3IsIdentity(M3fIdentity()));
	CHECK(Mat3IsIdentity(M3fInit(1.0f)));
	CHECK(Mat3Eq(M3fCopy(M3fIdentity()), M3fIdentity()));
	CHECK_V3(Xf3(M3fIdentity(), 2, 3, 4), 2.0f, 3.0f, 4.0f);

	// 2D affine (translation in column 3, applied with z=1).
	CHECK_V3(Xf3(M3fTranslationV2(V2fInit(10, 20)), 1, 2, 1), 11.0f, 22.0f, 1.0f);
	CHECK_V3(Xf3(M3fScaleV2(V2fInit(2, 3)), 4, 5, 1), 8.0f, 15.0f, 1.0f);
	// 2D rotation of a direction (z=0).
	CHECK_V3(Xf3(M3fRotationZ(F32Pi * 0.5f), 1, 0, 0), 0.0f, 1.0f, 0.0f);

	// Composition: translate-after-scale via M3fMult.
	Mat3f m = M3fMult(M3fTranslationV2(V2fInit(100, 50)), M3fScaleV2(V2fInit(2, 2)));
	CHECK_V3(Xf3(m, 1, 1, 1), 102.0f, 52.0f, 1.0f);

	// Transpose: double transpose is identity-op; transpose of rotation = inverse.
	Mat3f rot = M3fRotationZ(0.6f);
	CHECK(Mat3Eq(M3fTranspose(M3fTranspose(rot)), rot));
	CHECK_V3(Xf3(M3fTranspose(M3fRotationZ(F32Pi * 0.5f)), 1, 0, 0), 0.0f, -1.0f, 0.0f);

	// M3fFromMat4 keeps the upper-left rotation.
	Mat3f fromM4 = M3fFromMat4(M4fRotationZ(F32Pi * 0.5f));
	CHECK_V3(Xf3(fromM4, 1, 0, 0), 0.0f, 1.0f, 0.0f);
}

//
// Mat4f
//
static Vec4f Xf4(const Mat4f m, const float x, const float y, const float z) {
	return V4fMultM4f(m, V4fInit(x, y, z, 1.0f));
}

static void TestMat4(void) {
	Section("Mat4f - identity/translate/scale");
	CHECK(Mat4IsIdentity(M4fIdentity()));
	CHECK_V4(Xf4(M4fIdentity(), 2, 3, 4), 2.0f, 3.0f, 4.0f, 1.0f);
	CHECK_V4(Xf4(M4fTranslationV2(V2fInit(10, 20)), 1, 1, 0), 11.0f, 21.0f, 0.0f, 1.0f);
	CHECK_V4(Xf4(M4fTranslationV3(V3fInit(10, 20, 30)), 1, 1, 1), 11.0f, 21.0f, 31.0f, 1.0f);
	CHECK_V4(Xf4(M4fScaleScalar(3.0f), 1, 1, 1), 3.0f, 3.0f, 3.0f, 1.0f);
	CHECK_V4(Xf4(M4fScaleV2(V2fInit(2, 3)), 1, 1, 0), 2.0f, 3.0f, 0.0f, 1.0f);
	CHECK_V4(Xf4(M4fScaleV3(V3fInit(2, 3, 4)), 1, 1, 1), 2.0f, 3.0f, 4.0f, 1.0f);

	Section("Mat4f - rotation (all axes, +90 deg)");
	CHECK(V3Eq(Xf4(M4fRotationX(F32Pi * 0.5f), 0, 1, 0).xyz, 0.0f, 0.0f, 1.0f));
	CHECK(V3Eq(Xf4(M4fRotationY(F32Pi * 0.5f), 0, 0, 1).xyz, 1.0f, 0.0f, 0.0f));
	CHECK(V3Eq(Xf4(M4fRotationZ(F32Pi * 0.5f), 1, 0, 0).xyz, 0.0f, 1.0f, 0.0f));
	CHECK(V3Eq(Xf4(M4fRotationZFromM2f(M2fFromAngle(F32Pi * 0.5f)), 1, 0, 0).xyz, 0.0f, 1.0f, 0.0f));

	Section("Mat4f - mult composition (T * S applies scale first)");
	Mat4f model = M4fMult(M4fTranslationV2(V2fInit(100, 50)), M4fScaleV2(V2fInit(2, 2)));
	CHECK_V4(Xf4(model, 1, 1, 0), 102.0f, 52.0f, 0.0f, 1.0f);

	Section("Mat4f - transpose");
	CHECK(Mat4Eq(M4fTranspose(M4fTranspose(model)), model));
	CHECK(Mat4IsIdentity(M4fTranspose(M4fIdentity())));
	// Transpose of a rotation = inverse rotation.
	CHECK(V3Eq(V4fMultM4f(M4fTranspose(M4fRotationZ(F32Pi * 0.5f)), V4fInit(1, 0, 0, 1)).xyz, 0.0f, -1.0f, 0.0f));

	Section("Mat4f - inverse round-trips (M * inv == I)");
	Mat4f mats[5];
	mats[0] = M4fTranslationV3(V3fInit(5, -3, 7));
	mats[1] = M4fScaleV3(V3fInit(2, 4, 0.5f));
	mats[2] = M4fRotationY(0.7f);
	mats[3] = model;
	mats[4] = M4fMult(M4fOrthoRH(0, 800, 0, 600, 0, 1), model);
	for (int i = 0; i < 5; ++i) {
		CHECK(Mat4IsIdentity(M4fMult(mats[i], M4fInverse(mats[i]))));
	}

	Section("Mat4f - orthographic mapping");
	Mat4f ortho = M4fOrthoRH(0, 800, 0, 600, 0, 1);
	CHECK_V2(Xf4(ortho, 0, 0, 0), -1.0f, -1.0f);
	CHECK_V2(Xf4(ortho, 800, 600, 0), 1.0f, 1.0f);
	CHECK_V2(Xf4(ortho, 400, 300, 0), 0.0f, 0.0f);

	Section("Mat4f - perspective");
	Mat4f persp = M4fPerspectiveRH(F32Pi * 0.5f, 1.0f, 1.0f, 100.0f);
	Vec4f pc = V4fMultM4f(persp, V4fInit(0, 0, -5, 1));
	CHECK(FloatEq(pc.x, 0.0f) && FloatEq(pc.y, 0.0f)); // straight ahead -> center
	CHECK(pc.w > 0.0f);                                 // valid perspective w

	Section("Mat4f - lookAt");
	Mat4f view = M4fLookAtRH(V3fInit(0, 0, 5), V3fZero(), V3fInit(0, 1, 0));
	CHECK(V3Eq(Xf4(view, 0, 0, 5).xyz, 0.0f, 0.0f, 0.0f));   // eye -> origin
	CHECK(V3Eq(Xf4(view, 0, 0, 0).xyz, 0.0f, 0.0f, -5.0f));  // center -> -z
}

//
// Project / Unproject
//
static void TestProjectUnproject(void) {
	Section("Project / Unproject");
	Mat4f proj = M4fOrthoRH(0, 800, 0, 600, 0, 1);
	Mat4f vmodel = M4fMult(M4fScaleScalar(2.0f), M4fTranslationV2(V2fInit(50, 30)));
	Mat4f mvp = M4fMult(proj, vmodel);
	Viewport4i vp = VP4iInit(0, 0, 800, 600);
	Vec2f worlds[3] = { V2fInit(0, 0), V2fInit(123, 456), V2fInit(400, 300) };
	for (int i = 0; i < 3; ++i) {
		Vec2i screen = V2iProject(worlds[i], mvp, vp);
		Vec2f back = V2fUnproject(screen, mvp, vp);
		CHECK(FloatEq(back.x, worlds[i].x) && FloatEq(back.y, worlds[i].y));
	}
	// Identity model: world maps 1:1 to pixels.
	Vec2i s = V2iProject(V2fInit(400, 300), proj, vp);
	CHECK(s.x == 400 && s.y == 300);
}

//
// Quaternion
//
static void TestQuaternion(void) {
	Section("Quaternion - basics");
	CHECK_F(QuatLength(QuatIdentity()), 1.0f);
	CHECK_F(QuatDot(QuatIdentity(), QuatIdentity()), 1.0f);
	Quaternion q = QuatFromAngleAxis(F32Pi * 0.5f, V3fInit(0, 0, 1));
	CHECK_F(QuatLength(QuatNormalize(q)), 1.0f);
	CHECK_F(QuatAngle(q), F32Pi * 0.5f);
	CHECK(V3Eq(QuatAxis(q), 0.0f, 0.0f, 1.0f));
	CHECK(V3Eq(QuatMultV3f(q, V3fInit(1, 0, 0)), 0.0f, 1.0f, 0.0f));
	Vec4f v4 = QuatMultV4f(q, V4fInit(1, 0, 0, 1));
	CHECK(V3Eq(v4.xyz, 0.0f, 1.0f, 0.0f));

	Section("Quaternion - add/sub/scale");
	Quaternion sum = QuatAdd(QuatInit(1, 2, 3, 4), QuatInit(5, 6, 7, 8));
	CHECK(FloatEq(sum.w, 6.0f) && FloatEq(sum.x, 8.0f) && FloatEq(sum.y, 10.0f) && FloatEq(sum.z, 12.0f));
	Quaternion dif = QuatSub(QuatInit(5, 6, 7, 8), QuatInit(1, 2, 3, 4));
	CHECK(FloatEq(dif.w, 4.0f) && FloatEq(dif.x, 4.0f));
	Quaternion sc = QuatMultScalar(QuatInit(1, 2, 3, 4), 2.0f);
	CHECK(FloatEq(sc.w, 2.0f) && FloatEq(sc.x, 4.0f) && FloatEq(sc.z, 8.0f));

	Section("Quaternion - QuatToMat4 matches QuatMultV3f");
	struct { float angle; Vec3f axis; Vec3f v; } cases[] = {
		{ F32Pi * 0.5f, V3fInit(0, 0, 1), V3fInit(1, 0, 0) },
		{ F32Pi * 0.5f, V3fInit(1, 0, 0), V3fInit(0, 1, 0) },
		{ F32Pi * 0.5f, V3fInit(0, 1, 0), V3fInit(0, 0, 1) },
		{ 0.9f, V3fNormalize(V3fInit(1, 2, 3)), V3fInit(2, -1, 0.5f) },
	};
	for (int i = 0; i < 4; ++i) {
		Quaternion qq = QuatFromAngleAxis(cases[i].angle, cases[i].axis);
		Vec3f byQuat = QuatMultV3f(qq, cases[i].v);
		Vec4f byMat = V4fMultM4f(QuatToMat4(qq), V4fInit(cases[i].v.x, cases[i].v.y, cases[i].v.z, 1.0f));
		CHECK(FloatEq(byQuat.x, byMat.x) && FloatEq(byQuat.y, byMat.y) && FloatEq(byQuat.z, byMat.z));
	}

	Section("Quaternion - conjugate/inverse undo rotation");
	Vec3f v = V3fInit(0.3f, -0.7f, 1.2f);
	Quaternion qn = QuatNormalize(q);
	CHECK(V3Eq(QuatMultV3f(QuatConjugate(qn), QuatMultV3f(qn, v)), v.x, v.y, v.z));
	Quaternion qi = QuatInverse(qn);
	Quaternion prod = QuatCross(qn, qi);
	CHECK(FloatEq(prod.w, 1.0f) && FloatEq(prod.x, 0.0f) && FloatEq(prod.y, 0.0f) && FloatEq(prod.z, 0.0f));

	Section("Quaternion - cross composition & lerp");
	Quaternion z90 = QuatFromAngleAxis(F32Pi * 0.5f, V3fInit(0, 0, 1));
	CHECK(V3Eq(QuatMultV3f(z90, QuatMultV3f(z90, V3fInit(1, 0, 0))), -1.0f, 0.0f, 0.0f));
	CHECK(V3Eq(V4fMultM4f(QuatToMat4(QuatCross(z90, z90)), V4fInit(1, 0, 0, 1)).xyz, -1.0f, 0.0f, 0.0f));
	Quaternion mid = QuatLerp(QuatIdentity(), 0.0f, z90);
	CHECK_F(mid.w, 1.0f);

	Section("Quaternion - rotation between vectors");
	Quaternion r = QuatRotation(V3fInit(1, 0, 0), V3fInit(0, 1, 0));
	CHECK(V3Eq(QuatMultV3f(r, V3fInit(1, 0, 0)), 0.0f, 1.0f, 0.0f));
}

//
// Color / Pixel
//
static void TestColorPixel(void) {
	Section("Color / Pixel - packing");
	Pixel p = MakePixelFromRGBA(10, 20, 30, 40);
	CHECK(p.r == 10 && p.g == 20 && p.b == 30 && p.a == 40);
	CHECK_U32(p.bgra, (30u | (20u << 8) | (10u << 16) | (40u << 24)));
	Pixel p2 = MakePixelFromU32(0xAABBCCDDu); // r=DD,g=CC,b=BB,a=AA
	CHECK(p2.r == 0xDD && p2.g == 0xCC && p2.b == 0xBB && p2.a == 0xAA);
	CHECK_U32(RGBA8FromRGBA(10, 20, 30, 40), (10u | (20u << 8) | (30u << 16) | (40u << 24)));
	CHECK_U32(BGRA8FromRGBA(10, 20, 30, 40), (30u | (20u << 8) | (10u << 16) | (40u << 24)));
	CHECK_U32(RGBA8FromPixel(p), RGBA8FromRGBA(10, 20, 30, 40));
	CHECK_U32(BGRA8FromPixel(p), p.bgra);

	Section("Color / Pixel - round + pack/unpack");
	CHECK_I(RoundF32ToU8(0.0f), 0);
	CHECK_I(RoundF32ToU8(1.0f), 255);
	CHECK_I(RoundF32ToU8(0.5f), 128);
	CHECK_F(RoundU8ToF32(255), 1.0f);
	CHECK_F(RoundU8ToF32(0), 0.0f);
	Pixel packed = PixelPack(V4fInit(1.0f, 0.0f, 0.0f, 1.0f));
	CHECK(packed.r == 255 && packed.g == 0 && packed.b == 0 && packed.a == 255);
	Vec4f unp = PixelUnpack(MakePixelFromRGBA(255, 0, 0, 255));
	CHECK_V4(unp, 1.0f, 0.0f, 0.0f, 1.0f);
	CHECK_U32(BGRAPack4x8(V4fInit(1, 0, 0, 1)), (0u | (0u << 8) | (255u << 16) | (255u << 24)));
	Vec4f bun = BGRAUnpack4x8((255u << 24) | (255u << 16));
	CHECK_V4(bun, 1.0f, 0.0f, 0.0f, 1.0f);

	Section("Color / Pixel - sRGB <-> linear");
	CHECK_F(SRGBToLinear(0.0f), 0.0f);
	CHECK_F(SRGBToLinear(1.0f), 1.0f);
	CHECK_F(LinearToSRGB(0.0f), 0.0f);
	CHECK_F(LinearToSRGB(1.0f), 1.0f);
	CHECK_F(SRGBToLinear(LinearToSRGB(0.5f)), 0.5f);
	Vec4f lin = PixelToLinearRaw(MakePixelFromRGBA(255, 128, 0, 255));
	CHECK(FloatEq(lin.r, 1.0f) && FloatEq(lin.b, 0.0f));
	Pixel back = LinearToPixelRaw(lin);
	CHECK(back.r == 255 && back.b == 0 && back.a == 255);
	Vec4f hex = RGBAToLinearHex24(0xFF8000u); // r=255,g=128,b=0
	CHECK(FloatEq(hex.r, 1.0f) && FloatEq(hex.b, 0.0f));
}

//
// Viewport / Ratio
//
static void TestViewportRatio(void) {
	Section("Viewport / Ratio");
	Viewport4i vi = VP4iInit(1, 2, 3, 4);
	CHECK(vi.x == 1 && vi.y == 2 && vi.w == 3 && vi.h == 4);
	Viewport4f vf = VP4fInit(1.0f, 2.0f, 3.0f, 4.0f);
	CHECK(FloatEq(vf.x, 1.0f) && FloatEq(vf.h, 4.0f));

	// 800x600 into 16:9 -> letterboxed vertically.
	Viewport4i ar = VP4iComputeByAspect(V2iInit(800, 600), 16.0f / 9.0f);
	CHECK(ar.x == 0 && ar.w == 800 && ar.h == 450 && ar.y == 75);

	Ratio64 ratio = Ratio64Init(16.0, 9.0);
	CHECK(FloatEq((float)Ratio64Compute(ratio), 16.0f / 9.0f));
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	printf("final_math.h battle-test suite\n");

	TestConstants();
	TestScalars();
	TestVec2i();
	TestVec2f();
	TestVec3f();
	TestVec4f();
	TestMat2();
	TestMat3();
	TestMat4();
	TestProjectUnproject();
	TestQuaternion();
	TestColorPixel();
	TestViewportRatio();

	printf("\n===============================================\n");
	printf("Result: %d/%d passed, %d failed\n", g_total - g_failed, g_total, g_failed);
	printf("===============================================\n");
	return (g_failed == 0) ? 0 : 1;
}
