#ifndef FONT_H
#define FONT_H

#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

#include <stb/stb_truetype.h>

#include "vecmath.h"

struct FontGlyph {
	Vec2f alignPercentage;
	Vec2f uvMin, uvMax;
	Vec2f charSize;
	uint32_t charCode;
};

struct FontInfo {
	float ascent;
	float descent;
	float lineHeight;
	float baseline;
	float spaceAdvance;
};

struct Font {
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

inline float GetFontBaseline(FontInfo *fontInfo) {
	float result = fontInfo->baseline;
	return(result);
}

inline float GetFontAscent(FontInfo *fontInfo) {
	float result = fontInfo->ascent;
	return(result);
}

inline float GetFontDescent(FontInfo *fontInfo) {
	float result = fontInfo->descent;
	return(result);
}

inline float GetFontLineAdvance(FontInfo *fontInfo) {
	float result = fontInfo->lineHeight;
	return(result);
}

inline float GetFontCharacterAdvance(Font *font, uint32_t *codePoint, uint32_t *nextCodePoint) {
	float result = 0;
	if (codePoint) {
		FontGlyph *glyph = font->glyphs + *codePoint;
		result = font->defaultAdvance[*codePoint];
		if (nextCodePoint != nullptr) {
			float kerning = font->kerningTable[*codePoint * font->charCount + *nextCodePoint];
			result += kerning;
		}
	}
	return(result);
}

inline float GetTextWidth(const char *text, const uint32_t textLen, Font *font, const float maxCharHeight) {
	float result = 0;
	uint32_t textPos = 0;
	if (font != nullptr) {
		while (textPos < textLen) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float advance;
			uint32_t codePoint = at - font->firstChar;
			if (codePoint >= 0 && codePoint < font->charCount) {
				uint32_t nextCodePoint = (atNext > 0) ? atNext - font->firstChar : 0;
				advance = GetFontCharacterAdvance(font, &codePoint, (atNext > 0) ? &nextCodePoint : nullptr) * maxCharHeight;
			} else {
				advance = font->info.spaceAdvance * maxCharHeight;
			}
			result += advance;
			++textPos;
		}
	}
	return(result);
}

