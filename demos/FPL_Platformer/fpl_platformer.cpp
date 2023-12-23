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

#define COLLISION_PLAYGROUND 0

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

static Vec2f AABBExpand = V2fInit(0.1f, 0.1f);

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

	static bool IsPointInside(const AABB &box, const Vec2f &point) {
		Vec2f delta = point - box.center;
		bool result = Abs(delta.x) < box.halfExtents.w && Abs(delta.y) < box.halfExtents.h;
		return result;
	}
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

struct TileRect {
	Vec2i min, max;
};

struct Projection {
	float min;
	float max;
};

//
// Map
//
struct Map {
	fmemMemoryBlock temporaryMemory;
	fmemMemoryBlock persistentMemory;
	Vec2i origin;
	uint32_t *solidTiles;
	uint32_t width;
	uint32_t height;
public:
	inline Vec2i WorldCoordsToTile(const Vec2f worldPos) const {
		float wx = worldPos.x;
		float wy = worldPos.y;

		// Adjustment for negative coordinates
		float rx = 0.0f;
		float ry = 0.0f;
		if (wx < 0)
			rx = -1.0f;
		if (wy < 0)
			ry = -1.0f;

		int x = (int)(wx / TileWidth + rx);
		int y = (int)(wy / TileHeight + ry);

		return V2iInit(x, y);
	}
	inline Vec2f TileCoordsToWorld(const Vec2i tilePos) const {
		float x = (float)tilePos.x * TileWidth;
		float y = (float)tilePos.y * TileHeight;
		return V2fInit(x, y);
	}

	inline uint32_t GetTile(const int32_t x, const int32_t y) const {
		if (width == 0 || height == 0 || solidTiles == nullptr) {
			return UINT32_MAX;
		}
		int invY = height - 1 - y;
		if (x < 0 || invY < 0 || x >((int)width - 1) || invY >((int)height - 1)) {
			return UINT32_MAX;
		}
		uint32_t result = solidTiles[invY * width + x];
		return result;
	}

	inline uint32_t GetTile(const Vec2i &tilePos) const {
		return GetTile(tilePos.x, tilePos.y);
	}

