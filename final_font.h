#ifndef FNT_INCLUDE_H
#define FNT_INCLUDE_H

//
// C99 detection
//
// https://en.wikipedia.org/wiki/C99#Version_detection
// Visual Studio 2015+
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! Detected C99 compiler
#	define FNT_IS_C99
#elif defined(__cplusplus)
	//! Detected C++ compiler
#	define FNT_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

//
// Includes
//
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // uint32_t
#include <stdlib.h> // NULL
#include <stdbool.h> // bool

#define fnt_null NULL

//! Macro for initialize a struct to zero
#if defined(FNT_IS_C99)
#	define FNT_ZERO_INIT {0}
#else
#	define FNT_ZERO_INIT {}
#endif

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

#if !defined(FNT_MEMSET)
#include <string.h>
#define FNT_MEMSET(dst, value, size) memset(dst, value, size)
#endif

// ****************************************************************************
//
// > API
//
// ****************************************************************************
#ifdef __cplusplus
extern "C" {
#endif

	typedef struct fntFontData {
		//! The raw font file data
		const uint8_t *data;
		//! The number of bytes
		size_t size;
	} fntFontData;

	typedef enum fntFontBitmapFormat {
		//! 8-bit alpha only
		Alpha8 = 0,
		//! 32-bit RGBA
		RGBA8
	} fntFontBitmapFormat;

	typedef struct fntFontBitmap {
		//! The pixels from top-to-bottom
		const uint8_t *pixels;
		//! The format
		fntFontBitmapFormat format;
		//! The width in pixels
		uint16_t width;
		//! The height in pixels
		uint16_t height;
	} fntFontBitmap;

	typedef struct fntFontInfo {
		//! The font name
		const char *name;
		//! The font size
		float size;
		//! The ascent from the baseline in range of 0.0 to 1.0
		float ascent;
		//! The descent from the baseline in range of 0.0 to -1.0
		float descent;
		//! The horizontal advancement for the space character in range of 0.0 to 1.0
		float spaceAdvance;
		//! The font index
		uint32_t fontIndex;
	} fntFontInfo;

	typedef struct fntFontGlyph {
		//! The offset for X to the baseline
		float baselineX;
		//! The offset for Y to the baseline
		float baselineY;
		//! The width of the glyph in pixels
		uint16_t width;
		//! The height of the glyph in pixels
		uint16_t height;
		//! The X position in the bitmap
		uint16_t bitmapX;
		//! The Y position in the bitmap
		uint16_t bitmapY;
		//! The code point for validation
		uint32_t codePoint;
	} fntFontGlyph;

	typedef struct fntFontPage {
		//! The array of font glyphs
		fntFontGlyph *glyphs;
		//! The kerning table for all glyph pairs (codePointCount * codePointCount)
		float *kerningTable;
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The first code point
		uint32_t codePointStart;
		//! The number of code points
		uint32_t codePointCount;
	} fntFontPage;

// See: https://en.wikipedia.org/wiki/Code_point
#define FNT_UNICODE_PAGE_COUNT 17
#define FNT_UNICODE_POINT_COUNT 65536
#define FNT_MAX_UNICODE_POINT_COUNT (FNT_UNICODE_POINT_COUNT * FNT_UNICODE_PAGE_COUNT)

	typedef struct fntFontAtlas {
		//! The font info
		fntFontInfo info;
		//! The array of font pages
		fntFontPage *pages;
		//! The array of bitmaps
		fntFontBitmap *bitmaps;
		//! The array of code-points mapped to a font page number starting from 1 to N, zero means not-set
		uint32_t *codePointsToPageIndices;
		//! The number of pages
		uint32_t pageCount;
		//! The number of bitmaps
		uint32_t bitmapCount;
	} fntFontAtlas;

	fnt_api fntFontAtlas *fntCreateFontAtlas();
	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas);

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

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#ifdef __cplusplus
extern "C" {
#endif

	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas) {
		if(atlas != fnt_null) return;

		if(atlas->pages != fnt_null) {
			FNT_FREE(atlas->pages);
			atlas->pages = fnt_null;
		}

		if(atlas->bitmaps != fnt_null) {
			FNT_FREE(atlas->bitmaps);
			atlas->bitmaps = fnt_null;
		}

		if(atlas->codePointsToPageIndices != fnt_null) {
			FNT_FREE(atlas->codePointsToPageIndices);
			atlas->codePointsToPageIndices = fnt_null;
		}
	}

	fnt_api fntFontAtlas *fntCreateFontAtlas() {
		fntFontAtlas *result = (fntFontAtlas *)FNT_MALLOC(sizeof(fntFontAtlas));
		FNT_MEMSET(result, 0, sizeof(*result));

		result->codePointsToPageIndices = (uint32_t *)FNT_MALLOC(sizeof(uint32_t) * FNT_MAX_UNICODE_POINT_COUNT);
		FNT_MEMSET(result->codePointsToPageIndices, 0, sizeof(uint32_t) * FNT_MAX_UNICODE_POINT_COUNT);

		return(result);
	}

#ifdef __cplusplus
}
#endif

#endif // FNT_IMPLEMENTATION