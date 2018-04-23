/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Console
Description:
	A Hello-World Console Application testing Console Output/Input
Requirements:
	No requirements
Author:
	Torsten Spaete
Changelog:
	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always
	- Added read char input
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

int main(int argc, char *args[]) {
	if (fplPlatformInit(fplInitFlags_All, NULL)) {
		fplConsoleOut("Hello World");
		fplConsoleFormatOut("%s %s %d", "Hello", "World", 42);
		fplConsoleError("Error: Hello World!");
		fplConsoleFormatError("Error: %s %s %d!", "Hello", "World", 42);
		fplConsoleOut("Press enter a key: ");
		int key = fplConsoleWaitForCharInput();
		fplConsoleFormatOut("You pressed '%c'\n", key);
		fplPlatformRelease();
	}
	return 0;
}
