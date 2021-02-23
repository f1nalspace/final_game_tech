/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Input

Description:
	Application to visualize keyboard/mouse/gamepad input.
	Used for testing event and poll based input in FPL.

Requirements:
	- C++ Compiler
	- Final Platform Layer
	- Final Dynamic OpenGL
	- STB_image
	- STB_truetype

Author:
	Torsten Spaete

Changelog:
	## 2019-08-31
	- Use new field isActive from fplGamepadState now, which was introduced in the last commit
	- Draw mouse wheel delta for 500 msecs
	- Fixed euro key in german layout
	- Added text input
	- Toggle polling using F5

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-08-19:
	- Gamepad reading and drawing

	## 2018-08-14:
	- Draw mouse and buttons
	- Read mouse button states

	## 2018-08-13:
	- Initial creation of this description block

License:
	Copyright (c) 2017-2020 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include <wchar.h> // wcslen

// @BAD(final): CPP is such garbage!
// It cannot handle array index initializer such as [index] = value :-(
// So we need this nonsense just to initialize a static array -.-
template <typename TIndexType, typename TValueType, size_t valueCount>
class ArrayInitializer {
protected:
	TValueType a[valueCount];
public:
	ArrayInitializer() {
		fplMemoryClear(a, sizeof(TValueType) * fplArrayCount(a));
	}
	const TValueType& operator [] (TIndexType eindex) const {
		return a[(int)eindex];
	}
	TValueType& operator [] (TIndexType eindex) {
		return a[(int)eindex];
	}
	void Set(TIndexType e, const TValueType& value) {
		a[(int)e] = value;
	}
};

static char ToLowerCase(char ch) {
	if (ch >= 'A' && ch <= 'Z') {
		ch = 'a' + (ch - 'A');
	}
	return ch;
}

static int CompareStringIgnoreCase(const char* a, const char* b) {
	while (true) {
		if (!*a && !*b) {
			break;
		} else if (!*a || !*b) {
			return -1;
		}
		char ca = ToLowerCase(*a);
		char cb = ToLowerCase(*b);
		if (ca < cb || ca > cb) {
			return (int)ca - (int)cb;
		}
		++a;
		++b;
	}
	return(0);
}

static size_t GetWideStringLength(const wchar_t* text) {
	size_t result = wcslen(text);
	return(result);
}

inline void CopyWideString(const wchar_t* source, const size_t len, wchar_t* target, const size_t maxTargetLen) {
	size_t reqLen = len + 1;
	fplAssert(maxTargetLen >= reqLen);
	fplMemoryCopy(source, reqLen * sizeof(wchar_t), target);
}

union Vec2f {
	struct {
		float x;
		float y;
	};
	struct {
		float w;
		float h;
	};
	struct {
		float u;
		float v;
	};
	float m[2];
};

inline Vec2f V2f(const float x = 0.0f, const float y = 0.0f) {
	Vec2f result = { x, y };
	return(result);
}

union Vec2i {
	struct {
		int x;
		int y;
	};
	struct {
		int w;
		int h;
	};
	int m[2];
};

inline Vec2i V2i(const int x, const int y) {
	Vec2i result = { x, y };
	return(result);
}

inline Vec2f operator*(const Vec2f& a, float b) {
	Vec2f result = V2f(a.x * b, a.y * b);
	return(result);
}
inline Vec2f operator+(const Vec2f& a, const Vec2f& b) {
	Vec2f result = V2f(a.x + b.x, a.y + b.y);
	return(result);
}
inline Vec2f& operator+=(Vec2f& a, const Vec2f& b) {
	a = b + a;
	return(a);
}

struct Viewport {
	int x;
	int y;
	int w;
	int h;
};

static Viewport ComputeViewportByAspect(const Vec2i& screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2i(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2i(0, 0);
	if (targetHeight > screenSize.h) {
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

struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
};

inline UVRect UVRectFromPos(const Vec2i& imageSize, const Vec2i& partSize, const Vec2i& pos) {
	Vec2f texel = V2f(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
	return(result);
}

typedef struct FontGlyph {
	Vec2f offset;
	Vec2f uvMin;
	Vec2f uvMax;
	Vec2f charSize;
	uint32_t charCode;
} FontGlyph;

struct FontData {
	uint8_t* atlasAlphaBitmap;
	FontGlyph* glyphs;
	float ascent;
	float descent;
	float lineHeight;
	float spaceAdvance;
	float* defaultAdvance;
	float* kerningTable;
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint32_t firstChar;
	uint32_t charCount;
	bool hasKerningTable;
};

inline float GetFontAscent(const FontData* font) {
	float result = font->ascent;
	return(result);
}

inline float GetFontDescent(const FontData* font) {
	float result = font->descent;
	return(result);
}

inline float GetFontLineAdvance(const FontData* font) {
	float result = font->lineHeight;
	return(result);
}

static float GetFontCharacterAdvance(const FontData* font, const uint32_t thisCodePoint) {
	float result = 0;
	if (thisCodePoint >= font->firstChar && thisCodePoint < (font->firstChar - font->charCount)) {
		uint32_t thisIndex = thisCodePoint - font->firstChar;
		const FontGlyph* glyph = font->glyphs + thisIndex;
		result = font->defaultAdvance[thisIndex];
	}
	return(result);
}

static int GetFontAtlasIndexFromCodePoint(const size_t fontCount, const FontData fonts[], const uint32_t codePoint) {
	for (size_t i = 0; i < fontCount; ++i) {
		uint32_t lastChar = fonts[i].firstChar + (fonts[i].charCount - 1);
		if (codePoint >= fonts[i].firstChar && codePoint <= lastChar) {
			return (int)i;
		}
	}
	return -1;
}

static Vec2f GetTextSize(const wchar_t* text, const size_t textLen, const size_t fontCount, const FontData fonts[], const float maxCharHeight) {
	float xwidth = 0.0f;
	float ymax = 0.0f;
	if (fontCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		for (uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];

			int atlasIndex = GetFontAtlasIndexFromCodePoint(fontCount, fonts, at);
			const FontData* font;
			if (atlasIndex > -1) {
				font = &fonts[atlasIndex];
			} else {
				font = &fonts[0];
			}
            
            if (font->charCount == 0)
                continue;
            
            float xadvance;
            Vec2f offset = V2f(xpos, ypos);
            Vec2f size = V2f();
            uint32_t lastChar = font->firstChar + (font->charCount - 1);
            if (at >= font->firstChar && at <= lastChar) {
                uint32_t codePoint = at - font->firstChar;
                const FontGlyph* glyph = font->glyphs + codePoint;
                size = glyph->charSize;
                offset += glyph->offset;
                offset += V2f(size.x, -size.y) * 0.5f;
                xadvance = GetFontCharacterAdvance(font, (uint32_t)at);
            } else {
                xadvance = fonts[0].spaceAdvance;
            }
            Vec2f min = offset;
            Vec2f max = min + V2f(xadvance, size.y);
            xwidth += (max.x - min.x);
            ymax = fplMax(ymax, max.y - min.y);
            xpos += xadvance;
		}
	}
	Vec2f result = V2f(xwidth, ymax) * maxCharHeight;
	return(result);
}

static bool LoadFontFromMemory(const void* data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, FontData* outFont) {
	if (data == fpl_null || dataSize == 0) {
		return false;
	}
	if (outFont == fpl_null) {
		return false;
	}

	fplClearStruct(outFont);

	stbtt_fontinfo fontInfo = fplZeroInit;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char*)data, fontIndex);

	bool result = false;
	if (stbtt_InitFont(&fontInfo, (const unsigned char*)data, fontOffset)) {
		uint32_t charCount = (lastChar - firstChar) + 1;
		uint8_t* atlasAlphaBitmap = (uint8_t*)fplMemoryAllocate(atlasWidth * atlasHeight);
		stbtt_bakedchar* packedChars = (stbtt_bakedchar*)fplMemoryAllocate(charCount * sizeof(stbtt_bakedchar));
		stbtt_BakeFontBitmap((const unsigned char*)data, fontOffset, fontSize, atlasAlphaBitmap, atlasWidth, atlasHeight, firstChar, charCount, packedChars);

		// Get metrics
		int ascentRaw, descentRaw, lineGapRaw;
		int spaceAdvanceRaw, spaceLeftSideBearing;
		stbtt_GetFontVMetrics(&fontInfo, &ascentRaw, &descentRaw, &lineGapRaw);
		stbtt_GetCodepointHMetrics(&fontInfo, ' ', &spaceAdvanceRaw, &spaceLeftSideBearing);

		// Calculate scales
		float texelU = 1.0f / (float)atlasWidth;
		float texelV = 1.0f / (float)atlasHeight;
		float pixelsToUnits = 1.0f / fontSize;

		// Space advance in pixels
		float spaceAdvancePx = spaceAdvanceRaw * pixelsToUnits;

		// Ascent height from the baseline in pixels
		float ascentPx = fabsf((float)ascentRaw) * pixelsToUnits;

		// Descent height from the baseline in pixels
		float descentPx = fabsf((float)descentRaw) * pixelsToUnits;

		// Max height is always ascent + descent
		float heightPx = ascentPx + descentPx;

		// Calculate line height
		float lineGapPx = lineGapRaw * pixelsToUnits;
		float lineHeightPx = ascentPx + descentPx + lineGapPx;

		size_t glyphsSize = sizeof(FontGlyph) * charCount;
		FontGlyph* glyphs = (FontGlyph*)fplMemoryAllocate(glyphsSize);

		for (uint32_t glyphIndex = 0; glyphIndex < charCount; ++glyphIndex) {
			stbtt_bakedchar* sourceInfo = packedChars + glyphIndex;

			FontGlyph* destInfo = glyphs + glyphIndex;
			destInfo->charCode = firstChar + glyphIndex;

			// Compute UV coords
			float uMin = sourceInfo->x0 * texelU;
			float uMax = sourceInfo->x1 * texelU;
			float vMin = sourceInfo->y1 * texelV;
			float vMax = sourceInfo->y0 * texelV;
			destInfo->uvMin = V2f(uMin, vMin);
			destInfo->uvMax = V2f(uMax, vMax);

			// Compute character size
			int charWidthInPixels = sourceInfo->x1 - sourceInfo->x0;
			int charHeightInPixels = sourceInfo->y1 - sourceInfo->y0;
			destInfo->charSize = V2f((float)charWidthInPixels, (float)charHeightInPixels) * pixelsToUnits;

			// Compute offset to start/baseline in units
			destInfo->offset = V2f(sourceInfo->xoff, -sourceInfo->yoff) * pixelsToUnits;
		}

		// Build kerning table & default advance table
		size_t kerningTableSize;
		float* kerningTable;
		if (loadKerning) {
			kerningTableSize = sizeof(float) * charCount * charCount;
			kerningTable = (float*)fplMemoryAllocate(kerningTableSize);
		} else {
			kerningTableSize = 0;
			kerningTable = fpl_null;
		}

		size_t defaultAdvanceSize = charCount * sizeof(float);
		float* defaultAdvance = (float*)fplMemoryAllocate(defaultAdvanceSize);
		for (uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
			uint32_t codePointIndex = (uint32_t)(charIndex - firstChar);
			stbtt_bakedchar* leftInfo = packedChars + codePointIndex;
			defaultAdvance[codePointIndex] = leftInfo->xadvance * pixelsToUnits;

			if (loadKerning) {
				for (uint32_t nextCharIndex = charIndex + 1; nextCharIndex < lastChar; ++nextCharIndex) {
					float kerningPx = stbtt_GetCodepointKernAdvance(&fontInfo, charIndex, nextCharIndex) * pixelsToUnits;
					if (kerningPx != 0) {
						int widthPx = leftInfo->x1 - leftInfo->x0;
						if (widthPx > 0) {
							float kerning = kerningPx / (float)widthPx;
							uint32_t a = (uint32_t)(charIndex - firstChar);
							uint32_t b = (uint32_t)(nextCharIndex - firstChar);
							kerningTable[a * charCount + b] = kerning;
						}
					}
				}
			}
		}

		outFont->firstChar = firstChar;
		outFont->charCount = charCount;
		outFont->ascent = ascentPx * pixelsToUnits;
		outFont->descent = descentPx * pixelsToUnits;
		outFont->lineHeight = lineHeightPx * pixelsToUnits;
		outFont->spaceAdvance = spaceAdvancePx * pixelsToUnits;
		outFont->glyphs = glyphs;
		outFont->kerningTable = kerningTable;
		outFont->hasKerningTable = loadKerning;
		outFont->defaultAdvance = defaultAdvance;
		outFont->atlasAlphaBitmap = atlasAlphaBitmap;
		outFont->atlasWidth = atlasWidth;
		outFont->atlasHeight = atlasHeight;

		result = true;
	}

	return(result);
}

