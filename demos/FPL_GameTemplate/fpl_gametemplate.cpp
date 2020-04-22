/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | GameTemplate

Description:
	Simple c++ template containing a basic skeleton for a game.

Requirements:
	- C++ Compiler
	- Final Framework

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
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

#include "fpl_gametemplate.h"

//
// Utils
//
static void FormatSize(const size_t value, const size_t maxCount, char *buffer) {
	char *p = buffer;
	if (value < 0) {
		*p++;
	}

	size_t tmp = value;
	do {
		*p++;
		tmp = tmp / 10;
	} while (tmp);

	// Count thousands
	size_t thousandDotCount = 0;
	tmp = value;
	while (tmp >= 1000) {
		*p++;
		++thousandDotCount;
		tmp = tmp / 1000;
	}

	size_t charCount = p - buffer;

	fplAssert(charCount + 1 <= maxCount);

	*p = 0;
	const char *digits = "0123456789";
	size_t v = value;
	size_t c = 0;
	size_t t = thousandDotCount;
	do {
		if (t > 0) {
			if (c == 3) {
				c = 0;
				--t;
				*--p = '.';
			}
		}
		*--p = digits[v % 10];
		v /= 10;
		c++;
	} while (v != 0);
}

//
// Constants
//
constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
constexpr float WorldRadiusW = WorldWidth * 0.5f;
constexpr float WorldRadiusH = WorldHeight * 0.5f;

constexpr float DefaultLineWidth = 2.0f;

constexpr int TileCountX = 21;
constexpr int TileCountY = 11;
constexpr float TileWidth = WorldWidth / (float)TileCountX;
constexpr float TileHeight = WorldHeight / (float)(TileCountY + 1);
const static Vec2f TileSize = V2fInit(TileWidth, TileHeight);
constexpr float MaxTileSize = fplMax(TileWidth, TileHeight);

static void LoadAssets(RenderState &renderState, Assets &assets) {
	// Fonts
	char fontDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(fontDataPath, fplArrayCount(fontDataPath), 2, assets.dataPath, "fonts");
	FontAsset &hudFont = assets.consoleFont;
	if (LoadFontFromFile(fontDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont.desc)) {
		PushTexture(renderState, &hudFont.texture, hudFont.desc.atlasAlphaBitmap, hudFont.desc.atlasWidth, hudFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
	}
}

static void FreeAssets(Assets &assets) {
	ReleaseFontAsset(assets.consoleFont);
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

extern bool GameInit(GameMemory &gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;

	RenderState *renderState = gameMemory.render;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	LoadAssets(*renderState, state->assets);

	InitGame(state);

	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	if (state != nullptr) {
		FreeAssets(state->assets);
		state->~GameState();
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	RenderState *renderState = gameMemory.render;
	assert(renderState != nullptr);

	// Debug input
	const Controller &keyboardController = input.controllers[0];
	if (WasPressed(keyboardController.debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input.windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	Mat4f proj = Mat4OrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	Mat4f view = Mat4TranslationV2(state->camera.offset);
	state->viewProjection = proj * view;

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	const float dt = input.deltaTime;

	World &world = state->world;
	Entity &player = world.player;

	// Player
	const Controller &keyboardController = input.controllers[0];
	Vec2f movement = V2fInit(0.0f, 0.0f);
	if (IsDown(keyboardController.moveUp)) {
		movement += V2fInit(0.0f, 1.0f) * player.moveSpeed;
	} else if (IsDown(keyboardController.moveDown)) {
		movement += V2fInit(0.0f, -1.0f) * player.moveSpeed;
	}
	if (IsDown(keyboardController.moveLeft)) {
		movement += V2fInit(-1.0f, 0.0f) * player.moveSpeed;
	} else if (IsDown(keyboardController.moveRight)) {
		movement += V2fInit(1.0f, 0.0f) * player.moveSpeed;
	}

	// Apply movement
	if (V2fDot(movement, movement) > 0) {
		player.velocity += movement * dt;
	}

	// Apply drag
	if (V2fDot(player.velocity, player.velocity) > 0) {
		float len = V2fLength(player.velocity);
		float invLen = 1.0f / len;
		Vec2f vdir = player.velocity * invLen;
		float newVelocity = len * (1.0f - player.moveDrag);
		player.velocity = vdir * newVelocity;
	}

	// Integrate position
	player.position += player.velocity * dt;

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input.framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = ScalarAvg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);

	RenderState &renderState = *gameMemory.render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags::Color | ClearFlags::Depth);
	SetMatrix(renderState, state->viewProjection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);

	// Tile grid
	Vec2f gridOrigin = V2fInit(-w, -h);
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	Vec2f gridSize = V2fInit(w, h) * 2.0f;
	for (int i = 0; i <= TileCountX; ++i) {
		float xoffset = i * TileWidth;
		PushLine(renderState, gridOrigin + V2fInit(xoffset, 0.0f), gridOrigin + V2fInit(xoffset, gridSize.y), gridColor, 1.0f);
	}
	for (int i = 0; i <= TileCountY; ++i) {
		float yoffset = i * TileHeight;
		PushLine(renderState, gridOrigin + V2fInit(0.0f, yoffset), gridOrigin + V2fInit(gridSize.x, yoffset), gridColor, 1.0f);
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
		Vec2f p = gridOrigin + V2fHadamard(V2fInit((float)tilePosInt.x, (float)tilePosInt.y), TileSize);
		PushRectangle(renderState, p, TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);
	}

	if (state->isDebugRendering) {
		const FontAsset &font = state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = MaxTileSize * 0.25f;

		char sizeCharsBuffer[2][32 + 1];
		FormatSize(gameMemory.memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory.memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplFormatString(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState.lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState.memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplFormatString(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplFormatString(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);
	}
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
	GameInput(gameMemory, input);
	GameUpdate(gameMemory, input);
	GameRender(gameMemory, alpha);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = L"FPL Demo | GameTemplate";
	config.disableInactiveDetection = true;
	config.noUpdateRenderSeparation = true;
	int result = GameMain(config);
	return(result);
}