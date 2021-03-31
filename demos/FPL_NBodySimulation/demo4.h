/* Demo 4 - Data oriented style with 8/16 byte aligned structures */

#ifndef DEMO4_H
#define DEMO4_H

#include <assert.h>
#include <random>

#include "vecmath.h"
#include "sph.h"
#include "threading.h"
#include "base.h"
#include "render.h"

namespace Demo4 {
	const char *kDemoName = "Demo 4";

	enum BodyType {
		BodyType_None = 0,

		BodyType_Plane = 1,
		BodyType_Circle = 2,
		BodyType_LineSegment = 3,
		BodyType_Polygon = 4,

		BodyType_Count,
	};

	struct Plane {
		Vec2f normal;
		float distance;
		uint8_t padding0[4];

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Circle {
		Vec2f pos;
		float radius;
		uint8_t padding0[4];

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct LineSegment {
		Vec2f a, b;

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Poly {
		Vec2f verts[kMaxScenarioPolygonCount];
		size_t vertexCount;

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct Body {
		BodyType type;
		uint8_t padding0[4];

		union {
			Plane plane;
			Circle circle;
			LineSegment lineSegment;
			Poly polygon;
		};

		Body() {
		}

		Body(const Body &body) {
			type = body.type;
			plane = body.plane;
			circle = body.circle;
			lineSegment = body.lineSegment;
			polygon = body.polygon;
		}
	};

	struct ParticleData {
		Vec2f curPosition;
		Vec2f prevPosition;
		Vec2f acceleration;
		Vec2f velocity;
		union {
			struct {
				float density;
				float nearDensity;
			};
			float densities[2];
		};
		union {
			struct {
				float pressure;
				float nearPressure;
			};
			float pressures[2];
		};

		inline ParticleData(const Vec2f &pos) {
			prevPosition = curPosition = pos;
			acceleration = velocity = Vec2f(0,0);
			density = nearDensity = 0;
			pressure = nearPressure = 0;
		}
		inline ParticleData() : ParticleData(Vec2f(0,0)) {
		}
	};

	struct ParticleIndex {
		Vec2i cellIndex;
		size_t neighbors[kSPHMaxParticleNeighborCount];
		size_t neighborCount;
		size_t indexInCell;
	};

	struct Cell {
		size_t indices[kSPHMaxCellParticleCount];
		size_t count;
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

		void Render(Render::CommandBuffer *commandBuffer);
	};

	struct ParticleSimulation : BaseSimulation {
		SPHParameters params;
		SPHStatistics stats;

		Vec2f gravity;
		Vec2f externalForce;

		size_t particleCount;
		ParticleData *particleDatas;
		ParticleIndex *particleIndexes;
		Vec4f *particleColors;

		size_t bodyCount;
		Body *bodies;

		size_t emitterCount;
		ParticleEmitter *emitters;

		Cell *cells;

		bool isMultiThreading;
		ThreadPool workerPool;

		inline void InsertParticleIntoGrid(const size_t particleIndex);
		inline void RemoveParticleFromGrid(const size_t particleIndex);

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

		void UpdateEmitter(ParticleEmitter *emitter, float deltaTime);
		void ViscosityForces(const int64_t startIndex, const int64_t endIndex, const float deltaTime);
		void NeighborSearch(const int64_t startIndex, const int64_t endIndex, const float deltaTime);
		void DensityAndPressure(const int64_t startIndex, const int64_t endIndex, const float deltaTime);
		void DeltaPositions(const int64_t startIndex, const int64_t endIndex, const float deltaTime);

		void Update(const float deltaTime);
		void Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale);

		inline void AddExternalForces(const Vec2f &force) {
			externalForce += force;
		}
		inline void ClearExternalForce() {
			externalForce = Vec2f(0, 0);
		}

		inline size_t GetParticleCount() {
			return particleCount;
		}

		inline void SetMultiThreading(const bool value) {
			isMultiThreading = value;
		}
		inline bool IsMultiThreadingSupported() {
			return true;
		}
		inline bool IsMultiThreading() {
			return isMultiThreading;
		}
		inline size_t GetWorkerThreadCount() {
			return workerPool.GetThreadCount();
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
	};
};

#endif // DEMO4_H