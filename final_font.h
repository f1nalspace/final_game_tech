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

	typedef struct fntFontAtlas {
		//! The array of font pages
		fntFontPage *pages;
		//! The array of bitmaps
		fntFontBitmap *bitmaps;
		//! The array of code-points mapped to a font page number starting from 1 to N, zero means not-set
		uint32_t *codePointsToPageIndices;
		//! The font size in pixels
		float fontSize;
		//! The number of pages
		uint32_t pageCount;
		//! The number of bitmaps
		uint32_t bitmapCount;
	} fntFontAtlas;

	typedef struct fntFontContext {
		fntFontInfo info;
		fntFontData data;
		uint32_t maxBitmapSize;
	} fntFontContext;

	fnt_api bool fntLoadFontInfo(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize);
	fnt_api void fntFreeFontInfo(fntFontInfo *info);

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info, const uint32_t maxBitmapSize);
	fnt_api void fntFreeFontContext(fntFontContext *context);

	fnt_api fntFontAtlas *fntCreateFontAtlas();
	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const uint32_t codePointIndex, const uint32_t codePointCount);
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

//! Macro for initialize a struct to zero
#if defined(FNT_IS_C99)
#	define FNT__ZERO_INIT {0}
#else
#	define FNT__ZERO_INIT {}
#endif

// See: https://en.wikipedia.org/wiki/Code_point
#define FNT__UNICODE_PAGE_COUNT 17
#define FNT__UNICODE_POINT_COUNT 65536
#define FNT__MAX_UNICODE_POINT_COUNT (FNT__UNICODE_POINT_COUNT * FNT__UNICODE_PAGE_COUNT)

#define FNT__MIN_BITMAP_SIZE 32

#define FNT__MIN_FONT_SIZE 4

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

	static bool fnt__IsValidFontInfo(const fntFontInfo *info) {
		if(info == NULL) return(false);
		if(info->size < FNT__MIN_FONT_SIZE) return(false);
		if(info->name == NULL) return(false);
		return(true);
	}

	static bool fnt__IsValidFontData(const fntFontData *data) {
		if(data == NULL) return(false);
		if(data->size == 0) return(false);
		if(data->data == NULL) return(false);
		return(true);
	}

	static bool fnt__IsValidFontAtlas(const fntFontAtlas *atlas) {
		if(atlas == NULL) return(false);

		if(atlas->codePointsToPageIndices == NULL) return(false);

		if(atlas->fontSize < FNT__MIN_FONT_SIZE) return(false);

		return(true);
	}

	static bool fnt__IsValidFontContext(const fntFontContext *context) {
		if(context == NULL) return(false);

		const fnt__STBFontContext *internalCtx = (const fnt__STBFontContext *)context;

		if(context->maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(false);

		if(!fnt__IsValidFontInfo(&context->info)) return(false);

		if(!fnt__IsValidFontData(&context->data)) return(false);

		if(internalCtx->sinfo.data != context->data.data) return(false);

		return(true);
	}

	fnt_api bool fntLoadFontInfo(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize) {
		if(!fnt__IsValidFontData(data)) return(false);
		if(info == NULL) return(false);
		if(fontSize < FNT__MIN_FONT_SIZE) return(false);

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

	fnt_api void fntFreeFontInfo(fntFontInfo *info) {
		if(info != NULL) {
			FNT_MEMSET(info, 0, sizeof(*info));
		}
	}

	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas) {
		if(atlas == NULL) return;

		if(atlas->pages != NULL) {
			FNT_FREE(atlas->pages);
			atlas->pages = NULL;
		}

		if(atlas->bitmaps != NULL) {
			FNT_FREE(atlas->bitmaps);
			atlas->bitmaps = NULL;
		}

		if(atlas->codePointsToPageIndices != NULL) {
			FNT_FREE(atlas->codePointsToPageIndices);
			atlas->codePointsToPageIndices = NULL;
		}
	}

	fnt_api fntFontAtlas *fntCreateFontAtlas() {
		fntFontAtlas *result = (fntFontAtlas *)FNT_MALLOC(sizeof(fntFontAtlas));
		FNT_MEMSET(result, 0, sizeof(*result));

		result->codePointsToPageIndices = (uint32_t *)FNT_MALLOC(sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		FNT_MEMSET(result->codePointsToPageIndices, 0, sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);

		return(result);
	}

	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const uint32_t codePointStart, const uint32_t codePointCount) {
		if(!fnt__IsValidFontContext(context) || !fnt__IsValidFontAtlas(atlas)) return(false);

		if((codePointCount == 0) || ((uint64_t)codePointStart + (uint64_t)codePointCount > (uint64_t)FNT__MAX_UNICODE_POINT_COUNT)) return(false);

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;

		const float fontSize = context->info.size;

		stbtt_fontinfo *sinfo = &internalCtx->sinfo;

		stbtt_packedchar *packedChars = (stbtt_packedchar *)FNT_MALLOC(sizeof(stbtt_packedchar) * codePointCount);

		stbtt_pack_range range = FNT__ZERO_INIT;
		range.font_size = fontSize;
		range.num_chars = codePointCount;
		range.first_unicode_codepoint_in_range = codePointStart;
		range.chardata_for_range = packedChars;

		FNT_FREE(packedChars);

		return(true);
	}

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info, const uint32_t maxBitmapSize) {
		if(!fnt__IsValidFontData(data) || !fnt__IsValidFontInfo(info))return(NULL);
		if(maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(NULL);

		int fontOffset = stbtt_GetFontOffsetForIndex(data->data, info->fontIndex);
		if(fontOffset < 0) {
			return(NULL);
		}

		stbtt_fontinfo sinfo;
		if(!stbtt_InitFont(&sinfo, data->data, fontOffset)) {
			return(NULL);
		}

		fnt__STBFontContext *result = (fnt__STBFontContext *)FNT_MALLOC(sizeof(fnt__STBFontContext));
		FNT_MEMSET(result, 0, sizeof(*result));
		result->base.data = *data;
		result->base.info = *info;
		result->base.maxBitmapSize = maxBitmapSize;
		result->sinfo = sinfo;

		return(&result->base);
	}

	fnt_api void fntFreeFontContext(fntFontContext *context) {
		if(context != NULL) {
			fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
			FNT_FREE(internalCtx);
		}
	}

#ifdef __cplusplus
}
#endif

#endif // FNT_IMPLEMENTATION