/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev
Description:
	A tower defence clone
Requirements:
	- C++ Compiler
	- Final Dynamic OpenGL
Author:
	Torsten Spaete
Changelog:
	## 2018-06-05:
	- Initial creation
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_LOG_TO_DEBUGOUT
#include <final_platform_layer.h>

#include <math.h> // fabs

#include <string.h>
#include <stdlib.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#include <final_fontloader.h>

#define FXML_IMPLEMENTATION
#include <final_xml.h>

#include <final_arrayinitializer.h>

#include <final_game.h>

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
	const char *id;
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
	const char *level;
	CreepMultiplier enemyMultiplier;
	size_t enemyCount;
	float cooldown;
};

inline Wave MakeWave(const char *level, const CreepMultiplier &enemyMultiplier, const size_t enemyCount, const float cooldown) {
	Wave result = {};
	result.level = level;
	result.enemyMultiplier = enemyMultiplier;
	result.enemyCount = enemyCount;
	result.cooldown = cooldown;
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

static const TowerData TowerDefinitions[] = {
	MakeTowerData(
		/* id: */ "basic",
		/* structureRadius: */ TileSize * 0.35f,
		/* detectionRadius: */ TileSize * 2.0f,
		/* unlockRadius: */ TileSize * 2.25f,
		/* gunTubeLength: */ TileSize * 0.55f,
		/* gunCooldown: */ 0.3f,
		MakeBulletData(
		/* renderRadius: */ TileSize * 0.1f,
		/* collisionRadius: */ TileSize * 0.1f,
		/* speed: */ 6.0f,
		/* damage: */ 15
		)
	),
};

static const CreepData CreepDefinitions[] = {
	MakeCreepData(
		/* id: */ "first",
		/* renderRadius: */ TileSize * 0.25f,
		/* collisionRadius: */ TileSize * 0.2f,
		/* speed: */ 1.0,
		/* hp: */ 100,
		/* color: */ V4f(1, 1, 1, 1),
		/* style: */ CreepStyle::Quad
	),
};

static const Wave Waves[] = {
	MakeWave(
		/* level: */ "level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.0f, 1.0f, 1.0f),
		/* enemyCount: */ 10,
		/* cooldown: */ 2.0f
	),
	MakeWave(
		/* level: */ "level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.125f, 1.05f, 1.05f),
		/* enemyCount: */ 10,
		/* cooldown: */ 2.1f
	),
	MakeWave(
		/* level: */ "level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.25f, 1.1f, 1.1f),
		/* enemyCount: */ 10,
		/* cooldown: */ 2.0f
	),
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
	FontAsset hudFont;
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

namespace render {
	static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable, const bool isAlphaOnly = false) {
		GLuint handle;
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		GLuint internalFormat = isAlphaOnly ? GL_ALPHA8 : GL_RGBA8;
		GLenum format = isAlphaOnly ? GL_ALPHA : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, 0);

		return(handle);
	}

	static void DrawTile(const int x, const int y, const bool isFilled, const Vec4f &color) {
		Vec2f pos = TileToWorld(V2i(x, y));
		glColor4fv(&color.r);
		glBegin(isFilled ? GL_QUADS : GL_LINE_LOOP);
		glVertex2f(pos.x + TileSize, pos.y + TileSize);
		glVertex2f(pos.x, pos.y + TileSize);
		glVertex2f(pos.x, pos.y);
		glVertex2f(pos.x + TileSize, pos.y);
		glEnd();
	}

	static void DrawPoint(const Camera2D &camera, const float x, const float y, const float radius, const Vec4f &color) {
		glColor4fv(&color.r);
		glPointSize(radius * 2.0f * camera.worldToPixels);
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
		glPointSize(1);
	}

	static void DrawCircle(const float centerX, const float centerY, const float radius, const bool isFilled, const Vec4f &color, const int segments = 16) {
		float seg = ((float)M_PI * 2.0f) / (float)segments;
		glColor4fv(&color.r);
		glBegin(isFilled ? GL_POLYGON : GL_LINE_LOOP);
		for (int segmentIndex = 0; segmentIndex < segments; ++segmentIndex) {
			float x = centerX + cosf(segmentIndex * seg) * radius;
			float y = centerY + sinf(segmentIndex * seg) * radius;
			glVertex2f(x, y);
		}
		glEnd();
	}

	static void DrawNormal(const Vec2f &pos, const Vec2f &normal, const float length, const Vec4f &color) {
		glColor4fv(&color.r);
		glBegin(GL_LINES);
		glVertex2f(pos.x, pos.y);
		glVertex2f(pos.x + normal.x * length, pos.y + normal.y * length);
		glEnd();
	}
}

static void ReleaseGame(GameState &state) {
	fglUnloadOpenGL();
}

