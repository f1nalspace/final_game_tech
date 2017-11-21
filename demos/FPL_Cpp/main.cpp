#define FPL_AUTO_NAMESPACE
#include <final_platform_layer.hpp>

int main(int argc, char **args) {
	Settings settings = DefaultSettings();
	CopyAnsiString("Software Rendering Example", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle) - 1);
	settings.video.driverType = VideoDriverType::Software;
	if (InitPlatform(InitFlags::Video, settings)) {
		VideoBackBuffer *videoContext = GetVideoBackBuffer();
		while (WindowUpdate()) {
			uint32_t *p = videoContext->pixels;
			for (uint32_t y = 0; y < videoContext->height; ++y) {
				uint32_t color = 0xFFFF0000;
				for (uint32_t x = 0; x < videoContext->width; ++x) {
					*p++ = color;
				}
			}
			WindowFlip();
		}
		ReleasePlatform();
	}
	return 0;
}