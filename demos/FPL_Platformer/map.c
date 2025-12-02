#include "map.h"

extern bool MapInit(fmemMemoryBlock *memory, Map *map) {
	if (memory == fpl_null || map == fpl_null) {
		return false; // Invalid arguments
	}
	fplClearStruct(map);
	map->tileSize = TileSize;
	map->tileRadius = TileRadius;
	if (!fmemPushBlock(memory, &map->temporaryMemory, MAP_MEMORY_TRANSIENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for map transient memory
	}
	if (!fmemPushBlock(memory, &map->persistentMemory, MAP_MEMORY_PERSISTENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for map persistent memory
	}
	return true;
}

extern void MapClear(Map *map) {
	if (!MapIsValid(map)) {
		return;
	}
	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;
	map->width = map->height = 0;
	map->origin = V2iInit(0, 0);
	map->maxBounds.min = map->maxBounds.max = V2fInit(0.0f, 0.0f);
	map->solidTiles = fpl_null;
}

fpl_internal bool MapTilePosIsObstacleLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool result = MapTileTypeIsObstacle(tile.type);
	return result;
}

fpl_internal bool MapTilePosIsMaskableLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool isSolid = tile.type == TileType_Solid;
	if (isSolid) {
		if (local.x == 0 || local.x == widthMinusOne || local.y == 0 || local.y == heightMinusOne) {
			return false; // Outside border is not maskable, treated as non-visible
		}
		return true;
	}
	return false;
}

// Gets the tile area mask from the specified map and local position
fpl_internal TileAreaMask MapGetTileAreaMaskLocal(const Map *map, const int widthMinusOne, const heightMinusOne, const Vec2i local) {
	if (!MapIsValid(map)) {
		return TileAreaMask_None;
	}

	TileType centerType = MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, local);

	// NOTE(final): Map is defined as top-down in memory, so up is negative and down is positive
	Vec2i topLeft = V2iAdd(local, V2iInit(-1, -1));
	Vec2i topCenter = V2iAdd(local, V2iInit(0, -1));
	Vec2i topRight = V2iAdd(local, V2iInit(1, -1));
	Vec2i leftSide = V2iAdd(local, V2iInit(-1, 0));
	Vec2i rightSide = V2iAdd(local, V2iInit(1, 0));
	Vec2i bottomLeft = V2iAdd(local, V2iInit(-1, 1));
	Vec2i bottomCenter = V2iAdd(local, V2iInit(0, 1));
	Vec2i bottomRight = V2iAdd(local, V2iInit(1, 1));

	bool t = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, topCenter));
	bool l = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, leftSide));
	bool b = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, bottomCenter));
	bool r = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, rightSide));
	bool tl = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, topLeft));
	bool tr = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, topRight));
	bool bl = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, bottomLeft));
	bool br = (centerType == MapGetTileTypeLocal(map, widthMinusOne, heightMinusOne, bottomRight));

	// If surrounding edges aren't set, then corners must be false
	// This reduces from 256 combinations to 48
	if (!(t && l)) {
		tl = false;
	}
	if (!(t && r)) {
		tr = false;
	}
	if (!(b && l)) {
		bl = false;
	}
	if (!(b && r)) {
		br = false;
	}

	// Compute bitmask
	TileAreaMask result = TileAreaMask_None;
	if (t)
		result |= TileAreaMask_TopCenter;
	if (tr)
		result |= TileAreaMask_TopRight;
	if (r)
		result |= TileAreaMask_RightSide;
	if (br)
		result |= TileAreaMask_BottomRight;
	if (b)
		result |= TileAreaMask_BottomCenter;
	if (bl)
		result |= TileAreaMask_BottomLeft;
	if (l)
		result |= TileAreaMask_LeftSide;
	if (tl)
		result |= TileAreaMask_TopLeft;

	return result;
}

fpl_internal void MapUpdateAllAreaMasks(Map *map) {
	if (!MapIsValid(map)) {
		return;
	}
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i local = V2iInit(x, y);
			tile->areaMask = MapGetTileAreaMaskLocal(map, widthMinusOne, heightMinusOne, local);
	}
	}
}

fpl_internal void MapAutoSetAllGhostTiles(Map *map) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return;
	}

	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;

	// Clean ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (tile->type != TileType_Ghost) {
				continue;
			}
			if (!MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y - 1)) || !MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y + 1))) {
				tile->type = TileType_None;
			}
		}
	}

	// Set ghost tiles
	for (uint32_t y = 0; y < map->height; ++y) {
		for (uint32_t x = 0; x < map->width; ++x) {
			Tile *tile = &map->solidTiles[y * map->width + x];
			Vec2i p = V2iInit(x, y);
			if (MapTileTypeIsObstacle(tile->type)) {
				continue;
			}
			if (MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y - 1)) && MapTilePosIsObstacleLocal(map, widthMinusOne, heightMinusOne, V2iInit(p.x, p.y + 1))) {
				tile->type = TileType_Ghost;
			}
		}
	}
}