namespace tiles {
	inline Tile *GetTile(GameState &state, const Vec2i &tilePos) {
		if ((tilePos.x >= 0 && tilePos.x < TileCountX) && (tilePos.x >= 0 && tilePos.x < TileCountX)) {
			return &state.tiles[tilePos.y * TileCountX + tilePos.x];
		}
		return nullptr;
	}

	static Vec2i FindTilePosByEntityType(const GameState &state, const EntityType type) {
		for (int y = 0; y < TileCountY; ++y) {
			for (int x = 0; x < TileCountX; ++x) {
				int index = y * TileCountX + x;
				if (state.tiles[index].entityType == type) {
					return V2i(x, y);
				}
			}
		}
		return V2i(-1, -1);
	}
}

namespace waypoints {
	constexpr float WaypointDirectionWidth = 0.35f;
	static Waypoint *AddWaypoint(GameState &state, const Vec2i &tilePos, const Vec2f &dir) {
		Waypoint *waypoint = (Waypoint *)fplMemoryAllocate(sizeof(Waypoint));
		waypoint->tilePos = tilePos;
		waypoint->position = TileToWorld(tilePos, V2f(TileSize, TileSize) * 0.5f);
		waypoint->direction = dir;
		if (state.firstWaypoint == nullptr) {
			state.firstWaypoint = state.lastWaypoint = waypoint;
		} else {
			state.lastWaypoint->next = waypoint;
			state.lastWaypoint = waypoint;
		}
		return(waypoint);
	}
}

namespace towers {
	inline bool CanPlaceTower(GameState &state, const Vec2i &tilePos) {
		if ((state.selectedTowerIndex < 0) || !(state.selectedTowerIndex < FPL_ARRAYCOUNT(TowerDefinitions))) {
			return false;
		}
		if (state.towerCount == FPL_ARRAYCOUNT(state.towers)) {
			return false;
		}
		Tile *tile = tiles::GetTile(state, tilePos);
		if (tile == nullptr) {
			return false;
		}
		bool result = (!tile->isOccupied && (tile->entityType == EntityType::None) && (tile->wayType == WayType::None));
		return(result);
	}

	static Tower *PlaceTower(GameState &state, const Vec2i &tilePos) {
		const TowerData &data = TowerDefinitions[state.selectedTowerIndex];

		assert(state.towerCount < FPL_ARRAYCOUNT(state.towers));
		Tower *tower = &state.towers[state.towerCount++];
		FPL_CLEAR_STRUCT(tower);
		tower->data = data;
		tower->position = TileToWorld(tilePos, V2f(TileSize, TileSize) * 0.5f);
		tower->facingAngle = (float)M_PI * 0.5f; // Face north by default

		Tile *tile = tiles::GetTile(state, tilePos);
		assert(!tile->isOccupied);
		tile->isOccupied = true;

		return(tower);
	}

	static void ShootBullet(GameState &state, Tower &tower) {
		assert(state.bulletCount < FPL_ARRAYCOUNT(state.bullets));
		Bullet *bullet = &state.bullets[state.bulletCount++];
		FPL_CLEAR_STRUCT(bullet);
		Vec2f targetDir = V2f(cosf(tower.facingAngle), sinf(tower.facingAngle));
		bullet->position = bullet->prevPosition = tower.position + targetDir * tower.data.gunTubeLength;
		bullet->data = tower.data.bullet;
		bullet->velocity = targetDir * bullet->data.speed;
	}

	static void DrawTower(const Camera2D &camera, const TowerData &tower, const Vec2f &pos, const float angle) {
		// @TODO(final): Mulitple tower styles
		render::DrawPoint(camera, pos.x, pos.y, tower.structureRadius, V4f(1, 1, 0.5f, 1));

		glColor4f(1, 0.85f, 0.5f, 1);
		glLineWidth(3.0f);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * RadToDeg, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2f(tower.gunTubeLength, 0);
		glVertex2f(0, 0);
		glEnd();
		glLineWidth(2.0f);
		glPopMatrix();

		render::DrawCircle(pos.x, pos.y, tower.detectionRadius, false, V4f(1, 1, 1, 0.5f));
		render::DrawCircle(pos.x, pos.y, tower.unlockRadius, false, V4f(1, 0.5f, 1, 0.4f));
	}
}

namespace creeps {
	static void SpawnEnemy(GameState &state, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData &data) {
		assert(state.enemyCount < FPL_ARRAYCOUNT(state.enemies));
		Creep *enemy = &state.enemies[state.enemyCount++];
		FPL_CLEAR_STRUCT(enemy);
		enemy->data = data;
		enemy->position = enemy->prevPosition = spawnPos;
		enemy->speed = data.speed;
		enemy->hp = data.hp;
		if (state.firstWaypoint != nullptr) {
			enemy->targetWaypoint = state.firstWaypoint;
			enemy->targetPos = TileToWorld(state.firstWaypoint->tilePos, V2f(TileSize, TileSize) * 0.5f);
		} else {
			enemy->targetWaypoint = nullptr;
			enemy->targetPos = exitPos;
		}
		enemy->facingDirection = Vec2Normalize(enemy->targetPos - enemy->position);
		enemy->hasTarget = true;
	}

