/*
final_platform_layer.h - FPL -  v0.1 alpha
Open-Source Single-File Header-Library by Torsten Spaete

This library is designed to abstract the underlying platform to a very simple and easy to use api.
The only dependencies are built-in operatoring system libraries.

The main focus is game development, so the default settings will create a window and setup a opengl rendering context.

It is fully C-89 compatible but support C++ as well.
Some features can be fully compiled out as needed.

HOW TO USE:

- In one of your C or C++ translation units include this:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

- Provide the typical main entry point

int main(int argc, char **args) {
}

- Initialize the library and release it when you are done

fpl_Init(fpl_InitFlags_All);
...
fpl_Release();

USE CASE [OpenGL-Window]:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

int main(int argc, char **args) {
	if (fpl_Init(fpl_InitFlags_VideoOpenGL)) {
		while (fpl_WindowUpdate()) {
			glClear(GL_COLOR_BUFFER_BIT);
			fpl_WindowFlip();
		}
		fpl_Release();
	}
}

HOW TO COMPILE:

- Win32:

	* Link to kernel32.lib
	* Link to user32.lib
	* Link to shell32.lib
	* Link to opengl32.lib (Only needed if you use opengl)
	* When disabling C-Runtime you may need to specific other parameters as well

PREPROCESSOR OVERRIDES:

- FPL_API_AS_PRIVATE 0 or 1

- FPL_DEFAULT_WINDOW_WIDTH 1 or greater
- FPL_DEFAULT_WINDOW_HEIGHT 1 or greater

- FPL_ENABLE_ASSERTIONS 0 or 1
- FPL_ENABLE_C_ASSERT

- FPL_ENABLE_C_RUNTIME_LIBRARY 0 or 1

- FPL_ENABLE_WINDOW 0 or 1
- FPL_ENABLE_OPENGL 0 or 1

FEATURES:

[x] Creating a fixed or resizeable window
[x] Handling window, keyboard, mouse events
[ ] Enable/Disable window border
[ ] Change/Reset screen resolution
[ ] Polling gamepad informations
[ ] Clipboard string reading and writing

[x] Creating a 1.x opengl rendering context
[ ] Creating a 3.x + opengl rendering context

[ ] Audio playback using OS native libraries

[x] Memory allocation and de-allocation with custom alignment support
[x] Atomic operations
[x] Path functions
[x] File functions
[x] String conversion functions
[ ] Thread, mutex, condition handling

SUPPORTED ARCHITECTURES:

[x] x86
[x] x86_64

SUPPORTED COMPILERS:

[X] Compiles with MSVC
[ ] Compiles with GCC/G++
[ ] Compiles with Clang
[ ] Compiles with Intel C/C++ Compiler

SUPPORTED PLATFORMS:

[x] Win32
[ ] Linux
[ ] Unix/Posix
[ ] OSX (Not sure)

CODE-GUIDELINES:

- Must be compilable under C and C++ but never require any C++ features

- Everything must be prefixed with fpl_ or FPL_ but never with Fpl_ or fPl

- All internal types/structures/functions must be postfixed with _internal or _Internal or _INTERNAL

- Large names are prefered over short names

- No data hiding or indirection

- Keep it simple

- Do not allocate any memory, expect the user provides the memory always
	* In worst case its fine to pass NULL for the destination buffer and just return the required size

- Value by reference is passed by pointer only

- Returning short structs in functions are fine

- Dont rely on compilers, define/declare it explicitly always

- Preprocessor directives must be properly indented
	* Header and Implementation guards are treated as non-existing
	* There is no preprocessor directives allowed inside code blocks
	* Definitions must be formatted logically, first non directives than directives.

- Tab identations with windows \r\n linefeeds only

- All api functions must use function statement fpl_api

- Constants:
	* Simple defines in uppercase
	* Explicit type cast

	Good:
		#define FPL_MAX_EVENT_COUNT_INTERNAL (uint32_t)32768

	Bad:
		#define FPL_MAX_EVENT_COUNT_INTERNAL 32768
		const uint32_t FPL_MAX_EVENT_COUNT_INTERNAL = 32768;
		static const uint32_t fpl_MaxEventCount_Internal = 32768;

- Function pointers:
	* Prefix fpl_func_
	* Lowercase with underscore only
	* Use with care

	Good:
		typedef int (*fpl_func_my_function_doing_somthing)(char a, int b, void *c);

	Bad:
		typedef int (*fpl_my_function_ptr)(char a, int b, void *c);
		typedef int (*fpl_myFunctionPtr)(char a, int b, void *c);
		typedef int (*fpl_MyFunctionPtr)(char a, int b, void *c);
		typedef int (*MyFunctionPtr)(char a, int b, void *c);

- Function arguments
	* Readonly primitive types are defined as const
	* Any writeable type are passed by pointer
	* Short structs are allowed to pass by value

	Good:
		void fpl_MyFunctionDoingSomething(const int width, const int height);
	Bad:
		void fpl_MyFunctionDoingSomething(int width, int height);

- Function statements like inline, extern, etc. may never be used directly and must use its pendant already defined:
	fpl_inline		(Inline compiler hint)
	fpl_api			(Public API definition)
	fpl_internal	(For internals only)
	fpl_globalvar	(For global variables only)

- Primitive type definitions:
	Good:
		typedef uint32_t fpl_bool32;

	Bad:
		typedef uint32_t fplBool32;

- Use existing standard types from <stdint.h> always
	* Unless there is a type missing
	* When calculating with pointers use uintptr_t

	Good:
		typedef uint32_t fpl_bool32;

	Bad:
		typedef int32_t fpl_s32;
		typedef int fpl_myint;
		typedef int myint;

- Enum types:
	* Enum values must be set always
	* C-Style (Typedef enum, identifier is equal to the name)

	Good:
		typedef enum fpl_EventType {
			fpl_EventType_Window = 0,
			fpl_EventType_Mouse = 1,
		} fpl_EventType;

	Bad:
		typedef enum fpl_EventType {
			fpl_EventType_Window,
			fpl_EventType_Mouse,
		} fpl_EventType;

		typedef enum fpl_EventType {
			fpl_EventType_Window,
			fpl_EventType_Mouse,
		};

- Enum flags:
	* Enum values must be set always
	* Anonymous enums only
	* Typedef for the actual value as 32-bit unsigned integer

	Good:
		enum {
			fpl_Key_None = 0,
			fpl_Key_Escape = 27,
		};
		typedef uint32_t fpl_key;

	Bad:
		typedef enum fpl_key {
			fpl_Key_Escape = 27,
		} fpl_key;

		typedef enum fpl_key {
			fpl_Key_None,
			fpl_Key_Escape,
		} fpl_key;

		enum fpl_key {
			fpl_Key_Escape = 27,
		};

- Functions:
	* Pascal-case style
	* Void as nameless argument when there are no arguments

	Good:
		void fpl_MyEmptyFunction(void);
		uint32_t fpl_AddTwoValueTogetherU32(const uint32_t a, const uint32_t b);

	Bad:
		void fpl_MyEmptyFunction();
		void fpl_myemptyfunction();
		void fpl_my_empty_function();
		void my_empty_function();
		
- Structures:
	* Pascal-case style
	* C-Style (Typedef struct, both idents must be equal)

	Good:
		typedef struct fpl_MyStructForStuff {
		} fpl_MyStructForStuff;

	Bad:
		typedef struct fpl_MyStructForStuff {
		};
		typedef struct fpl_mystructforstuff {
		};
		typedef struct fpl_my_struct_for_stuff {
		};
		struct fpl_MyStructForStuff {
		};

LICENSE:

MIT License

Copyright (c) 2017 Torsten Spaete
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

VERSION HISTORY:

- v0.1 alpha:
	* Initial version

*/

#ifndef FPL_INCLUDE_H
#define FPL_INCLUDE_H

// ****************************************************************************
//
// Header
//
// ****************************************************************************

//
// Platform detection
//
#if defined(_WIN32)
#	define FPL_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FPL_PLATFORM_LINUX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FPL_PLATFORM_UNIX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// Architecture detection (x86, x64)
// See: https://sourceforge.net/p/predef/wiki/Architectures/
//
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
#	define FPL_ARCH_X64	
#elif defined(_WIN32) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#else
#	error "This architecture/compiler is not supported!"
#endif

//
// Build configuration and compilers
// See: http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
//
#if defined(_MSC_VER)
#	define FPL_COMPILER_MSVC
#	if defined(_DEBUG) || (!defined(NDEBUG))
#		define FPL_DEBUG
#	else
#		define FPL_RELEASE
#	endif
#elif defined(__llvm__)
#	define FPL_COMPILER_LLVM
#elif defined(__clang__)
#	define FPL_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define FPL_COMPILER_GCC
#elif defined(__MINGW32__)
#	define FPL_COMPILER_MINGW
#elif defined(__INTEL_COMPILER) || defined(__ICC)
#	define FPL_COMPILER_INTEL
#else
#	define FPL_COMPILER_UNKNOWN
#endif

//
// Default preprocessor overrides
//
#if !defined(FPL_DEFAULT_WINDOW_WIDTH)
#	define FPL_DEFAULT_WINDOW_WIDTH 800
#endif
#if !defined(FPL_DEFAULT_WINDOW_HEIGHT)
#	define FPL_DEFAULT_WINDOW_HEIGHT 600
#endif
#if !defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_DEBUG)
#		define FPL_ENABLE_ASSERTIONS 1
#	else
#		define FPL_ENABLE_ASSERTIONS 0
#	endif
#endif
#if !defined(FPL_ENABLE_OPENGL)
#	define FPL_ENABLE_OPENGL 1
#endif
#if !defined(FPL_ENABLE_WINDOW)
#	define FPL_ENABLE_WINDOW 1
#endif
#if !defined(FPL_ENABLE_C_RUNTIME_LIBRARY)
#	define FPL_ENABLE_C_RUNTIME_LIBRARY 1
#endif
#if FPL_ENABLE_ASSERTIONS
#	if !defined(FPL_ENABLE_C_ASSERT)
#		define FPL_ENABLE_C_ASSERT 1
#	endif
#endif

//
// Types
//
#include <stdint.h>
typedef uint32_t fpl_bool32;
enum {
	fpl_False = 0,
	fpl_True = 1,
};

