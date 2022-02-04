/***
final_font.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file bitmap font atlas creator.

This library is designed to load a TTF/TTC font and create font/bitmaps atlases from it.
It can be used to extract each glyph individually or prepare it for graphics rendering applications, such as engines or games.

The only dependencies are built-in operating system libraries, a C99 complaint compiler and the STB_truetype.h library.

Required linking is bare minimum:
	Win32: Link to kernel32.lib
	Unix/Linux: Link to ld.so

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you do the rendering.
- Define FNT_IMPLEMENTATION before including this header file in that translation unit.

- Load your font file/stream into the @ref fntFontData structure (size, data, name)

@code{.c}
size_t font_data_length = GetFileSizeFromFile("arial.ttf");
uint8_t *font_data = (uint8_t *)malloc(font_data_length);
LoadFontFileIntoArray("arial.ttf", font_data, font_data_length);

fntFontData fontData;
fontData.size = (size_t)font_data_length; // Get size of the font data
fontData.data = (const uint8_t *)font_data; // Get pointer to the data of the font
fontData.name = "Arial"; // Just any name, must not match the actual name but is required
@endcode

- Load the font informations for a particular font-size and font-index into @ref fntFontInfo struct, such as ascent, descent, space advance, etc.

@code{.c}
fntFontData fontData;
fntFontInfo fontInfo;
uint32_t fontIndex = 0;
float fontSize = 42.0f;
if (fntLoadFontInfo(&fontData, &fontInfo, fontIndex, fontSize)) {
	// ...
	fntFreeFontInfo(&fontInfo);
}
@endcode

- Create a font atlas with the loaded fntFontInfo
	> fntCreateFontAtlas()

- Create a font context with the loaded fntFontData and fntFontInfo
	> fntCreateFontContext()

- Add your unicode point ranges to to the context and the atlas, that will create the bitmaps automatically and compute all relevant informations
	> fntAddToFontAtlas()

- Release the font context
	> fntReleaseFontContext()

- Release the font atlas
	-> fntFreeFontAtlas()

- Release the font info
	-> fntFreeFontInfo()

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Font is released under the following license:

MIT License

Copyright (c) 2022 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_font.h
	\version v0.1.0 alpha
	\author Torsten Spaete
	\brief Final Font (FNT) - A open source C99 single header file bitmap font atlas creator library.
*/

// ****************************************************************************
//
// > Changelog
//
// ****************************************************************************

/*!
	\page page_changelog Changelog
	\tableofcontents

	# v0.1.0 alpha:
	- Initial version
*/

// ****************************************************************************
//
// > TODO
//
// ****************************************************************************

/*!
	\page page_todo Todo
	\tableofcontents

	- Custom allocator support
*/


// ****************************************************************************
//
// > Header
//
// ****************************************************************************
#ifndef FNT_INCLUDE_H
#define FNT_INCLUDE_H

//
// Includes
//
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // uint32_t
#include <stdlib.h> // NULL
#include <stdbool.h> // bool

#if defined(FNT_AS_PRIVATE)
	//! API functions exported as static
#	define fnt_api static
#else
	//! API functions exported as extern
#	define fnt_api extern
#endif

// Malloc
#if !defined(FNT_MALLOC)
#include <malloc.h>
#define FNT_MALLOC(size) malloc(size)
#define FNT_REALLOC(block, size) realloc(block, size)
#define FNT_FREE(block) free(block)
#endif

// Memset
#if !defined(FNT_MEMSET)
#include <string.h>
#define FNT_MEMSET(dst, value, size) memset(dst, value, size)
#define FNT_MEMCOPY(dst, src, size) memcpy(dst, src, size)
#endif

// Strings
#if !defined(FNT_STRLEN)
#include <string.h>
#define FNT_STRLEN(s) strlen(s)
#endif

// Assert
#if !defined(FNT_ASSERT)
#include <assert.h>
#define FNT_ASSERT(exp) assert(exp)
#endif

// File I/O
#if !defined(FNT_FILE_HANDLE)
#include <stdio.h>
#define FNT_FILE_HANDLE FILE *
#define FNT_CREATE_BINARY_FILE(filePath, fileHandle) (fopen_s(fileHandle, filePath, "wb") == 0)
#define FNT_WRITE_TO_FILE(fileHandle, buffer, size) fwrite(buffer, size, 1, fileHandle)
#define FNT_CLOSE_FILE(fileHandle) fclose(fileHandle)
#endif