	static void UpdateSpawner(GameState &state, CreepSpawner &spawner, const float deltaTime) {
		if (spawner.isActive) {
			assert(spawner.remainingCount > 0);
			assert(spawner.spawnTimer > 0);
			spawner.spawnTimer -= deltaTime;
			if (spawner.spawnTimer <= 0) {
				SpawnEnemy(state, spawner.spawnPosition, spawner.exitPosition, spawner.spawnTemplate);
				--spawner.remainingCount;
				if (spawner.remainingCount == 0) {
					spawner.spawnTimer = 0;
					spawner.isActive = false;
				} else {
					spawner.spawnTimer = spawner.cooldown;
				}
			}
		}
	}

	static void SetupSpawner(CreepSpawner &spawner, const Vec2f &spawnPos, const Vec2f &exitPos, const float initialCooldown, const float cooldown, const size_t count, const CreepData &spawnTemplate) {
		spawner.spawnPosition = spawnPos;
		spawner.exitPosition = exitPos;
		spawner.cooldown = cooldown;
		spawner.spawnTimer = initialCooldown;
		spawner.totalCount = count;
		spawner.remainingCount = count;
		spawner.spawnTemplate = spawnTemplate;
		spawner.isActive = true;
	}

	static void CreepReachedExit(GameState &state, Creep &creep) {
		creep.isDead = true;
	}

	static void SetCreepNextTarget(GameState &state, Creep &creep) {
		Vec2i creepTilePos = WorldToTile(creep.position);
		if (creep.targetWaypoint != nullptr) {
			Waypoint waypoint = *creep.targetWaypoint;
			assert(Vec2Length(waypoint.direction) == 1);
			if (waypoint.next != nullptr) {
				creep.targetPos = TileToWorld(waypoint.next->tilePos, V2f(TileSize, TileSize) * 0.5f);
				creep.targetWaypoint = waypoint.next;
			} else {
				creep.targetWaypoint = nullptr;
				Vec2i goalTilePos = tiles::FindTilePosByEntityType(state, EntityType::Goal);
				assert(goalTilePos.x > -1 && goalTilePos.y > -1);
				creep.targetPos = TileToWorld(goalTilePos, V2f(TileSize, TileSize) * 0.5f);
			}
			creep.hasTarget = true;
			creep.facingDirection = Vec2Normalize(creep.targetPos - creep.position);
		} else {
			creep.hasTarget = false;
			CreepReachedExit(state, creep);
		}
	}

	static const CreepData *FindEnemyById(GameState &state, const char *id) {
		for (int i = 0; i < FPL_ARRAYCOUNT(CreepDefinitions); ++i) {
			if (strcmp(CreepDefinitions[i].id, id) == 0) {
				return &CreepDefinitions[i];
			}
		}
		return nullptr;
	}

	static void SetupWave(GameState &state, const int waveIndex) {
		const Wave &wave = Waves[waveIndex];
		state.waveIndex = waveIndex;
		state.activeEnemyCount = wave.enemyCount;

		// @TODO(final): Load level here

		Vec2i goalTilePos = tiles::FindTilePosByEntityType(state, EntityType::Goal);
		if (goalTilePos.x == -1) {
			return;
		}
		Vec2f goalPos = TileToWorld(goalTilePos, V2f(TileSize, TileSize) * 0.5f);

		for (size_t objectIndex = 0; objectIndex < state.levelData.objectCount; ++objectIndex) {
			ObjectData &obj = state.levelData.objects[objectIndex];
			switch (obj.type) {
				case ObjectType::Spawn:
				{
					const SpawnData &data = obj.spawn;
					Vec2i spawnTilePos = obj.tilePos;

					const CreepData *creepData = FindEnemyById(state, data.enemyId);
					assert(creepData != nullptr);

					assert(state.spawnerCount < FPL_ARRAYCOUNT(state.spawners));
					CreepSpawner &spawner = state.spawners[state.spawnerCount++];
					spawner = {};
					Vec2f spawnPos = TileToWorld(spawnTilePos, V2f(TileSize, TileSize) * 0.5f);
					CreepData enemyTemplate = *creepData;
					enemyTemplate.hp *= wave.enemyMultiplier.hp;
					enemyTemplate.speed *= wave.enemyMultiplier.speed;
					enemyTemplate.collisionRadius *= wave.enemyMultiplier.scale;
					SetupSpawner(spawner, spawnPos, goalPos, data.initialCooldown, data.cooldown, wave.enemyCount, enemyTemplate);
				} break;
			}
		}
	}

