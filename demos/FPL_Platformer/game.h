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

fpl_extern void GameStateReleaseInternal(GameState *state);
fpl_extern bool GameStateInitInternal(fmemMemoryBlock *memory, GameState *state);
fpl_extern bool GameStateLoadInternal(RenderState *renderState, GameState *state);
fpl_extern bool GameStartInternal(GameState *state);

fpl_extern bool GameInit(GameMemory *gameMemory);
fpl_extern void GameRelease(GameMemory *gameMemory);
fpl_extern bool IsGameExiting(GameMemory *gameMemory);
fpl_extern void GameInput(GameMemory *gameMemory, const Input *input);
fpl_extern void GameUpdate(GameMemory *gameMemory, const Input *input);
fpl_extern void GameRender(GameMemory *gameMemory, const Input *input, const float alpha);

#endif // GAME_H
