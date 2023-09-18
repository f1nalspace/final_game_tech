/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | StaticLib_Host

Description:
	FPL implementation compiled in a static library

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2018-09-24
	- Reflect api changes in FPL 0.9.2
	
	## 2018-04-24:
	- Initial creation of this description block

License:
	Copyright (c) 2017-2023 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION // Include the implementation
#define FPL_NO_ENTRYPOINT // Disable any point entry inclusion
// Disable video and audio
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>