// Use nullptr when C++/11 are available
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#	define FPL_NULLPTR nullptr
#else
#	define FPL_NULLPTR (void *)0
#endif

//
// Assertions
//
#if FPL_ENABLE_ASSERTIONS
#	if FPL_ENABLE_C_ASSERT && FPL_ENABLE_C_RUNTIME_LIBRARY
#		include <assert.h>
#		define FPL_ASSERT(exp) assert(exp)
#		define FPL_STATICASSERT(exp) static_assert(exp, "static_assert")
#	else
#		define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
#		define FPL_STATICASSERT_INTERNAL(exp, line) \
			int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#		define FPL_STATICASSERT(exp) \
			FPL_STATICASSERT_INTERNAL(exp, __LINE__)
#	endif // FPL_ENABLE_C_ASSERT && FPL_ENABLE_C_RUNTIME_LIBRARY
#else
#	define FPL_ASSERT(exp)
#	define FPL_STATICASSERT(exp)
#endif // FPL_ENABLE_ASSERTIONS

//
// Static/Inline/Extern/Internal
//
#define fpl_globalvar static
#define fpl_inline inline
#define fpl_internal static
#if FPL_API_AS_PRIVATE
#	define fpl_api static
#else
#	define fpl_api extern
#endif // FPL_API_AS_PRIVATE

//
// Macro functions
//
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FPL_OFFSETOF(type, field) ((type *)((void *)0)->field)

//
// Disabled C-Runtime-Support
//
// @TODO(final): This is a pain in the ass to get it compile without CRT, so we may drop this feature when it gets overhand.
#if	!FPL_ENABLE_C_RUNTIME_LIBRARY
#	if defined(FPL_COMPILER_MSVC)
#		ifdef __cplusplus
extern "C" {
#		endif // __cplusplus

	int _fltused;
#		ifdef __cplusplus
}
#		endif // __cplusplus
#	endif // defined(FPL_COMPILER_MSVC)
#endif // !FPL_ENABLE_C_RUNTIME_LIBRARY

//
// API
//
#ifdef __cplusplus
extern
"C" {
#endif
	/* Handle to a loaded library */
	typedef struct fpl_LibraryHandle {
		void *internalHandle;
		fpl_bool32 isValid;
	} fpl_LibraryHandle;

	/* Handle to a loaded/created file */
	typedef struct fpl_FileHandle {
		void *internalHandle;
		fpl_bool32 isValid;
	} fpl_FileHandle;

	enum {
		fpl_FilePositionMode_Beginning = 0,
		fpl_FilePositionMode_Current = 1,
		fpl_FilePositionMode_End = 2,
	};
	/* File position mode (Beginning, Current, End) */
	typedef uint32_t fpl_FilePositionMode;

	enum {
		fpl_FileEntryType_Unknown = 0,
		fpl_FileEntryType_File = 1,
		fpl_FileEntryType_Directory = 2,
	};
	/* File entry type (File, Directory, etc.) */
	typedef uint32_t fpl_fpl_FileEntryType;

	enum {
		fpl_FileAttributeFlags_None = 0,
		fpl_FileAttributeFlags_Normal = 1 << 0,
		fpl_FileAttributeFlags_ReadOnly = 1 << 1,
		fpl_FileAttributeFlags_Hidden = 1 << 2,
		fpl_FileAttributeFlags_Archive = 1 << 3,
		fpl_FileAttributeFlags_System = 1 << 4,
	};
	/* File attribute flags (Normal, Readonly, Hidden, etc.) */
	typedef uint32_t fpl_FileAttributeFlags;

#define FPL_MAX_FILEENTRY_PATH_LENGTH (uint32_t)1024

	/* Entry for storing current file informations (path, type, attributes, etc.) */
	typedef struct fpl_FileEntry {
		fpl_fpl_FileEntryType type;
		fpl_FileAttributeFlags attributes;
		char path[FPL_MAX_FILEENTRY_PATH_LENGTH];
		void *internalHandle;
	} fpl_FileEntry;

	enum {
		fpl_InitFlags_None = 0,
		fpl_InitFlags_Window = 1 << 0,
		fpl_InitFlags_VideoOpenGL = 1 << 1,
		fpl_InitFlags_All = fpl_InitFlags_Window | fpl_InitFlags_VideoOpenGL,
	};
	/* Initialization flags (Window, Video, All, etc.) */
	typedef uint32_t fpl_InitFlags;

	//
	// Atomics
	//
	/* Insert a atomic read fence. This may be just a compiler optimization for some configurations only. */
	fpl_api void fpl_AtomicReadFence(void);
	/* Insert a atomic write fence. This may be just a compiler optimization for some configurations only. */
	fpl_api void fpl_AtomicWriteFence(void);
	/* Insert a atomic read/write fence. This may be just a compiler optimization for some configurations only. */
	fpl_api void fpl_AtomicReadWriteFence(void);
	/* Replace a 32-bit unsigned integer with the given value atomically. Returns the target before the replacement. */
	fpl_api uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
	/* Replace a 64-bit unsigned integer with the given value atomically. Returns the target before the replacement. */
	fpl_api uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
	/* Adds a 32-bit unsigned integer atomatically. Returns the value before the addition. */
	fpl_api uint32_t fpl_AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
	/* Adds a 64-bit unsigned integer atomatically. Returns the value before the addition. */
	fpl_api uint64_t fpl_AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
	/* Compares a 32-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange. */
	fpl_api uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand);
	/* Compares a 64-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange. */
	fpl_api uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand);

	//
	// Core
	//
	/* Initialize the platform layer. */
	fpl_api fpl_bool32 fpl_Init(const fpl_InitFlags initFlags);
	/* Releases the platform layer and resets all structures to zero. */
	fpl_api void fpl_Release(void);
	/* Loads a dynamic library and returns the loaded handle for it. */
	fpl_api fpl_LibraryHandle fpl_LoadLibrary(const char *libraryFilePath);
	/* Returns the dynamic library procedure address for the given procedure name. */
	fpl_api void *fpl_GetLibraryProc(const fpl_LibraryHandle *handle, const char *name);
	/* Releases the loaded library and resets the handle to zero. */
	fpl_api void fpl_ReleaseLibrary(fpl_LibraryHandle *handle);

	//
	// Console
	//
	/* Writes the given text to the default console output */
	fpl_api void fpl_ConsoleOut(const char *text);
	/* Writes the given formatted text to the default console output */
	fpl_api void fpl_ConsoleFormatOut(const char *format, ...);
	/* Writes the given text to the console error output */
	fpl_api void fpl_ConsoleError(const char *text);
	/* Writes the given formatted text to the console error output */
	fpl_api void fpl_ConsoleFormatError(const char *format, ...);

	//
	// Memory
	//
	/* Resets the given memory pointer by the given size to zero. */
	fpl_api void fpl_ClearMemory(void *mem, const size_t size);
	/* Allocates memory from the operating system by the given size. */
	fpl_api void *fpl_AllocateMemory(const size_t size);
	/* Releases the memory allocated from the operating system. */
	fpl_api void fpl_FreeMemory(void *ptr);
	/* Allocates aligned memory from the operating system by the given alignment. */
	fpl_api void *fpl_AllocateAlignedMemory(const size_t size, const size_t alignment);
	/* Releases aligned memory from the operating system. */
	fpl_api void fpl_FreeAlignedMemory(void *ptr);

	//
	// Timing
	//
	/* Returns the current system clock in seconds with the highest precision. */
	fpl_api double fpl_GetHighResolutionTimeInSeconds(void);

	//
	// Strings
	//
	/* Returns the number of characters of the given 8-bit Ansi string. Null terminator is not included. */
	fpl_api uint32_t fpl_GetAnsiStringLength(const char *str);
	/* Returns the number of characters of the given 16-bit Wide string. Null terminator is not included. */
	fpl_api uint32_t fpl_GetWideStringLength(const wchar_t *str);
	/* Copies the given 8-bit Ansi string into a destination Ansi string. Does not allocate any memory. */
	fpl_api void fpl_CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
	/* Copies the given 16-bit Wide string into a destination Wide string. Does not allocate any memory. */
	fpl_api void fpl_CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
	/* Converts the given 16-bit Wide string in a 8-bit Ansi string. Does not allocate any memory. */
	fpl_api void fpl_WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
	/* Converts the given 16-bit Wide string in a 8-bit UTF-8 string. Does not allocate any memory. */
	fpl_api void fpl_WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
	/* Converts the given 8-bit Ansi string in a 16-bit Wide string. Does not allocate any memory. */
	fpl_api void fpl_AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
	/* Converts the given 8-bit UTF-8 string in a 16-bit Wide string. Does not allocate any memory. */
	fpl_api void fpl_UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);

	//
	// Files / Directories
	//
	/* Opens a binary file for reading and returns the handle of it. */
	fpl_api fpl_FileHandle fpl_OpenBinaryFile(const char *filePath);
	/* Creates a binary file and returns the handle of it. */
	fpl_api fpl_FileHandle fpl_CreateBinaryFile(const char *filePath);
	/* Reads a block from the given file handle and returns the number of bytes readed. Operation is limited to a 2 GB byte boundary. */
	fpl_api uint32_t fpl_ReadFileBlock32(fpl_FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
	/* Writes a block to the given file handle and returns the number of bytes written. Operation is limited to a 2 GB byte boundary. */
	fpl_api uint32_t fpl_WriteFileBlock32(fpl_FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
	/* Sets the current file position by the given position, depending on the mode its absolute or relative. Position is limited to a 2 GB byte boundary. */
	fpl_api void fpl_SetFilePosition32(fpl_FileHandle *fileHandle, const uint32_t position, const fpl_FilePositionMode mode);
	/* Returns the current file position. Position is limited to a 2 GB byte boundary. */
	fpl_api uint32_t fpl_GetFilePosition32(fpl_FileHandle *fileHandle);
	/* Closes the given file handle and resets the handle to zero. */
	fpl_api void fpl_CloseFile(fpl_FileHandle *fileHandle);

	// @TODO(final): Add 64-bit file operations as well!

	/* Returns the 32-bit file size in bytes for the given file. Its limited to files < 2 GB. */
	fpl_api uint32_t fpl_GetFileSize32(const char *filePath);
	/* Returns true when the given file path physically exists. */
	fpl_api fpl_bool32 fpl_FileExists(const char *filePath);
	/* Copies the given source file to the target path and returns truwe when copy was successful. Target path must include the full path to the file. When overwrite is set, the target file path will always be overwritten. */
	fpl_api fpl_bool32 fpl_CopyFile(const char *sourceFilePath, const char *targetFilePath, const fpl_bool32 overwrite);
	/* Deletes the given file without confirmation and returns true when the deletion was successful. */
	fpl_api fpl_bool32 fpl_DeleteFile(const char *filePath);

	/* Returns true when the given directory path physically exists. */
	fpl_api fpl_bool32 fpl_DirectoryExists(const char *path);
	/* Deletes the given directory without confirmation and returns true when the deletion was successful. When recursive is set, all files and folders in sub-directories will be deleted as well. */
	fpl_api fpl_bool32 fpl_RemoveEmptyDirectory(const char *path);
	/* Iterates through files / directories in the given directory. The path must contain the filter as well. Returns true when there was a first entry found. */
	fpl_api fpl_bool32 fpl_ListFilesBegin(const char *pathAndFilter, fpl_FileEntry *firstEntry);
	/* Get next file entry from iterating through files / directories. Returns false when there is no entry found. */
	fpl_api fpl_bool32 fpl_ListFilesNext(fpl_FileEntry *nextEntry);
	/* Releases opened resources from iterating through files / directories. */
	fpl_api void fpl_ListFilesEnd(fpl_FileEntry *lastEntry);

	//
	// Directories and Paths
	//
	/* Returns the full path to this executable, including the executable file name. */
	fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
	/* Returns the full path to your home directory. */
	fpl_api void fpl_GetHomePath(char *destPath, const uint32_t maxDestLen);
	/* Returns the path from the given source path. */
	fpl_api char *fpl_ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
	/* Returns the file extension from the given source path. */
	fpl_api char *fpl_ExtractFileExtension(const char *sourcePath);
	/* Returns the file name including the file extension from the given source path. */
	fpl_api char *fpl_ExtractFileName(const char *sourcePath);
	/* Changes the file extension on the given source path and writes the result into the destination path. Returns the pointer of the destination path. */
	fpl_api char *fpl_ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *filePath, const char *newFileExtension);
	/* Combines all included path by the systems path separator. Returns the pointer of the destination path. */
	fpl_api char *fpl_CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);

