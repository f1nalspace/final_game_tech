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
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// We are a entry point
#define FPL_ENTRYPOINT

// We are a console application
#ifndef FPL_APPTYPE_CONSOLE
#	define FPL_APPTYPE_CONSOLE
#endif

// Disable platform includes, such as windows.h, pthread.h, etc.
#ifndef FPL_NO_PLATFORM_INCLUDES
#	define FPL_NO_PLATFORM_INCLUDES
#endif

// Use semi-opaque handles in the header
#ifndef FPL_OPAQUE_HANDLES
#	define FPL_OPAQUE_HANDLES
#endif

// FPL header
#include <final_platform_layer.h>

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplConsoleOut("Hello World without platform includes!\n");
		fplPlatformRelease();
	}
	return 0;
}