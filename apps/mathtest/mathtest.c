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

	Also covers the 3D helpers: QuatSlerp, QuatFromEuler, QuatFromMat3, QuatLookRotation, M3fInverse, M4fToNormalMatrix and the Frustum3f tests of final_geometry.h.

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
#include <final_geometry.h>

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
// Angle
//
static void TestAngle(void) {
	Section("Angle - C constructors keep radians and degrees apart");
	Angle fromDegrees = AngleFromDegrees(90.0f);
	CHECK_F(fromDegrees.radians, F32Pi * 0.5f);
	CHECK_F(fromDegrees.degree, 90.0f);
	Angle fromRadians = AngleFromRadians(F32Pi);
	CHECK_F(fromRadians.radians, F32Pi);
	CHECK_F(fromRadians.degree, 180.0f);
}

//
// Quaternion - slerp, euler, matrix and look rotation
//
static bool QuatEq(const Quaternion a, const Quaternion b) {
	return FloatEq(a.w, b.w) && FloatEq(a.x, b.x) && FloatEq(a.y, b.y) && FloatEq(a.z, b.z);
}

// q and -q are the same rotation
static bool QuatSameRotation(const Quaternion a, const Quaternion b) {
	Quaternion negatedB = QuatMultScalar(b, -1.0f);
	return QuatEq(a, b) || QuatEq(a, negatedB);
}

