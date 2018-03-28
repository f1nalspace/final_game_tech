#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		while (fplWindowUpdate()) {
			fplClearEvents();
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}