#include <final_platform_layer.h>

// ############################################################################
//
// No C-Runtime Support
//
// This block contains stuff to make FPL compilable without the CRT.
//
// ############################################################################
#if defined(FPL_NO_CRT)

// @STUPID(final): MSVC requires this for No-CRT always, stupid compiler -.-
#if defined(FPL_COMPILER_MSVC)

#if defined(__cplusplus)
extern "C" {
#endif

	int _fltused = 0;

	// CRT-Math routines
	// Based on https://gist.github.com/mmozeiko/6a365d6c483fc721b63a#file-win32_crt_float-cpp
#	ifdef _M_IX86

#	define CRT_LOWORD(x) dword ptr [x+0]
#	define CRT_HIWORD(x) dword ptr [x+4]

// Long-Long multiplication
	__declspec(naked) void _allmul() {
#		define A       esp + 8       // stack address of a
#		define B       esp + 16      // stack address of b

		__asm
		{
			push    ebx

			mov     eax, CRT_HIWORD(A)
			mov     ecx, CRT_LOWORD(B)
			mul     ecx; eax has AHI, ecx has BLO, so AHI * BLO
			mov     ebx, eax; save result

			mov     eax, CRT_LOWORD(A)
			mul     CRT_HIWORD(B); ALO * BHI
			add     ebx, eax; ebx = ((ALO * BHI) + (AHI * BLO))

			mov     eax, CRT_LOWORD(A); ecx = BLO
			mul     ecx; so edx : eax = ALO * BLO
			add     edx, ebx; now edx has all the LO*HI stuff

			pop     ebx

			ret     16; callee restores the stack
		}

#		undef A
#		undef B
	}

	__declspec(naked) void _alldiv() {
		__asm
		{
		}
	}

	__declspec(naked) void _aulldiv() {
		__asm
		{
		}
	}

	__declspec(naked) void _dtol3() {
		__asm
		{
		}
	}


	__declspec(naked) void _dtoui3() {
		__asm
		{
		}
	}


	__declspec(naked) void _dtoul3() {
		__asm
		{
		}
	}


	__declspec(naked) void _ftol3() {
		__asm
		{
		}
	}


	__declspec(naked) void _ftoui3() {
		__asm
		{
		}
	}


	__declspec(naked) void _ftoul3() {
		__asm
		{
		}
	}


	__declspec(naked) void _ltod3() {
		__asm
		{
		}
	}


	__declspec(naked) void _ultod3() {
		__asm
		{
		}
	}

#	endif

	//
	// Intrinsics
	//
#	pragma function(memset)
	void *memset(void *dest, int value, size_t count) {
		fplMemorySet(dest, value, count);
		return (dest);
	}

#	pragma function(memcpy)
	void *memcpy(void *dest, const void *source, size_t count) {
		fplMemoryCopy(source, count, dest);
		return (dest);
	}

	//
	// Run-Time-Checks
	//
	void __cdecl _RTC_InitBase(void) {
	}
	void __cdecl _RTC_Shutdown(void) {
	}
	void __cdecl _RTC_CheckEsp(void) {
	}
	void __fastcall _RTC_CheckStackVars(void *_Esp, struct _RTC_framedesc *_Fd) {

	}
#if defined(__cplusplus)
};
#endif

#endif // FPL_COMPILER_MSVC

#endif // FPL_NO_CRT