#if FPL_ENABLE_WINDOW
	// @NOTE(final): Based on MS Virtual-Key-Codes, mostly directly mappable to ASCII
	enum {
		fpl_Key_None = 0,

		// 0x07: Undefined

		fpl_Key_Backspace = 0x08,
		fpl_Key_Tab = 0x09,

		// 0x0A-0x0B: Reserved

		fpl_Key_Clear = 0x0C,
		fpl_Key_Enter = 0x0D,

		// 0x0E-0x0F: Undefined

		fpl_Key_Shift = 0x10,
		fpl_Key_Control = 0x11,
		fpl_Key_Alt = 0x12,
		fpl_Key_Pause = 0x13,
		fpl_Key_CapsLock = 0x14,

		// 0x15: IME-Keys
		// 0x16: Undefined
		// 0x17-0x19 IME-Keys
		// 0x1A: Undefined

		fpl_Key_Escape = 0x1B,

		// 0x1C - 0x1F: IME-Keys

		fpl_Key_Space = 0x20,
		fpl_Key_PageUp = 0x21,
		fpl_Key_PageDown = 0x22,
		fpl_Key_End = 0x23,
		fpl_Key_Home = 0x24,
		fpl_Key_Left = 0x25,
		fpl_Key_Up = 0x26,
		fpl_Key_Right = 0x27,
		fpl_Key_Down = 0x28,
		fpl_Key_Select = 0x29,
		fpl_Key_Print = 0x2A,
		fpl_Key_Execute = 0x2B,
		fpl_Key_Snapshot = 0x2C,
		fpl_Key_Insert = 0x2D,
		fpl_Key_Delete = 0x2E,
		fpl_Key_Help = 0x2F,

		fpl_Key_0 = 0x30,
		fpl_Key_1 = 0x31,
		fpl_Key_2 = 0x32,
		fpl_Key_3 = 0x33,
		fpl_Key_4 = 0x34,
		fpl_Key_5 = 0x35,
		fpl_Key_6 = 0x36,
		fpl_Key_7 = 0x37,
		fpl_Key_8 = 0x38,
		fpl_Key_9 = 0x39,

		// 0x3A-0x40: Undefined

		fpl_Key_A = 0x41,
		fpl_Key_B = 0x42,
		fpl_Key_C = 0x43,
		fpl_Key_D = 0x44,
		fpl_Key_E = 0x45,
		fpl_Key_F = 0x46,
		fpl_Key_G = 0x47,
		fpl_Key_H = 0x48,
		fpl_Key_I = 0x49,
		fpl_Key_J = 0x4A,
		fpl_Key_K = 0x4B,
		fpl_Key_L = 0x4C,
		fpl_Key_M = 0x4D,
		fpl_Key_N = 0x4E,
		fpl_Key_O = 0x4F,
		fpl_Key_P = 0x50,
		fpl_Key_Q = 0x51,
		fpl_Key_R = 0x52,
		fpl_Key_S = 0x53,
		fpl_Key_T = 0x54,
		fpl_Key_U = 0x55,
		fpl_Key_V = 0x56,
		fpl_Key_W = 0x57,
		fpl_Key_X = 0x58,
		fpl_Key_Y = 0x59,
		fpl_Key_Z = 0x5A,

		fpl_Key_LeftWin = 0x5B,
		fpl_Key_RightWin = 0x5C,
		fpl_Key_Apps = 0x5D,

		// 0x5E: Reserved

		fpl_Key_Sleep = 0x5F,
		fpl_Key_NumPad0 = 0x60,
		fpl_Key_NumPad1 = 0x61,
		fpl_Key_NumPad2 = 0x62,
		fpl_Key_NumPad3 = 0x63,
		fpl_Key_NumPad4 = 0x64,
		fpl_Key_NumPad5 = 0x65,
		fpl_Key_NumPad6 = 0x66,
		fpl_Key_NumPad7 = 0x67,
		fpl_Key_NumPad8 = 0x68,
		fpl_Key_NumPad9 = 0x69,
		fpl_Key_Multiply = 0x6A,
		fpl_Key_Add = 0x6B,
		fpl_Key_Separator = 0x6C,
		fpl_Key_Substract = 0x6D,
		fpl_Key_Decimal = 0x6E,
		fpl_Key_Divide = 0x6F,
		fpl_Key_F1 = 0x70,
		fpl_Key_F2 = 0x71,
		fpl_Key_F3 = 0x72,
		fpl_Key_F4 = 0x73,
		fpl_Key_F5 = 0x74,
		fpl_Key_F6 = 0x75,
		fpl_Key_F7 = 0x76,
		fpl_Key_F8 = 0x77,
		fpl_Key_F9 = 0x78,
		fpl_Key_F10 = 0x79,
		fpl_Key_F11 = 0x7A,
		fpl_Key_F12 = 0x7B,
		fpl_Key_F13 = 0x7C,
		fpl_Key_F14 = 0x7D,
		fpl_Key_F15 = 0x7E,
		fpl_Key_F16 = 0x7F,
		fpl_Key_F17 = 0x80,
		fpl_Key_F18 = 0x81,
		fpl_Key_F19 = 0x82,
		fpl_Key_F20 = 0x83,
		fpl_Key_F21 = 0x84,
		fpl_Key_F22 = 0x85,
		fpl_Key_F23 = 0x86,
		fpl_Key_F24 = 0x87,

		// 0x88-8F: Unassigned

		fpl_Key_NumLock = 0x90,
		fpl_Key_Scroll = 0x91,

		// 0x92-9x96: OEM specific
		// 0x97-0x9F: Unassigned

		fpl_Key_LeftShift = 0xA0,
		fpl_Key_RightShift = 0xA1,
		fpl_Key_LeftControl = 0xA2,
		fpl_Key_RightControl = 0xA3,
		fpl_Key_LeftAlt = 0xA4,
		fpl_Key_RightAlt = 0xA5,

		// 0xA6-0xFE: Dont care
	};

	/* Mapped key */
	typedef uint64_t fpl_Key;

	/* Window configuration (Title, Size, etc.) */
	typedef struct fpl_WindowConfiguration {
		char windowTitle[128];
		uint32_t windowWidth;
		uint32_t windowHeight;
	} fpl_WindowConfiguration;

	/* Window event type (Resized, PositionChanged, etc.) */
	typedef enum fpl_WindowEventType {
		fpl_WindowEventType_Resized = 1,
	} fpl_WindowEventType;

	/* Window event (Size, Position, etc.) */
	typedef struct fpl_WindowEvent {
		fpl_WindowEventType type;
		uint32_t width;
		uint32_t height;
	} fpl_WindowEvent;

	/* Keyboard event type (KeyDown, KeyUp, Char, ...) */
	typedef enum fpl_KeyboardEventType {
		fpl_KeyboardEventType_KeyDown = 1,
		fpl_KeyboardEventType_KeyUp = 2,
		fpl_KeyboardEventType_Char = 3,
	} fpl_KeyboardEventType;

	enum {
		fpl_KeyboardModifierFlags_None = 0,
		fpl_KeyboardModifierFlags_Alt = 1 << 0,
		fpl_KeyboardModifierFlags_Ctrl = 1 << 1,
		fpl_KeyboardModifierFlags_Shift = 1 << 2,
		fpl_KeyboardModifierFlags_Super = 1 << 3,
	};
	/* Keyboard modifier flags (Alt, Ctrl, ...) */
	typedef uint32_t fpl_KeyboardModifierFlags;

	/* Keyboard event (Type, Keycode, Mapped key, etc.) */
	typedef struct fpl_KeyboardEvent {
		fpl_KeyboardEventType type;
		uint64_t keyCode;
		fpl_Key mappedKey;
		fpl_KeyboardModifierFlags modifiers;
	} fpl_KeyboardEvent;

	/* Mouse event type (Move, ButtonDown, ...) */
	typedef enum fpl_MouseEventType {
		fpl_MouseEventType_Move = 1,
		fpl_MouseEventType_ButtonDown = 2,
		fpl_MouseEventType_ButtonUp = 3,
		fpl_MouseEventType_Wheel = 4,
	} fpl_MouseEventType;

	/* Mouse button type (Left, Right, ...) */
	typedef enum fpl_MouseButtonType {
		fpl_MouseButtonType_None = -1,
		fpl_MouseButtonType_Left = 0,
		fpl_MouseButtonType_Right = 1,
		fpl_MouseButtonType_Middle = 2,
	} fpl_MouseButtonType;

	/* Mouse event (Type, Button, Position, etc.) */
	typedef struct fpl_MouseEvent {
		fpl_MouseEventType type;
		fpl_MouseButtonType mouseButton;
		int32_t mouseX;
		int32_t mouseY;
		float wheelDelta;
		int32_t _padding;
	} fpl_MouseEvent;

	/* Event type (Window, Keyboard, Mouse, ...) */
	typedef enum fpl_EventType {
		fpl_EventType_Window = 1,
		fpl_EventType_Keyboard = 2,
		fpl_EventType_Mouse = 3,
	} fpl_EventType;

	/* Event (Type, Window, Keyboard, Mouse, etc.) */
	typedef struct fpl_Event {
		fpl_EventType type;
		union {
			fpl_WindowEvent window;
			fpl_KeyboardEvent keyboard;
			fpl_MouseEvent mouse;
		};
	} fpl_Event;

	/* Window size in screen coordinates */
	typedef struct fpl_WindowSize {
		uint32_t width;
		uint32_t height;
	} fpl_WindowSize;

	/* Window position in screen coordinates */
	typedef struct fpl_WindowPosition {
		int32_t left;
		int32_t top;
	} fpl_WindowPosition;

	//
	// Window
	//
	/* Returns true when the window is active. */
	fpl_api fpl_bool32 fpl_IsWindowRunning(void);
	/* Processes the message queue of the window. */
	fpl_api fpl_bool32 fpl_WindowUpdate(void);
	/* Forces the window to redraw or swap the back/front buffer. */
	fpl_api void fpl_WindowFlip(void);
	/* Enables or disables the window cursor. */
	fpl_api void fpl_SetWindowCursorEnabled(const fpl_bool32 value);
	/* Returns the inner window area. */
	fpl_api fpl_WindowSize fpl_GetWindowArea(void);
	/* Resizes the window to fit the inner area to the given size. */
	fpl_api void fpl_SetWindowArea(const uint32_t width, const uint32_t height);
	/* Returns true when the window is resizable. */
	fpl_api fpl_bool32 fpl_IsWindowResizable(void);
	/* Enables or disables the ability to resize the window. */
	fpl_api void fpl_SetWindowResizeable(const fpl_bool32 value);
	/* Returns the absolute window position. */
	fpl_api fpl_WindowPosition fpl_GetWindowPosition(void);
	/* Sets the window absolut position to the given coordinates. */
	fpl_api void fpl_SetWindowPosition(const int32_t left, const int32_t top);

	//
	// Events
	//
	/* Gets and removes the top event from the internal queue and fills out the "event" argument. Returns false when there are no events left, otherwise true. */
	fpl_api fpl_bool32 fpl_PollEvent(fpl_Event *event);
