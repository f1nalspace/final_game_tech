#include "world.h"

static bool PlayerInit(Entity *player, const Map *map) {
	if (player == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}

	player->radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player->velocity = V2fInit(0.0f, 0.0f);
	player->color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player->position[0] = player->position[1] = V2fInit(0.0f, 0.0f);

	Vec2i playerTilePos;
	if (MapFindPositionByTile(map, TileType_PlayerPosition, &playerTilePos)) {
		Vec2f tilePos = MapTileCoordsToWorld(map, playerTilePos);
		Vec2f tileBottomCenter = V2fAdd(tilePos, V2fInit(TileWidth * 0.5f, 0));

		// Move the player above the tile, but to the center
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(0, player->radius.h));

		// Move the player above the tile, but to the right
		player->position[0] = player->position[1] = V2fAdd(tileBottomCenter, V2fInit(TileSize.w * 0.5f - player->radius.w, player->radius.h));
	}

	player->applyFriction = true;
	player->groundFriction = PlayerGroundFriction;

	player->applyAirFriction = true;
	player->airFriction = PlayerAirFriction;

	player->jumpRequested = false;

	return true;
}

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
