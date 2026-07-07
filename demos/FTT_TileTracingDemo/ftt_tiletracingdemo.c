/*
-------------------------------------------------------------------------------
Name:
	FTT | TileTracingDemo

Description:
	This demo shows how to use the "Final Tile Tracing" library.
	FTT is a library for converting a Solid-Tilemap
	into connected line segments - using a image contour tracing algorythmn.
	This is useful to get create proper collision shapes in physics engines
	such as Box2D.

Requirements:
	- C17 Compiler
	- Final Platform Layer
	- Final Dynamic OpenGL

Author:
	Torsten Spaete

Changelog:
	## 2026-07-07
	- Added a line-stroke font, a minimal immediate-mode UI panel and mouse input
	- Added pause/resume/step control over the tile tracer
	- Added scenario switching between a small map, the original map and a randomly generated dungeon

	## 2026-06-04
	- Switched to the pure C17 final_tiletrace.h library and C api

	## 2026-05-10
	- Use final_dynamic_opengl instead of gl.h
	- No more size and position overwrite of window

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-06-29
	- Changed to use new keyboard/mouse button state

	## 2018-04-23:
	- Initial creation of this description block

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FTT_IMPLEMENTATION
#include "final_tiletrace.h"

#include "linefont.h"
#include "ui.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define TileSize 1.0f

enum {
	Scenario0Width = 12,
	Scenario0Height = 10,

	MaxTileMapCountW = 36,
	MaxTileMapCountH = 62,

	UiPanelWidthPixels = 240,

	DungeonRoomCount = 14,
	DungeonRoomMinSize = 3,
	DungeonRoomMaxSize = 7,
	DungeonBorderMargin = 1,
};

static const uint8_t Scenario0_TileMap[Scenario0Width * Scenario0Height] = {
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,0,0,0,0,1,1,0,1,
	1,0,1,0,0,0,0,0,0,1,0,1,
	1,0,0,0,0,1,1,0,0,0,0,1,
	1,0,0,0,0,1,1,0,0,0,0,1,
	1,0,1,0,0,0,0,0,0,1,0,1,
	1,0,1,1,0,0,0,0,1,1,0,1,
	1,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
};

static const uint8_t Scenario1_TileMap[MaxTileMapCountW * MaxTileMapCountH] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,1,1,
	1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,0,0,1,
	1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,
	1,1,1,1,0,0,1,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,
	1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,
	1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,
	1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,
	1,0,1,1,1,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,
	1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,
	1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,0,0,1,
	1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,
	1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,1,
	1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,1,1,1,
	1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1,1,1,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,
	1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,1,1,1,
	1,1,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,1,1,1,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
	1,1,0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
	1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
	1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,1,1,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,1,
	1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,1,
	1,1,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,
	1,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,1,
	1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,0,0,1,
	1,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1,1,1,0,1,1,0,1,1,1,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
	1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,
	1,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static uint8_t Scenario2_TileMap[MaxTileMapCountW * MaxTileMapCountH];

typedef struct fttScenario {
	const char *name;
	uint32_t width;
	uint32_t height;
	const uint8_t *tiles;
} fttScenario;

enum {
	Scenario_Simple = 0,
	Scenario_Current = 1,
	Scenario_Random = 2,
	ScenarioCount = 3,
};

static fttScenario Scenarios[ScenarioCount] = {
	{ "0 Simple", Scenario0Width, Scenario0Height, Scenario0_TileMap },
	{ "1 Current", MaxTileMapCountW, MaxTileMapCountH, Scenario1_TileMap },
	{ "2 Random", MaxTileMapCountW, MaxTileMapCountH, Scenario2_TileMap },
};

static uint32_t RandomRange(uint32_t minInclusive, uint32_t maxInclusive) {
	uint32_t span = maxInclusive - minInclusive + 1;
	return minInclusive + ((uint32_t)rand() % span);
}

static void CarveRoom(uint8_t *tiles, uint32_t mapWidth, uint32_t mapHeight, uint32_t roomX, uint32_t roomY, uint32_t roomWidth, uint32_t roomHeight) {
	uint32_t maxY = mapHeight - DungeonBorderMargin;
	uint32_t maxX = mapWidth - DungeonBorderMargin;
	for (uint32_t y = roomY; y < roomY + roomHeight && y < maxY; ++y) {
		for (uint32_t x = roomX; x < roomX + roomWidth && x < maxX; ++x) {
			tiles[y * mapWidth + x] = 0;
		}
	}
}

static void CarveCorridor(uint8_t *tiles, uint32_t mapWidth, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	int32_t x = x0;
	int32_t y = y0;
	while (x != x1) {
		tiles[(uint32_t)y * mapWidth + (uint32_t)x] = 0;
		x += (x1 > x) ? 1 : -1;
	}
	while (y != y1) {
		tiles[(uint32_t)y * mapWidth + (uint32_t)x] = 0;
		y += (y1 > y) ? 1 : -1;
	}
	tiles[(uint32_t)y * mapWidth + (uint32_t)x] = 0;
}

static void GenerateDungeon(uint8_t *tiles, uint32_t mapWidth, uint32_t mapHeight) {
	for (uint32_t index = 0; index < mapWidth * mapHeight; ++index) {
		tiles[index] = 1;
	}

	int32_t previousCenterX = 0;
	int32_t previousCenterY = 0;
	bool hasPreviousCenter = false;

	for (uint32_t roomIndex = 0; roomIndex < DungeonRoomCount; ++roomIndex) {
		uint32_t roomWidth = RandomRange(DungeonRoomMinSize, DungeonRoomMaxSize);
		uint32_t roomHeight = RandomRange(DungeonRoomMinSize, DungeonRoomMaxSize);
		uint32_t roomX = RandomRange(DungeonBorderMargin, mapWidth - roomWidth - DungeonBorderMargin - 1);
		uint32_t roomY = RandomRange(DungeonBorderMargin, mapHeight - roomHeight - DungeonBorderMargin - 1);

		CarveRoom(tiles, mapWidth, mapHeight, roomX, roomY, roomWidth, roomHeight);

		int32_t centerX = (int32_t)(roomX + roomWidth / 2);
		int32_t centerY = (int32_t)(roomY + roomHeight / 2);
		if (hasPreviousCenter) {
			CarveCorridor(tiles, mapWidth, previousCenterX, previousCenterY, centerX, centerY);
		}
		previousCenterX = centerX;
		previousCenterY = centerY;
		hasPreviousCenter = true;
	}
}

static void DrawTile(const int32_t x, const int32_t y, bool filled, float areaWidth, float areaHeight) {
	float tileExt = TileSize * 0.5f;
	float tx = -areaWidth * 0.5f + x * TileSize + TileSize * 0.5f;
	float ty = -areaHeight * 0.5f + y * TileSize + TileSize * 0.5f;
	glPushMatrix();
	glTranslatef(tx, ty, 0.0f);
	glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
	glVertex2f(tileExt, tileExt);
	glVertex2f(-tileExt, tileExt);
	glVertex2f(-tileExt, -tileExt);
	glVertex2f(tileExt, -tileExt);
	glEnd();
	glPopMatrix();
}

static void InitScenarioTracer(fttTileTracer *tracer, int scenarioIndex, bool freeExisting) {
	if (freeExisting) {
		fttFreeTileTracer(tracer);
	}
	fttVec2u tileCount;
	tileCount.w = Scenarios[scenarioIndex].width;
	tileCount.h = Scenarios[scenarioIndex].height;
	fttInitTileTracer(tracer, tileCount, Scenarios[scenarioIndex].tiles, ftt_null);
}

static const char *StepName(fttStep step) {
	switch (step) {
		case FTT_STEP_NONE: return "NONE";
		case FTT_STEP_FIND_START: return "FIND START";
		case FTT_STEP_GET_NEXT_OPEN_TILE: return "NEXT OPEN TILE";
		case FTT_STEP_FIND_NEXT_TILE: return "FIND NEXT TILE";
		case FTT_STEP_ROTATE_FORWARD: return "ROTATE FORWARD";
		case FTT_STEP_TRAVERSE_FIND_STARTING_EDGE: return "FIND START EDGE";
		case FTT_STEP_TRAVERSE_NEXT_EDGE: return "NEXT EDGE";
		case FTT_STEP_DONE: return "DONE";
		default: return "UNKNOWN";
	}
}

int main(int argc, char **args) {
	(void)argc;
	(void)args;

	srand((unsigned int)time(NULL));
	GenerateDungeon(Scenario2_TileMap, MaxTileMapCountW, MaxTileMapCountH);

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Tile-Tracing Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	if (!fplPlatformInit(fplInitFlags_Video, &settings)) {
		return -1;
	}
	if (!fglLoadOpenGL(true)) {
		fplPlatformRelease();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	int activeScenarioIndex = Scenario_Current;
	bool isPaused = false;
	bool stepRequested = false;
	bool showGrid = true;

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	bool mouseDown = false;
	fttUiState ui = { 0 };

	fttTileTracer tracer;
	InitScenarioTracer(&tracer, activeScenarioIndex, false);

	while (fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			switch (ev.type) {
				case fplEventType_Keyboard:
				{
					if (ev.keyboard.type == fplKeyboardEventType_Button) {
						bool isDown = (ev.keyboard.buttonState >= fplButtonState_Press);
						if (ev.keyboard.mappedKey == fplKey_Space && isDown) {
							isPaused = !isPaused;
						}
					}
				} break;
				case fplEventType_Mouse:
				{
					if (ev.mouse.type == fplMouseEventType_Move) {
						mouseX = (float)ev.mouse.mouseX;
						mouseY = (float)ev.mouse.mouseY;
					} else if (ev.mouse.type == fplMouseEventType_Button) {
						if (ev.mouse.mouseButton == fplMouseButtonType_Left) {
							mouseDown = (ev.mouse.buttonState >= fplButtonState_Press);
						}
					}
				} break;
				default:
					break;
			}
		}

		if (!isPaused || stepRequested) {
			fttNextTileTraceStep(&tracer);
			stepRequested = false;
		}

		fplWindowSize windowArea;
		fplGetWindowSize(&windowArea);

		const fttScenario *activeScenario = &Scenarios[activeScenarioIndex];
		float areaSizeW = (float)activeScenario->width * TileSize;
		float areaSizeH = (float)activeScenario->height * TileSize;
		float aspectRatio = areaSizeW / areaSizeH;
		const float halfAreaWidth = areaSizeW * 0.5f;
		const float halfAreaHeight = areaSizeH * 0.5f;

		uint32_t tileViewAreaWidth = (windowArea.width > (uint32_t)UiPanelWidthPixels) ? (windowArea.width - (uint32_t)UiPanelWidthPixels) : 1;
		uint32_t tileViewAreaHeight = windowArea.height;

		// Calculate a letterboxed viewport offset and size within the area left of the UI panel
		uint32_t viewportWidth = tileViewAreaWidth;
		uint32_t viewportHeight = (uint32_t)((float)tileViewAreaWidth / aspectRatio);
		if (viewportHeight > tileViewAreaHeight) {
			viewportHeight = tileViewAreaHeight;
			viewportWidth = (uint32_t)((float)viewportHeight * aspectRatio);
		}
		int32_t viewportX = (int32_t)UiPanelWidthPixels + (int32_t)(tileViewAreaWidth - viewportWidth) / 2;
		int32_t viewportY = (int32_t)(tileViewAreaHeight - viewportHeight) / 2;

		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-halfAreaWidth, halfAreaWidth, -halfAreaHeight, halfAreaHeight, 0.0f, 1.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw tilemap
		for (uint32_t y = 0; y < activeScenario->height; ++y) {
			for (uint32_t x = 0; x < activeScenario->width; ++x) {
				uint8_t tileValue = activeScenario->tiles[y * activeScenario->width + x];
				if (tileValue) {
					const fttTile *tile = fttGetTile(&tracer, (int32_t)x, (int32_t)y);
					if (tile->isSolid == -1) {
						glColor3f(0.75f, 0.775f, 0.75f);
					} else {
						glColor3f(0.5f, 0.5f, 0.5f);
					}
					DrawTile((int32_t)x, (int32_t)y, true, areaSizeW, areaSizeH);
				}
			}
		}

		// Draw grid
		if (showGrid) {
			glLineWidth(1.0f);
			glColor3f(0.0f, 0.0f, 0.0f);
			for (uint32_t i = 0; i <= activeScenario->width; ++i) {
				glBegin(GL_LINES);
				glVertex2f(-halfAreaWidth + i * TileSize, -halfAreaHeight);
				glVertex2f(-halfAreaWidth + i * TileSize, -halfAreaHeight + activeScenario->height * TileSize);
				glEnd();
			}
			for (uint32_t i = 0; i <= activeScenario->height; ++i) {
				glBegin(GL_LINES);
				glVertex2f(-halfAreaWidth, -halfAreaHeight + i * TileSize);
				glVertex2f(-halfAreaWidth + activeScenario->width * TileSize, -halfAreaHeight + i * TileSize);
				glEnd();
			}
		}

		// Draw start
		fttTile *startTile = fttGetStartTile(&tracer);
		if (startTile) {
			glColor3f(1.0f, 0.5f, 1.0f);
			DrawTile(startTile->x, startTile->y, true, areaSizeW, areaSizeH);
		}

		// Draw open list
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(2.0f);
		for (uint32_t index = 0, count = (uint32_t)fttGetOpenTileCount(&tracer); index < count; ++index) {
			fttTile *openTile = fttGetOpenTile(&tracer, index);
			DrawTile(openTile->x, openTile->y, false, areaSizeW, areaSizeH);
		}
		glLineWidth(1.0f);

		// Draw edges
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(3.0f);
		for (uint32_t index = 0, count = (uint32_t)fttGetEdgeCount(&tracer); index < count; ++index) {
			const fttEdge *edge = fttGetEdge(&tracer, index);
			if (!edge->isInvalid) {
				fttVec2i v0 = fttGetVertex(&tracer, edge->vertIndex0);
				fttVec2i v1 = fttGetVertex(&tracer, edge->vertIndex1);
				glBegin(GL_LINES);
				glVertex2f(-halfAreaWidth + v0.x * TileSize, -halfAreaHeight + v0.y * TileSize);
				glVertex2f(-halfAreaWidth + v1.x * TileSize, -halfAreaHeight + v1.y * TileSize);
				glEnd();
			}
		}
		glLineWidth(1.0f);

		// Draw chain segments
		glColor3f(0.0f, 1.0f, 0.0f);
		glLineWidth(3.0f);
		for (uint32_t segmentIndex = 0, count = (uint32_t)fttGetChainSegmentCount(&tracer); segmentIndex < count; ++segmentIndex) {
			const fttChainSegment *segment = fttGetChainSegment(&tracer, segmentIndex);
			glBegin(GL_LINE_LOOP);
			for (uint32_t vertexIndex = 0, vertexCount = (uint32_t)fttGetChainSegmentVertexCount(segment); vertexIndex < vertexCount; ++vertexIndex) {
				fttVec2i v = fttGetChainSegmentVertex(segment, vertexIndex);
				glVertex2f(-halfAreaWidth + v.x * TileSize, -halfAreaHeight + v.y * TileSize);
			}
			glEnd();
		}
		glLineWidth(1.0f);

		// Draw current tile
		fttTile *curTile = fttGetCurrentTile(&tracer);
		if (curTile) {
			glColor3f(1.0f, 1.0f, 0.0f);
			glLineWidth(2.0f);
			DrawTile(curTile->x, curTile->y, false, areaSizeW, areaSizeH);
			glLineWidth(1.0f);
		}

		// Draw the UI panel in a full-window, y-down pixel-space projection
		glViewport(0, 0, (int32_t)windowArea.width, (int32_t)windowArea.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, (float)windowArea.width, (float)windowArea.height, 0.0f, -1.0f, 1.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		UiBeginFrame(&ui, mouseX, mouseY, mouseDown);
		UiBeginPanel(&ui, 0.0f, 0.0f, (float)UiPanelWidthPixels, (float)windowArea.height);

		UiLabel(&ui, "FTT Tile Tracer");
		UiSpacer(&ui);

		UiLabel(&ui, "Scenario");
		int newScenarioIndex = activeScenarioIndex;
		for (int scenarioIndex = 0; scenarioIndex < ScenarioCount; ++scenarioIndex) {
			UiRadioButton(&ui, Scenarios[scenarioIndex].name, &newScenarioIndex, scenarioIndex);
		}
		if (newScenarioIndex != activeScenarioIndex) {
			activeScenarioIndex = newScenarioIndex;
			InitScenarioTracer(&tracer, activeScenarioIndex, true);
		}
		UiSpacer(&ui);

		UiLabel(&ui, "Playback");
		if (UiButton(&ui, isPaused ? "Resume" : "Pause")) {
			isPaused = !isPaused;
		}
		if (UiButton(&ui, "Step")) {
			stepRequested = true;
		}
		if (UiButton(&ui, "Reset")) {
			InitScenarioTracer(&tracer, activeScenarioIndex, true);
		}
		UiSpacer(&ui);

		UiCheckbox(&ui, "Show Grid", &showGrid);

		if (activeScenarioIndex == Scenario_Random) {
			UiSpacer(&ui);
			if (UiButton(&ui, "Regenerate")) {
				GenerateDungeon(Scenario2_TileMap, MaxTileMapCountW, MaxTileMapCountH);
				InitScenarioTracer(&tracer, activeScenarioIndex, true);
			}
		}

		// Live tracer-state overlay, drawn over the top-left of the tile-trace output right next to the panel
		{
			const float overlayMarginPixels = 10.0f;
			const float overlayPaddingPixels = 8.0f;
			const float overlayTextScale = 1.5f;
			const float overlayTextThickness = 1.25f;
			const float overlayLineGapPixels = 6.0f;
			const float overlayLineHeightPixels = (float)FontGridRows * overlayTextScale + overlayLineGapPixels;
			const size_t overlayMaxLineLength = 48;

			char overlayLines[8][48];
			int overlayLineCount = 0;

			uint32_t mapWidth = tracer.tileCount.w;
			uint32_t mapHeight = tracer.tileCount.h;
			fttStep currentStep = tracer.curStep;
			const char *stepName = StepName(currentStep);
			const char *statusText;
			if (currentStep == FTT_STEP_DONE) {
				statusText = "DONE";
			} else if (isPaused) {
				statusText = "PAUSED";
			} else {
				statusText = "RUNNING";
			}
			uint32_t openCount = (uint32_t)fttGetOpenTileCount(&tracer);
			uint32_t edgeCount = (uint32_t)fttGetEdgeCount(&tracer);
			uint32_t vertexCount = (uint32_t)fttGetVertexCount(&tracer);
			uint32_t chainCount = (uint32_t)fttGetChainSegmentCount(&tracer);
			fttTile *overlayCurTile = fttGetCurrentTile(&tracer);

			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Scenario: %s", activeScenario->name);
			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Map: %u x %u", mapWidth, mapHeight);
			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Status: %s", statusText);
			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Step: %s", stepName);
			if (overlayCurTile) {
				int32_t curTileX = overlayCurTile->x;
				int32_t curTileY = overlayCurTile->y;
				snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Cur: %d %d", curTileX, curTileY);
			} else {
				snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Cur: none");
			}
			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Open: %u  Edges: %u", openCount, edgeCount);
			snprintf(overlayLines[overlayLineCount++], overlayMaxLineLength, "Verts: %u  Chains: %u", vertexCount, chainCount);

			float overlayMaxTextWidth = 0.0f;
			for (int lineIndex = 0; lineIndex < overlayLineCount; ++lineIndex) {
				float lineWidth = MeasureLineTextWidthZ(overlayLines[lineIndex], overlayTextScale);
				if (lineWidth > overlayMaxTextWidth) {
					overlayMaxTextWidth = lineWidth;
				}
			}

			float overlayBoxX = (float)UiPanelWidthPixels + overlayMarginPixels;
			float overlayBoxY = overlayMarginPixels;
			float overlayBoxWidth = overlayMaxTextWidth + overlayPaddingPixels * 2.0f;
			float overlayBoxHeight = (float)overlayLineCount * overlayLineHeightPixels + overlayPaddingPixels * 2.0f;

			glColor3f(0.13f, 0.13f, 0.16f);
			UiDrawRectFilled(overlayBoxX, overlayBoxY, overlayBoxWidth, overlayBoxHeight);
			glColor3f(0.4f, 0.4f, 0.46f);
			UiDrawRectOutline(overlayBoxX, overlayBoxY, overlayBoxWidth, overlayBoxHeight, UiWidgetOutlineThickness);

			float overlayTextX = overlayBoxX + overlayPaddingPixels;
			float overlayTextY = overlayBoxY + overlayPaddingPixels;
			glColor3f(0.85f, 0.9f, 0.7f);
			for (int lineIndex = 0; lineIndex < overlayLineCount; ++lineIndex) {
				DrawLineTextZ(overlayLines[lineIndex], overlayTextX, overlayTextY, overlayTextScale, overlayTextThickness);
				overlayTextY += overlayLineHeightPixels;
			}
		}

		fplVideoFlip();
	}

	fttFreeTileTracer(&tracer);

	fglUnloadOpenGL();

	fplPlatformRelease();

	return 0;
}
