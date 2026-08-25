/***
fui_font_stbtt.h

--- About ---

A reference font provider for final_ui.h, built on nothing but stb_truetype.h. It bakes a coverage atlas
from a TrueType file once and then answers final_ui's glyph, kerning and metric callbacks out of it, so a
program can hand fuiInit a real font in three calls instead of writing a glyph cache first.

final_ui.h itself asks for glyphs and never loads a font, which is what keeps it dependency free. This
file is the other half of that bargain: one worked example of the fuiFont interface, small enough to read
in one sitting and to replace with your own.

--- Getting started ---

- Put this file next to final_ui.h, with stb_truetype.h reachable as <stb/stb_truetype.h>
- Define FUI_STBTT_IMPLEMENTATION in ONE translation unit before including it
- stb_truetype's OWN implementation is deliberately NOT emitted here. Define STB_TRUETYPE_IMPLEMENTATION
  in a translation unit of its own, because stb's implementation does not compile clean under a strict
  warning set and yours should not have to be lowered to hold it

--- Usage ---

	fuiStbttFont bakedFont;
	if(!fuiStbttFontBake(&bakedFont, myTrueTypeBytes, fui_null)) {
		return false;   // the file was not a font, or the atlas was too small to hold the range
	}

	// One channel, one byte of coverage per texel. Upload it however your renderer likes.
	fuiTextureId atlas = MyUploadAlphaTexture(bakedFont.atlasPixels, bakedFont.atlasWidth, bakedFont.atlasHeight);

	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, atlas);
	fuiInit(&context, &font, fui_null);
	...
	fuiStbttFontRelease(&bakedFont);

The TrueType bytes are NOT copied and must outlive the font: kerning is asked for one pair at a time and
is read straight out of them.

--- Preprocessor overrides ---

	FUI_STBTT_IMPLEMENTATION      | Define in ONE translation unit to emit the implementation
	FUI_STBTT_TRUETYPE_HEADER     | Where stb_truetype.h is included from, defaults to <stb/stb_truetype.h>
	FUI_STBTT_MALLOC / _FREE      | Allocator, defaults to malloc and free from <stdlib.h>
	FUI_STBTT_MEMSET              | Defaults to memset from <string.h>
	FUI_STBTT_ASSERT              | Defaults to FUI_ASSERT

--- License ---

MIT License, Copyright (c) 2017-2026 Torsten Spaete
***/

#ifndef FUI_FONT_STBTT_INCLUDE_H
#define FUI_FONT_STBTT_INCLUDE_H

#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

//! One glyph as it came out of the bake, in font units where the nominal font size is 1.0
typedef struct fuiStbttGlyph {
	//! The geometry final_ui.h asks for, ready to be handed straight back
	fuiGlyph glyph;
	//! What the font calls this codepoint, resolved once so no lookup has to walk the cmap again
	int32_t indexInFont;
	//! False when the font has no glyph for this codepoint at all
	bool isKnown;
} fuiStbttGlyph;

/**
* @struct fuiStbttBakeSettings
* @brief What to rasterize and how large. Pass null to @ref fuiStbttFontBake to take all of it as it comes.
* @note The atlas is baked at ONE size and scaled per draw, which is what lets one font serve every text
*       height in the interface. Bake it a little larger than the biggest text you draw.
*/
typedef struct fuiStbttBakeSettings {
	//! Height one em is rasterized at, in pixels
	float pixelHeight;
	//! First codepoint of the contiguous range to bake
	uint32_t firstCodePoint;
	//! How many codepoints the range covers
	uint32_t codePointCount;
	//! Width of the atlas to pack into, in texels
	uint32_t atlasWidth;
	//! Height of the atlas to pack into, in texels
	uint32_t atlasHeight;
} fuiStbttBakeSettings;