	static void EnemyHit(GameState &state, Creep &enemy, const Bullet &bullet) {
		enemy.hp -= bullet.data.damage;
		if (enemy.hp <= 0) {
			enemy.hp = 0;
			enemy.isDead = true;
		}
	}

	static void AllEnemiesKilled(GameState &state) {
		if (state.waveIndex < FPL_ARRAYCOUNT(Waves)) {
			SetupWave(state, state.waveIndex + 1);
		}
	}
}

static int StringToInt(const char *str, int def = 0) {
	int result = def;
	if (str != nullptr) {
		bool isNegative = false;
		const char *p = str;
		if (*p == '-') {
			isNegative = true;
			++p;
		}
		uint32_t value = 0;
		while (isdigit(*p)) {
			short v = *p - '0';
			value = value * 10 + v;
			++p;
		}
		if (isNegative) {
			result = -(int)value;
		} else {
			result = value;
		}
	}
	return(result);
}

static float StringToFloat(const char *str) {
	// @TODO(final): Implement this yourself!
	float result = (float)atof(str);
	return(result);
}

static void ParseLevelLayer(fxmlTag *childTag, const uint32_t mapWidth, const uint32_t mapHeight, uint32_t *outData) {
	fxmlTag *dataTag = fxmlFindTagByName(childTag, "data");
	if (dataTag != nullptr) {
		const char *encodingStr = fxmlGetAttributeValue(dataTag, "encoding");
		if (strcmp(encodingStr, "csv") == 0) {
			const char *p = dataTag->value;
			int index = 0;
			while (*p) {
				if (isdigit(*p)) {
					uint32_t tileValue = 0;
					while (isdigit(*p)) {
						short v = *p - '0';
						tileValue = tileValue * 10 + v;
						++p;
					}
					int row = index / (int)mapWidth;
					int col = index % (int)mapWidth;
					assert(row >= 0 && row < (int)mapHeight);
					assert(row >= 0 && row < TileCountY);
					assert(col >= 0 && col < TileCountX);
					int tileIndex = row * TileCountX + col;
					outData[tileIndex] = tileValue;
				} else if (*p == ',') {
					++p;
					++index;
				} else {
					++p;
				}
			}
		}
	}
}

static void ParseLevelObjects(fxmlTag *objectGroupTag, LevelData &level) {
	assert(level.tileWidth > 0);
	assert(level.tileHeight > 0);
	fxmlTag *childTag = objectGroupTag->firstChild;
	while (childTag != nullptr) {
		if (childTag->type == fxmlTagType_Element) {
			if (strcmp(childTag->name, "object") == 0) {
				int gid = StringToInt(fxmlGetAttributeValue(childTag, "gid"));
				int x = StringToInt(fxmlGetAttributeValue(childTag, "x"), -1);
				int y = StringToInt(fxmlGetAttributeValue(childTag, "y"), -1);
				int w = StringToInt(fxmlGetAttributeValue(childTag, "width"), 0);
				int h = StringToInt(fxmlGetAttributeValue(childTag, "height"), 0);
				float cx = (float)x + (float)w * 0.5f;
				float cy = (float)y - (float)h * 0.5f;
				int tileX = (int)floorf(cx / (float)level.tileWidth);
				int tileY = (int)floorf(cy / (float)level.tileHeight);
				Vec2i tilePos = V2i(tileX, tileY);

				uint32_t tileId = (gid > 0) ? ((gid - level.entitiesFirstGid) + 1) : 0;
				EntityType entityType = TilesetEntitiesToTypeMapping[tileId];

				ObjectData tmpObj = {};
				tmpObj.tilePos = tilePos;
				const char *typeName = fxmlGetAttributeValue(childTag, "type");
				if (strcmp(typeName, "Spawn") == 0) {
					tmpObj.type = ObjectType::Spawn;
					switch (entityType) {
						case EntityType::SpawnDown:
							tmpObj.spawn.direction = V2f(0.0f, -1.0f);
							break;
						case EntityType::SpawnUp:
							tmpObj.spawn.direction = V2f(0.0f, 1.0f);
							break;
						case EntityType::SpawnLeft:
							tmpObj.spawn.direction = V2f(-1.0f, 0.0f);
							break;
						case EntityType::SpawnRight:
							tmpObj.spawn.direction = V2f(1.0f, 0.0f);
							break;
						default:
							assert(!"Unsupported entity type for spawn!");
							break;
					}

					fxmlTag *propertiesTag = fxmlFindTagByName(childTag, "properties");
					if (propertiesTag != nullptr) {
						fxmlTag *propTag = propertiesTag->firstChild;
						while (propTag != nullptr) {
							const char *name = fxmlGetAttributeValue(propTag, "name");
							const char *value = fxmlGetAttributeValue(propTag, "value");
							if (strcmp(name, "enemy") == 0) {
								strcpy_s(tmpObj.spawn.enemyId, FPL_ARRAYCOUNT(tmpObj.spawn.enemyId), value);
							} else if (strcmp(name, "initialCooldown") == 0) {
								tmpObj.spawn.initialCooldown = StringToFloat(value);
							} else if (strcmp(name, "cooldown") == 0) {
								tmpObj.spawn.cooldown = StringToFloat(value);
							}
							propTag = propTag->next;
						}
					}

				}
				if (tmpObj.type != ObjectType::None) {
					assert(level.objectCount < FPL_ARRAYCOUNT(level.objects));
					ObjectData &obj = level.objects[level.objectCount++];
					obj = tmpObj;
				}
			}
		}
		childTag = childTag->next;
	}
}

