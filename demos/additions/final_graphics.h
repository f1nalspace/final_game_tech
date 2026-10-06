/*
Name:
	Final Graphics

Description:
	Software rendering primitives drawn directly into a fplVideoBackBuffer.

	Two layers:
	  1. Pixel layer  - integer top-down (y-down) primitives with clipping.
	  2. Pipeline layer - float world coordinates through an orthographic
	     projection + model matrix (final_math.h), with real clipping,
	     variable thickness, textures and bilinear sub-pixel sampling.

	This file is part of the final_framework.

	Optional define FINAL_GRAPHICS_USE_STB_IMAGE enables TextureLoadFromFile.
	The including translation unit must define STB_IMAGE_IMPLEMENTATION once.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_GRAPHICS_H
#define FINAL_GRAPHICS_H

#include <final_platform_layer.h>
#include <final_math.h>

#include <stdbool.h>

//
// Pixel layer (integer, top-down, y-down). All functions clip to the backbuffer.
//

// Filled axis-aligned quad, both corners inclusive.
extern void BackbufferFillQuad(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color);
// Horizontal line, 1 pixel thick.
extern void BackbufferDrawHLine(fplVideoBackBuffer *backBuffer, const float x0, const float x1, const float y, const uint32_t color);
// Vertical line, 1 pixel thick.
extern void BackbufferDrawVLine(fplVideoBackBuffer *backBuffer, const float x, const float y0, const float y1, const uint32_t color);
// Filled circle using the midpoint (Bresenham) algorithm.
extern void BackbufferFillCircle(fplVideoBackBuffer *backBuffer, const float cx, const float cy, const float radius, const uint32_t color);

// Existing primitives.
extern void BackbufferDrawLine(fplVideoBackBuffer *backBuffer, const float x0f, const float y0f, const float x1f, const float y1f, const uint32_t color);
extern void BackbufferDrawRect(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color);

//
// Pipeline layer (float world coordinates, orthographic projection + model matrix).
//

typedef struct PixelPipeline {
	Mat4f proj;          // Orthographic projection matrix.
	Mat4f model;         // Model matrix (scale then translation).
	Mat4f mvp;           // proj * model, recomputed on change.
	int viewportWidth;   // Backbuffer width in pixels.
	int viewportHeight;  // Backbuffer height in pixels.
} PixelPipeline;

// Builds an orthographic projection for the given world rectangle and stores the viewport.
extern void PipelineInitOrtho(PixelPipeline *pipeline, const fplVideoBackBuffer *backBuffer, const float left, const float right, const float bottom, const float top);
// Sets the model matrix to scale then translate, recomputing the mvp.
extern void PipelineSetModel(PixelPipeline *pipeline, const Vec2f position, const Vec2f scale);
// Resets the model matrix to identity.
extern void PipelineResetModel(PixelPipeline *pipeline);
// Projects a world point to screen pixel coordinates.
extern Vec2f PipelineProject(const PixelPipeline *pipeline, const Vec2f world);
// Row-vector transform (result = v * M). final_math.h matrices store translation
// in the bottom row, so V4fMultM4f cannot be used. Handy inside vertex shaders.
extern Vec4f GfxTransform(const Mat4f m, const Vec4f v);

// Arbitrary quad (4 world corners, CW or CCW). Filled or stroked with thickness (screen pixels).
extern void PipelineDrawQuad(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f p0, const Vec2f p1, const Vec2f p2, const Vec2f p3, const uint32_t color, const bool filled, const float thickness);
// Arbitrary line between two world points, thickness in screen pixels.
extern void PipelineDrawLine(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f a, const Vec2f b, const uint32_t color, const float thickness);
// Arbitrary circle at a world center with world radius. Filled or stroked with thickness (screen pixels).
extern void PipelineDrawCircle(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f center, const float radius, const uint32_t color, const bool filled, const float thickness);

//
// Texture
//

typedef struct Texture2D {
	uint32_t *pixels;  // RGBA packed as the backbuffer (0xAARRGGBB), top-down.
	int width;
	int height;
	bool ownsPixels;   // True when pixels were allocated by this module.
} Texture2D;

// Wraps existing pixel memory (not freed by TextureRelease).
extern void TextureInitFromMemory(Texture2D *texture, uint32_t *pixels, const int width, const int height);
// Frees pixel memory if owned and clears the struct.
extern void TextureRelease(Texture2D *texture);
#if defined(FINAL_GRAPHICS_USE_STB_IMAGE)
// Loads an image via stb_image (requires STB_IMAGE_IMPLEMENTATION in the TU).
extern bool TextureLoadFromFile(Texture2D *texture, const char *filePath);
#endif

// Texture-mapped quad with 4 UV coordinates (one per corner). bilinear toggles sub-pixel filtering.
extern void PipelineDrawTexturedQuad(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f p0, const Vec2f p1, const Vec2f p2, const Vec2f p3, const Vec2f uv0, const Vec2f uv1, const Vec2f uv2, const Vec2f uv3, const Texture2D *texture, const bool bilinear);

//
// Programmable triangle pipeline (software vertex/fragment shaders).
//
// A tiny OpenGL-style pipeline: vertex shader -> clip space -> perspective
// divide -> viewport -> edge-function rasterization with perspective-correct
// varyings -> depth test -> fragment shader. Shaders are plain C99 function
// pointers (no shading language); uniforms live in ShaderGlobals and per-vertex
// inputs/outputs are flat float arrays, mirroring GLSL attributes/varyings.
//

// Maximum interpolated varyings (besides clip position) a vertex shader may emit.
#define GFX_MAX_VARYINGS 12

// Uniform state shared by both shader stages (GLSL "uniform").
typedef struct ShaderGlobals {
	Mat4f mvp;         // Model-view-projection, row-vector: clip = vertex * mvp.
	Mat4f model;       // Model matrix alone (e.g. for world-space effects).
	Vec3f lightDir;    // Normalized light direction.
	float time;        // Seconds, for animation.
	const void *user;  // Optional extra uniform state.
} ShaderGlobals;

// Vertex shader output: clip-space position plus varyings interpolated per fragment.
typedef struct VertexOutput {
	Vec4f position;                    // Clip space (before perspective divide).
	float varyings[GFX_MAX_VARYINGS];  // Perspective-correct interpolated to fragments.
} VertexOutput;

// Vertex shader: maps one vertex's raw attributes to a VertexOutput.
typedef VertexOutput(*VertexShaderFn)(const ShaderGlobals *globals, const float *attributes);
// Fragment shader: maps interpolated varyings to a packed 0xAARRGGBB color.
typedef uint32_t(*FragmentShaderFn)(const ShaderGlobals *globals, const float *varyings);

// One draw command: an array-of-structs vertex buffer plus the two shader stages.
typedef struct DrawCall {
	const float *vertices;          // attributeCount floats per vertex, vertexCount vertices.
	int attributeCount;             // Floats per vertex.
	int varyingCount;               // Varyings the vertex shader fills (<= GFX_MAX_VARYINGS).
	int vertexCount;                // Total vertices, drawn as triangles (multiple of 3).
	VertexShaderFn vertexShader;
	FragmentShaderFn fragmentShader;
} DrawCall;

// Per-pixel float depth buffer paired with a backbuffer.
typedef struct DepthBuffer {
	float *values;  // width*height, smaller is nearer.
	int width;
	int height;
} DepthBuffer;

// Allocates or resizes the depth buffer to the given size.
extern void DepthBufferReset(DepthBuffer *depth, const int width, const int height);
// Frees the depth buffer and clears the struct.
extern void DepthBufferRelease(DepthBuffer *depth);
// Fills the whole depth buffer with a value (typically 1.0 = far plane).
extern void DepthBufferClear(DepthBuffer *depth, const float value);

// Runs the programmable pipeline for one draw call. Triangles whose vertices are
// at or behind the camera are skipped (no near-plane clipping). depth may be NULL
// to disable depth testing.
extern void GfxDrawTriangles(fplVideoBackBuffer *backBuffer, DepthBuffer *depth, const ShaderGlobals *globals, const DrawCall *draw);

#endif // FINAL_GRAPHICS_H

#if defined(FINAL_GRAPHICS_IMPLEMENTATION) && !defined(FINAL_GRAPHICS_IMPLEMENTED)
#define FINAL_GRAPHICS_IMPLEMENTED

static int ClampBackBufferPosition(int value, int min, int max) {
	int result = value;
	if (result < min) {
		result = min;
	}
	if (result > max) {
		result = max;
	}
	return(result);
}

static int GfxRoundToInt(const float value) {
	return (int)F32Floor(value + 0.5f);
}

// Integer horizontal span used by all higher-level fills (clipped, inclusive).
static void BackbufferHSpan(fplVideoBackBuffer *backBuffer, int x0, int x1, int y, const uint32_t color) {
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	if (y < 0 || y >= h) {
		return;
	}
	if (x0 > x1) {
		int tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (x1 < 0 || x0 >= w) {
		return;
	}
	if (x0 < 0) {
		x0 = 0;
	}
	if (x1 >= w) {
		x1 = w - 1;
	}
	uint32_t *pixel = backBuffer->pixels + (size_t)y * backBuffer->width + x0;
	for (int x = x0; x <= x1; ++x) {
		*pixel++ = color;
	}
}

// Integer vertical span (clipped, inclusive).
static void BackbufferVSpan(fplVideoBackBuffer *backBuffer, int x, int y0, int y1, const uint32_t color) {
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	if (x < 0 || x >= w) {
		return;
	}
	if (y0 > y1) {
		int tmp = y0;
		y0 = y1;
		y1 = tmp;
	}
	if (y1 < 0 || y0 >= h) {
		return;
	}
	if (y0 < 0) {
		y0 = 0;
	}
	if (y1 >= h) {
		y1 = h - 1;
	}
	uint32_t *pixel = backBuffer->pixels + (size_t)y0 * backBuffer->width + x;
	for (int y = y0; y <= y1; ++y) {
		*pixel = color;
		pixel += backBuffer->width;
	}
}

extern void BackbufferDrawHLine(fplVideoBackBuffer *backBuffer, const float x0, const float x1, const float y, const uint32_t color) {
	BackbufferHSpan(backBuffer, GfxRoundToInt(x0), GfxRoundToInt(x1), GfxRoundToInt(y), color);
}

extern void BackbufferDrawVLine(fplVideoBackBuffer *backBuffer, const float x, const float y0, const float y1, const uint32_t color) {
	BackbufferVSpan(backBuffer, GfxRoundToInt(x), GfxRoundToInt(y0), GfxRoundToInt(y1), color);
}

extern void BackbufferFillQuad(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color) {
	int minX = GfxRoundToInt(F32Min(x0, x1));
	int maxX = GfxRoundToInt(F32Max(x0, x1));
	int minY = GfxRoundToInt(F32Min(y0, y1));
	int maxY = GfxRoundToInt(F32Max(y0, y1));
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	if (maxX < 0 || minX >= w || maxY < 0 || minY >= h) {
		return;
	}
	minX = ClampBackBufferPosition(minX, 0, w - 1);
	maxX = ClampBackBufferPosition(maxX, 0, w - 1);
	minY = ClampBackBufferPosition(minY, 0, h - 1);
	maxY = ClampBackBufferPosition(maxY, 0, h - 1);
	for (int y = minY; y <= maxY; ++y) {
		uint32_t *pixel = backBuffer->pixels + (size_t)y * backBuffer->width + minX;
		for (int x = minX; x <= maxX; ++x) {
			*pixel++ = color;
		}
	}
}

extern void BackbufferFillCircle(fplVideoBackBuffer *backBuffer, const float cxf, const float cyf, const float radiusf, const uint32_t color) {
	int cx = GfxRoundToInt(cxf);
	int cy = GfxRoundToInt(cyf);
	int r = GfxRoundToInt(radiusf);
	if (r <= 0) {
		return;
	}
	int x = r;
	int y = 0;
	int err = 0;
	while (x >= y) {
		BackbufferHSpan(backBuffer, cx - x, cx + x, cy + y, color);
		BackbufferHSpan(backBuffer, cx - x, cx + x, cy - y, color);
		BackbufferHSpan(backBuffer, cx - y, cx + y, cy + x, color);
		BackbufferHSpan(backBuffer, cx - y, cx + y, cy - x, color);
		++y;
		if (err <= 0) {
			err += 2 * y + 1;
		}
		if (err > 0) {
			--x;
			err -= 2 * x + 1;
		}
	}
}

extern void BackbufferDrawLine(fplVideoBackBuffer *backBuffer, const float x0f, const float y0f, const float x1f, const float y1f, const uint32_t color) {
	int x0 = GfxRoundToInt(x0f);
	int y0 = GfxRoundToInt(y0f);
	int x1 = GfxRoundToInt(x1f);
	int y1 = GfxRoundToInt(y1f);

	int imageLine = backBuffer->width;
	int size = backBuffer->width * backBuffer->height;

	int dx = x1 - x0;
	int dy = y1 - y0;

	int dLong = abs(dx);
	int dShort = abs(dy);

	int offsetLong = dx > 0 ? 1 : -1;
	int offsetShort = dy > 0 ? imageLine : -imageLine;

	if (dLong < dShort) {
		int tmp[2] = { dShort, offsetShort };
		dShort = dLong;
		offsetShort = offsetLong;
		dLong = tmp[0];
		offsetLong = tmp[1];
	}

	int error = dLong / 2;
	int index = y0 * imageLine + x0;

	const int offset[] = { offsetLong, offsetLong + offsetShort };
	const int abs_d[] = { dShort, dShort - dLong };
	for (int i = 0; i <= dLong; ++i) {
		if (index >= 0 && index < size) {
			backBuffer->pixels[index] = color;
		}
		const int errorIsTooBig = error >= dLong;
		index += offset[errorIsTooBig];
		error += abs_d[errorIsTooBig];
	}
}

extern void BackbufferDrawRect(fplVideoBackBuffer *backBuffer, const float x0, const float y0, const float x1, const float y1, const uint32_t color) {
	int minX = GfxRoundToInt(x0);
	int minY = GfxRoundToInt(y0);
	int maxX = GfxRoundToInt(x1);
	int maxY = GfxRoundToInt(y1);
	if (minX > maxX) {
		int tmp = minX;
		minX = maxX;
		maxX = tmp;
	}
	if (minY > maxY) {
		int tmp = minY;
		minY = maxY;
		maxY = tmp;
	}
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	minX = ClampBackBufferPosition(minX, 0, w - 1);
	maxX = ClampBackBufferPosition(maxX, 0, w - 1);
	minY = ClampBackBufferPosition(minY, 0, h - 1);
	maxY = ClampBackBufferPosition(maxY, 0, h - 1);
	for (int yp = minY; yp < maxY; ++yp) {
		uint32_t *pixel = backBuffer->pixels + yp * backBuffer->width + minX;
		for (int xp = minX; xp < maxX; ++xp) {
			*pixel++ = color;
		}
	}
}

//
// Pipeline implementation
//

// Row-vector transform (result = v * M). The final_math.h ortho/translation
// matrices store translation in the bottom row, so V4fMultM4f cannot be used.
extern Vec4f GfxTransform(const Mat4f m, const Vec4f v) {
	Vec4f r;
	r.x = v.x * m.r[0][0] + v.y * m.r[1][0] + v.z * m.r[2][0] + v.w * m.r[3][0];
	r.y = v.x * m.r[0][1] + v.y * m.r[1][1] + v.z * m.r[2][1] + v.w * m.r[3][1];
	r.z = v.x * m.r[0][2] + v.y * m.r[1][2] + v.z * m.r[2][2] + v.w * m.r[3][2];
	r.w = v.x * m.r[0][3] + v.y * m.r[1][3] + v.z * m.r[2][3] + v.w * m.r[3][3];
	return(r);
}

extern void PipelineInitOrtho(PixelPipeline *pipeline, const fplVideoBackBuffer *backBuffer, const float left, const float right, const float bottom, const float top) {
	pipeline->proj = M4fOrthoRH(left, right, bottom, top, -1.0f, 1.0f);
	pipeline->model = M4fIdentity();
	pipeline->mvp = M4fMult(pipeline->proj, pipeline->model);
	pipeline->viewportWidth = (int)backBuffer->width;
	pipeline->viewportHeight = (int)backBuffer->height;
}

extern void PipelineSetModel(PixelPipeline *pipeline, const Vec2f position, const Vec2f scale) {
	Mat4f t = M4fTranslationV2(position);
	Mat4f s = M4fScaleV2(scale);
	pipeline->model = M4fMult(t, s);
	pipeline->mvp = M4fMult(pipeline->proj, pipeline->model);
}

extern void PipelineResetModel(PixelPipeline *pipeline) {
	pipeline->model = M4fIdentity();
	pipeline->mvp = M4fMult(pipeline->proj, pipeline->model);
}

extern Vec2f PipelineProject(const PixelPipeline *pipeline, const Vec2f world) {
	Vec4f clip = GfxTransform(pipeline->mvp, V4fInit(world.x, world.y, 0.0f, 1.0f));
	float invW = (clip.w != 0.0f) ? (1.0f / clip.w) : 1.0f;
	float ndcX = clip.x * invW;
	float ndcY = clip.y * invW;
	Vec2f result;
	// OpenGL convention: NDC y points up. The backbuffer is top-down, so flip y
	// to keep world-up visually up (matches glOrtho on a GL viewport).
	result.x = (ndcX * 0.5f + 0.5f) * (float)pipeline->viewportWidth;
	result.y = (1.0f - (ndcY * 0.5f + 0.5f)) * (float)pipeline->viewportHeight;
	return(result);
}

// Fills a convex polygon in screen space using a scanline span fill (clipped).
static void GfxFillConvexScreen(fplVideoBackBuffer *backBuffer, const Vec2f *pts, const int count, const uint32_t color) {
	if (count < 3) {
		return;
	}
	float minYf = pts[0].y;
	float maxYf = pts[0].y;
	for (int i = 1; i < count; ++i) {
		minYf = F32Min(minYf, pts[i].y);
		maxYf = F32Max(maxYf, pts[i].y);
	}
	int h = (int)backBuffer->height;
	int minY = ClampBackBufferPosition(GfxRoundToInt(minYf), 0, h - 1);
	int maxY = ClampBackBufferPosition(GfxRoundToInt(maxYf), 0, h - 1);
	for (int y = minY; y <= maxY; ++y) {
		float yc = (float)y + 0.5f;
		float xs[16];
		int n = 0;
		for (int i = 0; i < count && n < 16; ++i) {
			Vec2f a = pts[i];
			Vec2f b = pts[(i + 1) % count];
			if ((a.y <= yc && b.y > yc) || (b.y <= yc && a.y > yc)) {
				float t = (yc - a.y) / (b.y - a.y);
				xs[n++] = a.x + t * (b.x - a.x);
			}
		}
		// Sort intersections ascending.
		for (int i = 0; i < n - 1; ++i) {
			for (int j = i + 1; j < n; ++j) {
				if (xs[j] < xs[i]) {
					float tmp = xs[i];
					xs[i] = xs[j];
					xs[j] = tmp;
				}
			}
		}
		for (int i = 0; i + 1 < n; i += 2) {
			BackbufferHSpan(backBuffer, GfxRoundToInt(xs[i]), GfxRoundToInt(xs[i + 1]) - 1, y, color);
		}
	}
}

// Builds a thick segment quad in screen space and fills it.
static void GfxStrokeSegmentScreen(fplVideoBackBuffer *backBuffer, const Vec2f a, const Vec2f b, const float thickness, const uint32_t color) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float len = F32SquareRoot(dx * dx + dy * dy);
	float half = F32Max(thickness, 1.0f) * 0.5f;
	if (len < 1e-5f) {
		BackbufferFillQuad(backBuffer, a.x - half, a.y - half, a.x + half, a.y + half, color);
		return;
	}
	float nx = -dy / len * half;
	float ny = dx / len * half;
	Vec2f quad[4];
	quad[0] = V2fInit(a.x + nx, a.y + ny);
	quad[1] = V2fInit(b.x + nx, b.y + ny);
	quad[2] = V2fInit(b.x - nx, b.y - ny);
	quad[3] = V2fInit(a.x - nx, a.y - ny);
	GfxFillConvexScreen(backBuffer, quad, 4, color);
}

extern void PipelineDrawLine(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f a, const Vec2f b, const uint32_t color, const float thickness) {
	Vec2f sa = PipelineProject(pipeline, a);
	Vec2f sb = PipelineProject(pipeline, b);
	GfxStrokeSegmentScreen(backBuffer, sa, sb, thickness, color);
}

extern void PipelineDrawQuad(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f p0, const Vec2f p1, const Vec2f p2, const Vec2f p3, const uint32_t color, const bool filled, const float thickness) {
	Vec2f s[4];
	s[0] = PipelineProject(pipeline, p0);
	s[1] = PipelineProject(pipeline, p1);
	s[2] = PipelineProject(pipeline, p2);
	s[3] = PipelineProject(pipeline, p3);
	if (filled) {
		GfxFillConvexScreen(backBuffer, s, 4, color);
	} else {
		GfxStrokeSegmentScreen(backBuffer, s[0], s[1], thickness, color);
		GfxStrokeSegmentScreen(backBuffer, s[1], s[2], thickness, color);
		GfxStrokeSegmentScreen(backBuffer, s[2], s[3], thickness, color);
		GfxStrokeSegmentScreen(backBuffer, s[3], s[0], thickness, color);
	}
}

extern void PipelineDrawCircle(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f center, const float radius, const uint32_t color, const bool filled, const float thickness) {
	Vec2f sc = PipelineProject(pipeline, center);
	Vec2f se = PipelineProject(pipeline, V2fInit(center.x + radius, center.y));
	float rx = se.x - sc.x;
	float ry = se.y - sc.y;
	float r = F32SquareRoot(rx * rx + ry * ry);
	if (r < 0.5f) {
		return;
	}
	int h = (int)backBuffer->height;
	if (filled) {
		int minY = ClampBackBufferPosition(GfxRoundToInt(sc.y - r), 0, h - 1);
		int maxY = ClampBackBufferPosition(GfxRoundToInt(sc.y + r), 0, h - 1);
		for (int y = minY; y <= maxY; ++y) {
			float dy = ((float)y + 0.5f) - sc.y;
			float inside = r * r - dy * dy;
			if (inside <= 0.0f) {
				continue;
			}
			float dx = F32SquareRoot(inside);
			BackbufferHSpan(backBuffer, GfxRoundToInt(sc.x - dx), GfxRoundToInt(sc.x + dx) - 1, y, color);
		}
	} else {
		float half = F32Max(thickness, 1.0f) * 0.5f;
		float rOut = r + half;
		float rIn = r - half;
		if (rIn < 0.0f) {
			rIn = 0.0f;
		}
		int minY = ClampBackBufferPosition(GfxRoundToInt(sc.y - rOut), 0, h - 1);
		int maxY = ClampBackBufferPosition(GfxRoundToInt(sc.y + rOut), 0, h - 1);
		for (int y = minY; y <= maxY; ++y) {
			float dy = ((float)y + 0.5f) - sc.y;
			float outside = rOut * rOut - dy * dy;
			if (outside <= 0.0f) {
				continue;
			}
			float xo = F32SquareRoot(outside);
			float insideInner = rIn * rIn - dy * dy;
			if (insideInner > 0.0f) {
				float xi = F32SquareRoot(insideInner);
				BackbufferHSpan(backBuffer, GfxRoundToInt(sc.x - xo), GfxRoundToInt(sc.x - xi) - 1, y, color);
				BackbufferHSpan(backBuffer, GfxRoundToInt(sc.x + xi), GfxRoundToInt(sc.x + xo) - 1, y, color);
			} else {
				BackbufferHSpan(backBuffer, GfxRoundToInt(sc.x - xo), GfxRoundToInt(sc.x + xo) - 1, y, color);
			}
		}
	}
}

//
// Texture implementation
//

extern void TextureInitFromMemory(Texture2D *texture, uint32_t *pixels, const int width, const int height) {
	texture->pixels = pixels;
	texture->width = width;
	texture->height = height;
	texture->ownsPixels = false;
}

extern void TextureRelease(Texture2D *texture) {
	if (texture->ownsPixels && texture->pixels != fpl_null) {
		fplMemoryFree(texture->pixels);
	}
	texture->pixels = fpl_null;
	texture->width = 0;
	texture->height = 0;
	texture->ownsPixels = false;
}

#if defined(FINAL_GRAPHICS_USE_STB_IMAGE)
#include <stb/stb_image.h>
extern bool TextureLoadFromFile(Texture2D *texture, const char *filePath) {
	int w = 0;
	int h = 0;
	int comp = 0;
	// OpenGL texture convention: v=0 at the bottom. Flip so v points up.
	stbi_set_flip_vertically_on_load(1);
	unsigned char *data = stbi_load(filePath, &w, &h, &comp, 4);
	if (data == fpl_null) {
		return(false);
	}
	uint32_t *pixels = (uint32_t *)fplMemoryAllocate((size_t)w * (size_t)h * sizeof(uint32_t));
	for (int i = 0; i < w * h; ++i) {
		uint8_t r = data[i * 4 + 0];
		uint8_t g = data[i * 4 + 1];
		uint8_t b = data[i * 4 + 2];
		uint8_t a = data[i * 4 + 3];
		pixels[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
	}
	stbi_image_free(data);
	texture->pixels = pixels;
	texture->width = w;
	texture->height = h;
	texture->ownsPixels = true;
	return(true);
}
#endif

static uint32_t GfxSampleNearest(const Texture2D *texture, float u, float v) {
	int x = (int)F32Floor(u * (float)texture->width);
	int y = (int)F32Floor(v * (float)texture->height);
	x = ClampBackBufferPosition(x, 0, texture->width - 1);
	y = ClampBackBufferPosition(y, 0, texture->height - 1);
	return(texture->pixels[(size_t)y * texture->width + x]);
}

static uint32_t GfxSampleBilinear(const Texture2D *texture, float u, float v) {
	float fx = u * (float)texture->width - 0.5f;
	float fy = v * (float)texture->height - 0.5f;
	int x0 = (int)F32Floor(fx);
	int y0 = (int)F32Floor(fy);
	float tx = fx - (float)x0;
	float ty = fy - (float)y0;
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	x0 = ClampBackBufferPosition(x0, 0, texture->width - 1);
	x1 = ClampBackBufferPosition(x1, 0, texture->width - 1);
	y0 = ClampBackBufferPosition(y0, 0, texture->height - 1);
	y1 = ClampBackBufferPosition(y1, 0, texture->height - 1);
	uint32_t c00 = texture->pixels[(size_t)y0 * texture->width + x0];
	uint32_t c10 = texture->pixels[(size_t)y0 * texture->width + x1];
	uint32_t c01 = texture->pixels[(size_t)y1 * texture->width + x0];
	uint32_t c11 = texture->pixels[(size_t)y1 * texture->width + x1];
	uint32_t result = 0;
	for (int s = 0; s < 32; s += 8) {
		float a = (float)((c00 >> s) & 0xFF);
		float b = (float)((c10 >> s) & 0xFF);
		float c = (float)((c01 >> s) & 0xFF);
		float d = (float)((c11 >> s) & 0xFF);
		float top = a + (b - a) * tx;
		float bottom = c + (d - c) * tx;
		float value = top + (bottom - top) * ty;
		result |= ((uint32_t)(value + 0.5f) & 0xFF) << s;
	}
	return(result);
}

// Rasterizes a textured triangle with affine UV interpolation (sub-pixel, clipped).
static void GfxRasterTexTri(fplVideoBackBuffer *backBuffer, const Vec2f a, const Vec2f b, const Vec2f c, const Vec2f uva, const Vec2f uvb, const Vec2f uvc, const Texture2D *texture, const bool bilinear) {
	float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	if (F32Abs(area) < 1e-6f) {
		return;
	}
	float invArea = 1.0f / area;
	int w = (int)backBuffer->width;
	int h = (int)backBuffer->height;
	int minX = ClampBackBufferPosition(GfxRoundToInt(F32Min(a.x, F32Min(b.x, c.x))), 0, w - 1);
	int maxX = ClampBackBufferPosition(GfxRoundToInt(F32Max(a.x, F32Max(b.x, c.x))), 0, w - 1);
	int minY = ClampBackBufferPosition(GfxRoundToInt(F32Min(a.y, F32Min(b.y, c.y))), 0, h - 1);
	int maxY = ClampBackBufferPosition(GfxRoundToInt(F32Max(a.y, F32Max(b.y, c.y))), 0, h - 1);
	for (int y = minY; y <= maxY; ++y) {
		float py = (float)y + 0.5f;
		uint32_t *row = backBuffer->pixels + (size_t)y * backBuffer->width;
		for (int x = minX; x <= maxX; ++x) {
			float px = (float)x + 0.5f;
			float w0 = ((b.x - px) * (c.y - py) - (b.y - py) * (c.x - px)) * invArea;
			float w1 = ((c.x - px) * (a.y - py) - (c.y - py) * (a.x - px)) * invArea;
			float w2 = 1.0f - w0 - w1;
			if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) {
				continue;
			}
			float u = w0 * uva.x + w1 * uvb.x + w2 * uvc.x;
			float v = w0 * uva.y + w1 * uvb.y + w2 * uvc.y;
			row[x] = bilinear ? GfxSampleBilinear(texture, u, v) : GfxSampleNearest(texture, u, v);
		}
	}
}

extern void PipelineDrawTexturedQuad(const PixelPipeline *pipeline, fplVideoBackBuffer *backBuffer, const Vec2f p0, const Vec2f p1, const Vec2f p2, const Vec2f p3, const Vec2f uv0, const Vec2f uv1, const Vec2f uv2, const Vec2f uv3, const Texture2D *texture, const bool bilinear) {
	Vec2f s0 = PipelineProject(pipeline, p0);
	Vec2f s1 = PipelineProject(pipeline, p1);
	Vec2f s2 = PipelineProject(pipeline, p2);
	Vec2f s3 = PipelineProject(pipeline, p3);
	GfxRasterTexTri(backBuffer, s0, s1, s2, uv0, uv1, uv2, texture, bilinear);
	GfxRasterTexTri(backBuffer, s0, s2, s3, uv0, uv2, uv3, texture, bilinear);
}

//
// Programmable triangle pipeline implementation
//

extern void DepthBufferReset(DepthBuffer *depth, const int width, const int height) {
	if (depth->values == NULL || depth->width != width || depth->height != height) {
		if (depth->values != NULL) {
			fplMemoryFree(depth->values);
		}
		depth->values = (float *)fplMemoryAllocate((size_t)width * height * sizeof(float));
		depth->width = width;
		depth->height = height;
	}
}

extern void DepthBufferRelease(DepthBuffer *depth) {
	if (depth->values != NULL) {
		fplMemoryFree(depth->values);
	}
	depth->values = NULL;
	depth->width = 0;
	depth->height = 0;
}

extern void DepthBufferClear(DepthBuffer *depth, const float value) {
	int count = depth->width * depth->height;
	for (int i = 0; i < count; ++i) {
		depth->values[i] = value;
	}
}

// Signed area of triangle (a, b, c); sign encodes winding.
static float GfxEdge(const float ax, const float ay, const float bx, const float by, const float cx, const float cy) {
	return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

extern void GfxDrawTriangles(fplVideoBackBuffer *backBuffer, DepthBuffer *depth, const ShaderGlobals *globals, const DrawCall *draw) {
	int width = (int)backBuffer->width;
	int height = (int)backBuffer->height;
	int varyingCount = draw->varyingCount;
	int triCount = draw->vertexCount / 3;

	for (int t = 0; t < triCount; ++t) {
		// Vertex stage: run the vertex shader on the three vertices.
		VertexOutput vo[3];
		for (int i = 0; i < 3; ++i) {
			const float *attribs = draw->vertices + (size_t)(t * 3 + i) * draw->attributeCount;
			vo[i] = draw->vertexShader(globals, attribs);
		}

		// Reject triangles touching or behind the camera (no near-plane clipping).
		if (vo[0].position.w <= 1e-5f || vo[1].position.w <= 1e-5f || vo[2].position.w <= 1e-5f) {
			continue;
		}

		// Perspective divide and viewport transform (Y flipped for the top-down buffer).
		float sx[3], sy[3], sz[3], invW[3];
		for (int i = 0; i < 3; ++i) {
			float iw = 1.0f / vo[i].position.w;
			invW[i] = iw;
			sx[i] = ((vo[i].position.x * iw) * 0.5f + 0.5f) * (float)width;
			sy[i] = (1.0f - ((vo[i].position.y * iw) * 0.5f + 0.5f)) * (float)height;
			sz[i] = (vo[i].position.z * iw) * 0.5f + 0.5f;
		}

		float area = GfxEdge(sx[0], sy[0], sx[1], sy[1], sx[2], sy[2]);
		if (area == 0.0f) {
			continue;
		}
		float invArea = 1.0f / area;

		// Bounding box clipped to the viewport.
		float fminX = sx[0], fmaxX = sx[0], fminY = sy[0], fmaxY = sy[0];
		for (int i = 1; i < 3; ++i) {
			if (sx[i] < fminX) { fminX = sx[i]; }
			if (sx[i] > fmaxX) { fmaxX = sx[i]; }
			if (sy[i] < fminY) { fminY = sy[i]; }
			if (sy[i] > fmaxY) { fmaxY = sy[i]; }
		}
		int minX = ClampBackBufferPosition((int)F32Floor(fminX), 0, width - 1);
		int maxX = ClampBackBufferPosition((int)F32Ceil(fmaxX), 0, width - 1);
		int minY = ClampBackBufferPosition((int)F32Floor(fminY), 0, height - 1);
		int maxY = ClampBackBufferPosition((int)F32Ceil(fmaxY), 0, height - 1);

		for (int y = minY; y <= maxY; ++y) {
			uint32_t *row = backBuffer->pixels + (size_t)y * backBuffer->width;
			float *drow = (depth != NULL) ? (depth->values + (size_t)y * depth->width) : NULL;
			float py = (float)y + 0.5f;
			for (int x = minX; x <= maxX; ++x) {
				float px = (float)x + 0.5f;

				// Barycentric weights via edge functions; invArea handles winding sign.
				float b0 = GfxEdge(sx[1], sy[1], sx[2], sy[2], px, py) * invArea;
				float b1 = GfxEdge(sx[2], sy[2], sx[0], sy[0], px, py) * invArea;
				float b2 = GfxEdge(sx[0], sy[0], sx[1], sy[1], px, py) * invArea;
				if (b0 < 0.0f || b1 < 0.0f || b2 < 0.0f) {
					continue;
				}

				// Depth is linear in screen space after the perspective divide.
				float z = b0 * sz[0] + b1 * sz[1] + b2 * sz[2];
				if (drow != NULL && z >= drow[x]) {
					continue;
				}

				// Perspective-correct varyings: interpolate attribute/w, then divide by 1/w.
				float w0 = b0 * invW[0];
				float w1 = b1 * invW[1];
				float w2 = b2 * invW[2];
				float wsum = w0 + w1 + w2;
				float invWsum = (wsum != 0.0f) ? (1.0f / wsum) : 0.0f;
				float varyings[GFX_MAX_VARYINGS];
				for (int v = 0; v < varyingCount; ++v) {
					varyings[v] = (w0 * vo[0].varyings[v] + w1 * vo[1].varyings[v] + w2 * vo[2].varyings[v]) * invWsum;
				}

				row[x] = draw->fragmentShader(globals, varyings);
				if (drow != NULL) {
					drow[x] = z;
				}
			}
		}
	}
}

#endif // FINAL_GRAPHICS_IMPLEMENTATION
