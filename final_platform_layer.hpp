/*
final_platform_layer.hpp - FPL -  v0.2 alpha
Open-Source Single-File Header-Library by Torsten Spaete

This library is designed to abstract the underlying platform to a very simple and easy to use api.
The only dependencies are built-in operatoring system libraries and the C runtime library.

The main focus is game development, so the default settings will create a window and setup a opengl rendering context.

HOW TO USE:

- In one of your C++ translation units include this:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

- Provide the typical main entry point

int main(int argc, char **args) {
}

- Initialize the library and release it when you are done

InitPlatform(InitFlags::All);
...
ReleasePlatform();

USE CASE [OpenGL-Window]:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

int main(int argc, char **args) {
	using namespace fpl;
	using namespace fpl::window;
	if (InitPlatform(InitFlags::VideoOpenGL)) {
		while (WindowUpdate()) {
			glClear(GL_COLOR_BUFFER_BIT);
			WindowFlip();
		}
		ReleasePlatform();
	}
}

HOW TO COMPILE:

- Win32:

	* Link to kernel32.lib
	* Link to user32.lib
	* Link to shell32.lib
	* Link to opengl32.lib (Only needed if you use opengl)

PREPROCESSOR OVERRIDES:

- FPL_API_AS_PRIVATE 0 or 1

- FPL_DEFAULT_WINDOW_WIDTH 1 or greater
- FPL_DEFAULT_WINDOW_HEIGHT 1 or greater

- FPL_ENABLE_ASSERTIONS 0 or 1
- FPL_ENABLE_C_ASSERT

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
- v0.2 alpha:
	* Dropped C support and moved to a more C++ ish api
	* Dropped no C-Runtime support

*/

#ifndef FPL_INCLUDE_HPP
#define FPL_INCLUDE_HPP

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
#if !defined(FPL_ENABLE_WINDOW)
#	define FPL_ENABLE_WINDOW 1
#endif
#if !defined(FPL_ENABLE_OPENGL)
#	define FPL_ENABLE_OPENGL 1
#endif
#if FPL_ENABLE_ASSERTIONS
#	if !defined(FPL_ENABLE_C_ASSERT)
#		define FPL_ENABLE_C_ASSERT 1
#	endif
#endif
#if !defined(FPL_API_AS_PRIVATE)
#	define FPL_API_AS_PRIVATE 0
#endif

//
// Static/Inline/Extern/Internal
//
#define fpl_globalvar static
#define fpl_internal static
#if FPL_API_AS_PRIVATE
#	define fpl_api static
#else
#	define fpl_api extern
#endif // FPL_API_AS_PRIVATE

//
// Assertions
//
#if FPL_ENABLE_ASSERTIONS
#	if FPL_ENABLE_C_ASSERT
#		include <assert.h>
#		define FPL_ASSERT(exp) assert(exp)
#		define FPL_STATICASSERT(exp) static_assert(exp, "static_assert")
#	else
#		define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
#		define FPL_STATICASSERT_INTERNAL(exp, line) \
			int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#		define FPL_STATICASSERT(exp) \
			FPL_STATICASSERT_INTERNAL(exp, __LINE__)
#	endif // FPL_ENABLE_C_ASSERT
#else
#	define FPL_ASSERT(exp)
#	define FPL_STATICASSERT(exp)
#endif // FPL_ENABLE_ASSERTIONS

//
// Macro functions
//
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FPL_OFFSETOF(type, field) ((type *)((void *)0)->field)

//
// Types
//
#include <stdint.h>

//
// API
//
namespace fpl {
	/* There is no 32-bit boolean type in the standard yet */
	typedef int32_t bool32;