static bool ParseLevel(fxmlTag *root, LevelData &level) {
	bool result = false;
	fxmlTag *mapTag = fxmlFindTagByName(root, "map");
	if (mapTag == nullptr) {
		return false;
	}
	int mapWidth = StringToInt(fxmlGetAttributeValue(mapTag, "width"));
	int mapHeight = StringToInt(fxmlGetAttributeValue(mapTag, "height"));
	if ((mapWidth != TileCountX) || (mapHeight != TileCountY)) {
		return false;
	}

	const char *orientation = fxmlGetAttributeValue(mapTag, "orientation");
	if (strcmp(orientation, "orthogonal") != 0) {
		return false;
	}

	int tileWidth = StringToInt(fxmlGetAttributeValue(mapTag, "tilewidth"));
	int tileHeight = StringToInt(fxmlGetAttributeValue(mapTag, "tileheight"));
	if ((tileWidth == 0) || (tileHeight == 0)) {
		return false;
	}

	level.tileWidth = tileWidth;
	level.tileHeight = tileHeight;

	fxmlTag *childTag = mapTag->firstChild;
	level.wayFirstGid = 0;
	level.entitiesFirstGid = 0;
	while (childTag) {
		if (childTag->type == fxmlTagType_Element) {
			if (strcmp(childTag->name, "tileset") == 0) {
				const char *tilesetName = fxmlGetAttributeValue(childTag, "name");
				int firstGid = StringToInt(fxmlGetAttributeValue(childTag, "firstgid"));
				if (strcmp(tilesetName, "way") == 0) {
					level.wayFirstGid = firstGid;
				} else if (strcmp(tilesetName, "entities") == 0) {
					level.entitiesFirstGid = firstGid;
				}
			} else if (strcmp(childTag->name, "layer") == 0) {
				const char *layerName = fxmlGetAttributeValue(childTag, "name");
				if (strcmp(layerName, "way") == 0) {
					ParseLevelLayer(childTag, mapWidth, mapHeight, level.wayLayer);
				} else if (strcmp(layerName, "entities") == 0) {
					ParseLevelLayer(childTag, mapWidth, mapHeight, level.entitiesLayer);
				}
			} else if (strcmp(childTag->name, "objectgroup") == 0) {
				const char *objectGroupName = fxmlGetAttributeValue(childTag, "name");
				if (strcmp(objectGroupName, "objects") == 0) {
					ParseLevelObjects(childTag, level);
				}
			}
		}
		childTag = childTag->next;
	}

	return(true);
}

static bool LoadLevel(const char *dataPath, const char *filename, GameState &state) {
	bool result = false;

	char filePath[1024];
	fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);

	fplFileHandle file;
	if (fplOpenAnsiBinaryFile(filePath, &file)) {
		uint32_t dataSize = fplGetFileSizeFromHandle32(&file);
		char *data = (char *)fplMemoryAllocate(dataSize);
		fplReadFileBlock32(&file, dataSize, data, dataSize);
		fxmlContext ctx = {};
		if (fxmlInitFromMemory(data, dataSize, &ctx)) {
			fxmlTag root = {};
			if (fxmlParse(&ctx, &root)) {
				LevelData &level = state.levelData;
				level = {};
				if (ParseLevel(&root, level)) {

					// @TODO(final): Move to SetupWave()

					// Tiles
					for (int y = 0; y < TileCountY; ++y) {
						for (int x = 0; x < TileCountX; ++x) {
							int tileIndex = y * TileCountX + x;
							uint32_t wayValue = state.levelData.wayLayer[tileIndex] > 0 ? ((level.wayLayer[tileIndex] - level.wayFirstGid) + 1) : 0;
							uint32_t entityValue = state.levelData.entitiesLayer[tileIndex] > 0 ? ((level.entitiesLayer[tileIndex] - level.entitiesFirstGid) + 1) : 0;
							Tile tile = {};
							tile.wayType = TilesetWayToTypeMapping[wayValue];
							tile.entityType = TilesetEntitiesToTypeMapping[entityValue];
							state.tiles[tileIndex] = tile;
						}
					}

					// Make waypoints
					for (int y = 0; y < TileCountY; ++y) {
						for (int x = 0; x < TileCountX; ++x) {
							int tileIndex = y * TileCountX + x;
							const Tile &tile = state.tiles[tileIndex];
							switch (tile.entityType) {
								case EntityType::WaypointRight:
								case EntityType::WaypointLeft:
								case EntityType::WaypointDown:
								case EntityType::WaypointUp:
								{
									Vec2f dir;
									if (tile.entityType == EntityType::WaypointRight) {
										dir = V2f(1, 0);
									} else if (tile.entityType == EntityType::WaypointLeft) {
										dir = V2f(-1, 0);
									} else if (tile.entityType == EntityType::WaypointDown) {
										dir = V2f(0, -1);
									} else {
										assert(tile.entityType == EntityType::WaypointUp);
										dir = V2f(0, 1);
									}
									waypoints::AddWaypoint(state, V2i(x, y), dir);
								} break;
							}
						}
					}

					result = true;
				}
			}
			fxmlFree(&ctx);
		}
		fplMemoryFree(data);
		fplCloseFile(&file);
	}

	return(result);
}

