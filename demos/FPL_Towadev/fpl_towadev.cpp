/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev
Description:
	A tower defence clone
Requirements:
	- C++ Compiler
	- Final Dynamic OpenGL
Author:
	Torsten Spaete
Changelog:
	## 2018-06-05:
	- Initial creation
Todo:
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_LOG_TO_DEBUGOUT
#include <final_platform_layer.h>

#include <math.h> // fabs

#include <vector>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#include <final_game.h>

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
const Vec2f WorldRadius = V2f(WorldWidth, WorldHeight) * 0.5f;

struct GameState {
	Vec2i viewSize;
	Vec2i viewOffset;
	bool isExiting;
};

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable, const bool isAlphaOnly = false) {
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint internalFormat = isAlphaOnly ? GL_ALPHA8 : GL_RGBA8;
	GLenum format = isAlphaOnly ? GL_ALPHA : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	return(handle);
}

static void GameRelease(GameState &state) {
	fglUnloadOpenGL();
}

static bool GameInit(GameState &state) {
	if(!fglLoadOpenGL(true)) {
		return false;
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
	return(true);
}

extern GameMemory GameCreate() {
	GameMemory result = {};
	result.size = sizeof(GameState);
	result.base = fplMemoryAllocate(result.size);
	if(result.base == nullptr) {
		return {};
	}
	GameState *state = (GameState *)result.base;
	if(!GameInit(*state)) {
		GameDestroy(result);
		state = nullptr;
	}
	return(result);
}

extern void GameDestroy(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	GameRelease(*state);
	state->~GameState();
	fplMemoryFree(state);
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input, bool isActive) {
	if(!isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input, bool isActive) {
	if(!isActive) {
		return;
	}

	GameState *state = (GameState *)gameMemory.base;

	// Compute viewport
	Vec2i viewSize = V2i(input.windowSize.x, (int)(input.windowSize.x / GameAspect));
	if(viewSize.y > input.windowSize.y) {
		viewSize.y = input.windowSize.y;
		viewSize.x = (int)(input.windowSize.y * GameAspect);
	}
	Vec2i viewOffset = V2i((input.windowSize.x - viewSize.x) / 2, (input.windowSize.y - viewSize.y) / 2);
	state->viewSize = viewSize;
	state->viewOffset = viewOffset;
}

extern void GameDraw(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	const float w = WorldRadius.x;
	const float h = WorldRadius.y;

	Vec2i viewSize = state->viewSize;
	Vec2i viewOffset = state->viewOffset;
	glViewport(viewOffset.x, viewOffset.y, viewSize.x, viewSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float scale = 1.0f;
	glOrtho(-w * scale, w*scale, -h * scale, h*scale, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	int result = GameMain("FPL Demo | Towadev");
	return(result);
}