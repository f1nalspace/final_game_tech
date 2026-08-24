/*
Name:
	Final Font Loader

Description:
	Single-header font loader and glyph layout helper using STB_truetype.

	One LoadedFont = one alpha atlas + one font + one contiguous codepoint range.
	Multiple LoadedFonts can be combined via the multi-font helpers below.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
	## 2026-08-24
	- Fixed the kerning table held a ratio instead of an advance: the raw value was scaled by 1/fontSize instead of by the font's raw-to-pixel scale, and then divided by the left glyph's ATLAS width. FontGetCharacterAdvance adds the entry to defaultAdvance, which is in font units, so a kerned pair collapsed onto itself
	- Fixed kerning pairs whose left glyph carries no ink (the space, for one) were dropped, because the entry was only written when the glyph had a non-zero atlas width
	- Fixed the kerning table was left uninitialized: only the pairs a font actually kerns are written, and a custom MemoryAllocator makes no promise to hand back zeroed memory

	## 2026-05-28
	- Replaced compile-time FINAL_FONTLOADER_BETTERQUALITY with a runtime FontQuality enum
	- Added FontLoadFromMemoryEx / FontLoadFromFileEx accepting FontQuality
	- Added FontGetQuadCorners for 4-corner cartesian-flipped glyph layout (FFMpeg parity)
	- Added FontGetTextSizeCodepoints for pre-decoded uint32 arrays (FPL_Input parity)
	- Added FontFindAtlasForCodepoint and FontGetTextSizeMulti for multi-font fallback
	- Added FontGetBaseline returning the NBody-style centered baseline correction
	- Fixed kerning guard in FontGetCharacterAdvance (firstChar - charCount typo)
	- Fixed FontGetTextSize ymax computation (min.h typo)
	- Fixed defaultAdvance/kerning loop missing the last codepoint

	## 2026-04-19
	- Fixed raw to pixel scaling was incorrect

	## 2025-11-30
	- Introduced custom memory allocator support
	- Fixed various memory leaks

	## 2018-06-30
	- Fixed crash on ReleaseFont when not using kerning

	## 2018-06-27
	- Fixed font baking was totally busted
	- Removed GetTextWidth()
	- Added GetTextSize()
	- Signature of GetFontCharacterAdvance changed
*/

#ifndef FINAL_FONTLOADER_H
#define FINAL_FONTLOADER_H

// Legacy define: when set, old-signature loaders default to Packed4x quality.
// Prefer passing FontQuality_Packed4x explicitly via the *Ex loaders.
#ifndef FINAL_FONTLOADER_BETTERQUALITY
#define FINAL_FONTLOADER_BETTERQUALITY 0
#endif

#include <final_core.h>
#include <final_math.h>

typedef enum FontQuality {
	// stbtt_bakedchar, no oversampling. Cheapest, blurriest at small sizes.
	FontQuality_Baked = 0,
	// stbtt_packedchar, oversample 1x1.
	FontQuality_Packed1x,
	// stbtt_packedchar, oversample 2x2.
	FontQuality_Packed2x,
	// stbtt_packedchar, oversample 4x4. Sharpest, largest atlas footprint.
	FontQuality_Packed4x,
} FontQuality;

typedef struct FontGlyph {
	Vec2f offset;
	Vec2f uvMin, uvMax;
	Vec2f charSize;
	uint32_t charCode;
} FontGlyph;

typedef struct FontQuad {
	Vec2f offset;
	Vec2f size;
	Vec2f uvMin;
	Vec2f uvMax;
} FontQuad;

// 4-corner expansion of a glyph for VBO-style renderers.
// Order: TR, TL, BL, BR. Y axis is flipped when cartesian = true.
typedef struct FontQuadCorners {
	Vec2f uv[4];
	Vec2f offset[4];
	float advance;
	uint32_t charCode;
} FontQuadCorners;

typedef struct FontInfo {
	float ascent;
	float descent;
	float lineHeight;
	float spaceAdvance;
} FontInfo;

typedef struct LoadedFont {
	uint8_t *atlasAlphaBitmap;
	FontGlyph *glyphs;
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint32_t firstChar;
	uint32_t charCount;
	FontInfo info;
	float *defaultAdvance;
	float *kerningTable;
	bool hasKerningTable;
} LoadedFont;

