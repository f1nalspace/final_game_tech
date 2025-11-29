#ifndef ASSETS_H
#define TILESET_1x3_VERTICAL_MIDDLE V2I(0, 1)
#define ASSETS_H

#include <final_platform_layer.h>
#include <final_memory.h>
#include <final_assets.h>
#include <final_render.h>
#include <final_math.h>

#include "map.h"

// Size of the assets transient memory block
#define GAME_ASSETS_MEMORY_TRANSIENT_SIZE fplMegaBytes(8)

// Size of the assets persistent memory block
#define GAME_ASSETS_MEMORY_PERSISTENT_SIZE fplMegaBytes(4)

typedef enum RuleNumberType {
	RuleNumberType_Ignore = 0,
	RuleNumberType_Value,
	RuleNumberType_First,
	RuleNumberType_Last,
} RuleNumberType;

typedef struct RuleNumber {
	RuleNumberType type;
	int value;
} RuleNumber;

typedef struct TileIndexRule {
	Vec2i pos;
	RuleNumber col;
	RuleNumber row;
} TileIndexRule;

typedef struct TileAreaRule {
	Vec2i pos;
	TileAreaMask areaMask;
	uint32_t padding;
} TileAreaRule;

// Tileset 3x3 Fill/Plus 
#define TILESET_3x3_FILL_OFFSET V2I(0, 0)
#define TILESET_3x3_FILL_TOP_LEFT V2I(0, 0)
#define TILESET_3x3_FILL_TOP_MIDDLE V2I(1, 0)
#define TILESET_3x3_FILL_TOP_RIGHT V2I(2, 0)
#define TILESET_3x3_FILL_SIDE_LEFT V2I(0, 1)
#define TILESET_3x3_FILL_CENTROID V2I(1, 1)
#define TILESET_3x3_FILL_SIDE_RIGHT V2I(2, 1)
#define TILESET_3x3_FILL_BOTTOM_LEFT V2I(0, 2)
#define TILESET_3x3_FILL_BOTTOM_MIDDLE V2I(1, 2)
#define TILESET_3x3_FILL_BOTTOM_RIGHT V2I(2, 2)

// Tileset 1x3 Vertical
#define TILESET_1x3_VERTICAL_OFFSET V2I(3, 0)
#define TILESET_1x3_VERTICAL_TOP V2I(0, 0)
#define TILESET_1x3_VERTICAL_BOTTOM V2I(0, 2)

// Tileset 3x1 Horizontal
#define TILESET_3x1_HORIZONTAL_OFFSET V2I(4, 0)
#define TILESET_3x1_HORIZONTAL_LEFT V2I(0, 0)
#define TILESET_3x1_HORIZONTAL_CENTER V2I(1, 0)
#define TILESET_3x1_HORIZONTAL_RIGHT V2I(2, 0)

// Tileset 1x1 Block
#define TILESET_1x1_SINGLE_A V2I(4, 1)
#define TILESET_1x1_SINGLE_B V2I(5, 1)

// Tileset 3x1 Horizontal Half
#define TILESET_3x1_HORIZONTAL_HALF_OFFSET V2I(4, 2)
#define TILESET_3x1_HORIZONTAL_HALF_LEFT V2I(0, 0)
#define TILESET_3x1_HORIZONTAL_HALF_CENTER V2I(1, 0)
#define TILESET_3x1_HORIZONTAL_HALF_RIGHT V2I(2, 0)

// Tileset 1x1 Invalid
#define TILESET_1x1_INVALID V2I(7, 7)

typedef struct {
	// tileset offset
    Vec2i offset;
	// tile position within tileset
    Vec2i pos;
    // Is rule valid
    fpl_b32 valid;
} TileRule;

#define TILERULE_SINGLE_BLOCK {V2I(0, 0), TILESET_1x1_SINGLE_A, true}

static const TileRule gMapTileRules[256] = {
    // Isolated
    [0b00000000] = TILERULE_SINGLE_BLOCK,

	// Vertical line
    [0b00000001] = {TILESET_1x3_VERTICAL_OFFSET, TILESET_1x3_VERTICAL_BOTTOM, true},		// 1x3 Bottom
	[0b00010001] = {TILESET_1x3_VERTICAL_OFFSET, TILESET_1x3_VERTICAL_MIDDLE, true},		// 1x3 Middle
	[0b00010000] = {TILESET_1x3_VERTICAL_OFFSET, TILESET_1x3_VERTICAL_TOP, true},			// 1x3 Top

	// Horizontal line
	[0b00000100] = {TILESET_3x1_HORIZONTAL_OFFSET, TILESET_3x1_HORIZONTAL_LEFT, true},		// 3x1 Left
	[0b01000100] = {TILESET_3x1_HORIZONTAL_OFFSET, TILESET_3x1_HORIZONTAL_CENTER, true},	// 3x1 Center
	[0b01000000] = {TILESET_3x1_HORIZONTAL_OFFSET, TILESET_3x1_HORIZONTAL_RIGHT, true},		// 3x1 Right

	// Edges open
	[0b00010100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_LEFT, true},				// Top-left
	[0b01010000] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_RIGHT, true},				// Top-right
	[0b01000001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_RIGHT, true},			// Bottom-right
	[0b00000101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_LEFT, true},			// Bottom-left

	// Edges closed
	[0b00011100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_LEFT, true},				// Top-left
	[0b01110000] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_RIGHT, true},				// Top-right
	[0b11000001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_RIGHT, true},			// Bottom-right
	[0b00000111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_LEFT, true},			// Bottom-left

	// Middle pieces
	[0b01111100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_MIDDLE, true},			// Top
	[0b11110001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_RIGHT, true},			// Right
	[0b11000111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_MIDDLE, true},			// Bottom
	[0b00011111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_LEFT, true},				// Left

	// T-Shapes / Plus
	[0b00010101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_LEFT, true},				// To the Right
	[0b00011101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_LEFT, true},				// To the Right
	[0b00010111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_LEFT, true},				// To the Right
	[0b01010001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_RIGHT, true},			// To the Left
	[0b01110001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_RIGHT, true},			// To the Left
	[0b11010001] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_SIDE_RIGHT, true},			// To the Left
	[0b01010101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11111101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01110101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11010101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01010111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01011101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01111111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01111101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11010111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01011111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11110101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11011111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11110111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b11011101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Plus Centroid
	[0b01000101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_MIDDLE, true},			// To the Top
	[0b01000111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_MIDDLE, true},			// To the Top
	[0b11000101] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_BOTTOM_MIDDLE, true},			// To the Top
	[0b01010100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_MIDDLE, true},			// To the Bottom
	[0b01011100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_MIDDLE, true},			// To the Bottom
	[0b01110100] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_TOP_MIDDLE, true},			// To the Bottom

	[0b11111111] = {TILESET_3x3_FILL_OFFSET, TILESET_3x3_FILL_CENTROID, true},				// Filled Centroid
};

typedef struct GameAssets {
	fmemMemoryBlock transientMemory;
	fmemMemoryBlock persistentMemory;
	FontAsset consoleFont;
	TextureAsset tilesetTexture;
	char dataPath[1024];
} GameAssets;

fpl_extern void AssetsFree(GameAssets *assets);
fpl_extern bool AssetsInit(fmemMemoryBlock *memory, GameAssets *assets);
fpl_extern bool AssetsLoad(RenderState *renderState, GameAssets *assets);

#endif // ASSETS_H
