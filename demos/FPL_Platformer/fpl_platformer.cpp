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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#include <final_game.h>

#include <final_math.h>

#include "fpl_platformer.h"

//
// Constants
//
constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 640.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
constexpr float WorldRadiusW = WorldWidth * 0.5f;
constexpr float WorldRadiusH = WorldHeight * 0.5f;

constexpr float TileWidth = 32.0f;
constexpr float TileHeight = 32.0f;
static Vec2f TileSize = V2fInit(TileWidth, TileHeight);

static Vec2f Gravity = V2fInit(0, -10.0f);

static Vec2f AABBExpand = V2fInit(6.0f, 6.0f);

//
// Utils
//
static void FormatSize(const size_t value, const size_t maxCount, char *buffer) {
	char *p = buffer;
	if (value < 0) {
		*p++;
	}

	size_t tmp = value;
	do {
		*p++;
		tmp = tmp / 10;
	} while (tmp);

	// Count thousands
	size_t thousandDotCount = 0;
	tmp = value;
	while (tmp >= 1000) {
		*p++;
		++thousandDotCount;
		tmp = tmp / 1000;
	}

	size_t charCount = p - buffer;

	fplAssert(charCount + 1 <= maxCount);

	*p = 0;
	const char *digits = "0123456789";
	size_t v = value;
	size_t c = 0;
	size_t t = thousandDotCount;
	do {
		if (t > 0) {
			if (c == 3) {
				c = 0;
				--t;
				*--p = '.';
			}
		}
		*--p = digits[v % 10];
		v /= 10;
		c++;
	} while (v != 0);
}

//
// Math & Physics
//
struct AABB {
	Vec2f center;
	Vec2f halfExtents;

	inline Vec2f GetBottomLeft() {
		Vec2f result = center + V2fInit(-halfExtents.x, -halfExtents.y);
		return result;
	}

	inline Vec2f GetBottomRight() {
		Vec2f result = center + V2fInit(halfExtents.x, -halfExtents.y);
		return result;
	}

	inline Vec2f GetTopLeft() {
		Vec2f result = center + V2fInit(-halfExtents.x, halfExtents.y);
		return result;
	}

	inline Vec2f GetTopRight() {
		Vec2f result = center + V2fInit(halfExtents.x, halfExtents.y);
		return result;
	}

	inline Vec2f GetMin() {
		return GetBottomLeft();
	}

	inline Vec2f GetMax() {
		return GetTopRight();
	}

	inline void Update(const Vec2f &center, const Vec2f &halfExtents) {
		this->center = center;
		this->halfExtents = halfExtents;
	}

	static bool Overlap(const AABB &a, const AABB &b) {
		Vec2f centerDelta = V2fAbs(b.center - a.center);
		Vec2f halfExtentsSum = a.halfExtents + b.halfExtents;
		centerDelta -= halfExtentsSum;
		bool result = centerDelta.x < 0 && centerDelta.y < 0;
		return result;
	}
};

struct PointDistance {
	Vec2f pos;
	float dist;
};

struct Contact {
	Vec2f normal;
	Vec2f point;
	float impulse;
	float distance;

public:
	inline void Initialize(const Vec2f &normal, const float distance, const Vec2f &point) {
		this->normal = normal;
		this->point = point;
		this->impulse = 0;
		this->distance = distance;
	}
};

struct Circle {
	Vec2f pos;
	float radius;

	inline bool Contains(const Vec2f &p) {
		bool result = V2fLengthSquared(p - pos) < radius * radius;
		return result;
	}

	inline float DistanceToPoint(const Vec2f &p) {
		float len = V2fLength(p - pos);
		float result = len - radius;
		return result;
	}

	inline Vec2f ClosestPointOnEdge(const Vec2f &p, const float bias) {
		Vec2f unit = V2fNormalize(p - pos);
		Vec2f result = V2fMultScalar(unit, radius - bias) + pos;
		return result;
	}

	inline PointDistance ClosestPointAndDistOnEdge(const Vec2f &p) {
		Vec2f d = p - pos;
		float distCentre = V2fLength(d);
		float penetration = distCentre - radius;
		if (distCentre == 0) {
			return { pos + V2fInit(radius, 0), -radius }; // Default
		}

		// Generate point on edge
		Vec2f poe = V2fAddMultScalar(pos, d, radius / distCentre);
		return { poe, penetration };
	}
};

