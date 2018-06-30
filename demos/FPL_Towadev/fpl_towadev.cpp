/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev

Description:
	A tower defence clone.
	Levels are loaded from .TMX files (Tiled-Editor).
	Waves & Enemies are defined as constant arrays.
	Written in C++ (C-Style).

Requirements:
	- C++ Compiler
	- Final Dynamic OpenGL
	- Final XML
	- Final Framework

Author:
	Torsten Spaete

Changelog:
	## 2018-06-25:
	- Introduced Tower Buttons
	- Write selected tower name

	## 2018-06-20:
	- Refactoring
	- WaveData have now a list of SpawnerData, so we can have multiple & different spawners for each wave.
	- Fixed overlay font atlas was too small

	## 2018-06-19:
	- Started a very basic immediate mode UI system
	- Prepare for command buffer rendering
	- Draw tower preview on mouse tile
	- Bugfixes

	## 2018-06-18:
	- Simplified tower rotation
	- Added prediction flags & enemy range test type
	- Changed enemy detection using new flags and range test

	## 2018-06-16:
	- Changed enemy detection and fire on non-targets as well
	- Made tower rotation really bad

	## 2018-06-15:
	- Improved enemy position prediction for towers
	- Improved gun rotation
	- Cooldown only after tower has fired

	## 2018-06-14:
	- Improved HUD rendering
	- Added background for controls/UI rendering
	- Added simple gun rotation

	## 2018-06-11:
	- Heavy refactoring
	- Small bugfixes
	- Render enemy hp as a colored progressbar
	- Render wave cooldown timer
	- Introduced wave state

	## 2018-06-10:
	- Removed entity tile layer from TMX map
	- Waypoints / Goal are now loaded from objects
	- Introduced money and bounty
	- Show current money in HUD

	## 2018-06-09:
	- Tons of bugfixes (Waypoints, Target detection, etc.)
	- Added lots of new properties
	- Added simple HUD showing wave & lifes
	- Improved enemy spawner to support multiple spawners per wave

	## 2018-06-08:
	- Improved enemy target
	- Removed fixed towers
	- Added tower placement using mouse
	- Removed fixed tilemap
	- Added basic TMX parsing support
	- Tiles are now loaded from TMX layers

	## 2018-06-07:
	- Added enemy spawner
	- Added fixed towers with instant lock-on
	- Added bullet shoot from towers with a cooldown

	## 2018-06-06:
	- Improved enemy movement

	## 2018-06-05:
	- Initial creation

Todo:
	- Tower Selecting
		- Single Select
		- Multiple Select (From Mouse-Area)
	- Sell Tower + Button
	- Upgrade Tower + Button
	- Proper Drawing (Layers / Primitives / Barrels)
		- Towers
		- Enemies
	- Move all const arrays out into XML files
	- Manual reload of XMLs and update all data dynamically
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#define FINAL_FONTLOADER_BETTERQUALITY 1
#include <final_fontloader.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_OPENGL_RENDER_IMPLEMENTATION
#include <final_opengl_render.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define FXML_IMPLEMENTATION
#include <final_xml.h>

#include <final_arrayinitializer.h>

#include <final_game.h>

#include "fpl_towadev.h"

constexpr float ShotAngleTolerance = (Pi32 * 0.1f) * 0.5f;

static const TowerData TowerDefinitions[] = {
	MakeTowerData(
		/* id: */ "First Tower",
		/* structureRadius: */ MaxTileSize * 0.35f,
		/* detectionRadius: */ MaxTileSize * 2.25f,
		/* unlockRadius: */ MaxTileSize * 2.3f,
		/* gunTubeLength: */ MaxTileSize * 0.55f,
		/* gunCooldown: */ 0.35f,
		/* gunTubeThickness: */ MaxTileSize * 0.2f,
		/* gunRotationSpeed: */ 4.0f,
		/* costs: */ 50,
		/* enemyRangeTestType: */ FireRangeTestType::InSight,
		/* enemyPredictionFlags: */ EnemyPredictionFlags::All,
		/* enemyLockOnMode: */ EnemyLockTargetMode::LockedOn,
		MakeBulletData(
		/* renderRadius: */ MaxTileSize * 0.05f,
		/* collisionRadius: */ MaxTileSize * 0.05f,
		/* speed: */ 2.5f,
		/* damage: */ 15
	)
),
MakeTowerData(
	/* id: */ "Second Tower",
	/* structureRadius: */ MaxTileSize * 0.35f,
	/* detectionRadius: */ MaxTileSize * 2.15f,
	/* unlockRadius: */ MaxTileSize * 2.2f,
	/* gunTubeLength: */ MaxTileSize * 0.4f,
	/* gunCooldown: */ 0.2f,
	/* gunTubeThickness: */ MaxTileSize * 0.15f,
	/* gunRotationSpeed: */ 6.0f,
	/* costs: */ 100,
	/* enemyRangeTestType: */ FireRangeTestType::InSight,
	/* enemyPredictionFlags: */ EnemyPredictionFlags::All,
	/* enemyLockOnMode: */ EnemyLockTargetMode::LockedOn,
	MakeBulletData(
	/* renderRadius: */ MaxTileSize * 0.04f,
	/* collisionRadius: */ MaxTileSize * 0.04f,
	/* speed: */ 3.5f,
	/* damage: */ 8
)
),
};

static const CreepData CreepDefinitions[] = {
	MakeCreepData(
		/* id: */ "The Quad",
		/* renderRadius: */ MaxTileSize * 0.25f,
		/* collisionRadius: */ MaxTileSize * 0.2f,
		/* speed: */ 1.0,
		/* hp: */ 100,
		/* bounty: */ 1,
		/* color: */ V4f(1, 1, 1, 1),
		/* style: */ CreepStyle::Quad
	),
};

static const WaveData WaveDefinitions[] = {
	MakeWaveData(
		/* level: */ "level1",
		/* startupCooldown: */ 3.0f,
		/* completionBounty: */ 20,
		/* spawners: */
		{
			MakeSpawnData("spawn1", "The Quad", SpawnerStartMode::Fixed, 0, 1.5f, 25),
			MakeSpawnData("spawn1", "The Quad", SpawnerStartMode::AfterTheLast, 0, 1.0f, 10),
		}
	),
};

namespace gamelog {
	enum class LogLevel {
		Fatal = 0,
		Error,
		Warning,
		Info,
		Verbose
	};

	static void Write(const LogLevel level, const char *format, va_list argList) {
		char msg[1024];
		fplFormatAnsiStringArgs(msg, FPL_ARRAYCOUNT(msg), format, argList);
		if (level == LogLevel::Fatal)
			fplDebugOut("Fatal: ");
		else if (level == LogLevel::Error)
			fplDebugOut("Error: ");
		else if (level == LogLevel::Warning)
			fplDebugOut("Warning: ");
		fplDebugFormatOut("%s\n", msg);
	}

	static void Info(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Info, format, argList);
		va_end(argList);
	}

	static void Verbose(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Verbose, format, argList);
		va_end(argList);
	}

	static void Warning(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Warning, format, argList);
		va_end(argList);
	}

	static void Error(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Error, format, argList);
		va_end(argList);
	}

	static void Fatal(const char *format, ...) {
		va_list argList;
		va_start(argList, format);
		Write(LogLevel::Fatal, format, argList);
		va_end(argList);
	}
}

// Forward declarations
namespace level {
	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type);
	static void LoadWave(GameState &state, const int waveIndex);
}
namespace game {
	static void NewGame(GameState &state);
	static void SetSlowdown(GameState &state, const float duration, const WaveState nextState);
}

namespace ui {
	static void UIBegin(UIContext &ctx, GameState *gameState, const Input &input, const Vec2f &mousePos) {
		ctx.input = {};
		ctx.hot = 0;
		ctx.gameState = gameState;
		ctx.input.userPosition = mousePos;
		ctx.input.leftButton = input.mouse.left;
	}

	inline bool UIIsHot(const UIContext &ctx) {
		bool result = ctx.hot > 0;
		return(result);
	}

	inline bool UIIsActive(const UIContext &ctx) {
		bool result = ctx.active > 0;
		return(result);
	}

	inline Vec2f GetUIButtonExt(const Vec2f &radius) {
		Vec2f result = radius;
		return(result);
	}

