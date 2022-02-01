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

	typedef struct fntFontInfo {
		//! The font name
		const char *name;
		//! The font size
		float fontSize;
		//! The font height to scale factor
		float heightToScale;
		//! The ascent from the baseline in range of 0.0 to 1.0
		float ascent;
		//! The descent from the baseline in range of 0.0 to -1.0
		float descent;
		//! The horizontal advancement for the space character in range of 0.0 to 1.0
		float spaceAdvance;
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
		//! The kerning table for the code-point pairs (N * N)
		float *kerningTable;
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The first code point
		uint32_t codePointStart;
		//! The number of code points (N)
		uint32_t codePointCount;
	} fntFontPage;

	typedef struct fntFontAtlas {
		//! The array of font pages
		fntFontPage *pages;
		//! The array of alpha bitmaps
		fntBitmap *bitmaps;
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
		uint32_t maxBitmapSize;
	} fntFontContext;

	fnt_api bool fntLoadFontInfo(const fntFontData *data, fntFontInfo *info, const uint32_t fontIndex, const float fontSize);
	fnt_api void fntFreeFontInfo(fntFontInfo *info);

	fnt_api fntFontContext *fntCreateFontContext(const fntFontData *data, const fntFontInfo *info, const uint32_t maxBitmapSize);
	fnt_api void fntReleaseFontContext(fntFontContext *context);

	fnt_api fntFontAtlas *fntCreateFontAtlas(const fntFontInfo *info);
	fnt_api bool fntAddToFontAtlas(fntFontContext *context, fntFontAtlas *atlas, const uint32_t codePointIndex, const uint32_t codePointCount);
	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas);

	fnt_api bool fntSaveBitmapToFile(const fntBitmap *bitmap, const char *filePath);

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
#else
#	define FNT__ZERO_INIT {}
#endif

// See: https://en.wikipedia.org/wiki/Code_point
#define FNT__MAX_UNICODE_POINT_COUNT 65536 * 17

#define FNT__MIN_BITMAP_SIZE 32

