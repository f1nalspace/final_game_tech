/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev

Description:
	A tower defence clone.
	Levels are loaded from .TMX files (Tiled-Editor).
	Written in C++ (C-Style).

Requirements:
	- C++ Compiler
	- Final Dynamic OpenGL
	- Final XML
	- Final Framework

Author:
	Torsten Spaete

Changelog:
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
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#include <string.h>
#include <stdlib.h>
#include <varargs.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#define FINAL_FONTLOADER_BETTERQUALITY 0
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

constexpr float ShootAngleTolerance = (Pi32 * 0.1f) * 0.5f;

static const TowerData TowerDefinitions[] = {
	MakeTowerData(
		/* id: */ "Newbie Tower",
		/* structureRadius: */ MaxTileSize * 0.35f,
		/* detectionRadius: */ MaxTileSize * 2.25f,
		/* unlockRadius: */ MaxTileSize * 2.3f,
		/* gunTubeLength: */ MaxTileSize * 0.55f,
		/* gunCooldown: */ 0.35f,
		/* gunTubeThickness: */ MaxTileSize * 0.2f,
		/* gunRotationSpeed: */ 4.0f,
		/* costs: */ 50,
		/* enemyRangeTestType: */ FireRangeTestType::LineTrace,
		/* enemyPredictionFlags: */ EnemyPredictionFlags::All,
		MakeBulletData(
			/* renderRadius: */ MaxTileSize * 0.05f,
			/* collisionRadius: */ MaxTileSize * 0.05f,
			/* speed: */ 2.0f,
			/* damage: */ 20
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
		/* bounty: */ 2,
		/* color: */ V4f(1, 1, 1, 1),
		/* style: */ CreepStyle::Quad
	),
};

static const WaveData WaveDefinitions[] = {
	MakeWaveData(
		/* level: */ "Level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.0f, 1.0f, 1.0f, 1.0f),
		/* startupCooldown: */ 3.0f,
		/* completionBounty: */ 15
	),
	MakeWaveData(
		/* level: */ "Level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.15f, 1.15f, 1.15f, 1.15f),
		/* startupCooldown: */ 3.0f,
		/* completionBounty: */ 30
	),
	MakeWaveData(
		/* level: */ "Level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.3f, 1.3f, 1.3f, 1.3f),
		/* startupCooldown: */ 3.0f,
		/* completionBounty: */ 50
	),
	MakeWaveData(
		/* level: */ "Level1",
		/* enemyMultiplier */ MakeCreepMultiplier(1.45f, 1.45f, 1.45f, 1.45f),
		/* startupCooldown: */ 3.0f,
		/* completionBounty: */ 50
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
		if(level == LogLevel::Fatal)
			fplDebugOut("Fatal: ");
		else if(level == LogLevel::Error)
			fplDebugOut("Error: ");
		else if(level == LogLevel::Warning)
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
	static Vec2i FindTilePosByEntityType(const GameState &state, const EntityType type);
	static void LoadWave(GameState &state, const int waveIndex);
}
namespace game {
	static void NewGame(GameState &state);
}

namespace utils {
	static int StringToInt(const char *str, int def = 0) {
		int result = def;
		if(str != nullptr) {
			bool isNegative = false;
			const char *p = str;
			if(*p == '-') {
				isNegative = true;
				++p;
			}
			uint32_t value = 0;
			while(isdigit(*p)) {
				short v = *p - '0';
				value = value * 10 + v;
				++p;
			}
			if(isNegative) {
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
}

namespace creeps {
	static void SpawnEnemy(GameState &state, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData *data) {
		assert(state.enemyCount < FPL_ARRAYCOUNT(state.enemies));
		Creep *enemy = &state.enemies[state.enemyCount++];
		FPL_CLEAR_STRUCT(enemy);
		enemy->id = ++state.creepIdCounter;
		enemy->data = data;
		enemy->position = enemy->prevPosition = spawnPos;
		enemy->speed = data->speed;
		enemy->hp = data->hp;
		if(state.firstWaypoint != nullptr) {
			enemy->targetWaypoint = state.firstWaypoint;
			enemy->targetPos = TileToWorld(state.firstWaypoint->tilePos, TileExt);
		} else {
			enemy->targetWaypoint = nullptr;
			enemy->targetPos = exitPos;
		}
		enemy->facingDirection = Vec2Normalize(enemy->targetPos - enemy->position);
		enemy->hasTarget = true;
	}

	static void UpdateSpawner(GameState &state, CreepSpawner &spawner, const float deltaTime) {
		if(spawner.isActive) {
			assert(spawner.remainingCount > 0);
			assert(spawner.cooldown > 0);
			if(spawner.spawnTimer > 0) {
				spawner.spawnTimer -= deltaTime;
			}
			if(spawner.spawnTimer <= 0) {
				SpawnEnemy(state, spawner.spawnPosition, spawner.exitPosition, &spawner.spawnTemplate);
				--spawner.remainingCount;
				if(spawner.remainingCount == 0) {
					spawner.spawnTimer = 0;
					spawner.isActive = false;
				} else {
					spawner.spawnTimer = spawner.cooldown;
				}
			}
		}
	}

	static void AddSpawner(GameState &state, const Vec2i &spawnTilePos, const Vec2i &goalTilePos, const float initialCooldown, const float cooldown, const size_t count, const CreepData &spawnTemplate) {
		assert(state.spawnerCount < FPL_ARRAYCOUNT(state.spawners));
		CreepSpawner &spawner = state.spawners[state.spawnerCount++];
		spawner = {};
		spawner.spawnPosition = TileToWorld(spawnTilePos, TileExt);
		spawner.exitPosition = TileToWorld(goalTilePos, TileExt);
		spawner.cooldown = cooldown;
		spawner.spawnTimer = initialCooldown;
		spawner.totalCount = count;
		spawner.remainingCount = count;
		spawner.spawnTemplate = spawnTemplate;
		spawner.isActive = true;
	}

	static void CreepReachedExit(GameState &state, Creep &creep) {
		creep.isDead = true;
		state.lifes--;
		if(state.lifes <= 0) {
			state.lifes = 0;
			state.waveState = WaveState::Lost;
		}
	}

	static void SetCreepNextTarget(GameState &state, Creep &creep) {
		Vec2i creepTilePos = WorldToTile(creep.position);
		if(creep.targetWaypoint != nullptr) {
			const Waypoint waypoint = *creep.targetWaypoint;
			assert(Vec2Length(waypoint.direction) == 1);
			Vec2f creepDir = waypoint.direction;
			if(waypoint.next != nullptr) {
				creep.targetPos = TileToWorld(waypoint.next->tilePos, TileExt);
				creep.targetWaypoint = waypoint.next;
			} else {
				creep.targetWaypoint = nullptr;
				Vec2i goalTilePos = level::FindTilePosByEntityType(state, EntityType::Goal);
				assert(goalTilePos.x > -1 && goalTilePos.y > -1);
				creep.targetPos = TileToWorld(goalTilePos, TileExt);
			}
			creep.hasTarget = true;
			creep.facingDirection = Vec2Normalize(creep.targetPos - creep.position);
		} else {
			creep.hasTarget = false;
			CreepReachedExit(state, creep);
		}
	}

	static const CreepData *FindEnemyById(GameState &state, const char *id) {
		for(int i = 0; i < FPL_ARRAYCOUNT(CreepDefinitions); ++i) {
			if(strcmp(CreepDefinitions[i].id, id) == 0) {
				return &CreepDefinitions[i];
			}
		}
		return nullptr;
	}

	static void CreepHit(GameState &state, Creep &enemy, const Bullet &bullet) {
		enemy.hp -= bullet.data->damage;
		if(enemy.hp <= 0) {
			enemy.hp = 0;
			enemy.isDead = true;
			state.money += enemy.data->bounty;
		}
	}

	static void AllEnemiesKilled(GameState &state) {
		state.waveState = WaveState::Won;
		state.money += WaveDefinitions[state.activeWaveIndex].completionBounty;
		if(state.activeWaveIndex < (FPL_ARRAYCOUNT(WaveDefinitions) - 1)) {
			level::LoadWave(state, state.activeWaveIndex + 1);
		}
	}
}

namespace level {
	inline Tile *GetTile(GameState &state, const Vec2i &tilePos) {
		if((tilePos.x >= 0 && tilePos.x < TileCountX) && (tilePos.x >= 0 && tilePos.x < TileCountX)) {
			return &state.tiles[tilePos.y * TileCountX + tilePos.x];
		}
		return nullptr;
	}

	static Vec2i FindTilePosByEntityType(const GameState &state, const EntityType type) {
		for(int y = 0; y < TileCountY; ++y) {
			for(int x = 0; x < TileCountX; ++x) {
				int index = y * TileCountX + x;
				if(state.tiles[index].entityType == type) {
					return V2i(x, y);
				}
			}
		}
		return V2i(-1, -1);
	}

	constexpr float WaypointDirectionWidth = 0.35f;

	static void ClearWaypoints(GameState &state) {
		Waypoint *waypoint = state.firstWaypoint;
		while(waypoint != nullptr) {
			Waypoint *next = waypoint->next;
			fplMemoryFree(waypoint);
			waypoint = next;
		}
		state.firstWaypoint = state.lastWaypoint = nullptr;
	}

	static Waypoint *AddWaypoint(GameState &state, const Vec2i &tilePos, const Vec2f &dir) {
		Waypoint *waypoint = (Waypoint *)fplMemoryAllocate(sizeof(Waypoint));
		waypoint->tilePos = tilePos;
		waypoint->position = TileToWorld(tilePos, TileExt);
		waypoint->direction = dir;
		if(state.firstWaypoint == nullptr) {
			state.firstWaypoint = state.lastWaypoint = waypoint;
		} else {
			state.lastWaypoint->next = waypoint;
			state.lastWaypoint = waypoint;
		}
		return(waypoint);
	}

	static void ParseLevelLayer(fxmlTag *childTag, const uint32_t mapWidth, const uint32_t mapHeight, uint32_t *outData) {
		fxmlTag *dataTag = fxmlFindTagByName(childTag, "data");
		if(dataTag != nullptr) {
			const char *encodingStr = fxmlGetAttributeValue(dataTag, "encoding");
			if(strcmp(encodingStr, "csv") == 0) {
				const char *p = dataTag->value;
				int index = 0;
				while(*p) {
					if(isdigit(*p)) {
						uint32_t tileValue = 0;
						while(isdigit(*p)) {
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
					} else if(*p == ',') {
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
		while(childTag != nullptr) {
			if(childTag->type == fxmlTagType_Element) {
				if(strcmp(childTag->name, "object") == 0) {
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
					if(strcmp(typeName, "Spawn") == 0) {
						tmpObj.type = ObjectType::Spawn;
						switch(entityType) {
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
						if(propertiesTag != nullptr) {
							fxmlTag *propTag = propertiesTag->firstChild;
							while(propTag != nullptr) {
								const char *name = fxmlGetAttributeValue(propTag, "name");
								const char *value = fxmlGetAttributeValue(propTag, "value");
								if(strcmp(name, "enemy") == 0) {
									strcpy_s(tmpObj.spawn.enemyId, FPL_ARRAYCOUNT(tmpObj.spawn.enemyId), value);
								} else if(strcmp(name, "initialCooldown") == 0) {
									tmpObj.spawn.initialCooldown = utils::StringToFloat(value);
								} else if(strcmp(name, "cooldown") == 0) {
									tmpObj.spawn.cooldown = utils::StringToFloat(value);
								} else if(strcmp(name, "enemyCount") == 0) {
									tmpObj.spawn.enemyCount = utils::StringToInt(value);
								}
								propTag = propTag->nextSibling;
							}
						}
					} else if(strcmp(typeName, "Waypoint") == 0) {
						tmpObj.type = ObjectType::Waypoint;
						switch(entityType) {
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
					} else if(strcmp(typeName, "Goal") == 0) {
						tmpObj.type = ObjectType::Goal;
					}

					if(tmpObj.type != ObjectType::None) {
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
		if(mapTag == nullptr) {
			return false;
		}
		int mapWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "width"));
		int mapHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "height"));
		if((mapWidth > TileCountX) || (mapHeight > TileCountY)) {
			return false;
		}

		const char *orientation = fxmlGetAttributeValue(mapTag, "orientation");
		if(strcmp(orientation, "orthogonal") != 0) {
			return false;
		}

		int tileWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tilewidth"));
		int tileHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "tileheight"));
		if((tileWidth == 0) || (tileHeight == 0)) {
			return false;
		}

		level.tileWidth = tileWidth;
		level.tileHeight = tileHeight;

		fxmlTag *childTag = mapTag->firstChild;
		level.wayFirstGid = 0;
		level.entitiesFirstGid = 0;
		while(childTag) {
			if(childTag->type == fxmlTagType_Element) {
				if(strcmp(childTag->name, "tileset") == 0) {
					const char *tilesetName = fxmlGetAttributeValue(childTag, "name");
					int firstGid = utils::StringToInt(fxmlGetAttributeValue(childTag, "firstgid"));
					if(strcmp(tilesetName, "way") == 0) {
						level.wayFirstGid = firstGid;
					} else if(strcmp(tilesetName, "entities") == 0) {
						level.entitiesFirstGid = firstGid;
					}
				} else if(strcmp(childTag->name, "layer") == 0) {
					const char *layerName = fxmlGetAttributeValue(childTag, "name");
					if(strcmp(layerName, "way") == 0) {
						ParseLevelLayer(childTag, mapWidth, mapHeight, level.wayLayer);
					}
				} else if(strcmp(childTag->name, "objectgroup") == 0) {
					const char *objectGroupName = fxmlGetAttributeValue(childTag, "name");
					if(strcmp(objectGroupName, "objects") == 0) {
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
		if(fplOpenAnsiBinaryFile(filePath, &file)) {
			uint32_t dataSize = fplGetFileSizeFromHandle32(&file);
			char *data = (char *)fplMemoryAllocate(dataSize);
			fplReadFileBlock32(&file, dataSize, data, dataSize);
			fxmlContext ctx = {};
			if(fxmlInitFromMemory(data, dataSize, &ctx)) {
				fxmlTag root = {};
				if(fxmlParse(&ctx, &root)) {
					outLevel = {};
					if(ParseLevel(&root, outLevel)) {
						// Tiles
						for(int y = 0; y < TileCountY; ++y) {
							for(int x = 0; x < TileCountX; ++x) {
								int tileIndex = y * TileCountX + x;
								uint32_t wayValue = outLevel.wayLayer[tileIndex] > 0 ? ((outLevel.wayLayer[tileIndex] - outLevel.wayFirstGid) + 1) : 0;
								Tile tile = {};
								tile.wayType = TilesetWayToTypeMapping[wayValue];
								tile.entityType = EntityType::None;
								state.tiles[tileIndex] = tile;
							}
						}

						// Make waypoints/goal
						for(size_t objIndex = 0; objIndex < outLevel.objectCount; ++objIndex) {
							const ObjectData &obj = outLevel.objects[objIndex];
							if(IsValidTile(obj.tilePos)) {
								int tileIndex = obj.tilePos.y * TileCountX + obj.tilePos.x;
								switch(obj.type) {
									case ObjectType::Goal:
										state.tiles[tileIndex].entityType = EntityType::Goal;
										break;
									case ObjectType::Waypoint:
										AddWaypoint(state, obj.tilePos, obj.waypoint.direction);
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
		state.remainingEnemyCount = 0;
		state.enemyCount = 0;
		state.bulletCount = 0;
		state.spawnerCount = 0;
		for(size_t towerIndex = 0; towerIndex < state.towerCount; ++towerIndex) {
			state.towers[towerIndex].hasTarget = false;
			state.towers[towerIndex].targetEnemy = nullptr;
			state.towers[towerIndex].targetId = 0;
		}
	}

	static void ClearLevel(GameState &state) {
		gamelog::Verbose("Clear Level");
		state.towerCount = 0;
		state.selectedTowerIndex = -1;
		ClearWave(state);
		ClearWaypoints(state);
	}

	static void LoadWave(GameState &state, const int waveIndex) {
		const WaveData &wave = WaveDefinitions[waveIndex];

		state.waveState = WaveState::Stopped;

		gamelog::Verbose("Setup wave '%d'", waveIndex);

		if(state.activeLevelId == nullptr || strcmp(state.activeLevelId, wave.levelId) != 0) {
			gamelog::Verbose("Active level '%s' is different from '%s'", state.activeLevelId, wave.levelId);
			ClearLevel(state);
			char levelFilename[1024];
			fplCopyAnsiString(wave.levelId, levelFilename, FPL_ARRAYCOUNT(levelFilename));
			fplChangeFileExtension(levelFilename, ".tmx", levelFilename, FPL_ARRAYCOUNT(levelFilename));
			if(LoadLevel(state, state.assets.dataPath, levelFilename, state.levelData)) {
				fplCopyAnsiString(wave.levelId, state.activeLevelId, FPL_ARRAYCOUNT(state.activeLevelId));
			} else {
				gamelog::Error("Failed loading level '%s'!", levelFilename);
				return;
			}
			state.selectedTowerIndex = 0;
		}

		if(state.remainingEnemyCount > 0 || state.firstWaypoint != nullptr || state.spawnerCount > 0) {
			ClearWave(state);
		}

		if(fplGetAnsiStringLength(state.activeLevelId) == 0) {
			gamelog::Error("No level loaded!");
			return;
		}

		Vec2i goalTilePos = level::FindTilePosByEntityType(state, EntityType::Goal);
		if(!IsValidTile(goalTilePos)) {
			gamelog::Error("No goal entity in level '%s' found!", state.activeLevelId);
			return;
		}

		state.activeWaveIndex = waveIndex;
		state.remainingEnemyCount = 0;
		for(size_t objectIndex = 0; objectIndex < state.levelData.objectCount; ++objectIndex) {
			ObjectData &obj = state.levelData.objects[objectIndex];
			Vec2i objTilePos = obj.tilePos;
			if(!IsValidTile(objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Object '%zu:%s'!", objTilePos.x, objTilePos.y, objectIndex, ObjectTypeToString(obj.type));
				continue;
			}
			switch(obj.type) {
				case ObjectType::Spawn:
				{
					const SpawnData &data = obj.spawn;
					const CreepData *creepData = creeps::FindEnemyById(state, data.enemyId);
					if(creepData != nullptr) {
						CreepData enemyTemplate = *creepData;
						enemyTemplate.hp = (int)((float)enemyTemplate.hp * wave.enemyMultiplier.hp);
						enemyTemplate.bounty = (int)((float)enemyTemplate.bounty * wave.enemyMultiplier.bounty);
						enemyTemplate.speed *= wave.enemyMultiplier.speed;
						enemyTemplate.collisionRadius *= wave.enemyMultiplier.scale;
						creeps::AddSpawner(state, objTilePos, goalTilePos, data.initialCooldown, data.cooldown, data.enemyCount, enemyTemplate);
					} else {
						gamelog::Warning("Enemy by id '%s' does not exists!", data.enemyId);
					}
					state.remainingEnemyCount += data.enemyCount;
				} break;
			}
		}

		state.waveState = WaveState::Starting;
		state.waveCooldown = wave.startupCooldown;
	}
}

namespace towers {
	inline bool CanPlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *tower) {
		if((state.selectedTowerIndex < 0) || !(state.selectedTowerIndex < FPL_ARRAYCOUNT(TowerDefinitions))) {
			return false;
		}
		if(state.towerCount == FPL_ARRAYCOUNT(state.towers)) {
			return false;
		}
		Tile *tile = level::GetTile(state, tilePos);
		if(tile == nullptr) {
			return false;
		}
		if(tile->isOccupied || tile->entityType != EntityType::None || tile->wayType != WayType::None) {
			return false;
		}
		bool result = (state.money >= tower->costs);
		return(result);
	}

	static Tower *PlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *data) {
		assert(state.towerCount < FPL_ARRAYCOUNT(state.towers));
		Tower *tower = &state.towers[state.towerCount++];
		*tower = {};
		tower->data = data;
		tower->position = TileToWorld(tilePos, TileExt);
		tower->facingAngle = (float)M_PI * 0.5f; // Face north by default

		Tile *tile = level::GetTile(state, tilePos);
		assert(!tile->isOccupied);
		tile->isOccupied = true;

		state.money -= data->costs;

		return(tower);
	}

	static Vec2f PredictEnemyPosition(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f result;

		// First we compute how many frames we need until we can actually fire (Weapon cooldown)
		if(tower.data->enemyPredictionFlags != EnemyPredictionFlags::None) {
			float framesRequiredToFire;
			if((tower.data->enemyPredictionFlags & EnemyPredictionFlags::WeaponCooldown) == EnemyPredictionFlags::WeaponCooldown) {
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
			if((tower.data->enemyPredictionFlags & EnemyPredictionFlags::BulletDistance) == EnemyPredictionFlags::BulletDistance) {
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
		if(tower.data->enemyRangeTestType == FireRangeTestType::LineTrace) {
			float maxDistance = Vec2Length(distanceToEnemy) + enemy.data->collisionRadius;
			LineCastInput input = {};
			input.p1 = tower.position + lookDirection * tower.data->gunTubeLength;
			input.p2 = input.p1 + lookDirection * maxDistance;
			input.maxFraction = 1.0f;
			LineCastOutput output = {};
			result = LineCastCircle(input, enemy.position, enemy.data->collisionRadius, output);
		} else {
			float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
			if(projDistance > 0) {
				Vec2f lookPos = tower.position + lookDirection * projDistance;
				float dot = Vec2Dot(predictedEnemyPosition, lookPos);
				float det = predictedEnemyPosition.x * lookPos.y - predictedEnemyPosition.y * lookPos.x;
				float angle = ArcTan2(det, dot);
				result = angle >= -ShootAngleTolerance && angle <= ShootAngleTolerance;
			} else {
				result = false;
			}
		}
		return(result);
	}

	static void ShootBullet(GameState &state, Tower &tower) {
		assert(state.bulletCount < FPL_ARRAYCOUNT(state.bullets));
		Bullet *bullet = &state.bullets[state.bulletCount++];
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
		if(tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Vec2f distance = tower.targetEnemy->position - tower.position;
			assert(tower.data->unlockRadius >= tower.data->detectionRadius);
			if(tower.targetEnemy->isDead ||
				(tower.targetEnemy->id != tower.targetId) ||
			   (Vec2Length(distance) > tower.data->unlockRadius)) {
				tower.targetEnemy = nullptr;
				tower.hasTarget = false;
				tower.targetId = 0;
			}
		}

		// Detect a new target
		if(!tower.hasTarget) {
			float bestEnemyDistance = FLT_MAX;
			Creep *bestEnemy = nullptr;
			for(size_t enemyIndex = 0; enemyIndex < state.enemyCount; ++enemyIndex) {
				Creep *testEnemy = &state.enemies[enemyIndex];
				if(!testEnemy->isDead && testEnemy->id > 0) {
					float distanceRadius = Vec2Length(testEnemy->position - tower.position);
					if(distanceRadius < bestEnemyDistance) {
						bestEnemy = testEnemy;
						bestEnemyDistance = distanceRadius;
					}
				}
			}
			if(bestEnemy != nullptr && bestEnemyDistance <= tower.data->detectionRadius) {
				tower.targetEnemy = bestEnemy;
				tower.targetId = bestEnemy->id;
				tower.hasTarget = true;
			}
		}

		// Weapon cooldown
		if(!tower.canFire && tower.gunTimer > 0) {
			tower.gunTimer -= deltaTime;
		} else {
			tower.gunTimer = 0;
			tower.canFire = true;
		}

		//
		// Rotate gun
		//
		if(tower.hasTarget) {
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
		if(!tower.data->onlyFireOnLockedTarget) {
			for(size_t enemyIndex = 0; enemyIndex < state.enemyCount; ++enemyIndex) {
				Creep &enemy = state.enemies[enemyIndex];
				bool inFireRange = towers::InFireRange(tower, enemy, deltaTime);
				if(inFireRange && tower.canFire) {
					ShootBullet(state, tower);
				}
			}
		} else if(tower.hasTarget) {
			assert(tower.targetEnemy != nullptr);
			Creep *enemy = tower.targetEnemy;
			bool inFireRange = towers::InFireRange(tower, *enemy, deltaTime);
			if(inFireRange && tower.canFire) {
				ShootBullet(state, tower);
			}
		}
	}

	static void DrawTower(const Assets &assets, const Camera2D &camera, const TowerData &tower, const Vec2f &pos, const float angle) {
			// @TODO(final): Mulitple tower styles
		DrawPoint(camera, pos.x, pos.y, tower.structureRadius, V4f(1, 1, 0.5f, 1));

		glColor4f(1, 0.85f, 0.5f, 1);
		glLineWidth(camera.worldToPixels * tower.gunTubeThickness);
		glPushMatrix();
		glTranslatef(pos.x, pos.y, 0);
		glRotatef(angle * RadToDeg, 0, 0, 1);
		glBegin(GL_LINES);
		glVertex2f(tower.gunTubeLength, 0);
		glVertex2f(0, 0);
		glEnd();
		glLineWidth(2.0f);
		glPopMatrix();

		glColor4f(0.2f, 1, 0.2f, 0.25f);
		DrawSprite(assets.radiantTexture, tower.detectionRadius, tower.detectionRadius, 0.0f, 0.0f, 1.0f, 1.0f, pos.x, pos.y);
		glColor4f(1, 0.25f, 0.25f, 0.25f);
		DrawSprite(assets.radiantTexture, tower.unlockRadius, tower.unlockRadius, 0.0f, 0.0f, 1.0f, 1.0f, pos.x, pos.y);

		//DrawCircle(pos.x, pos.y, tower.detectionRadius, false, V4f(1, 1, 1, 0.5f));
		//DrawCircle(pos.x, pos.y, tower.unlockRadius, false, V4f(1, 0.5f, 1, 0.4f));
	}
}

namespace game {
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
		if(data != nullptr) {
			result = AllocateTexture(width, height, data, false, GL_LINEAR, false);
		}
		return(result);
	}

	static void LoadAssets(Assets &assets) {
		const char *fontFilename = "SulphurPoint-Bold.otf";
		char fontDataPath[1024];
		char texturesDataPath[1024];
		fplPathCombine(fontDataPath, FPL_ARRAYCOUNT(fontDataPath), 2, assets.dataPath, "fonts");
		fplPathCombine(texturesDataPath, FPL_ARRAYCOUNT(texturesDataPath), 2, assets.dataPath, "textures");
		if(LoadFontFromFile(fontDataPath, fontFilename, 0, 36.0f, 32, 127, 512, 512, &assets.hudFont.desc)) {
			assets.hudFont.texture = AllocateTexture(assets.hudFont.desc.atlasWidth, assets.hudFont.desc.atlasHeight, assets.hudFont.desc.atlasAlphaBitmap, false, GL_LINEAR, true);
		}
		if(LoadFontFromFile(fontDataPath, fontFilename, 0, 172.0f, 32, 127, 1024, 1024, &assets.overlayFont.desc)) {
			assets.overlayFont.texture = AllocateTexture(assets.overlayFont.desc.atlasWidth, assets.overlayFont.desc.atlasHeight, assets.overlayFont.desc.atlasAlphaBitmap, false, GL_LINEAR, true);
		}
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

		// Initial values from first wave?
		state.money = 100;
		state.lifes = 10;

		// Load initial wave
		level::LoadWave(state, 0);
	}

	static bool InitGame(GameState &state, GameMemory &gameMemory) {
		gamelog::Verbose("Initialize Game");

		if(!fglLoadOpenGL(true)) {
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
		glLineWidth(2.0f);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		state.mem = gameMemory;

		LoadAssets(state.assets);

		NewGame(state);

		return(true);
	}
}

extern GameMemory GameCreate() {
	gamelog::Verbose("Create Game");
	GameMemory result = {};
	result.capacity = sizeof(GameState) + FPL_MEGABYTES(16);
	result.base = fplMemoryAllocate(result.capacity);
	if(result.base == nullptr) {
		gamelog::Fatal("Failed allocating Game State memory of '%zu' bytes!", result.capacity);
		return {};
	}
	GameState *state = (GameState *)result.base;
	result.used = sizeof(GameState);
	if(!game::InitGame(*state, result)) {
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

extern void GameInput(GameMemory &gameMemory, const Input &input, bool isActive) {
	if(!isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;

	// Debug input
	const Controller &keyboardController = input.controllers[0];
	if(WasPressed(keyboardController.debugToggle)) {
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

	if(state->waveState == WaveState::Running || state->waveState == WaveState::Starting) {
		state->mouseTilePos = WorldToTile(state->mouseWorldPos);

		// Tower placement
		if(WasPressed(input.mouse.left)) {
			if(state->selectedTowerIndex > -1) {
				const TowerData *tower = &TowerDefinitions[state->selectedTowerIndex];
				if(towers::CanPlaceTower(*state, state->mouseTilePos, tower)) {
					towers::PlaceTower(*state, state->mouseTilePos, tower);
				}
			}
		}
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input, bool isActive) {
	if(!isActive) {
		return;
	}

#if 1
	GameState *state = (GameState *)gameMemory.base;

	// Startup wave
	if(state->waveState == WaveState::Starting) {
		state->waveCooldown -= input.deltaTime;
		if(state->waveCooldown <= 0.0f) {
			state->waveCooldown = 0;
			state->waveState = WaveState::Running;
		}
	}

	if(state->waveState == WaveState::Running) {
		//
		// Move enemies
		//
		for(size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
			Creep &enemy = state->enemies[enemyIndex];
			if(!enemy.isDead && enemy.hasTarget) {
				Vec2f distance = enemy.targetPos - enemy.position;
				Vec2f direction = Vec2Normalize(distance);
				float minRadius = MaxTileSize * 0.05f;
				enemy.position += direction * enemy.speed * input.deltaTime;
				if(Vec2Dot(distance, distance) <= minRadius * minRadius) {
					creeps::SetCreepNextTarget(*state, enemy);
				}
			}
		}

		// Update spawners
		for(size_t spawnerIndex = 0; spawnerIndex < state->spawnerCount; ++spawnerIndex) {
			CreepSpawner &spawner = state->spawners[spawnerIndex];
			creeps::UpdateSpawner(*state, spawner, input.deltaTime);
		}

		// Update towers
		for(size_t towerIndex = 0; towerIndex < state->towerCount; ++towerIndex) {
			Tower &tower = state->towers[towerIndex];
			towers::UpdateTower(*state, tower, input.deltaTime);
		}

		//
		// Move and collide bullets
		//
		for(size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
			Bullet &bullet = state->bullets[bulletIndex];
			if(!bullet.isDestroyed) {
				bullet.position += bullet.velocity * input.deltaTime;
				if(!bullet.hasHit) {
					for(size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
						Creep &enemy = state->enemies[enemyIndex];
						if(!enemy.isDead) {
							Vec2f distance = enemy.position - bullet.position;
							float bothRadi = bullet.data->collisionRadius + enemy.data->collisionRadius;
							float d = Vec2Dot(distance, distance);
							if(d < bothRadi) {
								bullet.hasHit = true;
								creeps::CreepHit(*state, enemy, bullet);
								break;
							}
						}
					}
				}
			}
		}

		//
		// Remove dead enemies and destroyed bullets
		//
		for(size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
			Bullet &bullet = state->bullets[bulletIndex];
			if(bullet.hasHit) {
				bullet.isDestroyed = true;
			}
			if(bullet.isDestroyed) {
				if(bulletIndex < state->bulletCount - 1) {
					const Bullet &lastBullet = state->bullets[state->bulletCount - 1];
					bullet = lastBullet;
				}
				--state->bulletCount;
			}
		}
		for(size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
			Creep &enemy = state->enemies[enemyIndex];
			if(enemy.isDead) {
				enemy.id = 0;
				if(enemyIndex < state->enemyCount - 1) {
					const Creep &lastEnemy = state->enemies[state->enemyCount - 1];
					enemy = lastEnemy;
				}
				--state->enemyCount;
				--state->remainingEnemyCount;
			}
		}
		if(state->remainingEnemyCount == 0) {
			creeps::AllEnemiesKilled(*state);
		}
	}
#endif
}

static void DrawHUD(GameState &state) {
	constexpr float hudPadding = MaxTileSize * 0.075f;
	constexpr float hudOriginX = -WorldRadiusW;
	constexpr float hudOriginY = WorldRadiusH;
	constexpr float hudFontHeight = TileHeight * 0.4f;
	constexpr float outlineOffset = hudFontHeight * 0.05f;
	const Vec4f textBackColor = V4f(0.2f, 0.2f, 0.8f, 1);
	const Vec4f textForeColor = V4f(1, 1, 1, 1);
	const FontAsset &font = state.assets.hudFont;
	{
		char text[256];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%s", state.activeLevelId);
		Vec2f textPos = V2f(hudOriginX + WorldRadiusW, hudOriginY - hudPadding - hudFontHeight * 0.5f);
		glColor4fv(&textBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 0.0f, 0.0f);
		glColor4fv(&textForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 0.0f, 0.0f);

		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Wave: %d / %zu", (state.activeWaveIndex + 1), FPL_ARRAYCOUNT(WaveDefinitions));
		textPos.y -= hudFontHeight;
		glColor4fv(&textBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 0.0f, 0.0f);
		glColor4fv(&textForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 0.0f, 0.0f);
	}
	{
		char text[256];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "$: %d", state.money);
		Vec2f textPos = V2f(hudOriginX + hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
		glColor4fv(&textBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, 1.0f, 0.0f);
		glColor4fv(&textForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, 1.0f, 0.0f);
	}
	{
		char text[256];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "HP: %d", state.lifes);
		Vec2f textPos = V2f(hudOriginX + WorldWidth - hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
		glColor4fv(&textBackColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x + outlineOffset, textPos.y - outlineOffset, hudFontHeight, -1.0f, 0.0f);
		glColor4fv(&textForeColor.m[0]);
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, hudFontHeight, -1.0f, 0.0f);
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
	glLineWidth(2);
}

extern void GameRender(GameMemory &gameMemory, const float alpha, const float deltaTime) {
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
	DrawPoint(state->camera, state->mouseWorldPos.x, state->mouseWorldPos.y, MaxTileSize * 0.25f, V4f(1, 1, 1, 1));

	Vec2f towerPos = V2f(0, 0);
	float towerRadius = MaxTileSize * 0.4f;
	float gunLength = MaxTileSize;
	static float faceAngle = -0.1f;

	Vec2f distance = state->mouseWorldPos - towerPos;
	Vec2f dirToEnemy = Vec2Normalize(distance);
	float targetAngle = Vec2AxisToAngle(dirToEnemy);

	Vec2f faceDir = Vec2AngleToAxis(faceAngle);

	char textBuffer[256];
	float textSize = MaxTileSize;
	fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "%f", targetAngle);
	glColor4f(1, 1, 1, 1);
	DrawTextFont(textBuffer, strlen(textBuffer), &state->assets.hudFont.desc, state->assets.hudFont.texture, -WorldRadiusW, WorldRadiusH - textSize, textSize, 1.0f, -1.0f);
	fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "%f", faceAngle);
	glColor4f(1, 1, 1, 1);

	float m = GetShortestAngleDist(faceAngle, targetAngle);
	DrawTextFont(textBuffer, strlen(textBuffer), &state->assets.hudFont.desc, state->assets.hudFont.texture, -WorldRadiusW, WorldRadiusH - textSize * 2.0f, textSize, 1.0f, -1.0f);
	fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "%f", m);
	glColor4f(1, 1, 1, 1);
	DrawTextFont(textBuffer, strlen(textBuffer), &state->assets.hudFont.desc, state->assets.hudFont.texture, -WorldRadiusW, WorldRadiusH - textSize * 3.0f, textSize, 1.0f, -1.0f);

	float deltaAngle = m;

	faceAngle += deltaAngle * deltaTime;

	DrawPoint(state->camera, towerPos.x, towerPos.y, towerRadius, V4f(0, 0, 1, 1));
	glColor4f(1, 0, 0, 1);
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex2f(towerPos.x, towerPos.y);
	glVertex2f(towerPos.x + faceDir.x * gunLength, towerPos.y + faceDir.y * gunLength);
	glEnd();
	glLineWidth(2);

	glColor4f(0, 1, 0, 1);
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex2f(towerPos.x, towerPos.y);
	glVertex2f(towerPos.x + dirToEnemy.x * gunLength, towerPos.y + dirToEnemy.y * gunLength);
	glEnd();
	glLineWidth(2);
#endif

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
	for(int y = 0; y < TileCountY; ++y) {
		for(int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->tiles[y * TileCountX + x];
			if(tile.wayType != WayType::None) {
				// @TODO(final): Draw sprite for this way tile
				render::DrawTile(x, y, true, V4f(0.0f, 0.0f, 1.0f, 1.0f));
			}
		}
	}

	// Draw tile entities
	for(int y = 0; y < TileCountY; ++y) {
		for(int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->tiles[y * TileCountX + x];
			switch(tile.entityType) {
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
	for(size_t spawnerIndex = 0; spawnerIndex < state->spawnerCount; ++spawnerIndex) {
		const CreepSpawner &spawner = state->spawners[spawnerIndex];
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
	for(int y = 0; y <= TileCountY; ++y) {
		glVertex2f(GridOriginX, GridOriginY + y * TileHeight);
		glVertex2f(GridOriginX + TileCountX * TileWidth, GridOriginY + y * TileHeight);
	}
	for(int x = 0; x <= TileCountX; ++x) {
		glVertex2f(GridOriginX + x * TileWidth, GridOriginY);
		glVertex2f(GridOriginX + x * TileWidth, GridOriginY + TileCountY * TileHeight);
	}
	glEnd();
#endif

	if(state->isDebugRendering) {
		// Waypoints
		for(Waypoint *waypoint = state->firstWaypoint; waypoint; waypoint = waypoint->next) {
			DrawPoint(state->camera, waypoint->position.x, waypoint->position.y, MaxTileSize * 0.15f, V4f(1, 0, 1, 1));
			DrawNormal(waypoint->position, waypoint->direction, level::WaypointDirectionWidth, V4f(1, 1, 1, 1));
		}
	}

	// Hover tile
	if(state->selectedTowerIndex > -1 && IsValidTile(state->mouseTilePos)) {
		const TowerData *tower = &TowerDefinitions[state->selectedTowerIndex];
		Vec4f hoverColor;
		if(towers::CanPlaceTower(*state, state->mouseTilePos, tower))
			hoverColor = V4f(0.1f, 1.0f, 0.1f, 1.0f);
		else
			hoverColor = V4f(1.0f, 0.1f, 0.1f, 1.0f);
		render::DrawTile(state->mouseTilePos.x, state->mouseTilePos.y, false, hoverColor);
	}

	//
	// Enemies
	//
	for(size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if(!enemy.isDead) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);

			// Mesh
			switch(enemy.data->style) {
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
				glLineWidth(2);
			}

			enemy.prevPosition = enemy.position;
		}
	}

	//
	// Towers
	//
	for(size_t towerIndex = 0; towerIndex < state->towerCount; ++towerIndex) {
		const Tower &tower = state->towers[towerIndex];
		towers::DrawTower(state->assets, state->camera, *tower.data, tower.position, tower.facingAngle);

		if(state->isDebugRendering) {
			if(tower.hasTarget) {
				const Creep *target = tower.targetEnemy;
				if(target->id == tower.targetId) {
					DrawCircle(target->position.x, target->position.y, target->data->collisionRadius, false, V4f(1, 0, 0, 1), 32);

					Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
					Vec2f distanceToEnemy = target->position - tower.position;
					float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
					Vec2f lookPos = tower.position + lookDirection * projDistance;

					DrawCircle(lookPos.x, lookPos.y, MaxTileSize * 0.25f, false, V4f(1, 1, 0, 1), 16);

					float dot = Vec2Dot(target->position, lookPos);
					float det = Vec2Cross(target->position, lookPos);
					float angle = ArcTan2(det, dot);

					if(angle >= -ShootAngleTolerance && angle <= ShootAngleTolerance) {
						Vec2f sightPos1 = tower.position + Vec2AngleToAxis(tower.facingAngle - ShootAngleTolerance) * projDistance;
						Vec2f sightPos2 = tower.position + Vec2AngleToAxis(tower.facingAngle + ShootAngleTolerance) * projDistance;
						glColor4f(1, 0, 0, 0.5);
						glLineWidth(1);
						glBegin(GL_LINES);
						glVertex2f(tower.position.x, tower.position.y);
						glVertex2f(sightPos1.x, sightPos1.y);
						glVertex2f(tower.position.x, tower.position.y);
						glVertex2f(sightPos2.x, sightPos2.y);
						glEnd();
						glLineWidth(2);
					}
				}
			}
		}
	}

	//
	// Bullets
	//
	for(size_t bulletIndex = 0; bulletIndex < state->bulletCount; ++bulletIndex) {
		Bullet &bullet = state->bullets[bulletIndex];
		if(!bullet.isDestroyed) {
			Vec2f bulletPos = Vec2Lerp(bullet.prevPosition, alpha, bullet.position);
			// @TODO(final): Use sprites for bullets
			DrawPoint(state->camera, bulletPos.x, bulletPos.y, bullet.data->renderRadius, V4f(1, 0, 0, 1));
			bullet.prevPosition = bullet.position;
		}
	}

	//
	// Overlay
	//
	if(state->waveState == WaveState::Starting) {
		const FontAsset &font = state->assets.overlayFont;
		char text[128];
		fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "%d", (int)ceilf(state->waveCooldown));
		glColor4f(1, 1, 1, 1);
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.25f;
		DrawTextFont(text, fplGetAnsiStringLength(text), &font.desc, font.texture, textPos.x, textPos.y, overlayFontHeight, 0.0f, 0.0f);
	}

	//
	// HUD & Controls
	//
	DrawHUD(*state);
	DrawControls(*state);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = "FPL Demo | Towadev";
	config.disableInactiveDetection = true;
	gamelog::Verbose("Startup game application '%s'", config.title);
	int result = GameMain(config);
	return(result);
}