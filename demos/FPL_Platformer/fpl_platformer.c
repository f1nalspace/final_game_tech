/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Platformer

Description:
	Platformer based on speculative contacts

Requirements:
	- C99 Compiler
	- Final Framework

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <final_game.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#define FINAL_GEOMETRY_IMPLEMENTATION
#include <final_geometry.h>

#include <final_utils.h>

// Headers
#include "entity.h"
#include "map.h"
#include "world.h"
#include "physics.h"

// Directly included translation units
#include "entity.c"
#include "map.c"
#include "world.c"
#include "physics.c"

//
// Levels
//
#define TestLevel_Width 11
#define TestLevel_Height 8

static TileType gTestLevelTiles[TestLevel_Width * TestLevel_Height] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 3, 0, 0, 1, 0, 1, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static MapDefinition gTestLevel = {
	.tiles = gTestLevelTiles,
	.width = TestLevel_Width,
	.height = TestLevel_Height,
};

//
// Game
//
typedef struct Assets {
	FontAsset consoleFont;
	char dataPath[1024];
} Assets;

typedef struct Editor {
	uint32_t drawTile;
	Vec2i drawTilePos;
	bool isDrawing;
} Editor;

typedef struct GameState {
	World world;
	Assets assets;

	Mat4f projection;
	Mat4f view;
	Mat4f viewProjection;
	Viewport viewport;
	Vec2f mouseWorldPos;

	Editor editor;
	Entity *dragEntity;
	Vec2f dragStart;
	bool isDragging;

	float deltaTime;
	float framesPerSecond[2];

	bool isExiting;
	bool isDebugRendering;
} GameState;

static void AssetsFree(Assets *assets) {
	ReleaseFontAsset(&assets->consoleFont);
}

static void AssetsLoad(RenderState *renderState, Assets *assets) {
	// Fonts
	char fontDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(fontDataPath, fplArrayCount(fontDataPath), 2, assets->dataPath, "fonts");
	FontAsset *hudFont = &assets->consoleFont;
	if (LoadFontFromFile(fontDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		PushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);
	}
}

static bool GameStateInit(fmemMemoryBlock *memory, GameState *state) {
	if (memory == fpl_null || state == fpl_null) {
		return false; // Invalid arguments
	}

	state->isDebugRendering = true;

	if (!WorldInit(memory, &state->world)) {
		return false; // Failed to initialize the world (insufficient memory, wrong values, etc.)
	}

	return true;
}

static bool GameStateLoadMap(GameState *state, const MapDefinition *mapDefinition) {
	if (state == fpl_null || mapDefinition == fpl_null) {
		return false; // Invalid arguments
	}

	if (!WorldLoad(&state->world, mapDefinition)) {
		return false; // Failed to load the map into the world (insufficient memory, wrong map, etc.)
	}

	return true;
}

extern bool GameInit(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return false; // Invalid arguments
	}

	GameState *state = (GameState *)fmemPush(gameMemory->memory, sizeof(GameState), fmemPushFlags_Clear);
	if (state == fpl_null) {
		return false; // Insufficient memory for game state
	}

	gameMemory->game = state;

	RenderState *renderState = gameMemory->render;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	AssetsLoad(renderState, &state->assets);

	if (!GameStateInit(gameMemory->memory, state)) {
		return false; // Failed to initialize the game state
	}

	if (!GameStateLoadMap(state, &gTestLevel)) {
		return false; // Failed to load the test level
	}

	return true;
}

extern void GameRelease(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return; // Invalid arguments
	}
	GameState *state = gameMemory->game;
	if (state == fpl_null) {
		return; // Game state not allocated
	}
	AssetsFree(&state->assets);
	fplClearStruct(state);
}

extern bool IsGameExiting(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return false; // Invalid arguments
	}
	GameState *state = gameMemory->game;
	assert(state != fpl_null);
	return state->isExiting;
}

