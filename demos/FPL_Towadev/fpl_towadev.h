#ifndef FPL_TOWADEV_H
#define FPL_TOWADEV_H

#include <final_math.h>
#include <final_geometry.h>

#include <final_fontloader.h>

#include <final_render.h>

#include <final_utils.h>

#include <final_assets.h>

constexpr float RadToDeg = 180.0f / (float)M_PI;

constexpr float GameAspect = 16.0f / 9.0f;
constexpr float WorldWidth = 20.0f;
constexpr float WorldHeight = WorldWidth / GameAspect;
constexpr float WorldRadiusW = WorldWidth * 0.5f;
constexpr float WorldRadiusH = WorldHeight * 0.5f;

constexpr float DefaultLineWidth = 2.0f;

constexpr int FieldTileCountX = 21;
constexpr int FieldTileCountY = 11;
constexpr float TileWidth = WorldWidth / (float)FieldTileCountX;
constexpr float TileHeight = WorldHeight / (float)(FieldTileCountY + 1);
const Vec2f TileExt = V2fInit(TileWidth, TileHeight) * 0.5f;
constexpr float MaxTileSize = fplMax(TileWidth, TileHeight);
constexpr float MaxTileRadius = MaxTileSize * 0.5f;

constexpr float ControlsWidth = WorldWidth;
constexpr float ControlsHeight = TileHeight;
constexpr float ControlsOriginX = -WorldRadiusW;
constexpr float ControlsOriginY = -WorldRadiusH;


const Vec4f TextBackColor = V4fInit(0.2f, 0.2f, 0.8f, 1);
const Vec4f TextForeColor = V4fInit(1, 1, 1, 1);

const char *TowersDataFilename = "towers.xml";
const char *WavesDataFilename = "waves.xml";
const char *CreepsDataFilename = "creeps.xml";

typedef const void *UIID;

struct FileInfo {
	size_t size;
	uint64_t modifyDate;
};

struct FileContents {
	FileInfo info;
	uint8_t *data;
};

struct UIInput {
	Vec2f userPosition;
	ButtonState leftButton;
};

