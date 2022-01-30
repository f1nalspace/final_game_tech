/*
-------------------------------------------------------------------------------
Name:
	Final's Testbed

Description:
	Playground for testing out all my stuff.

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/
#if 0

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

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

enum class AssetLoadState: int32_t {
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
	float angle;
	bool isExiting;
};

static bool Init(GameState &state) {
	state.debugFont.type = AssetType::Font;
	state.debugFont.loadState = AssetLoadState::Unloaded;
	size_t fontDataSize = fplArrayCount(fontDataArray);
	if(LoadFontFromMemory(fontDataArray, fontDataSize, 0, 36.0f, 32, 128, 512, 512, false, &state.debugFont.font.data)) {
		state.debugFont.loadState = AssetLoadState::ToUpload;
	}
	state.angle = 0.0f;
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
	fplAssert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if(!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
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
	fplAssert(state != nullptr);
	state->angle += input.fixedDeltaTime * 0.1f;
}

static Rect2f ComputeAspectRect(Vec2f targetSize, Vec2f sourceSize, Ratio sourceRatio) {
	float aspect_ratio;
	if(sourceRatio.numerator == 0.0) {
		aspect_ratio = 0.0f;
	} else {
		aspect_ratio = (float)ComputeRatio(sourceRatio);
	}
	if(aspect_ratio <= 0.0f) {
		aspect_ratio = 1.0f;
	}
	aspect_ratio *= sourceSize.w / sourceSize.h;

	float height = targetSize.h;
	float width = height * aspect_ratio;
	if(width > targetSize.w) {
		width = targetSize.w;
		height = width / aspect_ratio;
	}

	float x = (targetSize.w - width) * 0.5f;
	float y = (targetSize.h - height) * 0.5f;

	Rect2f result = R2fInit(V2fInit(x, y), V2fInit(width, height));
	return(result);
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
	RenderState &renderState = *gameMemory.render;

	if(state->debugFont.loadState == AssetLoadState::ToUpload) {
		fplAssert(state->debugFont.type == AssetType::Font);
		const LoadedFont &font = state->debugFont.font.data;
		PushTexture(renderState, &state->debugFont.font.texture, font.atlasAlphaBitmap, font.atlasWidth, font.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
		state->debugFont.loadState = AssetLoadState::Loaded;
	}

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4fInit(0.1f, 0.2f, 0.3f, 1), ClearFlags::Color);

	float w = 10.0f, h = 6.0f;
	Vec2f viewSize = V2fInit(w, h);

#define DEMO_IMAGEFIT 1
#define DEMO_TEST 2

#define DEMO DEMO_IMAGEFIT

#if DEMO == DEMO_IMAGEFIT
	Mat4f proj = Mat4OrthoRH(0.0f, viewSize.w, viewSize.h, 0.0f, 0.0f, 1.0f);
	Mat4f view = Mat4TranslationV2(V2fInit(0, 0)) * Mat4ScaleV2(V2fInit(1, 1));
	SetMatrix(renderState, proj * view);

	Vec2f maxSize = viewSize * 0.75f;
	Vec2f maxPos = (viewSize - maxSize) * 0.5f;

	Vec2f sourceImageSize = V2fInit(1000, 100);
	Ratio sourceImageAspect = MakeRatio(1, 1);
	float containerAspect = maxSize.w / maxSize.h;

	Rect2f imageRect = ComputeAspectRect(maxSize, sourceImageSize, sourceImageAspect);

	Vec2f imageSize = imageRect.size;

	Vec2f imageExt = imageSize * 0.5f;

	Vec2f imageCenter = maxPos + imageRect.pos + imageExt;

	PushRectangle(renderState, maxPos, maxSize, V4fInit(1, 1, 1, 1), false, 1.0f);

	PushRectangle(renderState, maxPos + imageRect.pos, imageSize, V4fInit(1, 0, 0, 1), false, 1.0f);

	float imageRot = state->angle;

	Mat4f initialTranslationMat = Mat4TranslationV2(imageCenter);
	Mat4f imageRotMat = Mat4RotationZFromAngle(imageRot);
	Mat4f imageMat = initialTranslationMat * imageRotMat;

	Vec2f verts[] = {
		V2fInit(-imageExt.w, -imageExt.h),
		V2fInit(imageExt.w, -imageExt.h),
		V2fInit(imageExt.w, imageExt.h),
		V2fInit(-imageExt.w, imageExt.h),
	};

	Vec2f min = Vec4MultMat4(imageRotMat, V4fInitXY(verts[0], 0.0f, 1.0f)).xy;
	Vec2f max = min;
	for(int i = 1; i < fplArrayCount(verts); ++i) {
		Vec2f v = Vec4MultMat4(imageRotMat, V4fInitXY(verts[i], 0.0f, 1.0f)).xy;
		min = V2fMin(min, v);
		max = V2fMax(max, v);
	}

	Vec2f rotatedSize = max - min;

	float factor = 1.0f;
	float rotatedAspect = rotatedSize.w / rotatedSize.h;
	if(rotatedAspect > containerAspect) {
		factor = maxSize.w / rotatedSize.w;
	} else {
		factor = maxSize.h / rotatedSize.h;
	}

	Vec2f scaledSize = imageSize * factor;

	PushMatrix(renderState, imageMat);
	PushRectangle(renderState, -imageExt, imageSize, V4fInit(0, 1, 0, 1), false, 1.0f);
	PopMatrix(renderState);

	PushRectangle(renderState, imageCenter - rotatedSize * 0.5f, rotatedSize, V4fInit(0, 0, 1, 1), false, 1.0f);

	PushMatrix(renderState, imageMat);
	PushRectangle(renderState, -scaledSize * 0.5f, scaledSize, V4fInit(0, 1, 1, 1), false, 2.0f);
	PopMatrix(renderState);
#endif // DEMO_IMAGEFIT

#if DEMO == DEMO_TEST
	Mat4f proj = Mat4OrthoRH(-viewSize.w * 0.5f, viewSize.w * 0.5f, -viewSize.h * 0.5f, viewSize.h * 0.5f, 0.0f, 1.0f);
	Mat4f view = Mat4TranslationV2(V2fInit(0, 0)) * Mat4ScaleV2(V2fInit(1, 1));
	SetMatrix(renderState, proj * view);

	PushRectangleCenter(renderState, V2fInit(0, 0), V2fInit(w * 0.2f, h * 0.2f), V4fInit(1, 1, 1, 1), false, 1.0f);

	PushRectangleCenter(renderState, V2fInit(0, 0), V2fInit(w * 0.1f, h * 0.1f), V4fInit(1, 1, 1, 1), true, 0.0f);

	Vec2f verts[] = {
		V2fInit(0.0f, h * 0.3f),
		V2fInit(-w * 0.3f, -h * 0.3f),
		V2fInit(w * 0.3f, -h * 0.3f),
	};
	PushVertices(renderState, verts, fplArrayCount(verts), true, V4fInit(0, 1, 1, 1), DrawMode::Lines, true, 1.0f);

	view = Mat4TranslationV2(V2fInit(w * 0.25f, -h * 0.1f)) * Mat4ScaleV2(V2fInit(0.5f, 0.5f));
	SetMatrix(renderState, proj * view);
	PushVertices(renderState, verts, fplArrayCount(verts), true, V4fInit(1, 0, 1, 1), DrawMode::Polygon, true, 1.0f);

	view = Mat4TranslationV2(V2fInit(0, 0));
	SetMatrix(renderState, proj * view);
	PushText(renderState, "Hello", 5, &state->debugFont.font.data, &state->debugFont.font.texture, V2fInit(0, 0), h * 0.1f, 0.0f, 0.0f, V4fInit(1, 0, 0, 1));

#endif // DEMO_TEST
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
}

int main(int argc, char **argv) {
	GameConfiguration config = {};
	config.title = L"Final´s Testbed";
	config.hideMouseCursor = false;
	config.disableInactiveDetection = true;
	int result = GameMain(config);
	return(result);
}

#endif

// Final-Font Test
#if 1
#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FNT_IMPLEMENTATION
#include <final_font.h>

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	if(fplPlatformInit(fplInitFlags_All, &settings)) {
		if(fglLoadOpenGL(true)) {

			const char *fontFilePath = "c:/windows/fonts/l_10646.ttf";
			const char *fontName = "Lucida Sans Unicode";
			if(fplFileExists(fontFilePath)) {
				fplFileHandle file;
				if(fplFileOpenBinary(fontFilePath, &file)) {
					size_t size = fplFileGetSizeFromHandle(&file);
					uint8_t *data = (uint8_t *)malloc(size);
					fplFileReadBlock(&file, size, data, size);
					fplFileClose(&file);

					fntFontData fontData = fplZeroInit;
					fontData.size = size;
					fontData.data = data;

					fntFontInfo fontInfo = fplZeroInit;
					if(fntLoadFontInfo(&fontData, &fontInfo, 0, 40.0f)) {
						fntFontAtlas *atlas = fntCreateFontAtlas();
						if(atlas != fpl_null) {
							fntFontContext *ctx = fntCreateFontContext(&fontData, &fontInfo, 512);
							if(ctx != fpl_null) {
								fntAddToFontAtlas(ctx, atlas, 32, 128);
								fntFreeFontContext(ctx);
							}
							fntFreeFontAtlas(atlas);
						}
						fntFreeFontInfo(&fontInfo);
					}

					free(data);
				}
			}

			while(fplWindowUpdate()) {
				fplEvent ev;
				while(fplPollEvent(&ev)) {

				}
			}

			fglUnloadOpenGL();
		}
		fplPlatformRelease();
	}
	return (0);
}
#endif