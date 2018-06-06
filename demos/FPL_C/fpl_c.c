/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | C
Description:
	Simple Platform-Initialization to test compilation in C99
Requirements:
	No requirements
Author:
	Torsten Spaete
Changelog:
	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C always
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
	fplSettings settings = fplMakeDefaultSettings();
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		while (fplWindowUpdate()) {
			fplClearEvents();
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}