static bool LoadFontFromFile(const char* dataPath, const char* filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, FontData* outFont) {
	if (filename == fpl_null) {
		return false;
	}
	if (outFont == fpl_null) {
		return false;
	}

	char filePath[1024];
	if (dataPath != fpl_null) {
		fplCopyString(dataPath, filePath, fplArrayCount(filePath));
		fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);
	} else {
		fplCopyString(filename, filePath, fplArrayCount(filePath));
	}

	bool result = false;

	fplFileHandle file;
	uint8_t* ttfBuffer = fpl_null;
	uint32_t ttfBufferSize = 0;
	if (fplOpenBinaryFile(filePath, &file)) {
		ttfBufferSize = fplGetFileSizeFromHandle32(&file);
		ttfBuffer = (uint8_t*)fplMemoryAllocate(ttfBufferSize);
		fplReadFileBlock32(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
		fplCloseFile(&file);
	}

	if (ttfBuffer != nullptr) {
		result = LoadFontFromMemory(ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, outFont);
		fplMemoryFree(ttfBuffer);
	}
	return(result);
}

static void ReleaseFont(FontData* font) {
	if (font != fpl_null) {
		if (font->hasKerningTable) {
			fplMemoryFree(font->kerningTable);
		}
		fplMemoryFree(font->glyphs);
		fplMemoryFree(font->atlasAlphaBitmap);
		fplClearStruct(font);
	}
}

static void DrawSprite(const GLuint texId, const float rx, const float ry, const float uMin, const float vMin, const float uMax, const float vMax, const float xoffset, const float yoffset) {
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

static void DrawSprite(const GLuint texId, const float rx, const float ry, const UVRect& uv, const float xoffset, const float yoffset) {
	DrawSprite(texId, rx, ry, uv.uMin, uv.vMax, uv.uMax, uv.vMin, xoffset, yoffset);
}

static void DrawRect(const float rx, const float ry, const float xoffset, const float yoffset, bool isFilled) {
	glBegin(isFilled ? GL_QUADS : GL_LINE_LOOP);
	glVertex2f(xoffset + rx, yoffset + ry);
	glVertex2f(xoffset - rx, yoffset + ry);
	glVertex2f(xoffset - rx, yoffset - ry);
	glVertex2f(xoffset + rx, yoffset - ry);
	glEnd();
}

static void DrawLine(const float x0, const float y0, const float x1, const float y1, const float lineWidth) {
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glEnd();
	glLineWidth(1.0f);
}

static void DrawArrow(const float x0, const float y0, const float x1, const float y1, const float arrowWidth, const float arrowDepth, const Vec2f& dir, const float lineWidth) {
	Vec2f al = V2f(-dir.y, dir.x) * arrowWidth * 0.5f;
	Vec2f ar = V2f(-dir.y, dir.x) * -arrowWidth * 0.5f;
	Vec2f b = dir * -arrowDepth;
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x1, y1);
	glVertex2f(x1 + al.x + b.x, y1 + al.y + b.y);
	glVertex2f(x1, y1);
	glVertex2f(x1 + ar.x + b.x, y1 + ar.y + b.y);
	glEnd();
	glLineWidth(1.0f);
}

static void DrawTextFont(const wchar_t* text, const size_t textLen, const size_t fontCount, const FontData fonts[], const GLuint textures[], const float x, const float y, const float maxCharHeight, const float sx, const float sy) {
	if (fontCount > 0) {
		Vec2f textSize = GetTextSize(text, textLen, fontCount, fonts, maxCharHeight);
		float xpos = x - textSize.x * 0.5f + (textSize.x * 0.5f * sx);
		float ypos = y - textSize.y * 0.5f + (textSize.y * 0.5f * sy);
		for (uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];
			int atlasIndex = GetFontAtlasIndexFromCodePoint(fontCount, fonts, at);
			const FontData* font;
			GLuint texture;
			if (atlasIndex > -1) {
				font = &fonts[atlasIndex];
				texture = textures[atlasIndex];
			} else {
				font = &fonts[0];
				texture = textures[0];
			}
            if (font->charCount == 0) 
                continue;
			uint32_t lastChar = font->firstChar + (font->charCount - 1);
			float advance;
			if ((uint32_t)at >= font->firstChar && (uint32_t)at <= lastChar) {
				uint32_t codePoint = at - font->firstChar;
				const FontGlyph* glyph = &font->glyphs[codePoint];
				Vec2f size = glyph->charSize * maxCharHeight;
				Vec2f offset = V2f(xpos, ypos);
				offset += glyph->offset * maxCharHeight;
				offset += V2f(size.x, -size.y) * 0.5f;
				DrawSprite(texture, size.x * 0.5f, size.y * 0.5f, glyph->uvMin.x, glyph->uvMin.y, glyph->uvMax.x, glyph->uvMax.y, offset.x, offset.y);
				advance = GetFontCharacterAdvance(font, at) * maxCharHeight;
			} else {
				advance = fonts[0].spaceAdvance * maxCharHeight;
			}
			xpos += advance;
		}
	}
}

static void DrawTextFont(const wchar_t* text, const size_t fontCount, const FontData fonts[], const GLuint textures[], const float x, const float y, const float maxCharHeight, const float sx, const float sy) {
	size_t textLen = GetWideStringLength(text);
	DrawTextFont(text, textLen, fontCount, fonts, textures, x, y, maxCharHeight, sx, sy);
}

static GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void* data, const bool repeatable, const GLint filter, const bool isAlphaOnly) {
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint internalFormat = isAlphaOnly ? GL_ALPHA8 : GL_RGBA8;
	GLenum format = isAlphaOnly ? GL_ALPHA : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeatable ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeatable ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	return(handle);
}

static GLuint LoadTexture(const char* dataPath, const char* filename) {
	GLuint result = 0;

	char filePath[FPL_MAX_PATH_LENGTH];
	fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);

	fplFileHandle file = {};
	if (fplOpenBinaryFile(filePath, &file)) {
		uint32_t dataSize = fplGetFileSizeFromHandle32(&file);
		uint8_t* data = (uint8_t*)fplMemoryAllocate(dataSize);
		fplReadFileBlock32(&file, dataSize, data, dataSize);
		int w, h, components;
		stbi_set_flip_vertically_on_load(0);
		uint8_t* pixels = stbi_load_from_memory(data, (int)dataSize, &w, &h, &components, 4);
		if (pixels != nullptr) {
			result = AllocateTexture(w, h, pixels, false, GL_LINEAR, false);
			stbi_image_free(pixels);
		}
		fplMemoryFree(data);
		fplCloseFile(&file);
	}
	return(result);
}

constexpr float AppAspect = 16.0f / 9.0f;
constexpr float AppWidth = 10.0f;
constexpr float AppHeight = AppWidth / AppAspect;

constexpr int KeyboardImageW = 2048;
constexpr int KeyboardImageH = 1024;
constexpr float KeyboardTexelW = 1.0f / (float)KeyboardImageW;
constexpr float KeyboardTexelH = 1.0f / (float)KeyboardImageH;
static const Vec2i KeyboardImageS = V2i(KeyboardImageW, KeyboardImageH);
static const Vec2i KeyboardSmallKeyS = V2i(68, 68);
static const Vec2i KeyboardLedS = V2i(11, 11);
constexpr float KeyboardAspect = KeyboardImageW / (float)KeyboardImageH;
constexpr float KeyboardW = AppWidth * 0.8f;
constexpr float KeyboardH = KeyboardW / KeyboardAspect;
static const Vec2f KeyboardSize = V2f(KeyboardW, KeyboardH);

constexpr int GamepadForegroundImageW = 2048;
constexpr int GamepadForegroundImageH = 1024;
static const Vec2i GamepadForegroundImageS = V2i(GamepadForegroundImageW, GamepadForegroundImageH);
static const Vec2i GamepadMaskImageS = V2i(1024, 1024);
constexpr float GamepadAspect = GamepadForegroundImageW / (float)GamepadForegroundImageH;
constexpr float GamepadW = AppWidth * 0.8f;
constexpr float GamepadH = GamepadW / GamepadAspect;
static const Vec2f GamepadSize = V2f(GamepadW, GamepadH);

constexpr int MouseImageW = 512;
constexpr int MouseImageH = 1024;
static const Vec2i MouseImageS = V2i(MouseImageW, MouseImageH);
constexpr float MouseAspect = MouseImageW / (float)MouseImageH;
constexpr float MouseW = AppWidth * 0.2f;
constexpr float MouseH = MouseW / MouseAspect;
static const Vec2f MouseSize = V2f(MouseW, MouseH);

struct KeyCharDef {
	wchar_t text[16 + 1];
	Vec2f align;
};
inline KeyCharDef MakeKeyChar(const wchar_t* text = nullptr, const Vec2f& align = {}) {
	KeyCharDef result = {};
	result.align = align;
	size_t len = GetWideStringLength(text);
	CopyWideString(text, len, result.text, fplArrayCount(result.text));
	return(result);
}
inline KeyCharDef MakeKeyChar(const uint32_t codePoint, const Vec2f& align = {}) {
	KeyCharDef result = {};
	result.align = align;
	fplAssert(codePoint > 0 && codePoint < 0x20000);
	result.text[0] = (wchar_t)codePoint;
	return(result);
}

struct KeyDef {
	KeyCharDef chars[4];
	UVRect uv;
	size_t count;
};

class KeyDefinitions : public ArrayInitializer<fplKey, KeyDef, 256> {
protected:
	void AddKeyDef(const fplKey index, const UVRect& uv, const int count, ...) {
		KeyDef def = {};
		def.uv = uv;
		va_list argList;
		va_start(argList, count);
		for (int i = 0; i < count; ++i) {
			KeyCharDef charDef = va_arg(argList, KeyCharDef);
			fplAssert(def.count < fplArrayCount(def.chars));
			def.chars[def.count++] = charDef;
		}
		va_end(argList);
		Set(index, def);
	}
public:
	const char* name;
	KeyDefinitions(const char* name) {
		this->name = name;
	}
};

