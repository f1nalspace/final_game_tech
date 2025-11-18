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

//
// Tile types
//
#define TileType_PlayerPosition 2

typedef struct Map {
	// Memory handling
	fmemMemoryBlock temporaryMemory;
	fmemMemoryBlock persistentMemory;

	// The origin in tile coordinate
	Vec2i origin;

	// The tile world size and radius
	Vec2f tileSize;
	Vec2f tileRadius;

	// The 1D tile data (width * height)
	uint32_t *solidTiles;

	// The width in tiles
	uint32_t width;

	// The height in tiles
	uint32_t height;
} Map;

// Converts the specified world position into a tile position
fpl_inline Vec2i MapWorldCoordsToTile(const Map *map, const Vec2f worldPos) {
	float wx = worldPos.x - (map->origin.x * map->tileSize.w);
	float wy = worldPos.y - (map->origin.y * map->tileSize.h);

	// Adjustment for negative coordinates
	float rx = 0.0f;
	float ry = 0.0f;
	if (wx < 0)
		rx = -1.0f;
	if (wy < 0)
		ry = -1.0f;

	int x = (int)(wx / map->tileSize.w + rx);
	int y = (int)(wy / map->tileSize.h + ry);

	return V2iInit(x, y);
}

// Converts the specified tile position into a world position
fpl_inline Vec2f MapTileCoordsToWorld(const Map *map, const Vec2i tilePos) {
	float x = ((float)(tilePos.x + map->origin.x) * map->tileSize.w);
	float y = ((float)(tilePos.y + map->origin.y) * map->tileSize.h);
	return V2fInit(x, y);
}

// Gets a tile by the specified tile position, note that Y of the tile position is converted into tile space
fpl_inline uint32_t MapGetTile(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0 || map->solidTiles == fpl_null) {
		return UINT32_MAX;
	}
	int invY = map->height - 1 - tilePos.y;
	if (tilePos.x < 0 || invY < 0 || tilePos.x > ((int)map->width - 1) || invY > ((int)map->height - 1)) {
		return UINT32_MAX;
	}
	uint32_t result = map->solidTiles[invY * map->width + tilePos.x];
	return result;
}

// Returns true if the specified tile position is inside the entire tile area
fpl_inline bool MapIsTileInside(const Map *map, const Vec2i tilePos) {
	if (map == fpl_null) {
		return false;
	}
	bool result = (tilePos.x >= 0 && tilePos.x < (int)map->width) && (tilePos.y >= 0 && tilePos.y < (int)map->height);
	return result;
}

// Returns true if the specified tile is an obstacle or not
fpl_inline bool MapIsObstacle(const Map *map, const uint32_t tile) {
	// @TODO(final): Obstacle tile mapping!
	bool result = tile == 1;
	return result;
}

fpl_inline AABB2f MapCreateTileAABB(const Map *map, const Vec2i tilePos) {
	Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
	AABB2f result = AABB2fInitFromCenter(V2fAdd(worldPos, map->tileRadius), map->tileRadius);
	return result;
}

// Finds the first tile position from the specified tile type and returns true if found, false otherwise
extern bool MapFindPositionByTile(const Map *map, const uint32_t type, Vec2i *outTilePos);

#endif // MAP_H
