/*
final_platform_layer.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

Final Platform Layer is a Single-Header-File cross-platform C development library designed to abstract the underlying platform to a simple and easy to use api - providing low level access to (Window, Video, Audio, Input, File/Path IO, Threads, Memory, Hardware, etc.).

The main focus is game/media/simulation development, so the default settings will create a window, setup a OpenGL rendering context and initialize audio playback on any platform.

It is written in C99 for simplicity and best portability, but is C++ compatible as well.

FPL supports the platforms Windows/Linux/Unix for the architectures x86/x64.

The only dependencies are built-in operating system libraries and a C99 complaint compiler.

It is licensed under the MIT-License. This license allows you to use FPL freely in any software.

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into any C/C++ projects you want and include it in any place you want
- In your main translation unit provide the typical main() entry point
- Define FPL_IMPLEMENTATION in at least one translation unit before including this header file
- Init the platform using fplPlatformInit()
- Use the features you want
- Release the platform when you are done using fplPlatformRelease()

-------------------------------------------------------------------------------
	Usage: Hello world console application
-------------------------------------------------------------------------------

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

int main(int argc, char **args){
	if (fplPlatformInit(fplInitFlags_None, fpl_null)) {
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
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	fplVideoSettings videoSettings = settings.video;

	videoSettings.driver = fplVideoDriverType_OpenGL;

	// Legacy OpenGL
	videoSettings.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;

	// or

	// Modern OpenGL
	videoSettings.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	videoSettings.opengl.majorVersion = 3;
	videoSettings.opengl.minorVersion = 3;

	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		// Event/Main loop
		while (fplWindowUpdate()) {
			// Handle actual window events
			fplEvent ev;
			while (fplPollEvent(ev)) {
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
	\version v0.7.6.0 beta
	\author Torsten Spaete
	\brief Final Platform Layer (FPL) - A C99 Single-Header-File Platform Abstract Library
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

	## v0.7.7.0 beta:
	- New: Added fplMutexTryLock
	- New: Added fplMakeDefaultSettings
	- New: Added fplStringAppend / fplStringAppendLen
	- Changed: Changed fplGetClipboardAnsiText to return bool instead of char *
	- Changed: Changed fplGetClipboardWideText to return bool instead of wchar_t *

	## v0.7.6.0 beta:
	- Changed: Renamed fplGetRunningArchitectureType to fplGetRunningArchitecture
	- Changed: Renamed fplThreadDestroy() to fplThreadTerminate() + signature changed (Returns bool)
	- Changed: fplSignalInit() + new parameter "initialValue"
	- Changed: All functions which uses timeout uses fplTimeoutValue instead of uint32_t
	- Changed: Removed timeout parameter from fplMutexLock()
	- New: Added struct fplConditionVariable
	- New: Added enum fplSignalValue
	- New: Added fplConditionInit()
	- New: Added fplConditionDestroy()
	- New: Added fplConditionWait()
	- New: Added fplConditionSignal()
	- New: Added fplConditionBroadcast()
	- New: Added typedef fplTimeoutValue
	- New: Added constant FPL_TIMEOUT_INFINITE

	- Changed: [Win32] Thread resources are automatically cleaned up when a thread is done running
	- Changed: [POSIX] Thread resources are automatically cleaned up when a thread is done running
	- Changed: [Win32] fplSignalInit updated to support isSetInitially
	- Changed: [Linux] fplSignalInit updated to support isSetInitially
	- Fixed: [GLX] Fallback to glXCreateContext was not working properly
	- New: [Linux] Implemented fplGetCurrentUsername
	- New: [Win32] Implemented all fplCondition*
	- New: [POSIX] Implemented all fplCondition*

	## v0.7.5.0 beta:
	- Changed: Updated documentations
	- Changed: Small refactoring of the internal audio system
	- Changed: Renamed fplMutexCreate to fplMutexInit + signature change (Returns bool, Mutex pointer argument)
	- Changed: Renamed fplSignalCreate to fplSignalInit + signature change (Returns bool, Signal pointer argument)
	- Changed: Removed mutex parameter from SignalWaitFor*

	- Changed: [GLX] Use glXCreateContext with visual info caching for GLX < 1.3 and glXCreateNewContext for GLX >= 1.3
	- Changed: [POSIX] All FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK are wrapped around a do-while loop so it can "break" properly.
	- Removed: [POSIX] All signal functions removed
	- New: [ALSA] Added rudimentary ALSA playback support
	- New: [Linux] Implemented signal functions using eventfd

	## v0.7.4.0 beta:
	- Fixed: [Win32] Removed x64 detection for fplAtomicStoreS64 and fplAtomicExchangeS64 and use _InterlockedExchange*64 directly
	- Changed: [GLX] Implemented modern opengl context creation

	## v0.7.3.0 beta:
	- Changed: fplConsoleWaitForCharInput returns char instead of const char
	- Changed: Added isDecorated field to fplWindowSettings
	- Changed: Added isFloating field to fplWindowSettings
	- Changed: Renamed fplSetWindowTitle() -> fplSetWindowAnsiTitle()
	- Changed: Copy Ansi/Wide String pushes error for buffer range error
	- Fixed: Fixed api name mismatch CloseFile() -> fplCloseFile()
	- Fixed: Corrected wrong doxygen defines
	- Fixed: Corrected most clang compile warnings
	- New: Added fplIsWindowDecorated() / fplSetWindowDecorated()
	- New: Added fplIsWindowFloating() / fplSetWindowFloating()
	- New: Added fplSetWindowWideTitle()
	- New: Added fplGetTimeInMillisecondsHP()
	- New: Added fplGetTimeInMillisecondsLP()
	- New: Added fplGetTimeInSecondsLP()
	- New: Added fplGetTimeInSecondsHP()
	- New: Added FPL_NO_ENTRYPOINT

	- Changed: [Win32] fplAtomicExchangeS64() / fplAtomicAddS64() / fplAtomicStoreS64() uses _Interlocked* operatings directly for x86
	- Fixed: [Win32] Corrected wrong case-sensitivity in includes
	- Fixed: [Win32] Fixed Cursor visibility was not properly changeable
	- Fixed: [Win32] Function prototype macros was not properly named
	- New: [Win32] Implemented fplIsWindowDecorated() / fplSetWindowDecorated()
	- New: [Win32] Implemented fplIsWindowFloating() / fplSetWindowFloating()
	- New: [Win32] Implemented fplSetWindowWideTitle()
	- New: [Win32] Implemented fplGetTimeInMillisecondsLP()
	- New: [Win32] Implemented fplGetTimeInMillisecondsHP()
	- New: [Win32] Implemented fplGetTimeInSecondsLP()
	- New: [Win32] Implemented fplGetTimeInSecondsHP()
	- New: [POSIX] Implemented fplGetTimeInMillisecondsLP()
	- New: [POSIX] Implemented fplGetTimeInMillisecondsHP()
	- New: [POSIX] Implemented fplGetTimeInSecondsLP()
	- New: [POSIX] Implemented fplGetTimeInSecondsHP()


	## v0.7.2.0 beta:
	- Changed: Signature of fplGetRunningMemoryInfos() changed
	- Changed: Renamed fplGetSystemMemoryInfos to fplGetRunningMemoryInfos
	- Changed: Added "p" prefix to linux and unix app state
	- New: Added enum fplArchType
	- New: Added fplGetRunningArchitectureType()
	- New: Added fplGetArchTypeString()
	- New: Added enum fplPlatformType
	- New: Added fplGetPlatformType()
	- New: Added fplGetPlatformTypeString()
	- New: Added struct fplVersionInfo
	- New: Added struct fplOSInfos
	- New: Added fplGetOperatingSystemInfos()
	- New: Added fplGetCurrentUsername()

	- Changed: [Docs] Updated all documentations to match the current state
	- Fixed: [Docs] Missing brief for fpl_extern

	- Changed: [MSVC] Removed the compiler specific No-CRT block such as _fltused, etc. -> The caller is responsible for this!
	- New: [Win32] Implemented fplGetCurrentUsername()
	- New: [Win32] Implemented fplGetOperatingSystemInfos()
	- New: [Win32] Implemented fplGetRunningArchitectureType()

	## v0.7.1.0 beta:
	- Changed: fplConsoleFormatOut/fplConsoleFormatError is now common_api instead of platform_api
	- Changed: FPL uses a keyMap for mapping OS key codes to fplKey for every platform
	- Changed: [Win32] Console does not cache the output/input/error handles
	- Changed: [Win32] fplConsole* uses ReadFile/WriteFile instead of ReadConsole/WriteConsole
	- Changed: BSD Platform is detected as a Unix Platform, but has its own subplatform as well
	- Fixed: [Win32] Console was always allocated
	- Fixed: No CRT stub defintions such as memset and RTC was added for all compilers which is wrong
	- Fixed: Several bugfixes
	- New: Added stubs for Unix Platform

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
	- Changed: PlatformInit uses InitResultType
	- Changed: fpl*DefaultSettings is now fplSet*DefaultSettings and does not return a struct anymore
	- Fixed: Get rid of const char* to char* warnings for Get*DriverString() functions
	- Fixed: [Win32] Icon was not default application icon
	- Fixed: ExtractFileName and ExtractFileExtension was not returning const char*
	- New: [X11][Window] Implemented X11 Subplatform
	- New: [X11][OpenGL] Implemented X11/GLX video driver
	- New: [X11][Software] Implemented Software video driver
	- New: [POSIX] Implemented all remaining posix functions
	- New: Introduced debug logging -> FPL_LOGGING
	- New: Added FPL_ALIGNMENT_OFFSET macro
	- New: Added FPL_ALIGNED_SIZE macro
	- New: Added InitResultType
	- New: CRT (C-Runtime) is not required anymore

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
	\page page_platform_status Platform Status
	\tableofcontents

	\section section_platform_status_supported_archs Supported Architectures

	- x86
	- x86_64
	- x64

	\section section_platform_status_planned_archs Planned Architectures

	- Arm32
	- Arm64

	\section section_platform_status_supported_platforms Supported Platforms

	- Windows
	- Linux (Partially, In-Progress)
	- Unix (Partially, In-Progress)

	\section section_platform_status_planned_platforms Planned Platforms

	- MacOSX
*/

/*!
	\page page_todo ToDo / Planned (Random order)
	\tableofcontents

	\section section_todo_required In progress / Todo

	- Source
		- Collapse down repeatable codes like argument checking, api from app state using macros
	- POSIX
		- Files & Path
			- File/Dir iteration
	- Linux/Unix
		- Window (X11)
			- Toggle Fullscreen
			- Toggle Resizable
			- Toggle Decorated
			- Toggle Floating
			- Show/Hide Cursor
		- Video
			- Software backbuffer (X11)
	- Threading
		- eventfd for Unix
		- Semaphores
		- TryMutexLock

	- Audio
		- Finalize Alsa driver (Device selection)

	\section section_todo_planned Planned

	- MacOSX Platform

	- Current Date/Time functions

	- App:
		- Custom icon from image (File/Memory)

	- Window:
		- Custom cursor from image (File/Memory)
		- Unicode/UTF-8 Support for character input
		- Open/Save file/folder dialog

	- Audio:
		- Support for channel mapping
		- PulseAudio driver
		- WASAPI audio driver
		- OpenAL audio driver

	- Video:
		- [Win32] Direct2D
		- [Win32] Direct3D
		- [Win32] Vulkan
		- [POSIX] Vulkan

	- Networking (UDP, TCP)
		- [Win32] WinSock
		- [POSIX] Socket

	- Documentation
		- Audio details
		- Window details
		- Memory
		- Atomics
		- Multi-Threading
		- File-IO
		- Paths
		- Strings
		- Hardware

	- DLL-Export support

	- Multimonitor-Support

	- Unicode-Support for commandline arguments (Win32)
*/

// ****************************************************************************
//
// > HEADER
//
// ****************************************************************************
#ifndef FPL_INCLUDE_H
#define FPL_INCLUDE_H

//
// C99 detection
//
// https://en.wikipedia.org/wiki/C99#Version_detection
// Visual Studio 2015+
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! Detected C99 compiler
#	define FPL_IS_C99
#elif defined(__cplusplus)
	//! Detected C++ compiler
#	define FPL_IS_CPP
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
#elif defined(__linux__) || defined(__gnu_linux__)
#	define FPL_PLATFORM_LINUX
#	define FPL_PLATFORM_NAME "Linux"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__bsdi__)
	// @NOTE(final): BSD is treated as a subplatform for now
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "BSD"
#	define FPL_SUBPLATFORM_BSD
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
// Compilers detection
// See: http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
//
#if defined(__llvm__)
	//! LLVM compiler detected
#	define FPL_COMPILER_LLVM
#elif defined(__clang__)
	//! CLANG compiler detected
#	define FPL_COMPILER_CLANG
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
// No-CRT Support is limited to Win32 for now
//
#if defined(FPL_NO_CRT)
#	if !defined(FPL_COMPILER_MSVC)
#		error "C-Runtime cannot be disabled on this compiler/platform!"
#	endif
#endif

//
// When C-Runtime is disabled we cannot use any function from the C-Standard Library <stdio.h> or <stdlib.h>
//
#if defined(FPL_NO_CRT)
#	undef FPL_SUBPLATFORM_STD_CONSOLE
#	undef FPL_SUBPLATFORM_STD_STRINGS
#	if !defined(FPL_USERFUNC_vsnprintf)
#		error "You need to provide a replacement for vsnprintf() by defining FPL_USERFUNC_vsnprintf!"
#	endif
#endif

//
// Application type detection
// - Can be disabled by FPL_NO_APPTYPE
// - Must be explicitly set for No-CRT on Win32
//
#if defined(FPL_APPTYPE_CONSOLE) && defined(FPL_APPTYPE_WINDOW)
#	error "Its now allowed to define both FPL_APPTYPE_CONSOLE and FPL_APPTYPE_WINDOW!"
#endif
#if defined(FPL_NO_CRT)
#	if !defined(FPL_APPTYPE_CONSOLE) && !defined(FPL_APPTYPE_WINDOW)
#		error "In 'No-CRT' mode you need to define either FPL_APPTYPE_CONSOLE or FPL_APPTYPE_WINDOW manually!"
#	endif
#elif !defined(FPL_NO_APPTYPE) && !(defined(FPL_APPTYPE_CONSOLE) || defined(FPL_APPTYPE_WINDOW))
#	if !defined(FPL_NO_WINDOW)
		//! Detected window application type
#		define FPL_APPTYPE_WINDOW
#	else
		//! Detected console application type
#		define FPL_APPTYPE_CONSOLE
#	endif
#endif

//
// Compiler settings
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

	//! Setup MSVC subsystem hints
#	if defined(FPL_APPTYPE_WINDOW)
#		pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#	elif defined(FPL_APPTYPE_CONSOLE)
#		pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#	endif

	// Setup MSVC linker hints
#	pragma comment(lib, "kernel32.lib")
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
#	if !defined(FPL_NO_C_ASSERT) && !defined(FPL_NO_CRT)
		//! Enable C-Runtime Assertions by default
#		define FPL_ENABLE_C_ASSERT
#	endif
#endif // FPL_ENABLE_ASSERTIONS

// Window
#if !defined(FPL_NO_WINDOW) && !defined(FPL_APPTYPE_CONSOLE)
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
#	if !defined(FPL_NO_AUDIO_ALSA) && defined(FPL_PLATFORM_LINUX)
		//! ALSA support is only available on POSIX
#		define FPL_SUPPORT_AUDIO_ALSA
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
#	if defined(FPL_SUPPORT_AUDIO_ALSA)
		//! Enable ALSA Audio
#		define FPL_ENABLE_AUDIO_ALSA
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
#define fpl_null NULL

#if defined(FPL_IS_CPP)
	//! No additional extern definition required for C++
#   define fpl_extern
#else
	//! Require extern definition on C99
#   define fpl_extern extern
#endif

#if defined(FPL_API_AS_PRIVATE)
	//! Private api call
#	define fpl_api static
#else
	//! Public api call
#	define fpl_api fpl_extern
#endif // FPL_API_AS_PRIVATE

//! Platform api definition
#if defined(FPL_IS_CPP)
	//! Disable function name mangling in C++
#	define fpl_no_function_name_mangling extern "C"
#else
	//! No function name mangling on C
#	define fpl_no_function_name_mangling
#endif
//! Platform api definition
#define fpl_platform_api fpl_no_function_name_mangling fpl_api
//! Common api definition
#define fpl_common_api fpl_no_function_name_mangling fpl_api
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
#include <stdbool.h> // bool
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <limits.h> // UINT32_MAX, ...

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

#if defined(FPL_PLATFORM_WIN32)
	//! Manually allocate memory on the stack (Win32)
#	include <malloc.h>
#	define FPL_STACKALLOCATE(size) _alloca(size)
#else
	//! Manually allocate memory on the stack (Others)
#	include <alloca.h>
#	define FPL_STACKALLOCATE(size) alloca(size)
#endif

#if defined(FPL_IS_CPP)
	//! Macro for overloading enum operators in C++
#	define FPL_ENUM_AS_FLAGS_OPERATORS(etype) \
	inline etype operator | (etype a, etype b) { \
		return static_cast<etype>(static_cast<int>(a) | static_cast<int>(b)); \
	} \
	inline bool operator & (etype a, etype b) { \
		return (static_cast<etype>(static_cast<int>(a) & static_cast<int>(b))) == b; \
	} \
	inline etype& operator |= (etype &a, etype b) { \
		return a = a | b; \
	}
#else
	//! No need to overload operators for enums in C99
#	define FPL_ENUM_AS_FLAGS_OPERATORS(etype)
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
#	include <Windows.h> // Win32 api
#endif // FPL_PLATFORM_WIN32

#if defined(FPL_SUBPLATFORM_POSIX)
#	include <pthread.h> // pthread_t, pthread_mutex_, pthread_cond_, pthread_barrier_
#endif // FPL_SUBPLATFORM_POSIX

#if defined(FPL_SUBPLATFORM_X11)
#   include <X11/X.h> // Window
#   include <X11/Xlib.h> // Display
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
  * \brief Atomic functions such as Compare And Exchange, Fences, Loads, Stores, etc.
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
  */
fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
/**
  * \brief Overwrites the 64-bit unsigned value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  */
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
/**
  * \brief Overwrites the 32-bit signed value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  */
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value);
/**
  * \brief Overwrites the 64-bit signed value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  */
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value);
/**
  * \brief Overwrites the pointer value atomically.
  * Ensures that memory operations are completed before the write.
  * \param dest The destination to write to.
  * \param value The value to exchange with.
  */
fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup OS Operating system infos
  * \brief Retrievement of operating system informations
  * \{
  *
  */
// ----------------------------------------------------------------------------

//! Version info container
typedef struct fplVersionInfo {
	//! Major version
	uint16_t major;
	//! Minor version
	uint16_t minor;
	//! Fix version
	uint16_t fix;
	//! Build version
	uint16_t build;
} fplVersionInfo;

//! Operating system info container
typedef struct fplOSInfos {
	//! Name of the system
	char systemName[256];
	//! Name of the kernel
	char kernelName[256];
	//! System version
	fplVersionInfo systemVersion;
	//! Kernel version
	fplVersionInfo kernelVersion;
} fplOSInfos;

/**
  * \brief Returns the basic infos of the operating system
  * \param outInfos Pointer to a \ref fplOSInfos structure
  * \note This may be called without initializing the platform
  * \return True when the infos could be retrieved, false otherwise.
  */
fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos);

/**
  * \brief Returns the username of the current logged-in user
  * \param nameBuffer Name buffer
  * \param maxNameBufferLen Max name buffer length
  * \return True when a username could be retrieved, false otherwise.
  */
fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, size_t maxNameBufferLen);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Hardware Hardware infos
  * \brief Retrievement of hardware informations
  * \{
  *
  */
// ----------------------------------------------------------------------------

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

//! Architecture type
typedef enum fplArchType {
	//! Unknown architecture
	fplArchType_Unknown = 0,
	//! X86 architecture
	fplArchType_x86,
	//! X86_64 architecture
	fplArchType_x86_64,
	//! X64 architecture
	fplArchType_x64,
	//! ARM32 architecture
	fplArchType_Arm32,
	//! ARM64 architecture
	fplArchType_Arm64,
} fplArchType;

/**
  * \brief Returns the string representation of the given architecture type
  * \param type Architecture type
  * \return Returns a string for the given architecture type
  */
fpl_inline const char *fplGetArchTypeString(const fplArchType type) {
	switch (type) {
		case fplArchType_x86:
			return "x86";
		case fplArchType_x86_64:
			return "x86_64";
		case fplArchType_x64:
			return "x64";
		case fplArchType_Arm32:
			return "Arm32";
		case fplArchType_Arm64:
			return "Arm64";
		case fplArchType_Unknown:
		default:
			return "Unknown";
	}
}

/**
  * \brief Returns the total number of processor cores.
  * \return Number of processor cores.
  */
fpl_platform_api size_t fplGetProcessorCoreCount();
/**
  * \brief Returns the name of the processor.
  * The processor name is written in the destination buffer.
  * \param destBuffer The character buffer to write the processor name into.
  * \param maxDestBufferLen The total number of characters available in the destination character buffer.
  * \return Name of the processor.
  */
fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen);
/**
  * \brief Returns the current system memory informations.
  * \param outInfos Pointer to a \ref fplMemoryInfos structure
  * \return Returns true when memory infos could be retrieved, false otherwise.
  */
fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos);

/**
  * \brief Returns the current architecture
  * \return \ref fplArchType
  */
fpl_platform_api fplArchType fplGetRunningArchitecture();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Settings Settings and configurations
  * \brief Video/audio/window settings
  * \{
  */
// ----------------------------------------------------------------------------

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

//! Init result type
typedef enum fplInitResultType {
	//! Window creation failed
	fplInitResultType_FailedWindow = -40,
	//! Audio initialization failed
	fplInitResultType_FailedAudio = -30,
	//! Video initialization failed
	fplInitResultType_FailedVideo = -20,
	//! Platform initialization failed
	fplInitResultType_FailedPlatform = -10,
	//! Failed allocating required memory
	fplInitResultType_FailedAllocatingMemory = -2,
	//! Platform is already initialized
	fplInitResultType_AlreadyInitialized = -1,
	//! Everything is fine
	fplInitResultType_Success = 1,
} fplInitResultType;

//! Returns the string representation of a \ref fplInitResultType
fpl_inline const char *fplGetInitResultTypeString(const fplInitResultType type) {
	switch (type) {
		case fplInitResultType_AlreadyInitialized:
			return "Already initialized";
		case fplInitResultType_FailedAllocatingMemory:
			return "Failed allocating memory";
		case fplInitResultType_FailedPlatform:
			return "Failed initializing platform";
		case fplInitResultType_FailedVideo:
			return "Failed initializing video";
		case fplInitResultType_FailedAudio:
			return "Failed initializing audio";
		case fplInitResultType_FailedWindow:
			return "Failed initializing window";
		case fplInitResultType_Success:
			return "Success";
		default:
			return "";
	}
}

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

//! Graphics Api settings union
typedef union fplGraphicsApiSettings {
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	//! OpenGL settings
	fplOpenGLVideoSettings opengl;
#endif
	//! Dummy field when no graphics drivers are available
	int dummy;
} fplGraphicsApiSettings;

//! Video settings container (Driver, Flags, Version, VSync, etc.)
typedef struct fplVideoSettings {
	//! Video driver type
	fplVideoDriverType driver;
	//! Vertical syncronisation enabled/disabled
	bool isVSync;
	//! Backbuffer size is automatically resized. Useable only for software rendering!
	bool isAutoSize;
	//! Graphics Api settings
	fplGraphicsApiSettings graphics;
} fplVideoSettings;

/**
  * \brief Resets the given video settings to default values
  * \param video Video settings
  * \note This will not change any video settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video);

//! Audio driver type
typedef enum fplAudioDriverType {
	//! No audio driver
	fplAudioDriverType_None = 0,
	//! Auto detection
	fplAudioDriverType_Auto,
	//! DirectSound
	fplAudioDriverType_DirectSound,
	//! ALSA
	fplAudioDriverType_Alsa,
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
#if defined(FPL_ENABLE_AUDIO_ALSA)
	//! ALSA Device Name
	char alsa[256];
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

#if defined(FPL_ENABLE_AUDIO_ALSA)
//! ALSA driver specific settings
typedef struct fplAlsaAudioSettings {
	//! Disable the usage of MMap in ALSA
	bool noMMap;
} fplAlsaAudioSettings;
#endif

//! Driver specific audio settings
typedef union fplSpecificAudioSettings {
#if defined(FPL_ENABLE_AUDIO_ALSA)
	//! Alsa specific settings
	fplAlsaAudioSettings alsa;
#endif
	//! Dummy field (When no drivers are available)
	int dummy;
} fplSpecificAudioSettings;

//! Audio Client Read Callback Function
typedef uint32_t(fpl_audio_client_read_callback)(const fplAudioDeviceFormat *deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

//! Audio settings
typedef struct fplAudioSettings {
	//! The device format
	fplAudioDeviceFormat deviceFormat;
	//! The device info
	fplAudioDeviceInfo deviceInfo;
	//! Specific settings
	fplSpecificAudioSettings specific;
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
  * \brief Resets the given audio settings to default settings (S16 PCM, 48 KHz, 2 Channels)
  * \param audio Audio settings
  * \note This will not change any audio settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultAudioSettings(fplAudioSettings *audio);

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
	//! Is window decorated
	bool isDecorated;
	//! Is floating
	bool isFloating;
	//! Is window in fullscreen mode
	bool isFullscreen;
} fplWindowSettings;

/**
  * \brief Resets the given window settings container to default settings
  * \param window Window settings
  * \note This will not change any window settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultWindowSettings(fplWindowSettings *window);

//! Input settings
typedef struct fplInputSettings {
	//! Frequency in ms for detecting new or removed controllers (Default: 100 ms)
	uint32_t controllerDetectionFrequency;
} fplInputSettings;

/**
  * \brief Resets the given input settings contains to default values.
  * \param input Input settings
  * \note This will not change any input settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultInputSettings(fplInputSettings *input);

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
  * \brief Resets the given settings container to default values for window, video, audio, etc.
  * \param settings Settings
  * \note This will not change the active settings! To change the actual settings you have to pass this settings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultSettings(fplSettings *settings);
/**
  * \brief Creates a full settings struct containing default values
  * \return Settings structure
  */
fpl_common_api fplSettings fplMakeDefaultSettings();
/**
  * \brief Returns the current settings
  */
fpl_common_api const fplSettings *fplGetCurrentSettings();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Platform Platform functions
  * \brief Platform functions
  * \{
  */
// ----------------------------------------------------------------------------

//! Platform type
typedef enum fplPlatformType {
	//! Unknown platform
	fplPlatformType_Unknown = 0,
	//! Windows platform
	fplPlatformType_Windows,
	//! Linux platform
	fplPlatformType_Linux,
	//! Unix platform
	fplPlatformType_Unix,
} fplPlatformType;

/**
  * \brief Returns the string representation of the given platform type
  * \param type Platform type
  * \return Returns a string for the given platform type
  */
fpl_inline const char *fplGetPlatformTypeString(const fplPlatformType type) {
	switch (type) {
		case fplPlatformType_Windows:
			return "Windows";
		case fplPlatformType_Linux:
			return "Linux";
		case fplPlatformType_Unix:
			return "Unix";
		case fplPlatformType_Unknown:
			return "Unknown";
		default:
			return "";
	}
}

/**
  * \brief Initializes the platform layer.
  * \param initFlags Optional init flags used for enable certain features, like video/audio etc. (Default: \ref fplInitFlags_All)
  * \param initSettings Optional initialization settings which can be passed to control the platform layer behavior or systems.
  * \note \ref fplPlatformRelease() must be called when you are done! After \ref fplPlatformRelease() has been called you can call this function again if needed.
  * \return Returns a \ref fplInitResultType . Will return \ref fplInitResultType_AlreadyInitialized when the platform layers is already initialized.
  */
fpl_common_api fplInitResultType fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings);
/**
  * \brief Releases the resources allocated by the platform layer.
  * \note Can only be called when \ref fplPlatformInit() was successful.
  */
fpl_common_api void fplPlatformRelease();
/**
  * \brief Returns the type of the platform
  * \return \ref fplPlatformType
  */
fpl_common_api fplPlatformType fplGetPlatformType();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup ErrorHandling Error Handling
  * \brief Functions for error handling
  * \{
  */
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
/**
  * \defgroup DynamicLibrary Dynamic library loading
  * \brief Loading dynamic libraries and retrieving the procedure addresses.
  * \{
  */
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
/**
  * \defgroup Console Console functions
  * \brief Console out/in functions
  * \{
  */
// ----------------------------------------------------------------------------

  /**
	* \brief Writes the given text to the standard output console buffer.
	* \param text The text to write into standard output console.
	* \note This is most likely just a wrapper call to fprintf(stdout)
	*/
fpl_platform_api void fplConsoleOut(const char *text);
/**
  * \brief Writes the given text to the standard error console buffer.
  * \param text The text to write into standard error console.
  * \note This is most likely just a wrapper call to fprintf(stderr)
  */
fpl_platform_api void fplConsoleError(const char *text);
/**
  * \brief Wait for a character to be typed in the console input and return it
  * \note This is most likely just a wrapper call to getchar()
  * \return Character typed in in the console input
  */
fpl_platform_api char fplConsoleWaitForCharInput();

/**
  * \brief Writes the given formatted text to the standard output console buffer.
  * \param format The format used for writing into the standard output console.
  * \param ... The dynamic arguments used for formatting the text.
  * \note This is most likely just a wrapper call to vfprintf(stdout)
  */
fpl_common_api void fplConsoleFormatOut(const char *format, ...);
/**
  * \brief Writes the given formatted text to the standard error console buffer.
  * \param format The format used for writing into the standard error console.
  * \param ... The dynamic arguments used for formatting the text.
  * \note This is most likely just a wrapper call to vfprintf(stderr)
  */
fpl_common_api void fplConsoleFormatError(const char *format, ...);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Timings Timing functions
  * \brief Functions for retrieving timebased informations
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Returns the current system clock in seconds in high precision (micro/nano seconds)
  * \return Returns number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api double fplGetTimeInSecondsHP();
/**
  * \brief Returns the current system clock in seconds in low precision (milliseconds)
  * \return Returns number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInSecondsLP();
/**
  * \brief Returns the current system clock in seconds in default precision
  * \return Returns number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time. There is no guarantee to get high precision here, use for high precision \ref fplGetTimeInSecondsHP() instead!
  */
fpl_platform_api double fplGetTimeInSeconds();
/**
  * \brief Returns the current system clock in milliseconds in high precision (micro/nano seconds)
  * \return Returns number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api double fplGetTimeInMillisecondsHP();
/**
  * \brief Returns the current system clock in milliseconds in low precision (milliseconds)
  * \return Returns number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInMillisecondsLP();
/**
  * \brief Returns the current system clock in milliseconds
  * \return Returns number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInMilliseconds();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Threading Threading and syncronisation routines
  * \brief Tons of functions for multithreading, mutex and signal creation and handling
  * \{
  */
// ----------------------------------------------------------------------------

//! Type definition for a timeout value in milliseconds
typedef uint32_t fplTimeoutValue;
//! Infinite timeout constant
#define FPL_TIMEOUT_INFINITE UINT32_MAX

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

//! Internal thread handle union
typedef union fplInternalThreadHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 thread handle
	HANDLE win32ThreadHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX thread
	pthread_t posixThread;
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
#elif defined(FPL_PLATFORM_LINUX)
	//! Linux event handle
	int linuxEventHandle;
#endif
	//! Dummy field
	int dummy;
} fplInternalSignalHandle;

//! Signal handle
typedef struct fplSignalHandle {
	//! The internal signal handle
	fplInternalSignalHandle internalHandle;
	//! Is it valid
	bool isValid;
} fplSignalHandle;

//! Signal value enumeration
typedef enum fplSignalValue {
	//! Value is unset
	fplSignalValue_Unset = 0,
	//! Value is set
	fplSignalValue_Set = 1,
} fplSignalValue;

//! Internal condition variable
typedef union fplInternalConditionVariable {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 condition variable
	CONDITION_VARIABLE win32Condition;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX condition variable
	pthread_cond_t posixCondition;
#endif	//! Dummy field
	int dummy;
} fplInternalConditionVariable;

//! Condition variable
typedef struct fplConditionVariable {
	//! The internal condition handle
	fplInternalConditionVariable internalHandle;
	//! Is it valid
	bool isValid;
} fplConditionVariable;

//! Returns the current thread state from the given thread
fpl_inline fplThreadState fplGetThreadState(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		return fplThreadState_Stopped;
	}
	fplThreadState result = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&thread->currentState);
	return(result);
}

/**
  * \brief Creates and starts a thread and returns the handle to it.
  * \param runFunc Function prototype called when this thread starts.
  * \param data User data passed to the run function.
  * \note The resources are automatically cleaned up when the thread terminates.
  * \warning Do not free this thread context directly!
  * \return Pointer to the thread handle or fpl_null when the limit of active threads has been reached.
  */
fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data);
/**
  * \brief Let the current thread sleep for the given amount of milliseconds.
  * \param milliseconds Number of milliseconds to sleep
  * \note There is no guarantee that the OS sleeps for the exact amount of milliseconds! This can vary based on the OS scheduler granularity.
  */
fpl_platform_api void fplThreadSleep(const uint32_t milliseconds);
/**
  * \brief Forced the given thread to stop and release all underlying resources.
  * \param thread Thread handle
  * \note This thread context may get re-used for another thread in the future.
  * \note Will return false for already terminated threads.
  * \warning Do not free the given thread context manually!
  * \return Returns true when the thread was terminated, false otherwise.
  */
fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread);
/**
  * \brief Wait until the given thread is done running or the given timeout has been reached.
  * \param thread Thread
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when the thread completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout);
/**
  * \brief Wait until all given threads are done running or the given timeout has been reached.
  * \param threads Array of threads
  * \param count Number of threads in the array
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when all threads completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Wait until one of given threads is done running or the given timeout has been reached.
  * \param threads Array of threads
  * \param count Number of threads in the array
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when one thread completes or when the timeout has been reached.
  */
fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout);

/**
  * \brief Initializes the given mutex
  * \param mutex Pointer to a mutex handle
  * \note Use \ref fplMutexDestroy() when you are done with this mutex.
  * \return True when the mutex was initialized or false otherwise.
  */
fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex);
/**
  * \brief Releases the given mutex and clears the structure to zero.
  * \param mutex Pointer to a mutex handle
  */
fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex);
/**
  * \brief Locks the given mutex and blocks other threads.
  * \param mutex Pointer to a mutex handle
  * \returns True when mutex was locked or false otherwise.
  */
fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex);
/**
  * \brief Tries to lock the given mutex without blocking other threads.
  * \param mutex Pointer to a mutex handle
  * \returns True when mutex was locked or false when mutex is already locked.
  */
fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex);
/**
 * \brief Unlocks the given mutex
 * \param mutex Pointer to a mutex handle
 * \returns True when mutex was unlocked or false otherwise.
 */
fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex);

/**
  * \brief Initializes the given signal
  * \param signal Pointer to a signal handle
  * \param initialValue Initial value the signal is set to
  * \note Use \ref fplSignalDestroy() when you are done with this Signal.
  * \return True when initialization was successful, false otherwise.
  */
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue);
/**
  * \brief Releases the given signal and clears the structure to zero.
  * \param signal Pointer to a signal handle
  */
fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal);
/**
  * \brief Waits until the given signal are waked up.
  * \param signal Pointer to a signal handle
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when the signal woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout);
/**
  * \brief Waits until all the given signal are waked up.
  * \param signals Array of signals
  * \param count Number of signals
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when all signals woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Waits until any of the given signals wakes up or the timeout has been reached.
  * \param signals Array of signals
  * \param count Number of signals
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when any of the signals woke up or the timeout has been reached, otherwise false.
  */
fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Sets the signal and wakes up the given signal.
  * \param signal Pointer to a signal handle
  * \return Returns true when the signal was set and broadcasted or false otherwise.
  */
fpl_platform_api bool fplSignalSet(fplSignalHandle *signal);
/**
  * \brief Resets the signal.
  * \param signal Pointer to a signal handle
  * \return Returns true when the signal was reset, false otherwise.
  */
