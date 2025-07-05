/*
Name:
	Final Gamebox

	Frontend-Part (Renderer Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#pragma once

#include <final_platform_layer.h>

#include <final_memory.h>

#include "final_fontloader.h"

#include "final_math.h"

typedef struct Color4f {
	float r, g, b, a;
} Color4f;

typedef enum TextureFormat {
	TextureFormat_Automatic = -1,
	TextureFormat_None = 0,
	TextureFormat_Alpha = 8,
	TextureFormat_RGB = 24,
	TextureFormat_RGBA = 32,
} TextureFormat;

typedef enum TextureFilter {
	TextureFilter_Linear = 0,
	TextureFilter_Nearest,
} TextureFilter;

typedef struct {
	uint32_t internalFormat;
	uint32_t format;
} APITextureFormat;

typedef uint32_t TextureID;

typedef enum {
	TextureState_None = 0,
	TextureState_Update,
	TextureState_Clear,
} TextureState;

typedef struct {
	uint32_t *pixels;
	APITextureFormat format;
	TextureID id;
	uint32_t width;
	uint32_t height;
	float uScale;
	float vScale;
	uint32_t isValid;
	uint32_t hasPixels;
	TextureState state;
} Texture;

extern const APITextureFormat InvalidAPITextureFormat;

extern const Texture InvalidTexture;

extern const Color4f ColorWhite;
extern const Color4f ColorBlack;
extern const Color4f ColorRed;
extern const Color4f ColorGreen;
extern const Color4f ColorBlue;
extern const Color4f ColorYellow;
extern const Color4f ColorGray;
extern const Color4f ColorDarkGray;

typedef void RendererContext;

extern RendererContext *RendererCreate(fmemMemoryBlock *memory);
extern void RendererDestroy(RendererContext *context);

extern Texture UploadTexture(const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter, const void *data);
extern Texture AllocateTexture(fmemMemoryBlock *mem, const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter);
extern Texture LoadTextureFromMemory(const uint8_t *data, const size_t size, const TextureFormat format, const TextureFilter textureFilter, const uint32_t actualWidth, const uint32_t actualHeight);
extern void ReleaseTexture(Texture *texture);
extern void UpdateTexture(const Texture *texture);
extern void ClearTexture(const Texture *texture);

extern void SetViewport(const int x, const int y, const int w, const int h);

extern void EnableClipping();
extern void SetViewClipRect(const Mat4f *projMat, const Mat4f *viewMat, const Viewport4i *viewport, const float x, const float y, const float w, const float h);
extern void DisableClipping();

extern void Clear(const float r, const float g, const float b, const float a);

extern void SetModelViewProjectionMatrix(const float *mvp);

extern void DrawString(const LoadedFont *font, const TextureID textureId, const char *text, const size_t textLen, const float x, const float y, const float scale, const Color4f color);

extern void DrawTexturedQuad(
	const TextureID textureId,
	const float x,
	const float y,
	const float w,
	const float h,
	const Color4f color,
	const float u0,
	const float v0,
	const float u1,
	const float v1);

extern void DrawFilledQuad(const float x, const float y, const float w, const float h, const Color4f color);

extern void DrawStrokedQuad(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f color);

extern void DrawLine(const float x0, const float y0, const float x1, const float y1, const float lineWidth, const Color4f color);

typedef enum {
	ControlBorderFlags_None = 0,
	ControlBorderFlags_Left = 1 << 0,
	ControlBorderFlags_Right = 1 << 1,
	ControlBorderFlags_Top = 1 << 2,
	ControlBorderFlags_Bottom = 1 << 3,
	ControlBorderFlags_All = ControlBorderFlags_Left | ControlBorderFlags_Right | ControlBorderFlags_Top | ControlBorderFlags_Bottom,
} ControlBorderFlags;

extern void DrawControlBorders(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f *color0, const Color4f *color1, const Color4f *color2, const ControlBorderFlags flags, const bool isDown);

extern void DrawControlBorder(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f *color0, const Color4f *color1, const Color4f *color2, const bool isDown);