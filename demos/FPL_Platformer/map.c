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

extern void MapClear(Map *map) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return;
	}
	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;
	map->width = map->height = 0;
	map->origin = V2iInit(0, 0);
	map->solidTiles = fpl_null;
}

static bool MapTilePosIsObstacleInternal(const Map *map, const int localX, const int localY) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return false;
	}
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (localX < 0 || localX > widthMinusOne || localY < 0 || localY > heightMinusOne) {
		return false;
	}
	Tile tile = map->solidTiles[localY * map->width + localX];
	bool result = MapTileTypeIsObstacle(map, tile.type);
	return result;
}

extern void MapAutoSetAllGhostTiles(Map *map) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return;
	}

	// Clean ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (tile->type != TileType_Ghost) {
				continue;
			}
			if (!MapTilePosIsObstacleInternal(map, p.x, p.y - 1) || !MapTilePosIsObstacleInternal(map, p.x, p.y + 1)) {
				tile->type = TileType_None;
			}
		}
	}

	// Set ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (MapTileTypeIsObstacle(map, tile->type)) {
				continue;
			}
			if (MapTilePosIsObstacleInternal(map, p.x, p.y - 1) && MapTilePosIsObstacleInternal(map, p.x, p.y + 1)) {
				tile->type = TileType_Ghost;
			}
		}
	}
}

extern bool MapAssign(Map *map, const MapDefinition *definition) {
	if (map == fpl_null || definition == fpl_null) {
		return false; // Invalid arguments
	}

	uint32_t totalTileCount = definition->width * definition->height;

	size_t requiredSize = totalTileCount * sizeof(Tile);
	if (requiredSize > map->persistentMemory.size) {
		return false; // Insufficient persistent memory
	}

	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;

	map->solidTiles = (Tile *)fmemPush(&map->persistentMemory, requiredSize, fmemPushFlags_Clear);
	if (map->solidTiles == fpl_null) {
		return false; // Insufficient persistent memory (allocation failed, even though the required size was validated beforehand)
	}

	map->width = definition->width;
	map->height = definition->height;
	map->origin = V2iInit(0, 0);

	for (uint32_t tileIndex = 0; tileIndex < totalTileCount; ++tileIndex) {
		TileType type = definition->tiles[tileIndex];
		map->solidTiles[tileIndex].id = MapTileIDStart + tileIndex;
		map->solidTiles[tileIndex].type = type;
	}

	MapAutoSetAllGhostTiles(map);

	return true;
}

extern Tile MapGetTile(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null) {
		return TileEmpty;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return TileEmpty;
	}
	Tile result = map->solidTiles[local.y * map->width + local.x];
	return result;
}

extern bool MapSetTileType(Map *map, const Vec2i tilePos, const TileType type) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);

	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return false;
	}

	Tile *curTile = &map->solidTiles[local.y * map->width + local.x];
	curTile->type = type;

	MapAutoSetAllGhostTiles(map);

	return true;
}

extern bool MapFindPositionByTile(const Map *map, const TileType type, Vec2i *outTilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null || outTilePos == fpl_null) {
		return false; // Invalid arguments
	}
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile tile = map->solidTiles[y * map->width + x];
			if (tile.type == type)
			{
				Vec2i local = V2iInit(x, y);
				*outTilePos = MapLocalTilePosToPublicTilePos(map, local);
				return true;
			}
		}
	}
	return false;
}