	inline bool IsInsideButton(UIContext &ctx, const Vec2f &pos, const Vec2f &radius) {
		bool result = Abs(ctx.input.userPosition.x - pos.x) <= radius.w && Abs(ctx.input.userPosition.y - pos.y) <= radius.h;
		return(result);
	}

	enum class UIButtonState {
		None = 0,
		Hover,
		Down,
	};
	typedef void(UIButtonDrawFunction)(GameState &gameState, const Vec2f &pos, const Vec2f &radius, const UIButtonState buttonState, void *userData);

	static bool UIButton(UIContext &ctx, const UIID &id, const Vec2f &pos, const Vec2f &radius, UIButtonDrawFunction *drawFunc, void *userData) {
		bool result = false;
		if (IsInsideButton(ctx, pos, radius)) {
			ctx.hot = id;
		}
		if (ctx.active == id) {
			if (WasPressed(ctx.input.leftButton)) {
				if (ctx.hot == id) {
					result = true;
				}
				ctx.active = 0;
			}
		} else if (ctx.hot == id) {
			if (ctx.input.leftButton.state == fplButtonState_Press) {
				ctx.active = id;
			}
		}

		UIButtonState buttonState = UIButtonState::None;
		if (ctx.hot == id) {
			if (ctx.active == ctx.hot) {
				buttonState = UIButtonState::Down;
			} else {
				buttonState = UIButtonState::Hover;
			}
		}

		drawFunc(*ctx.gameState, pos, radius, buttonState, userData);

		return(result);
	}
}

namespace utils {
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
}

namespace render {
	static void DrawTile(const int x, const int y, const bool isFilled, const Vec4f &color) {
		Vec2f pos = TileToWorld(V2i(x, y));
		glColor4fv(&color.r);
		glBegin(isFilled ? GL_QUADS : GL_LINE_LOOP);
		glVertex2f(pos.x + TileWidth, pos.y + TileHeight);
		glVertex2f(pos.x, pos.y + TileHeight);
		glVertex2f(pos.x, pos.y);
		glVertex2f(pos.x + TileWidth, pos.y);
		glEnd();
	}

	static void DrawLineStipple(const Vec2f &a, const Vec2f &b, const float stippleWidth, const int modCount) {
		// @NOTE(final): Expect line width and color to be set already
		assert(stippleWidth > 0);
		Vec2f ab = b - a;
		float d = Vec2Length(ab);
		Vec2f n = ab / d;
		int secCount = (d > stippleWidth) ? (int)(d / stippleWidth) : 1;
		assert(secCount > 0);
		glBegin(GL_LINES);
		for (int sec = 0; sec < secCount; ++sec) {
			float t = sec / (float)secCount;
			Vec2f start = Vec2Lerp(a, t, b);
			Vec2f end = start + n * stippleWidth;
			if (sec % modCount == 0) {
				glVertex2f(start.x, start.y);
				glVertex2f(end.x, end.y);
			}
		}
		glEnd();
	}

	static void DrawLineLoopStipple(const Vec2f *points, const size_t pointCount, const float stippleWidth, const int modCount) {
		assert(pointCount >= 2);
		// @NOTE(final): Expect line width and color to be set already
		for (size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
			Vec2f a = points[pointIndex];
			Vec2f b = points[(pointIndex + 1) % pointCount];
			DrawLineStipple(a, b, stippleWidth, modCount);
		}
	}
}

namespace creeps {
	static void SpawnEnemy(Creeps &enemies, const Waypoints &waypoints, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData *data) {
		assert(enemies.count < FPL_ARRAYCOUNT(enemies.list));
		Creep *enemy = &enemies.list[enemies.count++];
		FPL_CLEAR_STRUCT(enemy);
		enemy->id = ++enemies.creepIdCounter;
		enemy->data = data;
		enemy->position = enemy->prevPosition = spawnPos;
		enemy->speed = data->speed;
		enemy->hp = data->hp;
		if (waypoints.first != nullptr) {
			enemy->targetWaypoint = waypoints.first;
			enemy->targetPos = TileToWorld(waypoints.first->tilePos, TileExt);
		} else {
			enemy->targetWaypoint = nullptr;
			enemy->targetPos = exitPos;
		}
		enemy->facingDirection = Vec2Normalize(enemy->targetPos - enemy->position);
		enemy->hasTarget = true;
	}

