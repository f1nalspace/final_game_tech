/*
Temporary: Remove it when render commands are fully implemented
*/

#ifndef FINAL_OPENGL_RENDER_H
#define FINAL_OPENGL_RENDER_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#include <final_math.h>
#include <final_render.h>
#include <final_fontloader.h>
#include <final_dynamic_opengl.h>

#include <stdint.h>

extern void DrawSprite(const GLuint texId, const float rx, const float ry, const float uMin = 0.0f, const float vMin = 0.0f, const float uMax = 1.0f, const float vMax = 1.0f, const float xoffset = 0, const float yoffset = 0);
extern void DrawSprite(const GLuint texId, const float rx, const float ry, const UVRect &uv, const float xoffset = 0, const float yoffset = 0);
extern void DrawPoint(const Camera2D &camera, const float x, const float y, const float radius, const Vec4f &color);
extern void DrawTextFont(const char *text, const size_t textLen, const LoadedFont *fontDesc, const GLuint fontTexture, const float x, const float y, const float maxCharHeight, const float sx, const float sy);
extern void DrawCircle(const float centerX, const float centerY, const float radius, const bool isFilled, const Vec4f &color, const int segments = 16);
extern void DrawNormal(const Vec2f &pos, const Vec2f &normal, const float length, const Vec4f &color);
extern GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable, const GLint filter, const bool isAlphaOnly = false);
extern void InitOpenGLRenderer();
extern void RenderWithOpenGL(RenderState &renderState);

#endif // FINAL_OPENGL_RENDER_H

#if defined(FINAL_OPENGL_RENDER_IMPLEMENTATION) && !defined(FINAL_OPENGL_RENDER_IMPLEMENTED)
#define FINAL_OPENGL_RENDER_IMPLEMENTED

#include <final_utils.h>

extern void DrawSprite(const GLuint texId, const float rx, const float ry, const float uMin, const float vMin, const float uMax, const float vMax, const float xoffset, const float yoffset) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glTexCoord2f(uMax, vMax); glVertex2f(xoffset + rx, yoffset + ry);
	glTexCoord2f(uMin, vMax); glVertex2f(xoffset + -rx, yoffset + ry);
	glTexCoord2f(uMin, vMin); glVertex2f(xoffset + -rx, yoffset + -ry);
	glTexCoord2f(uMax, vMin); glVertex2f(xoffset + rx, yoffset + -ry);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

extern void DrawSprite(const GLuint texId, const float rx, const float ry, const UVRect &uv, const float xoffset, const float yoffset) {
	DrawSprite(texId, rx, ry, uv.uMin, uv.vMax, uv.uMax, uv.vMin, xoffset, yoffset);
}

extern void DrawPoint(const Camera2D &camera, const float x, const float y, const float radius, const Vec4f &color) {
	glColor4fv(&color.r);
	glPointSize(radius * 2.0f * camera.worldToPixels);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
	glPointSize(1);
}


extern void DrawTextFont(const char *text, const size_t textLen, const LoadedFont *fontDesc, const GLuint fontTexture, const float x, const float y, const float maxCharHeight, const float sx, const float sy) {
	if(fontDesc != nullptr) {
		Vec2f textSize = GetTextSize(text, textLen, fontDesc, maxCharHeight);
		float xpos = x - textSize.w * 0.5f + (textSize.w * 0.5f * sx);
		float ypos = y - textSize.h * 0.5f + (textSize.h * 0.5f * sy);
		uint32_t lastChar = fontDesc->firstChar + (fontDesc->charCount - 1);
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float advance;
			if((uint32_t)at >= fontDesc->firstChar && (uint32_t)at <= lastChar) {
				uint32_t codePoint = at - fontDesc->firstChar;
				const FontGlyph *glyph = &fontDesc->glyphs[codePoint];
				Vec2f size = glyph->charSize * maxCharHeight;
				Vec2f offset = V2f(xpos, ypos);
				offset += glyph->offset * maxCharHeight;
				offset += V2f(size.x, -size.y) * 0.5f;
				DrawSprite(fontTexture, size.x * 0.5f, size.y * 0.5f, glyph->uvMin.x, glyph->uvMin.y, glyph->uvMax.x, glyph->uvMax.y, offset.x, offset.y);
				advance = GetFontCharacterAdvance(fontDesc, at, atNext) * maxCharHeight;
			} else {
				advance = fontDesc->info.spaceAdvance * maxCharHeight;
			}
			xpos += advance;
		}
	}
}

extern void DrawCircle(const float centerX, const float centerY, const float radius, const bool isFilled, const Vec4f &color, const int segments) {
	float seg = Tau32 / (float)segments;
	glColor4fv(&color.r);
	glBegin(isFilled ? GL_POLYGON : GL_LINE_LOOP);
	for(int segmentIndex = 0; segmentIndex < segments; ++segmentIndex) {
		float x = centerX + Cosine(segmentIndex * seg) * radius;
		float y = centerY + sinf(segmentIndex * seg) * radius;
		glVertex2f(x, y);
	}
	glEnd();
}