class KeyDefinitionsDeDE : public KeyDefinitions {
public:
	KeyDefinitionsDeDE() : KeyDefinitions("de-DE") {
		AddKeyDef(fplKey_Escape, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(101, 286)), 1, MakeKeyChar(L"Esc"));
		AddKeyDef(fplKey_F1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(255, 286)), 1, MakeKeyChar(L"F1"));
		AddKeyDef(fplKey_F2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(327, 286)), 1, MakeKeyChar(L"F2"));
		AddKeyDef(fplKey_F3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(402, 286)), 1, MakeKeyChar(L"F3"));
		AddKeyDef(fplKey_F4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(478, 286)), 1, MakeKeyChar(L"F4"));
		AddKeyDef(fplKey_F5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(606, 286)), 1, MakeKeyChar(L"F5"));
		AddKeyDef(fplKey_F6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(678, 286)), 1, MakeKeyChar(L"F6"));
		AddKeyDef(fplKey_F7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(754, 286)), 1, MakeKeyChar(L"F7"));
		AddKeyDef(fplKey_F8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(829, 286)), 1, MakeKeyChar(L"F8"));
		AddKeyDef(fplKey_F9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(953, 286)), 1, MakeKeyChar(L"F9"));
		AddKeyDef(fplKey_F10, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1028, 286)), 1, MakeKeyChar(L"F10"));
		AddKeyDef(fplKey_F11, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1103, 286)), 1, MakeKeyChar(L"F11"));
		AddKeyDef(fplKey_F12, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1178, 286)), 1, MakeKeyChar(L"F12"));
		AddKeyDef(fplKey_Print, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 286)), 1, MakeKeyChar(L"Print"));
		AddKeyDef(fplKey_Scroll, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1388, 286)), 2, MakeKeyChar(L"Scroll", V2f(0.0f, 0.4f)), MakeKeyChar(L"Lock", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Pause, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1464, 286)), 1, MakeKeyChar(L"Pause"));

		AddKeyDef(fplKey_Oem5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(102, 382)), 2, MakeKeyChar(L"^", V2f(-0.5f, -0.4f)), MakeKeyChar(L"°", V2f(-0.5f, 0.3f)));
		AddKeyDef(fplKey_1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(192, 383)), 2, MakeKeyChar(L"1", V2f(-0.5f, -0.4f)), MakeKeyChar(L"!", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(268, 383)), 2, MakeKeyChar(L"2", V2f(-0.5f, -0.4f)), MakeKeyChar(L"\"", V2f(-0.5f, 0.3f)));
		AddKeyDef(fplKey_3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(343, 383)), 2, MakeKeyChar(L"3", V2f(-0.5f, -0.4f)), MakeKeyChar(L"§", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(419, 383)), 2, MakeKeyChar(L"4", V2f(-0.5f, -0.4f)), MakeKeyChar(L"$", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(494, 383)), 2, MakeKeyChar(L"5", V2f(-0.5f, -0.4f)), MakeKeyChar(L"%", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(569, 383)), 2, MakeKeyChar(L"6", V2f(-0.5f, -0.4f)), MakeKeyChar(L"&", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(645, 383)), 3, MakeKeyChar(L"7", V2f(-0.5f, -0.4f)), MakeKeyChar(L"/", V2f(-0.5f, 0.45f)), MakeKeyChar(L"{", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(722, 383)), 3, MakeKeyChar(L"8", V2f(-0.5f, -0.4f)), MakeKeyChar(L"(", V2f(-0.5f, 0.45f)), MakeKeyChar(L"[", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(797, 383)), 3, MakeKeyChar(L"9", V2f(-0.5f, -0.4f)), MakeKeyChar(L")", V2f(-0.5f, 0.45f)), MakeKeyChar(L"]", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_0, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(872, 383)), 3, MakeKeyChar(L"0", V2f(-0.5f, -0.4f)), MakeKeyChar(L"=", V2f(-0.5f, 0.45f)), MakeKeyChar(L"}", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_Oem4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(950, 381)), 3, MakeKeyChar(L"ß", V2f(-0.5f, -0.4f)), MakeKeyChar(L"?", V2f(-0.5f, 0.45f)), MakeKeyChar(L"\\", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_Oem6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1028, 381)), 2, MakeKeyChar(L"´", V2f(-0.5f, -0.75f)), MakeKeyChar(L"`", V2f(-0.5f, 0.2f)));
		AddKeyDef(fplKey_Backspace, UVRectFromPos(KeyboardImageS, V2i(139, 68), V2i(1105, 381)), 1, MakeKeyChar(L"Back"));

		Vec2f topLeftAlign = V2f(-0.25f, 0.25f);
		AddKeyDef(fplKey_Tab, UVRectFromPos(KeyboardImageS, V2i(87, 69), V2i(99, 466)), 1, MakeKeyChar(L"Tab"));
		AddKeyDef(fplKey_Q, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(195, 468)), 2, MakeKeyChar(L"Q", topLeftAlign), MakeKeyChar(L"@", V2f(0.4f, -0.4f)));
		AddKeyDef(fplKey_W, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(270, 468)), 1, MakeKeyChar(L"W", topLeftAlign));
		AddKeyDef(fplKey_E, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(345, 468)), 2, MakeKeyChar(L"E", topLeftAlign), MakeKeyChar(0x20AC, V2f(0.4f, -0.4f)));
		AddKeyDef(fplKey_R, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(421, 468)), 1, MakeKeyChar(L"R", topLeftAlign));
		AddKeyDef(fplKey_T, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(496, 468)), 1, MakeKeyChar(L"T", topLeftAlign));
		AddKeyDef(fplKey_Z, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(572, 468)), 1, MakeKeyChar(L"Z", topLeftAlign));
		AddKeyDef(fplKey_U, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(647, 468)), 1, MakeKeyChar(L"U", topLeftAlign));
		AddKeyDef(fplKey_I, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(724, 468)), 1, MakeKeyChar(L"I", topLeftAlign));
		AddKeyDef(fplKey_O, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(800, 468)), 1, MakeKeyChar(L"O", topLeftAlign));
		AddKeyDef(fplKey_P, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(875, 468)), 1, MakeKeyChar(L"P", topLeftAlign));
		AddKeyDef(fplKey_OemPlus, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1028, 467)), 3, MakeKeyChar(L"+", V2f(-0.25f, -0.35f)), MakeKeyChar(L"*", V2f(-0.25f, 0.35f)), MakeKeyChar(L"~", V2f(0.4f, -0.35f)));
		AddKeyDef(fplKey_Return, UVRectFromPos(KeyboardImageS, V2i(131, 152), V2i(1114, 465)), 1, MakeKeyChar(L"Return"));

		// [CapsLock]
		AddKeyDef(fplKey_A, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(208, 550)), 1, MakeKeyChar(L"A", topLeftAlign));
		AddKeyDef(fplKey_S, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(284, 550)), 1, MakeKeyChar(L"S", topLeftAlign));
		AddKeyDef(fplKey_D, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(359, 550)), 1, MakeKeyChar(L"D", topLeftAlign));
		AddKeyDef(fplKey_F, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(435, 550)), 1, MakeKeyChar(L"F", topLeftAlign));
		AddKeyDef(fplKey_G, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(510, 550)), 1, MakeKeyChar(L"G", topLeftAlign));
		AddKeyDef(fplKey_H, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(585, 550)), 1, MakeKeyChar(L"H", topLeftAlign));
		AddKeyDef(fplKey_J, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(661, 550)), 1, MakeKeyChar(L"J", topLeftAlign));
		AddKeyDef(fplKey_K, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(738, 550)), 1, MakeKeyChar(L"K", topLeftAlign));
		AddKeyDef(fplKey_L, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(813, 550)), 1, MakeKeyChar(L"L", topLeftAlign));

		// @TODO(final): fplKey_Oem1 ?????
		// @TODO(final): fplKey_Oem2 ?????
		// @TODO(final): fplKey_Oem3 ?????
		// @TODO(final): fplKey_Oem7 ?????
		// @TODO(final): fplKey_Oem8 ?????

		AddKeyDef(fplKey_LeftShift, UVRectFromPos(KeyboardImageS, V2i(87, 68), V2i(98, 633)), 1, MakeKeyChar(L"LShift"));
		AddKeyDef(fplKey_Y, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(195, 634)), 1, MakeKeyChar(L"Y", topLeftAlign));
		AddKeyDef(fplKey_X, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(271, 634)), 1, MakeKeyChar(L"X", topLeftAlign));
		AddKeyDef(fplKey_C, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(346, 634)), 1, MakeKeyChar(L"C", topLeftAlign));
		AddKeyDef(fplKey_V, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(422, 634)), 1, MakeKeyChar(L"V", topLeftAlign));
		AddKeyDef(fplKey_B, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(497, 634)), 1, MakeKeyChar(L"B", topLeftAlign));
		AddKeyDef(fplKey_N, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(572, 634)), 1, MakeKeyChar(L"N", topLeftAlign));
		AddKeyDef(fplKey_M, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(648, 634)), 1, MakeKeyChar(L"M", topLeftAlign));
		AddKeyDef(fplKey_OemComma, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(800, 633)), 2, MakeKeyChar(L",", V2f(-0.25f, -0.35f)), MakeKeyChar(L";", V2f(-0.25f, 0.4f)));
		AddKeyDef(fplKey_OemPeriod, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(875, 633)), 2, MakeKeyChar(L".", V2f(-0.25f, -0.35f)), MakeKeyChar(L":", V2f(-0.25f, 0.4f)));
		AddKeyDef(fplKey_OemMinus, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(954, 630)), 2, MakeKeyChar(L"-", V2f(-0.25f, -0.35f)), MakeKeyChar(L"_", V2f(-0.25f, 0.4f)));
		AddKeyDef(fplKey_RightShift, UVRectFromPos(KeyboardImageS, V2i(210, 68), V2i(1034, 629)), 1, MakeKeyChar(L"RShift"));

		// Controls
		AddKeyDef(fplKey_Insert, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 384)), 1, MakeKeyChar(L"Ins"));
		AddKeyDef(fplKey_Home, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1390, 384)), 1, MakeKeyChar(L"Home"));
		AddKeyDef(fplKey_PageUp, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1465, 384)), 2, MakeKeyChar(L"Page", V2f(0.0f, 0.4f)), MakeKeyChar(L"↑", V2f(0.0f, -0.3f)));
		AddKeyDef(fplKey_Delete, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 466)), 1, MakeKeyChar(L"Del"));
		AddKeyDef(fplKey_End, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1390, 466)), 1, MakeKeyChar(L"End"));
		AddKeyDef(fplKey_PageDown, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1464, 466)), 2, MakeKeyChar(L"Page", V2f(0.0f, 0.4f)), MakeKeyChar(L"↓", V2f(0.0f, -0.3f)));

		// Numpad
		AddKeyDef(fplKey_NumLock, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 380)), 1, MakeKeyChar(L"Num"));
		AddKeyDef(fplKey_Divide, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 380)), 1, MakeKeyChar(L"/"));
		AddKeyDef(fplKey_Multiply, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 380)), 1, MakeKeyChar(L"*"));
		AddKeyDef(fplKey_Substract, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1835, 380)), 1, MakeKeyChar(L"-"));
		AddKeyDef(fplKey_NumPad7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 465)), 2, MakeKeyChar(L"7", V2f(0.0f, 0.3f)), MakeKeyChar(L"Home", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 465)), 2, MakeKeyChar(L"8", V2f(0.0f, 0.3f)), MakeKeyChar(L"↑", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 465)), 2, MakeKeyChar(L"9", V2f(0.0f, 0.3f)), MakeKeyChar(L"Pg ↑", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Add, UVRectFromPos(KeyboardImageS, V2i(68, 146), V2i(1835, 467)), 1, MakeKeyChar(L"+"));
		AddKeyDef(fplKey_NumPad4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 548)), 2, MakeKeyChar(L"4", V2f(0.0f, 0.3f)), MakeKeyChar(L"←", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1683, 548)), 1, MakeKeyChar(L"5"));
		AddKeyDef(fplKey_NumPad6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 548)), 2, MakeKeyChar(L"6", V2f(0.0f, 0.3f)), MakeKeyChar(L"→", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 631)), 2, MakeKeyChar(L"1", V2f(0.0f, 0.3f)), MakeKeyChar(L"End", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 631)), 2, MakeKeyChar(L"2", V2f(0.0f, 0.3f)), MakeKeyChar(L"↓", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 631)), 2, MakeKeyChar(L"3", V2f(0.0f, 0.3f)), MakeKeyChar(L"Pg ↓", V2f(0.0f, -0.4f)));
		// [Enter]
		AddKeyDef(fplKey_NumPad0, UVRectFromPos(KeyboardImageS, V2i(138, 68), V2i(1610, 709)), 2, MakeKeyChar(L"0", V2f(0.0f, 0.3f)), MakeKeyChar(L"Ins", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Separator, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 709)), 2, MakeKeyChar(L",", V2f(0.0f, 0.3f)), MakeKeyChar(L"Del", V2f(0.0f, -0.4f)));

		AddKeyDef(fplKey_Up, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1392, 631)), 1, MakeKeyChar(L"↑"));
		AddKeyDef(fplKey_Left, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1316, 709)), 1, MakeKeyChar(L"←"));
		AddKeyDef(fplKey_Down, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1392, 709)), 1, MakeKeyChar(L"↓"));
		AddKeyDef(fplKey_Right, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1467, 709)), 1, MakeKeyChar(L"→"));

		AddKeyDef(fplKey_LeftControl, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(101, 716)), 1, MakeKeyChar(L"LCtrl"));
		AddKeyDef(fplKey_LeftSuper, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(210, 716)), 1, MakeKeyChar(L"LWin"));
		AddKeyDef(fplKey_LeftAlt, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(319, 716)), 1, MakeKeyChar(L"LAlt"));
		AddKeyDef(fplKey_Space, UVRectFromPos(KeyboardImageS, V2i(373, 68), V2i(434, 715)), 1, MakeKeyChar(L"Space"));
		AddKeyDef(fplKey_RightAlt, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(824, 717)), 1, MakeKeyChar(L"RAlt"));
		AddKeyDef(fplKey_RightSuper, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(933, 717)), 1, MakeKeyChar(L"RWin"));
		// [Meta]
		AddKeyDef(fplKey_RightControl, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(1146, 717)), 1, MakeKeyChar(L"RCtrl"));
	}
};