#endif

#if defined(FPL_PLATFORM_WINDOWS)
	// @NOTE(final): Required for access "main" from the actual win32 entry point
	fpl_api int main(int argc, char **args);
#endif

#ifdef __cplusplus
}
#endif

#endif // FPL_INCLUDE_H

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#	define FPL_IMPLEMENTED

//
// Non-Platform includes
//
#include <stdarg.h>  // va_start, va_end, va_list, va_arg
#include <stdio.h> // fprintf, vfprintf

//
// Platform constants
//
#	if defined(FPL_PLATFORM_WINDOWS)
#		define FPL_PATH_SEPARATOR '\\'
#		define FPL_FILE_EXT_SEPARATOR '.'
#	else
#		define FPL_PATH_SEPARATOR '/'
#		define FPL_FILE_EXT_SEPARATOR '.'
#	endif

// Internal types and functions
#define FPL_CLEARMEMORY_INTERNAL(type, mem, size, shift, mask) do { \
	type *dataBlock = (type *)mem; \
	type *dataBlockEnd = (type *)dataBlock + (size >> shift); \
	uint8_t *data8 = (uint8_t *)dataBlockEnd; \
	uint8_t *data8End = data8 + (size & mask); \
	while (dataBlock != dataBlockEnd) { \
		*dataBlock++ = 0; \
	} \
	while (data8 != data8End) { \
		*data8++ = 0; \
	} \
} while (0)

#define FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL (uint32_t)256
typedef struct fpl_ErrorState_Internal {
	char lastError[FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
} fpl_ErrorState_Internal;

#if FPL_ENABLE_WINDOW
#	define FPL_MAX_EVENT_COUNT_INTERNAL (uint32_t)32768
typedef struct fpl_EventQueue_Internal {
	fpl_Event events[FPL_MAX_EVENT_COUNT_INTERNAL];
	volatile uint32_t pollIndex;
	volatile uint32_t pushCount;
} fpl_EventQueue_Internal;

fpl_globalvar fpl_EventQueue_Internal *fpl_GlobalEventQueue_Internal = 0;

fpl_internal void fpl_PushEvent_Internal(fpl_Event *event) {
	fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
	FPL_ASSERT(eventQueue != NULL);
	if (eventQueue->pushCount < FPL_MAX_EVENT_COUNT_INTERNAL) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->pushCount, 1);
		FPL_ASSERT(eventIndex < FPL_MAX_EVENT_COUNT_INTERNAL);
		eventQueue->events[eventIndex] = *event;
	}
}
#endif

#define FPL_CLEARSTRUCT_INTERNAL(value) \
	fpl_ClearMemory(value, sizeof(*value))

//
// All Public String
//
fpl_api uint32_t fpl_GetAnsiStringLength(const char *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_api uint32_t fpl_GetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_api void fpl_CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	*dest = 0;
}

fpl_api void fpl_CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	dest[sourceLen] = 0;
}

//
// All Public Memory
//
fpl_api void *fpl_AllocateAlignedMemory(const size_t size, const size_t alignment) {
	FPL_ASSERT(size > 0);
	FPL_ASSERT((alignment > 0) && !(alignment & (alignment - 1)));

	// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
	size_t newSize = sizeof(void *) + size + (alignment << 1);
	void *basePtr = fpl_AllocateMemory(newSize);
	fpl_ClearMemory(basePtr, newSize);

	// The resulting address starts after the stored base pointer
	void *alignedPtr = (void *)((uintptr_t)basePtr + sizeof(void *));

	// Move the resulting address to a aligned one when not aligned
	// @TODO(final): This seems to do nothing on a typical i7 machine, regardless of the target - test with other architectures!
	uintptr_t mask = alignment - 1;
	if ((alignment > 1) && (((uintptr_t)alignedPtr & mask) != 0)) {
		*(uintptr_t *)alignedPtr += ((uintptr_t)alignment - ((uintptr_t)alignedPtr & mask));
	}

	// Write the base pointer before the alignment pointer
	*(void **)((void *)((uintptr_t)alignedPtr - sizeof(void *))) = basePtr;

	return(alignedPtr);
}

fpl_api void fpl_FreeAlignedMemory(void *ptr) {
	FPL_ASSERT(ptr != FPL_NULLPTR);

	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
	fpl_FreeMemory(basePtr);
}

fpl_api void fpl_ClearMemory(void *mem, const size_t size) {
	if (size % 8 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint64_t, mem, size, 3, 0x00000007);
	} else if (size % 4 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint32_t, mem, size, 2, 0x00000003);
	} else if (size % 2 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint16_t, mem, size, 1, 0x00000001);
	} else {
		uint8_t *data8 = (uint8_t *)mem;
		uint8_t *data8End = data8 + size;
		while (data8 != data8End) {
			*data8++ = 0;
		}
	}
}

//
// All Public Window
//
#if FPL_ENABLE_WINDOW
fpl_api fpl_bool32 fpl_PollEvent(fpl_Event *event) {
	fpl_bool32 result = fpl_False;
	fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
	FPL_ASSERT(eventQueue != NULL);
	if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->pollIndex, 1);
		*event = eventQueue->events[eventIndex];
		result = fpl_True;
	} else if (fpl_GlobalEventQueue_Internal->pushCount > 0) {
		fpl_AtomicExchangeU32(&eventQueue->pollIndex, 0);
		fpl_AtomicExchangeU32(&eventQueue->pushCount, 0);
	}
	return result;
}
#endif

//
// All Public Path, Directories
//
fpl_api char *fpl_ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
	char *result = (char *)FPL_NULLPTR;
	if (sourcePath) {
		int copyLen = 0;
		char *chPtr = (char *)sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				copyLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		if (copyLen) {
			fpl_CopyAnsiString(sourcePath, copyLen, destPath, maxDestLen);
		}
	}
	return(result);
}

fpl_api char *fpl_ExtractFileExtension(const char *sourcePath) {
	char *result = (char *)FPL_NULLPTR;
	if (sourcePath != FPL_NULLPTR) {
		char *filename = fpl_ExtractFileName(sourcePath);
		if (filename) {
			char *chPtr = filename;
			char *firstSeparatorPtr = (char *)FPL_NULLPTR;
			while (*chPtr) {
				if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
					firstSeparatorPtr = chPtr;
					break;
				}
				++chPtr;
			}
			if (firstSeparatorPtr != FPL_NULLPTR) {
				result = firstSeparatorPtr;
			}
		}
	}
	return(result);
}

fpl_api char *fpl_ExtractFileName(const char *sourcePath) {
	char *result = (char *)FPL_NULLPTR;
	if (sourcePath) {
		result = (char *)sourcePath;
		char *chPtr = (char *)sourcePath;
		char *lastPtr = (char *)FPL_NULLPTR;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				lastPtr = chPtr;
			}
			++chPtr;
		}
		if (lastPtr != FPL_NULLPTR) {
			result = lastPtr + 1;
		}
	}
	return(result);
}

