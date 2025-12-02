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

#define FINAL_LOG_IMPLEMENTATION
#include <final_log.h>

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
#include "game.h"

// Directly included translation units
#include "common.c"
#include "assets.c"
#include "entity.c"
#include "map.c"
#include "world.c"
#include "physics.c"
#include "editor.c"
#include "game.c"

//
// Entry point for one-time game initialization
//
fpl_extern bool GameInit(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return false; // Invalid arguments
	}

	fmemMemoryBlock *memory = gameMemory->memory;
	fplAssert(memory != fpl_null);

	GameState *gameState = (GameState *)fmemPush(memory, sizeof(GameState), fmemPushFlags_Clear);
	if (gameState == fpl_null) {
		return false; // Insufficient memory for game state
	}
	gameMemory->game = gameState;

	RenderState *renderState = gameMemory->render;

	if (!PlatformerGameInit(gameMemory, renderState, gameState)) {
		return false; // Failed to initialize game state
	}

	return true;
}

//
// Entry point for one-time game release
//
fpl_extern void GameRelease(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return; // Invalid arguments
	}
	GameState *gameState = gameMemory->game;
	if (gameState == fpl_null) {
		return; // Cannot release game state, because it is already freed
	}
	PlatformerGameRelease(gameMemory, gameState);
}

//
// Entry point for getting a value indicating whether the game is exiting or not
//
fpl_extern bool IsGameExiting(GameMemory *gameMemory) {
	if (gameMemory == fpl_null) {
		return false; // Invalid arguments
	}
	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);
	return PlatformerGameIsExiting(gameMemory, state);
}

//
// Entry point for handling the game input
//
fpl_extern void GameInput(GameMemory *gameMemory, const Input *input) {
	fplAssert(gameMemory != fpl_null);
	fplAssert(input != fpl_null);

	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);

	RenderState *renderState = gameMemory->render;
	fplAssert(renderState != fpl_null);

	if (!input->isActive) {
		return;
	}

	PlatformerGameInput(gameMemory, state, renderState, input);
}

//
// Entry point for updating the game logic
//
fpl_extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	fplAssert(gameMemory != fpl_null);
	fplAssert(input != fpl_null);

	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);

	if (!input->isActive) {
		return;
	}

	PlatformerGameUpdate(gameMemory, state, input);
}


//
// Entry point for rendering the current game frame
//
fpl_extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha) {
	fplAssert(gameMemory != fpl_null);
	fplAssert(input != fpl_null);

	GameState *state = gameMemory->game;
	fplAssert(state != fpl_null);

	RenderState *renderState = gameMemory->render;
	fplAssert(renderState != fpl_null);

	PlatformerGameRender(gameMemory, state, renderState, input, alpha);
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