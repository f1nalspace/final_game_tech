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
#include "common.h"
#include "camera.h"
#include "assets.h"
#include "entity.h"
#include "map.h"
#include "world.h"
#include "physics.h"
#include "editor.h"

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
	Camera camera;
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

static void GameChangeMode(GameState *state, const GameMode newMode) {
	World *world = &state->world;
	Entity *player = &world->entities.player;
	Editor *editor = &state->editor;
	Camera *camera = &state->camera;

	switch (newMode) {
		case GameMode_Game:
		{
			state->mode = GameMode_Game;
			state->camera.limits.isEnabled = true;
			CameraSetScale(camera, GameViewScale);
			CameraSetPos(camera, player->transform[0].pos);
			editor->mode = EditorMode_None;
		} break;

		case GameMode_EditorPlay:
		{
			state->mode = GameMode_EditorPlay;
			state->camera.limits.isEnabled = false;
			CameraSetScale(camera, editor->camera.transform[0].scale);

			// TODO(final): Compute based on editor scale
			editor->camera.transform[0].offset = player->transform[0].pos;

			CameraSetPos(camera, editor->camera.transform[0].offset);

			editor->mode = EditorMode_Live;
		} break;

		case GameMode_EditorPause:
		{
			state->mode = GameMode_EditorPause;
			state->camera.limits.isEnabled = false;
			CameraSetScale(camera, editor->camera.transform[0].scale);

			// TODO(final): Compute based on editor scale
			editor->camera.transform[0].offset = player->transform[0].pos;

			CameraSetPos(camera, editor->camera.transform[0].offset);

			editor->mode = EditorMode_Full;
		} break;

		default:
			FPL_NOT_IMPLEMENTED;
	}
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

	// NOTE(tspaete): Projection matrix will never change across the entire game run
	state->projection = M4fOrthoRH(-WorldRadiusW, WorldRadiusW, -WorldRadiusH, WorldRadiusH, 0.0f, 1.0f);

	// NOTE(tspaete): This will never change across the entire game run
	state->camera.viewRadius = V2fMultScalar(V2fInit(WorldRadiusW, WorldRadiusH), GameViewInvScale);

	state->isDebugRendering = true;

	GameChangeMode(state, GameMode_Game);

	return true;
}

static bool GameStateLoad(RenderState *renderState, GameState *state) {
	if (state == fpl_null) {
		return false; // Invalid arguments
	}

	Editor *editor = &state->editor;
	World *world = &state->world;
	GameAssets *assets = &state->assets;
	Entity *player = &world->entities.player;

	if (!AssetsLoad(renderState, assets)) {
		return false; // Failed to load game assets (insufficient memory, wrong paths, files not found, etc.)
	}

	if (!WorldLoad(world, &gTestLevel)) {
		return false; // Failed to load the map into the world (insufficient memory, wrong map, etc.)
	}

	GameChangeMode(state, GameMode_EditorPause);

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
	Map *map = &world->map;
	Entity *player = &world->entities.player;
	Editor *editor = &state->editor;
	Camera *camera = &state->camera;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	const bool isGameUpdating = state->mode != GameMode_EditorPause;
	const bool isEditorMode = state->mode != GameMode_Game;

	// Camera

	// TODO(final): Needs only to be updated when viewport changes!
	Viewport viewport = ViewportComputeByAspect(input->windowSize, WorldAspect);;
	CameraUpdateViewport(camera, &viewport, WorldWidth);
	CameraUpdateViewport(&editor->camera, &viewport, WorldWidth);

	// TODO(final): Needs only to be updated when map bounds changes!
	state->camera.limits.bounds = map->maxBounds;

	// Matrices
	Mat4f translationMat = M4fTranslationV2(state->camera.transform[0].offset);
	Mat4f scaleMat = M4fScaleV2(V2fInitScalar(state->camera.transform[0].scale));
	state->view = M4fMult(scaleMat, translationMat);
	state->viewProjection = M4fMult(state->projection, state->view);

	// Mouse
	int mouseCenterX = (input->mouse.pos.x) - input->windowSize.w / 2;
	int mouseCenterY = (input->windowSize.h - 1 - input->mouse.pos.y) - input->windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.transform[0].offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.transform[0].offset.y;
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
				GameChangeMode(state, GameMode_EditorPlay);
				break;
			case GameMode_EditorPlay:
				GameChangeMode(state, GameMode_EditorPause);
				break;
			case GameMode_EditorPause:
				GameChangeMode(state, GameMode_Game);
				break;
			default:
				break;
		}
	}

	if (state->mode != GameMode_Game) {
		EditorInput(editor, input);
	}

	if (state->mode <= GameMode_EditorPlay) {
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
	Camera *camera = &state->camera;
	Editor *editor = &state->editor;

	if (isGameUpdating) {
		WorldUpdate(world, input);
	}

	if (state->mode == GameMode_EditorPlay) {
		editor->camera.transform[0].offset = player->transform[0].pos;
	}

	// Overwrite camera scale when in editor mode
	if (isEditorMode) {
		CameraSetScale(camera, editor->camera.transform[0].scale);
		CameraSetPos(camera, editor->camera.transform[0].offset);
	} else {
		CameraSetScale(camera, GameViewScale);
		CameraSetPos(camera, player->transform[0].pos);
	}

	// FPS display
	const float fpsSmoothing = 0.1f;
	const float newFps = input->framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];
	state->deltaTime = dt;
	state->framesPerSecond[1] = F32Avg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

