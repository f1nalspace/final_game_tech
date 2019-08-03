/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Towadev

Description:
	A tower defence clone.
	Levels are loaded from .TMX files (Tiled-Editor).
	All data (Waves, Enemies, Towers) are loaded from xml files.
	Written in C++ (C-Style).

Requirements:
	- C++ Compiler
	- Final XML
	- Final Framework

Author:
	Torsten Spaete

Changelog:
	## 2019-04-27
	- Use Vec2Normalize instead of dividing by length

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-08-06
	- Do not use fplMemoryAllocate anymore, use fmemMemoryBlocks from game memory
	- Removed enemy prediction flags
	- Enemy prediction is now based on trajectory computation
	- Support for user-reload towers/creeps/waves data definitions

	## 2018-08-01
	- Introduced render parts in tower data

	## 2018-07-06
	- Level size can now be of any size

	## 2018-07-05
	- Corrected for api change in final_game.h
	- Corrected for api change in final_render.h
	- Migrated to new render system and removed all opengl calls

	## 2018-07-03
	- Fixed collision was broken
	- Fixed spawner was active while start-cooldown of wave
	- Fixed enemy prediction was broken

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
	- Manual reload of XMLs and update all data dynamically

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FXML_IMPLEMENTATION
#include <final_xml.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FINAL_ASSETS_IMPLEMENTATION
#include <final_assets.h>

#include <final_game.h>

#include "fpl_towadev.h"

constexpr float ShotAngleTolerance = (Pi32 * 0.05f);

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
		fplFormatStringArgs(msg, fplArrayCount(msg), format, argList);
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
	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type);
	static void LoadWave(GameState &state, const int waveIndex);
}
namespace game {
	static void NewGame(GameState &state);
	static void SetSlowdown(GameState &state, const float duration, const WaveState nextState);
}

namespace ui {
	static void UIBegin(UIContext &ctx, GameState *gameState, RenderState *renderState, const Input &input, const Vec2f &mousePos) {
		ctx.input = {};
		ctx.hot = 0;
		ctx.gameState = gameState;
		ctx.renderState = renderState;
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
	typedef void(UIButtonDrawFunction)(GameState &gameState, RenderState &renderState, const Vec2f &pos, const Vec2f &radius, const UIButtonState buttonState, void *userData);

	static bool UIButton(UIContext &ctx, const UIID &id, const Vec2f &pos, const Vec2f &radius, UIButtonDrawFunction *drawFunc, void *userData) {
		bool result = false;
		if(IsInsideButton(ctx, pos, radius)) {
			ctx.hot = id;
		}
		if(ctx.active == id) {
			if(WasPressed(ctx.input.leftButton)) {
				if(ctx.hot == id) {
					result = true;
				}
				ctx.active = 0;
			}
		} else if(ctx.hot == id) {
			if(IsDown(ctx.input.leftButton)) {
				ctx.active = id;
			}
		}

		UIButtonState buttonState = UIButtonState::None;
		if(ctx.hot == id) {
			if(ctx.active == ctx.hot) {
				buttonState = UIButtonState::Down;
			} else {
				buttonState = UIButtonState::Hover;
			}
		}

		drawFunc(*ctx.gameState, *ctx.renderState, pos, radius, buttonState, userData);

		return(result);
	}
}

namespace utils {
	static int StringToInt(const char *str, const int def = 0) {
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

	// https://stackoverflow.com/questions/4392665/converting-string-to-float-without-atof-in-c
	static void StringToFloatArray(const char *str, const size_t maxLen, float *outArray) {
		if(str != nullptr) {
			const char *p = str;
			size_t index = 0;
			while(*p) {
				while(isspace(*p)) {
					++p;
				}
				float r = 0.0f;
				float f = 1.0f;
				if(*p == '-') {
					++p;
					f = -1;
				}
				for(int pointSeen = 0; *p && *p != ','; p++) {
					if(*p == '.') {
						pointSeen = 1;
						continue;
					}
					int d = *p - '0';
					if(d >= 0 && d <= 9) {
						if(pointSeen) {
							f *= 0.1f;
						}
						r = r * 10.0f + (float)d;
					}
				}
				float value = r * f;
				if(index < maxLen) {
					outArray[index] = value;
				} else {
					break;
				}
				if(*p == ',') {
					++index;
				} else if(!*p) {
					break;
				}
				++p;
			}
		}
	}

	static float StringToFloat(const char *str, const float def = 0.0f) {
		float result = def;
		StringToFloatArray(str, 1, &result);
		return(result);
	}

	static Vec2f StringToVec2(const char *str, const Vec2f def = V2f(0, 0)) {
		Vec2f result = def;
		StringToFloatArray(str, 2, &result.m[0]);
		return(result);
	}

	static Vec4f StringToVec4(const char *str, const Vec4f def = V4f(0, 0, 0, 0)) {
		Vec4f result = def;
		StringToFloatArray(str, 4, &result.m[0]);
		return(result);
	}

	static FileContents LoadEntireFile(const char *filePath, fmemMemoryBlock *memory) {
		FileContents result = {};
		fplFileHandle file;
		if(fplOpenBinaryFile(filePath, &file)) {
			result.info.size = fplGetFileSizeFromHandle32(&file);
			fplFileTimeStamps timestamps = {};
			fplGetFileTimestampsFromHandle(&file, &timestamps);
			result.info.modifyDate = timestamps.lastModifyTime;
			result.data = (uint8_t *)fmemPush(memory, result.info.size, fmemPushFlags_None);
			fplReadFileBlock32(&file, (uint32_t)result.info.size, result.data, (uint32_t)result.info.size);
			fplCloseFile(&file);
		}
		return(result);
	}

	static FileInfo LoadFileInfo(const char *filePath) {
		FileInfo result = {};
		fplFileHandle file;
		if(fplOpenBinaryFile(filePath, &file)) {
			result.size = fplGetFileSizeFromHandle32(&file);
			fplFileTimeStamps timestamps = {};
			fplGetFileTimestampsFromHandle(&file, &timestamps);
			result.modifyDate = timestamps.lastModifyTime;
			fplCloseFile(&file);
		}
		return(result);
	}

	static bool IsEqualFileInfo(const FileInfo &a, const FileInfo &b) {
		bool result = (a.size == b.size) && (a.modifyDate == b.modifyDate);
		return(result);
	}
}

namespace render {
	static void DrawTile(RenderState &renderState, const LevelDimension &dim, const int x, const int y, const bool isFilled, const Vec4f &color) {
		Vec2f pos = TileToWorld(dim, V2i(x, y));
		PushRectangle(renderState, pos, V2f(TileWidth, TileHeight), color, isFilled, 1.0f);
	}

	static void DrawLineStipple(RenderState &renderState, const Vec2f &a, const Vec2f &b, const float stippleWidth, const int modCount, const Vec4f &color, const float lineWidth) {
		assert(stippleWidth > 0);
		Vec2f ab = b - a;
		float d = Vec2Length(ab);
		Vec2f n = Vec2Normalize(ab);
		int secCount = (d > stippleWidth) ? (int)(d / stippleWidth) : 1;
		assert(secCount > 0);
		size_t capacity = secCount * 2;
		VertexAllocation vertAlloc = AllocateVertices(renderState, capacity, color, DrawMode::Lines, false, lineWidth);
		Vec2f *p = vertAlloc.verts;
		size_t count = 0;
		for(int sec = 0; sec < secCount; ++sec) {
			float t = sec / (float)secCount;
			Vec2f start = Vec2Lerp(a, t, b);
			Vec2f end = start + n * stippleWidth;
			if(sec % modCount == 0) {
				*p++ = start;
				*p++ = end;
				count += 2;
			}
		}
		assert(count <= capacity);
		*vertAlloc.count = count;
	}

	static void DrawLineLoopStipple(RenderState &renderState, const Vec2f *points, const size_t pointCount, const float stippleWidth, const int modCount, const Vec4f &color, const float lineWidth) {
		assert(pointCount >= 2);
		for(size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
			Vec2f a = points[pointIndex];
			Vec2f b = points[(pointIndex + 1) % pointCount];
			DrawLineStipple(renderState, a, b, stippleWidth, modCount, color, lineWidth);
		}
	}