static Font LoadFont(const char *filename, const uint32_t fontIndex, const float fontSize, uint32_t firstChar, uint32_t lastChar, uint32_t atlasWidth, uint32_t atlasHeight) {
#define BETTER_QUALITY 0

	Font result = {};

	uint8_t *ttfBuffer = LoadFileContent(filename);
	if (ttfBuffer != nullptr) {

		stbtt_fontinfo fontInfo = {};
		int fontOffset = stbtt_GetFontOffsetForIndex(ttfBuffer, fontIndex);

		if (stbtt_InitFont(&fontInfo, ttfBuffer, fontOffset)) {
			uint32_t charCount = (lastChar - firstChar) + 1;
			uint8_t *atlasAlphaBitmap = (uint8_t *)fplMemoryAllocate(atlasWidth * atlasHeight);
#if BETTER_QUALITY
			stbtt_packedchar *packedChars = (stbtt_packedchar *)MemoryAllocate(charCount * sizeof(stbtt_packedchar));
			memset(packedChars, 0, charCount * sizeof(stbtt_packedchar));

			stbtt_pack_range characterRange = {};
			characterRange.font_size = fontSize;
			characterRange.num_chars = charCount;
			characterRange.first_unicode_codepoint_in_range = firstChar;
			characterRange.chardata_for_range = packedChars;

			stbtt_pack_context packContext = {};
			stbtt_pack_range *ranges[1] = {
				&characterRange,
			};
			stbtt_PackBegin(&packContext, atlasAlphaBitmap, atlasWidth, atlasHeight, atlasWidth, 0, 0);
			stbtt_PackSetOversampling(&packContext, 1, 1);
			stbtt_PackFontRanges(&packContext, ttfBuffer, fontIndex, ranges[0], 1);
			stbtt_PackEnd(&packContext);
#else
			stbtt_bakedchar *packedChars = (stbtt_bakedchar *)fplMemoryAllocate(charCount * sizeof(stbtt_bakedchar));
			stbtt_BakeFontBitmap(ttfBuffer, fontOffset, fontSize, atlasAlphaBitmap, atlasWidth, atlasHeight, firstChar, charCount, packedChars);
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
			assert(heightPx * fontScale == 1.0f);

			// Calculate line height
			float lineGapPx = lineGapRaw * pixelScale;
			float lineHeightPx = ascentPx + descentPx + lineGapPx;

			// Correction to center the font to the middle (baseline will not be in the middle!)
			float verticalCenterCorrectionPx = (0 + descentPx) - (heightPx * 0.5f);

			size_t glyphsSize = sizeof(FontGlyph) * charCount;
			FontGlyph *glyphs = (FontGlyph *)fplMemoryAllocate(glyphsSize);
			memset(glyphs, 0, glyphsSize);

			for (uint32_t glyphIndex = 0; glyphIndex < charCount; ++glyphIndex) {
#if BETTER_QUALITY
				stbtt_packedchar *sourceInfo = packedChars + glyphIndex;
#else
				stbtt_bakedchar *sourceInfo = packedChars + glyphIndex;
#endif

				FontGlyph * destInfo = glyphs + glyphIndex;
				*destInfo = {};
				destInfo->charCode = firstChar + glyphIndex;

				// Compute UV coords
				float uMin = sourceInfo->x0 * texelU;
				float uMax = sourceInfo->x1 * texelU;
				float vMin = sourceInfo->y1 * texelV;
				float vMax = sourceInfo->y0 * texelV;
				destInfo->uvMin = Vec2f(uMin, vMin);
				destInfo->uvMax = Vec2f(uMax, vMax);

				// Compute character size
				int charWidthInPixels = (sourceInfo->x1 - sourceInfo->x0) + 1;
				int charHeightInPixels = (sourceInfo->y1 - sourceInfo->y0) + 1;
				destInfo->charSize = Vec2f((float)charWidthInPixels, (float)charHeightInPixels) * fontScale;

				// Move the character half the size to the right - so we are left aligned
				float xoffset = destInfo->charSize.w * 0.5f;
				destInfo->alignPercentage.x = xoffset / destInfo->charSize.w;

				// Move down the character so that its top aligned
				// Move up the character to the baseline by the given y-offset (y-offset is negative, so we subtract it instead of adding)
				float halfHeight = destInfo->charSize.h * 0.5f;
				float baselineOffset = sourceInfo->yoff * fontScale;
				float yoffset = verticalCenterCorrectionPx * fontScale - halfHeight - baselineOffset;
				destInfo->alignPercentage.y = yoffset / destInfo->charSize.h;
			}

			// Build kerning table
			size_t kerningTableSize = sizeof(float) * charCount * charCount;
			float *kerningTable = (float *)fplMemoryAllocate(kerningTableSize);
			memset(kerningTable, 0, kerningTableSize);
			for (uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
#if BETTER_QUALITY
				stbtt_packedchar *leftInfo = packedChars + charIndex;
#else
				stbtt_bakedchar *leftInfo = packedChars + charIndex;
#endif
				for (uint32_t nextCharIndex = charIndex + 1; nextCharIndex < lastChar; ++nextCharIndex) {
					float kerningPx = stbtt_GetCodepointKernAdvance(&fontInfo, charIndex, nextCharIndex) * pixelScale;
					if (kerningPx != 0) {
						float widthPx = (float)(leftInfo->x1 - leftInfo->x0);
						float kerning = kerningPx / (float)widthPx;
						uint32_t a = (uint32_t)(charIndex - firstChar);
						uint32_t b = (uint32_t)(nextCharIndex - firstChar);
						kerningTable[a * charCount + b] = kerning;
					}
				}
			}

			// Build default advance table
			size_t defaultAdvanceSize = charCount * sizeof(float);
			float *defaultAdvance = (float *)fplMemoryAllocate(defaultAdvanceSize);
			memset(defaultAdvance, 0, defaultAdvanceSize);
			for (uint32_t charIndex = firstChar; charIndex < lastChar; ++charIndex) {
				int advanceRaw, leftSideBearing;
				stbtt_GetCodepointHMetrics(&fontInfo, charIndex, &advanceRaw, &leftSideBearing);
				uint32_t index = (uint32_t)(charIndex - firstChar);
				defaultAdvance[index] = (advanceRaw * pixelScale) * fontScale;
			}

			result.firstChar = firstChar;
			result.charCount = charCount;
			result.info.ascent = ascentPx * fontScale;
			result.info.descent = descentPx * fontScale;
			result.info.baseline = verticalCenterCorrectionPx * fontScale;
			result.info.lineHeight = lineHeightPx * fontScale;
			result.info.spaceAdvance = spaceAdvancePx * fontScale;
			result.glyphs = glyphs;
			result.kerningTable = kerningTable;
			result.defaultAdvance = defaultAdvance;
			result.atlasAlphaBitmap = atlasAlphaBitmap;
			result.atlasWidth = atlasWidth;
			result.atlasHeight = atlasHeight;
		}

		fplMemoryFree(ttfBuffer);
	}

	return(result);
}

static void ReleaseFont(Font *font) {
	fplMemoryFree(font->kerningTable);
	fplMemoryFree(font->glyphs);
	fplMemoryFree(font->atlasAlphaBitmap);
	*font = {};
}

#endif
