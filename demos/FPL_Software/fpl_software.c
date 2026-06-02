/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Software

Description:
	Software rendering demo with multiple switchable scenarios.

	Scenarios (cycle with F1):
	  1. Planes          - 4 planes + a bouncing quad (original demo, Phase 1).
	  2. Pixel Primitives - filled quads, H/V lines, filled circles + clipping.
	  3. Ortho Pipeline   - orthographic projection + model matrix (OpenGL style).
	  4. Vector Shapes    - float quads/lines/circles, filled/stroked, thickness.
	  5. Textured Quad    - stb-loaded texture mapped on a quad with UV control.
	  6. Sub-Pixel Motion - smooth fractional motion, nearest vs bilinear.

	F2 toggles bilinear filtering (textured scenarios).

	The pixel layer (Phase 1) is top-down (y-down). The pipeline layer (Phase 2)
	uses OpenGL coordinates (y-up, glOrtho-style, texture v-up).

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- stb_image

Author:
	Torsten Spaete

Changelog:
	## 2026-06-02
	- Pixel rendering pipeline (quads, lines, circles, triangles, texture-mapped)

	## 2025-01-28
	- Fixed makefiles for CC/CMake was broken

	## 2021-05-16
	- Use fplPollEvents() instead

	## 2020-04-20
	- Much better rendering

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-05-11:
	- Added CMakeLists.txt
	- Fixed Makefile was not working anymore

	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <final_math.h>

// final_graphics.h includes stb_image.h; we only request its implementation here.
#define STB_IMAGE_IMPLEMENTATION

#define FINAL_GRAPHICS_IMPLEMENTATION
#define FINAL_GRAPHICS_USE_STB_IMAGE
#include <final_graphics.h>

//
// Shared helpers
//

typedef struct RandomSeries {
	uint16_t index;
} RandomSeries;

static uint16_t RandomU16(RandomSeries *series) {
	series->index ^= (series->index << 13);
	series->index ^= (series->index >> 9);
	series->index ^= (series->index << 7);
	return (series->index);
}

static uint8_t RandomByte(RandomSeries *series) {
	uint8_t result = RandomU16(series) % UCHAR_MAX;
	return(result);
}

static const uint32_t kBackColor = 0xFF3A2727; // 0xAARRGGBB dark backdrop

static void ClearBackbuffer(fplVideoBackBuffer *bb, uint32_t color) {
	uint32_t count = bb->width * bb->height;
	for (uint32_t i = 0; i < count; ++i) {
		bb->pixels[i] = color;
	}
}

// Builds an owned RGBA checker texture (high frequency to show filtering).
static Texture2D MakeCheckerTexture(int size, int cells, uint32_t a, uint32_t b) {
	uint32_t *pixels = (uint32_t *)fplMemoryAllocate((size_t)size * size * sizeof(uint32_t));
	int cell = size / cells;
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			int cx = x / cell;
			int cy = y / cell;
			pixels[y * size + x] = ((cx + cy) & 1) ? a : b;
		}
	}
	Texture2D tex;
	TextureInitFromMemory(&tex, pixels, size, size);
	tex.ownsPixels = true;
	return(tex);
}

//
// Scenario state
//

typedef enum ScenarioType {
	ScenarioType_Planes = 0,
	ScenarioType_PixelPrimitives,
	ScenarioType_OrthoPipeline,
	ScenarioType_VectorShapes,
	ScenarioType_TexturedQuad,
	ScenarioType_SubPixelMotion,
	ScenarioType_Count
} ScenarioType;

static const char *ScenarioName(ScenarioType type) {
	switch (type) {
		case ScenarioType_Planes: return "Planes";
		case ScenarioType_PixelPrimitives: return "Pixel Primitives";
		case ScenarioType_OrthoPipeline: return "Ortho Pipeline";
		case ScenarioType_VectorShapes: return "Vector Shapes";
		case ScenarioType_TexturedQuad: return "Textured Quad";
		case ScenarioType_SubPixelMotion: return "Sub-Pixel Motion";
		default: return "Unknown";
	}
}

typedef struct PlanesScenario {
	Vec2f rectPos;
	Vec2f rectVel;
	Vec2f rectRadius;
	float margin;
	RandomSeries noise;
} PlanesScenario;

