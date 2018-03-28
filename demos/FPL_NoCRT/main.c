#define FPL_IMPLEMENTATION
#define FPL_NO_CRT
#define FPL_APPTYPE_CONSOLE

#include <stdint.h>
#include <varargs.h>

int dummy_vsnprintf(char *buf, size_t bufLen, const char *format, va_list argList);
#define FPL_USERFUNC_vsnprintf dummy_vsnprintf

#include <final_platform_layer.h>

int dummy_vsnprintf(char *buf, size_t bufLen, const char *format, va_list argList) {
	return 0;
}

int main(int argc, char **argv) {
	if(fplPlatformInit(fplInitFlags_All, fpl_null)) {
		fplConsoleOut("Hello World!\n");
		char c = fplConsoleWaitForCharInput();
		fplConsoleFormatOut("%c\n", c);
		fplPlatformRelease();
	}
	return 0;
}