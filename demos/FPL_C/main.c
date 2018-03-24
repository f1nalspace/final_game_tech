#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
	fplSettings settings = fplDefaultSettings();
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		while (fplWindowUpdate()) {
			fplEvent event;
			while (fplPollEvent(&event)) {
			}
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}