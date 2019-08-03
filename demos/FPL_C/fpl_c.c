/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | C

Description:
	Simple Platform-Initialization to test compilation in C99

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args) {
	fplSettings settings = fplMakeDefaultSettings();
	if(fplPlatformInit(fplInitFlags_All, &settings)) {
		while(fplWindowUpdate()) {
			fplEvent ev;
			while(fplPollEvent(&ev)) {}
			fplVideoFlip();
		}
		fplPlatformRelease();
	}
	return 0;
}