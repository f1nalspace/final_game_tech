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

typedef struct FontAsset {
	LoadedFont desc;
	TextureHandle texture;
} FontAsset;

typedef struct TextureData {
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint32_t components;
} TextureData;

typedef struct TextureAsset {
	TextureData data;
	TextureHandle texture;
} TextureAsset;

#define ASSETS_MEMORY_ALLOCATE_FUNC(name) void *name(const size_t size, void *userData)
typedef ASSETS_MEMORY_ALLOCATE_FUNC(AssetsMemoryAllocateFunc);

#define TEXTURE_DATA_FREE_FUNC(name) void name(void *ptr, void *userData)
typedef TEXTURE_DATA_FREE_FUNC(AssetsMemoryFreeFunc);

typedef struct AssetsMemoryAllocator {
	AssetsMemoryAllocateFunc *allocate;
	AssetsMemoryFreeFunc *free;
	void *userData;
	uintptr_t padding;
} AssetsMemoryAllocator;

typedef enum AssetType {
	AssetType_None = 0,
	AssetType_Texture,
	AssetType_Font,
} AssetType;

fpl_inline bool TextureDataIsValid(const TextureData *textureData) {
	if (textureData == fpl_null || textureData->data == fpl_null || textureData->width == 0 || textureData->height == 0 || textureData->components == 0) {
		return false;
	}
	return true;
}

fpl_extern bool TextureDataAllocate(AssetsMemoryAllocator *allocator, TextureData *target, const uint32_t w, const uint32_t h, const uint32_t components);
fpl_extern void TextureDataFree(AssetsMemoryAllocator *allocator, TextureData *texture);

fpl_extern bool TextureDataLoadFromFilePath(AssetsMemoryAllocator *allocator, TextureData *target, const char *filePath);
fpl_extern bool TextureDataLoadFromSourceRect(AssetsMemoryAllocator *allocator, const TextureData *source, TextureData *target, const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h);

fpl_extern void FontAssetFree(AssetsMemoryAllocator *allocator, FontAsset *font);

#endif // FINAL_ASSETS_H

#if defined(FINAL_ASSETS_IMPLEMENTATION) && !defined(FINAL_ASSETS_IMPLEMENTED)
#define FINAL_ASSETS_IMPLEMENTED

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#include <final_fontloader.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

static void *AssetsDefaultMemoryAllocate(const size_t size, void *userData) {
	if (size == 0) {
		return fpl_null;
	}
	void *result = STBI_MALLOC(size);
	return(result);
}

static void AssetsDefaultMemoryFree(void *ptr, void *userData) {
	if (ptr == fpl_null) {
		return;
	}
	STBI_FREE(ptr);
}

static AssetsMemoryAllocator gAssetsDefaultMemoryAllocator = {
	AssetsDefaultMemoryAllocate,
	AssetsDefaultMemoryFree,
	fpl_null,
	0,
};

static void *AssetsMemAllocateInternal(AssetsMemoryAllocator *allocator, const size_t size) {
	if (allocator == fpl_null) {
		allocator = &gAssetsDefaultMemoryAllocator;
	}
	fplAssertPtr(allocator->allocate);
	void *result = allocator->allocate(size, allocator->userData);
	return result;
}

static void AssetsMemFreeInternal(AssetsMemoryAllocator *allocator, void *ptr) {
	if (allocator == fpl_null) {
		allocator = &gAssetsDefaultMemoryAllocator;
	}
	fplAssertPtr(allocator->free);
	allocator->free(ptr, allocator->userData);
}

fpl_extern void TextureDataFree(AssetsMemoryAllocator *allocator, TextureData *texture) {
	if (TextureDataIsValid(texture)) {
		// TODO(final): Logging (Invalid Arguments)
		return;
	}
	AssetsMemFreeInternal(allocator, texture->data);
	fplClearStruct(texture);
}

