#include <final_platform_layer.h>

// ############################################################################
//
// No C-Runtime Support
//
// This block contains stuff to make FPL compilable without the CRT.
//
// ############################################################################
#if defined(FPL_NO_CRT)

// @NOTE(final): At the moment only MSVC requires this i assume
#if defined(FPL_COMPILER_MSVC)

#if defined(__cplusplus)
extern "C" {
#endif

	__declspec(selectany) int _fltused = 0;

	// CRT-Math routines, based on:
	// https://gist.github.com/mmozeiko/6a365d6c483fc721b63a#file-win32_crt_float-cpp
	// libSDL
#	ifdef _M_IX86

	__declspec(naked) void _ftol2()
    {
        __asm
        {
            fistp qword ptr [esp-8]
            mov   edx,[esp-4]
            mov   eax,[esp-8]
            ret
        }
    }

    __declspec(naked) void _ftol2_sse()
    {
        __asm
        {
            fistp dword ptr [esp-4]
            mov   eax,[esp-4]
            ret
        }
    }

	__declspec(naked) void _ltod3()
    {
        __asm
        {
        }
    }

	__declspec(naked) void _ultod3() {
		__asm
		{
		}
	}

	__declspec(naked) void _ftoui3() {
		__asm
		{
		}
	}

	// 64-bit math for 32-bit
	void __declspec(naked) _allmul() {
		__asm {
			mov         eax, dword ptr[esp + 8]
			mov         ecx, dword ptr[esp + 10h]
			or ecx, eax
			mov         ecx, dword ptr[esp + 0Ch]
			jne         hard
			mov         eax, dword ptr[esp + 4]
			mul         ecx
			ret         10h
			hard :
			push        ebx
				mul         ecx
				mov         ebx, eax
				mov         eax, dword ptr[esp + 8]
				mul         dword ptr[esp + 14h]
				add         ebx, eax
				mov         eax, dword ptr[esp + 8]
				mul         ecx
				add         edx, ebx
				pop         ebx
				ret         10h
		}
	}

	void __declspec(naked) _alldiv() {
		__asm {
			push        edi
			push        esi
			push        ebx
			xor         edi, edi
			mov         eax, dword ptr[esp + 14h]
			or eax, eax
			jge         L1
			inc         edi
			mov         edx, dword ptr[esp + 10h]
			neg         eax
			neg         edx
			sbb         eax, 0
			mov         dword ptr[esp + 14h], eax
			mov         dword ptr[esp + 10h], edx
			L1 :
			mov         eax, dword ptr[esp + 1Ch]
				or eax, eax
				jge         L2
				inc         edi
				mov         edx, dword ptr[esp + 18h]
				neg         eax
				neg         edx
				sbb         eax, 0
				mov         dword ptr[esp + 1Ch], eax
				mov         dword ptr[esp + 18h], edx
				L2 :
			or eax, eax
				jne         L3
				mov         ecx, dword ptr[esp + 18h]
				mov         eax, dword ptr[esp + 14h]
				xor edx, edx
				div         ecx
				mov         ebx, eax
				mov         eax, dword ptr[esp + 10h]
				div         ecx
				mov         edx, ebx
				jmp         L4
				L3 :
			mov         ebx, eax
				mov         ecx, dword ptr[esp + 18h]
				mov         edx, dword ptr[esp + 14h]
				mov         eax, dword ptr[esp + 10h]
				L5 :
				shr         ebx, 1
				rcr         ecx, 1
				shr         edx, 1
				rcr         eax, 1
				or ebx, ebx
				jne         L5
				div         ecx
				mov         esi, eax
				mul         dword ptr[esp + 1Ch]
				mov         ecx, eax
				mov         eax, dword ptr[esp + 18h]
				mul         esi
				add         edx, ecx
				jb          L6
				cmp         edx, dword ptr[esp + 14h]
				ja          L6
				jb          L7
				cmp         eax, dword ptr[esp + 10h]
				jbe         L7
				L6 :
			dec         esi
				L7 :
			xor         edx, edx
				mov         eax, esi
				L4 :
			dec         edi
				jne         L8
				neg         edx
				neg         eax
				sbb         edx, 0
				L8 :
				pop         ebx
				pop         esi
				pop         edi
				ret         10h
		}
	}

	void __declspec(naked) _aulldiv() {
		__asm {
			push        ebx
			push        esi
			mov         eax, dword ptr[esp + 18h]
			or eax, eax
			jne         L1
			mov         ecx, dword ptr[esp + 14h]
			mov         eax, dword ptr[esp + 10h]
			xor edx, edx
			div         ecx
			mov         ebx, eax
			mov         eax, dword ptr[esp + 0Ch]
			div         ecx
			mov         edx, ebx
			jmp         L2
			L1 :
			mov         ecx, eax
				mov         ebx, dword ptr[esp + 14h]
				mov         edx, dword ptr[esp + 10h]
				mov         eax, dword ptr[esp + 0Ch]
				L3 :
				shr         ecx, 1
				rcr         ebx, 1
				shr         edx, 1
				rcr         eax, 1
				or ecx, ecx
				jne         L3
				div         ebx
				mov         esi, eax
				mul         dword ptr[esp + 18h]
				mov         ecx, eax
				mov         eax, dword ptr[esp + 14h]
				mul         esi
				add         edx, ecx
				jb          L4
				cmp         edx, dword ptr[esp + 10h]
				ja          L4
				jb          L5
				cmp         eax, dword ptr[esp + 0Ch]
				jbe         L5
				L4 :
			dec         esi
				L5 :
			xor         edx, edx
				mov         eax, esi
				L2 :
			pop         esi
				pop         ebx
				ret         10h
		}
	}

	void __declspec(naked) _allrem() {
		__asm {
			push        ebx
			push        edi
			xor         edi, edi
			mov         eax, dword ptr[esp + 10h]
			or eax, eax
			jge         L1
			inc         edi
			mov         edx, dword ptr[esp + 0Ch]
			neg         eax
			neg         edx
			sbb         eax, 0
			mov         dword ptr[esp + 10h], eax
			mov         dword ptr[esp + 0Ch], edx
			L1 :
			mov         eax, dword ptr[esp + 18h]
				or eax, eax
				jge         L2
				mov         edx, dword ptr[esp + 14h]
				neg         eax
				neg         edx
				sbb         eax, 0
				mov         dword ptr[esp + 18h], eax
				mov         dword ptr[esp + 14h], edx
				L2 :
			or eax, eax
				jne         L3
				mov         ecx, dword ptr[esp + 14h]
				mov         eax, dword ptr[esp + 10h]
				xor edx, edx
				div         ecx
				mov         eax, dword ptr[esp + 0Ch]
				div         ecx
				mov         eax, edx
				xor         edx, edx
				dec         edi
				jns         L4
				jmp         L8
				L3 :
			mov         ebx, eax
				mov         ecx, dword ptr[esp + 14h]
				mov         edx, dword ptr[esp + 10h]
				mov         eax, dword ptr[esp + 0Ch]
				L5 :
				shr         ebx, 1
				rcr         ecx, 1
				shr         edx, 1
				rcr         eax, 1
				or ebx, ebx
				jne         L5
				div         ecx
				mov         ecx, eax
				mul         dword ptr[esp + 18h]
				xchg        eax, ecx
				mul         dword ptr[esp + 14h]
				add         edx, ecx
				jb          L6
				cmp         edx, dword ptr[esp + 10h]
				ja          L6
				jb          L7
				cmp         eax, dword ptr[esp + 0Ch]
				jbe         L7
				L6 :
			sub         eax, dword ptr[esp + 14h]
				sbb         edx, dword ptr[esp + 18h]
				L7 :
				sub         eax, dword ptr[esp + 0Ch]
				sbb         edx, dword ptr[esp + 10h]
				dec         edi
				jns         L8
				L4 :
			neg         edx
				neg         eax
				sbb         edx, 0
				L8 :
				pop         edi
				pop         ebx
				ret         10h
		}
	}

	void __declspec(naked) _aullrem() {
		__asm {
			push        ebx
			mov         eax, dword ptr[esp + 14h]
			or eax, eax
			jne         L1
			mov         ecx, dword ptr[esp + 10h]
			mov         eax, dword ptr[esp + 0Ch]
			xor edx, edx
			div         ecx
			mov         eax, dword ptr[esp + 8]
			div         ecx
			mov         eax, edx
			xor         edx, edx
			jmp         L2
			L1 :
			mov         ecx, eax
				mov         ebx, dword ptr[esp + 10h]
				mov         edx, dword ptr[esp + 0Ch]
				mov         eax, dword ptr[esp + 8]
				L3 :
				shr         ecx, 1
				rcr         ebx, 1
				shr         edx, 1
				rcr         eax, 1
				or ecx, ecx
				jne         L3
				div         ebx
				mov         ecx, eax
				mul         dword ptr[esp + 14h]
				xchg        eax, ecx
				mul         dword ptr[esp + 10h]
				add         edx, ecx
				jb          L4
				cmp         edx, dword ptr[esp + 0Ch]
				ja          L4
				jb          L5
				cmp         eax, dword ptr[esp + 8]
				jbe         L5
				L4 :
			sub         eax, dword ptr[esp + 10h]
				sbb         edx, dword ptr[esp + 14h]
				L5 :
				sub         eax, dword ptr[esp + 8]
				sbb         edx, dword ptr[esp + 0Ch]
				neg         edx
				neg         eax
				sbb         edx, 0
				L2 :
				pop         ebx
				ret         10h
		}
	}

	void __declspec(naked) _alldvrm() {
		__asm {
			push        edi
			push        esi
			push        ebp
			xor         edi, edi
			xor         ebp, ebp
			mov         eax, dword ptr[esp + 14h]
			or eax, eax
			jge         L1
			inc         edi
			inc         ebp
			mov         edx, dword ptr[esp + 10h]
			neg         eax
			neg         edx
			sbb         eax, 0
			mov         dword ptr[esp + 14h], eax
			mov         dword ptr[esp + 10h], edx
			L1 :
			mov         eax, dword ptr[esp + 1Ch]
				or eax, eax
				jge         L2
				inc         edi
				mov         edx, dword ptr[esp + 18h]
				neg         eax
				neg         edx
				sbb         eax, 0
				mov         dword ptr[esp + 1Ch], eax
				mov         dword ptr[esp + 18h], edx
				L2 :
			or eax, eax
				jne         L3
				mov         ecx, dword ptr[esp + 18h]
				mov         eax, dword ptr[esp + 14h]
				xor edx, edx
				div         ecx
				mov         ebx, eax
				mov         eax, dword ptr[esp + 10h]
				div         ecx
				mov         esi, eax
				mov         eax, ebx
				mul         dword ptr[esp + 18h]
				mov         ecx, eax
				mov         eax, esi
				mul         dword ptr[esp + 18h]
				add         edx, ecx
				jmp         L4
				L3 :
			mov         ebx, eax
				mov         ecx, dword ptr[esp + 18h]
				mov         edx, dword ptr[esp + 14h]
				mov         eax, dword ptr[esp + 10h]
				L5 :
				shr         ebx, 1
				rcr         ecx, 1
				shr         edx, 1
				rcr         eax, 1
				or ebx, ebx
				jne         L5
				div         ecx
				mov         esi, eax
				mul         dword ptr[esp + 1Ch]
				mov         ecx, eax
				mov         eax, dword ptr[esp + 18h]
				mul         esi
				add         edx, ecx
				jb          L6
				cmp         edx, dword ptr[esp + 14h]
				ja          L6
				jb          L7
				cmp         eax, dword ptr[esp + 10h]
				jbe         L7
				L6 :
			dec         esi
				sub         eax, dword ptr[esp + 18h]
				sbb         edx, dword ptr[esp + 1Ch]
				L7 :
				xor         ebx, ebx
				L4 :
			sub         eax, dword ptr[esp + 10h]
				sbb         edx, dword ptr[esp + 14h]
				dec         ebp
				jns         L9
				neg         edx
				neg         eax
				sbb         edx, 0
				L9:
			mov         ecx, edx
				mov         edx, ebx
				mov         ebx, ecx
				mov         ecx, eax
				mov         eax, esi
				dec         edi
				jne         L8
				neg         edx
				neg         eax
				sbb         edx, 0
				L8:
			pop         ebp
				pop         esi
				pop         edi
				ret         10h
		}
	}

	void __declspec(naked) _aulldvrm() {
		__asm {
			push        esi
			mov         eax, dword ptr[esp + 14h]
			or eax, eax
			jne         L1
			mov         ecx, dword ptr[esp + 10h]
			mov         eax, dword ptr[esp + 0Ch]
			xor edx, edx
			div         ecx
			mov         ebx, eax
			mov         eax, dword ptr[esp + 8]
			div         ecx
			mov         esi, eax
			mov         eax, ebx
			mul         dword ptr[esp + 10h]
			mov         ecx, eax
			mov         eax, esi
			mul         dword ptr[esp + 10h]
			add         edx, ecx
			jmp         L2
			L1 :
			mov         ecx, eax
				mov         ebx, dword ptr[esp + 10h]
				mov         edx, dword ptr[esp + 0Ch]
				mov         eax, dword ptr[esp + 8]
				L3 :
				shr         ecx, 1
				rcr         ebx, 1
				shr         edx, 1
				rcr         eax, 1
				or ecx, ecx
				jne         L3
				div         ebx
				mov         esi, eax
				mul         dword ptr[esp + 14h]
				mov         ecx, eax
				mov         eax, dword ptr[esp + 10h]
				mul         esi
				add         edx, ecx
				jb          L4
				cmp         edx, dword ptr[esp + 0Ch]
				ja          L4
				jb          L5
				cmp         eax, dword ptr[esp + 8]
				jbe         L5
				L4 :
			dec         esi
				sub         eax, dword ptr[esp + 10h]
				sbb         edx, dword ptr[esp + 14h]
				L5 :
				xor         ebx, ebx
				L2 :
			sub         eax, dword ptr[esp + 8]
				sbb         edx, dword ptr[esp + 0Ch]
				neg         edx
				neg         eax
				sbb         edx, 0
				mov         ecx, edx
				mov         edx, ebx
				mov         ebx, ecx
				mov         ecx, eax
				mov         eax, esi
				pop         esi
				ret         10h
		}
	}

	void __declspec(naked) _allshl() {
		__asm {
			cmp         cl, 40h
			jae         RETZERO
			cmp         cl, 20h
			jae         MORE32
			shld        edx, eax, cl
			shl         eax, cl
			ret
			MORE32 :
			mov         edx, eax
				xor         eax, eax
				and         cl, 1Fh
				shl         edx, cl
				ret
				RETZERO :
			xor         eax, eax
				xor         edx, edx
				ret
		}
	}

	void __declspec(naked) _allshr() {
		__asm {
			cmp         cl, 40h
			jae         RETZERO
			cmp         cl, 20h
			jae         MORE32
			shrd        eax, edx, cl
			sar         edx, cl
			ret
			MORE32 :
			mov         eax, edx
				xor         edx, edx
				and         cl, 1Fh
				sar         eax, cl
				ret
				RETZERO :
			xor         eax, eax
				xor         edx, edx
				ret
		}
	}

	void __declspec(naked) _aullshr() {
		__asm {
			cmp         cl, 40h
			jae         RETZERO
			cmp         cl, 20h
			jae         MORE32
			shrd        eax, edx, cl
			shr         edx, cl
			ret
			MORE32 :
			mov         eax, edx
				xor         edx, edx
				and         cl, 1Fh
				shr         eax, cl
				ret
				RETZERO :
			xor         eax, eax
				xor         edx, edx
				ret
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