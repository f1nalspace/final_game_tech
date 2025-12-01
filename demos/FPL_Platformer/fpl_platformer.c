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

Todo-List:

# Core / Platform

[X] Create project based on FPL_GameTemplate and build a skeleton for a our game functions
[X] Use the game memory block everything and replace malloc with that
[X] Split game memory into persistent and transient memory
[X] Give each system its own persistent and transient memory, remove all malloc/free calls
[X] Define sizes for each memory blocks in each system header, e.g. world.h, map.h, physics.h etc.
[X] Find and add a json loader single header file library
[X] Moved game functions into its own header and source file
[ ] Simple debug out logging system
[ ] Create a version type or use fplFileVersion that can be serialized

# Assets

[X] Proper memory handling by transient/persistent storage
[X] Create tileset for auto-tiles
[X] Define tile area masks for the most common auto-tiling cases
[X] Create tiles for entities such as (Player Start, Spike, Trampoline, Moving Platform)
[ ] Create tiles for entities such as (Levers, Doors, Lava)
[ ] Find a nice animated player sprite that fits the industrial theme (maybe a robot or something?)
[ ] Free opengl textures when the game finishes (not easy in the way final_render.h works)
[ ] Configuration JSON file, that may store infos such level, video, audio settings, etc.

# Level

[X] Introduce a persistent and transient memory
[X] Make map resizable in width and height
[X] Dynamic resizable map in all directions based on a negative offset
[X] Introduce and compute tile area mask, that defines which tiles around a tile are the same or not (required for auto-tiling)
[X] Use json loader and experiment with loading a LDtk json file
[ ] Map serialization into a JSON file (don't forget to set a "version" value)
[ ] Map de-serialization from a JSON file
[ ] Map entity serialization directly into the the map json file

# Physics

[X] Basic contact generation for AABB vs AABB using closest points from grown AABB to a point -> closest point on plane
[X] Fix ghost collisions contact generation AABB vs AABB by preventing using tiles in the same direction that is solid
[X] Implement collision response using speculative contacts
[X] Prevent edge collision contacts
[X] Define correct contact points A and B
[X] Contact list storage
[ ] Detect entity vs entity collisions
[ ] Collision detection callbacks
[ ] Jump through platforms
[ ] Body types (Static, Kinematic, Dynamic)
[ ] Dynamic vs Kinematic collision response

# Game mechanics

[X] Camera ID + Active camera switching
[ ] Smooth camera motion towards a target (code is already defined!)
[ ] Fix entity jump requesting (prevent infinite jumps while holding space)
[ ] Handle entity collisions and kill the player when touching a spike, that will restart the level
[ ] Simple doors
[ ] Opening/Closing doors with levers
[ ] Simple trampoline (walking does not do anything, but jumping and only in falling state raises the upward impulse)
[ ] Moveable Platform controlled by Player with line-raying stops
[ ] Jumppad velocity computation based on start and target position and gravity

# Entity

[X] Define a entity which is a player for now
[X] Make entity movable by the current controller (for now in all directions)
[X] Handle collisions with simple impulse responses from solid tiles
[X] Predict entity position and compute motion bounds
[X] Improve stability and performance by checking only tiles the player motion bounds is touching
[X] Expand motion bounds by a small value, to fix instabilities of the contact solver
[X] Extent entity to a position accumulator, so we can prevent any penetrations in the first place
[X] Clamp entity horizontal and vertical speed to prevent overshoot
[X] Apply gravity and detect if entity is on the ground or in the air
[X] Introduce proper ground state and detect state switches
[X] Make entity jumpable
[X] Introduce a transform with a current and last position / rotation, so we can interpolate it
[ ] Introduce entity types
[ ] Door entity
[ ] Spike entity
[ ] Trampoline entity
[ ] Platform entity
[ ] Data oriented entities (separate movement, rendering, states, etc.)

# Editor

[X] Detect which tile the mouse is over, regardless of the camera transformation and scale
[X] Easiely Paint solid and non-solid tiles with the left mouse button holding down
[X] Simple resize the map in width and height, when clicking outside
[X] Introduce a offset, so we can place the map everywhere we want
[X] Grow the map in all directions automatically, allow the offset to be negative
[X] Make editor more robust
[X] Camera translation by holding space and left mouse button
[ ] Fix mouse is still detected as "holding down", even though the button is not pressed at all anymore (this may be a general issue of final_gameplatform.h!)
[ ] Entity selection & placement editor (Doors, Spike, Trampoline, Platform, PlayerStart, etc.)

# Rendering

[X] Draw a simple tile map with either solid or non-solid state
[X] Draw a player rectangle and highlight the touching tiles
[X] Draw normals and origins
[X] Draw world bounds
[X] Introduce a simple camera with translation and scale
[X] Fix world screen size independent screen rendering was not working correctly when scaling
[X] Fix flickering bug when the camera follows the player (missing alpha interpolation)
[ ] Entity character animation system (Idle, walk, jump, fall, die, hit, activate)

# UI

[ ] Immediate UI library (Panel, Button, Progressbar, Trackbar, Label, Checkbox, Scrollbar)
[ ] File selection UI Dialog (call native one!)
[ ] Extended UI (TabSpinEdit, Colorpicker, Textbox)
[ ] Top file menu UI with drop down and submenus

# Debug

[ ] Quake like console

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