	static void DrawParts(RenderState &renderState, const Assets &assets, const Camera2D &camera, const Vec2f &center, const float scale, const float alpha, const float rotation, const size_t partCount, const PartData *parts) {
		Mat2f rotationMat = Mat2FromAngle(rotation);
		for(size_t partIndex = 0; partIndex < partCount; ++partIndex) {
			const PartData *part = parts + partIndex;
			Vec4f partColor = V4f(part->color.r, part->color.g, part->color.b, part->color.a * alpha);
			Vec2f partPosition;
			if((part->rotFlags & PartRotationFlags::ApplyToOffset) == PartRotationFlags::ApplyToOffset) {
				partPosition = center + Vec2MultMat2(rotationMat, part->offset) * scale;
			} else {
				partPosition = center + part->offset * scale;
			}
			float partLineWidth = camera.worldToPixels * part->lineWidth * scale;
			Mat4f r = Mat4RotationZ(part->orientation);
			if((part->rotFlags & PartRotationFlags::ApplyToTransform) == PartRotationFlags::ApplyToTransform) {
				r = r * Mat4RotationZ(rotationMat);
			}
			Mat4f m = Mat4Translation(partPosition) * r;
			PushMatrix(renderState, m);
			switch(part->type) {
				case PartType::FillCircle:
				case PartType::StrokeCircle:
				{
					bool isFilled = part->type == PartType::FillCircle;
					PushCircle(renderState, V2f(0.0f, 0.0f), part->radius * scale, 16, partColor, isFilled, partLineWidth);
				} break;
				case PartType::FillQuad:
				case PartType::StrokeQuad:
				{
					bool isFilled = part->type == PartType::FillQuad;
					PushRectangleCenter(renderState, V2f(0.0f, 0.0f), part->ext * scale, partColor, isFilled, partLineWidth);
				} break;
				case PartType::Line:
				{
					PushLine(renderState, V2f(part->ext.x, 0) * scale, V2f(-part->ext.x, 0) * scale, partColor, partLineWidth);
				} break;
			}
			PopMatrix(renderState);
		}
	}
}

namespace creeps {
	static void SpawnEnemy(Creeps &enemies, const LevelDimension &dim, const Waypoints &waypoints, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData *data) {
		assert(enemies.count < fplArrayCount(enemies.list));
		Creep *enemy = &enemies.list[enemies.count++];
		fplClearStruct(enemy);
		enemy->id = ++enemies.creepIdCounter;
		enemy->data = data;
		enemy->position = enemy->prevPosition = spawnPos;
		enemy->speed = data->speed;
		enemy->hp = data->hp;
		if(waypoints.first != nullptr) {
			enemy->targetWaypoint = waypoints.first;
			enemy->targetPos = TileToWorld(dim, waypoints.first->tilePos, TileExt);
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
				SpawnEnemy(state.enemies, state.level.dimension, state.waypoints, spawner.spawnPosition, spawner.exitPosition, spawner.spawnTemplate);
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

	static void AddSpawner(CreepSpawners &spawners, const LevelDimension &dim, const Vec2i &spawnTilePos, const Vec2i &goalTilePos, const float initialCooldown, const float cooldown, const size_t count, const SpawnerStartMode startMode, const CreepData *spawnTemplate) {
		assert(spawners.count < fplArrayCount(spawners.list));
		CreepSpawner &spawner = spawners.list[spawners.count++];
		spawner = {};
		spawner.spawnPosition = TileToWorld(dim, spawnTilePos, TileExt);
		spawner.exitPosition = TileToWorld(dim, goalTilePos, TileExt);
		spawner.cooldown = cooldown;
		spawner.spawnTimer = initialCooldown;
		spawner.totalCount = count;
		spawner.remainingCount = count;
		spawner.spawnTemplate = spawnTemplate;
		spawner.isActive = false;
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
		if(state.wave.isActive && state.stats.lifes <= 0) {
			state.stats.lifes = 0;
			state.wave.isActive = false;
			game::SetSlowdown(state, 6.0f, WaveState::Lost);
		}
	}

	static void SetCreepNextTarget(GameState &state, Creep &enemy) {
		const LevelDimension &dim = state.level.dimension;
		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		assert(goalTilePos.x > -1 && goalTilePos.y > -1);
		Vec2i creepTilePos = WorldToTile(dim, enemy.position);
		if(enemy.targetWaypoint != nullptr) {
			const Waypoint waypoint = *enemy.targetWaypoint;
			assert(Vec2Length(waypoint.direction) == 1);
			Vec2f creepDir = waypoint.direction;
			if(waypoint.next != nullptr) {
				enemy.targetPos = TileToWorld(dim, waypoint.next->tilePos, TileExt);
				enemy.targetWaypoint = waypoint.next;
			} else {
				enemy.targetWaypoint = nullptr;
				enemy.targetPos = TileToWorld(dim, goalTilePos, TileExt);
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
		for(size_t i = 0; i < state.assets.creepDefinitionCount; ++i) {
			if(strcmp(state.assets.creepDefinitions[i].id, id) == 0) {
				return &state.assets.creepDefinitions[i];
			}
		}
		return nullptr;
	}

	static void CreepHit(GameState &state, Creep &enemy, const Bullet &bullet) {
		enemy.hp -= bullet.data->damage;
		if(enemy.hp <= 0) {
			CreepDead(state, enemy);
			state.stats.money += enemy.data->bounty;
		}
	}

	static void AllEnemiesKilled(GameState &state) {
		state.stats.money += state.assets.waveDefinitions[state.wave.activeIndex].completionBounty;
		if(state.wave.activeIndex < ((int)state.assets.waveDefinitionCount - 1)) {
			level::LoadWave(state, state.wave.activeIndex + 1);
		} else {
			state.wave.state = WaveState::Won;
			state.wave.isActive = false;
			game::SetSlowdown(state, 6.0f, WaveState::Won);
		}
	}
}

namespace level {
	inline Tile *GetTile(Level &level, const Vec2i &tilePos) {
		if(IsValidTile(level.dimension, tilePos)) {
			int index = tilePos.y * (int)level.dimension.tileCountX + tilePos.x;
			return &level.tiles[index];
		}
		return nullptr;
	}

	static Vec2i FindTilePosByEntityType(const Level &level, const EntityType type) {
		for(size_t y = 0; y < level.dimension.tileCountY; ++y) {
			for(size_t x = 0; x < level.dimension.tileCountX; ++x) {
				size_t index = y * level.dimension.tileCountX + x;
				if(level.tiles[index].entityType == type) {
					return V2i((int)x, (int)y);
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

	static Waypoint *AddWaypoint(Waypoints &waypoints, const LevelDimension &dim, const Vec2i &tilePos, const Vec2f &dir) {
		assert(waypoints.used < fplArrayCount(waypoints.freeList));
		Waypoint *waypoint = &waypoints.freeList[waypoints.used++];
		waypoint->tilePos = tilePos;
		waypoint->position = TileToWorld(dim, tilePos, TileExt);
		waypoint->direction = dir;
		if(waypoints.first == nullptr) {
			waypoints.first = waypoints.last = waypoint;
		} else {
			waypoints.last->next = waypoint;
			waypoints.last = waypoint;
		}
		return(waypoint);
	}

	static void ParseLevelLayer(fxmlTag *childTag, LevelLayer *targetLayer, fmemMemoryBlock *memory) {
		const char *layerName = fxmlGetAttributeValue(childTag, "name");
		fplCopyString(layerName, targetLayer->name, fplArrayCount(targetLayer->name));
		targetLayer->mapWidth = utils::StringToInt(fxmlGetAttributeValue(childTag, "width"));
		targetLayer->mapHeight = utils::StringToInt(fxmlGetAttributeValue(childTag, "height"));
		targetLayer->data = (uint32_t *)fmemPush(memory, sizeof(uint32_t) * targetLayer->mapWidth * targetLayer->mapHeight, fmemPushFlags_Clear);
		targetLayer->opacity = utils::StringToFloat(fxmlGetAttributeValue(childTag, "opacity"), 1.0f);
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
						int row = index / (int)targetLayer->mapWidth;
						int col = index % (int)targetLayer->mapWidth;
						assert(row >= 0 && row < (int)targetLayer->mapHeight);
						assert(col >= 0 && col < (int)targetLayer->mapWidth);
						int tileIndex = row * targetLayer->mapWidth + col;
						targetLayer->data[tileIndex] = tileValue;
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

	static LevelTileset *FindLevelTileset(LevelData &level, const char *name) {
		for(size_t i = 0; i < level.tilesetCount; ++i) {
			if(fplIsStringEqual(level.tilesets[i].name, name)) {
				return &level.tilesets[i];
			}
		}
		return nullptr;
	}

	static void ParseLevelObjects(fxmlTag *objectGroupTag, LevelData &level, fmemMemoryBlock *memory) {
		assert(level.tileWidth > 0);
		assert(level.tileHeight > 0);
		LevelTileset *entitiesTileset = FindLevelTileset(level, "entities");
		assert(entitiesTileset != nullptr);
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

					uint32_t tileId = (gid > 0) ? ((gid - entitiesTileset->firstGid) + 1) : 0;
					EntityType entityType = TilesetEntitiesToTypeMapping[tileId];

					ObjectData tmpObj = {};
					tmpObj.tilePos = tilePos;
					const char *typeName = fxmlGetAttributeValue(childTag, "type");
					const char *objName = fxmlGetAttributeValue(childTag, "name");
					if(strcmp(typeName, "Spawn") == 0) {
						tmpObj.type = ObjectType::Spawn;
						fplCopyString(objName, tmpObj.spawn.spawnId, fplArrayCount(tmpObj.spawn.spawnId));
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
						assert(level.objectCount < fplArrayCount(level.objects));
						ObjectData &obj = level.objects[level.objectCount++];
						obj = tmpObj;
					}
				}
			}
			childTag = childTag->nextSibling;
		}
	}

	static bool ParseTileset(fxmlTag *tilesetTag, LevelTileset &outTileset, fmemMemoryBlock *memory) {
		const char *name = fxmlGetAttributeValue(tilesetTag, "name");
		fplCopyString(name, outTileset.name, fplArrayCount(outTileset.name));
		outTileset.firstGid = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "firstgid"));
		outTileset.tileWidth = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tilewidth"));
		outTileset.tileHeight = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tileheight"));
		outTileset.tileCount = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "tilecount"));
		outTileset.columns = utils::StringToInt(fxmlGetAttributeValue(tilesetTag, "columns"));
		outTileset.tileUVs = nullptr;
		fxmlTag *imageTag = fxmlFindTagByName(tilesetTag, "image");
		if(imageTag != nullptr) {
			const char *source = fxmlGetAttributeValue(imageTag, "source");
			fplCopyString(source, outTileset.image.source, fplArrayCount(outTileset.image.source));
			outTileset.image.width = utils::StringToInt(fxmlGetAttributeValue(imageTag, "width"));
			outTileset.image.height = utils::StringToInt(fxmlGetAttributeValue(imageTag, "height"));
		}
		if((outTileset.tileCount > 0 && outTileset.columns > 0) &&
			(outTileset.image.width > 0 && outTileset.image.height > 0) &&
		   (outTileset.tileWidth > 0 && outTileset.tileHeight > 0)) {
			outTileset.tileUVs = (UVRect *)fmemPush(memory, sizeof(UVRect) * outTileset.tileCount, fmemPushFlags_Clear);
			Vec2i tileSize = V2i(outTileset.tileWidth, outTileset.tileHeight);
			Vec2i imageSize = V2i(outTileset.image.width, outTileset.image.height);
			UVRect *p = outTileset.tileUVs;
			int rowCount = (int)outTileset.tileCount / outTileset.columns;
			for(size_t tileIndex = 0; tileIndex < outTileset.tileCount; ++tileIndex) {
				int tileY = (int)(tileIndex / outTileset.columns);
				int tileX = (int)(tileIndex % outTileset.columns);
				*p++ = UVRectFromTile(imageSize, tileSize, 0, V2i(tileX, rowCount - 1 - tileY));
			}
		}
		return(true);
	}

	static bool ParseLevel(fxmlTag *root, LevelData &level, fmemMemoryBlock *memory) {
		bool result = false;
		fxmlTag *mapTag = fxmlFindTagByName(root, "map");
		if(mapTag == nullptr) {
			return false;
		}
		level.mapWidth = utils::StringToInt(fxmlGetAttributeValue(mapTag, "width"));
		level.mapHeight = utils::StringToInt(fxmlGetAttributeValue(mapTag, "height"));

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

		level.tilesetCount = 0;
		level.layerCount = 0;
		level.objectCount = 0;

		fxmlTag *childTag = mapTag->firstChild;
		while(childTag) {
			if(childTag->type == fxmlTagType_Element) {
				if(strcmp(childTag->name, "tileset") == 0) {
					assert(level.tilesetCount < fplArrayCount(level.tilesets));
					LevelTileset *targetTileset = &level.tilesets[level.tilesetCount++];
					ParseTileset(childTag, *targetTileset, memory);
				} else if(strcmp(childTag->name, "layer") == 0) {
					assert(level.layerCount < MAX_LAYER_COUNT);
					LevelLayer *targetLayer = &level.layers[level.layerCount++];
					ParseLevelLayer(childTag, targetLayer, memory);
				} else if(strcmp(childTag->name, "objectgroup") == 0) {
					const char *objectGroupName = fxmlGetAttributeValue(childTag, "name");
					if(strcmp(objectGroupName, "objects") == 0) {
						ParseLevelObjects(childTag, level, memory);
					}
				}
			}
			childTag = childTag->nextSibling;
		}

		return(true);
	}

	static const char *GetNodeValue(fxmlTag *rootTag, const char *nodeName) {
		fxmlTag *foundTag = fxmlFindTagByName(rootTag, nodeName);
		if(foundTag != nullptr) {
			return foundTag->value;
		}
		return nullptr;
	}

	static void LoadCreepDefinitions(Assets &assets, const char *filename, const bool isReload, fmemMemoryBlock *memory) {
		assets.creepDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, fplArrayCount(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = utils::LoadEntireFile(filePath, memory);
		assets.creepsFileInfo = fileData.info;
		if(fileData.data != nullptr) {
			fxmlContext ctx = {};
			if(fxmlInitFromMemory(fileData.data, fileData.info.size, &ctx)) {
				fxmlTag root = {};
				if(fxmlParse(&ctx, &root)) {
					fxmlTag *creepDefinitionsTag = fxmlFindTagByName(&root, "CreepDefinitions");
					if(creepDefinitionsTag != nullptr) {
						for(fxmlTag *creepTag = creepDefinitionsTag->firstChild; creepTag; creepTag = creepTag->nextSibling) {
							if(strcmp("CreepData", creepTag->name) == 0) {
								const char *creepId = fxmlGetAttributeValue(creepTag, "id");
								assert(assets.creepDefinitionCount < fplArrayCount(assets.creepDefinitions));
								CreepData *creepData = &assets.creepDefinitions[assets.creepDefinitionCount++];
								*creepData = {};
								fplCopyString(creepId, creepData->id, fplArrayCount(creepData->id));
								creepData->renderRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(creepTag, "renderRadius"));
								creepData->collisionRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(creepTag, "collisionRadius"));
								creepData->speed = utils::StringToFloat(GetNodeValue(creepTag, "speed"));
								creepData->hp = utils::StringToInt(GetNodeValue(creepTag, "hp"));
								creepData->bounty = utils::StringToInt(GetNodeValue(creepTag, "bounty"));
								creepData->color = V4f(1, 1, 1, 1);
							}
						}
					}
				}
			}
		}
	}

	static void LoadPartDefinitions(const fxmlTag *rootTag, const size_t maxPartCount, size_t *partCount, PartData *parts) {
		for(fxmlTag *partTag = rootTag->firstChild; partTag; partTag = partTag->nextSibling) {
			if(strcmp("part", partTag->name) == 0) {
				assert(*partCount < maxPartCount);
				size_t partIndex = *partCount;
				PartData *part = parts + partIndex;
				*partCount = partIndex + 1;
				const char *typeStr = fxmlGetAttributeValue(partTag, "type");
				const char *rotFlagsStr = fxmlGetAttributeValue(partTag, "rotFlags");
				*part = {};
				part->type = PartType::None;
				if(strcmp(typeStr, "Line") == 0) {
					part->type = PartType::Line;
				} else if(strcmp(typeStr, "FillQuad") == 0) {
					part->type = PartType::FillQuad;
				} else if(strcmp(typeStr, "StrokeQuad") == 0) {
					part->type = PartType::StrokeQuad;
				} else if(strcmp(typeStr, "FillCircle") == 0) {
					part->type = PartType::FillCircle;
				} else if(strcmp(typeStr, "StrokeCircle") == 0) {
					part->type = PartType::StrokeCircle;
				}
				part->rotFlags = PartRotationFlags::None;
				{
					const char *p = rotFlagsStr;
					char buffer[32];
					while(*p) {
						while(isspace(*p)) {
							++p;
						}
						const char *start = p;
						while(isalpha(*p)) {
							++p;
						}
						size_t len = p - start;
						if(len > 0) {
							fplCopyStringLen(start, len, buffer, fplArrayCount(buffer));
							PartRotationFlags flag = PartRotationFlags::None;
							if(strcmp("ApplyToOffset", buffer) == 0) {
								flag = PartRotationFlags::ApplyToOffset;
							} else if(strcmp("ApplyToTransform", buffer) == 0) {
								flag = PartRotationFlags::ApplyToTransform;
							}
							part->rotFlags |= flag;
						}
						while(isspace(*p)) {
							++p;
						}
						if(*p == '|') {
							++p;
						} else if(!*p) {
							break;
						}
						++p;
					}
				}

				part->offset = utils::StringToVec2(fxmlGetAttributeValue(partTag, "offset"));
				part->ext = utils::StringToVec2(fxmlGetAttributeValue(partTag, "ext"));
				part->radius = utils::StringToFloat(fxmlGetAttributeValue(partTag, "radius"));
				part->lineWidth = utils::StringToFloat(fxmlGetAttributeValue(partTag, "lineWidth"));
				part->orientation = DegreesToRadians(utils::StringToFloat(fxmlGetAttributeValue(partTag, "orientation")));
				part->color = utils::StringToVec4(fxmlGetAttributeValue(partTag, "color"));
			}
		}
	}

	static void LoadTowerDefinitions(Assets &assets, const char *filename, const bool isReload, fmemMemoryBlock *memory) {
		assets.towerDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, fplArrayCount(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = utils::LoadEntireFile(filePath, memory);
		assets.towersFileInfo = fileData.info;
		if(fileData.data != nullptr) {
			fxmlContext ctx = {};
			if(fxmlInitFromMemory(fileData.data, fileData.info.size, &ctx)) {
				fxmlTag root = {};
				if(fxmlParse(&ctx, &root)) {
					size_t towerIndex = 0;
					fxmlTag *towerDefinitionsTag = fxmlFindTagByName(&root, "TowerDefinitions");
					if(towerDefinitionsTag != nullptr) {
						for(fxmlTag *towerTag = towerDefinitionsTag->firstChild; towerTag; towerTag = towerTag->nextSibling) {
							if(strcmp("TowerData", towerTag->name) == 0) {
								const char *towerId = fxmlGetAttributeValue(towerTag, "id");
								assert(assets.towerDefinitionCount < fplArrayCount(assets.towerDefinitions));
								TowerData *towerData = &assets.towerDefinitions[assets.towerDefinitionCount++];
								*towerData = {};
								fplCopyString(towerId, towerData->id, fplArrayCount(towerData->id));
								towerData->detectionRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(towerTag, "detectionRadius"));
								towerData->unlockRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(towerTag, "unlockRadius"));

								{
									fxmlTag *partsTag = fxmlFindTagByName(towerTag, "parts");
									if(partsTag != nullptr) {
										LoadPartDefinitions(partsTag, fplArrayCount(towerData->parts), &towerData->partCount, towerData->parts);
									}
								}

								fxmlTag *tubesTag = fxmlFindTagByName(towerTag, "tubes");
								if(tubesTag != nullptr) {
									for(fxmlTag *tubeTag = tubesTag->firstChild; tubeTag; tubeTag = tubeTag->nextSibling) {
										if(strcmp("tube", tubeTag->name) == 0) {
											assert(towerData->tubeCount < fplArrayCount(towerData->tubes));
											WeaponTubeData *tubeData = towerData->tubes + towerData->tubeCount++;
											*tubeData = {};
											tubeData->length = utils::StringToFloat(GetNodeValue(tubeTag, "length"));
											tubeData->offset = utils::StringToVec2(GetNodeValue(tubeTag, "offset"));
											fxmlTag *partsTag = fxmlFindTagByName(tubeTag, "parts");
											if(partsTag != nullptr) {
												LoadPartDefinitions(partsTag, fplArrayCount(tubeData->parts), &tubeData->partCount, tubeData->parts);
											}
										}
									}
								}

								towerData->gunCooldown = utils::StringToFloat(GetNodeValue(towerTag, "gunCooldown"));
								towerData->gunRotationSpeed = utils::StringToFloat(GetNodeValue(towerTag, "gunRotationSpeed"));
								const char *enemyRangeTestStr = GetNodeValue(towerTag, "enemyRangeTestType");
								towerData->enemyRangeTestType = FireRangeTestType::NoTest;
								if(strcmp("LineTrace", enemyRangeTestStr) == 0) {
									towerData->enemyRangeTestType = FireRangeTestType::LineTrace;
								} else if(strcmp("InSight", enemyRangeTestStr) == 0) {
									towerData->enemyRangeTestType = FireRangeTestType::InSight;
								}
								const char *enemyLockOnModeStr = GetNodeValue(towerTag, "enemyLockOnMode");
								towerData->enemyLockOnMode = EnemyLockTargetMode::LockedOn;
								if(strcmp("Any", enemyLockOnModeStr) == 0) {
									towerData->enemyLockOnMode = EnemyLockTargetMode::Any;
								}
								towerData->costs = utils::StringToInt(fxmlGetAttributeValue(towerTag, "costs"));
								fxmlTag *bulletTag = fxmlFindTagByName(towerTag, "bullet");
								if(bulletTag != nullptr) {
									towerData->bullet.renderRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(bulletTag, "renderRadius"));
									towerData->bullet.collisionRadius = MaxTileSize * utils::StringToFloat(GetNodeValue(bulletTag, "collisionRadius"));
									towerData->bullet.speed = utils::StringToFloat(GetNodeValue(bulletTag, "speed"));
									towerData->bullet.damage = utils::StringToInt(GetNodeValue(bulletTag, "damage"));
								}

								++towerIndex;
							}
						}
					}
				}
			}
		}
	}