fpl_api char *fpl_ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *filePath, const char *newFileExtension) {
	char *result = (char *)FPL_NULLPTR;
	if (filePath != FPL_NULLPTR) {
		// Find last path
		char *chPtr = (char *)filePath;
		char *lastPathSeparatorPtr = (char *)FPL_NULLPTR;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				lastPathSeparatorPtr = chPtr;
			}
			++chPtr;
		}

		// Find last ext separator
		if (lastPathSeparatorPtr != FPL_NULLPTR) {
			chPtr = lastPathSeparatorPtr + 1;
		} else {
			chPtr = (char *)filePath;
		}
		char *lastExtSeparatorPtr = (char *)FPL_NULLPTR;
		while (*chPtr) {
			if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
				lastExtSeparatorPtr = chPtr;
			}
			++chPtr;
		}

		uint32_t pathLen = fpl_GetAnsiStringLength(filePath);
		uint32_t copyLen;
		if (lastExtSeparatorPtr != FPL_NULLPTR) {
			copyLen = (uint32_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
		} else {
			copyLen = pathLen;
		}

		// Copy parts
		uint32_t extLen = fpl_GetAnsiStringLength(newFileExtension);
		fpl_CopyAnsiString(filePath, copyLen, destPath, maxDestLen);
		char *destExtPtr = destPath + copyLen;
		fpl_CopyAnsiString(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
		result = destPath;
	}
	return(result);
}

fpl_api char *fpl_CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
	uint32_t curDestPosition = 0;
	char *currentDestPtr = destPath;
	va_list vargs;
	va_start(vargs, pathCount);
	for (uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		char *path = va_arg(vargs, char *);
		uint32_t pathLen = fpl_GetAnsiStringLength(path);
		fpl_bool32 requireSeparator = pathIndex < (pathCount - 1);
		uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
		FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
		fpl_CopyAnsiString(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
		currentDestPtr += pathLen;
		if (requireSeparator) {
			*currentDestPtr++ = FPL_PATH_SEPARATOR;
		}
		curDestPosition += requiredPathLen;
	}
	*currentDestPtr = 0;
	va_end(vargs);
	return destPath;
}

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WINDOWS)
#	include <intrin.h>
	// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#	define NOMINMAX
#	include <windows.h> // Win32 api
#	include <windowsx.h> // macros for window messages
#	include <shlobj.h> // SHGetFolderPath
#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
#		include <gl\gl.h>
#	endif // FPL_ENABLE_WINDOW

#	define FPL_PATH_SEPARATOR '\\'
#	define FPL_FILE_EXT_SEPARATOR '.'

// Win32 internal functions
#	if defined(UNICODE)
typedef wchar_t fpl_win32_char_internal;
#	else
typedef char fpl_win32_char_internal;
#	endif // defined(UNICODE)

#	if FPL_ENABLE_WINDOW
typedef struct fpl_Win32WindowState_Internal {
	HWND windowHandle;
	fpl_win32_char_internal windowClass[256];
	HDC deviceContext;
	fpl_bool32 isCursorActive;
	HCURSOR defaultCursor;
	fpl_bool32 isRunning;
} fpl_Win32WindowState_Internal;
#	else
typedef void *fpl_Win32WindowState_Internal;
#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
typedef struct fpl_Win32OpenGLState_Internal {
	HGLRC renderingContext;
} fpl_Win32OpenGLState_Internal;
#	else
typedef void *fpl_Win32OpenGLState_Internal;
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

typedef struct fpl_Win32State_Internal {
	fpl_bool32 isInitialized;
	HINSTANCE appInstance;
	LARGE_INTEGER performanceFrequency;
	fpl_Win32WindowState_Internal window;
	fpl_Win32OpenGLState_Internal opengl;
} fpl_Win32State_Internal;

fpl_globalvar fpl_Win32State_Internal fpl_GlobalWin32State_Internal = { 0 };

// @TODO(final): Dont overwrite defines like that, just have one path for unicode and one for ansi
#	undef WNDCLASSEX
#	undef RegisterClassEx
#	undef UnregisterClass
#	undef CreateWindowEx
#	undef DefWindowProc
#	undef GetWindowLongPtr
#	undef SetWindowLongPtr
#	undef DispatchMessage
#	if defined(UNICODE)
#		define WIN32_CLASSNAME L"FPLWindowClassW"
#		define WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
#		define WNDCLASSEX WNDCLASSEXW
#		define RegisterClassEx RegisterClassExW
#		define UnregisterClass UnregisterClassW
#		define CreateWindowEx CreateWindowExW
#		define DefWindowProc DefWindowProcW
#		define GetWindowLongPtr GetWindowLongPtrW
#		define SetWindowLongPtr SetWindowLongPtrW
#		define GetWindowLong GetWindowLongW
#		define SetWindowLong SetWindowLongW
#		define PeekMessage PeekMessageW
#		define DispatchMessage DispatchMessageW
#		define fpl_Win32StringCopy fpl_CopyWideString
#		define fpl_Win32GetStringLength fpl_GetWideStringLength
#		define MapVirtualKey MapVirtualKeyW
#	else
#		define WIN32_CLASSNAME "FPLWindowClassA"
#		define WIN32_UNNAMED_WINDOW "Unnamed FPL Ansi Window"
#		define WNDCLASSEX WNDCLASSEXA
#		define RegisterClassEx RegisterClassExA
#		define UnregisterClass UnregisterClassA
#		define CreateWindowEx CreateWindowExA
#		define DefWindowProc DefWindowProcA
#		define GetWindowLongPtr GetWindowLongPtrA
#		define SetWindowLongPtr SetWindowLongPtrA
#		define GetWindowLong GetWindowLongA
#		define SetWindowLong SetWindowLongA
#		define PeekMessage PeekMessageA
#		define DispatchMessage DispatchMessageA
#		define fpl_Win32StringCopy fpl_CopyAnsiString
#		define fpl_Win32GetStringLength fpl_GetAnsiStringLength
#		define MapVirtualKey MapVirtualKeyA
#	endif // defined(UNICODE)

//
// Win32 Public Intrinsics
//
#	if defined(FPL_COMPILER_MSVC)
fpl_api void fpl_AtomicReadFence(void) {
	_ReadBarrier();
}
fpl_api void fpl_AtomicWriteFence(void) {
	_WriteBarrier();
}
fpl_api void fpl_AtomicReadWriteFence(void) {
	_ReadWriteBarrier();
}
fpl_api uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
	FPL_ASSERT(target != FPL_NULLPTR);
	uint32_t result = _InterlockedExchange((volatile long *)target, value);
	return (result);
}
fpl_api uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	FPL_ASSERT(target != FPL_NULLPTR);
	uint64_t result = InterlockedExchange64((volatile long long *)target, value);
	return (result);
}
fpl_api uint32_t fpl_AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
	FPL_ASSERT(value != FPL_NULLPTR);
	uint32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
	return (result);
}
fpl_api uint64_t fpl_AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
	FPL_ASSERT(value != FPL_NULLPTR);
	uint64_t result = InterlockedExchangeAdd64((volatile long long *)value, addend);
	return (result);
}
fpl_api uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand) {
	FPL_ASSERT(dest != FPL_NULLPTR);
	uint32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
	return (result);
}
fpl_api uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand) {
	FPL_ASSERT(dest != FPL_NULLPTR);
	uint64_t result = InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
	return (result);
}
#	endif // defined(FPL_COMPILER_MSVC)

// Win32 public console
#if FPL_ENABLE_C_RUNTIME_LIBRARY
fpl_api void fpl_ConsoleOut(const char *text) {
	FPL_ASSERT(text != FPL_NULLPTR);
	fprintf(stdout, text);
}
fpl_api void fpl_ConsoleFormatOut(const char *format, ...) {
	FPL_ASSERT(format != FPL_NULLPTR);
	va_list vaList;
	va_start(vaList, format);
	vfprintf(stdout, format, vaList);
	va_end(vaList);
}
fpl_api void fpl_ConsoleError(const char *text) {
	FPL_ASSERT(text != FPL_NULLPTR);
	fprintf(stderr, text);
}
fpl_api void fpl_ConsoleFormatError(const char *format, ...) {
	FPL_ASSERT(format != FPL_NULLPTR);
	va_list vaList;
	va_start(vaList, format);
	vfprintf(stderr, format, vaList);
	va_end(vaList);
}
#else
fpl_api void fpl_ConsoleOut(const char *text) {
	// @IMPLEMENT(final): Console output for disabled CRT
}
fpl_api void fpl_ConsoleFormatOut(const char *format, ...) {
	// @IMPLEMENT(final): Formatted console output for disabled CRT
}
fpl_api void fpl_ConsoleError(const char *text) {
	// @IMPLEMENT(final): Console error output for disabled CRT
}
fpl_api void fpl_ConsoleFormatError(const char *format, ...) {
	// @IMPLEMENT(final): Formatted console error output for disabled CRT
}
#endif

//
// Win32 Public Memory
//
fpl_api void *fpl_AllocateMemory(const size_t size) {
	FPL_ASSERT(size > 0);
	void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return(result);
}

fpl_api void fpl_FreeMemory(void *ptr) {
	FPL_ASSERT(ptr != NULL);
	VirtualFree(ptr, 0, MEM_FREE);
}

//
// Win32 Public File
//
fpl_api fpl_FileHandle fpl_OpenBinaryFile(const char *filePath) {
	FPL_ASSERT(filePath != FPL_NULLPTR);
	fpl_FileHandle result = { 0 };
	HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (win32FileHandle != INVALID_HANDLE_VALUE) {
		result.isValid = fpl_True;
		result.internalHandle = (void *)win32FileHandle;
	}
	return(result);
}

fpl_api fpl_FileHandle fpl_CreateBinaryFile(const char *filePath) {
	FPL_ASSERT(filePath != FPL_NULLPTR);
	fpl_FileHandle result = { 0 };
	HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (win32FileHandle != INVALID_HANDLE_VALUE) {
		result.isValid = fpl_True;
		result.internalHandle = (void *)win32FileHandle;
	}
	return(result);
}

