#include "assets.h"

fpl_extern void AssetsFree(GameAssets *assets) {
	ReleaseFontAsset(&assets->consoleFont);

	// NOTE(final): No need for freeing any memory block here, because everything is inside the game memory

	fplClearStruct(assets);
}

fpl_extern bool AssetsInit(fmemMemoryBlock *memory, GameAssets *assets) {
	if (memory == fpl_null || assets == fpl_null) {
		return false;
	}

	fplClearStruct(assets);

	fplGetExecutableFilePath(assets->dataPath, fplArrayCount(assets->dataPath));
	fplExtractFilePath(assets->dataPath, assets->dataPath, fplArrayCount(assets->dataPath));
	fplPathCombine(assets->dataPath, fplArrayCount(assets->dataPath), 2, assets->dataPath, "data");

	if (!fmemPushBlock(memory, &assets->transientMemory, GAME_ASSETS_MEMORY_TRANSIENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for transient asset memory
	}
	if (!fmemPushBlock(memory, &assets->persistentMemory, GAME_ASSETS_MEMORY_PERSISTENT_SIZE, fmemPushFlags_Clear)) {
		return false; // Insufficient memory for persistent asset memory
	}

	return true;
}

fpl_extern bool AssetsLoad(RenderState *renderState, GameAssets *assets) {
	// Fonts
	char tempDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(tempDataPath, fplArrayCount(tempDataPath), 2, assets->dataPath, "fonts");
	FontAsset *hudFont = &assets->consoleFont;
	if (LoadFontFromFile(tempDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		RendererPushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);
	}

#if 0
	// Maps
	const char *mapName = "Level_1"; // Must match the "identifier" of the level
	const char *mapFilename = "level1.ldtk";
	fplPathCombine(tempDataPath, fplArrayCount(tempDataPath), 2, assets->dataPath, "maps");

	fmemMemoryBlock tempMemory = fplZeroInit;
	fmemBeginTemporary(&assets->transientMemory, &tempMemory);
	MapDefinition *mapDef = fmemPushStruct(&tempMemory, MapDefinition, fmemPushFlags_Clear);
	if (!MapDefinitionLoadFromFile(&tempMemory, tempDataPath, mapFilename, mapName, mapDef)) {

	}
	fmemEndTemporary(&tempMemory);
#endif

	return true;
}