fpl_platform_api bool fplSignalReset(fplSignalHandle *signal);

/**
  * \brief Initializes the given condition
  * \param condition Pointer to a \ref fplConditionVariable
  * \note Use \ref fplSignalDestroy() when you are done with this Signal.
  * \return True when initialization was successful, false otherwise.
  */
fpl_platform_api bool fplConditionInit(fplConditionVariable *condition);
/**
  * \brief Releases the given condition and clears the structure to zero.
  * \param condition Pointer to a \ref fplConditionVariable
  */
fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition);
/**
  * \brief Sleeps on the given condition and releases the mutex when done.
  * \param condition Pointer to a \ref fplConditionVariable
  * \param mutex Pointer to a \ref fplMutexHandle
  * \param timeout Number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return True when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout);
/**
  * \brief Wakes up one thread which waits on the given condition.
  * \param condition Pointer to a \ref fplConditionVariable
  * \return True when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition);
/**
  * \brief Wakes up all threads which waits on the given condition.
  * \param condition Pointer to a \ref fplConditionVariable
  * \return True when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Memory Memory functions
  * \brief Memory allocation, clearing and copy functions
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Clears the given memory by the given size to zero.
  * \param mem Pointer to the memory.
  * \param size Size in bytes to be cleared to zero.
  */
fpl_common_api void fplMemoryClear(void *mem, const size_t size);
/**
  * \brief Sets the given memory by the given size to the given value.
  * \param mem Pointer to the memory.
  * \param value Value
  * \param size Size in bytes to be cleared to zero.
  */
fpl_common_api void fplMemorySet(void *mem, const uint8_t value, const size_t size);
/**
  * \brief Copies the given source memory with its length to the target memory.
  * \param sourceMem Pointer to the source memory to copy from.
  * \param sourceSize Size in bytes to be copied.
  * \param targetMem Pointer to the target memory to copy to.
  */
fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem);
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
  */
fpl_common_api void fplMemoryAlignedFree(void *ptr);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Strings String manipulation functions
  * \brief Functions for converting/manipulating strings
  * \{
  */
// ----------------------------------------------------------------------------

  /**
	* \brief Returns true when both ansi strings are equal with enforcing the given length.
	* \param a First string
	* \param aLen Number of characters for the first string
	* \param b Second string
	* \param bLen Number of characters for the second string
	* \note Len parameters does not include the null-terminator!
	* \return True when strings matches, otherwise false.
	*/
fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen);
/**
  * \brief Returns true when both ansi strings are equal.
  * \param a First string
  * \param b Second string
  * \return True when strings matches, otherwise false.
  */
fpl_common_api bool fplIsStringEqual(const char *a, const char *b);
/**
  * \brief Appends the source string to the given buffer
  * \param appended Appending string
  * \param appendedLen Length of the source string
  * \param buffer Target buffer
  * \param maxBufferLen Max length of the target buffer
  * \return Pointer to the first character or fpl_null.
  */
fpl_common_api char *fplStringAppendLen(const char *appended, const size_t appendedLen, char *buffer, size_t maxBufferLen);

/**
  * \brief Appends the source string to the given buffer
  * \param appended Appending string
  * \param buffer Target buffer
  * \param maxBufferLen Max length of the target buffer
  * \return Pointer to the first character or fpl_null.
  */
fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen);
/**
  * \brief Returns the number of characters of the given 8-bit Ansi string.
  * \param str The 8-bit ansi string
  * \note Null terminator is not included!
  * \return Returns the character length or zero when the input string is fpl_null.
  */
fpl_common_api size_t fplGetAnsiStringLength(const char *str);
/**
  * \brief Returns the number of characters of the given 16-bit wide string.
  * \param str The 16-bit wide string
  * \note Null terminator is not included!
  * \return Returns the character length or zero when the input string is fpl_null.
  */
fpl_common_api size_t fplGetWideStringLength(const wchar_t *str);
/**
  * \brief Copies the given 8-bit source ansi string with a fixed length into a destination ansi string.
  * \param source The 8-bit source ansi string.
  * \param sourceLen The number of characters to copy.
  * \param dest The 8-bit destination ansi string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api char *fplCopyAnsiStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 8-bit source ansi string into a destination ansi string.
  * \param source The 8-bit source ansi string.
  * \param dest The 8-bit destination ansi string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string with a fixed length into a destination wide string.
  * \param source The 16-bit source wide string.
  * \param sourceLen The number of characters to copy.
  * \param dest The 16-bit destination wide string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const size_t sourceLen, wchar_t *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string into a destination wide string.
  * \param source The 16-bit source wide string.
  * \param dest The 16-bit destination wide string buffer.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const size_t maxDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit ansi string.
  * \param wideSource The 16-bit source wide string.
  * \param maxWideSourceLen The number of characters of the source wide string.
  * \param ansiDest The 8-bit destination ansi string buffer.
  * \param maxAnsiDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const size_t maxWideSourceLen, char *ansiDest, const size_t maxAnsiDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit UTF-8 ansi string.
  * \param wideSource The 16-bit source wide string.
  * \param maxWideSourceLen The number of characters of the source wide string.
  * \param utf8Dest The 8-bit destination ansi string buffer.
  * \param maxUtf8DestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen);
/**
  * \brief Converts the given 8-bit source ansi string with length in a 16-bit wide string.
  * \param ansiSource The 8-bit source ansi string.
  * \param ansiSourceLen The number of characters of the source wide string.
  * \param wideDest The 16-bit destination wide string buffer.
  * \param maxWideDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen);
/**
  * \brief Converts the given 8-bit UTF-8 source ansi string with length in a 16-bit wide string.
  * \param utf8Source The 8-bit source ansi string.
  * \param utf8SourceLen The number of characters of the source wide string.
  * \param wideDest The 16-bit destination wide string buffer.
  * \param maxWideDestLen The total number of characters available in the destination buffer.
  * \note Null terminator is included always. Does not allocate any memory.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null when either the dest buffer is too small or the source string is invalid.
  */
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen);
/**
  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and variable arguments.
  * \param ansiDestBuffer The 8-bit destination ansi string buffer.
  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer.
  * \param format The string format.
  * \param ... Variable arguments.
  * \note This is most likely just a wrapper call to vsnprintf()
  * \return Pointer to the first character in the destination buffer or fpl_null.
  */

fpl_common_api char *fplFormatAnsiString(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, ...);
/**
  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and the arguments list.
  * \param ansiDestBuffer The 8-bit destination ansi string buffer.
  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer.
  * \param format The string format.
  * \param argList Arguments list.
  * \note This is most likely just a wrapper call to vsnprintf()
  * \return Pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplFormatAnsiStringArgs(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, va_list argList);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Files Files/IO functions
  * \brief Tons of file and directory IO functions
  * \{
  */
// ----------------------------------------------------------------------------

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
fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle);

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

// ----------------------------------------------------------------------------
/**
  * \defgroup Paths Path functions
  * \brief Functions for retrieving paths like HomePath, ExecutablePath, etc.
  * \{
  */
// ----------------------------------------------------------------------------

// @TODO(final): Support wide path for 'paths' as well

/**
  * \brief Returns the full path to this executable, including the executable file name.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen);
/**
  * \brief Returns the full path to your home directory.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen);
/**
  * \brief Returns the path from the given source path.
  * \param sourcePath Source path to extract from.
  * \param destPath Destination buffer
  * \param maxDestLen Total number of characters available in the destination buffer.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen);
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
fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen);
/**
  * \brief Combines all included path by the systems path separator.
  * \param destPath Destination buffer
  * \param maxDestPathLen Total number of characters available in the destination buffer.
  * \param pathCount Number of dynamic path arguments.
  * \param ... Dynamic path arguments.
  * \note Result is written in the destination buffer.
  * \return Returns the pointer to the first character in the destination buffer or fpl_null.
  */
fpl_common_api char *fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...);

/** \}*/

#if defined(FPL_ENABLE_WINDOW)
// ----------------------------------------------------------------------------
/**
* \defgroup WindowEvents Window events
* \brief Window event structures
* \{
*/
// ----------------------------------------------------------------------------

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
  * \param outSize Pointer to a window size container
  * \return True when we got the window area from the current window or false otherwise.
  */
fpl_platform_api bool fplGetWindowArea(fplWindowSize *outSize);
/**
  * \brief Resizes the window to fit the inner area to the given size.
  * \param width Width in screen units
  * \param height Height in screen units
  */
fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height);
/**
  * \brief Returns true when the window is resizable.
  * \return True when the window is resizable, otherwise false.
  */
fpl_platform_api bool fplIsWindowResizable();
/**
  * \brief Enables or disables the ability to resize the window.
  * \param value Set this to true for making the window resizable or false for making it static
  */
fpl_platform_api void fplSetWindowResizeable(const bool value);
/**
  * \brief Returns true when the window is decorated.
  * \return True when the window is decorated, otherwise false.
  */
fpl_platform_api bool fplIsWindowDecorated();
/**
  * \brief Enables or disables the window decoration (Titlebar, Border, etc.).
  * \param value Set this to true for making the window decorated or false for making it without decoration
  */
fpl_platform_api void fplSetWindowDecorated(const bool value);
/**
  * \brief Returns true when the window is floating.
  * \return True when the window is floating, otherwise false.
  */
fpl_platform_api bool fplIsWindowFloating();
/**
  * \brief Enables or disables the window floating (Top-most)
  * \param value Set this to true for making the window floated or false for making it not floated
  */
fpl_platform_api void fplSetWindowFloating(const bool value);
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
  * \param outPos Pointer to a window position container
  * \return True when we got the position otherwise false.
  */
fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos);
/**
  * \brief Sets the window absolut position to the given coordinates.
  * \param left Left position in screen units.
  * \param top Top position in screen units.
  */
fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top);
/**
  * \brief Sets the window title from a ansi string.
  * \param ansiTitle New title ansi string
  */
fpl_platform_api void fplSetWindowAnsiTitle(const char *ansiTitle);
/**
  * \brief Sets the window title from a wide string.
  * \param wideTitle New title wide string
  */
fpl_platform_api void fplSetWindowWideTitle(const wchar_t *wideTitle);

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
	* \return True when the clipboard contained text which is copied into the dest buffer, fpl_null otherwise.
	*/
fpl_platform_api bool fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
/**
  * \brief Returns the current clipboard wide text.
  * \param dest The destination wide string buffer to write the clipboard text into.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \return True when the clipboard contained text which is copied into the dest buffer, fpl_null otherwise.
  */
fpl_platform_api bool fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
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
// ----------------------------------------------------------------------------
/**
  * \defgroup Video Video functions
  * \brief Functions for retrieving or resizing the video buffer
  * \{
  */
// ----------------------------------------------------------------------------

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
	switch (driver) {
		case fplVideoDriverType_OpenGL:
			return "OpenGL";
		case fplVideoDriverType_Software:
			return "Software";
		case fplVideoDriverType_None:
			return "None";
		default:
			return "";
	}
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
// ----------------------------------------------------------------------------
/**
  * \defgroup Audio Audio functions
  * \brief Functions for start/stop playing audio and retrieving/changing some audio related settings.
  * \{
  */
// ----------------------------------------------------------------------------

//! Audio result
typedef enum fplAudioResult {
	fplAudioResult_None = 0,
	fplAudioResult_Success,
	fplAudioResult_DeviceNotInitialized,
	fplAudioResult_DeviceAlreadyStopped,
	fplAudioResult_DeviceAlreadyStarted,
	fplAudioResult_DeviceBusy,
	fplAudioResult_NoDeviceFound,
	fplAudioResult_ApiFailed,
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
	switch (format) {
		case fplAudioFormatType_U8:
			return "U8";
		case fplAudioFormatType_S16:
			return "S16";
		case fplAudioFormatType_S24:
			return "S24";
		case fplAudioFormatType_S32:
			return "S32";
		case fplAudioFormatType_S64:
			return "S64";
		case fplAudioFormatType_F32:
			return "F32";
		case fplAudioFormatType_F64:
			return "F64";
		default:
			return "None";
	}
}

/**
  * \brief Returns the string for the given audio driver
  * \param driver The audio driver
  * \return String for the given audio driver
  */
fpl_inline const char *fplGetAudioDriverString(fplAudioDriverType driver) {
	switch (driver) {
		case fplAudioDriverType_Auto:
			return "Auto";
		case fplAudioDriverType_DirectSound:
			return "DirectSound";
		case fplAudioDriverType_None:
			return "None";
		default:
			return "";
	}
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

// Only include C-Runtime functions when CRT is enabled
#if !defined(FPL_NO_CRT)
#	include <stdio.h> // stdin, stdout, stderr, fprintf, vfprintf, vsnprintf, getchar
#	include <stdlib.h> // wcstombs, mbstowcs, getenv
#endif

//
// Very simple logging system
//
#if defined(FPL_ENABLE_LOGGING)
#	define FPL_LOG_FORMAT(what, format) "[" what "] " format "\n"
#   define FPL_LOG(what, format, ...) do { \
		fplConsoleFormatOut(FPL_LOG_FORMAT(what, format), ## __VA_ARGS__); \
	} while (0)
#   define FPL_LOG_FUNCTION_N(what, name) fplConsoleFormatOut("> %s %s\n", what, name)
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

#if defined(FPL_PLATFORM_WIN32) && !defined(FPL_NO_ENTRYPOINT)
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

fpl_globalvar struct fpl__PlatformAppState *fpl__global__AppState = fpl_null;

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
#	include <ShlObj.h>		// SHGetFolderPath
#	include <intrin.h>		// Interlock*
#	include <Xinput.h>		// XInputGetState

#	if defined(FPL_IS_CPP)
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(a, b)
#	else
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(&a, &b)
#	endif

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
#define FPL__XINPUT_GET_CAPABILITIES(name) DWORD WINAPI name(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
typedef FPL__XINPUT_GET_CAPABILITIES(fpl__win32_func_XInputGetCapabilities);
FPL__XINPUT_GET_CAPABILITIES(fpl__Win32XInputGetCapabilitiesStub) {
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
#define FPL__FUNC_WIN32_ChoosePixelFormat(name) int WINAPI name(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FPL__FUNC_WIN32_ChoosePixelFormat(fpl__win32_func_ChoosePixelFormat);
#define FPL__FUNC_WIN32_SetPixelFormat(name) BOOL WINAPI name(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FPL__FUNC_WIN32_SetPixelFormat(fpl__win32_func_SetPixelFormat);
#define FPL__FUNC_WIN32_DescribePixelFormat(name) int WINAPI name(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
typedef FPL__FUNC_WIN32_DescribePixelFormat(fpl__win32_func_DescribePixelFormat);
#define FPL__FUNC_WIN32_GetDeviceCaps(name) int WINAPI name(HDC hdc, int index)
typedef FPL__FUNC_WIN32_GetDeviceCaps(fpl__win32_func_GetDeviceCaps);
#define FPL__FUNC_WIN32_StretchDIBits(name) int WINAPI name(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, CONST VOID *lpBits, CONST BITMAPINFO *lpbmi, UINT iUsage, DWORD rop)
typedef FPL__FUNC_WIN32_StretchDIBits(fpl__win32_func_StretchDIBits);
#define FPL__FUNC_WIN32_DeleteObject(name) BOOL WINAPI name( _In_ HGDIOBJ ho)
typedef FPL__FUNC_WIN32_DeleteObject(fpl__win32_func_DeleteObject);
#define FPL__FUNC_WIN32_SwapBuffers(name) BOOL WINAPI name(HDC)
typedef FPL__FUNC_WIN32_SwapBuffers(fpl__win32_func_SwapBuffers);

// ShellAPI
#define FPL__FUNC_WIN32_CommandLineToArgvW(name) LPWSTR* WINAPI name(LPCWSTR lpCmdLine, int *pNumArgs)
typedef FPL__FUNC_WIN32_CommandLineToArgvW(fpl__win32_func_CommandLineToArgvW);
#define FPL__FUNC_WIN32_SHGetFolderPathA(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
typedef FPL__FUNC_WIN32_SHGetFolderPathA(fpl__win32_func_SHGetFolderPathA);
#define FPL__FUNC_WIN32_SHGetFolderPathW(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
typedef FPL__FUNC_WIN32_SHGetFolderPathW(fpl__win32_func_SHGetFolderPathW);

// User32
#define FPL__FUNC_WIN32_RegisterClassExA(name) ATOM WINAPI name(CONST WNDCLASSEXA *)
typedef FPL__FUNC_WIN32_RegisterClassExA(fpl__win32_func_RegisterClassExA);
#define FPL__FUNC_WIN32_RegisterClassExW(name) ATOM WINAPI name(CONST WNDCLASSEXW *)
typedef FPL__FUNC_WIN32_RegisterClassExW(fpl__win32_func_RegisterClassExW);
#define FPL__FUNC_WIN32_UnregisterClassA(name) BOOL WINAPI name(LPCSTR lpClassName, HINSTANCE hInstance)
typedef FPL__FUNC_WIN32_UnregisterClassA(fpl__win32_func_UnregisterClassA);
#define FPL__FUNC_WIN32_UnregisterClassW(name) BOOL WINAPI name(LPCWSTR lpClassName, HINSTANCE hInstance)
typedef FPL__FUNC_WIN32_UnregisterClassW(fpl__win32_func_UnregisterClassW);
#define FPL__FUNC_WIN32_ShowWindow(name) BOOL WINAPI name(HWND hWnd, int nCmdShow)
typedef FPL__FUNC_WIN32_ShowWindow(fpl__win32_func_ShowWindow);
#define FPL__FUNC_WIN32_DestroyWindow(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_DestroyWindow(fpl__win32_func_DestroyWindow);
#define FPL__FUNC_WIN32_UpdateWindow(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_UpdateWindow(fpl__win32_func_UpdateWindow);
#define FPL__FUNC_WIN32_TranslateMessage(name) BOOL WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_WIN32_TranslateMessage(fpl__win32_func_TranslateMessage);
#define FPL__FUNC_WIN32_DispatchMessageA(name) LRESULT WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_WIN32_DispatchMessageA(fpl__win32_func_DispatchMessageA);
#define FPL__FUNC_WIN32_DispatchMessageW(name) LRESULT WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_WIN32_DispatchMessageW(fpl__win32_func_DispatchMessageW);
#define FPL__FUNC_WIN32_PeekMessageA(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
typedef FPL__FUNC_WIN32_PeekMessageA(fpl__win32_func_PeekMessageA);
#define FPL__FUNC_WIN32_PeekMessageW(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
typedef FPL__FUNC_WIN32_PeekMessageW(fpl__win32_func_PeekMessageW);
#define FPL__FUNC_WIN32_DefWindowProcA(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_DefWindowProcA(fpl__win32_func_DefWindowProcA);
#define FPL__FUNC_WIN32_DefWindowProcW(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_DefWindowProcW(fpl__win32_func_DefWindowProcW);
#define FPL__FUNC_WIN32_CreateWindowExW(name) HWND WINAPI name(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
typedef FPL__FUNC_WIN32_CreateWindowExW(fpl__win32_func_CreateWindowExW);
#define FPL__FUNC_WIN32_CreateWindowExA(name) HWND WINAPI name(DWORD dwExStyle, LPCSTR lpClassName, PCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
typedef FPL__FUNC_WIN32_CreateWindowExA(fpl__win32_func_CreateWindowExA);
#define FPL__FUNC_WIN32_SetWindowPos(name) BOOL WINAPI name(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
typedef FPL__FUNC_WIN32_SetWindowPos(fpl__win32_func_SetWindowPos);
#define FPL__FUNC_WIN32_GetWindowPlacement(name) BOOL WINAPI name(HWND hWnd, WINDOWPLACEMENT *lpwndpl)
typedef FPL__FUNC_WIN32_GetWindowPlacement(fpl__win32_func_GetWindowPlacement);
#define FPL__FUNC_WIN32_SetWindowPlacement(name) BOOL WINAPI name(HWND hWnd, CONST WINDOWPLACEMENT *lpwndpl)
typedef FPL__FUNC_WIN32_SetWindowPlacement(fpl__win32_func_SetWindowPlacement);
#define FPL__FUNC_WIN32_GetClientRect(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
typedef FPL__FUNC_WIN32_GetClientRect(fpl__win32_func_GetClientRect);
#define FPL__FUNC_WIN32_GetWindowRect(name) BOOL WINAPI name(HWND hWnd, LPRECT lpRect)
typedef FPL__FUNC_WIN32_GetWindowRect(fpl__win32_func_GetWindowRect);
#define FPL__FUNC_WIN32_AdjustWindowRect(name) BOOL WINAPI name(LPRECT lpRect, DWORD dwStyle, BOOL bMenu)
typedef FPL__FUNC_WIN32_AdjustWindowRect(fpl__win32_func_AdjustWindowRect);
#define FPL__FUNC_WIN32_ClientToScreen(name) BOOL name(HWND hWnd, LPPOINT lpPoint)
typedef FPL__FUNC_WIN32_ClientToScreen(fpl__func_win32_ClientToScreen);
#define FPL__FUNC_WIN32_GetAsyncKeyState(name) SHORT WINAPI name(int vKey)
typedef FPL__FUNC_WIN32_GetAsyncKeyState(fpl__win32_func_GetAsyncKeyState);
#define FPL__FUNC_WIN32_MapVirtualKeyA(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_WIN32_MapVirtualKeyA(fpl__win32_func_MapVirtualKeyA);
#define FPL__FUNC_WIN32_MapVirtualKeyW(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_WIN32_MapVirtualKeyW(fpl__win32_func_MapVirtualKeyW);
#define FPL__FUNC_WIN32_SetCursor(name) HCURSOR WINAPI name(HCURSOR hCursor)
typedef FPL__FUNC_WIN32_SetCursor(fpl__win32_func_SetCursor);
#define FPL__FUNC_WIN32_GetCursor(name) HCURSOR WINAPI name(VOID)
typedef FPL__FUNC_WIN32_GetCursor(fpl__win32_func_GetCursor);
#define FPL__WIN32_FUNC_GetCursorPos(name) BOOL WINAPI name(LPPOINT lpPoint)
typedef FPL__WIN32_FUNC_GetCursorPos(fpl__win32_func_GetCursorPos);
#define FPL__WIN32_FUNC_WindowFromPoint(name) HWND WINAPI name(POINT Point)
typedef FPL__WIN32_FUNC_WindowFromPoint(fpl__win32_func_WindowFromPoint);
#define FPL__WIN32_FUNC_ClientToScreen(name) BOOL WINAPI name(HWND hWnd, LPPOINT lpPoint)
typedef FPL__WIN32_FUNC_ClientToScreen(fpl__win32_func_ClientToScreen);
#define FPL__WIN32_FUNC_PtInRect(name) BOOL WINAPI name(CONST RECT *lprc, POINT pt)
typedef FPL__WIN32_FUNC_PtInRect(fpl__win32_func_PtInRect);
#define FPL__FUNC_WIN32_LoadCursorA(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCSTR lpCursorName)
typedef FPL__FUNC_WIN32_LoadCursorA(fpl__win32_func_LoadCursorA);
#define FPL__FUNC_WIN32_LoadCursorW(name) HCURSOR WINAPI name(HINSTANCE hInstance, LPCWSTR lpCursorName)
typedef FPL__FUNC_WIN32_LoadCursorW(fpl__win32_func_LoadCursorW);
#define FPL__FUNC_WIN32_LoadIconA(name) HICON WINAPI name(HINSTANCE hInstance, LPCSTR lpIconName)
typedef FPL__FUNC_WIN32_LoadIconA(fpl__win32_func_LoadIconA);
#define FPL__FUNC_WIN32_LoadIconW(name) HICON WINAPI name(HINSTANCE hInstance, LPCWSTR lpIconName)
typedef FPL__FUNC_WIN32_LoadIconW(fpl__win32_func_LoadIconW);
#define FPL__FUNC_WIN32_SetWindowTextA(name) BOOL WINAPI name(HWND hWnd, LPCSTR lpString)
typedef FPL__FUNC_WIN32_SetWindowTextA(fpl__win32_func_SetWindowTextA);
#define FPL__FUNC_WIN32_SetWindowTextW(name) BOOL WINAPI name(HWND hWnd, LPCWSTR lpString)
typedef FPL__FUNC_WIN32_SetWindowTextW(fpl__win32_func_SetWindowTextW);
#define FPL__FUNC_WIN32_SetWindowLongA(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongA(fpl__win32_func_SetWindowLongA);
#define FPL__FUNC_WIN32_SetWindowLongW(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongW(fpl__win32_func_SetWindowLongW);
#define FPL__FUNC_WIN32_GetWindowLongA(name) LONG WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongA(fpl__win32_func_GetWindowLongA);
#define FPL__FUNC_WIN32_GetWindowLongW(name) LONG WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongW(fpl__win32_func_GetWindowLongW);

#if defined(FPL_ARCH_X64)
#define FPL__FUNC_WIN32_SetWindowLongPtrA(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongPtrA(fpl__win32_func_SetWindowLongPtrA);
#define FPL__FUNC_WIN32_SetWindowLongPtrW(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongPtrW(fpl__win32_func_SetWindowLongPtrW);
#define FPL__FUNC_WIN32_GetWindowLongPtrA(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongPtrA(fpl__win32_func_GetWindowLongPtrA);
#define FPL__FUNC_WIN32_GetWindowLongPtrW(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongPtrW(fpl__win32_func_GetWindowLongPtrW);
#endif

#define FPL__FUNC_WIN32_ReleaseDC(name) int WINAPI name(HWND hWnd, HDC hDC)
typedef FPL__FUNC_WIN32_ReleaseDC(fpl__win32_func_ReleaseDC);
#define FPL__FUNC_WIN32_GetDC(name) HDC WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_GetDC(fpl__win32_func_GetDC);
#define FPL__FUNC_WIN32_ChangeDisplaySettingsA(name) LONG WINAPI name(DEVMODEA* lpDevMode, DWORD dwFlags)
typedef FPL__FUNC_WIN32_ChangeDisplaySettingsA(fpl__win32_func_ChangeDisplaySettingsA);
#define FPL__FUNC_WIN32_ChangeDisplaySettingsW(name) LONG WINAPI name(DEVMODEW* lpDevMode, DWORD dwFlags)
typedef FPL__FUNC_WIN32_ChangeDisplaySettingsW(fpl__win32_func_ChangeDisplaySettingsW);
#define FPL__FUNC_WIN32_EnumDisplaySettingsA(name) BOOL WINAPI name(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode)
typedef FPL__FUNC_WIN32_EnumDisplaySettingsA(fpl__win32_func_EnumDisplaySettingsA);
#define FPL__FUNC_WIN32_EnumDisplaySettingsW(name) BOOL WINAPI name(LPCWSTR lpszDeviceName, DWORD iModeNum, DEVMODEW* lpDevMode)
typedef FPL__FUNC_WIN32_EnumDisplaySettingsW(fpl__win32_func_EnumDisplaySettingsW);
#define FPL__FUNC_WIN32_OpenClipboard(name) BOOL WINAPI name(HWND hWndNewOwner)
typedef FPL__FUNC_WIN32_OpenClipboard(fpl__win32_func_OpenClipboard);
#define FPL__FUNC_WIN32_CloseClipboard(name) BOOL WINAPI name(VOID)
typedef FPL__FUNC_WIN32_CloseClipboard(fpl__win32_func_CloseClipboard);
#define FPL__FUNC_WIN32_EmptyClipboard(name) BOOL WINAPI name(VOID)
typedef FPL__FUNC_WIN32_EmptyClipboard(fpl__win32_func_EmptyClipboard);
#define FPL__FUNC_WIN32_IsClipboardFormatAvailable(name) BOOL WINAPI name(UINT format)
typedef FPL__FUNC_WIN32_IsClipboardFormatAvailable(fpl__win32_func_IsClipboardFormatAvailable);
#define FPL__FUNC_WIN32_SetClipboardData(name) HANDLE WINAPI name(UINT uFormat, HANDLE hMem)
typedef FPL__FUNC_WIN32_SetClipboardData(fpl__win32_func_SetClipboardData);
#define FPL__FUNC_WIN32_GetClipboardData(name) HANDLE WINAPI name(UINT uFormat)
typedef FPL__FUNC_WIN32_GetClipboardData(fpl__win32_func_GetClipboardData);
#define FPL__FUNC_WIN32_GetDesktopWindow(name) HWND WINAPI name(VOID)
typedef FPL__FUNC_WIN32_GetDesktopWindow(fpl__win32_func_GetDesktopWindow);
#define FPL__FUNC_WIN32_GetForegroundWindow(name) HWND WINAPI name(VOID)
typedef FPL__FUNC_WIN32_GetForegroundWindow(fpl__win32_func_GetForegroundWindow);
#define FPL__FUNC_WIN32_IsZoomed(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_IsZoomed(fpl__win32_func_IsZoomed);
#define FPL__FUNC_WIN32_SendMessageA(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_SendMessageA(fpl__win32_func_SendMessageA);
#define FPL__FUNC_WIN32_SendMessageW(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_SendMessageW(fpl__win32_func_SendMessageW);
#define FPL__FUNC_WIN32_GetMonitorInfoA(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
typedef FPL__FUNC_WIN32_GetMonitorInfoA(fpl__win32_func_GetMonitorInfoA);
#define FPL__FUNC_WIN32_GetMonitorInfoW(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
typedef FPL__FUNC_WIN32_GetMonitorInfoW(fpl__win32_func_GetMonitorInfoW);
#define FPL__FUNC_WIN32_MonitorFromRect(name) HMONITOR WINAPI name(LPCRECT lprc, DWORD dwFlags)
typedef FPL__FUNC_WIN32_MonitorFromRect(fpl__win32_func_MonitorFromRect);
#define FPL__FUNC_WIN32_MonitorFromWindow(name) HMONITOR WINAPI name(HWND hwnd, DWORD dwFlags)
typedef FPL__FUNC_WIN32_MonitorFromWindow(fpl__win32_func_MonitorFromWindow);
#define FPL__WIN32_FUNC_RegisterRawInputDevices(name) BOOL WINAPI name(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
typedef FPL__WIN32_FUNC_RegisterRawInputDevices(fpl__win32_func_RegisterRawInputDevices);
#define FPL__WIN32_FUNC_ClipCursor(name) BOOL WINAPI name(CONST RECT *lpRect)
typedef FPL__WIN32_FUNC_ClipCursor(fpl__win32_func_ClipCursor);

// OLE32
#define FPL__FUNC_WIN32_CoInitializeEx(name) HRESULT WINAPI name(LPVOID pvReserved, DWORD  dwCoInit)
typedef FPL__FUNC_WIN32_CoInitializeEx(fpl__win32_func_CoInitializeEx);
#define FPL__FUNC_WIN32_CoUninitialize(name) void WINAPI name(void)
typedef FPL__FUNC_WIN32_CoUninitialize(fpl__win32_func_CoUninitialize);
#define FPL__FUNC_WIN32_CoCreateInstance(name) HRESULT WINAPI name(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
typedef FPL__FUNC_WIN32_CoCreateInstance(fpl__win32_func_CoCreateInstance);
#define FPL__FUNC_WIN32_CoTaskMemFree(name) void WINAPI name(LPVOID pv)
typedef FPL__FUNC_WIN32_CoTaskMemFree(fpl__win32_func_CoTaskMemFree);
#define FPL__FUNC_WIN32_PropVariantClear(name) HRESULT WINAPI name(PROPVARIANT *pvar)
typedef FPL__FUNC_WIN32_PropVariantClear(fpl__win32_func_PropVariantClear);

typedef struct fpl__Win32GdiApi {
	HMODULE gdiLibrary;
	fpl__win32_func_ChoosePixelFormat *ChoosePixelFormat;
	fpl__win32_func_SetPixelFormat *SetPixelFormat;
	fpl__win32_func_DescribePixelFormat *DescribePixelFormat;
	fpl__win32_func_GetDeviceCaps *GetDeviceCaps;
	fpl__win32_func_StretchDIBits *StretchDIBits;
	fpl__win32_func_DeleteObject *DeleteObject;
	fpl__win32_func_SwapBuffers *SwapBuffers;
} fpl__Win32GdiApi;

typedef struct fpl__Win32ShellApi {
	HMODULE shellLibrary;
	fpl__win32_func_CommandLineToArgvW *CommandLineToArgvW;
	fpl__win32_func_SHGetFolderPathA *ShGetFolderPathA;
	fpl__win32_func_SHGetFolderPathW *ShGetFolderPathW;
} fpl__Win32ShellApi;

typedef struct fpl__Win32UserApi {
	HMODULE userLibrary;
	fpl__win32_func_RegisterClassExA *RegisterClassExA;
	fpl__win32_func_RegisterClassExW *RegisterClassExW;
	fpl__win32_func_UnregisterClassA *UnregisterClassA;
	fpl__win32_func_UnregisterClassW *UnregisterClassW;
	fpl__win32_func_ShowWindow *ShowWindow;
	fpl__win32_func_DestroyWindow *DestroyWindow;
	fpl__win32_func_UpdateWindow *UpdateWindow;
	fpl__win32_func_TranslateMessage *TranslateMessage;
	fpl__win32_func_DispatchMessageA *DispatchMessageA;
	fpl__win32_func_DispatchMessageW *DispatchMessageW;
	fpl__win32_func_PeekMessageA *PeekMessageA;
	fpl__win32_func_PeekMessageW *PeekMessageW;
	fpl__win32_func_DefWindowProcA *DefWindowProcA;
	fpl__win32_func_DefWindowProcW *DefWindowProcW;
	fpl__win32_func_CreateWindowExA *CreateWindowExA;
	fpl__win32_func_CreateWindowExW *CreateWindowExW;
	fpl__win32_func_SetWindowPos *SetWindowPos;
	fpl__win32_func_GetWindowPlacement *GetWindowPlacement;
	fpl__win32_func_SetWindowPlacement *SetWindowPlacement;
	fpl__win32_func_GetClientRect *GetClientRect;
	fpl__win32_func_GetWindowRect *GetWindowRect;
	fpl__win32_func_AdjustWindowRect *AdjustWindowRect;
	fpl__win32_func_GetAsyncKeyState *GetAsyncKeyState;
	fpl__win32_func_MapVirtualKeyA *MapVirtualKeyA;
	fpl__win32_func_MapVirtualKeyW *MapVirtualKeyW;
	fpl__win32_func_SetCursor *SetCursor;
	fpl__win32_func_GetCursor *GetCursor;
	fpl__win32_func_LoadCursorA *LoadCursorA;
	fpl__win32_func_LoadCursorW *LoadCursorW;
	fpl__win32_func_LoadIconA *LoadIconA;
	fpl__win32_func_LoadIconW *LoadIconW;
	fpl__win32_func_SetWindowTextA *SetWindowTextA;
	fpl__win32_func_SetWindowTextW *SetWindowTextW;
	fpl__win32_func_SetWindowLongA *SetWindowLongA;
	fpl__win32_func_SetWindowLongW *SetWindowLongW;
	fpl__win32_func_GetWindowLongA *GetWindowLongA;
	fpl__win32_func_GetWindowLongW *GetWindowLongW;
#if defined(FPL_ARCH_X64)
	fpl__win32_func_SetWindowLongPtrA *SetWindowLongPtrA;
	fpl__win32_func_SetWindowLongPtrW *SetWindowLongPtrW;
	fpl__win32_func_GetWindowLongPtrA *GetWindowLongPtrA;
	fpl__win32_func_GetWindowLongPtrW *GetWindowLongPtrW;
#endif
	fpl__win32_func_ReleaseDC *ReleaseDC;
	fpl__win32_func_GetDC *GetDC;
	fpl__win32_func_ChangeDisplaySettingsA *ChangeDisplaySettingsA;
	fpl__win32_func_ChangeDisplaySettingsW *ChangeDisplaySettingsW;
	fpl__win32_func_EnumDisplaySettingsA *EnumDisplaySettingsA;
	fpl__win32_func_EnumDisplaySettingsW *EnumDisplaySettingsW;
	fpl__win32_func_OpenClipboard *OpenClipboard;
	fpl__win32_func_CloseClipboard *CloseClipboard;
	fpl__win32_func_EmptyClipboard *EmptyClipboard;
	fpl__win32_func_IsClipboardFormatAvailable *IsClipboardFormatAvailable;
	fpl__win32_func_SetClipboardData *SetClipboardData;
	fpl__win32_func_GetClipboardData *GetClipboardData;
	fpl__win32_func_GetDesktopWindow *GetDesktopWindow;
	fpl__win32_func_GetForegroundWindow *GetForegroundWindow;
	fpl__win32_func_IsZoomed *IsZoomed;
	fpl__win32_func_SendMessageA *SendMessageA;
	fpl__win32_func_SendMessageW *SendMessageW;
	fpl__win32_func_GetMonitorInfoA *GetMonitorInfoA;
	fpl__win32_func_GetMonitorInfoW *GetMonitorInfoW;
	fpl__win32_func_MonitorFromRect *MonitorFromRect;
	fpl__win32_func_MonitorFromWindow *MonitorFromWindow;
	fpl__win32_func_GetCursorPos *GetCursorPos;
	fpl__win32_func_WindowFromPoint *WindowFromPoint;
	fpl__win32_func_ClientToScreen *ClientToScreen;
	fpl__win32_func_PtInRect *PtInRect;
	fpl__win32_func_RegisterRawInputDevices *RegisterRawInputDevices;
	fpl__win32_func_ClipCursor *ClipCursor;
} fpl__Win32UserApi;

typedef struct fpl__Win32OleApi {
	HMODULE oleLibrary;
	fpl__win32_func_CoInitializeEx *CoInitializeEx;
	fpl__win32_func_CoUninitialize *CoUninitialize;
	fpl__win32_func_CoCreateInstance *CoCreateInstance;
	fpl__win32_func_CoTaskMemFree *CoTaskMemFree;
	fpl__win32_func_PropVariantClear *PropVariantClear;
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
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.CommandLineToArgvW, fpl__win32_func_CommandLineToArgvW, "CommandLineToArgvW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.ShGetFolderPathA, fpl__win32_func_SHGetFolderPathA, "SHGetFolderPathA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, shellLibraryName, wapi->shell.ShGetFolderPathW, fpl__win32_func_SHGetFolderPathW, "SHGetFolderPathW");
	}

	// User32
	{
		const char *userLibraryName = "user32.dll";
		HMODULE library = wapi->user.userLibrary = LoadLibraryA(userLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", userLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.RegisterClassExA, fpl__win32_func_RegisterClassExA, "RegisterClassExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.RegisterClassExW, fpl__win32_func_RegisterClassExW, "RegisterClassExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.UnregisterClassA, fpl__win32_func_UnregisterClassA, "UnregisterClassA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.UnregisterClassW, fpl__win32_func_UnregisterClassW, "UnregisterClassW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ShowWindow, fpl__win32_func_ShowWindow, "ShowWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.DestroyWindow, fpl__win32_func_DestroyWindow, "DestroyWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.UpdateWindow, fpl__win32_func_UpdateWindow, "UpdateWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.TranslateMessage, fpl__win32_func_TranslateMessage, "TranslateMessage");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.DispatchMessageA, fpl__win32_func_DispatchMessageA, "DispatchMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.DispatchMessageW, fpl__win32_func_DispatchMessageW, "DispatchMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.PeekMessageA, fpl__win32_func_PeekMessageA, "PeekMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.PeekMessageW, fpl__win32_func_PeekMessageW, "PeekMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.DefWindowProcA, fpl__win32_func_DefWindowProcA, "DefWindowProcA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.DefWindowProcW, fpl__win32_func_DefWindowProcW, "DefWindowProcW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.CreateWindowExA, fpl__win32_func_CreateWindowExA, "CreateWindowExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.CreateWindowExW, fpl__win32_func_CreateWindowExW, "CreateWindowExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowPos, fpl__win32_func_SetWindowPos, "SetWindowPos");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowPlacement, fpl__win32_func_GetWindowPlacement, "GetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowPlacement, fpl__win32_func_SetWindowPlacement, "SetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetClientRect, fpl__win32_func_GetClientRect, "GetClientRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowRect, fpl__win32_func_GetWindowRect, "GetWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.AdjustWindowRect, fpl__win32_func_AdjustWindowRect, "AdjustWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetAsyncKeyState, fpl__win32_func_GetAsyncKeyState, "GetAsyncKeyState");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.MapVirtualKeyA, fpl__win32_func_MapVirtualKeyA, "MapVirtualKeyA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.MapVirtualKeyW, fpl__win32_func_MapVirtualKeyW, "MapVirtualKeyW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetCursor, fpl__win32_func_SetCursor, "SetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetCursor, fpl__win32_func_GetCursor, "GetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.LoadCursorA, fpl__win32_func_LoadCursorA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.LoadCursorW, fpl__win32_func_LoadCursorW, "LoadCursorW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetCursorPos, fpl__win32_func_GetCursorPos, "GetCursorPos");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.WindowFromPoint, fpl__win32_func_WindowFromPoint, "WindowFromPoint");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.LoadIconA, fpl__win32_func_LoadIconA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.LoadIconW, fpl__win32_func_LoadIconW, "LoadIconW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowTextA, fpl__win32_func_SetWindowTextA, "SetWindowTextA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowTextW, fpl__win32_func_SetWindowTextW, "SetWindowTextW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowLongA, fpl__win32_func_SetWindowLongA, "SetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowLongW, fpl__win32_func_SetWindowLongW, "SetWindowLongW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowLongA, fpl__win32_func_GetWindowLongA, "GetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowLongW, fpl__win32_func_GetWindowLongW, "GetWindowLongW");

