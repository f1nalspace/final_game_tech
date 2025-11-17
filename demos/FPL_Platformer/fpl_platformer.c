/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Platformer

Description:
	Platformer based speculative contacts

Requirements:
	- C++ Compiler
	- Final Framework

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2023 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <final_game.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#define FINAL_GEOMETRY_IMPLEMENTATION
#include <final_geometry.h>

#include <final_utils.h>

#include "constants.h"

#include "map.c"

#include "collision.c"

//
// Tiles
//
#define TileType_PlayerPosition 2

//
// Levels
//
#define TestLevel_Width 11
#define TestLevel_Height 8

static uint32_t gTestLevelTiles[TestLevel_Width * TestLevel_Height] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 2, 0, 0, 0, 1, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static Map gTestLevel = {
	{0},
	{0},
	{0, 0},
	{TileWidth, TileHeight},
	{TileWidth * 0.5f, TileHeight * 0.5f},
	gTestLevelTiles,
	TestLevel_Width,
	TestLevel_Height,
};

//
// Game
//
#define PlayerMaxSpeed 100.0f
#define PlayerWalkSpeed 30.0f
#define PlayerAirSpeed 40.0f
#define PlayerJumpVelocity (200.0f * 1.2f)
#define PlayerGroundFriction 0.2f
#define PlayerAirFriction 0.2f

typedef struct Assets {
	FontAsset consoleFont;
	char dataPath[1024];
} Assets;

typedef struct GroundState {
	fpl_b32 current;
	fpl_b32 last;
} GroundState;

typedef struct Entity {
	Vec4f color;
	Vec2f position;
	Vec2f velocity;
	Vec2f radius;
	GroundState groundState;
	float groundFriction;
	float airFriction;
	bool applyFriction;
	bool applyAirFriction;
	bool jumpRequested;
} Entity;

fpl_inline bool EntityIsGrounded(const Entity *entity) {
	if (entity == fpl_null) {
		return false;
	}
	return entity->groundState.current;
}

fpl_inline bool EntityIsAir(const Entity *entity) {
	if (entity == fpl_null) {
		return false;
	}
	return !entity->groundState.current;
}

static void LoadPlayer(Entity *player, const Map *map) {
	player->radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player->velocity = V2fInit(0.0f, 0.0f);
	player->color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player->position = V2fInit(0.0f, 0.0f);

	Vec2i playerTilePos;
	if (MapFindPositionByTile(map, TileType_PlayerPosition, &playerTilePos)) {
		Vec2f tilePos = MapTileCoordsToWorld(map, playerTilePos);
		Vec2f tileBottomCenter = V2fAdd(tilePos, V2fInit(TileWidth * 0.5f, 0));

		// Move the player above the tile, but to the center
		player->position = V2fAdd(tileBottomCenter, V2fInit(0, player->radius.h));

		// Move the player above the tile, but to the right
		player->position = V2fAdd(tileBottomCenter, V2fInit(TileSize.w * 0.5f - player->radius.w, player->radius.h));
	}

	player->applyFriction = true;
	player->groundFriction = PlayerGroundFriction;

	player->applyAirFriction = true;
	player->airFriction = PlayerAirFriction;

	player->jumpRequested = false;
}

static void InputPlayer(Entity *player, const Input *input) {
	const Controller *controller = (input->defaultControllerIndex == -1) ? &input->controllers[0] : &input->controllers[input->defaultControllerIndex];

	// Horizontal Movement
	float moveSpeed = EntityIsGrounded(player) ? PlayerWalkSpeed : PlayerAirSpeed;
	if (IsDown(controller->moveLeft)) {
		player->velocity.x -= moveSpeed;
	} else if (IsDown(controller->moveRight)) {
		player->velocity.x += moveSpeed;
	}

	// Jump can always be requested, regardless if in air or not
	if (IsDown(controller->actionDown)) {
		if (!player->jumpRequested) {
			player->jumpRequested = true;
		}
	} else {
		player->jumpRequested = false;
	}

	// Handle requested jump only when grounded
	if (EntityIsGrounded(player) && player->jumpRequested) {
		player->velocity.y = PlayerJumpVelocity;
		player->jumpRequested = false;
	}
}