fpl_api uint32_t fpl_ReadFileBlock32(fpl_FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	FPL_ASSERT(fileHandle != FPL_NULLPTR);
	FPL_ASSERT(targetBuffer != FPL_NULLPTR);
	FPL_ASSERT(sizeToRead > 0);
	uint32_t result = 0;
	if (fileHandle->isValid) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD bytesRead = 0;
		if (ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, NULL) == TRUE) {
			result = bytesRead;
		}
	}
	return(result);
}

fpl_api uint32_t fpl_WriteFileBlock32(fpl_FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	FPL_ASSERT(fileHandle != FPL_NULLPTR);
	FPL_ASSERT(sourceBuffer != FPL_NULLPTR);
	FPL_ASSERT(sourceSize > 0);
	uint32_t result = 0;
	if (fileHandle->isValid) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD bytesWritten = 0;
		if (WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, NULL) == TRUE) {
			result = bytesWritten;
		}
	}
	return(result);
}

fpl_api void fpl_SetFilePosition32(fpl_FileHandle *fileHandle, const uint32_t position, const fpl_FilePositionMode mode) {
	FPL_ASSERT(fileHandle != FPL_NULLPTR);
	if (fileHandle->isValid) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD moveMethod = FILE_BEGIN;
		if (mode == fpl_FilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if (mode == fpl_FilePositionMode_End) {
			moveMethod = FILE_END;
		}
		SetFilePointer(win32FileHandle, (LONG)position, NULL, moveMethod);
	}
}

fpl_api uint32_t fpl_GetFilePosition32(fpl_FileHandle *fileHandle) {
	FPL_ASSERT(fileHandle != FPL_NULLPTR);
	uint32_t result = 0;
	if (fileHandle->isValid) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD filePosition = SetFilePointer(win32FileHandle, 0L, NULL, FILE_CURRENT);
		if (filePosition != INVALID_SET_FILE_POINTER) {
			result = filePosition;
		}
	}
	return(result);
}

fpl_api void fpl_CloseFile(fpl_FileHandle *fileHandle) {
	FPL_ASSERT(fileHandle != FPL_NULLPTR);
	if (fileHandle->isValid) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		CloseHandle(win32FileHandle);
	}
	FPL_CLEARSTRUCT_INTERNAL(fileHandle);
}

fpl_api uint32_t fpl_GetFileSize32(const char *filePath) {
	uint32_t result = 0;
	HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (win32FileHandle != INVALID_HANDLE_VALUE) {
		DWORD fileSize = GetFileSize(win32FileHandle, NULL);
		result = fileSize;
		CloseHandle(win32FileHandle);
	}
	return(result);
}
fpl_api fpl_bool32 fpl_FileExists(const char *filePath) {
	fpl_bool32 result = fpl_False;
	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(filePath, &findData);
	if (searchHandle  != INVALID_HANDLE_VALUE) {
		result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		FindClose(searchHandle);
	}
	return(result);
}
fpl_api fpl_bool32 fpl_CopyFile(const char *sourceFilePath, const char *targetFilePath, const fpl_bool32 overwrite) {
	fpl_bool32 result = CopyFileA(sourceFilePath, targetFilePath, !overwrite);
	return(result);
}
fpl_api fpl_bool32 fpl_DeleteFile(const char *filePath) {
	fpl_bool32 result = DeleteFileA(filePath);
	return(result);
}
fpl_api fpl_bool32 fpl_DirectoryExists(const char *path) {
	fpl_bool32 result = fpl_False;
	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(path, &findData);
	if (searchHandle != INVALID_HANDLE_VALUE) {
		result = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		CloseHandle(searchHandle);
	}
	return(result);
}
fpl_api fpl_bool32 fpl_RemoveEmptyDirectory(const char *path) {
	fpl_bool32 result = RemoveDirectoryA(path);
	return(result);
}
fpl_internal void fpl_Win32FillFileEntry(WIN32_FIND_DATAA *findData, fpl_FileEntry *entry) {
	fpl_CopyAnsiString(findData->cFileName, fpl_GetAnsiStringLength(findData->cFileName), entry->path, FPL_ARRAYCOUNT(entry->path));

	entry->type = fpl_FileEntryType_Unknown;
	if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		entry->type = fpl_FileEntryType_Directory;
	} else if (
		(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
		entry->type = fpl_FileEntryType_File;
	}

	entry->attributes = fpl_FileAttributeFlags_None;
	if (findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		entry->attributes = fpl_FileAttributeFlags_Normal;
	} else {
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			entry->attributes |= fpl_FileAttributeFlags_Hidden;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			entry->attributes |= fpl_FileAttributeFlags_ReadOnly;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			entry->attributes |= fpl_FileAttributeFlags_Archive;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			entry->attributes |= fpl_FileAttributeFlags_System;
		}
	}
}
fpl_api fpl_bool32 fpl_ListFilesBegin(const char *pathAndFilter, fpl_FileEntry *firstEntry) {
	FPL_ASSERT(pathAndFilter != FPL_NULLPTR);
	FPL_ASSERT(firstEntry != FPL_NULLPTR);
	fpl_bool32 result = fpl_False;
	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(pathAndFilter, &findData);
	FPL_CLEARSTRUCT_INTERNAL(firstEntry);
	if (searchHandle != INVALID_HANDLE_VALUE) {
		firstEntry->internalHandle = (void *)searchHandle;
		fpl_Win32FillFileEntry(&findData, firstEntry);
		result = fpl_True;
	}
	return(result);
}
fpl_api fpl_bool32 fpl_ListFilesNext(fpl_FileEntry *nextEntry) {
	FPL_ASSERT(nextEntry != FPL_NULLPTR);
	fpl_bool32 result = fpl_False;
	if (nextEntry->internalHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = (HANDLE)nextEntry->internalHandle;
		WIN32_FIND_DATAA findData;
		if (FindNextFileA(searchHandle, &findData)) {
			fpl_Win32FillFileEntry(&findData, nextEntry);
			result = fpl_True;
		}
	}
	return(result);
}
fpl_api void fpl_ListFilesEnd(fpl_FileEntry *lastEntry) {
	FPL_ASSERT(lastEntry != FPL_NULLPTR);
	if (lastEntry->internalHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = (HANDLE)lastEntry->internalHandle;
		FindClose(searchHandle);
	}
	FPL_CLEARSTRUCT_INTERNAL(lastEntry);
}

//
// Win32 Public Path/Directories
//

#	if defined(UNICODE)
fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	FPL_ASSERT(destPath != FPL_NULLPTR);
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
	wchar_t modulePath[MAX_PATH];
	GetModuleFileNameW(NULL, modulePath, MAX_PATH);
	fpl_WideStringToAnsiString(modulePath, fpl_GetWideStringLength(modulePath), destPath, maxDestLen);
}
#	else
fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	FPL_ASSERT(destPath != FPL_NULLPTR);
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
	char modulePath[MAX_PATH];
	GetModuleFileNameA(NULL, modulePath, MAX_PATH);
	fpl_CopyAnsiString(modulePath, fpl_GetAnsiStringLength(modulePath), destPath, maxDestLen);
}
#	endif

fpl_api void fpl_GetHomePath(char *destPath, const uint32_t maxDestLen) {
	FPL_ASSERT(destPath != FPL_NULLPTR);
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
	char homePath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, homePath);
	fpl_CopyAnsiString(homePath, fpl_GetAnsiStringLength(homePath), destPath, maxDestLen);
}

//
// Win32 Public Timing
//
fpl_internal LARGE_INTEGER fpl_Win32GetWallClock_Internal(void) {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return(result);
}
fpl_api double fpl_GetHighResolutionTimeInSeconds() {
	LARGE_INTEGER clock = fpl_Win32GetWallClock_Internal();
	double result = clock.QuadPart / (double)fpl_GlobalWin32State_Internal.performanceFrequency.QuadPart;
	return(result);
}

//
// Win32 Public String
//
fpl_api void fpl_WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxAnsiDestLen >= (requiredLen + 1));
	WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, NULL, NULL);
	ansiDest[requiredLen] = 0;
}
fpl_api void fpl_WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxUtf8DestLen >= (requiredSize + 1));
	WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, NULL, NULL);
	utf8Dest[requiredLen] = 0;
}
fpl_api void fpl_AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}
fpl_api void fpl_UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}

//
// Win32 Public Core
//
fpl_api fpl_LibraryHandle fpl_LoadLibrary(const char *libraryFilePath) {
	FPL_ASSERT(libraryFilePath != NULL);
	fpl_LibraryHandle result = { 0 };
	HMODULE libModule = LoadLibraryA(libraryFilePath);
	if (libModule != FPL_NULLPTR) {
		result.internalHandle = (void *)libModule;
		result.isValid = fpl_True;
	}
	return(result);
}
fpl_api void *fpl_GetLibraryProc(const fpl_LibraryHandle *handle, const char *name) {
	FPL_ASSERT(handle != NULL);
	void *result = FPL_NULLPTR;
	if (handle->isValid) {
		FPL_ASSERT(handle->internalHandle != FPL_NULLPTR);
		HMODULE libModule = (HMODULE)handle;
		result = GetProcAddress(libModule, name);
	}
	return(result);
}
fpl_api void fpl_ReleaseLibrary(fpl_LibraryHandle *handle) {
	FPL_ASSERT(handle != NULL);
	if (handle->isValid) {
		FPL_ASSERT(handle->internalHandle != FPL_NULLPTR);
		HMODULE libModule = (HMODULE)handle->internalHandle;
		FreeLibrary(libModule);
	}
	FPL_CLEARSTRUCT_INTERNAL(handle);
}

//
// Win32 Public Window
//
#	if FPL_ENABLE_WINDOW

#		if FPL_ENABLE_OPENGL
fpl_api void fpl_WindowFlip(void) {
	SwapBuffers(fpl_GlobalWin32State_Internal.window.deviceContext);
}
#		else
fpl_api void fpl_WindowFlip(void) {
}
#		endif // FPL_ENABLE_OPENGL

fpl_api fpl_WindowSize fpl_GetWindowArea(void) {
	fpl_WindowSize result = { 0 };
	RECT windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
		result.width = windowRect.right - windowRect.left;
		result.height = windowRect.bottom - windowRect.top;
	}
	return(result);
}

