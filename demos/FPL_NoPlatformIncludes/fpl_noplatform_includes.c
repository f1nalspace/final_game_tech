/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | NoPlatformIncludes

Description:
	Do not include any platform header files in the API.
	Use opaque handles for platform handles in the API.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2021-03-15
	- Initial version

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// We are a entry point
#define FPL_ENTRYPOINT
// We are a console application
#define FPL_APPTYPE_CONSOLE
// Disable platform includes, such as windows.h, pthread.h, etc.
#define FPL_NO_PLATFORM_INCLUDES
// Use semi-opaque handles in the header
#define FPL_OPAQUE_HANDLES
// FPL header
#include <final_platform_layer.h>

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplConsoleOut("Hello World without platform includes!\n");
		fplPlatformRelease();
	}
	return 0;
}