typedef struct PixelPrimitivesScenario {
	float time;
	Vec2f ballPos;
	Vec2f ballVel;
} PixelPrimitivesScenario;

typedef struct OrthoPipelineScenario {
	float angle;
} OrthoPipelineScenario;

typedef struct VectorShapesScenario {
	float time;
} VectorShapesScenario;

typedef struct TexturedQuadScenario {
	float time;
	Texture2D texture;
} TexturedQuadScenario;

typedef struct SubPixelMotionScenario {
	float time;
	Texture2D texture;
} SubPixelMotionScenario;

typedef struct AppScenarios {
	ScenarioType current;
	union {
		PlanesScenario planes;
		PixelPrimitivesScenario pixelPrimitives;
		OrthoPipelineScenario orthoPipeline;
		VectorShapesScenario vectorShapes;
		TexturedQuadScenario texturedQuad;
		SubPixelMotionScenario subPixelMotion;
	} data;
} AppScenarios;

// Attempts to load the fpl logo, falling back to a procedural checker.
static Texture2D LoadDemoTexture(void) {
	const char *candidates[] = {
		"resources/fpl_logo_512x512.png",
		"../resources/fpl_logo_512x512.png",
		"demos/resources/fpl_logo_512x512.png",
		"../../demos/resources/fpl_logo_512x512.png",
	};
	Texture2D tex = fplZeroInit;
	for (size_t i = 0; i < fplArrayCount(candidates); ++i) {
		if (fplFileExists(candidates[i])) {
			if (TextureLoadFromFile(&tex, candidates[i])) {
				return(tex);
			}
		}
	}
	return MakeCheckerTexture(256, 8, 0xFFE0E0E0, 0xFF404040);
}

//
// Scenario lifecycle
//

static void ScenarioInit(AppScenarios *app, ScenarioType type, fplVideoBackBuffer *bb) {
	fplMemoryClear(&app->data, sizeof(app->data));
	app->current = type;
	switch (type) {
		case ScenarioType_Planes: {
			PlanesScenario *s = &app->data.planes;
			s->margin = bb->width / 50.0f;
			s->rectRadius = V2fInit(bb->width / 25.0f, bb->width / 25.0f);
			s->rectVel = V2fInit(s->rectRadius.x * 4.0f, s->rectRadius.y * 4.0f);
			s->rectPos = V2fInit(s->rectRadius.x, s->rectRadius.y);
			s->noise.index = 1337;
		} break;

		case ScenarioType_PixelPrimitives: {
			PixelPrimitivesScenario *s = &app->data.pixelPrimitives;
			s->ballPos = V2fInit(bb->width * 0.5f, bb->height * 0.5f);
			s->ballVel = V2fInit(140.0f, 110.0f);
		} break;

		case ScenarioType_TexturedQuad: {
			TexturedQuadScenario *s = &app->data.texturedQuad;
			s->texture = LoadDemoTexture();
		} break;

		case ScenarioType_SubPixelMotion: {
			SubPixelMotionScenario *s = &app->data.subPixelMotion;
			s->texture = MakeCheckerTexture(64, 8, 0xFFFFD040, 0xFF2040A0);
		} break;

		default:
			break;
	}
}

static void ScenarioCleanup(AppScenarios *app) {
	switch (app->current) {
		case ScenarioType_TexturedQuad:
			TextureRelease(&app->data.texturedQuad.texture);
			break;
		case ScenarioType_SubPixelMotion:
			TextureRelease(&app->data.subPixelMotion.texture);
			break;
		default:
			break;
	}
}

//
// Scenario: Planes (Phase 1 baseline)
//

