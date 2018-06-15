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
		float textWidth = GetTextWidth(text, textLen, fontDesc, maxCharHeight);
		float textHeight = 0;
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			char at = text[textPos];
			uint32_t codePoint = at - fontDesc->firstChar;
			if(codePoint >= 0 && codePoint < fontDesc->charCount) {
				const FontGlyph *glyph = &fontDesc->glyphs[codePoint];
				Vec2f size = V2f(glyph->charSize.x, glyph->charSize.y) * maxCharHeight;
				textHeight = FPL_MAX(textHeight, size.h);
			}
		}
		float xpos = x - textWidth * 0.5f + textWidth * 0.5f * sx;
		float ypos = y + textHeight * 0.5f * sy;
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			uint32_t codePoint = at - fontDesc->firstChar;
			float advance;
			if(codePoint >= 0 && codePoint < fontDesc->charCount) {
				const FontGlyph *glyph = &fontDesc->glyphs[codePoint];
				Vec2f offset = V2f(xpos, ypos);
				offset += V2f(glyph->charSize.x * glyph->alignPercentage.x, glyph->charSize.y * glyph->alignPercentage.y) * maxCharHeight;
				Vec2f size = V2f(glyph->charSize.x, glyph->charSize.y) * maxCharHeight;
				DrawSprite(fontTexture, size.x * 0.5f, size.y * 0.5f, glyph->uvMin.x, glyph->uvMin.y, glyph->uvMax.x, glyph->uvMax.y, offset.x, offset.y);
				uint32_t nextCodePoint = (atNext > 0) ? atNext - fontDesc->firstChar : 0;
				advance = GetFontCharacterAdvance(fontDesc, &codePoint, (atNext > 0) ? &nextCodePoint : nullptr) * maxCharHeight;
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