// API
#ifdef __cplusplus
extern "C" {
#endif

	typedef struct fntFontData {
		//! The raw font file data
		const uint8_t *data;
		//! The name of the font
		const char *name;
		//! The number of bytes
		size_t size;
	} fntFontData;

	typedef enum fntBitmapFormat {
		//! 8-bit alpha only
		fntFontBitmapFormat_Alpha8 = 0,
		//! 32-bit RGBA
		fntFontBitmapFormat_RGBA8
	} fntBitmapFormat;

	typedef struct fntBitmap {
		//! The pixels from top-to-bottom
		const uint8_t *pixels;
		//! The format
		fntBitmapFormat format;
		//! The width in pixels
		uint16_t width;
		//! The height in pixels
		uint16_t height;
	} fntBitmap;

	typedef struct fntWhiteSpaceTable {
		//! The table that maps from a code-point to a number 0/1 that indicates if the point is a whitespace or not
		uint32_t *states;
		//! The horizontal whitespace advancement table, each value in pixels
		float *advancements;
	} fntWhiteSpaceTable;

	typedef struct fntFontInfo {
		//! The whitespace table
		fntWhiteSpaceTable whitespaceTable;
		//! The font name
		const char *name;
		//! The font size
		float fontSize;
		//! The font height to scale factor
		float heightToScale;
		//! The ascent from the baseline in pixels
		float ascent;
		//! The descent from the baseline in pixels
		float descent;
		//! The font index
		uint32_t fontIndex;
	} fntFontInfo;

	typedef union fntVec2 {
		struct {
			//! The X component of the vector
			float x;
			//! The Y component of the vector
			float y;
		};
		struct {
			//! The width component
			float w;
			//! The height component
			float h;
		};
		struct {
			//! The U texcoord component
			float u;
			//! The V texcoord component
			float v;
		};
		float m[2];
	} fntVec2;

	typedef struct fntFontGlyph {
		//! The baseline offsets in pixels
		fntVec2 baselineOffsets[2];
		//! The horizontal advancement in pixels
		float horizontalAdvance;
		//! The code point for validation
		uint32_t codePoint;
		//! The width in the bitmap
		uint16_t width;
		//! The height in the bitmap
		uint16_t height;
		//! The X position in the bitmap
		uint16_t bitmapX;
		//! The Y position in the bitmap
		uint16_t bitmapY;
	} fntFontGlyph;

	typedef struct fntFontPage {
		//! The array of font glyphs
		fntFontGlyph *glyphs;
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The first code point
		uint32_t codePointStart;
		//! The number of code points (N)
		uint32_t codePointCount;
	} fntFontPage;

	typedef struct fntKerningPagePair {
		//! The index to the first kerning value
		size_t valueIndex;
		//! The number of kerning pairs
		size_t pairCount;
		//! The first page index
		uint32_t pageIndexA;
		//! The second page index
		uint32_t pageIndexB;
	} fntKerningPagePair;

	typedef struct fntFontAtlas {
		//! The kerning table for all pages, each pair of pages has N x N code-point pairs
		float *kerningValues;
		//! The kerning page pairs, so we can fast go from a page-pair to kerning table start
		fntKerningPagePair *kerningPages;
		//! The array of font pages
		fntFontPage *pages;
		//! The array of alpha bitmaps
		fntBitmap *bitmaps;
		//! The array of code-points mapped to a font page number in range of 1 to N, zero means not-set
		uint32_t *codePointsToPageIndices;
		//! The number of pages
		uint32_t pageCount;
		//! The number of bitmaps
		uint32_t bitmapCount;
	} fntFontAtlas;

	typedef void *fntFontContext;

	fnt_api bool fntLoadFontInfo(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize);
	fnt_api void fntFreeFontInfo(fntFontInfo *info);

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info, const uint32_t maxBitmapSize);
	fnt_api void fntReleaseFontContext(fntFontContext *context);

	fnt_api bool fntInitFontAtlas(const fntFontInfo *info, fntFontAtlas *atlas);
	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const uint32_t codePointIndex, const uint32_t codePointCount);
	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas);

	fnt_api bool fntSaveBitmapToFile(const fntBitmap *bitmap, const char *filePath);

	fnt_api bool fntComputeAtlasKernings(const fntFontContext *context, fntFontAtlas *atlas);

	typedef struct fntFontQuad {
		fntVec2 uv[2];
		fntVec2 rect[2];
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The code point
		uint32_t codePoint;
	} fntFontQuad;

	// Top-Left aligned, Top-Down is default
	typedef enum fntComputeQuadsFlags {
		fntComputeQuadsFlags_None = 0,
		fntComputeQuadsFlags_Cartesian = 1 << 0,
		fntComputeQuadsFlags_AlignHorizontalLeft = 1 << 1,
		fntComputeQuadsFlags_AlignHorizontalCenter = 1 << 2,
		fntComputeQuadsFlags_AlignHorizontalRight = 1 << 3,
		fntComputeQuadsFlags_AlignVerticalTop = 1 << 4,
		fntComputeQuadsFlags_AlignVerticalMiddle = 1 << 5,
		fntComputeQuadsFlags_AlignVerticalBottom = 1 << 6,
	} fntComputeQuadsFlags;

	fnt_api size_t fntGetQuadCountFromUTF8(const fntFontAtlas *atlas, const char *utf8);
	fnt_api bool fntComputeQuadsFromUTF8(const fntFontAtlas *atlas, const fntFontInfo *info, const char *utf8, const float charHeight, const fntComputeQuadsFlags flags, const size_t numQuads, fntFontQuad *outQuads, fntVec2 *outSize);

#ifdef __cplusplus
}
#endif

#endif // FNT_INCLUDE_H

// ****************************************************************************
//
// > Implementation
//
// ****************************************************************************
#if defined(FNT_IMPLEMENTATION) && !defined(FNT_IMPLEMENTED)
#	define FNT_IMPLEMENTED

#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
#	define FNT_IS_C99
#elif defined(__cplusplus)
#	define FNT_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

//! Macro for initialize a struct to zero
#if defined(FNT_IS_C99)
#	define FNT__ZERO_INIT {0}
#	define FNT__STRUCT_INIT(type, ...) (type){__VA_ARGS__}
#else
#	define FNT__ZERO_INIT {}
#	define FNT__STRUCT_INIT(type, ...) {__VA_ARGS__}
#endif