static void RenderNormal(RenderState *renderState, const Vec2f position, const Vec2f normal, const float length, const float chairSize) {
	// const float length = 20.0f, const float chairSize = 8.0f

	Vec2f tangent = V2fCrossR(normal, 1.0f);

	Vec2f a = position;
	Vec2f b = V2fAddMultScalar(a, normal, length);
	RenderPushLine(renderState, a, b, V4fInit(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, normal, chairSize);
	b = V2fAddMultScalar(a, tangent, chairSize);
	RenderPushLine(renderState, a, b, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, tangent, chairSize);
	b = V2fAddMultScalar(a, normal, chairSize);
	RenderPushLine(renderState, a, b, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
}

static void RenderOrigin(RenderState *renderState, const Vec2f origin) {
	RenderPushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, 2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	RenderPushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, -2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	RenderPushQuad(renderState, V2fAdd(origin, V2fInit(-2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	RenderPushQuad(renderState, V2fAdd(origin, V2fInit(2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	RenderPushQuad(renderState, origin, 1.0f, V4fInit(0.25f, 0.25f, 0.25f, 1), true, 1.0f);
}

extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha) {
	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	RenderState *renderState = gameMemory->render;

	const Editor *editor = &state->editor;
	const Camera *camera = &state->camera;
	const GameAssets *assets = &state->assets;

	World *world = &state->world;
	Map *map = &world->map;
	Physics *physics = &world->physics;
	Entity *player = &world->entities.player;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;
	const bool isEditorMode = state->mode >= GameMode_EditorPlay && state->mode <= GameMode_EditorPause;

	// Re-compute matrices for smooth rendering
	const float cameraScale = F32Lerp(state->camera.transform[1].scale, alpha, state->camera.transform[0].scale);
	const Vec2f cameraOffset = V2fLerp(state->camera.transform[1].offset, alpha, state->camera.transform[0].offset);

	Mat4f translationMat = M4fTranslationV2(cameraOffset);
	Mat4f scaleMat = M4fScaleV2(V2fInit(cameraScale, cameraScale));

	state->view = M4fMult(scaleMat, translationMat);
	state->viewProjection = M4fMult(state->projection, state->view);

	Vec2i mapSize = V2iInit(map->width, map->height);

	// TODO(final): Move colors to a global static struct
	Vec4f mapSolidTileColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4f mapGhostTileColor = V4fInit(0.5f, 0.5f, 0.5f, 1.0f);

	Vec4f playerTileColor = V4fInit(0.3f, 0.1f, 0.7f, 1.0f);
	Vec4f groundSensorTileColor = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
	Vec4f groundSensorColor = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);

	RenderPushViewport(renderState, camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);
	RenderPushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags_Color | ClearFlags_Depth);

	// Only ortho projection
	RenderSetMatrix(renderState, &state->projection);

	// Draw world size
	RenderPushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	// Back to full view projection
	RenderSetMatrix(renderState, &state->viewProjection);

	// World cross
	RenderPushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
	RenderPushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);

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
				Vec2f tileCenter = V2fAdd(worldPos, TileRadius);
				Vec4f tileColor = tile.type == TileType_Ghost ? mapGhostTileColor : mapSolidTileColor;

				Vec2i textureTilePos = V2iInit(0, 0);

				UVRect tileUVRect = UVRectFromTile(V2iInit(assets->tilesetTexture.data.width, assets->tilesetTexture.data.height), V2iInit(32, 32), 0, textureTilePos);
				RenderPushSprite(renderState, tileCenter, TileRadius, assets->tilesetTexture.texture, tileColor, tileUVRect, SpriteFlags_FlipV);

			}
		}
	}

	// Map bounds
	Vec2f mapBoundsSize = AABB2fGetSize(&map->maxBounds);
	RenderPushRectangle(renderState, map->maxBounds.min, mapBoundsSize, V4fInit(0.3f, 1.0f, 0.3f, 1.0f), false, 4.0f);

	// Player
	Vec2f playerPos = V2fLerp(player->transform[1].pos, alpha, player->transform[0].pos);
	RenderPushRectangleCenter(renderState, playerPos, player->radius, player->color, false, 2.0f);
	RenderOrigin(renderState, playerPos);

	// Camera view
	if (state->mode == GameMode_Game || state->mode == GameMode_EditorPlay) {
		RenderPushRectangleCenter(renderState, V2fNegate(cameraOffset), state->camera.viewRadius, V4fInit(0.2f, 0.1f, 1.0f, 1.0f), false, 2.0f);
	} else {
		RenderPushRectangleCenter(renderState, playerPos, state->camera.viewRadius, V4fInit(0.2f, 0.1f, 1.0f, 1.0f), false, 2.0f);
	}

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
		RenderPushCircle(renderState, groundSensorEndLeft, 2.0f, 8, groundSensorColor, true, 0.0f);
		RenderPushCircle(renderState, groundSensorEndRight, 2.0f, 8, groundSensorColor, true, 0.0f);
		Vec2i groundSensorEndLeftTilePos = MapWorldCoordsToTile(map, groundSensorEndLeft);
		Vec2i groundSensorEndRightTilePos = MapWorldCoordsToTile(map, groundSensorEndRight);
		Vec2f worldPosLeft = MapTileCoordsToWorld(map, groundSensorEndLeftTilePos);
		Vec2f worldPosRight = MapTileCoordsToWorld(map, groundSensorEndRightTilePos);
		RenderPushRectangleCenter(renderState, V2fAdd(worldPosLeft, TileRadius), TileRadius, groundSensorTileColor, false, 1.0f);
		RenderPushRectangleCenter(renderState, V2fAdd(worldPosRight, TileRadius), TileRadius, groundSensorTileColor, false, 1.0f);
	}

	// Get player bounds
	AABB2f playerMotionBounds = EntityGetMotionBounds(player, dt);
	AABB2fExpandScalar(&playerMotionBounds, PhysicsAABBExpansion);

	// Render collision tile bounds
	bool renderPlayerTileBounds = state->mode != GameMode_EditorPause;
	if (renderPlayerTileBounds) {
		// Tile tiles area from AABB
		TileBounds tileBounds = MapGetTileBounds(map, &playerMotionBounds);

		for (int x = tileBounds.min.x; x <= tileBounds.max.x; ++x) {
			for (int y = tileBounds.min.y; y <= tileBounds.max.y; ++y) {
				Vec2i tilePos = V2iInit(x, y);
				Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
				Vec2f tileCenter = V2fAdd(worldPos, TileRadius);
				RenderPushRectangleCenter(renderState, tileCenter, TileRadius, playerTileColor, false, 1.0f);
			}
		}
	}

	// Contacts
	bool renderContacts = state->mode != GameMode_EditorPause;
	if (renderContacts) {
		for (size_t i = 0; i < physics->contactList.used; ++i) {
			const Contact *contact = physics->contactList.data + i;
			RenderPushCircle(renderState, contact->posA, 2.0f, 16, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
			RenderPushCircle(renderState, contact->posB, 2.0f, 16, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), true, 0.0f);
			RenderNormal(renderState, contact->posB, contact->normal, 10.0f, 5.0f);
		}
	}

	// Mouse cursor
	if (state->mode != GameMode_Game) {
		RenderPushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(2, 2), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
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
		RenderSetMatrix(renderState, &state->projection);

		const FontAsset *font = &assets->consoleFont;
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec4f blackColor = V4fInit(0, 0, 0, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 6.0f;

		// Game memory
		FormatSize(gameMemory->memory->used, fplArrayCount(debugOSDSizeCharBuffer[0]), debugOSDSizeCharBuffer[0]);
		FormatSize(gameMemory->memory->size, fplArrayCount(debugOSDSizeCharBuffer[1]), debugOSDSizeCharBuffer[1]);
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Game Memory: %s / %s bytes", debugOSDSizeCharBuffer[0], debugOSDSizeCharBuffer[1]);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		// Render memory
		FormatSize(renderState->lastMemoryUsage, fplArrayCount(debugOSDSizeCharBuffer[0]), debugOSDSizeCharBuffer[0]);
		FormatSize(renderState->memory.size, fplArrayCount(debugOSDSizeCharBuffer[1]), debugOSDSizeCharBuffer[1]);
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Render Memory: %s / %s bytes", debugOSDSizeCharBuffer[0], debugOSDSizeCharBuffer[1]);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w - 1, blockPos.y - 1), fontHeight, 0.0f, -1.0f, blackColor);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);

		// Frame timings
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f - 1, blockPos.y - 1), fontHeight, -1.0f, -1.0f, blackColor);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);

		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight * 2.0f));

		// Game state
		const char *gameModeName = gGameModeNames[state->mode];
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Game Mode: %s", gameModeName);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));

		// Player states
		fplStringFormat(debugOSDCharBuffer, fplArrayCount(debugOSDCharBuffer), "Player ground: %s", player->groundState.current ? "yes" : "no");
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		RenderPushText(renderState, debugOSDCharBuffer, fplGetStringLength(debugOSDCharBuffer), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);
		blockPos = V2fSub(blockPos, V2fInit(0, fontHeight));
	}

	// Render editor OSD
	bool drawEditorOSD = state->mode >= GameMode_EditorPlay;
	if (drawEditorOSD) {
		RenderSetMatrix(renderState, &state->projection);
		EditorOSDRender(renderState, editor, input);
	}

	// Overwrite render infos such as position, rotation for "previous" from "current" states
	// This is important for getting smooth interpolated rendering
	player->transform[1] = player->transform[0];
	state->camera.transform[1] = state->camera.transform[0];
}

// Directly included translation units
#include "common.c"
#include "assets.c"
#include "entity.c"
#include "map.c"
#include "world.c"
#include "physics.c"
#include "editor.c"

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