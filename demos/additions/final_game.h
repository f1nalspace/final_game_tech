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

Changelog:
	## 2018-06-29
	- Changed to use new keyboard/mouse button state
*/

#ifndef FINAL_GAME_H
#define FINAL_GAME_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include <final_platform_layer.h>

#include "final_math.h"
#include "final_render.h"

struct ButtonState {
	fplButtonState state;
	int halfTransitionCount;
};

inline bool WasPressed(const ButtonState &state) {
	bool result = ((state.halfTransitionCount > 1) || ((state.halfTransitionCount == 1) && (state.state == fplButtonState_Release)));
	return(result);
}
inline bool IsDown(const ButtonState &state) {
	bool result = state.state >= fplButtonState_Press;
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
			ButtonState actionStart;
			ButtonState debugToggle;
		};
		ButtonState buttons[11];
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
	float framesPerSeconds;
	union {
		struct {
			Controller keyboard;
			Controller gamepad[4];
		};
		Controller controllers[5];
	};
	Mouse mouse;
	Vec2i windowSize;
	int defaultControllerIndex;
	bool isActive;
};

struct GameMemory {
	void *base;
	size_t capacity;
	size_t used;
};

enum class GameWindowActiveType : uint32_t {
	None = 0,
	GotFocus = 1 << 0,
	LostFocus = 1 << 1,
	Minimized = 1 << 2,
	Maximized = 1 << 3,
	Restored = 1 << 4,
};
FPL_ENUM_AS_FLAGS_OPERATORS(GameWindowActiveType);

extern GameMemory GameCreate();
extern void GameDestroy(GameMemory &gameMemory);
extern void GameInput(GameMemory &gameMemory, const Input &input);
extern void GameUpdate(GameMemory &gameMemory, const Input &input);
extern void GameRender(GameMemory &gameMemory, RenderState &renderState, const float alpha);
extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, RenderState &renderState, const float alpha);
extern bool IsGameExiting(GameMemory &gameMemory);

#endif // FINAL_GAME_H