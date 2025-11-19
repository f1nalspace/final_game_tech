#ifndef PHYSICS_H
#define PHYSICS_H

#include <final_math.h>

#include <final_geometry.h>

#include "map.h"

typedef union {
	struct {
		uint32_t a;
		uint32_t b;
	};
	uint64_t value;
} ContactIDPair;

typedef struct {
	ContactIDPair idPair;
	Vec2f posA;
	Vec2f posB;
	Vec2f normal;
	float distance;
	float impulse;
} Contact;

#define PhysicsMaxContactCount 512

typedef struct ContactList {
	size_t capacity;
	size_t used;
	Contact data[PhysicsMaxContactCount];
} ContactList;

typedef struct Physics {
	ContactList contactList;
} Physics;

extern bool PhysicsInit(Physics *physics);
extern bool PhysicsClear(Physics *physics);
extern void PhysicsBegin(Physics *physics);
extern void PhysicsEnd(Physics *physics);
extern bool PhysicsPushContact(Physics *physics, const Contact *contact);

extern uint32_t CreateContactsAABBvsAABB(const Map *map, const uint32_t idA, const AABB2f *a, const uint32_t idB, const AABB2f *b, const Vec2i tilePos, const bool checkInternal, Contact contacts[2]);

// A small distance used as a collision and constraint tolerance. Usually it is chosen to be numerically significant, but visually insignificant.
#define PhysicsLinearSlope 0.001f

// Additional distance that is added to motion AABB's, to ensure that tiles are encapsulated in all cases
#define PhysicsAABBExpansion 1.5f

#endif // PHYSICS_H