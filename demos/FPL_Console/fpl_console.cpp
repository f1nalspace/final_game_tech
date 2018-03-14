#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.hpp>

int main(int argc, char *args[]) {
	if(fpl::InitPlatform(fpl::InitFlags::All)) {
		fpl::console::ConsoleOut("Hello World");
		fpl::console::ConsoleFormatOut("%s %s %d", "Hello", "World", 42);
		fpl::console::ConsoleError("Hello World!");
		fpl::console::ConsoleFormatError("%s %s %d!", "Hello", "World", 42);
		fpl::ReleasePlatform();
	}
	return 0;
}