static bool MapResizeToTilePos(Map *map, const Vec2i tilePos) {
	if (map == fpl_null || map->width == 0 || map->height == 0) {
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
	} else if (tilePos.y > mapHeightMinusOne) {
		int ycount = tilePos.y - mapHeightMinusOne;
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
	size_t requiredNewSize = newMapSize.w * newMapSize.h * sizeof(uint32_t);
	fplAssert(requiredNewSize <= map->persistentMemory.size);

	// Create new tiles and copy old tiles over it
	map->width = newMapSize.w;
	map->height = newMapSize.h;
	map->origin = newOrigin;
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

	fmemEndTemporary(&map->temporaryMemory);

	return true;
}

static void EditorPaintTile(Editor *editor, Map *map, const Vec2i tilePos) {
	if (MapIsTileInside(map, tilePos)) {
		editor->drawTilePos = tilePos;
		MapSetTileType(map, tilePos, editor->drawTile);
		fplDebugFormatOut("Paint tile %d x %d with %d", tilePos.x, tilePos.y, editor->drawTile);
	} else {
		// NOTE(final): Resize returns a new tile position, due to offset change
		if (MapResizeToTilePos(map, tilePos)) {
			//MapSetTileType(map, tilePos, editor->drawTile);
			editor->drawTilePos = tilePos;
			editor->isDrawing = false;
		}
	}
}

static void EditorInput(GameState *state, const Input *input) {
	Map *map = &state->world.map;

	Editor *editor = &state->editor;

	Vec2i mouseTilePos = MapWorldCoordsToTile(map, state->mouseWorldPos);

	if (IsDown(input->mouse.left)) {
		if (!editor->isDrawing) {
			editor->isDrawing = true;
			editor->drawTilePos = mouseTilePos;
			if (MapIsTileInside(map, mouseTilePos)) {
				Tile tile = MapGetTile(map, mouseTilePos);
				editor->drawTile = tile.type != TileType_Solid ? TileType_Solid : TileType_None;
			} else {
				editor->drawTile = TileType_Solid;
			}
			EditorPaintTile(editor, map, mouseTilePos);
		} else {
			fplAssert(editor->drawTile != UINT32_MAX);
			if (!V2iEquals(mouseTilePos, editor->drawTilePos)) {
				EditorPaintTile(editor, map, mouseTilePos);
			}
		}
	} else {
		editor->drawTile = UINT32_MAX;
		editor->drawTilePos = mouseTilePos;
		editor->isDrawing = false;
	}
}

extern void GameInput(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	RenderState *renderState = gameMemory->render;
	assert(renderState != fpl_null);

	World *world = &state->world;

	Entity *player = &world->entities.player;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	// Debug input
	const Controller *keyboardController = &input->controllers[0];
	if (WasPressed(keyboardController->debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	const float cameraScale = world->camera.scale[0];
	const float invCameraScale = 1.0f / cameraScale;
	state->viewport = ComputeViewportByAspect(input->windowSize, WorldAspect);
	world->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * cameraScale;
	world->camera.pixelsToWorld = 1.0f / world->camera.worldToPixels;

	state->projection = M4fOrthoRH(-w * invCameraScale, w * invCameraScale, -h * invCameraScale, h * invCameraScale, 0.0f, 1.0f);
	state->view = M4fTranslationV2(world->camera.offset[0]);
	state->viewProjection = M4fMult(state->projection, state->view);

	// Mouse
	int mouseCenterX = (input->mouse.pos.x) - input->windowSize.w / 2;
	int mouseCenterY = (input->windowSize.h - 1 - input->mouse.pos.y) - input->windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * world->camera.pixelsToWorld) - world->camera.offset[0].x;
	state->mouseWorldPos.y = (mouseCenterY * world->camera.pixelsToWorld) - world->camera.offset[0].y;

	// Editor input
	EditorInput(state, input);

	// Player input
	EntityInput(player, input);
}

extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	const float dt = input->fixedDeltaTime;

	World *world = &state->world;
	Map *map = &world->map;
	
	WorldUpdate(world, input);

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input->framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = F32Avg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

static void PushNormal(RenderState *renderState, const Vec2f position, const Vec2f normal, const float length, const float chairSize) {
	// const float length = 20.0f, const float chairSize = 8.0f

	Vec2f tangent = V2fCrossR(normal, 1.0f);

	Vec2f a = position;
	Vec2f b = V2fAddMultScalar(a, normal, length);
	PushLine(renderState, a, b, V4fInit(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, normal, chairSize);
	b = V2fAddMultScalar(a, tangent, chairSize);
	PushLine(renderState, a, b, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, tangent, chairSize);
	b = V2fAddMultScalar(a, normal, chairSize);
	PushLine(renderState, a, b, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
}

static void PushOrigin(RenderState *renderState, const Vec2f origin) {
	PushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, 2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, -2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(-2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin, 1.0f, V4fInit(0.25f, 0.25f, 0.25f, 1), true, 1.0f);
}

extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha) {
	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	World *world = &state->world;

	Map *map = &world->map;

	Physics *physics = &world->physics;

	Entity *player = &world->entities.player;

	RenderState *renderState = gameMemory->render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	// Re-compute matrices for smooth rendering
	const float cameraScale = F32ScalarLerp(world->camera.scale[1], alpha, world->camera.scale[0]);
	const float invCameraScale = 1.0f / cameraScale;
	const Vec2f cameraOffset = V2fLerp(world->camera.offset[1], alpha, world->camera.offset[0]);
	state->projection = M4fOrthoRH(-w * invCameraScale, w * invCameraScale, -h * invCameraScale, h * invCameraScale, 0.0f, 1.0f);
	state->view = M4fTranslationV2(cameraOffset);
	state->viewProjection = M4fMult(state->projection, state->view);

	Vec2i mapSize = V2iInit(map->width, map->height);
	Vec2f mapArea = V2fHadamard(TileSize, V2fInit((float)mapSize.x, (float)mapSize.y));
	Vec2f mapOrigin = MapTileCoordsToWorld(map, map->origin);
	Vec4f mapSolidTileColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4f mapGhostTileColor = V4fInit(0.3f, 0.3f, 0.3f, 1.0f);
	Vec4f playerTileColor = V4fInit(0.3f, 0.1f, 0.7f, 1.0f);

	Vec2f gridSize = mapArea;
	Vec2f gridOrigin = mapOrigin;
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	int gridTileCountX = map->width;
	int gridTileCountY = map->height;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags_Color | ClearFlags_Depth);
	SetMatrix(renderState, &state->projection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	SetMatrix(renderState, &state->viewProjection);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);

	// Tile grid
	for (int i = 0; i <= gridTileCountX; ++i) {
		float xoffset = i * TileWidth;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(xoffset, 0));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(xoffset, gridSize.y));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}
	for (int i = 0; i <= gridTileCountY; ++i) {
		float yoffset = i * TileHeight;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(0, yoffset));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(gridSize.x, yoffset));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}

	// Map
	for (int y = 0; y < mapSize.h; ++y) {
		for (int x = 0; x < mapSize.w; ++x) {
			Tile tile = MapGetTile(map, V2iInit(x, y));
			if (MapTileTypeIsObstacle(map, tile.type)) {
				Vec2f tilePos = MapTileCoordsToWorld(map, V2iInit(x, y));
				Vec4f tileColor = tile.type == TileType_Ghost ? mapGhostTileColor : mapSolidTileColor;
				PushRectangle(renderState, tilePos, TileSize, tileColor, true, 1.0f);
			}
		}
	}

	// Player
	Vec2f playerPos = V2fLerp(player->position[1], alpha, player->position[0]);
	PushRectangleCenter(renderState, playerPos, player->radius, player->color, false, 2.0f);
	PushOrigin(renderState, playerPos);

	// Render collision tile bounds
	bool renderPlayerTileBounds = true;
	if (renderPlayerTileBounds) {
		// Predict position for next frame
		Vec2f predictedPos = V2fAddMultScalar(player->position[0], player->velocity, dt);

		// Create motion bounds AABB and expand it
		Vec2f min = V2fSub(V2fMin(predictedPos, player->position[0]), player->radius);
		Vec2f max = V2fAdd(V2fMax(predictedPos, player->position[0]), player->radius);
		AABB2f entityMotionBounds = AABB2fInit(min, max);
		AABB2fExpandScalar(&entityMotionBounds, PhysicsAABBExpansion);

		// Tile tiles area from AABB
		Vec2i minTile = MapWorldCoordsToTile(map, entityMotionBounds.min);
		Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(entityMotionBounds.max, V2fInit(0.5f, 0.5f)));

		for (int x = minTile.x; x <= maxTile.x; ++x) {
			for (int y = minTile.y; y <= maxTile.y; ++y) {
				if (x < 0 || x >= (int)map->width || y < 0 || y >= (int)map->height) {
					continue;
				}
				Vec2i tilePos = V2iInit(x, y);
				Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
				Vec2f tileCenter = V2fAdd(worldPos, TileRadius);
				PushRectangleCenter(renderState, tileCenter, TileRadius, playerTileColor, false, 1.0f);
			}
		}
	}

	// Contacts
	bool renderContacts = true;
	if (renderContacts) {
		for (size_t i = 0; i < physics->contactList.used; ++i) {
			const Contact *contact = physics->contactList.data + i;
			PushCircle(renderState, contact->posA, 2.0f, 16, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
			PushCircle(renderState, contact->posB, 2.0f, 16, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), true, 0.0f);
			PushNormal(renderState, contact->posB, contact->normal, 10.0f, 5.0f);
		}
	}

	// Mouse cursor
	PushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(2, 2), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);

	// Mouse tile
	Vec2i mouseTilePos = MapWorldCoordsToTile(map, state->mouseWorldPos);
	Vec2f mouseWorldPos = MapTileCoordsToWorld(map, mouseTilePos);
	int32_t mouseTileIndex = mouseTilePos.y * map->width + mouseTilePos.x;
	uint32_t mouseTileId = MapTileIDStart + mouseTileIndex;
	PushRectangle(renderState, mouseWorldPos, TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);

	const FontAsset *font = &state->assets.consoleFont;
	float fontHeight = 6.0f;

	char buffer[100];
	fplStringFormat(buffer, fplArrayCount(buffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
	PushText(renderState, buffer, fplGetStringLength(buffer), &font->desc, font->texture, mouseWorldPos, fontHeight, 1.0f, 1.0f, V4fInit(1, 0, 1, 1));
	fplStringFormat(buffer, fplArrayCount(buffer), "%u", mouseTileId);
	PushText(renderState, buffer, fplGetStringLength(buffer), &font->desc, font->texture, V2fAdd(mouseWorldPos, TileRadius), fontHeight, 0.0f, 0.0f, V4fInit(0, 1, 1, 1));

	if (state->isDebugRendering) {
		SetMatrix(renderState, &state->projection);

		const FontAsset *font = &state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec4f blackColor = V4fInit(0, 0, 0, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 8.0f;

		fpl_localvar char sizeCharsBuffer[2][32 + 1] = fplZeroInit;
		FormatSize(gameMemory->memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory->memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState->lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState->memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w - 1, blockPos.y - 1), fontHeight, 0.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplStringFormat(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f - 1, blockPos.y - 1), fontHeight, -1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);

		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
		fplStringFormat(text, fplArrayCount(text), "Player on ground: %s", player->groundState.current ? "yes" : "no");
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
		fplStringFormat(text, fplArrayCount(text), "Drawing: %d x %d, Active: %s, Tile: %d", state->editor.drawTilePos.x, state->editor.drawTilePos.y, state->editor.isDrawing ? "yes" : "no", state->editor.drawTile);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
		fplStringFormat(text, fplArrayCount(text), "Mouse pos: %.04f x %.04f, down: %s", state->mouseWorldPos.x, state->mouseWorldPos.y, input->mouse.left.endedDown ? "yes" : "no");
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
	}

	// Overwrite render infos such as position, rotation for "previous" from "current" states
	// This is important for getting smooth interpolated rendering
	world->camera.offset[1] = world->camera.offset[0];
	world->camera.scale[1] = world->camera.scale[0];
	player->position[1] = player->position[0];
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = fplZeroInit;
	config.title = "FPL Demo | Platformer";
	config.disableInactiveDetection = true;
	config.disableVerticalSync = false;
	int result = GameMain(&config);
	return(result);
}