fpl_extern bool TextureDataAllocate(AssetsMemoryAllocator *allocator, TextureData *target, const uint32_t w, const uint32_t h, const uint32_t components) {
	if (target == fpl_null || w == 0 || h == 0 || components == 0 || components > 4) {
		// TODO(final): Logging (Invalid Arguments)
		return false;
	}
	if (allocator == fpl_null) {
		allocator = &gAssetsDefaultMemoryAllocator;
	}
	size_t size = w * h * sizeof(uint8_t) * components;
	uint8_t *data = (uint8_t *)AssetsMemAllocateInternal(allocator, size);
	if (data == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		return false;
	}
	fplClearStruct(target);
	target->width = w;
	target->height = h;
	target->components = components;
	target->data = data;
	return true;
}

fpl_extern bool TextureDataLoadFromSourceRect(AssetsMemoryAllocator *allocator, const TextureData *source, TextureData *target, const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h) {
	if (!TextureDataIsValid(source) || target == fpl_null || w == 0 || h == 0) {
		// TODO(final): Logging (Invalid Arguments)
		return false;
	}
	uint32_t components = source->components;
	if (components > 4) {
		// TODO(final): Logging (Source texture data is invalid)
		return false;
	}

	if (components != 4) {
		// TODO(final): Logging (Only 32-bit texture data are supported right now)
		return false;
	}

	TextureData result = fplZeroInit;
	if (!TextureDataAllocate(allocator, &result, w, h, components)) {
		// TODO(final): Logging (Insufficient memory)
		return false;
	}

	uint32_t sourceScanline = source->width * components;
	uint32_t destScanline = w * components;
	uint32_t dstY = 0;
	uint32_t endX = x + w;
	uint32_t endY = y + h;
	for (uint32_t srcY = y; srcY < endY; ++srcY) {
		uint32_t dstX = 0;
		for (uint32_t srcX = x; srcX < endX; ++srcX) {
			uint8_t *src = source->data + (srcY * sourceScanline + srcX * components);
			uint8_t *dst = result.data + (dstY * destScanline + dstX * components);

			// TODO(final): Support non 32-bit components!

			uint32_t *srcPixel = (uint32_t *)src;
			uint32_t *dstPixel = (uint32_t *)dst;

			*dstPixel = *srcPixel;
			++dstX;
		}
		++dstY;
	}

	*target = result;

	return true;
}

fpl_extern bool TextureDataLoadFromFilePath(AssetsMemoryAllocator *allocator, TextureData *target, const char *filePath) {
	if (target == fpl_null || fplGetStringLength(filePath) == 0) {
		return false;
	}

	fplFileHandle file = fplZeroInit;
	uint8_t *fileBuffer = fpl_null;

	bool result = false;

	if (!fplFileOpenBinary(filePath, &file)) {
		// TODO: Logging (File not found);
		goto failed;
	}

	uint32_t fileLen = fplFileGetSizeFromHandle32(&file);
	if (fileLen == 0) {
		// TODO(final): Logging (Empty file)
		goto failed;
	}

	fileBuffer = (uint8_t *)AssetsMemAllocateInternal(allocator, fileLen);
	if (fileBuffer == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		goto failed;
	}

	if (!fplFileReadBlock32(&file, fileLen, fileBuffer, fileLen) == fileLen) {
		// TODO(final): Logging (Failed to load the file into memory)
		goto failed;
	}

	int imageWidth = 0;
	int imageHeight = 0;
	int imageComponents = 0;
	stbi_set_flip_vertically_on_load(0);
	stbi_uc *imageData = stbi_load_from_memory(fileBuffer, fileLen, &imageWidth, &imageHeight, &imageComponents, 4);
	if (imageData == fpl_null) {
		// TODO(final): Logging (Failed to load/decode the image from memory)
		goto failed;
	}

	fplClearStruct(target);
	target->width = imageWidth;
	target->height = imageHeight;
	target->components = imageComponents;
	target->data = imageData;
	result = true;
	goto done;

failed:
	result = false;

done:
	if (fileBuffer != fpl_null) {
		AssetsMemFreeInternal(allocator, fileBuffer);
	}
	if (file.isValid) {
		fplFileClose(&file);
	}
	return result;
}

fpl_extern void FontAssetFree(AssetsMemoryAllocator *allocator, FontAsset *font) {
	if (font == fpl_null) {
		return;
	}

	// @NOTE(final): Texture allocation is tied to the renderer itself, such as OpenGL so we cannot free the texture handle here

	ReleaseFont(&font->desc);
}

#endif // FINAL_ASSETS_IMPLEMENTATION