class KeyDefinitionsEnUS : public KeyDefinitions {
public:
	KeyDefinitionsEnUS() : KeyDefinitions("en-US") {
		AddKeyDef(fplKey_Escape, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(101, 286)), 1, MakeKeyChar(L"Esc"));
		AddKeyDef(fplKey_F1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(255, 286)), 1, MakeKeyChar(L"F1"));
		AddKeyDef(fplKey_F2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(327, 286)), 1, MakeKeyChar(L"F2"));
		AddKeyDef(fplKey_F3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(402, 286)), 1, MakeKeyChar(L"F3"));
		AddKeyDef(fplKey_F4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(478, 286)), 1, MakeKeyChar(L"F4"));
		AddKeyDef(fplKey_F5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(606, 286)), 1, MakeKeyChar(L"F5"));
		AddKeyDef(fplKey_F6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(678, 286)), 1, MakeKeyChar(L"F6"));
		AddKeyDef(fplKey_F7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(754, 286)), 1, MakeKeyChar(L"F7"));
		AddKeyDef(fplKey_F8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(829, 286)), 1, MakeKeyChar(L"F8"));
		AddKeyDef(fplKey_F9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(953, 286)), 1, MakeKeyChar(L"F9"));
		AddKeyDef(fplKey_F10, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1028, 286)), 1, MakeKeyChar(L"F10"));
		AddKeyDef(fplKey_F11, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1103, 286)), 1, MakeKeyChar(L"F11"));
		AddKeyDef(fplKey_F12, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1178, 286)), 1, MakeKeyChar(L"F12"));
		AddKeyDef(fplKey_Print, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 286)), 1, MakeKeyChar(L"Print"));
		AddKeyDef(fplKey_Scroll, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1388, 286)), 2, MakeKeyChar(L"Scroll", V2f(0.0f, 0.4f)), MakeKeyChar(L"Lock", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Pause, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1464, 286)), 1, MakeKeyChar(L"Pause"));

		// @TODO(final): fplKey_Oem3 `~
		AddKeyDef(fplKey_1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(192, 383)), 2, MakeKeyChar(L"1", V2f(-0.5f, -0.4f)), MakeKeyChar(L"!", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(268, 383)), 2, MakeKeyChar(L"2", V2f(-0.5f, -0.4f)), MakeKeyChar(L"@", V2f(-0.5f, 0.3f)));
		AddKeyDef(fplKey_3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(343, 383)), 2, MakeKeyChar(L"3", V2f(-0.5f, -0.4f)), MakeKeyChar(L"#", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(419, 383)), 2, MakeKeyChar(L"4", V2f(-0.5f, -0.4f)), MakeKeyChar(L"$", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(494, 383)), 2, MakeKeyChar(L"5", V2f(-0.5f, -0.4f)), MakeKeyChar(L"%", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(569, 383)), 2, MakeKeyChar(L"6", V2f(-0.5f, -0.4f)), MakeKeyChar(L"^", V2f(-0.5f, 0.45f)));
		AddKeyDef(fplKey_7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(645, 383)), 3, MakeKeyChar(L"7", V2f(-0.5f, -0.4f)), MakeKeyChar(L"&", V2f(-0.5f, 0.45f)), MakeKeyChar(L"{", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(722, 383)), 3, MakeKeyChar(L"8", V2f(-0.5f, -0.4f)), MakeKeyChar(L"*", V2f(-0.5f, 0.45f)), MakeKeyChar(L"[", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(797, 383)), 3, MakeKeyChar(L"9", V2f(-0.5f, -0.4f)), MakeKeyChar(L"(", V2f(-0.5f, 0.45f)), MakeKeyChar(L"]", V2f(0.5f, -0.3f)));
		AddKeyDef(fplKey_0, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(872, 383)), 3, MakeKeyChar(L"0", V2f(-0.5f, -0.4f)), MakeKeyChar(L")", V2f(-0.5f, 0.45f)), MakeKeyChar(L"}", V2f(0.5f, -0.3f)));
		// @TODO(final): fplKey_OemMinus -*
		// @TODO(final): fplKey_OemPlus +=
		AddKeyDef(fplKey_Backspace, UVRectFromPos(KeyboardImageS, V2i(139, 68), V2i(1105, 381)), 1, MakeKeyChar(L"Back"));

		AddKeyDef(fplKey_Tab, UVRectFromPos(KeyboardImageS, V2i(87, 69), V2i(99, 466)), 1, MakeKeyChar(L"Tab"));
		AddKeyDef(fplKey_Q, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(195, 468)), 1, MakeKeyChar(L"Q"));
		AddKeyDef(fplKey_W, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(270, 468)), 1, MakeKeyChar(L"W"));
		AddKeyDef(fplKey_E, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(345, 468)), 1, MakeKeyChar(L"E"));
		AddKeyDef(fplKey_R, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(421, 468)), 1, MakeKeyChar(L"R"));
		AddKeyDef(fplKey_T, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(496, 468)), 1, MakeKeyChar(L"T"));
		AddKeyDef(fplKey_Y, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(572, 468)), 1, MakeKeyChar(L"Y"));
		AddKeyDef(fplKey_U, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(647, 468)), 1, MakeKeyChar(L"U"));
		AddKeyDef(fplKey_I, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(724, 468)), 1, MakeKeyChar(L"I"));
		AddKeyDef(fplKey_O, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(800, 468)), 1, MakeKeyChar(L"O"));
		AddKeyDef(fplKey_P, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(875, 468)), 1, MakeKeyChar(L"P"));
		// @TODO(final): fplKey_Oem4 [{
		// @TODO(final): fplKey_Oem6 ]}
		// @TODO(final): fplKey_Oem5 \|
		AddKeyDef(fplKey_Return, UVRectFromPos(KeyboardImageS, V2i(131, 152), V2i(1114, 465)), 1, MakeKeyChar(L"Return"));

		// [CapsLock]
		AddKeyDef(fplKey_A, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(208, 550)), 1, MakeKeyChar(L"A"));
		AddKeyDef(fplKey_S, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(284, 550)), 1, MakeKeyChar(L"S"));
		AddKeyDef(fplKey_D, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(359, 550)), 1, MakeKeyChar(L"D"));
		AddKeyDef(fplKey_F, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(435, 550)), 1, MakeKeyChar(L"F"));
		AddKeyDef(fplKey_G, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(510, 550)), 1, MakeKeyChar(L"G"));
		AddKeyDef(fplKey_H, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(585, 550)), 1, MakeKeyChar(L"H"));
		AddKeyDef(fplKey_J, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(661, 550)), 1, MakeKeyChar(L"J"));
		AddKeyDef(fplKey_K, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(738, 550)), 1, MakeKeyChar(L"K"));
		AddKeyDef(fplKey_L, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(813, 550)), 1, MakeKeyChar(L"L"));
		// @TODO(final): fplKey_Oem1 ;:
		// @TODO(final): fplKey_Oem7 '"

		AddKeyDef(fplKey_LeftShift, UVRectFromPos(KeyboardImageS, V2i(87, 68), V2i(98, 633)), 1, MakeKeyChar(L"LShift"));
		// [<>|]
		AddKeyDef(fplKey_Z, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(195, 634)), 1, MakeKeyChar(L"Z"));
		AddKeyDef(fplKey_X, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(271, 634)), 1, MakeKeyChar(L"X"));
		AddKeyDef(fplKey_C, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(346, 634)), 1, MakeKeyChar(L"C"));
		AddKeyDef(fplKey_V, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(422, 634)), 1, MakeKeyChar(L"V"));
		AddKeyDef(fplKey_B, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(497, 634)), 1, MakeKeyChar(L"B"));
		AddKeyDef(fplKey_N, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(572, 634)), 1, MakeKeyChar(L"N"));
		AddKeyDef(fplKey_M, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(648, 634)), 1, MakeKeyChar(L"M"));
		// @TODO(final): fplKey_OemComma ,<
		// @TODO(final): fplKey_OemPeriod .>
		// @TODO(final): fplKey_Oem2 /?
		AddKeyDef(fplKey_RightShift, UVRectFromPos(KeyboardImageS, V2i(210, 68), V2i(1034, 629)), 1, MakeKeyChar(L"RShift"));

		// Controls
		AddKeyDef(fplKey_Insert, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 384)), 1, MakeKeyChar(L"Ins"));
		AddKeyDef(fplKey_Home, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1390, 384)), 1, MakeKeyChar(L"Home"));
		AddKeyDef(fplKey_PageUp, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1465, 384)), 2, MakeKeyChar(L"Page", V2f(0.0f, 0.4f)), MakeKeyChar(L"↑", V2f(0.0f, -0.3f)));
		AddKeyDef(fplKey_Delete, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1313, 466)), 1, MakeKeyChar(L"Del"));
		AddKeyDef(fplKey_End, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1390, 466)), 1, MakeKeyChar(L"End"));
		AddKeyDef(fplKey_PageDown, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1464, 466)), 2, MakeKeyChar(L"Page", V2f(0.0f, 0.4f)), MakeKeyChar(L"↓", V2f(0.0f, -0.3f)));

		// Numpad
		AddKeyDef(fplKey_NumLock, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 380)), 1, MakeKeyChar(L"Num"));
		AddKeyDef(fplKey_Divide, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 380)), 1, MakeKeyChar(L"/"));
		AddKeyDef(fplKey_Multiply, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 380)), 1, MakeKeyChar(L"*"));
		AddKeyDef(fplKey_Substract, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1835, 380)), 1, MakeKeyChar(L"-"));
		AddKeyDef(fplKey_NumPad7, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 465)), 2, MakeKeyChar(L"7", V2f(0.0f, 0.3f)), MakeKeyChar(L"Home", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad8, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 465)), 2, MakeKeyChar(L"8", V2f(0.0f, 0.3f)), MakeKeyChar(L"↑", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad9, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 465)), 2, MakeKeyChar(L"9", V2f(0.0f, 0.3f)), MakeKeyChar(L"Pg ↑", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Add, UVRectFromPos(KeyboardImageS, V2i(68, 146), V2i(1835, 467)), 1, MakeKeyChar(L"+"));
		AddKeyDef(fplKey_NumPad4, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 548)), 2, MakeKeyChar(L"4", V2f(0.0f, 0.3f)), MakeKeyChar(L"←", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad5, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1683, 548)), 1, MakeKeyChar(L"5"));
		AddKeyDef(fplKey_NumPad6, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 548)), 2, MakeKeyChar(L"6", V2f(0.0f, 0.3f)), MakeKeyChar(L"→", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad1, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1607, 631)), 2, MakeKeyChar(L"1", V2f(0.0f, 0.3f)), MakeKeyChar(L"End", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad2, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1682, 631)), 2, MakeKeyChar(L"2", V2f(0.0f, 0.3f)), MakeKeyChar(L"↓", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_NumPad3, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 631)), 2, MakeKeyChar(L"3", V2f(0.0f, 0.3f)), MakeKeyChar(L"Pg ↓", V2f(0.0f, -0.4f)));
		// [Enter]
		AddKeyDef(fplKey_NumPad0, UVRectFromPos(KeyboardImageS, V2i(138, 68), V2i(1610, 709)), 2, MakeKeyChar(L"0", V2f(0.0f, 0.3f)), MakeKeyChar(L"Ins", V2f(0.0f, -0.4f)));
		AddKeyDef(fplKey_Separator, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1758, 709)), 2, MakeKeyChar(L",", V2f(0.0f, 0.3f)), MakeKeyChar(L"Del", V2f(0.0f, -0.4f)));

		AddKeyDef(fplKey_Up, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1392, 631)), 1, MakeKeyChar(L"↑"));
		AddKeyDef(fplKey_Left, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1316, 709)), 1, MakeKeyChar(L"←"));
		AddKeyDef(fplKey_Down, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1392, 709)), 1, MakeKeyChar(L"↓"));
		AddKeyDef(fplKey_Right, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(1467, 709)), 1, MakeKeyChar(L"→"));

		AddKeyDef(fplKey_LeftControl, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(101, 716)), 1, MakeKeyChar(L"LCtrl"));
		AddKeyDef(fplKey_LeftSuper, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(210, 716)), 1, MakeKeyChar(L"LWin"));
		AddKeyDef(fplKey_LeftAlt, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(319, 716)), 1, MakeKeyChar(L"LAlt"));
		AddKeyDef(fplKey_Space, UVRectFromPos(KeyboardImageS, V2i(373, 68), V2i(434, 715)), 1, MakeKeyChar(L"Space"));
		AddKeyDef(fplKey_RightAlt, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(824, 717)), 1, MakeKeyChar(L"RAlt"));
		AddKeyDef(fplKey_RightSuper, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(933, 717)), 1, MakeKeyChar(L"RWin"));
		// [Meta]
		AddKeyDef(fplKey_RightControl, UVRectFromPos(KeyboardImageS, V2i(100, 68), V2i(1146, 717)), 1, MakeKeyChar(L"RCtrl"));
	}
};

