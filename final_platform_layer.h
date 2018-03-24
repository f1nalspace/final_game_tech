/*
final_platform_layer.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A Open source C single file header platform abstraction layer library.

Final Platform Layer is a cross-platform single-header-file development library designed to abstract the underlying platform to a very simple and easy to use low-level api for accessing input devices (keyboard, mouse, gamepad), audio playback, window handling, IO handling (files, directories, paths), multithreading (threads, mutex, signals) and graphics software or hardware rendering initialization.
The main focus is game/simulation development, so the default settings will create a window, setup a opengl rendering context and initialize audio playback on any platform.
The only dependencies are built-in operating system libraries, a C99 compiler and the C runtime library.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into any C/C++ projects you want and include it in any place you want.
- In your main translation unit provide the typical main() entry point.
- Define FPL_IMPLEMENTATION before including this header file in the translation units you want  the source to be compiled in.
- Init the platform using fplPlatformInit()
- Use the features you want.
- Release the platform when you are done using fplPlatformRelease().

-------------------------------------------------------------------------------
	Usage: Hello world console application
-------------------------------------------------------------------------------

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args){
	if (fplPlatformInit(fplInitFlags_None)) {
		fplConsoleOut("Hello World!");
		fplPlatformRelease();
		return 0;
	} else {
		return -1;
	}
}

-------------------------------------------------------------------------------
	Usage: OpenGL legacy or modern application
-------------------------------------------------------------------------------

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args){
	fplSettings settings = fplDefaultSettings();
	fplVideoSettings videoSettings = settings.video;

	videoSettings.driver = fplVideoDriverType_OpenGL;

	// Legacy OpenGL
	videoSettings.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;

	// or

	// Modern OpenGL
	videoSettings.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	videoSettings.opengl.majorVersion = 3;
	videoSettings.opengl.minorVersion = 3;
FPL_LOG_FUNCTION
	if (fplPlatformInit(fplInitFlags_Video, settings)) {
		// Event/Main loop
		while (fplWindowUpdate()) {
			// Handle actual window events
			fplEvent ev;
			while (fplPollWindowEvent(ev)) {
				/// ...
			}

			// your code goes here

			fplVideoFlip();
		}
		fplPlatformRelease();
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
	\file final_platform_layer.h
	\version v0.7.0.0 beta
	\author Torsten Spaete
	\brief Final Platform Layer (FPL) - A Open source C single file header platform abstraction layer library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.7.0.0 beta:
	- Changed: Switched to C99
	- Changed: Changed Init/app state structs a lot
	- Changed: Removed most anonymous unions and structs
	- Changed: Renamed a lot of the internal handles
	- Changed: Renamed ThreadContext to ThreadHandle
	- Changed: Renamed ThreadSignal to SignalHandle
	- Changed: Renamed ThreadMutex to MutexHandle
	- Changed: All structs, functions with name *API renamed to *Api
	- Changed: Moved internal functions around and restructured things
	- Changed: Moved platforms and subplatforms into its own namespace
	- Changed: Updated documentation a bit
	- Changed: Introduced common::Argument*Error functions
	- Changed: Removed MemoryStackAllocate()
	- Changed: App state memory size is aligned by 16-bytes as well
	- Changed: Video/Audio state memory block is included in app state memory as well
	- Fixed: Get rid of const char* to char* warnings for Get*DriverString() functions
	- Fixed: [Win32] Icon was not default application icon
	- Fixed: paths::ExtractFileName and paths::ExtractFileExtension was not returning const char*
	- New: [X11][Window] Experimental X11 window creation and handling
	- New: [X11][OpenGL] Experimental X11/GLX video driver
	- New: [POSIX] Implemented all remaining posix functions
	- New: Introduced debug logging
	- New: Added FPL_ALIGNMENT_OFFSET macro
	- New: Added FPL_ALIGNED_SIZE macro

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

	\section section_todo_required In process

	- Linux Platform:
		- Files & Path (Copy, File/Dir iteration)
		- Window (X11)
		- Video opengl (GLX)
		- Video software (X11)

	\section section_todo_planned Planned

	- BSD Platform (POSIX, X11)

	- Mac OSX Platform

	- Audio:
		- Support for channel mapping
		- ALSA audio driver
		- WASAPI audio driver

	- Video:
		- Direct2D
		- Direct3D 9/10/11
		- Vulkan

	- Networking (UDP, TCP)
		- [Win32] WinSock
		- [POSIX] Socket

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

	- Unicode-Support for commandline arguments (Win32)

	- Window
		- Custom icon
		- Custom window style (Border, Resizeable, etc.)
		- Unicode/UTF-8 Support for character input

	\section section_todo_optional Optional

	- Make VideoBackBuffer be readonly and separate functions for getting the data pointer and setting the output rectangle.

	- Additional parameters for passing pointers instead of returning structs (Method overloading)

	- Pack/Unpack functions (Handle endianess)

	- Open/Save file/folder dialog
*/

// ****************************************************************************
//
// > HEADER
//
// ****************************************************************************
#ifndef FPL_INCLUDE_H
#define FPL_INCLUDE_H

// C99 detection
#if defined(__cplusplus) || (defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1900)
#	define FPL_IS_CPP
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && _MSC_VER >= 1900)
#	define FPL_IS_C99
#else
#	error "This C/C++ compiler is not supported!"
#endif

//
// Platform detection
//
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
#if defined(_WIN32) || defined(_WIN64)
#	define FPL_PLATFORM_WIN32
#	define FPL_PLATFORM_NAME "Windows"
#	define FPL_SUBPLATFORM_STD_CONSOLE
#elif defined(__linux__) || defined(__gnu_linux__)
#	define FPL_PLATFORM_LINUX
#	define FPL_PLATFORM_NAME "Linux"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__bsdi__)
#	define FPL_PLATFORM_BSD
#	define FPL_PLATFORM_NAME "BSD"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
#	error "Not implemented yet!"
#elif defined(unix) || defined(__unix) || defined(__unix__)
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "Unix"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
#	error "Not implemented yet!"
#else
#	error "This platform is not supported!"
#endif // FPL_PLATFORM

//
// @TEMPORARY(final): Simulate all platforms so we can compile everything properly
//
#if 0
#if !defined(FPL_PLATFORM_WIN32)
#	define FPL_PLATFORM_WIN32
#endif
#if !defined(FPL_PLATFORM_LINUX)
#	define FPL_PLATFORM_LINUX
#endif
#if !defined(FPL_SUBPLATFORM_POSIX)
#	define FPL_SUBPLATFORM_POSIX
#	define RTLD_NOW 0x10
typedef int pthread_t;
typedef int pthread_mutex_t;
typedef int pthread_cond_t;
typedef int pthread_attr_t;
typedef int pthread_mutexattr_t;
typedef int pthread_condattr_t;
#endif
#if !defined(FPL_SUBPLATFORM_X11)
#	define FPL_SUBPLATFORM_X11
typedef int Display;
typedef int Window;
typedef int Visual;
typedef int Colormap;
typedef int XSetWindowAttributes;
#endif
#if !defined(FPL_COMPILER_GCC)
#	define FPL_COMPILER_GCC 1
#endif
#endif

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
#elif defined(__GNUC__) && !defined(__clang__)
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

	//! Function name macro (Win32)
#   define FPL_FUNCTION_NAME __FUNCTION__
#else
	// @NOTE(final): Expect all other compilers to pass in FPL_DEBUG manually
#	if defined(FPL_DEBUG)
		//! Debug mode detected
#		define FPL_ENABLE_DEBUG
#	else
		//! Non-debug (Release) mode detected
#		define FPL_ENABLE_RELEASE
#	endif

	//! Function name macro (Other compilers)
#   define FPL_FUNCTION_NAME __FUNCTION__
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
#	if defined(FPL_SUBPLATFORM_X11)
#		undef FPL_SUBPLATFORM_X11
#	endif

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
#define fpl_internal_inline inline
//! Null
#define fpl_null '\0'

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
// Logging
//
#if defined(FPL_LOGGING)
	//! Enable logging
#   define FPL_ENABLE_LOGGING
#endif

//
// Types & Limits
//
#include <stdint.h> // uint32_t, ...
#include <stddef.h> // size_t
#include <stdlib.h> // UINT32_MAX, ...
#include <stdbool.h> // bool

//
// Macro functions
//
//! Macro for initialize a struct to zero
#if defined(FPL_IS_C99)
#	define FPL_ZERO_INIT {0}
#else
#	define FPL_ZERO_INIT {}
#endif

//! Returns the element count from a static array,
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//! Returns the offset in bytes to a field in a structure
#define FPL_OFFSETOF(type, field) ((size_t)(&(((type*)(0))->field)))

//! Returns the offset for the value to satisfy the given alignment boundary
#define FPL_ALIGNMENT_OFFSET(value, alignment) ( (((alignment) > 1) && (((value) & ((alignment) - 1)) != 0)) ? ((alignment) - ((value) & (alignment - 1))) : 0)           
//! Returns the given size extended o to satisfy the given alignment boundary
#define FPL_ALIGNED_SIZE(size, alignment) (((size) > 0 && (alignment) > 0) ? ((size) + FPL_ALIGNMENT_OFFSET(size, alignment)) : (size))
//! Returns true when the given pointer address is aligned to the given alignment
#define FPL_IS_ALIGNED(ptr, alignment) (((uintptr_t)(const void *)(ptr)) % (alignment) == 0)

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

//! Manually allocate memory on the stack (Use this with care!)
#define FPL_STACKALLOCATE(size) _alloca(size)

#if defined(__cplusplus)
	//! Macro for overloading enum operators in C++
#	define FPL_ENUM_AS_FLAGS_OPERATORS(etype) \
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
#else
//! No need to overload operators for enums in C99
#define FPL_ENUM_AS_FLAGS_OPERATORS(etype)
#endif

// ****************************************************************************
// ****************************************************************************
//
// Platform includes
//
// ****************************************************************************
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
#	ifdef __cplusplus
#		define fpl__Win32IsEqualGuid(a, b) IsEqualGUID(a, b)
#	else
#		define fpl__Win32IsEqualGuid(a, b) IsEqualGUID(&a, &b)
#	endif
#endif // FPL_PLATFORM_WIN32

#if defined(FPL_SUBPLATFORM_POSIX)
//#	include <pthread.h> // pthread_t, pthread_mutex_, pthread_cond_, pthread_barrier_
#endif // FPL_SUBPLATFORM_POSIX

#if defined(FPL_SUBPLATFORM_X11)
//#   include <X11/X.h> // Window
//#   include <X11/Xlib.h> // Display
#   undef None
#   undef Success
#endif // FPL_SUBPLATFORM_X11

// ****************************************************************************
// ****************************************************************************
//
// API Declaration
//
// ****************************************************************************
// ****************************************************************************

// ----------------------------------------------------------------------------
/**
  * \defgroup Atomics Atomic functions
  * \brief Atomic functions, like AtomicCompareAndExchange, AtomicReadFence, etc.
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Insert a memory read fence/barrier.
  *
  * This will complete previous reads before future reads and prevents the compiler from reordering memory reads across this fence.
  */
fpl_platform_api void fplAtomicReadFence();
/**
  * \brief Insert a memory write fence/barrier.
  * This will complete previous writes before future writes and prevents the compiler from reordering memory writes across this fence.
  */
fpl_platform_api void fplAtomicWriteFence();
/**
  * \brief Insert a memory read/write fence/barrier.
  * This will complete previous reads/writes before future reads/writes and prevents the compiler from reordering memory access across this fence.
  */
fpl_platform_api void fplAtomicReadWriteFence();

/**
  * \brief Replace a 32-bit unsigned integer with the given value atomically.
  * Ensures that memory operations are completed in order.
  * \param target The target value to write into.
  * \param value The source value used for exchange.
  * \return Returns the initial value before the replacement.
  */
fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
/**
  * \brief Replace a 64-bit unsigned integer with the given value atomically.
  * Ensures that memory operations are completed in order.
  * \param target The target value to write into.
  * \param value The source value used for exchange.
  * \return Returns the initial value before the replacement.
  */
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
/**
  * \brief Replace a 32-bit signed integer with the given value atomically.
  * Ensures that memory operations are completed in order.
  * \param target The target value to write into.
  * \param value The source value used for exchange.
  * \return Returns the initial value before the replacement.
  */
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value);
/**
  * \brief Replace a 64-bit signed integer with the given value atomically.
  * Ensures that memory operations are completed in order.
  * \param target The target value to write into.
  * \param value The source value used for exchange.
  * \return Returns the initial value before the replacement.
  */
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value);
/**
  * \brief Replace a pointer with the given value atomically.
  * Ensures that memory operations are completed in order.
  * \param target The target value to write into.
  * \param value The source value used for exchange.
  * \return Returns the initial value before the replacement.
  */
fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value);

/**
  * \brief Adds a 32-bit unsigned integer to the value by the given addend atomically.
  * Ensures that memory operations are completed in order.
  * \param value The target value to append to.
  * \param addend The value used for adding.
  * \return Returns the initial value before the append.
  */
fpl_platform_api uint32_t fplAtomicAddU32(volatile uint32_t *value, const uint32_t addend);
/**
  * \brief Adds a 64-bit unsigned integer to the value by the given addend atomically.
  * Ensures that memory operations are completed in order.
  * \param value The target value to append to.
  * \param addend The value used for adding.
  * \return Returns the initial value before the append.
  */
fpl_platform_api uint64_t fplAtomicAddU64(volatile uint64_t *value, const uint64_t addend);
/**
  * \brief Adds a 32-bit signed integer to the value by the given addend atomically.
  * Ensures that memory operations are completed in order.
  * \param value The target value to append to.
  * \param addend The value used for adding.
  * \return Returns the initial value before the append.
  */
fpl_platform_api int32_t fplAtomicAddS32(volatile int32_t *value, const int32_t addend);
/**
  * \brief Adds a 64-bit signed integer to the value by the given addend atomically.
  * Ensures that memory operations are completed in order.
  * \param value The target value to append to.
  * \param addend The value used for adding.
  * \return Returns the initial value before the append.
  */
fpl_platform_api int64_t fplAtomicAddS64(volatile int64_t *value, const int64_t addend);

/**
  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \note Use \ref fplIsAtomicCompareAndExchangeU32() when you want to check if the exchange has happened or not.
  * \return Returns the dest before the exchange, regardless of the result.
  */
fpl_platform_api uint32_t fplAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \note Use \ref fplIsAtomicCompareAndExchangeU64() when you want to check if the exchange has happened or not.
  * \return Returns the value of the destination before the exchange, regardless of the result.
  */
fpl_platform_api uint64_t fplAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \note Use \ref fplIsAtomicCompareAndExchangeS32() when you want to check if the exchange has happened or not.
  * \return Returns the value of the destination before the exchange, regardless of the result.
  */
fpl_platform_api int32_t fplAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \note Use \ref fplIsAtomicCompareAndExchangeS64() when you want to check if the exchange has happened or not.
  * \return Returns the value of the destination before the exchange, regardless of the result.
  */
fpl_platform_api int64_t fplAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \note Use \ref fplIsAtomicCompareAndExchangePtr() when you want to check if the exchange has happened or not.
  * \return Returns the value of the destination before the exchange, regardless of the result.
  */
fpl_common_api void *fplAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

/**
  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \return Returns true when the exchange happened, otherwise false.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \return Returns true when the exchange happened, otherwise false.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \return Returns true when the exchange happened, otherwise false.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \return Returns true when the exchange happened, otherwise false.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * Ensures that memory operations are completed in order.
  * \param dest The target value to write into.
  * \param comparand The value to compare with.
  * \param exchange The value to exchange with.
  * \return Returns true when the exchange happened, otherwise false.
  */
fpl_common_api bool fplIsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

/**
  * \brief Loads the 32-bit unsigned value atomically and returns the value.
  * Ensures that memory operations are completed before the read.
  * \param source The source value to read from.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \return Returns the source value.
  */
fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source);
/**
  * \brief Loads the 64-bit unsigned value atomically and returns the value.
  * Ensures that memory operations are completed before the read.
  * \param source The source value to read from.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \return Returns the source value.
  */
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source);
/**
  * \brief Loads the 32-bit signed value atomically and returns the value.
  * Ensures that memory operations are completed before the read.
  * \param source The source value to read from.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \return Returns the source value.
  */
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source);
/**
  * \brief Loads the 64-bit signed value atomically and returns the value.
  * Ensures that memory operations are completed before the read.
  * \param source The source value to read from.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \return Returns the source value.
  */
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source);
/**
  * \brief Loads the pointer value atomically and returns the value.
  * Ensures that memory operations are completed before the read.
  * \param source The source value to read from.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \return Returns the source value.
  */
fpl_common_api void *fplAtomicLoadPtr(volatile void **source);

/**
  * \brief Overwrites the 32-bit unsigned value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  * \return Returns the source value.
  */
fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
/**
  * \brief Overwrites the 64-bit unsigned value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  * \return Returns the source value.
  */
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
/**
  * \brief Overwrites the 32-bit signed value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  * \return Returns the source value.
  */
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value);
/**
  * \brief Overwrites the 64-bit signed value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  * \return Returns the source value.
  */
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value);
/**
  * \brief Overwrites the pointer value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  * \return Returns the source value.
  */
fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value);

/** \}*/

/**
  * \defgroup Hardware Hardware functions
  * \brief Hardware functions, like GetProcessorCoreCount, GetProcessorName, etc.
  * \{
  */

//! Memory informations
typedef struct fplMemoryInfos {
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
} fplMemoryInfos;

/**
  * \brief Returns the total number of processor cores.
  * \return Number of processor cores.
  */
fpl_platform_api uint32_t fplGetProcessorCoreCount();
/**
  * \brief Returns the name of the processor.
  * The processor name is written in the destination buffer.
  * \param destBuffer The character buffer to write the processor name into.
  * \param maxDestBufferLen The total number of characters available in the destination character buffer.
  * \return Name of the processor.
  */
fpl_platform_api char *fplGetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen);
/**
  * \brief Returns the current system memory informations.
  * \return Current system memory informations.
  */
fpl_platform_api fplMemoryInfos fplGetSystemMemoryInfos();

/** \}*/

/**
  * \defgroup Settings Settings and configurations
  * \brief Video/audio/window settings
  * \{
  */

//! Initialization flags (Window, Video, etc.)
typedef enum fplInitFlags {
	//! No init flags
	fplInitFlags_None = 0,
	//! Create a single window
	fplInitFlags_Window = 1 << 0,
	//! Use a video backbuffer (This flag ensures that \ref fplInitFlags_Window is included always)
	fplInitFlags_Video = 1 << 1,
	//! Use asyncronous audio playback
	fplInitFlags_Audio = 1 << 2,
	//! Default init flags for initializing everything
	fplInitFlags_All = fplInitFlags_Window | fplInitFlags_Video | fplInitFlags_Audio
} fplInitFlags;
//! InitFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplInitFlags);

//! Video driver type
typedef enum fplVideoDriverType {
	//! No video driver
	fplVideoDriverType_None = 0,
	//! OpenGL
	fplVideoDriverType_OpenGL,
	//! Software
	fplVideoDriverType_Software
} fplVideoDriverType;

#if defined(FPL_ENABLE_VIDEO_OPENGL)
//! OpenGL compability flags
typedef enum fplOpenGLCompabilityFlags {
	//! Use legacy context
	fplOpenGLCompabilityFlags_Legacy = 0,
	//! Use core profile
	fplOpenGLCompabilityFlags_Core = 1 << 1,
	//! Use compability profile
	fplOpenGLCompabilityFlags_Compability = 1 << 2,
	//! Remove features marked as deprecated
	fplOpenGLCompabilityFlags_Forward = 1 << 3,
} fplOpenGLCompabilityFlags;

//! OpenGL video settings container
typedef struct fplOpenGLVideoSettings {
	//! Compability flags
	fplOpenGLCompabilityFlags compabilityFlags;
	//! Desired major version
	uint32_t majorVersion;
	//! Desired minor version
	uint32_t minorVersion;
} fplOpenGLVideoSettings;
#endif // FPL_ENABLE_VIDEO_OPENGL

	//! Graphics API settings union
typedef union fplGraphicsAPISettings {
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	//! OpenGL settings
	fplOpenGLVideoSettings opengl;
#endif
	//! Dummy field when no graphics drivers are available
	int dummy;
} fplGraphicsAPISettings;

//! Video settings container (Driver, Flags, Version, VSync, etc.)
typedef struct fplVideoSettings {
	//! Video driver type
	fplVideoDriverType driver;
	//! Vertical syncronisation enabled/disabled
	bool isVSync;
	//! Backbuffer size is automatically resized. Useable only for software rendering!
	bool isAutoSize;
	//! Graphics API settings
	fplGraphicsAPISettings graphics;
} fplVideoSettings;

/**
  * \brief Make default video settings
  * \note This will not change any video settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_inline fplVideoSettings fplDefaultVideoSettings() {
	fplVideoSettings result = FPL_ZERO_INIT;

	result.isVSync = false;
	result.isAutoSize = true;

	// @NOTE(final): Auto detect video driver
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	result.driver = fplVideoDriverType_OpenGL;
	result.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#elif defined(FPL_ENABLE_VIDEO_SOFTWARE)
	result.driver = fplVideoDriverType_Software;
#else
	result.driver = fplVideoDriverType_None;
#endif

	return(result);
}

//! Audio driver type
typedef enum fplAudioDriverType {
	//! No audio driver
	fplAudioDriverType_None = 0,
	//! Auto detection
	fplAudioDriverType_Auto,
	//! DirectSound
	fplAudioDriverType_DirectSound,
} fplAudioDriverType;

//! Audio format type
typedef enum fplAudioFormatType {
	// No audio format
	fplAudioFormatType_None = 0,
	// Unsigned 8-bit integer PCM
	fplAudioFormatType_U8,
	// Signed 16-bit integer PCM
	fplAudioFormatType_S16,
	// Signed 24-bit integer PCM
	fplAudioFormatType_S24,
	// Signed 32-bit integer PCM
	fplAudioFormatType_S32,
	// Signed 64-bit integer PCM
	fplAudioFormatType_S64,
	// 32-bit IEEE_FLOAT
	fplAudioFormatType_F32,
	// 64-bit IEEE_FLOAT
	fplAudioFormatType_F64,
} fplAudioFormatType;

//! Audio device format
typedef struct fplAudioDeviceFormat {
	//! Audio format
	fplAudioFormatType type;
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
} fplAudioDeviceFormat;

//! Audio device id union
typedef union fplAudioDeviceID {
#if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
	//! DirectShow Device GUID
	GUID dshow;
#endif
	//! Dummy field (When no drivers are available)
	int dummy;
} fplAudioDeviceID;

//! Audio device info
typedef struct fplAudioDeviceInfo {
	//! Device name
	char name[256];
	//! Device id
	fplAudioDeviceID id;
} fplAudioDeviceInfo;

//! Audio Client Read Callback Function
typedef uint32_t(fpl_audio_client_read_callback)(const fplAudioDeviceFormat *deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

//! Audio settings
typedef struct fplAudioSettings {
	//! The device format
	fplAudioDeviceFormat deviceFormat;
	//! The device info
	fplAudioDeviceInfo deviceInfo;
	//! The callback for retrieving audio data from the client
	fpl_audio_client_read_callback *clientReadCallback;
	//! The targeted driver
	fplAudioDriverType driver;
	//! Audio buffer in milliseconds
	uint32_t bufferSizeInMilliSeconds;
	//! Is exclude mode prefered
	bool preferExclusiveMode;
	//! User data pointer for client read callback
	void *userData;
} fplAudioSettings;

/**
  * \brief Make default audio settings (S16 PCM, 48 KHz, 2 Channels)
  * \note This will not change any audio settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_inline fplAudioSettings fplDefaultAudioSettings() {
	fplAudioSettings result = FPL_ZERO_INIT;
	result.bufferSizeInMilliSeconds = 25;
	result.preferExclusiveMode = false;
	result.deviceFormat.channels = 2;
	result.deviceFormat.sampleRate = 48000;
	result.deviceFormat.type = fplAudioFormatType_S16;

	result.driver = fplAudioDriverType_None;
#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
	result.driver = fplAudioDriverType_DirectSound;
#	endif
	return(result);
}

//! Window settings (Size, Title etc.)
typedef struct fplWindowSettings {
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
} fplWindowSettings;

/**
  * \brief Make default settings for the window
  * \note This will not change any window settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_inline fplWindowSettings fplDefaultWindowSettings() {
	fplWindowSettings result = FPL_ZERO_INIT;
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
typedef struct fplInputSettings {
	//! Frequency in ms for detecting new or removed controllers (Default: 100 ms)
	uint32_t controllerDetectionFrequency;
} fplInputSettings;

/**
  * \brief Make default settings for input devices.
  * \note This will not change any input settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_inline fplInputSettings fplDefaultInputSettings() {
	fplInputSettings result = FPL_ZERO_INIT;
	result.controllerDetectionFrequency = 100;
	return(result);
}

//! Settings container (Window, Video, etc)
typedef struct fplSettings {
	//! Window settings
	fplWindowSettings window;
	//! Video settings
	fplVideoSettings video;
	//! Audio settings
	fplAudioSettings audio;
	//! Input settings
	fplInputSettings input;
} fplSettings;

/**
  * \brief Make default settings for window, video, audio, etc.
  * \note This will not change any settings! To change the actual settings you have to pass this settings container to a argument in \ref fplPlatformInit().
  */
fpl_inline fplSettings fplDefaultSettings() {
	fplSettings result = FPL_ZERO_INIT;
	result.window = fplDefaultWindowSettings();
	result.video = fplDefaultVideoSettings();
	result.audio = fplDefaultAudioSettings();
	result.input = fplDefaultInputSettings();
	return(result);
}

/**
  * \brief Returns the current settings
  */
fpl_common_api const fplSettings *fplGetCurrentSettings();

/** \}*/

/**
  * \defgroup Initialization Initialization functions
  * \brief Initialization and release functions
  * \{
  */

/**
  * \brief Initializes the platform layer.
  * \param initFlags Optional init flags used for enable certain features, like video/audio etc. (Default: \ref fplInitFlags_All)
  * \param initSettings Optional initialization settings which can be passed to control the platform layer behavior or systems. (Default: \ref fplSettings provided by \ref fplDefaultSettings())
  * \note \ref fplPlatformRelease() must be called when you are done! After \ref fplPlatformRelease() has been called you can call this function again if needed.
  * \return Returns true when the initialzation was successful, otherwise false. Will return false when the platform layers is already initialized successfully.
  */
fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings);
/**
  * \brief Releases the resources allocated by the platform layer.
  * \note Can only be called when \ref fplPlatformInit() was successful.
  */
fpl_common_api void fplPlatformRelease();

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
fpl_common_api const char *fplGetPlatformError();
/**
  * \brief Returns the last error string from the given index
  * \param index The index
  * \note This function can be called regardless of the initialization state!
  * \return Last error string from the given index or empty when there was no error.
  */
fpl_common_api const char *fplGetPlatformErrorFromIndex(const size_t index);
/**
  * \brief Returns the count of total last errors
  * \note This function can be called regardless of the initialization state!
  * \return Number of last errors or zero when there was no error.
  */
fpl_common_api size_t fplGetPlatformErrorCount();
/**
  * \brief Clears all the current errors in the platform
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api void fplClearPlatformErrors();

/** \}*/

/**
  * \defgroup DynamicLibrary Dynamic library loading
  * \brief Loading dynamic libraries and retrieving the procedure addresses.
  * \{
  */

//! Internal library handle union
typedef union fplInternalDynamicLibraryHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 library handle
	HMODULE win32LibraryHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix library handle
	void *posixLibraryHandle;
#endif
} fplInternalDynamicLibraryHandle;

  //! Handle to a loaded dynamic library
typedef struct fplDynamicLibraryHandle {
	//! Internal library handle
	fplInternalDynamicLibraryHandle internalHandle;
	//! Library opened successfully
	bool isValid;
} fplDynamicLibraryHandle;

/**
  * \brief Loads a dynamic library and returns the loaded handle for it.
  * \param libraryFilePath The path to the library with included file extension (.dll / .so)
  * \note To check for success, just check the DynamicLibraryHandle.isValid field from the result.
  * \return Handle container of the loaded library.
  */
fpl_platform_api fplDynamicLibraryHandle fplDynamicLibraryLoad(const char *libraryFilePath);
/**
  * \brief Returns the dynamic library procedure address for the given procedure name.
  * \param handle Handle to the loaded library
  * \param name Name of the procedure
  * \return Procedure address for the given procedure name or fpl_null when procedure not found or library is not loaded.
  */
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name);
/**
  * \brief Unloads the loaded library and resets the handle to zero.
  * \param handle Loaded dynamic library handle
  */
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle);

/** \}*/

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
fpl_platform_api void fplConsoleOut(const char *text);
/**
  * \brief Writes the given formatted text to the standard output console buffer.
  * \param format The format used for writing into the standard output console.
  * \param ... The dynamic arguments used for formatting the text.
  * \note This is most likely just a wrapper call to vfprintf(stdout)
  */
fpl_platform_api void fplConsoleFormatOut(const char *format, ...);
/**
  * \brief Writes the given text to the standard error console buffer.
  * \param text The text to write into standard error console.
  * \note This is most likely just a wrapper call to fprintf(stderr)
  */
fpl_platform_api void fplConsoleError(const char *text);
/**
  * \brief Writes the given formatted text to the standard error console buffer.
  * \param format The format used for writing into the standard error console.
  * \param ... The dynamic arguments used for formatting the text.
  * \note This is most likely just a wrapper call to vfprintf(stderr)
  */
fpl_platform_api void fplConsoleFormatError(const char *format, ...);
/**
  * \brief Wait for a character to be typed in the console input and return it
  * \note This is most likely just a wrapper call to getchar()
  * \return Character typed in in the console input
  */
fpl_platform_api const char fplConsoleWaitForCharInput();

/** \}*/

/**
  * \defgroup Threading Threading routines
  * \brief Tons of functions for multithreading, mutex and signal creation and handling
  * \{
  */

  //! Thread state
typedef enum fplThreadStates {
	//! Thread is stopped
	fplThreadState_Stopped = 0,
	//! Thread is being started
	fplThreadState_Starting,
	//! Thread is still running
	fplThreadState_Running,
	//! Thread is being stopped
	fplThreadState_Stopping,
} fplThreadStates;

//! Thread state type definition
typedef uint32_t fplThreadState;

typedef struct fplThreadHandle fplThreadHandle;
//! Run function type definition for CreateThread
typedef void (fpl_run_thread_function)(const fplThreadHandle *thread, void *data);

#if defined(FPL_SUBPLATFORM_POSIX)
//! Posix internal thread handle
typedef struct fplPosixInternalThreadHandle {
	//! Thread
	pthread_t thread;
	//! Mutex
	pthread_mutex_t mutex;
	//! Stop condition
	pthread_cond_t stopCondition;
} fplPosixInternalThreadHandle;
#endif

//! Internal thread handle union
typedef union fplInternalThreadHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 thread handle
	HANDLE win32ThreadHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix thread handle
	fplPosixInternalThreadHandle posix;
#endif
} fplInternalThreadHandle;

//! Thread handle
typedef struct fplThreadHandle {
	//! The internal thread handle
	fplInternalThreadHandle internalHandle;
	//! The identifier of the thread
	uint64_t id;
	//! The stored run function
	fpl_run_thread_function *runFunc;
	//! The user data passed to the run function
	void *data;
	//! Thread state
	volatile fplThreadState currentState;
	//! Is this thread valid
	volatile bool isValid;
	//! Is this thread stopping
	volatile bool isStopping;
} fplThreadHandle;

//! Internal mutex handle union
typedef union fplInternalMutexHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 mutex handle
	CRITICAL_SECTION win32CriticalSection;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix mutex handle
	pthread_mutex_t posixMutex;
#endif
} fplInternalMutexHandle;

//! Mutex handle
typedef struct fplMutexHandle {
	//! The internal mutex handle
	fplInternalMutexHandle internalHandle;
	//! Is it valid
	bool isValid;
} fplMutexHandle;

//! Internal signal handle union
typedef union fplInternalSignalHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 event handle
	HANDLE win32EventHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix condition
	pthread_cond_t posixCondition;
#endif
} fplInternalSignalHandle;

//! Signal handle
typedef struct fplSignalHandle {
	//! The internal signal handle
	fplInternalSignalHandle internalHandle;
	//! Is it valid
	bool isValid;
} fplSignalHandle;

//! Returns the current thread state from the given thread
fpl_inline fplThreadState fplGetThreadState(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		return fplThreadState_Stopped;
	}
	fplThreadState result = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&thread->currentState);
	return(result);
}

/**
  * \brief Creates a thread and return a handle to it.
  * \param runFunc Function prototype called when this thread starts.
  * \param data User data passed to the run function.
  * \note Use \ref fplThreadDestroy() with this thread context when you dont need this thread anymore. You can only have 64 threads suspended/running at the same time!
  * \warning Do not free this thread context directly! Use \ref fplThreadDestroy() instead.
  * \return Pointer to a internal stored thread-context or return fpl_null when the limit of current threads has been reached.
  */
fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data);
/**
  * \brief Let the current thread sleep for the given amount of milliseconds.
  * \param milliseconds Number of milliseconds to sleep
  * \note There is no guarantee that the OS sleeps for the exact amount of milliseconds! This can vary based on the OS scheduler granularity.
  */
fpl_platform_api void fplThreadSleep(const uint32_t milliseconds);
/**
  * \brief Stop the given thread and release all underlying resources.
  * \param thread Thread
  * \note This thread context may get re-used for another thread in the future!
  * \warning Do not free the given thread context manually!
  */
fpl_platform_api void fplThreadDestroy(fplThreadHandle *thread);
/**
  * \brief Wait until the given thread is done running or the given timeout has been reached.
  * \param thread Thread
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when the thread completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const uint32_t maxMilliseconds);
/**
  * \brief Wait until all given threads are done running or the given timeout has been reached.
  * \param threads Array of threads
  * \param count Number of threads in the array
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when all threads completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds);
/**
  * \brief Wait until one of given threads is done running or the given timeout has been reached.
  * \param threads Array of threads
  * \param count Number of threads in the array
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when one thread completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds);

/**
  * \brief Creates a mutex and returns a copy of the handle to it.
  * \note Use \ref fplMutexDestroy() when you are done with this mutex.
  * \return Copy of the handle to the mutex.
  */
fpl_platform_api fplMutexHandle fplMutexCreate();
/**
  * \brief Releases the given mutex and clears the structure to zero.
  * \param mutex The mutex reference to destroy.
  */
fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex);
/**
  * \brief Locks the given mutex and ensures that other threads will wait until it gets unlocked or the timeout has been reached.
  * \param mutex The mutex reference to lock
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \returns True when mutex was locked or false otherwise.
  */
fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex, const uint32_t maxMilliseconds);
/**
 * \brief Unlocks the given mutex
 * \param mutex The mutex reference to unlock
 * \returns True when mutex was unlocked or false otherwise.
 */
fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex);

/**
  * \brief Creates a signal and returns a copy of the handle to it.
  * \note Use \ref fplSignalDestroy() when you are done with this signal.
  * \return Copy of the handle to the signal.
  */
fpl_platform_api fplSignalHandle fplSignalCreate();
/**
  * \brief Releases the given signal and clears the structure to zero.
  * \param signal The signal reference to destroy.
  */
fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal);
/**
  * \brief Waits until the given signal are waked up.
  * \param mutex The mutex reference
  * \param signal The signal reference to signal.
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when the signal woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForOne(fplMutexHandle *mutex, fplSignalHandle *signal, const uint32_t maxMilliseconds);
/**
  * \brief Waits until all the given signal are waked up.
  * \param mutex The mutex reference
  * \param signals Array of signals
  * \param count Number of signals
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when all signals woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForAll(fplMutexHandle *mutex, fplSignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds);
/**
  * \brief Waits until any of the given signals wakes up or the timeout has been reached.
  * \param mutex The mutex reference
  * \param signals Array of signals
  * \param count Number of signals
  * \param maxMilliseconds Optional number of milliseconds to wait. When this is set to UINT32_MAX it may wait infinitly. (Default: UINT32_MAX)
  * \return Returns true when any of the signals woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForAny(fplMutexHandle *mutex, fplSignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds);
/**
  * \brief Sets the signal and wakes up the given signal.
  * \param signal The reference to the signal
  * \return Returns true when the signal was set and woke up, otherwise false.
  */
fpl_platform_api bool fplSignalSet(fplSignalHandle *signal);

/** \}*/

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
fpl_common_api void fplMemoryClear(void *mem, const size_t size);
/**
  * \brief Copies the given source memory with its length to the target memory.
  * \param sourceMem Pointer to the source memory to copy from.
  * \param sourceSize Size in bytes to be copied.
  * \param targetMem Pointer to the target memory to copy to.
  */
fpl_common_api void fplMemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem);
/**
  * \brief Allocates memory from the operating system by the given size.
  * \param size Size to by allocated in bytes.
  * \note The memory is guaranteed to be initialized by zero.
  * \warning Alignment is not ensured here, the OS decides how to handle this. If you want to force a specific alignment use \ref fplMemoryAlignedAllocate() instead.
  * \return Pointer to the new allocated memory.
  */
fpl_platform_api void *fplMemoryAllocate(const size_t size);
/**
  * \brief Releases the memory allocated from the operating system.
  * \param ptr Pointer to the allocated memory.
  * \warning This should never be called with a aligned memory pointer! For freeing aligned memory, use \ref fplMemoryAlignedFree() instead.
  * \return Pointer to the new allocated memory.
  */
fpl_platform_api void fplMemoryFree(void *ptr);
/**
  * \brief Allocates aligned memory from the operating system by the given alignment.
  * \param size Size amount in bytes
  * \param alignment Alignment in bytes (Needs to be a power-of-two!)
  * \note The memory is guaranteed to be initialized by zero.
  * \return Pointer to the new allocated aligned memory.
  */
fpl_common_api void *fplMemoryAlignedAllocate(const size_t size, const size_t alignment);
/**
  * \brief Releases the aligned memory allocated from the operating system.
  * \param ptr Pointer to the aligned allocated memory.
  * \warning This should never be called with a not-aligned memory pointer! For freeing not-aligned memory, use \ref fplMemoryFree() instead.
  * \return Pointer to the new allocated memory.
  */
fpl_common_api void fplMemoryAlignedFree(void *ptr);

/** \}*/

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
fpl_platform_api double fplGetTimeInSeconds();

/**
  * \brief Returns the current system in milliseconds without deeper precision.
  * \return Returns number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInMilliseconds();

/** \}*/

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
fpl_common_api bool fplIsStringEqualLen(const char *a, const uint32_t aLen, const char *b, const uint32_t bLen);
/**
  * \brief Returns true when both ansi strings are equal.
  * \param a First string
  * \param b Second string
  * \return True when strings matches, otherwise false.
  */
fpl_common_api bool fplIsStringEqual(const char *a, const char *b);
/**
  * \brief Returns the number of characters of the given 8-bit Ansi string.
  * \param str The 8-bit ansi string
  * \note Null terminator is not included!
  * \return Returns the character length or zero when the input string is fpl_null.
  */
fpl_common_api uint32_t fplGetAnsiStringLength(const char *str);
/**
  * \brief Returns the number of characters of the given 16-bit wide string.
  * \param str The 16-bit wide string
  * \note Null terminator is not included!
  * \return Returns the character length or zero when the input string is fpl_null.
  */
fpl_common_api uint32_t fplGetWideStringLength(const wchar_t *str);
/**
  * \brief Copies the given 8-bit source ansi string with a fixed length into a destination ansi string.
  * \param source The 8-bit source ansi string.
  * \param sourceLen The number of characters to copy.
  * \param dest The 8-bit destination ansi string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api char *fplCopyAnsiStringLen(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
/**
  * \brief Copies the given 8-bit source ansi string into a destination ansi string.
  * \param source The 8-bit source ansi string.
  * \param dest The 8-bit destination ansi string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string with a fixed length into a destination wide string.
  * \param source The 16-bit source wide string.
  * \param sourceLen The number of characters to copy.
  * \param dest The 16-bit destination wide string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string into a destination wide string.
  * \param source The 16-bit source wide string.
  * \param dest The 16-bit destination wide string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit ansi string.
  * \param wideSource The 16-bit source wide string.
  * \param maxWideSourceLen The number of characters of the source wide string.
  * \param ansiDest The 8-bit destination ansi string buffer.
  * \param maxAnsiDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit UTF-8 ansi string.
  * \param wideSource The 16-bit source wide string.
  * \param maxWideSourceLen The number of characters of the source wide string.
  * \param utf8Dest The 8-bit destination ansi string buffer.
  * \param maxUtf8DestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
/**
  * \brief Converts the given 8-bit source ansi string with length in a 16-bit wide string.
  * \param ansiSource The 8-bit source ansi string.
  * \param ansiSourceLen The number of characters of the source wide string.
  * \param wideDest The 16-bit destination wide string buffer.
  * \param maxWideDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
/**
  * \brief Converts the given 8-bit UTF-8 source ansi string with length in a 16-bit wide string.
  * \param utf8Source The 8-bit source ansi string.
  * \param utf8SourceLen The number of characters of the source wide string.
  * \param wideDest The 16-bit destination wide string buffer.
  * \param maxWideDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
/**
  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and variable arguments.
  * \param ansiDestBuffer The 8-bit destination ansi string buffer.
  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer.
  * \param format The string format.
  * \param ... Variable arguments.
  * \note This is most likely just a wrapper call to vsnprintf()
  * \return Pointer to the first character in the destination buffer or fpl_null.
  */
fpl_platform_api char *fplFormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...);

/** \}*/

/**
  * \defgroup Files Files/IO functions
  * \brief Tons of file and directory IO functions
  * \{
  */

//! Internal file handle union
typedef union fplInternalFileHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 file handle
	HANDLE win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix file handle
	int posixFileHandle;
#endif
} fplInternalFileHandle;

//! Handle to a loaded/created file
typedef struct fplFileHandle {
	//! Internal file handle
	fplInternalFileHandle internalHandle;
	//! File opened successfully
	bool isValid;
} fplFileHandle;

//! File position mode (Beginning, Current, End)
typedef enum fplFilePositionMode {
	//! Starts from the beginning
	fplFilePositionMode_Beginning = 0,
	//! Starts from the current position
	fplFilePositionMode_Current,
	//! Starts from the end
	fplFilePositionMode_End
} fplFilePositionMode;

//! File entry type (File, Directory, etc.)
typedef enum fplFileEntryType {
	//! Unknown entry type
	fplFileEntryType_Unknown = 0,
	//! Entry is a file
	fplFileEntryType_File,
	//! Entry is a directory
	fplFileEntryType_Directory
} fplFileEntryType;

//! File attribute flags (Normal, Readonly, Hidden, etc.)
typedef enum fplFileAttributeFlags {
	//! No attributes
	fplFileAttributeFlags_None = 0,
	//! Normal
	fplFileAttributeFlags_Normal = 1 << 0,
	//! Readonly
	fplFileAttributeFlags_ReadOnly = 1 << 1,
	//! Hidden
	fplFileAttributeFlags_Hidden = 1 << 2,
	//! Archive
	fplFileAttributeFlags_Archive = 1 << 3,
	//! System
	fplFileAttributeFlags_System = 1 << 4
} fplFileAttributeFlags;
//! FileAttributeFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplFileAttributeFlags);

//! Maximum length of a file entry path
#define FPL_MAX_FILEENTRY_PATH_LENGTH 1024

//! Internal file entry handle
typedef struct fplInternalFileEntryHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 file handle
	HANDLE win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix file handle
	int posixFileHandle;
#endif
} fplInternalFileEntryHandle;

//! Entry for storing current file informations (path, type, attributes, etc.)
typedef struct fplFileEntry {
	//! File path
	char path[FPL_MAX_FILEENTRY_PATH_LENGTH];
	//! Internal file handle
	fplInternalFileEntryHandle internalHandle;
	//! Entry type
	fplFileEntryType type;
	//! File attributes
	fplFileAttributeFlags attributes;
} fplFileEntry;

/**
  * \brief Opens a binary file for reading from a ansi string path and returns the handle of it.
  * \param filePath Ansi file path.
  * \param outHandle Pointer to the file handle
  * \return True when binary ansi file was opened, false otherwise
  */
fpl_platform_api bool fplOpenAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle);
/**
  * \brief Opens a binary file for reading from a wide string path and returns the handle of it.
  * \param filePath Wide file path.
  * \param outHandle Pointer to the file handle
  * \return True when binary wide file was opened, false otherwise
  */
fpl_platform_api bool fplOpenWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle);
/**
  * \brief Create a binary file for writing to the given ansi string path and returns the handle of it.
  * \param filePath Ansi file path.
  * \param outHandle Pointer to the file handle
  * \return True when binary ansi file was created, false otherwise
  */
fpl_platform_api bool fplCreateAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle);
/**
  * \brief Create a binary file for writing to the given wide string path and returns the handle of it.
  * \param filePath Wide file path.
  * \param outHandle Pointer to the file handle
  * \return True when binary wide file was created, false otherwise
  */
fpl_platform_api bool fplCreateWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle);
/**
  * \brief Reads a block from the given file handle and returns the number of bytes read.
  * \param fileHandle Reference to the file handle.
  * \param sizeToRead Number of bytes to read.
  * \param targetBuffer Target memory to write into.
  * \param maxTargetBufferSize Total number of bytes available in the target buffer.
  * \note Its limited to files < 2 GB.
  * \return Number of bytes read or zero.
  */
fpl_platform_api uint32_t fplReadFileBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
/**
  * \brief Writes a block to the given file handle and returns the number of bytes written.
  * \param fileHandle Reference to the file handle.
  * \param sourceBuffer Source memory to read from.
  * \param sourceSize Number of bytes to write.
  * \note Its limited to files < 2 GB.
  * \return Number of bytes written or zero.
  */
fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
/**
  * \brief Sets the current file position by the given position, depending on the mode its absolute or relative.
  * \param fileHandle Reference to the file handle.
  * \param position Position in bytes
  * \param mode Position mode
  * \note Its limited to files < 2 GB.
  */
fpl_platform_api void fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode);
/**
  * \brief Returns the current file position in bytes.
  * \param fileHandle Reference to the file handle.
  * \note Its limited to files < 2 GB.
  * \return Current file position in bytes.
  */
fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle);
/**
  * \brief Closes the given file and releases the underlying resources and clears the handle to zero.
  * \param fileHandle Reference to the file handle.
  */
fpl_platform_api void CloseFile(fplFileHandle *fileHandle);

// @TODO(final): Add 64-bit file operations
// @TODO(final): Add wide file operations

/**
  * \brief Returns the 32-bit file size in bytes for the given file.
  * \param filePath Ansi path to the file.
  * \note Its limited to files < 2 GB.
  * \return File size in bytes or zero.
  */
fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath);
/**
  * \brief Returns the 32-bit file size in bytes for a opened file.
  * \param fileHandle Reference to the file handle.
  * \note Its limited to files < 2 GB.
  * \return File size in bytes or zero.
  */
fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle);
/**
  * \brief Returns true when the given file physically exists.
  * \param filePath Ansi path to the file.
  * \return True when the file exists, otherwise false.
  */
fpl_platform_api bool fplFileExists(const char *filePath);
/**
  * \brief Copies the given source file to the target path and returns true when copy was successful.
  * \param sourceFilePath Ansi source file path.
  * \param targetFilePath Ansi target file path.
  * \param overwrite When true the target file always be overwritten, otherwise it will return false when file already exists.
  * \return True when the file was copied, otherwise false.
  */
fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
/**
  * \brief Movies the given source file to the target file and returns true when the move was successful.
  * \param sourceFilePath Ansi source file path.
  * \param targetFilePath Ansi target file path.
  * \return True when the file was moved, otherwise false.
  */
fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath);
/**
  * \brief Deletes the given file without confirmation and returns true when the deletion was successful.
  * \param filePath Ansi path to the file.
  * \return True when the file was deleted, otherwise false.
  */
fpl_platform_api bool fplFileDelete(const char *filePath);

/**
  * \brief Creates all the directories in the given path.
  * \param path Ansi path to the directory.
  * \return True when at least one directory was created, otherwise false.
  */
fpl_platform_api bool fplDirectoriesCreate(const char *path);
/**
  * \brief Returns true when the given directory physically exists.
  * \param path Ansi path to the directory.
  * \return True when the directory exists, otherwise false.
  */
fpl_platform_api bool fplDirectoryExists(const char *path);
/**
  * \brief Deletes the given empty directory without confirmation and returns true when the deletion was successful.
  * \param path Ansi path to the directory.
  * \return True when the empty directory was deleted, otherwise false.
  */
fpl_platform_api bool fplDirectoryRemove(const char *path);
/**
  * \brief Iterates through files / directories in the given directory.
  * \param pathAndFilter The path with its included after the path separator.
  * \param firstEntry The reference to a file entry.
  * \note The path must contain the filter as well.
  * \return Returns true when there was a first entry found otherwise false.
  */
fpl_platform_api bool fplListFilesBegin(const char *pathAndFilter, fplFileEntry *firstEntry);
/**
  * \brief Gets the next file entry from iterating through files / directories.
  * \param nextEntry The reference to the current file entry.
  * \return Returns true when there was a next file otherwise false if not.
  */
fpl_platform_api bool fplListFilesNext(fplFileEntry *nextEntry);
/**
  * \brief Releases opened resources from iterating through files / directories.
  * \param lastEntry The reference to the last file entry.
  */
fpl_platform_api void fplListFilesEnd(fplFileEntry *lastEntry);

/** \}*/

/**
  * \defgroup Paths Path functions
  * \brief Functions for retrieving paths like HomePath, ExecutablePath, etc.
  * \{
  */

// @TODO(final): Support wide path for 'paths' as well

/**
  * \brief Returns the full path to this executable, including the executable file name.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
/**
  * \brief Returns the full path to your home directory.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_platform_api char *fplGetHomePath(char *destPath, const uint32_t maxDestLen);
/**
  * \brief Returns the path from the given source path.
  * \param sourcePath Source path to extract from.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen);
/**
  * \brief Returns the file extension from the given source path.
  * \param sourcePath Source path to extract from.
  * \return Returns the pointer to the first character of the extension.
  */
fpl_common_api const char *fplExtractFileExtension(const char *sourcePath);
/**
  * \brief Returns the file name including the file extension from the given source path.
  * \param sourcePath Source path to extract from.
  * \return Returns the pointer to the first character of the filename.
  */
fpl_common_api const char *fplExtractFileName(const char *sourcePath);
/**
  * \brief Changes the file extension on the given source path and writes the result into the destination path.
  * \param filePath File path to search for the extension.
  * \param newFileExtension New file extension.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen);
/**
  * \brief Combines all included path by the systems path separator.
  * \param destPath Destination buffer
  * \param maxDestPathLen Total number of characters available in the destination buffer.
  * \param pathCount Number of dynamic path arguments.
  * \param ... Dynamic path arguments.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplPathCombine(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);

/** \}*/

#if defined(FPL_ENABLE_WINDOW)
/**
* \defgroup WindowEvents Window events
* \brief Window event structures
* \{
*/

//! Mapped keys (Based on MS Virtual-Key-Codes, mostly directly mapped from ASCII)
typedef enum fplKey {
	fplKey_None = 0,

	// 0x07: Undefined

	fplKey_Backspace = 0x08,
	fplKey_Tab = 0x09,

	// 0x0A-0x0B: Reserved

	fplKey_Clear = 0x0C,
	fplKey_Enter = 0x0D,

	// 0x0E-0x0F: Undefined

	fplKey_Shift = 0x10,
	fplKey_Control = 0x11,
	fplKey_Alt = 0x12,
	fplKey_Pause = 0x13,
	fplKey_CapsLock = 0x14,

	// 0x15: IME-fplKeys
	// 0x16: Undefined
	// 0x17-0x19 IME-fplKeys
	// 0x1A: Undefined

	fplKey_Escape = 0x1B,

	// 0x1C - 0x1F: IME-fplKeys

	fplKey_Space = 0x20,
	fplKey_PageUp = 0x21,
	fplKey_PageDown = 0x22,
	fplKey_End = 0x23,
	fplKey_Home = 0x24,
	fplKey_Left = 0x25,
	fplKey_Up = 0x26,
	fplKey_Right = 0x27,
	fplKey_Down = 0x28,
	fplKey_Select = 0x29,
	fplKey_Print = 0x2A,
	fplKey_Execute = 0x2B,
	fplKey_Snapshot = 0x2C,
	fplKey_Insert = 0x2D,
	fplKey_Delete = 0x2E,
	fplKey_Help = 0x2F,

	fplKey_0 = 0x30,
	fplKey_1 = 0x31,
	fplKey_2 = 0x32,
	fplKey_3 = 0x33,
	fplKey_4 = 0x34,
	fplKey_5 = 0x35,
	fplKey_6 = 0x36,
	fplKey_7 = 0x37,
	fplKey_8 = 0x38,
	fplKey_9 = 0x39,

	// 0x3A-0x40: Undefined

	fplKey_A = 0x41,
	fplKey_B = 0x42,
	fplKey_C = 0x43,
	fplKey_D = 0x44,
	fplKey_E = 0x45,
	fplKey_F = 0x46,
	fplKey_G = 0x47,
	fplKey_H = 0x48,
	fplKey_I = 0x49,
	fplKey_J = 0x4A,
	fplKey_K = 0x4B,
	fplKey_L = 0x4C,
	fplKey_M = 0x4D,
	fplKey_N = 0x4E,
	fplKey_O = 0x4F,
	fplKey_P = 0x50,
	fplKey_Q = 0x51,
	fplKey_R = 0x52,
	fplKey_S = 0x53,
	fplKey_T = 0x54,
	fplKey_U = 0x55,
	fplKey_V = 0x56,
	fplKey_W = 0x57,
	fplKey_X = 0x58,
	fplKey_Y = 0x59,
	fplKey_Z = 0x5A,

	fplKey_LeftWin = 0x5B,
	fplKey_RightWin = 0x5C,
	fplKey_Apps = 0x5D,

	// 0x5E: Reserved

	fplKey_Sleep = 0x5F,
	fplKey_NumPad0 = 0x60,
	fplKey_NumPad1 = 0x61,
	fplKey_NumPad2 = 0x62,
	fplKey_NumPad3 = 0x63,
	fplKey_NumPad4 = 0x64,
	fplKey_NumPad5 = 0x65,
	fplKey_NumPad6 = 0x66,
	fplKey_NumPad7 = 0x67,
	fplKey_NumPad8 = 0x68,
	fplKey_NumPad9 = 0x69,
	fplKey_Multiply = 0x6A,
	fplKey_Add = 0x6B,
	fplKey_Separator = 0x6C,
	fplKey_Substract = 0x6D,
	fplKey_Decimal = 0x6E,
	fplKey_Divide = 0x6F,
	fplKey_F1 = 0x70,
	fplKey_F2 = 0x71,
	fplKey_F3 = 0x72,
	fplKey_F4 = 0x73,
	fplKey_F5 = 0x74,
	fplKey_F6 = 0x75,
	fplKey_F7 = 0x76,
	fplKey_F8 = 0x77,
	fplKey_F9 = 0x78,
	fplKey_F10 = 0x79,
	fplKey_F11 = 0x7A,
	fplKey_F12 = 0x7B,
	fplKey_F13 = 0x7C,
	fplKey_F14 = 0x7D,
	fplKey_F15 = 0x7E,
	fplKey_F16 = 0x7F,
	fplKey_F17 = 0x80,
	fplKey_F18 = 0x81,
	fplKey_F19 = 0x82,
	fplKey_F20 = 0x83,
	fplKey_F21 = 0x84,
	fplKey_F22 = 0x85,
	fplKey_F23 = 0x86,
	fplKey_F24 = 0x87,

	// 0x88-8F: Unassigned

	fplKey_NumLock = 0x90,
	fplKey_Scroll = 0x91,

	// 0x92-9x96: OEM specific
	// 0x97-0x9F: Unassigned

	fplKey_LeftShift = 0xA0,
	fplKey_RightShift = 0xA1,
	fplKey_LeftControl = 0xA2,
	fplKey_RightControl = 0xA3,
	fplKey_LeftAlt = 0xA4,
	fplKey_RightAlt = 0xA5,

	// 0xA6-0xFE: Dont care
} fplKey;

//! Window event type (Resized, PositionChanged, etc.)
typedef enum fplWindowEventType {
	//! None window event type
	fplWindowEventType_None = 0,
	//! Window has been resized
	fplWindowEventType_Resized,
	//! Window got focus
	fplWindowEventType_GotFocus,
	//! Window lost focus
	fplWindowEventType_LostFocus,
} fplWindowEventType;

//! Window event data (Size, Position, etc.)
typedef struct fplWindowEvent {
	//! Window event type
	fplWindowEventType type;
	//! Window width in screen coordinates
	uint32_t width;
	//! Window height in screen coordinates
	uint32_t height;
} fplWindowEvent;

//! Keyboard event type (KeyDown, KeyUp, Char, ...)
typedef enum fplKeyboardEventType {
	//! None key event type
	fplKeyboardEventType_None = 0,
	//! Key is down
	fplKeyboardEventType_KeyDown,
	//! Key was released
	fplKeyboardEventType_KeyUp,
	//! Character was entered
	fplKeyboardEventType_CharInput,
} fplKeyboardEventType;

//! Keyboard modifier flags (Alt, Ctrl, ...)
typedef enum fplKeyboardModifierFlags {
	//! No modifiers
	fplKeyboardModifierFlags_None = 0,
	//! Alt key is down
	fplKeyboardModifierFlags_Alt = 1 << 0,
	//! Ctrl key is down
	fplKeyboardModifierFlags_Ctrl = 1 << 1,
	//! Shift key is down
	fplKeyboardModifierFlags_Shift = 1 << 2,
	//! Super key is down
	fplKeyboardModifierFlags_Super = 1 << 3,
} fplKeyboardModifierFlags;
//! KeyboardModifierFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplKeyboardModifierFlags);

//! Keyboard event data (Type, Keycode, Mapped key, etc.)
typedef struct fplKeyboardEvent {
	//! Keyboard event type
	fplKeyboardEventType type;
	//! Raw key code
	uint64_t keyCode;
	//! Mapped key
	fplKey mappedKey;
	//! Keyboard modifiers
	fplKeyboardModifierFlags modifiers;
} fplKeyboardEvent;

//! Mouse event type (Move, ButtonDown, ...)
typedef enum fplMouseEventType {
	//! No mouse event type
	fplMouseEventType_None,
	//! Mouse position has been changed
	fplMouseEventType_Move,
	//! Mouse button is down
	fplMouseEventType_ButtonDown,
	//! Mouse button was released
	fplMouseEventType_ButtonUp,
	//! Mouse wheel up/down
	fplMouseEventType_Wheel,
} fplMouseEventType;

//! Mouse button type (Left, Right, ...)
typedef enum fplMouseButtonType {
	//! No mouse button
	fplMouseButtonType_None = -1,
	//! Left mouse button
	fplMouseButtonType_Left = 0,
	//! Right mouse button
	fplMouseButtonType_Right = 1,
	//! Middle mouse button
	fplMouseButtonType_Middle = 2,
} fplMouseButtonType;

//! Mouse event data (Type, Button, Position, etc.)
typedef struct fplMouseEvent {
	//! Mouse event type
	fplMouseEventType type;
	//! Mouse button
	fplMouseButtonType mouseButton;
	//! Mouse X-Position
	int32_t mouseX;
	//! Mouse Y-Position
	int32_t mouseY;
	//! Mouse wheel delta
	float wheelDelta;
} fplMouseEvent;

//! Gamepad event type (Connected, Disconnected, StateChanged, etc.)
typedef enum fplGamepadEventType {
	//! No gamepad event
	fplGamepadEventType_None = 0,
	//! Gamepad connected
	fplGamepadEventType_Connected,
	//! Gamepad disconnected
	fplGamepadEventType_Disconnected,
	//! Gamepad state updated
	fplGamepadEventType_StateChanged,
} fplGamepadEventType;

//! Gamepad button (IsDown, etc.)
typedef struct fplGamepadButton {
	//! Is button down
	bool isDown;
} fplGamepadButton;

//! Gamepad state data
typedef struct fplGamepadState {
	union {
		struct {
			//! Digital button up
			fplGamepadButton dpadUp;
			//! Digital button right
			fplGamepadButton dpadRight;
			//! Digital button down
			fplGamepadButton dpadDown;
			//! Digital button left
			fplGamepadButton dpadLeft;

			//! Action button A
			fplGamepadButton actionA;
			//! Action button B
			fplGamepadButton actionB;
			//! Action button X
			fplGamepadButton actionX;
			//! Action button Y
			fplGamepadButton actionY;

			//! Start button
			fplGamepadButton start;
			//! Back button
			fplGamepadButton back;

			//! Analog left thumb button
			fplGamepadButton leftThumb;
			//! Analog right thumb button
			fplGamepadButton rightThumb;

			//! Left shoulder button
			fplGamepadButton leftShoulder;
			//! Right shoulder button
			fplGamepadButton rightShoulder;
		};
		//! All gamepad buttons
		fplGamepadButton buttons[14];
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
} fplGamepadState;

//! Gamepad event data (Type, Device, State, etc.)
typedef struct fplGamepadEvent {
	//! Gamepad event type
	fplGamepadEventType type;
	//! Gamepad device index
	uint32_t deviceIndex;
	//! Full gamepad state
	fplGamepadState state;
} fplGamepadEvent;

//! Event type (Window, Keyboard, Mouse, ...)
typedef enum fplEventType {
	//! None event type
	fplEventType_None = 0,
	//! Window event
	fplEventType_Window,
	//! Keyboard event
	fplEventType_Keyboard,
	//! Mouse event
	fplEventType_Mouse,
	//! Gamepad event
	fplEventType_Gamepad,
} fplEventType;

//! Event data (Type, Window, Keyboard, Mouse, etc.)
typedef struct fplEvent {
	//! Event type
	fplEventType type;
	union {
		//! Window event data
		fplWindowEvent window;
		//! Keyboard event data
		fplKeyboardEvent keyboard;
		//! Mouse event data
		fplMouseEvent mouse;
		//! Gamepad event data
		fplGamepadEvent gamepad;
	};
} fplEvent;

/**
  * \brief Gets the top event from the internal event queue and removes it.
  * \param ev Reference to an event
  * \return Returns false when there are no events left, otherwise true.
  */
fpl_common_api bool fplPollEvent(fplEvent *ev);
/**
  * \brief Removes all the events from the internal event queue.
  * \note Dont call when you care about any event!
  */
fpl_common_api void fplClearEvents();
/**
  * \brief Reads the next window event from the OS and pushes it into the internal queue.
  * \return Returns true when there was a event from the OS, otherwise true.
  * \note Use this only if dont use \ref fplWindowUpdate() and want to handle the events more granular!
  */
fpl_platform_api bool fplPushEvent();
/**
  * \brief Updates the game controller states and detects new and disconnected devices.
  * \note Use this only if dont use \ref fplWindowUpdate() and want to handle the events more granular!
  */
fpl_platform_api void fplUpdateGameControllers();

/*\}*/

/**
  * \defgroup WindowBase Window functions
  * \brief Functions for reading/setting/handling the window
  * \{
  */

  //! Window size in screen coordinates
typedef struct fplWindowSize {
	//! Width in screen coordinates
	uint32_t width;
	//! Height in screen coordinates
	uint32_t height;
} fplWindowSize;

//! Window position in screen coordinates
typedef struct fplWindowPosition {
	//! Left position in screen coordinates
	int32_t left;
	//! Top position in screen coordinates
	int32_t top;
} fplWindowPosition;

/**
  * \brief Returns true when the window is active.
  * \return True when the window is active, otherwise false.
  */
fpl_platform_api bool fplIsWindowRunning();
/**
  * \brief Processes the message queue of the window.
  * \note This will update the game controller states as well.
  * \return True when the window is still active, otherwise false.
  */
fpl_platform_api bool fplWindowUpdate();
/**
  * \brief Enables or disables the window cursor.
  * \param value Set this to true for enabling the cursor or false for disabling the cursor.
  */
fpl_platform_api void fplSetWindowCursorEnabled(const bool value);
/**
  * \brief Returns the inner window area.
  * \return Window area size
  */
fpl_platform_api fplWindowSize GetWindowArea();
/**
  * \brief Resizes the window to fit the inner area to the given size.
  * \param width Width in screen units
  * \param height Height in screen units
  */
fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height);
/**
  * \brief Returns true when the window is resizable.
  * \return True when the window resizable, otherwise false.
  */
fpl_platform_api bool fplIsWindowResizable();
/**
  * \brief Enables or disables the ability to resize the window.
  * \param value Set this to true for making the window resizable or false for making it static
  */
fpl_platform_api void fplSetWindowResizeable(const bool value);
/**
  * \brief Enables or disables fullscreen mode.
  * \param value Set this to true for changing the window to fullscreen or false for switching it back to window mode.
  * \param fullscreenWidth Optional fullscreen width in screen units. When set to zero the desktop default is being used. (Default: 0)
  * \param fullscreenHeight Optional fullscreen height in screen units. When set to zero the desktop default is being used. (Default: 0)
  * \param refreshRate Optional refresh rate in screen units. When set to zero the desktop default is being used. (Default: 0)
  * \return True when the window was changed to the desire fullscreen mode, false when otherwise.
  */
fpl_platform_api bool fplSetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate);
/**
  * \brief Returns true when the window is in fullscreen mode
  * \return True when the window is in fullscreen mode, otherwise false.
  */
fpl_platform_api bool fplIsWindowFullscreen();
/**
  * \brief Returns the absolute window position.
  * \return Window position in screen units
  */
fpl_platform_api fplWindowPosition fplGetWindowPosition();
/**
  * \brief Sets the window absolut position to the given coordinates.
  * \param left Left position in screen units.
  * \param top Top position in screen units.
  */
fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top);
/**
  * \brief Sets the window title.
  * \param title New title ansi string
  */
fpl_platform_api void fplSetWindowTitle(const char *title);

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
	* \return Pointer to the first character in the clipboard text or fpl_null otherwise.
	*/
fpl_platform_api char *fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
/**
  * \brief Returns the current clipboard wide text.
  * \param dest The destination wide string buffer to write the clipboard text into.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \return Pointer to the first character in the clipboard text or fpl_null otherwise.
  */
fpl_platform_api wchar_t *fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
/**
  * \brief Overwrites the current clipboard ansi text with the given one.
  * \param ansiSource The new clipboard ansi string.
  * \return Returns true when the text in the clipboard was changed, otherwise false.
  */
fpl_platform_api bool fplSetClipboardAnsiText(const char *ansiSource);
/**
  * \brief Overwrites the current clipboard wide text with the given one.
  * \param wideSource The new clipboard wide string.
  * \return Returns true when the text in the clipboard was changed, otherwise false.
  */
fpl_platform_api bool fplSetClipboardWideText(const wchar_t *wideSource);

/** \}*/
#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_VIDEO)
/**
  * \defgroup Video Video functions
  * \brief Functions for retrieving or resizing the video buffer
  * \{
  */

//! Video rectangle
typedef struct fplVideoRect {
	//! Left position in pixels
	int32_t x;
	//! Top position in pixels
	int32_t y;
	//! Width in pixels
	int32_t width;
	//! Height in pixels
	int32_t height;
} fplVideoRect;

/**
  * \brief Makes a video rectangle from a LT-RB rectangle
  * \param left Left position in screen units.
  * \param top Top position in screen units.
  * \param right Right position in screen units.
  * \param bottom Bottom position in screen units.
  * \return Computed video rectangle
  */
fpl_inline fplVideoRect fplCreateVideoRectFromLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
	fplVideoRect result = { left, top, (right - left) + 1, (bottom - top) + 1 };
	return(result);
}

/**
  * \brief Returns the string for the given video driver
  * \param driver The audio driver
  * \return String for the given audio driver
  */
fpl_inline const char *fplGetVideoDriverString(fplVideoDriverType driver) {
	const char *VIDEO_DRIVER_TYPE_STRINGS[] = {
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
typedef struct fplVideoBackBuffer {
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
	fplVideoRect outputRect;
	//! Set this to true to actually use the output rectangle
	bool useOutputRect;
} fplVideoBackBuffer;

/**
  * \brief Returns the pointer to the video software context.
  * \warning Do not release this memory by any means, otherwise you will corrupt heap memory!
  * \return Pointer to the video backbuffer.
  */
fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer();
/**
  * \brief Resizes the current video backbuffer.
  * \param width Width in pixels.
  * \param height Height in pixels.
  * \return Returns true when video back buffer could be resized or false otherwise.
  */
fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height);
/**
  * \brief Returns the current video driver type used.
  * \return The current video driver type used.
  */
fpl_common_api fplVideoDriverType fplGetVideoDriver();

/**
  * \brief Forces the window to redraw or to swap the back/front buffer.
  */
fpl_common_api void fplVideoFlip();

/** \}*/
#endif // FPL_ENABLE_VIDEO

#if defined(FPL_ENABLE_AUDIO)
/**
  * \defgroup Audio Audio functions
  * \brief Functions for start/stop playing audio and retrieving/changing some audio related settings.
  * \{
  */

//! Audio result
typedef enum fplAudioResult {
	fplAudioResult_None = 0,
	fplAudioResult_Success,
	fplAudioResult_DeviceNotInitialized,
	fplAudioResult_DeviceAlreadyStopped,
	fplAudioResult_DeviceAlreadyStarted,
	fplAudioResult_DeviceBusy,
	fplAudioResult_Failed,
} fplAudioResult;

/**
  * \brief Start playing asyncronous audio.
  * \return Audio result code.
  */
fpl_common_api fplAudioResult fplPlayAudio();
/**
  * \brief Stop playing asyncronous audio.
  * \return Audio result code.
  */
fpl_common_api fplAudioResult fplStopAudio();
/**
  * \brief Returns the native format for the current audio device.
  * \return Copy fo the audio device format.
  */
fpl_common_api fplAudioDeviceFormat fplGetAudioHardwareFormat();
/**
  * \brief Overwrites the audio client read callback.
  * \param newCallback Pointer to the client read callback.
  * \param userData Pointer to the client/user data.
  * \note This has no effect when audio is already playing, you have to call it when audio is in a stopped state!
  */
fpl_common_api void fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData);
/**
  * \brief Gets all playback audio devices.
  * \param devices Target device id array.
  * \param maxDeviceCount Total number of devices available in the devices array.
  * \return Number of devices found.
  */
fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount);

/**
  * \brief Returns the number of bytes required to write one sample with one channel
  * \param format The audio format
  * \return Number of bytes for one sample with one channel
  */
fpl_inline uint32_t fplGetAudioSampleSizeInBytes(const fplAudioFormatType format) {
	switch (format) {
		case fplAudioFormatType_U8:
			return 1;
		case fplAudioFormatType_S16:
			return 2;
		case fplAudioFormatType_S24:
			return 3;
		case fplAudioFormatType_S32:
		case fplAudioFormatType_F32:
			return 4;
		case fplAudioFormatType_S64:
		case fplAudioFormatType_F64:
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
fpl_inline const char *fplGetAudioFormatString(const fplAudioFormatType format) {
	// @NOTE(final): Order must be equal to the AudioFormatType enum!
	const char *AUDIO_FORMAT_TYPE_STRINGS[] = {
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
fpl_inline const char *fplGetAudioDriverString(fplAudioDriverType driver) {
	const char *AUDIO_DRIVER_TYPE_STRINGS[] = {
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
fpl_inline uint32_t fplGetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds) {
	uint32_t result = (sampleRate / 1000) * bufferSizeInMilliSeconds;
	return(result);
}

/**
  * \brief Returns the number of bytes required for one interleaved audio frame - containing all the channels
  * \param format The audio format
  * \param channelCount The number of channels
  * \return Number of bytes for one frame in bytes
  */
fpl_inline uint32_t fplGetAudioFrameSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount) {
	uint32_t result = fplGetAudioSampleSizeInBytes(format) * channelCount;
	return(result);
}

/**
  * \brief Returns the total number of bytes for the buffer and the given parameters
  * \param format The audio format
  * \param channelCount The number of channels
  * \param frameCount The number of frames
  * \return Total number of bytes for the buffer
  */
fpl_inline uint32_t fplGetAudioBufferSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount, const uint32_t frameCount) {
	uint32_t frameSize = fplGetAudioFrameSizeInBytes(format, channelCount);
	uint32_t result = frameSize * frameCount;
	return(result);
}

/** \}*/
#endif // FPL_ENABLE_AUDIO

#endif // FPL_INCLUDE_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// FPL uses several implementation blocks to structure things in categories.
// Each block has its own ifdef definition to collapse it down if needed.
// But the baseline structure is the follow:
//
// - Platform Constants & Types (All required structs, Constants, Global variables, etc.)
// - Common implementations
// - Actual platform implementations (Win32, Linux)
// - Sub platform implementations (X11, POSIX, STD)
// - Driver implementations (Video: OpenGL/Software, Audio: DirectSound/Alsa)
// - Systems (Audio, Video, Window systems)
// - Core (Init & Release of the specific platform by selection)
//
// You can use the following strings to search for implementation blocks - including the > prefix:
//
// > PLATFORM_CONSTANTS
//
// > TYPES
// > TYPES_WIN32
// > TYPES_POSIX
// > TYPES_LINUX
// > TYPES_X11
//
// > PLATFORM_STATES
//
// > COMMON
//
// > WIN32_PLATFORM
// > LINUX_PLATFORM
//
// > POSIX_SUBPLATFORM
// > X11_SUBPLATFORM
//
// > STD_STRINGS_SUBPLATFORM
// > STD_CONSOLE_SUBPLATFORM
//
// > VIDEO_DRIVERS
// > VIDEO_DRIVER_OPENGL_WIN32
// > VIDEO_DRIVER_OPENGL_X11
// > VIDEO_DRIVER_SOFTWARE_WIN32
//
// > AUDIO_DRIVERS
// > AUDIO_DRIVER_DIRECTSOUND
//
// > SYSTEM_AUDIO_L1
// > SYSTEM_VIDEO_L1
// > SYSTEM_WINDOW
// > SYSTEM_AUDIO_L2
// > SYSTEM_VIDEO_L2 (Video backbuffer access and present of the frame)
// > SYSTEM_INIT (Init & Release of the Platform)
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#define FPL_IMPLEMENTED

//
// Very simple logging system
//
#if defined(FPL_ENABLE_LOGGING)
#	include <stdio.h> // fprintf
#	define FPL_LOG_FORMAT(what, format) "[" what "] " format "\n"
#   define FPL_LOG(what, format, ...) do { \
		::fprintf(stdout, FPL_LOG_FORMAT(what, format), ## __VA_ARGS__); \
	} while (0)
#   define FPL_LOG_FUNCTION_N(what, name) fprintf(stdout, "> %s %s\n", what, name)
#   define FPL_LOG_FUNCTION(what) FPL_LOG_FUNCTION_N(what, FPL_FUNCTION_NAME)

#if defined(FPL_IS_CPP)
struct fplLogBlock {
	const char *funcName;
	fplLogBlock(const char *funcName) {
		this->funcName = funcName;
		FPL_LOG_FUNCTION_N("Enter", funcName);
	}
	~fplLogBlock() {
		FPL_LOG_FUNCTION_N("Leave", this->funcName);
	}
};
#   define FPL_LOG_BLOCK_NAME flb__COUNTER__
#   define FPL_LOG_BLOCK_N(name) fplLogBlock FPL_LOG_BLOCK_NAME(name)
#   define FPL_LOG_BLOCK FPL_LOG_BLOCK_N(FPL_FUNCTION_NAME)
#else
#   define FPL_LOG_BLOCK
#endif
#else
#   define FPL_LOG(what, format, ...)
#   define FPL_LOG_FUNCTION(what)
#   define FPL_LOG_BLOCK
#endif

#define FPL_CLEAR_STRUCT(ptr) fplMemoryClear((void *)(ptr), sizeof(*(ptr)))

// ############################################################################
//
// > PLATFORM_CONSTANTS
//
// ############################################################################
#if !defined(FPL_PLATFORM_CONSTANTS_DEFINED)
#define FPL_PLATFORM_CONSTANTS_DEFINED

#if defined(FPL_PLATFORM_WIN32)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_main int main(int argc, char **args);
#endif // FPL_PLATFORM_WIN32

#if defined(FPL__PLATFORM_WIN32)
#	define FPL__PATH_SEPARATOR '\\'
#	define FPL__FILE_EXT_SEPARATOR '.'
#else
#	define FPL__PATH_SEPARATOR '/'
#	define FPL__FILE_EXT_SEPARATOR '.'
#endif

// One cacheline worth of padding
#define FPL__SIZE_PADDING 64

fpl_globalvar struct fpl__PlatformAppState* fpl__global__AppState = fpl_null;

fpl_internal void fpl__PushError(const char *format, ...);
#endif // FPL_PLATFORM_CONSTANTS_DEFINED

// ****************************************************************************
//
// > TYPES
//
// This implementation block includes all the required platform-specific
// header files and defines all the constants, structures and types.
//
// ****************************************************************************

// ############################################################################
//
// > TYPES_WIN32
//
// ############################################################################
#if defined(FPL_PLATFORM_WIN32)
#	include <windowsx.h>	// Macros for window messages
#	include <shlobj.h>		// SHGetFolderPath
#	include <intrin.h>		// Interlock*
#	include <xinput.h>		// XInputGetState

// Little macro to not write 5 lines of code all the time
#define FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(libHandle, libName, target, type, name) \
	target = (type *)GetProcAddress(libHandle, name); \
	if (target == fpl_null) { \
		fpl__PushError("Failed getting procedure address '%s' from library '%s'", name, libName); \
		return false; \
	}
#define FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, target, type, name) \
	target = (type *)GetProcAddress(libHandle, name); \
	if (target == fpl_null) { \
		fpl__PushError("Failed getting procedure address '%s' from library '%s'", name, libName); \
		break; \
	}

//
// XInput
//
#define FPL__XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef FPL__XINPUT_GET_STATE(fpl__win32_func_XInputGetState);
FPL__XINPUT_GET_STATE(fpl__Win32XInputGetStateStub) {
	return(ERROR_DEVICE_NOT_CONNECTED);
}
#define FPL_XINPUT_GET_CAPABILITIES(name) DWORD WINAPI name(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
typedef FPL_XINPUT_GET_CAPABILITIES(fpl__win32_func_XInputGetCapabilities);
FPL_XINPUT_GET_CAPABILITIES(fpl__Win32XInputGetCapabilitiesStub) {
	return(ERROR_DEVICE_NOT_CONNECTED);
}
typedef struct fpl__Win32XInputApi {
	HMODULE xinputLibrary;
	fpl__win32_func_XInputGetState *xInputGetState;
	fpl__win32_func_XInputGetCapabilities *xInputGetCapabilities;
} fpl__Win32XInputApi;

fpl_internal void fpl__Win32UnloadXInputApi(fpl__Win32XInputApi *xinputApi) {
	FPL_ASSERT(xinputApi != fpl_null);
	if (xinputApi->xinputLibrary) {
		FPL_LOG("XInput", "Unload XInput Library");
		FreeLibrary(xinputApi->xinputLibrary);
		xinputApi->xinputLibrary = fpl_null;
		xinputApi->xInputGetState = fpl__Win32XInputGetStateStub;
		xinputApi->xInputGetCapabilities = fpl__Win32XInputGetCapabilitiesStub;
	}
}

fpl_internal void fpl__Win32LoadXInputApi(fpl__Win32XInputApi *xinputApi) {
	FPL_ASSERT(xinputApi != fpl_null);

	FPL_LOG("XInput", "Load XInput Library");

	// Windows 8
	HMODULE xinputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!xinputLibrary) {
		// Windows 7
		xinputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (!xinputLibrary) {
		// Windows Generic
		xinputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if (xinputLibrary) {
		xinputApi->xinputLibrary = xinputLibrary;
		xinputApi->xInputGetState = (fpl__win32_func_XInputGetState *)GetProcAddress(xinputLibrary, "XInputGetState");
		xinputApi->xInputGetCapabilities = (fpl__win32_func_XInputGetCapabilities *)GetProcAddress(xinputLibrary, "XInputGetCapabilities");
	}
	if (xinputApi->xInputGetState == fpl_null) {
		xinputApi->xInputGetState = fpl__Win32XInputGetStateStub;
	}
	if (xinputApi->xInputGetCapabilities == fpl_null) {
		xinputApi->xInputGetCapabilities = fpl__Win32XInputGetCapabilitiesStub;
	}
}

//
// WINAPI functions
//

// GDI32
#define FPL__FUNC_CHOOSE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FPL__FUNC_CHOOSE_PIXEL_FORMAT(fpl__win32_func_ChoosePixelFormat);
#define FPL__FUNC_SET_PIXEL_FORMAT(name) BOOL WINAPI name(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FPL__FUNC_SET_PIXEL_FORMAT(fpl__win32_func_SetPixelFormat);
#define FPL__FUNC_DESCRIPE_PIXEL_FORMAT(name) int WINAPI name(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
typedef FPL__FUNC_DESCRIPE_PIXEL_FORMAT(fpl__win32_func_DescribePixelFormat);
#define FPL__FUNC_GET_DEVICE_CAPS(name) int WINAPI name(HDC hdc, int index)
typedef FPL__FUNC_GET_DEVICE_CAPS(fpl__win32_func_GetDeviceCaps);
#define FPL__FUNC_STRETCH_DIBITS(name) int WINAPI name(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, CONST VOID *lpBits, CONST BITMAPINFO *lpbmi, UINT iUsage, DWORD rop)
typedef FPL__FUNC_STRETCH_DIBITS(fpl__win32_func_StretchDIBits);
#define FPL__FUNC_DELETE_OBJECT(name) BOOL WINAPI name( _In_ HGDIOBJ ho)
typedef FPL__FUNC_DELETE_OBJECT(fpl__win32_func_DeleteObject);
#define FPL__FUNC_SWAP_BUFFERS(name) BOOL WINAPI name(HDC)
typedef FPL__FUNC_SWAP_BUFFERS(fpl__win32_func_SwapBuffers);

// ShellAPI
#define FPL__FUNC_COMMAND_LINE_TO_ARGV_W(name) LPWSTR* WINAPI name(LPCWSTR lpCmdLine, int *pNumArgs)
typedef FPL__FUNC_COMMAND_LINE_TO_ARGV_W(fpl__win32_func_CommandLineToArgvW);
#define FPL__FUNC_SH_GET_FOLDER_PATH_A(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
typedef FPL__FUNC_SH_GET_FOLDER_PATH_A(fpl__win32_func_SHGetFolderPathA);
#define FPL__FUNC_SH_GET_FOLDER_PATH_W(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
typedef FPL__FUNC_SH_GET_FOLDER_PATH_W(fpl__win32_func_SHGetFolderPathW);

// User32
#define FPL__FUNC_REGISTER_CLASS_EX_A(name) ATOM WINAPI name(CONST WNDCLASSEXA *)
typedef FPL__FUNC_REGISTER_CLASS_EX_A(fpl__win32_func_RegisterClassExA);
#define FPL__FUNC_REGISTER_CLASS_EX_W(name) ATOM WINAPI name(CONST WNDCLASSEXW *)
typedef FPL__FUNC_REGISTER_CLASS_EX_W(fpl__win32_func_RegisterClassExW);
#define FPL__FUNC_UNREGISTER_CLASS_EX_A(name) BOOL WINAPI name(LPCSTR lpClassName, HINSTANCE hInstance)
typedef FPL__FUNC_UNREGISTER_CLASS_EX_A(fpl__win32_func_UnregisterClassA);
#define FPL__FUNC_UNREGISTER_CLASS_EX_W(name) BOOL WINAPI name(LPCWSTR lpClassName, HINSTANCE hInstance)
typedef FPL__FUNC_UNREGISTER_CLASS_EX_W(fpl__win32_func_UnregisterClassW);
#define FPL__FUNC_SHOW_WINDOW(name) BOOL WINAPI name(HWND hWnd, int nCmdShow)
typedef FPL__FUNC_SHOW_WINDOW(fpl__win32_func_ShowWindow);
#define FPL__FUNC_DESTROY_WINDOW(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_DESTROY_WINDOW(fpl__win32_func_DestroyWindow);
#define FPL__FUNC_UPDATE_WINDOW(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_UPDATE_WINDOW(fpl__win32_func_UpdateWindow);
#define FPL__FUNC_TRANSLATE_MESSAGE(name) BOOL WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_TRANSLATE_MESSAGE(fpl__win32_func_TranslateMessage);
#define FPL__FUNC_DISPATCH_MESSAGE_A(name) LRESULT WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_DISPATCH_MESSAGE_A(fpl__win32_func_DispatchMessageA);
#define FPL__FUNC_DISPATCH_MESSAGE_W(name) LRESULT WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_DISPATCH_MESSAGE_W(fpl__win32_func_DispatchMessageW);
#define FPL__FUNC_PEEK_MESSAGE_A(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
typedef FPL__FUNC_PEEK_MESSAGE_A(fpl__win32_func_PeekMessageA);
#define FPL__FUNC_PEEK_MESSAGE_W(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
typedef FPL__FUNC_PEEK_MESSAGE_W(fpl__win32_func_PeekMessageW);
#define FPL__FUNC_DEF_WINDOW_PROC_A(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_DEF_WINDOW_PROC_A(fpl__win32_func_DefWindowProcA);
#define FPL__FUNC_DEF_WINDOW_PROC_W(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_DEF_WINDOW_PROC_W(fpl__win32_func_DefWindowProcW);
#define FPL__FUNC_CREATE_WINDOW_EX_W(name) HWND WINAPI name(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
typedef FPL__FUNC_CREATE_WINDOW_EX_W(fpl__win32_func_CreateWindowExW);
#define FPL__FUNC_CREATE_WINDOW_EX_A(name) HWND WINAPI name(DWORD dwExStyle, LPCSTR lpClassName, PCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
typedef FPL__FUNC_CREATE_WINDOW_EX_A(fpl__win32_func_CreateWindowExA);
#define FPL__FUNC_SET_WINDOW_POS(name) BOOL WINAPI name(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
typedef FPL__FUNC_SET_WINDOW_POS(fpl__win32_func_SetWindowPos);
#define FPL__FUNC_GET_WINDOW_PLACEMENT(name) BOOL WINAPI name(HWND hWnd, WINDOWPLACEMENT *lpwndpl)
typedef FPL__FUNC_GET_WINDOW_PLACEMENT(fpl__win32_func_GetWindowPlacement);
#define FPL__FUNC_SET_WINDOW_PLACEMENT(name) BOOL WINAPI name(HWND hWnd, CONST WINDOWPLACEMENT *lpwndpl)
typedef FPL__FUNC_SET_WINDOW_PLACEMENT(fpl__win32_func_SetWindowPlacement);
#define FPL__FUNC_GET_CLIENT_RECT(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
typedef FPL__FUNC_GET_CLIENT_RECT(fpl__win32_func_GetClientRect);
#define FPL__FUNC_GET_WINDOW_RECT(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
typedef FPL__FUNC_GET_WINDOW_RECT(fpl__win32_func_GetWindowRect);
#define FPL__FUNC_ADJUST_WINDOW_RECT(name) BOOL WINAPI name(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
typedef FPL__FUNC_ADJUST_WINDOW_RECT(fpl__win32_func_AdjustWindowRect);
#define FPL__FUNC_GET_ASYNC_KEY_STATE(name) SHORT WINAPI name(int vKey)
typedef FPL__FUNC_GET_ASYNC_KEY_STATE(fpl__win32_func_GetAsyncKeyState);
#define FPL__FUNC_MAP_VIRTUAL_KEY_A(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_MAP_VIRTUAL_KEY_A(fpl__win32_func_MapVirtualKeyA);
#define FPL__FUNC_MAP_VIRTUAL_KEY_W(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_MAP_VIRTUAL_KEY_W(fpl__win32_func_MapVirtualKeyW);
#define FPL__FUNC_SET_CURSOR(name) HCURSOR WINAPI name(HCURSOR hCursor)
typedef FPL__FUNC_SET_CURSOR(fpl__win32_func_SetCursor);
#define FPL__FUNC_GET_CURSOR(name) HCURSOR WINAPI name(VOID)
typedef FPL__FUNC_GET_CURSOR(fpl__win32_func_GetCursor);
#define FPL__FUNC_LOAD_CURSOR_A(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCSTR lpCursorName)
typedef FPL__FUNC_LOAD_CURSOR_A(fpl__win32_func_LoadCursorA);
#define FPL__FUNC_LOAD_CURSOR_W(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCWSTR lpCursorName)
typedef FPL__FUNC_LOAD_CURSOR_W(fpl__win32_func_LoadCursorW);
#define FPL__FUNC_LOAD_ICON_A(name) HICON WINAPI name(HINSTANCE hInstance, LPCSTR lpIconName)
typedef FPL__FUNC_LOAD_ICON_A(fpl__win32_func_LoadIconA);
#define FPL__FUNC_LOAD_ICON_W(name) HICON WINAPI name(HINSTANCE hInstance, LPCWSTR lpIconName)
typedef FPL__FUNC_LOAD_ICON_W(fpl__win32_func_LoadIconW);
#define FPL__FUNC_SET_WINDOW_TEXT_A(name) BOOL WINAPI name(HWND hWnd, LPCSTR lpString)
typedef FPL__FUNC_SET_WINDOW_TEXT_A(fpl__win32_func_SetWindowTextA);
#define FPL__FUNC_SET_WINDOW_TEXT_W(name) BOOL WINAPI name(HWND hWnd, LPCWSTR lpString)
typedef FPL__FUNC_SET_WINDOW_TEXT_W(fpl__win32_func_SetWindowTextW);
#define FPL__FUNC_SET_WINDOW_LONG_A(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
typedef FPL__FUNC_SET_WINDOW_LONG_A(fpl__win32_func_SetWindowLongA);
#define FPL__FUNC_SET_WINDOW_LONG_W(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
typedef FPL__FUNC_SET_WINDOW_LONG_W(fpl__win32_func_SetWindowLongW);
#define FPL__FUNC_GET_WINDOW_LONG_A(name) LONG WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_GET_WINDOW_LONG_A(fpl__win32_func_GetWindowLongA);
#define FPL__FUNC_GET_WINDOW_LONG_W(name) LONG WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_GET_WINDOW_LONG_W(fpl__win32_func_GetWindowLongW);

#if defined(FPL_ARCH_X64)
#define FPL__FUNC_SET_WINDOW_LONG_PTR_A(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
typedef FPL__FUNC_SET_WINDOW_LONG_PTR_A(fpl__win32_func_SetWindowLongPtrA);
#define FPL__FUNC_SET_WINDOW_LONG_PTR_W(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
typedef FPL__FUNC_SET_WINDOW_LONG_PTR_W(fpl__win32_func_SetWindowLongPtrW);
#define FPL__FUNC_GET_WINDOW_LONG_PTR_A(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_GET_WINDOW_LONG_PTR_A(fpl__win32_func_GetWindowLongPtrA);
#define FPL__FUNC_GET_WINDOW_LONG_PTR_W(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_GET_WINDOW_LONG_PTR_W(fpl__win32_func_GetWindowLongPtrW);
#endif

#define FPL__FUNC_RELEASE_DC(name) int WINAPI name(HWND hWnd, HDC hDC)
typedef FPL__FUNC_RELEASE_DC(fpl__win32_func_ReleaseDC);
#define FPL__FUNC_GET_DC(name) HDC WINAPI name(HWND hWnd)
typedef FPL__FUNC_GET_DC(fpl__win32_func_GetDC);
#define FPL__FUNC_CHANGE_DISPLAY_SETTINGS_A(name) LONG WINAPI name(DEVMODEA* lpDevMode, DWORD dwFlags)
typedef FPL__FUNC_CHANGE_DISPLAY_SETTINGS_A(fpl__win32_func_ChangeDisplaySettingsA);
#define FPL__FUNC_CHANGE_DISPLAY_SETTINGS_W(name) LONG WINAPI name(DEVMODEW* lpDevMode, DWORD dwFlags)
typedef FPL__FUNC_CHANGE_DISPLAY_SETTINGS_W(fpl__win32_func_ChangeDisplaySettingsW);
#define FPL__FUNC_ENUM_DISPLAY_SETTINGS_A(name) BOOL WINAPI name(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode)
typedef FPL__FUNC_ENUM_DISPLAY_SETTINGS_A(fpl__win32_func_EnumDisplaySettingsA);
#define FPL__FUNC_ENUM_DISPLAY_SETTINGS_W(name) BOOL WINAPI name(LPCWSTR lpszDeviceName, DWORD iModeNum, DEVMODEW* lpDevMode)
typedef FPL__FUNC_ENUM_DISPLAY_SETTINGS_W(fpl__win32_func_EnumDisplaySettingsW);
#define FPL__FUNC_OPEN_CLIPBOARD(name) BOOL WINAPI name(HWND hWndNewOwner)
typedef FPL__FUNC_OPEN_CLIPBOARD(fpl__win32_func_OpenClipboard);
#define FPL__FUNC_CLOSE_CLIPBOARD(name) BOOL WINAPI name(VOID)
typedef FPL__FUNC_CLOSE_CLIPBOARD(fpl__win32_func_CloseClipboard);
#define FPL__FUNC_EMPTY_CLIPBOARD(name) BOOL WINAPI name(VOID)
typedef FPL__FUNC_EMPTY_CLIPBOARD(fpl__win32_func_EmptyClipboard);
#define FPL__FUNC_IS_CLIPBOARD_FORMAT_AVAILABLE(name) BOOL WINAPI name(UINT format)
typedef FPL__FUNC_IS_CLIPBOARD_FORMAT_AVAILABLE(fpl__win32_func_IsClipboardFormatAvailable);
#define FPL__FUNC_SET_CLIPBOARD_DATA(name) HANDLE WINAPI name(UINT uFormat, HANDLE hMem)
typedef FPL__FUNC_SET_CLIPBOARD_DATA(fpl__win32_func_SetClipboardData);
#define FPL__FUNC_GET_CLIPBOARD_DATA(name) HANDLE WINAPI name(UINT uFormat)
typedef FPL__FUNC_GET_CLIPBOARD_DATA(fpl__win32_func_GetClipboardData);
#define FPL__FUNC_GET_DESKTOP_WINDOW(name) HWND WINAPI name(VOID)
typedef FPL__FUNC_GET_DESKTOP_WINDOW(fpl__win32_func_GetDesktopWindow);
#define FPL__FUNC_GET_FOREGROUND_WINDOW(name) HWND WINAPI name(VOID)
typedef FPL__FUNC_GET_FOREGROUND_WINDOW(fpl__win32_func_GetForegroundWindow);
#define FPL__FUNC_IS_ZOOMED(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_IS_ZOOMED(fpl__win32_func_IsZoomed);
#define FPL__FUNC_SEND_MESSAGE_A(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_SEND_MESSAGE_A(fpl__win32_func_SendMessageA);
#define FPL__FUNC_SEND_MESSAGE_W(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_SEND_MESSAGE_W(fpl__win32_func_SendMessageW);
#define FPL__FUNC_GET_MONITOR_INFO_A(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
typedef FPL__FUNC_GET_MONITOR_INFO_A(fpl__win32_func_GetMonitorInfoA);
#define FPL__FUNC_GET_MONITOR_INFO_W(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
typedef FPL__FUNC_GET_MONITOR_INFO_W(fpl__win32_func_GetMonitorInfoW);
#define FPL__FUNC_MONITOR_FROM_RECT(name) HMONITOR WINAPI name(LPCRECT lprc, DWORD dwFlags)
typedef FPL__FUNC_MONITOR_FROM_RECT(fpl__win32_func_MonitorFromRect);
#define FPL__FUNC_MONITOR_FROM_WINDOW(name) HMONITOR WINAPI name(HWND hwnd, DWORD dwFlags)
typedef FPL__FUNC_MONITOR_FROM_WINDOW(fpl__win32_func_MonitorFromWindow);

// OLE32
#define FPL__FUNC_WIN32_CO_INITIALIZE_EX(name) HRESULT WINAPI name(LPVOID pvReserved, DWORD  dwCoInit)
typedef FPL__FUNC_WIN32_CO_INITIALIZE_EX(fpl__win32_func_CoInitializeEx);
#define FPL__FUNC_WIN32_CO_UNINITIALIZE(name) void WINAPI name(void)
typedef FPL__FUNC_WIN32_CO_UNINITIALIZE(fpl__win32_func_CoUninitialize);
#define FPL__FUNC_WIN32_CO_CREATE_INSTANCE(name) HRESULT WINAPI name(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
typedef FPL__FUNC_WIN32_CO_CREATE_INSTANCE(fpl__win32_func_CoCreateInstance);
#define FPL__FUNC_WIN32_CO_TASK_MEM_FREE(name) void WINAPI name(LPVOID pv)
typedef FPL__FUNC_WIN32_CO_TASK_MEM_FREE(fpl__win32_func_CoTaskMemFree);
#define FPL__FUNC_WIN32_PROP_VARIANT_CLEAR(name) HRESULT WINAPI name(PROPVARIANT *pvar)
typedef FPL__FUNC_WIN32_PROP_VARIANT_CLEAR(fpl__win32_func_PropVariantClear);

typedef struct fpl__Win32GdiApi {
	HMODULE gdiLibrary;
	fpl__win32_func_ChoosePixelFormat *choosePixelFormat;
	fpl__win32_func_SetPixelFormat *setPixelFormat;
	fpl__win32_func_DescribePixelFormat *describePixelFormat;
	fpl__win32_func_GetDeviceCaps *getDeviceCaps;
	fpl__win32_func_StretchDIBits *stretchDIBits;
	fpl__win32_func_DeleteObject *deleteObject;
	fpl__win32_func_SwapBuffers *swapBuffers;
} fpl__Win32GdiApi;

typedef struct fpl__Win32ShellApi {
	HMODULE shellLibrary;
	fpl__win32_func_CommandLineToArgvW *commandLineToArgvW;
	fpl__win32_func_SHGetFolderPathA *shGetFolderPathA;
	fpl__win32_func_SHGetFolderPathW *shGetFolderPathW;
} fpl__Win32ShellApi;

typedef struct fpl__Win32UserApi {
	HMODULE userLibrary;
	fpl__win32_func_RegisterClassExA *registerClassExA;
	fpl__win32_func_RegisterClassExW *registerClassExW;
	fpl__win32_func_UnregisterClassA *unregisterClassA;
	fpl__win32_func_UnregisterClassW *unregisterClassW;
	fpl__win32_func_ShowWindow *showWindow;
	fpl__win32_func_DestroyWindow *destroyWindow;
	fpl__win32_func_UpdateWindow *updateWindow;
	fpl__win32_func_TranslateMessage *translateMessage;
	fpl__win32_func_DispatchMessageA *dispatchMessageA;
	fpl__win32_func_DispatchMessageW *dispatchMessageW;
	fpl__win32_func_PeekMessageA *peekMessageA;
	fpl__win32_func_PeekMessageW *peekMessageW;
	fpl__win32_func_DefWindowProcA *defWindowProcA;
	fpl__win32_func_DefWindowProcW *defWindowProcW;
	fpl__win32_func_CreateWindowExA *createWindowExA;
	fpl__win32_func_CreateWindowExW *createWindowExW;
	fpl__win32_func_SetWindowPos *setWindowPos;
	fpl__win32_func_GetWindowPlacement *getWindowPlacement;
	fpl__win32_func_SetWindowPlacement *setWindowPlacement;
	fpl__win32_func_GetClientRect *getClientRect;
	fpl__win32_func_GetWindowRect *getWindowRect;
	fpl__win32_func_AdjustWindowRect *adjustWindowRect;
	fpl__win32_func_GetAsyncKeyState *getAsyncKeyState;
	fpl__win32_func_MapVirtualKeyA *mapVirtualKeyA;
	fpl__win32_func_MapVirtualKeyW *mapVirtualKeyW;
	fpl__win32_func_SetCursor *setCursor;
	fpl__win32_func_GetCursor *getCursor;
	fpl__win32_func_LoadCursorA *loadCursorA;
	fpl__win32_func_LoadCursorW *loadCursorW;
	fpl__win32_func_LoadIconA *loadIconA;
	fpl__win32_func_LoadIconW *loadIconW;
	fpl__win32_func_SetWindowTextA *setWindowTextA;
	fpl__win32_func_SetWindowTextW *setWindowTextW;
	fpl__win32_func_SetWindowLongA *setWindowLongA;
	fpl__win32_func_SetWindowLongW *setWindowLongW;
	fpl__win32_func_GetWindowLongA *getWindowLongA;
	fpl__win32_func_GetWindowLongW *getWindowLongW;
#if defined(FPL_ARCH_X64)
	fpl__win32_func_SetWindowLongPtrA *setWindowLongPtrA;
	fpl__win32_func_SetWindowLongPtrW *setWindowLongPtrW;
	fpl__win32_func_GetWindowLongPtrA *getWindowLongPtrA;
	fpl__win32_func_GetWindowLongPtrW *getWindowLongPtrW;
#endif
	fpl__win32_func_ReleaseDC *releaseDC;
	fpl__win32_func_GetDC *getDC;
	fpl__win32_func_ChangeDisplaySettingsA *changeDisplaySettingsA;
	fpl__win32_func_ChangeDisplaySettingsW *changeDisplaySettingsW;
	fpl__win32_func_EnumDisplaySettingsA *enumDisplaySettingsA;
	fpl__win32_func_EnumDisplaySettingsW *enumDisplaySettingsW;

	fpl__win32_func_OpenClipboard *openClipboard;
	fpl__win32_func_CloseClipboard *closeClipboard;
	fpl__win32_func_EmptyClipboard *emptyClipboard;
	fpl__win32_func_IsClipboardFormatAvailable *isClipboardFormatAvailable;
	fpl__win32_func_SetClipboardData *setClipboardData;
	fpl__win32_func_GetClipboardData *getClipboardData;

	fpl__win32_func_GetDesktopWindow *getDesktopWindow;
	fpl__win32_func_GetForegroundWindow *getForegroundWindow;
	fpl__win32_func_IsZoomed *isZoomed;
	fpl__win32_func_SendMessageA *sendMessageA;
	fpl__win32_func_SendMessageW *sendMessageW;
	fpl__win32_func_GetMonitorInfoA *getMonitorInfoA;
	fpl__win32_func_GetMonitorInfoW *getMonitorInfoW;
	fpl__win32_func_MonitorFromRect *monitorFromRect;
	fpl__win32_func_MonitorFromWindow *monitorFromWindow;
} fpl__Win32UserApi;

typedef struct fpl__Win32OleApi {
	HMODULE oleLibrary;
	fpl__win32_func_CoInitializeEx *coInitializeEx;
	fpl__win32_func_CoUninitialize *coUninitialize;
	fpl__win32_func_CoCreateInstance *coCreateInstance;
	fpl__win32_func_CoTaskMemFree *coTaskMemFree;
	fpl__win32_func_PropVariantClear *propVariantClear;
} fpl__Win32OleApi;

typedef struct fpl__Win32Api {
	fpl__Win32GdiApi gdi;
	fpl__Win32ShellApi shell;
	fpl__Win32UserApi user;
	fpl__Win32OleApi ole;
} fpl__Win32Api;

fpl_internal void fpl__Win32UnloadApi(fpl__Win32Api *wapi) {
	FPL_ASSERT(wapi != fpl_null);
	if (wapi->ole.oleLibrary != fpl_null) {
		FreeLibrary(wapi->ole.oleLibrary);
		FPL_CLEAR_STRUCT(&wapi->ole);
	}
	if (wapi->gdi.gdiLibrary != fpl_null) {
		FreeLibrary(wapi->gdi.gdiLibrary);
		FPL_CLEAR_STRUCT(&wapi->gdi);
	}
	if (wapi->user.userLibrary != fpl_null) {
		FreeLibrary(wapi->user.userLibrary);
		FPL_CLEAR_STRUCT(&wapi->user);
	}
	if (wapi->shell.shellLibrary != fpl_null) {
		FreeLibrary(wapi->shell.shellLibrary);
		FPL_CLEAR_STRUCT(&wapi->shell);
	}
}

fpl_internal bool fpl__Win32LoadApi(fpl__Win32Api *wapi) {
	FPL_ASSERT(wapi != fpl_null);

	// Shell32
	{
		const char *shellLibraryName = "shell32.dll";
		HMODULE library = wapi->shell.shellLibrary = LoadLibraryA(shellLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", shellLibraryName);
			return false;
		}
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.commandLineToArgvW, fpl__win32_func_CommandLineToArgvW, "CommandLineToArgvW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.shGetFolderPathA, fpl__win32_func_SHGetFolderPathA, "SHGetFolderPathA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.shGetFolderPathW, fpl__win32_func_SHGetFolderPathW, "SHGetFolderPathW");
	}

	// User32
	{
		const char *userLibraryName = "user32.dll";
		HMODULE library = wapi->user.userLibrary = LoadLibraryA(userLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", userLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.registerClassExA, fpl__win32_func_RegisterClassExA, "RegisterClassExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.registerClassExW, fpl__win32_func_RegisterClassExW, "RegisterClassExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.unregisterClassA, fpl__win32_func_UnregisterClassA, "UnregisterClassA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.unregisterClassW, fpl__win32_func_UnregisterClassW, "UnregisterClassW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.showWindow, fpl__win32_func_ShowWindow, "ShowWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.destroyWindow, fpl__win32_func_DestroyWindow, "DestroyWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.updateWindow, fpl__win32_func_UpdateWindow, "UpdateWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.translateMessage, fpl__win32_func_TranslateMessage, "TranslateMessage");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.dispatchMessageA, fpl__win32_func_DispatchMessageA, "DispatchMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.dispatchMessageW, fpl__win32_func_DispatchMessageW, "DispatchMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.peekMessageA, fpl__win32_func_PeekMessageA, "PeekMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.peekMessageW, fpl__win32_func_PeekMessageW, "PeekMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.defWindowProcA, fpl__win32_func_DefWindowProcA, "DefWindowProcA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.defWindowProcW, fpl__win32_func_DefWindowProcW, "DefWindowProcW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.createWindowExA, fpl__win32_func_CreateWindowExA, "CreateWindowExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.createWindowExW, fpl__win32_func_CreateWindowExW, "CreateWindowExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowPos, fpl__win32_func_SetWindowPos, "SetWindowPos");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowPlacement, fpl__win32_func_GetWindowPlacement, "GetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowPlacement, fpl__win32_func_SetWindowPlacement, "SetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getClientRect, fpl__win32_func_GetClientRect, "GetClientRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowRect, fpl__win32_func_GetWindowRect, "GetWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.adjustWindowRect, fpl__win32_func_AdjustWindowRect, "AdjustWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getAsyncKeyState, fpl__win32_func_GetAsyncKeyState, "GetAsyncKeyState");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.mapVirtualKeyA, fpl__win32_func_MapVirtualKeyA, "MapVirtualKeyA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.mapVirtualKeyW, fpl__win32_func_MapVirtualKeyW, "MapVirtualKeyW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setCursor, fpl__win32_func_SetCursor, "SetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getCursor, fpl__win32_func_GetCursor, "GetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.loadCursorA, fpl__win32_func_LoadCursorA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.loadCursorW, fpl__win32_func_LoadCursorW, "LoadCursorW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.loadIconA, fpl__win32_func_LoadIconA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.loadIconW, fpl__win32_func_LoadIconW, "LoadIconW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowTextA, fpl__win32_func_SetWindowTextA, "SetWindowTextA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowTextW, fpl__win32_func_SetWindowTextW, "SetWindowTextW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowLongA, fpl__win32_func_SetWindowLongA, "SetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowLongW, fpl__win32_func_SetWindowLongW, "SetWindowLongW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowLongA, fpl__win32_func_GetWindowLongA, "GetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowLongW, fpl__win32_func_GetWindowLongW, "GetWindowLongW");

#				if defined(FPL_ARCH_X64)
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowLongPtrA, fpl__win32_func_SetWindowLongPtrA, "SetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setWindowLongPtrW, fpl__win32_func_SetWindowLongPtrW, "SetWindowLongPtrW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowLongPtrA, fpl__win32_func_GetWindowLongPtrA, "GetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getWindowLongPtrW, fpl__win32_func_GetWindowLongPtrW, "GetWindowLongPtrW");
#				endif

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.releaseDC, fpl__win32_func_ReleaseDC, "ReleaseDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getDC, fpl__win32_func_GetDC, "GetDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.changeDisplaySettingsA, fpl__win32_func_ChangeDisplaySettingsA, "ChangeDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.changeDisplaySettingsW, fpl__win32_func_ChangeDisplaySettingsW, "ChangeDisplaySettingsW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.enumDisplaySettingsA, fpl__win32_func_EnumDisplaySettingsA, "EnumDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.enumDisplaySettingsW, fpl__win32_func_EnumDisplaySettingsW, "EnumDisplaySettingsW");

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.openClipboard, fpl__win32_func_OpenClipboard, "OpenClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.closeClipboard, fpl__win32_func_CloseClipboard, "CloseClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.emptyClipboard, fpl__win32_func_EmptyClipboard, "EmptyClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.setClipboardData, fpl__win32_func_SetClipboardData, "SetClipboardData");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getClipboardData, fpl__win32_func_GetClipboardData, "GetClipboardData");

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getDesktopWindow, fpl__win32_func_GetDesktopWindow, "GetDesktopWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getForegroundWindow, fpl__win32_func_GetForegroundWindow, "GetForegroundWindow");

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.isZoomed, fpl__win32_func_IsZoomed, "IsZoomed");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.sendMessageA, fpl__win32_func_SendMessageA, "SendMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.sendMessageW, fpl__win32_func_SendMessageW, "SendMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getMonitorInfoA, fpl__win32_func_GetMonitorInfoA, "GetMonitorInfoA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.getMonitorInfoW, fpl__win32_func_GetMonitorInfoW, "GetMonitorInfoW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.monitorFromRect, fpl__win32_func_MonitorFromRect, "MonitorFromRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.monitorFromWindow, fpl__win32_func_MonitorFromWindow, "MonitorFromWindow");
	}

	// GDI32
	{
		const char *gdiLibraryName = "gdi32.dll";
		HMODULE library = wapi->gdi.gdiLibrary = LoadLibraryA(gdiLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", gdiLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.choosePixelFormat, fpl__win32_func_ChoosePixelFormat, "ChoosePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.setPixelFormat, fpl__win32_func_SetPixelFormat, "SetPixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.describePixelFormat, fpl__win32_func_DescribePixelFormat, "DescribePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.stretchDIBits, fpl__win32_func_StretchDIBits, "StretchDIBits");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.deleteObject, fpl__win32_func_DeleteObject, "DeleteObject");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.swapBuffers, fpl__win32_func_SwapBuffers, "SwapBuffers");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.getDeviceCaps, fpl__win32_func_GetDeviceCaps, "GetDeviceCaps");
	}

	// OLE32
	{
		const char *oleLibraryName = "ole32.dll";
		HMODULE library = wapi->ole.oleLibrary = LoadLibraryA(oleLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", oleLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.coInitializeEx, fpl__win32_func_CoInitializeEx, "CoInitializeEx");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.coUninitialize, fpl__win32_func_CoUninitialize, "CoUninitialize");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.coCreateInstance, fpl__win32_func_CoCreateInstance, "CoCreateInstance");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.coTaskMemFree, fpl__win32_func_CoTaskMemFree, "CoTaskMemFree");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.propVariantClear, fpl__win32_func_PropVariantClear, "PropVariantClear");
	}

	return true;
}

// Unicode dependent function calls and types
#if !defined(UNICODE)
#	define FPL_WIN32_CLASSNAME "FPLWindowClassA"
#	define FPL_WIN32_UNNAMED_WINDOW "Unnamed FPL Ansi Window"
#	define win32_wndclassex WNDCLASSEXA
#	if defined(FPL_ARCH_X64)
#		define win32_setWindowLongPtr fpl__global__AppState->win32.winApi.user.setWindowLongPtrA
#	else
#		define win32_setWindowLongPtr fpl__global__AppState->win32.winApi.user.setWindowLongA
#	endif
#	define win32_setWindowLong fpl__global__AppState->win32.winApi.user.setWindowLongA
#	define win32_getWindowLong fpl__global__AppState->win32.winApi.user.getWindowLongA
#	define win32_peekMessage fpl__global__AppState->win32.winApi.user.peekMessageA
#	define win32_dispatchMessage fpl__global__AppState->win32.winApi.user.dispatchMessageA
#	define win32_defWindowProc fpl__global__AppState->win32.winApi.user.defWindowProcA
#	define win32_registerClassEx fpl__global__AppState->win32.winApi.user.registerClassExA
#	define win32_unregisterClass fpl__global__AppState->win32.winApi.user.unregisterClassA
#	define win32_createWindowEx fpl__global__AppState->win32.winApi.user.createWindowExA
#	define win32_loadIcon fpl__global__AppState->win32.winApi.user.loadIconA
#	define win32_loadCursor fpl__global__AppState->win32.winApi.user.loadCursorA
#	define win32_sendMessage fpl__global__AppState->win32.winApi.user.sendMessageA
#	define win32_getMonitorInfo fpl__global__AppState->win32.winApi.user.getMonitorInfoA
#else
#	define FPL_WIN32_CLASSNAME L"FPLWindowClassW"
#	define FPL_WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
#	define win32_wndclassex WNDCLASSEXW
#	if defined(FPL_ARCH_X64)
#		define win32_setWindowLongPtr fpl__global__AppState->win32.winApi.user.setWindowLongPtrW
#	else
#		define win32_setWindowLongPtr fpl__global__AppState->win32.winApi.user.setWindowLongW
#	endif
#	define win32_setWindowLong fpl__global__AppState->win32.winApi.user.setWindowLongW
#	define win32_getWindowLong fpl__global__AppState->win32.winApi.user.getWindowLongW
#	define win32_peekMessage fpl__global__AppState->win32.winApi.user.peekMessageW
#	define win32_dispatchMessage fpl__global__AppState->win32.winApi.user.dispatchMessageW
#	define win32_defWindowProc fpl__global__AppState->win32.winApi.user.defWindowProcW
#	define win32_registerClassEx fpl__global__AppState->win32.winApi.user.registerClassExW
#	define win32_unregisterClass fpl__global__AppState->win32.winApi.user.unregisterClassW
#	define win32_createWindowEx fpl__global__AppState->win32.winApi.user.createWindowExW
#	define win32_loadIcon fpl__global__AppState->win32.winApi.user.loadIconW
#	define win32_loadCursor fpl__global__AppState->win32.winApi.user.loadCursorW
#	define win32_sendMessage fpl__global__AppState->win32.winApi.user.sendMessageW
#	define win32_getMonitorInfo fpl__global__AppState->win32.winApi.user.getMonitorInfoW
#endif // UNICODE

typedef struct fpl__Win32XInputState {
	bool isConnected[XUSER_MAX_COUNT];
	LARGE_INTEGER lastDeviceSearchTime;
	fpl__Win32XInputApi xinputApi;
} fpl__Win32XInputState;

typedef struct fpl__Win32InitState {
	HINSTANCE appInstance;
	LARGE_INTEGER performanceFrequency;
} fpl__Win32InitState;

typedef struct fpl__Win32AppState {
	fpl__Win32XInputState xinput;
	fpl__Win32Api winApi;
} fpl__Win32AppState;

#if defined(FPL_ENABLE_WINDOW)
typedef struct fpl__Win32LastWindowInfo {
	WINDOWPLACEMENT placement;
	DWORD style;
	DWORD exStyle;
	bool isMaximized;
	bool wasResolutionChanged;
} fpl__Win32LastWindowInfo;

typedef struct fpl__Win32WindowState {
#if _UNICODE
	wchar_t windowClass[256];
#else
	char windowClass[256];
#endif
	HWND windowHandle;
	HDC deviceContext;
	HCURSOR defaultCursor;
	fpl__Win32LastWindowInfo lastFullscreenInfo;
	bool isRunning;
	bool isCursorActive;
} fpl__Win32WindowState;
#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ############################################################################
//
// > TYPES_POSIX
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_POSIX)
//#	include <sys/mman.h> // mmap, munmap
//#	include <sys/types.h> // data types
//#	include <sys/stat.h> // mkdir
//#	include <sys/errno.h> // errno
//#	include <signal.h> // pthread_kill
//#	include <time.h> // clock_gettime, nanosleep
//#	include <dlfcn.h> // dlopen, dlclose
//#	include <fcntl.h> // open
//#	include <unistd.h> // read, write, close, access, rmdir

// Little macro to not write 5 lines of code all the time
#define FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, target, type, name) \
	target = (type *)dlsym(libHandle, name); \
	if (target == fpl_null) { \
		fpl__PushError("Failed getting '%s' from library '%s'", name, libName); \
		break; \
	}

#define FPL__FUNC_PTHREAD_CREATE(name) int name(pthread_t *, const pthread_attr_t *, void *(*__start_routine) (void *), void *)
typedef FPL__FUNC_PTHREAD_CREATE(fpl__pthread_func_pthread_create);
#define FPL__FUNC_PTHREAD_KILL(name) int name(pthread_t thread, int sig)
typedef FPL__FUNC_PTHREAD_KILL(fpl__pthread_func_pthread_kill);
#define FPL__FUNC_PTHREAD_JOIN(name) int name(pthread_t __th, void **__thread_return)
typedef FPL__FUNC_PTHREAD_JOIN(fpl__pthread_func_pthread_join);
#define FPL__FUNC_PTHREAD_EXIT(name) void name(void *__retval)
typedef FPL__FUNC_PTHREAD_EXIT(fpl__pthread_func_pthread_exit);
#define FPL__FUNC_PTHREAD_YIELD(name) int name(void)
typedef FPL__FUNC_PTHREAD_YIELD(fpl__pthread_func_pthread_yield);

#define FPL__FUNC_PTHREAD_MUTEX_INIT(name) int name(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
typedef FPL__FUNC_PTHREAD_MUTEX_INIT(fpl__pthread_func_pthread_mutex_init);
#define FPL__FUNC_PTHREAD_MUTEX_DESTROY(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_MUTEX_DESTROY(fpl__pthread_func_pthread_mutex_destroy);
#define FPL__FUNC_PTHREAD_MUTEX_LOCK(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_MUTEX_LOCK(fpl__pthread_func_pthread_mutex_lock);
#define FPL__FUNC_PTHREAD_MUTEX_TRYLOCK(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_MUTEX_TRYLOCK(fpl__pthread_func_pthread_mutex_trylock);
#define FPL__FUNC_PTHREAD_MUTEX_UNLOCK(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_MUTEX_UNLOCK(fpl__pthread_func_pthread_mutex_unlock);

#define FPL__FUNC_PTHREAD_COND_INIT(name) int name(pthread_cond_t *cond, const pthread_condattr_t *attr)
typedef FPL__FUNC_PTHREAD_COND_INIT(fpl__pthread_func_pthread_cond_init);
#define FPL__FUNC_PTHREAD_COND_DESTROY(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_COND_DESTROY(fpl__pthread_func_pthread_cond_destroy);
#define FPL__FUNC_PTHREAD_COND_TIMEDWAIT(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
typedef FPL__FUNC_PTHREAD_COND_TIMEDWAIT(fpl__pthread_func_pthread_cond_timedwait);
#define FPL__FUNC_PTHREAD_COND_WAIT(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_COND_WAIT(fpl__pthread_func_pthread_cond_wait);
#define FPL__FUNC_PTHREAD_COND_BROADCAST(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_COND_BROADCAST(fpl__pthread_func_pthread_cond_broadcast);
#define FPL__FUNC_PTHREAD_COND_SIGNAL(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_COND_SIGNAL(fpl__pthread_func_pthread_cond_signal);

typedef struct {
	void *libHandle;
	fpl__pthread_func_pthread_create *pthread_create;
	fpl__pthread_func_pthread_kill *pthread_kill;
	fpl__pthread_func_pthread_join *pthread_join;
	fpl__pthread_func_pthread_exit *pthread_exit;
	fpl__pthread_func_pthread_yield *pthread_yield;

	fpl__pthread_func_pthread_mutex_init *pthread_mutex_init;
	fpl__pthread_func_pthread_mutex_destroy *pthread_mutex_destroy;
	fpl__pthread_func_pthread_mutex_lock *pthread_mutex_lock;
	fpl__pthread_func_pthread_mutex_trylock *pthread_mutex_trylock;
	fpl__pthread_func_pthread_mutex_unlock *pthread_mutex_unlock;

	fpl__pthread_func_pthread_cond_init *pthread_cond_init;
	fpl__pthread_func_pthread_cond_destroy *pthread_cond_destroy;
	fpl__pthread_func_pthread_cond_timedwait *pthread_cond_timedwait;
	fpl__pthread_func_pthread_cond_wait *pthread_cond_wait;
	fpl__pthread_func_pthread_cond_broadcast *pthread_cond_broadcast;
	fpl__pthread_func_pthread_cond_signal *pthread_cond_signal;
} fpl__PThreadApi;

#define FPL__POSIX_DL_LOADTYPE RTLD_NOW

fpl_internal void fpl__PThreadApiUnloadPThreadApi(fpl__PThreadApi *pthreadApi) {
	FPL_ASSERT(pthreadApi != fpl_null);
	if (pthreadApi->libHandle != fpl_null) {
		dlclose(pthreadApi->libHandle);
	}
	FPL_CLEAR_STRUCT(pthreadApi);
}

fpl_internal bool fpl__PThreadApiLoadPThreadApi(fpl__PThreadApi *pthreadApi) {
	const char* libpthreadFileNames[] = {
		"libpthread.so",
		"libpthread.so.0",
	};
	bool result = false;
	for (uint32_t index = 0; index < FPL_ARRAYCOUNT(libpthreadFileNames); ++index) {
		const char * libName = libpthreadFileNames[index];
		void *libHandle = pthreadApi->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if (libHandle != fpl_null) {
			// pthread_t
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_create, fpl__pthread_func_pthread_create, "pthread_create");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_kill, fpl__pthread_func_pthread_kill, "pthread_kill");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_join, fpl__pthread_func_pthread_join, "pthread_join");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_exit, fpl__pthread_func_pthread_exit, "pthread_exit");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_yield, fpl__pthread_func_pthread_yield, "pthread_yield");

			// pthread_mutex_t
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_mutex_init, fpl__pthread_func_pthread_mutex_init, "pthread_mutex_init");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_mutex_destroy, fpl__pthread_func_pthread_mutex_destroy, "pthread_mutex_destroy");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_mutex_lock, fpl__pthread_func_pthread_mutex_lock, "pthread_mutex_lock");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_mutex_trylock, fpl__pthread_func_pthread_mutex_trylock, "pthread_mutex_trylock");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_mutex_unlock, fpl__pthread_func_pthread_mutex_unlock, "pthread_mutex_unlock");

			// pthread_cond_t
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_init, fpl__pthread_func_pthread_cond_init, "pthread_cond_init");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_destroy, fpl__pthread_func_pthread_cond_destroy, "pthread_cond_destroy");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_timedwait, fpl__pthread_func_pthread_cond_timedwait, "pthread_cond_timedwait");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_wait, fpl__pthread_func_pthread_cond_wait, "pthread_cond_wait");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_broadcast, fpl__pthread_func_pthread_cond_broadcast, "pthread_cond_broadcast");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, pthreadApi->pthread_cond_signal, fpl__pthread_func_pthread_cond_signal, "pthread_cond_signal");
			result = true;
			break;
		}
		fpl__PThreadApiUnloadPThreadApi(pthreadApi);
	}
	return(result);
}

typedef struct {
	//! Dummy field
	int dummy;
} fpl__PosixInitState;

typedef struct {
	fpl__PThreadApi pthreadApi;
} fpl__PosixAppState;
#endif // FPL_SUBPLATFORM_POSIX

// ############################################################################
//
// > TYPES_LINUX
//
// ############################################################################
#if defined(FPL_PLATFORM_LINUX)
typedef struct {
	//! Dummy field
	int dummy;
} fpl__LinuxInitState;

typedef struct {
	//! Dummy field
	int dummy;
} fpl__LinuxAppState;
#endif // FPL_PLATFORM_LINUX

// ############################################################################
//
// > TYPES_X11
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_X11)
//
// X11 Api
//
#define FPL__FUNC_X11_X_FREE(name) int name(void *data)
typedef FPL__FUNC_X11_X_FREE(fpl__func_x11_XFree);
#define FPL__FUNC_X11_X_FLUSH(name) int name(Display *display)
typedef FPL__FUNC_X11_X_FLUSH(fpl__func_x11_XFlush);
#define FPL__FUNC_X11_X_OPEN_DISPLAY(name) Display *name(char *display_name)
typedef FPL__FUNC_X11_X_OPEN_DISPLAY(fpl__func_x11_XOpenDisplay);
#define FPL__FUNC_X11_X_CLOSE_DISPLAY(name) int name(Display *display)
typedef FPL__FUNC_X11_X_CLOSE_DISPLAY(fpl__func_x11_XCloseDisplay);
#define FPL__FUNC_X11_X_DEFAULT_SCREEN(name) int name(Display *display)
typedef FPL__FUNC_X11_X_DEFAULT_SCREEN(fpl__func_x11_XDefaultScreen);
#define FPL__FUNC_X11_X_ROOT_WINDOW(name) Window name(Display *display, int screen_number)
typedef FPL__FUNC_X11_X_ROOT_WINDOW(fpl__func_x11_XRootWindow);
#define FPL__FUNC_X11_X_CREATE_WINDOW(name) Window name(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int clazz, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
typedef FPL__FUNC_X11_X_CREATE_WINDOW(fpl__func_x11_XCreateWindow);
#define FPL__FUNC_X11_X_DESTROY_WINDOW(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_X_DESTROY_WINDOW(fpl__func_x11_XDestroyWindow);
#define FPL__FUNC_X11_X_CREATE_COLORMAP(name) Colormap name(Display *display, Window w, Visual *visual, int alloc)
typedef FPL__FUNC_X11_X_CREATE_COLORMAP(fpl__func_x11_XCreateColormap);
#define FPL__FUNC_X11_X_DEFAULT_COLORMAP(name) Colormap name(Display *display, int screen_number)
typedef FPL__FUNC_X11_X_DEFAULT_COLORMAP(fpl__func_x11_XDefaultColormap);
#define FPL__FUNC_X11_X_FREE_COLORMAP(name) int name(Display *display, Colormap colormap)
typedef FPL__FUNC_X11_X_FREE_COLORMAP(fpl__func_x11_XFreeColormap);
#define FPL__FUNC_X11_X_MAP_WINDOW(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_X_MAP_WINDOW(fpl__func_x11_XMapWindow);
#define FPL__FUNC_X11_X_UNMAP_WINDOW(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_X_UNMAP_WINDOW(fpl__func_x11_XUnmapWindow);
#define FPL__FUNC_X11_X_STORE_NAME(name) int name(Display *display, Window w, char *window_name)
typedef FPL__FUNC_X11_X_STORE_NAME(fpl__func_x11_XStoreName);
#define FPL__FUNC_X11_X_DEFAULT_VISUAL(name) Visual *name(Display *display, int screen_number)
typedef FPL__FUNC_X11_X_DEFAULT_VISUAL(fpl__func_x11_XDefaultVisual);
#define FPL__FUNC_X11_X_DEFAULT_DEPTH(name) int name(Display *display, int screen_number)
typedef FPL__FUNC_X11_X_DEFAULT_DEPTH(fpl__func_x11_XDefaultDepth);

typedef struct {
	void *libHandle;
	fpl__func_x11_XFlush *XFlush;
	fpl__func_x11_XFree *XFree;
	fpl__func_x11_XOpenDisplay *XOpenDisplay;
	fpl__func_x11_XCloseDisplay *XCloseDisplay;
	fpl__func_x11_XDefaultScreen *XDefaultScreen;
	fpl__func_x11_XRootWindow *XRootWindow;
	fpl__func_x11_XCreateWindow *XCreateWindow;
	fpl__func_x11_XDestroyWindow *XDestroyWindow;
	fpl__func_x11_XCreateColormap *XCreateColormap;
	fpl__func_x11_XFreeColormap *XFreeColormap;
	fpl__func_x11_XDefaultColormap *XDefaultColormap;
	fpl__func_x11_XMapWindow *XMapWindow;
	fpl__func_x11_XUnmapWindow *XUnmapWindow;
	fpl__func_x11_XStoreName *XStoreName;
	fpl__func_x11_XDefaultVisual *XDefaultVisual;
	fpl__func_x11_XDefaultDepth *XDefaultDepth;
} fpl__X11Api;

fpl_internal void fpl__UnloadX11Api(fpl__X11Api *x11Api) {
	FPL_ASSERT(x11Api != fpl_null);
	if (x11Api->libHandle != fpl_null) {
		dlclose(x11Api->libHandle);
	}
	FPL_CLEAR_STRUCT(x11Api);
}

fpl_internal bool fpl__LoadX11Api(fpl__X11Api *x11Api) {
	const char* libFileNames[] = {
		"libX11.so",
		"libX11.so.7",
		"libX11.so.6",
		"libX11.so.5",
	};
	bool result = false;
	for (uint32_t index = 0; index < FPL_ARRAYCOUNT(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		void *libHandle = x11Api->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if (libHandle != fpl_null) {
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XFlush, fpl__func_x11_XFlush, "XFlush");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XFree, fpl__func_x11_XFree, "XFree");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XOpenDisplay, fpl__func_x11_XOpenDisplay, "XOpenDisplay");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XCloseDisplay, fpl__func_x11_XCloseDisplay, "XCloseDisplay");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XDefaultScreen, fpl__func_x11_XDefaultScreen, "XDefaultScreen");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XRootWindow, fpl__func_x11_XRootWindow, "XRootWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XCreateWindow, fpl__func_x11_XCreateWindow, "XCreateWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XDestroyWindow, fpl__func_x11_XDestroyWindow, "XDestroyWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XCreateColormap, fpl__func_x11_XCreateColormap, "XCreateColormap");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XFreeColormap, fpl__func_x11_XFreeColormap, "XFreeColormap");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XDefaultColormap, fpl__func_x11_XDefaultColormap, "XDefaultColormap");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XMapWindow, fpl__func_x11_XMapWindow, "XMapWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XUnmapWindow, fpl__func_x11_XUnmapWindow, "XUnmapWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XStoreName, fpl__func_x11_XStoreName, "XStoreName");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XDefaultVisual, fpl__func_x11_XDefaultVisual, "XDefaultVisual");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XDefaultDepth, fpl__func_x11_XDefaultDepth, "XDefaultDepth");
			result = true;
			break;
		}
		fpl__UnloadX11Api(x11Api);
	}
	return(result);
}

typedef struct {
	fpl__X11Api api;
} fpl__X11SubplatformState;

typedef struct {
	Display* display;
	int screen;
	Window root;
	Colormap colorMap;
	Window window;
} fpl__X11WindowState;

typedef struct {
	Visual *visual;
	int colorDepth;
} fpl__X11PreWindowSetupResult;
#endif // FPL_SUBPLATFORM_X11

// ****************************************************************************
//
// > PLATFORM_STATES
//
// - Defines the final PlatformInitState and PlatformAppState
// - Declares all required global variables
//
// ****************************************************************************
#if !defined(FPL_PLATFORM_STATES_DEFINED)
#define FPL_PLATFORM_STATES_DEFINED
//
// Platform initialization state
//
typedef struct {
#if defined(FPL_SUBPLATFORM_POSIX)
	fpl__PosixInitState posix;
#endif
	bool isInitialized;

	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32InitState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxInitState linux;
#	endif               
	};
} fpl__PlatformInitState;
fpl_globalvar fpl__PlatformInitState fpl__global__InitState = FPL_ZERO_INIT;

#if defined(FPL_ENABLE_WINDOW)
#define FPL__MAX_EVENT_COUNT 32768
typedef struct {
	fplEvent events[FPL__MAX_EVENT_COUNT];
	volatile uint32_t pollIndex;
	volatile uint32_t pushCount;
} fpl__EventQueue;

typedef struct {
	fpl__EventQueue eventQueue;
	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32WindowState win32;
#	elif defined(FPL_SUBPLATFORM_X11)
		fpl__X11WindowState x11;
#	endif
	};
} fpl__PlatformWindowState;
#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_VIDEO)
typedef struct fpl__PlatformVideoState {
	void *mem; // Points to fpl__VideoState
	size_t memSize;
} fpl__PlatformVideoState;
#endif // FPL_ENABLE_VIDEO

#if defined(FPL_ENABLE_AUDIO)
typedef struct fpl__PlatformAudioState {
	void *mem; // Points to fpl__AudioState
	size_t memSize;
} fpl__PlatformAudioState;
#endif

//
// Platform application state
//
typedef struct fpl__PlatformAppState fpl__PlatformAppState;
struct fpl__PlatformAppState {
#if defined(FPL_SUBPLATFORM_POSIX)
	fpl__PosixAppState posix;
#       endif
#if defined(FPL_SUBPLATFORM_X11)
	fpl__X11SubplatformState x11;
#endif

#if defined(FPL_ENABLE_WINDOW)
	fpl__PlatformWindowState window;
#endif

#if defined(FPL_ENABLE_VIDEO)
	fpl__PlatformVideoState video;
#endif

#if defined(FPL_ENABLE_AUDIO)
	fpl__PlatformAudioState audio;
#endif

	fplSettings initSettings;
	fplSettings currentSettings;
	fplInitFlags initFlags;

	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32AppState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxAppState linux;
#	endif
	};
};

#if defined(FPL_ENABLE_WINDOW)
fpl_internal_inline void fpl__PushEvent(const fplEvent *event) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	if (eventQueue->pushCount < FPL__MAX_EVENT_COUNT) {
		uint32_t eventIndex = fplAtomicAddU32(&eventQueue->pushCount, 1);
		FPL_ASSERT(eventIndex < FPL__MAX_EVENT_COUNT);
		eventQueue->events[eventIndex] = *event;
	}
}

typedef union fpl__PreSetupWindowResult {
#if defined(FPL_SUBPLATFORM_X11)
	fpl__X11PreWindowSetupResult x11;
#endif
	//! Dummy field when no subplatforms are available
	int dummy;
} fpl__PreSetupWindowResult;

// @NOTE(final): Callback used for setup a window before it is created
#define FPL__FUNC_PRE_SETUP_WINDOW(name) bool name(fpl__PlatformAppState *appState, const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PreSetupWindowResult *outResult)
typedef FPL__FUNC_PRE_SETUP_WINDOW(callback_PreSetupWindow);

// @NOTE(final): Callback used for setup a window after it was created
#define FPL__FUNC_POST_SETUP_WINDOW(name) bool name(fpl__PlatformAppState *appState, const fplInitFlags initFlags, const fplSettings *initSettings)
typedef FPL__FUNC_POST_SETUP_WINDOW(callback_PostSetupWindow);

typedef struct fpl__SetupWindowCallbacks {
	callback_PreSetupWindow *preSetup;
	callback_PostSetupWindow *postSetup;
} fpl__SetupWindowCallbacks;
#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_STATES_DEFINED

// ****************************************************************************
//
// > COMMON (Common Implementations)
//
// - Standard C library includes
// - Internal macros
// - Error state handling
// - Threading state handling
// - CommonAudioState
// - Common Implementations (Strings, Memory, Atomics, Path, etc.)
//
// ****************************************************************************
#if !defined(FPL_COMMON_DEFINED)
#define FPL_COMMON_DEFINED

// Standard includes
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h> // fprintf, vfprintf, vsnprintf, getchar
#include <stdlib.h> // wcstombs, mbstowcs, getenv

//
// Macros
//

// Clearing memory in chunks
#define FPL_CLEARMEM(T, memory, size, shift, mask) \
	do { \
		size_t clearedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = (T *)(memory); \
			T *dataBlockEnd = (T *)(memory) + (size >> shift); \
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
			T *sourceDataBlock = (T *)(source); \
			T *sourceDataBlockEnd = (T *)(source) + (sourceSize >> shift); \
			T *destDataBlock = (T *)(dest); \
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

//
// Internal types and functions
//
#define FPL__MAX_LAST_ERROR_STRING_LENGTH 1024
#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
#	define FPL__MAX_ERRORSTATE_COUNT 256
#else
#	define FPL__MAX_ERRORSTATE_COUNT 1
#endif

typedef struct fpl__ErrorState {
	char errors[FPL__MAX_ERRORSTATE_COUNT][FPL__MAX_LAST_ERROR_STRING_LENGTH];
	uint32_t count;
} fpl__ErrorState;

fpl_globalvar fpl__ErrorState fpl__global__LastErrorState;

#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
fpl_internal_inline void fpl__PushError_Formatted(const char *format, va_list argList) {
	fpl__ErrorState *state = &fpl__global__LastErrorState;
	FPL_ASSERT(format != fpl_null);
	char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH];
	vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	uint32_t messageLen = fplGetAnsiStringLength(buffer);
	FPL_ASSERT(state->count < FPL__MAX_ERRORSTATE_COUNT);
	size_t errorIndex = state->count;
	state->count = (state->count + 1) % FPL__MAX_ERRORSTATE_COUNT;
	fplCopyAnsiStringLen(buffer, messageLen, state->errors[errorIndex], FPL__MAX_LAST_ERROR_STRING_LENGTH);
#if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
	const char *platformName = FPL_PLATFORM_NAME;
	fplConsoleFormatError("FPL Error[%s]: %s\n", platformName, buffer);
#endif
}
#else
fpl_internal_inline void fpl__PushError_Formatted(const char *format, va_list argList) {
	ErrorState *state = fpl__global__LastErrorState;
	FPL_ASSERT(format != fpl_null);
	char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH];
	vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	uint32_t messageLen = fplGetAnsiStringLength(buffer);
	state->count = 1;
	fplCopyAnsiStringLen(buffer, messageLen, state->errors[0], FPL__MAX_LAST_ERROR_STRING_LENGTH);
#if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
	fplConsoleFormatError("FPL Error[%s]: %s\h", FPL_PLATFORM_NAME, buffer);
#endif
}
#endif // FPL_ENABLE_MULTIPLE_ERRORSTATES

#if defined(FPL_ENABLE_AUDIO)

// @TODO(final): Why is CommonAudioState here???
typedef struct fpl__CommonAudioState {
	fplAudioDeviceFormat internalFormat;
	fpl_audio_client_read_callback *clientReadCallback;
	void *clientUserData;
} fpl__CommonAudioState;

fpl_internal_inline uint32_t fpl__ReadAudioFramesFromClient(const fpl__CommonAudioState *commonAudio, uint32_t frameCount, void *pSamples) {
	uint32_t outputSamplesWritten = 0;
	if (commonAudio->clientReadCallback != fpl_null) {
		outputSamplesWritten = commonAudio->clientReadCallback(&commonAudio->internalFormat, frameCount, pSamples, commonAudio->clientUserData);
	}
	return outputSamplesWritten;
}
#endif // FPL_ENABLE_AUDIO

fpl_internal void fpl__PushError(const char *format, ...) {
	va_list valist;
	va_start(valist, format);
	fpl__PushError_Formatted(format, valist);
	va_end(valist);
}

fpl_internal void fpl__ArgumentNullError(const char *paramName) {
	fpl__PushError("%s parameter are not allowed to be null", paramName);
}
fpl_internal void fpl__ArgumentZeroError(const char *paramName) {
	fpl__PushError("%s parameter must be greater than zero", paramName);
}
fpl_internal void fpl__ArgumentSizeTooSmallError(const char *paramName, const size_t value, const size_t minValue) {
	fpl__PushError("%s parameter '%zu' must be greater or equal than '%zu'", paramName, value, minValue);
}
fpl_internal void fpl__ArgumentSizeTooBigError(const char *paramName, const size_t value, const size_t maxValue) {
	fpl__PushError("%s parameter '%zu' must be less or equal than '%zu'", paramName, value, maxValue);
}

// Maximum number of active threads you can have in your process
#define FPL__MAX_THREAD_COUNT 64

// Maximum number of active signals you can wait for
#define FPL__MAX_SIGNAL_COUNT 256

typedef struct fpl__ThreadState {
	fplThreadHandle mainThread;
	fplThreadHandle threads[FPL__MAX_THREAD_COUNT];
} fpl__ThreadState;

fpl_globalvar fpl__ThreadState fpl__global__ThreadState = FPL_ZERO_INIT;

fpl_internal_inline fplThreadHandle *fpl__GetFreeThread() {
	fplThreadHandle *result = fpl_null;
	for (uint32_t index = 0; index < FPL__MAX_THREAD_COUNT; ++index) {
		fplThreadHandle *thread = fpl__global__ThreadState.threads + index;
		fplThreadState state = fplGetThreadState(thread);
		if (state == fplThreadState_Stopped) {
			result = thread;
			break;
		}
	}
	return(result);
}

//
// Common Strings
//
fpl_common_api bool fplIsStringEqualLen(const char *a, const uint32_t aLen, const char *b, const uint32_t bLen) {
	if ((a == fpl_null) || (b == fpl_null)) {
		return (a == b);
	}
	if (aLen != bLen) {
		return false;
	}
	FPL_ASSERT(aLen == bLen);
	bool result = true;
	for (uint32_t index = 0; index < aLen; ++index) {
		char aChar = a[index];
		char bChar = b[index];
		if (aChar != bChar) {
			result = false;
			break;
		}
	}
	return(result);
}

fpl_common_api bool fplIsStringEqual(const char *a, const char *b) {
	if ((a == fpl_null) || (b == fpl_null)) {
		return (a == b);
	}
	bool result = true;
	for (;;) {
		const char aChar = *(a++);
		const char bChar = *(b++);
		if (aChar == 0 || bChar == 0) {
			result = (aChar == bChar);
			break;
		}
		if (aChar != bChar) {
			result = false;
			break;
		}
	}
	return(result);
}

fpl_common_api uint32_t fplGetAnsiStringLength(const char *str) {
	uint32_t result = 0;
	if (str != fpl_null) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api uint32_t fplGetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str != fpl_null) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api char *fplCopyAnsiStringLen(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
	char *result = fpl_null;
	if ((source != fpl_null && dest != fpl_null) && ((sourceLen + 1) <= maxDestLen)) {
		result = dest;
		uint32_t index = 0;
		while (index++ < sourceLen) {
			*dest++ = *source++;
		}
		*dest = 0;
	} else {
		// @TODO(final): Do we want to push a error here?
	}
	return(result);
}

fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen) {
	char *result = fpl_null;
	if (source != fpl_null) {
		uint32_t sourceLen = fplGetAnsiStringLength(source);
		result = fplCopyAnsiStringLen(source, sourceLen, dest, maxDestLen);
	} else {
		// @TODO(final): Do we want to push a error here?
	}
	return(result);
}

fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
	wchar_t *result = fpl_null;
	if ((source != fpl_null && dest != fpl_null) && ((sourceLen + 1) <= maxDestLen)) {
		result = dest;
		uint32_t index = 0;
		while (index++ < sourceLen) {
			*dest++ = *source++;
		}
		*dest = 0;
	} else {
		// @TODO(final): Do we want to push a error here?
	}
	return(result);
}

fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen) {
	wchar_t *result = fpl_null;
	if (source != fpl_null) {
		uint32_t sourceLen = fplGetWideStringLength(source);
		result = fplCopyWideStringLen(source, sourceLen, dest, maxDestLen);
	} else {
		// @TODO(final): Do we want to push a error here?
	}
	return(result);
}

//
// Common Memory
//
fpl_common_api void *fplMemoryAlignedAllocate(const size_t size, const size_t alignment) {
	if (!size) {
		fpl__ArgumentZeroError("Size");
		return fpl_null;
	}
	if (!alignment) {
		fpl__ArgumentZeroError("Alignment");
		return fpl_null;
	}
	if (alignment & (alignment - 1)) {
		fpl__PushError("Alignment parameter '%zu' must be a power of two", alignment);
		return fpl_null;
	}

	// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
	size_t newSize = sizeof(void *) + size + (alignment << 1);
	void *basePtr = fplMemoryAllocate(newSize);

	// The resulting address starts after the stored base pointer
	void *alignedPtr = (void *)((uint8_t *)basePtr + sizeof(void *));

	// Move the resulting address to a aligned one when not aligned
	uintptr_t mask = alignment - 1;
	if ((alignment > 1) && (((uintptr_t)alignedPtr & mask) != 0)) {
		uintptr_t offset = ((uintptr_t)alignment - ((uintptr_t)alignedPtr & mask));
		alignedPtr = (uint8_t *)alignedPtr + offset;
	}

	// Write the base pointer before the alignment pointer
	*(void **)((void *)((uint8_t *)alignedPtr - sizeof(void *))) = basePtr;

	// Ensure alignment
	FPL_ASSERT(FPL_IS_ALIGNED(alignedPtr, alignment));

	return(alignedPtr);
}

fpl_common_api void fplMemoryAlignedFree(void *ptr) {
	if (ptr == fpl_null) {
		fpl__ArgumentNullError("Pointer");
		return;
	}

	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = *(void **)((void *)((uint8_t *)ptr - sizeof(void *)));
	FPL_ASSERT(basePtr != fpl_null);
	fplMemoryFree(basePtr);
}

#define FPL__MEM_SHIFT_64 3
#define FPL__MEM_MASK_64 0x00000007
#define FPL__MEM_SHIFT_32 2
#define FPL__MEM_MASK_32 0x00000003
#define FPL__MEM_SHIFT_16 1
#define FPL__MEM_MASK_16 0x0000000

fpl_common_api void fplMemoryClear(void *mem, const size_t size) {
	if (mem == fpl_null) {
		fpl__ArgumentNullError("Memory");
		return;
	}
	if (size == 0) {
		fpl__ArgumentSizeTooSmallError("Size", size, 1);
		return;
	}
	// @SPEED(final): Try faster memory copy using SIMD
	if (size % 8 == 0) {
		FPL_CLEARMEM(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if (size % 4 == 0) {
		FPL_CLEARMEM(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if (size % 2 == 0) {
		FPL_CLEARMEM(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16);
	} else {
		FPL_CLEARMEM(uint8_t, mem, size, 0, 0);
	}
}

fpl_common_api void fplMemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem) {
	if (sourceMem == fpl_null) {
		fpl__ArgumentNullError("Source memory");
		return;
	}
	if (!sourceSize) {
		fpl__ArgumentZeroError("Source size");
		return;
	}
	if (targetMem == fpl_null) {
		fpl__ArgumentNullError("Target memory");
		return;
	}
	if (sourceSize % 8 == 0) {
		FPL_COPYMEM(uint64_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if (sourceSize % 4 == 0) {
		FPL_COPYMEM(uint32_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if (sourceSize % 2 == 0) {
		FPL_COPYMEM(uint16_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else {
		FPL_COPYMEM(uint8_t, sourceMem, sourceSize, targetMem, 0, 0);
	}
}

//
// Common Atomics
//
fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value) {
	FPL_ASSERT(target != fpl_null);
#if defined(FPL_ARCH_X64)
	void *result = (void *)fplAtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#elif defined(FPL_ARCH_X86)
	void *result = (void *)fplAtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_ARCH_X64)
	void *result = (void *)fplAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_ARCH_X86)
	void *result = (void *)fplAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api bool fplIsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_ARCH_X64)
	bool result = fplIsAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_ARCH_X86)
	bool result = fplIsAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicLoadPtr(volatile void **source) {
#if defined(FPL_ARCH_X64)
	void *result = (void *)fplAtomicLoadU64((volatile uint64_t *)source);
#elif defined(FPL_ARCH_X86)
	void *result = (void *)fplAtomicLoadU32((volatile uint32_t *)source);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return(result);
}

fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value) {
#if defined(FPL_ARCH_X64)
	fplAtomicStoreU64((volatile uint64_t *)dest, (uint64_t)value);
#elif defined(FPL_ARCH_X86)
	fplAtomicStoreU32((volatile uint32_t *)dest, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
}

//
// Common Paths
//
fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen) {
	if (sourcePath == fpl_null) {
		fpl__ArgumentNullError("Source path");
		return fpl_null;
	}
	uint32_t sourceLen = fplGetAnsiStringLength(sourcePath);
	if (sourceLen == 0) {
		fpl__ArgumentZeroError("Source len");
		return fpl_null;
	}
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredDestLen = sourceLen + 1;
	if (maxDestLen < requiredDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredDestLen);
		return fpl_null;
	}

	char *result = fpl_null;
	if (sourcePath) {
		int copyLen = 0;
		char *chPtr = (char *)sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL__PATH_SEPARATOR) {
				copyLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		if (copyLen) {
			result = fplCopyAnsiStringLen(sourcePath, copyLen, destPath, maxDestLen);
		}
	}
	return(result);
}

fpl_common_api const char *fplExtractFileExtension(const char *sourcePath) {
	const char *result = fpl_null;
	if (sourcePath != fpl_null) {
		const char *filename = fplExtractFileName(sourcePath);
		if (filename) {
			const char *chPtr = filename;
			while (*chPtr) {
				if (*chPtr == FPL__FILE_EXT_SEPARATOR) {
					result = chPtr;
					break;
				}
				++chPtr;
			}
		}
	}
	return(result);
}

fpl_common_api const char *fplExtractFileName(const char *sourcePath) {
	const char *result = fpl_null;
	if (sourcePath) {
		result = sourcePath;
		const char *chPtr = sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL__PATH_SEPARATOR) {
				result = chPtr + 1;
			}
			++chPtr;
		}
	}
	return(result);
}

fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen) {
	if (filePath == fpl_null) {
		fpl__ArgumentNullError("File path");
		return fpl_null;
	}
	if (newFileExtension == fpl_null) {
		fpl__ArgumentNullError("New file extension");
		return fpl_null;
	}
	uint32_t pathLen = fplGetAnsiStringLength(filePath);
	if (pathLen == 0) {
		fpl__ArgumentZeroError("Path len");
		return fpl_null;
	}
	uint32_t extLen = fplGetAnsiStringLength(newFileExtension);

	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredDestLen = pathLen + extLen + 1;
	if (maxDestLen < requiredDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredDestLen);
		return fpl_null;
	}

	char *result = fpl_null;
	if (filePath != fpl_null) {
		// Find last path
		char *chPtr = (char *)filePath;
		char *lastPathSeparatorPtr = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL__PATH_SEPARATOR) {
				lastPathSeparatorPtr = chPtr;
			}
			++chPtr;
		}

		// Find last ext separator
		if (lastPathSeparatorPtr != fpl_null) {
			chPtr = lastPathSeparatorPtr + 1;
		} else {
			chPtr = (char *)filePath;
		}
		char *lastExtSeparatorPtr = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL__FILE_EXT_SEPARATOR) {
				lastExtSeparatorPtr = chPtr;
			}
			++chPtr;
		}

		uint32_t copyLen;
		if (lastExtSeparatorPtr != fpl_null) {
			copyLen = (uint32_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
		} else {
			copyLen = pathLen;
		}

		// Copy parts
		fplCopyAnsiStringLen(filePath, copyLen, destPath, maxDestLen);
		char *destExtPtr = destPath + copyLen;
		fplCopyAnsiStringLen(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
		result = destPath;
	}
	return(result);
}

fpl_common_api char *fplPathCombine(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
	if (pathCount == 0) {
		fpl__ArgumentZeroError("Path count");
		return fpl_null;
	}
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	if (maxDestPathLen == 0) {
		fpl__ArgumentZeroError("Max dest path len");
		return fpl_null;
	}

	uint32_t curDestPosition = 0;
	char *currentDestPtr = destPath;
	va_list vargs;
	va_start(vargs, pathCount);
	for (uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		char *path = va_arg(vargs, char *);
		uint32_t pathLen = fplGetAnsiStringLength(path);
		bool requireSeparator = pathIndex < (pathCount - 1);
		uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
		FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
		fplCopyAnsiStringLen(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
		currentDestPtr += pathLen;
		if (requireSeparator) {
			*currentDestPtr++ = FPL__PATH_SEPARATOR;
		}
		curDestPosition += requiredPathLen;
	}
	*currentDestPtr = 0;
	va_end(vargs);
	return destPath;
}

#if defined(FPL_ENABLE_WINDOW)
fpl_common_api bool fplPollEvent(fplEvent *ev) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	bool result = false;
	if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
		uint32_t eventIndex = fplAtomicAddU32(&eventQueue->pollIndex, 1);
		*ev = eventQueue->events[eventIndex];
		result = true;
	} else if (eventQueue->pushCount > 0) {
		fplAtomicExchangeU32(&eventQueue->pollIndex, 0);
		fplAtomicExchangeU32(&eventQueue->pushCount, 0);
	}
	return result;
}

fpl_common_api void fplClearEvents() {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	fplAtomicExchangeU32(&eventQueue->pollIndex, 0);
	fplAtomicExchangeU32(&eventQueue->pushCount, 0);
}
#endif // FPL_ENABLE_WINDOW

fpl_common_api const char *fplGetPlatformError() {
	const char *result = "";
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
	if (errorState->count > 0) {
		size_t index = errorState->count - 1;
		result = fplGetPlatformErrorFromIndex(index);
	}
#	else
	result = errorState.errors[0];
#	endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
	return (result);
}

fpl_common_api const char *fplGetPlatformErrorFromIndex(const size_t index) {
	const char *result = "";
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
	if (index < errorState->count) {
		result = errorState->errors[index];
	} else {
		result = errorState->errors[errorState->count - 1];
	}
#else
	result = errorState->errors[0];
#endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
	return (result);
}

fpl_common_api size_t fplGetPlatformErrorCount() {
	size_t result = 0;
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	result = errorState->count;
	return (result);
}

fpl_common_api void fplClearPlatformErrors() {
	fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	FPL_CLEAR_STRUCT(errorState);
}

fpl_common_api const fplSettings *fplGetCurrentSettings() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	return &appState->currentSettings;
}
#endif // FPL_COMMON_DEFINED

// ############################################################################
//
// > WIN32_PLATFORM (Win32, Win64)
//
// ############################################################################
#if defined(FPL_PLATFORM_WIN32)

#	if defined(FPL_ARCH_X86)
#		define FPL_MEMORY_BARRIER() \
			LONG barrier; \
			_InterlockedOr(&barrier, 0);
#	elif defined(FPL_ARCH_X64)
		// @NOTE(final): No need for hardware memory fence on X64 because the hardware guarantees memory order always.
#		define FPL_MEMORY_BARRIER()
#	endif

#if defined(FPL_ENABLE_WINDOW)
typedef struct fpl__Win32WindowStyle {
	DWORD style;
	DWORD exStyle;
} fpl__Win32WindowStyle;

#define FPL__Win32ResizeableWindowStyle WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE
#define FPL__Win32ResizeableWindowExtendedStyle WS_EX_LEFT

#define FPL__Win32NonResizableWindowStyle WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE
#define FPL__Win32NonResizableWindowExtendedStyle WS_EX_LEFT

#define FPL__Win32FullscreenWindowStyle WS_POPUP | WS_VISIBLE
#define FPL__Win32FullscreenWindowExtendedStyle WS_EX_APPWINDOW | WS_EX_TOPMOST

fpl_internal bool fpl__Win32LeaveFullscreen() {
	// @TODO(final): The old window rect may be wrong when the display was changed (Turn off, Orientation, Grid Position, Screen res).
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *platState = fpl__global__AppState;
	const fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	const fpl__Win32WindowState *win32Window = &platState->window.win32;
	const fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;

	HWND windowHandle = win32Window->windowHandle;

	FPL_ASSERT(fullscreenInfo->style > 0 && fullscreenInfo->exStyle > 0);
	win32_setWindowLong(windowHandle, GWL_STYLE, fullscreenInfo->style);
	win32_setWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo->exStyle);
	wapi->user.setWindowPlacement(windowHandle, &fullscreenInfo->placement);
	wapi->user.setWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	if (fullscreenInfo->isMaximized) {
		win32_sendMessage(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}

	bool result;
	if (fullscreenInfo->wasResolutionChanged) {
		result = (wapi->user.changeDisplaySettingsA(fpl_null, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
	} else {
		result = true;
	}

	return(result);
}

fpl_internal bool fpl__Win32EnterFullscreen(const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *platState = fpl__global__AppState;
	fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	fpl__Win32WindowState *win32Window = &platState->window.win32;
	fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;

	HWND windowHandle = win32Window->windowHandle;
	HDC deviceContext = win32Window->deviceContext;

	FPL_ASSERT(fullscreenInfo->style > 0 && fullscreenInfo->exStyle > 0);
	win32_setWindowLong(windowHandle, GWL_STYLE, fullscreenInfo->style & ~(WS_CAPTION | WS_THICKFRAME));
	win32_setWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo->exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

	MONITORINFO monitor = FPL_ZERO_INIT;
	monitor.cbSize = sizeof(monitor);
	win32_getMonitorInfo(wapi->user.monitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST), &monitor);

	bool result;
	if (fullscreenWidth > 0 && fullscreenHeight > 0) {
		DWORD useFullscreenWidth = fullscreenWidth;
		DWORD useFullscreenHeight = fullscreenHeight;

		DWORD useRefreshRate = refreshRate;
		if (!useRefreshRate) {
			useRefreshRate = wapi->gdi.getDeviceCaps(deviceContext, VREFRESH);
		}

		DWORD useColourBits = colorBits;
		if (!useColourBits) {
			useColourBits = wapi->gdi.getDeviceCaps(deviceContext, BITSPIXEL);
		}

		// @TODO(final): Is this correct to assume the fullscreen rect is at (0, 0, w - 1, h - 1)?
		RECT windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = windowRect.left + (useFullscreenWidth - 1);
		windowRect.bottom = windowRect.left + (useFullscreenHeight - 1);

		WINDOWPLACEMENT placement = FPL_ZERO_INIT;
		placement.length = sizeof(placement);
		placement.rcNormalPosition = windowRect;
		placement.showCmd = SW_SHOW;
		wapi->user.setWindowPlacement(windowHandle, &placement);
		wapi->user.setWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		DEVMODEA fullscreenSettings = FPL_ZERO_INIT;
		wapi->user.enumDisplaySettingsA(fpl_null, 0, &fullscreenSettings);
		fullscreenSettings.dmPelsWidth = useFullscreenWidth;
		fullscreenSettings.dmPelsHeight = useFullscreenHeight;
		fullscreenSettings.dmBitsPerPel = useColourBits;
		fullscreenSettings.dmDisplayFrequency = useRefreshRate;
		fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		result = (wapi->user.changeDisplaySettingsA(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
		fullscreenInfo->wasResolutionChanged = true;
	} else {
		RECT windowRect = monitor.rcMonitor;

		WINDOWPLACEMENT placement = FPL_ZERO_INIT;
		placement.length = sizeof(placement);
		placement.rcNormalPosition = windowRect;
		placement.showCmd = SW_SHOW;
		wapi->user.setWindowPlacement(windowHandle, &placement);
		wapi->user.setWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		result = true;
		fullscreenInfo->wasResolutionChanged = false;
	}

	return(result);
}

fpl_internal_inline float fpl__Win32XInputProcessStickValue(const SHORT value, const SHORT deadZoneThreshold) {
	float result = 0;
	if (value < -deadZoneThreshold) {
		result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	} else if (value > deadZoneThreshold) {
		result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}
	return(result);
}

fpl_internal void fpl__Win32PollControllers(const fplSettings *settings, const fpl__Win32InitState *initState, fpl__Win32XInputState *xinputState) {
	FPL_ASSERT(settings != fpl_null);
	FPL_ASSERT(xinputState != fpl_null);
	if (xinputState->xinputApi.xInputGetState != fpl_null) {
		//
		// Detect new controller (Only on a fixed frequency)
		//
		if (xinputState->lastDeviceSearchTime.QuadPart == 0) {
			QueryPerformanceCounter(&xinputState->lastDeviceSearchTime);
		}
		LARGE_INTEGER currentDeviceSearchTime;
		QueryPerformanceCounter(&currentDeviceSearchTime);
		uint64_t deviceSearchDifferenceTimeInMs = ((currentDeviceSearchTime.QuadPart - xinputState->lastDeviceSearchTime.QuadPart) / (initState->performanceFrequency.QuadPart / 1000));
		if ((settings->input.controllerDetectionFrequency == 0) || (deviceSearchDifferenceTimeInMs > settings->input.controllerDetectionFrequency)) {
			xinputState->lastDeviceSearchTime = currentDeviceSearchTime;
			for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
				XINPUT_STATE controllerState = FPL_ZERO_INIT;
				if (xinputState->xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					if (!xinputState->isConnected[controllerIndex]) {
						// Connected
						xinputState->isConnected[controllerIndex] = true;
						fplEvent ev = FPL_ZERO_INIT;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Connected;
						ev.gamepad.deviceIndex = controllerIndex;
						fpl__PushEvent(&ev);
					}
				} else {
					if (xinputState->isConnected[controllerIndex]) {
						// Disonnected
						xinputState->isConnected[controllerIndex] = false;
						fplEvent ev = FPL_ZERO_INIT;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Disconnected;
						ev.gamepad.deviceIndex = controllerIndex;
						fpl__PushEvent(&ev);
					}
				}
			}
		}

		//
		// Update controller state when connected only
		//
		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
			if (xinputState->isConnected[controllerIndex]) {
				XINPUT_STATE controllerState = FPL_ZERO_INIT;
				if (xinputState->xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					// State changed
					fplEvent ev = FPL_ZERO_INIT;
					ev.type = fplEventType_Gamepad;
					ev.gamepad.type = fplGamepadEventType_StateChanged;
					ev.gamepad.deviceIndex = controllerIndex;

					XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

					// Analog sticks
					ev.gamepad.state.leftStickX = fpl__Win32XInputProcessStickValue(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					ev.gamepad.state.leftStickY = fpl__Win32XInputProcessStickValue(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
					ev.gamepad.state.rightStickX = fpl__Win32XInputProcessStickValue(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
					ev.gamepad.state.rightStickY = fpl__Win32XInputProcessStickValue(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

					// Triggers
					ev.gamepad.state.leftTrigger = (float)pad->bLeftTrigger / 255.0f;
					ev.gamepad.state.rightTrigger = (float)pad->bRightTrigger / 255.0f;

					// Digital pad buttons
					if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
						ev.gamepad.state.dpadUp.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
						ev.gamepad.state.dpadDown.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
						ev.gamepad.state.dpadLeft.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
						ev.gamepad.state.dpadRight.isDown = true;

					// Action buttons
					if (pad->wButtons & XINPUT_GAMEPAD_A)
						ev.gamepad.state.actionA.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_B)
						ev.gamepad.state.actionB.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_X)
						ev.gamepad.state.actionX.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_Y)
						ev.gamepad.state.actionY.isDown = true;

					// Center buttons
					if (pad->wButtons & XINPUT_GAMEPAD_START)
						ev.gamepad.state.start.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_BACK)
						ev.gamepad.state.back.isDown = true;

					// Shoulder buttons
					if (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
						ev.gamepad.state.leftShoulder.isDown = true;
					if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
						ev.gamepad.state.rightShoulder.isDown = true;

					fpl__PushEvent(&ev);
				}
			}
		}
	}
}

fpl_internal_inline void fpl__Win32PushMouseEvent(const fplMouseEventType mouseEventType, const fplMouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = mouseEventType;
	newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
	newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
	newEvent.mouse.mouseButton = mouseButton;
	if (mouseEventType == fplMouseEventType_Wheel) {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
	}
	fpl__PushEvent(&newEvent);
}

fpl_internal fplKey fpl__Win32MapVirtualKey(const uint64_t keyCode) {
	switch (keyCode) {
		case VK_BACK:
			return fplKey_Backspace;
		case VK_TAB:
			return fplKey_Tab;

		case VK_CLEAR:
			return fplKey_Clear;
		case VK_RETURN:
			return fplKey_Enter;

		case VK_SHIFT:
			return fplKey_Shift;
		case VK_CONTROL:
			return fplKey_Control;
		case VK_MENU:
			return fplKey_Alt;
		case VK_PAUSE:
			return fplKey_Pause;
		case VK_CAPITAL:
			return fplKey_CapsLock;

		case VK_ESCAPE:
			return fplKey_Escape;
		case VK_SPACE:
			return fplKey_Space;
		case VK_PRIOR:
			return fplKey_PageUp;
		case VK_NEXT:
			return fplKey_PageDown;
		case VK_END:
			return fplKey_End;
		case VK_HOME:
			return fplKey_Home;
		case VK_LEFT:
			return fplKey_Left;
		case VK_UP:
			return fplKey_Up;
		case VK_RIGHT:
			return fplKey_Right;
		case VK_DOWN:
			return fplKey_Down;
		case VK_SELECT:
			return fplKey_Select;
		case VK_PRINT:
			return fplKey_Print;
		case VK_EXECUTE:
			return fplKey_Execute;
		case VK_SNAPSHOT:
			return fplKey_Snapshot;
		case VK_INSERT:
			return fplKey_Insert;
		case VK_DELETE:
			return fplKey_Delete;
		case VK_HELP:
			return fplKey_Help;

		case 0x30:
			return fplKey_0;
		case 0x31:
			return fplKey_1;
		case 0x32:
			return fplKey_2;
		case 0x33:
			return fplKey_3;
		case 0x34:
			return fplKey_4;
		case 0x35:
			return fplKey_5;
		case 0x36:
			return fplKey_6;
		case 0x37:
			return fplKey_7;
		case 0x38:
			return fplKey_8;
		case 0x39:
			return fplKey_9;

		case 0x41:
			return fplKey_A;
		case 0x42:
			return fplKey_B;
		case 0x43:
			return fplKey_C;
		case 0x44:
			return fplKey_D;
		case 0x45:
			return fplKey_E;
		case 0x46:
			return fplKey_F;
		case 0x47:
			return fplKey_G;
		case 0x48:
			return fplKey_H;
		case 0x49:
			return fplKey_I;
		case 0x4A:
			return fplKey_J;
		case 0x4B:
			return fplKey_K;
		case 0x4C:
			return fplKey_L;
		case 0x4D:
			return fplKey_M;
		case 0x4E:
			return fplKey_N;
		case 0x4F:
			return fplKey_O;
		case 0x50:
			return fplKey_P;
		case 0x51:
			return fplKey_Q;
		case 0x52:
			return fplKey_R;
		case 0x53:
			return fplKey_S;
		case 0x54:
			return fplKey_T;
		case 0x55:
			return fplKey_U;
		case 0x56:
			return fplKey_V;
		case 0x57:
			return fplKey_W;
		case 0x58:
			return fplKey_X;
		case 0x59:
			return fplKey_Y;
		case 0x5A:
			return fplKey_Z;

		case VK_LWIN:
			return fplKey_LeftWin;
		case VK_RWIN:
			return fplKey_RightWin;
		case VK_APPS:
			return fplKey_Apps;

		case VK_SLEEP:
			return fplKey_Sleep;
		case VK_NUMPAD0:
			return fplKey_NumPad0;
		case VK_NUMPAD1:
			return fplKey_NumPad1;
		case VK_NUMPAD2:
			return fplKey_NumPad2;
		case VK_NUMPAD3:
			return fplKey_NumPad3;
		case VK_NUMPAD4:
			return fplKey_NumPad4;
		case VK_NUMPAD5:
			return fplKey_NumPad5;
		case VK_NUMPAD6:
			return fplKey_NumPad6;
		case VK_NUMPAD7:
			return fplKey_NumPad7;
		case VK_NUMPAD8:
			return fplKey_NumPad8;
		case VK_NUMPAD9:
			return fplKey_NumPad9;
		case VK_MULTIPLY:
			return fplKey_Multiply;
		case VK_ADD:
			return fplKey_Add;
		case VK_SEPARATOR:
			return fplKey_Separator;
		case VK_SUBTRACT:
			return fplKey_Substract;
		case VK_DECIMAL:
			return fplKey_Decimal;
		case VK_DIVIDE:
			return fplKey_Divide;
		case VK_F1:
			return fplKey_F1;
		case VK_F2:
			return fplKey_F2;
		case VK_F3:
			return fplKey_F3;
		case VK_F4:
			return fplKey_F4;
		case VK_F5:
			return fplKey_F5;
		case VK_F6:
			return fplKey_F6;
		case VK_F7:
			return fplKey_F7;
		case VK_F8:
			return fplKey_F8;
		case VK_F9:
			return fplKey_F9;
		case VK_F10:
			return fplKey_F10;
		case VK_F11:
			return fplKey_F11;
		case VK_F12:
			return fplKey_F12;
		case VK_F13:
			return fplKey_F13;
		case VK_F14:
			return fplKey_F14;
		case VK_F15:
			return fplKey_F15;
		case VK_F16:
			return fplKey_F16;
		case VK_F17:
			return fplKey_F17;
		case VK_F18:
			return fplKey_F18;
		case VK_F19:
			return fplKey_F19;
		case VK_F20:
			return fplKey_F20;
		case VK_F21:
			return fplKey_F21;
		case VK_F22:
			return fplKey_F22;
		case VK_F23:
			return fplKey_F23;
		case VK_F24:
			return fplKey_F24;

		case VK_LSHIFT:
			return fplKey_LeftShift;
		case VK_RSHIFT:
			return fplKey_RightShift;
		case VK_LCONTROL:
			return fplKey_LeftControl;
		case VK_RCONTROL:
			return fplKey_RightControl;
		case VK_LMENU:
			return fplKey_LeftAlt;
		case VK_RMENU:
			return fplKey_RightAlt;

		default:
			return fplKey_None;
	}
}

fpl_internal_inline void fpl__Win32PushKeyboardEvent(const fplKeyboardEventType keyboardEventType, const uint64_t keyCode, const fplKeyboardModifierFlags modifiers, const bool isDown) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = fpl__Win32MapVirtualKey(keyCode);
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl__PushEvent(&newEvent);
}

fpl_internal_inline bool fpl__Win32IsKeyDown(const fpl__Win32Api *wapi, const uint64_t keyCode) {
	bool result = (wapi->user.getAsyncKeyState((int)keyCode) & 0x8000) > 0;
	return(result);
}

LRESULT CALLBACK fpl__Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;

	fpl__Win32AppState *win32State = &appState->win32;
	fpl__Win32WindowState *win32Window = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32State->winApi;

	if (!win32Window->windowHandle) {
		return win32_defWindowProc(hwnd, msg, wParam, lParam);
	}

	LRESULT result = 0;
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			win32Window->isRunning = false;
		} break;

		case WM_SIZE:
		{
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			if (appState->currentSettings.video.driver == fplVideoDriverType_Software) {
				if (appState->initSettings.video.isAutoSize) {
					uint32_t w = LOWORD(lParam);
					uint32_t h = HIWORD(lParam);
					fplResizeVideoBackBuffer(w, h);
				}
			}
#			endif

			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_Resized;
			newEvent.window.width = LOWORD(lParam);
			newEvent.window.height = HIWORD(lParam);
			fpl__PushEvent(&newEvent);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint64_t keyCode = wParam;
			bool wasDown = ((int)(lParam & (1 << 30)) != 0);
			bool isDown = ((int)(lParam & (1 << 31)) == 0);

			bool altKeyWasDown = fpl__Win32IsKeyDown(wapi, VK_MENU);
			bool shiftKeyWasDown = fpl__Win32IsKeyDown(wapi, VK_LSHIFT);
			bool ctrlKeyWasDown = fpl__Win32IsKeyDown(wapi, VK_LCONTROL);
			bool superKeyWasDown = fpl__Win32IsKeyDown(wapi, VK_LMENU);

			fplKeyboardEventType keyEventType = isDown ? fplKeyboardEventType_KeyDown : fplKeyboardEventType_KeyUp;
			fplKeyboardModifierFlags modifiers = fplKeyboardModifierFlags_None;
			if (altKeyWasDown) {
				modifiers |= fplKeyboardModifierFlags_Alt;
			}
			if (shiftKeyWasDown) {
				modifiers |= fplKeyboardModifierFlags_Shift;
			}
			if (ctrlKeyWasDown) {
				modifiers |= fplKeyboardModifierFlags_Ctrl;
			}
			if (superKeyWasDown) {
				modifiers |= fplKeyboardModifierFlags_Super;
			}
			fpl__Win32PushKeyboardEvent(keyEventType, keyCode, modifiers, isDown);

			if (wasDown != isDown) {
				if (isDown) {
					if (keyCode == VK_F4 && altKeyWasDown) {
						win32Window->isRunning = 0;
					}
				}
			}
		} break;

		case WM_CHAR:
		{
			// @TODO(final): Add unicode support (WM_UNICHAR)!
			if (wParam >= 0 && wParam < 256) {
				uint64_t keyCode = wParam;
				fplKeyboardModifierFlags modifiers = fplKeyboardModifierFlags_None;
				fpl__Win32PushKeyboardEvent(fplKeyboardEventType_CharInput, keyCode, modifiers, 0);
			}
		} break;

		case WM_ACTIVATE:
		{
			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Window;
			if (wParam == WA_INACTIVE) {
				newEvent.window.type = fplWindowEventType_LostFocus;
			} else {
				newEvent.window.type = fplWindowEventType_GotFocus;
			}
			fpl__PushEvent(&newEvent);
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			fplMouseEventType mouseEventType;
			if (msg == WM_LBUTTONDOWN) {
				mouseEventType = fplMouseEventType_ButtonDown;
			} else {
				mouseEventType = fplMouseEventType_ButtonUp;
			}
			fpl__Win32PushMouseEvent(mouseEventType, fplMouseButtonType_Left, lParam, wParam);
		} break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			fplMouseEventType mouseEventType;
			if (msg == WM_RBUTTONDOWN) {
				mouseEventType = fplMouseEventType_ButtonDown;
			} else {
				mouseEventType = fplMouseEventType_ButtonUp;
			}
			fpl__Win32PushMouseEvent(mouseEventType, fplMouseButtonType_Right, lParam, wParam);
		} break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			fplMouseEventType mouseEventType;
			if (msg == WM_MBUTTONDOWN) {
				mouseEventType = fplMouseEventType_ButtonDown;
			} else {
				mouseEventType = fplMouseEventType_ButtonUp;
			}
			fpl__Win32PushMouseEvent(mouseEventType, fplMouseButtonType_Middle, lParam, wParam);
		} break;
		case WM_MOUSEMOVE:
		{
			fpl__Win32PushMouseEvent(fplMouseEventType_Move, fplMouseButtonType_None, lParam, wParam);
		} break;
		case WM_MOUSEWHEEL:
		{
			fpl__Win32PushMouseEvent(fplMouseEventType_Wheel, fplMouseButtonType_None, lParam, wParam);
		} break;

		case WM_SETCURSOR:
		{
			// @TODO(final): This is not right to assume default cursor always, because the size cursor does not work this way!
			if (win32Window->isCursorActive) {
				HCURSOR cursor = wapi->user.getCursor();
				wapi->user.setCursor(cursor);
			} else {
				wapi->user.setCursor(fpl_null);
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

fpl_internal bool fpl__Win32InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *platAppState, fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	FPL_ASSERT(appState != fpl_null);
	const fpl__Win32Api *wapi = &appState->winApi;
	const fplWindowSettings *initWindowSettings = &initSettings->window;

	// Register window class
	win32_wndclassex windowClass = FPL_ZERO_INIT;
	windowClass.cbSize = sizeof(win32_wndclassex);
	windowClass.hInstance = GetModuleHandleA(fpl_null);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hCursor = win32_loadCursor(windowClass.hInstance, IDC_ARROW);
	windowClass.hIcon = win32_loadIcon(windowClass.hInstance, IDI_APPLICATION);
	windowClass.hIconSm = win32_loadIcon(windowClass.hInstance, IDI_APPLICATION);
	windowClass.lpszClassName = FPL_WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl__Win32MessageProc;
	windowClass.style |= CS_OWNDC;

#if _UNICODE
	fplCopyWideString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#else
	fplCopyAnsiString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#endif

	if (win32_registerClassEx(&windowClass) == 0) {
		fpl__PushError("Failed registering window class '%s'", windowState->windowClass);
		return false;
	}

	// Set window title
#if _UNICODE
	wchar_t windowTitleBuffer[1024];
	if (fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplAnsiStringToWideString(initWindowSettings->windowTitle, fplGetAnsiStringLength(initWindowSettings->windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const wchar_t *defaultTitle = FPL_WIN32_UNNAMED_WINDOW;
		fplCopyWideString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	wchar_t *windowTitle = windowTitleBuffer;
	fplWideStringToAnsiString(windowTitle, fplGetWideStringLength(windowTitle), currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#else
	char windowTitleBuffer[1024];
	if (fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplCopyAnsiString(initWindowSettings->windowTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const char *defaultTitle = FPL_WIN32_UNNAMED_WINDOW;
		fplCopyAnsiString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	char *windowTitle = windowTitleBuffer;
	fplCopyAnsiString(windowTitle, currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#endif

#if 0
	win32_char windowTitleBuffer[1024];
	const win32_char *defaultTitle = FPL_WIN32_UNNAMED_WINDOW;
	win32_ansiToString(defaultTitle, fplGetAnsiStringLength(defaultTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	currentWindowSettings->isFullscreen = false;
	if (fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		win32_ansiToString(initWindowSettings->windowTitle, fplGetAnsiStringLength(initWindowSettings->windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
#endif

	// Create window
	DWORD style;
	DWORD exStyle;
	if (initSettings->window.isResizable) {
		style = FPL__Win32ResizeableWindowStyle;
		exStyle = FPL__Win32ResizeableWindowExtendedStyle;
		currentWindowSettings->isResizable = true;
	} else {
		style = FPL__Win32NonResizableWindowStyle;
		exStyle = FPL__Win32NonResizableWindowExtendedStyle;
		currentWindowSettings->isResizable = false;
	}

	int windowX = CW_USEDEFAULT;
	int windowY = CW_USEDEFAULT;
	int windowWidth;
	int windowHeight;
	if ((initWindowSettings->windowWidth > 0) &&
		(initWindowSettings->windowHeight > 0)) {
		RECT windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = initWindowSettings->windowWidth;
		windowRect.bottom = initWindowSettings->windowHeight;
		wapi->user.adjustWindowRect(&windowRect, style, false);
		windowWidth = windowRect.right - windowRect.left;
		windowHeight = windowRect.bottom - windowRect.top;
	} else {
		// @NOTE(final): Operating system decide how big the window should be.
		windowWidth = CW_USEDEFAULT;
		windowHeight = CW_USEDEFAULT;
	}

	// Create window
	windowState->windowHandle = win32_createWindowEx(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, fpl_null, fpl_null, windowClass.hInstance, fpl_null);
	if (windowState->windowHandle == fpl_null) {
		fpl__PushError("Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", windowState->windowClass, windowX, windowY, windowWidth, windowHeight);
		return false;
	}

	// Get actual window size and store results
	currentWindowSettings->windowWidth = windowWidth;
	currentWindowSettings->windowHeight = windowHeight;
	RECT clientRect;
	if (wapi->user.getClientRect(windowState->windowHandle, &clientRect)) {
		currentWindowSettings->windowWidth = clientRect.right - clientRect.left;
		currentWindowSettings->windowHeight = clientRect.bottom - clientRect.top;
	}

	// Get device context so we can swap the back and front buffer
	windowState->deviceContext = wapi->user.getDC(windowState->windowHandle);
	if (windowState->deviceContext == fpl_null) {
		fpl__PushError("Failed aquiring device context from window '%d'", windowState->windowHandle);
		return false;
	}

	// Call post window setup callback
	if (setupCallbacks->postSetup != fpl_null) {
		setupCallbacks->postSetup(platAppState, platAppState->initFlags, initSettings);
	}

	// Enter fullscreen if needed
	if (initWindowSettings->isFullscreen) {
		fplSetWindowFullscreen(true, initWindowSettings->fullscreenWidth, initWindowSettings->fullscreenHeight, 0);
	}

	// Show window
	wapi->user.showWindow(windowState->windowHandle, SW_SHOW);
	wapi->user.updateWindow(windowState->windowHandle);

	// Cursor is visible at start
	windowState->defaultCursor = windowClass.hCursor;
	windowState->isCursorActive = true;
	windowState->isRunning = true;

	return true;
}

fpl_internal void fpl__Win32ReleaseWindow(const fpl__Win32InitState *initState, const fpl__Win32AppState *appState, fpl__Win32WindowState *windowState) {
	const fpl__Win32Api *wapi = &appState->winApi;

	if (windowState->deviceContext != fpl_null) {
		wapi->user.releaseDC(windowState->windowHandle, windowState->deviceContext);
		windowState->deviceContext = fpl_null;
	}

	if (windowState->windowHandle != fpl_null) {
		wapi->user.destroyWindow(windowState->windowHandle);
		windowState->windowHandle = fpl_null;
		win32_unregisterClass(windowState->windowClass, initState->appInstance);
	}
}

typedef struct fpl__Win32CommandLineUTF8Arguments {
	void *mem;
	char **args;
	uint32_t count;
} fpl__Win32CommandLineUTF8Arguments;

fpl_internal fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseWideArguments(LPWSTR cmdLine) {
	fpl__Win32CommandLineUTF8Arguments args = FPL_ZERO_INIT;

	// @NOTE(final): Temporary load and unload shell32 for parsing the arguments
	HMODULE shellapiLibrary = LoadLibraryA("shell32.dll");
	if (shellapiLibrary != fpl_null) {
		fpl__win32_func_CommandLineToArgvW *commandLineToArgvW = (fpl__win32_func_CommandLineToArgvW *)GetProcAddress(shellapiLibrary, "CommandLineToArgvW");
		if (commandLineToArgvW != fpl_null) {
			// Parse arguments and compute total UTF8 string length
			int executableFilePathArgumentCount = 0;
			wchar_t **executableFilePathArgs = commandLineToArgvW(L"", &executableFilePathArgumentCount);
			uint32_t executableFilePathLen = 0;
			for (int i = 0; i < executableFilePathArgumentCount; ++i) {
				if (i > 0) {
					// Include whitespace
					executableFilePathLen++;
				}
				uint32_t sourceLen = fplGetWideStringLength(executableFilePathArgs[i]);
				uint32_t destLen = WideCharToMultiByte(CP_UTF8, 0, executableFilePathArgs[i], sourceLen, fpl_null, 0, 0, 0);
				executableFilePathLen += destLen;
			}

			// @NOTE(final): Do not parse the arguments when there are no actual arguments, otherwise we will get back the executable arguments again.
			int actualArgumentCount = 0;
			wchar_t **actualArgs = fpl_null;
			uint32_t actualArgumentsLen = 0;
			if (cmdLine != fpl_null && fplGetWideStringLength(cmdLine) > 0) {
				actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
				for (int i = 0; i < actualArgumentCount; ++i) {
					uint32_t sourceLen = fplGetWideStringLength(actualArgs[i]);
					uint32_t destLen = WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], sourceLen, fpl_null, 0, 0, 0);
					actualArgumentsLen += destLen;
				}
			}

			// Calculate argument 
			args.count = 1 + actualArgumentCount;
			uint32_t totalStringLen = executableFilePathLen + actualArgumentsLen + args.count;
			size_t singleArgStringSize = sizeof(char) * (totalStringLen);
			size_t arbitaryPadding = FPL__SIZE_PADDING;
			size_t argArraySize = sizeof(char **) * args.count;
			size_t totalArgSize = singleArgStringSize + arbitaryPadding + argArraySize;

			args.mem = (uint8_t *)fplMemoryAllocate(totalArgSize);
			char *argsString = (char *)args.mem;
			args.args = (char **)((uint8_t *)args.mem + singleArgStringSize + arbitaryPadding);

			// Convert executable path to UTF8
			char *destArg = argsString;
			{
				args.args[0] = argsString;
				for (int i = 0; i < executableFilePathArgumentCount; ++i) {
					if (i > 0) {
						*destArg++ = ' ';
					}
					wchar_t *sourceArg = executableFilePathArgs[i];
					uint32_t sourceArgLen = fplGetWideStringLength(sourceArg);
					uint32_t destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, fpl_null, 0, 0, 0);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
					destArg += destArgLen;
				}
				*destArg++ = 0;
				LocalFree(executableFilePathArgs);
			}

			// Convert actual arguments to UTF8
			if (actualArgumentCount > 0) {
				FPL_ASSERT(actualArgs != fpl_null);
				for (int i = 0; i < actualArgumentCount; ++i) {
					args.args[1 + i] = destArg;
					wchar_t *sourceArg = actualArgs[i];
					uint32_t sourceArgLen = fplGetWideStringLength(sourceArg);
					uint32_t destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, fpl_null, 0, 0, 0);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
					destArg += destArgLen;
					*destArg++ = 0;
				}
				LocalFree(actualArgs);
			}
		}
		FreeLibrary(shellapiLibrary);
		shellapiLibrary = fpl_null;
	}
	return(args);
}

fpl_internal fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseAnsiArguments(LPSTR cmdLine) {
	fpl__Win32CommandLineUTF8Arguments result;
	if (cmdLine != fpl_null) {
		uint32_t ansiSourceLen = fplGetAnsiStringLength(cmdLine);
		uint32_t wideDestLen = MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, fpl_null, 0);
		// @TODO(final): Can we use a stack allocation here?
		wchar_t *wideCmdLine = (wchar_t *)fplMemoryAllocate(sizeof(wchar_t) * (wideDestLen + 1));
		MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, wideCmdLine, wideDestLen);
		wideCmdLine[wideDestLen] = 0;
		result = fpl__Win32ParseWideArguments(wideCmdLine);
		fplMemoryFree(wideCmdLine);
	} else {
		wchar_t tmp[1] = { 0 };
		result = fpl__Win32ParseWideArguments(tmp);
	}
	return(result);
}

#endif // FPL_ENABLE_WINDOW

fpl_internal bool fpl__Win32ThreadWaitForMultiple(fplThreadHandle *threads[], const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
	if (threads == fpl_null) {
		fpl__ArgumentNullError("Threads");
		return false;
	}
	if (count > FPL__MAX_THREAD_COUNT) {
		fpl__ArgumentSizeTooBigError("Count", count, FPL__MAX_THREAD_COUNT);
		return false;
	}
	HANDLE threadHandles[FPL__MAX_THREAD_COUNT];
	for (uint32_t index = 0; index < count; ++index) {
		fplThreadHandle *thread = threads[index];
		if (thread == fpl_null) {
			fpl__PushError("Thread for index '%d' are not allowed to be null", index);
			return false;
		}
		if (thread->internalHandle.win32ThreadHandle == fpl_null) {
			fpl__PushError("Thread handle for index '%d' are not allowed to be null", index);
			return false;
		}
		HANDLE handle = thread->internalHandle.win32ThreadHandle;
		threadHandles[index] = handle;
	}
	DWORD code = WaitForMultipleObjects(count, threadHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
	bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
	return(result);
}

fpl_internal bool fpl__Win32SignalWaitForMultiple(fplSignalHandle *signals[], const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
	if (signals == fpl_null) {
		fpl__ArgumentNullError("Signals");
		return false;
	}
	if (count > FPL__MAX_SIGNAL_COUNT) {
		fpl__ArgumentSizeTooBigError("Count", count, FPL__MAX_SIGNAL_COUNT);
		return false;
	}
	HANDLE signalHandles[FPL__MAX_SIGNAL_COUNT];
	for (uint32_t index = 0; index < count; ++index) {
		fplSignalHandle *availableSignal = signals[index];
		if (availableSignal == fpl_null) {
			fpl__PushError("Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if (availableSignal->internalHandle.win32EventHandle == fpl_null) {
			fpl__PushError("Signal handle for index '%d' are not allowed to be null", index);
			return false;
		}
		HANDLE handle = availableSignal->internalHandle.win32EventHandle;
		signalHandles[index] = handle;
	}
	DWORD code = WaitForMultipleObjects(count, signalHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
	bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
	return(result);
}

fpl_internal void fpl__Win32ReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32InitState *win32InitState = &initState->win32;
	fpl__Win32UnloadXInputApi(&win32AppState->xinput.xinputApi);
	fpl__Win32UnloadApi(&win32AppState->winApi);
}

fpl_internal bool fpl__Win32InitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	fpl__Win32InitState *win32InitState = &initState->win32;
	win32InitState->appInstance = GetModuleHandleA(fpl_null);

	FPL_ASSERT(appState != fpl_null);
	fpl__Win32AppState *win32AppState = &appState->win32;

	// @NOTE(final): Expect kernel32.lib to be linked always, so VirtualAlloc and LoadLibrary will always work.

	// Timing
	QueryPerformanceFrequency(&win32InitState->performanceFrequency);

	// Get main thread infos
	HANDLE mainThreadHandle = GetCurrentThread();
	DWORD mainThreadHandleId = GetCurrentThreadId();
	fplThreadHandle *mainThread = &fpl__global__ThreadState.mainThread;
	FPL_CLEAR_STRUCT(mainThread);
	mainThread->id = mainThreadHandleId;
	mainThread->internalHandle.win32ThreadHandle = mainThreadHandle;
	mainThread->currentState = fplThreadState_Running;

	// Load windows api library
	if (!fpl__Win32LoadApi(&win32AppState->winApi)) {
		// @NOTE(final): Assume that errors are pushed on already.
		fpl__Win32ReleasePlatform(initState, appState);
		return false;
	}

	// Load XInput
	fpl__Win32LoadXInputApi(&win32AppState->xinput.xinputApi);

	return (true);
}

//
// Win32 Atomics
//
fpl_platform_api void fplAtomicReadFence() {
	FPL_MEMORY_BARRIER();
	_ReadBarrier();
}
fpl_platform_api void fplAtomicWriteFence() {
	FPL_MEMORY_BARRIER();
	_WriteBarrier();
}
fpl_platform_api void fplAtomicReadWriteFence() {
	FPL_MEMORY_BARRIER();
	_ReadWriteBarrier();
}

fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
	FPL_ASSERT(target != fpl_null);
	uint32_t result = _InterlockedExchange((volatile unsigned long *)target, value);
	return (result);
}
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value) {
	FPL_ASSERT(target != fpl_null);
	int32_t result = _InterlockedExchange((volatile long *)target, value);
	return (result);
}
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	FPL_ASSERT(target != fpl_null);
	uint64_t result = _InterlockedExchange64((volatile LONG64 *)target, value);
	return (result);
}
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value) {
	FPL_ASSERT(target != fpl_null);
#	if defined(FPL_ARCH_X64)
	int64_t result = _InterlockedExchange64((volatile long long *)target, value);
#	else
	// @NOTE(final): Why does MSVC have no _InterlockedExchange64 on x86???
	int64_t result = _InterlockedCompareExchange64((volatile long long *)target, value, value);
#	endif
	return (result);
}

fpl_platform_api uint32_t fplAtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = _InterlockedExchangeAdd((volatile unsigned long *)value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicAddS32(volatile int32_t *value, const int32_t addend) {
	FPL_ASSERT(value != fpl_null);
	int32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint64_t result = _InterlockedExchangeAdd64((volatile LONG64 *)value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicAddS64(volatile int64_t *value, const int64_t addend) {
	FPL_ASSERT(value != fpl_null);
#	if defined(FPL_ARCH_X64)
	int64_t result = _InterlockedExchangeAdd64((volatile long long *)value, addend);
#	else
		// @NOTE(final): Why does MSVC have no _InterlockedExchangeAdd64 on x86???
	int64_t oldValue = fplAtomicLoadS64(value);
	int64_t newValue = oldValue + addend;
	int64_t result = oldValue;
	fplAtomicStoreS64(value, newValue);
#	endif
	return (result);
}

fpl_platform_api uint32_t fplAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint32_t result = _InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api int32_t fplAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api uint64_t fplAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint64_t result = _InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api int64_t fplAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int64_t result = _InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
	return (result);
}

fpl_platform_api bool fplIsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint32_t value = _InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t value = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint64_t value = _InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int64_t value = _InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}

fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source) {
	uint32_t result = _InterlockedCompareExchange((volatile unsigned long *)source, 0, 0);
	return(result);
}
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source) {
	uint64_t result = _InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
	return(result);
}
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source) {
	int32_t result = _InterlockedCompareExchange((volatile long *)source, 0, 0);
	return(result);
}
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source) {
	int64_t result = _InterlockedCompareExchange64((volatile __int64 *)source, 0, 0);
	return(result);
}

fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
	_InterlockedExchange((volatile unsigned long *)dest, value);
}
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
	_InterlockedExchange64((volatile LONG64 *)dest, value);
}
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value) {
	_InterlockedExchange((volatile long *)dest, value);
}
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value) {
#if defined(FPL_ARCH_X64)
	_InterlockedExchange64((volatile __int64 *)dest, value);
#else
	// @NOTE(final): Why does MSVC have no _InterlockedExchange64 on x86???
	int64_t oldValue = fplAtomicLoadS64(dest);
	_InterlockedCompareExchange64((volatile __int64 *)dest, value, oldValue);
#endif
}

//
// Win32 Hardware
//
fpl_platform_api uint32_t fplGetProcessorCoreCount() {
	SYSTEM_INFO sysInfo = FPL_ZERO_INIT;
	GetSystemInfo(&sysInfo);
	// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
	uint32_t result = sysInfo.dwNumberOfProcessors;
	return(result);
}

fpl_platform_api fplMemoryInfos fplGetSystemMemoryInfos() {
	fplMemoryInfos result = FPL_ZERO_INIT;
	MEMORYSTATUSEX statex = FPL_ZERO_INIT;
	statex.dwLength = sizeof(statex);
	ULONGLONG totalMemorySize;
	if (GetPhysicallyInstalledSystemMemory(&totalMemorySize) && GlobalMemoryStatusEx(&statex)) {
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

fpl_platform_api char *fplGetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
#	define CPU_BRAND_BUFFER_SIZE 0x40

	if (destBuffer == fpl_null) {
		fpl__ArgumentNullError("Dest buffer");
		return fpl_null;
	}

	size_t requiredDestBufferLen = CPU_BRAND_BUFFER_SIZE + 1;
	if (maxDestBufferLen < requiredDestBufferLen) {
		fpl__ArgumentSizeTooSmallError("Max dest buffer len", maxDestBufferLen, requiredDestBufferLen);
		return fpl_null;
	}

	// @TODO(final): __cpuid may not be available on other Win32 Compilers!

	int cpuInfo[4] = { -1 };
	char cpuBrandBuffer[CPU_BRAND_BUFFER_SIZE] = FPL_ZERO_INIT;
	__cpuid(cpuInfo, 0x80000000);
	uint32_t extendedIds = cpuInfo[0];

	// Get the information associated with each extended ID. Interpret CPU brand string.
	uint32_t max = FPL_MIN(extendedIds, 0x80000004);
	for (uint32_t i = 0x80000002; i <= max; ++i) {
		__cpuid(cpuInfo, i);
		uint32_t offset = (i - 0x80000002) << 4;
		fplMemoryCopy(cpuInfo, sizeof(cpuInfo), cpuBrandBuffer + offset);
	}

	// Copy result back to the dest buffer
	uint32_t sourceLen = fplGetAnsiStringLength(cpuBrandBuffer);
	fplCopyAnsiStringLen(cpuBrandBuffer, sourceLen, destBuffer, maxDestBufferLen);

#	undef CPU_BRAND_BUFFER_SIZE

	return(destBuffer);
}

//
// Win32 Threading
//
fpl_internal DWORD WINAPI fpl__Win32ThreadProc(void *data) {
	fplThreadHandle *thread = (fplThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);
	DWORD result = 0;
	if (thread->runFunc != fpl_null) {
		thread->runFunc(thread, thread->data);
	}
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	return(result);
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data) {
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if (thread != fpl_null) {
		DWORD creationFlags = 0;
		DWORD threadId = 0;
		thread->data = data;
		thread->runFunc = runFunc;
		thread->currentState = fplThreadState_Starting;
		HANDLE handle = CreateThread(fpl_null, 0, fpl__Win32ThreadProc, thread, creationFlags, &threadId);
		if (handle != fpl_null) {
			thread->isValid = true;
			thread->id = threadId;
			thread->internalHandle.win32ThreadHandle = handle;
			result = thread;
		} else {
			fpl__PushError("Failed creating thread, error code: %d", GetLastError());
		}
	} else {
		fpl__PushError("All %d threads are in use, you cannot create until you free one", FPL__MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api void fplThreadSleep(const uint32_t milliseconds) {
	Sleep((DWORD)milliseconds);
}

fpl_platform_api void fplThreadDestroy(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		fpl__ArgumentNullError("Thread");
		return;
	}
	if (thread->internalHandle.win32ThreadHandle == fpl_null) {
		fpl__PushError("Win32 thread handle are not allowed to be null");
		return;
	}
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	TerminateThread(handle, 0);
	CloseHandle(handle);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	FPL_CLEAR_STRUCT(thread);
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const uint32_t maxMilliseconds) {
	if (thread == fpl_null) {
		fpl__ArgumentNullError("Thread");
		return false;
	}
	if (thread->internalHandle.win32ThreadHandle == fpl_null) {
		fpl__PushError("Win32 thread handle are not allowed to be null");
		return false;
	}
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	bool result = (WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, true, maxMilliseconds);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, false, maxMilliseconds);
	return(result);
}

fpl_platform_api fplMutexHandle fplMutexCreate() {
	fplMutexHandle result = FPL_ZERO_INIT;
	InitializeCriticalSection(&result.internalHandle.win32CriticalSection);
	result.isValid = true;
	return(result);
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return;
	}
	if (mutex->isValid) {
		DeleteCriticalSection(&mutex->internalHandle.win32CriticalSection);
	}
	FPL_CLEAR_STRUCT(mutex);
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex, const uint32_t maxMilliseconds) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (!mutex->isValid) {
		fpl__PushError("Mutex parameter must be valid");
		return false;
	}
	EnterCriticalSection(&mutex->internalHandle.win32CriticalSection);
	return true;
}

fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (!mutex->isValid) {
		fpl__PushError("Mutex parameter must be valid");
		return false;
	}
	LeaveCriticalSection(&mutex->internalHandle.win32CriticalSection);
	return true;
}

fpl_platform_api fplSignalHandle fplSignalCreate() {
	fplSignalHandle result = FPL_ZERO_INIT;
	HANDLE handle = CreateEventA(fpl_null, FALSE, FALSE, fpl_null);
	if (handle != fpl_null) {
		result.isValid = true;
		result.internalHandle.win32EventHandle = handle;
	} else {
		fpl__PushError("Failed creating signal (Win32 event): %d", GetLastError());
	}
	return(result);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return;
	}
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		fpl__PushError("Signal handle are not allowed to be null");
		return;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	CloseHandle(handle);
	FPL_CLEAR_STRUCT(signal);
}

fpl_platform_api bool fplSignalWaitForOne(fplMutexHandle *mutex, fplSignalHandle *signal, const uint32_t maxMilliseconds) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		fpl__PushError("Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = (WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAll(fplMutexHandle *mutex, fplSignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = fpl__Win32SignalWaitForMultiple((fplSignalHandle **)signals, count, true, maxMilliseconds);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAny(fplMutexHandle *mutex, fplSignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = fpl__Win32SignalWaitForMultiple((fplSignalHandle **)signals, count, false, maxMilliseconds);
	return(result);
}

fpl_platform_api bool fplSignalSet(fplSignalHandle *signal) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		fpl__PushError("Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = SetEvent(handle) == TRUE;
	return(result);
}

//
// Win32 Memory
//
fpl_platform_api void *fplMemoryAllocate(const size_t size) {
	if (size == 0) {
		fpl__ArgumentZeroError("Size");
		return fpl_null;
	}
	void *result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (result == fpl_null) {
		fpl__PushError("Failed allocating memory of %xu bytes", size);
	}
	return(result);
}

fpl_platform_api void fplMemoryFree(void *ptr) {
	if (ptr == fpl_null) {
		fpl__ArgumentNullError("Pointer");
		return;
	}
	VirtualFree(ptr, 0, MEM_FREE);
}

//
// Win32 Files
//
fpl_platform_api bool fplOpenAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	if (outHandle == fpl_null) {
		return false;
	}
	if (filePath != fpl_null) {
		FPL_CLEAR_STRUCT(outHandle);
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplOpenWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	if (outHandle == fpl_null) {
		return false;
	}
	FPL_CLEAR_STRUCT(outHandle);
	if (filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api bool fplCreateAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	if (outHandle == fpl_null) {
		return false;
	}
	FPL_CLEAR_STRUCT(outHandle);
	if (filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, fpl_null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplCreateWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	if (outHandle == fpl_null) {
		return false;
	}
	FPL_CLEAR_STRUCT(outHandle);
	if (filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, fpl_null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api uint32_t fplReadFileBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return 0;
	}
	if (sizeToRead == 0) {
		return 0;
	}
	if (targetBuffer == fpl_null) {
		fpl__ArgumentNullError("Target buffer");
		return 0;
	}
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		fpl__PushError("File handle is not opened for reading");
		return 0;
	}
	uint32_t result = 0;
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	DWORD bytesRead = 0;
	if (ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, fpl_null) == TRUE) {
		result = bytesRead;
	}
	return(result);
}

fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return 0;
	}
	if (sourceSize == 0) {
		fpl__ArgumentZeroError("Source size");
		return 0;
	}
	if (sourceBuffer == fpl_null) {
		fpl__ArgumentNullError("Source buffer");
		return 0;
	}
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		fpl__PushError("File handle is not opened for writing");
		return 0;
	}
	uint32_t result = 0;
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	DWORD bytesWritten = 0;
	if (WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, fpl_null) == TRUE) {
		result = bytesWritten;
	}
	return(result);
}

fpl_platform_api void fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return;
	}
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD moveMethod = FILE_BEGIN;
		if (mode == fplFilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if (mode == fplFilePositionMode_End) {
			moveMethod = FILE_END;
		}
		SetFilePointer(win32FileHandle, (LONG)position, fpl_null, moveMethod);
	}
}

fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return 0;
	}
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD filePosition = SetFilePointer(win32FileHandle, 0L, fpl_null, FILE_CURRENT);
		if (filePosition != INVALID_SET_FILE_POINTER) {
			return filePosition;
		}
	}
	return 0;
}

fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return;
	}
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		CloseHandle(win32FileHandle);
		FPL_CLEAR_STRUCT(fileHandle);
	}
}

fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath) {
	if (filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
			CloseHandle(win32FileHandle);
			return fileSize;
		}
	}
	return 0;
}

fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return 0;
	}
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
		return fileSize;
	}
	return 0;
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if (filePath != fpl_null) {
		WIN32_FIND_DATAA findData;
		HANDLE searchHandle = FindFirstFileA(filePath, &findData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	if (sourceFilePath == fpl_null) {
		fpl__ArgumentNullError("Source file path");
		return false;
	}
	if (targetFilePath == fpl_null) {
		fpl__ArgumentNullError("Target file path");
		return false;
	}
	bool result = (CopyFileA(sourceFilePath, targetFilePath, !overwrite) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath) {
	if (sourceFilePath == fpl_null) {
		fpl__ArgumentNullError("Source file path");
		return false;
	}
	if (targetFilePath == fpl_null) {
		fpl__ArgumentNullError("Target file path");
		return false;
	}
	bool result = (MoveFileA(sourceFilePath, targetFilePath) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileDelete(const char *filePath) {
	if (filePath == fpl_null) {
		fpl__ArgumentNullError("File path");
		return false;
	}
	bool result = (DeleteFileA(filePath) == TRUE);
	return(result);
}

fpl_platform_api bool fplDirectoryExists(const char *path) {
	bool result = false;
	if (path != fpl_null) {
		WIN32_FIND_DATAA findData;
		HANDLE searchHandle = FindFirstFileA(path, &findData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplDirectoriesCreate(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = CreateDirectoryA(path, fpl_null) > 0;
	return(result);
}
fpl_platform_api bool fplDirectoryRemove(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = RemoveDirectoryA(path) > 0;
	return(result);
}
fpl_internal_inline void fpl__Win32FillFileEntry(const WIN32_FIND_DATAA *findData, fplFileEntry *entry) {
	fplCopyAnsiStringLen(findData->cFileName, fplGetAnsiStringLength(findData->cFileName), entry->path, FPL_ARRAYCOUNT(entry->path));

	entry->type = fplFileEntryType_Unknown;
	if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		entry->type = fplFileEntryType_Directory;
	} else if (
		(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
		entry->type = fplFileEntryType_File;
	}

	entry->attributes = fplFileAttributeFlags_None;
	if (findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		entry->attributes = fplFileAttributeFlags_Normal;
	} else {
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			entry->attributes |= fplFileAttributeFlags_Hidden;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			entry->attributes |= fplFileAttributeFlags_ReadOnly;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			entry->attributes |= fplFileAttributeFlags_Archive;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			entry->attributes |= fplFileAttributeFlags_System;
		}
	}
}
fpl_platform_api bool fplListFilesBegin(const char *pathAndFilter, fplFileEntry *firstEntry) {
	if (pathAndFilter == fpl_null) {
		fpl__ArgumentNullError("Path and filter");
		return false;
	}
	if (firstEntry == fpl_null) {
		fpl__ArgumentNullError("First entry");
		return false;
	}
	bool result = false;
	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(pathAndFilter, &findData);
	if (searchHandle != INVALID_HANDLE_VALUE) {
		FPL_CLEAR_STRUCT(firstEntry);
		firstEntry->internalHandle.win32FileHandle = searchHandle;
		fpl__Win32FillFileEntry(&findData, firstEntry);
		result = true;
	}
	return(result);
}
fpl_platform_api bool fplListFilesNext(fplFileEntry *nextEntry) {
	bool result = false;
	if (nextEntry == fpl_null) {
		fpl__ArgumentNullError("Next entry");
		return false;
	}
	if (nextEntry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = nextEntry->internalHandle.win32FileHandle;
		WIN32_FIND_DATAA findData;
		if (FindNextFileA(searchHandle, &findData)) {
			fpl__Win32FillFileEntry(&findData, nextEntry);
			result = true;
		}
	}
	return(result);
}
fpl_platform_api void fplListFilesEnd(fplFileEntry *lastEntry) {
	if (lastEntry == fpl_null) {
		fpl__ArgumentNullError("Last entry");
		return;
	}
	if (lastEntry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = lastEntry->internalHandle.win32FileHandle;
		FindClose(searchHandle);
		FPL_CLEAR_STRUCT(lastEntry);
	}
}

//
// Win32 Path/Directories
//
#if defined(UNICODE)
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredMaxDestLen = MAX_PATH + 1;
	if (maxDestLen < requiredMaxDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredMaxDestLen);
		return fpl_null;
	}
	wchar_t modulePath[MAX_PATH];
	GetModuleFileNameW(fpl_null, modulePath, MAX_PATH);
	fplWideStringToAnsiString(modulePath, fplGetWideStringLength(modulePath), destPath, maxDestLen);
	return(destPath);
}
#else
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredMaxDestLen = MAX_PATH + 1;
	if (maxDestLen < requiredMaxDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredMaxDestLen);
		return fpl_null;
	}
	char modulePath[MAX_PATH];
	GetModuleFileNameA(fpl_null, modulePath, MAX_PATH);
	fplCopyAnsiStringLen(modulePath, fplGetAnsiStringLength(modulePath), destPath, maxDestLen);
	return(destPath);
}
#endif // UNICODE

#if defined(UNICODE)
fpl_platform_api char *fplGetHomePath(char *destPath, const uint32_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredMaxDestLen = MAX_PATH + 1;
	if (maxDestLen < requiredMaxDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredMaxDestLen);
		return fpl_null;
	}
	if (fpl__global__AppState == fpl_null) {
		fpl__PushError("Platform is not initialized");
		return fpl_null;
	}
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	wchar_t homePath[MAX_PATH];
	wapi->shell.shGetFolderPathW(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	fplWideStringToAnsiString(homePath, fplGetWideStringLength(homePath), destPath, maxDestLen);
	return(destPath);
}
#else
fpl_platform_api char *fplGetHomePath(char *destPath, const uint32_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	size_t requiredMaxDestLen = MAX_PATH + 1;
	if (maxDestLen < requiredMaxDestLen) {
		fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredMaxDestLen);
		return fpl_null;
	}
	if (fpl__global__AppState == fpl_null) {
		fpl__PushError("Platform is not initialized");
		return fpl_null;
	}
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	char homePath[MAX_PATH];
	wapi->shell.shGetFolderPathA(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	fplCopyAnsiStringLen(homePath, fplGetAnsiStringLength(homePath), destPath, maxDestLen);
	return(destPath);
}
#endif // UNICODE

//
// Win32 Timings
//
fpl_platform_api double fplGetTimeInSeconds() {
	const fpl__Win32InitState *initState = &fpl__global__InitState.win32;
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double result = time.QuadPart / (double)initState->performanceFrequency.QuadPart;
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInMilliseconds() {
	uint64_t result = GetTickCount();
	return(result);
}

//
// Win32 Strings
//
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (ansiDest == fpl_null) {
		fpl__ArgumentNullError("Ansi dest");
		return fpl_null;
	}
	uint32_t requiredLen = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	size_t minRequiredLen = requiredLen + 1;
	if (maxAnsiDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestLen, minRequiredLen);
		return fpl_null;
	}
	WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, fpl_null, fpl_null);
	ansiDest[requiredLen] = 0;
	return(ansiDest);
}
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (utf8Dest == fpl_null) {
		fpl__ArgumentNullError("UTF8 dest");
		return fpl_null;
	}
	uint32_t requiredLen = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxUtf8DestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max utf8 dest len", maxUtf8DestLen, minRequiredLen);
		return fpl_null;
	}
	WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, fpl_null, fpl_null);
	utf8Dest[requiredLen] = 0;
	return(utf8Dest);
}
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	if (ansiSource == fpl_null) {
		fpl__ArgumentNullError("Ansi source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	uint32_t requiredLen = MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, fpl_null, 0);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	if (utf8Source == fpl_null) {
		fpl__ArgumentNullError("UTF8 source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	uint32_t requiredLen = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, fpl_null, 0);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api char *fplFormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...) {
	if (ansiDestBuffer == fpl_null) {
		fpl__ArgumentNullError("Ansi dest buffer");
		return fpl_null;
	}
	if (maxAnsiDestBufferLen == 0) {
		fpl__ArgumentZeroError("Max ansi dest len");
		return fpl_null;
	}
	if (format == fpl_null) {
		fpl__ArgumentNullError("Format");
		return fpl_null;
	}
	va_list argList;
	va_start(argList, format);
	// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
	ansiDestBuffer[0] = 0;
	int charCount = vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
	if (charCount < 0) {
		fpl__PushError("Format parameter are '%s' are invalid!", format);
		return fpl_null;
	}
	uint32_t requiredMaxAnsiDestBufferLen = charCount + 1;
	if ((int)maxAnsiDestBufferLen < requiredMaxAnsiDestBufferLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestBufferLen, requiredMaxAnsiDestBufferLen);
		return fpl_null;
	}
	va_end(argList);
	FPL_ASSERT(charCount > 0);
	ansiDestBuffer[charCount] = 0;
	char *result = ansiDestBuffer;
	return(result);
}

//
// Win32 Library
//
fpl_platform_api fplDynamicLibraryHandle fplDynamicLibraryLoad(const char *libraryFilePath) {
	fplDynamicLibraryHandle result = FPL_ZERO_INIT;
	if (libraryFilePath != fpl_null) {
		HMODULE libModule = LoadLibraryA(libraryFilePath);
		if (libModule != fpl_null) {
			result.internalHandle.win32LibraryHandle = libModule;
			result.isValid = true;
		}
	}
	return(result);
}
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name) {
	if (handle == fpl_null) {
		fpl__ArgumentNullError("Handle");
		return fpl_null;
	}
	if (handle->internalHandle.win32LibraryHandle != fpl_null && name != fpl_null) {
		HMODULE libModule = handle->internalHandle.win32LibraryHandle;
		return (void *)GetProcAddress(libModule, name);
	}
	return fpl_null;
}
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if (handle == fpl_null) {
		fpl__ArgumentNullError("Handle");
		return;
	}
	if (handle->internalHandle.win32LibraryHandle != fpl_null) {
		HMODULE libModule = (HMODULE)handle->internalHandle.win32LibraryHandle;
		FreeLibrary(libModule);
		FPL_CLEAR_STRUCT(handle);
	}
}

#if defined(FPL_ENABLE_WINDOW)
	//
	// Win32 Window
	//
fpl_platform_api fplWindowSize fplGetWindowArea() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	fplWindowSize result = FPL_ZERO_INIT;
	RECT windowRect;
	if (wapi->user.getClientRect(windowState->windowHandle, &windowRect)) {
		result.width = windowRect.right - windowRect.left;
		result.height = windowRect.bottom - windowRect.top;
	}
	return(result);
}

fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	RECT clientRect, windowRect;
	if (wapi->user.getClientRect(windowState->windowHandle, &clientRect) &&
		wapi->user.getWindowRect(windowState->windowHandle, &windowRect)) {
		int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		int newWidth = width + borderWidth;
		int newHeight = height + borderHeight;
		wapi->user.setWindowPos(windowState->windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

fpl_platform_api bool fplIsWindowResizable() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	DWORD style = win32_getWindowLong(windowState->windowHandle, GWL_STYLE);
	bool result = (style & WS_THICKFRAME) > 0;
	return(result);
}

fpl_platform_api void fplSetWindowResizeable(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	const fpl__Win32AppState *win32State = &appState->win32;
	if (!appState->currentSettings.window.isFullscreen) {
		DWORD style;
		DWORD exStyle;
		if (value) {
			style = FPL__Win32ResizeableWindowStyle;
			exStyle = FPL__Win32ResizeableWindowExtendedStyle;
		} else {
			style = FPL__Win32NonResizableWindowStyle;
			exStyle = FPL__Win32NonResizableWindowExtendedStyle;
		}
		win32_setWindowLong(windowState->windowHandle, GWL_STYLE, style);
		win32_setWindowLong(windowState->windowHandle, GWL_EXSTYLE, exStyle);
		appState->currentSettings.window.isResizable = value;
	}
}

fpl_platform_api bool fplIsWindowFullscreen() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	HWND windowHandle = windowState->windowHandle;
	DWORD style = win32_getWindowLong(windowHandle, GWL_STYLE);
	bool result = (style & FPL__Win32FullscreenWindowStyle) > 0;
	return(result);
}

fpl_platform_api bool fplSetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32WindowState *windowState = &appState->window.win32;
	fplWindowSettings *windowSettings = &appState->currentSettings.window;
	fpl__Win32LastWindowInfo *fullscreenState = &windowState->lastFullscreenInfo;
	const fpl__Win32Api *wapi = &win32AppState->winApi;

	HWND windowHandle = windowState->windowHandle;

	// Save current window info if not already fullscreen
	if (!windowSettings->isFullscreen) {
		fullscreenState->isMaximized = !!wapi->user.isZoomed(windowHandle);
		if (fullscreenState->isMaximized) {
			win32_sendMessage(windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		fullscreenState->style = win32_getWindowLong(windowHandle, GWL_STYLE);
		fullscreenState->exStyle = win32_getWindowLong(windowHandle, GWL_EXSTYLE);
		wapi->user.getWindowPlacement(windowHandle, &fullscreenState->placement);
	}

	if (value) {
		// Enter fullscreen mode or fallback to window mode
		windowSettings->isFullscreen = fpl__Win32EnterFullscreen(fullscreenWidth, fullscreenHeight, refreshRate, 0);
		if (!windowSettings->isFullscreen) {
			fpl__Win32LeaveFullscreen();
		}
	} else {
		fpl__Win32LeaveFullscreen();
		windowSettings->isFullscreen = false;
	}
	return(windowSettings->isFullscreen);
}

fpl_platform_api fplWindowPosition fplGetWindowPosition() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;

	fplWindowPosition result = FPL_ZERO_INIT;
	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	if (wapi->user.getWindowPlacement(windowState->windowHandle, &placement) == TRUE) {
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

fpl_platform_api void fplSetWindowTitle(const char *title) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;

	// @TODO(final): Add function for setting the unicode window title!
	HWND handle = windowState->windowHandle;
	wapi->user.setWindowTextA(handle, title);
}

fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;

	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	RECT windowRect;
	if (wapi->user.getWindowPlacement(windowState->windowHandle, &placement) &&
		wapi->user.getWindowRect(windowState->windowHandle, &windowRect)) {
		switch (placement.showCmd) {
			case SW_NORMAL:
			case SW_SHOW:
			{
				placement.rcNormalPosition.left = left;
				placement.rcNormalPosition.top = top;
				placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
				placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
				wapi->user.setWindowPlacement(windowState->windowHandle, &placement);
			} break;
		}
	}
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	windowState->isCursorActive = value;
}

fpl_platform_api bool fplPushWindowEvent() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	bool result = false;
	MSG msg;
	BOOL R = win32_peekMessage(&msg, fpl_null, 0, 0, PM_REMOVE);
	if (R == TRUE) {
		wapi->user.translateMessage(&msg);
		win32_dispatchMessage(&msg);
		result = true;
	}
	return (result);
}

fpl_platform_api void fplUpdateGameControllers() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	fpl__Win32PollControllers(&appState->currentSettings, win32InitState, &win32AppState->xinput);
}

fpl_platform_api bool fplWindowUpdate() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;

	bool result = false;

	// Poll gamepad controller states
	fpl__Win32PollControllers(&appState->currentSettings, win32InitState, &win32AppState->xinput);

	// Poll window events
	if (windowState->windowHandle != 0) {
		MSG msg;
		while (win32_peekMessage(&msg, fpl_null, 0, 0, PM_REMOVE) != 0) {
			wapi->user.translateMessage(&msg);
			win32_dispatchMessage(&msg);
		}
		result = windowState->isRunning;
	}

	return(result);
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	bool result = windowState->isRunning;
	return(result);
}

fpl_platform_api char *fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	char *result = fpl_null;
	if (wapi->user.openClipboard(windowState->windowHandle)) {
		if (wapi->user.isClipboardFormatAvailable(CF_TEXT)) {
			HGLOBAL dataHandle = wapi->user.getClipboardData(CF_TEXT);
			if (dataHandle != fpl_null) {
				const char *stringValue = (const char *)GlobalLock(dataHandle);
				result = fplCopyAnsiString(stringValue, dest, maxDestLen);
				GlobalUnlock(dataHandle);
			}
		}
		wapi->user.closeClipboard();
	}
	return(result);
}

fpl_platform_api wchar_t *fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	wchar_t *result = fpl_null;
	if (wapi->user.openClipboard(windowState->windowHandle)) {
		if (wapi->user.isClipboardFormatAvailable(CF_UNICODETEXT)) {
			HGLOBAL dataHandle = wapi->user.getClipboardData(CF_UNICODETEXT);
			if (dataHandle != fpl_null) {
				const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
				result = fplCopyWideString(stringValue, dest, maxDestLen);
				GlobalUnlock(dataHandle);
			}
		}
		wapi->user.closeClipboard();
	}
	return(result);
}

fpl_platform_api bool fplSetClipboardAnsiText(const char *ansiSource) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.openClipboard(windowState->windowHandle)) {
		const uint32_t ansiLen = fplGetAnsiStringLength(ansiSource);
		const uint32_t ansiBufferLen = ansiLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
		if (handle != fpl_null) {
			char *target = (char*)GlobalLock(handle);
			fplCopyAnsiStringLen(ansiSource, ansiLen, target, ansiBufferLen);
			GlobalUnlock(handle);
			wapi->user.emptyClipboard();
			wapi->user.setClipboardData(CF_TEXT, handle);
			result = true;
		}
		wapi->user.closeClipboard();
	}
	return(result);
}

fpl_platform_api bool fplSetClipboardWideText(const wchar_t *wideSource) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.openClipboard(windowState->windowHandle)) {
		const uint32_t wideLen = fplGetWideStringLength(wideSource);
		const uint32_t wideBufferLen = wideLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
		if (handle != fpl_null) {
			wchar_t *wideTarget = (wchar_t*)GlobalLock(handle);
			fplCopyWideStringLen(wideSource, wideLen, wideTarget, wideBufferLen);
			GlobalUnlock(handle);
			wapi->user.emptyClipboard();
			wapi->user.setClipboardData(CF_UNICODETEXT, handle);
			result = true;
		}
		wapi->user.closeClipboard();
	}
	return(result);
}

#	if defined(UNICODE)

int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseWideArguments(cmdLine);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	return(result);
}

#	else

int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseAnsiArguments(cmdLine);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	return(result);
}
#	endif // UNICODE

#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ############################################################################
//
// > POSIX_SUBPLATFORM (Linux, Unix)
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_POSIX)
fpl_internal void PosixReleaseSubplatform(PosixAppState &appState) {
	UnloadPThreadApi(appState.pthreadApi);
}

fpl_internal bool PosixInitSubplatform(const InitFlags initFlags, const Settings &initSettings, PosixInitState &initState, PosixAppState &appState) {
	if (!LoadPThreadApi(appState.pthreadApi)) {
		PushError("Failed initializing PThread API");
		return false;
	}
	return true;
}

void *PosixThreadProc(void *data) {
	threading::ThreadHandle *thread = (threading::ThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	atomics::AtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)threading::ThreadState::Running);
	if (thread->runFunc != fpl_null) {
		thread->runFunc(*thread, thread->data);
	}
	atomics::AtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)threading::ThreadState::Stopped);
	pthread_exit(fpl_null);
}

fpl_internal bool PosixMutexLock(const PThreadApi &pthreadAPI, pthread_mutex_t *handle) {
	int lockRes;
	do {
		lockRes = pthreadAPI.pthread_mutex_lock(handle);
	} while (lockRes == EAGAIN);
	bool result = (lockRes == 0);
	return(result);
}

fpl_internal bool PosixMutexUnlock(const PThreadApi &pthreadAPI, pthread_mutex_t *handle) {
	int unlockRes;
	do {
		unlockRes = pthreadAPI.pthread_mutex_unlock(handle);
	} while (unlockRes == EAGAIN);
	bool result = (unlockRes == 0);
	return(result);
}

fpl_internal int PosixMutexCreate(const PThreadApi &pthreadAPI, pthread_mutex_t *handle) {
	*handle = PTHREAD_MUTEX_INITIALIZER;
	int mutexRes;
	do {
		mutexRes = pthreadAPI.pthread_mutex_init(handle, fpl_null);
	} while (mutexRes == EAGAIN);
	return(mutexRes);
}

fpl_internal int PosixConditionCreate(const PThreadApi &pthreadAPI, pthread_cond_t *handle) {
	*handle = PTHREAD_COND_INITIALIZER;
	int condRes;
	do {
		condRes = pthreadAPI.pthread_cond_init(handle, fpl_null);
	} while (condRes == EAGAIN);
	return(condRes);
}

fpl_internal_inline timespec CreateWaitTimeSpec(const uint32_t milliseconds) {
	time_t secs = milliseconds / 1000;
	uint64_t nanoSecs = (milliseconds - (secs * 1000)) * 1000000;
	if (nanoSecs >= 1000000000) {
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

fpl_internal bool PosixThreadWaitForMultiple(threading::ThreadHandle *threads[], const uint32_t minCount, const uint32_t maxCount, const uint32_t maxMilliseconds) {
	if (threads == fpl_null) {
		fpl__ArgumentNullError("Threads");
		return false;
	}
	if (maxCount > MAX_THREAD_COUNT) {
		fpl__ArgumentSizeTooBigError("Max count", maxCount, MAX_THREAD_COUNT);
		return false;
	}
	for (uint32_t index = 0; index < maxCount; ++index) {
		threading::ThreadHandle *thread = threads[index];
		if (thread == fpl_null) {
			PushError("Thread for index '%d' are not allowed to be null", index);
			return false;
		}
		if (!thread->isValid) {
			PushError("Thread for index '%d' is not valid", index);
			return false;
		}
	}

	volatile bool isRunning[fpl::MAX_THREAD_COUNT];
	for (uint32_t index = 0; index < maxCount; ++index) {
		isRunning[index] = true;
	}

	volatile uint32_t completeCount = 0;
	volatile uint64_t startTime = timings::GetTimeInMilliseconds();
	bool result = false;
	while (completeCount < minCount) {
		for (uint32_t index = 0; index < maxCount; ++index) {
			threading::ThreadHandle *thread = threads[index];
			if (isRunning[index]) {
				threading::ThreadState state = threading::GetThreadState(thread);
				if (state == threading::ThreadState::Stopped) {
					isRunning[index] = false;
					++completeCount;
					if (completeCount >= minCount) {
						result = true;
						break;
					}
				}
			}
			threading::ThreadSleep(10);
		}
		if ((maxMilliseconds != UINT32_MAX) && (timings::GetTimeInMilliseconds() - startTime) >= maxMilliseconds) {
			result = false;
			break;
		}
	}
	return(result);
}

fpl_internal bool PosixSignalWaitForMultiple(const PThreadApi &pthreadAPI, threading::MutexHandle &mutex, threading::SignalHandle *signals[], const uint32_t minCount, const uint32_t maxCount, const uint32_t maxMilliseconds, const uint32_t smallWaitDuration = 5) {
	if (signals == fpl_null) {
		fpl__ArgumentNullError("Signals");
		return false;
	}
	if (maxCount > MAX_SIGNAL_COUNT) {
		fpl__ArgumentSizeTooBigError("Max count", maxCount, MAX_SIGNAL_COUNT);
		return false;
	}
	for (uint32_t index = 0; index < maxCount; ++index) {
		threading::SignalHandle *signal = signals[index];
		if (signal == fpl_null) {
			PushError("Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if (!signal->isValid) {
			PushError("Signal for index '%d' is not valid", index);
			return false;
		}
	}

	volatile bool isSignaled[fpl::MAX_SIGNAL_COUNT];
	for (uint32_t index = 0; index < maxCount; ++index) {
		isSignaled[index] = false;
	}


	volatile uint32_t signaledCount = 0;
	volatile uint64_t startTime = timings::GetTimeInMilliseconds();
	bool result = false;
	while (signaledCount < minCount) {
		for (uint32_t index = 0; index < maxCount; ++index) {
			threading::SignalHandle *signal = signals[index];
			if (!isSignaled[index]) {
				timespec t = CreateWaitTimeSpec(smallWaitDuration);
				int condRes = pthreadAPI.pthread_cond_timedwait(&signal->internalHandle.posixCondition, &mutex.internalHandle.posixMutex, &t);
				if (condRes == 0) {
					isSignaled[index] = true;
					++signaledCount;
					if (signaledCount >= minCount) {
						result = true;
						break;
					}
				}
			}
		}
		if ((maxMilliseconds != UINT32_MAX) && (timings::GetTimeInMilliseconds() - startTime) >= maxMilliseconds) {
			result = false;
			break;
		}
	}
	return(result);
}

	//
	// POSIX Atomics
	//
#if defined(FPL_COMPILER_GCC)
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
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}

fpl_platform_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}

fpl_platform_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
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
#else
#	error "This POSIX compiler/platform is not supported!"
#endif

//
// POSIX Timings
//
fpl_platform_api double GetHighResolutionTimeInSeconds() {
	// @TODO(final): Do we need to take the performance frequency into account?
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	double result = (double)t.tv_sec + ((double)t.tv_nsec * 1e-9);
	return(result);
}

fpl_platform_api uint64_t GetTimeInMilliseconds() {
	// @TODO(final): Find a faster way to get the milliseconds
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	uint64_t result = t.tv_sec * 1000 + (uint64_t)(t.tv_nsec / 1.0e6);
	return(result);
}

//
// POSIX Threading
//
fpl_platform_api void ThreadDestroy(ThreadHandle *thread) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	FPL_ASSERT(pthreadAPI.libHandle != fpl_null);
	if (thread != fpl_null && thread->isValid) {
		pthread_t threadHandle = thread->internalHandle.posix.thread;
		pthread_mutex_t *mutexHandle = &thread->internalHandle.posix.mutex;
		pthread_cond_t *condHandle = &thread->internalHandle.posix.stopCondition;

		// If thread is not stopped yet, kill it and wait for termination
		if (pthreadAPI.pthread_kill(threadHandle, 0) == 0) {
			pthreadAPI.pthread_join(threadHandle, fpl_null);
		}
		pthreadAPI.pthread_cond_destroy(condHandle);
		pthreadAPI.pthread_mutex_destroy(mutexHandle);
		atomics::AtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)ThreadState::Stopped);
		FPL_CLEAR_STRUCT(thread);
	}
}

fpl_platform_api ThreadHandle *ThreadCreate(run_thread_function *runFunc, void *data) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	ThreadHandle *result = fpl_null;
	ThreadHandle *thread = fpl__GetFreeThread();
	if (thread != fpl_null) {
		thread->currentState = ThreadState::Stopped;
		thread->data = data;
		thread->runFunc = runFunc;
		thread->isValid = false;
		thread->isStopping = false;

		// Create mutex
		int mutexRes = subplatform_posix::PosixMutexCreate(pthreadAPI, &thread->internalHandle.posix.mutex);
		if (mutexRes != 0) {
			PushError("Failed creating pthread mutex, error code: %d", mutexRes);
		}

		// Create stop condition
		int condRes = -1;
		if (mutexRes == 0) {
			condRes = subplatform_posix::PosixConditionCreate(pthreadAPI, &thread->internalHandle.posix.stopCondition);
			if (condRes != 0) {
				PushError("Failed creating pthread condition, error code: %d", condRes);
				pthreadAPI.pthread_mutex_destroy(&thread->internalHandle.posix.mutex);
			}
		}

		// Create thread
		int threadRes = -1;
		if (condRes == 0) {
			thread->isValid = true;
			// @TODO(final): Better pthread id!
			memory::MemoryCopy(&thread->internalHandle.posix.thread, FPL_MIN(sizeof(thread->id), sizeof(thread->internalHandle.posix.thread)), &thread->id);
			do {
				threadRes = pthreadAPI.pthread_create(&thread->internalHandle.posix.thread, fpl_null, subplatform_posix::PosixThreadProc, (void *)thread);
			} while (threadRes == EAGAIN);
			if (threadRes != 0) {
				PushError("Failed creating pthread, error code: %d", threadRes);
				pthreadAPI.pthread_cond_destroy(&thread->internalHandle.posix.stopCondition);
				pthreadAPI.pthread_mutex_destroy(&thread->internalHandle.posix.mutex);
			}
		}

		if (threadRes == 0) {
			result = thread;
		} else {
			FPL_CLEAR_STRUCT(thread);
		}
	} else {
		PushError("All %d threads are in use, you cannot create until you free one", MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api bool ThreadWaitForOne(ThreadHandle *thread, const uint32_t maxMilliseconds) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	FPL_ASSERT(pthreadAPI.libHandle != fpl_null);
	bool result = false;
	if (thread != fpl_null && thread->isValid) {
		// Set a flag and signal indicating that this thread is being stopped
		pthread_mutex_t *mutexHandle = &thread->internalHandle.posix.mutex;
		pthread_cond_t *condHandle = &thread->internalHandle.posix.stopCondition;
		if (subplatform_posix::PosixMutexLock(pthreadAPI, mutexHandle)) {
			thread->isStopping = true;
			pthreadAPI.pthread_cond_signal(condHandle);
			pthreadAPI.pthread_cond_broadcast(condHandle);
			subplatform_posix::PosixMutexUnlock(pthreadAPI, mutexHandle);
		}

		// Wait until it shuts down
		pthread_t threadHandle = thread->internalHandle.posix.thread;
		int joinRes = pthreadAPI.pthread_join(threadHandle, fpl_null);
		result = (joinRes == 0);
	}
	return (result);
}

fpl_platform_api bool ThreadWaitForAll(ThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = subplatform_posix::PosixThreadWaitForMultiple(threads, count, count, maxMilliseconds);
	return(result);
}

fpl_platform_api bool ThreadWaitForAny(ThreadHandle *threads[], const uint32_t count, const uint32_t maxMilliseconds) {
	bool result = subplatform_posix::PosixThreadWaitForMultiple(threads, 1, count, maxMilliseconds);
	return(result);
}

fpl_platform_api void ThreadSleep(const uint32_t milliseconds) {
	uint32_t ms;
	uint32_t s;
	if (milliseconds > 1000) {
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

fpl_platform_api MutexHandle MutexCreate() {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	MutexHandle result = FPL_ZERO_INIT;
	int mutexRes = subplatform_posix::PosixMutexCreate(pthreadAPI, &result.internalHandle.posixMutex);
	result.isValid = (mutexRes == 0);
	return(result);
}

fpl_platform_api void MutexDestroy(MutexHandle &mutex) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	if (mutex.isValid) {
		pthread_mutex_t *handle = &mutex.internalHandle.posixMutex;
		pthreadAPI.pthread_mutex_destroy(handle);
	}
	mutex = {};
}

fpl_platform_api bool MutexLock(MutexHandle &mutex, const uint32_t maxMilliseconds) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	bool result = false;
	if (mutex.isValid) {
		pthread_mutex_t *handle = &mutex.internalHandle.posixMutex;
		result = subplatform_posix::PosixMutexLock(pthreadAPI, handle);
	}
	return (result);
}

fpl_platform_api bool MutexUnlock(MutexHandle &mutex) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	bool result = false;
	if (mutex.isValid) {
		pthread_mutex_t *handle = &mutex.internalHandle.posixMutex;
		result = subplatform_posix::PosixMutexUnlock(pthreadAPI, handle);
	}
	return (result);
}

fpl_platform_api SignalHandle SignalCreate() {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	SignalHandle result = {};
	int condRes = subplatform_posix::PosixConditionCreate(pthreadAPI, &result.internalHandle.posixCondition);
	result.isValid = (condRes == 0);
	return(result);
}

fpl_platform_api void SignalDestroy(SignalHandle &signal) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	if (signal.isValid) {
		pthread_cond_t *handle = &signal.internalHandle.posixCondition;
		pthreadAPI.pthread_cond_destroy(handle);
	}
	signal = {};
}

fpl_platform_api bool SignalWaitForOne(MutexHandle &mutex, SignalHandle &signal, const uint32_t maxMilliseconds) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	if (!signal.isValid) {
		PushError("Signal is not valid");
		return(false);
	}
	if (!mutex.isValid) {
		PushError("Mutex is not valid");
		return(false);
	}
	if (maxMilliseconds != UINT32_MAX) {
		timespec t = subplatform_posix::CreateWaitTimeSpec(maxMilliseconds);
		pthreadAPI.pthread_cond_timedwait(&signal.internalHandle.posixCondition, &mutex.internalHandle.posixMutex, &t);
	} else {
		pthreadAPI.pthread_cond_wait(&signal.internalHandle.posixCondition, &mutex.internalHandle.posixMutex);
	}
	return(true);
}

fpl_platform_api bool SignalWaitForAll(MutexHandle &mutex, SignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	bool result = subplatform_posix::PosixSignalWaitForMultiple(pthreadAPI, mutex, signals, count, count, maxMilliseconds);
	return(result);
}

fpl_platform_api bool SignalWaitForAny(MutexHandle &mutex, SignalHandle *signals[], const uint32_t count, const uint32_t maxMilliseconds) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	bool result = subplatform_posix::PosixSignalWaitForMultiple(pthreadAPI, mutex, signals, 1, count, maxMilliseconds);
	return(result);
}

fpl_platform_api bool SignalSet(SignalHandle &signal) {
	const PlatformAppState *appState = fpl__global__AppState;
	const subplatform_posix::PThreadApi &pthreadAPI = appState->posix.pthreadApi;
	bool result = false;
	if (signal.isValid) {
		pthread_cond_t *handle = &signal.internalHandle.posixCondition;
		int condRes = pthreadAPI.pthread_cond_signal(handle);
		pthreadAPI.pthread_cond_broadcast(handle);
		result = (condRes == 0);
	}
	return(result);
}

//
// POSIX Library
//
fpl_platform_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
	DynamicLibraryHandle result = {};
	if (libraryFilePath != fpl_null) {
		void *p = dlopen(libraryFilePath, FPL__POSIX_DL_LOADTYPE);
		if (p != fpl_null) {
			result.internalHandle.posixLibraryHandle = p;
			result.isValid = true;
		}
	}
	return(result);
}

fpl_platform_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
	void *result = fpl_null;
	if (handle.internalHandle.posixLibraryHandle != fpl_null && name != fpl_null) {
		void *p = handle.internalHandle.posixLibraryHandle;
		result = ::dlsym(p, name);
	}
	return(result);
}

fpl_platform_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
	if (handle.internalHandle.posixLibraryHandle != fpl_null) {
		void *p = handle.internalHandle.posixLibraryHandle;
		::dlclose(p);
		handle = {};
	}
}

//
// POSIX Memory
//
fpl_platform_api void *MemoryAllocate(const size_t size) {
	// @NOTE(final): MAP_ANONYMOUS ensures that the memory is cleared to zero.

	// Allocate empty memory to hold the size + some arbitary padding + the actual data
	size_t newSize = sizeof(size_t) + SIZE_PADDING + size;
	void *basePtr = ::mmap(fpl_null, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	// Write the size at the beginning
	*(size_t *)basePtr = newSize;

	// The resulting address starts after the arbitary padding
	void *result = (uint8_t *)basePtr + sizeof(size_t) + SIZE_PADDING;
	return(result);
}

fpl_platform_api void MemoryFree(void *ptr) {
	// Free the base pointer which is stored to the left at the start of the size_t
	void *basePtr = (void *)((uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(size_t)));
	size_t storedSize = *(size_t *)basePtr;
	::munmap(basePtr, storedSize);
}

//
// POSIX Files
//
fpl_platform_api FileHandle OpenBinaryFile(const char *filePath) {
	FileHandle result = {};
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = ::open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			result.isValid = true;
			result.internalHandle.posixFileHandle = posixFileHandle;
		}
	}
	return(result);
}
fpl_platform_api FileHandle OpenBinaryFile(const wchar_t *filePath) {
	FileHandle result = {};
	if (filePath != fpl_null) {
		char utf8FilePath[1024] = {};
		strings::WideStringToAnsiString(filePath, strings::GetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		result = OpenBinaryFile(utf8FilePath);
	}
	return(result);
}

fpl_platform_api FileHandle CreateBinaryFile(const char *filePath) {
	FileHandle result = {};
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = ::open(filePath, O_WRONLY | O_CREAT | O_TRUNC);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			result.isValid = true;
			result.internalHandle.posixFileHandle = posixFileHandle;
		}
	}
	return(result);
}
fpl_platform_api FileHandle CreateBinaryFile(const wchar_t *filePath) {
	FileHandle result = {};
	if (filePath != fpl_null) {
		char utf8FilePath[1024] = {};
		strings::WideStringToAnsiString(filePath, strings::GetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		result = CreateBinaryFile(utf8FilePath);
	}
	return(result);
}

fpl_platform_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	if (sizeToRead == 0) {
		return 0;
	}
	if (targetBuffer == fpl_null) {
		fpl__ArgumentNullError("Target buffer");
		return 0;
	}
	if (!fileHandle.internalHandle.posixFileHandle) {
		PushError("File handle is not opened for reading");
		return 0;
	}
	int posixFileHandle = fileHandle.internalHandle.posixFileHandle;

	ssize_t res;
	do {
		res = ::read(posixFileHandle, targetBuffer, sizeToRead);
	} while (res == -1 && errno == EINTR);

	uint32_t result = 0;
	if (res != -1) {
		result = (uint32_t)res;
	}
	return(result);
}

fpl_platform_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	if (sourceSize == 0) {
		return 0;
	}
	if (sourceBuffer == fpl_null) {
		fpl__ArgumentNullError("Source buffer");
		return 0;
	}
	if (!fileHandle.internalHandle.posixFileHandle) {
		PushError("File handle is not opened for writing");
		return 0;
	}

	int posixFileHandle = fileHandle.internalHandle.posixFileHandle;

	ssize_t res;
	do {
		res = ::write(posixFileHandle, sourceBuffer, sourceSize);
	} while (res == -1 && errno == EINTR);

	uint32_t result = 0;
	if (res != -1) {
		result = (uint32_t)res;
	}
	return(result);
}

fpl_platform_api void SetFilePosition32(const FileHandle &fileHandle, const int32_t position, const FilePositionMode mode) {
	if (fileHandle.internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle.internalHandle.posixFileHandle;
		int whence = SEEK_SET;
		if (mode == FilePositionMode::Current) {
			whence = SEEK_CUR;
		} else if (mode == FilePositionMode::End) {
			whence = SEEK_END;
		}
		::lseek(posixFileHandle, position, whence);
	}
}

fpl_platform_api uint32_t GetFilePosition32(const FileHandle &fileHandle) {
	uint32_t result = 0;
	if (fileHandle.internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle.internalHandle.posixFileHandle;
		off_t res = ::lseek(posixFileHandle, 0, SEEK_CUR);
		if (res != -1) {
			result = (uint32_t)res;
		}
	}
	return(result);
}

fpl_platform_api void CloseFile(FileHandle &fileHandle) {
	if (fileHandle.internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle.internalHandle.posixFileHandle;
		::close(posixFileHandle);
		fileHandle = {};
	}
}

fpl_platform_api uint32_t GetFileSize32(const char *filePath) {
	uint32_t result = 0;
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = ::open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			off_t res = ::lseek(posixFileHandle, 0, SEEK_END);
			if (res != -1) {
				result = (uint32_t)res;
			}
			::close(posixFileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint32_t GetFileSize32(const FileHandle &fileHandle) {
	uint32_t result = 0;
	if (fileHandle.internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle.internalHandle.posixFileHandle;
		off_t curPos = ::lseek(posixFileHandle, 0, SEEK_CUR);
		if (curPos != -1) {
			result = (uint32_t)::lseek(posixFileHandle, 0, SEEK_END);
			::lseek(posixFileHandle, curPos, SEEK_SET);
		}
	}
	return(result);
}

fpl_platform_api bool FileExists(const char *filePath) {
	bool result = false;
	if (filePath != fpl_null) {
		result = ::access(filePath, F_OK) != -1;
	}
	return(result);
}

fpl_platform_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	if (sourceFilePath == fpl_null) {
		fpl__ArgumentNullError("Source file path");
		return false;
	}
	if (targetFilePath == fpl_null) {
		fpl__ArgumentNullError("Target file path");
		return false;
	}
	bool result = false;

	// @IMPLEMENT(final): POSIX FileCopy - there is no built-in copy-file function in POSIX so we open a file and buffer copy it over

	return(result);
}

fpl_platform_api bool FileMove(const char *sourceFilePath, const char *targetFilePath) {
	if (sourceFilePath == fpl_null) {
		fpl__ArgumentNullError("Source file path");
		return false;
	}
	if (targetFilePath == fpl_null) {
		fpl__ArgumentNullError("Target file path");
		return false;
	}
	bool result = ::rename(sourceFilePath, targetFilePath) == 0;
	return(result);
}

fpl_platform_api bool FileDelete(const char *filePath) {
	if (filePath == fpl_null) {
		fpl__ArgumentNullError("File path");
		return false;
	}
	bool result = ::unlink(filePath) == 0;
	return(result);
}

fpl_platform_api bool DirectoryExists(const char *path) {
	bool result = false;
	if (path != fpl_null) {
		struct stat sb;
		result = (::stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
	}
	return(result);
}

fpl_platform_api bool CreateDirectories(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = ::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	return(result);
}
fpl_platform_api bool RemoveEmptyDirectory(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = ::rmdir(path) == 0;
	return(result);
}
fpl_platform_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry) {
	if (pathAndFilter == fpl_null) {
		fpl__ArgumentNullError("Path and filter");
		return false;
	}
	bool result = false;
	// @IMPLEMENT(final): POSIX ListFilesBegin
	return(result);
}
fpl_platform_api bool ListFilesNext(FileEntry &nextEntry) {
	bool result = false;
	if (nextEntry.internalHandle.posixFileHandle) {
		// @IMPLEMENT(final): POSIX ListFilesNext
	}
	return(result);
}
fpl_platform_api void ListFilesEnd(FileEntry &lastEntry) {
	if (lastEntry.internalHandle.posixFileHandle) {
		// @IMPLEMENT(final): POSIX ListFilesEnd
		lastEntry = {};
	}
}
#endif // FPL_SUBPLATFORM_POSIX

// ############################################################################
//
// > STD_STRINGS_SUBPLATFORM
//
// Strings Implementation using C Standard Library
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_STD_STRINGS)
// @NOTE(final): stdio.h is already included
fpl_platform_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (ansiDest == fpl_null) {
		fpl__ArgumentNullError("Ansi dest");
		return fpl_null;
	}
	uint32_t requiredLen = ::wcstombs(fpl_null, wideSource, maxWideSourceLen);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxAnsiDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestLen, minRequiredLen);
		return fpl_null;
	}
	::wcstombs(ansiDest, wideSource, maxWideSourceLen);
	ansiDest[requiredLen] = 0;
	return(ansiDest);
}
fpl_platform_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (utf8Dest == fpl_null) {
		fpl__ArgumentNullError("UTF8 dest");
		return fpl_null;
	}
	// @TODO(final): UTF-8!
	uint32_t requiredLen = ::wcstombs(fpl_null, wideSource, maxWideSourceLen);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxUtf8DestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max utf8 dest len", maxUtf8DestLen, minRequiredLen);
		return fpl_null;
	}
	::wcstombs(utf8Dest, wideSource, maxWideSourceLen);
	utf8Dest[requiredLen] = 0;
	return(utf8Dest);
}
fpl_platform_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	if (ansiSource == fpl_null) {
		fpl__ArgumentNullError("Ansi source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	uint32_t requiredLen = ::mbstowcs(fpl_null, ansiSource, ansiSourceLen);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	::mbstowcs(wideDest, ansiSource, ansiSourceLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	if (utf8Source == fpl_null) {
		fpl__ArgumentNullError("UTF8 source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	// @TODO(final): UTF-8!
	uint32_t requiredLen = ::mbstowcs(fpl_null, utf8Source, utf8SourceLen);
	uint32_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	::mbstowcs(wideDest, utf8Source, utf8SourceLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api char *FormatAnsiString(char *ansiDestBuffer, const uint32_t maxAnsiDestBufferLen, const char *format, ...) {
	if (ansiDestBuffer == fpl_null) {
		fpl__ArgumentNullError("Ansi dest buffer");
		return fpl_null;
	}
	if (maxAnsiDestBufferLen == 0) {
		fpl__ArgumentZeroError("Max ansi dest len");
		return fpl_null;
	}
	if (format == fpl_null) {
		fpl__ArgumentNullError("Format");
		return fpl_null;
	}
	va_list argList;
	va_start(argList, format);
	// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
	ansiDestBuffer[0] = 0;
	int charCount = ::vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
	if (charCount < 0) {
		PushError("Format parameter are '%s' are invalid!", format);
		return fpl_null;
	}
	uint32_t minAnsiDestBufferLen = charCount + 1;
	if (maxAnsiDestBufferLen < minAnsiDestBufferLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestBufferLen, minAnsiDestBufferLen);
		return fpl_null;
	}
	va_end(argList);
	FPL_ASSERT(charCount > 0);
	ansiDestBuffer[charCount] = 0;
	char *result = ansiDestBuffer;
	return(result);
}
#endif // FPL_SUBPLATFORM_STD_STRINGS

// ############################################################################
//
// > STD_CONSOLE_SUBPLATFORM
//
// Console Implementation using C Standard Library
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_STD_CONSOLE)
// @NOTE(final): stdio.h is already included
fpl_platform_api void fplConsoleOut(const char *text) {
	if (text != fpl_null) {
		fprintf(stdout, "%s", text);
	}
}
fpl_platform_api void fplConsoleFormatOut(const char *format, ...) {
	if (format != fpl_null) {
		va_list vaList;
		va_start(vaList, format);
		vfprintf(stdout, format, vaList);
		va_end(vaList);
	}
}
fpl_platform_api void fplConsoleError(const char *text) {
	if (text != fpl_null) {
		fprintf(stderr, "%s", text);
	}
}
fpl_platform_api void fplConsoleFormatError(const char *format, ...) {
	if (format != fpl_null) {
		va_list vaList;
		va_start(vaList, format);
		vfprintf(stderr, format, vaList);
		va_end(vaList);
	}
}
fpl_platform_api const char fplConsoleWaitForCharInput() {
	int c = getchar();
	const char result = (c >= 0 && c < 256) ? (char)c : 0;
	return(result);
}
#endif // FPL_SUBPLATFORM_STD_CONSOLE

// ############################################################################
//
// > X11_SUBPLATFORM
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_X11)
fpl_internal void X11ReleaseSubplatform(X11SubplatformState &subplatform) {
	UnloadX11Api(subplatform.api);
}

fpl_internal bool X11InitSubplatform(X11SubplatformState &subplatform) {
	if (!LoadX11Api(subplatform.api)) {
		PushError("Failed loading x11 api!");
		return false;
	}
	return true;
}

fpl_internal void X11ReleaseWindow(const X11SubplatformState &subplatform, X11WindowState &windowState) {
	const X11Api &x11Api = subplatform.api;
	if (windowState.window) {
		FPL_LOG("X11", "Hide window '%d' from display '%p'", (int)windowState.window, windowState.display);
		x11Api.XUnmapWindow(windowState.display, windowState.window);
		FPL_LOG("X11", "Destroy window '%d' on display '%p'", (int)windowState.window, windowState.display);
		x11Api.XDestroyWindow(windowState.display, windowState.window);
		windowState.window = 0;
	}
	if (windowState.colorMap) {
		FPL_LOG("X11", "Release color map '%d' from display '%p'", (int)windowState.colorMap, windowState.display);
		x11Api.XFreeColormap(windowState.display, windowState.colorMap);
		windowState.colorMap = 0;
	}
	if (windowState.display) {
		FPL_LOG("X11", "Close display '%p'", windowState.display);
		x11Api.XCloseDisplay(windowState.display);
		windowState.display = fpl_null;
	}
	windowState = {};
}

fpl_internal bool X11InitWindow(const Settings &initSettings, WindowSettings &currentWindowSettings, PlatformAppState *appState, X11SubplatformState &subplatform, X11WindowState &windowState, const SetupWindowCallbacks &setupCallbacks) {
	const X11Api &x11Api = subplatform.api;

	FPL_LOG("X11", "Open default Display");
	windowState.display = XOpenDisplay(fpl_null);
	if (windowState.display == fpl_null) {
		FPL_LOG("X11", "Failed opening default Display!");
		return false;
	}
	FPL_LOG("X11", "Successfully opened default Display: %p", windowState.display);

	FPL_LOG("X11", "Get default screen from display '%p'", windowState.display);
	windowState.screen = x11Api.XDefaultScreen(windowState.display);
	FPL_LOG("X11", "Got default screen from display '%p': %d", windowState.display, windowState.screen);

	FPL_LOG("X11", "Get root window from display '%p' and screen '%d'", windowState.display, windowState.screen);
	windowState.root = x11Api.XRootWindow(windowState.display, windowState.screen);
	FPL_LOG("X11", "Got root window from display '%p' and screen '%d': %d", windowState.display, windowState.screen, (int)windowState.root);

	bool usePreSetupWindow = false;
	PreSetupWindowResult setupResult = {};
	if (setupCallbacks.preSetup != fpl_null) {
		FPL_LOG("X11", "Call Pre-Setup for Window");
		usePreSetupWindow = setupCallbacks.preSetup(appState, appState->initFlags, initSettings, setupResult);
	}

	Visual *visual = fpl_null;
	int colorDepth = 0;
	Colormap colormap;
	if (usePreSetupWindow) {
		FPL_LOG("X11", "Got visual '%p' and color depth '%d' from pre-setup", setupResult.x11.visual, setupResult.x11.colorDepth);
		FPL_ASSERT(setupResult.x11.visual != fpl_null);
		visual = setupResult.x11.visual;
		colorDepth = setupResult.x11.colorDepth;
		colormap = x11Api.XCreateColormap(windowState.display, windowState.root, visual, AllocNone);
	} else {
		FPL_LOG("X11", "Using default colormap, visual, color depth");
		visual = x11Api.XDefaultVisual(windowState.display, windowState.root);
		colorDepth = x11Api.XDefaultDepth(windowState.display, windowState.root);
		colormap = x11Api.XDefaultColormap(windowState.display, windowState.root);
	}

	FPL_LOG("X11", "Using visual: %p", visual);
	FPL_LOG("X11", "Using color depth: %d", colorDepth);
	FPL_LOG("X11", "Using color map: %d", (int)colormap);

	windowState.colorMap = colormap;

	XSetWindowAttributes swa;
	swa.colormap = colormap;
	swa.event_mask = StructureNotifyMask;

	// @TODO(final): Proper window size
	uint32_t windowWidth = 640;
	uint32_t windowHeight = 640;

	// @TODO(final): Fullscreen support

	FPL_LOG("X11", "Create window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'", windowState.display, (int)windowState.root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
	windowState.window = x11Api.XCreateWindow(windowState.display,
											  windowState.root,
											  0,
											  0,
											  windowWidth,
											  windowHeight,
											  0,
											  colorDepth,
											  InputOutput,
											  visual,
											  CWColormap | CWEventMask,
											  &swa);
	if (!windowState.window) {
		FPL_LOG("X11", "Failed creating window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'!", windowState.display, (int)windowState.root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
		X11ReleaseWindow(subplatform, windowState);
		return false;
	}
	FPL_LOG("X11", "Successfully created window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d': %d", windowState.display, (int)windowState.root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap, (int)windowState.window);

	char nameBuffer[1024] = {};
	strings::CopyAnsiString("Unnamed FPL X11 Window", nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
	FPL_LOG("X11", "Show window '%d' on display '%p' with title '%s'", (int)windowState.window, windowState.display, nameBuffer);
	x11Api.XStoreName(windowState.display, windowState.window, nameBuffer);
	x11Api.XMapWindow(windowState.display, windowState.window);

	return true;
}

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
	return fpl_null;
}

fpl_platform_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
	// @IMPLEMENT(final): X11 GetClipboardWideText
	return fpl_null;
}

fpl_platform_api bool SetClipboardText(const char *ansiSource) {
	// @IMPLEMENT(final): X11 SetClipboardText (ansi)
	return false;
}

fpl_platform_api bool SetClipboardText(const wchar_t *wideSource) {
	// @IMPLEMENT(final): X11 SetClipboardText (wide)
	return false;
}
#endif // FPL_SUBPLATFORM_X11

// ############################################################################
//
// > LINUX_PLATFORM
//
// ############################################################################
#if defined(FPL_PLATFORM_LINUX)
#   include <ctype.h> // isspace

fpl_internal void LinuxReleasePlatform(PlatformInitState &initState, PlatformAppState *appState) {
	FPL_LOG_BLOCK;
}

fpl_internal bool LinuxInitPlatform(const InitFlags initFlags, const Settings &initSettings, PlatformInitState &initState, PlatformAppState *appState) {
	FPL_LOG_BLOCK;

	return true;
}

//
// Linux Hardware
//
fpl_platform_api uint32_t GetProcessorCoreCount() {
	uint32_t result = sysconf(_SC_NPROCESSORS_ONLN);
	return(result);
}

fpl_platform_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
	if (destBuffer == fpl_null) {
		fpl__ArgumentNullError("Dest buffer");
		return fpl_null;
	}
	if (maxDestBufferLen == 0) {
		fpl__ArgumentZeroError("Max dest buffer len");
		return fpl_null;
	}
	char *result = fpl_null;
	FILE *fileHandle = fopen("/proc/cpuinfo", "rb");
	if (fileHandle != fpl_null) {
		char buffer[256];
		char line[256];
		const size_t maxBufferSize = FPL_ARRAYCOUNT(buffer);
		int32_t readSize = maxBufferSize;
		int32_t readPos = 0;
		bool found = false;
		int bytesRead = 0;
		while ((bytesRead = fread(&buffer[readPos], readSize, 1, fileHandle)) > 0) {
			char *lastP = &buffer[0];
			char *p = &buffer[0];
			while (*p) {
				if (*p == '\n') {
					int32_t len = p - lastP;
					FPL_ASSERT(len > 0);
					if (strings::IsStringEqual(lastP, 10, "model name", 10)) {
						strings::CopyAnsiString(lastP, len, line, FPL_ARRAYCOUNT(line));
						found = true;
						break;
					}
					lastP = p + 1;
				}
				++p;
			}
			if (found) {
				break;
			}

			int32_t remaining = &buffer[maxBufferSize] - lastP;
			FPL_ASSERT(remaining >= 0);
			if (remaining > 0) {
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
		if (found) {
			char *p = line;
			while (*p) {
				if (*p == ':') {
					++p;
					// Skip whitespaces
					while (*p && isspace(*p)) {
						++p;
					}
					break;
				}
				++p;
			}
			if (p != line) {
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

fpl_platform_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	return fpl_null;
}

fpl_platform_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
	return fpl_null;
}
#endif // FPL_PLATFORM_LINUX

// ****************************************************************************
//
// > VIDEO_DRIVERS
//
// ****************************************************************************
#if !defined(FPL_VIDEO_DRIVERS_IMPLEMENTED)
#	define FPL_VIDEO_DRIVERS_IMPLEMENTED

// ############################################################################
//
// > VIDEO_DRIVER_OPENGL_WIN32
//
// ############################################################################
#if defined(FPL_ENABLE_VIDEO_OPENGL) && defined(FPL_PLATFORM_WIN32)

#define FPL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define FPL_GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define FPL_GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define FPL_GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#define FPL_GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define FPL_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

#define FPL_WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define FPL_WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define FPL_WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define FPL_WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define FPL_WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define FPL_WGL_CONTEXT_FLAGS_ARB 0x2094
#define FPL_WGL_DRAW_TO_WINDOW_ARB 0x2001
#define FPL_WGL_ACCELERATION_ARB 0x2003
#define FPL_WGL_SWAP_METHOD_ARB 0x2007
#define FPL_WGL_SUPPORT_OPENGL_ARB 0x2010
#define FPL_WGL_DOUBLE_BUFFER_ARB 0x2011
#define FPL_WGL_PIXEL_TYPE_ARB 0x2013
#define FPL_WGL_COLOR_BITS_ARB 0x2014
#define FPL_WGL_DEPTH_BITS_ARB 0x2022
#define FPL_WGL_STENCIL_BITS_ARB 0x2023
#define FPL_WGL_FULL_ACCELERATION_ARB 0x2027
#define FPL_WGL_SWAP_EXCHANGE_ARB 0x2028
#define FPL_WGL_TYPE_RGBA_ARB 0x202B

#define FPL__FUNC_WGL_MAKE_CURRENT(name) BOOL WINAPI name(HDC deviceContext, HGLRC renderingContext)
typedef FPL__FUNC_WGL_MAKE_CURRENT(fpl__win32_func_wglMakeCurrent);
#define FPL__FUNC_WGL_GET_PROC_ADDRESS(name) PROC WINAPI name(LPCSTR procedure)
typedef FPL__FUNC_WGL_GET_PROC_ADDRESS(fpl__win32_func_wglGetProcAddress);
#define FPL__FUNC_WGL_DELETE_CONTEXT(name) BOOL WINAPI name(HGLRC renderingContext)
typedef FPL__FUNC_WGL_DELETE_CONTEXT(fpl__win32_func_wglDeleteContext);
#define FPL__FUNC_WGL_CREATE_CONTEXT(name) HGLRC WINAPI name(HDC deviceContext)
typedef FPL__FUNC_WGL_CREATE_CONTEXT(fpl__win32_func_wglCreateContext);

#define FPL__FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
typedef FPL__FUNC_WGL_CHOOSE_PIXEL_FORMAT_ARB(fpl__win32_func_wglChoosePixelFormatARB);
#define FPL__FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
typedef FPL__FUNC_WGL_CREATE_CONTEXT_ATTRIBS_ARB(fpl__win32_func_wglCreateContextAttribsARB);
#define FPL__FUNC_WGL_SWAP_INTERVAL_EXT(name) BOOL WINAPI name(int interval)
typedef FPL__FUNC_WGL_SWAP_INTERVAL_EXT(fpl__win32_func_wglSwapIntervalEXT);

typedef struct fpl__Win32OpenGLApi {
	HMODULE openglLibrary;
	fpl__win32_func_wglMakeCurrent *wglMakeCurrent;
	fpl__win32_func_wglGetProcAddress *wglGetProcAddress;
	fpl__win32_func_wglDeleteContext *wglDeleteContext;
	fpl__win32_func_wglCreateContext *wglCreateContext;
	fpl__win32_func_wglChoosePixelFormatARB *wglChoosePixelFormatArb;
	fpl__win32_func_wglCreateContextAttribsARB *wglCreateContextAttribsArb;
	fpl__win32_func_wglSwapIntervalEXT *wglSwapIntervalExt;
} fpl__Win32OpenGLApi;

fpl_internal void fpl__Win32UnloadVideoOpenGLApi(fpl__Win32OpenGLApi *api) {
	if (api->openglLibrary != fpl_null) {
		FreeLibrary(api->openglLibrary);
	}
	FPL_CLEAR_STRUCT(api);
}

fpl_internal bool fpl__Win32LoadVideoOpenGLApi(fpl__Win32OpenGLApi *api) {
	const char *openglLibraryName = "opengl32.dll";

	api->openglLibrary = LoadLibraryA("opengl32.dll");
	if (api->openglLibrary == fpl_null) {
		fpl__PushError("Failed loading opengl library '%s'", openglLibraryName);
		return false;
	}

	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(api->openglLibrary, openglLibraryName, api->wglGetProcAddress, fpl__win32_func_wglGetProcAddress, "wglGetProcAddress");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(api->openglLibrary, openglLibraryName, api->wglCreateContext, fpl__win32_func_wglCreateContext, "wglCreateContext");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(api->openglLibrary, openglLibraryName, api->wglDeleteContext, fpl__win32_func_wglDeleteContext, "wglDeleteContext");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(api->openglLibrary, openglLibraryName, api->wglMakeCurrent, fpl__win32_func_wglMakeCurrent, "wglMakeCurrent");

	return true;
}

typedef struct fpl__Win32VideoOpenGLState {
	HGLRC renderingContext;
	fpl__Win32OpenGLApi api;
} fpl__Win32VideoOpenGLState;

fpl_internal bool fpl__Win32PostSetupWindowForOpenGL(fpl__Win32AppState *appState, const fpl__Win32WindowState *windowState, const fplVideoSettings *videoSettings) {
	const fpl__Win32Api *wapi = &appState->winApi;

	//
	// Prepare window for OpenGL
	//
	HDC deviceContext = windowState->deviceContext;
	HWND handle = windowState->windowHandle;

	PIXELFORMATDESCRIPTOR pfd = FPL_ZERO_INIT;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = wapi->gdi.choosePixelFormat(deviceContext, &pfd);
	if (!pixelFormat) {
		fpl__PushError("Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
		return false;
	}

	if (!wapi->gdi.setPixelFormat(deviceContext, pixelFormat, &pfd)) {
		fpl__PushError("Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
		return false;
	}

	wapi->gdi.describePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

	return true;
}

fpl_internal bool fpl__Win32InitVideoOpenGL(const fpl__Win32AppState *appState, const fpl__Win32WindowState *windowState, const fplVideoSettings *videoSettings, fpl__Win32VideoOpenGLState *glState) {
	const fpl__Win32Api *wapi = &appState->winApi;
	fpl__Win32OpenGLApi *glapi = &glState->api;

	//
	// Create opengl rendering context
	//
	HDC deviceContext = windowState->deviceContext;
	HGLRC legacyRenderingContext = glapi->wglCreateContext(deviceContext);
	if (!legacyRenderingContext) {
		fpl__PushError("Failed creating Legacy OpenGL Rendering Context for DC '%x')", deviceContext);
		return false;
	}

	if (!glapi->wglMakeCurrent(deviceContext, legacyRenderingContext)) {
		fpl__PushError("Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')", deviceContext, legacyRenderingContext);
		glapi->wglDeleteContext(legacyRenderingContext);
		return false;
	}

	// Load WGL Extensions
	glapi->wglSwapIntervalExt = (fpl__win32_func_wglSwapIntervalEXT *)glapi->wglGetProcAddress("wglSwapIntervalEXT");
	glapi->wglChoosePixelFormatArb = (fpl__win32_func_wglChoosePixelFormatARB *)glapi->wglGetProcAddress("wglChoosePixelFormatARB");
	glapi->wglCreateContextAttribsArb = (fpl__win32_func_wglCreateContextAttribsARB *)glapi->wglGetProcAddress("wglCreateContextAttribsARB");

	// Disable legacy context
	glapi->wglMakeCurrent(fpl_null, fpl_null);

	HGLRC activeRenderingContext;
	if (videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if (!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			fpl__PushError("You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			return false;
		}

		if (glapi->wglChoosePixelFormatArb == fpl_null) {
			fpl__PushError("wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
			return false;
		}
		if (glapi->wglCreateContextAttribsArb == fpl_null) {
			fpl__PushError("wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			return false;
		}

		int profile = 0;
		int flags = 0;
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			fpl__PushError("No opengl compability profile selected, please specific Core OpenGLCompabilityFlags::Core or OpenGLCompabilityFlags::Compability");
			return false;
		}
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
			flags = FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		}

		int contextAttribIndex = 0;
		int contextAttribList[20 + 1] = FPL_ZERO_INIT;
		contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MAJOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)videoSettings->graphics.opengl.majorVersion;
		contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MINOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)videoSettings->graphics.opengl.minorVersion;
		contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_PROFILE_MASK_ARB;
		contextAttribList[contextAttribIndex++] = profile;
		if (flags > 0) {
			contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}

		// Create modern opengl rendering context
		HGLRC modernRenderingContext = glapi->wglCreateContextAttribsArb(deviceContext, 0, contextAttribList);
		if (modernRenderingContext) {
			if (!glapi->wglMakeCurrent(deviceContext, modernRenderingContext)) {
				fpl__PushError("Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

				glapi->wglDeleteContext(modernRenderingContext);
				modernRenderingContext = fpl_null;

				// Fallback to legacy context
				glapi->wglMakeCurrent(deviceContext, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			} else {
				// Destroy legacy rendering context
				glapi->wglDeleteContext(legacyRenderingContext);
				legacyRenderingContext = fpl_null;
				activeRenderingContext = modernRenderingContext;
			}
		} else {
			fpl__PushError("Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

			// Fallback to legacy context
			glapi->wglMakeCurrent(deviceContext, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		}
	} else {
		// Caller wants legacy context
		glapi->wglMakeCurrent(deviceContext, legacyRenderingContext);
		activeRenderingContext = legacyRenderingContext;
	}

	FPL_ASSERT(activeRenderingContext != fpl_null);

	glState->renderingContext = activeRenderingContext;

	// Set vertical syncronisation if available
	if (glapi->wglSwapIntervalExt != fpl_null) {
		int swapInterval = videoSettings->isVSync ? 1 : 0;
		glapi->wglSwapIntervalExt(swapInterval);
	}

	return true;
}

fpl_internal void fpl__Win32ReleaseVideoOpenGL(fpl__Win32VideoOpenGLState *glState) {
	const fpl__Win32OpenGLApi *glapi = &glState->api;
	if (glState->renderingContext) {
		glapi->wglMakeCurrent(fpl_null, fpl_null);
		glapi->wglDeleteContext(glState->renderingContext);
		glState->renderingContext = fpl_null;
	}
}
#endif // FPL_ENABLE_VIDEO_OPENGL && FPL_PLATFORM_WIN32

// ############################################################################
//
// > VIDEO_DRIVER_OPENGL_X11
//
// ############################################################################
#if defined(FPL_ENABLE_VIDEO_OPENGL) && defined(FPL_SUBPLATFORM_X11)
//#	include <GL/glx.h> // XVisualInfo, GLXContext, GLXDrawable

#		define FPL__FUNC_GL_X_CHOOSE_VISUAL(name) XVisualInfo* name(Display *dpy, int screen, int *attribList)
typedef FPL__FUNC_GL_X_CHOOSE_VISUAL(fpl__func_glx_glXChooseVisual);
#		define FPL__FUNC_GL_X_CREATE_CONTEXT(name) GLXContext name(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct)
typedef FPL__FUNC_GL_X_CREATE_CONTEXT(fpl__func_glx_glXCreateContext);
#		define FPL__FUNC_GL_X_CREATE_NEW_CONTEXT(name) GLXContext name(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct)
typedef FPL__FUNC_GL_X_CREATE_NEW_CONTEXT(fpl__func_glx_glXCreateNewContext);
#		define FPL__FUNC_GL_X_DESTROY_CONTEXT(name) void name(Display *dpy, GLXContext ctx)
typedef FPL__FUNC_GL_X_DESTROY_CONTEXT(fpl__func_glx_glXDestroyContext);
#		define FPL__FUNC_GL_X_MAKE_CURRENT(name) Bool name(Display *dpy, GLXDrawable drawable, GLXContext ctx)
typedef FPL__FUNC_GL_X_MAKE_CURRENT(fpl__func_glx_glXMakeCurrent);
#		define FPL__FUNC_GL_X_SWAP_BUFFERS(name) void name(Display *dpy, GLXDrawable drawable)
typedef FPL__FUNC_GL_X_SWAP_BUFFERS(fpl__func_glx_glXSwapBuffers);
#		define FPL__FUNC_GL_X_GET_PROC_ADDRESS(name) void *name(const GLubyte *procName)
typedef FPL__FUNC_GL_X_GET_PROC_ADDRESS(fpl__func_glx_glXGetProcAddress);
#		define FPL__FUNC_GL_X_CHOOSE_FB_CONFIG(name) GLXFBConfig *name(Display *dpy, int screen, const int *attrib_list, int *nelements)
typedef FPL__FUNC_GL_X_CHOOSE_FB_CONFIG(fpl__func_glx_glXChooseFBConfig);
#		define FPL__FUNC_GL_X_GET_FB_CONFIGS(name) GLXFBConfig *name(Display *dpy, int screen, int *nelements)
typedef FPL__FUNC_GL_X_GET_FB_CONFIGS(fpl__func_glx_glXGetFBConfigs);
#		define FPL__FUNC_GL_X_GET_VISUAL_FROM_FB_CONFIG(name) XVisualInfo *name(Display *dpy, GLXFBConfig config)
typedef FPL__FUNC_GL_X_GET_VISUAL_FROM_FB_CONFIG(fpl__func_glx_glXGetVisualFromFBConfig);
#		define FPL__FUNC_GL_X_GET_FB_CONFIG_ATTRIB(name) int name(Display *dpy, GLXFBConfig config, int attribute, int *value)
typedef FPL__FUNC_GL_X_GET_FB_CONFIG_ATTRIB(fpl__func_glx_glXGetFBConfigAttrib);
#		define FPL__FUNC_GL_X_CREATE_WINDOW(name) GLXWindow name(Display *dpy, GLXFBConfig config, Window win,  const int *attrib_list)
typedef FPL__FUNC_GL_X_CREATE_WINDOW(fpl__func_glx_glXCreateWindow);
#		define FPL__FUNC_GL_X_QUERY_EXTENSION(name) Bool name(Display *dpy, int *errorBase, int *eventBase)
typedef FPL__FUNC_GL_X_QUERY_EXTENSION(fpl__func_glx_glXQueryExtension);
#		define FPL__FUNC_GL_X_QUERY_EXTENSIONS_STRING(name) const char *name(Display *dpy, int screen)
typedef FPL__FUNC_GL_X_QUERY_EXTENSIONS_STRING(fpl__func_glx_glXQueryExtensionsString);

struct X11VideoOpenGLApi {
	void *libHandle;
	fpl__func_glx_glXChooseVisual *glXChooseVisual;
	fpl__func_glx_glXCreateContext *glXCreateContext;
	fpl__func_glx_glXDestroyContext *glXDestroyContext;
	fpl__func_glx_glXCreateNewContext *glXCreateNewContext;
	fpl__func_glx_glXMakeCurrent *glXMakeCurrent;
	fpl__func_glx_glXSwapBuffers *glXSwapBuffers;
	fpl__func_glx_glXGetProcAddress *glXGetProcAddress;
	fpl__func_glx_glXChooseFBConfig *glXChooseFBConfig;
	fpl__func_glx_glXGetFBConfigs *glXGetFBConfigs;
	fpl__func_glx_glXGetVisualFromFBConfig *glXGetVisualFromFBConfig;
	fpl__func_glx_glXGetFBConfigAttrib *glXGetFBConfigAttrib;
	fpl__func_glx_glXCreateWindow *glXCreateWindow;
	fpl__func_glx_glXQueryExtension *glXQueryExtension;
	fpl__func_glx_glXQueryExtensionsString *glXQueryExtensionsString;
};

fpl_internal void X11UnloadVideoOpenGLApi(X11VideoOpenGLApi &api) {
	FPL_LOG_BLOCK;

	if (api.libHandle != fpl_null) {
		FPL_LOG("GLX", "Unload Api (Library '%p')", api.libHandle);
		::dlclose(api.libHandle);
	}
	api = {};
}

fpl_internal bool X11LoadVideoOpenGLApi(X11VideoOpenGLApi &api) {
	FPL_LOG_BLOCK;

	const char* libFileNames[] = {
		"libGL.so.1",
		"libGL.so",
	};
	bool result = false;
	for (uint32_t index = 0; index < FPL_ARRAYCOUNT(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		FPL_LOG("GLX", "Load GLX Api from Library: %s", libName);
		void *libHandle = api.libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if (libHandle != fpl_null) {
			FPL_LOG("GLX", "Library Found: '%s', Resolving Procedures", libName);
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXChooseVisual, fpl__func_glx_glXChooseVisual, "glXChooseVisual");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXCreateContext, fpl__func_glx_glXCreateContext, "glXCreateContext");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXDestroyContext, fpl__func_glx_glXDestroyContext, "glXDestroyContext");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXCreateNewContext, fpl__func_glx_glXCreateNewContext, "glXCreateNewContext");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXMakeCurrent, fpl__func_glx_glXMakeCurrent, "glXMakeCurrent");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXSwapBuffers, fpl__func_glx_glXSwapBuffers, "glXSwapBuffers");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXGetProcAddress, fpl__func_glx_glXGetProcAddress, "glXGetProcAddress");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXChooseFBConfig, fpl__func_glx_glXChooseFBConfig, "glXChooseFBConfig");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXGetFBConfigs, fpl__func_glx_glXGetFBConfigs, "glXGetFBConfigs");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXGetVisualFromFBConfig, fpl__func_glx_glXGetVisualFromFBConfig, "glXGetVisualFromFBConfig");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXGetFBConfigAttrib, fpl__func_glx_glXGetFBConfigAttrib, "glXGetFBConfigAttrib");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXCreateWindow, fpl__func_glx_glXCreateWindow, "glXCreateWindow");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXQueryExtension, fpl__func_glx_glXQueryExtension, "glXQueryExtension");
			FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api.glXQueryExtensionsString, fpl__func_glx_glXQueryExtensionsString, "glXQueryExtensionsString");
			FPL_LOG("GLX", "Successfully loaded GLX Api from Library '%s'", libName);
			result = true;
			break;
		}
		X11UnloadVideoOpenGLApi(api);
	}
	return (result);
}

struct X11VideoOpenGLState {
	X11VideoOpenGLApi api;
	GLXFBConfig fbConfig;
	GLXContext context;
	bool isActiveContext;
};

fpl_internal_inline bool X11SetPreWindowSetupForOpenGL(const subplatform_x11::X11Api &x11Api, const subplatform_x11::X11WindowState &windowState, const X11VideoOpenGLState &glState, subplatform_x11::X11PreWindowSetupResult &outResult) {
	const X11VideoOpenGLApi &glApi = glState.api;
	FPL_ASSERT(glState.fbConfig != fpl_null);

	FPL_LOG("GLX", "Get visual info from display '%p' and frame buffer config '%p'", windowState.display, glState.fbConfig);
	XVisualInfo *visualInfo = glApi.glXGetVisualFromFBConfig(windowState.display, glState.fbConfig);
	if (visualInfo == fpl_null) {
		FPL_LOG("GLX", "Failed getting visual info from display '%p' and frame buffer config '%p'", windowState.display, glState.fbConfig);
		return false;
	}
	FPL_LOG("GLX", "Successfully got visual info from display '%p' and frame buffer config '%p': %p", windowState.display, glState.fbConfig, visualInfo);

	FPL_LOG("GLX", "Using visual: %p", visualInfo->visual);
	FPL_LOG("GLX", "Using color depth: %d", visualInfo->depth);

	outResult.visual = visualInfo->visual;
	outResult.colorDepth = visualInfo->depth;

	FPL_LOG("GLX", "Release visual info '%p'", visualInfo);
	x11Api.XFree(visualInfo);

	return true;
}

fpl_internal bool X11InitFrameBufferConfigVideoOpenGL(const subplatform_x11::X11Api &x11Api, const subplatform_x11::X11WindowState &windowState, X11VideoOpenGLState &glState) {
	const X11VideoOpenGLApi &glApi = glState.api;

	FPL_LOG("GLX", "Query OpenGL extension on display '%p'", windowState.display);
	if (!glApi.glXQueryExtension(windowState.display, fpl_null, fpl_null)) {
		FPL_LOG("GLX", "OpenGL GLX Extension is not supported by the active display '%p'", windowState.display);
		return false;
	}

	// @NOTE(final): Required for AMD Catalyst Drivers?
	const char *extensionString = glApi.glXQueryExtensionsString(windowState.display, windowState.screen);
	if (extensionString != fpl_null) {
		FPL_LOG("GLX", "OpenGL GLX extensions: %s", extensionString);
	}

	int attr[32];
	int* p = attr;
	*p++ = GLX_X_VISUAL_TYPE; *p++ = GLX_TRUE_COLOR;
	*p++ = GLX_DOUBLEBUFFER;  *p++ = True;
	*p++ = GLX_RED_SIZE;      *p++ = 8;
	*p++ = GLX_GREEN_SIZE;    *p++ = 8;
	*p++ = GLX_BLUE_SIZE;     *p++ = 8;
	*p++ = GLX_DEPTH_SIZE;    *p++ = 24;
	*p++ = GLX_STENCIL_SIZE;  *p++ = 8;
	*p++ = 0;

	FPL_LOG("GLX", "Get framebuffer configuration from display '%p' and screen '%d'", windowState.display, windowState.screen);
	int configCount = 0;
	GLXFBConfig *configs = glApi.glXChooseFBConfig(windowState.display, windowState.screen, attr, &configCount);
	if (configs == fpl_null || !configCount) {
		FPL_LOG("GLX", "No framebuffer configuration from display '%p' and screen '%d' found!", windowState.display, windowState.screen);
		glState.fbConfig = fpl_null;
		return false;
	}
	glState.fbConfig = configs[0];
	FPL_LOG("GLX", "Successfully got framebuffer configuration from display '%p' and screen '%d': %p", windowState.display, windowState.screen, glState.fbConfig);

	FPL_LOG("GLX", "Release %d framebuffer configurations", configCount);
	x11Api.XFree(configs);

	return true;
}

fpl_internal void X11ReleaseVideoOpenGL(const subplatform_x11::X11WindowState &windowState, X11VideoOpenGLState &glState) {
	const X11VideoOpenGLApi &glApi = glState.api;

	if (glState.isActiveContext) {
		FPL_LOG("GLX", "Deactivate GLX rendering context for display '%p'", windowState.display);
		glApi.glXMakeCurrent(windowState.display, 0, fpl_null);
		glState.isActiveContext = false;
	}

	if (glState.context != fpl_null) {
		FPL_LOG("GLX", "Destroy GLX rendering context '%p' for display '%p'", glState.context, windowState.display);
		glApi.glXDestroyContext(windowState.display, glState.context);
		glState.context = fpl_null;
	}
}

fpl_internal bool X11InitVideoOpenGL(const subplatform_x11::X11SubplatformState &subplatform, const subplatform_x11::X11WindowState &windowState, const VideoSettings &videoSettings, X11VideoOpenGLState &glState) {
	const X11VideoOpenGLApi &glApi = glState.api;
	const subplatform_x11::X11Api &x11Api = subplatform.api;

	if (glState.fbConfig == fpl_null) {
		FPL_LOG("GLX", "No frame buffer configuration found");
		return false;
	}

#define USE_NEW_CTX 0

#if !USE_NEW_CTX
	FPL_LOG("GLX", "Get visual info from display '%p' and frame buffer config '%p'", windowState.display, glState.fbConfig);
	XVisualInfo *visualInfo = glApi.glXGetVisualFromFBConfig(windowState.display, glState.fbConfig);
	if (visualInfo == fpl_null) {
		FPL_LOG("GLX", "Failed getting visual info from display '%p' and frame buffer config '%p'", windowState.display, glState.fbConfig);
		return false;
	}
	FPL_LOG("GLX", "Successfully got visual info from display '%p' and frame buffer config '%p': %p", windowState.display, glState.fbConfig, visualInfo);
#endif

	bool result = false;

#if USE_NEW_CTX
	FPL_LOG("GLX", "Create GLX rendering context on display '%p' and frame buffer config '%p'", windowState.display, glState.fbConfig);
	glState.context = glApi.glXCreateNewContext(windowState.display, glState.fbConfig, GLX_RGBA_TYPE, fpl_null, GL_TRUE);
#else
	FPL_LOG("GLX", "Create GLX rendering context on display '%p' and visual info '%p'", windowState.display, visualInfo);
	glState.context = glApi.glXCreateContext(windowState.display, visualInfo, fpl_null, GL_TRUE);
#endif

	if (glState.context != fpl_null) {
		FPL_LOG("GLX", "Activate GLX rendering context '%p' on display '%p' and window '%d'", glState.context, windowState.display, (int)windowState.window);
		if (glApi.glXMakeCurrent(windowState.display, windowState.window, glState.context)) {
			FPL_LOG("GLX", "Successfully activated GLX rendering context '%p' on display '%p' and window '%d'", glState.context, windowState.display, (int)windowState.window);
			glState.isActiveContext = true;
			result = true;
		} else {
			FPL_LOG("GLX", "Failed activating GLX rendering context '%p' on display '%p' and window '%d'", glState.context, windowState.display, (int)windowState.window);
		}
	} else {
		FPL_LOG("GLX", "Failed creating GLX rendering context on display '%p' and visual info '%p'", windowState.display, visualInfo);
	}

#if !USE_NEW_CTX
	FPL_LOG("GLX", "Release visual info '%p'", visualInfo);
	x11Api.XFree(visualInfo);
#endif

	if (!result) {
		X11ReleaseVideoOpenGL(windowState, glState);
	}
	return (result);
}
#endif // FPL_ENABLE_VIDEO_OPENGL && FPL_SUBPLATFORM_X11

// ############################################################################
//
// > VIDEO_DRIVER_SOFTWARE_WIN32
//
// ############################################################################
#if defined(FPL_ENABLE_VIDEO_SOFTWARE) && defined(FPL_PLATFORM_WIN32)
typedef struct fpl__Win32VideoSoftwareState {
	BITMAPINFO bitmapInfo;
} fpl__Win32VideoSoftwareState;

fpl_internal bool fpl__Win32InitVideoSoftware(const fplVideoBackBuffer *backbuffer, fpl__Win32VideoSoftwareState *software) {
	FPL_CLEAR_STRUCT(software);
	software->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	software->bitmapInfo.bmiHeader.biWidth = (LONG)backbuffer->width;
	software->bitmapInfo.bmiHeader.biHeight = (LONG)backbuffer->height;
	software->bitmapInfo.bmiHeader.biBitCount = 32;
	software->bitmapInfo.bmiHeader.biCompression = BI_RGB;
	software->bitmapInfo.bmiHeader.biPlanes = 1;
	software->bitmapInfo.bmiHeader.biSizeImage = (DWORD)(backbuffer->height * backbuffer->lineWidth);
	return true;
}

fpl_internal void fpl__Win32ReleaseVideoSoftware(fpl__Win32VideoSoftwareState *software) {
	FPL_CLEAR_STRUCT(software);
}
#endif // FPL_ENABLE_VIDEO_SOFTWARE && FPL_PLATFORM_WIN32

#endif // FPL_VIDEO_DRIVERS_IMPLEMENTED

// ****************************************************************************
//
// > AUDIO_DRIVERS
//
// ****************************************************************************
#if !defined(FPL_AUDIO_DRIVERS_IMPLEMENTED)
#	define FPL_AUDIO_DRIVERS_IMPLEMENTED

// Global Audio GUIDs
#if defined(FPL_PLATFORM_WIN32)
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
#endif

// ############################################################################
//
// > AUDIO_DRIVER_DIRECTSOUND
//
// ############################################################################
#if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
#	include <mmreg.h>
#	include <dsound.h>

#define FPL__FUNC_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(const GUID* pcGuidDevice, LPDIRECTSOUND *ppDS8, LPUNKNOWN pUnkOuter)
typedef FPL__FUNC_DIRECT_SOUND_CREATE(func_DirectSoundCreate);
#define FPL__FUNC_DIRECT_SOUND_ENUMERATE_A(name) HRESULT WINAPI name(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
typedef FPL__FUNC_DIRECT_SOUND_ENUMERATE_A(func_DirectSoundEnumerateA);
static GUID FPL__IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16} };
#define FPL__DIRECTSOUND_MAX_PERIODS 4

typedef struct fpl__DirectSoundState {
	HMODULE dsoundLibrary;
	LPDIRECTSOUND directSound;
	LPDIRECTSOUNDBUFFER primaryBuffer;
	LPDIRECTSOUNDBUFFER secondaryBuffer;
	LPDIRECTSOUNDNOTIFY notify;
	HANDLE notifyEvents[FPL__DIRECTSOUND_MAX_PERIODS];
	HANDLE stopEvent;
	uint32_t lastProcessedFrame;
	bool breakMainLoop;
} fpl__DirectSoundState;

typedef struct fpl__DirectSoundDeviceInfos {
	uint32_t foundDeviceCount;
	fplAudioDeviceInfo *deviceInfos;
	uint32_t maxDeviceCount;
} fpl__DirectSoundDeviceInfos;

fpl_internal BOOL CALLBACK fpl__GetDeviceCallbackDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext) {
	(void)lpcstrModule;

	fpl__DirectSoundDeviceInfos *infos = (fpl__DirectSoundDeviceInfos *)lpContext;
	FPL_ASSERT(infos != fpl_null);
	if (infos->deviceInfos != fpl_null) {
		uint32_t index = infos->foundDeviceCount++;
		if (index < infos->maxDeviceCount) {
			fplAudioDeviceInfo *deviceInfo = infos->deviceInfos + index;
			FPL_CLEAR_STRUCT(deviceInfo);
			fplCopyAnsiString(lpcstrDescription, deviceInfo->name, FPL_ARRAYCOUNT(deviceInfo->name));
			if (lpGuid != fpl_null) {
				fplMemoryCopy(lpGuid, sizeof(deviceInfo->id.dshow), &deviceInfo->id.dshow);
			}
			return TRUE;
		}
	}
	return FALSE;
}

fpl_internal uint32_t fpl__GetDevicesDirectSound(fpl__DirectSoundState *dsoundState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
	uint32_t result = 0;
	func_DirectSoundEnumerateA *directSoundEnumerateA = (func_DirectSoundEnumerateA *)GetProcAddress(dsoundState->dsoundLibrary, "DirectSoundEnumerateA");
	if (directSoundEnumerateA != fpl_null) {
		fpl__DirectSoundDeviceInfos infos = FPL_ZERO_INIT;
		infos.maxDeviceCount = maxDeviceCount;
		infos.deviceInfos = deviceInfos;
		directSoundEnumerateA(fpl__GetDeviceCallbackDirectSound, &infos);
		result = infos.foundDeviceCount;
	}
	return(result);
}

fpl_internal bool fpl__ReleaseDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
	if (dsoundState->dsoundLibrary != fpl_null) {
		if (dsoundState->stopEvent != fpl_null) {
			CloseHandle(dsoundState->stopEvent);
		}

		for (uint32_t i = 0; i < commonAudio->internalFormat.periods; ++i) {
			if (dsoundState->notifyEvents[i]) {
				CloseHandle(dsoundState->notifyEvents[i]);
			}
		}

		if (dsoundState->notify != fpl_null) {
			IDirectSoundNotify_Release(dsoundState->notify);
		}

		if (dsoundState->secondaryBuffer != fpl_null) {
			IDirectSoundBuffer_Release(dsoundState->secondaryBuffer);
		}

		if (dsoundState->primaryBuffer != fpl_null) {
			IDirectSoundBuffer_Release(dsoundState->primaryBuffer);
		}

		if (dsoundState->directSound != fpl_null) {
			IDirectSound_Release(dsoundState->directSound);
		}

		FreeLibrary(dsoundState->dsoundLibrary);
		FPL_CLEAR_STRUCT(dsoundState);
	}

	return true;
}

fpl_internal fplAudioResult fpl__InitDirectSound(const fplAudioSettings *audioSettings, fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
#ifdef __cplusplus
	GUID guid_IID_IDirectSoundNotify = FPL__IID_IDirectSoundNotify;
#else
	GUID* guid_IID_IDirectSoundNotify = &FPL__IID_IDirectSoundNotify;
#endif

	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32Api *apiFuncs = &win32AppState->winApi;

	// Load direct sound library
	dsoundState->dsoundLibrary = LoadLibraryA("dsound.dll");
	func_DirectSoundCreate *directSoundCreate = (func_DirectSoundCreate *)GetProcAddress(dsoundState->dsoundLibrary, "DirectSoundCreate");
	if (directSoundCreate == fpl_null) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Load direct sound object
	const GUID *deviceId = fpl_null;
	if (fplGetAnsiStringLength(audioSettings->deviceInfo.name) > 0) {
		deviceId = &audioSettings->deviceInfo.id.dshow;
	}
	if (!SUCCEEDED(directSoundCreate(deviceId, &dsoundState->directSound, fpl_null))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Setup wave format ex
	WAVEFORMATEXTENSIBLE wf = FPL_ZERO_INIT;
	wf.Format.cbSize = sizeof(wf);
	wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wf.Format.nChannels = (WORD)audioSettings->deviceFormat.channels;
	wf.Format.nSamplesPerSec = (DWORD)audioSettings->deviceFormat.sampleRate;
	wf.Format.wBitsPerSample = fplGetAudioSampleSizeInBytes(audioSettings->deviceFormat.type) * 8;
	wf.Format.nBlockAlign = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
	wf.Format.nAvgBytesPerSec = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
	wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
	if ((audioSettings->deviceFormat.type == fplAudioFormatType_F32) || (audioSettings->deviceFormat.type == fplAudioFormatType_F64)) {
		wf.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	} else {
		wf.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM;
	}

	// Get either local window handle or desktop handle
	HWND windowHandle = fpl_null;
#		if defined(FPL_ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_Window) {
		windowHandle = appState->window.win32.windowHandle;
	}
#		endif
	if (windowHandle == fpl_null) {
		windowHandle = apiFuncs->user.getDesktopWindow();
	}

	// The cooperative level must be set before doing anything else
	if (FAILED(IDirectSound_SetCooperativeLevel(dsoundState->directSound, windowHandle, (audioSettings->preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create primary buffer
	DSBUFFERDESC descDSPrimary = FPL_ZERO_INIT;
	descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
	descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	if (FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDSPrimary, &dsoundState->primaryBuffer, fpl_null))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Set format
	if (FAILED(IDirectSoundBuffer_SetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)&wf))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get the required size in bytes
	DWORD requiredSize;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, fpl_null, 0, &requiredSize))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get actual format
	char rawdata[1024];
	WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)pActualFormat, requiredSize, fpl_null))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Set internal format
	fplAudioDeviceFormat internalFormat = FPL_ZERO_INIT;
	if (fpl__Win32IsEqualGuid(pActualFormat->SubFormat, FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
		if (pActualFormat->Format.wBitsPerSample == 64) {
			internalFormat.type = fplAudioFormatType_F64;
		} else {
			internalFormat.type = fplAudioFormatType_F32;
		}
	} else {
		switch (pActualFormat->Format.wBitsPerSample) {
			case 8:
				internalFormat.type = fplAudioFormatType_U8;
				break;
			case 16:
				internalFormat.type = fplAudioFormatType_S16;
				break;
			case 24:
				internalFormat.type = fplAudioFormatType_S24;
				break;
			case 32:
				internalFormat.type = fplAudioFormatType_S32;
				break;
			case 64:
				internalFormat.type = fplAudioFormatType_S64;
				break;
		}
	}
	internalFormat.channels = pActualFormat->Format.nChannels;
	internalFormat.sampleRate = pActualFormat->Format.nSamplesPerSec;

	// @NOTE(final): We divide up our playback buffer into this number of periods and let directsound notify us when one of it needs to play.
	internalFormat.periods = 2;
	internalFormat.bufferSizeInFrames = fplGetAudioBufferSizeInFrames(internalFormat.sampleRate, audioSettings->bufferSizeInMilliSeconds);
	internalFormat.bufferSizeInBytes = internalFormat.bufferSizeInFrames * internalFormat.channels * fplGetAudioSampleSizeInBytes(internalFormat.type);

	commonAudio->internalFormat = internalFormat;

	// Create secondary buffer
	DSBUFFERDESC descDS = FPL_ZERO_INIT;
	descDS.dwSize = sizeof(DSBUFFERDESC);
	descDS.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	descDS.dwBufferBytes = (DWORD)internalFormat.bufferSizeInBytes;
	descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;
	if (FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDS, &dsoundState->secondaryBuffer, fpl_null))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
	if (FAILED(IDirectSoundBuffer_QueryInterface(dsoundState->secondaryBuffer, guid_IID_IDirectSoundNotify, (void**)&dsoundState->notify))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Setup notifications
	uint32_t periodSizeInBytes = internalFormat.bufferSizeInBytes / internalFormat.periods;
	DSBPOSITIONNOTIFY notifyPoints[FPL__DIRECTSOUND_MAX_PERIODS];
	for (uint32_t i = 0; i < internalFormat.periods; ++i) {
		dsoundState->notifyEvents[i] = CreateEventA(fpl_null, false, false, fpl_null);
		if (dsoundState->notifyEvents[i] == fpl_null) {
			fpl__ReleaseDirectSound(commonAudio, dsoundState);
			return fplAudioResult_Failed;
		}

		// The notification offset is in bytes.
		notifyPoints[i].dwOffset = i * periodSizeInBytes;
		notifyPoints[i].hEventNotify = dsoundState->notifyEvents[i];
	}
	if (FAILED(IDirectSoundNotify_SetNotificationPositions(dsoundState->notify, internalFormat.periods, notifyPoints))) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create stop event
	dsoundState->stopEvent = CreateEventA(fpl_null, false, false, fpl_null);
	if (dsoundState->stopEvent == fpl_null) {
		fpl__ReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	return fplAudioResult_Success;
}

fpl_internal_inline void fpl__StopMainLoopDirectSound(fpl__DirectSoundState *dsoundState) {
	dsoundState->breakMainLoop = true;
	SetEvent(dsoundState->stopEvent);
}

fpl_internal bool fpl__GetCurrentFrameDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState, uint32_t* pCurrentPos) {
	FPL_ASSERT(pCurrentPos != fpl_null);
	*pCurrentPos = 0;

	FPL_ASSERT(dsoundState->secondaryBuffer != fpl_null);
	DWORD dwCurrentPosition;
	if (FAILED(IDirectSoundBuffer_GetCurrentPosition(dsoundState->secondaryBuffer, fpl_null, &dwCurrentPosition))) {
		return false;
	}

	FPL_ASSERT(commonAudio->internalFormat.channels > 0);
	*pCurrentPos = (uint32_t)dwCurrentPosition / fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type) / commonAudio->internalFormat.channels;
	return true;
}

fpl_internal uint32_t fpl__GetAvailableFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
	// Get current frame from current play position
	uint32_t currentFrame;
	if (!fpl__GetCurrentFrameDirectSound(commonAudio, dsoundState, &currentFrame)) {
		return 0;
	}

	// In a playback device the last processed frame should always be ahead of the current frame. The space between
	// the last processed and current frame (moving forward, starting from the last processed frame) is the amount
	// of space available to write.
	uint32_t totalFrameCount = commonAudio->internalFormat.bufferSizeInFrames;
	uint32_t committedBeg = currentFrame;
	uint32_t committedEnd;
	committedEnd = dsoundState->lastProcessedFrame;
	if (committedEnd <= committedBeg) {
		committedEnd += totalFrameCount;
	}

	uint32_t committedSize = (committedEnd - committedBeg);
	FPL_ASSERT(committedSize <= totalFrameCount);

	return totalFrameCount - committedSize;
}

fpl_internal uint32_t fpl__WaitForFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
	FPL_ASSERT(commonAudio->internalFormat.sampleRate > 0);
	FPL_ASSERT(commonAudio->internalFormat.periods > 0);

	// The timeout to use for putting the thread to sleep is based on the size of the buffer and the period count.
	DWORD timeoutInMilliseconds = (commonAudio->internalFormat.bufferSizeInFrames / (commonAudio->internalFormat.sampleRate / 1000)) / commonAudio->internalFormat.periods;
	if (timeoutInMilliseconds < 1) {
		timeoutInMilliseconds = 1;
	}

	// Copy event handles so we can wait for each one
	unsigned int eventCount = commonAudio->internalFormat.periods + 1;
	HANDLE pEvents[FPL__DIRECTSOUND_MAX_PERIODS + 1]; // +1 for the stop event.
	fplMemoryCopy(dsoundState->notifyEvents, sizeof(HANDLE) * commonAudio->internalFormat.periods, pEvents);
	pEvents[eventCount - 1] = dsoundState->stopEvent;

	while (!dsoundState->breakMainLoop) {
		// Get available frames from directsound
		uint32_t framesAvailable = fpl__GetAvailableFramesDirectSound(commonAudio, dsoundState);
		if (framesAvailable > 0) {
			return framesAvailable;
		}

		// If we get here it means we weren't able to find any frames. We'll just wait here for a bit.
		WaitForMultipleObjects(eventCount, pEvents, FALSE, timeoutInMilliseconds);
	}

	// We'll get here if the loop was terminated. Just return whatever's available.
	return fpl__GetAvailableFramesDirectSound(commonAudio, dsoundState);
}

fpl_internal bool fpl__StopDirectSound(fpl__DirectSoundState *dsoundState) {
	FPL_ASSERT(dsoundState->secondaryBuffer != fpl_null);
	if (FAILED(IDirectSoundBuffer_Stop(dsoundState->secondaryBuffer))) {
		return false;
	}
	IDirectSoundBuffer_SetCurrentPosition(dsoundState->secondaryBuffer, 0);
	return true;
}

fpl_internal fplAudioResult fpl__StartDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
	FPL_ASSERT(commonAudio->internalFormat.channels > 0);
	FPL_ASSERT(commonAudio->internalFormat.periods > 0);
	uint32_t audioSampleSizeBytes = fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type);
	FPL_ASSERT(audioSampleSizeBytes > 0);

	// Before playing anything we need to grab an initial group of samples from the client.
	uint32_t framesToRead = commonAudio->internalFormat.bufferSizeInFrames / commonAudio->internalFormat.periods;
	uint32_t desiredLockSize = framesToRead * commonAudio->internalFormat.channels * audioSampleSizeBytes;

	void* pLockPtr;
	DWORD actualLockSize;
	void* pLockPtr2;
	DWORD actualLockSize2;

	if (SUCCEEDED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, 0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
		framesToRead = actualLockSize / audioSampleSizeBytes / commonAudio->internalFormat.channels;
		fpl__ReadAudioFramesFromClient(commonAudio, framesToRead, pLockPtr);
		IDirectSoundBuffer_Unlock(dsoundState->secondaryBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
		dsoundState->lastProcessedFrame = framesToRead;
		if (FAILED(IDirectSoundBuffer_Play(dsoundState->secondaryBuffer, 0, 0, DSBPLAY_LOOPING))) {
			return fplAudioResult_Failed;
		}
	} else {
		return fplAudioResult_Failed;
	}
	return fplAudioResult_Success;
}

fpl_internal void fpl__DirectSoundMainLoop(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundState *dsoundState) {
	FPL_ASSERT(commonAudio->internalFormat.channels > 0);
	uint32_t audioSampleSizeBytes = fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type);
	FPL_ASSERT(audioSampleSizeBytes > 0);

	// Make sure the stop event is not signaled to ensure we don't end up immediately returning from WaitForMultipleObjects().
	ResetEvent(dsoundState->stopEvent);

	// Main loop
	dsoundState->breakMainLoop = false;
	while (!dsoundState->breakMainLoop) {
		// Wait until we get available frames from directsound
		uint32_t framesAvailable = fpl__WaitForFramesDirectSound(commonAudio, dsoundState);
		if (framesAvailable == 0) {
			continue;
		}

		// Don't bother grabbing more data if the device is being stopped.
		if (dsoundState->breakMainLoop) {
			break;
		}

		// Lock playback buffer
		DWORD lockOffset = dsoundState->lastProcessedFrame * commonAudio->internalFormat.channels * audioSampleSizeBytes;
		DWORD lockSize = framesAvailable * commonAudio->internalFormat.channels * audioSampleSizeBytes;
		{
			void* pLockPtr;
			DWORD actualLockSize;
			void* pLockPtr2;
			DWORD actualLockSize2;
			if (FAILED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
				// @TODO(final): Handle error
				break;
			}

			// Read actual frames from user
			uint32_t frameCount = actualLockSize / audioSampleSizeBytes / commonAudio->internalFormat.channels;
			fpl__ReadAudioFramesFromClient(commonAudio, frameCount, pLockPtr);
			dsoundState->lastProcessedFrame = (dsoundState->lastProcessedFrame + frameCount) % commonAudio->internalFormat.bufferSizeInFrames;

			// Unlock playback buffer
			IDirectSoundBuffer_Unlock(dsoundState->secondaryBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
		}
	}
}
#endif // FPL_ENABLE_AUDIO_DIRECTSOUND

#endif // FPL_AUDIO_DRIVERS_IMPLEMENTED

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_AUDIO_L1 (Audio System, Private Implementation)
//
// The audio system is based on a stripped down version of "mini_al.h" by David Reid.
// See: https://github.com/dr-soft/mini_al
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_ENABLE_AUDIO)
typedef enum fpl__AudioDeviceState {
	fpl__AudioDeviceState_Uninitialized = 0,
	fpl__AudioDeviceState_Stopped,
	fpl__AudioDeviceState_Started,
	fpl__AudioDeviceState_Starting,
	fpl__AudioDeviceState_Stopping,
} fpl__AudioDeviceState;

typedef struct fpl__AudioState {
	fpl__CommonAudioState common;

	fplMutexHandle lock;
	fplThreadHandle *workerThread;
	fplSignalHandle startSignal;
	fplSignalHandle stopSignal;
	fplSignalHandle wakeupSignal;
	volatile fpl__AudioDeviceState state;
	volatile fplAudioResult workResult;

	fplAudioDriverType activeDriver;
	uint32_t periods;
	uint32_t bufferSizeInFrames;
	uint32_t bufferSizeInBytes;
	bool isAsyncDriver;

	union {
#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		fpl__DirectSoundState dsound;
#	endif
	};
} fpl__AudioState;

fpl_internal_inline fpl__AudioState *fpl__GetAudioState(fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__AudioState *result = fpl_null;
	if (appState->audio.mem != fpl_null) {
		result = (fpl__AudioState *)appState->audio.mem;
	}
	return(result);
}

fpl_internal void fpl__AudioDeviceStopMainLoop(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			fpl__StopMainLoopDirectSound(&audioState->dsound);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal bool fpl__AudioReleaseDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	bool result = false;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__ReleaseDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal bool fpl__AudioStopDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	bool result = false;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__StopDirectSound(&audioState->dsound);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal fplAudioResult fpl__AudioStartDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	fplAudioResult result = fplAudioResult_Failed;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__StartDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal void fpl__AudioDeviceMainLoop(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			fpl__DirectSoundMainLoop(&audioState->common, &audioState->dsound);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal_inline bool fpl__IsAudioDriverAsync(fplAudioDriverType audioDriver) {
	switch (audioDriver) {
		case fplAudioDriverType_DirectSound:
			return false;
		default:
			return false;
	}
}

fpl_internal_inline void fpl__AudioSetDeviceState(fpl__AudioState *audioState, fpl__AudioDeviceState newState) {
	fplAtomicStoreU32((volatile uint32_t *)&audioState->state, (uint32_t)newState);
}

fpl_internal_inline fpl__AudioDeviceState fpl__AudioGetDeviceState(fpl__AudioState *audioState) {
	fpl__AudioDeviceState result = (fpl__AudioDeviceState)fplAtomicLoadU32((volatile uint32_t *)&audioState->state);
	return(result);
}

fpl_internal_inline bool fpl__IsAudioDeviceInitialized(fpl__AudioState *audioState) {
	if (audioState == fpl_null) {
		return false;
	}
	fpl__AudioDeviceState state = fpl__AudioGetDeviceState(audioState);
	return(state != fpl__AudioDeviceState_Uninitialized);
}

fpl_internal_inline bool fpl__IsAudioDeviceStarted(fpl__AudioState *audioState) {
	if (audioState == fpl_null) {
		return false;
	}
	fpl__AudioDeviceState state = fpl__AudioGetDeviceState(audioState);
	return(state == fpl__AudioDeviceState_Started);
}

fpl_internal void fpl__AudioWorkerThread(const fplThreadHandle *thread, void *data) {
#if defined(FPL_PLATFORM_WIN32)
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	fpl__AudioState *audioState = (fpl__AudioState *)data;
	FPL_ASSERT(audioState != fpl_null);
	FPL_ASSERT(audioState->activeDriver != fplAudioDriverType_None);

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.coInitializeEx(fpl_null, 0);
#endif

	for (;;) {
		// Stop the device at the start of the iteration always
		fpl__AudioStopDevice(audioState);

		// Let the other threads know that the device has been stopped.
		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Stopped);
		fplSignalSet(&audioState->stopSignal);

		// We wait until the audio device gets wake up
		fplSignalWaitForOne(&audioState->lock, &audioState->wakeupSignal, UINT32_MAX);

		// Default result code.
		audioState->workResult = fplAudioResult_Success;

		// Just break if we're terminating.
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Uninitialized) {
			break;
		}

		// Expect that the device is currently be started by the client
		FPL_ASSERT(fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Starting);

		// Start audio device
		audioState->workResult = fpl__AudioStartDevice(audioState);
		if (audioState->workResult != fplAudioResult_Success) {
			fplSignalSet(&audioState->startSignal);
			continue;
		}

		// The audio device is started, mark it as such
		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Started);
		fplSignalSet(&audioState->startSignal);

		// Enter audio device main loop
		fpl__AudioDeviceMainLoop(audioState);
	}

	// Signal to stop any audio threads, in case there are some waiting
	fplSignalSet(&audioState->stopSignal);

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.coUninitialize();
#endif
}

fpl_internal void fpl__ReleaseAudio(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState != fpl_null);

#if defined(FPL_PLATFORM_WIN32)
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	if (fpl__IsAudioDeviceInitialized(audioState)) {

		// Wait until the audio device is stopped
		if (fpl__IsAudioDeviceStarted(audioState)) {
			while (fplStopAudio() == fplAudioResult_DeviceBusy) {
				fplThreadSleep(1);
			}
		}

		// Putting the device into an uninitialized state will make the worker thread return.
		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Uninitialized);

		// Wake up the worker thread and wait for it to properly terminate.
		fplSignalSet(&audioState->wakeupSignal);
		fplThreadWaitForOne(audioState->workerThread, UINT32_MAX);
		fplThreadDestroy(audioState->workerThread);

		// Release signals and thread
		fplSignalDestroy(&audioState->stopSignal);
		fplSignalDestroy(&audioState->startSignal);
		fplSignalDestroy(&audioState->wakeupSignal);
		fplMutexDestroy(&audioState->lock);

		// Release audio device
		fpl__AudioReleaseDevice(audioState);

		// Clear audio state
		FPL_CLEAR_STRUCT(audioState);
	}

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.coUninitialize();
#endif
}

fpl_internal fplAudioResult fpl__InitAudio(const fplAudioSettings *audioSettings, fpl__AudioState *audioState) {
	FPL_ASSERT(audioState != fpl_null);

#if defined(FPL_PLATFORM_WIN32)
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	if (audioState->activeDriver != fplAudioDriverType_None) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	if (audioSettings->deviceFormat.channels == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if (audioSettings->deviceFormat.sampleRate == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if (audioSettings->bufferSizeInMilliSeconds == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	audioState->common.clientReadCallback = audioSettings->clientReadCallback;
	audioState->common.clientUserData = audioSettings->userData;

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.coInitializeEx(fpl_null, 0);
#endif

	// Create mutex and signals
	audioState->lock = fplMutexCreate();
	if (!audioState->lock.isValid) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	audioState->wakeupSignal = fplSignalCreate();
	if (!audioState->wakeupSignal.isValid) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	audioState->startSignal = fplSignalCreate();
	if (!audioState->startSignal.isValid) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	audioState->stopSignal = fplSignalCreate();
	if (!audioState->stopSignal.isValid) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	// Prope drivers
	fplAudioDriverType propeDrivers[16];
	uint32_t driverCount = 0;
	if (audioSettings->driver == fplAudioDriverType_Auto) {
		// @NOTE(final): Add all audio drivers here, regardless of the platform.
		propeDrivers[driverCount++] = fplAudioDriverType_DirectSound;
	} else {
		// @NOTE(final): Forced audio driver
		propeDrivers[driverCount++] = audioSettings->driver;
	}
	fplAudioResult initResult = fplAudioResult_Failed;
	for (uint32_t driverIndex = 0; driverIndex < driverCount; ++driverIndex) {
		fplAudioDriverType propeDriver = propeDrivers[driverIndex];

		initResult = fplAudioResult_Failed;
		switch (propeDriver) {

#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioDriverType_DirectSound:
			{
				initResult = fpl__InitDirectSound(audioSettings, &audioState->common, &audioState->dsound);
				if (initResult != fplAudioResult_Success) {
					fpl__ReleaseDirectSound(&audioState->common, &audioState->dsound);
				}
			} break;
#		endif

			default:
				break;
		}
		if (initResult == fplAudioResult_Success) {
			audioState->activeDriver = propeDriver;
			audioState->isAsyncDriver = fpl__IsAudioDriverAsync(propeDriver);
			break;
		}
	}

	if (initResult != fplAudioResult_Success) {
		fpl__ReleaseAudio(audioState);
		return initResult;
	}

	if (!audioState->isAsyncDriver) {
		// Create and start worker thread
		audioState->workerThread = fplThreadCreate(fpl__AudioWorkerThread, audioState);
		if (audioState->workerThread == fpl_null) {
			fpl__ReleaseAudio(audioState);
			return fplAudioResult_Failed;
		}
		// Wait for the worker thread to put the device into the stopped state.
		fplSignalWaitForOne(&audioState->lock, &audioState->stopSignal, UINT32_MAX);
	} else {
		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Stopped);
	}

	FPL_ASSERT(fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Stopped);

	return(fplAudioResult_Success);
}
#endif // FPL_ENABLE_AUDIO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_VIDEO_L1 (Video System, Private Implementation)
//
// The audio system is based on a stripped down version of "mini_al.h" by David Reid.
// See: https://github.com/dr-soft/mini_al
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_ENABLE_VIDEO)

#if defined(FPL_PLATFORM_WIN32)
typedef struct fpl__Win32VideoState {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
	fpl__Win32VideoOpenGLState opengl;
#	endif
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	fpl__Win32VideoSoftwareState software;
#endif
} fpl__Win32VideoState;
#elif defined(FPL_SUBPLATFORM_X11)
typedef struct fpl__X11VideoState {
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	fpl__X11VideoOpenGLState opengl;
#endif
} fpl__X11VideoState;
#endif // FPL_PLATFORM

typedef struct fpl__VideoState {
	fplVideoDriverType activeDriver;
#if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	fplVideoBackBuffer softwareBackbuffer;
#endif
	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32VideoState win32;
#	elif defined(FPL_SUBPLATFORM_X11)
		fpl__X11VideoState x11;
#	endif // FPL_PLATFORM
	};
} fpl__VideoState;

fpl_internal_inline fpl__VideoState *fpl__GetVideoState(fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__VideoState *result = fpl_null;
	if (appState->video.mem != fpl_null) {
		result = (fpl__VideoState *)appState->video.mem;
	}
	return(result);
}

fpl_internal void fpl__ShutdownVideo(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	FPL_LOG_BLOCK;
	FPL_ASSERT(appState != fpl_null);
	if (videoState != fpl_null) {
		switch (videoState->activeDriver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_PLATFORM_WIN32)
				fpl__Win32ReleaseVideoOpenGL(&videoState->win32.opengl);
#			elif defined(FPL_SUBPLATFORM_X11)
				fpl__X11ReleaseVideoOpenGL(&appState->window.x11, &videoState->x11.opengl);
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_OPENGL

#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case fplVideoDriverType_Software:
			{
#			if defined(FPL_PLATFORM_WIN32)
				fpl__Win32ReleaseVideoSoftware(&videoState->win32.software);
#			elif defined(FPL_SUBPLATFORM_X11)
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_SOFTWARE

			default:
			{
			} break;
		}

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
		if (backbuffer->pixels != fpl_null) {
			fplMemoryAlignedFree(backbuffer->pixels);
		}
		FPL_CLEAR_STRUCT(backbuffer);
#	endif
	}
}

fpl_internal void fpl__ReleaseVideoState(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	FPL_LOG_BLOCK;

	switch (videoState->activeDriver) {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		case fplVideoDriverType_OpenGL:
		{
#		if defined(FPL_PLATFORM_WIN32)
			fpl__Win32UnloadVideoOpenGLApi(&videoState->win32.opengl.api);
#		elif defined(FPL_SUBPLATFORM_X11)
			videoState->x11.opengl.fbConfig = fpl_null;
			fpl__X11UnloadVideoOpenGLApi(&videoState->x11.opengl.api);
#		endif
		}; break;
#	endif

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		case fplVideoDriverType_Software:
		{
			// Nothing todo
		}; break;
#	endif

		default:
			break;
	}
	FPL_CLEAR_STRUCT(videoState);
}

fpl_internal bool fpl__LoadVideoState(const fplVideoDriverType driver, fpl__VideoState *videoState) {
	FPL_LOG_BLOCK;

	bool result = true;

	switch (driver) {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		case fplVideoDriverType_OpenGL:
		{
#		if defined(FPL_PLATFORM_WIN32)
			result = fpl__Win32LoadVideoOpenGLApi(&videoState->win32.opengl.api);
#		elif defined(FPL_SUBPLATFORM_X11)
			result = fpl__X11LoadVideoOpenGLApi(&videoState->x11.opengl.api);
#		endif
		}; break;
#	endif

		default:
			break;
	}
	return(result);
}

fpl_internal bool fpl__InitVideo(const fplVideoDriverType driver, const fplVideoSettings *videoSettings, const uint32_t windowWidth, const uint32_t windowHeight, fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	FPL_LOG_BLOCK;

	// @NOTE(final): Video drivers are platform independent, so we cannot have to same system as audio.
	FPL_ASSERT(appState != fpl_null);
	FPL_ASSERT(videoState != fpl_null);

	videoState->activeDriver = driver;

	// Allocate backbuffer context if needed
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	if (driver == fplVideoDriverType_Software) {
		fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
		backbuffer->width = windowWidth;
		backbuffer->height = windowHeight;
		backbuffer->pixelStride = sizeof(uint32_t);
		backbuffer->lineWidth = backbuffer->width * backbuffer->pixelStride;
		size_t size = backbuffer->lineWidth * backbuffer->height;
		backbuffer->pixels = (uint32_t *)fplMemoryAlignedAllocate(size, 4);
		if (backbuffer->pixels == fpl_null) {
			fpl__PushError("Failed allocating video software backbuffer of size %xu bytes", size);
			fpl__ShutdownVideo(appState, videoState);
			return false;
		}

		// Clear to black by default
		// @NOTE(final): Bitmap is top-down, 0xAABBGGRR
		// @TODO(final): Backbuffer endianess???
		uint32_t *p = backbuffer->pixels;
		for (uint32_t y = 0; y < backbuffer->height; ++y) {
			uint32_t color = 0xFF000000;
			for (uint32_t x = 0; x < backbuffer->width; ++x) {
				*p++ = color;
			}
		}
	}
#		endif // FPL_ENABLE_VIDEO_SOFTWARE

	bool videoInitResult = false;
	switch (driver) {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		case fplVideoDriverType_OpenGL:
		{
#		if defined(FPL_PLATFORM_WIN32)
			videoInitResult = fpl__Win32InitVideoOpenGL(&appState->win32, &appState->window.win32, videoSettings, &videoState->win32.opengl);
#		elif defined(FPL_SUBPLATFORM_X11)
			videoInitResult = fpl__X11InitVideoOpenGL(&appState->x11, &appState->window.x11, videoSettings, &videoState->x11.opengl);
#		endif
		} break;
#	endif // FPL_ENABLE_VIDEO_OPENGL

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		case fplVideoDriverType_Software:
		{
#		if defined(FPL_PLATFORM_WIN32)
			videoInitResult = fpl__Win32InitVideoSoftware(&videoState->softwareBackbuffer, &videoState->win32.software);
#		elif defined(FPL_SUBPLATFORM_X11)
#		endif
		} break;
#	endif // FPL_ENABLE_VIDEO_SOFTWARE

		default:
		{
			fpl__PushError("Unsupported video driver '%s' for this platform", fplGetVideoDriverString(videoSettings->driver));
		} break;
	}
	if (!videoInitResult) {
		FPL_ASSERT(fplGetPlatformErrorCount() > 0);
		fpl__ShutdownVideo(appState, videoState);
		return false;
	}

	return true;
}
#endif // FPL_ENABLE_VIDEO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_WINDOW (Window System, Private Implementation)
//
// - Init window
// - Release window
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_ENABLE_WINDOW)
fpl_internal FPL__FUNC_PRE_SETUP_WINDOW(fpl__PreSetupWindowDefault) {
	FPL_ASSERT(appState != fpl_null);
	bool result = false;

#	if defined(FPL_ENABLE_VIDEO)
	if (initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		switch (initSettings->video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_SUBPLATFORM_X11)
				if (fpl__X11InitFrameBufferConfigVideoOpenGL(&appState->x11.api, &appState->window.x11, &videoState->x11.opengl)) {
					result = fpl__X11SetPreWindowSetupForOpenGL(&appState->x11.api, &appState->window.x11, &videoState->x11.opengl, &outResult.x11);
				}
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_OPENGL

			default:
			{
			} break;
		}
	}
#	endif // FPL_ENABLE_VIDEO

	return(result);
}

fpl_internal FPL__FUNC_POST_SETUP_WINDOW(fpl__PostSetupWindowDefault) {
	FPL_ASSERT(appState != fpl_null);

#if defined(FPL_ENABLE_VIDEO)
	if (initFlags & fplInitFlags_Video) {
		switch (initSettings->video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_PLATFORM_WIN32)
				if (!fpl__Win32PostSetupWindowForOpenGL(&appState->win32, &appState->window.win32, &initSettings->video)) {
					return false;
				}
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_OPENGL

			default:
			{
			} break;
		}
	}
#endif // FPL_ENABLE_VIDEO

	return false;
}

fpl_internal bool fpl__InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *appState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	FPL_LOG_BLOCK;

	bool result = false;
	if (appState != fpl_null) {
#	if defined(FPL_PLATFORM_WIN32)
		result = fpl__Win32InitWindow(initSettings, currentWindowSettings, appState, &appState->win32, &appState->window.win32, setupCallbacks);
#	elif defined(FPL_SUBPLATFORM_X11)
		result = fpl__X11InitWindow(initSettings, currentWindowSettings, appState, &appState->x11, &appState->window.x11, setupCallbacks);
#	endif
	}
	return (result);
}

fpl_internal void fpl__ReleaseWindow(const fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_LOG_BLOCK;

	if (appState != fpl_null) {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32ReleaseWindow(&initState->win32, &appState->win32, &appState->window.win32);
#	elif defined(FPL_SUBPLATFORM_X11)
		fpl__X11ReleaseWindow(&appState->x11, &appState->window.x11);
#	endif
	}
}
#endif // FPL_ENABLE_WINDOW

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_AUDIO_L2 (Audio System, Public Implementation)
//
// - Stop audio
// - Play audio
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_ENABLE_AUDIO)
fpl_common_api fplAudioResult fplStopAudio() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return fplAudioResult_Failed;
	}

	if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Uninitialized) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock, UINT32_MAX);
	{
		// Check if the device is already stopped
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Stopping) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}

		// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(audioState) != fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Stopping);

		if (audioState->isAsyncDriver) {
			// Asynchronous drivers (Has their own thread)
			fpl__AudioStopDevice(audioState);
		} else {
			// Synchronous drivers

			// The audio worker thread is most likely in a wait state, so let it stop properly.
			fpl__AudioDeviceStopMainLoop(audioState);

			// We need to wait for the worker thread to become available for work before returning.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the stopped state.
			fplSignalWaitForOne(&audioState->lock, &audioState->stopSignal, UINT32_MAX);
			result = fplAudioResult_Success;
		}
	}
	fplMutexUnlock(&audioState->lock);

	return result;
}

fpl_common_api fplAudioResult fplPlayAudio() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return fplAudioResult_Failed;
	}

	if (!fpl__IsAudioDeviceInitialized(audioState)) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock, UINT32_MAX);
	{
		// Be a bit more descriptive if the device is already started or is already in the process of starting.
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Starting) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}

		// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(audioState) != fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Starting);

		if (audioState->isAsyncDriver) {
			// Asynchronous drivers (Has their own thread)
			fpl__AudioStartDevice(audioState);
			fpl__AudioSetDeviceState(audioState, fpl__AudioDeviceState_Started);
		} else {
			// Synchronous drivers
			fplSignalSet(&audioState->wakeupSignal);

			// Wait for the worker thread to finish starting the device.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the started state.
			fplSignalWaitForOne(&audioState->lock, &audioState->startSignal, UINT32_MAX);
			result = audioState->workResult;
		}
	}
	fplMutexUnlock(&audioState->lock);

	return result;
}

fpl_common_api fplAudioDeviceFormat fplGetAudioHardwareFormat() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		fplAudioDeviceFormat result = FPL_ZERO_INIT;
		return(result);
	}
	return audioState->common.internalFormat;
}

fpl_common_api void fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return;
	}
	if (audioState->activeDriver > fplAudioDriverType_Auto) {
		if (fpl__AudioGetDeviceState(audioState) == fpl__AudioDeviceState_Stopped) {
			audioState->common.clientReadCallback = newCallback;
			audioState->common.clientUserData = userData;
		}
	}
}

fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount) {
	if (devices == fpl_null) {
		return 0;
	}
	if (maxDeviceCount == 0) {
		return 0;
	}

	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return 0;
	}

	uint32_t result = 0;
	if (audioState->activeDriver > fplAudioDriverType_Auto) {
		switch (audioState->activeDriver) {
#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioDriverType_DirectSound:
			{
				result = fpl__GetDevicesDirectSound(&audioState->dsound, devices, maxDeviceCount);
			} break;
#		endif
		}
	}
	return(result);
}
#endif // FPL_ENABLE_AUDIO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_VIDEO_L2 (Video System, Public Implementation)
//
// - Video Backbuffer Access
// - Frame flipping
// - Utiltity functions
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_ENABLE_VIDEO)
fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;

	fplVideoBackBuffer *result = fpl_null;
	if (appState->video.mem != fpl_null) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		if (appState->currentSettings.video.driver == fplVideoDriverType_Software) {
			result = &videoState->softwareBackbuffer;
		}
#	endif
	}

	return(result);
}

fpl_common_api fplVideoDriverType fplGetVideoDriver() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	fplVideoDriverType result = appState->currentSettings.video.driver;
	return(result);
}

fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__PlatformVideoState *video = &appState->video;
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	bool result = false;
	if (videoState != fpl_null) {
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		if (appState->currentSettings.video.driver == fplVideoDriverType_Software) {
			fpl__ShutdownVideo(appState, videoState);
			result = fpl__InitVideo(fplVideoDriverType_Software, &appState->currentSettings.video, width, height, appState, videoState);
		}
#	endif
	}
	return (result);
}

fpl_common_api void fplVideoFlip() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__VideoState *videoState = fpl__GetVideoState(appState);

	if (videoState != fpl_null) {
#	if defined(FPL_PLATFORM_WIN32)
		const fpl__Win32AppState *win32AppState = &appState->win32;
		const fpl__Win32WindowState *win32WindowState = &appState->window.win32;
		const fpl__Win32Api *wapi = &win32AppState->winApi;
		switch (appState->currentSettings.video.driver) {
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case fplVideoDriverType_Software:
			{
				const fpl__Win32VideoSoftwareState *software = &videoState->win32.software;
				const fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
				fplWindowSize area = fplGetWindowArea();
				int32_t targetX = 0;
				int32_t targetY = 0;
				int32_t targetWidth = area.width;
				int32_t targetHeight = area.height;
				int32_t sourceWidth = backbuffer->width;
				int32_t sourceHeight = backbuffer->height;
				if (backbuffer->useOutputRect) {
					targetX = backbuffer->outputRect.x;
					targetY = backbuffer->outputRect.y;
					targetWidth = backbuffer->outputRect.width;
					targetHeight = backbuffer->outputRect.height;
					wapi->gdi.stretchDIBits(win32WindowState->deviceContext, 0, 0, area.width, area.height, 0, 0, 0, 0, fpl_null, fpl_null, DIB_RGB_COLORS, BLACKNESS);
				}
				wapi->gdi.stretchDIBits(win32WindowState->deviceContext, targetX, targetY, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, backbuffer->pixels, &software->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
			} break;
#		endif

#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
				wapi->gdi.swapBuffers(win32WindowState->deviceContext);
			} break;
#		endif

			default:
				break;
		}
#	endif // FPL_PLATFORM_WIN32
	}
}
#endif // FPL_ENABLE_VIDEO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_INIT
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if !defined(FPL_SYSTEM_INIT_DEFINED)
#define FPL_SYSTEM_INIT_DEFINED

fpl_internal void fpl__ReleasePlatformStates(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_LOG_BLOCK;

	FPL_ASSERT(initState != fpl_null);

	// Release audio
#	if defined(FPL_ENABLE_AUDIO)
	{
		FPL_LOG("Core", "Release Audio");
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		if (audioState != fpl_null) {
			// @TODO(final): Rename to ShutdownAudio
			fpl__ReleaseAudio(audioState);
		}
	}
#	endif

	// Shutdown video (Release context only)
#	if defined(FPL_ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState != fpl_null) {
			FPL_LOG("Core", "Shutdown Video for Driver '%s'", fplGetVideoDriverString(videoState->activeDriver));
			fpl__ShutdownVideo(appState, videoState);
		}
	}
#	endif

	// Release window
#	if defined(FPL_ENABLE_WINDOW)
	{
		FPL_LOG("Core", "Release Window");
		fpl__ReleaseWindow(initState, appState);
	}
#	endif

	// Release video state
#	if defined(FPL_ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState != fpl_null) {
			FPL_LOG("Core", "Release Video for Driver '%s'", fplGetVideoDriverString(videoState->activeDriver));
			fpl__ReleaseVideoState(appState, videoState);
		}
	}
#	endif

	// @TODO(final): Release audio state here

	if (appState != fpl_null) {
		// Release actual platform (There can only be one platform!)
		{
#		if defined(FPL_PLATFORM_WIN32)
			FPL_LOG("Core", "Release Win32 Platform");
			fpl__Win32ReleasePlatform(initState, appState);
#		elif defined(FPL_PLATFORM_LINUX)
			FPL_LOG("Core", "Release Linux Platform");
			fpl__LinuxReleasePlatform(initState, appState);
#		endif
		}

		// Release sub platforms
		{
#		if defined(FPL_SUBPLATFORM_X11)
			FPL_LOG("Core", "Release X11 Subplatform");
			fpl__X11ReleaseSubplatform(appState->x11);
#		endif
#		if defined(FPL_SUBPLATFORM_POSIX)
			FPL_LOG("Core", "Release POSIX Subplatform");
			fpl__PosixReleaseSubplatform(appState->posix);
#		endif
		}

		// Release platform applicatiom state memory
		FPL_LOG("Core", "Release allocated Platform App State Memory");
		fplMemoryFree(appState);
		fpl__global__AppState = fpl_null;
	}
	initState->isInitialized = false;
}

fpl_common_api void fplPlatformRelease() {
	FPL_LOG_BLOCK;

	// Exit out if platform is not initialized
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	if (!initState->isInitialized) {
		fpl__PushError("Platform is not initialized");
		return;
	}
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__ReleasePlatformStates(initState, appState);
	FPL_LOG("Core", "Platform released");
}

fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {
	FPL_LOG_BLOCK;

	// Exit out if platform is already initialized
	if (fpl__global__InitState.isInitialized) {
		fpl__PushError("Platform is already initialized");
		return false;
	}

	// Allocate platform app state memory (By boundary of 16-bytes)
	size_t platformAppStateSize = FPL_ALIGNED_SIZE(sizeof(fpl__PlatformAppState), 16);

	// Include video/audio state memory in app state memory as well
#if defined(FPL_ENABLE_VIDEO)
	size_t videoMemoryOffset = 0;
	if (initFlags & fplInitFlags_Video) {
		platformAppStateSize += FPL__SIZE_PADDING;
		videoMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__VideoState);
	}
#endif

#if defined(FPL_ENABLE_AUDIO)
	size_t audioMemoryOffset = 0;
	if (initFlags & fplInitFlags_Audio) {
		platformAppStateSize += FPL__SIZE_PADDING;
		audioMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__AudioState);
	}
#endif

	FPL_LOG("Core", "Allocate Platform App State Memory of size '%zu':", platformAppStateSize);
	FPL_ASSERT(fpl__global__AppState == fpl_null);
	void *platformAppStateMemory = fplMemoryAllocate(platformAppStateSize);
	if (platformAppStateMemory == fpl_null) {
		FPL_LOG("Core", "Failed Allocating Platform App State Memory of size '%zu'", platformAppStateSize);
		fpl__PushError("Failed allocating app state memory of size '%zu'", platformAppStateSize);
		return false;
	}

	fpl__PlatformAppState *appState = fpl__global__AppState = (fpl__PlatformAppState *)platformAppStateMemory;
	appState->initFlags = initFlags;
	if (initSettings != fpl_null) {
		appState->initSettings = *initSettings;
	} else {
		appState->initSettings = fplDefaultSettings();
	}
	appState->currentSettings = appState->initSettings;

	fpl__PlatformInitState *initState = &fpl__global__InitState;
	FPL_CLEAR_STRUCT(initState);
	FPL_LOG("Core", "Successfully allocated Platform App State Memory of size '%zu'", platformAppStateSize);

// Window is required for video always
#	if defined(FPL_ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		appState->initFlags |= fplInitFlags_Window;
	}
#	endif

	// Initialize sub-platforms
#	if defined(FPL_SUBPLATFORM_POSIX)
	{
		FPL_LOG("Core", "Initialize POSIX Subplatform:");
		if (!fpl__PosixInitSubplatform(initFlags, initSettings, initState.posix, appState->posix)) {
			FPL_LOG("Core", "Failed initializing POSIX Subplatform!");
			PushError("Failed initializing POSIX Subplatform");
			fpl__ReleasePlatformStates(initState, appState);
			return false;
		}
		FPL_LOG("Core", "Successfully initialized POSIX Subplatform");
	}
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
	{
		FPL_LOG("Core", "Initialize X11 Subplatform:");
		if (!fpl__X11InitSubplatform(appState->x11)) {
			FPL_LOG("Core", "Failed initializing X11 Subplatform!");
			PushError("Failed initializing X11 Subplatform");
			fpl__ReleasePlatformStates(initState, appState);
			return false;
		}
		FPL_LOG("Core", "Successfully initialized X11 Subplatform");
	}
#	endif // FPL_SUBPLATFORM_X11

		// Initialize the actual platform (There can only be one at a time!)
	bool isInitialized = false;
	FPL_LOG("Core", "Initialize %s Platform:", FPL_PLATFORM_NAME);
#	if defined(FPL_PLATFORM_WIN32)
	isInitialized = fpl__Win32InitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#   elif defined(FPL_PLATFORM_LINUX)
	isInitialized = fpl__LinuxInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	endif

	if (!isInitialized) {
		FPL_LOG("Core", "Failed initializing %s Platform!", FPL_PLATFORM_NAME);
		fpl__ReleasePlatformStates(initState, appState);
		return false;
	}
	FPL_LOG("Core", "Successfully initialized %s Platform", FPL_PLATFORM_NAME);

// Init video state
#	if defined(FPL_ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		FPL_LOG("Core", "Init video state:");
		appState->video.mem = (uint8_t *)platformAppStateMemory + videoMemoryOffset;
		appState->video.memSize = sizeof(fpl__VideoState);
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		FPL_ASSERT(videoState != fpl_null);

		fplVideoDriverType videoDriver = appState->initSettings.video.driver;
		const char *videoDriverString = fplGetVideoDriverString(videoDriver);
		FPL_LOG("Core", "Load Video API for Driver '%s':", videoDriverString);
		{
			if (!fpl__LoadVideoState(videoDriver, videoState)) {
				FPL_LOG("Core", "Failed loading Video API for Driver '%s'!", videoDriverString);
				fpl__ReleasePlatformStates(initState, appState);
				return false;
			}
		}
		FPL_LOG("Core", "Successfully loaded Video API for Driver '%s'", videoDriverString);
	}
#	endif // FPL_ENABLE_VIDEO

	// Init Window & event queue
#	if defined(FPL_ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_Window) {
		FPL_LOG("Core", "Init Window:");
		fpl__SetupWindowCallbacks winCallbacks = FPL_ZERO_INIT;
		winCallbacks.postSetup = fpl__PostSetupWindowDefault;
		winCallbacks.preSetup = fpl__PreSetupWindowDefault;
		if (!fpl__InitWindow(&appState->initSettings, &appState->currentSettings.window, appState, &winCallbacks)) {
			FPL_LOG("Core", "Failed initializing Window!");
			fpl__PushError("Failed initialization window");
			fpl__ReleasePlatformStates(initState, appState);
			return false;
		}
		FPL_LOG("Core", "Successfully initialized Window");
	}
#	endif // FPL_ENABLE_WINDOW

	// Init Video
#	if defined(FPL_ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		FPL_ASSERT(videoState != fpl_null);
		uint32_t windowWidth, windowHeight;
		if (appState->currentSettings.window.isFullscreen) {
			windowWidth = appState->currentSettings.window.fullscreenWidth;
			windowHeight = appState->currentSettings.window.fullscreenHeight;
		} else {
			windowWidth = appState->currentSettings.window.windowWidth;
			windowHeight = appState->currentSettings.window.windowWidth;
		}
		const char *videoDriverName = fplGetVideoDriverString(appState->initSettings.video.driver);
		FPL_LOG("Core", "Init Video with Driver '%s':", videoDriverName);
		if (!fpl__InitVideo(appState->initSettings.video.driver, &appState->initSettings.video, windowWidth, windowHeight, appState, videoState)) {
			FPL_LOG("Core", "Failed initializing Video Driver '%s'!", videoDriverName);
			fpl__PushError("Failed initialization video with settings (Driver=%s, Width=%d, Height=%d)", videoDriverName, windowWidth, windowHeight);
			fpl__ReleasePlatformStates(initState, appState);
			return false;
		}
		FPL_LOG("Core", "Successfully initialized Video Driver '%s'", videoDriverName);
	}
#	endif // FPL_ENABLE_VIDEO

	// Init Audio
#	if defined(FPL_ENABLE_AUDIO)
	if (appState->initFlags & fplInitFlags_Audio) {
		appState->audio.mem = (uint8_t *)platformAppStateMemory + audioMemoryOffset;
		appState->audio.memSize = sizeof(fpl__AudioState);
		const char *audioDriverName = fplGetAudioDriverString(appState->initSettings.audio.driver);
		FPL_LOG("Core", "Init Audio with Driver '%s':", audioDriverName);
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		FPL_ASSERT(audioState != fpl_null);
		if (fpl__InitAudio(&initSettings->audio, audioState) != fplAudioResult_Success) {
			FPL_LOG("Core", "Failed initializing Audio Driver '%s'!", audioDriverName);
			fpl__PushError("Failed initialization audio with settings (Driver=%s, Format=%s, SampleRate=%d, Channels=%d, BufferSize=%d)", audioDriverName, fplGetAudioFormatString(initSettings->audio.deviceFormat.type), initSettings->audio.deviceFormat.sampleRate, initSettings->audio.deviceFormat.channels);
			fpl__ReleasePlatformStates(initState, appState);
			return false;
		}
		FPL_LOG("Core", "Successfully initialized Audio Driver '%s'", audioDriverName);
	}
#	endif // FPL_ENABLE_AUDIO

	initState->isInitialized = true;
	return true;
}

#endif // FPL_SYSTEM_INIT_DEFINED

#endif // FPL_IMPLEMENTATION && !FPL_IMPLEMENTED