	static void UpdateSpawner(Creeps &enemies, CreepSpawner &spawner, const Waypoints &waypoints, const float deltaTime) {
		if (spawner.isActive) {
			assert(spawner.remainingCount > 0);
			assert(spawner.cooldown > 0);
			if (spawner.spawnTimer > 0) {
				spawner.spawnTimer -= deltaTime;
			}
			if (spawner.spawnTimer <= 0) {
				SpawnEnemy(enemies, waypoints, spawner.spawnPosition, spawner.exitPosition, spawner.spawnTemplate);
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

	static void AddSpawner(CreepSpawners &spawners, const Vec2i &spawnTilePos, const Vec2i &goalTilePos, const float initialCooldown, const float cooldown, const size_t count, const SpawnerStartMode startMode, const CreepData *spawnTemplate) {
		assert(spawners.count < FPL_ARRAYCOUNT(spawners.list));
		CreepSpawner &spawner = spawners.list[spawners.count++];
		spawner = {};
		spawner.spawnPosition = TileToWorld(spawnTilePos, TileExt);
		spawner.exitPosition = TileToWorld(goalTilePos, TileExt);
		spawner.cooldown = cooldown;
		spawner.spawnTimer = initialCooldown;
		spawner.totalCount = count;
		spawner.remainingCount = count;
		spawner.spawnTemplate = spawnTemplate;
		spawner.isActive = (startMode == SpawnerStartMode::Fixed);
		spawner.startMode = startMode;
	}

	static void CreepDead(GameState &state, Creep &enemy) {
		enemy.id = 0;
		enemy.hasTarget = false;
		enemy.targetWaypoint = nullptr;
		enemy.isDead = true;
		enemy.hp = 0;
	}

	static void CreepReachedExit(GameState &state, Creep &enemy) {
		CreepDead(state, enemy);
		state.stats.lifes--;
		if (state.wave.isActive && state.stats.lifes <= 0) {
			state.stats.lifes = 0;
			state.wave.isActive = false;
			game::SetSlowdown(state, 2.0f, WaveState::Lost);
		}
	}

	static void SetCreepNextTarget(GameState &state, Creep &enemy) {
		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		assert(goalTilePos.x > -1 && goalTilePos.y > -1);
		Vec2i creepTilePos = WorldToTile(enemy.position);
		if (enemy.targetWaypoint != nullptr) {
			const Waypoint waypoint = *enemy.targetWaypoint;
			assert(Vec2Length(waypoint.direction) == 1);
			Vec2f creepDir = waypoint.direction;
			if (waypoint.next != nullptr) {
				enemy.targetPos = TileToWorld(waypoint.next->tilePos, TileExt);
				enemy.targetWaypoint = waypoint.next;
			} else {
				enemy.targetWaypoint = nullptr;
				enemy.targetPos = TileToWorld(goalTilePos, TileExt);
			}
			enemy.hasTarget = true;
			enemy.facingDirection = Vec2Normalize(enemy.targetPos - enemy.position);
		} else {
			enemy.hasTarget = false;
			assert(IsVec2Equals(creepTilePos, goalTilePos));
			CreepReachedExit(state, enemy);
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

	static void CreepHit(GameState &state, Creep &enemy, const Bullet &bullet) {
		enemy.hp -= bullet.data->damage;
		if (enemy.hp <= 0) {
			CreepDead(state, enemy);
			state.stats.money += enemy.data->bounty;
		}
	}

	static void AllEnemiesKilled(GameState &state) {
		state.wave.state = WaveState::Won;
		state.stats.money += WaveDefinitions[state.wave.activeIndex].completionBounty;
		if (state.wave.activeIndex < (FPL_ARRAYCOUNT(WaveDefinitions) - 1)) {
			level::LoadWave(state, state.wave.activeIndex + 1);
		}
	}
}

namespace level {
	inline Tile *GetTile(Level &level, const Vec2i &tilePos) {
		if (IsValidTile(tilePos)) {
			return &level.tiles[tilePos.y * TileCountX + tilePos.x];
		}
		return nullptr;
	}

	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type) {
		for (int y = 0; y < TileCountY; ++y) {
			for (int x = 0; x < TileCountX; ++x) {
				int index = y * TileCountX + x;
				if (level.tiles[index].entityType == type) {
					return V2i(x, y);
				}
			}
		}
		return V2i(-1, -1);
	}

	constexpr float WaypointDirectionWidth = 0.35f;

	static void ClearWaypoints(Waypoints &waypoints) {
		waypoints.first = waypoints.last = nullptr;
		waypoints.used = 0;
	}

	static Waypoint *AddWaypoint(Waypoints &waypoints, const Vec2i &tilePos, const Vec2f &dir) {
		assert(waypoints.used < FPL_ARRAYCOUNT(waypoints.freeList));
		Waypoint *waypoint = &waypoints.freeList[waypoints.used++];
		waypoint->tilePos = tilePos;
		waypoint->position = TileToWorld(tilePos, TileExt);
		waypoint->direction = dir;
		if (waypoints.first == nullptr) {
			waypoints.first = waypoints.last = waypoint;
		} else {
			waypoints.last->next = waypoint;
			waypoints.last = waypoint;
		}
		return(waypoint);
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
					int gid = utils::StringToInt(fxmlGetAttributeValue(childTag, "gid"));
					int x = utils::StringToInt(fxmlGetAttributeValue(childTag, "x"), -1);
					int y = utils::StringToInt(fxmlGetAttributeValue(childTag, "y"), -1);
					int w = utils::StringToInt(fxmlGetAttributeValue(childTag, "width"), 0);
					int h = utils::StringToInt(fxmlGetAttributeValue(childTag, "height"), 0);
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
					const char *objName = fxmlGetAttributeValue(childTag, "name");
					if (strcmp(typeName, "Spawn") == 0) {
						tmpObj.type = ObjectType::Spawn;
						fplCopyAnsiString(objName, tmpObj.spawn.spawnId, FPL_ARRAYCOUNT(tmpObj.spawn.spawnId));
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
								propTag = propTag->nextSibling;
							}
						}
					} else if (strcmp(typeName, "Waypoint") == 0) {
						tmpObj.type = ObjectType::Waypoint;
						switch (entityType) {
							case EntityType::WaypointDown:
								tmpObj.waypoint.direction = V2f(0.0f, -1.0f);
								break;
							case EntityType::WaypointUp:
								tmpObj.waypoint.direction = V2f(0.0f, 1.0f);
								break;
							case EntityType::WaypointLeft:
								tmpObj.waypoint.direction = V2f(-1.0f, 0.0f);
								break;
							case EntityType::WaypointRight:
								tmpObj.waypoint.direction = V2f(1.0f, 0.0f);
								break;
							default:
								assert(!"Unsupported entity type for waypoint!");
								break;
						}
					} else if (strcmp(typeName, "Goal") == 0) {
						tmpObj.type = ObjectType::Goal;
					}

					if (tmpObj.type != ObjectType::None) {
						assert(level.objectCount < FPL_ARRAYCOUNT(level.objects));
						ObjectData &obj = level.objects[level.objectCount++];
						obj = tmpObj;
					}
				}
			}
			childTag = childTag->nextSibling;
		}
	}

	static bool ParseLevel(fxmlTag *root, LevelData &level) {
		bool result = false;
		fxmlTag *mapTag = fxmlFindTagByName(root, "map");
		if (mapTag == nullptr) {
			return false;
		}
		int mapWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "width"));
		int mapHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "height"));
		if ((mapWidth > TileCountX) || (mapHeight > TileCountY)) {
			return false;
		}

		const char *orientation = fxmlGetAttributeValue(mapTag, "orientation");
		if (strcmp(orientation, "orthogonal") != 0) {
			return false;
		}

		int tileWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tilewidth"));
		int tileHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tileheight"));
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
					int firstGid = utils::StringToInt(fxmlGetAttributeValue(childTag, "firstgid"));
					if (strcmp(tilesetName, "way") == 0) {
						level.wayFirstGid = firstGid;
					} else if (strcmp(tilesetName, "entities") == 0) {
						level.entitiesFirstGid = firstGid;
					}
				} else if (strcmp(childTag->name, "layer") == 0) {
					const char *layerName = fxmlGetAttributeValue(childTag, "name");
					if (strcmp(layerName, "way") == 0) {
						ParseLevelLayer(childTag, mapWidth, mapHeight, level.wayLayer);
					}
				} else if (strcmp(childTag->name, "objectgroup") == 0) {
					const char *objectGroupName = fxmlGetAttributeValue(childTag, "name");
					if (strcmp(objectGroupName, "objects") == 0) {
						ParseLevelObjects(childTag, level);
					}
				}
			}
			childTag = childTag->nextSibling;
		}

		return(true);
	}

	static bool LoadLevel(GameState &state, const char *dataPath, const char *filename, LevelData &outLevel) {
		bool result = false;

		char filePath[1024];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 3, dataPath, "levels", filename);

		gamelog::Verbose("Loading level '%s'", filePath);

		fplFileHandle file;
		if (fplOpenAnsiBinaryFile(filePath, &file)) {
			uint32_t dataSize = fplGetFileSizeFromHandle32(&file);
			char *data = (char *)fplMemoryAllocate(dataSize);
			fplReadFileBlock32(&file, dataSize, data, dataSize);
			fxmlContext ctx = {};
			if (fxmlInitFromMemory(data, dataSize, &ctx)) {
				fxmlTag root = {};
				if (fxmlParse(&ctx, &root)) {
					outLevel = {};
					if (ParseLevel(&root, outLevel)) {
						// Tiles
						for (int y = 0; y < TileCountY; ++y) {
							for (int x = 0; x < TileCountX; ++x) {
								int tileIndex = y * TileCountX + x;
								uint32_t wayValue = outLevel.wayLayer[tileIndex] > 0 ? ((outLevel.wayLayer[tileIndex] - outLevel.wayFirstGid) + 1) : 0;
								Tile tile = {};
								tile.wayType = TilesetWayToTypeMapping[wayValue];
								tile.entityType = EntityType::None;
								state.level.tiles[tileIndex] = tile;
							}
						}

						// Make waypoints/goal
						for (size_t objIndex = 0; objIndex < outLevel.objectCount; ++objIndex) {
							const ObjectData &obj = outLevel.objects[objIndex];
							if (IsValidTile(obj.tilePos)) {
								int tileIndex = obj.tilePos.y * TileCountX + obj.tilePos.x;
								switch (obj.type) {
									case ObjectType::Goal:
										state.level.tiles[tileIndex].entityType = EntityType::Goal;
										break;
									case ObjectType::Waypoint:
										AddWaypoint(state.waypoints, obj.tilePos, obj.waypoint.direction);
										break;
									default:
										break;
								}
							}
						}

						result = true;
					} else {
						gamelog::Error("Level file '%s' is not valid!", filePath);
					}
				} else {
					gamelog::Error("Level file '%s' is not a valid XML file!", filePath);
				}
				fxmlFree(&ctx);
			}
			fplMemoryFree(data);
			fplCloseFile(&file);
		} else {
			gamelog::Error("Level file '%s' could not be found!", filePath);
		}

		return(result);
	}

	static void ClearWave(GameState &state) {
		gamelog::Verbose("Clear Wave");
		state.wave.totalEnemyCount = 0;
		state.wave.isActive = false;
		state.enemies.count = 0;
		state.spawners.count = 0;
		for (size_t towerIndex = 0; towerIndex < state.towers.count; ++towerIndex) {
			Tower &tower = state.towers.list[towerIndex];
			tower.hasTarget = false;
			tower.targetEnemy = nullptr;
			tower.targetId = 0;
		}
	}

	static void ClearLevel(GameState &state) {
		gamelog::Verbose("Clear Level");
		state.towers.count = 0;
		state.towers.selectedIndex = -1;
		ClearWave(state);
		ClearWaypoints(state.waypoints);
	}

	static const ObjectData *FindSpawnObjectById(const Level &level, const char *spawnId) {
		for (size_t objectIndex = 0; objectIndex < level.data.objectCount; ++objectIndex) {
			const ObjectData *obj = &level.data.objects[objectIndex];
			if (obj->type == ObjectType::Spawn) {
				if (fplIsStringEqual(obj->spawn.spawnId, spawnId)) {
					return(obj);
				}
			}
		}
		return(nullptr);
	}

	static void LoadWave(GameState &state, const int waveIndex) {
		const WaveData &wave = WaveDefinitions[waveIndex];

		state.wave.state = WaveState::Stopped;

		gamelog::Verbose("Setup wave '%d'", waveIndex);

		if (state.level.activeId == nullptr || strcmp(state.level.activeId, wave.levelId) != 0) {
			gamelog::Verbose("Active level '%s' is different from '%s'", state.level.activeId, wave.levelId);
			ClearLevel(state);
			char levelFilename[1024];
			fplCopyAnsiString(wave.levelId, levelFilename, FPL_ARRAYCOUNT(levelFilename));
			fplChangeFileExtension(levelFilename, ".tmx", levelFilename, FPL_ARRAYCOUNT(levelFilename));
			if (LoadLevel(state, state.assets.dataPath, levelFilename, state.level.data)) {
				fplCopyAnsiString(wave.levelId, state.level.activeId, FPL_ARRAYCOUNT(state.level.activeId));
			} else {
				gamelog::Error("Failed loading level '%s'!", levelFilename);
				return;
			}
			state.towers.selectedIndex = 0;
		}

		if (state.wave.totalEnemyCount > 0 || state.waypoints.first != nullptr || state.spawners.count > 0) {
			ClearWave(state);
		}

		if (fplGetAnsiStringLength(state.level.activeId) == 0) {
			gamelog::Error("No level loaded!");
			return;
		}

		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		if (!IsValidTile(goalTilePos)) {
			gamelog::Error("No goal entity in level '%s' found!", state.level.activeId);
			return;
		}

		state.wave.activeIndex = waveIndex;
		state.wave.totalEnemyCount = 0;
		for (size_t objectIndex = 0; objectIndex < state.level.data.objectCount; ++objectIndex) {
			ObjectData &obj = state.level.data.objects[objectIndex];
			Vec2i objTilePos = obj.tilePos;
			if (!IsValidTile(objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Object '%zu:%s'!", objTilePos.x, objTilePos.y, objectIndex, ObjectTypeToString(obj.type));
				continue;
			}
#if 0
			switch (obj.type) {
				case ObjectType::Spawn:
				{
					const SpawnData &data = obj.spawn;
					const CreepData *creepData = creeps::FindEnemyById(state, data.enemyId);
					if (creepData != nullptr) {
						CreepData enemyTemplate = *creepData;
						enemyTemplate.hp = (int)((float)enemyTemplate.hp * wave.enemyMultiplier.hp);
						enemyTemplate.bounty = (int)((float)enemyTemplate.bounty * wave.enemyMultiplier.bounty);
						enemyTemplate.speed *= wave.enemyMultiplier.speed;
						enemyTemplate.collisionRadius *= wave.enemyMultiplier.scale;
						creeps::AddSpawner(state.spawners, objTilePos, goalTilePos, data.initialCooldown, data.cooldown, data.enemyCount, enemyTemplate);
					} else {
						gamelog::Warning("Enemy by id '%s' does not exists!", data.enemyId);
					}
					state.wave.totalEnemyCount += data.enemyCount;
				} break;
			}
#endif
		}

		for (size_t spawnerIndex = 0; spawnerIndex < wave.spawnerCount; ++spawnerIndex) {
			const SpawnData &spawnerFromWave = wave.spawners[spawnerIndex];
			if (spawnerFromWave.enemyCount == 0) {
				continue;
				gamelog::Warning("No enemies for Spawner '%s'!", spawnerFromWave.spawnId);
			}
			const ObjectData *spawnObj = FindSpawnObjectById(state.level, spawnerFromWave.spawnId);
			if (spawnObj == nullptr) {
				continue;
				gamelog::Warning("Spawner by id '%s' does not exists!", spawnerFromWave.spawnId);
			}
			Vec2i objTilePos = spawnObj->tilePos;
			if (!IsValidTile(objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Spawner '%s'!", objTilePos.x, objTilePos.y, spawnObj->spawn.spawnId);
				continue;
			}
			const CreepData *creepData = creeps::FindEnemyById(state, spawnerFromWave.enemyId);
			if (creepData == nullptr) {
				continue;
				gamelog::Warning("Enemy by id '%s' does not exists!", spawnerFromWave.enemyId);
			}
			creeps::AddSpawner(state.spawners, objTilePos, goalTilePos, spawnerFromWave.initialCooldown, spawnerFromWave.cooldown, spawnerFromWave.enemyCount, spawnerFromWave.startMode, creepData);
			state.wave.totalEnemyCount += spawnerFromWave.enemyCount;
		}

		state.wave.state = WaveState::Starting;
		state.wave.isActive = true;
		state.wave.warmupTimer = wave.startupCooldown;
	}
}

namespace towers {
	enum class CanPlaceTowerResult {
		Success = 0,
		NoTowerSelected,
		TooManyTowers,
		TileOccupied,
		NotEnoughMoney,
	};

	inline CanPlaceTowerResult CanPlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *tower) {
		if ((state.towers.selectedIndex < 0) || !(state.towers.selectedIndex < FPL_ARRAYCOUNT(TowerDefinitions))) {
			return CanPlaceTowerResult::NoTowerSelected;
		}
		if (state.towers.count == FPL_ARRAYCOUNT(state.towers.list)) {
			return CanPlaceTowerResult::TooManyTowers;
		}
		Tile *tile = level::GetTile(state.level, tilePos);
		if (tile == nullptr) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if (tile->isOccupied || tile->entityType != EntityType::None || tile->wayType != WayType::None) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if (state.stats.money < tower->costs) {
			return CanPlaceTowerResult::NotEnoughMoney;
		}
		return(CanPlaceTowerResult::Success);
	}

	static Tower *PlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *data) {
		assert(state.towers.count < FPL_ARRAYCOUNT(state.towers.list));
		Tower *tower = &state.towers.list[state.towers.count++];
		*tower = {};
		tower->data = data;
		tower->position = TileToWorld(tilePos, TileExt);
		tower->facingAngle = (float)M_PI * 0.5f; // Face north by default

		Tile *tile = level::GetTile(state.level, tilePos);
		assert(!tile->isOccupied);
		tile->isOccupied = true;

		assert(state.stats.money >= data->costs);
		state.stats.money -= data->costs;

		return(tower);
	}

	static Vec2f PredictEnemyPosition(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f result;

		// First we compute how many frames we need until we can actually fire (Weapon cooldown)
		if (tower.data->enemyPredictionFlags != EnemyPredictionFlags::None) {
			float framesRequiredToFire;
			if ((tower.data->enemyPredictionFlags & EnemyPredictionFlags::WeaponCooldown) == EnemyPredictionFlags::WeaponCooldown) {
				framesRequiredToFire = (tower.gunTimer / deltaTime);
			} else {
				framesRequiredToFire = 0;
			}
			float timeScale = Max(framesRequiredToFire, 1.0f);

			// Now we predict the enemy position based on the enemy speed and the number of frames required to fire
			Vec2f predictedPosition = enemy.position + enemy.facingDirection * enemy.speed * deltaTime * timeScale;
			Vec2f distanceToEnemy = predictedPosition - tower.position;

			// Second we compute how many frames we need the bullet to move to the predicted position
			float framesRequiredForBullet;
			if ((tower.data->enemyPredictionFlags & EnemyPredictionFlags::BulletDistance) == EnemyPredictionFlags::BulletDistance) {
				assert(tower.data->bullet.speed > 0);
				float bulletDistance = Vec2Length(distanceToEnemy) / (tower.data->bullet.speed / deltaTime);
				framesRequiredForBullet = (bulletDistance / deltaTime);
			} else {
				framesRequiredForBullet = 0;
			}

			// Now we recompute the time scale and the predicted enemy position
			timeScale = Max(framesRequiredToFire + framesRequiredForBullet, 1.0f);
			result = enemy.position + enemy.facingDirection * enemy.speed * deltaTime * timeScale;
		} else {
			result = enemy.position;
		}
		return(result);
	}

	static bool InFireRange(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
		Vec2f predictedEnemyPosition = PredictEnemyPosition(tower, enemy, deltaTime);
		Vec2f distanceToEnemy = predictedEnemyPosition - tower.position;
		bool result;
		if (tower.data->enemyRangeTestType == FireRangeTestType::LineTrace) {
			float maxDistance = Vec2Length(distanceToEnemy) + enemy.data->collisionRadius;
			LineCastInput input = {};
			input.p1 = tower.position + lookDirection * tower.data->gunTubeLength;
			input.p2 = input.p1 + lookDirection * maxDistance;
			input.maxFraction = 1.0f;
			LineCastOutput output = {};
			result = LineCastCircle(input, enemy.position, enemy.data->collisionRadius, output);
		} else {
			float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
			if (projDistance > 0) {
				Vec2f lookPos = tower.position + lookDirection * projDistance;
				float dot = Vec2Dot(predictedEnemyPosition, lookPos);
				float det = predictedEnemyPosition.x * lookPos.y - predictedEnemyPosition.y * lookPos.x;
				float angle = ArcTan2(det, dot);
				result = angle >= -ShotAngleTolerance && angle <= ShotAngleTolerance;
			} else {
				result = false;
			}
		}
		return(result);
	}

	static void ShootBullet(Bullets &bullets, Tower &tower) {
		assert(bullets.count < FPL_ARRAYCOUNT(bullets.list));
		Bullet *bullet = &bullets.list[bullets.count++];
		*bullet = {};
		Vec2f targetDir = V2f(Cosine(tower.facingAngle), Sine(tower.facingAngle));
		bullet->position = bullet->prevPosition = tower.position + targetDir * tower.data->gunTubeLength;
		bullet->data = &tower.data->bullet;
		bullet->velocity = targetDir * bullet->data->speed;
		tower.canFire = false;
		tower.gunTimer = tower.data->gunCooldown;
	}

	static void UpdateTower(GameState &state, Tower &tower, const float deltaTime) {
		// Remove lost or dead target
		// @NOTE(final): Dead enemies can be immediately reused in the next frame, so we cannot use isDead only
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Vec2f distance = tower.targetEnemy->position - tower.position;
			assert(tower.data->unlockRadius >= tower.data->detectionRadius);
			if (tower.targetEnemy->isDead ||
				(tower.targetEnemy->id != tower.targetId) ||
				(Vec2Length(distance) > tower.data->unlockRadius)) {
				tower.targetEnemy = nullptr;
				tower.hasTarget = false;
				tower.targetId = 0;
			}
		}

		// Detect a new target
		if (!tower.hasTarget) {
			float bestEnemyDistance = FLT_MAX;
			Creep *bestEnemy = nullptr;
			for (size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep *testEnemy = &state.enemies.list[enemyIndex];
				if (!testEnemy->isDead) {
					float distanceRadius = Vec2Length(testEnemy->position - tower.position);
					if (distanceRadius < bestEnemyDistance) {
						bestEnemy = testEnemy;
						bestEnemyDistance = distanceRadius;
					}
				}
			}
			if (bestEnemy != nullptr && bestEnemyDistance <= tower.data->detectionRadius) {
				tower.targetEnemy = bestEnemy;
				tower.targetId = bestEnemy->id;
				tower.hasTarget = true;
			}
		}

		// Weapon cooldown
		if (!tower.canFire && tower.gunTimer > 0) {
			tower.gunTimer -= deltaTime;
		} else {
			tower.gunTimer = 0;
			tower.canFire = true;
		}

		//
		// Rotate gun
		//
		if (tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Creep *enemy = tower.targetEnemy;
			assert(enemy->id == tower.targetId);
			Vec2f predictedEnemyPosition = towers::PredictEnemyPosition(tower, *enemy, deltaTime);
			Vec2f directionToEnemy = Vec2Normalize(predictedEnemyPosition - tower.position);
			float angleToEnemy = Vec2AxisToAngle(directionToEnemy);
			tower.facingAngle = AngleLerp(tower.facingAngle, deltaTime * tower.data->gunRotationSpeed, angleToEnemy);
		}

		//
		// Shoot
		//
		if (tower.data->enemyLockOnMode == EnemyLockTargetMode::Any) {
			for (size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep &enemy = state.enemies.list[enemyIndex];
				if (!enemy.isDead) {
					bool inFireRange = towers::InFireRange(tower, enemy, deltaTime);
					if (inFireRange && tower.canFire) {
						ShootBullet(state.bullets, tower);
					}
				}
			}
		} else if (tower.data->enemyLockOnMode == EnemyLockTargetMode::LockedOn) {
			if (tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				Creep *enemy = tower.targetEnemy;
				assert(!enemy->isDead);
				bool inFireRange = towers::InFireRange(tower, *enemy, deltaTime);
				if (inFireRange && tower.canFire) {
					ShootBullet(state.bullets, tower);
				}
			}
		}
	}

	static void DrawTower(const Assets &assets, const Camera2D &camera, const TowerData &tower, const Vec2f &pos, const Vec2f &maxRadius, const float angle, const float alpha, const bool drawRadius) {
		assert(MaxTileRadius > 0);
		float scale = FPL_MAX(maxRadius.x, maxRadius.y) / MaxTileRadius;

		// @TODO(final): Mulitple tower styles
		DrawPoint(camera, pos.x, pos.y, tower.structureRadius * scale, V4f(1, 1, 0.5f, alpha));

		glColor4f(1, 0.85f, 0.5f, alpha);
		glLineWidth(camera.worldToPixels * tower.gunTubeThickness * scale);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * RadToDeg, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2f(tower.gunTubeLength * scale, 0);
		glVertex2f(0, 0);
		glEnd();
		glPopMatrix();
		glLineWidth(DefaultLineWidth);

		if (drawRadius) {
			glColor4f(0.2f, 1, 0.2f, alpha*0.25f);
			DrawSprite(assets.radiantTexture, tower.detectionRadius * scale, tower.detectionRadius * scale, 0.0f, 0.0f, 1.0f, 1.0f, pos.x, pos.y);
			glColor4f(1, 0.25f, 0.25f, alpha*0.25f);
			DrawSprite(assets.radiantTexture, tower.unlockRadius * scale, tower.unlockRadius * scale, 0.0f, 0.0f, 1.0f, 1.0f, pos.x, pos.y);
		}
	}
}