	inline bool IsTileInside(const Vec2i &tilePos) const {
		bool result = (tilePos.x >= 0 && tilePos.x < (int)width) && (tilePos.y >= 0 && tilePos.y < (int)height);
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
	static Vec2f RightAxis = V2fInit(1, 0);
	static Vec2f UpAxis = V2fInit(0, 1);

	static bool IsInternalCollision(const Map &map, const Vec2i &tilePos, const Vec2f &normal) {
		int nextTileX = tilePos.x + (int)normal.x;
		int nextTileY = tilePos.y + (int)normal.y;

		uint32_t currentTile = map.GetTile(tilePos.x, tilePos.y);
		uint32_t nextTile = map.GetTile(nextTileX, nextTileY);

		bool internalEdge = map.IsObstacle(nextTile);

		return internalEdge;
	}

	static bool IsPointInsideAABB(const Vec2f &center, const Vec2f &ext, const Vec2f &point) {
		float dx = Abs(point.x - center.x);
		float dy = Abs(point.y - center.y);
		bool result = dx < ext.w && dy < ext.h;
		return result;
	}

	static Vec2f GetClosestPointOnAABB(const Vec2f &center, const Vec2f &ext, const Vec2f &point) {
		Vec2f result = center;

		Vec2f r = point - center;

		Vec2f n;
		float d;

		// Right axis
		n = V2fInit(1.0f, 0.0f);
		d = V2fDot(r, n);
		d = Min(ext.w, d);
		d = Max(-ext.w, d);
		result = V2fAddMultScalar(result, n, d);

		// Up axis
		n = V2fInit(0.0f, 1.0f);
		d = V2fDot(r, n);
		d = Min(ext.h, d);
		d = Max(-ext.h, d);
		result = V2fAddMultScalar(result, n, d);

		return result;
	}

	static Projection AABBProjection(Vec2f center, Vec2f extents, Vec2f normal) {
		float r = Abs(V2fDot(normal, RightAxis)) * extents.w + Abs(V2fDot(normal, UpAxis)) * extents.h;
		return { -r, +r };
	}

	static bool AabbVsAabb(const AABB &a, const AABB &b, Contact &outContact, const Vec2i &tilePos, const Map &map, const bool checkInternal = true) {
		Vec2f delta = b.center - a.center;
		Vec2f sumExtents = a.halfExtents + b.halfExtents;

		Vec2f axes[] =
		{
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
			Projection projA = AABBProjection(a.center, a.halfExtents, n);
			Projection projB = AABBProjection(b.center, b.halfExtents, n);

			// Add relative offset to A´s projection
			float relativeProjection = V2fDot(n, delta);
			projA.min += relativeProjection;
			projA.max += relativeProjection;

			// Calculate overlap and get smallest (greatest negative) projection
			float d0 = projA.min - projB.max;
			float d1 = projB.min - projA.max;
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

		//
		// Separation case: Skip vertex contacts entirely (outside of the voronoi region)
		//
		if (collisionDistance >= 0) {

			// Get closest point and build segment from tangent
			Vec2f closestPoint = GetClosestPointOnAABB(a.center, sumExtents, b.center);
			Vec2f tangent = V2fCrossL(1.0f, collisionNormal);
			Vec2f segmentA = a.center + V2fHadamard(tangent, sumExtents);
			Vec2f segmentB = a.center - V2fHadamard(tangent, sumExtents);

			// Get closest point on segment
			Vec2f n = segmentB - segmentA;
			float segmentLength = V2fDot(n, n);
			Vec2f q = b.center;
			Vec2f pa = q - segmentA;
			Vec2f segmentClosest = segmentA + n * (V2fDot(pa, n) / segmentLength);

			// Get barycentric coordinates and detect a edge or vertex case
			float v = V2fDot(segmentClosest - segmentA, n) / segmentLength;
			float u = V2fDot(segmentB - segmentClosest, n) / segmentLength;
			bool isEdge = u >= 0 && v >= 0;

			if (!isEdge)
				return false; // Skip vertex contacts
		}

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
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, p, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	};

	static Map Level = { {}, {}, {}, Tiles, Width, Height };
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

	inline AABB GetAABB() const {
		AABB aabb = { position, radius };
		return aabb;
	}
};

constexpr float MaxSpeed = 100.0f;
constexpr float PlayerWalkSpeed = 30.0f;
constexpr float PlayerAirSpeed = 40.0f;
constexpr float PlayerJumpVelocity = 200.0f * 1.2f;
constexpr float PlayerGroundFriction = 0.2f;
constexpr float PlayerAirFriction = 0.2f;

static void LoadPlayer(Entity &player, const Map &map) {
	player.radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player.velocity = V2fInit(0.0f, 0.0f);
	player.color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player.position = V2fInit(0.0f, 0.0f);

#if !COLLISION_PLAYGROUND
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
#endif
}

static void InputPlayer(Entity &player, const Input &input) {
	const Controller &controller = (input.defaultControllerIndex == -1) ? input.controllers[0] : input.controllers[input.defaultControllerIndex];

#if !COLLISION_PLAYGROUND
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
#endif
}

static void CollisionResponse(Entity &player, const Vec2f &normal, const float dist, const float dt, const Vec2i &tilePos) {
	// Get the separation and penetration separately, this is to stop pentration from causing the objects to ping apart
	float separation = Max(dist, 0);
	float penetration = Min(dist, 0);

	// Compute relative normal velocity require to be object to an exact stop at the surface
	float nv = V2fDot(player.velocity, normal) + separation / dt;

	// Accumulate the penetration correction, this is applied in the update function and ensures we don't add any energy to the system
	player.positionCorrection -= V2fMultScalar(normal, penetration / dt);

	fplDebugFormatOut("Collision response on normal (%.6f, %.6f), separation: %.6f, penetration: %.6f, nv: %.6f, tile: (%d x %d)\n", normal.x, normal.y, separation, penetration, nv, tilePos.x, tilePos.y);

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

static TileRect ComputeTileRect(const Entity &player, const Map &map, const Vec2f &nextPos) {
	// Find min/max
	Vec2f min = V2fMin(player.position, nextPos);
	Vec2f max = V2fMax(player.position, nextPos);

	// Adjust by map origin
	Vec2f originWorld = map.TileCoordsToWorld(map.origin);
	min -= originWorld;
	max -= originWorld;

	// Extent by radius
	min -= player.radius;
	max += player.radius;

	// Expand a bit more to really capture all tiles
	min -= AABBExpand;
	max += AABBExpand;

	// Get tile range min/max
	Vec2i tileMin = map.WorldCoordsToTile(min);
	Vec2i tileMax = map.WorldCoordsToTile(max + V2fInit(0.5f, 0.5f));

	return { tileMin, tileMax };
}

static void DetectCollision(Entity &player, const Map &map, const float dt) {
	// Predict position
	Vec2f predictedPos = V2fAddMultScalar(player.position, player.velocity, dt);

	// Get tile min/max
	TileRect tileRect = ComputeTileRect(player, map, predictedPos);

	// Player bounds
	AABB playerAABB = player.GetAABB();

	Vec2f originWorld = map.TileCoordsToWorld(map.origin);

	// Collide against all solid tiles
	for (int x = tileRect.min.x; x <= tileRect.max.x; ++x) {
		for (int y = tileRect.min.y; y <= tileRect.max.y; ++y) {
			uint32_t tile = map.GetTile(x, y);
			if (map.IsObstacle(tile)) {
				Vec2i tilePos = V2iInit(x, y);
				Vec2f tileWorld = map.TileCoordsToWorld(tilePos) + originWorld;
				Vec2f aabbCenter = tileWorld + TileSize * 0.5f;
				AABB tileAABB = { aabbCenter, TileSize * 0.5f };
				bool collided = Collision::AabbVsAabb(tileAABB, playerAABB, player.contact, tilePos, map);
				if (collided) {
					CollisionResponse(player, player.contact.normal, player.contact.distance, dt, tilePos);
				}
			}
		}
	}
}

static void UpdatePlayer(Entity &player, const Map &map, const float dt) {
#if !COLLISION_PLAYGROUND
	// Gravity
	player.velocity += Gravity;

	// Air friction
	if (player.applyAirFriction && !player.onGround[0] && Abs(player.velocity.x) > 0) {
		player.velocity.x *= (1.0f - player.airFriction);
	}
#endif

	// Clamp speed
	player.velocity.x = ScalarClamp(player.velocity.x, -MaxSpeed, MaxSpeed);

#if !COLLISION_PLAYGROUND
	// Grounding
	player.onGround[1] = player.onGround[0];
	player.onGround[0] = false;

	// Collision detection
	DetectCollision(player, map, dt);
#endif

	// Integrate
	player.position += (player.velocity + player.positionCorrection) * dt;

	// Clear
	player.positionCorrection = V2fZero();
}

struct World {
	fmemMemoryBlock memory;
	Map map;
	Entity player;
	Entity box;
};

// One time initialization of the map
static void InitMap(fmemMemoryBlock *memory, Map &map) {
	map = {};
	fmemPushBlock(memory, &map.persistentMemory, fplMegaBytes(8), fmemPushFlags_Clear);
	fmemPushBlock(memory, &map.temporaryMemory, fplMegaBytes(8), fmemPushFlags_Clear);
}

static void LoadMap(Map &map, const Map &source) {
	map.persistentMemory.used = 0;
	map.temporaryMemory.used = 0;

	size_t requiredSize = source.width * source.height * sizeof(uint32_t);
	fplAssert(requiredSize <= map.persistentMemory.size);

	map.solidTiles = (uint32_t *)fmemPush(&map.persistentMemory, requiredSize, fmemPushFlags_Clear);
	map.width = source.width;
	map.height = source.height;
	map.origin = source.origin;
	for (uint32_t tileIndex = 0; tileIndex < source.width * source.height; ++tileIndex) {
		map.solidTiles[tileIndex] = source.solidTiles[tileIndex];
	}
}

// One time initialization of the world
static void InitWorld(fmemMemoryBlock *memory, World &world) {
	world = {};

	fmemPushBlock(memory, &world.memory, fplMegaBytes(64), fmemPushFlags_Clear);

	InitMap(&world.memory, world.map);
}

// Load entire world (can be called anytime)
static void LoadWorld(World &world, const Map &level) {
	LoadMap(world.map, level);

	world.box.color = V4fInit(0.0f, 1.0f, 0.0f, 1.0f);
	world.box.radius = V2fInit(TileWidth * 0.5f, TileHeight * 0.5f);
	world.box.position = V2fInit(TileWidth * 3.0f, 0.0f);

	LoadPlayer(world.player, world.map);
}

struct Editor {
	uint32_t drawTile;
	bool isDrawing;
};

struct GameState {
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

static void InitGame(fmemMemoryBlock *memory, GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// Input
	state->isDebugRendering = true;

	// World
	InitWorld(memory, state->world);
}

static void LoadGame(GameState *state) {
	// Camera
	state->camera.scale = 1.0f;
	state->camera.offset.x = 0;
	state->camera.offset.y = 0;

	// World
	LoadWorld(state->world, TestLevel::Level);
}

extern bool GameInit(GameMemory &gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;

	RenderState *renderState = gameMemory.render;

	fplGetExecutableFilePath(state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplExtractFilePath(state->assets.dataPath, state->assets.dataPath, fplArrayCount(state->assets.dataPath));
	fplPathCombine(state->assets.dataPath, fplArrayCount(state->assets.dataPath), 2, state->assets.dataPath, "data");

	LoadAssets(*renderState, state->assets);

	InitGame(gameMemory.memory, state);

	LoadGame(state);

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

static void DrawTile(Map &map, const Vec2i &tilePos, const uint32_t newTile) {
	Vec2i newOrigin = map.origin;
	Vec2i newSizeAppend = V2iInit(0, 0);
	Vec2i newTilePos = tilePos;
	if (tilePos.x < 0) {
		int xcount = abs(tilePos.x);
		fplAssert(xcount > 0);
		newSizeAppend.x += xcount;
		newOrigin.x -= xcount;
		newTilePos.x += xcount;
	} else if (tilePos.x > ((int)map.width - 1)) {
		int xcount = tilePos.x - ((int)map.width - 1);
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
	} else if (tilePos.y > ((int)map.height - 1)) {
		int ycount = tilePos.y - ((int)map.height - 1);
		fplAssert(ycount > 0);
		newSizeAppend.y += ycount;
		newTilePos.y = tilePos.y;
	}

	if (newSizeAppend.x > 0 || newSizeAppend.y > 0) {
		Vec2i oldMapSize = V2iInit(map.width, map.height);
		Vec2i newMapSize = V2iInit(map.width + newSizeAppend.x, map.height + newSizeAppend.y);

		map.temporaryMemory.used = 0;

		fmemMemoryBlock tempBlock;
		fmemBeginTemporary(&map.temporaryMemory, &tempBlock);

		size_t requiredOldSize = oldMapSize.w * oldMapSize.h * sizeof(uint32_t);
		fplAssert(requiredOldSize <= tempBlock.size);

		uint32_t *oldTiles = (uint32_t *)fmemPush(&tempBlock, requiredOldSize, fmemPushFlags_None);
		fplMemoryCopy(map.solidTiles, requiredOldSize, oldTiles);

		map.persistentMemory.used = 0;
		size_t requiredNewSize = newMapSize.w * newMapSize.h * sizeof(uint32_t);
		fplAssert(requiredNewSize <= map.persistentMemory.size);

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

		map.width = newMapSize.w;
		map.height = newMapSize.h;
		map.solidTiles = (uint32_t *)fmemPush(&map.persistentMemory, requiredNewSize, fmemPushFlags_Clear);
		for (int y = 0; y < oldMapSize.h; ++y) {
			for (int x = 0; x < oldMapSize.w; ++x) {
				int ox = newSizeAppend.x + x + offsetX;
				int oy = newSizeAppend.y + y + offsetY;
				fplAssert(ox >= 0 && ox < newMapSize.w);
				fplAssert(oy >= 0 && oy < newMapSize.h);
				map.solidTiles[oy * newMapSize.w + ox] = oldTiles[y * oldMapSize.w + x];
			}
		}

		fmemEndTemporary(&map.temporaryMemory);
	}

	int invY = map.height - 1 - newTilePos.y;
	int curTile = map.solidTiles[invY * map.width + newTilePos.x];
	map.solidTiles[invY * map.width + newTilePos.x] = newTile;

	map.origin = newOrigin;
}

static void EditorInput(GameState *state, const Input &input) {
	Map &map = state->world.map;

	Editor &editor = state->editor;

	Vec2f originWorld = map.TileCoordsToWorld(map.origin);

	Vec2i mouseTilePos = map.WorldCoordsToTile(state->mouseWorldPos - originWorld);

	if (IsDown(input.mouse.left)) {
		if (!editor.isDrawing) {
			editor.isDrawing = true;

			if (map.IsTileInside(mouseTilePos)) {
				uint32_t tile = map.GetTile(mouseTilePos);
				editor.drawTile = tile == 0 ? 1 : 0;
			} else {
				editor.drawTile = 1;
			}
		}
		if (editor.isDrawing) {
			fplAssert(editor.drawTile != UINT32_MAX);
			DrawTile(map, mouseTilePos, editor.drawTile);
		}
	} else {
		if (editor.isDrawing) {
			editor.drawTile = UINT32_MAX;
			editor.isDrawing = false;
		}
	}
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
	state->projection = Mat4OrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	state->view = Mat4TranslationV2(state->camera.offset);
	state->viewProjection = state->projection * state->view;

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	// Editor input
	EditorInput(state, input);

#if COLLISION_PLAYGROUND
	Entity *entities[] = {
		&state->world.player,
		&state->world.box,
	};

	if (IsDown(input.mouse.left)) {
		if (!state->isDragging) {

			Entity *dragEntity = nullptr;

			AABB aabb;
			for (int i = 0; i < fplArrayCount(entities); ++i) {
				Entity *entity = entities[i];
				aabb = { entity->position, entity->radius };
				if (AABB::IsPointInside(aabb, state->mouseWorldPos)) {
					dragEntity = entity;
					break;
				}
			}

			if (dragEntity != nullptr) {
				state->dragEntity = dragEntity;
				state->isDragging = true;
				state->dragStart = state->mouseWorldPos;
			}
		} else {
			fplAssertPtr(state->dragEntity);
			Vec2f deltaPos = state->mouseWorldPos - state->dragStart;
			state->dragEntity->position += deltaPos;
			state->dragStart = state->mouseWorldPos;
		}
	} else {
		if (state->isDragging) {
			state->isDragging = false;
			state->dragEntity = nullptr;
			state->dragStart = V2fZero();
		}
	}
#endif

#if !COLLISION_PLAYGROUND
	// Player input
	InputPlayer(state->world.player, input);
#endif
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

#if !COLLISION_PLAYGROUND
	// Camera
	state->camera.offset = -player.position;
	state->camera.scale = 1;
#endif

	// FPS display
	const float fpsSmoothing = 0.1f;

	const float newFps = input.framesPerSeconds;
	const float oldFps = state->framesPerSecond[0];

	state->deltaTime = dt;
	state->framesPerSecond[1] = ScalarAvg(oldFps, fpsSmoothing, newFps);
	state->framesPerSecond[0] = state->framesPerSecond[1];
}

static void PushNormal(RenderState &renderState, const Vec2f &position, const Vec2f &normal, const float length = 20.0f, const float chairSize = 8.0f) {
	Vec2f tangent = V2fCrossR(normal, 1.0f);

	Vec2f a = position;
	Vec2f b = a + normal * length;
	PushLine(renderState, a, b, V4fInit(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

	a = position + normal * chairSize;
	b = a + tangent * chairSize;
	PushLine(renderState, a, b, V4fInit(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	a = position + tangent * chairSize;
	b = a + normal * chairSize;
	PushLine(renderState, a, b, V4fInit(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);

	const World &world = state->world;

	const Map &map = world.map;

	const Entity &player = world.player;

	const Entity &box = world.box;

	RenderState &renderState = *gameMemory.render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	Vec2i mapSize = V2iInit(map.width, map.height);
	Vec2f mapArea = V2fHadamard(TileSize, V2fInit((float)mapSize.x, (float)mapSize.y));
	Vec2f mapExtents = mapArea * 0.5f;
	Vec2f mapOrigin = map.TileCoordsToWorld(map.origin);
	Vec4f mapSolidColor = V4fInit(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4f playerTileColor = V4fInit(0.3f, 0.1f, 0.7f, 1.0f);

	Vec2f gridSize = mapArea;
	Vec2f gridOrigin = mapOrigin;
	Vec4f gridColor = V4fInit(0.1f, 0.2f, 0.1f, 1.0f);
	int gridTileCountX = map.width;
	int gridTileCountY = map.height;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0, 0, 0, 1), ClearFlags::Color | ClearFlags::Depth);
	SetMatrix(renderState, state->projection);

	// World size
	PushRectangle(renderState, V2fInit(-w, -h), V2fInit(w * 2, h * 2), V4fInit(1.0f, 1.0f, 0.0f, 1.0f), false, 1.0f);

	SetMatrix(renderState, state->viewProjection);

	// World cross
	PushLine(renderState, V2fInit(0.0f, -h), V2fInit(0.0f, h), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);
	PushLine(renderState, V2fInit(-w, 0.0f), V2fInit(w, 0.0f), V4fInit(1.0f, 0.0f, 0.0f, 0.5f), 1.0f);

#if !COLLISION_PLAYGROUND
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
				Vec2f tilePos = gridOrigin + V2fInit(x * TileWidth, y * TileHeight);
				PushRectangle(renderState, tilePos, TileSize, mapSolidColor, true, 1.0f);
			}
		}
	}
#endif

	// Player
	PushRectangleCenter(renderState, player.position, player.radius, player.color, false, 2.0f);
	PushCircle(renderState, player.position, 2.0f, 8, V4fInit(1, 1, 1, 1), true, 0);

#if COLLISION_PLAYGROUND
	// Box
	PushRectangleCenter(renderState, box.position, box.radius, box.color, false, 2.0f);
	PushCircle(renderState, box.position, 2.0f, 8, V4fInit(1, 1, 1, 1), true, 0);

	// Minkowski sum
	Vec2f deltaPos = box.position - player.position;
	Vec2f sumExtents = player.radius + box.radius;
	PushRectangleCenter(renderState, box.position, sumExtents, V4f(0.25, 0.25f, 0.25f, 1.0f), false, 1.0f);

	bool insideBox = Collision::IsPointInsideAABB(box.position, sumExtents, player.position);

	float u, v;
	Vec2f segmentA;
	Vec2f segmentB;
	if (insideBox) {
		//
		// Penetration case (SAT)
		//
		Vec2f axes[] = {
			V2fInit(1.0f, 0.0f),
			V2fInit(0.0f, 1.0f),
		};

		int axisIndex = -1;
		Vec2f normal = V2fZero();
		float distance = FLT_MAX;
		for (int i = 0; i < fplArrayCount(axes); ++i) {
			Vec2f n = axes[i];
			float proj = V2fDot(deltaPos, n);

			// Project A and B on normal
			Projection projA = Collision::AABBProjection(box.position, box.radius, n);
			Projection projB = Collision::AABBProjection(player.position, player.radius, n);

			// Add relative offset to B´s projection
			projB.min += proj;
			projB.max += proj;

			// Calculate overlap and get smallest (greatest negative) projection
			float d0 = projA.min - projB.max;
			float d1 = projB.min - projA.max;
			float overlap = d0 > d1 ? d0 : d1;
			if (axisIndex == -1 || overlap > distance) {
				axisIndex = i;
				normal = n;
				distance = overlap;
			}
		}

		fplAssert(axisIndex > -1);

		// Make sure the collision normal faces in the right direction
		if (V2fDot(normal, deltaPos) < 0) {
			normal = -normal;
		}

		// Always invert normal
		normal = -normal;

		Vec2f closestPointA = box.position + V2fHadamard(normal, box.radius);
		PushCircle(renderState, closestPointA, 2.0f, 8, V4fInit(1, 1, 0, 1), true, 0);

		// Find surface segment from direction
		Vec2f tangent = V2fCrossR(normal, -1.0f);
		Vec2f segmentA = box.position + V2fHadamard(normal, box.radius) + V2fHadamard(tangent, -box.radius);
		Vec2f segmentB = box.position + V2fHadamard(normal, box.radius) + V2fHadamard(tangent, box.radius);
		PushLine(renderState, segmentA, segmentB, V4fInit(0.3f, 0.2f, 0.7f, 1.0f), 2.0f);

		// Get closest point in segment
		Vec2f q = player.position;
		Vec2f n = segmentB - segmentA;
		Vec2f pa = player.position - segmentA;
		float l = V2fDot(n, n);
		Vec2f segmentClosest = segmentA + n * (V2fDot(pa, n) / l);
		PushCircle(renderState, segmentClosest, 2.0f, 8, V4fInit(0, 1, 1, 1), true, 0);

		// Get barycentric coordinates
		v = V2fDot(segmentClosest - segmentA, n) / l;
		u = V2fDot(segmentB - segmentClosest, n) / l;

		u = v = 0;
	} else {
		//
		// Separation case (Closest Point)
		//
		// Closest point on box (separated)
		Vec2f closestPointMinkowski = Collision::GetClosestPointOnAABB(box.position, sumExtents, player.position);
		PushCircle(renderState, closestPointMinkowski, 2.0f, 8, V4fInit(0, 1, 0, 1), true, 0);

		// Direction
		Vec2f relativeToClosestPoint = player.position - closestPointMinkowski;
		Vec2f direction = V2fNormalize(relativeToClosestPoint);
		Vec2f majorDirection = V2fMajorAxis(deltaPos) * -1.0f;
		PushNormal(renderState, closestPointMinkowski, direction);

		// Get actual closest point on box
		Vec2f specialDirection = V2fInit(SignF32(direction.x), SignF32(direction.y));
		Vec2f closestPointA = box.position + V2fHadamard(specialDirection, box.radius);
		PushCircle(renderState, closestPointA, 2.0f, 8, V4fInit(1, 1, 0, 1), true, 0);

		// Find distance
		float distanceToClosestPoint = V2fDot(relativeToClosestPoint, direction);
		Vec2f closestPointB = box.position + direction * distanceToClosestPoint;
		PushCircle(renderState, closestPointB, 2.0f, 8, V4fInit(1, 0, 0, 1), true, 0);

		// Find surface segment from direction
		Vec2f majorTangent = V2fCrossR(majorDirection, -1.0f);
		Vec2f segmentA = box.position + V2fHadamard(majorDirection, sumExtents) + V2fHadamard(majorTangent, -sumExtents);
		Vec2f segmentB = box.position + V2fHadamard(majorDirection, sumExtents) + V2fHadamard(majorTangent, sumExtents);
		PushLine(renderState, segmentA, segmentB, V4fInit(0.3f, 0.2f, 0.7f, 1.0f), 2.0f);

		// Get closest point in segment
		Vec2f q = player.position;
		Vec2f n = segmentB - segmentA;
		Vec2f pa = player.position - segmentA;
		float l = V2fDot(n, n);
		Vec2f segmentClosest = segmentA + n * (V2fDot(pa, n) / l);
		PushCircle(renderState, segmentClosest, 2.0f, 8, V4fInit(0, 1, 1, 1), true, 0);

		// Get barycentric coordinates
		v = V2fDot(segmentClosest - segmentA, n) / l;
		u = V2fDot(segmentB - segmentClosest, n) / l;
	}



	bool isEdge = u >= 0 && v >= 0;

	const FontAsset &collisionFont = state->assets.consoleFont;
	float collisionFontHeight = 6.0f;

	char collisionTextBuffer[100];

	fplStringFormat(collisionTextBuffer, fplArrayCount(collisionTextBuffer), "v: %f", v);
	PushText(renderState, collisionTextBuffer, fplGetStringLength(collisionTextBuffer), &collisionFont.desc, &collisionFont.texture, segmentA, collisionFontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));

	fplStringFormat(collisionTextBuffer, fplArrayCount(collisionTextBuffer), "u: %f", u);
	PushText(renderState, collisionTextBuffer, fplGetStringLength(collisionTextBuffer), &collisionFont.desc, &collisionFont.texture, segmentB, collisionFontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));

	fplStringFormat(collisionTextBuffer, fplArrayCount(collisionTextBuffer), "%s", isEdge ? "Edge" : "Vertex");
	PushText(renderState, collisionTextBuffer, fplGetStringLength(collisionTextBuffer), &collisionFont.desc, &collisionFont.texture, box.position, collisionFontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));
#endif

#if !COLLISION_PLAYGROUND
	// Player tile rects
	TileRect playerTileRect = ComputeTileRect(player, map, player.position + player.velocity * dt);
	for (int y = playerTileRect.min.y; y <= playerTileRect.max.y; ++y) {
		for (int x = playerTileRect.min.x; x <= playerTileRect.max.x; ++x) {
			Vec2f tilePos = mapOrigin + V2fInit(x * TileWidth, y * TileHeight);
			PushRectangle(renderState, tilePos, TileSize, playerTileColor, false, 2.0f);
		}
	}

	// Mouse cursor
	PushRectangleCenter(renderState, state->mouseWorldPos, V2fInit(2, 2), V4fInit(1.0f, 0.0f, 0.0f, 1.0f), true, 0.0f);

	// Mouse tile
	Vec2f invTileSize = V2fInit(1.0f / TileWidth, 1.0f / TileHeight);

	Vec2i mouseTilePos = map.WorldCoordsToTile(state->mouseWorldPos - mapOrigin);
	Vec2f mouseWorldPos = map.TileCoordsToWorld(mouseTilePos);
	Vec2f p = gridOrigin + mouseWorldPos;
	PushRectangle(renderState, p, TileSize, V4fInit(1, 1, 1, 1), false, 1.0f);

	const FontAsset &font = state->assets.consoleFont;
	float fontHeight = 6.0f;

	char buffer[100];
	fplStringFormat(buffer, fplArrayCount(buffer), "%i x %i", mouseTilePos.x, mouseTilePos.y);
	PushText(renderState, buffer, fplGetStringLength(buffer), &font.desc, &font.texture, mouseWorldPos, fontHeight, 1.0f, -1.0f, V4fInit(1, 1, 1, 1));
#endif

	if (state->isDebugRendering) {
		SetMatrix(renderState, state->projection);

		const FontAsset &font = state->assets.consoleFont;
		char text[256];
		Vec4f textColor = V4fInit(1, 1, 1, 1);
		Vec4f blackColor = V4fInit(0, 0, 0, 1);
		Vec2f blockPos = V2fInit(-w, h);
		float fontHeight = 8.0f;

		char sizeCharsBuffer[2][32 + 1];
		FormatSize(gameMemory.memory->used, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(gameMemory.memory->size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Game Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x - 1, blockPos.y - 1), fontHeight, 1.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x, blockPos.y), fontHeight, 1.0f, -1.0f, textColor);

		FormatSize(renderState.lastMemoryUsage, fplArrayCount(sizeCharsBuffer[0]), sizeCharsBuffer[0]);
		FormatSize(renderState.memory.size, fplArrayCount(sizeCharsBuffer[1]), sizeCharsBuffer[1]);
		fplStringFormat(text, fplArrayCount(text), "Render Memory: %s / %s bytes", sizeCharsBuffer[0], sizeCharsBuffer[1]);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w - 1, blockPos.y - 1), fontHeight, 0.0f, -1.0f, blackColor);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w, blockPos.y), fontHeight, 0.0f, -1.0f, textColor);
		fplStringFormat(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond[1], state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2fInit(blockPos.x + w * 2.0f - 1, blockPos.y - 1), fontHeight, -1.0f, -1.0f, blackColor);
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