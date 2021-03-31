#ifndef SPH_H
#define SPH_H

#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

#include <stdio.h>
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>

#include "vecmath.h"
#include "utils.h"

//
// Boundary condition
//
constexpr float kSPHBoundaryAspect = 16.0f / 9.0f;
constexpr float kSPHBoundaryWidth = 10.0f;
constexpr float kSPHBoundaryHeight = kSPHBoundaryWidth / kSPHBoundaryAspect;
constexpr float kSPHBoundaryHalfWidth = kSPHBoundaryWidth * 0.5f;
constexpr float kSPHBoundaryHalfHeight = kSPHBoundaryHeight * 0.5f;
static const Vec2f kSPHGridOrigin = V2f(-kSPHBoundaryHalfWidth, -kSPHBoundaryHalfHeight);
static const Vec2f kSPHGridMin = kSPHGridOrigin;
static const Vec2f kSPHGridMax = kSPHGridOrigin + V2f(kSPHBoundaryWidth, kSPHBoundaryHeight);

//
// Default constants
//
// @NOTE: kH must be chosen well, everything else depends on this!
// @NOTE: Particle spacing must be smaller than kH, otherwise there will be no interaction, smaller = more particles, greater = less particles
// @NOTE: Near Stiffness must be greater than stiffness, smaller = more sticking, greater = less sticking
//
constexpr float kSPHDeltaTime = 1.0f / 60.0f;
constexpr int kSPHSubsteps = 1;
constexpr float kSPHSubstepDeltaTime = kSPHDeltaTime / (float)kSPHSubsteps;

constexpr float kSPHParticleRadius = 0.05f;
constexpr float kSPHKernelHeight = 6.0f * kSPHParticleRadius;
constexpr float kSPHParticleSpacing = kSPHKernelHeight * 0.5f;
constexpr float kSPHParticleCollisionRadius = kSPHParticleRadius;

constexpr float kSPHRestDensity = 20.0f;
constexpr float kSPHStiffness = 0.6f;
constexpr float kSPHNearStiffness = kSPHStiffness * 10.0f;
constexpr float kSPHLinearViscosity = 0.5f;
constexpr float kSPHQuadraticViscosity = 0.3f;

//
// Other constants
//
constexpr float kSPHParticleRenderRadius = kSPHParticleRadius * 1.0f;
constexpr float kSPHVisualPlaneLength = kSPHBoundaryHalfWidth;
constexpr float kSPHVolumeParticleDistributionScale = 0.01f;

// @NOTE: Collision margin must be chosen to be numerical significant, but visually insignificant
constexpr float kSPHCollisionMargin = 0.005f * 2.0f;
constexpr float kSPHCollisionEpsilon = FLT_EPSILON;

//
// Uniform grid
//
const float kSPHGridCellSize = kSPHKernelHeight;
const int kSPHGridCountX = (int)(kSPHBoundaryWidth / kSPHGridCellSize);
const int kSPHGridCountY = (int)(kSPHBoundaryHeight / kSPHGridCellSize);
const int kSPHGridTotalCount = kSPHGridCountX * kSPHGridCountY;
const float kSPHGridWidth = kSPHGridCountX * kSPHGridCellSize;
const float kSPHGridHeight = kSPHGridCountY * kSPHGridCellSize;

// Max constants
constexpr uint32_t kSPHMaxCellParticleCount = 500;
constexpr uint32_t kSPHMaxParticleNeighborCount = 1000;
constexpr uint32_t kSPHMaxParticleCount = 10000;
constexpr uint32_t kSPHMaxBodyCount = 100;
constexpr uint32_t kSPHMaxEmitterCount = 8;

// @NOTE: Particle radius must never be smaller collision margin
fplStaticAssert(kSPHParticleRadius > kSPHCollisionMargin);

struct SPHParameters {
	float kernelHeight;
	float cellSize;
	float particleSpacing;
	float invKernelHeight;
	float restDensity;
	float stiffness;
	float nearStiffness;
	float linearViscosity;
	float quadraticViscosity;