namespace game {
	static void SetSlowdown(GameState &state, const float duration, const WaveState nextState) {
		assert(!state.isSlowDown);
		state.isSlowDown = true;
		state.slowdownScale = 0.0f;
		state.slowdownTimer[0] = state.slowdownTimer[1] = duration;
		state.waveStateAfterSlowdown = nextState;
	}

	static void ReleaseFontAsset(FontAsset &font) {
		glDeleteTextures(1, &font.texture);
		ReleaseFont(&font.desc);
	}

	static void ReleaseAssets(Assets &assets) {
		ReleaseFontAsset(assets.overlayFont);
		ReleaseFontAsset(assets.hudFont);
	}

	static GLuint LoadTexture(const char *dataPath, const char *filename) {
		char filePath[1024];
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);
		int width, height, comp;
		uint8_t *data = stbi_load(filePath, &width, &height, &comp, 4);
		GLuint result = 0;
		if (data != nullptr) {
			result = AllocateTexture(width, height, data, false, GL_LINEAR, false);
		}
		return(result);
	}

	static void LoadAssets(Assets &assets) {
		char fontDataPath[1024];
		const char *fontFilename = "SulphurPoint-Bold.otf";
		fplPathCombine(fontDataPath, FPL_ARRAYCOUNT(fontDataPath), 2, assets.dataPath, "fonts");
		//const char *fontDataPath = "c:\\windows\\fonts";
		//const char *fontFilename = "times.ttf";
		if (LoadFontFromFile(fontDataPath, fontFilename, 0, 36.0f, 32, 128, 512, 512, false, &assets.hudFont.desc)) {
			assets.hudFont.texture = AllocateTexture(assets.hudFont.desc.atlasWidth, assets.hudFont.desc.atlasHeight, assets.hudFont.desc.atlasAlphaBitmap, false, GL_LINEAR, true);
		}
		if (LoadFontFromFile(fontDataPath, fontFilename, 0, 240.0f, 32, 128, 4096, 4096, false, &assets.overlayFont.desc)) {
			assets.overlayFont.texture = AllocateTexture(assets.overlayFont.desc.atlasWidth, assets.overlayFont.desc.atlasHeight, assets.overlayFont.desc.atlasAlphaBitmap, false, GL_LINEAR, true);
		}

		char texturesDataPath[1024];
		fplPathCombine(texturesDataPath, FPL_ARRAYCOUNT(texturesDataPath), 2, assets.dataPath, "textures");
		assets.radiantTexture = LoadTexture(texturesDataPath, "radiant.png");
	}

	static void ReleaseGame(GameState &state) {
		gamelog::Verbose("Release Game");

		level::ClearLevel(state);
		ReleaseAssets(state.assets);
		fglUnloadOpenGL();
	}

	static void NewGame(GameState &state) {
		// Reset camera
		state.camera.scale = 1.0f;
		state.camera.offset.x = 0;
		state.camera.offset.y = 0;

		// @TODO(final): Initial values from first wave?
		state.stats.money = 100;
		state.stats.lifes = 3;

		// Load initial wave
		level::LoadWave(state, 0);
	}

	static bool InitGame(GameState &state, GameMemory &gameMemory) {
		gamelog::Verbose("Initialize Game");

		if (!fglLoadOpenGL(true)) {
			gamelog::Fatal("Failed loading opengl!");
			return false;
		}

		fplGetExecutableFilePath(state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath));
		fplExtractFilePath(state.assets.dataPath, state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath));
		fplPathCombine(state.assets.dataPath, FPL_ARRAYCOUNT(state.assets.dataPath), 2, state.assets.dataPath, "data");
		gamelog::Info("Using assets path: %s", state.assets.dataPath);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_TEXTURE_2D);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(DefaultLineWidth);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		state.mem = gameMemory;

		LoadAssets(state.assets);

		NewGame(state);

		return(true);
	}

	static void DrawHUD(GameState &state) {
		constexpr float hudPadding = MaxTileSize * 0.075f;
		constexpr float hudOriginX = -WorldRadiusW;
		constexpr float hudOriginY = WorldRadiusH;
		constexpr float hudFontHeight = TileHeight * 0.4f;
		constexpr float outlineOffset = hudFontHeight * 0.05f;
		const FontAsset &font = state.assets.hudFont;
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%s", state.level.activeId);
			Vec2f textPos = V2f(hudOriginX + WorldRadiusW, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			glColor4fv(&TextBackColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 0.0f, 0.0f);
			glColor4fv(&TextForeColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 0.0f, 0.0f);

			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Wave: %d / %zu", (state.wave.activeIndex + 1), FPL_ARRAYCOUNT(WaveDefinitions));
			textPos.y -= hudFontHeight;
			glColor4fv(&TextBackColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 0.0f, 0.0f);
			glColor4fv(&TextForeColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 0.0f, 0.0f);
		}
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "$: %d", state.stats.money);
			Vec2f textPos = V2f(hudOriginX + hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			glColor4fv(&TextBackColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 1.0f, 0.0f);
			glColor4fv(&TextForeColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 1.0f, 0.0f);
		}
		{
			char text[256];
			fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "HP: %d", state.stats.lifes);
			Vec2f textPos = V2f(hudOriginX + WorldWidth - hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			glColor4fv(&TextBackColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, -1.0f, 0.0f);
			glColor4fv(&TextForeColor.m[0]);
			DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, -1.0f, 0.0f);
		}
	}

	static void DrawTowerControl(GameState &gameState, const Vec2f &pos, const Vec2f &radius, const ui::UIButtonState buttonState, void *userData) {
		int towerDataIndex = (int)(uintptr_t)(userData);
		assert(towerDataIndex >= 0 && towerDataIndex < FPL_ARRAYCOUNT(TowerDefinitions));
		const TowerData *towerData = &TowerDefinitions[towerDataIndex];
		float alpha = 0.75f;
		if (buttonState == ui::UIButtonState::Hover) {
			alpha = 1.0f;
		}
		towers::DrawTower(gameState.assets, gameState.camera, *towerData, pos, radius, Pi32 * 0.5f, alpha, false);

		// Draw selection frame
		if (gameState.towers.selectedIndex == towerDataIndex) {
			Vec2f borderVecs[] = {
				V2f(pos.x + radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y - radius.h),
				V2f(pos.x + radius.w, pos.y - radius.h),
			};
			float stippleWidth = (FPL_MIN(radius.x, radius.y) * 2.0f) / 10.0f;
			glColor4f(1.0f, 1.0f, 1.0f, alpha);
			glLineWidth(1.0f);
			render::DrawLineLoopStipple(borderVecs, 4, stippleWidth, 3);
			glLineWidth(DefaultLineWidth);
		}
	}

	static void DrawControls(GameState &state) {
		//
		// Controls Background
		//
		glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2f(ControlsOriginX + ControlsWidth, ControlsOriginY + ControlsHeight);
		glVertex2f(ControlsOriginX, ControlsOriginY + ControlsHeight);
		glVertex2f(ControlsOriginX, ControlsOriginY);
		glVertex2f(ControlsOriginX + ControlsWidth, ControlsOriginY);
		glEnd();

		// Controls Border
		float lineWidth = 2.0f;
		float lineWidthWorld = lineWidth * state.camera.pixelsToWorld * 0.5f;
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
		glLineWidth(lineWidth);
		glBegin(GL_LINE_LOOP);
		glVertex2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld);
		glVertex2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld);
		glVertex2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + lineWidthWorld);
		glVertex2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + lineWidthWorld);
		glEnd();
		glLineWidth(DefaultLineWidth);

		// Tower buttons
		float buttonPadding = MaxTileSize * 0.1f;
		float buttonMargin = lineWidthWorld + (MaxTileSize * 0.15f);
		float buttonHeight = ControlsHeight - buttonMargin * 2.0f;
		Vec2f buttonRadius = V2f(buttonHeight * 0.5f);
		Vec2f buttonOutputRadius = ui::GetUIButtonExt(buttonRadius);
		for (int towerIndex = 0; towerIndex < FPL_ARRAYCOUNT(TowerDefinitions); ++towerIndex) {
			void *buttonId = (void *)&TowerDefinitions[towerIndex]; // Totally dont care about const removal here
			float buttonX = ControlsOriginX + buttonMargin + (towerIndex * (buttonOutputRadius.w * 2.0f) + (FPL_MAX(0, towerIndex - 1) * buttonPadding));
			float buttonY = ControlsOriginY + buttonMargin;
			if (ui::UIButton(state.ui, buttonId, V2f(buttonX + buttonRadius.w, buttonY + buttonRadius.h), buttonRadius, DrawTowerControl, (void *)(uintptr_t)towerIndex)) {
				state.towers.selectedIndex = towerIndex;
			}
		}

		if (state.towers.selectedIndex > -1) {
			const FontAsset &font = state.assets.hudFont;
			float fontHeight = MaxTileSize * 0.4f;
			const TowerData &towerData = TowerDefinitions[state.towers.selectedIndex];
			Vec2f textPos = V2f(ControlsOriginX + ControlsWidth - lineWidthWorld - buttonMargin, ControlsOriginY + ControlsHeight * 0.5f);
			char textBuffer[256];
			fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "[%s / $%d]", towerData.id, towerData.costs);
			glColor4fv(&TextForeColor.m[0]);
			DrawTextFont(textBuffer, fplGetAnsiStringLength(textBuffer), &font.desc, font.texture, textPos.x, textPos.y, fontHeight, -1.0f, 0.0f);
		}

	}

}