	//
	// Atomics
	//
	namespace atomics {
		/* Insert a atomic read fence. This may be just a compiler optimization for some configurations only. */
		fpl_api void AtomicReadFence(void);
		/* Insert a atomic write fence. This may be just a compiler optimization for some configurations only. */
		fpl_api void AtomicWriteFence(void);
		/* Insert a atomic read/write fence. This may be just a compiler optimization for some configurations only. */
		fpl_api void AtomicReadWriteFence(void);
		/* Replace a 32-bit unsigned integer with the given value atomically. Returns the target before the replacement. */
		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
		/* Replace a 64-bit unsigned integer with the given value atomically. Returns the target before the replacement. */
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
		/* Adds a 32-bit unsigned integer atomatically. Returns the value before the addition. */
		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
		/* Adds a 64-bit unsigned integer atomatically. Returns the value before the addition. */
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
		/* Compares a 32-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange. */
		fpl_api uint32_t AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand);
		/* Compares a 64-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange. */
		fpl_api uint64_t AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand);
	};

	//
	// Core
	//
		/* Initialization flags (Window, Video, All, etc.) */
	enum class InitFlags : uint32_t {
		None = 0,
		Window = 1 << 0,
		VideoOpenGL = 1 << 1,
		All = Window | VideoOpenGL,
	};
	inline InitFlags operator | (const InitFlags lhs, const InitFlags rhs) {
		return (InitFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}
	inline bool operator & (const InitFlags lhs, const InitFlags rhs) {
		return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) == static_cast<uint32_t>(rhs);
	}
	inline InitFlags& operator |= (InitFlags &lhs, const InitFlags rhs) {
		lhs = (InitFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		return lhs;
	}
	/* Initialize the platform layer. */
	fpl_api bool32 InitPlatform(const InitFlags initFlags);
	/* Releases the platform layer and resets all structures to zero. */
	fpl_api void ReleasePlatform(void);

	/* Handle to a loaded dynamic library */
	namespace library {
		struct DynamicLibraryHandle {
			void *internalHandle;
			bool32 isValid;
		};

		/* Loads a dynamic library and returns the loaded handle for it. */
		fpl_api DynamicLibraryHandle LoadDynamicLibrary(const char *libraryFilePath);
		/* Returns the dynamic library procedure address for the given procedure name. */
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle *handle, const char *name);
		/* Releases the loaded library and resets the handle to zero. */
		fpl_api void ReleaseDynamicLibrary(DynamicLibraryHandle *handle);
	};

	//
	// Console
	//
	namespace console {
		/* Writes the given text to the default console output */
		fpl_api void ConsoleOut(const char *text);
		/* Writes the given formatted text to the default console output */
		fpl_api void ConsoleFormatOut(const char *format, ...);
		/* Writes the given text to the console error output */
		fpl_api void ConsoleError(const char *text);
		/* Writes the given formatted text to the console error output */
		fpl_api void ConsoleFormatError(const char *format, ...);
	};

	//
	// Memory
	//
	namespace memory {
		/* Resets the given memory pointer by the given size to zero. */
		fpl_api void ClearMemory(void *mem, const size_t size);
		/* Allocates memory from the operating system by the given size. */
		fpl_api void *AllocateMemory(const size_t size);
		/* Releases the memory allocated from the operating system. */
		fpl_api void FreeMemory(void *ptr);
		/* Allocates aligned memory from the operating system by the given alignment. */
		fpl_api void *AllocateAlignedMemory(const size_t size, const size_t alignment);
		/* Releases aligned memory from the operating system. */
		fpl_api void FreeAlignedMemory(void *ptr);
	};

	//
	// Timing
	//
	namespace timings {
		/* Returns the current system clock in seconds with the highest precision. */
		fpl_api double GetHighResolutionTimeInSeconds(void);
	};

	//
	// Strings
	//
	namespace strings {
		/* Returns the number of characters of the given 8-bit Ansi string. Null terminator is not included. */
		fpl_api uint32_t GetAnsiStringLength(const char *str);
		/* Returns the number of characters of the given 16-bit Wide string. Null terminator is not included. */
		fpl_api uint32_t GetWideStringLength(const wchar_t *str);
		/* Copies the given 8-bit Ansi string into a destination Ansi string. Does not allocate any memory. */
		fpl_api void CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
		/* Copies the given 16-bit Wide string into a destination Wide string. Does not allocate any memory. */
		fpl_api void CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
		/* Converts the given 16-bit Wide string in a 8-bit Ansi string. Does not allocate any memory. */
		fpl_api void WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
		/* Converts the given 16-bit Wide string in a 8-bit UTF-8 string. Does not allocate any memory. */
		fpl_api void WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
		/* Converts the given 8-bit Ansi string in a 16-bit Wide string. Does not allocate any memory. */
		fpl_api void AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
		/* Converts the given 8-bit UTF-8 string in a 16-bit Wide string. Does not allocate any memory. */
		fpl_api void UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
	};

	//
	// Files / Directories
	//
	namespace files {
		/* Handle to a loaded/created file */
		struct FileHandle {
			void *internalHandle;
			bool32 isValid;
		};

		/* File position mode (Beginning, Current, End) */
		enum class FilePositionMode {
			Beginning = 0,
			Current = 1,
			End = 2,
		};

		/* File entry type (File, Directory, etc.) */
		enum class FileEntryType {
			Unknown = 0,
			File = 1,
			Directory = 2,
		};

		/* File attribute flags (Normal, Readonly, Hidden, etc.) */
		enum class FileAttributeFlags : uint32_t {
			None = 0,
			Normal = 1 << 0,
			ReadOnly = 1 << 1,
			Hidden = 1 << 2,
			Archive = 1 << 3,
			System = 1 << 4,
		};
		inline FileAttributeFlags operator | (FileAttributeFlags lhs, FileAttributeFlags rhs) {
			return (FileAttributeFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		inline FileAttributeFlags& operator |= (FileAttributeFlags &lhs, FileAttributeFlags rhs) {
			lhs = (FileAttributeFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
			return lhs;
		}

		constexpr uint32_t MAX_FILEENTRY_PATH_LENGTH = 1024;

		/* Entry for storing current file informations (path, type, attributes, etc.) */
		struct FileEntry {
			FileEntryType type;
			FileAttributeFlags attributes;
			char path[MAX_FILEENTRY_PATH_LENGTH];
			void *internalHandle;
		};

		/* Opens a binary file for reading and returns the handle of it. */
		fpl_api FileHandle OpenBinaryFile(const char *filePath);
		/* Creates a binary file and returns the handle of it. */
		fpl_api FileHandle CreateBinaryFile(const char *filePath);
		/* Reads a block from the given file handle and returns the number of bytes readed. Operation is limited to a 2 GB byte boundary. */
		fpl_api uint32_t ReadFileBlock32(FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
		/* Writes a block to the given file handle and returns the number of bytes written. Operation is limited to a 2 GB byte boundary. */
		fpl_api uint32_t WriteFileBlock32(FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
		/* Sets the current file position by the given position, depending on the mode its absolute or relative. Position is limited to a 2 GB byte boundary. */
		fpl_api void SetFilePosition32(FileHandle *fileHandle, const uint32_t position, const FilePositionMode mode);
		/* Returns the current file position. Position is limited to a 2 GB byte boundary. */
		fpl_api uint32_t GetFilePosition32(FileHandle *fileHandle);
		/* Closes the given file handle and resets the handle to zero. */
		fpl_api void CloseFile2(FileHandle *fileHandle);

		// @TODO(final): Add 64-bit file operations as well!

		/* Returns the 32-bit file size in bytes for the given file. Its limited to files < 2 GB. */
		fpl_api uint32_t GetFileSize32(const char *filePath);
		/* Returns true when the given file path physically exists. */
		fpl_api bool32 FileExists(const char *filePath);
		/* Copies the given source file to the target path and returns truwe when copy was successful. Target path must include the full path to the file. When overwrite is set, the target file path will always be overwritten. */
		fpl_api bool32 CopyFile2(const char *sourceFilePath, const char *targetFilePath, const bool32 overwrite);
		/* Deletes the given file without confirmation and returns true when the deletion was successful. */
		fpl_api bool32 DeleteFile2(const char *filePath);

		/* Returns true when the given directory path physically exists. */
		fpl_api bool32 DirectoryExists(const char *path);
		/* Deletes the given directory without confirmation and returns true when the deletion was successful. When recursive is set, all files and folders in sub-directories will be deleted as well. */
		fpl_api bool32 RemoveEmptyDirectory(const char *path);
		/* Iterates through files / directories in the given directory. The path must contain the filter as well. Returns true when there was a first entry found. */
		fpl_api bool32 ListFilesBegin(const char *pathAndFilter, FileEntry *firstEntry);
		/* Get next file entry from iterating through files / directories. Returns false when there is no entry found. */
		fpl_api bool32 ListFilesNext(FileEntry *nextEntry);
		/* Releases opened resources from iterating through files / directories. */
		fpl_api void ListFilesEnd(FileEntry *lastEntry);
	};

	//
	// Directories and Paths
	//
	namespace paths {
		/* Returns the full path to this executable, including the executable file name. */
		fpl_api void GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
		/* Returns the full path to your home directory. */
		fpl_api void GetHomePath(char *destPath, const uint32_t maxDestLen);
		/* Returns the path from the given source path. */
		fpl_api char *ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
		/* Returns the file extension from the given source path. */
		fpl_api char *ExtractFileExtension(const char *sourcePath);
		/* Returns the file name including the file extension from the given source path. */
		fpl_api char *ExtractFileName(const char *sourcePath);
		/* Changes the file extension on the given source path and writes the result into the destination path. Returns the pointer of the destination path. */
		fpl_api char *ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *filePath, const char *newFileExtension);
		/* Combines all included path by the systems path separator. Returns the pointer of the destination path. */
		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);
	};

#if FPL_ENABLE_WINDOW
	namespace window {
		// @NOTE(final): Based on MS Virtual-Key-Codes, mostly directly mappable to ASCII
		enum class Key {
			Key_None = 0,

			// 0x07: Undefined

			Key_Backspace = 0x08,
			Key_Tab = 0x09,

			// 0x0A-0x0B: Reserved

			Key_Clear = 0x0C,
			Key_Enter = 0x0D,

			// 0x0E-0x0F: Undefined

			Key_Shift = 0x10,
			Key_Control = 0x11,
			Key_Alt = 0x12,
			Key_Pause = 0x13,
			Key_CapsLock = 0x14,

			// 0x15: IME-Keys
			// 0x16: Undefined
			// 0x17-0x19 IME-Keys
			// 0x1A: Undefined

			Key_Escape = 0x1B,

			// 0x1C - 0x1F: IME-Keys

			Key_Space = 0x20,
			Key_PageUp = 0x21,
			Key_PageDown = 0x22,
			Key_End = 0x23,
			Key_Home = 0x24,
			Key_Left = 0x25,
			Key_Up = 0x26,
			Key_Right = 0x27,
			Key_Down = 0x28,
			Key_Select = 0x29,
			Key_Print = 0x2A,
			Key_Execute = 0x2B,
			Key_Snapshot = 0x2C,
			Key_Insert = 0x2D,
			Key_Delete = 0x2E,
			Key_Help = 0x2F,

			Key_0 = 0x30,
			Key_1 = 0x31,
			Key_2 = 0x32,
			Key_3 = 0x33,
			Key_4 = 0x34,
			Key_5 = 0x35,
			Key_6 = 0x36,
			Key_7 = 0x37,
			Key_8 = 0x38,
			Key_9 = 0x39,

			// 0x3A-0x40: Undefined

			Key_A = 0x41,
			Key_B = 0x42,
			Key_C = 0x43,
			Key_D = 0x44,
			Key_E = 0x45,
			Key_F = 0x46,
			Key_G = 0x47,
			Key_H = 0x48,
			Key_I = 0x49,
			Key_J = 0x4A,
			Key_K = 0x4B,
			Key_L = 0x4C,
			Key_M = 0x4D,
			Key_N = 0x4E,
			Key_O = 0x4F,
			Key_P = 0x50,
			Key_Q = 0x51,
			Key_R = 0x52,
			Key_S = 0x53,
			Key_T = 0x54,
			Key_U = 0x55,
			Key_V = 0x56,
			Key_W = 0x57,
			Key_X = 0x58,
			Key_Y = 0x59,
			Key_Z = 0x5A,

			Key_LeftWin = 0x5B,
			Key_RightWin = 0x5C,
			Key_Apps = 0x5D,

			// 0x5E: Reserved

			Key_Sleep = 0x5F,
			Key_NumPad0 = 0x60,
			Key_NumPad1 = 0x61,
			Key_NumPad2 = 0x62,
			Key_NumPad3 = 0x63,
			Key_NumPad4 = 0x64,
			Key_NumPad5 = 0x65,
			Key_NumPad6 = 0x66,
			Key_NumPad7 = 0x67,
			Key_NumPad8 = 0x68,
			Key_NumPad9 = 0x69,
			Key_Multiply = 0x6A,
			Key_Add = 0x6B,
			Key_Separator = 0x6C,
			Key_Substract = 0x6D,
			Key_Decimal = 0x6E,
			Key_Divide = 0x6F,
			Key_F1 = 0x70,
			Key_F2 = 0x71,
			Key_F3 = 0x72,
			Key_F4 = 0x73,
			Key_F5 = 0x74,
			Key_F6 = 0x75,
			Key_F7 = 0x76,
			Key_F8 = 0x77,
			Key_F9 = 0x78,
			Key_F10 = 0x79,
			Key_F11 = 0x7A,
			Key_F12 = 0x7B,
			Key_F13 = 0x7C,
			Key_F14 = 0x7D,
			Key_F15 = 0x7E,
			Key_F16 = 0x7F,
			Key_F17 = 0x80,
			Key_F18 = 0x81,
			Key_F19 = 0x82,
			Key_F20 = 0x83,
			Key_F21 = 0x84,
			Key_F22 = 0x85,
			Key_F23 = 0x86,
			Key_F24 = 0x87,

			// 0x88-8F: Unassigned

			Key_NumLock = 0x90,
			Key_Scroll = 0x91,

			// 0x92-9x96: OEM specific
			// 0x97-0x9F: Unassigned

			Key_LeftShift = 0xA0,
			Key_RightShift = 0xA1,
			Key_LeftControl = 0xA2,
			Key_RightControl = 0xA3,
			Key_LeftAlt = 0xA4,
			Key_RightAlt = 0xA5,

			// 0xA6-0xFE: Dont care
		};

		/* Window configuration (Title, Size, etc.) */
		struct WindowConfiguration {
			char windowTitle[128];
			uint32_t windowWidth;
			uint32_t windowHeight;
		};

		/* Window event type (Resized, PositionChanged, etc.) */
		enum class WindowEventType {
			Resized = 1,
		};

		/* Window event (Size, Position, etc.) */
		struct WindowEvent {
			WindowEventType type;
			uint32_t width;
			uint32_t height;
		};

		/* Keyboard event type (KeyDown, KeyUp, Char, ...) */
		enum class KeyboardEventType {
			KeyDown = 1,
			KeyUp = 2,
			Char = 3,
		};

		/* Keyboard modifier flags (Alt, Ctrl, ...) */
		enum class KeyboardModifierFlags : uint32_t {
			None = 0,
			Alt = 1 << 0,
			Ctrl = 1 << 1,
			Shift = 1 << 2,
			Super = 1 << 3,
		};
		inline KeyboardModifierFlags operator | (KeyboardModifierFlags lhs, KeyboardModifierFlags rhs) {
			return (KeyboardModifierFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
		}
		inline KeyboardModifierFlags& operator |= (KeyboardModifierFlags &lhs, KeyboardModifierFlags rhs) {
			lhs = (KeyboardModifierFlags)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
			return lhs;
		}

		/* Keyboard event (Type, Keycode, Mapped key, etc.) */
		struct KeyboardEvent {
			KeyboardEventType type;
			uint64_t keyCode;
			Key mappedKey;
			KeyboardModifierFlags modifiers;
		};

		/* Mouse event type (Move, ButtonDown, ...) */
		enum class MouseEventType {
			Move = 1,
			ButtonDown = 2,
			ButtonUp = 3,
			Wheel = 4,
		};

		/* Mouse button type (Left, Right, ...) */
		enum class MouseButtonType : int32_t {
			None = -1,
			Left = 0,
			Right = 1,
			Middle = 2,
		};

		/* Mouse event (Type, Button, Position, etc.) */
		struct MouseEvent {
			MouseEventType type;
			MouseButtonType mouseButton;
			int32_t mouseX;
			int32_t mouseY;
			float wheelDelta;
			int32_t _padding;
		};

		/* Event type (Window, Keyboard, Mouse, ...) */
		enum class EventType {
			Window = 1,
			Keyboard = 2,
			Mouse = 3,
		};

		/* Event (Type, Window, Keyboard, Mouse, etc.) */
		struct Event {
			EventType type;
			union {
				WindowEvent window;
				KeyboardEvent keyboard;
				MouseEvent mouse;
			};
		};

		/* Window size in screen coordinates */
		struct WindowSize {
			uint32_t width;
			uint32_t height;
		};

		/* Window position in screen coordinates */
		struct WindowPosition {
			int32_t left;
			int32_t top;
		};

		//
		// Window
		//
		/* Returns true when the window is active. */
		fpl_api bool32 IsWindowRunning(void);
		/* Processes the message queue of the window. */
		fpl_api bool32 WindowUpdate(void);
		/* Forces the window to redraw or swap the back/front buffer. */
		fpl_api void WindowFlip(void);
		/* Enables or disables the window cursor. */
		fpl_api void SetWindowCursorEnabled(const bool32 value);
		/* Returns the inner window area. */
		fpl_api WindowSize GetWindowArea(void);
		/* Resizes the window to fit the inner area to the given size. */
		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height);
		/* Returns true when the window is resizable. */
		fpl_api bool32 IsWindowResizable(void);
		/* Enables or disables the ability to resize the window. */
		fpl_api void SetWindowResizeable(const bool32 value);
		/* Returns the absolute window position. */
		fpl_api WindowPosition GetWindowPosition(void);
		/* Sets the window absolut position to the given coordinates. */
		fpl_api void SetWindowPosition(const int32_t left, const int32_t top);

		//
		// Events
		//
		/* Gets and removes the top event from the internal queue and fills out the "event" argument. Returns false when there are no events left, otherwise true. */
		fpl_api bool32 PollWindowEvent(Event *event);
	};
#endif

};

#if defined(FPL_PLATFORM_WINDOWS)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_api int main(int argc, char **args);
#endif

#endif // FPL_INCLUDE_HPP

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

namespace fpl {

	//
	// Platform constants
	//
#	if defined(FPL_PLATFORM_WINDOWS)
	constexpr char FPL_PATH_SEPARATOR = '\\';
	constexpr char FPL_FILE_EXT_SEPARATOR = '.';
#	else
	constexpr char FPL_PATH_SEPARATOR = '/';
	constexpr char FPL_FILE_EXT_SEPARATOR = '.';
#	endif

	//
	// Internal types and functions
	//
	namespace memory {
		template <typename T>
		inline static void ClearMemory(void *memory, const size_t size, const uint32_t shift, const uint32_t mask) {
			T *dataBlock = static_cast<T *>(memory);
			T *dataBlockEnd = static_cast<T *>(memory) + (size >> shift);
			uint8_t *data8 = (uint8_t *)dataBlockEnd;
			uint8_t *data8End = data8 + (size & mask);
			while (dataBlock != dataBlockEnd) {
				*dataBlock++ = 0;
			}
			while (data8 != data8End) {
				*data8++ = 0;
			}
		}
	}

	constexpr uint32_t FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL = 256;
	struct fpl_ErrorState_Internal {
		char lastError[FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
	};

#if FPL_ENABLE_WINDOW
	constexpr uint32_t FPL_MAX_EVENT_COUNT_INTERNAL = 32768;
	struct fpl_EventQueue_Internal {
		window::Event events[FPL_MAX_EVENT_COUNT_INTERNAL];
		volatile uint32_t pollIndex;
		volatile uint32_t pushCount;
	};

	fpl_globalvar fpl_EventQueue_Internal *fpl_GlobalEventQueue_Internal = 0;
#endif

	//
	// All Public String
	//
	namespace strings {
		fpl_api uint32_t GetAnsiStringLength(const char *str) {
			uint32_t result = 0;
			if (str) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api uint32_t GetWideStringLength(const wchar_t *str) {
			uint32_t result = 0;
			if (str) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api void CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source && dest);
			FPL_ASSERT((sourceLen + 1) <= maxDestLen);
			uint32_t index = 0;
			while (index++ < sourceLen) {
				*dest++ = *source++;
			}
			*dest = 0;
		}

		fpl_api void CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source && dest);
			FPL_ASSERT((sourceLen + 1) <= maxDestLen);
			uint32_t index = 0;
			while (index++ < sourceLen) {
				*dest++ = *source++;
			}
			dest[sourceLen] = 0;
		}
	}

	//
	// All Public Memory
	//
	namespace memory {
		fpl_api void *AllocateAlignedMemory(const size_t size, const size_t alignment) {
			FPL_ASSERT(size > 0);
			FPL_ASSERT((alignment > 0) && !(alignment & (alignment - 1)));

			// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
			size_t newSize = sizeof(void *) + size + (alignment << 1);
			void *basePtr = AllocateMemory(newSize);
			ClearMemory(basePtr, newSize);

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

		fpl_api void FreeAlignedMemory(void *ptr) {
			FPL_ASSERT(ptr != nullptr);

			// Free the base pointer which is stored to the left from the given pointer
			void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
			FreeMemory(basePtr);
		}

		fpl_api void ClearMemory(void *mem, const size_t size) {
			if (size % 8 == 0) {
				ClearMemory<uint64_t>(mem, size, 3, 0x00000007);
			} else if (size % 4 == 0) {
				ClearMemory<uint32_t>(mem, size, 2, 0x00000003);
			} else if (size % 2 == 0) {
				ClearMemory<uint16_t>(mem, size, 1, 0x00000001);
			} else {
				uint8_t *data8 = (uint8_t *)mem;
				uint8_t *data8End = data8 + size;
				while (data8 != data8End) {
					*data8++ = 0;
				}
			}
		}
	}

	//
	// All Public Window
	//
#if FPL_ENABLE_WINDOW
	fpl_internal void PushEvent_Internal(const window::Event &event) {
		fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
		FPL_ASSERT(eventQueue != NULL);
		if (eventQueue->pushCount < FPL_MAX_EVENT_COUNT_INTERNAL) {
			uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pushCount, 1);
			FPL_ASSERT(eventIndex < FPL_MAX_EVENT_COUNT_INTERNAL);
			eventQueue->events[eventIndex] = event;
		}
	}

	namespace window {
		fpl_api bool32 PollWindowEvent(Event *event) {
			bool32 result = false;
			fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
			FPL_ASSERT(eventQueue != NULL);
			if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pollIndex, 1);
				*event = eventQueue->events[eventIndex];
				result = true;
			} else if (fpl_GlobalEventQueue_Internal->pushCount > 0) {
				atomics::AtomicExchangeU32(&eventQueue->pollIndex, 0);
				atomics::AtomicExchangeU32(&eventQueue->pushCount, 0);
			}
			return result;
		}
	}
