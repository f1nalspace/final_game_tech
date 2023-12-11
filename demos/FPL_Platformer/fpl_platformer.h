#ifndef FPL_GAMETEMPLATE_H
#define FPL_GAMETEMPLATE_H

#include <final_math.h>
#include <final_geometry.h>

#include <final_fontloader.h>

#include <final_render.h>

#include <final_utils.h>

#include <final_assets.h>

struct Assets {
	FontAsset consoleFont;
	char dataPath[1024];
};

struct Entity {
	Vec2f position;
	Vec2f velocity;
	Vec2f radius;
	Vec4f color;
	float moveSpeed;
	float moveDrag;
};

struct World {
	Entity player;
};

struct GameState {
	Assets assets;
	World world;

	Camera2D camera;
	Mat4f viewProjection;
	Viewport viewport;
	Vec2f mouseWorldPos;

	float deltaTime;
	float framesPerSecond[2];

	bool isExiting;
	bool isDebugRendering;
};

#endif // FPL_GAMETEMPLATE_H