#ifndef RENDER_H
#define RENDER_H

#include <assert.h>
#include <stack>

#include "vecmath.h"
#include "threading.h"
#include "memory.h"
#include "font.h"

namespace Render {
	enum class TextureOperationType {
		Allocate,
		Release,
	};

	typedef void *TextureHandle;

	struct TextureOperationAllocate {
		uint32_t width;
		uint32_t height;
		uint32_t bytesPerPixel;
		void *data;
		TextureHandle *targetHandle;
		bool isTopDown;
		bool isPreMultiplied;
	};

	struct TextureOperationRelease {
		TextureHandle *handle;
	};

	struct TextureOperation {
		TextureOperationType type;
		union {
			TextureOperationAllocate allocate;
			TextureOperationRelease release;
		};
	};

	enum class CommandType {
		Rectangle,
		Sprite,
		Lines,
		Polygon,
		Circle,
		VertexIndexHeader,
		VerticesDraw,
		IndicesDraw,
		Viewport,
		OrthoProjection,
		Clear,
		Attribute,
	};

	struct Clear {
		Vec4f color;
		bool isColor;
		bool isDepth;
	};

	struct Viewport {
		int x;
		int y;
		int w;
		int h;
	};

	struct OrthoProjection {
		float left;
		float right;
		float bottom;
		float top;
		float nearClip;
		float farClip;
	};

	struct ClipRect {
		float x;
		float y;
		float w;
		float h;
		bool isActive;
	};

	struct Rectangle {
		Vec2f bottomLeft;
		Vec2f size;
		Vec4f color;
		float lineWidth;
		bool isFilled;
	};

	struct Vertices {
		Vec4f color;
		size_t pointCount;
		Vec2f *points;
		float lineWidth;
		bool isFilled;
	};

	struct Circle {
		Vec2f position;
		float radius;
		Vec4f color;
		float lineWidth;
		bool isFilled;
	};

	struct Vertex {
		Vec2f position;
		Vec2f texcoord;
		Vec4f color;
	};

	enum class PrimitiveType {
		Points,
		Triangles,
	};

	struct VertexIndexArrayHeader {
		uint32_t vertexStride;
		uint32_t colorStride;
		uint32_t texcoordStride;
		uint32_t indexSize;
		void *vertices;
		void *colors;
		void *texcoords;
		void *indices;
	};

	struct VertexIndexArrayDraw {
		TextureHandle texture;
		float pointSize;
		uint32_t count;
		ClipRect clipRect;
		PrimitiveType drawType;
	};

	enum class Attribute {
		AlphaBlending,
		BackfaceCulling,
		DepthTest,
		ScissorTest,
		Texture2D,
	};

	struct AttributeState {
		Attribute attribute;
		bool boolValue;
	};

	struct Sprite {
		Vec2f position;
		Vec2f size;
		Vec2f uvMin;
		Vec2f uvMax;
		Vec4f color;
		TextureHandle texture;
	};

	struct CommandBuffer {
		MemoryBlock commands;
		MemoryBlock textureData;
		std::stack<TextureOperation> textureOperations;

		CommandBuffer() {
			commands = AllocateMemoryBlock(1024 * 1024 * 16);
			textureData = AllocateMemoryBlock(1024 * 1024 * 4);
		}

		~CommandBuffer() {
			ReleaseMemoryBlock(&textureData);
			ReleaseMemoryBlock(&commands);
		}
	};

	struct CommandHeader {
		size_t dataSize;
		CommandType type;
	};

	inline CommandHeader *PushHeader(CommandBuffer *commandBuffer, CommandType type) {
		CommandHeader *result = PushStruct<CommandHeader>(&commandBuffer->commands, true);
		result->type = type;
		return(result);
	}

	template<typename T>
	inline T *PushTypes(CommandBuffer *commandBuffer, CommandHeader *header, const size_t count = 1, const bool clear = true) {
		size_t size = sizeof(T) * count;
		T *result = PushSize<T>(&commandBuffer->commands, size, clear);
		header->dataSize += size;
		return(result);
	}

