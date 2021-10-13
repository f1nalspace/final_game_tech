/*
Name:
	Final Font Loader

Description:
	Simple font loader using STB_truetype.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2021 Torsten Spaete

Changelog:
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

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

#ifndef FINAL_FONTLOADER_BETTERQUALITY
#define FINAL_FONTLOADER_BETTERQUALITY 0
#endif

#include <final_platform_layer.h>

#include "final_math.h"

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

inline const FontGlyph *GetFontGlyph(const LoadedFont *font, const uint32_t codePoint) {
	if(font == fpl_null) {
		return(fpl_null);
	}
	uint32_t lastCharPastOne = font->firstChar + font->charCount;
	fplAssert(codePoint >= font->firstChar && codePoint < lastCharPastOne);
	uint32_t charIndex = codePoint - font->firstChar;
	const FontGlyph *result = &font->glyphs[charIndex];
	return(result);
}

inline float GetFontAscent(const FontInfo *fontInfo) {
	float result = fontInfo->ascent;
	return(result);
}

inline float GetFontDescent(const FontInfo *fontInfo) {
	float result = fontInfo->descent;
	return(result);
}

inline float GetFontLineAdvance(const FontInfo *fontInfo) {
	float result = fontInfo->lineHeight;
	return(result);
}

extern Vec2f GetTextSize(const char *text, const size_t textLen, const LoadedFont *fontDesc, const float maxCharHeight);
extern FontQuad GetFontQuad(const LoadedFont *font, const uint32_t codePoint, const float scale);
extern float GetFontCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint);
extern bool LoadFontFromFile(const char *dataPath, const char *filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
extern bool LoadFontFromMemory(const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont);
extern void ReleaseFont(LoadedFont *font);

#endif // FINAL_FONTLOADER_H

#if defined(FINAL_FONTLOADER_IMPLEMENTATION) && !defined(FINAL_FONTLOADER_IMPLEMENTED)
#define FINAL_FONTLOADER_IMPLEMENTED

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

extern Vec2f GetTextSize(const char *text, const size_t textLen, const LoadedFont *fontDesc, const float maxCharHeight) {
	float xwidth = 0.0f;
	float ymax = 0.0f;
	if(fontDesc != nullptr && fontDesc->charCount > 0) {
		float xpos = 0.0f;
		float ypos = 0.0f;
		uint32_t lastChar = fontDesc->firstChar + (fontDesc->charCount - 1);
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			uint32_t at = text[textPos];
			uint32_t atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float xadvance;
			Vec2f offset = V2fInit(xpos, ypos);
			Vec2f size = V2fZero();
			if(at >= fontDesc->firstChar && at <= lastChar) {
				uint32_t codePoint = at - fontDesc->firstChar;
				const FontGlyph *glyph = fontDesc->glyphs + codePoint;
				size = glyph->charSize;
				offset += glyph->offset;
				offset += V2fInit(size.x, -size.y) * 0.5f;
				xadvance = GetFontCharacterAdvance(fontDesc, (uint32_t)at, (uint32_t)atNext);
			} else {
				xadvance = fontDesc->info.spaceAdvance;
			}
			Vec2f min = offset;
			Vec2f max = min + V2fInit(xadvance, size.y);
			xwidth += (max.x - min.x);
			ymax = fplMax(ymax, max.y - min.h);
			xpos += xadvance;
		}
	}
	Vec2f result = V2fInit(xwidth, ymax) * maxCharHeight;
	return(result);
}

extern FontQuad GetFontQuad(const LoadedFont *font, const uint32_t codePoint, const float scale) {
	FontQuad result = fplZeroInit;
	if(font != fpl_null) {
		uint32_t lastChar = font->firstChar + (font->charCount - 1);
		if(codePoint >= font->firstChar && codePoint <= lastChar) {
			uint32_t index = codePoint - font->firstChar;
			const FontGlyph *glyph = &font->glyphs[index];
			Vec2f size = glyph->charSize * scale;
			Vec2f offset = V2fZero();
			offset += glyph->offset * scale;
			offset += V2fInit(size.x, -size.y) * 0.5f;
			result.offset = offset;
			result.size = size;
			result.uvMin = glyph->uvMin;
			result.uvMax = glyph->uvMax;
		}
	}
	return(result);
}

extern float GetFontCharacterAdvance(const LoadedFont *font, const uint32_t thisCodePoint, const uint32_t nextCodePoint) {
	float result = 0;
	if(thisCodePoint >= font->firstChar && thisCodePoint < (font->firstChar - font->charCount)) {
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
	}
	return(result);
}

extern bool LoadFontFromMemory(const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
	if(data == fpl_null || dataSize == 0) {
		return false;
	}
	if(outFont == fpl_null) {
		return false;
	}

	fplClearStruct(outFont);

	stbtt_fontinfo fontInfo = fplZeroInit;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char *)data, fontIndex);

	bool result = false;
	if(!stbtt_InitFont(&fontInfo, (const unsigned char *)data, fontOffset)) {
		return(false);
	}

	uint32_t charCount = (lastChar - firstChar) + 1;
	uint8_t *atlasAlphaBitmap = (uint8_t *)fplMemoryAllocate(atlasWidth * atlasHeight);
#if FINAL_FONTLOADER_BETTERQUALITY
	stbtt_packedchar *packedChars = (stbtt_packedchar *)fplMemoryAllocate(charCount * sizeof(stbtt_packedchar));

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
	stbtt_bakedchar *packedChars = (stbtt_bakedchar *)fplMemoryAllocate(charCount * sizeof(stbtt_bakedchar));
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

	size_t glyphsSize = sizeof(FontGlyph) * charCount;
	FontGlyph *glyphs = (FontGlyph *)fplMemoryAllocate(glyphsSize);

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
		destInfo->charSize = V2fInit((float)charWidthInPixels, (float)charHeightInPixels) * pixelsToUnits;

		// Compute offset to start/baseline in units
		destInfo->offset = V2fInit(sourceInfo->xoff, -sourceInfo->yoff) * pixelsToUnits;
	}

	// Build kerning table & default advance table
	size_t kerningTableSize;
	float *kerningTable;
	if(loadKerning) {
		kerningTableSize = sizeof(float) * charCount * charCount;
		kerningTable = (float *)fplMemoryAllocate(kerningTableSize);
	} else {
		kerningTableSize = 0;
		kerningTable = fpl_null;
	}

	size_t defaultAdvanceSize = charCount * sizeof(float);
	float *defaultAdvance = (float *)fplMemoryAllocate(defaultAdvanceSize);
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

	return(result);
}

extern bool LoadFontFromFile(const char *dataPath, const char *filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, const bool loadKerning, LoadedFont *outFont) {
	if(filename == fpl_null) {
		return false;
	}
	if(outFont == fpl_null) {
		return false;
	}

	char filePath[FPL_MAX_PATH_LENGTH];
	if(dataPath != fpl_null) {
		fplCopyString(dataPath, filePath, fplArrayCount(filePath));
		fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);
	} else {
		fplCopyString(filename, filePath, fplArrayCount(filePath));
	}

	bool result = false;

	fplFileHandle file;
	uint8_t *ttfBuffer = fpl_null;
	uint32_t ttfBufferSize = 0;
	if(fplFileOpenBinary(filePath, &file)) {
		ttfBufferSize = fplFileGetSizeFromHandle32(&file);
		ttfBuffer = (uint8_t *)fplMemoryAllocate(ttfBufferSize);
		fplFileReadBlock32(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
		fplFileClose(&file);
	}

	if(ttfBuffer != nullptr) {
		result = LoadFontFromMemory(ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, loadKerning, outFont);
		fplMemoryFree(ttfBuffer);
	}
	return(result);
}

extern void ReleaseFont(LoadedFont *font) {
	if(font != fpl_null) {
		if(font->hasKerningTable) {
			fplMemoryFree(font->kerningTable);
		}
		fplMemoryFree(font->glyphs);
		fplMemoryFree(font->atlasAlphaBitmap);
		fplClearStruct(font);
	}
}

#endif // FINAL_FONTLOADER_IMPLEMENTATION && !FINAL_FONTLOADER_IMPLEMENTED