#		if defined(FPL_ARCH_X64)
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowLongPtrA, fpl__win32_func_SetWindowLongPtrA, "SetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetWindowLongPtrW, fpl__win32_func_SetWindowLongPtrW, "SetWindowLongPtrW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowLongPtrA, fpl__win32_func_GetWindowLongPtrA, "GetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetWindowLongPtrW, fpl__win32_func_GetWindowLongPtrW, "GetWindowLongPtrW");
#		endif

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ReleaseDC, fpl__win32_func_ReleaseDC, "ReleaseDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetDC, fpl__win32_func_GetDC, "GetDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ChangeDisplaySettingsA, fpl__win32_func_ChangeDisplaySettingsA, "ChangeDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ChangeDisplaySettingsW, fpl__win32_func_ChangeDisplaySettingsW, "ChangeDisplaySettingsW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.EnumDisplaySettingsA, fpl__win32_func_EnumDisplaySettingsA, "EnumDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.EnumDisplaySettingsW, fpl__win32_func_EnumDisplaySettingsW, "EnumDisplaySettingsW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.OpenClipboard, fpl__win32_func_OpenClipboard, "OpenClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.CloseClipboard, fpl__win32_func_CloseClipboard, "CloseClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.EmptyClipboard, fpl__win32_func_EmptyClipboard, "EmptyClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SetClipboardData, fpl__win32_func_SetClipboardData, "SetClipboardData");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetClipboardData, fpl__win32_func_GetClipboardData, "GetClipboardData");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetDesktopWindow, fpl__win32_func_GetDesktopWindow, "GetDesktopWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetForegroundWindow, fpl__win32_func_GetForegroundWindow, "GetForegroundWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.IsZoomed, fpl__win32_func_IsZoomed, "IsZoomed");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SendMessageA, fpl__win32_func_SendMessageA, "SendMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.SendMessageW, fpl__win32_func_SendMessageW, "SendMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetMonitorInfoA, fpl__win32_func_GetMonitorInfoA, "GetMonitorInfoA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.GetMonitorInfoW, fpl__win32_func_GetMonitorInfoW, "GetMonitorInfoW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.MonitorFromRect, fpl__win32_func_MonitorFromRect, "MonitorFromRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.MonitorFromWindow, fpl__win32_func_MonitorFromWindow, "MonitorFromWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ClientToScreen, fpl__win32_func_ClientToScreen, "ClientToScreen");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.PtInRect, fpl__win32_func_PtInRect, "PtInRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.RegisterRawInputDevices, fpl__win32_func_RegisterRawInputDevices, "RegisterRawInputDevices");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, userLibraryName, wapi->user.ClipCursor, fpl__win32_func_ClipCursor, "ClipCursor");
	}

	// GDI32
	{
		const char *gdiLibraryName = "gdi32.dll";
		HMODULE library = wapi->gdi.gdiLibrary = LoadLibraryA(gdiLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", gdiLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.ChoosePixelFormat, fpl__win32_func_ChoosePixelFormat, "ChoosePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.SetPixelFormat, fpl__win32_func_SetPixelFormat, "SetPixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.DescribePixelFormat, fpl__win32_func_DescribePixelFormat, "DescribePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.StretchDIBits, fpl__win32_func_StretchDIBits, "StretchDIBits");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.DeleteObject, fpl__win32_func_DeleteObject, "DeleteObject");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.SwapBuffers, fpl__win32_func_SwapBuffers, "SwapBuffers");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, gdiLibraryName, wapi->gdi.GetDeviceCaps, fpl__win32_func_GetDeviceCaps, "GetDeviceCaps");
	}

	// OLE32
	{
		const char *oleLibraryName = "ole32.dll";
		HMODULE library = wapi->ole.oleLibrary = LoadLibraryA(oleLibraryName);
		if (library == fpl_null) {
			fpl__PushError("Failed loading library '%s'", oleLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.CoInitializeEx, fpl__win32_func_CoInitializeEx, "CoInitializeEx");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.CoUninitialize, fpl__win32_func_CoUninitialize, "CoUninitialize");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.CoCreateInstance, fpl__win32_func_CoCreateInstance, "CoCreateInstance");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.CoTaskMemFree, fpl__win32_func_CoTaskMemFree, "CoTaskMemFree");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, oleLibraryName, wapi->ole.PropVariantClear, fpl__win32_func_PropVariantClear, "PropVariantClear");
	}

	return true;
}

// Unicode dependent function calls and types
#if !defined(UNICODE)
#	define FPL__WIN32_CLASSNAME "FPLWindowClassA"
#	define FPL__WIN32_UNNAMED_WINDOW "Unnamed FPL Ansi Window"
#	define fpl__win32_WNDCLASSEX WNDCLASSEXA
#	if defined(FPL_ARCH_X64)
#		define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongPtrA
#	else
#		define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongA
#	endif
#	define fpl__win32_SetWindowLong fpl__global__AppState->win32.winApi.user.SetWindowLongA
#	define fpl__win32_GetWindowLong fpl__global__AppState->win32.winApi.user.GetWindowLongA
#	define fpl__win32_PeekMessage fpl__global__AppState->win32.winApi.user.PeekMessageA
#	define fpl__win32_DispatchMessage fpl__global__AppState->win32.winApi.user.DispatchMessageA
#	define fpl__win32_DefWindowProc fpl__global__AppState->win32.winApi.user.DefWindowProcA
#	define fpl__win32_RegisterClassEx fpl__global__AppState->win32.winApi.user.RegisterClassExA
#	define fpl__win32_UnregisterClass fpl__global__AppState->win32.winApi.user.UnregisterClassA
#	define fpl__win32_CreateWindowEx fpl__global__AppState->win32.winApi.user.CreateWindowExA
#	define fpl__win32_LoadIcon fpl__global__AppState->win32.winApi.user.LoadIconA
#	define fpl__win32_LoadCursor fpl__global__AppState->win32.winApi.user.LoadCursorA
#	define fpl__win32_SendMessage fpl__global__AppState->win32.winApi.user.SendMessageA
#	define fpl__win32_GetMonitorInfo fpl__global__AppState->win32.winApi.user.GetMonitorInfoA
#else
#	define FPL__WIN32_CLASSNAME L"FPLWindowClassW"
#	define FPL__WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
#	define fpl__win32_WNDCLASSEX WNDCLASSEXW
#	if defined(FPL_ARCH_X64)
#		define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongPtrW
#	else
#		define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongW
#	endif
#	define fpl__win32_SetWindowLong fpl__global__AppState->win32.winApi.user.SetWindowLongW
#	define fpl__win32_GetWindowLong fpl__global__AppState->win32.winApi.user.GetWindowLongW
#	define fpl__win32_PeekMessage fpl__global__AppState->win32.winApi.user.PeekMessageW
#	define fpl__win32_DispatchMessage fpl__global__AppState->win32.winApi.user.DispatchMessageW
#	define fpl__win32_DefWindowProc fpl__global__AppState->win32.winApi.user.DefWindowProcW
#	define fpl__win32_RegisterClassEx fpl__global__AppState->win32.winApi.user.RegisterClassExW
#	define fpl__win32_UnregisterClass fpl__global__AppState->win32.winApi.user.UnregisterClassW
#	define fpl__win32_CreateWindowEx fpl__global__AppState->win32.winApi.user.CreateWindowExW
#	define fpl__win32_LoadIcon fpl__global__AppState->win32.winApi.user.LoadIconW
#	define fpl__win32_LoadCursor fpl__global__AppState->win32.winApi.user.LoadCursorW
#	define fpl__win32_SendMessage fpl__global__AppState->win32.winApi.user.SendMessageW
#	define fpl__win32_GetMonitorInfo fpl__global__AppState->win32.winApi.user.GetMonitorInfoW
#endif // UNICODE

typedef struct fpl__Win32XInputState {
	bool isConnected[XUSER_MAX_COUNT];
	LARGE_INTEGER lastDeviceSearchTime;
	fpl__Win32XInputApi xinputApi;
} fpl__Win32XInputState;

typedef struct fpl__Win32ConsoleState {
	bool isAllocated;
} fpl__Win32ConsoleState;

typedef struct fpl__Win32InitState {
	HINSTANCE appInstance;
	LARGE_INTEGER performanceFrequency;
} fpl__Win32InitState;

typedef struct fpl__Win32AppState {
	fpl__Win32XInputState xinput;
	fpl__Win32Api winApi;
	fpl__Win32ConsoleState console;
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
	bool isCursorActive;
	bool isFrameInteraction;
} fpl__Win32WindowState;
#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ############################################################################
//
// > TYPES_POSIX
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_POSIX)
#	include <sys/mman.h> // mmap, munmap
#	include <sys/types.h> // data types
#	include <sys/stat.h> // mkdir
#	include <sys/errno.h> // errno
#	include <sys/time.h> // gettimeofday
#	include <signal.h> // pthread_kill
#	include <time.h> // clock_gettime, nanosleep
#	include <dlfcn.h> // dlopen, dlclose
#	include <fcntl.h> // open
#	include <unistd.h> // read, write, close, access, rmdir

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

typedef struct fpl__PThreadApi {
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

fpl_internal void fpl__PThreadUnloadApi(fpl__PThreadApi *pthreadApi) {
	FPL_ASSERT(pthreadApi != fpl_null);
	if (pthreadApi->libHandle != fpl_null) {
		dlclose(pthreadApi->libHandle);
	}
	FPL_CLEAR_STRUCT(pthreadApi);
}

fpl_internal bool fpl__PThreadLoadApi(fpl__PThreadApi *pthreadApi) {
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
			do {
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
			} while (0);
			if (result) {
				break;
			}
		}
		fpl__PThreadUnloadApi(pthreadApi);
	}
	return(result);
}

typedef struct fpl__PosixInitState {
	//! Dummy field
	int dummy;
} fpl__PosixInitState;

typedef struct fpl__PosixAppState {
	fpl__PThreadApi pthreadApi;
} fpl__PosixAppState;
#endif // FPL_SUBPLATFORM_POSIX

// ############################################################################
//
// > TYPES_LINUX
//
// ############################################################################
#if defined(FPL_PLATFORM_LINUX)
typedef struct fpl__LinuxInitState {
	//! Dummy field
	int dummy;
} fpl__LinuxInitState;

typedef struct fpl__LinuxAppState {
	//! Dummy field
	int dummy;
} fpl__LinuxAppState;
#endif // FPL_PLATFORM_LINUX

// ############################################################################
//
// > TYPES_UNIX
//
// ############################################################################
#if defined(FPL_PLATFORM_UNIX)
typedef struct fpl__UnixInitState {
	//! Dummy field
	int dummy;
} fpl__UnixInitState;

typedef struct fpl__UnixAppState {
	//! Dummy field
	int dummy;
} fpl__UnixAppState;
#endif // FPL_PLATFORM_UNIX

// ############################################################################
//
// > TYPES_X11
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_X11)

#include <X11/keysym.h> // Keyboard symbols (XK_Escape, etc.)

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
#define FPL__FUNC_X11_X_INTERN_ATOM(name) Atom name(Display *display, char *atom_name, Bool only_if_exists)
typedef FPL__FUNC_X11_X_INTERN_ATOM(fpl__func_x11_XInternAtom);
#define FPL__FUNC_X11_X_SET_WM_PROTOCOLS(name) Status name(Display *display, Window w, Atom *protocols, int count)
typedef FPL__FUNC_X11_X_SET_WM_PROTOCOLS(fpl__func_x11_XSetWMProtocols);
#define FPL__FUNC_X11_X_PENDING(name) int name(Display *display)
typedef FPL__FUNC_X11_X_PENDING(fpl__func_x11_XPending);
#define FPL__FUNC_X11_X_SYNC(name) int name(Display *display, Bool discard)
typedef FPL__FUNC_X11_X_SYNC(fpl__func_x11_XSync);
#define FPL__FUNC_X11_X_NEXT_EVENT(name) int name(Display *display, XEvent *event_return)
typedef FPL__FUNC_X11_X_NEXT_EVENT(fpl__func_x11_XNextEvent);
#define FPL__FUNC_X11_X_PEEK_EVENT(name) int name(Display *display, XEvent *event_return)
typedef FPL__FUNC_X11_X_PEEK_EVENT(fpl__func_x11_XPeekEvent);
#define FPL__FUNC_X11_X_GET_WINDOW_ATTRIBUTES(name) Status name(Display *display, Window w, XWindowAttributes *window_attributes_return)
typedef FPL__FUNC_X11_X_GET_WINDOW_ATTRIBUTES(fpl__func_x11_XGetWindowAttributes);
#define FPL__FUNC_X11_X_RESIZE_WINDOW(name) int name(Display *display, Window w, unsigned int width, unsigned int height)
typedef FPL__FUNC_X11_X_RESIZE_WINDOW(fpl__func_x11_XResizeWindow);
#define FPL__FUNC_X11_X_MOVE_WINDOW(name) int name(Display *display, Window w, int x, int y)
typedef FPL__FUNC_X11_X_MOVE_WINDOW(fpl__func_x11_XMoveWindow);
#define FPL__FUNC_X11_X_GET_KEYBOARD_MAPPING(name) KeySym *name(Display *display, KeyCode first_keycode, int keycode_count, int *keysyms_per_keycode_return)
typedef FPL__FUNC_X11_X_GET_KEYBOARD_MAPPING(fpl__func_x11_XGetKeyboardMapping);

typedef struct fpl__X11Api {
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
	fpl__func_x11_XInternAtom *XInternAtom;
	fpl__func_x11_XSetWMProtocols *XSetWMProtocols;
	fpl__func_x11_XPending *XPending;
	fpl__func_x11_XSync *XSync;
	fpl__func_x11_XNextEvent *XNextEvent;
	fpl__func_x11_XPeekEvent *XPeekEvent;
	fpl__func_x11_XGetWindowAttributes *XGetWindowAttributes;
	fpl__func_x11_XResizeWindow *XResizeWindow;
	fpl__func_x11_XMoveWindow *XMoveWindow;
	fpl__func_x11_XGetKeyboardMapping *XGetKeyboardMapping;
} fpl__X11Api;

fpl_internal void fpl__UnloadX11Api(fpl__X11Api *x11Api) {
	FPL_ASSERT(x11Api != fpl_null);
	if (x11Api->libHandle != fpl_null) {
		dlclose(x11Api->libHandle);
	}
	FPL_CLEAR_STRUCT(x11Api);
}

fpl_internal bool fpl__LoadX11Api(fpl__X11Api *x11Api) {
	// @TODO(final): Correct order of libX11*.so files to prope
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
			do {
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
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XInternAtom, fpl__func_x11_XInternAtom, "XInternAtom");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XSetWMProtocols, fpl__func_x11_XSetWMProtocols, "XSetWMProtocols");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XPending, fpl__func_x11_XPending, "XPending");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XSync, fpl__func_x11_XSync, "XSync");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XNextEvent, fpl__func_x11_XNextEvent, "XNextEvent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XPeekEvent, fpl__func_x11_XPeekEvent, "XPeekEvent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XGetWindowAttributes, fpl__func_x11_XGetWindowAttributes, "XGetWindowAttributes");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XResizeWindow, fpl__func_x11_XResizeWindow, "XResizeWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XMoveWindow, fpl__func_x11_XMoveWindow, "XMoveWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, x11Api->XGetKeyboardMapping, fpl__func_x11_XGetKeyboardMapping, "XGetKeyboardMapping");
				result = true;
			} while (0);
			if (result) {
				break;
			}
		}
		fpl__UnloadX11Api(x11Api);
	}
	return(result);
}

typedef struct fpl__X11SubplatformState {
	fpl__X11Api api;
} fpl__X11SubplatformState;

typedef struct fpl__X11WindowState {
	Display* display;
	int screen;
	Window root;
	Colormap colorMap;
	Window window;
	Atom wmDeleteWindow;
} fpl__X11WindowState;

typedef struct fpl__X11PreWindowSetupResult {
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
typedef struct fpl__PlatformInitState {
#if defined(FPL_SUBPLATFORM_POSIX)
	fpl__PosixInitState posix;
#endif
	bool isInitialized;

	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32InitState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxInitState plinux;
#	elif defined(FPL_PLATFORM_UNIX)
		fpl__UnixInitState punix;
#	endif               
	};
} fpl__PlatformInitState;
fpl_globalvar fpl__PlatformInitState fpl__global__InitState = FPL_ZERO_INIT;

#if defined(FPL_ENABLE_WINDOW)
#define FPL__MAX_EVENT_COUNT 32768
typedef struct fpl__EventQueue {
	fplEvent events[FPL__MAX_EVENT_COUNT];
	volatile uint32_t pollIndex;
	volatile uint32_t pushCount;
} fpl__EventQueue;

typedef struct fpl__PlatformWindowState {
	fpl__EventQueue eventQueue;
	fplKey keyMap[256];
	bool isRunning;

#if defined(FPL_PLATFORM_WIN32)
	fpl__Win32WindowState win32;
#endif
#if defined(FPL_SUBPLATFORM_X11)
	fpl__X11WindowState x11;
#endif
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
	// Subplatforms
#if defined(FPL_SUBPLATFORM_POSIX)
	fpl__PosixAppState posix;
#endif
#if defined(FPL_SUBPLATFORM_X11)
	fpl__X11SubplatformState x11;
#endif

	// Window/video/audio
#if defined(FPL_ENABLE_WINDOW)
	fpl__PlatformWindowState window;
#endif
#if defined(FPL_ENABLE_VIDEO)
	fpl__PlatformVideoState video;
#endif
#if defined(FPL_ENABLE_AUDIO)
	fpl__PlatformAudioState audio;
#endif

	// Settings
	fplSettings initSettings;
	fplSettings currentSettings;
	fplInitFlags initFlags;

	// Platforms
	union {
#	if defined(FPL_PLATFORM_WIN32)
		fpl__Win32AppState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxAppState plinux;
#	elif defined(FPL_PLATFORM_UNIX)
		fpl__UnixAppState plinux;
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
	//! Dummy field
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

//
// Macros
//

// Clearing memory in chunks
#define FPL__MEMORY_SET(T, memory, size, shift, mask, value) \
	do { \
		size_t clearedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = (T *)(memory); \
			T *dataBlockEnd = (T *)(memory) + (size >> shift); \
			while (dataBlock != dataBlockEnd) { \
				*dataBlock++ = value; \
				clearedBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + clearedBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 != data8End) { \
			*data8++ = value; \
		} \
	} while (0);

#define FPL__MEMORY_COPY(T, source, sourceSize, dest, shift, mask) \
	do { \
		size_t copiedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			const T *sourceDataBlock = (const T *)(source); \
			const T *sourceDataBlockEnd = (const T *)(source) + (sourceSize >> shift); \
			T *destDataBlock = (T *)(dest); \
			while (sourceDataBlock != sourceDataBlockEnd) { \
				*destDataBlock++ = *sourceDataBlock++; \
				copiedBytes += sizeof(T); \
			} \
		} \
		const uint8_t *sourceData8 = (const uint8_t *)source + copiedBytes; \
		const uint8_t *sourceData8End = (const uint8_t *)source + sourceSize; \
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
fpl_internal void fpl__PushError_Formatted(const char *format, va_list argList) {
	fpl__ErrorState *state = &fpl__global__LastErrorState;
	FPL_ASSERT(format != fpl_null);
	char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH] = FPL_ZERO_INIT;
	fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	size_t messageLen = fplGetAnsiStringLength(buffer);
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
fpl_internal void fpl__PushError_Formatted(const char *format, va_list argList) {
	ErrorState *state = fpl__global__LastErrorState;
	FPL_ASSERT(format != fpl_null);
	char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH];
	fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	uint32_t messageLen = fplGetAnsiStringLength(buffer);
	state->count = 1;
	fplCopyAnsiStringLen(buffer, messageLen, state->errors[0], FPL__MAX_LAST_ERROR_STRING_LENGTH);
#if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
	fplConsoleFormatError("FPL Error[%s]: %s\h", FPL_PLATFORM_NAME, buffer);
#endif
}
#endif // FPL_ENABLE_MULTIPLE_ERRORSTATES

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

#if defined(FPL_ENABLE_WINDOW)
fpl_internal_inline fplKey fpl__GetMappedKey(const fpl__PlatformWindowState *windowState, const uint64_t keyCode) {
	fplKey result;
	if (keyCode < FPL_ARRAYCOUNT(windowState->keyMap))
		result = windowState->keyMap[keyCode];
	else
		result = fplKey_None;
	return(result);
}
#endif

//
// Common Strings
//
#if !defined(FPL__COMMON_STRINGS_DEFINED)
#define FPL__COMMON_STRINGS_DEFINED

fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen) {
	if ((a == fpl_null) || (b == fpl_null)) {
		return (a == b);
	}
	if (aLen != bLen) {
		return false;
	}
	FPL_ASSERT(aLen == bLen);
	bool result = true;
	for (size_t index = 0; index < aLen; ++index) {
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

fpl_common_api char *fplStringAppendLen(const char *appended, const size_t appendedLen, char *buffer, size_t maxBufferLen) {
	if (buffer == fpl_null) {
		fpl__ArgumentNullError("Buffer");
		return fpl_null;
	}
	if (maxBufferLen == 0) {
		fpl__ArgumentZeroError("Max buffer length");
		return fpl_null;
	}
	if (appendedLen == 0) {
		// Nothing to append
		return buffer;
	}
	size_t curBufferLen = fplGetAnsiStringLength(buffer);
	size_t requiredSize = curBufferLen + appendedLen + 1;
	if (requiredSize > maxBufferLen) {
		fpl__ArgumentSizeTooSmallError("Max buffer length", maxBufferLen, requiredSize);
		return fpl_null;
	}

	char *str = buffer + curBufferLen;
	size_t i = 0;
	while (i < appendedLen) {
		*str++ = appended[i++];
	}
	*str = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(buffer);
}

fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen) {
	size_t appendedLen = fplGetAnsiStringLength(appended);
	char *result = fplStringAppendLen(appended, appendedLen, buffer, maxBufferLen);
	return(result);
}

fpl_common_api size_t fplGetAnsiStringLength(const char *str) {
	uint32_t result = 0;
	if (str != fpl_null) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api size_t fplGetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str != fpl_null) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api char *fplCopyAnsiStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen) {
	if (source != fpl_null && dest != fpl_null) {
		size_t requiredLen = sourceLen + 1;
		if (maxDestLen < requiredLen) {
			fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredLen);
			return fpl_null;
		}
		char *result = dest;
		size_t index = 0;
		while (index++ < sourceLen) {
			*dest++ = *source++;
		}
		*dest = 0;
		return(result);
	} else {
		return(fpl_null);
	}
	// @TODO(final): Useless return, return the last written character instead
}

fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const size_t maxDestLen) {
	char *result = fpl_null;
	if (source != fpl_null) {
		size_t sourceLen = fplGetAnsiStringLength(source);
		result = fplCopyAnsiStringLen(source, sourceLen, dest, maxDestLen);
	}
	return(result);
}

fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const size_t sourceLen, wchar_t *dest, const size_t maxDestLen) {
	if (source != fpl_null && dest != fpl_null) {
		size_t requiredLen = sourceLen + 1;
		if (maxDestLen < requiredLen) {
			fpl__ArgumentSizeTooSmallError("Max dest len", maxDestLen, requiredLen);
			return fpl_null;
		}
		wchar_t *result = dest;
		size_t index = 0;
		while (index++ < sourceLen) {
			*dest++ = *source++;
		}
		*dest = 0;
		return(result);
	} else {
		return(fpl_null);
	}
	// @TODO(final): Useless return, return the last written character instead
}

fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const size_t maxDestLen) {
	wchar_t *result = fpl_null;
	if (source != fpl_null) {
		size_t sourceLen = fplGetWideStringLength(source);
		result = fplCopyWideStringLen(source, sourceLen, dest, maxDestLen);
	}
	return(result);
}

fpl_common_api char *fplFormatAnsiStringArgs(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, va_list argList) {
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
	if (argList == fpl_null) {
		fpl__ArgumentNullError("Arg list");
		return fpl_null;
	}
	// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
	ansiDestBuffer[0] = 0;

	int charCount = 0;
#	if defined(FPL_NO_CRT)
#		if defined(FPL_USERFUNC_vsnprintf)
	charCount = FPL_USERFUNC_vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
#		else
	charCount = 0;
#		endif
#	else
	charCount = vsnprintf(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
#	endif

	if (charCount < 0) {
		fpl__PushError("Format parameter are '%s' are invalid!", format);
		return fpl_null;
	}
	size_t requiredMaxAnsiDestBufferLen = charCount + 1;
	if (maxAnsiDestBufferLen < requiredMaxAnsiDestBufferLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestBufferLen, requiredMaxAnsiDestBufferLen);
		return fpl_null;
	}
	ansiDestBuffer[charCount] = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(ansiDestBuffer);
}

