/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Dynamic Library Host

Description:
	Showing how to compile FPL into a dynamic shared library.

Requirements:
	- C99 Compiler

Author:
	Torsten Spaete

Changelog:
	## 2019-07-22
	- Initial version
-------------------------------------------------------------------------------
*/

// Include the entire implementation of FPL
#define FPL_IMPLEMENTATION
// Disable the inclusion of the main entry point
#define FPL_NO_ENTRYPOINT
// Use FPL as DLL host and export every public function 
#define FPL_DLLEXPORT
// Include the FPL header file
#include <final_platform_layer.h>