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
	settings.video.driver = VideoDriverType::Software;
	if (InitPlatform(InitFlags::Video, settings)) {
		RandomSeries series = {1337};
		VideoBackBuffer *backBuffer = GetVideoBackBuffer();
		while (WindowUpdate()) {
			for (uint32_t y = 0; y < backBuffer->height; ++y) {
				uint32_t *p = (uint32_t *)((uint8_t *)backBuffer->pixels + y * backBuffer->lineWidth);
				for (uint32_t x = 0; x < backBuffer->width; ++x) {
#if 1
					uint8_t r = RandomByte(series);
					uint8_t g = RandomByte(series);
					uint8_t b = RandomByte(series);
					uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
#else
					uint32_t color = 0xFFFF00FF;
#endif
					*p++ = color;
				}
			}
			VideoFlip();
		}
		ReleasePlatform();
	}
	return 0;
}