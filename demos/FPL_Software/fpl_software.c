/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Software

Description:
	Simple demo drawing random pixels with software graphics rendering

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2025-01-28
	- Fixed makefiles for CC/CMake was broken

	## 2021-05-16
	- Use fplPollEvents() instead

	## 2020-04-20
	- Much better rendering

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-05-11:
	- Added CMakeLists.txt
	- Fixed Makefile was not working anymore

	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <final_math.h>

#define FINAL_GRAPHICS_IMPLEMENTATION
#include <final_graphics.h>

typedef struct RandomSeries {
	uint16_t index;
} RandomSeries;

static uint16_t RandomU16(RandomSeries *series) {
	series->index ^= (series->index << 13);
	series->index ^= (series->index >> 9);
	series->index ^= (series->index << 7);
	return (series->index);
}

static uint8_t RandomByte(RandomSeries *series) {
	uint8_t result = RandomU16(series) % UCHAR_MAX;
	return(result);
}

int main(int argc, char **args) {
	fplColor32 backColor = fplCreateColorRGBA(39, 58, 91, 255);

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Software Rendering Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.backend = fplVideoBackendType_Software;
	settings.video.isAutoSize = true;
	settings.window.background = backColor;

	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		fplWindowSize winSize = fplZeroInit;
		fplGetWindowSize(&winSize);

		RandomSeries series = { 1337 };
		float dt = 1.0f / 60.0f;
		float margin = winSize.width / 50.0f;
		Vec2f rectRadius = V2fInit(winSize.width / 25.0f, winSize.width / 25.0f);
		Vec2f rectVel = V2fInit(rectRadius.x * 4.0f, rectRadius.y * 4.0f);
		Vec2f rectPos = V2fInit(rectRadius.w, rectRadius.h);
		while (fplWindowUpdate()) {
			fplPollEvents();

			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();

			// World
			float worldWidth = backBuffer->width - margin * 2;
			float worldHeight = backBuffer->height - margin * 2;
			float worldLeft = 0;
			float worldRight = worldWidth;
			float worldTop = 0;
			float worldBottom = worldHeight;

			// Background
			for (uint32_t i = 0; i < backBuffer->width * backBuffer->height; ++i) {
				uint32_t *p = (uint32_t *)((uint8_t *)backBuffer->pixels + i * backBuffer->pixelStride);
				*p = backColor.value;
			}

			// Noise
			uint32_t stepX = 5;
			uint32_t stepY = 5;
			fplColor32 pixelColor = fplCreateColorRGBA(0, 0, 0, 255);
			for (uint32_t y = 0; y < backBuffer->height; ++y) {
				if (y % stepY == 0) {
					uint32_t *p = (uint32_t *)((uint8_t *)backBuffer->pixels + y * backBuffer->lineWidth);
					for (uint32_t x = 0; x < backBuffer->width; ++x) {
						if (x % stepX == 0) {
							pixelColor.components.r = RandomByte(&series);
							pixelColor.components.g = RandomByte(&series);
							pixelColor.components.b = RandomByte(&series);
							*p++ = pixelColor.value;
						} else {
							++p;
						}
					}
				}
			}

			// Area
			BackbufferDrawLine(backBuffer, margin + worldLeft, margin + worldTop, margin + worldRight, margin + worldTop, 0xFFFFFF00);
			BackbufferDrawLine(backBuffer, margin + worldLeft, margin + worldBottom, margin + worldRight, margin + worldBottom, 0xFFFFFF00);
			BackbufferDrawLine(backBuffer, margin + worldLeft, margin + worldTop, margin + worldLeft, margin + worldBottom, 0xFFFFFF00);
			BackbufferDrawLine(backBuffer, margin + worldRight, margin + worldTop, margin + worldRight, margin + worldBottom, 0xFFFFFF00);

			// Moving rectangle
			float rectX = margin + rectPos.x - rectRadius.w;
			float rectY = margin + rectPos.y - rectRadius.h;
			BackbufferDrawRect(backBuffer, rectX, rectY, rectX + rectRadius.w * 2, rectY + rectRadius.h * 2, 0xFFFFFFFF);

			fplVideoFlip();

			// Integration
			rectPos = V2fAddMultScalar(rectPos, rectVel, dt);

			// Collision
			if (rectPos.y < (worldTop + rectRadius.h)) {
				rectPos.y = worldTop + rectRadius.h;
				rectVel.y *= -1;
			} else if (rectPos.y > (worldBottom - rectRadius.h)) {
				rectPos.y = worldBottom - rectRadius.h;
				rectVel.y *= -1;
			}

			if (rectPos.x < (worldLeft + rectRadius.w)) {
				rectPos.x = worldLeft + rectRadius.w;
				rectVel.x *= -1;
			} else if (rectPos.x > (worldRight - rectRadius.w)) {
				rectPos.x = worldRight - rectRadius.w;
				rectVel.x *= -1;
			}
		}
		fplPlatformRelease();
	}
	return 0;
}