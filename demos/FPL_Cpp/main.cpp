#define FPL_AUTO_NAMESPACE
#include <final_platform_layer.hpp>

int main(int argc, char **args) {
	Settings settings = Settings();
	CopyAnsiString("Software Rendering Example", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle) - 1);
	settings.video.driverType = VideoDriverType::Software;
	if (InitPlatform(InitFlags::Video, settings)) {
		VideoBackBuffer *videoContext = GetVideoBackBuffer();
		while (WindowUpdate()) {
			size_t size = videoContext->height * videoContext->width;
			uint32_t *p = videoContext->pixels;
			for (uint32_t index = 0; index < size; ++index) {
				uint32_t color = 0xFF00FF00;
				*p++ = color;
			}
		}
		ReleasePlatform();
	}
	return 0;
}