#endif

	//
	// All Public Path, Directories
	//
	namespace paths {
		fpl_api char *ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
			using namespace strings;
			char *result = (char *)nullptr;
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
					CopyAnsiString(sourcePath, copyLen, destPath, maxDestLen);
				}
			}
			return(result);
		}

		fpl_api char *ExtractFileExtension(const char *sourcePath) {
			char *result = (char *)nullptr;
			if (sourcePath != nullptr) {
				char *filename = ExtractFileName(sourcePath);
				if (filename) {
					char *chPtr = filename;
					char *firstSeparatorPtr = (char *)nullptr;
					while (*chPtr) {
						if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
							firstSeparatorPtr = chPtr;
							break;
						}
						++chPtr;
					}
					if (firstSeparatorPtr != nullptr) {
						result = firstSeparatorPtr;
					}
				}
			}
			return(result);
		}

		fpl_api char *ExtractFileName(const char *sourcePath) {
			char *result = (char *)nullptr;
			if (sourcePath) {
				result = (char *)sourcePath;
				char *chPtr = (char *)sourcePath;
				char *lastPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == FPL_PATH_SEPARATOR) {
						lastPtr = chPtr;
					}
					++chPtr;
				}
				if (lastPtr != nullptr) {
					result = lastPtr + 1;
				}
			}
			return(result);
		}

		fpl_api char *ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *filePath, const char *newFileExtension) {
			using namespace strings;
			char *result = (char *)nullptr;
			if (filePath != nullptr) {
				// Find last path
				char *chPtr = (char *)filePath;
				char *lastPathSeparatorPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == FPL_PATH_SEPARATOR) {
						lastPathSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				// Find last ext separator
				if (lastPathSeparatorPtr != nullptr) {
					chPtr = lastPathSeparatorPtr + 1;
				} else {
					chPtr = (char *)filePath;
				}
				char *lastExtSeparatorPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
						lastExtSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				uint32_t pathLen = GetAnsiStringLength(filePath);
				uint32_t copyLen;
				if (lastExtSeparatorPtr != nullptr) {
					copyLen = (uint32_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
				} else {
					copyLen = pathLen;
				}

				// Copy parts
				uint32_t extLen = GetAnsiStringLength(newFileExtension);
				CopyAnsiString(filePath, copyLen, destPath, maxDestLen);
				char *destExtPtr = destPath + copyLen;
				CopyAnsiString(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
				result = destPath;
			}
			return(result);
		}

		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
			using namespace strings;
			uint32_t curDestPosition = 0;
			char *currentDestPtr = destPath;
			va_list vargs;
			va_start(vargs, pathCount);
			for (uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
				char *path = va_arg(vargs, char *);
				uint32_t pathLen = GetAnsiStringLength(path);
				bool32 requireSeparator = pathIndex < (pathCount - 1);
				uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
				FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
				CopyAnsiString(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
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
	}

	//
	// Compiler Intrinsics
	//
	namespace atomics {
#	if defined(FPL_COMPILER_MSVC)
#		include <intrin.h>
		fpl_api void AtomicReadFence(void) {
			_ReadBarrier();
		}
		fpl_api void AtomicWriteFence(void) {
			_WriteBarrier();
		}
		fpl_api void AtomicReadWriteFence(void) {
			_ReadWriteBarrier();
		}
		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			FPL_ASSERT(target != nullptr);
			uint32_t result = _InterlockedExchange((volatile long *)target, value);
			return (result);
		}
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			FPL_ASSERT(target != nullptr);
			uint64_t result = _InterlockedExchange64((volatile long long *)target, value);
			return (result);
		}
		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
			return (result);
		}
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint64_t result = _InterlockedExchangeAdd64((volatile long long *)value, addend);
			return (result);
		}
		fpl_api uint32_t AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			return (result);
		}
		fpl_api uint64_t AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = _InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			return (result);
		}