//
// Map
//
struct Map {
	uint32_t width;
	uint32_t height;
	const uint32_t *solidTiles;

public:
	inline Vec2i WorldCoordsToTile(const Vec2f worldPos) const {
		// Adjustment for negative coordinates
		float rx = 0.0f;
		float ry = 0.0f;
		if (worldPos.x < 0)
			rx = -1.0f;
		if (worldPos.y < 0)
			ry = -1.0f;
		int x = (int)(worldPos.x / TileWidth + rx);
		int y = (int)(worldPos.y / TileHeight + ry);
		return V2iInit(x, y);
	}
	inline Vec2f TileCoordsToWorld(const Vec2i tilePos) const  {
		float x = (float)tilePos.x * TileWidth;
		float y = (float)tilePos.y * TileHeight;
		return V2fInit(x, y);
	}

	inline uint32_t GetTile(const int32_t x, const int32_t y) const {
		if (width == 0 || height == 0 || solidTiles == nullptr) {
			return UINT32_MAX;
		}
		int invY = height - 1 - y;
		if (x < 0 || invY < 0 || x > ((int)width - 1) || invY > ((int)height - 1)) {
			return UINT32_MAX;
		}
		uint32_t result = solidTiles[invY * width + x];
		return result;
	}

	inline bool IsObstacle(const uint32_t tile) const {
		// @TODO(final): Obstacle tile mapping!
		bool result = tile == 1;
		return result;
	}

	inline bool FindPositionByTile(const uint32_t type, Vec2i *outTilePos) const {
		if (width == 0 || height == 0 || solidTiles == nullptr) {
			return false;
		}
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				uint32_t tile = GetTile(x, y);
				if (tile == type)
				{
					*outTilePos = V2iInit(x, y);
					return true;
				}
			}
		}
		return false;
	}
};

//
// Collision Detection
//
namespace Collision {
	static bool IsInternalCollision(const Map &map, const Vec2i &tilePos, const Vec2f& normal) {
		int nextTileX = tilePos.x + (int)normal.x;
		int nextTileY = tilePos.y + (int)normal.y;

		uint32_t currentTile = map.GetTile(tilePos.x, tilePos.y);
		uint32_t nextTile = map.GetTile(nextTileX, nextTileY);

		bool internalEdge = map.IsObstacle(nextTile);

		return internalEdge;
	}

	static Vec2f RightAxis = V2fInit(1, 0);
	static Vec2f UpAxis = V2fInit(0, 1);

	static Vec2f AABBProjection(Vec2f center, Vec2f extents, Vec2f normal) {
		float r = Abs(V2fDot(normal, RightAxis)) * extents.w + Abs(V2fDot(normal, UpAxis)) * extents.h;
		return V2fInit(-r, +r);
	}

	static bool AabbVsAabb(const AABB &a, const AABB &b, Contact &outContact, const Vec2i &tilePos, const Map &map, const bool checkInternal = true) {
		Vec2f delta = b.center - a.center;

		Vec2f axes[] = {
			V2fInit(1, 0),
			V2fInit(0, 1),
		};

		Vec2f collisionNormal = V2fZero();
		float collisionDistance = 0;
		int index = -1;
		
		int axesCount = fplArrayCount(axes);
		for (int i = 0; i < axesCount; ++i) {
			Vec2f n = axes[i];

			// Project A and B on normal
			Vec2f projA = AABBProjection(a.center, a.halfExtents, n);
			Vec2f projB = AABBProjection(b.center, b.halfExtents, n);

			// Add relative offset to B´s projection
			float relativeProjection = V2fDot(n, delta);
			projB.x += relativeProjection;
			projB.y += relativeProjection;

			// Calculate overlap and get smallest (greatest negative) projection
			float d0 = projA.x - projB.y;
			float d1 = projB.x - projA.y;
			float overlap = d0 > d1 ? d0 : d1;

			// Store smallest (greatest negative) collision distance and normal when needed
			if (index == -1 || overlap > collisionDistance) {
				index = i;
				collisionDistance = overlap;
				collisionNormal = n;
			}
		}

		// Make sure the collision normal faces in the right direction
		if (V2fDot(collisionNormal, delta) < 0) {
			collisionNormal = -collisionNormal;
		}

		// Normal needs to be to flipped always!
		collisionNormal = -collisionNormal;

		// Fill out contact
		outContact.Initialize(collisionNormal, collisionDistance, a.center);

		// Check for internal collisions (internal edges)
		if (checkInternal) {
			return !IsInternalCollision(map, tilePos, collisionNormal);
		}

		// Always return true, because with speculative contacts we want "speculative" contacts that may collide or not
		return true;
	}
}