fpl_api void fpl_SetWindowArea(const uint32_t width, const uint32_t height) {
	RECT clientRect, windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &clientRect) && GetWindowRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
		int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		int newWidth = width + borderWidth;
		int newHeight = height + borderHeight;
		SetWindowPos(fpl_GlobalWin32State_Internal.window.windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

fpl_api fpl_bool32 fpl_IsWindowResizable(void) {
	LONG style = GetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE);
	fpl_bool32 result = style & WS_THICKFRAME;
	return(result);
}

fpl_api void fpl_SetWindowResizeable(const fpl_bool32 value) {
	LONG style = GetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE);
	if (value) {
		style |= WS_THICKFRAME;
	} else {
		style |= ~WS_THICKFRAME;
	}
	SetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE, style);
}

fpl_api fpl_WindowPosition fpl_GetWindowPosition(void) {
	fpl_WindowPosition result = { 0 };
	WINDOWPLACEMENT placement = { 0 };
	placement.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(fpl_GlobalWin32State_Internal.window.windowHandle, &placement) == TRUE) {
		switch (placement.showCmd) {
			case SW_MAXIMIZE:
			{
				result.left = placement.ptMaxPosition.x;
				result.top = placement.ptMaxPosition.y;
			} break;
			case SW_MINIMIZE:
			{
				result.left = placement.ptMinPosition.x;
				result.top = placement.ptMinPosition.y;
			} break;
			default:
			{
				result.left = placement.rcNormalPosition.left;
				result.top = placement.rcNormalPosition.top;
			} break;
		}
	}
	return(result);
}

fpl_api void fpl_SetWindowPosition(const int32_t left, const int32_t top) {
	WINDOWPLACEMENT placement = { 0 };
	placement.length = sizeof(WINDOWPLACEMENT);
	RECT windowRect;
	if (GetWindowPlacement(fpl_GlobalWin32State_Internal.window.windowHandle, &placement) && GetWindowRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
		switch (placement.showCmd) {
			case SW_NORMAL:
			case SW_SHOW:
			{
				placement.rcNormalPosition.left = left;
				placement.rcNormalPosition.top = top;
				placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
				placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
				SetWindowPlacement(fpl_GlobalWin32State_Internal.window.windowHandle, &placement);
			} break;
		}
	}
}

fpl_api void fpl_SetWindowCursorEnabled(const fpl_bool32 value) {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	win32State->window.isCursorActive = value;
}

fpl_internal void fpl_Win32PushMouseEvent_Internal(const fpl_MouseEventType mouseEventType, const fpl_MouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT_INTERNAL(&newEvent);
	newEvent.type = fpl_EventType_Mouse;
	newEvent.mouse.type = mouseEventType;
	newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
	newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
	newEvent.mouse.mouseButton = mouseButton;
	if (mouseEventType == fpl_MouseEventType_Wheel) {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
	}
	fpl_PushEvent_Internal(&newEvent);
}

fpl_internal fpl_Key fpl_Win32MapVirtualKey_Internal(const uint64_t keyCode) {
	switch (keyCode) {
		case VK_BACK:
			return fpl_Key_Backspace;
		case VK_TAB:
			return fpl_Key_Tab;

		case VK_CLEAR:
			return fpl_Key_Clear;
		case VK_RETURN:
			return fpl_Key_Enter;

		case VK_SHIFT:
			return fpl_Key_Shift;
		case VK_CONTROL:
			return fpl_Key_Control;
		case VK_MENU:
			return fpl_Key_Alt;
		case VK_PAUSE:
			return fpl_Key_Pause;
		case VK_CAPITAL:
			return fpl_Key_CapsLock;

		case VK_ESCAPE:
			return fpl_Key_Escape;
		case VK_SPACE:
			return fpl_Key_Space;
		case VK_PRIOR:
			return fpl_Key_PageUp;
		case VK_NEXT:
			return fpl_Key_PageDown;
		case VK_END:
			return fpl_Key_End;
		case VK_HOME:
			return fpl_Key_Home;
		case VK_LEFT:
			return fpl_Key_Left;
		case VK_UP:
			return fpl_Key_Up;
		case VK_RIGHT:
			return fpl_Key_Right;
		case VK_DOWN:
			return fpl_Key_Down;
		case VK_SELECT:
			return fpl_Key_Select;
		case VK_PRINT:
			return fpl_Key_Print;
		case VK_EXECUTE:
			return fpl_Key_Execute;
		case VK_SNAPSHOT:
			return fpl_Key_Snapshot;
		case VK_INSERT:
			return fpl_Key_Insert;
		case VK_DELETE:
			return fpl_Key_Delete;
		case VK_HELP:
			return fpl_Key_Help;

		case 0x30:
			return fpl_Key_0;
		case 0x31:
			return fpl_Key_1;
		case 0x32:
			return fpl_Key_2;
		case 0x33:
			return fpl_Key_3;
		case 0x34:
			return fpl_Key_4;
		case 0x35:
			return fpl_Key_5;
		case 0x36:
			return fpl_Key_6;
		case 0x37:
			return fpl_Key_7;
		case 0x38:
			return fpl_Key_8;
		case 0x39:
			return fpl_Key_9;

		case 0x41:
			return fpl_Key_A;
		case 0x42:
			return fpl_Key_B;
		case 0x43:
			return fpl_Key_C;
		case 0x44:
			return fpl_Key_D;
		case 0x45:
			return fpl_Key_E;
		case 0x46:
			return fpl_Key_F;
		case 0x47:
			return fpl_Key_G;
		case 0x48:
			return fpl_Key_H;
		case 0x49:
			return fpl_Key_I;
		case 0x4A:
			return fpl_Key_J;
		case 0x4B:
			return fpl_Key_K;
		case 0x4C:
			return fpl_Key_L;
		case 0x4D:
			return fpl_Key_M;
		case 0x4E:
			return fpl_Key_N;
		case 0x4F:
			return fpl_Key_O;
		case 0x50:
			return fpl_Key_P;
		case 0x51:
			return fpl_Key_Q;
		case 0x52:
			return fpl_Key_R;
		case 0x53:
			return fpl_Key_S;
		case 0x54:
			return fpl_Key_T;
		case 0x55:
			return fpl_Key_U;
		case 0x56:
			return fpl_Key_V;
		case 0x57:
			return fpl_Key_W;
		case 0x58:
			return fpl_Key_X;
		case 0x59:
			return fpl_Key_Y;
		case 0x5A:
			return fpl_Key_Z;

		case VK_LWIN:
			return fpl_Key_LeftWin;
		case VK_RWIN:
			return fpl_Key_RightWin;
		case VK_APPS:
			return fpl_Key_Apps;

		case VK_SLEEP:
			return fpl_Key_Sleep;
		case VK_NUMPAD0:
			return fpl_Key_NumPad0;
		case VK_NUMPAD1:
			return fpl_Key_NumPad1;
		case VK_NUMPAD2:
			return fpl_Key_NumPad2;
		case VK_NUMPAD3:
			return fpl_Key_NumPad3;
		case VK_NUMPAD4:
			return fpl_Key_NumPad4;
		case VK_NUMPAD5:
			return fpl_Key_NumPad5;
		case VK_NUMPAD6:
			return fpl_Key_NumPad6;
		case VK_NUMPAD7:
			return fpl_Key_NumPad7;
		case VK_NUMPAD8:
			return fpl_Key_NumPad8;
		case VK_NUMPAD9:
			return fpl_Key_NumPad9;
		case VK_MULTIPLY:
			return fpl_Key_Multiply;
		case VK_ADD:
			return fpl_Key_Add;
		case VK_SEPARATOR:
			return fpl_Key_Separator;
		case VK_SUBTRACT:
			return fpl_Key_Substract;
		case VK_DECIMAL:
			return fpl_Key_Decimal;
		case VK_DIVIDE:
			return fpl_Key_Divide;
		case VK_F1:
			return fpl_Key_F1;
		case VK_F2:
			return fpl_Key_F2;
		case VK_F3:
			return fpl_Key_F3;
		case VK_F4:
			return fpl_Key_F4;
		case VK_F5:
			return fpl_Key_F5;
		case VK_F6:
			return fpl_Key_F6;
		case VK_F7:
			return fpl_Key_F7;
		case VK_F8:
			return fpl_Key_F8;
		case VK_F9:
			return fpl_Key_F9;
		case VK_F10:
			return fpl_Key_F10;
		case VK_F11:
			return fpl_Key_F11;
		case VK_F12:
			return fpl_Key_F12;
		case VK_F13:
			return fpl_Key_F13;
		case VK_F14:
			return fpl_Key_F14;
		case VK_F15:
			return fpl_Key_F15;
		case VK_F16:
			return fpl_Key_F16;
		case VK_F17:
			return fpl_Key_F17;
		case VK_F18:
			return fpl_Key_F18;
		case VK_F19:
			return fpl_Key_F19;
		case VK_F20:
			return fpl_Key_F20;
		case VK_F21:
			return fpl_Key_F21;
		case VK_F22:
			return fpl_Key_F22;
		case VK_F23:
			return fpl_Key_F23;
		case VK_F24:
			return fpl_Key_F24;

		case VK_LSHIFT:
			return fpl_Key_LeftShift;
		case VK_RSHIFT:
			return fpl_Key_RightShift;
		case VK_LCONTROL:
			return fpl_Key_LeftControl;
		case VK_RCONTROL:
			return fpl_Key_RightControl;
		case VK_LMENU:
			return fpl_Key_LeftAlt;
		case VK_RMENU:
			return fpl_Key_RightAlt;

		default:
			return fpl_Key_None;
	}
}

fpl_internal void fpl_Win32PushKeyboardEvent_Internal(const fpl_KeyboardEventType keyboardEventType, const uint64_t keyCode, const fpl_KeyboardModifierFlags modifiers, const fpl_bool32 isDown) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT_INTERNAL(&newEvent);
	newEvent.type = fpl_EventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = fpl_Win32MapVirtualKey_Internal(keyCode);
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl_PushEvent_Internal(&newEvent);
}

fpl_internal fpl_bool32 fpl_Win32IsKeyDown_Internal(const uint64_t keyCode) {
	fpl_bool32 result = GetAsyncKeyState((int)keyCode) & 0x8000;
	return(result);
}

