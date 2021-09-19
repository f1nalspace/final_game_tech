#include "demo4.h"

#ifndef DEMO4_IMPLEMENTATION
#define DEMO4_IMPLEMENTATION

#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>

#include "render.h"

namespace Demo4 {
#if PARTICLE_VALIDATION
	static void ValidateParticles(const ParticleData *particles, const ParticleIndex *cellIndices, const size_t count) {
		for(size_t particleIndex = 0; particleIndex < count; ++particleIndex) {
			const ParticleData &particle = particles[particleIndex];
			const ParticleIndex &index = cellIndices[particleIndex];

			bool isInGrid = SPHIsPositionInGrid(index.cellIndex.x, index.cellIndex.y);
			fplAssert(isInGrid);

			float l = Vec2Length(particle.velocity);
			fplAssert(l < 1000);

			Vec2f cur = particle.curPosition;

			float apron = 1.0f;
			float halfW = kSPHBoundaryHalfWidth + apron;
			float halfH = kSPHBoundaryHalfHeight + apron;

			fplAssert(cur.x > -halfW && cur.x < halfW);
			fplAssert(cur.y > -halfH && cur.y < halfH);
		}
	}
#else
#	ifndef ValidateParticles
#		define ValidateParticles(...)
#	endif
#endif

	ParticleSimulation::ParticleSimulation():
		gravity(V2f(0, 0)),
		externalForce(V2f(0, 0)),
		particleCount(0),
		bodyCount(0),
		emitterCount(0) {
		cells = new Cell[kSPHGridTotalCount];
		particleDatas = new ParticleData[kSPHMaxParticleCount];
		particleIndexes = new ParticleIndex[kSPHMaxParticleCount];
		particleColors = new Vec4f[kSPHMaxParticleCount];
		bodies = new Body[kSPHMaxBodyCount];
		emitters = new ParticleEmitter[kSPHMaxEmitterCount];
		isMultiThreading = workerPool.GetThreadCount() > 1;
	}

	ParticleSimulation::~ParticleSimulation() {
		delete emitters;
		delete bodies;
		delete particleColors;
		delete particleIndexes;
		delete particleDatas;
		delete cells;
	}

	void ParticleSimulation::InsertParticleIntoGrid(const size_t particleIndex) {
		Vec2f position = particleDatas[particleIndex].curPosition;
		Vec2i cellIndex = SPHComputeCellIndex(position);

		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);
		Cell *cell = &cells[cellOffset];
		assert(cell != nullptr);

		assert(cell->count < kSPHMaxCellParticleCount);
		size_t indexInCell = cell->count++;
		cell->indices[indexInCell] = particleIndex;
		particleIndexes[particleIndex].cellIndex = cellIndex;
		particleIndexes[particleIndex].indexInCell = indexInCell;

