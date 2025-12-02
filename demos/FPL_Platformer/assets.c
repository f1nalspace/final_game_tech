#include "assets.h"

fpl_extern void AssetsFree(GameAssets *assets) {
	// TODO(final): Use a proper allocator here
	MemoryAllocator *allocator = fpl_null;

	TextureDataFree(allocator, &assets->tilesetTexture.data);

	FontAssetFree(allocator, &assets->consoleFont);

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
	bool result = false;

	// TODO(final): Use a proper allocator here
	MemoryAllocator *allocator = fpl_null;

	fmemMemoryBlock tempMemory = fplZeroInit;
	if (!fmemBeginTemporary(&assets->transientMemory, &tempMemory)) {
		// TODO(final): Logging (Failed to start transient memory access)
		goto failed;
	}

	fpl_localvar char tempPath[1024];

	FontAsset *hudFont = &assets->consoleFont;
	TextureAsset *tilesetTextureAsset = &assets->tilesetTexture;

	// Fonts
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(tempPath, fplArrayCount(tempPath), 3, assets->dataPath, "fonts", fontFilename);
	if (!FontLoadFromFile(allocator, tempPath, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		// TODO(final): Logging (Font file not found)
		goto failed;
	}
	RenderPushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);

	// Textures
	const char *tilesetTextureFilename = "tileset.png";
	fplPathCombine(tempPath, fplArrayCount(tempPath), 3, assets->dataPath, "textures", tilesetTextureFilename);
	if (!TextureDataLoadFromFile(allocator, &tilesetTextureAsset->data, tempPath)) {
		// TODO(final): Logging (Texture not found)
		goto failed;
	}
	tilesetTextureAsset->texture = fpl_null;
	RenderPushTexture(renderState, &tilesetTextureAsset->texture, tilesetTextureAsset->data.data, tilesetTextureAsset->data.width, tilesetTextureAsset->data.height, tilesetTextureAsset->data.components, TextureFilterType_Nearest, TextureWrapMode_ClampToEdge, false, false);

	result = true;
	goto finished;

failed:
	result = false;

finished:
	fmemEndTemporary(&tempMemory);
	return result;
}