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
	void *data;
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
struct CommandBuffer {
	TextureOperation textureOperations[MAX_TEXTURE_OPERATION_COUNT];
	fmemMemoryBlock memory;
	size_t textureOperationCount;
};

enum class CommandType {
	None = 0,
	Clear,
	Viewport
};

struct CommandHeader {
	size_t dataSize;
	CommandType type;
};

enum class ClearFlags : uint32_t {
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
};

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

struct VerticesCommand {
	Vec4f color;
	Vec2f *points;
	size_t pointCount;
	float lineWidth;
	bool isFilled;
};

struct SpriteCommand {
	Vec4f color;
	Vec2f position;
	Vec2f ext;
	Vec2f uvMin;
	Vec2f uvMax;
	TextureHandle texture;
};

extern void PushClear(CommandBuffer &buffer, const Vec4f &color, const ClearFlags flags);
extern void PushViewport(CommandBuffer &buffer, const int x, const int y, const int w, const int h);

#endif // FINAL_RENDER_H

#if defined(FINAL_RENDER_IMPLEMENTATION) && !defined(FINAL_RENDER_IMPLEMENTED)
#define FINAL_RENDER_IMPLEMENTED

static CommandHeader *PushHeader(CommandBuffer &buffer, const CommandType type) {
	CommandHeader *result = (CommandHeader *)fmemPush(&buffer.memory, sizeof(CommandHeader), fmemPushFlags_None);
	if(result != nullptr) {
		result->type = type;
		result->dataSize = 0;
	}
	return(result);
}

template<typename T>
static T *PushTypes(CommandBuffer &buffer, CommandHeader *header, const size_t count = 1, const bool clear = true) {
	if(header != nullptr) {
		size_t size = sizeof(T) * count;
		T *result = (T *)fmemPush(&buffer.memory, size, clear ? fmemPushFlags_Clear : fmemPushFlags_None);
		header->dataSize += size;
		return(result);
	}
	return nullptr;
}

extern void PushClear(CommandBuffer &buffer, const Vec4f &color, const ClearFlags flags) {
	CommandHeader *header = PushHeader(buffer, CommandType::Clear);
	ClearCommand *clearCmd = PushTypes<ClearCommand>(buffer, header);
	if(clearCmd != nullptr) {
		clearCmd->color = color;
		clearCmd->flags = flags;
	}
}

extern void PushViewport(CommandBuffer &buffer, const int x, const int y, const int w, const int h) {
	CommandHeader *header = PushHeader(buffer, CommandType::Viewport);
	ViewportCommand *cmd = PushTypes<ViewportCommand>(buffer, header);
	if(cmd != nullptr) {
		cmd->x = x;
		cmd->y = y;
		cmd->w = w;
		cmd->h = h;
	}
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