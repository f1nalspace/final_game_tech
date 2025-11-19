#ifndef ENTITY_H
#define ENTITY_H

#include <final_math.h>

#include "map.h"

#include "physics.h"

//
// Entity / Player values
//
#define PlayerGravity (TileHeight * 0.5f)

#define PlayerMaxHorizontalSpeed (TileWidth * 4.0f)
#define PlayerMaxVerticalSpeed (TileHeight * 20.0f)

#define PlayerWalkSpeed (TileWidth * 3.0f)
#define PlayerAirSpeed (TileWidth * 0.1f)
#define PlayerJumpVelocity (TileHeight * 12.0f)

#define PlayerGroundFriction 0.2f
#define PlayerAirFriction 0.0f

//
// Types
//
typedef struct GroundState {
	fpl_b32 current;
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

extern bool PlayerInit(Entity *player, const Map *map);
extern void PlayerInput(Entity *player, const Input *input);
extern void PlayerUpdate(Physics *physics, Entity *player, const Map *map, const float dt);

#endif // ENTITY_H
