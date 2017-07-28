#define FPL_IMPLEMENTATION
#define FPL_ENABLE_OPENGL 0
#define FPL_ENABLE_WINDOW 0
#define FPL_ENABLE_C_RUNTIME_LIBRARY 0
#include "final_platform_layer.h"

int main(int argc, char **args) {
	int result = 0;
#if FPL_ENABLE_WINDOW
	if (fpl_Init(fpl_InitFlags_Window)) {
		while (fpl_WindowUpdate()) {
		}
#else
	if (fpl_Init(fpl_InitFlags_None)) {
#endif
		fpl_Release();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}