static void UpdatePlayer(Entity *player, const Map *map, const float dt) {
#if 0
	// Gravity
	player->velocity += Gravity;
#endif

	// Air friction
	if (player->applyAirFriction && EntityIsAir(player) && F32Abs(player->velocity.x) > 0) {
		player->velocity.x *= (1.0f - player->airFriction);
	}

	// Clamp speed
	player->velocity.x = F32Clamp(player->velocity.x, -PlayerMaxSpeed, PlayerMaxSpeed);

	// Grounding
	player->groundState.last = player->groundState.current;
	player->groundState.current = false;

	// Integrate
	player->position = V2fAddMultScalar(player->position, player->velocity, dt);
}

typedef struct World {
	fmemMemoryBlock memory;
	Map map;
	Contact contacts[MaxContactCount];
	Entity player;
	uint32_t numContacts;
} World;

// One time initialization of the map
static void InitMap(fmemMemoryBlock *memory, Map *map) {
	fplClearStruct(map);
	map->tileSize = TileSize;
	map->tileRadius = TileRadius;
	fmemPushBlock(memory, &map->persistentMemory, fplMegaBytes(8), fmemPushFlags_Clear);
	fmemPushBlock(memory, &map->temporaryMemory, fplMegaBytes(8), fmemPushFlags_Clear);
}

static void LoadMap(Map *map, const Map *source) {
	map->persistentMemory.used = 0;
	map->temporaryMemory.used = 0;

	size_t requiredSize = source->width * source->height * sizeof(uint32_t);
	fplAssert(requiredSize <= map->persistentMemory.size);

	map->solidTiles = (uint32_t *)fmemPush(&map->persistentMemory, requiredSize, fmemPushFlags_Clear);
	map->width = source->width;
	map->height = source->height;
	map->origin = source->origin;
	for (uint32_t tileIndex = 0; tileIndex < source->width * source->height; ++tileIndex) {
		map->solidTiles[tileIndex] = source->solidTiles[tileIndex];
	}
}

// One time initialization of the world
static void InitWorld(fmemMemoryBlock *memory, World *world) {
	fplClearStruct(world);

	fmemPushBlock(memory, &world->memory, fplMegaBytes(64), fmemPushFlags_Clear);

	InitMap(&world->memory, &world->map);
}

// Load entire world (can be called anytime)
static void LoadWorld(World *world, const Map *level) {
	LoadMap(&world->map, level);
	LoadPlayer(&world->player, &world->map);
}

typedef struct Editor {
	uint32_t drawTile;
	bool isDrawing;
} Editor;

typedef struct GameState {
	Assets assets;
	World world;
	Editor editor;

	Camera2D camera;
	Mat4f projection;
	Mat4f view;
	Mat4f viewProjection;
	Viewport viewport;
	Vec2f mouseWorldPos;

	Entity *dragEntity;
	Vec2f dragStart;
	bool isDragging;

	float deltaTime;
	float framesPerSecond[2];

	bool isExiting;
	bool isDebugRendering;
} GameState;

static void FreeAssets(Assets *assets) {
	ReleaseFontAsset(&assets->consoleFont);
}

static void LoadAssets(RenderState *renderState, Assets *assets) {
	// Fonts
	char fontDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(fontDataPath, fplArrayCount(fontDataPath), 2, assets->dataPath, "fonts");
	FontAsset *hudFont = &assets->consoleFont;
	if (LoadFontFromFile(fontDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont->desc)) {
		PushTexture(renderState, &hudFont->texture, hudFont->desc.atlasAlphaBitmap, hudFont->desc.atlasWidth, hudFont->desc.atlasHeight, 1, TextureFilterType_Linear, TextureWrapMode_ClampToEdge, false, false);
	}
}

static void InitGame(fmemMemoryBlock *memory, GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// Input
	state->isDebugRendering = true;

	// World
	InitWorld(memory, &state->world);
}

static void LoadGame(GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// World
	LoadWorld(&state->world, &gTestLevel);
}

extern bool GameInit(GameMemory *gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory->memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory->game = state;

	RenderState *renderState = gameMemory->render;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	LoadAssets(renderState, &state->assets);

	InitGame(gameMemory->memory, state);

	LoadGame(state);

	return(true);
}

extern void GameRelease(GameMemory *gameMemory) {
	GameState *state = gameMemory->game;
	if (state != fpl_null) {
		FreeAssets(&state->assets);
	}
}

extern bool IsGameExiting(GameMemory *gameMemory) {
	GameState *state = gameMemory->game;
	assert(state != fpl_null);
	return state->isExiting;
}

