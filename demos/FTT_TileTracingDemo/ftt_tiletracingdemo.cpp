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
	- C++ Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-06-29
	- Changed to use new keyboard/mouse button state

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C++ always

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#include "final_platform_layer.h"

#include <GL\GL.h>

#define FTT_IMPLEMENTATION
#include "final_tiletrace.hpp"

constexpr int TileMapCountW = 36;
constexpr int TileMapCountH = 62;

constexpr float TileSize = 1.0f;
constexpr float AreaSizeW = TileMapCountW * TileSize;
constexpr float AreaSizeH = TileMapCountH * TileSize;
constexpr float AspectRatio = AreaSizeW / AreaSizeH;

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
	int result = 0;
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Tile-Tracing Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.driver = fplVideoDriverType_OpenGL;
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		fplSetWindowSize(640, 480);
		fplSetWindowPosition(0, 0);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		bool doNextStep = false;

		ftt::TileTracer tracer({ TileMapCountW, TileMapCountH }, &TileMap[0]);

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
						}
					} break;
					default:
						break;
				}
			}

#if 1
			tracer.Next();
#else
			if (doNextStep) {
				tracer.Next();
				doNextStep = false;
			}
#endif

			fplWindowSize windowArea;
			fplGetWindowSize(&windowArea);

			const float halfAreaWidth = AreaSizeW * 0.5f;
			const float halfAreaHeight = AreaSizeH * 0.5f;
			const float tileExt = TileSize * 0.5f;

			// Calculate a letterboxed viewport offset and size
			float viewScale = (float)windowArea.width / AreaSizeW;
			uint32_t viewportWidth = windowArea.width;
			uint32_t viewportHeight = (int)(windowArea.width / AspectRatio);
			if (viewportHeight > windowArea.height) {
				viewportHeight = windowArea.height;
				viewportWidth = (int)(viewportHeight * AspectRatio);
				viewScale = (float)viewportWidth / AreaSizeW;
			}
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
						const ftt::Tile &tile = tracer.GetTile(x, y);
						if (tile.isSolid == -1) {
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
			ftt::Tile *startTile = tracer.GetStartTile();
			if (startTile) {
				glColor3f(1.0f, 0.5f, 1.0f);
				DrawTile(startTile->x, startTile->y, true);
			}

			// Draw open list
			glColor3f(0.0f, 0.0f, 0.0f);
			glLineWidth(2.0f);
			for (uint32_t index = 0, count = (uint32_t)tracer.GetOpenTileCount(); index < count; ++index) {
				ftt::Tile *openTile = tracer.GetOpenTile(index);
				DrawTile(openTile->x, openTile->y, false);
			}
			glLineWidth(1.0f);

			// Draw edges
			glColor3f(1.0f, 0.0f, 0.0f);
			glLineWidth(3.0f);
			for (uint32_t index = 0, count = (uint32_t)tracer.GetEdgeCount(); index < count; ++index) {
				const ftt::Edge &edge = tracer.GetEdge(index);
				if (!edge.isInvalid) {
					const ftt::Vec2i &v0 = tracer.GetVertex(edge.vertIndex0);
					const ftt::Vec2i &v1 = tracer.GetVertex(edge.vertIndex1);
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
			for (uint32_t segmentIndex = 0, count = (uint32_t)tracer.GetChainSegmentCount(); segmentIndex < count; ++segmentIndex) {
				const ftt::ChainSegment &segment = tracer.GetChainSegment(segmentIndex);
				glBegin(GL_LINE_LOOP);
				for (uint32_t vertexIndex = 0; vertexIndex < segment.vertices.size(); ++vertexIndex) {
					const ftt::Vec2i &v = segment.vertices[vertexIndex];
					glVertex2f(-halfAreaWidth + v.x * TileSize, -halfAreaHeight + v.y * TileSize);
				}
				glEnd();
			}
			glLineWidth(1.0f);

			// Draw current tile
			ftt::Tile *curTile = tracer.GetCurrentTile();
			if (curTile) {
				glColor3f(1.0f, 1.0f, 0.0f);
				glLineWidth(2.0f);
				DrawTile(curTile->x, curTile->y, false);
				glLineWidth(1.0f);
			}

			fplVideoFlip();
		}

		fplPlatformRelease();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}