#ifndef BASE_H
#define BASE_H

#include "vecmath.h"
#include "sph.h"
#include "render.h"

class BaseSimulation {
public:
	virtual void ResetStats() = 0;
	virtual void ClearBodies() = 0;
	virtual void ClearParticles() = 0;
	virtual void ClearEmitters() = 0;

	virtual void AddPlane(const Vec2f &normal, const float distance) = 0;
	virtual void AddCircle(const Vec2f &pos, const float radius) = 0;
	virtual void AddLineSegment(const Vec2f &a, const Vec2f &b) = 0;
	virtual void AddPolygon(const size_t vertexCount, const Vec2f *verts) = 0;

	virtual size_t AddParticle(const Vec2f &position, const Vec2f &force) = 0;
	virtual void AddVolume(const Vec2f &center, const Vec2f &force, const int countX, const int countY, const float spacing) = 0;
	virtual void AddEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration) = 0;

	virtual void Update(const float deltaTime) = 0;
	virtual void Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale) = 0;

	virtual void AddExternalForces(const Vec2f &force) = 0;
	virtual void ClearExternalForce() = 0;

	virtual size_t GetParticleCount() = 0;
	virtual void SetGravity(const Vec2f &gravity) = 0;
	virtual const SPHParameters &GetParams() = 0;
	virtual SPHStatistics &GetStats() = 0;
	virtual void SetParams(const SPHParameters &params) = 0;
	virtual void SetMultiThreading(const bool value) = 0;
	virtual bool IsMultiThreadingSupported() = 0;
	virtual bool IsMultiThreading() = 0;
	virtual size_t GetWorkerThreadCount() = 0;
};

#endif
