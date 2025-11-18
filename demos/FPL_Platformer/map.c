#include "map.h"

extern bool MapInit(fmemMemoryBlock *memory, Map *map) {
	if (memory == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(map);
	map->tileSize = TileSize;
	map->tileRadius = TileRadius;
	if (!fmemPushBlock(memory, &map->persistentMemory, fplMegaBytes(8), fmemPushFlags_Clear)) {
		return false; // Memory is too small
	}
	if (!fmemPushBlock(memory, &map->temporaryMemory, fplMegaBytes(8), fmemPushFlags_Clear)) {
		return false; // Memory is too small
	}
	return true;
}

extern bool MapAssign(Map *map, const Map *source) {
	if (map == fpl_null || source == fpl_null) {
		return false; // Invalid arguments
	}

	size_t totalTileCount = source->width * source->height;

	size_t requiredSize = totalTileCount * sizeof(uint32_t);
	if (requiredSize > map->persistentMemory.size) {
		return false; // Insufficient persistent memory
	}

	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;

	map->solidTiles = (uint32_t *)fmemPush(&map->persistentMemory, requiredSize, fmemPushFlags_Clear);
	if (map->solidTiles == fpl_null) {
		return false; // Insufficient persistent memory (allocation failed, even though the required size was validated beforehand)
	}

	map->width = source->width;
	map->height = source->height;
	map->origin = source->origin;

	for (size_t tileIndex = 0; tileIndex < totalTileCount; ++tileIndex) {
		map->solidTiles[tileIndex] = source->solidTiles[tileIndex];
	}

	return true;
}

extern bool MapFindPositionByTile(const Map *map, const uint32_t type, Vec2i *outTilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null || outTilePos == fpl_null) {
		return false; // Invalid arguments
	}
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			uint32_t tile = MapGetTile(map, V2iInit(x, y));
			if (tile == type)
			{
				*outTilePos = V2iInit(x, y);
				return true;
			}
		}
	}
	return false;
}