/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | StaticLib_Client

Description:
	Client application using the FPL static library

Requirements:
	- C99 Compiler
	- Final Platform Layer
	- StaticLib_Host (Static-Library)

Author:
	Torsten Spaete

Changelog:
	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-04-24:
	- Initial creation of this description block

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

 // Force the inclusion of the entry point
#define FPL_ENTRYPOINT
// Disable video and audio
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplEvent ev;
		while(fplWindowUpdate(&ev)) {
			fplEvent ev;
			while(fplPollEvent(&ev)) {}
		}
		fplPlatformRelease();
	}
	return 0;
}