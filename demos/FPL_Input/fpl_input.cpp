#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// @BAD(final): CPP is such garbage!
// It cannot handle array index initializer such as [index] = value :-(
// So we need this nonsense just to initialize a static array -.-template <typename TIndexType, typename TValueType, size_t size>
template <typename TIndexType, typename TValueType, size_t valueCount>
class ArrayInitializer {
protected:
	TValueType a[valueCount];
public:
	ArrayInitializer() {
		fplMemoryClear(a, sizeof(TValueType) * FPL_ARRAYCOUNT(a));
	}
	const TValueType &operator [] (TIndexType eindex) const {
		return a[(int)eindex];
	}
	TValueType &operator [] (TIndexType eindex) {
		return a[(int)eindex];
	}
	void Set(TIndexType e, const TValueType &value) {
		a[(int)e] = value;
	}
};

char ToLowerCase(char ch) {
	if (ch >= 'A' && ch <= 'Z') {
		ch = 'a' + (ch - 'A');
	}
	return ch;
}

static int CompareStringIgnoreCase(const char *a, const char *b) {
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

inline Vec2f operator*(const Vec2f &a, float b) {
	Vec2f result = V2f(a.x * b, a.y * b);
	return(result);
}
inline Vec2f operator+(const Vec2f &a, const Vec2f &b) {
	Vec2f result = V2f(a.x + b.x, a.y + b.y);
	return(result);
}
inline Vec2f& operator+=(Vec2f &a, const Vec2f &b) {
	a = b + a;
	return(a);
}

struct Viewport {
	int x;
	int y;
	int w;
	int h;
};

Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect) {
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

inline UVRect UVRectFromPos(const Vec2i &imageSize, const Vec2i &partSize, const Vec2i &pos) {
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
	uint8_t *atlasAlphaBitmap;
	FontGlyph *glyphs;
	float ascent;
	float descent;
	float lineHeight;
	float spaceAdvance;
	float *defaultAdvance;
	float *kerningTable;
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint32_t firstChar;
	uint32_t charCount;
	bool hasKerningTable;
};

inline float GetFontAscent(const FontData *font) {
	float result = font->ascent;
	return(result);
}

inline float GetFontDescent(const FontData *font) {
	float result = font->descent;
	return(result);
}

inline float GetFontLineAdvance(const FontData *font) {
	float result = font->lineHeight;
	return(result);
}

float GetFontCharacterAdvance(const FontData *font, const uint32_t thisCodePoint) {
	float result = 0;
	if (thisCodePoint >= font->firstChar && thisCodePoint < (font->firstChar - font->charCount)) {
		uint32_t thisIndex = thisCodePoint - font->firstChar;
		const FontGlyph *glyph = font->glyphs + thisIndex;
		result = font->defaultAdvance[thisIndex];
	}
	return(result);
}

int GetFontAtlasIndexFromCodePoint(const size_t fontCount, const FontData fonts[], const uint32_t codePoint) {
	for (size_t i = 0; i < fontCount; ++i) {
		uint32_t lastChar = fonts[i].firstChar + (fonts[i].charCount - 1);
		if (codePoint >= fonts[i].firstChar && codePoint <= lastChar) {
			return (int)i;
		}
	}
	return -1;
}

Vec2f GetTextSize(const wchar_t *text, const size_t textLen, const size_t fontCount, const FontData fonts[], const float maxCharHeight) {
	float xwidth = 0.0f;
	float ymax = 0.0f;
	if (fontCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		for (uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];

			int atlasIndex = GetFontAtlasIndexFromCodePoint(fontCount, fonts, at);
			const FontData *font;
			if (atlasIndex > -1) {
				font = &fonts[atlasIndex];
			} else {
				font = &fonts[0];
			}

			float xadvance;
			Vec2f offset = V2f(xpos, ypos);
			Vec2f size = V2f();
			uint32_t lastChar = font->firstChar + (font->charCount - 1);
			if (at >= font->firstChar && at <= lastChar) {
				uint32_t codePoint = at - font->firstChar;
				const FontGlyph *glyph = font->glyphs + codePoint;
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
			ymax = FPL_MAX(ymax, max.y - min.y);
			xpos += xadvance;
		}
	}
	Vec2f result = V2f(xwidth, ymax) * maxCharHeight;
	return(result);
}

bool LoadFontFromMemory(const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, FontData *outFont) {
	if (data == fpl_null || dataSize == 0) {
		return false;
	}
	if (outFont == fpl_null) {
		return false;
	}

	FPL_CLEAR_STRUCT(outFont);

	stbtt_fontinfo fontInfo = FPL_ZERO_INIT;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char *)data, fontIndex);

	bool result = false;
	if (stbtt_InitFont(&fontInfo, (const unsigned char *)data, fontOffset)) {
		uint32_t charCount = (lastChar - firstChar) + 1;
		uint8_t *atlasAlphaBitmap = (uint8_t *)fplMemoryAllocate(atlasWidth * atlasHeight);
		stbtt_bakedchar *packedChars = (stbtt_bakedchar *)fplMemoryAllocate(charCount * sizeof(stbtt_bakedchar));
		stbtt_BakeFontBitmap((const unsigned char *)data, fontOffset, fontSize, atlasAlphaBitmap, atlasWidth, atlasHeight, firstChar, charCount, packedChars);

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
		FontGlyph *glyphs = (FontGlyph *)fplMemoryAllocate(glyphsSize);

		for (uint32_t glyphIndex = 0; glyphIndex < charCount; ++glyphIndex) {
			stbtt_bakedchar *sourceInfo = packedChars + glyphIndex;

			FontGlyph *destInfo = glyphs + glyphIndex;
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
		float *kerningTable;
		if (loadKerning) {
			kerningTableSize = sizeof(float) * charCount * charCount;
			kerningTable = (float *)fplMemoryAllocate(kerningTableSize);
		} else {
			kerningTableSize = 0;
			kerningTable = fpl_null;
		}

		size_t defaultAdvanceSize = charCount * sizeof(float);
		float *defaultAdvance = (float *)fplMemoryAllocate(defaultAdvanceSize);
		for (uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
			uint32_t codePointIndex = (uint32_t)(charIndex - firstChar);
			stbtt_bakedchar *leftInfo = packedChars + codePointIndex;
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

bool LoadFontFromFile(const char *dataPath, const char *filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, FontData *outFont) {
	if (filename == fpl_null) {
		return false;
	}
	if (outFont == fpl_null) {
		return false;
	}

	char filePath[1024];
	if (dataPath != fpl_null) {
		fplCopyAnsiString(dataPath, filePath, FPL_ARRAYCOUNT(filePath));
		fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);
	} else {
		fplCopyAnsiString(filename, filePath, FPL_ARRAYCOUNT(filePath));
	}

	bool result = false;

	fplFileHandle file;
	uint8_t *ttfBuffer = fpl_null;
	uint32_t ttfBufferSize = 0;
	if (fplOpenAnsiBinaryFile(filePath, &file)) {
		ttfBufferSize = fplGetFileSizeFromHandle32(&file);
		ttfBuffer = (uint8_t *)fplMemoryAllocate(ttfBufferSize);
		fplReadFileBlock32(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
		fplCloseFile(&file);
	}

	if (ttfBuffer != nullptr) {
		result = LoadFontFromMemory(ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, outFont);
		fplMemoryFree(ttfBuffer);
	}
	return(result);
}

void ReleaseFont(FontData *font) {
	if (font != fpl_null) {
		if (font->hasKerningTable) {
			fplMemoryFree(font->kerningTable);
		}
		fplMemoryFree(font->glyphs);
		fplMemoryFree(font->atlasAlphaBitmap);
		FPL_CLEAR_STRUCT(font);
	}
}

void DrawSprite(const GLuint texId, const float rx, const float ry, const float uMin, const float vMin, const float uMax, const float vMax, const float xoffset, const float yoffset) {
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

void DrawRect(const float rx, const float ry, const float xoffset, const float yoffset) {
	glBegin(GL_LINE_LOOP);
	glVertex2f(xoffset + rx, yoffset + ry);
	glVertex2f(xoffset - rx, yoffset + ry);
	glVertex2f(xoffset - rx, yoffset - ry);
	glVertex2f(xoffset + rx, yoffset - ry);
	glEnd();
}

void DrawTextFont(const wchar_t *text, const size_t fontCount, const FontData fonts[], const GLuint textures[], const float x, const float y, const float maxCharHeight, const float sx, const float sy) {
	if (fontCount > 0) {
		size_t textLen = fplGetWideStringLength(text);
		Vec2f textSize = GetTextSize(text, textLen, fontCount, fonts, maxCharHeight);
		float xpos = x - textSize.x * 0.5f + (textSize.x * 0.5f * sx);
		float ypos = y - textSize.y * 0.5f + (textSize.y * 0.5f * sy);
		for (uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];

			int atlasIndex = GetFontAtlasIndexFromCodePoint(fontCount, fonts, at);
			const FontData *font;
			GLuint texture;
			if (atlasIndex > -1) {
				font = &fonts[atlasIndex];
				texture = textures[atlasIndex];
			} else {
				font = &fonts[0];
				texture = textures[0];
			}

			uint32_t lastChar = font->firstChar + (font->charCount - 1);
			float advance;
			if ((uint32_t)at >= font->firstChar && (uint32_t)at <= lastChar) {
				uint32_t codePoint = at - font->firstChar;
				const FontGlyph *glyph = &font->glyphs[codePoint];
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

GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const bool repeatable, const GLint filter, const bool isAlphaOnly) {
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
constexpr float KeyboardW = AppWidth;
constexpr float KeyboardH = KeyboardW / KeyboardAspect;

struct KeyCharDef {
	const wchar_t *text;
	Vec2f align;
};
inline KeyCharDef MakeKeyChar(const wchar_t *text = nullptr, const Vec2f &align = {}) {
	KeyCharDef result = { text, align };
	return(result);
}

struct KeyDef {
	KeyCharDef chars[4];
	UVRect uv;
	size_t count;
};

class KeyDefinitions : public ArrayInitializer<fplKey, KeyDef, 256> {
protected:
	void AddKeyDef(const fplKey index, const UVRect &uv, const int count, ...) {
		KeyDef def = {};
		def.uv = uv;
		va_list argList;
		va_start(argList, count);
		for (int i = 0; i < count; ++i) {
			KeyCharDef charDef = va_arg(argList, KeyCharDef);
			FPL_ASSERT(def.count < FPL_ARRAYCOUNT(def.chars));
			def.chars[def.count++] = charDef;
		}
		va_end(argList);
		Set(index, def);
	}
public:
	const char *name;
	KeyDefinitions(const char *name) {
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
		AddKeyDef(fplKey_E, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(345, 468)), 2, MakeKeyChar(L"E", topLeftAlign)), MakeKeyChar(L"€", V2f(0.4f, -0.4f));
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
		AddKeyDef(fplKey_E, UVRectFromPos(KeyboardImageS, KeyboardSmallKeyS, V2i(345, 468)), 2, MakeKeyChar(L"E"), MakeKeyChar(L"€", V2f(0.5f, -0.5f)));
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
	UVRect uv;
};

static KeyLedDef KeyLedDefinitions[] = {
	{fplKeyboardModifierFlags_CapsLock, UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1686, 325))},
	{fplKeyboardModifierFlags_ScrollLock, UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1753, 325))},
	{fplKeyboardModifierFlags_NumLock, UVRectFromPos(KeyboardImageS, KeyboardLedS, V2i(1820, 325))},
};

constexpr size_t CodePointCount = 10000;
constexpr size_t CodePointsPerAtlas = 256;
constexpr size_t FontCount = CodePointCount / CodePointsPerAtlas + 1;

struct AppState {
	FontData fontData[FontCount];
	GLuint fontTextures[FontCount];
	GLuint keyboardTexture;
	GLuint gamepadTexture;
	Vec2f mousePos;
};

struct InputState {
	Vec2i mousePos;
	fplButtonState keyStates[256];
	fplKeyboardModifierFlags ledStates;
};

static GLuint LoadTexture(const char *dataPath, const char *filename) {
	GLuint result = 0;

	char filePath[FPL_MAX_PATH_LENGTH];
	fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);

	fplFileHandle file = {};
	if (fplOpenAnsiBinaryFile(filePath, &file)) {
		uint32_t dataSize = fplGetFileSizeFromHandle32(&file);
		uint8_t *data = (uint8_t *)fplMemoryAllocate(dataSize);
		fplReadFileBlock32(&file, dataSize, data, dataSize);
		int w, h, components;
		stbi_set_flip_vertically_on_load(0);
		uint8_t *pixels = stbi_load_from_memory(data, (int)dataSize, &w, &h, &components, 4);
		if (pixels != nullptr) {
			result = AllocateTexture(w, h, pixels, false, GL_LINEAR, false);
			stbi_image_free(pixels);
		}
		fplMemoryFree(data);
		fplCloseFile(&file);
	}
	return(result);
}

static void InitApp(AppState *appState) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glClearColor(0.1f, 0.2f, 0.6f, 1.0f);

	char dataPath[FPL_MAX_PATH_LENGTH];
	fplGetExecutableFilePath(dataPath, FPL_ARRAYCOUNT(dataPath));
	fplExtractFilePath(dataPath, dataPath, FPL_ARRAYCOUNT(dataPath));
	fplPathCombine(dataPath, FPL_ARRAYCOUNT(dataPath), 2, dataPath, "data");

	for (int i = 0; i < FontCount; ++i) {
		int cpStart = i * CodePointsPerAtlas;
		int cpEnd = cpStart + (CodePointsPerAtlas - 1);
		if (LoadFontFromFile(dataPath, "NotoSans-Regular.ttf", 0, 36.0f, cpStart, cpEnd, 512, 512, false, &appState->fontData[i])) {
			appState->fontTextures[i] = AllocateTexture(appState->fontData[i].atlasWidth, appState->fontData[i].atlasHeight, appState->fontData[i].atlasAlphaBitmap, false, GL_LINEAR, true);
		}
	}

	appState->keyboardTexture = LoadTexture(dataPath, "keyboard.png");
	appState->gamepadTexture = LoadTexture(dataPath, "gamepad.png");

}

static void ReleaseApp(AppState *appState) {
	glDeleteTextures(1, &appState->gamepadTexture);
	glDeleteTextures(1, &appState->keyboardTexture);
	glDeleteTextures(FontCount, &appState->fontTextures[0]);
	for (int i = 0; i < FontCount; ++i) {
		ReleaseFont(&appState->fontData[i]);
	}
}

static void RenderApp(AppState *appState, const InputState *input, const uint32_t winWidth, const uint32_t winHeight) {
	constexpr float w = AppWidth * 0.5f;
	constexpr float h = AppHeight * 0.5f;

	Viewport vp = ComputeViewportByAspect(V2i(winWidth, winHeight), AppAspect);

	float worldToScreenX = vp.w / (float)AppWidth;
	float worldToScreenY = vp.h / (float)AppHeight;
	float screenToWorldX = 1.0f / worldToScreenX;
	float screenToWorldY = 1.0f / worldToScreenY;

	int mouseCenterX = (input->mousePos.x - winWidth / 2);
	int mouseCenterY = (winHeight - 1 - input->mousePos.y) - winHeight / 2;
	appState->mousePos.x = (mouseCenterX * screenToWorldX);
	appState->mousePos.y = (mouseCenterY * screenToWorldY);

	glViewport(vp.x, vp.y, vp.w, vp.h);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w, w, -h, h, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	constexpr float osdFontHeight = w * 0.05f;
	constexpr float keyFontHeight = w * 0.03f;

	const KeyDefinitions *keyDefinitions = &keyDefinitionsArray[0];
	char inputLocale[16];
	if (fplGetInputLocale(fplLocaleFormat_ISO639, inputLocale, FPL_ARRAYCOUNT(inputLocale))) {
		for (int keyDefIndex = 0; keyDefIndex < FPL_ARRAYCOUNT(keyDefinitionsArray); ++keyDefIndex) {
			const KeyDefinitions *testKeyDefinitions = &keyDefinitionsArray[keyDefIndex];
			if (CompareStringIgnoreCase(testKeyDefinitions->name, inputLocale) == 0) {
				keyDefinitions = testKeyDefinitions;
				break;
			}
		}
	}
	if (keyDefinitions == nullptr) {
		keyDefinitions = &keyDefinitionsArray[0];
	}
	FPL_ASSERT(keyDefinitions != nullptr);

	char textBuffer[256];
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "Keyboard: %s", keyDefinitions->name);
	wchar_t wideTextBuffer[256];
	fplAnsiStringToWideString(textBuffer, fplGetAnsiStringLength(textBuffer), wideTextBuffer, FPL_ARRAYCOUNT(wideTextBuffer));
	DrawTextFont(wideTextBuffer, FontCount, appState->fontData, appState->fontTextures, 0, h - osdFontHeight, osdFontHeight, 0.0f, 0.0f);

	{
		float x = 0.0f;
		float y = 0.0f;
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		DrawSprite(appState->keyboardTexture, KeyboardW * 0.5f, KeyboardH * 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, x, y);
	}

	for (int i = 0; i < FPL_ARRAYCOUNT(KeyLedDefinitions); ++i) {
		const KeyLedDef keyLedDef = KeyLedDefinitions[i];
		float keyW = KeyboardW * (keyLedDef.uv.uMax - keyLedDef.uv.uMin);
		float keyH = KeyboardH * (keyLedDef.uv.vMax - keyLedDef.uv.vMin);
		float keyOffsetX = KeyboardW * (keyLedDef.uv.uMin);
		float keyOffsetY = KeyboardH * (1.0f - keyLedDef.uv.vMax);
		float keyCenterX = 0.0f - KeyboardW * 0.5f + keyOffsetX + keyW * 0.5f;
		float keyCenterY = 0.0f - KeyboardH * 0.5f + keyOffsetY + keyH * 0.5f;
		if (input->ledStates & keyLedDef.flag) {
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawSprite(appState->keyboardTexture, keyW * 0.5f, keyH * 0.5f, keyLedDef.uv, keyCenterX, keyCenterY);
		}
	}

	for (int keyIndex = 0; keyIndex < 256; keyIndex++) {
		const KeyDef &key = (*keyDefinitions)[(fplKey)keyIndex];
		float keyW = KeyboardW * (key.uv.uMax - key.uv.uMin);
		float keyH = KeyboardH * (key.uv.vMax - key.uv.vMin);
		float keyOffsetX = KeyboardW * (key.uv.uMin);
		float keyOffsetY = KeyboardH * (1.0f - key.uv.vMax);
		float keyCenterX = 0.0f - KeyboardW * 0.5f + keyOffsetX + keyW * 0.5f;
		float keyCenterY = 0.0f - KeyboardH * 0.5f + keyOffsetY + keyH * 0.5f;
#if 0
		bool down = true;// input->keyStates[keyIndex] >= fplButtonState_Press;
#else
		bool down = input->keyStates[keyIndex] >= fplButtonState_Press;
#endif
		if (down) {
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			DrawSprite(appState->keyboardTexture, keyW * 0.5f, keyH * 0.5f, key.uv, keyCenterX, keyCenterY);
		}
		for (int i = 0; i < key.count; ++i) {
			KeyCharDef keyChar = key.chars[i];
			if (down) {
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			} else {
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			}
			float x = keyCenterX + ((keyW * 0.5f) * keyChar.align.x);
			float y = keyCenterY + ((keyH * 0.5f) * keyChar.align.y);
			DrawTextFont(keyChar.text, FontCount, appState->fontData, appState->fontTextures, x, y, keyFontHeight, 0.0f, 0.0f);
		}
	}
#if 0
	{
		int pixelsW = 1914;
		int pixelsH = 612;
		float mouseW = KeyboardW * (pixelsW / (float)KeyboardImageW);
		float mouseH = KeyboardH * (pixelsH / (float)KeyboardImageH);
		float mouseX = appState->mousePos.x + mouseW * 0.5f;
		float mouseY = appState->mousePos.y + mouseH * 0.5f - mouseH;

		float worldX = (appState->mousePos.x / (KeyboardW * 0.5f)) * 0.5f + 0.5f;
		float worldY = (-appState->mousePos.y / (KeyboardH * 0.5f)) * 0.5f + 0.5f;

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

		char textBuffer[100];
		fplFormatAnsiString(textBuffer, FPL_ARRAYCOUNT(textBuffer), "%d x %d (%f x %f)", posX, posY, worldX, worldY);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		DrawTextFont(textBuffer, &appState->fontData, appState->fontTexture, appState->mousePos.x, appState->mousePos.y, osdFontHeight * 0.5f, 1.0f, -1.0f);
		}
#endif
	}