static void LoadAssets(GameState &state) {
	char dataPath[1024];
	fplGetExecutableFilePath(dataPath, FPL_ARRAYCOUNT(dataPath));
	fplExtractFilePath(dataPath, dataPath, FPL_ARRAYCOUNT(dataPath));
	fplPathCombine(dataPath, FPL_ARRAYCOUNT(dataPath), 2, dataPath, "data");

	LoadLevel(dataPath, "level1.tmx", state);
}

static bool InitGame(GameState &state, GameMemory &gameMemory) {
	if (!fglLoadOpenGL(true)) {
		return false;
	}

	state.mem = gameMemory;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	LoadAssets(state);

	state.camera.scale = 1.0f;
	state.camera.offset.x = 0;
	state.camera.offset.y = 0;

#if 0
	// Setup tiles
	int ys[] = { 1, TileCountY - 2 };
	for (int y = 0; y < FPL_ARRAYCOUNT(ys); ++y) {
		int row = ys[y];
		for (int x = 1; x < TileCountX - 1; ++x) {
			tiles::SetTile(state, x, row, TileType::EnemyPath);
		}
	}
	int xs[] = { 1, TileCountX - 2 };
	for (int x = 0; x < FPL_ARRAYCOUNT(xs); ++x) {
		int col = xs[x];
		for (int y = 1; y < TileCountY - 1; ++y) {
			tiles::SetTile(state, col, y, TileType::EnemyPath);
		}
	}
	tiles::SetTile(state, 0, 1, TileType::SpawnPoint);
	tiles::SetTile(state, 1, 3, TileType::Exit);
	tiles::SetTile(state, 1, 2, TileType::None);

	// Add waypoints
	waypoints::AddWaypoint(state, V2i(TileCountX - 2, 1), V2f(0, -1));
	waypoints::AddWaypoint(state, V2i(TileCountX - 2, TileCountY - 2), V2f(-1, 0));
	waypoints::AddWaypoint(state, V2i(1, TileCountY - 2), V2f(0, 1));
#endif

	// Setup initial wave and spawner
	creeps::SetupWave(state, 0);

	// Initial selected tower
	state.selectedTowerIndex = 0;

	return(true);
}

extern GameMemory GameCreate() {
	GameMemory result = {};
	result.capacity = sizeof(GameState) + FPL_MEGABYTES(16);
	result.base = fplMemoryAllocate(result.capacity);
	if (result.base == nullptr) {
		return {};
	}
	GameState *state = (GameState *)result.base;
	result.used = sizeof(GameState);
	if (!InitGame(*state, result)) {
		GameDestroy(result);
		state = nullptr;
	}
	return(result);
}

