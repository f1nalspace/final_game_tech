#include "demo3.h"

#ifndef DEMO3_IMPLEMENTATION
#define DEMO3_IMPLEMENTATION

#include <chrono>
#include <algorithm>
#include <cstddef>

#include "render.h"

namespace Demo3 {
	ParticleSimulation::ParticleSimulation() :
		gravity(V2f(0, 0)) {
		particles.reserve(kSPHMaxParticleCount);
		cells = new Cell[kSPHGridTotalCount];
		_isMultiThreading = workerPool.GetThreadCount() > 1;
	}

	ParticleSimulation::~ParticleSimulation() {
		for (size_t bodyIndex = 0; bodyIndex < bodies.size(); ++bodyIndex) {
			delete bodies[bodyIndex];
		}
		delete cells;
	}

	void ParticleSimulation::InsertParticleIntoGrid(Particle &particle, const size_t particleIndex) {
		Vec2f position = particle.curPosition;
		Vec2i cellIndex = SPHComputeCellIndex(position);

		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);
		Cell *cell = &cells[cellOffset];
		assert(cell != nullptr);

		cell->indices.push_back(particleIndex);
		particle.cellIndex = cellIndex;

		size_t count = cell->indices.size();
		stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
		stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
	}

	void ParticleSimulation::RemoveParticleFromGrid(Particle & particle, const size_t particleIndex) {
		Vec2i cellIndex = particle.cellIndex;
		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);

		Cell *cell = &cells[cellOffset];
		assert(cell != nullptr);

		auto result = std::find(cell->indices.begin(), cell->indices.end(), particleIndex);
		assert(result != cell->indices.end());
		cell->indices.erase(result);

		size_t count = cell->indices.size();
		stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
		stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
	}

	void ParticleSimulation::AddPlane(const Vec2f & normal, const float distance) {
		Plane *plane = new Plane(normal, distance);
		bodies.push_back(plane);
	}

	void ParticleSimulation::AddCircle(const Vec2f & pos, const float radius) {
		bodies.push_back(new Circle(pos, radius));
	}

	void ParticleSimulation::AddLineSegment(const Vec2f & a, const Vec2f & b) {
		bodies.push_back(new LineSegment(a, b));
	}

	void ParticleSimulation::AddPolygon(const size_t vertexCount, const Vec2f *verts) {
		std::vector<Vec2f> polyVerts;
		for (size_t i = 0; i < vertexCount; ++i) {
			polyVerts.push_back(verts[i]);
		}
		bodies.push_back(new Poly(polyVerts));
	}

	void ParticleSimulation::ClearBodies() {
		for (int bodyIndex = 0; bodyIndex < bodies.size(); ++bodyIndex) {
			Body *body = bodies[bodyIndex];
			delete body;
		}
		bodies.clear();
	}

	void ParticleSimulation::ClearParticles() {
		for (size_t cellIndex = 0; cellIndex < kSPHGridTotalCount; ++cellIndex) {
			Cell *cell = &cells[cellIndex];
			assert(cell != nullptr);
			cell->indices.clear();
		}
		particles.clear();
	}

	void ParticleSimulation::ClearEmitters() {
		emitters.clear();
	}

	void ParticleSimulation::ResetStats() {
		stats = {};
	}

	size_t ParticleSimulation::AddParticle(const Vec2f &position, const Vec2f &force) {
		size_t particleIndex = particles.size();
		particles.push_back(Particle(position));
		Particle &particle = particles[particleIndex];
		particle.acceleration = force;
		InsertParticleIntoGrid(particle, particleIndex);
		return particleIndex;
	}

	void ParticleSimulation::AddVolume(const Vec2f &center, const Vec2f &force, const int countX, const int countY, const float spacing) {
		Vec2f offset = V2f(countX * spacing, countY * spacing) * 0.5f;
		for (int yIndex = 0; yIndex < countY; ++yIndex) {
			for (int xIndex = 0; xIndex < countX; ++xIndex) {
				Vec2f p = V2f((float)xIndex, (float)yIndex) * spacing;
				p += V2f(spacing * 0.5f, spacing * 0.5f);
				p += center - offset;
				Vec2f jitter = Vec2RandomDirection() * kSPHKernelHeight * kSPHVolumeParticleDistributionScale;
				p += jitter;
				AddParticle(p, force);
			}
		}
	}

	void ParticleSimulation::AddEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration) {
		ParticleEmitter emitter = ParticleEmitter(position, direction, radius, speed, rate, duration);
		emitters.push_back(emitter);
	}

	void ParticleSimulation::NeighborSearch(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndexA = startIndex; particleIndexA <= endIndex; ++particleIndexA) {
			Particle &particleA = particles[particleIndexA];
			particleA.neighbors.clear();
			Vec2i &cellIndex = particleA.cellIndex;
			for (int y = -1; y <= 1; ++y) {
				for (int x = -1; x <= 1; ++x) {
					int cellPosX = cellIndex.x + x;
					int cellPosY = cellIndex.y + y;
					if (SPHIsPositionInGrid(cellPosX, cellPosY)) {
						size_t cellOffset = SPHComputeCellOffset(cellPosX, cellPosY);
						Cell *cell = &cells[cellOffset];
						size_t particleCountInCell = cell->indices.size();
						if (particleCountInCell > 0) {
							for (size_t index = 0; index < particleCountInCell; ++index) {
								size_t particleIndexB = cell->indices[index];
								particleA.neighbors.push_back(particleIndexB);
							}
						}
					}
				}
			}
		}
	}

	void ParticleSimulation::DensityAndPressure(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle &particle = particles[particleIndex];
			particle.density = particle.nearDensity = 0;
			size_t neighborCount = particle.neighbors.size();
			for (size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particle.neighbors[index];
				Particle &neighbor = particles[neighborIndex];
				SPHComputeDensity(params, particle.curPosition, neighbor.curPosition, &particle.density);
			}
			SPHComputePressure(params, &particle.density, &particle.pressure);
		}
	}

	void ParticleSimulation::ViscosityForces(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle &particle = particles[particleIndex];
			size_t neighborCount = particle.neighbors.size();
			for (size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particle.neighbors[index];
				Particle &neighbor = particles[neighborIndex];
				Vec2f force = V2f(0,0);
				SPHComputeViscosityForce(params, particle.curPosition, neighbor.curPosition, particle.velocity, neighbor.velocity, &force);
				particle.velocity -= force * deltaTime * 0.5f;
				neighbor.velocity += force * deltaTime * 0.5f;
			}
		}
	}

	void ParticleSimulation::DeltaPositions(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle &particle = particles[particleIndex];
			Vec2f accumulatedDelta = V2f(0,0);
			size_t neighborCount = particle.neighbors.size();
			for (size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particle.neighbors[index];
				Particle &neighbor = particles[neighborIndex];
				Vec2f delta = V2f(0,0);
				SPHComputeDelta(params, particle.curPosition, neighbor.curPosition, &particle.pressure, deltaTime, &delta);
				neighbor.curPosition += delta * 0.5f;
				accumulatedDelta -= delta * 0.5f;
			}
			particle.curPosition += accumulatedDelta;
		}
	}

	void ParticleSimulation::UpdateEmitter(ParticleEmitter *emitter, const float deltaTime) {
		const float spacing = params.particleSpacing;
		const float invDeltaTime = 1.0f / deltaTime;
		if (emitter->isActive) {
			const float rate = 1.0f / emitter->rate;
			emitter->elapsed += deltaTime;
			emitter->totalElapsed += deltaTime;
			if (emitter->elapsed >= rate) {
				emitter->elapsed = 0;
				Vec2f acceleration = emitter->direction * emitter->speed * invDeltaTime;
				Vec2f dir = Vec2Cross(1.0f, emitter->direction);
				int count = (int)floor(emitter->radius / spacing);
				float halfSize = (float)count * spacing * 0.5f;
				Vec2f offset = dir * (float)count * spacing * 0.5f;
				for (int index = 0; index < count; ++index) {
					Vec2f p = dir * (float)index * spacing;
					p += dir * spacing * 0.5f;
					p += emitter->position - offset;
					Vec2f jitter = Vec2RandomDirection() * kSPHKernelHeight * kSPHVolumeParticleDistributionScale;
					p += jitter;
					AddParticle(p, acceleration);
				}
			}
			if (emitter->totalElapsed >= emitter->duration) {
				emitter->isActive = false;
			}
		}
	}

	void ParticleSimulation::Update(const float deltaTime) {
		const float invDt = 1.0f / deltaTime;
		const bool useMultiThreading = _isMultiThreading;

		// Emitters
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t emitterIndex = 0; emitterIndex < emitters.size(); ++emitterIndex) {
				ParticleEmitter *emitter = &emitters[emitterIndex];
				UpdateEmitter(emitter, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.emitters = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Integrate forces
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle &particle = particles[particleIndex];
				particle.acceleration += gravity + externalForce;
				particle.velocity += particle.acceleration * deltaTime;
				particle.acceleration = V2f(0,0);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.integration = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Viscosity force
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				workerPool.CreateTasks(particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->ViscosityForces(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->ViscosityForces(0, particles.size() - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.viscosityForces = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Predict
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle &particle = particles[particleIndex];
				particle.prevPosition = particle.curPosition;
				particle.curPosition += particle.velocity * deltaTime;
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.predict = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Update grid
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle &particle = particles[particleIndex];
				Vec2i newCellIndex = SPHComputeCellIndex(particle.curPosition);
				Vec2i oldCellIndex = particle.cellIndex;
				if (newCellIndex.x != oldCellIndex.x || newCellIndex.y != oldCellIndex.y) {
					RemoveParticleFromGrid(particle, particleIndex);
					InsertParticleIntoGrid(particle, particleIndex);
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.updateGrid = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Neighbor search
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				workerPool.CreateTasks(particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->NeighborSearch(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->NeighborSearch(0, particles.size() - 1, deltaTime);
			}
			stats.minParticleNeighborCount = kSPHMaxParticleNeighborCount;
			stats.maxParticleNeighborCount = 0;
			for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle &particle = particles[particleIndex];
				size_t neighborCount = particle.neighbors.size();
				stats.minParticleNeighborCount = std::min(neighborCount, stats.minParticleNeighborCount);
				stats.maxParticleNeighborCount = std::max(neighborCount, stats.maxParticleNeighborCount);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.neighborSearch = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Density and pressure
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				workerPool.CreateTasks(particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DensityAndPressure(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->DensityAndPressure(0, particles.size(), deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.densityAndPressure = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Calculate delta position
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				workerPool.CreateTasks(particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DeltaPositions(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->DeltaPositions(0, particles.size() - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.deltaPositions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Solve collisions
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle &particle = particles[particleIndex];
				for (size_t bodyIndex = 0; bodyIndex < bodies.size(); ++bodyIndex) {
					Body *body = bodies[bodyIndex];
					switch (body->type) {
						case BodyType::BodyType_Plane:
						{
							Plane *plane = static_cast<Plane *>(body);
							SPHSolvePlaneCollision(&particle.curPosition, plane->normal, plane->distance);
						} break;
						case BodyType::BodyType_Circle:
						{
							Circle *circle = static_cast<Circle *>(body);
							SPHSolveCircleCollision(&particle.curPosition, circle->pos, circle->radius);
						} break;
						case BodyType::BodyType_LineSegment:
						{
							LineSegment *lineSegment = static_cast<LineSegment *>(body);
							SPHSolveLineSegmentCollision(&particle.curPosition, lineSegment->a, lineSegment->b);
						} break;
						case BodyType::BodyType_Polygon:
						{
							Poly *polygon = static_cast<Poly *>(body);
							SPHSolvePolygonCollision(&particle.curPosition, polygon->verts.size(), &polygon->verts[0]);
						} break;
					}
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.collisions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Recalculate velocity for next frame
		for (size_t particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
			Particle &particle = particles[particleIndex];
			particle.velocity = (particle.curPosition - particle.prevPosition) * invDt;
		}
	}

	void ParticleSimulation::Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale) {
		// Domain
		Vec4f domainColor = V4f(1.0f, 0.0f, 1.0f, 1.0f);
		Render::PushRectangle(commandBuffer, V2f(-kSPHBoundaryHalfWidth, -kSPHBoundaryHalfHeight), V2f(kSPHBoundaryHalfWidth, kSPHBoundaryHalfHeight) * 2.0f, domainColor, false, 1.0f);

		// Grid fill
		for (int yIndexInner = 0; yIndexInner < kSPHGridCountY; ++yIndexInner) {
			for (int xIndexInner = 0; xIndexInner < kSPHGridCountX; ++xIndexInner) {
				size_t cellOffset = SPHComputeCellOffset(xIndexInner, yIndexInner);
				Cell *cell = &cells[cellOffset];
				Vec2f innerP = kSPHGridOrigin + V2f((float)xIndexInner, (float)yIndexInner) * kSPHGridCellSize;
				Vec2f innerSize = V2f(kSPHGridCellSize, kSPHGridCellSize);
				if (cell->indices.size() > 0) {
					Render::PushRectangle(commandBuffer, innerP, innerSize, ColorLightGray, true);
				}
			}
		}

		// Grid lines
		for (int yIndex = 0; yIndex < kSPHGridCountY; ++yIndex) {
			Vec2f startP = kSPHGridOrigin + V2f(0, (float)yIndex) * kSPHGridCellSize;
			Vec2f endP = kSPHGridOrigin + V2f((float)kSPHGridCountX, (float)yIndex) * kSPHGridCellSize;
			Render::PushLine(commandBuffer, startP, endP, ColorDarkGray, 1.0f);
		}
		for (int xIndex = 0; xIndex < kSPHGridCountX; ++xIndex) {
			Vec2f startP = kSPHGridOrigin + V2f((float)xIndex, 0) * kSPHGridCellSize;
			Vec2f endP = kSPHGridOrigin + V2f((float)xIndex, (float)kSPHGridCountY) * kSPHGridCellSize;
			Render::PushLine(commandBuffer, startP, endP, ColorDarkGray, 1.0f);
		}

		// Bodies
		for (int bodyIndex = 0; bodyIndex < bodies.size(); ++bodyIndex) {
			Body *body = bodies[bodyIndex];
			switch (body->type) {
				case BodyType::BodyType_Plane:
				{
					Plane *plane = static_cast<Plane *>(body);
					plane->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_Circle:
				{
					Circle *circle = static_cast<Circle *>(body);
					circle->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_LineSegment:
				{
					LineSegment *lineSegment = static_cast<LineSegment *>(body);
					lineSegment->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_Polygon:
				{
					Poly *polygon = static_cast<Poly *>(body);
					polygon->Render(commandBuffer);
				}
				break;
			}
		}

		// Particles
		if (particles.size() > 0) {
			for (int particleIndex = 0; particleIndex < particles.size(); ++particleIndex) {
				Particle *particle = &particles[particleIndex];
				particle->color = SPHGetParticleColor(params.restDensity, particle->density, particle->pressure, particle->velocity);
			}
			float pointSize = kSPHParticleRenderRadius * 2.0f * worldToScreenScale;
			void *vertices = (void *)((uint8_t *)&particles[0] + offsetof(Particle, curPosition));
			void *colors = (void *)((uint8_t *)&particles[0] + offsetof(Particle, color));
			uint32_t particleCount = (uint32_t)particles.size();
			uint32_t vertexStride = sizeof(Particle);
			Render::PushVertexIndexArrayHeader(commandBuffer, vertexStride, vertices, 0, nullptr, vertexStride, colors, 0, nullptr);
			Render::PushVertexIndexArrayDraw(commandBuffer, Render::PrimitiveType::Points, particleCount, pointSize, nullptr, {}, false);
		}
	}

	Particle::Particle(const Vec2f & position) :
		prevPosition(position),
		curPosition(position) {
		neighbors.reserve(kSPHMaxParticleNeighborCount);
	}

	Cell::Cell() {
		indices.reserve(kSPHMaxCellParticleCount);
	}

	void Plane::Render(Render::CommandBuffer *commandBuffer) {
		Vec2f p = normal * distance;
		Vec2f t = V2f(normal.y, -normal.x);
		Vec4f color = ColorBlue;
		Vec2f a = V2f(p.x + t.x * kSPHVisualPlaneLength, p.y + t.y * kSPHVisualPlaneLength);
		Vec2f b = V2f(p.x - t.x * kSPHVisualPlaneLength, p.y - t.y * kSPHVisualPlaneLength);
		Render::PushLine(commandBuffer, a, b, color, 1.0f);
	}

	void Circle::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushCircle(commandBuffer, pos, radius, color, 1.0f, false);
	}

	void LineSegment::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushLine(commandBuffer, a, b, color, 1.0f);
	}

	void Poly::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushPolygonFrom(commandBuffer, &verts[0], verts.size(), color, false, 1.0f);
	}
}

#endif // DEMO3_IMPLEMENTATION