#define FNT__MIN_FONT_SIZE 4

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct fnt__STBFontContext {
		fntFontContext base;
		stbtt_fontinfo sinfo;

		stbtt_pack_context currentPackContext;
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

		if (context->maxBitmapSize < FNT__MIN_BITMAP_SIZE) return(false);

		if (!fnt__IsValidFontInfo(&context->info)) return(false);

		if (!fnt__IsValidFontData(&context->data)) return(false);

		if (internalCtx->sinfo.data != context->data.data) return(false);

		return(true);
	}

	static void fnt__NewPack(fnt__STBFontContext *internalCtx, uint8_t *pixels) {
		FNT_ASSERT(internalCtx != NULL);
		FNT_ASSERT(pixels != NULL);
		stbtt_pack_context *packCtx = &internalCtx->currentPackContext;
		stbtt_PackBegin(packCtx, (unsigned char *)pixels, internalCtx->base.maxBitmapSize, internalCtx->base.maxBitmapSize, 0, 1, NULL);
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
		FNT_ASSERT(atlas != NULL);
		FNT_ASSERT(bitmapIndex != UINT32_MAX);
		FNT_ASSERT(codePointCount > 0);

		uint32_t oldCount = atlas->pageCount;
		uint32_t newCount = oldCount + 1;
		fntFontPage *newPages = (fntFontPage *)FNT_REALLOC(atlas->pages, sizeof(fntFontPage) * newCount);
		if (newPages == NULL) return(UINT32_MAX);
		atlas->pages = newPages;
		atlas->pageCount = newCount;

		const stbtt_fontinfo *fontInfo = &context->sinfo;

		uint32_t pageIndex = oldCount;

		size_t glyphsSize = sizeof(fntFontGlyph) * codePointCount;

		size_t kerningTableSize = sizeof(float) * codePointCount * codePointCount;

		uint32_t endCodePointPastOne = codePointStart + codePointCount;

		fntFontPage *newPage = newPages + pageIndex;
		FNT_MEMSET(newPage, 0, sizeof(*newPage));
		newPage->bitmapIndex = bitmapIndex;
		newPage->codePointStart = codePointStart;
		newPage->codePointCount = codePointCount;
		newPage->glyphs = (fntFontGlyph *)FNT_MALLOC(glyphsSize);
		newPage->kerningTable = (float *)FNT_MALLOC(kerningTableSize);

		FNT_MEMSET(newPage->glyphs, 0, glyphsSize);
		for (uint32_t glyphIndex = 0; glyphIndex < codePointCount; ++glyphIndex) {
			uint32_t codePoint = codePointStart + glyphIndex;

			const stbtt_packedchar *packedChar = packedChars + glyphIndex;

			fntFontGlyph *targetGlyph = newPage->glyphs + glyphIndex;

			uint16_t w = (packedChar->x1 - packedChar->x0) + 1;
			uint16_t h = (packedChar->y1 - packedChar->y0) + 1;

			float xoffset0 = packedChar->xoff;
			float yoffset0 = packedChar->yoff;
			float xoffset1 = packedChar->xoff2;
			float yoffset1 = packedChar->yoff2;

			targetGlyph->width = w;
			targetGlyph->height = h;
			targetGlyph->baselineOffsets[0] = fnt__MakeVec2(xoffset0, yoffset0);
			targetGlyph->baselineOffsets[1] = fnt__MakeVec2(xoffset1, yoffset1);
			targetGlyph->horizontalAdvance = packedChar->xadvance;
			targetGlyph->codePoint = codePoint;
		}

		for (uint32_t codePointIndex = codePointStart; codePointIndex < endCodePointPastOne; ++codePointIndex) {
			atlas->codePointsToPageIndices[codePointIndex] = pageIndex + 1; // We store page (index +1) instead, so zero is invalid
		}

		FNT_MEMSET(newPage->kerningTable, 0, kerningTableSize);
		for (uint32_t codePointIndexA = 0; codePointIndexA < codePointCount; ++codePointIndexA) {
			for (uint32_t codePointIndexB = 0; codePointIndexB < codePointCount; ++codePointIndexB) {
				if (codePointIndexA != codePointIndexB) {
					uint32_t codePointA = codePointStart + codePointIndexA;
					uint32_t codePointB = codePointStart + codePointIndexB;
					int kerningRaw = stbtt_GetCodepointKernAdvance(fontInfo, (int)codePointA, (int)codePointB);
					float kerning = (float)kerningRaw;
					newPage->kerningTable[codePointIndexA * codePointCount + codePointIndexB] = kerning;
				}
			}
		}

		return(pageIndex);
	}

	static void fnt__FreePage(fntFontPage *page) {
		FNT_ASSERT(page != NULL);

		if (page->kerningTable != NULL) {
			FNT_FREE(page->kerningTable);
			page->kerningTable = NULL;
		}

		if (page->glyphs != NULL) {
			FNT_FREE(page->glyphs);
			page->glyphs = NULL;
		}
	}

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
		info->fontSize = fontSize;
		info->heightToScale = pixelScale;
		info->ascent = ascentRaw * rawToPercentage;
		info->descent = descentRaw * rawToPercentage;
		info->spaceAdvance = spaceAdvanceRaw * rawToPercentage;

		return(true);
	}

	fnt_api void fntFreeFontInfo(fntFontInfo *info) {
		if (info != NULL) {
			FNT_MEMSET(info, 0, sizeof(*info));
		}
	}

	fnt_api void fntFreeFontAtlas(fntFontAtlas *atlas) {
		if (atlas == NULL) return;

		if (atlas->pages != NULL) {
			FNT_FREE(atlas->pages);
			atlas->pages = NULL;
		}

		if (atlas->bitmaps != NULL) {
			FNT_FREE(atlas->bitmaps);
			atlas->bitmaps = NULL;
		}

		if (atlas->codePointsToPageIndices != NULL) {
			FNT_FREE(atlas->codePointsToPageIndices);
			atlas->codePointsToPageIndices = NULL;
		}
	}

	fnt_api fntFontAtlas *fntCreateFontAtlas(const fntFontInfo *info) {
		if (info == NULL) return(NULL);

		fntFontAtlas *result = (fntFontAtlas *)FNT_MALLOC(sizeof(fntFontAtlas));
		if (result == NULL) return(NULL);
		FNT_MEMSET(result, 0, sizeof(*result));

		uint32_t *pageIndices = result->codePointsToPageIndices = (uint32_t *)FNT_MALLOC(sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		if (pageIndices == NULL) {
			fntFreeFontAtlas(result);
			return(NULL);
		}
		FNT_MEMSET(pageIndices, 0, sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);

		return(result);
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

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;

		const float fontSize = context->info.fontSize;

		stbtt_fontinfo *sinfo = &internalCtx->sinfo;

		size_t packedCharsSize = sizeof(stbtt_packedchar) * codePointCount;
		stbtt_packedchar *packedChars = (stbtt_packedchar *)FNT_MALLOC(packedCharsSize);
		FNT_MEMSET(packedChars, 0, packedCharsSize);

		uint32_t currentCodePointIndex = 0;
		uint32_t currentCodePointStart = codePointStart;
		uint32_t remainingCodePointCount = codePointCount;

		uint32_t pageCodePointIndex = 0;
		uint32_t pageCodePointStart = codePointStart;
		uint32_t totalPageCodePointCount = 0;

		stbtt_pack_context *packCtx = &internalCtx->currentPackContext;

		while (remainingCodePointCount > 0) {
			if (internalCtx->bitmapIndex == UINT32_MAX) {
				internalCtx->bitmapIndex = fnt__AddBitmap(atlas, context->maxBitmapSize, context->maxBitmapSize);

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

			int packResult = stbtt_PackFontRanges(packCtx, context->data.data, context->info.fontIndex, &range, 1);
			if (packResult) {
				totalPageCodePointCount += range.num_chars;
				remainingCodePointCount -= range.num_chars;
				FNT_ASSERT(remainingCodePointCount == 0);

				stbtt_packedchar *pagePackedChars = packedChars + pageCodePointIndex;
				fnt__AddPage(internalCtx, atlas, bitmapIndex, pageCodePointStart, totalPageCodePointCount, pagePackedChars);

				break; // Finished, but leave bitmap intact
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
					fnt__AddPage(internalCtx, atlas, bitmapIndex, pageCodePointStart, totalPageCodePointCount, pagePackedChars);
					pageCodePointIndex = 0;
					pageCodePointStart = currentCodePointStart;
					totalPageCodePointCount = 0;

					// Bitmap is complete, start another bitmap in the next iteration and finish the pack
					fnt__FinishPack(internalCtx);
					internalCtx->bitmapIndex = UINT32_MAX;
				} else {
					remainingCodePointCount -= numChars;
					currentCodePointStart += numChars;

					// Page is incomplete, just increase the total count
					totalPageCodePointCount += numChars;
				}
			}
		}

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
		FNT_MEMSET(result, 0, sizeof(*result));
		result->base.data = *data;
		result->base.info = *info;
		result->base.maxBitmapSize = maxBitmapSize;
		result->sinfo = sinfo;

		result->bitmapIndex = UINT32_MAX;

		return(&result->base);
	}

	fnt_api void fntReleaseFontContext(fntFontContext *context) {
		if (context != NULL) {
			fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;
			fnt__FinishPack(internalCtx);
			FNT_FREE(internalCtx);
		}
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