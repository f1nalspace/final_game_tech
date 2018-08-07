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

extern bool GameInit(GameMemory &gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;
	if(!Init(*state)) {
		GameRelease(gameMemory);
		return(false);
	}
	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	if(state != nullptr) {
		Kill(state);
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	FPL_ASSERT(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	FPL_ASSERT(state != nullptr);
	state->viewport.x = 0;
	state->viewport.y = 0;
	state->viewport.w = input.windowSize.w;
	state->viewport.h = input.windowSize.h;
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	FPL_ASSERT(state != nullptr);
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	FPL_ASSERT(state != nullptr);
	RenderState &renderState = *gameMemory.render;

	if(state->debugFont.loadState == AssetLoadState::ToUpload) {
		FPL_ASSERT(state->debugFont.type == AssetType::Font);
		const LoadedFont &font = state->debugFont.font.data;
		PushTexture(renderState, &state->debugFont.font.texture, font.atlasAlphaBitmap, font.atlasWidth, font.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
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
	PushVertices(renderState, verts, FPL_ARRAYCOUNT(verts), true, V4f(0, 1, 1, 1), DrawMode::Lines, true, 1.0f);

	view = Mat4Translation(V2f(w * 0.25f, -h * 0.1f)) * Mat4Scale(V2f(0.5f, 0.5f));
	SetMatrix(renderState, proj * view);
	PushVertices(renderState, verts, FPL_ARRAYCOUNT(verts), true, V4f(1, 0, 1, 1), DrawMode::Polygon, true, 1.0f);

	view = Mat4Translation(V2f(0, 0));
	SetMatrix(renderState, proj * view);
	PushText(renderState, "Hello", 5, &state->debugFont.font.data, &state->debugFont.font.texture, V2f(0, 0), h * 0.1f, 0.0f, 0.0f, V4f(1, 0, 0, 1));
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
}

int main(int argc, char **argv) {
	GameConfiguration config = {};
	config.title = "Final´s Testbed";
	config.hideMouseCursor = false;
	config.disableInactiveDetection = true;
	int result = GameMain(config);
	return(result);
}