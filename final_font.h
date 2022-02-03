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

	typedef struct fntFontInfo {
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
		//! The horizontal advancement for the space character in pixels
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
		//! The index of the bitmap
		uint32_t bitmapIndex;
		//! The first code point
		uint32_t codePointStart;
		//! The number of code points (N)
		uint32_t codePointCount;
	} fntFontPage;

	typedef union fntKerningKey {
		struct {
			uint16_t a;
			uint16_t b;
		};
		uint32_t value;
	} fntKerningKey;

	typedef struct fntKerningTableEntry {
		fntKerningKey key;
		float value;
	} fntKerningTableEntry;

	typedef struct fntKerningTable {
		fntKerningTableEntry *entries;
		uint32_t capacity;
		uint32_t count;
	} fntKerningTable;

	typedef struct fntFontAtlas {
		//! The kerning table for all code point pairs, mapping a uint64_t key that builds up a code-point pair to the actual kerning value
		fntKerningTable kerningTable;
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
#else
#	define FNT__ZERO_INIT {}
#endif

// See: https://en.wikipedia.org/wiki/Code_point
#define FNT__MAX_UNICODE_POINT_COUNT 0xFFFF

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

	typedef struct {
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

	static uint32_t fnt__NextPowerOfTwo(const uint32_t input) {
		uint32_t x = input;
		x--;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x++;
		return(x);
	}

	static void fnt__InitKerningTable(fntKerningTable *kerningTable) {
		FNT_ASSERT(kerningTable != NULL);
		FNT_MEMSET(kerningTable, 0, sizeof(*kerningTable));
	}

	static void fnt__FreeKerningTable(fntKerningTable *kerningTable) {
		FNT_ASSERT(kerningTable != NULL);
		if (kerningTable->entries != NULL) {
			FNT_FREE(kerningTable->entries);
		}
		FNT_MEMSET(kerningTable, 0, sizeof(*kerningTable));
	}

	static bool fnt__ResizeKerningTable(fntKerningTable *kerningTable, const uint32_t newCapacity);

	static bool fnt__InsertIntoKerningTable(fntKerningTable *kerningTable, const fntKerningKey key, const float value) {
		if (kerningTable->capacity == 0 || kerningTable->count == kerningTable->capacity) {
			uint32_t newCapacity;
			if (kerningTable->capacity > 0)
				newCapacity = kerningTable->capacity * 2;
			else
				newCapacity = 1024;
			bool r = fnt__ResizeKerningTable(kerningTable, newCapacity);
			if (!r) return(false);
		}

		uint32_t probeCount = 0;

		uint32_t hash = key.value;
		uint32_t index = (uint32_t)(hash % (uint64_t)kerningTable->capacity);
		uint32_t initialIndex = index;
		do {
			++probeCount;
			fntKerningTableEntry *entry = kerningTable->entries + index;
			if (entry->key.value == key.value) {
				return(false); // Duplicate key
			} else if (entry->key.value == 0) {
				entry->key = key;
				entry->value = value;
				++kerningTable->count;
				return(true);
			}
			index = (index + 1) % kerningTable->capacity;
		} while (index != initialIndex);
		return(false); // Not enough space
	}

	static float fnt__GetKerningValue(const fntKerningTable *kerningTable, const fntKerningKey key) {
		uint32_t hash = key.value;
		uint32_t index = (uint32_t)(hash % kerningTable->capacity);
		uint32_t initialIndex = index;
		uint32_t probeCount = 0;
		do {
			++probeCount;
			fntKerningTableEntry *entry = kerningTable->entries + index;
			if (entry->key.value == 0) {
				return(0.0f);
			}
			if (entry->key.value == key.value) {
				return(entry->value);
			}
			index = (index + 1) % kerningTable->capacity;
		} while (index != initialIndex);
		return(0.0f);
	}

	static bool fnt__ResizeKerningTable(fntKerningTable *kerningTable, const uint32_t newCapacity) {
		FNT_ASSERT(kerningTable != NULL);

		size_t newEntriesSize = sizeof(fntKerningTableEntry) * newCapacity;
		fntKerningTableEntry *newEntries = (fntKerningTableEntry *)FNT_MALLOC(newEntriesSize);
		if (newEntries == NULL) return(false);

		fntKerningTable newTable = FNT__ZERO_INIT;
		newTable.capacity = newCapacity;
		newTable.entries = newEntries;

		// Empty kerning table
		if (kerningTable->entries == NULL) {
			FNT_MEMSET(newEntries, 0, newEntriesSize);
			*kerningTable = newTable;
			return(true);
		}

		size_t oldCapacity = kerningTable->capacity;
		for (uint32_t oldIndex = 0; oldIndex < oldCapacity; ++oldIndex) {
			const fntKerningTableEntry *entry = kerningTable->entries + oldIndex;
			if (entry->key.value != 0) {
				fntKerningKey key = entry->key;
				float value = entry->value;

				bool r = fnt__InsertIntoKerningTable(&newTable, key, value);
				FNT_ASSERT(r);
				if (!r) {
					FNT_FREE(newEntries);
					return(false);
				}
			}
		}

		// Free old entries
		FNT_FREE(kerningTable->entries);

		// Copy new table back
		*kerningTable = newTable;

		return(true);
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

		uint32_t endCodePointPastOne = codePointStart + codePointCount;

		fntFontPage *newPage = newPages + pageIndex;
		FNT_MEMSET(newPage, 0, sizeof(*newPage));
		newPage->bitmapIndex = bitmapIndex;
		newPage->codePointStart = codePointStart;
		newPage->codePointCount = codePointCount;
		newPage->glyphs = (fntFontGlyph *)FNT_MALLOC(glyphsSize);

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

		return(pageIndex);
	}

	static void fnt__FreePage(fntFontPage *page) {
		FNT_ASSERT(page != NULL);

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
		info->ascent = ascentRaw * pixelScale;
		info->descent = descentRaw * pixelScale;
		info->spaceAdvance = spaceAdvanceRaw * pixelScale;

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

		fnt__FreeKerningTable(&atlas->kerningTable);
	}

	fnt_api bool fntInitFontAtlas(const fntFontInfo *info, fntFontAtlas *atlas) {
		if (info == NULL || atlas == NULL) return(false);

		uint32_t *pageIndices = (uint32_t *)FNT_MALLOC(sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);
		if (pageIndices == NULL) {
			return(false);
		}
		FNT_MEMSET(pageIndices, 0, sizeof(uint32_t) * FNT__MAX_UNICODE_POINT_COUNT);

		FNT_MEMSET(atlas, 0, sizeof(*atlas));
		atlas->codePointsToPageIndices = pageIndices;

		fnt__InitKerningTable(&atlas->kerningTable);

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

	fnt_api bool fntComputeAtlasKernings(const fntFontContext *context, fntFontAtlas *atlas) {
		if (context == NULL || atlas == NULL) return(false);

		fnt__STBFontContext *internalCtx = (fnt__STBFontContext *)context;

		const stbtt_fontinfo *sinfo = &internalCtx->sinfo;

		const float heightToScale = internalCtx->base.info.heightToScale;

		// Compute total glyph count for all pages
		uint32_t totalGlyphCount = 0;
		for (uint32_t pageIndex = 0; pageIndex < atlas->pageCount; ++pageIndex) {
			const fntFontPage *page = atlas->pages + pageIndex;
			totalGlyphCount += page->codePointCount;
		}

		const uint32_t pageCount = atlas->pageCount;

		// Compute kerning pairs
		uint32_t pairCount = totalGlyphCount * totalGlyphCount;
		uint32_t kerningTableCapacity = fnt__NextPowerOfTwo(pairCount);
		fnt__ResizeKerningTable(&atlas->kerningTable, kerningTableCapacity);
		for (uint32_t pageIndexA = 0; pageIndexA < pageCount; ++pageIndexA) {
			const fntFontPage *pageA = atlas->pages + pageIndexA;
			for (uint32_t pageIndexB = 0; pageIndexB < pageCount; ++pageIndexB) {
				const fntFontPage *pageB = atlas->pages + pageIndexB;
				for (uint32_t glyphIndexA = 0; glyphIndexA < pageA->codePointCount; ++glyphIndexA) {
					for (uint32_t glyphIndexB = 0; glyphIndexB < pageB->codePointCount; ++glyphIndexB) {
						uint32_t codePointA = pageA->codePointStart + glyphIndexA;
						uint32_t codePointB = pageB->codePointStart + glyphIndexB;
						float kerning = 0.0f;
						if (codePointA != codePointB) {
							int kerningRaw = stbtt_GetCodepointKernAdvance(sinfo, (int)codePointA, (int)codePointB);
							kerning = (float)kerningRaw * heightToScale;
						}
						if (codePointA != codePointB) {
							fntKerningKey key;
							key.a = codePointA;
							key.b = codePointB;
							bool r = fnt__InsertIntoKerningTable(&atlas->kerningTable, key, kerning);
							FNT_ASSERT(r);
						}
					}
				}
			}
		}

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

	fnt_api bool fntComputeQuadsFromUTF8(const fntFontAtlas *atlas, const fntFontInfo *info, const char *utf8, const float charHeight, const fntComputeQuadsFlags flags, const size_t maxQuadCount, fntFontQuad *outQuads, fntVec2 *outSize) {
		if (atlas == NULL || info == NULL || utf8 == NULL || charHeight <= 0.0f || maxQuadCount == 0)  return(false);

		size_t codePointCount = fnt__CountUTF8String(utf8);
		if (codePointCount == 0 || maxQuadCount < codePointCount) return(false);

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

				uint32_t pageNum = atlas->codePointsToPageIndices[codePoint];
				if (pageNum == 0) {
					// TODO: Page not found, use a substitute character instead
					codePoint = 32;
				}
				if (codePoint == 32) {
					// TODO: Space-character
				} else {
					FNT_ASSERT(pageNum > 0);
					if (currentPageNum != pageNum) {
						uint32_t pageIndex = pageNum - 1;
						FNT_ASSERT(pageIndex < atlas->pageCount);
						currentPage = atlas->pages + pageIndex;
						currentPageNum = pageNum;
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

					float u0 = (float)glyph->bitmapX * texelU;
					float v0 = (float)glyph->bitmapY * texelV;
					float u1 = (float)(glyph->bitmapX + glyph->width) * texelU;
					float v1 = (float)(glyph->bitmapY + glyph->height) * texelV;

					float xoffset0 = glyph->baselineOffsets[0].x * info->heightToScale * charHeight;
					float yoffset0 = glyph->baselineOffsets[0].y * info->heightToScale * charHeight;
					float xoffset1 = glyph->baselineOffsets[1].x * info->heightToScale * charHeight;
					float yoffset1 = glyph->baselineOffsets[1].y * info->heightToScale * charHeight;

					float width = xoffset1 - xoffset0;
					float height = yoffset1 - yoffset0;

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
					if (hasNextCodePoint) {
						fntKerningKey kerningKey;
						kerningKey.a = (uint16_t)codePoint;
						kerningKey.b = (uint16_t)nextCodePoint;
						kerning = fnt__GetKerningValue(&atlas->kerningTable, kerningKey);
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