/**
* @struct fuiStbttFont
* @brief One baked font: the atlas, the glyph table and what is needed to keep answering questions about it.
* @note All fields are public for inspection, but only the library writes them.
*/
typedef struct fuiStbttFont {
	//! Coverage atlas, one byte per texel and atlasWidth * atlasHeight of them. Upload as a one channel texture
	unsigned char *atlasPixels;
	//! Width of the atlas in texels
	uint32_t atlasWidth;
	//! Height of the atlas in texels
	uint32_t atlasHeight;
	//! Vertical metrics in font units, copied into the fuiFont as they are
	fuiFontMetrics metrics;
	//! One entry per codepoint of the baked range
	fuiStbttGlyph *glyphs;
	//! Codepoint the glyph table starts at
	uint32_t firstCodePoint;
	//! How many entries the glyph table has
	uint32_t codePointCount;
	//! Kerning of every pair of the baked range, in font units, or null when the range was too large to
	//! table and the pairs are read out of the font one at a time instead. Row major, left codepoint first
	float *kerningPairs;
	//! False when every pair in the range came back zero, which is what lets the callback be dropped entirely
	bool hasAnyKerning;
	//! The stbtt_fontinfo, kept alive for the codepoints the baked table does not cover
	void *fontInfo;
	//! The caller's TrueType bytes. NOT copied, and they must outlive this font
	const unsigned char *ttfData;
	//! What the atlas was rasterized at, which is what every measurement above was divided by
	float bakedPixelHeight;
	//! Scale from the font's own design units to the baked pixel height
	float rawToPixels;
} fuiStbttFont;

/**
* @brief Returns the settings a bake takes when it is given none.
* @return Returns 32 pixels, the printable ASCII range, and a 256 by 256 atlas.
*/
fui_api fuiStbttBakeSettings fuiStbttDefaultBakeSettings(void);

/**
* @brief Rasterizes a range of a TrueType font into a coverage atlas and builds its glyph table.
* @param[out] font Receives the baked font @ref fuiStbttFont.
* @param[in] ttfData The TrueType file's bytes. NOT copied - they must outlive the font.
* @param[in] settings What to bake @ref fuiStbttBakeSettings, or null for @ref fuiStbttDefaultBakeSettings.
* @return Returns true when the whole range was baked.
* @note An atlas too small to hold the range is a FAILURE, not a partial bake. Half an alphabet that only
*       goes missing at run time is worse than a bake that says so.
* @note Release it with @ref fuiStbttFontRelease.
*/
fui_api bool fuiStbttFontBake(fuiStbttFont *font, const void *ttfData, const fuiStbttBakeSettings *settings);

/**
* @brief Releases everything a bake allocated and leaves the font safely empty.
* @param[in,out] font Reference to the baked font @ref fuiStbttFont.
* @note The caller's TrueType bytes are not touched - this file never owned them.
*/
fui_api void fuiStbttFontRelease(fuiStbttFont *font);

/**
* @brief Wraps a baked font as the @ref fuiFont final_ui.h draws with.
* @param[in] font Reference to the baked font @ref fuiStbttFont, which must outlive the returned font.
* @param[in] atlasTexture The already uploaded atlas, as the renderer identifies it.
* @return Returns the font to hand to @ref fuiInit or @ref fuiSetFont.
*/
fui_api fuiFont fuiStbttFontToFuiFont(fuiStbttFont *font, const fuiTextureId atlasTexture);

#ifdef __cplusplus
}
#endif

#endif // FUI_FONT_STBTT_INCLUDE_H

// ****************************************************************************
// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
// ****************************************************************************
#if (defined(FUI_STBTT_IMPLEMENTATION) && !defined(FUI_STBTT_IMPLEMENTED)) || (FUI_IS_IDE)

#ifndef FUI_STBTT_IMPLEMENTED
#	define FUI_STBTT_IMPLEMENTED
#endif

#if !defined(FUI_STBTT_TRUETYPE_HEADER)
	//! Where stb_truetype.h is included from
#	define FUI_STBTT_TRUETYPE_HEADER <stb/stb_truetype.h>
#endif

// Interface only. stb's implementation belongs in a translation unit of its own, see "Getting started".
//
// Included only when it is not here yet, because stb_truetype.h puts its implementation OUTSIDE its own
// include guard: a second include while STB_TRUETYPE_IMPLEMENTATION is still defined emits every one of
// its functions a second time. A caller that includes stb first - which a single file demo naturally does
// - would otherwise not link.
#if !defined(__STB_INCLUDE_STB_TRUETYPE_H__)
#	include FUI_STBTT_TRUETYPE_HEADER
#endif