extern void GameDestroy(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	ReleaseGame(*state);
	state->~GameState();
	fplMemoryFree(state);
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input, bool isActive) {
	if (!isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;

	float scale = state->camera.scale;
	state->viewport = ComputeViewportByAspect(input.windowSize, GameAspect);
	state->camera.worldToPixels = (state->viewport.w / (float)WorldWidth) * scale;
	state->camera.pixelsToWorld = 1.0f / state->camera.worldToPixels;

	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;

	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	float deltaH = WorldHeight - GridHeight;
	state->mouseTilePos = WorldToTile(V2f(state->mouseWorldPos.x, state->mouseWorldPos.y - deltaH * 0.5f));
	if (WasPressed(input.mouse.left)) {
		if (towers::CanPlaceTower(*state, state->mouseTilePos)) {
			towers::PlaceTower(*state, state->mouseTilePos);
		}
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input, bool isActive) {
	if (!isActive) {
		return;
	}

	GameState *state = (GameState *)gameMemory.base;

	//
	// Move enemies
	//
	for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if (!enemy.isDead && enemy.hasTarget) {
			Vec2f distance = enemy.targetPos - enemy.position;
			Vec2f direction = Vec2Normalize(distance);
			float minRadius = TileSize * 0.05f;
			enemy.position += direction * enemy.speed * input.deltaTime;
			if (Vec2Dot(distance, distance) <= minRadius * minRadius) {
				creeps::SetCreepNextTarget(*state, enemy);
			}
		}
	}

	// Update spawners
	for (size_t spawnerIndex = 0; spawnerIndex < state->spawnerCount; ++spawnerIndex) {
		CreepSpawner &spawner = state->spawners[spawnerIndex];
		creeps::UpdateSpawner(*state, spawner, input.deltaTime);
	}

	//
	// Tower update:
	// Remove lock when out of detection
	// Lock on nearest enemy
	// Face weapon to locked on target
	// Fire to target
	//
	for (size_t towerIndex = 0; towerIndex < state->towerCount; ++towerIndex) {
		Tower &tower = state->towers[towerIndex];
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Vec2f distance = tower.targetEnemy->position - tower.position;
			assert(tower.data.unlockRadius >= tower.data.detectionRadius);
			if (tower.targetEnemy->isDead || Vec2Dot(distance, distance) > tower.data.unlockRadius * tower.data.unlockRadius) {
				tower.targetEnemy = nullptr;
				tower.hasTarget = false;
			}
		}
		if (!tower.hasTarget) {
			float nearestEnemyDistance = FLT_MAX;
			Creep *nearestEnemy = nullptr;
			for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
				Creep *testEnemy = &state->enemies[enemyIndex];
				float distance = Vec2Length(testEnemy->position - tower.position);
				if ((distance <= tower.data.detectionRadius) && (nearestEnemyDistance > distance)) {
					nearestEnemy = testEnemy;
					nearestEnemyDistance = distance;
				}
			}
			if (nearestEnemy != nullptr) {
				tower.targetEnemy = nearestEnemy;
				tower.hasTarget = true;
				tower.gunTimer = tower.data.gunCooldown;
			}
		}
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			// @TODO(final): Much better prediction scheme (Multiply delta time by some factor computed by: Tower cooldown, Bullet speed, Distance to target + Prediction change percentage modifier)
			Vec2f predictedPosition = tower.targetEnemy->position + tower.targetEnemy->facingDirection * tower.targetEnemy->speed * input.deltaTime;
			Vec2f directionToEnemy = Vec2Normalize(predictedPosition - tower.position);
			float angleToEnemy = atan2f(directionToEnemy.y, directionToEnemy.x);
			// @TODO(final): Slowly move facing angle towards enemy using a rotation speed
			tower.facingAngle = angleToEnemy;
			tower.gunTimer -= input.deltaTime;
			if (tower.gunTimer <= 0.0f) {
				tower.gunTimer = tower.data.gunCooldown;
				towers::ShootBullet(*state, tower);
			}
		}
	}

	//
	// Move and collide bullets
	//
	for (size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
		Bullet &bullet = state->bullets[bulletIndex];
		if (!bullet.isDestroyed) {
			bullet.position += bullet.velocity * input.deltaTime;
			for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
				Creep &enemy = state->enemies[enemyIndex];
				if (!enemy.isDead) {
					Vec2f distance = enemy.position - bullet.position;
					float bothRadi = bullet.data.collisionRadius + enemy.data.collisionRadius;
					float d = Vec2Dot(distance, distance);
					if (d < bothRadi) {
						bullet.isDestroyed = true;
						creeps::EnemyHit(*state, enemy, bullet);
						break;
					}
				}
			}
		}
	}

	//
	// Remove dead enemies and destroyed bullets
	//
	for (size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
		Bullet &bullet = state->bullets[bulletIndex];
		if (bullet.isDestroyed) {
			if (bulletIndex < state->bulletCount - 1) {
				const Bullet &lastBullet = state->bullets[state->bulletCount - 1];
				bullet = lastBullet;
			}
			--state->bulletCount;
		}
	}
	for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if (enemy.isDead) {
			if (enemyIndex < state->enemyCount - 1) {
				const Creep &lastEnemy = state->enemies[state->enemyCount - 1];
				enemy = lastEnemy;
			}
			--state->enemyCount;
			--state->activeEnemyCount;
		}
	}
	if (state->activeEnemyCount == 0) {
		creeps::AllEnemiesKilled(*state);
	}
}