// See: https://en.wikipedia.org/wiki/Code_point
#define FNT__MAX_UNICODE_POINT_COUNT 0xFFFF

#define FNT__MIN_BITMAP_SIZE 32

#define FNT__MIN_FONT_SIZE 4

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#define FINAL_STBTT_PACKEDCHAR_CODEPOINT
#include <stb/stb_truetype.h>

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#ifdef __cplusplus
extern "C" {
#endif

	static void fnt__FreeNotNull(void *ptr) {
		if (ptr != NULL)
			FNT_FREE(ptr);
	}

	typedef struct {
		fntFontInfo info;
		fntFontData data;
		stbtt_fontinfo sinfo;
		stbtt_pack_context currentPackContext;
		uint32_t maxBitmapSize;
		uint32_t bitmapIndex;
		uint32_t hasPackContext;
	} fnt__STBFontContext;

	inline fntVec2 fnt__MakeVec2(const float x, const float y) {
		fntVec2 result;
		result.x = x;
		result.y = y;
		return(result);
	}

	static bool fnt__IsValidFontInfo(const fntFontInfo *info) {
		if (info == NULL) return(false);
		if (info->fontSize < FNT__MIN_FONT_SIZE) return(false);
		if (info->name == NULL) return(false);
		return(true);
	}

	static bool fnt__IsValidFontData(const fntFontData *data) {
		if (data == NULL) return(false);
		if (data->size == 0) return(false);
		if (data->data == NULL) return(false);
		return(true);
	}

	static bool fnt__IsValidFontAtlas(const fntFontAtlas *atlas) {
		if (atlas == NULL) return(false);

		if (atlas->codePointsToPageIndices == NULL) return(false);

		return(true);
	}

	static bool fnt__IsValidFontContext(const fntFontContext *context) {
		if (context == NULL) return(false);

		const fnt__STBFontContext *internalCtx = (const fnt__STBFontContext *)context;

		if (internalCtx->maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(false);

		if (!fnt__IsValidFontInfo(&internalCtx->info)) return(false);

		if (!fnt__IsValidFontData(&internalCtx->data)) return(false);

		if (internalCtx->sinfo.data != internalCtx->data.data) return(false);

		return(true);
	}

	static void fnt__NewPack(fnt__STBFontContext *internalCtx, uint8_t *pixels) {
		FNT_ASSERT(internalCtx != NULL);
		FNT_ASSERT(pixels != NULL);
		stbtt_pack_context *packCtx = &internalCtx->currentPackContext;
		stbtt_PackBegin(packCtx, (unsigned char *)pixels, internalCtx->maxBitmapSize, internalCtx->maxBitmapSize, 0, 1, NULL);
		internalCtx->hasPackContext = 1;
	}

	static void fnt__FinishPack(fnt__STBFontContext *internalCtx) {
		FNT_ASSERT(internalCtx != NULL);
		if (internalCtx->hasPackContext) {
			stbtt_PackEnd(&internalCtx->currentPackContext);
			internalCtx->hasPackContext = 0;
		}
	}

	static uint32_t fnt__AddPage(const fnt__STBFontContext *context, fntFontAtlas *atlas, const uint32_t bitmapIndex, const uint32_t codePointStart, const uint32_t codePointCount, const stbtt_packedchar *packedChars) {
		FNT_ASSERT(context != NULL);
		FNT_ASSERT(atlas != NULL);
		FNT_ASSERT(bitmapIndex != UINT32_MAX);
		FNT_ASSERT(codePointCount > 0);
		FNT_ASSERT(packedChars != NULL);

		size_t glyphsSize = sizeof(fntFontGlyph) * codePointCount;
		fntFontGlyph *glyphs = (fntFontGlyph *)FNT_MALLOC(glyphsSize);
		if (glyphs == NULL) {
			return(UINT32_MAX);
		}
		FNT_MEMSET(glyphs, 0, glyphsSize);

		const stbtt_fontinfo *fontInfo = &context->sinfo;

		uint32_t endCodePointPastOne = codePointStart + codePointCount;

		for (uint32_t glyphIndex = 0; glyphIndex < codePointCount; ++glyphIndex) {
			uint32_t codePoint = codePointStart + glyphIndex;

			const stbtt_packedchar *packedChar = packedChars + glyphIndex;

			FNT_ASSERT(packedChar->codePoint == codePoint);

			fntFontGlyph *targetGlyph = glyphs + glyphIndex;

			uint16_t w = (packedChar->x1 - packedChar->x0) + 1;
			uint16_t h = (packedChar->y1 - packedChar->y0) + 1;

			float xoffset0 = packedChar->xoff;
			float yoffset0 = packedChar->yoff;
			float xoffset1 = packedChar->xoff2;
			float yoffset1 = packedChar->yoff2;

			targetGlyph->bitmapX = packedChar->x0;
			targetGlyph->bitmapY = packedChar->y0;
			targetGlyph->width = w;
			targetGlyph->height = h;
			targetGlyph->baselineOffsets[0] = fnt__MakeVec2(xoffset0, yoffset0);
			targetGlyph->baselineOffsets[1] = fnt__MakeVec2(xoffset1, yoffset1);
			targetGlyph->horizontalAdvance = packedChar->xadvance;
			targetGlyph->codePoint = codePoint;
		}

		uint32_t oldCount = atlas->pageCount;
		uint32_t pageIndex = oldCount;
		uint32_t newCount = oldCount + 1;
		fntFontPage *newPages = (fntFontPage *)FNT_REALLOC(atlas->pages, sizeof(fntFontPage) * newCount);
		if (newPages == NULL) return(UINT32_MAX);
		atlas->pages = newPages;
		atlas->pageCount = newCount;

		fntFontPage *newPage = newPages + pageIndex;
		FNT_MEMSET(newPage, 0, sizeof(*newPage));
		newPage->bitmapIndex = bitmapIndex;
		newPage->codePointStart = codePointStart;
		newPage->codePointCount = codePointCount;
		newPage->glyphs = glyphs;

		for (uint32_t codePointIndex = codePointStart; codePointIndex < endCodePointPastOne; ++codePointIndex) {
			atlas->codePointsToPageIndices[codePointIndex] = pageIndex + 1; // We store page (index +1) instead, so zero is invalid
		}

		return(pageIndex);
	}

	static void fnt__FreePage(fntFontPage *page) {
		FNT_ASSERT(page != NULL);
		fnt__FreeNotNull(page->glyphs);
		page->glyphs = NULL;
	}

	// https://en.wikipedia.org/wiki/Whitespace_character
	// Table containing all the whitespaces, mapped to a codepoint
	static uint32_t fnt__global__whitespaceCodepointsMap[] = {
		// Unicode characters with property White_Space=yes
		0x0009,	// HT, Basic Latin (\t)
		0x000A,	// LF, Basic Latin (\n)
		0x000B,	// VT, Basic Latin (\v)
		0x000C,	// FF, Basic Latin (\f)
		0x000D,	// CR, Basic Latin (\r)
		0x0020,	// Space, Basic Latin ( )
		0x0085,	// NEL, Next line
		0x00A0,	// Non-breaking space, identical to U+0020 ( )
		0x1680,	// Ogham Space Mark, Ogham
		0x2000,	// EN Quad, General Punctuation
		0x2001,	// EM Quad, General Punctuation
		0x2002,	// EN Space, General Punctuation
		0x2003,	// EM Space, General Punctuation
		0x2004, // Thick space, General Punctuation
		0x2005, // Mid space, General Punctuation
		0x2006, // Sixth of an em wide, General Punctuation
		0x2007, // Figure Space, General Punctuation
		0x2008, // Narrow punctuation, General Punctuation
		0x2009, // Thin Space, General Punctuation
		0x200A, // Hair Space, General Punctuation
		0x2028, // Line Separator, General Punctuation
		0x2029, // Paragraph Separator, General Punctuation
		0x202F, // Narrow no-break space, General Punctuation
		0x205F, // MMSP, General Punctuation
		0x3000, // CJK char, CJK Symbols and Punctuation

		// Related Unicode characters property White_Space=no
		0x180E, // MVS, Mongolian
		0x200B, // ZWSP, zero-width space, General Punctuation
		0x200C, // ZWNJ, zero-width non-joiner, General Punctuation
		0x200D, // ZWJ, zero-width joiner, General Punctuation
		0x2060, // WJ, word joiner, General Punctuation 
		0xFEFF, // WJ, Zero-width non-breaking space, Arabic Presentation Forms-B
	};

	fnt_api bool fntLoadFontInfo(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize) {
		if (!fnt__IsValidFontData(data)) return(false);
		if (info == NULL) return(false);
		if (fontSize < FNT__MIN_FONT_SIZE) return(false);

		int fontOffset = stbtt_GetFontOffsetForIndex(data->data, fontIndex);
		if (fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo sinfo;
		if (!stbtt_InitFont(&sinfo, data->data, fontOffset)) {
			return(false);
		}

		float pixelScale = stbtt_ScaleForPixelHeight(&sinfo, fontSize);

		// Get vertical metrics
		int ascentRaw, descentRaw, lineGapRaw;
		stbtt_GetFontVMetrics(&sinfo, &ascentRaw, &descentRaw, &lineGapRaw);

		// Allocate states & advancements for whitespaces
		const uint32_t numWhitespaceCodePoints = sizeof(fnt__global__whitespaceCodepointsMap) / sizeof(fnt__global__whitespaceCodepointsMap[0]);
		uint32_t *whitespaceStates = FNT_MALLOC(sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		float *whitespaceAdvancements = FNT_MALLOC(sizeof(float) * FNT__MAX_UNICODE_POINT_COUNT);
		if (whitespaceStates == NULL || whitespaceAdvancements == NULL) {
			fnt__FreeNotNull(whitespaceStates);
			fnt__FreeNotNull(whitespaceAdvancements);
			return(false);
		}
		FNT_MEMSET(whitespaceStates, 0, sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		FNT_MEMSET(whitespaceAdvancements, 0, sizeof(float) * FNT__MAX_UNICODE_POINT_COUNT);

		// Write states & advancements for whitespaces
		for (uint32_t whitespaceCodeIndex = 0; whitespaceCodeIndex < numWhitespaceCodePoints; ++whitespaceCodeIndex) {
			uint32_t whitespaceCodePoint = fnt__global__whitespaceCodepointsMap[whitespaceCodeIndex];

			int advanceRaw, leftSideBearing;
			stbtt_GetCodepointHMetrics(&sinfo, whitespaceCodePoint, &advanceRaw, &leftSideBearing);

			FNT_ASSERT(whitespaceCodePoint < FNT__MAX_UNICODE_POINT_COUNT);
			whitespaceStates[whitespaceCodePoint] = 1;
			whitespaceAdvancements[whitespaceCodePoint] = (float)advanceRaw * pixelScale;
		}

		FNT_MEMSET(info, 0, sizeof(*info));
		info->fontIndex = fontIndex;
		info->name = data->name;
		info->fontSize = fontSize;
		info->heightToScale = pixelScale;
		info->ascent = ascentRaw * pixelScale;
		info->descent = descentRaw * pixelScale;
		info->whitespaceTable.advancements = whitespaceAdvancements;
		info->whitespaceTable.states = whitespaceStates;

		return(true);
	}

	fnt_api void fntFreeFontInfo(fntFontInfo *info) {
		if (info == NULL) return;
		fnt__FreeNotNull(info->whitespaceTable.advancements);
		fnt__FreeNotNull(info->whitespaceTable.states);
		FNT_MEMSET(info, 0, sizeof(*info));
	}

	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas) {
		if (atlas == NULL) return;
		fnt__FreeNotNull(atlas->pages);
		fnt__FreeNotNull(atlas->bitmaps);
		fnt__FreeNotNull(atlas->codePointsToPageIndices);
		fnt__FreeNotNull(atlas->kerningValues);
		fnt__FreeNotNull(atlas->kerningPages);
		FNT_MEMSET(atlas, 0, sizeof(*atlas));
	}

	fnt_api bool fntInitFontAtlas(const fntFontInfo *info, fntFontAtlas *atlas) {
		if (info == NULL || atlas == NULL) return(false);

		size_t pageIndicesSize = sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT;
		uint32_t *pageIndices = (uint32_t *)FNT_MALLOC(pageIndicesSize);
		if (pageIndices == NULL) {
			return(false);
		}
		FNT_MEMSET(pageIndices, 0, pageIndicesSize);

		FNT_MEMSET(atlas, 0, sizeof(*atlas));
		atlas->codePointsToPageIndices = pageIndices;

		return(true);
	}

	static uint32_t fnt__AddBitmap(fntFontAtlas *atlas, const uint32_t width, const uint32_t height) {
		FNT_ASSERT(atlas != NULL);
		FNT_ASSERT(width >= FNT__MIN_BITMAP_SIZE);
		FNT_ASSERT(height >= FNT__MIN_BITMAP_SIZE);

		// @TODO(final): Collapse into one memory allocation (fntBitmap + pixels)

		uint32_t oldCount = atlas->bitmapCount;
		uint32_t newCount = oldCount + 1;
		fntBitmap *newBitmaps = (fntBitmap *)FNT_REALLOC(atlas->bitmaps, sizeof(fntBitmap) * newCount);
		if (newBitmaps == NULL) return(UINT32_MAX);

		uint32_t index = oldCount;

		size_t bitmapLen = sizeof(uint8_t) * width * height;

		fntBitmap *newBitmap = newBitmaps + index;
		newBitmap->width = width;
		newBitmap->height = height;
		newBitmap->format = fntFontBitmapFormat_Alpha8;
		newBitmap->pixels = (uint8_t *)FNT_MALLOC(bitmapLen);
		FNT_MEMSET((uint8_t *)newBitmap->pixels, 0, bitmapLen);

		atlas->bitmaps = newBitmaps;
		atlas->bitmapCount = newCount;

		return(index);
	}

	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const uint32_t codePointStart, const uint32_t codePointCount) {
		if (!fnt__IsValidFontContext(context) || !fnt__IsValidFontAtlas(atlas)) return(false);

		if ((codePointCount == 0) || ((uint64_t)codePointStart + (uint64_t)codePointCount > (uint64_t)FNT__MAX_UNICODE_POINT_COUNT)) return(false);

		size_t packedCharsSize = sizeof(stbtt_packedchar) * codePointCount;
		stbtt_packedchar *packedChars = (stbtt_packedchar *)FNT_MALLOC(packedCharsSize);
		if (packedChars == NULL) {
			return(false);
		}
		FNT_MEMSET(packedChars, 0, packedCharsSize);

		uint32_t currentCodePointIndex = 0;
		uint32_t currentCodePointStart = codePointStart;
		uint32_t remainingCodePointCount = codePointCount;

		uint32_t pageCodePointIndex = 0;
		uint32_t pageCodePointStart = codePointStart;
		uint32_t totalPageCodePointCount = 0;

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
		const float fontSize = internalCtx->info.fontSize;
		stbtt_fontinfo *sinfo = &internalCtx->sinfo;
		stbtt_pack_context *packCtx = &internalCtx->currentPackContext;

		while (remainingCodePointCount > 0) {
			if (internalCtx->bitmapIndex == UINT32_MAX) {
				internalCtx->bitmapIndex = fnt__AddBitmap(atlas, internalCtx->maxBitmapSize, internalCtx->maxBitmapSize);

				FNT_ASSERT(internalCtx->bitmapIndex < atlas->bitmapCount);
				fntBitmap *bitmap = atlas->bitmaps + internalCtx->bitmapIndex;

				FNT_MEMSET(packCtx, 0, sizeof(*packCtx));
				fnt__NewPack(internalCtx, (uint8_t *)bitmap->pixels);
			}

			uint32_t bitmapIndex = internalCtx->bitmapIndex;

			stbtt_packedchar *currentPackedChars = packedChars + currentCodePointIndex;

			stbtt_pack_range range = FNT__ZERO_INIT;
			range.font_size = fontSize;
			range.num_chars = remainingCodePointCount;
			range.first_unicode_codepoint_in_range = currentCodePointStart;
			range.chardata_for_range = currentPackedChars;

			int packResult = stbtt_PackFontRanges(packCtx, internalCtx->data.data, internalCtx->info.fontIndex, &range, 1);
			if (packResult) {
				totalPageCodePointCount += range.num_chars;
				remainingCodePointCount -= range.num_chars;
				FNT_ASSERT(remainingCodePointCount == 0);
				fnt__AddPage(internalCtx, atlas, bitmapIndex, range.first_unicode_codepoint_in_range, range.num_chars, currentPackedChars);
				currentCodePointIndex += range.num_chars;
				break; // Finished, but leave bitmap intact - no need to reset any index states here
			} else {
				uint32_t numChars = 0;
				for (uint32_t charIndex = 0; charIndex < (uint32_t)range.num_chars; ++charIndex) {
					stbtt_packedchar *packedChar = currentPackedChars + charIndex;
					if (packedChar->x0 == 0 && packedChar->x1 == 0 && packedChar->y0 == 0 && packedChar->y1 == 0) {
						break;
					}
					++numChars;
				}

				if (numChars == 0) {
					// Page is complete, start another page
					stbtt_packedchar *pagePackedChars = packedChars + pageCodePointIndex;
					FNT_ASSERT(pagePackedChars->codePoint == pageCodePointStart);
					fnt__AddPage(internalCtx, atlas, bitmapIndex, pageCodePointStart, totalPageCodePointCount, pagePackedChars);
					pageCodePointIndex = currentCodePointIndex;
					pageCodePointStart = currentCodePointStart;
					totalPageCodePointCount = 0;

					// Bitmap is complete, start another bitmap in the next iteration and finish the pack
					fnt__FinishPack(internalCtx);
					internalCtx->bitmapIndex = UINT32_MAX;
				} else {
					remainingCodePointCount -= numChars;
					currentCodePointStart += numChars;
					currentCodePointIndex += numChars;

					// Page is incomplete, just increase the total count
					totalPageCodePointCount += numChars;
				}
			}
		}

		FNT_ASSERT(currentCodePointIndex == codePointCount);

		FNT_FREE(packedChars);

		return(true);
	}

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info, const uint32_t maxBitmapSize) {
		if (!fnt__IsValidFontData(data) || !fnt__IsValidFontInfo(info))return(NULL);
		if (maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(NULL);

		int fontOffset = stbtt_GetFontOffsetForIndex(data->data, info->fontIndex);
		if (fontOffset < 0) {
			return(NULL);
		}

		stbtt_fontinfo sinfo;
		if (!stbtt_InitFont(&sinfo, data->data, fontOffset)) {
			return(NULL);
		}

		fnt__STBFontContext *result = (fnt__STBFontContext *)FNT_MALLOC(sizeof(fnt__STBFontContext));
		if (result == NULL) {
			return(NULL);
		}

		FNT_MEMSET(result, 0, sizeof(*result));
		result->data = *data;
		result->info = *info;
		result->maxBitmapSize = maxBitmapSize;
		result->sinfo = sinfo;
		result->bitmapIndex = UINT32_MAX;

		return((void *)result);
	}

	fnt_api bool fntComputeAtlasKernings(const fntFontContext *context, fntFontAtlas *atlas) {
		if (context == NULL || atlas == NULL) return(false);

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;

		const stbtt_fontinfo *sinfo = &internalCtx->sinfo;

		const float heightToScale = internalCtx->info.heightToScale;

		const uint32_t pageCount = atlas->pageCount;

		// Compute the total number of code points from all pages
		uint32_t totalCodePointCount = 0;
		for (uint32_t pageIndexA = 0; pageIndexA < pageCount; ++pageIndexA) {
			const fntFontPage *pageA = atlas->pages + pageIndexA;
			uint32_t countA = pageA->codePointCount;
			for (uint32_t pageIndexB = 0; pageIndexB < pageCount; ++pageIndexB) {
				const fntFontPage *pageB = atlas->pages + pageIndexB;
				uint32_t countB = pageB->codePointCount;
				totalCodePointCount += countA * countB;
			}
		}

		// Allocate kerning values for all codepoint pairs
		size_t valuesSize = sizeof(float) * totalCodePointCount;
		float *values = (float *)FNT_MALLOC(valuesSize);
		if (values == NULL) {
			return(false);
		}
		FNT_MEMSET(values, 0, valuesSize);

		// Allocate page pairs for all pages
		const uint32_t pagePairCount = pageCount * pageCount;
		size_t pagePairsSize = sizeof(fntKerningPagePair) * pagePairCount;
		fntKerningPagePair *pagePairs = (fntKerningPagePair *)FNT_MALLOC(pagePairsSize);
		if (pagePairs == NULL) {
			FNT_FREE(values);
			return(false);
		}
		FNT_MEMSET(pagePairs, 0, pagePairsSize);

		// Compute kerning pairs
		size_t valuesIndex = 0;
		for (uint32_t pageIndexA = 0; pageIndexA < pageCount; ++pageIndexA) {
			const fntFontPage *pageA = atlas->pages + pageIndexA;
			for (uint32_t pageIndexB = 0; pageIndexB < pageCount; ++pageIndexB) {
				const fntFontPage *pageB = atlas->pages + pageIndexB;

				size_t pageCodePointCount = pageA->codePointCount * pageB->codePointCount;

				size_t pagePairIndex = pageIndexA * pageCount + pageIndexB;
				fntKerningPagePair *pagePair = pagePairs + pagePairIndex;
				pagePair->valueIndex = valuesIndex;
				pagePair->pageIndexA = pageIndexA;
				pagePair->pageIndexB = pageIndexB;
				pagePair->pairCount = pageCodePointCount;

				float *pagePairValues = values + valuesIndex;

				for (uint32_t glyphIndexA = 0; glyphIndexA < pageA->codePointCount; ++glyphIndexA) {
					for (uint32_t glyphIndexB = 0; glyphIndexB < pageB->codePointCount; ++glyphIndexB) {
						uint32_t codePointA = pageA->codePointStart + glyphIndexA;
						uint32_t codePointB = pageB->codePointStart + glyphIndexB;
						float kerning = 0.0f;
						if (codePointA != codePointB) {
							int kerningRaw = stbtt_GetCodepointKernAdvance(sinfo, (int)codePointA, (int)codePointB);
							kerning = (float)kerningRaw * heightToScale;
						}
						size_t valueIndex = glyphIndexA * pageA->codePointCount + glyphIndexB;
						pagePairValues[valueIndex] = kerning;
					}
				}

				valuesIndex += pageCodePointCount;
			}
		}

		atlas->kerningValues = values;
		atlas->kerningPages = pagePairs;

		return(true);
	}

	fnt_api void fntReleaseFontContext(fntFontContext *context) {
		if (context != NULL) {
			fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
			fnt__FinishPack(internalCtx);
			FNT_FREE(internalCtx);
		}
	}

	// See: https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
	static const uint8_t fnt__global__UTF8_DecodeTable[] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
		0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
		0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
		0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
		1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
		1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
		1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
	};

	typedef enum fnt__UTF8State {
		fnt__UTF8State_Accept = 0,
		fnt__UTF8State_Reject = 1,
	} fnt__UTF8State;

	inline uint32_t fnt__TestDecodeUTF8(uint32_t *state, const uint32_t byte) {
		uint32_t type = fnt__global__UTF8_DecodeTable[byte];
		*state = fnt__global__UTF8_DecodeTable[256 + *state * 16 + type];
		return *state;
	}

	inline uint32_t fnt__DecodeUTF8(uint32_t *state, uint32_t *codePoint, const uint32_t byte) {
		uint32_t type = fnt__global__UTF8_DecodeTable[byte];
		*codePoint = (*state != fnt__UTF8State_Accept) ? (byte & 0x3fu) | (*codePoint << 6) : (0xff >> type) & (byte);
		*state = fnt__global__UTF8_DecodeTable[256 + *state * 16 + type];
		return *state;
	}

	static size_t fnt__CountUTF8String(const char *text) {
		const uint8_t *s = (const uint8_t *)text;
		size_t count;
		uint32_t state = 0;
		for (count = 0; *s; ++s) {
			if (fnt__TestDecodeUTF8(&state, *s) == fnt__UTF8State_Accept) {
				++count;
			}
		}
		if (state == fnt__UTF8State_Accept) {
			return(count);
		} else {
			return(0); // Malformed UTF-8 string
		}
	}

	fnt_api size_t fntGetQuadCountFromUTF8(const fntFontAtlas *atlas, const char *utf8) {
		if (utf8 == NULL) return(0);
		size_t result = fnt__CountUTF8String(utf8);
		return(result);
	}

	inline bool fnt__IsCodePointWhiteSpace(const fntFontInfo *info, const uint32_t codePoint) {
		FNT_ASSERT(info != NULL);
		bool result = info->whitespaceTable.states[codePoint] != 0;
		return(result);
	}

	fnt_api bool fntComputeQuadsFromUTF8(const fntFontAtlas *atlas, const fntFontInfo *info, const char *utf8, const float charHeight, const fntComputeQuadsFlags flags, const size_t maxQuadCount, fntFontQuad *outQuads, fntVec2 *outSize) {
		if (atlas == NULL || info == NULL || utf8 == NULL || charHeight <= 0.0f || maxQuadCount == 0)  return(false);

		size_t codePointCount = fnt__CountUTF8String(utf8);
		if (codePointCount == 0 || maxQuadCount < codePointCount) return(false);

		const float pixelScale = info->heightToScale;

		const uint8_t *s = (const uint8_t *)utf8;
		uint32_t codePointIndex;
		uint32_t codePoint;
		uint32_t state = 0;
		const fntFontPage *currentPage = NULL;
		const fntBitmap *currentBitmap = NULL;
		uint32_t currentPageNum = 0;
		uint32_t currentBitmapIndex = UINT32_MAX;
		for (codePointIndex = 0; *s; ++s) {
			if (fnt__DecodeUTF8(&state, &codePoint, *s) == fnt__UTF8State_Accept) {

				// Decode next code point (for kerning)
				uint32_t nextState = 0;
				uint32_t nextCodePoint = 0;
				uint32_t hasNextCodePoint = 0;
				const uint8_t *tmp = s;
				int tmpLen = 0;
				while (*++tmp && tmpLen < 4) {
					if (fnt__DecodeUTF8(&state, &nextCodePoint, *tmp) == fnt__UTF8State_Accept) {
						hasNextCodePoint = 1;
						break;
					}
					++tmpLen;
				}

				if (fnt__IsCodePointWhiteSpace(info, nextCodePoint)) {
					// Any whitespace is a special character, so dont accept it for kerning
					nextCodePoint = 0;
				}

				uint32_t pageNumA = atlas->codePointsToPageIndices[codePoint];

				if (pageNumA == 0) {
					// TODO: Page not found, use a substitute character instead
					codePoint = 32;
				}

				if (fnt__IsCodePointWhiteSpace(info, codePoint)) {
					// TODO: Whitespace-character
				} else {
					FNT_ASSERT(pageNumA > 0);
					if (currentPageNum != pageNumA) {
						uint32_t pageIndex = pageNumA - 1;
						FNT_ASSERT(pageIndex < atlas->pageCount);
						currentPage = atlas->pages + pageIndex;
						currentPageNum = pageNumA;
					}

					FNT_ASSERT(currentPage != NULL);

					uint32_t bitmapIndex = currentPage->bitmapIndex;

					if (currentBitmapIndex != bitmapIndex) {
						FNT_ASSERT(bitmapIndex < atlas->bitmapCount);
						currentBitmapIndex = bitmapIndex;
						currentBitmap = atlas->bitmaps + bitmapIndex;
					}

					FNT_ASSERT(currentBitmap != NULL);

					const uint16_t bitmapWidth = currentBitmap->width;
					const uint16_t bitmapHeight = currentBitmap->height;
					const float texelU = 1.0f / (float)bitmapWidth;
					const float texelV = 1.0f / (float)bitmapHeight;

					FNT_ASSERT(codePoint >= currentPage->codePointStart && codePoint < currentPage->codePointStart + currentPage->codePointCount);

					uint32_t glyphIndex = codePoint - currentPage->codePointStart;
					const fntFontGlyph *glyph = currentPage->glyphs + glyphIndex;

					FNT_ASSERT(glyph->codePoint == codePoint);

					float u0 = (float)glyph->bitmapX * texelU;
					float v0 = (float)glyph->bitmapY * texelV;
					float u1 = (float)(glyph->bitmapX + glyph->width) * texelU;
					float v1 = (float)(glyph->bitmapY + glyph->height) * texelV;

					float width = (float)glyph->width * pixelScale * charHeight;
					float height = (float)glyph->height * pixelScale * charHeight;

					float xoffset0 = 0;
					float yoffset0 = 0;
					float xoffset1 = width;
					float yoffset1 = height;

					if (outQuads != NULL) {
						fntFontQuad *quad = &outQuads[codePointIndex];
						quad->bitmapIndex = bitmapIndex;
						quad->codePoint = codePoint;
						quad->uv[0] = fnt__MakeVec2(u0, v0);
						quad->uv[1] = fnt__MakeVec2(u1, v1);
						quad->rect[0] = fnt__MakeVec2(xoffset0, yoffset0);
						quad->rect[1] = fnt__MakeVec2(xoffset1, yoffset1);
					}

					float kerning = 0.0f;
					if (hasNextCodePoint && atlas->kerningPages != NULL && nextCodePoint > 0) {
						uint32_t pageIndexA = pageNumA - 1;

						uint32_t pageNumB = atlas->codePointsToPageIndices[nextCodePoint];
						FNT_ASSERT(pageNumB > 0);

						uint32_t pageIndexB = pageNumB - 1;
						FNT_ASSERT(pageIndexB < atlas->pageCount);

						const fntFontPage *pageA = currentPage;
						const fntFontPage *pageB = atlas->pages + pageIndexB;

						FNT_ASSERT(codePoint >= pageA->codePointStart);
						FNT_ASSERT(nextCodePoint >= pageB->codePointStart);

						uint32_t kerningPagePairIndex = pageIndexA * atlas->pageCount + pageIndexB;

						const fntKerningPagePair *pagePair = atlas->kerningPages + kerningPagePairIndex;

						size_t firstValueIndex = pagePair->valueIndex;

						size_t codePointIndexA = codePoint - pageA->codePointStart;
						size_t codePointIndexB = nextCodePoint - pageB->codePointStart;

						size_t valueIndex = codePointIndexA * pageA->codePointCount + codePointIndexB;

						kerning = atlas->kerningValues[valueIndex];
					}

					// @TODO(final): Compute x and y advancement
				}
				++codePointIndex;
			}
		}

		return(true);
	}

	typedef struct {
		FNT_FILE_HANDLE handle;
	} fnt__SaveBitmapFileContext;

	static void fnt__SaveBitmapDataFunc(void *context, void *data, int size) {
		fnt__SaveBitmapFileContext *fileContext = (fnt__SaveBitmapFileContext *)context;
		FNT_WRITE_TO_FILE(fileContext->handle, data, size);
	}

	fnt_api bool fntSaveBitmapToFile(const fntBitmap *bitmap, const char *filePath) {
		fnt__SaveBitmapFileContext context = FNT__ZERO_INIT;
		if (!FNT_CREATE_BINARY_FILE(filePath, &context.handle)) {
			return(false);
		}

		int components = bitmap->format == fntFontBitmapFormat_Alpha8 ? 1 : 3;
		stbi_write_bmp_to_func(fnt__SaveBitmapDataFunc, &context, bitmap->width, bitmap->height, components, bitmap->pixels);

		FNT_CLOSE_FILE(context.handle);
		return(true);
	}

#ifdef __cplusplus
	}
#endif

#endif // FNT_IMPLEMENTATION