extern GameMemory GameCreate() {
	gamelog::Verbose("Create Game");
	GameMemory result = {};
	result.capacity = sizeof(GameState) + FPL_MEGABYTES(16);
	result.base = fplMemoryAllocate(result.capacity);
	if (result.base == nullptr) {
		gamelog::Fatal("Failed allocating Game State memory of '%zu' bytes!", result.capacity);
		return {};
	}
	GameState *state = (GameState *)result.base;
	result.used = sizeof(GameState);
	if (!game::InitGame(*state, result)) {
		gamelog::Fatal("Failed initializing Game!");
		GameDestroy(result);
		state = nullptr;
	}
	return(result);
}

extern void GameDestroy(GameMemory &gameMemory) {
	gamelog::Verbose("Destroy Game");
	GameState *state = (GameState *)gameMemory.base;
	game::ReleaseGame(*state);
	state->~GameState();
	fplMemoryFree(state);
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;

	ui::UIBegin(state->ui, state, input, state->mouseWorldPos);

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

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	if (state->wave.state == WaveState::Running || state->wave.state == WaveState::Starting) {
		// Update tile position from mouse
		state->mouseTilePos = WorldToTile(state->mouseWorldPos);

		// Tower placement
		if (WasPressed(input.mouse.left) && !ui::UIIsHot(state->ui)) {
			if (state->towers.selectedIndex > -1) {
				const TowerData *tower = &TowerDefinitions[state->towers.selectedIndex];
				if (towers::CanPlaceTower(*state, state->mouseTilePos, tower) == towers::CanPlaceTowerResult::Success) {
					towers::PlaceTower(*state, state->mouseTilePos, tower);
				}
			}
		}
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}

	GameState *state = (GameState *)gameMemory.base;

	float dtScale = 1.0f;
	if (state->isSlowDown) {
		assert(state->slowdownTimer[1] > 0);
		if (state->slowdownTimer[0] > 0.0f) {
			state->slowdownTimer[0] -= input.deltaTime;
		} else {
			state->slowdownTimer[0] = 0;
			if (state->wave.state != state->waveStateAfterSlowdown) {
				state->wave.state = state->waveStateAfterSlowdown;
			}
		}
		float t = 1.0f - (state->slowdownTimer[0] / state->slowdownTimer[1]);
		dtScale = ScalarLerp(1.0f, t, state->slowdownScale);
	}
	const float dt = input.deltaTime * dtScale;

	state->deltaTime = dt;
	state->framesPerSecond = input.framesPerSeconds;

	// Startup wave
	if (state->wave.state == WaveState::Starting) {
		state->wave.warmupTimer -= dt;
		if (state->wave.warmupTimer <= 0.0f) {
			state->wave.warmupTimer = 0;
			state->wave.state = WaveState::Running;
		}
	}

	if (state->wave.state == WaveState::Running) {
		//
		// Move enemies
		//
		for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if (!enemy.isDead && enemy.hasTarget) {
				Vec2f distance = enemy.targetPos - enemy.position;
				float minRadius = MaxTileSize * 0.05f;
				enemy.position += enemy.facingDirection * enemy.speed * dt;
				if (Vec2Dot(distance, distance) <= minRadius * minRadius) {
					creeps::SetCreepNextTarget(*state, enemy);
				}
			}
		}

		// Update spawners
		for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
			CreepSpawner &spawner = state->spawners.list[spawnerIndex];
			creeps::UpdateSpawner(state->enemies, spawner, state->waypoints, dt);
		}

		// Update towers
		for (size_t towerIndex = 0; towerIndex < state->towers.count; ++towerIndex) {
			Tower &tower = state->towers.list[towerIndex];
			towers::UpdateTower(*state, tower, dt);
		}

		//
		// Move and collide bullets
		//
		for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if (!bullet.isDestroyed) {
				bullet.position += bullet.velocity * dt;
				if (!bullet.hasHit) {
					for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
						Creep &enemy = state->enemies.list[enemyIndex];
						if (!enemy.isDead) {
							Vec2f distance = enemy.position - bullet.position;
							float bothRadi = bullet.data->collisionRadius + enemy.data->collisionRadius;
							float d = Vec2Dot(distance, distance);
							if (d < bothRadi) {
								bullet.hasHit = true;
								creeps::CreepHit(*state, enemy, bullet);
								break;
							}
						}
					}
				}
				if (!bullet.hasHit) {
					if (((bullet.position.x + bullet.data->renderRadius) > WorldRadiusW) ||
						((bullet.position.y + bullet.data->renderRadius) > WorldRadiusH) ||
						((bullet.position.y - bullet.data->renderRadius) < -WorldRadiusH) ||
						((bullet.position.x - bullet.data->renderRadius) < -WorldRadiusW)) {
						bullet.isDestroyed = true;
					}
				}
			}
		}

		//
		// Remove dead enemies and destroyed bullets
		//
		for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if (bullet.hasHit) {
				bullet.isDestroyed = true;
			}
			if (bullet.isDestroyed) {
				if (bulletIndex < state->bullets.count - 1) {
					const Bullet &lastBullet = state->bullets.list[state->bullets.count - 1];
					bullet = lastBullet;
				}
				--state->bullets.count;
			}
		}
		size_t deadEnemyCount = 0;
		size_t nonDeadEnemyCount = 0;
		for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if (enemy.isDead) {
				++deadEnemyCount;
			} else {
				++nonDeadEnemyCount;
			}
		}

		if (state->wave.totalEnemyCount == deadEnemyCount) {
			creeps::AllEnemiesKilled(*state);
		} else {
			bool hasActiveSpawners = false;
			CreepSpawner *nextSpawner = nullptr;
			for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
				CreepSpawner *spawner = &state->spawners.list[spawnerIndex];
				if (spawner->isActive) {
					hasActiveSpawners = true;
					break;
				} else {
					if (nextSpawner == nullptr && spawner->startMode == SpawnerStartMode::AfterTheLast) {
						nextSpawner = spawner;
					}
				}
			}
			if (nonDeadEnemyCount == 0 && !hasActiveSpawners) {
				// All enemies but not all from all spawners has been killed
				if (nextSpawner != nullptr) {
					nextSpawner->isActive = true;
					nextSpawner->spawnTimer = nextSpawner->cooldown;
					nextSpawner->remainingCount = nextSpawner->totalCount;
				}
			}
		}
	}
}