#if !defined(FUI_STBTT_MALLOC) || !defined(FUI_STBTT_FREE)
#	include <stdlib.h>
#	if !defined(FUI_STBTT_MALLOC)
#		define FUI_STBTT_MALLOC(size) malloc(size)
#	endif
#	if !defined(FUI_STBTT_FREE)
#		define FUI_STBTT_FREE(ptr) free(ptr)
#	endif
#endif

#if !defined(FUI_STBTT_MEMSET)
#	include <string.h>
#	define FUI_STBTT_MEMSET(destination, value, size) memset(destination, value, size)
#endif

#if !defined(FUI_STBTT_ASSERT)
#	define FUI_STBTT_ASSERT(expression) FUI_ASSERT(expression)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//! Height one em is rasterized at when the caller says nothing
#define FUI__STBTT_DEFAULT_PIXEL_HEIGHT 32.0f

//! First codepoint of the default range, which is the space
#define FUI__STBTT_DEFAULT_FIRST_CODEPOINT 32u

//! How many codepoints the default range covers, which is printable ASCII up to the tilde
#define FUI__STBTT_DEFAULT_CODEPOINT_COUNT 95u

//! Side of the default atlas in texels
#define FUI__STBTT_DEFAULT_ATLAS_SIDE 256u

//! Glyph index a TrueType font gives a codepoint it does not have
#define FUI__STBTT_MISSING_GLYPH_INDEX 0

//! Largest range that gets a baked kerning table. The table is one float per PAIR, so this is the point
//! where it would cost a megabyte and reading the font per pair is the better trade after all
#define FUI__STBTT_MAX_KERNED_CODEPOINTS 512u

fui_api fuiStbttBakeSettings fuiStbttDefaultBakeSettings(void) {
	fuiStbttBakeSettings result;
	result.pixelHeight = FUI__STBTT_DEFAULT_PIXEL_HEIGHT;
	result.firstCodePoint = FUI__STBTT_DEFAULT_FIRST_CODEPOINT;
	result.codePointCount = FUI__STBTT_DEFAULT_CODEPOINT_COUNT;
	result.atlasWidth = FUI__STBTT_DEFAULT_ATLAS_SIDE;
	result.atlasHeight = FUI__STBTT_DEFAULT_ATLAS_SIDE;
	return(result);
}

//! Answers one codepoint out of the baked table. The glyph callback final_ui.h calls per character
static bool fui__StbttGetGlyph(void *userData, uint32_t codePoint, fuiGlyph *outGlyph) {
	const fuiStbttFont *font = (const fuiStbttFont *)userData;
	if(font == fui_null || font->glyphs == fui_null || outGlyph == fui_null) {
		return(false);
	}
	if(codePoint < font->firstCodePoint) {
		return(false);
	}
	uint32_t glyphIndex = codePoint - font->firstCodePoint;
	if(glyphIndex >= font->codePointCount) {
		return(false);
	}
	const fuiStbttGlyph *entry = &font->glyphs[glyphIndex];
	if(!entry->isKnown) {
		// Reported as missing rather than handed back as stb's .notdef box. A codepoint outside the baked
		// range comes back the same way, so one answer covers both and the caller sees no difference.
		return(false);
	}
	*outGlyph = entry->glyph;
	return(true);
}

/*
	Answers one kerning pair.

	This is called once per CHARACTER of every string the interface measures or draws, which for a table
	of a few hundred visible cells is tens of thousands of calls a frame. Asking the font each time is
	what that used to do, and stbtt_GetCodepointKernAdvance is not a lookup: it walks the character map
	twice to turn the two codepoints into glyph indices and then binary searches the kerning table. It
	measured at over eighty percent of the whole frame.

	So the baked range gets a real table, one entry per PAIR, filled once at bake time. A range of
	printable ASCII is 95 by 95, which is thirty six kilobytes and one indexed read per call. A range too
	large to table that way falls through to the old path, which is still correct, just slow.
*/
static float fui__StbttGetKerning(void *userData, uint32_t leftCodePoint, uint32_t rightCodePoint) {
	fuiStbttFont *font = (fuiStbttFont *)userData;
	if(font == fui_null || font->bakedPixelHeight <= 0.0f) {
		return(0.0f);
	}

	uint32_t firstCodePoint = font->firstCodePoint;
	uint32_t codePointCount = font->codePointCount;
	bool leftIsInRange = (leftCodePoint >= firstCodePoint) && ((leftCodePoint - firstCodePoint) < codePointCount);
	bool rightIsInRange = (rightCodePoint >= firstCodePoint) && ((rightCodePoint - firstCodePoint) < codePointCount);
	if(font->kerningPairs != fui_null && leftIsInRange && rightIsInRange) {
		size_t leftIndex = (size_t)(leftCodePoint - firstCodePoint);
		size_t rightIndex = (size_t)(rightCodePoint - firstCodePoint);
		size_t pairIndex = leftIndex * (size_t)codePointCount + rightIndex;
		return(font->kerningPairs[pairIndex]);
	}

	if(font->fontInfo == fui_null) {
		return(0.0f);
	}
	stbtt_fontinfo *fontInfo = (stbtt_fontinfo *)font->fontInfo;
	int rawKerning = stbtt_GetCodepointKernAdvance(fontInfo, (int)leftCodePoint, (int)rightCodePoint);
	if(rawKerning == 0) {
		return(0.0f);
	}
	float kerningInPixels = (float)rawKerning * font->rawToPixels;
	float result = kerningInPixels / font->bakedPixelHeight;
	return(result);
}

