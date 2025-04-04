/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | No Runtime Linking

Description:
	Demo for showing how FPL works without runtime-linking, falling back to static linking.

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- Linking to POSIX libraries (pthread, x11, gl, libasound, libvulkan, m)
	- Linking to Win32 libraries (kernel32.lib, user32.lib, shell32.lib, gdi32.lib, ole32.lib, opengl32.lib, vulkan.lib, xinput.lib, dsound.lib)

Author:
	Torsten Spaete

Changelog:
	## 2025-03-28
	- Updated documentation

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-10-01
	- Initial version

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_RUNTIME_LINKING
#include <final_platform_layer.h>

int main(int argc, char **args) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("No Runtime Linking", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.backend = fplVideoBackendType_Software;
	settings.video.isAutoSize = true;
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		while (fplWindowUpdate()) {
			fplEvent ev;
			while (fplPollEvent(&ev)) {}
			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();
			for (uint32_t y = 0; y < backBuffer->height; ++y) {
				uint32_t *p = (uint32_t *)((uint8_t *)backBuffer->pixels + y * backBuffer->lineWidth);
				for (uint32_t x = 0; x < backBuffer->width; ++x) {
					uint32_t color = 0xFFFFFF00;
					*p++ = color;
				}
			}
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}