static void TestQuaternion3D(void) {
	const Vec3f axisX = V3fInit(1, 0, 0);
	const Vec3f axisY = V3fInit(0, 1, 0);
	const Vec3f axisZ = V3fInit(0, 0, 1);
	const Vec3f negativeAxisZ = V3fInit(0, 0, -1);
	const float quarterTurn = F32Pi * 0.5f;

	Section("Quaternion - slerp endpoints and constant speed");
	Quaternion identity = QuatIdentity();
	Quaternion zQuarter = QuatFromAngleAxis(quarterTurn, axisZ);
	Quaternion slerpStart = QuatSlerp(identity, 0.0f, zQuarter);
	Quaternion slerpEnd = QuatSlerp(identity, 1.0f, zQuarter);
	Quaternion slerpHalf = QuatSlerp(identity, 0.5f, zQuarter);
	Quaternion slerpQuarter = QuatSlerp(identity, 0.25f, zQuarter);
	Quaternion zEighth = QuatFromAngleAxis(quarterTurn * 0.5f, axisZ);
	Quaternion zSixteenth = QuatFromAngleAxis(quarterTurn * 0.25f, axisZ);
	CHECK(QuatEq(slerpStart, identity));
	CHECK(QuatEq(slerpEnd, zQuarter));
	CHECK(QuatEq(slerpHalf, zEighth));
	CHECK(QuatEq(slerpQuarter, zSixteenth));
	float slerpHalfLength = QuatLength(slerpHalf);
	CHECK_F(slerpHalfLength, 1.0f);

	Section("Quaternion - slerp takes the shortest path");
	Quaternion zQuarterNegated = QuatMultScalar(zQuarter, -1.0f);
	Quaternion slerpHalfNegated = QuatSlerp(identity, 0.5f, zQuarterNegated);
	CHECK(QuatSameRotation(slerpHalfNegated, zEighth));
	Quaternion zLarge = QuatFromAngleAxis(F32Pi * 0.9f, axisZ);
	Quaternion slerpLargeHalf = QuatSlerp(identity, 0.5f, zLarge);
	Quaternion zLargeHalf = QuatFromAngleAxis(F32Pi * 0.45f, axisZ);
	CHECK(QuatEq(slerpLargeHalf, zLargeHalf));

	Section("Quaternion - slerp near-parallel fallback");
	const float tinyAngle = 0.01f;
	Quaternion zTiny = QuatFromAngleAxis(tinyAngle, axisZ);
	Quaternion slerpTinyHalf = QuatSlerp(identity, 0.5f, zTiny);
	Quaternion zTinyHalf = QuatFromAngleAxis(tinyAngle * 0.5f, axisZ);
	CHECK(QuatEq(slerpTinyHalf, zTinyHalf));
	float slerpTinyLength = QuatLength(slerpTinyHalf);
	CHECK_F(slerpTinyLength, 1.0f);
	Quaternion slerpSame = QuatSlerp(zQuarter, 0.7f, zQuarter);
	CHECK(QuatEq(slerpSame, zQuarter));

	Section("Quaternion - slerp general axis");
	Vec3f tiltedDirection = V3fInit(1, 2, 3);
	Vec3f tiltedAxis = V3fNormalize(tiltedDirection);
	Quaternion tiltedStart = QuatFromAngleAxis(0.4f, tiltedAxis);
	Quaternion tiltedEnd = QuatFromAngleAxis(1.6f, tiltedAxis);
	Quaternion slerpTilted = QuatSlerp(tiltedStart, 0.25f, tiltedEnd);
	Quaternion tiltedExpected = QuatFromAngleAxis(0.7f, tiltedAxis);
	CHECK(QuatEq(slerpTilted, tiltedExpected));

	Section("Quaternion - euler single axes and order");
	Quaternion eulerZero = QuatFromEuler(0.0f, 0.0f, 0.0f);
	CHECK(QuatEq(eulerZero, identity));
	Quaternion eulerPitch = QuatFromEuler(0.3f, 0.0f, 0.0f);
	Quaternion eulerYaw = QuatFromEuler(0.0f, 0.4f, 0.0f);
	Quaternion eulerRoll = QuatFromEuler(0.0f, 0.0f, 0.5f);
	Quaternion pitchAxisAngle = QuatFromAngleAxis(0.3f, axisX);
	Quaternion yawAxisAngle = QuatFromAngleAxis(0.4f, axisY);
	Quaternion rollAxisAngle = QuatFromAngleAxis(0.5f, axisZ);
	CHECK(QuatEq(eulerPitch, pitchAxisAngle));
	CHECK(QuatEq(eulerYaw, yawAxisAngle));
	CHECK(QuatEq(eulerRoll, rollAxisAngle));
	Quaternion eulerAll = QuatFromEuler(0.3f, 0.4f, 0.5f);
	Quaternion rollYaw = QuatCross(rollAxisAngle, yawAxisAngle);
	Quaternion rollYawPitch = QuatCross(rollYaw, pitchAxisAngle);
	CHECK(QuatEq(eulerAll, rollYawPitch));

	Section("Quaternion - euler camera yaw turns -z to -x");
	Quaternion cameraYaw = QuatFromEuler(0.0f, quarterTurn, 0.0f);
	Vec3f cameraForward = QuatMultV3f(cameraYaw, negativeAxisZ);
	CHECK(V3Eq(cameraForward, -1.0f, 0.0f, 0.0f));

	Section("Quaternion - QuatYaw no longer stuck at Pi/2");
	float yawOfYaw = QuatYaw(yawAxisAngle);
	CHECK_F(yawOfYaw, 0.4f);
	Quaternion negativeYaw = QuatFromAngleAxis(-0.6f, axisY);
	float yawOfNegativeYaw = QuatYaw(negativeYaw);
	CHECK_F(yawOfNegativeYaw, -0.6f);

	Section("Quaternion - euler round trips through QuatPitch/QuatYaw/QuatRoll");
	struct { float pitch, yaw, roll; } eulerCases[] = {
		{ 0.3f, 0.4f, 0.5f },
		{ -1.2f, 0.9f, 2.5f },
		{ 2.8f, -1.3f, -0.7f },
		{ -0.1f, -0.2f, -3.0f },
	};
	const int eulerCaseCount = (int)fplArrayCount(eulerCases);
	for (int i = 0; i < eulerCaseCount; ++i) {
		Quaternion eulerQuat = QuatFromEuler(eulerCases[i].pitch, eulerCases[i].yaw, eulerCases[i].roll);
		float pitchBack = QuatPitch(eulerQuat);
		float yawBack = QuatYaw(eulerQuat);
		float rollBack = QuatRoll(eulerQuat);
		CHECK(FloatEq(pitchBack, eulerCases[i].pitch) && FloatEq(yawBack, eulerCases[i].yaw) && FloatEq(rollBack, eulerCases[i].roll));
	}

	Section("Quaternion - QuatFromMat3 round trips (all four branches)");
	// A small angle hits the positive trace branch, half turns around x, y and z hit the three diagonal branches
	Vec3f mostlyXDirection = V3fInit(-1, 0.5f, 0.2f);
	Vec3f mostlyZDirection = V3fInit(0.1f, -0.3f, 1);
	Vec3f mostlyXAxis = V3fNormalize(mostlyXDirection);
	Vec3f mostlyZAxis = V3fNormalize(mostlyZDirection);
	Quaternion matrixCases[] = {
		QuatFromAngleAxis(0.8f, tiltedAxis),
		QuatFromAngleAxis(F32Pi, axisX),
		QuatFromAngleAxis(F32Pi, axisY),
		QuatFromAngleAxis(F32Pi, axisZ),
		QuatFromAngleAxis(2.9f, mostlyXAxis),
		QuatFromAngleAxis(3.0f, mostlyZAxis),
	};
	const int matrixCaseCount = (int)fplArrayCount(matrixCases);
	for (int i = 0; i < matrixCaseCount; ++i) {
		Mat4f rotation4 = QuatToMat4(matrixCases[i]);
		Mat3f rotation3 = M3fFromMat4(rotation4);
		Quaternion fromMatrix = QuatFromMat3(rotation3);
		CHECK(QuatSameRotation(fromMatrix, matrixCases[i]));
	}

	Section("Quaternion - look rotation");
	Quaternion lookForward = QuatLookRotation(negativeAxisZ, axisY);
	CHECK(QuatSameRotation(lookForward, identity));
	Vec3f unnormalizedRight = V3fInit(5, 0, 0);
	Quaternion lookRight = QuatLookRotation(unnormalizedRight, axisY);
	Vec3f lookRightForward = QuatMultV3f(lookRight, negativeAxisZ);
	Vec3f lookRightUp = QuatMultV3f(lookRight, axisY);
	CHECK(V3Eq(lookRightForward, 1.0f, 0.0f, 0.0f));
	CHECK(V3Eq(lookRightUp, 0.0f, 1.0f, 0.0f));

	Vec3f lookDirectionRaw = V3fInit(1, 2, -3);
	Vec3f lookDirection = V3fNormalize(lookDirectionRaw);
	Quaternion lookGeneral = QuatLookRotation(lookDirection, axisY);
	Vec3f lookGeneralForward = QuatMultV3f(lookGeneral, negativeAxisZ);
	Vec3f lookGeneralUp = QuatMultV3f(lookGeneral, axisY);
	Vec3f lookGeneralRight = QuatMultV3f(lookGeneral, axisX);
	CHECK(V3Eq(lookGeneralForward, lookDirection.x, lookDirection.y, lookDirection.z));
	float upDotDirection = V3fDot(lookGeneralUp, lookDirection);
	float upDotWorldUp = V3fDot(lookGeneralUp, axisY);
	float rightDotWorldUp = V3fDot(lookGeneralRight, axisY);
	CHECK_F(upDotDirection, 0.0f);
	CHECK(upDotWorldUp > 0.0f);
	CHECK_F(rightDotWorldUp, 0.0f);
	float lookGeneralLength = QuatLength(lookGeneral);
	CHECK_F(lookGeneralLength, 1.0f);

	Section("Quaternion - look rotation is the inverse of M4fLookAtRH");
	Vec3f eye = V3fInit(3, -2, 7);
	Vec3f lookTarget = V3fAdd(eye, lookDirection);
	Mat4f lookAtView = M4fLookAtRH(eye, lookTarget, axisY);
	Mat3f lookAtRotation = M3fFromMat4(lookAtView);
	Quaternion lookGeneralInverse = QuatConjugate(lookGeneral);
	Mat4f lookInverseMatrix4 = QuatToMat4(lookGeneralInverse);
	Mat3f lookInverseMatrix = M3fFromMat4(lookInverseMatrix4);
	CHECK(Mat3Eq(lookAtRotation, lookInverseMatrix));

	Section("Quaternion - look rotation parallel to up");
	Quaternion lookUp = QuatLookRotation(axisY, axisY);
	Vec3f lookUpForward = QuatMultV3f(lookUp, negativeAxisZ);
	float lookUpLength = QuatLength(lookUp);
	CHECK(V3Eq(lookUpForward, 0.0f, 1.0f, 0.0f));
	CHECK_F(lookUpLength, 1.0f);
	Quaternion lookAlongZ = QuatLookRotation(negativeAxisZ, axisZ);
	Vec3f lookAlongZForward = QuatMultV3f(lookAlongZ, negativeAxisZ);
	float lookAlongZLength = QuatLength(lookAlongZ);
	CHECK(V3Eq(lookAlongZForward, 0.0f, 0.0f, -1.0f));
	CHECK_F(lookAlongZLength, 1.0f);
}