	SPHParameters() {
		kernelHeight = kSPHKernelHeight;
		cellSize = kSPHGridCellSize;
		particleSpacing = kSPHParticleSpacing;
		invKernelHeight = 1.0f / kernelHeight;
		restDensity = kSPHRestDensity;
		stiffness = kSPHStiffness;
		nearStiffness = kSPHNearStiffness;
		linearViscosity = kSPHLinearViscosity;
		quadraticViscosity = kSPHQuadraticViscosity;
	}

	SPHParameters(const SPHParameters &other) {
		kernelHeight = other.kernelHeight;
		invKernelHeight = 1.0f / kernelHeight;
		cellSize = other.cellSize;
		particleSpacing = other.particleSpacing;
		restDensity = other.restDensity;
		stiffness = other.stiffness;
		nearStiffness = other.nearStiffness;
		linearViscosity = other.linearViscosity;
		quadraticViscosity = other.quadraticViscosity;
	}

	SPHParameters(const float kernelHeight, const float cellSize, const float particleSpacing, const float restDensity, const float stiffness, const float nearStiffness, const float linearViscosity, const float quadraticViscosity) {
		this->kernelHeight = kSPHKernelHeight;
		this->cellSize = cellSize;
		this->particleSpacing = particleSpacing;
		this->invKernelHeight = 1.0f / kernelHeight;
		this->restDensity = kSPHRestDensity;
		this->stiffness = stiffness;
		this->nearStiffness = nearStiffness;
		this->linearViscosity = linearViscosity;
		this->quadraticViscosity = quadraticViscosity;
	}
};

struct SPHStatistics {
	size_t minParticleNeighborCount;
	size_t maxParticleNeighborCount;
	size_t minCellParticleCount;
	size_t maxCellParticleCount;

	struct {
		float emitters;
		float integration;
		float viscosityForces;
		float predict;
		float updateGrid;
		float neighborSearch;
		float densityAndPressure;
		float deltaPositions;
		float collisions;
	} time;

	SPHStatistics() :
		minParticleNeighborCount(kSPHMaxCellParticleCount),
		maxParticleNeighborCount(0),
		minCellParticleCount(kSPHMaxCellParticleCount),
		maxCellParticleCount(0) {
		time = {};
	}
};

enum SPHScenarioBodyType {
	SPHScenarioBodyType_None,

	SPHScenarioBodyType_Circle,
	SPHScenarioBodyType_Plane,
	SPHScenarioBodyType_LineSegment,
	SPHScenarioBodyType_Polygon,
};

static const size_t kMaxScenarioPolygonCount = 8;
struct SPHScenarioBody {
	SPHScenarioBodyType type;
	Vec2f position;
	Mat2f orientation;
	float radius;
	Vec2f localVerts[kMaxScenarioPolygonCount];
	size_t vertexCount;

	inline SPHScenarioBody() {
		vertexCount = 0;
	}

	static inline SPHScenarioBody CreateCircle(const Vec2f &position, const float radius) {
		SPHScenarioBody result = SPHScenarioBody();
		result.type = SPHScenarioBodyType::SPHScenarioBodyType_Circle;
		result.position = position;
		result.orientation = Mat2Identity();
		result.radius = radius;
		return(result);
	}

	static inline SPHScenarioBody CreatePlane(const Vec2f &position, const Vec2f &normal) {
		SPHScenarioBody result = SPHScenarioBody();
		result.type = SPHScenarioBodyType::SPHScenarioBodyType_Plane;
		result.position = position;
		result.orientation = Mat2FromAxis(normal);
		result.radius = 0;
		return(result);
	}

	static inline SPHScenarioBody CreateSegment(const Vec2f &position, float rotation, const Vec2f &a, const Vec2f &b) {
		SPHScenarioBody result = SPHScenarioBody();
		result.type = SPHScenarioBodyType::SPHScenarioBodyType_LineSegment;
		result.radius = 0.0f;
		result.position = position;
		result.orientation = Mat2FromAngle(rotation);
		result.vertexCount = 2;
		result.localVerts[0] = a;
		result.localVerts[1] = b;
		return(result);
	}

