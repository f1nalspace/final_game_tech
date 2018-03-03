/*
final_platform_layer.hpp

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A Open source C++ single file header platform abstraction layer library.

Final Platform Layer is a cross-platform single-header-file development library designed to abstract the underlying platform to a very simple and easy to use low-level api for accessing input devices (keyboard, mouse, gamepad), audio playback, window handling, IO handling (files, directories, paths), multithreading (threads, mutex, signals) and graphics software or hardware rendering initialization.
The main focus is game/simulation development, so the default settings will create a window, setup a opengl rendering context and initialize audio playback on any platform.
The only dependencies are built-in operating system libraries, a C++/11 compiler and the C runtime library.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into any C++ projects you want and include it in any place you want.
- In your main translation unit provide the typical main() entry point.
- Define FPL_IMPLEMENTATION before including this header file in at least one translation unit.
- Init the platform.
- Use the features you want.
- Release the platform when you are done.

-------------------------------------------------------------------------------
	Usage: Hello world console application
-------------------------------------------------------------------------------

#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

int main(int argc, char **args){
	if (fpl::InitPlatform(fpl::InitFlags::None)) {
		fpl::console::ConsoleOut("Hello World!\n");
		fpl::ReleasePlatform();
		return 0;
	} else {
		return -1;
	}
}

-------------------------------------------------------------------------------
	Usage: OpenGL legacy or modern application
-------------------------------------------------------------------------------

#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

int main(int argc, char **args){
	fpl::Settings settings = DefaultSettings();
	fpl::VideoSettings &videoSettings = settings.video;

	videoSettings.driver = fpl::VideoDriverType::OpenGL;

	// Legacy OpenGL
	videoSettings.opengl.compabilityFlags = fpl::OpenGLCompabilityFlags::Legacy;

	// or

	// Modern OpenGL
	videoSettings.opengl.compabilityFlags = fpl::OpenGLCompabilityFlags::Core;
	videoSettings.opengl.majorVersion = 3;
	videoSettings.opengl.minorVersion = 3;

	if (fpl::InitPlatform(fpl::InitFlags::Video, settings)) {
		// Event/Main loop
		while (fpl::window::WindowUpdate()) {
			// Handle actual window events
			fpl::window::Event ev;
			while (fpl::window::PollWindowEvent(ev)) {
				/// ...
			}

			// your code goes here

			fpl::video::VideoFlip();
		}
		fpl::ReleasePlatform();
		return 0;
	} else {
		return -1;
	}

}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Platform Layer is released under the following license:

MIT License

Copyright (c) 2017-2018 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*!
	\file final_platform_layer.hpp
	\version v0.6.0.0 beta
	\author Torsten Spaete
	\brief Final Platform Layer (FPL) - A Open source C++ single file header platform abstraction layer library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.6.0.0 beta:
	- Changed: Documentation changed a bit
	- Changed: Renamed SignalWakeUp() to SignalSet()
	- Changed: [Win32] Start thread always immediatly
	- Changed: Included "Windows" prefix for output path in visual studio projects
	- Changed: Moved opengl settings into graphics union
	- Changed: All global functions use :: as a prefix
	- Deleted: Removed ThreadSuspend() -> Not complaint with pthread
	- Deleted: Removed ThreadResume() -> Not complaint with pthread
	- Deleted: Removed SignalReset() -> Not complaint with pthread
	- Fixed: Resize video backbuffer was not working anymore
	- Fixed: [Win32] Some _Interlocked* functions was not existing on x86
	- New: Introduced PrepareWindowForVideoAfter() so we can setup a video context after the window has been created
	- New: Introduced PrepareWindowForVideoBefore() so we can setup any window before initializing the video context
	- New: Added timings::GetTimeInMilliseconds() with [Win32] and [POSIX] implementation
	- New: [POSIX] Implemented all threading functions
	- New: [Linux] Added a makefile for each demo project
	- New: Added files::FileMove

	## v0.5.9.1 beta:
	- Changed: MemoryInfos uses uint64_t instead of size_t
	- Changed: Added BSD to platform detecton
	- Changed: Architecture detection does not use _WIN64 or _WIN32 anymore
	- Changed: fpl_api is now either fpl_platform_api or fpl_common_api
	- Changed: Internal code refactoring
	- Changed: Renamed VideoSettings driverType to driver
	- Changed: Video initialization is now platform independent
	- Changed: Window initialization is now platform independent
	- Changed: Moved window::WindowFlip to video::VideoFlip
	- Changed: [POSIX] Replaced file-io with POSIX open, read, write
	- Changed: [Win32] Removed fpl_api from main entry point forward declararation
	- Changed: [Win32] Moved main entry point inside the implementation block
	- Fixed: Arm64 was misdetected as X64, this is now its own arch
	- Fixed: GNUC and ICC are pure C-compilers, it makes no sense to detect such in a C++ library
	- Fixed: [Linux][POSIX] Refactor init and release to match PlatformInitState and PlatformAppState
	- Updated: Documentations updated
	- New: [POSIX][X11][Linux] Added missing functions as not-implemented

	## v0.5.9.0 beta:
	- Changed: Moved documentation inside its own file "final_platform_layer.documentation"
	- Changed: [Win32] Window creation uses CS_OWNDC always
	- Changed: Refactoring of platform and non-platform code
	- Changed: InitPlatform() and ReleasePlatform() is now platform independent

	## v0.5.8.1 beta:
	- Changed: KeyboardEventType::Char renamed to KeyboardEventType::CharInput
	- Changed: Completed basic documentation
	- Changed: Renamed VideoBackBuffer.stride to VideoBackBuffer.lineWidth
	- Changed: Moved a few internal inline audio functions to the global audio namespace
	- Fixed: [Win32] WM_CHAR was allowing unicode characters as well, which is wrong because it must be properly converted first.
	- Fixed: Audio driver selection was buggy
	- Added: pixelStride to fpl::video::VideoBackBuffer
	- Added: fpl::window::ClearWindowEvents()
	- Added: fpl::window::PushWindowEvent()
	- Added: fpl::window::UpdateGameControllers()
	- Added: fpl::audio::GetAudioBufferSizeInFrames()
	- Added: fpl::audio::GetAudioDriverString()
	- Added: fpl::audio::GetAudioFormatString()
	- Added: fpl::audio::GetAudioSampleSizeInBytes()
	- Added: fpl::audio::GetAudioFrameSizeInBytes()
	- Added: fpl::audio::GetAudioBufferSizeInBytes()

	## v0.5.8.0 beta:
	- Changed: SignalWaitFor* requires additional parameter for passing in the ThreadMutex reference (pthread compability)
	- Changed: Signal* does not use const reference anymore (pthread compability)
	- Changed: Updated documentation a ton
	- Changed: Decreased MAX_ERRORSTATE_COUNT from 1024 to 256
	- Changed: global__LastErrorState is a non-pointer global now
	- Changed: Renamed GetPlatformLastError() to GetPlatformError()
	- Changed: All array of objects parameters uses C++ array style -> int *arr[] instead of int **arr
	- Fixed: MemoryAlignedFree() had wrong signature (void **) instead of (void *)
	- Fixed: GetPlatformErrorCount() was not increasing when an empty error was pushed on for single error states
	- Fixed: [Win32] InitPlatform() was not cleaning up the Win32State when the initialization failed
	- Replaced: VideoCompabilityProfile is replaced by OpenGLCompabilityFlags (Only available with OpenGL)
	- New: Added ClearPlatformErrors()

	## v0.5.7.4 beta:
	- Changed: Updated code documentation for all functions and types to doxygen/javadoc style
	- Changed: SetFilePosition32 position is now int32_t instead of uint32_t to support negative positions as well
	- Changed: Renamed desiredFormat to deviceFormat in AudioSettings
	- Changed: [DirectSound] Use deviceID as GUID for the audio device
	- New: Introduced AudioDeviceID
	- New: Added deviceID field in AudioSettings
	- New: Added audio::GetAudioDevices()
	- New: Added strings::FormatString()

	## v0.5.7.3 beta:
	- Fixed: [Win32] Fixed SetWindowFullscreen was not working properly
	- New: Introduced outputRect in VideoBackBuffer + Win32 implementation
	- Changed: SetWindowFullscreen returns bool

	## v0.5.7.2 beta:
	- Added: Added new audio formats (AudioFormatType::F64, AudioFormatType::S64)

	## v0.5.7.1 beta:
	- Fixed: xInputSetState renamed to xInputGetState internally
	- Added: Introduced InputSettings
	- Added: [Win32] XInput controller detection is now limited to a fixed frequency, for improving performance (InputSettings.controllerDetectionFrequency)

	## v0.5.7.0 beta:
	- Changed: Total change of documentation style
	- Changed: [Win32] ThreadMutex uses a critical section instead of a event
	- Changed: [Win32] Include windows.h in the header, so we can define HANDLE, CRITICAL_SECTION in the api
	- Changed: [Win32] Changed lots of functions to use conditions instead of asserts
	- Changed: [Win32] Changed format specifiers to use either %d or %zu for integer types
	- Changed: All Thread*Wait functions returns if the wait was successful or not in the same way Signal*Wait
	- Changed: All ListFiles* functions uses reference instead of pointer

	## v0.5.6.0 beta:
	- Changed: We are back to C++/11 and we will never going back to C++/98

	## v0.5.5.1 beta:
	- New[POSIX]: Implemented fpl::timings
	- New[POSIX]: Implemented fpl::library
	- New[POSIX]: Implemented fpl::threading::ThreadSleep
	- Changed[POSIX]: Moved Linux fpl::console functions to Posix

	## v0.5.5.0 beta:
	- Changed: All internal handles are now unions now, so can have different sizes of handles
	- Changed: Introduced POSIX platform and moved linux atomics into it

	## v0.5.4.0 beta:
	- Fixed: Some enum types was not using the namespace version

	## v0.5.3.0 beta:
	- Changed: Use custom int types because C++/98 has no default types unfortunatly
	- Fixed: Changed compiler detection order, because some non-MSVC compilers define _MSVC
	- Changed: Better C++/11 feature detection for optional nullptr and constexpr

	## v0.5.2.1 beta:
	- Fixed: Corrected all doxygen statements to match new enum style or added missing exlamation marks.

	## v0.5.2 beta:
	- Changed: Library is now C++/98 complaint
	- Changed: The actual enum type for "flags" has no "s" at the end anymore.
	- Opimization: Changed some internal functions to "static inline"
	- Changed: Renamed audio::GetAudioNativeFormat to audio::GetAudioHardwareFormat
	- New: Added "periods", "bufferSizeInBytes", "bufferSizeInFrames" to AudioDeviceFormat

	## v0.5.1 beta:
	- New: audio::GetAudioNativeFormat()
	- New: audio::SetAudioClientReadCallback()
	- Fixed: InitFlags::Audio was never tested before InitAudio() was being called.
	- Changed: Renamed ThreadStop to ThreadDestroy

	## v0.5.0 beta:
	- Added: [Win32] DirectSound playback support
	- Added: Asyncronous audio playback

	## v0.4.11 alpha:
	- Fixed: [Win32] For now, load all user32 functions always, even when window is not used (This is to prepare for audio playback)
	- Fixed: [Win32] ThreadStop was not releasing the thread handle
	- Added: [Win32] ThreadWaitForAny
	- Added: [Win32] SignalWaitForAll
	- Added: [Win32] SignalWaitForAny
	- Added: [Win32] SignalReset
	- Added: FPL_NO_VIDEO
	- Changed: ThreadWaitForSingle renamed to ThreadWaitForOne
	- Changed: ThreadWaitForMultiple renamed to ThreadWaitForAll
	- Changed: SignalWait renamed to SignalWaitForOne

	## v0.4.10 alpha:
	- Removed: Removed all _internal _INTERNAL postfixes from types, functions and macros
	- Changed: Proper identitation for compiler directives based on context
	- Added: [Win32] Dynamically loading ole32 functions (CoCreateInstance, CoInitializeEx, etc.)
	- Fixed: [Win32] GetCursor was not using the dynamic loaded function
	- Fixed: [Win32] Missing *Clipboard* dynamic functions

	## v0.4.9 alpha:
	- Removed: Removed all audio code for now
	- Changed: A total cleanup of all internal stuff, so its much easier to add in new features

	## v0.4.8 alpha:
	- New: AtomicLoadU32, AtomicLoadU64, AtomicLoadS32, AtomicLoadS64, AtomicLoadPtr
	- New: AtomicStoreU32, AtomicStoreU64, AtomicStoreS32, AtomicStoreS64, AtomicStorePtr
	- New: AtomicExchangePtr, AtomicCompareAndExchangePtr, IsAtomicCompareAndExchangePtr
	- New: [Win32] Implementation for AtomicLoadU32, AtomicLoadU64, AtomicLoadS32, AtomicLoadS64, AtomicLoadPtr
	- New: [Win32] Implementation for AtomicStoreU32, AtomicStoreU64, AtomicStoreS32, AtomicStoreS64, AtomicStorePtr
	- New: [Linux] Implementation for AtomicLoadU32, AtomicLoadU64, AtomicLoadS32, AtomicLoadS64, AtomicLoadPtr
	- New: [Linux] Implementation for AtomicStoreU32, AtomicStoreU64, AtomicStoreS32, AtomicStoreS64, AtomicStorePtr
	- New: [Win32] Loading of DirectSound (Prepare for audio output support)
	- Draft: Added first audio output api
	- Fixed: Threading context determination
	- Fixed: [Win32] Fixed all thread implementations
	- Fixed: [Win32] SetWindowLongPtrA does not exists on X86
	- Fixed: [Win32] Missing call convention in SHGetFolderPathA and SHGetFolderPathW
	- Changed: Improved header documentation (More examples, better descriptions, proper markdown syntax, etc.)
	- Changed: All threading functions uses pointer instead of reference
	- Changed: [Linux] Atomic* uses __sync instead of __atomic
	- Changed: A bit of internal cleanup

	## v0.4.7 alpha:
	- Changed: [Win32] Load all user32 and shell32 functions dynamically
	- Changed: FPL_ENUM_AS_FLAGS_OPERATORS_INTERNAL requires a int type as well
	- Fixed: MemoryAlignedAllocate and MemoryAlignedFree was broken
	- Added: FPL_IS_ALIGNED macro

	## v0.4.6 alpha:
	- Fixed: [Win32] Crash when window is not set in the InitFlags but FPL_USE_WINDOW is set.

	## v0.4.5 alpha:
	- Changed: [Win32] Use CommandLineToArgvW for command line parsing

	## v0.4.4 alpha:
	- New: [Win32] Implemented argument parsing for WinMain and wWinMain
	- Fixed: Corrected small things for doxygen
	- Changed: Renamed CopyAFile to FileCopy
	- Changed: Renamed DeleteAFile to FileDelete

	## v0.4.3 alpha:
	- New: Introduced IsAtomicCompareAndExchange
	- Added: [Linux] Implemented IsAtomicCompareAndExchange for all 32 and 64 bit integer types
	- Added: [Win32] Implemented IsAtomicCompareAndExchange for all 32 and 64 bit integer types
	- Added: [Win32] Loading gdi32.dll dynamically for ChoosePixelFormat, etc.
	- Added: [Win32] Loading opengl32.dll dynamically for wglGetProcAddress, wglMakeCurrent, etc.
	- Fixed: [Win32] Adding memory fence for AtomicReadWriteFence on non-x64 architectures
	- Fixed: [Win32] Adding memory fence for AtomicReadFence on non-x64 architectures
	- Fixed: [Win32] Adding memory fence for AtomicWriteFence on non-x64 architectures
	- Fixed: Solidified descriptions for all Atomic*Fence
	- Changed: Enabled FPL_FORCE_ASSERTIONS will ensure that C asserts are never used, because it may be compiled out.
	- Changed: Removed all FPL_WIN32_ kernel32 macros and replaced it with normal calls.
	- Changed: [Win32] Changed a lof ot the internals

	## v0.4.2 alpha:
	- Added: [Linux] Started linux implementation
	- Added: [Linux] Memory allocations
	- Added: [Linux] Atomic operations
	- Added: Check for C++/11 compiler and fail if not supported
	- Added: Nasty vstudio 2015+ workaround to detect C++/11
	- Added: &= operator overloading for enums
	- Changed: AtomicCompareAndExchange argument "comparand" and "exchange" flipped.
	- Changed: constexpr is now fpl_constant to make clear what is a constant
	- Removed: [Win32] CreateDIBSection is not needed for a software backbuffer
	- Fixed: [Win32] Software rendering was not working properly.
	- Fixed: Some AtomicCompareAndExchange signatures was still AtomicAndCompareExchange

	## v0.4.1 alpha:
	- Cleanup: Internal cleanup
	- Changed: All the settings constructors removed and replaced by a simple inline function.
	- Added: Added native C++ unit test project to demos solution
	- Fixed: FPL_OFFSETOF was not working
	- Fixed: All file size macros like FPL_MEGABYTES was returning invalid results.
	- Removed: FPL_PETABYTES and higher are removed, just because its useless.

	## v0.4.0 alpha:
	- Changed: All FPL_ENABLE_ defines are internal now, the caller must use FPL_NO_ or FPL_YES_ respectivily.
	- Changed: AtomicCompareExchange* is now AtomicCompareAndExchange*
	- Changed: InitFlags::VideoOpenGL is now InitFlags::Video
	- Added: Software rendering support
	- Added: VideoDriverType enumeration for selecting the active video driver
	- Added: video::GetVideoBackBuffer with [Win32] implementation
	- Added: video::ResizeVideoBackBuffer with [Win32] implementation
	- Added: FPL_PETABYTES macro
	- Added: FPL_EXABYTES macro
	- Added: FPL_ZETTABYTES macro
	- Added: FPL_YOTTABYTES macro
	- Added: FPL_MIN macro
	- Added: FPL_MAX macro
	- Added: MutexCreate with [Win32] implementation
	- Added: MutexDestroy with [Win32] implementation
	- Added: MutexLock with [Win32] implementation
	- Added: MutexUnlock with [Win32] implementation
	- Added: SignalCreate with [Win32] implementation
	- Added: SignalDestroy with [Win32] implementation
	- Added: SignalWait with [Win32] implementation
	- Added: SignalWakeUp with [Win32] implementation
	- Added: GetClipboardAnsiText with [Win32] implementation
	- Added: GetClipboardWideText with [Win32] implementation
	- Added: SetClipboardText with [Win32] implementation for ansi and wide strings
	- Added [MSVC]: AtomicExchangeS32 (Signed integer)
	- Added [MSVC]: AtomicExchangeS64 (Signed integer)
	- Added [MSVC]: AtomicAddS32 (Signed integer)
	- Added [MSVC]: AtomicAddS64 (Signed integer)
	- Added [MSVC]: AtomicCompareExchangeS32 (Signed integer)
	- Added [MSVC]: AtomicCompareExchangeS64 (Signed integer)
	- Fixed [MSVC]: AtomicExchangeU32 was not using unsigned intrinsic
	- Fixed [MSVC]: AtomicExchangeU64 was not using unsigned intrinsic
	- Fixed [MSVC]: AtomicAddU32 was not using unsigned intrinsic
	- Fixed [MSVC]: AtomicAddU64 was not using unsigned intrinsic
	- Fixed [MSVC]: AtomicCompareExchangeU32 was not using unsigned intrinsic
	- Fixed [MSVC]: AtomicCompareExchangeU64 was not using unsigned intrinsic
	- Implemented [Win32]: GetProcessorCoreCount
	- Implemented [Win32]: Main thread infos
	- Performance [Win32]: GetProcessorName (3 loop iterations at max)

	## v0.3.6 alpha:
	- Cleanup: All win32 functions are macro calls now (prepare for dynamic function loading)
	- Fixed: FPL_ENABLE_WINDOW was enabling window features even when it was deactivated by the caller

	## v0.3.5 alpha:
	- Renamed: All memory/library/threading functions
	- Removed: FPL_ENABLE_PUSHMEMORY removed entirely

	## v0.3.4 alpha:
	- Renamed: CopyFile/DeleteFile/All memory functions renamed (Stupid win32!)
	- Renamed: All internal opengl defines renamed, so that it wont conflict with other libraries
	- Fixed: [Win32] strings::All Wide conversions was not working properly
	- Removed: [Win32] Undefs for CopyFile
	- Changed: [Win32/OpenGL] Test for already included gl.h

	## v0.3.3 alpha:
	- Basic threading creation and handling
	- Fixed strings::All Wide convertions was not working properly

	## v0.3.2 alpha:
	- Introduced: Automatic namespace inclusion (FPL_AUTO_NAMESPACE)
	- Introduced: Push memory (FPL_ENABLE_PUSHMEMORY)
	- Signature changed for: ExtractFilePath/ChangeFileExtension (source first, destination second)
	- Window features not not compiled out anymore when FPL_ENABLE_WINDOW is 0
	- New overloaded CombinePath without any destination arguments
	- New: AllocateStackMemory function
	- Optional destination arguments for: GetExecutableFilePath/GetHomePath/ChangeFileExtension/CombinePath
	- Fixed strings::CopyAnsiString/CopyWideString was not returning the correct value

	## v0.3.1 alpha:
	- All types/structs/fields/functions documented
	- [Win32] Fixed legacy opengl (GL_INVALID_OPERATION)

	## v0.3.0 alpha:
	- Updated documentation a lot
	- [Win32] Support for WGL opengl profile selection

	## v0.2.6 alpha:
	- Added memory::CopyMemory
	- Added fpl::GetLastError and fpl::GetLastErrorCount for proper error handling
	- Added files::CreateBinaryFile and files::OpenBinaryFile for wide file paths
	- Added basic support for creating a modern opengl rendering context, see VideoCompabilityProfile in VideoSettings
	- Added support for enabling opengl vsync through WGL
	- Returns char * for all paths:: get like functions
	- Returns char/wchar_t * for all strings:: functions
	- Fixed files::CreateBinaryFile was never able to overwrite the file.
	- Fixed include was in some namespaces defined
	- Fixed files::ClearMemory was wrong
	- Replaced all const constants with fpl_constant
	- Removed template code / Replaced it with macros

	## v0.2.5 alpha:
	- Added CreateDirectories
	- Returns char * for all path get like functions
	- Fixed CreateBinaryFile was never able to overwrite the file.

	## v.2.4 alpha:
	- Changed to a doxygen + vc complaint documentation style
	- CopyFile2, DeleteFile2 and CloseFile2 are now CopyFile, DeleteFile, CloseFile

	## v0.2.3 alpha:
	- Support for doxygen in documentations

	## v0.2.2 alpha:
	- Added XInput support

	## v0.2.1 alpha:
	- Changed a lot of pointer arguments to reference
	- Added gamepad event structures

	## v0.2 alpha:
	- Dropped C support and moved to a more C++ ish api
	- Dropped no C-Runtime support

	## v0.1 alpha:
	- Initial version
*/

/*!
	\page page_todo ToDo / Planned (Random order)
	\tableofcontents

	- Make VideoBackBuffer be readonly and separate functions for getting the data pointer and setting the output rectangle.

	- Finish Linux Platform:
		- Files & Path (Look out for . files and folders!)
		- Window (X11)
		- Video opengl (GLX)
		- Video software

	- BSD Platform (POSIX, X11)

	- Audio:
		- Support for channel mapping
		- ALSA audio driver
		- WASAPI audio driver

	- Video:
		- Direct2D
		- Direct3D 9/10/11

	- Additional parameters for passing pointers instead of returning structs (Method overloading)

	- Documentation
		- Audio deviceID
		- Audio introduction (What is a frame, a sample, a buffer, how data is layed out, etc.)
		- Memory
		- Atomics
		- Multi-Threading
		- File-IO
		- Paths
		- Strings

	- DLL-Export support

	- Multimonitor-Support

	- Window
		- Custom icon
		- Custom window style (Border, Resizeable, etc.)

	- Unicode-Support for commandline arguments (Win32)

	- Unicode/UTF-8 Support for character input

	- Pack/Unpack functions (Handle endianess)

	- Networking (UDP, TCP)
		- [Win32] WinSock
		- [POSIX] Socket

	- Open/Save file/folder dialog
*/

// ****************************************************************************
//
// Header
//
// ****************************************************************************
#ifndef FPL_INCLUDE_HPP
#define FPL_INCLUDE_HPP

// C++/11 detection
#if !((defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(_MSC_VER) && _MSC_VER >= 1900))
#	error "You need a C++/11 compatible compiler for this library!"
#endif

//
// Platform detection
//
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
#if defined(_WIN32) || defined(_WIN64)
#	define FPL_PLATFORM_WIN32
#	define FPL_PLATFORM_NAME "Windows"
#elif defined(__linux__) || defined(__gnu_linux__)
#	define FPL_PLATFORM_LINUX
#	define FPL_PLATFORM_NAME "Linux"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__bsdi__)
#	define FPL_PLATFORM_BSD
#	define FPL_PLATFORM_NAME "BSD"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	error "Not implemented yet!"
#elif defined(unix) || defined(__unix) || defined(__unix__)
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "Unix"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	error "Not implemented yet!"
#else
#	error "This platform is not supported!"
#endif // FPL_PLATFORM

//
// Architecture detection (x86, x64)
// See: https://sourceforge.net/p/predef/wiki/Architectures/
//
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#	define FPL_ARCH_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#elif defined(__arm__) || defined(_M_ARM)
#	if defined(__aarch64__)
#		define FPL_ARCH_ARM64
#	else	
#		define FPL_ARCH_ARM32
#	endif
#else
#	error "This architecture is not supported!"
#endif // FPL_ARCH

//
// Build configuration and compilers
// See: http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
//
#if defined(__clang__)
	//! CLANG compiler detected
#	define FPL_COMPILER_CLANG
#elif defined(__llvm__)
	//! LLVM compiler detected
#	define FPL_COMPILER_LLVM
#elif defined(__INTEL_COMPILER)
	//! Intel compiler detected
#	define FPL_COMPILER_INTEL
#elif defined(__MINGW32__)
	//! MingW compiler detected
#	define FPL_COMPILER_MINGW
#elif defined(__GNUG__) && !defined(__clang__)
	//! GCC compiler detected
#	define FPL_COMPILER_GCC
#elif defined(_MSC_VER)
	//! Visual studio compiler detected
#	define FPL_COMPILER_MSVC
#else
	//! No compiler detected
#	define FPL_COMPILER_UNKNOWN
#endif // FPL_COMPILER

//
// Compiler depended settings and detections
//
#if defined(FPL_COMPILER_MSVC)
	//! Disable noexcept compiler warning for C++
#	pragma warning( disable : 4577 )
	//! Disable "switch statement contains 'default' but no 'case' labels" compiler warning for C++
#	pragma warning( disable : 4065 )

#	if defined(_DEBUG) || (!defined(NDEBUG))
		//! Debug mode detected
#		define FPL_ENABLE_DEBUG
#	else
		//! Non-debug (Release) mode detected
#		define FPL_ENABLE_RELEASE
#	endif
#else
	// @NOTE(final): Expect all other compilers to pass in FPL_DEBUG manually
#	if defined(FPL_DEBUG)
		//! Debug mode detected
#		define FPL_ENABLE_DEBUG
#	else
		//! Non-debug (Release) mode detected
#		define FPL_ENABLE_RELEASE
#	endif
#endif // FPL_COMPILER

//
// Options & Feature detection
//

// Assertions
#if !defined(FPL_NO_ASSERTIONS)
#	if !defined(FPL_FORCE_ASSERTIONS)
#		if defined(FPL_ENABLE_DEBUG)
			//! Enable Assertions in Debug Mode by default
#			define FPL_ENABLE_ASSERTIONS
#		endif
#	else
		//! Enable Assertions always
#		define FPL_ENABLE_ASSERTIONS
#	endif
#endif // !FPL_NO_ASSERTIONS
#if defined(FPL_ENABLE_ASSERTIONS)
#	if !defined(FPL_NO_C_ASSERT)
		//! Enable C-Runtime Assertions by default
#		define FPL_ENABLE_C_ASSERT
#	endif
#endif // FPL_ENABLE_ASSERTIONS

// Window
#if !defined(FPL_NO_WINDOW)
	//! Window support enabled by default
#	define FPL_SUPPORT_WINDOW
#endif

// Video
#if !defined(FPL_NO_VIDEO)
	//! Video support
#	define FPL_SUPPORT_VIDEO
#endif
#if defined(FPL_SUPPORT_VIDEO)
#	if !defined(FPL_NO_VIDEO_OPENGL)
		//! OpenGL support enabled by default
#		define FPL_SUPPORT_VIDEO_OPENGL
#	endif
#	if !defined(FPL_NO_VIDEO_SOFTWARE)
		//! Software rendering support enabled by default
#		define FPL_SUPPORT_VIDEO_SOFTWARE
#	endif
#endif // FPL_SUPPORT_VIDEO

// Audio
#if !defined(FPL_NO_AUDIO)
	//! Audio support
#	define FPL_SUPPORT_AUDIO
#endif
#if defined(FPL_SUPPORT_AUDIO)
#	if !defined(FPL_NO_AUDIO_DIRECTSOUND) && defined(FPL_PLATFORM_WIN32)
		//! DirectSound support is only available on Win32
#		define FPL_SUPPORT_AUDIO_DIRECTSOUND
#	endif
#endif // FPL_SUPPORT_AUDIO

// Remove video support when window is disabled
#if !defined(FPL_SUPPORT_WINDOW)
#	if defined(FPL_SUPPORT_VIDEO)
#		undef FPL_SUPPORT_VIDEO
#	endif
#	if defined(FPL_SUPPORT_VIDEO_OPENGL)
#		undef FPL_SUPPORT_VIDEO_OPENGL
#	endif
#	if defined(FPL_SUPPORT_VIDEO_SOFTWARE)
#		undef FPL_SUPPORT_VIDEO_SOFTWARE
#	endif
#endif // !FPL_SUPPORT_WINDOW

//
// Enable supports (FPL uses _ENABLE_ internally only)
//
#if defined(FPL_SUPPORT_WINDOW)
	//! Enable Window
#	define FPL_ENABLE_WINDOW
#endif
#if defined(FPL_SUPPORT_VIDEO)
	//! Enable Video
#	define FPL_ENABLE_VIDEO
#	if defined(FPL_SUPPORT_VIDEO_OPENGL)
		//! Enable OpenGL Video
#		define FPL_ENABLE_VIDEO_OPENGL
#	endif
#	if defined(FPL_SUPPORT_VIDEO_SOFTWARE)
		//! Enable Software Rendering Video
#		define FPL_ENABLE_VIDEO_SOFTWARE
#	endif
#endif // FPL_SUPPORT_VIDEO
#if defined(FPL_SUPPORT_AUDIO)
	//! Enable Audio
#	define FPL_ENABLE_AUDIO
#	if defined(FPL_SUPPORT_AUDIO_DIRECTSOUND)
		//! Enable DirectSound Audio
#		define FPL_ENABLE_AUDIO_DIRECTSOUND
#	endif
#endif // FPL_SUPPORT_AUDIO

#if !defined(FPL_NO_ERROR_IN_CONSOLE)
	//! Write errors in console
#	define FPL_ENABLE_ERROR_IN_CONSOLE
#endif
#if !defined(FPL_NO_MULTIPLE_ERRORSTATES)
	//! Allow multiple error states
#	define FPL_ENABLE_MULTIPLE_ERRORSTATES
#endif
#if defined(FPL_AUTO_NAMESPACE)
	//! Expand namespaces at the header end always
#	define FPL_ENABLE_AUTO_NAMESPACE
#endif

//
// Static/Inline/Extern/Internal
//
//! Global persistent variable
#define fpl_globalvar static
//! Local persistent variable
#define fpl_localvar static
//! Private/Internal function
#define fpl_internal static
//! Inline function
#define fpl_inline inline
//! Internal inlined function
#define fpl_internal_inline fpl_internal fpl_inline
//! Constant
#define fpl_constant constexpr

#if defined(FPL_API_AS_PRIVATE)
	//! Private api call
#	define fpl_api static
#else
	//! Public api call
#	define fpl_api extern
#endif // FPL_API_AS_PRIVATE

//! Platform api definition
#define fpl_platform_api fpl_api
//! Common api definition
#define fpl_common_api fpl_api
//! Main entry point api definition
#define fpl_main

//
// Assertions
//
#if defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_ENABLE_C_ASSERT) && !defined(FPL_FORCE_ASSERTIONS)
#		include <assert.h>
		//! Runtime assert (C Runtime)
#		define FPL_ASSERT(exp) assert(exp)
		//! Compile error assert (C Runtime)
#		define FPL_STATICASSERT(exp) static_assert(exp, "static_assert")
#	else
		//! Runtime assert
#		define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
		//! Compile error assert
#		define FPL_STATICASSERT_(exp, line) \
			int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#		define FPL_STATICASSERT(exp) \
			FPL_STATICASSERT_(exp, __LINE__)
#	endif // FPL_ENABLE_C_ASSERT
#else
	//! Runtime assertions disabled
#	define FPL_ASSERT(exp)
	//! Compile time assertions disabled
#	define FPL_STATICASSERT(exp)
#endif // FPL_ENABLE_ASSERTIONS

//! This will full-on crash when something is not implemented always.
#define FPL_NOT_IMPLEMENTED {*(int *)0 = 0xBAD;}

//
// Types & Limits
//
#include <stdint.h> // uint32_t, ...
#include <stddef.h> // size_t
#include <limits.h> // UINT32_MAX, ...

//
// Macro functions
//
//! Returns the element count from a static array,
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//! Returns the offset in bytes to a field in a structure
#define FPL_OFFSETOF(type, field) ((size_t)(&(((type*)(0))->field)))

//! Returns the smallest value
#define FPL_MIN(a, b) ((a) < (b)) ? (a) : (b)
//! Returns the biggest value
#define FPL_MAX(a, b) ((a) > (b)) ? (a) : (b)

//! Returns the number of bytes for the given kilobytes
#define FPL_KILOBYTES(value) (((value) * 1024ull))
//! Returns the number of bytes for the given megabytes
#define FPL_MEGABYTES(value) ((FPL_KILOBYTES(value) * 1024ull))
//! Returns the number of bytes for the given gigabytes
#define FPL_GIGABYTES(value) ((FPL_MEGABYTES(value) * 1024ull))
//! Returns the number of bytes for the given terabytes
#define FPL_TERABYTES(value) ((FPL_GIGABYTES(value) * 1024ull))

//! Returns true when the given pointer address is aligned to the given alignment
#define FPL_IS_ALIGNED(ptr, alignment) (((uintptr_t)(const void *)(ptr)) % (alignment) == 0)

//! Defines the operator overloads for a enum used as flags
#define FPL_ENUM_AS_FLAGS_OPERATORS(etype) \
		inline etype operator | (etype lhs, etype rhs) { \
			return (etype)(static_cast<int>(lhs) | static_cast<int>(rhs)); \
		} \
		inline bool operator & (const etype lhs, const etype rhs) { \
			return (static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs); \
		} \
		inline etype& operator |= (etype &lhs, etype rhs) { \
			lhs = (etype)(static_cast<int>(lhs) | static_cast<int>(rhs)); \
			return lhs; \
		} \
		inline etype& operator &= (etype &lhs, etype rhs) { \
			lhs = (etype)(static_cast<int>(lhs) & static_cast<int>(rhs)); \
			return lhs; \
		}

// ****************************************************************************
//
// Platform includes
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)
// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#	if !defined(NOMINMAX)
#		define NOMINMAX
#	endif
// @NOTE(final): For now we dont want any network, com or gdi stuff at all, maybe later who knows.
#	if !defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <windows.h>		// Win32 api, its unfortunate we have to include this in the header as well, but there are structures
#endif // FPL_PLATFORM_WIN32

#if defined(FPL_SUBPLATFORM_POSIX)
#	include <pthread.h> // pthread_t, pthread_mutex_, pthread_cond_, pthread_barrier_
#endif // FPL_SUBPLATFORM_POSIX

#if defined(FPL_SUBPLATFORM_X11)
#	include <X11/X.h> // XVisualInfo
#	include <X11/Xlib.h> // ???
#   undef None
#endif

// ****************************************************************************
//
// API Declaration
//
// ****************************************************************************

//! Core
namespace fpl {
	//! Atomic functions
	namespace atomics {
		/**
		  * \defgroup Atomics Atomic functions
		  * \brief Atomic functions, like AtomicCompareAndExchange, AtomicReadFence, etc.
		  * \{
		  */

		/**
		  * \brief Insert a memory read fence/barrier.
		  *
		  * This will complete previous reads before future reads and prevents the compiler from reordering memory reads across this fence.
		  */
		fpl_platform_api void AtomicReadFence();
		/**
		  * \brief Insert a memory write fence/barrier.
		  * This will complete previous writes before future writes and prevents the compiler from reordering memory writes across this fence.
		  */
		fpl_platform_api void AtomicWriteFence();
		/**
		  * \brief Insert a memory read/write fence/barrier.
		  * This will complete previous reads/writes before future reads/writes and prevents the compiler from reordering memory access across this fence.
		  */
		fpl_platform_api void AtomicReadWriteFence();

		/**
		  * \brief Replace a 32-bit unsigned integer with the given value atomically.
		  * Ensures that memory operations are completed in order.
		  * \param target The target value to write into.
		  * \param value The source value used for exchange.
		  * \return Returns the initial value before the replacement.
		  */
		fpl_platform_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
		/**
		  * \brief Replace a 64-bit unsigned integer with the given value atomically.
		  * Ensures that memory operations are completed in order.
		  * \param target The target value to write into.
		  * \param value The source value used for exchange.
		  * \return Returns the initial value before the replacement.
		  */
		fpl_platform_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
		/**
		  * \brief Replace a 32-bit signed integer with the given value atomically.
		  * Ensures that memory operations are completed in order.
		  * \param target The target value to write into.
		  * \param value The source value used for exchange.
		  * \return Returns the initial value before the replacement.
		  */
		fpl_platform_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value);
		/**
		  * \brief Replace a 64-bit signed integer with the given value atomically.
		  * Ensures that memory operations are completed in order.
		  * \param target The target value to write into.
		  * \param value The source value used for exchange.
		  * \return Returns the initial value before the replacement.
		  */
		fpl_platform_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value);
		/**
		  * \brief Replace a pointer with the given value atomically.
		  * Ensures that memory operations are completed in order.
		  * \param target The target value to write into.
		  * \param value The source value used for exchange.
		  * \return Returns the initial value before the replacement.
		  */
		fpl_common_api void *AtomicExchangePtr(volatile void **target, const void *value);

		/**
		  * \brief Adds a 32-bit unsigned integer to the value by the given addend atomically.
		  * Ensures that memory operations are completed in order.
		  * \param value The target value to append to.
		  * \param addend The value used for adding.
		  * \return Returns the initial value before the append.
		  */
		fpl_platform_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
		/**
		  * \brief Adds a 64-bit unsigned integer to the value by the given addend atomically.
		  * Ensures that memory operations are completed in order.
		  * \param value The target value to append to.
		  * \param addend The value used for adding.
		  * \return Returns the initial value before the append.
		  */
		fpl_platform_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
		/**
		  * \brief Adds a 32-bit signed integer to the value by the given addend atomically.
		  * Ensures that memory operations are completed in order.
		  * \param value The target value to append to.
		  * \param addend The value used for adding.
		  * \return Returns the initial value before the append.
		  */
		fpl_platform_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend);
		/**
		  * \brief Adds a 64-bit signed integer to the value by the given addend atomically.
		  * Ensures that memory operations are completed in order.
		  * \param value The target value to append to.
		  * \param addend The value used for adding.
		  * \return Returns the initial value before the append.
		  */
		fpl_platform_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend);

		/**
		  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \note Use \ref IsAtomicCompareAndExchangeU32() when you want to check if the exchange has happened or not.
		  * \return Returns the dest before the exchange, regardless of the result.
		  */
		fpl_platform_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
		/**
		  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \note Use \ref IsAtomicCompareAndExchangeU64() when you want to check if the exchange has happened or not.
		  * \return Returns the value of the destination before the exchange, regardless of the result.
		  */
		fpl_platform_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
		/**
		  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \note Use \ref IsAtomicCompareAndExchangeS32() when you want to check if the exchange has happened or not.
		  * \return Returns the value of the destination before the exchange, regardless of the result.
		  */
		fpl_platform_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
		/**
		  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \note Use \ref IsAtomicCompareAndExchangeS64() when you want to check if the exchange has happened or not.
		  * \return Returns the value of the destination before the exchange, regardless of the result.
		  */
		fpl_platform_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
		/**
		  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \note Use \ref IsAtomicCompareAndExchangePtr() when you want to check if the exchange has happened or not.
		  * \return Returns the value of the destination before the exchange, regardless of the result.
		  */
		fpl_common_api void *AtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

		/**
		  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \return Returns true when the exchange happened, otherwise false.
		  */
		fpl_platform_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
		/**
		  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \return Returns true when the exchange happened, otherwise false.
		  */
		fpl_platform_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
		/**
		  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \return Returns true when the exchange happened, otherwise false.
		  */
		fpl_platform_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
		/**
		  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \return Returns true when the exchange happened, otherwise false.
		  */
		fpl_platform_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
		/**
		  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
		  * Ensures that memory operations are completed in order.
		  * \param dest The target value to write into.
		  * \param comparand The value to compare with.
		  * \param exchange The value to exchange with.
		  * \return Returns true when the exchange happened, otherwise false.
		  */
		fpl_common_api bool IsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

		/**
		  * \brief Loads the 32-bit unsigned value atomically and returns the value.
		  * Ensures that memory operations are completed before the read.
		  * \param source The source value to read from.
		  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
		  * \return Returns the source value.
		  */
		fpl_platform_api uint32_t AtomicLoadU32(volatile uint32_t *source);
		/**
		  * \brief Loads the 64-bit unsigned value atomically and returns the value.
		  * Ensures that memory operations are completed before the read.
		  * \param source The source value to read from.
		  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
		  * \return Returns the source value.
		  */
		fpl_platform_api uint64_t AtomicLoadU64(volatile uint64_t *source);
		/**
		  * \brief Loads the 32-bit signed value atomically and returns the value.
		  * Ensures that memory operations are completed before the read.
		  * \param source The source value to read from.
		  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
		  * \return Returns the source value.
		  */
		fpl_platform_api int32_t AtomicLoadS32(volatile int32_t *source);
		/**
		  * \brief Loads the 64-bit signed value atomically and returns the value.
		  * Ensures that memory operations are completed before the read.
		  * \param source The source value to read from.
		  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
		  * \return Returns the source value.
		  */
		fpl_platform_api int64_t AtomicLoadS64(volatile int64_t *source);
		/**
		  * \brief Loads the pointer value atomically and returns the value.
		  * Ensures that memory operations are completed before the read.
		  * \param source The source value to read from.
		  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
		  * \return Returns the source value.
		  */
		fpl_common_api void *AtomicLoadPtr(volatile void **source);

		/**
		  * \brief Overwrites the 32-bit unsigned value atomically.
		  * Ensures that memory operations are completed before the write.
		  * \param dest The destination to write to.
		  * \param value The value to exchange with.
		  * \return Returns the source value.
		  */
		fpl_platform_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
		/**
		  * \brief Overwrites the 64-bit unsigned value atomically.
		  * Ensures that memory operations are completed before the write.
		  * \param dest The destination to write to.
		  * \param value The value to exchange with.
		  * \return Returns the source value.
		  */
		fpl_platform_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
		/**
		  * \brief Overwrites the 32-bit signed value atomically.
		  * Ensures that memory operations are completed before the write.
		  * \param dest The destination to write to.
		  * \param value The value to exchange with.
		  * \return Returns the source value.
		  */
		fpl_platform_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value);
		/**
		  * \brief Overwrites the 64-bit signed value atomically.
		  * Ensures that memory operations are completed before the write.
		  * \param dest The destination to write to.
		  * \param value The value to exchange with.
		  * \return Returns the source value.
		  */
		fpl_platform_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value);
		/**
		  * \brief Overwrites the pointer value atomically.
		  * Ensures that memory operations are completed before the write.
		  * \param dest The destination to write to.
		  * \param value The value to exchange with.
		  * \return Returns the source value.
		  */
		fpl_common_api void AtomicStorePtr(volatile void **dest, const void *value);

		/** \}*/
	}

	//! Hardware functions
	namespace hardware {
		/**
		  * \defgroup Hardware Hardware functions
		  * \brief Hardware functions, like GetProcessorCoreCount, GetProcessorName, etc.
		  * \{
		  */

		  //! Memory informations
		struct MemoryInfos {
			//! Total size of physical memory in bytes (Amount of RAM installed)
			uint64_t totalPhysicalSize;
			//! Available size of physical memory in bytes (May be less than the amount of RAM installed)
			uint64_t availablePhysicalSize;
			//! Free size of physical memory in bytes
			uint64_t usedPhysicalSize;
			//! Total size of virtual memory in bytes
			uint64_t totalVirtualSize;
			//! Used size of virtual memory in bytes
			uint64_t usedVirtualSize;
			//! Total page size in bytes
			uint64_t totalPageSize;
			//! Used page size in bytes
			uint64_t usedPageSize;
		};

		/**
		  * \brief Returns the total number of processor cores.
		  * \return Number of processor cores.
		  */
		fpl_platform_api uint32_t GetProcessorCoreCount();
		/**
		  * \brief Returns the name of the processor.
		  * The processor name is written in the destination buffer.
		  * \param destBuffer The character buffer to write the processor name into.
		  * \param maxDestBufferLen The total number of characters available in the destination character buffer.
		  * \return Name of the processor.
		  */
		fpl_platform_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen);
		/**
		  * \brief Returns the current system memory informations.
		  * \return Current system memory informations.
		  */
		fpl_platform_api MemoryInfos GetSystemMemoryInfos();

		/** \}*/
	}

	/**
	  * \defgroup Settings Settings and configurations
	  * \brief Video/audio/window settings
	  * \{
	  */

	//! Initialization flags (Window, Video, etc.)
	enum class InitFlags : int {
		//! No init flags
		None = 0,
		//! Create a single window
		Window = 1 << 0,
		//! Use a video backbuffer (This flag ensures that \ref InitFlags::Window is included always)
		Video = 1 << 1,
		//! Use asyncronous audio playback
		Audio = 1 << 2,
		//! Default init flags for initializing everything
		All = Window | Video | Audio
	};
	//! Operator support for InitFlags
	FPL_ENUM_AS_FLAGS_OPERATORS(InitFlags);

	//! Video driver type
	enum class VideoDriverType {
		//! No video driver
		None,
		//! OpenGL
		OpenGL,
		//! Software
		Software
	};

#if defined(FPL_ENABLE_VIDEO_OPENGL)
	//! OpenGL compability flags
	enum class OpenGLCompabilityFlags : uint32_t {
		//! Use legacy context
		Legacy = 0,
		//! Use core profile
		Core = 1 << 1,
		//! Use compability profile
		Compability = 1 << 2,
		//! Remove features marked as deprecated
		Forward = 1 << 3,
	};
	//! Defines the operator overloads for a enum used as flags
	FPL_ENUM_AS_FLAGS_OPERATORS(OpenGLCompabilityFlags);

	//! OpenGL video settings container
	struct OpenGLVideoSettings {
		//! Compability flags
		OpenGLCompabilityFlags compabilityFlags;
		//! Desired major version
		uint32_t majorVersion;
		//! Desired minor version
		uint32_t minorVersion;
	};
#endif // FPL_ENABLE_VIDEO_OPENGL

	//! Video settings container (Driver, Flags, Version, VSync, etc.)
	struct VideoSettings {
		//! Video driver type
		VideoDriverType driver;
		//! Vertical syncronisation enabled/disabled
		bool isVSync;
		//! Backbuffer size is automatically resized. Useable only for software rendering!
		bool isAutoSize;
		//! Graphics API settings
		union {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			OpenGLVideoSettings opengl;
#		endif
		} graphics;
	};

	/**
	  * \brief Make default video settings
	  * \note This will not change any video settings! To change the actual settings you have to pass the entire \ref Settings container to a argument in \ref InitPlatform().
	  */
	fpl_inline VideoSettings DefaultVideoSettings() {
		VideoSettings result = {};

		result.isVSync = false;
		result.isAutoSize = true;

		// @NOTE(final): Auto detect video driver
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		result.driver = VideoDriverType::OpenGL;
		result.graphics.opengl.compabilityFlags = OpenGLCompabilityFlags::Legacy;
#	elif defined(FPL_ENABLE_VIDEO_SOFTWARE)
		result.driver = VideoDriverType::Software;
#	else
		result.driver = VideoDriverType::None;
#	endif

		return(result);
	}

	//! Audio driver type
	enum class AudioDriverType {
		//! No audio driver
		None,
		//! Auto detection
		Auto,
		//! DirectSound
		DirectSound,
	};

	//! Audio format type
	enum class AudioFormatType : uint32_t {
		// No audio format
		None,
		// Unsigned 8-bit integer PCM
		U8,
		// Signed 16-bit integer PCM
		S16,
		// Signed 24-bit integer PCM
		S24,
		// Signed 32-bit integer PCM
		S32,
		// Signed 64-bit integer PCM
		S64,
		// 32-bit IEEE_FLOAT
		F32,
		// 64-bit IEEE_FLOAT
		F64,
	};

	//! Audio device format
	struct AudioDeviceFormat {
		//! Audio format
		AudioFormatType type;
		//! Samples per seconds
		uint32_t sampleRate;
		//! Number of channels
		uint32_t channels;
		//! Number of periods
		uint32_t periods;
		//! Buffer size for the device
		uint32_t bufferSizeInBytes;
		//! Buffer size in frames
		uint32_t bufferSizeInFrames;
	};

	//! Audio device id
	struct AudioDeviceID {
		//! Device name
		char name[256];
		union {
#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
			//! DirectShow Device GUID
			GUID dshow;
#		endif
		};
	};

	//! Audio Client Read Callback Function
	typedef uint32_t(AudioClientReadFunction)(const AudioDeviceFormat &deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

	//! Audio settings
	struct AudioSettings {
		//! The device format
		AudioDeviceFormat deviceFormat;
		//! The device id
		AudioDeviceID deviceID;
		//! The callback for retrieving audio data from the client
		AudioClientReadFunction *clientReadCallback;
		//! The targeted driver
		AudioDriverType driver;
		//! Audio buffer in milliseconds
		uint32_t bufferSizeInMilliSeconds;
		//! Is exclude mode prefered
		bool preferExclusiveMode;
		//! User data pointer for client read callback
		void *userData;
	};

	/**
	  * \brief Make default audio settings (S16 PCM, 48 KHz, 2 Channels)
	  * \note This will not change any audio settings! To change the actual settings you have to pass the entire \ref Settings container to a argument in \ref InitPlatform().
	  */
	fpl_inline AudioSettings DefaultAudioSettings() {
		AudioSettings result = {};
		result.bufferSizeInMilliSeconds = 25;
		result.preferExclusiveMode = false;
		result.deviceFormat.channels = 2;
		result.deviceFormat.sampleRate = 48000;
		result.deviceFormat.type = AudioFormatType::S16;

		result.driver = AudioDriverType::None;
#	if defined(FPL_PLATFORM_WIN32)
#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		result.driver = AudioDriverType::DirectSound;
#		endif
#	endif
		return(result);
	}

	//! Window settings (Size, Title etc.)
	struct WindowSettings {
		//! Window title
		char windowTitle[256];
		//! Window width in screen coordinates
		uint32_t windowWidth;
		//! Window height in screen coordinates
		uint32_t windowHeight;
		//! Fullscreen width in screen coordinates
		uint32_t fullscreenWidth;
		//! Fullscreen height in screen coordinates
		uint32_t fullscreenHeight;
		//! Is window resizable
		bool isResizable;
		//! Is window in fullscreen mode
		bool isFullscreen;
	};

	/**
	  * \brief Make default settings for the window
	  * \note This will not change any window settings! To change the actual settings you have to pass the entire \ref Settings container to a argument in \ref InitPlatform().
	  */
	fpl_inline WindowSettings DefaultWindowSettings() {
		WindowSettings result = {};
		result.windowTitle[0] = 0;
		result.windowWidth = 800;
		result.windowHeight = 600;
		result.fullscreenWidth = 0;
		result.fullscreenHeight = 0;
		result.isResizable = true;
		result.isFullscreen = false;
		return(result);
	}

	//! Input settings
	struct InputSettings {
		//! Frequency in ms for detecting new or removed controllers (Default: 100 ms)
		uint32_t controllerDetectionFrequency;
	};

	/**
	  * \brief Make default settings for input devices.
	  * \note This will not change any input settings! To change the actual settings you have to pass the entire \ref Settings container to a argument in \ref InitPlatform().
	  */
	fpl_inline InputSettings DefaultInputSettings() {
		InputSettings result = {};
		result.controllerDetectionFrequency = 100;
		return(result);
	}

	//! Resulting container for preparing the window before the video context is created
	struct PrepareWindowForVideoResult {
		//! Window settings for video
		union {
#		if defined(FPL_PLATFORM_WIN32)
			//! Win32 result
			struct {
				// @NOTE(final): Win32 does not require to do anything before the window is created
			} win32;
#		endif
#		if defined(FPL_SUBPLATFORM_X11)
			//! X11 result
			struct {
				//! Visual info created by GLX
				void *visualInfoOpaque;
			} x11;
#		endif
		} window;
		//! Was successful
		bool wasSuccessful;
	};

	//! Macro for prepare video for window callback
#	define FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_BEFORE(name) PrepareWindowForVideoResult name(const VideoSettings &videoSettings, void *userData)
	//! Prepare window for video before callback
	typedef FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_BEFORE(callback_PrepareWindowForVideoBefore);

	//! Settings container for storing fields required for preparing the window for video
	struct WindowForVideoSettings {
		//! User data for callback
		void *userData;
		//! Is called from the window system before the window is created
		callback_PrepareWindowForVideoBefore *beforeCallback;
	};

	//! Settings container (Window, Video, etc)
	struct Settings {
		//! Window settings
		WindowSettings window;
		//! Video settings
		VideoSettings video;
		//! Audio settings
		AudioSettings audio;
		//! Input settings
		InputSettings input;
		//! Settings for preparing the window for video
		WindowForVideoSettings windowForVideo;
	};

	/**
	  * \brief Make default settings for window, video, audio, etc.
	  * \note This will not change any settings! To change the actual settings you have to pass this settings container to a argument in \ref InitPlatform().
	  */
	fpl_inline Settings DefaultSettings() {
		Settings result = {};
		result.window = DefaultWindowSettings();
		result.video = DefaultVideoSettings();
		result.audio = DefaultAudioSettings();
		result.input = DefaultInputSettings();
		return(result);
	}

	/**
	  * \brief Returns the current settings
	  */
	fpl_common_api const Settings &GetCurrentSettings();

	/** \}*/

	/**
	  * \defgroup Initialization Initialization functions
	  * \brief Initialization and release functions
	  * \{
	  */

	  /**
		* \brief Initializes the platform layer.
		* \param initFlags Optional init flags used for enable certain features, like video/audio etc. (Default: \ref InitFlags::All)
		* \param initSettings Optional initialization settings which can be passed to control the platform layer behavior or systems. (Default: \ref Settings provided by \ref DefaultSettings())
		* \note \ref ReleasePlatform() must be called when you are done! After \ref ReleasePlatform() has been called you can call this function again if needed.
		* \return Returns true when the initialzation was successful, otherwise false. Will return false when the platform layers is already initialized successfully.
		*/
	fpl_common_api bool InitPlatform(const InitFlags initFlags = InitFlags::All, const Settings &initSettings = DefaultSettings());
	/**
	  * \brief Releases the resources allocated by the platform layer.
	  * \note Can only be called when \ref InitPlatform() was successful.
	  */
	fpl_common_api void ReleasePlatform();

	/** \}*/

	/**
	  * \defgroup ErrorHandling Error Handling
	  * \brief Functions for error handling
	  * \{
	  */

	  /**
		* \brief Returns the last internal error string
		* \note This function can be called regardless of the initialization state!
		* \return Last error string or empty string when there was no error.
		*/
	fpl_common_api const char *GetPlatformError();
	/**
	  * \brief Returns the last error string from the given index
	  * \param index The index
	  * \note This function can be called regardless of the initialization state!
	  * \return Last error string from the given index or empty when there was no error.
	  */
	fpl_common_api const char *GetPlatformError(const size_t index);
	/**
	  * \brief Returns the count of total last errors
	  * \note This function can be called regardless of the initialization state!
	  * \return Number of last errors or zero when there was no error.
	  */
	fpl_common_api size_t GetPlatformErrorCount();
	/**
	  * \brief Clears all the current errors in the platform
	  * \note This function can be called regardless of the initialization state!
	  */
	fpl_common_api void ClearPlatformErrors();

	/** \}*/

	//! Dynamic library functions and types
	namespace library {
		/**
		  * \defgroup DynamicLibrary Dynamic library loading
		  * \brief Loading dynamic libraries and retrieving the procedure addresses.
		  * \{
		  */

		  //! Handle to a loaded dynamic library
		struct DynamicLibraryHandle {
			//! Internal library handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				HMODULE win32Handle;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				void *posixHandle;
#			endif
			} internalHandle;
			//! Library opened successfully
			bool isValid;
		};

		/**
		  * \brief Loads a dynamic library and returns the loaded handle for it.
		  * \param libraryFilePath The path to the library with included file extension (.dll / .so)
		  * \note To check for success, just check the DynamicLibraryHandle.isValid field from the result.
		  * \return Handle container of the loaded library.
		  */
		fpl_platform_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath);
		/**
		  * \brief Returns the dynamic library procedure address for the given procedure name.
		  * \param handle Handle to the loaded library
		  * \param name Name of the procedure
		  * \return Procedure address for the given procedure name or nullptr when procedure not found or library is not loaded.
		  */
		fpl_platform_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name);
		/**
		  * \brief Unloads the loaded library and resets the handle to zero.
		  * \param handle Loaded dynamic library handle
		  */
		fpl_platform_api void DynamicLibraryUnload(DynamicLibraryHandle &handle);

		/** \}*/
	}

	//! Console functions
	namespace console {
		/**
		  * \defgroup Console Console functions
		  * \brief Console out/in functions
		  * \{
		  */

		  /**
			* \brief Writes the given text to the standard output console buffer.
			* \param text The text to write into standard output console.
			* \note This is most likely just a wrapper call to fprintf(stdout)
			*/
		fpl_platform_api void ConsoleOut(const char *text);
		/**
		  * \brief Writes the given formatted text to the standard output console buffer.
		  * \param format The format used for writing into the standard output console.
		  * \param ... The dynamic arguments used for formatting the text.
		  * \note This is most likely just a wrapper call to vfprintf(stdout)
		  */
		fpl_platform_api void ConsoleFormatOut(const char *format, ...);
		/**
		  * \brief Writes the given text to the standard error console buffer.
		  * \param text The text to write into standard error console.
		  * \note This is most likely just a wrapper call to fprintf(stderr)
		  */
		fpl_platform_api void ConsoleError(const char *text);
		/**
		  * \brief Writes the given formatted text to the standard error console buffer.
		  * \param format The format used for writing into the standard error console.
		  * \param ... The dynamic arguments used for formatting the text.
		  * \note This is most likely just a wrapper call to vfprintf(stderr)
		  */
		fpl_platform_api void ConsoleFormatError(const char *format, ...);
		/**
		  * \brief Wait for a character to be typed in the console input and return it
		  * \note This is most likely just a wrapper call to getchar()
		  * \return Character typed in in the console input
		  */
		fpl_platform_api const char ConsoleWaitForCharInput();

		/** \}*/
	}

	//! Threading functions
	namespace threading {
		/**
		  * \defgroup Threading Threading routines
		  * \brief Tons of functions for multithreading, mutex and signal creation and handling
		  * \{
		  */

		  //! Thread state
		enum class ThreadState : uint32_t {
			//! Thread is stopped
			Stopped,
			//! Thread is being started
			Starting,
			//! Thread is still running
			Running,
			//! Thread is being stopped
			Stopping,
		};

		struct ThreadContext;
		//! Run function type definition for CreateThread
		typedef void (run_thread_function)(const ThreadContext &context, void *data);

		//! Stores all required information for a thread, like its handle and its state, etc.
		struct ThreadContext {
			//! The identifier of the thread
			uint64_t id;
			//! The stored run function
			run_thread_function *runFunc;
			//! The user data passed to the run function
			void *data;
			//! The internal thread handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
					HANDLE handle;
				} win32;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				struct {
					pthread_t thread;
					pthread_mutex_t mutex;
					pthread_cond_t stopCondition;
				} posix;
#			endif		
			} internalHandle;
			//! Thread state
			volatile ThreadState currentState;
			//! Is this thread valid
			volatile bool isValid;
			//! Is this thread stopping
			volatile bool isStopping;
		};

		//! Mutex context
		struct ThreadMutex {
			//! The internal mutex handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
					CRITICAL_SECTION criticalSection;
				} win32;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				struct {
					pthread_mutex_t mutex;
				} posix;
#			endif		
			} internalHandle;			//! Is it valid
			bool isValid;
		};

		//! Signal context
		struct ThreadSignal {
			//! The internal signal handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
					HANDLE eventHandle;
				} win32;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				struct {
					pthread_cond_t condition;
				} posix;
#			endif
			} internalHandle;
			//! Is it valid
			bool isValid;
		};

		//! Returns the current thread state from the given thread context
		fpl_inline ThreadState GetThreadState(ThreadContext *thread) {
			if(thread == nullptr) {
				return ThreadState::Stopped;
			}
			ThreadState result = (ThreadState)atomics::AtomicLoadU32((volatile uint32_t *)&thread->currentState);
			return(result);
		}

		/**
		  * \brief Creates a thread and return the context for it.
		  * \param runFunc Function prototype called when this thread starts.
		  * \param data User data passed to the run function.
		  * \note Use \ref ThreadDestroy() with this thread context when you dont need this thread anymore. You can only have 64 threads suspended/running at the same time!
		  * \warning Do not free this thread context directly! Use \ref ThreadDestroy() instead.
		  * \return Pointer to a internal stored thread-context or return nullptr when the limit of current threads has been reached.
		  */
		fpl_platform_api ThreadContext *ThreadCreate(run_thread_function *runFunc, void *data);
		/**
		  * \brief Let the current thread sleep for the given amount of milliseconds.
		  * \param milliseconds Number of milliseconds to sleep
		  * \note There is no guarantee that the OS sleeps for the exact amount of milliseconds! This can vary based on the OS scheduler granularity.
		  */
		fpl_platform_api void ThreadSleep(const uint32_t milliseconds);
		/**
		  * \brief Stop the given thread and release all underlying resources.
		  * \param context Context to the thread
		  * \note This thread context may get re-used for another thread in the future!
		  * \warning Do not free the given thread context manually!
		  */
		fpl_platform_api void ThreadDestroy(ThreadContext *context);
		/**
		  * \brief Wait until the given thread is done running or the given timeout has been reached.
		  * \param context Thread context
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when the thread completes or when the timeout has been reached.
		  */
		fpl_platform_api bool ThreadWaitForOne(ThreadContext *context, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		  * \brief Wait until all given threads are done running or the given timeout has been reached.
		  * \param contexts Array of thread contexts
		  * \param count Number of thread contexts in the array
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when all threads completes or when the timeout has been reached.
		  */
		fpl_platform_api bool ThreadWaitForAll(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		  * \brief Wait until one of given threads is done running or the given timeout has been reached.
		  * \param contexts Array of thread contexts
		  * \param count Number of thread contexts in the array
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when one thread completes or when the timeout has been reached.
		  */
		fpl_platform_api bool ThreadWaitForAny(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);

		/**
		  * \brief Creates a mutex and returns a copy of the handle to it.
		  * \note Use \ref MutexDestroy() when you are done with this mutex.
		  * \return Copy of the handle to the mutex.
		  */
		fpl_platform_api ThreadMutex MutexCreate();
		/**
		  * \brief Releases the given mutex and clears the structure to zero.
		  * \param mutex The mutex reference to destroy.
		  */
		fpl_platform_api void MutexDestroy(ThreadMutex &mutex);
		/**
		  * \brief Locks the given mutex and ensures that other threads will wait until it gets unlocked or the timeout has been reached.
		  * \param mutex The mutex reference to lock
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \returns True when mutex was locked or false otherwise.
		  */
		fpl_platform_api bool MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		 * \brief Unlocks the given mutex
		 * \param mutex The mutex reference to unlock
		 * \returns True when mutex was unlocked or false otherwise.
		 */
		fpl_platform_api bool MutexUnlock(ThreadMutex &mutex);

		/**
		  * \brief Creates a signal and returns a copy of the handle to it.
		  * \note Use \ref SignalDestroy() when you are done with this signal.
		  * \return Copy of the handle to the signal.
		  */
		fpl_platform_api ThreadSignal SignalCreate();
		/**
		  * \brief Releases the given signal and clears the structure to zero.
		  * \param signal The signal reference to destroy.
		  */
		fpl_platform_api void SignalDestroy(ThreadSignal &signal);
		/**
		  * \brief Waits until the given signal are waked up.
		  * \param mutex The mutex reference
		  * \param signal The signal reference to signal.
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when the signal woke up or the timeout has been reached, otherwise false.
		  */
		fpl_platform_api bool SignalWaitForOne(ThreadMutex &mutex, ThreadSignal &signal, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		  * \brief Waits until all the given signal are waked up.
		  * \param mutex The mutex reference
		  * \param signals Array of signals
		  * \param count Number of signals
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when all signals woke up or the timeout has been reached, otherwise false.
		  */
		fpl_platform_api bool SignalWaitForAll(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		  * \brief Waits until any of the given signals wakes up or the timeout has been reached.
		  * \param mutex The mutex reference
		  * \param signals Array of signals
		  * \param count Number of signals
		  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
		  * \return Returns true when any of the signals woke up or the timeout has been reached, otherwise false.
		  */
		fpl_platform_api bool SignalWaitForAny(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		  * \brief Sets the signal and wakes up the given signal.
		  * \param signal The reference to the signal
		  * \return Returns true when the signal was set and woke up, otherwise false.
		  */
		fpl_platform_api bool SignalSet(ThreadSignal &signal);

		/** \}*/
	}

	//! Memory allocation, clearing and copy functions
	namespace memory {
		/**
		  * \defgroup Memory Memory functions
		  * \brief Memory allocation, clearing and copy functions
		  * \{
		  */

		  /**
			* \brief Clears the given memory by the given size to zero.
			* \param mem Pointer to the memory.
			* \param size Size in bytes to be cleared to zero.
			*/
		fpl_common_api void MemoryClear(void *mem, const size_t size);
		/**
		  * \brief Copies the given source memory with its length to the target memory.
		  * \param sourceMem Pointer to the source memory to copy from.
		  * \param sourceSize Size in bytes to be copied.
		  * \param targetMem Pointer to the target memory to copy to.
		  */
		fpl_common_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem);
		/**
		  * \brief Allocates memory from the operating system by the given size.
		  * \param size Size to by allocated in bytes.
		  * \note The memory is guaranteed to be initialized by zero.
		  * \warning Alignment is not ensured here, the OS decides how to handle this. If you want to force a specific alignment use \ref MemoryAlignedAllocate() instead.
		  * \return Pointer to the new allocated memory.
		  */
		fpl_platform_api void *MemoryAllocate(const size_t size);
		/**
		  * \brief Releases the memory allocated from the operating system.
		  * \param ptr Pointer to the allocated memory.
		  * \warning This should never be called with a aligned memory pointer! For freeing aligned memory, use \ref MemoryAlignedFree() instead.
		  * \return Pointer to the new allocated memory.
		  */
		fpl_platform_api void MemoryFree(void *ptr);
		/**
		  * \brief Allocates memory on the current stack by the given amount in bytes.
		  * \param size Size amount in bytes
		  * \warning Use this very carefully, the memory will be released then the current scope goes out of scope!
		  * \return Pointer to the new allocated stack memory.
		  */
		fpl_platform_api void *MemoryStackAllocate(const size_t size);
		/**
		  * \brief Allocates aligned memory from the operating system by the given alignment.
		  * \param size Size amount in bytes
		  * \param alignment Alignment in bytes (Needs to be a power-of-two!)
		  * \note The memory is guaranteed to be initialized by zero.
		  * \return Pointer to the new allocated aligned memory.
		  */
		fpl_common_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment);
		/**
		  * \brief Releases the aligned memory allocated from the operating system.
		  * \param ptr Pointer to the aligned allocated memory.
		  * \warning This should never be called with a not-aligned memory pointer! For freeing not-aligned memory, use \ref MemoryFree() instead.
		  * \return Pointer to the new allocated memory.
		  */
		fpl_common_api void MemoryAlignedFree(void *ptr);

		/** \}*/
	}

	//! Timing and measurement functions
	namespace timings {
		/**
		  * \defgroup Timings Timing functions
		  * \brief Functions for retrieving timebased informations
		  * \{
		  */

		/**
		  * \brief Returns the current system clock in seconds with the highest precision possible.
		  * \return Returns number of second since some fixed starting point (OS start, System start, etc).
		  * \note Can only be used to calculate a difference in time!
		  */
		fpl_platform_api double GetHighResolutionTimeInSeconds();

		/**
		  * \brief Returns the current system in milliseconds without deeper precision.
		  * \return Returns number of milliseconds since some fixed starting point (OS start, System start, etc).
		  * \note Can only be used to calculate a difference in time!
		  */
		fpl_platform_api uint64_t GetTimeInMilliseconds();

		/** \}*/
	}

	//! String functions
	namespace strings {
		/**
		  * \defgroup Strings String manipulation functions
		  * \brief Functions for converting/manipulating strings
		  * \{
		  */

		  /**
			* \brief Returns true when both ansi strings are equal with enforcing the given length.
			* \param a First string
			* \param aLen Number of characters for the first string
			* \param b Second string
			* \param bLen Number of characters for the second string
			* \note Len parameters does not include the null-terminator!
			* \return True when strings matches, otherwise false.
			*/
		fpl_common_api bool IsStringEqual(const char *a, const uint32_t aLen, const char *b, const uint32_t bLen);
		/**
		  * \brief Returns true when both ansi strings are equal.
		  * \param a First string
		  * \param b Second string
		  * \return True when strings matches, otherwise false.
		  */
		fpl_common_api bool IsStringEqual(const char *a, const char *b);
		/**
		  * \brief Returns the number of characters of the given 8-bit Ansi string.
		  * \param str The 8-bit ansi string
		  * \note Null terminator is not included!
		  * \return Returns the character length or zero when the input string is nullptr.
		  */
		fpl_common_api uint32_t GetAnsiStringLength(const char *str);
		/**
		  * \brief Returns the number of characters of the given 16-bit wide string.
		  * \param str The 16-bit wide string
		  * \note Null terminator is not included!
		  * \return Returns the character length or zero when the input string is nullptr.
		  */
		fpl_common_api uint32_t GetWideStringLength(const wchar_t *str);
		/**
		  * \brief Copies the given 8-bit source ansi string with a fixed length into a destination ansi string.
		  * \param source The 8-bit source ansi string.
		  * \param sourceLen The number of characters to copy.
		  * \param dest The 8-bit destination ansi string buffer.
		  * \param maxDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_common_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
		/**
		  * \brief Copies the given 8-bit source ansi string into a destination ansi string.
		  * \param source The 8-bit source ansi string.
		  * \param dest The 8-bit destination ansi string buffer.
		  * \param maxDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_common_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen);
		/**
		  * \brief Copies the given 16-bit source wide string with a fixed length into a destination wide string.
		  * \param source The 16-bit source wide string.
		  * \param sourceLen The number of characters to copy.
		  * \param dest The 16-bit destination wide string buffer.
		  * \param maxDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_common_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
		/**
		  * \brief Copies the given 16-bit source wide string into a destination wide string.
		  * \param source The 16-bit source wide string.
		  * \param dest The 16-bit destination wide string buffer.
		  * \param maxDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_common_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen);
		/**
		  * \brief Converts the given 16-bit source wide string with length in a 8-bit ansi string.
		  * \param wideSource The 16-bit source wide string.
		  * \param maxWideSourceLen The number of characters of the source wide string.
		  * \param ansiDest The 8-bit destination ansi string buffer.
		  * \param maxAnsiDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_platform_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
		/**
		  * \brief Converts the given 16-bit source wide string with length in a 8-bit UTF-8 ansi string.
		  * \param wideSource The 16-bit source wide string.
		  * \param maxWideSourceLen The number of characters of the source wide string.
		  * \param utf8Dest The 8-bit destination ansi string buffer.
		  * \param maxUtf8DestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_platform_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
		/**
		  * \brief Converts the given 8-bit source ansi string with length in a 16-bit wide string.
		  * \param ansiSource The 8-bit source ansi string.
		  * \param ansiSourceLen The number of characters of the source wide string.
		  * \param wideDest The 16-bit destination wide string buffer.
		  * \param maxWideDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_platform_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
		/**
		  * \brief Converts the given 8-bit UTF-8 source ansi string with length in a 16-bit wide string.
		  * \param utf8Source The 8-bit source ansi string.
		  * \param utf8SourceLen The number of characters of the source wide string.
		  * \param wideDest The 16-bit destination wide string buffer.
		  * \param maxWideDestLen The total number of characters available in the destination buffer.
		  * \note Null terminator is included always. Does not allocate any memory.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr when either the dest buffer is too small or the source string is invalid.
		  */
		fpl_platform_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
		/**
		  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and variable arguments.
		  * \param ansiDestBuffer The 8-bit destination ansi string buffer.
		  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer.
		  * \param format The string format.
		  * \param ... Variable arguments.
		  * \note This is most likely just a wrapper call to vsnprintf()
		  * \return Pointer to the first character in the destination buffer or nullptr.
		  */
		fpl_platform_api char *FormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...);

		/** \}*/
	}

	//! Files & directory functions and types
	namespace files {
		/**
		  * \defgroup Files Files/IO functions
		  * \brief Tons of file and directory IO functions
		  * \{
		  */

		  //! Handle to a loaded/created file
		struct FileHandle {
			//! Internal file handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
					HANDLE handle;
				} win32;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				struct {
					int handle;
				} posix;
#			endif
			} internalHandle;
			//! File opened successfully
			bool isValid;
		};

		//! File position mode (Beginning, Current, End)
		enum class FilePositionMode {
			//! Starts from the beginning
			Beginning,
			//! Starts from the current position
			Current,
			//! Starts from the end
			End
		};

		//! File entry type (File, Directory, etc.)
		enum class FileEntryType {
			//! Unknown entry type
			Unknown,
			//! Entry is a file
			File,
			//! Entry is a directory
			Directory
		};

		//! File attribute flags (Normal, Readonly, Hidden, etc.)
		enum class FileAttributeFlags : int {
			//! No attributes
			None = 0,
			//! Normal
			Normal = 1 << 0,
			//! Readonly
			ReadOnly = 1 << 1,
			//! Hidden
			Hidden = 1 << 2,
			//! Archive
			Archive = 1 << 3,
			//! System
			System = 1 << 4
		};
		//! Operator support for FileAttributeFlags
		FPL_ENUM_AS_FLAGS_OPERATORS(FileAttributeFlags);

		//! Maximum length of a file entry path
		fpl_constant uint32_t MAX_FILEENTRY_PATH_LENGTH = 1024;

		//! Entry for storing current file informations (path, type, attributes, etc.)
		struct FileEntry {
			//! Entry type
			FileEntryType type;
			//! File attributes
			FileAttributeFlags attributes;
			//! File path
			char path[MAX_FILEENTRY_PATH_LENGTH];
			//! Internal file handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
					HANDLE fileHandle;
				} win32;
#			endif
#			if defined(FPL_SUBPLATFORM_POSIX)
				struct {
					int fileHandle;
				} posix;
#			endif
			} internalHandle;
		};

		/**
		  * \brief Opens a binary file for reading from a ansi string path and returns the handle of it.
		  * \param filePath Ansi file path.
		  * \note To check for success just test the \ref FileHandle.isValid field from the result.
		  * \return Copy of the handle to the open file.
		  */
		fpl_platform_api FileHandle OpenBinaryFile(const char *filePath);
		/**
		  * \brief Opens a binary file for reading from a wide string path and returns the handle of it.
		  * \param filePath Wide file path.
		  * \note To check for success just test the \ref FileHandle.isValid field from the result.
		  * \return Copy of the handle to the open file.
		  */
		fpl_platform_api FileHandle OpenBinaryFile(const wchar_t *filePath);
		/**
		  * \brief Create a binary file for writing to the given ansi string path and returns the handle of it.
		  * \param filePath Ansi file path.
		  * \note To check for success just test the \ref FileHandle.isValid field from the result. The file is ensured to be overriden always.
		  * \return Copy of the handle to the created file.
		  */
		fpl_platform_api FileHandle CreateBinaryFile(const char *filePath);
		/**
		  * \brief Create a binary file for writing to the given wide string path and returns the handle of it.
		  * \param filePath Wide file path.
		  * \note To check for success just test the \ref FileHandle.isValid field from the result. The file is ensured to be overriden always.
		  * \return Copy of the handle to the created file.
		  */
		fpl_platform_api FileHandle CreateBinaryFile(const wchar_t *filePath);
		/**
		  * \brief Reads a block from the given file handle and returns the number of bytes read.
		  * \param fileHandle Reference to the file handle.
		  * \param sizeToRead Number of bytes to read.
		  * \param targetBuffer Target memory to write into.
		  * \param maxTargetBufferSize Total number of bytes available in the target buffer.
		  * \note Its limited to files < 2 GB.
		  * \return Number of bytes read or zero.
		  */
		fpl_platform_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
		/**
		  * \brief Writes a block to the given file handle and returns the number of bytes written.
		  * \param fileHandle Reference to the file handle.
		  * \param sourceBuffer Source memory to read from.
		  * \param sourceSize Number of bytes to write.
		  * \note Its limited to files < 2 GB.
		  * \return Number of bytes written or zero.
		  */
		fpl_platform_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize);
		/**
		  * \brief Sets the current file position by the given position, depending on the mode its absolute or relative.
		  * \param fileHandle Reference to the file handle.
		  * \param position Position in bytes
		  * \param mode Position mode
		  * \note Its limited to files < 2 GB.
		  */
		fpl_platform_api void SetFilePosition32(const FileHandle &fileHandle, const int32_t position, const FilePositionMode mode);
		/**
		  * \brief Returns the current file position in bytes.
		  * \param fileHandle Reference to the file handle.
		  * \note Its limited to files < 2 GB.
		  * \return Current file position in bytes.
		  */
		fpl_platform_api uint32_t GetFilePosition32(const FileHandle &fileHandle);
		/**
		  * \brief Closes the given file and releases the underlying resources and clears the handle to zero.
		  * \param fileHandle Reference to the file handle.
		  */
		fpl_platform_api void CloseFile(FileHandle &fileHandle);

		// @TODO(final): Add 64-bit file operations
		// @TODO(final): Add wide file operations

		/**
		  * \brief Returns the 32-bit file size in bytes for the given file.
		  * \param filePath Ansi path to the file.
		  * \note Its limited to files < 2 GB.
		  * \return File size in bytes or zero.
		  */
		fpl_platform_api uint32_t GetFileSize32(const char *filePath);
		/**
		  * \brief Returns the 32-bit file size in bytes for a opened file.
		  * \param fileHandle Reference to the file handle.
		  * \note Its limited to files < 2 GB.
		  * \return File size in bytes or zero.
		  */
		fpl_platform_api uint32_t GetFileSize32(const FileHandle &fileHandle);
		/**
		  * \brief Returns true when the given file physically exists.
		  * \param filePath Ansi path to the file.
		  * \return True when the file exists, otherwise false.
		  */
		fpl_platform_api bool FileExists(const char *filePath);
		/**
		  * \brief Copies the given source file to the target path and returns true when copy was successful.
		  * \param sourceFilePath Ansi source file path.
		  * \param targetFilePath Ansi target file path.
		  * \param overwrite When true the target file always be overwritten, otherwise it will return false when file already exists.
		  * \return True when the file was copied, otherwise false.
		  */
		fpl_platform_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
		/**
		  * \brief Movies the given source file to the target file and returns true when the move was successful.
		  * \param sourceFilePath Ansi source file path.
		  * \param targetFilePath Ansi target file path.
		  * \return True when the file was moved, otherwise false.
		  */
		fpl_platform_api bool FileMove(const char *sourceFilePath, const char *targetFilePath);
		/**
		  * \brief Deletes the given file without confirmation and returns true when the deletion was successful.
		  * \param filePath Ansi path to the file.
		  * \return True when the file was deleted, otherwise false.
		  */
		fpl_platform_api bool FileDelete(const char *filePath);

		/**
		  * \brief Creates all the directories in the given path.
		  * \param path Ansi path to the directory.
		  * \return True when at least one directory was created, otherwise false.
		  */
		fpl_platform_api bool CreateDirectories(const char *path);
		/**
		  * \brief Returns true when the given directory physically exists.
		  * \param path Ansi path to the directory.
		  * \return True when the directory exists, otherwise false.
		  */
		fpl_platform_api bool DirectoryExists(const char *path);
		/**
		  * \brief Deletes the given empty directory without confirmation and returns true when the deletion was successful.
		  * \param path Ansi path to the directory.
		  * \return True when the empty directory was deleted, otherwise false.
		  */
		fpl_platform_api bool RemoveEmptyDirectory(const char *path);
		/**
		  * \brief Iterates through files / directories in the given directory.
		  * \param pathAndFilter The path with its included after the path separator.
		  * \param firstEntry The reference to a file entry.
		  * \note The path must contain the filter as well.
		  * \return Returns true when there was a first entry found otherwise false.
		  */
		fpl_platform_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry);
		/**
		  * \brief Gets the next file entry from iterating through files / directories.
		  * \param nextEntry The reference to the current file entry.
		  * \return Returns true when there was a next file otherwise false if not.
		  */
		fpl_platform_api bool ListFilesNext(FileEntry &nextEntry);
		/**
		  * \brief Releases opened resources from iterating through files / directories.
		  * \param lastEntry The reference to the last file entry.
		  */
		fpl_platform_api void ListFilesEnd(FileEntry &lastEntry);

		/** \}*/
	}

	//! Directory and paths functions
	namespace paths {
		/**
		  * \defgroup Paths Path functions
		  * \brief Functions for retrieving paths like HomePath, ExecutablePath, etc.
		  * \{
		  */

		  // @TODO(final): Support wide paths as well

		  /**
			* \brief Returns the full path to this executable, including the executable file name.
			* \param destPath Destination buffer
			* \param maxDestLen Total number of characters available in the destination buffer.
			* \note Result is written in the destination buffer.
			* \return Returns the pointer to the first character in the destination buffer or nullptr.
			*/
		fpl_platform_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
		/**
		  * \brief Returns the full path to your home directory.
		  * \param destPath Destination buffer
		  * \param maxDestLen Total number of characters available in the destination buffer.
		  * \note Result is written in the destination buffer.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr.
		  */
		fpl_platform_api char *GetHomePath(char *destPath, const uint32_t maxDestLen);
		/**
		  * \brief Returns the path from the given source path.
		  * \param sourcePath Source path to extract from.
		  * \param destPath Destination buffer
		  * \param maxDestLen Total number of characters available in the destination buffer.
		  * \note Result is written in the destination buffer.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr.
		  */
		fpl_common_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen);
		/**
		  * \brief Returns the file extension from the given source path.
		  * \param sourcePath Source path to extract from.
		  * \return Returns the pointer to the first character of the extension.
		  */
		fpl_common_api char *ExtractFileExtension(const char *sourcePath);
		/**
		  * \brief Returns the file name including the file extension from the given source path.
		  * \param sourcePath Source path to extract from.
		  * \return Returns the pointer to the first character of the filename.
		  */
		fpl_common_api char *ExtractFileName(const char *sourcePath);
		/**
		  * \brief Changes the file extension on the given source path and writes the result into the destination path.
		  * \param filePath File path to search for the extension.
		  * \param newFileExtension New file extension.
		  * \param destPath Destination buffer
		  * \param maxDestLen Total number of characters available in the destination buffer.
		  * \note Result is written in the destination buffer.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr.
		  */
		fpl_common_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen);
		/**
		  * \brief Combines all included path by the systems path separator.
		  * \param destPath Destination buffer
		  * \param maxDestPathLen Total number of characters available in the destination buffer.
		  * \param pathCount Number of dynamic path arguments.
		  * \param ... Dynamic path arguments.
		  * \note Result is written in the destination buffer.
		  * \return Returns the pointer to the first character in the destination buffer or nullptr.
		  */
		fpl_common_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);

		/** \}*/
	}

#if defined(FPL_ENABLE_WINDOW)
	//! Window based functions and types
	namespace window {
		/**
		* \defgroup WindowEvents Window events
		* \brief Window event structures
		* \{
		*/

		//! Mapped keys (Based on MS Virtual-Key-Codes, mostly directly mapped from ASCII)
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

		//! Window event type (Resized, PositionChanged, etc.)
		enum class WindowEventType {
			//! Window has been resized
			Resized,
			//! Window got focus
			GotFocus,
			//! Window lost focus
			LostFocus,
		};

		//! Window event data (Size, Position, etc.)
		struct WindowEvent {
			//! Window event type
			WindowEventType type;
			//! Window width in screen coordinates
			uint32_t width;
			//! Window height in screen coordinates
			uint32_t height;
		};

		//! Keyboard event type (KeyDown, KeyUp, Char, ...)
		enum class KeyboardEventType {
			//! Key is down
			KeyDown,
			//! Key was released
			KeyUp,
			//! Character was entered
			CharInput,
		};

		//! Keyboard modifier flags (Alt, Ctrl, ...)
		enum class KeyboardModifierFlags : int {
			//! No modifiers
			None = 0,
			//! Alt key is down
			Alt = 1 << 0,
			//! Ctrl key is down
			Ctrl = 1 << 1,
			//! Shift key is down
			Shift = 1 << 2,
			//! Super key is down
			Super = 1 << 3,
		};
		//! Operator support for KeyboardModifierFlags
		FPL_ENUM_AS_FLAGS_OPERATORS(KeyboardModifierFlags);

		//! Keyboard event data (Type, Keycode, Mapped key, etc.)
		struct KeyboardEvent {
			//! Keyboard event type
			KeyboardEventType type;
			//! Raw key code
			uint64_t keyCode;
			//! Mapped key
			Key mappedKey;
			//! Keyboard modifiers
			KeyboardModifierFlags modifiers;
		};

		//! Mouse event type (Move, ButtonDown, ...)
		enum class MouseEventType {
			//! Mouse position has been changed
			Move,
			//! Mouse button is down
			ButtonDown,
			//! Mouse button was released
			ButtonUp,
			//! Mouse wheel up/down
			Wheel,
		};

		//! Mouse button type (Left, Right, ...)
		enum class MouseButtonType : int {
			//! No mouse button
			None = -1,
			//! Left mouse button
			Left = 0,
			//! Right mouse button
			Right = 1,
			//! Middle mouse button
			Middle = 2,
		};

		//! Mouse event data (Type, Button, Position, etc.)
		struct MouseEvent {
			//! Mouse event type
			MouseEventType type;
			//! Mouse button
			MouseButtonType mouseButton;
			//! Mouse X-Position
			int32_t mouseX;
			//! Mouse Y-Position
			int32_t mouseY;
			//! Mouse wheel delta
			float wheelDelta;
		};

		//! Gamepad event type (Connected, Disconnected, StateChanged, etc.)
		enum class GamepadEventType {
			//! No gamepad event
			None,
			//! Gamepad connected
			Connected,
			//! Gamepad disconnected
			Disconnected,
			//! Gamepad state updated
			StateChanged,
		};

		//! Gamepad button (IsDown, etc.)
		struct GamepadButton {
			//! Is button down
			bool isDown;
		};

		//! Gamepad state data
		struct GamepadState {
			union {
				struct {
					//! Digital button up
					GamepadButton dpadUp;
					//! Digital button right
					GamepadButton dpadRight;
					//! Digital button down
					GamepadButton dpadDown;
					//! Digital button left
					GamepadButton dpadLeft;

					//! Action button A
					GamepadButton actionA;
					//! Action button B
					GamepadButton actionB;
					//! Action button X
					GamepadButton actionX;
					//! Action button Y
					GamepadButton actionY;

					//! Start button
					GamepadButton start;
					//! Back button
					GamepadButton back;

					//! Analog left thumb button
					GamepadButton leftThumb;
					//! Analog right thumb button
					GamepadButton rightThumb;

					//! Left shoulder button
					GamepadButton leftShoulder;
					//! Right shoulder button
					GamepadButton rightShoulder;
				};

				//! All gamepad buttons
				GamepadButton buttons[14];
			};

			//! Analog left thumb X in range (-1.0 to 1.0f)
			float leftStickX;
			//! Analog left thumb Y in range (-1.0 to 1.0f)
			float leftStickY;
			//! Analog right thumb X in range (-1.0 to 1.0f)
			float rightStickX;
			//! Analog right thumb Y in range (-1.0 to 1.0f)
			float rightStickY;

			//! Analog left trigger in range (-1.0 to 1.0f)
			float leftTrigger;
			//! Analog right trigger in range (-1.0 to 1.0f)
			float rightTrigger;
		};

		//! Gamepad event data (Type, Device, State, etc.)
		struct GamepadEvent {
			//! Gamepad event type
			GamepadEventType type;
			//! Gamepad device index
			uint32_t deviceIndex;
			//! Full gamepad state
			GamepadState state;
		};

		//! Event type (Window, Keyboard, Mouse, ...)
		enum class EventType {
			//! Window event
			Window,
			//! Keyboard event
			Keyboard,
			//! Mouse event
			Mouse,
			//! Gamepad event
			Gamepad,
		};

		//! Event data (Type, Window, Keyboard, Mouse, etc.)
		struct Event {
			//! Event type
			EventType type;
			union {
				//! Window event data
				WindowEvent window;
				//! Keyboard event data
				KeyboardEvent keyboard;
				//! Mouse event data
				MouseEvent mouse;
				//! Gamepad event data
				GamepadEvent gamepad;
			};
		};

		/**
		  * \brief Gets the top event from the internal event queue and removes it.
		  * \param ev Reference to an event
		  * \return Returns false when there are no events left, otherwise true.
		  */
		fpl_common_api bool PollWindowEvent(Event &ev);
		/**
		  * \brief Removes all the events from the internal event queue.
		  * \note Dont call when you care about any event!
		  */
		fpl_common_api void ClearWindowEvents();
		/**
		  * \brief Reads the next window event from the OS and pushes it into the internal queue.
		  * \return Returns true when there was a event from the OS, otherwise true.
		  * \note Use this only if dont use \ref WindowUpdate() and want to handle the events more granular!
		  */
		fpl_platform_api bool PushWindowEvent();
		/**
		  * \brief Updates the game controller states and detects new and disconnected devices.
		  * \note Use this only if dont use \ref WindowUpdate() and want to handle the events more granular!
		  */
		fpl_platform_api void UpdateGameControllers();

		/*\}*/

		/**
		  * \defgroup WindowBase Window functions
		  * \brief Functions for reading/setting/handling the window
		  * \{
		  */

		  //! Window size in screen coordinates
		struct WindowSize {
			//! Width in screen coordinates
			uint32_t width;
			//! Height in screen coordinates
			uint32_t height;
		};

		//! Window position in screen coordinates
		struct WindowPosition {
			//! Left position in screen coordinates
			int32_t left;
			//! Top position in screen coordinates
			int32_t top;
		};

		/**
		  * \brief Returns true when the window is active.
		  * \return True when the window is active, otherwise false.
		  */
		fpl_platform_api bool IsWindowRunning();
		/**
		  * \brief Processes the message queue of the window.
		  * \note This will update the game controller states as well.
		  * \return True when the window is still active, otherwise false.
		  */
		fpl_platform_api bool WindowUpdate();
		/**
		  * \brief Enables or disables the window cursor.
		  * \param value Set this to true for enabling the cursor or false for disabling the cursor.
		  */
		fpl_platform_api void SetWindowCursorEnabled(const bool value);
		/**
		  * \brief Returns the inner window area.
		  * \return Window area size
		  */
		fpl_platform_api WindowSize GetWindowArea();
		/**
		  * \brief Resizes the window to fit the inner area to the given size.
		  * \param width Width in screen units
		  * \param height Height in screen units
		  */
		fpl_platform_api void SetWindowArea(const uint32_t width, const uint32_t height);
		/**
		  * \brief Returns true when the window is resizable.
		  * \return True when the window resizable, otherwise false.
		  */
		fpl_platform_api bool IsWindowResizable();
		/**
		  * \brief Enables or disables the ability to resize the window.
		  * \param value Set this to true for making the window resizable or false for making it static
		  */
		fpl_platform_api void SetWindowResizeable(const bool value);
		/**
		  * \brief Enables or disables fullscreen mode.
		  * \param value Set this to true for changing the window to fullscreen or false for switching it back to window mode.
		  * \param fullscreenWidth Optional fullscreen width in screen units. When set to zero the desktop default is being used. (Default: 0)
		  * \param fullscreenHeight Optional fullscreen height in screen units. When set to zero the desktop default is being used. (Default: 0)
		  * \param refreshRate Optional refresh rate in screen units. When set to zero the desktop default is being used. (Default: 0)
		  * \return True when the window was changed to the desire fullscreen mode, false when otherwise.
		  */
		fpl_platform_api bool SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth = 0, const uint32_t fullscreenHeight = 0, const uint32_t refreshRate = 0);
		/**
		  * \brief Returns true when the window is in fullscreen mode
		  * \return True when the window is in fullscreen mode, otherwise false.
		  */
		fpl_platform_api bool IsWindowFullscreen();
		/**
		  * \brief Returns the absolute window position.
		  * \return Window position in screen units
		  */
		fpl_platform_api WindowPosition GetWindowPosition();
		/**
		  * \brief Sets the window absolut position to the given coordinates.
		  * \param left Left position in screen units.
		  * \param top Top position in screen units.
		  */
		fpl_platform_api void SetWindowPosition(const int32_t left, const int32_t top);
		/**
		  * \brief Sets the window title.
		  * \param title New title ansi string
		  */
		fpl_platform_api void SetWindowTitle(const char *title);

		/*\}*/

		/**
		  * \defgroup WindowClipboard Clipboard functions
		  * \brief Functions for reading/writing clipboard data
		  * \{
		  */

		  /**
			* \brief Returns the current clipboard ansi text.
			* \param dest The destination ansi string buffer to write the clipboard text into.
			* \param maxDestLen The total number of characters available in the destination buffer.
			* \return Pointer to the first character in the clipboard text or nullptr otherwise.
			*/
		fpl_platform_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
		/**
		  * \brief Returns the current clipboard wide text.
		  * \param dest The destination wide string buffer to write the clipboard text into.
		  * \param maxDestLen The total number of characters available in the destination buffer.
		  * \return Pointer to the first character in the clipboard text or nullptr otherwise.
		  */
		fpl_platform_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
		/**
		  * \brief Overwrites the current clipboard ansi text with the given one.
		  * \param ansiSource The new clipboard ansi string.
		  * \return Returns true when the text in the clipboard was changed, otherwise false.
		  */
		fpl_platform_api bool SetClipboardText(const char *ansiSource);
		/**
		  * \brief Overwrites the current clipboard wide text with the given one.
		  * \param wideSource The new clipboard wide string.
		  * \return Returns true when the text in the clipboard was changed, otherwise false.
		  */
		fpl_platform_api bool SetClipboardText(const wchar_t *wideSource);

		/** \}*/
	};
#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_VIDEO)
	//! Video context access
	namespace video {
		/**
		  * \defgroup Video Video functions
		  * \brief Functions for retrieving or resizing the video buffer
		  * \{
		  */

		//! Video rectangle
		struct VideoRect {
			//! Left position in pixels
			int32_t x;
			//! Top position in pixels
			int32_t y;
			//! Width in pixels
			int32_t width;
			//! Height in pixels
			int32_t height;
		};

		/**
		  * \brief Makes a video rectangle from a LT-RB rectangle
		  * \param left Left position in screen units.
		  * \param top Top position in screen units.
		  * \param right Right position in screen units.
		  * \param bottom Bottom position in screen units.
		  * \return Computed video rectangle
		  */
		fpl_inline VideoRect CreateVideoRectFromLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
			VideoRect result = { left, top, (right - left) + 1, (bottom - top) + 1 };
			return(result);
		}

		/**
		  * \brief Returns the string for the given video driver
		  * \param driver The audio driver
		  * \return String for the given audio driver
		  */
		fpl_inline const char *GetVideoDriverString(VideoDriverType driver) {
			fpl_constant char *VIDEO_DRIVER_TYPE_STRINGS[] = {
				"None",
				"OpenGL",
				"Software",
			};
			uint32_t index = (uint32_t)driver;
			FPL_ASSERT(index < FPL_ARRAYCOUNT(VIDEO_DRIVER_TYPE_STRINGS));
			const char *result = VIDEO_DRIVER_TYPE_STRINGS[index];
			return(result);
		}


		//! Video backbuffer container. Use this for accessing the pixels directly. Use with care!
		struct VideoBackBuffer {
			//! The 32-bit pixel top-down array, format: 0xAABBGGRR. Do not modify before WindowUpdate
			uint32_t *pixels;
			//! The width of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t width;
			//! The height of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t height;
			//! The size of one entire pixel with all 4 components in bytes. Do not modify, it will be set automatically.
			size_t pixelStride;
			//! The width of one line in bytes. Do not modify, it will be set automatically.
			size_t lineWidth;
			//! The output rectangle for displaying the backbuffer (Size may not match backbuffer size!)
			VideoRect outputRect;
			//! Set this to true to actually use the output rectangle
			bool useOutputRect;
		};

		/**
		  * \brief Returns the pointer to the video software context.
		  * \warning Do not release this memory by any means, otherwise you will corrupt heap memory!
		  * \return Pointer to the video backbuffer.
		  */
		fpl_common_api VideoBackBuffer *GetVideoBackBuffer();
		/**
		  * \brief Resizes the current video backbuffer.
		  * \param width Width in pixels.
		  * \param height Height in pixels.
		  * \return Returns true when video back buffer could be resized or false otherwise.
		  */
		fpl_common_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height);
		/**
		  * \brief Returns the current video driver type used.
		  * \return The current video driver type used.
		  */
		fpl_common_api VideoDriverType GetVideoDriver();

		/**
		  * \brief Forces the window to redraw or to swap the back/front buffer.
		  */
		fpl_common_api void VideoFlip();

		/** \}*/
	};
#endif // FPL_ENABLE_VIDEO

#if defined(FPL_ENABLE_AUDIO)
	//! Audio functions
	namespace audio {
		/**
		  * \defgroup Audio Audio functions
		  * \brief Functions for start/stop playing audio and retrieving/changing some audio related settings.
		  * \{
		  */

		  //! Audio result
		enum class AudioResult {
			Success,
			DeviceNotInitialized,
			DeviceAlreadyStopped,
			DeviceAlreadyStarted,
			DeviceBusy,
			Failed,
		};

		/**
		  * \brief Start playing asyncronous audio.
		  * \return Audio result code.
		  */
		fpl_common_api AudioResult PlayAudio();
		/**
		  * \brief Stop playing asyncronous audio.
		  * \return Audio result code.
		  */
		fpl_common_api AudioResult StopAudio();
		/**
		  * \brief Returns the native format for the current audio device.
		  * \return Copy fo the audio device format.
		  */
		fpl_common_api AudioDeviceFormat GetAudioHardwareFormat();
		/**
		  * \brief Overwrites the audio client read callback.
		  * \param newCallback Pointer to the client read callback.
		  * \param userData Pointer to the client/user data.
		  * \note This has no effect when audio is already playing, you have to call it when audio is in a stopped state!
		  */
		fpl_common_api void SetAudioClientReadCallback(AudioClientReadFunction *newCallback, void *userData);
		/**
		  * \brief Gets all playback audio devices.
		  * \param devices Target device id array.
		  * \param maxDeviceCount Total number of devices available in the devices array.
		  * \return Number of devices found.
		  */
		fpl_common_api uint32_t GetAudioDevices(AudioDeviceID *devices, uint32_t maxDeviceCount);

		/**
		  * \brief Returns the number of bytes required to write one sample with one channel
		  * \param format The audio format
		  * \return Number of bytes for one sample with one channel
		  */
		fpl_inline uint32_t GetAudioSampleSizeInBytes(const AudioFormatType format) {
			switch(format) {
				case AudioFormatType::U8:
					return 1;
				case AudioFormatType::S16:
					return 2;
				case AudioFormatType::S24:
					return 3;
				case AudioFormatType::S32:
				case AudioFormatType::F32:
					return 4;
				case AudioFormatType::S64:
				case AudioFormatType::F64:
					return 8;
				default:
					return 0;
			}
		}

		/**
		  * \brief Returns the string for the given format type
		  * \param format The audio format
		  * \return String for the given format type
		  */
		fpl_inline const char *GetAudioFormatString(const AudioFormatType format) {
			// @NOTE(final): Order must be equal to the AudioFormatType enum!
			fpl_constant char *AUDIO_FORMAT_TYPE_STRINGS[] = {
				"None",
				"U8",
				"S16",
				"S24",
				"S32",
				"S64",
				"F32",
				"F64",
			};
			uint32_t index = (uint32_t)format;
			FPL_ASSERT(index < FPL_ARRAYCOUNT(AUDIO_FORMAT_TYPE_STRINGS));
			const char *result = AUDIO_FORMAT_TYPE_STRINGS[index];
			return(result);
		}

		/**
		  * \brief Returns the string for the given audio driver
		  * \param driver The audio driver
		  * \return String for the given audio driver
		  */
		fpl_inline const char *GetAudioDriverString(AudioDriverType driver) {
			fpl_constant char *AUDIO_DRIVER_TYPE_STRINGS[] = {
				"None",
				"Auto",
				"DirectSound",
			};
			uint32_t index = (uint32_t)driver;
			FPL_ASSERT(index < FPL_ARRAYCOUNT(AUDIO_DRIVER_TYPE_STRINGS));
			const char *result = AUDIO_DRIVER_TYPE_STRINGS[index];
			return(result);
		}

		/**
		  * \brief Returns the total frame count for given sample rate and buffer size in milliseconds
		  * \param sampleRate The sample rate in Hz
		  * \param bufferSizeInMilliSeconds The buffer size in number of milliseconds
		  * \return Number of frames
		  */
		fpl_inline uint32_t GetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds) {
			uint32_t result = (sampleRate / 1000) * bufferSizeInMilliSeconds;
			return(result);
		}

		/**
		  * \brief Returns the number of bytes required for one interleaved audio frame - containing all the channels
		  * \param format The audio format
		  * \param channelCount The number of channels
		  * \return Number of bytes for one frame in bytes
		  */
		fpl_inline uint32_t GetAudioFrameSizeInBytes(const AudioFormatType format, const uint32_t channelCount) {
			uint32_t result = GetAudioSampleSizeInBytes(format) * channelCount;
			return(result);
		}

		/**
		  * \brief Returns the total number of bytes for the buffer and the given parameters
		  * \param format The audio format
		  * \param channelCount The number of channels
		  * \param frameCount The number of frames
		  * \return Total number of bytes for the buffer
		  */
		fpl_inline uint32_t GetAudioBufferSizeInBytes(const AudioFormatType format, const uint32_t channelCount, const uint32_t frameCount) {
			uint32_t frameSize = GetAudioFrameSizeInBytes(format, channelCount);
			uint32_t result = frameSize * frameCount;
			return(result);
		}

		/** \}*/
	};
#endif // FPL_ENABLE_AUDIO
}

// Expand all namespaces if the callers wants this
#if defined(FPL_ENABLE_AUTO_NAMESPACE)
using namespace fpl;
#	if defined(FPL_ENABLE_WINDOW)
using namespace fpl::window;
using namespace fpl::video;
#	endif
#	if defined(FPL_ENABLE_AUDIO)
using namespace fpl::audio;
#	endif
using namespace fpl::atomics;
using namespace fpl::hardware;
using namespace fpl::memory;
using namespace fpl::timings;
using namespace fpl::paths;
using namespace fpl::files;
using namespace fpl::library;
using namespace fpl::strings;
using namespace fpl::console;
using namespace fpl::threading;
#endif // FPL_ENABLE_AUTO_NAMESPACE

#endif // FPL_INCLUDE_HPP

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#define FPL_IMPLEMENTED

//
// Main entry point forward declarations
//
#if defined(FPL_PLATFORM_WIN32)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_main int main(int argc, char *args[]);
#endif // FPL_PLATFORM_WIN32

// ****************************************************************************
//
// Platform constants
//
// ****************************************************************************
namespace fpl {
	namespace platform {
#	if defined(FPL_PLATFORM_WIN32)
		fpl_constant char PATH_SEPARATOR = '\\';
		fpl_constant char FILE_EXT_SEPARATOR = '.';
#	else
		fpl_constant char PATH_SEPARATOR = '/';
		fpl_constant char FILE_EXT_SEPARATOR = '.';
#	endif

		// Forward declarations
		struct PlatformAppState;
		fpl_globalvar PlatformAppState* global__AppState = nullptr;

		// Prepare window for video callback
#		define FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_AFTER(name) bool name(const VideoSettings &videoSettings, platform::PlatformAppState *appState)
		typedef FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_AFTER(callback_PrepareWindowForVideoAfter);
	}
}

// ****************************************************************************
//
// Win32 Types
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)
#	include <windowsx.h>	// Macros for window messages
#	include <shlobj.h>		// SHGetFolderPath
#	include <intrin.h>		// Interlock*
#	include <xinput.h>		// XInputGetState

namespace fpl {
	namespace platform {
		//
		// XInput
		//
#		define FPL_XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
		typedef FPL_XINPUT_GET_STATE(win32_func_XInputGetState);
		FPL_XINPUT_GET_STATE(Win32XInputGetStateStub) {
			return(ERROR_DEVICE_NOT_CONNECTED);
		}
#		define FPL_XINPUT_GET_CAPABILITIES(name) DWORD WINAPI name(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
		typedef FPL_XINPUT_GET_CAPABILITIES(win32_func_XInputGetCapabilities);
		FPL_XINPUT_GET_CAPABILITIES(Win32XInputGetCapabilitiesStub) {
			return(ERROR_DEVICE_NOT_CONNECTED);
		}
		struct Win32XInputFunctions {
			HMODULE xinputLibrary;
			win32_func_XInputGetState *xInputGetState = Win32XInputGetStateStub;
			win32_func_XInputGetCapabilities *xInputGetCapabilities = Win32XInputGetCapabilitiesStub;
		};

		//
		// WINAPI functions
		//

		// GDI32
#		define FPL_FUNC_CHOOSE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
		typedef FPL_FUNC_CHOOSE_PIXEL_FORMAT(win32_func_ChoosePixelFormat);
#		define FPL_FUNC_SET_PIXEL_FORMAT(name) BOOL WINAPI name(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
		typedef FPL_FUNC_SET_PIXEL_FORMAT(win32_func_SetPixelFormat);
#		define FPL_FUNC_DESCRIPE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
		typedef FPL_FUNC_DESCRIPE_PIXEL_FORMAT(win32_func_DescribePixelFormat);
#		define FPL_FUNC_GET_DEVICE_CAPS(name) int WINAPI name(HDC hdc, int index)
		typedef FPL_FUNC_GET_DEVICE_CAPS(win32_func_GetDeviceCaps);
#		define FPL_FUNC_STRETCH_DIBITS(name) int WINAPI name(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, CONST VOID *lpBits, CONST BITMAPINFO *lpbmi, UINT iUsage, DWORD rop)
		typedef FPL_FUNC_STRETCH_DIBITS(win32_func_StretchDIBits);
#		define FPL_FUNC_DELETE_OBJECT(name) BOOL WINAPI name( _In_ HGDIOBJ ho)
		typedef FPL_FUNC_DELETE_OBJECT(win32_func_DeleteObject);
#		define FPL_FUNC_SWAP_BUFFERS(name) BOOL WINAPI name(HDC)
		typedef FPL_FUNC_SWAP_BUFFERS(win32_func_SwapBuffers);

		// ShellAPI
#		define FPL_FUNC_COMMAND_LINE_TO_ARGV_W(name) LPWSTR* WINAPI name(LPCWSTR lpCmdLine, int *pNumArgs)
		typedef FPL_FUNC_COMMAND_LINE_TO_ARGV_W(win32_func_CommandLineToArgvW);
#		define FPL_FUNC_SH_GET_FOLDER_PATH_A(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
		typedef FPL_FUNC_SH_GET_FOLDER_PATH_A(win32_func_SHGetFolderPathA);
#		define FPL_FUNC_SH_GET_FOLDER_PATH_W(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
		typedef FPL_FUNC_SH_GET_FOLDER_PATH_W(win32_func_SHGetFolderPathW);

		// User32
#		define FPL_FUNC_REGISTER_CLASS_EX_A(name) ATOM WINAPI name(CONST WNDCLASSEXA *)
		typedef FPL_FUNC_REGISTER_CLASS_EX_A(win32_func_RegisterClassExA);
#		define FPL_FUNC_REGISTER_CLASS_EX_W(name) ATOM WINAPI name(CONST WNDCLASSEXW *)
		typedef FPL_FUNC_REGISTER_CLASS_EX_W(win32_func_RegisterClassExW);
#		define FPL_FUNC_UNREGISTER_CLASS_EX_A(name) BOOL WINAPI name(LPCSTR lpClassName, HINSTANCE hInstance)
		typedef FPL_FUNC_UNREGISTER_CLASS_EX_A(win32_func_UnregisterClassA);
#		define FPL_FUNC_UNREGISTER_CLASS_EX_W(name) BOOL WINAPI name(LPCWSTR lpClassName, HINSTANCE hInstance)
		typedef FPL_FUNC_UNREGISTER_CLASS_EX_W(win32_func_UnregisterClassW);
#		define FPL_FUNC_SHOW_WINDOW(name) BOOL WINAPI name(HWND hWnd, int nCmdShow)
		typedef FPL_FUNC_SHOW_WINDOW(win32_func_ShowWindow);
#		define FPL_FUNC_DESTROY_WINDOW(name) BOOL WINAPI name(HWND hWnd)
		typedef FPL_FUNC_DESTROY_WINDOW(win32_func_DestroyWindow);
#		define FPL_FUNC_UPDATE_WINDOW(name) BOOL WINAPI name(HWND hWnd)
		typedef FPL_FUNC_UPDATE_WINDOW(win32_func_UpdateWindow);
#		define FPL_FUNC_TRANSLATE_MESSAGE(name) BOOL WINAPI name(CONST MSG *lpMsg)
		typedef FPL_FUNC_TRANSLATE_MESSAGE(win32_func_TranslateMessage);
#		define FPL_FUNC_DISPATCH_MESSAGE_A(name) LRESULT WINAPI name(CONST MSG *lpMsg)
		typedef FPL_FUNC_DISPATCH_MESSAGE_A(win32_func_DispatchMessageA);
#		define FPL_FUNC_DISPATCH_MESSAGE_W(name) LRESULT WINAPI name(CONST MSG *lpMsg)
		typedef FPL_FUNC_DISPATCH_MESSAGE_W(win32_func_DispatchMessageW);
#		define FPL_FUNC_PEEK_MESSAGE_A(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
		typedef FPL_FUNC_PEEK_MESSAGE_A(win32_func_PeekMessageA);
#		define FPL_FUNC_PEEK_MESSAGE_W(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
		typedef FPL_FUNC_PEEK_MESSAGE_W(win32_func_PeekMessageW);
#		define FPL_FUNC_DEF_WINDOW_PROC_A(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		typedef FPL_FUNC_DEF_WINDOW_PROC_A(win32_func_DefWindowProcA);
#		define FPL_FUNC_DEF_WINDOW_PROC_W(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		typedef FPL_FUNC_DEF_WINDOW_PROC_W(win32_func_DefWindowProcW);
#		define FPL_FUNC_CREATE_WINDOW_EX_W(name) HWND WINAPI name(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
		typedef FPL_FUNC_CREATE_WINDOW_EX_W(win32_func_CreateWindowExW);
#		define FPL_FUNC_CREATE_WINDOW_EX_A(name) HWND WINAPI name(DWORD dwExStyle, LPCSTR lpClassName, PCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
		typedef FPL_FUNC_CREATE_WINDOW_EX_A(win32_func_CreateWindowExA);
#		define FPL_FUNC_SET_WINDOW_POS(name) BOOL WINAPI name(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
		typedef FPL_FUNC_SET_WINDOW_POS(win32_func_SetWindowPos);
#		define FPL_FUNC_GET_WINDOW_PLACEMENT(name) BOOL WINAPI name(HWND hWnd, WINDOWPLACEMENT *lpwndpl)
		typedef FPL_FUNC_GET_WINDOW_PLACEMENT(win32_func_GetWindowPlacement);
#		define FPL_FUNC_SET_WINDOW_PLACEMENT(name) BOOL WINAPI name(HWND hWnd, CONST WINDOWPLACEMENT *lpwndpl)
		typedef FPL_FUNC_SET_WINDOW_PLACEMENT(win32_func_SetWindowPlacement);
#		define FPL_FUNC_GET_CLIENT_RECT(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
		typedef FPL_FUNC_GET_CLIENT_RECT(win32_func_GetClientRect);
#		define FPL_FUNC_GET_WINDOW_RECT(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
		typedef FPL_FUNC_GET_WINDOW_RECT(win32_func_GetWindowRect);
#		define FPL_FUNC_ADJUST_WINDOW_RECT(name) BOOL WINAPI name(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
		typedef FPL_FUNC_ADJUST_WINDOW_RECT(win32_func_AdjustWindowRect);
#		define FPL_FUNC_GET_ASYNC_KEY_STATE(name) SHORT WINAPI name(int vKey)
		typedef FPL_FUNC_GET_ASYNC_KEY_STATE(win32_func_GetAsyncKeyState);
#		define FPL_FUNC_MAP_VIRTUAL_KEY_A(name) UINT WINAPI name(UINT uCode, UINT uMapType)
		typedef FPL_FUNC_MAP_VIRTUAL_KEY_A(win32_func_MapVirtualKeyA);
#		define FPL_FUNC_MAP_VIRTUAL_KEY_W(name) UINT WINAPI name(UINT uCode, UINT uMapType)
		typedef FPL_FUNC_MAP_VIRTUAL_KEY_W(win32_func_MapVirtualKeyW);
#		define FPL_FUNC_SET_CURSOR(name) HCURSOR WINAPI name(HCURSOR hCursor)
		typedef FPL_FUNC_SET_CURSOR(win32_func_SetCursor);
#		define FPL_FUNC_GET_CURSOR(name) HCURSOR WINAPI name(VOID)
		typedef FPL_FUNC_GET_CURSOR(win32_func_GetCursor);
#		define FPL_FUNC_LOAD_CURSOR_A(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCSTR lpCursorName)
		typedef FPL_FUNC_LOAD_CURSOR_A(win32_func_LoadCursorA);
#		define FPL_FUNC_LOAD_CURSOR_W(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCWSTR lpCursorName)
		typedef FPL_FUNC_LOAD_CURSOR_W(win32_func_LoadCursorW);
#		define FPL_FUNC_LOAD_ICON_A(name) HICON WINAPI name(HINSTANCE hInstance, LPCSTR lpIconName)
		typedef FPL_FUNC_LOAD_ICON_A(win32_func_LoadIconA);
#		define FPL_FUNC_LOAD_ICON_W(name) HICON WINAPI name(HINSTANCE hInstance, LPCWSTR lpIconName)
		typedef FPL_FUNC_LOAD_ICON_W(win32_func_LoadIconW);
#		define FPL_FUNC_SET_WINDOW_TEXT_A(name) BOOL WINAPI name(HWND hWnd, LPCSTR lpString)
		typedef FPL_FUNC_SET_WINDOW_TEXT_A(win32_func_SetWindowTextA);
#		define FPL_FUNC_SET_WINDOW_TEXT_W(name) BOOL WINAPI name(HWND hWnd, LPCWSTR lpString)
		typedef FPL_FUNC_SET_WINDOW_TEXT_W(win32_func_SetWindowTextW);
#		define FPL_FUNC_SET_WINDOW_LONG_A(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
		typedef FPL_FUNC_SET_WINDOW_LONG_A(win32_func_SetWindowLongA);
#		define FPL_FUNC_SET_WINDOW_LONG_W(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
		typedef FPL_FUNC_SET_WINDOW_LONG_W(win32_func_SetWindowLongW);
#		define FPL_FUNC_GET_WINDOW_LONG_A(name) LONG WINAPI name(HWND hWnd, int nIndex)
		typedef FPL_FUNC_GET_WINDOW_LONG_A(win32_func_GetWindowLongA);
#		define FPL_FUNC_GET_WINDOW_LONG_W(name) LONG WINAPI name(HWND hWnd, int nIndex)
		typedef FPL_FUNC_GET_WINDOW_LONG_W(win32_func_GetWindowLongW);

#	if defined(FPL_ARCH_X64)
#		define FPL_FUNC_SET_WINDOW_LONG_PTR_A(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
		typedef FPL_FUNC_SET_WINDOW_LONG_PTR_A(win32_func_SetWindowLongPtrA);
#		define FPL_FUNC_SET_WINDOW_LONG_PTR_W(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
		typedef FPL_FUNC_SET_WINDOW_LONG_PTR_W(win32_func_SetWindowLongPtrW);
#		define FPL_FUNC_GET_WINDOW_LONG_PTR_A(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
		typedef FPL_FUNC_GET_WINDOW_LONG_PTR_A(win32_func_GetWindowLongPtrA);
#		define FPL_FUNC_GET_WINDOW_LONG_PTR_W(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
		typedef FPL_FUNC_GET_WINDOW_LONG_PTR_W(win32_func_GetWindowLongPtrW);
#	endif

#		define FPL_FUNC_RELEASE_DC(name) int WINAPI name(HWND hWnd, HDC hDC)
		typedef FPL_FUNC_RELEASE_DC(win32_func_ReleaseDC);
#		define FPL_FUNC_GET_DC(name) HDC WINAPI name(HWND hWnd)
		typedef FPL_FUNC_GET_DC(win32_func_GetDC);
#		define FPL_FUNC_CHANGE_DISPLAY_SETTINGS_A(name) LONG WINAPI name(DEVMODEA* lpDevMode, DWORD dwFlags)
		typedef FPL_FUNC_CHANGE_DISPLAY_SETTINGS_A(win32_func_ChangeDisplaySettingsA);
#		define FPL_FUNC_CHANGE_DISPLAY_SETTINGS_W(name) LONG WINAPI name(DEVMODEW* lpDevMode, DWORD dwFlags)
		typedef FPL_FUNC_CHANGE_DISPLAY_SETTINGS_W(win32_func_ChangeDisplaySettingsW);
#		define FPL_FUNC_ENUM_DISPLAY_SETTINGS_A(name) BOOL WINAPI name(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode)
		typedef FPL_FUNC_ENUM_DISPLAY_SETTINGS_A(win32_func_EnumDisplaySettingsA);
#		define FPL_FUNC_ENUM_DISPLAY_SETTINGS_W(name) BOOL WINAPI name(LPCWSTR lpszDeviceName, DWORD iModeNum, DEVMODEW* lpDevMode)
		typedef FPL_FUNC_ENUM_DISPLAY_SETTINGS_W(win32_func_EnumDisplaySettingsW);

#		define FPL_FUNC_OPEN_CLIPBOARD(name) BOOL WINAPI name(HWND hWndNewOwner)
		typedef FPL_FUNC_OPEN_CLIPBOARD(win32_func_OpenClipboard);
#		define FPL_FUNC_CLOSE_CLIPBOARD(name) BOOL WINAPI name(VOID)
		typedef FPL_FUNC_CLOSE_CLIPBOARD(win32_func_CloseClipboard);
#		define FPL_FUNC_EMPTY_CLIPBOARD(name) BOOL WINAPI name(VOID)
		typedef FPL_FUNC_EMPTY_CLIPBOARD(win32_func_EmptyClipboard);
#		define FPL_FUNC_IS_CLIPBOARD_FORMAT_AVAILABLE(name) BOOL WINAPI name(UINT format)
		typedef FPL_FUNC_IS_CLIPBOARD_FORMAT_AVAILABLE(win32_func_IsClipboardFormatAvailable);
#		define FPL_FUNC_SET_CLIPBOARD_DATA(name) HANDLE WINAPI name(UINT uFormat, HANDLE hMem)
		typedef FPL_FUNC_SET_CLIPBOARD_DATA(win32_func_SetClipboardData);
#		define FPL_FUNC_GET_CLIPBOARD_DATA(name) HANDLE WINAPI name(UINT uFormat)
		typedef FPL_FUNC_GET_CLIPBOARD_DATA(win32_func_GetClipboardData);

#		define FPL_FUNC_GET_DESKTOP_WINDOW(name) HWND WINAPI name(VOID)
		typedef FPL_FUNC_GET_DESKTOP_WINDOW(win32_func_GetDesktopWindow);
#		define FPL_FUNC_GET_FOREGROUND_WINDOW(name) HWND WINAPI name(VOID)
		typedef FPL_FUNC_GET_FOREGROUND_WINDOW(win32_func_GetForegroundWindow);

#		define FPL_FUNC_IS_ZOOMED(name) BOOL WINAPI name(HWND hWnd)
		typedef FPL_FUNC_IS_ZOOMED(win32_func_IsZoomed);
#		define FPL_FUNC_SEND_MESSAGE_A(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		typedef FPL_FUNC_SEND_MESSAGE_A(win32_func_SendMessageA);
#		define FPL_FUNC_SEND_MESSAGE_W(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
		typedef FPL_FUNC_SEND_MESSAGE_W(win32_func_SendMessageW);
#		define FPL_FUNC_GET_MONITOR_INFO_A(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
		typedef FPL_FUNC_GET_MONITOR_INFO_A(win32_func_GetMonitorInfoA);
#		define FPL_FUNC_GET_MONITOR_INFO_W(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
		typedef FPL_FUNC_GET_MONITOR_INFO_W(win32_func_GetMonitorInfoW);
#		define FPL_FUNC_MONITOR_FROM_RECT(name) HMONITOR WINAPI name(LPCRECT lprc, DWORD dwFlags)
		typedef FPL_FUNC_MONITOR_FROM_RECT(win32_func_MonitorFromRect);
#		define FPL_FUNC_MONITOR_FROM_WINDOW(name) HMONITOR WINAPI name(HWND hwnd, DWORD dwFlags)
		typedef FPL_FUNC_MONITOR_FROM_WINDOW(win32_func_MonitorFromWindow);

		// OLE32
#		define FPL_FUNC_WIN32_CO_INITIALIZE_EX(name) HRESULT WINAPI name(LPVOID pvReserved, DWORD  dwCoInit)
		typedef FPL_FUNC_WIN32_CO_INITIALIZE_EX(win32_func_CoInitializeEx);
#		define FPL_FUNC_WIN32_CO_UNINITIALIZE(name) void WINAPI name(void)
		typedef FPL_FUNC_WIN32_CO_UNINITIALIZE(win32_func_CoUninitialize);
#		define FPL_FUNC_WIN32_CO_CREATE_INSTANCE(name) HRESULT WINAPI name(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
		typedef FPL_FUNC_WIN32_CO_CREATE_INSTANCE(win32_func_CoCreateInstance);
#		define FPL_FUNC_WIN32_CO_TASK_MEM_FREE(name) void WINAPI name(LPVOID pv)
		typedef FPL_FUNC_WIN32_CO_TASK_MEM_FREE(win32_func_CoTaskMemFree);
#		define FPL_FUNC_WIN32_PROP_VARIANT_CLEAR(name) HRESULT WINAPI name(PROPVARIANT *pvar)
		typedef FPL_FUNC_WIN32_PROP_VARIANT_CLEAR(win32_func_PropVariantClear);

		struct Win32APIFunctions {
			struct {
				HMODULE gdiLibrary;
				win32_func_ChoosePixelFormat *choosePixelFormat;
				win32_func_SetPixelFormat *setPixelFormat;
				win32_func_DescribePixelFormat *describePixelFormat;
				win32_func_GetDeviceCaps *getDeviceCaps;
				win32_func_StretchDIBits *stretchDIBits;
				win32_func_DeleteObject *deleteObject;
				win32_func_SwapBuffers *swapBuffers;
			} gdi;

			struct {
				HMODULE shellLibrary;
				win32_func_CommandLineToArgvW *commandLineToArgvW;
				win32_func_SHGetFolderPathA *shGetFolderPathA;
				win32_func_SHGetFolderPathW *shGetFolderPathW;
			} shell;

			struct {
				HMODULE userLibrary;
				win32_func_RegisterClassExA *registerClassExA;
				win32_func_RegisterClassExW *registerClassExW;
				win32_func_UnregisterClassA *unregisterClassA;
				win32_func_UnregisterClassW *unregisterClassW;
				win32_func_ShowWindow *showWindow;
				win32_func_DestroyWindow *destroyWindow;
				win32_func_UpdateWindow *updateWindow;
				win32_func_TranslateMessage *translateMessage;
				win32_func_DispatchMessageA *dispatchMessageA;
				win32_func_DispatchMessageW *dispatchMessageW;
				win32_func_PeekMessageA *peekMessageA;
				win32_func_PeekMessageW *peekMessageW;
				win32_func_DefWindowProcA *defWindowProcA;
				win32_func_DefWindowProcW *defWindowProcW;
				win32_func_CreateWindowExA *createWindowExA;
				win32_func_CreateWindowExW *createWindowExW;
				win32_func_SetWindowPos *setWindowPos;
				win32_func_GetWindowPlacement *getWindowPlacement;
				win32_func_SetWindowPlacement *setWindowPlacement;
				win32_func_GetClientRect *getClientRect;
				win32_func_GetWindowRect *getWindowRect;
				win32_func_AdjustWindowRect *adjustWindowRect;
				win32_func_GetAsyncKeyState *getAsyncKeyState;
				win32_func_MapVirtualKeyA *mapVirtualKeyA;
				win32_func_MapVirtualKeyW *mapVirtualKeyW;
				win32_func_SetCursor *setCursor;
				win32_func_GetCursor *getCursor;
				win32_func_LoadCursorA *loadCursorA;
				win32_func_LoadCursorW *loadCursorW;
				win32_func_LoadIconA *loadIconA;
				win32_func_LoadIconW *loadIconW;
				win32_func_SetWindowTextA *setWindowTextA;
				win32_func_SetWindowTextW *setWindowTextW;
				win32_func_SetWindowLongA *setWindowLongA;
				win32_func_SetWindowLongW *setWindowLongW;
				win32_func_GetWindowLongA *getWindowLongA;
				win32_func_GetWindowLongW *getWindowLongW;
#			if defined(FPL_ARCH_X64)
				win32_func_SetWindowLongPtrA *setWindowLongPtrA;
				win32_func_SetWindowLongPtrW *setWindowLongPtrW;
				win32_func_GetWindowLongPtrA *getWindowLongPtrA;
				win32_func_GetWindowLongPtrW *getWindowLongPtrW;
#			endif
				win32_func_ReleaseDC *releaseDC;
				win32_func_GetDC *getDC;
				win32_func_ChangeDisplaySettingsA *changeDisplaySettingsA;
				win32_func_ChangeDisplaySettingsW *changeDisplaySettingsW;
				win32_func_EnumDisplaySettingsA *enumDisplaySettingsA;
				win32_func_EnumDisplaySettingsW *enumDisplaySettingsW;

				win32_func_OpenClipboard *openClipboard;
				win32_func_CloseClipboard *closeClipboard;
				win32_func_EmptyClipboard *emptyClipboard;
				win32_func_IsClipboardFormatAvailable *isClipboardFormatAvailable;
				win32_func_SetClipboardData *setClipboardData;
				win32_func_GetClipboardData *getClipboardData;

				win32_func_GetDesktopWindow *getDesktopWindow;
				win32_func_GetForegroundWindow *getForegroundWindow;
				win32_func_IsZoomed *isZoomed;
				win32_func_SendMessageA *sendMessageA;
				win32_func_SendMessageW *sendMessageW;
				win32_func_GetMonitorInfoA *getMonitorInfoA;
				win32_func_GetMonitorInfoW *getMonitorInfoW;
				win32_func_MonitorFromRect *monitorFromRect;
				win32_func_MonitorFromWindow *monitorFromWindow;
			} user;

			struct {
				HMODULE oleLibrary;
				win32_func_CoInitializeEx *coInitializeEx;
				win32_func_CoUninitialize *coUninitialize;
				win32_func_CoCreateInstance *coCreateInstance;
				win32_func_CoTaskMemFree *coTaskMemFree;
				win32_func_PropVariantClear *propVariantClear;
			} ole;
		};

		// Unicode dependent function calls and types
#	if !defined(UNICODE)
#		define FPL_WIN32_CLASSNAME "FPLWindowClassA"
#		define FPL_WIN32_UNNAMED_WINDOW "Unnamed FPL Ansi Window"
		typedef char win32_char;
#		define win32_copyString strings::CopyAnsiString
#		define win32_getStringLength strings::GetAnsiStringLength
#		define win32_ansiToString strings::CopyAnsiString
#		define win32_wndclassex WNDCLASSEXA
#		if defined(FPL_ARCH_X64)
#			define win32_setWindowLongPtr global__AppState->win32.winApi.user.setWindowLongPtrA
#		else
#			define win32_setWindowLongPtr global__AppState->win32.winApi.user.setWindowLongA
#		endif
#		define win32_setWindowLong global__AppState->win32.winApi.user.setWindowLongA
#		define win32_getWindowLong global__AppState->win32.winApi.user.getWindowLongA
#		define win32_peekMessage global__AppState->win32.winApi.user.peekMessageA
#		define win32_dispatchMessage global__AppState->win32.winApi.user.dispatchMessageA
#		define win32_defWindowProc global__AppState->win32.winApi.user.defWindowProcA
#		define win32_registerClassEx global__AppState->win32.winApi.user.registerClassExA
#		define win32_unregisterClass global__AppState->win32.winApi.user.unregisterClassA
#		define win32_createWindowEx global__AppState->win32.winApi.user.createWindowExA
#		define win32_loadIcon global__AppState->win32.winApi.user.loadIconA
#		define win32_loadCursor global__AppState->win32.winApi.user.loadCursorA
#		define win32_sendMessage global__AppState->win32.winApi.user.sendMessageA
#		define win32_getMonitorInfo global__AppState->win32.winApi.user.getMonitorInfoA
#	else
#		define FPL_WIN32_CLASSNAME L"FPLWindowClassW"
#		define FPL_WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
		typedef wchar_t win32_char;
#		define win32_copyString strings::CopyWideString
#		define win32_getStringLength strings::GetWideStringLength
#		define win32_ansiToString strings::AnsiStringToWideString
#		define win32_wndclassex WNDCLASSEXW
#		if defined(FPL_ARCH_X64)
#			define win32_setWindowLongPtr global__AppState->win32.winApi.user.setWindowLongPtrW
#		else
#			define win32_setWindowLongPtr global__AppState->win32.winApi.user.setWindowLongW
#		endif
#		define win32_setWindowLong global__AppState->win32.winApi.user.setWindowLongW
#		define win32_getWindowLong global__AppState->win32.winApi.user.getWindowLongW
#		define win32_peekMessage global__AppState->win32.winApi.user.peekMessageW
#		define win32_dispatchMessage global__AppState->win32.winApi.user.dispatchMessageW
#		define win32_defWindowProc global__AppState->win32.winApi.user.defWindowProcW
#		define win32_registerClassEx global__AppState->win32.winApi.user.registerClassExW
#		define win32_unregisterClass global__AppState->win32.winApi.user.unregisterClassW
#		define win32_createWindowEx global__AppState->win32.winApi.user.createWindowExW
#		define win32_loadIcon global__AppState->win32.winApi.user.loadIconW
#		define win32_loadCursor global__AppState->win32.winApi.user.loadCursorW
#		define win32_sendMessage global__AppState->win32.winApi.user.sendMessageW
#		define win32_getMonitorInfo global__AppState->win32.winApi.user.getMonitorInfoW
#	endif // UNICODE

		struct Win32XInputState {
			bool isConnected[XUSER_MAX_COUNT];
			LARGE_INTEGER lastDeviceSearchTime;
			Win32XInputFunctions xinputApi;
		};

		struct Win32InitState {
			HINSTANCE appInstance;
			LARGE_INTEGER performanceFrequency;
		};

		struct Win32AppState {
			Win32XInputState xinput;
			Win32APIFunctions winApi;
		};

#	if defined(FPL_ENABLE_WINDOW)
		struct Win32LastWindowInfo {
			WINDOWPLACEMENT placement;
			DWORD style;
			DWORD exStyle;
			bool isMaximized;
			bool wasResolutionChanged;
		};

		struct Win32WindowState {
			win32_char windowClass[256];
			HWND windowHandle;
			HDC deviceContext;
			HCURSOR defaultCursor;
			Win32LastWindowInfo lastFullscreenInfo;
			bool isRunning;
			bool isCursorActive;
		};
#	endif // FPL_ENABLE_WINDOW

	} // platform
} // fpl
#endif // FPL_PLATFORM_WIN32

// ****************************************************************************
//
// POSIX Types
//
// ****************************************************************************
#if defined(FPL_SUBPLATFORM_POSIX)
namespace fpl {
	namespace platform {
#       define FPL_FUNC_PTHREAD_CREATE(name) int name(pthread_t *, const pthread_attr_t *, void *(*__start_routine) (void *), void *)
		typedef FPL_FUNC_PTHREAD_CREATE(pthread_func_pthread_create);
#       define FPL_FUNC_PTHREAD_KILL(name) int name(pthread_t thread, int sig)
		typedef FPL_FUNC_PTHREAD_KILL(pthread_func_pthread_kill);
#       define FPL_FUNC_PTHREAD_JOIN(name) int name(pthread_t __th, void **__thread_return)
		typedef FPL_FUNC_PTHREAD_JOIN(pthread_func_pthread_join);
#       define FPL_FUNC_PTHREAD_EXIT(name) void name(void *__retval)
		typedef FPL_FUNC_PTHREAD_EXIT(pthread_func_pthread_exit);
#       define FPL_FUNC_PTHREAD_YIELD(name) int name(void)
		typedef FPL_FUNC_PTHREAD_YIELD(pthread_func_pthread_yield);

#		define FPL_FUNC_PTHREAD_MUTEX_INIT(name) int name(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
		typedef FPL_FUNC_PTHREAD_MUTEX_INIT(pthread_func_pthread_mutex_init);
#		define FPL_FUNC_PTHREAD_MUTEX_DESTROY(name) int name(pthread_mutex_t *mutex)
		typedef FPL_FUNC_PTHREAD_MUTEX_DESTROY(pthread_func_pthread_mutex_destroy);
#		define FPL_FUNC_PTHREAD_MUTEX_LOCK(name) int name(pthread_mutex_t *mutex)
		typedef FPL_FUNC_PTHREAD_MUTEX_LOCK(pthread_func_pthread_mutex_lock);
#		define FPL_FUNC_PTHREAD_MUTEX_TRYLOCK(name) int name(pthread_mutex_t *mutex)
		typedef FPL_FUNC_PTHREAD_MUTEX_TRYLOCK(pthread_func_pthread_mutex_trylock);
#		define FPL_FUNC_PTHREAD_MUTEX_UNLOCK(name) int name(pthread_mutex_t *mutex)
		typedef FPL_FUNC_PTHREAD_MUTEX_UNLOCK(pthread_func_pthread_mutex_unlock);

#		define FPL_FUNC_PTHREAD_COND_INIT(name) int name(pthread_cond_t *cond, const pthread_condattr_t *attr)
		typedef FPL_FUNC_PTHREAD_COND_INIT(pthread_func_pthread_cond_init);
#		define FPL_FUNC_PTHREAD_COND_DESTROY(name) int name(pthread_cond_t *cond)
		typedef FPL_FUNC_PTHREAD_COND_DESTROY(pthread_func_pthread_cond_destroy);
#		define FPL_FUNC_PTHREAD_COND_TIMEDWAIT(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
		typedef FPL_FUNC_PTHREAD_COND_TIMEDWAIT(pthread_func_pthread_cond_timedwait);
#		define FPL_FUNC_PTHREAD_COND_WAIT(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex)
		typedef FPL_FUNC_PTHREAD_COND_WAIT(pthread_func_pthread_cond_wait);
#		define FPL_FUNC_PTHREAD_COND_BROADCAST(name) int name(pthread_cond_t *cond)
		typedef FPL_FUNC_PTHREAD_COND_BROADCAST(pthread_func_pthread_cond_broadcast);
#		define FPL_FUNC_PTHREAD_COND_SIGNAL(name) int name(pthread_cond_t *cond)
		typedef FPL_FUNC_PTHREAD_COND_SIGNAL(pthread_func_pthread_cond_signal);


		struct PThreadAPI {
			void *libraryHandle;
			pthread_func_pthread_create *pthread_create;
			pthread_func_pthread_kill *pthread_kill;
			pthread_func_pthread_join *pthread_join;
			pthread_func_pthread_exit *pthread_exit;
			pthread_func_pthread_yield *pthread_yield;

			pthread_func_pthread_mutex_init *pthread_mutex_init;
			pthread_func_pthread_mutex_destroy *pthread_mutex_destroy;
			pthread_func_pthread_mutex_lock *pthread_mutex_lock;
			pthread_func_pthread_mutex_trylock *pthread_mutex_trylock;
			pthread_func_pthread_mutex_unlock *pthread_mutex_unlock;

			pthread_func_pthread_cond_init *pthread_cond_init;
			pthread_func_pthread_cond_destroy *pthread_cond_destroy;
			pthread_func_pthread_cond_timedwait *pthread_cond_timedwait;
			pthread_func_pthread_cond_wait *pthread_cond_wait;
			pthread_func_pthread_cond_broadcast *pthread_cond_broadcast;
			pthread_func_pthread_cond_signal *pthread_cond_signal;
		};


		struct POSIXInitState {
		};

		struct POSIXAppState {
			PThreadAPI pthreadApi;
		};
	}
}
#endif

// ****************************************************************************
//
// Linux Types
//
// ****************************************************************************
#if defined(FPL_PLATFORM_LINUX)
namespace fpl {
	namespace platform {
		struct LinuxInitState {
			POSIXInitState posix;
		};

		struct LinuxAppState {
			POSIXAppState posix;
		};
	}
}
#endif

// ****************************************************************************
//
// All Platforms & Forward Declarations
//
// ****************************************************************************
namespace fpl {
	namespace platform {
		//
		// Platform initialization state
		//
		struct PlatformInitState {
			union {
#		if defined(FPL_PLATFORM_WIN32)
				Win32InitState win32;
#		endif
#       if defined(FPL_PLATFORM_LINUX)
				LinuxInitState linux;
#       endif
			};
			bool isInitialized;
		};
		fpl_globalvar PlatformInitState global__InitState = {};

#	if defined(FPL_ENABLE_WINDOW)
		struct PlatformWindowState {
			union {
#			if defined(FPL_PLATFORM_WIN32)
				Win32WindowState win32;
#			endif
			};
		};
#	endif

#	if defined(FPL_ENABLE_VIDEO)
		struct PlatformVideoState {
			void *mem; // Points to common::VideoState
			size_t memSize;
		};
#	endif // FPL_ENABLE_VIDEO

#	if defined(FPL_ENABLE_AUDIO)
		struct PlatformAudioState {
			void *mem; // Points to common::AudioState
			size_t memSize;
		};
#	endif

		//
		// Platform application state
		//
		struct PlatformAppState {
			union {
#		if defined(FPL_PLATFORM_WIN32)
				Win32AppState win32;
#		endif
#       if defined(FPL_PLATFORM_LINUX)
				LinuxAppState linux;
#       endif
			};
#		if defined(FPL_ENABLE_WINDOW)
			PlatformWindowState window;
#		endif
#		if defined(FPL_ENABLE_VIDEO)
			PlatformVideoState video;
#		endif
#		if defined(FPL_ENABLE_AUDIO)
			PlatformAudioState audio;
#		endif
			Settings initSettings;
			Settings currentSettings;
			InitFlags initFlags;
		};

#	if defined(FPL_ENABLE_WINDOW)
		// @TODO(final): Move event queue into PlatformWindowState
		fpl_constant uint32_t MAX_EVENT_COUNT = 32768;
		struct EventQueue {
			window::Event events[MAX_EVENT_COUNT];
			volatile uint32_t pollIndex;
			volatile uint32_t pushCount;
		};
		fpl_globalvar EventQueue *global__EventQueue = nullptr;

		fpl_internal_inline void PushEvent(const window::Event &event) {
			EventQueue *eventQueue = global__EventQueue;
			FPL_ASSERT(eventQueue != nullptr);
			if(eventQueue->pushCount < MAX_EVENT_COUNT) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pushCount, 1);
				FPL_ASSERT(eventIndex < MAX_EVENT_COUNT);
				eventQueue->events[eventIndex] = event;
			}
		}
#endif // FPL_ENABLE_WINDOW

	} // platform

} // fpl

// ****************************************************************************
//
// Non-Platform specifics
//
// ****************************************************************************

// Standard includes
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h> // fprintf, vfprintf, vsnprintf, getchar

//
// Macros
//
#define FPL_CLEARMEM(T, memory, size, shift, mask) \
	do { \
		size_t clearedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = static_cast<T *>(memory); \
			T *dataBlockEnd = static_cast<T *>(memory) + (size >> shift); \
			while (dataBlock != dataBlockEnd) { \
				*dataBlock++ = 0; \
				clearedBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + clearedBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 != data8End) { \
			*data8++ = 0; \
		} \
	} while (0);

#define FPL_COPYMEM(T, source, sourceSize, dest, shift, mask) \
	do { \
		size_t copiedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *sourceDataBlock = static_cast<T *>(source); \
			T *sourceDataBlockEnd = static_cast<T *>(source) + (sourceSize >> shift); \
			T *destDataBlock = static_cast<T *>(dest); \
			while (sourceDataBlock != sourceDataBlockEnd) { \
				*destDataBlock++ = *sourceDataBlock++; \
				copiedBytes += sizeof(T); \
			} \
		} \
		uint8_t *sourceData8 = (uint8_t *)source + copiedBytes; \
		uint8_t *sourceData8End = (uint8_t *)source + sourceSize; \
		uint8_t *destData8 = (uint8_t *)dest + copiedBytes; \
		while (sourceData8 != sourceData8End) { \
			*destData8++ = *sourceData8++; \
		} \
	} while (0);

namespace fpl {
	namespace common {
		//
		// Internal types and functions
		//
		fpl_constant uint32_t MAX_LAST_ERROR_STRING_LENGTH = 1024;
#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		fpl_constant uint32_t MAX_ERRORSTATE_COUNT = 256;
#	else
		fpl_constant uint32_t MAX_ERRORSTATE_COUNT = 1;
#	endif

		struct ErrorState {
			char errors[MAX_ERRORSTATE_COUNT][MAX_LAST_ERROR_STRING_LENGTH];
			uint32_t count;
		};

		fpl_globalvar ErrorState global__LastErrorState;

#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		fpl_internal_inline void PushError_Formatted(const char *format, va_list &argList) {
			ErrorState &state = global__LastErrorState;
			FPL_ASSERT(format != nullptr);
			char buffer[MAX_LAST_ERROR_STRING_LENGTH];
			::vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
			uint32_t messageLen = strings::GetAnsiStringLength(buffer);
			FPL_ASSERT(state.count < MAX_ERRORSTATE_COUNT);
			size_t errorIndex = state.count;
			state.count = (state.count + 1) % MAX_ERRORSTATE_COUNT;
			strings::CopyAnsiString(buffer, messageLen, state.errors[errorIndex], MAX_LAST_ERROR_STRING_LENGTH);
#		if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
			console::ConsoleFormatError("FPL Error: %s\n", buffer);
#		endif
		}
#	else
		fpl_internal_inline void PushError_Formatted(const char *format, va_list &argList) {
			ErrorState &state = global__LastErrorState;
			FPL_ASSERT(format != nullptr);
			char buffer[MAX_LAST_ERROR_STRING_LENGTH];
			::vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
			uint32_t messageLen = strings::GetAnsiStringLength(buffer);
			state.count = 1;
			strings::CopyAnsiString(buffer, messageLen, state.errors[0], MAX_LAST_ERROR_STRING_LENGTH);
#		if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
			console::ConsoleFormatError("FPL Error: %s\n", buffer);
#		endif
		}
#	endif // FPL_ENABLE_MULTIPLE_ERRORSTATES

#if defined(FPL_ENABLE_AUDIO)
		struct CommonAudioState {
			AudioDeviceFormat internalFormat;
			AudioClientReadFunction *clientReadCallback;
			void *clientUserData;
		};

		fpl_internal_inline uint32_t ReadAudioFramesFromClient(const CommonAudioState &commonAudio, uint32_t frameCount, void *pSamples) {
			uint32_t outputSamplesWritten = 0;
			if(commonAudio.clientReadCallback != nullptr) {
				outputSamplesWritten = commonAudio.clientReadCallback(commonAudio.internalFormat, frameCount, pSamples, commonAudio.clientUserData);
			}
			return outputSamplesWritten;
		}
#endif // FPL_ENABLE_AUDIO

		fpl_internal_inline void PushError(const char *format, ...) {
			va_list valist;
			va_start(valist, format);
			PushError_Formatted(format, valist);
			va_end(valist);
		}

		// Maximum number of active threads you can have in your process
		fpl_constant uint32_t MAX_THREAD_COUNT = 64;

		// Maximum number of active signals you can wait for
		fpl_constant uint32_t MAX_SIGNAL_COUNT = 256;

		struct ThreadState {
			threading::ThreadContext mainThread;
			threading::ThreadContext threads[MAX_THREAD_COUNT];
		};

		fpl_globalvar ThreadState global__ThreadState = {};

		fpl_internal_inline threading::ThreadContext *GetFreeThreadContext() {
			threading::ThreadContext *result = nullptr;
			for(uint32_t index = 0; index < MAX_THREAD_COUNT; ++index) {
				threading::ThreadContext *thread = global__ThreadState.threads + index;
				threading::ThreadState state = threading::GetThreadState(thread);
				if(state == threading::ThreadState::Stopped) {
					result = thread;
					break;
				}
			}
			return(result);
		}
	} // common

	//
	// Common Strings
	//
	namespace strings {
		fpl_common_api bool IsStringEqual(const char *a, const uint32_t aLen, const char *b, const uint32_t bLen) {
			if((a == nullptr) || (b == nullptr)) {
				return (a == b);
			}
			if(aLen != bLen) {
				return false;
			}
			FPL_ASSERT(aLen == bLen);
			bool result = true;
			for(uint32_t index = 0; index < aLen; ++index) {
				char aChar = a[index];
				char bChar = b[index];
				if(aChar != bChar) {
					result = false;
					break;
				}
			}
			return(result);
		}

		fpl_common_api bool IsStringEqual(const char *a, const char *b) {
			if((a == nullptr) || (b == nullptr)) {
				return (a == b);
			}
			bool result = true;
			for(;;) {
				const char aChar = *(a++);
				const char bChar = *(b++);
				if(aChar == 0 || bChar == 0) {
					result = (aChar == bChar);
					break;
				}
				if(aChar != bChar) {
					result = false;
					break;
				}
			}
			return(result);
		}

		fpl_common_api uint32_t GetAnsiStringLength(const char *str) {
			uint32_t result = 0;
			if(str != nullptr) {
				while(*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_common_api uint32_t GetWideStringLength(const wchar_t *str) {
			uint32_t result = 0;
			if(str != nullptr) {
				while(*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_common_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
			char *result = nullptr;
			if((source != nullptr && dest != nullptr) && ((sourceLen + 1) <= maxDestLen)) {
				result = dest;
				uint32_t index = 0;
				while(index++ < sourceLen) {
					*dest++ = *source++;
				}
				*dest = 0;
			} else {
				// @TODO(final): Do we want to push a error here?
			}
			return(result);
		}

		fpl_common_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen) {
			char *result = nullptr;
			if(source != nullptr) {
				uint32_t sourceLen = GetAnsiStringLength(source);
				char *result = CopyAnsiString(source, sourceLen, dest, maxDestLen);
			} else {
				// @TODO(final): Do we want to push a error here?
			}
			return(result);
		}

		fpl_common_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
			wchar_t *result = nullptr;
			if((source != nullptr && dest != nullptr) && ((sourceLen + 1) <= maxDestLen)) {
				result = dest;
				uint32_t index = 0;
				while(index++ < sourceLen) {
					*dest++ = *source++;
				}
				*dest = 0;
			} else {
				// @TODO(final): Do we want to push a error here?
			}
			return(result);
		}

		fpl_common_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen) {
			wchar_t *result = nullptr;
			if(source != nullptr) {
				uint32_t sourceLen = GetWideStringLength(source);
				result = CopyWideString(source, sourceLen, dest, maxDestLen);
			} else {
				// @TODO(final): Do we want to push a error here?
			}
			return(result);
		}
	} // strings

	//
	// Common Memory
	//
	namespace memory {
		fpl_common_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment) {
			if(size == 0) {
				common::PushError("Memory size parameter must be greater than zero");
				return nullptr;
			}
			if((alignment == 0) || (alignment & (alignment - 1))) {
				common::PushError("Alignment '%zu' must be greater than zero and a power of two", alignment);
				return nullptr;
			}

			// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
			size_t newSize = sizeof(void *) + size + (alignment << 1);
			void *basePtr = fpl::memory::MemoryAllocate(newSize);

			// The resulting address starts after the stored base pointer
			void *alignedPtr = (void *)((uint8_t *)basePtr + sizeof(void *));

			// Move the resulting address to a aligned one when not aligned
			uintptr_t mask = alignment - 1;
			if((alignment > 1) && (((uintptr_t)alignedPtr & mask) != 0)) {
				uintptr_t offset = ((uintptr_t)alignment - ((uintptr_t)alignedPtr & mask));
				alignedPtr = (uint8_t *)alignedPtr + offset;
			}

			// Write the base pointer before the alignment pointer
			*(void **)((void *)((uint8_t *)alignedPtr - sizeof(void *))) = basePtr;

			// Ensure alignment
			FPL_ASSERT(FPL_IS_ALIGNED(alignedPtr, alignment));

			return(alignedPtr);
		}

		fpl_common_api void MemoryAlignedFree(void *ptr) {
			if(ptr == nullptr) {
				common::PushError("Memory pointer parameter are not allowed to be null");
				return;
			}

			// Free the base pointer which is stored to the left from the given pointer
			void *basePtr = *(void **)((void *)((uint8_t *)ptr - sizeof(void *)));
			FPL_ASSERT(basePtr != nullptr);
			MemoryFree(basePtr);
		}

		fpl_constant size_t MEM_SHIFT_64 = 3;
		fpl_constant size_t MEM_MASK_64 = 0x00000007;
		fpl_constant size_t MEM_SHIFT_32 = 2;
		fpl_constant size_t MEM_MASK_32 = 0x00000003;
		fpl_constant size_t MEM_SHIFT_16 = 1;
		fpl_constant size_t MEM_MASK_16 = 0x00000001;

		fpl_common_api void MemoryClear(void *mem, const size_t size) {
			if(mem == nullptr) {
				common::PushError("Memory parameter are not allowed to be null");
				return;
			}
			if(size == 0) {
				common::PushError("Size parameter must be greater than zero");
				return;
			}
			if(size % 8 == 0) {
				FPL_CLEARMEM(uint64_t, mem, size, MEM_SHIFT_64, MEM_MASK_64);
			} else if(size % 4 == 0) {
				FPL_CLEARMEM(uint32_t, mem, size, MEM_SHIFT_32, MEM_MASK_32);
			} else if(size % 2 == 0) {
				FPL_CLEARMEM(uint16_t, mem, size, MEM_SHIFT_16, MEM_MASK_16);
			} else {
				FPL_CLEARMEM(uint8_t, mem, size, 0, 0);
			}
		}

		fpl_common_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem) {
			if(sourceMem == nullptr) {
				common::PushError("Source memory parameter are not allowed to be null");
				return;
			}
			if(sourceSize == 0) {
				common::PushError("Source size parameter must be greater than zero");
				return;
			}
			if(targetMem == nullptr) {
				common::PushError("Target memory parameter are not allowed to be null");
				return;
			}
			if(sourceSize % 8 == 0) {
				FPL_COPYMEM(uint64_t, sourceMem, sourceSize, targetMem, MEM_SHIFT_64, MEM_MASK_64);
			} else if(sourceSize % 4 == 0) {
				FPL_COPYMEM(uint32_t, sourceMem, sourceSize, targetMem, MEM_SHIFT_32, MEM_MASK_32);
			} else if(sourceSize % 2 == 0) {
				FPL_COPYMEM(uint16_t, sourceMem, sourceSize, targetMem, MEM_SHIFT_32, MEM_MASK_32);
			} else {
				FPL_COPYMEM(uint8_t, sourceMem, sourceSize, targetMem, 0, 0);
			}
		}
	} // memory

	//
	// Common Atomics
	//
	namespace atomics {
		fpl_common_api void *AtomicExchangePtr(volatile void **target, const void *value) {
			FPL_ASSERT(target != nullptr);
#		if defined(FPL_ARCH_X64)
			void *result = (void *)AtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#		elif defined(FPL_ARCH_X86)
			void *result = (void *)AtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#		else
#			error "Unsupported architecture/platform!"
#		endif
			return (result);
		}

		fpl_common_api void *AtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
			FPL_ASSERT(dest != nullptr);
#		if defined(FPL_ARCH_X64)
			void *result = (void *)AtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#		elif defined(FPL_ARCH_X86)
			void *result = (void *)AtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#		else
#			error "Unsupported architecture/platform!"
#		endif
			return (result);
		}

		fpl_common_api bool IsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
			FPL_ASSERT(dest != nullptr);
#		if defined(FPL_ARCH_X64)
			bool result = IsAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#		elif defined(FPL_ARCH_X86)
			bool result = IsAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#		else
#			error "Unsupported architecture/platform!"
#		endif // FPL_ARCH
			return (result);
		}

		fpl_common_api void *AtomicLoadPtr(volatile void **source) {
#		if defined(FPL_ARCH_X64)
			void *result = (void *)AtomicLoadU64((volatile uint64_t *)source);
#		elif defined(FPL_ARCH_X86)
			void *result = (void *)AtomicLoadU32((volatile uint32_t *)source);
#		else
#			error "Unsupported architecture/platform!"
#		endif
			return(result);
		}

		fpl_common_api void AtomicStorePtr(volatile void **dest, const void *value) {
#		if defined(FPL_ARCH_X64)
			AtomicStoreU64((volatile uint64_t *)dest, (uint64_t)value);
#		elif defined(FPL_ARCH_X86)
			AtomicStoreU32((volatile uint32_t *)dest, (uint32_t)value);
#		else
#			error "Unsupported architecture/platform!"
#		endif
		}
	} // atomics

	//
	// Common Paths
	//
	namespace paths {
		fpl_common_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen) {
			if(sourcePath == nullptr) {
				return nullptr;
			}
			uint32_t sourceLen = strings::GetAnsiStringLength(sourcePath);
			if(sourceLen == 0) {
				return nullptr;
			}
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (sourceLen + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, sourceLen + 1);
				return nullptr;
			}

			char *result = nullptr;
			if(sourcePath) {
				int copyLen = 0;
				char *chPtr = (char *)sourcePath;
				while(*chPtr) {
					if(*chPtr == platform::PATH_SEPARATOR) {
						copyLen = (int)(chPtr - sourcePath);
					}
					++chPtr;
				}
				if(copyLen) {
					result = strings::CopyAnsiString(sourcePath, copyLen, destPath, maxDestLen);
				}
			}
			return(result);
		}

		fpl_common_api char *ExtractFileExtension(const char *sourcePath) {
			char *result = (char *)nullptr;
			if(sourcePath != nullptr) {
				char *filename = ExtractFileName(sourcePath);
				if(filename) {
					char *chPtr = filename;
					char *firstSeparatorPtr = (char *)nullptr;
					while(*chPtr) {
						if(*chPtr == platform::FILE_EXT_SEPARATOR) {
							firstSeparatorPtr = chPtr;
							break;
						}
						++chPtr;
					}
					if(firstSeparatorPtr != nullptr) {
						result = firstSeparatorPtr;
					}
				}
			}
			return(result);
		}

		fpl_common_api char *ExtractFileName(const char *sourcePath) {
			char *result = (char *)nullptr;
			if(sourcePath) {
				result = (char *)sourcePath;
				char *chPtr = (char *)sourcePath;
				char *lastPtr = (char *)nullptr;
				while(*chPtr) {
					if(*chPtr == platform::PATH_SEPARATOR) {
						lastPtr = chPtr;
					}
					++chPtr;
				}
				if(lastPtr != nullptr) {
					result = lastPtr + 1;
				}
			}
			return(result);
		}

		fpl_common_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen) {
			if(filePath == nullptr) {
				return nullptr;
			}
			if(newFileExtension == nullptr) {
				return nullptr;
			}
			uint32_t pathLen = strings::GetAnsiStringLength(filePath);
			if(pathLen == 0) {
				return nullptr;
			}
			uint32_t extLen = strings::GetAnsiStringLength(newFileExtension);

			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (pathLen + extLen + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, pathLen + extLen + 1);
				return nullptr;
			}

			char *result = nullptr;
			if(filePath != nullptr) {
				// Find last path
				char *chPtr = (char *)filePath;
				char *lastPathSeparatorPtr = nullptr;
				while(*chPtr) {
					if(*chPtr == platform::PATH_SEPARATOR) {
						lastPathSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				// Find last ext separator
				if(lastPathSeparatorPtr != nullptr) {
					chPtr = lastPathSeparatorPtr + 1;
				} else {
					chPtr = (char *)filePath;
				}
				char *lastExtSeparatorPtr = nullptr;
				while(*chPtr) {
					if(*chPtr == platform::FILE_EXT_SEPARATOR) {
						lastExtSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				uint32_t copyLen;
				if(lastExtSeparatorPtr != nullptr) {
					copyLen = (uint32_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
				} else {
					copyLen = pathLen;
				}

				// Copy parts
				strings::CopyAnsiString(filePath, copyLen, destPath, maxDestLen);
				char *destExtPtr = destPath + copyLen;
				strings::CopyAnsiString(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
				result = destPath;
			}
			return(result);
		}

		fpl_common_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
			if(pathCount == 0) {
				return nullptr;
			}
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestPathLen == 0) {
				common::PushError("Max dest path len parameter must be greater than zero");
				return nullptr;
			}

			uint32_t curDestPosition = 0;
			char *currentDestPtr = destPath;
			va_list vargs;
			va_start(vargs, pathCount);
			for(uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
				char *path = va_arg(vargs, char *);
				uint32_t pathLen = strings::GetAnsiStringLength(path);
				bool requireSeparator = pathIndex < (pathCount - 1);
				uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
				FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
				strings::CopyAnsiString(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
				currentDestPtr += pathLen;
				if(requireSeparator) {
					*currentDestPtr++ = platform::PATH_SEPARATOR;
				}
				curDestPosition += requiredPathLen;
			}
			*currentDestPtr = 0;
			va_end(vargs);
			return destPath;
		}

	} // paths

#if defined(FPL_ENABLE_WINDOW)
	namespace window {
		fpl_common_api bool PollWindowEvent(Event &ev) {
			FPL_ASSERT(platform::global__EventQueue != nullptr);
			platform::EventQueue *eventQueue = platform::global__EventQueue;
			bool result = false;
			if(eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pollIndex, 1);
				ev = eventQueue->events[eventIndex];
				result = true;
			} else if(eventQueue->pushCount > 0) {
				atomics::AtomicExchangeU32(&eventQueue->pollIndex, 0);
				atomics::AtomicExchangeU32(&eventQueue->pushCount, 0);
			}
			return result;
		}

		fpl_common_api void ClearWindowEvents() {
			FPL_ASSERT(platform::global__EventQueue != nullptr);
			platform::EventQueue *eventQueue = platform::global__EventQueue;
			atomics::AtomicExchangeU32(&eventQueue->pollIndex, 0);
			atomics::AtomicExchangeU32(&eventQueue->pushCount, 0);
		}
	} // window
#endif // FPL_ENABLE_WINDOW


} // fpl

// ****************************************************************************
//
// WIN32 Platform
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)

#	if defined(FPL_ARCH_X86)
#		define FPL_MEMORY_BARRIER() \
			LONG barrier; \
			::_InterlockedOr(&barrier, 0);
#	elif defined(FPL_ARCH_X64)
		// @NOTE(final): No need for hardware memory fence on X64 because the hardware guarantees memory order always.
#		define FPL_MEMORY_BARRIER()
#	endif

	// @NOTE(final): Little macro to not write 5 lines of code all the time
#	define FPL_WIN32_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)::GetProcAddress(libHandle, name); \
	if (target == nullptr) { \
		fpl::common::PushError("Failed getting '%s' from library '%s'", name, libName); \
		return false; \
	}

namespace fpl {
	namespace platform {
#	if defined(FPL_ENABLE_WINDOW)
		struct Win32WindowStyle {
			DWORD style;
			DWORD exStyle;
		};

		fpl_constant DWORD Win32ResizeableWindowStyle = WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
		fpl_constant DWORD Win32ResizeableWindowExtendedStyle = WS_EX_LEFT;

		fpl_constant DWORD Win32NonResizableWindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
		fpl_constant DWORD Win32NonResizableWindowExtendedStyle = WS_EX_LEFT;

		fpl_constant DWORD Win32FullscreenWindowStyle = WS_POPUP | WS_VISIBLE;
		fpl_constant DWORD Win32FullscreenWindowExtendedStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

		fpl_internal bool Win32LeaveFullscreen() {
			// @TODO(final): The old window rect may be wrong when the display was changed (Turn off, Orientation, Grid Position, Screen res).
			FPL_ASSERT(global__AppState != nullptr);
			const PlatformAppState *platState = global__AppState;
			const Win32AppState &win32State = platState->win32;
			const Win32APIFunctions &wapi = win32State.winApi;
			const WindowSettings &settings = platState->currentSettings.window;
			const Win32WindowState &win32Window = platState->window.win32;
			const Win32LastWindowInfo &fullscreenInfo = win32Window.lastFullscreenInfo;

			HWND windowHandle = win32Window.windowHandle;

			FPL_ASSERT(fullscreenInfo.style > 0 && fullscreenInfo.exStyle > 0);
			win32_setWindowLong(windowHandle, GWL_STYLE, fullscreenInfo.style);
			win32_setWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo.exStyle);
			wapi.user.setWindowPlacement(windowHandle, &fullscreenInfo.placement);
			wapi.user.setWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

			if(fullscreenInfo.isMaximized) {
				win32_sendMessage(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
			}

			bool result;
			if(fullscreenInfo.wasResolutionChanged) {
				result = (wapi.user.changeDisplaySettingsA(nullptr, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
			} else {
				result = true;
			}

			return(result);
		}

		fpl_internal bool Win32EnterFullscreen(const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits) {
			FPL_ASSERT(global__AppState != nullptr);
			PlatformAppState *platState = global__AppState;
			Win32AppState &win32State = platState->win32;
			const Win32APIFunctions &wapi = win32State.winApi;
			const WindowSettings &settings = platState->currentSettings.window;
			Win32WindowState &win32Window = platState->window.win32;
			Win32LastWindowInfo &fullscreenInfo = win32Window.lastFullscreenInfo;

			HWND windowHandle = win32Window.windowHandle;
			HDC deviceContext = win32Window.deviceContext;

			FPL_ASSERT(fullscreenInfo.style > 0 && fullscreenInfo.exStyle > 0);
			win32_setWindowLong(windowHandle, GWL_STYLE, fullscreenInfo.style & ~(WS_CAPTION | WS_THICKFRAME));
			win32_setWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo.exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

			MONITORINFO monitor = {};
			monitor.cbSize = sizeof(monitor);
			win32_getMonitorInfo(wapi.user.monitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST), &monitor);

			bool result;
			if(fullscreenWidth > 0 && fullscreenHeight > 0) {
				DWORD useFullscreenWidth = fullscreenWidth;
				DWORD useFullscreenHeight = fullscreenHeight;

				DWORD useRefreshRate = refreshRate;
				if(!useRefreshRate) {
					useRefreshRate = wapi.gdi.getDeviceCaps(deviceContext, VREFRESH);
				}

				DWORD useColourBits = colorBits;
				if(!useColourBits) {
					useColourBits = wapi.gdi.getDeviceCaps(deviceContext, BITSPIXEL);
				}

				// @TODO(final): Is this correct to assume the fullscreen rect is at (0, 0, w - 1, h - 1)?
				RECT windowRect;
				windowRect.left = 0;
				windowRect.top = 0;
				windowRect.right = windowRect.left + (useFullscreenWidth - 1);
				windowRect.bottom = windowRect.left + (useFullscreenHeight - 1);

				WINDOWPLACEMENT placement = {};
				placement.length = sizeof(placement);
				placement.rcNormalPosition = windowRect;
				placement.showCmd = SW_SHOW;
				wapi.user.setWindowPlacement(windowHandle, &placement);
				wapi.user.setWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

				DEVMODEA fullscreenSettings = {};
				wapi.user.enumDisplaySettingsA(nullptr, 0, &fullscreenSettings);
				fullscreenSettings.dmPelsWidth = useFullscreenWidth;
				fullscreenSettings.dmPelsHeight = useFullscreenHeight;
				fullscreenSettings.dmBitsPerPel = useColourBits;
				fullscreenSettings.dmDisplayFrequency = useRefreshRate;
				fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
				result = (wapi.user.changeDisplaySettingsA(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
				fullscreenInfo.wasResolutionChanged = true;
			} else {
				RECT windowRect = monitor.rcMonitor;

				WINDOWPLACEMENT placement = {};
				placement.length = sizeof(placement);
				placement.rcNormalPosition = windowRect;
				placement.showCmd = SW_SHOW;
				wapi.user.setWindowPlacement(windowHandle, &placement);
				wapi.user.setWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

				result = true;
				fullscreenInfo.wasResolutionChanged = false;
			}

			return(result);
		}

		fpl_internal_inline float Win32XInputProcessStickValue(const SHORT value, const SHORT deadZoneThreshold) {
			float result = 0;
			if(value < -deadZoneThreshold) {
				result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
			} else if(value > deadZoneThreshold) {
				result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
			}
			return(result);
		}

		fpl_internal void Win32PollControllers(const Settings &settings, const Win32InitState &initState, Win32XInputState &xinputState) {
			using namespace window;
			if(xinputState.xinputApi.xInputGetState != nullptr) {
				//
				// Detect new controller (Only on a fixed frequency)
				//
				if(xinputState.lastDeviceSearchTime.QuadPart == 0) {
					::QueryPerformanceCounter(&xinputState.lastDeviceSearchTime);
				}
				LARGE_INTEGER currentDeviceSearchTime;
				::QueryPerformanceCounter(&currentDeviceSearchTime);
				uint64_t deviceSearchDifferenceTimeInMs = ((currentDeviceSearchTime.QuadPart - xinputState.lastDeviceSearchTime.QuadPart) / (initState.performanceFrequency.QuadPart / 1000));
				if((settings.input.controllerDetectionFrequency == 0) || (deviceSearchDifferenceTimeInMs > settings.input.controllerDetectionFrequency)) {
					xinputState.lastDeviceSearchTime = currentDeviceSearchTime;
					for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
						XINPUT_STATE controllerState = {};
						if(xinputState.xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
							if(!xinputState.isConnected[controllerIndex]) {
								// Connected
								xinputState.isConnected[controllerIndex] = true;
								Event ev = {};
								ev.type = EventType::Gamepad;
								ev.gamepad.type = GamepadEventType::Connected;
								ev.gamepad.deviceIndex = controllerIndex;
								PushEvent(ev);
							}
						} else {
							if(xinputState.isConnected[controllerIndex]) {
								// Disonnected
								xinputState.isConnected[controllerIndex] = false;
								Event ev = {};
								ev.type = EventType::Gamepad;
								ev.gamepad.type = GamepadEventType::Disconnected;
								ev.gamepad.deviceIndex = controllerIndex;
								PushEvent(ev);
							}
						}
					}
				}

				//
				// Update controller state when connected only
				//
				for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
					if(xinputState.isConnected[controllerIndex]) {
						XINPUT_STATE controllerState = {};
						if(xinputState.xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
							// State changed
							Event ev = {};
							ev.type = EventType::Gamepad;
							ev.gamepad.type = GamepadEventType::StateChanged;
							ev.gamepad.deviceIndex = controllerIndex;

							XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

							// Analog sticks
							ev.gamepad.state.leftStickX = Win32XInputProcessStickValue(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							ev.gamepad.state.leftStickY = Win32XInputProcessStickValue(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
							ev.gamepad.state.rightStickX = Win32XInputProcessStickValue(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
							ev.gamepad.state.rightStickY = Win32XInputProcessStickValue(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

							// Triggers
							ev.gamepad.state.leftTrigger = (float)pad->bLeftTrigger / 255.0f;
							ev.gamepad.state.rightTrigger = (float)pad->bRightTrigger / 255.0f;

							// Digital pad buttons
							if(pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
								ev.gamepad.state.dpadUp = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
								ev.gamepad.state.dpadDown = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
								ev.gamepad.state.dpadLeft = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
								ev.gamepad.state.dpadRight = { true };

							// Action buttons
							if(pad->wButtons & XINPUT_GAMEPAD_A)
								ev.gamepad.state.actionA = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_B)
								ev.gamepad.state.actionB = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_X)
								ev.gamepad.state.actionX = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_Y)
								ev.gamepad.state.actionY = { true };

							// Center buttons
							if(pad->wButtons & XINPUT_GAMEPAD_START)
								ev.gamepad.state.start = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_BACK)
								ev.gamepad.state.back = { true };

							// Shoulder buttons
							if(pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
								ev.gamepad.state.leftShoulder = { true };
							if(pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
								ev.gamepad.state.rightShoulder = { true };

							PushEvent(ev);
						}
					}
				}
			}
		}

		fpl_internal_inline void Win32PushMouseEvent(const window::MouseEventType &mouseEventType, const window::MouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
			using namespace window;
			Event newEvent = {};
			newEvent.type = EventType::Mouse;
			newEvent.mouse.type = mouseEventType;
			newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
			newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
			newEvent.mouse.mouseButton = mouseButton;
			if(mouseEventType == MouseEventType::Wheel) {
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
			}
			PushEvent(newEvent);
		}

		fpl_internal window::Key Win32MapVirtualKey(const uint64_t keyCode) {
			using namespace window;
			switch(keyCode) {
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

		fpl_internal_inline void Win32PushKeyboardEvent(const window::KeyboardEventType keyboardEventType, const uint64_t keyCode, const window::KeyboardModifierFlags modifiers, const bool isDown) {
			using namespace window;
			Event newEvent = {};
			newEvent.type = EventType::Keyboard;
			newEvent.keyboard.keyCode = keyCode;
			newEvent.keyboard.mappedKey = Win32MapVirtualKey(keyCode);
			newEvent.keyboard.type = keyboardEventType;
			newEvent.keyboard.modifiers = modifiers;
			PushEvent(newEvent);
		}

		fpl_internal_inline bool Win32IsKeyDown(const Win32APIFunctions &wapi, const uint64_t keyCode) {
			bool result = (wapi.user.getAsyncKeyState((int)keyCode) & 0x8000) > 0;
			return(result);
		}

		LRESULT CALLBACK Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			using namespace window;

			FPL_ASSERT(global__AppState != nullptr);
			PlatformAppState *appState = global__AppState;
			Win32AppState &win32State = appState->win32;
			Win32WindowState &win32Window = appState->window.win32;
			const Win32APIFunctions &wapi = win32State.winApi;

			if(!win32Window.windowHandle) {
				return win32_defWindowProc(hwnd, msg, wParam, lParam);
			}

			LRESULT result = 0;
			switch(msg) {
				case WM_DESTROY:
				case WM_CLOSE:
				{
					win32Window.isRunning = false;
				} break;

				case WM_SIZE:
				{
#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					if(appState->currentSettings.video.driver == VideoDriverType::Software) {
						if(appState->initSettings.video.isAutoSize) {
							uint32_t w = LOWORD(lParam);
							uint32_t h = HIWORD(lParam);
							video::ResizeVideoBackBuffer(w, h);
						}
					}
#				endif

					Event newEvent = {};
					newEvent.type = EventType::Window;
					newEvent.window.type = WindowEventType::Resized;
					newEvent.window.width = LOWORD(lParam);
					newEvent.window.height = HIWORD(lParam);
					platform::PushEvent(newEvent);
				} break;

				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
				{
					uint64_t keyCode = wParam;
					bool wasDown = ((int)(lParam & (1 << 30)) != 0);
					bool isDown = ((int)(lParam & (1 << 31)) == 0);

					bool altKeyWasDown = platform::Win32IsKeyDown(wapi, VK_MENU);
					bool shiftKeyWasDown = platform::Win32IsKeyDown(wapi, VK_LSHIFT);
					bool ctrlKeyWasDown = platform::Win32IsKeyDown(wapi, VK_LCONTROL);
					bool superKeyWasDown = platform::Win32IsKeyDown(wapi, VK_LMENU);

					KeyboardEventType keyEventType = isDown ? KeyboardEventType::KeyDown : KeyboardEventType::KeyUp;
					KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
					if(altKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Alt;
					}
					if(shiftKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Shift;
					}
					if(ctrlKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Ctrl;
					}
					if(superKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Super;
					}
					platform::Win32PushKeyboardEvent(keyEventType, keyCode, modifiers, isDown);

					if(wasDown != isDown) {
						if(isDown) {
							if(keyCode == VK_F4 && altKeyWasDown) {
								win32Window.isRunning = 0;
							}
						}
					}
				} break;

				case WM_CHAR:
				{
					// @TODO(final): Add unicode support (WM_UNICHAR)!
					if(wParam >= 0 && wParam < 256) {
						uint64_t keyCode = wParam;
						KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
						platform::Win32PushKeyboardEvent(KeyboardEventType::CharInput, keyCode, modifiers, 0);
					}
				} break;

				case WM_ACTIVATE:
				{
					Event newEvent = {};
					newEvent.type = EventType::Window;
					if(wParam == WA_INACTIVE) {
						newEvent.window.type = WindowEventType::LostFocus;
					} else {
						newEvent.window.type = WindowEventType::GotFocus;
					}
					platform::PushEvent(newEvent);
				} break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				{
					MouseEventType mouseEventType;
					if(msg == WM_LBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					platform::Win32PushMouseEvent(mouseEventType, MouseButtonType::Left, lParam, wParam);
				} break;
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				{
					MouseEventType mouseEventType;
					if(msg == WM_RBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					platform::Win32PushMouseEvent(mouseEventType, MouseButtonType::Right, lParam, wParam);
				} break;
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
				{
					MouseEventType mouseEventType;
					if(msg == WM_MBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					platform::Win32PushMouseEvent(mouseEventType, MouseButtonType::Middle, lParam, wParam);
				} break;
				case WM_MOUSEMOVE:
				{
					platform::Win32PushMouseEvent(MouseEventType::Move, MouseButtonType::None, lParam, wParam);
				} break;
				case WM_MOUSEWHEEL:
				{
					platform::Win32PushMouseEvent(MouseEventType::Wheel, MouseButtonType::None, lParam, wParam);
				} break;

				case WM_SETCURSOR:
				{
					// @TODO(final): This is not right to assume default cursor always, because the size cursor does not work this way!
					if(win32Window.isCursorActive) {
						HCURSOR cursor = wapi.user.getCursor();
						wapi.user.setCursor(cursor);
					} else {
						wapi.user.setCursor(nullptr);
						return 1;
					}
				} break;

				case WM_ERASEBKGND:
				{
					// Prevent erase background always
					return 1;
				} break;

				default:
					break;
			}
			result = win32_defWindowProc(hwnd, msg, wParam, lParam);
			return (result);
		}

		fpl_internal bool Win32InitWindow(const Settings &initSettings, WindowSettings &currentWindowSettings, PlatformAppState *platAppState, Win32AppState &appState, Win32WindowState &windowState, callback_PrepareWindowForVideoAfter *prepareVideoForWindowAfter) {
			const Win32APIFunctions &wapi = appState.winApi;
			const WindowSettings &initWindowSettings = initSettings.window;

			// Register window class
			win32_wndclassex windowClass = {};
			windowClass.cbSize = sizeof(win32_wndclassex);
			windowClass.hInstance = GetModuleHandleA(nullptr);
			windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			windowClass.cbSize = sizeof(windowClass);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.hCursor = win32_loadCursor(nullptr, IDC_ARROW);
			windowClass.hIcon = win32_loadIcon(nullptr, IDI_APPLICATION);
			windowClass.hIconSm = win32_loadIcon(nullptr, IDI_APPLICATION);
			windowClass.lpszClassName = FPL_WIN32_CLASSNAME;
			windowClass.lpfnWndProc = Win32MessageProc;
			windowClass.style |= CS_OWNDC;
			win32_copyString(windowClass.lpszClassName, win32_getStringLength(windowClass.lpszClassName), windowState.windowClass, FPL_ARRAYCOUNT(windowState.windowClass));
			if(win32_registerClassEx(&windowClass) == 0) {
				common::PushError("Failed Registering Window Class '%s'", windowState.windowClass);
				return false;
			}

			// Create window
			platform::win32_char windowTitleBuffer[1024];
			platform::win32_char *windowTitle = FPL_WIN32_UNNAMED_WINDOW;
			currentWindowSettings.isFullscreen = false;
			if(strings::GetAnsiStringLength(initWindowSettings.windowTitle) > 0) {
				win32_ansiToString(initWindowSettings.windowTitle, strings::GetAnsiStringLength(initWindowSettings.windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
				windowTitle = windowTitleBuffer;
				strings::CopyAnsiString(initWindowSettings.windowTitle, strings::GetAnsiStringLength(initWindowSettings.windowTitle), currentWindowSettings.windowTitle, FPL_ARRAYCOUNT(currentWindowSettings.windowTitle));
			}

			DWORD style;
			DWORD exStyle;
			if(initWindowSettings.isResizable) {
				style = Win32ResizeableWindowStyle;
				exStyle = Win32ResizeableWindowExtendedStyle;
				currentWindowSettings.isResizable = true;
			} else {
				style = Win32NonResizableWindowStyle;
				exStyle = Win32NonResizableWindowExtendedStyle;
				currentWindowSettings.isResizable = false;
			}

			int windowX = CW_USEDEFAULT;
			int windowY = CW_USEDEFAULT;
			int windowWidth;
			int windowHeight;
			if((initWindowSettings.windowWidth > 0) &&
				(initWindowSettings.windowHeight > 0)) {
				RECT windowRect;
				windowRect.left = 0;
				windowRect.top = 0;
				windowRect.right = initWindowSettings.windowWidth;
				windowRect.bottom = initWindowSettings.windowHeight;
				wapi.user.adjustWindowRect(&windowRect, style, false);
				windowWidth = windowRect.right - windowRect.left;
				windowHeight = windowRect.bottom - windowRect.top;
			} else {
				// @NOTE(final): Operating system decide how big the window should be.
				windowWidth = CW_USEDEFAULT;
				windowHeight = CW_USEDEFAULT;
			}


			// Create window
			windowState.windowHandle = win32_createWindowEx(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, windowClass.hInstance, nullptr);
			if(windowState.windowHandle == nullptr) {
				common::PushError("Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", windowState.windowClass, windowWidth, windowHeight, windowWidth, windowHeight);
				return false;
			}

			// Get actual window size and store results
			currentWindowSettings.windowWidth = windowWidth;
			currentWindowSettings.windowHeight = windowHeight;
			RECT clientRect;
			if(wapi.user.getClientRect(windowState.windowHandle, &clientRect)) {
				currentWindowSettings.windowWidth = clientRect.right - clientRect.left;
				currentWindowSettings.windowHeight = clientRect.bottom - clientRect.top;
			}

			// Get device context so we can swap the back and front buffer
			windowState.deviceContext = wapi.user.getDC(windowState.windowHandle);
			if(windowState.deviceContext == nullptr) {
				common::PushError("Failed aquiring device context from window '%d'", windowState.windowHandle);
				return false;
			}

			// Prepare video if needed
			if((prepareVideoForWindowAfter != nullptr) && (platAppState->initFlags & InitFlags::Video)) {
				const VideoSettings &initVideoSettings = initSettings.video;
				prepareVideoForWindowAfter(initVideoSettings, platAppState);
			}

			// Enter fullscreen if needed
			if(initWindowSettings.isFullscreen) {
				window::SetWindowFullscreen(true, initWindowSettings.fullscreenWidth, initWindowSettings.fullscreenHeight);
			}

			// Show window
			wapi.user.showWindow(windowState.windowHandle, SW_SHOW);
			wapi.user.updateWindow(windowState.windowHandle);

			// Cursor is visible at start
			windowState.defaultCursor = windowClass.hCursor;
			windowState.isCursorActive = true;
			windowState.isRunning = true;

			return true;
		}

		fpl_internal void Win32ReleaseWindow(const Win32InitState &initState, const Win32AppState &appState, Win32WindowState &windowState) {
			const Win32APIFunctions &wapi = appState.winApi;

			if(windowState.deviceContext != nullptr) {
				wapi.user.releaseDC(windowState.windowHandle, windowState.deviceContext);
				windowState.deviceContext = nullptr;
			}

			if(windowState.windowHandle != nullptr) {
				wapi.user.destroyWindow(windowState.windowHandle);
				windowState.windowHandle = nullptr;
				win32_unregisterClass(windowState.windowClass, initState.appInstance);
			}
		}

		struct Win32CommandLineUTF8Arguments {
			void *mem;
			char **args;
			uint32_t count;
		};
		fpl_internal Win32CommandLineUTF8Arguments Win32ParseWideArguments(LPWSTR cmdLine) {
			Win32CommandLineUTF8Arguments args = {};

			// @NOTE(final): Temporary load and unload shell32 for parsing the arguments
			HMODULE shellapiLibrary = ::LoadLibraryA("shell32.dll");
			if(shellapiLibrary != nullptr) {
				win32_func_CommandLineToArgvW *commandLineToArgvW = (win32_func_CommandLineToArgvW *)::GetProcAddress(shellapiLibrary, "CommandLineToArgvW");
				if(commandLineToArgvW != nullptr) {
					// Parse arguments and compute total UTF8 string length
					int executableFilePathArgumentCount = 0;
					wchar_t **executableFilePathArgs = commandLineToArgvW(L"", &executableFilePathArgumentCount);
					uint32_t executableFilePathLen = 0;
					for(int i = 0; i < executableFilePathArgumentCount; ++i) {
						if(i > 0) {
							// Include whitespace
							executableFilePathLen++;
						}
						uint32_t sourceLen = strings::GetWideStringLength(executableFilePathArgs[i]);
						uint32_t destLen = ::WideCharToMultiByte(CP_UTF8, 0, executableFilePathArgs[i], sourceLen, nullptr, 0, 0, 0);
						executableFilePathLen += destLen;
					}

					// @NOTE(final): Do not parse the arguments when there are no actual arguments, otherwise we will get back the executable arguments again.
					int actualArgumentCount = 0;
					wchar_t **actualArgs = nullptr;
					uint32_t actualArgumentsLen = 0;
					if(cmdLine != nullptr && strings::GetWideStringLength(cmdLine) > 0) {
						actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
						for(int i = 0; i < actualArgumentCount; ++i) {
							uint32_t sourceLen = strings::GetWideStringLength(actualArgs[i]);
							uint32_t destLen = ::WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], sourceLen, nullptr, 0, 0, 0);
							actualArgumentsLen += destLen;
						}
					}

					// Calculate argument 
					args.count = 1 + actualArgumentCount;
					uint32_t totalStringLen = executableFilePathLen + actualArgumentsLen + args.count;
					size_t singleArgStringSize = sizeof(char) * (totalStringLen);
					size_t arbitaryPadding = sizeof(char) * 8;
					size_t argArraySize = sizeof(char **) * args.count;
					size_t totalArgSize = singleArgStringSize + arbitaryPadding + argArraySize;

					args.mem = (uint8_t *)memory::MemoryAllocate(totalArgSize);
					char *argsString = (char *)args.mem;
					args.args = (char **)((uint8_t *)args.mem + singleArgStringSize + arbitaryPadding);

					// Convert executable path to UTF8
					char *destArg = argsString;
					{
						args.args[0] = argsString;
						for(int i = 0; i < executableFilePathArgumentCount; ++i) {
							if(i > 0) {
								*destArg++ = ' ';
							}
							wchar_t *sourceArg = executableFilePathArgs[i];
							uint32_t sourceArgLen = strings::GetWideStringLength(sourceArg);
							uint32_t destArgLen = ::WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, nullptr, 0, 0, 0);
							::WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
							destArg += destArgLen;
						}
						*destArg++ = 0;
						::LocalFree(executableFilePathArgs);
					}

					// Convert actual arguments to UTF8
					if(actualArgumentCount > 0) {
						FPL_ASSERT(actualArgs != nullptr);
						for(int i = 0; i < actualArgumentCount; ++i) {
							args.args[1 + i] = destArg;
							wchar_t *sourceArg = actualArgs[i];
							uint32_t sourceArgLen = strings::GetWideStringLength(sourceArg);
							uint32_t destArgLen = ::WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, nullptr, 0, 0, 0);
							::WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
							destArg += destArgLen;
							*destArg++ = 0;
						}
						LocalFree(actualArgs);
					}
				}
				::FreeLibrary(shellapiLibrary);
				shellapiLibrary = nullptr;
			}
			return(args);
		}

		fpl_internal Win32CommandLineUTF8Arguments Win32ParseAnsiArguments(LPSTR cmdLine) {
			Win32CommandLineUTF8Arguments result;
			if(cmdLine != nullptr) {
				uint32_t ansiSourceLen = strings::GetAnsiStringLength(cmdLine);
				uint32_t wideDestLen = ::MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, nullptr, 0);
				// @TODO(final): Can we use a stack allocation here?
				wchar_t *wideCmdLine = (wchar_t *)memory::MemoryAllocate(sizeof(wchar_t) * (wideDestLen + 1));
				::MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, wideCmdLine, wideDestLen);
				wideCmdLine[wideDestLen] = 0;
				result = Win32ParseWideArguments(wideCmdLine);
				memory::MemoryFree(wideCmdLine);
			} else {
				result = Win32ParseWideArguments(L"");
			}
			return(result);
		}

#	endif // FPL_ENABLE_WINDOW

		fpl_internal void Win32LoadXInput(Win32XInputState &xinputState) {
			// Windows 8
			HMODULE xinputLibrary = ::LoadLibraryA("xinput1_4.dll");
			if(!xinputLibrary) {
				// Windows 7
				xinputLibrary = ::LoadLibraryA("xinput1_3.dll");
			}
			if(!xinputLibrary) {
				// Windows Generic
				xinputLibrary = ::LoadLibraryA("xinput9_1_0.dll");
			}
			Win32XInputFunctions &inputFuncs = xinputState.xinputApi;
			if(xinputLibrary) {
				inputFuncs.xinputLibrary = xinputLibrary;
				inputFuncs.xInputGetState = (win32_func_XInputGetState *)::GetProcAddress(xinputLibrary, "XInputGetState");
				inputFuncs.xInputGetCapabilities = (win32_func_XInputGetCapabilities *)::GetProcAddress(xinputLibrary, "XInputGetCapabilities");
			}
			if(inputFuncs.xInputGetState == nullptr) {
				inputFuncs.xInputGetState = Win32XInputGetStateStub;
			}
			if(inputFuncs.xInputGetCapabilities == nullptr) {
				inputFuncs.xInputGetCapabilities = Win32XInputGetCapabilitiesStub;
			}
		}

		fpl_internal void Win32UnloadXInput(Win32XInputState &xinputState) {
			Win32XInputFunctions &inputFuncs = xinputState.xinputApi;
			if(inputFuncs.xinputLibrary) {
				::FreeLibrary(inputFuncs.xinputLibrary);
				inputFuncs.xinputLibrary = nullptr;
				inputFuncs.xInputGetState = Win32XInputGetStateStub;
			}
		}

		fpl_internal bool Win32LoadAPI(Win32APIFunctions &wapi) {
			// Shell32
			{
				const char *shellLibraryName = "shell32.dll";
				HMODULE library = wapi.shell.shellLibrary = ::LoadLibraryA(shellLibraryName);
				if(library == nullptr) {
					common::PushError("Failed loading win32 library '%s'", shellLibraryName);
					return false;
				}
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, shellLibraryName, wapi.shell.commandLineToArgvW, win32_func_CommandLineToArgvW, "CommandLineToArgvW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, shellLibraryName, wapi.shell.shGetFolderPathA, win32_func_SHGetFolderPathA, "SHGetFolderPathA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, shellLibraryName, wapi.shell.shGetFolderPathW, win32_func_SHGetFolderPathW, "SHGetFolderPathW");
			}

			// User32
			{
				const char *userLibraryName = "user32.dll";
				HMODULE library = wapi.user.userLibrary = ::LoadLibraryA(userLibraryName);
				if(library == nullptr) {
					common::PushError("Failed loading win32 library '%s'", userLibraryName);
					return false;
				}

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.registerClassExA, win32_func_RegisterClassExA, "RegisterClassExA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.registerClassExW, win32_func_RegisterClassExW, "RegisterClassExW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.unregisterClassA, win32_func_UnregisterClassA, "UnregisterClassA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.unregisterClassW, win32_func_UnregisterClassW, "UnregisterClassW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.showWindow, win32_func_ShowWindow, "ShowWindow");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.destroyWindow, win32_func_DestroyWindow, "DestroyWindow");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.updateWindow, win32_func_UpdateWindow, "UpdateWindow");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.translateMessage, win32_func_TranslateMessage, "TranslateMessage");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.dispatchMessageA, win32_func_DispatchMessageA, "DispatchMessageA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.dispatchMessageW, win32_func_DispatchMessageW, "DispatchMessageW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.peekMessageA, win32_func_PeekMessageA, "PeekMessageA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.peekMessageW, win32_func_PeekMessageW, "PeekMessageW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.defWindowProcA, win32_func_DefWindowProcA, "DefWindowProcA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.defWindowProcW, win32_func_DefWindowProcW, "DefWindowProcW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.createWindowExA, win32_func_CreateWindowExA, "CreateWindowExA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.createWindowExW, win32_func_CreateWindowExW, "CreateWindowExW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowPos, win32_func_SetWindowPos, "SetWindowPos");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowPlacement, win32_func_GetWindowPlacement, "GetWindowPlacement");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowPlacement, win32_func_SetWindowPlacement, "SetWindowPlacement");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getClientRect, win32_func_GetClientRect, "GetClientRect");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowRect, win32_func_GetWindowRect, "GetWindowRect");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.adjustWindowRect, win32_func_AdjustWindowRect, "AdjustWindowRect");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getAsyncKeyState, win32_func_GetAsyncKeyState, "GetAsyncKeyState");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.mapVirtualKeyA, win32_func_MapVirtualKeyA, "MapVirtualKeyA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.mapVirtualKeyW, win32_func_MapVirtualKeyW, "MapVirtualKeyW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setCursor, win32_func_SetCursor, "SetCursor");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getCursor, win32_func_GetCursor, "GetCursor");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.loadCursorA, win32_func_LoadCursorA, "LoadCursorA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.loadCursorW, win32_func_LoadCursorW, "LoadCursorW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.loadIconA, win32_func_LoadIconA, "LoadCursorA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.loadIconW, win32_func_LoadIconW, "LoadIconW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowTextA, win32_func_SetWindowTextA, "SetWindowTextA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowTextW, win32_func_SetWindowTextW, "SetWindowTextW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowLongA, win32_func_SetWindowLongA, "SetWindowLongA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowLongW, win32_func_SetWindowLongW, "SetWindowLongW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowLongA, win32_func_GetWindowLongA, "GetWindowLongA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowLongW, win32_func_GetWindowLongW, "GetWindowLongW");

#				if defined(FPL_ARCH_X64)
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowLongPtrA, win32_func_SetWindowLongPtrA, "SetWindowLongPtrA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setWindowLongPtrW, win32_func_SetWindowLongPtrW, "SetWindowLongPtrW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowLongPtrA, win32_func_GetWindowLongPtrA, "GetWindowLongPtrA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getWindowLongPtrW, win32_func_GetWindowLongPtrW, "GetWindowLongPtrW");
#				endif

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.releaseDC, win32_func_ReleaseDC, "ReleaseDC");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getDC, win32_func_GetDC, "GetDC");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.changeDisplaySettingsA, win32_func_ChangeDisplaySettingsA, "ChangeDisplaySettingsA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.changeDisplaySettingsW, win32_func_ChangeDisplaySettingsW, "ChangeDisplaySettingsW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.enumDisplaySettingsA, win32_func_EnumDisplaySettingsA, "EnumDisplaySettingsA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.enumDisplaySettingsW, win32_func_EnumDisplaySettingsW, "EnumDisplaySettingsW");

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.openClipboard, win32_func_OpenClipboard, "OpenClipboard");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.closeClipboard, win32_func_CloseClipboard, "CloseClipboard");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.emptyClipboard, win32_func_EmptyClipboard, "EmptyClipboard");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.setClipboardData, win32_func_SetClipboardData, "SetClipboardData");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getClipboardData, win32_func_GetClipboardData, "GetClipboardData");

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getDesktopWindow, win32_func_GetDesktopWindow, "GetDesktopWindow");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getForegroundWindow, win32_func_GetForegroundWindow, "GetForegroundWindow");

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.isZoomed, win32_func_IsZoomed, "IsZoomed");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.sendMessageA, win32_func_SendMessageA, "SendMessageA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.sendMessageW, win32_func_SendMessageW, "SendMessageW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getMonitorInfoA, win32_func_GetMonitorInfoA, "GetMonitorInfoA");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.getMonitorInfoW, win32_func_GetMonitorInfoW, "GetMonitorInfoW");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.monitorFromRect, win32_func_MonitorFromRect, "MonitorFromRect");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, userLibraryName, wapi.user.monitorFromWindow, win32_func_MonitorFromWindow, "MonitorFromWindow");
			}

			// GDI32
			{
				const char *gdiLibraryName = "gdi32.dll";
				HMODULE library = wapi.gdi.gdiLibrary = ::LoadLibraryA(gdiLibraryName);
				if(library == nullptr) {
					common::PushError("Failed loading win32 library '%s'", gdiLibraryName);
					return false;
				}

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.choosePixelFormat, win32_func_ChoosePixelFormat, "ChoosePixelFormat");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.setPixelFormat, win32_func_SetPixelFormat, "SetPixelFormat");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.describePixelFormat, win32_func_DescribePixelFormat, "DescribePixelFormat");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.stretchDIBits, win32_func_StretchDIBits, "StretchDIBits");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.deleteObject, win32_func_DeleteObject, "DeleteObject");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.swapBuffers, win32_func_SwapBuffers, "SwapBuffers");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, gdiLibraryName, wapi.gdi.getDeviceCaps, win32_func_GetDeviceCaps, "GetDeviceCaps");
			}

			// OLE32
			{
				const char *oleLibraryName = "ole32.dll";
				HMODULE library = wapi.ole.oleLibrary = ::LoadLibraryA(oleLibraryName);
				if(library == nullptr) {
					common::PushError("Failed loading win32 library '%s'", oleLibraryName);
					return false;
				}

				FPL_WIN32_GET_FUNCTION_ADDRESS(library, oleLibraryName, wapi.ole.coInitializeEx, win32_func_CoInitializeEx, "CoInitializeEx");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, oleLibraryName, wapi.ole.coUninitialize, win32_func_CoUninitialize, "CoUninitialize");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, oleLibraryName, wapi.ole.coCreateInstance, win32_func_CoCreateInstance, "CoCreateInstance");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, oleLibraryName, wapi.ole.coTaskMemFree, win32_func_CoTaskMemFree, "CoTaskMemFree");
				FPL_WIN32_GET_FUNCTION_ADDRESS(library, oleLibraryName, wapi.ole.propVariantClear, win32_func_PropVariantClear, "PropVariantClear");
			}


			return true;
		}

		fpl_internal void Win32UnloadAPI(Win32APIFunctions &wapi) {
			if(wapi.ole.oleLibrary != nullptr) {
				::FreeLibrary(wapi.ole.oleLibrary);
				wapi.ole = {};
			}
			if(wapi.gdi.gdiLibrary != nullptr) {
				::FreeLibrary(wapi.gdi.gdiLibrary);
				wapi.gdi = {};
			}
			if(wapi.user.userLibrary != nullptr) {
				::FreeLibrary(wapi.user.userLibrary);
				wapi.user = {};
			}
			if(wapi.shell.shellLibrary != nullptr) {
				::FreeLibrary(wapi.shell.shellLibrary);
				wapi.shell = {};
			}
		}

		fpl_internal bool Win32ThreadWaitForMultiple(threading::ThreadContext *contexts[], const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
			if(contexts == nullptr) {
				common::PushError("Contexts parameter are not allowed to be null");
				return false;
			}
			if(count > common::MAX_THREAD_COUNT) {
				common::PushError("Count parameter '%d' must be greater or equal than '%d'", count, common::MAX_THREAD_COUNT);
				return false;
			}
			HANDLE threadHandles[common::MAX_THREAD_COUNT];
			for(uint32_t index = 0; index < count; ++index) {
				threading::ThreadContext *context = contexts[index];
				if(context == nullptr) {
					common::PushError("Thread context for index '%d' are not allowed to be null", index);
					return false;
				}
				if(context->internalHandle.win32.handle == nullptr) {
					common::PushError("Thread handle for index '%d' are not allowed to be null", index);
					return false;
				}
				HANDLE handle = context->internalHandle.win32.handle;
				threadHandles[index] = handle;
			}
			DWORD code = ::WaitForMultipleObjects(count, threadHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
			bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
			return(result);
		}

		fpl_internal bool Win32SignalWaitForMultiple(threading::ThreadSignal *signals[], const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
			if(signals == nullptr) {
				common::PushError("Signals parameter are not allowed to be null");
				return false;
			}
			if(count > common::MAX_SIGNAL_COUNT) {
				common::PushError("Count parameter '%d' must be less or equal than '%d'", count, common::MAX_SIGNAL_COUNT);
				return false;
			}
			HANDLE signalHandles[common::MAX_SIGNAL_COUNT];
			for(uint32_t index = 0; index < count; ++index) {
				threading::ThreadSignal *availableSignal = signals[index];
				if(availableSignal == nullptr) {
					common::PushError("Signal for index '%d' are not allowed to be null", index);
					return false;
				}
				if(availableSignal->internalHandle.win32.eventHandle == nullptr) {
					common::PushError("Signal handle for index '%d' are not allowed to be null", index);
					return false;
				}
				HANDLE handle = availableSignal->internalHandle.win32.eventHandle;
				signalHandles[index] = handle;
			}
			DWORD code = ::WaitForMultipleObjects(count, signalHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
			bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
			return(result);
		}

		fpl_internal void Win32ReleasePlatform(PlatformInitState &initState, PlatformAppState *appState) {
			FPL_ASSERT(appState != nullptr);
			Win32AppState &win32AppState = appState->win32;
			Settings &currentSettings = appState->currentSettings;
			Win32InitState &win32InitState = initState.win32;

			Win32UnloadXInput(win32AppState.xinput);

			Win32UnloadAPI(win32AppState.winApi);
		}

		fpl_internal bool Win32InitPlatform(const InitFlags initFlags, const Settings &initSettings, PlatformInitState &initState, PlatformAppState *appState) {
			Win32InitState &win32InitState = initState.win32;
			win32InitState.appInstance = ::GetModuleHandleA(nullptr);

			FPL_ASSERT(appState != nullptr);
			Win32AppState &win32AppState = appState->win32;

			// @NOTE(final): Expect kernel32.lib to be linked always, so VirtualAlloc and LoadLibrary will always work.

			// Timing
			::QueryPerformanceFrequency(&win32InitState.performanceFrequency);

			// Get main thread infos
			HANDLE mainThreadHandle = ::GetCurrentThread();
			DWORD mainThreadHandleId = ::GetCurrentThreadId();
			threading::ThreadContext *context = &common::global__ThreadState.mainThread;
			*context = {};
			context->id = mainThreadHandleId;
			context->internalHandle.win32.handle = mainThreadHandle;
			context->currentState = threading::ThreadState::Running;

			// Load windows api library
			if(!Win32LoadAPI(win32AppState.winApi)) {
				// @NOTE(final): Assume that errors are pushed on already.
				Win32ReleasePlatform(initState, appState);
				return false;
			}

			// Load XInput
			Win32LoadXInput(win32AppState.xinput);

			return (true);
		}

	} // platform

	//
	// Win32 Atomics
	//
	namespace atomics {
		fpl_platform_api void AtomicReadFence() {
			FPL_MEMORY_BARRIER();
			::_ReadBarrier();
		}
		fpl_platform_api void AtomicWriteFence() {
			FPL_MEMORY_BARRIER();
			::_WriteBarrier();
		}
		fpl_platform_api void AtomicReadWriteFence() {
			FPL_MEMORY_BARRIER();
			::_ReadWriteBarrier();
		}

		fpl_platform_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			FPL_ASSERT(target != nullptr);
			uint32_t result = ::_InterlockedExchange((volatile unsigned long *)target, value);
			return (result);
		}
		fpl_platform_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value) {
			FPL_ASSERT(target != nullptr);
			int32_t result = ::_InterlockedExchange((volatile long *)target, value);
			return (result);
		}
		fpl_platform_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			FPL_ASSERT(target != nullptr);
			uint64_t result = ::_InterlockedExchange((volatile unsigned __int64 *)target, value);
			return (result);
		}
		fpl_platform_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value) {
			FPL_ASSERT(target != nullptr);
#		if defined(FPL_ARCH_X64)
			int64_t result = ::_InterlockedExchange64((volatile long long *)target, value);
#		else
			// @NOTE(final): Why does MSVC have no _InterlockedExchange64 on x86???
			int64_t result = ::_InterlockedCompareExchange64((volatile long long *)target, value, value);
#		endif
			return (result);
		}

		fpl_platform_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = ::_InterlockedExchangeAdd((volatile unsigned long *)value, addend);
			return (result);
		}
		fpl_platform_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
			FPL_ASSERT(value != nullptr);
			int32_t result = ::_InterlockedExchangeAdd((volatile long *)value, addend);
			return (result);
		}
		fpl_platform_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint64_t result = ::_InterlockedExchangeAdd((volatile unsigned __int64 *)value, addend);
			return (result);
		}
		fpl_platform_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
			FPL_ASSERT(value != nullptr);
#		if defined(FPL_ARCH_X64)
			int64_t result = ::_InterlockedExchangeAdd64((volatile long long *)value, addend);
#		else
			// @NOTE(final): Why does MSVC have no _InterlockedExchangeAdd64 on x86???
			int64_t oldValue = AtomicLoadS64(value);
			int64_t newValue = oldValue + addend;
			int64_t result = oldValue;
			AtomicStoreS64(value, newValue);
#		endif
			return (result);
		}

		fpl_platform_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = ::_InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
			return (result);
		}
		fpl_platform_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t result = ::_InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			return (result);
		}
		fpl_platform_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = ::_InterlockedCompareExchange((volatile unsigned __int64 *)dest, exchange, comparand);
			return (result);
		}
		fpl_platform_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t result = ::_InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			return (result);
		}

		fpl_platform_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t value = ::_InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t value = ::_InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t value = ::_InterlockedCompareExchange((volatile unsigned __int64 *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t value = ::_InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}

		fpl_platform_api uint32_t AtomicLoadU32(volatile uint32_t *source) {
			uint32_t result = ::_InterlockedCompareExchange((volatile unsigned long *)source, 0, 0);
			return(result);
		}
		fpl_platform_api uint64_t AtomicLoadU64(volatile uint64_t *source) {
			uint64_t result = ::_InterlockedCompareExchange((volatile unsigned __int64 *)source, 0, 0);
			return(result);
		}
		fpl_platform_api int32_t AtomicLoadS32(volatile int32_t *source) {
			int32_t result = ::_InterlockedCompareExchange((volatile long *)source, 0, 0);
			return(result);
		}
		fpl_platform_api int64_t AtomicLoadS64(volatile int64_t *source) {
			int64_t result = ::_InterlockedCompareExchange64((volatile __int64 *)source, 0, 0);
			return(result);
		}

		fpl_platform_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
			::_InterlockedExchange((volatile unsigned long *)dest, value);
		}
		fpl_platform_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
			::_InterlockedExchange((volatile unsigned __int64 *)dest, value);
		}
		fpl_platform_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value) {
			::_InterlockedExchange((volatile long *)dest, value);
		}
		fpl_platform_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value) {
#		if defined(FPL_ARCH_X64)
			::_InterlockedExchange64((volatile __int64 *)dest, value);
#		else
			// @NOTE(final): Why does MSVC have no _InterlockedExchange64 on x86???
			int64_t oldValue = AtomicLoadS64(dest);
			::_InterlockedCompareExchange64((volatile __int64 *)dest, value, oldValue);
#		endif
		}
	} // atomics

	//
	// Win32 Hardware
	//
	namespace hardware {
		fpl_platform_api uint32_t GetProcessorCoreCount() {
			SYSTEM_INFO sysInfo = {};
			::GetSystemInfo(&sysInfo);
			// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
			uint32_t result = sysInfo.dwNumberOfProcessors;
			return(result);
		}

		fpl_platform_api MemoryInfos GetSystemMemoryInfos() {
			MemoryInfos result = {};
			MEMORYSTATUSEX statex = {};
			statex.dwLength = sizeof(statex);
			ULONGLONG totalMemorySize;
			if(::GetPhysicallyInstalledSystemMemory(&totalMemorySize) && ::GlobalMemoryStatusEx(&statex)) {
				result.totalPhysicalSize = totalMemorySize * 1024ull;
				result.availablePhysicalSize = statex.ullTotalPhys;
				result.usedPhysicalSize = result.availablePhysicalSize - statex.ullAvailPhys;
				result.totalVirtualSize = statex.ullTotalVirtual;
				result.usedVirtualSize = result.totalVirtualSize - statex.ullAvailVirtual;
				result.totalPageSize = statex.ullTotalPageFile;
				result.usedPageSize = result.totalPageSize - statex.ullAvailPageFile;
			}
			return(result);
		}

		fpl_platform_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
			fpl_constant uint32_t CPU_BRAND_BUFFER_SIZE = 0x40;

			if(destBuffer == nullptr) {
				common::PushError("Dest buffer parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestBufferLen < (CPU_BRAND_BUFFER_SIZE + 1)) {
				common::PushError("Max dest buffer len parameter '%d' must be greater or equal than '%d'", maxDestBufferLen, CPU_BRAND_BUFFER_SIZE + 1);
				return nullptr;
			}

			// @TODO(final): __cpuid may not be available on other Win32 Compilers!

			int cpuInfo[4] = { -1 };
			char cpuBrandBuffer[CPU_BRAND_BUFFER_SIZE] = {};
			::__cpuid(cpuInfo, 0x80000000);
			uint32_t extendedIds = cpuInfo[0];

			// Get the information associated with each extended ID. Interpret CPU brand string.
			uint32_t max = FPL_MIN(extendedIds, 0x80000004);
			for(uint32_t i = 0x80000002; i <= max; ++i) {
				::__cpuid(cpuInfo, i);
				uint32_t offset = (i - 0x80000002) << 4;
				memory::MemoryCopy(cpuInfo, sizeof(cpuInfo), cpuBrandBuffer + offset);
			}

			// Copy result back to the dest buffer
			uint32_t sourceLen = strings::GetAnsiStringLength(cpuBrandBuffer);
			strings::CopyAnsiString(cpuBrandBuffer, sourceLen, destBuffer, maxDestBufferLen);

			return(destBuffer);
		}
	} // hardware

	//
	// Win32 Console
	//
	namespace console {
		fpl_platform_api void ConsoleOut(const char *text) {
			if(text != nullptr) {
				::fprintf(stdout, text);
			}
		}
		fpl_platform_api void ConsoleFormatOut(const char *format, ...) {
			if(format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				::vfprintf(stdout, format, vaList);
				va_end(vaList);
			}
		}
		fpl_platform_api void ConsoleError(const char *text) {
			if(text != nullptr) {
				::fprintf(stderr, "%s", text);
			}
		}
		fpl_platform_api void ConsoleFormatError(const char *format, ...) {
			if(format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				::vfprintf(stderr, format, vaList);
				va_end(vaList);
			}
		}
		fpl_platform_api const char ConsoleWaitForCharInput() {
			int c = ::getchar();
			const char result = (c >= 0 && c < 256) ? (char)c : 0;
			return(result);
		}
	}

	//
	// Win32 Threading
	//
	namespace threading {
		fpl_internal DWORD WINAPI Win32ThreadProc(void *data) {
			ThreadContext *context = (ThreadContext *)data;
			FPL_ASSERT(context != nullptr);
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Running);
			DWORD result = 0;
			if(context->runFunc != nullptr) {
				context->runFunc(*context, context->data);
			}
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
			return(result);
		}

		fpl_platform_api ThreadContext *ThreadCreate(run_thread_function *runFunc, void *data) {
			ThreadContext *result = nullptr;
			ThreadContext *context = common::GetFreeThreadContext();
			if(context != nullptr) {
				DWORD creationFlags = 0;
				DWORD threadId = 0;
				context->data = data;
				context->runFunc = runFunc;
				context->currentState = ThreadState::Starting;
				HANDLE handle = ::CreateThread(nullptr, 0, Win32ThreadProc, context, creationFlags, &threadId);
				if(handle != nullptr) {
					context->isValid = true;
					context->id = threadId;
					context->internalHandle.win32.handle = handle;
					result = context;
				} else {
					common::PushError("Failed creating thread, error code: %d", GetLastError());
				}
			} else {
				common::PushError("All %d threads are in use, you cannot create until you free one", common::MAX_THREAD_COUNT);
			}
			return(result);
		}

		fpl_platform_api void ThreadSleep(const uint32_t milliseconds) {
			::Sleep((DWORD)milliseconds);
		}

		fpl_platform_api void ThreadDestroy(ThreadContext *context) {
			if(context == nullptr) {
				common::PushError("Context parameter are not allowed to be null");
				return;
			}
			if(context->internalHandle.win32.handle == nullptr) {
				common::PushError("Thread context handle are not allowed to be null");
				return;
			}
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopping);
			HANDLE handle = context->internalHandle.win32.handle;
			::TerminateThread(handle, 0);
			::CloseHandle(handle);
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
			*context = {};
		}

		fpl_platform_api bool ThreadWaitForOne(ThreadContext *context, const uint32_t maxMilliseconds) {
			if(context == nullptr) {
				common::PushError("Context parameter are not allowed to be null");
				return false;
			}
			if(context->internalHandle.win32.handle == nullptr) {
				common::PushError("Thread context handle are not allowed to be null");
				return false;
			}
			HANDLE handle = context->internalHandle.win32.handle;
			bool result = (::WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
			return(result);
		}

		fpl_platform_api bool ThreadWaitForAll(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32ThreadWaitForMultiple(contexts, count, true, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool ThreadWaitForAny(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32ThreadWaitForMultiple(contexts, count, false, maxMilliseconds);
			return(result);
		}

		fpl_platform_api ThreadMutex MutexCreate() {
			ThreadMutex result = {};
			::InitializeCriticalSection(&result.internalHandle.win32.criticalSection);
			result.isValid = true;
			return(result);
		}

		fpl_platform_api void MutexDestroy(ThreadMutex &mutex) {
			if(mutex.isValid) {
				::DeleteCriticalSection(&mutex.internalHandle.win32.criticalSection);
			}
			mutex = {};
		}

		fpl_platform_api bool MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds) {
			if(!mutex.isValid) {
				common::PushError("Mutex parameter must be valid");
				return false;
			}
			::EnterCriticalSection(&mutex.internalHandle.win32.criticalSection);
			return true;
		}

		fpl_platform_api bool MutexUnlock(ThreadMutex &mutex) {
			if(!mutex.isValid) {
				common::PushError("Mutex parameter must be valid");
				return false;
			}
			::LeaveCriticalSection(&mutex.internalHandle.win32.criticalSection);
			return true;
		}

		fpl_platform_api ThreadSignal SignalCreate() {
			ThreadSignal result = {};
			HANDLE handle = ::CreateEventA(nullptr, FALSE, FALSE, nullptr);
			if(handle != nullptr) {
				result.isValid = true;
				result.internalHandle.win32.eventHandle = handle;
			} else {
				common::PushError("Failed creating signal (Win32 event): %d", GetLastError());
			}
			return(result);
		}

		fpl_platform_api void SignalDestroy(ThreadSignal &signal) {
			if(signal.internalHandle.win32.eventHandle == nullptr) {
				common::PushError("Signal handle are not allowed to be null");
				return;
			}
			HANDLE handle = signal.internalHandle.win32.eventHandle;
			::CloseHandle(handle);
			signal = {};
		}

		fpl_platform_api bool SignalWaitForOne(ThreadMutex &mutex, ThreadSignal &signal, const uint32_t maxMilliseconds) {
			if(signal.internalHandle.win32.eventHandle == nullptr) {
				common::PushError("Signal handle are not allowed to be null");
				return false;
			}
			HANDLE handle = signal.internalHandle.win32.eventHandle;
			bool result = (::WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
			return(result);
		}

		fpl_platform_api bool SignalWaitForAll(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32SignalWaitForMultiple((ThreadSignal **)signals, count, true, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool SignalWaitForAny(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32SignalWaitForMultiple((ThreadSignal **)signals, count, false, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool SignalSet(ThreadSignal &signal) {
			if(signal.internalHandle.win32.eventHandle == nullptr) {
				common::PushError("Signal handle are not allowed to be null");
				return false;
			}
			HANDLE handle = signal.internalHandle.win32.eventHandle;
			bool result = ::SetEvent(handle) == TRUE;
			return(result);
		}

	} // threading

	//
	// Win32 Memory
	//
	namespace memory {
		fpl_platform_api void *MemoryAllocate(const size_t size) {
			if(size == 0) {
				common::PushError("Size parameter must be greater than zero");
				return nullptr;
			}
			void *result = ::VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(result == nullptr) {
				common::PushError("Failed allocating memory of %xu bytes", size);
			}
			return(result);
		}

		fpl_platform_api void MemoryFree(void *ptr) {
			if(ptr == nullptr) {
				common::PushError("Pointer parameter are not allowed to be null");
				return;
			}
			::VirtualFree(ptr, 0, MEM_FREE);
		}

		fpl_platform_api void *MemoryStackAllocate(const size_t size) {
			if(size == 0) {
				common::PushError("Size parameter must be greater than zero");
				return nullptr;
			}
			void *result = _malloca(size);
			return(result);
		}
	} // memory

	//
	// Win32 Files
	//
	namespace files {
		fpl_platform_api FileHandle OpenBinaryFile(const char *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				HANDLE win32FileHandle = ::CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32.handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}
		fpl_platform_api FileHandle OpenBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				HANDLE win32FileHandle = ::CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32.handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}

		fpl_platform_api FileHandle CreateBinaryFile(const char *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				HANDLE win32FileHandle = ::CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32.handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}
		fpl_platform_api FileHandle CreateBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				HANDLE win32FileHandle = ::CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32.handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}

		fpl_platform_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
			if(sizeToRead == 0) {
				return 0;
			}
			if(targetBuffer == nullptr) {
				common::PushError("Target buffer parameter are now allowed to be null");
				return 0;
			}
			if(fileHandle.internalHandle.win32.handle == nullptr) {
				common::PushError("File handle is not opened for reading");
				return 0;
			}
			uint32_t result = 0;
			HANDLE win32FileHandle = (HANDLE)fileHandle.internalHandle.win32.handle;
			DWORD bytesRead = 0;
			if(::ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, nullptr) == TRUE) {
				result = bytesRead;
			}
			return(result);
		}

		fpl_platform_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
			if(sourceSize == 0) {
				common::PushError("Source size parameter must be greater than zero");
				return 0;
			}
			if(sourceBuffer == nullptr) {
				common::PushError("Source buffer parameter are now allowed to be null");
				return 0;
			}
			if(fileHandle.internalHandle.win32.handle == nullptr) {
				common::PushError("File handle is not opened for writing");
				return 0;
			}
			uint32_t result = 0;
			HANDLE win32FileHandle = (HANDLE)fileHandle.internalHandle.win32.handle;
			DWORD bytesWritten = 0;
			if(::WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, nullptr) == TRUE) {
				result = bytesWritten;
			}
			return(result);
		}

		fpl_platform_api void SetFilePosition32(const FileHandle &fileHandle, const int32_t position, const FilePositionMode mode) {
			if(fileHandle.internalHandle.win32.handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32.handle;
				DWORD moveMethod = FILE_BEGIN;
				if(mode == FilePositionMode::Current) {
					moveMethod = FILE_CURRENT;
				} else if(mode == FilePositionMode::End) {
					moveMethod = FILE_END;
				}
				::SetFilePointer(win32FileHandle, (LONG)position, nullptr, moveMethod);
			}
		}

		fpl_platform_api uint32_t GetFilePosition32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if(fileHandle.internalHandle.win32.handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32.handle;
				DWORD filePosition = ::SetFilePointer(win32FileHandle, 0L, nullptr, FILE_CURRENT);
				if(filePosition != INVALID_SET_FILE_POINTER) {
					result = filePosition;
				}
			}
			return(result);
		}

		fpl_platform_api void CloseFile(FileHandle &fileHandle) {
			if(fileHandle.internalHandle.win32.handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32.handle;
				::CloseHandle(win32FileHandle);
				fileHandle = {};
			}
		}

		fpl_platform_api uint32_t GetFileSize32(const char *filePath) {
			uint32_t result = 0;
			if(filePath != nullptr) {
				HANDLE win32FileHandle = ::CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(win32FileHandle != INVALID_HANDLE_VALUE) {
					DWORD fileSize = ::GetFileSize(win32FileHandle, nullptr);
					result = fileSize;
					::CloseHandle(win32FileHandle);
				}
			}
			return(result);
		}

		fpl_platform_api uint32_t GetFileSize32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if(fileHandle.internalHandle.win32.handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32.handle;
				DWORD fileSize = ::GetFileSize(win32FileHandle, nullptr);
				result = fileSize;
			}
			return(result);
		}

		fpl_platform_api bool FileExists(const char *filePath) {
			bool result = false;
			if(filePath != nullptr) {
				WIN32_FIND_DATAA findData;
				HANDLE searchHandle = ::FindFirstFileA(filePath, &findData);
				if(searchHandle != INVALID_HANDLE_VALUE) {
					result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
					::FindClose(searchHandle);
				}
			}
			return(result);
		}

		fpl_platform_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
			if(sourceFilePath == nullptr) {
				common::PushError("Source file path parameter are not allowed to be null");
				return false;
			}
			if(targetFilePath == nullptr) {
				common::PushError("Target file path parameter are not allowed to be null");
				return false;
			}
			bool result = (::CopyFileA(sourceFilePath, targetFilePath, !overwrite) == TRUE);
			return(result);
		}

		fpl_platform_api bool FileMove(const char *sourceFilePath, const char *targetFilePath) {
			if(sourceFilePath == nullptr) {
				common::PushError("Source file path parameter are not allowed to be null");
				return false;
			}
			if(targetFilePath == nullptr) {
				common::PushError("Target file path parameter are not allowed to be null");
				return false;
			}
			bool result = (::MoveFileA(sourceFilePath, targetFilePath) == TRUE);
			return(result);
		}

		fpl_platform_api bool FileDelete(const char *filePath) {
			if(filePath == nullptr) {
				common::PushError("File path parameter are not allowed to be null");
				return false;
			}
			bool result = (::DeleteFileA(filePath) == TRUE);
			return(result);
		}

		fpl_platform_api bool DirectoryExists(const char *path) {
			bool result = false;
			if(path != nullptr) {
				WIN32_FIND_DATAA findData;
				HANDLE searchHandle = ::FindFirstFileA(path, &findData);
				if(searchHandle != INVALID_HANDLE_VALUE) {
					result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
					::FindClose(searchHandle);
				}
			}
			return(result);
		}

		fpl_platform_api bool CreateDirectories(const char *path) {
			if(path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = ::CreateDirectoryA(path, nullptr) > 0;
			return(result);
		}
		fpl_platform_api bool RemoveEmptyDirectory(const char *path) {
			if(path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = ::RemoveDirectoryA(path) > 0;
			return(result);
		}
		fpl_internal_inline void Win32FillFileEntry(const WIN32_FIND_DATAA &findData, FileEntry &entry) {
			strings::CopyAnsiString(findData.cFileName, strings::GetAnsiStringLength(findData.cFileName), entry.path, FPL_ARRAYCOUNT(entry.path));

			entry.type = FileEntryType::Unknown;
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				entry.type = FileEntryType::Directory;
			} else if(
				(findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
				entry.type = FileEntryType::File;
			}

			entry.attributes = FileAttributeFlags::None;
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
				entry.attributes = FileAttributeFlags::Normal;
			} else {
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
					entry.attributes |= FileAttributeFlags::Hidden;
				}
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					entry.attributes |= FileAttributeFlags::ReadOnly;
				}
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
					entry.attributes |= FileAttributeFlags::Archive;
				}
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
					entry.attributes |= FileAttributeFlags::System;
				}
			}
		}
		fpl_platform_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry) {
			if(pathAndFilter == nullptr) {
				return false;
			}
			bool result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = ::FindFirstFileA(pathAndFilter, &findData);
			if(searchHandle != INVALID_HANDLE_VALUE) {
				firstEntry = {};
				firstEntry.internalHandle.win32.fileHandle = searchHandle;
				Win32FillFileEntry(findData, firstEntry);
				result = true;
			}
			return(result);
		}
		fpl_platform_api bool ListFilesNext(FileEntry &nextEntry) {
			bool result = false;
			if(nextEntry.internalHandle.win32.fileHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = nextEntry.internalHandle.win32.fileHandle;
				WIN32_FIND_DATAA findData;
				if(::FindNextFileA(searchHandle, &findData)) {
					Win32FillFileEntry(findData, nextEntry);
					result = true;
				}
			}
			return(result);
		}
		fpl_platform_api void ListFilesEnd(FileEntry &lastEntry) {
			if(lastEntry.internalHandle.win32.fileHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = lastEntry.internalHandle.win32.fileHandle;
				::FindClose(searchHandle);
				lastEntry = {};
			}
		}
	} // files

	//
	// Win32 Path/Directories
	//
	namespace paths {
#	if defined(UNICODE)
		fpl_platform_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			wchar_t modulePath[MAX_PATH];
			::GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
			strings::WideStringToAnsiString(modulePath, strings::GetWideStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
		}
#	else
		fpl_platform_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			char modulePath[MAX_PATH];
			::GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
			strings::CopyAnsiString(modulePath, strings::GetAnsiStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
		}
#	endif // UNICODE

#	if defined(UNICODE)
		fpl_platform_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			if(platform::global__AppState == nullptr) {
				common::PushError("Platform is not initialized");
				return nullptr;
			}
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
			wchar_t homePath[MAX_PATH];
			wapi.shell.shGetFolderPathW(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			strings::WideStringToAnsiString(homePath, strings::GetWideStringLength(homePath), destPath, maxDestLen);
			return(destPath);
		}
#	else
		fpl_platform_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			if(destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			if(platform::global__AppState == nullptr) {
				common::PushError("Platform is not initialized");
				return nullptr;
			}
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
			char homePath[MAX_PATH];
			wapi.shell.shGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			strings::CopyAnsiString(homePath, strings::GetAnsiStringLength(homePath), destPath, maxDestLen);
			return(destPath);
		}
#	endif // UNICODE
	} // paths

	//
	// Win32 Timings
	//
	namespace timings {
		fpl_platform_api double GetHighResolutionTimeInSeconds() {
			const platform::Win32InitState &initState = platform::global__InitState.win32;
			LARGE_INTEGER time;
			::QueryPerformanceCounter(&time);
			double result = time.QuadPart / (double)initState.performanceFrequency.QuadPart;
			return(result);
		}

		fpl_platform_api uint64_t GetTimeInMilliseconds() {
			uint64_t result = ::GetTickCount();
			return(result);
		}
	} // timings

	//
	// Win32 Strings
	//
	namespace strings {
		fpl_platform_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
			if(wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if(ansiDest == nullptr) {
				common::PushError("Ansi dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = ::WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			if(maxAnsiDestLen < (requiredLen + 1)) {
				common::PushError("Max ansi dest len parameter '%d' must be greater or equal than ''", maxAnsiDestLen, (requiredLen + 1));
				return nullptr;
			}
			::WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, nullptr, nullptr);
			ansiDest[requiredLen] = 0;
			return(ansiDest);
		}
		fpl_platform_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
			if(wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if(utf8Dest == nullptr) {
				common::PushError("UTF8 dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = ::WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			if(maxUtf8DestLen < (requiredLen + 1)) {
				common::PushError("Max utf8 dest len parameter '%d' must be greater or equal than ''", maxUtf8DestLen, (requiredLen + 1));
				return nullptr;
			}
			::WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, nullptr, nullptr);
			utf8Dest[requiredLen] = 0;
			return(utf8Dest);
		}
		fpl_platform_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if(ansiSource == nullptr) {
				common::PushError("Ansi source parameter are not allowed to be null");
				return nullptr;
			}
			if(wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = ::MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, nullptr, 0);
			if(maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than '%d'", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			::MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_platform_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if(utf8Source == nullptr) {
				common::PushError("UTF8 source parameter are not allowed to be null");
				return nullptr;
			}
			if(wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = ::MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, nullptr, 0);
			if(maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than '%d'", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			::MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_platform_api char *FormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...) {
			if(ansiDestBuffer == nullptr) {
				common::PushError("Ansi dest buffer parameter are not allowed to be null");
				return nullptr;
			}
			if(maxAnsiDestBufferLen == 0) {
				common::PushError("Max ansi dest len parameter must be greater that zero");
				return nullptr;
			}
			if(format == nullptr) {
				common::PushError("Format parameter are not allowed to be null");
				return nullptr;
			}
			va_list argList;
			va_start(argList, format);
			// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
			ansiDestBuffer[0] = 0;
			int charCount = ::vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
			if(charCount < 0) {
				common::PushError("Format parameter are '%s' are invalid!", format);
				return nullptr;
			}
			if((int)maxAnsiDestBufferLen < (charCount + 1)) {
				common::PushError("Max ansi dest len parameter '%d' must be greater or equal than '%d'", maxAnsiDestBufferLen, (charCount + 1));
				return nullptr;
			}
			va_end(argList);
			FPL_ASSERT(charCount > 0);
			ansiDestBuffer[charCount] = 0;
			char *result = ansiDestBuffer;
			return(result);
		}
	} // strings

	//
	// Win32 Library
	//
	namespace library {
		fpl_platform_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
			DynamicLibraryHandle result = {};
			if(libraryFilePath != nullptr) {
				HMODULE libModule = ::LoadLibraryA(libraryFilePath);
				if(libModule != nullptr) {
					result.internalHandle.win32Handle = libModule;
					result.isValid = true;
				}
			}
			return(result);
		}
		fpl_platform_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
			void *result = nullptr;
			if(handle.internalHandle.win32Handle != nullptr && name != nullptr) {
				HMODULE libModule = handle.internalHandle.win32Handle;
				result = (void *)::GetProcAddress(libModule, name);
			}
			return(result);
		}
		fpl_platform_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
			if(handle.internalHandle.win32Handle != nullptr) {
				HMODULE libModule = (HMODULE)handle.internalHandle.win32Handle;
				::FreeLibrary(libModule);
				handle = {};
			}
		}
	} // library

#if defined(FPL_ENABLE_WINDOW)
	//
	// Win32 Window
	//
	namespace window {
		fpl_platform_api WindowSize GetWindowArea() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			WindowSize result = {};
			RECT windowRect;
			if(wapi.user.getClientRect(windowState.windowHandle, &windowRect)) {
				result.width = windowRect.right - windowRect.left;
				result.height = windowRect.bottom - windowRect.top;
			}
			return(result);
		}

		fpl_platform_api void SetWindowArea(const uint32_t width, const uint32_t height) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			RECT clientRect, windowRect;
			if(wapi.user.getClientRect(windowState.windowHandle, &clientRect) &&
			   wapi.user.getWindowRect(windowState.windowHandle, &windowRect)) {
				int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
				int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
				int newWidth = width + borderWidth;
				int newHeight = height + borderHeight;
				wapi.user.setWindowPos(windowState.windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

		fpl_platform_api bool IsWindowResizable() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			DWORD style = platform::win32_getWindowLong(windowState.windowHandle, GWL_STYLE);
			bool result = (style & WS_THICKFRAME) > 0;
			return(result);
		}

		fpl_platform_api void SetWindowResizeable(const bool value) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			const platform::Win32WindowState &windowState = appState->window.win32;
			const platform::Win32AppState &win32State = appState->win32;
			if(!appState->currentSettings.window.isFullscreen) {
				DWORD style;
				DWORD exStyle;
				if(value) {
					style = platform::Win32ResizeableWindowStyle;
					exStyle = platform::Win32ResizeableWindowExtendedStyle;
				} else {
					style = platform::Win32NonResizableWindowStyle;
					exStyle = platform::Win32NonResizableWindowExtendedStyle;
				}
				platform::win32_setWindowLong(windowState.windowHandle, GWL_STYLE, style);
				platform::win32_setWindowLong(windowState.windowHandle, GWL_EXSTYLE, exStyle);
				appState->currentSettings.window.isResizable = value;
			}
		}

		fpl_platform_api bool IsWindowFullscreen() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			HWND windowHandle = windowState.windowHandle;
			DWORD style = platform::win32_getWindowLong(windowHandle, GWL_STYLE);
			bool result = (style & platform::Win32FullscreenWindowStyle) > 0;
			return(result);
		}

		fpl_platform_api bool SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			platform::Win32AppState &win32AppState = appState->win32;
			platform::Win32WindowState &windowState = appState->window.win32;
			WindowSettings &windowSettings = appState->currentSettings.window;
			platform::Win32LastWindowInfo &fullscreenState = windowState.lastFullscreenInfo;
			const platform::Win32APIFunctions &wapi = win32AppState.winApi;

			HWND windowHandle = windowState.windowHandle;

			// Save current window info if not already fullscreen
			if(!windowSettings.isFullscreen) {
				fullscreenState.isMaximized = !!wapi.user.isZoomed(windowHandle);
				if(fullscreenState.isMaximized) {
					platform::win32_sendMessage(windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
				}
				fullscreenState.style = platform::win32_getWindowLong(windowHandle, GWL_STYLE);
				fullscreenState.exStyle = platform::win32_getWindowLong(windowHandle, GWL_EXSTYLE);
				wapi.user.getWindowPlacement(windowHandle, &fullscreenState.placement);
			}

			if(value) {
				// Enter fullscreen mode or fallback to window mode
				windowSettings.isFullscreen = platform::Win32EnterFullscreen(fullscreenWidth, fullscreenHeight, refreshRate, 0);
				if(!windowSettings.isFullscreen) {
					platform::Win32LeaveFullscreen();
				}
			} else {
				platform::Win32LeaveFullscreen();
				windowSettings.isFullscreen = false;
			}
			return(windowSettings.isFullscreen);
		}

		fpl_platform_api WindowPosition GetWindowPosition() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;

			WindowPosition result = {};
			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			if(wapi.user.getWindowPlacement(windowState.windowHandle, &placement) == TRUE) {
				switch(placement.showCmd) {
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

		fpl_platform_api void SetWindowTitle(const char *title) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;

			// @TODO(final): Add function for setting the unicode window title!
			HWND handle = windowState.windowHandle;
			wapi.user.setWindowTextA(handle, title);
		}

		fpl_platform_api void SetWindowPosition(const int32_t left, const int32_t top) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;

			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			RECT windowRect;
			if(wapi.user.getWindowPlacement(windowState.windowHandle, &placement) &&
			   wapi.user.getWindowRect(windowState.windowHandle, &windowRect)) {
				switch(placement.showCmd) {
					case SW_NORMAL:
					case SW_SHOW:
					{
						placement.rcNormalPosition.left = left;
						placement.rcNormalPosition.top = top;
						placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
						placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
						wapi.user.setWindowPlacement(windowState.windowHandle, &placement);
					} break;
				}
			}
		}

		fpl_platform_api void SetWindowCursorEnabled(const bool value) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			windowState.isCursorActive = value;
		}

		fpl_platform_api bool PushWindowEvent() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
			bool result = false;
			MSG msg;
			BOOL R = platform::win32_peekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
			if(R == TRUE) {
				wapi.user.translateMessage(&msg);
				platform::win32_dispatchMessage(&msg);
				result = true;
			}
			return (result);
		}

		fpl_platform_api void UpdateGameControllers() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			platform::Win32AppState &win32AppState = appState->win32;
			const platform::Win32InitState &win32InitState = platform::global__InitState.win32;
			platform::Win32PollControllers(appState->currentSettings, win32InitState, win32AppState.xinput);
		}

		fpl_platform_api bool WindowUpdate() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			platform::Win32AppState &win32AppState = appState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32InitState &win32InitState = platform::global__InitState.win32;
			const platform::Win32APIFunctions &wapi = win32AppState.winApi;

			bool result = false;

			// Poll gamepad controller states
			platform::Win32PollControllers(appState->currentSettings, win32InitState, win32AppState.xinput);

			// Poll window events
			if(windowState.windowHandle != 0) {
				MSG msg;
				while(platform::win32_peekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
					wapi.user.translateMessage(&msg);
					platform::win32_dispatchMessage(&msg);
				}
				result = windowState.isRunning;
			}

			return(result);
		}

		fpl_platform_api bool IsWindowRunning() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			bool result = windowState.isRunning;
			return(result);
		}

		fpl_platform_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			char *result = nullptr;
			if(wapi.user.openClipboard(windowState.windowHandle)) {
				if(wapi.user.isClipboardFormatAvailable(CF_TEXT)) {
					HGLOBAL dataHandle = wapi.user.getClipboardData(CF_TEXT);
					if(dataHandle != nullptr) {
						const char *stringValue = (const char *)GlobalLock(dataHandle);
						result = strings::CopyAnsiString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}

		fpl_platform_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			wchar_t *result = nullptr;
			if(wapi.user.openClipboard(windowState.windowHandle)) {
				if(wapi.user.isClipboardFormatAvailable(CF_UNICODETEXT)) {
					HGLOBAL dataHandle = wapi.user.getClipboardData(CF_UNICODETEXT);
					if(dataHandle != nullptr) {
						const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
						result = strings::CopyWideString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}

		fpl_platform_api bool SetClipboardText(const char *ansiSource) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			bool result = false;
			if(wapi.user.openClipboard(windowState.windowHandle)) {
				const uint32_t ansiLen = strings::GetAnsiStringLength(ansiSource);
				const uint32_t ansiBufferLen = ansiLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
				if(handle != nullptr) {
					char *target = (char*)GlobalLock(handle);
					strings::CopyAnsiString(ansiSource, ansiLen, target, ansiBufferLen);
					GlobalUnlock(handle);
					wapi.user.emptyClipboard();
					wapi.user.setClipboardData(CF_TEXT, handle);
					result = true;
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}

		fpl_platform_api bool SetClipboardText(const wchar_t *wideSource) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32AppState &appState = platform::global__AppState->win32;
			const platform::Win32WindowState &windowState = platform::global__AppState->window.win32;
			const platform::Win32APIFunctions &wapi = appState.winApi;
			bool result = false;
			if(wapi.user.openClipboard(windowState.windowHandle)) {
				const uint32_t wideLen = strings::GetWideStringLength(wideSource);
				const uint32_t wideBufferLen = wideLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
				if(handle != nullptr) {
					wchar_t *wideTarget = (wchar_t*)GlobalLock(handle);
					strings::CopyWideString(wideSource, wideLen, wideTarget, wideBufferLen);
					GlobalUnlock(handle);
					wapi.user.emptyClipboard();
					wapi.user.setClipboardData(CF_UNICODETEXT, handle);
					result = true;
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}
	} // window
#endif // FPL_ENABLE_WINDOW

} // fpl

#	if defined(FPL_ENABLE_WINDOW)

#		if defined(UNICODE)

int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl::platform::Win32CommandLineUTF8Arguments args = fpl::platform::Win32ParseWideArguments(cmdLine);
	int result = main(args.count, args.args);
	fpl::memory::MemoryFree(args.mem);
	return(result);
}

#		else

int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl::platform::Win32CommandLineUTF8Arguments args = fpl::platform::Win32ParseAnsiArguments(cmdLine);
	int result = main(args.count, args.args);
	fpl::memory::MemoryFree(args.mem);
	return(result);
}
#		endif // UNICODE

#	endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ****************************************************************************
//
// POSIX Sub-Platform (Linux, Unix)
//
// ****************************************************************************
#if defined(FPL_SUBPLATFORM_POSIX)
#include <sys/mman.h> // mmap, munmap
#include <sys/types.h> // data types
#include <sys/stat.h> // mkdir
#include <sys/errno.h> // errno
#include <signal.h> // pthread_kill
#include <stdlib.h> // wcstombs, mbstowcs
#include <time.h> // clock_gettime, nanosleep
#include <dlfcn.h> // dlopen, dlclose
#include <fcntl.h> // open
#include <unistd.h> // read, write, close, access, rmdir

	// @NOTE(final): Little macro to not write 5 lines of code all the time
#	define FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)dlsym(libHandle, name); \
	if (target == nullptr) { \
		fpl::common::PushError("Failed getting '%s' from library '%s'", name, libName); \
		return false; \
	}

namespace fpl {
	//
	// POSIX Platform
	//
	namespace platform {
		fpl_internal bool LoadPThreadAPI(PThreadAPI &pthreadAPI) {
			const char* libpthreadFileNames[] = {
				"libpthread.so",
				"libpthread.so.0",
			};
			pthreadAPI = {};
			const char *libName = nullptr;
			void *libHandle = nullptr;
			for(uint32_t index = 0; index < FPL_ARRAYCOUNT(libpthreadFileNames); ++index) {
				libName = libpthreadFileNames[index];
				libHandle = dlopen(libName, RTLD_NOW);
				if(libHandle != nullptr) {
					break;
				}
			}
			if(libHandle == nullptr) {
				return false;
			}

			// pthread_t
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_create, pthread_func_pthread_create, "pthread_create");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_kill, pthread_func_pthread_kill, "pthread_kill");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_join, pthread_func_pthread_join, "pthread_join");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_exit, pthread_func_pthread_exit, "pthread_exit");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_yield, pthread_func_pthread_yield, "pthread_yield");

			// pthread_mutex_t
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_mutex_init, pthread_func_pthread_mutex_init, "pthread_mutex_init");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_mutex_destroy, pthread_func_pthread_mutex_destroy, "pthread_mutex_destroy");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_mutex_lock, pthread_func_pthread_mutex_lock, "pthread_mutex_lock");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_mutex_trylock, pthread_func_pthread_mutex_trylock, "pthread_mutex_trylock");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_mutex_unlock, pthread_func_pthread_mutex_unlock, "pthread_mutex_unlock");

			// pthread_cond_t
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_init, pthread_func_pthread_cond_init, "pthread_cond_init");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_destroy, pthread_func_pthread_cond_destroy, "pthread_cond_destroy");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_timedwait, pthread_func_pthread_cond_timedwait, "pthread_cond_timedwait");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_wait, pthread_func_pthread_cond_wait, "pthread_cond_wait");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_broadcast, pthread_func_pthread_cond_broadcast, "pthread_cond_broadcast");
			FPL_DL_GET_FUNCTION_ADDRESS(libHandle, libName, pthreadAPI.pthread_cond_signal, pthread_func_pthread_cond_signal, "pthread_cond_signal");

			pthreadAPI.libraryHandle = libHandle;
			return true;
		}

		fpl_internal void UnloadPThreadAPI(PThreadAPI &pthreadAPI) {
			if(pthreadAPI.libraryHandle != nullptr) {
				dlclose(pthreadAPI.libraryHandle);
			}
			pthreadAPI = {};
		}

		fpl_internal void POSIXReleasePlatform(POSIXAppState &appState) {
			UnloadPThreadAPI(appState.pthreadApi);
		}

		fpl_internal bool POSIXInitPlatform(const InitFlags initFlags, const Settings &initSettings, POSIXInitState &initState, POSIXAppState &appState) {
			if(!LoadPThreadAPI(appState.pthreadApi)) {
				common::PushError("Failed initializing PThread API");
				return false;
			}
			return true;
		}
	}

	//
	// POSIX Atomics
	//
	namespace atomics {
#	if defined(FPL_COMPILER_GCC)
		// @NOTE(final): See: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#g_t_005f_005fsync-Builtins
		fpl_platform_api void AtomicReadFence() {
			// @TODO(final): Wrong to ensure a full memory fence here!
			__sync_synchronize();
		}
		fpl_platform_api void AtomicWriteFence() {
			// @TODO(final): Wrong to ensure a full memory fence here!
			__sync_synchronize();
		}
		fpl_platform_api void AtomicReadWriteFence() {
			__sync_synchronize();
		}

		fpl_platform_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			uint32_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_platform_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value) {
			int32_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_platform_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			uint64_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_platform_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value) {
			int64_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}

		fpl_platform_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_platform_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_platform_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_platform_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}

		fpl_platform_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}

		fpl_platform_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_platform_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}

		fpl_platform_api uint32_t AtomicLoadU32(volatile uint32_t *source) {
			uint32_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_platform_api uint64_t AtomicLoadU64(volatile uint64_t *source) {
			uint64_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_platform_api int32_t AtomicLoadS32(volatile int32_t *source) {
			int32_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_platform_api int64_t AtomicLoadS64(volatile int64_t *source) {
			int64_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}

		fpl_platform_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_platform_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_platform_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_platform_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
#	else
#		error "This POSIX compiler/platform is not supported!"
#	endif
	}

	//
	// POSIX Timings
	//
	namespace timings {
		fpl_platform_api double GetHighResolutionTimeInSeconds() {
			// @TODO(final): Do we need to take the performance frequency into account?
			timespec t;
			clock_gettime(CLOCK_MONOTONIC, &t);
			double result = (double)t.tv_sec + ((double)t.tv_nsec * 1e-9);
			return(result);
		}

		fpl_platform_api uint64_t GetTimeInMilliseconds() {
			timespec t;
			clock_gettime(CLOCK_MONOTONIC, &t);
			uint64_t result = t.tv_sec * 1000 + (uint64_t)(t.tv_nsec / 1.0e6);
			return(result);
		}
	}

	//
	// POSIX Threading
	//
	namespace threading {
		// @TODO(final): Move internal stuff into "platform" namespace!
		void *PosixThreadProc(void *data) {
			ThreadContext *context = (ThreadContext *)data;
			FPL_ASSERT(context != nullptr);
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Running);
			if(context->runFunc != nullptr) {
				context->runFunc(*context, context->data);
			}
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
			pthread_exit(nullptr);
		}

		fpl_internal bool PosixMutexLock(const platform::PThreadAPI &pthreadAPI, pthread_mutex_t *handle) {
			int lockRes;
			do {
				lockRes = pthreadAPI.pthread_mutex_lock(handle);
			} while(lockRes == EAGAIN);
			bool result = (lockRes == 0);
			return(result);
		}

		fpl_internal bool PosixMutexUnlock(const platform::PThreadAPI &pthreadAPI, pthread_mutex_t *handle) {
			int unlockRes;
			do {
				unlockRes = pthreadAPI.pthread_mutex_unlock(handle);
			} while(unlockRes == EAGAIN);
			bool result = (unlockRes == 0);
			return(result);
		}

		fpl_internal int PosixMutexCreate(const platform::PThreadAPI &pthreadAPI, pthread_mutex_t *handle) {
			*handle = PTHREAD_MUTEX_INITIALIZER;
			int mutexRes;
			do {
				mutexRes = pthreadAPI.pthread_mutex_init(handle, nullptr);
			} while(mutexRes == EAGAIN);
			return(mutexRes);
		}

		fpl_internal int PosixConditionCreate(const platform::PThreadAPI &pthreadAPI, pthread_cond_t *handle) {
			*handle = PTHREAD_COND_INITIALIZER;
			int condRes;
			do {
				condRes = pthreadAPI.pthread_cond_init(handle, nullptr);
			} while(condRes == EAGAIN);
			return(condRes);
		}

		fpl_internal_inline timespec CreateWaitTimeSpec(const uint32_t milliseconds) {
			time_t secs = milliseconds / 1000;
			uint64_t nanoSecs = (milliseconds - (secs * 1000)) * 1000000;
			if(nanoSecs >= 1000000000) {
				time_t addonSecs = (time_t)(nanoSecs / 1000000000);
				nanoSecs -= (addonSecs * 1000000000);
				secs += addonSecs;
			}
			timespec result;
			clock_gettime(CLOCK_REALTIME, &result);
			result.tv_sec += secs;
			result.tv_nsec += nanoSecs;
			return(result);
		}

		fpl_internal bool PosixThreadWaitForMultiple(ThreadContext *contexts[], const uint32_t minCount, const uint32_t maxCount, const uint32_t maxMilliseconds) {
			if(contexts == nullptr) {
				common::PushError("Contexts parameter are not allowed to be null");
				return false;
			}
			if(maxCount > common::MAX_THREAD_COUNT) {
				common::PushError("Count parameter '%d' must be less or equal than '%d'", maxCount, common::MAX_THREAD_COUNT);
				return false;
			}
			for(uint32_t index = 0; index < maxCount; ++index) {
				ThreadContext *context = contexts[index];
				if(context == nullptr) {
					common::PushError("Thread context for index '%d' are not allowed to be null", index);
					return false;
				}
				if(!context->isValid) {
					common::PushError("Thread for index '%d' is not valid", index);
					return false;
				}
			}

			volatile bool isRunning[fpl::common::MAX_THREAD_COUNT];
			for(uint32_t index = 0; index < maxCount; ++index) {
				isRunning[index] = true;
			}

			volatile uint32_t completeCount = 0;
			volatile uint64_t startTime = timings::GetTimeInMilliseconds();
			bool result = false;
			while(completeCount < minCount) {
				for(uint32_t index = 0; index < maxCount; ++index) {
					ThreadContext *context = contexts[index];
					if(isRunning[index]) {
						ThreadState state = GetThreadState(context);
						if(state == ThreadState::Stopped) {
							isRunning[index] = false;
							++completeCount;
							if(completeCount >= minCount) {
								result = true;
								break;
							}
						}
					}
					ThreadSleep(10);
				}
				if((maxMilliseconds != UINT32_MAX) && (timings::GetTimeInMilliseconds() - startTime) >= maxMilliseconds) {
					result = false;
					break;
				}
			}
			return(result);
		}

		fpl_internal bool PosixSignalWaitForMultiple(const platform::PThreadAPI &pthreadAPI, ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t minCount, const uint32_t maxCount, const uint32_t maxMilliseconds, const uint32_t smallWaitDuration = 5) {
			if(signals == nullptr) {
				common::PushError("Signals parameter are not allowed to be null");
				return false;
			}
			if(maxCount > common::MAX_SIGNAL_COUNT) {
				common::PushError("Count parameter '%d' must be less or equal than '%d'", maxCount, common::MAX_SIGNAL_COUNT);
				return false;
			}
			for(uint32_t index = 0; index < maxCount; ++index) {
				ThreadSignal *signal = signals[index];
				if(signal == nullptr) {
					common::PushError("Signal for index '%d' are not allowed to be null", index);
					return false;
				}
				if(!signal->isValid) {
					common::PushError("Signal for index '%d' is not valid", index);
					return false;
				}
			}

			volatile bool isSignaled[fpl::common::MAX_SIGNAL_COUNT];
			for(uint32_t index = 0; index < maxCount; ++index) {
				isSignaled[index] = false;
			}


			volatile uint32_t signaledCount = 0;
			volatile uint64_t startTime = timings::GetTimeInMilliseconds();
			bool result = false;
			while(signaledCount < minCount) {
				for(uint32_t index = 0; index < maxCount; ++index) {
					ThreadSignal *signal = signals[index];
					if(!isSignaled[index]) {
						timespec t = CreateWaitTimeSpec(smallWaitDuration);
						int condRes = pthreadAPI.pthread_cond_timedwait(&signal->internalHandle.posix.condition, &mutex.internalHandle.posix.mutex, &t);
						if(condRes == 0) {
							isSignaled[index] = true;
							++signaledCount;
							if(signaledCount >= minCount) {
								result = true;
								break;
							}
						}
					}
				}
				if((maxMilliseconds != UINT32_MAX) && (timings::GetTimeInMilliseconds() - startTime) >= maxMilliseconds) {
					result = false;
					break;
				}
			}
			return(result);
		}
		
		fpl_platform_api void ThreadDestroy(ThreadContext *context) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			FPL_ASSERT(pthreadAPI.libHandle != nullptr);
			if(context != nullptr && context->isValid) {
				pthread_t threadHandle = context->internalHandle.posix.thread;
				pthread_mutex_t *mutexHandle = &context->internalHandle.posix.mutex;
				pthread_cond_t *condHandle = &context->internalHandle.posix.stopCondition;

				// If thread is not stopped yet, kill it and wait for termination
				if(pthreadAPI.pthread_kill(threadHandle, 0) == 0) {
					pthreadAPI.pthread_join(threadHandle, nullptr);
				}
				pthreadAPI.pthread_cond_destroy(condHandle);
				pthreadAPI.pthread_mutex_destroy(mutexHandle);
				atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
				*context = {};
			}
		}

		fpl_platform_api ThreadContext *ThreadCreate(run_thread_function *runFunc, void *data) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			ThreadContext *result = nullptr;
			ThreadContext *context = common::GetFreeThreadContext();
			if(context != nullptr) {
				context->currentState = ThreadState::Stopped;
				context->data = data;
				context->runFunc = runFunc;
				context->isValid = false;
				context->isStopping = false;

				// Create mutex
				int mutexRes = PosixMutexCreate(pthreadAPI, &context->internalHandle.posix.mutex);
				if(mutexRes != 0) {
					common::PushError("Failed creating pthread mutex, error code: %d", mutexRes);
				}

				// Create stop condition
				int condRes = -1;
				if(mutexRes == 0) {
					condRes = PosixConditionCreate(pthreadAPI, &context->internalHandle.posix.stopCondition);
					if(condRes != 0) {
						common::PushError("Failed creating pthread condition, error code: %d", condRes);
						pthreadAPI.pthread_mutex_destroy(&context->internalHandle.posix.mutex);
					}
				}

				// Create thread
				int threadRes = -1;
				if(condRes == 0) {
					context->isValid = true;
					// @TODO(final): Better pthread id!
					memory::MemoryCopy(&context->internalHandle.posix.thread, FPL_MIN(sizeof(context->id), sizeof(context->internalHandle.posix.thread)), &context->id);
					do {
						threadRes = pthreadAPI.pthread_create(&context->internalHandle.posix.thread, nullptr, PosixThreadProc, (void *)context);
					} while(threadRes == EAGAIN);
					if(threadRes != 0) {
						common::PushError("Failed creating pthread, error code: %d", threadRes);
						pthreadAPI.pthread_cond_destroy(&context->internalHandle.posix.stopCondition);
						pthreadAPI.pthread_mutex_destroy(&context->internalHandle.posix.mutex);
					}
				}

				if(threadRes == 0) {
					result = context;
				} else {
					*context = {};
				}
			} else {
				common::PushError("All %d threads are in use, you cannot create until you free one", common::MAX_THREAD_COUNT);
			}
			return(result);
		}

		fpl_platform_api bool ThreadWaitForOne(ThreadContext *context, const uint32_t maxMilliseconds) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			FPL_ASSERT(pthreadAPI.libHandle != nullptr);
			bool result = false;
			if(context != nullptr && context->isValid) {
				// Set a flag and signal indicating that this thread is being stopped
				pthread_mutex_t *mutexHandle = &context->internalHandle.posix.mutex;
				pthread_cond_t *condHandle = &context->internalHandle.posix.stopCondition;
				if(PosixMutexLock(pthreadAPI, mutexHandle)) {
					context->isStopping = true;
					pthreadAPI.pthread_cond_signal(condHandle);
					pthreadAPI.pthread_cond_broadcast(condHandle);
					PosixMutexUnlock(pthreadAPI, mutexHandle);
				}

				// Wait until it shuts down
				pthread_t threadHandle = context->internalHandle.posix.thread;
				int joinRes = pthreadAPI.pthread_join(threadHandle, nullptr);
				result = (joinRes == 0);
			}
			return (result);
		}

		fpl_platform_api bool ThreadWaitForAll(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = PosixThreadWaitForMultiple(contexts, count, count, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool ThreadWaitForAny(ThreadContext *contexts[], const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = PosixThreadWaitForMultiple(contexts, 1, count, maxMilliseconds);
			return(result);
		}

		fpl_platform_api void ThreadSleep(const uint32_t milliseconds) {
			uint32_t ms;
			uint32_t s;
			if(milliseconds > 1000) {
				s = milliseconds / 1000;
				ms = milliseconds % 1000;
			} else {
				s = 0;
				ms = milliseconds;
			}
			timespec input, output;
			input.tv_sec = s;
			input.tv_nsec = ms * 1000000;
			nanosleep(&input, &output);
		}

		fpl_platform_api ThreadMutex MutexCreate() {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			ThreadMutex result = {};
			int mutexRes = PosixMutexCreate(pthreadAPI, &result.internalHandle.posix.mutex);
			result.isValid = (mutexRes == 0);
			return(result);
		}

		fpl_platform_api void MutexDestroy(ThreadMutex &mutex) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			if(mutex.isValid) {
				pthread_mutex_t *handle = &mutex.internalHandle.posix.mutex;
				pthreadAPI.pthread_mutex_destroy(handle);
			}
			mutex = {};
		}

		fpl_platform_api bool MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			bool result = false;
			if(mutex.isValid) {
				pthread_mutex_t *handle = &mutex.internalHandle.posix.mutex;
				result = PosixMutexLock(pthreadAPI, handle);
			}
			return (result);
		}

		fpl_platform_api bool MutexUnlock(ThreadMutex &mutex) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			bool result = false;
			if(mutex.isValid) {
				pthread_mutex_t *handle = &mutex.internalHandle.posix.mutex;
				result = PosixMutexUnlock(pthreadAPI, handle);
			}
			return (result);
		}

		fpl_platform_api ThreadSignal SignalCreate() {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			ThreadSignal result = {};
			int condRes = PosixConditionCreate(pthreadAPI, &result.internalHandle.posix.condition);
			result.isValid = (condRes == 0);
			return(result);
		}

		fpl_platform_api void SignalDestroy(ThreadSignal &signal) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			if(signal.isValid) {
				pthread_cond_t *handle = &signal.internalHandle.posix.condition;
				pthreadAPI.pthread_cond_destroy(handle);
			}
			signal = {};
		}

		

		fpl_platform_api bool SignalWaitForOne(ThreadMutex &mutex, ThreadSignal &signal, const uint32_t maxMilliseconds) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			if(!signal.isValid) {
				common::PushError("Signal is not valid");
				return(false);
			}
			if(!mutex.isValid) {
				common::PushError("Mutex is not valid");
				return(false);
			}
			if(maxMilliseconds != UINT32_MAX) {
				timespec t = CreateWaitTimeSpec(maxMilliseconds);
				pthreadAPI.pthread_cond_timedwait(&signal.internalHandle.posix.condition, &mutex.internalHandle.posix.mutex, &t);
			} else {
				pthreadAPI.pthread_cond_wait(&signal.internalHandle.posix.condition, &mutex.internalHandle.posix.mutex);
			}
			return(true);
		}

		fpl_platform_api bool SignalWaitForAll(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			bool result = PosixSignalWaitForMultiple(pthreadAPI, mutex, signals, count, count, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool SignalWaitForAny(ThreadMutex &mutex, ThreadSignal *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			bool result = PosixSignalWaitForMultiple(pthreadAPI, mutex, signals, 1, count, maxMilliseconds);
			return(result);
		}

		fpl_platform_api bool SignalSet(ThreadSignal &signal) {
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PThreadAPI &pthreadAPI = appState->linux.posix.pthreadApi;
			bool result = false;
			if(signal.isValid) {
				pthread_cond_t *handle = &signal.internalHandle.posix.condition;
				int condRes = pthreadAPI.pthread_cond_signal(handle);
				pthreadAPI.pthread_cond_broadcast(handle);
				result = (condRes == 0);
			}
			return(result);
		}
	}

	//
	// POSIX Library
	//
	namespace library {
		fpl_platform_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
			DynamicLibraryHandle result = {};
			if(libraryFilePath != nullptr) {
				// @TODO(final): Is RTLD_NOW for dlopen correct?
				void *p = dlopen(libraryFilePath, RTLD_NOW);
				if(p != nullptr) {
					result.internalHandle.posixHandle = p;
					result.isValid = true;
				}
			}
			return(result);
		}

		fpl_platform_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
			void *result = nullptr;
			if(handle.internalHandle.posixHandle != nullptr && name != nullptr) {
				void *p = handle.internalHandle.posixHandle;
				result = dlsym(p, name);
			}
			return(result);
		}

		fpl_platform_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
			if(handle.internalHandle.posixHandle != nullptr) {
				void *p = handle.internalHandle.posixHandle;
				dlclose(p);
				handle = {};
			}
		}
	}


	//
	// POSIX Console
	//
	namespace console {
		fpl_platform_api void ConsoleOut(const char *text) {
			if(text != nullptr) {
				fprintf(stdout, "%s", text);
			}
		}
		fpl_platform_api void ConsoleFormatOut(const char *format, ...) {
			if(format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stdout, format, vaList);
				va_end(vaList);
			}
		}
		fpl_platform_api void ConsoleError(const char *text) {
			if(text != nullptr) {
				fprintf(stderr, "%s", text);
			}
		}
		fpl_platform_api void ConsoleFormatError(const char *format, ...) {
			if(format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stderr, format, vaList);
				va_end(vaList);
			}
		}
		fpl_platform_api const char ConsoleWaitForCharInput() {
			int c = getchar();
			const char result = (c >= 0 && c < 256) ? (char)c : 0;
			return(result);
		}
	}

	//
	// POSIX Memory
	//
	namespace memory {
		fpl_platform_api void *MemoryAllocate(const size_t size) {
			// @NOTE(final): MAP_ANONYMOUS ensures that the memory is cleared to zero.

			// Allocate empty memory to hold the size + some arbitary padding + the actual data
			size_t newSize = sizeof(size_t) + sizeof(uintptr_t) + size;
			void *basePtr = mmap(nullptr, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

			// Write the size at the beginning
			*(size_t *)basePtr = newSize;

			// The resulting address starts after the arbitary padding
			void *result = (uint8_t *)basePtr + sizeof(size_t) + sizeof(uintptr_t);
			return(result);
		}

		fpl_platform_api void MemoryFree(void *ptr) {
			// Free the base pointer which is stored to the left at the start of the size_t
			void *basePtr = (void *)((uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(size_t)));
			size_t storedSize = *(size_t *)basePtr;
			munmap(basePtr, storedSize);
		}
	}

	//
	// POSIX Strings
	//
	namespace strings {
		fpl_platform_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
			if(wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if(ansiDest == nullptr) {
				common::PushError("Ansi dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = wcstombs(nullptr, wideSource, maxWideSourceLen);
			if(maxAnsiDestLen < (requiredLen + 1)) {
				common::PushError("Max ansi dest len parameter '%d' must be greater or equal than ''", maxAnsiDestLen, (requiredLen + 1));
				return nullptr;
			}
			wcstombs(ansiDest, wideSource, maxWideSourceLen);
			ansiDest[requiredLen] = 0;
			return(ansiDest);
		}
		fpl_platform_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
			if(wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if(utf8Dest == nullptr) {
				common::PushError("UTF8 dest parameter are not allowed to be null");
				return nullptr;
			}
			// @TODO(final): UTF-8!
			uint32_t requiredLen = wcstombs(nullptr, wideSource, maxWideSourceLen);
			if(maxUtf8DestLen < (requiredLen + 1)) {
				common::PushError("Max utf8 dest len parameter '%d' must be greater or equal than ''", maxUtf8DestLen, (requiredLen + 1));
				return nullptr;
			}
			wcstombs(utf8Dest, wideSource, maxWideSourceLen);
			utf8Dest[requiredLen] = 0;
			return(utf8Dest);
		}
		fpl_platform_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if(ansiSource == nullptr) {
				common::PushError("Ansi source parameter are not allowed to be null");
				return nullptr;
			}
			if(wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = mbstowcs(nullptr, ansiSource, ansiSourceLen);
			if(maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than '%d'", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			mbstowcs(wideDest, ansiSource, ansiSourceLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_platform_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if(utf8Source == nullptr) {
				common::PushError("UTF8 source parameter are not allowed to be null");
				return nullptr;
			}
			if(wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			// @TODO(final): UTF-8!
			uint32_t requiredLen = mbstowcs(nullptr, utf8Source, utf8SourceLen);
			if(maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than '%d'", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			mbstowcs(wideDest, utf8Source, utf8SourceLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_platform_api char *FormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...) {
			if(ansiDestBuffer == nullptr) {
				common::PushError("Ansi dest buffer parameter are not allowed to be null");
				return nullptr;
			}
			if(maxAnsiDestBufferLen == 0) {
				common::PushError("Max ansi dest len parameter must be greater that zero");
				return nullptr;
			}
			if(format == nullptr) {
				common::PushError("Format parameter are not allowed to be null");
				return nullptr;
			}
			va_list argList;
			va_start(argList, format);
			// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
			ansiDestBuffer[0] = 0;
			int charCount = vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
			if(charCount < 0) {
				common::PushError("Format parameter are '%s' are invalid!", format);
				return nullptr;
			}
			if((int)maxAnsiDestBufferLen < (charCount + 1)) {
				common::PushError("Max ansi dest len parameter '%d' must be greater or equal than '%d'", maxAnsiDestBufferLen, (charCount + 1));
				return nullptr;
			}
			va_end(argList);
			FPL_ASSERT(charCount > 0);
			ansiDestBuffer[charCount] = 0;
			char *result = ansiDestBuffer;
			return(result);
		}
	}

	//
	// POSIX Files
	//
	namespace files {
		fpl_platform_api FileHandle OpenBinaryFile(const char *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				int posixFileHandle;
				do {
					posixFileHandle = ::open(filePath, O_RDONLY);
				} while(posixFileHandle == -1 && errno == EINTR);
				if(posixFileHandle != -1) {
					result.isValid = true;
					result.internalHandle.posix.handle = posixFileHandle;
				}
			}
			return(result);
		}
		fpl_platform_api FileHandle OpenBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				char utf8FilePath[1024] = {};
				strings::WideStringToAnsiString(filePath, strings::GetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
				result = OpenBinaryFile(utf8FilePath);
			}
			return(result);
		}

		fpl_platform_api FileHandle CreateBinaryFile(const char *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				int posixFileHandle;
				do {
					posixFileHandle = ::open(filePath, O_WRONLY | O_CREAT | O_TRUNC);
				} while(posixFileHandle == -1 && errno == EINTR);
				if(posixFileHandle != -1) {
					result.isValid = true;
					result.internalHandle.posix.handle = posixFileHandle;
				}
			}
			return(result);
		}
		fpl_platform_api FileHandle CreateBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if(filePath != nullptr) {
				char utf8FilePath[1024] = {};
				strings::WideStringToAnsiString(filePath, strings::GetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
				result = CreateBinaryFile(utf8FilePath);
			}
			return(result);
		}

		fpl_platform_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
			if(sizeToRead == 0) {
				return 0;
			}
			if(targetBuffer == nullptr) {
				common::PushError("Target buffer parameter are now allowed to be null");
				return 0;
			}
			if(!fileHandle.internalHandle.posix.handle) {
				common::PushError("File handle is not opened for reading");
				return 0;
			}
			int posixFileHandle = fileHandle.internalHandle.posix.handle;

			ssize_t res;
			do {
				res = ::read(posixFileHandle, targetBuffer, sizeToRead);
			} while(res == -1 && errno == EINTR);

			uint32_t result = 0;
			if(res != -1) {
				result = (uint32_t)res;
			}
			return(result);
		}

		fpl_platform_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
			if(sourceSize == 0) {
				return 0;
			}
			if(sourceBuffer == nullptr) {
				common::PushError("Source buffer parameter are now allowed to be null");
				return 0;
			}
			if(!fileHandle.internalHandle.posix.handle) {
				common::PushError("File handle is not opened for writing");
				return 0;
			}

			int posixFileHandle = fileHandle.internalHandle.posix.handle;

			ssize_t res;
			do {
				res = ::write(posixFileHandle, sourceBuffer, sourceSize);
			} while(res == -1 && errno == EINTR);

			uint32_t result = 0;
			if(res != -1) {
				result = (uint32_t)res;
			}
			return(result);
		}

		fpl_platform_api void SetFilePosition32(const FileHandle &fileHandle, const int32_t position, const FilePositionMode mode) {
			if(fileHandle.internalHandle.posix.handle) {
				int posixFileHandle = fileHandle.internalHandle.posix.handle;
				int whence = SEEK_SET;
				if(mode == FilePositionMode::Current) {
					whence = SEEK_CUR;
				} else if(mode == FilePositionMode::End) {
					whence = SEEK_END;
				}
				::lseek(posixFileHandle, position, whence);
			}
		}

		fpl_platform_api uint32_t GetFilePosition32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if(fileHandle.internalHandle.posix.handle) {
				int posixFileHandle = fileHandle.internalHandle.posix.handle;
				off_t res = ::lseek(posixFileHandle, 0, SEEK_CUR);
				if(res != -1) {
					result = (uint32_t)res;
				}
			}
			return(result);
		}

		fpl_platform_api void CloseFile(FileHandle &fileHandle) {
			if(fileHandle.internalHandle.posix.handle) {
				int posixFileHandle = fileHandle.internalHandle.posix.handle;
				::close(posixFileHandle);
				fileHandle = {};
			}
		}

		fpl_platform_api uint32_t GetFileSize32(const char *filePath) {
			uint32_t result = 0;
			if(filePath != nullptr) {
				int posixFileHandle;
				do {
					posixFileHandle = ::open(filePath, O_RDONLY);
				} while(posixFileHandle == -1 && errno == EINTR);
				if(posixFileHandle != -1) {
					off_t res = ::lseek(posixFileHandle, 0, SEEK_END);
					if(res != -1) {
						result = (uint32_t)res;
					}
					::close(posixFileHandle);
				}
			}
			return(result);
		}

		fpl_platform_api uint32_t GetFileSize32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if(fileHandle.internalHandle.posix.handle) {
				int posixFileHandle = fileHandle.internalHandle.posix.handle;
				off_t curPos = ::lseek(posixFileHandle, 0, SEEK_CUR);
				if(curPos != -1) {
					result = (uint32_t)::lseek(posixFileHandle, 0, SEEK_END);
					::lseek(posixFileHandle, curPos, SEEK_SET);
				}
			}
			return(result);
		}

		fpl_platform_api bool FileExists(const char *filePath) {
			bool result = false;
			if(filePath != nullptr) {
				result = ::access(filePath, F_OK) != -1;
			}
			return(result);
		}

		fpl_platform_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
			if(sourceFilePath == nullptr) {
				common::PushError("Source file path parameter are not allowed to be null");
				return false;
			}
			if(targetFilePath == nullptr) {
				common::PushError("Target file path parameter are not allowed to be null");
				return false;
			}
			bool result = false;

			// @IMPLEMENT(final): POSIX FileCopy - there is no built-in copy-file function in POSIX so we open a file and buffer copy it over

			return(result);
		}

		fpl_platform_api bool FileMove(const char *sourceFilePath, const char *targetFilePath) {
			if(sourceFilePath == nullptr) {
				common::PushError("Source file path parameter are not allowed to be null");
				return false;
			}
			if(targetFilePath == nullptr) {
				common::PushError("Target file path parameter are not allowed to be null");
				return false;
			}
			bool result = ::rename(sourceFilePath, targetFilePath) == 0;
			return(result);
		}

		fpl_platform_api bool FileDelete(const char *filePath) {
			if(filePath == nullptr) {
				common::PushError("File path parameter are not allowed to be null");
				return false;
			}
			bool result = ::unlink(filePath) == 0;
			return(result);
		}

		fpl_platform_api bool DirectoryExists(const char *path) {
			bool result = false;
			if(path != nullptr) {
				struct stat sb;
				result = (::stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
			}
			return(result);
		}

		fpl_platform_api bool CreateDirectories(const char *path) {
			if(path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = ::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == 0;
			return(result);
		}
		fpl_platform_api bool RemoveEmptyDirectory(const char *path) {
			if(path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = ::rmdir(path) == 0;
			return(result);
		}
		fpl_platform_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry) {
			if(pathAndFilter == nullptr) {
				return false;
			}
			bool result = false;
			// @IMPLEMENT(final): POSIX ListFilesBegin
			return(result);
		}
		fpl_platform_api bool ListFilesNext(FileEntry &nextEntry) {
			bool result = false;
			if(nextEntry.internalHandle.posix.fileHandle) {
				// @IMPLEMENT(final): POSIX ListFilesNext
			}
			return(result);
		}
		fpl_platform_api void ListFilesEnd(FileEntry &lastEntry) {
			if(lastEntry.internalHandle.posix.fileHandle) {
				// @IMPLEMENT(final): POSIX ListFilesEnd
				lastEntry = {};
			}
		}
	} // files
} // fpl

#endif // FPL_SUBPLATFORM_POSIX

#if defined(FPL_SUBPLATFORM_X11)
namespace fpl {

#	if defined(FPL_ENABLE_WINDOW)
	namespace window {
		fpl_platform_api bool PushWindowEvent() {
			// @IMPLEMENT(final): X11 PushWindowEvent
			return false;
		}

		fpl_platform_api void UpdateGameControllers() {
			// @IMPLEMENT(final): X11 UpdateGameControllers
		}

		fpl_platform_api bool IsWindowRunning() {
			// @IMPLEMENT(final): X11 IsWindowRunning
			return false;
		}

		fpl_platform_api bool WindowUpdate() {
			// @IMPLEMENT(final): X11 WindowUpdate
			return false;
		}

		fpl_platform_api void SetWindowCursorEnabled(const bool value) {
			// @IMPLEMENT(final): X11 SetWindowCursorEnabled
		}

		fpl_platform_api WindowSize GetWindowArea() {
			WindowSize result = {};
			// @IMPLEMENT(final): X11 GetWindowArea
			return(result);
		}

		fpl_platform_api void SetWindowArea(const uint32_t width, const uint32_t height) {
			// @IMPLEMENT(final): X11 SetWindowArea
		}

		fpl_platform_api bool IsWindowResizable() {
			// @IMPLEMENT(final): X11 IsWindowResizable
			return false;
		}

		fpl_platform_api void SetWindowResizeable(const bool value) {
			// @IMPLEMENT(final): X11 SetWindowResizeable
		}

		fpl_platform_api bool SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
			// @IMPLEMENT(final): X11 SetWindowFullscreen
			return false;
		}

		fpl_platform_api bool IsWindowFullscreen() {
			// @IMPLEMENT(final): X11 IsWindowFullscreen
			return false;
		}

		fpl_platform_api WindowPosition GetWindowPosition() {
			WindowPosition result = {};
			// @IMPLEMENT(final): X11 GetWindowPosition
			return(result);
		}

		fpl_platform_api void SetWindowPosition(const int32_t left, const int32_t top) {
			// @IMPLEMENT(final): X11 SetWindowPosition
		}

		fpl_platform_api void SetWindowTitle(const char *title) {
			// @IMPLEMENT(final): X11 SetWindowTitle
		}

		fpl_platform_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
			// @IMPLEMENT(final): X11 GetClipboardAnsiText
			return nullptr;
		}

		fpl_platform_api char *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
			// @IMPLEMENT(final): X11 GetClipboardWideText
			return nullptr;
		}

		fpl_platform_api bool SetClipboardText(const char *ansiSource) {
			// @IMPLEMENT(final): X11 SetClipboardText (ansi)
			return false;
		}

		fpl_platform_api bool SetClipboardText(const wchar_t *wideSource) {
			// @IMPLEMENT(final): X11 SetClipboardText (wide)
			return false;
		}
	} // window
#	endif // FPL_ENABLE_WINDOW

#	if defined(FPL_ENABLE_VIDEO)
	namespace video {
		fpl_platform_api VideoBackBuffer *GetVideoBackBuffer() {
			// @IMPLEMENT(final): X11 GetVideoBackBuffer
			return nullptr;
		}

		fpl_platform_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
			// @IMPLEMENT(final): X11 ResizeVideoBackBuffer
			return false;
		}

		fpl_platform_api VideoDriverType GetVideoDriver() {
			// @IMPLEMENT(final): X11 GetVideoDriver
			return VideoDriverType::None;
		}
	}
#	endif // FPL_ENABLE_VIDEO

} // fpl
#endif // FPL_SUBPLATFORM_X11

// ****************************************************************************
//
// Linux Platform
//
// ****************************************************************************
#if defined(FPL_PLATFORM_LINUX)
#   include <ctype.h> // isspace

namespace fpl {
	namespace platform {
		fpl_internal void LinuxReleasePlatform(PlatformInitState &initState, PlatformAppState *appState) {
			LinuxAppState &linuxAppState = appState->linux;
			platform::POSIXReleasePlatform(linuxAppState.posix);
		}

		fpl_internal bool LinuxInitPlatform(const InitFlags initFlags, const Settings &initSettings, PlatformInitState &initState, PlatformAppState *appState) {
			LinuxInitState &linuxInitState = initState.linux;
			LinuxAppState &linuxAppState = appState->linux;
			if(!platform::POSIXInitPlatform(initFlags, initSettings, linuxInitState.posix, linuxAppState.posix)) {
				common::PushError("Failed initalizing POSIX platform");
				return false;
			}
			return true;
		}
	}

	// Linux Hardware
	namespace hardware {
		fpl_platform_api uint32_t GetProcessorCoreCount() {
			uint32_t result = sysconf(_SC_NPROCESSORS_ONLN);
			return(result);
		}

		fpl_platform_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
			if(destBuffer == nullptr) {
				common::PushError("Dest buffer parameter are not allowed to be null");
				return nullptr;
			}
			if(maxDestBufferLen == 0) {
				common::PushError("Max dest buffer len parameter '%d' must be greater than zero");
				return nullptr;
			}
			char *result = nullptr;
			FILE *fileHandle = fopen("/proc/cpuinfo", "rb");
			if(fileHandle != nullptr) {
				char buffer[256];
				char line[256];
				const size_t maxBufferSize = FPL_ARRAYCOUNT(buffer);
				int32_t readSize = maxBufferSize;
				int32_t readPos = 0;
				bool found = false;
				int bytesRead = 0;
				while((bytesRead = fread(&buffer[readPos], readSize, 1, fileHandle)) > 0) {
					char *lastP = &buffer[0];
					char *p = &buffer[0];
					while(*p) {
						if(*p == '\n') {
							int32_t len = p - lastP;
							FPL_ASSERT(len > 0);
							if(strings::IsStringEqual(lastP, 10, "model name", 10)) {
								strings::CopyAnsiString(lastP, len, line, FPL_ARRAYCOUNT(line));
								found = true;
								break;
							}
							lastP = p + 1;
						}
						++p;
					}
					if(found) {
						break;
					}

					int32_t remaining = &buffer[maxBufferSize] - lastP;
					FPL_ASSERT(remaining >= 0);
					if(remaining > 0) {
						// Buffer does not contain a line separator - copy back to remaining characters to the line
						strings::CopyAnsiString(lastP, remaining, line, FPL_ARRAYCOUNT(line));
						// Copy back line to buffer and use a different read position/size
						strings::CopyAnsiString(line, remaining, buffer, maxBufferSize);
						readPos = remaining;
						readSize = maxBufferSize - remaining;
					} else {
						readPos = 0;
						readSize = maxBufferSize;
					}
				}
				if(found) {
					char *p = line;
					while(*p) {
						if(*p == ':') {
							++p;
							// Skip whitespaces
							while(*p && isspace(*p)) {
								++p;
							}
							break;
						}
						++p;
					}
					if(p != line) {
						strings::CopyAnsiString(p, destBuffer, maxDestBufferLen);
						result = destBuffer;
					}
				}
				fclose(fileHandle);
			}
			return(result);
		}

		fpl_platform_api MemoryInfos GetSystemMemoryInfos() {
			MemoryInfos result = {};
			return(result);
		}
	} // hardware

	namespace paths {
		fpl_platform_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			return nullptr;
		}

		fpl_platform_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			return nullptr;
		}
	} // paths
} // fpl
#endif // FPL_PLATFORM_LINUX

// ****************************************************************************
//
// Video Drivers
//
// ****************************************************************************
namespace fpl {
	namespace drivers {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)

#		if defined(FPL_PLATFORM_WIN32)
		//
		// OpenGL Constants and Function-Prototypes
		//
#		define FPL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#		define FPL_GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#		define FPL_GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#		define FPL_GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#		define FPL_GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#		define FPL_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

#		define FPL_WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#		define FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#		define FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#		define FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#		define FPL_WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#		define FPL_WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#		define FPL_WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#		define FPL_WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#		define FPL_WGL_CONTEXT_FLAGS_ARB 0x2094

#		define FPL_WGL_DRAW_TO_WINDOW_ARB 0x2001
#		define FPL_WGL_ACCELERATION_ARB 0x2003
#		define FPL_WGL_SWAP_METHOD_ARB 0x2007
#		define FPL_WGL_SUPPORT_OPENGL_ARB 0x2010
#		define FPL_WGL_DOUBLE_BUFFER_ARB 0x2011
#		define FPL_WGL_PIXEL_TYPE_ARB 0x2013
#		define FPL_WGL_COLOR_BITS_ARB 0x2014
#		define FPL_WGL_DEPTH_BITS_ARB 0x2022
#		define FPL_WGL_STENCIL_BITS_ARB 0x2023
#		define FPL_WGL_FULL_ACCELERATION_ARB 0x2027
#		define FPL_WGL_SWAP_EXCHANGE_ARB 0x2028
#		define FPL_WGL_TYPE_RGBA_ARB 0x202B

#		define FPL_FUNC_WGL_MAKE_CURRENT(name) BOOL WINAPI name(HDC deviceContext, HGLRC renderingContext)
		typedef FPL_FUNC_WGL_MAKE_CURRENT(win32_func_wglMakeCurrent);
#		define FPL_FUNC_WGL_GET_PROC_ADDRESS(name) PROC WINAPI name(LPCSTR procedure)
		typedef FPL_FUNC_WGL_GET_PROC_ADDRESS(win32_func_wglGetProcAddress);
#		define FPL_FUNC_WGL_DELETE_CONTEXT(name) BOOL WINAPI name(HGLRC renderingContext)
		typedef FPL_FUNC_WGL_DELETE_CONTEXT(win32_func_wglDeleteContext);
#		define FPL_FUNC_WGL_CREATE_CONTEXT(name) HGLRC WINAPI name(HDC deviceContext)
		typedef FPL_FUNC_WGL_CREATE_CONTEXT(win32_func_wglCreateContext);

#		define FPL_FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
		typedef FPL_FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(win32_func_wglChoosePixelFormatARB);
#		define FPL_FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
		typedef FPL_FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(win32_func_wglCreateContextAttribsARB);
#		define FPL_FUNC_WGL_SWAP_INTERVAL_EXT(name) BOOL WINAPI name(int interval)
		typedef FPL_FUNC_WGL_SWAP_INTERVAL_EXT(win32_func_wglSwapIntervalEXT);

		struct Win32OpenGLFunctions {
			HMODULE openglLibrary;
			win32_func_wglMakeCurrent *wglMakeCurrent;
			win32_func_wglGetProcAddress *wglGetProcAddress;
			win32_func_wglDeleteContext *wglDeleteContext;
			win32_func_wglCreateContext *wglCreateContext;
			win32_func_wglChoosePixelFormatARB *wglChoosePixelFormatArb;
			win32_func_wglCreateContextAttribsARB *wglCreateContextAttribsArb;
			win32_func_wglSwapIntervalEXT *wglSwapIntervalExt;
		};

		struct Win32VideoOpenGLState {
			HGLRC renderingContext;
			Win32OpenGLFunctions glApi;
		};

		fpl_internal bool Win32PrepareWindowForOpenGL(platform::Win32AppState &appState, const platform::Win32WindowState &windowState, const VideoSettings &videoSettings) {
			const platform::Win32APIFunctions &wapi = appState.winApi;

			//
			// Prepare window for OpenGL
			//
			HDC deviceContext = windowState.deviceContext;
			HWND handle = windowState.windowHandle;

			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.cAlphaBits = 8;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = wapi.gdi.choosePixelFormat(deviceContext, &pfd);
			if(!pixelFormat) {
				common::PushError("Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			if(!wapi.gdi.setPixelFormat(deviceContext, pixelFormat, &pfd)) {
				common::PushError("Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			wapi.gdi.describePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

			return true;
		}

		fpl_internal bool Win32InitVideoOpenGL(const platform::Win32AppState &appState, const platform::Win32WindowState &windowState, const VideoSettings &videoSettings, Win32VideoOpenGLState &glState) {
			const platform::Win32APIFunctions &wapi = appState.winApi;
			Win32OpenGLFunctions &glFuncs = glState.glApi;

			//
			// Load OpenGL Library
			//
			{
				const char *openglLibraryName = "opengl32.dll";
				glFuncs.openglLibrary = ::LoadLibraryA("opengl32.dll");
				if(glFuncs.openglLibrary == nullptr) {
					common::PushError("Failed loading opengl library '%s'", openglLibraryName);
					return false;
				}

				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglGetProcAddress, win32_func_wglGetProcAddress, "wglGetProcAddress");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglCreateContext, win32_func_wglCreateContext, "wglCreateContext");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglDeleteContext, win32_func_wglDeleteContext, "wglDeleteContext");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglMakeCurrent, win32_func_wglMakeCurrent, "wglMakeCurrent");
			}

			//
			// Create opengl rendering context
			//
			HDC deviceContext = windowState.deviceContext;
			HGLRC legacyRenderingContext = glFuncs.wglCreateContext(deviceContext);
			if(!legacyRenderingContext) {
				common::PushError("Failed creating Legacy OpenGL Rendering Context for DC '%x')", deviceContext);
				return false;
			}

			if(!glFuncs.wglMakeCurrent(deviceContext, legacyRenderingContext)) {
				common::PushError("Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')", deviceContext, legacyRenderingContext);
				glFuncs.wglDeleteContext(legacyRenderingContext);
				return false;
			}

			// Load WGL Extensions
			glFuncs.wglSwapIntervalExt = (win32_func_wglSwapIntervalEXT *)glFuncs.wglGetProcAddress("wglSwapIntervalEXT");
			glFuncs.wglChoosePixelFormatArb = (win32_func_wglChoosePixelFormatARB *)glFuncs.wglGetProcAddress("wglChoosePixelFormatARB");
			glFuncs.wglCreateContextAttribsArb = (win32_func_wglCreateContextAttribsARB *)glFuncs.wglGetProcAddress("wglCreateContextAttribsARB");

			// Disable legacy context
			glFuncs.wglMakeCurrent(nullptr, nullptr);

			HGLRC activeRenderingContext;
			if(videoSettings.graphics.opengl.compabilityFlags != OpenGLCompabilityFlags::Legacy) {
				// @NOTE(final): This is only available in OpenGL 3.0+
				if(!(videoSettings.graphics.opengl.majorVersion >= 3 && videoSettings.graphics.opengl.minorVersion >= 0)) {
					common::PushError("You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
					return false;
				}

				if(glFuncs.wglChoosePixelFormatArb == nullptr) {
					common::PushError("wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
					return false;
				}
				if(glFuncs.wglCreateContextAttribsArb == nullptr) {
					common::PushError("wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
					return false;
				}

				int profile = 0;
				int flags = 0;
				if(videoSettings.graphics.opengl.compabilityFlags & OpenGLCompabilityFlags::Core) {
					profile = FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
				} else if(videoSettings.graphics.opengl.compabilityFlags & OpenGLCompabilityFlags::Compability) {
					profile = FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
				} else {
					common::PushError("No opengl compability profile selected, please specific Core OpenGLCompabilityFlags::Core or OpenGLCompabilityFlags::Compability");
					return false;
				}
				if(videoSettings.graphics.opengl.compabilityFlags & OpenGLCompabilityFlags::Forward) {
					flags = FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
				}

				int contextAttribIndex = 0;
				int contextAttribList[20 + 1] = {};
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MAJOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.graphics.opengl.majorVersion;
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MINOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.graphics.opengl.minorVersion;
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_PROFILE_MASK_ARB;
				contextAttribList[contextAttribIndex++] = profile;
				if(flags > 0) {
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FLAGS_ARB;
					contextAttribList[contextAttribIndex++] = flags;
				}

				// Create modern opengl rendering context
				HGLRC modernRenderingContext = glFuncs.wglCreateContextAttribsArb(deviceContext, 0, contextAttribList);
				if(modernRenderingContext) {
					if(!glFuncs.wglMakeCurrent(deviceContext, modernRenderingContext)) {
						common::PushError("Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings.graphics.opengl.majorVersion, videoSettings.graphics.opengl.minorVersion, videoSettings.graphics.opengl.compabilityFlags, deviceContext);

						glFuncs.wglDeleteContext(modernRenderingContext);
						modernRenderingContext = nullptr;

						// Fallback to legacy context
						glFuncs.wglMakeCurrent(deviceContext, legacyRenderingContext);
						activeRenderingContext = legacyRenderingContext;
					} else {
						// Destroy legacy rendering context
						glFuncs.wglDeleteContext(legacyRenderingContext);
						legacyRenderingContext = nullptr;
						activeRenderingContext = modernRenderingContext;
					}
				} else {
					common::PushError("Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings.graphics.opengl.majorVersion, videoSettings.graphics.opengl.minorVersion, videoSettings.graphics.opengl.compabilityFlags, deviceContext);

					// Fallback to legacy context
					glFuncs.wglMakeCurrent(deviceContext, legacyRenderingContext);
					activeRenderingContext = legacyRenderingContext;
				}
			} else {
				// Caller wants legacy context
				glFuncs.wglMakeCurrent(deviceContext, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			}

			FPL_ASSERT(activeRenderingContext != nullptr);

			glState.renderingContext = activeRenderingContext;

			// Set vertical syncronisation if available
			if(glFuncs.wglSwapIntervalExt != nullptr) {
				int swapInterval = videoSettings.isVSync ? 1 : 0;
				glFuncs.wglSwapIntervalExt(swapInterval);
			}

			return true;
		}

		fpl_internal void Win32ReleaseVideoOpenGL(Win32VideoOpenGLState &glState) {
			Win32OpenGLFunctions &glFuncs = glState.glApi;
			if(glState.renderingContext) {
				glFuncs.wglMakeCurrent(nullptr, nullptr);
				glFuncs.wglDeleteContext(glState.renderingContext);
			}
			if(glFuncs.openglLibrary != nullptr) {
				::FreeLibrary(glFuncs.openglLibrary);
			}
			glState = {};
		}
#		endif // FPL_PLATFORM_WIN32

#	endif // FPL_ENABLE_VIDEO_OPENGL

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)

#		if defined(FPL_PLATFORM_WIN32)
		struct Win32VideoSoftwareState {
			BITMAPINFO bitmapInfo;
		};

		fpl_internal bool Win32InitVideoSoftware(const video::VideoBackBuffer &backbuffer, Win32VideoSoftwareState &software) {
			software = {};
			software.bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			software.bitmapInfo.bmiHeader.biWidth = (LONG)backbuffer.width;
			software.bitmapInfo.bmiHeader.biHeight = (LONG)backbuffer.height;
			software.bitmapInfo.bmiHeader.biBitCount = 32;
			software.bitmapInfo.bmiHeader.biCompression = BI_RGB;
			software.bitmapInfo.bmiHeader.biPlanes = 1;
			software.bitmapInfo.bmiHeader.biSizeImage = (DWORD)(backbuffer.height * backbuffer.lineWidth);
			return true;
		}

		fpl_internal void Win32ReleaseVideoSoftware(Win32VideoSoftwareState &softwareState) {
			softwareState = {};
		}
#		endif // FPL_PLATFORM_WIN32

#	endif // FPL_ENABLE_VIDEO_SOFTWARE
	}
}

// ****************************************************************************
//
// Audio Drivers
//
// ****************************************************************************
#if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
#	include <mmreg.h>
#	include <dsound.h>
#endif
namespace fpl {
	namespace drivers {
#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		//
		// DirectSound
		//
#		define FPL_FUNC_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(const GUID* pcGuidDevice, LPDIRECTSOUND *ppDS8, LPUNKNOWN pUnkOuter)
		typedef FPL_FUNC_DIRECT_SOUND_CREATE(func_DirectSoundCreate);
#		define FPL_FUNC_DIRECT_SOUND_ENUMERATE_A(name) HRESULT WINAPI name(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
		typedef FPL_FUNC_DIRECT_SOUND_ENUMERATE_A(func_DirectSoundEnumerateA);

		static const GUID FPL_IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16} };

		fpl_constant uint32_t FPL_DIRECTSOUND_MAX_PERIODS = 4;

		struct DirectSoundState {
			HMODULE dsoundLibrary;
			LPDIRECTSOUND directSound;
			LPDIRECTSOUNDBUFFER primaryBuffer;
			LPDIRECTSOUNDBUFFER secondaryBuffer;
			LPDIRECTSOUNDNOTIFY notify;
			HANDLE notifyEvents[FPL_DIRECTSOUND_MAX_PERIODS];
			HANDLE stopEvent;
			uint32_t lastProcessedFrame;
			bool breakMainLoop;
		};

		struct DirectSoundDeviceInfos {
			uint32_t foundDeviceCount;
			AudioDeviceID *devices;
			uint32_t maxDeviceCount;
		};

		fpl_internal BOOL CALLBACK GetDeviceCallbackDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext) {
			(void)lpcstrModule;

			DirectSoundDeviceInfos *infos = (DirectSoundDeviceInfos *)lpContext;
			FPL_ASSERT(infos != nullptr);
			if(infos->devices != nullptr) {
				uint32_t index = infos->foundDeviceCount++;
				if(index < infos->maxDeviceCount) {
					AudioDeviceID *deviceId = infos->devices + index;
					*deviceId = {};
					strings::CopyAnsiString(lpcstrDescription, deviceId->name, FPL_ARRAYCOUNT(deviceId->name));
					if(lpGuid != nullptr) {
						memory::MemoryCopy(lpGuid, sizeof(deviceId->dshow), &deviceId->dshow);
					}
					return TRUE;
				}
			}
			return FALSE;
		}

		fpl_internal uint32_t GetDevicesDirectSound(DirectSoundState &dsoundState, AudioDeviceID *devices, uint32_t maxDeviceCount) {
			uint32_t result = 0;
			func_DirectSoundEnumerateA *directSoundEnumerateA = (func_DirectSoundEnumerateA *)::GetProcAddress(dsoundState.dsoundLibrary, "DirectSoundEnumerateA");
			if(directSoundEnumerateA != nullptr) {
				DirectSoundDeviceInfos infos = {};
				infos.maxDeviceCount = maxDeviceCount;
				infos.devices = devices;
				directSoundEnumerateA(GetDeviceCallbackDirectSound, &infos);
				result = infos.foundDeviceCount;
			}
			return(result);
		}

		fpl_internal bool ReleaseDirectSound(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			if(dsoundState.dsoundLibrary != nullptr) {
				if(dsoundState.stopEvent != nullptr) {
					::CloseHandle(dsoundState.stopEvent);
				}

				for(uint32_t i = 0; i < commonAudio.internalFormat.periods; ++i) {
					if(dsoundState.notifyEvents[i]) {
						::CloseHandle(dsoundState.notifyEvents[i]);
					}
				}

				if(dsoundState.notify != nullptr) {
					dsoundState.notify->Release();
				}

				if(dsoundState.secondaryBuffer != nullptr) {
					dsoundState.secondaryBuffer->Release();
				}

				if(dsoundState.primaryBuffer != nullptr) {
					dsoundState.primaryBuffer->Release();
				}

				if(dsoundState.directSound != nullptr) {
					dsoundState.directSound->Release();
				}

				::FreeLibrary(dsoundState.dsoundLibrary);
				dsoundState = {};
			}

			return true;
		}

		fpl_internal audio::AudioResult InitDirectSound(const AudioSettings &audioSettings, common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			platform::Win32AppState &win32AppState = appState->win32;
			const platform::Win32APIFunctions &apiFuncs = win32AppState.winApi;

			// Load direct sound library
			dsoundState.dsoundLibrary = ::LoadLibraryA("dsound.dll");
			func_DirectSoundCreate *directSoundCreate = (func_DirectSoundCreate *)::GetProcAddress(dsoundState.dsoundLibrary, "DirectSoundCreate");
			if(directSoundCreate == nullptr) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Load direct sound object
			const GUID *deviceId = nullptr;
			if(strings::GetAnsiStringLength(audioSettings.deviceID.name) > 0) {
				deviceId = &audioSettings.deviceID.dshow;
			}
			if(!SUCCEEDED(directSoundCreate(deviceId, &dsoundState.directSound, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Setup wave format ex
			WAVEFORMATEXTENSIBLE wf = {};
			wf.Format.cbSize = sizeof(wf);
			wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			wf.Format.nChannels = (WORD)audioSettings.deviceFormat.channels;
			wf.Format.nSamplesPerSec = (DWORD)audioSettings.deviceFormat.sampleRate;
			wf.Format.wBitsPerSample = audio::GetAudioSampleSizeInBytes(audioSettings.deviceFormat.type) * 8;
			wf.Format.nBlockAlign = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
			wf.Format.nAvgBytesPerSec = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
			wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
			if((audioSettings.deviceFormat.type == AudioFormatType::F32) || (audioSettings.deviceFormat.type == AudioFormatType::F64)) {
				wf.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
			} else {
				wf.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			}

			// Get either local window handle or desktop handle
			HWND windowHandle = nullptr;
#		if defined(FPL_ENABLE_WINDOW)
			if(appState->initFlags & InitFlags::Window) {
				windowHandle = appState->window.win32.windowHandle;
			}
#		endif
			if(windowHandle == nullptr) {
				windowHandle = apiFuncs.user.getDesktopWindow();
			}

			// The cooperative level must be set before doing anything else
			if(FAILED(dsoundState.directSound->SetCooperativeLevel(windowHandle, (audioSettings.preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Create primary buffer
			DSBUFFERDESC descDSPrimary = {};
			descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
			descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
			if(FAILED(dsoundState.directSound->CreateSoundBuffer(&descDSPrimary, &dsoundState.primaryBuffer, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Set format
			if(FAILED(dsoundState.primaryBuffer->SetFormat((WAVEFORMATEX*)&wf))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Get the required size in bytes
			DWORD requiredSize;
			if(FAILED(dsoundState.primaryBuffer->GetFormat(nullptr, 0, &requiredSize))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Get actual format
			char rawdata[1024];
			WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
			if(FAILED(dsoundState.primaryBuffer->GetFormat((WAVEFORMATEX*)pActualFormat, requiredSize, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Set internal format
			AudioDeviceFormat internalFormat = {};
			if(IsEqualGUID(pActualFormat->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
				if(pActualFormat->Format.wBitsPerSample == 64) {
					internalFormat.type = AudioFormatType::F64;
				} else {
					internalFormat.type = AudioFormatType::F32;
				}
			} else {
				switch(pActualFormat->Format.wBitsPerSample) {
					case 8:
						internalFormat.type = AudioFormatType::U8;
						break;
					case 16:
						internalFormat.type = AudioFormatType::S16;
						break;
					case 24:
						internalFormat.type = AudioFormatType::S24;
						break;
					case 32:
						internalFormat.type = AudioFormatType::S32;
						break;
					case 64:
						internalFormat.type = AudioFormatType::S64;
						break;
				}
			}
			internalFormat.channels = pActualFormat->Format.nChannels;
			internalFormat.sampleRate = pActualFormat->Format.nSamplesPerSec;

			// @NOTE(final): We divide up our playback buffer into this number of periods and let directsound notify us when one of it needs to play.
			internalFormat.periods = 2;
			internalFormat.bufferSizeInFrames = audio::GetAudioBufferSizeInFrames(internalFormat.sampleRate, audioSettings.bufferSizeInMilliSeconds);
			internalFormat.bufferSizeInBytes = internalFormat.bufferSizeInFrames * internalFormat.channels * audio::GetAudioSampleSizeInBytes(internalFormat.type);

			commonAudio.internalFormat = internalFormat;

			// Create secondary buffer
			DSBUFFERDESC descDS = {};
			descDS.dwSize = sizeof(DSBUFFERDESC);
			descDS.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
			descDS.dwBufferBytes = (DWORD)internalFormat.bufferSizeInBytes;
			descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;
			if(FAILED(dsoundState.directSound->CreateSoundBuffer(&descDS, &dsoundState.secondaryBuffer, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
			if(FAILED(dsoundState.secondaryBuffer->QueryInterface(FPL_IID_IDirectSoundNotify, (void**)&dsoundState.notify))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Setup notifications
			uint32_t periodSizeInBytes = internalFormat.bufferSizeInBytes / internalFormat.periods;
			DSBPOSITIONNOTIFY notifyPoints[FPL_DIRECTSOUND_MAX_PERIODS];
			for(uint32_t i = 0; i < internalFormat.periods; ++i) {
				dsoundState.notifyEvents[i] = CreateEventA(nullptr, false, false, nullptr);
				if(dsoundState.notifyEvents[i] == nullptr) {
					ReleaseDirectSound(commonAudio, dsoundState);
					return audio::AudioResult::Failed;
				}

				// The notification offset is in bytes.
				notifyPoints[i].dwOffset = i * periodSizeInBytes;
				notifyPoints[i].hEventNotify = dsoundState.notifyEvents[i];
			}
			if(FAILED(dsoundState.notify->SetNotificationPositions(internalFormat.periods, notifyPoints))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Create stop event
			dsoundState.stopEvent = ::CreateEventA(nullptr, false, false, nullptr);
			if(dsoundState.stopEvent == nullptr) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			return audio::AudioResult::Success;
		}

		fpl_internal_inline void StopMainLoopDirectSound(DirectSoundState &dsoundState) {
			dsoundState.breakMainLoop = true;
			::SetEvent(dsoundState.stopEvent);
		}

		fpl_internal bool GetCurrentFrameDirectSound(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState, uint32_t* pCurrentPos) {
			FPL_ASSERT(pCurrentPos != nullptr);
			*pCurrentPos = 0;

			FPL_ASSERT(dsoundState.secondaryBuffer != nullptr);
			DWORD dwCurrentPosition;
			if(FAILED(dsoundState.secondaryBuffer->GetCurrentPosition(nullptr, &dwCurrentPosition))) {
				return false;
			}

			FPL_ASSERT(commonAudio.internalFormat.channels > 0);
			*pCurrentPos = (uint32_t)dwCurrentPosition / audio::GetAudioSampleSizeInBytes(commonAudio.internalFormat.type) / commonAudio.internalFormat.channels;
			return true;
		}

		fpl_internal uint32_t GetAvailableFramesDirectSound(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			// Get current frame from current play position
			uint32_t currentFrame;
			if(!GetCurrentFrameDirectSound(commonAudio, dsoundState, &currentFrame)) {
				return 0;
			}

			// In a playback device the last processed frame should always be ahead of the current frame. The space between
			// the last processed and current frame (moving forward, starting from the last processed frame) is the amount
			// of space available to write.
			uint32_t totalFrameCount = commonAudio.internalFormat.bufferSizeInFrames;
			uint32_t committedBeg = currentFrame;
			uint32_t committedEnd;
			committedEnd = dsoundState.lastProcessedFrame;
			if(committedEnd <= committedBeg) {
				committedEnd += totalFrameCount;
			}

			uint32_t committedSize = (committedEnd - committedBeg);
			FPL_ASSERT(committedSize <= totalFrameCount);

			return totalFrameCount - committedSize;
		}

		fpl_internal uint32_t WaitForFramesDirectSound(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(commonAudio.internalFormat.sampleRate > 0);
			FPL_ASSERT(commonAudio.internalFormat.periods > 0);

			// The timeout to use for putting the thread to sleep is based on the size of the buffer and the period count.
			DWORD timeoutInMilliseconds = (commonAudio.internalFormat.bufferSizeInFrames / (commonAudio.internalFormat.sampleRate / 1000)) / commonAudio.internalFormat.periods;
			if(timeoutInMilliseconds < 1) {
				timeoutInMilliseconds = 1;
			}

			// Copy event handles so we can wait for each one
			unsigned int eventCount = commonAudio.internalFormat.periods + 1;
			HANDLE pEvents[FPL_DIRECTSOUND_MAX_PERIODS + 1]; // +1 for the stop event.
			memory::MemoryCopy(dsoundState.notifyEvents, sizeof(HANDLE) * commonAudio.internalFormat.periods, pEvents);
			pEvents[eventCount - 1] = dsoundState.stopEvent;

			while(!dsoundState.breakMainLoop) {
				// Get available frames from directsound
				uint32_t framesAvailable = GetAvailableFramesDirectSound(commonAudio, dsoundState);
				if(framesAvailable > 0) {
					return framesAvailable;
				}

				// If we get here it means we weren't able to find any frames. We'll just wait here for a bit.
				::WaitForMultipleObjects(eventCount, pEvents, FALSE, timeoutInMilliseconds);
			}

			// We'll get here if the loop was terminated. Just return whatever's available.
			return GetAvailableFramesDirectSound(commonAudio, dsoundState);
		}

		fpl_internal bool StopDirectSound(DirectSoundState &dsoundState) {
			FPL_ASSERT(dsoundState.secondaryBuffer != nullptr);
			if(FAILED(dsoundState.secondaryBuffer->Stop())) {
				return false;
			}
			dsoundState.secondaryBuffer->SetCurrentPosition(0);
			return true;
		}

		fpl_internal audio::AudioResult StartDirectSound(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(commonAudio.internalFormat.channels > 0);
			FPL_ASSERT(commonAudio.internalFormat.periods > 0);
			uint32_t audioSampleSizeBytes = audio::GetAudioSampleSizeInBytes(commonAudio.internalFormat.type);
			FPL_ASSERT(audioSampleSizeBytes > 0);

			// Before playing anything we need to grab an initial group of samples from the client.
			uint32_t framesToRead = commonAudio.internalFormat.bufferSizeInFrames / commonAudio.internalFormat.periods;
			uint32_t desiredLockSize = framesToRead * commonAudio.internalFormat.channels * audioSampleSizeBytes;

			void* pLockPtr;
			DWORD actualLockSize;
			void* pLockPtr2;
			DWORD actualLockSize2;
			if(SUCCEEDED(dsoundState.secondaryBuffer->Lock(0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
				framesToRead = actualLockSize / audioSampleSizeBytes / commonAudio.internalFormat.channels;
				common::ReadAudioFramesFromClient(commonAudio, framesToRead, pLockPtr);
				dsoundState.secondaryBuffer->Unlock(pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
				dsoundState.lastProcessedFrame = framesToRead;
				if(FAILED(dsoundState.secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
					return audio::AudioResult::Failed;
				}
			} else {
				return audio::AudioResult::Failed;
			}
			return audio::AudioResult::Success;
		}

		fpl_internal void DirectSoundMainLoop(const common::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(commonAudio.internalFormat.channels > 0);
			uint32_t audioSampleSizeBytes = audio::GetAudioSampleSizeInBytes(commonAudio.internalFormat.type);
			FPL_ASSERT(audioSampleSizeBytes > 0);

			// Make sure the stop event is not signaled to ensure we don't end up immediately returning from WaitForMultipleObjects().
			::ResetEvent(dsoundState.stopEvent);

			// Main loop
			dsoundState.breakMainLoop = false;
			while(!dsoundState.breakMainLoop) {
				// Wait until we get available frames from directsound
				uint32_t framesAvailable = WaitForFramesDirectSound(commonAudio, dsoundState);
				if(framesAvailable == 0) {
					continue;
				}

				// Don't bother grabbing more data if the device is being stopped.
				if(dsoundState.breakMainLoop) {
					break;
				}

				// Lock playback buffer
				DWORD lockOffset = dsoundState.lastProcessedFrame * commonAudio.internalFormat.channels * audioSampleSizeBytes;
				DWORD lockSize = framesAvailable * commonAudio.internalFormat.channels * audioSampleSizeBytes;
				{
					void* pLockPtr;
					DWORD actualLockSize;
					void* pLockPtr2;
					DWORD actualLockSize2;
					if(FAILED(dsoundState.secondaryBuffer->Lock(lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
						// @TODO(final): Handle error
						break;
					}

					// Read actual frames from user
					uint32_t frameCount = actualLockSize / audioSampleSizeBytes / commonAudio.internalFormat.channels;
					common::ReadAudioFramesFromClient(commonAudio, frameCount, pLockPtr);
					dsoundState.lastProcessedFrame = (dsoundState.lastProcessedFrame + frameCount) % commonAudio.internalFormat.bufferSizeInFrames;

					// Unlock playback buffer
					dsoundState.secondaryBuffer->Unlock(pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
				}
			}
		}
#	endif //FPL_ENABLE_AUDIO_DIRECTSOUND

	} // drivers
} // fpl

// ****************************************************************************
//
// Platform Independent (Audio)
//
// The audio system is based on a stripped down version of "mini_al.h" by David Reid.
// See: https://github.com/dr-soft/mini_al
//
// ****************************************************************************
namespace fpl {

	namespace common {
		//
		// Audio
		//
#if defined(FPL_ENABLE_AUDIO)
		enum class AudioDeviceState : uint32_t {
			Uninitialized = 0,
			Stopped,
			Started,
			Starting,
			Stopping,
		};

		struct AudioState {
			CommonAudioState common;

			threading::ThreadMutex lock;
			threading::ThreadContext *workerThread;
			threading::ThreadSignal startSignal;
			threading::ThreadSignal stopSignal;
			threading::ThreadSignal wakeupSignal;
			volatile AudioDeviceState state;
			volatile audio::AudioResult workResult;

			AudioDriverType activeDriver;
			uint32_t periods;
			uint32_t bufferSizeInFrames;
			uint32_t bufferSizeInBytes;
			bool isAsyncDriver;

			union {
#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				drivers::DirectSoundState dsound;
#			endif
			};
		};

		fpl_internal void AudioDeviceStopMainLoop(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			switch(audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					drivers::StopMainLoopDirectSound(audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
		}

		fpl_internal bool AudioReleaseDevice(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			bool result = false;
			switch(audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = drivers::ReleaseDirectSound(audioState.common, audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
			return (result);
		}

		fpl_internal bool AudioStopDevice(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			bool result = false;
			switch(audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = drivers::StopDirectSound(audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
			return (result);
		}

		fpl_internal audio::AudioResult AudioStartDevice(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			audio::AudioResult result = audio::AudioResult::Failed;
			switch(audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = drivers::StartDirectSound(audioState.common, audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
			return (result);
		}

		fpl_internal void AudioDeviceMainLoop(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			switch(audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					drivers::DirectSoundMainLoop(audioState.common, audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
		}

		fpl_internal_inline bool IsAudioDriverAsync(AudioDriverType audioDriver) {
			switch(audioDriver) {
				case AudioDriverType::DirectSound:
					return false;
				default:
					return false;
			}
		}

		fpl_internal_inline void AudioSetDeviceState(AudioState &audioState, AudioDeviceState newState) {
			atomics::AtomicStoreU32((volatile uint32_t *)&audioState.state, (uint32_t)newState);
		}

		fpl_internal_inline AudioDeviceState AudioGetDeviceState(AudioState &audioState) {
			AudioDeviceState result = (AudioDeviceState)atomics::AtomicLoadU32((volatile uint32_t *)&audioState.state);
			return(result);
		}

		fpl_internal_inline bool IsAudioDeviceInitialized(AudioState *audioState) {
			if(audioState == nullptr) {
				return false;
			}
			AudioDeviceState state = AudioGetDeviceState(*audioState);
			return(state != AudioDeviceState::Uninitialized);
		}

		fpl_internal_inline bool IsAudioDeviceStarted(AudioState *audioState) {
			if(audioState == nullptr) {
				return false;
			}
			AudioDeviceState state = AudioGetDeviceState(*audioState);
			return(state == AudioDeviceState::Started);
		}

		fpl_internal void AudioWorkerThread(const threading::ThreadContext &context, void *data) {
#		if defined(FPL_PLATFORM_WIN32)
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
#		endif

			AudioState *audioState = (AudioState *)data;
			FPL_ASSERT(audioState != nullptr);
			FPL_ASSERT(audioState->activeDriver != AudioDriverType::None);

#		if defined(FPL_PLATFORM_WIN32)
			wapi.ole.coInitializeEx(nullptr, 0);
#		endif

			for(;;) {
				// Stop the device at the start of the iteration always
				AudioStopDevice(*audioState);

				// Let the other threads know that the device has been stopped.
				AudioSetDeviceState(*audioState, AudioDeviceState::Stopped);
				threading::SignalSet(audioState->stopSignal);

				// We wait until the audio device gets wake up
				threading::SignalWaitForOne(audioState->lock, audioState->wakeupSignal);

				// Default result code.
				audioState->workResult = audio::AudioResult::Success;

				// Just break if we're terminating.
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Uninitialized) {
					break;
				}

				// Expect that the device is currently be started by the client
				FPL_ASSERT(AudioGetDeviceState(*audioState) == AudioDeviceState::Starting);

				// Start audio device
				audioState->workResult = AudioStartDevice(*audioState);
				if(audioState->workResult != audio::AudioResult::Success) {
					threading::SignalSet(audioState->startSignal);
					continue;
				}

				// The audio device is started, mark it as such
				AudioSetDeviceState(*audioState, AudioDeviceState::Started);
				threading::SignalSet(audioState->startSignal);

				// Enter audio device main loop
				AudioDeviceMainLoop(*audioState);
			}

			// Signal to stop any audio threads, in case there are some waiting
			threading::SignalSet(audioState->stopSignal);

#		if defined(FPL_PLATFORM_WIN32)
			wapi.ole.coUninitialize();
#		endif
		}

		fpl_internal AudioState *AllocateAudioState(platform::PlatformAudioState &platAudioState) {
			platAudioState = {};
			platAudioState.memSize = sizeof(AudioState);
			platAudioState.mem = memory::MemoryAlignedAllocate(platAudioState.memSize, 16);
			AudioState *result = (AudioState *)platAudioState.mem;
			return(result);
		}

		fpl_internal void FreeAudioState(platform::PlatformAudioState &platAudioState) {
			if(platAudioState.mem != nullptr) {
				memory::MemoryAlignedFree(platAudioState.mem);
			}
			platAudioState = {};
		}

		fpl_internal void ReleaseAudio() {
#		if defined(FPL_PLATFORM_WIN32)
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
#		endif

			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;

			if(platAudioState.mem != nullptr) {
				AudioState *audioState = (AudioState *)platAudioState.mem;

				if(IsAudioDeviceInitialized(audioState)) {

					// Wait until the audio device is stopped
					if(IsAudioDeviceStarted(audioState)) {
						while(audio::StopAudio() == audio::AudioResult::DeviceBusy) {
							threading::ThreadSleep(1);
						}
					}

					// Putting the device into an uninitialized state will make the worker thread return.
					AudioSetDeviceState(*audioState, AudioDeviceState::Uninitialized);

					// Wake up the worker thread and wait for it to properly terminate.
					SignalSet(audioState->wakeupSignal);
					ThreadWaitForOne(audioState->workerThread);
					ThreadDestroy(audioState->workerThread);

					// Release signals and thread
					threading::SignalDestroy(audioState->stopSignal);
					threading::SignalDestroy(audioState->startSignal);
					threading::SignalDestroy(audioState->wakeupSignal);
					threading::MutexDestroy(audioState->lock);

					// Release audio device
					AudioReleaseDevice(*audioState);
				}

				// Release state memory
				FreeAudioState(platAudioState);
			}

#		if defined(FPL_PLATFORM_WIN32)
			wapi.ole.coUninitialize();
#		endif
		}

		fpl_internal audio::AudioResult InitAudio(const AudioSettings &audioSettings) {
#		if defined(FPL_PLATFORM_WIN32)
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::Win32APIFunctions &wapi = platform::global__AppState->win32.winApi;
#		endif

			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			FPL_ASSERT(platAudioState.mem == nullptr);
			AudioState *audioState = AllocateAudioState(platAudioState);

			if(audioState->activeDriver != AudioDriverType::None) {
				FreeAudioState(platAudioState);
				return audio::AudioResult::Failed;
			}

			if(audioSettings.deviceFormat.channels == 0) {
				FreeAudioState(platAudioState);
				return audio::AudioResult::Failed;
			}
			if(audioSettings.deviceFormat.sampleRate == 0) {
				FreeAudioState(platAudioState);
				return audio::AudioResult::Failed;
			}
			if(audioSettings.bufferSizeInMilliSeconds == 0) {
				FreeAudioState(platAudioState);
				return audio::AudioResult::Failed;
			}

			audioState->common.clientReadCallback = audioSettings.clientReadCallback;
			audioState->common.clientUserData = audioSettings.userData;

#		if defined(FPL_PLATFORM_WIN32)
			wapi.ole.coInitializeEx(nullptr, 0);
#		endif

			// Create mutex and signals
			audioState->lock = threading::MutexCreate();
			if(!audioState->lock.isValid) {
				ReleaseAudio();
				return audio::AudioResult::Failed;
			}
			audioState->wakeupSignal = threading::SignalCreate();
			if(!audioState->wakeupSignal.isValid) {
				ReleaseAudio();
				return audio::AudioResult::Failed;
			}
			audioState->startSignal = threading::SignalCreate();
			if(!audioState->startSignal.isValid) {
				ReleaseAudio();
				return audio::AudioResult::Failed;
			}
			audioState->stopSignal = threading::SignalCreate();
			if(!audioState->stopSignal.isValid) {
				ReleaseAudio();
				return audio::AudioResult::Failed;
			}

			// Prope drivers
			AudioDriverType propeDrivers[16];
			uint32_t driverCount = 0;
			if(audioSettings.driver == AudioDriverType::Auto) {
				// @NOTE(final): Add all audio drivers here, regardless of the platform.
				propeDrivers[driverCount++] = AudioDriverType::DirectSound;
			} else {
				// @NOTE(final): Forced audio driver
				propeDrivers[driverCount++] = audioSettings.driver;
			}
			audio::AudioResult initResult = audio::AudioResult::Failed;
			for(uint32_t driverIndex = 0; driverIndex < driverCount; ++driverIndex) {
				AudioDriverType propeDriver = propeDrivers[driverIndex];

				initResult = audio::AudioResult::Failed;
				switch(propeDriver) {

#				if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
					case AudioDriverType::DirectSound:
					{
						initResult = drivers::InitDirectSound(audioSettings, audioState->common, audioState->dsound);
						if(initResult != audio::AudioResult::Success) {
							drivers::ReleaseDirectSound(audioState->common, audioState->dsound);
						}
					} break;
#				endif

					default:
						break;
				}
				if(initResult == audio::AudioResult::Success) {
					audioState->activeDriver = propeDriver;
					audioState->isAsyncDriver = IsAudioDriverAsync(propeDriver);
					break;
				}
			}

			if(initResult != audio::AudioResult::Success) {
				ReleaseAudio();
				return initResult;
			}

			if(!audioState->isAsyncDriver) {
				// Create and start worker thread
				audioState->workerThread = threading::ThreadCreate(AudioWorkerThread, audioState);
				if(audioState->workerThread == nullptr) {
					ReleaseAudio();
					return audio::AudioResult::Failed;
				}
				// Wait for the worker thread to put the device into the stopped state.
				SignalWaitForOne(audioState->lock, audioState->stopSignal);
			} else {
				AudioSetDeviceState(*audioState, AudioDeviceState::Stopped);
			}

			FPL_ASSERT(AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped);

			return(audio::AudioResult::Success);
		}

#endif // FPL_ENABLE_AUDIO

#if defined(FPL_ENABLE_WINDOW)
		fpl_internal bool InitWindow(const Settings &initSettings, WindowSettings &currentWindowSettings, platform::PlatformAppState *appState, const WindowForVideoSettings &windowForVideoSettings, platform::callback_PrepareWindowForVideoAfter *prepareWindowForVideoAfter) {
			bool result = false;
			if(appState != nullptr) {
#		if defined(FPL_PLATFORM_WIN32)
				result = platform::Win32InitWindow(initSettings, currentWindowSettings, appState, appState->win32, appState->window.win32, prepareWindowForVideoAfter);
#		elif defined(FPL_SUBPLATFORM_X11)
				// @TODO(final): Implement this!
				// For GLX you have to do the following steps:
				// - windowForVideoSettings.beforeCallback() to get a XVisualInfo back.
				// - Create a colormap from with that visual info
				// - Pass the colormap to the XSetWindowAttributes
				// - Create the video with the properties
				// See: https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib
#		endif
			}
			return (result);
		}

		fpl_internal void ReleaseWindow(const platform::PlatformInitState &initState, platform::PlatformAppState *appState) {
			if(appState != nullptr) {
#		if defined(FPL_PLATFORM_WIN32)
				platform::Win32ReleaseWindow(initState.win32, appState->win32, appState->window.win32);
#		endif
			}
		}
#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_VIDEO)
		struct VideoState {
			union {
#			if defined(FPL_PLATFORM_WIN32)
				struct {
#				if defined(FPL_ENABLE_VIDEO_OPENGL)
					drivers::Win32VideoOpenGLState opengl;
#				endif
#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					drivers::Win32VideoSoftwareState software;
#				endif
				} win32;
#			endif // FPL_PLATFORM_WIN32
			};
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			video::VideoBackBuffer softwareBackbuffer;
#		endif
		};

		fpl_internal void ReleaseVideo(platform::PlatformAppState *appState) {
			if(appState != nullptr) {
				if(appState->video.mem != nullptr) {
					VideoState *videoState = (VideoState *)appState->video.mem;

					switch(appState->currentSettings.video.driver) {
#					if defined(FPL_ENABLE_VIDEO_OPENGL)
						case VideoDriverType::OpenGL:
						{
#						if defined(FPL_PLATFORM_WIN32)
							drivers::Win32ReleaseVideoOpenGL(videoState->win32.opengl);
#						endif
						} break;
#					endif // FPL_ENABLE_VIDEO_OPENGL

#					if defined(FPL_ENABLE_VIDEO_SOFTWARE)
						case VideoDriverType::Software:
						{
#						if defined(FPL_PLATFORM_WIN32)
							drivers::Win32ReleaseVideoSoftware(videoState->win32.software);
#						endif
						} break;
#					endif // FPL_ENABLE_VIDEO_SOFTWARE

						default:
						{
						} break;
					}

#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					fpl::video::VideoBackBuffer &backbuffer = videoState->softwareBackbuffer;
					if(backbuffer.pixels != nullptr) {
						memory::MemoryAlignedFree(backbuffer.pixels);
					}
					backbuffer = {};
#				endif

					memory::MemoryAlignedFree(appState->video.mem);
					appState->video = {};
				}
				appState->currentSettings.video.driver = VideoDriverType::None;
			}
		}

		fpl_internal FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_AFTER(PrepareWindowForVideoAfter) {
			switch(videoSettings.driver) {
#				if defined(FPL_ENABLE_VIDEO_OPENGL)
				case VideoDriverType::OpenGL:
				{
#				if defined(FPL_PLATFORM_WIN32)
					if(!drivers::Win32PrepareWindowForOpenGL(appState->win32, appState->window.win32, videoSettings)) {
						return false;
					}
#	
#				endif
				} break;
#				endif // FPL_ENABLE_VIDEO_OPENGL

				default:
				{
				} break;
			}
			return true;
		}

		fpl_internal FPL_FUNC_PREPARE_WINDOW_FOR_VIDEO_BEFORE(PrepareWindowForVideoBefore) {
			platform::PlatformAppState *appState = platform::global__AppState;
			FPL_ASSERT(appState != nullptr);
			PrepareWindowForVideoResult result = {};
			switch(videoSettings.driver) {
#				if defined(FPL_ENABLE_VIDEO_OPENGL)
				case VideoDriverType::OpenGL:
				{
#				if defined(FPL_SUBPLATFORM_X11)
#				endif
				} break;
#				endif // FPL_ENABLE_VIDEO_OPENGL

				default:
				{
				} break;
			}
			return(result);
		}

		fpl_internal bool InitVideo(const VideoDriverType driver, const VideoSettings &videoSettings, const uint32_t windowWidth, const uint32_t windowHeight, platform::PlatformAppState *appState) {
			// @NOTE(final): Video drivers are platform independent, so we cannot have to same system as audio.

			if(appState != nullptr) {
				FPL_ASSERT(appState->video.mem == nullptr);
				appState->video.memSize = sizeof(common::VideoState);
				appState->video.mem = memory::MemoryAlignedAllocate(appState->video.memSize, 16);
				if(appState->video.mem == nullptr) {
					PushError("Failed allocating video state memory of %xu bytes", appState->video.memSize);
					return false;
				}

				VideoState *videoState = (VideoState *)appState->video.mem;
				appState->currentSettings.video.driver = driver;

				// Allocate backbuffer context if needed
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				if(driver == VideoDriverType::Software) {
					fpl::video::VideoBackBuffer &backbuffer = videoState->softwareBackbuffer;
					backbuffer.width = windowWidth;
					backbuffer.height = windowHeight;
					backbuffer.pixelStride = sizeof(uint32_t);
					backbuffer.lineWidth = backbuffer.width * backbuffer.pixelStride;
					size_t size = backbuffer.lineWidth * backbuffer.height;
					backbuffer.pixels = (uint32_t *)memory::MemoryAlignedAllocate(size, 16);
					if(backbuffer.pixels == nullptr) {
						PushError("Failed allocating video software backbuffer of size %xu bytes", size);
						ReleaseVideo(appState);
						return false;
					}

					// Clear to black by default
					// @NOTE(final): Bitmap is top-down, 0xAABBGGRR
					// @TODO(final): Backbuffer endianess???
					uint32_t *p = backbuffer.pixels;
					for(uint32_t y = 0; y < backbuffer.height; ++y) {
						uint32_t color = 0xFF000000;
						for(uint32_t x = 0; x < backbuffer.width; ++x) {
							*p++ = color;
						}
					}
				}
#			endif // FPL_ENABLE_VIDEO_SOFTWARE

				switch(driver) {
#				if defined(FPL_ENABLE_VIDEO_OPENGL)
					case VideoDriverType::OpenGL:
					{
#					if defined(FPL_PLATFORM_WIN32)
						if(!drivers::Win32InitVideoOpenGL(appState->win32, appState->window.win32, videoSettings, videoState->win32.opengl)) {
							// @NOTE(final): Expect that errors are already written
							ReleaseVideo(appState);
							return false;
						}
#					endif
					} break;
#				endif // FPL_ENABLE_VIDEO_OPENGL

#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					case VideoDriverType::Software:
					{
#					if defined(FPL_PLATFORM_WIN32)
						if(!drivers::Win32InitVideoSoftware(videoState->softwareBackbuffer, videoState->win32.software)) {
							// @NOTE(final): Expect that errors are already written
							ReleaseVideo(appState);
							return false;
						}
#					endif
					} break;
#				endif // FPL_ENABLE_VIDEO_SOFTWARE

					default:
					{
						PushError("Unsupported video driver '%s' for this platform", fpl::video::GetVideoDriverString(videoSettings.driver));
						ReleaseVideo(appState);
						return false;
					} break;
				}

			}

			return true;
		}
#endif // FPL_ENABLE_VIDEO

		fpl_internal void ReleasePlatformResources(platform::PlatformInitState &initState, platform::PlatformAppState *appState) {
#		if defined(FPL_ENABLE_AUDIO)
			ReleaseAudio();
#		endif

#		if defined(FPL_ENABLE_VIDEO)
			ReleaseVideo(appState);
#		endif

#		if defined(FPL_ENABLE_WINDOW)
			ReleaseWindow(initState, appState);
#		endif

			if(appState != nullptr) {
				// Release actual platform
#			if defined(FPL_PLATFORM_WIN32)
				platform::Win32ReleasePlatform(initState, appState);
#           elif defined(FPL_PLATFORM_LINUX)
				platform::LinuxReleasePlatform(initState, appState);
#			endif

				// Release platform applicatiom state memory
				memory::MemoryAlignedFree(appState);
				platform::global__AppState = nullptr;
			}

#		if defined(FPL_ENABLE_WINDOW)
			if(platform::global__EventQueue != nullptr) {
				memory::MemoryAlignedFree(platform::global__EventQueue);
				platform::global__EventQueue = nullptr;
			}
#		endif

			initState.isInitialized = false;
		}

	} // common

#if defined(FPL_ENABLE_AUDIO)
	namespace audio {
		fpl_common_api AudioResult StopAudio() {
			using namespace common;

			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			if(platAudioState.mem == nullptr) {
				return AudioResult::Failed;
			}
			AudioState *audioState = (AudioState *)platAudioState.mem;

			if(AudioGetDeviceState(*audioState) == AudioDeviceState::Uninitialized) {
				return AudioResult::DeviceNotInitialized;
			}

			AudioResult result = AudioResult::Failed;
			MutexLock(audioState->lock);
			{
				// Check if the device is already stopped
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Stopping) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStopped;
				}
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStopped;
				}

				// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
				if(AudioGetDeviceState(*audioState) != AudioDeviceState::Started) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceBusy;
				}

				AudioSetDeviceState(*audioState, AudioDeviceState::Stopping);

				if(audioState->isAsyncDriver) {
					// Asynchronous drivers (Has their own thread)
					AudioStopDevice(*audioState);
				} else {
					// Synchronous drivers

					// The audio worker thread is most likely in a wait state, so let it stop properly.
					AudioDeviceStopMainLoop(*audioState);

					// We need to wait for the worker thread to become available for work before returning.
					// @NOTE(final): The audio worker thread will be the one who puts the device into the stopped state.
					SignalWaitForOne(audioState->lock, audioState->stopSignal);
					result = AudioResult::Success;
				}
			}
			MutexUnlock(audioState->lock);

			return result;
		}

		fpl_common_api AudioResult PlayAudio() {
			using namespace common;

			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			if(platAudioState.mem == nullptr) {
				return AudioResult::Failed;
			}
			AudioState *audioState = (AudioState *)platAudioState.mem;

			if(!IsAudioDeviceInitialized(audioState)) {
				return AudioResult::DeviceNotInitialized;
			}

			AudioResult result = AudioResult::Failed;
			MutexLock(audioState->lock);
			{
				// Be a bit more descriptive if the device is already started or is already in the process of starting.
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Starting) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStarted;
				}
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Started) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStarted;
				}

				// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
				if(AudioGetDeviceState(*audioState) != AudioDeviceState::Stopped) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceBusy;
				}

				AudioSetDeviceState(*audioState, AudioDeviceState::Starting);

				if(audioState->isAsyncDriver) {
					// Asynchronous drivers (Has their own thread)
					AudioStartDevice(*audioState);
					AudioSetDeviceState(*audioState, AudioDeviceState::Started);
				} else {
					// Synchronous drivers
					SignalSet(audioState->wakeupSignal);

					// Wait for the worker thread to finish starting the device.
					// @NOTE(final): The audio worker thread will be the one who puts the device into the started state.
					SignalWaitForOne(audioState->lock, audioState->startSignal);
					result = audioState->workResult;
				}
			}
			MutexUnlock(audioState->lock);

			return result;
		}

		fpl_common_api AudioDeviceFormat GetAudioHardwareFormat() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			if(platAudioState.mem == nullptr) {
				return {};
			}
			common::AudioState *audioState = (common::AudioState *)platAudioState.mem;
			return audioState->common.internalFormat;
		}

		fpl_common_api void SetAudioClientReadCallback(AudioClientReadFunction *newCallback, void *userData) {
			using namespace common;
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			if(platAudioState.mem == nullptr) {
				return;
			}
			common::AudioState *audioState = (common::AudioState *)platAudioState.mem;
			if(audioState->activeDriver > AudioDriverType::Auto) {
				if(AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped) {
					audioState->common.clientReadCallback = newCallback;
					audioState->common.clientUserData = userData;
				}
			}
		}

		fpl_common_api uint32_t GetAudioDevices(AudioDeviceID *devices, uint32_t maxDeviceCount) {
			using namespace common;
			if(devices == nullptr) {
				return 0;
			}
			if(maxDeviceCount == 0) {
				return 0;
			}

			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAudioState &platAudioState = platform::global__AppState->audio;
			if(platAudioState.mem == nullptr) {
				return 0;
			}
			common::AudioState *audioState = (common::AudioState *)platAudioState.mem;

			uint32_t result = 0;
			if(audioState->activeDriver > AudioDriverType::Auto) {
				switch(audioState->activeDriver) {
#				if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
					case AudioDriverType::DirectSound:
					{
						result = drivers::GetDevicesDirectSound(audioState->dsound, devices, maxDeviceCount);
					} break;
#				endif
				}
			}
			return(result);
		}
	} // audio
#endif // FPL_ENABLE_AUDIO

#if defined(FPL_ENABLE_VIDEO)
	namespace video {
		fpl_common_api VideoBackBuffer *GetVideoBackBuffer() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;

			VideoBackBuffer *result = nullptr;
			if(appState->video.mem != nullptr) {
				common::VideoState *videoState = (common::VideoState *)appState->video.mem;
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				if(appState->currentSettings.video.driver == VideoDriverType::Software) {
					result = &videoState->softwareBackbuffer;
				}
#			endif
			}

			return(result);
		}

		fpl_common_api VideoDriverType GetVideoDriver() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::PlatformAppState *appState = platform::global__AppState;
			VideoDriverType result = appState->currentSettings.video.driver;
			return(result);
		}

		fpl_common_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
			FPL_ASSERT(platform::global__AppState != nullptr);
			platform::PlatformAppState *appState = platform::global__AppState;
			platform::PlatformVideoState &video = appState->video;

			bool result = false;

			if(video.mem != nullptr) {
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				if(appState->currentSettings.video.driver == VideoDriverType::Software) {
					common::ReleaseVideo(appState);
					result = common::InitVideo(VideoDriverType::Software, appState->currentSettings.video, width, height, appState);
				}
#			endif
			}

			return (result);
		}

		fpl_common_api void VideoFlip() {
			FPL_ASSERT(platform::global__AppState != nullptr);
			const platform::PlatformAppState *appState = platform::global__AppState;
			const platform::PlatformVideoState &video = appState->video;

			if(video.mem != nullptr) {
				const common::VideoState *videoState = (common::VideoState *)video.mem;

#		if defined(FPL_PLATFORM_WIN32)
				const platform::Win32AppState &win32AppState = appState->win32;
				const platform::Win32WindowState &win32WindowState = appState->window.win32;
				const platform::Win32APIFunctions &wapi = win32AppState.winApi;
				switch(appState->currentSettings.video.driver) {
#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					case VideoDriverType::Software:
					{
						const drivers::Win32VideoSoftwareState &software = videoState->win32.software;
						const video::VideoBackBuffer &backbuffer = videoState->softwareBackbuffer;
						window::WindowSize area = window::GetWindowArea();
						int32_t targetX = 0;
						int32_t targetY = 0;
						int32_t targetWidth = area.width;
						int32_t targetHeight = area.height;
						int32_t sourceWidth = backbuffer.width;
						int32_t sourceHeight = backbuffer.height;
						if(backbuffer.useOutputRect) {
							targetX = backbuffer.outputRect.x;
							targetY = backbuffer.outputRect.y;
							targetWidth = backbuffer.outputRect.width;
							targetHeight = backbuffer.outputRect.height;
							wapi.gdi.stretchDIBits(win32WindowState.deviceContext, 0, 0, area.width, area.height, 0, 0, 0, 0, nullptr, nullptr, DIB_RGB_COLORS, BLACKNESS);
						}
						wapi.gdi.stretchDIBits(win32WindowState.deviceContext, targetX, targetY, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, backbuffer.pixels, &software.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
					} break;
#				endif

#				if defined(FPL_ENABLE_VIDEO_OPENGL)
					case VideoDriverType::OpenGL:
					{
						wapi.gdi.swapBuffers(win32WindowState.deviceContext);
					} break;
#				endif

					default:
						break;
				}
#			endif // FPL_PLATFORM_WIN32
			}
		}
	} // video
#endif // FPL_ENABLE_VIDEO

	fpl_common_api const char *GetPlatformError() {
		const char *result = "";
		const common::ErrorState &errorState = common::global__LastErrorState;
#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		if(errorState.count > 0) {
			size_t index = errorState.count - 1;
			result = GetPlatformError(index);
		}
#	else
		result = errorState.errors[0];
#	endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
		return (result);
	}

	fpl_common_api const char *GetPlatformError(const size_t index) {
		const char *result = "";
		const common::ErrorState &errorState = common::global__LastErrorState;
#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		if(index > -1 && index < (int32_t)errorState.count) {
			result = errorState.errors[index];
		} else {
			result = errorState.errors[errorState.count - 1];
		}
#	else
		result = errorState.errors[0];
#	endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
		return (result);
	}

	fpl_common_api size_t GetPlatformErrorCount() {
		size_t result = 0;
		const common::ErrorState &errorState = common::global__LastErrorState;
		result = errorState.count;
		return (result);
	}

	fpl_common_api void ClearPlatformErrors() {
		common::ErrorState &errorState = common::global__LastErrorState;
		memory::MemoryClear(&errorState, sizeof(errorState));
	}

	fpl_common_api const Settings &GetCurrentSettings() {
		FPL_ASSERT(platform::global__AppState != nullptr);
		const platform::PlatformAppState *appState = platform::global__AppState;
		return (appState->currentSettings);
	}

	fpl_common_api void ReleasePlatform() {
		// Exit out if platform is not initialized
		platform::PlatformInitState &initState = platform::global__InitState;
		if(!initState.isInitialized) {
			common::PushError("Platform is not initialized");
			return;
		}
		platform::PlatformAppState *appState = platform::global__AppState;
		common::ReleasePlatformResources(initState, appState);
	}

	fpl_common_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings) {
		// Exit out if platform is already initialized
		if(platform::global__InitState.isInitialized) {
			common::PushError("Platform is already initialized");
			return false;
		}

		// Allocate platform app state memory
		FPL_ASSERT(platform::global__AppState == nullptr);
		size_t platformAppStateSize = sizeof(platform::PlatformAppState);
		void *platformAppStateMemory = memory::MemoryAlignedAllocate(platformAppStateSize, 16);
		if(platformAppStateMemory == nullptr) {
			common::PushError("Failed allocating app state memory of size '%zu'", platformAppStateSize);
			return false;
		}

		// Setup platform app state
		platform::PlatformAppState *appState = platform::global__AppState = (platform::PlatformAppState *)platformAppStateMemory;
		appState->initSettings = initSettings;
		appState->initFlags = initFlags;
		appState->currentSettings = initSettings;

		// Reset platform init state
		platform::PlatformInitState &initState = platform::global__InitState;
		initState = {};

#	if defined(FPL_ENABLE_VIDEO) && defined(FPL_ENABLE_WINDOW)
		// Window is required for video always
		if(appState->initFlags & InitFlags::Video) {
			appState->initFlags |= InitFlags::Window;
		}
#	endif

		// Initialize the actual platform
		bool isInitialized = false;
#	if defined(FPL_PLATFORM_WIN32)
		isInitialized = platform::Win32InitPlatform(appState->initFlags, initSettings, initState, appState);
#   elif defined(FPL_PLATFORM_LINUX)
		isInitialized = platform::LinuxInitPlatform(appState->initFlags, initSettings, initState, appState);
#	endif

		if(isInitialized) {

			// Init Window
#		if defined(FPL_ENABLE_WINDOW)
			if(appState->initFlags & InitFlags::Window) {
				// Allocate event queue
				size_t eventQueueMemorySize = sizeof(platform::EventQueue);
				void *eventQueueMemory = memory::MemoryAlignedAllocate(eventQueueMemorySize, 16);
				if(eventQueueMemory == nullptr) {
					common::PushError("Failed Allocating Event Queue Memory with size '%zu'", eventQueueMemorySize);
					common::ReleasePlatformResources(initState, appState);
					return false;
				}
				platform::global__EventQueue = (platform::EventQueue *)eventQueueMemory;
				if(!common::InitWindow(initSettings, appState->currentSettings.window, appState, appState->currentSettings.windowForVideo, common::PrepareWindowForVideoAfter)) {
					common::PushError("Failed initialization window");
					common::ReleasePlatformResources(initState, appState);
					return false;
				}
			}
#		endif // FPL_ENABLE_WINDOW

			// Init Video
#		if defined(FPL_ENABLE_VIDEO)
			if(appState->initFlags & InitFlags::Video) {
				uint32_t windowWidth, windowHeight;
				if(appState->currentSettings.window.isFullscreen) {
					windowWidth = appState->currentSettings.window.fullscreenWidth;
					windowHeight = appState->currentSettings.window.fullscreenHeight;
				} else {
					windowWidth = appState->currentSettings.window.windowWidth;
					windowHeight = appState->currentSettings.window.windowWidth;
				}
				if(!common::InitVideo(initSettings.video.driver, initSettings.video, windowWidth, windowHeight, appState)) {
					common::PushError("Failed initialization video with settings (Driver=%s, Width=%d, Height=%d)", video::GetVideoDriverString(initSettings.video.driver), windowWidth, windowHeight);
					common::ReleasePlatformResources(initState, appState);
					return false;
				}
			}
#		endif // FPL_ENABLE_VIDEO

			// Init Audio
#		if defined(FPL_ENABLE_AUDIO)
			if(appState->initFlags & InitFlags::Audio) {
				if(common::InitAudio(initSettings.audio) != audio::AudioResult::Success) {
					common::PushError("Failed initialization audio with settings (Driver=%s, Format=%s, SampleRate=%d, Channels=%d, BufferSize=%d)", audio::GetAudioDriverString(initSettings.audio.driver), audio::GetAudioFormatString(initSettings.audio.deviceFormat.type), initSettings.audio.deviceFormat.sampleRate, initSettings.audio.deviceFormat.channels);
					common::ReleasePlatformResources(initState, appState);
					return false;
				}
			}
#		endif // FPL_ENABLE_AUDIO

			initState.isInitialized = true;
			return true;
		} else {
			common::ReleasePlatformResources(initState, appState);
			return false;
		}
	}

} // fpl

#endif // FPL_IMPLEMENTATION && !FPL_IMPLEMENTED