//
// Levels
//
namespace TestLevel {
	constexpr uint32_t Width = 11;
	constexpr uint32_t Height = 8;

	const uint32_t p = (uint32_t)'p';

	static uint32_t Tiles[Width * Height] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, p, 0, 0, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	};

	static Map Level = { Width, Height, Tiles };
};



//
// Game
//
struct Assets {
	FontAsset consoleFont;
	char dataPath[1024];
};

struct Entity {
	Contact contact;
	Vec4f color;
	Vec2f position;
	Vec2f positionCorrection;
	Vec2f velocity;
	Vec2f radius;
	float groundFriction;
	float airFriction;
	bool onGround[2];
	bool applyFriction;
	bool applyAirFriction;
	bool jumpRequested;
};

constexpr float MaxSpeed = 100.0;
constexpr float PlayerWalkSpeed = 40.0;
constexpr float PlayerAirSpeed = PlayerWalkSpeed * 0.75f;
constexpr float PlayerJumpVelocity = 200.0f * 1.2f;
constexpr float PlayerGroundFriction = 0.3f;
constexpr float PlayerAirFriction = 0.75f;

static void InitPlayer(Entity &player, const Map &map) {
	player.radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player.velocity = V2fInit(0.0f, 0.0f);
	player.color = V4fInit(0.05f, 0.1f, 0.95f, 1);

	player.position = V2fInit(0.0f, 0.0f);

	Vec2f worldHalfExtents = V2fHadamard(V2fInit((float)map.width, (float)map.height), TileSize);

	Vec2i playerTilePos;
	if (map.FindPositionByTile(TestLevel::p, &playerTilePos)) {
		Vec2f tilePos = map.TileCoordsToWorld(playerTilePos);
		Vec2f tileBottomCenter = tilePos + V2f(TileWidth * 0.5f, 0);
		player.position = tileBottomCenter + V2fInit(0, player.radius.y);
	}

	player.applyFriction = true;
	player.groundFriction = PlayerGroundFriction;

	player.applyAirFriction = true;
	player.airFriction = PlayerAirFriction;

	player.jumpRequested = false;
}

static void InputPlayer(Entity &player, const Input &input) {
	const Controller &controller = (input.defaultControllerIndex == -1) ? input.controllers[0] : input.controllers[input.defaultControllerIndex];

	// Horizontal Movement
	float moveSpeed = player.onGround[0] ? PlayerWalkSpeed : PlayerAirSpeed;
	if (IsDown(controller.moveLeft)) {
		player.velocity.x -= moveSpeed;
	} else if (IsDown(controller.moveRight)) {
		player.velocity.x += moveSpeed;
	}

	// Jump can always be requested, regardless if in air or not
	if (IsDown(controller.actionDown)) {
		if (!player.jumpRequested) {
			player.jumpRequested = true;
		}
	} else {
		player.jumpRequested = false;
	}

	// Handle requested jump only when grounded
	if (player.onGround[0] && player.jumpRequested) {
		player.velocity.y = PlayerJumpVelocity;
		player.jumpRequested = false;
	}
}

static void CollisionResponse(Entity &player, const Vec2f &normal, const float dist, const float dt) {
	// Get the separation and penetration separately, this is to stop pentration from causing the objects to ping apart
	float separation = Max(dist, 0);
	float penetration = Min(dist, 0);

	// Compute relative normal velocity require to be object to an exact stop at the surface
	float nv = V2fDot(player.velocity, normal) + separation / dt;

	// Accumulate the penetration correction, this is applied in the update function and ensures we don't add any energy to the system
	player.positionCorrection -= V2fMultScalar(normal, penetration / dt);

	if (nv < 0) {
		// Remove normal velocity
		player.velocity -= V2fMultScalar(normal, nv);

		// Is grounded?
		if (normal.y > 0.0f) {
			player.onGround[0] = true;

			// Friction
			if (player.applyFriction) {
				// Form perpendicular vector from the normal
				Vec2f tangent = V2fCrossR(normal, -1.0f);

				// Compute the tangential velocity, scale by friction
				float tv = V2fDot(player.velocity, tangent) * player.groundFriction;

				// Subtract that from the main velocity
				player.velocity -= V2fMultScalar(tangent, tv);
			}
		}
	}
}