fpl_common_api char *fplFormatAnsiString(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, ...) {
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
	char *result = fplFormatAnsiStringArgs(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
	va_end(argList);
	return(result);
}
#endif // FPL__COMMON_STRINGS_DEFINED

//
// Common Console
//
#if !defined(FPL__COMMON_CONSOLE_DEFINED)
#define FPL__COMMON_CONSOLE_DEFINED

fpl_common_api void fplConsoleFormatOut(const char *format, ...) {
	char buffer[1024 * 10];
	va_list argList;
	va_start(argList, format);
	char *str = fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	va_end(argList);
	if (str != fpl_null) {
		fplConsoleOut(str);
	}
}

fpl_common_api void fplConsoleFormatError(const char *format, ...) {
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	char *str = fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	va_end(argList);
	if (str != fpl_null) {
		fplConsoleError(str);
	}
}
#endif // FPL__COMMON_CONSOLE_DEFINED

//
// Common Memory
//
#if !defined(FPL__COMMON_MEMORY_DEFINED)
#define FPL__COMMON_MEMORY_DEFINED

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

fpl_common_api void fplMemorySet(void *mem, const uint8_t value, const size_t size) {
	if (mem == fpl_null) {
		fpl__ArgumentNullError("Memory");
		return;
	}
	if (size == 0) {
		fpl__ArgumentSizeTooSmallError("Size", size, 1);
		return;
	}
	if (size % 8 == 0) {
		FPL__MEMORY_SET(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64, value);
	} else if (size % 4 == 0) {
		FPL__MEMORY_SET(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32, value);
	} else if (size % 2 == 0) {
		FPL__MEMORY_SET(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16, value);
	} else {
		FPL__MEMORY_SET(uint8_t, mem, size, 0, 0, value);
	}
}

fpl_common_api void fplMemoryClear(void *mem, const size_t size) {
	if (mem == fpl_null) {
		fpl__ArgumentNullError("Memory");
		return;
	}
	if (size == 0) {
		fpl__ArgumentSizeTooSmallError("Size", size, 1);
		return;
	}
	if (size % 8 == 0) {
		FPL__MEMORY_SET(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64, 0);
	} else if (size % 4 == 0) {
		FPL__MEMORY_SET(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32, 0);
	} else if (size % 2 == 0) {
		FPL__MEMORY_SET(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16, 0);
	} else {
		FPL__MEMORY_SET(uint8_t, mem, size, 0, 0, 0);
	}
}

fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem) {
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
		FPL__MEMORY_COPY(uint64_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if (sourceSize % 4 == 0) {
		FPL__MEMORY_COPY(uint32_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if (sourceSize % 2 == 0) {
		FPL__MEMORY_COPY(uint16_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else {
		FPL__MEMORY_COPY(uint8_t, sourceMem, sourceSize, targetMem, 0, 0);
	}
}
#endif // FPL__COMMON_MEMORY_DEFINED

//
// Common Atomics
//
#if !defined(FPL__COMMON_ATOMICS_DEFINED)
#define FPL__COMMON_ATOMICS_DEFINED

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
#endif // FPL__COMMON_ATOMICS_DEFINED

//
// Common Paths
//
#if !defined(FPL__COMMON_PATHS_DEFINED)
#define FPL__COMMON_PATHS_DEFINED

fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen) {
	if (sourcePath == fpl_null) {
		fpl__ArgumentNullError("Source path");
		return fpl_null;
	}
	size_t sourceLen = fplGetAnsiStringLength(sourcePath);
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

fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen) {
	if (filePath == fpl_null) {
		fpl__ArgumentNullError("File path");
		return fpl_null;
	}
	if (newFileExtension == fpl_null) {
		fpl__ArgumentNullError("New file extension");
		return fpl_null;
	}
	size_t pathLen = fplGetAnsiStringLength(filePath);
	if (pathLen == 0) {
		fpl__ArgumentZeroError("Path len");
		return fpl_null;
	}
	size_t extLen = fplGetAnsiStringLength(newFileExtension);

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

		size_t copyLen;
		if (lastExtSeparatorPtr != fpl_null) {
			copyLen = (size_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
		} else {
			copyLen = pathLen;
		}

		// Copy parts
		fplCopyAnsiStringLen(filePath, copyLen, destPath, maxDestLen);
		char *destExtPtr = destPath + copyLen;
		fplCopyAnsiStringLen(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);

		// @TODO(final): Useless return, return the last written character instead
		result = destPath;
	}
	return(result);
}

fpl_common_api char *fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...) {
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

	size_t curDestPosition = 0;
	char *currentDestPtr = destPath;
	va_list vargs;
	va_start(vargs, pathCount);
	for (size_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		char *path = va_arg(vargs, char *);
		size_t pathLen = fplGetAnsiStringLength(path);
		bool requireSeparator = pathIndex < (pathCount - 1);
		size_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
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

	// @TODO(final): Useless return, return the last written character instead
	return destPath;
}
#endif // FPL__COMMON_PATHS_DEFINED

#if defined(FPL_ENABLE_WINDOW)

#if !defined(FPL__COMMON_WINDOW_DEFINED)
#define FPL__COMMON_WINDOW_DEFINED

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
#endif // FPL__COMMON_WINDOW_DEFINED

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

fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video) {
	FPL_CLEAR_STRUCT(video);
	video->isVSync = false;
	video->isAutoSize = true;
	// @NOTE(final): Auto detect video driver
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	video->driver = fplVideoDriverType_OpenGL;
	video->graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#elif defined(FPL_ENABLE_VIDEO_SOFTWARE)
	video->driver = fplVideoDriverType_Software;
#else
	video->driver = fplVideoDriverType_None;
#endif
}

fpl_common_api void fplSetDefaultAudioSettings(fplAudioSettings *audio) {
	FPL_CLEAR_STRUCT(audio);
	audio->bufferSizeInMilliSeconds = 25;
	audio->preferExclusiveMode = false;
	audio->deviceFormat.channels = 2;
	audio->deviceFormat.sampleRate = 48000;
	audio->deviceFormat.type = fplAudioFormatType_S16;

	audio->driver = fplAudioDriverType_None;
#	if defined(FPL_PLATFORM_WIN32) && defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
	audio->driver = fplAudioDriverType_DirectSound;
#	endif
#	if defined(FPL_PLATFORM_LINUX) && defined(FPL_ENABLE_AUDIO_ALSA)
	audio->driver = fplAudioDriverType_Alsa;
#	endif
}

fpl_common_api void fplSetDefaultWindowSettings(fplWindowSettings *window) {
	FPL_CLEAR_STRUCT(window);
	window->windowTitle[0] = 0;
	window->windowWidth = 800;
	window->windowHeight = 600;
	window->fullscreenWidth = 0;
	window->fullscreenHeight = 0;
	window->isFullscreen = false;
	window->isResizable = true;
	window->isDecorated = true;
	window->isFloating = false;
}

fpl_common_api void fplSetDefaultInputSettings(fplInputSettings *input) {
	FPL_CLEAR_STRUCT(input);
	input->controllerDetectionFrequency = 100;
}

fpl_common_api void fplSetDefaultSettings(fplSettings *settings) {
	FPL_CLEAR_STRUCT(settings);
	fplSetDefaultWindowSettings(&settings->window);
	fplSetDefaultVideoSettings(&settings->video);
	fplSetDefaultAudioSettings(&settings->audio);
	fplSetDefaultInputSettings(&settings->input);
}

fpl_common_api fplSettings fplMakeDefaultSettings() {
	fplSettings result;
	fplSetDefaultSettings(&result);
	return(result);
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

fpl_internal_inline DWORD fpl__Win32GetWindowStyle(const fplWindowSettings *settings) {
	DWORD result = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (settings->isFullscreen) {
		result |= WS_POPUP;
	} else {
		result |= WS_SYSMENU | WS_MINIMIZEBOX;

		if (settings->isDecorated) {
			result |= WS_CAPTION;
			if (settings->isResizable) {
				result |= WS_MAXIMIZEBOX | WS_THICKFRAME;
			}
		} else {
			result |= WS_POPUP;
		}
	}
	return(result);
}

fpl_internal_inline DWORD fpl__Win32GetWindowExStyle(const fplWindowSettings *settings) {
	DWORD result = WS_EX_APPWINDOW;
	if (settings->isFullscreen || settings->isFloating) {
		result |= WS_EX_TOPMOST;
	}
	return(result);
}

fpl_internal_inline void fpl__Win32UpdateWindowStyles(const fplWindowSettings *settings, const fpl__Win32WindowState *windowState) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	DWORD style = fpl__Win32GetWindowStyle(settings);
	DWORD exStyle = fpl__Win32GetWindowExStyle(settings);
	fpl__win32_SetWindowLong(windowState->windowHandle, GWL_STYLE, style);
	fpl__win32_SetWindowLong(windowState->windowHandle, GWL_EXSTYLE, exStyle);
}

fpl_internal bool fpl__Win32LeaveFullscreen() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *platState = fpl__global__AppState;
	const fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	const fpl__Win32WindowState *win32Window = &platState->window.win32;
	const fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;

	HWND windowHandle = win32Window->windowHandle;

	FPL_ASSERT(fullscreenInfo->style > 0 && fullscreenInfo->exStyle > 0);
	fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, fullscreenInfo->style);
	fpl__win32_SetWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo->exStyle);
	wapi->user.SetWindowPlacement(windowHandle, &fullscreenInfo->placement);
	wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	// @NOTE(final): No need to handle minimized here because it is unlikly that you switch to fullscreen mode when the app is minimized
	if (fullscreenInfo->isMaximized) {
		fpl__win32_SendMessage(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}

	bool result;
	if (fullscreenInfo->wasResolutionChanged) {
		result = (wapi->user.ChangeDisplaySettingsA(fpl_null, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
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
	fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, fullscreenInfo->style & ~(WS_CAPTION | WS_THICKFRAME));
	fpl__win32_SetWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo->exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

	MONITORINFO monitor = FPL_ZERO_INIT;
	monitor.cbSize = sizeof(monitor);
	fpl__win32_GetMonitorInfo(wapi->user.MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST), &monitor);

	bool result;
	if (fullscreenWidth > 0 && fullscreenHeight > 0) {
		DWORD useFullscreenWidth = fullscreenWidth;
		DWORD useFullscreenHeight = fullscreenHeight;

		DWORD useRefreshRate = refreshRate;
		if (!useRefreshRate) {
			useRefreshRate = wapi->gdi.GetDeviceCaps(deviceContext, VREFRESH);
		}

		DWORD useColourBits = colorBits;
		if (!useColourBits) {
			useColourBits = wapi->gdi.GetDeviceCaps(deviceContext, BITSPIXEL);
		}

		RECT windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = windowRect.left + useFullscreenWidth;
		windowRect.bottom = windowRect.left + useFullscreenHeight;

		WINDOWPLACEMENT placement = FPL_ZERO_INIT;
		placement.length = sizeof(placement);
		placement.rcNormalPosition = windowRect;
		placement.showCmd = SW_SHOW;
		wapi->user.SetWindowPlacement(windowHandle, &placement);
		wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		DEVMODEA fullscreenSettings = FPL_ZERO_INIT;
		wapi->user.EnumDisplaySettingsA(fpl_null, 0, &fullscreenSettings);
		fullscreenSettings.dmPelsWidth = useFullscreenWidth;
		fullscreenSettings.dmPelsHeight = useFullscreenHeight;
		fullscreenSettings.dmBitsPerPel = useColourBits;
		fullscreenSettings.dmDisplayFrequency = useRefreshRate;
		fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		result = (wapi->user.ChangeDisplaySettingsA(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
		fullscreenInfo->wasResolutionChanged = true;
	} else {
		RECT windowRect = monitor.rcMonitor;

		WINDOWPLACEMENT placement = FPL_ZERO_INIT;
		placement.length = sizeof(placement);
		placement.rcNormalPosition = windowRect;
		placement.showCmd = SW_SHOW;
		wapi->user.SetWindowPlacement(windowHandle, &placement);
		wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

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

fpl_internal_inline void fpl__Win32PushKeyboardEvent(const fpl__PlatformWindowState *windowState, const fplKeyboardEventType keyboardEventType, const uint64_t keyCode, const fplKeyboardModifierFlags modifiers, const bool isDown) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = fpl__GetMappedKey(windowState, keyCode);
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl__PushEvent(&newEvent);
}

fpl_internal_inline bool fpl__Win32IsKeyDown(const fpl__Win32Api *wapi, const uint64_t keyCode) {
	bool result = (wapi->user.GetAsyncKeyState((int)keyCode) & 0x8000) > 0;
	return(result);
}

fpl_internal_inline bool fpl__Win32IsCursorInWindow(const fpl__Win32Api *wapi, const fpl__Win32WindowState *win32Window) {
	POINT pos;
	if (!wapi->user.GetCursorPos(&pos)) {
		return false;
	}
	// Not this window?
	if (wapi->user.WindowFromPoint(pos) != win32Window->windowHandle) {
		return false;
	}

	// Cursor in client rect?
	RECT area;
	wapi->user.GetClientRect(win32Window->windowHandle, &area);
	wapi->user.ClientToScreen(win32Window->windowHandle, (POINT *)&area.left);
	wapi->user.ClientToScreen(win32Window->windowHandle, (POINT *)&area.right);

	bool result = wapi->user.PtInRect(&area, pos) == TRUE;
	return(result);
}

fpl_internal void fpl__Win32LoadCursor(const fpl__Win32Api *wapi, const fpl__Win32WindowState *window) {
	if (window->isCursorActive) {
		wapi->user.SetCursor(fpl__win32_LoadCursor(fpl_null, IDC_ARROW));
	} else {
		wapi->user.SetCursor(fpl_null);
	}
}

fpl_internal void fpl__Win32UpdateClipRect(const fpl__Win32Api *wapi, const fpl__Win32WindowState *window) {
	if (window != fpl_null) {
		RECT clipRect;
		wapi->user.GetClientRect(window->windowHandle, &clipRect);
		wapi->user.ClientToScreen(window->windowHandle, (POINT *)&clipRect.left);
		wapi->user.ClientToScreen(window->windowHandle, (POINT *)&clipRect.right);
		wapi->user.ClipCursor(&clipRect);
	} else {
		wapi->user.ClipCursor(fpl_null);
	}
}

fpl_internal void fpl__Win32SetCursorState(const fpl__Win32Api *wapi, fpl__Win32WindowState *window, const bool state) {
	if (!state) {
		fpl__Win32UpdateClipRect(wapi, window);
		const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window->windowHandle };
		if (!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			fpl__PushError("Failed register raw input mouse device for window handle '%p'", window->windowHandle);
		}
	} else {
		fpl__Win32UpdateClipRect(wapi, fpl_null);
		const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, fpl_null };
		if (!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			fpl__PushError("Failed to unregister raw input mouse device");
		}
	}
	if (fpl__Win32IsCursorInWindow(wapi, window)) {
		fpl__Win32LoadCursor(wapi, window);
	}
}

fpl_internal_inline void fpl__Win32ShowCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
	fpl__Win32SetCursorState(wapi, window, false);
}
fpl_internal_inline void fpl__Win32HideCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
	fpl__Win32SetCursorState(wapi, window, true);
}

LRESULT CALLBACK fpl__Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;

	fpl__Win32AppState *win32State = &appState->win32;
	fpl__Win32WindowState *win32Window = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32State->winApi;

	if (!win32Window->windowHandle) {
		return fpl__win32_DefWindowProc(hwnd, msg, wParam, lParam);
	}

	// @TODO(final): Handle WM_DISPLAYCHANGE 

	LRESULT result = 0;
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			appState->window.isRunning = false;
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

			// @TODO(final): Is it possible to detect the key state from the MSG?
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
			fpl__Win32PushKeyboardEvent(&appState->window, keyEventType, keyCode, modifiers, isDown);

			if (wasDown != isDown) {
				if (isDown) {
					if (keyCode == VK_F4 && altKeyWasDown) {
						appState->window.isRunning = false;
					}
				}
			}
		} break;

		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_UNICHAR:
		{
			if ((msg == WM_UNICHAR) && (wParam == UNICODE_NOCHAR)) {
				// @NOTE(final): WM_UNICHAR was sent by a third-party input method. Do not add any chars here!
				return TRUE;
			}

			uint64_t keyCode = wParam;
			fpl__Win32PushKeyboardEvent(&appState->window, fplKeyboardEventType_CharInput, keyCode, fplKeyboardModifierFlags_None, 0);

			return 0;
		} break;

		case WM_ACTIVATE:
		{
		} break;

		case WM_MOUSEACTIVATE:
		{
			// @NOTE(final): User starts to click/move the window frame
			if (HIWORD(lParam) == WM_LBUTTONDOWN) {
				if (LOWORD(lParam) == HTCLOSE || LOWORD(lParam) == HTMINBUTTON || LOWORD(lParam) == HTMAXBUTTON) {
					win32Window->isFrameInteraction = true;
				}
			}
		} break;

		case WM_CAPTURECHANGED:
		{
			// User is done with interaction with the the window frame
			if (lParam == 0 && win32Window->isFrameInteraction) {
				// Hide cursor when needed
				if (!win32Window->isCursorActive) {
					fpl__Win32HideCursor(wapi, win32Window);
				}
				win32Window->isFrameInteraction = false;
			}
		} break;

		case WM_SETFOCUS:
		{
			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_GotFocus;
			fpl__PushEvent(&newEvent);

			// @NOTE(final): Do not disable the cursor while the user interacts with the window frame
			if (win32Window->isFrameInteraction) {
				break;
			}

			// Hide cursor when needed
			if (!win32Window->isCursorActive) {
				fpl__Win32HideCursor(wapi, win32Window);
			}

			return 0;
		} break;

		case WM_KILLFOCUS:
		{
			// Restore cursor when needed
			if (!win32Window->isCursorActive) {
				fpl__Win32ShowCursor(wapi, win32Window);
			}

			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_LostFocus;
			fpl__PushEvent(&newEvent);

			return 0;
		} break;

		case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
		{
			// Restore cursor when needed
			if (!win32Window->isCursorActive) {
				fpl__Win32ShowCursor(wapi, win32Window);
			}
		} break;

		case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
		{
			// Hide cursor when needed
			if (!win32Window->isCursorActive) {
				fpl__Win32HideCursor(wapi, win32Window);
			}
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
			// @NOTE(final): Load cursor only when we are in the window client area
			if (LOWORD(lParam) == HTCLIENT) {
				fpl__Win32LoadCursor(wapi, win32Window);
				return TRUE;
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
	result = fpl__win32_DefWindowProc(hwnd, msg, wParam, lParam);
	return (result);
}

fpl_internal bool fpl__Win32InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *platAppState, fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	FPL_ASSERT(appState != fpl_null);
	const fpl__Win32Api *wapi = &appState->winApi;
	const fplWindowSettings *initWindowSettings = &initSettings->window;

	// Register window class
	fpl__win32_WNDCLASSEX windowClass = FPL_ZERO_INIT;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.hInstance = GetModuleHandleA(fpl_null);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hCursor = fpl__win32_LoadCursor(windowClass.hInstance, IDC_ARROW);
	windowClass.hIcon = fpl__win32_LoadIcon(windowClass.hInstance, IDI_APPLICATION);
	windowClass.hIconSm = fpl__win32_LoadIcon(windowClass.hInstance, IDI_APPLICATION);
	windowClass.lpszClassName = FPL__WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl__Win32MessageProc;
	windowClass.style |= CS_OWNDC;

#if _UNICODE
	fplCopyWideString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#else
	fplCopyAnsiString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#endif

	if (fpl__win32_RegisterClassEx(&windowClass) == 0) {
		fpl__PushError("Failed registering window class '%s'", windowState->windowClass);
		return false;
	}

	// Set window title
#if _UNICODE
	wchar_t windowTitleBuffer[1024];
	if (fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplAnsiStringToWideString(initWindowSettings->windowTitle, fplGetAnsiStringLength(initWindowSettings->windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const wchar_t *defaultTitle = FPL__WIN32_UNNAMED_WINDOW;
		fplCopyWideString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	wchar_t *windowTitle = windowTitleBuffer;
	fplWideStringToAnsiString(windowTitle, fplGetWideStringLength(windowTitle), currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#else
	char windowTitleBuffer[1024];
	if (fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplCopyAnsiString(initWindowSettings->windowTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const char *defaultTitle = FPL__WIN32_UNNAMED_WINDOW;
		fplCopyAnsiString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	char *windowTitle = windowTitleBuffer;
	fplCopyAnsiString(windowTitle, currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#endif

	// Create window
	DWORD style = fpl__Win32GetWindowStyle(&initSettings->window);
	DWORD exStyle = fpl__Win32GetWindowExStyle(&initSettings->window);
	if (initSettings->window.isResizable) {
		currentWindowSettings->isResizable = true;
	} else {
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
		wapi->user.AdjustWindowRect(&windowRect, style, false);
		windowWidth = windowRect.right - windowRect.left;
		windowHeight = windowRect.bottom - windowRect.top;
	} else {
		// @NOTE(final): Operating system decide how big the window should be.
		windowWidth = CW_USEDEFAULT;
		windowHeight = CW_USEDEFAULT;
	}

	// Create window
	windowState->windowHandle = fpl__win32_CreateWindowEx(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, fpl_null, fpl_null, windowClass.hInstance, fpl_null);
	if (windowState->windowHandle == fpl_null) {
		fpl__PushError("Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", windowState->windowClass, windowX, windowY, windowWidth, windowHeight);
		return false;
	}

	// Get actual window size and store results
	currentWindowSettings->windowWidth = windowWidth;
	currentWindowSettings->windowHeight = windowHeight;
	RECT clientRect;
	if (wapi->user.GetClientRect(windowState->windowHandle, &clientRect)) {
		currentWindowSettings->windowWidth = clientRect.right - clientRect.left;
		currentWindowSettings->windowHeight = clientRect.bottom - clientRect.top;
	}

	// Get device context so we can swap the back and front buffer
	windowState->deviceContext = wapi->user.GetDC(windowState->windowHandle);
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
	wapi->user.ShowWindow(windowState->windowHandle, SW_SHOW);
	wapi->user.UpdateWindow(windowState->windowHandle);

	// Cursor is visible at start
	windowState->defaultCursor = windowClass.hCursor;
	windowState->isCursorActive = true;
	platAppState->window.isRunning = true;

	return true;
}

fpl_internal void fpl__Win32ReleaseWindow(const fpl__Win32InitState *initState, const fpl__Win32AppState *appState, fpl__Win32WindowState *windowState) {
	const fpl__Win32Api *wapi = &appState->winApi;

	if (windowState->deviceContext != fpl_null) {
		wapi->user.ReleaseDC(windowState->windowHandle, windowState->deviceContext);
		windowState->deviceContext = fpl_null;
	}

	if (windowState->windowHandle != fpl_null) {
		wapi->user.DestroyWindow(windowState->windowHandle);
		windowState->windowHandle = fpl_null;
		fpl__win32_UnregisterClass(windowState->windowClass, initState->appInstance);
	}
}

#endif // FPL_ENABLE_WINDOW

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
			size_t executableFilePathLen = 0;
			for (int i = 0; i < executableFilePathArgumentCount; ++i) {
				if (i > 0) {
					// Include whitespace
					executableFilePathLen++;
				}
				size_t sourceLen = fplGetWideStringLength(executableFilePathArgs[i]);
				int destLen = WideCharToMultiByte(CP_UTF8, 0, executableFilePathArgs[i], (int)sourceLen, fpl_null, 0, fpl_null, fpl_null);
				executableFilePathLen += destLen;
			}

			// @NOTE(final): Do not parse the arguments when there are no actual arguments, otherwise we will get back the executable arguments again.
			int actualArgumentCount = 0;
			wchar_t **actualArgs = fpl_null;
			size_t actualArgumentsLen = 0;
			if (cmdLine != fpl_null && fplGetWideStringLength(cmdLine) > 0) {
				actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
				for (int i = 0; i < actualArgumentCount; ++i) {
					size_t sourceLen = fplGetWideStringLength(actualArgs[i]);
					int destLen = WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], (int)sourceLen, fpl_null, 0, fpl_null, fpl_null);
					actualArgumentsLen += destLen;
				}
			}

			// Calculate argument 
			args.count = 1 + actualArgumentCount;
			size_t totalStringLen = executableFilePathLen + actualArgumentsLen + args.count;
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
					size_t sourceArgLen = fplGetWideStringLength(sourceArg);
					int destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, fpl_null, 0, fpl_null, fpl_null);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, destArg, destArgLen, fpl_null, fpl_null);
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
					size_t sourceArgLen = fplGetWideStringLength(sourceArg);
					int destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, fpl_null, 0, fpl_null, fpl_null);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, destArg, destArgLen, fpl_null, fpl_null);
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
		size_t ansiSourceLen = fplGetAnsiStringLength(cmdLine);
		int wideDestLen = MultiByteToWideChar(CP_ACP, 0, cmdLine, (int)ansiSourceLen, fpl_null, 0);
		wchar_t *wideCmdLine = (wchar_t *)fplMemoryAllocate(sizeof(wchar_t) * (wideDestLen + 1));
		MultiByteToWideChar(CP_ACP, 0, cmdLine, (int)ansiSourceLen, wideCmdLine, wideDestLen);
		wideCmdLine[wideDestLen] = 0;
		result = fpl__Win32ParseWideArguments(wideCmdLine);
		fplMemoryFree(wideCmdLine);
	} else {
		wchar_t tmp[1] = { 0 };
		result = fpl__Win32ParseWideArguments(tmp);
	}
	return(result);
}

fpl_internal bool fpl__Win32ThreadWaitForMultiple(fplThreadHandle *threads[], const size_t count, const bool waitForAll, const fplTimeoutValue timeout) {
	if (threads == fpl_null) {
		fpl__ArgumentNullError("Threads");
		return false;
	}
	if (count > FPL__MAX_THREAD_COUNT) {
		fpl__ArgumentSizeTooBigError("Count", count, FPL__MAX_THREAD_COUNT);
		return false;
	}
	HANDLE threadHandles[FPL__MAX_THREAD_COUNT];
	for (size_t index = 0; index < count; ++index) {
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
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	DWORD code = WaitForMultipleObjects((DWORD)count, threadHandles, waitForAll ? TRUE : FALSE, t);
	bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
	return(result);
}

fpl_internal bool fpl__Win32SignalWaitForMultiple(fplSignalHandle *signals[], const size_t count, const bool waitForAll, const fplTimeoutValue timeout) {
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
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	DWORD code = WaitForMultipleObjects((DWORD)count, signalHandles, waitForAll ? TRUE : FALSE, t);
	bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
	return(result);
}

fpl_internal void fpl__Win32ReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32InitState *win32InitState = &initState->win32;

	if (win32AppState->console.isAllocated) {
		FreeConsole();
		win32AppState->console.isAllocated = false;
	}

	fpl__Win32UnloadXInputApi(&win32AppState->xinput.xinputApi);
	fpl__Win32UnloadApi(&win32AppState->winApi);
}

#if defined(FPL_ENABLE_WINDOW)
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
#endif

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

	// Init console
	if (!(initFlags & fplInitFlags_Window)) {
		HANDLE tmpOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (tmpOut == fpl_null) {
			// @TODO(final): This case seems to never be executed even on non-CRT -> When do i need to call AllocConsole()?
			AllocConsole();
			win32AppState->console.isAllocated = true;
		}
	}

	// Init keymap
#	if defined(FPL_ENABLE_WINDOW)
	FPL_CLEAR_STRUCT(appState->window.keyMap);
	for (int i = 0; i < 256; ++i) {
		appState->window.keyMap[i] = fpl__Win32MapVirtualKey(i);
	}
#	endif

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
	int64_t result = _InterlockedExchange64((volatile LONG64 *)target, value);
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
	int64_t result = _InterlockedExchangeAdd64((volatile LONG64 *)value, addend);
	return(result);
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
	int64_t result = _InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
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
	int64_t result = _InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
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
	_InterlockedExchange64((volatile LONG64 *)dest, value);
}

//
// Win32 OS
//
#define FPL__FUNC_KERNEL32_GetVersion(name) DWORD name()
typedef FPL__FUNC_KERNEL32_GetVersion(fpl__func_kernel32_GetVersion);
#define FPL__FUNC_KERNEL32_GetVersionExA(name) BOOL WINAPI name(LPOSVERSIONINFOA lpVersionInfo)
typedef FPL__FUNC_KERNEL32_GetVersionExA(fpl__func_kernel32_GetVersionEx);

fpl_internal const char *fpl__Win32GetVersionName(DWORD major, DWORD minor) {
	const char *result;
	if (major == 5 && minor == 0) {
		result = "Windows 2000";
	} else if (major == 5 && minor == 1) {
		result = "Windows XP";
	} else if (major == 5 && minor == 2) {
		result = "Windows XP";
	} else if (major == 6 && minor == 0) {
		result = "Windows Vista";
	} else if (major == 6 && minor == 1) {
		result = "Windows 7";
	} else if (major == 6 && minor == 2) {
		result = "Windows 8";
	} else if (major == 6 && minor == 3) {
		result = "Windows 8.1";
	} else if (major == 10) {
		result = "Windows 10";
	} else {
		result = "Windows";
	}
	return(result);
}

fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos) {
	if (outInfos == fpl_null) {
		fpl__ArgumentNullError("Out infos");
		return false;
	}

	// @NOTE(final): GetVersion() and GetVersionExA() is deprecated as of windows 8.1 O_o
	HMODULE kernelLib = LoadLibraryA("kernel32.dll");
	if (kernelLib == fpl_null) {
		fpl__PushError("Kernel32 library could not be loaded!");
		return false;
	}
	fpl__func_kernel32_GetVersion *getVersionProc = (fpl__func_kernel32_GetVersion *)GetProcAddress(kernelLib, "GetVersion");
	fpl__func_kernel32_GetVersionEx *getVersionExProc = (fpl__func_kernel32_GetVersionEx *)GetProcAddress(kernelLib, "GetVersionExA");
	FreeLibrary(kernelLib);

	FPL_CLEAR_STRUCT(outInfos);

	bool result = false;
	DWORD dwVersion = 0;
	if (getVersionExProc != fpl_null) {
		OSVERSIONINFOA info = FPL_ZERO_INIT;
		info.dwOSVersionInfoSize = sizeof(info);
		if (getVersionExProc(&info) == TRUE) {
			FPL_ASSERT(info.dwMajorVersion <= UINT16_MAX);
			outInfos->systemVersion.major = (uint16_t)info.dwMajorVersion;
			FPL_ASSERT(info.dwMinorVersion <= UINT16_MAX);
			outInfos->systemVersion.minor = (uint16_t)info.dwMinorVersion;
			FPL_ASSERT(info.dwBuildNumber <= UINT16_MAX);
			outInfos->systemVersion.build = (uint16_t)info.dwBuildNumber;

			const char *versionName = fpl__Win32GetVersionName(info.dwMajorVersion, info.dwMinorVersion);
			fplCopyAnsiString(versionName, outInfos->systemName, FPL_ARRAYCOUNT(outInfos->systemName));

			result = true;
		}
	} else if (getVersionProc != fpl_null) {
		dwVersion = getVersionProc();

		DWORD major = (DWORD)(LOBYTE(LOWORD(dwVersion)));
		DWORD minor = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		DWORD build = 0;
		if (dwVersion < 0x80000000) {
			build = (DWORD)((DWORD)(HIWORD(dwVersion)));
		}

		FPL_ASSERT(major <= UINT16_MAX);
		outInfos->systemVersion.major = (uint16_t)major;
		FPL_ASSERT(minor <= UINT16_MAX);
		outInfos->systemVersion.minor = (uint16_t)minor;
		FPL_ASSERT(build <= UINT16_MAX);
		outInfos->systemVersion.build = (uint16_t)build;

		const char *versionName = fpl__Win32GetVersionName(major, minor);
		fplCopyAnsiString(versionName, outInfos->systemName, FPL_ARRAYCOUNT(outInfos->systemName));

		result = dwVersion > 0;
	}

	return(result);
}

#define FPL__FUNC_ADV32_GetUserNameA(name) BOOL WINAPI name(LPSTR lpBuffer, LPDWORD pcbBuffer)
typedef FPL__FUNC_ADV32_GetUserNameA(fpl__func_adv32_GetUserNameA);

fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, size_t maxNameBufferLen) {
	if (nameBuffer == fpl_null) {
		fpl__ArgumentNullError("Name buffer");
		return false;
	}
	if (maxNameBufferLen == 0) {
		fpl__ArgumentZeroError("Max name buffer len");
		return false;
	}

	const char *libName = "advapi32.dll";
	HMODULE adv32Lib = LoadLibraryA(libName);
	if (adv32Lib == fpl_null) {
		fpl__PushError("Failed loading library '%s'", libName);
		return false;
	}
	fpl__func_adv32_GetUserNameA *getUserNameProc = (fpl__func_adv32_GetUserNameA *)GetProcAddress(adv32Lib, "GetUserNameA");

	bool result = false;
	if (getUserNameProc != fpl_null) {
		DWORD size = (DWORD)maxNameBufferLen;
		if (getUserNameProc(nameBuffer, &size) == TRUE) {
			result = true;
		}
	}

	if (adv32Lib != fpl_null) {
		FreeLibrary(adv32Lib);
	}
	return(result);
}

//
// Win32 Hardware
//
fpl_platform_api size_t fplGetProcessorCoreCount() {
	SYSTEM_INFO sysInfo = FPL_ZERO_INIT;
	GetSystemInfo(&sysInfo);
	// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
	size_t result = sysInfo.dwNumberOfProcessors;
	return(result);
}

#define FPL__WIN32_PROCESSOR_ARCHITECTURE_ARM64 12

fpl_platform_api fplArchType fplGetRunningArchitecture() {
	fplArchType result;
	SYSTEM_INFO sysInfo = FPL_ZERO_INIT;
	GetSystemInfo(&sysInfo);
	switch (sysInfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			result = fplArchType_x86_64;
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			result = fplArchType_x64;
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			result = fplArchType_Arm32;
			break;
		case FPL__WIN32_PROCESSOR_ARCHITECTURE_ARM64:
			result = fplArchType_Arm64;
			break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			result = fplArchType_Unknown;
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
		default:
			result = fplArchType_x86;
			break;
	}
	return(result);
}

fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos) {
	if (outInfos == fpl_null) {
		fpl__ArgumentNullError("Out infos");
		return false;
	}

	MEMORYSTATUSEX statex = FPL_ZERO_INIT;
	statex.dwLength = sizeof(statex);
	ULONGLONG totalMemorySize;
	bool result = false;
	if (GetPhysicallyInstalledSystemMemory(&totalMemorySize) && GlobalMemoryStatusEx(&statex)) {
		FPL_CLEAR_STRUCT(outInfos);
		// @NOTE(final): Requires _allmul when CRT is disabled
		outInfos->totalPhysicalSize = totalMemorySize * 1024ull;
		outInfos->availablePhysicalSize = statex.ullTotalPhys;
		outInfos->usedPhysicalSize = outInfos->availablePhysicalSize - statex.ullAvailPhys;
		outInfos->totalVirtualSize = statex.ullTotalVirtual;
		outInfos->usedVirtualSize = outInfos->totalVirtualSize - statex.ullAvailVirtual;
		outInfos->totalPageSize = statex.ullTotalPageFile;
		outInfos->usedPageSize = outInfos->totalPageSize - statex.ullAvailPageFile;
		result = true;
	}
	return(result);
}

fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen) {
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
	size_t sourceLen = fplGetAnsiStringLength(cpuBrandBuffer);
	fplCopyAnsiStringLen(cpuBrandBuffer, sourceLen, destBuffer, maxDestBufferLen);

#	undef CPU_BRAND_BUFFER_SIZE

	// @TODO(final): Useless return, return the last written character instead
	return(destBuffer);
}

//
// Win32 Threading
//
fpl_internal DWORD WINAPI fpl__Win32ThreadProc(void *data) {
	fplThreadHandle *thread = (fplThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);
	if (thread->runFunc != fpl_null) {
		thread->runFunc(thread, thread->data);
	}
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	CloseHandle(handle);
	thread->internalHandle.win32ThreadHandle = fpl_null;
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	ExitThread(0);
	return(0);
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

fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		fpl__ArgumentNullError("Thread");
		return false;
	}
	if (thread->isValid && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
		HANDLE handle = thread->internalHandle.win32ThreadHandle;
		TerminateThread(handle, 0);
		CloseHandle(handle);
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
		FPL_CLEAR_STRUCT(thread);
		return true;
	} else {
		return false;
	}
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout) {
	if (thread == fpl_null) {
		fpl__ArgumentNullError("Thread");
		return false;
	}
	if (thread->internalHandle.win32ThreadHandle == fpl_null) {
		fpl__PushError("Win32 thread handle are not allowed to be null");
		return false;
	}
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = (WaitForSingleObject(handle, t) == WAIT_OBJECT_0);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, true, timeout);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, false, timeout);
	return(result);
}

fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (mutex->isValid) {
		fpl__PushError("Mutex '%p' is already initialized!", mutex);
		return false;
	}
	FPL_CLEAR_STRUCT(mutex);
	InitializeCriticalSection(&mutex->internalHandle.win32CriticalSection);
	mutex->isValid = true;
	return true;
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

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
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

fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (!mutex->isValid) {
		fpl__PushError("Mutex parameter must be valid");
		return false;
	}
	bool result = TryEnterCriticalSection(&mutex->internalHandle.win32CriticalSection) == TRUE;
	return(result);
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

fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->isValid) {
		fpl__PushError("Signal '%p' is already initialized!", signal);
		return false;
	}
	FPL_CLEAR_STRUCT(signal);

	HANDLE handle = CreateEventA(fpl_null, FALSE, (initialValue == fplSignalValue_Set) ? TRUE : FALSE, fpl_null);
	if (handle == fpl_null) {
		fpl__PushError("Failed creating signal (Win32 event): %d", GetLastError());
		return false;
	}

	signal->isValid = true;
	signal->internalHandle.win32EventHandle = handle;
	return(true);
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

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		fpl__PushError("Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = (WaitForSingleObject(handle, t) == WAIT_OBJECT_0);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__Win32SignalWaitForMultiple((fplSignalHandle **)signals, count, true, timeout);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__Win32SignalWaitForMultiple((fplSignalHandle **)signals, count, false, timeout);
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

fpl_platform_api bool fplSignalReset(fplSignalHandle *signal) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		fpl__PushError("Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = ResetEvent(handle) == TRUE;
	return(result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	FPL_CLEAR_STRUCT(condition);
	InitializeConditionVariable(&condition->internalHandle.win32Condition);
	condition->isValid = true;
	return true;
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return;
	}
	FPL_CLEAR_STRUCT(condition);
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	if (!mutex->isValid) {
		fpl__PushError("Mutex is not valid!");
		return false;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = SleepConditionVariableCS(&condition->internalHandle.win32Condition, &mutex->internalHandle.win32CriticalSection, t) != 0;
	return(result);
}

fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	WakeConditionVariable(&condition->internalHandle.win32Condition);
	return true;
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	WakeAllConditionVariable(&condition->internalHandle.win32Condition);
	return true;
}

//
// Win32 Console
//
fpl_platform_api void fplConsoleOut(const char *text) {
	DWORD charsToWrite = (DWORD)fplGetAnsiStringLength(text);
	DWORD writtenChars = 0;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(handle, text, charsToWrite, &writtenChars, fpl_null);
}

fpl_platform_api void fplConsoleError(const char *text) {
	DWORD charsToWrite = (DWORD)fplGetAnsiStringLength(text);
	DWORD writtenChars = 0;
	HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
	WriteFile(handle, text, charsToWrite, &writtenChars, fpl_null);
}

fpl_platform_api char fplConsoleWaitForCharInput() {
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	DWORD savedMode;
	GetConsoleMode(handle, &savedMode);
	SetConsoleMode(handle, ENABLE_PROCESSED_INPUT);
	char result = 0;
	if (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0) {
		DWORD charsRead = 0;
		char inputBuffer[2] = FPL_ZERO_INIT;
		if (ReadFile(handle, inputBuffer, 1, &charsRead, fpl_null) != 0) {
			result = inputBuffer[0];
		}
	}
	SetConsoleMode(handle, savedMode);
	return (result);
}

//
// Win32 Memory
//
fpl_platform_api void *fplMemoryAllocate(const size_t size) {
	if (size == 0) {
		fpl__ArgumentZeroError("Size");
		return fpl_null;
	}
	void *result = VirtualAlloc(fpl_null, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
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
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
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
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
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
	wapi->shell.ShGetFolderPathW(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	fplWideStringToAnsiString(homePath, fplGetWideStringLength(homePath), destPath, maxDestLen);
	return(destPath);
}
#else
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
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
	wapi->shell.ShGetFolderPathA(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	fplCopyAnsiStringLen(homePath, fplGetAnsiStringLength(homePath), destPath, maxDestLen);
	return(destPath);
}
#endif // UNICODE

//
// Win32 Timings
//
fpl_platform_api double fplGetTimeInSecondsHP() {
	const fpl__Win32InitState *initState = &fpl__global__InitState.win32;
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double result = time.QuadPart / (double)initState->performanceFrequency.QuadPart;
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInSecondsLP() {
	uint64_t result = (uint64_t)GetTickCount() / 1000;
	return(result);
}

fpl_platform_api double fplGetTimeInSeconds() {
	double result = fplGetTimeInSecondsHP();
	return(result);
}

fpl_platform_api double fplGetTimeInMillisecondsHP() {
	const fpl__Win32InitState *initState = &fpl__global__InitState.win32;
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	double result = (double)(time.QuadPart / initState->performanceFrequency.QuadPart) * 1000.0;
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInMillisecondsLP() {
	uint64_t result = GetTickCount();
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInMilliseconds() {
	uint64_t result = fplGetTimeInMillisecondsLP();
	return(result);
}

//
// Win32 Strings
//
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const size_t maxWideSourceLen, char *ansiDest, const size_t maxAnsiDestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (ansiDest == fpl_null) {
		fpl__ArgumentNullError("Ansi dest");
		return fpl_null;
	}
	int requiredLen = WideCharToMultiByte(CP_ACP, 0, wideSource, (int)maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	size_t minRequiredLen = requiredLen + 1;
	if (maxAnsiDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestLen, minRequiredLen);
		return fpl_null;
	}
	WideCharToMultiByte(CP_ACP, 0, wideSource, (int)maxWideSourceLen, ansiDest, (int)maxAnsiDestLen, fpl_null, fpl_null);
	ansiDest[requiredLen] = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(ansiDest);
}
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (utf8Dest == fpl_null) {
		fpl__ArgumentNullError("UTF8 dest");
		return fpl_null;
	}
	int requiredLen = WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	size_t minRequiredLen = requiredLen + 1;
	if (maxUtf8DestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max utf8 dest len", maxUtf8DestLen, minRequiredLen);
		return fpl_null;
	}
	WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)maxWideSourceLen, utf8Dest, (int)maxUtf8DestLen, fpl_null, fpl_null);
	utf8Dest[requiredLen] = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(utf8Dest);
}
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	if (ansiSource == fpl_null) {
		fpl__ArgumentNullError("Ansi source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	int requiredLen = MultiByteToWideChar(CP_ACP, 0, ansiSource, (int)ansiSourceLen, fpl_null, 0);
	size_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	MultiByteToWideChar(CP_ACP, 0, ansiSource, (int)ansiSourceLen, wideDest, (int)maxWideDestLen);
	wideDest[requiredLen] = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(wideDest);
}
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	if (utf8Source == fpl_null) {
		fpl__ArgumentNullError("UTF8 source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	int requiredLen = MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, fpl_null, 0);
	size_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, wideDest, (int)maxWideDestLen);
	wideDest[requiredLen] = 0;

	// @TODO(final): Useless return, return the last written character instead
	return(wideDest);
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
fpl_platform_api bool fplGetWindowArea(fplWindowSize *outSize) {
	if (outSize == fpl_null) {
		fpl__ArgumentNullError("Out size");
		return false;
	}
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	RECT windowRect;
	if (wapi->user.GetClientRect(windowState->windowHandle, &windowRect)) {
		outSize->width = windowRect.right - windowRect.left;
		outSize->height = windowRect.bottom - windowRect.top;
		result = true;
	}
	return(result);
}

fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	RECT clientRect, windowRect;
	if (wapi->user.GetClientRect(windowState->windowHandle, &clientRect) &&
		wapi->user.GetWindowRect(windowState->windowHandle, &windowRect)) {
		int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		int newWidth = width + borderWidth;
		int newHeight = height + borderHeight;
		wapi->user.SetWindowPos(windowState->windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

fpl_platform_api bool fplIsWindowResizable() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isResizable;
	return(result);
}

fpl_platform_api void fplSetWindowResizeable(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if (!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isResizable = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowDecorated() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isDecorated;
	return(result);
}

fpl_platform_api void fplSetWindowDecorated(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if (!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isDecorated = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowFloating() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFloating;
	return(result);
}

fpl_platform_api void fplSetWindowFloating(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if (!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isFloating = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowFullscreen() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFullscreen;
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
		fullscreenState->isMaximized = !!wapi->user.IsZoomed(windowHandle);
		if (fullscreenState->isMaximized) {
			fpl__win32_SendMessage(windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		fullscreenState->style = fpl__win32_GetWindowLong(windowHandle, GWL_STYLE);
		fullscreenState->exStyle = fpl__win32_GetWindowLong(windowHandle, GWL_EXSTYLE);
		wapi->user.GetWindowPlacement(windowHandle, &fullscreenState->placement);
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

fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos) {
	if (outPos == fpl_null) {
		fpl__ArgumentNullError("Outpos");
		return false;
	}

	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;

	bool result = false;
	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	if (wapi->user.GetWindowPlacement(windowState->windowHandle, &placement) == TRUE) {
		switch (placement.showCmd) {
			case SW_MAXIMIZE:
			{
				outPos->left = placement.ptMaxPosition.x;
				outPos->top = placement.ptMaxPosition.y;
			} break;
			case SW_MINIMIZE:
			{
				outPos->left = placement.ptMinPosition.x;
				outPos->top = placement.ptMinPosition.y;
			} break;
			default:
			{
				outPos->left = placement.rcNormalPosition.left;
				outPos->top = placement.rcNormalPosition.top;
			} break;
		}
		result = true;
	}
	return(result);
}

fpl_platform_api void fplSetWindowAnsiTitle(const char *ansiTitle) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HWND handle = windowState->windowHandle;
	wapi->user.SetWindowTextA(handle, ansiTitle);
}

fpl_platform_api void fplSetWindowWideTitle(const wchar_t *wideTitle) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HWND handle = windowState->windowHandle;
	wapi->user.SetWindowTextW(handle, wideTitle);
}

fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;

	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	RECT windowRect;
	if (wapi->user.GetWindowPlacement(windowState->windowHandle, &placement) &&
		wapi->user.GetWindowRect(windowState->windowHandle, &windowRect)) {
		switch (placement.showCmd) {
			case SW_NORMAL:
			case SW_SHOW:
			{
				placement.rcNormalPosition.left = left;
				placement.rcNormalPosition.top = top;
				placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
				placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
				wapi->user.SetWindowPlacement(windowState->windowHandle, &placement);
			} break;
		}
	}
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	windowState->isCursorActive = value;
}

fpl_platform_api bool fplPushEvent() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	bool result = false;
	MSG msg;
	BOOL R = fpl__win32_PeekMessage(&msg, fpl_null, 0, 0, PM_REMOVE);
	if (R == TRUE) {
		wapi->user.TranslateMessage(&msg);
		fpl__win32_DispatchMessage(&msg);
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
		while (fpl__win32_PeekMessage(&msg, fpl_null, 0, 0, PM_REMOVE) != 0) {
			wapi->user.TranslateMessage(&msg);
			fpl__win32_DispatchMessage(&msg);
		}
		result = appState->window.isRunning;
	}

	return(result);
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	bool result = fpl__global__AppState->window.isRunning;
	return(result);
}

fpl_platform_api bool fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		if (wapi->user.IsClipboardFormatAvailable(CF_TEXT)) {
			HGLOBAL dataHandle = wapi->user.GetClipboardData(CF_TEXT);
			if (dataHandle != fpl_null) {
				const char *stringValue = (const char *)GlobalLock(dataHandle);
				fplCopyAnsiString(stringValue, dest, maxDestLen);
				GlobalUnlock(dataHandle);
				result = true;
			}
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

fpl_platform_api bool fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		if (wapi->user.IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			HGLOBAL dataHandle = wapi->user.GetClipboardData(CF_UNICODETEXT);
			if (dataHandle != fpl_null) {
				const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
				fplCopyWideString(stringValue, dest, maxDestLen);
				GlobalUnlock(dataHandle);
				result = true;
			}
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

fpl_platform_api bool fplSetClipboardAnsiText(const char *ansiSource) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		const size_t ansiLen = fplGetAnsiStringLength(ansiSource);
		const size_t ansiBufferLen = ansiLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
		if (handle != fpl_null) {
			char *target = (char*)GlobalLock(handle);
			fplCopyAnsiStringLen(ansiSource, ansiLen, target, ansiBufferLen);
			GlobalUnlock(handle);
			wapi->user.EmptyClipboard();
			wapi->user.SetClipboardData(CF_TEXT, handle);
			result = true;
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

fpl_platform_api bool fplSetClipboardWideText(const wchar_t *wideSource) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		const size_t wideLen = fplGetWideStringLength(wideSource);
		const size_t wideBufferLen = wideLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
		if (handle != fpl_null) {
			wchar_t *wideTarget = (wchar_t*)GlobalLock(handle);
			fplCopyWideStringLen(wideSource, wideLen, wideTarget, wideBufferLen);
			GlobalUnlock(handle);
			wapi->user.EmptyClipboard();
			wapi->user.SetClipboardData(CF_UNICODETEXT, handle);
			result = true;
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

#if !defined(FPL_NO_ENTRYPOINT)

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

#endif // FPL_NO_ENTRYPOINT

#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ############################################################################
//
// > POSIX_SUBPLATFORM (Linux, Unix)
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_POSIX)
fpl_internal void fpl__PosixReleaseSubplatform(fpl__PosixAppState *appState) {
	fpl__PThreadUnloadApi(&appState->pthreadApi);
}

fpl_internal bool fpl__PosixInitSubplatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PosixInitState *initState, fpl__PosixAppState *appState) {
	if (!fpl__PThreadLoadApi(&appState->pthreadApi)) {
		fpl__PushError("Failed initializing PThread API");
		return false;
	}
	return true;
}

void *fpl__PosixThreadProc(void *data) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PThreadApi *pthreadApi = &fpl__global__AppState->posix.pthreadApi;
	fplThreadHandle *thread = (fplThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);
	if (thread->runFunc != fpl_null) {
		thread->runFunc(thread, thread->data);
	}
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	pthreadApi->pthread_exit(data);
	return 0;
}

fpl_internal bool fpl__PosixMutexLock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int lockRes;
	do {
		lockRes = pthreadApi->pthread_mutex_lock(handle);
	} while (lockRes == EAGAIN);
	bool result = (lockRes == 0);
	return(result);
}

fpl_internal bool fpl__PosixMutexTryLock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int lockRes;
	do {
		lockRes = pthreadApi->pthread_mutex_trylock(handle);
	} while (lockRes == EAGAIN);
	bool result = (lockRes == 0);
	return(result);
}

fpl_internal bool fpl__PosixMutexUnlock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int unlockRes;
	do {
		unlockRes = pthreadApi->pthread_mutex_unlock(handle);
	} while (unlockRes == EAGAIN);
	bool result = (unlockRes == 0);
	return(result);
}

fpl_internal int fpl__PosixMutexCreate(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	*handle = PTHREAD_MUTEX_INITIALIZER;
	int mutexRes;
	do {
		mutexRes = pthreadApi->pthread_mutex_init(handle, fpl_null);
	} while (mutexRes == EAGAIN);
	return(mutexRes);
}

fpl_internal_inline void fpl__InitWaitTimeSpec(const uint32_t milliseconds, timespec *outSpec) {
	time_t secs = milliseconds / 1000;
	uint64_t nanoSecs = (milliseconds - (secs * 1000)) * 1000000;
	if (nanoSecs >= 1000000000) {
		time_t addonSecs = (time_t)(nanoSecs / 1000000000);
		nanoSecs -= (addonSecs * 1000000000);
		secs += addonSecs;
	}
	clock_gettime(CLOCK_REALTIME, outSpec);
	outSpec->tv_sec += secs;
	outSpec->tv_nsec += nanoSecs;
}

fpl_internal bool fpl__PosixThreadWaitForMultiple(fplThreadHandle *threads[], const uint32_t minCount, const uint32_t maxCount, const fplTimeoutValue timeout) {
	if (threads == fpl_null) {
		fpl__ArgumentNullError("Threads");
		return false;
	}
	if (maxCount > FPL__MAX_THREAD_COUNT) {
		fpl__ArgumentSizeTooBigError("Max count", maxCount, FPL__MAX_THREAD_COUNT);
		return false;
	}
	for (uint32_t index = 0; index < maxCount; ++index) {
		fplThreadHandle *thread = threads[index];
		if (thread == fpl_null) {
			fpl__PushError("Thread for index '%d' are not allowed to be null", index);
			return false;
		}
		if (!thread->isValid) {
			fpl__PushError("Thread for index '%d' is not valid", index);
			return false;
		}
	}

	bool isRunning[FPL__MAX_THREAD_COUNT];
	for (uint32_t index = 0; index < maxCount; ++index) {
		isRunning[index] = true;
	}

	uint32_t completeCount = 0;
	uint64_t startTime = fplGetTimeInMilliseconds();
	bool result = false;
	while (completeCount < minCount) {
		for (uint32_t index = 0; index < maxCount; ++index) {
			fplThreadHandle *thread = threads[index];
			if (isRunning[index]) {
				fplThreadState state = fplGetThreadState(thread);
				if (state == fplThreadState_Stopped) {
					isRunning[index] = false;
					++completeCount;
					if (completeCount >= minCount) {
						result = true;
						break;
					}
				}
			}
			fplThreadSleep(10);
		}
		if ((timeout != FPL_TIMEOUT_INFINITE) && (fplGetTimeInMilliseconds() - startTime) >= timeout) {
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
fpl_platform_api void fplAtomicReadFence() {
	// @TODO(final): Wrong to ensure a full memory fence here!
	__sync_synchronize();
}
fpl_platform_api void fplAtomicWriteFence() {
	// @TODO(final): Wrong to ensure a full memory fence here!
	__sync_synchronize();
}
fpl_platform_api void fplAtomicReadWriteFence() {
	__sync_synchronize();
}

fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
	uint32_t result = __sync_lock_test_and_set(target, value);
	__sync_synchronize();
	return(result);
}
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value) {
	int32_t result = __sync_lock_test_and_set(target, value);
	__sync_synchronize();
	return(result);
}
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	uint64_t result = __sync_lock_test_and_set(target, value);
	__sync_synchronize();
	return(result);
}
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value) {
	int64_t result = __sync_lock_test_and_set(target, value);
	__sync_synchronize();
	return(result);
}

fpl_platform_api uint32_t fplAtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicAddS32(volatile int32_t *value, const int32_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicAddS64(volatile int64_t *value, const int64_t addend) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}

fpl_platform_api uint32_t fplAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int32_t fplAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api uint64_t fplAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int64_t fplAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}

fpl_platform_api bool fplIsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}

fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source) {
	uint32_t result = __sync_fetch_and_or(source, 0);
	return(result);
}
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source) {
	uint64_t result = __sync_fetch_and_or(source, 0);
	return(result);
}
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source) {
	int32_t result = __sync_fetch_and_or(source, 0);
	return(result);
}
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source) {
	int64_t result = __sync_fetch_and_or(source, 0);
	return(result);
}

fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
	__sync_lock_test_and_set(dest, value);
	__sync_synchronize();
}
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
	__sync_lock_test_and_set(dest, value);
	__sync_synchronize();
}
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value) {
	__sync_lock_test_and_set(dest, value);
	__sync_synchronize();
}
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value) {
	__sync_lock_test_and_set(dest, value);
	__sync_synchronize();
}
#else
#	error "This POSIX compiler/platform is not supported!"
#endif

//
// POSIX Timings
//
fpl_platform_api double fplGetTimeInSecondsHP() {
	// @TODO(final): Do we need to take the performance frequency into account?
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	double result = (double)t.tv_sec + ((double)t.tv_nsec * 1e-9);
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInSecondsLP() {
	uint64_t result = (uint64_t)time(fpl_null);
	return(result);
}

fpl_platform_api double fplGetTimeInSeconds() {
	struct timeval  tv;
	gettimeofday(&tv, fpl_null);
	double result = (double)tv.tv_sec + ((double)tv.tv_usec * 1e-6);
	return(result);
}

fpl_platform_api double fplGetTimeInMillisecondsHP() {
	timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	double result = ((double)t.tv_sec + ((double)t.tv_nsec * 1e-9)) * 1000.0;
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInMillisecondsLP() {
	uint64_t result = (uint64_t)time(fpl_null) * 1000;
	return(result);
}

fpl_platform_api uint64_t fplGetTimeInMilliseconds() {
	struct timeval  tv;
	gettimeofday(&tv, fpl_null);
	uint64_t result = tv.tv_sec * 1000 + ((uint64_t)tv.tv_usec / 1000);
	return(result);
}

//
// POSIX Threading
//
fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		fpl__ArgumentNullError("Thread");
		return false;
	}
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	if (thread->isValid && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		pthread_t threadHandle = thread->internalHandle.posixThread;
		if (pthreadApi->pthread_kill(threadHandle, 0) == 0) {
			pthreadApi->pthread_join(threadHandle, fpl_null);
		}
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
		FPL_CLEAR_STRUCT(thread);
		return true;
	} else {
		return false;
	}
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return fpl_null;
	}
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if (thread != fpl_null) {
		thread->currentState = fplThreadState_Stopped;
		thread->data = data;
		thread->runFunc = runFunc;
		thread->isValid = false;
		thread->isStopping = false;

		// Create thread
		// @TODO(final): Better thread id (pthread_t)
		thread->currentState = fplThreadState_Starting;
		fplMemoryCopy(&thread->internalHandle.posixThread, FPL_MIN(sizeof(thread->id), sizeof(thread->internalHandle.posixThread)), &thread->id);
		int threadRes;
		do {
			threadRes = pthreadApi->pthread_create(&thread->internalHandle.posixThread, fpl_null, fpl__PosixThreadProc, (void *)thread);
		} while (threadRes == EAGAIN);
		if (threadRes != 0) {
			fpl__PushError("Failed creating pthread, error code: %d", threadRes);
		}
		if (threadRes == 0) {
			thread->isValid = true;
			result = thread;
		} else {
			FPL_CLEAR_STRUCT(thread);
		}
	} else {
		fpl__PushError("All %d threads are in use, you cannot create until you free one", FPL__MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout) {
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	bool result = false;
	if (thread != fpl_null && thread->isValid) {
		// Wait until it shuts down
		pthread_t threadHandle = thread->internalHandle.posixThread;

		// @TODO(final): POSIX Use timeout in fplThreadWaitForOne

		int joinRes = pthreadApi->pthread_join(threadHandle, fpl_null);
		result = (joinRes == 0);
	}
	return (result);
}

fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__PosixThreadWaitForMultiple(threads, count, count, timeout);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__PosixThreadWaitForMultiple(threads, 1, count, timeout);
	return(result);
}

fpl_platform_api void fplThreadSleep(const uint32_t milliseconds) {
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

fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	if (mutex->isValid) {
		fpl__PushError("Mutex '%p' is already initialized!", mutex);
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	FPL_CLEAR_STRUCT(mutex);
	int mutexRes = fpl__PosixMutexCreate(pthreadApi, &mutex->internalHandle.posixMutex);
	if (mutexRes != 0) {
		fpl__PushError("Failed creating POSIX condition!");
		return false;
	}
	mutex->isValid = true;
	return(true);
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return;
	}
	if (mutex != fpl_null) {
		if (mutex->isValid) {
			pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
			pthreadApi->pthread_mutex_destroy(handle);
		}
		FPL_CLEAR_STRUCT(mutex);
	}
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	bool result = false;
	if (mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexLock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	bool result = false;
	if (mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexTryLock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex) {
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Mutex");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	bool result = false;
	if (mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexUnlock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	FPL_CLEAR_STRUCT(condition);

	pthread_cond_t *handle = &condition->internalHandle.posixCondition;
	*handle = PTHREAD_COND_INITIALIZER;
	int condRes;
	do {
		condRes = pthreadApi->pthread_cond_init(handle, fpl_null);
	} while (condRes == EAGAIN);
	if (condRes == 0) {
		condition->isValid = true;
	}
	return(condition->isValid);
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return;
	}
	if (condition->isValid) {
		pthread_cond_t *handle = &condition->internalHandle.posixCondition;
		pthreadApi->pthread_cond_destroy(handle);
	}
	FPL_CLEAR_STRUCT(condition);
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (mutex == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	if (!mutex->isValid) {
		fpl__PushError("Mutex is not valid!");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	pthread_cond_t *cond = &condition->internalHandle.posixCondition;
	pthread_mutex_t *mut = &mutex->internalHandle.posixMutex;
	bool result;
	if (timeout == FPL_TIMEOUT_INFINITE) {
		result = pthreadApi->pthread_cond_wait(cond, mut) == 0;
	} else {
		timespec t;
		fpl__InitWaitTimeSpec(timeout, &t);
		result = pthreadApi->pthread_cond_timedwait(cond, mut, &t) == 0;
	}
	return(result);
}

fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	pthread_cond_t *handle = &condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_signal(handle) == 0;
	return(result);
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	if (condition == fpl_null) {
		fpl__ArgumentNullError("Condition");
		return false;
	}
	if (!condition->isValid) {
		fpl__PushError("Condition is not valid!");
		return false;
	}
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (pthreadApi->libHandle == fpl_null) {
		fpl__PushError("PThread api not loaded");
		return false;
	}
	pthread_cond_t *handle = &condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_broadcast(handle) == 0;
	return(result);
}

//
// POSIX Library
//
fpl_platform_api fplDynamicLibraryHandle fplDynamicLibraryLoad(const char *libraryFilePath) {
	fplDynamicLibraryHandle result = FPL_ZERO_INIT;
	if (libraryFilePath != fpl_null) {
		void *p = dlopen(libraryFilePath, FPL__POSIX_DL_LOADTYPE);
		if (p != fpl_null) {
			result.internalHandle.posixLibraryHandle = p;
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
	if (name == fpl_null) {
		fpl__ArgumentNullError("Name");
		return fpl_null;
	}
	void *result = fpl_null;
	if (handle->internalHandle.posixLibraryHandle != fpl_null) {
		void *p = handle->internalHandle.posixLibraryHandle;
		result = dlsym(p, name);
	}
	return(result);
}

fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if (handle != fpl_null) {
		if (handle->internalHandle.posixLibraryHandle != fpl_null) {
			void *p = handle->internalHandle.posixLibraryHandle;
			dlclose(p);
			FPL_CLEAR_STRUCT(handle);
		}
	}
}

//
// POSIX Memory
//
fpl_platform_api void *fplMemoryAllocate(const size_t size) {
	// @NOTE(final): MAP_ANONYMOUS ensures that the memory is cleared to zero.

	// Allocate empty memory to hold the size + some arbitary padding + the actual data
	size_t newSize = sizeof(size_t) + FPL__SIZE_PADDING + size;
	void *basePtr = mmap(fpl_null, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	// Write the size at the beginning
	*(size_t *)basePtr = newSize;

	// The resulting address starts after the arbitary padding
	void *result = (uint8_t *)basePtr + sizeof(size_t) + FPL__SIZE_PADDING;
	return(result);
}

fpl_platform_api void fplMemoryFree(void *ptr) {
	// Free the base pointer which is stored to the left at the start of the size_t
	void *basePtr = (void *)((uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(size_t)));
	size_t storedSize = *(size_t *)basePtr;
	munmap(basePtr, storedSize);
}

//
// POSIX Files
//
fpl_platform_api bool fplOpenBinaryAnsiFile(const char *filePath, fplFileHandle *outHandle) {
	if (filePath != fpl_null && outHandle != fpl_null) {
		FPL_CLEAR_STRUCT(outHandle);
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			outHandle->isValid = true;
			outHandle->internalHandle.posixFileHandle = posixFileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplOpenBinaryWideFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	if (filePath != fpl_null && outHandle != fpl_null) {
		char utf8FilePath[1024] = FPL_ZERO_INIT;
		fplWideStringToAnsiString(filePath, fplGetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		bool result = fplOpenBinaryAnsiFile(utf8FilePath, outHandle);
		return(result);
	}
	return false;
}

fpl_platform_api bool fplCreateBinaryAnsiFile(const char *filePath, fplFileHandle *outHandle) {
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			outHandle->isValid = true;
			outHandle->internalHandle.posixFileHandle = posixFileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplCreateBinaryWideFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	if (filePath != fpl_null && outHandle != fpl_null) {
		char utf8FilePath[1024] = FPL_ZERO_INIT;
		fplWideStringToAnsiString(filePath, fplGetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		bool result = fplCreateBinaryAnsiFile(utf8FilePath, outHandle);
		return(result);
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
	if (!fileHandle->internalHandle.posixFileHandle) {
		fpl__PushError("File handle is not opened for reading");
		return 0;
	}
	int posixFileHandle = fileHandle->internalHandle.posixFileHandle;

	ssize_t res;
	do {
		res = read(posixFileHandle, targetBuffer, sizeToRead);
	} while (res == -1 && errno == EINTR);

	uint32_t result = 0;
	if (res != -1) {
		result = (uint32_t)res;
	}
	return(result);
}

fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	if (fileHandle == fpl_null) {
		fpl__ArgumentNullError("File handle");
		return 0;
	}
	if (sourceSize == 0) {
		return 0;
	}
	if (sourceBuffer == fpl_null) {
		fpl__ArgumentNullError("Source buffer");
		return 0;
	}
	if (!fileHandle->internalHandle.posixFileHandle) {
		fpl__PushError("File handle is not opened for writing");
		return 0;
	}

	int posixFileHandle = fileHandle->internalHandle.posixFileHandle;

	ssize_t res;
	do {
		res = write(posixFileHandle, sourceBuffer, sourceSize);
	} while (res == -1 && errno == EINTR);

	uint32_t result = 0;
	if (res != -1) {
		result = (uint32_t)res;
	}
	return(result);
}

fpl_platform_api void fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		int whence = SEEK_SET;
		if (mode == fplFilePositionMode_Current) {
			whence = SEEK_CUR;
		} else if (mode == fplFilePositionMode_End) {
			whence = SEEK_END;
		}
		lseek(posixFileHandle, position, whence);
	}
}

fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle) {
	uint32_t result = 0;
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		off_t res = lseek(posixFileHandle, 0, SEEK_CUR);
		if (res != -1) {
			result = (uint32_t)res;
		}
	}
	return(result);
}

fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle) {
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		close(posixFileHandle);
		FPL_CLEAR_STRUCT(fileHandle);
	}
}

fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath) {
	uint32_t result = 0;
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			off_t res = lseek(posixFileHandle, 0, SEEK_END);
			if (res != -1) {
				result = (uint32_t)res;
			}
			close(posixFileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle) {
	uint32_t result = 0;
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		off_t curPos = lseek(posixFileHandle, 0, SEEK_CUR);
		if (curPos != -1) {
			result = (uint32_t)lseek(posixFileHandle, 0, SEEK_END);
			lseek(posixFileHandle, curPos, SEEK_SET);
		}
	}
	return(result);
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if (filePath != fpl_null) {
		result = access(filePath, F_OK) != -1;
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

	if (access(sourceFilePath, F_OK) == -1) {
		fpl__PushError("Source file '%s' does not exits", sourceFilePath);
		return false;
	}
	if (!overwrite && access(sourceFilePath, F_OK) != -1) {
		fpl__PushError("Target file '%s' already exits", targetFilePath);
		return false;
	}

	int inputFileHandle;
	do {
		inputFileHandle = open(sourceFilePath, O_RDONLY);
	} while (inputFileHandle == -1 && errno == EINTR);
	if (inputFileHandle == -1) {
		fpl__PushError("Failed open source file '%s', error code: %d", sourceFilePath, inputFileHandle);
		return false;
	}

	int outputFileHandle;
	do {
		outputFileHandle = open(targetFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	} while (outputFileHandle == -1 && errno == EINTR);
	if (outputFileHandle == -1) {
		close(inputFileHandle);
		fpl__PushError("Failed creating target file '%s', error code: %d", targetFilePath, inputFileHandle);
		return false;
	}

	uint8_t buffer[1024 * 10]; // 10 kb buffer

	for (;;) {
		ssize_t readbytes;
		do {
			readbytes = read(inputFileHandle, buffer, FPL_ARRAYCOUNT(buffer));
		} while (readbytes == -1 && errno == EINTR);
		if (readbytes > 0) {
			ssize_t writtenBytes;
			do {
				writtenBytes = write(outputFileHandle, buffer, readbytes);
			} while (writtenBytes == -1 && errno == EINTR);
			if (writtenBytes <= 0) {
				break;
			}
		} else {
			break;
		}
	}

	close(outputFileHandle);
	close(inputFileHandle);

	return(true);
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
	bool result = rename(sourceFilePath, targetFilePath) == 0;
	return(result);
}

fpl_platform_api bool fplFileDelete(const char *filePath) {
	if (filePath == fpl_null) {
		fpl__ArgumentNullError("File path");
		return false;
	}
	bool result = unlink(filePath) == 0;
	return(result);
}

fpl_platform_api bool fplDirectoryExists(const char *path) {
	bool result = false;
	if (path != fpl_null) {
		struct stat sb;
		result = (stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
	}
	return(result);
}

fpl_platform_api bool fplDirectoriesCreate(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	return(result);
}
fpl_platform_api bool fplRemoveDirectory(const char *path) {
	if (path == fpl_null) {
		fpl__ArgumentNullError("Path");
		return false;
	}
	bool result = rmdir(path) == 0;
	return(result);
}
fpl_platform_api bool fplListFilesBegin(const char *pathAndFilter, fplFileEntry *firstEntry) {
	bool result = false;
	if (pathAndFilter != fpl_null && firstEntry != fpl_null) {
		// @IMPLEMENT(final): POSIX fplListFilesBegin
	}
	return(result);
}
fpl_platform_api bool fplListFilesNext(fplFileEntry *nextEntry) {
	bool result = false;
	if (nextEntry != fpl_null && nextEntry->internalHandle.posixFileHandle) {
		// @IMPLEMENT(final): POSIX fplListFilesNext
	}
	return(result);
}
fpl_platform_api void fplListFilesEnd(fplFileEntry *lastEntry) {
	if (lastEntry != fpl_null && lastEntry->internalHandle.posixFileHandle) {
		// @IMPLEMENT(final): POSIX fplListFilesEnd
		FPL_CLEAR_STRUCT(lastEntry);
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
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const size_t maxWideSourceLen, char *ansiDest, const size_t maxAnsiDestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (ansiDest == fpl_null) {
		fpl__ArgumentNullError("Ansi dest");
		return fpl_null;
	}
	size_t requiredLen = wcstombs(fpl_null, wideSource, maxWideSourceLen);
	size_t minRequiredLen = requiredLen + 1;
	if (maxAnsiDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max ansi dest len", maxAnsiDestLen, minRequiredLen);
		return fpl_null;
	}
	wcstombs(ansiDest, wideSource, maxWideSourceLen);
	ansiDest[requiredLen] = 0;
	return(ansiDest);
}
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	if (wideSource == fpl_null) {
		fpl__ArgumentNullError("Wide source");
		return fpl_null;
	}
	if (utf8Dest == fpl_null) {
		fpl__ArgumentNullError("UTF8 dest");
		return fpl_null;
	}
	// @TODO(final): UTF-8!
	size_t requiredLen = wcstombs(fpl_null, wideSource, maxWideSourceLen);
	size_t minRequiredLen = requiredLen + 1;
	if (maxUtf8DestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max utf8 dest len", maxUtf8DestLen, minRequiredLen);
		return fpl_null;
	}
	wcstombs(utf8Dest, wideSource, maxWideSourceLen);
	utf8Dest[requiredLen] = 0;
	return(utf8Dest);
}
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	if (ansiSource == fpl_null) {
		fpl__ArgumentNullError("Ansi source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	size_t requiredLen = mbstowcs(fpl_null, ansiSource, ansiSourceLen);
	size_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	mbstowcs(wideDest, ansiSource, ansiSourceLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	if (utf8Source == fpl_null) {
		fpl__ArgumentNullError("UTF8 source");
		return fpl_null;
	}
	if (wideDest == fpl_null) {
		fpl__ArgumentNullError("Wide dest");
		return fpl_null;
	}
	// @TODO(final): UTF-8!
	size_t requiredLen = mbstowcs(fpl_null, utf8Source, utf8SourceLen);
	size_t minRequiredLen = requiredLen + 1;
	if (maxWideDestLen < minRequiredLen) {
		fpl__ArgumentSizeTooSmallError("Max wide dest len", maxWideDestLen, minRequiredLen);
		return fpl_null;
	}
	mbstowcs(wideDest, utf8Source, utf8SourceLen);
	wideDest[requiredLen] = 0;
	return(wideDest);
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
fpl_platform_api void fplConsoleError(const char *text) {
	if (text != fpl_null) {
		fprintf(stderr, "%s", text);
	}
}
fpl_platform_api char fplConsoleWaitForCharInput() {
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
fpl_internal void fpl__X11ReleaseSubplatform(fpl__X11SubplatformState *subplatform) {
	fpl__UnloadX11Api(&subplatform->api);
}

fpl_internal bool fpl__X11InitSubplatform(fpl__X11SubplatformState *subplatform) {
	if (!fpl__LoadX11Api(&subplatform->api)) {
		fpl__PushError("Failed loading x11 api!");
		return false;
	}
	return true;
}

fpl_internal void fpl__X11ReleaseWindow(const fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState) {
	const fpl__X11Api *x11Api = &subplatform->api;
	if (windowState->window) {
		FPL_LOG("X11", "Hide window '%d' from display '%p'", (int)windowState->window, windowState->display);
		x11Api->XUnmapWindow(windowState->display, windowState->window);
		FPL_LOG("X11", "Destroy window '%d' on display '%p'", (int)windowState->window, windowState->display);
		x11Api->XDestroyWindow(windowState->display, windowState->window);
		windowState->window = 0;
	}
	if (windowState->colorMap) {
		FPL_LOG("X11", "Release color map '%d' from display '%p'", (int)windowState->colorMap, windowState->display);
		x11Api->XFreeColormap(windowState->display, windowState->colorMap);
		windowState->colorMap = 0;
	}
	if (windowState->display) {
		FPL_LOG("X11", "Close display '%p'", windowState->display);
		x11Api->XCloseDisplay(windowState->display);
		windowState->display = fpl_null;
	}
	FPL_CLEAR_STRUCT(windowState);
}

fpl_internal fplKey fpl__X11TranslateKeySymbol(const KeySym keySym) {
	switch (keySym) {
		case XK_BackSpace:
			return fplKey_Backspace;
		case XK_Tab:
			return fplKey_Tab;

		case XK_Return:
			return fplKey_Enter;

		case XK_Pause:
			return fplKey_Pause;
		case XK_Caps_Lock:
			return fplKey_CapsLock;

		case XK_Escape:
			return fplKey_Escape;
		case XK_space:
			return fplKey_Space;
		case XK_Page_Up:
			return fplKey_PageUp;
		case XK_Page_Down:
			return fplKey_PageDown;
		case XK_End:
			return fplKey_End;
		case XK_Home:
			return fplKey_Home;
		case XK_Left:
			return fplKey_Left;
		case XK_Up:
			return fplKey_Up;
		case XK_Right:
			return fplKey_Right;
		case XK_Down:
			return fplKey_Down;
		case XK_Print:
			return fplKey_Print;
		case XK_Insert:
			return fplKey_Insert;
		case XK_Delete:
			return fplKey_Delete;

		case XK_0:
			return fplKey_0;
		case XK_1:
			return fplKey_1;
		case XK_2:
			return fplKey_2;
		case XK_3:
			return fplKey_3;
		case XK_4:
			return fplKey_4;
		case XK_5:
			return fplKey_5;
		case XK_6:
			return fplKey_6;
		case XK_7:
			return fplKey_7;
		case XK_8:
			return fplKey_8;
		case XK_9:
			return fplKey_9;

		case XK_a:
			return fplKey_A;
		case XK_b:
			return fplKey_B;
		case XK_c:
			return fplKey_C;
		case XK_d:
			return fplKey_D;
		case XK_e:
			return fplKey_E;
		case XK_f:
			return fplKey_F;
		case XK_g:
			return fplKey_G;
		case XK_h:
			return fplKey_H;
		case XK_i:
			return fplKey_I;
		case XK_j:
			return fplKey_J;
		case XK_k:
			return fplKey_K;
		case XK_l:
			return fplKey_L;
		case XK_m:
			return fplKey_M;
		case XK_n:
			return fplKey_N;
		case XK_o:
			return fplKey_O;
		case XK_p:
			return fplKey_P;
		case XK_q:
			return fplKey_Q;
		case XK_r:
			return fplKey_R;
		case XK_s:
			return fplKey_S;
		case XK_t:
			return fplKey_T;
		case XK_u:
			return fplKey_U;
		case XK_v:
			return fplKey_V;
		case XK_w:
			return fplKey_W;
		case XK_x:
			return fplKey_X;
		case XK_y:
			return fplKey_Y;
		case XK_z:
			return fplKey_Z;

		case XK_Super_L:
			return fplKey_LeftWin;
		case XK_Super_R:
			return fplKey_RightWin;

		case XK_KP_0:
			return fplKey_NumPad0;
		case XK_KP_1:
			return fplKey_NumPad1;
		case XK_KP_2:
			return fplKey_NumPad2;
		case XK_KP_3:
			return fplKey_NumPad3;
		case XK_KP_4:
			return fplKey_NumPad4;
		case XK_KP_5:
			return fplKey_NumPad5;
		case XK_KP_6:
			return fplKey_NumPad6;
		case XK_KP_7:
			return fplKey_NumPad7;
		case XK_KP_8:
			return fplKey_NumPad8;
		case XK_KP_9:
			return fplKey_NumPad9;
		case XK_KP_Multiply:
			return fplKey_Multiply;
		case XK_KP_Add:
			return fplKey_Add;
		case XK_KP_Subtract:
			return fplKey_Substract;
		case XK_KP_Delete:
			return fplKey_Decimal;
		case XK_KP_Divide:
			return fplKey_Divide;
		case XK_F1:
			return fplKey_F1;
		case XK_F2:
			return fplKey_F2;
		case XK_F3:
			return fplKey_F3;
		case XK_F4:
			return fplKey_F4;
		case XK_F5:
			return fplKey_F5;
		case XK_F6:
			return fplKey_F6;
		case XK_F7:
			return fplKey_F7;
		case XK_F8:
			return fplKey_F8;
		case XK_F9:
			return fplKey_F9;
		case XK_F10:
			return fplKey_F10;
		case XK_F11:
			return fplKey_F11;
		case XK_F12:
			return fplKey_F12;
		case XK_F13:
			return fplKey_F13;
		case XK_F14:
			return fplKey_F14;
		case XK_F15:
			return fplKey_F15;
		case XK_F16:
			return fplKey_F16;
		case XK_F17:
			return fplKey_F17;
		case XK_F18:
			return fplKey_F18;
		case XK_F19:
			return fplKey_F19;
		case XK_F20:
			return fplKey_F20;
		case XK_F21:
			return fplKey_F21;
		case XK_F22:
			return fplKey_F22;
		case XK_F23:
			return fplKey_F23;
		case XK_F24:
			return fplKey_F24;

		case XK_Shift_L:
			return fplKey_LeftShift;
		case XK_Shift_R:
			return fplKey_RightShift;
		case XK_Control_L:
			return fplKey_LeftControl;
		case XK_Control_R:
			return fplKey_RightControl;
		case XK_Meta_L:
		case XK_Alt_L:
			return fplKey_LeftAlt;
		case XK_Mode_switch:
		case XK_ISO_Level3_Shift:
		case XK_Meta_R:
		case XK_Alt_R:
			return fplKey_RightAlt;

		default:
			return fplKey_None;
	}
}

fpl_internal bool fpl__X11InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *appState, fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	const fpl__X11Api *x11Api = &subplatform->api;

	FPL_LOG("X11", "Open default Display");
	windowState->display = x11Api->XOpenDisplay(fpl_null);
	if (windowState->display == fpl_null) {
		FPL_LOG("X11", "Failed opening default Display!");
		return false;
	}
	FPL_LOG("X11", "Successfully opened default Display: %p", windowState->display);

	FPL_LOG("X11", "Get default screen from display '%p'", windowState->display);
	windowState->screen = x11Api->XDefaultScreen(windowState->display);
	FPL_LOG("X11", "Got default screen from display '%p': %d", windowState->display, windowState->screen);

	FPL_LOG("X11", "Get root window from display '%p' and screen '%d'", windowState->display, windowState->screen);
	windowState->root = x11Api->XRootWindow(windowState->display, windowState->screen);
	FPL_LOG("X11", "Got root window from display '%p' and screen '%d': %d", windowState->display, windowState->screen, (int)windowState->root);

	bool usePreSetupWindow = false;
	fpl__PreSetupWindowResult setupResult = FPL_ZERO_INIT;
	if (setupCallbacks->preSetup != fpl_null) {
		FPL_LOG("X11", "Call Pre-Setup for Window");
		usePreSetupWindow = setupCallbacks->preSetup(appState, appState->initFlags, initSettings, &setupResult);
	}

	Visual *visual = fpl_null;
	int colorDepth = 0;
	Colormap colormap;
	if (usePreSetupWindow) {
		FPL_LOG("X11", "Got visual '%p' and color depth '%d' from pre-setup", setupResult.x11.visual, setupResult.x11.colorDepth);
		FPL_ASSERT(setupResult.x11.visual != fpl_null);
		visual = setupResult.x11.visual;
		colorDepth = setupResult.x11.colorDepth;
		colormap = x11Api->XCreateColormap(windowState->display, windowState->root, visual, AllocNone);
	} else {
		FPL_LOG("X11", "Using default colormap, visual, color depth");
		visual = x11Api->XDefaultVisual(windowState->display, windowState->root);
		colorDepth = x11Api->XDefaultDepth(windowState->display, windowState->root);
		colormap = x11Api->XDefaultColormap(windowState->display, windowState->root);
	}

	FPL_LOG("X11", "Using visual: %p", visual);
	FPL_LOG("X11", "Using color depth: %d", colorDepth);
	FPL_LOG("X11", "Using color map: %d", (int)colormap);

	windowState->colorMap = colormap;

	XSetWindowAttributes swa;
	swa.colormap = colormap;
	swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;

	int windowX = 0;
	int windowY = 0;
	int windowWidth;
	int windowHeight;
	if ((initSettings->window.windowWidth > 0) &&
		(initSettings->window.windowHeight > 0)) {
		windowWidth = initSettings->window.windowWidth;
		windowHeight = initSettings->window.windowHeight;
	} else {
		windowWidth = 400;
		windowHeight = 400;
	}

	// @TODO(final): X11 Fullscreen support

	FPL_LOG("X11", "Create window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
	windowState->window = x11Api->XCreateWindow(windowState->display,
												windowState->root,
												windowX,
												windowY,
												windowWidth,
												windowHeight,
												0,
												colorDepth,
												InputOutput,
												visual,
												CWColormap | CWEventMask,
												&swa);
	if (!windowState->window) {
		FPL_LOG("X11", "Failed creating window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'!", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
		fpl__X11ReleaseWindow(subplatform, windowState);
		return false;
	}
	FPL_LOG("X11", "Successfully created window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d': %d", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap, (int)windowState->window);

	char wm_delete_window_name[100] = "WM_DELETE_WINDOW";
	windowState->wmDeleteWindow = x11Api->XInternAtom(windowState->display, wm_delete_window_name, False);
	x11Api->XSetWMProtocols(windowState->display, windowState->window, &windowState->wmDeleteWindow, 1);

	char nameBuffer[1024] = FPL_ZERO_INIT;
	fplCopyAnsiString("Unnamed FPL X11 Window", nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
	FPL_LOG("X11", "Show window '%d' on display '%p' with title '%s'", (int)windowState->window, windowState->display, nameBuffer);
	x11Api->XStoreName(windowState->display, windowState->window, nameBuffer);
	x11Api->XMapWindow(windowState->display, windowState->window);

	FPL_ASSERT(FPL_ARRAYCOUNT(appState->window.keyMap) >= 256);

	// @NOTE(final): Valid key range for XLib is 8 to 255
	FPL_LOG("X11", "Build X11 Keymap");
	FPL_CLEAR_STRUCT(appState->window.keyMap);
	for (int keyCode = 8; keyCode <= 255; ++keyCode) {
		int dummy = 0;
		KeySym *keySyms = x11Api->XGetKeyboardMapping(windowState->display, keyCode, 1, &dummy);
		KeySym keySym = keySyms[0];
		fplKey mappedKey = fpl__X11TranslateKeySymbol(keySym);
		appState->window.keyMap[keyCode] = mappedKey;
		x11Api->XFree(keySyms);
	}

	appState->window.isRunning = true;

	return true;
}

fpl_internal fplKeyboardModifierFlags fpl__X11TranslateModifierFlags(const int state) {
	fplKeyboardModifierFlags result = fplKeyboardModifierFlags_None;
	if (state & ShiftMask) {
		result |= fplKeyboardModifierFlags_Shift;
	}
	if (state & ControlMask) {
		result |= fplKeyboardModifierFlags_Ctrl;
	}
	if (state & Mod1Mask) {
		result |= fplKeyboardModifierFlags_Alt;
	}
	if (state & Mod4Mask) {
		result |= fplKeyboardModifierFlags_Super;
	}
	return(result);
}

fpl_internal void fpl__X11PushMouseEvent(const fplMouseEventType eventType, const fplMouseButtonType mouseButton, const int x, const int y, const float wheelDelta) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = eventType;
	newEvent.mouse.mouseButton = mouseButton;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	newEvent.mouse.wheelDelta = wheelDelta;
	fpl__PushEvent(&newEvent);
}

fpl_internal bool fpl__X11HandleEvent(const fpl__X11SubplatformState *subplatform, fpl__PlatformWindowState *winState, XEvent *ev) {
	const fpl__X11WindowState *x11WinState = &winState->x11;
	bool result = true;
	switch (ev->type) {
		case ConfigureNotify:
		{
			// Window resized
			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_Resized;
			newEvent.window.width = ev->xconfigure.width;
			newEvent.window.height = ev->xconfigure.height;
			fpl__PushEvent(&newEvent);
		} break;

		case ClientMessage:
		{
			if ((Atom)ev->xclient.data.l[0] == x11WinState->wmDeleteWindow) {
				// Window exiting
				winState->isRunning = false;
				result = false;
			}
		} break;

		case KeyPress:
		case KeyRelease:
		{
			// Keyboard down/up
			int keyState = ev->xkey.state;
			int keyCode = ev->xkey.keycode;
			bool isDown = ev->type == KeyPress;

			fplEvent newEvent = FPL_ZERO_INIT;
			newEvent.type = fplEventType_Keyboard;
			newEvent.keyboard.keyCode = keyCode;
			newEvent.keyboard.mappedKey = fpl__GetMappedKey(winState, keyCode);
			newEvent.keyboard.type = isDown ? fplKeyboardEventType_KeyDown : fplKeyboardEventType_KeyUp;
			newEvent.keyboard.modifiers = fpl__X11TranslateModifierFlags(keyState);

			fpl__PushEvent(&newEvent);
		} break;

		case ButtonPress:
		{
			// Mouse down
			int x = ev->xbutton.x;
			int y = ev->xbutton.y;
			if (ev->xbutton.button == Button1) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonDown, fplMouseButtonType_Left, x, y, 0);
			} else if (ev->xbutton.button == Button2) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonDown, fplMouseButtonType_Middle, x, y, 0);
			} else if (ev->xbutton.button == Button3) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonDown, fplMouseButtonType_Right, x, y, 0);
			} else if (ev->xbutton.button == Button4) {
				fpl__X11PushMouseEvent(fplMouseEventType_Wheel, fplMouseButtonType_None, x, y, 1.0);
			} else if (ev->xbutton.button == Button5) {
				fpl__X11PushMouseEvent(fplMouseEventType_Wheel, fplMouseButtonType_None, x, y, -1.0);
			}
		} break;

		case ButtonRelease:
		{
			// Mouse up
			int x = ev->xbutton.x;
			int y = ev->xbutton.y;
			if (ev->xbutton.button == Button1) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonUp, fplMouseButtonType_Left, x, y, 0);
			} else if (ev->xbutton.button == Button2) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonUp, fplMouseButtonType_Middle, x, y, 0);
			} else if (ev->xbutton.button == Button3) {
				fpl__X11PushMouseEvent(fplMouseEventType_ButtonUp, fplMouseButtonType_Right, x, y, 0);
			}

		} break;

		case MotionNotify:
		{
			// Mouse move
			fpl__X11PushMouseEvent(fplMouseEventType_Move, fplMouseButtonType_None, ev->xmotion.x, ev->xmotion.y, 0);
		} break;

		case Expose:
		{
			// Repaint
		} break;

		default:
			break;
	}

	return(result);
}

