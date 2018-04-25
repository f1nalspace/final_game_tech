#pragma once

#include <final_platform_layer.h>

// The only reason why this demo require C++ -.-
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <Box2D/Box2D.h>

struct ButtonState {
	bool isDown = false;
	glm::int32 halfTransitionCount = 0;

	inline bool WasPressed() const {
		bool result =
			((halfTransitionCount > 1) ||
			((halfTransitionCount == 1) && (isDown)));
		return(result);
	}
	ButtonState() {}
};

struct Controller {
	bool isConnected = false;
	bool isAnalog = false;
	glm::vec2 analogMovement = glm::vec2(0, 0);
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
		ButtonState buttons[9] = {};
	};
	Controller() {}
};

struct Mouse {
	glm::ivec2 pos = glm::vec2(0, 0);
	glm::f32 wheelDelta = 0;
	union {
		struct {
			ButtonState left;
			ButtonState middle;
			ButtonState right;
		};
		ButtonState buttons[3] = {};
	};
	Mouse() {}
};

struct Input {
	glm::f32 deltaTime = 0.0f;
	union {
		struct {
			Controller keyboard;
			Controller gamepad[4];
		};
		Controller controllers[5] = {};
	};
	Mouse mouse = {};
	glm::ivec2 windowSize = glm::vec2(0, 0);
	Input() {}
};

struct GameState;

extern GameState *GameCreate();
extern void GameDestroy(GameState *state);
extern void GameUpdate(GameState &state, const Input &input);
extern void GameDraw(GameState &state);