#		else
#			error "Unsupported win32 compiler for intrinsics"
#		endif // defined(FPL_COMPILER_MSVC)
	}
}

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WINDOWS)
// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#	define NOMINMAX
#	include <Windows.h> // Win32 api
#	include <windowsx.h> // macros for window messages
#	include <ShlObj.h> // SHGetFolderPath

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
#		include <gl\gl.h>
#	endif // FPL_ENABLE_WINDOW

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
#		define fpl_Win32StringCopy strings::CopyWideString
#		define fpl_Win32GetStringLength strings::GetWideStringLength
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
#		define fpl_Win32StringCopy strings::CopyAnsiString
#		define fpl_Win32GetStringLength strings::GetAnsiStringLength
#		define MapVirtualKey MapVirtualKeyA
#	endif // defined(UNICODE)

namespace fpl {

	// Win32 internal functions
#	if defined(UNICODE)
	typedef wchar_t fpl_win32_char_internal;
#	else
	typedef char fpl_win32_char_internal;
#	endif // defined(UNICODE)

#	if FPL_ENABLE_WINDOW
	struct fpl_Win32WindowState_Internal {
		HWND windowHandle;
		fpl_win32_char_internal windowClass[256];
		HDC deviceContext;
		bool32 isCursorActive;
		HCURSOR defaultCursor;
		bool32 isRunning;
	};
#	else
	typedef void *fpl_Win32WindowState_Internal;
#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
	struct fpl_Win32OpenGLState_Internal {
		HGLRC renderingContext;
	};
#	else
	typedef void *fpl_Win32OpenGLState_Internal;
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