static void MapPaintTile(Map *map, const Vec2i tilePos, const uint32_t newTile) {
	Vec2i newOrigin = map->origin;
	Vec2i newSizeAppend = V2iInit(0, 0);
	Vec2i newTilePos = tilePos;
	if (tilePos.x < 0) {
		int xcount = abs(tilePos.x);
		fplAssert(xcount > 0);
		newSizeAppend.x += xcount;
		newOrigin.x -= xcount;
		newTilePos.x += xcount;
	} else if (tilePos.x > ((int)map->width - 1)) {
		int xcount = tilePos.x - ((int)map->width - 1);
		fplAssert(xcount > 0);
		newSizeAppend.x += xcount;
		newTilePos.x = tilePos.x;
	}
	if (tilePos.y < 0) {
		int ycount = abs(tilePos.y);
		fplAssert(ycount > 0);
		newSizeAppend.y += ycount;
		newOrigin.y -= ycount;
		newTilePos.y += ycount;
	} else if (tilePos.y > ((int)map->height - 1)) {
		int ycount = tilePos.y - ((int)map->height - 1);
		fplAssert(ycount > 0);
		newSizeAppend.y += ycount;
		newTilePos.y = tilePos.y;
	}

	if (newSizeAppend.x > 0 || newSizeAppend.y > 0) {
		Vec2i oldMapSize = V2iInit(map->width, map->height);
		Vec2i newMapSize = V2iInit(map->width + newSizeAppend.x, map->height + newSizeAppend.y);

		map->temporaryMemory.used = 0;

		fmemMemoryBlock tempBlock;
		fmemBeginTemporary(&map->temporaryMemory, &tempBlock);

		size_t requiredOldSize = oldMapSize.w * oldMapSize.h * sizeof(uint32_t);
		fplAssert(requiredOldSize <= tempBlock.size);

		uint32_t *oldTiles = (uint32_t *)fmemPush(&tempBlock, requiredOldSize, fmemPushFlags_None);
		fplMemoryCopy(map->solidTiles, requiredOldSize, oldTiles);

		map->persistentMemory.used = 0;
		size_t requiredNewSize = newMapSize.w * newMapSize.h * sizeof(uint32_t);
		fplAssert(requiredNewSize <= map->persistentMemory.size);

		int offsetX = 0;
		int offsetY = 0;

		if (tilePos.x >= 0 && newSizeAppend.x > 0) {
			offsetX -= abs(newSizeAppend.x);
		}

		if (tilePos.y >= 0 && newSizeAppend.x > 0) {
			offsetY += abs(newSizeAppend.y);
		} else if (tilePos.y < 0) {
			offsetY -= abs(tilePos.y);
		}

		map->width = newMapSize.w;
		map->height = newMapSize.h;
		map->solidTiles = (uint32_t *)fmemPush(&map->persistentMemory, requiredNewSize, fmemPushFlags_Clear);
		for (int y = 0; y < oldMapSize.h; ++y) {
			for (int x = 0; x < oldMapSize.w; ++x) {
				int ox = newSizeAppend.x + x + offsetX;
				int oy = newSizeAppend.y + y + offsetY;
				fplAssert(ox >= 0 && ox < newMapSize.w);
				fplAssert(oy >= 0 && oy < newMapSize.h);
				map->solidTiles[oy * newMapSize.w + ox] = oldTiles[y * oldMapSize.w + x];
			}
		}

		fmemEndTemporary(&map->temporaryMemory);
	}

	int invY = map->height - 1 - newTilePos.y;
	int curTile = map->solidTiles[invY * map->width + newTilePos.x];
	map->solidTiles[invY * map->width + newTilePos.x] = newTile;

	map->origin = newOrigin;
}

static void EditorInput(GameState *state, const Input *input) {
	Map *map = &state->world.map;

	Editor *editor = &state->editor;

	Vec2f originWorld = MapTileCoordsToWorld(map, map->origin);

	Vec2i mouseTilePos = MapWorldCoordsToTile(map, V2fSub(state->mouseWorldPos, originWorld));

	if (IsDown(input->mouse.left)) {
		if (!editor->isDrawing) {
			editor->isDrawing = true;

			if (MapIsTileInside(map, mouseTilePos)) {
				uint32_t tile = MapGetTile(map, mouseTilePos);
				editor->drawTile = tile == 0 ? 1 : 0;
			} else {
				editor->drawTile = 1;
			}
		}
		if (editor->isDrawing) {
			fplAssert(editor->drawTile != UINT32_MAX);
			MapPaintTile(map, mouseTilePos, editor->drawTile);
		}
	} else {
		if (editor->isDrawing) {
			editor->drawTile = UINT32_MAX;
			editor->isDrawing = false;
		}
	}
}

