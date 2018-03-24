#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

int main(int argc, char *args[]) {
	if (fplPlatformInit(fplInitFlags_All, nullptr)) {
		fplConsoleOut("Hello World");
		fplConsoleFormatOut("%s %s %d", "Hello", "World", 42);
		fplConsoleError("Hello World!");
		fplConsoleFormatError("%s %s %d!", "Hello", "World", 42);
		fplPlatformRelease();
	}
	return 0;
}