static void UpdatePlanes(PlanesScenario *s, fplVideoBackBuffer *bb, float dt) {
	float worldWidth = bb->width - s->margin * 2;
	float worldHeight = bb->height - s->margin * 2;

	s->rectPos = V2fAddMultScalar(s->rectPos, s->rectVel, dt);

	if (s->rectPos.y < s->rectRadius.y) {
		s->rectPos.y = s->rectRadius.y;
		s->rectVel.y *= -1;
	} else if (s->rectPos.y > (worldHeight - s->rectRadius.y)) {
		s->rectPos.y = worldHeight - s->rectRadius.y;
		s->rectVel.y *= -1;
	}
	if (s->rectPos.x < s->rectRadius.x) {
		s->rectPos.x = s->rectRadius.x;
		s->rectVel.x *= -1;
	} else if (s->rectPos.x > (worldWidth - s->rectRadius.x)) {
		s->rectPos.x = worldWidth - s->rectRadius.x;
		s->rectVel.x *= -1;
	}
}

static void RenderPlanes(PlanesScenario *s, fplVideoBackBuffer *bb) {
	ClearBackbuffer(bb, kBackColor);

	// Noise
	uint32_t stepX = 5;
	uint32_t stepY = 5;
	for (uint32_t y = 0; y < bb->height; y += stepY) {
		uint32_t *p = bb->pixels + (size_t)y * bb->width;
		for (uint32_t x = 0; x < bb->width; x += stepX) {
			uint8_t r = RandomByte(&s->noise);
			uint8_t g = RandomByte(&s->noise);
			uint8_t b = RandomByte(&s->noise);
			p[x] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
		}
	}

	float margin = s->margin;
	float left = margin;
	float right = margin + (bb->width - margin * 2);
	float top = margin;
	float bottom = margin + (bb->height - margin * 2);

	// Planes
	BackbufferDrawHLine(bb, left, right, top, 0xFFFFFF00);
	BackbufferDrawHLine(bb, left, right, bottom, 0xFFFFFF00);
	BackbufferDrawVLine(bb, left, top, bottom, 0xFFFFFF00);
	BackbufferDrawVLine(bb, right, top, bottom, 0xFFFFFF00);

	// Moving quad
	float rx = margin + s->rectPos.x - s->rectRadius.x;
	float ry = margin + s->rectPos.y - s->rectRadius.y;
	BackbufferFillQuad(bb, rx, ry, rx + s->rectRadius.x * 2, ry + s->rectRadius.y * 2, 0xFFFFFFFF);
}

//
// Scenario: Pixel Primitives (Phase 1.1-1.3)
//

static void UpdatePixelPrimitives(PixelPrimitivesScenario *s, fplVideoBackBuffer *bb, float dt) {
	s->time += dt;
	s->ballPos = V2fAddMultScalar(s->ballPos, s->ballVel, dt);
	float r = 60.0f;
	if (s->ballPos.x < r || s->ballPos.x > bb->width - r) {
		s->ballVel.x *= -1;
	}
	if (s->ballPos.y < r || s->ballPos.y > bb->height - r) {
		s->ballVel.y *= -1;
	}
}

static void RenderPixelPrimitives(PixelPrimitivesScenario *s, fplVideoBackBuffer *bb) {
	ClearBackbuffer(bb, kBackColor);
	float w = (float)bb->width;
	float h = (float)bb->height;

	// Filled quads, including ones clipped against edges.
	BackbufferFillQuad(bb, 40, 40, 200, 160, 0xFF4080FF);
	BackbufferFillQuad(bb, -60, h * 0.5f - 50, 120, h * 0.5f + 50, 0xFFFF6040); // off left
	BackbufferFillQuad(bb, w - 120, h - 80, w + 80, h + 80, 0xFF40C060);        // off bottom-right

	// Horizontal / vertical lines spanning past the edges.
	BackbufferDrawHLine(bb, -50, w + 50, h * 0.25f, 0xFFFFFFFF);
	BackbufferDrawVLine(bb, w * 0.75f, -50, h + 50, 0xFFFFFFFF);

	// Filled circles: inside, clipped at corner, and animated.
	BackbufferFillCircle(bb, w * 0.5f, h * 0.5f, 70, 0xFFFFD040);
	BackbufferFillCircle(bb, 0, 0, 90, 0xFFC060FF);  // clipped top-left
	BackbufferFillCircle(bb, s->ballPos.x, s->ballPos.y, 60, 0xFF60D0FF);
}

//
// Scenario: Ortho Pipeline (Phase 2.1-2.2)
//

static void UpdateOrthoPipeline(OrthoPipelineScenario *s, float dt) {
	s->angle += dt;
}

