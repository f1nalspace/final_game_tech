#ifndef FPL_TOWADEV_H
#define FPL_TOWADEV_H

#include <final_math.h>

#include <final_fontloader.h>

#include <final_render.h>

constexpr float RadToDeg = 180.0f / (float)M_PI;

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
constexpr float WorldRadiusW = WorldWidth * 0.5f;
constexpr float WorldRadiusH = WorldHeight * 0.5f;

constexpr float DefaultLineWidth = 2.0f;

constexpr int TileCountX = 21;
constexpr int TileCountY = 11;
constexpr float TileWidth = WorldWidth / (float)TileCountX;
constexpr float TileHeight = WorldHeight / (float)(TileCountY + 1);
const Vec2f TileExt = V2f(TileWidth, TileHeight) * 0.5f;
constexpr float MaxTileSize = FPL_MAX(TileWidth, TileHeight);
constexpr float MaxTileRadius = MaxTileSize * 0.5f;
constexpr int TotalTileCount = TileCountX * TileCountY;

constexpr float ControlsWidth = WorldWidth;
constexpr float ControlsHeight = TileHeight;
constexpr float ControlsOriginX = -WorldRadiusW;
constexpr float ControlsOriginY = -WorldRadiusH;

constexpr float GridWidth = TileWidth * (float)TileCountX;
constexpr float GridHeight = TileHeight * (float)TileCountY;
constexpr float GridOriginX = -WorldRadiusW + ((WorldWidth - GridWidth) * 0.5f);
constexpr float GridOriginY = -WorldRadiusH + ControlsHeight;

const Vec4f TextBackColor = V4f(0.2f, 0.2f, 0.8f, 1);
const Vec4f TextForeColor = V4f(1, 1, 1, 1);

typedef const void *UIID;

struct UIInput {
	Vec2f userPosition;
	ButtonState leftButton;
};

struct UIContext {
	UIInput input;
	UIID hot;
	UIID active;
	struct GameState *gameState;
};

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

enum class SpawnerStartMode {
	Fixed = 0,
	AfterTheLast,
};

struct SpawnData {
	char spawnId[128];
	char enemyId[128];
	Vec2f direction;
	float initialCooldown;
	float cooldown;
	size_t enemyCount;
	SpawnerStartMode startMode;
};

inline SpawnData MakeSpawnData(const char *spawnId, const char *enemyId, const SpawnerStartMode startMode, const float initialCooldown, const float cooldown, const size_t enemyCount) {
	SpawnData result = {};
	fplCopyAnsiString(spawnId, result.spawnId, FPL_ARRAYCOUNT(result.spawnId));
	fplCopyAnsiString(enemyId, result.enemyId, FPL_ARRAYCOUNT(result.enemyId));
	result.initialCooldown = initialCooldown;
	result.cooldown = cooldown;
	result.enemyCount = enemyCount;
	result.startMode = startMode;
	return(result);
}

struct WaypointData {
	Vec2f direction;
};

enum class ObjectType {
	None = 0,
	Spawn,
	Waypoint,
	Goal,
};

inline const char *ObjectTypeToString(const ObjectType type) {
	switch(type) {
		case ObjectType::Spawn:
			return "Spawn";
		case ObjectType::Waypoint:
			return "Waypoint";
		case ObjectType::Goal:
			return "Goal";
		default:
			return "None";
	}
}

struct ObjectData {
	ObjectType type;
	Vec2i tilePos;
	union {
		SpawnData spawn;
		WaypointData waypoint;
	};
};

