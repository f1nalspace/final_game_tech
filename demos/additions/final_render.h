/*
Name:
	Final Render

Description:
	Contains structures for computing render specific stuff and
	has a render command system.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_RENDER_H
#define FINAL_RENDER_H

#include <final_memory.h>

#include "final_math.h"
#include <final_fontloader.h>

typedef struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
} UVRect;

inline UVRect UVRectInit(const float uMin, const float vMin, const float uMax, const float vMax) {
	UVRect result = fplZeroInit;
	result.uMin = uMin;
	result.vMin = vMin;
	result.uMax = uMax;
	result.vMax = vMax;
	return(result);
}

inline UVRect UVRectDefault() {
	return UVRectInit(0.0f, 0.0f, 1.0f, 1.0f);
}

inline UVRect UVRectFromTile(const Vec2i imageSize, const Vec2i tileSize, const int border, const Vec2i pos) {
	Vec2f texel = V2fInit(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	int imgX = border + pos.x * tileSize.x + border * pos.x;
	int imgY = border + pos.y * tileSize.y + border * pos.y;
	UVRect result = fplZeroInit;
	result.uMin = imgX * texel.x;
	result.vMin = imgY * texel.y;
	result.uMax = result.uMin + tileSize.x * texel.x;
	result.vMax = result.vMin + tileSize.y * texel.y;
	return(result);
}

inline UVRect UVRectFromPos(const Vec2i imageSize, const Vec2i partSize, const Vec2i pos) {
	Vec2f texel = V2fInit(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result = fplZeroInit;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
	return(result);
}

typedef struct Viewport {
	int x;
	int y;
	int w;
	int h;
} Viewport;

extern Viewport ViewportComputeByAspect(const Vec2i screenSize, const float targetAspect);

typedef enum TextureOperationType {
	TextureOperationType_None = 0,
	TextureOperationType_Upload,
	TextureOperationType_Release
} TextureOperationType;

typedef void *TextureHandle;

typedef enum TextureFilterType {
	TextureFilterType_Nearest = 0,
	TextureFilterType_Linear
} TextureFilterType;

typedef enum TextureWrapMode {
	TextureWrapMode_Repeat = 0,
	TextureWrapMode_ClampToEdge,
	TextureWrapMode_ClampToBorder,
} TextureWrapMode;

typedef struct TextureOperation {
	TextureHandle *handle;
	const void *data;
	TextureOperationType type;
	TextureFilterType filter;
	TextureWrapMode wrap;
	uint32_t width;
	uint32_t height;
	uint32_t bytesPerPixel;
	bool isTopDown;
	bool isPreMultiplied;
} TextureOperation;

#define MAX_TEXTURE_OPERATION_COUNT 1024
#define MAX_MATRIX_STACK_COUNT 32

typedef struct RenderState {
	TextureOperation textureOperations[MAX_TEXTURE_OPERATION_COUNT];
	Mat4f matrixStack[MAX_MATRIX_STACK_COUNT];
	size_t matrixTop;
	fmemMemoryBlock memory;
	size_t textureOperationCount;
	size_t lastMemoryUsage;
} RenderState;

typedef enum CommandType {
	CommandType_None = 0,
	CommandType_Clear,
	CommandType_Viewport,
	CommandType_Matrix,
	CommandType_Rectangle,
	CommandType_Vertices,
	CommandType_Sprite,
	CommandType_Text
} CommandType;

typedef struct CommandHeader {
	size_t dataSize;
	CommandType type;
} CommandHeader;

typedef enum MatrixMode {
	MatrixMode_Set = 0,
	MatrixMode_Push,
	MatrixMode_Pop
} MatrixMode;

typedef struct MatrixCommand {
	Mat4f mat;
	MatrixMode mode;
} MatrixCommand;

typedef enum ClearFlags {
	ClearFlags_None = 0,
	ClearFlags_Color = 1 << 0,
	ClearFlags_Depth = 1 << 1,
} ClearFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

typedef struct ClearCommand {
	Vec4f color;
	ClearFlags flags;
} ClearCommand;

typedef struct ViewportCommand {
	int x;
	int y;
	int w;
	int h;
} ViewportCommand;

typedef struct RectangleCommand {
	Vec4f color;
	Vec2f bottomLeft;
	Vec2f size;
	float lineWidth;
	bool isFilled;
} RectangleCommand;

typedef enum DrawMode {
	DrawMode_None,
	DrawMode_Points,
	DrawMode_Lines,
	DrawMode_Triangles,
	DrawMode_Polygon
} DrawMode;

typedef struct VertexAllocation {
	Vec2f *verts;
	size_t *count;
} VertexAllocation;

typedef struct VerticesCommand {
	Vec4f color;
	const Vec2f *verts;
	size_t capacity;
	size_t count;
	DrawMode drawMode;
	float thickness;
	bool isLoop;
} VerticesCommand;

typedef struct SpriteCommand {
	Vec4f color;
	Vec2f position;
	Vec2f ext;
	Vec2f uvMin;
	Vec2f uvMax;
	TextureHandle texture;
} SpriteCommand;

typedef struct TextCommand {
	Vec4f color;
	Vec2f position;
	TextureHandle texture;
	const LoadedFont *font;
	float horizontalAlignment;
	float verticalAlignment;
	float maxHeight;
	size_t textLength;
} TextCommand;

extern void RenderInit(RenderState *state, fmemMemoryBlock block);
extern void RenderReset(RenderState *state);
extern void PushClear(RenderState *state, const Vec4f color, const ClearFlags flags);
extern void PushViewport(RenderState *state, const int x, const int y, const int w, const int h);
extern void PushMatrix(RenderState *state, const Mat4f *mat, const MatrixMode mode);
extern void SetMatrix(RenderState *state, const Mat4f *mat);
extern void PopMatrix(RenderState *state);
extern void PushRectangle(RenderState *state, const Vec2f bottomLeft, const Vec2f size, const Vec4f color, const bool isFilled, const float lineWidth);
extern void PushRectangleCenter(RenderState *state, const Vec2f center, const Vec2f ext, const Vec4f color, const bool isFilled, const float lineWidth);
extern void PushQuad(RenderState *state,const Vec2f center,const float radius,const Vec4f color,const bool isFilled,const float lineWidth);
extern VertexAllocation AllocateVertices(RenderState *state, const size_t capacity, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness);
extern void PushVertices(RenderState *state, const Vec2f *verts, const size_t vertexCount, const bool copyVerts, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness);
extern void PushSprite(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect);
extern void PushTexture(RenderState *state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied);
extern void PopTexture(RenderState *state, TextureHandle *targetTexture);
extern void PushText(RenderState *state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle texture, const Vec2f position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f color);
extern void PushCircle(RenderState *state, const Vec2f position, const float radius, const size_t segmentCount, const Vec4f color, const bool isFilled, const float lineWidth);
extern void PushLine(RenderState *state, const Vec2f a, const Vec2f b, const Vec4f color, const float lineWidth);

#endif // FINAL_RENDER_H

#if defined(FINAL_RENDER_IMPLEMENTATION) && !defined(FINAL_RENDER_IMPLEMENTED)
#define FINAL_RENDER_IMPLEMENTED

static CommandHeader *PushHeader(RenderState *state, const CommandType type) {
	if (state == fpl_null) {
		return fpl_null;
	}
	CommandHeader *result = (CommandHeader *)fmemPush(&state->memory, sizeof(CommandHeader), fmemPushFlags_None);
	if (result == fpl_null) {
		return fpl_null;
	}
	result->type = type;
	result->dataSize = 0;
	return result;
}

static void *PushTypes(RenderState *state, CommandHeader *header, const size_t count, const size_t typeSize, const bool clear) {
	if (state == fpl_null || header == fpl_null || count == 0 || typeSize == 0) {
		return fpl_null;
	}
	size_t size = typeSize * count;
	void *result = fmemPush(&state->memory, size, clear ? fmemPushFlags_Clear : fmemPushFlags_None);
	header->dataSize += size;
	return result;
}

#define PushTypesAs(state, header, count, type, clear) (type*)PushTypes(state, header, count, sizeof(type), clear)
#define PushTypeAs(state, header, type, clear) (type*)PushTypes(state, header, 1, sizeof(type), clear)

extern void RenderInit(RenderState *state, fmemMemoryBlock block) {
	if (state == fpl_null) {
		return;
	}
	state->memory = block;
	state->textureOperationCount = 0;
}

extern void RenderReset(RenderState *state) {
	if (state == fpl_null) {
		return;
	}
	state->lastMemoryUsage = state->memory.used;
	state->memory.used = 0;
}

extern void PushMatrix(RenderState *state, const Mat4f *mat, const MatrixMode mode) {
	if (state == fpl_null || mat == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Matrix);
	MatrixCommand *cmd = PushTypeAs(state, header, MatrixCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->mat = *mat;
	cmd->mode = mode;
}

extern void PopMatrix(RenderState *state) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Matrix);
	MatrixCommand *cmd = PushTypeAs(state, header, MatrixCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->mode = MatrixMode_Pop;
}

extern void SetMatrix(RenderState *state, const Mat4f *mat) {
	if (state == fpl_null || mat == fpl_null) {
		return;
	}
	PushMatrix(state, mat, MatrixMode_Set);
}

extern void PushClear(RenderState *state, const Vec4f color, const ClearFlags flags) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Clear);
	ClearCommand *cmd = PushTypeAs(state, header, ClearCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->color = color;
	cmd->flags = flags;
}

extern void PushViewport(RenderState *state, const int x, const int y, const int w, const int h) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Viewport);
	ViewportCommand *cmd = PushTypeAs(state, header, ViewportCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
}

extern void PushRectangle(RenderState *state, const Vec2f bottomLeft, const Vec2f size, const Vec4f color, const bool isFilled, const float lineWidth) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Rectangle);
	RectangleCommand *cmd = PushTypeAs(state, header, RectangleCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->bottomLeft = bottomLeft;
	cmd->size = size;
	cmd->color = color;
	cmd->isFilled = isFilled;
	cmd->lineWidth = lineWidth;
}

extern void PushRectangleCenter(RenderState *state, const Vec2f center, const Vec2f ext, const Vec4f color, const bool isFilled, const float lineWidth) {
	Vec2f bottomLeft = V2fSub(center, ext);
	Vec2f size = V2fMultScalar(ext, 2.0f);
	PushRectangle(state, bottomLeft, size, color, isFilled, lineWidth);
}

extern void PushQuad(RenderState *state, const Vec2f center, const float radius, const Vec4f color, const bool isFilled, const float lineWidth) {
	Vec2f ext = V2fInitScalar(radius);
	PushRectangleCenter(state, center, ext, color, isFilled, lineWidth);
}

extern VertexAllocation AllocateVertices(RenderState *state, const size_t capacity, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness) {
	VertexAllocation result = fplZeroInit;
	if (state == fpl_null) {
		return result;
	}
	CommandHeader *header = PushHeader(state, CommandType_Vertices);
	VerticesCommand *cmd = PushTypeAs(state, header, VerticesCommand, true);
	Vec2f *verts = PushTypesAs(state, header, capacity, Vec2f, true);
	if (cmd == fpl_null || verts == fpl_null) {
		return result;
	}
	cmd->capacity = capacity;
	cmd->count = 0;
	cmd->color = color;
	cmd->drawMode = drawMode;
	cmd->thickness = thickness;
	cmd->isLoop = isLoop;
	cmd->verts = verts;

	result.verts = verts;
	result.count = &cmd->count;
	return result;
}

extern void PushVertices(RenderState *state, const Vec2f *verts, const size_t vertexCount, const bool copyVerts, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness) {
	if (state == fpl_null || verts == fpl_null || vertexCount == 0) {
		return;
	}

	CommandHeader *header = PushHeader(state, CommandType_Vertices);
	VerticesCommand *cmd = PushTypeAs(state, header, VerticesCommand, true);
	if (cmd == fpl_null) {
		return;
	}

	if (copyVerts) {
		Vec2f *dstVerts = PushTypesAs(state, header, vertexCount, Vec2f, true);
		if (dstVerts == fpl_null) {
			return;
		}
		for (size_t i = 0; i < vertexCount; ++i) {
			dstVerts[i] = verts[i];
		}
		cmd->verts = dstVerts;
	} else {
		cmd->verts = verts;
	}

	cmd->capacity = vertexCount;
	cmd->count = vertexCount;
	cmd->color = color;
	cmd->drawMode = drawMode;
	cmd->thickness = thickness;
	cmd->isLoop = isLoop;
}

extern void PushSprite(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect) {
	if (state == fpl_null) {
		return;
	}

	CommandHeader *header = PushHeader(state, CommandType_Sprite);
	SpriteCommand *cmd = PushTypeAs(state, header, SpriteCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->position = position;
	cmd->ext = ext;
	cmd->texture = texture;
	cmd->color = color;
	cmd->uvMin = V2fInit(uvRect.uMin, uvRect.vMin);
	cmd->uvMax = V2fInit(uvRect.uMax, uvRect.vMax);
}

extern void PushTexture(RenderState *state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied) {
	if (state == fpl_null || targetTexture == fpl_null || data == fpl_null || width == 0 || height == 0) {
		return;
	}
	if (state->textureOperationCount >= fplArrayCount(state->textureOperations)) {
		return;
	}
	TextureOperation *op = &state->textureOperations[state->textureOperationCount++];
	fplClearStruct(op);
	op->data = data;
	op->width = width;
	op->height = height;
	op->bytesPerPixel = bytesPerPixel;
	op->type = TextureOperationType_Upload;
	op->wrap = wrap;
	op->filter = filter;
	op->handle = targetTexture;
	op->isPreMultiplied = isPreMultiplied;
	op->isTopDown = isTopDown;
}

extern void PopTexture(RenderState *state, TextureHandle *targetTexture) {
	if (state == fpl_null || targetTexture == fpl_null) {
		return;
	}
	if (state->textureOperationCount >= fplArrayCount(state->textureOperations)) {
		return;
	}
	TextureOperation *op = &state->textureOperations[state->textureOperationCount++];
	fplClearStruct(op);
	op->handle = targetTexture;
	op->type = TextureOperationType_Release;
}

extern void PushCircle(RenderState *state, const Vec2f position, const float radius, const size_t segmentCount, const Vec4f color, const bool isFilled, const float lineWidth) {
	if (state == fpl_null || radius <= 0.0f || segmentCount < 3) {
		return;
	}
	float seg = F32Tau / (float)segmentCount;
	size_t vertexCapacity = segmentCount;
	DrawMode drawMode;
	if (isFilled) {
		drawMode = DrawMode_Polygon;
	} else {
		drawMode = DrawMode_Lines;
	}
	VertexAllocation vertAlloc = AllocateVertices(state, vertexCapacity, color, drawMode, true, lineWidth);
	if (vertAlloc.count == 0 || vertAlloc.verts == fpl_null) {
		return;
	}
	size_t vertexCount = 0;
	Vec2f *p = vertAlloc.verts;
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
		float x = position.x + F32Cos(segmentIndex * seg) * radius;
		float y = position.y + F32Sin(segmentIndex * seg) * radius;
		*p++ = V2fInit(x, y);
		++vertexCount;
	}
	*vertAlloc.count = vertexCount;
}

extern void PushText(RenderState *state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle texture, const Vec2f position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f color) {
	if (state == fpl_null || text == fpl_null || textLen == 0 || font == fpl_null || texture == fpl_null) {
		return;
	}
	CommandHeader *header = PushHeader(state, CommandType_Text);
	TextCommand *cmd = PushTypeAs(state, header, TextCommand, true);
	if (cmd == fpl_null) {
		return;
	}

	size_t capacity = textLen + 1;

	char *pt = PushTypesAs(state, header, capacity, char, false);
	if (pt == fpl_null) {
		return;
	}
	fplCopyStringLen(text, textLen, pt, capacity);

	cmd->position = position;
	cmd->texture = texture;
	cmd->font = font;
	cmd->color = color;
	cmd->textLength = textLen;
	cmd->maxHeight = maxHeight;
	cmd->horizontalAlignment = horizontalAlignment;
	cmd->verticalAlignment = verticalAlignment;
}

extern void PushLine(RenderState *state, const Vec2f a, const Vec2f b, const Vec4f color, const float lineWidth) {
	Vec2f verts[] = {a, b};
	PushVertices(state, verts, 2, true, color, DrawMode_Lines, false, lineWidth);
}

extern Viewport ViewportComputeByAspect(const Vec2i screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2iInit(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2iInit(0, 0);
	if (targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport result = {viewOffset.x, viewOffset.y, viewSize.w, viewSize.h};
	return(result);
}

#endif // FINAL_RENDER_IMPLEMENTATION