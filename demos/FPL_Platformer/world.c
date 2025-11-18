#include "world.h"

extern bool WorldInit(fmemMemoryBlock *memory, World *world) {
	if (memory == fpl_null || world == fpl_null) {
		return false; // Invalid arguments
	}

	fplClearStruct(world);

	if (!fmemPushBlock(memory, &world->memory, fplMegaBytes(64), fmemPushFlags_Clear)) {
		return false; // Insufficient memory
	}

	if (!MapInit(&world->memory, &world->map)) {
		return false; // Failed map initialization
	}

	world->camera.scale[0] = world->camera.scale[1] = 1.0f;
	world->camera.offset[0] = world->camera.offset[1] = V2fInit(0.0f, 0.0f);

	return true;
}

extern bool WorldLoadMap(World *world, const Map *map) {
	if (world == fpl_null || map == fpl_null) {
		return false;
	}

	if (!MapAssign(&world->map, map)) {
		return false; // Failed map assignment
	}

	if (!PlayerInit(&world->player, &world->map)) {
		return false; // Failed player initialization
	}

	world->camera.scale[0] = world->camera.scale[1] = 1.0f;
	world->camera.offset[0] = world->camera.offset[1] = V2fInit(0.0f, 0.0f);

	return true;
}
