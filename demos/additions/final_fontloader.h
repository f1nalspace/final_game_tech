/*
Name:
	Final Font Loader

Description:
	Simple font loader using STB_truetype.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete

Changelog:
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

#ifndef FINAL_FONTLOADER_BETTERQUALITY
#define FINAL_FONTLOADER_BETTERQUALITY 0
#endif

#include <final_core.h>
#include <final_math.h>

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

fpl_extern Vec2f FontGetTextSize(const LoadedFont *font, const char *text, const size_t textLen, const float maxCharHeight);
fpl_extern FontQuad FontGetQuad(const LoadedFont *font, const uint32_t codePoint, const float scale);
fpl_extern float FontGetCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint);
fpl_extern bool FontLoadFromFile(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
fpl_extern bool FontLoadFromMemory(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
fpl_extern void FontFree(MemoryAllocator *allocator, LoadedFont *font);

#endif // FINAL_FONTLOADER_H

#if (defined(FINAL_FONTLOADER_IMPLEMENTATION) && !defined(FINAL_FONTLOADER_IMPLEMENTED)) || (FPL_IS_IDE)

#ifndef FINAL_FONTLOADER_IMPLEMENTED
#define FINAL_FONTLOADER_IMPLEMENTED
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

fpl_extern Vec2f FontGetTextSize(const LoadedFont *font, const char *text, const size_t textLen, const float maxCharHeight) {
	if (font == fpl_null || text == fpl_null || textLen == 0) {
		return V2fZero();
	}

	float xwidth = 0.0f;
	float ymax = 0.0f;
	if(font != fpl_null && font->charCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		uint32_t lastChar = font->firstChar + (font->charCount - 1);
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];
			uint32_t atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float xadvance;
			Vec2f offset = V2fInit(xpos, ypos);
			Vec2f size = V2fZero();
			if(at >= font->firstChar && at <= lastChar) {
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
			ymax = fplMax(ymax, max.y - min.h);
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

fpl_extern float FontGetCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint) {
	if (font == fpl_null) {
		return 0.0f;
	}

	uint32_t lastChar = font->firstChar + (font->charCount - 1);
	if (thisCodePoint < font->firstChar || thisCodePoint > lastChar) {
		return 0.0f;
	}

	float result = 0;
	uint32_t thisIndex = thisCodePoint - font->firstChar;
	const FontGlyph *glyph = font->glyphs + thisIndex;
	result = font->defaultAdvance[thisIndex];
	if(font->hasKerningTable) {
		if(nextCodePoint >= font->firstChar && nextCodePoint < (font->firstChar - font->charCount)) {
			uint32_t nextIndex = nextCodePoint - font->firstChar;
			float kerning = font->kerningTable[thisIndex * font->charCount + nextIndex];
			result += kerning;
		}
	}
	return(result);
}

fpl_extern bool FontLoadFromMemory(MemoryAllocator *allocator, const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
	if(data == fpl_null || dataSize == 0 || fontSize <= 0.0f || firstChar > lastChar || atlasWidth == 0 || atlasHeight == 0 || outFont == fpl_null) {
		// TODO(final): Logging (Invalid arguments)
		return false;
	}

	bool result = false;

	float *defaultAdvance = fpl_null;
	float *kerningTable = fpl_null;
	FontGlyph *glyphs = fpl_null;
	uint8_t *atlasAlphaBitmap = fpl_null;

#if FINAL_FONTLOADER_BETTERQUALITY
	stbtt_packedchar *packedChars = fpl_null;
#else
	stbtt_bakedchar *packedChars = fpl_null;
#endif

	stbtt_fontinfo fontInfo = fplZeroInit;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char *)data, fontIndex);

	if(!stbtt_InitFont(&fontInfo, (const unsigned char *)data, fontOffset)) {
		// TODO(final): Logging (Failed to load with STB_TrueType)
		goto failed;
	}

	const uint32_t charCount = (lastChar - firstChar) + 1;
	const size_t glyphsSize = sizeof(FontGlyph) * charCount;
	const size_t kerningTableSize = loadKerning ? sizeof(float) * charCount * charCount : 0;
	const size_t defaultAdvanceSize = charCount * sizeof(float);

	defaultAdvance = (float *)MemoryAllocatorAlloc(allocator, defaultAdvanceSize);
	if (defaultAdvance == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

	if (loadKerning) {
		kerningTable = (float *)MemoryAllocatorAlloc(allocator, kerningTableSize);
		if (kerningTable == fpl_null) {
			// TODO(final): Logging (Insufficient memory)
			goto failed;
		}
	}

	glyphs = (FontGlyph *)MemoryAllocatorAlloc(allocator, glyphsSize);
	if (glyphs == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

	atlasAlphaBitmap = (uint8_t *)MemoryAllocatorAlloc(allocator, atlasWidth * atlasHeight);
	if (atlasAlphaBitmap == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

#if FINAL_FONTLOADER_BETTERQUALITY
	packedChars = (stbtt_packedchar *)MemoryAllocatorAlloc(charCount * sizeof(stbtt_packedchar));
	if (packedChars == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

	stbtt_pack_range characterRange = fplZeroInit;
	characterRange.font_size = fontSize;
	characterRange.num_chars = charCount;
	characterRange.first_unicode_codepoint_in_range = firstChar;
	characterRange.chardata_for_range = packedChars;

	stbtt_pack_context packContext = fplZeroInit;
	stbtt_pack_range *ranges[1] = {
		&characterRange,
	};
	stbtt_PackSetOversampling(&packContext, 4, 4);
	stbtt_PackBegin(&packContext, atlasAlphaBitmap, atlasWidth, atlasHeight, atlasWidth, 0, 0);
	stbtt_PackFontRanges(&packContext, (const unsigned char *)data, fontIndex, ranges[0], 1);
	stbtt_PackEnd(&packContext);
#else
	packedChars = (stbtt_bakedchar *)MemoryAllocatorAlloc(allocator, charCount * sizeof(stbtt_bakedchar));
	if (packedChars == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}
	
	// TODO(final): Handle return value
	stbtt_BakeFontBitmap((const unsigned char *)data, fontOffset, fontSize, atlasAlphaBitmap, atlasWidth, atlasHeight, firstChar, charCount, packedChars);
#endif

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

	for(uint32_t glyphIndex = 0; glyphIndex < charCount; ++glyphIndex) {
#if FINAL_FONTLOADER_BETTERQUALITY
		stbtt_packedchar *sourceInfo = packedChars + glyphIndex;
#else
		stbtt_bakedchar *sourceInfo = packedChars + glyphIndex;
#endif

		FontGlyph *destInfo = glyphs + glyphIndex;
		destInfo->charCode = firstChar + glyphIndex;

		// Compute UV coords
		float uMin = sourceInfo->x0 * texelU;
		float uMax = sourceInfo->x1 * texelU;
		float vMin = sourceInfo->y1 * texelV;
		float vMax = sourceInfo->y0 * texelV;
		destInfo->uvMin = V2fInit(uMin, vMin);
		destInfo->uvMax = V2fInit(uMax, vMax);

		// Compute character size
		int charWidthInPixels = sourceInfo->x1 - sourceInfo->x0;
		int charHeightInPixels = sourceInfo->y1 - sourceInfo->y0;
		destInfo->charSize = V2fMultScalar(V2fInit((float)charWidthInPixels, (float)charHeightInPixels), pixelsToUnits);

		// Compute offset to start/baseline in units
		destInfo->offset = V2fMultScalar(V2fInit(sourceInfo->xoff, -sourceInfo->yoff), pixelsToUnits);
	}

	// Build kerning table & default advance table
	for(uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
		uint32_t codePointIndex = (uint32_t)(charIndex - firstChar);
#if FINAL_FONTLOADER_BETTERQUALITY
		stbtt_packedchar *leftInfo = packedChars + codePointIndex;
#else
		stbtt_bakedchar *leftInfo = packedChars + codePointIndex;
#endif
		defaultAdvance[codePointIndex] = leftInfo->xadvance * pixelsToUnits;

		if(loadKerning) {
			for(uint32_t nextCharIndex = charIndex + 1; nextCharIndex < lastChar; ++nextCharIndex) {
				float kerningPx = stbtt_GetCodepointKernAdvance(&fontInfo, charIndex, nextCharIndex) * pixelsToUnits;
				if(kerningPx != 0) {
					int widthPx = leftInfo->x1 - leftInfo->x0;
					if(widthPx > 0) {
						float kerning = kerningPx / (float)widthPx;
						uint32_t a = (uint32_t)(charIndex - firstChar);
						uint32_t b = (uint32_t)(nextCharIndex - firstChar);
						kerningTable[a * charCount + b] = kerning;
					}
				}
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

	result = true;
	goto done;

failed:
	if (packedChars != fpl_null) {
		MemoryAllocatorFree(allocator, packedChars);
	}
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
	result = false;

done:
	return result;
}

fpl_extern bool FontLoadFromFile(MemoryAllocator *allocator, const char *filePath, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
	if(fplGetStringLength(filePath) == 0 || fontSize <= 0.0f || firstChar > lastChar || atlasWidth == 0 || atlasHeight == 0 || outFont == fpl_null) {
		return false;
	}

	bool result = false;

	uint8_t *ttfBuffer = fpl_null;

	fplFileHandle file = fplZeroInit;

	if (!fplFileOpenBinary(filePath, &file)) {
		// TODO(final): Logging (File not found)
		goto failed;
	}

	size_t ttfBufferSize = fplFileGetSizeFromHandle(&file);

	ttfBuffer = (uint8_t *)MemoryAllocatorAlloc(allocator, ttfBufferSize);
	if (ttfBuffer == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

	size_t read = fplFileReadBlock(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
	if (read != ttfBufferSize) {
		// TODO(final): Logging (File read error)
		goto failed;
	}

	fplFileClose(&file);

	if (!FontLoadFromMemory(allocator, ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, outFont)) {
		// TODO(final): Logging (Failed loading font from memory)
		goto failed;
	}

	result = true;
	goto done;

failed:
	result = false;

done:
	if (ttfBuffer != fpl_null) {
		MemoryAllocatorFree(allocator, ttfBuffer);
	}
	if (file.isValid) {
		fplFileClose(&file);
	}
	return result;
}

fpl_extern void FontFree(MemoryAllocator *allocator, LoadedFont *font) {
	if (font == fpl_null) {
		return;
	}
	if(font->kerningTable != fpl_null) {
		MemoryAllocatorFree(allocator, font->kerningTable);
	}
	if(font->defaultAdvance != fpl_null) {
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