static KeyDefinitions keyDefinitionsArray[] = {
	KeyDefinitionsDeDE(),
	KeyDefinitionsEnUS(),
};

struct KeyLedDef {
	fplKeyboardModifierFlags flag;
	KeyCharDef key;
	UVRect uv;
};

static KeyLedDef KeyLedDefinitions[] = {
	{fplKeyboardModifierFlags_CapsLock, MakeKeyChar(L"Caps", V2f(0.0f, 6.0f)), UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1686, 325))},
	{fplKeyboardModifierFlags_ScrollLock, MakeKeyChar(L"Scroll", V2f(0.0f, 6.0f)), UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1753, 325))},
	{fplKeyboardModifierFlags_NumLock, MakeKeyChar(L"Num", V2f(0.0f, 6.0f)), UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1820, 325))},
};

struct MouseButtonDef {
	fplMouseButtonType buttonType;
	UVRect uv;
};

static MouseButtonDef MouseButtonDefinitions[] = {
	{fplMouseButtonType_Left, UVRectFromPos(MouseImageS, V2i(139, 181), V2i(96, 332))},
	{fplMouseButtonType_Right, UVRectFromPos(MouseImageS, V2i(139, 181), V2i(277, 332))},
	{fplMouseButtonType_Middle, UVRectFromPos(MouseImageS, V2i(42, 102), V2i(235, 376))},
};

struct MouseWheelDef {
	int wheel;
	UVRect uv;
};

static MouseWheelDef MouseWheelDefinitions[] = {
	{+1, UVRectFromPos(MouseImageS, V2i(42, 32), V2i(235, 376))},
	{-1, UVRectFromPos(MouseImageS, V2i(42, 32), V2i(235, 446))},
};

struct GamepadButtonDef {
	fplGamepadButtonType button;
	UVRect foregroundUV;
	fpl_b32 useMask;
	UVRect maskUV;
};

static UVRect GamepadLeftStickUV = UVRectFromPos(GamepadForegroundImageS, V2i(258, 249), V2i(600, 756));
static UVRect GamepadRightStickUV = UVRectFromPos(GamepadForegroundImageS, V2i(258, 249), V2i(1200, 756));

static GamepadButtonDef GamepadButtonsDefinitions[] = {
	{fplGamepadButtonType_DPadUp, UVRectFromPos(GamepadForegroundImageS, V2i(115, 133), V2i(376, 437)), 1, UVRectFromPos(GamepadMaskImageS, V2i(115, 133), V2i(2, 2))},
	{fplGamepadButtonType_DPadRight, UVRectFromPos(GamepadForegroundImageS, V2i(127, 98), V2i(469, 537)), 1, UVRectFromPos(GamepadMaskImageS, V2i(127, 98), V2i(236, 2))},
	{fplGamepadButtonType_DPadDown, UVRectFromPos(GamepadForegroundImageS, V2i(115, 133), V2i(376, 603)), 1, UVRectFromPos(GamepadMaskImageS, V2i(115, 133), V2i(119, 2))},
	{fplGamepadButtonType_DPadLeft, UVRectFromPos(GamepadForegroundImageS, V2i(127, 98), V2i(262, 537)), 1, UVRectFromPos(GamepadMaskImageS, V2i(127, 98), V2i(365, 2))},

	{fplGamepadButtonType_ActionY, UVRectFromPos(GamepadForegroundImageS, V2i(137, 138), V2i(1554, 393)), 1, UVRectFromPos(GamepadMaskImageS, V2i(137, 138), V2i(2, 137))},
	{fplGamepadButtonType_ActionA, UVRectFromPos(GamepadForegroundImageS, V2i(137, 138), V2i(1554, 650)), 1, UVRectFromPos(GamepadMaskImageS, V2i(137, 138), V2i(141, 137))},
	{fplGamepadButtonType_ActionX, UVRectFromPos(GamepadForegroundImageS, V2i(138, 136), V2i(1393, 525)), 1, UVRectFromPos(GamepadMaskImageS, V2i(138, 136), V2i(280, 137))},
	{fplGamepadButtonType_ActionB, UVRectFromPos(GamepadForegroundImageS, V2i(138, 136), V2i(1715, 525)), 1, UVRectFromPos(GamepadMaskImageS, V2i(138, 136), V2i(420, 137))},

	{fplGamepadButtonType_Start, UVRectFromPos(GamepadForegroundImageS, V2i(115, 81), V2i(1149, 539)), 1, UVRectFromPos(GamepadMaskImageS, V2i(115, 81), V2i(613, 2))},
	{fplGamepadButtonType_Back, UVRectFromPos(GamepadForegroundImageS, V2i(117, 72), V2i(795, 544)), 1, UVRectFromPos(GamepadMaskImageS, V2i(117, 72), V2i(494, 2))},

	{fplGamepadButtonType_LeftShoulder, UVRectFromPos(GamepadForegroundImageS, V2i(238, 85), V2i(314, 67)), 1, UVRectFromPos(GamepadMaskImageS, V2i(238, 85), V2i(560, 85))},
	{fplGamepadButtonType_RightShoulder, UVRectFromPos(GamepadForegroundImageS, V2i(238, 85), V2i(1502, 67)), 1, UVRectFromPos(GamepadMaskImageS, V2i(238, 85), V2i(560, 172))},

	{fplGamepadButtonType_LeftThumb, GamepadLeftStickUV, 1, UVRectFromPos(GamepadMaskImageS, V2i(258, 249), V2i(2, 277))},
	{fplGamepadButtonType_RightThumb, GamepadRightStickUV, 1, UVRectFromPos(GamepadMaskImageS, V2i(258, 249), V2i(262, 277))},
};

static UVRect GamepadLeftTriggerUV = UVRectFromPos(GamepadForegroundImageS, V2i(217, 42), V2i(324, 0));
static UVRect GamepadRightTriggerUV = UVRectFromPos(GamepadForegroundImageS, V2i(217, 42), V2i(1513, 0));

struct SpritePosition {
	Vec2f pos;
	Vec2f size;
};

constexpr size_t CodePointCount = 10000;
constexpr size_t CodePointsPerAtlas = 256;
constexpr size_t FontCount = CodePointCount / CodePointsPerAtlas + 1;