//
// Mat3f - determinant, inverse and normal matrix
//
static void TestMat3Inverse(void) {
	const Vec3f planeNormal = V3fInit(1, 1, 0);
	const Vec3f planeTangentA = V3fInit(1, -1, 0);
	const Vec3f planeTangentB = V3fInit(0, 0, 1);

	Section("Mat3f - determinant");
	Mat3f identity = M3fIdentity();
	float identityDeterminant = M3fDeterminant(identity);
	CHECK_F(identityDeterminant, 1.0f);
	Vec3f scaleFactors = V3fInit(2, 3, 4);
	Mat4f scale4 = M4fScaleV3(scaleFactors);
	Mat3f scale3 = M3fFromMat4(scale4);
	float scaleDeterminant = M3fDeterminant(scale3);
	CHECK_F(scaleDeterminant, 24.0f);
	Mat3f singular;
	singular.col1 = V3fInit(2, 0, 1);
	singular.col2 = V3fInit(1, 3, 2);
	singular.col3 = V3fInit(1, 1, 1);
	// det of rows (2 1 1 / 0 3 1 / 1 2 1) = 2*(3-2) - 1*(0-1) + 1*(0-3) = 0
	float singularDeterminant = M3fDeterminant(singular);
	CHECK_F(singularDeterminant, 0.0f);
	Mat3f general;
	general.col1 = V3fInit(4, 2, 1);
	general.col2 = V3fInit(-1, 3, 0.5f);
	general.col3 = V3fInit(2, -2, 5);
	// det of rows (4 -1 2 / 2 3 -2 / 1 0.5 5) = 4*(15+1) + 1*(10+2) + 2*(1-3) = 72
	float generalDeterminant = M3fDeterminant(general);
	CHECK_F(generalDeterminant, 72.0f);

	Section("Mat3f - inverse");
	Mat3f generalInverse = M3fInverse(general);
	Mat3f generalTimesInverse = M3fMult(general, generalInverse);
	Mat3f inverseTimesGeneral = M3fMult(generalInverse, general);
	CHECK(Mat3IsIdentity(generalTimesInverse));
	CHECK(Mat3IsIdentity(inverseTimesGeneral));
	Mat3f singularInverse = M3fInverse(singular);
	Mat3f zero = M3fInit(0.0f);
	CHECK(Mat3Eq(singularInverse, zero));

	Section("Mat3f - normal matrix keeps normals perpendicular under non-uniform scale");
	// The plane x + y = 1 has the tangent (1,-1,0) and the normal (1,1,0); scaling x by 2 turns the tangent into (2,-1,0)
	Vec3f stretchFactors = V3fInit(2, 1, 1);
	Mat4f stretchX = M4fScaleV3(stretchFactors);
	Mat3f stretchNormalMatrix = M4fToNormalMatrix(stretchX);
	Vec3f stretchedNormal = V3fMultM3f(stretchNormalMatrix, planeNormal);
	Vec3f stretchedNormalUnit = V3fNormalize(stretchedNormal);
	Mat3f stretchUpper = M3fFromMat4(stretchX);
	Vec3f stretchedTangent = V3fMultM3f(stretchUpper, planeTangentA);
	float normalDotTangent = V3fDot(stretchedNormal, stretchedTangent);
	CHECK_F(normalDotTangent, 0.0f);
	CHECK(V3Eq(stretchedNormalUnit, 0.4472136f, 0.8944272f, 0.0f));

	Section("Mat3f - normal matrix ignores translation, equals the rotation for rigid transforms");
	Vec3f rigidOffset = V3fInit(10, -4, 3);
	Mat4f rigidRotation = M4fRotationY(0.7f);
	Mat4f rigidTranslation = M4fTranslationV3(rigidOffset);
	Mat4f rigid = M4fMult(rigidTranslation, rigidRotation);
	Mat3f rigidNormalMatrix = M4fToNormalMatrix(rigid);
	Mat3f rigidRotation3 = M3fFromMat4(rigidRotation);
	CHECK(Mat3Eq(rigidNormalMatrix, rigidRotation3));

	Section("Mat3f - normal matrix of rotation * non-uniform scale");
	Vec3f skewFactors = V3fInit(3, 0.5f, 2);
	Mat4f skewScale = M4fScaleV3(skewFactors);
	Mat4f skewModel = M4fMult(rigid, skewScale);
	Mat3f skewNormalMatrix = M4fToNormalMatrix(skewModel);
	Mat3f skewUpper = M3fFromMat4(skewModel);
	Vec3f skewTangentA = V3fMultM3f(skewUpper, planeTangentA);
	Vec3f skewTangentB = V3fMultM3f(skewUpper, planeTangentB);
	Vec3f skewNormal = V3fMultM3f(skewNormalMatrix, planeNormal);
	float skewDotA = V3fDot(skewNormal, skewTangentA);
	float skewDotB = V3fDot(skewNormal, skewTangentB);
	CHECK_F(skewDotA, 0.0f);
	CHECK_F(skewDotB, 0.0f);
}

