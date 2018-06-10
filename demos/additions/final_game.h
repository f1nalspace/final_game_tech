/*
Name:
	Final Game
Description:
	Structures, Functions for setting up a game quickly.
	Also declares extern functions for init/kill/update/render a game instance.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_GAME_H
#define FINAL_GAME_H

#if !defined(__cplusplus)
#error "C++ is required for this to run!"
#endif

#include <final_platform_layer.h>

#include "final_vecmath.h"

struct ButtonState {
	bool isDown;
	int halfTransitionCount;
};

inline bool WasPressed(const ButtonState &state) {
	bool result = ((state.halfTransitionCount > 1) || ((state.halfTransitionCount == 1) && (state.isDown)));
	return(result);
}

struct Controller {
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
		};
		ButtonState buttons[9];
	};
};

struct Mouse {
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
};

struct Input {
	float deltaTime;
	union {
		struct {
			Controller keyboard;
			Controller gamepad[4];
		};
		Controller controllers[5];
	};
	int defaultControllerIndex;
	Mouse mouse;
	Vec2i windowSize;
};

struct GameMemory {
	void *base;
	size_t capacity;
	size_t used;
};

extern GameMemory GameCreate();
extern void GameDestroy(GameMemory &gameMemory);
extern void GameInput(GameMemory &gameMemory, const Input &input, bool isActive);
extern void GameUpdate(GameMemory &gameMemory, const Input &input, bool isActive);
extern void GameRender(GameMemory &gameMemory, const float alpha);
extern bool IsGameExiting(GameMemory &gameMemory);

#endif // FINAL_GAME_H