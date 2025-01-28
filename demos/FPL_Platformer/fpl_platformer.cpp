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
		p++;
	}

	size_t tmp = value;
	do {
		p++;
		tmp = tmp / 10;
	} while (tmp);

	// Count thousands
	size_t thousandDotCount = 0;
	tmp = value;
	while (tmp >= 1000) {
		p++;
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
struct TileRect {
	// The minium tile coordinate
	Vec2i min;
	// The maximum tile coordinate
	Vec2i max;
};

struct Projection {
	// The smallest projection
	float min;
	// The largest projection
	float max;
};

struct AABB {
	Vec2f center;
	Vec2f halfExtents;

	const Vec2f RightAxis = V2fInit(1, 0);
	const Vec2f UpAxis = V2fInit(0, 1);

	static AABB Construct(const Vec2f &center, const Vec2f &halfExtents) {
		AABB result = { center, halfExtents };
		return result;
	}

	// Get bottom left corner
	inline Vec2f GetBottomLeft() const {
		Vec2f result = center + V2fInit(-halfExtents.x, -halfExtents.y);
		return result;
	}

	// Get bottom right corner
	inline Vec2f GetBottomRight() const {
		Vec2f result = center + V2fInit(halfExtents.x, -halfExtents.y);
		return result;
	}

	// Get top left corner
	inline Vec2f GetTopLeft() const {
		Vec2f result = center + V2fInit(-halfExtents.x, halfExtents.y);
		return result;
	}

	// Get top right corner
	inline Vec2f GetTopRight() const {
		Vec2f result = center + V2fInit(halfExtents.x, halfExtents.y);
		return result;
	}

	// Get minimum corner
	inline Vec2f GetMin() const {
		return GetBottomLeft();
	}

	// Get maximum corner
	inline Vec2f GetMax() const {
		return GetTopRight();
	}

	// Overwrites this AABB with specified center and half extents
	inline void Update(const Vec2f &center, const Vec2f &halfExtents) {
		this->center = center;
		this->halfExtents = halfExtents;
	}

	// Returns true if the specific AABB overlaps with this AABB
	bool IsOverlap(const AABB &b) const {
		Vec2f centerDelta = V2fAbs(b.center - center);
		Vec2f halfExtentsSum = halfExtents + b.halfExtents;
		centerDelta -= halfExtentsSum;
		bool result = centerDelta.x < 0 && centerDelta.y < 0;
		return result;
	}

	// Returns true if the specific point is inside
	bool IsPointInside(const Vec2f &point) const {
		Vec2f delta = point - center;
		bool result = Abs(delta.x) < halfExtents.w && Abs(delta.y) < halfExtents.h;
		return result;
	}

	// Gets the closest point, by projecting the point to this AABB
	Vec2f GetClosestPoint(const Vec2f &point) const {
		Vec2f result = center;

		Vec2f r = point - center;

		Vec2f n;
		float d;

		// Right axis
		n = V2fInit(1.0f, 0.0f);
		d = V2fDot(r, n);
		d = Min(halfExtents.w, d);
		d = Max(-halfExtents.w, d);
		result = V2fAddMultScalar(result, n, d);

		// Up axis
		n = V2fInit(0.0f, 1.0f);
		d = V2fDot(r, n);
		d = Min(halfExtents.h, d);
		d = Max(-halfExtents.h, d);
		result = V2fAddMultScalar(result, n, d);

		return result;
	}

	// Project the extents to the specified axis
	inline Projection Project(Vec2f axis) const {
		float r = Abs(V2fDot(axis, RightAxis)) * halfExtents.w + Abs(V2fDot(axis, UpAxis)) * halfExtents.h;
		return { -r, +r };
	}
};

struct Contact {
	Vec2f normal;
	Vec2f point;
	float impulse;
	float distance;

	// Overwrites the contact data
	inline void Set(const Vec2f &normal, const float distance, const Vec2f &point) {
		this->normal = normal;
		this->point = point;
		this->impulse = 0;
		this->distance = distance;
	}
};

//
// Map
//
struct Map {
	// Memory handling
	fmemMemoryBlock temporaryMemory;
	fmemMemoryBlock persistentMemory;

	// The origin in tile coordinate
	Vec2i origin;

	// The 1D tile data (width * height)
	uint32_t *solidTiles;

	// The width in tiles
	uint32_t width;

	// The height in tiles
	uint32_t height;

	// Converts the specified world position into a tile position
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

	// Converts the specified tile position into a world position
	inline Vec2f TileCoordsToWorld(const Vec2i tilePos) const {
		float x = (float)tilePos.x * TileWidth;
		float y = (float)tilePos.y * TileHeight;
		return V2fInit(x, y);
	}

	// Gets a tile by the specified X and Y indices, note that Y is converted into tile space
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

	// Gets a tile by the specified tile position, note that Y of the tile position is converted into tile space
	inline uint32_t GetTile(const Vec2i &tilePos) const {
		return GetTile(tilePos.x, tilePos.y);
	}

	// Returns true if the specified tile position is inside the entire tile area
	inline bool IsTileInside(const Vec2i &tilePos) const {
		bool result = (tilePos.x >= 0 && tilePos.x < (int)width) && (tilePos.y >= 0 && tilePos.y < (int)height);
		return result;
	}

	// Returns true if the specified tile is an obstacle or not
	inline bool IsObstacle(const uint32_t tile) const {
		// @TODO(final): Obstacle tile mapping!
		bool result = tile == 1;
		return result;
	}

	// Finds the first tile position from the specified tile type and returns true if found, false otherwise
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
// Tiles
//
namespace Tiles {
	const uint32_t PlayerPosition = (uint32_t)'p';
};

//
// Levels
//
namespace TestLevel {
	constexpr uint32_t Width = 11;
	constexpr uint32_t Height = 8;

	const uint32_t p = Tiles::PlayerPosition;

	static uint32_t Tiles[Width * Height] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
		1, 0, 0, 0, 0, 0, p, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	};

	static Map Level = { {}, {}, {}, Tiles, Width, Height };
};

//
// Game
//
constexpr float MaxSpeed = 100.0f;
constexpr float PlayerWalkSpeed = 30.0f;
constexpr float PlayerAirSpeed = 40.0f;
constexpr float PlayerJumpVelocity = 200.0f * 1.2f;
constexpr float PlayerGroundFriction = 0.2f;
constexpr float PlayerAirFriction = 0.2f;

struct Assets {
	FontAsset consoleFont;
	char dataPath[1024];
};

struct GroundState {
	bool current;
	bool last;
};

struct SensorDefinition {
	// Start/origin of the sensor in unit space, relative to the entity radius
	Vec2f origin;
	// Target unit direction
	Vec2f targetDirection;
	// Line color
	Vec4f color;
	// Minimum distance in unit space, relative to the entity radius
	float minDistance;
};

static const SensorDefinition EntitySensorGroundLeft = { { -0.325f, 0.0f }, {0.0f, -1.0f}, {0.0f, 0.8f, 0.0f, 1.0f}, 0.8f };
static const SensorDefinition EntitySensorGroundRight = {{ 0.325f, 0.0f }, {0.0f, -1.0f}, {0.0f, 0.7f, 0.0f, 1.0f}, 0.8f };
static const SensorDefinition EntitySensorCeilingLeft = {{ -0.325f, 0.0f }, {0.0f, 1.0f}, {1.0f, 0.8f, 0.0f, 1.0f}, 0.8f };
static const SensorDefinition EntitySensorCeilingRight = {{ 0.325f, 0.0f }, {0.0f, 1.0f}, {1.0f, 0.7f, 0.0f, 1.0f}, 0.8f };
static const SensorDefinition EntitySensorWallLeft = {{ 0.0f, 0.0f }, { -1.0f, 0.0f }, {0.7f, 0.0f, 0.0f, 1.0f}, 0.4};
static const SensorDefinition EntitySensorWallRight = {{ 0.0f, 0.0f }, { 1.0f, 0.0f }, {0.6f, 0.0f, 0.0f, 1.0f}, 0.4};

static SensorDefinition EntitySensorDefinitions[6] = {
	EntitySensorGroundLeft,
	EntitySensorGroundRight,
	EntitySensorCeilingLeft,
	EntitySensorCeilingRight,
	EntitySensorWallLeft,
	EntitySensorWallRight,
};

struct Sensor {
	// Color of the line
	Vec4f color;
	// Origin in world units
	Vec2f origin;
	// Target in world units
	Vec2f target;
	// Length in world units
	float length;
	// Is sensor active or not
	fpl_b32 isActive;
};

struct Entity {
	Sensor sensors[6];
	Contact contact;
	Vec4f color;
	Vec2f position;
	Vec2f velocity;
	Vec2f radius;
	float groundFriction;
	float airFriction;
	GroundState groundState;
	bool applyFriction;
	bool applyAirFriction;
	bool jumpRequested;

	inline AABB GetAABB() const {
		AABB aabb = { position, radius };
		return aabb;
	}

	inline Projection Project(const Vec2f &axis) const {
		AABB aabb = GetAABB();
		return aabb.Project(axis);
	}

	inline bool IsGrounded() const {
		return groundState.current;
	}

	inline bool IsAir() const {
		return !groundState.current;
	}
};

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

static void LoadPlayer(Entity &player, const Map &map) {
	player.radius = V2fInit(TileWidth * 0.4f, TileHeight * 0.8f);
	player.velocity = V2fInit(0.0f, 0.0f);
	player.color = V4fInit(0.05f, 0.1f, 0.95f, 1);
	player.position = V2fInit(0.0f, 0.0f);

#if !COLLISION_PLAYGROUND
	Vec2i playerTilePos;
	if (map.FindPositionByTile(Tiles::PlayerPosition, &playerTilePos)) {
		Vec2f tilePos = map.TileCoordsToWorld(playerTilePos);
		Vec2f tileBottomCenter = tilePos + V2f(TileWidth * 0.5f, 0);

		// Move the player above the tile, but to the center
		player.position = tileBottomCenter + V2fInit(0, player.radius.h);

		// Move the player above the tile, but to the right
		player.position = tileBottomCenter + V2fInit(TileSize.w * 0.5f - player.radius.w, player.radius.h);
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

	// Horizontal Movement
	float moveSpeed = player.IsGrounded() ? PlayerWalkSpeed : PlayerAirSpeed;
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
	if (player.IsGrounded() && player.jumpRequested) {
		player.velocity.y = PlayerJumpVelocity;
		player.jumpRequested = false;
	}
}

static void UpdateSensors(Entity &player, const float dt) {
	Vec2f predictedPos = player.position + player.velocity * dt;
	for (uint8_t index = 0; index < 6; ++index) {
		const SensorDefinition &def = EntitySensorDefinitions[index];
		Vec2f distance = V2fHadamard(V2fInitScalar(def.minDistance), TileSize);
		player.sensors[index].isActive = false;
		player.sensors[index].color = def.color;
		player.sensors[index].origin = predictedPos + V2fHadamard(def.origin, TileSize);
		player.sensors[index].length = Abs(V2fDot(distance, def.targetDirection));
		player.sensors[index].target = player.sensors[index].origin + V2fMultScalar(def.targetDirection, player.sensors[index].length);
	}
}

static void UpdatePlayer(Entity &player, const Map &map, const float dt) {
	// Gravity
	//player.velocity += Gravity;

	// Air friction
	if (player.applyAirFriction && player.IsAir() && Abs(player.velocity.x) > 0) {
		player.velocity.x *= (1.0f - player.airFriction);
	}

	// Clamp speed
	player.velocity.x = ScalarClamp(player.velocity.x, -MaxSpeed, MaxSpeed);

	// Grounding
	player.groundState.last = player.groundState.current;
	player.groundState.current = false;

	// Sensors
	UpdateSensors(player, dt);

	// Collision

	// Integrate
	player.position += player.velocity * dt;
}

struct World {
	fmemMemoryBlock memory;
	Map map;
	Entity player;
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

	// Camera
	state->camera.offset = -player.position;
	state->camera.scale = 1;

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

static void PushOrigin(RenderState &renderState, const Vec2f &origin) {
	PushQuad(renderState, origin + V2fInit(0.0f, 2.0f), 1.0f, V4f(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin + V2fInit(0.0f, -2.0f), 1.0f, V4f(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin + V2fInit(-2.0f, 0.0f), 1.0f, V4f(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin + V2fInit(2.0f, 0.0f), 1.0f, V4f(0.75f, 0.75f, 0.75f, 1), true, 1.0f);
	PushQuad(renderState, origin, 1.0f, V4f(0.25f, 0.25f, 0.25f, 1), true, 1.0f);
}

static void PushSensor(RenderState &renderState, const Sensor &sensor, const float width = 1.0f) {
	PushLine(renderState, sensor.origin, sensor.target, sensor.color, width);
	PushQuad(renderState, sensor.target, 1.0f, V4f(1, 1, 1, 1), true, 1.0f);
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);

	const World &world = state->world;

	const Map &map = world.map;

	const Entity &player = world.player;

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

	// Player
	PushRectangleCenter(renderState, player.position, player.radius, player.color, false, 2.0f);
	PushOrigin(renderState, player.position);
	for (uint8_t sensorIndex = 0; sensorIndex < 6; ++sensorIndex) {
		PushSensor(renderState, player.sensors[sensorIndex]);
	}

	// Sensor tiles
	for (uint8_t sensorIndex = 0; sensorIndex < 6; ++sensorIndex) {
		Vec2i tileIndex = map.WorldCoordsToTile(player.sensors[sensorIndex].target);
		Vec2f tilePos = V2fFromV2i(tileIndex);
		Vec2f worldPos = mapOrigin + V2fHadamard(tilePos, TileSize);
		PushRectangle(renderState, worldPos, TileSize, playerTileColor, false, 2.0f);
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
	config.disableVerticalSync = true;
	int result = GameMain(config);
	return(result);
}