struct LevelData {
	uint32_t wayLayer[TotalTileCount];
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

struct CreepMultiplier {
	float hp;
	float bounty;
	float speed;
	float scale;
};
inline CreepMultiplier MakeCreepMultiplier(const float scale, const float speed, const float hp, const float bounty) {
	CreepMultiplier result = {};
	result.scale = scale;
	result.speed = speed;
	result.hp = hp;
	result.bounty = bounty;
	return(result);
}

struct CreepData {
	Vec4f color;
	const char *id;
	float renderRadius;
	float collisionRadius;
	float speed;
	int hp;
	int bounty;
	CreepStyle style;
};
inline CreepData MakeCreepData(const char *id, const float renderRadius, const float collisionRadius, const float speed, const int hp, const int bounty, const Vec4f &color, const CreepStyle style) {
	CreepData result = {};
	result.id = id;
	result.renderRadius = renderRadius;
	result.collisionRadius = collisionRadius;
	result.speed = speed;
	result.hp = hp;
	result.bounty = bounty;
	result.color = color;
	result.style = style;
	return(result);
}

struct Creep {
	const CreepData *data;
	uint64_t id;
	Vec2f prevPosition;
	Vec2f position;
	Vec2f facingDirection;
	Vec2f targetPos;
	const Waypoint *targetWaypoint;
	float speed;
	int hp;
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
	SpawnerStartMode startMode;
};

struct BulletData {
	float renderRadius;
	float collisionRadius;
	float speed;
	int damage;
};
inline BulletData MakeBulletData(const float renderRadius, const float collisionRadius, const float speed, const int damage) {
	BulletData result = {};
	result.renderRadius = renderRadius;
	result.collisionRadius = collisionRadius;
	result.speed = speed;
	result.damage = damage;
	return(result);
}

enum class FireRangeTestType {
	InSight = 0,
	LineTrace,
};

enum class EnemyLockTargetMode {
	Any,
	LockedOn
};

enum class EnemyPredictionFlags : int32_t {
	None = 0,
	WeaponCooldown = 1 << 0,
	BulletDistance = 1 << 1,
	All = WeaponCooldown | BulletDistance,
};
FPL_ENUM_AS_FLAGS_OPERATORS(EnemyPredictionFlags);

struct TowerData {
	BulletData bullet;
	const char *id;
	float structureRadius;
	float detectionRadius;
	float unlockRadius;
	float gunTubeLength;
	float gunTubeThickness;
	float gunCooldown;
	float gunRotationSpeed;
	FireRangeTestType enemyRangeTestType;
	EnemyPredictionFlags enemyPredictionFlags;
	EnemyLockTargetMode enemyLockOnMode;
	int costs;
};
inline TowerData MakeTowerData(const char *id, const float structureRadius, const float detectionRadius, const float unlockRadius, const float gunTubeLength, const float gunCooldown, const float gunTubeThickness, const float gunRotationSpeed, const int costs, const FireRangeTestType enemyRangeTestType, const EnemyPredictionFlags enemyPredictionFlags, const EnemyLockTargetMode enemyLockOnMode, const BulletData &bullet) {
	TowerData result = {};
	result.id = id;
	result.structureRadius = structureRadius;
	result.detectionRadius = detectionRadius;
	result.unlockRadius = unlockRadius;
	result.gunTubeLength = gunTubeLength;
	result.gunCooldown = gunCooldown;
	result.gunTubeThickness = gunTubeThickness;
	result.gunRotationSpeed = gunRotationSpeed;
	result.costs = costs;
	result.enemyRangeTestType = enemyRangeTestType;
	result.enemyPredictionFlags = enemyPredictionFlags;
	result.enemyLockOnMode = enemyLockOnMode;
	result.bullet = bullet;
	return(result);
}

struct WaveData {
	const char *levelId;
	CreepMultiplier enemyMultiplier;
	SpawnData spawners[16];
	size_t spawnerCount;
	float startupCooldown;
	int completionBounty;
};

template<size_t N>
inline WaveData MakeWaveData(const char *levelId, const float startupCooldown, const int completionBounty, const SpawnData(&spawners)[N]) {
	WaveData result = {};
	result.levelId = levelId;
	result.startupCooldown = startupCooldown;
	result.completionBounty = completionBounty;
	assert(N <= FPL_ARRAYCOUNT(result.spawners));
	result.spawnerCount = N;
	for(size_t i = 0; i < N; ++i) {
		result.spawners[i] = spawners[i];
	}
	return(result);
}

struct Tower {
	const TowerData *data;
	Vec2f position;
	Creep *targetEnemy;
	uint64_t targetId;
	float facingAngle;
	float gunTimer;
	bool hasTarget;
	bool canFire;
};

struct Bullet {
	const BulletData *data;
	Vec2f prevPosition;
	Vec2f position;
	Vec2f velocity;
	bool hasHit;
	bool isDestroyed;
};

enum class WaveState {
	Stopped = 0,
	Starting,
	Running,
	Won,
	Lost
};

struct FontAsset {
	LoadedFont desc;
	GLuint texture;
};

struct Assets {
	char dataPath[1024];
	FontAsset hudFont;
	FontAsset overlayFont;
	GLint radiantTexture;
};

struct Waypoints {
	Waypoint freeList[128];
	Waypoint *first;
	Waypoint *last;
	size_t used;
};

struct Creeps {
	uint64_t creepIdCounter;
	Creep list[1024];
	size_t count;
};

struct Bullets {
	Bullet list[10240];
	size_t count;
};

struct Towers {
	Tower list[256];
	size_t count;
	int selectedIndex;
};

struct CreepSpawners {
	CreepSpawner list[16];
	size_t count;
};

struct Level {
	LevelData data;
	Tile tiles[TotalTileCount];
	char activeId[256];
};

struct Wave {
	size_t totalEnemyCount;
	float warmupTimer;
	WaveState state;
	int activeIndex;
	bool isActive;
};

struct Stats {
	int money;
	int lifes;
};

struct GameState {
	GameMemory mem;

	Level level;
	Towers towers;
	Bullets bullets;
	Creeps enemies;
	Waypoints waypoints;
	CreepSpawners spawners;

	Assets assets;

	Wave wave;

	UIContext ui;

	Camera2D camera;
	Viewport viewport;
	Vec2f mouseWorldPos;
	Vec2i mouseTilePos;

	Stats stats;

	float deltaTime;
	float framesPerSecond;

	float slowdownTimer[2];
	float slowdownScale;
	bool isSlowDown;
	WaveState waveStateAfterSlowdown;

	bool isExiting;
	bool isDebugRendering;
};

inline Vec2f TileToWorld(const Vec2i &tilePos, const Vec2f &offset = V2f(0, 0)) {
	Vec2f result = {
		GridOriginX + tilePos.x * TileWidth,
		GridOriginY + (TileCountY - 1 - tilePos.y) * TileHeight
	};
	result += offset;
	return(result);
}

inline Vec2i WorldToTile(const Vec2f &worldPos) {
	float x = worldPos.x + WorldWidth * 0.5f;
	float y = worldPos.y + WorldHeight * 0.5f;
	Vec2i result = {
		(int)floorf((x) / TileWidth),
		TileCountY - 1 - (int)floorf((y - ControlsHeight) / TileHeight)
	};
	return(result);
}

inline bool IsValidTile(const Vec2i &tilePos) {
	bool result = !(tilePos.x < 0 || tilePos.x >(TileCountX - 1) || tilePos.y < 0 || tilePos.y >(TileCountY - 1));
	return(result);
}

#endif // FPL_TOWADEV_H