#define FPL_IMPLEMENTATION
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_OPENGL 0
#include "final_platform_layer.h"

#include <stdio.h>

int main(int argc, char **args) {
	fpl_InitFlag initFlags = { 0 };
	fpl_Init(initFlags);

	char exeFilePath[1024];
	fpl_GetExecutableFilePath(exeFilePath, FPL_ARRAYCOUNT(exeFilePath));

	char exePath[1024];
	fpl_ExtractFilePath(exeFilePath, exePath, FPL_ARRAYCOUNT(exePath));
	printf(exePath);

	fpl_Release();
}