	static void LoadWaveDefinitions(Assets &assets, const char *filename, const bool isReload, fmemMemoryBlock *memory) {
		assets.waveDefinitionCount = 0;
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, fplArrayCount(filePath), 3, assets.dataPath, "levels", filename);
		FileContents fileData = utils::LoadEntireFile(filePath, memory);
		assets.wavesFileInfo = fileData.info;
		if(fileData.data != nullptr) {
			fxmlContext ctx = {};
			if(fxmlInitFromMemory(fileData.data, fileData.info.size, &ctx)) {
				fxmlTag root = {};
				if(fxmlParse(&ctx, &root)) {
					fxmlTag *waveDefinitionsTag = fxmlFindTagByName(&root, "WaveDefinitions");
					if(waveDefinitionsTag != nullptr) {
						for(fxmlTag *waveTag = waveDefinitionsTag->firstChild; waveTag; waveTag = waveTag->nextSibling) {
							if(strcmp("WaveData", waveTag->name) == 0) {
								const char *levelId = fxmlGetAttributeValue(waveTag, "level");
								assert(assets.waveDefinitionCount < fplArrayCount(assets.waveDefinitions));
								WaveData *waveData = &assets.waveDefinitions[assets.waveDefinitionCount++];
								*waveData = {};
								fplCopyString(levelId, waveData->levelId, fplArrayCount(waveData->levelId));
								waveData->startupCooldown = utils::StringToFloat(GetNodeValue(waveTag, "startupCooldown"));
								waveData->spawnerCount = 0;
								waveData->completionBounty = utils::StringToInt(GetNodeValue(waveTag, "completionBounty"));
								fxmlTag *spawnersTag = fxmlFindTagByName(waveTag, "spawners");
								if(spawnersTag != nullptr) {
									for(fxmlTag *spawnTag = spawnersTag->firstChild; spawnTag; spawnTag = spawnTag->nextSibling) {
										if(strcmp("SpawnData", spawnTag->name) == 0) {
											assert(waveData->spawnerCount < fplArrayCount(waveData->spawners));
											SpawnData *spawnData = &waveData->spawners[waveData->spawnerCount++];
											const char *spawnId = fxmlGetAttributeValue(spawnTag, "id");
											const char *enemyId = fxmlGetAttributeValue(spawnTag, "enemy");
											fplCopyString(spawnId, spawnData->spawnId, fplArrayCount(spawnData->spawnId));
											fplCopyString(enemyId, spawnData->enemyId, fplArrayCount(spawnData->enemyId));
											spawnData->initialCooldown = utils::StringToFloat(GetNodeValue(spawnTag, "initialCooldown"));
											spawnData->cooldown = utils::StringToFloat(GetNodeValue(spawnTag, "cooldown"));
											spawnData->enemyCount = utils::StringToInt(GetNodeValue(spawnTag, "enemyCount"));
											const char *startModeString = GetNodeValue(spawnTag, "startMode");
											if(strcmp("AfterTheLast", startModeString) == 0) {
												spawnData->startMode = SpawnerStartMode::AfterTheLast;
											} else {
												spawnData->startMode = SpawnerStartMode::Fixed;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	static LevelLayer *FindLayerByName(LevelData &level, const char *name) {
		for(size_t i = 0; i < level.layerCount; ++i) {
			if(fplIsStringEqual(level.layers[i].name, name)) {
				return &level.layers[i];
			}
		}
		return nullptr;
	}

	static bool LoadLevel(GameState &state, const char *dataPath, const char *filename, LevelData &outLevel, fmemMemoryBlock *memory) {
		bool result = false;

		char filePath[1024];
		fplPathCombine(filePath, fplArrayCount(filePath), 3, dataPath, "levels", filename);
		gamelog::Verbose("Loading level '%s'", filePath);

		fmemMemoryBlock tempMem;
		if(fmemBeginTemporary(&state.transientMem, &tempMem)) {
			FileContents fileData = utils::LoadEntireFile(filePath, &tempMem);
			if(fileData.data != nullptr) {
				fxmlContext ctx = {};
				if(fxmlInitFromMemory(fileData.data, fileData.info.size, &ctx)) {
					fxmlTag root = {};
					if(fxmlParse(&ctx, &root)) {
						outLevel = {};
						if(ParseLevel(&root, outLevel, memory)) {
							LevelLayer *wayLayer = FindLayerByName(outLevel, "way");
							assert(wayLayer != nullptr);

							// Tiles
							LevelTileset *wayTileset = FindLevelTileset(outLevel, "way");
							assert(wayTileset != nullptr);
							assert(state.level.tiles == nullptr);
							state.level.dimension.tileCountX = outLevel.mapWidth;
							state.level.dimension.tileCountY = outLevel.mapHeight;
							state.level.dimension.gridWidth = outLevel.mapWidth * TileWidth;
							state.level.dimension.gridHeight = outLevel.mapHeight * TileHeight;
							state.level.dimension.gridOriginX = -WorldRadiusW + ((WorldWidth - state.level.dimension.gridWidth) * 0.5f);
							state.level.dimension.gridOriginY = -WorldRadiusH + ControlsHeight;
							state.level.tiles = (Tile *)fmemPush(memory, sizeof(Tile) * outLevel.mapWidth * outLevel.mapHeight, fmemPushFlags_Clear);
							for(size_t y = 0; y < outLevel.mapHeight; ++y) {
								for(size_t x = 0; x < outLevel.mapWidth; ++x) {
									size_t tileIndex = y * outLevel.mapWidth + x;
									uint32_t wayValue = wayLayer->data[tileIndex] > 0 ? ((wayLayer->data[tileIndex] - wayTileset->firstGid) + 1) : 0;
									Tile tile = {};
									tile.wayType = TilesetWayToTypeMapping[wayValue];
									tile.entityType = EntityType::None;
									state.level.tiles[tileIndex] = tile;
								}
							}

							// Make waypoints/goal
							for(size_t objIndex = 0; objIndex < outLevel.objectCount; ++objIndex) {
								const ObjectData &obj = outLevel.objects[objIndex];
								if(IsValidTile(state.level.dimension, obj.tilePos)) {
									int tileIndex = obj.tilePos.y * outLevel.mapWidth + obj.tilePos.x;
									switch(obj.type) {
										case ObjectType::Goal:
											state.level.tiles[tileIndex].entityType = EntityType::Goal;
											break;
										case ObjectType::Waypoint:
											AddWaypoint(state.waypoints, state.level.dimension, obj.tilePos, obj.waypoint.direction);
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
			} else {
				gamelog::Error("Level file '%s' could not be found!", filePath);
			}

			fmemEndTemporary(&tempMem);
		} else {
			gamelog::Error("Failed begin temporary memory for load level!");
		}

		return(result);
	}

	static void ClearWave(GameState &state) {
		gamelog::Verbose("Clear Wave");
		state.wave.totalEnemyCount = 0;
		state.wave.isActive = false;
		state.enemies.count = 0;
		state.spawners.count = 0;
		for(size_t towerIndex = 0; towerIndex < state.towers.activeCount; ++towerIndex) {
			Tower &tower = state.towers.activeList[towerIndex];
			tower.hasTarget = false;
			tower.targetEnemy = nullptr;
			tower.targetId = 0;
		}
	}

	static void FreeLevel(Level &level) {
		for(size_t i = 0; i < level.data.layerCount; ++i) {
			if(level.data.layers[i].data != nullptr) {
				level.data.layers[i].data = nullptr;
			}
		}
		for(size_t i = 0; i < level.data.tilesetCount; ++i) {
			if(level.data.tilesets[i].tileUVs != nullptr) {
				level.data.tilesets[i].tileUVs = nullptr;
			}
		}
		if(level.tiles != nullptr) {
			level.tiles = nullptr;
		}
		level.data.layerCount = 0;
		level.data.tilesetCount = 0;
		level.data.objectCount = 0;
		fmemReset(&level.levelMem);
	}

	static void ClearLevel(GameState &state) {
		gamelog::Verbose("Clear Level");
		state.towers.activeCount = 0;
		state.towers.selectedIndex = -1;
		ClearWave(state);
		ClearWaypoints(state.waypoints);
		FreeLevel(state.level);
	}

	static const ObjectData *FindSpawnObjectById(const Level &level, const char *spawnId) {
		for(size_t objectIndex = 0; objectIndex < level.data.objectCount; ++objectIndex) {
			const ObjectData *obj = &level.data.objects[objectIndex];
			if(obj->type == ObjectType::Spawn) {
				if(fplIsStringEqual(obj->spawn.spawnId, spawnId)) {
					return(obj);
				}
			}
		}
		return(nullptr);
	}

	static void LoadWave(GameState &state, const int waveIndex) {
		const WaveData &wave = state.assets.waveDefinitions[waveIndex];

		state.wave.state = WaveState::Stopped;

		gamelog::Verbose("Setup wave '%d'", waveIndex);

		if(state.level.activeId == nullptr || strcmp(state.level.activeId, wave.levelId) != 0) {
			gamelog::Verbose("Active level '%s' is different from '%s'", state.level.activeId, wave.levelId);
			ClearLevel(state);
			char levelFilename[1024];
			fplCopyString(wave.levelId, levelFilename, fplArrayCount(levelFilename));
			fplChangeFileExtension(levelFilename, ".tmx", levelFilename, fplArrayCount(levelFilename));
			if(LoadLevel(state, state.assets.dataPath, levelFilename, state.level.data, &state.level.levelMem)) {
				fplCopyString(wave.levelId, state.level.activeId, fplArrayCount(state.level.activeId));
			} else {
				gamelog::Error("Failed loading level '%s'!", levelFilename);
				return;
			}
			state.towers.selectedIndex = 0;
		}

		if(state.wave.totalEnemyCount > 0 || state.waypoints.first != nullptr || state.spawners.count > 0) {
			ClearWave(state);
		}

		if(fplGetStringLength(state.level.activeId) == 0) {
			gamelog::Error("No level loaded!");
			return;
		}

		Vec2i goalTilePos = level::FindTilePosByEntityType(state.level, EntityType::Goal);
		if(!IsValidTile(state.level.dimension, goalTilePos)) {
			gamelog::Error("No goal entity in level '%s' found!", state.level.activeId);
			return;
		}

		state.wave.activeIndex = waveIndex;
		state.wave.totalEnemyCount = 0;
		for(size_t objectIndex = 0; objectIndex < state.level.data.objectCount; ++objectIndex) {
			ObjectData &obj = state.level.data.objects[objectIndex];
			Vec2i objTilePos = obj.tilePos;
			if(!IsValidTile(state.level.dimension, objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Object '%zu:%s'!", objTilePos.x, objTilePos.y, objectIndex, ObjectTypeToString(obj.type));
				continue;
			}
		}

		for(size_t spawnerIndex = 0; spawnerIndex < wave.spawnerCount; ++spawnerIndex) {
			const SpawnData &spawnerFromWave = wave.spawners[spawnerIndex];
			if(spawnerFromWave.enemyCount == 0) {
				continue;
				gamelog::Warning("No enemies for Spawner '%s'!", spawnerFromWave.spawnId);
			}
			const ObjectData *spawnObj = FindSpawnObjectById(state.level, spawnerFromWave.spawnId);
			if(spawnObj == nullptr) {
				continue;
				gamelog::Warning("Spawner by id '%s' does not exists!", spawnerFromWave.spawnId);
			}
			Vec2i objTilePos = spawnObj->tilePos;
			if(!IsValidTile(state.level.dimension, objTilePos)) {
				gamelog::Warning("Invalid tile position '%d x %d for Spawner '%s'!", objTilePos.x, objTilePos.y, spawnObj->spawn.spawnId);
				continue;
			}
			const CreepData *creepData = creeps::FindEnemyById(state, spawnerFromWave.enemyId);
			if(creepData == nullptr) {
				continue;
				gamelog::Warning("Enemy by id '%s' does not exists!", spawnerFromWave.enemyId);
			}
			creeps::AddSpawner(state.spawners, state.level.dimension, objTilePos, goalTilePos, spawnerFromWave.initialCooldown, spawnerFromWave.cooldown, spawnerFromWave.enemyCount, spawnerFromWave.startMode, creepData);
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
		if((state.towers.selectedIndex < 0) || !(state.towers.selectedIndex < (int)state.assets.towerDefinitionCount)) {
			return CanPlaceTowerResult::NoTowerSelected;
		}
		if(state.towers.activeCount == fplArrayCount(state.towers.activeList)) {
			return CanPlaceTowerResult::TooManyTowers;
		}
		Tile *tile = level::GetTile(state.level, tilePos);
		if(tile == nullptr) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if(tile->isOccupied || tile->entityType != EntityType::None || tile->wayType != WayType::None) {
			return CanPlaceTowerResult::TileOccupied;
		}
		if(state.stats.money < tower->costs) {
			return CanPlaceTowerResult::NotEnoughMoney;
		}
		return(CanPlaceTowerResult::Success);
	}

	static Tower *PlaceTower(GameState &state, const Vec2i &tilePos, const TowerData *data) {
		assert(state.towers.activeCount < fplArrayCount(state.towers.activeList));
		Tower *tower = &state.towers.activeList[state.towers.activeCount++];
		*tower = {};
		tower->data = data;
		tower->position = TileToWorld(state.level.dimension, tilePos, TileExt);
		tower->facingAngle = (float)M_PI * 0.5f; // Face north by default

		Tile *tile = level::GetTile(state.level, tilePos);
		assert(!tile->isOccupied);
		tile->isOccupied = true;

		assert(state.stats.money >= data->costs);
		state.stats.money -= data->costs;

		return(tower);
	}

	static Vec2f PredictEnemyPosition(const Tower &tower, const Creep &enemy, const float deltaTime) {
		// Based on:
		// https://gamedev.stackexchange.com/questions/14469/2d-tower-defense-a-bullet-to-an-enemy
		Vec2f distanceToTarget = enemy.position - tower.position;
		Vec2f enemyVelocity = enemy.facingDirection * (enemy.speed * deltaTime);
		Vec2f bulletVelocity = Vec2Normalize(distanceToTarget) * (tower.data->bullet.speed * deltaTime);
		float a = Vec2Dot(enemyVelocity, enemyVelocity) - Vec2Dot(bulletVelocity, bulletVelocity);
		float b = 2.0f * Vec2Dot(enemyVelocity, distanceToTarget);
		float c = Vec2Dot(distanceToTarget, distanceToTarget);
		float d = -b / (2.0f * a);
		float q = (float)SquareRoot((b * b) - 4.0f * a * c) / (2.0f * a);
		float t1 = d - q;
		float t2 = d + q;
		float t;
		if(t1 > t2 && t2 > 0) {
			t = t2;
		} else {
			t = t1;
		}
		Vec2f result = enemy.position + enemyVelocity * t;
		return(result);
	}

	static Vec2f GetRelativeTubeTip(const WeaponTubeData *tube, const Vec2f &lookDirection) {
		Mat2f rotMat = Mat2FromAxis(lookDirection);
		Vec2f rotatedOffset = Vec2MultMat2(rotMat, tube->offset);
		Vec2f gunTip = rotatedOffset + tube->length * lookDirection;
		return(gunTip);
	}

	static bool InFireRange(const Tower &tower, const Creep &enemy, const float deltaTime) {
		Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
		Vec2f predictedEnemyPosition = PredictEnemyPosition(tower, enemy, deltaTime);
		Vec2f distanceToEnemy = predictedEnemyPosition - tower.position;
		bool result = true;
		if(tower.data->enemyRangeTestType == FireRangeTestType::LineTrace) {
			float maxDistance = Vec2Length(distanceToEnemy) + enemy.data->collisionRadius;
			for(size_t tubeIndex = 0; tubeIndex < tower.data->tubeCount; ++tubeIndex) {
				const WeaponTubeData *tube = tower.data->tubes + tubeIndex;
				Vec2f gunTip = tower.position + GetRelativeTubeTip(tube, lookDirection);
				LineCastInput input = {};
				input.p1 = gunTip;
				input.p2 = input.p1 + lookDirection * maxDistance;
				input.maxFraction = 1.0f;
				LineCastOutput output = {};
				result = LineCastCircle(input, enemy.position, enemy.data->collisionRadius, output);
				if(result) {
					break;
				}
			}
		} else if(tower.data->enemyRangeTestType == FireRangeTestType::InSight) {
			float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
			if(projDistance > 0) {
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
		for(size_t tubeIndex = 0; tubeIndex < tower.data->tubeCount; ++tubeIndex) {
			const WeaponTubeData *tube = tower.data->tubes + tubeIndex;
			assert(bullets.count < fplArrayCount(bullets.list));
			Bullet *bullet = &bullets.list[bullets.count++];
			*bullet = {};
			Vec2f targetDir = V2f(Cosine(tower.facingAngle), Sine(tower.facingAngle));
			Vec2f gunTip = tower.position + GetRelativeTubeTip(tube, targetDir);
			bullet->position = bullet->prevPosition = gunTip;
			bullet->data = &tower.data->bullet;
			bullet->velocity = targetDir * bullet->data->speed;
		}
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
			for(size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep *testEnemy = &state.enemies.list[enemyIndex];
				if(!testEnemy->isDead) {
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
		if(tower.data->enemyLockOnMode == EnemyLockTargetMode::Any) {
			for(size_t enemyIndex = 0; enemyIndex < state.enemies.count; ++enemyIndex) {
				Creep &enemy = state.enemies.list[enemyIndex];
				if(!enemy.isDead) {
					bool inFireRange = towers::InFireRange(tower, enemy, deltaTime);
					if(inFireRange && tower.canFire) {
						ShootBullet(state.bullets, tower);
					}
				}
			}
		} else if(tower.data->enemyLockOnMode == EnemyLockTargetMode::LockedOn) {
			if(tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				Creep *enemy = tower.targetEnemy;
				assert(!enemy->isDead);
				bool inFireRange = towers::InFireRange(tower, *enemy, deltaTime);
				if(inFireRange && tower.canFire) {
					ShootBullet(state.bullets, tower);
				}
			}
		}
	}

	static void DrawTower(RenderState &renderState, const Assets &assets, const Camera2D &camera, const TowerData &tower, const Vec2f &pos, const Vec2f &maxRadius, const float angle, const float alpha, const bool drawRadius) {
		assert(MaxTileRadius > 0);
		float scale = fplMax(maxRadius.x, maxRadius.y) / MaxTileRadius;

		render::DrawParts(renderState, assets, camera, pos, scale, alpha, angle, tower.partCount, tower.parts);

		for(size_t tubeIndex = 0; tubeIndex < tower.tubeCount; ++tubeIndex) {
			const WeaponTubeData *tube = &tower.tubes[tubeIndex];
			render::DrawParts(renderState, assets, camera, pos, scale, alpha, angle, tube->partCount, tube->parts);
		}

		if(drawRadius) {
			const TextureAsset &radiantTexture = assets.radiantTexture;
			PushSprite(renderState, pos, V2f(tower.detectionRadius * scale, tower.detectionRadius * scale), radiantTexture.texture, V4f(0.2f, 1, 0.2f, alpha*0.25f), V2f(0, 0), V2f(1, 1));
			PushSprite(renderState, pos, V2f(tower.unlockRadius * scale, tower.unlockRadius * scale), radiantTexture.texture, V4f(1, 0.25f, 0.25f, alpha*0.25f), V2f(0, 0), V2f(1, 1));
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

	static void ReleaseAssets(Assets &assets) {
		ReleaseFontAsset(assets.overlayFont);
		ReleaseFontAsset(assets.hudFont);
	}

	static void LoadTextureAsset(RenderState &renderState, const char *dataPath, const char *filename, const bool isTopDown, TextureAsset *outAsset) {
		char filePath[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);
		int width, height, comp;
		stbi_set_flip_vertically_on_load(isTopDown ? 0 : 1);
		uint8_t *data = stbi_load(filePath, &width, &height, &comp, 4);
		if(data != nullptr) {
			outAsset->data.data = data;
			outAsset->data.components = 4;
			outAsset->data.width = width;
			outAsset->data.height = height;
			const TextureData &texData = outAsset->data;
			PushTexture(renderState, &outAsset->texture, texData.data, texData.width, texData.height, 4, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		}
	}

	static void LoadAssets(GameState &gameState, RenderState &renderState) {
		fmemMemoryBlock tempMem = {};
		if(fmemBeginTemporary(&gameState.transientMem, &tempMem)) {
			Assets &assets = gameState.assets;

			// Towers/Enemies/Waves
			level::LoadCreepDefinitions(assets, CreepsDataFilename, false, &tempMem);
			level::LoadTowerDefinitions(assets, TowersDataFilename, false, &tempMem);
			level::LoadWaveDefinitions(assets, WavesDataFilename, false, &tempMem);

			// Fonts
			char fontDataPath[1024];
			const char *fontFilename = "SulphurPoint-Bold.otf";
			fplPathCombine(fontDataPath, fplArrayCount(fontDataPath), 2, assets.dataPath, "fonts");
			FontAsset &hudFont = assets.hudFont;
			if(LoadFontFromFile(fontDataPath, fontFilename, 0, 36.0f, 32, 128, 512, 512, false, &hudFont.desc)) {
				PushTexture(renderState, &hudFont.texture, hudFont.desc.atlasAlphaBitmap, hudFont.desc.atlasWidth, hudFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
			}
			FontAsset &overlayFont = assets.overlayFont;
			if(LoadFontFromFile(fontDataPath, fontFilename, 0, 240.0f, 32, 128, 4096, 4096, false, &overlayFont.desc)) {
				PushTexture(renderState, &overlayFont.texture, overlayFont.desc.atlasAlphaBitmap, overlayFont.desc.atlasWidth, overlayFont.desc.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
			}

			// Textures
			char texturesDataPath[1024];
			char levelsDataPath[1024];
			fplPathCombine(texturesDataPath, fplArrayCount(texturesDataPath), 2, assets.dataPath, "textures");
			fplPathCombine(levelsDataPath, fplArrayCount(levelsDataPath), 2, assets.dataPath, "levels");
			LoadTextureAsset(renderState, texturesDataPath, "radiant.png", false, &assets.radiantTexture);
			LoadTextureAsset(renderState, levelsDataPath, "way_tileset.png", false, &assets.wayTilesetTexture);
			LoadTextureAsset(renderState, levelsDataPath, "entities_tileset.png", false, &assets.entitiesTilesetTexture);
			LoadTextureAsset(renderState, levelsDataPath, "ground_tileset.png", false, &assets.groundTilesetTexture);

			fmemEndTemporary(&tempMem);
		} else {
			gamelog::Error("Failed begin temporary memory for assets!");
		}
	}

	static void ReleaseGame(GameState &state) {
		gamelog::Verbose("Release Game");
		level::ClearLevel(state);
		ReleaseAssets(state.assets);
	}

	static void NewGame(GameState &state) {
		// Reset camera
		state.camera.scale = 1.0f;
		state.camera.offset.x = 0;
		state.camera.offset.y = 0;

		// @TODO(final): Read from game.xml
		state.stats.money = 50;
		state.stats.lifes = 10;

		// Load initial wave
		level::LoadWave(state, 0);
	}

	static bool InitGame(GameState &state, GameMemory &gameMemory) {
		gamelog::Verbose("Initialize Game");

		fplGetExecutableFilePath(state.assets.dataPath, fplArrayCount(state.assets.dataPath));
		fplExtractFilePath(state.assets.dataPath, state.assets.dataPath, fplArrayCount(state.assets.dataPath));
		fplPathCombine(state.assets.dataPath, fplArrayCount(state.assets.dataPath), 2, state.assets.dataPath, "data");
		gamelog::Info("Using assets path: %s", state.assets.dataPath);

		size_t levelMemorySize = FMEM_MEGABYTES(32);
		uint8_t *levelMemory = fmemPush(gameMemory.memory, levelMemorySize, fmemPushFlags_None);
		if(!fmemInitFromSource(&state.level.levelMem, levelMemory, levelMemorySize)) {
			gamelog::Fatal("Failed pushing %zu level memory!", levelMemory);
			return(false);
		}

		size_t transientMemorySize = FMEM_MEGABYTES(8);
		uint8_t *transientMemory = fmemPush(gameMemory.memory, transientMemorySize, fmemPushFlags_None);
		if(!fmemInitFromSource(&state.transientMem, transientMemory, transientMemorySize)) {
			gamelog::Fatal("Failed pushing %zu transient memory!", transientMemorySize);
			return(false);
		}

		LoadAssets(state, *gameMemory.render);

		NewGame(state);

		return(true);
	}

	static void DrawHUD(GameState &state, RenderState &renderState) {
		constexpr float hudPadding = MaxTileSize * 0.075f;
		constexpr float hudOriginX = -WorldRadiusW;
		constexpr float hudOriginY = WorldRadiusH;
		constexpr float hudFontHeight = TileHeight * 0.4f;
		constexpr float outlineOffset = hudFontHeight * 0.05f;
		const FontAsset &font = state.assets.hudFont;
		{
			char text[256];
			fplFormatString(text, fplArrayCount(text), "%s", state.level.activeId);
			Vec2f textPos = V2f(hudOriginX + WorldRadiusW, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);

			fplFormatString(text, fplArrayCount(text), "Wave: %d / %zu", (state.wave.activeIndex + 1), state.assets.waveDefinitionCount);
			textPos.y -= hudFontHeight;
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);

			fplFormatString(text, fplArrayCount(text), "Enemies: %zu / %zu", state.enemies.count, state.wave.totalEnemyCount);
			textPos.y -= hudFontHeight;
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 0.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 0.0f, 0.0f, TextForeColor);
		}
		{
			char text[256];
			fplFormatString(text, fplArrayCount(text), "$: %d", state.stats.money);
			Vec2f textPos = V2f(hudOriginX + hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, 1.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, 1.0f, 0.0f, TextForeColor);
		}
		{
			char text[256];
			fplFormatString(text, fplArrayCount(text), "HP: %d", state.stats.lifes);
			Vec2f textPos = V2f(hudOriginX + WorldWidth - hudPadding, hudOriginY - hudPadding - hudFontHeight * 0.5f);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + outlineOffset, textPos.y - outlineOffset), hudFontHeight, -1.0f, 0.0f, TextBackColor);
			PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), hudFontHeight, -1.0f, 0.0f, TextForeColor);
		}
	}

	static void DrawTowerControl(GameState &gameState, RenderState &renderState, const Vec2f &pos, const Vec2f &radius, const ui::UIButtonState buttonState, void *userData) {
		size_t towerDataIndex = (size_t)(uintptr_t)(userData);
		assert(towerDataIndex >= 0 && towerDataIndex < gameState.assets.towerDefinitionCount);
		const TowerData *towerData = &gameState.assets.towerDefinitions[towerDataIndex];
		float alpha = 0.75f;
		if(buttonState == ui::UIButtonState::Hover) {
			alpha = 1.0f;
		}
		towers::DrawTower(renderState, gameState.assets, gameState.camera, *towerData, pos, radius, Pi32 * 0.5f, alpha, false);

		// Draw selection frame
		if(gameState.towers.selectedIndex == towerDataIndex) {
			Vec2f borderVecs[] = {
				V2f(pos.x + radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y + radius.h),
				V2f(pos.x - radius.w, pos.y - radius.h),
				V2f(pos.x + radius.w, pos.y - radius.h),
			};
			float stippleWidth = (fplMin(radius.x, radius.y) * 2.0f) / 10.0f;
			Vec4f stippleColor = V4f(1.0f, 1.0f, 1.0f, alpha);
			float stippleLineWidth = 1.0f;
			render::DrawLineLoopStipple(renderState, borderVecs, 4, stippleWidth, 3, stippleColor, stippleLineWidth);
		}
	}

	static void DrawControls(GameState &state, RenderState &renderState) {
		//
		// Controls Background
		//
		Vec4f backgroundColor = V4f(0.2f, 0.2f, 0.2f, 1.0f);
		PushRectangle(renderState, V2f(ControlsOriginX, ControlsOriginY), V2f(ControlsWidth, ControlsHeight), backgroundColor, true, 0.0f);

		// Controls Border
		float lineWidth = 2.0f;
		float lineWidthWorld = lineWidth * state.camera.pixelsToWorld * 0.5f;
		Vec4f controlsBorderColor = V4f(0.5f, 0.5f, 0.5f, 1.0f);
		Vec2f controlsBottomLeft = V2f();
		Vec2f controlsVerts[] = {
			V2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld),
			V2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + ControlsHeight - lineWidthWorld),
			V2f(ControlsOriginX + lineWidthWorld, ControlsOriginY + lineWidthWorld),
			V2f(ControlsOriginX + ControlsWidth - lineWidthWorld, ControlsOriginY + lineWidthWorld),
		};
		PushVertices(renderState, controlsVerts, 4, true, controlsBorderColor, DrawMode::Lines, true, lineWidth);

		// Tower buttons
		float buttonPadding = MaxTileSize * 0.1f;
		float buttonMargin = lineWidthWorld + (MaxTileSize * 0.15f);
		float buttonHeight = ControlsHeight - buttonMargin * 2.0f;
		Vec2f buttonRadius = V2f(buttonHeight * 0.5f);
		Vec2f buttonOutputRadius = ui::GetUIButtonExt(buttonRadius);
		for(size_t towerIndex = 0; towerIndex < state.assets.towerDefinitionCount; ++towerIndex) {
			void *buttonId = (void *)&state.assets.towerDefinitions[towerIndex]; // Totally dont care about const removal here
			float buttonX = ControlsOriginX + buttonMargin + (towerIndex * (buttonOutputRadius.w * 2.0f) + (fplMax(0, towerIndex - 1) * buttonPadding));
			float buttonY = ControlsOriginY + buttonMargin;
			if(ui::UIButton(state.ui, buttonId, V2f(buttonX + buttonRadius.w, buttonY + buttonRadius.h), buttonRadius, DrawTowerControl, (void *)(uintptr_t)towerIndex)) {
				state.towers.selectedIndex = (int)towerIndex;
			}
		}

		if(state.towers.selectedIndex > -1) {
			const FontAsset &font = state.assets.hudFont;
			float fontHeight = MaxTileSize * 0.4f;
			const TowerData &towerData = state.assets.towerDefinitions[state.towers.selectedIndex];
			Vec2f textPos = V2f(ControlsOriginX + ControlsWidth - lineWidthWorld - buttonMargin, ControlsOriginY + ControlsHeight * 0.5f);
			char textBuffer[256];
			fplFormatString(textBuffer, fplArrayCount(textBuffer), "[%s / $%d]", towerData.id, towerData.costs);
			PushText(renderState, textBuffer, fplGetStringLength(textBuffer), &font.desc, &font.texture, V2f(textPos.x, textPos.y), fontHeight, -1.0f, 0.0f, TextForeColor);
		}

	}
}

extern bool GameInit(GameMemory &gameMemory) {
	gamelog::Verbose("Init Game");
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;
	if(!game::InitGame(*state, gameMemory)) {
		gamelog::Fatal("Failed initializing Game!");
		GameRelease(gameMemory);
		return(false);
	}
	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	gamelog::Verbose("Destroy Game");
	GameState *state = gameMemory.game;
	if(state != nullptr) {
		game::ReleaseGame(*state);
		state->~GameState();
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	RenderState *renderState = gameMemory.render;

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

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;

	float invScale = 1.0f / state->camera.scale;
	Mat4f proj = Mat4OrthoRH(-w * invScale, w * invScale, -h * invScale, h * invScale, 0.0f, 1.0f);
	Mat4f view = Mat4Translation(state->camera.offset);
	state->viewProjection = proj * view;

	ui::UIBegin(state->ui, state, renderState, input, state->mouseWorldPos);

	// Mouse
	int mouseCenterX = (input.mouse.pos.x - input.windowSize.w / 2);
	int mouseCenterY = (input.windowSize.h - 1 - input.mouse.pos.y) - input.windowSize.h / 2;
	state->mouseWorldPos.x = (mouseCenterX * state->camera.pixelsToWorld) - state->camera.offset.x;
	state->mouseWorldPos.y = (mouseCenterY * state->camera.pixelsToWorld) - state->camera.offset.y;

	if(state->wave.state == WaveState::Running || state->wave.state == WaveState::Starting) {
		// Update tile position from mouse
		state->mouseTilePos = WorldToTile(state->level.dimension, state->mouseWorldPos);

		// Tower placement
		if(WasPressed(input.mouse.left) && !ui::UIIsHot(state->ui)) {
			if(state->towers.selectedIndex > -1) {
				const TowerData *tower = &state->assets.towerDefinitions[state->towers.selectedIndex];
				if(towers::CanPlaceTower(*state, state->mouseTilePos, tower) == towers::CanPlaceTowerResult::Success) {
					towers::PlaceTower(*state, state->mouseTilePos, tower);
				}
			}
		}
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}

	GameState *state = gameMemory.game;
	assert(state != nullptr);

	if(WasPressed(input.keyboard.debugReload)) {
		char filePathBuffer[FPL_MAX_PATH_LENGTH];
		fplPathCombine(filePathBuffer, fplArrayCount(filePathBuffer), 3, state->assets.dataPath, "levels", TowersDataFilename);
		FileInfo towersFileContents = utils::LoadFileInfo(filePathBuffer);
		fplPathCombine(filePathBuffer, fplArrayCount(filePathBuffer), 3, state->assets.dataPath, "levels", CreepsDataFilename);
		FileInfo creepsFileContents = utils::LoadFileInfo(filePathBuffer);
		fplPathCombine(filePathBuffer, fplArrayCount(filePathBuffer), 3, state->assets.dataPath, "levels", WavesDataFilename);
		FileInfo wavesFileContents = utils::LoadFileInfo(filePathBuffer);

		//utils::IsEqualFileInfo(towersFileContents, state->assets.towersFileInfo)
		//utils::IsEqualFileInfo(creepsFileContents, state->assets.creepsFileInfo)
		//utils::IsEqualFileInfo(wavesFileContents, state->assets.wavesFileInfo)

		fmemMemoryBlock tempMem = {};
		if(fmemBeginTemporary(&state->transientMem, &tempMem)) {
			level::LoadCreepDefinitions(state->assets, CreepsDataFilename, true, &tempMem);
			level::LoadTowerDefinitions(state->assets, TowersDataFilename, true, &tempMem);
			level::LoadWaveDefinitions(state->assets, WavesDataFilename, true, &tempMem);
			fmemEndTemporary(&tempMem);
		}
	}


	float dtScale = 1.0f;
	if(state->isSlowDown) {
		assert(state->slowdownTimer[1] > 0);
		if(state->slowdownTimer[0] > 0.0f) {
			state->slowdownTimer[0] -= input.deltaTime;
		} else {
			state->slowdownTimer[0] = 0;
			if(state->wave.state != state->waveStateAfterSlowdown) {
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
	if(state->wave.state == WaveState::Starting) {
		state->wave.warmupTimer -= dt;
		if(state->wave.warmupTimer <= 0.0f) {
			state->wave.warmupTimer = 0;
			state->wave.state = WaveState::Running;
			for(size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
				CreepSpawner &spawner = state->spawners.list[spawnerIndex];
				if(spawner.startMode == SpawnerStartMode::Fixed) {
					spawner.isActive = true;
				}
			}
		}
	}

	bool updateGameCode = state->wave.state == WaveState::Running;

	if(state->wave.state != WaveState::Stopped) {
		//
		// Move enemies
		//
		for(size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if(!enemy.isDead && enemy.hasTarget) {
				Vec2f distance = enemy.targetPos - enemy.position;
				float minRadius = MaxTileSize * 0.05f;
				enemy.position += enemy.facingDirection * enemy.speed * dt;
				if(Vec2Dot(distance, distance) <= minRadius * minRadius) {
					creeps::SetCreepNextTarget(*state, enemy);
				}
			}
		}

		// Update spawners
		for(size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
			CreepSpawner &spawner = state->spawners.list[spawnerIndex];
			creeps::UpdateSpawner(*state, spawner, dt);
		}

		// Update towers
		if(updateGameCode) {
			for(size_t towerIndex = 0; towerIndex < state->towers.activeCount; ++towerIndex) {
				Tower &tower = state->towers.activeList[towerIndex];
				towers::UpdateTower(*state, tower, dt);
			}
		}

		//
		// Move and collide bullets
		//
		for(size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if(!bullet.isDestroyed) {
				bullet.position += bullet.velocity * dt;
				if(!bullet.hasHit) {
					for(size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
						Creep &enemy = state->enemies.list[enemyIndex];
						if(!enemy.isDead) {
							Vec2f distance = enemy.position - bullet.position;
							float bothRadi = bullet.data->collisionRadius + enemy.data->collisionRadius;
							float d = Vec2Dot(distance, distance);
							if(d < bothRadi * bothRadi) {
								bullet.hasHit = true;
								if(updateGameCode) {
									creeps::CreepHit(*state, enemy, bullet);
								}
								break;
							}
						}
					}
				}
				if(!bullet.hasHit) {
					if(((bullet.position.x + bullet.data->renderRadius) > WorldRadiusW) ||
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
		for(size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
			Bullet &bullet = state->bullets.list[bulletIndex];
			if(bullet.hasHit) {
				bullet.isDestroyed = true;
			}
			if(bullet.isDestroyed) {
				if(bulletIndex < state->bullets.count - 1) {
					const Bullet &lastBullet = state->bullets.list[state->bullets.count - 1];
					bullet = lastBullet;
				}
				--state->bullets.count;
			}
		}
		size_t deadEnemyCount = 0;
		size_t nonDeadEnemyCount = 0;
		for(size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
			Creep &enemy = state->enemies.list[enemyIndex];
			if(enemy.isDead) {
				++deadEnemyCount;
			} else {
				++nonDeadEnemyCount;
			}
		}

		if(updateGameCode) {
			if(state->wave.totalEnemyCount == deadEnemyCount) {
				creeps::AllEnemiesKilled(*state);
			} else {
				if(state->stats.lifes <= 0) {
					state->stats.lifes = 0;
					state->wave.isActive = false;
					game::SetSlowdown(*state, 6.0f, WaveState::Lost);
				} else {
					bool hasActiveSpawners = false;
					CreepSpawner *nextSpawner = nullptr;
					for(size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
						CreepSpawner *spawner = &state->spawners.list[spawnerIndex];
						if(spawner->isActive) {
							hasActiveSpawners = true;
							break;
						} else {
							if(nextSpawner == nullptr && spawner->startMode == SpawnerStartMode::AfterTheLast) {
								nextSpawner = spawner;
							}
						}
					}
					if(nonDeadEnemyCount == 0 && !hasActiveSpawners) {
						// All enemies but not all from all spawners has been killed
						if(nextSpawner != nullptr) {
							nextSpawner->isActive = true;
							nextSpawner->spawnTimer = nextSpawner->cooldown;
							nextSpawner->remainingCount = nextSpawner->totalCount;
						}
					}
				}
			}
		}
	}
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	assert(state != nullptr);
	RenderState &renderState = *gameMemory.render;

	const float w = WorldRadiusW;
	const float h = WorldRadiusH;
	const float dt = state->deltaTime;

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4f(0, 0, 0, 1), ClearFlags::Color | ClearFlags::Depth);
	SetMatrix(renderState, state->viewProjection);

	const Level &level = state->level;
	const LevelDimension &dim = level.dimension;

	//
	// Tiles
	//
	// @TODO(final): Do this gid-to-tileset mapping once and not everytime on render
	const LevelTileset *gidToTileset[256 + 1] = {};
	const TextureAsset *tilesetToTexture[MAX_TILESET_COUNT] = {};
	for(size_t tilesetIndex = 0; tilesetIndex < state->level.data.tilesetCount; ++tilesetIndex) {
		const LevelTileset *tileset = &state->level.data.tilesets[tilesetIndex];
		for(size_t i = tileset->firstGid; i < (tileset->firstGid + tileset->tileCount); ++i) {
			gidToTileset[i] = tileset;
		}
		uintptr_t tilesetTextureIndex = (uintptr_t)(tileset - state->level.data.tilesets);
		assert(tilesetTextureIndex >= 0 && tilesetTextureIndex < MAX_TILESET_COUNT);
		if(fplIsStringEqual("way", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.wayTilesetTexture;
		} else if(fplIsStringEqual("ground", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.groundTilesetTexture;
		} else if(fplIsStringEqual("entities", tileset->name)) {
			tilesetToTexture[tilesetTextureIndex] = &state->assets.entitiesTilesetTexture;
		}
	}

	// Tile layers
	// @SLOW(final): This is slowest way you imagine how to render multiple tile layers
	for(size_t layerIndex = 0; layerIndex < state->level.data.layerCount; ++layerIndex) {
		const LevelLayer &layer = state->level.data.layers[layerIndex];
		for(size_t y = 0; y < layer.mapHeight; ++y) {
			for(size_t x = 0; x < layer.mapWidth; ++x) {
				uint32_t tileData = layer.data[y * layer.mapWidth + x];
				if(tileData > 0 && tileData < fplArrayCount(gidToTileset)) {
					const LevelTileset *tileset = gidToTileset[tileData];
					assert(tileset != nullptr);
					assert(tileData >= tileset->firstGid);
					size_t indexToTilesheet = tileData - tileset->firstGid;
					uintptr_t tilesetTextureIndex = (uintptr_t)(tileset - state->level.data.tilesets);
					const TextureAsset *texAsset = tilesetToTexture[tilesetTextureIndex];
					const UVRect &uvRect = tileset->tileUVs[indexToTilesheet];
					if(texAsset != nullptr) {
						Vec2f pos = TileToWorld(state->level.dimension, V2i((int)x, (int)y), TileExt);
						PushSprite(renderState, pos, TileExt, texAsset->texture, V4f(1, 1, 1, layer.opacity), uvRect);
					}
				}
			}
		}
	}


	if(state->isDebugRendering) {
		for(int y = 0; y < (int)dim.tileCountY; ++y) {
			for(int x = 0; x < (int)dim.tileCountX; ++x) {
				const Tile &tile = state->level.tiles[y * dim.tileCountX + x];
				if(tile.wayType != WayType::None) {
					render::DrawTile(renderState, dim, x, y, true, V4f(0.0f, 0.0f, 1.0f, 0.5f));
				}
			}
		}

		// Draw tile entities
		for(int y = 0; y < (int)dim.tileCountY; ++y) {
			for(int x = 0; x < (int)dim.tileCountX; ++x) {
				const Tile &tile = state->level.tiles[y * dim.tileCountX + x];
				switch(tile.entityType) {
					case EntityType::Goal:
					{
						render::DrawTile(renderState, dim, x, y, true, V4f(0.1f, 1.0f, 0.2f, 1.0f));
					} break;
				}
			}
		}

		// Draw spawners tile
		for(size_t spawnerIndex = 0; spawnerIndex < state->spawners.count; ++spawnerIndex) {
			const CreepSpawner &spawner = state->spawners.list[spawnerIndex];
			Vec2i tilePos = WorldToTile(dim, spawner.spawnPosition);
			render::DrawTile(renderState, dim, tilePos.x, tilePos.y, true, V4f(0.0f, 1.0f, 1.0f, 1.0f));
		}
	}


	//
	// Grid
	//
	Vec4f gridColor = V4f(1.0f, 1.0f, 1.0f, 0.25f);
	float gridLineWidth = DefaultLineWidth;
	size_t totalGridVerts = (dim.tileCountX + 1) * 2 + (dim.tileCountY + 1) * 2;
	VertexAllocation vertAlloc = AllocateVertices(renderState, totalGridVerts, gridColor, DrawMode::Lines, false, gridLineWidth);
	Vec2f *gridVertex = vertAlloc.verts;
	size_t count = 0;
	for(size_t y = 0; y <= dim.tileCountY; ++y) {
		*gridVertex++ = V2f(dim.gridOriginX, dim.gridOriginY + y * TileHeight);
		*gridVertex++ = V2f(dim.gridOriginX + dim.tileCountX * TileWidth, dim.gridOriginY + y * TileHeight);
		count += 2;
	}
	for(size_t x = 0; x <= dim.tileCountX; ++x) {
		*gridVertex++ = V2f(dim.gridOriginX + x * TileWidth, dim.gridOriginY);
		*gridVertex++ = V2f(dim.gridOriginX + x * TileWidth, dim.gridOriginY + dim.tileCountY * TileHeight);
		count += 2;
	}
	assert(count == totalGridVerts);
	*vertAlloc.count = count;

	if(state->isDebugRendering) {
		// Waypoints
		for(Waypoint *waypoint = state->waypoints.first; waypoint != nullptr; waypoint = waypoint->next) {
			PushRectangleCenter(renderState, waypoint->position, V2f(MaxTileSize * 0.15f), V4f(1, 0, 1, 1), true, 0.0f);
			PushLine(renderState, waypoint->position, waypoint->position + waypoint->direction * level::WaypointDirectionWidth, V4f(1, 1, 1, 1), 1.0f);
		}
	}

	// Hover tile
	if(state->towers.selectedIndex > -1 && IsValidTile(dim, state->mouseTilePos)) {
		const TowerData *tower = &state->assets.towerDefinitions[state->towers.selectedIndex];

		towers::CanPlaceTowerResult placeRes = towers::CanPlaceTower(*state, state->mouseTilePos, tower);
		Vec4f hoverColor;
		if(placeRes == towers::CanPlaceTowerResult::Success) {
			hoverColor = V4f(0.1f, 1.0f, 0.1f, 1.0f);
		} else {
			hoverColor = V4f(1.0f, 0.1f, 0.1f, 1.0f);
		}

		if(placeRes == towers::CanPlaceTowerResult::Success || placeRes == towers::CanPlaceTowerResult::NotEnoughMoney) {
			float alpha = placeRes == towers::CanPlaceTowerResult::Success ? 0.5f : 0.2f;
			Vec2f towerCenter = TileToWorld(dim, state->mouseTilePos, TileExt);
			towers::DrawTower(renderState, state->assets, state->camera, *tower, towerCenter, V2f(MaxTileRadius), Pi32 * 0.5f, alpha, true);
		}

		render::DrawTile(renderState, dim, state->mouseTilePos.x, state->mouseTilePos.y, false, hoverColor);
	}

	//
	// Enemies
	//
	for(size_t enemyIndex = 0; enemyIndex < state->enemies.count; ++enemyIndex) {
		Creep &enemy = state->enemies.list[enemyIndex];
		if(!enemy.isDead && enemy.id > 0) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);

			// Mesh
			PushRectangleCenter(renderState, enemyPos, V2f(enemy.data->renderRadius, enemy.data->renderRadius), enemy.data->color, true, 0.0f);

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
				Vec4f progressColor = V4f(colorRed, colorGreen, 0.0f, 1.0f);
				Vec2f progressVerts[] = {
					V2f(barX + barWidth * barScale, barY + barHeight),
					V2f(barX, barY + barHeight),
					V2f(barX, barY),
					V2f(barX + barWidth * barScale, barY),
				};
				PushVertices(renderState, progressVerts, fplArrayCount(progressVerts), true, progressColor, DrawMode::Polygon, true, 0.0f);

				Vec4f borderColor = V4f(0.25f, 0.25f, 0.25f, 1.0f);
				float borderLineWidth = 2.0f;
				Vec2f borderVerts[] = {
					V2f(barX + barWidth, barY + barHeight),
					V2f(barX, barY + barHeight),
					V2f(barX, barY),
					V2f(barX + barWidth, barY),
				};
				PushVertices(renderState, borderVerts, fplArrayCount(borderVerts), true, borderColor, DrawMode::Lines, true, borderLineWidth);
			}

			enemy.prevPosition = enemy.position;
		}
	}

	//
	// Towers
	//
	for(size_t towerIndex = 0; towerIndex < state->towers.activeCount; ++towerIndex) {
		const Tower &tower = state->towers.activeList[towerIndex];
		towers::DrawTower(renderState, state->assets, state->camera, *tower.data, tower.position, V2f(MaxTileRadius), tower.facingAngle, 1.0f, false);

		if(state->isDebugRendering) {
			if(tower.hasTarget) {
				assert(tower.targetEnemy != nullptr);
				const Creep *target = tower.targetEnemy;
				if((target->id > 0) && (target->id == tower.targetId)) {
					PushCircle(renderState, target->position, target->data->collisionRadius, 32, V4f(1, 0, 0, 1), false, 1.0f);

					Vec2f lookPos = towers::PredictEnemyPosition(tower, *target, dt);
					PushCircle(renderState, lookPos, MaxTileSize * 0.25f, 16, V4f(1, 1, 0, 1), false, 1.0f);

					float dot = Vec2Dot(target->position, lookPos);
					float det = Vec2Cross(target->position, lookPos);
					float angle = ArcTan2(det, dot);

					if(angle >= -ShotAngleTolerance && angle <= ShotAngleTolerance) {
						Vec2f lookDirection = Vec2AngleToAxis(tower.facingAngle);
						Vec2f distanceToEnemy = target->position - tower.position;
						float projDistance = Vec2Dot(distanceToEnemy, lookDirection);
						Vec2f sightPos1 = tower.position + Vec2AngleToAxis(tower.facingAngle - ShotAngleTolerance) * projDistance;
						Vec2f sightPos2 = tower.position + Vec2AngleToAxis(tower.facingAngle + ShotAngleTolerance) * projDistance;
						Vec4f sightColor = V4f(1, 0, 0, 0.5);
						float sightLineWidth = 1.0f;
						Vec2f sightVec2[] = {
							V2f(tower.position.x, tower.position.y),
							V2f(sightPos1.x, sightPos1.y),
							V2f(tower.position.x, tower.position.y),
							V2f(sightPos2.x, sightPos2.y),
						};
						PushVertices(renderState, sightVec2, fplArrayCount(sightVec2), true, sightColor, DrawMode::Lines, false, sightLineWidth);
					}
				}
			}
		}
	}

	//
	// Bullets
	//
	for(size_t bulletIndex = 0; bulletIndex < state->bullets.count; ++bulletIndex) {
		Bullet &bullet = state->bullets.list[bulletIndex];
		if(!bullet.isDestroyed) {
			Vec2f bulletPos = Vec2Lerp(bullet.prevPosition, alpha, bullet.position);
			// @TODO(final): Use sprites for bullets
			PushCircle(renderState, bulletPos, bullet.data->renderRadius, 32, V4f(1, 0, 0, 1), true, 0.0f);
			bullet.prevPosition = bullet.position;
		}
	}

	//
	// Selected tower text
	//

	//
	// Overlay
	//
	if(state->wave.state == WaveState::Starting) {
		const FontAsset &font = state->assets.overlayFont;
		char text[128];
		fplFormatString(text, fplArrayCount(text), "%d", (int)ceilf(state->wave.warmupTimer));
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.25f;
		float foffset = overlayFontHeight * 0.01f;
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), overlayFontHeight, 0.0f, 0.0f, TextBackColor);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + foffset, textPos.y - foffset), overlayFontHeight, 0.0f, 0.0f, TextForeColor);
	} else if(state->wave.state == WaveState::Won || state->wave.state == WaveState::Lost) {
		const FontAsset &font = state->assets.overlayFont;
		const char *text = state->wave.state == WaveState::Won ? "You Win!" : "Game Over!";
		Vec2f textPos = V2f(0, 0);
		float overlayFontHeight = WorldWidth * 0.15f;
		float foffset = overlayFontHeight * 0.01f;
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), overlayFontHeight, 0.0f, 0.0f, TextBackColor);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + foffset, textPos.y - foffset), overlayFontHeight, 0.0f, 0.0f, TextForeColor);
	}

	if(state->isDebugRendering) {
		const FontAsset &font = state->assets.hudFont;
		char text[256];
		fplFormatString(text, fplArrayCount(text), "Enemies: %03zu/%03zu, Bullets: %03zu, Towers: %03zu, Spawners: %03zu", state->enemies.count, state->wave.totalEnemyCount, state->bullets.count, state->towers.activeCount, state->spawners.count);
		Vec4f textColor = V4f(1, 1, 1, 1);
		float padding = MaxTileSize * 0.1f;
		Vec2f textPos = V2f(dim.gridOriginX + padding, dim.gridOriginY + padding);
		float fontHeight = MaxTileSize * 0.5f;
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x, textPos.y), fontHeight, 1.0f, 1.0f, textColor);

		fplFormatString(text, fplArrayCount(text), "Game Memory: %zu / %zu", gameMemory.memory->used, gameMemory.memory->size);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y + fontHeight * 2), fontHeight, -1.0f, 1.0f, textColor);
		fplFormatString(text, fplArrayCount(text), "Render Memory: %zu / %zu", gameMemory.render->lastMemoryUsage, gameMemory.render->memory.size);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y + fontHeight * 1), fontHeight, -1.0f, 1.0f, textColor);
		fplFormatString(text, fplArrayCount(text), "Fps: %.5f, Delta: %.5f", state->framesPerSecond, state->deltaTime);
		PushText(renderState, text, fplGetStringLength(text), &font.desc, &font.texture, V2f(textPos.x + dim.gridWidth - padding * 2.0f, textPos.y), fontHeight, -1.0f, 1.0f, textColor);
	}

	//
	// HUD & Controls
	//
	game::DrawHUD(*state, renderState);
	game::DrawControls(*state, renderState);
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
	GameInput(gameMemory, input);
	GameUpdate(gameMemory, input);
	GameRender(gameMemory, alpha);
}

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

int main(int argc, char *argv[]) {
	GameConfiguration config = {};
	config.title = L"FPL Demo | Towadev";
	config.disableInactiveDetection = true;
	config.noUpdateRenderSeparation = true;
	gamelog::Verbose("Startup game application '%s'", config.title);
	int result = GameMain(config);
	return(result);
}