static void RenderOrthoPipeline(OrthoPipelineScenario *s, fplVideoBackBuffer *bb) {
	ClearBackbuffer(bb, kBackColor);

	// Custom world coordinate system: x in [-10,10], y in [-7.5,7.5], y-up.
	PixelPipeline pl;
	PipelineInitOrtho(&pl, bb, -10.0f, 10.0f, -7.5f, 7.5f);

	// World axes (identity model).
	PipelineResetModel(&pl);
	PipelineDrawLine(&pl, bb, V2fInit(-10, 0), V2fInit(10, 0), 0xFF802020, 2.0f);
	PipelineDrawLine(&pl, bb, V2fInit(0, -7.5f), V2fInit(0, 7.5f), 0xFF208020, 2.0f);

	// A unit quad placed via the model matrix: orbit + pulsing scale.
	float radius = 5.0f;
	Vec2f pos = V2fInit(F32Cos(s->angle) * radius, F32Sin(s->angle) * radius);
	float sc = 1.5f + 0.5f * F32Sin(s->angle * 2.0f);
	PipelineSetModel(&pl, pos, V2fInit(sc, sc));
	PipelineDrawQuad(&pl, bb,
		V2fInit(-1, -1), V2fInit(1, -1), V2fInit(1, 1), V2fInit(-1, 1),
		0xFFFFD040, true, 0.0f);

	// A second quad orbiting the opposite way, stroked.
	Vec2f pos2 = V2fInit(F32Cos(-s->angle * 1.3f) * 3.0f, F32Sin(-s->angle * 1.3f) * 3.0f);
	PipelineSetModel(&pl, pos2, V2fInit(1.0f, 1.0f));
	PipelineDrawQuad(&pl, bb,
		V2fInit(-1, -1), V2fInit(1, -1), V2fInit(1, 1), V2fInit(-1, 1),
		0xFF40D0FF, false, 3.0f);
}

//
// Scenario: Vector Shapes (Phase 2.2-2.4)
//

static void UpdateVectorShapes(VectorShapesScenario *s, float dt) {
	s->time += dt;
}

static void RenderVectorShapes(VectorShapesScenario *s, fplVideoBackBuffer *bb) {
	ClearBackbuffer(bb, kBackColor);

	// World matches the pixel grid (OpenGL y-up): x in [0,w], y in [0,h].
	PixelPipeline pl;
	PipelineInitOrtho(&pl, bb, 0.0f, (float)bb->width, 0.0f, (float)bb->height);
	PipelineResetModel(&pl);

	float w = (float)bb->width;
	float h = (float)bb->height;
	float t = s->time;

	// Rotating filled quad.
	float a = t;
	float hs = 80.0f;
	Vec2f c = V2fInit(w * 0.25f, h * 0.6f);
	Vec2f corners[4];
	float cs = F32Cos(a);
	float sn = F32Sin(a);
	float ox[4] = { -hs, hs, hs, -hs };
	float oy[4] = { -hs, -hs, hs, hs };
	for (int i = 0; i < 4; ++i) {
		corners[i] = V2fInit(c.x + ox[i] * cs - oy[i] * sn, c.y + ox[i] * sn + oy[i] * cs);
	}
	PipelineDrawQuad(&pl, bb, corners[0], corners[1], corners[2], corners[3], 0xFFFF8040, true, 0.0f);

	// Same quad, rotated the other way, stroked with pulsing thickness.
	float th = 4.0f + 6.0f * (0.5f + 0.5f * F32Sin(t * 2.0f));
	Vec2f c2 = V2fInit(w * 0.5f, h * 0.6f);
	for (int i = 0; i < 4; ++i) {
		corners[i] = V2fInit(c2.x + ox[i] * cs + oy[i] * sn, c2.y - ox[i] * sn + oy[i] * cs);
	}
	PipelineDrawQuad(&pl, bb, corners[0], corners[1], corners[2], corners[3], 0xFF40FF80, false, th);

	// Line fan with increasing thickness.
	Vec2f origin = V2fInit(w * 0.8f, h * 0.6f);
	for (int i = 0; i < 6; ++i) {
		float ang = t * 0.5f + i * (3.14159265f / 6.0f);
		Vec2f end = V2fInit(origin.x + F32Cos(ang) * 120.0f, origin.y + F32Sin(ang) * 120.0f);
		PipelineDrawLine(&pl, bb, origin, end, 0xFFFFFFFF, 1.0f + i * 2.0f);
	}

	// Filled circle and stroked circle.
	PipelineDrawCircle(&pl, bb, V2fInit(w * 0.25f, h * 0.25f), 60.0f, 0xFFFFD040, true, 0.0f);
	PipelineDrawCircle(&pl, bb, V2fInit(w * 0.5f, h * 0.25f), 60.0f, 0xFF40D0FF, false, 8.0f);

	// A circle partly off the right edge to show clipping.
	PipelineDrawCircle(&pl, bb, V2fInit(w + 20.0f, h * 0.25f), 80.0f, 0xFFC060FF, true, 0.0f);
}