fpl_platform_api bool fplPushEvent() {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	if (x11Api->XPending(windowState->display)) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		result = fpl__X11HandleEvent(&appState->x11, &appState->window, &ev);
	}
	return (result);
}

fpl_platform_api void fplUpdateGameControllers() {
	// @IMPLEMENT(final): X11 fplUpdateGameControllers
	// #include <linux/joystick.h>
	// https://linux.die.net/man/3/joystick_init
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	bool result = fpl__global__AppState->window.isRunning;
	return(result);
}

fpl_platform_api bool fplWindowUpdate() {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	int pendingCount = x11Api->XPending(windowState->display);
	while (pendingCount--) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		fpl__X11HandleEvent(&appState->x11, &appState->window, &ev);
	}
	x11Api->XFlush(windowState->display);
	result = appState->window.isRunning;
	return(result);
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	// @IMPLEMENT(final): X11 fplSetWindowCursorEnabled
}

fpl_platform_api bool fplGetWindowArea(fplWindowSize *outSize) {
	if (outSize == fpl_null) {
		fpl__ArgumentNullError("Out Size");
		return false;
	}

	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	XWindowAttributes attribs;
	x11Api->XGetWindowAttributes(windowState->display, windowState->window, &attribs);

	outSize->width = attribs.width;
	outSize->height = attribs.height;
	return(true);
}

fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	x11Api->XResizeWindow(windowState->display, windowState->window, width, height);
	x11Api->XFlush(windowState->display);
}

fpl_platform_api bool fplIsWindowResizable() {
	// @IMPLEMENT(final): X11 fplIsWindowResizable
	return false;
}

fpl_platform_api void fplSetWindowResizeable(const bool value) {
	// @IMPLEMENT(final): X11 fplSetWindowResizeable
}

fpl_platform_api bool fplIsWindowDecorated() {
	// @IMPLEMENT(final): X11 fplIsWindowDecorated
	return false;
}

fpl_platform_api void fplSetWindowDecorated(const bool value) {
	// @IMPLEMENT(final): X11 fplSetWindowDecorated
}

fpl_platform_api bool fplIsWindowFloating() {
	// @IMPLEMENT(final): X11 fplIsWindowFloating
	return false;
}

fpl_platform_api void fplSetWindowFloating(const bool value) {
	// @IMPLEMENT(final): X11 fplSetWindowFloating
}

fpl_platform_api bool fplSetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
	// @IMPLEMENT(final): X11 fplSetWindowFullscreen
	return false;
}

fpl_platform_api bool fplIsWindowFullscreen() {
	// @IMPLEMENT(final): X11 fplIsWindowFullscreen
	return false;
}

fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos) {
	if (outPos == fpl_null) {
		fpl__ArgumentNullError("Out pos");
		return false;
	}

	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	XWindowAttributes attribs;
	x11Api->XGetWindowAttributes(windowState->display, windowState->window, &attribs);

	outPos->left = attribs.x;
	outPos->top = attribs.y;

	return(true);
}

fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	x11Api->XMoveWindow(windowState->display, windowState->window, left, top);
}

fpl_platform_api void fplSetWindowAnsiTitle(const char *ansiTitle) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	char nameBuffer[256];
	fplCopyAnsiString(ansiTitle, nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
	x11Api->XStoreName(windowState->display, windowState->window, nameBuffer);
}

fpl_platform_api void fplSetWindowWideTitle(const wchar_t *wideTitle) {
	// @IMPLEMENT(final): X11 fplSetWindowWideTitle
}

fpl_platform_api bool fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
	// @IMPLEMENT(final): X11 fplGetClipboardAnsiText
	return false;
}

fpl_platform_api bool fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
	// @IMPLEMENT(final): X11 fplGetClipboardWideText
	return false;
}

fpl_platform_api bool fplSetClipboardAnsiText(const char *ansiSource) {
	// @IMPLEMENT(final): X11 fplSetClipboardAnsiText (ansi)
	return false;
}

fpl_platform_api bool fplSetClipboardWideText(const wchar_t *wideSource) {
	// @IMPLEMENT(final): X11 fplSetClipboardWideText (wide)
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
#   include <pwd.h> // getpwuid
#	include <sys/eventfd.h> // eventfd
#	include <sys/epoll.h> // epoll_create, epoll_ctl, epoll_wait
#	include <sys/select.h> // select
#	include <unistd.h> // write

fpl_internal void fpl__LinuxReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
}

fpl_internal bool fpl__LinuxInitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	return true;
}

//
// Linux OS
//
fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos) {
	// @IMPLEMENT(final): Linux fplGetOperatingSystemInfos
	return false;
}

fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, size_t maxNameBufferLen) {
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	bool result = false;
	if (pw != fpl_null) {
		fplCopyAnsiString(pw->pw_name, nameBuffer, maxNameBufferLen);
		result = true;
	} else {
		FPL_LOG("Linux", "Cannot find username from UID '%u'", uid);
	}
	return(result);
}

//
// Linux Threading
//
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (signal->isValid) {
		fpl__PushError("Signal '%p' is already valid", signal);
		return false;
	}

	int linuxEventHandle = eventfd((initialValue == fplSignalValue_Set) ? 1 : 0, EFD_CLOEXEC);
	if (linuxEventHandle == -1) {
		fpl__PushError("Failed initializing signal '%p'", signal);
		return false;
	}

	FPL_CLEAR_STRUCT(signal);
	signal->isValid = true;
	signal->internalHandle.linuxEventHandle = linuxEventHandle;
	return(true);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return;
	}
	if (!signal->isValid) {
		fpl__PushError("Signal '%p' is not valid", signal);
		return;
	}
	close(signal->internalHandle.linuxEventHandle);
	FPL_CLEAR_STRUCT(signal);
}

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (!signal->isValid) {
		fpl__PushError("Signal '%p' is not valid", signal);
		return(false);
	}
	int ev = signal->internalHandle.linuxEventHandle;
	if (timeout == FPL_TIMEOUT_INFINITE) {
		uint64_t value;
		read(ev, &value, sizeof(value));
		return true;
	} else {
		fd_set f;
		FD_ZERO(&f);
		FD_SET(ev, &f);
		struct timeval t = { 0, timeout * 1000 };
		int selectResult = select(1, &f, NULL, NULL, &t);
		if (selectResult == 0) {
			// Timeout
			return false;
		} else if (selectResult == -1) {
			// Error
			return false;
		} else {
			return true;
		}
	}
}

fpl_internal bool fpl__LinuxSignalWaitForMultiple(fplSignalHandle *signals[], const uint32_t minCount, const uint32_t maxCount, const fplTimeoutValue timeout) {
	if (signals == fpl_null) {
		fpl__ArgumentNullError("Signals");
		return false;
	}
	if (maxCount > FPL__MAX_SIGNAL_COUNT) {
		fpl__ArgumentSizeTooBigError("Max count", maxCount, FPL__MAX_SIGNAL_COUNT);
		return false;
	}
	for (uint32_t index = 0; index < maxCount; ++index) {
		fplSignalHandle *signal = signals[index];
		if (signal == fpl_null) {
			fpl__PushError("Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if (!signal->isValid) {
			fpl__PushError("Signal '%p' for index '%d' is not valid", signal, index);
			return false;
		}
	}

	int e = epoll_create(maxCount);
	FPL_ASSERT(e != 0);

	// Register events and map each to the array index
	struct epoll_event events[FPL__MAX_SIGNAL_COUNT];
	for (int i = 0; i < maxCount; i++) {
		events[i].events = EPOLLIN;
		events[i].data.u32 = i;
		int x = epoll_ctl(e, EPOLL_CTL_ADD, signals[i]->internalHandle.linuxEventHandle, events + i);
		FPL_ASSERT(x == 0);
	}

	// Wait
	int t = timeout == FPL_TIMEOUT_INFINITE ? -1 : timeout;
	int eventsResult = -1;
	int waiting = minCount;
	struct epoll_event revent[FPL__MAX_SIGNAL_COUNT];
	while (waiting > 0) {
		int ret = epoll_wait(e, revent, waiting, t);
		if (ret == 0) {
			if (minCount == maxCount) {
				eventsResult = -1;
			}
			break;
		}
		for (int i = 0; i < ret; i++) {
			epoll_ctl(e, EPOLL_CTL_DEL, signals[revent[i].data.u32]->internalHandle.linuxEventHandle, NULL);
		}
		eventsResult = revent[0].data.u32;
		waiting -= ret;
	}
	close(e);
	bool result = (waiting == 0);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__LinuxSignalWaitForMultiple(signals, count, count, timeout);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout) {
	bool result = fpl__LinuxSignalWaitForMultiple(signals, 1, count, timeout);
	return(result);
}

fpl_platform_api bool fplSignalSet(fplSignalHandle *signal) {
	if (signal == fpl_null) {
		fpl__ArgumentNullError("Signal");
		return false;
	}
	if (!signal->isValid) {
		fpl__PushError("Signal '%p' is not valid", signal);
		return(false);
	}
	uint64_t value = 1;
	int writtenBytes = write(signal->internalHandle.linuxEventHandle, &value, sizeof(value));
	bool result = writtenBytes == sizeof(value);
	return(result);
}

//
// Linux Hardware
//
fpl_platform_api size_t fplGetProcessorCoreCount() {
	size_t result = sysconf(_SC_NPROCESSORS_ONLN);
	return(result);
}

fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen) {
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
		size_t readSize = maxBufferSize;
		size_t readPos = 0;
		bool found = false;
		int bytesRead = 0;
		while ((bytesRead = fread(&buffer[readPos], readSize, 1, fileHandle)) > 0) {
			char *lastP = &buffer[0];
			char *p = &buffer[0];
			while (*p) {
				if (*p == '\n') {
					int len = p - lastP;
					FPL_ASSERT(len > 0);
					if (fplIsStringEqualLen(lastP, 10, "model name", 10)) {
						fplCopyAnsiStringLen(lastP, len, line, FPL_ARRAYCOUNT(line));
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

			int remaining = &buffer[maxBufferSize] - lastP;
			FPL_ASSERT(remaining >= 0);
			if (remaining > 0) {
				// Buffer does not contain a line separator - copy back to remaining characters to the line
				fplCopyAnsiStringLen(lastP, remaining, line, FPL_ARRAYCOUNT(line));
				// Copy back line to buffer and use a different read position/size
				fplCopyAnsiStringLen(line, remaining, buffer, maxBufferSize);
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
				fplCopyAnsiString(p, destBuffer, maxDestBufferLen);
				result = destBuffer;
			}
		}
		fclose(fileHandle);
	}
	return(result);
}

fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos) {
	// @IMPLEMENT(final): Linux fplGetRunningMemoryInfos
	return(false);
}

fpl_platform_api fplArchType fplGetRunningArchitecture() {
	// @IMPLEMENT(final): Linux fplGetRunningArchitecture
	return(fplArchType_Unknown);
}

//
// Linux Paths
//
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	if (maxDestLen == 0) {
		fpl__ArgumentZeroError("Max dest len");
		return fpl_null;
	}
	char buf[1024];
	if (readlink("/proc/self/exe", buf, FPL_ARRAYCOUNT(buf) - 1)) {
		int len = fplGetAnsiStringLength(buf);
		char *lastP = buf + (len - 1);
		char *p = lastP;
		while (p != buf) {
			if (*p == '/') {
				len = (lastP - buf) + 1;
				break;
			}
			--p;
		}
		size_t requiredLen = len + 1;
		if (maxDestLen < requiredLen) {
			fpl__ArgumentSizeTooSmallError("Max dest len", len, requiredLen);
			return fpl_null;
		}
		char *result = fplCopyAnsiStringLen(buf, len, destPath, maxDestLen);
		return(result);
	}
	return fpl_null;
}

fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
	const char *homeDir = getenv("HOME");
	if (homeDir == fpl_null) {
		int userId = getuid();
		passwd *userPwd = getpwuid(userId);
		homeDir = userPwd->pw_dir;
	}
	char *result = fplCopyAnsiString(homeDir, destPath, maxDestLen);
	return(result);
}
#endif // FPL_PLATFORM_LINUX

// ############################################################################
//
// > UNIX_PLATFORM
//
// ############################################################################
#if defined(FPL_PLATFORM_UNIX)
fpl_internal void fpl__UnixReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
}

fpl_internal bool fpl__UnixInitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	return true;
}

//
// Unix OS
//
fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos) {
	// @IMPLEMENT(final): Unix fplGetOperatingSystemInfos
	return false;
}

fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, size_t maxNameBufferLen) {
	// @IMPLEMENT(final): Unix fplGetCurrentUsername
	return false;
}

//
// Unix Hardware
//
fpl_platform_api size_t fplGetProcessorCoreCount() {
	size_t result = 1;
	// @IMPLEMENT(final): Unix fplGetProcessorCoreCount
	return(result);
}

fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen) {
	if (destBuffer == fpl_null) {
		fpl__ArgumentNullError("Dest buffer");
		return fpl_null;
	}
	if (maxDestBufferLen == 0) {
		fpl__ArgumentZeroError("Max dest buffer len");
		return fpl_null;
	}
	// @IMPLEMENT(final): Unix fplGetProcessorName
	return(fpl_null);
}

fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos) {
	// @IMPLEMENT(final): Unix fplGetRunningMemoryInfos
	return(false);
}

fpl_platform_api fplArchType fplGetRunningArchitecture() {
	// @IMPLEMENT(final): Unix fplGetRunningArchitectureType
	return(fplArchType_Unknown);
}

//
// Unix Paths
//
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	if (destPath == fpl_null) {
		fpl__ArgumentNullError("Dest path");
		return fpl_null;
	}
	if (maxDestLen == 0) {
		fpl__ArgumentZeroError("Max dest len");
		return fpl_null;
	}
	// @IMPLEMENT(final): Unix fplGetExecutableFilePath
	return fpl_null;
}

fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
	// @IMPLEMENT(final): Unix fplGetHomePath
	return fpl_null;
}
#endif // FPL_PLATFORM_UNIX

// ****************************************************************************
//
// > VIDEO_DRIVERS
//
// ****************************************************************************
#if !defined(FPL_VIDEO_DRIVERS_IMPLEMENTED) && defined(FPL_ENABLE_VIDEO)
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

	int pixelFormat = wapi->gdi.ChoosePixelFormat(deviceContext, &pfd);
	if (!pixelFormat) {
		fpl__PushError("Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
		return false;
	}

	if (!wapi->gdi.SetPixelFormat(deviceContext, pixelFormat, &pfd)) {
		fpl__PushError("Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
		return false;
	}

	wapi->gdi.DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

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
			fpl__PushError("No opengl compability profile selected, please specific Core fplOpenGLCompabilityFlags_Core or fplOpenGLCompabilityFlags_Compability");
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
#	include <GL/glx.h> // XVisualInfo, GLXContext, GLXDrawable

#define FPL__FUNC_GLX_glXQueryVersion(name) Bool name(Display *dpy,  int *major,  int *minor)
typedef FPL__FUNC_GLX_glXQueryVersion(fpl__func_glx_glXQueryVersion);
#define FPL__FUNC_GLX_glXChooseVisual(name) XVisualInfo* name(Display *dpy, int screen, int *attribList)
typedef FPL__FUNC_GLX_glXChooseVisual(fpl__func_glx_glXChooseVisual);
#define FPL__FUNC_GLX_glXCreateContext(name) GLXContext name(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct)
typedef FPL__FUNC_GLX_glXCreateContext(fpl__func_glx_glXCreateContext);
#define FPL__FUNC_GLX_glXCreateNewContext(name) GLXContext name(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct)
typedef FPL__FUNC_GLX_glXCreateNewContext(fpl__func_glx_glXCreateNewContext);
#define FPL__FUNC_GLX_glXDestroyContext(name) void name(Display *dpy, GLXContext ctx)
typedef FPL__FUNC_GLX_glXDestroyContext(fpl__func_glx_glXDestroyContext);
#define FPL__FUNC_GLX_glXMakeCurrent(name) Bool name(Display *dpy, GLXDrawable drawable, GLXContext ctx)
typedef FPL__FUNC_GLX_glXMakeCurrent(fpl__func_glx_glXMakeCurrent);
#define FPL__FUNC_GLX_glXSwapBuffers(name) void name(Display *dpy, GLXDrawable drawable)
typedef FPL__FUNC_GLX_glXSwapBuffers(fpl__func_glx_glXSwapBuffers);
#define FPL__FUNC_GLX_glXGetProcAddress(name) void *name(const GLubyte *procName)
typedef FPL__FUNC_GLX_glXGetProcAddress(fpl__func_glx_glXGetProcAddress);
#define FPL__FUNC_GLX_glXChooseFBConfig(name) GLXFBConfig *name(Display *dpy, int screen, const int *attrib_list, int *nelements)
typedef FPL__FUNC_GLX_glXChooseFBConfig(fpl__func_glx_glXChooseFBConfig);
#define FPL__FUNC_GLX_glXGetFBConfigs(name) GLXFBConfig *name(Display *dpy, int screen, int *nelements)
typedef FPL__FUNC_GLX_glXGetFBConfigs(fpl__func_glx_glXGetFBConfigs);
#define FPL__FUNC_GLX_glXGetVisualFromFBConfig(name) XVisualInfo *name(Display *dpy, GLXFBConfig config)
typedef FPL__FUNC_GLX_glXGetVisualFromFBConfig(fpl__func_glx_glXGetVisualFromFBConfig);
#define FPL__FUNC_GLX_glXGetFBConfigAttrib(name) int name(Display *dpy, GLXFBConfig config, int attribute, int *value)
typedef FPL__FUNC_GLX_glXGetFBConfigAttrib(fpl__func_glx_glXGetFBConfigAttrib);
#define FPL__FUNC_GLX_glXCreateWindow(name) GLXWindow name(Display *dpy, GLXFBConfig config, Window win,  const int *attrib_list)
typedef FPL__FUNC_GLX_glXCreateWindow(fpl__func_glx_glXCreateWindow);
#define FPL__FUNC_GLX_glXQueryExtension(name) Bool name(Display *dpy, int *errorBase, int *eventBase)
typedef FPL__FUNC_GLX_glXQueryExtension(fpl__func_glx_glXQueryExtension);
#define FPL__FUNC_GLX_glXQueryExtensionsString(name) const char *name(Display *dpy, int screen)
typedef FPL__FUNC_GLX_glXQueryExtensionsString(fpl__func_glx_glXQueryExtensionsString);

// Modern GLX
#define FPL__FUNC_GLX_glXCreateContextAttribsARB(name) GLXContext name(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list)
typedef FPL__FUNC_GLX_glXCreateContextAttribsARB(fpl__func_glx_glXCreateContextAttribsARB);

#define FPL__GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define FPL__GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define FPL__GLX_CONTEXT_FLAGS_ARB 0x2094
#define FPL__GLX_CONTEXT_PROFILE_MASK_ARB 0x9126

#define FPL__GLX_CONTEXT_DEBUG_BIT_ARB 0x0001
#define FPL__GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define FPL__GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define FPL__GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef struct fpl__X11VideoOpenGLApi {
	void *libHandle;
	fpl__func_glx_glXQueryVersion *glXQueryVersion;
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

	fpl__func_glx_glXCreateContextAttribsARB *glXCreateContextAttribsARB;
} fpl__X11VideoOpenGLApi;

fpl_internal void fpl__X11UnloadVideoOpenGLApi(fpl__X11VideoOpenGLApi *api) {
	if (api->libHandle != fpl_null) {
		FPL_LOG("GLX", "Unload Api (Library '%p')", api->libHandle);
		dlclose(api->libHandle);
	}
	FPL_CLEAR_STRUCT(api);
}

fpl_internal bool fpl__X11LoadVideoOpenGLApi(fpl__X11VideoOpenGLApi *api) {
	const char* libFileNames[] = {
		"libGL.so.1",
		"libGL.so",
	};

	bool result = false;
	for (uint32_t index = 0; index < FPL_ARRAYCOUNT(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		FPL_LOG("GLX", "Load GLX Api from Library: %s", libName);
		void *libHandle = api->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if (libHandle != fpl_null) {
			FPL_LOG("GLX", "Library Found: '%s', Resolving Procedures", libName);
			do {
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXQueryVersion, fpl__func_glx_glXQueryVersion, "glXQueryVersion");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXChooseVisual, fpl__func_glx_glXChooseVisual, "glXChooseVisual");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXCreateContext, fpl__func_glx_glXCreateContext, "glXCreateContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXDestroyContext, fpl__func_glx_glXDestroyContext, "glXDestroyContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXCreateNewContext, fpl__func_glx_glXCreateNewContext, "glXCreateNewContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXMakeCurrent, fpl__func_glx_glXMakeCurrent, "glXMakeCurrent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXSwapBuffers, fpl__func_glx_glXSwapBuffers, "glXSwapBuffers");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXGetProcAddress, fpl__func_glx_glXGetProcAddress, "glXGetProcAddress");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXChooseFBConfig, fpl__func_glx_glXChooseFBConfig, "glXChooseFBConfig");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXGetFBConfigs, fpl__func_glx_glXGetFBConfigs, "glXGetFBConfigs");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXGetVisualFromFBConfig, fpl__func_glx_glXGetVisualFromFBConfig, "glXGetVisualFromFBConfig");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXGetFBConfigAttrib, fpl__func_glx_glXGetFBConfigAttrib, "glXGetFBConfigAttrib");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXCreateWindow, fpl__func_glx_glXCreateWindow, "glXCreateWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXQueryExtension, fpl__func_glx_glXQueryExtension, "glXQueryExtension");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, api->glXQueryExtensionsString, fpl__func_glx_glXQueryExtensionsString, "glXQueryExtensionsString");
				result = true;
			} while (0);
			if (result) {
				FPL_LOG("GLX", "Successfully loaded GLX Api from Library '%s'", libName);
				break;
			}
		}
		fpl__X11UnloadVideoOpenGLApi(api);
	}
	return (result);
}

typedef struct fpl__X11VideoOpenGLState {
	fpl__X11VideoOpenGLApi api;
	GLXFBConfig fbConfig;
	XVisualInfo *visualInfo;
	GLXContext context;
	bool isActiveContext;
} fpl__X11VideoOpenGLState;

fpl_internal bool fpl__X11InitFrameBufferConfigVideoOpenGL(const fpl__X11Api *x11Api, const fpl__X11WindowState *windowState, fpl__X11VideoOpenGLState *glState) {
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	FPL_LOG("GLX", "Query GLX version for display '%p'", windowState->display);
	int major = 0, minor = 0;
	if (!glApi->glXQueryVersion(windowState->display, &major, &minor)) {
		FPL_LOG("GLX", "Failed querying GLX version for display '%p'", windowState->display);
		return false;
	}
	FPL_LOG("GLX", "Successfully queried GLX version for display '%p': %d.%d", windowState->display, major, minor);

	// @NOTE(final): Required for AMD Drivers?

	FPL_LOG("GLX", "Query OpenGL extension on display '%p'", windowState->display);
	if (!glApi->glXQueryExtension(windowState->display, fpl_null, fpl_null)) {
		FPL_LOG("GLX", "OpenGL GLX Extension is not supported by the active display '%p'", windowState->display);
		return false;
	}

	const char *extensionString = glApi->glXQueryExtensionsString(windowState->display, windowState->screen);
	if (extensionString != fpl_null) {
		FPL_LOG("GLX", "OpenGL GLX extensions: %s", extensionString);
	}

	bool isModern = major > 1 || (major == 1 && minor >= 3);

	int attr[32] = FPL_ZERO_INIT;
	int attrIndex = 0;

	attr[attrIndex++] = GLX_X_VISUAL_TYPE;
	attr[attrIndex++] = GLX_TRUE_COLOR;

	if (!isModern) {
		attr[attrIndex++] = GLX_RGBA;
		attr[attrIndex++] = True;
	}

	attr[attrIndex++] = GLX_DOUBLEBUFFER;
	attr[attrIndex++] = True;

	attr[attrIndex++] = GLX_RED_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex++] = GLX_GREEN_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex++] = GLX_BLUE_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex++] = GLX_DEPTH_SIZE;
	attr[attrIndex++] = 24;

	attr[attrIndex++] = GLX_STENCIL_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex] = 0;

	if (isModern) {
		// Use frame buffer config approach (GLX >= 1.3)
		FPL_LOG("GLX", "Get framebuffer configuration from display '%p' and screen '%d'", windowState->display, windowState->screen);
		int configCount = 0;
		GLXFBConfig *configs = glApi->glXChooseFBConfig(windowState->display, windowState->screen, attr, &configCount);
		if (configs == fpl_null || !configCount) {
			FPL_LOG("GLX", "No framebuffer configuration from display '%p' and screen '%d' found!", windowState->display, windowState->screen);
			glState->fbConfig = fpl_null;
			return false;
		}
		glState->fbConfig = configs[0];
		glState->visualInfo = fpl_null;
		FPL_LOG("GLX", "Successfully got framebuffer configuration from display '%p' and screen '%d': %p", windowState->display, windowState->screen, glState->fbConfig);

		FPL_LOG("GLX", "Release %d framebuffer configurations", configCount);
		x11Api->XFree(configs);
	} else {
		// Use choose visual (Old way)
		FPL_LOG("GLX", "Choose visual from display '%p' and screen '%d'", windowState->display, windowState->screen);
		XVisualInfo *visualInfo = glApi->glXChooseVisual(windowState->display, windowState->screen, attr);
		if (visualInfo == fpl_null) {
			FPL_LOG("GLX", "No visual info for display '%p' and screen '%d' found!", windowState->display, windowState->screen);
			return false;
		}
		glState->visualInfo = visualInfo;
		glState->fbConfig = fpl_null;
		FPL_LOG("GLX", "Successfully got visual info from display '%p' and screen '%d': %p", windowState->display, windowState->screen, glState->visualInfo);
	}

	return true;
}

