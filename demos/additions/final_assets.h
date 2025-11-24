/*
Name:
	Final Assets
Description:
	Simple asset system.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_ASSETS_H
#define FINAL_ASSETS_H

#include <final_platform_layer.h>
#include <final_memory.h>

#include <final_math.h>
#include <final_fontloader.h>
#include <final_render.h>

typedef struct {
	LoadedFont desc;
	TextureHandle texture;
} FontAsset;

typedef struct {
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint8_t components;
} TextureData;

typedef struct {
	TextureData data;
	TextureHandle texture;
} TextureAsset;

typedef enum {
	AssetType_None = 0,
	AssetType_Texture,
	AssetType_Font,
} AssetType;

fpl_extern TextureData LoadTextureData(const char *dataPath, const char *filename);
fpl_extern void FreeTextureData(TextureData *texture);
fpl_extern TextureData CreateSubTextureData(TextureData *source, const int x, const int y, const int w, const int h);
fpl_extern void ReleaseFontAsset(FontAsset *font);

#endif // FINAL_ASSETS_H

#if defined(FINAL_ASSETS_IMPLEMENTATION) && !defined(FINAL_ASSETS_IMPLEMENTED)
#define FINAL_ASSETS_IMPLEMENTED

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#include <final_fontloader.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

fpl_extern void FreeTextureData(TextureData *texture) {
	if (texture == fpl_null || texture->data == fpl_null) {
		return;
	}
	stbi_image_free(texture->data);
	texture->data = fpl_null;
}

fpl_extern TextureData CreateSubTextureData(TextureData *source, const int x, const int y, const int w, const int h) {
	TextureData result = fplZeroInit;
	if (source == fpl_null || source->data == fpl_null || source->components != 4) {
		return result;
	}
	result.width = w;
	result.height = h;
	result.components = 4;
	result.data = (uint8_t *)STBI_MALLOC(w * h * 4);
	int sourceScanline = source->width * 4;
	int destScanline = w * 4;
	int dstY = 0;
	for (int srcY = y; srcY < (y + h); ++srcY) {
		int dstX = 0;
		for (int srcX = x; srcX < (x + w); ++srcX) {
			uint8_t *src = source->data + (srcY * sourceScanline + srcX * 4);
			uint8_t *dst = result.data + (dstY * destScanline + dstX * 4);
			uint32_t *srcPixel = (uint32_t *)src;
			uint32_t *dstPixel = (uint32_t *)dst;
			*dstPixel = *srcPixel;
			++dstX;
		}
		++dstY;
	}
	return result;
}

fpl_extern TextureData LoadTextureData(const char *dataPath, const char *filename) {
	TextureData result = fplZeroInit;

	char filePath[1024];
	fplPathCombine(filePath, fplArrayCount(filePath), 2, dataPath, filename);

	fplFileHandle file;
	if (fplFileOpenBinary(filePath, &file)) {
		uint32_t fileLen = fplFileGetSizeFromHandle32(&file);
		uint8_t *fileBuffer = (uint8_t *)fplMemoryAllocate(fileLen);
		if (fileBuffer != fpl_null) {
			if (fplFileReadBlock32(&file, fileLen, fileBuffer, fileLen) == fileLen) {
				int imageWidth = 0;
				int imageHeight = 0;
				int imageComponents = 0;
				stbi_set_flip_vertically_on_load(0);
				stbi_uc *imageData = stbi_load_from_memory(fileBuffer, fileLen, &imageWidth, &imageHeight, &imageComponents, 4);
				if (imageData != fpl_null) {
					result.width = imageWidth;
					result.height = imageHeight;
					result.components = imageComponents;
					result.data = imageData;
				} else {
					fplConsoleFormatError("Image file '%s' of size '%lu' is broken!\n", filePath, fileLen);
				}
			} else {
				fplConsoleFormatError("Failed reading file of size '%lu'!\n", fileLen);
			}
			fplMemoryFree(fileBuffer);
		} else {
			fplConsoleFormatError("Failed allocating memory of size '%lu'!\n", fileLen);
		}
		fplFileClose(&file);
	} else {
		fplConsoleFormatError("Image file '%s' could not be found!\n", filePath);
	}
	return(result);
}

fpl_extern void ReleaseFontAsset(FontAsset *font) {
	// @TODO(final): Release texture somehow
	ReleaseFont(&font->desc);
}

#endif // FINAL_ASSETS_IMPLEMENTATION