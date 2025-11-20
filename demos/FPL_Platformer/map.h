#ifndef MAP_H
#define MAP_H

#include <final_platform_layer.h>

#include <final_memory.h>

#include <final_math.h>

//
// Fixed tile definition in world space
//
#define TileWidth 32.0f
#define TileHeight 32.0f
#define TileSize V2fInit(TileWidth, TileHeight)
#define TileRadius V2fInit(TileWidth * 0.5f, TileHeight * 0.5f)

typedef enum TileType {
	TileType_None = 0,
	TileType_Solid = 1,
	TileType_Ghost = 2,
	TileType_PlayerStart = 3,
} TileType;

typedef struct Tile {
	// Tile type
	TileType type;
	// Positive tile id
	uint32_t id;
} Tile;

static const Tile gEmptyTile = fplZeroInit;

#define TileEmpty gEmptyTile

typedef struct TileBounds {
	Vec2i min;
	Vec2i max;
} TileBounds;

fpl_inline TileBounds TileBoundsInit(const Vec2i min, const Vec2i max) {
	TileBounds result = fplStructInit(TileBounds, min, max);
	return result;
}

typedef struct MapDefinition {
	// The 1D tile type array (width * height)
	TileType *tiles;
	// The width in tiles
	uint32_t width;
	// The height in tiles
	uint32_t height;
} MapDefinition;

typedef struct Map {
	// Temporary memory block
	fmemMemoryBlock temporaryMemory;

	// Persistent memory block
	fmemMemoryBlock persistentMemory;

	// The origin in tile coordinate
	Vec2i origin;

	// The tile world size and radius
	Vec2f tileSize;

	// Tile radius
	Vec2f tileRadius;

	// The 1D tile data (width * height)
	Tile *solidTiles;

	// The width in tiles
	uint32_t width;

	// The height in tiles
	uint32_t height;
} Map;

// Converts the public tile position into a local tile position
fpl_inline Vec2i MapPublicTilePosToLocalTilePos(const Map *map, const Vec2i publicTilePos) {
	Vec2i l = V2iSub(publicTilePos, map->origin);
	Vec2i result = V2iInit(l.x, map->height - 1 - l.y);
	return result;
}

// Converts the local tile position into a public tile position
fpl_inline Vec2i MapLocalTilePosToPublicTilePos(const Map *map, const Vec2i localTilePos) {
	Vec2i p = V2iInit(localTilePos.x, map->height - 1 - localTilePos.y);
	Vec2i result = V2iAdd(p, map->origin);
	return result;
}

// Converts the specified world position into a tile position
fpl_inline Vec2i MapWorldCoordsToTile(const Map *map, const Vec2f worldPos) {
	float wx = worldPos.x / map->tileSize.w;
	float wy = worldPos.y / map->tileSize.h;

	// Adjustment for negative world coordinates
	int rx = 0;
	int ry = 0;
	if (wx < 0) {
		rx = -1;
	}
	if (wy < 0) {
		ry = -1;
	}

	int x = (int)wx + rx;
	int y = (int)wy + ry;
	return V2iInit(x, y);
}

// Converts the specified tile position into a world position
fpl_inline Vec2f MapTileCoordsToWorld(const Map *map, const Vec2i tilePos) {
	float x = (float)(tilePos.x * map->tileSize.w);
	float y = (float)(tilePos.y * map->tileSize.h);
	return V2fInit(x, y);
}

// Gets a tile by the specified tile position, note that Y of the tile position is converted into tile space
extern Tile MapGetTile(const Map *map, const Vec2i tilePos);

// Overwrites a tile type to a fixed tile position, without resizing the map
extern bool MapSetTileType(Map *map, const Vec2i tilePos, const TileType type);

// Returns true if the specified tile position is inside the entire tile area
fpl_inline bool MapIsTileInside(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return false;
	}
	return true;
}

// Returns true if the specified tile is an obstacle or not
fpl_inline bool MapTileTypeIsObstacle(const Map *map, const TileType tileType) {
	if (map == fpl_null) {
		return false;
	}
	switch (tileType) {
		case TileType_Solid:
		case TileType_Ghost:
			return true;
		default:
			return false;
	}
}

// Returns true if the specified tile is an obstacle or not
fpl_inline bool MapTilePosIsObstacle(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool result = MapTileTypeIsObstacle(map, tile.type);
	return result;
}

fpl_inline AABB2f MapCreateTileAABB(const Map *map, const Vec2i tilePos) {
	Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
	AABB2f result = AABB2fInitFromCenter(V2fAdd(worldPos, map->tileRadius), map->tileRadius);
	return result;
}

fpl_inline TileBounds MapGetTileBounds(const Map *map, const AABB2f aabb) {
	Vec2i minTile = MapWorldCoordsToTile(map, aabb.min);
	Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(aabb.max, V2fInit(0.5f, 0.5f)));
	return TileBoundsInit(minTile, maxTile);
}

extern bool MapInit(fmemMemoryBlock *memory, Map *map);

extern void MapClear(Map *map);

extern bool MapAssign(Map *map, const MapDefinition *definition);

// Finds the first tile position from the specified tile type and returns true if found, false otherwise
extern bool MapFindPositionByTile(const Map *map, const TileType type, Vec2i *outTilePos);

extern void MapAutoSetAllGhostTiles(Map *map);

extern bool MapResizeToTilePos(Map *map, const Vec2i tilePos);

#endif // MAP_H