enum class RenderMode {
	KeyboardAndMouse,
	Gamepad,
};

struct AppState {
	FontData letterFontData[FontCount];
	FontData osdFontData;
	FontData consoleFontData;

	GLuint letterFontTextures[FontCount];
	GLuint osdFontTexture;
	GLuint consoleFontTexture;

	GLuint keyboardTexture;
	GLuint gamepadForegroundTexture;
	GLuint gamepadMaskTexture;

	GLuint mouseTexture;
	Vec2f mousePos;

	RenderMode renderMode;

	fpl_b32 usePolling;
};

struct InputState {
	wchar_t *text;
	fplGamepadState gamepadState;
	fplButtonState keyStates[256];
	fplButtonState mouseStates[fplMouseButtonType_MaxCount];
	Vec2i mousePos;
	size_t textLen;
	size_t textCapacity;
	uint64_t lastMouseWheelUpdateTime;
	uint64_t lastTextInputTime;
	uint64_t lastTextCursorBlinkTime;
	fpl_b32 showTextCursor;
	float mouseWheelDelta;
	fplKeyboardModifierFlags ledStates;
};

static void InitApp(AppState* appState) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	char dataPath[FPL_MAX_PATH_LENGTH];
	fplGetExecutableFilePath(dataPath, fplArrayCount(dataPath));
	fplExtractFilePath(dataPath, dataPath, fplArrayCount(dataPath));
	fplPathCombine(dataPath, fplArrayCount(dataPath), 2, dataPath, "data");

	if (LoadFontFromFile(dataPath, "NotoSans-Regular.ttf", 0, 48.0f, 32, 255, 512, 512, false, &appState->osdFontData)) {
		appState->osdFontTexture = AllocateTexture(appState->osdFontData.atlasWidth, appState->osdFontData.atlasHeight, appState->osdFontData.atlasAlphaBitmap, false, GL_LINEAR, true);
	}

	if (LoadFontFromFile(dataPath, "VeraMono.ttf", 0, 48.0f, 32, 255, 512, 512, false, &appState->consoleFontData)) {
		appState->consoleFontTexture = AllocateTexture(appState->consoleFontData.atlasWidth, appState->consoleFontData.atlasHeight, appState->consoleFontData.atlasAlphaBitmap, false, GL_LINEAR, true);
	}

	float letterFontSize = 16.0f;
	int letterAtlasWidth = 512;
	int letterAtlasHeight = 512;
	for (int i = 0; i < FontCount; ++i) {
		int cpStart = i * CodePointsPerAtlas;
		int cpEnd = cpStart + (CodePointsPerAtlas - 1);
		if (LoadFontFromFile(dataPath, "NotoSans-Regular.ttf", 0, letterFontSize, cpStart, cpEnd, letterAtlasWidth, letterAtlasHeight, false, &appState->letterFontData[i])) {
			appState->letterFontTextures[i] = AllocateTexture(appState->letterFontData[i].atlasWidth, appState->letterFontData[i].atlasHeight, appState->letterFontData[i].atlasAlphaBitmap, false, GL_LINEAR, true);
		}
	}

	appState->keyboardTexture = LoadTexture(dataPath, "keyboard.png");
	appState->gamepadForegroundTexture = LoadTexture(dataPath, "gamepad.png");
	appState->gamepadMaskTexture = LoadTexture(dataPath, "gamepad_mask.png");
	appState->mouseTexture = LoadTexture(dataPath, "mouse.png");
	appState->usePolling = false;
	appState->renderMode = RenderMode::KeyboardAndMouse;
}

static void ReleaseApp(AppState* appState) {
	glDeleteTextures(1, &appState->gamepadMaskTexture);
	glDeleteTextures(1, &appState->gamepadForegroundTexture);
	glDeleteTextures(1, &appState->keyboardTexture);

	glDeleteTextures(FontCount, &appState->letterFontTextures[0]);
	for (int i = 0; i < FontCount; ++i) {
		ReleaseFont(&appState->letterFontData[i]);
	}

	glDeleteTextures(1, &appState->osdFontTexture);
	ReleaseFont(&appState->osdFontData);
}

static SpritePosition ComputeSpritePosition(const Vec2f& fullCenter, const Vec2f& fullSize, const UVRect& uv) {
	float w = fullSize.w * (uv.uMax - uv.uMin);
	float h = fullSize.h * (uv.vMax - uv.vMin);
	float ox = fullSize.w * (uv.uMin);
	float oy = fullSize.h * (1.0f - uv.vMax);
	float x = fullCenter.x - fullSize.w * 0.5f + ox + w * 0.5f;
	float y = fullCenter.y - fullSize.h * 0.5f + oy + h * 0.5f;
	SpritePosition result;
	result.pos = V2f(x, y);
	result.size = V2f(w, h);
	return(result);
}

struct RenderScale {
	Viewport viewport;
	Vec2f worldToScreen;
	Vec2f screenToWorld;
	Vec2f worldSize;
	uint32_t winWidth;
	uint32_t winHeight;
};

inline Vec2f ScreenToWorldSize(const RenderScale &scale, const Vec2i &screenSize) {
	Vec2f result = V2f(screenSize.x * scale.screenToWorld.x, screenSize.y * scale.screenToWorld.y);
	return(result);
}

inline Vec2f ScreenToWorldPos(const RenderScale &scale, const Vec2i &screenPos) {
	Vec2f result = V2f(-scale.worldSize.w * 0.5f + (screenPos.x - scale.viewport.x) * scale.screenToWorld.x, -scale.worldSize.h * 0.5f + (screenPos.y - scale.viewport.y) * scale.screenToWorld.y);
	return(result);
}

inline Vec2f PixelToWorldPos(const RenderScale &scale, const Vec2i &pixel) {
	int x = pixel.x - scale.winWidth / 2;
	int y = pixel.y - scale.winHeight / 2;
	Vec2f result = V2f(x * scale.screenToWorld.x, y * scale.screenToWorld.y);
	return(result);
}

inline Vec2i WorldToScreenSize(const RenderScale &scale, const Vec2f &worldSize) {
	Vec2i result = V2i((int)(worldSize.x * scale.worldToScreen.x), (int)(worldSize.y * scale.worldToScreen.y));
	return(result);
}

inline Vec2i WorldToScreenPos(const RenderScale &scale, const Vec2f &worldPos) {
	Vec2i result = V2i(scale.viewport.x + (int)((worldPos.x + scale.worldSize.x * 0.5f) * scale.worldToScreen.x), scale.viewport.y + (int)((worldPos.y + scale.worldSize.y * 0.5f) * scale.worldToScreen.y));
	return(result);
}

