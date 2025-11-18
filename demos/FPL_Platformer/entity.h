#ifndef ENTITY_H
#define ENTITY_H

#include <final_math.h>

#include "map.h"

//
// Entity / Player values
//
#define PlayerGravity V2fInit(0, -10.0f)
#define PlayerMaxSpeed 100.0f
#define PlayerWalkSpeed 30.0f
#define PlayerAirSpeed 40.0f
#define PlayerJumpVelocity (200.0f * 1.2f)
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
	Vec2f position[2];
	Vec2f velocity;
	Vec2f radius;
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
extern void PlayerUpdate(Entity *player, const Map *map, const float dt);

#endif // ENTITY_H
