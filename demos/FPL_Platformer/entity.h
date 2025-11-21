#ifndef ENTITY_H
#define ENTITY_H

#include <final_math.h>

#include "map.h"

#include "physics.h"

//
// Entity / Player values
//
#define EntityGravity (TileHeight * 0.5f)

#define EntityMaxHorizontalSpeed (TileWidth * 4.0f)
#define EntityMaxVerticalSpeed (TileHeight * 20.0f)

#define EntityDefaultWalkSpeed (TileWidth * 3.0f)
#define EntityDefaultAirSpeed (TileWidth * 0.1f)
#define EntityDefaultJumpVelocity (TileHeight * 12.0f)

#define EntityDefaultGroundFriction 0.2f
#define EntityDefaultAirFriction 0.0f

#define EntitySensorGroundDistanceFromFoot 0.1f

//
// Types
//
typedef struct GroundState {
	// Is entity grounded
	fpl_b32 current;
	// Was entity grounded
	fpl_b32 last;
} GroundState;

typedef struct Entity {
	// Display color
	Vec4f color;

	// 0 = Current position, 1 = Last position
	Vec2f position[2];

	// Fixed radius
	Vec2f radius;

	// Current speed/velocity
	Vec2f velocity;

	// Position correction accumulator
	Vec2f posCorrect;

	// Ground state
	GroundState groundState;

	// ID of the entity
	uint32_t id;

	// Friction that is applied when on on-ground (0.0: Ice, 1.0: Glue)
	float groundFriction;

	// Friction that is applied when on in the air (0.0: No friction, 1.0: Glue)
	float airFriction;
	
	// Value indicating whether any ground friction is applied or not
	bool applyGroundFriction;

	// Value indicating whether any air friction is applied or not
	bool applyAirFriction;

	// Value indicating whether any air friction is applied or not
	bool isJumpRequested;

	// Padding to fix alignment
	bool padding;
} Entity;

//
// Public functions
//
fpl_inline bool EntityIsGrounded(const Entity *entity) {
	if (entity == fpl_null) {
		return false;
	}
	return entity->groundState.current;
}

fpl_inline bool EntityIsAir(const Entity *entity) {
	if (entity == fpl_null) {
		return false;
	}
	return !entity->groundState.current;
}

fpl_inline AABB2f EntityGetMotionBounds(const Entity *entity, const float dt) {
	// Predict position for next frame
	Vec2f predictedPos = V2fAddMultScalar(entity->position[0], V2fAdd(entity->velocity, entity->posCorrect), dt);
	// Create motion and return it
	Vec2f min = V2fSub(V2fMin(predictedPos, entity->position[0]), entity->radius);
	Vec2f max = V2fAdd(V2fMax(predictedPos, entity->position[0]), entity->radius);
	AABB2f result = AABB2fInit(min, max);
	return result;
}

extern bool EntityInit(Entity *entity, const uint32_t id, const Vec2f radius, const Vec2f position, const Vec4f color);
extern void EntityInput(Entity *entity, const Input *input);
extern void EntityUpdate(Physics *physics, Entity *entity, const Map *map, const float dt);

#endif // ENTITY_H