extern void DrawNormal(const Vec2f &pos, const Vec2f &normal, const float length, const Vec4f &color) {
	glColor4fv(&color.r);
	glBegin(GL_LINES);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x + normal.x * length, pos.y + normal.y * length);
	glEnd();
}

extern GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable, const GLint filter, const bool isAlphaOnly) {
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint internalFormat = isAlphaOnly ? GL_ALPHA8 : GL_RGBA8;
	GLenum format = isAlphaOnly ? GL_ALPHA : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	return(handle);
}

extern void InitOpenGLRenderer() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
}

extern void RenderWithOpenGL(RenderState &renderState) {
	size_t index = 0;
	while(renderState.textureOperationCount > 0) {
		TextureOperation &op = renderState.textureOperations[index];
		if(op.type == TextureOperationType::Upload) {
			bool isAlphaOnly = op.bytesPerPixel == 1;
			GLuint texId = AllocateTexture(op.width, op.height, op.data, false, GL_LINEAR, isAlphaOnly);
			*op.handle = ValueToPointer<GLuint>(texId);
		} else if(op.type == TextureOperationType::Release) {
			GLuint texId = PointerToValue<GLuint>(op.handle);
			if(texId > 0) {
				glDeleteTextures(1, &texId);
				*op.handle = nullptr;
			}
		}
		--renderState.textureOperationCount;
		++index;
	}
	fplAssert(renderState.textureOperationCount == 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(renderState.memory.size > sizeof(CommandHeader)) {
		uint8_t *mem = (uint8_t *)renderState.memory.base;
		size_t remaining = renderState.memory.used;
		Mat4f mvpCur = M4f();
		renderState.matrixTop = 0;
		while(remaining > 0) {
			uint8_t *startMem = mem;
			CommandHeader *header = (CommandHeader *)mem;
			mem += sizeof(*header);
			uint8_t *dataStart = mem;
			size_t dataSize = header->dataSize;
			switch(header->type) {
				case CommandType::Viewport:
				{
					fplAssert(dataSize == sizeof(ViewportCommand));
					ViewportCommand *cmd = (ViewportCommand *)dataStart;
					glViewport(cmd->x, cmd->y, cmd->w, cmd->h);
				} break;

				case CommandType::Clear:
				{
					fplAssert(dataSize == sizeof(ClearCommand));
					ClearCommand *cmd = (ClearCommand *)dataStart;
					GLbitfield mask = 0;
					if((cmd->flags & ClearFlags::Color) == ClearFlags::Color) {
						mask |= GL_COLOR_BUFFER_BIT;
					}
					if((cmd->flags & ClearFlags::Depth) == ClearFlags::Depth) {
						mask |= GL_DEPTH_BUFFER_BIT;
					}
					glClearColor(cmd->color.r, cmd->color.g, cmd->color.g, cmd->color.a);
					glClear(mask);
				} break;

				case CommandType::Matrix:
				{
					fplAssert(dataSize == sizeof(MatrixCommand));
					MatrixCommand *cmd = (MatrixCommand *)dataStart;
					if(cmd->mode == MatrixMode::Set) {
						renderState.matrixTop = 0;
						mvpCur = cmd->mat;
					} else if(cmd->mode == MatrixMode::Push) {
						fplAssert(renderState.matrixTop < fplArrayCount(renderState.matrixStack));
						Mat4f *newMatrix = &renderState.matrixStack[renderState.matrixTop++];
						*newMatrix = mvpCur;
						mvpCur = *newMatrix * cmd->mat;
					} else if(cmd->mode == MatrixMode::Pop) {
						fplAssert(renderState.matrixTop > 0);
						mvpCur = renderState.matrixStack[--renderState.matrixTop];
					}
					glMatrixMode(GL_MODELVIEW);
					glLoadMatrixf(&mvpCur.m[0]);
				} break;

				case CommandType::Rectangle:
				{
					fplAssert(dataSize == sizeof(RectangleCommand));
					RectangleCommand *cmd = (RectangleCommand *)dataStart;
					if(!cmd->isFilled) {
						glLineWidth(cmd->lineWidth);
					}
					glColor4fv(&cmd->color.m[0]);
					glBegin(cmd->isFilled ? GL_QUADS : GL_LINE_LOOP);
					glVertex2f(cmd->bottomLeft.x + cmd->size.w, cmd->bottomLeft.y + cmd->size.h);
					glVertex2f(cmd->bottomLeft.x, cmd->bottomLeft.y + cmd->size.h);
					glVertex2f(cmd->bottomLeft.x, cmd->bottomLeft.y);
					glVertex2f(cmd->bottomLeft.x + cmd->size.w, cmd->bottomLeft.y);
					glEnd();
				} break;

				case CommandType::Sprite:
				{
					fplAssert(dataSize == sizeof(SpriteCommand));
					SpriteCommand *cmd = (SpriteCommand *)dataStart;
					GLuint texId = PointerToValue<GLuint>(cmd->texture);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, texId);
					glColor4fv(&cmd->color.m[0]);
					glBegin(GL_QUADS);
					glTexCoord2f(cmd->uvMax.x, cmd->uvMax.y); glVertex2f(cmd->position.x + cmd->ext.w, cmd->position.y + cmd->ext.h);
					glTexCoord2f(cmd->uvMin.x, cmd->uvMax.y); glVertex2f(cmd->position.x - cmd->ext.w, cmd->position.y + cmd->ext.h);
					glTexCoord2f(cmd->uvMin.x, cmd->uvMin.y); glVertex2f(cmd->position.x - cmd->ext.w, cmd->position.y - cmd->ext.h);
					glTexCoord2f(cmd->uvMax.x, cmd->uvMin.y); glVertex2f(cmd->position.x + cmd->ext.w, cmd->position.y - cmd->ext.h);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
				} break;

				case CommandType::Vertices:
				{
					fplAssert(dataSize >= sizeof(VerticesCommand));
					VerticesCommand *cmd = (VerticesCommand *)dataStart;
					glColor4fv(&cmd->color.m[0]);
					GLenum drawMode;
					switch(cmd->drawMode) {
						case DrawMode::Lines:
						{
							glLineWidth(cmd->thickness);
							drawMode = cmd->isLoop ? GL_LINE_LOOP : GL_LINES;
						} break;

						case DrawMode::Points:
						{
							glPointSize(cmd->thickness);
							drawMode = GL_POINTS;
						} break;

						case DrawMode::Polygon:
						{
							drawMode = GL_POLYGON;
						} break;

						case DrawMode::Triangles:
						{
							drawMode = cmd->isLoop ? GL_TRIANGLE_FAN : GL_POLYGON;
						} break;

						default:
							drawMode = GL_POLYGON;
					}
					glBegin(drawMode);
					for(size_t i = 0; i < cmd->count; ++i) {
						glVertex2fv(&cmd->verts[i].m[0]);
					}
					glEnd();
				} break;

				case CommandType::Text:
				{
					fplAssert(dataSize >= sizeof(TextCommand));
					TextCommand *cmd = (TextCommand *)dataStart;
					fplAssert(dataSize == (sizeof(TextCommand) + cmd->textLength + 1));
					const char *text = (const char *)(dataStart + sizeof(TextCommand));
					const size_t textLen = cmd->textLength;
					const LoadedFont *fontDesc = cmd->font;
					const TextureHandle *texture = cmd->texture;
					if(fontDesc != nullptr && fontDesc->charCount > 0 && texture != nullptr) {
						const float maxHeight = cmd->maxHeight;
						const float ax = cmd->horizontalAlignment;
						const float ay = cmd->verticalAlignment;
						Vec2f textSize = GetTextSize(text, cmd->textLength, fontDesc, maxHeight);
						float xpos = cmd->position.x - textSize.w * 0.5f + (textSize.w * 0.5f * ax);
						float ypos = cmd->position.y - textSize.h * 0.5f + (textSize.h * 0.5f * ay);
						uint32_t lastChar = fontDesc->firstChar + (fontDesc->charCount - 1);

						GLuint texId = PointerToValue<GLuint>(*texture);

						glColor4fv(&cmd->color.m[0]);
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, texId);
						for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
							char at = text[textPos];
							char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
							float advance;
							if((uint32_t)at >= fontDesc->firstChar && (uint32_t)at <= lastChar) {
								uint32_t codePoint = at - fontDesc->firstChar;
								const FontGlyph *glyph = &fontDesc->glyphs[codePoint];
								Vec2f size = glyph->charSize * maxHeight;
								Vec2f offset = V2f(xpos, ypos);
								offset += glyph->offset * maxHeight;
								offset += V2f(size.x, -size.y) * 0.5f;

								float extW = size.w * 0.5f;
								float extH = size.h * 0.5f;

								glBegin(GL_QUADS);
								glTexCoord2f(glyph->uvMax.x, glyph->uvMax.y); glVertex2f(offset.x + extW, offset.y + extH);
								glTexCoord2f(glyph->uvMin.x, glyph->uvMax.y); glVertex2f(offset.x + -extW, offset.y + extH);
								glTexCoord2f(glyph->uvMin.x, glyph->uvMin.y); glVertex2f(offset.x + -extW, offset.y + -extH);
								glTexCoord2f(glyph->uvMax.x, glyph->uvMin.y); glVertex2f(offset.x + extW, offset.y + -extH);
								glEnd();

								advance = GetFontCharacterAdvance(fontDesc, at, atNext) * maxHeight;
							} else {
								advance = fontDesc->info.spaceAdvance * maxHeight;
							}
							xpos += advance;
						}
						glBindTexture(GL_TEXTURE_2D, 0);
						glDisable(GL_TEXTURE_2D);
					}
				} break;

				default:
					fplAssert(!"Invalid default case!");
			}
			mem += dataSize;
			size_t consumed = (size_t)(mem - startMem);
			remaining -= consumed;
		}
	}

#if 0
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glEnd();
#endif
}

#endif // FINAL_OPENGL_RENDER_IMPLEMENTATION