#include <final_platform_layer.hpp>

int main(int argc, char **args) {
	fpl::Settings settings = fpl::Settings();
	fpl::strings::CopyAnsiString("Software Rendering Example", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle) - 1);
	settings.video.driverType = fpl::VideoDriverType::Software;
	if (fpl::InitPlatform(fpl::InitFlags::Video, settings)) {
		fpl::video::VideoBackBuffer *videoContext = fpl::video::GetVideoBackBuffer();
		while (fpl::window::WindowUpdate()) {
			size_t size = videoContext->height * videoContext->width;
			uint32_t *p = videoContext->pixels;
			for (uint32_t index = 0; index < size; ++index) {
				uint32_t color = 0xFF00FF00;
				*p++ = color;
			}
		}
		fpl::ReleasePlatform();
	}
	return 0;
}