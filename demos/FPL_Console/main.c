#define FPL_IMPLEMENTATION
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_OPENGL 0
#define FPL_ENABLE_C_RUNTIME_LIBRARY 1
#include "final_platform_layer.h"

int main(int argc, char **args) {
	fpl_Init(fpl_InitFlag_None);

	char exeFilePath[1024];
	fpl_GetExecutableFilePath(exeFilePath, FPL_ARRAYCOUNT(exeFilePath));
	fpl_ConsoleFormatOut("%s\n", exeFilePath);

	char exePath[1024];
	fpl_ExtractFilePath(exePath, FPL_ARRAYCOUNT(exePath), exeFilePath);
	fpl_ConsoleFormatOut("%s\n", exePath);

	fpl_Release();
}