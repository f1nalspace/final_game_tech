#ifndef MAP_H
#define MAP_H

#include <final_platform_layer.h>

#include <final_memory.h>

#include <final_math.h>

#include <final_geometry.h>

#include "common.h"

// Size of the map transient memory block
#define MAP_MEMORY_TRANSIENT_SIZE fplMegaBytes(4)

// Size of the map persistent memory block
#define MAP_MEMORY_PERSISTENT_SIZE fplMegaBytes(8)

//
// Fixed tile definition in world space
//
#define TileWidth 32.0f
#define TileHeight 32.0f
#define TileSize V2fInit(TileWidth, TileHeight)
#define TileRadius V2fInit(TileWidth * 0.5f, TileHeight * 0.5f)

// Tile type enumeration (16-bit)
typedef enum TileType {
	TileType_None = 0,
	TileType_Solid = 1,
	TileType_Ghost = 2,
	TileType_PlayerStart = 3,
	TileType_Spike = 4,
	TileType_Trampoline = 5,
	TileType_Platform = 6,
} TileType;

// Tile Area Mask (Number represents in bit shift, 1 >> value)
//
// |---|---|---|
// | 7 | 0 | 1 |
// |---|---|---|
// | 6 |   | 2 |
// |---|---|---|
// | 5 | 4 | 3 |
// |---|---|---|

// Tile area mask (8-bit, stored as 16-bit)
typedef enum TileAreaMask {
	// None
	TileAreaMask_None = 0,
	// Top Center
	TileAreaMask_TopCenter = 1 << 0,
	// Top Right
	TileAreaMask_TopRight = 1 << 1,
	// Right Side
	TileAreaMask_RightSide = 1 << 2,
	// Bottom Right
	TileAreaMask_BottomRight = 1 << 3,
	// Bottom Center
	TileAreaMask_BottomCenter = 1 << 4,
	// Bottom Left
	TileAreaMask_BottomLeft = 1 << 5,
	// Left Side
	TileAreaMask_LeftSide = 1 << 6,
	// Top Left
	TileAreaMask_TopLeft = 1 << 7,
} TileAreaMask;

typedef struct Tile {
	struct {
		// Tile type
		TileType type : 16;
		// Area mask
		TileAreaMask areaMask : 16;
	};
	// Positive tile id
	uint32_t id;
} Tile;

// Convert a TileAreaMask (with centroid removed at bit 4) into 8-bit adjacency mask.
// Bits 0–3 are kept as-is, bit 4 (centroid) is discarded,
// bits 5–8 are shifted down into positions 4–7.
fpl_inline uint8_t TileAreaMaskToU8(const TileAreaMask mask) {
	// Clear the centroid bit 4
	uint16_t mask16 = (mask & 0b0000000111101111);

    uint8_t result = 0;

    // Keep bits 0–3
    result |= (mask16 & 0b0000000000001111); // TopLeft, TopCenter, TopRight, Left

    // Shift bits 5–8 down by 1
    result |= ((mask16 & 0b0000000111100000) >> 1); // Right, BottomLeft, BottomCenter, BottomRight

    return result;
}

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

typedef enum MapLayerType {
	MapLayerType_None = 0,
	MapLayerType_IntGrid,
	MapLayerType_Entities,
} MapLayerType;

typedef struct MapIntGridValue {
	int value;
	String identifier;
} MapIntGridValue;

typedef struct MapEntity {
	String identifier;
} MapEntity;

typedef struct MapLayer {
	MapLayerType type;
	uint32_t uid;
	uint32_t count;
	union {
		MapIntGridValue *intGridValues;
	};
} MapLayer;

typedef struct MapDefinition {
	// Map unique id
	IID16 iid;
	// Map name
	String name;
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

	// Maximum map bounds in world coordinates
	AABB2f maxBounds;

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

fpl_inline bool MapIsValid(const Map *map) {
	if (map == fpl_null) {
		return false;
	}
	bool result = map->width > 0 && map->height > 0 && map->solidTiles != fpl_null;
	return result;
}

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

fpl_inline bool MapIsTileOutsideLocal(const int widthMinusOne, const int heightMinusOne, Vec2i local) {
	return local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne;
}

// Returns true if the specified tile is visible or not (Ghost tiles are not visible!)
fpl_inline bool MapTileTypeIsVisible(const TileType tileType) {
	switch (tileType) {
		case TileType_Solid:
			return true;
		default:
			return false;
	}
}

// Returns true if the specified tile is an obstacle or not
fpl_inline bool MapTileTypeIsObstacle(const TileType tileType) {
	switch (tileType) {
		case TileType_Solid:
		case TileType_Ghost:
			return true;
		default:
			return false;
	}
}

fpl_inline TileType MapGetTileTypeLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return TileType_None;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	return tile.type;
}

fpl_inline bool MapTileIsObstacleLocal(const Map *map, const int widthMinusOne, const int heightMinusOne, Vec2i local) {
	if (!MapIsValid(map) || MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool result = MapTileTypeIsObstacle(tile.type);
	return result;
}

// Returns true if the specified public tile position is inside the entire tile area
fpl_inline bool MapIsTileInside(const Map *map, const Vec2i tilePos) {
	if (!MapIsValid(map)) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (MapIsTileOutsideLocal(widthMinusOne, heightMinusOne, local)) {
		return false;
	}
	return true;
}


// Returns true if the specified tile is an obstacle or not
fpl_inline bool MapTilePosIsObstacle(const Map *map, const Vec2i tilePos) {
	if (!MapIsValid(map)) {
		return false;
	}
	Vec2i local = MapPublicTilePosToLocalTilePos(map, tilePos);
	int widthMinusOne = map->width - 1;
	int heightMinusOne = map->height - 1;
	if (local.x < 0 || local.x > widthMinusOne || local.y < 0 || local.y > heightMinusOne) {
		return false;
	}
	Tile tile = map->solidTiles[local.y * map->width + local.x];
	bool result = MapTileTypeIsObstacle(tile.type);
	return result;
}

fpl_inline AABB2f MapCreateTileAABB(const Map *map, const Vec2i tilePos) {
	Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
	AABB2f result = AABB2fInitFromCenter(V2fAdd(worldPos, map->tileRadius), map->tileRadius);
	return result;
}

fpl_inline TileBounds MapGetTileBounds(const Map *map, const AABB2f *aabb) {
	Vec2i minTile = MapWorldCoordsToTile(map, aabb->min);
	Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(aabb->max, V2fInit(0.5f, 0.5f)));
	return TileBoundsInit(minTile, maxTile);
}

extern bool MapInit(fmemMemoryBlock *memory, Map *map);

extern void MapClear(Map *map);

extern bool MapDefinitionLoadFromFile(fmemMemoryBlock *transientMemory, fmemMemoryBlock *persistentMemory, const char *filePath, const char *levelName, MapDefinition *outDefinition);

extern bool MapAssign(Map *map, const MapDefinition *definition);

// Finds the first tile position from the specified tile type and returns true if found, false otherwise
extern bool MapFindPositionByTile(const Map *map, const TileType type, Vec2i *outTilePos);

extern bool MapResizeToTilePos(Map *map, const Vec2i tilePos);

#endif // MAP_H