static void DetectCollision(Entity &player, const Map &map, const float dt) {
	// Predict position
	Vec2f predictedPos = V2fAddMultScalar(player.position, player.velocity, dt);

	// Find min/max
	Vec2f min = V2fMin(player.position, predictedPos);
	Vec2f max = V2fMax(player.position, predictedPos);

	// Extent by radius
	min -= player.radius;
	max += player.radius;

	// Expand a bit more to really capture all tiles
	min -= AABBExpand;
	max += AABBExpand;

	// Get world half extents
	Vec2f worldHalfExtents = V2fInit(map.width * TileWidth, map.height * TileHeight) * 0.5f;

	// Get tile range min/max
	Vec2i tileMin = map.WorldCoordsToTile(min);
	Vec2i tileMax = map.WorldCoordsToTile(max + V2fInitScalar(0.5f));

	// Player bounds
	AABB playerAABB = { player.position, player.radius };

	// Collide against all solid tiles
	for (int y = tileMin.y; y <= tileMax.y; ++y) {
		for (int x = tileMin.x; x <= tileMax.x; ++x) {
			Vec2i tilePos = V2iInit(x, y);
			Vec2f tileWorld = map.TileCoordsToWorld(tilePos);
			Vec2f aabbCenter = tileWorld + TileSize * 0.5f;
			AABB tileAABB = { aabbCenter, TileSize * 0.5f };
			uint32_t tile = map.GetTile(x, y);
			if (map.IsObstacle(tile)) {
				bool collided = Collision::AabbVsAabb(playerAABB, tileAABB, player.contact, tilePos, map);
				if (collided) {
					CollisionResponse(player, player.contact.normal, player.contact.distance, dt);
				}
			}
		}
	}
}

static void UpdatePlayer(Entity &player, const Map &map, const float dt) {
	// Gravity
	player.velocity += Gravity;

	// Air friction
	if (player.applyAirFriction && !player.onGround[0] && Abs(player.velocity.x) > 0) {
		player.velocity.x *= player.airFriction;
	}

	// Clamp speed
	player.velocity.x = ScalarClamp(player.velocity.x, -MaxSpeed, MaxSpeed);

	// Grounding
	player.onGround[1] = player.onGround[0];
	player.onGround[0] = false;

	// Collision detection
	DetectCollision(player, map, dt);

	// Integrate
	player.position = V2fAddMultScalar(player.position, player.velocity + player.positionCorrection, dt);

	// Clear
	player.positionCorrection = V2fZero();
}

struct World {
	Map map;
	Entity player;
};

static void InitWorld(World &world) {
	world.map = TestLevel::Level;

	InitPlayer(world.player, world.map);
}

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

