/*
Name:
	Final Game

Description:
	Structures, Functions for setting up a game quickly.
	Also declares fpl_extern functions for init/kill/update/render a game instance.

	This file is part of the final_framework.

Changelog:
	## 2025-11-21
	- Changed: GameRender to allow an Input argument as well
	- Changed: Renamed ButtonWasPressed to ButtonButtonWasPressed
	- Changed: Renamed IsDown to ButtonIsDown
	- Changed: Use fpl_inline intead of inline
	- Changed: Use fpl_extern intead of fpl_extern
	
License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_GAME_H
#define FINAL_GAME_H

#include <final_platform_layer.h>

#include "final_math.h"
#include "final_render.h"
#include "final_audiosystem.h"
#include "final_memory.h"
#include "final_log.h"

typedef struct ButtonState {
	int halfTransitionCount;
	fpl_b32 endedDown;
} ButtonState;

fpl_inline bool ButtonIsDown(const ButtonState state) {
	bool result = state.endedDown != 0;
	return(result);
}

fpl_inline bool ButtonWasPressed(const ButtonState state) {
	bool result = ((state.halfTransitionCount > 1) ||
				  ((state.halfTransitionCount == 1) && (!state.endedDown)));
	return(result);
}

typedef struct Controller {
	Vec2f analogMovement;
	union {
		struct {
			ButtonState moveUp;
			ButtonState moveDown;
			ButtonState moveLeft;
			ButtonState moveRight;
			ButtonState actionUp;
			ButtonState actionDown;
			ButtonState actionLeft;
			ButtonState actionRight;
			ButtonState actionBack;
			ButtonState actionStart;
		};
		ButtonState buttons[10];
	};
	bool isConnected;
	bool isAnalog;
} Controller;

typedef struct Mouse {
	Vec2i pos;
	float wheelDelta;
	union {
		struct {
			ButtonState left;
			ButtonState middle;
			ButtonState right;
		};
		ButtonState buttons[3];
	};
} Mouse;

typedef struct Keyboard {
	ButtonState keys[256];
} Keyboard;

typedef struct Input {
	Keyboard tastatur;

	union {
		struct {
			Controller keyboard;
			Controller gamepad[4];
		};
		Controller controllers[5];
	};
	Mouse mouse;

	// Size of window in pixels
	Vec2i windowSize;
	// Fixed delta time in seconds, used for game update or physics
	float fixedDeltaTime;
	// Dynamic frame time in seconds
	float dynamicFrameTime;
	// Current frames per seconds
	float framesPerSeconds;
	// Current index of the rendered frame
	int frameIndex;
	// Index to the default controller
	int defaultControllerIndex;

	// Indicates that the application is active or not -> only handle any input when this is true!
	bool isActive;
	// Indicates that the first GameUpdate() is called inside the Accumulator-Loop -> use this to prevent multiple key transitions by ButtonWasPressed()
	bool isFirstUpdateOfFrame;
} Input;

struct GameState;

typedef struct GameMemory {
	Input inputs[2];
	fplKeyboardState keyboard;
	AudioSystem *audio;
	RenderState *render;
	fmemMemoryBlock *memory;
	struct GameState *game;
} GameMemory;

typedef enum GameWindowActiveType {
	GameWindowActiveType_None = 0,
	GameWindowActiveType_GotFocus = 1 << 0,
	GameWindowActiveType_LostFocus = 1 << 1,
	GameWindowActiveType_Minimized = 1 << 2,
	GameWindowActiveType_Maximized = 1 << 3,
	GameWindowActiveType_Restored = 1 << 4,
} GameWindowActiveType;
FPL_ENUM_AS_FLAGS_OPERATORS(GameWindowActiveType);

fpl_extern bool GameInit(GameMemory *gameMemory, const int argumentCount, char **arguments);
fpl_extern void GameRelease(GameMemory *gameMemory);
fpl_extern void GameInput(GameMemory *gameMemory, const Input *input);
fpl_extern void GameUpdate(GameMemory *gameMemory, const Input *input);
fpl_extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha);
fpl_extern bool IsGameExiting(GameMemory *gameMemory);

#endif // FINAL_GAME_H