LRESULT CALLBACK fpl_Win32MessageProc_Internal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			win32State->window.isRunning = fpl_False;
		} break;

		case WM_SIZE:
		{
			fpl_Event newEvent;
			FPL_CLEARSTRUCT_INTERNAL(&newEvent);
			newEvent.type = fpl_EventType_Window;
			newEvent.window.type = fpl_WindowEventType_Resized;
			newEvent.window.width = LOWORD(lParam);
			newEvent.window.height = HIWORD(lParam);
			fpl_PushEvent_Internal(&newEvent);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint64_t keyCode = wParam;
			fpl_bool32 wasDown = ((lParam & (1 << 30)) != 0);
			fpl_bool32 isDown = ((lParam & (1 << 31)) == 0);

			fpl_bool32 altKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_MENU);
			fpl_bool32 shiftKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LSHIFT);
			fpl_bool32 ctrlKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LCONTROL);
			fpl_bool32 superKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LMENU);

			fpl_KeyboardEventType keyEventType = isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp;
			fpl_KeyboardModifierFlags modifiers = fpl_KeyboardModifierFlags_None;
			if (altKeyWasDown) {
				modifiers |= fpl_KeyboardModifierFlags_Alt;
			}
			if (shiftKeyWasDown) {
				modifiers |= fpl_KeyboardModifierFlags_Shift;
			}
			if (ctrlKeyWasDown) {
				modifiers |= fpl_KeyboardModifierFlags_Ctrl;
			}
			if (superKeyWasDown) {
				modifiers |= fpl_KeyboardModifierFlags_Super;
			}
			fpl_Win32PushKeyboardEvent_Internal(keyEventType, keyCode, modifiers, isDown);

			if (wasDown != isDown) {
				if (isDown) {
					if (keyCode == VK_F4 && altKeyWasDown) {
						win32State->window.isRunning = 0;
					}
				}
			}
			result = 1;
		} break;

		case WM_CHAR:
		{
			uint64_t keyCode = wParam;
			fpl_KeyboardModifierFlags modifiers = { 0 };
			fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType_Char, keyCode, modifiers, 0);
			result = 1;
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_LBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Left, lParam, wParam);
			result = 1;
		} break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_RBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Right, lParam, wParam);
			result = 1;
		} break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_MBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Middle, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEMOVE:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Move, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEWHEEL:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Wheel, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;

		case WM_SETCURSOR:
		{
			// @TODO(final): This is not right to assume default cursor always, because the size cursor does not work this way!
			if (win32State->window.isCursorActive) {
				SetCursor(win32State->window.defaultCursor);
			} else {
				SetCursor(NULL);
			}
			result = 1;
		} break;

		default:
			result = DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return (result);
}

fpl_api fpl_bool32 fpl_WindowUpdate(void) {
	fpl_bool32 result = fpl_False;
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	if (win32State->window.windowHandle != 0) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		result = win32State->window.isRunning;
	}
	return(result);
}

fpl_api fpl_bool32 fpl_IsWindowRunning(void) {
	fpl_bool32 result = fpl_GlobalWin32State_Internal.window.isRunning;
	return(result);
}
#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
fpl_internal fpl_bool32 fpl_Win32CreateOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(win32State->window.deviceContext, &pfd);
	if (pixelFormat == 0) {
		// @TODO(final): Log error
		return fpl_False;
	}
	if (!SetPixelFormat(win32State->window.deviceContext, pixelFormat, &pfd)) {
		// @TODO(final): Log error
		return fpl_False;
	}

	win32State->opengl.renderingContext = wglCreateContext(win32State->window.deviceContext);
	if (!win32State->opengl.renderingContext) {
		// @TODO(final): Log error
		return fpl_False;
	}

	if (!wglMakeCurrent(win32State->window.deviceContext, win32State->opengl.renderingContext)) {
		// @TODO(final): Log error
		return fpl_False;
	}

	return fpl_True;
}

fpl_internal void fpl_Win32ReleaseOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
	if (win32State->opengl.renderingContext) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(win32State->opengl.renderingContext);
		win32State->opengl.renderingContext = NULL;
	}
}
#	else
#		define fpl_Win32CreateOpenGLContext_Internal(win32State) (fpl_True)
#		define fpl_Win32ReleaseOpenGLContext_Internal(win32State)
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

#	if FPL_ENABLE_WINDOW
fpl_internal fpl_bool32 fpl_Win32InitWindow_Internal(fpl_Win32State_Internal *win32State, const fpl_InitFlags initFlags) {
	// Register window class
	WNDCLASSEX windowClass;
	FPL_CLEARSTRUCT_INTERNAL(&windowClass);
	windowClass.hInstance = win32State->appInstance;
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.lpszClassName = WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl_Win32MessageProc_Internal;
	if (RegisterClassEx(&windowClass) == 0) {
		// @TODO(final): Log error
		return fpl_False;
	}
	fpl_Win32StringCopy(windowClass.lpszClassName, fpl_Win32GetStringLength(windowClass.lpszClassName), win32State->window.windowClass, FPL_ARRAYCOUNT(win32State->window.windowClass));

	// Create window
	win32State->window.windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, windowClass.lpszClassName, WIN32_UNNAMED_WINDOW, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FPL_DEFAULT_WINDOW_WIDTH, FPL_DEFAULT_WINDOW_HEIGHT, NULL, NULL, windowClass.hInstance, NULL);
	if (win32State->window.windowHandle == NULL) {
		// @TODO(final): Log error
		return fpl_False;
	}

	// Get device context so we can swap the back and front buffer
	win32State->window.deviceContext = GetDC(win32State->window.windowHandle);
	if (win32State->window.deviceContext == NULL) {
		// @TODO(final): Log error
		return fpl_False;
	}

	// Create opengl rendering context if required
	if (initFlags & fpl_InitFlags_VideoOpenGL) {
		fpl_bool32 openglResult = fpl_Win32CreateOpenGLContext_Internal(win32State);
		if (!openglResult) {
			// @TODO(final): Log error
			return fpl_False;
		}
	}

	void *eventQueueMemory = fpl_AllocateAlignedMemory(sizeof(fpl_EventQueue_Internal), 16);
	fpl_GlobalEventQueue_Internal = (fpl_EventQueue_Internal *)eventQueueMemory;

	// Show window
	ShowWindow(win32State->window.windowHandle, SW_SHOW);
	UpdateWindow(win32State->window.windowHandle);

	// Cursor is visible at start
	win32State->window.defaultCursor = windowClass.hCursor;
	win32State->window.isCursorActive = fpl_True;
	win32State->window.isRunning = fpl_True;

	return fpl_True;
}

fpl_internal void fpl_Win32ReleaseWindow_Internal(fpl_Win32State_Internal *win32State) {
	if (win32State->window.deviceContext != NULL) {
		ReleaseDC(win32State->window.windowHandle, win32State->window.deviceContext);
		win32State->window.deviceContext = NULL;
	}

	if (win32State->window.windowHandle != NULL) {
		DestroyWindow(win32State->window.windowHandle);
		win32State->window.windowHandle = NULL;
		UnregisterClass(win32State->window.windowClass, win32State->appInstance);
	}

	fpl_FreeAlignedMemory(fpl_GlobalEventQueue_Internal);
}
#	else
#		define fpl_Win32InitWindow_Internal(win32State, initFlags) (fpl_True)
#		define fpl_Win32ReleaseWindow_Internal(void)
#	endif // FPL_ENABLE_WINDOW

fpl_api fpl_bool32 fpl_Init(const fpl_InitFlags initFlags) {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	FPL_ASSERT(!win32State->isInitialized);

	// Timing
	QueryPerformanceFrequency(&win32State->performanceFrequency);

	fpl_InitFlags usedInitFlags = initFlags;
	if (usedInitFlags & fpl_InitFlags_VideoOpenGL) {
		usedInitFlags |= fpl_InitFlags_Window;
	}

	if (usedInitFlags & fpl_InitFlags_Window) {
		if (!fpl_Win32InitWindow_Internal(win32State, usedInitFlags)) {
			return fpl_False;
		}
	}

	win32State->isInitialized = fpl_True;

	return (fpl_True);
}

fpl_api void fpl_Release(void) {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	FPL_ASSERT(win32State->isInitialized);
	fpl_Win32ReleaseOpenGLContext_Internal(win32State);
	fpl_Win32ReleaseWindow_Internal(win32State);
	win32State->isInitialized = fpl_False;
}

//
// Win32 Entry-Point
// This is a bit ugly because:
// - Support for Window and Console application
// - Support for Unicode and Ansi
// - Support for disabled C-Runtime-Library
//
// @NOTE(final): If possible clean this up
#	if FPL_ENABLE_WINDOW
#		if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#			if !FPL_ENABLE_C_RUNTIME_LIBRARY
void __stdcall WinMainCRTStartup() {
	int result = wWinMain(GetModuleHandleW(0), 0, 0, 0);
	ExitProcess(result);
}
#			endif // !FPL_ENABLE_C_RUNTIME_LIBRARY
#		else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#if			!FPL_ENABLE_C_RUNTIME_LIBRARY
void __stdcall WinMainCRTStartup() {
	int result = WinMain(GetModuleHandleA(0), 0, 0, 0);
	ExitProcess(result);
}
#			endif // !FPL_ENABLE_C_RUNTIME_LIBRARY
#		endif // defined(UNICODE)
#	else // FPL_ENABLE_WINDOW

#if		!FPL_ENABLE_C_RUNTIME_LIBRARY
#			if defined(UNICODE)
void __stdcall mainCRTStartup() {
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	ExitProcess(result);
}
#			else
void __stdcall mainCRTStartup() {
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	ExitProcess(result);
}
#			endif
#		else
// The main() entry point is used directly
#		endif // !FPL_ENABLE_C_RUNTIME_LIBRARY	

#	endif // FPL_ENABLE_WINDOW

#elif defined(FPL_PLATFORM_LINUX) // FPL_PLATFORM_WINDOWS
	//
	// Linux platform implementation
	//
#	error "Implement linux platform!"
#elif defined(FPL_PLATFORM_UNIX) // FPL_PLATFORM_LINUX
	//
	// Unix platform implementation
	//
#	error "Implement unix platform!"
#else
#	error "Unsupported Platform!"
#endif // !defined(FPL_PLATFORM_UNIX)

#endif // defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)