static void SetButtonStateFromModifier(InputState *input, const fplKeyboardState *kbstate, const fplKeyboardModifierFlags flag, const fplKey key) {
	if (kbstate->modifiers & flag) {
		input->keyStates[(int)key] = fplButtonState_Press;
	} else {
		input->keyStates[(int)key] = fplButtonState_Release;
	}
}

int main(int argc, char *argv[]) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyAnsiString("FPL Input Demo", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	fplInitResultType initRes = fplPlatformInit(fplInitFlags_All, &settings);
	if (initRes == fplInitResultType_Success) {
		if (fglLoadOpenGL(true)) {
			AppState *appState = (AppState *)fplMemoryAllocate(sizeof(AppState));
			InputState input = {};
			InitApp(appState);
			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					switch (ev.type) {
						case fplEventType_Mouse:
						{
							if (ev.mouse.type == fplMouseEventType_Move) {
								input.mousePos.x = ev.mouse.mouseX;
								input.mousePos.y = ev.mouse.mouseY;
							}
						} break;

#if 1
						case fplEventType_Keyboard:
						{
							if (ev.keyboard.type == fplKeyboardEventType_Button) {
								input.keyStates[(int)ev.keyboard.mappedKey] = ev.keyboard.buttonState;
							}
						} break;
#endif
					}
				}

				fplKeyboardState keyboardState = {};
				if (fplGetKeyboardState(&keyboardState)) {
					for (int i = 0; i < 256; ++i) {
						//input.keyStates[i] = keyboardState.buttonStatesMapped[i];
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

				fplWindowSize wsize = {};
				fplGetWindowArea(&wsize);
				RenderApp(appState, &input, wsize.width, wsize.height);
				fplVideoFlip();
			}
			ReleaseApp(appState);
			fglUnloadOpenGL();
			fplPlatformRelease();
		}
		return 0;
	} else {
		return -1;
	}
}