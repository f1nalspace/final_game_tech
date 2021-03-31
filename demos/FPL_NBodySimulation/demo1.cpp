#include "demo1.h"

#ifndef DEMO1_IMPLEMENTATION
#define DEMO1_IMPLEMENTATION

#include <chrono>
#include <algorithm>

#include "render.h"

namespace Demo1 {
	Grid::Grid(const size_t maxCellCount) {
		_cells.resize(maxCellCount);
		for (int cellIndex = 0; cellIndex < maxCellCount; ++cellIndex) {
			_cells[cellIndex] = nullptr;
		}
	}

	Grid::~Grid() {
		for (int cellIndex = 0; cellIndex < _cells.size(); ++cellIndex) {
			Cell *cell = _cells[cellIndex];
			if (cell != nullptr) {
				delete cell;
			}
		}
		_cells.clear();
	}

	Cell *Grid::GetCell(const size_t index) {
		Cell *result = _cells[index];
		return(result);
	}

	Cell *Grid::EnforceCell(const size_t index) {
		Cell *cell = _cells[index];
		if (cell == nullptr) {
			cell = _cells[index] = new Cell();
		}
		return(cell);
	}

	void Grid::Clear() {
		for (size_t cellIndex = 0; cellIndex < _cells.size(); ++cellIndex) {
			Cell *cell = _cells[cellIndex];
			if (cell != nullptr) {
				delete cell;
			}
			_cells[cellIndex] = nullptr;
		}
	}

	void Grid::InsertParticleIntoGrid(Particle *particle, SPHStatistics &stats) {
		Vec2f position = particle->GetPosition();
		Vec2i cellIndex = SPHComputeCellIndex(position);
		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);
		Cell *cell = EnforceCell(cellOffset);
		assert(cell != nullptr);
		cell->Add(particle);
		particle->SetCellIndex(cellIndex);