	static inline SPHScenarioBody CreateBox(const Vec2f &position, float rotation, const Vec2f &ext) {
		SPHScenarioBody result = SPHScenarioBody();
		result.type = SPHScenarioBodyType::SPHScenarioBodyType_Polygon;
		result.radius = 0;
		result.position = position;
		result.orientation = Mat2FromAngle(rotation);
		result.vertexCount = 4;
		result.localVerts[0] = V2f(ext.x, ext.y);
		result.localVerts[1] = V2f(-ext.x, ext.y);
		result.localVerts[2] = V2f(-ext.x, -ext.y);
		result.localVerts[3] = V2f(ext.x, -ext.y);
		return(result);
	}
};

struct SPHScenarioVolume {
	Vec2f position;
	Vec2f size;
	Vec2f force;

	inline SPHScenarioVolume() {
	}
	inline SPHScenarioVolume(const Vec2f &position, const Vec2f &size, const Vec2f &force) {
		this->position = position;
		this->size = size;
		this->force = force;
	}
};

struct SPHScenarioEmitter {
	// @NOTE: Initial position
	Vec2f position;
	// @NOTE: The direction the particles are being emitted
	Vec2f direction;
	// @NOTE: One dimensional size of one column of particles
	float radius;
	// @NOTE: How fast the particles are moving initially
	float speed;
	// @NOTE: How fast the particles are emitted per second
	float rate;
	// @NOTE: Total duration in seconds
	float duration;

	SPHScenarioEmitter() {
		position = direction = V2f(0,0);
		radius = speed = rate = duration = 0.0f;
	}

	SPHScenarioEmitter(const Vec2f &pos, const Vec2f &dir, const float radius, const float speed, const float rate, const float duration) {
		this->position = pos;
		this->direction = dir;
		this->radius = radius;
		this->speed = speed;
		this->rate = rate;
		this->duration = duration;
	}
};

const uint32_t kSPHMaxScenarioVolumeCount = 8;
const uint32_t kSPHMaxScenarioBodyCount = 32;
const uint32_t kSPHMaxScenarioEmitterCount = 8;
struct SPHScenario {
	char name[100];

	Vec2f gravity;

	size_t volumeCount;
	SPHScenarioVolume volumes[kSPHMaxScenarioVolumeCount];

	size_t bodyCount;
	SPHScenarioBody bodies[kSPHMaxScenarioBodyCount];

	size_t emitterCount;
	SPHScenarioEmitter emitters[kSPHMaxScenarioEmitterCount];

	SPHParameters parameters;

	inline SPHScenario(const char *name, const Vec2f &gravity, const std::vector<SPHScenarioVolume> &volumes, const std::vector<SPHScenarioEmitter> &emitters, const std::vector<SPHScenarioBody> &bodies, const SPHParameters &params) {
		strcpy_s(this->name, 100, name);

		this->parameters = params;
		this->gravity = gravity;

		volumeCount = volumes.size();
		assert(volumeCount <= kSPHMaxScenarioVolumeCount);
		for (size_t volumeIndex = 0; volumeIndex < volumeCount; ++volumeIndex) {
			this->volumes[volumeIndex] = volumes[volumeIndex];
		}

		bodyCount = bodies.size();
		assert(bodyCount <= kSPHMaxScenarioBodyCount);
		for (size_t bodyIndex = 0; bodyIndex < bodyCount; ++bodyIndex) {
			this->bodies[bodyIndex] = bodies[bodyIndex];
		}

		emitterCount = emitters.size();
		assert(emitterCount <= kSPHMaxScenarioEmitterCount);
		for (size_t emitterIndex = 0; emitterIndex < emitterCount; ++emitterIndex) {
			this->emitters[emitterIndex] = emitters[emitterIndex];
		}
	}
};

const float kSPHDambreakWallWidth = kSPHBoundaryWidth * 0.05f;
const float kSPHDambreakWallHeight = kSPHBoundaryHeight * 0.85f;
const float kSPHDambreakVolumeWidth = kSPHBoundaryWidth * 0.25f;
const float kSPHDambreakVolumeHeight = kSPHBoundaryHeight * 0.95f;

