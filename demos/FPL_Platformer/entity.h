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
#define PlayerAirFriction 0.2f

//
// Types
//
typedef struct GroundState {
	fpl_b32 current;
	fpl_b32 last;
} GroundState;

typedef struct Entity {
	Vec4f color;

	// 0 = Current position, 1 = Last position
	Vec2f position[2];

	// Fixed radius of the player
	Vec2f radius;

	Vec2f velocity;
	Vec2f posCorrect;

	GroundState groundState;
	float groundFriction;
	float airFriction;
	bool applyFriction;
	bool applyAirFriction;
	bool jumpRequested;
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