		size_t count = cell->GetCount();
		stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
		stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
	}

	void Grid::RemoveParticleFromGrid(Particle *particle, SPHStatistics &stats) {
		Vec2i cellIndex = particle->GetCellIndex();
		size_t cellOffset = SPHComputeCellOffset(cellIndex.x, cellIndex.y);
		Cell *cell = _cells[cellOffset];
		if (cell != nullptr) {
			cell->Remove(particle);
			size_t count = cell->GetCount();
			stats.minCellParticleCount = std::min(count, stats.minCellParticleCount);
			stats.maxCellParticleCount = std::max(count, stats.maxCellParticleCount);
			if (cell->GetCount() == 0) {
				_cells[cellOffset] = nullptr;
				delete cell;
			}
		}
	}

	ParticleSimulation::ParticleSimulation() :
		_gravity(V2f(0, 0)),
		_externalForce(V2f(0, 0)) {
		_grid = new Grid(kSPHGridTotalCount);
		_workerPool = new ThreadPool();
		_isMultiThreading = _workerPool->GetThreadCount() > 1;
	}

	ParticleSimulation::~ParticleSimulation() {
		for (int emitterIndex = 0; emitterIndex < _emitters.size(); ++emitterIndex) {
			delete _emitters[emitterIndex];
		}
		for (int bodyIndex = 0; bodyIndex < _bodies.size(); ++bodyIndex) {
			delete _bodies[bodyIndex];
		}
		for (int particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
			delete _particles[particleIndex];
		}
		delete _workerPool;
		delete _grid;
	}

	void ParticleSimulation::AddExternalForces(const Vec2f &force) {
		_externalForce += force;
	}

	void ParticleSimulation::ClearExternalForce() {
		_externalForce = V2f(0.0f, 0.0f);
	}

	void ParticleSimulation::AddPlane(const Vec2f & normal, const float distance) {
		Plane *plane = new Plane(normal, distance);
		_bodies.push_back(plane);
	}

	void ParticleSimulation::ClearBodies() {
		for (int bodyIndex = 0; bodyIndex < _bodies.size(); ++bodyIndex) {
			Body *body = _bodies[bodyIndex];
			delete body;
		}
		_bodies.clear();
	}

	void ParticleSimulation::ClearParticles() {
		_grid->Clear();
		for (int particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
			delete _particles[particleIndex];
		}
		_particles.clear();
		_particleRenderObjects.clear();
	}

	void ParticleSimulation::ClearEmitters() {
		for (size_t emitterIndex = 0; emitterIndex < _emitters.size(); ++emitterIndex) {
			delete _emitters[emitterIndex];
		}
		_emitters.clear();
	}

	void ParticleSimulation::ResetStats() {
		_stats = {};
	}

	size_t ParticleSimulation::AddParticle(const Vec2f & position, const Vec2f &force) {
		size_t particleIndex = _particles.size();
		Particle *particle = new Particle(position);
		particle->SetAcceleration(force);
		_particles.push_back(particle);
		_particleRenderObjects.push_back(ParticleRenderObject());
		_grid->InsertParticleIntoGrid(particle, _stats);
		return particleIndex;
	}

	void ParticleSimulation::AddVolume(const Vec2f & center, const Vec2f &force, const int countX, const int countY, const float spacing) {
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
		ParticleEmitter *emitter = new ParticleEmitter(position, direction, radius, speed, rate, duration);
		_emitters.push_back(emitter);
	}

	void Particle::SetVelocity(const Vec2f &v) {
		_velocity = v;
	}
	const Vec2f &Particle::GetVelocity() {
		return _velocity;
	}

	const Vec2f &Particle::GetPosition() {
		return _curPosition;
	}

	void Particle::SetPosition(const Vec2f &position) {
		_curPosition = position;
	}
	const Vec2f &Particle::GetPrevPosition() {
		return _prevPosition;
	}

	void Particle::SetPrevPosition(const Vec2f &prevPosition) {
		_prevPosition = prevPosition;
	}

	void Particle::SetAcceleration(const Vec2f &acceleration) {
		_acceleration = acceleration;
	}
	const Vec2f &Particle::GetAcceleration() {
		return _acceleration;
	}

	const Vec2i &Particle::GetCellIndex() {
		return _cellIndex;
	}
	void Particle::SetCellIndex(const Vec2i &cellIndex) {
		_cellIndex = cellIndex;
	}

	Particle *Particle::GetNeighbor(size_t index) {
		return _neighbors[index];
	}

	size_t Particle::GetNeighborCount() {
		return _neighbors.size();
	}

	float Particle::GetDensity() {
		return _density;
	}

	float Particle::GetNearDensity() {
		return _nearDensity;
	}

	void Particle::SetDensity(const float density) {
		_density = density;
	}

	void Particle::SetNearDensity(const float nearDensity) {
		_nearDensity = nearDensity;
	}

	void Particle::SetPressure(const float pressure) {
		_pressure = pressure;
	}

	float Particle::GetPressure() {
		return _pressure;
	}

	float Particle::GetNearPressure() {
		return _nearPressure;
	}

	void Particle::SetNearPressure(const float nearPressure) {
		_nearPressure = nearPressure;
	}

	void Particle::ComputeDensityAndPressure(const SPHParameters &params, SPHStatistics &stats) {
		_density = _nearDensity = 0;
		size_t neighborCount = GetNeighborCount();
		for (size_t index = 0; index < neighborCount; ++index) {
			Particle *neighbor = GetNeighbor(index);
			float outDensities[2] = { _density, _nearDensity };
			SPHComputeDensity(params, _curPosition, neighbor->GetPosition(), outDensities);
			_density = outDensities[0];
			_nearDensity = outDensities[1];
		}
		float inDensities[2] = { _density, _nearDensity };
		float outPressures[2];
		SPHComputePressure(params, inDensities, outPressures);
		_pressure = outPressures[0];
		_nearPressure = outPressures[1];
	}

	void Particle::ComputeDeltaPosition(const SPHParameters &params, const float deltaTime, SPHStatistics &stats) {
		Vec2f dx = V2f(0,0);
		size_t neighborCount = GetNeighborCount();
		float pressure[2] = { _pressure, _nearPressure };
		for (size_t index = 0; index < neighborCount; ++index) {
			Particle *neighbor = GetNeighbor(index);
			Vec2f delta = V2f(0,0);
			SPHComputeDelta(params, _curPosition, neighbor->GetPosition(), pressure, deltaTime, &delta);
			neighbor->SetPosition(neighbor->GetPosition() + delta * 0.5f);
			dx -= delta * 0.5f;
		}
		_curPosition += dx;
	}

	void Particle::ComputeViscosityForces(const SPHParameters &params, const float deltaTime, SPHStatistics &stats) {
		size_t neighborCount = GetNeighborCount();
		for (size_t index = 0; index < neighborCount; ++index) {
			Particle *neighbor = GetNeighbor(index);
			Vec2f force;
			SPHComputeViscosityForce(params, _curPosition, neighbor->GetPosition(), _velocity, neighbor->GetVelocity(), &force);
			_velocity += -force * deltaTime * 0.5f;
			neighbor->SetVelocity(neighbor->GetVelocity() + force * deltaTime * 0.5f);
		}
	}

	void ParticleSimulation::NeighborSearch(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle *particle = _particles[particleIndex];
			particle->UpdateNeighbors(_grid);
		}
	}

	void ParticleSimulation::DensityAndPressure(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle *particle = _particles[particleIndex];
			particle->ComputeDensityAndPressure(_params, _stats);
		}
	}

	void ParticleSimulation::ViscosityForces(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle *particle = _particles[particleIndex];
			particle->ComputeViscosityForces(_params, deltaTime, _stats);
		}
	}

	void ParticleSimulation::DeltaPositions(const size_t startIndex, const size_t endIndex, const float deltaTime) {
		for (size_t particleIndex = startIndex; particleIndex <= endIndex; ++particleIndex) {
			Particle *particle = _particles[particleIndex];
			particle->ComputeDeltaPosition(_params, deltaTime, _stats);
		}
	}

	void ParticleSimulation::UpdateEmitter(ParticleEmitter *emitter, const float deltaTime) {
		const float spacing = _params.particleSpacing;
		const float invDeltaTime = 1.0f / deltaTime;
		if (emitter->GetIsActive()) {
			const float rate = 1.0f / emitter->GetRate();
			emitter->SetElapsed(emitter->GetElapsed() + deltaTime);
			emitter->SetTotalElapsed(emitter->GetTotalElapsed() + deltaTime);
			if (emitter->GetElapsed() >= rate) {
				emitter->SetElapsed(0);
				Vec2f acceleration = emitter->GetDirection() * emitter->GetSpeed() * invDeltaTime;
				Vec2f dir = Vec2Cross(1.0f, emitter->GetDirection());
				int count = (int)floor(emitter->GetRadius() / spacing);
				float halfSize = (float)count * spacing * 0.5f;
				Vec2f offset = dir * (float)count * spacing * 0.5f;
				for (int index = 0; index < count; ++index) {
					Vec2f p = dir * (float)index * spacing;
					p += dir * spacing * 0.5f;
					p += emitter->GetPosition() - offset;
					Vec2f jitter = Vec2RandomDirection() * kSPHKernelHeight * kSPHVolumeParticleDistributionScale;
					p += jitter;
					AddParticle(p, acceleration);
				}
			}
			if (emitter->GetTotalElapsed() >= emitter->GetDuration()) {
				emitter->SetIsActive(false);
			}
		}
	}

	void ParticleSimulation::Update(const float deltaTime) {
		const float invDt = 1.0f / deltaTime;
		const bool useMultiThreading = _isMultiThreading;

		// Emitters
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t emitterIndex = 0; emitterIndex < _emitters.size(); ++emitterIndex) {
				ParticleEmitter *emitter = _emitters[emitterIndex];
				UpdateEmitter(emitter, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.emitters = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Integrate forces
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				Particle *particle = _particles[particleIndex];
				particle->SetAcceleration(particle->GetAcceleration() + _gravity + _externalForce);
				particle->IntegrateForces(deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.integration = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Viscosity forces
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				_workerPool->CreateTasks(_particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->ViscosityForces(startIndex, endIndex, deltaTime);
				}, deltaTime);
				_workerPool->WaitUntilDone();
			} else {
				this->ViscosityForces(0, _particles.size() - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.viscosityForces = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Predict
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				Particle *particle = _particles[particleIndex];
				particle->Predict(deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.predict = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Update grid
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				Particle *particle = _particles[particleIndex];
				Vec2i oldCellIndex = particle->GetCellIndex();
				Vec2i newCellIndex = SPHComputeCellIndex(particle->GetPosition());
				if (newCellIndex.x != oldCellIndex.x || newCellIndex.y != oldCellIndex.y) {
					_grid->RemoveParticleFromGrid(particle, _stats);
					_grid->InsertParticleIntoGrid(particle, _stats);
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.updateGrid = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Neighbor search
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				_workerPool->CreateTasks(_particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->NeighborSearch(startIndex, endIndex, deltaTime);
				}, deltaTime);
				_workerPool->WaitUntilDone();
			} else {
				this->NeighborSearch(0, _particles.size() - 1, deltaTime);
			}
			_stats.minParticleNeighborCount = kSPHMaxParticleNeighborCount;
			_stats.maxParticleNeighborCount = 0;
			for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				Particle *particle = _particles[particleIndex];
				_stats.minParticleNeighborCount = std::min(particle->GetNeighborCount(), _stats.minParticleNeighborCount);
				_stats.maxParticleNeighborCount = std::max(particle->GetNeighborCount(), _stats.maxParticleNeighborCount);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.neighborSearch = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Density and pressure
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				_workerPool->CreateTasks(_particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DensityAndPressure(startIndex, endIndex, deltaTime);
				}, deltaTime);
				_workerPool->WaitUntilDone();
			} else {
				this->DensityAndPressure(0, _particles.size() - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.densityAndPressure = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Calculate delta position
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			if (useMultiThreading) {
				_workerPool->CreateTasks(_particles.size(), [=](const size_t startIndex, const size_t endIndex, const float deltaTime) {
					this->DeltaPositions(startIndex, endIndex, deltaTime);
				}, deltaTime);
				_workerPool->WaitUntilDone();
			} else {
				this->DeltaPositions(0, _particles.size() - 1, deltaTime);
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.deltaPositions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Solve collisions
		{
			auto startClock = std::chrono::high_resolution_clock::now();
			for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				Particle *particle = _particles[particleIndex];
				for (size_t bodyIndex = 0; bodyIndex < _bodies.size(); ++bodyIndex) {
					Body *body = _bodies[bodyIndex];
					body->SolveCollision(particle);
				}
			}
			auto deltaClock = std::chrono::high_resolution_clock::now() - startClock;
			_stats.time.collisions = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaClock).count() * nanosToMilliseconds;
		}

		// Recalculate velocity for next frame
		for (size_t particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
			Particle *particle = _particles[particleIndex];
			particle->UpdateVelocity(invDt);
		}
	}

	void ParticleSimulation::Render(Render::CommandBuffer *commandBuffer, const float worldToScreenScale) {
		// Domain
		Vec4f domainColor = V4f(1.0f, 0.0f, 1.0f, 1.0f);
		Render::PushRectangle(commandBuffer, V2f(-kSPHBoundaryHalfWidth, -kSPHBoundaryHalfHeight), V2f(kSPHBoundaryHalfWidth, kSPHBoundaryHalfHeight) * 2.0f, domainColor, false, 1.0f);

		// Grid fill
		const Vec2f innerSize = V2f(kSPHGridCellSize, kSPHGridCellSize);
		for (int yIndexInner = 0; yIndexInner < kSPHGridCountY; ++yIndexInner) {
			for (int xIndexInner = 0; xIndexInner < kSPHGridCountX; ++xIndexInner) {
				size_t cellOffset = SPHComputeCellOffset(xIndexInner, yIndexInner);
				Vec2f innerP = kSPHGridOrigin + V2f((float)xIndexInner, (float)yIndexInner) * kSPHGridCellSize;
				Cell *cell = _grid->GetCell(cellOffset);
				if (cell != nullptr && cell->GetCount() > 0) {
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
		for (int bodyIndex = 0; bodyIndex < _bodies.size(); ++bodyIndex) {
			Body *body = _bodies[bodyIndex];
			body->Render(commandBuffer);
		}

		// Particles
		if (_particles.size() > 0) {
			for (int particleIndex = 0; particleIndex < _particles.size(); ++particleIndex) {
				ParticleRenderObject *particleRenderObject = &_particleRenderObjects[particleIndex];
				Particle *particle = _particles[particleIndex];
				particleRenderObject->pos = particle->GetPosition();
				particleRenderObject->color = SPHGetParticleColor(_params.restDensity, particle->GetDensity(), particle->GetPressure(), particle->GetVelocity());
			}
			float pointSize = kSPHParticleRenderRadius * 2.0f * worldToScreenScale;
			void *vertices = (void *)((uint8_t *)&_particleRenderObjects[0] + offsetof(ParticleRenderObject, pos));
			void *colors = (void *)((uint8_t *)&_particleRenderObjects[0] + offsetof(ParticleRenderObject, color));
			uint32_t particleCount = (uint32_t)_particles.size();
			uint32_t vertexStride = sizeof(ParticleRenderObject);
			Render::PushVertexIndexArrayHeader(commandBuffer, vertexStride, vertices, 0, nullptr, vertexStride, colors, 0, nullptr);
			Render::PushVertexIndexArrayDraw(commandBuffer, Render::PrimitiveType::Points, particleCount, pointSize, nullptr, {}, false);
		}
	}

	Particle::Particle(const Vec2f &position) :
		_acceleration(V2f(0,0)),
		_velocity(V2f(0,0)),
		_cellIndex(V2i(0,0)),
		_prevPosition(position),
		_curPosition(position),
		_density(0),
		_nearDensity(0),
		_pressure(0),
		_nearPressure(0) {
	}

	void Particle::ClearDensity() {
		_density = _nearDensity = 0;
	}
	void Particle::IntegrateForces(const float deltaTime) {
		_velocity += _acceleration * deltaTime;
		_acceleration = V2f(0,0);
	}

	void Particle::Predict(const float deltaTime) {
		_prevPosition = _curPosition;
		_curPosition += _velocity * deltaTime;
	}

	void Particle::UpdateVelocity(const float invDeltaTime) {
		_velocity = (_curPosition - _prevPosition) * invDeltaTime;
	}

	void Particle::UpdateNeighbors(Grid *grid) {
		_neighbors.clear();
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				int cellPosX = _cellIndex.x + x;
				int cellPosY = _cellIndex.y + y;
				if (SPHIsPositionInGrid(cellPosX, cellPosY)) {
					size_t cellOffset = SPHComputeCellOffset(cellPosX, cellPosY);
					Cell *cell = grid->EnforceCell(cellOffset);
					assert(cell != nullptr);
					size_t particleCountInCell = cell->GetCount();
					if (particleCountInCell > 0) {
						for (size_t index = 0; index < particleCountInCell; ++index) {
							Particle *particleB = cell->GetParticle(index);
							_neighbors.push_back(particleB);
						}
					}
				}
			}
		}
	}

	void Plane::Render(Render::CommandBuffer *commandBuffer) {
		Vec2f p = _normal * _distance;
		Vec2f t = V2f(_normal.y, -_normal.x);
		Vec4f color = ColorBlue;
		Vec2f a = V2f(p.x + t.x * kSPHVisualPlaneLength, p.y + t.y * kSPHVisualPlaneLength);
		Vec2f b = V2f(p.x - t.x * kSPHVisualPlaneLength, p.y - t.y * kSPHVisualPlaneLength);
		Render::PushLine(commandBuffer, a, b, color, 1.0f);
	}

	Cell::Cell() {
	}

	void Cell::Add(Particle *particle) {
		_particles.push_back(particle);
	}

	void Cell::Remove(Particle *particle) {
		auto result = std::find(_particles.begin(), _particles.end(), particle);
		assert(result != _particles.end());
		_particles.erase(result);
	}

	void Cell::Clear() {
		_particles.clear();
	}

	Particle *Cell::GetParticle(size_t index) {
		return _particles[index];
	}
	size_t Cell::GetCount() {
		return _particles.size();
	}


	void ParticleSimulation::AddCircle(const Vec2f & pos, const float radius) {
		_bodies.push_back(new Circle(pos, radius));
	}

	void ParticleSimulation::AddLineSegment(const Vec2f & a, const Vec2f & b) {
		_bodies.push_back(new LineSegment(a, b));
	}

	void ParticleSimulation::AddPolygon(const size_t vertexCount, const Vec2f *verts) {
		std::vector<Vec2f> polyVerts;
		for (size_t i = 0; i < vertexCount; ++i) {
			polyVerts.push_back(verts[i]);
		}
		_bodies.push_back(new Poly(polyVerts));
	}

	// ParticleEmitter

	ParticleEmitter::ParticleEmitter(const Vec2f &position, const Vec2f &direction, const float radius, const float speed, const float rate, const float duration) {
		_position = position;
		_direction = direction;
		_radius = radius;
		_speed = speed;
		_rate = rate;
		_duration = duration;
		_elapsed = 0;
		_totalElapsed = 0;
		_isActive = true;
	}

	const Vec2f &ParticleEmitter::GetPosition() {
		return _position;
	}

	const Vec2f &ParticleEmitter::GetDirection() {
		return _direction;
	}

	float ParticleEmitter::GetRadius() {
		return _radius;
	}

	float ParticleEmitter::GetSpeed() {
		return _speed;
	}

	float ParticleEmitter::GetRate() {
		return _rate;
	}

	float ParticleEmitter::GetDuration() {
		return _duration;
	}

	float ParticleEmitter::GetElapsed() {
		return _elapsed;
	}
	void ParticleEmitter::SetElapsed(const float elapsed) {
		_elapsed = elapsed;
	}
	float ParticleEmitter::GetTotalElapsed() {
		return _totalElapsed;
	}
	void ParticleEmitter::SetTotalElapsed(const float totalElapsed) {
		_totalElapsed = totalElapsed;
	}
	bool ParticleEmitter::GetIsActive() {
		return _isActive;
	}
	void ParticleEmitter::SetIsActive(const bool isActive) {
		_isActive = isActive;
	}


	// Body

	Body::Body(const BodyType type) {
		_type = type;
	}

	Body::~Body() {

	}

	BodyType Body::GetType() {
		return _type;
	}

	// Plane

	Plane::Plane(const Vec2f &normal, const float distance) :
		Body(BodyType::Plane) {
		_normal = normal;
		_distance = distance;
	}

	const Vec2f &Plane::GetNormal() {
		return _normal;
	}
	float Plane::GetDistance() {
		return _distance;
	}

	void Plane::SolveCollision(Particle *particle) {
		Vec2f p = particle->GetPosition();
		SPHSolvePlaneCollision(&p, GetNormal(), GetDistance());
		particle->SetPosition(p);
	}

	// Circle

	Circle::Circle(const Vec2f &pos, const float radius) :
		Body(BodyType::Circle) {
		_pos = pos;
		_radius = radius;
	}

	const Vec2f &Circle::GetPosition() {
		return _pos;
	}
	float Circle::GetRadius() {
		return _radius;
	}

	void Circle::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushCircle(commandBuffer, _pos, _radius, color, 1.0f, false);
	}

	void Circle::SolveCollision(Particle *particle) {
		Vec2f p = particle->GetPosition();
		SPHSolveCircleCollision(&p, GetPosition(), GetRadius());
		particle->SetPosition(p);
	}

	// LineSegment

	LineSegment::LineSegment(const Vec2f &a, const Vec2f &b) :
		Body(BodyType::LineSegment) {
		_a = a;
		_b = b;
	}

	const Vec2f &LineSegment::GetA() {
		return _a;
	}
	const Vec2f &LineSegment::GetB() {
		return _b;
	}

	void LineSegment::SolveCollision(Particle *particle) {
		Vec2f p = particle->GetPosition();
		SPHSolveLineSegmentCollision(&p, GetA(), GetB());
		particle->SetPosition(p);
	}

	void LineSegment::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushLine(commandBuffer, _a, _b, color, 1.0f);
	}

	// Poly

	Poly::Poly(const std::vector<Vec2f> &verts) :
		Body(BodyType::Polygon) {
		_verts = verts;
	}

	const size_t Poly::GetVertexCount() {
		return _verts.size();
	}

	const Vec2f &Poly::GetVertex(size_t index) {
		return _verts[index];
	}

	void Poly::SolveCollision(Particle *particle) {
		Vec2f p = particle->GetPosition();
		SPHSolvePolygonCollision(&p, _verts.size(), &_verts[0]);
		particle->SetPosition(p);
	}

	void Poly::Render(Render::CommandBuffer *commandBuffer) {
		Vec4f color = ColorBlue;
		Render::PushPolygonFrom(commandBuffer, &_verts[0], _verts.size(), color, false, 1.0f);
	}
}

#endif // DEMO1_IMPLEMENTATION