	struct fpl_Win32State_Internal {
		bool32 isInitialized;
		HINSTANCE appInstance;
		LARGE_INTEGER performanceFrequency;
		fpl_Win32WindowState_Internal window;
		fpl_Win32OpenGLState_Internal opengl;
	};

	fpl_globalvar fpl_Win32State_Internal fpl_GlobalWin32State_Internal = { 0 };

	// Win32 public console
	namespace console {
		fpl_api void ConsoleOut(const char *text) {
			FPL_ASSERT(text != nullptr);
			fprintf(stdout, "%s", text);
		}
		fpl_api void ConsoleFormatOut(const char *format, ...) {
			FPL_ASSERT(format != nullptr);
			va_list vaList;
			va_start(vaList, format);
			vfprintf(stdout, format, vaList);
			va_end(vaList);
		}
		fpl_api void ConsoleError(const char *text) {
			FPL_ASSERT(text != nullptr);
			fprintf(stderr, "%s", text);
		}
		fpl_api void ConsoleFormatError(const char *format, ...) {
			FPL_ASSERT(format != nullptr);
			va_list vaList;
			va_start(vaList, format);
			vfprintf(stderr, format, vaList);
			va_end(vaList);
		}
	}

	//
	// Win32 Public Memory
	//
	namespace memory {
		fpl_api void *AllocateMemory(const size_t size) {
			FPL_ASSERT(size > 0);
			void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			return(result);
		}

		fpl_api void FreeMemory(void *ptr) {
			FPL_ASSERT(ptr != NULL);
			VirtualFree(ptr, 0, MEM_FREE);
		}
	}

	//
	// Win32 Public File
	//
	namespace files {
		fpl_api FileHandle OpenBinaryFile(const char *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = { 0 };
			HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}

		fpl_api FileHandle CreateBinaryFile(const char *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = { 0 };
			HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}

		fpl_api uint32_t ReadFileBlock32(FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
			FPL_ASSERT(fileHandle != nullptr);
			FPL_ASSERT(targetBuffer != nullptr);
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

		fpl_api uint32_t WriteFileBlock32(FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
			FPL_ASSERT(fileHandle != nullptr);
			FPL_ASSERT(sourceBuffer != nullptr);
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

		fpl_api void SetFilePosition32(FileHandle *fileHandle, const uint32_t position, const FilePositionMode mode) {
			FPL_ASSERT(fileHandle != nullptr);
			if (fileHandle->isValid) {
				FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
				DWORD moveMethod = FILE_BEGIN;
				if (mode == FilePositionMode::Current) {
					moveMethod = FILE_CURRENT;
				} else if (mode == FilePositionMode::End) {
					moveMethod = FILE_END;
				}
				SetFilePointer(win32FileHandle, (LONG)position, NULL, moveMethod);
			}
		}

		fpl_api uint32_t GetFilePosition32(FileHandle *fileHandle) {
			FPL_ASSERT(fileHandle != nullptr);
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

		fpl_api void CloseFile2(FileHandle *fileHandle) {
			FPL_ASSERT(fileHandle != nullptr);
			if (fileHandle->isValid) {
				FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
				CloseHandle(win32FileHandle);
			}
			*fileHandle = {};
		}

		fpl_api uint32_t GetFileSize32(const char *filePath) {
			uint32_t result = 0;
			HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				DWORD fileSize = GetFileSize(win32FileHandle, NULL);
				result = fileSize;
				CloseHandle(win32FileHandle);
			}
			return(result);
		}
		fpl_api bool32 FileExists(const char *filePath) {
			bool32 result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FindFirstFileA(filePath, &findData);
			if (searchHandle != INVALID_HANDLE_VALUE) {
				result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				FindClose(searchHandle);
			}
			return(result);
		}
		fpl_api bool32 CopyFile2(const char *sourceFilePath, const char *targetFilePath, const bool32 overwrite) {
			bool32 result = CopyFileA(sourceFilePath, targetFilePath, !overwrite);
			return(result);
		}
		fpl_api bool32 DeleteFile2(const char *filePath) {
			bool32 result = DeleteFileA(filePath);
			return(result);
		}
		fpl_api bool32 DirectoryExists(const char *path) {
			bool32 result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FindFirstFileA(path, &findData);
			if (searchHandle != INVALID_HANDLE_VALUE) {
				result = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
				CloseHandle(searchHandle);
			}
			return(result);
		}
		fpl_api bool32 RemoveEmptyDirectory(const char *path) {
			bool32 result = RemoveDirectoryA(path);
			return(result);
		}
		fpl_internal void fpl_Win32FillFileEntry(WIN32_FIND_DATAA *findData, FileEntry *entry) {
			using namespace strings;

			CopyAnsiString(findData->cFileName, GetAnsiStringLength(findData->cFileName), entry->path, FPL_ARRAYCOUNT(entry->path));

			entry->type = FileEntryType::Unknown;
			if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				entry->type = FileEntryType::Directory;
			} else if (
				(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
				entry->type = FileEntryType::File;
			}

			entry->attributes = FileAttributeFlags::None;
			if (findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
				entry->attributes = FileAttributeFlags::Normal;
			} else {
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
					entry->attributes |= FileAttributeFlags::Hidden;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					entry->attributes |= FileAttributeFlags::ReadOnly;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
					entry->attributes |= FileAttributeFlags::Archive;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
					entry->attributes |= FileAttributeFlags::System;
				}
			}
		}
		fpl_api bool32 ListFilesBegin(const char *pathAndFilter, FileEntry *firstEntry) {
			FPL_ASSERT(pathAndFilter != nullptr);
			FPL_ASSERT(firstEntry != nullptr);
			bool32 result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FindFirstFileA(pathAndFilter, &findData);
			*firstEntry = {};
			if (searchHandle != INVALID_HANDLE_VALUE) {
				firstEntry->internalHandle = (void *)searchHandle;
				fpl_Win32FillFileEntry(&findData, firstEntry);
				result = true;
			}
			return(result);
		}
		fpl_api bool32 ListFilesNext(FileEntry *nextEntry) {
			FPL_ASSERT(nextEntry != nullptr);
			bool32 result = false;
			if (nextEntry->internalHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = (HANDLE)nextEntry->internalHandle;
				WIN32_FIND_DATAA findData;
				if (FindNextFileA(searchHandle, &findData)) {
					fpl_Win32FillFileEntry(&findData, nextEntry);
					result = true;
				}
			}
			return(result);
		}
		fpl_api void ListFilesEnd(FileEntry *lastEntry) {
			FPL_ASSERT(lastEntry != nullptr);
			if (lastEntry->internalHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = (HANDLE)lastEntry->internalHandle;
				FindClose(searchHandle);
			}
			*lastEntry = {};
		}
	}