		size_t count = cell->count;
		stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
		stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
	}

	void ParticleSimulation::RemoveParticleFromGrid(const size_t particleIndex) {
		Vec2i cellIndex = particleIndexes[particleIndex].cellIndex;
		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);

		Cell *cell = &cells[cellOffset];
		assert(cell != nullptr);

		assert(particleIndexes[particleIndex].indexInCell < cell->count);
		size_t removalIndex = particleIndexes[particleIndex].indexInCell;
		size_t lastIndex = cell->count - 1;
		if(removalIndex != lastIndex) {
			cell->indices[removalIndex] = cell->indices[lastIndex];
			cell->indices[lastIndex] = particleIndex;
			particleIndexes[cell->indices[removalIndex]].indexInCell = removalIndex;
		}
		--cell->count;

		size_t count = cell->count;
		stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
		stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
	}

	void ParticleSimulation::ClearBodies() {
		bodyCount = 0;
	}

	void ParticleSimulation::AddPlane(const Vec2f &normal, const float distance) {
		Body body = Body();
		body.type = BodyType::BodyType_Plane;
		body.plane.normal = normal;
		body.plane.distance = distance;
		assert(bodyCount < kSPHMaxBodyCount);
		size_t bodyIndex = bodyCount++;
		bodies[bodyIndex] = body;
	}

	void ParticleSimulation::AddCircle(const Vec2f &pos, const float radius) {
		Body body = Body();
		body.type = BodyType::BodyType_Circle;
		body.circle.pos = pos;
		body.circle.radius = radius;
		assert(bodyCount < kSPHMaxBodyCount);
		size_t bodyIndex = bodyCount++;
		bodies[bodyIndex] = body;
	}

	void ParticleSimulation::AddLineSegment(const Vec2f &a, const Vec2f &b) {
		Body body = Body();
		body.type = BodyType::BodyType_LineSegment;
		body.lineSegment.a = a;
		body.lineSegment.b = b;
		assert(bodyCount < kSPHMaxBodyCount);
		size_t bodyIndex = bodyCount++;
		bodies[bodyIndex] = body;
	}

	void ParticleSimulation::AddPolygon(const size_t vertexCount, const Vec2f *verts) {
		Body body = Body();
		body.type = BodyType::BodyType_Polygon;
		assert(vertexCount <= kMaxScenarioPolygonCount);
		for(size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
			body.polygon.verts[vertexIndex] = verts[vertexIndex];
		}
		body.polygon.vertexCount = vertexCount;
		assert(bodyCount < kSPHMaxBodyCount);
		size_t bodyIndex = bodyCount++;
		bodies[bodyIndex] = body;
	}

	void ParticleSimulation::ClearParticles() {
		for(size_t cellIndex = 0; cellIndex < kSPHGridTotalCount; ++cellIndex) {
			Cell *cell = &cells[cellIndex];
			assert(cell != nullptr);
			cell->count = 0;
		}
		particleCount = 0;
	}

	void ParticleSimulation::ClearEmitters() {
		emitterCount = 0;
	}

	void ParticleSimulation::ResetStats() {
		stats = {};
	}

	size_t ParticleSimulation::AddParticle(const Vec2f &position, const Vec2f &acceleration) {
		size_t particleIndex = particleCount++;

		particleDatas[particleIndex] = ParticleData(position);
		particleDatas[particleIndex].acceleration = acceleration;

		particleIndexes[particleIndex] = ParticleIndex();
		particleColors[particleIndex] = V4f(0, 0, 0, 1);

		InsertParticleIntoGrid(particleIndex);
		return particleIndex;
	}

	void ParticleSimulation::AddEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration) {
		assert(emitterCount < kSPHMaxEmitterCount);
		ParticleEmitter *emitter = &emitters[emitterCount++];
		emitter->position = position;
		emitter->direction = direction;
		emitter->radius = radius;
		emitter->speed = speed;
		emitter->rate = rate;
		emitter->duration = duration;
		emitter->elapsed = 0;
		emitter->totalElapsed = 0;
		emitter->isActive = true;
	}

	void ParticleSimulation::AddVolume(const Vec2f &center, const Vec2f &force, const int countX, const int countY, const float spacing) {
		Vec2f offset = V2f(countX * spacing, countY * spacing) * 0.5f;
		for(int yIndex = 0; yIndex < countY; ++yIndex) {
			for(int xIndex = 0; xIndex < countX; ++xIndex) {
				Vec2f p = V2f((float)xIndex, (float)yIndex) * spacing;
				p += V2f(spacing * 0.5f, spacing * 0.5f);
				p += center - offset;
				Vec2f jitter = Vec2RandomDirection() * kSPHKernelHeight * kSPHVolumeParticleDistributionScale;
				p += jitter;
				AddParticle(p, force);
			}
		}
	}

	void ParticleSimulation::NeighborSearch(const int64_t startIndex, const int64_t endIndex, const float deltaTime) {
		for(int64_t particleIndexA = startIndex; particleIndexA <= endIndex; ++particleIndexA) {
			ParticleIndex *particleIndexContainerA = &particleIndexes[particleIndexA];
			particleIndexContainerA->neighborCount = 0;
			Vec2i cellIndex = particleIndexContainerA->cellIndex;
			for(int y = -1; y <= 1; ++y) {
				for(int x = -1; x <= 1; ++x) {
					int cellPosX = cellIndex.x + x;
					int cellPosY = cellIndex.y + y;
					if(SPHIsPositionInGrid(cellPosX, cellPosY)) {
						size_t cellOffset = SPHComputeCellOffset(cellPosX, cellPosY);
						Cell *cell = &cells[cellOffset];
						assert(cell != nullptr);
						size_t particleCountInCell = cell->count;
						for(size_t index = 0; index < particleCountInCell; ++index) {
							size_t particleIndexB = cell->indices[index];
							assert(particleIndexContainerA->neighborCount < kSPHMaxParticleNeighborCount);
							particleIndexContainerA->neighbors[particleIndexContainerA->neighborCount++] = particleIndexB;
						}
					}
				}
			}
		}
	}

	void ParticleSimulation::DensityAndPressure(const int64_t startIndex, const int64_t endIndex, const float deltaTime) {
		for(int64_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			ParticleData *particleDataContainer = &particleDatas[particleIndex];
			ParticleIndex *particleIndexContainer = &particleIndexes[particleIndex];
			particleDataContainer->density = particleDataContainer->nearDensity = 0;
			size_t neighborCount = particleIndexContainer->neighborCount;
			for(size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particleIndexContainer->neighbors[index];
				ParticleData *neighborDataContainer = &particleDatas[neighborIndex];
				SPHComputeDensity(params, particleDataContainer->curPosition, neighborDataContainer->curPosition, particleDataContainer->densities);
			}
			SPHComputePressure(params, particleDataContainer->densities, particleDataContainer->pressures);
		}
	}

	void ParticleSimulation::ViscosityForces(const int64_t startIndex, const int64_t endIndex, const float deltaTime) {
		for(int64_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			ParticleData *particleDataContainer = &particleDatas[particleIndex];
			ParticleIndex *particleIndexContainer = &particleIndexes[particleIndex];
			size_t neighborCount = particleIndexContainer->neighborCount;
			for(size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particleIndexContainer->neighbors[index];
				ParticleData *neighborDataContainer = &particleDatas[neighborIndex];
				Vec2f force = V2f(0, 0);
				SPHComputeViscosityForce(params, particleDataContainer->curPosition, neighborDataContainer->curPosition, particleDataContainer->velocity, neighborDataContainer->velocity, &force);
				particleDataContainer->velocity -= force * 0.5f * deltaTime;
				neighborDataContainer->velocity += force * 0.5f * deltaTime;
			}
		}
	}

	void ParticleSimulation::DeltaPositions(const int64_t startIndex, const int64_t endIndex, const float deltaTime) {
		for(int64_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			ParticleData *particleDataContainer = &particleDatas[particleIndex];
			ParticleIndex *particleIndexContainer = &particleIndexes[particleIndex];
			Vec2f dx = V2f(0, 0);
			size_t neighborCount = particleIndexContainer->neighborCount;
			for(size_t index = 0; index < neighborCount; ++index) {
				size_t neighborIndex = particleIndexContainer->neighbors[index];
				ParticleData *neighborDataContainer = &particleDatas[neighborIndex];
				Vec2f delta = V2f(0, 0);
				SPHComputeDelta(params, particleDataContainer->curPosition, neighborDataContainer->curPosition, particleDataContainer->pressures, deltaTime, &delta);
				neighborDataContainer->curPosition += delta * 0.5f;
				dx -= delta * 0.5f;
			}
			particleDataContainer->curPosition += dx;
		}
	}

	void ParticleSimulation::UpdateEmitter(ParticleEmitter *emitter, const float deltaTime) {
		const float spacing = params.particleSpacing;
		const float invDeltaTime = 1.0f / deltaTime;
		if(emitter->isActive) {
			const float rate = 1.0f / emitter->rate;
			emitter->elapsed += deltaTime;
			emitter->totalElapsed += deltaTime;
			if(emitter->elapsed >= rate) {
				emitter->elapsed = 0;
				Vec2f acceleration = emitter->direction * emitter->speed * invDeltaTime;
				Vec2f dir = Vec2Cross(1.0f, emitter->direction);
				int count = (int)floor(emitter->radius / spacing);
				float halfSize = (float)count * spacing * 0.5f;
				Vec2f offset = dir * (float)count * spacing * 0.5f;
				for(int index = 0; index < count; ++index) {
					Vec2f p = dir * (float)index * spacing;
					p += dir * spacing * 0.5f;
					p += emitter->position - offset;
					Vec2f jitter = Vec2RandomDirection() * kSPHKernelHeight * kSPHVolumeParticleDistributionScale;
					p += jitter;
					AddParticle(p, acceleration);
				}
			}
			if(emitter->totalElapsed >= emitter->duration) {
				emitter->isActive = false;
			}
		}
	}

	void ParticleSimulation::Update(const float deltaTime) {
		const float invDt = 1.0f / deltaTime;
		const bool useMultiThreading = isMultiThreading;

		// Emitters
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for(size_t emitterIndex = 0; emitterIndex < emitterCount; ++emitterIndex) {
				ParticleEmitter *emitter = &emitters[emitterIndex];
				UpdateEmitter(emitter, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.emitters = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Integrate forces
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
				ParticleData *dataContainer = &particleDatas[particleIndex];
				dataContainer->acceleration += gravity + externalForce;
				dataContainer->velocity += dataContainer->acceleration * deltaTime;
				dataContainer->acceleration = V2f(0, 0);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.integration = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Viscosity force
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if(useMultiThreading) {
				workerPool.CreateTasks(particleCount, [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->ViscosityForces(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->ViscosityForces(0, particleCount - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.viscosityForces = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Predict
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
				ParticleData *dataContainer = &particleDatas[particleIndex];
				dataContainer->prevPosition = dataContainer->curPosition;
				dataContainer->curPosition += dataContainer->velocity * deltaTime;
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.predict = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Update grid
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
				ParticleData *dataContainer = &particleDatas[particleIndex];
				ParticleIndex *indexContainer = &particleIndexes[particleIndex];
				Vec2i newCellIndex = SPHComputeCellIndex(dataContainer->curPosition);
				Vec2i *oldCellIndex = &indexContainer->cellIndex;
				if(newCellIndex.x != oldCellIndex->x || newCellIndex.y != oldCellIndex->y) {
					RemoveParticleFromGrid(particleIndex);
					InsertParticleIntoGrid(particleIndex);
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.updateGrid = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Neighbor search
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if(useMultiThreading) {
				workerPool.CreateTasks(particleCount, [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->NeighborSearch(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->NeighborSearch(0, particleCount - 1, deltaTime);
			}
			stats.minParticleNeighborCount = kSPHMaxParticleNeighborCount;
			stats.maxParticleNeighborCount = 0;
			for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
				ParticleIndex *particleIndexContainer = &particleIndexes[particleIndex];
				size_t neighborCount = particleIndexContainer->neighborCount;
				stats.minParticleNeighborCount = std::min(neighborCount, stats.minParticleNeighborCount);
				stats.maxParticleNeighborCount = std::max(neighborCount, stats.maxParticleNeighborCount);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.neighborSearch = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Density and pressure
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if(useMultiThreading) {
				workerPool.CreateTasks(particleCount, [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DensityAndPressure(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->DensityAndPressure(0, particleCount - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.densityAndPressure = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Calculate delta position
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if(useMultiThreading) {
				workerPool.CreateTasks(particleCount, [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DeltaPositions(startIndex, endIndex, deltaTime);
				}, deltaTime);
				workerPool.WaitUntilDone();
			} else {
				this->DeltaPositions(0, particleCount - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.deltaPositions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Solve collisions
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
				ParticleData *dataContainer = &particleDatas[particleIndex];
				for(size_t bodyIndex = 0; bodyIndex < bodyCount; ++bodyIndex) {
					Body *body = &bodies[bodyIndex];
					switch(body->type) {
						case BodyType::BodyType_Plane:
						{
							Plane *plane = &body->plane;
							SPHSolvePlaneCollision(&dataContainer->curPosition, plane->normal, plane->distance);
						} break;
						case BodyType::BodyType_Circle:
						{
							Circle *circle = &body->circle;
							SPHSolveCircleCollision(&dataContainer->curPosition, circle->pos, circle->radius);
						} break;
						case BodyType::BodyType_LineSegment:
						{
							LineSegment *lineSegment = &body->lineSegment;
							SPHSolveLineSegmentCollision(&dataContainer->curPosition, lineSegment->a, lineSegment->b);
						} break;
						case BodyType::BodyType_Polygon:
						{
							Poly *polygon = &body->polygon;
							SPHSolvePolygonCollision(&dataContainer->curPosition, polygon->vertexCount, polygon->verts);
						} break;
					}
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			stats.time.collisions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);

		// Recalculate velocity for next frame
		for(size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
			ParticleData *dataContainer = &particleDatas[particleIndex];
			dataContainer->velocity = (dataContainer->curPosition - dataContainer->prevPosition) * invDt;
		}

		ValidateParticles(particleDatas, particleIndexes, particleCount);
	}

	void ParticleSimulation::Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale) {
		// Domain
		Vec4f domainColor = V4f(1.0f, 0.0f, 1.0f, 1.0f);
		Render::PushRectangle(commandBuffer, V2f(-kSPHBoundaryHalfWidth, -kSPHBoundaryHalfHeight), V2f(kSPHBoundaryHalfWidth, kSPHBoundaryHalfHeight) * 2.0f, domainColor, false, 1.0f);

		// Grid fill
		for(int yIndexInner = 0; yIndexInner < kSPHGridCountY; ++yIndexInner) {
			for(int xIndexInner = 0; xIndexInner < kSPHGridCountX; ++xIndexInner) {
				size_t cellOffset = SPHComputeCellOffset(xIndexInner, yIndexInner);
				Cell *cell = &cells[cellOffset];
				Vec2f innerP = kSPHGridOrigin + V2f((float)xIndexInner, (float)yIndexInner) * kSPHGridCellSize;
				Vec2f innerSize = V2f(kSPHGridCellSize, kSPHGridCellSize);
				if(cell->count > 0) {
					Render::PushRectangle(commandBuffer, innerP, innerSize, ColorLightGray, true);
				}
			}
		}

		// Grid lines
		for(int yIndex = 0; yIndex < kSPHGridCountY; ++yIndex) {
			Vec2f startP = kSPHGridOrigin + V2f(0, (float)yIndex) * kSPHGridCellSize;
			Vec2f endP = kSPHGridOrigin + V2f((float)kSPHGridCountX, (float)yIndex) * kSPHGridCellSize;
			Render::PushLine(commandBuffer, startP, endP, ColorDarkGray, 1.0f);
		}
		for(int xIndex = 0; xIndex < kSPHGridCountX; ++xIndex) {
			Vec2f startP = kSPHGridOrigin + V2f((float)xIndex, 0) * kSPHGridCellSize;
			Vec2f endP = kSPHGridOrigin + V2f((float)xIndex, (float)kSPHGridCountY) * kSPHGridCellSize;
			Render::PushLine(commandBuffer, startP, endP, ColorDarkGray, 1.0f);
		}

		// Bodies
		for(int bodyIndex = 0; bodyIndex < bodyCount; ++bodyIndex) {
			Body *body = &bodies[bodyIndex];
			switch(body->type) {
				case BodyType::BodyType_Plane:
				{
					Plane *plane = &body->plane;
					plane->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_Circle:
				{
					Circle *circle = &body->circle;
					circle->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_LineSegment:
				{
					LineSegment *lineSegment = &body->lineSegment;
					lineSegment->Render(commandBuffer);
				}
				break;
				case BodyType::BodyType_Polygon:
				{
					Poly *polygon = &body->polygon;
					polygon->Render(commandBuffer);
				}
				break;
			}
		}

		// Emitters
		for(int emitterIndex = 0; emitterIndex < emitterCount; ++emitterIndex) {
			ParticleEmitter *emitter = &emitters[emitterIndex];
			emitter->Render(commandBuffer);
		}

		// Particles
		for(int particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
			ParticleData &dataContainer = particleDatas[particleIndex];
			particleColors[particleIndex] = SPHGetParticleColor(params.restDensity, dataContainer.density, dataContainer.pressure, dataContainer.velocity);
		}
		float pointSize = kSPHParticleRenderRadius * 2.0f * worldToScreenScale;
		void *vertices = (void *)((uint8_t *)&particleDatas[0]);
		void *colors = (void *)((uint8_t *)&particleColors[0]);
		uint32_t vertexStride = sizeof(ParticleData);
		uint32_t colorStride = sizeof(Vec4f);
		Render::PushVertexIndexArrayHeader(commandBuffer, vertexStride, vertices, 0, nullptr, colorStride, colors, 0, nullptr);
		Render::PushVertexIndexArrayDraw(commandBuffer, Render::PrimitiveType::Points, (uint32_t)particleCount, pointSize, nullptr, {}, false);
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
		Render::PushPolygonFrom(commandBuffer, &verts[0], vertexCount, color, false, 1.0f);
	}

	void ParticleEmitter::Render(Render::CommandBuffer *commandBuffer) {
	}
}

#endif // DEMO4_IMPLEMENTATION