extern void GameInput(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	RenderState *renderState = gameMemory->render;
	assert(renderState != fpl_null);

	// Debug input
	const Controller *keyboardController = &input->controllers[0];
	if (WasPressed(keyboardController->debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input->windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	state->projection = M4fOrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	state->view = M4fTranslationV2(state->camera.offset);
	state->viewProjection = M4fMult(state->projection, state->view);

	// Mouse
	int mouseCenterX = (input->mouse.pos.x) - input->windowSize.w / 2;
	int mouseCenterY = (input->windowSize.h - 1 - input->mouse.pos.y) - input->windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	// Editor input
	EditorInput(state, input);

	// Player input
	InputPlayer(&state->world.player, input);
}

extern void GameUpdate(GameMemory *gameMemory, const Input *input) {
	if (!input->isActive) {
		return;
	}

	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	const float dt = input->fixedDeltaTime;

	World *world = &state->world;
	Map *map = &world->map;
	Entity *player = &world->player;

	// Player
	UpdatePlayer(player, map, dt);

	// Camera
	state->camera.offset = V2fNegate(player->position);
	state->camera.scale = 1;

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input->framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = F32Avg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

static void PushNormal(RenderState *renderState, const Vec2f position, const Vec2f normal, const float length, const float chairSize) {
	// const float length = 20.0f, const float chairSize = 8.0f

	Vec2f tangent = V2fCrossR(normal, 1.0f);

	Vec2f a = position;
	Vec2f b = V2fAddMultScalar(a, normal, length);
	PushLine(renderState, a, b, V4fInit(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, normal, chairSize);
	b = V2fAddMultScalar(a, tangent, chairSize);
	PushLine(renderState, a, b, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	a = V2fAddMultScalar(position, tangent, chairSize);
	b = V2fAddMultScalar(a, normal, chairSize);
	PushLine(renderState, a, b, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
}

static void PushOrigin(RenderState *renderState, const Vec2f origin) {
	PushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, 2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(0.0f, -2.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(-2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, V2fAdd(origin, V2fInit(2.0f, 0.0f)), 1.0f, V4fInit(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin, 1.0f, V4fInit(0.25f, 0.25f, 0.25f, 1), true, 1.0f);
}

extern void GameRender(GameMemory *gameMemory, const float alpha) {
	GameState *state = gameMemory->game;
	assert(state != fpl_null);

	World *world = &state->world;

	const Map *map = &world->map;

	const Entity *player = &world->player;

	RenderState *renderState = gameMemory->render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	Vec2i mapSize = V2iInit(map->width, map->height);
	Vec2f mapArea = V2fHadamard(TileSize, V2fInit((float)mapSize.x, (float)mapSize.y));
	Vec2f mapOrigin = MapTileCoordsToWorld(map, map->origin);
	Vec4f mapSolidColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4f playerTileColor = V4fInit(0.3f, 0.1f, 0.7f, 1.0f);

	Vec2f gridSize = mapArea;
	Vec2f gridOrigin = mapOrigin;
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	int gridTileCountX = map->width;
	int gridTileCountY = map->height;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags_Color | ClearFlags_Depth);
	SetMatrix(renderState, &state->projection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	SetMatrix(renderState, &state->viewProjection);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);

	// Tile grid
	for (int i = 0; i <= gridTileCountX; ++i) {
		float xoffset = i * TileWidth;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(xoffset, 0));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(xoffset, gridSize.y));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}
	for (int i = 0; i <= gridTileCountY; ++i) {
		float yoffset = i * TileHeight;
		Vec2f a = V2fAdd(gridOrigin, V2fInit(0, yoffset));
		Vec2f b = V2fAdd(gridOrigin, V2fInit(gridSize.x, yoffset));
		PushLine(renderState, a, b, gridColor, 1.0f);
	}

	// Map
	for (int y = 0; y < mapSize.h; ++y) {
		for (int x = 0; x < mapSize.w; ++x) {
			uint32_t tile = MapGetTile(map, V2iInit(x, y));
			if (MapIsObstacle(map, tile)) {
				Vec2f tilePos = V2fAdd(gridOrigin, V2fInit(x * TileWidth, y * TileHeight));
				PushRectangle(renderState, tilePos, TileSize, mapSolidColor, true, 1.0f);
			}
		}
	}

	// Player
	PushRectangleCenter(renderState, player->position, player->radius, player->color, false, 2.0f);
	PushOrigin(renderState, player->position);

	world->numContacts = 0;

	// Render collision tile bounds
	bool renderPlayerTileBounds = true;
	if (renderPlayerTileBounds) {
		uint32_t entityId = StartEntityID;

		// Predict position for next frame
		Vec2f predictedPos = V2fAddMultScalar(player->position, player->velocity, dt);

		// Get min/max
		Vec2f min = V2fMin(predictedPos, player->position);
		Vec2f max = V2fMax(predictedPos, player->position);

		// Create AABB and expand it
		Vec2f expandedRadius = V2fAdd(player->radius, V2fInitScalar(AABBExpand));
		min = V2fSub(min, expandedRadius);
		max = V2fAdd(max, expandedRadius);
		AABB2f entityAABB = AABB2fInit(min, max);

		// Tile tiles area from AABB
		Vec2i minTile = MapWorldCoordsToTile(map, entityAABB.min);
		Vec2i maxTile = MapWorldCoordsToTile(map, V2fAdd(entityAABB.max, V2fInit(0.5f, 0.5f)));

		for (int x = minTile.x; x <= maxTile.x; ++x) {
			for (int y = minTile.y; y <= maxTile.y; ++y) {
				if (x < 0 || x >= (int)map->width || y < 0 || y >= (int)map->height) {
					continue;
				}

				Vec2i tilePos = V2iInit(x, y);
				uint32_t tile = MapGetTile(map, tilePos);

				Vec2f worldPos = MapTileCoordsToWorld(map, tilePos);
				Vec2f tileCenter = V2fAdd(worldPos, TileRadius);
				PushRectangleCenter(renderState, tileCenter, TileRadius, playerTileColor, false, 1.0f);

				if (MapIsObstacle(map, tile)) {
					uint32_t tileId = StartMapTileID + (1 + (y * map->width + x));
					AABB2f tileAABB = MapCreateTileAABB(map, tilePos);
					AABB2f entityAABB = AABB2fInitFromCenter(player->position, player->radius);
					fplAssert(world->numContacts < fplArrayCount(world->contacts));
					Contact *contacts = world->contacts + world->numContacts;
					uint32_t contactCount = CreateContactsAABBvsAABB(map, entityId, &entityAABB, tileId, &tileAABB, tilePos, true, contacts);
					if (contactCount > 0) {
						const Contact *contact = contacts + 0;
						PushCircle(renderState, contact->posA, 2.0f, 16, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
						PushCircle(renderState, contact->posB, 2.0f, 16, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), true, 0.0f);
					}
					world->numContacts += contactCount;
				}
			}
		}
	}

	// Render contacts
	bool renderContacts = true;
	if (renderContacts) {
		for (uint32_t i = 0; i < world->numContacts; ++i) {
			const Contact *contact = world->contacts + i;
			//PushCircle(renderState, contact->pos, 2.0f, 16, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);
		}
	}

	// Mouse cursor
	PushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(2, 2), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);

	// Mouse tile
	Vec2i mouseTilePos = MapWorldCoordsToTile(map, V2fSub(state->mouseWorldPos, mapOrigin));
	Vec2f mouseWorldPos = MapTileCoordsToWorld(map, mouseTilePos);
	PushRectangle(renderState, V2fAdd(gridOrigin, mouseWorldPos), TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);

	const FontAsset *font = &state->assets.consoleFont;
	float fontHeight = 6.0f;

	char buffer[100];
	fplStringFormat(buffer, fplArrayCount(buffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
	PushText(renderState, buffer, fplGetStringLength(buffer), &font->desc, font->texture, mouseWorldPos, fontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));

	if (state->isDebugRendering) {
		SetMatrix(renderState, &state->projection);

		const FontAsset *font = &state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec4f blackColor = V4fInit(0, 0, 0, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 8.0f;

		fpl_localvar char sizeCharsBuffer[2][32 + 1] = fplZeroInit;
		FormatSize(gameMemory->memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory->memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState->lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState->memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w - 1, blockPos.y - 1), fontHeight, 0.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplStringFormat(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f - 1, blockPos.y - 1), fontHeight, -1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font->desc, font->texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);
	}
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = fplZeroInit;
	config.title = "FPL Demo | Platformer";
	config.disableInactiveDetection = true;
	config.disableVerticalSync = false;
	int result = GameMain(&config);
	return(result);
}