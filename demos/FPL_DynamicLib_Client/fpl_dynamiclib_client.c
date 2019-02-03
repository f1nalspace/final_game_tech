// Force the inclusion of the main entry point
#define FPL_ENTRYPOINT
// Use FPL as DLL client and import every public function
#define FPL_DLLIMPORT
// Include the FPL header file
#include <final_platform_layer.h>

int main(int argc, char **argv) {
	if (fplPlatformInit(fplInitFlags_All, fpl_null)) {
		while (fplWindowUpdate()) {
			fplPollEvents();
		}
		fplPlatformRelease();
	}
	return(0);
}