fpl_internal_inline bool fpl__X11SetPreWindowSetupForOpenGL(const fpl__X11Api *x11Api, const fpl__X11WindowState *windowState, const fpl__X11VideoOpenGLState *glState, fpl__X11PreWindowSetupResult *outResult) {
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	if (glState->fbConfig != fpl_null) {
		FPL_LOG("GLX", "Get visual info from display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
		XVisualInfo *visualInfo = glApi->glXGetVisualFromFBConfig(windowState->display, glState->fbConfig);
		if (visualInfo == fpl_null) {
			FPL_LOG("GLX", "Failed getting visual info from display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
			return false;
		}
		FPL_LOG("GLX", "Successfully got visual info from display '%p' and frame buffer config '%p': %p", windowState->display, glState->fbConfig, visualInfo);

		FPL_LOG("GLX", "Using visual: %p", visualInfo->visual);
		FPL_LOG("GLX", "Using color depth: %d", visualInfo->depth);

		outResult->visual = visualInfo->visual;
		outResult->colorDepth = visualInfo->depth;

		FPL_LOG("GLX", "Release visual info '%p'", visualInfo);
		x11Api->XFree(visualInfo);
	} else if (glState->visualInfo != fpl_null) {
		FPL_LOG("GLX", "Using existing visual info: %p", glState->visualInfo);
		FPL_LOG("GLX", "Using visual: %p", glState->visualInfo->visual);
		FPL_LOG("GLX", "Using color depth: %d", glState->visualInfo->depth);
		outResult->visual = glState->visualInfo->visual;
		outResult->colorDepth = glState->visualInfo->depth;
	} else {
		FPL_LOG("GLX", "No visual info or frame buffer config defined!");
		return false;
	}

	return true;
}

fpl_internal void fpl__X11ReleaseVideoOpenGL(const fpl__X11SubplatformState *subplatform, const fpl__X11WindowState *windowState, fpl__X11VideoOpenGLState *glState) {
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	if (glState->isActiveContext) {
		FPL_LOG("GLX", "Deactivate GLX rendering context for display '%p'", windowState->display);
		glApi->glXMakeCurrent(windowState->display, 0, fpl_null);
		glState->isActiveContext = false;
	}

	if (glState->context != fpl_null) {
		FPL_LOG("GLX", "Destroy GLX rendering context '%p' for display '%p'", glState->context, windowState->display);
		glApi->glXDestroyContext(windowState->display, glState->context);
		glState->context = fpl_null;
	}

	if (glState->visualInfo != fpl_null) {
		FPL_LOG("GLX", "Destroy visual info '%p' (Fallback)", glState->visualInfo);
		x11Api->XFree(glState->visualInfo);
		glState->visualInfo = fpl_null;
	}
}

fpl_internal bool fpl__X11InitVideoOpenGL(const fpl__X11SubplatformState *subplatform, const fpl__X11WindowState *windowState, const fplVideoSettings *videoSettings, fpl__X11VideoOpenGLState *glState) {
	const fpl__X11Api *x11Api = &subplatform->api;
	fpl__X11VideoOpenGLApi *glApi = &glState->api;

	//
	// Create legacy context
	//
	GLXContext legacyRenderingContext;
	if (glState->fbConfig != fpl_null) {
		FPL_LOG("GLX", "Create GLX legacy rendering context on display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
		legacyRenderingContext = glApi->glXCreateNewContext(windowState->display, glState->fbConfig, GLX_RGBA_TYPE, fpl_null, GL_TRUE);
		if (!legacyRenderingContext) {
			FPL_LOG("GLX", "Failed creating GLX legacy rendering context on display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
			goto failed_x11_glx;
		}
		FPL_LOG("GLX", "Successfully created GLX legacy rendering context '%p' on display '%p' and frame buffer config '%p'", legacyRenderingContext, windowState->display, glState->fbConfig);
	} else if (glState->visualInfo != fpl_null) {
		FPL_LOG("GLX", "Create GLX legacy rendering context on display '%p' and visual info '%p'", windowState->display, glState->visualInfo);
		legacyRenderingContext = glApi->glXCreateContext(windowState->display, glState->visualInfo, fpl_null, GL_TRUE);
		if (!legacyRenderingContext) {
			FPL_LOG("GLX", "Failed creating GLX legacy rendering context on display '%p' and visual info '%p'", windowState->display, glState->visualInfo);
			goto failed_x11_glx;
		}
	} else {
		FPL_LOG("GLX", "No visual info or frame buffer config defined!");
		goto failed_x11_glx;
	}

	//
	// Activate legacy context
	//
	FPL_LOG("GLX", "Activate GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
	if (!glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext)) {
		FPL_LOG("GLX", "Failed activating GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
		goto failed_x11_glx;
	} else {
		FPL_LOG("GLX", "Successfully activated GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
	}

	//
	// Load extensions
	//
	glApi->glXCreateContextAttribsARB = (fpl__func_glx_glXCreateContextAttribsARB *)glApi->glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

	// Disable legacy rendering context
	glApi->glXMakeCurrent(windowState->display, 0, fpl_null);

	GLXContext activeRenderingContext;

	if ((videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) && (glState->fbConfig != fpl_null)) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if (!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			fpl__PushError("You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			goto failed_x11_glx;
		}

		if (glApi->glXCreateContextAttribsARB == fpl_null) {
			fpl__PushError("glXCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			goto failed_x11_glx;
		}

		int flags = 0;
		int profile = 0;
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL__GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL__GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			fpl__PushError("No opengl compability profile selected, please specific Core OpenGLCompabilityFlags_Core or OpenGLCompabilityFlags_Compability");
			goto failed_x11_glx;
		}
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
			flags = FPL__GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		}

		int contextAttribIndex = 0;
		int contextAttribList[32] = FPL_ZERO_INIT;
		contextAttribList[contextAttribIndex++] = FPL__GLX_CONTEXT_MAJOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = videoSettings->graphics.opengl.majorVersion;
		contextAttribList[contextAttribIndex++] = FPL__GLX_CONTEXT_MINOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = videoSettings->graphics.opengl.minorVersion;
		contextAttribList[contextAttribIndex++] = FPL__GLX_CONTEXT_PROFILE_MASK_ARB;
		contextAttribList[contextAttribIndex++] = profile;
		if (flags > 0) {
			contextAttribList[contextAttribIndex++] = FPL__GLX_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}
		contextAttribList[contextAttribIndex] = 0;

		GLXContext modernRenderingContext = glApi->glXCreateContextAttribsARB(windowState->display, glState->fbConfig, fpl_null, True, contextAttribList);
		if (!modernRenderingContext) {
			fpl__PushError("Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags);

			// Fallback to legacy rendering context
			glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		} else {
			if (!glApi->glXMakeCurrent(windowState->display, windowState->window, modernRenderingContext)) {
				fpl__PushError(
					"Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context",
					videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion,
					videoSettings->graphics.opengl.compabilityFlags);

			// Destroy modern rendering context
				glApi->glXDestroyContext(windowState->display, modernRenderingContext);

				// Fallback to legacy rendering context
				glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			} else {
				// Destroy legacy rendering context
				glApi->glXDestroyContext(windowState->display, legacyRenderingContext);
				legacyRenderingContext = fpl_null;
				activeRenderingContext = modernRenderingContext;
			}
		}
	} else {
		// Caller wants legacy context
		glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext);
		activeRenderingContext = legacyRenderingContext;
	}

	bool result;

	FPL_ASSERT(activeRenderingContext != fpl_null);
	glState->context = activeRenderingContext;
	glState->isActiveContext = true;
	result = true;
	goto done_x11_glx;

failed_x11_glx:
	result = false;

done_x11_glx:
	if (glState->visualInfo != fpl_null) {
		// If there is a cached visual info, get rid of it now - regardless of its result
		FPL_LOG("GLX", "Destroy visual info '%p'", glState->visualInfo);
		x11Api->XFree(glState->visualInfo);
		glState->visualInfo = fpl_null;
	}

	if (!result) {
		if (legacyRenderingContext) {
			glApi->glXDestroyContext(windowState->display, legacyRenderingContext);
		}
		fpl__X11ReleaseVideoOpenGL(subplatform, windowState, glState);
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
#if !defined(FPL_AUDIO_DRIVERS_IMPLEMENTED) && defined(FPL_ENABLE_AUDIO)
#	define FPL_AUDIO_DRIVERS_IMPLEMENTED

typedef enum fpl__AudioDeviceState {
	fpl__AudioDeviceState_Uninitialized = 0,
	fpl__AudioDeviceState_Stopped,
	fpl__AudioDeviceState_Started,
	fpl__AudioDeviceState_Starting,
	fpl__AudioDeviceState_Stopping,
} fpl__AudioDeviceState;

typedef struct fpl__CommonAudioState {
	fplAudioDeviceFormat internalFormat;
	fpl_audio_client_read_callback *clientReadCallback;
	void *clientUserData;
	volatile fpl__AudioDeviceState state;
} fpl__CommonAudioState;

fpl_internal_inline uint32_t fpl__ReadAudioFramesFromClient(const fpl__CommonAudioState *commonAudio, uint32_t frameCount, void *pSamples) {
	uint32_t outputSamplesWritten = 0;
	if (commonAudio->clientReadCallback != fpl_null) {
		outputSamplesWritten = commonAudio->clientReadCallback(&commonAudio->internalFormat, frameCount, pSamples, commonAudio->clientUserData);
	}
	return outputSamplesWritten;
}

// Global Audio GUIDs
#if defined(FPL_PLATFORM_WIN32)
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
#endif

// Forward declarations
fpl_internal fpl__AudioDeviceState fpl__AudioGetDeviceState(fpl__CommonAudioState *audioState);
fpl_internal bool fpl__IsAudioDeviceInitialized(fpl__CommonAudioState *audioState);
fpl_internal bool fpl__IsAudioDeviceStarted(fpl__CommonAudioState *audioState);

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

typedef struct fpl__DirectSoundApi {
	HMODULE dsoundLibrary;
	func_DirectSoundCreate *DirectSoundCreate;
	func_DirectSoundEnumerateA *DirectSoundEnumerateA;
} fpl__DirectSoundApi;

fpl_internal void fpl__UnloadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	FPL_ASSERT(dsoundApi != fpl_null);
	if (dsoundApi->dsoundLibrary != fpl_null) {
		FreeLibrary(dsoundApi->dsoundLibrary);
	}
	FPL_CLEAR_STRUCT(dsoundApi);
}

fpl_internal bool fpl__LoadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	FPL_ASSERT(dsoundApi != fpl_null);

	const char *dsoundLibraryName = "dsound.dll";
	HMODULE library = dsoundApi->dsoundLibrary = LoadLibraryA(dsoundLibraryName);
	if (library == fpl_null) {
		fpl__PushError("Failed loading library '%s'", dsoundLibraryName);
		return false;
	}
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, dsoundLibraryName, dsoundApi->DirectSoundCreate, func_DirectSoundCreate, "DirectSoundCreate");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(library, dsoundLibraryName, dsoundApi->DirectSoundEnumerateA, func_DirectSoundEnumerateA, "DirectSoundEnumerateA");

	return true;
}

typedef struct fpl__DirectSoundAudioState {
	fpl__DirectSoundApi api;
	LPDIRECTSOUND directSound;
	LPDIRECTSOUNDBUFFER primaryBuffer;
	LPDIRECTSOUNDBUFFER secondaryBuffer;
	LPDIRECTSOUNDNOTIFY notify;
	HANDLE notifyEvents[FPL__DIRECTSOUND_MAX_PERIODS];
	HANDLE stopEvent;
	uint32_t lastProcessedFrame;
	bool breakMainLoop;
} fpl__DirectSoundAudioState;

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

fpl_internal uint32_t fpl__GetDevicesDirectSound(fpl__DirectSoundAudioState *dsoundState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
	uint32_t result = 0;
	const fpl__DirectSoundApi *dsoundApi = &dsoundState->api;
	fpl__DirectSoundDeviceInfos infos = FPL_ZERO_INIT;
	infos.maxDeviceCount = maxDeviceCount;
	infos.deviceInfos = deviceInfos;
	dsoundApi->DirectSoundEnumerateA(fpl__GetDeviceCallbackDirectSound, &infos);
	result = infos.foundDeviceCount;
	return(result);
}

fpl_internal bool fpl__AudioReleaseDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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

	fpl__UnloadDirectSoundApi(&dsoundState->api);

	FPL_CLEAR_STRUCT(dsoundState);

	return true;
}

fpl_internal fplAudioResult fpl__AudioInitDirectSound(const fplAudioSettings *audioSettings, fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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
	fpl__DirectSoundApi *dsoundApi = &dsoundState->api;
	if (!fpl__LoadDirectSoundApi(dsoundApi)) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_ApiFailed;
	}

	// Load direct sound object
	const GUID *deviceId = fpl_null;
	if (fplGetAnsiStringLength(audioSettings->deviceInfo.name) > 0) {
		deviceId = &audioSettings->deviceInfo.id.dshow;
	}
	if (!SUCCEEDED(dsoundApi->DirectSoundCreate(deviceId, &dsoundState->directSound, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_NoDeviceFound;
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
#	if defined(FPL_ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_Window) {
		windowHandle = appState->window.win32.windowHandle;
	}
#	endif
	if (windowHandle == fpl_null) {
		windowHandle = apiFuncs->user.GetDesktopWindow();
	}

	// The cooperative level must be set before doing anything else
	if (FAILED(IDirectSound_SetCooperativeLevel(dsoundState->directSound, windowHandle, (audioSettings->preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create primary buffer
	DSBUFFERDESC descDSPrimary = FPL_ZERO_INIT;
	descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
	descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	if (FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDSPrimary, &dsoundState->primaryBuffer, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Set format
	if (FAILED(IDirectSoundBuffer_SetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)&wf))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get the required size in bytes
	DWORD requiredSize;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, fpl_null, 0, &requiredSize))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get actual format
	char rawdata[1024];
	WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)pActualFormat, requiredSize, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
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
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
	if (FAILED(IDirectSoundBuffer_QueryInterface(dsoundState->secondaryBuffer, guid_IID_IDirectSoundNotify, (void**)&dsoundState->notify))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Setup notifications
	uint32_t periodSizeInBytes = internalFormat.bufferSizeInBytes / internalFormat.periods;
	DSBPOSITIONNOTIFY notifyPoints[FPL__DIRECTSOUND_MAX_PERIODS];
	for (uint32_t i = 0; i < internalFormat.periods; ++i) {
		dsoundState->notifyEvents[i] = CreateEventA(fpl_null, false, false, fpl_null);
		if (dsoundState->notifyEvents[i] == fpl_null) {
			fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
			return fplAudioResult_Failed;
		}

		// The notification offset is in bytes.
		notifyPoints[i].dwOffset = i * periodSizeInBytes;
		notifyPoints[i].hEventNotify = dsoundState->notifyEvents[i];
	}
	if (FAILED(IDirectSoundNotify_SetNotificationPositions(dsoundState->notify, internalFormat.periods, notifyPoints))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create stop event
	dsoundState->stopEvent = CreateEventA(fpl_null, false, false, fpl_null);
	if (dsoundState->stopEvent == fpl_null) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	return fplAudioResult_Success;
}

fpl_internal_inline void fpl__AudioStopMainLoopDirectSound(fpl__DirectSoundAudioState *dsoundState) {
	dsoundState->breakMainLoop = true;
	SetEvent(dsoundState->stopEvent);
}

fpl_internal bool fpl__GetCurrentFrameDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState, uint32_t* pCurrentPos) {
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

fpl_internal uint32_t fpl__GetAvailableFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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

fpl_internal uint32_t fpl__WaitForFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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

fpl_internal bool fpl__AudioStopDirectSound(fpl__DirectSoundAudioState *dsoundState) {
	FPL_ASSERT(dsoundState->secondaryBuffer != fpl_null);
	if (FAILED(IDirectSoundBuffer_Stop(dsoundState->secondaryBuffer))) {
		return false;
	}
	IDirectSoundBuffer_SetCurrentPosition(dsoundState->secondaryBuffer, 0);
	return true;
}

fpl_internal fplAudioResult fpl__AudioStartDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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

fpl_internal void fpl__AudioRunMainLoopDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
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
				fpl__PushError("Failed to lock directsound secondary buffer '%p' for offset/size (%lu / %lu)!", dsoundState->secondaryBuffer, lockOffset, lockSize);
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

// ############################################################################
//
// > AUDIO_DRIVER_ALSA
//
// Based on mini_al.h
//
// ############################################################################
#if defined(FPL_ENABLE_AUDIO_ALSA)
#	include <alsa/asoundlib.h>

#define FPL__ALSA_FUNC_snd_pcm_open(name) int name(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode)
typedef FPL__ALSA_FUNC_snd_pcm_open(fpl__alsa_func_snd_pcm_open);
#define FPL__ALSA_FUNC_snd_pcm_close(name) int name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_close(fpl__alsa_func_snd_pcm_close);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_sizeof(name) size_t name(void)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_sizeof(fpl__alsa_func_snd_pcm_hw_params_sizeof);
#define FPL__ALSA_FUNC_snd_pcm_hw_params(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params(fpl__alsa_func_snd_pcm_hw_params);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_any(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_any(fpl__alsa_func_snd_pcm_hw_params_any);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_format(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_format(fpl__alsa_func_snd_pcm_hw_params_set_format);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_format_first(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t *format)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_format_first(fpl__alsa_func_snd_pcm_hw_params_set_format_first);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_format_mask(name) void name(snd_pcm_hw_params_t *params, snd_pcm_format_mask_t *mask)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_format_mask(fpl__alsa_func_snd_pcm_hw_params_get_format_mask);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_channels_near(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_channels_near(fpl__alsa_func_snd_pcm_hw_params_set_channels_near);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_rate_resample(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_rate_resample(fpl__alsa_func_snd_pcm_hw_params_set_rate_resample);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_rate_near(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_rate_near(fpl__alsa_func_snd_pcm_hw_params_set_rate_near);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_buffer_size_near(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_buffer_size_near(fpl__alsa_func_snd_pcm_hw_params_set_buffer_size_near);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_periods_near(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_periods_near(fpl__alsa_func_snd_pcm_hw_params_set_periods_near);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_set_access(name) int name(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_set_access(fpl__alsa_func_snd_pcm_hw_params_set_access);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_format(name) int name(const snd_pcm_hw_params_t *params, snd_pcm_format_t *val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_format(fpl__alsa_func_snd_pcm_hw_params_get_format);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_channels(name) int name(const snd_pcm_hw_params_t *params, unsigned int *val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_channels(fpl__alsa_func_snd_pcm_hw_params_get_channels);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_rate(name) int name(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_rate(fpl__alsa_func_snd_pcm_hw_params_get_rate);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_buffer_size(name) int name(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_buffer_size(fpl__alsa_func_snd_pcm_hw_params_get_buffer_size);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_periods(name) int name(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_periods(fpl__alsa_func_snd_pcm_hw_params_get_periods);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_access(name) int name(const snd_pcm_hw_params_t *params, snd_pcm_access_t *_access)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_access(fpl__alsa_func_snd_pcm_hw_params_get_access);
#define FPL__ALSA_FUNC_snd_pcm_hw_params_get_sbits(name) int name(const snd_pcm_hw_params_t *params)
typedef FPL__ALSA_FUNC_snd_pcm_hw_params_get_sbits(fpl__alsa_func_snd_pcm_hw_params_get_sbits);
#define FPL__ALSA_FUNC_snd_pcm_sw_params_sizeof(name) size_t name(void)
typedef FPL__ALSA_FUNC_snd_pcm_sw_params_sizeof(fpl__alsa_func_snd_pcm_sw_params_sizeof);
#define FPL__ALSA_FUNC_snd_pcm_sw_params_current(name) int name(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
typedef FPL__ALSA_FUNC_snd_pcm_sw_params_current(fpl__alsa_func_snd_pcm_sw_params_current);
#define FPL__ALSA_FUNC_snd_pcm_sw_params_set_avail_min(name) int name(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
typedef FPL__ALSA_FUNC_snd_pcm_sw_params_set_avail_min(fpl__alsa_func_snd_pcm_sw_params_set_avail_min);
#define FPL__ALSA_FUNC_snd_pcm_sw_params_set_start_threshold(name) int name(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
typedef FPL__ALSA_FUNC_snd_pcm_sw_params_set_start_threshold(fpl__alsa_func_snd_pcm_sw_params_set_start_threshold);
#define FPL__ALSA_FUNC_snd_pcm_sw_params(name) int name(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
typedef FPL__ALSA_FUNC_snd_pcm_sw_params(fpl__alsa_func_snd_pcm_sw_params);
#define FPL__ALSA_FUNC_snd_pcm_format_mask_sizeof(name) size_t name(void)
typedef FPL__ALSA_FUNC_snd_pcm_format_mask_sizeof(fpl__alsa_func_snd_pcm_format_mask_sizeof);
#define FPL__ALSA_FUNC_snd_pcm_format_mask_test(name) int name(const snd_pcm_format_mask_t *mask, snd_pcm_format_t val)
typedef FPL__ALSA_FUNC_snd_pcm_format_mask_test(fpl__alsa_func_snd_pcm_format_mask_test);
#define FPL__ALSA_FUNC_snd_pcm_get_chmap(name) snd_pcm_chmap_t *name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_get_chmap(fpl__alsa_func_snd_pcm_get_chmap);
#define FPL__ALSA_FUNC_snd_pcm_prepare(name) int name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_prepare(fpl__alsa_func_snd_pcm_prepare);
#define FPL__ALSA_FUNC_snd_pcm_start(name) int name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_start(fpl__alsa_func_snd_pcm_start);
#define FPL__ALSA_FUNC_snd_pcm_drop(name) int name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_drop(fpl__alsa_func_snd_pcm_drop);
#define FPL__ALSA_FUNC_snd_device_name_hint(name) int name(int card, const char *iface, void ***hints)
typedef FPL__ALSA_FUNC_snd_device_name_hint(fpl__alsa_func_snd_device_name_hint);
#define FPL__ALSA_FUNC_snd_device_name_get_hint(name) char *name(const void *hint, const char *id)
typedef FPL__ALSA_FUNC_snd_device_name_get_hint(fpl__alsa_func_snd_device_name_get_hint);
#define FPL__ALSA_FUNC_snd_card_get_index(name) int name(const char *name)
typedef FPL__ALSA_FUNC_snd_card_get_index(fpl__alsa_func_snd_card_get_index);
#define FPL__ALSA_FUNC_snd_device_name_free_hint(name) int name(void **hints)
typedef FPL__ALSA_FUNC_snd_device_name_free_hint(fpl__alsa_func_snd_device_name_free_hint);
#define FPL__ALSA_FUNC_snd_pcm_mmap_begin(name) int name(snd_pcm_t *pcm, const snd_pcm_channel_area_t **areas, snd_pcm_uframes_t *offset, snd_pcm_uframes_t *frames)
typedef FPL__ALSA_FUNC_snd_pcm_mmap_begin(fpl__alsa_func_snd_pcm_mmap_begin);
#define FPL__ALSA_FUNC_snd_pcm_mmap_commit(name) snd_pcm_sframes_t name(snd_pcm_t *pcm, snd_pcm_uframes_t offset, snd_pcm_uframes_t frames)
typedef FPL__ALSA_FUNC_snd_pcm_mmap_commit(fpl__alsa_func_snd_pcm_mmap_commit);
#define FPL__ALSA_FUNC_snd_pcm_recover(name) int name(snd_pcm_t *pcm, int err, int silent)
typedef FPL__ALSA_FUNC_snd_pcm_recover(fpl__alsa_func_snd_pcm_recover);
#define FPL__ALSA_FUNC_snd_pcm_writei(name) snd_pcm_sframes_t name(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
typedef FPL__ALSA_FUNC_snd_pcm_writei(fpl__alsa_func_snd_pcm_writei);
#define FPL__ALSA_FUNC_snd_pcm_avail(name) snd_pcm_sframes_t name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_avail(fpl__alsa_func_snd_pcm_avail);
#define FPL__ALSA_FUNC_snd_pcm_avail_update(name) snd_pcm_sframes_t name(snd_pcm_t *pcm)
typedef FPL__ALSA_FUNC_snd_pcm_avail_update(fpl__alsa_func_snd_pcm_avail_update);
#define FPL__ALSA_FUNC_snd_pcm_wait(name) int name(snd_pcm_t *pcm, int timeout)
typedef FPL__ALSA_FUNC_snd_pcm_wait(fpl__alsa_func_snd_pcm_wait);

typedef struct fpl__AlsaAudioApi {
	void *libHandle;
	fpl__alsa_func_snd_pcm_open *snd_pcm_open;
	fpl__alsa_func_snd_pcm_close *snd_pcm_close;
	fpl__alsa_func_snd_pcm_hw_params_sizeof *snd_pcm_hw_params_sizeof;
	fpl__alsa_func_snd_pcm_hw_params *snd_pcm_hw_params;
	fpl__alsa_func_snd_pcm_hw_params_any *snd_pcm_hw_params_any;
	fpl__alsa_func_snd_pcm_hw_params_set_format *snd_pcm_hw_params_set_format;
	fpl__alsa_func_snd_pcm_hw_params_set_format_first *snd_pcm_hw_params_set_format_first;
	fpl__alsa_func_snd_pcm_hw_params_get_format_mask *snd_pcm_hw_params_get_format_mask;
	fpl__alsa_func_snd_pcm_hw_params_set_channels_near *snd_pcm_hw_params_set_channels_near;
	fpl__alsa_func_snd_pcm_hw_params_set_rate_resample *snd_pcm_hw_params_set_rate_resample;
	fpl__alsa_func_snd_pcm_hw_params_set_rate_near *snd_pcm_hw_params_set_rate_near;
	fpl__alsa_func_snd_pcm_hw_params_set_buffer_size_near *snd_pcm_hw_params_set_buffer_size_near;
	fpl__alsa_func_snd_pcm_hw_params_set_periods_near *snd_pcm_hw_params_set_periods_near;
	fpl__alsa_func_snd_pcm_hw_params_set_access *snd_pcm_hw_params_set_access;
	fpl__alsa_func_snd_pcm_hw_params_get_format *snd_pcm_hw_params_get_format;
	fpl__alsa_func_snd_pcm_hw_params_get_channels *snd_pcm_hw_params_get_channels;
	fpl__alsa_func_snd_pcm_hw_params_get_rate *snd_pcm_hw_params_get_rate;
	fpl__alsa_func_snd_pcm_hw_params_get_buffer_size *snd_pcm_hw_params_get_buffer_size;
	fpl__alsa_func_snd_pcm_hw_params_get_periods *snd_pcm_hw_params_get_periods;
	fpl__alsa_func_snd_pcm_hw_params_get_access *snd_pcm_hw_params_get_access;
	fpl__alsa_func_snd_pcm_hw_params_get_sbits *snd_pcm_hw_params_get_sbits;
	fpl__alsa_func_snd_pcm_sw_params_sizeof *snd_pcm_sw_params_sizeof;
	fpl__alsa_func_snd_pcm_sw_params_current *snd_pcm_sw_params_current;
	fpl__alsa_func_snd_pcm_sw_params_set_avail_min *snd_pcm_sw_params_set_avail_min;
	fpl__alsa_func_snd_pcm_sw_params_set_start_threshold *snd_pcm_sw_params_set_start_threshold;
	fpl__alsa_func_snd_pcm_sw_params *snd_pcm_sw_params;
	fpl__alsa_func_snd_pcm_format_mask_sizeof *snd_pcm_format_mask_sizeof;
	fpl__alsa_func_snd_pcm_format_mask_test *snd_pcm_format_mask_test;
	fpl__alsa_func_snd_pcm_get_chmap *snd_pcm_get_chmap;
	fpl__alsa_func_snd_pcm_prepare *snd_pcm_prepare;
	fpl__alsa_func_snd_pcm_start *snd_pcm_start;
	fpl__alsa_func_snd_pcm_drop *snd_pcm_drop;
	fpl__alsa_func_snd_device_name_hint *snd_device_name_hint;
	fpl__alsa_func_snd_device_name_get_hint *snd_device_name_get_hint;
	fpl__alsa_func_snd_card_get_index *snd_card_get_index;
	fpl__alsa_func_snd_device_name_free_hint *snd_device_name_free_hint;
	fpl__alsa_func_snd_pcm_mmap_begin *snd_pcm_mmap_begin;
	fpl__alsa_func_snd_pcm_mmap_commit *snd_pcm_mmap_commit;
	fpl__alsa_func_snd_pcm_recover *snd_pcm_recover;
	fpl__alsa_func_snd_pcm_writei *snd_pcm_writei;
	fpl__alsa_func_snd_pcm_avail *snd_pcm_avail;
	fpl__alsa_func_snd_pcm_avail_update *snd_pcm_avail_update;
	fpl__alsa_func_snd_pcm_wait *snd_pcm_wait;
} fpl__AlsaAudioApi;

typedef struct fpl__AlsaAudioState {
	fpl__AlsaAudioApi api;
	snd_pcm_t* pcmDevice;
	void *intermediaryBuffer;
	bool isUsingMMap;
	bool breakMainLoop;
} fpl__AlsaAudioState;

fpl_internal void fpl__UnloadAlsaApi(fpl__AlsaAudioApi *alsaApi) {
	FPL_ASSERT(alsaApi != fpl_null);
	if (alsaApi->libHandle != fpl_null) {
		dlclose(alsaApi->libHandle);
	}
	FPL_CLEAR_STRUCT(alsaApi);
}

fpl_internal bool fpl__LoadAlsaApi(fpl__AlsaAudioApi *alsaApi) {
	FPL_ASSERT(alsaApi != fpl_null);
	const char* libraryNames[] = {
		"libasound.so",
	};
	bool result = false;
	for (uint32_t index = 0; index < FPL_ARRAYCOUNT(libraryNames); ++index) {
		const char * libName = libraryNames[index];
		void *libHandle = alsaApi->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if (libHandle != fpl_null) {
			do {
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_open, fpl__alsa_func_snd_pcm_open, "snd_pcm_open");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_close, fpl__alsa_func_snd_pcm_close, "snd_pcm_close");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_sizeof, fpl__alsa_func_snd_pcm_hw_params_sizeof, "snd_pcm_hw_params_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params, fpl__alsa_func_snd_pcm_hw_params, "snd_pcm_hw_params");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_any, fpl__alsa_func_snd_pcm_hw_params_any, "snd_pcm_hw_params_any");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_format, fpl__alsa_func_snd_pcm_hw_params_set_format, "snd_pcm_hw_params_set_format");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_format_first, fpl__alsa_func_snd_pcm_hw_params_set_format_first, "snd_pcm_hw_params_set_format_first");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_format_mask, fpl__alsa_func_snd_pcm_hw_params_get_format_mask, "snd_pcm_hw_params_get_format_mask");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_channels_near, fpl__alsa_func_snd_pcm_hw_params_set_channels_near, "snd_pcm_hw_params_set_channels_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_rate_resample, fpl__alsa_func_snd_pcm_hw_params_set_rate_resample, "snd_pcm_hw_params_set_rate_resample");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_rate_near, fpl__alsa_func_snd_pcm_hw_params_set_rate_near, "snd_pcm_hw_params_set_rate_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_buffer_size_near, fpl__alsa_func_snd_pcm_hw_params_set_buffer_size_near, "snd_pcm_hw_params_set_buffer_size_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_periods_near, fpl__alsa_func_snd_pcm_hw_params_set_periods_near, "snd_pcm_hw_params_set_periods_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_set_access, fpl__alsa_func_snd_pcm_hw_params_set_access, "snd_pcm_hw_params_set_access");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_format, fpl__alsa_func_snd_pcm_hw_params_get_format, "snd_pcm_hw_params_get_format");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_channels, fpl__alsa_func_snd_pcm_hw_params_get_channels, "snd_pcm_hw_params_get_channels");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_rate, fpl__alsa_func_snd_pcm_hw_params_get_rate, "snd_pcm_hw_params_get_rate");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_buffer_size, fpl__alsa_func_snd_pcm_hw_params_get_buffer_size, "snd_pcm_hw_params_get_buffer_size");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_periods, fpl__alsa_func_snd_pcm_hw_params_get_periods, "snd_pcm_hw_params_get_periods");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_access, fpl__alsa_func_snd_pcm_hw_params_get_access, "snd_pcm_hw_params_get_access");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_hw_params_get_sbits, fpl__alsa_func_snd_pcm_hw_params_get_sbits, "snd_pcm_hw_params_get_sbits");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_sw_params_sizeof, fpl__alsa_func_snd_pcm_sw_params_sizeof, "snd_pcm_sw_params_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_sw_params_current, fpl__alsa_func_snd_pcm_sw_params_current, "snd_pcm_sw_params_current");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_sw_params_set_avail_min, fpl__alsa_func_snd_pcm_sw_params_set_avail_min, "snd_pcm_sw_params_set_avail_min");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_sw_params_set_start_threshold, fpl__alsa_func_snd_pcm_sw_params_set_start_threshold, "snd_pcm_sw_params_set_start_threshold");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_sw_params, fpl__alsa_func_snd_pcm_sw_params, "snd_pcm_sw_params");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_format_mask_sizeof, fpl__alsa_func_snd_pcm_format_mask_sizeof, "snd_pcm_format_mask_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_format_mask_test, fpl__alsa_func_snd_pcm_format_mask_test, "snd_pcm_format_mask_test");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_get_chmap, fpl__alsa_func_snd_pcm_get_chmap, "snd_pcm_get_chmap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_prepare, fpl__alsa_func_snd_pcm_prepare, "snd_pcm_prepare");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_start, fpl__alsa_func_snd_pcm_start, "snd_pcm_start");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_drop, fpl__alsa_func_snd_pcm_drop, "snd_pcm_drop");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_device_name_hint, fpl__alsa_func_snd_device_name_hint, "snd_device_name_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_device_name_get_hint, fpl__alsa_func_snd_device_name_get_hint, "snd_device_name_get_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_card_get_index, fpl__alsa_func_snd_card_get_index, "snd_card_get_index");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_device_name_free_hint, fpl__alsa_func_snd_device_name_free_hint, "snd_device_name_free_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_mmap_begin, fpl__alsa_func_snd_pcm_mmap_begin, "snd_pcm_mmap_begin");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_mmap_commit, fpl__alsa_func_snd_pcm_mmap_commit, "snd_pcm_mmap_commit");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_recover, fpl__alsa_func_snd_pcm_recover, "snd_pcm_recover");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_writei, fpl__alsa_func_snd_pcm_writei, "snd_pcm_writei");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_avail, fpl__alsa_func_snd_pcm_avail, "snd_pcm_avail");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_avail_update, fpl__alsa_func_snd_pcm_avail_update, "snd_pcm_avail_update");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(libHandle, libName, alsaApi->snd_pcm_wait, fpl__alsa_func_snd_pcm_wait, "snd_pcm_wait");
				result = true;
			} while (0);
			if (result) {
				break;
			}
		}
		fpl__UnloadAlsaApi(alsaApi);
	}
	return(result);
}

