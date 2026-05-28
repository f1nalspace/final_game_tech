#ifndef VECMATH_H
#define VECMATH_H

// Thin compatibility wrapper over final_math.h.
// Keeps the legacy NBody Vec2/Mat2/Pixel helper names while reusing the shared types/operators.

#include <final_math.h>

const float kDeg2Rad = (float)M_PI / 180.0f;

// Vec2f / Vec3f / Mat4f operators are provided by final_math.h.

// Legacy Vec2f name aliases.
inline float Vec2Dot(const Vec2f &a, const Vec2f &b) {
	return V2fDot(a, b);
}
inline float Vec2Length(const Vec2f &v) {
	return V2fLength(v);
}
inline Vec2f Vec2Normalize(const Vec2f &v) {
	return V2fNormalize(v);
}
inline Vec2f Vec2Hadamard(const Vec2f &a, const Vec2f &b) {
	return V2fHadamard(a, b);
}
inline Vec2f Vec2MultScalar(const Vec2f &v, const float scalar) {
	return V2fMultScalar(v, scalar);
}
inline Vec2f Vec2MultMat2(const Mat2f &A, const Vec2f &v) {
	return V2fMultMat2(A, v);
}
inline float Vec2DistanceSquared(const Vec2f &a, const Vec2f &b) {
	return V2fDistanceSquared(a, b);
}
// Right perpendicular: V x s
inline Vec2f Vec2Cross(const Vec2f &a, float s) {
	return V2fCrossR(a, s);
}
// Left perpendicular: s x V
inline Vec2f Vec2Cross(float s, const Vec2f &a) {
	return V2fCrossL(s, a);
}
inline float Vec2Cross(const Vec2f &a, const Vec2f &b) {
	return V2fCrossZ(a, b);
}
inline float Vec2AxisToAngle(const Vec2f &axis) {
	return V2fAngleFromAxis(axis);
}
inline Vec2f Vec2RandomDirection() {
	return V2fRandomDirection();
}
inline Vec2f Vec2Lerp(const Vec2f &a, float t, const Vec2f &b) {
	return V2fLerp(a, t, b);
}

// Legacy Mat2 aliases.
inline Mat2f Mat2Identity() {
	Mat2f result;
	result.col1 = V2fInit(1, 0);
	result.col2 = V2fInit(0, 1);
	return result;
}
inline Mat2f Mat2FromAngle(float angle) {
	return M2fFromAngle(angle);
}
inline Mat2f Mat2FromAxis(const Vec2f &axis) {
	return M2fFromAxis(axis);
}
inline Mat2f Mat2Transpose(const Mat2f &m) {
	return M2fTranspose(m);
}
inline Mat2f Mat2Mult(const Mat2f &a, const Mat2f &b) {
	return M2fMult(a, b);
}
inline float Mat2ToAngle(const Mat2f &mat) {
	return M2fToAngle(mat);
}
inline Mat2f Mat2MultTranspose(const Mat2f &a, const Mat2f &b) {
	return M2fMultTranspose(a, b);
}

// Legacy scalar lerp.
inline float ScalarLerp(float a, float t, float b) {
	return F32Lerp(a, t, b);
}

// Legacy Pixel / RGBA helpers. final_math's Pixel is BGRA in memory but the .r/.g/.b/.a field names match,
// so all conversions go through named-field access and are byte-order-safe.
inline Pixel RGBA32ToPixel(const uint32_t rgba) {
	Pixel result;
	result.r = (uint8_t)((rgba >> 0) & 0xFF);
	result.g = (uint8_t)((rgba >> 8) & 0xFF);
	result.b = (uint8_t)((rgba >> 16) & 0xFF);
	result.a = (uint8_t)((rgba >> 24) & 0xFF);
	return result;
}

inline uint32_t RGBA32(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}

static const float INV255 = 1.0f / 255.0f;

inline Vec4f PixelToLinear(const Pixel &pixel) {
	return V4fInit(pixel.r * INV255, pixel.g * INV255, pixel.b * INV255, pixel.a * INV255);
}

inline Vec4f RGBA32ToLinear(const uint32_t rgba) {
	return PixelToLinear(RGBA32ToPixel(rgba));
}

inline Vec4f AlphaToLinear(const uint8_t alpha) {
	return V4fInit(1, 1, 1, alpha * INV255);
}

inline uint32_t LinearToRGBA32(const Vec4f &linear) {
	uint8_t r = (uint8_t)((linear.x * 255.0f) + 0.5f);
	uint8_t g = (uint8_t)((linear.y * 255.0f) + 0.5f);
	uint8_t b = (uint8_t)((linear.z * 255.0f) + 0.5f);
	uint8_t a = (uint8_t)((linear.w * 255.0f) + 0.5f);
	return RGBA32(r, g, b, a);
}

static const Vec4f ColorWhite = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
static const Vec4f ColorRed = V4fInit(1.0f, 0.0f, 0.0f, 1.0f);
static const Vec4f ColorGreen = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
static const Vec4f ColorBlue = V4fInit(0.0f, 0.0f, 1.0f, 1.0f);
static const Vec4f ColorLightGray = V4fInit(0.3f, 0.3f, 0.3f, 1.0f);
static const Vec4f ColorDarkGray = V4fInit(0.2f, 0.2f, 0.2f, 1.0f);

#endif