struct UIContext {
	UIInput input;
	UIID hot;
	UIID active;
	struct GameState *gameState;
	struct RenderState *renderState;
	Mat4f viewProjection;
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
	fplCopyString(spawnId, result.spawnId, fplArrayCount(result.spawnId));
	fplCopyString(enemyId, result.enemyId, fplArrayCount(result.enemyId));
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

constexpr size_t MAX_LAYER_COUNT = 8;
struct LevelLayer {
	char name[64];
	float opacity;
	uint32_t *data;
	uint32_t mapWidth;
	uint32_t mapHeight;
};

struct TileImage {
	char source[256];
	uint32_t width;
	uint32_t height;
};

struct TilesetRange {
	int fromGid;
	int toGid;
};

constexpr size_t MAX_TILESET_COUNT = 8;
struct LevelTileset {
	TileImage image;
	char name[64];
	TilesetRange range;
	uint32_t tileWidth;
	uint32_t tileHeight;
	uint32_t tileCount;
	uint32_t columns;
	uint32_t firstGid;
	UVRect *tileUVs;
};

struct LevelData {
	LevelLayer layers[MAX_LAYER_COUNT];
	LevelTileset tilesets[MAX_TILESET_COUNT];
	ObjectData objects[256];
	size_t layerCount;
	size_t objectCount;
	size_t tilesetCount;
	size_t creepsFileSize;
	size_t towersFileSize;
	size_t wavesFileSize;
	uint32_t tileWidth;
	uint32_t tileHeight;
	uint32_t mapWidth;
	uint32_t mapHeight;
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
	char id[128];
	float renderRadius;
	float collisionRadius;
	float speed;
	int hp;
	int bounty;
};
inline CreepData MakeCreepData(const char *id, const float renderRadius, const float collisionRadius, const float speed, const int hp, const int bounty, const Vec4f &color) {
	CreepData result = {};
	fplCopyString(id, result.id, fplArrayCount(result.id));
	result.renderRadius = renderRadius;
	result.collisionRadius = collisionRadius;
	result.speed = speed;
	result.hp = hp;
	result.bounty = bounty;
	result.color = color;
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
	const CreepData *spawnTemplate;
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
	NoTest,
	InSight,
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

enum class PartType {
	None = 0,
	FillQuad,
	StrokeQuad,
	FillCircle,
	StrokeCircle,
	Line,
};

enum class PartRotationFlags : int32_t {
	None = 0,
	ApplyToOffset = 1 << 0,
	ApplyToTransform = 1 << 1,
};
FPL_ENUM_AS_FLAGS_OPERATORS(PartRotationFlags);

struct PartData {
	PartType type;
	PartRotationFlags rotFlags;

	Vec2f offset;
	float orientation;

	Vec4f color;
	Vec2f ext;
	float lineWidth;
	float radius;
};

struct WeaponTubeData {
	PartData parts[4];
	size_t partCount;
	float length;
	Vec2f offset;
};

struct TowerData {
	char id[64];
	BulletData bullet;
	WeaponTubeData tubes[4];
	size_t tubeCount;
	float detectionRadius;
	float unlockRadius;

	PartData parts[16];
	size_t partCount;

	float gunCooldown;
	float gunRotationSpeed;
	FireRangeTestType enemyRangeTestType;
	EnemyLockTargetMode enemyLockOnMode;
	int costs;
};

struct WaveData {
	char levelId[64];
	CreepMultiplier enemyMultiplier;
	SpawnData spawners[16];
	size_t spawnerCount;
	float startupCooldown;
	int completionBounty;
};

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

struct Assets {
	TowerData towerDefinitions[256];
	CreepData creepDefinitions[128];
	WaveData waveDefinitions[128];
	FontAsset hudFont;
	FontAsset overlayFont;
	char dataPath[1024];
	size_t towerDefinitionCount;
	size_t creepDefinitionCount;
	size_t waveDefinitionCount;
	FileInfo towersFileInfo;
	FileInfo wavesFileInfo;
	FileInfo creepsFileInfo;
	TextureAsset radiantTexture;
	TextureAsset entitiesTilesetTexture;
	TextureAsset wayTilesetTexture;
	TextureAsset groundTilesetTexture;
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
	Tower activeList[128];
	size_t activeCount;
	int selectedIndex;
};

struct CreepSpawners {
	CreepSpawner list[16];
	size_t count;
};

struct LevelDimension {
	size_t tileCountX;
	size_t tileCountY;
	float gridOriginX;
	float gridOriginY;
	float gridWidth;
	float gridHeight;
};

struct Level {
	fmemMemoryBlock levelMem;
	LevelData data;
	char activeId[256];
	LevelDimension dimension;
	Tile *tiles;
	size_t tileCapacity;
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
	Level level;
	Towers towers;
	Bullets bullets;
	Creeps enemies;
	Waypoints waypoints;
	CreepSpawners spawners;

	fmemMemoryBlock transientMem;

	Assets assets;

	Wave wave;

	UIContext ui;

	Camera2D camera;
	Mat4f viewProjection;
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

inline Vec2f TileToWorld(const LevelDimension &dim, const Vec2i &tilePos, const Vec2f &offset = V2fInit(0, 0)) {
	Vec2f result = {
		dim.gridOriginX + tilePos.x * TileWidth,
		dim.gridOriginY + (dim.tileCountY - 1 - tilePos.y) * TileHeight
	};
	result += offset;
	return(result);
}

inline Vec2i WorldToTile(const LevelDimension &dim, const Vec2f &worldPos) {
	float x = worldPos.x + WorldWidth * 0.5f;
	float y = worldPos.y + WorldHeight * 0.5f;
	Vec2i result = {
		(int)floorf((x) / TileWidth),
		(int)dim.tileCountY - 1 - (int)floorf((y - ControlsHeight) / TileHeight)
	};
	return(result);
}

inline bool IsValidTile(const LevelDimension &dim, const Vec2i &tilePos) {
	bool result = !(tilePos.x < 0 || tilePos.x >((int)dim.tileCountX - 1) || tilePos.y < 0 || tilePos.y >((int)dim.tileCountY - 1));
	return(result);
}

#endif // FPL_TOWADEV_H