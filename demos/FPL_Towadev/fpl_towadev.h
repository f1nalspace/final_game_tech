#ifndef FPL_TOWADEV_H
#define FPL_TOWADEV_H

#include "final_vecmath.h"

constexpr float RadToDeg = 180.0f / (float)M_PI;

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
constexpr float WorldRadiusW = WorldWidth * 0.5f;
constexpr float WorldRadiusH = WorldHeight * 0.5f;

constexpr float TileSize = 1.0f;
constexpr int TileCountX = (int)(WorldWidth / TileSize);
constexpr int TileCountY = (int)(WorldHeight / TileSize);
constexpr int TotalTileCount = TileCountX * TileCountY;

constexpr float GridWidth = TileSize * (float)TileCountX;
constexpr float GridHeight = TileSize * (float)TileCountY;
constexpr float GridOriginX = -WorldRadiusW + ((WorldWidth - GridWidth) * 0.5f);
constexpr float GridOriginY = -WorldRadiusH + ((WorldHeight - GridHeight) * 0.5f);

struct TilesetInfo {
	uint32_t tileCount;
	uint32_t columnCount;
};

enum class EntityType {
	None = 0,
	SpawnRight,
	SpawnLeft,
	SpawnDown,
	SpawnUp,
	WaypointRight,
	WaypointLeft,
	WaypointDown,
	WaypointUp,
	Goal,
};
class EntitiesTilesetMappingClass : public ArrayInitializer<uint32_t, EntityType, 16> {
public:
	EntitiesTilesetMappingClass() : ArrayInitializer() {
		Set(1, EntityType::SpawnRight);
		Set(2, EntityType::SpawnLeft);
		Set(3, EntityType::SpawnDown);
		Set(4, EntityType::SpawnUp);
		Set(5, EntityType::WaypointRight);
		Set(6, EntityType::WaypointLeft);
		Set(7, EntityType::WaypointDown);
		Set(8, EntityType::WaypointUp);
		Set(9, EntityType::Goal);
	}
};
static const TilesetInfo TilesetEntitiesInfo = { 16, 4 };
static const EntitiesTilesetMappingClass TilesetEntitiesToTypeMapping = EntitiesTilesetMappingClass();

enum class WayType {
	None = 0,
	Horizontal,
	Vertical,
	Cross,
	EdgeUpLeft,
	EdgeUpRight,
	EdgeDownLeft,
	EdgeDownRight,
	EndUp,
	EndDown,
	EndLeft,
	EndRight,
};
class WayTilesetMappingClass : public ArrayInitializer<uint32_t, WayType, 16> {
public:
	WayTilesetMappingClass() : ArrayInitializer() {
		Set(1, WayType::Horizontal);
		Set(2, WayType::Vertical);
		Set(3, WayType::Cross);
		Set(5, WayType::EdgeUpLeft);
		Set(6, WayType::EdgeUpRight);
		Set(7, WayType::EndUp);
		Set(9, WayType::EdgeDownLeft);
		Set(10, WayType::EdgeDownRight);
		Set(11, WayType::EndDown);
		Set(13, WayType::EndLeft);
		Set(14, WayType::EndRight);
	}
};
static const TilesetInfo TilesetWayInfo = { 16, 4 };
static const WayTilesetMappingClass TilesetWayToTypeMapping = WayTilesetMappingClass();

struct SpawnData {
	Vec2f direction;
	float initialCooldown;
	float cooldown;
	char enemyId[256];
};

enum class ObjectType {
	None = 0,
	Spawn,
};

struct ObjectData {
	ObjectType type;
	Vec2i tilePos;
	union {
		SpawnData spawn;
	};
};

struct LevelData {
	uint32_t wayLayer[TotalTileCount];
	uint32_t entitiesLayer[TotalTileCount];
	ObjectData objects[256];
	size_t objectCount;
	uint32_t wayFirstGid;
	uint32_t entitiesFirstGid;
	uint32_t tileWidth;
	uint32_t tileHeight;
};

struct Tile {
	WayType wayType;
	EntityType entityType;
	bool isOccupied;
};

struct Waypoint {
	Vec2i tilePos;
	Vec2f position;
	Vec2f direction;
	struct Waypoint *next;
};

enum class CreepStyle {
	None,
	Quad,
	Triangle
};

struct CreepData {
	Vec4f color;
	const char *id;
	float renderRadius;
	float collisionRadius;
	float speed;
	float hp;
	CreepStyle style;
};

struct CreepMultiplier {
	float hp;
	float speed;
	float scale;
};

