#ifndef GAME_H
#define GAME_H

#include <final_platform_layer.h>

#include "common.h"

#include "map.h"
#include "world.h"
#include "assets.h"
#include "camera.h"
#include "editor.h"

//
// Game State
//
typedef enum GameMode {
	// Only game is running
	GameMode_Game = 0,
	// Editor is active and game is still running
	GameMode_EditorPlay,
	// Editor is active and game is fully paused
	GameMode_EditorPause,
	// Total number of game modes
	GameMode_Count,
} GameMode;

typedef struct GameState {
	World world;

	GameAssets assets;

	Mat4f projection;
	Mat4f view;
	Mat4f viewProjection;
	Camera gameCamera;
	Vec2f mouseWorldPos;

	Editor editor;

	fmemMemoryBlock *memory;
	Camera *activeCamera;

	float framesPerSecond[2];
	float deltaTime;

	GameMode mode;

	bool isExiting;
	bool isDebugRendering;
} GameState;

fpl_extern void PlatformerGameRelease(GameMemory *gameMemory, GameState *state);
fpl_extern bool PlatformerGameInit(GameMemory *gameMemory, RenderState *renderState, GameState *state);
fpl_extern bool PlatformerGameIsExiting(GameMemory *gameMemory, GameState *state);
fpl_extern void PlatformerGameInput(GameMemory *gameMemory, GameState *state, RenderState *renderState, const Input *input);
fpl_extern void PlatformerGameUpdate(GameMemory *gameMemory, GameState *state, const Input *input);
fpl_extern void PlatformerGameRender(GameMemory *gameMemory, GameState *state, RenderState *renderState, const Input *input, const float alpha);

#endif // GAME_H