const float kSPHBlobVolumeWidth = kSPHBoundaryWidth * 0.5f;
const float kSPHBlobVolumeHeight = kSPHBoundaryHeight * 0.5f;

static SPHScenario SPHScenarios[] = {
	SPHScenario("Dambreak", V2f(0, -10),
	{
		SPHScenarioVolume(V2f(-kSPHBoundaryHalfWidth + kSPHDambreakVolumeWidth * 0.5f, 0), V2f(kSPHDambreakVolumeWidth, kSPHDambreakVolumeHeight), V2f(0,0)),
	},
	{
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
		SPHScenarioBody::CreateBox(V2f(-kSPHBoundaryHalfWidth + kSPHDambreakVolumeWidth + kSPHDambreakWallWidth * 0.5f + kSPHParticleCollisionRadius, kSPHBoundaryHeight * 0.1f), 0.0f, V2f(kSPHDambreakWallWidth * 0.5f, kSPHDambreakWallHeight * 0.5f)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 6.0f, kSPHRestDensity, kSPHStiffness, kSPHNearStiffness, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Dambreak x 2", V2f(0, -10),
	{
		SPHScenarioVolume(V2f(-kSPHBoundaryHalfWidth + kSPHDambreakVolumeWidth * 0.5f, 0), V2f(kSPHDambreakVolumeWidth, kSPHDambreakVolumeHeight), V2f(0,0)),
		SPHScenarioVolume(V2f(kSPHBoundaryHalfWidth - kSPHDambreakVolumeWidth * 0.5f, 0), V2f(kSPHDambreakVolumeWidth, kSPHDambreakVolumeHeight), V2f(0,0)),
	},
	{
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 3.0f, kSPHRestDensity, kSPHStiffness, kSPHStiffness * 20.0f, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Blob", V2f(0, 0),
	{
		SPHScenarioVolume(V2f(0, 0), V2f(kSPHBlobVolumeWidth, kSPHBlobVolumeHeight), V2f(0,0)),
	},
	{
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 3.0f, kSPHRestDensity, kSPHStiffness, kSPHNearStiffness, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Blob x 2", V2f(0, 0),
	{
		SPHScenarioVolume(V2f(-kSPHBlobVolumeHeight * 0.75f, 0), V2f(kSPHBlobVolumeHeight * 0.75f, kSPHBlobVolumeHeight * 0.75f), V2f(10, 0)),
		SPHScenarioVolume(V2f(kSPHBlobVolumeHeight * 0.75f, 0), V2f(kSPHBlobVolumeHeight * 0.75f, kSPHBlobVolumeHeight * 0.75f), V2f(-10, 0)),
	},
	{
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 3.0f, kSPHRestDensity, kSPHStiffness, kSPHNearStiffness, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Liquid", V2f(0, -2),
	{
	},
	{
		SPHScenarioEmitter(V2f(-3.5f, 0.0f), V2f(1, 0), kSPHKernelHeight * 3, 2.5f, 15.0f, 30.0f),
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 4.0f, kSPHRestDensity, kSPHStiffness, kSPHStiffness * 10.0f, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Glass", V2f(0, -10),
	{
	},
	{
		SPHScenarioEmitter(V2f(-1.5f, 2.0f), V2f(1, 0), kSPHKernelHeight * 3, 2.5f, 15.0f, 25.0f),
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
		SPHScenarioBody::CreateBox(V2f(0.0f, -2.0f), 0.0f, V2f(1.0f, 0.2f)),
		SPHScenarioBody::CreateBox(V2f(-1.0f, -0.5f), 0.0f, V2f(0.2f, 1.5f)),
		SPHScenarioBody::CreateBox(V2f(1.0f, -0.5f), 0.0f, V2f(0.2f, 1.5f)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 4.0f, kSPHRestDensity, kSPHStiffness, kSPHStiffness * 6.0f, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Fontain", V2f(0, -10),
	{
	},
	{
		SPHScenarioEmitter(V2f(0, -kSPHBoundaryHalfHeight + 1.0f), V2f(0, 1), kSPHKernelHeight * 4, 8.0f, 15.0f, 25.0f),
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 4.0f, kSPHRestDensity, kSPHStiffness, kSPHStiffness * 2.0f, kSPHLinearViscosity, kSPHQuadraticViscosity)),

	SPHScenario("Fun", V2f(0, -10),
	{
	},
	{
		SPHScenarioEmitter(V2f(-4, 2), V2f(1, 0), kSPHKernelHeight * 4, 3.5f, 15.0f, 20.0f),
	},
	{
		SPHScenarioBody::CreatePlane(V2f(0, -kSPHBoundaryHalfHeight), V2f(0, 1)),
		//SPHScenarioBody::CreatePlane(V2f(0, kSPHBoundaryHalfHeight), V2f(0, -1)),
		SPHScenarioBody::CreatePlane(V2f(-kSPHBoundaryHalfWidth, 0), V2f(1, 0)),
		SPHScenarioBody::CreatePlane(V2f(kSPHBoundaryHalfWidth, 0), V2f(-1, 0)),
		SPHScenarioBody::CreateBox(V2f(-1.5f, 1.0f), kDeg2Rad * -2.5f, V2f(3.5f, 0.1f)),
		SPHScenarioBody::CreateBox(V2f(1.5f, -0.25f), kDeg2Rad * 2.5f, V2f(3.5f, 0.1f)),
		SPHScenarioBody::CreateCircle(V2f(-4.0f, -1.5f), 0.5f),
		SPHScenarioBody::CreateBox(V2f(0, -kSPHBoundaryHalfHeight + 0.5f), 0, V2f(0.3f, 1.0f)),
	},
	SPHParameters(kSPHKernelHeight, kSPHGridCellSize, kSPHKernelHeight / 4.0f, kSPHRestDensity, kSPHStiffness, kSPHStiffness * 6.0f, kSPHLinearViscosity, kSPHQuadraticViscosity)),
};

force_inline bool SPHIsPositionInGrid(int x, int y) {
	bool result = ((x >= 0 && x < kSPHGridCountX) && (y >= 0 && y < kSPHGridCountY));
	return(result);
}

force_inline size_t SPHComputeCellOffset(const int x, const int y) {
	size_t result = y * kSPHGridCountX + x;
	assert(result >= 0 && result < kSPHGridTotalCount);
	return(result);
}

force_inline Vec2i SPHComputeCellPos(const Vec2f &p, const Vec2f &center, const float cellSize) {
	int x = (int)((p.x + center.x) / cellSize);
	int y = (int)((p.y + center.y) / cellSize);
	return V2i(x, y);
}

force_inline Vec2i SPHComputeCellIndex(const Vec2f & p) {
	Vec2f center = V2f(kSPHBoundaryHalfWidth, kSPHBoundaryHalfHeight);
	Vec2i cellPos = SPHComputeCellPos(p, center, kSPHGridCellSize);
	cellPos.x = std::min(std::max(cellPos.x, 0), (int)kSPHGridCountX - 1);
	cellPos.y = std::min(std::max(cellPos.y, 0), (int)kSPHGridCountY - 1);
	Vec2i result = V2i(cellPos.x, cellPos.y);
	return (result);
}

force_inline void SPHComputeDensity(const SPHParameters &params, const Vec2f &position, const Vec2f &neighborPosition, float outDensity[2]) {
	Vec2f Rij = neighborPosition - position;
	float rijSquared = Vec2Dot(Rij, Rij);

	// @TODO: Make it branch-free
	if (rijSquared < (params.kernelHeight * params.kernelHeight)) {
		float rij = sqrtf(rijSquared);
		float term = 1.0f - rij * params.invKernelHeight;
		outDensity[0] += (term * term);
		outDensity[1] += (term * term * term);
	}
}

force_inline void SPHComputePressure(const SPHParameters &params, const float density[2], float outPressure[2]) {
	outPressure[0] = params.stiffness * (density[0] - params.restDensity);
	outPressure[1] = params.nearStiffness * density[1];
}

force_inline void SPHComputeDelta(const SPHParameters &params, const Vec2f &position, const Vec2f &neighborPosition, const float pressure[2], const float deltaTime, Vec2f *outDelta) {
	// @TODO: Make it branch-free

	Vec2f Rij = neighborPosition - position;
	float rijSquared = Vec2Dot(Rij, Rij);
	if (rijSquared < (params.kernelHeight * params.kernelHeight)) {
		float rij = sqrtf(rijSquared);
		Vec2f n = Vec2Normalize(Rij);
		float term = 1.0f - rij * params.invKernelHeight;
		float d = (deltaTime * deltaTime) * (pressure[0] * term + pressure[1] * (term * term));
		*outDelta = Vec2MultScalar(n, d);
	}
}

force_inline void SPHComputeViscosityForce(const SPHParameters &params, const Vec2f &position, const Vec2f &neighborPosition, const Vec2f &velocity, const Vec2f &neighborVelocity, Vec2f *outForce) {
	// @TODO: Make it branch-free

	Vec2f Rij = neighborPosition - position;
	float rijSquared = Vec2Dot(Rij, Rij);
	if (rijSquared < (params.kernelHeight * params.kernelHeight)) {
		float rij = sqrtf(rijSquared);
		float q = rij * params.invKernelHeight;
		Vec2f n = Vec2Normalize(Rij);
		float u = Vec2Dot(velocity - neighborVelocity, n);
		if (u > 0.0f) {
			float f = (1.0f - q) * (params.linearViscosity * u + params.quadraticViscosity * (u * u));
			*outForce = Vec2MultScalar(n, f);
		}
	}
}

force_inline void SPHSolvePlaneCollision(Vec2f *position, const Vec2f &normal, const float distance) {
	Vec2f p = normal * distance;
	Vec2f particlePos = *position;
	Vec2f delta = particlePos - p;
	float proj = Vec2Dot(delta, normal);
	if (proj <= kSPHParticleCollisionRadius) {
		float penetration = kSPHParticleCollisionRadius - proj;
		particlePos += normal * penetration;
		*position = particlePos;
	}
}

force_inline void SPHSolveCircleCollision(Vec2f *particlePosition, const Vec2f &circlePos, const float circleRadius) {
	float bothRadius = circleRadius + kSPHParticleCollisionRadius;
	Vec2f particlePos = *particlePosition;
	Vec2f deltaPos = particlePos - circlePos;
	float distanceSquared = Vec2Dot(deltaPos, deltaPos);
	if (distanceSquared <= bothRadius * bothRadius) {
		Vec2f normal = V2f(0, -1);
		float penetration = bothRadius;
		if (abs(distanceSquared) > 0) {
			float distance = sqrtf(distanceSquared);
			normal = deltaPos * (1.0f / distance);
			penetration = bothRadius - distance;
			particlePos += normal * penetration;
			*particlePosition = particlePos;
		}
	}
}

static void SPHSolveLineSegmentCollision(Vec2f *particlePosition, const Vec2f &a, const Vec2f &b) {
	float bothRadius = kSPHCollisionMargin + kSPHParticleCollisionRadius;
	Vec2f particlePos = *particlePosition;

	Vec2f e = b - a;
	float u = Vec2Dot(e, b - particlePos);
	float v = Vec2Dot(e, particlePos - a);

	Vec2f closest = V2f(0,0);
	Vec2f normal = V2f(0,0);

	if (v <= 0.0f) {
		// Region A
		closest = a;
		Vec2f d = particlePos - closest;
		float dd = Vec2Dot(d, d);
		if (dd > bothRadius * bothRadius) {
			return;
		}
		normal = Vec2Normalize(particlePos - closest);
	} else if (u <= 0.0f) {
		// Region B
		closest = b;
		Vec2f d = particlePos - closest;
		float dd = Vec2Dot(d, d);
		if (dd > bothRadius * bothRadius) {
			return;
		}
		normal = Vec2Normalize(particlePos - closest);
	} else {
		// Region AB
		float den = Vec2Dot(e, e);
		assert(den > 0.0f);
		closest = (a * u + b * v) * (1.0f / den);
		Vec2f d = particlePos - closest;
		float dd = Vec2Dot(d, d);
		if (dd > bothRadius * bothRadius) {
			return;
		}

		normal = V2f(-e.y, e.x);

		// Swap normal when on other side
		if (Vec2Dot(normal, particlePos - a) < 0.0f) {
			normal = -normal;
		}
		normal = Vec2Normalize(normal);
	}

	Vec2f deltaPos = particlePos - closest;
	float distance = Vec2Dot(normal, deltaPos);
	float penetration = bothRadius - distance;
	particlePos += normal * penetration;
	*particlePosition = particlePos;
}

static bool FindMTVCirclePolygon(const Vec2f &circlePosition, const size_t vertexCount, const Vec2f *verts, Vec2f *mtv) {
	size_t edgeIndex = 0;
	Vec2f normal = V2f(0, 0);
	float separation = -FLT_MAX;
	float radius = kSPHCollisionMargin + kSPHParticleCollisionRadius;

	for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
		Vec2f a = verts[vertexIndex];
		Vec2f b = verts[(vertexIndex + 1) % vertexCount];
		Vec2f n = Vec2Normalize(Vec2Cross(b - a, 1.0f));
		float s = Vec2Dot(n, circlePosition - a);
		if (s > radius) {
			return false;
		}

		if (s > separation) {
			normal = n;
			separation = s;
			edgeIndex = vertexIndex;
		}
	}

	Vec2f v1 = verts[edgeIndex];
	Vec2f v2 = verts[(edgeIndex + 1) % vertexCount];

	// If the center is inside the polygon
	if (separation < kSPHCollisionEpsilon) {
		float penetration = radius - separation;
		*mtv = normal * penetration;
		return true;
	}

	// Compute barycentric coordinates
	float u1 = Vec2Dot(circlePosition - v1, v2 - v1);
	float u2 = Vec2Dot(circlePosition - v2, v1 - v2);
	if (u1 <= 0.0f) {
		// Region A
		if (Vec2DistanceSquared(circlePosition, v1) > radius * radius) {
			return false;
		}

		Vec2f distanceToEdge = circlePosition - v1;
		normal = Vec2Normalize(distanceToEdge);
		float penetration = radius - Vec2Dot(normal, distanceToEdge);
		*mtv = normal * penetration;
		return true;
	} else if (u2 <= 0.0f) {
		// Region B
		if (Vec2DistanceSquared(circlePosition, v2) > radius * radius) {
			return false;
		}

		Vec2f distanceToEdge = circlePosition - v2;
		normal = Vec2Normalize(distanceToEdge);
		float penetration = radius - Vec2Dot(normal, distanceToEdge);
		*mtv = normal * penetration;
		return true;
	} else {
		// Region AB
		Vec2f faceCenter = Vec2Lerp(v1, 0.5f, v2);
		Vec2f distanceToFace = circlePosition - faceCenter;
		float s = Vec2Dot(distanceToFace, normal);
		if (s > radius) {
			return false;
		}

		float penetration = radius - s;
		*mtv = normal * penetration;
		return true;
	}

	assert(!"Invalid!");
}

force_inline void SPHSolvePolygonCollision(Vec2f *particlePosition, const size_t vertexCount, const Vec2f *verts) {
	Vec2f particlePos = *particlePosition;
	Vec2f mtv = V2f(0,0);
	if (FindMTVCirclePolygon(particlePos, vertexCount, verts, &mtv)) {
		particlePos += mtv;
		*particlePosition = particlePos;
	}
}

force_inline Vec4f SPHGetParticleColor(const float restDensity, const float density, const float pressure, const Vec2f &velocity) {
	// @TODO: This is are totally wrong, when the default parameters are different!
	float r = pressure / (-10.0f);
	float g = density / (restDensity);
	float b = Vec2Length(velocity) / 10.0f;

	Vec4f result;
	result.r = std::max(std::min(r, 1.0f), 0.0f);
	result.g = std::max(std::min(g, 1.0f), 0.0f);
	result.b = std::max(std::min(b, 1.0f), 0.0f);
	result.a = 1.0f;
	return(result);
}

#endif