inline CreepMultiplier MakeCreepMultiplier(const float hp, const float speed, const float scale) {
	CreepMultiplier result = {};
	result.hp = hp;
	result.speed = speed;
	result.scale = scale;
	return(result);
}

inline CreepData MakeCreepData(const char *id, const float renderRadius, const float collisionRadius, const float speed, const float hp, const Vec4f &color, const CreepStyle style) {
	CreepData result = {};
	result.id = id;
	result.renderRadius = renderRadius;
	result.collisionRadius = collisionRadius;
	result.speed = speed;
	result.hp = hp;
	result.color = color;
	result.style = style;
	return(result);
}

struct Creep {
	CreepData data;
	Vec2f prevPosition;
	Vec2f position;
	Vec2f facingDirection;
	Vec2f targetPos;
	const Waypoint *targetWaypoint;
	float speed;
	float hp;
	bool hasTarget;
	bool isDead;
};

struct CreepSpawner {
	CreepData spawnTemplate;
	Vec2f spawnPosition;
	Vec2f exitPosition;
	size_t totalCount;
	size_t remainingCount;
	float spawnTimer;
	float cooldown;
	bool isActive;
};

struct BulletData {
	float renderRadius;
	float collisionRadius;
	float speed;
	float damage;
};

inline BulletData MakeBulletData(const float renderRadius, const float collisionRadius, const float speed, const float damage) {
	BulletData result = {};
	result.renderRadius = renderRadius;
	result.collisionRadius = collisionRadius;
	result.speed = speed;
	result.damage = damage;
	return(result);
}

struct TowerData {
	BulletData bullet;
	const char *id;
	float structureRadius;
	float detectionRadius;
	float unlockRadius;
	float gunTubeLength;
	float gunCooldown;
};

inline TowerData MakeTowerData(const char *id, const float structureRadius, const float detectionRadius, const float unlockRadius, const float gunTubeLength, const float gunCooldown, const BulletData &bullet) {
	TowerData result = {};
	result.id = id;
	result.structureRadius = structureRadius;
	result.detectionRadius = detectionRadius;
	result.unlockRadius = unlockRadius;
	result.gunTubeLength = gunTubeLength;
	result.gunCooldown = gunCooldown;
	result.bullet = bullet;
	return(result);
}

struct Wave {
	const char *levelId;
	CreepMultiplier enemyMultiplier;
	size_t enemyCount;
};

inline Wave MakeWave(const char *levelId, const CreepMultiplier &enemyMultiplier, const size_t enemyCount) {
	Wave result = {};
	result.levelId = levelId;
	result.enemyMultiplier = enemyMultiplier;
	result.enemyCount = enemyCount;
	return(result);
}

struct Tower {
	TowerData data;
	Vec2f position;
	Creep *targetEnemy;
	float facingAngle;
	float gunTimer;
	bool hasTarget;
};

struct Bullet {
	BulletData data;
	Vec2f prevPosition;
	Vec2f position;
	Vec2f velocity;
	bool isDestroyed;
};

struct FontAsset {
	LoadedFont desc;
	GLint texture;
};

struct GameState {
	GameMemory mem;
	Creep enemies[100];
	Tower towers[100];
	Bullet bullets[10000];
	Tile tiles[TotalTileCount];
	CreepSpawner spawners[16];
	LevelData levelData;
	char activeLevelId[256];
	FontAsset hudFont;
	char dataPath[1024];
	size_t spawnerCount;
	Camera2D camera;
	Viewport viewport;
	Vec2f mouseWorldPos;
	Vec2i mouseTilePos;
	Waypoint *firstWaypoint;
	Waypoint *lastWaypoint;
	size_t activeEnemyCount;
	size_t enemyCount;
	size_t towerCount;
	size_t bulletCount;
	int selectedTowerIndex;
	int waveIndex;
	bool isExiting;
};

inline Vec2f TileToWorld(const Vec2i &tilePos, const Vec2f &offset = V2f(0, 0)) {
	Vec2f result = {
		GridOriginX + tilePos.x * TileSize,
		GridOriginY + (TileCountY - 1 - tilePos.y) * TileSize
	};
	result += offset;
	return(result);
}

inline Vec2i WorldToTile(const Vec2f &worldPos) {
	float x = worldPos.x + WorldWidth * 0.5f;
	float y = worldPos.y + WorldHeight * 0.5f;
	Vec2i result = {
		(int)floorf((x) / TileSize),
		TileCountY - 1 - (int)floorf((y) / TileSize)
	};
	return(result);
}

#endif // FPL_TOWADEV_H
