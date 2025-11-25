#ifndef ASSETS_H
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

// Rules for Border 3x3 (Simple indices rules)
// 
// Column == 0: TILESET_A_BORDER_3x3_SIDE_LEFT
// Column == WIDTH - 1: TILESET_A_BORDER_3x3_SIDE_RIGHT
// Row == 0: TILESET_A_BORDER_3x3_TOP_MIDDLE
// Row == HEIGHT - 1:TILESET_A_BORDER_3x3_BOTTOM_MIDDLE
// Row == 0 && Column == 0: TILESET_A_BORDER_3x3_TOP_LEFT
// Row == 0 && Column == WIDTH - 1: TILESET_A_BORDER_3x3_TOP_RIGHT
// Row == HEIGHT - 1 && Column == 0: TILESET_A_BORDER_3x3_BOTTOM_LEFT
// Row == HEIGHT - 1 && Column == WIDTH - 1: TILESET_A_BORDER_3x3_BOTTOM_RIGHT

// Tileset A Border 3x3
#define TILESET_A_BORDER_3x3_OFFSET V2I(7, 0)
#define TILESET_A_BORDER_3x3_TOP_LEFT V2I(0, 0)
#define TILESET_A_BORDER_3x3_TOP_MIDDLE V2I(1, 0)
#define TILESET_A_BORDER_3x3_TOP_RIGHT V2I(2, 0)
#define TILESET_A_BORDER_3x3_SIDE_LEFT V2I(0, 1)
#define TILESET_A_BORDER_3x3_MIDDLE V2I(1, 1)
#define TILESET_A_BORDER_3x3_SIDE_RIGHT V2I(2, 1)
#define TILESET_A_BORDER_3x3_BOTTOM_LEFT V2I(0, 2)
#define TILESET_A_BORDER_3x3_BOTTOM_MIDDLE V2I(1, 2)
#define TILESET_A_BORDER_3x3_BOTTOM_RIGHT V2I(2, 2)

// Tileset A Collidable 3x3
#define TILESET_A_COLLIDABLE_3x3_OFFSET V2I(0, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_LEFT V2I(0, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_MIDDLE V2I(1, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_RIGHT V2I(2, 0)
#define TILESET_A_COLLIDABLE_3x3_SIDE_LEFT V2I(0, 1)
#define TILESET_A_COLLIDABLE_3x3_MIDDLE V2I(1, 1)
#define TILESET_A_COLLIDABLE_3x3_SIDE_RIGHT V2I(2, 1)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_LEFT V2I(0, 2)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_MIDDLE V2I(1, 2)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_RIGHT V2I(2, 2)

// Tileset A Collidable 1x3
#define TILESET_A_COLLIDABLE_1x3_OFFSET V2I(3, 0)
#define TILESET_A_COLLIDABLE_1x3_TOP V2I(0, 0)
#define TILESET_A_COLLIDABLE_1x3_MIDDLE V2I(0, 1)
#define TILESET_A_COLLIDABLE_1x3_BOTTOM V2I(0, 2)

// Tileset A Collidable 3x1
#define TILESET_A_COLLIDABLE_3x1_OFFSET V2I(4, 0)
#define TILESET_A_COLLIDABLE_3x1_LEFT V2I(0, 0)
#define TILESET_A_COLLIDABLE_3x1_CENTER V2I(1, 0)
#define TILESET_A_COLLIDABLE_3x1_RIGHT V2I(2, 0)

// Tileset A Collidable 1x1
#define TILESET_A_COLLIDABLE_1x1_A V2I(4, 1)
#define TILESET_A_COLLIDABLE_1x1_B V2I(5, 1)

// Tileset A Collidable 3x0.5
#define TILESET_A_COLLIDABLE_3x0_5_OFFSET V2I(4, 2)
#define TILESET_A_COLLIDABLE_3x0_5_LEFT V2I(0, 0)
#define TILESET_A_COLLIDABLE_3x0_5_CENTER V2I(1, 0)
#define TILESET_A_COLLIDABLE_3x0_5_RIGHT V2I(2, 0)

// Tileset A Collidable 1x1 (Invalid)
#define TILESET_A_COLLIDABLE_1x1_INVALID V2I(11, 0)

typedef struct {
	// tileset offset
    Vec2i offset;
	// tile position within tileset
    Vec2i pos;
    // Is rule valid
    fpl_b32 valid;
} TileRule;

#define TILERULE_SINGLE_BLOCK {V2I(0, 0), TILESET_A_COLLIDABLE_1x1_A, true}

