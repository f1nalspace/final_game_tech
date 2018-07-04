/*
Name:
	Final Render

Description:
	Contains structures for computing render specific stuff and
	has a render command system.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_RENDER_H
#define FINAL_RENDER_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include <final_memory.h>

#include "final_math.h"
#include <final_fontloader.h>

struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
};

inline UVRect UVRectFromTile(const Vec2i &imageSize, const Vec2i &tileSize, const int border, const Vec2i &pos) {
	Vec2f texel = V2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	int imgX = border + pos.x * tileSize.x + border * pos.x;
	int imgY = border + pos.y * tileSize.y + border * pos.y;
	UVRect result;
	result.uMin = imgX * texel.x;
	result.vMin = imgY * texel.y;
	result.uMax = result.uMin + tileSize.x * texel.x;
	result.vMax = result.vMin + tileSize.y * texel.y;
	return(result);
}

inline UVRect UVRectFromPos(const Vec2i &imageSize, const Vec2i &partSize, const Vec2i &pos) {
	Vec2f texel = V2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
	return(result);
}

struct Viewport {
	int x;
	int y;
	int w;
	int h;
};

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect);

struct Camera2D {
	Vec2f offset;
	float worldToPixels;
	float pixelsToWorld;
	float scale;
};

enum class TextureOperationType {
	None = 0,
	Upload,
	Release
};

typedef void *TextureHandle;

enum class TextureFilterType {
	Nearest,
	Linear
};

enum class TextureWrapMode {
	Repeat,
	ClampToEdge,
	ClampToBorder,
};

struct TextureOperation {
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
};

constexpr size_t MAX_TEXTURE_OPERATION_COUNT = 1024;
constexpr size_t MAX_MATRIX_STACK_COUNT = 32;
struct RenderState {
	TextureOperation textureOperations[MAX_TEXTURE_OPERATION_COUNT];
	Mat4f matrixStack[MAX_MATRIX_STACK_COUNT];
	size_t matrixTop;
	fmemMemoryBlock memory;
	size_t textureOperationCount;
};

enum class CommandType {
	None = 0,
	Clear,
	Viewport,
	Matrix,
	Rectangle,
	Vertices,
	Sprite,
	Text
};

struct CommandHeader {
	size_t dataSize;
	CommandType type;
};

enum class MatrixMode {
	Set,
	Push,
	Pop
};

struct MatrixCommand {
	Mat4f mat;
	MatrixMode mode;
};

enum class ClearFlags : uint32_t {
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
};
FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

struct ClearCommand {
	Vec4f color;
	ClearFlags flags;
};

struct ViewportCommand {
	int x;
	int y;
	int w;
	int h;
};

struct RectangleCommand {
	Vec4f color;
	Vec2f bottomLeft;
	Vec2f size;
	float lineWidth;
	bool isFilled;
};

enum class VerticesDrawMode {
	None,
	Points,
	Lines,
	Triangles,
	Polygon
};

struct VerticesCommand {
	Vec4f color;
	const Vec2f *points;
	size_t pointCount;
	VerticesDrawMode drawMode;
	float thickness;
	bool isLoop;
};

struct SpriteCommand {
	Vec4f color;
	Vec2f position;
	Vec2f ext;
	Vec2f uvMin;
	Vec2f uvMax;
	TextureHandle texture;
};

struct TextCommand {
	Vec4f color;
	Vec2f position;
	const TextureHandle *texture;
	const LoadedFont *font;
	float horizontalAlignment;
	float verticalAlignment;
	float maxHeight;
	size_t textLength;
};

extern void InitRenderState(RenderState &state, fmemMemoryBlock block);
extern void ResetRenderState(RenderState &state);
extern void PushClear(RenderState &state, const Vec4f &color, const ClearFlags flags);
extern void PushViewport(RenderState &state, const int x, const int y, const int w, const int h);
extern void PushMatrix(RenderState &state, const Mat4f &mat, const MatrixMode mode = MatrixMode::Push);
extern void SetMatrix(RenderState &state, const Mat4f &mat);
extern void PopMatrix(RenderState &state);
extern void PushRectangle(RenderState &state, const Vec2f &bottomLeft, const Vec2f &size, const Vec4f &color, const bool isFilled, const float lineWidth);
extern void PushRectangleCenter(RenderState &state, const Vec2f &center, const Vec2f &ext, const Vec4f &color, const bool isFilled, const float lineWidth);
extern void PushVertices(RenderState &state, const Vec2f *points, const size_t pointCount, const bool copyPoints, const Vec4f &color, const VerticesDrawMode drawMode, const bool isLoop, const float radius);
extern void PushSprite(RenderState &state, const Vec2f &position, const Vec2f &ext, const TextureHandle texture, const Vec4f &color, const Vec2f &uvMin, const Vec2f &uvMax);
extern void PushSprite(RenderState &state, const Vec2f &position, const Vec2f &ext, const TextureHandle texture, const Vec4f &color, const UVRect &uvRect);
extern void PushTexture(RenderState &state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied);
extern void PopTexture(RenderState &state, TextureHandle *targetTexture);
extern void PushText(RenderState &state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle *texture, const Vec2f &position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f &color);

#endif // FINAL_RENDER_H

#if defined(FINAL_RENDER_IMPLEMENTATION) && !defined(FINAL_RENDER_IMPLEMENTED)
#define FINAL_RENDER_IMPLEMENTED

extern void InitRenderState(RenderState &state, fmemMemoryBlock block) {
	state.memory = block;
	state.textureOperationCount = 0;
}

extern void ResetRenderState(RenderState &state) {
	state.memory.used = 0;
}

static CommandHeader *PushHeader(RenderState &state, const CommandType type) {
	CommandHeader *result = (CommandHeader *)fmemPush(&state.memory, sizeof(CommandHeader), fmemPushFlags_None);
	if(result != nullptr) {
		result->type = type;
		result->dataSize = 0;
	}
	return(result);
}

template<typename T>
static T *PushTypes(RenderState &state, CommandHeader *header, const size_t count = 1, const bool clear = true) {
	if(header != nullptr) {
		size_t size = sizeof(T) * count;
		T *result = (T *)fmemPush(&state.memory, size, clear ? fmemPushFlags_Clear : fmemPushFlags_None);
		header->dataSize += size;
		return(result);
	}
	return nullptr;
}

extern void PushMatrix(RenderState &state, const Mat4f &mat, const MatrixMode mode) {
	CommandHeader *header = PushHeader(state, CommandType::Matrix);
	MatrixCommand *cmd = PushTypes<MatrixCommand>(state, header);
	if(cmd != nullptr) {
		cmd->mat = mat;
		cmd->mode = mode;
	}
}

extern void PopMatrix(RenderState &state) {
	CommandHeader *header = PushHeader(state, CommandType::Matrix);
	MatrixCommand *cmd = PushTypes<MatrixCommand>(state, header);
	if(cmd != nullptr) {
		cmd->mode = MatrixMode::Pop;
	}
}

extern void SetMatrix(RenderState &state, const Mat4f &mat) {
	PushMatrix(state, mat, MatrixMode::Set);
}

extern void PushClear(RenderState &state, const Vec4f &color, const ClearFlags flags) {
	CommandHeader *header = PushHeader(state, CommandType::Clear);
	ClearCommand *cmd = PushTypes<ClearCommand>(state, header);
	if(cmd != nullptr) {
		cmd->color = color;
		cmd->flags = flags;
	}
}

extern void PushViewport(RenderState &state, const int x, const int y, const int w, const int h) {
	CommandHeader *header = PushHeader(state, CommandType::Viewport);
	ViewportCommand *cmd = PushTypes<ViewportCommand>(state, header);
	if(cmd != nullptr) {
		cmd->x = x;
		cmd->y = y;
		cmd->w = w;
		cmd->h = h;
	}
}

extern void PushRectangle(RenderState &state, const Vec2f &bottomLeft, const Vec2f &size, const Vec4f &color, const bool isFilled, const float lineWidth) {
	CommandHeader *header = PushHeader(state, CommandType::Rectangle);
	RectangleCommand *cmd = PushTypes<RectangleCommand>(state, header);
	if(cmd != nullptr) {
		cmd->bottomLeft = bottomLeft;
		cmd->size = size;
		cmd->color = color;
		cmd->isFilled = isFilled;
		cmd->lineWidth = lineWidth;
	}
}

extern void PushRectangleCenter(RenderState &state, const Vec2f &center, const Vec2f &ext, const Vec4f &color, const bool isFilled, const float lineWidth) {
	PushRectangle(state, center - ext, ext * 2.0f, color, isFilled, lineWidth);
}

extern void PushVertices(RenderState &state, const Vec2f *points, const size_t pointCount, const bool copyPoints, const Vec4f &color, const VerticesDrawMode drawMode, const bool isLoop, const float thickness) {
	CommandHeader *header = PushHeader(state, CommandType::Vertices);
	VerticesCommand *cmd = PushTypes<VerticesCommand>(state, header);
	if(cmd != nullptr) {
		if(copyPoints) {
			Vec2f *dstPoints = PushTypes<Vec2f>(state, header, pointCount);
			for(size_t i = 0; i < pointCount; ++i) {
				dstPoints[i] = points[i];
			}
			cmd->points = dstPoints;
		} else {
			cmd->points = points;
		}
		cmd->pointCount = pointCount;
		cmd->color = color;
		cmd->drawMode = drawMode;
		cmd->thickness = thickness;
		cmd->isLoop = isLoop;
	}
}

extern void PushSprite(RenderState &state, const Vec2f &position, const Vec2f &ext, const TextureHandle texture, const Vec4f &color, const Vec2f &uvMin, const Vec2f &uvMax) {
	CommandHeader *header = PushHeader(state, CommandType::Sprite);
	SpriteCommand *cmd = PushTypes<SpriteCommand>(state, header);
	if(cmd != nullptr) {
		cmd->position = position;
		cmd->ext = ext;
		cmd->texture = texture;
		cmd->color = color;
		cmd->uvMin = uvMin;
		cmd->uvMax = uvMax;
	}
}

extern void PushSprite(RenderState &state, const Vec2f &position, const Vec2f &ext, const TextureHandle texture, const Vec4f &color, const UVRect &uvRect) {
	PushSprite(state, position, ext, texture, color, V2f(uvRect.uMin, uvRect.vMin), V2f(uvRect.uMax, uvRect.vMax));
}

extern void PushTexture(RenderState &state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied) {
	if(state.textureOperationCount < FPL_ARRAYCOUNT(state.textureOperations)) {
		TextureOperation *op = &state.textureOperations[state.textureOperationCount++];
		*op = {};
		op->data = data;
		op->width = width;
		op->height = height;
		op->bytesPerPixel = bytesPerPixel;
		op->type = TextureOperationType::Upload;
		op->wrap = wrap;
		op->filter = filter;
		op->handle = targetTexture;
		op->isPreMultiplied = isPreMultiplied;
		op->isTopDown = isTopDown;
	}
}

extern void PopTexture(RenderState &state, TextureHandle *targetTexture) {
	if(state.textureOperationCount < FPL_ARRAYCOUNT(state.textureOperations)) {
		TextureOperation *op = &state.textureOperations[state.textureOperationCount++];
		*op = {};
		op->handle = targetTexture;
		op->type = TextureOperationType::Release;
	}
}

extern void PushText(RenderState &state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle *texture, const Vec2f &position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f &color) {
	CommandHeader *header = PushHeader(state, CommandType::Text);
	TextCommand *cmd = PushTypes<TextCommand>(state, header);
	if(cmd != nullptr) {
		cmd->position = position;
		cmd->texture = texture;
		cmd->font = font;
		cmd->color = color;
		cmd->textLength = textLen;
		cmd->maxHeight = maxHeight;
		cmd->horizontalAlignment = horizontalAlignment;
		cmd->verticalAlignment = verticalAlignment;

	}
	char *pt = PushTypes<char>(state, header, textLen + 1, false);
	fplCopyAnsiStringLen(text, textLen, pt, textLen + 1);
}

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2i(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2i(0, 0);
	if(targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport result = { viewOffset.x, viewOffset.y, viewSize.w, viewSize.h };
	return(result);
}

#endif // FINAL_RENDER_IMPLEMENTATION