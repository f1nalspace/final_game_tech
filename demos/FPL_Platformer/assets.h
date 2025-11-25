#ifndef ASSETS_H
#define ASSETS_H

#include <final_platform_layer.h>
#include <final_memory.h>
#include <final_assets.h>
#include <final_render.h>
#include <final_math.h>

// Size of the assets transient memory block
#define GAME_ASSETS_MEMORY_TRANSIENT_SIZE fplMegaBytes(8)

// Size of the assets persistent memory block
#define GAME_ASSETS_MEMORY_PERSISTENT_SIZE fplMegaBytes(4)

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