static const TileRule gMapTileRules[512] = {
    // Isolated
    [0b00000000] = TILERULE_SINGLE_BLOCK,

    // Single blocks diagonal connected
    [0b00000001] = TILERULE_SINGLE_BLOCK,
    [0b10000000] = TILERULE_SINGLE_BLOCK,
    [0b00000100] = TILERULE_SINGLE_BLOCK,
    [0b00000101] = TILERULE_SINGLE_BLOCK,
    [0b00100000] = TILERULE_SINGLE_BLOCK,
    [0b00100100] = TILERULE_SINGLE_BLOCK,
    [0b10100000] = TILERULE_SINGLE_BLOCK,
    [0b10100100] = TILERULE_SINGLE_BLOCK,
    [0b00100001] = TILERULE_SINGLE_BLOCK,
    [0b00100101] = TILERULE_SINGLE_BLOCK,
    [0b10100101] = TILERULE_SINGLE_BLOCK,
    [0b10000100] = TILERULE_SINGLE_BLOCK,
    [0b10100001] = TILERULE_SINGLE_BLOCK,
    [0b10000101] = TILERULE_SINGLE_BLOCK,

    // 3x3 with center
    [0b11111111] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_MIDDLE, true },          // center
    [0b11010000] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_TOP_LEFT, true },        // top-left corner
    [0b11111000] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_TOP_MIDDLE, true },      // top-middle corner
    [0b01101000] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_TOP_RIGHT, true },       // top-right corner
    [0b11010110] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_SIDE_LEFT, true },       // side-left
    [0b00010110] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_BOTTOM_LEFT, true },     // bottom-left
    [0b00011111] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_BOTTOM_MIDDLE, true },   // bottom-middle
    [0b00001011] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_BOTTOM_RIGHT, true },    // bottom-right
    [0b01101011] = { TILESET_A_COLLIDABLE_3x3_OFFSET, TILESET_A_COLLIDABLE_3x3_SIDE_RIGHT, true },      // side-right

    // 3x3 without center
    [0b01010000] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_TOP_LEFT, true },                // top-left
    [0b10111000] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_TOP_MIDDLE, true },              // top-middle
    [0b01001000] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_TOP_RIGHT, true },               // top-right
    [0b01100011] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_SIDE_RIGHT, true },              // side-right
    [0b11000110] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_SIDE_LEFT, true },               // side-left
    [0b00010010] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_BOTTOM_LEFT, true },             // bottom-left
    [0b00001010] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_BOTTOM_RIGHT, true },            // bottom-right
    [0b00011101] = { TILESET_A_BORDER_3x3_OFFSET, TILESET_A_BORDER_3x3_BOTTOM_MIDDLE, true },           // bottom-middle

    // 3x1 Horizontal line
    [0b00011000] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_CENTER, true },          // left+right
    [0b00010000] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_LEFT, true },            // left only
    [0b00001000] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_RIGHT, true },           // right only

    // 2x1 Horizontal line with diagonal connections
    [0b00110000] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_LEFT, true },            // left, left-bottom diagonal
    [0b00010001] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_LEFT, true },            // left, left-top diagonal
    [0b00110001] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_LEFT, true },            // left, left-bottom diagonal + left-top diagonal
    [0b10001000] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_RIGHT, true },           // right, right-bottom diagonal
    [0b00001100] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_RIGHT, true },           // right, right-top diagonal
    [0b10001100] = { TILESET_A_COLLIDABLE_3x1_OFFSET, TILESET_A_COLLIDABLE_3x1_RIGHT, true },           // right, right-bottom diagonal + right-top diagonal

    // 1x3 Vertical line
    [0b01000010] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_MIDDLE, true },          // top+bottom
    [0b01000000] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_TOP, true },             // top only
    [0b00000010] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_BOTTOM, true },          // bottom only

    // 1x2 Vertical line with diagonal connections
    [0b01000100] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_TOP, true },             // top, right-top diagonal
    [0b01000001] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_TOP, true },             // top, left-top diagonal
    [0b01000101] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_TOP, true },             // top, left-top diagonal + right-top diagonal
    [0b00100010] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_BOTTOM, true },          // bottom, left-bottom diagonal
    [0b10000010] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_BOTTOM, true },          // bottom, right-bottom diagonal
    [0b10100010] = { TILESET_A_COLLIDABLE_1x3_OFFSET, TILESET_A_COLLIDABLE_1x3_BOTTOM, true },          // bottom, left-bottom diagonal + right-bottom diagonal
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