	//
	// Win32 Public Path/Directories
	//
	namespace paths {
#	if defined(UNICODE)
		fpl_api void GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			using namespace strings;
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			wchar_t modulePath[MAX_PATH];
			GetModuleFileNameW(NULL, modulePath, MAX_PATH);
			WideStringToAnsiString(modulePath, GetWideStringLength(modulePath), destPath, maxDestLen);
		}
#	else
		fpl_api void GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			using namespace strings;
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			char modulePath[MAX_PATH];
			GetModuleFileNameA(NULL, modulePath, MAX_PATH);
			CopyAnsiString(modulePath, GetAnsiStringLength(modulePath), destPath, maxDestLen);
		}
#	endif

		fpl_api void GetHomePath(char *destPath, const uint32_t maxDestLen) {
			using namespace strings;
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			char homePath[MAX_PATH];
			SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, homePath);
			CopyAnsiString(homePath, GetAnsiStringLength(homePath), destPath, maxDestLen);
		}
	}

	//
	// Win32 Public Timing
	//
	namespace timings {
		fpl_internal LARGE_INTEGER Win32GetWallClock_Internal(void) {
			LARGE_INTEGER result;
			QueryPerformanceCounter(&result);
			return(result);
		}
		fpl_api double GetHighResolutionTimeInSeconds() {
			LARGE_INTEGER clock = Win32GetWallClock_Internal();
			double result = clock.QuadPart / (double)fpl_GlobalWin32State_Internal.performanceFrequency.QuadPart;
			return(result);
		}
	};

	//
	// Win32 Public String
	//
	namespace strings {
		fpl_api void WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
			uint32_t requiredSize = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
			uint32_t requiredLen = requiredSize / sizeof(char);
			FPL_ASSERT(maxAnsiDestLen >= (requiredLen + 1));
			WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, NULL, NULL);
			ansiDest[requiredLen] = 0;
		}
		fpl_api void WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
			uint32_t requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
			uint32_t requiredLen = requiredSize / sizeof(char);
			FPL_ASSERT(maxUtf8DestLen >= (requiredSize + 1));
			WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, NULL, NULL);
			utf8Dest[requiredLen] = 0;
		}
		fpl_api void AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			uint32_t requiredSize = MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, NULL, 0);
			uint32_t requiredLen = requiredSize / sizeof(wchar_t);
			FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
			MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
		}
		fpl_api void UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			uint32_t requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, NULL, 0);
			uint32_t requiredLen = requiredSize / sizeof(wchar_t);
			FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
			MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
		}
	}

	//
	// Win32 Public Core
	//
	namespace library {
		fpl_api DynamicLibraryHandle LoadDynamicLibrary(const char *libraryFilePath) {
			FPL_ASSERT(libraryFilePath != NULL);
			DynamicLibraryHandle result = { 0 };
			HMODULE libModule = LoadLibraryA(libraryFilePath);
			if (libModule != nullptr) {
				result.internalHandle = (void *)libModule;
				result.isValid = true;
			}
			return(result);
		}
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle *handle, const char *name) {
			FPL_ASSERT(handle != NULL);
			void *result = nullptr;
			if (handle->isValid) {
				FPL_ASSERT(handle->internalHandle != nullptr);
				HMODULE libModule = (HMODULE)handle;
				result = (void *)GetProcAddress(libModule, name);
			}
			return(result);
		}
		fpl_api void ReleaseDynamicLibrary(DynamicLibraryHandle *handle) {
			FPL_ASSERT(handle != NULL);
			if (handle->isValid) {
				FPL_ASSERT(handle->internalHandle != nullptr);
				HMODULE libModule = (HMODULE)handle->internalHandle;
				FreeLibrary(libModule);
			}
			*handle = {};
		}
	}

	//
	// Win32 Public Window
	//