static void RenderApp(AppState* appState, const InputState* input, const uint32_t winWidth, const uint32_t winHeight) {
	constexpr float w = AppWidth * 0.5f;
	constexpr float h = AppHeight * 0.5f;

	Viewport vp = ComputeViewportByAspect(V2i(winWidth, winHeight), AppAspect);

	RenderScale scale = {};
	scale.worldSize = V2f(AppWidth, AppHeight);
	scale.worldToScreen = V2f(vp.w / (float)AppWidth, vp.h / (float)AppHeight);
	scale.screenToWorld = V2f(1.0f / scale.worldToScreen.x, 1.0f / scale.worldToScreen.y);
	scale.winWidth = winWidth;
	scale.winHeight = winHeight;
	scale.viewport = vp;

	Vec2i mouseCoord = V2i(input->mousePos.x, winHeight - 1 - input->mousePos.y);
	appState->mousePos = PixelToWorldPos(scale, mouseCoord);

	glViewport(vp.x, vp.y, vp.w, vp.h);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w, w, -h, h, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.1f, 0.2f, 0.6f, 1.0f);
	DrawRect(w, h, 0.0f, 0.0f, true);

	constexpr float osdFontHeight = w * 0.05f;

	const KeyDefinitions* keyDefinitions = &keyDefinitionsArray[0];
	char inputLocale[1024];
	if (fplGetInputLocale(fplLocaleFormat_ISO639, inputLocale, fplArrayCount(inputLocale))) {
		for (int keyDefIndex = 0; keyDefIndex < fplArrayCount(keyDefinitionsArray); ++keyDefIndex) {
			const KeyDefinitions* testKeyDefinitions = &keyDefinitionsArray[keyDefIndex];
			if (CompareStringIgnoreCase(testKeyDefinitions->name, inputLocale) == 0) {
				keyDefinitions = testKeyDefinitions;
				break;
			}
		}
	}
	if (keyDefinitions == nullptr) {
		keyDefinitions = &keyDefinitionsArray[0];
	}
	fplAssert(keyDefinitions != nullptr);

	char textBuffer[256];
	wchar_t wideTextBuffer[256];
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (appState->renderMode == RenderMode::KeyboardAndMouse) {
		fplFormatString(textBuffer, fplArrayCount(textBuffer), "Keyboard: %s (%s)", keyDefinitions->name, (appState->usePolling ? "Use polling" : "Use events"));
	} else if (appState->renderMode == RenderMode::Gamepad) {
		const char* controllerName = input->gamepadState.deviceName;
		fplFormatString(textBuffer, fplArrayCount(textBuffer), "Gamepad: %s (%s)", (controllerName == fpl_null ? "No controller detected" : controllerName), (appState->usePolling ? "Use polling" : "Use events"));
	}
	fplUTF8StringToWideString(textBuffer, fplGetStringLength(textBuffer), wideTextBuffer, fplArrayCount(wideTextBuffer));
	DrawTextFont(wideTextBuffer, 1, &appState->osdFontData, &appState->osdFontTexture, 0, h - osdFontHeight, osdFontHeight, 0.0f, 0.0f);

	DrawTextFont(L"F1 (Keyboard) - F2 (Gamepad)", 1, &appState->osdFontData, &appState->osdFontTexture, 0, -h + osdFontHeight, osdFontHeight, 0.0f, 0.0f);

	if (appState->renderMode == RenderMode::KeyboardAndMouse) {
		constexpr float keyFontHeight = KeyboardW * 0.015f;

		// Draw keyboard
		float keyboardCenterX = -(AppWidth - KeyboardW) * 0.5f;
		float keyboardCenterY = 0.0f;
		Vec2f keyboardCenter = V2f(keyboardCenterX, keyboardCenterY);
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawSprite(appState->keyboardTexture, KeyboardW * 0.5f, KeyboardH * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, keyboardCenterX, keyboardCenterY);
		}

		// Draw keyboard leds
		for (int i = 0; i < fplArrayCount(KeyLedDefinitions); ++i) {
			const KeyLedDef keyLedDef = KeyLedDefinitions[i];
			SpritePosition keyPos = ComputeSpritePosition(keyboardCenter, KeyboardSize, keyLedDef.uv);
			if (input->ledStates & keyLedDef.flag) {
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				DrawSprite(appState->keyboardTexture, keyPos.size.w * 0.5f, keyPos.size.h * 0.5f, keyLedDef.uv, keyPos.pos.x, keyPos.pos.y);
			}
			KeyCharDef keyChar = keyLedDef.key;
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			float x = keyPos.pos.x + ((keyPos.size.w * 0.5f) * keyChar.align.x);
			float y = keyPos.pos.y + ((keyPos.size.h * 0.5f) * keyChar.align.y);
			DrawTextFont(keyChar.text, FontCount, appState->letterFontData, appState->letterFontTextures, x, y, keyFontHeight, 0.0f, 0.0f);
		}

		// Draw keyboard buttons
		for (int keyIndex = 0; keyIndex < 256; keyIndex++) {
			const KeyDef& key = (*keyDefinitions)[(fplKey)keyIndex];
			SpritePosition keyPos = ComputeSpritePosition(keyboardCenter, KeyboardSize, key.uv);
			bool down = input->keyStates[keyIndex] >= fplButtonState_Press;
			if (down) {
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				DrawSprite(appState->keyboardTexture, keyPos.size.w * 0.5f, keyPos.size.h * 0.5f, key.uv, keyPos.pos.x, keyPos.pos.y);
			}
			for (size_t i = 0; i < key.count; ++i) {
				KeyCharDef keyChar = key.chars[i];
				if (down) {
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				} else {
					glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				}
				float x = keyPos.pos.x + ((keyPos.size.w * 0.5f) * keyChar.align.x);
				float y = keyPos.pos.y + ((keyPos.size.h * 0.5f) * keyChar.align.y);
				DrawTextFont(keyChar.text, FontCount, appState->letterFontData, appState->letterFontTextures, x, y, keyFontHeight, 0.0f, 0.0f);
			}
		}

		// Draw mouse and buttons
		constexpr float offsetMouseX = -MouseW * 0.1f;
		float mouseCenterX = keyboardCenterX + KeyboardW * 0.5f + offsetMouseX + MouseW * 0.5f;
		float mouseCenterY = keyboardCenterY;
		Vec2f mouseCenter = V2f(mouseCenterX, mouseCenterY);
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawSprite(appState->mouseTexture, MouseW * 0.5f, MouseH * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, mouseCenterX, mouseCenterY);
		}
		for (int i = 0; i < fplArrayCount(input->mouseStates); ++i) {
			const MouseButtonDef mouseButtonDef = MouseButtonDefinitions[i];
			SpritePosition buttonPos = ComputeSpritePosition(mouseCenter, MouseSize, mouseButtonDef.uv);
			if (input->mouseStates[i] >= fplButtonState_Press) {
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				DrawSprite(appState->mouseTexture, buttonPos.size.w * 0.5f, buttonPos.size.h * 0.5f, mouseButtonDef.uv, buttonPos.pos.x, buttonPos.pos.y);
			}
		}

		// Mouse wheels
		if (fabs(input->mouseWheelDelta) > 0.0f) {
			int wheelIndex = input->mouseWheelDelta > 0 ? 0 : 1;
			const MouseWheelDef wheelDef = MouseWheelDefinitions[wheelIndex];
			SpritePosition buttonPos = ComputeSpritePosition(mouseCenter, MouseSize, wheelDef.uv);
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawSprite(appState->mouseTexture, buttonPos.size.w * 0.5f, buttonPos.size.h * 0.5f, wheelDef.uv, buttonPos.pos.x, buttonPos.pos.y);
		}

		// Draw mouse cursor as key region
		if (1) {
			int pixelsW = KeyboardSmallKeyS.x;
			int pixelsH = KeyboardSmallKeyS.y;
			float mouseW = KeyboardW * (pixelsW / (float)KeyboardImageW);
			float mouseH = KeyboardH * (pixelsH / (float)KeyboardImageH);
			float mouseX = appState->mousePos.x + mouseW * 0.5f;
			float mouseY = appState->mousePos.y + mouseH * 0.5f - mouseH;

			float worldX = ((appState->mousePos.x - keyboardCenterX) / (KeyboardW * 0.5f)) * 0.5f + 0.5f;
			float worldY = ((-appState->mousePos.y + keyboardCenterY) / (KeyboardH * 0.5f)) * 0.5f + 0.5f;

			int posX = (int)(worldX * KeyboardImageW);
			int posY = (int)(worldY * KeyboardImageH);

			UVRect uv = UVRectFromPos(KeyboardImageS, V2i(pixelsW, pixelsH), V2i(posX, posY));

			glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
			DrawSprite(appState->keyboardTexture, mouseW * 0.5f, mouseH * 0.5f, uv, mouseX, mouseY);

			glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(mouseX + mouseW * 0.5f, mouseY + mouseH * 0.5f);
			glVertex2f(mouseX - mouseW * 0.5f, mouseY + mouseH * 0.5f);
			glVertex2f(mouseX - mouseW * 0.5f, mouseY - mouseH * 0.5f);
			glVertex2f(mouseX + mouseW * 0.5f, mouseY - mouseH * 0.5f);
			glEnd();
		}

		// Text input
		{
			float consoleFontHeight = osdFontHeight;
			float padding = consoleFontHeight * 0.35f;

			Vec2f inputSize = V2f(AppWidth * 0.96f, consoleFontHeight);
			Vec2f inputPos = V2f(-w + w * 0.035f, keyboardCenter.y - KeyboardSize.y * 0.45f - consoleFontHeight);

			// Text
			size_t maxBoxTextLength = 73;
			size_t textCharLen = 0;
			size_t textCharPos = 0;
			if (input->textLen > 0) {
				textCharLen = input->textLen;
				if (input->textLen > maxBoxTextLength) {
					size_t delta = input->textLen - maxBoxTextLength;
					textCharPos = delta;
					textCharLen = maxBoxTextLength;
				}
			}

			const wchar_t *textStart = input->text + textCharPos;

			Vec2f textPos = V2f(inputPos.x + padding, inputPos.y + inputSize.y * 0.5f - consoleFontHeight * 0.25f);
			Vec2f textSize = GetTextSize(textStart, textCharLen, 1, &appState->consoleFontData, consoleFontHeight);

			Vec2f clipSize = V2f(inputSize.x - padding * 2.0f, inputSize.y);
			Vec2f clipPos = V2f(inputPos.x + padding, inputPos.y);

			Vec2i scissorPos = WorldToScreenPos(scale, clipPos);
			Vec2i scissorSize = WorldToScreenSize(scale, clipSize);

			clipSize = ScreenToWorldSize(scale, scissorSize);
			clipPos = ScreenToWorldPos(scale, scissorPos);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glScissor(scissorPos.x, scissorPos.y, scissorSize.x, scissorSize.y);
			glEnable(GL_SCISSOR_TEST);

			DrawTextFont(textStart, textCharLen, 1, &appState->consoleFontData, &appState->consoleFontTexture, textPos.x, textPos.y, consoleFontHeight, 1.0f, 1.0f);

			glDisable(GL_SCISSOR_TEST);
			glScissor(0, 0, 0, 0);

			// Cursor
			if (input->showTextCursor) {
				DrawTextFont(L"|", 1, &appState->consoleFontData, &appState->consoleFontTexture, textPos.x + textSize.x - consoleFontHeight * 0.15f, textPos.y, consoleFontHeight, 1.0f, 1.0f);
			}

			{
				char textBuffer[256];
				wchar_t wideTextBuffer[256];
				fplFormatString(textBuffer, fplArrayCount(textBuffer), "Input: (%zu chars)", input->textLen);
				fplUTF8StringToWideString(textBuffer, fplGetStringLength(textBuffer), wideTextBuffer, fplArrayCount(wideTextBuffer));
				DrawTextFont(wideTextBuffer, 1, &appState->osdFontData, &appState->osdFontTexture, inputPos.x, inputPos.y + consoleFontHeight * 1.5f, osdFontHeight, 1.0f, 1.0f);
			}

			// Border
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawRect(inputSize.x * 0.5f, inputSize.y * 0.5f, inputPos.x + inputSize.x * 0.5f, inputPos.y + inputSize.y * 0.5f, false);

#if 0
			// Clip rect
			glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
			DrawRect(clipSize.x * 0.5f, clipSize.y * 0.5f, clipPos.x + clipSize.x * 0.5f, clipPos.y + clipSize.y * 0.5f, false);
#endif
		}
	} else if (appState->renderMode == RenderMode::Gamepad) {
		float gamepadCenterX = 0.0f;
		float gamepadCenterY = 0.0f;
		Vec2f gamepadCenter = V2f(gamepadCenterX, gamepadCenterY);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		DrawSprite(appState->gamepadForegroundTexture, GamepadW * 0.5f, GamepadH * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, gamepadCenterX, gamepadCenterY);

		for (int i = 0; i < fplArrayCount(GamepadButtonsDefinitions); ++i) {
			const GamepadButtonDef def = GamepadButtonsDefinitions[i];
			SpritePosition foregroundPos = ComputeSpritePosition(gamepadCenter, GamepadSize, def.foregroundUV);
			bool down = input->gamepadState.buttons[def.button].isDown == 1;
			if (down) {
				// Background
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawSprite(appState->gamepadForegroundTexture, foregroundPos.size.w * 0.5f, foregroundPos.size.h * 0.5f, def.foregroundUV, foregroundPos.pos.x, foregroundPos.pos.y);

				// Mask
				glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawSprite(appState->gamepadMaskTexture, foregroundPos.size.w * 0.5f, foregroundPos.size.h * 0.5f, def.maskUV, foregroundPos.pos.x, foregroundPos.pos.y);

				// Foreground
				glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				DrawSprite(appState->gamepadForegroundTexture, foregroundPos.size.w * 0.5f, foregroundPos.size.h * 0.5f, def.foregroundUV, foregroundPos.pos.x, foregroundPos.pos.y);

				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}

		// Sticks
		float maxStickLength = GamepadSize.w * 0.065f;
		if (fabsf(input->gamepadState.leftStickX) > 0 || fabsf(input->gamepadState.leftStickY) > 0) {
			SpritePosition stickLeftPos = ComputeSpritePosition(gamepadCenter, GamepadSize, GamepadLeftStickUV);
			float leftStickLength = maxStickLength;
			Vec2f leftStickDirection = V2f(input->gamepadState.leftStickX, input->gamepadState.leftStickY);
			Vec2f leftStickDistance = leftStickDirection * leftStickLength;
			float leftStickArrowWidth = leftStickLength * 0.65f;
			float leftStickArrowDepth = leftStickLength * 0.65f * 0.5f;
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawArrow(stickLeftPos.pos.x, stickLeftPos.pos.y, stickLeftPos.pos.x + leftStickDistance.x, stickLeftPos.pos.y + leftStickDistance.y, leftStickArrowWidth, leftStickArrowDepth, leftStickDirection, 6.0f);
		}
		if (fabsf(input->gamepadState.rightStickX) > 0 || fabsf(input->gamepadState.rightStickY) > 0) {
			SpritePosition rightStickPos = ComputeSpritePosition(gamepadCenter, GamepadSize, GamepadRightStickUV);
			float rightStickLength = maxStickLength;
			Vec2f rightStickDirection = V2f(input->gamepadState.rightStickX, input->gamepadState.rightStickY);
			Vec2f rightStickDistance = rightStickDirection * rightStickLength;
			float rightStickArrowWidth = rightStickLength * 0.65f;
			float rightStickArrowDepth = rightStickLength * 0.65f * 0.5f;
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawArrow(rightStickPos.pos.x, rightStickPos.pos.y, rightStickPos.pos.x + rightStickDistance.x, rightStickPos.pos.y + rightStickDistance.y, rightStickArrowWidth, rightStickArrowDepth, rightStickDirection, 6.0f);
		}

		// Trigger
		SpritePosition leftTriggerPos = ComputeSpritePosition(gamepadCenter, GamepadSize, GamepadLeftTriggerUV);
		SpritePosition rightTriggerPos = ComputeSpritePosition(gamepadCenter, GamepadSize, GamepadRightTriggerUV);
		float maxTriggerLength = GamepadSize.w * 0.065f;
		Vec2f triggerDirection = V2f(0, 1);
		if (fabsf(input->gamepadState.leftTrigger) > 0) {
			float leftTriggerLength = maxTriggerLength * input->gamepadState.leftTrigger;
			float leftTriggerArrowWidth = leftTriggerLength * 0.65f;
			float leftTriggerArrowDepth = leftTriggerLength * 0.65f * 0.5f;
			Vec2f leftTriggerDistance = triggerDirection * leftTriggerLength;
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawArrow(leftTriggerPos.pos.x, leftTriggerPos.pos.y, leftTriggerPos.pos.x + leftTriggerDistance.x, leftTriggerPos.pos.y + leftTriggerDistance.y, leftTriggerArrowWidth, leftTriggerArrowDepth, triggerDirection, 6.0f);
		}
		if (fabsf(input->gamepadState.rightTrigger) > 0) {
			float rightTriggerLength = maxTriggerLength * input->gamepadState.rightTrigger;
			float rightTriggerArrowWidth = rightTriggerLength * 0.65f;
			float rightTriggerArrowDepth = rightTriggerLength * 0.65f * 0.5f;
			Vec2f rightTriggerDistance = triggerDirection * rightTriggerLength;
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawArrow(rightTriggerPos.pos.x, rightTriggerPos.pos.y, rightTriggerPos.pos.x + rightTriggerDistance.x, rightTriggerPos.pos.y + rightTriggerDistance.y, rightTriggerArrowWidth, rightTriggerArrowDepth, triggerDirection, 6.0f);
		}
	}
}

