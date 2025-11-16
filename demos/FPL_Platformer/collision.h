#ifndef COLLISION_H
#define COLLISION_H

#include <final_math.h>

#include <final_geometry.h>

typedef union {
	struct {
		uint32_t low;
		uint32_t high;
	};
	uint64_t value;
} ContactIDPair;

typedef struct {
	Vec2f pos;
	Vec2f normal;
	ContactIDPair idPair;
	float distance;
	float impulse;
} Contact;

bool CreateContactsAABBvsAABB();

#endif // COLLISION_H