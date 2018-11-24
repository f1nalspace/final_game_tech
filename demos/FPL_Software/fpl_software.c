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
-------------------------------------------------------------------------------
*/

#define FPL_ENTRYPOINT // Force the inclusion of the entry point
#include <final_platform_layer.h>

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
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("Software Rendering Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.driver = fplVideoDriverType_Software;
	settings.video.isAutoSize = true;
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		RandomSeries series = { 1337 };
		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {}
			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();
			for (uint32_t y = 0; y < backBuffer->height; ++y) {
				uint32_t *p = (uint32_t *)((uint8_t *)backBuffer->pixels + y * backBuffer->lineWidth);
				for (uint32_t x = 0; x < backBuffer->width; ++x) {
#if 1
					uint8_t r = RandomByte(&series);
					uint8_t g = RandomByte(&series);
					uint8_t b = RandomByte(&series);
					uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
#else
					uint32_t color = 0xFFFF00FF;
#endif
					*p++ = color;
				}
			}
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}