fpl_inline const FontGlyph *FontGetGlyph(const LoadedFont *font, const uint32_t codePoint) {
	if(font == fpl_null) {
		return(fpl_null);
	}
	uint32_t lastCharPastOne = font->firstChar + font->charCount;
	fplAssert(codePoint >= font->firstChar && codePoint < lastCharPastOne);
	uint32_t charIndex = codePoint - font->firstChar;
	const FontGlyph *result = &font->glyphs[charIndex];
	return(result);
}

fpl_inline float FontGetAscent(const FontInfo *fontInfo) {
	float result = fontInfo->ascent;
	return(result);
}

fpl_inline float FontGetDescent(const FontInfo *fontInfo) {
	float result = fontInfo->descent;
	return(result);
}

fpl_inline float FontGetLineAdvance(const FontInfo *fontInfo) {
	float result = fontInfo->lineHeight;
	return(result);
}

// Centered baseline correction in unit-scale: shift to move the glyph midline to y=0.
// Matches the old NBody font.h baseline = (descent - 0.5 * (ascent + descent)).
fpl_inline float FontGetBaseline(const FontInfo *fontInfo) {
	float result = fontInfo->descent - 0.5f * (fontInfo->ascent + fontInfo->descent);
	return(result);
}

fpl_extern Vec2f FontGetTextSize(const LoadedFont *font, const char *text, const size_t textLen, const float maxCharHeight);
fpl_extern Vec2f FontGetTextSizeCodepoints(const LoadedFont *font, const uint32_t *codepoints, const size_t count, const float maxCharHeight);
fpl_extern FontQuad FontGetQuad(const LoadedFont *font, const uint32_t codePoint, const float scale);
fpl_extern FontQuadCorners FontGetQuadCorners(const LoadedFont *font, const uint32_t codePoint, const float scale, const bool cartesian);
fpl_extern float FontGetCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint);

// Returns the index of the font in fonts[] whose [firstChar..lastChar] range covers codePoint, or -1.
fpl_extern int FontFindAtlasForCodepoint(const LoadedFont *fonts, const size_t fontCount, const uint32_t codePoint);
// Like FontGetTextSize but walks a codepoint array and picks per-codepoint atlas via FontFindAtlasForCodepoint.
// Fallback when no atlas covers a codepoint: fonts[0] (used only for spaceAdvance).
fpl_extern Vec2f FontGetTextSizeMulti(const LoadedFont *fonts, const size_t fontCount, const uint32_t *codepoints, const size_t count, const float maxCharHeight);

