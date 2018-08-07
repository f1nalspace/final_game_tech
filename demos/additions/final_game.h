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

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include <final_platform_layer.h>

#include "final_math.h"
#include "final_render.h"
#include "final_audiosystem.h"
#include "final_memory.h"

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
			ButtonState debugReload;
		};
		ButtonState buttons[12];
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
	struct fmemMemoryBlock *memory;
	struct GameState *game;
	struct RenderState *render;
	struct AudioSystem *audio;
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

extern bool GameInit(GameMemory &gameMemory);
extern void GameRelease(GameMemory &gameMemory);
extern void GameInput(GameMemory &gameMemory, const Input &input);
extern void GameUpdate(GameMemory &gameMemory, const Input &input);
extern void GameRender(GameMemory &gameMemory, const float alpha);
extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha);
extern bool IsGameExiting(GameMemory &gameMemory);

#endif // FINAL_GAME_H