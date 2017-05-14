#define FPL_IMPLEMENTATION
#define FPL_USE_CLIB_ASSERTIONS 1
#include "final_platform_layer.h"

int main(int argc, char **args) {
	fpl_Init(fpl_InitFlag_All);
	while (fpl_Update()) {
		glClear(GL_COLOR_BUFFER_BIT);
		fpl_Flip();
	}
	fpl_Release();
	return 0;
}