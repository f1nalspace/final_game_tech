#ifndef SIMULATION_H
#define SIMULATION_H

#include "vecmath.h"

#define force_inline __forceinline

const float kSPHBoundaryAspect = 16.0f / 9.0f;
const float kSPHBoundaryWidth = 10.0f;
const float kSPHBoundaryHeight = kSPHBoundaryWidth / kSPHBoundaryAspect;
const float kSPHHalfBoundaryWidth = kSPHBoundaryWidth * 0.5f;
const float kSPHHalfBoundaryHeight = kSPHBoundaryHeight * 0.5f;
const int kSPHGridCountX = 64;
const int kSPHGridCountY = (int)(kSPHGridCountX / kSPHBoundaryAspect);
const int kSPHGridTotalCount = kSPHGridCountX * kSPHGridCountY;
const float kSPHGridCellSize = kSPHBoundaryWidth / (float)kSPHGridCountX;
const float kSPHParticleRadius = kSPHGridCellSize;
const static Vec2f kSPHGridOrigin = Vec2f(-kSPHHalfBoundaryWidth, -kSPHHalfBoundaryHeight);

const float kDeltaTime = 1.0f / 60.0f;

const float kSPHSmoothingLength = kSPHParticleRadius * 1.0f;
const float kSPHRestDensity = 1000.0f;
const float kSPHStiffness = 0.1f;
const float kSPHNearStiffness = 0.15f;
const float kSPHLinearViscosity = 1.0f;
const float kSPHQuadraticViscosity = 0.5f;

const float kDropCountScale = 0.5f;
const float kDropDistanceScale = 0.95f;

force_inline size_t SPHComputeCellOffset(const int x, const int y) {
	size_t result = y * kSPHGridCountY + x;
	return(result);
}

force_inline Vec2i SPHComputeCellPos(const Vec2f &p, const Vec2f &center, const float cellSize) {
	int x = (int)((p.x + center.x) / cellSize);
	int y = (int)((p.y + center.y) / cellSize);
	return Vec2i(x, y);
}

force_inline Vec2i SPHComputeCellIndex(const Vec2f & p) {
	Vec2i cellPos = SPHComputeCellPos(p, Vec2f(kSPHHalfBoundaryWidth, kSPHHalfBoundaryHeight), kSPHGridCellSize);
	cellPos.x = Min(Max(cellPos.x, 0), (int)kSPHGridCountX - 1);
	cellPos.y = Min(Max(cellPos.y, 0), (int)kSPHGridCountY - 1);
	Vec2i result = Vec2i(cellPos.x, cellPos.y);
	return (result);
}

force_inline void SPHComputeDensity(const Vec2f &position, const Vec2f &neighborPosition, float outDensity[2]) {
	Vec2f Rij = position - neighborPosition;
	float rijSquared = Vec2Dot(Rij, Rij);

	// TODO: Make it branch-free
	if (rijSquared < kSPHSmoothingLength * kSPHSmoothingLength) {
		float rij = sqrtf(rijSquared);
		float q = rij / kSPHSmoothingLength;
		float oneminusq = 1 - q;
		outDensity[0] += (oneminusq * oneminusq);
		outDensity[1] += (oneminusq * oneminusq * oneminusq);
	}
}

force_inline void SPHComputePressure(const float density[2], float outPressure[2]) {
	outPressure[0] = kSPHStiffness * (density[0] - kSPHRestDensity);
	outPressure[1] = kSPHNearStiffness * density[1];
}

force_inline bool SPHComputeDelta(const Vec2f &position, const Vec2f &neighborPosition, const float pressure[2], const float deltaTime, Vec2f *outDelta) {
	// TODO: Make it branch-free

	Vec2f Rij = position - neighborPosition;
	float rijSquared = Vec2Dot(Rij, Rij);
	if (rijSquared < kSPHSmoothingLength * kSPHSmoothingLength) {
		float rij = sqrtf(rijSquared);
		float q = rij / kSPHSmoothingLength;
		Vec2f n = Vec2Normalize(Rij);
		float oneminusq = 1 - q;
		*outDelta = Vec2Hadamard(0.5f * deltaTime * deltaTime * (pressure[0] * oneminusq + pressure[1] * (oneminusq * oneminusq)), n);
		return true;
	}
	return false;
}

force_inline bool SPHComputeViscosityVelocity(const Vec2f &position, const Vec2f &neighborPosition, const Vec2f &velocity, const Vec2f &neighborVelocity, const float deltaTime, Vec2f *outVel) {
	// TODO: Make it branch-free

	Vec2f Rij = position - neighborPosition;
	float rijSquared = Vec2Dot(Rij, Rij);
	if (rijSquared < kSPHSmoothingLength * kSPHSmoothingLength) {
		float rij = sqrtf(rijSquared);
		float q = rij / kSPHSmoothingLength;
		Vec2f n = Vec2Normalize(Rij);
		float u = Vec2Dot(velocity - neighborVelocity, n);
		if (u > 0) {
			*outVel = Vec2Hadamard(0.5f * deltaTime * (1 - q) * (kSPHLinearViscosity * u + kSPHQuadraticViscosity * u * u), n);
			return true;
		}
	}
	return false;
}

#endif