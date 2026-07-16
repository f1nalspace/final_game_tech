/*
Name:
	Final Assets

Description:
	Simple asset system.

	This file is part of the final_framework.

Changelog:

	## 2026-07-16
	- Fixed heap corruption for releasing TextureData with the normal memory allocator instead of stbi_image_free()

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_ASSETS_H
#define FINAL_ASSETS_H

#include <final_platform_layer.h>

#include <final_core.h>
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
	// True when data came out of the image decoder (stb_image) and must be given back to it
	// (stbi_image_free), never to a MemoryAllocator. The default allocator free is fplMemoryFree
	// (VirtualFree), and VirtualFree page-aligns the address DOWN before validating: a LARGE decode
	// (a multi-MB scan owns its own VirtualAlloc region, with the malloc block in the first page)
	// rounds to exactly the region base, so the release SUCCEEDS and rips the heap's own region away
	// while the heap still lists the block as busy -- delayed heap corruption on the next allocation.
	bool isDecoderOwned;
} TextureData;

typedef struct TextureAsset {
	TextureData data;
	TextureHandle texture;
} TextureAsset;

typedef enum AssetType {
	AssetType_None = 0,
	AssetType_Texture,
	AssetType_Font,
} AssetType;

fpl_extern_inline bool TextureDataIsValid(const TextureData *textureData) {
	if (textureData == fpl_null || textureData->data == fpl_null || textureData->width == 0 || textureData->height == 0 || textureData->components == 0) {
		return false;
	}
	return true;
}

fpl_extern bool TextureDataAllocate(MemoryAllocator *allocator, TextureData *target, const uint32_t w, const uint32_t h, const uint32_t components);
fpl_extern void TextureDataFree(MemoryAllocator *allocator, TextureData *texture);

fpl_extern bool TextureDataLoadFromFile(MemoryAllocator *allocator, TextureData *target, const char *filePath);
fpl_extern bool TextureDataLoadFromSourceRect(MemoryAllocator *allocator, const TextureData *source, TextureData *target, const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h);

fpl_extern void FontAssetFree(MemoryAllocator *allocator, FontAsset *font);

#endif // FINAL_ASSETS_H

#if (defined(FINAL_ASSETS_IMPLEMENTATION) && !defined(FINAL_ASSETS_IMPLEMENTED)) || FPL_IS_IDE

#ifndef FINAL_ASSETS_IMPLEMENTED
#define FINAL_ASSETS_IMPLEMENTED
#endif

// GCC at -O3 raises a false-positive -Wstringop-overflow inside stb_image's PNG tRNS
// parser (tc[3]): s->img_n is bounded to 1 or 3 there, but the optimizer can't prove it.
#if defined(__GNUC__) && !defined(__clang__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#if defined(__GNUC__) && !defined(__clang__)
#	pragma GCC diagnostic pop
#endif

#define FINAL_FONTLOADER_IMPLEMENTATION
#include <final_fontloader.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

fpl_extern void TextureDataFree(MemoryAllocator *allocator, TextureData *texture) {
	if (texture == fpl_null) {
		// TODO(final): Logging (Invalid Arguments)
		return;
	}
	if (TextureDataIsValid(texture)) {
		if (texture->isDecoderOwned) {
			// Decoder memory goes back to the decoder; see the field comment in TextureData.
			stbi_image_free(texture->data);
		} else {
			MemoryAllocatorFree(allocator, texture->data);
		}
	}
	fplClearStruct(texture);
}

fpl_extern bool TextureDataAllocate(MemoryAllocator *allocator, TextureData *target, const uint32_t w, const uint32_t h, const uint32_t components) {
	if (target == fpl_null || w == 0 || h == 0 || components == 0 || components > 4) {
		// TODO(final): Logging (Invalid Arguments)
		return false;
	}
	size_t size = w * h * sizeof(uint8_t) * components;
	uint8_t *data = (uint8_t *)MemoryAllocatorAlloc(allocator, size);
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

fpl_extern bool TextureDataLoadFromSourceRect(MemoryAllocator *allocator, const TextureData *source, TextureData *target, const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h) {
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

fpl_internal void InternalTextureDataLoadFromFileShutdown(MemoryAllocator *allocator, fplFileHandle *file, uint8_t *fileBuffer) {
	if (fileBuffer != fpl_null) {
		MemoryAllocatorFree(allocator, fileBuffer);
	}
	if (file->isValid) {
		fplFileClose(file);
	}
}

fpl_extern bool TextureDataLoadFromFile(MemoryAllocator *allocator, TextureData *target, const char *filePath) {
	if (target == fpl_null || fplGetStringLength(filePath) == 0) {
		return false;
	}

	fplFileHandle file = fplZeroInit;
	uint8_t *fileBuffer = fpl_null;

	if (!fplFileOpenBinary(filePath, &file)) {
		// TODO: Logging (File not found);
		InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);
		return false;
	}

	uint32_t fileLen = fplFileGetSizeFromHandle32(&file);
	if (fileLen == 0) {
		// TODO(final): Logging (Empty file)
		InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);
		return false;
	}

	fileBuffer = (uint8_t *)MemoryAllocatorAlloc(allocator, fileLen);
	if (fileBuffer == fpl_null) {
		// TODO(final): Logging (Insufficient memory)
		InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);
		return false;
	}

	if (fplFileReadBlock32(&file, fileLen, fileBuffer, fileLen) != fileLen) {
		// TODO(final): Logging (Failed to load the file into memory)
		InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);
		return false;
	}

	int imageWidth = 0;
	int imageHeight = 0;
	int imageComponents = 0;
	stbi_set_flip_vertically_on_load(0);
	stbi_uc *imageData = stbi_load_from_memory(fileBuffer, fileLen, &imageWidth, &imageHeight, &imageComponents, 4);
	if (imageData == fpl_null) {
		// TODO(final): Logging (Failed to load/decode the image from memory)
		InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);
		return false;
	}

	fplClearStruct(target);
	target->width = imageWidth;
	target->height = imageHeight;
	target->components = imageComponents;
	target->data = imageData;
	target->isDecoderOwned = true;

	InternalTextureDataLoadFromFileShutdown(allocator, &file, fileBuffer);

	return true;
}

fpl_extern void FontAssetFree(MemoryAllocator *allocator, FontAsset *font) {
	if (font == fpl_null) {
		return;
	}

	// @NOTE(final): Texture allocation is tied to the renderer itself, such as OpenGL so we cannot free the texture handle here

	FontFree(allocator, &font->desc);
}

#endif // FINAL_ASSETS_IMPLEMENTATION