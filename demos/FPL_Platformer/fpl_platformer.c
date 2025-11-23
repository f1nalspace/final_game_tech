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
#include "editor.h"

// Directly included translation units
#include "entity.c"
#include "map.c"
#include "world.c"
#include "physics.c"
#include "editor.c"

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
typedef struct GameAssets {
	fmemMemoryBlock transientMemory;
	FontAsset consoleFont;
	char dataPath[1024];
} GameAssets;

typedef struct GameState {
	World world;

	GameAssets assets;

	Mat4f projection;
	Mat4f view;
	Mat4f viewProjection;
	Viewport viewport;
	Vec2f mouseWorldPos;

	Editor editor;

	float deltaTime;
	float framesPerSecond[2];

	bool isExiting;
	bool isDebugRendering;
} GameState;

static void AssetsFree(GameAssets *assets) {
	ReleaseFontAsset(&assets->consoleFont);
}

static void AssetsLoad(RenderState *renderState, GameAssets *assets) {
	// Fonts
	char tempDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(tempDataPath, fplArrayCount(tempDataPath), 2, assets->dataPath, "fonts");
	FontAsset *hudFont = &assets->consoleFont;
	if (LoadFontFromFile(tempDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		PushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);
	}

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
}

static bool GameStateInit(fmemMemoryBlock *memory, GameState *state) {
	if (memory == fpl_null || state == fpl_null) {
		return false; // Invalid arguments
	}

	Editor *editor = &state->editor;
	World *world = &state->world;
	GameAssets *assets = &state->assets;

	state->isDebugRendering = true;

	if (!WorldInit(memory, world)) {
		return false; // Failed to initialize world (insufficient memory, wrong values, etc.)
	}

	if (!EditorInit(memory, editor, world)) {
		return false; // Failed to initialize editor (insufficient memory, etc.)
	}

	// TEMPORARY(tspaete): Pass all relevant assets to the editor, because the editor has no access to the game-assets (for now)
	editor->assets.consoleFont = &assets->consoleFont;

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

	// Assets transient memory
	if (!fmemPushBlock(gameMemory->memory, &state->assets.transientMemory, fplMegaBytes(4), fmemPushFlags_Clear)) {
		return false; // Insufficient memory for transient asset memory
	}

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

	Editor *editor = &state->editor;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

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
	editor->mouseWorldPos = state->mouseWorldPos;

	// Debug input
	const Controller *keyboardController = &input->controllers[0];
	if (ButtonWasPressed(keyboardController->debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Editor input
	EditorInput(editor, input);

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

	RenderState *renderState = gameMemory->render;

	Editor *editor = &state->editor;
	World *world = &state->world;
	Map *map = &world->map;
	Physics *physics = &world->physics;
	Entity *player = &world->entities.player;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	// Re-compute matrices for smooth rendering
	const float cameraScale = F32Lerp(world->camera.scale[1], alpha, world->camera.scale[0]);
	const float invCameraScale = 1.0f / cameraScale;
	const Vec2f cameraOffset = V2fLerp(world->camera.offset[1], alpha, world->camera.offset[0]);
	state->projection = M4fOrthoRH(-w * invCameraScale, w * invCameraScale, -h * invCameraScale, h * invCameraScale, 0.0f, 1.0f);
	state->view = M4fTranslationV2(cameraOffset);
	state->viewProjection = M4fMult(state->projection, state->view);

	Vec2i mapSize = V2iInit(map->width, map->height);

	// TODO(final): Move colors to a global static struct
	Vec4f mapSolidTileColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4f mapGhostTileColor = V4fInit(0.3f, 0.3f, 0.3f, 1.0f);

	Vec4f playerTileColor = V4fInit(0.3f, 0.1f, 0.7f, 1.0f);
	Vec4f groundSensorTileColor = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
	Vec4f groundSensorColor = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags_Color | ClearFlags_Depth);

	// Only ortho projection
	SetMatrix(renderState, &state->projection);

	// Draw world size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	// Back to full view projection
	SetMatrix(renderState, &state->viewProjection);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);

	// Render editor (Pre -> Grid)
	EditorPreRender(renderState, editor, input);

	// Map
	for (int y = map->origin.y; y < map->origin.y + mapSize.h; ++y) {
		for (int x = map->origin.x; x < map->origin.x + mapSize.w; ++x) {
			Vec2i tilePos = V2iInit(x, y);
			Tile tile = MapGetTile(map, tilePos);
			if (MapTileTypeIsObstacle(map, tile.type)) {
				Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
				Vec4f tileColor = tile.type == TileType_Ghost ? mapGhostTileColor : mapSolidTileColor;
				PushRectangle(renderState, worldPos, TileSize, tileColor, true, 1.0f);
			}
		}
	}

	// Player
	Vec2f playerPos = V2fLerp(player->position[1], alpha, player->position[0]);
	PushRectangleCenter(renderState, playerPos, player->radius, player->color, false, 2.0f);
	PushOrigin(renderState, playerPos);

	// Player sensors
	bool drawPlayerSensor = false;
	if (drawPlayerSensor) {
		Vec2f groundSensorDirection = V2fInit(0.0f, -1.0f);
		float groundSensorDepth = 2.0f;
		float groundSensorLeft = -player->radius.w * 0.8f;
		float groundSensorRight = player->radius.w * 0.8f;
		Vec2f groundSensorStartLeft = V2fAdd(playerPos, V2fInit(groundSensorLeft, -(player->radius.h + EntitySensorGroundDistanceFromFoot)));
		Vec2f groundSensorStartRight = V2fAdd(playerPos, V2fInit(groundSensorRight, -(player->radius.h + EntitySensorGroundDistanceFromFoot)));
		Vec2f groundSensorEndLeft = V2fAddMultScalar(groundSensorStartLeft, groundSensorDirection, groundSensorDepth);
		Vec2f groundSensorEndRight = V2fAddMultScalar(groundSensorStartRight, groundSensorDirection, groundSensorDepth);
		PushCircle(renderState, groundSensorEndLeft, 2.0f, 8, groundSensorColor, true, 0.0f);
		PushCircle(renderState, groundSensorEndRight, 2.0f, 8, groundSensorColor, true, 0.0f);
		Vec2i groundSensorEndLeftTilePos = MapWorldCoordsToTile(map, groundSensorEndLeft);
		Vec2i groundSensorEndRightTilePos = MapWorldCoordsToTile(map, groundSensorEndRight);
		Vec2f worldPosLeft = MapTileCoordsToWorld(map, groundSensorEndLeftTilePos);
		Vec2f worldPosRight = MapTileCoordsToWorld(map, groundSensorEndRightTilePos);
		PushRectangleCenter(renderState, V2fAdd(worldPosLeft, TileRadius), TileRadius, groundSensorTileColor, false, 1.0f);
		PushRectangleCenter(renderState, V2fAdd(worldPosRight, TileRadius), TileRadius, groundSensorTileColor, false, 1.0f);
	}

	// Get player bounds
	AABB2f playerMotionBounds = EntityGetMotionBounds(player, dt);
	AABB2fExpandScalar(&playerMotionBounds, PhysicsAABBExpansion);

	// Render collision tile bounds
	bool renderPlayerTileBounds = true;
	if (renderPlayerTileBounds) {
		// Tile tiles area from AABB
		TileBounds tileBounds = MapGetTileBounds(map, &playerMotionBounds);

		for (int x = tileBounds.min.x; x <= tileBounds.max.x; ++x) {
			for (int y = tileBounds.min.y; y <= tileBounds.max.y; ++y) {
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

	// Render editor (Post -> Paint tiles)
	EditorPostRender(renderState, editor, input);

	// Char buffer to hold characters for formatting strings in the debug OSD
	fpl_localvar char debugOSDCharBuffer[256];

	// Char buffer to hold a size_t formatted with thousand separators
	fpl_localvar char debugOSDSizeCharBuffer[2][32 + 1] = fplZeroInit;

	//
	// OSD rendering
	//

	// Render debug OSD on the top
	if (state->isDebugRendering) {
		SetMatrix(renderState, &state->projection);

		const FontAsset *font = &state->assets.consoleFont; 
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec4f blackColor = V4fInit(0, 0, 0, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 6.0f;

		// Game memory
		FormatSize(gameMemory->memory->used, fplArrayCount(debugOSDSizeCharBuffer[0]), debugOSDSizeCharBuffer[0]);
		FormatSize(gameMemory->memory->size, fplArrayCount(debugOSDSizeCharBuffer[1]), debugOSDSizeCharBuffer[1]);
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Game Memory: %s / %s bytes", debugOSDSizeCharBuffer[0], debugOSDSizeCharBuffer[1]);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		// Render memory
		FormatSize(renderState->lastMemoryUsage, fplArrayCount(debugOSDSizeCharBuffer[0]), debugOSDSizeCharBuffer[0]);
		FormatSize(renderState->memory.size, fplArrayCount(debugOSDSizeCharBuffer[1]), debugOSDSizeCharBuffer[1]);
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Render Memory: %s / %s bytes", debugOSDSizeCharBuffer[0], debugOSDSizeCharBuffer[1]);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w - 1, blockPos.y - 1), fontHeight, 0.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);

		// Frame timings
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f - 1, blockPos.y - 1), fontHeight, -1.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);

		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight * 2.0f));

		// Player states
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Player ground: %s", player->groundState.current ? "yes" : "no");
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
	}

	// Render editor OSD
	bool drawEditorOSD = true;
	if (drawEditorOSD) {
		SetMatrix(renderState, &state->projection);
		EditorOSDRender(renderState, editor, input);
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