extern void GameDraw(GameMemory &gameMemory, const float alpha) {
	GameState *state = (GameState *)gameMemory.base;
	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float invScale = 1.0f / state->camera.scale;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(state->camera.offset.x, state->camera.offset.y, 0);

#if 0
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	glVertex2f(WorldRadius.x, WorldRadius.y);
	glVertex2f(-WorldRadius.x, WorldRadius.y);
	glVertex2f(-WorldRadius.x, -WorldRadius.y);
	glVertex2f(WorldRadius.x, -WorldRadius.y);
	glEnd();
#endif

#if 0
	glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glVertex2f(GridOriginX + GridWidth, GridOriginY + GridHeight);
	glVertex2f(GridOriginX, GridOriginY + GridHeight);
	glVertex2f(GridOriginX, GridOriginY);
	glVertex2f(GridOriginX + GridWidth, GridOriginY);
	glEnd();
#endif

	//
	// Tiles
	//
	for (int y = 0; y < TileCountY; ++y) {
		for (int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->tiles[y * TileCountX + x];
			if (tile.wayType != WayType::None) {
				render::DrawTile(x, y, true, V4f(0.0f, 0.0f, 1.0f, 1.0f));
			}
			switch (tile.entityType) {
				case EntityType::Goal:
				{
					render::DrawTile(x, y, true, V4f(0.1f, 1.0f, 0.2f, 1.0f));
				} break;
			}
		}
	}

	// Spawners
	for (size_t spawnerIndex = 0; spawnerIndex < state->spawnerCount; ++spawnerIndex) {
		const CreepSpawner &spawner = state->spawners[spawnerIndex];
		Vec2i tilePos = WorldToTile(spawner.spawnPosition);
		render::DrawTile(tilePos.x, tilePos.y, true, V4f(0.0f, 1.0f, 1.0f, 1.0f));
	}


	//
	// Grid
	//
	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
	glBegin(GL_LINES);
	for (int y = 0; y <= TileCountY; ++y) {
		glVertex2f(GridOriginX, GridOriginY + y * TileSize);
		glVertex2f(GridOriginX + TileCountX * TileSize, GridOriginY + y * TileSize);
	}
	for (int x = 0; x <= TileCountX; ++x) {
		glVertex2f(GridOriginX + x * TileSize, GridOriginY);
		glVertex2f(GridOriginX + x * TileSize, GridOriginY + TileCountY * TileSize);
	}
	glEnd();

	//
	// Waypoints
	//
	for (Waypoint *waypoint = state->firstWaypoint; waypoint; waypoint = waypoint->next) {
		render::DrawPoint(state->camera, waypoint->position.x, waypoint->position.y, 0.25f, V4f(1, 0, 1, 1));
		render::DrawNormal(waypoint->position, waypoint->direction, waypoints::WaypointDirectionWidth, V4f(1, 1, 1, 1));
		if (waypoint->next == state->firstWaypoint) {
			break;
		}
	}

	// Hover tile
	Vec4f hoverColor;
	if (towers::CanPlaceTower(*state, state->mouseTilePos))
		hoverColor = V4f(0.1f, 1.0f, 0.1f, 1.0f);
	else
		hoverColor = V4f(1.0f, 0.1f, 0.1f, 1.0f);
	render::DrawTile(state->mouseTilePos.x, state->mouseTilePos.y, false, hoverColor);

	//
	// Enemies
	//
	for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if (!enemy.isDead) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);
			switch (enemy.data.style) {
				case CreepStyle::Triangle:
				{
					float angle = atan2f(enemy.facingDirection.y, enemy.facingDirection.x);
					float r = enemy.data.renderRadius;
					glColor4fv(&enemy.data.color.m[0]);
					glPushMatrix();
					glTranslatef(enemyPos.x, enemyPos.y, 0);
					glRotatef(angle * RadToDeg, 0, 0, 1);
					glBegin(GL_POLYGON);
					glVertex2f(+r, 0);
					glVertex2f(-r, +r);
					glVertex2f(-r, -r);
					glEnd();
					glPopMatrix();
				} break;

				case CreepStyle::Quad:
				{
					float r = enemy.data.renderRadius;
					render::DrawPoint(state->camera, enemyPos.x, enemyPos.y, r, enemy.data.color);
				} break;
			}
			enemy.prevPosition = enemy.position;
		}
	}

	//
	// Towers
	//
	for (size_t towerIndex = 0; towerIndex < state->towerCount; ++towerIndex) {
		const Tower &tower = state->towers[towerIndex];
		towers::DrawTower(state->camera, tower.data, tower.position, tower.facingAngle);
	}

	//
	// Bullets
	//
	for (size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
		Bullet &bullet = state->bullets[bulletIndex];
		if (!bullet.isDestroyed) {
			Vec2f bulletPos = Vec2Lerp(bullet.prevPosition, alpha, bullet.position);
			render::DrawPoint(state->camera, bulletPos.x, bulletPos.y, bullet.data.renderRadius, V4f(1, 0, 0, 1));
			bullet.prevPosition = bullet.position;
		}
	}
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = "FPL Demo | Towadev";
	int result = GameMain(config);
	return(result);
}