extern void GameRender(GameMemory &gameMemory, CommandBuffer &renderCommands, const float alpha) {
	GameState *state = (GameState *)gameMemory.base;
	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float invScale = 1.0f / state->camera.scale;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);

#if 0
	// Font rendering test
	glLineWidth(1.0f);
	glColor4f(0, 1, 0, 1);
	glBegin(GL_LINES);
	glVertex2f(w, 0);
	glVertex2f(-w, 0);
	glVertex2f(0, h);
	glVertex2f(0, -h);
	glEnd();
	glLineWidth(DefaultLineWidth);

	const FontAsset *fontAsset = &state->assets.hudFont;
	const char *text = "Hello World!";
	glColor4f(1, 1, 1, 1);
	DrawTextFont(text, fplGetAnsiStringLength(text), &fontAsset->desc, fontAsset->texture, 0.0f, 0.0f, MaxTileSize * 2.0f, 0.0f, 1.0f);
#endif

#if 1
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(state->camera.offset.x, state->camera.offset.y, 0);

	//
	// Tiles
	//
	for (int y = 0; y < TileCountY; ++y) {
		for (int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->level.tiles[y * TileCountX + x];
			if (tile.wayType != WayType::None) {
				// @TODO(final): Draw sprite for this way tile
				render::DrawTile(x, y, true, V4f(0.0f, 0.0f, 1.0f, 1.0f));
			}
		}
	}

	// Draw tile entities
	for (int y = 0; y < TileCountY; ++y) {
		for (int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->level.tiles[y * TileCountX + x];
			switch (tile.entityType) {
				case EntityType::Goal:
				{
					// @TODO(final): Draw goal sprite
					render::DrawTile(x, y, true, V4f(0.1f, 1.0f, 0.2f, 1.0f));
				} break;
			}
		}
	}

	//
	// Spawners
	//
	for (size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
		const CreepSpawner &spawner = state->spawners.list[spawnerIndex];
		Vec2i tilePos = WorldToTile(spawner.spawnPosition);
		// @TODO(final): Draw spawner sprite
		render::DrawTile(tilePos.x, tilePos.y, true, V4f(0.0f, 1.0f, 1.0f, 1.0f));
	}