fui_api bool fuiStbttFontBake(fuiStbttFont *font, const void *ttfData, const fuiStbttBakeSettings *settings) {
	FUI_STBTT_ASSERT(font != fui_null && ttfData != fui_null);
	if(font == fui_null || ttfData == fui_null) {
		return(false);
	}

	// Cleared BEFORE anything can go wrong, so every way out of this function leaves a font that is safe to
	// hand to fuiStbttFontRelease. An error path a caller has to remember not to unwind is a trap.
	FUI_STBTT_MEMSET(font, 0, sizeof(*font));

	fuiStbttBakeSettings resolved = (settings != fui_null) ? *settings : fuiStbttDefaultBakeSettings();
	bool settingsAreUsable = (resolved.pixelHeight > 0.0f) && (resolved.codePointCount > 0) && (resolved.atlasWidth > 0) && (resolved.atlasHeight > 0);
	if(!settingsAreUsable) {
		return(false);
	}

	const unsigned char *fontBytes = (const unsigned char *)ttfData;
	stbtt_fontinfo *fontInfo = (stbtt_fontinfo *)FUI_STBTT_MALLOC(sizeof(stbtt_fontinfo));
	if(fontInfo == fui_null) {
		return(false);
	}
	const int firstFontInTheFile = 0;
	int fontOffset = stbtt_GetFontOffsetForIndex(fontBytes, firstFontInTheFile);
	if(fontOffset < 0 || !stbtt_InitFont(fontInfo, fontBytes, fontOffset)) {
		FUI_STBTT_FREE(fontInfo);
		return(false);
	}

	size_t atlasByteCount = (size_t)resolved.atlasWidth * (size_t)resolved.atlasHeight;
	unsigned char *atlasPixels = (unsigned char *)FUI_STBTT_MALLOC(atlasByteCount);
	fuiStbttGlyph *glyphs = (fuiStbttGlyph *)FUI_STBTT_MALLOC((size_t)resolved.codePointCount * sizeof(fuiStbttGlyph));
	stbtt_packedchar *packedCharacters = (stbtt_packedchar *)FUI_STBTT_MALLOC((size_t)resolved.codePointCount * sizeof(stbtt_packedchar));
	if(atlasPixels == fui_null || glyphs == fui_null || packedCharacters == fui_null) {
		FUI_STBTT_FREE(packedCharacters);
		FUI_STBTT_FREE(glyphs);
		FUI_STBTT_FREE(atlasPixels);
		FUI_STBTT_FREE(fontInfo);
		return(false);
	}
	FUI_STBTT_MEMSET(atlasPixels, 0, atlasByteCount);
	FUI_STBTT_MEMSET(glyphs, 0, (size_t)resolved.codePointCount * sizeof(fuiStbttGlyph));

	// Oversampling stays at one by one on purpose. It sharpens small text by packing the glyph at a higher
	// resolution, but only a sampler that knows it was done gets that back - and a backend handed a plain
	// atlas and a uv rectangle does not know. A reference implementation must not need a secret.
	const int noOversampling = 1;
	const int noPaddingBetweenGlyphs = 1;
	stbtt_pack_context packContext;
	if(!stbtt_PackBegin(&packContext, atlasPixels, (int)resolved.atlasWidth, (int)resolved.atlasHeight, 0, noPaddingBetweenGlyphs, fui_null)) {
		FUI_STBTT_FREE(packedCharacters);
		FUI_STBTT_FREE(glyphs);
		FUI_STBTT_FREE(atlasPixels);
		FUI_STBTT_FREE(fontInfo);
		return(false);
	}
	stbtt_PackSetOversampling(&packContext, (unsigned int)noOversampling, (unsigned int)noOversampling);
	int packedTheWholeRange = stbtt_PackFontRange(&packContext, fontBytes, firstFontInTheFile, resolved.pixelHeight, (int)resolved.firstCodePoint, (int)resolved.codePointCount, packedCharacters);
	stbtt_PackEnd(&packContext);
	if(!packedTheWholeRange) {
		FUI_STBTT_FREE(packedCharacters);
		FUI_STBTT_FREE(glyphs);
		FUI_STBTT_FREE(atlasPixels);
		FUI_STBTT_FREE(fontInfo);
		return(false);
	}

	// stbtt_ScaleForPixelHeight is what PackFontRange used, and it maps ascent minus descent onto exactly
	// the pixel height asked for. Dividing by that height again is what puts everything into font units
	// where the nominal size is 1.0, which is the convention fuiFont is defined in.
	float scaleToPixels = stbtt_ScaleForPixelHeight(fontInfo, resolved.pixelHeight);
	int rawAscent = 0;
	int rawDescent = 0;
	int rawLineGap = 0;
	stbtt_GetFontVMetrics(fontInfo, &rawAscent, &rawDescent, &rawLineGap);
	float rawToFontUnits = scaleToPixels / resolved.pixelHeight;

	font->atlasPixels = atlasPixels;
	font->atlasWidth = resolved.atlasWidth;
	font->atlasHeight = resolved.atlasHeight;
	font->glyphs = glyphs;
	font->firstCodePoint = resolved.firstCodePoint;
	font->codePointCount = resolved.codePointCount;
	font->fontInfo = fontInfo;
	font->ttfData = fontBytes;
	font->bakedPixelHeight = resolved.pixelHeight;
	font->rawToPixels = scaleToPixels;

	// A descent points DOWN from the baseline and TrueType writes it as a negative number, while final_ui.h
	// wants both halves of the line as positive distances.
	font->metrics.ascent = (float)rawAscent * rawToFontUnits;
	font->metrics.descent = (float)(-rawDescent) * rawToFontUnits;
	font->metrics.lineHeight = (float)(rawAscent - rawDescent + rawLineGap) * rawToFontUnits;

	for(uint32_t glyphIndex = 0; glyphIndex < resolved.codePointCount; ++glyphIndex) {
		uint32_t codePoint = resolved.firstCodePoint + glyphIndex;

		// The pen starts at the origin, so the quad comes back as the offset from the pen to the glyph's
		// top-left corner and the pen is left holding this glyph's advance.
		float penX = 0.0f;
		float penY = 0.0f;
		stbtt_aligned_quad quad;
		const int doNotAlignToInteger = 0;
		stbtt_GetPackedQuad(packedCharacters, (int)resolved.atlasWidth, (int)resolved.atlasHeight, (int)glyphIndex, &penX, &penY, &quad, doNotAlignToInteger);

		fuiStbttGlyph *entry = &glyphs[glyphIndex];
		int indexInFont = stbtt_FindGlyphIndex(fontInfo, (int)codePoint);
		entry->indexInFont = (int32_t)indexInFont;
		entry->isKnown = (indexInFont != FUI__STBTT_MISSING_GLYPH_INDEX);
		entry->glyph.offset = fuiV2(quad.x0 / resolved.pixelHeight, quad.y0 / resolved.pixelHeight);
		entry->glyph.size = fuiV2((quad.x1 - quad.x0) / resolved.pixelHeight, (quad.y1 - quad.y0) / resolved.pixelHeight);
		entry->glyph.uvMin = fuiV2(quad.s0, quad.t0);
		entry->glyph.uvMax = fuiV2(quad.s1, quad.t1);
		entry->glyph.advance = penX / resolved.pixelHeight;
	}
	FUI_STBTT_FREE(packedCharacters);

	/*
		The kerning table.

		Every pair of the baked range, resolved once, so the callback that runs per character of every
		string is an array read. The glyph indices were resolved in the loop above, so this asks the font
		by GLYPH and never touches the character map again.

		A range too large to table is left alone: the callback falls back to reading the font per pair,
		which is what this used to do for everything.
	*/
	font->kerningPairs = fui_null;
	font->hasAnyKerning = false;
	if(resolved.codePointCount <= FUI__STBTT_MAX_KERNED_CODEPOINTS) {
		size_t pairCount = (size_t)resolved.codePointCount * (size_t)resolved.codePointCount;
		float *kerningPairs = (float *)FUI_STBTT_MALLOC(pairCount * sizeof(float));
		if(kerningPairs != fui_null) {
			for(uint32_t leftIndex = 0; leftIndex < resolved.codePointCount; ++leftIndex) {
				int leftGlyph = (int)glyphs[leftIndex].indexInFont;
				size_t rowBase = (size_t)leftIndex * (size_t)resolved.codePointCount;
				for(uint32_t rightIndex = 0; rightIndex < resolved.codePointCount; ++rightIndex) {
					int rightGlyph = (int)glyphs[rightIndex].indexInFont;
					int rawKerning = 0;
					// A pair the font has no glyph for on either side cannot kern, and asking would walk
					// the kerning table for an answer that is always zero.
					if(leftGlyph != FUI__STBTT_MISSING_GLYPH_INDEX && rightGlyph != FUI__STBTT_MISSING_GLYPH_INDEX) {
						rawKerning = stbtt_GetGlyphKernAdvance(fontInfo, leftGlyph, rightGlyph);
					}
					float kerningInFontUnits = 0.0f;
					if(rawKerning != 0) {
						float kerningInPixels = (float)rawKerning * scaleToPixels;
						kerningInFontUnits = kerningInPixels / resolved.pixelHeight;
						font->hasAnyKerning = true;
					}
					kerningPairs[rowBase + rightIndex] = kerningInFontUnits;
				}
			}
			font->kerningPairs = kerningPairs;
		}
	} else {
		// Nothing is known about a range that was not tabled, so the callback has to stay wired up.
		font->hasAnyKerning = true;
	}

	// The space is asked for by name rather than looked up per character, because it is what a codepoint
	// with no glyph falls back to and that lookup has already failed by then.
	int rawSpaceAdvance = 0;
	int rawSpaceLeftSideBearing = 0;
	stbtt_GetCodepointHMetrics(fontInfo, (int)' ', &rawSpaceAdvance, &rawSpaceLeftSideBearing);
	font->metrics.spaceAdvance = (float)rawSpaceAdvance * rawToFontUnits;

	return(true);
}

