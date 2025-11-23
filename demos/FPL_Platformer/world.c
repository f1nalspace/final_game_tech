#include "world.h"

static bool GetMapPositionForEntity(const Vec2f entityRadius, const Map *map, const TileType tileType, Vec2f *outPos) {
	if (map == fpl_null || outPos == fpl_null) {
		return false; // Invalid arguments
	}

	Vec2i tilePos;
	if (!MapFindPositionByTile(map, TileType_PlayerStart, &tilePos)) {
		return false; // No tile by type found
	}

	Vec2f tilePosBottomLeft = MapTileCoordsToWorld(map, tilePos);

	Vec2f tileBottomCenter = V2fAdd(tilePosBottomLeft, V2fInit(TileWidth * 0.5f, 0));

	Vec2f finalPos;

	// Move the player above the tile, but to the center
	finalPos = V2fAdd(tileBottomCenter, V2fInit(0, entityRadius.h));

	// Move the player above the tile, but to the right
	finalPos = V2fAdd(tileBottomCenter, V2fInit(TileSize.w * 0.5f - entityRadius.w, entityRadius.h));

	// Move the player above the tile, but to the left
	finalPos = V2fAdd(tileBottomCenter, V2fInit(-TileSize.w * 0.5f + entityRadius.w, entityRadius.h));

	*outPos = finalPos;

	return true;
}

extern bool WorldInit(fmemMemoryBlock *memory, World *world) {
	if (memory == fpl_null || world == fpl_null) {
		return false; // Invalid arguments
	}

	fplClearStruct(world);

	if (!fmemPushBlock(memory, &world->transientMemory, WORLD_MEMORY_TRANSIENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for world transient memory
	}
	if (!fmemPushBlock(memory, &world->persistentMemory, WORLD_MEMORY_PERSISTENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for world persistent memory
	}

	if (!MapInit(&world->persistentMemory, &world->map)) {
		return false; // Failed to initialize map
	}

	if (!PhysicsInit(&world->physics)) {
		return false; // Failed to initialize physics system
	}

	world->camera.scale[0] = world->camera.scale[1] = 1.0f;
	world->camera.offset[0] = world->camera.offset[1] = V2fInit(0.0f, 0.0f);

	return true;
}

extern bool WorldClear(World *world) {
	if (world == fpl_null) {
		return false;
	}

	// NOTE(final): Do not fully clear the world struct, otherwise we will kill the memory blocks!

	fplClearStruct(&world->entities);

	fplClearStruct(&world->camera);

	MapClear(&world->map);

	PhysicsClear(&world->physics);

	return true;
}

extern bool WorldLoad(World *world, const MapDefinition *mapDefinition) {
	if (world == fpl_null || mapDefinition == fpl_null) {
		return false; // Invalid arguments
	}

	Map *map = &world->map;

	if (!WorldClear(world)) {
		return false; // Failed to clear the world
	}

	if (!MapAssign(&world->map, mapDefinition)) {
		return false; // Failed to assign the map
	}

	uint32_t playerIndex = (uint32_t)world->entities.count++;
	uint32_t playerId = EntityIDStart + playerIndex;
	Vec2f playerRadius = V2fHadamard(TileSize, V2fInit(0.3f, 0.7f));
	Vec4f playerColor = V4fInit(0.05f, 0.1f, 0.95f, 1);
	Vec2f playerPosition;
	if (!GetMapPositionForEntity(playerRadius, map, TileType_PlayerStart, &playerPosition)) {
		playerPosition = V2fZero();
	}
	Entity *player = &world->entities.player;
	if (!EntityInit(player, playerId, playerRadius, playerPosition, playerColor)) {
		return false; // Failed to initialize the player
	}

	world->camera.scale[0] = world->camera.scale[1] = 1.0f;
	world->camera.offset[0] = world->camera.offset[1] = V2fInit(0.0f, 0.0f);

	return true;
}

extern void WorldUpdate(World *world, const Input *input) {
	const float dt = input->fixedDeltaTime;

	Map *map = &world->map;
	Entity *player = &world->entities.player;
	Physics *physics = &world->physics;

	// Clears contacts and may reset accumulators
	PhysicsBegin(physics);

	for (size_t i = 0; i < world->entities.count; ++i) {
		Entity *entity = world->entities.list + i;
		EntityUpdate(physics, entity, map, dt);
	}

	// May double-check results later
	PhysicsEnd(physics);

	world->camera.offset[0] = V2fNegate(player->position[0]);
	world->camera.scale[0] = 1.0f;
}