static void SetButtonStateFromModifier(InputState* input, const fplKeyboardState* kbstate, const fplKeyboardModifierFlags flag, const fplKey key) {
	if (kbstate->modifiers & flag) {
		input->keyStates[(int)key] = fplButtonState_Press;
	} else {
		input->keyStates[(int)key] = fplButtonState_Release;
	}
}

static bool KeyWasPressed(const fplButtonState oldState, const fplButtonState newState) {
	bool result = (oldState != newState) && (newState == fplButtonState_Release);
	return(result);
}

static bool KeyIsDown(const fplButtonState newState) {
	bool result = newState >= fplButtonState_Press;
	return(result);
}

static void InsertInputChar(InputState *input, const wchar_t c) {
	// Alloc/Grow input capacity if needed
	if (input->textLen >= input->textCapacity) {
		size_t oldCapacity = input->textCapacity;
		wchar_t *oldText = input->text;
		size_t newCapacity = fplMax(1024, oldCapacity * 2);
		wchar_t *newText = (wchar_t *)fplMemoryAllocate(newCapacity * sizeof(wchar_t));
		if (oldText != fpl_null) {
			CopyWideString(oldText, oldCapacity, newText, newCapacity);
			fplMemoryFree(oldText);
		}
		input->text = newText;
		input->textCapacity = newCapacity;
	}

	// Append char
	fplAssert(input->textLen + 1 <= input->textCapacity);
	input->text[input->textLen++] = c;
}

static void InsertInputText(InputState *input, const wchar_t *text) {
	size_t textLen = GetWideStringLength(text);
	for (size_t i = 0; i < textLen; ++i) {
		InsertInputChar(input, text[i]);
	}
}

static void HandleKeyDown(AppState* appState, InputState *input, const fplKey key) {
	switch (key) {
		case fplKey_Backspace:
			if (input->textLen > 0) {
				input->text[input->textLen - 1] = 0;
				--input->textLen;
			}
			break;
		default:
			break;
	}
}

static void HandleKeyPressed(AppState* appState, InputState *input, const fplKey key) {
	switch (key) {
		case fplKey_F1:
			appState->renderMode = RenderMode::KeyboardAndMouse;
			break;
		case fplKey_F2:
			appState->renderMode = RenderMode::Gamepad;
			break;
		case fplKey_F5:
			appState->usePolling = !appState->usePolling;
			fplSetWindowInputEvents(!appState->usePolling);
			break;
		case fplKey_Tab:
			InsertInputText(input, L"    ");
			break;
		case fplKey_Return:
			if (input->textLen > 0) {
				input->textLen = 0;
				input->text[0] = 0;
			}
			break;
		default:
			break;
	}
}

int main(int argc, char* argv[]) {
	AppState* appState = (AppState*)fplMemoryAllocate(sizeof(AppState));
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Input Demo", settings.window.title, fplArrayCount(settings.window.title));
	int retCode = 0;

	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			InputState input = {};
			fplButtonState lastKeyStates[256] = {};
			InitApp(appState);
			fplSetWindowInputEvents(!appState->usePolling);
			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					switch (ev.type) {
						case fplEventType_Mouse:
						{
							if (ev.mouse.type == fplMouseEventType_Move) {
								input.mousePos.x = ev.mouse.mouseX;
								input.mousePos.y = ev.mouse.mouseY;
							} else if (ev.mouse.type == fplMouseEventType_Button) {
								input.mouseStates[(int)ev.mouse.mouseButton] = ev.mouse.buttonState;
							} else if (ev.mouse.type == fplMouseEventType_Wheel) {
								input.mouseWheelDelta = ev.mouse.wheelDelta;
								input.lastMouseWheelUpdateTime = fplGetTimeInMillisecondsLP();
							}
						} break;

						case fplEventType_Keyboard:
						{
							if (!appState->usePolling) {
								if (ev.keyboard.type == fplKeyboardEventType_Button) {
									input.keyStates[(int)ev.keyboard.mappedKey] = ev.keyboard.buttonState;
								}
							}
							if (ev.keyboard.type == fplKeyboardEventType_Button) {
								if (ev.keyboard.buttonState == fplButtonState_Release) {
									HandleKeyPressed(appState, &input, ev.keyboard.mappedKey);
								} else if (ev.keyboard.buttonState >= fplButtonState_Press) {
									HandleKeyDown(appState, &input, ev.keyboard.mappedKey);
								}
							} else if (ev.keyboard.type == fplKeyboardEventType_Input) {
								if ((ev.keyboard.keyCode > 0 && ev.keyboard.keyCode < INT16_MAX) &&
									((ev.keyboard.mappedKey != fplKey_Backspace) && (ev.keyboard.mappedKey != fplKey_Tab) && (ev.keyboard.mappedKey != fplKey_Return))) {
									wchar_t c = (wchar_t)ev.keyboard.keyCode;
									InsertInputChar(&input, c);
								}
							}
						} break;

						case fplEventType_Gamepad:
						{
							if (!appState->usePolling) {
								if ((ev.gamepad.type == fplGamepadEventType_StateChanged) ||
									(ev.gamepad.type == fplGamepadEventType_Connected) ||
									(ev.gamepad.type == fplGamepadEventType_Disconnected)) {
									input.gamepadState = ev.gamepad.state;
								}
							}
						} break;

						default:
							break;
					}
				}

				fplKeyboardState keyboardState = {};
				if (fplPollKeyboardState(&keyboardState)) {
					if (appState->usePolling) {
						for (int i = 0; i < 256; ++i) {
							if (KeyWasPressed(lastKeyStates[i], keyboardState.buttonStatesMapped[i])) {
								HandleKeyPressed(appState, &input, (fplKey)i);
							} else if (KeyIsDown(keyboardState.buttonStatesMapped[i])) {
								HandleKeyDown(appState, &input, (fplKey)i);
							}
							input.keyStates[i] = keyboardState.buttonStatesMapped[i];
							lastKeyStates[i] = input.keyStates[i];
						}
					}
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_LShift, fplKey_LeftShift);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_RShift, fplKey_RightShift);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_LAlt, fplKey_LeftAlt);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_RAlt, fplKey_RightAlt);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_LCtrl, fplKey_LeftControl);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_RCtrl, fplKey_RightControl);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_LSuper, fplKey_LeftSuper);
					SetButtonStateFromModifier(&input, &keyboardState, fplKeyboardModifierFlags_RSuper, fplKey_RightSuper);
					input.ledStates = fplKeyboardModifierFlags_None;
					if (keyboardState.modifiers & fplKeyboardModifierFlags_CapsLock) {
						input.ledStates |= fplKeyboardModifierFlags_CapsLock;
					}
					if (keyboardState.modifiers & fplKeyboardModifierFlags_ScrollLock) {
						input.ledStates |= fplKeyboardModifierFlags_ScrollLock;
					}
					if (keyboardState.modifiers & fplKeyboardModifierFlags_NumLock) {
						input.ledStates |= fplKeyboardModifierFlags_NumLock;
					}
				}

				if (appState->usePolling) {
					fplGamepadStates gamepadStates = {};
					if (fplPollGamepadStates(&gamepadStates)) {
						int found = -1;
						for (int i = 0; i < fplArrayCount(gamepadStates.deviceStates); ++i) {
							if (gamepadStates.deviceStates[i].isConnected && gamepadStates.deviceStates[i].isActive) {
								found = i;
								break;
							}
						}
						input.gamepadState = {};
						if (found > -1) {
							input.gamepadState = gamepadStates.deviceStates[found];
						}
					}
				}

				if (appState->usePolling) {
					fplMouseState mouseState = {};
					if (fplPollMouseState(&mouseState)) {
						input.mousePos.x = mouseState.x;
						input.mousePos.y = mouseState.y;
						fplAssert(fplArrayCount(mouseState.buttonStates) <= fplArrayCount(input.mouseStates));
						for (int i = 0; i < fplArrayCount(mouseState.buttonStates); ++i) {
							input.mouseStates[i] = mouseState.buttonStates[i];
						}
					}
				}

				// Reset mouse wheel delta after half a second
				uint64_t maxWheelShowTime = 500;
				if (input.lastMouseWheelUpdateTime > 0) {
					if (fplGetTimeInMillisecondsLP() - input.lastMouseWheelUpdateTime >= maxWheelShowTime) {
						input.lastMouseWheelUpdateTime = 0;
						input.mouseWheelDelta = 0.0f;
					}
				}

				// Cursor blinking
				uint64_t maxCursorShowTime = 500;
				if (input.lastTextCursorBlinkTime == 0) {
					input.lastTextCursorBlinkTime = fplGetTimeInMillisecondsLP();
					input.showTextCursor = true;
				} else {
					if ((fplGetTimeInMillisecondsLP() - input.lastTextCursorBlinkTime) >= maxCursorShowTime) {
						input.showTextCursor = !input.showTextCursor;
						input.lastTextCursorBlinkTime = fplGetTimeInMillisecondsLP();
					}
				}

				fplWindowSize wsize = {};
				fplGetWindowSize(&wsize);
				RenderApp(appState, &input, wsize.width, wsize.height);
				fplVideoFlip();
			}
			ReleaseApp(appState);
			fglUnloadOpenGL();
		} else {
			retCode = -1;
		}
		fplPlatformRelease();
	} else {
		retCode = -1;
	}
	fplMemoryFree(appState);
	return(retCode);
}