fui_api void fuiStbttFontRelease(fuiStbttFont *font) {
	FUI_STBTT_ASSERT(font != fui_null);
	if(font == fui_null) {
		return;
	}
	FUI_STBTT_FREE(font->glyphs);
	FUI_STBTT_FREE(font->kerningPairs);
	FUI_STBTT_FREE(font->atlasPixels);
	FUI_STBTT_FREE(font->fontInfo);
	FUI_STBTT_MEMSET(font, 0, sizeof(*font));
}

fui_api fuiFont fuiStbttFontToFuiFont(fuiStbttFont *font, const fuiTextureId atlasTexture) {
	fuiFont result = fuiZeroFont();
	FUI_STBTT_ASSERT(font != fui_null);
	if(font == fui_null) {
		return(result);
	}
	result.getGlyph = fui__StbttGetGlyph;
	// A font whose whole baked range kerns to zero is handed back with NO kerning callback at all, which
	// is one indirect call per character that never happens. Plenty of faces are like this.
	result.getKerning = font->hasAnyKerning ? fui__StbttGetKerning : fui_null;
	// No measureText fast path: summing the advances final_ui.h already asks for cannot disagree with
	// itself, and a second measurement path is one more place for a caret to land on the wrong character.
	result.measureText = fui_null;
	result.metrics = font->metrics;
	result.atlasTexture = atlasTexture;
	result.atlasSize = fuiV2((float)font->atlasWidth, (float)font->atlasHeight);
	result.userData = font;
	return(result);
}

#ifdef __cplusplus
}
#endif

#endif // FUI_STBTT_IMPLEMENTATION
