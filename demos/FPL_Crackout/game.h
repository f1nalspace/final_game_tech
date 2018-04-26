#pragma once

#include <stdlib.h>

#include <final_platform_layer.h>

#include <Box2D/Box2D.h>

struct Vec2f {
	float x;
	float y;
};
inline Vec2f MakeVec2f(float x, float y) {
	Vec2f result;
	result.x = x;
	result.y = y;
	return(result);
}

struct Vec4f {
	float x;
	float y;
	float z;
	float w;
};
inline Vec4f MakeVec4f(float x, float y, float z, float w) {
	Vec4f result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return(result);
}

struct Vec2i {
	int x;
	int y;
};
inline Vec2i MakeVec2i(int x, int y) {
	Vec2i result;
	result.x = x;
	result.y = y;
	return(result);
}

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
			ButtonState editorToggle;
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

struct GameState;

extern GameState *GameCreate();
extern void GameDestroy(GameState *state);
extern void GameInput(GameState &state, const Input &input, bool isActive);
extern void GameUpdate(GameState &state, const Input &input, bool isActive);
extern void GameDraw(GameState &state);
extern bool IsGameExiting(GameState &state);
