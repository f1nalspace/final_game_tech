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

struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
};

inline UVRect UVRectFromTile(const Vec2i &imageSize, const Vec2i &tileSize, const int border, const Vec2i &pos) {
	Vec2f texel = MakeVec2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	int imgX = border + pos.x * tileSize.x + border * pos.x;
	int imgY = border + pos.y * tileSize.y + border * pos.y;
	UVRect result;
	result.uMin = imgX * texel.x;
	result.vMin = imgY * texel.y;
	result.uMax = result.uMin + tileSize.x * texel.x;
	result.vMax = result.vMin + tileSize.y * texel.y;
	return(result);
}

inline UVRect UVRectFromPos(const Vec2i &imageSize, const Vec2i &partSize, const Vec2i &pos) {
	Vec2f texel = MakeVec2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
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