#	if FPL_ENABLE_WINDOW
	namespace window {

#		if FPL_ENABLE_OPENGL
		fpl_api void WindowFlip(void) {
			SwapBuffers(fpl_GlobalWin32State_Internal.window.deviceContext);
		}
#		else
		fpl_api void WindowFlip(void) {
		}
#		endif // FPL_ENABLE_OPENGL

		fpl_api WindowSize GetWindowArea(void) {
			WindowSize result = { 0 };
			RECT windowRect;
			if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
				result.width = windowRect.right - windowRect.left;
				result.height = windowRect.bottom - windowRect.top;
			}
			return(result);
		}

		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height) {
			RECT clientRect, windowRect;
			if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &clientRect) && GetWindowRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
				int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
				int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
				int newWidth = width + borderWidth;
				int newHeight = height + borderHeight;
				SetWindowPos(fpl_GlobalWin32State_Internal.window.windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

		fpl_api bool32 IsWindowResizable(void) {
			LONG style = GetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE);
			bool32 result = style & WS_THICKFRAME;
			return(result);
		}

		fpl_api void SetWindowResizeable(const bool32 value) {
			LONG style = GetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE);
			if (value) {
				style |= WS_THICKFRAME;
			} else {
				style |= ~WS_THICKFRAME;
			}
			SetWindowLong(fpl_GlobalWin32State_Internal.window.windowHandle, GWL_STYLE, style);
		}

		fpl_api WindowPosition GetWindowPosition(void) {
			WindowPosition result = { 0 };
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

		fpl_api void SetWindowPosition(const int32_t left, const int32_t top) {
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

		fpl_api void SetWindowCursorEnabled(const bool32 value) {
			fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
			FPL_ASSERT(win32State != NULL);
			win32State->window.isCursorActive = value;
		}

		fpl_api bool32 WindowUpdate(void) {
			bool32 result = false;
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

		fpl_api bool32 IsWindowRunning(void) {
			bool32 result = fpl_GlobalWin32State_Internal.window.isRunning;
			return(result);
		}
	}

	fpl_internal void fpl_Win32PushMouseEvent_Internal(const window::MouseEventType &mouseEventType, const window::MouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
		using namespace window;

		Event newEvent = {};
		newEvent.type = EventType::Mouse;
		newEvent.mouse.type = mouseEventType;
		newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
		newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
		newEvent.mouse.mouseButton = mouseButton;
		if (mouseEventType == MouseEventType::Wheel) {
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
		}
		PushEvent_Internal(newEvent);
	}

	fpl_internal window::Key fpl_Win32MapVirtualKey_Internal(const uint64_t keyCode) {
		using namespace window;
		switch (keyCode) {
			case VK_BACK:
				return Key::Key_Backspace;
			case VK_TAB:
				return Key::Key_Tab;

			case VK_CLEAR:
				return Key::Key_Clear;
			case VK_RETURN:
				return Key::Key_Enter;

			case VK_SHIFT:
				return Key::Key_Shift;
			case VK_CONTROL:
				return Key::Key_Control;
			case VK_MENU:
				return Key::Key_Alt;
			case VK_PAUSE:
				return Key::Key_Pause;
			case VK_CAPITAL:
				return Key::Key_CapsLock;

			case VK_ESCAPE:
				return Key::Key_Escape;
			case VK_SPACE:
				return Key::Key_Space;
			case VK_PRIOR:
				return Key::Key_PageUp;
			case VK_NEXT:
				return Key::Key_PageDown;
			case VK_END:
				return Key::Key_End;
			case VK_HOME:
				return Key::Key_Home;
			case VK_LEFT:
				return Key::Key_Left;
			case VK_UP:
				return Key::Key_Up;
			case VK_RIGHT:
				return Key::Key_Right;
			case VK_DOWN:
				return Key::Key_Down;
			case VK_SELECT:
				return Key::Key_Select;
			case VK_PRINT:
				return Key::Key_Print;
			case VK_EXECUTE:
				return Key::Key_Execute;
			case VK_SNAPSHOT:
				return Key::Key_Snapshot;
			case VK_INSERT:
				return Key::Key_Insert;
			case VK_DELETE:
				return Key::Key_Delete;
			case VK_HELP:
				return Key::Key_Help;

			case 0x30:
				return Key::Key_0;
			case 0x31:
				return Key::Key_1;
			case 0x32:
				return Key::Key_2;
			case 0x33:
				return Key::Key_3;
			case 0x34:
				return Key::Key_4;
			case 0x35:
				return Key::Key_5;
			case 0x36:
				return Key::Key_6;
			case 0x37:
				return Key::Key_7;
			case 0x38:
				return Key::Key_8;
			case 0x39:
				return Key::Key_9;

			case 0x41:
				return Key::Key_A;
			case 0x42:
				return Key::Key_B;
			case 0x43:
				return Key::Key_C;
			case 0x44:
				return Key::Key_D;
			case 0x45:
				return Key::Key_E;
			case 0x46:
				return Key::Key_F;
			case 0x47:
				return Key::Key_G;
			case 0x48:
				return Key::Key_H;
			case 0x49:
				return Key::Key_I;
			case 0x4A:
				return Key::Key_J;
			case 0x4B:
				return Key::Key_K;
			case 0x4C:
				return Key::Key_L;
			case 0x4D:
				return Key::Key_M;
			case 0x4E:
				return Key::Key_N;
			case 0x4F:
				return Key::Key_O;
			case 0x50:
				return Key::Key_P;
			case 0x51:
				return Key::Key_Q;
			case 0x52:
				return Key::Key_R;
			case 0x53:
				return Key::Key_S;
			case 0x54:
				return Key::Key_T;
			case 0x55:
				return Key::Key_U;
			case 0x56:
				return Key::Key_V;
			case 0x57:
				return Key::Key_W;
			case 0x58:
				return Key::Key_X;
			case 0x59:
				return Key::Key_Y;
			case 0x5A:
				return Key::Key_Z;

			case VK_LWIN:
				return Key::Key_LeftWin;
			case VK_RWIN:
				return Key::Key_RightWin;
			case VK_APPS:
				return Key::Key_Apps;

			case VK_SLEEP:
				return Key::Key_Sleep;
			case VK_NUMPAD0:
				return Key::Key_NumPad0;
			case VK_NUMPAD1:
				return Key::Key_NumPad1;
			case VK_NUMPAD2:
				return Key::Key_NumPad2;
			case VK_NUMPAD3:
				return Key::Key_NumPad3;
			case VK_NUMPAD4:
				return Key::Key_NumPad4;
			case VK_NUMPAD5:
				return Key::Key_NumPad5;
			case VK_NUMPAD6:
				return Key::Key_NumPad6;
			case VK_NUMPAD7:
				return Key::Key_NumPad7;
			case VK_NUMPAD8:
				return Key::Key_NumPad8;
			case VK_NUMPAD9:
				return Key::Key_NumPad9;
			case VK_MULTIPLY:
				return Key::Key_Multiply;
			case VK_ADD:
				return Key::Key_Add;
			case VK_SEPARATOR:
				return Key::Key_Separator;
			case VK_SUBTRACT:
				return Key::Key_Substract;
			case VK_DECIMAL:
				return Key::Key_Decimal;
			case VK_DIVIDE:
				return Key::Key_Divide;
			case VK_F1:
				return Key::Key_F1;
			case VK_F2:
				return Key::Key_F2;
			case VK_F3:
				return Key::Key_F3;
			case VK_F4:
				return Key::Key_F4;
			case VK_F5:
				return Key::Key_F5;
			case VK_F6:
				return Key::Key_F6;
			case VK_F7:
				return Key::Key_F7;
			case VK_F8:
				return Key::Key_F8;
			case VK_F9:
				return Key::Key_F9;
			case VK_F10:
				return Key::Key_F10;
			case VK_F11:
				return Key::Key_F11;
			case VK_F12:
				return Key::Key_F12;
			case VK_F13:
				return Key::Key_F13;
			case VK_F14:
				return Key::Key_F14;
			case VK_F15:
				return Key::Key_F15;
			case VK_F16:
				return Key::Key_F16;
			case VK_F17:
				return Key::Key_F17;
			case VK_F18:
				return Key::Key_F18;
			case VK_F19:
				return Key::Key_F19;
			case VK_F20:
				return Key::Key_F20;
			case VK_F21:
				return Key::Key_F21;
			case VK_F22:
				return Key::Key_F22;
			case VK_F23:
				return Key::Key_F23;
			case VK_F24:
				return Key::Key_F24;

			case VK_LSHIFT:
				return Key::Key_LeftShift;
			case VK_RSHIFT:
				return Key::Key_RightShift;
			case VK_LCONTROL:
				return Key::Key_LeftControl;
			case VK_RCONTROL:
				return Key::Key_RightControl;
			case VK_LMENU:
				return Key::Key_LeftAlt;
			case VK_RMENU:
				return Key::Key_RightAlt;

			default:
				return Key::Key_None;
		}
	}

	fpl_internal void fpl_Win32PushKeyboardEvent_Internal(const window::KeyboardEventType keyboardEventType, const uint64_t keyCode, const window::KeyboardModifierFlags modifiers, const bool32 isDown) {
		using namespace window;
		Event newEvent = {};
		newEvent.type = EventType::Keyboard;
		newEvent.keyboard.keyCode = keyCode;
		newEvent.keyboard.mappedKey = fpl_Win32MapVirtualKey_Internal(keyCode);
		newEvent.keyboard.type = keyboardEventType;
		newEvent.keyboard.modifiers = modifiers;
		PushEvent_Internal(newEvent);
	}

	fpl_internal bool32 fpl_Win32IsKeyDown_Internal(const uint64_t keyCode) {
		bool32 result = GetAsyncKeyState((int)keyCode) & 0x8000;
		return(result);
	}

	LRESULT CALLBACK fpl_Win32MessageProc_Internal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		using namespace window;

		LRESULT result = 0;
		fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
		FPL_ASSERT(win32State != NULL);
		switch (msg) {
			case WM_DESTROY:
			case WM_CLOSE:
			{
				win32State->window.isRunning = false;
			} break;

			case WM_SIZE:
			{
				Event newEvent = {};
				newEvent.type = EventType::Window;
				newEvent.window.type = WindowEventType::Resized;
				newEvent.window.width = LOWORD(lParam);
				newEvent.window.height = HIWORD(lParam);
				PushEvent_Internal(newEvent);
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				uint64_t keyCode = wParam;
				bool32 wasDown = ((lParam & (1 << 30)) != 0);
				bool32 isDown = ((lParam & (1 << 31)) == 0);

				bool32 altKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_MENU);
				bool32 shiftKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LSHIFT);
				bool32 ctrlKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LCONTROL);
				bool32 superKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LMENU);

				KeyboardEventType keyEventType = isDown ? KeyboardEventType::KeyDown : KeyboardEventType::KeyUp;
				KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
				if (altKeyWasDown) {
					modifiers |= KeyboardModifierFlags::Alt;
				}
				if (shiftKeyWasDown) {
					modifiers |= KeyboardModifierFlags::Shift;
				}
				if (ctrlKeyWasDown) {
					modifiers |= KeyboardModifierFlags::Ctrl;
				}
				if (superKeyWasDown) {
					modifiers |= KeyboardModifierFlags::Super;
				}
				fpl_Win32PushKeyboardEvent_Internal(keyEventType, keyCode, modifiers, isDown);

				if (wasDown != isDown) {
					if (isDown) {
						if (keyCode == VK_F4 && altKeyWasDown) {
							win32State->window.isRunning = 0;
						}
					}
				}

				result = 0;
			} break;

			case WM_CHAR:
			{
				uint64_t keyCode = wParam;
				KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
				fpl_Win32PushKeyboardEvent_Internal(KeyboardEventType::Char, keyCode, modifiers, 0);
				result = 1;
			} break;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				MouseEventType mouseEventType;
				if (msg == WM_LBUTTONDOWN) {
					mouseEventType = MouseEventType::ButtonDown;
				} else {
					mouseEventType = MouseEventType::ButtonUp;
				}
				fpl_Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Left, lParam, wParam);
				result = 1;
			} break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			{
				MouseEventType mouseEventType;
				if (msg == WM_RBUTTONDOWN) {
					mouseEventType = MouseEventType::ButtonDown;
				} else {
					mouseEventType = MouseEventType::ButtonUp;
				}
				fpl_Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Right, lParam, wParam);
				result = 1;
			} break;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				MouseEventType mouseEventType;
				if (msg == WM_MBUTTONDOWN) {
					mouseEventType = MouseEventType::ButtonDown;
				} else {
					mouseEventType = MouseEventType::ButtonUp;
				}
				fpl_Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Middle, lParam, wParam);
				result = 1;
			} break;
			case WM_MOUSEMOVE:
			{
				fpl_Win32PushMouseEvent_Internal(MouseEventType::Move, MouseButtonType::None, lParam, wParam);
				result = 1;
			} break;
			case WM_MOUSEWHEEL:
			{
				fpl_Win32PushMouseEvent_Internal(MouseEventType::Wheel, MouseButtonType::None, lParam, wParam);
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

#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
	fpl_internal bool32 fpl_Win32CreateOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
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
			return false;
		}
		if (!SetPixelFormat(win32State->window.deviceContext, pixelFormat, &pfd)) {
			// @TODO(final): Log error
			return false;
		}

		win32State->opengl.renderingContext = wglCreateContext(win32State->window.deviceContext);
		if (!win32State->opengl.renderingContext) {
			// @TODO(final): Log error
			return false;
		}

		if (!wglMakeCurrent(win32State->window.deviceContext, win32State->opengl.renderingContext)) {
			// @TODO(final): Log error
			return false;
		}

		return true;
	}

	fpl_internal void fpl_Win32ReleaseOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
		if (win32State->opengl.renderingContext) {
			wglMakeCurrent(0, 0);
			wglDeleteContext(win32State->opengl.renderingContext);
			win32State->opengl.renderingContext = NULL;
		}
	}
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

