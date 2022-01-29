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
		//! The name of the font
		const char *name;
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

	typedef struct fntFontContext {
		fntFontInfo info;
		fntFontData data;
	} fntFontContext;

	fnt_api bool fntLoadFont(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize);
	fnt_api void fntFreeFont(fntFontInfo *info);

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info);
	fnt_api void fntFreeFontContext(fntFontContext *context);

	fnt_api fntFontAtlas *fntCreateFontAtlas(const fntFontInfo *info);
	fnt_api bool fntAddToFontAtlas(const fntFontContext *context, fntFontAtlas *atlas);
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

	typedef struct fnt__STBFontContext {
		fntFontContext base;
		stbtt_fontinfo sinfo;
	} fnt__STBFontContext;

	fnt_api bool fntLoadFont(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize) {
		if(data == fnt_null || info == fnt_null) {
			return(false);
		}

		int fontOffset = stbtt_GetFontOffsetForIndex(data->data, fontIndex);
		if(fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo sinfo;
		if(!stbtt_InitFont(&sinfo, data->data, fontOffset)) {
			return(false);
		}

		float pixelScale = stbtt_ScaleForPixelHeight(&sinfo, fontSize);

		float invFontSize = 1.0f / fontSize;

		float rawToPercentage = pixelScale * invFontSize;

		// Get metrics
		int ascentRaw, descentRaw, lineGapRaw;
		int spaceAdvanceRaw, leftSideBearing;
		stbtt_GetFontVMetrics(&sinfo, &ascentRaw, &descentRaw, &lineGapRaw);
		stbtt_GetCodepointHMetrics(&sinfo, ' ', &spaceAdvanceRaw, &leftSideBearing);

		FNT_MEMSET(info, 0, sizeof(*info));
		info->fontIndex = fontIndex;
		info->name = data->name;
		info->size = fontSize;
		info->ascent = ascentRaw * rawToPercentage;
		info->descent = descentRaw * rawToPercentage;
		info->spaceAdvance = spaceAdvanceRaw * rawToPercentage;

		return(true);
	}

	fnt_api void fntFreeFont(fntFontInfo *info) {
		if(info != fnt_null) {
			FNT_MEMSET(info, 0, sizeof(*info));
		}
	}

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

	fnt_api fntFontAtlas *fntCreateFontAtlas(const fntFontInfo *info) {
		fntFontAtlas *result = (fntFontAtlas *)FNT_MALLOC(sizeof(fntFontAtlas));
		FNT_MEMSET(result, 0, sizeof(*result));

		result->codePointsToPageIndices = (uint32_t *)FNT_MALLOC(sizeof(uint32_t) * FNT_MAX_UNICODE_POINT_COUNT);
		FNT_MEMSET(result->codePointsToPageIndices, 0, sizeof(uint32_t) * FNT_MAX_UNICODE_POINT_COUNT);

		result->info = *info;

		return(result);
	}

	fnt_api bool fntAddToFontAtlas(const fntFontContext *context, fntFontAtlas *atlas) {
		if(context == fnt_null || atlas == fnt_null) {
			return(false);
		}
		if(context->data.data == fnt_null || context->data.size == 0) {
			return(false);
		}

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;

		return(true);
	}

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info) {
		if(data == fnt_null || info == fnt_null) {
			return(fnt_null);
		}

		int fontOffset = stbtt_GetFontOffsetForIndex(data->data, info->fontIndex);
		if(fontOffset < 0) {
			return(fnt_null);
		}

		stbtt_fontinfo sinfo;
		if(!stbtt_InitFont(&sinfo, data->data, fontOffset)) {
			return(fnt_null);
		}

		fnt__STBFontContext *result = (fnt__STBFontContext *)FNT_MALLOC(sizeof(fnt__STBFontContext));
		FNT_MEMSET(result, 0, sizeof(*result));
		result->base.data = *data;
		result->base.info = *info;
		result->sinfo = sinfo;

		return(&result->base);
	}

	fnt_api void fntFreeFontContext(fntFontContext *context) {
		if(context != fnt_null) {
			fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
			FNT_FREE(internalCtx);
		}
	}

#ifdef __cplusplus
}
#endif

#endif // FNT_IMPLEMENTATION