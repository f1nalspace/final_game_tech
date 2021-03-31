/* Demo 3 - Object oriented style 3 (Structs only, no virtual function calls, reserved vectors, fixed grid) */

#ifndef DEMO3_H
#define DEMO3_H

#include <assert.h>
#include <vector>
#include <random>

#include "vecmath.h"
#include "sph.h"
#include "threading.h"
#include "base.h"
#include "render.h"

namespace Demo3 {
	const char *kDemoName = "Demo 3";

	struct Particle {
		Vec2f acceleration;
		Vec2f velocity;
		Vec2f prevPosition;
		Vec2f curPosition;
		Vec2i cellIndex;
		Vec4f color;
		float density;
		float nearDensity;
		float pressure;
		float nearPressure;
		std::vector<size_t> neighbors;

		Particle(const Vec2f &position);
	};

	enum BodyType {
		BodyType_None = 0,

		BodyType_Plane = 1,
		BodyType_Circle = 2,
		BodyType_LineSegment = 3,
		BodyType_Polygon = 4,

		BodyType_Count,
	};

	struct Body {
	public:
		BodyType type;

		inline Body(BodyType type) {
			this->type = type;
		}
	};

	struct Plane : Body {
		Vec2f normal;
		float distance;

		inline Plane(const Vec2f &normal, const float distance) :
			Body(BodyType::BodyType_Plane) {
			this->normal = normal;
			this->distance = distance;
		}

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Circle : public Body {
		Vec2f pos;
		float radius;

		inline Circle(const Vec2f &pos, const float radius) :
			Body(BodyType::BodyType_Circle) {
			this->pos = pos;
			this->radius = radius;
		}

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct LineSegment : public Body {
		Vec2f a, b;

		inline LineSegment(const Vec2f &a, const Vec2f &b) :
			Body(BodyType::BodyType_LineSegment) {
			this->a = a;
			this->b = b;
		}

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Poly : public Body {
		std::vector<Vec2f> verts;

		inline Poly(const std::vector<Vec2f> &verts) :
			Body(BodyType::BodyType_Polygon) {
			this->verts = verts;
		}

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Cell {
		std::vector<size_t> indices;

		Cell();
	};

	struct ParticleEmitter {
		Vec2f position;
		Vec2f direction;
		float radius;
		float speed;
		float rate;
		float duration;
		float elapsed;
		float totalElapsed;
		int32_t isActive;

		ParticleEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration) {
			this->position = position;
			this->direction = direction;
			this->radius = radius;
			this->speed = speed;
			this->rate = rate;
			this->duration = duration;
			this->elapsed = 0;
			this->totalElapsed = 0;
			this->isActive = true;
		}

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct ParticleSimulation : BaseSimulation {
		SPHParameters params;
		SPHStatistics stats;

		Vec2f gravity;
		Vec2f externalForce;

		std::vector<Particle> particles;

		std::vector<Body *> bodies;

		std::vector<ParticleEmitter> emitters;

		Cell *cells;

		bool _isMultiThreading;
		ThreadPool workerPool;

		inline void InsertParticleIntoGrid(Particle &particle, const size_t particleIndex);
		inline void RemoveParticleFromGrid(Particle &particle, const size_t particleIndex);

		ParticleSimulation();
		~ParticleSimulation();

		void ResetStats();
		void ClearBodies();
		void ClearParticles();
		void ClearEmitters();

		void AddPlane(const Vec2f &normal, const float distance);
		void AddCircle(const Vec2f &pos, const float radius);
		void AddLineSegment(const Vec2f &a, const Vec2f &b);
		void AddPolygon(const size_t vertexCount, const Vec2f *verts);

		size_t AddParticle(const Vec2f &position, const Vec2f &force);
		void AddVolume(const Vec2f &center, const Vec2f &force, const int countX, const int countY, const float spacing);
		void AddEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration);

		void UpdateEmitter(ParticleEmitter *emitter, const float deltaTime);
		void ViscosityForces(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void NeighborSearch(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void DensityAndPressure(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void DeltaPositions(const size_t startIndex, const size_t endIndex, const float deltaTime);

		void Update(const float deltaTime);
		void Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale);

		inline void AddExternalForces(const Vec2f &force) {
			externalForce += force;
		}
		inline void ClearExternalForce() {
			externalForce = V2f(0,0);
		}

		inline void SetGravity(const Vec2f &gravity) {
			this->gravity = gravity;
		}

		inline const SPHParameters &GetParams() {
			return params;
		}
		inline SPHStatistics &GetStats() {
			return stats;
		}
		inline void SetParams(const SPHParameters &params) {
			this->params = params;
		}

		inline size_t GetParticleCount() {
			return particles.size();
		}

		inline void SetMultiThreading(const bool value) {
			_isMultiThreading = value;
		}
		inline bool IsMultiThreadingSupported() {
			return true;
		}
		inline bool IsMultiThreading() {
			return _isMultiThreading;
		}
		inline size_t GetWorkerThreadCount() {
			return workerPool.GetThreadCount();
		}
	};
};

#endif // DEMO3_H