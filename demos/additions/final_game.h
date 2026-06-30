/*
Name:
	Final Game

Description:
	Structures, Functions for setting up a game quickly.
	Also declares fpl_extern functions for init/kill/update/render a game instance.

	This file is part of the final_framework.

Changelog:
	## 2026-06-30
	- Added ControllerButtonType_ShoulderLeft and ControllerButtonType_ShoulderRight
	- Increased MAX_KEYBOARD_CONTROLLER_BUTTON_MAPPING_COUNT from 32 to 128

	## 2026-06-08
	- New: Added function ButtonWentDown() that returns whether a button went down this frame (the press/down edge)

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

fpl_extern_inline bool ButtonIsDown(const ButtonState state) {
	bool result = state.endedDown != 0;
	return(result);
}

fpl_extern_inline bool ButtonWasPressed(const ButtonState state) {
	bool result = ((state.halfTransitionCount > 1) ||
				  ((state.halfTransitionCount == 1) && (!state.endedDown)));
	return(result);
}

// True on the DOWN edge (key went down this frame). Use this for press reactions
// like a jump; ButtonWasPressed() above actually fires on the release edge.
fpl_extern_inline bool ButtonWentDown(const ButtonState state) {
	bool result = ((state.endedDown && state.halfTransitionCount >= 1) ||
				  (!state.endedDown && state.halfTransitionCount >= 2));
	return(result);
}

typedef enum ControllerButtonType {
	ControllerButtonType_MoveUp = 0,
	ControllerButtonType_MoveDown,
	ControllerButtonType_MoveLeft,
	ControllerButtonType_MoveRight,
	ControllerButtonType_ActionUp,
	ControllerButtonType_ActionDown,
	ControllerButtonType_ActionLeft,
	ControllerButtonType_ActionRight,
	ControllerButtonType_ActionBack,
	ControllerButtonType_ActionStart,
	ControllerButtonType_ShoulderLeft,
	ControllerButtonType_ShoulderRight,

	ControllerButtonType_Count,

	ControllerButtonType_First = ControllerButtonType_MoveUp,
	ControllerButtonType_Last = ControllerButtonType_ShoulderRight,
} ControllerButtonType;

// Total number of controller button types
#define MAX_CONTROLLER_BUTTON_TYPE_COUNT (ControllerButtonType_Count)

// Total number of controller buttons
#define MAX_CONTROLLER_BUTTON_COUNT 12

fplStaticAssert(MAX_CONTROLLER_BUTTON_TYPE_COUNT == MAX_CONTROLLER_BUTTON_COUNT);

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
			ButtonState leftShoulder;
			ButtonState rightShoulder;
		};
		ButtonState buttons[MAX_CONTROLLER_BUTTON_COUNT];
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

typedef struct KeyboardControllerButtonMapping {
	fplKey key;
	ControllerButtonType type;
} KeyboardControllerButtonMapping;

// Total number of keyboard controller button mappings
#define MAX_KEYBOARD_CONTROLLER_BUTTON_MAPPING_COUNT 128

typedef struct KeyboardButtonMappings {
	KeyboardControllerButtonMapping values[MAX_KEYBOARD_CONTROLLER_BUTTON_MAPPING_COUNT];
	uint32_t count;
	bool isCustom;
} KeyboardButtonMappings;

typedef struct KeyboardButtonStates {
	fplButtonState states[MAX_CONTROLLER_BUTTON_COUNT];
	bool changed[MAX_CONTROLLER_BUTTON_COUNT];
	bool mapped[MAX_CONTROLLER_BUTTON_COUNT];
} KeyboardButtonStates;

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