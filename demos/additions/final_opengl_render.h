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

#endif // FINAL_OPENGL_RENDER_H

#if defined(FINAL_OPENGL_RENDER_IMPLEMENTATION) && !defined(FINAL_OPENGL_RENDER_IMPLEMENTED)
#define FINAL_OPENGL_RENDER_IMPLEMENTED

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

#endif // FINAL_OPENGL_RENDER_IMPLEMENTATION