static void LoadAssets(RenderState &renderState, Assets &assets) {
	// Fonts
	char fontDataPath[1024];
	const char *fontFilename = "lucida_console.ttf";
	fplPathCombine(fontDataPath, fplArrayCount(fontDataPath), 2, assets.dataPath, "fonts");
	FontAsset &hudFont = assets.consoleFont;
	if (LoadFontFromFile(fontDataPath, fontFilename, 0, 24.0f, 32, 128, 512, 512, false, &hudFont.desc)) {
		PushTexture(renderState, &hudFont.texture, hudFont.desc.atlasAlphaBitmap, hudFont.desc.atlasWidth, hudFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
	}
}

static void FreeAssets(Assets &assets) {
	ReleaseFontAsset(assets.consoleFont);
}

static void InitGame(GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// Input
	state->isDebugRendering = true;

	// World
	InitWorld(state->world);
}

extern bool GameInit(GameMemory &gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;

	RenderState *renderState = gameMemory.render;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	LoadAssets(*renderState, state->assets);

	InitGame(state);

	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	if (state != nullptr) {
		FreeAssets(state->assets);
		state->~GameState();
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	RenderState *renderState = gameMemory.render;
	assert(renderState != nullptr);

	// Debug input
	const Controller &keyboardController = input.controllers[0];
	if (WasPressed(keyboardController.debugToggle)) {
		state->isDebugRendering = !state->isDebugRendering;
	}

	// Camera
	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input.windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	Mat4f proj = Mat4OrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	Mat4f view = Mat4TranslationV2(state->camera.offset);
	state->viewProjection = proj * view;

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	// Player input
	InputPlayer(state->world.player, input);
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	const float dt = input.fixedDeltaTime;

	World &world = state->world;
	Map &map = world.map;
	Entity &player = world.player;

	// Player
	UpdatePlayer(player, map, dt);

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input.framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = ScalarAvg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);

	const World &world = state->world;

	const Map &map = world.map;

	RenderState &renderState = *gameMemory.render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	Vec2i mapSize = V2iInit(map.width, map.height);
	Vec2f mapArea = V2fHadamard(TileSize, V2fInit((float)mapSize.x, (float)mapSize.y));
	Vec2f mapExtents = mapArea * 0.5f;
	Vec2f mapOrigin = V2fInit(0,0);
	Vec4f mapSolidColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);

	Vec2f gridSize = mapArea;
	Vec2f gridOrigin = mapOrigin;
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	int gridTileCountX = map.width;
	int gridTileCountY = map.height;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags::Color | ClearFlags::Depth);
	SetMatrix(renderState, state->viewProjection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);

	// Tile grid
	for (int i = 0; i <= gridTileCountX; ++i) {
		float xoffset = i * TileWidth;
		PushLine(renderState, gridOrigin + V2fInit(xoffset, 0), gridOrigin + V2fInit(xoffset, gridSize.y), gridColor, 1.0f);
	}
	for (int i = 0; i <= gridTileCountY; ++i) {
		float yoffset = i * TileHeight;
		PushLine(renderState, gridOrigin + V2fInit(0, yoffset), gridOrigin + V2fInit(gridSize.x, yoffset), gridColor, 1.0f);
	}

	// Map
	for (int y = 0; y < mapSize.h; ++y) {
		for (int x = 0; x < mapSize.w; ++x) {
			uint32_t tile = map.GetTile(x, y);
			if (map.IsObstacle(tile)) {
				Vec2f tilePos = mapOrigin + V2fInit(x * TileWidth, y * TileHeight);
				PushRectangle(renderState, tilePos, TileSize, mapSolidColor, true, 1.0f);
			}
		}
	}

	// Player
	PushRectangleCenter(renderState, state->world.player.position, state->world.player.radius, state->world.player.color, true, 0.0f);

	// Mouse tile
	Vec2f invTileSize = V2fInit(1.0f / TileWidth, 1.0f / TileHeight);
    if (state->mouseWorldPos.x >= -w && state->mouseWorldPos.x <= w &&
        state->mouseWorldPos.y >= -h && state->mouseWorldPos.y <= h) {
		PushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(8, 8), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);

		Vec2i mouseTilePos = map.WorldCoordsToTile(state->mouseWorldPos);
		Vec2f mouseWorldPos = map.TileCoordsToWorld(mouseTilePos);
		Vec2f p = gridOrigin + mouseWorldPos;
		PushRectangle(renderState, p, TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);

		const FontAsset &font = state->assets.consoleFont;
		float fontHeight = 6.0f;

		char buffer[100];
		fplStringFormat(buffer, fplArrayCount(buffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
		PushText(renderState, buffer, fplGetStringLength(buffer), &font.desc, &font.texture, mouseWorldPos, fontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));
	}

	if (state->isDebugRendering) {
		const FontAsset &font = state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 8.0f;

		char sizeCharsBuffer[2][32 + 1];
		FormatSize(gameMemory.memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory.memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState.lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState.memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplStringFormat(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w * 2.0f, blockPos.y), fontHeight, -1.0f, -1.0f, textColor);
	}
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = L"FPL Demo | GameTemplate";
	config.disableInactiveDetection = true;
	int result = GameMain(config);
	return(result);
}