	inline void AllocateTexture(CommandBuffer *commandBuffer, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, void *data, const bool isTopDown, const bool isPreMultiplied, TextureHandle *targetHandle) {
		TextureOperation texOperation = {};
		texOperation.type = TextureOperationType::Allocate;
		texOperation.allocate.bytesPerPixel = bytesPerPixel;
		texOperation.allocate.data = data;
		texOperation.allocate.width = width;
		texOperation.allocate.height = height;
		texOperation.allocate.targetHandle = targetHandle;
		texOperation.allocate.isTopDown = isTopDown;
		texOperation.allocate.isPreMultiplied = isPreMultiplied;
		commandBuffer->textureOperations.push(texOperation);
		*targetHandle = nullptr;
	}

	inline void ReleaseTexture(CommandBuffer *commandBuffer, TextureHandle *handle) {
		TextureOperation texOperation = {};
		texOperation.type = TextureOperationType::Release;
		texOperation.release.handle = handle;
		commandBuffer->textureOperations.push(texOperation);
	}

	inline void PushRectangle(CommandBuffer *commandBuffer, const Vec2f &bottomLeft, const Vec2f &size, const Vec4f &color, const bool isFilled, const float lineWidth = 1.0f) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Rectangle);
		Rectangle *rectangle = PushTypes<Rectangle>(commandBuffer, header);
		rectangle->bottomLeft = bottomLeft;
		rectangle->size = size;
		rectangle->color = color;
		rectangle->lineWidth = lineWidth;
		rectangle->isFilled = isFilled;
	}

	inline void PushSprite(CommandBuffer *commandBuffer, const Vec2f &pos, const Vec2f &size, TextureHandle texture, const Vec4f &color, const Vec2f &uvMin = Vec2f(0, 0), const Vec2f &uvMax = Vec2f(1.0f, 1.0f)) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Sprite);
		Sprite *sprite = PushTypes<Sprite>(commandBuffer, header);
		sprite->position = pos;
		sprite->size = size;
		sprite->color = color;
		sprite->uvMin = uvMin;
		sprite->uvMax = uvMax;
		sprite->texture = texture;
	}

	inline void PushLine(CommandBuffer *commandBuffer, const Vec2f &a, const Vec2f &b, const Vec4f &color, const float lineWidth) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Lines);
		Vertices *verts = PushTypes<Vertices>(commandBuffer, header);
		verts->pointCount = 2;
		verts->points = PushTypes<Vec2f>(commandBuffer, header, 2);
		verts->points[0] = a;
		verts->points[1] = b;
		verts->color = color;
		verts->lineWidth = lineWidth;
	}

	inline void PushLinesFrom(CommandBuffer *commandBuffer, Vec2f *points, const size_t pointCount, const Vec4f &color, const float lineWidth) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Lines);
		Vertices *verts = PushTypes<Vertices>(commandBuffer, header);
		verts->pointCount = pointCount;
		verts->points = PushTypes<Vec2f>(commandBuffer, header, pointCount);
		for (size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
			verts->points[pointIndex] = points[pointIndex];
		}
		verts->color = color;
		verts->lineWidth = lineWidth;
	}

	inline Vec2f *PushPolygon(CommandBuffer *commandBuffer, const size_t pointCount, const Vec4f &color, bool isFilled, const float lineWidth = 1.0f) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Polygon);
		Vertices *verts = PushTypes<Vertices>(commandBuffer, header);
		verts->pointCount = pointCount;
		verts->points = PushTypes<Vec2f>(commandBuffer, header, pointCount);
		verts->color = color;
		verts->lineWidth = lineWidth;
		verts->isFilled = isFilled;
		return (verts->points);
	}

	inline void PushPolygonFrom(CommandBuffer *commandBuffer, Vec2f *points, const size_t pointCount, const Vec4f &color, bool isFilled, const float lineWidth = 1.0f) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Polygon);
		Vertices *verts = PushTypes<Vertices>(commandBuffer, header);
		verts->pointCount = pointCount;
		verts->points = PushTypes<Vec2f>(commandBuffer, header, pointCount);
		for (size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
			verts->points[pointIndex] = points[pointIndex];
		}
		verts->color = color;
		verts->lineWidth = lineWidth;
		verts->isFilled = isFilled;
	}

	inline Vec2f *PushLines(CommandBuffer *commandBuffer, const size_t pointCount, const Vec4f &color, const float lineWidth) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Lines);
		Vertices *verts = PushTypes<Vertices>(commandBuffer, header);
		verts->pointCount = pointCount;
		verts->points = PushTypes<Vec2f>(commandBuffer, header, pointCount);
		verts->color = color;
		verts->lineWidth = lineWidth;
		return(verts->points);
	}

	inline void PushCircle(CommandBuffer *commandBuffer, Vec2f center, const float radius, const Vec4f &color, const bool isFilled, const float lineWidth = 1.0f) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Circle);
		Circle *circle = PushTypes<Circle>(commandBuffer, header);
		circle->position = center;
		circle->radius = radius;
		circle->color = color;
		circle->lineWidth = lineWidth;
		circle->isFilled = isFilled;
	}

	inline void PushBoolAttribute(CommandBuffer *commandBuffer, Attribute attr, bool boolValue) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Attribute);
		AttributeState *attrState = PushTypes<AttributeState>(commandBuffer, header);
		attrState->attribute = attr;
		attrState->boolValue = boolValue;
	}

	inline void PushClear(CommandBuffer *commandBuffer, const bool isColor, const bool isDepth, const Vec4f &color) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Clear);
		Clear *clear = PushTypes<Clear>(commandBuffer, header);
		clear->isColor = isColor;
		clear->isDepth = isDepth;
		clear->color = color;
	}

	inline void PushViewport(CommandBuffer *commandBuffer, const int x, const int y, const int w, const int h) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::Viewport);
		Viewport *viewport = PushTypes<Viewport>(commandBuffer, header);
		viewport->x = x;
		viewport->y = y;
		viewport->w = w;
		viewport->h = h;
	}

	inline void PushOrthoProjection(CommandBuffer *commandBuffer, const float left, const float right, const float bottom, const float top, const float nearClip, const float farClip) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::OrthoProjection);
		OrthoProjection *proj = PushTypes<OrthoProjection>(commandBuffer, header);
		proj->left = left;
		proj->right = right;
		proj->bottom = bottom;
		proj->top = top;
		proj->nearClip = nearClip;
		proj->farClip = farClip;
	}

	inline void PushVertexIndexArrayHeader(CommandBuffer *commandBuffer, uint32_t vertexStride, void *vertices, uint32_t texcoordStride, void *texcoords, uint32_t colorStride, void *colors, uint32_t indexSize, void *indices) {
		CommandHeader *header = PushHeader(commandBuffer, CommandType::VertexIndexHeader);
		VertexIndexArrayHeader *result = PushTypes<VertexIndexArrayHeader>(commandBuffer, header);
		result->vertexStride = vertexStride;
		result->vertices = vertices;
		result->texcoordStride = texcoordStride;
		result->texcoords = texcoords;
		result->colorStride = colorStride;
		result->colors = colors;
		result->indexSize = indexSize;
		result->indices = indices;
	}

	inline void PushVertexIndexArrayDraw(CommandBuffer *commandBuffer, PrimitiveType drawType, uint32_t count, float pointSize, TextureHandle texture, const ClipRect &clipRect, bool useIndices) {
		CommandHeader *header = PushHeader(commandBuffer, useIndices ? CommandType::IndicesDraw : CommandType::VerticesDraw);
		VertexIndexArrayDraw *result = PushTypes<VertexIndexArrayDraw>(commandBuffer, header);
		result->drawType = drawType;
		result->count = count;
		result->texture = texture;
		result->clipRect = clipRect;
		result->pointSize = pointSize;
	}

	inline void PushText(CommandBuffer *commandBuffer, const Vec2f &bottomLeft, const char *text, Font *font, TextureHandle texture, const float maxCharHeight, const Vec4f &textColor) {
		if (font == nullptr) {
			return;
		}
		uint32_t textLen = (uint32_t)strlen(text);
		if (textLen == 0) {
			return;
		}

		CommandHeader *header = PushHeader(commandBuffer, CommandType::VertexIndexHeader);
		VertexIndexArrayHeader *vertexIndexArray = PushTypes<VertexIndexArrayHeader>(commandBuffer, header);

		uint32_t maxVertexCount = textLen * 4;
		uint32_t maxIndexCount = textLen * 6;
		Vertex *verts = PushTypes<Vertex>(commandBuffer, header, maxVertexCount, false);
		uint32_t *indices = PushTypes<uint32_t>(commandBuffer, header, maxIndexCount, false);

		vertexIndexArray->vertices = (void *)((uint8_t *)verts + offsetof(Vertex, position));
		vertexIndexArray->vertexStride = sizeof(Vertex);
		vertexIndexArray->texcoords = (void *)((uint8_t *)verts + offsetof(Vertex, texcoord));
		vertexIndexArray->texcoordStride = sizeof(Vertex);
		vertexIndexArray->colors = (void *)((uint8_t *)verts + offsetof(Vertex, color));
		vertexIndexArray->colorStride = sizeof(Vertex);
		vertexIndexArray->indices = indices;
		vertexIndexArray->indexSize = sizeof(uint32_t);

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		float x = bottomLeft.x;
		float y = bottomLeft.y;
		uint32_t textPos = 0;
		while (textPos < textLen) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float advance;
			// @FIX: Codepoint wraps when gets negative, this is wrong!
			int32_t codePointTest = at - font->firstChar;
			if ((codePointTest >= 0) && (codePointTest < (int32_t)font->charCount)) {
				uint32_t codePoint = at - font->firstChar;
				FontGlyph *glyph = font->glyphs + codePoint;
				Vec2f offset = Vec2f(x, y);
				offset += Vec2Hadamard(glyph->charSize, glyph->alignPercentage) * maxCharHeight;
				offset -= Vec2f(glyph->charSize.x, glyph->charSize.y) * 0.5f * maxCharHeight;
				offset += Vec2f(0, maxCharHeight * 0.5f);
				Vec2f size = Vec2f(glyph->charSize.x, glyph->charSize.y) * maxCharHeight;

				uint32_t nextCodePoint = (atNext > 0) ? atNext - font->firstChar : 0;
				advance = GetFontCharacterAdvance(font, &codePoint, (atNext > 0) ? &nextCodePoint : nullptr) * maxCharHeight;

				Vertex *v0 = verts + (vertexCount + 0);
				Vertex *v1 = verts + (vertexCount + 1);
				Vertex *v2 = verts + (vertexCount + 2);
				Vertex *v3 = verts + (vertexCount + 3);

				uint32_t *i0 = indices + (indexCount + 0);
				uint32_t *i1 = indices + (indexCount + 1);
				uint32_t *i2 = indices + (indexCount + 2);
				uint32_t *i3 = indices + (indexCount + 3);
				uint32_t *i4 = indices + (indexCount + 4);
				uint32_t *i5 = indices + (indexCount + 5);

				// Top-
				*v0 = {};
				v0->color = textColor;
				v0->texcoord = Vec2f(glyph->uvMax.x, glyph->uvMax.y);
				v0->position = Vec2f(offset.x + size.w, offset.y + size.h);

				// Top-Left
				*v1 = {};
				v1->color = textColor;
				v1->texcoord = Vec2f(glyph->uvMin.x, glyph->uvMax.y);
				v1->position = Vec2f(offset.x, offset.y + size.h);

				// Bottom-Left
				*v2 = {};
				v2->color = textColor;
				v2->texcoord = Vec2f(glyph->uvMin.x, glyph->uvMin.y);
				v2->position = Vec2f(offset.x, offset.y);

				// Bottom-Right
				*v3 = {};
				v3->color = textColor;
				v3->texcoord = Vec2f(glyph->uvMax.x, glyph->uvMin.y);
				v3->position = Vec2f(offset.x + size.w, offset.y);

				// Indices for two triangles
				*i0 = vertexCount + 0;
				*i1 = vertexCount + 1;
				*i2 = vertexCount + 2;
				*i3 = vertexCount + 2;
				*i4 = vertexCount + 3;
				*i5 = vertexCount + 0;

				vertexCount += 4;
				indexCount += 6;
			} else {
				advance = font->info.spaceAdvance * maxCharHeight;
			}

			x += advance;
			++textPos;
		}
		assert(vertexCount <= maxVertexCount);
		assert(indexCount <= maxIndexCount);

		PushVertexIndexArrayDraw(commandBuffer, PrimitiveType::Triangles, indexCount, 1.0f, texture, ClipRect(), true);
	}

	inline void ResetCommandBuffer(CommandBuffer *commandBuffer) {
		commandBuffer->commands.offset = 0;
	}

};

#endif