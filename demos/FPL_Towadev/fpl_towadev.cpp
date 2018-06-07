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
Todo:
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_LOG_TO_DEBUGOUT
#include <final_platform_layer.h>

#include <math.h> // fabs

#include <vector>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

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

enum class TileType {
	None,
	EnemyPath,
	SpawnPoint,
	Exit,
};

struct Tile {
	TileType type;
	bool isOccupied;
};

struct Waypoint {
	Vec2i tilePos;
	Vec2f position;
	Vec2f direction;
	struct Waypoint *next;
};

struct CreepData {
	Vec4f color;
	float renderRadius;
	float collisionRadius;
	float speed;
	uint64_t hp;
};

inline CreepData MakeCreepData(const Vec4f &color, const float radius, const float speed, const uint64_t hp) {
	CreepData result;
	result.color = color;
	result.renderRadius = radius;
	result.collisionRadius = radius;
	result.speed = speed;
	result.hp = hp;
	return(result);
}

struct Creep {
	CreepData data;
	Vec2f prevPosition;
	Vec2f position;
	Vec2f facingDirection;
	float speed;
	bool isDead;
	const Waypoint *targetWaypoint;
	Vec2f targetPos;
	bool hasTarget;
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

struct Tower {
	Vec2f position;
	Creep *targetEnemy;
	float detectionRadius;
	bool hasTarget;
};

struct GameState {
	Viewport viewport;
	Camera2D camera;
	Vec2f mouseWorldPos;
	Tile tiles[TotalTileCount];
	Waypoint *firstWaypoint;
	Waypoint *lastWaypoint;
	Creep enemies[100];
	size_t enemyCount;
	CreepSpawner enemySpawner;
	bool isExiting;
};

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

static void GameRelease(GameState &state) {
	fplMemoryFree(state.tiles);
	fglUnloadOpenGL();
}

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

inline Tile *GetTile(GameState &state, const Vec2i &tilePos) {
	if ((tilePos.x >= 0 && tilePos.x < TileCountX) && (tilePos.x >= 0 && tilePos.x < TileCountX)) {
		return &state.tiles[tilePos.y * TileCountX + tilePos.x];
	}
	return nullptr;
}

inline Vec2i SetTile(GameState &state, const int x, const int y, const TileType type) {
	int index = y * TileCountX + x;
	assert(index >= 0 && index < TotalTileCount);
	state.tiles[index].type = type;
	Vec2i result = { x, y };
	return(result);
}

static void SpawnEnemy(GameState &state, const Vec2f &spawnPos, const Vec2f &exitPos, const CreepData &data) {
	assert(state.enemyCount < FPL_ARRAYCOUNT(state.enemies));
	Creep &enemy = state.enemies[state.enemyCount++];
	enemy.data = data;
	enemy.position = enemy.prevPosition = spawnPos;
	enemy.speed = data.speed;
	if (state.firstWaypoint != nullptr) {
		enemy.targetWaypoint = state.firstWaypoint;
		enemy.targetPos = TileToWorld(state.firstWaypoint->tilePos, V2f(TileSize, TileSize) * 0.5f);
	} else {
		enemy.targetWaypoint = nullptr;
		enemy.targetPos = exitPos;
	}
	enemy.facingDirection = Vec2Normalize(enemy.targetPos - enemy.position);
	enemy.hasTarget = true;
}

static void UpdateSpawner(GameState &state, const float deltaTime) {
	if (state.enemySpawner.isActive) {
		assert(state.enemySpawner.remainingCount > 0);
		assert(state.enemySpawner.spawnTimer > 0);
		state.enemySpawner.spawnTimer -= deltaTime;
		if (state.enemySpawner.spawnTimer <= 0) {
			SpawnEnemy(state, state.enemySpawner.spawnPosition, state.enemySpawner.exitPosition, state.enemySpawner.spawnTemplate);
			--state.enemySpawner.remainingCount;
			if (state.enemySpawner.remainingCount == 0) {
				state.enemySpawner.spawnTimer = 0;
				state.enemySpawner.isActive = false;
			} else {
				state.enemySpawner.spawnTimer = state.enemySpawner.cooldown;
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

static bool GameInit(GameState &state) {
	if (!fglLoadOpenGL(true)) {
		return false;
	}

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

	state.camera.scale = 1.0f;
	state.camera.offset.x = 0;
	state.camera.offset.y = 0;

	// Setup tiles
	int ys[] = { 1, TileCountY - 2 };
	for (int y = 0; y < FPL_ARRAYCOUNT(ys); ++y) {
		int row = ys[y];
		for (int x = 1; x < TileCountX - 1; ++x) {
			SetTile(state, x, row, TileType::EnemyPath);
		}
	}
	int xs[] = { 1, TileCountX - 2 };
	for (int x = 0; x < FPL_ARRAYCOUNT(xs); ++x) {
		int col = xs[x];
		for (int y = 1; y < TileCountY - 1; ++y) {
			SetTile(state, col, y, TileType::EnemyPath);
		}
	}
	Vec2i spawnTilePos = SetTile(state, 0, 1, TileType::SpawnPoint);
	Vec2i exitTilePos = SetTile(state, 1, 3, TileType::Exit);
	SetTile(state, 1, 2, TileType::None);

	// Add waypoints
	AddWaypoint(state, V2i(TileCountX - 2, 1), V2f(0, -1));
	AddWaypoint(state, V2i(TileCountX - 2, TileCountY - 2), V2f(-1, 0));
	AddWaypoint(state, V2i(1, TileCountY - 2), V2f(0, 1));

	// Setup spawner
	Vec2f spawnPos = TileToWorld(spawnTilePos, V2f(TileSize, TileSize) * 0.5f);
	Vec2f exitPos = TileToWorld(exitTilePos, V2f(TileSize, TileSize) * 0.5f);
	CreepData enemyTemplate = MakeCreepData(V4f(1, 1, 1, 1), TileSize * 0.25f, 1.5, 100);
	SetupSpawner(state.enemySpawner, spawnPos, exitPos, 3.0f, 1.5f, 20, enemyTemplate);

	return(true);
}

extern GameMemory GameCreate() {
	GameMemory result = {};
	result.size = FPL_MEGABYTES(16);
	result.base = fplMemoryAllocate(result.size);
	if (result.base == nullptr) {
		return {};
	}
	GameState *state = (GameState *)result.base;
	if (!GameInit(*state)) {
		GameDestroy(result);
		state = nullptr;
	}
	return(result);
}

extern void GameDestroy(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	GameRelease(*state);
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
	Vec2i mouseTilePos = WorldToTile(V2f(state->mouseWorldPos.x, state->mouseWorldPos.y - deltaH * 0.5f));
	if (WasPressed(input.mouse.left)) {
		Tile *mouseTile = GetTile(*state, mouseTilePos);
		if (mouseTile != nullptr) {
			if (mouseTile->type == TileType::None && !mouseTile->isOccupied) {
				mouseTile->isOccupied = true;
			}
		}
	}
}

static Vec2i FindTilePosByTile(const GameState &state, const TileType type) {
	for (int y = 0; y < TileCountY; ++y) {
		for (int x = 0; x < TileCountX; ++x) {
			int index = y * TileCountX + x;
			if (state.tiles[index].type == type) {
				return V2i(x, y);
			}
		}
	}
	return V2i(-1, -1);
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
			Vec2i exitTilePos = FindTilePosByTile(state, TileType::Exit);
			assert(exitTilePos.x > -1 && exitTilePos.y > -1);
			creep.targetPos = TileToWorld(exitTilePos, V2f(TileSize, TileSize) * 0.5f);
		}
		creep.hasTarget = true;
		creep.facingDirection = Vec2Normalize(creep.targetPos - creep.position);
	} else {
		creep.hasTarget = false;
		CreepReachedExit(state, creep);
	}
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input, bool isActive) {
	if (!isActive) {
		return;
	}

	GameState *state = (GameState *)gameMemory.base;

	for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if (!enemy.isDead && enemy.hasTarget) {
			Vec2f distance = enemy.targetPos - enemy.position;
			Vec2f direction = Vec2Normalize(distance);
			float minRadius = TileSize * 0.05f;
			enemy.position += direction * enemy.speed * input.deltaTime;
			if (Vec2Dot(distance, distance) <= minRadius * minRadius) {
				SetCreepNextTarget(*state, enemy);
			}
		}
	}

	UpdateSpawner(*state, input.deltaTime);
}

static void DrawTile(int x, int y) {
	float xpos = GridOriginX + (float)x * TileSize;
	float ypos = GridOriginY + (float)y * TileSize;
	glBegin(GL_QUADS);
	glVertex2f(xpos + TileSize, ypos + TileSize);
	glVertex2f(xpos, ypos + TileSize);
	glVertex2f(xpos, ypos);
	glVertex2f(xpos + TileSize, ypos);
	glEnd();
}

static void DrawNormal(const Vec2f &pos, const Vec2f &normal, const float length) {
	glBegin(GL_LINES);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x + normal.x * length, pos.y + normal.y * length);
	glEnd();

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

	for (int yIndex = 0; yIndex < TileCountY; ++yIndex) {
		int y = TileCountY - 1 - yIndex;
		for (int x = 0; x < TileCountX; ++x) {
			const Tile &tile = state->tiles[yIndex * TileCountX + x];
			float xpos = GridOriginX + (float)x * TileSize;
			float ypos = GridOriginY + (float)y * TileSize;
			if (tile.type == TileType::EnemyPath) {
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				DrawTile(x, y);
			} else if (tile.type == TileType::SpawnPoint) {
				glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
				DrawTile(x, y);
			} else if (tile.type == TileType::Exit) {
				glColor4f(0.1f, 1.0f, 0.2f, 1.0f);
				DrawTile(x, y);
			}
			if (tile.isOccupied) {
				glColor4f(1, 1, 0.5f, 1);
				glPointSize(0.25f * state->camera.worldToPixels);
				glBegin(GL_POINTS);
				glVertex2f(xpos + TileSize * 0.5f, ypos + TileSize * 0.5f);
				glEnd();
				glPointSize(1);
			}
		}
	}

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

	const float WaypointDirectionWidth = 0.35f;
	for (Waypoint *waypoint = state->firstWaypoint; waypoint; waypoint = waypoint->next) {
		glColor4f(1, 0, 1, 1);
		glPointSize(0.25f * state->camera.worldToPixels);
		glBegin(GL_POINTS);
		glVertex2f(waypoint->position.x, waypoint->position.y);
		glEnd();
		glPointSize(1);

		glColor4f(1, 1, 1, 1);
		DrawNormal(waypoint->position, waypoint->direction, WaypointDirectionWidth);

		if (waypoint->next == state->firstWaypoint) {
			break;
		}
	}

	glColor4f(1, 1, 0, 1);
	glPointSize(0.25f * state->camera.worldToPixels);
	glBegin(GL_POINTS);
	glVertex2f(state->mouseWorldPos.x, state->mouseWorldPos.y);
	glEnd();
	glPointSize(1);

	float deltaH = WorldHeight - GridHeight;
	Vec2i mouseTilePos = WorldToTile(V2f(state->mouseWorldPos.x, state->mouseWorldPos.y - deltaH * 0.5f));
	Vec2f mouseTileWorld = TileToWorld(mouseTilePos);
	glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(mouseTileWorld.x + TileSize, mouseTileWorld.y + TileSize);
	glVertex2f(mouseTileWorld.x, mouseTileWorld.y + TileSize);
	glVertex2f(mouseTileWorld.x, mouseTileWorld.y);
	glVertex2f(mouseTileWorld.x + TileSize, mouseTileWorld.y);
	glEnd();

	for (size_t enemyIndex = 0; enemyIndex < state->enemyCount; ++enemyIndex) {
		Creep &enemy = state->enemies[enemyIndex];
		if (!enemy.isDead) {
			Vec2f enemyPos = Vec2Lerp(enemy.prevPosition, alpha, enemy.position);
			float rot = atan2f(enemy.facingDirection.y, enemy.facingDirection.x);
			glColor4fv(&enemy.data.color.m[0]);
			glPushMatrix();
			glTranslatef(enemyPos.x, enemyPos.y, 0);
			glRotatef(rot * RadToDeg, 0, 0, 1);
#if 0
			glPointSize(enemy.radius * state->camera.worldToPixels);
			glBegin(GL_POINTS);
			glVertex2f(enemyPos.x, enemyPos.y);
			glEnd();
			glPointSize(1);
#else
			float r = enemy.data.renderRadius;
			glBegin(GL_POLYGON);
			glVertex2f(+r, 0);
			glVertex2f(-r, +r);
			glVertex2f(-r, -r);
			glEnd();
			glPopMatrix();
#endif

			enemy.prevPosition = enemy.position;
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