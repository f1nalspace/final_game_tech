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
#include "assets.h"
#include "entity.h"
#include "map.h"
#include "world.h"
#include "physics.h"
#include "editor.h"

// Directly included translation units
#include "assets.c"
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
// Game State
//
typedef enum GameMode {
	// Only game is running
	GameMode_Game = 0,
	// Editor is active and game is still running
	GameMode_EditorPlay,
	// Editor is active and game is fully paused
	GameMode_EditorPause,
	// Total number of game modes
	GameMode_Count,
} GameMode;

fpl_globalvar const char *gGameModeNames[GameMode_Count] = {
	"Game",
	"Editor (Play-mode)",
	"Editor (Full)",
};

typedef struct GameState {
	World world;

	GameAssets assets;

	Mat4f projection;
	Mat4f view;
	Mat4f viewProjection;
	Viewport viewport;
	Vec2f mouseWorldPos;

	Editor editor;

	fmemMemoryBlock *memory;

	float framesPerSecond[2];
	float deltaTime;

	GameMode mode;

	bool isExiting;
	bool isDebugRendering;
} GameState;

static void GameStateRelease(GameState *state) {
	if (state == fpl_null) {
		return; // Invalid arguments
	}
	AssetsFree(&state->assets);
	fplClearStruct(state);
}

static bool GameStateInit(fmemMemoryBlock *memory, GameState *state) {
	if (memory == fpl_null || state == fpl_null) {
		return false; // Invalid arguments
	}

	fplClearStruct(state);

	state->memory = memory;

	Editor *editor = &state->editor;
	World *world = &state->world;
	GameAssets *assets = &state->assets;

	if (!AssetsInit(state->memory, &state->assets)) {
		return false; // Failed to initialize assets (insufficient memory, etc.)
	}

	if (!WorldInit(state->memory, world)) {
		return false; // Failed to initialize world (insufficient memory, wrong values, etc.)
	}

	if (!EditorInit(state->memory, editor, assets, world)) {
		return false; // Failed to initialize editor (insufficient memory, etc.)
	}

	state->isDebugRendering = true;

	state->mode = GameMode_Game;

	return true;
}

static bool GameStateLoad(RenderState *renderState, GameState *state) {
	if (state == fpl_null) {
		return false; // Invalid arguments
	}

	Editor *editor = &state->editor;
	World *world = &state->world;
	GameAssets *assets = &state->assets;

	if (!AssetsLoad(renderState, assets)) {
		return false; // Failed to load game assets (insufficient memory, wrong paths, files not found, etc.)
	}

	if (!WorldLoad(world, &gTestLevel)) {
		return false; // Failed to load the map into the world (insufficient memory, wrong map, etc.)
	}

	return true;
}

extern bool GameInit(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return false; // Invalid arguments
	}

	fmemMemoryBlock *gameMem = gameMemory->memory;
	fplAssert(gameMem != fpl_null);

	GameState *gameState = (GameState *)fmemPush(gameMem, sizeof(GameState), fmemPushFlags_Clear);
	if (gameState == fpl_null) {
		return false; // Insufficient memory for game state
	}
	gameMemory->game = gameState;

	RenderState *renderState = gameMemory->render;

	if (!GameStateInit(gameMem, gameState)) {
		return false; // Failed to initialize game state
	}

	if (!GameStateLoad(renderState, gameState)) {
		return false; // Failed to load the test level
	}

	return true;
}

extern void GameRelease(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return; // Invalid arguments
	}
	GameState *gameState = gameMemory->game;
	if (gameState == fpl_null) {
		return; // Game state not allocated
	}
	GameStateRelease(gameState);
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

	const bool isGameUpdating = state->mode != GameMode_EditorPause;
	const bool isEditorMode = state->mode != GameMode_Game;

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

	const Controller *keyboardController = &input->controllers[0];
	
	// Debug rendering on/off (F6)
	if (ButtonWasPressed(keyboardController->debug6)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Toggle game state (F1)
	if (ButtonWasPressed(keyboardController->debug1)) {
		switch (state->mode) {
			case GameMode_Game:
				state->mode = GameMode_EditorPlay;
				break;
			case GameMode_EditorPlay:
				state->mode = GameMode_EditorPause;
				break;
			case GameMode_EditorPause:
				state->mode = GameMode_Game;
				break;
			default:
				break;
		}
	}

	if (isEditorMode) {
		EditorInput(editor, input);
	}

	if (isGameUpdating) {
		EntityInput(player, input);
	}
}

extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	const float dt = input->fixedDeltaTime;
	const bool isGameUpdating = state->mode != GameMode_EditorPause;
	const bool isEditorMode = state->mode != GameMode_Game;

	World *world = &state->world;
	Map *map = &world->map;
	Entity *player = &world->entities.player;

	if (isGameUpdating) {
		WorldUpdate(world, input);
		world->camera.offset[0] = V2fNegate(player->position[0]);
		world->camera.scale[0] = 1.0f;
	}

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
	const bool isEditorMode = state->mode >= GameMode_EditorPlay && state->mode <= GameMode_EditorPause;

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
	if (isEditorMode) {
		EditorPreRender(renderState, editor, input);
	}

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
	if (state->mode != GameMode_Game) {
		PushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(2, 2), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
	}

	// Render editor (Post -> Paint tiles)
	if (isEditorMode) {
		EditorPostRender(renderState, editor, input);
	}

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

		// Game state
		const char *gameModeName = gGameModeNames[state->mode];
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Game Mode: %s", gameModeName);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));

		// Player states
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Player ground: %s", player->groundState.current ? "yes" : "no");
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
	}

	// Render editor OSD
	bool drawEditorOSD = state->mode >= GameMode_EditorPlay;
	if (drawEditorOSD) {
		SetMatrix(renderState, &state->projection);
		EditorOSDRender(renderState, editor, input);
	}

	// Overwrite render infos such as position, rotation for "previous" from "current" states
	// This is important for getting smooth interpolated rendering
	player->position[1] = player->position[0];
	world->camera.offset[1] = world->camera.offset[0];
	world->camera.scale[1] = world->camera.scale[0];
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