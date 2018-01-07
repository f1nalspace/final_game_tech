#define FPL_AUTO_NAMESPACE
#define FPL_NO_AUDIO
#include "final_platform_layer.hpp"

struct RandomSeries {
	uint16_t index;
};

static uint16_t RandomU16(RandomSeries &series) {
	series.index ^= (series.index << 13);
    series.index ^= (series.index >> 9);
	series.index ^= (series.index << 7);
	return (series.index);
}

static uint8_t RandomByte(RandomSeries &series) {
	uint8_t result = RandomU16(series) % UCHAR_MAX;
	return(result);
}

int main(int argc, char **args) {
	Settings settings = DefaultSettings();
	CopyAnsiString("Software Rendering Example", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle) - 1);
	settings.video.driverType = VideoDriverType::Software;
	if (InitPlatform(InitFlags::Video, settings)) {
		RandomSeries series = {1337};
		VideoBackBuffer *videoContext = GetVideoBackBuffer();
		while (WindowUpdate()) {
			uint32_t *p = videoContext->pixels;
			for (uint32_t y = 0; y < videoContext->height; ++y) {
				for (uint32_t x = 0; x < videoContext->width; ++x) {
					uint8_t r = RandomByte(series);
					uint8_t g = RandomByte(series);
					uint8_t b = RandomByte(series);
					uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
					*p++ = color;
				}
			}
			WindowFlip();
		}
		ReleasePlatform();
	}
	return 0;
}