//
// Scenario: Textured Quad (Phase 2.5-2.6)
//

static void UpdateTexturedQuad(TexturedQuadScenario *s, float dt) {
	s->time += dt;
}

static void RenderTexturedQuad(TexturedQuadScenario *s, fplVideoBackBuffer *bb, bool bilinear) {
	ClearBackbuffer(bb, kBackColor);

	PixelPipeline pl;
	PipelineInitOrtho(&pl, bb, 0.0f, (float)bb->width, 0.0f, (float)bb->height);
	PipelineResetModel(&pl);

	float w = (float)bb->width;
	float h = (float)bb->height;
	float t = s->time;

	// Quad moves and scales over time.
	float cx = w * 0.5f + F32Cos(t * 0.6f) * w * 0.2f;
	float cy = h * 0.5f + F32Sin(t * 0.5f) * h * 0.15f;
	float size = 140.0f + 40.0f * F32Sin(t);

	Vec2f p0 = V2fInit(cx - size, cy - size);
	Vec2f p1 = V2fInit(cx + size, cy - size);
	Vec2f p2 = V2fInit(cx + size, cy + size);
	Vec2f p3 = V2fInit(cx - size, cy + size);

	// Animated UVs: scroll + zoom to show texture-mapping control.
	float uvScroll = t * 0.1f;
	float uvZoom = 1.0f + 0.5f * F32Sin(t * 0.7f);
	Vec2f uv0 = V2fInit(uvScroll, uvScroll);
	Vec2f uv1 = V2fInit(uvScroll + uvZoom, uvScroll);
	Vec2f uv2 = V2fInit(uvScroll + uvZoom, uvScroll + uvZoom);
	Vec2f uv3 = V2fInit(uvScroll, uvScroll + uvZoom);

	PipelineDrawTexturedQuad(&pl, bb, p0, p1, p2, p3, uv0, uv1, uv2, uv3, &s->texture, bilinear);
}

//
// Scenario: Sub-Pixel Motion (Phase 2.7)
//

static void UpdateSubPixelMotion(SubPixelMotionScenario *s, float dt) {
	s->time += dt;
}

static void RenderSubPixelMotion(SubPixelMotionScenario *s, fplVideoBackBuffer *bb, bool bilinear) {
	ClearBackbuffer(bb, kBackColor);

	PixelPipeline pl;
	PipelineInitOrtho(&pl, bb, 0.0f, (float)bb->width, 0.0f, (float)bb->height);
	PipelineResetModel(&pl);

	float w = (float)bb->width;
	float h = (float)bb->height;
	// Very slow fractional drift to expose sub-pixel smoothness.
	float drift = F32Sin(s->time * 0.4f) * 30.0f;
	float size = 90.0f;

	Vec2f uv0 = V2fInit(0, 0);
	Vec2f uv1 = V2fInit(1, 0);
	Vec2f uv2 = V2fInit(1, 1);
	Vec2f uv3 = V2fInit(0, 1);

	// Left: nearest. Right: current filter (bilinear toggle).
	float ly = h * 0.5f + drift;
	Vec2f l0 = V2fInit(w * 0.3f - size, ly - size);
	Vec2f l1 = V2fInit(w * 0.3f + size, ly - size);
	Vec2f l2 = V2fInit(w * 0.3f + size, ly + size);
	Vec2f l3 = V2fInit(w * 0.3f - size, ly + size);
	PipelineDrawTexturedQuad(&pl, bb, l0, l1, l2, l3, uv0, uv1, uv2, uv3, &s->texture, false);

	Vec2f r0 = V2fInit(w * 0.7f - size, ly - size);
	Vec2f r1 = V2fInit(w * 0.7f + size, ly - size);
	Vec2f r2 = V2fInit(w * 0.7f + size, ly + size);
	Vec2f r3 = V2fInit(w * 0.7f - size, ly + size);
	PipelineDrawTexturedQuad(&pl, bb, r0, r1, r2, r3, uv0, uv1, uv2, uv3, &s->texture, bilinear);
}