fpl_internal uint32_t fpl__AudioWaitForFramesAlsa(const fplAudioDeviceFormat *deviceFormat, fpl__AlsaAudioState *alsaState, bool *requiresRestart) {
	FPL_ASSERT(commonAudio != fpl_null && deviceFormat != fpl_null);
	if (requiresRestart != fpl_null) {
		*requiresRestart = false;
	}
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	uint32_t periodSizeInFrames = deviceFormat->bufferSizeInFrames / deviceFormat->periods;
	while (!alsaState->breakMainLoop) {
		const int timeoutInMilliseconds = 10;
		int waitResult = alsaApi->snd_pcm_wait(alsaState->pcmDevice, timeoutInMilliseconds);
		if (waitResult < 0) {
			if (waitResult == -EPIPE) {
				if (alsaApi->snd_pcm_recover(alsaState->pcmDevice, waitResult, 1) < 0) {
					return 0;
				}
				if (requiresRestart != fpl_null) {
					*requiresRestart = true;
				}
			}
		}

		if (alsaState->breakMainLoop) {
			return 0;
		}

		snd_pcm_sframes_t framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
		if (framesAvailable < 0) {
			if (framesAvailable == -EPIPE) {
				if (alsaApi->snd_pcm_recover(alsaState->pcmDevice, framesAvailable, 1) < 0) {
					return 0;
				}
				if (requiresRestart != fpl_null) {
					*requiresRestart = true;
				}
				framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
				if (framesAvailable < 0) {
					return 0;
				}
			}
		}

		// Keep the returned number of samples consistent and based on the period size.
		if (framesAvailable >= periodSizeInFrames) {
			return periodSizeInFrames;
		}

		// We'll get here if the loop was terminated. Just return whatever's available.
		framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
		if (framesAvailable < 0) {
			return 0;
		}
		return framesAvailable;
	}
}

fpl_internal bool fpl__GetAudioFramesFromClientAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(commonAudio != fpl_null && alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;

	if (!fpl__IsAudioDeviceStarted(commonAudio) && fpl__AudioGetDeviceState(commonAudio) != fpl__AudioDeviceState_Starting) {
		return false;
	}
	if (alsaState->breakMainLoop) {
		return false;
	}

	if (alsaState->isUsingMMap) {
		// mmap path
		bool requiresRestart;
		uint32_t framesAvailable = fpl__AudioWaitForFramesAlsa(&commonAudio->internalFormat, alsaState, &requiresRestart);
		if (framesAvailable == 0) {
			return false;
		}
		if (alsaState->breakMainLoop) {
			return false;
		}

		const snd_pcm_channel_area_t* channelAreas;
		snd_pcm_uframes_t mappedOffset;
		snd_pcm_uframes_t mappedFrames = framesAvailable;
		while (framesAvailable > 0) {
			int result = alsaApi->snd_pcm_mmap_begin(alsaState->pcmDevice, &channelAreas, &mappedOffset, &mappedFrames);
			if (result < 0) {
				return false;
			}
			if (mappedFrames > 0) {
				void *bufferPtr = (uint8_t *)channelAreas[0].addr + ((channelAreas[0].first + (mappedOffset * channelAreas[0].step)) / 8);
				fpl__ReadAudioFramesFromClient(commonAudio, mappedFrames, bufferPtr);
			}
			result = alsaApi->snd_pcm_mmap_commit(alsaState->pcmDevice, mappedOffset, mappedFrames);
			if (result < 0 || (snd_pcm_uframes_t)result != mappedFrames) {
				alsaApi->snd_pcm_recover(alsaState->pcmDevice, result, 1);
				return false;
			}
			framesAvailable -= mappedFrames;
			if (requiresRestart) {
				if (alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
					return false;
				}
			}
		}
	} else {
		// readi/writei path
		while (!alsaState->breakMainLoop) {
			uint32_t framesAvailable = fpl__AudioWaitForFramesAlsa(&commonAudio->internalFormat, alsaState, fpl_null);
			if (framesAvailable == 0) {
				continue;
			}
			if (alsaState->breakMainLoop) {
				return false;
			}
			fpl__ReadAudioFramesFromClient(commonAudio, framesAvailable, alsaState->intermediaryBuffer);
			snd_pcm_sframes_t framesWritten = alsaApi->snd_pcm_writei(alsaState->pcmDevice, alsaState->intermediaryBuffer, framesAvailable);
			if (framesWritten < 0) {
				if (framesWritten == -EAGAIN) {
					// Keep trying
					continue;
				} else if (framesWritten == -EPIPE) {
					// Underrun -> Recover and try again
					if (alsaApi->snd_pcm_recover(alsaState->pcmDevice, framesWritten, 1) < 0) {
						FPL_LOG("ALSA", "Failed to recover device after underrun!");
						return false;
					}
					framesWritten = alsaApi->snd_pcm_writei(alsaState->pcmDevice, alsaState->intermediaryBuffer, framesAvailable);
					if (framesWritten < 0) {
						FPL_LOG("ALSA", "Failed to write data to the PCM device!");
						return false;
					}
					// Success
					break;
				} else {
					FPL_LOG("ALSA", "Failed to write audio frames from client, error code: %d!", framesWritten);
					return false;
				}
			} else {
				// Success
				break;
			}
		}
	}
	return true;
}

fpl_internal_inline void fpl__AudioStopMainLoopAlsa(fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(alsaState != fpl_null);
	alsaState->breakMainLoop = true;
}

fpl_internal bool fpl__AudioReleaseAlsa(const fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(commonAudio != fpl_null && alsaState != fpl_null);
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (alsaState->pcmDevice != fpl_null) {
		alsaApi->snd_pcm_close(alsaState->pcmDevice);
		alsaState->pcmDevice = fpl_null;
		if (alsaState->intermediaryBuffer != fpl_null) {
			fplMemoryFree(alsaState->intermediaryBuffer);
			alsaState->intermediaryBuffer = fpl_null;
		}
	}
	fpl__UnloadAlsaApi(alsaApi);
	FPL_CLEAR_STRUCT(alsaState);
	return true;
}

fpl_internal fplAudioResult fpl__AudioStartAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(commonAudio != fpl_null && alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;

	// Prepare the device
	if (alsaApi->snd_pcm_prepare(alsaState->pcmDevice) < 0) {
		FPL_LOG("ALSA", "Failed to prepare PCM device '%p'!", alsaState->pcmDevice);
		return fplAudioResult_Failed;
	}

	// Get initial frames to fill from the client
	if (!fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
		FPL_LOG("ALSA", "Failed to get initial audio frames from client!");
		return fplAudioResult_Failed;
	}

	if (alsaState->isUsingMMap) {
		if (alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
			FPL_LOG("ALSA", "Failed to start PCM device '%p'!", alsaState->pcmDevice);
			return fplAudioResult_Failed;
		}
	}

	return fplAudioResult_Success;
}

fpl_internal bool fpl__AudioStopAlsa(fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (alsaApi->snd_pcm_drop(alsaState->pcmDevice)) {
		FPL_LOG("ALSA", "Failed to drop the PCM device '%p'!", alsaState->pcmDevice);
		return false;
	}
	return true;
}

fpl_internal_inline snd_pcm_format_t fpl__MapAudioFormatToAlsaFormat(fplAudioFormatType format) {
	switch (format) {
		case fplAudioFormatType_U8:
			return SND_PCM_FORMAT_U8;
		case fplAudioFormatType_S16:
			return SND_PCM_FORMAT_S16_LE;
		case fplAudioFormatType_S24:
			return SND_PCM_FORMAT_S24_3LE;
		case fplAudioFormatType_S32:
			return SND_PCM_FORMAT_S32_LE;
		case fplAudioFormatType_F32:
			return SND_PCM_FORMAT_FLOAT_LE;
		default:
			return SND_PCM_FORMAT_UNKNOWN;
	}
}

fpl_internal void fpl__AudioRunMainLoopAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(alsaState != fpl_null);
	alsaState->breakMainLoop = false;
	while (!alsaState->breakMainLoop && fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
	}
}

fpl_internal_inline fplAudioFormatType fpl__MapAlsaFormatToAudioFormat(snd_pcm_format_t format) {
	switch (format) {
		case SND_PCM_FORMAT_U8:
			return fplAudioFormatType_U8;
		case SND_PCM_FORMAT_S16_LE:
			return fplAudioFormatType_S16;
		case SND_PCM_FORMAT_S24_3LE:
			return fplAudioFormatType_S24;
		case SND_PCM_FORMAT_S32_LE:
			return fplAudioFormatType_S32;
		case SND_PCM_FORMAT_FLOAT_LE:
			return fplAudioFormatType_F32;
		default:
			return fplAudioFormatType_None;
	}
}

fpl_internal fplAudioResult fpl__AudioInitAlsa(const fplAudioSettings *audioSettings, fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
#	define FPL__ALSA_INIT_ERROR(ret, format, ...) do { \
		FPL_LOG("ALSA", format, __VA_ARGS__); \
		fpl__AudioReleaseAlsa(commonAudio, alsaState); \
		return fplAudioResult_Failed; \
	} while (0)

	// Load ALSA library
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (!fpl__LoadAlsaApi(alsaApi)) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_ApiFailed, "Failed loading ALSA api!");
	}

	//
	// Open PCM Device
	//
	char deviceName[256] = FPL_ZERO_INIT;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	if (fplGetAnsiStringLength(audioSettings->deviceInfo.name) > 0) {
		// @TODO(final): Support for forced audio device ALSA
		FPL__ALSA_INIT_ERROR(fplAudioResult_NoDeviceFound, "Forced audio device on ALSA is not supported yet!");
	} else {
		const char *defaultDeviceNames[16];
		int defaultDeviceCount = 0;
		defaultDeviceNames[defaultDeviceCount++] = "default";
		if (!audioSettings->preferExclusiveMode) {
			defaultDeviceNames[defaultDeviceCount++] = "dmix";
			defaultDeviceNames[defaultDeviceCount++] = "dmix:0";
			defaultDeviceNames[defaultDeviceCount++] = "dmix:0,0";
		}
		defaultDeviceNames[defaultDeviceCount++] = "hw";
		defaultDeviceNames[defaultDeviceCount++] = "hw:0";
		defaultDeviceNames[defaultDeviceCount++] = "hw:0,0";

		bool isDeviceOpen = false;
		for (size_t defaultDeviceIndex = 0; defaultDeviceIndex < defaultDeviceCount; ++defaultDeviceIndex) {
			const char *defaultDeviceName = defaultDeviceNames[defaultDeviceIndex];
			FPL_LOG("ALSA", "Opening PCM audio device '%s'", defaultDeviceName);
			if (alsaApi->snd_pcm_open(&alsaState->pcmDevice, defaultDeviceName, stream, 0) == 0) {
				FPL_LOG("ALSA", "Successfully opened PCM audio device '%s'", defaultDeviceName);
				isDeviceOpen = true;
				fplCopyAnsiString(defaultDeviceName, deviceName, FPL_ARRAYCOUNT(deviceName));
				break;
			} else {
				FPL_LOG("ALSA", "Failed opening PCM audio device '%s'!", defaultDeviceName);
			}
		}
		if (!isDeviceOpen) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_NoDeviceFound, "No PCM audio device found!");
		}
	}

	//
	// Get hardware parameters
	//
	FPL_ASSERT(alsaState->pcmDevice != fpl_null);
	FPL_ASSERT(fplGetAnsiStringLength(deviceName) > 0);

	FPL_LOG("ALSA", "Get hardware parameters from device '%s'", deviceName);
	size_t hardwareParamsSize = alsaApi->snd_pcm_hw_params_sizeof();
	snd_pcm_hw_params_t *hardwareParams = (snd_pcm_hw_params_t *)FPL_STACKALLOCATE(hardwareParamsSize);
	fplMemoryClear(hardwareParams, hardwareParamsSize);
	if (alsaApi->snd_pcm_hw_params_any(alsaState->pcmDevice, hardwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed getting hardware parameters from device '%s'!", deviceName);
	}
	FPL_LOG("ALSA", "Successfullyy got hardware parameters from device '%s'", deviceName);

	//
	// Access mode (Interleaved MMap or Standard readi/writei)
	//
	alsaState->isUsingMMap = false;
	if (!audioSettings->specific.alsa.noMMap) {
		if (alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_MMAP_INTERLEAVED) == 0) {
			alsaState->isUsingMMap = true;
		} else {
			FPL_LOG("ALSA", "Failed setting MMap access mode for device '%s', trying fallback to standard mode!", deviceName);
		}
	}
	if (!alsaState->isUsingMMap) {
		if (alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting default access mode for device '%s'!", deviceName);
		}
	}

	//
	// Get format
	//

	// Get all supported formats
	size_t formatMaskSize = alsaApi->snd_pcm_format_mask_sizeof();
	snd_pcm_format_mask_t *formatMask = (snd_pcm_format_mask_t *)FPL_STACKALLOCATE(formatMaskSize);
	fplMemoryClear(formatMask, formatMaskSize);
	alsaApi->snd_pcm_hw_params_get_format_mask(hardwareParams, formatMask);

	snd_pcm_format_t foundFormat;
	snd_pcm_format_t preferredFormat = fpl__MapAudioFormatToAlsaFormat(audioSettings->deviceFormat.type);
	if (!alsaApi->snd_pcm_format_mask_test(formatMask, preferredFormat)) {
		// The required format is not supported. Try a list of default formats.
		snd_pcm_format_t defaultFormats[] = {
			SND_PCM_FORMAT_FLOAT_LE,
			SND_PCM_FORMAT_S32_LE,
			SND_PCM_FORMAT_S24_3LE,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_FORMAT_U8,
		};
		foundFormat = SND_PCM_FORMAT_UNKNOWN;
		for (size_t defaultFormatIndex = 0; defaultFormatIndex < FPL_ARRAYCOUNT(defaultFormats); ++defaultFormatIndex) {
			snd_pcm_format_t defaultFormat = defaultFormats[defaultFormatIndex];
			if (alsaApi->snd_pcm_format_mask_test(formatMask, defaultFormat)) {
				foundFormat = defaultFormat;
				break;
			}
		}
	} else {
		foundFormat = preferredFormat;
	}
	if (foundFormat == SND_PCM_FORMAT_UNKNOWN) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "No supported audio format for device '%s' found!", deviceName);
	}

	//
	// Set format
	//
	fplAudioFormatType internalFormatType = fpl__MapAlsaFormatToAudioFormat(foundFormat);
	if (alsaApi->snd_pcm_hw_params_set_format(alsaState->pcmDevice, hardwareParams, foundFormat) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM format '%s' for device '%s'!", fplGetAudioFormatString(internalFormatType), deviceName);
	}

	//
	// Set channels
	//
	uint32_t internalChannels = audioSettings->deviceFormat.channels;
	if (alsaApi->snd_pcm_hw_params_set_channels_near(alsaState->pcmDevice, hardwareParams, &internalChannels) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM channels '%lu' for device '%s'!", internalChannels, deviceName);
	}

	//
	// Set sample rate
	//

	// @NOTE(final): We disable the resampling of the sample rate to not get caught into any driver bugs
	alsaApi->snd_pcm_hw_params_set_rate_resample(alsaState->pcmDevice, hardwareParams, 0);

	uint32_t internalSampleRate = audioSettings->deviceFormat.sampleRate;
	if (alsaApi->snd_pcm_hw_params_set_rate_near(alsaState->pcmDevice, hardwareParams, &internalSampleRate, 0) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM sample rate '%lu' for device '%s'!", internalSampleRate, deviceName);
	}

	//
	// Set periods
	//
	uint32_t internalPeriods = audioSettings->deviceFormat.periods;
	int periodsDir = 0;
	if (alsaApi->snd_pcm_hw_params_set_periods_near(alsaState->pcmDevice, hardwareParams, &internalPeriods, &periodsDir) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM periods '%lu' for device '%s'!", internalPeriods, deviceName);
	}

	//
	// Set buffer size
	//
	snd_pcm_uframes_t actualBufferSize = audioSettings->deviceFormat.bufferSizeInFrames;
	if (alsaApi->snd_pcm_hw_params_set_buffer_size_near(alsaState->pcmDevice, hardwareParams, &actualBufferSize) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM buffer size '%lu' for device '%s'!", actualBufferSize, deviceName);
	}
	uint32_t internalBufferSizeInFrame = actualBufferSize;

	//
	// Set hardware parameters
	//
	if (alsaApi->snd_pcm_hw_params(alsaState->pcmDevice, hardwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to install PCM hardware parameters for device '%s'!", deviceName);
	}

	// Set internal format
	fplAudioDeviceFormat internalFormat = FPL_ZERO_INIT;
	internalFormat.type = internalFormatType;
	internalFormat.channels = internalChannels;
	internalFormat.sampleRate = internalSampleRate;
	internalFormat.periods = internalPeriods;
	internalFormat.bufferSizeInFrames = internalBufferSizeInFrame;
	internalFormat.bufferSizeInBytes = internalFormat.bufferSizeInFrames * internalFormat.channels * fplGetAudioSampleSizeInBytes(internalFormat.type);
	commonAudio->internalFormat = internalFormat;

	//
	// Software parameters
	//
	size_t softwareParamsSize = alsaApi->snd_pcm_sw_params_sizeof();
	snd_pcm_sw_params_t *softwareParams = (snd_pcm_sw_params_t *)FPL_STACKALLOCATE(softwareParamsSize);
	fplMemoryClear(softwareParams, softwareParamsSize);
	if (alsaApi->snd_pcm_sw_params_current(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to get software parameters for device '%s'!", deviceName);
	}
	snd_pcm_uframes_t minAvailableFrames = (internalFormat.sampleRate / 1000) * 1;
	if (alsaApi->snd_pcm_sw_params_set_avail_min(alsaState->pcmDevice, softwareParams, minAvailableFrames) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to set software available min for device '%s'!", deviceName);
	}
	if (!alsaState->isUsingMMap) {
		if (alsaApi->snd_pcm_sw_params_set_start_threshold(alsaState->pcmDevice, softwareParams, minAvailableFrames) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to set start threshold of '%lu' for device '%s'!", minAvailableFrames, deviceName);
		}
	}
	if (alsaApi->snd_pcm_sw_params(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to install PCM software parameters for device '%s'!", deviceName);
	}

	if (!alsaState->isUsingMMap) {
		alsaState->intermediaryBuffer = fplMemoryAllocate(internalFormat.bufferSizeInBytes);
		if (alsaState->intermediaryBuffer == fpl_null) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed allocating intermediary buffer of size '%lu' for device '%s'!", internalFormat.bufferSizeInBytes, deviceName);
		}
	}

	// @NOTE(final): We do not ALSA support channel mapping right know, so we limit it to mono or stereo
	FPL_ASSERT(internalFormat.channels <= 2);

#undef FPL__ALSA_INIT_ERROR

	return fplAudioResult_Success;
}

#endif // FPL_ENABLE_AUDIO_ALSA

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
typedef struct fpl__AudioState {
	fpl__CommonAudioState common;

	fplMutexHandle lock;
	fplThreadHandle *workerThread;
	fplSignalHandle startSignal;
	fplSignalHandle stopSignal;
	fplSignalHandle wakeupSignal;
	volatile fplAudioResult workResult;

	fplAudioDriverType activeDriver;
	uint32_t periods;
	uint32_t bufferSizeInFrames;
	uint32_t bufferSizeInBytes;
	bool isAsyncDriver;

	union {
#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		fpl__DirectSoundAudioState dsound;
#	endif
#	if defined(FPL_ENABLE_AUDIO_ALSA)
		fpl__AlsaAudioState alsa;
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

fpl_internal void fpl__StopAudioDeviceMainLoop(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			fpl__AudioStopMainLoopDirectSound(&audioState->dsound);
		} break;
#	endif

#	if defined(FPL_ENABLE_AUDIO_ALSA)
		case fplAudioDriverType_Alsa:
		{
			fpl__AudioStopMainLoopAlsa(&audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal bool fpl__ReleaseAudioDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	bool result = false;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__AudioReleaseDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL_ENABLE_AUDIO_ALSA)
		case fplAudioDriverType_Alsa:
		{
			result = fpl__AudioReleaseAlsa(&audioState->common, &audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal bool fpl__StopAudioDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	bool result = false;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__AudioStopDirectSound(&audioState->dsound);
		} break;
#	endif

#	if defined(FPL_ENABLE_AUDIO_ALSA)
		case fplAudioDriverType_Alsa:
		{
			result = fpl__AudioStopAlsa(&audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal fplAudioResult fpl__StartAudioDevice(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	fplAudioResult result = fplAudioResult_Failed;
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			result = fpl__AudioStartDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL_ENABLE_AUDIO_ALSA)
		case fplAudioDriverType_Alsa:
		{
			result = fpl__AudioStartAlsa(&audioState->common, &audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal void fpl__RunAudioDeviceMainLoop(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	switch (audioState->activeDriver) {

#	if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioDriverType_DirectSound:
		{
			fpl__AudioRunMainLoopDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL_ENABLE_AUDIO_ALSA)
		case fplAudioDriverType_Alsa:
		{
			fpl__AudioRunMainLoopAlsa(&audioState->common, &audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal_inline bool fpl__IsAudioDriverAsync(fplAudioDriverType audioDriver) {
	switch (audioDriver) {
		case fplAudioDriverType_DirectSound:
		case fplAudioDriverType_Alsa:
			return false;
		default:
			return false;
	}
}

fpl_internal_inline void fpl__AudioSetDeviceState(fpl__CommonAudioState *audioState, fpl__AudioDeviceState newState) {
	fplAtomicStoreU32((volatile uint32_t *)&audioState->state, (uint32_t)newState);
}

fpl_internal fpl__AudioDeviceState fpl__AudioGetDeviceState(fpl__CommonAudioState *audioState) {
	fpl__AudioDeviceState result = (fpl__AudioDeviceState)fplAtomicLoadU32((volatile uint32_t *)&audioState->state);
	return(result);
}

fpl_internal bool fpl__IsAudioDeviceInitialized(fpl__CommonAudioState *audioState) {
	if (audioState == fpl_null) {
		return false;
	}
	fpl__AudioDeviceState state = fpl__AudioGetDeviceState(audioState);
	return(state != fpl__AudioDeviceState_Uninitialized);
}

fpl_internal bool fpl__IsAudioDeviceStarted(fpl__CommonAudioState *audioState) {
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
	fpl__CommonAudioState *commonAudioState = &audioState->common;
	FPL_ASSERT(audioState != fpl_null);
	FPL_ASSERT(audioState->activeDriver != fplAudioDriverType_None);

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.CoInitializeEx(fpl_null, 0);
#endif

	for (;;) {
		// Stop the device at the start of the iteration always
		fpl__StopAudioDevice(audioState);

		// Let the other threads know that the device has been stopped.
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Stopped);
		fplSignalSet(&audioState->stopSignal);

		// We wait until the audio device gets wake up
		fplSignalWaitForOne(&audioState->wakeupSignal, FPL_TIMEOUT_INFINITE);

		// Default result code.
		audioState->workResult = fplAudioResult_Success;

		// Just break if we're terminating.
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Uninitialized) {
			break;
		}

		// Expect that the device is currently be started by the client
		FPL_ASSERT(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting);

		// Start audio device
		audioState->workResult = fpl__StartAudioDevice(audioState);
		if (audioState->workResult != fplAudioResult_Success) {
			fplSignalSet(&audioState->startSignal);
			continue;
		}

		// The audio device is started, mark it as such
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Started);
		fplSignalSet(&audioState->startSignal);

		// Enter audio device main loop
		fpl__RunAudioDeviceMainLoop(audioState);
	}

	// Signal to stop any audio threads, in case there are some waiting
	fplSignalSet(&audioState->stopSignal);

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.CoUninitialize();
#endif
}

fpl_internal void fpl__ReleaseAudio(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState != fpl_null);

#if defined(FPL_PLATFORM_WIN32)
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (fpl__IsAudioDeviceInitialized(commonAudioState)) {

		// Wait until the audio device is stopped
		if (fpl__IsAudioDeviceStarted(commonAudioState)) {
			while (fplStopAudio() == fplAudioResult_DeviceBusy) {
				fplThreadSleep(1);
			}
		}

		// Putting the device into an uninitialized state will make the worker thread return.
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Uninitialized);

		// Wake up the worker thread and wait for it to properly terminate.
		fplSignalSet(&audioState->wakeupSignal);

		fplThreadWaitForOne(audioState->workerThread, FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(audioState->workerThread);

		// Release signals and thread
		fplSignalDestroy(&audioState->stopSignal);
		fplSignalDestroy(&audioState->startSignal);
		fplSignalDestroy(&audioState->wakeupSignal);
		fplMutexDestroy(&audioState->lock);

		// Release audio device
		fpl__ReleaseAudioDevice(audioState);

		// Clear audio state
		FPL_CLEAR_STRUCT(audioState);
	}

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.CoUninitialize();
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
	wapi->ole.CoInitializeEx(fpl_null, 0);
#endif

	// Create mutex and signals
	if (!fplMutexInit(&audioState->lock)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if (!fplSignalInit(&audioState->wakeupSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if (!fplSignalInit(&audioState->startSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if (!fplSignalInit(&audioState->stopSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	// Prope drivers
	fplAudioDriverType propeDrivers[16];
	uint32_t driverCount = 0;
	if (audioSettings->driver == fplAudioDriverType_Auto) {
		// @NOTE(final): Add all audio drivers here, regardless of the platform.
		propeDrivers[driverCount++] = fplAudioDriverType_DirectSound;
		propeDrivers[driverCount++] = fplAudioDriverType_Alsa;
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
				initResult = fpl__AudioInitDirectSound(audioSettings, &audioState->common, &audioState->dsound);
				if (initResult != fplAudioResult_Success) {
					fpl__AudioReleaseDirectSound(&audioState->common, &audioState->dsound);
				}
			} break;
#		endif

#		if defined(FPL_ENABLE_AUDIO_ALSA)
			case fplAudioDriverType_Alsa:
			{
				initResult = fpl__AudioInitAlsa(audioSettings, &audioState->common, &audioState->alsa);
				if (initResult != fplAudioResult_Success) {
					fpl__AudioReleaseAlsa(&audioState->common, &audioState->alsa);
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
		fplSignalWaitForOne(&audioState->stopSignal, FPL_TIMEOUT_INFINITE);
	} else {
		fpl__AudioSetDeviceState(&audioState->common, fpl__AudioDeviceState_Stopped);
	}

	FPL_ASSERT(fpl__AudioGetDeviceState(&audioState->common) == fpl__AudioDeviceState_Stopped);

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
#	endif
} fpl__Win32VideoState;
#endif // FPL_PLATFORM_WIN32

#if defined(FPL_SUBPLATFORM_X11)
typedef struct fpl__X11VideoState {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
	fpl__X11VideoOpenGLState opengl;
#	endif
} fpl__X11VideoState;
#endif // FPL_SUBPLATFORM_X11

typedef struct fpl__VideoState {
	fplVideoDriverType activeDriver;
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	fplVideoBackBuffer softwareBackbuffer;
#	endif

#	if defined(FPL_PLATFORM_WIN32)
	fpl__Win32VideoState win32;
#	endif
#	if defined(FPL_SUBPLATFORM_X11)
	fpl__X11VideoState x11;
#	endif
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
	FPL_ASSERT(appState != fpl_null);
	if (videoState != fpl_null) {
		switch (videoState->activeDriver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_PLATFORM_WIN32)
				fpl__Win32ReleaseVideoOpenGL(&videoState->win32.opengl);
#			elif defined(FPL_SUBPLATFORM_X11)
				fpl__X11ReleaseVideoOpenGL(&appState->x11, &appState->window.x11, &videoState->x11.opengl);
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
					result = fpl__X11SetPreWindowSetupForOpenGL(&appState->x11.api, &appState->window.x11, &videoState->x11.opengl, &outResult->x11);
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

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Uninitialized) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock);
	{
		// Check if the device is already stopped
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopping) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}

		// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Stopping);

		if (audioState->isAsyncDriver) {
			// Asynchronous drivers (Has their own thread)
			fpl__StopAudioDevice(audioState);
		} else {
			// Synchronous drivers

			// The audio worker thread is most likely in a wait state, so let it stop properly.
			fpl__StopAudioDeviceMainLoop(audioState);

			// We need to wait for the worker thread to become available for work before returning.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the stopped state.
			fplSignalWaitForOne(&audioState->stopSignal, FPL_TIMEOUT_INFINITE);
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

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (!fpl__IsAudioDeviceInitialized(commonAudioState)) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock);
	{
		// Be a bit more descriptive if the device is already started or is already in the process of starting.
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}

		// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Starting);

		if (audioState->isAsyncDriver) {
			// Asynchronous drivers (Has their own thread)
			fpl__StartAudioDevice(audioState);
			fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Started);
		} else {
			// Synchronous drivers
			fplSignalSet(&audioState->wakeupSignal);

			// Wait for the worker thread to finish starting the device.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the started state.
			fplSignalWaitForOne(&audioState->startSignal, FPL_TIMEOUT_INFINITE);
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
		if (fpl__AudioGetDeviceState(&audioState->common) == fpl__AudioDeviceState_Stopped) {
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
			default:
				break;
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
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	bool result = false;
	if (videoState != fpl_null) {
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		if (videoState->activeDriver == fplVideoDriverType_Software) {
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
				fplWindowSize area;
				if (fplGetWindowArea(&area)) {
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
						wapi->gdi.StretchDIBits(win32WindowState->deviceContext, 0, 0, area.width, area.height, 0, 0, 0, 0, fpl_null, fpl_null, DIB_RGB_COLORS, BLACKNESS);
					}
					wapi->gdi.StretchDIBits(win32WindowState->deviceContext, targetX, targetY, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, backbuffer->pixels, &software->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
				}
			} break;
#		endif

#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
				wapi->gdi.SwapBuffers(win32WindowState->deviceContext);
			} break;
#		endif

			default:
				break;
		}
#   elif defined(FPL_SUBPLATFORM_X11)
		const fpl__X11WindowState *x11WinState = &appState->window.x11;
		switch (appState->currentSettings.video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
				const fpl__X11VideoOpenGLApi *glApi = &videoState->x11.opengl.api;
				glApi->glXSwapBuffers(x11WinState->display, x11WinState->window);
			} break;
#		endif

			default:
				break;
		}
#	endif // FPL_PLATFORM || FPL_SUBPLATFORM
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
	FPL_ASSERT(initState != fpl_null);

	// Release audio
#	if defined(FPL_ENABLE_AUDIO)
	{
		FPL_LOG("Core", "Release Audio");
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		if (audioState != fpl_null) {
			// @TODO(final): Rename to ShutdownAudio?
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

	// @TODO(final): Release audio state here?

	if (appState != fpl_null) {
		// Release actual platform (There can only be one platform!)
		{
#		if defined(FPL_PLATFORM_WIN32)
			FPL_LOG("Core", "Release Win32 Platform");
			fpl__Win32ReleasePlatform(initState, appState);
#		elif defined(FPL_PLATFORM_LINUX)
			FPL_LOG("Core", "Release Linux Platform");
			fpl__LinuxReleasePlatform(initState, appState);
#		elif defined(FPL_PLATFORM_UNIX)
			FPL_LOG("Core", "Release Unix Platform");
			fpl__UnixReleasePlatform(initState, appState);
#		endif
		}

		// Release sub platforms
		{
#		if defined(FPL_SUBPLATFORM_X11)
			FPL_LOG("Core", "Release X11 Subplatform");
			fpl__X11ReleaseSubplatform(&appState->x11);
#		endif
#		if defined(FPL_SUBPLATFORM_POSIX)
			FPL_LOG("Core", "Release POSIX Subplatform");
			fpl__PosixReleaseSubplatform(&appState->posix);
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

fpl_common_api fplInitResultType fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {
	// Exit out if platform is already initialized
	if (fpl__global__InitState.isInitialized) {
		fpl__PushError("Platform is already initialized");
		return fplInitResultType_AlreadyInitialized;
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
		return fplInitResultType_FailedAllocatingMemory;
	}

	fpl__PlatformAppState *appState = fpl__global__AppState = (fpl__PlatformAppState *)platformAppStateMemory;
	appState->initFlags = initFlags;
	if (initSettings != fpl_null) {
		appState->initSettings = *initSettings;
	} else {
		fplSetDefaultSettings(&appState->initSettings);
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
#	if !defined(FPL_ENABLE_WINDOW)
	appState->initFlags = (fplInitFlags)(appState->initFlags & ~fplInitFlags_Window);
#	endif

	// Initialize sub-platforms
#	if defined(FPL_SUBPLATFORM_POSIX)
	{
		FPL_LOG("Core", "Initialize POSIX Subplatform:");
		if (!fpl__PosixInitSubplatform(initFlags, initSettings, &initState->posix, &appState->posix)) {
			FPL_LOG("Core", "Failed initializing POSIX Subplatform!");
			fpl__PushError("Failed initializing POSIX Subplatform");
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedPlatform;
		}
		FPL_LOG("Core", "Successfully initialized POSIX Subplatform");
	}
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
	{
		FPL_LOG("Core", "Initialize X11 Subplatform:");
		if (!fpl__X11InitSubplatform(&appState->x11)) {
			FPL_LOG("Core", "Failed initializing X11 Subplatform!");
			fpl__PushError("Failed initializing X11 Subplatform");
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedPlatform;
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
#   elif defined(FPL_PLATFORM_UNIX)
	isInitialized = fpl__UnixInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	endif

	if (!isInitialized) {
		FPL_LOG("Core", "Failed initializing %s Platform!", FPL_PLATFORM_NAME);
		fpl__ReleasePlatformStates(initState, appState);
		return fplInitResultType_FailedPlatform;
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
				return fplInitResultType_FailedVideo;
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
			return fplInitResultType_FailedWindow;
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
			return fplInitResultType_FailedVideo;
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
		if (fpl__InitAudio(&appState->initSettings.audio, audioState) != fplAudioResult_Success) {
			FPL_LOG("Core", "Failed initializing Audio Driver '%s'!", audioDriverName);
			fpl__PushError("Failed initialization audio with settings (Driver=%s, Format=%s, SampleRate=%d, Channels=%d, BufferSize=%d)", audioDriverName, fplGetAudioFormatString(initSettings->audio.deviceFormat.type), initSettings->audio.deviceFormat.sampleRate, initSettings->audio.deviceFormat.channels);
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedAudio;
		}
		FPL_LOG("Core", "Successfully initialized Audio Driver '%s'", audioDriverName);
	}
#	endif // FPL_ENABLE_AUDIO

	initState->isInitialized = true;
	return fplInitResultType_Success;
}

fpl_common_api fplPlatformType fplGetPlatformType() {
	fplPlatformType result;
#if defined(FPL_PLATFORM_WIN32)
	result = fplPlatformType_Windows;
#elif defined(FPL_PLATFORM_LINUX)
	result = fplPlatformType_Linux;
#elif defined(FPL_PLATFORM_UNIX)
	result = fplPlatformType_Unix;
#else
	result = fplPlatformType_Unknown;
#endif
	return(result);
}

#endif // FPL_SYSTEM_INIT_DEFINED

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > NO_CRT_IMPL
//
// This block contains entry points required when CRT is disabled.
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL_NO_CRT) && !defined(FPL_NO_ENTRYPOINT)

// ****************************************************************************
//
// Win32-Platform (No-CRT)
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)

#	if defined(FPL_APPTYPE_WINDOW)

#		if defined(UNICODE)
void __stdcall wWinMainCRTStartup(void) {
	LPWSTR argsW = GetCommandLineW();
	int result = wWinMain(GetModuleHandleW(fpl_null), fpl_null, argsW, SW_SHOW);
	ExitProcess(result);
}
#		else
void __stdcall WinMainCRTStartup(void) {
	LPSTR argsA = GetCommandLineA();
	int result = WinMain(GetModuleHandleA(fpl_null), fpl_null, argsA, SW_SHOW);
	ExitProcess(result);
}
#		endif // UNICODE

#	elif defined(FPL_APPTYPE_CONSOLE)
void __stdcall mainCRTStartup(void) {
	fpl__Win32CommandLineUTF8Arguments args;
#	if defined(UNICODE)
	LPWSTR argsW = GetCommandLineW();
	args = fpl__Win32ParseWideArguments(argsW);
#	else
	LPSTR argsA = GetCommandLineA();
	args = fpl__Win32ParseAnsiArguments(argsA);
#	endif
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	ExitProcess(result);
}
#	else
#		error "Application type not set!"
#	endif // FPL_APPTYPE

#else

// ****************************************************************************
//
// Non-Win32 Platforms (No CRT)
//
// ****************************************************************************
// @TODO(final): Support for not using libC in linux as well

#endif // FPL_PLATFORM

#endif // FPL_NO_CRT

#endif // FPL_IMPLEMENTATION && !FPL_IMPLEMENTED

// end-of-file