/* Demo 2 - Object oriented style 2 (Public, reserved vectors, fixed grid, more sane) */

#ifndef DEMO2_H
#define DEMO2_H

#include <assert.h>
#include <vector>
#include <random>

#include "vecmath.h"
#include "sph.h"
#include "threading.h"
#include "base.h"
#include "render.h"

namespace Demo2 {
	const char *kDemoName = "Demo 2";

	class Particle {
	public:
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

	class Body {
	public:
		BodyType type;

		inline Body(BodyType type) {
			this->type = type;
		}

		virtual ~Body() {
		}

		virtual void Render(Render::CommandBuffer *commandBuffer) {
		}

		virtual void SolveCollision(Particle &particle) {
		}
	};

	class Plane : public Body {
	public:
		Vec2f normal;
		float distance;

		inline Plane(const Vec2f &normal, const float distance) :
			Body(BodyType::BodyType_Plane) {
			this->normal = normal;
			this->distance = distance;
		}

		void Render(Render::CommandBuffer *commandBuffer);

		void SolveCollision(Particle &particle);
	};

	class Circle : public Body {
	public:
		Vec2f pos;
		float radius;

		inline Circle(const Vec2f &pos, const float radius) :
			Body(BodyType::BodyType_Circle) {
			this->pos = pos;
			this->radius = radius;
		}

		void SolveCollision(Particle &particle);

		void Render(Render::CommandBuffer *commandBuffer);
	};

	class LineSegment : public Body {
	public:
		Vec2f a, b;

		inline LineSegment(const Vec2f &a, const Vec2f &b) :
			Body(BodyType::BodyType_LineSegment) {
			this->a = a;
			this->b = b;
		}

		void SolveCollision(Particle &particle);

		void Render(Render::CommandBuffer *commandBuffer);
	};

	class Poly : public Body {
	public:
		std::vector<Vec2f> verts;

		Poly(const std::vector<Vec2f> &verts) :
			Body(BodyType::BodyType_Polygon) {
			this->verts = verts;
		}

		void Render(Render::CommandBuffer *commandBuffer);

		void SolveCollision(Particle &particle);
	};

	class Cell {
	public:
		std::vector<size_t> indices;

		Cell();
	};

	class ParticleEmitter {
	public:
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

	class ParticleSimulation : public BaseSimulation {
	private:
		SPHParameters _params;
		SPHStatistics _stats;

		Vec2f _gravity;
		Vec2f _externalForce;

		std::vector<Particle> _particles;

		std::vector<Body *> _bodies;

		std::vector<ParticleEmitter> _emitters;

		std::vector<Cell> _cells;

		bool _isMultiThreading;
		ThreadPool _workerPool;

		inline void InsertParticleIntoGrid(Particle &particle, const size_t particleIndex);
		inline void RemoveParticleFromGrid(Particle &particle, const size_t particleIndex);

		void UpdateEmitter(ParticleEmitter *emitter, const float deltaTime);
		void ViscosityForces(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void NeighborSearch(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void DensityAndPressure(const size_t startIndex, const size_t endIndex, const float deltaTime);
		void DeltaPositions(const size_t startIndex, const size_t endIndex, const float deltaTime);
	public:
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

		void Update(const float deltaTime);
		void Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale);

		void AddExternalForces(const Vec2f &force);
		void ClearExternalForce();

		void SetGravity(const Vec2f &gravity) {
			this->_gravity = gravity;
		}
		const SPHParameters &GetParams() {
			return _params;
		}
		SPHStatistics &GetStats() {
			return _stats;
		}
		void SetParams(const SPHParameters &params) {
			_params = params;
		}
		size_t GetParticleCount() {
			return _particles.size();
		}
		void SetMultiThreading(const bool value) {
			_isMultiThreading = value;
		}
		bool IsMultiThreadingSupported() {
			return true;
		}
		bool IsMultiThreading() {
			return _isMultiThreading;
		}
		size_t GetWorkerThreadCount() {
			return _workerPool.GetThreadCount();
		}
	};
};

#endif // DEMO2_H