fpl_extern bool FontLoadFromFile(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
fpl_extern bool FontLoadFromMemory(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
fpl_extern bool FontLoadFromFileEx(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, const FontQuality quality, LoadedFont *outFont);
fpl_extern bool FontLoadFromMemoryEx(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, const FontQuality quality, LoadedFont *outFont);
fpl_extern void FontFree(MemoryAllocator *allocator, LoadedFont *font);

#endif // FINAL_FONTLOADER_H

#if (defined(FINAL_FONTLOADER_IMPLEMENTATION) && !defined(FINAL_FONTLOADER_IMPLEMENTED)) || (FPL_IS_IDE)

#ifndef FINAL_FONTLOADER_IMPLEMENTED
#define FINAL_FONTLOADER_IMPLEMENTED
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#endif
#include <stb/stb_truetype.h>

fpl_extern Vec2f FontGetTextSize(const LoadedFont *font, const char *text, const size_t textLen, const float maxCharHeight) {
	if (font == fpl_null || text == fpl_null || textLen == 0) {
		return V2fZero();
	}

	float xwidth = 0.0f;
	float ymax = 0.0f;
	if (font->charCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		uint32_t lastChar = font->firstChar + (font->charCount - 1);
		for (uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = (uint8_t)text[textPos];
			uint32_t atNext = textPos < (textLen - 1) ? (uint8_t)text[textPos + 1] : 0;
			float xadvance;
			Vec2f offset = V2fInit(xpos, ypos);
			Vec2f size = V2fZero();
			if (at >= font->firstChar && at <= lastChar) {
				uint32_t codePoint = at - font->firstChar;
				const FontGlyph *glyph = font->glyphs + codePoint;
				size = glyph->charSize;
				offset = V2fAdd(offset, glyph->offset);
				offset = V2fAddMultScalar(offset, V2fInit(size.x, -size.y), 0.5f);
				xadvance = FontGetCharacterAdvance(font, (uint32_t)at, (uint32_t)atNext);
			} else {
				xadvance = font->info.spaceAdvance;
			}
			Vec2f min = offset;
			Vec2f max = V2fAdd(min, V2fInit(xadvance, size.y));
			xwidth += (max.x - min.x);
			ymax = fplMax(ymax, max.y - min.y);
			xpos += xadvance;
		}
	}
	Vec2f result = V2fMultScalar(V2fInit(xwidth, ymax), maxCharHeight);
	return(result);
}

fpl_extern Vec2f FontGetTextSizeCodepoints(const LoadedFont *font, const uint32_t *codepoints, const size_t count, const float maxCharHeight) {
	if (font == fpl_null || codepoints == fpl_null || count == 0) {
		return V2fZero();
	}

	float xwidth = 0.0f;
	float ymax = 0.0f;
	if (font->charCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		uint32_t lastChar = font->firstChar + (font->charCount - 1);
		for (size_t textPos = 0; textPos < count; ++textPos) {
			uint32_t at = codepoints[textPos];
			uint32_t atNext = (textPos < count - 1) ? codepoints[textPos + 1] : 0;
			float xadvance;
			Vec2f offset = V2fInit(xpos, ypos);
			Vec2f size = V2fZero();
			if (at >= font->firstChar && at <= lastChar) {
				uint32_t codePoint = at - font->firstChar;
				const FontGlyph *glyph = font->glyphs + codePoint;
				size = glyph->charSize;
				offset = V2fAdd(offset, glyph->offset);
				offset = V2fAddMultScalar(offset, V2fInit(size.x, -size.y), 0.5f);
				xadvance = FontGetCharacterAdvance(font, at, atNext);
			} else {
				xadvance = font->info.spaceAdvance;
			}
			Vec2f min = offset;
			Vec2f max = V2fAdd(min, V2fInit(xadvance, size.y));
			xwidth += (max.x - min.x);
			ymax = fplMax(ymax, max.y - min.y);
			xpos += xadvance;
		}
	}
	Vec2f result = V2fMultScalar(V2fInit(xwidth, ymax), maxCharHeight);
	return(result);
}

fpl_extern FontQuad FontGetQuad(const LoadedFont *font, const uint32_t codePoint, const float scale) {
	FontQuad empty = fplZeroInit;
	if (font == fpl_null) {
		return empty;
	}

	uint32_t lastChar = font->firstChar + (font->charCount - 1);
	if (codePoint < font->firstChar || codePoint > lastChar) {
		return empty;
	}

	FontQuad result = fplZeroInit;
	uint32_t index = codePoint - font->firstChar;
	const FontGlyph *glyph = &font->glyphs[index];
	Vec2f size = V2fMultScalar(glyph->charSize, scale);
	Vec2f offset = V2fZero();
	offset = V2fAddMultScalar(offset, glyph->offset, scale);
	offset = V2fAddMultScalar(offset, V2fInit(size.x, -size.y), 0.5f);
	result.offset = offset;
	result.size = size;
	result.uvMin = glyph->uvMin;
	result.uvMax = glyph->uvMax;
	return result;
}

fpl_extern FontQuadCorners FontGetQuadCorners(const LoadedFont *font, const uint32_t codePoint, const float scale, const bool cartesian) {
	FontQuadCorners empty = fplZeroInit;
	if (font == fpl_null) {
		return empty;
	}

	uint32_t lastChar = font->firstChar + (font->charCount - 1);
	if (codePoint < font->firstChar || codePoint > lastChar) {
		return empty;
	}

	uint32_t index = codePoint - font->firstChar;
	const FontGlyph *glyph = &font->glyphs[index];

	// fontloader's glyph->offset.y is Y-up by construction (top has higher Y than bottom).
	float x0 = glyph->offset.x * scale;
	float y0 = glyph->offset.y * scale;
	float x1 = x0 + glyph->charSize.x * scale;
	float y1 = y0 - glyph->charSize.y * scale;

	// Flip Y for top-down screens where Y+ points down.
	if (!cartesian) {
		y0 = -y0;
		y1 = -y1;
	}

	FontQuadCorners result = fplZeroInit;
	result.charCode = glyph->charCode;
	result.advance = font->defaultAdvance != fpl_null ? font->defaultAdvance[index] * scale : 0.0f;

	// Order: TR, TL, BL, BR
	result.offset[0] = V2fInit(x1, y0);
	result.offset[1] = V2fInit(x0, y0);
	result.offset[2] = V2fInit(x0, y1);
	result.offset[3] = V2fInit(x1, y1);

	float uMin = glyph->uvMin.x;
	float uMax = glyph->uvMax.x;
	float vMin = glyph->uvMin.y;
	float vMax = glyph->uvMax.y;
	result.uv[0] = V2fInit(uMax, vMax);
	result.uv[1] = V2fInit(uMin, vMax);
	result.uv[2] = V2fInit(uMin, vMin);
	result.uv[3] = V2fInit(uMax, vMin);

	return result;
}

fpl_extern float FontGetCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint) {
	if (font == fpl_null) {
		return 0.0f;
	}

	uint32_t lastCharPastOne = font->firstChar + font->charCount;
	if (thisCodePoint < font->firstChar || thisCodePoint >= lastCharPastOne) {
		return 0.0f;
	}

	uint32_t thisIndex = thisCodePoint - font->firstChar;
	float result = font->defaultAdvance[thisIndex];
	if (font->hasKerningTable) {
		// Bug fix: original used (firstChar - charCount), which underflowed for any nextCodePoint > 0.
		if (nextCodePoint >= font->firstChar && nextCodePoint < lastCharPastOne) {
			uint32_t nextIndex = nextCodePoint - font->firstChar;
			float kerning = font->kerningTable[thisIndex * font->charCount + nextIndex];
			result += kerning;
		}
	}
	return(result);
}

fpl_extern int FontFindAtlasForCodepoint(const LoadedFont *fonts, const size_t fontCount, const uint32_t codePoint) {
	if (fonts == fpl_null) {
		return -1;
	}
	for (size_t i = 0; i < fontCount; ++i) {
		if (fonts[i].charCount == 0) {
			continue;
		}
		uint32_t lastChar = fonts[i].firstChar + (fonts[i].charCount - 1);
		if (codePoint >= fonts[i].firstChar && codePoint <= lastChar) {
			return (int)i;
		}
	}
	return -1;
}

fpl_extern Vec2f FontGetTextSizeMulti(const LoadedFont *fonts, const size_t fontCount, const uint32_t *codepoints, const size_t count, const float maxCharHeight) {
	if (fonts == fpl_null || fontCount == 0 || codepoints == fpl_null || count == 0) {
		return V2fZero();
	}

	float xwidth = 0.0f;
	float ymax = 0.0f;
	float xpos = 0.0f;
	float ypos = 0.0f;
	for (size_t textPos = 0; textPos < count; ++textPos) {
		uint32_t at = codepoints[textPos];
		uint32_t atNext = (textPos < count - 1) ? codepoints[textPos + 1] : 0;

		int atlasIndex = FontFindAtlasForCodepoint(fonts, fontCount, at);
		const LoadedFont *font = (atlasIndex >= 0) ? &fonts[atlasIndex] : &fonts[0];
		if (font->charCount == 0) {
			continue;
		}

		float xadvance;
		Vec2f offset = V2fInit(xpos, ypos);
		Vec2f size = V2fZero();
		uint32_t lastChar = font->firstChar + (font->charCount - 1);
		if (at >= font->firstChar && at <= lastChar) {
			uint32_t codePoint = at - font->firstChar;
			const FontGlyph *glyph = font->glyphs + codePoint;
			size = glyph->charSize;
			offset = V2fAdd(offset, glyph->offset);
			offset = V2fAddMultScalar(offset, V2fInit(size.x, -size.y), 0.5f);
			xadvance = FontGetCharacterAdvance(font, at, atNext);
		} else {
			xadvance = fonts[0].info.spaceAdvance;
		}
		Vec2f min = offset;
		Vec2f max = V2fAdd(min, V2fInit(xadvance, size.y));
		xwidth += (max.x - min.x);
		ymax = fplMax(ymax, max.y - min.y);
		xpos += xadvance;
	}
	Vec2f result = V2fMultScalar(V2fInit(xwidth, ymax), maxCharHeight);
	return(result);
}

fpl_internal void InternalFontLoadFromMemoryShutdown(MemoryAllocator *allocator, void *packedChars, uint8_t *atlasAlphaBitmap, FontGlyph *glyphs, float *kerningTable, float *defaultAdvance) {
	if (packedChars != fpl_null) {
		MemoryAllocatorFree(allocator, packedChars);
	}
	if (atlasAlphaBitmap != fpl_null) {
		MemoryAllocatorFree(allocator, atlasAlphaBitmap);
	}
	if (glyphs != fpl_null) {
		MemoryAllocatorFree(allocator, glyphs);
	}
	if (kerningTable != fpl_null) {
		MemoryAllocatorFree(allocator, kerningTable);
	}
	if (defaultAdvance != fpl_null) {
		MemoryAllocatorFree(allocator, defaultAdvance);
	}
}

fpl_extern bool FontLoadFromMemoryEx(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, const FontQuality quality, LoadedFont *outFont) {
	if (data == fpl_null || dataSize == 0 || fontSize <= 0.0f || firstChar > lastChar || atlasWidth == 0 || atlasHeight == 0 || outFont == fpl_null) {
		return false;
	}

	const bool usePacked = (quality != FontQuality_Baked);

	float *defaultAdvance = fpl_null;
	float *kerningTable = fpl_null;
	FontGlyph *glyphs = fpl_null;
	uint8_t *atlasAlphaBitmap = fpl_null;
	void *packedChars = fpl_null;

	stbtt_fontinfo fontInfo = fplZeroInit;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char *)data, fontIndex);

	if (!stbtt_InitFont(&fontInfo, (const unsigned char *)data, fontOffset)) {
		InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
		return false;
	}

	const uint32_t charCount = (lastChar - firstChar) + 1;
	const size_t glyphsSize = sizeof(FontGlyph) * charCount;
	const size_t kerningTableSize = loadKerning ? sizeof(float) * charCount * charCount : 0;
	const size_t defaultAdvanceSize = charCount * sizeof(float);

	defaultAdvance = (float *)MemoryAllocatorAlloc(allocator, defaultAdvanceSize);
	if (defaultAdvance == fpl_null) {
		InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
		return false;
	}

	if (loadKerning) {
		kerningTable = (float *)MemoryAllocatorAlloc(allocator, kerningTableSize);
		if (kerningTable == fpl_null) {
			InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
			return false;
		}
		// Only the pairs the font actually kerns are written below, so the rest has to be zero already. The
		// default allocator hands back cleared pages, a custom MemoryAllocator promises nothing.
		fplMemoryClear(kerningTable, kerningTableSize);
	}

	glyphs = (FontGlyph *)MemoryAllocatorAlloc(allocator, glyphsSize);
	if (glyphs == fpl_null) {
		InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
		return false;
	}

	atlasAlphaBitmap = (uint8_t *)MemoryAllocatorAlloc(allocator, atlasWidth * atlasHeight);
	if (atlasAlphaBitmap == fpl_null) {
		InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
		return false;
	}

	if (usePacked) {
		packedChars = MemoryAllocatorAlloc(allocator, charCount * sizeof(stbtt_packedchar));
		if (packedChars == fpl_null) {
			InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
			return false;
		}

		int oversampleH = 1, oversampleV = 1;
		if (quality == FontQuality_Packed2x) {
			oversampleH = 2;
			oversampleV = 2;
		} else if (quality == FontQuality_Packed4x) {
			oversampleH = 4;
			oversampleV = 4;
		}

		stbtt_pack_range characterRange = fplZeroInit;
		characterRange.font_size = fontSize;
		characterRange.num_chars = charCount;
		characterRange.first_unicode_codepoint_in_range = firstChar;
		characterRange.chardata_for_range = (stbtt_packedchar *)packedChars;

		stbtt_pack_context packContext = fplZeroInit;
		stbtt_PackBegin(&packContext, atlasAlphaBitmap, atlasWidth, atlasHeight, atlasWidth, 0, fpl_null);
		stbtt_PackSetOversampling(&packContext, oversampleH, oversampleV);
		stbtt_PackFontRange(&packContext, (const unsigned char *)data, fontIndex, fontSize, firstChar, charCount, (stbtt_packedchar *)packedChars);
		stbtt_PackEnd(&packContext);
	} else {
		packedChars = MemoryAllocatorAlloc(allocator, charCount * sizeof(stbtt_bakedchar));
		if (packedChars == fpl_null) {
			InternalFontLoadFromMemoryShutdown(allocator, packedChars, atlasAlphaBitmap, glyphs, kerningTable, defaultAdvance);
			return false;
		}
		stbtt_BakeFontBitmap((const unsigned char *)data, fontOffset, fontSize, atlasAlphaBitmap, atlasWidth, atlasHeight, firstChar, charCount, (stbtt_bakedchar *)packedChars);
	}

	int ascentRaw, descentRaw, lineGapRaw;
	int spaceAdvanceRaw, spaceLeftSideBearing;
	stbtt_GetFontVMetrics(&fontInfo, &ascentRaw, &descentRaw, &lineGapRaw);
	stbtt_GetCodepointHMetrics(&fontInfo, ' ', &spaceAdvanceRaw, &spaceLeftSideBearing);

	float texelU = 1.0f / (float)atlasWidth;
	float texelV = 1.0f / (float)atlasHeight;
	float pixelsToUnits = 1.0f / fontSize;
	float scaleToPixels = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

	float spaceAdvancePx = spaceAdvanceRaw * scaleToPixels;
	float ascentPx = fabsf((float)ascentRaw) * scaleToPixels;
	float descentPx = fabsf((float)descentRaw) * scaleToPixels;
	float lineGapPx = lineGapRaw * scaleToPixels;
	float lineHeightPx = ascentPx + descentPx + lineGapPx;

	// packedchar/bakedchar share x0/y0/x1/y1 (atlas coords, possibly oversampled), xoff/yoff/xadvance (screen pixels).
	// Only packedchar has xoff2/yoff2 — for bakedchar derive them from atlas size (no oversampling, so atlas pixels = screen pixels).
	for (uint32_t glyphIndex = 0; glyphIndex < charCount; ++glyphIndex) {
		int x0, y0, x1, y1;
		float xoff, yoff, xoff2, yoff2, xadvance;
		if (usePacked) {
			const stbtt_packedchar *src = ((const stbtt_packedchar *)packedChars) + glyphIndex;
			x0 = src->x0; y0 = src->y0; x1 = src->x1; y1 = src->y1;
			xoff = src->xoff; yoff = src->yoff;
			xoff2 = src->xoff2; yoff2 = src->yoff2;
			xadvance = src->xadvance;
		} else {
			const stbtt_bakedchar *src = ((const stbtt_bakedchar *)packedChars) + glyphIndex;
			x0 = src->x0; y0 = src->y0; x1 = src->x1; y1 = src->y1;
			xoff = src->xoff; yoff = src->yoff;
			xoff2 = src->xoff + (float)(src->x1 - src->x0);
			yoff2 = src->yoff + (float)(src->y1 - src->y0);
			xadvance = src->xadvance;
		}

		FontGlyph *destInfo = glyphs + glyphIndex;
		destInfo->charCode = firstChar + glyphIndex;

		float uMin = x0 * texelU;
		float uMax = x1 * texelU;
		float vMin = y1 * texelV;
		float vMax = y0 * texelV;
		destInfo->uvMin = V2fInit(uMin, vMin);
		destInfo->uvMax = V2fInit(uMax, vMax);

		// Screen-pixel size from xoff/xoff2 so oversampled atlases do not inflate the on-screen quad.
		float charWidthPx = xoff2 - xoff;
		float charHeightPx = yoff2 - yoff;
		destInfo->charSize = V2fMultScalar(V2fInit(charWidthPx, charHeightPx), pixelsToUnits);

		destInfo->offset = V2fMultScalar(V2fInit(xoff, -yoff), pixelsToUnits);

		defaultAdvance[glyphIndex] = xadvance * pixelsToUnits;
	}

	if (loadKerning) {
		// One entry per ordered pair, in the same FONT UNITS as defaultAdvance above - FontGetCharacterAdvance
		// adds the two together, so anything else here does not even have a meaning. The raw value out of the
		// font is in design units, which take the same two steps to get there as every other metric in this
		// function: times scaleToPixels for pixels, times pixelsToUnits for units.
		// Bug fix: was (charIndex < lastChar) and (nextCharIndex < lastChar), missing the last codepoint pair.
		for (uint32_t charIndex = firstChar; charIndex <= lastChar; ++charIndex) {
			for (uint32_t nextCharIndex = firstChar; nextCharIndex <= lastChar; ++nextCharIndex) {
				if (nextCharIndex == charIndex) {
					continue;
				}
				int rawKerning = stbtt_GetCodepointKernAdvance(&fontInfo, (int)charIndex, (int)nextCharIndex);
				if (rawKerning == 0) {
					continue;
				}
				float kerningPx = (float)rawKerning * scaleToPixels;
				float kerning = kerningPx * pixelsToUnits;
				uint32_t a = (uint32_t)(charIndex - firstChar);
				uint32_t b = (uint32_t)(nextCharIndex - firstChar);
				kerningTable[a * charCount + b] = kerning;
			}
		}
	}

	fplClearStruct(outFont);
	outFont->firstChar = firstChar;
	outFont->charCount = charCount;
	outFont->info.ascent = ascentPx * pixelsToUnits;
	outFont->info.descent = descentPx * pixelsToUnits;
	outFont->info.lineHeight = lineHeightPx * pixelsToUnits;
	outFont->info.spaceAdvance = spaceAdvancePx * pixelsToUnits;
	outFont->glyphs = glyphs;
	outFont->kerningTable = kerningTable;
	outFont->hasKerningTable = loadKerning;
	outFont->defaultAdvance = defaultAdvance;
	outFont->atlasAlphaBitmap = atlasAlphaBitmap;
	outFont->atlasWidth = atlasWidth;
	outFont->atlasHeight = atlasHeight;

	MemoryAllocatorFree(allocator, packedChars);

	return true;
}