#	if FPL_ENABLE_WINDOW
	fpl_internal bool32 fpl_Win32InitWindow_Internal(fpl_Win32State_Internal *win32State, const InitFlags initFlags) {
		using namespace memory;

		// Register window class
		WNDCLASSEX windowClass = {};
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
			return false;
		}
		fpl_Win32StringCopy(windowClass.lpszClassName, fpl_Win32GetStringLength(windowClass.lpszClassName), win32State->window.windowClass, FPL_ARRAYCOUNT(win32State->window.windowClass));

		// Create window
		win32State->window.windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, windowClass.lpszClassName, WIN32_UNNAMED_WINDOW, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FPL_DEFAULT_WINDOW_WIDTH, FPL_DEFAULT_WINDOW_HEIGHT, NULL, NULL, windowClass.hInstance, NULL);
		if (win32State->window.windowHandle == NULL) {
			// @TODO(final): Log error
			return false;
		}

		// Get device context so we can swap the back and front buffer
		win32State->window.deviceContext = GetDC(win32State->window.windowHandle);
		if (win32State->window.deviceContext == NULL) {
			// @TODO(final): Log error
			return false;
		}

		// Create opengl rendering context if required
		if (initFlags & InitFlags::VideoOpenGL) {
			bool32 openglResult = fpl_Win32CreateOpenGLContext_Internal(win32State);
			if (!openglResult) {
				// @TODO(final): Log error
				return false;
			}
		}

		void *eventQueueMemory = AllocateAlignedMemory(sizeof(fpl_EventQueue_Internal), 16);
		fpl_GlobalEventQueue_Internal = (fpl_EventQueue_Internal *)eventQueueMemory;

		// Show window
		ShowWindow(win32State->window.windowHandle, SW_SHOW);
		UpdateWindow(win32State->window.windowHandle);

		// Cursor is visible at start
		win32State->window.defaultCursor = windowClass.hCursor;
		win32State->window.isCursorActive = true;
		win32State->window.isRunning = true;

		return true;
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

		memory::FreeAlignedMemory(fpl_GlobalEventQueue_Internal);
	}
#	endif // FPL_ENABLE_WINDOW

	fpl_api bool32 InitPlatform(const InitFlags initFlags) {
		fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
		FPL_ASSERT(win32State != NULL);
		FPL_ASSERT(!win32State->isInitialized);

		// Timing
		QueryPerformanceFrequency(&win32State->performanceFrequency);

#if FPL_ENABLE_WINDOW
		InitFlags usedInitFlags = initFlags;

#	if FPL_ENABLE_OPENGL
		if (usedInitFlags & InitFlags::VideoOpenGL) {
			usedInitFlags |= InitFlags::Window;
		}
#	endif

		if (usedInitFlags & InitFlags::Window) {
			if (!fpl_Win32InitWindow_Internal(win32State, usedInitFlags)) {
				return false;
			}
		}
#endif // FPL_ENABLE_WINDOW

		win32State->isInitialized = true;

		return (true);
	}

	fpl_api void ReleasePlatform(void) {
		fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
		FPL_ASSERT(win32State != NULL);
		FPL_ASSERT(win32State->isInitialized);
#if FPL_ENABLE_WINDOW
#	if FPL_ENABLE_OPENGL
		fpl_Win32ReleaseOpenGLContext_Internal(win32State);
#	endif
		fpl_Win32ReleaseWindow_Internal(win32State);
#endif
		win32State->isInitialized = false;
	}
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
	fpl::fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#		else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl::fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#		endif // defined(UNICODE)
#	endif // FPL_ENABLE_WINDOW

#endif // defined(FPL_PLATFORM_WINDOWS)

#endif // defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)