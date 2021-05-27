/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | NoCRT

Description:
	Compiling a FPL Hello-World Console Application without the need of the CRT

Requirements:
	- C99 Compiler
	- Win32 / MSVC only
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2021-02-24
	- Fixed missing _ultod3 for double math

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

// Define dummy vsnprintf (requires stdint.h and stdarg.h, before FPL is included)
#include <stdint.h>
#include <stdarg.h>
int dummy_vsnprintf(char *buf, size_t bufLen, const char *format, va_list argList);
#define FPL_USERFUNC_vsnprintf dummy_vsnprintf

#define FPL_IMPLEMENTATION
// We are a console application
#define FPL_APPTYPE_CONSOLE
// Disable C-RunTime Library
#define FPL_NO_CRT
// Disable video and audio
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
// FPL header
#include <final_platform_layer.h>

// Include tinycrt.h which simulates a tiny-crt ()
// Most of the asm instructions there are not implemented yet
// The user is responsible to use their one "Tiny-CRT" and write intrinsics for _ltod3, _ftol2, _allmul
#include "tinycrt.h"

int dummy_vsnprintf(char *buf, size_t bufLen, const char *format, va_list argList) {
	return 0;
}

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplConsoleOut("Hello World with the CRT!\n");
		char c = fplConsoleWaitForCharInput();
		fplConsoleFormatOut("%c\n", c);
		fplPlatformRelease();
	}
	return 0;
}