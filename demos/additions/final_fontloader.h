/*
Name:
	Final Font Loader
Description:
	Simple font loader using STB_truetype.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_FONTLOADER_H
#define FINAL_FONTLOADER_H

#ifndef FINAL_FONTLOADER_BETTERQUALITY
#define FINAL_FONTLOADER_BETTERQUALITY 0
#endif

#include <final_platform_layer.h>

#include <math.h> // fabsf

typedef struct FontVec2f {
	float x;
	float y;
} FontVec2f;

inline FontVec2f MakeFontVec2f(const float x, const float y) {
	FontVec2f result;
	result.x = x;
	result.y = y;
	return(result);
}

typedef struct FontGlyph {
	FontVec2f alignPercentage;
	FontVec2f uvMin, uvMax;
	FontVec2f charSize;
	uint32_t charCode;
} FontGlyph;

struct FontInfo {
	float ascent;
	float descent;
	float lineHeight;
	float baseline;
	float spaceAdvance;
};

struct LoadedFont {
	uint8_t *atlasAlphaBitmap;
	FontGlyph *glyphs;
	uint32_t atlasWidth;
	uint32_t atlasHeight;
	uint32_t firstChar;
	uint32_t charCount;
	FontInfo info;
	float *defaultAdvance;
	float *kerningTable;
};

inline float GetFontBaseline(const FontInfo *fontInfo) {
	float result = fontInfo->baseline;
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

extern float GetTextWidth(const char *text, const size_t textLen, const LoadedFont *font, const float maxCharHeight);
extern float GetFontCharacterAdvance(const LoadedFont *font, const uint32_t *codePoint, const uint32_t *nextCodePoint);
extern bool LoadFontFromFile(const char *dataPath, const char *filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, LoadedFont *outFont);
extern bool LoadFontFromMemory(const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, LoadedFont *outFont);
extern void ReleaseFont(LoadedFont *font);

#endif // FINAL_FONTLOADER_H

#if defined(FINAL_FONTLOADER_IMPLEMENTATION) && !defined(FINAL_FONTLOADER_IMPLEMENTED)
#define FINAL_FONTLOADER_IMPLEMENTED

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

extern float GetTextWidth(const char *text, const size_t textLen, const LoadedFont *font, const float maxCharHeight) {
	float result = 0;
	if(font != nullptr) {
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float advance;
			uint32_t codePoint = at - font->firstChar;
			if(codePoint >= 0 && codePoint < font->charCount) {
				uint32_t nextCodePoint = (atNext > 0) ? atNext - font->firstChar : 0;
				advance = GetFontCharacterAdvance(font, &codePoint, (atNext > 0) ? &nextCodePoint : nullptr) * maxCharHeight;
			} else {
				advance = font->info.spaceAdvance * maxCharHeight;
			}
			result += advance;
		}
	}
	return(result);
}

extern float GetFontCharacterAdvance(const LoadedFont *font, const uint32_t *codePoint, const uint32_t *nextCodePoint) {
	float result = 0;
	if(codePoint) {
		const FontGlyph *glyph = font->glyphs + *codePoint;
		result = font->defaultAdvance[*codePoint];
		if(nextCodePoint != nullptr) {
			float kerning = font->kerningTable[*codePoint * font->charCount + *nextCodePoint];
			result += kerning;
		}
	}
	return(result);
}

extern bool LoadFontFromMemory(const void *data, const size_t dataSize, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t  atlasWidth, const uint32_t atlasHeight, LoadedFont *outFont) {
	if(data == fpl_null || dataSize == 0) {
		return false;
	}
	if(outFont == fpl_null) {
		return false;
	}

	FPL_CLEAR_STRUCT(outFont);

	stbtt_fontinfo fontInfo = FPL_ZERO_INIT;
	int fontOffset = stbtt_GetFontOffsetForIndex((const unsigned char *)data, fontIndex);

	bool result = false;
	if(stbtt_InitFont(&fontInfo, (const unsigned char *)data, fontOffset)) {
		uint32_t charCount = (lastChar - firstChar) + 1;
		uint8_t *atlasAlphaBitmap = (uint8_t *)fplMemoryAllocate(atlasWidth * atlasHeight);
#if FINAL_FONTLOADER_BETTERQUALITY
		stbtt_packedchar *packedChars = (stbtt_packedchar *)fplMemoryAllocate(charCount * sizeof(stbtt_packedchar));

		stbtt_pack_range characterRange = FPL_ZERO_INIT;
		characterRange.font_size = fontSize;
		characterRange.num_chars = charCount;
		characterRange.first_unicode_codepoint_in_range = firstChar;
		characterRange.chardata_for_range = packedChars;

		stbtt_pack_context packContext = FPL_ZERO_INIT;
		stbtt_pack_range *ranges[1] = {
			&characterRange,
		};
		stbtt_PackBegin(&packContext, atlasAlphaBitmap, atlasWidth, atlasHeight, atlasWidth, 0, 0);
		stbtt_PackSetOversampling(&packContext, 1, 1);
		stbtt_PackFontRanges(&packContext, (const unsigned char *)data, fontIndex, ranges[0], 1);
		stbtt_PackEnd(&packContext);
#else
		stbtt_bakedchar *packedChars = (stbtt_bakedchar *)fplMemoryAllocate((charCount + 1) * sizeof(stbtt_bakedchar));
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
		float pixelScale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);
		float fontScale = 1.0f / fontSize;

		// Space advance in pixels
		float spaceAdvancePx = spaceAdvanceRaw * pixelScale;

		// Ascent height from the baseline in pixels
		float ascentPx = fabsf((float)ascentRaw) * pixelScale;

		// Descent height from the baseline in pixels
		float descentPx = fabsf((float)descentRaw) * pixelScale;

		// Max height is always ascent + descent
		float heightPx = ascentPx + descentPx;
		//assert(heightPx * fontScale == 1.0f);

		// Calculate line height
		float lineGapPx = lineGapRaw * pixelScale;
		float lineHeightPx = ascentPx + descentPx + lineGapPx;

		// Correction to center the font to the middle (baseline will not be in the middle!)
		float verticalCenterCorrectionPx = (0 + descentPx) - (heightPx * 0.5f);

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
			destInfo->uvMin = MakeFontVec2f(uMin, vMin);
			destInfo->uvMax = MakeFontVec2f(uMax, vMax);

			// Compute character size
			int charWidthInPixels = (sourceInfo->x1 - sourceInfo->x0) + 1;
			int charHeightInPixels = (sourceInfo->y1 - sourceInfo->y0) + 1;
			destInfo->charSize = MakeFontVec2f((float)charWidthInPixels * fontScale, (float)charHeightInPixels * fontScale);

			// Move the character half the size to the right - so we are left aligned
			float xoffset = destInfo->charSize.x * 0.5f;
			destInfo->alignPercentage.x = xoffset / destInfo->charSize.x;

			// Move down the character so that its top aligned
			// Move up the character to the baseline by the given y-offset (y-offset is negative, so we subtract it instead of adding)
			float halfHeight = destInfo->charSize.y * 0.5f;
			float baselineOffset = sourceInfo->yoff * fontScale;
			float yoffset = verticalCenterCorrectionPx * fontScale - halfHeight - baselineOffset;
			destInfo->alignPercentage.y = yoffset / destInfo->charSize.y;
		}

		// Build kerning table
		size_t kerningTableSize = sizeof(float) * charCount * charCount;
		float *kerningTable = (float *)fplMemoryAllocate(kerningTableSize);
		for(uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
			int packetCharIndex = charIndex - firstChar;
#if FINAL_FONTLOADER_BETTERQUALITY
			stbtt_packedchar *leftInfo = packedChars + packetCharIndex;
#else
			stbtt_bakedchar *leftInfo = packedChars + packetCharIndex;
#endif
			for(uint32_t nextCharIndex = charIndex + 1; nextCharIndex < lastChar; ++nextCharIndex) {
				float kerningPx = stbtt_GetCodepointKernAdvance(&fontInfo, charIndex, nextCharIndex) * pixelScale;
				if(kerningPx != 0) {
					int widthPx = leftInfo->x1 - leftInfo->x0 + 1;
                    float kerning = kerningPx / (float)widthPx;
                    uint32_t a = (uint32_t) (charIndex - firstChar);
                    uint32_t b = (uint32_t) (nextCharIndex - firstChar);
                    kerningTable[a * charCount + b] = kerning;
				}
			}
		}

		// Build default advance table
		size_t defaultAdvanceSize = charCount * sizeof(float);
		float *defaultAdvance = (float *)fplMemoryAllocate(defaultAdvanceSize);
		for(uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
			int advanceRaw, leftSideBearing;
			stbtt_GetCodepointHMetrics(&fontInfo, charIndex, &advanceRaw, &leftSideBearing);
			uint32_t index = (uint32_t)(charIndex - firstChar);
			defaultAdvance[index] = (advanceRaw * pixelScale) * fontScale;
		}

		outFont->firstChar = firstChar;
		outFont->charCount = charCount;
		outFont->info.ascent = ascentPx * fontScale;
		outFont->info.descent = descentPx * fontScale;
		outFont->info.baseline = verticalCenterCorrectionPx * fontScale;
		outFont->info.lineHeight = lineHeightPx * fontScale;
		outFont->info.spaceAdvance = spaceAdvancePx * fontScale;
		outFont->glyphs = glyphs;
		outFont->kerningTable = kerningTable;
		outFont->defaultAdvance = defaultAdvance;
		outFont->atlasAlphaBitmap = atlasAlphaBitmap;
		outFont->atlasWidth = atlasWidth;
		outFont->atlasHeight = atlasHeight;

		result = true;
	}

	return(result);
}

extern bool LoadFontFromFile(const char *dataPath, const char *filename, const uint32_t fontIndex, const float fontSize, const uint32_t firstChar, const uint32_t lastChar, const uint32_t atlasWidth, const uint32_t atlasHeight, LoadedFont *outFont) {
	if(dataPath == fpl_null || filename == fpl_null) {
		return false;
	}
	if(outFont == fpl_null) {
		return false;
	}

	char filePath[1024];
	fplCopyAnsiString(dataPath, filePath, FPL_ARRAYCOUNT(filePath));
	fplPathCombine(filePath, FPL_ARRAYCOUNT(filePath), 2, dataPath, filename);

	bool result = false;

	fplFileHandle file;
	uint8_t *ttfBuffer = fpl_null;
	uint32_t ttfBufferSize = 0;
	if(fplOpenAnsiBinaryFile(filePath, &file)) {
		ttfBufferSize = fplGetFileSizeFromHandle32(&file);
		ttfBuffer = (uint8_t *)fplMemoryAllocate(ttfBufferSize);
		fplReadFileBlock32(&file, ttfBufferSize, ttfBuffer, ttfBufferSize);
		fplCloseFile(&file);
	}

	if(ttfBuffer != nullptr) {
		result = LoadFontFromMemory(ttfBuffer, ttfBufferSize, fontIndex, fontSize, firstChar, lastChar, atlasWidth, atlasHeight, outFont);
		fplMemoryFree(ttfBuffer);
	}
	return(result);
}

extern void ReleaseFont(LoadedFont *font) {
	if(font != fpl_null) {
		fplMemoryFree(font->kerningTable);
		fplMemoryFree(font->glyphs);
		fplMemoryFree(font->atlasAlphaBitmap);
		FPL_CLEAR_STRUCT(font);
	}
}

#endif // FINAL_FONTLOADER_IMPLEMENTATION && !FINAL_FONTLOADER_IMPLEMENTED