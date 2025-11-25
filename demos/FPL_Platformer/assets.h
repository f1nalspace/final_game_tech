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
#define TILESET_A_BORDER_3x3_OFFSET V2iInit(7, 0)
#define TILESET_A_BORDER_3x3_TOP_LEFT V2iInit(0, 0)
#define TILESET_A_BORDER_3x3_TOP_MIDDLE V2iInit(1, 0)
#define TILESET_A_BORDER_3x3_TOP_RIGHT V2iInit(2, 0)
#define TILESET_A_BORDER_3x3_SIDE_LEFT V2iInit(0, 1)
#define TILESET_A_BORDER_3x3_MIDDLE V2iInit(1, 1)
#define TILESET_A_BORDER_3x3_SIDE_RIGHT V2iInit(2, 1)
#define TILESET_A_BORDER_3x3_BOTTOM_LEFT V2iInit(0, 2)
#define TILESET_A_BORDER_3x3_BOTTOM_MIDDLE V2iInit(1, 2)
#define TILESET_A_BORDER_3x3_BOTTOM_RIGHT V2iInit(2, 2)

// Tileset A Collidable 3x3
#define TILESET_A_COLLIDABLE_3x3_OFFSET V2iInit(0, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_LEFT V2iInit(0, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_MIDDLE V2iInit(1, 0)
#define TILESET_A_COLLIDABLE_3x3_TOP_RIGHT V2iInit(2, 0)
#define TILESET_A_COLLIDABLE_3x3_SIDE_LEFT V2iInit(0, 1)
#define TILESET_A_COLLIDABLE_3x3_MIDDLE V2iInit(1, 1)
#define TILESET_A_COLLIDABLE_3x3_SIDE_RIGHT V2iInit(2, 1)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_LEFT V2iInit(0, 2)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_MIDDLE V2iInit(1, 2)
#define TILESET_A_COLLIDABLE_3x3_BOTTOM_RIGHT V2iInit(2, 2)

// Tileset A Collidable 1x3
#define TILESET_A_COLLIDABLE_1x3_OFFSET V2iInit(3, 0)
#define TILESET_A_COLLIDABLE_1x3_TOP V2iInit(0, 0)
#define TILESET_A_COLLIDABLE_1x3_MIDDLE V2iInit(0, 1)
#define TILESET_A_COLLIDABLE_1x3_BOTTOM V2iInit(0, 2)

// Tileset A Collidable 3x1
#define TILESET_A_COLLIDABLE_3x1_OFFSET V2iInit(4, 0)
#define TILESET_A_COLLIDABLE_3x1_LEFT V2iInit(0, 0)
#define TILESET_A_COLLIDABLE_3x1_CENTER V2iInit(1, 0)
#define TILESET_A_COLLIDABLE_3x1_RIGHT V2iInit(2, 0)

// Tileset A Collidable 1x1
#define TILESET_A_COLLIDABLE_1x1_A V2iInit(4, 1)
#define TILESET_A_COLLIDABLE_1x1_B V2iInit(5, 1)

// Tileset A Collidable 3x0.5
#define TILESET_A_COLLIDABLE_3x0_5_OFFSET V2iInit(4, 2)
#define TILESET_A_COLLIDABLE_3x0_5_LEFT V2iInit(0, 0)
#define TILESET_A_COLLIDABLE_3x0_5_CENTER V2iInit(1, 0)
#define TILESET_A_COLLIDABLE_3x0_5_RIGHT V2iInit(2, 0)

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