fpl_extern bool FontLoadFromMemory(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
#if FINAL_FONTLOADER_BETTERQUALITY
	const FontQuality defaultQuality = FontQuality_Packed4x;
#else
	const FontQuality defaultQuality = FontQuality_Baked;
#endif
	return FontLoadFromMemoryEx(allocator, data, dataSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, defaultQuality, outFont);
}

fpl_internal void InternalFontLoadFromFileShutdown(MemoryAllocator *allocator, fplFileHandle *file, uint8_t *ttfBuffer) {
	if (ttfBuffer != fpl_null) {
		MemoryAllocatorFree(allocator, ttfBuffer);
	}
	if (file->isValid) {
		fplFileClose(file);
	}
}

fpl_extern bool FontLoadFromFileEx(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, const FontQuality quality, LoadedFont *outFont) {
	if (fplGetStringLength(filePath) == 0 || fontSize <= 0.0f || firstChar > lastChar || atlasWidth == 0 || atlasHeight == 0 || outFont == fpl_null) {
		return false;
	}

	uint8_t *ttfBuffer = fpl_null;
	fplFileHandle file = fplZeroInit;

	if (!fplFileOpenBinary(filePath, &file)) {
		InternalFontLoadFromFileShutdown(allocator, &file, ttfBuffer);
		return false;
	}

	size_t ttfBufferSize = fplFileGetSizeFromHandle(&file);

	ttfBuffer = (uint8_t *)MemoryAllocatorAlloc(allocator, ttfBufferSize);
	if (ttfBuffer == fpl_null) {
		InternalFontLoadFromFileShutdown(allocator, &file, ttfBuffer);
		return false;
	}

	size_t read = fplFileReadBlock(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
	if (read != ttfBufferSize) {
		InternalFontLoadFromFileShutdown(allocator, &file, ttfBuffer);
		return false;
	}

	fplFileClose(&file);

	if (!FontLoadFromMemoryEx(allocator, ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, quality, outFont)) {
		InternalFontLoadFromFileShutdown(allocator, &file, ttfBuffer);
		return false;
	}

	InternalFontLoadFromFileShutdown(allocator, &file, ttfBuffer);
	return true;
}

fpl_extern bool FontLoadFromFile(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
#if FINAL_FONTLOADER_BETTERQUALITY
	const FontQuality defaultQuality = FontQuality_Packed4x;
#else
	const FontQuality defaultQuality = FontQuality_Baked;
#endif
	return FontLoadFromFileEx(allocator, filePath, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, defaultQuality, outFont);
}

fpl_extern void FontFree(MemoryAllocator *allocator, LoadedFont *font) {
	if (font == fpl_null) {
		return;
	}
	if (font->kerningTable != fpl_null) {
		MemoryAllocatorFree(allocator, font->kerningTable);
	}
	if (font->defaultAdvance != fpl_null) {
		MemoryAllocatorFree(allocator, font->defaultAdvance);
	}
	if (font->glyphs != fpl_null) {
		MemoryAllocatorFree(allocator, font->glyphs);
	}
	if (font->atlasAlphaBitmap != fpl_null) {
		MemoryAllocatorFree(allocator, font->atlasAlphaBitmap);
	}
	fplClearStruct(font);
}

#endif // FINAL_FONTLOADER_IMPLEMENTATION && !FINAL_FONTLOADER_IMPLEMENTED