fpl_internal void *_MapAllocateJSONMemoryInternal(void *userData, const size_t size) {
	if (userData == fpl_null || size == 0) {
		return fpl_null;
	}
	fmemMemoryBlock *transientBlock = (fmemMemoryBlock *)userData;
	void *result = fmemPush(transientBlock, size, fmemPushFlags_Clear);
	return result;
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

	Vec2f mapArea = V2fInit(definition->width * map->tileSize.w, definition->height * map->tileSize.h);
	Vec2f mapBottomLeft = V2fInit(map->origin.x * map->tileSize.w, map->origin.y * map->tileSize.h);
	AABB2f maxBounds = AABB2fInitFromBottomLeft(mapBottomLeft, mapArea);

	map->width = definition->width;
	map->height = definition->height;
	map->origin = V2iInit(0, 0);
	map->maxBounds = maxBounds;

	for (uint32_t tileIndex = 0; tileIndex < totalTileCount; ++tileIndex) {
		TileType type = definition->tiles[tileIndex];
		map->solidTiles[tileIndex].id = MapTileIDStart + tileIndex;
		map->solidTiles[tileIndex].type = type;
	}

	MapAutoSetAllGhostTiles(map);

	MapUpdateAllAreaMasks(map);

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

	MapUpdateAllAreaMasks(map);

	return true;
}

extern bool MapFindPositionByTile(const Map *map, const TileType type, Vec2i *outTilePos) {
	if (!MapIsValid(map) || outTilePos == fpl_null) {
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

extern bool MapResizeToTilePos(Map *map, const Vec2i tilePos) {
	if (!MapIsValid(map)) {
		return false; // Invalid arguments
	}

	int mapWidthMinusOne = map->width - 1;
	int mapHeightMinusOne = map->height - 1;

	Vec2i oldOrigin = map->origin;

	Vec2i newOrigin = map->origin;

	Vec2i append = V2iInit(0, 0);

	if (tilePos.x < oldOrigin.x) {
		int xcount = oldOrigin.x - tilePos.x;
		fplAssert(xcount > 0);
		append.w += xcount;
		newOrigin.x -= xcount;
	} else if ((tilePos.x - oldOrigin.x) > mapWidthMinusOne) {
		int xcount = (tilePos.x - oldOrigin.x) - mapWidthMinusOne;
		fplAssert(xcount > 0);
		append.w += xcount;
	}

	if (tilePos.y < oldOrigin.y) {
		int ycount = oldOrigin.y - tilePos.y;
		fplAssert(ycount > 0);
		append.h += ycount;
		newOrigin.y -= ycount;
	} else if ((tilePos.y - oldOrigin.y) > mapHeightMinusOne) {
		int ycount = (tilePos.y - oldOrigin.y) - mapHeightMinusOne;
		fplAssert(ycount > 0);
		append.h += ycount;
	}

	if (append.w == 0 && append.h == 0) {
		return false; // Nothnig to resize
	}

	fplAssert(append.w >= 0 || append.h >= 0);

	Vec2i oldMapSize = V2iInit(map->width, map->height);
	Vec2i newMapSize = V2iInit(map->width + append.w, map->height + append.h);

	map->temporaryMemory.used = 0;

	fmemMemoryBlock tempBlock;
	fmemBeginTemporary(&map->temporaryMemory, &tempBlock);

	size_t requiredOldSize = oldMapSize.w * oldMapSize.h * sizeof(Tile);
	fplAssert(requiredOldSize <= tempBlock.size);

	Tile *oldTiles = (Tile *)fmemPush(&tempBlock, requiredOldSize, fmemPushFlags_None);
	fplMemoryCopy(map->solidTiles, requiredOldSize, oldTiles);

	map->persistentMemory.used = 0;
	size_t requiredNewSize = newMapSize.w * newMapSize.h * sizeof(Tile);
	fplAssert(requiredNewSize <= map->persistentMemory.size);

	Vec2f mapArea = V2fInit(newMapSize.w * map->tileSize.w, newMapSize.h * map->tileSize.h);
	Vec2f mapBottomLeft = V2fInit(newOrigin.x * map->tileSize.w, newOrigin.y * map->tileSize.h);
	AABB2f maxBounds = AABB2fInitFromBottomLeft(mapBottomLeft, mapArea);

	// Create new tiles and copy old tiles over it
	map->width = newMapSize.w;
	map->height = newMapSize.h;
	map->origin = newOrigin;
	map->maxBounds = maxBounds;
	map->solidTiles = (Tile *)fmemPush(&map->persistentMemory, requiredNewSize, fmemPushFlags_Clear);

	Vec2i offsetFromOldToNew = V2iSub(oldOrigin, newOrigin);

	if (offsetFromOldToNew.y > 0) {
		offsetFromOldToNew.y = 0;
	} else if (append.h > 0) {
		offsetFromOldToNew.y = append.h;
	}

	for (int y = 0; y < oldMapSize.h; ++y) {
		for (int x = 0; x < oldMapSize.w; ++x) {
			Vec2i oldLocal = V2iInit(x, y);
			Vec2i newLocal = V2iAdd(oldLocal, offsetFromOldToNew);
			fplAssert(newLocal.x >= 0 && newLocal.x < newMapSize.w);
			fplAssert(newLocal.y >= 0 && newLocal.y < newMapSize.h);
			map->solidTiles[newLocal.y * newMapSize.w + newLocal.x] = oldTiles[oldLocal.y * oldMapSize.w + oldLocal.x];
		}
	}

	// Re-assign id's
	for (int y = 0; y < newMapSize.h; ++y) {
		for (int x = 0; x < newMapSize.w; ++x) {
			uint32_t index = y * newMapSize.w + x;
			map->solidTiles[index].id = MapTileIDStart + index;
		}
	}

	MapUpdateAllAreaMasks(map);

	fmemEndTemporary(&map->temporaryMemory);

	return true;
}