#if 1
	//
	// Grid
	//
	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
	glBegin(GL_LINES);
	for (int y = 0; y <= TileCountY; ++y) {
		glVertex2f(GridOriginX, GridOriginY + y * TileHeight);
		glVertex2f(GridOriginX + TileCountX * TileWidth, GridOriginY + y * TileHeight);
	}
	for (int x = 0; x <= TileCountX; ++x) {
		glVertex2f(GridOriginX + x * TileWidth, GridOriginY);
		glVertex2f(GridOriginX + x * TileWidth, GridOriginY + TileCountY * TileHeight);
	}
	glEnd();
#endif

	if (state->isDebugRendering) {
		// Waypoints
		for (Waypoint *waypoint = state->waypoints.first; waypoint != nullptr; waypoint = waypoint->next) {
			DrawPoint(state->camera, waypoint->position.x, waypoint->position.y, MaxTileSize * 0.15f, V4f(1, 0, 1, 1));
			DrawNormal(waypoint->position, waypoint->direction, level::WaypointDirectionWidth, V4f(1, 1, 1, 1));
		}
	}

	// Hover tile
	if (state->towers.selectedIndex > -1 && IsValidTile(state->mouseTilePos)) {
		const TowerData *tower = &TowerDefinitions[state->towers.selectedIndex];

		towers::CanPlaceTowerResult placeRes = towers::CanPlaceTower(*state, state->mouseTilePos, tower);
		Vec4f hoverColor;
		if (placeRes == towers::CanPlaceTowerResult::Success) {
			hoverColor = V4f(0.1f, 1.0f, 0.1f, 1.0f);
		} else {
			hoverColor = V4f(1.0f, 0.1f, 0.1f, 1.0f);
		}

		if (placeRes == towers::CanPlaceTowerResult::Success || placeRes == towers::CanPlaceTowerResult::NotEnoughMoney) {
			float alpha = placeRes == towers::CanPlaceTowerResult::Success ? 0.5f : 0.2f;
			Vec2f towerCenter = TileToWorld(state->mouseTilePos, TileExt);
			towers::DrawTower(state->assets, state->camera, *tower, towerCenter, V2f(MaxTileRadius), Pi32 * 0.5f, alpha, true);
		}

		render::DrawTile(state->mouseTilePos.x, state->mouseTilePos.y, false, hoverColor);
	}

	//
	// Enemies
	//
	for (size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
		Creep &enemy = state->enemies.list[enemyIndex];
		if (!enemy.isDead && enemy.id > 0) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);

			// Mesh
			switch (enemy.data->style) {
				case CreepStyle::Triangle:
				{
					float angle = ArcTan2(enemy.facingDirection.y, enemy.facingDirection.x);
					float r = enemy.data->renderRadius;
					glColor4fv(&enemy.data->color.m[0]);
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
					float r = enemy.data->renderRadius;
					DrawPoint(state->camera, enemyPos.x, enemyPos.y, r, enemy.data->color);
				} break;
			}

			// HP Bar
			{
				float barWidth = TileWidth * 0.85f;
				float barHeight = TileHeight * 0.1625f;
				float barSpaceUnder = TileHeight * 0.15f;
				float barX = enemy.position.x - barWidth * 0.5f;
				float barY = enemy.position.y - enemy.data->renderRadius - barSpaceUnder - barHeight;
				float barScale = enemy.hp / (float)enemy.data->hp;

				float colorGreen = barScale;
				float colorRed = 1.0f - colorGreen;

				glColor4f(colorRed, colorGreen, 0.0f, 1.0f);
				glBegin(GL_QUADS);
				glVertex2f(barX + barWidth * barScale, barY + barHeight);
				glVertex2f(barX, barY + barHeight);
				glVertex2f(barX, barY);
				glVertex2f(barX + barWidth * barScale, barY);
				glEnd();

				glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
				glLineWidth(2);
				glBegin(GL_LINE_LOOP);
				glVertex2f(barX + barWidth, barY + barHeight);
				glVertex2f(barX, barY + barHeight);
				glVertex2f(barX, barY);
				glVertex2f(barX + barWidth, barY);
				glEnd();
				glLineWidth(DefaultLineWidth);
			}

			enemy.prevPosition = enemy.position;
		}
	}

	//
	// Towers
	//
	for (size_t towerIndex = 0; towerIndex < state->towers.count; ++towerIndex) {
		const Tower &tower = state->towers.list[towerIndex];
		towers::DrawTower(state->assets, state->camera, *tower.data, tower.position, V2f(MaxTileRadius), tower.facingAngle, 1.0f, false);

		if (state->isDebugRendering) {
			if (tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				const Creep *target = tower.targetEnemy;
				if ((target->id > 0) && (target->id == tower.targetId)) {
					DrawCircle(target->position.x, target->position.y, target->data->collisionRadius, false, V4f(1, 0, 0, 1), 32);

					Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
					Vec2f distanceToEnemy = target->position - tower.position;
					float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
					Vec2f lookPos = tower.position + lookDirection * projDistance;

					DrawCircle(lookPos.x, lookPos.y, MaxTileSize * 0.25f, false, V4f(1, 1, 0, 1), 16);

					float dot = Vec2Dot(target->position, lookPos);
					float det = Vec2Cross(target->position, lookPos);
					float angle = ArcTan2(det, dot);

					if (angle >= -ShotAngleTolerance && angle <= ShotAngleTolerance) {
						Vec2f sightPos1 = tower.position + Vec2AngleToAxis(tower.facingAngle - ShotAngleTolerance) * projDistance;
						Vec2f sightPos2 = tower.position + Vec2AngleToAxis(tower.facingAngle + ShotAngleTolerance) * projDistance;
						glColor4f(1, 0, 0, 0.5);
						glLineWidth(1);
						glBegin(GL_LINES);
						glVertex2f(tower.position.x, tower.position.y);
						glVertex2f(sightPos1.x, sightPos1.y);
						glVertex2f(tower.position.x, tower.position.y);
						glVertex2f(sightPos2.x, sightPos2.y);
						glEnd();
						glLineWidth(DefaultLineWidth);
					}
				}
			}
		}
	}

	//
	// Bullets
	//
	for (size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
		Bullet &bullet = state->bullets.list[bulletIndex];
		if (!bullet.isDestroyed) {
			Vec2f bulletPos = Vec2Lerp(bullet.prevPosition, alpha, bullet.position);
			// @TODO(final): Use sprites for bullets
			DrawPoint(state->camera, bulletPos.x, bulletPos.y, bullet.data->renderRadius, V4f(1, 0, 0, 1));
			bullet.prevPosition = bullet.position;
		}
	}

	//
	// Selected tower text
	//

	//
	// Overlay
	//
	if (state->wave.state == WaveState::Starting) {
		const FontAsset &font = state->assets.overlayFont;
		char text[128];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%d", (int)ceilf(state->wave.warmupTimer));
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.25f;
		float foffset = overlayFontHeight * 0.01f;
		glColor4fv(&TextBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, overlayFontHeight, 0.0f, 0.0f);
		glColor4fv(&TextForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + foffset, textPos.y - foffset, overlayFontHeight, 0.0f, 0.0f);
	} else if (state->wave.state == WaveState::Won || state->wave.state == WaveState::Lost) {
		const FontAsset &font = state->assets.overlayFont;
		const char *text = state->wave.state == WaveState::Won ? "You Win!" : "Game Over!";
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.15f;
		float foffset = overlayFontHeight * 0.01f;
		glColor4fv(&TextBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, overlayFontHeight, 0.0f, 0.0f);
		glColor4fv(&TextForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + foffset, textPos.y - foffset, overlayFontHeight, 0.0f, 0.0f);
	}

	if (state->isDebugRendering) {
		const FontAsset &font = state->assets.hudFont;
		char text[256];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Enemies: %03zu/%03zu, Bullets: %03zu, Towers: %03zu, Spawners: %03zu", state->enemies.count, state->wave.totalEnemyCount, state->bullets.count, state->towers.count, state->spawners.count);
		glColor4f(1, 1, 1, 1);
		float padding = MaxTileSize * 0.1f;
		Vec2f textPos = V2f(GridOriginX + padding, GridOriginY + padding);
		float fontHeight = MaxTileSize * 0.5f;
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, fontHeight, 1.0f, 1.0f);

		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond, state->deltaTime);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + GridWidth - padding * 2.0f, textPos.y, fontHeight, -1.0f, 1.0f);
	}
#endif

	//
	// HUD & Controls
	//
	game::DrawHUD(*state);
	game::DrawControls(*state);
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, CommandBuffer &renderCommands, const float alpha) {
	GameInput(gameMemory, input);
	GameUpdate(gameMemory, input);
	GameRender(gameMemory, renderCommands, alpha);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = "FPL Demo | Towadev";
	config.disableInactiveDetection = true;
	config.noUpdateRenderSeparation = true;
	gamelog::Verbose("Startup game application '%s'", config.title);
	int result = GameMain(config);
	return(result);
}