//
// Dispatch
//

static void ScenarioUpdate(AppScenarios *app, fplVideoBackBuffer *bb, float dt) {
	switch (app->current) {
		case ScenarioType_Planes: UpdatePlanes(&app->data.planes, bb, dt); break;
		case ScenarioType_PixelPrimitives: UpdatePixelPrimitives(&app->data.pixelPrimitives, bb, dt); break;
		case ScenarioType_OrthoPipeline: UpdateOrthoPipeline(&app->data.orthoPipeline, dt); break;
		case ScenarioType_VectorShapes: UpdateVectorShapes(&app->data.vectorShapes, dt); break;
		case ScenarioType_TexturedQuad: UpdateTexturedQuad(&app->data.texturedQuad, dt); break;
		case ScenarioType_SubPixelMotion: UpdateSubPixelMotion(&app->data.subPixelMotion, dt); break;
		default: break;
	}
}

static void ScenarioRender(AppScenarios *app, fplVideoBackBuffer *bb) {
	switch (app->current) {
		case ScenarioType_Planes: RenderPlanes(&app->data.planes, bb); break;
		case ScenarioType_PixelPrimitives: RenderPixelPrimitives(&app->data.pixelPrimitives, bb); break;
		case ScenarioType_OrthoPipeline: RenderOrthoPipeline(&app->data.orthoPipeline, bb); break;
		case ScenarioType_VectorShapes: RenderVectorShapes(&app->data.vectorShapes, bb); break;
		case ScenarioType_TexturedQuad: RenderTexturedQuad(&app->data.texturedQuad, bb, false); break;
		case ScenarioType_SubPixelMotion: RenderSubPixelMotion(&app->data.subPixelMotion, bb, true); break;
		default: break;
	}
}

static void UpdateWindowTitle(const AppScenarios *app) {
	char title[256];
	fplStringFormat(title, fplArrayCount(title), "Software Rendering - [F1] %s (%d/%d)", ScenarioName(app->current), (int)app->current + 1, (int)ScenarioType_Count);
	fplSetWindowTitle(title);
}

int main(int argc, char **args) {
	fplColor32 backColor = fplCreateColorRGBA(39, 39, 58, 255);

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Software Rendering Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.backend = fplVideoBackendType_Software;
	settings.video.isAutoSize = true;
	settings.window.background = backColor;

	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		fplVideoBackBuffer *initialBackBuffer = fplGetVideoBackBuffer();

		AppScenarios app = fplZeroInit;
		ScenarioInit(&app, ScenarioType_Planes, initialBackBuffer);
		UpdateWindowTitle(&app);

		float dt = 1.0f / 60.0f;
		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {
				if (ev.type == fplEventType_Keyboard && ev.keyboard.type == fplKeyboardEventType_Button && ev.keyboard.buttonState == fplButtonState_Press) {
					if (ev.keyboard.mappedKey == fplKey_F1) {
						fplVideoBackBuffer *bb = fplGetVideoBackBuffer();
						ScenarioCleanup(&app);
						ScenarioType next = (ScenarioType)((app.current + 1) % ScenarioType_Count);
						ScenarioInit(&app, next, bb);
						UpdateWindowTitle(&app);
					}
				}
			}

			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();
			ScenarioUpdate(&app, backBuffer, dt);
			ScenarioRender(&app, backBuffer);
			fplVideoFlip();
		}

		ScenarioCleanup(&app);
		fplPlatformRelease();
	}
	return 0;
}