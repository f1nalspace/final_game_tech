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

#define TileMapCountW 36
#define TileMapCountH 62

#define TileSize 1.0f
#define AreaSizeW (TileMapCountW * TileSize)
#define AreaSizeH (TileMapCountH * TileSize)
#define AspectRatio (AreaSizeW / AreaSizeH)

static uint8_t TileMap[TileMapCountW * TileMapCountH] = {
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

static void DrawTile(const int32_t x, const int32_t y, bool filled) {
	float tileExt = TileSize * 0.5f;
	float tx = -AreaSizeW * 0.5f + x * TileSize + TileSize * 0.5f;
	float ty = -AreaSizeH * 0.5f + y * TileSize + TileSize * 0.5f;
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

int main(int argc, char **args) {
	(void)argc;
	(void)args;
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

	bool doNextStep = false;

	fttTileTracer tracer;
	fttVec2u tileCount;
	tileCount.w = TileMapCountW;
	tileCount.h = TileMapCountH;
	fttInitTileTracer(&tracer, tileCount, &TileMap[0], ftt_null);

	while (fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			switch (ev.type) {
				case fplEventType_Keyboard:
				{
					switch (ev.keyboard.type) {
						case fplKeyboardEventType_Button:
						{
							bool isDown = (ev.keyboard.buttonState >= fplButtonState_Press);
							if (ev.keyboard.mappedKey == fplKey_Space) {
								if (isDown != doNextStep) {
									doNextStep = isDown;
								}
							}
						} break;
						default:
							break;
					}
				} break;
				default:
					break;
			}
		}

#if 1
		fttNextTileTraceStep(&tracer);
#else
		if (doNextStep) {
			fttNextTileTraceStep(&tracer);
			doNextStep = false;
		}
#endif

		fplWindowSize windowArea;
		fplGetWindowSize(&windowArea);

		const float halfAreaWidth = AreaSizeW * 0.5f;
		const float halfAreaHeight = AreaSizeH * 0.5f;

		// Calculate a letterboxed viewport offset and size
		float viewScale = (float)windowArea.width / AreaSizeW;
		uint32_t viewportWidth = windowArea.width;
		uint32_t viewportHeight = (int)(windowArea.width / AspectRatio);
		if (viewportHeight > windowArea.height) {
			viewportHeight = windowArea.height;
			viewportWidth = (int)(viewportHeight * AspectRatio);
			viewScale = (float)viewportWidth / AreaSizeW;
		}
		(void)viewScale;
		int32_t viewportX = (windowArea.width - viewportWidth) / 2;
		int32_t viewportY = (windowArea.height - viewportHeight) / 2;

		glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-halfAreaWidth, halfAreaWidth, -halfAreaHeight, halfAreaHeight, 0.0f, 1.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw tilemap
		for (int y = 0; y < TileMapCountH; ++y) {
			for (int x = 0; x < TileMapCountW; ++x) {
				uint8_t tileValue = TileMap[y * TileMapCountW + x];
				if (tileValue) {
					const fttTile *tile = fttGetTile(&tracer, x, y);
					if (tile->isSolid == -1) {
						glColor3f(0.75f, 0.775f, 0.75f);
					} else {
						glColor3f(0.5f, 0.5f, 0.5f);
					}
					DrawTile(x, y, true);
				}
			}
		}

		// Draw grid
		glLineWidth(1.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (int i = 0; i <= TileMapCountW; ++i) {
			glBegin(GL_LINES);
			glVertex2f(-halfAreaWidth + i * TileSize, -halfAreaHeight);
			glVertex2f(-halfAreaWidth + i * TileSize, -halfAreaHeight + TileMapCountH * TileSize);
			glEnd();
		}
		for (int i = 0; i <= TileMapCountH; ++i) {
			glBegin(GL_LINES);
			glVertex2f(-halfAreaWidth, -halfAreaHeight + i * TileSize);
			glVertex2f(-halfAreaWidth + TileMapCountW * TileSize, -halfAreaHeight + i * TileSize);
			glEnd();
		}

		// Draw start
		fttTile *startTile = fttGetStartTile(&tracer);
		if (startTile) {
			glColor3f(1.0f, 0.5f, 1.0f);
			DrawTile(startTile->x, startTile->y, true);
		}

		// Draw open list
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(2.0f);
		for (uint32_t index = 0, count = (uint32_t)fttGetOpenTileCount(&tracer); index < count; ++index) {
			fttTile *openTile = fttGetOpenTile(&tracer, index);
			DrawTile(openTile->x, openTile->y, false);
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
			DrawTile(curTile->x, curTile->y, false);
			glLineWidth(1.0f);
		}

		fplVideoFlip();
	}

	fttFreeTileTracer(&tracer);

	fglUnloadOpenGL();

	fplPlatformRelease();

	return 0;
}
