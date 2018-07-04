/*
-------------------------------------------------------------------------------
Name:
	Final's Testbed

Description:
	Playground for testing out all my stuff.

Author:
	Torsten Spaete
-------------------------------------------------------------------------------
*/

#if 0
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <malloc.h>
#include <assert.h>
#include <algorithm>
int main(int argc, char **argv) {
	const char *filePath = "C:\\Users\\X123713\\Downloads\\SulphurPoint-Bold.otf";
	FILE *file;
	if(fopen_s(&file, filePath, "rb") == 0) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		fseek(file, 0, SEEK_SET);
		uint8_t *mem = (uint8_t *)malloc(size);
		fread_s(mem, size, size, 1, file);
		fclose(file);

		printf("const unsigned char dataArray[] = {\n");

		size_t maxColCount = 32;
		size_t remainingSize = size;
		const uint8_t *p = mem;
		size_t outByteCount = 0;
		while(remainingSize > 0) {
			size_t colCount = std::min(remainingSize, maxColCount);
			printf("\t");
			for(int col = 0; col < colCount; ++col) {
				uint8_t value = p[col];
				if(col > 0) {
					printf(",");
				}
				printf("0x%02x", value);
				++outByteCount;
			}
			remainingSize -= colCount;
			p += colCount;
			if(remainingSize > 0) {
				printf(",");
			}
			printf("\n");
		}

		assert(outByteCount == size);

		printf("};\n");

		free(mem);
	}
}
#endif

#if 1
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#define FINAL_FONTLOADER_BETTERQUALITY 1
#include <final_fontloader.h>

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

#include <final_math.h>

#include <new> // placement new

#include "static_data.h"

struct FontAsset {
	LoadedFont data;
	TextureHandle texture;
};

enum class AssetType {
	None,
	Font
};

enum class AssetLoadState : int32_t {
	Failed = -1,
	Unloaded = 0,
	ToUpload,
	ToFree,
	Loaded
};

struct Asset {
	AssetType type;
	AssetLoadState loadState;
	union {
		FontAsset font;
	};
};

struct GameState {
	Asset debugFont;
	Viewport viewport;
	size_t lastRenderMemoryUsed;
	bool isExiting;
};

static bool Init(GameState &state) {
	state.debugFont.type = AssetType::Font;
	state.debugFont.loadState = AssetLoadState::Unloaded;
	size_t fontDataSize = FPL_ARRAYCOUNT(fontDataArray);
	if(LoadFontFromMemory(fontDataArray, fontDataSize, 0, 36.0f, 32, 128, 512, 512, false, &state.debugFont.font.data)) {
		state.debugFont.loadState = AssetLoadState::ToUpload;
	}

	return(true);
}

static void Kill(GameState *state) {
	ReleaseFont(&state->debugFont.font.data);
}

extern GameMemory GameCreate() {
	if(!fglLoadOpenGL(true)) {
		return {};
	}
	GameMemory result = {};
	result.capacity = sizeof(GameState) + FPL_MEGABYTES(16);
	result.base = fplMemoryAllocate(result.capacity);
	if(result.base == nullptr) {
		return {};
	}
	GameState *state = new (result.base)GameState;
	result.used = sizeof(GameState);
	if(!Init(*state)) {
		GameDestroy(result);
	}
	return(result);
}

extern void GameDestroy(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	if(state != nullptr) {
		Kill(state);
		state->~GameState();
		fplMemoryFree(state);
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = (GameState *)gameMemory.base;
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;
	state->viewport.x = 0;
	state->viewport.y = 0;
	state->viewport.w = input.windowSize.w;
	state->viewport.h = input.windowSize.h;
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = (GameState *)gameMemory.base;
}

extern void GameRender(GameMemory &gameMemory, RenderState &renderState, const float alpha) {
	GameState *state = (GameState *)gameMemory.base;

	if(state->debugFont.loadState == AssetLoadState::ToUpload) {
		FPL_ASSERT(state->debugFont.type == AssetType::Font);
		const LoadedFont &font = state->debugFont.font.data;
		PushTexture(renderState, &state->debugFont.font.texture, font.atlasAlphaBitmap, font.atlasWidth, font.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		state->debugFont.loadState = AssetLoadState::Loaded;
	}

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4f(0.1f, 0.2f, 0.3f, 1), ClearFlags::Color);

	float w = 10.0f;
	float h = 6.0f;

	Mat4f proj = Mat4Ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, 0.0f, 1.0f);
	Mat4f view = Mat4Translation(V2f(0, 0)) * Mat4Scale(V2f(1, 1));
	SetMatrix(renderState, proj * view);

	PushRectangleCenter(renderState, V2f(0, 0), V2f(w * 0.2f, h * 0.2f), V4f(1, 1, 1, 1), false, 1.0f);
	PushRectangle(renderState, V2f(0, 0), V2f(w * 0.25f, h * 0.25f), V4f(1, 1, 1, 1), true, 0.0f);

	Vec2f verts[] = {
		V2f(0.0f, h * 0.3f),
		V2f(-w * 0.3f, -h * 0.3f),
		V2f(w * 0.3f, -h * 0.3f),
	};
	PushVertices(renderState, verts, FPL_ARRAYCOUNT(verts), true, V4f(0, 1, 1, 1), VerticesDrawMode::Lines, true, 1.0f);

	view = Mat4Translation(V2f(w * 0.25f, -h * 0.1f)) * Mat4Scale(V2f(0.5f, 0.5f));
	SetMatrix(renderState, proj * view);
	PushVertices(renderState, verts, FPL_ARRAYCOUNT(verts), true, V4f(1, 0, 1, 1), VerticesDrawMode::Polygon, true, 1.0f);

	view = Mat4Translation(V2f(0, 0));
	SetMatrix(renderState, proj * view);
	char text[256];
	fplFormatAnsiString(text, FPL_ARRAYCOUNT(text), "Used memory (Render): %zu / %zu", state->lastRenderMemoryUsed, renderState.memory.size);
	PushText(renderState, text, fplGetAnsiStringLength(text), &state->debugFont.font.data, &state->debugFont.font.texture, V2f(0, 0), h * 0.1f, 0.0f, 0.0f, V4f(1, 0, 0, 1));

	state->lastRenderMemoryUsed = renderState.memory.used;
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, RenderState &renderState, const float alpha) {
	GameInput(gameMemory, input);
	GameUpdate(gameMemory, input);
	GameRender(gameMemory, renderState, alpha);
}

int main(int argc, char **argv) {
	GameConfiguration config = {};
	config.title = "Final's Testbed";
	config.hideMouseCursor = false;
	config.disableInactiveDetection = true;
	int result = GameMain(config);
	return(result);
}
#endif