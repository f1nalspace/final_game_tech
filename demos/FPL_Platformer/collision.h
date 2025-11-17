#ifndef COLLISION_H
#define COLLISION_H

#include <final_math.h>

#include <final_geometry.h>

#include "map.h"

typedef union {
	struct {
		uint32_t low;
		uint32_t high;
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

extern uint32_t CreateContactsAABBvsAABB(const Map *map, const uint32_t idA, const AABB2f *a, const uint32_t idB, const AABB2f *b, const Vec2i tilePos, const bool checkInternal, Contact outContacts[2]);

#endif // COLLISION_H