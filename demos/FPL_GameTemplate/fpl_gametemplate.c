/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | GameTemplate

Description:
	Simple C99 template containing a basic skeleton for a game.

Requirements:
	- C99 Compiler
	- Final Framework

Author:
	Torsten Spaete

Changelog:
	## 2025-11-21
	- Changed: Reflect for API changes final_game.h (GameRender has an additional Input argument)
	- Changed: Reflect for API changes final_game.h (WasPressed / IsDown was renamed to ButtonWasPressed / ButtonIsDown)

	## 2025-11-16
	- Switched from C++ to C99

	## 2025-11-15
	- Fixed only keyboard controller was used
	- Migrated to new final addition libraries (C99 standard)

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#include <final_game.h>

#include <final_fonts.h>

#include "fpl_gametemplate.h"

//
// Constants
//
#define GameAspect (16.0f / 9.0f)
#define WorldWidth 20.0f
#define WorldHeight (WorldWidth / GameAspect)
#define WorldRadiusW (WorldWidth * 0.5f)
#define WorldRadiusH (WorldHeight * 0.5f)

#define DefaultLineWidth 2.0f

#define TileCountX 21
#define TileCountY 11
#define TileWidth (WorldWidth / (float)TileCountX)
#define TileHeight (WorldHeight / (float)(TileCountY + 1))
#define TileSize V2fInit(TileWidth, TileHeight)
#define MaxTileSize fplMax(TileWidth, TileHeight)

static void LoadAssets(RenderState *renderState, Assets *assets) {
	// Fonts
	FontAsset *hudFont = &assets->consoleFont;
	if (LoadFontFromMemory(ptr_fontVeraFontRegular, sizeOf_fontVeraFontRegular, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		PushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);
	}
}

static void FreeAssets(Assets *assets) {
	ReleaseFontAsset(&assets->consoleFont);
}

static void InitGame(GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// Input
	state->isDebugRendering = true;

	// Player
	state->world.player.radius = V2fInit(MaxTileSize * 0.4f, MaxTileSize);
	state->world.player.position = V2fInit(0.0f, 0.0f);
	state->world.player.velocity = V2fInit(0.0f, 0.0f);
	state->world.player.color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	state->world.player.moveSpeed = MaxTileSize * 20.0f;
	state->world.player.moveDrag = 0.1f;
}

extern bool GameInit(GameMemory *gameMemory) {
	if(gameMemory == fpl_null) {
		return false;
	}

	GameState *state = (GameState *)fmemPush(gameMemory->memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory->game = state;

	RenderState *renderState = gameMemory->render;

	Assets *assets  = &state->assets;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	LoadAssets(renderState, assets);

	InitGame(state);

	return(true);
}

extern void GameRelease(GameMemory *gameMemory) {
	if(gameMemory == fpl_null) {
		return;
	}

	GameState *state = gameMemory->game;
	if(state == fpl_null) {
		return;
	}
	FreeAssets(&state->assets);
}

extern bool IsGameExiting(GameMemory *gameMemory) {
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

	// Debug input
	const Controller *keyboardController = &input->controllers[0];
	if (ButtonWasPressed(keyboardController->debug4)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input->windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	Mat4f proj = M4fOrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	Mat4f view = M4fTranslationV2(state->camera.offset);
	state->viewProjection = M4fMult(proj, view);

	// Mouse
	int mouseCenterX = (input->mouse.pos.x - input->windowSize.w / 2);
	int mouseCenterY = (input->windowSize.h - 1 - input->mouse.pos.y) - input->windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;
}

extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	const float dt = input->fixedDeltaTime;

	World *world = &state->world;
	Entity *player = &world->player;

	// Player
	Vec2f movement = V2fInit(0.0f, 0.0f);
	if(input->defaultControllerIndex > -1 && input->defaultControllerIndex < fplArrayCount(input->controllers)){
		const Controller *controller = &input->controllers[input->defaultControllerIndex];
		if(ButtonIsDown(controller->moveUp)) {
			movement = V2fAddMultScalar(movement, V2fInit(0.0f, 1.0f), player->moveSpeed);
		} else if(ButtonIsDown(controller->moveDown)) {
			movement = V2fAddMultScalar(movement, V2fInit(0.0f, -1.0f), player->moveSpeed);
		}
		if(ButtonIsDown(controller->moveLeft)) {
			movement = V2fAddMultScalar(movement, V2fInit(-1.0f,0.0f), player->moveSpeed);
		} else if(ButtonIsDown(controller->moveRight)) {
			movement = V2fAddMultScalar(movement, V2fInit(1.0f,0.0f), player->moveSpeed);
		}
	}

	// Apply movement
	if (V2fDot(movement, movement) > 0) {
		player->velocity = V2fAddMultScalar(player->velocity, movement, dt);
	}

	// Apply drag
	if (V2fDot(player->velocity, player->velocity) > 0) {
		float len = V2fLength(player->velocity);
		float invLen = 1.0f / len;
		Vec2f vdir = V2fMultScalar(player->velocity, invLen);
		float newVelocity = len * (1.0f - player->moveDrag);
		player->velocity = V2fMultScalar(vdir, newVelocity);
	}

	// Integrate position
	player->position = V2fAddMultScalar(player->position, player->velocity, dt);

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input->framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = F32Avg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha) {
	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	RenderState *renderState = gameMemory->render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags_Color | ClearFlags_Depth);
	SetMatrix(renderState, &state->viewProjection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);

	// Tile grid
	Vec2f gridOrigin = V2fInit(-w, -h);
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	Vec2f gridSize = V2fInit(w * 2.0f, h * 2.0f);
	for (int i = 0; i <= TileCountX; ++i) {
		float xoffset = i * TileWidth;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(xoffset,0.0f));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(xoffset, gridSize.y));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}
	for (int i = 0; i <= TileCountY; ++i) {
		float yoffset = i * TileHeight;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(0.0f, yoffset));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(gridSize.x, yoffset));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}

	// Player
	PushRectangleCenter(renderState, state->world.player.position, state->world.player.radius, state->world.player.color, true, 0.0f);

	// Mouse tile
	Vec2f invTileSize = V2fInit(1.0f / TileWidth, 1.0f / TileHeight);
	if (state->mouseWorldPos.x >= -w && state->mouseWorldPos.x <= w &&
		state->mouseWorldPos.y >= -h && state->mouseWorldPos.y <= h) {
		Vec2f gridPos = V2fInit(state->mouseWorldPos.x - gridOrigin.x, state->mouseWorldPos.y - gridOrigin.y);
		Vec2f tilePosFloat = V2fHadamard(gridPos, invTileSize);
		Vec2i tilePosInt = V2iInit((int)tilePosFloat.x, (int)tilePosFloat.y);
		Vec2f p = V2fAdd(gridOrigin, V2fHadamard(V2fInit((float)tilePosInt.x, (float)tilePosInt.y), TileSize));
		PushRectangle(renderState, p, TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);
	}

	if (state->isDebugRendering) {
		const FontAsset *font = &state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = MaxTileSize * 0.25f;

		char sizeCharsBuffer[2][32 + 1];
		FormatSize(gameMemory->memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory->memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState->lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState->memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplStringFormat(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);
	}
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = fplZeroInit;
	config.title = "FPL Demo | GameTemplate";
	config.disableInactiveDetection = true;
	int result = GameMain(&config);
	return(result);
}