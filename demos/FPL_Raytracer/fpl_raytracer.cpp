/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Raytracer

Description:
	Multi-threaded progressive CPU path tracer with physically based light transport.
	* Inspired by handmade ray (Casey Muratori)

	Light transport: Unidirectional path tracing with next event estimation, multiple importance sampling (power heuristic) and russian roulette.
	Caustics: Light tracing from the lights through mirrors and glass, connected to the camera; the path tracer skips exactly these paths, so the sum stays unbiased.
	Caustics seen in mirrors or through glass and on sharp lobes: Photon merging with a radius that shrinks every pass (consistent, the bias vanishes over time).
	Path tracing, light tracing and merging are combined with VCM weights (power heuristic over all three strategies).
	Materials: Lambert diffuse, GGX rough conductor with multiple scattering compensation, mirror, smooth glass with absorption, coated plastic, emitters.
	Lights: Sphere and quad area lights, analytic sky with a sampled sun disk, uniform environment.
	Geometry: Spheres, quads, oriented boxes and infinite planes, accelerated by a binned SAH bounding volume hierarchy.
	Output: Progressive accumulation, exposure, ACES tone mapping and sRGB, rendered tile by tile on all CPU cores into the software backbuffer.

	The point of this demo is to show a correct path tracer, multithreading and software video output.
	Every pass renders exactly one sample per pixel, so the image is identical for any thread count.

Controls:
	1-5 = Select scene, 0 = White furnace test scene
	Left mouse drag = Orbit, Right/Middle mouse drag = Pan, Mouse wheel = Zoom
	W/A/S/D/Q/E = Move (hold Shift to move faster), Home/Backspace = Reset camera
	Space = Pause/Resume, R = Restart accumulation
	+/- or PageUp/PageDown = Exposure, T = Tone mapper
	B / Shift+B = Max bounces, M = Integrator mode, C = Caustics (light tracing and photon mapping), F = Firefly clamp, O = Depth of field
	P = Save screenshot (BMP), Shift+P = Save HDR image (PFM), H/F1 = Help, Escape = Quit

Command line:
	--scene N, --width W, --height H, --spp N, --threads N, --bounces N, --seed N
	--exposure EV, --tonemap aces|neutral|none, --integrator mis|nee|bsdf, --clamp on|off, --caustics on|off
	--out file.bmp|file.pfm (renders without a window, writes the file and exits, may be given up to 4 times)
	--time-limit seconds (stops a headless render early)

Requirements:
	- C++/11 Compiler
	- Final Platform Layer
	- final_math.h, final_geometry.h

Author:
	Torsten Spaete

Changelog:
	## 2026-10-05
	- Rewritten as physically based progressive path tracer (NEE, MIS, russian roulette)
	- Light tracing and photon merging for caustics, combined by VCM weights
	- New materials: GGX conductor, mirror, glass, coated plastic, emitters
	- New lights: Sphere and quad area lights, sky with sun, uniform environment
	- New scenes: Cornell box, golden hour, night studio, classic Cornell box, sphere field
	- New shapes: Quad, oriented box
	- BVH, robust ray offsets, stable sphere intersection, per-sample random numbers
	- New job system without races, lost wake-ups or busy waiting
	- Interactive orbit camera with low resolution preview and depth of field
	- Resizable window, scaled blit, ACES tone mapping
	- Headless rendering to BMP/PFM via command line
	- Removed OpenGL preview

	## 2019-08-09
	- Fixed false sharing issues for work queue

	## 2019-06-01
	- Initial version

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#if !defined(FPL_IMPLEMENTATION)
#	define FPL_IMPLEMENTATION
#endif
#if !defined(FPL_NO_AUDIO)
#	define FPL_NO_AUDIO
#endif
#if !defined(FPL_NO_VIDEO_VULKAN)
#	define FPL_NO_VIDEO_VULKAN
#endif
#if !defined(FPL_NO_VIDEO_OPENGL)
#	define FPL_NO_VIDEO_OPENGL
#endif
// Console subsystem on Windows (no /SUBSYSTEM:WINDOWS hint), so the headless mode prints to the calling shell; the window still works
#if !defined(FPL_NO_APPTYPE)
#	define FPL_NO_APPTYPE
#endif
#include <final_platform_layer.h>

#include <final_math.h>
#include <final_geometry.h>

#include <math.h>   // expf, copysignf
#include <stdlib.h> // strtoul, strtod
#include <string.h> // memcpy for float/int bit casts

#include <algorithm> // std::sort
#include <new>       // placement new
#include <vector>

// Custom types to save a bit of typing :D
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef int32_t b32;

static constexpr u32 NoIndex = UINT32_MAX;
static const f32 UnboundedDistance = F32MaxValue;

static const Vec3f UnitUp = V3fInit(0.0f, 0.0f, 1.0f);

//
// Numeric helpers
//
static constexpr f32 LuminanceWeightRed = 0.2126f;
static constexpr f32 LuminanceWeightGreen = 0.7152f;
static constexpr f32 LuminanceWeightBlue = 0.0722f;

static constexpr u32 FloatExponentShift = 23;
static constexpr u32 FloatExponentMask = 0xFFu;

static inline f32 Luminance(const Vec3f &color) {
	f32 result = LuminanceWeightRed * color.r + LuminanceWeightGreen * color.g + LuminanceWeightBlue * color.b;
	return(result);
}

static inline f32 MaxComponent(const Vec3f &value) {
	f32 maxXY = F32Max(value.x, value.y);
	f32 result = F32Max(maxXY, value.z);
	return(result);
}

static inline bool IsBlack(const Vec3f &value) {
	bool result = (value.x <= 0.0f) && (value.y <= 0.0f) && (value.z <= 0.0f);
	return(result);
}

// Component-wise exp(-value), used for Beer-Lambert transmittance
static inline Vec3f V3fExpNegative(const Vec3f &value) {
	f32 x = expf(-value.x);
	f32 y = expf(-value.y);
	f32 z = expf(-value.z);
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

static inline Vec3f FaceTowards(const Vec3f &normal, const Vec3f &direction) {
	f32 side = V3fDot(normal, direction);
	Vec3f result = (side >= 0.0f) ? normal : -normal;
	return(result);
}

// Bit casts via memcpy (union punning is UB in C++, reinterpret_cast violates strict aliasing)
static inline s32 FloatAsInt(const f32 value) {
	s32 result;
	memcpy(&result, &value, sizeof(result));
	return(result);
}

static inline f32 IntAsFloat(const s32 value) {
	f32 result;
	memcpy(&result, &value, sizeof(result));
	return(result);
}

// Bit test, so it still works when someone compiles with -ffast-math (where x != x is optimized away)
static inline bool IsFiniteF32(const f32 value) {
	u32 bits;
	memcpy(&bits, &value, sizeof(bits));
	u32 exponent = (bits >> FloatExponentShift) & FloatExponentMask;
	bool result = exponent != FloatExponentMask;
	return(result);
}

static inline bool IsFiniteV3(const Vec3f &value) {
	bool result = IsFiniteF32(value.x) && IsFiniteF32(value.y) && IsFiniteF32(value.z);
	return(result);
}

//
// Random numbers: PCG32 (O'Neill, XSH-RR), seeded per pixel sample so the image does not depend on which thread rendered which tile
//
struct PathSampler {
	u64 state;
};

static constexpr u64 PcgMultiplier = 6364136223846793005ull;
static constexpr u64 PcgIncrement = 1442695040888963407ull;
static constexpr u32 PcgXorShift = 18;
static constexpr u32 PcgOutputShift = 27;
static constexpr u32 PcgRotationShift = 59;
static constexpr u32 PcgRotationMask = 31;
static constexpr u32 RandomMantissaShift = 8;
static constexpr f32 OneOver2Pow24 = 5.9604644775390625e-08f; // 2^-24, hex float literals are C++17

static constexpr u64 SplitMixIncrement = 0x9E3779B97F4A7C15ull;
static constexpr u64 SplitMixMultiplier1 = 0xBF58476D1CE4E5B9ull;
static constexpr u64 SplitMixMultiplier2 = 0x94D049BB133111EBull;
static constexpr u32 SplitMixShift1 = 30;
static constexpr u32 SplitMixShift2 = 27;
static constexpr u32 SplitMixShift3 = 31;
static constexpr u32 SampleKeyPixelShift = 32;

static inline u32 PathSamplerNextU32(PathSampler &sampler) {
	u64 oldState = sampler.state;
	sampler.state = oldState * PcgMultiplier + PcgIncrement;
	u32 xorShifted = (u32)(((oldState >> PcgXorShift) ^ oldState) >> PcgOutputShift);
	u32 rotation = (u32)(oldState >> PcgRotationShift);
	u32 result = (xorShifted >> rotation) | (xorShifted << ((0u - rotation) & PcgRotationMask));
	return(result);
}

// Uniform in [0, 1 - 2^-24], never returns 1.0, so 1-u > 0 and sqrt(1-u) > 0
static inline f32 PathSamplerNext01(PathSampler &sampler) {
	u32 bits = PathSamplerNextU32(sampler);
	u32 mantissa = bits >> RandomMantissaShift;
	f32 result = (f32)mantissa * OneOver2Pow24;
	return(result);
}

static inline u64 SplitMix64(u64 value) {
	value += SplitMixIncrement;
	value = (value ^ (value >> SplitMixShift1)) * SplitMixMultiplier1;
	value = (value ^ (value >> SplitMixShift2)) * SplitMixMultiplier2;
	u64 result = value ^ (value >> SplitMixShift3);
	return(result);
}

// Seed per (pixel, sample index); the first output is discarded so similar seeds decorrelate
static inline PathSampler MakePathSampler(const u32 pixelIndex, const u32 sampleIndex, const u64 seedSalt) {
	u64 sampleKey = ((u64)pixelIndex << SampleKeyPixelShift) | (u64)sampleIndex;
	u64 mixedKey = SplitMix64(sampleKey);
	PathSampler result;
	result.state = mixedKey ^ seedSalt;
	PathSamplerNextU32(result);
	return(result);
}

static inline u64 MakeSeedSalt(const u32 seed) {
	u64 result = (seed == 0) ? 0 : SplitMix64(seed);
	return(result);
}

//
// Local shading frame (Duff et al. 2017, branchless orthonormal basis)
//
static inline void BuildOrthonormalBasis(const Vec3f &normal, Vec3f &outTangent, Vec3f &outBitangent) {
	f32 sign = copysignf(1.0f, normal.z);
	f32 scale = -1.0f / (sign + normal.z);
	f32 xyScaled = normal.x * normal.y * scale;
	outTangent = V3fInit(1.0f + sign * normal.x * normal.x * scale, sign * xyScaled, -sign * normal.x);
	outBitangent = V3fInit(xyScaled, sign + normal.y * normal.y * scale, -normal.y);
}

struct ShadingFrame {
	Vec3f tangent;
	Vec3f bitangent;
	Vec3f normal;
};

static inline ShadingFrame MakeShadingFrame(const Vec3f &normal) {
	ShadingFrame result;
	result.normal = normal;
	BuildOrthonormalBasis(normal, result.tangent, result.bitangent);
	return(result);
}

static inline Vec3f ToLocal(const ShadingFrame &frame, const Vec3f &value) {
	f32 x = V3fDot(value, frame.tangent);
	f32 y = V3fDot(value, frame.bitangent);
	f32 z = V3fDot(value, frame.normal);
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

static inline Vec3f ToWorld(const ShadingFrame &frame, const Vec3f &value) {
	Vec3f result = value.x * frame.tangent + value.y * frame.bitangent + value.z * frame.normal;
	return(result);
}

//
// Self-intersection: origin offset instead of a t-epsilon (Waechter and Binder, Ray Tracing Gems ch. 6)
//
static constexpr f32 OffsetOriginThreshold = 1.0f / 32.0f; // below this |coordinate| use the float offset
static constexpr f32 OffsetFloatScale = 1.0f / 65536.0f;
static constexpr f32 OffsetIntScale = 256.0f;

static inline f32 OffsetComponent(const f32 position, const f32 normalComponent) {
	s32 integerOffset = (s32)(OffsetIntScale * normalComponent);
	s32 positionBits = FloatAsInt(position);
	s32 signedOffset = (position < 0.0f) ? -integerOffset : integerOffset;
	// Unsigned arithmetic: -0.0 (bits INT_MIN) plus a negative offset would be a signed overflow, the result is unused there anyway
	u32 movedBits = (u32)positionBits + (u32)signedOffset;
	f32 integerOffsetPosition = IntAsFloat((s32)movedBits);
	f32 floatOffsetPosition = position + OffsetFloatScale * normalComponent;
	f32 absolutePosition = F32Abs(position);
	f32 result = (absolutePosition < OffsetOriginThreshold) ? floatOffsetPosition : integerOffsetPosition;
	return(result);
}

// sideNormal: unit geometric normal pointing to the side the new ray travels into
static inline Vec3f OffsetRayOrigin(const Vec3f &position, const Vec3f &sideNormal) {
	f32 x = OffsetComponent(position.x, sideNormal.x);
	f32 y = OffsetComponent(position.y, sideNormal.y);
	f32 z = OffsetComponent(position.z, sideNormal.z);
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

// Spawns a ray leaving a surface point in the given unit direction, the side is chosen by the direction so refraction works
static inline Ray3f SpawnRay(const Vec3f &position, const Vec3f &geometricNormal, const Vec3f &direction) {
	Vec3f sideNormal = FaceTowards(geometricNormal, direction);
	Vec3f origin = OffsetRayOrigin(position, sideNormal);
	Ray3f result = Ray3fInit(origin, direction);
	return(result);
}

//
// Geometry
//
static constexpr f32 RayMinDistance = 0.0f;                 // no t-epsilon, self-intersection is solved by OffsetRayOrigin
static constexpr f32 ParallelDirectionEpsilon = 1e-20f;     // direction components smaller than this are clamped before 1/d
static constexpr f32 RobustSlabScale = 1.0000004f;          // 1 + 2*gamma(3) (Ize 2013), makes the slab test conservative
static const f32 NoHitDistance = F32MaxValue;               // sentinel distance returned by the bounds test on a miss

enum class PrimitiveKind : u32 {
	Sphere = 0,
	Quad,
	Box,
};

struct SphereShape {
	Vec3f center;
	f32 radius;
};

// Parallelogram corner + s * edgeU + t * edgeV with s,t in [0,1], normal = normalize(cross(edgeU, edgeV))
struct QuadShape {
	Vec3f corner;
	Vec3f edgeU;
	Vec3f edgeV;
	Vec3f normal;
	Vec3f dualU;       // alpha = dot(hit - corner, dualU)
	Vec3f dualV;       // beta = dot(hit - corner, dualV)
	f32 planeOffset;   // dot(normal, corner)
	f32 area;
};

// Oriented box: the axes are orthonormal (rotation only), the size lives in halfExtents
struct BoxShape {
	Vec3f center;
	Vec3f halfExtents;
	Vec3f axisX;
	Vec3f axisY;
	Vec3f axisZ;
};

struct Primitive {
	union {
		SphereShape sphere;
		QuadShape quad;
		BoxShape box;
	};
	PrimitiveKind kind;
	u32 materialIndex;
	u32 lightIndex; // index into the light list or NoIndex
};

// Infinite plane: dot(normal, p) == planeOffset, kept outside the BVH because it is unbounded
struct PlanePrimitive {
	Vec3f normal;
	f32 planeOffset;
	u32 materialIndex;
};

struct SurfaceHit {
	Vec3f position;        // reprojected onto the surface
	Vec3f geometricNormal; // unit, outward for closed shapes, the defining normal for planes and quads
	f32 distance;          // t along the unit ray direction
	u32 materialIndex;
	u32 lightIndex;        // index into the light list or NoIndex
	b32 isFrontFace;       // dot(ray.direction, geometricNormal) < 0
};

// 1/value without infinities: avoids 0*inf = NaN in slab tests when the origin lies exactly on a slab plane
static inline f32 SafeReciprocal(const f32 value) {
	f32 absoluteValue = F32Abs(value);
	f32 safeValue = (absoluteValue > ParallelDirectionEpsilon) ? value : copysignf(ParallelDirectionEpsilon, value);
	f32 result = 1.0f / safeValue;
	return(result);
}

static Primitive MakeSpherePrimitive(const Vec3f &center, const f32 radius, const u32 materialIndex) {
	fplAssert(radius > 0.0f);
	Primitive result = {};
	result.kind = PrimitiveKind::Sphere;
	result.materialIndex = materialIndex;
	result.lightIndex = NoIndex;
	result.sphere.center = center;
	result.sphere.radius = radius;
	return(result);
}

// One-sided emitters emit along normalize(cross(edgeU, edgeV)), so choose the edge order to face the scene
static Primitive MakeQuadPrimitive(const Vec3f &corner, const Vec3f &edgeU, const Vec3f &edgeV, const u32 materialIndex) {
	Vec3f crossUV = V3fCross(edgeU, edgeV);
	f32 crossLengthSquared = V3fDot(crossUV, crossUV);
	fplAssert(crossLengthSquared > 0.0f);
	f32 crossLength = F32SquareRoot(crossLengthSquared);
	f32 inverseCrossLengthSquared = 1.0f / crossLengthSquared;
	f32 inverseCrossLength = 1.0f / crossLength;
	Vec3f planeW = inverseCrossLengthSquared * crossUV;
	Primitive result = {};
	result.kind = PrimitiveKind::Quad;
	result.materialIndex = materialIndex;
	result.lightIndex = NoIndex;
	QuadShape &quad = result.quad;
	quad.corner = corner;
	quad.edgeU = edgeU;
	quad.edgeV = edgeV;
	quad.normal = inverseCrossLength * crossUV;
	quad.dualU = V3fCross(edgeV, planeW);
	quad.dualV = V3fCross(planeW, edgeU);
	quad.planeOffset = V3fDot(quad.normal, corner);
	quad.area = crossLength;
	return(result);
}

// Box rotated around the world up axis (z)
static Primitive MakeBoxPrimitive(const Vec3f &center, const Vec3f &halfExtents, const f32 yawRadians, const u32 materialIndex) {
	fplAssert(halfExtents.x > 0.0f && halfExtents.y > 0.0f && halfExtents.z > 0.0f);
	f32 cosine = F32Cos(yawRadians);
	f32 sine = F32Sin(yawRadians);
	Primitive result = {};
	result.kind = PrimitiveKind::Box;
	result.materialIndex = materialIndex;
	result.lightIndex = NoIndex;
	result.box.center = center;
	result.box.halfExtents = halfExtents;
	result.box.axisX = V3fInit(cosine, sine, 0.0f);
	result.box.axisY = V3fInit(-sine, cosine, 0.0f);
	result.box.axisZ = V3fInit(0.0f, 0.0f, 1.0f);
	return(result);
}

//
// Intersection routines: return true only for the nearest root with tMin < t < tMax (the !(t > tMin && t < tMax) form also rejects NaN)
//
// Sphere with the numerically stable form of Ray Tracing Gems ch. 7, the direction must be unit length
static inline bool IntersectSphere(const SphereShape &sphere, const Vec3f &origin, const Vec3f &direction, const f32 tMin, const f32 tMax, f32 &outT) {
	Vec3f centerToOrigin = origin - sphere.center;
	f32 halfB = V3fDot(centerToOrigin, direction);
	Vec3f perpendicular = centerToOrigin - halfB * direction;
	f32 radiusSquared = sphere.radius * sphere.radius;
	f32 perpendicularSquared = V3fDot(perpendicular, perpendicular);
	f32 discriminant = radiusSquared - perpendicularSquared;
	if (discriminant < 0.0f) {
		return(false);
	}
	f32 originDistanceSquared = V3fDot(centerToOrigin, centerToOrigin);
	f32 c = originDistanceSquared - radiusSquared;
	f32 discriminantRoot = F32SquareRoot(discriminant);
	f32 signedRoot = copysignf(discriminantRoot, halfB);
	f32 q = -(halfB + signedRoot);
	f32 rootFromC = c / q; // q == 0 only for a tangent ray starting on the surface: NaN, rejected below
	f32 rootFromQ = q;
	f32 tNear = F32Min(rootFromC, rootFromQ);
	f32 tFar = F32Max(rootFromC, rootFromQ);
	if (tNear > tMin && tNear < tMax) {
		outT = tNear;
		return(true);
	}
	if (tFar > tMin && tFar < tMax) {
		outT = tFar;
		return(true);
	}
	return(false);
}

static inline bool IntersectPlane(const PlanePrimitive &plane, const Vec3f &origin, const Vec3f &direction, const f32 tMin, const f32 tMax, f32 &outT) {
	f32 denominator = V3fDot(plane.normal, direction);
	if (denominator == 0.0f) {
		return(false);
	}
	f32 originDot = V3fDot(plane.normal, origin);
	f32 originDistance = plane.planeOffset - originDot;
	f32 t = originDistance / denominator;
	if (!(t > tMin && t < tMax)) {
		return(false);
	}
	outT = t;
	return(true);
}

// Plane test, then the coordinates inside the parallelogram via precomputed dual vectors; inclusive so adjacent quads overlap on shared edges
static inline bool IntersectQuad(const QuadShape &quad, const Vec3f &origin, const Vec3f &direction, const f32 tMin, const f32 tMax, f32 &outT) {
	f32 denominator = V3fDot(quad.normal, direction);
	if (denominator == 0.0f) {
		return(false);
	}
	f32 originDot = V3fDot(quad.normal, origin);
	f32 originDistance = quad.planeOffset - originDot;
	f32 t = originDistance / denominator;
	if (!(t > tMin && t < tMax)) {
		return(false);
	}
	Vec3f hitPoint = origin + t * direction;
	Vec3f planarHit = hitPoint - quad.corner;
	f32 alpha = V3fDot(planarHit, quad.dualU);
	f32 beta = V3fDot(planarHit, quad.dualV);
	if (alpha < 0.0f || alpha > 1.0f || beta < 0.0f || beta > 1.0f) {
		return(false);
	}
	outT = t;
	return(true);
}

// Slab test in box space; a pure rotation keeps the local direction unit length, so t is the same in both spaces
static inline bool IntersectBox(const BoxShape &box, const Vec3f &origin, const Vec3f &direction, const f32 tMin, const f32 tMax, f32 &outT) {
	Vec3f relativeOrigin = origin - box.center;
	f32 localOriginX = V3fDot(relativeOrigin, box.axisX);
	f32 localOriginY = V3fDot(relativeOrigin, box.axisY);
	f32 localOriginZ = V3fDot(relativeOrigin, box.axisZ);
	f32 localDirectionX = V3fDot(direction, box.axisX);
	f32 localDirectionY = V3fDot(direction, box.axisY);
	f32 localDirectionZ = V3fDot(direction, box.axisZ);
	Vec3f localOrigin = V3fInit(localOriginX, localOriginY, localOriginZ);
	Vec3f localDirection = V3fInit(localDirectionX, localDirectionY, localDirectionZ);
	f32 tEnter = -F32MaxValue;
	f32 tExit = F32MaxValue;
	for (u32 axis = 0; axis < 3; ++axis) {
		f32 inverseDirection = SafeReciprocal(localDirection.m[axis]);
		f32 halfExtent = box.halfExtents.m[axis];
		f32 slabNear = (-halfExtent - localOrigin.m[axis]) * inverseDirection;
		f32 slabFar = (halfExtent - localOrigin.m[axis]) * inverseDirection;
		if (slabNear > slabFar) {
			f32 swapTemp = slabNear;
			slabNear = slabFar;
			slabFar = swapTemp;
		}
		tEnter = F32Max(slabNear, tEnter);
		tExit = F32Min(slabFar, tExit);
	}
	if (tEnter > tExit) {
		return(false);
	}
	if (tEnter > tMin && tEnter < tMax) {
		outT = tEnter;
		return(true);
	}
	if (tExit > tMin && tExit < tMax) {
		outT = tExit;
		return(true);
	}
	return(false);
}

static inline bool IntersectPrimitive(const Primitive &primitive, const Vec3f &origin, const Vec3f &direction, const f32 tMin, const f32 tMax, f32 &outT) {
	switch (primitive.kind) {
		case PrimitiveKind::Sphere:
			return IntersectSphere(primitive.sphere, origin, direction, tMin, tMax, outT);
		case PrimitiveKind::Quad:
			return IntersectQuad(primitive.quad, origin, direction, tMin, tMax, outT);
		case PrimitiveKind::Box:
			return IntersectBox(primitive.box, origin, direction, tMin, tMax, outT);
		default:
			return(false);
	}
}

//
// Surface reconstruction, only for the final hit; positions are reprojected onto the surface so the origin offset stays valid
//
static void ComputePrimitiveSurface(const Primitive &primitive, const Ray3f &ray, const f32 t, SurfaceHit &outHit) {
	Vec3f position = ray.origin + t * ray.direction;
	Vec3f normal = V3fZero();
	switch (primitive.kind) {
		case PrimitiveKind::Sphere:
		{
			const SphereShape &sphere = primitive.sphere;
			Vec3f centerToHit = position - sphere.center;
			normal = V3fNormalize(centerToHit);
			position = sphere.center + sphere.radius * normal;
		} break;

		case PrimitiveKind::Quad:
		{
			const QuadShape &quad = primitive.quad;
			f32 positionDot = V3fDot(quad.normal, position);
			f32 planeError = positionDot - quad.planeOffset;
			normal = quad.normal;
			position = position - planeError * normal;
		} break;

		case PrimitiveKind::Box:
		{
			const BoxShape &box = primitive.box;
			Vec3f relative = position - box.center;
			f32 localX = V3fDot(relative, box.axisX);
			f32 localY = V3fDot(relative, box.axisY);
			f32 localZ = V3fDot(relative, box.axisZ);
			Vec3f local = V3fInit(localX, localY, localZ);
			// The face is the axis where |local| / halfExtent is largest (edges are measure zero)
			u32 faceAxis = 0;
			f32 bestRatio = -1.0f;
			for (u32 axis = 0; axis < 3; ++axis) {
				f32 absoluteLocal = F32Abs(local.m[axis]);
				f32 ratio = absoluteLocal / box.halfExtents.m[axis];
				if (ratio > bestRatio) {
					bestRatio = ratio;
					faceAxis = axis;
				}
			}
			const Vec3f *axes[3] = { &box.axisX, &box.axisY, &box.axisZ };
			f32 faceSign = (local.m[faceAxis] < 0.0f) ? -1.0f : 1.0f;
			normal = faceSign * *axes[faceAxis];
			local.m[faceAxis] = faceSign * box.halfExtents.m[faceAxis];
			position = box.center + local.x * box.axisX + local.y * box.axisY + local.z * box.axisZ;
		} break;

		default:
			break;
	}
	outHit.position = position;
	outHit.geometricNormal = normal;
	outHit.distance = t;
	outHit.materialIndex = primitive.materialIndex;
	outHit.lightIndex = primitive.lightIndex;
}

static void ComputePlaneSurface(const PlanePrimitive &plane, const Ray3f &ray, const f32 t, SurfaceHit &outHit) {
	Vec3f position = ray.origin + t * ray.direction;
	f32 positionDot = V3fDot(plane.normal, position);
	f32 planeError = positionDot - plane.planeOffset;
	outHit.position = position - planeError * plane.normal;
	outHit.geometricNormal = plane.normal;
	outHit.distance = t;
	outHit.materialIndex = plane.materialIndex;
	outHit.lightIndex = NoIndex; // infinite planes can never be lights (unbounded area, not sampleable)
}

//
// Bounding volume hierarchy (binned SAH, 32 byte nodes, sibling pairs adjacent)
//
struct Aabb {
	Vec3f minimum;
	Vec3f maximum;
};

static inline Aabb AabbEmpty() {
	Aabb result;
	result.minimum = V3fInitScalar(F32MaxValue);
	result.maximum = V3fInitScalar(-F32MaxValue);
	return(result);
}

static inline void AabbGrowPoint(Aabb &box, const Vec3f &point) {
	for (u32 axis = 0; axis < 3; ++axis) {
		box.minimum.m[axis] = F32Min(box.minimum.m[axis], point.m[axis]);
		box.maximum.m[axis] = F32Max(box.maximum.m[axis], point.m[axis]);
	}
}

// Union must combine min with min and max with max, growing by the corners of an empty box would push the maximum to +F32MaxValue
static inline void AabbGrowBox(Aabb &box, const Aabb &other) {
	for (u32 axis = 0; axis < 3; ++axis) {
		box.minimum.m[axis] = F32Min(box.minimum.m[axis], other.minimum.m[axis]);
		box.maximum.m[axis] = F32Max(box.maximum.m[axis], other.maximum.m[axis]);
	}
}

static inline f32 AabbHalfSurfaceArea(const Aabb &box) {
	Vec3f extent = box.maximum - box.minimum;
	f32 result = extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
	return(result);
}

static constexpr f32 BoundsPaddingRelative = 1e-5f; // pads flat bounds (axis aligned quads) so the slab test is never degenerate

static Aabb ComputePrimitiveBounds(const Primitive &primitive) {
	Aabb result = AabbEmpty();
	switch (primitive.kind) {
		case PrimitiveKind::Sphere:
		{
			Vec3f radiusVector = V3fInitScalar(primitive.sphere.radius);
			result.minimum = primitive.sphere.center - radiusVector;
			result.maximum = primitive.sphere.center + radiusVector;
		} break;

		case PrimitiveKind::Quad:
		{
			const QuadShape &quad = primitive.quad;
			Vec3f cornerU = quad.corner + quad.edgeU;
			Vec3f cornerV = quad.corner + quad.edgeV;
			Vec3f cornerUV = cornerU + quad.edgeV;
			AabbGrowPoint(result, quad.corner);
			AabbGrowPoint(result, cornerU);
			AabbGrowPoint(result, cornerV);
			AabbGrowPoint(result, cornerUV);
		} break;

		case PrimitiveKind::Box:
		{
			const BoxShape &box = primitive.box;
			Vec3f extent;
			for (u32 axis = 0; axis < 3; ++axis) {
				f32 extentFromX = F32Abs(box.axisX.m[axis]) * box.halfExtents.x;
				f32 extentFromY = F32Abs(box.axisY.m[axis]) * box.halfExtents.y;
				f32 extentFromZ = F32Abs(box.axisZ.m[axis]) * box.halfExtents.z;
				extent.m[axis] = extentFromX + extentFromY + extentFromZ;
			}
			result.minimum = box.center - extent;
			result.maximum = box.center + extent;
		} break;

		default:
			break;
	}
	f32 maxAbsoluteCoordinate = 1.0f;
	for (u32 axis = 0; axis < 3; ++axis) {
		f32 absoluteMinimum = F32Abs(result.minimum.m[axis]);
		f32 absoluteMaximum = F32Abs(result.maximum.m[axis]);
		maxAbsoluteCoordinate = F32Max(absoluteMinimum, maxAbsoluteCoordinate);
		maxAbsoluteCoordinate = F32Max(absoluteMaximum, maxAbsoluteCoordinate);
	}
	Vec3f padding = V3fInitScalar(BoundsPaddingRelative * maxAbsoluteCoordinate);
	result.minimum = result.minimum - padding;
	result.maximum = result.maximum + padding;
	return(result);
}

struct BvhNode {
	Vec3f boundsMin;
	u32 leftOrFirst;    // interior: index of the left child (right = left + 1), leaf: first primitive
	Vec3f boundsMax;
	u32 primitiveCount; // 0 = interior node
};
fplStaticAssert(sizeof(BvhNode) == 32);

static constexpr u32 BvhBinCount = 16;
static constexpr u32 BvhMaxLeafPrimitives = 4;
static constexpr u32 BvhMaxDepth = 48;
static constexpr u32 BvhStackCapacity = 64;
static constexpr u32 BvhFirstChildNodeIndex = 2; // node 1 stays unused so sibling pairs start on even indices (one cache line per pair)
static constexpr f32 BvhTraversalCost = 2.0f;
static constexpr f32 BvhIntersectionCost = 1.0f;

struct BvhBuilder {
	const Aabb *primitiveBounds;
	const Vec3f *primitiveCentroids;
	u32 *primitiveIndices;
	BvhNode *nodes;
	u32 nodeCount;
};

struct BvhBin {
	Aabb bounds;
	u32 count;
};

static void BvhUpdateNodeBounds(BvhBuilder &builder, const u32 nodeIndex) {
	BvhNode &node = builder.nodes[nodeIndex];
	Aabb bounds = AabbEmpty();
	for (u32 offset = 0; offset < node.primitiveCount; ++offset) {
		u32 primitiveIndex = builder.primitiveIndices[node.leftOrFirst + offset];
		AabbGrowBox(bounds, builder.primitiveBounds[primitiveIndex]);
	}
	node.boundsMin = bounds.minimum;
	node.boundsMax = bounds.maximum;
}

// Must be used identically for binning and partitioning, otherwise a float mismatch can produce empty children
static inline u32 BvhBinIndex(const f32 centroid, const f32 centroidMin, const f32 binScale) {
	s32 rawIndex = (s32)((centroid - centroidMin) * binScale);
	s32 lastBin = (s32)BvhBinCount - 1;
	s32 clampedIndex = (rawIndex < 0) ? 0 : ((rawIndex > lastBin) ? lastBin : rawIndex);
	return((u32)clampedIndex);
}

static void BvhSubdivide(BvhBuilder &builder, const u32 nodeIndex, const u32 depth) {
	BvhNode &node = builder.nodes[nodeIndex];
	u32 first = node.leftOrFirst;
	u32 count = node.primitiveCount;
	if (count <= 1 || depth >= BvhMaxDepth) {
		return;
	}

	Aabb centroidBounds = AabbEmpty();
	for (u32 offset = 0; offset < count; ++offset) {
		u32 primitiveIndex = builder.primitiveIndices[first + offset];
		AabbGrowPoint(centroidBounds, builder.primitiveCentroids[primitiveIndex]);
	}

	constexpr u32 splitPlaneCount = BvhBinCount - 1;
	f32 bestCost = F32MaxValue;
	s32 bestAxis = -1;
	u32 bestSplitPlane = 0;
	for (u32 axis = 0; axis < 3; ++axis) {
		f32 centroidMin = centroidBounds.minimum.m[axis];
		f32 centroidMax = centroidBounds.maximum.m[axis];
		if (!(centroidMax > centroidMin)) {
			continue; // all centroids coincide on this axis
		}
		BvhBin bins[BvhBinCount];
		for (u32 binIndex = 0; binIndex < BvhBinCount; ++binIndex) {
			bins[binIndex].bounds = AabbEmpty();
			bins[binIndex].count = 0;
		}
		f32 binScale = (f32)BvhBinCount / (centroidMax - centroidMin);
		for (u32 offset = 0; offset < count; ++offset) {
			u32 primitiveIndex = builder.primitiveIndices[first + offset];
			f32 centroid = builder.primitiveCentroids[primitiveIndex].m[axis];
			u32 binIndex = BvhBinIndex(centroid, centroidMin, binScale);
			bins[binIndex].count++;
			AabbGrowBox(bins[binIndex].bounds, builder.primitiveBounds[primitiveIndex]);
		}
		// Sweep: split plane i separates bins [0..i] from [i+1..BvhBinCount-1]
		f32 leftArea[BvhBinCount - 1];
		f32 rightArea[BvhBinCount - 1];
		u32 leftCount[BvhBinCount - 1];
		u32 rightCount[BvhBinCount - 1];
		Aabb leftBox = AabbEmpty();
		Aabb rightBox = AabbEmpty();
		u32 leftSum = 0;
		u32 rightSum = 0;
		for (u32 planeIndex = 0; planeIndex < splitPlaneCount; ++planeIndex) {
			leftSum += bins[planeIndex].count;
			leftCount[planeIndex] = leftSum;
			AabbGrowBox(leftBox, bins[planeIndex].bounds);
			leftArea[planeIndex] = (leftSum > 0) ? AabbHalfSurfaceArea(leftBox) : 0.0f;
			u32 rightBinIndex = BvhBinCount - 1 - planeIndex;
			u32 rightPlaneIndex = splitPlaneCount - 1 - planeIndex;
			rightSum += bins[rightBinIndex].count;
			rightCount[rightPlaneIndex] = rightSum;
			AabbGrowBox(rightBox, bins[rightBinIndex].bounds);
			rightArea[rightPlaneIndex] = (rightSum > 0) ? AabbHalfSurfaceArea(rightBox) : 0.0f;
		}
		for (u32 planeIndex = 0; planeIndex < splitPlaneCount; ++planeIndex) {
			if (leftCount[planeIndex] == 0 || rightCount[planeIndex] == 0) {
				continue;
			}
			f32 cost = (f32)leftCount[planeIndex] * leftArea[planeIndex] + (f32)rightCount[planeIndex] * rightArea[planeIndex];
			if (cost < bestCost) {
				bestCost = cost;
				bestAxis = (s32)axis;
				bestSplitPlane = planeIndex;
			}
		}
	}
	if (bestAxis < 0) {
		return; // identical centroids: keep as leaf
	}

	Aabb nodeBounds;
	nodeBounds.minimum = node.boundsMin;
	nodeBounds.maximum = node.boundsMax;
	f32 parentArea = AabbHalfSurfaceArea(nodeBounds);
	f32 leafCost = (f32)count * BvhIntersectionCost;
	f32 splitCost = BvhTraversalCost + BvhIntersectionCost * bestCost / parentArea;
	if (splitCost >= leafCost && count <= BvhMaxLeafPrimitives) {
		return;
	}

	// Partition in place with signed cursors (an unsigned right cursor would underflow at 0)
	f32 centroidMin = centroidBounds.minimum.m[bestAxis];
	f32 centroidMax = centroidBounds.maximum.m[bestAxis];
	f32 binScale = (f32)BvhBinCount / (centroidMax - centroidMin);
	s32 leftCursor = (s32)first;
	s32 rightCursor = (s32)(first + count) - 1;
	while (leftCursor <= rightCursor) {
		u32 primitiveIndex = builder.primitiveIndices[leftCursor];
		f32 centroid = builder.primitiveCentroids[primitiveIndex].m[bestAxis];
		u32 binIndex = BvhBinIndex(centroid, centroidMin, binScale);
		if (binIndex <= bestSplitPlane) {
			++leftCursor;
		} else {
			u32 swapTemp = builder.primitiveIndices[leftCursor];
			builder.primitiveIndices[leftCursor] = builder.primitiveIndices[rightCursor];
			builder.primitiveIndices[rightCursor] = swapTemp;
			--rightCursor;
		}
	}
	u32 leftPrimitiveCount = (u32)leftCursor - first;
	if (leftPrimitiveCount == 0 || leftPrimitiveCount == count) {
		return;
	}

	u32 leftChildIndex = builder.nodeCount;
	u32 rightChildIndex = leftChildIndex + 1;
	builder.nodeCount += 2;
	BvhNode &leftChild = builder.nodes[leftChildIndex];
	BvhNode &rightChild = builder.nodes[rightChildIndex];
	leftChild.leftOrFirst = first;
	leftChild.primitiveCount = leftPrimitiveCount;
	rightChild.leftOrFirst = first + leftPrimitiveCount;
	rightChild.primitiveCount = count - leftPrimitiveCount;
	node.leftOrFirst = leftChildIndex;
	node.primitiveCount = 0;
	BvhUpdateNodeBounds(builder, leftChildIndex);
	BvhUpdateNodeBounds(builder, rightChildIndex);
	BvhSubdivide(builder, leftChildIndex, depth + 1);
	BvhSubdivide(builder, rightChildIndex, depth + 1);
}

// Builds the BVH and reorders the primitives into leaf order (leaves reference contiguous ranges), main thread only
static void BuildBvh(std::vector<Primitive> &primitives, std::vector<BvhNode> &outNodes) {
	u32 primitiveCount = (u32)primitives.size();
	outNodes.clear();
	if (primitiveCount == 0) {
		return;
	}
	std::vector<Aabb> bounds(primitiveCount);
	std::vector<Vec3f> centroids(primitiveCount);
	std::vector<u32> indices(primitiveCount);
	for (u32 primitiveIndex = 0; primitiveIndex < primitiveCount; ++primitiveIndex) {
		Aabb primitiveBounds = ComputePrimitiveBounds(primitives[primitiveIndex]);
		Vec3f boundsSum = primitiveBounds.minimum + primitiveBounds.maximum;
		bounds[primitiveIndex] = primitiveBounds;
		centroids[primitiveIndex] = 0.5f * boundsSum;
		indices[primitiveIndex] = primitiveIndex;
	}
	u32 nodeCapacity = 2 * primitiveCount + BvhFirstChildNodeIndex;
	BvhNode emptyNode = {};
	outNodes.assign(nodeCapacity, emptyNode);
	BvhBuilder builder;
	builder.primitiveBounds = bounds.data();
	builder.primitiveCentroids = centroids.data();
	builder.primitiveIndices = indices.data();
	builder.nodes = outNodes.data();
	builder.nodeCount = BvhFirstChildNodeIndex;
	outNodes[0].leftOrFirst = 0;
	outNodes[0].primitiveCount = primitiveCount;
	BvhUpdateNodeBounds(builder, 0);
	BvhSubdivide(builder, 0, 0);
	std::vector<Primitive> ordered(primitiveCount);
	for (u32 orderedIndex = 0; orderedIndex < primitiveCount; ++orderedIndex) {
		ordered[orderedIndex] = primitives[indices[orderedIndex]];
	}
	primitives.swap(ordered);
	outNodes.resize(builder.nodeCount);
}

struct RayQuery {
	Vec3f origin;
	Vec3f direction;
	Vec3f inverseDirection; // computed once per ray, never per node
};

static inline RayQuery MakeRayQuery(const Ray3f &ray) {
	f32 inverseX = SafeReciprocal(ray.direction.x);
	f32 inverseY = SafeReciprocal(ray.direction.y);
	f32 inverseZ = SafeReciprocal(ray.direction.z);
	RayQuery result;
	result.origin = ray.origin;
	result.direction = ray.direction;
	result.inverseDirection = V3fInit(inverseX, inverseY, inverseZ);
	return(result);
}

// Returns the entry distance or NoHitDistance, conservative (RobustSlabScale) so grazing hits are never culled
static inline f32 IntersectNodeBounds(const BvhNode &node, const RayQuery &ray, const f32 tMin, const f32 tMax) {
	f32 tx0 = (node.boundsMin.x - ray.origin.x) * ray.inverseDirection.x;
	f32 tx1 = (node.boundsMax.x - ray.origin.x) * ray.inverseDirection.x;
	f32 ty0 = (node.boundsMin.y - ray.origin.y) * ray.inverseDirection.y;
	f32 ty1 = (node.boundsMax.y - ray.origin.y) * ray.inverseDirection.y;
	f32 tz0 = (node.boundsMin.z - ray.origin.z) * ray.inverseDirection.z;
	f32 tz1 = (node.boundsMax.z - ray.origin.z) * ray.inverseDirection.z;
	f32 nearX = F32Min(tx0, tx1);
	f32 farX = F32Max(tx0, tx1);
	f32 nearY = F32Min(ty0, ty1);
	f32 farY = F32Max(ty0, ty1);
	f32 nearZ = F32Min(tz0, tz1);
	f32 farZ = F32Max(tz0, tz1);
	f32 nearXY = F32Max(nearX, nearY);
	f32 nearZClamped = F32Max(nearZ, tMin);
	f32 tNear = F32Max(nearXY, nearZClamped);
	f32 farXY = F32Min(farX, farY);
	f32 farZClamped = F32Min(farZ, tMax);
	f32 tFarUnscaled = F32Min(farXY, farZClamped);
	f32 tFar = tFarUnscaled * RobustSlabScale;
	f32 result = (tNear <= tFar) ? tNear : NoHitDistance;
	return(result);
}

// Bounding sphere of a specular (mirror or glass) primitive, light tracing aims its photons at these
struct CausticCaster {
	Vec3f center;
	f32 radius;
};

// Immutable while workers render: raw pointers and counts, no std::vector access in the hot loop
struct Material;
struct Light;
struct Environment;

struct SceneView {
	const Primitive *primitives; // BVH leaf order
	u32 primitiveCount;
	const BvhNode *bvhNodes;
	u32 bvhNodeCount;            // 0 when there are no finite primitives
	const PlanePrimitive *planes;
	u32 planeCount;
	const Material *materials;
	u32 materialCount;
	const Light *lights;
	u32 lightCount;
	const f32 *lightCdf;
	const Environment *environment;
	const CausticCaster *casters;
	u32 casterCount;
	f32 casterAreaSum;           // sum of r^2 over all casters
	Vec3f regionCenter;          // bounds of the finite primitives
	f32 regionRadius;
	b32 isLightTracingAvailable; // specular casters and at least one light that light tracing can emit from
};

struct ClosestHit {
	f32 t;
	u32 primitiveIndex; // NoIndex when a plane (or nothing) was hit
	u32 planeIndex;     // NoIndex when a primitive (or nothing) was hit
};

static bool TraceClosest(const SceneView &scene, const Ray3f &ray, const f32 tMax, ClosestHit &outHit) {
	f32 closestT = tMax;
	u32 hitPrimitive = NoIndex;
	u32 hitPlane = NoIndex;
	// Planes first: the floor is hit often, which shrinks tMax early and makes BVH culling more effective
	for (u32 planeIndex = 0; planeIndex < scene.planeCount; ++planeIndex) {
		f32 t;
		if (IntersectPlane(scene.planes[planeIndex], ray.origin, ray.direction, RayMinDistance, closestT, t)) {
			closestT = t;
			hitPlane = planeIndex;
		}
	}
	if (scene.bvhNodeCount > 0) {
		RayQuery query = MakeRayQuery(ray);
		u32 stackNodes[BvhStackCapacity];
		f32 stackDistances[BvhStackCapacity];
		u32 stackSize = 0;
		u32 nodeIndex = 0;
		f32 rootDistance = IntersectNodeBounds(scene.bvhNodes[0], query, RayMinDistance, closestT);
		bool isActive = rootDistance != NoHitDistance;
		while (isActive) {
			const BvhNode &node = scene.bvhNodes[nodeIndex];
			if (node.primitiveCount > 0) {
				u32 primitiveEnd = node.leftOrFirst + node.primitiveCount;
				for (u32 primitiveIndex = node.leftOrFirst; primitiveIndex < primitiveEnd; ++primitiveIndex) {
					f32 t;
					if (IntersectPrimitive(scene.primitives[primitiveIndex], ray.origin, ray.direction, RayMinDistance, closestT, t)) {
						closestT = t;
						hitPrimitive = primitiveIndex;
						hitPlane = NoIndex;
					}
				}
			} else {
				u32 nearChild = node.leftOrFirst;
				u32 farChild = nearChild + 1;
				f32 nearDistance = IntersectNodeBounds(scene.bvhNodes[nearChild], query, RayMinDistance, closestT);
				f32 farDistance = IntersectNodeBounds(scene.bvhNodes[farChild], query, RayMinDistance, closestT);
				if (nearDistance > farDistance) {
					f32 swapDistance = nearDistance;
					nearDistance = farDistance;
					farDistance = swapDistance;
					u32 swapChild = nearChild;
					nearChild = farChild;
					farChild = swapChild;
				}
				if (nearDistance != NoHitDistance) {
					if (farDistance != NoHitDistance) {
						fplAssert(stackSize < BvhStackCapacity);
						stackNodes[stackSize] = farChild;
						stackDistances[stackSize] = farDistance;
						++stackSize;
					}
					nodeIndex = nearChild;
					continue;
				}
			}
			// Pop the next node that can still contain a closer hit
			isActive = false;
			while (stackSize > 0) {
				--stackSize;
				if (stackDistances[stackSize] < closestT) {
					nodeIndex = stackNodes[stackSize];
					isActive = true;
					break;
				}
			}
		}
	}
	outHit.t = closestT;
	outHit.primitiveIndex = hitPrimitive;
	outHit.planeIndex = hitPlane;
	bool result = (hitPrimitive != NoIndex) || (hitPlane != NoIndex);
	return(result);
}

// Any hit in (0, tMax) for shadow rays: no ordering, returns at the first hit.
// The sampled light itself is skipped: the origin offset can push it inside tMax near the light, and a convex or flat emitter cannot occlude its own sampled point.
static bool TraceOccluded(const SceneView &scene, const Ray3f &ray, const f32 tMax, const u32 ignorePrimitiveIndex) {
	for (u32 planeIndex = 0; planeIndex < scene.planeCount; ++planeIndex) {
		f32 t;
		if (IntersectPlane(scene.planes[planeIndex], ray.origin, ray.direction, RayMinDistance, tMax, t)) {
			return(true);
		}
	}
	if (scene.bvhNodeCount == 0) {
		return(false);
	}
	RayQuery query = MakeRayQuery(ray);
	u32 stackNodes[BvhStackCapacity];
	u32 stackSize = 0;
	stackNodes[stackSize++] = 0;
	while (stackSize > 0) {
		u32 nodeIndex = stackNodes[--stackSize];
		const BvhNode &node = scene.bvhNodes[nodeIndex];
		f32 entryDistance = IntersectNodeBounds(node, query, RayMinDistance, tMax);
		if (entryDistance == NoHitDistance) {
			continue;
		}
		if (node.primitiveCount > 0) {
			u32 primitiveEnd = node.leftOrFirst + node.primitiveCount;
			for (u32 primitiveIndex = node.leftOrFirst; primitiveIndex < primitiveEnd; ++primitiveIndex) {
				if (primitiveIndex == ignorePrimitiveIndex) {
					continue;
				}
				f32 t;
				if (IntersectPrimitive(scene.primitives[primitiveIndex], ray.origin, ray.direction, RayMinDistance, tMax, t)) {
					return(true);
				}
			}
		} else {
			fplAssert(stackSize + 2 <= BvhStackCapacity);
			stackNodes[stackSize++] = node.leftOrFirst;
			stackNodes[stackSize++] = node.leftOrFirst + 1;
		}
	}
	return(false);
}

// Closest hit plus surface reconstruction: the main entry point for the integrator
static bool TraceSurface(const SceneView &scene, const Ray3f &ray, const f32 tMax, SurfaceHit &outHit) {
	ClosestHit closest;
	bool isHit = TraceClosest(scene, ray, tMax, closest);
	if (!isHit) {
		return(false);
	}
	if (closest.primitiveIndex != NoIndex) {
		ComputePrimitiveSurface(scene.primitives[closest.primitiveIndex], ray, closest.t, outHit);
	} else {
		ComputePlaneSurface(scene.planes[closest.planeIndex], ray, closest.t, outHit);
	}
	f32 facing = V3fDot(ray.direction, outHit.geometricNormal);
	outHit.isFrontFace = facing < 0.0f;
	return(true);
}

//
// Materials
//
// Conventions: all directions are in the local shading frame whose normal faces the viewer, so wo.z > 0 always holds.
// A BSDF value never includes the cosine, a sample weight is f * |cos(wi)| / pdf.
//
static const f32 InversePi = 1.0f / F32Pi;
static constexpr f32 DeltaAlphaThreshold = 1e-3f;          // conductors with a smaller GGX alpha are perfect mirrors
static constexpr f32 MinPlasticCoatAlpha = 2e-3f;          // plastic coats are always glossy, never delta
static constexpr f32 MinCosine = 1e-6f;
static constexpr f32 SchlickAverageFactor = 1.0f / 21.0f;  // hemispherical average of (1 - cos)^5 weighted by cos
static constexpr f32 PlasticSpecularProbabilityMin = 0.1f;
static constexpr f32 PlasticSpecularProbabilityMax = 0.9f;
static constexpr f32 MinEnergyCompensationDenominator = 1e-4f;
static constexpr b32 UseMultipleScatteringCompensation = true; // Kulla-Conty energy compensation for rough conductors
static constexpr f32 RoughLobeAlphaThreshold = 0.3f;          // lobes below this roughness are sharp: photon merging is offered there as an extra caustic strategy

enum class MaterialKind : u32 {
	Diffuse = 0,
	Conductor,
	Dielectric,
	Plastic,
	Emissive,
};

// Coordinate into a vertex-centered lookup table axis: value = lerp(table[index0], fraction, table[index1])
struct TableCoordinate {
	u32 index0;
	u32 index1;
	f32 fraction;
};

struct Material {
	MaterialKind kind;
	Vec3f baseColor;              // Diffuse albedo, Plastic base albedo, first checker color (linear RGB)
	Vec3f checkerColor;           // second checker color
	f32 checkerSize;
	f32 checkerFadeStart;         // distance from the origin in XY where the checker starts to fade into its average
	f32 checkerFadeEnd;
	b32 hasChecker;
	Vec3f specularColor;          // Conductor: Schlick F0 color
	f32 alpha;                    // GGX alpha (not perceptual roughness)
	b32 isDeltaSpecular;          // Conductor: perfect mirror
	f32 ior;                      // Dielectric: index of refraction, Plastic: coat index of refraction
	f32 coatF0;                   // Plastic: ((ior - 1) / (ior + 1))^2
	Vec3f emission;               // Emissive: radiance
	b32 isTwoSided;               // Emissive: emits on both sides
	Vec3f absorptionCoefficient;  // Dielectric: Beer-Lambert sigma_a per unit length
	b32 hasAbsorption;
	TableCoordinate roughnessCoordinate; // GGX albedo table row for alpha
	f32 averageAlbedo;            // Conductor: average albedo with F0 = 1, Plastic: average coat albedo
	Vec3f multipleScatteringColor; // Conductor: Kulla-Conty Fms / (pi * (1 - averageAlbedo))
	f32 baseNormalization;        // Plastic: 1 / (pi * (1 - averageAlbedo))
};

//
// GGX (Trowbridge-Reitz) with Smith height-correlated masking-shadowing
//
static inline f32 GgxDistribution(const Vec3f &halfVector, const f32 alpha) {
	if (halfVector.z <= 0.0f) {
		return(0.0f);
	}
	f32 alphaSquared = alpha * alpha;
	f32 cosSquared = halfVector.z * halfVector.z;
	f32 denominatorRoot = cosSquared * (alphaSquared - 1.0f) + 1.0f;
	f32 result = alphaSquared / (F32Pi * denominatorRoot * denominatorRoot);
	return(result);
}

// S(w) = |w.z| * sqrt(1 + alpha^2 tan^2 theta)
static inline f32 GgxProjectedRoughness(const Vec3f &direction, const f32 alphaSquared) {
	f32 cosSquared = direction.z * direction.z;
	f32 radicand = cosSquared * (1.0f - alphaSquared) + alphaSquared;
	f32 result = F32SquareRoot(radicand);
	return(result);
}

static inline f32 GgxLambda(const Vec3f &direction, const f32 alphaSquared) {
	f32 projected = GgxProjectedRoughness(direction, alphaSquared);
	f32 absoluteCos = F32Abs(direction.z);
	f32 result = 0.5f * (projected / absoluteCos - 1.0f);
	return(result);
}

static inline f32 GgxMaskingG1(const Vec3f &direction, const f32 alphaSquared) {
	f32 lambda = GgxLambda(direction, alphaSquared);
	f32 result = 1.0f / (1.0f + lambda);
	return(result);
}

static inline f32 GgxMaskingShadowingG2(const Vec3f &wo, const Vec3f &wi, const f32 alphaSquared) {
	f32 lambdaOut = GgxLambda(wo, alphaSquared);
	f32 lambdaIn = GgxLambda(wi, alphaSquared);
	f32 result = 1.0f / (1.0f + lambdaOut + lambdaIn);
	return(result);
}

// Vis = G2 / (4 wo.z wi.z), the algebraically identical cheaper form
static inline f32 GgxVisibility(const Vec3f &wo, const Vec3f &wi, const f32 alphaSquared) {
	f32 projectedOut = GgxProjectedRoughness(wo, alphaSquared);
	f32 projectedIn = GgxProjectedRoughness(wi, alphaSquared);
	f32 result = 0.5f / (projectedOut * wi.z + projectedIn * wo.z);
	return(result);
}

// Solid angle pdf of a reflection sampled with the visible normal distribution: G1(wo) D(h) / (4 wo.z)
static inline f32 GgxReflectionPdf(const Vec3f &wo, const Vec3f &halfVector, const f32 alpha) {
	f32 alphaSquared = alpha * alpha;
	f32 distribution = GgxDistribution(halfVector, alpha);
	f32 masking = GgxMaskingG1(wo, alphaSquared);
	f32 result = masking * distribution / (4.0f * wo.z);
	return(result);
}

static inline f32 SchlickWeight(const f32 cosine) {
	f32 clampedCosine = F32Clamp(cosine, 0.0f, 1.0f);
	f32 oneMinusCosine = 1.0f - clampedCosine;
	f32 squared = oneMinusCosine * oneMinusCosine;
	f32 result = squared * squared * oneMinusCosine;
	return(result);
}

static inline Vec3f SchlickFresnel(const Vec3f &f0, const f32 cosine) {
	f32 weight = SchlickWeight(cosine);
	Vec3f one = V3fInit(1.0f, 1.0f, 1.0f);
	Vec3f result = f0 + weight * (one - f0);
	return(result);
}

static inline f32 SchlickFresnelScalar(const f32 f0, const f32 cosine) {
	f32 weight = SchlickWeight(cosine);
	f32 result = f0 + weight * (1.0f - f0);
	return(result);
}

static inline Vec3f Reflect(const Vec3f &wo, const Vec3f &normal) {
	f32 projection = V3fDot(wo, normal);
	Vec3f result = (2.0f * projection) * normal - wo;
	return(result);
}

// Heitz 2018, "Sampling the GGX Distribution of Visible Normals" (isotropic), requires wo.z > 0, returns the microfacet normal
static Vec3f SampleGgxVisibleNormal(const Vec3f &wo, const f32 alpha, const f32 u0, const f32 u1) {
	Vec3f stretched = V3fInit(alpha * wo.x, alpha * wo.y, wo.z);
	Vec3f hemisphereView = V3fNormalize(stretched);
	f32 lengthSquared = hemisphereView.x * hemisphereView.x + hemisphereView.y * hemisphereView.y;
	Vec3f tangent1 = V3fInit(1.0f, 0.0f, 0.0f);
	if (lengthSquared > 0.0f) {
		f32 inverseLength = 1.0f / F32SquareRoot(lengthSquared);
		tangent1 = V3fInit(-hemisphereView.y * inverseLength, hemisphereView.x * inverseLength, 0.0f);
	}
	Vec3f tangent2 = V3fCross(hemisphereView, tangent1);
	f32 radius = F32SquareRoot(u0);
	f32 phi = F32Tau * u1;
	f32 cosPhi = F32Cos(phi);
	f32 sinPhi = F32Sin(phi);
	f32 p1 = radius * cosPhi;
	f32 p2 = radius * sinPhi;
	f32 blend = 0.5f * (1.0f + hemisphereView.z);
	f32 p1Squared = p1 * p1;
	f32 lowerRoot = F32SquareRoot(F32Max(0.0f, 1.0f - p1Squared));
	p2 = (1.0f - blend) * lowerRoot + blend * p2;
	f32 pzSquared = 1.0f - p1Squared - p2 * p2;
	f32 pz = F32SquareRoot(F32Max(0.0f, pzSquared));
	Vec3f hemisphereNormal = p1 * tangent1 + p2 * tangent2 + pz * hemisphereView;
	Vec3f unstretched = V3fInit(alpha * hemisphereNormal.x, alpha * hemisphereNormal.y, F32Max(0.0f, hemisphereNormal.z));
	Vec3f result = V3fNormalize(unstretched);
	return(result);
}

// Cosine weighted hemisphere (Malley), pdf = wi.z / pi
static inline Vec3f SampleCosineHemisphere(const f32 u0, const f32 u1) {
	f32 radius = F32SquareRoot(u0);
	f32 phi = F32Tau * u1;
	f32 cosPhi = F32Cos(phi);
	f32 sinPhi = F32Sin(phi);
	f32 heightSquared = F32Max(0.0f, 1.0f - u0);
	f32 height = F32SquareRoot(heightSquared);
	Vec3f result = V3fInit(radius * cosPhi, radius * sinPhi, height);
	return(result);
}

//
// Directional albedo tables of the GGX reflection lobe, built once at startup and read-only afterwards
// With Schlick, E(mu; F0) = F0 * fullFresnel(mu) + (1 - F0) * schlickWeight(mu), so two tables cover every F0 and alpha.
// Rows are indexed by sqrt(alpha), columns by mu = cos(theta_o), both vertex-centered so the end points 0 and 1 are covered.
//
static constexpr u32 GgxAlbedoTableSize = 32;
static constexpr u32 GgxAlbedoStrataPerAxis = 32;
static constexpr u64 GgxAlbedoTableSeed = 0x5EEDA1BEDull;
static constexpr f32 GgxAlbedoTableMinAlpha = 1e-4f;
static constexpr f32 GgxAlbedoTableMinCosine = 1e-3f;

struct GgxAlbedoTables {
	f32 fullFresnel[GgxAlbedoTableSize][GgxAlbedoTableSize];   // directional albedo with F = 1
	f32 schlickWeight[GgxAlbedoTableSize][GgxAlbedoTableSize]; // directional albedo with F = (1 - cos)^5
	f32 fullFresnelAverage[GgxAlbedoTableSize];                // 2 * integral E(mu) mu dmu
	f32 schlickWeightAverage[GgxAlbedoTableSize];
	b32 isBuilt;
};

static GgxAlbedoTables GlobalGgxAlbedoTables;

static TableCoordinate MakeTableCoordinate(const f32 unitValue) {
	f32 lastIndex = (f32)(GgxAlbedoTableSize - 1);
	f32 position = unitValue * lastIndex;
	TableCoordinate result;
	if (!(position > 0.0f)) {
		result.index0 = 0;
		result.index1 = 0;
		result.fraction = 0.0f;
	} else if (position >= lastIndex) {
		result.index0 = GgxAlbedoTableSize - 1;
		result.index1 = GgxAlbedoTableSize - 1;
		result.fraction = 0.0f;
	} else {
		u32 index = (u32)position;
		result.index0 = index;
		result.index1 = index + 1;
		result.fraction = position - (f32)index;
	}
	return(result);
}

static inline f32 LookupAlbedoTable(const f32 table[GgxAlbedoTableSize][GgxAlbedoTableSize], const TableCoordinate &roughness, const f32 cosine) {
	TableCoordinate column = MakeTableCoordinate(cosine);
	f32 row0 = F32Lerp(table[roughness.index0][column.index0], column.fraction, table[roughness.index0][column.index1]);
	f32 row1 = F32Lerp(table[roughness.index1][column.index0], column.fraction, table[roughness.index1][column.index1]);
	f32 result = F32Lerp(row0, roughness.fraction, row1);
	return(result);
}

static inline f32 LookupAlbedoAverage(const f32 averages[GgxAlbedoTableSize], const TableCoordinate &roughness) {
	f32 result = F32Lerp(averages[roughness.index0], roughness.fraction, averages[roughness.index1]);
	return(result);
}

// Directional albedo of a GGX lobe with Schlick F0 (scalar)
static inline f32 GgxDirectionalAlbedo(const TableCoordinate &roughness, const f32 f0, const f32 cosine) {
	const GgxAlbedoTables &tables = GlobalGgxAlbedoTables;
	f32 full = LookupAlbedoTable(tables.fullFresnel, roughness, cosine);
	f32 schlick = LookupAlbedoTable(tables.schlickWeight, roughness, cosine);
	f32 result = f0 * full + (1.0f - f0) * schlick;
	return(result);
}

static inline f32 GgxAverageAlbedo(const TableCoordinate &roughness, const f32 f0) {
	const GgxAlbedoTables &tables = GlobalGgxAlbedoTables;
	f32 full = LookupAlbedoAverage(tables.fullFresnelAverage, roughness);
	f32 schlick = LookupAlbedoAverage(tables.schlickWeightAverage, roughness);
	f32 result = f0 * full + (1.0f - f0) * schlick;
	return(result);
}

// Estimates the tables with stratified visible normal sampling and a fixed seed, so every run builds identical tables
static void BuildGgxAlbedoTables(GgxAlbedoTables &tables) {
	PathSampler sampler;
	sampler.state = GgxAlbedoTableSeed;
	constexpr u32 sampleCount = GgxAlbedoStrataPerAxis * GgxAlbedoStrataPerAxis;
	constexpr f32 inverseStrata = 1.0f / (f32)GgxAlbedoStrataPerAxis;
	constexpr f32 inverseSampleCount = 1.0f / (f32)sampleCount;
	for (u32 roughnessIndex = 0; roughnessIndex < GgxAlbedoTableSize; ++roughnessIndex) {
		f32 roughness = (f32)roughnessIndex / (f32)(GgxAlbedoTableSize - 1);
		f32 alpha = F32Max(roughness * roughness, GgxAlbedoTableMinAlpha);
		f32 alphaSquared = alpha * alpha;
		f32 fullAverageSum = 0.0f;
		f32 schlickAverageSum = 0.0f;
		for (u32 cosineIndex = 0; cosineIndex < GgxAlbedoTableSize; ++cosineIndex) {
			f32 nodeCosine = (f32)cosineIndex / (f32)(GgxAlbedoTableSize - 1);
			f32 cosine = F32Max(nodeCosine, GgxAlbedoTableMinCosine);
			f32 sine = F32SquareRoot(1.0f - cosine * cosine);
			Vec3f wo = V3fInit(sine, 0.0f, cosine);
			f32 maskingOut = GgxMaskingG1(wo, alphaSquared);
			f32 fullSum = 0.0f;
			f32 schlickSum = 0.0f;
			for (u32 strataY = 0; strataY < GgxAlbedoStrataPerAxis; ++strataY) {
				for (u32 strataX = 0; strataX < GgxAlbedoStrataPerAxis; ++strataX) {
					f32 jitterX = PathSamplerNext01(sampler);
					f32 jitterY = PathSamplerNext01(sampler);
					f32 u0 = ((f32)strataX + jitterX) * inverseStrata;
					f32 u1 = ((f32)strataY + jitterY) * inverseStrata;
					Vec3f halfVector = SampleGgxVisibleNormal(wo, alpha, u0, u1);
					Vec3f wi = Reflect(wo, halfVector);
					if (wi.z <= 0.0f) {
						continue;
					}
					f32 maskingShadowing = GgxMaskingShadowingG2(wo, wi, alphaSquared);
					f32 weight = maskingShadowing / maskingOut;
					f32 cosineHalf = V3fDot(wo, halfVector);
					f32 schlick = SchlickWeight(cosineHalf);
					fullSum += weight;
					schlickSum += weight * schlick;
				}
			}
			f32 fullAlbedo = fullSum * inverseSampleCount;
			f32 schlickAlbedo = schlickSum * inverseSampleCount;
			tables.fullFresnel[roughnessIndex][cosineIndex] = fullAlbedo;
			tables.schlickWeight[roughnessIndex][cosineIndex] = schlickAlbedo;
			// Trapezoid rule over the nodes for 2 * integral E(mu) mu dmu: the end nodes count half
			bool isEndNode = (cosineIndex == 0) || (cosineIndex == GgxAlbedoTableSize - 1);
			f32 trapezoidWeight = isEndNode ? 0.5f : 1.0f;
			fullAverageSum += trapezoidWeight * 2.0f * nodeCosine * fullAlbedo;
			schlickAverageSum += trapezoidWeight * 2.0f * nodeCosine * schlickAlbedo;
		}
		f32 nodeSpacing = 1.0f / (f32)(GgxAlbedoTableSize - 1);
		tables.fullFresnelAverage[roughnessIndex] = fullAverageSum * nodeSpacing;
		tables.schlickWeightAverage[roughnessIndex] = schlickAverageSum * nodeSpacing;
	}
	tables.isBuilt = true;
}

// Fills the derived fields of a material, requires the albedo tables
static void FinalizeMaterial(Material &material) {
	fplAssert(GlobalGgxAlbedoTables.isBuilt);
	if (material.kind == MaterialKind::Plastic) {
		material.alpha = F32Max(material.alpha, MinPlasticCoatAlpha);
	}
	f32 roughness = F32SquareRoot(material.alpha);
	material.roughnessCoordinate = MakeTableCoordinate(roughness);
	material.isDeltaSpecular = (material.kind == MaterialKind::Conductor) && (material.alpha < DeltaAlphaThreshold);
	if (material.kind == MaterialKind::Conductor && !material.isDeltaSpecular) {
		// Kulla-Conty: Favg = F0 + (1 - F0) / 21, Fms = Favg^2 Eavg / (1 - Favg (1 - Eavg))
		f32 averageAlbedo = GgxAverageAlbedo(material.roughnessCoordinate, 1.0f);
		Vec3f one = V3fInit(1.0f, 1.0f, 1.0f);
		Vec3f averageFresnel = material.specularColor + SchlickAverageFactor * (one - material.specularColor);
		Vec3f averageFresnelSquared = V3fHadamard(averageFresnel, averageFresnel);
		f32 missingEnergy = 1.0f - averageAlbedo;
		Vec3f multipleScattering = V3fZero();
		for (u32 channel = 0; channel < 3; ++channel) {
			f32 denominator = 1.0f - averageFresnel.m[channel] * missingEnergy;
			multipleScattering.m[channel] = averageFresnelSquared.m[channel] * averageAlbedo / denominator;
		}
		material.averageAlbedo = averageAlbedo;
		material.multipleScatteringColor = V3fZero();
		if (UseMultipleScatteringCompensation && missingEnergy > MinEnergyCompensationDenominator) {
			f32 normalization = 1.0f / (F32Pi * missingEnergy);
			material.multipleScatteringColor = normalization * multipleScattering;
		}
	}
	if (material.kind == MaterialKind::Plastic) {
		f32 iorRatio = (material.ior - 1.0f) / (material.ior + 1.0f);
		material.coatF0 = iorRatio * iorRatio;
		material.averageAlbedo = GgxAverageAlbedo(material.roughnessCoordinate, material.coatF0);
		f32 missingEnergy = F32Max(1.0f - material.averageAlbedo, MinEnergyCompensationDenominator);
		material.baseNormalization = 1.0f / (F32Pi * missingEnergy);
	}
	if (material.kind == MaterialKind::Dielectric) {
		material.hasAbsorption = !IsBlack(material.absorptionCoefficient);
	}
}

// Procedural checker that fades into its average with distance, which stops horizon moire
static Vec3f EvaluateBaseColor(const Material &material, const Vec3f &position) {
	if (!material.hasChecker) {
		return(material.baseColor);
	}
	Vec3f averageColor = 0.5f * (material.baseColor + material.checkerColor);
	f32 distanceFromOrigin = F32SquareRoot(position.x * position.x + position.y * position.y);
	f32 fadeRange = material.checkerFadeEnd - material.checkerFadeStart;
	f32 fadeRaw = (distanceFromOrigin - material.checkerFadeStart) / fadeRange;
	f32 fade = F32Clamp(fadeRaw, 0.0f, 1.0f);
	Vec3f result = averageColor;
	if (fade < 1.0f) {
		// Floor, not a cast: truncation toward zero would make the cells at the origin double wide
		f32 cellX = F32Floor(position.x / material.checkerSize);
		f32 cellY = F32Floor(position.y / material.checkerSize);
		s32 parity = ((s32)cellX + (s32)cellY) & 1;
		Vec3f checker = parity ? material.checkerColor : material.baseColor;
		result = V3fLerp(checker, fade, averageColor);
	}
	return(result);
}

static inline bool MaterialHasNonDeltaLobe(const Material &material) {
	switch (material.kind) {
		case MaterialKind::Diffuse:
		case MaterialKind::Plastic:
			return(true);
		case MaterialKind::Conductor:
			return(!material.isDeltaSpecular);
		default:
			return(false);
	}
}

static inline f32 PlasticSpecularProbability(const Vec3f &baseColor, const f32 coatAlbedoOut) {
	f32 baseLuminance = Luminance(baseColor);
	f32 specularShare = coatAlbedoOut / (coatAlbedoOut + baseLuminance * (1.0f - coatAlbedoOut));
	f32 result = F32Clamp(specularShare, PlasticSpecularProbabilityMin, PlasticSpecularProbabilityMax);
	return(result);
}

static inline f32 ConductorMultipleScatteringProbability(const Material &material, const f32 albedoOut) {
	if (IsBlack(material.multipleScatteringColor)) {
		return(0.0f);
	}
	f32 result = F32Clamp(1.0f - albedoOut, 0.0f, 1.0f);
	return(result);
}

// Non-delta part of the BSDF, without the cosine
static Vec3f EvaluateBsdf(const Material &material, const Vec3f &baseColor, const Vec3f &wo, const Vec3f &wi) {
	Vec3f result = V3fZero();
	if (wo.z <= 0.0f || wi.z <= 0.0f) {
		return(result);
	}
	switch (material.kind) {
		case MaterialKind::Diffuse:
		{
			result = InversePi * baseColor;
		} break;

		case MaterialKind::Conductor:
		{
			if (material.isDeltaSpecular) {
				break;
			}
			Vec3f halfSum = wo + wi;
			Vec3f halfVector = V3fNormalize(halfSum);
			f32 alphaSquared = material.alpha * material.alpha;
			f32 distribution = GgxDistribution(halfVector, material.alpha);
			f32 visibility = GgxVisibility(wo, wi, alphaSquared);
			f32 cosineHalf = V3fDot(wo, halfVector);
			Vec3f fresnel = SchlickFresnel(material.specularColor, cosineHalf);
			result = (distribution * visibility) * fresnel;
			if (!IsBlack(material.multipleScatteringColor)) {
				f32 albedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, 1.0f, wo.z);
				f32 albedoIn = GgxDirectionalAlbedo(material.roughnessCoordinate, 1.0f, wi.z);
				f32 missingProduct = (1.0f - albedoOut) * (1.0f - albedoIn);
				result += missingProduct * material.multipleScatteringColor;
			}
		} break;

		case MaterialKind::Plastic:
		{
			Vec3f halfSum = wo + wi;
			Vec3f halfVector = V3fNormalize(halfSum);
			f32 alphaSquared = material.alpha * material.alpha;
			f32 distribution = GgxDistribution(halfVector, material.alpha);
			f32 visibility = GgxVisibility(wo, wi, alphaSquared);
			f32 cosineHalf = V3fDot(wo, halfVector);
			f32 fresnel = SchlickFresnelScalar(material.coatF0, cosineHalf);
			f32 coatValue = fresnel * distribution * visibility;
			f32 coatAlbedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, material.coatF0, wo.z);
			f32 coatAlbedoIn = GgxDirectionalAlbedo(material.roughnessCoordinate, material.coatF0, wi.z);
			f32 baseScale = (1.0f - coatAlbedoOut) * (1.0f - coatAlbedoIn) * material.baseNormalization;
			Vec3f coat = V3fInit(coatValue, coatValue, coatValue);
			result = coat + baseScale * baseColor;
		} break;

		default:
			break;
	}
	return(result);
}

// Sharp (glossy) lobes defeat light tracing connections, so photon merging is offered as an extra strategy where the camera sees them
static inline bool HasSharpLobe(const Material &material) {
	bool isGlossyKind = (material.kind == MaterialKind::Conductor && !material.isDeltaSpecular) || (material.kind == MaterialKind::Plastic);
	bool result = isGlossyKind && (material.alpha < RoughLobeAlphaThreshold);
	return(result);
}

// Solid angle pdf of the non-delta part, including the lobe selection probabilities
static f32 BsdfPdf(const Material &material, const Vec3f &baseColor, const Vec3f &wo, const Vec3f &wi) {
	if (wo.z <= 0.0f || wi.z <= 0.0f) {
		return(0.0f);
	}
	f32 cosinePdf = wi.z * InversePi;
	switch (material.kind) {
		case MaterialKind::Diffuse:
			return(cosinePdf);

		case MaterialKind::Conductor:
		{
			if (material.isDeltaSpecular) {
				return(0.0f);
			}
			Vec3f halfSum = wo + wi;
			Vec3f halfVector = V3fNormalize(halfSum);
			f32 specularPdf = GgxReflectionPdf(wo, halfVector, material.alpha);
			f32 albedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, 1.0f, wo.z);
			f32 multipleProbability = ConductorMultipleScatteringProbability(material, albedoOut);
			f32 result = (1.0f - multipleProbability) * specularPdf + multipleProbability * cosinePdf;
			return(result);
		}

		case MaterialKind::Plastic:
		{
			Vec3f halfSum = wo + wi;
			Vec3f halfVector = V3fNormalize(halfSum);
			f32 specularPdf = GgxReflectionPdf(wo, halfVector, material.alpha);
			f32 coatAlbedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, material.coatF0, wo.z);
			f32 specularProbability = PlasticSpecularProbability(baseColor, coatAlbedoOut);
			f32 result = specularProbability * specularPdf + (1.0f - specularProbability) * cosinePdf;
			return(result);
		}

		default:
			return(0.0f);
	}
}

// Exact unpolarized Fresnel reflectance, eta = eta_transmitted / eta_incident, cosThetaI in [0,1]
static f32 FresnelDielectric(const f32 cosThetaI, const f32 eta) {
	f32 cosine = F32Clamp(cosThetaI, 0.0f, 1.0f);
	f32 sin2ThetaI = F32Max(0.0f, 1.0f - cosine * cosine);
	f32 sin2ThetaT = sin2ThetaI / (eta * eta);
	if (sin2ThetaT >= 1.0f) {
		return(1.0f); // total internal reflection
	}
	f32 cosThetaT = F32SquareRoot(F32Max(0.0f, 1.0f - sin2ThetaT));
	f32 parallel = (eta * cosine - cosThetaT) / (eta * cosine + cosThetaT);
	f32 perpendicular = (cosine - eta * cosThetaT) / (cosine + eta * cosThetaT);
	f32 result = 0.5f * (parallel * parallel + perpendicular * perpendicular);
	return(result);
}

struct BsdfSample {
	Vec3f wi;      // local frame, unit
	Vec3f weight;  // f * |wi.z| / pdf (analytic for delta lobes)
	f32 pdf;       // solid angle pdf including lobe selection, for delta lobes the discrete probability (never used for MIS)
	b32 isDelta;
};

// Mixture lobes use the one-sample estimator: weight = f_total * cos / pdf_total, so the BSDF strategy density matches the MIS pdf
static bool FinishMixtureSample(const Material &material, const Vec3f &baseColor, const Vec3f &wo, const Vec3f &wi, BsdfSample &outSample) {
	if (wi.z <= 0.0f) {
		return(false);
	}
	f32 pdf = BsdfPdf(material, baseColor, wo, wi);
	if (!(pdf > 0.0f)) {
		return(false);
	}
	Vec3f value = EvaluateBsdf(material, baseColor, wo, wi);
	Vec3f weight = (wi.z / pdf) * value;
	if (!IsFiniteV3(weight)) {
		return(false);
	}
	outSample.wi = wi;
	outSample.weight = weight;
	outSample.pdf = pdf;
	outSample.isDelta = false;
	return(true);
}

static bool SampleBsdf(const Material &material, const Vec3f &baseColor, const Vec3f &wo, const bool isFrontFace, PathSampler &sampler, BsdfSample &outSample) {
	f32 uLobe = PathSamplerNext01(sampler);
	f32 u0 = PathSamplerNext01(sampler);
	f32 u1 = PathSamplerNext01(sampler);
	switch (material.kind) {
		case MaterialKind::Diffuse:
		{
			Vec3f wi = SampleCosineHemisphere(u0, u1);
			f32 pdf = wi.z * InversePi;
			if (!(pdf > 0.0f)) {
				return(false);
			}
			outSample.wi = wi;
			outSample.weight = baseColor;
			outSample.pdf = pdf;
			outSample.isDelta = false;
			return(true);
		}

		case MaterialKind::Conductor:
		{
			if (material.isDeltaSpecular) {
				Vec3f mirrored = V3fInit(-wo.x, -wo.y, wo.z);
				outSample.wi = mirrored;
				outSample.weight = SchlickFresnel(material.specularColor, wo.z);
				outSample.pdf = 1.0f;
				outSample.isDelta = true;
				return(true);
			}
			f32 albedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, 1.0f, wo.z);
			f32 multipleProbability = ConductorMultipleScatteringProbability(material, albedoOut);
			Vec3f wi;
			if (uLobe < multipleProbability) {
				wi = SampleCosineHemisphere(u0, u1);
			} else {
				Vec3f halfVector = SampleGgxVisibleNormal(wo, material.alpha, u0, u1);
				wi = Reflect(wo, halfVector);
			}
			bool result = FinishMixtureSample(material, baseColor, wo, wi, outSample);
			return(result);
		}

		case MaterialKind::Plastic:
		{
			f32 coatAlbedoOut = GgxDirectionalAlbedo(material.roughnessCoordinate, material.coatF0, wo.z);
			f32 specularProbability = PlasticSpecularProbability(baseColor, coatAlbedoOut);
			Vec3f wi;
			if (uLobe < specularProbability) {
				Vec3f halfVector = SampleGgxVisibleNormal(wo, material.alpha, u0, u1);
				wi = Reflect(wo, halfVector);
			} else {
				wi = SampleCosineHemisphere(u0, u1);
			}
			bool result = FinishMixtureSample(material, baseColor, wo, wi, outSample);
			return(result);
		}

		case MaterialKind::Dielectric:
		{
			// The relative IOR is chosen by the side the ray arrived from; the 1/eta^2 radiance scale cancels because camera and lights are in air
			f32 etaRelative = isFrontFace ? material.ior : (1.0f / material.ior);
			f32 reflectance = FresnelDielectric(wo.z, etaRelative);
			Vec3f one = V3fInit(1.0f, 1.0f, 1.0f);
			if (uLobe < reflectance) {
				outSample.wi = V3fInit(-wo.x, -wo.y, wo.z);
				outSample.pdf = reflectance;
			} else {
				f32 sin2ThetaT = (1.0f - wo.z * wo.z) / (etaRelative * etaRelative);
				f32 cosThetaT = F32SquareRoot(F32Max(0.0f, 1.0f - sin2ThetaT));
				outSample.wi = V3fInit(-wo.x / etaRelative, -wo.y / etaRelative, -cosThetaT);
				outSample.pdf = 1.0f - reflectance;
			}
			outSample.weight = one;
			outSample.isDelta = true;
			return(true);
		}

		default:
			return(false);
	}
}

//
// Lights and environment
//
static constexpr f32 SmallAngleSin2Threshold = 0.00068523f; // sin^2(1.5 degrees): below this use the Taylor form of 1 - cos(thetaMax)
static constexpr f32 MinLightCosine = 1e-6f;
static const f32 UniformSpherePdf = 1.0f / (4.0f * F32Pi);
static constexpr f32 LightSelectionPowerShare = 0.5f;       // defensive mixture: half by power, half uniform
static constexpr f32 MinRegionRadius = 1.0f;

enum class LightKind : u32 {
	Sphere = 0,
	Quad,
	Sun,
	Environment, // uniform environment sampled as a light (white furnace MIS test)
};

struct Light {
	LightKind kind;
	u32 primitiveIndex;         // Sphere/Quad: the emissive primitive, NoIndex otherwise
	Vec3f radiance;
	f32 selectionProbability;
	Vec3f direction;            // Sun: unit vector towards the sun
	f32 oneMinusCosThetaMax;    // Sun: 2 sin^2(theta / 2), precise for tiny angles
	f32 cosThetaMax;            // Sun
	f32 conePdf;                // Sun: 1 / (2 pi oneMinusCosThetaMax)
	b32 isTwoSided;
};

struct LightSample {
	Vec3f wi;       // world, unit, from the shading point towards the light
	f32 distance;   // to the sampled light point, UnboundedDistance for the sun and the environment
	Vec3f radiance; // radiance arriving along -wi
	f32 pdf;        // solid angle, without the selection probability
};

enum class EnvironmentKind : u32 {
	Black = 0,
	Uniform,
	Sky,
};

struct SkyDesc {
	Vec3f zenithRadiance;
	Vec3f horizonRadiance;
	Vec3f belowHorizonRadiance;  // only reached if a downward ray misses all geometry
	f32 horizonExponent;         // width of the bright horizon band, larger = thinner band
	Vec3f aureoleTint;           // forward scattering glow around the sun (part of the sky, reached by BSDF sampling only)
	f32 aureoleStrength;
	f32 aureoleExponent;         // keep it low (<= 64), a sharp aureole would be a second small light without NEE
	f32 skyScale;                // multiplies gradient and aureole, not the sun
	f32 sunElevationDegrees;
	f32 sunAzimuthDegrees;       // counter-clockwise from +X, 90 degrees = +Y
	Vec3f sunTint;               // chromaticity only, normalized to luminance 1
	f32 sunIlluminance;          // luminance of the irradiance on a plane facing the sun, independent of the disk size
	f32 sunAngularRadiusDegrees;
};

struct Environment {
	EnvironmentKind kind;
	Vec3f uniformRadiance;
	b32 isUniformSampled;        // Uniform: also sample the environment as a light (MIS furnace test)
	SkyDesc sky;
	Vec3f sunDirection;
	u32 sunLightIndex;           // NoIndex when there is no sun
	u32 environmentLightIndex;   // NoIndex unless the uniform environment is sampled
};

static Vec3f EvaluateSkyBase(const SkyDesc &sky, const Vec3f &sunDirection, const Vec3f &direction) {
	if (direction.z <= 0.0f) {
		Vec3f below = sky.skyScale * sky.belowHorizonRadiance;
		return(below);
	}
	f32 oneMinusUp = 1.0f - direction.z;
	f32 horizonWeight = F32Power(oneMinusUp, sky.horizonExponent);
	Vec3f gradient = V3fLerp(sky.zenithRadiance, horizonWeight, sky.horizonRadiance);
	f32 cosToSun = V3fDot(direction, sunDirection);
	f32 clampedCosToSun = F32Max(cosToSun, 0.0f);
	f32 aureoleFalloff = F32Power(clampedCosToSun, sky.aureoleExponent);
	Vec3f aureole = (sky.aureoleStrength * aureoleFalloff) * sky.aureoleTint;
	Vec3f sum = gradient + aureole;
	Vec3f result = sky.skyScale * sum;
	return(result);
}

// Environment radiance for an escaped ray, without the sun disk (the sun is a separately sampled light)
static Vec3f EvaluateEnvironment(const Environment &environment, const Vec3f &direction) {
	switch (environment.kind) {
		case EnvironmentKind::Uniform:
			return(environment.uniformRadiance);
		case EnvironmentKind::Sky:
		{
			Vec3f sky = EvaluateSkyBase(environment.sky, environment.sunDirection, direction);
			return(sky);
		}
		default:
		{
			Vec3f black = V3fZero();
			return(black);
		}
	}
}

// Cone of directions subtended by a sphere, shared by sampling and the pdf on hit, so MIS weights always sum to one
struct SphereCone {
	b32 isInside;
	f32 distanceToCenter;
	f32 oneMinusCosThetaMax;
	Vec3f axis;
};

static SphereCone ComputeSphereCone(const Vec3f &position, const Vec3f &center, const f32 radius) {
	SphereCone result = {};
	Vec3f toCenter = center - position;
	f32 distanceSquared = V3fDot(toCenter, toCenter);
	f32 radiusSquared = radius * radius;
	result.isInside = distanceSquared <= radiusSquared;
	if (result.isInside) {
		return(result);
	}
	result.distanceToCenter = F32SquareRoot(distanceSquared);
	result.axis = (1.0f / result.distanceToCenter) * toCenter;
	f32 sin2ThetaMax = radiusSquared / distanceSquared;
	f32 cosThetaMax = F32SquareRoot(F32Max(0.0f, 1.0f - sin2ThetaMax));
	result.oneMinusCosThetaMax = (sin2ThetaMax < SmallAngleSin2Threshold) ? (0.5f * sin2ThetaMax) : (1.0f - cosThetaMax);
	return(result);
}

static inline f32 ConePdf(const f32 oneMinusCosThetaMax) {
	f32 result = 1.0f / (F32Tau * oneMinusCosThetaMax);
	return(result);
}

// Uniform direction inside a cone around axis, sin^2 is computed as omc (2 - omc) to avoid cancellation
static Vec3f SampleCone(const Vec3f &axis, const f32 oneMinusCosThetaMax, const f32 u0, const f32 u1, f32 &outCosTheta, f32 &outSin2Theta) {
	f32 oneMinusCosTheta = u0 * oneMinusCosThetaMax;
	f32 cosTheta = 1.0f - oneMinusCosTheta;
	f32 sin2Theta = oneMinusCosTheta * (2.0f - oneMinusCosTheta);
	f32 sinTheta = F32SquareRoot(F32Max(0.0f, sin2Theta));
	f32 phi = F32Tau * u1;
	f32 cosPhi = F32Cos(phi);
	f32 sinPhi = F32Sin(phi);
	Vec3f tangent;
	Vec3f bitangent;
	BuildOrthonormalBasis(axis, tangent, bitangent);
	Vec3f local = (sinTheta * cosPhi) * tangent + (sinTheta * sinPhi) * bitangent + cosTheta * axis;
	Vec3f result = V3fNormalize(local);
	outCosTheta = cosTheta;
	outSin2Theta = sin2Theta;
	return(result);
}

static bool SampleLight(const SceneView &scene, const Light &light, const Vec3f &position, const f32 u0, const f32 u1, LightSample &outSample) {
	switch (light.kind) {
		case LightKind::Sphere:
		{
			const SphereShape &sphere = scene.primitives[light.primitiveIndex].sphere;
			SphereCone cone = ComputeSphereCone(position, sphere.center, sphere.radius);
			if (cone.isInside) {
				return(false); // a one-sided outward emitter sends nothing inward
			}
			f32 cosTheta;
			f32 sin2Theta;
			Vec3f wi = SampleCone(cone.axis, cone.oneMinusCosThetaMax, u0, u1, cosTheta, sin2Theta);
			f32 distanceSquaredToCenter = cone.distanceToCenter * cone.distanceToCenter;
			f32 radiusSquared = sphere.radius * sphere.radius;
			f32 chordRadicand = F32Max(0.0f, radiusSquared - distanceSquaredToCenter * sin2Theta);
			f32 chordHalf = F32SquareRoot(chordRadicand);
			outSample.wi = wi;
			outSample.distance = cone.distanceToCenter * cosTheta - chordHalf;
			outSample.radiance = light.radiance;
			outSample.pdf = ConePdf(cone.oneMinusCosThetaMax);
			return(true);
		}

		case LightKind::Quad:
		{
			const QuadShape &quad = scene.primitives[light.primitiveIndex].quad;
			Vec3f lightPoint = quad.corner + u0 * quad.edgeU + u1 * quad.edgeV;
			Vec3f toLight = lightPoint - position;
			f32 distanceSquared = V3fDot(toLight, toLight);
			if (!(distanceSquared > 0.0f)) {
				return(false);
			}
			f32 distance = F32SquareRoot(distanceSquared);
			Vec3f wi = (1.0f / distance) * toLight;
			f32 signedCosine = -V3fDot(quad.normal, wi);
			f32 cosineAtLight = light.isTwoSided ? F32Abs(signedCosine) : signedCosine;
			if (cosineAtLight < MinLightCosine) {
				return(false); // back side or grazing: no emission, the pdf would explode
			}
			outSample.wi = wi;
			outSample.distance = distance;
			outSample.radiance = light.radiance;
			outSample.pdf = distanceSquared / (cosineAtLight * quad.area);
			return(true);
		}

		case LightKind::Sun:
		{
			f32 cosTheta;
			f32 sin2Theta;
			Vec3f wi = SampleCone(light.direction, light.oneMinusCosThetaMax, u0, u1, cosTheta, sin2Theta);
			outSample.wi = wi;
			outSample.distance = UnboundedDistance;
			outSample.radiance = light.radiance;
			outSample.pdf = light.conePdf;
			return(true);
		}

		case LightKind::Environment:
		{
			f32 z = 1.0f - 2.0f * u0;
			f32 ringRadius = F32SquareRoot(F32Max(0.0f, 1.0f - z * z));
			f32 phi = F32Tau * u1;
			f32 cosPhi = F32Cos(phi);
			f32 sinPhi = F32Sin(phi);
			outSample.wi = V3fInit(ringRadius * cosPhi, ringRadius * sinPhi, z);
			outSample.distance = UnboundedDistance;
			outSample.radiance = light.radiance;
			outSample.pdf = UniformSpherePdf;
			return(true);
		}

		default:
			return(false);
	}
}

// Solid angle pdf that SampleLight would give for the direction from position that hit the light at hit (no selection probability)
static f32 LightPdfForHit(const SceneView &scene, const Light &light, const Vec3f &position, const Vec3f &rayDirection, const SurfaceHit &hit) {
	switch (light.kind) {
		case LightKind::Sphere:
		{
			const SphereShape &sphere = scene.primitives[light.primitiveIndex].sphere;
			SphereCone cone = ComputeSphereCone(position, sphere.center, sphere.radius);
			if (cone.isInside) {
				return(0.0f);
			}
			f32 result = ConePdf(cone.oneMinusCosThetaMax);
			return(result);
		}

		case LightKind::Quad:
		{
			const QuadShape &quad = scene.primitives[light.primitiveIndex].quad;
			f32 signedCosine = -V3fDot(quad.normal, rayDirection);
			f32 cosineAtLight = light.isTwoSided ? F32Abs(signedCosine) : signedCosine;
			if (cosineAtLight < MinLightCosine) {
				return(0.0f);
			}
			f32 distanceSquared = hit.distance * hit.distance;
			f32 result = distanceSquared / (cosineAtLight * quad.area);
			return(result);
		}

		default:
			return(0.0f);
	}
}

static u32 SelectLight(const SceneView &scene, const f32 u) {
	for (u32 lightIndex = 0; lightIndex < scene.lightCount; ++lightIndex) {
		if (u < scene.lightCdf[lightIndex]) {
			return(lightIndex);
		}
	}
	// Float rounding can leave the last cdf entry slightly below 1
	u32 result = scene.lightCount - 1;
	return(result);
}

//
// Scene
//
struct SceneCameraDesc {
	Vec3f eye;
	Vec3f target;              // look-at point, the orbit pivot is its projection onto the view axis
	f32 fovYDegrees;           // vertical field of view at the reference aspect
	f32 referenceAspect;       // width / height the framing was designed for, narrower windows widen the vertical fov
	f32 apertureRadius;        // thin lens radius in world units, 0 = pinhole
	f32 focusDistance;         // distance of the focus plane along the view axis, <= 0 = target distance
	f32 defaultApertureRadius; // aperture restored when depth of field is toggled on
};

static constexpr f32 DefaultPhotonRadius = 0.025f;

struct Scene {
	const char *name;
	const char *fileName;
	std::vector<Material> materials;
	std::vector<Primitive> primitives;
	std::vector<PlanePrimitive> planes;
	std::vector<BvhNode> bvhNodes;
	std::vector<Light> lights;
	std::vector<f32> lightCdf;
	std::vector<CausticCaster> casters;
	Environment environment;
	SceneCameraDesc camera;
	f32 exposureEV;
	u32 maxBounces;
	f32 photonRadius; // initial gather radius of the caustic photon map in world units
	SceneView view;

	Scene() : name(""), fileName("scene"), environment(), camera(), exposureEV(0.0f), maxBounces(0), photonRadius(DefaultPhotonRadius), view() {
		environment.kind = EnvironmentKind::Black;
		environment.sunLightIndex = NoIndex;
		environment.environmentLightIndex = NoIndex;
	}

	u32 AddMaterial(Material material) {
		FinalizeMaterial(material);
		u32 result = (u32)materials.size();
		materials.push_back(material);
		return(result);
	}

	u32 AddDiffuse(const Vec3f &albedo) {
		Material material = {};
		material.kind = MaterialKind::Diffuse;
		material.baseColor = albedo;
		u32 result = AddMaterial(material);
		return(result);
	}

	u32 AddCheckerDiffuse(const Vec3f &albedoA, const Vec3f &albedoB, const f32 squareSize, const f32 fadeStart, const f32 fadeEnd) {
		Material material = {};
		material.kind = MaterialKind::Diffuse;
		material.baseColor = albedoA;
		material.checkerColor = albedoB;
		material.checkerSize = squareSize;
		material.checkerFadeStart = fadeStart;
		material.checkerFadeEnd = fadeEnd;
		material.hasChecker = true;
		u32 result = AddMaterial(material);
		return(result);
	}

	u32 AddConductor(const Vec3f &f0, const f32 alpha) {
		Material material = {};
		material.kind = MaterialKind::Conductor;
		material.specularColor = f0;
		material.alpha = alpha;
		u32 result = AddMaterial(material);
		return(result);
	}

	u32 AddMirror(const Vec3f &f0) {
		u32 result = AddConductor(f0, 0.0f);
		return(result);
	}

	// absorption = Beer-Lambert sigma_a per unit length (0 = clear), a tint "transmittance T after distance D" is -ln(T) / D
	u32 AddDielectric(const f32 ior, const Vec3f &absorption) {
		Material material = {};
		material.kind = MaterialKind::Dielectric;
		material.ior = ior;
		material.absorptionCoefficient = absorption;
		u32 result = AddMaterial(material);
		return(result);
	}

	u32 AddPlastic(const Vec3f &baseAlbedo, const f32 coatAlpha, const f32 coatIor) {
		Material material = {};
		material.kind = MaterialKind::Plastic;
		material.baseColor = baseAlbedo;
		material.alpha = coatAlpha;
		material.ior = coatIor;
		u32 result = AddMaterial(material);
		return(result);
	}

	u32 AddEmissive(const Vec3f &radiance, const bool isTwoSided = false) {
		Material material = {};
		material.kind = MaterialKind::Emissive;
		material.emission = radiance;
		material.isTwoSided = isTwoSided;
		u32 result = AddMaterial(material);
		return(result);
	}

	void AddPlane(const Vec3f &normal, const Vec3f &pointOnPlane, const u32 materialIndex) {
		fplAssert(materialIndex < materials.size());
		PlanePrimitive plane = {};
		plane.normal = V3fNormalize(normal);
		plane.planeOffset = V3fDot(plane.normal, pointOnPlane);
		plane.materialIndex = materialIndex;
		planes.push_back(plane);
	}

	void AddSphere(const Vec3f &center, const f32 radius, const u32 materialIndex) {
		fplAssert(materialIndex < materials.size());
		Primitive primitive = MakeSpherePrimitive(center, radius, materialIndex);
		primitives.push_back(primitive);
	}

	void AddQuad(const Vec3f &corner, const Vec3f &edgeU, const Vec3f &edgeV, const u32 materialIndex) {
		fplAssert(materialIndex < materials.size());
		Primitive primitive = MakeQuadPrimitive(corner, edgeU, edgeV, materialIndex);
		primitives.push_back(primitive);
	}

	// One-sided quad centered at center whose +normal (emitting side) faces target, UnitUp must not be parallel to (target - center)
	void AddQuadFacing(const Vec3f &center, const Vec3f &target, const f32 width, const f32 height, const u32 materialIndex) {
		Vec3f toTarget = target - center;
		Vec3f facing = V3fNormalize(toTarget);
		Vec3f sideRaw = V3fCross(UnitUp, facing);
		Vec3f side = V3fNormalize(sideRaw);
		Vec3f upward = V3fCross(facing, side);
		Vec3f edgeU = width * side;
		Vec3f edgeV = height * upward;
		Vec3f corner = center - 0.5f * edgeU - 0.5f * edgeV;
		AddQuad(corner, edgeU, edgeV, materialIndex);
	}

	void AddBox(const Vec3f &center, const Vec3f &halfExtents, const f32 rotationZDegrees, const u32 materialIndex) {
		fplAssert(materialIndex < materials.size());
		f32 yawRadians = F32DegreesToRadians(rotationZDegrees);
		Primitive primitive = MakeBoxPrimitive(center, halfExtents, yawRadians, materialIndex);
		primitives.push_back(primitive);
	}

	void SetEnvironmentBlack() {
		environment.kind = EnvironmentKind::Black;
	}

	void SetEnvironmentUniform(const Vec3f &radiance, const bool isSampledAsLight) {
		environment.kind = EnvironmentKind::Uniform;
		environment.uniformRadiance = radiance;
		environment.isUniformSampled = isSampledAsLight;
	}

	void SetEnvironmentSky(const SkyDesc &sky) {
		environment.kind = EnvironmentKind::Sky;
		environment.sky = sky;
	}

	void SetCamera(const Vec3f &eye, const Vec3f &target, const f32 fovYDegrees, const f32 referenceAspect, const f32 apertureRadius, const f32 focusDistance) {
		camera.eye = eye;
		camera.target = target;
		camera.fovYDegrees = fovYDegrees;
		camera.referenceAspect = referenceAspect;
		camera.apertureRadius = apertureRadius;
		camera.focusDistance = focusDistance;
		camera.defaultApertureRadius = apertureRadius;
	}

	// Builds the BVH, the light list and the raw view; must only be called while no worker renders this scene
	void Bake();
};

static f32 EstimateLightPower(const Scene &scene, const Light &light, const f32 regionRadius) {
	f32 luminance = Luminance(light.radiance);
	f32 sideFactor = light.isTwoSided ? 2.0f : 1.0f;
	switch (light.kind) {
		case LightKind::Sphere:
		{
			// Area 4 pi r^2, each point emits pi L
			f32 radius = scene.primitives[light.primitiveIndex].sphere.radius;
			f32 result = 4.0f * F32Pi * F32Pi * radius * radius * luminance * sideFactor;
			return(result);
		}
		case LightKind::Quad:
		{
			f32 area = scene.primitives[light.primitiveIndex].quad.area;
			f32 result = F32Pi * area * luminance * sideFactor;
			return(result);
		}
		case LightKind::Sun:
		{
			// Irradiance at normal incidence times the cross section of the region of interest
			f32 irradiance = luminance * F32Pi * light.oneMinusCosThetaMax * (2.0f - light.oneMinusCosThetaMax);
			f32 result = F32Pi * regionRadius * regionRadius * irradiance;
			return(result);
		}
		case LightKind::Environment:
		{
			f32 result = 4.0f * F32Pi * F32Pi * regionRadius * regionRadius * luminance;
			return(result);
		}
		default:
			return(0.0f);
	}
}

void Scene::Bake() {
	BuildBvh(primitives, bvhNodes);

	// Lights are assigned after the BVH reordered the primitives, so the indices match
	lights.clear();
	u32 primitiveCount = (u32)primitives.size();
	for (u32 primitiveIndex = 0; primitiveIndex < primitiveCount; ++primitiveIndex) {
		Primitive &primitive = primitives[primitiveIndex];
		const Material &material = materials[primitive.materialIndex];
		primitive.lightIndex = NoIndex;
		bool isEmissive = material.kind == MaterialKind::Emissive;
		bool isSampleable = primitive.kind == PrimitiveKind::Sphere || primitive.kind == PrimitiveKind::Quad;
		if (isEmissive && isSampleable) {
			Light light = {};
			light.kind = (primitive.kind == PrimitiveKind::Sphere) ? LightKind::Sphere : LightKind::Quad;
			light.primitiveIndex = primitiveIndex;
			light.radiance = material.emission;
			light.isTwoSided = material.isTwoSided && (primitive.kind == PrimitiveKind::Quad);
			primitive.lightIndex = (u32)lights.size();
			lights.push_back(light);
		}
	}

	environment.sunLightIndex = NoIndex;
	environment.environmentLightIndex = NoIndex;
	if (environment.kind == EnvironmentKind::Sky && environment.sky.sunIlluminance > 0.0f) {
		const SkyDesc &sky = environment.sky;
		f32 elevation = F32DegreesToRadians(sky.sunElevationDegrees);
		f32 azimuth = F32DegreesToRadians(sky.sunAzimuthDegrees);
		f32 cosElevation = F32Cos(elevation);
		f32 sinElevation = F32Sin(elevation);
		f32 cosAzimuth = F32Cos(azimuth);
		f32 sinAzimuth = F32Sin(azimuth);
		environment.sunDirection = V3fInit(cosElevation * cosAzimuth, cosElevation * sinAzimuth, sinElevation);
		// The sun is specified by illuminance, so changing its size keeps the lighting level: L = E / (projected solid angle)
		f32 angularRadius = F32DegreesToRadians(sky.sunAngularRadiusDegrees);
		f32 sinHalfRadius = F32Sin(0.5f * angularRadius);
		f32 oneMinusCosRadius = 2.0f * sinHalfRadius * sinHalfRadius;
		f32 sin2Radius = oneMinusCosRadius * (2.0f - oneMinusCosRadius);
		f32 projectedSolidAngle = F32Pi * sin2Radius;
		f32 tintLuminance = Luminance(sky.sunTint);
		f32 radianceScale = sky.sunIlluminance / (projectedSolidAngle * tintLuminance);
		Light sun = {};
		sun.kind = LightKind::Sun;
		sun.primitiveIndex = NoIndex;
		sun.radiance = radianceScale * sky.sunTint;
		sun.direction = environment.sunDirection;
		sun.oneMinusCosThetaMax = oneMinusCosRadius;
		sun.cosThetaMax = 1.0f - oneMinusCosRadius;
		sun.conePdf = ConePdf(oneMinusCosRadius);
		environment.sunLightIndex = (u32)lights.size();
		lights.push_back(sun);
	} else {
		environment.sunDirection = UnitUp;
	}
	if (environment.kind == EnvironmentKind::Uniform && environment.isUniformSampled) {
		Light environmentLight = {};
		environmentLight.kind = LightKind::Environment;
		environmentLight.primitiveIndex = NoIndex;
		environmentLight.radiance = environment.uniformRadiance;
		environment.environmentLightIndex = (u32)lights.size();
		lights.push_back(environmentLight);
	}

	// Defensive light selection: half proportional to power, half uniform, so dim lights near the subject are never starved
	f32 regionRadius = MinRegionRadius;
	Vec3f regionCenter = V3fZero();
	if (!bvhNodes.empty()) {
		Vec3f rootExtent = bvhNodes[0].boundsMax - bvhNodes[0].boundsMin;
		Vec3f rootSum = bvhNodes[0].boundsMax + bvhNodes[0].boundsMin;
		f32 halfDiagonal = 0.5f * V3fLength(rootExtent);
		regionRadius = F32Max(halfDiagonal, MinRegionRadius);
		regionCenter = 0.5f * rootSum;
	}
	u32 lightCount = (u32)lights.size();
	lightCdf.assign(lightCount, 0.0f);
	if (lightCount > 0) {
		f32 totalPower = 0.0f;
		for (u32 lightIndex = 0; lightIndex < lightCount; ++lightIndex) {
			f32 power = EstimateLightPower(*this, lights[lightIndex], regionRadius);
			totalPower += power;
		}
		f32 uniformProbability = 1.0f / (f32)lightCount;
		f32 cumulative = 0.0f;
		for (u32 lightIndex = 0; lightIndex < lightCount; ++lightIndex) {
			Light &light = lights[lightIndex];
			f32 power = EstimateLightPower(*this, light, regionRadius);
			f32 powerProbability = (totalPower > 0.0f) ? (power / totalPower) : uniformProbability;
			light.selectionProbability = LightSelectionPowerShare * powerProbability + (1.0f - LightSelectionPowerShare) * uniformProbability;
			cumulative += light.selectionProbability;
			lightCdf[lightIndex] = cumulative;
		}
		lightCdf[lightCount - 1] = 1.0f;
	}

	// Specular casters for light tracing: bounding spheres of all mirror and glass primitives
	casters.clear();
	f32 casterAreaSum = 0.0f;
	for (u32 primitiveIndex = 0; primitiveIndex < primitiveCount; ++primitiveIndex) {
		const Primitive &primitive = primitives[primitiveIndex];
		const Material &material = materials[primitive.materialIndex];
		bool isSpecular = (material.kind == MaterialKind::Dielectric) || (material.kind == MaterialKind::Conductor && material.isDeltaSpecular);
		if (!isSpecular) {
			continue;
		}
		CausticCaster caster = {};
		switch (primitive.kind) {
			case PrimitiveKind::Sphere:
			{
				caster.center = primitive.sphere.center;
				caster.radius = primitive.sphere.radius;
			} break;
			case PrimitiveKind::Quad:
			{
				const QuadShape &quad = primitive.quad;
				Vec3f diagonal = quad.edgeU + quad.edgeV;
				Vec3f antiDiagonal = quad.edgeU - quad.edgeV;
				f32 diagonalLength = V3fLength(diagonal);
				f32 antiDiagonalLength = V3fLength(antiDiagonal);
				caster.center = quad.corner + 0.5f * diagonal;
				caster.radius = 0.5f * F32Max(diagonalLength, antiDiagonalLength);
			} break;
			case PrimitiveKind::Box:
			{
				caster.center = primitive.box.center;
				caster.radius = V3fLength(primitive.box.halfExtents);
			} break;
			default:
				break;
		}
		casterAreaSum += caster.radius * caster.radius;
		casters.push_back(caster);
	}
	bool hasLightTracedLight = false;
	for (u32 lightIndex = 0; lightIndex < lightCount; ++lightIndex) {
		LightKind kind = lights[lightIndex].kind;
		if (kind == LightKind::Sphere || kind == LightKind::Quad || kind == LightKind::Sun) {
			hasLightTracedLight = true;
		}
	}

	view.casters = casters.data();
	view.casterCount = (u32)casters.size();
	view.casterAreaSum = casterAreaSum;
	view.regionCenter = regionCenter;
	view.regionRadius = regionRadius;
	view.isLightTracingAvailable = hasLightTracedLight && !casters.empty();
	view.primitives = primitives.data();
	view.primitiveCount = primitiveCount;
	view.bvhNodes = bvhNodes.data();
	view.bvhNodeCount = (u32)bvhNodes.size();
	view.planes = planes.data();
	view.planeCount = (u32)planes.size();
	view.materials = materials.data();
	view.materialCount = (u32)materials.size();
	view.lights = lights.data();
	view.lightCount = lightCount;
	view.lightCdf = lightCdf.data();
	view.environment = &environment;
}

//
// Scenes (Z up, Y forward, X right, colors in linear RGB, roughness values are GGX alpha)
//
enum class TonemapOperator : u32 {
	AcesFitted = 0,
	PbrNeutral,
	None,
	Count,
};

static const char *TonemapOperatorNames[] = {
	"ACES",
	"Neutral",
	"None",
};
fplStaticAssert(fplArrayCount(TonemapOperatorNames) == (u32)TonemapOperator::Count);

struct SceneEntry {
	Scene scene;
	TonemapOperator tonemapOperator;
};

static constexpr f32 WideReferenceAspect = 5.0f / 3.0f;
static constexpr f32 SquareReferenceAspect = 1.0f;
static constexpr u32 DefaultMaxBounces = 12;
static constexpr f32 GlassIor = 1.5f;
static constexpr f32 CoatIor = 1.5f;

static const Vec3f GoldF0 = V3fInit(1.000f, 0.766f, 0.336f);
static const Vec3f SilverF0 = V3fInit(0.972f, 0.960f, 0.915f);
static const Vec3f CopperF0 = V3fInit(0.955f, 0.638f, 0.538f);
static const Vec3f AluminiumF0 = V3fInit(0.913f, 0.922f, 0.924f);
static const Vec3f ChromiumF0 = V3fInit(0.549f, 0.556f, 0.554f);
static const Vec3f ClearAbsorption = V3fInit(0.0f, 0.0f, 0.0f);
static const Vec3f GroundNormal = V3fInit(0.0f, 0.0f, 1.0f);
static const Vec3f GroundPoint = V3fInit(0.0f, 0.0f, 0.0f);

// Energy conservation test: in a uniform environment of radiance 1 every lossless BSDF is invisible
static void BuildWhiteFurnaceScene(Scene &scene) {
	constexpr f32 sphereRadius = 1.0f;
	constexpr f32 sphereSpacing = 2.4f;
	constexpr s32 sphereHalfCount = 2;
	constexpr f32 roughConductorAlpha = 0.5f;
	constexpr f32 plasticCoatAlpha = 0.2f;
	const Vec3f white = V3fInit(1.0f, 1.0f, 1.0f);

	scene.name = "White Furnace";
	scene.fileName = "white_furnace";
	u32 diffuseMaterial = scene.AddDiffuse(white);
	u32 glassMaterial = scene.AddDielectric(GlassIor, ClearAbsorption);
	u32 mirrorMaterial = scene.AddMirror(white);
	u32 roughConductorMaterial = scene.AddConductor(white, roughConductorAlpha);
	u32 plasticMaterial = scene.AddPlastic(white, plasticCoatAlpha, CoatIor);
	const u32 sphereMaterials[] = { diffuseMaterial, glassMaterial, mirrorMaterial, roughConductorMaterial, plasticMaterial };
	for (s32 sphereIndex = -sphereHalfCount; sphereIndex <= sphereHalfCount; ++sphereIndex) {
		f32 centerX = (f32)sphereIndex * sphereSpacing;
		Vec3f center = V3fInit(centerX, 0.0f, 0.0f);
		u32 materialIndex = sphereMaterials[sphereIndex + sphereHalfCount];
		scene.AddSphere(center, sphereRadius, materialIndex);
	}

	const Vec3f environmentRadiance = V3fInit(1.0f, 1.0f, 1.0f);
	constexpr bool isEnvironmentSampled = true;
	scene.SetEnvironmentUniform(environmentRadiance, isEnvironmentSampled);

	const Vec3f eye = V3fInit(0.0f, -15.0f, 0.0f);
	const Vec3f target = V3fInit(0.0f, 0.0f, 0.0f);
	constexpr f32 fovYDegrees = 30.0f;
	constexpr f32 furnaceExposureEV = -1.0f;
	constexpr u32 furnaceMaxBounces = 64;
	scene.SetCamera(eye, target, fovYDegrees, WideReferenceAspect, 0.0f, 0.0f);
	scene.exposureEV = furnaceExposureEV;
	scene.maxBounces = furnaceMaxBounces;
}

// Measured Cornell box converted to Z up (1 unit = 100 mm): X = (278 - x) / 100, Y = z / 100, Z = y / 100
static void BuildCornellBoxScene(Scene &scene, const bool useClassicBoxes) {
	constexpr f32 halfWidth = 2.78f;
	constexpr f32 width = halfWidth * 2.0f;
	constexpr f32 depth = 5.592f;
	constexpr f32 height = 5.488f;
	constexpr f32 lightSizeX = 1.30f;
	constexpr f32 lightSizeY = 1.05f;
	constexpr f32 lightMinY = 2.27f;
	constexpr f32 lightGapBelowCeiling = 0.003f;
	constexpr f32 lightZ = height - lightGapBelowCeiling;

	const Vec3f whiteAlbedo = V3fInit(0.725f, 0.71f, 0.68f);
	const Vec3f redAlbedo = V3fInit(0.63f, 0.065f, 0.05f);
	const Vec3f greenAlbedo = V3fInit(0.14f, 0.45f, 0.091f);
	const Vec3f lightRadiance = V3fInit(17.0f, 12.0f, 4.0f);
	u32 whiteMaterial = scene.AddDiffuse(whiteAlbedo);
	u32 redMaterial = scene.AddDiffuse(redAlbedo);
	u32 greenMaterial = scene.AddDiffuse(greenAlbedo);
	u32 lightMaterial = scene.AddEmissive(lightRadiance);

	const Vec3f alongX = V3fInit(width, 0.0f, 0.0f);
	const Vec3f alongY = V3fInit(0.0f, depth, 0.0f);
	const Vec3f alongZ = V3fInit(0.0f, 0.0f, height);
	const Vec3f leftFrontBottom = V3fInit(-halfWidth, 0.0f, 0.0f);
	const Vec3f rightFrontBottom = V3fInit(halfWidth, 0.0f, 0.0f);
	const Vec3f leftFrontTop = V3fInit(-halfWidth, 0.0f, height);
	const Vec3f leftBackBottom = V3fInit(-halfWidth, depth, 0.0f);
	scene.AddQuad(leftFrontBottom, alongX, alongY, whiteMaterial);   // floor, normal +Z
	scene.AddQuad(leftFrontTop, alongY, alongX, whiteMaterial);      // ceiling, normal -Z
	scene.AddQuad(leftBackBottom, alongX, alongZ, whiteMaterial);    // back wall, normal -Y
	scene.AddQuad(leftFrontBottom, alongY, alongZ, redMaterial);     // left wall, normal +X
	scene.AddQuad(rightFrontBottom, alongZ, alongY, greenMaterial);  // right wall, normal -X

	constexpr f32 lightMinX = -0.5f * lightSizeX;
	const Vec3f lightCorner = V3fInit(lightMinX, lightMinY, lightZ);
	const Vec3f lightEdgeU = V3fInit(0.0f, lightSizeY, 0.0f);
	const Vec3f lightEdgeV = V3fInit(lightSizeX, 0.0f, 0.0f);
	scene.AddQuad(lightCorner, lightEdgeU, lightEdgeV, lightMaterial); // normal -Z, emits downward

	if (useClassicBoxes) {
		// Measured short and tall blocks (sides about 166 mm, rotated by about -17 and +17.2 degrees)
		constexpr f32 blockHalfSide = 0.83f;
		constexpr f32 shortHalfHeight = 0.825f;
		constexpr f32 tallHalfHeight = 1.65f;
		constexpr f32 shortRotationDegrees = -17.0f;
		constexpr f32 tallRotationDegrees = 17.2f;
		const Vec3f shortCenter = V3fInit(0.925f, 1.69f, shortHalfHeight);
		const Vec3f tallCenter = V3fInit(-0.905f, 3.5125f, tallHalfHeight);
		const Vec3f shortHalfExtents = V3fInit(blockHalfSide, blockHalfSide, shortHalfHeight);
		const Vec3f tallHalfExtents = V3fInit(blockHalfSide, blockHalfSide, tallHalfHeight);
		scene.AddBox(shortCenter, shortHalfExtents, shortRotationDegrees, whiteMaterial);
		scene.AddBox(tallCenter, tallHalfExtents, tallRotationDegrees, whiteMaterial);
		scene.name = "Cornell Box Classic";
		scene.fileName = "cornell_box_classic";
	} else {
		// Glass ball with a caustic next to a silver mirror ball
		constexpr f32 sphereRadius = 1.0f;
		u32 glassMaterial = scene.AddDielectric(GlassIor, ClearAbsorption);
		u32 mirrorMaterial = scene.AddMirror(SilverF0);
		const Vec3f mirrorCenter = V3fInit(-1.30f, 3.80f, sphereRadius);
		const Vec3f glassCenter = V3fInit(1.25f, 1.90f, sphereRadius);
		scene.AddSphere(mirrorCenter, sphereRadius, mirrorMaterial);
		scene.AddSphere(glassCenter, sphereRadius, glassMaterial);
		scene.name = "Cornell Box";
		scene.fileName = "cornell_box";
	}
	scene.SetEnvironmentBlack();

	// The measured camera (278, 273, -800) mm with a 35 mm lens on 25 mm film, the target sits on the view axis at the box center depth
	const Vec3f eye = V3fInit(0.0f, -8.0f, 2.73f);
	const Vec3f target = V3fInit(0.0f, 2.796f, 2.73f);
	constexpr f32 fovYDegrees = 39.3077f;
	constexpr f32 cornellExposureEV = 0.7f;
	constexpr u32 cornellMaxBounces = 16;
	scene.SetCamera(eye, target, fovYDegrees, SquareReferenceAspect, 0.0f, 0.0f);
	scene.exposureEV = cornellExposureEV;
	scene.maxBounces = cornellMaxBounces;
}

// Seven spheres in a row on a sunny plaza, backlit by a low golden sun that throws long shadows towards the camera
static void BuildGoldenHourScene(Scene &scene) {
	constexpr f32 sphereRadius = 1.0f;
	constexpr f32 rowAngleDegrees = 30.0f;
	constexpr f32 rowSpacing = 2.5f;
	constexpr s32 rowHalfCount = 3;
	const Vec3f rowOrigin = V3fInit(1.8f, 3.0f, sphereRadius);

	const Vec3f clayAlbedo = V3fInit(0.80f, 0.77f, 0.72f);
	const Vec3f redBase = V3fInit(0.75f, 0.06f, 0.04f);
	const Vec3f blueBase = V3fInit(0.04f, 0.16f, 0.55f);
	constexpr f32 redCoatAlpha = 0.04f;
	constexpr f32 blueCoatAlpha = 0.18f;
	constexpr f32 aluminiumAlpha = 0.08f; // not a delta mirror: lets NEE reach the sun at the metal, otherwise the reflected sun never converges
	constexpr f32 goldAlpha = 0.25f;
	constexpr f32 copperAlpha = 0.12f;

	scene.name = "Golden Hour";
	scene.fileName = "golden_hour";
	u32 clayMaterial = scene.AddDiffuse(clayAlbedo);
	u32 redMaterial = scene.AddPlastic(redBase, redCoatAlpha, CoatIor);
	u32 aluminiumMaterial = scene.AddConductor(AluminiumF0, aluminiumAlpha);
	u32 goldMaterial = scene.AddConductor(GoldF0, goldAlpha);
	u32 blueMaterial = scene.AddPlastic(blueBase, blueCoatAlpha, CoatIor);
	u32 copperMaterial = scene.AddConductor(CopperF0, copperAlpha);
	u32 glassMaterial = scene.AddDielectric(GlassIor, ClearAbsorption);
	// Near (front left) to far (back right); the glass is at the far end so its sun caustic lands outside the frame
	const u32 rowMaterials[] = { clayMaterial, redMaterial, aluminiumMaterial, goldMaterial, blueMaterial, copperMaterial, glassMaterial };

	f32 rowAngle = F32DegreesToRadians(rowAngleDegrees);
	f32 rowCos = F32Cos(rowAngle);
	f32 rowSin = F32Sin(rowAngle);
	Vec3f rowStep = V3fInit(rowCos * rowSpacing, rowSin * rowSpacing, 0.0f);
	for (s32 rowIndex = -rowHalfCount; rowIndex <= rowHalfCount; ++rowIndex) {
		f32 rowOffset = (f32)rowIndex;
		Vec3f center = rowOrigin + rowOffset * rowStep;
		u32 materialIndex = rowMaterials[rowIndex + rowHalfCount];
		scene.AddSphere(center, sphereRadius, materialIndex);
	}

	const Vec3f tileLight = V3fInit(0.58f, 0.56f, 0.52f);
	const Vec3f tileDark = V3fInit(0.40f, 0.39f, 0.37f);
	constexpr f32 tileSize = 1.0f;
	constexpr f32 tileFadeStart = 30.0f;
	constexpr f32 tileFadeEnd = 80.0f;
	u32 groundMaterial = scene.AddCheckerDiffuse(tileLight, tileDark, tileSize, tileFadeStart, tileFadeEnd);
	scene.AddPlane(GroundNormal, GroundPoint, groundMaterial);

	SkyDesc sky = {};
	sky.zenithRadiance = V3fInit(0.13f, 0.26f, 0.58f);
	sky.horizonRadiance = V3fInit(0.78f, 0.74f, 0.70f);
	sky.belowHorizonRadiance = V3fInit(0.18f, 0.16f, 0.14f);
	sky.horizonExponent = 4.0f;
	sky.aureoleTint = V3fInit(1.0f, 0.70f, 0.42f);
	sky.aureoleStrength = 0.6f;
	sky.aureoleExponent = 8.0f;
	sky.skyScale = 1.0f;
	sky.sunElevationDegrees = 18.0f;
	sky.sunAzimuthDegrees = 135.0f;
	sky.sunTint = V3fInit(1.0f, 0.76f, 0.52f);
	sky.sunIlluminance = 5.0f;
	sky.sunAngularRadiusDegrees = 1.5f; // 5.6x the real sun: 31x less variance for glossy and caustic paths, still crisp contact shadows
	scene.SetEnvironmentSky(sky);

	const Vec3f eye = V3fInit(-0.8f, -14.0f, 2.4f);
	const Vec3f target = V3fInit(0.0f, 3.0f, 0.9f);
	constexpr f32 fovYDegrees = 32.0f;
	constexpr f32 apertureRadius = 0.07f;
	constexpr f32 focusDistance = 17.16f; // depth of the rough gold sphere along the view axis
	constexpr f32 goldenHourExposureEV = 0.4f;
	constexpr f32 goldenHourPhotonRadius = 0.05f;
	scene.SetCamera(eye, target, fovYDegrees, WideReferenceAspect, apertureRadius, focusDistance);
	scene.exposureEV = goldenHourExposureEV;
	scene.photonRadius = goldenHourPhotonRadius;
	scene.maxBounces = DefaultMaxBounces;
}

// Product shot in a black void: warm softbox key, overhead strip, colored orbs, glossy black floor
static void BuildNightStudioScene(Scene &scene) {
	const Vec3f floorBase = V3fInit(0.035f, 0.035f, 0.04f);
	const Vec3f pedestalBase = V3fInit(0.62f, 0.60f, 0.57f);
	const Vec3f redBase = V3fInit(0.65f, 0.05f, 0.04f);
	const Vec3f heroGlassAbsorption = V3fInit(0.0834f, 0.0305f, 0.0408f); // transmittance (0.92, 0.97, 0.96) per unit, a faint aqua
	constexpr f32 floorCoatAlpha = 0.06f;
	constexpr f32 pedestalCoatAlpha = 0.35f;
	constexpr f32 redCoatAlpha = 0.03f;
	constexpr f32 goldAlpha = 0.10f;
	constexpr f32 copperAlpha = 0.20f;

	scene.name = "Night Studio";
	scene.fileName = "night_studio";
	u32 floorMaterial = scene.AddPlastic(floorBase, floorCoatAlpha, CoatIor);
	u32 pedestalMaterial = scene.AddPlastic(pedestalBase, pedestalCoatAlpha, CoatIor);
	u32 redMaterial = scene.AddPlastic(redBase, redCoatAlpha, CoatIor);
	u32 goldMaterial = scene.AddConductor(GoldF0, goldAlpha);
	u32 copperMaterial = scene.AddConductor(CopperF0, copperAlpha);
	u32 chromeMaterial = scene.AddMirror(ChromiumF0);
	u32 glassMaterial = scene.AddDielectric(GlassIor, heroGlassAbsorption);

	scene.AddPlane(GroundNormal, GroundPoint, floorMaterial);

	constexpr f32 heroRadius = 1.0f;
	constexpr f32 goldRadius = 0.8f;
	constexpr f32 copperRadius = 0.4f;
	constexpr f32 redRadius = 0.3f;
	constexpr f32 chromeRadius = 0.5f;
	const Vec3f heroCenter = V3fInit(0.0f, 1.2f, heroRadius);
	const Vec3f goldCenter = V3fInit(-2.2f, 0.5f, goldRadius);
	const Vec3f copperCenter = V3fInit(1.2f, -0.9f, copperRadius);
	const Vec3f redCenter = V3fInit(-0.7f, -1.25f, redRadius);
	scene.AddSphere(heroCenter, heroRadius, glassMaterial);
	scene.AddSphere(goldCenter, goldRadius, goldMaterial);
	scene.AddSphere(copperCenter, copperRadius, copperMaterial);
	scene.AddSphere(redCenter, redRadius, redMaterial);

	constexpr f32 pedestalHalfSide = 0.6f;
	constexpr f32 pedestalHalfHeight = 0.675f;
	constexpr f32 pedestalRotationDegrees = 30.0f;
	constexpr f32 pedestalTop = pedestalHalfHeight * 2.0f;
	constexpr f32 chromeCenterZ = pedestalTop + chromeRadius;
	const Vec3f pedestalCenter = V3fInit(2.2f, 1.9f, pedestalHalfHeight);
	const Vec3f pedestalHalfExtents = V3fInit(pedestalHalfSide, pedestalHalfSide, pedestalHalfHeight);
	const Vec3f chromeCenter = V3fInit(2.2f, 1.9f, chromeCenterZ);
	scene.AddBox(pedestalCenter, pedestalHalfExtents, pedestalRotationDegrees, pedestalMaterial);
	scene.AddSphere(chromeCenter, chromeRadius, chromeMaterial);

	const Vec3f keyRadiance = V3fInit(24.0f, 20.0f, 15.0f);
	const Vec3f stripRadiance = V3fInit(7.0f, 7.2f, 7.6f);
	const Vec3f cyanRadiance = V3fInit(0.5f, 2.2f, 4.0f);
	const Vec3f magentaRadiance = V3fInit(4.0f, 0.6f, 2.6f);
	u32 keyMaterial = scene.AddEmissive(keyRadiance);
	u32 stripMaterial = scene.AddEmissive(stripRadiance);
	u32 cyanMaterial = scene.AddEmissive(cyanRadiance);
	u32 magentaMaterial = scene.AddEmissive(magentaRadiance);

	constexpr f32 keyWidth = 2.6f;
	constexpr f32 keyHeight = 1.6f;
	const Vec3f keyCenter = V3fInit(-3.6f, -2.4f, 4.2f);
	const Vec3f keyTarget = V3fInit(0.0f, 0.9f, 0.8f);
	scene.AddQuadFacing(keyCenter, keyTarget, keyWidth, keyHeight, keyMaterial);

	const Vec3f stripCorner = V3fInit(-2.2f, 2.6f, 5.0f);
	const Vec3f stripEdgeU = V3fInit(0.0f, 0.35f, 0.0f);
	const Vec3f stripEdgeV = V3fInit(4.4f, 0.0f, 0.0f);
	scene.AddQuad(stripCorner, stripEdgeU, stripEdgeV, stripMaterial); // normal -Z

	constexpr f32 orbRadius = 0.35f;
	const Vec3f cyanCenter = V3fInit(-4.4f, 4.4f, 2.2f);
	const Vec3f magentaCenter = V3fInit(4.3f, 4.8f, 2.4f);
	scene.AddSphere(cyanCenter, orbRadius, cyanMaterial);
	scene.AddSphere(magentaCenter, orbRadius, magentaMaterial);

	scene.SetEnvironmentBlack();

	const Vec3f eye = V3fInit(0.4f, -7.2f, 1.7f);
	const Vec3f target = V3fInit(0.0f, 1.0f, 0.85f);
	constexpr f32 fovYDegrees = 32.0f;
	constexpr f32 apertureRadius = 0.08f;
	constexpr f32 focusDistance = 7.64f; // front surface of the glass sphere
	constexpr f32 nightStudioExposureEV = 0.4f;
	constexpr f32 nightStudioPhotonRadius = 0.05f; // a wider merge radius helps the glossy floor and metals (it still shrinks every pass)
	scene.SetCamera(eye, target, fovYDegrees, WideReferenceAspect, apertureRadius, focusDistance);
	scene.exposureEV = nightStudioExposureEV;
	scene.photonRadius = nightStudioPhotonRadius;
	scene.maxBounces = DefaultMaxBounces;
}

// Tribute to the "Ray Tracing in One Weekend" cover with real sky light, about 470 spheres (needs the BVH)
static void BuildSphereFieldScene(Scene &scene) {
	constexpr f32 bigRadius = 1.0f;
	constexpr f32 smallRadius = 0.2f;
	constexpr s32 gridMin = -11;
	constexpr s32 gridMax = 10;
	constexpr f32 cellInset = 0.225f;
	constexpr f32 cellJitter = 0.55f; // with 0.9 (as in the book) neighbors could intersect
	constexpr f32 bigSphereClearance = 1.35f;
	constexpr f32 diffuseThreshold = 0.75f;
	constexpr f32 conductorThreshold = 0.92f;
	constexpr f32 conductorMinF0 = 0.5f;
	constexpr f32 conductorMinAlpha = 0.08f; // smaller alphas leave speckle halos in sunlight
	constexpr f32 conductorAlphaRange = 0.3f;
	constexpr u64 sphereFieldSeed = 1337;

	scene.name = "Sphere Field";
	scene.fileName = "sphere_field";

	const Vec3f groundAlbedo = V3fInit(0.5f, 0.5f, 0.5f);
	u32 groundMaterial = scene.AddDiffuse(groundAlbedo);
	scene.AddPlane(GroundNormal, GroundPoint, groundMaterial);

	const Vec3f brownAlbedo = V3fInit(0.4f, 0.2f, 0.1f);
	const Vec3f bronzeF0 = V3fInit(0.7f, 0.6f, 0.5f);
	constexpr f32 bronzeAlpha = 0.02f;
	u32 glassMaterial = scene.AddDielectric(GlassIor, ClearAbsorption);
	u32 brownMaterial = scene.AddDiffuse(brownAlbedo);
	u32 bronzeMaterial = scene.AddConductor(bronzeF0, bronzeAlpha);
	const Vec3f glassCenter = V3fInit(0.0f, 0.0f, bigRadius);
	const Vec3f brownCenter = V3fInit(-4.0f, 0.0f, bigRadius);
	const Vec3f bronzeCenter = V3fInit(4.0f, 0.0f, bigRadius);
	scene.AddSphere(glassCenter, bigRadius, glassMaterial);
	scene.AddSphere(brownCenter, bigRadius, brownMaterial);
	scene.AddSphere(bronzeCenter, bigRadius, bronzeMaterial);
	const Vec3f bigCenters[] = { glassCenter, brownCenter, bronzeCenter };

	PathSampler random;
	random.state = sphereFieldSeed;
	for (s32 gridA = gridMin; gridA <= gridMax; ++gridA) {
		for (s32 gridB = gridMin; gridB <= gridMax; ++gridB) {
			f32 materialChoice = PathSamplerNext01(random);
			f32 jitterX = PathSamplerNext01(random);
			f32 jitterY = PathSamplerNext01(random);
			f32 centerX = (f32)gridA + cellInset + cellJitter * jitterX;
			f32 centerY = (f32)gridB + cellInset + cellJitter * jitterY;
			Vec3f center = V3fInit(centerX, centerY, smallRadius);
			bool isTooClose = false;
			for (u32 bigIndex = 0; bigIndex < fplArrayCount(bigCenters); ++bigIndex) {
				f32 deltaX = centerX - bigCenters[bigIndex].x;
				f32 deltaY = centerY - bigCenters[bigIndex].y;
				f32 planarDistanceSquared = deltaX * deltaX + deltaY * deltaY;
				if (planarDistanceSquared < bigSphereClearance * bigSphereClearance) {
					isTooClose = true;
				}
			}
			if (isTooClose) {
				continue;
			}
			u32 materialIndex;
			if (materialChoice < diffuseThreshold) {
				// Squared random numbers favor darker, more saturated colors
				Vec3f albedo;
				for (u32 channel = 0; channel < 3; ++channel) {
					f32 first = PathSamplerNext01(random);
					f32 second = PathSamplerNext01(random);
					albedo.m[channel] = first * second;
				}
				materialIndex = scene.AddDiffuse(albedo);
			} else if (materialChoice < conductorThreshold) {
				Vec3f f0;
				for (u32 channel = 0; channel < 3; ++channel) {
					f32 channelRandom = PathSamplerNext01(random);
					f0.m[channel] = conductorMinF0 + (1.0f - conductorMinF0) * channelRandom;
				}
				f32 alphaRandom = PathSamplerNext01(random);
				f32 alpha = conductorMinAlpha + conductorAlphaRange * alphaRandom;
				materialIndex = scene.AddConductor(f0, alpha);
			} else {
				materialIndex = glassMaterial;
			}
			scene.AddSphere(center, smallRadius, materialIndex);
		}
	}

	SkyDesc sky = {};
	sky.zenithRadiance = V3fInit(0.13f, 0.26f, 0.58f);
	sky.horizonRadiance = V3fInit(0.78f, 0.74f, 0.70f);
	sky.belowHorizonRadiance = V3fInit(0.18f, 0.16f, 0.14f);
	sky.horizonExponent = 4.0f;
	sky.aureoleTint = V3fInit(1.0f, 0.70f, 0.42f);
	sky.aureoleStrength = 0.6f;
	sky.aureoleExponent = 8.0f;
	sky.skyScale = 1.0f;
	sky.sunElevationDegrees = 22.0f;
	sky.sunAzimuthDegrees = 160.0f;
	sky.sunTint = V3fInit(1.0f, 0.76f, 0.52f);
	sky.sunIlluminance = 5.0f;
	sky.sunAngularRadiusDegrees = 1.5f;
	scene.SetEnvironmentSky(sky);

	const Vec3f eye = V3fInit(13.0f, -3.0f, 2.0f);
	const Vec3f target = V3fInit(0.0f, 0.0f, 0.0f);
	constexpr f32 fovYDegrees = 20.0f;
	constexpr f32 apertureRadius = 0.1f;
	constexpr f32 focusDistance = 10.0f;
	constexpr f32 sphereFieldExposureEV = 0.4f;
	constexpr u32 sphereFieldMaxBounces = 10;
	constexpr f32 sphereFieldPhotonRadius = 0.03f;
	scene.SetCamera(eye, target, fovYDegrees, WideReferenceAspect, apertureRadius, focusDistance);
	scene.photonRadius = sphereFieldPhotonRadius;
	scene.exposureEV = sphereFieldExposureEV;
	scene.maxBounces = sphereFieldMaxBounces;
}

static constexpr u32 SceneCount = 6;
static constexpr u32 DefaultSceneIndex = 1;

// Index 0 is the furnace test, 1..5 are the showcase scenes (keys 0..5)
static void BuildScenes(SceneEntry *entries) {
	BuildWhiteFurnaceScene(entries[0].scene);
	entries[0].tonemapOperator = TonemapOperator::None;
	BuildCornellBoxScene(entries[1].scene, false);
	entries[1].tonemapOperator = TonemapOperator::AcesFitted;
	BuildGoldenHourScene(entries[2].scene);
	entries[2].tonemapOperator = TonemapOperator::AcesFitted;
	BuildNightStudioScene(entries[3].scene);
	entries[3].tonemapOperator = TonemapOperator::AcesFitted;
	BuildCornellBoxScene(entries[4].scene, true);
	entries[4].tonemapOperator = TonemapOperator::AcesFitted;
	BuildSphereFieldScene(entries[5].scene);
	entries[5].tonemapOperator = TonemapOperator::AcesFitted;
	for (u32 sceneIndex = 0; sceneIndex < SceneCount; ++sceneIndex) {
		entries[sceneIndex].scene.Bake();
	}
}

//
// Vertex connection and merging (VCM) for caustic paths
//
// Caustic paths end with a non-specular vertex followed by one or more specular bounces into a light. Three strategies sample them:
// - the path tracer: it hits the light by chance through the specular chain
// - light tracing: a photon through the specular chain, connected to the camera (only when the camera sees a non-specular surface first)
// - photon merging: photons around the camera path's first non-specular vertex (offered behind mirrors and glass, and on sharp lobes seen directly)
// All three weight their contribution with the power heuristic. The density ratios to the path tracer are products over the vertices of light side area density / camera side area density, built incrementally on both walks.
// Specular vertices make their neighbor's density 1 on that side (the delta factors cancel, as in PBRT's BDPT). Merging accepts a photon within the radius, which adds the disc area pi r^2 (Georgiev et al. 2012).
// The sample counts are one camera path per pixel and the photons per pass for both light tracing and merging.
//
static constexpr f64 MaxWeightedRatio = 1.0e150; // keeps the squared ratios finite

struct BidirectionalContext {
	f64 lightPathCount;     // photons per pass, 0 = light tracing off
	f64 mergeArea;          // pi r^2 of this pass's photon map radius
	f32 cameraDirectionPdf; // solid angle density of the camera ray for its pixel: tent filter density / (pixel film area * cos^3)
};

struct BidirectionalWeights {
	f64 pathTracing;
	f64 lightTracing;
	f64 merging;
};

// Power heuristic over the three strategies; the ratios are light tracing and merging densities relative to the path tracer (0 = strategy not available for this path).
// Every strategy computes the same ratios for the same path, so the weights always sum to one.
static inline BidirectionalWeights ComputeBidirectionalWeights(const f64 lightTracingRatio, const f64 mergeRatio, const f64 lightPathCount) {
	f64 lightTerm = lightTracingRatio * lightPathCount;
	f64 mergeTerm = mergeRatio * lightPathCount;
	// NaN guard, shared by all strategies: an undefined ratio means the strategy is not counted
	if (!(lightTerm >= 0.0)) {
		lightTerm = 0.0;
	}
	if (!(mergeTerm >= 0.0)) {
		mergeTerm = 0.0;
	}
	lightTerm = fplMin(lightTerm, MaxWeightedRatio);
	mergeTerm = fplMin(mergeTerm, MaxWeightedRatio);
	f64 lightSquared = lightTerm * lightTerm;
	f64 mergeSquared = mergeTerm * mergeTerm;
	f64 inverseSum = 1.0 / (1.0 + lightSquared + mergeSquared);
	BidirectionalWeights result;
	result.pathTracing = inverseSum;
	result.lightTracing = lightSquared * inverseSum;
	result.merging = mergeSquared * inverseSum;
	return(result);
}

// Solid angle density of the caster aimed photon emission from origin in direction, without the rule that a sampled cone always contains its own sample
static f32 CasterDirectionDensity(const SceneView &scene, const Vec3f &origin, const Vec3f &direction) {
	f32 totalWeight = 0.0f;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		SphereCone cone = ComputeSphereCone(origin, caster.center, caster.radius);
		if (!cone.isInside) {
			totalWeight += cone.oneMinusCosThetaMax;
		}
	}
	if (!(totalWeight > 0.0f)) {
		return(0.0f);
	}
	f32 result = 0.0f;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		SphereCone cone = ComputeSphereCone(origin, caster.center, caster.radius);
		if (cone.isInside) {
			continue;
		}
		f32 cosToAxis = V3fDot(direction, cone.axis);
		f32 cosThetaMax = 1.0f - cone.oneMinusCosThetaMax;
		if (cosToAxis >= cosThetaMax) {
			f32 selectionProbability = cone.oneMinusCosThetaMax / totalWeight;
			f32 conePdf = ConePdf(cone.oneMinusCosThetaMax);
			result += selectionProbability * conePdf;
		}
	}
	return(result);
}

// Area density of sun photon lines through point (on the plane perpendicular to travel): caster discs covering the line, each 1 / (pi sum r^2)
static f32 SunCasterAreaDensity(const SceneView &scene, const Vec3f &point, const Vec3f &travel) {
	if (!(scene.casterAreaSum > 0.0f)) {
		return(0.0f);
	}
	u32 coverCount = 0;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		Vec3f toCenter = caster.center - point;
		f32 along = V3fDot(toCenter, travel);
		f32 lengthSquared = V3fDot(toCenter, toCenter);
		f32 perpendicularSquared = lengthSquared - along * along;
		if (perpendicularSquared <= caster.radius * caster.radius) {
			++coverCount;
		}
	}
	f32 result = (f32)coverCount / (F32Pi * scene.casterAreaSum);
	return(result);
}

// Density of the light vertex itself: selection probability times the area density (sphere and quad lights) or the direction density (sun)
static f64 LightVertexDensity(const SceneView &scene, const Light &light) {
	f64 selection = (f64)light.selectionProbability;
	switch (light.kind) {
		case LightKind::Sphere:
		{
			f32 radius = scene.primitives[light.primitiveIndex].sphere.radius;
			f64 area = 4.0 * (f64)F32Pi * (f64)radius * (f64)radius;
			f64 result = selection / area;
			return(result);
		}
		case LightKind::Quad:
		{
			f64 area = (f64)scene.primitives[light.primitiveIndex].quad.area;
			f64 result = selection / area;
			return(result);
		}
		case LightKind::Sun:
		{
			f64 result = selection * (f64)light.conePdf;
			return(result);
		}
		default:
			return(0.0);
	}
}

//
// Progressive photon mapping for caustics seen through mirrors and glass
//
// Light tracing cannot connect to the camera through a specular surface, so a caustic seen in a mirror or through glass would only be found by chance.
// Photons stored at every non-specular vertex behind the light's specular chain give a density estimate of that light at the first non-specular vertex of such camera paths.
// The radius shrinks every pass (Knaus and Zwicker 2011, probabilistic progressive photon mapping), so the estimate is consistent: its bias vanishes as passes accumulate.
//
static constexpr f64 PhotonRadiusShrinkAlpha = 2.0 / 3.0;
static constexpr f32 PhotonPlaneToleranceFactor = 0.25f;  // photons farther from the tangent plane than this fraction of the radius belong to another surface
static constexpr u32 PhotonGridMinTableSize = 1024;
static constexpr f32 PhotonCellsPerRadius = 0.5f;          // cell size = 2 * radius, so 2x2x2 cells cover the search sphere
static constexpr u32 PhotonCellsPerAxis = 2;
static constexpr u32 PhotonVisitedCellCount = 8;
static constexpr f32 PhotonCellCoordinateLimit = 1.0e9f;   // keeps far away hit points inside the integer range of the cell coordinates
static constexpr u32 PhotonHashPrimeX = 73856093u;
static constexpr u32 PhotonHashPrimeY = 19349663u;
static constexpr u32 PhotonHashPrimeZ = 83492791u;
static constexpr u32 PhotonBufferCount = 2;                // light jobs of pass e write buffer e % 2, while pass e builds its map from buffer (e - 1) % 2

struct PhotonRecord {
	Vec3f position;
	Vec3f towardsOrigin;      // unit, the direction the photon came from
	Vec3f power;              // photon throughput / photons per pass
	u32 vertexCount;          // scattering vertices of the photon path including this one
	f32 misPartialRatio;      // light / camera side density product of the photon path, without the camera side density of its previous vertex (needs the merge vertex BSDF)
	f32 misPreviousGeometry;  // cos(previous vertex) / distance^2 of the last segment, converts that camera side pdf to area density
};

struct PhotonMapView {
	const PhotonRecord *const *photons; // sorted by grid cell
	const u32 *cellStarts;              // tableMask + 2 entries
	u32 tableMask;
	f32 radius;
	f32 inverseCellSize;
};

// Light jobs fill their own job buffer, the photon map job at the start of the next pass builds the grid
struct PhotonStorage {
	std::vector<std::vector<PhotonRecord> > jobPhotons[PhotonBufferCount];
	std::vector<const PhotonRecord *> sortedPhotons;
	std::vector<u32> photonHashes;
	std::vector<u32> cellStarts;
	std::vector<u32> cellCursors;
	PhotonMapView view;
	volatile u32 readyEpoch; // epoch + 1 of the pass the view belongs to, 0 = none
};

static inline s64 PhotonCellCoordinate(const f32 value, const f32 inverseCellSize) {
	f32 scaled = value * inverseCellSize;
	f32 clamped = F32Clamp(scaled, -PhotonCellCoordinateLimit, PhotonCellCoordinateLimit);
	f32 floored = F32Floor(clamped);
	s64 result = (s64)floored;
	return(result);
}

static inline u32 PhotonCellHash(const s64 cellX, const s64 cellY, const s64 cellZ, const u32 tableMask) {
	u32 hashX = (u32)cellX * PhotonHashPrimeX;
	u32 hashY = (u32)cellY * PhotonHashPrimeY;
	u32 hashZ = (u32)cellZ * PhotonHashPrimeZ;
	u32 result = (hashX ^ hashY ^ hashZ) & tableMask;
	return(result);
}

// r_e^2 = r_1^2 * prod_{i=1}^{e-1} (i + alpha) / (i + 1) = r_1^2 * Gamma(e + alpha) / (Gamma(1 + alpha) * Gamma(e + 1))
static f32 PhotonRadiusForEpoch(const f32 initialRadius, const u32 epoch) {
	u32 passNumber = fplMax(epoch, 1u);
	f64 pass = (f64)passNumber;
	f64 logGammaNumerator = lgamma(pass + PhotonRadiusShrinkAlpha);
	f64 logGammaAlpha = lgamma(1.0 + PhotonRadiusShrinkAlpha);
	f64 logGammaDenominator = lgamma(pass + 1.0);
	f64 logShrink = logGammaNumerator - logGammaAlpha - logGammaDenominator;
	f64 shrink = exp(logShrink);
	f64 radiusSquared = (f64)initialRadius * (f64)initialRadius * shrink;
	f64 radius = sqrt(radiusSquared);
	f32 result = (f32)radius;
	return(result);
}

static f64 MergeAreaForEpoch(const f32 initialRadius, const u32 epoch) {
	f32 radius = PhotonRadiusForEpoch(initialRadius, epoch);
	f64 result = (f64)F32Pi * (f64)radius * (f64)radius;
	return(result);
}

// Counting sort of all photons of the previous pass into a hashed grid, in job order, so the map is identical for any thread count
static void BuildPhotonMap(PhotonStorage &storage, const u32 bufferIndex, const f32 radius) {
	std::vector<std::vector<PhotonRecord> > &jobs = storage.jobPhotons[bufferIndex];
	size_t jobCount = jobs.size();
	size_t photonCount = 0;
	for (size_t jobIndex = 0; jobIndex < jobCount; ++jobIndex) {
		photonCount += jobs[jobIndex].size();
	}
	u32 tableSize = PhotonGridMinTableSize;
	while ((size_t)tableSize < 2 * photonCount) {
		tableSize <<= 1;
	}
	u32 tableMask = tableSize - 1;
	f32 inverseCellSize = PhotonCellsPerRadius / radius;
	storage.photonHashes.resize(photonCount);
	storage.cellStarts.assign(tableSize + 1, 0);
	size_t flatIndex = 0;
	for (size_t jobIndex = 0; jobIndex < jobCount; ++jobIndex) {
		const std::vector<PhotonRecord> &records = jobs[jobIndex];
		for (size_t recordIndex = 0; recordIndex < records.size(); ++recordIndex) {
			const Vec3f &position = records[recordIndex].position;
			s64 cellX = PhotonCellCoordinate(position.x, inverseCellSize);
			s64 cellY = PhotonCellCoordinate(position.y, inverseCellSize);
			s64 cellZ = PhotonCellCoordinate(position.z, inverseCellSize);
			u32 hash = PhotonCellHash(cellX, cellY, cellZ, tableMask);
			storage.photonHashes[flatIndex++] = hash;
			++storage.cellStarts[hash + 1];
		}
	}
	for (u32 tableIndex = 1; tableIndex <= tableSize; ++tableIndex) {
		storage.cellStarts[tableIndex] += storage.cellStarts[tableIndex - 1];
	}
	storage.cellCursors.assign(storage.cellStarts.begin(), storage.cellStarts.end() - 1);
	storage.sortedPhotons.resize(photonCount);
	flatIndex = 0;
	for (size_t jobIndex = 0; jobIndex < jobCount; ++jobIndex) {
		const std::vector<PhotonRecord> &records = jobs[jobIndex];
		for (size_t recordIndex = 0; recordIndex < records.size(); ++recordIndex) {
			u32 hash = storage.photonHashes[flatIndex++];
			u32 sortedIndex = storage.cellCursors[hash]++;
			storage.sortedPhotons[sortedIndex] = &records[recordIndex];
		}
	}
	storage.view.photons = storage.sortedPhotons.data();
	storage.view.cellStarts = storage.cellStarts.data();
	storage.view.tableMask = tableMask;
	storage.view.radius = radius;
	storage.view.inverseCellSize = inverseCellSize;
}

// Photon merging at the camera path's first non-specular vertex: each photon within the radius forms a full caustic path with the camera prefix, weighted against path tracing and light tracing.
// cameraDensity: camera side area density of the merge vertex when it is the camera hit (light tracing possible), 0 behind mirrors and glass
static Vec3f GatherPhotonRadiance(const PhotonMapView &map, const Material &material, const Vec3f &baseColor, const Vec3f &position, const ShadingFrame &frame, const Vec3f &wo, const u32 maxVertexCount, const f64 cameraDensity, const f64 lightPathCount) {
	f32 radius = map.radius;
	f32 radiusSquared = radius * radius;
	f64 mergeArea = (f64)F32Pi * (f64)radiusSquared;
	f32 planeTolerance = PhotonPlaneToleranceFactor * radius;
	s64 baseCellX = PhotonCellCoordinate(position.x - radius, map.inverseCellSize);
	s64 baseCellY = PhotonCellCoordinate(position.y - radius, map.inverseCellSize);
	s64 baseCellZ = PhotonCellCoordinate(position.z - radius, map.inverseCellSize);
	u32 visitedHashes[PhotonVisitedCellCount];
	u32 visitedCount = 0;
	Vec3f sum = V3fZero();
	for (u32 offsetZ = 0; offsetZ < PhotonCellsPerAxis; ++offsetZ) {
		for (u32 offsetY = 0; offsetY < PhotonCellsPerAxis; ++offsetY) {
			for (u32 offsetX = 0; offsetX < PhotonCellsPerAxis; ++offsetX) {
				u32 hash = PhotonCellHash(baseCellX + offsetX, baseCellY + offsetY, baseCellZ + offsetZ, map.tableMask);
				// Two cells can share a hash bucket, a bucket must be gathered only once
				bool isVisited = false;
				for (u32 visitedIndex = 0; visitedIndex < visitedCount; ++visitedIndex) {
					if (visitedHashes[visitedIndex] == hash) {
						isVisited = true;
					}
				}
				if (isVisited) {
					continue;
				}
				visitedHashes[visitedCount++] = hash;
				u32 bucketEnd = map.cellStarts[hash + 1];
				for (u32 sortedIndex = map.cellStarts[hash]; sortedIndex < bucketEnd; ++sortedIndex) {
					const PhotonRecord &photon = *map.photons[sortedIndex];
					if (photon.vertexCount > maxVertexCount) {
						continue;
					}
					Vec3f offset = photon.position - position;
					f32 distanceSquared = V3fDot(offset, offset);
					if (distanceSquared > radiusSquared) {
						continue;
					}
					f32 planeDistance = V3fDot(offset, frame.normal);
					if (F32Abs(planeDistance) > planeTolerance) {
						continue;
					}
					Vec3f wi = ToLocal(frame, photon.towardsOrigin);
					if (wi.z <= 0.0f) {
						continue;
					}
					Vec3f bsdfValue = EvaluateBsdf(material, baseColor, wo, wi);
					if (IsBlack(bsdfValue)) {
						continue;
					}
					// The camera side density of the photon's previous vertex: the path tracer would sample it from here, coming from the camera
					f32 cameraSidePdf = BsdfPdf(material, baseColor, wo, wi);
					f64 previousCameraDensity = (f64)cameraSidePdf * (f64)photon.misPreviousGeometry;
					f64 photonRatio = (f64)photon.misPartialRatio / previousCameraDensity;
					f64 mergeRatio = photonRatio * mergeArea;
					f64 lightTracingRatio = (cameraDensity > 0.0) ? (photonRatio / cameraDensity) : 0.0;
					BidirectionalWeights weights = ComputeBidirectionalWeights(lightTracingRatio, mergeRatio, lightPathCount);
					Vec3f weighted = V3fHadamard(bsdfValue, photon.power);
					sum += (f32)weights.merging * weighted;
				}
			}
		}
	}
	f32 inverseDiscArea = (f32)(1.0 / mergeArea);
	Vec3f result = inverseDiscArea * sum;
	return(result);
}

//
// Integrator: unidirectional path tracer with next event estimation, multiple importance sampling and russian roulette
//
enum class IntegratorMode : u32 {
	Mis = 0,   // light sampling and BSDF sampling combined with the power heuristic
	LightOnly, // debug: emitters found by BSDF sampling after a non-delta bounce are ignored
	BsdfOnly,  // debug: no light sampling
	Count,
};

static const char *IntegratorModeNames[] = {
	"MIS",
	"NEE",
	"BSDF",
};
fplStaticAssert(fplArrayCount(IntegratorModeNames) == (u32)IntegratorMode::Count);

struct RenderSettings {
	u32 maxBounces;                 // scattering events
	u32 russianRouletteStartBounce; // bounces below this are never terminated by russian roulette
	f32 russianRouletteMinSurvival;
	f32 russianRouletteMaxSurvival;
	f32 indirectClampLuminance;     // 0 = off; biased when on, removes energy from rare high value paths
	IntegratorMode mode;
	b32 isLightTracingActive;       // light tracing and photon mapping run for this generation (caustic paths are shared with them)
};

struct PathStats {
	u32 rayCount;
};

static constexpr u32 RussianRouletteStartBounce = 3;
static constexpr f32 RussianRouletteMinSurvival = 0.05f; // caps the boost at 20x
static constexpr f32 RussianRouletteMaxSurvival = 0.95f; // guarantees termination of throughput-1 loops (glass, furnace)
static constexpr f32 ShadowDistanceScale = 0.999f;       // shadow rays stop just before the sampled light point
static constexpr f32 FireflyClampLuminance = 20.0f;

// Robust ratio form of the power heuristic (beta = 2): avoids pdf^2 overflow, 0 for pdfA == 0, 1 for pdfB == 0
static inline f32 PowerHeuristic(const f32 pdfA, const f32 pdfB) {
	if (!(pdfA > 0.0f)) {
		return(0.0f);
	}
	f32 ratio = pdfB / pdfA;
	f32 result = 1.0f / (1.0f + ratio * ratio);
	return(result);
}

// MIS weight of an emitter found by BSDF sampling, full weight on camera rays and after delta bounces
static inline f32 EmitterHitMisWeight(const RenderSettings &settings, const bool previousWasDelta, const f32 previousBsdfPdf, const f32 lightPdfWithSelection) {
	if (previousWasDelta) {
		return(1.0f);
	}
	if (settings.mode == IntegratorMode::BsdfOnly) {
		return(1.0f);
	}
	if (settings.mode == IntegratorMode::LightOnly) {
		return(0.0f);
	}
	f32 result = PowerHeuristic(previousBsdfPdf, lightPdfWithSelection);
	return(result);
}

static inline Vec3f ClampIndirect(const RenderSettings &settings, const Vec3f &contribution, const bool isIndirect) {
	if (!isIndirect || settings.indirectClampLuminance <= 0.0f) {
		return(contribution);
	}
	f32 luminance = Luminance(contribution);
	if (luminance <= settings.indirectClampLuminance) {
		return(contribution);
	}
	f32 scale = settings.indirectClampLuminance / luminance;
	Vec3f result = scale * contribution;
	return(result);
}

// photonMap: caustic density estimate for paths that reach their first non-specular vertex through mirrors or glass, null in the first pass and without light tracing
static Vec3f TracePath(const SceneView &scene, const RenderSettings &settings, const BidirectionalContext &bidirectional, const PhotonMapView *photonMap, Ray3f ray, PathSampler &sampler, PathStats &stats) {
	const Environment &environment = *scene.environment;
	Vec3f radiance = V3fZero();
	Vec3f throughput = V3fInit(1.0f, 1.0f, 1.0f);
	bool previousWasDelta = true;   // the camera ray behaves like a delta vertex: emission seen directly is added fully
	f32 previousBsdfPdf = 0.0f;     // solid angle pdf of the direction sampled at the previous vertex
	Vec3f previousPosition = ray.origin;
	Vec3f previousNormal = V3fZero();
	// Caustic paths (a non-specular vertex followed by specular bounces into a light) are shared with light tracing and photon merging by VCM weights.
	// Light tracing needs a non-specular camera hit, merging happens at the first non-specular vertex (the merge vertex).
	bool isLightTracingActive = settings.isLightTracingActive != 0;
	bool isBidirectionalPath = false;     // the camera hit is non-specular, so light tracing can sample this path
	f64 densityRatio = 1.0;               // light tracing / path tracing density of the finished vertices
	f32 previousCameraDensity = 0.0f;     // area density of the previous vertex from the camera side
	bool hasMergeVertex = false;
	bool isMergeOffered = false;          // merging is a strategy for this path (behind mirrors and glass, or a sharp lobe at the camera hit)
	u32 mergeVertexBounce = 0;
	f32 mergeVertexCameraDensity = 0.0f;
	f64 mergeDensityRatio = 1.0;          // light / camera side density of the finished vertices from the merge vertex on

	for (u32 bounceIndex = 0; ; ++bounceIndex) {
		bool isEmissionIndirect = bounceIndex >= 2;
		// A light hit right after a specular bounce, behind a non-specular vertex, ends a caustic path; the first pass has no photon map, so its merging share is missing (a 1/passes bias)
		bool isCausticTail = isLightTracingActive && hasMergeVertex && previousWasDelta && bounceIndex >= 2;
		SurfaceHit hit;
		++stats.rayCount;
		bool isHit = TraceSurface(scene, ray, UnboundedDistance, hit);
		Vec3f viewDirection = -ray.direction;
		f32 previousCosine = F32Abs(V3fDot(previousNormal, ray.direction));

		if (!isHit) {
			// Environment: weight 1 unless the uniform environment is also sampled as a light
			Vec3f environmentRadiance = EvaluateEnvironment(environment, ray.direction);
			f32 environmentWeight = 1.0f;
			if (environment.environmentLightIndex != NoIndex) {
				const Light &environmentLight = scene.lights[environment.environmentLightIndex];
				f32 environmentLightPdf = environmentLight.selectionProbability * UniformSpherePdf;
				environmentWeight = EmitterHitMisWeight(settings, previousWasDelta, previousBsdfPdf, environmentLightPdf);
			}
			Vec3f environmentContribution = environmentWeight * V3fHadamard(throughput, environmentRadiance);
			Vec3f clampedEnvironment = ClampIndirect(settings, environmentContribution, isEmissionIndirect);
			radiance += clampedEnvironment;
			// Sun disk: sampled by NEE, so MIS applies
			if (environment.sunLightIndex != NoIndex) {
				const Light &sun = scene.lights[environment.sunLightIndex];
				f32 cosToSun = V3fDot(ray.direction, sun.direction);
				if (cosToSun >= sun.cosThetaMax) {
					f32 sunLightPdf = sun.selectionProbability * sun.conePdf;
					f32 sunWeight = EmitterHitMisWeight(settings, previousWasDelta, previousBsdfPdf, sunLightPdf);
					if (isCausticTail) {
						// The sun photon line through the previous vertex, the sun vertex itself has camera side density 1 behind the specular vertex
						f32 sunAreaDensity = SunCasterAreaDensity(scene, previousPosition, viewDirection);
						f64 previousLightDensity = (f64)sunAreaDensity * (f64)previousCosine;
						f64 sunVertexDensity = LightVertexDensity(scene, sun);
						f64 tailRatio = (previousLightDensity / (f64)previousCameraDensity) * sunVertexDensity;
						f64 lightTracingRatio = isBidirectionalPath ? (densityRatio * tailRatio) : 0.0;
						f64 mergeRatio = isMergeOffered ? (mergeDensityRatio * tailRatio * (f64)mergeVertexCameraDensity * bidirectional.mergeArea) : 0.0;
						BidirectionalWeights weights = ComputeBidirectionalWeights(lightTracingRatio, mergeRatio, bidirectional.lightPathCount);
						sunWeight *= (f32)weights.pathTracing;
					}
					Vec3f sunContribution = sunWeight * V3fHadamard(throughput, sun.radiance);
					Vec3f clampedSun = ClampIndirect(settings, sunContribution, isEmissionIndirect);
					radiance += clampedSun;
				}
			}
			break;
		}

		const Material &material = scene.materials[hit.materialIndex];
		f32 hitCosine = F32Abs(V3fDot(hit.geometricNormal, ray.direction));
		f32 hitDistanceSquared = hit.distance * hit.distance;
		f32 cameraDensity = 1.0f; // area density of this vertex from the camera side (1 behind a specular vertex)
		if (bounceIndex == 0) {
			cameraDensity = bidirectional.cameraDirectionPdf * hitCosine / hitDistanceSquared;
		} else if (!previousWasDelta) {
			cameraDensity = previousBsdfPdf * hitCosine / hitDistanceSquared;
		}

		// Beer-Lambert: the segment that just ended at a back face of a dielectric ran inside the medium
		if (material.kind == MaterialKind::Dielectric && !hit.isFrontFace && material.hasAbsorption) {
			Vec3f opticalDepth = hit.distance * material.absorptionCoefficient;
			Vec3f transmittance = V3fExpNegative(opticalDepth);
			throughput = V3fHadamard(throughput, transmittance);
		}

		if (material.kind == MaterialKind::Emissive) {
			bool isEmittingSide = hit.isFrontFace || material.isTwoSided;
			if (isEmittingSide) {
				f32 emitterWeight = 1.0f;
				bool isLightListed = hit.lightIndex != NoIndex;
				if (isLightListed) {
					const Light &light = scene.lights[hit.lightIndex];
					f32 lightPdf = LightPdfForHit(scene, light, previousPosition, ray.direction, hit);
					f32 lightPdfWithSelection = light.selectionProbability * lightPdf;
					emitterWeight = EmitterHitMisWeight(settings, previousWasDelta, previousBsdfPdf, lightPdfWithSelection);
					if (isCausticTail) {
						// Light tracing emits from this point towards the casters, a direction on the back of a one-sided emitter is never emitted
						f32 emissionDensity = (hitCosine >= MinLightCosine) ? CasterDirectionDensity(scene, hit.position, viewDirection) : 0.0f;
						f64 previousLightDensity = (f64)emissionDensity * (f64)previousCosine / (f64)hitDistanceSquared;
						f64 lightVertexDensity = LightVertexDensity(scene, light);
						f64 tailRatio = (previousLightDensity / (f64)previousCameraDensity) * (lightVertexDensity / (f64)cameraDensity);
						f64 lightTracingRatio = isBidirectionalPath ? (densityRatio * tailRatio) : 0.0;
						f64 mergeRatio = isMergeOffered ? (mergeDensityRatio * tailRatio * (f64)mergeVertexCameraDensity * bidirectional.mergeArea) : 0.0;
						BidirectionalWeights weights = ComputeBidirectionalWeights(lightTracingRatio, mergeRatio, bidirectional.lightPathCount);
						emitterWeight *= (f32)weights.pathTracing;
					}
				}
				Vec3f emitterContribution = emitterWeight * V3fHadamard(throughput, material.emission);
				Vec3f clampedEmitter = ClampIndirect(settings, emitterContribution, isEmissionIndirect);
				radiance += clampedEmitter;
			}
			break; // emitters are black bodies, no scattering
		}

		// The bounce limit sits after the emission and before NEE: NEE at vertex b covers the same path lengths as the emission found at vertex b+1
		if (bounceIndex >= settings.maxBounces) {
			break;
		}

		Vec3f frameNormal = hit.isFrontFace ? hit.geometricNormal : -hit.geometricNormal;
		ShadingFrame frame = MakeShadingFrame(frameNormal);
		Vec3f wo = ToLocal(frame, viewDirection);
		if (wo.z < MinCosine) {
			break; // exactly grazing: measure zero, avoids divisions by wo.z
		}
		Vec3f baseColor = EvaluateBaseColor(material, hit.position);
		bool hasNonDeltaLobe = MaterialHasNonDeltaLobe(material);
		bool isMergeVertex = isLightTracingActive && !hasMergeVertex && hasNonDeltaLobe;
		bool isMergeOfferedHere = isMergeVertex && ((bounceIndex > 0) || HasSharpLobe(material));

		// Photon merging at the first non-specular vertex, weighted against path tracing and light tracing (the first pass has no photon map yet)
		if (isMergeOfferedHere && photonMap != fpl_null) {
			u32 maxPhotonVertexCount = settings.maxBounces - bounceIndex;
			f64 lightTracingCameraDensity = (bounceIndex == 0) ? (f64)cameraDensity : 0.0;
			Vec3f photonRadiance = GatherPhotonRadiance(*photonMap, material, baseColor, hit.position, frame, wo, maxPhotonVertexCount, lightTracingCameraDensity, bidirectional.lightPathCount);
			Vec3f photonContribution = V3fHadamard(throughput, photonRadiance);
			radiance += photonContribution;
		}

		// Next event estimation
		bool doLightSampling = settings.mode != IntegratorMode::BsdfOnly;
		if (doLightSampling && hasNonDeltaLobe && scene.lightCount > 0) {
			f32 uSelect = PathSamplerNext01(sampler);
			f32 u0 = PathSamplerNext01(sampler);
			f32 u1 = PathSamplerNext01(sampler);
			u32 lightIndex = SelectLight(scene, uSelect);
			const Light &light = scene.lights[lightIndex];
			LightSample lightSample;
			bool hasLightSample = SampleLight(scene, light, hit.position, u0, u1, lightSample);
			if (hasLightSample && lightSample.pdf > 0.0f && IsFiniteF32(lightSample.pdf)) {
				Vec3f wi = ToLocal(frame, lightSample.wi);
				Vec3f bsdfValue = EvaluateBsdf(material, baseColor, wo, wi);
				if (!IsBlack(bsdfValue)) {
					f32 lightPdf = light.selectionProbability * lightSample.pdf;
					f32 bsdfPdf = BsdfPdf(material, baseColor, wo, wi);
					f32 lightWeight = (settings.mode == IntegratorMode::Mis) ? PowerHeuristic(lightPdf, bsdfPdf) : 1.0f;
					Ray3f shadowRay = SpawnRay(hit.position, hit.geometricNormal, lightSample.wi);
					f32 shadowMaxDistance = (lightSample.distance < UnboundedDistance) ? (lightSample.distance * ShadowDistanceScale) : UnboundedDistance;
					++stats.rayCount;
					bool isOccluded = TraceOccluded(scene, shadowRay, shadowMaxDistance, light.primitiveIndex);
					if (!isOccluded) {
						f32 scale = wi.z * lightWeight / lightPdf;
						Vec3f unshadowed = scale * V3fHadamard(bsdfValue, lightSample.radiance);
						Vec3f lightContribution = V3fHadamard(throughput, unshadowed);
						bool isLightIndirect = bounceIndex >= 1;
						Vec3f clampedLight = ClampIndirect(settings, lightContribution, isLightIndirect);
						radiance += clampedLight;
					}
				}
			}
		}

		// BSDF sampling
		BsdfSample bsdfSample;
		bool hasBsdfSample = SampleBsdf(material, baseColor, wo, hit.isFrontFace, sampler, bsdfSample);
		if (!hasBsdfSample) {
			break;
		}
		bool isSampleDelta = bsdfSample.isDelta != 0;

		// The continuation is known now, so the previous vertex gets its light side density: light tracing would sample it from this vertex coming from the continuation
		if (isLightTracingActive && bounceIndex > 0) {
			f64 previousLightDensity = 1.0;
			if (!isSampleDelta) {
				f32 reversePdf = BsdfPdf(material, baseColor, bsdfSample.wi, wo);
				previousLightDensity = (f64)reversePdf * (f64)previousCosine / (f64)hitDistanceSquared;
			}
			f64 previousFactor = previousLightDensity / (f64)previousCameraDensity;
			densityRatio *= previousFactor;
			bool isPreviousFromMergeVertexOn = hasMergeVertex && (bounceIndex - 1 >= mergeVertexBounce);
			if (isPreviousFromMergeVertexOn) {
				mergeDensityRatio *= previousFactor;
			}
		}
		if (bounceIndex == 0) {
			isBidirectionalPath = isLightTracingActive && !isSampleDelta && bidirectional.cameraDirectionPdf > 0.0f;
		}
		if (isMergeVertex) {
			hasMergeVertex = true;
			isMergeOffered = isMergeOfferedHere;
			mergeVertexBounce = bounceIndex;
			mergeVertexCameraDensity = cameraDensity;
		}

		Vec3f worldDirection = ToWorld(frame, bsdfSample.wi);
		Vec3f newDirection = V3fNormalize(worldDirection);
		throughput = V3fHadamard(throughput, bsdfSample.weight);
		previousWasDelta = isSampleDelta;
		previousBsdfPdf = bsdfSample.pdf;
		previousPosition = hit.position;
		previousNormal = hit.geometricNormal;
		previousCameraDensity = cameraDensity;
		ray = SpawnRay(hit.position, hit.geometricNormal, newDirection);

		// Russian roulette after BSDF sampling: the continuation survives with probability q and is boosted by 1/q (unbiased for any q in (0,1])
		if (bounceIndex >= settings.russianRouletteStartBounce) {
			f32 maxThroughput = MaxComponent(throughput);
			f32 survival = F32Clamp(maxThroughput, settings.russianRouletteMinSurvival, settings.russianRouletteMaxSurvival);
			f32 uRoulette = PathSamplerNext01(sampler);
			if (uRoulette >= survival) {
				break;
			}
			throughput *= 1.0f / survival;
		}
	}
	return(radiance);
}

//
// Camera: orbit controller, vertical field of view with an aspect fit rule, thin lens with a planar focus, tent pixel filter
//
static constexpr f32 MaxPitchRadians = 1.5533430f; // 89 degrees, so cross(forward, up) never degenerates
static constexpr f32 TentFilterRadiusPixels = 1.0f;

struct OrbitCamera {
	Vec3f target;
	f32 yawRadians;
	f32 pitchRadians;
	f32 distance;
	f32 fovYRadians;
	f32 referenceAspect;
	f32 apertureRadius;
	f32 focusRatio; // focus distance / target distance, so zooming keeps the same depth in focus
};

struct CameraFrame {
	Vec3f eye;
	Vec3f forward;
	Vec3f right;
	Vec3f up;
	f32 tanHalfX;
	f32 tanHalfY;
	f32 apertureRadius;
	f32 focusDistance;
	f32 inverseOutputWidth;
	f32 inverseOutputHeight;
	f32 resolutionDivisor; // render pixel size in output pixels
};

static OrbitCamera MakeOrbitCamera(const SceneCameraDesc &desc) {
	Vec3f offset = desc.eye - desc.target;
	f32 distance = V3fLength(offset);
	fplAssert(distance > 0.0f);
	f32 heightRatio = offset.z / distance;
	OrbitCamera result = {};
	result.target = desc.target;
	result.distance = distance;
	result.yawRadians = F32ArcTan2(offset.y, offset.x);
	result.pitchRadians = F32ArcSin(heightRatio);
	result.fovYRadians = F32DegreesToRadians(desc.fovYDegrees);
	result.referenceAspect = desc.referenceAspect;
	result.apertureRadius = desc.apertureRadius;
	result.focusRatio = (desc.focusDistance > 0.0f) ? (desc.focusDistance / distance) : 1.0f;
	return(result);
}

static Vec3f ComputeOrbitOffset(const OrbitCamera &camera) {
	f32 pitch = F32Clamp(camera.pitchRadians, -MaxPitchRadians, MaxPitchRadians);
	f32 cosPitch = F32Cos(pitch);
	f32 sinPitch = F32Sin(pitch);
	f32 cosYaw = F32Cos(camera.yawRadians);
	f32 sinYaw = F32Sin(camera.yawRadians);
	Vec3f offsetDirection = V3fInit(cosPitch * cosYaw, cosPitch * sinYaw, sinPitch);
	Vec3f result = camera.distance * offsetDirection;
	return(result);
}

static CameraFrame MakeCameraFrame(const OrbitCamera &camera, const u32 outputWidth, const u32 outputHeight, const u32 resolutionDivisor) {
	fplAssert(outputWidth > 0 && outputHeight > 0);
	Vec3f offset = ComputeOrbitOffset(camera);
	Vec3f forward = (-1.0f / camera.distance) * offset;
	Vec3f rightRaw = V3fCross(forward, UnitUp);
	Vec3f right = V3fNormalize(rightRaw);
	Vec3f up = V3fCross(right, forward);
	f32 aspect = (f32)outputWidth / (f32)outputHeight;
	f32 tanHalfY = F32Tan(0.5f * camera.fovYRadians);
	if (aspect < camera.referenceAspect) {
		// Narrower window: widen vertically so the horizontal framing survives
		tanHalfY *= camera.referenceAspect / aspect;
	}
	CameraFrame result;
	result.eye = camera.target + offset;
	result.forward = forward;
	result.right = right;
	result.up = up;
	result.tanHalfY = tanHalfY;
	result.tanHalfX = tanHalfY * aspect;
	result.apertureRadius = camera.apertureRadius;
	result.focusDistance = camera.focusRatio * camera.distance;
	result.inverseOutputWidth = 1.0f / (f32)outputWidth;
	result.inverseOutputHeight = 1.0f / (f32)outputHeight;
	result.resolutionDivisor = (f32)resolutionDivisor;
	return(result);
}

// Maps u in [0,1) to an offset in (-1, 1) with the tent pdf 1 - |x| (filter importance sampling: every sample has weight 1)
static inline f32 SampleTentOffset(const f32 u) {
	f32 twiceU = 2.0f * u;
	if (twiceU < 1.0f) {
		f32 root = F32SquareRoot(twiceU);
		f32 result = root - 1.0f;
		return(result);
	}
	f32 mirrored = 2.0f - twiceU;
	f32 root = F32SquareRoot(mirrored);
	f32 result = 1.0f - root;
	return(result);
}

// Shirley-Chiu concentric mapping of [0,1)^2 to the unit disk (uniform round bokeh)
static inline Vec2f SampleConcentricDisk(const f32 u0, const f32 u1) {
	f32 offsetX = 2.0f * u0 - 1.0f;
	f32 offsetY = 2.0f * u1 - 1.0f;
	if (offsetX == 0.0f && offsetY == 0.0f) {
		Vec2f center = V2fInit(0.0f, 0.0f);
		return(center);
	}
	const f32 quarterPi = 0.25f * F32Pi;
	const f32 halfPi = 0.5f * F32Pi;
	f32 radius;
	f32 theta;
	if (offsetX * offsetX > offsetY * offsetY) {
		radius = offsetX;
		theta = quarterPi * (offsetY / offsetX);
	} else {
		radius = offsetY;
		theta = halfPi - quarterPi * (offsetX / offsetY);
	}
	f32 cosTheta = F32Cos(theta);
	f32 sinTheta = F32Sin(theta);
	Vec2f result = V2fInit(radius * cosTheta, radius * sinTheta);
	return(result);
}

// The frustum is defined in output pixels, so preview and full resolution show exactly the same view; rows are top-down.
// outFilterDensity: tent filter density of the film sample per render pixel area (for the bidirectional MIS camera density)
static Ray3f GenerateCameraRay(const CameraFrame &frame, const u32 pixelX, const u32 pixelY, PathSampler &sampler, f32 &outFilterDensity) {
	f32 filmU = PathSamplerNext01(sampler);
	f32 filmV = PathSamplerNext01(sampler);
	f32 lensU = PathSamplerNext01(sampler);
	f32 lensV = PathSamplerNext01(sampler);
	f32 tentX = SampleTentOffset(filmU);
	f32 tentY = SampleTentOffset(filmV);
	f32 absoluteTentX = F32Abs(tentX);
	f32 absoluteTentY = F32Abs(tentY);
	outFilterDensity = (1.0f - absoluteTentX) * (1.0f - absoluteTentY);
	f32 filmX = ((f32)pixelX + 0.5f + TentFilterRadiusPixels * tentX) * frame.resolutionDivisor;
	f32 filmY = ((f32)pixelY + 0.5f + TentFilterRadiusPixels * tentY) * frame.resolutionDivisor;
	f32 ndcX = 2.0f * filmX * frame.inverseOutputWidth - 1.0f;
	f32 ndcY = 1.0f - 2.0f * filmY * frame.inverseOutputHeight;
	Vec3f horizontal = (ndcX * frame.tanHalfX) * frame.right;
	Vec3f vertical = (ndcY * frame.tanHalfY) * frame.up;
	// The forward component is exactly 1, so eye + focusDistance * cameraDirection lies on the focal plane
	Vec3f cameraDirection = frame.forward + horizontal + vertical;
	if (frame.apertureRadius <= 0.0f) {
		Vec3f pinholeDirection = V3fNormalize(cameraDirection);
		Ray3f pinholeRay = Ray3fInit(frame.eye, pinholeDirection);
		return(pinholeRay);
	}
	Vec3f focusPoint = frame.eye + frame.focusDistance * cameraDirection;
	Vec2f diskPoint = SampleConcentricDisk(lensU, lensV);
	Vec3f lensRight = (frame.apertureRadius * diskPoint.x) * frame.right;
	Vec3f lensUp = (frame.apertureRadius * diskPoint.y) * frame.up;
	Vec3f lensPoint = frame.eye + lensRight + lensUp;
	Vec3f lensToFocus = focusPoint - lensPoint;
	Vec3f direction = V3fNormalize(lensToFocus);
	Ray3f result = Ray3fInit(lensPoint, direction);
	return(result);
}

//
// Light tracing for caustics
//
// A path tracer finds light that reaches a diffuse surface through mirrors or glass only by chance (the light is a tiny target behind a specular chain), which leaves speckles that need many thousand samples.
// Light tracing starts at the lights, aims at the specular objects and connects every non-specular vertex behind them to the camera, so caustics (also their diffuse bounce light) converge quickly.
// The path tracer skips exactly these paths (camera -> non-specular -> ... -> non-specular -> specular+ -> light), so the two strategies partition the paths and the result stays unbiased.
//
static constexpr f32 LightPathsPerPixel = 0.25f;
static constexpr u32 PhotonsPerLightJob = 2048;
static constexpr u64 LightTracingSeedSalt = 0x9A3C1F7D25E8B461ull;
static constexpr f64 CausticFixedPointScale = 4294967296.0;  // 2^32: integer sums are order independent, so the image stays deterministic
static constexpr f64 InverseCausticFixedPointScale = 1.0 / 4294967296.0;
static constexpr f64 CausticFixedPointMax = 1.0e19;          // below 2^64
static constexpr f32 SunPhotonStartDistanceFactor = 2.0f;    // sun photons start this many region radii away from the photon line's caster point
static constexpr u32 CausticChannelCount = 3;
static constexpr u32 TentFootprint = 2;                      // a tent of radius 1 touches 2 pixel centers per axis

struct CausticTarget {
	volatile u64 *sums;  // fixed point RGB per render pixel
	u32 width;
	u32 height;
	f32 scale;           // 1 / photons per pass
	f32 pixelFilmArea;   // area of one render pixel on the film plane at distance 1
};

// Per-worker direct mapped cache of fixed point splats: photons concentrate on a few caustic pixels, so summing locally avoids contended atomics
static constexpr u32 SplatCacheSize = 4096; // power of two
static constexpr u32 SplatCacheMask = SplatCacheSize - 1;
static constexpr u32 SplatCacheEmptyKey = 0;

struct SplatCacheEntry {
	u32 key;  // pixel index + 1, 0 = empty
	u32 reserved;
	u64 sums[CausticChannelCount];
};

struct SplatCache {
	SplatCacheEntry entries[SplatCacheSize];
};

static void FlushSplatCacheEntry(const CausticTarget &target, SplatCacheEntry &entry) {
	u32 pixelIndex = entry.key - 1;
	volatile u64 *pixelSums = target.sums + (size_t)pixelIndex * CausticChannelCount;
	for (u32 channel = 0; channel < CausticChannelCount; ++channel) {
		if (entry.sums[channel] > 0) {
			fplAtomicFetchAndAddU64(&pixelSums[channel], entry.sums[channel]);
		}
		entry.sums[channel] = 0;
	}
	entry.key = SplatCacheEmptyKey;
}

static void FlushSplatCache(const CausticTarget &target, SplatCache &cache) {
	for (u32 entryIndex = 0; entryIndex < SplatCacheSize; ++entryIndex) {
		SplatCacheEntry &entry = cache.entries[entryIndex];
		if (entry.key != SplatCacheEmptyKey) {
			FlushSplatCacheEntry(target, entry);
		}
	}
}

struct PhotonEmission {
	Vec3f lightPoint; // emission point on the light (without the ray origin offset), for the bidirectional MIS densities
	Ray3f ray;
	Vec3f power; // emitted radiance * cos / (area pdf * direction pdf), without the light selection probability
};

// Picks a caster proportional to the solid angle it subtends and samples a direction inside its cone, the pdf is the mixture over all cones that contain the direction
static bool SampleDirectionTowardsCasters(const SceneView &scene, const Vec3f &origin, const f32 uSelect, const f32 u0, const f32 u1, Vec3f &outDirection, f32 &outPdf) {
	f32 totalWeight = 0.0f;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		SphereCone cone = ComputeSphereCone(origin, caster.center, caster.radius);
		if (!cone.isInside) {
			totalWeight += cone.oneMinusCosThetaMax;
		}
	}
	if (!(totalWeight > 0.0f)) {
		return(false);
	}
	f32 selectTarget = uSelect * totalWeight;
	f32 cumulative = 0.0f;
	u32 selectedIndex = NoIndex;
	SphereCone selectedCone = {};
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		SphereCone cone = ComputeSphereCone(origin, caster.center, caster.radius);
		if (cone.isInside) {
			continue;
		}
		cumulative += cone.oneMinusCosThetaMax;
		selectedIndex = casterIndex;
		selectedCone = cone;
		if (selectTarget < cumulative) {
			break;
		}
	}
	if (selectedIndex == NoIndex) {
		return(false);
	}
	f32 cosTheta;
	f32 sin2Theta;
	Vec3f direction = SampleCone(selectedCone.axis, selectedCone.oneMinusCosThetaMax, u0, u1, cosTheta, sin2Theta);
	f32 pdf = 0.0f;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		SphereCone cone = ComputeSphereCone(origin, caster.center, caster.radius);
		if (cone.isInside) {
			continue;
		}
		f32 cosToAxis = V3fDot(direction, cone.axis);
		f32 cosThetaMax = 1.0f - cone.oneMinusCosThetaMax;
		// The selected cone always contains its own sample, even when rounding puts it a hair outside
		bool isInsideCone = (casterIndex == selectedIndex) || (cosToAxis >= cosThetaMax);
		if (isInsideCone) {
			f32 selectionProbability = cone.oneMinusCosThetaMax / totalWeight;
			f32 conePdf = ConePdf(cone.oneMinusCosThetaMax);
			pdf += selectionProbability * conePdf;
		}
	}
	if (!(pdf > 0.0f)) {
		return(false);
	}
	outDirection = direction;
	outPdf = pdf;
	return(true);
}

// Sphere and quad lights: uniform point on the light, direction aimed at the casters
static bool SampleAreaLightEmission(const SceneView &scene, const Light &light, PathSampler &sampler, PhotonEmission &outEmission) {
	f32 u0 = PathSamplerNext01(sampler);
	f32 u1 = PathSamplerNext01(sampler);
	f32 uSelect = PathSamplerNext01(sampler);
	f32 v0 = PathSamplerNext01(sampler);
	f32 v1 = PathSamplerNext01(sampler);
	const Primitive &primitive = scene.primitives[light.primitiveIndex];
	Vec3f position;
	Vec3f normal;
	f32 area;
	if (light.kind == LightKind::Sphere) {
		const SphereShape &sphere = primitive.sphere;
		f32 z = 1.0f - 2.0f * u0;
		f32 ringRadius = F32SquareRoot(F32Max(0.0f, 1.0f - z * z));
		f32 phi = F32Tau * u1;
		f32 cosPhi = F32Cos(phi);
		f32 sinPhi = F32Sin(phi);
		normal = V3fInit(ringRadius * cosPhi, ringRadius * sinPhi, z);
		position = sphere.center + sphere.radius * normal;
		area = 4.0f * F32Pi * sphere.radius * sphere.radius;
	} else {
		const QuadShape &quad = primitive.quad;
		position = quad.corner + u0 * quad.edgeU + u1 * quad.edgeV;
		normal = quad.normal;
		area = quad.area;
	}
	Vec3f direction;
	f32 directionPdf;
	bool hasDirection = SampleDirectionTowardsCasters(scene, position, uSelect, v0, v1, direction, directionPdf);
	if (!hasDirection) {
		return(false);
	}
	f32 signedCosine = V3fDot(normal, direction);
	f32 cosineAtLight = light.isTwoSided ? F32Abs(signedCosine) : signedCosine;
	if (cosineAtLight < MinLightCosine) {
		return(false); // the back of a one-sided emitter sends nothing
	}
	outEmission.lightPoint = position;
	outEmission.ray = SpawnRay(position, normal, direction);
	outEmission.power = (cosineAtLight * area / directionPdf) * light.radiance;
	return(true);
}

// Sun: direction inside the sun cone, origin on the disc of a caster's bounding sphere perpendicular to the direction
static bool SampleSunEmission(const SceneView &scene, const Light &sun, PathSampler &sampler, PhotonEmission &outEmission) {
	f32 u0 = PathSamplerNext01(sampler);
	f32 u1 = PathSamplerNext01(sampler);
	f32 uSelect = PathSamplerNext01(sampler);
	f32 v0 = PathSamplerNext01(sampler);
	f32 v1 = PathSamplerNext01(sampler);
	if (!(scene.casterAreaSum > 0.0f)) {
		return(false);
	}
	f32 cosTheta;
	f32 sin2Theta;
	Vec3f towardsSun = SampleCone(sun.direction, sun.oneMinusCosThetaMax, u0, u1, cosTheta, sin2Theta);
	Vec3f travel = -towardsSun;
	// Caster chosen proportional to its projected disc area r^2
	f32 selectTarget = uSelect * scene.casterAreaSum;
	f32 cumulative = 0.0f;
	u32 selectedIndex = scene.casterCount - 1;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		cumulative += caster.radius * caster.radius;
		if (selectTarget < cumulative) {
			selectedIndex = casterIndex;
			break;
		}
	}
	const CausticCaster &selected = scene.casters[selectedIndex];
	Vec3f tangent;
	Vec3f bitangent;
	BuildOrthonormalBasis(travel, tangent, bitangent);
	Vec2f diskPoint = SampleConcentricDisk(v0, v1);
	Vec3f point = selected.center + (selected.radius * diskPoint.x) * tangent + (selected.radius * diskPoint.y) * bitangent;
	// Area pdf on the plane perpendicular to the travel direction: discs covering the photon line, each with density 1 / (pi sum r^2)
	u32 coverCount = 0;
	for (u32 casterIndex = 0; casterIndex < scene.casterCount; ++casterIndex) {
		const CausticCaster &caster = scene.casters[casterIndex];
		Vec3f toCenter = caster.center - point;
		f32 along = V3fDot(toCenter, travel);
		f32 lengthSquared = V3fDot(toCenter, toCenter);
		f32 perpendicularSquared = lengthSquared - along * along;
		bool isCovering = (casterIndex == selectedIndex) || (perpendicularSquared <= caster.radius * caster.radius);
		if (isCovering) {
			++coverCount;
		}
	}
	f32 areaPdf = (f32)coverCount / (F32Pi * scene.casterAreaSum);
	Vec3f pointToCenter = point - scene.regionCenter;
	f32 pointDistance = V3fLength(pointToCenter);
	f32 startDistance = pointDistance + SunPhotonStartDistanceFactor * scene.regionRadius;
	Vec3f origin = point - startDistance * travel;
	outEmission.lightPoint = origin;
	outEmission.ray = Ray3fInit(origin, travel);
	outEmission.power = (1.0f / (areaPdf * sun.conePdf)) * sun.radiance;
	return(true);
}

// Stochastic rounding to the fixed point grid keeps even tiny contributions unbiased
static inline u64 ToCausticFixedPoint(const f32 value, const f64 roundingOffset) {
	f64 scaled = (f64)value * CausticFixedPointScale + roundingOffset;
	if (!(scaled >= 1.0)) {
		return(0); // also rejects NaN
	}
	f64 clamped = fplMin(scaled, CausticFixedPointMax);
	u64 result = (u64)clamped;
	return(result);
}

// Splats with the same tent filter (radius 1 render pixel) that the camera rays sample, so both strategies estimate the same pixel values.
// The camera side density of the connected vertex contains the pixel's tent density, so every pixel gets its own bidirectional MIS weight.
// mergeRatio: merging density relative to the path tracer (independent of the pixel), 0 when merging is not offered at this vertex
static void SplatCaustic(const CausticTarget &target, SplatCache &cache, const f32 renderX, const f32 renderY, const Vec3f &value, const f64 ratioWithoutCamera, const f64 cameraDensityWithoutFilter, const f64 mergeRatio, const f64 roundingOffset) {
	f64 lightPathCount = 1.0 / (f64)target.scale;
	f32 shiftedX = renderX - 0.5f;
	f32 shiftedY = renderY - 0.5f;
	f32 floorX = F32Floor(shiftedX);
	f32 floorY = F32Floor(shiftedY);
	f32 fractionX = shiftedX - floorX;
	f32 fractionY = shiftedY - floorY;
	s32 baseX = (s32)floorX;
	s32 baseY = (s32)floorY;
	const f32 weightsX[TentFootprint] = { 1.0f - fractionX, fractionX };
	const f32 weightsY[TentFootprint] = { 1.0f - fractionY, fractionY };
	for (u32 offsetY = 0; offsetY < TentFootprint; ++offsetY) {
		s32 pixelY = baseY + (s32)offsetY;
		if (pixelY < 0 || pixelY >= (s32)target.height) {
			continue;
		}
		for (u32 offsetX = 0; offsetX < TentFootprint; ++offsetX) {
			s32 pixelX = baseX + (s32)offsetX;
			f32 filterWeight = weightsX[offsetX] * weightsY[offsetY];
			if (pixelX < 0 || pixelX >= (s32)target.width || filterWeight <= 0.0f) {
				continue;
			}
			f64 cameraDensity = (f64)filterWeight * cameraDensityWithoutFilter;
			f64 pixelRatio = ratioWithoutCamera / cameraDensity;
			BidirectionalWeights weights = ComputeBidirectionalWeights(pixelRatio, mergeRatio, lightPathCount);
			f32 pixelWeight = filterWeight * (f32)weights.lightTracing;
			if (!(pixelWeight > 0.0f)) {
				continue;
			}
			u32 pixelIndex = (u32)pixelY * target.width + (u32)pixelX;
			u32 key = pixelIndex + 1;
			SplatCacheEntry &entry = cache.entries[pixelIndex & SplatCacheMask];
			if (entry.key != key) {
				if (entry.key != SplatCacheEmptyKey) {
					FlushSplatCacheEntry(target, entry);
				}
				entry.key = key;
			}
			for (u32 channel = 0; channel < CausticChannelCount; ++channel) {
				f32 channelValue = pixelWeight * value.m[channel];
				u64 fixedPoint = ToCausticFixedPoint(channelValue, roundingOffset);
				entry.sums[channel] += fixedPoint;
			}
		}
	}
}

// Bidirectional MIS state of a photon walk at the current vertex y_i (y_0 = the light)
struct LightPathMisState {
	f64 densityRatio;            // light tracing / path tracing density of the finished vertices y_0 .. y_(i-2)
	f64 previousLightDensity;    // light side area density of y_(i-1)
	f64 lightDensity;            // light side area density of y_i
	f32 previousCosine;          // cosine at y_(i-1) of the segment y_(i-1) -> y_i
	f32 segmentDistanceSquared;  // squared length of that segment
};

// Connects a photon at a non-specular surface to a point on the lens: contribution = throughput * f * cos(surface) / (pixel film area * cos^3(camera) * distance^2), weighted by bidirectional MIS
static void ConnectPhotonToCamera(const SceneView &scene, const CameraFrame &camera, const CausticTarget &target, SplatCache &cache, const SurfaceHit &hit, const Material &material, const Vec3f &towardsPhotonOrigin, const Vec3f &throughput, const LightPathMisState &mis, const f64 mergeArea, PathSampler &sampler, PathStats &stats) {
	f32 lensU = PathSamplerNext01(sampler);
	f32 lensV = PathSamplerNext01(sampler);
	f32 roundingRandom = PathSamplerNext01(sampler);
	Vec3f lensPoint = camera.eye;
	if (camera.apertureRadius > 0.0f) {
		Vec2f diskPoint = SampleConcentricDisk(lensU, lensV);
		Vec3f lensRight = (camera.apertureRadius * diskPoint.x) * camera.right;
		Vec3f lensUp = (camera.apertureRadius * diskPoint.y) * camera.up;
		lensPoint = camera.eye + lensRight + lensUp;
	}
	Vec3f toLens = lensPoint - hit.position;
	f32 distanceSquared = V3fDot(toLens, toLens);
	if (!(distanceSquared > 0.0f)) {
		return;
	}
	f32 distance = F32SquareRoot(distanceSquared);
	Vec3f towardsLens = (1.0f / distance) * toLens;
	Vec3f viewDirection = -towardsLens;
	f32 cosCamera = V3fDot(viewDirection, camera.forward);
	if (cosCamera < MinCosine) {
		return;
	}
	// Film position: through the focal plane for a thin lens, so it is the exact inverse of GenerateCameraRay
	Vec3f cameraDirection = (1.0f / cosCamera) * viewDirection;
	if (camera.apertureRadius > 0.0f) {
		f32 focusTravel = camera.focusDistance / cosCamera;
		Vec3f focusPoint = lensPoint + focusTravel * viewDirection;
		Vec3f eyeToFocus = focusPoint - camera.eye;
		cameraDirection = (1.0f / camera.focusDistance) * eyeToFocus;
	}
	f32 rightAmount = V3fDot(cameraDirection, camera.right);
	f32 upAmount = V3fDot(cameraDirection, camera.up);
	f32 ndcX = rightAmount / camera.tanHalfX;
	f32 ndcY = upAmount / camera.tanHalfY;
	f32 filmX = 0.5f * (ndcX + 1.0f) / camera.inverseOutputWidth;
	f32 filmY = 0.5f * (1.0f - ndcY) / camera.inverseOutputHeight;
	f32 renderX = filmX / camera.resolutionDivisor;
	f32 renderY = filmY / camera.resolutionDivisor;
	f32 footprintMinimum = -0.5f;
	f32 footprintMaximumX = (f32)target.width + 0.5f;
	f32 footprintMaximumY = (f32)target.height + 0.5f;
	if (!(renderX > footprintMinimum && renderX < footprintMaximumX && renderY > footprintMinimum && renderY < footprintMaximumY)) {
		return;
	}
	Vec3f frameNormal = FaceTowards(hit.geometricNormal, towardsLens);
	ShadingFrame frame = MakeShadingFrame(frameNormal);
	Vec3f wo = ToLocal(frame, towardsLens);
	Vec3f wi = ToLocal(frame, towardsPhotonOrigin);
	Vec3f baseColor = EvaluateBaseColor(material, hit.position);
	Vec3f bsdfValue = EvaluateBsdf(material, baseColor, wo, wi);
	if (IsBlack(bsdfValue)) {
		return;
	}
	Ray3f shadowRay = SpawnRay(hit.position, hit.geometricNormal, towardsLens);
	f32 shadowMaxDistance = distance * ShadowDistanceScale;
	++stats.rayCount;
	bool isOccluded = TraceOccluded(scene, shadowRay, shadowMaxDistance, NoIndex);
	if (isOccluded) {
		return;
	}
	f32 cosCameraCubed = cosCamera * cosCamera * cosCamera;
	f32 importance = wo.z / (target.pixelFilmArea * cosCameraCubed * distanceSquared);
	Vec3f weightedThroughput = V3fHadamard(throughput, bsdfValue);
	Vec3f contribution = (importance * target.scale) * weightedThroughput;
	if (!IsFiniteV3(contribution)) {
		return;
	}
	// The path tracer would sample the photon's previous vertex from here, coming from the camera
	f32 cameraSidePdf = BsdfPdf(material, baseColor, wo, wi);
	f64 previousCameraDensity = (f64)cameraSidePdf * (f64)mis.previousCosine / (f64)mis.segmentDistanceSquared;
	f64 ratioWithoutCamera = mis.densityRatio * (mis.previousLightDensity / previousCameraDensity) * mis.lightDensity;
	// Area density of this vertex from the camera without the tent density: the solid angle density 1 / (pixel film area * cos^3) times cos(surface) / distance^2
	f64 cameraDensityWithoutFilter = (f64)importance;
	// Merging at this vertex replaces its camera side density by the disc area, so its ratio does not depend on the pixel
	bool isMergeOffered = HasSharpLobe(material);
	f64 mergeRatio = isMergeOffered ? (ratioWithoutCamera * mergeArea) : 0.0;
	SplatCaustic(target, cache, renderX, renderY, contribution, ratioWithoutCamera, cameraDensityWithoutFilter, mergeRatio, (f64)roundingRandom);
}

// One photon: emission aimed at the casters, one or more specular bounces, then a camera connection and a photon record at every non-specular vertex
static void TraceLightPath(const SceneView &scene, const RenderSettings &settings, const CameraFrame &camera, const CausticTarget &target, const f64 mergeArea, SplatCache &cache, std::vector<PhotonRecord> &photonOutput, PathSampler &sampler, PathStats &stats) {
	f32 uLight = PathSamplerNext01(sampler);
	u32 lightIndex = SelectLight(scene, uLight);
	const Light &light = scene.lights[lightIndex];
	PhotonEmission emission;
	bool hasEmission = false;
	bool isSun = light.kind == LightKind::Sun;
	if (isSun) {
		hasEmission = SampleSunEmission(scene, light, sampler, emission);
	} else if (light.kind == LightKind::Sphere || light.kind == LightKind::Quad) {
		hasEmission = SampleAreaLightEmission(scene, light, sampler, emission);
	}
	if (!hasEmission) {
		return;
	}
	Vec3f throughput = (1.0f / light.selectionProbability) * emission.power;
	Vec3f relativeThroughput = V3fInit(1.0f, 1.0f, 1.0f); // product of the scattering weights, russian roulette must not look at the absolute photon power
	Ray3f ray = emission.ray;
	u32 vertexCount = 0;           // scattering vertices so far, a camera connection at vertex m forms a path with m of them (path tracer limit: maxBounces)
	u32 specularCount = 0;
	bool hasPassedSpecularChain = false;
	// Bidirectional MIS: the light vertex has camera side density 1, because the first photon hit is always specular
	LightPathMisState mis = {};
	mis.densityRatio = LightVertexDensity(scene, light);
	Vec3f previousNormal = V3fZero();
	f32 previousSamplePdf = 0.0f;
	bool previousWasDelta = false;
	for (;;) {
		SurfaceHit hit;
		++stats.rayCount;
		bool isHit = TraceSurface(scene, ray, UnboundedDistance, hit);
		if (!isHit) {
			return;
		}
		const Material &material = scene.materials[hit.materialIndex];
		if (material.kind == MaterialKind::Emissive) {
			return;
		}
		++vertexCount;
		f32 hitCosine = F32Abs(V3fDot(hit.geometricNormal, ray.direction));
		f32 hitDistanceSquared = hit.distance * hit.distance;
		f32 previousCosine = F32Abs(V3fDot(previousNormal, ray.direction));
		f64 lightDensity = 1.0; // light side area density of this vertex (1 behind a specular vertex)
		if (vertexCount == 1) {
			if (isSun) {
				f32 sunAreaDensity = SunCasterAreaDensity(scene, hit.position, ray.direction);
				lightDensity = (f64)sunAreaDensity * (f64)hitCosine;
			} else {
				f32 emissionDensity = CasterDirectionDensity(scene, emission.lightPoint, ray.direction);
				lightDensity = (f64)emissionDensity * (f64)hitCosine / (f64)hitDistanceSquared;
			}
		} else if (!previousWasDelta) {
			lightDensity = (f64)previousSamplePdf * (f64)hitCosine / (f64)hitDistanceSquared;
		}
		if (material.kind == MaterialKind::Dielectric && !hit.isFrontFace && material.hasAbsorption) {
			Vec3f opticalDepth = hit.distance * material.absorptionCoefficient;
			Vec3f transmittance = V3fExpNegative(opticalDepth);
			throughput = V3fHadamard(throughput, transmittance);
			relativeThroughput = V3fHadamard(relativeThroughput, transmittance);
		}
		Vec3f towardsOrigin = -ray.direction;
		bool hasNonDeltaLobe = MaterialHasNonDeltaLobe(material);
		if (hasNonDeltaLobe) {
			if (!hasPassedSpecularChain) {
				// Light that reaches a non-specular surface without a specular bounce stays with the path tracer
				if (specularCount == 0) {
					return;
				}
				hasPassedSpecularChain = true;
			}
			mis.lightDensity = lightDensity;
			mis.previousCosine = previousCosine;
			mis.segmentDistanceSquared = hitDistanceSquared;
			f64 partialRatio = mis.densityRatio * mis.previousLightDensity * lightDensity;
			f64 previousGeometry = (f64)previousCosine / (f64)hitDistanceSquared;
			PhotonRecord record;
			record.position = hit.position;
			record.towardsOrigin = towardsOrigin;
			record.power = target.scale * throughput;
			record.vertexCount = vertexCount;
			record.misPartialRatio = (f32)partialRatio;
			record.misPreviousGeometry = (f32)previousGeometry;
			photonOutput.push_back(record);
			ConnectPhotonToCamera(scene, camera, target, cache, hit, material, towardsOrigin, throughput, mis, mergeArea, sampler, stats);
		} else if (!hasPassedSpecularChain) {
			++specularCount;
		}
		if (vertexCount >= settings.maxBounces) {
			return;
		}
		Vec3f frameNormal = hit.isFrontFace ? hit.geometricNormal : -hit.geometricNormal;
		ShadingFrame frame = MakeShadingFrame(frameNormal);
		Vec3f wo = ToLocal(frame, towardsOrigin);
		if (wo.z < MinCosine) {
			return;
		}
		// All BSDFs here are symmetric (no shading normals, the refraction radiance scale is omitted), so the adjoint equals the BSDF
		Vec3f baseColor = EvaluateBaseColor(material, hit.position);
		BsdfSample bsdfSample;
		bool hasBsdfSample = SampleBsdf(material, baseColor, wo, hit.isFrontFace, sampler, bsdfSample);
		if (!hasBsdfSample) {
			return;
		}
		bool isSampleDelta = bsdfSample.isDelta != 0;
		// The continuation is known now, so the previous vertex gets its camera side density: the path tracer would sample it from here coming from the continuation
		if (vertexCount >= 2) {
			f64 previousCameraDensity = 1.0;
			if (!isSampleDelta) {
				f32 cameraSidePdf = BsdfPdf(material, baseColor, bsdfSample.wi, wo);
				previousCameraDensity = (f64)cameraSidePdf * (f64)previousCosine / (f64)hitDistanceSquared;
			}
			mis.densityRatio *= mis.previousLightDensity / previousCameraDensity;
		}
		mis.previousLightDensity = lightDensity;
		previousNormal = hit.geometricNormal;
		previousSamplePdf = bsdfSample.pdf;
		previousWasDelta = isSampleDelta;
		throughput = V3fHadamard(throughput, bsdfSample.weight);
		relativeThroughput = V3fHadamard(relativeThroughput, bsdfSample.weight);
		Vec3f worldDirection = ToWorld(frame, bsdfSample.wi);
		Vec3f newDirection = V3fNormalize(worldDirection);
		ray = SpawnRay(hit.position, hit.geometricNormal, newDirection);
		if (vertexCount >= settings.russianRouletteStartBounce) {
			f32 maxThroughput = MaxComponent(relativeThroughput);
			f32 survival = F32Clamp(maxThroughput, settings.russianRouletteMinSurvival, settings.russianRouletteMaxSurvival);
			f32 uRoulette = PathSamplerNext01(sampler);
			if (uRoulette >= survival) {
				return;
			}
			f32 boost = 1.0f / survival;
			throughput *= boost;
			relativeThroughput *= boost;
		}
	}
}

//
// Resolve: exposure -> tone mapping -> sRGB -> dither -> 0xAARRGGBB
//
// ACES fitted by Stephen Hill (RRT + ODT with sRGB to AP1 handling), row-major: out[i] = sum_j M[i][j] * in[j]
static constexpr f32 AcesInputMatrix[3][3] = {
	{ 0.59719f, 0.35458f, 0.04823f },
	{ 0.07600f, 0.90834f, 0.01566f },
	{ 0.02840f, 0.13383f, 0.83777f },
};
static constexpr f32 AcesOutputMatrix[3][3] = {
	{ 1.60475f, -0.53108f, -0.07367f },
	{ -0.10208f, 1.10813f, -0.00605f },
	{ -0.00327f, -0.07276f, 1.07602f },
};
static constexpr f32 AcesFitA = 0.0245786f;
static constexpr f32 AcesFitB = 0.000090537f;
static constexpr f32 AcesFitC = 0.983729f;
static constexpr f32 AcesFitD = 0.4329510f;
static constexpr f32 AcesFitE = 0.238081f;
static constexpr f32 AcesMaxInput = 1.0e4f; // the fit saturates long before, larger inputs would overflow to inf/inf = NaN

// Khronos PBR Neutral: identity below about 0.76, keeps base colors accurate
static constexpr f32 NeutralStartCompression = 0.8f - 0.04f;
static constexpr f32 NeutralDesaturation = 0.15f;
static constexpr f32 NeutralToeThreshold = 0.08f;
static constexpr f32 NeutralToeScale = 6.25f;
static constexpr f32 NeutralToeOffset = 0.04f;

static constexpr f32 ColorByteMax = 255.0f;
static constexpr u32 PixelRedShift = 16;
static constexpr u32 PixelGreenShift = 8;
static constexpr u32 OpaqueBlackPixel = 0xFF000000u;

// Jarzynski and Olano 2020 PCG hash, for the static per pixel dither
static constexpr u32 HashMultiplier = 747796405u;
static constexpr u32 HashIncrement = 2891336453u;
static constexpr u32 HashShiftBase = 28u;
static constexpr u32 HashShiftOffset = 4u;
static constexpr u32 HashOutputMultiplier = 277803737u;
static constexpr u32 HashOutputShift = 22u;
static constexpr u32 DitherHalfBits = 16u;
static constexpr u32 DitherHalfMask = 0xFFFFu;
static constexpr f32 DitherHalfScale = 1.0f / 65536.0f;

static inline Vec3f MultiplyMatrix3(const f32 matrix[3][3], const Vec3f &value) {
	f32 x = matrix[0][0] * value.x + matrix[0][1] * value.y + matrix[0][2] * value.z;
	f32 y = matrix[1][0] * value.x + matrix[1][1] * value.y + matrix[1][2] * value.z;
	f32 z = matrix[2][0] * value.x + matrix[2][1] * value.y + matrix[2][2] * value.z;
	Vec3f result = V3fInit(x, y, z);
	return(result);
}

static inline f32 AcesRrtOdtFit(const f32 input) {
	f32 value = F32Min(input, AcesMaxInput);
	f32 numerator = value * (value + AcesFitA) - AcesFitB;
	f32 denominator = value * (AcesFitC * value + AcesFitD) + AcesFitE;
	f32 result = numerator / denominator;
	return(result);
}

static Vec3f TonemapAcesFitted(const Vec3f &color) {
	Vec3f input = MultiplyMatrix3(AcesInputMatrix, color);
	f32 fittedX = AcesRrtOdtFit(input.x);
	f32 fittedY = AcesRrtOdtFit(input.y);
	f32 fittedZ = AcesRrtOdtFit(input.z);
	Vec3f fitted = V3fInit(fittedX, fittedY, fittedZ);
	Vec3f result = MultiplyMatrix3(AcesOutputMatrix, fitted);
	return(result);
}

static Vec3f TonemapPbrNeutral(const Vec3f &color) {
	f32 minimumGreenBlue = F32Min(color.g, color.b);
	f32 minimum = F32Min(color.r, minimumGreenBlue);
	f32 offset = (minimum < NeutralToeThreshold) ? (minimum - NeutralToeScale * minimum * minimum) : NeutralToeOffset;
	Vec3f offsetVector = V3fInitScalar(offset);
	Vec3f shifted = color - offsetVector;
	f32 peak = MaxComponent(shifted);
	if (peak < NeutralStartCompression) {
		return(shifted);
	}
	f32 compressionRange = 1.0f - NeutralStartCompression;
	f32 newPeak = 1.0f - compressionRange * compressionRange / (peak + compressionRange - NeutralStartCompression);
	Vec3f compressed = (newPeak / peak) * shifted;
	f32 desaturationBlend = 1.0f - 1.0f / (NeutralDesaturation * (peak - newPeak) + 1.0f);
	Vec3f white = V3fInitScalar(newPeak);
	Vec3f result = V3fLerp(compressed, desaturationBlend, white);
	return(result);
}

static inline u32 HashPcg(const u32 value) {
	u32 state = value * HashMultiplier + HashIncrement;
	u32 word = ((state >> ((state >> HashShiftBase) + HashShiftOffset)) ^ state) * HashOutputMultiplier;
	u32 result = (word >> HashOutputShift) ^ word;
	return(result);
}

// Triangular dither in (-1, 1) LSB, static per pixel so a converged image does not shimmer
static inline f32 DitherOffset(const u32 x, const u32 y) {
	u32 rowHash = HashPcg(y);
	u32 hash = HashPcg(x + rowHash);
	f32 first = (f32)(hash & DitherHalfMask) * DitherHalfScale;
	f32 second = (f32)(hash >> DitherHalfBits) * DitherHalfScale;
	f32 result = first - second;
	return(result);
}

static inline u32 EncodeColorChannel(const f32 linear, const f32 dither) {
	f32 encoded = LinearToSRGB(linear);
	f32 scaled = encoded * ColorByteMax + 0.5f + dither;
	// Explicit clamp: values above 255.5 would wrap to black
	f32 clamped = F32Clamp(scaled, 0.0f, ColorByteMax);
	u32 result = (u32)clamped;
	return(result);
}

static u32 ResolvePixel(const Vec3f &mean, const f32 exposureScale, const TonemapOperator tonemapOperator, const u32 x, const u32 y) {
	f32 clampedRed = F32Max(mean.r, 0.0f);
	f32 clampedGreen = F32Max(mean.g, 0.0f);
	f32 clampedBlue = F32Max(mean.b, 0.0f);
	Vec3f clampedMean = V3fInit(clampedRed, clampedGreen, clampedBlue);
	Vec3f exposed = exposureScale * clampedMean;
	Vec3f mapped;
	switch (tonemapOperator) {
		case TonemapOperator::AcesFitted:
			mapped = TonemapAcesFitted(exposed);
			break;
		case TonemapOperator::PbrNeutral:
			mapped = TonemapPbrNeutral(exposed);
			break;
		default:
			mapped = exposed;
			break;
	}
	f32 dither = DitherOffset(x, y);
	u32 red = EncodeColorChannel(mapped.r, dither);
	u32 green = EncodeColorChannel(mapped.g, dither);
	u32 blue = EncodeColorChannel(mapped.b, dither);
	u32 result = OpaqueBlackPixel | (red << PixelRedShift) | (green << PixelGreenShift) | blue;
	return(result);
}

//
// Render target and tiles
//
static constexpr u32 FullResolutionTileSize = 32;
static constexpr u32 PreviewTileSize = 16;
static constexpr u32 MaxTileSize = 32;                // size of the per-worker tile scratch buffer
static constexpr u32 MaxWorkerCount = 256;            // must not exceed FPL_MAX_THREAD_WAIT_COUNT
static constexpr size_t CacheLinePairSize = 128;      // two 64-byte lines: adjacent-line prefetchers pair lines
static constexpr size_t RenderBufferAlignment = 64;
static constexpr u32 TileLockSpinCount = 64;          // yields before sleeping
static constexpr u32 TileLockSleepMilliseconds = 1;
static constexpr u32 DisplayBufferCount = 2;

fplStaticAssert(MaxWorkerCount <= FPL_MAX_THREAD_WAIT_COUNT);
fplStaticAssert(PreviewTileSize <= MaxTileSize && FullResolutionTileSize <= MaxTileSize);

struct AccumulationPixel {
	f64 r;
	f64 g;
	f64 b;
};

struct TileRect {
	u32 x0; // inclusive, render pixels
	u32 y0;
	u32 x1; // exclusive
	u32 y1;
};

struct TileState {
	volatile u32 lock;   // 0 = free, 1 = held, contention only between a committing worker and the main thread
	u32 sampleCount;     // samples per pixel committed for this tile in the current generation
	u32 displayVersion;  // display settings version used for the last resolve of this tile
	u32 reserved;
};

// Owned by the main thread, workers only see the copies in RenderConfig
struct RenderTarget {
	u32 outputWidth;              // backbuffer size the camera frustum refers to
	u32 outputHeight;
	u32 resolutionDivisor;        // 1 = full resolution, > 1 = preview
	u32 width;                    // render resolution = ceil(output / resolutionDivisor)
	u32 height;
	u32 tileSize;
	u32 tileCountX;
	u32 tileCountY;
	u32 tileCount;
	size_t pixelCapacity;         // grow-only
	AccumulationPixel *accumulation;
	u32 *displayBuffers[DisplayBufferCount]; // 0xAARRGGBB, stride = width
	u32 frontDisplayIndex;
	size_t tileCapacity;          // grow-only
	TileRect *tiles;              // job index -> tile, center-out order
	TileState *tileStates;        // indexed by job index
	volatile u64 *causticSums;    // light tracing splats, 3 fixed point channels per pixel, same capacity as the accumulation
	u32 lightJobCount;            // 0 when light tracing is off for the current generation
	size_t lightJobCapacity;      // grow-only
	u32 *lightJobStates;          // passes completed per light job
};

static void FillPixels(u32 *pixels, const size_t pixelCount, const u32 color) {
	for (size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
		pixels[pixelIndex] = color;
	}
}

// Nearest neighbor resample, so a resize or a resolution change shows the stretched old image until new samples arrive
static void ResampleDisplay(const u32 *source, const u32 sourceWidth, const u32 sourceHeight, u32 *destination, const u32 destinationWidth, const u32 destinationHeight) {
	for (u32 y = 0; y < destinationHeight; ++y) {
		u64 sourceY = (u64)y * sourceHeight / destinationHeight;
		const u32 *sourceRow = source + sourceY * sourceWidth;
		u32 *destinationRow = destination + (size_t)y * destinationWidth;
		for (u32 x = 0; x < destinationWidth; ++x) {
			u64 sourceX = (u64)x * sourceWidth / destinationWidth;
			destinationRow[x] = sourceRow[sourceX];
		}
	}
}

struct TileSortKey {
	s64 distanceSquared;
	u32 rowMajorIndex;
};

static bool CompareTileSortKeys(const TileSortKey &a, const TileSortKey &b) {
	if (a.distanceSquared != b.distanceSquared) {
		return(a.distanceSquared < b.distanceSquared);
	}
	return(a.rowMajorIndex < b.rowMajorIndex);
}

// Tiles sorted center-out, so the interesting part appears first; the order never affects the result
static bool BuildTiles(RenderTarget *target) {
	u32 tileSize = target->tileSize;
	u32 tileCountX = (target->width + tileSize - 1) / tileSize;
	u32 tileCountY = (target->height + tileSize - 1) / tileSize;
	u32 tileCount = tileCountX * tileCountY;
	if (tileCount > target->tileCapacity) {
		TileRect *newTiles = (TileRect *)fplMemoryAlignedAllocate(sizeof(TileRect) * tileCount, RenderBufferAlignment);
		TileState *newTileStates = (TileState *)fplMemoryAlignedAllocate(sizeof(TileState) * tileCount, RenderBufferAlignment);
		if (newTiles == fpl_null || newTileStates == fpl_null) {
			if (newTiles != fpl_null) {
				fplMemoryAlignedFree(newTiles);
			}
			if (newTileStates != fpl_null) {
				fplMemoryAlignedFree(newTileStates);
			}
			target->tileCount = 0;
			return(false);
		}
		if (target->tiles != fpl_null) {
			fplMemoryAlignedFree(target->tiles);
			fplMemoryAlignedFree(target->tileStates);
		}
		target->tiles = newTiles;
		target->tileStates = newTileStates;
		target->tileCapacity = tileCount;
	}
	target->tileCountX = tileCountX;
	target->tileCountY = tileCountY;
	target->tileCount = tileCount;
	std::vector<TileSortKey> keys(target->tileCount);
	for (u32 tileY = 0; tileY < target->tileCountY; ++tileY) {
		for (u32 tileX = 0; tileX < target->tileCountX; ++tileX) {
			u32 rowMajorIndex = tileY * target->tileCountX + tileX;
			u32 x0 = tileX * tileSize;
			u32 y0 = tileY * tileSize;
			u32 x1 = fplMin(x0 + tileSize, target->width);
			u32 y1 = fplMin(y0 + tileSize, target->height);
			// Doubled integer coordinates avoid halves
			s64 deltaX = (s64)(x0 + x1) - (s64)target->width;
			s64 deltaY = (s64)(y0 + y1) - (s64)target->height;
			keys[rowMajorIndex].distanceSquared = deltaX * deltaX + deltaY * deltaY;
			keys[rowMajorIndex].rowMajorIndex = rowMajorIndex;
		}
	}
	std::sort(keys.begin(), keys.end(), CompareTileSortKeys);
	for (u32 jobIndex = 0; jobIndex < target->tileCount; ++jobIndex) {
		u32 rowMajorIndex = keys[jobIndex].rowMajorIndex;
		u32 tileX = rowMajorIndex % target->tileCountX;
		u32 tileY = rowMajorIndex / target->tileCountX;
		TileRect &tile = target->tiles[jobIndex];
		tile.x0 = tileX * tileSize;
		tile.y0 = tileY * tileSize;
		tile.x1 = fplMin(tile.x0 + tileSize, target->width);
		tile.y1 = fplMin(tile.y0 + tileSize, target->height);
	}
	return(true);
}

// Only while no worker is active: sizes the light jobs for the current resolution and clears the caustic sums and the job states
static bool ConfigureLightTracing(RenderTarget *target, const bool isEnabled) {
	target->lightJobCount = 0;
	if (!isEnabled) {
		return(true);
	}
	f32 photonsPerPass = (f32)target->width * (f32)target->height * LightPathsPerPixel;
	u32 jobCount = (u32)(photonsPerPass / (f32)PhotonsPerLightJob) + 1;
	if (jobCount > target->lightJobCapacity) {
		u32 *newStates = (u32 *)fplMemoryAlignedAllocate(sizeof(u32) * jobCount, RenderBufferAlignment);
		if (newStates == fpl_null) {
			return(false);
		}
		if (target->lightJobStates != fpl_null) {
			fplMemoryAlignedFree(target->lightJobStates);
		}
		target->lightJobStates = newStates;
		target->lightJobCapacity = jobCount;
	}
	fplMemoryClear(target->lightJobStates, sizeof(u32) * jobCount);
	size_t sumCount = (size_t)target->width * target->height * CausticChannelCount;
	fplMemoryClear((void *)target->causticSums, sizeof(u64) * sumCount);
	target->lightJobCount = jobCount;
	return(true);
}

static void ResetTileStates(RenderTarget *target) {
	for (u32 jobIndex = 0; jobIndex < target->tileCount; ++jobIndex) {
		TileState &tileState = target->tileStates[jobIndex];
		tileState.lock = 0;
		tileState.sampleCount = 0;
		tileState.displayVersion = 0;
		tileState.reserved = 0;
	}
}

// Only while no worker is active: resizes the grow-only buffers, keeps the old image visible (resampled) and rebuilds the tiles when needed, false when out of memory
static bool ConfigureRenderTarget(RenderTarget *target, const u32 outputWidth, const u32 outputHeight, const u32 resolutionDivisor) {
	fplAssert(outputWidth > 0 && outputHeight > 0 && resolutionDivisor > 0);
	u32 newWidth = (outputWidth + resolutionDivisor - 1) / resolutionDivisor;
	u32 newHeight = (outputHeight + resolutionDivisor - 1) / resolutionDivisor;
	u32 newTileSize = (resolutionDivisor > 1) ? PreviewTileSize : FullResolutionTileSize;
	bool isDimensionChanged = (newWidth != target->width) || (newHeight != target->height);
	bool isTileSizeChanged = newTileSize != target->tileSize;
	if (isDimensionChanged) {
		size_t newPixelCount = (size_t)newWidth * (size_t)newHeight;
		u32 *oldFront = target->displayBuffers[target->frontDisplayIndex];
		bool hasOldImage = (oldFront != fpl_null) && (target->width > 0) && (target->height > 0);
		if (newPixelCount > target->pixelCapacity) {
			AccumulationPixel *newAccumulation = (AccumulationPixel *)fplMemoryAlignedAllocate(sizeof(AccumulationPixel) * newPixelCount, RenderBufferAlignment);
			volatile u64 *newCausticSums = (volatile u64 *)fplMemoryAlignedAllocate(sizeof(u64) * CausticChannelCount * newPixelCount, RenderBufferAlignment);
			u32 *newDisplays[DisplayBufferCount];
			bool isAllocated = (newAccumulation != fpl_null) && (newCausticSums != fpl_null);
			for (u32 bufferIndex = 0; bufferIndex < DisplayBufferCount; ++bufferIndex) {
				newDisplays[bufferIndex] = (u32 *)fplMemoryAlignedAllocate(sizeof(u32) * newPixelCount, RenderBufferAlignment);
				isAllocated = isAllocated && (newDisplays[bufferIndex] != fpl_null);
			}
			if (!isAllocated) {
				// Keep the old buffers, the caller reports the failure
				if (newAccumulation != fpl_null) {
					fplMemoryAlignedFree(newAccumulation);
				}
				if (newCausticSums != fpl_null) {
					fplMemoryAlignedFree((void *)newCausticSums);
				}
				for (u32 bufferIndex = 0; bufferIndex < DisplayBufferCount; ++bufferIndex) {
					if (newDisplays[bufferIndex] != fpl_null) {
						fplMemoryAlignedFree(newDisplays[bufferIndex]);
					}
				}
				return(false);
			}
			if (hasOldImage) {
				ResampleDisplay(oldFront, target->width, target->height, newDisplays[0], newWidth, newHeight);
			} else {
				FillPixels(newDisplays[0], newPixelCount, OpaqueBlackPixel);
			}
			if (target->accumulation != fpl_null) {
				fplMemoryAlignedFree(target->accumulation);
			}
			if (target->causticSums != fpl_null) {
				fplMemoryAlignedFree((void *)target->causticSums);
			}
			for (u32 bufferIndex = 0; bufferIndex < DisplayBufferCount; ++bufferIndex) {
				if (target->displayBuffers[bufferIndex] != fpl_null) {
					fplMemoryAlignedFree(target->displayBuffers[bufferIndex]);
				}
				target->displayBuffers[bufferIndex] = newDisplays[bufferIndex];
			}
			target->accumulation = newAccumulation;
			target->causticSums = newCausticSums;
			target->pixelCapacity = newPixelCount;
			target->frontDisplayIndex = 0;
		} else {
			u32 backIndex = (target->frontDisplayIndex + 1) % DisplayBufferCount;
			u32 *back = target->displayBuffers[backIndex];
			if (hasOldImage) {
				ResampleDisplay(oldFront, target->width, target->height, back, newWidth, newHeight);
			} else {
				FillPixels(back, newPixelCount, OpaqueBlackPixel);
			}
			target->frontDisplayIndex = backIndex;
		}
		target->width = newWidth;
		target->height = newHeight;
	}
	target->outputWidth = outputWidth;
	target->outputHeight = outputHeight;
	target->resolutionDivisor = resolutionDivisor;
	if (isDimensionChanged || isTileSizeChanged) {
		target->tileSize = newTileSize;
		bool areTilesBuilt = BuildTiles(target);
		if (!areTilesBuilt) {
			return(false);
		}
	}
	return(true);
}

static void ReleaseRenderTarget(RenderTarget *target) {
	if (target->accumulation != fpl_null) {
		fplMemoryAlignedFree(target->accumulation);
	}
	if (target->causticSums != fpl_null) {
		fplMemoryAlignedFree((void *)target->causticSums);
	}
	if (target->lightJobStates != fpl_null) {
		fplMemoryAlignedFree(target->lightJobStates);
	}
	for (u32 bufferIndex = 0; bufferIndex < DisplayBufferCount; ++bufferIndex) {
		if (target->displayBuffers[bufferIndex] != fpl_null) {
			fplMemoryAlignedFree(target->displayBuffers[bufferIndex]);
		}
	}
	if (target->tiles != fpl_null) {
		fplMemoryAlignedFree(target->tiles);
		fplMemoryAlignedFree(target->tileStates);
	}
	fplClearStruct(target);
}

static void TileLock(TileState *tileState) {
	u32 spinCount = 0;
	for (;;) {
		bool isAcquired = fplAtomicIsCompareAndSwapU32(&tileState->lock, 0, 1);
		if (isAcquired) {
			return;
		}
		++spinCount;
		if (spinCount < TileLockSpinCount) {
			fplThreadYield();
		} else {
			// The real yield: a low priority worker may wait for the normal priority main thread
			fplThreadSleep(TileLockSleepMilliseconds);
		}
	}
}

static void TileUnlock(TileState *tileState) {
	fplAtomicStoreU32(&tileState->lock, 0); // FPL atomics are full barriers, so this is a release
}

//
// Display settings: changeable at any time without a restart, packed into one 64-bit atomic so a worker reads a consistent snapshot
//
static constexpr u32 DisplayTonemapShift = 32;
static constexpr u32 DisplayVersionShift = 40;
static constexpr u64 DisplayTonemapMask = 0xFFull;
static constexpr u64 DisplayVersionMask = 0xFFFFFFull;
static constexpr u64 DisplayExposureMask = 0xFFFFFFFFull;

struct DisplaySettings {
	f32 exposureEV;
	TonemapOperator tonemapOperator;
	u32 version; // +1 on every change, 24 bits are used when packed
};

static u64 PackDisplaySettings(const DisplaySettings &display) {
	u32 exposureBits;
	memcpy(&exposureBits, &display.exposureEV, sizeof(exposureBits));
	u64 versionPart = ((u64)display.version & DisplayVersionMask) << DisplayVersionShift;
	u64 tonemapPart = ((u64)display.tonemapOperator & DisplayTonemapMask) << DisplayTonemapShift;
	u64 result = versionPart | tonemapPart | (u64)exposureBits;
	return(result);
}

static DisplaySettings UnpackDisplaySettings(const u64 packed) {
	u32 exposureBits = (u32)(packed & DisplayExposureMask);
	DisplaySettings result;
	memcpy(&result.exposureEV, &exposureBits, sizeof(exposureBits));
	result.tonemapOperator = (TonemapOperator)((packed >> DisplayTonemapShift) & DisplayTonemapMask);
	result.version = (u32)((packed >> DisplayVersionShift) & DisplayVersionMask);
	return(result);
}

//
// Job system
//
// Generation = immutable render configuration: camera, scene, settings, resolution, tiles and buffers never change while a worker is active.
// Epoch = pass = sample index: pass e renders exactly sample e of every pixel, and the next epoch is published only after all tiles of the current one are committed.
// So every pixel sum is built in a fixed order and the image is identical for any thread count and timing.
//
static constexpr u64 JobCursorIndexMask = 0xFFFFFFFFull;
static constexpr u32 JobCursorEpochShift = 32;

// Each hot shared atomic gets its own 128-byte block, so claims, completions and abort polling never share a cache line
struct alignas(128) PaddedU32 {
	volatile u32 value;
	u8 padding[CacheLinePairSize - sizeof(u32)];
};

struct alignas(128) PaddedU64 {
	volatile u64 value; // alignas also guarantees the 8-byte alignment 64-bit atomics need on 32-bit x86
	u8 padding[CacheLinePairSize - sizeof(u64)];
};

fplStaticAssert(sizeof(PaddedU32) == CacheLinePairSize);
fplStaticAssert(sizeof(PaddedU64) == CacheLinePairSize);

enum class RenderState : u32 {
	Idle = 0,  // nothing dispatched, or quiesced for reconfiguration
	Running,
	Paused,
	Converged, // targetSamplesPerPixel reached
};

struct RenderConfig {
	SceneView scene;
	RenderSettings transport;
	CameraFrame camera;
	u64 seedSalt;
	u32 targetSamplesPerPixel; // 0 = unlimited
	u32 width;
	u32 height;
	u32 tileCount;
	const TileRect *tiles;
	TileState *tileStates;
	AccumulationPixel *accumulation;
	u32 *display;
	u32 lightJobCount;   // light tracing jobs of every pass, 0 = off
	u32 *lightJobStates;
	CausticTarget caustics;
	PhotonStorage *photons; // photon buffers and the photon map, used when light tracing is on
	f32 photonRadius;       // initial photon gather radius, shrinks every pass
};

struct alignas(128) WorkerStats {
	volatile u64 sampleCount;        // written by the owner with fplAtomicStoreU64, read by the main thread with fplAtomicLoadU64
	volatile u64 rayCount;
	volatile u64 droppedSampleCount; // non-finite or negative samples, non-zero means a bug
};

struct JobSystem;

struct alignas(128) WorkerContext {
	JobSystem *jobs;
	u32 workerIndex;
	u64 sampleCountShadow;           // owner-only copies, so the owner never needs an atomic load of its own stats
	u64 rayCountShadow;
	u64 droppedSampleCountShadow;
	WorkerStats stats;
	Vec3f tileRadiance[MaxTileSize * MaxTileSize]; // job-local samples, stays in L1/L2
	SplatCache splatCache;                         // job-local light tracing splats, flushed at the end of every light job
};

struct JobSystem {
	// Hot lock-free fields, one padding block each
	PaddedU64 jobCursor;             // (epoch << 32) | nextJobIndex
	PaddedU32 completedJobCount;     // jobs of the current epoch committed or skipped
	PaddedU32 abortRequested;        // 1 while the main thread quiesces
	PaddedU32 displayCommitCount;    // +1 per tile commit, the main thread presents when it changed
	PaddedU64 displaySettingsPacked;

	// Protected by the mutex
	fplMutexHandle mutex;
	fplConditionVariable workCondition; // workers wait for a dispatch
	fplConditionVariable mainCondition; // the main thread waits for idle or converged
	RenderState state;
	b32 quit;
	u32 activeWorkerCount;           // workers between leaving the wait and re-entering it
	u32 generation;
	u32 publishedEpoch;              // current pass index
	u32 completedEpochCount;         // passes fully committed in this generation
	u64 dispatchSerial;              // +1 on every dispatch or epoch publish
	fplTimestamp convergedTimestamp;

	// Written by the main thread only while activeWorkerCount == 0, read lock-free by workers
	alignas(128) RenderConfig config;

	// Threads
	u32 workerCount;
	WorkerContext *workers;
	fplThreadHandle *workerThreads[MaxWorkerCount];
};

enum class JobResult : u32 {
	Completed = 0,
	Skipped,  // the tile already has this epoch's sample (resume after a pause)
	Aborted,  // abortRequested seen: nothing committed, nothing counted
};

static void ResolveTileLocked(const RenderConfig *config, const u32 jobIndex, const u64 displaySettingsPacked) {
	const TileRect &tile = config->tiles[jobIndex];
	TileState *tileState = &config->tileStates[jobIndex];
	if (tileState->sampleCount == 0) {
		return;
	}
	DisplaySettings display = UnpackDisplaySettings(displaySettingsPacked);
	f32 exposureScale = F32Power(2.0f, display.exposureEV);
	f64 inverseSampleCount = 1.0 / (f64)tileState->sampleCount;
	bool hasCaustics = config->lightJobCount > 0;
	for (u32 y = tile.y0; y < tile.y1; ++y) {
		for (u32 x = tile.x0; x < tile.x1; ++x) {
			size_t pixelIndex = (size_t)y * config->width + x;
			const AccumulationPixel &sum = config->accumulation[pixelIndex];
			f64 sumRed = sum.r;
			f64 sumGreen = sum.g;
			f64 sumBlue = sum.b;
			if (hasCaustics) {
				volatile u64 *causticSum = config->caustics.sums + pixelIndex * CausticChannelCount;
				u64 causticRed = fplAtomicLoadU64(&causticSum[0]);
				u64 causticGreen = fplAtomicLoadU64(&causticSum[1]);
				u64 causticBlue = fplAtomicLoadU64(&causticSum[2]);
				sumRed += (f64)causticRed * InverseCausticFixedPointScale;
				sumGreen += (f64)causticGreen * InverseCausticFixedPointScale;
				sumBlue += (f64)causticBlue * InverseCausticFixedPointScale;
			}
			f32 meanRed = (f32)(sumRed * inverseSampleCount);
			f32 meanGreen = (f32)(sumGreen * inverseSampleCount);
			f32 meanBlue = (f32)(sumBlue * inverseSampleCount);
			Vec3f mean = V3fInit(meanRed, meanGreen, meanBlue);
			config->display[pixelIndex] = ResolvePixel(mean, exposureScale, display.tonemapOperator, x, y);
		}
	}
	tileState->displayVersion = display.version;
}

static void CommitTile(WorkerContext *worker, const RenderConfig *config, const u32 jobIndex, const u32 samplesAfterCommit) {
	JobSystem *jobs = worker->jobs;
	const TileRect &tile = config->tiles[jobIndex];
	TileState *tileState = &config->tileStates[jobIndex];
	u32 tileWidth = tile.x1 - tile.x0;
	TileLock(tileState);
	bool isFirstCommit = tileState->sampleCount == 0; // the first commit stores, so the buffer never needs clearing
	for (u32 y = tile.y0; y < tile.y1; ++y) {
		for (u32 x = tile.x0; x < tile.x1; ++x) {
			u32 localIndex = (y - tile.y0) * tileWidth + (x - tile.x0);
			const Vec3f &sample = worker->tileRadiance[localIndex];
			AccumulationPixel *pixel = &config->accumulation[(size_t)y * config->width + x];
			if (isFirstCommit) {
				pixel->r = (f64)sample.r;
				pixel->g = (f64)sample.g;
				pixel->b = (f64)sample.b;
			} else {
				pixel->r += (f64)sample.r;
				pixel->g += (f64)sample.g;
				pixel->b += (f64)sample.b;
			}
		}
	}
	tileState->sampleCount = samplesAfterCommit;
	u64 displaySettingsPacked = fplAtomicLoadU64(&jobs->displaySettingsPacked.value);
	ResolveTileLocked(config, jobIndex, displaySettingsPacked);
	TileUnlock(tileState);
	fplAtomicIncrementU32(&jobs->displayCommitCount.value);
}

static void UpdateWorkerStats(WorkerContext *worker, const u32 sampleCount, const u32 rayCount, const u32 droppedSampleCount) {
	worker->sampleCountShadow += sampleCount;
	worker->rayCountShadow += rayCount;
	worker->droppedSampleCountShadow += droppedSampleCount;
	fplAtomicStoreU64(&worker->stats.sampleCount, worker->sampleCountShadow);
	fplAtomicStoreU64(&worker->stats.rayCount, worker->rayCountShadow);
	fplAtomicStoreU64(&worker->stats.droppedSampleCount, worker->droppedSampleCountShadow);
}

static JobResult ExecuteTileJob(WorkerContext *worker, const RenderConfig *config, const u32 epoch, const u32 jobIndex) {
	JobSystem *jobs = worker->jobs;
	const TileRect &tile = config->tiles[jobIndex];
	TileState *tileState = &config->tileStates[jobIndex];
	// The photon map of this pass is built by the first job of the pass from the photons of the previous pass
	const PhotonMapView *photonMap = fpl_null;
	if (config->lightJobCount > 0 && epoch > 0 && tileState->sampleCount < epoch + 1) {
		u32 readyValue = epoch + 1;
		for (;;) {
			u32 readyEpoch = fplAtomicLoadU32(&config->photons->readyEpoch);
			if (readyEpoch == readyValue) {
				break;
			}
			u32 waitAbortValue = fplAtomicLoadU32(&jobs->abortRequested.value);
			if (waitAbortValue != 0) {
				return(JobResult::Aborted);
			}
			fplThreadYield();
		}
		photonMap = &config->photons->view;
	}
	u32 sampleIndex = epoch; // one sample per pixel per pass
	u32 samplesAfterCommit = epoch + 1;

	// Resume after a pause re-dispatches the current epoch: tiles that already have this sample are skipped
	if (tileState->sampleCount >= samplesAfterCommit) {
		return(JobResult::Skipped);
	}
	fplAssert(tileState->sampleCount == sampleIndex);

	u32 tileWidth = tile.x1 - tile.x0;
	u32 tileHeight = tile.y1 - tile.y0;
	PathStats pathStats = {};
	u32 droppedSampleCount = 0;
	f64 lightPathCount = (f64)config->lightJobCount * (f64)PhotonsPerLightJob;
	f64 mergeArea = MergeAreaForEpoch(config->photonRadius, epoch);
	for (u32 y = tile.y0; y < tile.y1; ++y) {
		// Abort is polled once per row: an atomic load is a locked read-modify-write in FPL
		u32 abortValue = fplAtomicLoadU32(&jobs->abortRequested.value);
		if (abortValue != 0) {
			return(JobResult::Aborted);
		}
		for (u32 x = tile.x0; x < tile.x1; ++x) {
			u32 pixelIndex = y * config->width + x;
			PathSampler sampler = MakePathSampler(pixelIndex, sampleIndex, config->seedSalt);
			f32 filterDensity;
			Ray3f primaryRay = GenerateCameraRay(config->camera, x, y, sampler, filterDensity);
			// Solid angle density of this camera ray for its pixel: tent density / (pixel film area * cos^3)
			f32 cosCamera = V3fDot(primaryRay.direction, config->camera.forward);
			f32 cosCameraCubed = cosCamera * cosCamera * cosCamera;
			BidirectionalContext bidirectional;
			bidirectional.lightPathCount = lightPathCount;
			bidirectional.mergeArea = mergeArea;
			bidirectional.cameraDirectionPdf = (cosCamera > 0.0f) ? (filterDensity / (config->caustics.pixelFilmArea * cosCameraCubed)) : 0.0f;
			Vec3f radiance = TracePath(config->scene, config->transport, bidirectional, photonMap, primaryRay, sampler, pathStats);
			bool isFinite = IsFiniteV3(radiance);
			bool isValidSample = isFinite && radiance.r >= 0.0f && radiance.g >= 0.0f && radiance.b >= 0.0f;
			if (!isValidSample) {
				radiance = V3fZero(); // still counts as a sample, a NaN would poison the sum forever
				++droppedSampleCount;
			}
			u32 localIndex = (y - tile.y0) * tileWidth + (x - tile.x0);
			worker->tileRadiance[localIndex] = radiance;
		}
	}

	CommitTile(worker, config, jobIndex, samplesAfterCommit);
	UpdateWorkerStats(worker, tileWidth * tileHeight, pathStats.rayCount, droppedSampleCount);
	return(JobResult::Completed);
}

// Light jobs never abort halfway: splats cannot be taken back, so a job always runs to completion and records its pass
static JobResult ExecuteLightJob(WorkerContext *worker, const RenderConfig *config, const u32 epoch, const u32 lightJobIndex) {
	u32 *jobState = &config->lightJobStates[lightJobIndex];
	u32 passesAfterJob = epoch + 1;
	// Resume after a pause re-dispatches the current epoch: light jobs that already ran for it are skipped
	if (*jobState >= passesAfterJob) {
		return(JobResult::Skipped);
	}
	PathStats pathStats = {};
	u64 lightSeedSalt = config->seedSalt ^ LightTracingSeedSalt;
	u32 firstPhotonIndex = lightJobIndex * PhotonsPerLightJob;
	u32 photonBufferIndex = epoch % PhotonBufferCount;
	std::vector<PhotonRecord> &photonOutput = config->photons->jobPhotons[photonBufferIndex][lightJobIndex];
	photonOutput.clear();
	f64 mergeArea = MergeAreaForEpoch(config->photonRadius, epoch);
	for (u32 photonOffset = 0; photonOffset < PhotonsPerLightJob; ++photonOffset) {
		u32 photonIndex = firstPhotonIndex + photonOffset;
		PathSampler sampler = MakePathSampler(photonIndex, epoch, lightSeedSalt);
		TraceLightPath(config->scene, config->transport, config->camera, config->caustics, mergeArea, worker->splatCache, photonOutput, sampler, pathStats);
	}
	FlushSplatCache(config->caustics, worker->splatCache);
	*jobState = passesAfterJob;
	UpdateWorkerStats(worker, 0, pathStats.rayCount, 0);
	return(JobResult::Completed);
}

// First job of every pass with light tracing: builds this pass's photon map from the previous pass's photons, never aborts halfway
static JobResult ExecutePhotonMapJob(const RenderConfig *config, const u32 epoch) {
	PhotonStorage *storage = config->photons;
	u32 readyValue = epoch + 1;
	u32 readyEpoch = fplAtomicLoadU32(&storage->readyEpoch);
	if (readyEpoch == readyValue) {
		return(JobResult::Skipped);
	}
	if (epoch > 0) {
		u32 previousBufferIndex = (epoch - 1) % PhotonBufferCount;
		f32 radius = PhotonRadiusForEpoch(config->photonRadius, epoch);
		BuildPhotonMap(*storage, previousBufferIndex, radius);
	}
	fplAtomicStoreU32(&storage->readyEpoch, readyValue);
	return(JobResult::Completed);
}

static void PublishNextEpoch(JobSystem *jobs, const RenderConfig *config, const u32 finishedEpoch) {
	bool isPublished = false;
	fplMutexLock(&jobs->mutex);
	u32 nextEpoch = finishedEpoch + 1;
	jobs->completedEpochCount = nextEpoch;
	if (jobs->state == RenderState::Running) {
		bool isTargetReached = (config->targetSamplesPerPixel > 0) && (nextEpoch >= config->targetSamplesPerPixel);
		if (isTargetReached) {
			jobs->state = RenderState::Converged;
			jobs->convergedTimestamp = fplTimestampQuery();
			fplConditionBroadcast(&jobs->mainCondition);
		} else {
			jobs->publishedEpoch = nextEpoch;
			// Reset the completion counter BEFORE publishing the cursor, or an early completion of the new epoch is lost
			fplAtomicStoreU32(&jobs->completedJobCount.value, 0);
			u64 nextCursor = (u64)nextEpoch << JobCursorEpochShift;
			fplAtomicExchangeU64(&jobs->jobCursor.value, nextCursor);
			++jobs->dispatchSerial;
			isPublished = true;
		}
	}
	fplMutexUnlock(&jobs->mutex);
	if (isPublished) {
		fplConditionBroadcast(&jobs->workCondition);
	}
}

static void RunJobs(WorkerContext *worker, const RenderConfig *config) {
	JobSystem *jobs = worker->jobs;
	for (;;) {
		u32 abortValue = fplAtomicLoadU32(&jobs->abortRequested.value);
		if (abortValue != 0) {
			return;
		}
		// Claim first, then check the bounds: an out-of-range claim is harmless because the index only grows past tileCount
		u64 claimedCursor = fplAtomicFetchAndAddU64(&jobs->jobCursor.value, 1);
		u32 epoch = (u32)(claimedCursor >> JobCursorEpochShift);
		u32 jobIndex = (u32)(claimedCursor & JobCursorIndexMask);
		// With light tracing: the photon map job first, then the light jobs, then the tiles (which wait for the photon map)
		u32 photonMapJobCount = (config->lightJobCount > 0) ? 1 : 0;
		u32 firstTileJobIndex = photonMapJobCount + config->lightJobCount;
		u32 totalJobCount = firstTileJobIndex + config->tileCount;
		if (jobIndex >= totalJobCount) {
			return;
		}
		JobResult jobResult;
		if (jobIndex < photonMapJobCount) {
			jobResult = ExecutePhotonMapJob(config, epoch);
		} else if (jobIndex < firstTileJobIndex) {
			u32 lightJobIndex = jobIndex - photonMapJobCount;
			jobResult = ExecuteLightJob(worker, config, epoch, lightJobIndex);
		} else {
			u32 tileIndex = jobIndex - firstTileJobIndex;
			jobResult = ExecuteTileJob(worker, config, epoch, tileIndex);
		}
		if (jobResult == JobResult::Aborted) {
			return;
		}
		// Commit happens before this increment, so the publisher and the next claimer of the tile see the committed data
		u32 completedCount = fplAtomicAddAndFetchU32(&jobs->completedJobCount.value, 1);
		if (completedCount == totalJobCount) {
			PublishNextEpoch(jobs, config, epoch);
		}
	}
}

static void WorkerThreadProc(const fplThreadHandle * /* thread */, void *userData) {
	WorkerContext *worker = (WorkerContext *)userData;
	JobSystem *jobs = worker->jobs;
	u64 seenDispatchSerial = 0;
	fplMutexLock(&jobs->mutex);
	for (;;) {
		// Predicate loop under the mutex: no lost wake-ups (every predicate variable changes under the mutex), spurious wake-ups are harmless
		while (!jobs->quit && (jobs->state != RenderState::Running || jobs->dispatchSerial == seenDispatchSerial)) {
			fplConditionWait(&jobs->workCondition, &jobs->mutex, FPL_TIMEOUT_INFINITE);
		}
		if (jobs->quit) {
			break;
		}
		u64 enteredDispatchSerial = jobs->dispatchSerial;
		const RenderConfig *config = &jobs->config;
		++jobs->activeWorkerCount;
		fplMutexUnlock(&jobs->mutex);

		RunJobs(worker, config);

		fplMutexLock(&jobs->mutex);
		seenDispatchSerial = enteredDispatchSerial;
		--jobs->activeWorkerCount;
		if (jobs->activeWorkerCount == 0) {
			fplConditionBroadcast(&jobs->mainCondition);
		}
	}
	fplMutexUnlock(&jobs->mutex);
}

static bool JobSystemInit(JobSystem *jobs, const u32 requestedWorkerCount) {
	if (!fplMutexInit(&jobs->mutex)) {
		return(false);
	}
	if (!fplConditionInit(&jobs->workCondition) || !fplConditionInit(&jobs->mainCondition)) {
		return(false);
	}
	jobs->state = RenderState::Idle;
	jobs->quit = false;
	jobs->activeWorkerCount = 0;
	jobs->dispatchSerial = 0;
	fplAtomicStoreU32(&jobs->abortRequested.value, 1);

	u32 hardwareThreadCount = (u32)fplCPUGetCoreCount();
	u32 desiredWorkerCount = (requestedWorkerCount > 0) ? requestedWorkerCount : hardwareThreadCount;
	u32 cappedWorkerCount = fplMin(desiredWorkerCount, MaxWorkerCount);
	u32 clampedWorkerCount = fplMax(1u, cappedWorkerCount);
	size_t workerMemorySize = sizeof(WorkerContext) * clampedWorkerCount;
	jobs->workers = (WorkerContext *)fplMemoryAlignedAllocate(workerMemorySize, alignof(WorkerContext));
	if (jobs->workers == fpl_null) {
		return(false);
	}

	u32 createdCount = 0;
	for (u32 workerIndex = 0; workerIndex < clampedWorkerCount; ++workerIndex) {
		WorkerContext *worker = jobs->workers + workerIndex;
		new (worker) WorkerContext();
		worker->jobs = jobs;
		worker->workerIndex = workerIndex;
		fplThreadParameters parameters = fplZeroInit;
		parameters.runFunc = WorkerThreadProc;
		parameters.userData = worker;
		parameters.priority = fplThreadPriority_Low; // the main thread wins the CPU on Windows, no effect on POSIX
		fplThreadHandle *thread = fplThreadCreateWithParameters(&parameters);
		if (thread == fpl_null) {
			break;
		}
		jobs->workerThreads[createdCount] = thread;
		++createdCount;
	}
	jobs->workerCount = createdCount;
	bool result = createdCount > 0;
	return(result);
}

// Stops all work; on return no worker is active, and config, buffers and tiles may be changed
static void JobSystemQuiesce(JobSystem *jobs, const RenderState stateAfterQuiesce) {
	fplMutexLock(&jobs->mutex);
	jobs->state = stateAfterQuiesce;
	fplAtomicStoreU32(&jobs->abortRequested.value, 1);
	while (jobs->activeWorkerCount > 0) {
		fplConditionWait(&jobs->mainCondition, &jobs->mutex, FPL_TIMEOUT_INFINITE);
	}
	fplMutexUnlock(&jobs->mutex);
}

// Precondition: quiesced. Starts at the given epoch, a new generation also resets the completed pass count.
static void JobSystemDispatch(JobSystem *jobs, const u32 epoch, const bool isNewGeneration) {
	fplMutexLock(&jobs->mutex);
	fplAssert(jobs->activeWorkerCount == 0);
	if (isNewGeneration) {
		++jobs->generation;
		jobs->completedEpochCount = 0;
	}
	jobs->publishedEpoch = epoch;
	fplAtomicStoreU32(&jobs->completedJobCount.value, 0);
	u64 cursor = (u64)epoch << JobCursorEpochShift;
	fplAtomicExchangeU64(&jobs->jobCursor.value, cursor);
	fplAtomicStoreU32(&jobs->abortRequested.value, 0);
	jobs->state = RenderState::Running;
	++jobs->dispatchSerial;
	fplMutexUnlock(&jobs->mutex);
	fplConditionBroadcast(&jobs->workCondition);
}

static void JobSystemShutdown(JobSystem *jobs) {
	fplMutexLock(&jobs->mutex);
	jobs->quit = true;
	jobs->state = RenderState::Idle;
	fplAtomicStoreU32(&jobs->abortRequested.value, 1);
	fplMutexUnlock(&jobs->mutex);
	fplConditionBroadcast(&jobs->workCondition);
	if (jobs->workerCount > 0) {
		fplThreadWaitForAll(jobs->workerThreads, jobs->workerCount, 0, FPL_TIMEOUT_INFINITE);
	}
	// No fplThreadTerminate: the threads have returned, and their slots may already belong to someone else
	if (jobs->workers != fpl_null) {
		for (u32 workerIndex = 0; workerIndex < jobs->workerCount; ++workerIndex) {
			WorkerContext *worker = jobs->workers + workerIndex;
			worker->~WorkerContext();
		}
		fplMemoryAlignedFree(jobs->workers);
		jobs->workers = fpl_null;
	}
	fplConditionDestroy(&jobs->mainCondition);
	fplConditionDestroy(&jobs->workCondition);
	fplMutexDestroy(&jobs->mutex);
}

struct JobProgress {
	RenderState state;
	u32 completedEpochCount;
	u32 completedJobCount;
	fplTimestamp convergedTimestamp;
};

static JobProgress JobSystemGetProgress(JobSystem *jobs) {
	JobProgress result;
	fplMutexLock(&jobs->mutex);
	result.state = jobs->state;
	result.completedEpochCount = jobs->completedEpochCount;
	result.convergedTimestamp = jobs->convergedTimestamp;
	fplMutexUnlock(&jobs->mutex);
	result.completedJobCount = fplAtomicLoadU32(&jobs->completedJobCount.value);
	return(result);
}

struct WorkerTotals {
	u64 sampleCount;
	u64 rayCount;
	u64 droppedSampleCount;
};

static WorkerTotals JobSystemGetTotals(JobSystem *jobs) {
	WorkerTotals result = {};
	for (u32 workerIndex = 0; workerIndex < jobs->workerCount; ++workerIndex) {
		WorkerContext *worker = jobs->workers + workerIndex;
		u64 samples = fplAtomicLoadU64(&worker->stats.sampleCount);
		u64 rays = fplAtomicLoadU64(&worker->stats.rayCount);
		u64 dropped = fplAtomicLoadU64(&worker->stats.droppedSampleCount);
		result.sampleCount += samples;
		result.rayCount += rays;
		result.droppedSampleCount += dropped;
	}
	return(result);
}

//
// Application
//
static constexpr u32 DefaultWindowWidth = 1280;
static constexpr u32 DefaultWindowHeight = 768;
static constexpr u32 MaxImageDimension = 16384;

static constexpr f64 PreviewSettleSeconds = 0.15;            // full resolution resumes after this much time without a change
static constexpr f64 PreviewFrameBudgetSeconds = 0.012;      // one preview pass must fit in this
static constexpr u32 PreviewDivisors[] = { 1, 2, 3, 4, 6, 8 };
static constexpr u32 PreviewMaxBounces = 4;
static constexpr f64 InitialSamplesPerSecondPerWorker = 250000.0;
static constexpr f64 ThroughputSmoothing = 0.25;             // exponential moving average factor per statistics update

static constexpr f64 TitleUpdateIntervalSeconds = 0.25;
static constexpr f64 MinPresentIntervalSeconds = 1.0 / 60.0;
static constexpr f64 MaxPresentIntervalSeconds = 0.2;        // re-present regularly: X11 silently recreates (and clears) the backbuffer
static constexpr f64 RetonemapBudgetSeconds = 0.004;
static constexpr u32 IdleSleepMilliseconds = 8;
static constexpr u32 InteractiveSleepMilliseconds = 1;
static constexpr f64 MaxFrameDeltaSeconds = 0.1;
static constexpr f64 HeadlessWaitMilliseconds = 250.0;
static constexpr f64 HeadlessProgressIntervalSeconds = 1.0;

static constexpr f32 ExposureStepEV = 0.5f;
static constexpr f32 MinExposureEV = -10.0f;
static constexpr f32 MaxExposureEV = 10.0f;
static constexpr u32 BounceDepthChoices[] = { 1, 2, 4, 8, 12, 16, 32, 64 };

static constexpr f32 OrbitRadiansPerPixel = 0.005f;
static constexpr f32 ZoomFactorPerWheelNotch = 0.88f;
static constexpr f32 MinOrbitDistance = 0.05f;
static constexpr f32 MaxOrbitDistance = 1000.0f;
static constexpr f32 FlySpeedDistanceFactorPerSecond = 0.75f;
static constexpr f32 FlyFastMultiplier = 4.0f;
static constexpr f32 FallbackApertureDistanceFactor = 0.01f; // aperture for scenes without depth of field when it is toggled on

static constexpr u32 DefaultHeadlessSamplesPerPixel = 256;
static constexpr u64 RenderBytesPerPixel = sizeof(AccumulationPixel) + DisplayBufferCount * sizeof(u32) + CausticChannelCount * sizeof(u64);
static constexpr u64 MaxRenderBufferBytes32 = 1ull << 30;
static constexpr u64 MaxRenderBufferBytes64 = 1ull << 36;
static constexpr u32 MaxOutputFileCount = 4;
static constexpr u32 TitleBufferSize = 512;
static constexpr u32 PathBufferSize = 512;
static constexpr u32 KeyStateCount = 256;
static constexpr u32 MouseButtonCount = 3;
static constexpr f64 MillionFactor = 1.0 / 1000000.0;

struct UserRenderSettings {
	u32 maxBounces;
	b32 isFireflyClampEnabled;
	IntegratorMode integratorMode;
	u32 seed;
	u32 targetSamplesPerPixel; // 0 = unlimited
	b32 isDepthOfFieldEnabled;
	b32 isLightTracingEnabled; // caustics through light tracing (never in the preview)
};

struct InputState {
	b32 isKeyDown[KeyStateCount];
	b32 isMouseButtonDown[MouseButtonCount];
	b32 isShiftDown;
};

struct FrameCommands {
	b32 isSceneChanged;
	u32 sceneIndex;
	b32 isCameraReset;
	b32 isCameraChanged;
	b32 isRenderSettingsChanged;
	b32 isRestartRequested;
	b32 isPauseToggled;
	b32 isDisplaySettingsChanged;
	b32 isScreenshotRequested;
	b32 isHdrScreenshotRequested;
	b32 isHelpRequested;
};

struct App {
	JobSystem jobs;
	RenderTarget target;
	SceneEntry *scenes;
	u32 currentSceneIndex;
	OrbitCamera camera;
	UserRenderSettings settings;
	DisplaySettings display;
	InputState input;
	u32 previewDivisor;              // chosen when an interaction starts
	b32 wasInteracting;
	b32 hasInteraction;
	fplTimestamp lastInteractionTimestamp;
	fplTimestamp generationStartTimestamp;
	fplTimestamp pauseStartTimestamp;
	f64 pausedSeconds;
	f64 smoothedSamplesPerSecond;
	f64 smoothedRaysPerSecond;
	WorkerTotals lastTotals;
	fplTimestamp lastTotalsTimestamp;
	fplTimestamp lastTitleTimestamp;
	u32 retonemapCursor;             // == tileCount when no retonemap is pending
	u32 lastPresentedCommitCount;
	fplTimestamp lastPresentTimestamp;
	b32 forcePresent;
	u32 *blitColumnLookup;           // grow-only, one entry per backbuffer column
	size_t blitColumnCapacity;
	u32 screenshotCounter;
	b32 wasSettled;                  // paused or converged in the previous frame
	PhotonStorage photons;
};

static const Scene &GetCurrentScene(const App *app) {
	const Scene &result = app->scenes[app->currentSceneIndex].scene;
	return(result);
}

static void StoreDisplaySettings(App *app) {
	++app->display.version;
	u64 packed = PackDisplaySettings(app->display);
	fplAtomicStoreU64(&app->jobs.displaySettingsPacked.value, packed);
	app->retonemapCursor = 0;
	app->forcePresent = true;
}

static void ApplyCameraDepthOfField(App *app) {
	const Scene &scene = GetCurrentScene(app);
	f32 sceneAperture = scene.camera.defaultApertureRadius;
	f32 fallbackAperture = FallbackApertureDistanceFactor * app->camera.distance;
	f32 enabledAperture = (sceneAperture > 0.0f) ? sceneAperture : fallbackAperture;
	app->camera.apertureRadius = app->settings.isDepthOfFieldEnabled ? enabledAperture : 0.0f;
}

static void ResetCameraToScene(App *app) {
	const Scene &scene = GetCurrentScene(app);
	app->camera = MakeOrbitCamera(scene.camera);
	ApplyCameraDepthOfField(app);
}

static void ApplySceneDefaults(App *app, const u32 sceneIndex) {
	app->currentSceneIndex = sceneIndex;
	const SceneEntry &entry = app->scenes[sceneIndex];
	app->settings.maxBounces = entry.scene.maxBounces;
	app->settings.isDepthOfFieldEnabled = entry.scene.camera.defaultApertureRadius > 0.0f;
	app->display.exposureEV = entry.scene.exposureEV;
	app->display.tonemapOperator = entry.tonemapOperator;
	ResetCameraToScene(app);
}

static void BuildRenderConfig(App *app, RenderConfig *config) {
	const Scene &scene = GetCurrentScene(app);
	const RenderTarget &target = app->target;
	bool isPreview = target.resolutionDivisor > 1;
	config->scene = scene.view;
	config->transport.maxBounces = isPreview ? fplMin(app->settings.maxBounces, PreviewMaxBounces) : app->settings.maxBounces;
	config->transport.russianRouletteStartBounce = RussianRouletteStartBounce;
	config->transport.russianRouletteMinSurvival = RussianRouletteMinSurvival;
	config->transport.russianRouletteMaxSurvival = RussianRouletteMaxSurvival;
	// Preview frames are replaced within a fraction of a second, so they always clamp fireflies (biased, but never accumulated)
	bool isClamped = app->settings.isFireflyClampEnabled || isPreview;
	config->transport.indirectClampLuminance = isClamped ? FireflyClampLuminance : 0.0f;
	config->transport.mode = app->settings.integratorMode;
	config->camera = MakeCameraFrame(app->camera, target.outputWidth, target.outputHeight, target.resolutionDivisor);
	config->seedSalt = MakeSeedSalt(app->settings.seed);
	config->targetSamplesPerPixel = isPreview ? 0 : app->settings.targetSamplesPerPixel;
	config->width = target.width;
	config->height = target.height;
	config->tileCount = target.tileCount;
	config->tiles = target.tiles;
	config->tileStates = target.tileStates;
	config->accumulation = target.accumulation;
	config->display = target.displayBuffers[target.frontDisplayIndex];
	config->lightJobCount = target.lightJobCount;
	config->lightJobStates = target.lightJobStates;
	config->photons = &app->photons;
	config->photonRadius = scene.photonRadius;
	config->transport.isLightTracingActive = target.lightJobCount > 0;
	u32 photonsPerPass = target.lightJobCount * PhotonsPerLightJob;
	f32 renderPixelFilmWidth = 2.0f * config->camera.tanHalfX * config->camera.resolutionDivisor * config->camera.inverseOutputWidth;
	f32 renderPixelFilmHeight = 2.0f * config->camera.tanHalfY * config->camera.resolutionDivisor * config->camera.inverseOutputHeight;
	config->caustics.sums = target.causticSums;
	config->caustics.width = target.width;
	config->caustics.height = target.height;
	config->caustics.scale = (photonsPerPass > 0) ? (1.0f / (f32)photonsPerPass) : 0.0f;
	config->caustics.pixelFilmArea = renderPixelFilmWidth * renderPixelFilmHeight;
}

// Quiesce -> reconfigure -> dispatch: nothing from an old generation can ever land in a reset buffer, false when the render buffers could not be allocated
static bool RestartRendering(App *app, const u32 resolutionDivisor) {
	JobSystemQuiesce(&app->jobs, RenderState::Idle);
	bool isConfigured = ConfigureRenderTarget(&app->target, app->target.outputWidth, app->target.outputHeight, resolutionDivisor);
	if (!isConfigured) {
		return(false);
	}
	ResetTileStates(&app->target);
	const Scene &scene = GetCurrentScene(app);
	bool isLightTracing = app->settings.isLightTracingEnabled && scene.view.isLightTracingAvailable && resolutionDivisor == 1;
	bool isLightTracingConfigured = ConfigureLightTracing(&app->target, isLightTracing);
	if (!isLightTracingConfigured) {
		// Out of memory for the light job states: render without caustic light tracing, the path tracer alone is still unbiased
		ConfigureLightTracing(&app->target, false);
	}
	// Photon buffers per light job (grow-only capacity), no photon map before the first pass completed
	for (u32 bufferIndex = 0; bufferIndex < PhotonBufferCount; ++bufferIndex) {
		std::vector<std::vector<PhotonRecord> > &jobPhotons = app->photons.jobPhotons[bufferIndex];
		if (jobPhotons.size() < app->target.lightJobCount) {
			jobPhotons.resize(app->target.lightJobCount);
		}
		for (size_t jobIndex = 0; jobIndex < jobPhotons.size(); ++jobIndex) {
			jobPhotons[jobIndex].clear();
		}
	}
	fplAtomicStoreU32(&app->photons.readyEpoch, 0);
	BuildRenderConfig(app, &app->jobs.config);
	app->generationStartTimestamp = fplTimestampQuery();
	app->pausedSeconds = 0.0;
	app->retonemapCursor = app->target.tileCount; // reset tiles are resolved by their first commit
	app->forcePresent = true;
	JobSystemDispatch(&app->jobs, 0, true);
	return(true);
}

static void PauseRendering(App *app) {
	JobSystemQuiesce(&app->jobs, RenderState::Paused); // in-flight jobs are discarded and re-run on resume
	app->pauseStartTimestamp = fplTimestampQuery();
}

static void ResumeRendering(App *app) {
	fplTimestamp now = fplTimestampQuery();
	f64 pausedNow = fplTimestampElapsed(app->pauseStartTimestamp, now);
	app->pausedSeconds += pausedNow;
	fplMutexLock(&app->jobs.mutex);
	u32 epochToRestart = app->jobs.publishedEpoch;
	fplMutexUnlock(&app->jobs.mutex);
	JobSystemDispatch(&app->jobs, epochToRestart, false); // committed tiles are skipped
}

// Picks the preview resolution divisor so that one pass fits in the frame budget
static u32 ChoosePreviewDivisor(const f64 samplesPerSecond, const u32 outputWidth, const u32 outputHeight) {
	f64 budgetSamples = samplesPerSecond * PreviewFrameBudgetSeconds;
	u32 divisorCount = fplArrayCount(PreviewDivisors);
	for (u32 divisorIndex = 0; divisorIndex < divisorCount; ++divisorIndex) {
		u32 divisor = PreviewDivisors[divisorIndex];
		u32 previewWidth = (outputWidth + divisor - 1) / divisor;
		u32 previewHeight = (outputHeight + divisor - 1) / divisor;
		f64 previewPixelCount = (f64)previewWidth * (f64)previewHeight;
		if (previewPixelCount <= budgetSamples) {
			return(divisor);
		}
	}
	u32 largestDivisor = PreviewDivisors[divisorCount - 1];
	return(largestDivisor);
}

static f64 GetElapsedRenderSeconds(App *app, const JobProgress &progress, const fplTimestamp now) {
	fplTimestamp endTimestamp = now;
	if (progress.state == RenderState::Converged) {
		endTimestamp = progress.convergedTimestamp;
	} else if (progress.state == RenderState::Paused) {
		endTimestamp = app->pauseStartTimestamp;
	}
	f64 totalSeconds = fplTimestampElapsed(app->generationStartTimestamp, endTimestamp);
	f64 activeSeconds = totalSeconds - app->pausedSeconds;
	f64 result = fplMax(activeSeconds, 0.0);
	return(result);
}

static void UpdateThroughput(App *app, const fplTimestamp now) {
	WorkerTotals totals = JobSystemGetTotals(&app->jobs);
	f64 elapsed = fplTimestampElapsed(app->lastTotalsTimestamp, now);
	if (elapsed > 0.0) {
		u64 sampleDelta = totals.sampleCount - app->lastTotals.sampleCount;
		u64 rayDelta = totals.rayCount - app->lastTotals.rayCount;
		if (sampleDelta > 0) {
			f64 samplesPerSecond = (f64)sampleDelta / elapsed;
			f64 raysPerSecond = (f64)rayDelta / elapsed;
			app->smoothedSamplesPerSecond += ThroughputSmoothing * (samplesPerSecond - app->smoothedSamplesPerSecond);
			app->smoothedRaysPerSecond += ThroughputSmoothing * (raysPerSecond - app->smoothedRaysPerSecond);
		}
	}
	app->lastTotals = totals;
	app->lastTotalsTimestamp = now;
}

static void UpdateWindowTitle(App *app) {
	JobProgress progress = JobSystemGetProgress(&app->jobs);
	const Scene &scene = GetCurrentScene(app);
	const RenderTarget &target = app->target;
	// A restart earlier in this frame may have set the generation start after the frame timestamp
	fplTimestamp titleTimestamp = fplTimestampQuery();
	f64 elapsedSeconds = GetElapsedRenderSeconds(app, progress, titleTimestamp);
	u32 photonMapJobCount = (target.lightJobCount > 0) ? 1 : 0;
	u32 passJobCount = target.tileCount + target.lightJobCount + photonMapJobCount;
	u32 passPercent = (passJobCount > 0) ? (u32)((u64)progress.completedJobCount * 100 / passJobCount) : 0;
	f64 megaRaysPerSecond = app->smoothedRaysPerSecond * MillionFactor;
	const char *tonemapName = TonemapOperatorNames[(u32)app->display.tonemapOperator];
	const char *integratorName = IntegratorModeNames[(u32)app->settings.integratorMode];
	const char *stateSuffix = "";
	if (progress.state == RenderState::Paused) {
		stateSuffix = " | PAUSED";
	} else if (progress.state == RenderState::Converged) {
		stateSuffix = " | DONE";
	}
	char previewText[32] = "";
	if (target.resolutionDivisor > 1) {
		fplStringFormat(previewText, fplArrayCount(previewText), " preview 1/%u", target.resolutionDivisor);
	}
	const char *clampText = app->settings.isFireflyClampEnabled ? " clamp" : "";
	const char *dofText = app->settings.isDepthOfFieldEnabled ? " DOF" : "";
	const char *causticText = (target.lightJobCount > 0) ? " +caustics" : "";
	char title[TitleBufferSize];
	fplStringFormat(title, fplArrayCount(title), "FPL Raytracer | %s | %ux%u%s | %u spp (%u%%) | %.1f s | %.1f MR/s | EV %+.1f %s | %u bounces %s%s%s%s | %u threads%s", scene.name, target.outputWidth, target.outputHeight, previewText, progress.completedEpochCount, passPercent, elapsedSeconds, megaRaysPerSecond, app->display.exposureEV, tonemapName, app->settings.maxBounces, integratorName, causticText, clampText, dofText, app->jobs.workerCount, stateSuffix);
	fplSetWindowTitle(title);
}

static void PrintHelp() {
	fplConsoleOut("FPL Raytracer - progressive path tracer\n");
	fplConsoleOut("  1-5                     Select scene (0 = white furnace test)\n");
	fplConsoleOut("  Left mouse drag         Orbit\n");
	fplConsoleOut("  Right/Middle mouse drag Pan\n");
	fplConsoleOut("  Mouse wheel             Zoom\n");
	fplConsoleOut("  W/A/S/D/Q/E             Move (hold Shift to move faster)\n");
	fplConsoleOut("  Home/Backspace          Reset camera\n");
	fplConsoleOut("  Space                   Pause/Resume\n");
	fplConsoleOut("  R                       Restart accumulation\n");
	fplConsoleOut("  +/- PageUp/PageDown     Exposure\n");
	fplConsoleOut("  T                       Tone mapper (ACES, Neutral, None)\n");
	fplConsoleOut("  B / Shift+B             Max bounces\n");
	fplConsoleOut("  M                       Integrator mode (MIS, NEE only, BSDF only)\n");
	fplConsoleOut("  C                       Caustics through light tracing and photon mapping\n");
	fplConsoleOut("  F                       Firefly clamp (biased)\n");
	fplConsoleOut("  O                       Depth of field\n");
	fplConsoleOut("  P / Shift+P             Save screenshot (BMP) / also HDR image (PFM)\n");
	fplConsoleOut("  H/F1                    Help\n");
	fplConsoleOut("  Escape                  Quit\n");
}

static void ClearInput(App *app) {
	fplClearStruct(&app->input);
}

static void CycleBounces(App *app, const bool isBackward) {
	u32 choiceCount = fplArrayCount(BounceDepthChoices);
	u32 current = app->settings.maxBounces;
	u32 next = BounceDepthChoices[0];
	if (isBackward) {
		next = BounceDepthChoices[choiceCount - 1];
		for (u32 choiceIndex = choiceCount; choiceIndex > 0; --choiceIndex) {
			u32 choice = BounceDepthChoices[choiceIndex - 1];
			if (choice < current) {
				next = choice;
				break;
			}
		}
	} else {
		for (u32 choiceIndex = 0; choiceIndex < choiceCount; ++choiceIndex) {
			u32 choice = BounceDepthChoices[choiceIndex];
			if (choice > current) {
				next = choice;
				break;
			}
		}
	}
	app->settings.maxBounces = next;
}

static void ChangeExposure(App *app, const f32 deltaEV, FrameCommands &commands) {
	f32 newExposure = app->display.exposureEV + deltaEV;
	app->display.exposureEV = F32Clamp(newExposure, MinExposureEV, MaxExposureEV);
	commands.isDisplaySettingsChanged = true;
}

static void HandleKeyPress(App *app, const fplKey key, const bool isRepeat, FrameCommands &commands) {
	bool isShift = app->input.isShiftDown != 0;
	// Exposure also follows key repeats
	if (key == fplKey_OemPlus || key == fplKey_Add || key == fplKey_PageUp) {
		ChangeExposure(app, ExposureStepEV, commands);
		return;
	}
	if (key == fplKey_OemMinus || key == fplKey_Substract || key == fplKey_PageDown) {
		ChangeExposure(app, -ExposureStepEV, commands);
		return;
	}
	if (isRepeat) {
		return;
	}
	if (key >= fplKey_0 && key <= fplKey_9) {
		u32 sceneIndex = (u32)(key - fplKey_0);
		if (sceneIndex < SceneCount) {
			commands.isSceneChanged = true;
			commands.sceneIndex = sceneIndex;
		}
		return;
	}
	switch (key) {
		case fplKey_Escape:
			fplWindowShutdown();
			break;
		case fplKey_Space:
			commands.isPauseToggled = true;
			break;
		case fplKey_R:
			commands.isRestartRequested = true;
			break;
		case fplKey_T:
		{
			u32 nextTonemap = ((u32)app->display.tonemapOperator + 1) % (u32)TonemapOperator::Count;
			app->display.tonemapOperator = (TonemapOperator)nextTonemap;
			commands.isDisplaySettingsChanged = true;
		} break;
		case fplKey_B:
			CycleBounces(app, isShift);
			commands.isRenderSettingsChanged = true;
			break;
		case fplKey_M:
		{
			u32 nextMode = ((u32)app->settings.integratorMode + 1) % (u32)IntegratorMode::Count;
			app->settings.integratorMode = (IntegratorMode)nextMode;
			commands.isRenderSettingsChanged = true;
		} break;
		case fplKey_F:
			app->settings.isFireflyClampEnabled = !app->settings.isFireflyClampEnabled;
			commands.isRenderSettingsChanged = true;
			break;
		case fplKey_C:
			app->settings.isLightTracingEnabled = !app->settings.isLightTracingEnabled;
			commands.isRenderSettingsChanged = true;
			break;
		case fplKey_O:
			app->settings.isDepthOfFieldEnabled = !app->settings.isDepthOfFieldEnabled;
			ApplyCameraDepthOfField(app);
			commands.isRenderSettingsChanged = true;
			break;
		case fplKey_P:
			commands.isScreenshotRequested = true;
			commands.isHdrScreenshotRequested = isShift;
			break;
		case fplKey_H:
		case fplKey_F1:
			commands.isHelpRequested = true;
			break;
		case fplKey_Home:
		case fplKey_Backspace:
			commands.isCameraReset = true;
			break;
		default:
			break;
	}
}

static void OrbitCameraBy(App *app, const s32 deltaX, const s32 deltaY) {
	app->camera.yawRadians -= (f32)deltaX * OrbitRadiansPerPixel;
	f32 newPitch = app->camera.pitchRadians + (f32)deltaY * OrbitRadiansPerPixel;
	app->camera.pitchRadians = F32Clamp(newPitch, -MaxPitchRadians, MaxPitchRadians);
}

// The point under the cursor stays under it
static void PanCameraBy(App *app, const s32 deltaX, const s32 deltaY) {
	CameraFrame frame = MakeCameraFrame(app->camera, app->target.outputWidth, app->target.outputHeight, 1);
	f32 worldPerPixel = 2.0f * app->camera.distance * frame.tanHalfY / (f32)app->target.outputHeight;
	Vec3f horizontalMove = ((f32)deltaX * worldPerPixel) * frame.right;
	Vec3f verticalMove = ((f32)deltaY * worldPerPixel) * frame.up;
	app->camera.target = app->camera.target - horizontalMove + verticalMove;
}

static void ZoomCameraBy(App *app, const f32 wheelDelta) {
	f32 zoomFactor = F32Power(ZoomFactorPerWheelNotch, wheelDelta);
	f32 newDistance = app->camera.distance * zoomFactor;
	app->camera.distance = F32Clamp(newDistance, MinOrbitDistance, MaxOrbitDistance);
}

static void HandleEvent(App *app, const fplEvent &ev, FrameCommands &commands, fplTimestamp now) {
	switch (ev.type) {
		case fplEventType_Keyboard:
		{
			if (ev.keyboard.type != fplKeyboardEventType_Button) {
				break;
			}
			u32 shiftMask = fplKeyboardModifierFlags_LShift | fplKeyboardModifierFlags_RShift;
			app->input.isShiftDown = ((u32)ev.keyboard.modifiers & shiftMask) != 0;
			u32 keyIndex = (u32)ev.keyboard.mappedKey;
			bool isPress = ev.keyboard.buttonState == fplButtonState_Press;
			bool isRepeat = ev.keyboard.buttonState == fplButtonState_Repeat;
			bool isRelease = ev.keyboard.buttonState == fplButtonState_Release;
			if (keyIndex < KeyStateCount) {
				if (isPress) {
					app->input.isKeyDown[keyIndex] = true;
				} else if (isRelease) {
					app->input.isKeyDown[keyIndex] = false;
				}
			}
			if (isPress || isRepeat) {
				HandleKeyPress(app, ev.keyboard.mappedKey, isRepeat, commands);
			}
		} break;

		case fplEventType_Mouse:
		{
			if (ev.mouse.type == fplMouseEventType_Button) {
				s32 buttonIndex = (s32)ev.mouse.mouseButton;
				if (buttonIndex >= 0 && buttonIndex < (s32)MouseButtonCount) {
					app->input.isMouseButtonDown[buttonIndex] = ev.mouse.buttonState != fplButtonState_Release;
				}
			} else if (ev.mouse.type == fplMouseEventType_Move) {
				bool isOrbiting = app->input.isMouseButtonDown[fplMouseButtonType_Left] != 0;
				bool isPanning = (app->input.isMouseButtonDown[fplMouseButtonType_Right] != 0) || (app->input.isMouseButtonDown[fplMouseButtonType_Middle] != 0);
				bool hasDelta = (ev.mouse.deltaX != 0) || (ev.mouse.deltaY != 0);
				if (hasDelta && isOrbiting) {
					OrbitCameraBy(app, ev.mouse.deltaX, ev.mouse.deltaY);
					commands.isCameraChanged = true;
				} else if (hasDelta && isPanning) {
					PanCameraBy(app, ev.mouse.deltaX, ev.mouse.deltaY);
					commands.isCameraChanged = true;
				}
			} else if (ev.mouse.type == fplMouseEventType_Wheel) {
				ZoomCameraBy(app, ev.mouse.wheelDelta);
				ApplyCameraDepthOfField(app);
				commands.isCameraChanged = true;
			}
		} break;

		case fplEventType_Window:
		{
			switch (ev.window.type) {
				case fplWindowEventType_Resized:
					app->lastInteractionTimestamp = now;
					app->hasInteraction = true;
					app->forcePresent = true;
					break;
				case fplWindowEventType_LostFocus:
					ClearInput(app);
					app->forcePresent = true;
					break;
				case fplWindowEventType_GotFocus:
				case fplWindowEventType_Maximized:
				case fplWindowEventType_Restored:
				case fplWindowEventType_Shown:
				case fplWindowEventType_Exposed:
				case fplWindowEventType_PositionChanged:
					app->forcePresent = true;
					break;
				default:
					break;
			}
		} break;

		default:
			break;
	}
}

// Held keys move the target along the view axes
static bool ApplyFlyMovement(App *app, const f32 deltaSeconds) {
	const InputState &input = app->input;
	f32 forwardAmount = (f32)(input.isKeyDown[fplKey_W] - input.isKeyDown[fplKey_S]);
	f32 rightAmount = (f32)(input.isKeyDown[fplKey_D] - input.isKeyDown[fplKey_A]);
	f32 upAmount = (f32)(input.isKeyDown[fplKey_E] - input.isKeyDown[fplKey_Q]);
	if (forwardAmount == 0.0f && rightAmount == 0.0f && upAmount == 0.0f) {
		return(false);
	}
	CameraFrame frame = MakeCameraFrame(app->camera, app->target.outputWidth, app->target.outputHeight, 1);
	f32 speedFactor = input.isShiftDown ? FlyFastMultiplier : 1.0f;
	f32 step = FlySpeedDistanceFactorPerSecond * app->camera.distance * speedFactor * deltaSeconds;
	Vec3f forwardMove = (forwardAmount * step) * frame.forward;
	Vec3f rightMove = (rightAmount * step) * frame.right;
	Vec3f upMove = (upAmount * step) * UnitUp;
	app->camera.target = app->camera.target + forwardMove + rightMove + upMove;
	return(true);
}

// Re-resolves already rendered tiles after an exposure or tone mapper change, with a time budget per frame
static void RetonemapStep(App *app) {
	const RenderConfig *config = &app->jobs.config; // main thread owned, readable at any time
	u64 displaySettingsPacked = PackDisplaySettings(app->display);
	u32 currentVersion = (u32)((u64)app->display.version & DisplayVersionMask);
	fplTimestamp startTimestamp = fplTimestampQuery();
	while (app->retonemapCursor < config->tileCount) {
		TileState *tileState = &config->tileStates[app->retonemapCursor];
		TileLock(tileState);
		bool needsResolve = tileState->sampleCount > 0 && tileState->displayVersion != currentVersion;
		if (needsResolve) {
			ResolveTileLocked(config, app->retonemapCursor, displaySettingsPacked);
		}
		TileUnlock(tileState);
		++app->retonemapCursor;
		fplTimestamp nowTimestamp = fplTimestampQuery();
		f64 elapsedSeconds = fplTimestampElapsed(startTimestamp, nowTimestamp);
		if (elapsedSeconds >= RetonemapBudgetSeconds) {
			break;
		}
	}
	app->forcePresent = true;
}

// Re-resolves every tile, so light tracing splats that arrived after a tile's last commit are shown
static void ResolveAllTiles(App *app) {
	StoreDisplaySettings(app);
	while (app->retonemapCursor < app->target.tileCount) {
		RetonemapStep(app);
	}
}

static void EnsureBlitColumnCapacity(App *app, const u32 columnCount) {
	if (columnCount <= app->blitColumnCapacity) {
		return;
	}
	if (app->blitColumnLookup != fpl_null) {
		fplMemoryFree(app->blitColumnLookup);
	}
	app->blitColumnCapacity = columnCount;
	app->blitColumnLookup = (u32 *)fplMemoryAllocate(sizeof(u32) * columnCount);
}

// Nearest neighbor scaling from the render resolution to the backbuffer, the blit reads pixels while workers write them (aligned u32, never torn)
static void BlitDisplayToBackbuffer(App *app, fplVideoBackBuffer *backbuffer) {
	const RenderTarget *target = &app->target;
	const u32 *display = target->displayBuffers[target->frontDisplayIndex];
	u32 backbufferWidth = backbuffer->width;
	u32 backbufferHeight = backbuffer->height;
	bool isSameSize = backbufferWidth == target->width && backbufferHeight == target->height;
	if (isSameSize) {
		size_t rowSize = sizeof(u32) * backbufferWidth;
		for (u32 row = 0; row < backbufferHeight; ++row) {
			const u32 *sourceRow = display + (size_t)row * target->width;
			u8 *targetRow = (u8 *)backbuffer->pixels + (size_t)row * backbuffer->lineWidth;
			memcpy(targetRow, sourceRow, rowSize);
		}
		return;
	}
	EnsureBlitColumnCapacity(app, backbufferWidth);
	if (app->blitColumnLookup == fpl_null) {
		return;
	}
	u32 lastRenderColumn = target->width - 1;
	u32 lastRenderRow = target->height - 1;
	for (u32 column = 0; column < backbufferWidth; ++column) {
		u64 outputX = (u64)column * target->outputWidth / backbufferWidth;
		u32 renderX = (u32)(outputX / target->resolutionDivisor);
		app->blitColumnLookup[column] = fplMin(renderX, lastRenderColumn);
	}
	for (u32 row = 0; row < backbufferHeight; ++row) {
		u64 outputY = (u64)row * target->outputHeight / backbufferHeight;
		u32 unclampedRenderY = (u32)(outputY / target->resolutionDivisor);
		u32 renderY = fplMin(unclampedRenderY, lastRenderRow);
		const u32 *sourceRow = display + (size_t)renderY * target->width;
		u32 *targetRow = (u32 *)((u8 *)backbuffer->pixels + (size_t)row * backbuffer->lineWidth);
		for (u32 column = 0; column < backbufferWidth; ++column) {
			u32 sourceColumn = app->blitColumnLookup[column];
			targetRow[column] = sourceRow[sourceColumn];
		}
	}
}

//
// Image writers
//
static constexpr u32 BmpFileHeaderSize = 14;
static constexpr u32 BmpInfoHeaderSize = 40;
static constexpr u32 BmpPixelDataOffset = BmpFileHeaderSize + BmpInfoHeaderSize;
static constexpr u16 BmpPlaneCount = 1;
static constexpr u16 BmpBitsPerPixel = 24;
static constexpr u32 BmpPixelsPerMeter = 2835; // 72 dpi
static constexpr u32 BmpBytesPerPixel = 3;
static constexpr u32 BmpRowAlignmentMask = 3;
static constexpr u32 ByteMask = 0xFFu;
static constexpr u32 BitsPerByte = 8;

static void PutU16(std::vector<u8> &buffer, const size_t offset, const u16 value) {
	buffer[offset + 0] = (u8)(value & ByteMask);
	buffer[offset + 1] = (u8)((value >> BitsPerByte) & ByteMask);
}

static void PutU32(std::vector<u8> &buffer, const size_t offset, const u32 value) {
	for (u32 byteIndex = 0; byteIndex < sizeof(u32); ++byteIndex) {
		buffer[offset + byteIndex] = (u8)((value >> (byteIndex * BitsPerByte)) & ByteMask);
	}
}

static bool WriteFileBlock(const char *filePath, const void *data, const size_t size) {
	fplFileHandle file;
	if (!fplFileCreateBinary(filePath, &file)) {
		return(false);
	}
	size_t written = fplFileWriteBlock(&file, data, size);
	fplFileClose(&file);
	bool result = written == size;
	return(result);
}

// 24-bit bottom-up BMP of the tone mapped display buffer
static bool WriteBmp(const char *filePath, const u32 *pixels, const u32 width, const u32 height) {
	u32 rowStride = (width * BmpBytesPerPixel + BmpRowAlignmentMask) & ~BmpRowAlignmentMask;
	u32 pixelDataSize = rowStride * height;
	u32 fileSize = BmpPixelDataOffset + pixelDataSize;
	std::vector<u8> buffer(fileSize, 0);
	buffer[0] = 'B';
	buffer[1] = 'M';
	PutU32(buffer, 2, fileSize);
	PutU32(buffer, 10, BmpPixelDataOffset);
	PutU32(buffer, 14, BmpInfoHeaderSize);
	PutU32(buffer, 18, width);
	PutU32(buffer, 22, height);
	PutU16(buffer, 26, BmpPlaneCount);
	PutU16(buffer, 28, BmpBitsPerPixel);
	PutU32(buffer, 34, pixelDataSize);
	PutU32(buffer, 38, BmpPixelsPerMeter);
	PutU32(buffer, 42, BmpPixelsPerMeter);
	for (u32 row = 0; row < height; ++row) {
		u32 sourceRow = height - 1 - row;
		size_t rowOffset = BmpPixelDataOffset + (size_t)row * rowStride;
		for (u32 column = 0; column < width; ++column) {
			u32 pixel = pixels[(size_t)sourceRow * width + column];
			size_t pixelOffset = rowOffset + (size_t)column * BmpBytesPerPixel;
			buffer[pixelOffset + 0] = (u8)(pixel & ByteMask);
			buffer[pixelOffset + 1] = (u8)((pixel >> PixelGreenShift) & ByteMask);
			buffer[pixelOffset + 2] = (u8)((pixel >> PixelRedShift) & ByteMask);
		}
	}
	bool result = WriteFileBlock(filePath, buffer.data(), buffer.size());
	return(result);
}

// Mean radiance per pixel (no exposure, no tone mapping), read tile by tile under the tile lock
static void GatherMeanRadiance(const RenderTarget &target, std::vector<f32> &outRgb) {
	size_t pixelCount = (size_t)target.width * target.height;
	outRgb.assign(pixelCount * 3, 0.0f);
	for (u32 jobIndex = 0; jobIndex < target.tileCount; ++jobIndex) {
		const TileRect &tile = target.tiles[jobIndex];
		TileState *tileState = &target.tileStates[jobIndex];
		TileLock(tileState);
		u32 sampleCount = tileState->sampleCount;
		if (sampleCount > 0) {
			f64 inverseSampleCount = 1.0 / (f64)sampleCount;
			for (u32 y = tile.y0; y < tile.y1; ++y) {
				for (u32 x = tile.x0; x < tile.x1; ++x) {
					size_t pixelIndex = (size_t)y * target.width + x;
					const AccumulationPixel &sum = target.accumulation[pixelIndex];
					f64 sums[CausticChannelCount] = { sum.r, sum.g, sum.b };
					for (u32 channel = 0; channel < CausticChannelCount; ++channel) {
						if (target.lightJobCount > 0) {
							u64 causticSum = fplAtomicLoadU64(&target.causticSums[pixelIndex * CausticChannelCount + channel]);
							sums[channel] += (f64)causticSum * InverseCausticFixedPointScale;
						}
						outRgb[pixelIndex * 3 + channel] = (f32)(sums[channel] * inverseSampleCount);
					}
				}
			}
		}
		TileUnlock(tileState);
	}
}

// Portable float map: little-endian (negative scale), rows from bottom to top
static bool WritePfm(const char *filePath, const RenderTarget &target) {
	std::vector<f32> rgb;
	GatherMeanRadiance(target, rgb);
	char header[64];
	size_t headerLength = fplStringFormat(header, fplArrayCount(header), "PF\n%u %u\n-1.0\n", target.width, target.height);
	size_t rowSize = sizeof(f32) * 3 * target.width;
	std::vector<u8> buffer(headerLength + rowSize * target.height);
	memcpy(buffer.data(), header, headerLength);
	for (u32 row = 0; row < target.height; ++row) {
		u32 sourceRow = target.height - 1 - row;
		const f32 *source = rgb.data() + (size_t)sourceRow * target.width * 3;
		u8 *destination = buffer.data() + headerLength + (size_t)row * rowSize;
		memcpy(destination, source, rowSize);
	}
	bool result = WriteFileBlock(filePath, buffer.data(), buffer.size());
	return(result);
}

static bool HasFileExtension(const char *filePath, const char *extension) {
	size_t pathLength = strlen(filePath);
	size_t extensionLength = strlen(extension);
	if (pathLength < extensionLength) {
		return(false);
	}
	const char *pathExtension = filePath + pathLength - extensionLength;
	for (size_t charIndex = 0; charIndex < extensionLength; ++charIndex) {
		char pathChar = pathExtension[charIndex];
		char lowerPathChar = (pathChar >= 'A' && pathChar <= 'Z') ? (char)(pathChar - 'A' + 'a') : pathChar;
		if (lowerPathChar != extension[charIndex]) {
			return(false);
		}
	}
	return(true);
}

// Copies the tone mapped display tile by tile under the tile locks, workers may still commit while a screenshot is taken
static void GatherDisplayPixels(const RenderTarget &target, std::vector<u32> &outPixels) {
	size_t pixelCount = (size_t)target.width * target.height;
	outPixels.assign(pixelCount, OpaqueBlackPixel);
	const u32 *front = target.displayBuffers[target.frontDisplayIndex];
	for (u32 jobIndex = 0; jobIndex < target.tileCount; ++jobIndex) {
		const TileRect &tile = target.tiles[jobIndex];
		TileState *tileState = &target.tileStates[jobIndex];
		TileLock(tileState);
		for (u32 y = tile.y0; y < tile.y1; ++y) {
			size_t rowStart = (size_t)y * target.width;
			for (u32 x = tile.x0; x < tile.x1; ++x) {
				outPixels[rowStart + x] = front[rowStart + x];
			}
		}
		TileUnlock(tileState);
	}
}

static bool WriteImageFile(const char *filePath, const RenderTarget &target) {
	bool result = false;
	if (HasFileExtension(filePath, ".pfm")) {
		result = WritePfm(filePath, target);
	} else {
		std::vector<u32> displayPixels;
		GatherDisplayPixels(target, displayPixels);
		result = WriteBmp(filePath, displayPixels.data(), target.width, target.height);
	}
	return(result);
}

static void SaveScreenshot(App *app, const bool isHdr) {
	JobProgress progress = JobSystemGetProgress(&app->jobs);
	const Scene &scene = GetCurrentScene(app);
	++app->screenshotCounter;
	char bmpPath[PathBufferSize];
	fplStringFormat(bmpPath, fplArrayCount(bmpPath), "fpl_raytracer_%s_%uspp_%u.bmp", scene.fileName, progress.completedEpochCount, app->screenshotCounter);
	bool isBmpWritten = WriteImageFile(bmpPath, app->target);
	fplConsoleFormatOut("%s screenshot: %s\n", isBmpWritten ? "Saved" : "Failed to save", bmpPath);
	if (isHdr) {
		char pfmPath[PathBufferSize];
		fplStringFormat(pfmPath, fplArrayCount(pfmPath), "fpl_raytracer_%s_%uspp_%u.pfm", scene.fileName, progress.completedEpochCount, app->screenshotCounter);
		bool isPfmWritten = WriteImageFile(pfmPath, app->target);
		fplConsoleFormatOut("%s HDR image: %s\n", isPfmWritten ? "Saved" : "Failed to save", pfmPath);
	}
}

static void PrintImageStatistics(const RenderTarget &target) {
	std::vector<f32> rgb;
	GatherMeanRadiance(target, rgb);
	size_t pixelCount = (size_t)target.width * target.height;
	f64 sumRed = 0.0;
	f64 sumGreen = 0.0;
	f64 sumBlue = 0.0;
	f32 minLuminance = F32MaxValue;
	f32 maxLuminance = 0.0f;
	for (size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
		Vec3f color = V3fInit(rgb[pixelIndex * 3 + 0], rgb[pixelIndex * 3 + 1], rgb[pixelIndex * 3 + 2]);
		f32 luminance = Luminance(color);
		sumRed += color.r;
		sumGreen += color.g;
		sumBlue += color.b;
		minLuminance = F32Min(minLuminance, luminance);
		maxLuminance = F32Max(maxLuminance, luminance);
	}
	f64 inversePixelCount = 1.0 / (f64)pixelCount;
	fplConsoleFormatOut("Image mean RGB: %.5f %.5f %.5f, luminance min %.5f max %.5f\n", sumRed * inversePixelCount, sumGreen * inversePixelCount, sumBlue * inversePixelCount, minLuminance, maxLuminance);
}

//
// Command line
//
enum class ExitCode : int {
	Success = 0,
	UsageError = 1,
	InitFailed = 2,
	WriteFailed = 3,
	TimeLimitReached = 4,
};

struct CommandLineOptions {
	u32 sceneIndex;
	u32 width;
	u32 height;
	u32 samplesPerPixel;   // 0 = default (unlimited interactive, 256 headless)
	u32 threadCount;       // 0 = all hardware threads
	u32 maxBounces;        // 0 = scene default
	u32 seed;
	f32 exposureEV;
	b32 hasExposure;
	TonemapOperator tonemapOperator;
	b32 hasTonemap;
	IntegratorMode integratorMode;
	b32 isFireflyClampEnabled;
	b32 isLightTracingDisabled;
	f64 timeLimitSeconds;  // 0 = none
	const char *outputPaths[MaxOutputFileCount];
	u32 outputCount;
	b32 isHelpRequested;
};

static void PrintUsage() {
	fplConsoleOut("Usage: FPL_Raytracer [options]\n");
	fplConsoleOut("  --scene <0-5>                     Initial scene (default 1)\n");
	fplConsoleOut("  --width <px> --height <px>        Window or image size (default 1280x768)\n");
	fplConsoleOut("  --spp <N>                         Target samples per pixel (default unlimited, headless 256)\n");
	fplConsoleOut("  --threads <N>                     Worker count (default all hardware threads)\n");
	fplConsoleOut("  --bounces <N>                     Max bounces (default from the scene)\n");
	fplConsoleOut("  --seed <N>                        Sampler seed (default 0)\n");
	fplConsoleOut("  --exposure <EV>                   Exposure (default from the scene)\n");
	fplConsoleOut("  --tonemap <aces|neutral|none>     Tone mapper\n");
	fplConsoleOut("  --integrator <mis|nee|bsdf>       Integrator mode\n");
	fplConsoleOut("  --clamp <on|off>                  Firefly clamp (biased)\n");
	fplConsoleOut("  --caustics <on|off>               Caustics through light tracing (default on)\n");
	fplConsoleOut("  --out <file.bmp|file.pfm>         Render without a window, write the file and exit (up to 4 times)\n");
	fplConsoleOut("  --time-limit <seconds>            Stop a headless render early\n");
	fplConsoleOut("  --help                            Show this help\n");
}

// Only plain digits: strtoul accepts '-1' (wrapping to the maximum) and is 32 bits wide on Windows and 32-bit targets
static bool ParseU32Argument(const char *text, const u32 minValue, const u32 maxValue, u32 &outValue) {
	bool startsWithDigit = text[0] >= '0' && text[0] <= '9';
	if (!startsWithDigit) {
		return(false);
	}
	char *end = fpl_null;
	unsigned long long value = strtoull(text, &end, 10);
	if (end == text || *end != 0 || value < minValue || value > maxValue) {
		return(false);
	}
	outValue = (u32)value;
	return(true);
}

static bool ParseF64Argument(const char *text, f64 &outValue) {
	char *end = fpl_null;
	f64 value = strtod(text, &end);
	if (end == text || *end != 0) {
		return(false);
	}
	outValue = value;
	return(true);
}

static bool ParseCommandLine(const int argc, char **argv, CommandLineOptions &options) {
	fplClearStruct(&options);
	options.sceneIndex = DefaultSceneIndex;
	options.width = DefaultWindowWidth;
	options.height = DefaultWindowHeight;
	options.integratorMode = IntegratorMode::Mis;
	for (int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
		const char *name = argv[argumentIndex];
		if (fplIsStringEqual(name, "--help") || fplIsStringEqual(name, "-h")) {
			options.isHelpRequested = true;
			continue;
		}
		if (argumentIndex + 1 >= argc) {
			fplConsoleFormatError("Missing value for '%s'\n", name);
			return(false);
		}
		const char *value = argv[++argumentIndex];
		bool isValid = true;
		f64 number = 0.0;
		if (fplIsStringEqual(name, "--scene")) {
			isValid = ParseU32Argument(value, 0, SceneCount - 1, options.sceneIndex);
		} else if (fplIsStringEqual(name, "--width")) {
			isValid = ParseU32Argument(value, 1, MaxImageDimension, options.width);
		} else if (fplIsStringEqual(name, "--height")) {
			isValid = ParseU32Argument(value, 1, MaxImageDimension, options.height);
		} else if (fplIsStringEqual(name, "--spp")) {
			isValid = ParseU32Argument(value, 1, UINT32_MAX, options.samplesPerPixel);
		} else if (fplIsStringEqual(name, "--threads")) {
			isValid = ParseU32Argument(value, 1, MaxWorkerCount, options.threadCount);
		} else if (fplIsStringEqual(name, "--bounces")) {
			isValid = ParseU32Argument(value, 1, UINT16_MAX, options.maxBounces);
		} else if (fplIsStringEqual(name, "--seed")) {
			isValid = ParseU32Argument(value, 0, UINT32_MAX, options.seed);
		} else if (fplIsStringEqual(name, "--exposure")) {
			// The range check also rejects NaN and infinity
			bool isNumber = ParseF64Argument(value, number);
			isValid = isNumber && number >= (f64)MinExposureEV && number <= (f64)MaxExposureEV;
			if (isValid) {
				options.exposureEV = (f32)number;
				options.hasExposure = true;
			}
		} else if (fplIsStringEqual(name, "--tonemap")) {
			options.hasTonemap = true;
			if (fplIsStringEqual(value, "aces")) {
				options.tonemapOperator = TonemapOperator::AcesFitted;
			} else if (fplIsStringEqual(value, "neutral")) {
				options.tonemapOperator = TonemapOperator::PbrNeutral;
			} else if (fplIsStringEqual(value, "none")) {
				options.tonemapOperator = TonemapOperator::None;
			} else {
				isValid = false;
			}
		} else if (fplIsStringEqual(name, "--integrator")) {
			if (fplIsStringEqual(value, "mis")) {
				options.integratorMode = IntegratorMode::Mis;
			} else if (fplIsStringEqual(value, "nee")) {
				options.integratorMode = IntegratorMode::LightOnly;
			} else if (fplIsStringEqual(value, "bsdf")) {
				options.integratorMode = IntegratorMode::BsdfOnly;
			} else {
				isValid = false;
			}
		} else if (fplIsStringEqual(name, "--clamp")) {
			if (fplIsStringEqual(value, "on")) {
				options.isFireflyClampEnabled = true;
			} else if (fplIsStringEqual(value, "off")) {
				options.isFireflyClampEnabled = false;
			} else {
				isValid = false;
			}
		} else if (fplIsStringEqual(name, "--caustics")) {
			if (fplIsStringEqual(value, "on")) {
				options.isLightTracingDisabled = false;
			} else if (fplIsStringEqual(value, "off")) {
				options.isLightTracingDisabled = true;
			} else {
				isValid = false;
			}
		} else if (fplIsStringEqual(name, "--out")) {
			if (options.outputCount < MaxOutputFileCount) {
				options.outputPaths[options.outputCount++] = value;
			} else {
				isValid = false;
			}
		} else if (fplIsStringEqual(name, "--time-limit")) {
			isValid = ParseF64Argument(value, options.timeLimitSeconds) && options.timeLimitSeconds > 0.0;
		} else {
			fplConsoleFormatError("Unknown option '%s'\n", name);
			return(false);
		}
		if (!isValid) {
			fplConsoleFormatError("Invalid value '%s' for '%s'\n", value, name);
			return(false);
		}
	}
	// The render buffers must fit the address space: on 32-bit the byte count would wrap around otherwise
	u64 pixelCount = (u64)options.width * (u64)options.height;
	u64 renderBufferBytes = pixelCount * RenderBytesPerPixel;
	bool is32Bit = sizeof(void *) == sizeof(u32);
	u64 maxRenderBufferBytes = is32Bit ? MaxRenderBufferBytes32 : MaxRenderBufferBytes64;
	if (renderBufferBytes > maxRenderBufferBytes) {
		fplConsoleFormatError("Image size %ux%u is too large for this build\n", options.width, options.height);
		return(false);
	}
	return(true);
}

//
// Startup and main loops
//
static App *CreateApp(const CommandLineOptions &options) {
	void *appMemory = fplMemoryAlignedAllocate(sizeof(App), alignof(App));
	if (appMemory == fpl_null) {
		return(fpl_null);
	}
	// C++11 new ignores alignments above 16, so App (128 byte aligned) is placement constructed
	App *app = new (appMemory) App();
	app->scenes = new SceneEntry[SceneCount];
	BuildGgxAlbedoTables(GlobalGgxAlbedoTables);
	BuildScenes(app->scenes);
	app->settings.integratorMode = options.integratorMode;
	app->settings.isFireflyClampEnabled = options.isFireflyClampEnabled;
	app->settings.isLightTracingEnabled = !options.isLightTracingDisabled;
	app->settings.seed = options.seed;
	app->settings.targetSamplesPerPixel = options.samplesPerPixel;
	ApplySceneDefaults(app, options.sceneIndex);
	if (options.maxBounces > 0) {
		app->settings.maxBounces = options.maxBounces;
	}
	if (options.hasExposure) {
		app->display.exposureEV = options.exposureEV;
	}
	if (options.hasTonemap) {
		app->display.tonemapOperator = options.tonemapOperator;
	}
	app->target.outputWidth = options.width;
	app->target.outputHeight = options.height;
	return(app);
}

static void DestroyApp(App *app) {
	ReleaseRenderTarget(&app->target);
	if (app->blitColumnLookup != fpl_null) {
		fplMemoryFree(app->blitColumnLookup);
	}
	delete[] app->scenes;
	app->~App();
	fplMemoryAlignedFree(app);
}

static void PrintStartupInfo(const App *app) {
	char cpuName[256] = "";
	fplCPUGetName(cpuName, fplArrayCount(cpuName));
	fplConsoleFormatOut("CPU: %s, %u worker threads\n", cpuName, app->jobs.workerCount);
}

static void PrintDroppedSamples(App *app) {
	WorkerTotals totals = JobSystemGetTotals(&app->jobs);
	if (totals.droppedSampleCount > 0) {
		fplConsoleFormatError("Warning: %llu non-finite or negative samples were dropped\n", (unsigned long long)totals.droppedSampleCount);
	}
}

static ExitCode RunHeadless(App *app, const CommandLineOptions &options) {
	if (app->settings.targetSamplesPerPixel == 0) {
		app->settings.targetSamplesPerPixel = DefaultHeadlessSamplesPerPixel;
	}
	const Scene &scene = GetCurrentScene(app);
	fplConsoleFormatOut("Rendering '%s' at %ux%u with %u spp on %u threads\n", scene.name, app->target.outputWidth, app->target.outputHeight, app->settings.targetSamplesPerPixel, app->jobs.workerCount);
	u64 packed = PackDisplaySettings(app->display);
	fplAtomicStoreU64(&app->jobs.displaySettingsPacked.value, packed);
	fplTimestamp startTimestamp = fplTimestampQuery();
	fplTimestamp lastProgressTimestamp = startTimestamp;
	bool isStarted = RestartRendering(app, 1);
	if (!isStarted) {
		fplConsoleFormatError("Failed to allocate the render buffers for %ux%u\n", app->target.outputWidth, app->target.outputHeight);
		return(ExitCode::InitFailed);
	}
	bool isTimeLimitReached = false;
	fplMutexLock(&app->jobs.mutex);
	while (app->jobs.state != RenderState::Converged) {
		fplConditionWait(&app->jobs.mainCondition, &app->jobs.mutex, (fplTimeoutValue)HeadlessWaitMilliseconds);
		u32 completedPasses = app->jobs.completedEpochCount;
		fplMutexUnlock(&app->jobs.mutex);
		fplTimestamp now = fplTimestampQuery();
		f64 elapsedSeconds = fplTimestampElapsed(startTimestamp, now);
		f64 sinceProgress = fplTimestampElapsed(lastProgressTimestamp, now);
		if (sinceProgress >= HeadlessProgressIntervalSeconds) {
			fplConsoleFormatOut("  %u / %u spp, %.1f s\n", completedPasses, app->settings.targetSamplesPerPixel, elapsedSeconds);
			lastProgressTimestamp = now;
		}
		if (options.timeLimitSeconds > 0.0 && elapsedSeconds >= options.timeLimitSeconds) {
			isTimeLimitReached = true;
			fplMutexLock(&app->jobs.mutex);
			break;
		}
		fplMutexLock(&app->jobs.mutex);
	}
	fplMutexUnlock(&app->jobs.mutex);
	// Tiles keep their own sample count, so a partial image is still correct per tile
	JobSystemQuiesce(&app->jobs, RenderState::Idle);
	ResolveAllTiles(app);
	fplTimestamp endTimestamp = fplTimestampQuery();
	f64 totalSeconds = fplTimestampElapsed(startTimestamp, endTimestamp);
	WorkerTotals totals = JobSystemGetTotals(&app->jobs);
	f64 megaRaysPerSecond = (totalSeconds > 0.0) ? ((f64)totals.rayCount / totalSeconds * MillionFactor) : 0.0;
	fplConsoleFormatOut("Done in %.2f s, %.1f MR/s\n", totalSeconds, megaRaysPerSecond);
	PrintImageStatistics(app->target);
	PrintDroppedSamples(app);
	ExitCode result = isTimeLimitReached ? ExitCode::TimeLimitReached : ExitCode::Success;
	for (u32 outputIndex = 0; outputIndex < options.outputCount; ++outputIndex) {
		const char *outputPath = options.outputPaths[outputIndex];
		bool isWritten = WriteImageFile(outputPath, app->target);
		if (isWritten) {
			fplConsoleFormatOut("Wrote %s\n", outputPath);
		} else {
			fplConsoleFormatError("Failed to write %s\n", outputPath);
			result = ExitCode::WriteFailed;
		}
	}
	return(result);
}

static void RunInteractive(App *app) {
	u64 packed = PackDisplaySettings(app->display);
	fplAtomicStoreU64(&app->jobs.displaySettingsPacked.value, packed);
	fplVideoBackBuffer *initialBackbuffer = fplGetVideoBackBuffer();
	if (initialBackbuffer != fpl_null && initialBackbuffer->pixels != fpl_null && initialBackbuffer->width > 0 && initialBackbuffer->height > 0) {
		app->target.outputWidth = initialBackbuffer->width;
		app->target.outputHeight = initialBackbuffer->height;
	}
	f64 initialSamplesPerSecond = InitialSamplesPerSecondPerWorker * (f64)app->jobs.workerCount;
	app->smoothedSamplesPerSecond = initialSamplesPerSecond;
	fplTimestamp startTimestamp = fplTimestampQuery();
	app->lastTotalsTimestamp = startTimestamp;
	app->lastTitleTimestamp = startTimestamp;
	app->lastPresentTimestamp = startTimestamp;
	bool isStarted = RestartRendering(app, 1);
	if (!isStarted) {
		fplConsoleFormatError("Failed to allocate the render buffers for %ux%u\n", app->target.outputWidth, app->target.outputHeight);
		return;
	}
	PrintHelp();

	fplTimestamp lastFrameTimestamp = startTimestamp;
	while (fplWindowUpdate()) {
		fplTimestamp now = fplTimestampQuery();
		f64 frameSeconds = fplTimestampElapsed(lastFrameTimestamp, now);
		f32 deltaSeconds = (f32)fplMin(frameSeconds, MaxFrameDeltaSeconds);
		lastFrameTimestamp = now;

		// 1. Events (the backbuffer may be reallocated in here)
		FrameCommands commands = {};
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			HandleEvent(app, ev, commands, now);
		}

		// 2. Held-key movement
		bool isFlying = ApplyFlyMovement(app, deltaSeconds);
		if (isFlying) {
			commands.isCameraChanged = true;
		}
		if (commands.isCameraChanged) {
			app->lastInteractionTimestamp = now;
			app->hasInteraction = true;
		}

		// 3. Output size, polled after the events
		fplVideoBackBuffer *backbuffer = fplGetVideoBackBuffer();
		bool hasBackbuffer = backbuffer != fpl_null && backbuffer->pixels != fpl_null && backbuffer->width > 0 && backbuffer->height > 0;
		bool isSizeChanged = hasBackbuffer && (backbuffer->width != app->target.outputWidth || backbuffer->height != app->target.outputHeight);
		if (isSizeChanged) {
			app->target.outputWidth = backbuffer->width;
			app->target.outputHeight = backbuffer->height;
			app->lastInteractionTimestamp = now;
			app->hasInteraction = true;
		}

		// 4. Preview decision: the divisor is chosen once when an interaction starts, so throughput noise cannot cause restart oscillation
		f64 sinceInteraction = fplTimestampElapsed(app->lastInteractionTimestamp, now);
		bool isInteracting = app->hasInteraction && sinceInteraction < PreviewSettleSeconds;
		if (isInteracting && !app->wasInteracting) {
			app->previewDivisor = ChoosePreviewDivisor(app->smoothedSamplesPerSecond, app->target.outputWidth, app->target.outputHeight);
		}
		app->wasInteracting = isInteracting;
		u32 desiredDivisor = isInteracting ? app->previewDivisor : 1;

		// 5. Apply, at most one restart per frame
		if (commands.isSceneChanged) {
			ApplySceneDefaults(app, commands.sceneIndex);
			commands.isDisplaySettingsChanged = true;
		}
		if (commands.isCameraReset) {
			ResetCameraToScene(app);
		}
		bool isUserRestart = commands.isCameraChanged || isSizeChanged || commands.isSceneChanged || commands.isCameraReset || commands.isRenderSettingsChanged || commands.isRestartRequested;
		bool isRestartNeeded = isUserRestart || (desiredDivisor != app->target.resolutionDivisor);
		if (isRestartNeeded) {
			JobProgress progressBeforeRestart = JobSystemGetProgress(&app->jobs);
			bool wasPaused = progressBeforeRestart.state == RenderState::Paused;
			bool isRestarted = RestartRendering(app, desiredDivisor);
			if (!isRestarted) {
				fplConsoleFormatError("Failed to allocate the render buffers for %ux%u\n", app->target.outputWidth, app->target.outputHeight);
				fplWindowShutdown();
				break;
			}
			// Only user changes end a pause, the automatic switch from the preview to full resolution keeps it
			if (wasPaused && !isUserRestart) {
				PauseRendering(app);
			}
		}
		// After a restart in the same frame, so a pause key press is never lost
		if (commands.isPauseToggled) {
			JobProgress progress = JobSystemGetProgress(&app->jobs);
			if (progress.state == RenderState::Running) {
				PauseRendering(app);
			} else if (progress.state == RenderState::Paused) {
				ResumeRendering(app);
			}
		}
		if (commands.isDisplaySettingsChanged) {
			StoreDisplaySettings(app);
		}

		// Paused or converged: show the light tracing splats of the last pass in every tile
		JobProgress frameProgress = JobSystemGetProgress(&app->jobs);
		bool isSettled = frameProgress.state == RenderState::Paused || frameProgress.state == RenderState::Converged;
		if (isSettled && !app->wasSettled && app->target.lightJobCount > 0) {
			StoreDisplaySettings(app);
		}
		app->wasSettled = isSettled;

		// 6. Re-resolve tiles after a display settings change
		if (app->retonemapCursor < app->target.tileCount) {
			RetonemapStep(app);
		}

		// 7. Screenshot and help
		if (commands.isScreenshotRequested) {
			SaveScreenshot(app, commands.isHdrScreenshotRequested != 0);
		}
		if (commands.isHelpRequested) {
			PrintHelp();
		}

		// 8. Statistics and title
		f64 sinceTitle = fplTimestampElapsed(app->lastTitleTimestamp, now);
		if (sinceTitle >= TitleUpdateIntervalSeconds || isRestartNeeded || commands.isPauseToggled || commands.isDisplaySettingsChanged) {
			UpdateThroughput(app, now);
			UpdateWindowTitle(app);
			app->lastTitleTimestamp = now;
		}

		// 9. Present, the backbuffer pointer is fetched again because it is only valid until the next event poll
		u32 commitCount = fplAtomicLoadU32(&app->jobs.displayCommitCount.value);
		bool isDisplayChanged = commitCount != app->lastPresentedCommitCount;
		f64 sincePresent = fplTimestampElapsed(app->lastPresentTimestamp, now);
		bool isPresentNeeded = isDisplayChanged || app->forcePresent || sincePresent >= MaxPresentIntervalSeconds;
		bool isPresentAllowed = app->forcePresent || sincePresent >= MinPresentIntervalSeconds;
		if (isPresentNeeded && isPresentAllowed && hasBackbuffer) {
			BlitDisplayToBackbuffer(app, backbuffer);
			fplVideoFlip();
			app->lastPresentedCommitCount = commitCount;
			app->lastPresentTimestamp = now;
			app->forcePresent = false;
		}

		// 10. Sleep, the workers do the real work
		u32 sleepMilliseconds = isInteracting ? InteractiveSleepMilliseconds : IdleSleepMilliseconds;
		fplThreadSleep(sleepMilliseconds);
	}
}

int main(int argc, char **argv) {
	CommandLineOptions options;
	bool isParsed = ParseCommandLine(argc, argv, options);
	if (!isParsed || options.isHelpRequested) {
		PrintUsage();
		ExitCode usageExitCode = isParsed ? ExitCode::Success : ExitCode::UsageError;
		return((int)usageExitCode);
	}
	bool isHeadless = options.outputCount > 0;

	fplSettings settings = fplMakeDefaultSettings();
	fplStringFormat(settings.window.title, fplArrayCount(settings.window.title), "FPL Raytracer");
	settings.window.windowSize.width = options.width;
	settings.window.windowSize.height = options.height;
	settings.window.isResizable = true;
	settings.video.backend = fplVideoBackendType_Software;
	settings.video.isAutoSize = true;
	fplInitFlags initFlags = fplInitFlags_Console;
	if (!isHeadless) {
		initFlags = (fplInitFlags)(fplInitFlags_Console | fplInitFlags_Window | fplInitFlags_Video | fplInitFlags_Keyboard | fplInitFlags_Mouse);
	}
	if (!fplPlatformInit(initFlags, &settings)) {
		return((int)ExitCode::InitFailed);
	}

	ExitCode exitCode = ExitCode::Success;
	App *app = CreateApp(options);
	if (app == fpl_null) {
		fplConsoleFormatError("Failed to allocate the application\n");
		fplPlatformRelease();
		return((int)ExitCode::InitFailed);
	}
	bool isJobSystemReady = JobSystemInit(&app->jobs, options.threadCount);
	if (!isJobSystemReady) {
		fplConsoleFormatError("Failed to start the worker threads\n");
		exitCode = ExitCode::InitFailed;
	} else {
		PrintStartupInfo(app);
		if (isHeadless) {
			exitCode = RunHeadless(app, options);
		} else {
			RunInteractive(app);
			PrintDroppedSamples(app);
		}
	}
	JobSystemShutdown(&app->jobs);
	DestroyApp(app);
	fplPlatformRelease();
	return((int)exitCode);
}
