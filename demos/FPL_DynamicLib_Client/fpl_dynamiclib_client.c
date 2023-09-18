/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Dynamic Library Client

Description:
	Showing how to use FPL as a dynamic shared library.

Requirements:
	- C99 Compiler

Author:
	Torsten Spaete

Changelog:
	## 2019-07-22
	- Initial version

License:
	Copyright (c) 2017-2023 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Force the inclusion of the main entry point
#define FPL_ENTRYPOINT
// Use FPL as DLL client and import every public function
#define FPL_DLLIMPORT
// Disable video and audio
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
// Include the FPL header file
#include <final_platform_layer.h>

int main(int argc, char **argv) {
	if (fplPlatformInit(fplInitFlags_All, fpl_null)) {
		while (fplWindowUpdate()) {
			fplPollEvents();
		}
		fplPlatformRelease();
	}
	return(0);
}