#define FPL_AUTO_NAMESPACE
#define FPL_NO_AUDIO
#include "final_platform_layer.hpp"

struct RandomSeries {
	fpl_u16 index;
};

static fpl_u16 RandomU16(RandomSeries &series) {
	series.index ^= (series.index << 13);
    series.index ^= (series.index >> 9);
	series.index ^= (series.index << 7);
	return (series.index);
}

static fpl_u8 RandomByte(RandomSeries &series) {
	fpl_u8 result = RandomU16(series) % FPL_MAX_U8;
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
			fpl_u32 *p = videoContext->pixels;
			for (fpl_u32 y = 0; y < videoContext->height; ++y) {
				for (fpl_u32 x = 0; x < videoContext->width; ++x) {
					fpl_u8 r = RandomByte(series);
					fpl_u8 g = RandomByte(series);
					fpl_u8 b = RandomByte(series);
					fpl_u32 color = (0xFF << 24) | (r << 16) | (g << 8) | b;
					*p++ = color;
				}
			}
			WindowFlip();
		}
		ReleasePlatform();
	}
	return 0;
}