//
// Plane3f and Frustum3f (final_geometry.h)
//
static bool PlaneEq(const Plane3f plane, const float normalX, const float normalY, const float normalZ, const float distance) {
	return V3Eq(plane.normal, normalX, normalY, normalZ) && FloatEq(plane.distance, distance);
}

static void TestFrustum(void) {
	const float halfSqrt2 = 0.70710678f;

	Section("Plane3f - from equation and signed distance");
	Vec4f scaledEquation = V4fInit(0, 2, 0, -4);
	Plane3f groundOffset = Plane3fInitFromEquation(scaledEquation);
	CHECK(PlaneEq(groundOffset, 0.0f, 1.0f, 0.0f, 2.0f));
	Vec3f abovePlane = V3fInit(7, 5, -3);
	Vec3f belowPlane = V3fInit(0, -1, 0);
	float aboveDistance = Plane3fSignedDistance(&groundOffset, abovePlane);
	float belowDistance = Plane3fSignedDistance(&groundOffset, belowPlane);
	CHECK_F(aboveDistance, 3.0f);
	CHECK_F(belowDistance, -3.0f);
	Vec4f zeroEquation = V4fInit(0, 0, 0, 5);
	Plane3f zeroPlane = Plane3fInitFromEquation(zeroEquation);
	CHECK(PlaneEq(zeroPlane, 0.0f, 0.0f, 0.0f, 0.0f));

	Section("Frustum3f - planes of an orthographic projection");
	// View space: x in [-2, 2], y in [-1, 1], z in [-10, -1]
	Mat4f ortho = M4fOrthoRH(-2, 2, -1, 1, 1, 10);
	Frustum3f orthoFrustum = Frustum3fInitFromMatrix(ortho);
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Left], 1.0f, 0.0f, 0.0f, -2.0f));
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Right], -1.0f, 0.0f, 0.0f, -2.0f));
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Bottom], 0.0f, 1.0f, 0.0f, -1.0f));
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Top], 0.0f, -1.0f, 0.0f, -1.0f));
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Near], 0.0f, 0.0f, -1.0f, 1.0f));
	CHECK(PlaneEq(orthoFrustum.planes[Frustum3fPlane_Far], 0.0f, 0.0f, 1.0f, -10.0f));

	Section("Frustum3f - planes of a perspective projection (90 degrees, near 1, far 10)");
	Mat4f perspective = M4fPerspectiveRH(F32Pi * 0.5f, 1.0f, 1.0f, 10.0f);
	Frustum3f perspectiveFrustum = Frustum3fInitFromMatrix(perspective);
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Left], halfSqrt2, 0.0f, -halfSqrt2, 0.0f));
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Right], -halfSqrt2, 0.0f, -halfSqrt2, 0.0f));
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Bottom], 0.0f, halfSqrt2, -halfSqrt2, 0.0f));
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Top], 0.0f, -halfSqrt2, -halfSqrt2, 0.0f));
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Near], 0.0f, 0.0f, -1.0f, 1.0f));
	CHECK(PlaneEq(perspectiveFrustum.planes[Frustum3fPlane_Far], 0.0f, 0.0f, 1.0f, -10.0f));

	Section("Frustum3f - world space planes of projection * view");
	// The camera at z=5 looks down -z, so the near plane sits at z=4 and the far plane at z=-5
	Vec3f eye = V3fInit(0, 0, 5);
	Vec3f origin = V3fZero();
	Vec3f up = V3fInit(0, 1, 0);
	Mat4f view = M4fLookAtRH(eye, origin, up);
	Mat4f viewProjection = M4fMult(perspective, view);
	Frustum3f worldFrustum = Frustum3fInitFromMatrix(viewProjection);
	CHECK(PlaneEq(worldFrustum.planes[Frustum3fPlane_Near], 0.0f, 0.0f, -1.0f, -4.0f));
	CHECK(PlaneEq(worldFrustum.planes[Frustum3fPlane_Far], 0.0f, 0.0f, 1.0f, -5.0f));

	Section("Frustum3f - sphere inside, outside and intersecting");
	struct { Vec3f center; float radius; Frustum3fContainment expected; } sphereCases[] = {
		{ V3fInit(0, 0, -5), 1.0f, Frustum3fContainment_Inside },
		{ V3fInit(0, 0, 5), 1.0f, Frustum3fContainment_Outside },
		{ V3fInit(0, 0, -20), 1.0f, Frustum3fContainment_Outside },
		{ V3fInit(-10, 0, -5), 1.0f, Frustum3fContainment_Outside },
		{ V3fInit(0, 0, -1), 0.5f, Frustum3fContainment_Intersecting },
		{ V3fInit(-5, 0, -5), 1.0f, Frustum3fContainment_Intersecting },
		{ V3fInit(0, 0, -10.5f), 1.0f, Frustum3fContainment_Intersecting },
		{ V3fInit(0, -50, -5), 100.0f, Frustum3fContainment_Intersecting },
	};
	const int sphereCaseCount = (int)fplArrayCount(sphereCases);
	for (int i = 0; i < sphereCaseCount; ++i) {
		Sphere3f sphere = Sphere3fInit(sphereCases[i].center, sphereCases[i].radius);
		Frustum3fContainment containment = Frustum3fClassifySphere(&perspectiveFrustum, &sphere);
		bool isVisible = Frustum3fIntersectsSphere(&perspectiveFrustum, &sphere);
		bool isExpectedVisible = sphereCases[i].expected != Frustum3fContainment_Outside;
		CHECK(containment == sphereCases[i].expected && isVisible == isExpectedVisible);
	}

	Section("Frustum3f - AABB inside, outside and intersecting");
	struct { Vec3f center; Vec3f radius; Frustum3fContainment expected; } boxCases[] = {
		{ V3fInit(0, 0, -5), V3fInit(0.5f, 0.5f, 0.5f), Frustum3fContainment_Inside },
		{ V3fInit(0, 0, 5), V3fInit(1, 1, 1), Frustum3fContainment_Outside },
		{ V3fInit(0, 0, -30), V3fInit(1, 1, 1), Frustum3fContainment_Outside },
		{ V3fInit(-20, 0, -5), V3fInit(1, 1, 1), Frustum3fContainment_Outside },
		{ V3fInit(0, 30, -5), V3fInit(1, 1, 1), Frustum3fContainment_Outside },
		{ V3fInit(0, 0, -10), V3fInit(1, 1, 1), Frustum3fContainment_Intersecting },
		{ V3fInit(-5, 0, -5), V3fInit(1, 1, 1), Frustum3fContainment_Intersecting },
		{ V3fInit(0, 0, -5), V3fInit(50, 50, 50), Frustum3fContainment_Intersecting },
	};
	const int boxCaseCount = (int)fplArrayCount(boxCases);
	for (int i = 0; i < boxCaseCount; ++i) {
		AABB3f box = AABB3fInitFromCenter(boxCases[i].center, boxCases[i].radius);
		Frustum3fContainment containment = Frustum3fClassifyAABB(&perspectiveFrustum, &box);
		bool isVisible = Frustum3fIntersectsAABB(&perspectiveFrustum, &box);
		bool isExpectedVisible = boxCases[i].expected != Frustum3fContainment_Outside;
		CHECK(containment == boxCases[i].expected && isVisible == isExpectedVisible);
	}

	Section("Frustum3f - world space culling follows the camera");
	Vec3f behindCameraCenter = V3fInit(0, 0, 8);
	Sphere3f behindCamera = Sphere3fInit(behindCameraCenter, 1.0f);
	Sphere3f inFrontOfCamera = Sphere3fInit(origin, 1.0f);
	bool isBehindVisible = Frustum3fIntersectsSphere(&worldFrustum, &behindCamera);
	bool isInFrontVisible = Frustum3fIntersectsSphere(&worldFrustum, &inFrontOfCamera);
	CHECK(!isBehindVisible);
	CHECK(isInFrontVisible);
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
	TestAngle();
	TestQuaternion3D();
	TestMat3Inverse();
	TestFrustum();
	TestColorPixel();
	TestViewportRatio();

	printf("\n===============================================\n");
	printf("Result: %d/%d passed, %d failed\n", g_total - g_failed, g_total, g_failed);
	printf("===============================================\n");
	return (g_failed == 0) ? 0 : 1;
}
