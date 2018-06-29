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
	if (fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplConsoleOut("Hello World\n");
		fplConsoleFormatOut("%s %s %d\n", "Hello", "World", 42);
		fplConsoleError("Error: Hello World!\n");
		fplConsoleFormatError("Error: %s %s %d!\n", "Hello", "World", 42);
		fplConsoleOut("Press enter a key: ");
		int key = fplConsoleWaitForCharInput();
		fplConsoleFormatOut("You pressed '%c'\n", key);
		fplPlatformRelease();
	}
	return 0;
}
