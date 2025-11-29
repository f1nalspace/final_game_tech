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

# Project

[X] Moved game functions into its own header and source file

# Level

[ ] Map serialization
[ ] Map save & load
[ ] Map entity serialization

# Physics

[ ] Jump through platforms
[ ] Body types (Static, Kinematic, Dynamic)
[ ] Dynamic vs Kinematic collision response

# Game mechanics

[X] Camera ID + Active camera switching
[ ] Smooth camera motion towards a target
[ ] Fix entity jump requesting (infinite jumps, while holding space)
[ ] Hurting entities
[ ] Simple doors
[ ] Opening/Closing doors with levers
[ ] Simple trampoline (walking does not do anything, but jumping and only in falling state raises the upward impulse)
[ ] Moveable Platform controlled by Player with line-raying stops
[ ] Jumppad velocity computation based on start and target position and gravity

# Entity

[ ] Entity type
[ ] Door entity
[ ] Spike entity
[ ] Trampoline entity
[ ] Platform entity
[ ] Data oriented entities (separate movement, rendering, states, etc.)

# Editor

[X] Camera translation by holding space and left mouse button
[ ] Entity selection & placement editor (Doors, Spike, Trampoline, Platform, PlayerStart, etc.)

# Rendering

[ ] Player character animated sprite (Idle, walk, jump, fall, die, hit, activate)

# UI

[ ] Immediate UI library (Panel, Button, Progressbar, Trackbar, Label, Checkbox, Scrollbar)
[ ] Extended UI (TabSpinEdit, Colorpicker, Textbox)

# Debug

[ ] Console

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