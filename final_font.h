/***
final_font.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

An open source single-header-library bitmap font atlas generator.

This library is designed to load a TTF/TTC font and create font/bitmaps atlases from it.
The main purpose of this library is to prepare for rendering fonts for custom rendering engines.
It supports multiple code-ranges in a single atlas

-------------------------------------------------------------------------------
	Limitations
-------------------------------------------------------------------------------

- Performance of kerning factor loading is really poor, due to the N^2+N algorythmn used.
  Its recommend to compute this in a offline process.

-------------------------------------------------------------------------------
	Dependencies
-------------------------------------------------------------------------------

- Built-in operating system libraries

- C99 complaint or C++ compiler

- STB_truetype.h

- Bare minimum linking:
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
#else
#	if !defined(FNT_MALLOC) || !defined(FNT_REALLOC) || !defined(FNT_FREE)
#		error "Please implement all the required memory allocation macros: FNT_MALLOC, FNT_REALLOC, FNT_FREE)
#	endif
#endif

// Memset
#if !defined(FNT_MEMSET)
#	include <string.h>
#	define FNT_MEMSET(dst, value, size) memset(dst, value, size)
#	define FNT_MEMCOPY(dst, src, size) memcpy(dst, src, size)
#else
#	if !defined(FNT_MEMSET) || !defined(FNT_MEMCOPY)
#		error "Please implement all the required memory set macros: FNT_MEMSET, FNT_MEMCOPY)
#	endif
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
#	include <stdio.h>
#	define FNT_FILE_HANDLE FILE *
#	define FNT_CREATE_BINARY_FILE(filePath, fileHandle) (fopen_s(fileHandle, filePath, "wb") == 0)
#	define FNT_WRITE_TO_FILE(fileHandle, buffer, size) fwrite(buffer, size, 1, fileHandle)
#	define FNT_CLOSE_FILE(fileHandle) fclose(fileHandle)
#else
#	if !defined(FNT_FILE_HANDLE) || !defined(FNT_CREATE_BINARY_FILE) || !defined(FNT_WRITE_TO_FILE) || !defined(FNT_CLOSE_FILE)
#		error "Please implement all the required File/IO macros: FNT_FILE_HANDLE, FNT_CREATE_BINARY_FILE, FNT_WRITE_TO_FILE, FNT_CLOSE_FILE)
#	endif
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
		//! The font index
		uint32_t index;
	} fntFontData;

	typedef enum fntBitmapFormat {
		//! 8-bit alpha only
		fntFontBitmapFormat_Alpha8 = 0,
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

	typedef struct fntBitmapRect {
		//! The left-position in the bitmap
		uint16_t x;
		//! The top-position in the bitmap
		uint16_t y;
		//! The width in the bitmap
		uint16_t width;
		//! The height in the bitmap
		uint16_t height;
	} fntBitmapRect;

	typedef enum fntWhiteSpaceState {
		fntWhiteSpaceState_No = 0,
		fntWhiteSpaceState_WhiteSpace,
	} fntWhiteSpaceState;

	typedef struct fntWhiteSpaceTable {
		//! The table that maps from a code-point to a number 0/1 that indicates if the code-point is a whitespace or not
		fntWhiteSpaceState *states;
		//! The horizontal whitespace mapped to all code-points in pixels
		float *advancements;
	} fntWhiteSpaceTable;

	typedef struct fntFontSize {
		//! The font size in pixels
		float f32;
	} fntFontSize;

	typedef struct fntCodePoint {
		//! The 16-bit unicode codepoint
		uint16_t u16;
	} fntCodePoint;

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

	typedef struct fntBounds {
		//! The offset to the left
		float left;
		//! The offset to the right
		float right;
		//! The offset to the top
		float top;
		//! The offset to the bottom
		float bottom;
	} fntBounds;

	typedef struct fntFontGlyph {
		//! The baseline offset in pixels
		fntVec2 offset;
		//! The size in pixels
		fntVec2 size;
		//! The bitmap rectangle
		fntBitmapRect bitmapRect;
		//! The default advancement to any next character in pixels
		float horizontalAdvance;
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The code point for validation
		fntCodePoint codePoint;
	} fntFontGlyph;

	typedef struct fntFontPage {
		//! The kerning values for glyph pairs in pixels
		float *kerningValues;
		//! The array of font glyphs
		fntFontGlyph *glyphs;
		//! The first code point
		fntCodePoint codePointStart;
		//! The number of code points (N)
		fntCodePoint codePointCount;
	} fntFontPage;

	typedef struct fntFontAtlas {
		//! The whitespace table
		fntWhiteSpaceTable whitespaceTable;
		//! The array of font pages
		fntFontPage *pages;
		//! The array of alpha bitmaps
		fntBitmap *bitmaps;
		//! The array of code-points mapped to a font page number in range of 1 to N, zero means not-set
		uint32_t *codePointsToPageIndices;
		//! The font size
		fntFontSize fontSize;
		//! The ascent from the baseline in pixels
		float ascent;
		//! The descent from the baseline in pixels
		float descent;
		//! The linegap in pixels
		float lineGap;
		//! The number of pages
		uint32_t pageCount;
		//! The number of bitmaps
		uint32_t bitmapCount;
	} fntFontAtlas;

	typedef void fntFontContext;
	fnt_api fntFontContext *fntCreateFontContext(const uint32_t maxBitmapSize);
	fnt_api void fntReleaseFontContext(fntFontContext *context);

	fnt_api bool fntInitFontAtlas(fntFontAtlas *atlas, const fntFontData *fontData, const fntFontSize fontSize);
	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const fntFontData *fontData, const fntFontSize fontSize, const fntCodePoint codePointStart, const fntCodePoint codePointEnd);
	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas);

	fnt_api bool fntSaveBitmapToFile(const fntBitmap *bitmap, const char *filePath);

	typedef struct fntFontQuad {
		//! The UV coordinates (bottom-left, top-right)
		fntVec2 uv[2];
		//! The output coordinates (top-left, bottom-right)
		fntVec2 coords[2];
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The code point
		fntCodePoint codePoint;
	} fntFontQuad;

	// Top-Left aligned, Top-Down is default
	typedef enum fntComputeQuadsFlags {
		fntComputeQuadsFlags_None = 0,
		fntComputeQuadsFlags_Cartesian = 1 << 0,
		fntComputeQuadsFlags_WithoutKerning = 1 << 1,
		fntComputeQuadsFlags_GlyphAdvancement = 1 << 2,
	} fntComputeQuadsFlags;

	typedef struct fntComputeQuadConfig {
		float additionalAdvancement;
		fntComputeQuadsFlags flags;
	} fntComputeQuadConfig;

	typedef struct fntTextSize {
		float totalWidth;
		float totalHeight;
		float baselineOffset;
		uint32_t lineCount;
	} fntTextSize;

	fnt_api bool fntGetFontMetrics(const fntFontAtlas *atlas, const float charScale, float *outAscent, float *outDescent, float *outLineGap);

	fnt_api size_t fntGetQuadCountFromUTF8(const char *utf8);
	fnt_api bool fntComputeQuadsFromUTF8(const fntFontAtlas *atlas, const char *utf8, const float charScale, const fntComputeQuadsFlags flags, const size_t numQuads, fntFontQuad *outQuads, fntBounds *outBounds, uint32_t *outLineCount, float *outBaselineOffset);
	fnt_api fntTextSize fntComputeTextSizeFromUTF8(const fntFontAtlas *atlas, const char *utf8, const float charScale, const fntComputeQuadsFlags flags);

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
#define FNT__MAX_UNICODE_POINT_COUNT (uint16_t)0xFFFF

#define FNT__MIN_BITMAP_SIZE 32

#define FNT__MIN_FONT_SIZE 4

#define FNT__MIN(a, b) ((a) < (b) ? (a) : (b))
#define FNT__MAX(a, b) ((a) > (b) ? (a) : (b))

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

	static void fnt__FreeWhiteSpaceTable(fntWhiteSpaceTable *whitespaceTable) {
		FNT_ASSERT(whitespaceTable != NULL);
		fnt__FreeNotNull(whitespaceTable->advancements);
		fnt__FreeNotNull(whitespaceTable->states);
		FNT_MEMSET(whitespaceTable, 0, sizeof(*whitespaceTable));
	}

	static void fnt__InitWhiteSpaceTable(fntWhiteSpaceTable *whitespaceTable, const stbtt_fontinfo *sinfo, const float fontSize) {
		FNT_ASSERT(whitespaceTable != NULL);
		FNT_ASSERT(sinfo != NULL);
		FNT_ASSERT(fontSize >= FNT__MIN_FONT_SIZE);

		FNT_MEMSET(whitespaceTable, 0, sizeof(*whitespaceTable));

		const float rawToPixels = stbtt_ScaleForPixelHeight(sinfo, fontSize);

		// Allocate states & advancements for whitespaces
		const uint32_t numWhitespaceCodePoints = sizeof(fnt__global__whitespaceCodepointsMap) / sizeof(fnt__global__whitespaceCodepointsMap[0]);
		fntWhiteSpaceState *whitespaceStates = FNT_MALLOC(sizeof(fntWhiteSpaceState) * FNT__MAX_UNICODE_POINT_COUNT);
		float *whitespaceAdvancements = FNT_MALLOC(sizeof(float) * FNT__MAX_UNICODE_POINT_COUNT);
		if (whitespaceStates == NULL || whitespaceAdvancements == NULL) {
			fnt__FreeNotNull(whitespaceStates);
			fnt__FreeNotNull(whitespaceAdvancements);
			return;
		}
		FNT_MEMSET(whitespaceStates, 0, sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		FNT_MEMSET(whitespaceAdvancements, 0, sizeof(float) * FNT__MAX_UNICODE_POINT_COUNT);

		// Write states & advancements for whitespaces
		for (uint32_t whitespaceCodeIndex = 0; whitespaceCodeIndex < numWhitespaceCodePoints; ++whitespaceCodeIndex) {
			uint32_t whitespaceCodePoint = fnt__global__whitespaceCodepointsMap[whitespaceCodeIndex];

			int advanceRaw, leftSideBearingRaw;
			stbtt_GetCodepointHMetrics(sinfo, whitespaceCodePoint, &advanceRaw, &leftSideBearingRaw);

			FNT_ASSERT(whitespaceCodePoint < FNT__MAX_UNICODE_POINT_COUNT);
			whitespaceStates[whitespaceCodePoint] = fntWhiteSpaceState_WhiteSpace;
			whitespaceAdvancements[whitespaceCodePoint] = (float)advanceRaw * rawToPixels;
		}

		whitespaceTable->advancements = whitespaceAdvancements;
		whitespaceTable->states = whitespaceStates;
	}

	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas) {
		if (atlas == NULL) return;
		fnt__FreeWhiteSpaceTable(&atlas->whitespaceTable);
		fnt__FreeNotNull(atlas->codePointsToPageIndices);
		fnt__FreeNotNull(atlas->pages);
		fnt__FreeNotNull(atlas->bitmaps);
		FNT_MEMSET(atlas, 0, sizeof(*atlas));
	}

	fnt_api bool fntInitFontAtlas(fntFontAtlas *atlas, const fntFontData *fontData, const fntFontSize fontSize) {
		if (atlas == NULL || !fnt__IsValidFontData(fontData) || fontSize.f32 < FNT__MIN_FONT_SIZE) return(false);

		int fontOffset = stbtt_GetFontOffsetForIndex(fontData->data, fontData->index);
		if (fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo sinfo;
		if (!stbtt_InitFont(&sinfo, fontData->data, fontOffset)) {
			return(false);
		}

		const float rawToPixels = stbtt_ScaleForPixelHeight(&sinfo, fontSize.f32);

		// Get vertical metrics
		int ascentRaw, descentRaw, lineGapRaw;
		stbtt_GetFontVMetrics(&sinfo, &ascentRaw, &descentRaw, &lineGapRaw);

		size_t pageIndicesSize = sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT;
		uint32_t *pageIndices = (uint32_t *)FNT_MALLOC(pageIndicesSize);
		if (pageIndices == NULL) {
			return(false);
		}
		FNT_MEMSET(pageIndices, 0, pageIndicesSize);

		FNT_MEMSET(atlas, 0, sizeof(*atlas));
		atlas->codePointsToPageIndices = pageIndices;
		atlas->fontSize = fontSize;
		atlas->ascent = ascentRaw * rawToPixels;
		atlas->descent = descentRaw * rawToPixels;
		atlas->lineGap = lineGapRaw * rawToPixels;

		fnt__InitWhiteSpaceTable(&atlas->whitespaceTable, &sinfo, fontSize.f32);

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

	static void fnt__FreePage(fntFontPage *page) {
		FNT_ASSERT(page != NULL);
		fnt__FreeNotNull(page->glyphs);
		fnt__FreeNotNull(page->kerningValues);
		FNT_MEMSET(page, 0, sizeof(*page));
	}

	static fntFontPage *fnt__AppendPage(fntFontAtlas *atlas, const uint32_t codePointStart, const uint32_t codePointCount) {
		size_t glyphsSize = sizeof(fntFontGlyph) * codePointCount;
		size_t kerningValuesSize = sizeof(float) * codePointCount * codePointCount;

		uint32_t oldCount = atlas->pageCount;
		uint32_t pageIndex = oldCount;
		uint32_t newCount = oldCount + 1;

		fntFontPage *newPages = (fntFontPage *)FNT_REALLOC(atlas->pages, sizeof(fntFontPage) * newCount);
		fntFontGlyph *glyphs = (fntFontGlyph *)FNT_MALLOC(glyphsSize);
		if (newPages == NULL || glyphs == NULL) {
			fnt__FreeNotNull(newPages);
			fnt__FreeNotNull(glyphs);
			return(NULL);
		}
		FNT_MEMSET(glyphs, 0, glyphsSize);

		fntFontPage *newPage = newPages + pageIndex;
		FNT_MEMSET(newPage, 0, sizeof(*newPage));
		newPage->codePointStart.u16 = codePointStart;
		newPage->codePointCount.u16 = codePointCount;
		newPage->glyphs = glyphs;
		newPage->kerningValues = (float *)FNT_MALLOC(kerningValuesSize);

		atlas->pages = newPages;
		atlas->pageCount = newCount;

		uint32_t endCodePointPastOne = codePointStart + codePointCount;
		for (uint32_t codePointIndex = codePointStart; codePointIndex < endCodePointPastOne; ++codePointIndex) {
			atlas->codePointsToPageIndices[codePointIndex] = pageIndex + 1; // We store page (index +1) instead, so zero is invalid
		}

		return(newPage);
	}

	static void fnt__AddGlyphsToPage(fntFontGlyph *glyphs, const stbtt_fontinfo *fontInfo, const float rawToPixels, const uint32_t codePointStart, const uint32_t glyphCount, const uint32_t bitmapIndex, const stbtt_packedchar *packedChars) {
		FNT_ASSERT(glyphs != NULL);
		FNT_ASSERT(fontInfo != NULL);
		FNT_ASSERT(packedChars != NULL);
		FNT_ASSERT(glyphCount > 0);
		FNT_ASSERT(packedChars != NULL);

		for (uint32_t index = 0; index < glyphCount; ++index) {
			const uint32_t codePoint = codePointStart + index;

			int advanceRaw, leftSideBearingRaw;
			stbtt_GetCodepointHMetrics(fontInfo, codePoint, &advanceRaw, &leftSideBearingRaw);

			const stbtt_packedchar *packedChar = packedChars + index;
			FNT_ASSERT(packedChar->codePoint == codePoint);

			fntFontGlyph *targetGlyph = glyphs + index;

			float x0 = packedChar->xoff;
			float y0 = packedChar->yoff;
			float x1 = packedChar->xoff2;
			float y1 = packedChar->yoff2;

			float w = (float)(packedChar->xoff2 - packedChar->xoff);
			float h = (float)(packedChar->yoff2 - packedChar->yoff);

			float advance = packedChar->xadvance;

			// Bitmap coordinates (UV)
			targetGlyph->bitmapRect.x = (uint16_t)packedChar->x0;
			targetGlyph->bitmapRect.y = (uint16_t)packedChar->y0;
			targetGlyph->bitmapRect.width = (uint16_t)(packedChar->x1 - packedChar->x0);
			targetGlyph->bitmapRect.height = (uint16_t)(packedChar->y1 - packedChar->y0);

			targetGlyph->offset = fnt__MakeVec2(x0, y0);
			targetGlyph->size = fnt__MakeVec2(w, h);
			targetGlyph->horizontalAdvance = advance;
			targetGlyph->bitmapIndex = bitmapIndex;
			targetGlyph->codePoint.u16 = codePoint;
		}
	}

	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const fntFontData *fontData, const fntFontSize fontSize, const fntCodePoint codePointStart, const fntCodePoint codePointEnd) {
		if (context == NULL || !fnt__IsValidFontAtlas(atlas) || !fnt__IsValidFontData(fontData) || (fontSize.f32 < FNT__MIN_FONT_SIZE)) return(false);

		if (codePointStart.u16 == 0 || codePointEnd.u16 == 0 || codePointEnd.u16 < codePointStart.u16) return(false);

		int fontOffset = stbtt_GetFontOffsetForIndex(fontData->data, fontData->index);
		if (fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo sinfo;
		if (!stbtt_InitFont(&sinfo, fontData->data, fontOffset)) {
			return(false);
		}

		const float rawToPixels = stbtt_ScaleForPixelHeight(&sinfo, fontSize.f32);

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
		FNT_ASSERT(internalCtx->maxBitmapSize > 0);

		uint32_t codePointCount = (codePointEnd.u16 - codePointStart.u16) + 1;

		fntFontPage *page = fnt__AppendPage(atlas, (uint32_t)codePointStart.u16, codePointCount);
		if (page == NULL) return(false);

		size_t packedCharsSize = sizeof(stbtt_packedchar) * codePointCount;
		stbtt_packedchar *packedChars = (stbtt_packedchar *)FNT_MALLOC(packedCharsSize);
		if (packedChars == NULL) {
			return(false);
		}
		FNT_MEMSET(packedChars, 0, packedCharsSize);

		uint32_t currentCodePointIndex = 0;
		uint32_t currentCodePointStart = codePointStart.u16;
		uint32_t remainingCodePointCount = codePointCount;

		uint32_t pageCodePointIndex = 0;
		uint32_t pageCodePointStart = codePointStart.u16;
		uint32_t totalPageCodePointCount = 0;

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
			range.font_size = fontSize.f32;
			range.num_chars = remainingCodePointCount;
			range.first_unicode_codepoint_in_range = currentCodePointStart;
			range.chardata_for_range = currentPackedChars;

			int packResult = stbtt_PackFontRanges(packCtx, fontData->data, fontData->index, &range, 1);
			if (packResult) {
				totalPageCodePointCount += range.num_chars;
				remainingCodePointCount -= range.num_chars;
				FNT_ASSERT(remainingCodePointCount == 0);

				stbtt_packedchar *pagePackedChars = currentPackedChars;
				fntFontGlyph *pageGlyphs = page->glyphs + pageCodePointIndex;
				fnt__AddGlyphsToPage(pageGlyphs, &sinfo, rawToPixels, range.first_unicode_codepoint_in_range, range.num_chars, bitmapIndex, pagePackedChars);

				// Finished, but leave bitmap intact - no need to reset any index states here
				currentCodePointIndex += range.num_chars;
				break;
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

					fntFontGlyph *pageGlyphs = page->glyphs + pageCodePointIndex;
					fnt__AddGlyphsToPage(pageGlyphs, &sinfo, rawToPixels, pageCodePointStart, totalPageCodePointCount, bitmapIndex, pagePackedChars);

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

#if 0
		char outBuffer[255];
#endif

		if (page->kerningValues != NULL) {
			for (uint32_t codePointIndexA = 0; codePointIndexA < codePointCount; ++codePointIndexA) {
				for (uint32_t codePointIndexB = 0; codePointIndexB < codePointCount; ++codePointIndexB) {
					float kerning = 0.0f;
					if (codePointIndexA != codePointIndexB) {
						uint32_t codePointA = (uint32_t)codePointStart.u16 + codePointIndexA;
						uint32_t codePointB = (uint32_t)codePointStart.u16 + codePointIndexB;
						int kerningRaw = stbtt_GetCodepointKernAdvance(&sinfo, (int)codePointA, (int)codePointB);

#if 0
						if (kerningRaw != 0) {
							sprintf_s(outBuffer, sizeof(outBuffer), "Found kerning for codepoint %c vs %c: %d\n", (char)codePointA, (char)codePointB, kerningRaw);
							OutputDebugStringA(outBuffer);
						}
#endif

						kerning = (float)kerningRaw * rawToPixels;
					}
					uint32_t kerningValueIndex = codePointIndexA * codePointCount + codePointIndexB;
					page->kerningValues[kerningValueIndex] = kerning;
				}
			}
		}

		FNT_FREE(packedChars);

		return(true);
	}

	fnt_api void fntReleaseFontContext(fntFontContext *context) {
		if (context != NULL) {
			fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
			fnt__FinishPack(internalCtx);
			FNT_FREE(internalCtx);
		}
	}

	fnt_api fntFontContext *fntCreateFontContext(const uint32_t maxBitmapSize) {
		if (maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(NULL);

		fnt__STBFontContext *result = (fnt__STBFontContext *)FNT_MALLOC(sizeof(fnt__STBFontContext));
		if (result == NULL) {
			return(NULL);
		}

		FNT_MEMSET(result, 0, sizeof(*result));
		result->maxBitmapSize = maxBitmapSize;
		result->bitmapIndex = UINT32_MAX;

		return((void *)result);
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
		size_t count = 0;
		uint32_t current, prev;

		for (prev = 0, current = 0; *s; prev = current, ++s) {
			uint32_t r = fnt__TestDecodeUTF8(&current, *s);
			if (r == fnt__UTF8State_Accept) {
				++count;
			} else {
				current = fnt__UTF8State_Accept;
				if (prev != fnt__UTF8State_Accept) {
					--s;
				}
			}
		}

		if (current == fnt__UTF8State_Accept) {
			return(count);
		} else {
			return(0); // Malformed UTF-8 string
		}
	}

	fnt_api size_t fntGetQuadCountFromUTF8(const char *utf8) {
		if (utf8 == NULL) return(0);
		size_t result = fnt__CountUTF8String(utf8);
		return(result);
	}

	inline bool fnt__IsCodePointWhiteSpace(const fntFontAtlas *atlas, const uint16_t codePoint) {
		FNT_ASSERT(atlas != NULL);
		FNT_ASSERT(atlas->whitespaceTable.states != NULL);
		bool result = atlas->whitespaceTable.states[codePoint] != 0;
		return(result);
	}

	static void fnt__ComputeQuad(const fntFontGlyph *glyph, const fntVec2 texel, const uint32_t bitmapIndex, const float scale, fntFontQuad *outQuad) {
		float u0 = (float)glyph->bitmapRect.x * texel.u;
		float v0 = (float)glyph->bitmapRect.y * texel.v;
		float u1 = (float)(glyph->bitmapRect.x + glyph->bitmapRect.width) * texel.u;
		float v1 = (float)(glyph->bitmapRect.y + glyph->bitmapRect.height) * texel.v;
		fntVec2 uv0 = fnt__MakeVec2(u0, v0);
		fntVec2 uv1 = fnt__MakeVec2(u1, v1);

		fntVec2 size = fnt__MakeVec2(glyph->size.w * scale, glyph->size.h * scale);

		fntVec2 offset = fnt__MakeVec2(glyph->offset.x * scale, glyph->offset.y * scale);

		fntVec2 coords0 = fnt__MakeVec2(offset.x, offset.y);
		fntVec2 coords1 = fnt__MakeVec2(offset.x + size.w, offset.y + size.h);

		FNT_MEMSET(outQuad, 0, sizeof(*outQuad));
		outQuad->uv[0] = uv0;
		outQuad->uv[1] = uv1;
		outQuad->coords[0] = coords0;
		outQuad->coords[1] = coords1;
		outQuad->bitmapIndex = bitmapIndex;
		outQuad->codePoint = glyph->codePoint;
	}

	fnt_api bool fntComputeQuadsFromUTF8(const fntFontAtlas *atlas, const char *utf8, const float charScale, const fntComputeQuadsFlags flags, const size_t maxQuadCount, fntFontQuad *outQuads, fntBounds *outBounds, uint32_t *outLineCount, float *outBaselineOffset) {
		if (!fnt__IsValidFontAtlas(atlas) || utf8 == NULL || charScale <= 0.0f || maxQuadCount == 0)  return(false);

		size_t codePointCount = fnt__CountUTF8String(utf8);
		if (codePointCount == 0 || maxQuadCount < codePointCount) return(false);

		if (outQuads != NULL) {
			FNT_MEMSET(outQuads, 0, sizeof(fntFontQuad) * codePointCount);
		}

		if (outBounds != NULL) {
			FNT_MEMSET(outBounds, 0, sizeof(*outBounds));
		}
		if (outBaselineOffset != NULL)
			*outBaselineOffset = 0;

		const float invFontSize = 1.0f / atlas->fontSize.f32;

		float scale = invFontSize * charScale;

		float ascent = atlas->ascent * scale;
		float descent = atlas->descent * scale;

		const uint8_t *s = (const uint8_t *)utf8;
		const fntFontPage *currentPage = NULL;
		const fntBitmap *currentBitmap = NULL;
		uint32_t currentPageNum = 0;
		uint32_t currentBitmapIndex = UINT32_MAX;

		const float xInitalPos = 0.0f;
		const float yInitalPos = 0.0f;

		float xPos = xInitalPos;
		float yPos = yInitalPos;

		fntBounds exactBounds = FNT__ZERO_INIT;
		exactBounds.left = exactBounds.right = xPos;
		exactBounds.top = exactBounds.bottom = yPos;

		bool advanceByWidthOnly = (flags & fntComputeQuadsFlags_GlyphAdvancement);

		const uint32_t spaceCodePoint = 32;

		uint32_t codePoint = 0;
		uint32_t codePointIndex;
		uint32_t currentDecodingState, prevDecodingState;
		for (codePointIndex = 0, currentDecodingState = 0, prevDecodingState = 0; *s; prevDecodingState = currentDecodingState, ++s) {
			uint32_t d = fnt__DecodeUTF8(&currentDecodingState, &codePoint, *s);
			if (d != fnt__UTF8State_Reject && d != fnt__UTF8State_Accept) {
				// Special case for extended ascii-set
				if (*s >= 128 && *s <= 255) {
					codePoint = (uint32_t)*s;
				} else {
					codePoint = spaceCodePoint;
				}
				// Reset decoding state
				d = fnt__UTF8State_Accept;
				currentDecodingState = prevDecodingState = 0;
			}
			if (d == fnt__UTF8State_Reject) {
				currentDecodingState = fnt__UTF8State_Accept;
				if (prevDecodingState != fnt__UTF8State_Accept) {
					--s;
				}
			} else if (d == fnt__UTF8State_Accept) {
				// Decode next code point (for kerning)
				uint32_t nextState = 0;
				uint32_t nextCodePoint = 0;
				uint32_t hasNextCodePoint = 0;
				const uint8_t *tmp = s;
				int tmpLen = 0;
				while (*++tmp && tmpLen < 4) {
					uint32_t r = fnt__DecodeUTF8(&nextState, &nextCodePoint, *tmp);
					if (r == fnt__UTF8State_Accept) {
						hasNextCodePoint = 1;
						break;
					}
					++tmpLen;
				}

				if (fnt__IsCodePointWhiteSpace(atlas, nextCodePoint)) {
					// Any whitespace is a special character, so dont accept it for kerning
					nextCodePoint = 0;
				}

				uint32_t pageNumA = atlas->codePointsToPageIndices[codePoint];

				if (pageNumA == 0) {
					// TODO: Page not found, use a substitute character instead
					codePoint = 32;
				}

				if (fnt__IsCodePointWhiteSpace(atlas, codePoint)) {
					float whitespaceAdvancement;
					if (codePoint == '\n') {
						whitespaceAdvancement = 0.0f;
					} else if (codePoint == '\t') {
						whitespaceAdvancement = atlas->whitespaceTable.advancements[spaceCodePoint];
					} else if (codePoint == '\r') {
						whitespaceAdvancement = 0.0f;
					} else {
						whitespaceAdvancement = atlas->whitespaceTable.advancements[codePoint];
					}
					xPos += whitespaceAdvancement * scale;
					exactBounds.right += whitespaceAdvancement * scale;
				} else {
					FNT_ASSERT(pageNumA > 0);

					// Page change
					if (currentPageNum != pageNumA) {
						uint32_t pageIndex = pageNumA - 1;
						FNT_ASSERT(pageIndex < atlas->pageCount);
						currentPage = atlas->pages + pageIndex;
						currentPageNum = pageNumA;
					}
					FNT_ASSERT(currentPage != NULL);

					uint32_t codePointStart = (uint32_t)currentPage->codePointStart.u16;
					uint32_t codePointCount = (uint32_t)currentPage->codePointCount.u16;
					uint32_t codePointEndPastOne = codePointStart + codePointCount;

					FNT_ASSERT(codePoint >= codePointStart && codePoint < codePointEndPastOne);
					uint32_t glyphIndex = codePoint - codePointStart;
					const fntFontGlyph *glyph = currentPage->glyphs + glyphIndex;

					FNT_ASSERT(glyph->codePoint.u16 == codePoint);

					// Bitmap change
					uint32_t bitmapIndex = glyph->bitmapIndex;
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
					fntVec2 texel = fnt__MakeVec2(texelU, texelV);

					float advance = glyph->horizontalAdvance * scale;

					fntFontQuad fontQuad;
					fnt__ComputeQuad(glyph, texel, bitmapIndex, scale, &fontQuad);

					// Advance by the glyph width only
					if (advanceByWidthOnly) {
						advance = glyph->size.w * scale;
						fontQuad.coords[0].x = 0 * scale;
						fontQuad.coords[1].x = glyph->size.w * scale;
					}

					if (flags & fntComputeQuadsFlags_Cartesian) {
						fontQuad.coords[0].y *= -1.0f;
						fontQuad.coords[1].y *= -1.0f;
					}

					// Move quad into absolute coordinates
					fontQuad.coords[0].x += xPos;
					fontQuad.coords[1].x += xPos;
					fontQuad.coords[0].y += yPos;
					fontQuad.coords[1].y += yPos;

					if (codePointIndex == 0) {
						exactBounds.left = fontQuad.coords[0].x;
						exactBounds.right = fontQuad.coords[1].x;
						if (flags & fntComputeQuadsFlags_Cartesian) {
							exactBounds.top = fontQuad.coords[1].y;
							exactBounds.bottom = fontQuad.coords[0].y;
						} else {
							exactBounds.top = fontQuad.coords[0].y;
							exactBounds.bottom = fontQuad.coords[1].y;
						}
					} else {
						exactBounds.left = FNT__MIN(exactBounds.left, fontQuad.coords[0].x);
						exactBounds.right = FNT__MAX(exactBounds.right, fontQuad.coords[1].x);
						if (flags & fntComputeQuadsFlags_Cartesian) {
							exactBounds.top = FNT__MIN(exactBounds.top, fontQuad.coords[1].y);
							exactBounds.bottom = FNT__MAX(exactBounds.bottom, fontQuad.coords[0].y);
						} else {
							exactBounds.top = FNT__MIN(exactBounds.top, fontQuad.coords[0].y);
							exactBounds.bottom = FNT__MAX(exactBounds.bottom, fontQuad.coords[1].y);
						}
					}

					if (outQuads != NULL) {
						fntFontQuad *quad = &outQuads[codePointIndex];
						*quad = fontQuad;
					}

					// We only support kerning inside the same page
					if (!advanceByWidthOnly && !(flags & fntComputeQuadsFlags_WithoutKerning) && (currentPage->kerningValues != NULL && (hasNextCodePoint && (nextCodePoint >= codePointStart && nextCodePoint < codePointEndPastOne)))) {
						uint32_t codePointIndexA = codePoint - codePointStart;
						uint32_t codePointIndexB = nextCodePoint - codePointStart;
						FNT_ASSERT(codePointIndexA < codePointCount);
						FNT_ASSERT(codePointIndexB < codePointCount);
						uint32_t kerningIndex = codePointIndexA * codePointCount + codePointIndexB;
						float kerning = currentPage->kerningValues[kerningIndex] * scale;
						advance += kerning;
					}

					xPos += advance;
				}
				++codePointIndex;
			}
		}

		// Align the quads and the bounds to the left (left = 0)
		{
			float deltaX = exactBounds.left;
			exactBounds.left -= deltaX;
			exactBounds.right -= deltaX;
			if (outQuads != NULL) {
				for (uint32_t i = 0; i < codePointIndex; ++i) {
					outQuads[i].coords[0].x -= deltaX;
					outQuads[i].coords[1].x -= deltaX;
				}
			}
		}

		// Align the quads and the bounds to the bottom (bottom = 0)
		{
			float deltaY;
			if (flags & fntComputeQuadsFlags_Cartesian) {
				deltaY = 0 - exactBounds.top;
			} else {
				deltaY = -exactBounds.bottom + (exactBounds.bottom - exactBounds.top);
			}
			exactBounds.top += deltaY;
			exactBounds.bottom += deltaY;
			if (outQuads != NULL) {
				for (uint32_t i = 0; i < codePointIndex; ++i) {
					outQuads[i].coords[0].y += deltaY;
					outQuads[i].coords[1].y += deltaY;
				}
			}
		}

		// Final adjustment to move the entire quads to the correct positions
		if (!(flags & fntComputeQuadsFlags_Cartesian)) {
			float deltaY = ascent - descent;
			if (outQuads != NULL) {
				for (uint32_t i = 0; i < codePointIndex; ++i) {
					//outQuads[i].coords[0].y += deltaY;
					//outQuads[i].coords[1].y += deltaY;
				}
			}
			//exactBounds.top += deltaY;
			//exactBounds.bottom += deltaY;
		}

		if (outBounds != NULL) {
			outBounds->left = exactBounds.left;
			outBounds->right = exactBounds.right;
			outBounds->top = exactBounds.top;
			outBounds->bottom = exactBounds.bottom;
		}

		return(true);
	}

	fnt_api fntTextSize fntComputeTextSizeFromUTF8(const fntFontAtlas *atlas, const char *utf8, const float charScale, const fntComputeQuadsFlags flags) {
		fntTextSize result = FNT__ZERO_INIT;
		size_t quadCount = fnt__CountUTF8String(utf8);
		fntBounds bounds = FNT__ZERO_INIT;
		uint32_t lineCount = 0;
		float baselineOffset = 0;
		if (fntComputeQuadsFromUTF8(atlas, utf8, charScale, flags, quadCount, NULL, &bounds, &lineCount, &baselineOffset)) {
			result.totalWidth = FNT__MAX(bounds.right, bounds.left) - FNT__MIN(bounds.right, bounds.left);
			result.totalHeight = FNT__MAX(bounds.bottom, bounds.top) - FNT__MIN(bounds.bottom, bounds.top);
			result.lineCount = lineCount;
			result.baselineOffset = baselineOffset;
		}
		return(result);
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

	fnt_api bool fntGetFontMetrics(const fntFontAtlas *atlas, const float charScale, float *outAscent, float *outDescent, float *outLineGap) {
		if (!fnt__IsValidFontAtlas(atlas)) {
			return(false);
		}
		const float invFontSize = 1.0f / atlas->fontSize.f32;
		const float scale = invFontSize * charScale;
		*outAscent = atlas->ascent * scale;
		*outDescent = atlas->descent * scale;
		*outLineGap = atlas->lineGap * scale;
		return(true);
	}

#ifdef __cplusplus
}
#endif

#endif // FNT_IMPLEMENTATION