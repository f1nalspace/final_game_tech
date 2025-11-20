/*
Name:
	Final Game

Description:
	Structures, Functions for setting up a game quickly.
	Also declares extern functions for init/kill/update/render a game instance.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_GAME_H
#define FINAL_GAME_H

#include <final_platform_layer.h>

#include "final_math.h"
#include "final_render.h"
#include "final_audiosystem.h"
#include "final_memory.h"

typedef struct ButtonState {
	int halfTransitionCount;
	fpl_b32 endedDown;
} ButtonState;

inline bool IsDown(const ButtonState state) {
	bool result = state.endedDown != 0;
	return(result);
}

inline bool WasPressed(const ButtonState state) {
	bool result = ((state.halfTransitionCount > 1) ||
				  ((state.halfTransitionCount == 1) && (!state.endedDown)));
	return(result);
}

inline bool WasReleased(const ButtonState state) {
	bool result = ((state.halfTransitionCount > 1) ||
				  ((state.halfTransitionCount == 1) && (state.endedDown)));
	return(result);
}

typedef struct Controller {
	bool isConnected;
	bool isAnalog;
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
			ButtonState debugToggle;
			ButtonState debugReload;
		};
		ButtonState buttons[12];
	};
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

typedef struct Input {
	// Fixed delta time in seconds, used for game update or physics
	float fixedDeltaTime;
	// Dynamic frame time in seconds
	float dynamicFrameTime;
	// Current frames per seconds
	float framesPerSeconds;

	// Current index of the rendered frame
	int frameIndex;

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
	// Index to the default controller
	int defaultControllerIndex;
	// Indicates that the application is active or not -> only handle any input when this is true!
	bool isActive;
	// Indicates that the first GameUpdate() is called inside the Accumulator-Loop -> use this to prevent multiple key transitions by WasPressed() and WasReleased()!
	bool isFirstUpdateOfFrame;
} Input;

struct GameState;

typedef struct GameMemory {
	fmemMemoryBlock *memory;
	struct GameState *game;
	RenderState *render;
	AudioSystem *audio;
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

extern bool GameInit(GameMemory *gameMemory);
extern void GameRelease(GameMemory *gameMemory);
extern void GameInput(GameMemory *gameMemory, const Input *input);
extern void GameUpdate(GameMemory *gameMemory, const Input *input);
extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha);
extern void GameUpdateAndRender(GameMemory *gameMemory, const Input *input, const float alpha);
extern bool IsGameExiting(GameMemory *gameMemory);

#endif // FINAL_GAME_H