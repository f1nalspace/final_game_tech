/*
final_platform_layer.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

Final Platform Layer is a Single-Header-File cross-platform C development library designed to abstract the underlying platform to a simple and easy to use api - providing low level access to (Window, Video, Audio, Input, File/Path IO, Threads, Memory, Hardware, etc.).

The main focus is game/media/simulation development, so the default settings will create a window, setup a OpenGL rendering context and initialize audio playback on any platform.

It is written in C99 for simplicity and best portability, but is C++ compatible as well.

FPL supports the platforms Windows/Linux for the architectures x86/x64.

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
	\version v0.9.0.0 beta
	\author Torsten Spaete
	\brief Final Platform Layer (FPL) - A C99 Single-Header-File Platform Abstraction Library
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

## v0.9.0.0 beta:
	- Changed: fplKey_Enter renamed to fplKey_Return
	- Changed: fplKey_LeftWin renamed to fplKey_LeftSuper
	- Changed: fplKey_RightWin renamed to fplKey_RightSuper
	- Changed: fplKey_Plus renamed to fplKey_OemPlus
	- Changed: fplKey_Minus renamed to fplKey_OemMinus
	- Changed: fplKey_Comma renamed to fplKey_OemComma
	- Changed: fplKey_Period renamed to fplKey_OemPeriod
	- Changed: fplKeyboardModifierFlags_Alt are split into left/right part respectively
	- Changed: fplKeyboardModifierFlags_Shift are split into left/right part respectively
	- Changed: fplKeyboardModifierFlags_Super are split into left/right part respectively
	- Changed: fplKeyboardModifierFlags_Ctrl are split into left/right part respectively
	- Changed: All bool fields in structs are replaced with fpl_b32
	- Changed: Added COUNTER macro to non-CRT FPL_STATICASSERT
	- Changed: Renamed fields in fplMemoryInfos to match correct meaning
	- Changed: String copy functions uses fplMemoryCopy instead of iterating and copy each char
	- Changed: fplSetFilePosition32 returns now uint32_t instead of void
    - Changed: Added field timeStamps to fplFileEntry
	- Changed: [X11] Window title uses XChangeProperty now instead of XStoreName
	- Changed: [Win32] Detection of left/right keyboard modifier flags
	- Changed: [Win32] Mapping OEM 1-8 keys
	- Changed: [Win32] Use MapVirtualKeyA to map key code to virtual key
	- Fixed: Corrected a ton of comments
	- Fixed: fpl__HandleKeyboardButtonEvent had incorrect previous state mapping
	- Fixed: [X11] fplMouseEventType_Move event was never created anymore
	- New: Added typedef fpl_b32 (32-bit boolean type)
	- New: Added struct fplKeyboardState
	- New: Added struct fplGamepadStates
	- New: Added function fplGetKeyboardState()
	- New: Added function fplGetSystemLocale()
	- New: Added function fplGetUserLocale()
	- New: Added function fplGetInputLocale()
	- New: Added function fplGetGamepadStates()
	- New: Added function fplKey_Oem1-fplKey_Oem8
	- New: Added function fplGetFileTimestampsFromPath
	- New: Added function fplGetFileTimestampsFromHandle
	- New: Added internal function fpl__ParseTextFile used for parsing /proc/cpuinfo or other device files
	- New: Added function fplGetFileSizeFromHandle
	- New: Added function fplGetFileSizeFromHandle64
	- New: Added function fplGetFileSizeFromPath
	- New: Added function fplGetFileSizeFromPath64
	- New: Added function fplGetFilePosition
	- New: Added function fplGetFilePosition64
	- New: Added function fplSetFilePosition
	- New: Added function fplSetFilePosition64
	- New: Added function fplWriteFileBlock
	- New: Added function fplWriteFileBlock64
	- New: Added function fplReadFileBlock
	- New: Added function fplReadFileBlock64
	- New: [Win32] Implemented fplGetKeyboardState
	- New: [Win32] Implemented fplGetGamepadStates
	- New: [Win32] Implemented fplGetSystemLocale
	- New: [Win32] Implemented fplGetUserLocale
	- New: [Win32] Implemented fplGetInputLocale
	- New: [Win32] Implemented fplGetFileTimestampsFromPath
	- New: [Win32] Implemented fplGetFileTimestampsFromHandle
	- New: [Win32] Implemented fplReadFileBlock64
	- New: [Win32] Implemented fplWriteFileBlock64
	- New: [Win32] Implemented fplSetFilePosition64
	- New: [Win32] Implemented fplGetFilePosition64
	- New: [Win32] Implemented fplGetFileSizeFromPath64
	- New: [Win32] Implemented fplGetFileSizeFromHandle64
	- New: [X11] Implemented fplGetKeyboardState
	- New: [X11] Set window icon title using XChangeProperty
	- New: [X11] Give window our process id
	- New: [X11] Load window icons on startup
	- New: [X11] Added ping support for letting the WM wakeup the window
	- New: [Linux] Implemented fplGetSystemLocale
	- New: [Linux] Implemented fplGetUserLocale
	- New: [Linux] Implemented fplGetInputLocale
	- New: [Linux] Reading first joystick as game controller in linux
	- New: [POSIX] Implemented fplGetFileTimestampsFromPath
	- New: [POSIX] Implemented fplGetFileTimestampsFromHandle
	- New: [POSIX] Implemented fplReadFileBlock64
	- New: [POSIX] Implemented fplWriteFileBlock64
	- New: [POSIX] Implemented fplSetFilePosition64
	- New: [POSIX] Implemented fplGetFilePosition64
	- New: [POSIX] Implemented fplGetFileSizeFromPath64
	- New: [POSIX] Implemented fplGetFileSizeFromHandle64

	## v0.8.4.0 beta:
	- New: Added macro function FPL_STRUCT_INIT
	- New: Added enum value fplWindowEventType_DropSingleFile
	- New: Added structure fplWindowDropFiles
	- New: Added enum value fplInitFlags_Console
	- New: [Win32] Support for WM_DROPFILES -> fplWindowEventType_DropSingleFile event
	- Changed: Move fplInitFlags_All out and made it a static constant
	- Changed: Use fplInitFlags_Console to activate console or not
	- Fixed: fplExtractFileExtension was returning wrong result for files with multiple separators
	- Fixed: [Win32] Fullscreen toggling was broken in maximize/minimize mode
	- Fixed: [Win32] ClientToScreen prototype was missing WINAPI call
	- Fixed: [POSIX] fplListDirNext() was broken
	- Fixed: [X11] Key up event was never fired (KeyRelease + KeyPress = Key-Repeat)

	## v0.8.3.0 beta:
	- Changed: fplVersionInfo is now parsed as char[4] instead of uint32_t
	- Changed: fplMouseEvent / fplKeyboardEvent uses fplButtonState instead of a bool for the state
	- Changed: Replaced fplMouseEventType_ButtonDown / fplMouseEventType_ButtonUp with fplMouseEventType_Button
	- Changed: Replaced fplKeyboardEventType_KeyDown / fplKeyboardEventType_KeyUp with fplKeyboardEventType_Button
	- Fixed: Fixed incompabilties with MingW compiler
	- New: Added function fplStringToS32
	- New: Added function fplStringToS32Len
	- New: Added function fplS32ToString
	- New: Added function fplAtomicLoadSize
	- New: Added function fplAtomicStoreSize
	- New: Added function fplAtomicExchangeSize
	- New: Added function fplAtomicCompareAndExchangeSize
	- New: Added function fplIsAtomicCompareAndExchangeSize
	- New: Added function fplAtomicAddSize
	- New: Added function fplAtomicIncU32
	- New: Added function fplAtomicIncU64
	- New: Added function fplAtomicIncS32
	- New: Added function fplAtomicIncS64
	- New: Added function fplAtomicIncSize
	- New: Added macro FPL_IS_POWEROFTWO
	- New: Introduced FPL_CPU_32BIT and FPL_CPU_64BIT
	- New: fplAtomic*Ptr uses FPL_CPU_ instead of FPL_ARCH_ now
	- New: Added enumeration fplButtonState
	- New: Added field multiSamplingCount to fplOpenGLVideoSettings

	- Changed: [Win32] Changed keyboard and mouse handling to use fplButtonState now
	- Changed: [Win32] Changed fplListDirBegin/fplListDirNext to ignore . and ..
	- Changed: [Win32] Console activation is done when _CONSOLE is set as well
	- Changed: [X11] Changed keyboard and mouse handling to use fplButtonState now
	- Fixed: [Win32] fplGetTimeInMillisecondsHP was not returning a proper double value
	- Fixed: [Win32] Fixed crash when using fplThreadTerminate while threads are already exiting
	- Fixed: [Win32] fplThreadWaitForAll/fplThreadWaitForAny was not working properly when threads was already in the process of exiting naturally
	- Fixed: [Win32] Fixed incompabilties with MingW compiler
	- Fixed: [POSIX] Fixed crash when using fplThreadTerminate while threads are already exiting
	- New: [Win32] Support for OpenGL multi sampling context creation
	- New: [GLX] Support for OpenGL multi sampling context creation

	## v0.8.2.0 beta:
	- Changed: Ensures const correctness on all functions
	- Changed: Signature of fplDynamicLibraryLoad changed (Returns bool + Target handle parameter)
	- Fixed: Corrected code documentation for all categories
	- Fixed: FPL_ENUM_AS_FLAGS_OPERATORS had invalid signature for operator overloadings and missed some operators
	- Fixed: Fixed fplMemorySet was not working for values > 0
	- New: Forward declare thread handle for thread callback
	- New: Added fplKey for OEM keys (Plus, Comma, Minus, Period)
	- New: Added fplKey for Media & Audio keys
	- New: Added fplWindowEventType_Maximized / fplWindowEventType_Minimized / fplWindowEventType_Restored
	- New: Added documentation category: Assertions & Debug
	- New: Added documentation category: Storage class identifiers
	- New: Added documentation category: Constants
	- New: Added documentation category: Function macros
	- New: Undef annoying defined constants such as None, Success, etc.
	- New: Added fplImageType enumeration
	- New: Added fplImageSource structure
	- New: Added icons as fplImageSource array to fplWindowSettings

	- Changed: [Win32] Correct fplDynamicLibraryLoad to match signature change
	- Changed: [Win32] Corrected function prototype macro names
	- Fixed: [Win32] ClientToScreen prototype was defined twice
	- New: [Win32] OEM keys mapping
	- New: [Win32] Media & Audio key mapping
	- New: [Win32] Handle window maximized/minimized and restored events
	- New: [Win32] Load small/big icon from the fplWindowSettings icon imagesources

	- Changed: [X11] Corrected function prototype macro names
	- New: [X11] OEM keys mapping

	- Changed: [X11] Corrected function prototype macro names
	- Changed: [POSIX] Correct fplDynamicLibraryLoad to match signature change
	- Changed: [POSIX] Use of pthread_timedjoin_np to support timeout in fplThreadWaitForOne when available (GNU extension)

	## v0.8.1.0 beta:
	- Changed: Locked error states to multiple and removed FPL_NO_MULTIPLE_ERRORSTATES
	- Changed: Renamed fplClearPlatformErrors() to fplClearErrors()
	- Changed: Renamed fplGetPlatformErrorCount() to fplGetErrorCount()
	- Changed: Renamed fplGetPlatformErrorFromIndex() to fplGetErrorByIndex()
	- Changed: Renamed fplGetPlatformError() to fplGetLastError()
	- Changed: Refactored logging system
	- Changed: Updated comments
	- Fixed: fplAnsiString<->WideString conversion enforces ANSI locale (Non Win32)
	- New: Added enum fplLogLevel
	- New: Added enum fplLogWriterFlags
	- New: Added struct fplLogWriter
	- New: Added struct fplLogSettings
	- New: Added struct fplSemaphoreHandle
	- New: Added fplSetLogSettings()
	- New: Added fplGetLogSettings()
	- New: Added fplSetMaxLogLevel()
	- New: Added fplGetMaxLogLevel()
	- New: Added fplSemaphoreInit()
	- New: Added fplSemaphoreDestroy()
	- New: Added fplSemaphoreWait()
	- New: Added fplSemaphoreTryWait()
	- New: Added fplSemaphoreValue()
	- New: Added fplSemaphoreRelease()
	- New: Added fplIsStringMatchWildcard()

	- Changed: [ALSA] Allow user selection of audio device id
	- Fixed: [Win32] WaitForMultipleObjects >= WAIT_OBJECT_0 bugfix
	- New: [ALSA] Implemented fplGetAudioDevices
	- New: [Win32] Implemented Semaphores
	- New: [POSIX] Implemented Semaphores

	## v0.8.0.0 beta:
	- Changed: Changed from inline to api call for fplGetAudioBufferSizeInFrames/fplGetAudioFrameSizeInBytes/fplGetAudioBufferSizeInBytes
	- Changed: Changed from inline to api call for fplGetArchTypeString / fplGetInitResultTypeString / fplGetPlatformTypeString
	- Changed: Moved FPL_CLEAR_STRUCT to public api
	- New: Added fplThreadYield()

	- Changed: [GLX] Added types such as XVisualInfo directly without relying on glx.h
	- Fixed: [Win32] Console window was not working anymore the second time fplPlatformInit was called
	- Fixed: [GLX] XVisualInfo was manually defined, now we use Xutil.h
	- New: [Win32] Implemented fplThreadYield()
	- New: [POSIX] Implemented fplThreadYield()
	- New: [Linux] Implemented fplGetRunningArchitecture()
	- New: [Linux] Implemented fplGetOperatingSystemInfos()
	- New: [X11] Implemented Video Software output for X11


	## v0.7.8.0 beta:
	- Changed: Collapsed down all argument checking using macros
	- Changed: Use FPL_CLEAR_STRUCT only when it is appropriate
	- Changed: All public checks for fpl__global__AppState returns proper error now
	- Changed: fplGetAudioHardwareFormat returns bool and requires a outFormat argument now
	- Changed: fplSetAudioClientReadCallback returns bool now
	- Changed: fplListFiles* is renamed to fplListDir*
	- Changed: fplListDir* argument for fplFileEntry is renamed to entry for all 3 functions
	- Changed. fplFileEntry stores the fullPath instead of the name + internal root infos	- Changed: Introduced fplFilePermissions in fplFileEntry
	- Changed: Removed flag fplFileAttributeFlags_ReadOnly from fplFileAttributeFlags
	- Fixed: Fixed a ton of wrong inline definitions
	- Fixed: Fixed GCC warning -Wwrite-strings
	- Fixed: fplDebugBreak() was missing function braces for __debugbreak
	- New: Added fplEnforcePathSeparatorLen()
	- New: Added fplEnforcePathSeparator()
	- New: Added fileSize field to fplFileEntry
	- New: Added struct fplFilePermissions
	- New: Added enum fplFilePermissionMasks
	- New: Added fplDebugOut()
	- New: Added fplDebugFormatOut()
	- New: Added fplWindowShutdown()
	- New: Added macro FPL_STRUCT_SET

	- Changed: [POSIX] Removed all pthread checks, because there is a check for platform initialization now
	- Changed: [Win32] Changed fplListDir* to support fplFilePermissions
	- Changed: [Win32] Showing cursor does not clip cursor anymore
	- Fixed: [POSIX] Fixed a ton of C99 compile errors
	- New: [POSIX] Implemented fplListDirBegin
	- New: [POSIX] Implemented fplListDirNext
	- New: [POSIX] Implemented fplListDirEnd
	- New: [X11] Implemented fplWindowShutdown()
	- New: [Win32] Fill out fileSize for fplFileEntry in fplListDir*
	- New: [Win32] Implemented fplWindowShutdown()

	## v0.7.7.0 beta:
	- New: Added fplMutexTryLock()
	- New: Added fplMakeDefaultSettings()
	- New: Added fplStringAppend() / fplStringAppendLen()
	- New: Added fplDebugBreak()
	- Changed: Any string buffer writing functions returns the last written character now
	- Changed: Changed fplGetClipboardAnsiText() to return bool instead of char *
	- Changed: Changed fplGetClipboardWideText() to return bool instead of wchar_t *
	- Changed: Entry point definition implementation is now a separated block and controlled by FPL_ENTRYPOINT
	- Changed: MSVC compiler warnings are only disabled inside the implementation block
	- Fixed: Never detected Win32 Path separator (Wrong define check)
	- Fixed: MSVC compiler warnings was overwritten always, now uses push/pop
	- Fixed: MSVC _Interlocked* functions has no signature for unsigned, so we use either LONG or LONG64

	- New: [X11] Implemented fplIsWindowFullscreen
	- New: [X11] Implemented basic fplSetWindowFullscreen
	- Fixed: [POSIX] Create/Open*BinaryFile was wrong named
	- Fixed: [Win32] fplMemoryFree actually never freed any memory


	## v0.7.6.0 beta:
	- Changed: Renamed fplGetRunningArchitectureType to fplGetRunningArchitecture
	- Changed: Renamed fplThreadDestroy() to fplThreadTerminate() + signature changed (Returns bool)
	- Changed: fplSignalInit() + new parameter "initialValue"
	- Changed: All functions which uses timeout uses fplTimeoutValue instead of uint32_t
	- Changed: All string buffer writing functions returns the last written character instead
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

	\section section_platform_status_planned_platforms Planned Platforms

	- Rasberry Pi
	- Unix (BSD family)

	\section section_platform_status_optional_platforms Optional Platforms

	- MacOSX
*/

/*!
	\page page_todo ToDo / Planned (Top priority order)
	\tableofcontents

	\section section_todo_inprogress In progress

	- Input
		- Gamepad support (Linux)
		- Mapping OEM-Keys (X11)

	- Window
		- Toggle Resizable (X11)
		- Toggle Decorated (X11)
		- Toggle Floating (X11)
		- Show/Hide Cursor (X11)
		- Minimize/Maximize/Restore (X11)
		- Minimize/Maximize/Restore (Win32)

	- Application
		- Support icon image in gnome (X11)
		- Support icon title in gnome (X11)

	\section section_todo_planned Planned

	- Window
		- Realtime resize (fiber)

	- DLL-Export support

	- Networking (UDP, TCP)
		- [Win32] WinSock
		- [POSIX] Socket

	- Unicode-Support for commandline arguments (Win32)

	- Multimonitor-Support

	- Date/Time functions

	- Audio:
		- OpenAL audio driver
		- Support for channel mapping

	- Documentation
		- Window
		- Debug & Logging
		- Memory
		- Atomics
		- Threading (Syncronisation)
		- File-IO
		- Paths
		- Strings
		- Hardware

	- Threading
		- Signals (POSIX, Non-Linux)

	\section section_todo_optional Optional

	- Window:
		- Custom cursor from image (File/Memory)

	- Video:
		- [Win32] Direct3D
		- [Win32] Vulkan
		- [POSIX] Vulkan
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
#	define FPL_CPU_64BIT
#elif defined(_M_IX86) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#	define FPL_CPU_32BIT
#elif defined(__arm__) || defined(_M_ARM)
#	if defined(__aarch64__)
#		define FPL_ARCH_ARM64
#		define FPL_CPU_64BIT
#	else	
#		define FPL_ARCH_ARM32
#		define FPL_CPU_32BIT
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
#elif defined(__CC_ARM)
	//! ARM compiler detected
#	define FPL_COMPILER_ARM
#elif defined(_MSC_VER)
	//! Visual studio compiler detected
#	define FPL_COMPILER_MSVC
#else
	//! No compiler detected
#	define FPL_COMPILER_UNKNOWN
#endif // FPL_COMPILER

//
// Macros needs to set on certain compiler/platform configurations
//
#if defined(FPL_IS_C99) && defined(FPL_SUBPLATFORM_POSIX)
#   if defined(FPL_PLATFORM_LINUX)
		//! Enable features such as MAP_ANONYMOUS for mmap, clock_gettime, readlink, nanosleep etc.
#       define _GNU_SOURCE 1
#   elif defined(FPL_PLATFORM_UNIX)
#       if defined(FPL_SUBPLATFORM_BSD)
#           define _BSD_SOURCE
#       else
#           define _XOPEN_SOURCE 500
#       endif
#   endif
#endif

//
// Storage class identifiers
//

/**
  * \defgroup StorageClassIdents Storage class identifiers
  * \brief This category contains storage class identifiers, such as static, extern, inline, etc.
  * \{
  */

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

#if defined(FPL_IS_CPP)
	//! External call
#   define fpl_extern
#else
	//! External call
#   define fpl_extern extern
#endif

#if defined(FPL_API_AS_PRIVATE)
	//! Api call
#	define fpl_api static
#else
	//! Api call
#	define fpl_api fpl_extern
#endif // FPL_API_AS_PRIVATE

//! Platform api definition
#define fpl_platform_api fpl_api
//! Common api definition
#define fpl_common_api fpl_api
//! Main entry point api definition
#define fpl_main

//
// Force inline
//
#if defined(FPL_COMPILER_GCC) && (__GNUC__ >= 4)
	//! Force inline
#	define fpl_force_inline __attribute__((__always_inline__)) inline
#elif defined(FPL_COMPILER_MSVC) && (_MSC_VER >= 1200)
	//! Force inline
#	define fpl_force_inline __forceinline
#else
	//! Force inline
#	define fpl_force_inline inline
#endif

/** \}*/

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
// Include entry points always when its not disabled and implementation block is compiled in
//
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_NO_ENTRYPOINT)
#	define FPL_ENTRYPOINT
#endif

//
// Compiler settings
//
#if defined(FPL_COMPILER_MSVC)
	//! Change warnings for the entire implementation block (Pop is at the end of the implementation)
#	pragma warning( push )

	//! Disable noexcept compiler warning for C++
#	pragma warning( disable : 4577 )
	//! Disable "switch statement contains 'default' but no 'case' labels" compiler warning for C++
#	pragma warning( disable : 4065 )
	//! Disable "conditional expression is constant" warning
#	pragma warning( disable : 4127 )
	//! Disable "unreferenced formal parameter" warning
#	pragma warning( disable : 4100 )
	//! Disable "nonstandard extension used: nameless struct/union" warning
#	pragma warning( disable : 4201 )
	//! Disable "local variable is initialized but not referenced" warning
#	pragma warning( disable : 4189 )
	//! Disable "nonstandard extension used: non-constant aggregate initializer" warning
#	pragma warning( disable : 4204 )

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

// MingW compiler hacks
#if defined(FPL_COMPILER_MINGW)
#   if !defined(_WIN32_WINNT)
#       define _WIN32_WINNT 0x0600
#   endif //!_WIN32_WINNT
#endif // FPL_COMPILER_MINGW

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

#if defined(FPL_LOGGING)
	//! Enable logging
#   define FPL_ENABLE_LOGGING
#	if defined(FPL_LOG_MULTIPLE_WRITERS)
		//! Enable multiple writers
#		define FPL_ENABLE_LOG_MULTIPLE_WRITERS
#	endif
#endif

//
// Assertions & Debug
//

/**
  * \defgroup Debug Assertion & Debug
  * \brief This category contains assertion & debug macro functions
  * \{
  */

#if defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_ENABLE_C_ASSERT) && !defined(FPL_FORCE_ASSERTIONS)
#		include <assert.h>
		//! Runtime assert (C Runtime)
#		define FPL_ASSERT(exp) assert(exp)
		//! Static assert (C Runtime)
#		define FPL_STATICASSERT(exp) static_assert(exp, "static_assert")
#	else
		//! Runtime assert
#		define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
		//! Static assert
#		define FPL_STATICASSERT_(exp, line, counter) \
			int fpl_static_assert_##line_##counter(int static_assert_failed[(exp)?1:-1])
#		define FPL_STATICASSERT(exp) \
			FPL_STATICASSERT_(exp, __LINE__, __COUNTER__)
#	endif // FPL_ENABLE_C_ASSERT
#else
	//! Runtime assert disabled
#	define FPL_ASSERT(exp)
	//! Static assert disabled
#	define FPL_STATICASSERT(exp)
#endif // FPL_ENABLE_ASSERTIONS

//
// Debug-Break
// Based on: https://stackoverflow.com/questions/173618/is-there-a-portable-equivalent-to-debugbreak-debugbreak
//
#if defined(__has_builtin)
#	if __has_builtin(__builtin_debugtrap)
		//! Stop on a line in the debugger (Trap)
#		define fplDebugBreak() __builtin_debugtrap()
#	elif __has_builtin(__debugbreak)
		//! Stop on a line in the debugger (Break)
#		define fplDebugBreak() __debugbreak()
#	endif
#endif
#if !defined(fplDebugBreak)
#	if defined(FPL_COMPILER_MSVC) || defined(FPL_COMPILER_INTEL)
		//! Triggers a breakout in the debugger (MSVC/Intel)
#		define fplDebugBreak() __debugbreak()
#	elif defined(FPL_COMPILER_ARM)
		//! Triggers a breakout in the debugger (Arm)
#		define fplDebugBreak() __breakpoint(42)
#	elif defined(FPL_ARCH_X86) || defined(FPL_ARCH_X64)
		//! Triggers a breakout in the debugger (X86/64)
static fpl_force_inline void fplDebugBreak() { __asm__ __volatile__("int $03"); }
#	elif defined(__thumb__)
		//! Triggers a breakout in the debugger (ARM Thumb mode)
static fpl_force_inline void fplDebugBreak() { __asm__ __volatile__(".inst 0xde01"); }
#	elif defined(FPL_ARCH_ARM64)
		//! Triggers a breakout in the debugger (ARM64)
static fpl_force_inline void fplDebugBreak() { __asm__ __volatile__(".inst 0xd4200000"); }
#	elif defined(FPL_ARCH_ARM32)
		//! Triggers a breakout in the debugger (ARM32)
static fpl_force_inline void fplDebugBreak() { __asm__ __volatile__(".inst 0xe7f001f0"); }
#	elif defined(FPL_COMPILER_GCC)
		//! Triggers a breakout in the debugger (GCC)
#       define fplDebugBreak() __builtin_trap()
#	else
#		include	<signal.h>
#		if defined(SIGTRAP)
			//! Triggers a breakout in the debugger (Sigtrap)
#			define fplDebugBreak() raise(SIGTRAP)
#		else
			//! Triggers a breakout in the debugger (Sigabort)
#			define fplDebugBreak() raise(SIGABRT)
#		endif
#	endif
#endif

/** \}*/

//
// Types & Limits
//
#include <stdint.h> // uint32_t, ...
#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <limits.h> // UINT32_MAX, ...

//
// Test sizes
//
// @TODO(final): At the moment this is sufficient, but as soon as there are special CPUs detected, this will break!
#if defined(FPL_CPU_64BIT)
FPL_STATICASSERT(sizeof(uintptr_t) == sizeof(uint64_t));
FPL_STATICASSERT(sizeof(size_t) == sizeof(uint64_t));
#elif defined(FPL_CPU_32BIT)
FPL_STATICASSERT(sizeof(uintptr_t) == sizeof(uint32_t));
FPL_STATICASSERT(sizeof(size_t) == sizeof(uint32_t));
#endif

//
// Macro functions
//

/**
  * \defgroup Macros Function macros
  * \brief This category contains several useful macro functions
  * \{
  */

//! This will full-on crash when something is not implemented always.
#define FPL_NOT_IMPLEMENTED {*(int *)0 = 0xBAD;}

#if defined(FPL_IS_C99)
	//! Initialize a struct to zero (C99)
#	define FPL_ZERO_INIT {0}
	//! Sets a struct pointer to the given value (C99)
#	define FPL_STRUCT_SET(ptr, type, value) *(ptr) = (type)value
	//! Inits a struct by the given type (C99)
#	define FPL_STRUCT_INIT(type, ...) (type){## __VA_ARGS__}
#else
	//! Initialize a struct to zero (C++)
#	define FPL_ZERO_INIT {}
	//! Sets a struct pointer to the given value (C++)
#	define FPL_STRUCT_SET(ptr, type, value) *(ptr) = value
	//! Inits a struct by the given type (C++)
#	define FPL_STRUCT_INIT(type, ...) {__VA_ARGS__}
#endif

//! Clears the given struct pointer to zero
#define FPL_CLEAR_STRUCT(ptr) fplMemoryClear((void *)(ptr), sizeof(*(ptr)))

//! Returns the element count from a static array,
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//! Returns the offset in bytes to a field in a structure
#define FPL_OFFSETOF(type, field) ((size_t)(&(((type*)(0))->field)))

//! Returns the offset for the value to satisfy the given alignment boundary
#define FPL_ALIGNMENT_OFFSET(value, alignment) ( (((alignment) > 1) && (((value) & ((alignment) - 1)) != 0)) ? ((alignment) - ((value) & (alignment - 1))) : 0)           
//! Returns the given size extended to satisfy the given alignment boundary
#define FPL_ALIGNED_SIZE(size, alignment) (((size) > 0 && (alignment) > 0) ? ((size) + FPL_ALIGNMENT_OFFSET(size, alignment)) : (size))
//! Returns true when the given pointer address is aligned to the given alignment
#define FPL_IS_ALIGNED(ptr, alignment) (((uintptr_t)(const void *)(ptr)) % (alignment) == 0)
//! Returns true when the given value is a power of two value
#define FPL_IS_POWEROFTWO(value) (((value) != 0) && (((value) & (~(value) + 1)) == (value)))

//! Returns the smallest value
#define FPL_MIN(a, b) ((a) < (b) ? (a) : (b))
//! Returns the biggest value
#define FPL_MAX(a, b) ((a) > (b) ? (a) : (b))

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

/** \}*/

#if defined(FPL_IS_CPP)
	//! Macro for overloading enum operators in C++
#	define FPL_ENUM_AS_FLAGS_OPERATORS(etype) \
	inline etype operator | (etype a, etype b) { \
		return static_cast<etype>(static_cast<int>(a) | static_cast<int>(b)); \
	} \
	inline etype& operator |= (etype &a, etype b) { \
		return a = a | b; \
	} \
	inline etype operator & (etype a, etype b) { \
		return static_cast<etype>(static_cast<int>(a) & static_cast<int>(b)); \
	} \
	inline etype& operator &= (etype &a, etype b) { \
		return a = a & b; \
	} \
	inline etype operator ~ (etype a) { \
		return static_cast<etype>(~static_cast<int>(a)); \
	} \
	inline etype operator ^ (etype a, etype b) { \
		return static_cast<etype>(static_cast<int>(a) ^ static_cast<int>(b)); \
	} \
	inline etype& operator ^= (etype &a, etype b) { \
		return a = a ^ b; \
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
#	include <semaphore.h> // sem_t
#	include <dirent.h> // DIR, dirent
#endif // FPL_SUBPLATFORM_POSIX

#if defined(FPL_SUBPLATFORM_X11)
#   include <X11/X.h> // Window
#   include <X11/Xlib.h> // Display
#   include <X11/Xutil.h> // XVisualInfo
#   include <X11/Xatom.h> // XA_CARDINAL
#endif // FPL_SUBPLATFORM_X11

//
// Constants
//

/**
  * \defgroup Constants Constants
  * \brief This category contains constants
  * \{
  */

//! Null
#define fpl_null NULL
//! 32-bit boolean
typedef int32_t fpl_b32;

#if defined(FPL_PLATFORM_WIN32)
	//! Maximum length of a filename (Win32)
#	define FPL_MAX_FILENAME_LENGTH (MAX_PATH + 1)
	//! Maximum length of a path (Win32)
#	define FPL_MAX_PATH_LENGTH (MAX_PATH * 2 + 1)
	//! Path separator character (Win32)
#	define FPL_PATH_SEPARATOR '\\'
	//! File extension character (Win32)
#	define FPL_FILE_EXT_SEPARATOR '.'
#else
	//! Maximum length of a filename (Non win32)
#	define FPL_MAX_FILENAME_LENGTH (512 + 1)
	//! Maximum length of a path (Non win32)
#	define FPL_MAX_PATH_LENGTH (2048 + 1)
	//! Path separator character (Non win32)
#	define FPL_PATH_SEPARATOR '/'
	//! File extension character (Non win32)
#	define FPL_FILE_EXT_SEPARATOR '.'
#endif

/** \}*/

// ****************************************************************************
// ****************************************************************************
//
// API Declaration
//
// ****************************************************************************
// ****************************************************************************

// ----------------------------------------------------------------------------
/**
  * \defgroup Atomics Atomic operations
  * \brief This category contains functions for handling atomic operations, such as Add, Compare And/Or Exchange, Fences, Loads/Stores, etc.
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Inserts a memory read fence/barrier.
  * \note This will complete previous reads before future reads and prevents the compiler from reordering memory reads across this fence.
  */
fpl_platform_api void fplAtomicReadFence();
/**
  * \brief Inserts a memory write fence/barrier.
  * \note This will complete previous writes before future writes and prevents the compiler from reordering memory writes across this fence.
  */
fpl_platform_api void fplAtomicWriteFence();
/**
  * \brief Inserts a memory read/write fence/barrier.
  * \note This will complete previous reads/writes before future reads/writes and prevents the compiler from reordering memory access across this fence.
  */
fpl_platform_api void fplAtomicReadWriteFence();

/**
  * \brief Replaces a 32-bit unsigned integer with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
/**
  * \brief Replaces a 64-bit unsigned integer with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
/**
  * \brief Replaces a 32-bit signed integer with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value);
/**
  * \brief Replaces a 64-bit signed integer with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value);
/**
  * \brief Replaces a pointer with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value);
/**
  * \brief Replaces a size with the given value atomically.
  * \param target The target value to write into
  * \param value The source value used for exchange
  * \return Returns the initial value before the replacement.
  * \note Ensures that memory operations are completed in order.
  */
fpl_common_api size_t fplAtomicExchangeSize(volatile size_t *target, const size_t value);

/**
  * \brief Adds a 32-bit unsigned integer to the value by the given addend atomically.
  * \param value The target value to append to
  * \param addend The value used for adding
  * \return Returns the initial value before the append.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint32_t fplAtomicAddU32(volatile uint32_t *value, const uint32_t addend);
/**
  * \brief Adds a 64-bit unsigned integer to the value by the given addend atomically.
  * \param value The target value to append to
  * \param addend The value used for adding
  * \return Returns the initial value before the append.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint64_t fplAtomicAddU64(volatile uint64_t *value, const uint64_t addend);
/**
  * \brief Adds a 32-bit signed integer to the value by the given addend atomically.
  * \param value The target value to append to
  * \param addend The value used for adding
  * \return Returns the initial value before the append.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int32_t fplAtomicAddS32(volatile int32_t *value, const int32_t addend);
/**
  * \brief Adds a 64-bit signed integer to the value by the given addend atomically.
  * \param value The target value to append to
  * \param addend The value used for adding
  * \return Returns the initial value before the append.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int64_t fplAtomicAddS64(volatile int64_t *value, const int64_t addend);
/**
  * \brief Adds a size to the value by the given addend atomically.
  * \param value The target value to append to
  * \param addend The value used for adding
  * \return Returns the initial value before the append.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api size_t fplAtomicAddSize(volatile size_t *value, const size_t addend);

/**
  * \brief Increments the given 32-bit unsigned integer by one atomically.
  * \param value The target value to increment
  * \return Returns the value after the increment.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint32_t fplAtomicIncU32(volatile uint32_t *value);
/**
  * \brief Increments the given 64-bit unsigned integer by one atomically.
  * \param value The target value to increment
  * \return Returns the value after the increment.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api uint64_t fplAtomicIncU64(volatile uint64_t *value);
/**
  * \brief Increments the given 32-bit signed integer by one atomically.
  * \param value The target value to increment
  * \return Returns the value after the increment.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int32_t fplAtomicIncS32(volatile int32_t *value);
/**
  * \brief Increments the given 64-bit signed integer by one atomically.
  * \param value The target value to increment
  * \return Returns the value after the increment.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api int64_t fplAtomicIncS64(volatile int64_t *value);
/**
  * \brief Increments the given size atomically.
  * \param value The target value to increment
  * \return Returns the value after the increment.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api size_t fplAtomicIncSize(volatile size_t *value);


/**
  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangeU32() when you want to check if the exchange has happened or not.
  */
fpl_platform_api uint32_t fplAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangeU64() when you want to check if the exchange has happened or not.
  */
fpl_platform_api uint64_t fplAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangeS32() when you want to check if the exchange has happened or not.
  */
fpl_platform_api int32_t fplAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangeS64() when you want to check if the exchange has happened or not.
  */
fpl_platform_api int64_t fplAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangePtr() when you want to check if the exchange has happened or not.
  */
fpl_common_api void *fplAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);
/**
  * \brief Compares a size with a comparand and exchange it when comparand matches destination.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns the value of the destination before the exchange, regardless of the result.
  * \note Ensures that memory operations are completed in order.
  * \note Use \ref fplIsAtomicCompareAndExchangePtr() when you want to check if the exchange has happened or not.
  */
fpl_common_api size_t fplAtomicCompareAndExchangeSize(volatile size_t *dest, const size_t comparand, const size_t exchange);

/**
  * \brief Compares a 32-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
  * \brief Compares a 64-bit unsigned integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
  * \brief Compares a 32-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
  * \brief Compares a 64-bit signed integer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_platform_api bool fplIsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
  * \brief Compares a pointer with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_common_api bool fplIsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);
/**
  * \brief Compares a size with a comparand and exchange it when comparand matches destination and returns a bool indicating the result.
  * \param dest The target value to write into
  * \param comparand The value to compare with
  * \param exchange The value to exchange with
  * \return Returns true when the exchange happened, false otherwise.
  * \note Ensures that memory operations are completed in order.
  */
fpl_common_api bool fplIsAtomicCompareAndExchangeSize(volatile size_t *dest, const size_t comparand, const size_t exchange);

/**
  * \brief Loads the 32-bit unsigned value atomically and returns the value.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  */
fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source);
/**
  * \brief Loads the 64-bit unsigned value atomically and returns the value.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  */
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source);
/**
  * \brief Loads the 32-bit signed value atomically and returns the value.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  */
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source);
/**
  * \brief Loads the 64-bit signed value atomically and returns the value.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  */
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source);
/**
  * \brief Loads the pointer value atomically and returns the value.
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  */
fpl_common_api void *fplAtomicLoadPtr(volatile void **source);
/**
  * \brief Loads the size value atomically and returns the value.
  * \note Ensures that memory operations are completed before the read.
  * \note This may use a CAS instruction when there is no suitable compiler intrinsics found.
  * \param source The source value to read from
  * \return Returns the atomically loaded source value
  */
fpl_common_api size_t fplAtomicLoadSize(volatile size_t *source);

/**
  * \brief Overwrites the 32-bit unsigned value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
/**
  * \brief Overwrites the 64-bit unsigned value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
/**
  * \brief Overwrites the 32-bit signed value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value);
/**
  * \brief Overwrites the 64-bit signed value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value);
/**
  * \brief Overwrites the pointer value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value);
/**
  * \brief Overwrites the size value atomically.
  * \param dest The destination to write to
  * \param value The value to exchange with
  * \note Ensures that memory operations are completed before the write.
  */
fpl_common_api void fplAtomicStoreSize(volatile size_t *dest, const size_t value);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup OS Operating system infos
  * \brief This category contains functions for retrievement several operating system informations such as version, name etc.
  * \{
  *
  */
// ----------------------------------------------------------------------------

//! A type definition mapping a part of a version number
typedef char fplVersionNumberPart[4];

//! A structure that contains version information
typedef struct fplVersionInfo {
	//! Full name
	char fullName[256];
	union {
		// @TODO(final): Dont use decimals for version numbers, just use char array of max size of 3 -> 16.04 is not the same as 16.4!
		struct {
			//! Major version
			fplVersionNumberPart major;
			//! Minor version
			fplVersionNumberPart minor;
			//! Fix version
			fplVersionNumberPart fix;
			//! Build version
			fplVersionNumberPart build;
		};
		//! Version number as array
		fplVersionNumberPart values[4];
	};
} fplVersionInfo;

//! A structure that contains operating system infos
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
  * \brief Gets system informations from the operating system
  * \param outInfos The target \ref fplOSInfos structure
  * \return Returns true when the infos could be retrieved, false otherwise.
  * \note This may be called without initializing the platform
  */
fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos);

/**
  * \brief Gets the username of the current logged-in user
  * \param nameBuffer The target buffer
  * \param maxNameBufferLen The max length of the target buffer
  * \return Returns true when a username could be retrieved, false otherwise.
  */
fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, const size_t maxNameBufferLen);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Hardware Hardware infos
  * \brief This category contains functions for retrievement hardware informations such as memory usage, cpu infos, etc.
  * \{
  *
  */
// ----------------------------------------------------------------------------

//! A structure that contains informations about current memory usage
typedef struct fplMemoryInfos {
	//! Size of physical installed memory in bytes
	uint64_t installedPhysicalSize;
	//! Total size of physical memory in bytes (May be less than size of installed physical memory, due to shared memory stuff)
	uint64_t totalPhysicalSize;
	//! Available physical memory in bytes
	uint64_t freePhysicalSize;
	//! Total size of memory cache in bytes
	uint64_t totalCacheSize;
	//! Available size of the memory cache in bytes
	uint64_t freeCacheSize;
	//! Total number of memory pages
	uint64_t totalPageCount;
	//! Number of available memory pages
	uint64_t freePageCount;
	//! Page size in bytes
	uint64_t pageSize;
} fplMemoryInfos;

//! An enumeration of architecture types
typedef enum fplArchType {
	//! Unknown architecture
	fplArchType_Unknown = 0,
	//! X86 architecture
	fplArchType_x86,
	//! X86 with 64-bit architecture
	fplArchType_x86_64,
	//! X64 only architecture
	fplArchType_x64,
	//! ARM32 architecture
	fplArchType_Arm32,
	//! ARM64 architecture
	fplArchType_Arm64,
} fplArchType;

/**
  * \brief Gets the string representation of the given architecture type
  * \param type The \ref fplArchType enumeration value
  * \return Returns a string for the given architecture type
  */
fpl_common_api const char *fplGetArchTypeString(const fplArchType type);

/**
  * \brief Retrieves the total number of processor cores.
  * \return Returns the total number of processor cores.
  */
fpl_platform_api size_t fplGetProcessorCoreCount();
/**
  * \brief Retrieves the name of the processor.
  * \param destBuffer The destination buffer
  * \param maxDestBufferLen The max length of the destination buffer
  * \return Returns a pointer to the last written character or \ref fpl_null otherwise.
  */
fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen);
/**
  * \brief Retrieves the current system memory usage.
  * \param outInfos The target \ref fplMemoryInfos structure
  * \return Returns true when the memory infos was retrieved, false otherwise.
  */
fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos);

/**
  * \brief Gets the running architecture type
  * \return Returns the running architecture type
  */
fpl_platform_api fplArchType fplGetRunningArchitecture();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Settings Settings & Configurations
  * \brief This category contains global settings structures/enumerations and functions to initialize/set them
  * \{
  */
// ----------------------------------------------------------------------------

//! An enumeration of initialization flags
typedef enum fplInitFlags {
	//! No init flags
	fplInitFlags_None = 0,
	//! Create a console window
	fplInitFlags_Console = 1 << 0,
	//! Create a single window
	fplInitFlags_Window = 1 << 1,
	//! Use a video backbuffer (This flag ensures that \ref fplInitFlags_Window is included always)
	fplInitFlags_Video = 1 << 2,
	//! Use asyncronous audio playback
	fplInitFlags_Audio = 1 << 3,
} fplInitFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(fplInitFlags);

//! Default init flags for initializing everything
static const fplInitFlags fplInitFlags_All = fplInitFlags_Console | fplInitFlags_Window | fplInitFlags_Video | fplInitFlags_Audio;

//! InitFlags operator overloads for C++
//! An emenumeration of init result types
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

/**
  * \brief Gets the string representation of a init result type.
  * \param type The init result type \ref fplInitResultType
  * \return Returns the string representation of a init result type.
  */
fpl_common_api const char *fplGetInitResultTypeString(const fplInitResultType type);

//! An enumeration of video driver types
typedef enum fplVideoDriverType {
	//! No video driver
	fplVideoDriverType_None = 0,
	//! OpenGL
	fplVideoDriverType_OpenGL,
	//! Software
	fplVideoDriverType_Software
} fplVideoDriverType;

#if defined(FPL_ENABLE_VIDEO_OPENGL)
//! An enumeration of OpenGL compability flags
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

//! A structure that contains OpenGL video settings
typedef struct fplOpenGLVideoSettings {
	//! Compability flags
	fplOpenGLCompabilityFlags compabilityFlags;
	//! Desired major version
	uint32_t majorVersion;
	//! Desired minor version
	uint32_t minorVersion;
	//! Multisampling count
	uint8_t multiSamplingCount;
} fplOpenGLVideoSettings;
#endif // FPL_ENABLE_VIDEO_OPENGL

//! A union that contains graphics api settings
typedef union fplGraphicsApiSettings {
#if defined(FPL_ENABLE_VIDEO_OPENGL)
	//! OpenGL settings
	fplOpenGLVideoSettings opengl;
#endif
	//! Dummy field when no graphics drivers are available
	int dummy;
} fplGraphicsApiSettings;

//! A structure that contains video settings such as driver, vsync, api-settings etc.
typedef struct fplVideoSettings {
	//! Graphics Api settings
	fplGraphicsApiSettings graphics;
	//! Video driver type
	fplVideoDriverType driver;
	//! Is vertical syncronisation enabled. Useable only for hardware rendering!
	fpl_b32 isVSync;
	//! Is backbuffer automatically resized. Useable only for software rendering!
	fpl_b32 isAutoSize;
} fplVideoSettings;

/**
  * \brief Resets the given video settings to default values
  * \param video The target \ref fplVideoSettings structure
  * \note This will not change any video settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video);

//! An enumeration of audio driver types
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

//! An enumeration of audio format types
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

//! A structure containing audio device format properties, such as type, samplerate, channels, etc.
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

//! A union containing a id of the underlying driver
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

//! A structure containing the name and the id of the audio device
typedef struct fplAudioDeviceInfo {
	//! Device name
	char name[256];
	//! Device id
	fplAudioDeviceID id;
} fplAudioDeviceInfo;

#if defined(FPL_ENABLE_AUDIO_ALSA)
//! A structure containing settings for the ALSA audio driver
typedef struct fplAlsaAudioSettings {
	//! Disable the usage of MMap in ALSA
	fpl_b32 noMMap;
} fplAlsaAudioSettings;
#endif

//! A union containing driver specific audio settings
typedef union fplSpecificAudioSettings {
#if defined(FPL_ENABLE_AUDIO_ALSA)
	//! Alsa specific settings
	fplAlsaAudioSettings alsa;
#endif
	//! Dummy field (When no drivers are available)
	int dummy;
} fplSpecificAudioSettings;

/**
  * \brief A callback for reading audio samples from the client
  * \param deviceFormat The pointer to the \ref fplAudioDeviceFormat structure, the audio cards expects
  * \param frameCount The numbers if frames the client should write at max
  * \param outputSamples The pointer to the target samples
  * \param userData The pointer to the user data specified in \ref fplAudioSettings
  * \return Returns the number written frames
  */
typedef uint32_t(fpl_audio_client_read_callback)(const fplAudioDeviceFormat *deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

//! A structure containing audio settings, such as format, device info, callbacks, driver, etc.
typedef struct fplAudioSettings {
	//! The device format
	fplAudioDeviceFormat deviceFormat;
	//! The device info
	fplAudioDeviceInfo deviceInfo;
	//! Specific settings
	fplSpecificAudioSettings specific;
	//! The callback for retrieving audio data from the client
	fpl_audio_client_read_callback *clientReadCallback;
	//! User data pointer for client read callback
	void *userData;
	//! The targeted driver
	fplAudioDriverType driver;
	//! Audio buffer in milliseconds
	uint32_t bufferSizeInMilliSeconds;
	//! Is exclude mode prefered
	fpl_b32 preferExclusiveMode;
} fplAudioSettings;

/**
  * \brief Resets the given audio settings to default settings (S16 PCM, 48 KHz, 2 Channels)
  * \param audio The target \ref fplAudioSettings structure
  * \note This will not change any audio settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultAudioSettings(fplAudioSettings *audio);

//! An enumeration of image types
typedef enum fplImageType {
	//! No image type
	fplImageType_None = 0,
	//! RGBA image type
	fplImageType_RGBA,
} fplImageType;

//! A structure containing data for working with a image source
typedef struct fplImageSource {
	//! Pointer to the source data
	const uint8_t *data;
	//! Width in pixels
	uint32_t width;
	//! Height in pixels
	uint32_t height;
	//! Image type
	fplImageType type;
} fplImageSource;

//! A structure containing window settings, such as size, title etc.
typedef struct fplWindowSettings {
	//! Window title
	char windowTitle[256];
	//! Window icons (0 = Small, 1 = Large)
	fplImageSource icons[2];
	//! Window width in screen coordinates
	uint32_t windowWidth;
	//! Window height in screen coordinates
	uint32_t windowHeight;
	//! Fullscreen width in screen coordinates
	uint32_t fullscreenWidth;
	//! Fullscreen height in screen coordinates
	uint32_t fullscreenHeight;
	//! Is window resizable
	fpl_b32 isResizable;
	//! Is window decorated
	fpl_b32 isDecorated;
	//! Is floating
	fpl_b32 isFloating;
	//! Is window in fullscreen mode
	fpl_b32 isFullscreen;
} fplWindowSettings;

/**
  * \brief Resets the given window settings container to default settings
  * \param window The target \ref fplWindowSettings structure
  * \note This will not change any window settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultWindowSettings(fplWindowSettings *window);

//! A structure containing input settings
typedef struct fplInputSettings {
	//! Frequency in ms for detecting new or removed controllers (Default: 200)
	uint32_t controllerDetectionFrequency;
	//! Skip repeated key presses and just push when button state was different
	fpl_b32 skipKeyRepeats;
} fplInputSettings;

/**
  * \brief Resets the given input settings contains to default values.
  * \param input The target \ref fplInputSettings structure
  * \note This will not change any input settings! To change the actual settings you have to pass the entire \ref fplSettings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultInputSettings(fplInputSettings *input);

//! A structure containg settings, such as window, video, etc.
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
  * \param settings The target \ref fplSettings structure
  * \note This will not change the active settings! To change the actual settings you have to pass this settings container to a argument in \ref fplPlatformInit().
  */
fpl_common_api void fplSetDefaultSettings(fplSettings *settings);
/**
  * \brief Creates a full settings structure containing default values
  * \return Returns a defaulted \ref fplSettings structure
  */
fpl_common_api fplSettings fplMakeDefaultSettings();
/**
  * \brief Gets the current settings
  * \return Returns a pointer to the \ref fplSettings structure
  */
fpl_common_api const fplSettings *fplGetCurrentSettings();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Platform Platform functions
  * \brief This category contains structures, enumerations and functions for initializing/releasing the platform.
  * \{
  */
// ----------------------------------------------------------------------------

//! An enumeration of platform types
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
  * \brief Gets the string representation of the given platform type
  * \param type The platform type \ref fplPlatformType
  * \return Returns the string representation for the given platform type \ref fplPlatformType
  */
fpl_common_api const char *fplGetPlatformTypeString(const fplPlatformType type);

/**
  * \brief Initializes the platform layer.
  * \param initFlags The init flags \ref fplInitFlags used for enable certain features, like video/audio etc.
  * \param initSettings The \ref fplSettings structure to control the platform layer behavior or systems, if null is passed here default values are used automatically.
  * \return Returns \ref fplInitResultType indicating the result of the platform initialization
  * \note \ref fplPlatformRelease() must be called when you are done! After \ref fplPlatformRelease() has been called you can call this function again if needed.
  */
fpl_common_api fplInitResultType fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings);
/**
  * \brief Releases the resources allocated by the platform layer.
  * \note Can only be called when \ref fplPlatformInit() was successful.
  */
fpl_common_api void fplPlatformRelease();
/**
  * \brief Gets the type of the platform
  * \return Returns the platform type \ref fplPlatformType
  */
fpl_common_api fplPlatformType fplGetPlatformType();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Logging Logging
  * \brief This category contains functions and types for controlling logging output
  * \{
  */
// ----------------------------------------------------------------------------

//! An enumeration of log levels
typedef enum fplLogLevel {
	//! All
	fplLogLevel_All = -1,
	//! Critical
	fplLogLevel_Critical = 0,
	//! Error
	fplLogLevel_Error = 1,
	//! Warning
	fplLogLevel_Warning = 2,
	//! Info
	fplLogLevel_Info = 3,
	//! Verbose
	fplLogLevel_Verbose = 4,
	//! Debug
	fplLogLevel_Debug = 5,
} fplLogLevel;

#if defined(FPL_ENABLE_LOGGING)
/**
  * \brief A callback for printing a log message
  * \param level The log level \ref fplLogLevel
  * \param message The log message string
  */
typedef void (fplFuncLogCallback)(const fplLogLevel level, const char *message);

//! An enumeration of log writer flags
typedef enum fplLogWriterFlags {
	//! No appender flags
	fplLogWriterFlags_None = 0,
	//! Console output
	fplLogWriterFlags_Console = 1 << 0,
	//! Debug output
	fplLogWriterFlags_DebugOut = 1 << 1,
	//! Custom output
	fplLogWriterFlags_Custom = 1 << 2,
} fplLogWriterFlags;
//! Log writer flags enumeration operators
FPL_ENUM_AS_FLAGS_OPERATORS(fplLogWriterFlags);

//! A structure containing console logging properties
typedef struct fplLogWriterConsole {
	//! Enable this to log to the error console instead
	fpl_b32 logToError;
} fplLogWriterConsole;

//! A structure containing properties custom logging properties
typedef struct fplLogWriterCustom {
	//! User callback
	fplFuncLogCallback *callback;
} fplLogWriterCustom;

//! A structure containing log writer settings
typedef struct fplLogWriter {
	//! Flags
	fplLogWriterFlags flags;
	//! Console
	fplLogWriterConsole console;
	//! Custom
	fplLogWriterCustom custom;
} fplLogWriter;

//! A structure containing log settings
typedef struct fplLogSettings {
#if defined(FPL_ENABLE_LOG_MULTIPLE_WRITERS)
	union {
		//! All writers
		fplLogWriter writers[6];
		struct {
			//! Critical writer
			fplLogWriter criticalWriter;
			//! Error writer
			fplLogWriter errorWriter;
			//! Warning writer
			fplLogWriter warningWriter;
			//! Info writer
			fplLogWriter infoWriter;
			//! Verbose writer
			fplLogWriter verboseWriter;
			//! Debug writer
			fplLogWriter debugWriter;
		};
	};
#else
	//! Single writer
	fplLogWriter writers[1];
#endif // FPL_USE_LOG_SIMPLE
	//! Maximum log level
	fplLogLevel maxLevel;
	//! Is initialized (When set to false all values will be set to default values)
	fpl_b32 isInitialized;
} fplLogSettings;

/**
  * \brief Overwrites the current log settings
  * \param params The source \ref fplLogSettings structure
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api void fplSetLogSettings(const fplLogSettings *params);
/**
  * \brief Gets the current log settings
  * \return Returns a pointer the \ref fplLogSettings structure
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api const fplLogSettings *fplGetLogSettings();
/**
  * \brief Changes the current maximum log level to the given value
  * \param maxLevel The new maximum log level \ref fplLogLevel
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api void fplSetMaxLogLevel(const fplLogLevel maxLevel);
/**
  * \brief Gets the current maximum allowed log level
  * \return Returns the current maximum log level \ref fplLogLevel
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api fplLogLevel fplGetMaxLogLevel();
#endif // FPL_ENABLE_LOGGING

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup ErrorHandling Error Handling
  * \brief This category contains functions for handling errors
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Gets the last internal error string
  * \return Returns the last error string or empty string when there was no error.
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api const char *fplGetLastError();
/**
  * \brief Gets the last error string from the given index
  * \param index The index
  * \return Returns the last error string from the given index or empty when there was no error.
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api const char *fplGetErrorByIndex(const size_t index);
/**
  * \brief Gets the count of total last errors
  * \note This function can be called regardless of the initialization state!
  * \return Returns the number of last errors or zero when there was no error.
  */
fpl_common_api size_t fplGetErrorCount();
/**
  * \brief Clears all the current errors in the platform
  * \note This function can be called regardless of the initialization state!
  */
fpl_common_api void fplClearErrors();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup DynamicLibrary Dynamic library loading
  * \brief This category contains functions for loading dynamic libraries.
  * \{
  */
// ----------------------------------------------------------------------------

//! A union containing the library handle for any platform
typedef union fplInternalDynamicLibraryHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 library handle
	HMODULE win32LibraryHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix library handle
	void *posixLibraryHandle;
#endif
} fplInternalDynamicLibraryHandle;

//! A structure containing the internal handle to a dynamic library
typedef struct fplDynamicLibraryHandle {
	//! Internal library handle
	fplInternalDynamicLibraryHandle internalHandle;
	//! Library opened successfully
	fpl_b32 isValid;
} fplDynamicLibraryHandle;

/**
  * \brief Loads a dynamic library and returns if the load was successful or not.
  * \param libraryFilePath The path to the library with included file extension (.dll / .so)
  * \param outHandle The output handle \ref fplDynamicLibraryHandle
  * \return Returns true when the library was loaded successfully, false otherwise.
  */
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle);
/**
  * \brief Returns the dynamic library procedure address for the given procedure name.
  * \param handle The \ref fplDynamicLibraryHandle handle to the loaded library
  * \param name The name of the procedure
  * \return Returns the procedure address for the given procedure name or \ref fpl_null when procedure not found or library is not loaded.
  */
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name);
/**
  * \brief Unloads the loaded library and resets the handle to zero.
  * \param handle The library handle \ref fplDynamicLibraryHandle
  */
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Debug Debug
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Writes the given text into the debugger output stream
  * \param text The text to write into the debugger output stream
  * \note This function will only work in IDEs such as MSVC
  */
fpl_platform_api void fplDebugOut(const char *text);
/**
  * \brief Writes the given formatted text into the debugger output stream
  * \param format The format used for writing into the debugger output stream
  * \param ... The dynamic arguments used for formatting the text.
  * \note This function will only work in IDEs such as MSVC
  */
fpl_common_api void fplDebugFormatOut(const char *format, ...);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Console Console functions
  * \brief This category contains function for handling console in/out
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
  * \brief Wait for a character to be typed in the console input and return it.
  * \note This is most likely just a wrapper call to getchar()
  * \return Returns the character typed in in the console input
  */
fpl_platform_api char fplConsoleWaitForCharInput();

/**
  * \brief Writes the given formatted text to the standard output console buffer.
  * \param format The format used for writing into the standard output console
  * \param ... The dynamic arguments used for formatting the text
  * \note This is most likely just a wrapper call to vfprintf(stdout)
  */
fpl_common_api void fplConsoleFormatOut(const char *format, ...);
/**
  * \brief Writes the given formatted text to the standard error console buffer.
  * \param format The format used for writing into the standard error console
  * \param ... The dynamic arguments used for formatting the text
  * \note This is most likely just a wrapper call to vfprintf(stderr)
  */
fpl_common_api void fplConsoleFormatError(const char *format, ...);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Timings Timing functions
  * \brief This category contains functions for time comparisons
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Gets the current system clock in seconds in high precision (micro/nano seconds).
  * \return Returns the number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api double fplGetTimeInSecondsHP();
/**
  * \brief Gets the current system clock in seconds in low precision (milliseconds).
  * \return Returns the number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInSecondsLP();
/**
  * \brief Gets the current system clock in seconds in default precision.
  * \return Returns the number of seconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time. There is no guarantee to get high precision here, use for high precision \ref fplGetTimeInSecondsHP() instead!
  */
fpl_platform_api double fplGetTimeInSeconds();
/**
  * \brief Gets the current system clock in milliseconds in high precision (micro/nano seconds)
  * \return Returns the number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api double fplGetTimeInMillisecondsHP();
/**
  * \brief Gets the current system clock in milliseconds in low precision (milliseconds)
  * \return Returns the number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInMillisecondsLP();
/**
  * \brief Gets the current system clock in milliseconds
  * \return Returns the number of milliseconds since some fixed starting point (OS start, System start, etc).
  * \note Can only be used to calculate a difference in time!
  */
fpl_platform_api uint64_t fplGetTimeInMilliseconds();

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Threading Threading and syncronisation routines
  * \brief This category contains functions/types for dealing with concurrent programming, such as threads, mutexes, conditions, etc.
  * \{
  */
// ----------------------------------------------------------------------------

//! A type definition for a timeout value in milliseconds
typedef uint32_t fplTimeoutValue;
//! Infinite timeout constant
#define FPL_TIMEOUT_INFINITE UINT32_MAX

//! An enumeration of thread states
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

//! A type definition for mapping \ref fplThreadState into a 32-bit integer
typedef uint32_t fplThreadState;

//! Forward declare thread handle
typedef struct fplThreadHandle fplThreadHandle;
/**
  * \brief A callback to execute user code from a \ref fplThreadHandle
  * \param thread The thread handle \ref fplThreadHandle
  * \param data The user data pointer
  */
//! Run function type definition for \ref fplThreadCreate()
typedef void (fpl_run_thread_function)(const fplThreadHandle *thread, void *data);

//! A union containing the thread handle for any platform
typedef union fplInternalThreadHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 thread handle
	HANDLE win32ThreadHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX thread handle
	pthread_t posixThread;
#endif
} fplInternalThreadHandle;

//! The thread handle structure
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
	volatile fpl_b32 isValid;
	//! Is this thread stopping
	volatile fpl_b32 isStopping;
} fplThreadHandle;

#if defined(FPL_PLATFORM_WIN32)
//! A structure containing the semaphore handle and the value for win32
typedef struct fplInternalSemaphoreHandleWin32 {
	//! Semaphore handle
	HANDLE handle;
	//! Semaphore value
	volatile int32_t value;
} fplInternalSemaphoreHandleWin32;
#endif

//! A union containing the internal semaphore handle for any platform
typedef union fplInternalSemaphoreHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 semaphore handle
	fplInternalSemaphoreHandleWin32 win32;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix semaphore handle
	sem_t posixHandle;
#endif
} fplInternalSemaphoreHandle;

//! The semaphore handle structure
typedef struct fplSemaphoreHandle {
	//! The internal semaphore handle
	fplInternalSemaphoreHandle internalHandle;
	//! Is it valid
	fpl_b32 isValid;
} fplSemaphoreHandle;

//! A union containing the internal mutex handle for any platform
typedef union fplInternalMutexHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 mutex handle
	CRITICAL_SECTION win32CriticalSection;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix mutex handle
	pthread_mutex_t posixMutex;
#endif
} fplInternalMutexHandle;

//! The mutex handle structure
typedef struct fplMutexHandle {
	//! The internal mutex handle
	fplInternalMutexHandle internalHandle;
	//! Is it valid
	fpl_b32 isValid;
} fplMutexHandle;

//! A union containing the internal signal handle for any platform
typedef union fplInternalSignalHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 event handle
	HANDLE win32EventHandle;
#elif defined(FPL_PLATFORM_LINUX)
	//! Linux event handle
	int linuxEventHandle;
#endif
} fplInternalSignalHandle;

//! The signal handle structure
typedef struct fplSignalHandle {
	//! The internal signal handle
	fplInternalSignalHandle internalHandle;
	//! Is it valid
	fpl_b32 isValid;
} fplSignalHandle;

//! An enumeration of signal values
typedef enum fplSignalValue {
	//! Value is unset
	fplSignalValue_Unset = 0,
	//! Value is set
	fplSignalValue_Set = 1,
} fplSignalValue;

//! A union containing the internal condition variable for any platform
typedef union fplInternalConditionVariable {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 condition variable
	CONDITION_VARIABLE win32Condition;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX condition variable
	pthread_cond_t posixCondition;
#endif	//! Dummy field
} fplInternalConditionVariable;

//! The condition variable structure
typedef struct fplConditionVariable {
	//! The internal condition handle
	fplInternalConditionVariable internalHandle;
	//! Is it valid
	fpl_b32 isValid;
} fplConditionVariable;

/**
  * \brief Gets the current thread state for the given thread
  * \param thread The thread handle \ref fplThreadHandle
  * \return Returns the current thread state \ref fplThreadState for the given thread
  */
fpl_common_api fplThreadState fplGetThreadState(fplThreadHandle *thread);
/**
  * \brief Creates and starts a thread and returns the handle to it.
  * \param runFunc The pointer to the \ref fpl_run_thread_function
  * \param data The user data pointer passed to the execution function callback
  * \return Returns a pointer to the \ref fplThreadHandle structure or \ref fpl_null when the limit of active threads has been reached.
  * \warning Do not free this thread context directly!
  * \note The resources are automatically cleaned up when the thread terminates.
  */
fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data);
/**
  * \brief Let the current thread sleep for the given amount of milliseconds.
  * \param milliseconds Number of milliseconds to sleep
  * \note There is no guarantee that the OS sleeps for the exact amount of milliseconds! This can vary based on the OS scheduler granularity.
  */
fpl_platform_api void fplThreadSleep(const uint32_t milliseconds);
/**
  * \brief Let the current thread yield execution to another thread that is ready to run on this core.
  * \return Returns true when the functions succeeds, false otherwise.
  */
fpl_platform_api bool fplThreadYield();
/**
  * \brief Forces the given thread to stop and release all underlying resources.
  * \param thread The pointer to the \ref fplThreadHandle structure
  * \return True when the thread was terminated, false otherwise.
  * \warning Do not free the given thread context manually!
  * \note This thread context may get re-used for another thread in the future.
  * \note Returns true when the threads was terminated, false otherwise.
  */
fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread);
/**
  * \brief Wait until the given thread is done running or the given timeout has been reached.
  * \param thread The pointer to the \ref fplThreadHandle structure
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when the thread completes or when the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout);
/**
  * \brief Wait until all given threads are done running or the given timeout has been reached.
  * \param threads The array \ref fplThreadHandle pointers
  * \param count The number of threads in the array
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when all threads completes or when the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Wait until one of given threads is done running or the given timeout has been reached.
  * \param threads The array \ref fplThreadHandle pointers
  * \param count The number of threads in the array
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when one thread completes or when the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle *threads[], const size_t count, const fplTimeoutValue timeout);

/**
  * \brief Initializes the given mutex
  * \param mutex The pointer to the \ref fplMutexHandle structure
  * \return Returns true when the mutex was initialized, false otherwise.
  * \note Use \ref fplMutexDestroy() when you are done with this mutex.
  */
fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex);
/**
  * \brief Releases the given mutex and clears the structure to zero.
  * \param mutex The pointer to the \ref fplMutexHandle structure
  */
fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex);
/**
  * \brief Locks the given mutex and blocks any other threads.
  * \param mutex The pointer to the \ref fplMutexHandle structure
  * \returns Returns true when the mutex was locked, false otherwise.
  */
fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex);
/**
  * \brief Tries to lock the given mutex without blocking other threads.
  * \param mutex The pointer to the \ref fplMutexHandle structure
  * \returns Returns true when the mutex was locked, false otherwise.
  */
fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex);
/**
 * \brief Unlocks the given mutex
 * \param mutex The pointer to the \ref fplMutexHandle structure
 * \returns Returns true when the mutex was unlocked, false otherwise.
 */
fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex);

/**
  * \brief Initializes the given signal
  * \param signal The pointer to the \ref fplSignalHandle structure
  * \param initialValue The initial value the signal is set to
  * \return Returns true when initialization was successful, false otherwise.
  * \note Use \ref fplSignalDestroy() when you are done with this Signal to release it.
  */
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue);
/**
  * \brief Releases the given signal and clears the structure to zero.
  * \param signal The pointer to the \ref fplSignalHandle structure
  */
fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal);
/**
  * \brief Waits until the given signal are waked up.
  * \param signal The pointer to the \ref fplSignalHandle structure
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Retrns true when the signal woke up or the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout);
/**
  * \brief Waits until all the given signal are waked up.
  * \param signals The array of \ref fplSignalHandle pointers
  * \param count The number of signals in the array
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when all signals woke up or the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Waits until any of the given signals wakes up or the timeout has been reached.
  * \param signals The array of \ref fplSignalHandle pointers
  * \param count The number of signals in the array
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when any of the signals woke up or the timeout has been reached, false otherwise.
  */
fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle *signals[], const size_t count, const fplTimeoutValue timeout);
/**
  * \brief Sets the signal and wakes up the given signal.
  * \param signal The pointer to the \ref fplSignalHandle structure
  * \return Returns true when the signal was set and broadcasted or false otherwise.
  */
fpl_platform_api bool fplSignalSet(fplSignalHandle *signal);
/**
  * \brief Resets the signal.
  * \param signal The pointer to the \ref fplSignalHandle structure
  * \return Returns true when the signal was reset, false otherwise.
  */
fpl_platform_api bool fplSignalReset(fplSignalHandle *signal);

/**
  * \brief Initialize
  s the given condition
  * \param condition The pointer to the \ref fplConditionVariable structure
  * \return Returns true when initialization was successful, false otherwise.
  * \note Use \ref fplSignalDestroy() when you are done with this Condition Variable to release its resources.
  */
fpl_platform_api bool fplConditionInit(fplConditionVariable *condition);
/**
  * \brief Releases the given condition and clears the structure to zero.
  * \param condition The pointer to the \ref fplConditionVariable structure
  */
fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition);
/**
  * \brief Sleeps on the given condition and releases the mutex when done.
  * \param condition The pointer to the \ref fplConditionVariable structure
  * \param mutex The pointer to the mutex handle \ref fplMutexHandle structure
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout);
/**
  * \brief Wakes up one thread which waits on the given condition.
  * \param condition The pointer to the \ref fplConditionVariable structure
  * \return Returns true when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition);
/**
  * \brief Wakes up all threads which waits on the given condition.
  * \param condition The pointer to the \ref fplConditionVariable structure
  * \return Returns true when the function succeeds, false otherwise.
  */
fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition);

/**
  * \brief Initializes the semaphore with the given initial value
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \param initialValue The initial value
  * \return Returns true when the semaphores got initialized, false otherwise.
  */
fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue);
/**
  * \brief Releases the internal semaphore resources
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \warning Do not call this when a thread is still waiting on this semaphore
  */
fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore);
/**
  * \brief Waits for the semaphore until it get signaled or the timeout has been reached.
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \param timeout The number of milliseconds to wait. When this is set to \ref FPL_TIMEOUT_INFINITE it will wait infinitly.
  * \return Returns true when the semaphore got signaled, false otherwise.
  * \note When a semaphore got signaled, the semaphore value is decreased by one.
  */
fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout);
/**
  * \brief Tries to wait for the semaphore until it get signaled or return immediatly.
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \return Returns true when the semaphore got signaled, false otherwise.
  * \note When a semaphore got signaled, the semaphore value is decreased by one.
  */
fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore);
/**
  * \brief Gets the current semaphore value
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \return Returns the current semaphore value
  */
fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore);
/**
  * \brief Increments the semaphore value by one
  * \param semaphore The pointer to the \ref fplSemaphoreHandle structure
  * \return Returns true when semaphore was incremented, false otherwise.
  */
fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Memory Memory functions
  * \brief The category contains functions for allocating/manipulating memory
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Clears the given memory by the given size to zero.
  * \param mem The pointer to the memory
  * \param size The size in bytes to be cleared to zero
  */
fpl_common_api void fplMemoryClear(void *mem, const size_t size);
/**
  * \brief Sets the given memory by the given size to the given value.
  * \param mem The pointer to the memory
  * \param value The value to be set
  * \param size The size in bytes to be cleared to zero
  */
fpl_common_api void fplMemorySet(void *mem, const uint8_t value, const size_t size);
/**
  * \brief Copies the given source memory with its length to the target memory.
  * \param sourceMem The pointer to the source memory to copy from
  * \param sourceSize The size in bytes to be copied
  * \param targetMem The pointer to the target memory to copy into
  */
fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem);
/**
  * \brief Allocates memory from the operating system by the given size.
  * \param size The size to by allocated in bytes.
  * \return Returns a pointer to the new allocated memory.
  * \warning Alignment is not ensured here, the OS decides how to handle this. If you want to force a specific alignment use \ref fplMemoryAlignedAllocate() instead.
  * \note The memory is guaranteed to be initialized by zero.
  */
fpl_platform_api void *fplMemoryAllocate(const size_t size);
/**
  * \brief Releases the memory allocated from the operating system.
  * \param ptr The pointer to the allocated memory
  * \warning This should never be called with a aligned memory pointer! For freeing aligned memory, use \ref fplMemoryAlignedFree() instead.
  */
fpl_platform_api void fplMemoryFree(void *ptr);
/**
  * \brief Allocates aligned memory from the operating system by the given alignment.
  * \param size The size amount in bytes
  * \param alignment The alignment in bytes (Must be a power-of-two!)
  * \return Returns the pointer to the new allocated aligned memory.
  * \note The memory is guaranteed to be initialized by zero.
  */
fpl_common_api void *fplMemoryAlignedAllocate(const size_t size, const size_t alignment);
/**
  * \brief Releases the aligned memory allocated from the operating system.
  * \param ptr The pointer to the aligned allocated memory
  * \warning This should never be called with a not-aligned memory pointer! For freeing not-aligned memory, use \ref fplMemoryFree() instead.
  */
fpl_common_api void fplMemoryAlignedFree(void *ptr);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Strings String functions
  * \brief This category contains tons of functions for converting/manipulating strings
  * \{
  */
// ----------------------------------------------------------------------------

/**
  * \brief Matches the given string by the given wildcard and returns a boolean indicating the match.
  * \param source The source string
  * \param wildcard The wildcard string
  * \return Returns true when source string matches the wildcard, false otherwise.
  */
fpl_common_api bool fplIsStringMatchWildcard(const char *source, const char *wildcard);
/**
  * \brief Compares two strings with constrained lengths and returns a boolean indicating the equality.
  * \param a The first string
  * \param aLen The number of characters for the first string
  * \param b The second string
  * \param bLen The number of characters for the second string
  * \return Returns true when both strings are equal, false otherwise.
  * \note Len parameters does not include the null-terminator!
  */
fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen);
/**
  * \brief Compares two strings and returns a boolean indicating the equality.
  * \param a The first string
  * \param b The second string
  * \return Returns true when both strings are equal, false otherwise.
  */
fpl_common_api bool fplIsStringEqual(const char *a, const char *b);
/**
  * \brief Ensures that the given string always ends with a path separator with length constrained
  * \param path The target path string
  * \param maxPathLen The max length of the target path
  * \return Returns a pointer to the last written character or \ref fpl_null.
  */
fpl_common_api char *fplEnforcePathSeparatorLen(char *path, size_t maxPathLen);
/**
  * \brief Ensures that the given string always ends with a path separator
  * \param path The path string
  * \return Returns a pointer to the last written character or \ref fpl_null.
  * \note This function is unsafe as it does not know the maximum length of the string!
  */
fpl_common_api char *fplEnforcePathSeparator(char *path);
/**
  * \brief Appends the source string to the given buffer
  * \param appended The appending source string
  * \param appendedLen The length of the appending source string
  * \param buffer The target buffer
  * \param maxBufferLen The max length of the target buffer
  * \return Returns a pointer to the last written character or \ref fpl_null.
  */
fpl_common_api char *fplStringAppendLen(const char *appended, const size_t appendedLen, char *buffer, size_t maxBufferLen);
/**
  * \brief Appends the source string to the given buffer
  * \param appended The appending source string
  * \param buffer The target buffer
  * \param maxBufferLen The max length of the target buffer
  * \return Returns a pointer to the last written character or \ref fpl_null.
  */
fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen);
/**
  * \brief Counts the number of ansi characters without including the zero terminator.
  * \param str The 8-bit ansi string
  * \return Returns the number of characters of the given 8-bit Ansi string or zero when the input string is fpl_null.
  */
fpl_common_api size_t fplGetAnsiStringLength(const char *str);
/**
  * \brief Counts the number of wide characters without including the zero terminator.
  * \param str The 16-bit wide string
  * \return Returns the number of characters of the given 16-bit Wide string or zero when the input string is fpl_null.
  */
fpl_common_api size_t fplGetWideStringLength(const wchar_t *str);
/**
  * \brief Copies the given 8-bit source ansi string with a constrained length into a destination ansi string.
  * \param source The 8-bit source ansi string
  * \param sourceLen The number of characters to copy
  * \param dest The 8-bit destination ansi string buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_common_api char *fplCopyAnsiStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 8-bit source ansi string into a destination ansi string.
  * \param source The 8-bit source ansi string
  * \param dest The 8-bit destination ansi string buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string with a fixed length into a destination wide string.
  * \param source The 16-bit source wide string
  * \param sourceLen The number of characters to copy
  * \param dest The 16-bit destination wide string buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const size_t sourceLen, wchar_t *dest, const size_t maxDestLen);
/**
  * \brief Copies the given 16-bit source wide string into a destination wide string.
  * \param source The 16-bit source wide string
  * \param dest The 16-bit destination wide string buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const size_t maxDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit ansi string.
  * \param wideSource The 16-bit source wide string
  * \param maxWideSourceLen The number of characters of the source wide string
  * \param ansiDest The 8-bit destination ansi string buffer
  * \param maxAnsiDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_platform_api char *fplWideStringToAnsiString(const wchar_t *wideSource, const size_t maxWideSourceLen, char *ansiDest, const size_t maxAnsiDestLen);
/**
  * \brief Converts the given 16-bit source wide string with length in a 8-bit UTF-8 ansi string.
  * \param wideSource The 16-bit source wide string
  * \param maxWideSourceLen The number of characters of the source wide string
  * \param utf8Dest The 8-bit destination ansi string buffer
  * \param maxUtf8DestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen);
/**
  * \brief Converts the given 8-bit source ansi string with length in a 16-bit wide string.
  * \param ansiSource The 8-bit source ansi string
  * \param ansiSourceLen The number of characters of the source wide string
  * \param wideDest The 16-bit destination wide string buffer
  * \param maxWideDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always. Does not allocate any memory.
  */
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen);
/**
  * \brief Converts the given 8-bit UTF-8 source ansi string with length in a 16-bit wide string.
  * \param utf8Source The 8-bit source ansi string
  * \param utf8SourceLen The number of characters of the source wide string
  * \param wideDest The 16-bit destination wide string buffer
  * \param maxWideDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note Null terminator is included always.
  */
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen);
/**
  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and variable arguments.
  * \param ansiDestBuffer The 8-bit destination ansi string buffer
  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer
  * \param format The string format
  * \param ... The variable arguments
  * \return Returns the pointer to the last written character or \ref fpl_null.
  * \note This is most likely just a wrapper call to vsnprintf()
  */
fpl_common_api char *fplFormatAnsiString(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, ...);
/**
  * \brief Fills out the given destination ansi string buffer with a formatted string, using the format specifier and the arguments list.
  * \param ansiDestBuffer The 8-bit destination ansi string buffer
  * \param maxAnsiDestBufferLen The total number of characters available in the destination buffer
  * \param format The string format
  * \param argList The arguments list
  * \return Returns the pointer to the first character in the destination buffer or \ref fpl_null.
  * \note This is most likely just a wrapper call to vsnprintf()
  */
fpl_common_api char *fplFormatAnsiStringArgs(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, va_list argList);

/**
  * \brief Converts the given ansi string into a 32-bit integer constrained by string length
  * \param str The source string
  * \param len The length of the source string
  * \return Returns a 32-bit integer converted from the given string
  */
fpl_common_api int32_t fplStringToS32Len(const char *str, const size_t len);

/**
  * \brief Converts the given ansi string into a 32-bit integer.
  * \param str The source string
  * \return Returns a 32-bit integer converted from the given string
  */
fpl_common_api int32_t fplStringToS32(const char *str);

/**
  * \brief Converts the given 32-bit integer value into a ansi string.
  * \param value The source value
  * \param maxBufferLen The maximum length of the buffer
  * \param buffer The target buffer
  * \return Returns the pointer to the last written character of the buffer or \ref fpl_null.
  */
fpl_common_api char *fplS32ToString(const int32_t value, const size_t maxBufferLen, char *buffer);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Files Files/IO functions
  * \brief This category contains and types and functions for handling files & directories
  * \{
  */
// ----------------------------------------------------------------------------

//! A union containing the internal file handle for any platform
typedef union fplInternalFileHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 file handle
	HANDLE win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix file handle
	int posixFileHandle;
#endif
} fplInternalFileHandle;

//! The file handle structure
typedef struct fplFileHandle {
	//! Internal file handle
	fplInternalFileHandle internalHandle;
	//! File opened successfully
	fpl_b32 isValid;
} fplFileHandle;

//! An enumeration of file position modes (Beginning, Current, End)
typedef enum fplFilePositionMode {
	//! Starts from the beginning
	fplFilePositionMode_Beginning = 0,
	//! Starts from the current position
	fplFilePositionMode_Current,
	//! Starts from the end
	fplFilePositionMode_End
} fplFilePositionMode;

//! An enumeration of file entry types (File, Directory, etc.)
typedef enum fplFileEntryType {
	//! Unknown entry type
	fplFileEntryType_Unknown = 0,
	//! Entry is a file
	fplFileEntryType_File,
	//! Entry is a directory
	fplFileEntryType_Directory
} fplFileEntryType;

//! An enumeration of file permission flags
typedef enum fplFilePermissionFlags {
	//! All (Read, Write, Execute, Search)
	fplFilePermissionFlags_All = 0,
	//! CanExecute
	fplFilePermissionFlags_CanExecuteSearch = 1 << 0,
	//! CanWrite
	fplFilePermissionFlags_CanWrite = 1 << 1,
	//! CanRead
	fplFilePermissionFlags_CanRead = 1 << 2,
} fplFilePermissionFlags;
//! fplFilePermissionFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplFilePermissionFlags);

//! An enumeration of file permission types
typedef enum fplFilePermissionMasks {
	//! No mask
	fplFilePermissionMasks_None = 0,
	//! User
	fplFilePermissionMasks_User = 0xFF0000,
	//! Group
	fplFilePermissionMasks_Group = 0x00FF00,
	//! Owner
	fplFilePermissionMasks_Owner = 0x0000FF,
} fplFilePermissionMasks;
//! fplFilePermissionMasks operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplFilePermissionMasks);

//! A union containing the file permissions (UMask)
typedef union fplFilePermissions {
	struct {
		//! User flags
		uint8_t user;
		//! Group flags
		uint8_t group;
		//! Owner flags
		uint8_t owner;
		//! Unused
		uint8_t unused;
	};
	//! UMask
	uint32_t umask;
} fplFilePermissions;

//! An enumeratation of file attribute flags (Normal, Readonly, Hidden, etc.)
typedef enum fplFileAttributeFlags {
	//! No attributes
	fplFileAttributeFlags_None = 0,
	//! Normal
	fplFileAttributeFlags_Normal = 1 << 1,
	//! Hidden
	fplFileAttributeFlags_Hidden = 1 << 2,
	//! System
	fplFileAttributeFlags_System = 1 << 3,
	//! Archive
	fplFileAttributeFlags_Archive = 1 << 4
} fplFileAttributeFlags;
//! FileAttributeFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplFileAttributeFlags);

//! A union containing the internal file handle for any platform
typedef union fplInternalFileEntryHandle {
#if defined(FPL_PLATFORM_WIN32)
	//! Win32 file handle
	HANDLE win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix directory handle
	DIR *posixDirHandle;
#endif
} fplInternalFileEntryHandle;

//! A structure containing the internal root file informations
typedef struct fplInternalFileRootInfo {
	//! Saved root path
	const char *rootPath;
	//! Saved filter wildcard
	const char *filter;
} fplInternalFileRootInfo;

//! A structure containing filestamps for creation/access/modify date
typedef struct fplFileTimeStamps {
	//! Creation timestamp
	uint64_t creationTime;
	//! Last access timestamp
	uint64_t lastAccessTime;
	//! Last modify timestamp
	uint64_t lastModifyTime;
} fplFileTimeStamps;

//! A structure containing the informations for a file or directory (name, type, attributes, etc.)
typedef struct fplFileEntry {
	//! Full path
	char fullPath[FPL_MAX_PATH_LENGTH];
	//! Internal file handle
	fplInternalFileEntryHandle internalHandle;
	//! Internal root info
	fplInternalFileRootInfo internalRoot;
	//! Time stamps
	fplFileTimeStamps timeStamps;
	//! Permissions
	fplFilePermissions permissions;
	//! Entry type
	fplFileEntryType type;
	//! Attributes
	fplFileAttributeFlags attributes;
	//! Size (Zero when not a file)
	size_t size;
} fplFileEntry;

/**
  * \brief Opens a binary file for reading from a ansi string path and returns the handle of it.
  * \param filePath The ansi file path
  * \param outHandle The pointer to the \ref fplFileHandle structure
  * \return Returns true when binary ansi file was opened, false otherwise.
  */
fpl_platform_api bool fplOpenAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle);
/**
  * \brief Opens a binary file for reading from a wide string path and returns the handle of it.
  * \param filePath The wide file path
  * \param outHandle The pointer to the \ref fplFileHandle structure
  * \return Returns true when binary wide file was opened, false otherwise.
  */
fpl_platform_api bool fplOpenWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle);
/**
  * \brief Create a binary file for writing to the given ansi string path and returns the handle of it.
  * \param filePath The Ansi file path
  * \param outHandle The pointer to the \ref fplFileHandle structure
  * \return Returns true when binary ansi file was created, false otherwise.
  */
fpl_platform_api bool fplCreateAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle);
/**
  * \brief Create a binary file for writing to the given wide string path and returns the handle of it.
  * \param filePath The wide file path
  * \param outHandle The pointer to the \ref fplFileHandle structure
  * \return Returns true when binary wide file was created, false otherwise.
  */
fpl_platform_api bool fplCreateWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle);
/**
  * \brief Reads a block from the given file and returns the number of read bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param sizeToRead The number of bytes to read
  * \param targetBuffer The target memory to write into
  * \param maxTargetBufferSize Total number of bytes available in the target buffer
  * \return Returns the number of bytes read or zero.
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplReadFileBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
/**
  * \brief Reads a block from the given file and returns the number of read bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param sizeToRead The number of bytes to read
  * \param targetBuffer The target memory to write into
  * \param maxTargetBufferSize Total number of bytes available in the target buffer
  * \return Returns the number of bytes read or zero.
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplReadFileBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize);
/**
  * \brief Reads a block from the given file and returns the number of read bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param sizeToRead The number of bytes to read
  * \param targetBuffer The target memory to write into
  * \param maxTargetBufferSize Total number of bytes available in the target buffer
  * \return Returns the number of bytes read or zero.
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_platform_api size_t fplReadFileBlock(const fplFileHandle *fileHandle, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize);
/**
  * \brief Writes a block to the given file and returns the number of written bytes.
  * \param fileHandle The pointer to the file handle \ref fplFileHandle
  * \param sourceBuffer Source memory to read from
  * \param sourceSize Number of bytes to write
  * \return Returns the number of bytes written or zero.
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
/**
  * \brief Writes a block to the given file and returns the number of written bytes.
  * \param fileHandle The pointer to the file handle \ref fplFileHandle
  * \param sourceBuffer Source memory to read from
  * \param sourceSize Number of bytes to write
  * \return Returns the number of bytes written or zero.
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplWriteFileBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize);
/**
  * \brief Writes a block to the given file and returns the number of written bytes.
  * \param fileHandle The pointer to the file handle \ref fplFileHandle
  * \param sourceBuffer Source memory to read from
  * \param sourceSize Number of bytes to write
  * \return Returns the number of bytes written or zero.
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_common_api size_t fplWriteFileBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize);
/**
  * \brief Sets the current file position by the given position, depending on the mode its absolute or relative.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param position Position in bytes
  * \param mode Position mode
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode);
/**
  * \brief Sets the current file position by the given position, depending on the mode its absolute or relative.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param position Position in bytes
  * \param mode Position mode
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplSetFilePosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode);
/**
  * \brief Sets the current file position by the given position, depending on the mode its absolute or relative.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param position Position in bytes
  * \param mode Position mode
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_common_api size_t fplSetFilePosition(const fplFileHandle *fileHandle, const intptr_t position, const fplFilePositionMode mode);
/**
  * \brief Gets the current file position in bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the current file position in bytes.
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle);
/**
  * \brief Gets the current file position in bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the current file position in bytes.
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplGetFilePosition64(const fplFileHandle *fileHandle);
/**
  * \brief Gets the current file position in bytes.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the current file position in bytes.
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_common_api size_t fplGetFilePosition(const fplFileHandle *fileHandle);
/**
  * \brief Closes the given file and releases the underlying resources and clears the handle to zero.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  */
fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle);

// @TODO(final): Add wide file operations

/**
  * \brief Gets the file size in bytes for the given file.
  * \param filePath The ansi path to the file
  * \return Returns the file size in bytes or zero.
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath);
/**
  * \brief Gets the file size in bytes for the given file.
  * \param filePath The ansi path to the file
  * \return Returns the file size in bytes or zero.
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplGetFileSizeFromPath64(const char *filePath);
/**
  * \brief Gets the file size in bytes for the given file.
  * \param filePath The ansi path to the file
  * \return Returns the file size in bytes or zero.
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_platform_api size_t fplGetFileSizeFromPath(const char *filePath);
/**
  * \brief Gets the file size in bytes for a opened file.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the file size in bytes or zero.
  * \note Supports max size of 2^31
  */
fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle);
/**
  * \brief Gets the file size in bytes for a opened file.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the file size in bytes or zero.
  * \note Supports max size of 2^63
  */
fpl_platform_api uint64_t fplGetFileSizeFromHandle64(const fplFileHandle *fileHandle);
/**
  * \brief Gets the file size in bytes for a opened file.
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \return Returns the file size in bytes or zero.
  * \note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
  */
fpl_common_api size_t fplGetFileSizeFromHandle(const fplFileHandle *fileHandle);
/**
  * \brief Gets the timestamps for the given file
  * \param filePath The ansi path to the file
  * \param outStamps The pointer to the \ref fplFileTimeStamps structure
  * \return Returns true when the function succeeded, false otherwise.
  */
fpl_platform_api bool fplGetFileTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps);
/**
  * \brief Gets the timestamps for a opened file
  * \param fileHandle The pointer to the \ref fplFileHandle structure
  * \param outStamps The pointer to the \ref fplFileTimeStamps structure
  * \return Returns true when the function succeeded, false otherwise.
  */
fpl_platform_api bool fplGetFileTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps);
/**
  * \brief Checks if the file exists and returns a boolean indicating the existance.
  * \param filePath The ansi path to the file
  * \return Returns true when the file exists, false otherwise.
  */
fpl_platform_api bool fplFileExists(const char *filePath);
/**
  * \brief Copies the given source file to the target path and returns true when copy was successful.
  * \param sourceFilePath The ansi source file path
  * \param targetFilePath The ansi target file path
  * \param overwrite The overwrite boolean indicating if the file can be overwritten or not
  * \return Returns true when the file was copied, false otherwise.
  */
fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
/**
  * \brief Movies the given source file to the target file and returns true when the move was successful.
  * \param sourceFilePath The ansi source file path
  * \param targetFilePath The ansi target file path
  * \return Returns true when the file was moved, false otherwise.
  */
fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath);
/**
  * \brief Deletes the given file without confirmation and returns true when the deletion was successful.
  * \param filePath The ansi path to the file
  * \return Returns true when the file was deleted, false otherwise.
  */
fpl_platform_api bool fplFileDelete(const char *filePath);

/**
  * \brief Creates all the directories in the given path.
  * \param path The Ansi path to the directory
  * \return Returns true when at least one directory was created, false otherwise.
  */
fpl_platform_api bool fplDirectoriesCreate(const char *path);
/**
  * \brief Checks if the given directory exists and returns a boolean indicating its existance.
  * \param path The ansi path to the directory
  * \return Returns true when the directory exists, false otherwise.
  */
fpl_platform_api bool fplDirectoryExists(const char *path);
/**
  * \brief Deletes the given empty directory without confirmation and returns true when the deletion was successful.
  * \param path The ansi path to the directory.
  * \return Returns true when the empty directory was deleted, false otherwise.
  */
fpl_platform_api bool fplDirectoryRemove(const char *path);
/**
  * \brief Iterates through files / directories in the given directory.
  * \param path The full path
  * \param filter The filter wildcard (If empty or null it will not filter anything at all)
  * \param entry The pointer to the \ref fplFileEntry structure
  * \return Returns true when there was a first entry found, false otherwise.
  * \note This function is not recursive, so it will traverse the first level only!
  * \note When no first entry are found, the resources are automatically cleaned up.
  */
fpl_platform_api bool fplListDirBegin(const char *path, const char *filter, fplFileEntry *entry);
/**
  * \brief Gets the next file entry from iterating through files / directories.
  * \param entry The pointer to the \ref fplFileEntry structure
  * \return Returns true when there was a next file otherwise false if not.
  * \note This function is not recursive, so it will traverse the first level only!
  * \note When no next entry are found, the resources are automatically cleaned up.
  */
fpl_platform_api bool fplListDirNext(fplFileEntry *entry);
/**
  * \brief Releases opened resources from iterating through files / directories.
  * \param entry The pointer to the \ref fplFileEntry structure
  * \note Its safe to call this when the file entry is already closed.
  */
fpl_platform_api void fplListDirEnd(fplFileEntry *entry);

/** \}*/

// ----------------------------------------------------------------------------
/**
  * \defgroup Paths Path functions
  * \brief This category contains functions for retrieving and building paths
  * \{
  */
// ----------------------------------------------------------------------------

// @TODO(final): Support wide strings for 'paths' as well

/**
  * \brief Gets the full path to this executable, including the executable file name.
  * \param destPath The destination buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character in the destination buffer or \ref fpl_null.
  */
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen);
/**
  * \brief Gets the full path to your home directory.
  * \param destPath The destination buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character in the destination buffer or \ref fpl_null.
  */
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen);
/**
  * \brief Extracts the directory path from the given file path.
  * \param sourcePath The source path to extract from
  * \param destPath The destination buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character in the destination buffer or \ref fpl_null.
  */
fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen);
/**
  * \brief Extracts the file extension from the given source path.
  * \param sourcePath The source path to extract from
  * \return Returns the pointer to the first character of the extension.
  */
fpl_common_api const char *fplExtractFileExtension(const char *sourcePath);
/**
  * \brief Extracts the file name including the file extension from the given source path.
  * \param sourcePath The source path to extract from
  * \return Returns the pointer to the first character of the filename.
  */
fpl_common_api const char *fplExtractFileName(const char *sourcePath);
/**
  * \brief Changes the file extension on the given source path and writes the result into a destination buffer.
  * \param filePath The File path to search for the extension
  * \param newFileExtension The new file extension
  * \param destPath The destination buffer
  * \param maxDestLen The total number of characters available in the destination buffer
  * \return Returns the pointer to the last written character in the destination buffer or \ref fpl_null.
  */
fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen);
/**
  * \brief Combines all given paths by the systems path separator.
  * \param destPath The destination buffer
  * \param maxDestPathLen The total number of characters available in the destination buffer
  * \param pathCount The number of dynamic path arguments
  * \param ... The dynamic path arguments
  * \return Returns the pointer to the last written character in the destination buffer or \ref fpl_null.
  */
fpl_common_api char *fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...);

/** \}*/

#if defined(FPL_ENABLE_WINDOW)
// ----------------------------------------------------------------------------
/**
* \defgroup WindowEvents Window events
* \brief This category contains types/functions for handling window events
* \{
*/
// ----------------------------------------------------------------------------

//! An enumeration of mapped keys (Based on MS Virtual-Key-Codes, mostly directly mapped from ASCII)
typedef enum fplKey {
	fplKey_None = 0,

	// 0x0-0x07: Undefined

	fplKey_Backspace = 0x08,
	fplKey_Tab = 0x09,

	// 0x0A-0x0B: Reserved

	fplKey_Clear = 0x0C,
	fplKey_Return = 0x0D,

	// 0x0E-0x0F: Undefined

	fplKey_Shift = 0x10,
	fplKey_Control = 0x11,
	fplKey_Alt = 0x12,
	fplKey_Pause = 0x13,
	fplKey_CapsLock = 0x14,

	// 0x15: IME-Keys
	// 0x16: Undefined
	// 0x17-0x19 IME-Keys
	// 0x1A: Undefined

	fplKey_Escape = 0x1B,

	// 0x1C-0x1F: IME-Keys

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

	fplKey_LeftSuper = 0x5B,
	fplKey_RightSuper = 0x5C,
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

	// 0xA6-0xAC: Dont care

	fplKey_VolumeMute = 0xAD,
	fplKey_VolumeDown = 0xAE,
	fplKey_VolumeUp = 0xAF,
	fplKey_MediaNextTrack = 0xB0,
	fplKey_MediaPrevTrack = 0xB1,
	fplKey_MediaStop = 0xB2,
	fplKey_MediaPlayPause = 0xB3,

	// 0xB4-0xB9 Dont care

	//! '/?' for US
	fplKey_Oem1 = 0xBA,
	//! '+' for any country
	fplKey_OemPlus = 0xBB,
	//! ',' for any country
	fplKey_OemComma = 0xBC,
	//! '-' for any country
	fplKey_OemMinus = 0xBD,
	//! '.' for any country
	fplKey_OemPeriod = 0xBE,
	//! '/?' for US
	fplKey_Oem2 = 0xBF,
	//! '`~' for US
	fplKey_Oem3 = 0xC0,

	// 0xC1-0xD7 Reserved
	// 0xD8-0xDA Unassigned

	//! '[{' for US
	fplKey_Oem4 = 0xDB,
	//! '\|' for US
	fplKey_Oem5 = 0xDC,
	//! ']}' for US
	fplKey_Oem6 = 0xDD,
	//! ''"' for US
	fplKey_Oem7 = 0xDE,
	fplKey_Oem8 = 0xDF,

	// 0xE0-0xFE Dont care
} fplKey;

//! An enumeration of window event types (Resized, PositionChanged, etc.)
typedef enum fplWindowEventType {
	//! None window event type
	fplWindowEventType_None = 0,
	//! Window has been resized
	fplWindowEventType_Resized,
	//! Window got focus
	fplWindowEventType_GotFocus,
	//! Window lost focus
	fplWindowEventType_LostFocus,
	//! Window has been minimized
	fplWindowEventType_Minimized,
	//! Window has been maximized
	fplWindowEventType_Maximized,
	//! Window has been restored
	fplWindowEventType_Restored,
	//! Dropped a single file into the window
	fplWindowEventType_DropSingleFile,
} fplWindowEventType;

//! A structure containing number and dropped files informations
typedef union fplWindowDropFiles {
	//! Single file
	struct {
		//! File path
		char filePath[FPL_MAX_PATH_LENGTH];
	} single;
	//! Number of dropped in files
	size_t fileCount;
} fplWindowDropFiles;

//! A structure containing window event data (Size, Position, etc.)
typedef struct fplWindowEvent {
	//! Window event type
	fplWindowEventType type;
	//! Window width in screen coordinates
	uint32_t width;
	//! Window height in screen coordinates
	uint32_t height;
	//! Drop files
	fplWindowDropFiles dropFiles;
} fplWindowEvent;

//! An enumeration of button states
typedef enum fplButtonState {
	//! Key released
	fplButtonState_Release = 0,
	//! Key pressed
	fplButtonState_Press = 1,
	//! Key is hold down
	fplButtonState_Repeat = 2,
} fplButtonState;

//! An enumeration of keyboard event types
typedef enum fplKeyboardEventType {
	//! None key event type
	fplKeyboardEventType_None = 0,
	//! Key button event
	fplKeyboardEventType_Button,
	//! Character was entered
	fplKeyboardEventType_Input,
} fplKeyboardEventType;

//! An enumeration of keyboard modifier flags
typedef enum fplKeyboardModifierFlags {
	//! No modifiers
	fplKeyboardModifierFlags_None = 0,
	//! Left alt key is down
	fplKeyboardModifierFlags_LAlt = 1 << 0,
	//! Right alt key is down
	fplKeyboardModifierFlags_RAlt = 1 << 1,
	//! Left ctrl key is down
	fplKeyboardModifierFlags_LCtrl = 1 << 2,
	//! Right ctrl key is down
	fplKeyboardModifierFlags_RCtrl = 1 << 3,
	//! Left shift key is down
	fplKeyboardModifierFlags_LShift = 1 << 4,
	//! Right shift key is down
	fplKeyboardModifierFlags_RShift = 1 << 5,
	//! Left super key is down
	fplKeyboardModifierFlags_LSuper = 1 << 6,
	//! Right super key is down
	fplKeyboardModifierFlags_RSuper = 1 << 7,
	//! Capslock state is active
	fplKeyboardModifierFlags_CapsLock = 1 << 8,
	//! Numlock state is active
	fplKeyboardModifierFlags_NumLock = 1 << 9,
	//! Scrolllock state is active
	fplKeyboardModifierFlags_ScrollLock = 1 << 10,
} fplKeyboardModifierFlags;
//! fplKeyboardModifierFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplKeyboardModifierFlags);

//! A structure containing keyboard event data (Type, Keycode, Mapped key, etc.)
typedef struct fplKeyboardEvent {
	//! Raw key code
	uint64_t keyCode;
	//! Keyboard event type
	fplKeyboardEventType type;
	//! Keyboard modifiers
	fplKeyboardModifierFlags modifiers;
	//! Button state
	fplButtonState buttonState;
	//! Mapped key
	fplKey mappedKey;
} fplKeyboardEvent;

//! An enumeration of mouse event types (Move, ButtonDown, ...)
typedef enum fplMouseEventType {
	//! No mouse event type
	fplMouseEventType_None,
	//! Mouse position has been changed
	fplMouseEventType_Move,
	//! Mouse button event
	fplMouseEventType_Button,
	//! Mouse wheel event
	fplMouseEventType_Wheel,
} fplMouseEventType;

//! An enumeration of mouse button types (Left, Right, ...)
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

//! A structure containing mouse event data (Type, Button, Position, etc.)
typedef struct fplMouseEvent {
	//! Mouse event type
	fplMouseEventType type;
	//! Mouse button
	fplMouseButtonType mouseButton;
	//! Button state
	fplButtonState buttonState;
	//! Mouse X-Position
	int32_t mouseX;
	//! Mouse Y-Position
	int32_t mouseY;
	//! Mouse wheel delta
	float wheelDelta;
} fplMouseEvent;

//! An enumeration of gamepad event types (Connected, Disconnected, StateChanged, etc.)
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

//! A structure containing properties for a gamepad button (IsDown, etc.)
typedef struct fplGamepadButton {
	//! Is button down
	fpl_b32 isDown;
} fplGamepadButton;

//! A structure containing the entire gamepad state
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

	//! Is device connected
	fpl_b32 isConnected;
} fplGamepadState;

//! A structure containing gamepad event data (Type, Device, State, etc.)
typedef struct fplGamepadEvent {
	//! Full gamepad state
	fplGamepadState state;
	//! Gamepad event type
	fplGamepadEventType type;
	//! Gamepad device index
	uint32_t deviceIndex;
} fplGamepadEvent;

//! An enumeration of event types (Window, Keyboard, Mouse, ...)
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

//! A structure containing event data for all event types (Window, Keyboard, Mouse, etc.)
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
  * \brief Polls the next event from the internal event queue and removes it.
  * \param ev The pointer to the \ref fplEvent structure
  * \return Returns false when there are no events left, true otherwise.
  */
fpl_common_api bool fplPollEvent(fplEvent *ev);
/**
  * \brief Removes all the events from the internal event queue.
  * \note Dont call when you care about any event!
  */
fpl_common_api void fplClearEvents();
/**
  * \brief Reads the next window event from the OS and pushes it into the internal queue.
  * \return Returns true when there was a event from the OS, false otherwise.
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
  * \defgroup InputData Input functions
  * \brief This category contains functions for retrieving input data
  * \{
  */

//! A struct containing the full keyboard state
typedef struct fplKeyboardState {
	//! Modifier flags
	fplKeyboardModifierFlags modifiers;
	//! Key states
	fpl_b32 keyStatesRaw[256];
	//! Mapped button states
	fplButtonState buttonStatesMapped[256];
} fplKeyboardState;

//! A struct containing the game controller device states
typedef struct fplGamepadStates {
	//! Device states
	fplGamepadState deviceStates[4];
} fplGamepadStates;

/**
  * \brief Gets current keyboard state and writes it out into the given structure.
  * \param outState The pointer to the \ref fplKeyboardState structure
  */
fpl_platform_api bool fplGetKeyboardState(fplKeyboardState *outState);

/**
  * \brief Gets current gamepad states and writes it out into the given structure.
  * \param outStates The pointer to the \ref fplGamepadStates structure
  */
fpl_platform_api bool fplGetGamepadStates(fplGamepadStates *outStates);

/*\}*/

/**
  * \defgroup WindowBase Window functions
  * \brief This category contains functions for handling the window
  * \{
  */

//! A structure containing the size a window
typedef struct fplWindowSize {
	//! Width in screen coordinates
	uint32_t width;
	//! Height in screen coordinates
	uint32_t height;
} fplWindowSize;

//! A structure containing the position of a window
typedef struct fplWindowPosition {
	//! Left position in screen coordinates
	int32_t left;
	//! Top position in screen coordinates
	int32_t top;
} fplWindowPosition;

/**
  * \brief Gets the window running state as boolean.
  * \return Returns true when the window is running, false otherwise.
  */
fpl_platform_api bool fplIsWindowRunning();
/**
  * \brief Closes the window and stops the event loop
  */
fpl_platform_api void fplWindowShutdown();
/**
  * \brief Processes the message queue of the window.
  * \return Returns true when the window is still active, false otherwise.
  * \note This will update the game controller states as well.
  */
fpl_platform_api bool fplWindowUpdate();
/**
  * \brief Enables or disables the window cursor.
  * \param value The new cursor visibility state
  */
fpl_platform_api void fplSetWindowCursorEnabled(const bool value);
/**
  * \brief Retrieves the inner window area.
  * \param outSize The pointer to the \ref fplWindowSize structure
  * \return Returns true when we got the window area from the current window, false otherwise.
  */
fpl_platform_api bool fplGetWindowArea(fplWindowSize *outSize);
/**
  * \brief Resizes the window to fit the inner area to the given size.
  * \param width The width in screen units
  * \param height The height in screen units
  */
fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height);
/**
  * \brief Gets the window resizable state as boolean.
  * \return Returns true when the window is resizable, false otherwise.
  */
fpl_platform_api bool fplIsWindowResizable();
/**
  * \brief Enables or disables the ability to resize the window.
  * \param value The new resizable state
  */
fpl_platform_api void fplSetWindowResizeable(const bool value);
/**
  * \brief Gets the window decorated state as boolean.
  * \return Returns true when the window is decorated, false otherwise.
  */
fpl_platform_api bool fplIsWindowDecorated();
/**
  * \brief Enables or disables the window decoration (Titlebar, Border, etc.).
  * \param value The new decorated state
  */
fpl_platform_api void fplSetWindowDecorated(const bool value);
/**
  * \brief Gets the window floating state as boolean.
  * \return Returns true when the window is floating, false otherwise.
  */
fpl_platform_api bool fplIsWindowFloating();
/**
  * \brief Enables or disables the window floating (Top-most)
  * \param value The new floating state
  */
fpl_platform_api void fplSetWindowFloating(const bool value);
/**
  * \brief Enables or disables fullscreen mode.
  * \param value The new fullscreen state
  * \param fullscreenWidth The fullscreen width in screen units. When set to zero the desktop default is being used.
  * \param fullscreenHeight The fullscreen height in screen units. When set to zero the desktop default is being used.
  * \param refreshRate The refresh rate in Hz. When set to zero the desktop default is being used.
  * \return Returns true when the window was changed to the desire fullscreen mode, false otherwise.
  */
fpl_platform_api bool fplSetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate);
/**
  * \brief Gets the window fullscreen state as boolean.
  * \return Returns true when the window is in fullscreen mode, false otherwise.
  */
fpl_platform_api bool fplIsWindowFullscreen();
/**
  * \brief Retrieves the absolute window position.
  * \param outPos The pointer to the \ref fplWindowPosition structure
  * \return Returns true when we got the position, false otherwise.
  */
fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos);
/**
  * \brief Changes the window absolut position to the given coordinates.
  * \param left The left position in screen units
  * \param top The top position in screen units
  */
fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top);
/**
  * \brief Changes the window title to the given ansi string.
  * \param ansiTitle The title ansi string
  */
fpl_platform_api void fplSetWindowAnsiTitle(const char *ansiTitle);
/**
  * \brief Changes the window title to the given wide string.
  * \param wideTitle The title wide string
  */
fpl_platform_api void fplSetWindowWideTitle(const wchar_t *wideTitle);

/*\}*/

/**
  * \defgroup WindowClipboard Clipboard functions
  * \brief This category contains functions for reading/writing clipboard data
  * \{
  */

  /**
	* \brief Retrieves the current clipboard ansi text.
	* \param dest The destination ansi string buffer to write the clipboard text into.
	* \param maxDestLen The total number of characters available in the destination buffer.
	* \return Returns true when the clipboard contained text which is copied into the dest buffer, \ref fpl_null otherwise.
	*/
fpl_platform_api bool fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
/**
  * \brief Retrieves the current clipboard wide text.
  * \param dest The destination wide string buffer to write the clipboard text into.
  * \param maxDestLen The total number of characters available in the destination buffer.
  * \return Returns true when the clipboard contained text which is copied into the dest buffer, \ref fpl_null otherwise.
  */
fpl_platform_api bool fplGetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
/**
  * \brief Overwrites the current clipboard ansi text with the given one.
  * \param ansiSource The new clipboard ansi string.
  * \return Returns true when the text in the clipboard was changed, false otherwise.
  */
fpl_platform_api bool fplSetClipboardAnsiText(const char *ansiSource);
/**
  * \brief Overwrites the current clipboard wide text with the given one.
  * \param wideSource The new clipboard wide string.
  * \return Returns true when the text in the clipboard was changed, false otherwise.
  */
fpl_platform_api bool fplSetClipboardWideText(const wchar_t *wideSource);

/** \}*/
#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_VIDEO)
// ----------------------------------------------------------------------------
/**
  * \defgroup Video Video functions
  * \brief This category contains functions for retrieving or resizing the video buffer
  * \{
  */
// ----------------------------------------------------------------------------

//! A structure defining a video rectangles position and size
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
  * \brief Makes a video rectangle from a LTRB rectangle
  * \param left The left position in screen units
  * \param top The top position in screen units
  * \param right The right position in screen units
  * \param bottom The bottom position in screen units
  * \return Returns the computed video rectangle \ref fplVideoRect
  */
fpl_inline fplVideoRect fplCreateVideoRectFromLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom) {
	fplVideoRect result = { left, top, (right - left) + 1, (bottom - top) + 1 };
	return(result);
}

//! A structure containing video backbuffer properties
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
	fpl_b32 useOutputRect;
} fplVideoBackBuffer;

/**
  * \brief Gets a string which represents the given video driver
  * \param driver The video driver type \ref fplVideoDriverType
  * \return Returns a string for the given video driver type
  */
fpl_common_api const char *fplGetVideoDriverString(fplVideoDriverType driver);
/**
  * \brief Retrieves the pointer to the current video backbuffer.
  * \return Returns the pointer to the current \ref fplVideoBackBuffer.
  * \warning Do not release this memory by any means, otherwise you will corrupt heap memory!
  */
fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer();
/**
  * \brief Resizes the current video backbuffer.
  * \param width The width in pixels
  * \param height The height in pixels
  * \return Returns true when video back buffer could be resized, false otherwise.
  */
fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height);
/**
  * \brief Gets the current video driver
  * \return Returns the current video driver type \ref fplVideoDriverType
  */
fpl_common_api fplVideoDriverType fplGetVideoDriver();

/**
  * \brief Forces the window to be redrawn or to swap the back/front buffer.
  */
fpl_common_api void fplVideoFlip();

/** \}*/
#endif // FPL_ENABLE_VIDEO

#if defined(FPL_ENABLE_AUDIO)
// ----------------------------------------------------------------------------
/**
  * \defgroup Audio Audio functions
  * \brief This category contains functions for start/stop playing audio and retrieving/changing some audio related settings.
  * \{
  */
// ----------------------------------------------------------------------------

//! An enumeration of audio results
typedef enum fplAudioResult {
	fplAudioResult_None = 0,
	fplAudioResult_Success,
	fplAudioResult_DeviceNotInitialized,
	fplAudioResult_DeviceAlreadyStopped,
	fplAudioResult_DeviceAlreadyStarted,
	fplAudioResult_DeviceBusy,
	fplAudioResult_NoDeviceFound,
	fplAudioResult_ApiFailed,
	fplAudioResult_PlatformNotInitialized,
	fplAudioResult_Failed,
} fplAudioResult;

/**
  * \brief Start playing asyncronous audio.
  * \return Returns the audio result \ref fplAudioResult
  */
fpl_common_api fplAudioResult fplPlayAudio();
/**
  * \brief Stop playing asyncronous audio.
  * \return Returns the audio result \ref fplAudioResult
  */
fpl_common_api fplAudioResult fplStopAudio();
/**
  * \brief Retrieves the native format for the current audio device.
  * \param outFormat The pointer to the \ref fplAudioDeviceFormat structure
  * \return Returns true when a hardware format was active, false otherwise.
  */
fpl_common_api bool fplGetAudioHardwareFormat(fplAudioDeviceFormat *outFormat);
/**
  * \brief Overwrites the audio client read callback.
  * \param newCallback The pointer to the \ref fpl_audio_client_read_callback callback
  * \param userData The pointer to the client/user data
  * \return Returns true when a audio device is ready and the callback was set, false otherwise.
  * \note This has no effect when audio is already playing, you have to call it when audio is in a stopped state!
  */
fpl_common_api bool fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData);
/**
  * \brief Retrieves all playback audio devices.
  * \param devices A array of audio device info \ref fplAudioDeviceInfo
  * \param maxDeviceCount The total number of devices available in the devices array.
  * \return Returns the number of devices found.
  */
fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount);
/**
  * \brief Computes the number of bytes required to write one sample with one channel.
  * \param format The audio format type \ref fplAudioFormatType
  * \return Returns the number of bytes for one sample with one channel
  */
fpl_common_api uint32_t fplGetAudioSampleSizeInBytes(const fplAudioFormatType format);
/**
  * \brief Gets the string which represents the given audio format type.
  * \param format The audio format type \ref fplAudioFormatType
  * \return Returns a string for the given audio format type
  */
fpl_common_api const char *fplGetAudioFormatString(const fplAudioFormatType format);
/**
  * \brief Gets the string which represents the given audio driver type.
  * \param driver The audio driver type \ref fplAudioDriverType
  * \return Returns a string for the given audio driver type
  */
fpl_common_api const char *fplGetAudioDriverString(fplAudioDriverType driver);
/**
  * \brief Computes the total number of frames for given sample rate and buffer size.
  * \param sampleRate The sample rate in Hz
  * \param bufferSizeInMilliSeconds The buffer size in milliseconds
  * \return Returns the total number of frames for given sample rate and buffer size
  */
fpl_common_api uint32_t fplGetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds);
/**
  * \brief Computes the number of bytes required for one interleaved audio frame - containing all the channels.
  * \param format The audio format
  * \param channelCount The number of channels
  * \return Returns the number of bytes for one frame in bytes
  */
fpl_common_api uint32_t fplGetAudioFrameSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount);
/**
  * \brief Computes the total number of bytes for the buffer and the given parameters
  * \param format The audio format
  * \param channelCount The number of channels
  * \param frameCount The number of frames
  * \return Returns the total number of bytes for the buffer
  */
fpl_common_api uint32_t fplGetAudioBufferSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount, const uint32_t frameCount);

/** \}*/
#endif // FPL_ENABLE_AUDIO

// ----------------------------------------------------------------------------
/**
  * \defgroup Internationalisation Internationalisation functions
  * \brief This category contains functions for getting informations about current locale
  * \{
  */
// ----------------------------------------------------------------------------

//! A enumeration of locale formats
typedef enum fplLocaleFormat {
	//! No locale format
	fplLocaleFormat_None = 0,
	//! ISO-639 format (de-DE, en-US, etc.)
	fplLocaleFormat_ISO639,
} fplLocaleFormat;

/**
  * \brief Gets the user locale in the given target format
  * \param targetFormat Target \ref fplLocaleFormat
  * \param buffer Target string buffer for writing the locale into
  * \param maxBufferLen The maximum length of the buffer
  * \return Returns true when the function succeeds, false otherwise
  */
fpl_platform_api bool fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/**
  * \brief Gets the system locale in the given target format
  * \param targetFormat Target \ref fplLocaleFormat
  * \param buffer Target string buffer for writing the locale into
  * \param maxBufferLen The maximum length of the buffer
  * \return Returns true when the function succeeds, false otherwise
  */
fpl_platform_api bool fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/**
  * \brief Gets the input locale in the given target format
  * \param targetFormat Target \ref fplLocaleFormat
  * \param buffer Target string buffer for writing the locale into
  * \param maxBufferLen The maximum length of the buffer
  * \return Returns true when the function succeeds, false otherwise
  */
fpl_platform_api bool fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/** \}*/


// Ignore any doxygen documentation from here
//! \cond FPL_IGNORE_DOXYGEN

// ****************************************************************************
//
// > EXPORT
//
// Internal functions for static library
// Entry-Point forward declaration
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)
//
// Export internal functions (Static Library)
//
typedef struct fpl__Win32CommandLineUTF8Arguments {
	void *mem;
	char **args;
	uint32_t count;
} fpl__Win32CommandLineUTF8Arguments;
fpl_api fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseWideArguments(LPWSTR cmdLine);
#	if defined(FPL_ENTRYPOINT)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_main int main(int argc, char **args);
#	endif
#endif // FPL_PLATFORM_WIN32

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
// > UTILITY_FUNCTIONS
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

// Module constants used for logging
#define FPL__MODULE_CORE "Core"
#define FPL__MODULE_FILES "Files"
#define FPL__MODULE_THREADING "Threading"
#define FPL__MODULE_MEMORY "Memory"
#define FPL__MODULE_WINDOW "Window"
#define FPL__MODULE_LIBRARIES "Libraries"
#define FPL__MODULE_OS "OS"
#define FPL__MODULE_HARDWARE "Hardware"
#define FPL__MODULE_STRINGS "Strings"
#define FPL__MODULE_PATHS "Paths"
#define FPL__MODULE_ARGS "Arguments"

#define FPL__MODULE_AUDIO "Audio"
#define FPL__MODULE_AUDIO_DIRECTSOUND "DirectSound"
#define FPL__MODULE_AUDIO_ALSA "ALSA"

#define FPL__MODULE_VIDEO "Video"
#define FPL__MODULE_VIDEO_OPENGL "OpenGL"
#define FPL__MODULE_VIDEO_SOFTWARE "Software"

#define FPL__MODULE_WIN32 "Win32"
#define FPL__MODULE_XINPUT "XInput"

#define FPL__MODULE_LINUX "Linux"
#define FPL__MODULE_UNIX "Unix"
#define FPL__MODULE_POSIX "POSIX"
#define FPL__MODULE_PTHREAD "pthread"
#define FPL__MODULE_X11 "X11"
#define FPL__MODULE_GLX "GLX"

//
// Compiler warnings
//
#if defined(FPL_COMPILER_MSVC)
	//! Don't spill our preferences to the outside
#	pragma warning( push )

	//! Disable noexcept compiler warning for C++
#	pragma warning( disable : 4577 )
	//! Disable "switch statement contains 'default' but no 'case' labels" compiler warning for C++
#	pragma warning( disable : 4065 )
	//! Disable "conditional expression is constant" warning
#	pragma warning( disable : 4127 )
	//! Disable "unreferenced formal parameter" warning
#	pragma warning( disable : 4100 )
	//! Disable "nonstandard extension used: nameless struct/union" warning
#	pragma warning( disable : 4201 )
	//! Disable "local variable is initialized but not referenced" warning
#	pragma warning( disable : 4189 )
	//! Disable "nonstandard extension used: non-constant aggregate initializer" warning
#	pragma warning( disable : 4204 )
#endif // FPL_COMPILER

// Only include C-Runtime functions when CRT is enabled
#if !defined(FPL_NO_CRT)
#	include <stdio.h> // stdin, stdout, stderr, fprintf, vfprintf, vsnprintf, getchar
#	include <stdlib.h> // wcstombs, mbstowcs, getenv
#	include <locale.h> // setlocale, struct lconv, localeconv
#endif

//
// Internal logging system
//
#define FPL__MODULE_CONCAT(mod, format) "[" mod "] " format

#if defined(FPL_ENABLE_LOGGING)
fpl_globalvar fplLogSettings fpl__global__LogSettings = FPL_ZERO_INIT;

fpl_internal void fpl__LogWrite(const fplLogLevel level, const char *message) {
	fplLogSettings *settings = &fpl__global__LogSettings;
	if(!settings->isInitialized) {
#if defined(FPL_LOG_MULTIPLE_WRITERS)
		settings->criticalWriter.console.logToError = true;
		settings->criticalWriter.flags = fplLogWriterFlags_Console | fplLogWriterFlags_DebugOut;
		settings->errorWriter = settings->criticalWriter;
		settings->warningWriter = settings->criticalWriter;
		settings->infoWriter.flags = fplLogWriterFlags_Console | fplLogWriterFlags_DebugOut;
		settings->verboseWriter = settings->infoWriter;
		settings->debugWriter.flags = fplLogWriterFlags_DebugOut;
#else
		settings->writers[0].flags = fplLogWriterFlags_Console | fplLogWriterFlags_DebugOut;
#endif
		settings->isInitialized = true;
	}

	if((settings->maxLevel == -1) || (level <= settings->maxLevel)) {
#if defined(FPL_LOG_MULTIPLE_WRITERS)
		FPL_ASSERT(level < FPL_ARRAYCOUNT(settings->writers));
		const fplLogWriter *writer = &settings->writers[(int)level];
#else
		const fplLogWriter *writer = &settings->writers[0];
#endif
		const char *levelStr;
		switch(level) {
			case fplLogLevel_Critical:
				levelStr = "CRITICAL";
				break;
			case fplLogLevel_Error:
				levelStr = "ERROR";
				break;
			case fplLogLevel_Warning:
				levelStr = "WARNING";
				break;
			case fplLogLevel_Info:
				levelStr = "INFO";
				break;
			case fplLogLevel_Verbose:
				levelStr = "VERBOSE";
				break;
			case fplLogLevel_Debug:
				levelStr = "DEBUG";
				break;
			default:
				levelStr = "NONE";
				break;
		}
		if(writer->flags & fplLogWriterFlags_Console) {
			if(writer->console.logToError) {
				fplConsoleFormatError("[%s]%s\n", levelStr, message);
			} else {
				fplConsoleFormatOut("[%s]%s\n", levelStr, message);
			}
		}
		if(writer->flags & fplLogWriterFlags_DebugOut) {
			fplDebugFormatOut("[%s]%s\n", levelStr, message);
		}
		if(writer->flags & fplLogWriterFlags_Custom && writer->custom.callback != fpl_null) {
			writer->custom.callback(level, message);
		}
	}
}
fpl_internal_inline void fpl__LogWriteArgs(const fplLogLevel level, const char *format, va_list argList) {
	char buffer[1024];
	fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	fpl__LogWrite(level, buffer);
}
fpl_internal_inline void fpl__LogWriteVarArgs(const fplLogLevel level, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	fpl__LogWriteArgs(level, format, argList);
	va_end(argList);
}
#	define FPL_LOG(lvl, mod, format, ...) fpl__LogWriteVarArgs(lvl, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)
#	define FPL_LOG_CRITICAL(mod, format, ...) FPL_LOG(fplLogLevel_Critical, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_ERROR(mod, format, ...) FPL_LOG(fplLogLevel_Error, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_WARN(mod, format, ...) FPL_LOG(fplLogLevel_Warning, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_INFO(mod, format, ...) FPL_LOG(fplLogLevel_Info, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_VERBOSE(mod, format, ...) FPL_LOG(fplLogLevel_Verbose, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_DEBUG(mod, format, ...) FPL_LOG(fplLogLevel_Debug, mod, format, ## __VA_ARGS__)
#   define FPL_LOG_FUNCTION_N(mod, name) FPL_LOG(fplLogLevel_Debug, mod, "-> %s()", name)
#   define FPL_LOG_FUNCTION(mod) FPL_LOG_FUNCTION_N(mod, FPL_FUNCTION_NAME)
#else
#	define FPL_LOG(lvl, mod, format, ...)
#	define FPL_LOG_CRITICAL(mod, format, ...)
#	define FPL_LOG_ERROR(mod, format, ...)
#	define FPL_LOG_WARN(mod, format, ...)
#   define FPL_LOG_INFO(mod, format, ...)
#   define FPL_LOG_VERBOSE(mod, format, ...)
#   define FPL_LOG_DEBUG(mod, format, ...)
#   define FPL_LOG_FUNCTION(mod)
#endif

//
// Error handling
//

#define FPL_CRITICAL(mod, format, ...)  fpl__PushError(fplLogLevel_Critical, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)
#define FPL_ERROR(mod, format, ...) fpl__PushError(fplLogLevel_Error, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)

//
// Debug out
//
#if defined(FPL_PLATFORM_WIN32)
fpl_platform_api void fplDebugOut(const char *text) {
	OutputDebugStringA(text);
}
#else
fpl_platform_api void fplDebugOut(const char *text) {
	// @NOTE(final): For now this does nothing on all other platforms
}
#endif
fpl_common_api void fplDebugFormatOut(const char *format, ...) {
	if(format != fpl_null) {
		char buffer[1024];
		va_list argList;
		va_start(argList, format);
		fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
		va_end(argList);
		fplDebugOut(buffer);
	}
}

// ############################################################################
//
// > PLATFORM_CONSTANTS
//
// ############################################################################
#if !defined(FPL_PLATFORM_CONSTANTS_DEFINED)
#define FPL_PLATFORM_CONSTANTS_DEFINED

// One cacheline worth of padding
#define FPL__ARBITARY_PADDING 64

fpl_globalvar struct fpl__PlatformAppState *fpl__global__AppState = fpl_null;

fpl_internal void fpl__PushError(const fplLogLevel level, const char *format, ...);
#endif // FPL_PLATFORM_CONSTANTS_DEFINED

// ############################################################################
//
// > UTILITY_FUNCTIONS
//
// ############################################################################
fpl_internal bool fpl__AddLineWhenAnyMatches(const char *line, const char **wildcards, const size_t maxWildcardCount, const size_t maxLineSize, const size_t maxLineCount, char **outLines, size_t *outCount) {
	for(size_t i = 0; i < maxWildcardCount; ++i) {
		const char *wildcard = wildcards[i];
		if(fplIsStringMatchWildcard(line, wildcard)) {
			size_t index = *outCount;
			char *target = outLines[index];
			fplCopyAnsiString(line, target, maxLineSize);
			*outCount = index + 1;
			break;
		}
	}
	bool result = *outCount < maxLineCount;
	return(result);
}

fpl_internal size_t fpl__ParseTextFile(const char *filePath, const char **wildcards, const size_t maxWildcardCount, const size_t maxLineSize, const size_t maxLineCount, char **outLines) {
	if(filePath == fpl_null || wildcards == fpl_null || maxWildcardCount == 0 || maxLineSize == 0 || maxLineCount == 0 || outLines == fpl_null) {
		return(0);
	}
	// @NOTE(final): Forced Zero-Terminator is not nessecary here, but we do it here to debug it better
	// This function supports maxLineSize < FPL_ARRAYCOUNT(buffer)
	// We allocate the line buffer on the stack because we do not know how large the line will be on compile time
	size_t result = 0;
	fplFileHandle fileHandle = FPL_ZERO_INIT;
	if(fplOpenAnsiBinaryFile(filePath, &fileHandle)) {
		char *line = (char *)FPL_STACKALLOCATE(maxLineSize);
		char buffer[256 + 1];
		const size_t maxBufferSize = FPL_ARRAYCOUNT(buffer) - 1;
		size_t bytesRead = 0;
		size_t posLineBytes = 0;
		bool done = false;
		while(!done && ((bytesRead = fplReadFileBlock(&fileHandle, maxBufferSize, &buffer[0], maxBufferSize)) > 0)) {
			buffer[bytesRead] = 0;
			char *start = &buffer[0];
			char *p = start;
			size_t readPos = 0;
			size_t lineSizeToRead = 0;
			while(readPos < bytesRead) {
				if(*p == '\n') {
					size_t remainingLineBytes = maxLineSize - posLineBytes;
					char *lineTargetP = line + posLineBytes;
					if(lineSizeToRead < remainingLineBytes) {
						fplCopyAnsiStringLen(start, lineSizeToRead, lineTargetP, remainingLineBytes);
					} else {
						fplCopyAnsiStringLen(start, remainingLineBytes - 1, lineTargetP, remainingLineBytes);
					}
					if(!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
						done = true;
						break;
					}
					start = p + 1;
					line[0] = 0;
					lineSizeToRead = 0;
					posLineBytes = 0;
				} else {
					++lineSizeToRead;
				}
				++p;
				++readPos;
			}
			if(done) {
				break;
			}
			if(lineSizeToRead > 0) {
				size_t remainingLineBytes = maxLineSize - posLineBytes;
				char *lineTargetP = line + posLineBytes;
				if(lineSizeToRead < remainingLineBytes) {
					fplCopyAnsiStringLen(start, lineSizeToRead, lineTargetP, remainingLineBytes);
					posLineBytes += lineSizeToRead;
					if(bytesRead <= maxBufferSize) {
						if(!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
							done = true;
						}
					}
				} else {
					fplCopyAnsiStringLen(start, remainingLineBytes - 1, lineTargetP, remainingLineBytes);
					line[0] = 0;
					lineSizeToRead = 0;
					posLineBytes = 0;
					if(!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
						done = true;
					}
				}
			}
		}
		fplCloseFile(&fileHandle);
	}
	return(result);
}

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
#	include <shellapi.h>	// HDROP

#	if defined(FPL_IS_CPP)
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(a, b)
#	else
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(&a, &b)
#	endif

// Little macro to not write 5 lines of code all the time
#define FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(mod, libHandle, libName, target, type, name) \
	target = (type *)GetProcAddress(libHandle, name); \
	if (target == fpl_null) { \
		FPL_ERROR(mod, "Failed getting procedure address '%s' from library '%s'", name, libName); \
		return false; \
	}
#define FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK(mod, libHandle, libName, target, type, name) \
	target = (type *)GetProcAddress(libHandle, name); \
	if (target == fpl_null) { \
		FPL_ERROR(mod, "Failed getting procedure address '%s' from library '%s'", name, libName); \
		break; \
	}

//
// XInput
//
#define FPL__FUNC_XINPUT_XInputGetState(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef FPL__FUNC_XINPUT_XInputGetState(fpl__win32_func_XInputGetState);
FPL__FUNC_XINPUT_XInputGetState(fpl__Win32XInputGetStateStub) {
	return(ERROR_DEVICE_NOT_CONNECTED);
}
#define FPL__FUNC_XINPUT_XInputGetCapabilities(name) DWORD WINAPI name(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
typedef FPL__FUNC_XINPUT_XInputGetCapabilities(fpl__win32_func_XInputGetCapabilities);
FPL__FUNC_XINPUT_XInputGetCapabilities(fpl__Win32XInputGetCapabilitiesStub) {
	return(ERROR_DEVICE_NOT_CONNECTED);
}
typedef struct fpl__Win32XInputApi {
	HMODULE xinputLibrary;
	fpl__win32_func_XInputGetState *xInputGetState;
	fpl__win32_func_XInputGetCapabilities *xInputGetCapabilities;
} fpl__Win32XInputApi;

fpl_internal void fpl__Win32UnloadXInputApi(fpl__Win32XInputApi *xinputApi) {
	FPL_ASSERT(xinputApi != fpl_null);
	if(xinputApi->xinputLibrary) {
		FPL_LOG_DEBUG("XInput", "Unload XInput Library");
		FreeLibrary(xinputApi->xinputLibrary);
		xinputApi->xinputLibrary = fpl_null;
		xinputApi->xInputGetState = fpl__Win32XInputGetStateStub;
		xinputApi->xInputGetCapabilities = fpl__Win32XInputGetCapabilitiesStub;
	}
}

fpl_internal void fpl__Win32LoadXInputApi(fpl__Win32XInputApi *xinputApi) {
	FPL_ASSERT(xinputApi != fpl_null);

	FPL_LOG_DEBUG("XInput", "Load XInput Library");

	// Windows 8
	HMODULE xinputLibrary = LoadLibraryA("xinput1_4.dll");
	if(!xinputLibrary) {
		// Windows 7
		xinputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if(!xinputLibrary) {
		// Windows Generic
		xinputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if(xinputLibrary) {
		xinputApi->xinputLibrary = xinputLibrary;
		xinputApi->xInputGetState = (fpl__win32_func_XInputGetState *)GetProcAddress(xinputLibrary, "XInputGetState");
		xinputApi->xInputGetCapabilities = (fpl__win32_func_XInputGetCapabilities *)GetProcAddress(xinputLibrary, "XInputGetCapabilities");
	}
	if(xinputApi->xInputGetState == fpl_null) {
		xinputApi->xInputGetState = fpl__Win32XInputGetStateStub;
	}
	if(xinputApi->xInputGetCapabilities == fpl_null) {
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
#define FPL__FUNC_WIN32_CreateDIBSection(name) HBITMAP WINAPI name(HDC hdc, CONST BITMAPINFO *pbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset)
typedef FPL__FUNC_WIN32_CreateDIBSection(fpl__win32_func_CreateDIBSection);
#define FPL__FUNC_WIN32_CreateBitmap(name) HBITMAP WINAPI name(int nWidth, int nHeight, UINT nPlanes, UINT nBitCount, CONST VOID *lpBits)
typedef FPL__FUNC_WIN32_CreateBitmap(fpl__win32_func_CreateBitmap);

// ShellAPI
#define FPL__FUNC_WIN32_CommandLineToArgvW(name) LPWSTR* WINAPI name(LPCWSTR lpCmdLine, int *pNumArgs)
typedef FPL__FUNC_WIN32_CommandLineToArgvW(fpl__win32_func_CommandLineToArgvW);
#define FPL__FUNC_WIN32_SHGetFolderPathA(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
typedef FPL__FUNC_WIN32_SHGetFolderPathA(fpl__win32_func_SHGetFolderPathA);
#define FPL__FUNC_WIN32_SHGetFolderPathW(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
typedef FPL__FUNC_WIN32_SHGetFolderPathW(fpl__win32_func_SHGetFolderPathW);
#define FPL__FUNC_WIN32_DragQueryFileA(name) UINT WINAPI name(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch)
typedef FPL__FUNC_WIN32_DragQueryFileA(fpl__win32_func_DragQueryFileA);
#define FPL__FUNC_WIN32_DragQueryFileW(name) UINT WINAPI name(HDROP hDrop, UINT iFile, LPWSTR lpszFile, UINT cch)
typedef FPL__FUNC_WIN32_DragQueryFileW(fpl__win32_func_DragQueryFileW);
#define FPL__FUNC_WIN32_DragAcceptFiles(name) void WINAPI name(HWND hWnd, BOOL fAccept)
typedef FPL__FUNC_WIN32_DragAcceptFiles(fpl__win32_func_DragAcceptFiles);

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
#define FPL__FUNC_WIN32_ClientToScreen(name) BOOL WINAPI name(HWND hWnd, LPPOINT lpPoint)
typedef FPL__FUNC_WIN32_ClientToScreen(fpl__win32_func_ClientToScreen);
#define FPL__FUNC_WIN32_GetAsyncKeyState(name) SHORT WINAPI name(int vKey)
typedef FPL__FUNC_WIN32_GetAsyncKeyState(fpl__win32_func_GetAsyncKeyState);
#define FPL__FUNC_WIN32_GetKeyState(name) SHORT WINAPI name(int vKey)
typedef FPL__FUNC_WIN32_GetKeyState(fpl__win32_func_GetKeyState);
#define FPL__FUNC_WIN32_MapVirtualKeyA(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_WIN32_MapVirtualKeyA(fpl__win32_func_MapVirtualKeyA);
#define FPL__FUNC_WIN32_MapVirtualKeyW(name) UINT WINAPI name(UINT uCode, UINT uMapType)
typedef FPL__FUNC_WIN32_MapVirtualKeyW(fpl__win32_func_MapVirtualKeyW);
#define FPL__FUNC_WIN32_SetCursor(name) HCURSOR WINAPI name(HCURSOR hCursor)
typedef FPL__FUNC_WIN32_SetCursor(fpl__win32_func_SetCursor);
#define FPL__FUNC_WIN32_GetCursor(name) HCURSOR WINAPI name(VOID)
typedef FPL__FUNC_WIN32_GetCursor(fpl__win32_func_GetCursor);
#define FPL__FUNC_WIN32_GetCursorPos(name) BOOL WINAPI name(LPPOINT lpPoint)
typedef FPL__FUNC_WIN32_GetCursorPos(fpl__win32_func_GetCursorPos);
#define FPL__FUNC_WIN32_WindowFromPoint(name) HWND WINAPI name(POINT Point)
typedef FPL__FUNC_WIN32_WindowFromPoint(fpl__win32_func_WindowFromPoint);
#define FPL__FUNC_WIN32_PtInRect(name) BOOL WINAPI name(CONST RECT *lprc, POINT pt)
typedef FPL__FUNC_WIN32_PtInRect(fpl__win32_func_PtInRect);
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
#define FPL__FUNC_WIN32_IsIconic(name) BOOL WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_IsIconic(fpl__win32_func_IsIconic);
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
#define FPL__FUNC_WIN32_RegisterRawInputDevices(name) BOOL WINAPI name(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
typedef FPL__FUNC_WIN32_RegisterRawInputDevices(fpl__win32_func_RegisterRawInputDevices);
#define FPL__FUNC_WIN32_ClipCursor(name) BOOL WINAPI name(CONST RECT *lpRect)
typedef FPL__FUNC_WIN32_ClipCursor(fpl__win32_func_ClipCursor);
#define FPL__FUNC_WIN32_PostQuitMessage(name) VOID WINAPI name(int nExitCode)
typedef FPL__FUNC_WIN32_PostQuitMessage(fpl__win32_func_PostQuitMessage);
#define FPL__FUNC_WIN32_CreateIconIndirect(name) HICON WINAPI name(PICONINFO piconinfo)
typedef FPL__FUNC_WIN32_CreateIconIndirect(fpl__win32_func_CreateIconIndirect);
#define FPL__FUNC_WIN32_GetKeyboardLayout(name) HKL WINAPI name(DWORD idThread)
typedef FPL__FUNC_WIN32_GetKeyboardLayout(fpl__win32_func_GetKeyboardLayout);

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
	fpl__win32_func_CreateDIBSection *CreateDIBSection;
	fpl__win32_func_CreateBitmap *CreateBitmap;
} fpl__Win32GdiApi;

typedef struct fpl__Win32ShellApi {
	HMODULE shellLibrary;
	fpl__win32_func_CommandLineToArgvW *CommandLineToArgvW;
	fpl__win32_func_SHGetFolderPathA *ShGetFolderPathA;
	fpl__win32_func_SHGetFolderPathW *ShGetFolderPathW;
	fpl__win32_func_DragQueryFileA *DragQueryFileA;
	fpl__win32_func_DragQueryFileW *DragQueryFileW;
	fpl__win32_func_DragAcceptFiles *DragAcceptFiles;
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
	fpl__win32_func_IsIconic *IsIconic;
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
	fpl__win32_func_PostQuitMessage *PostQuitMessage;
	fpl__win32_func_CreateIconIndirect *CreateIconIndirect;
	fpl__win32_func_GetKeyboardLayout *GetKeyboardLayout;
	fpl__win32_func_GetKeyState *GetKeyState;
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
	fpl_b32 isValid;
} fpl__Win32Api;

fpl_internal void fpl__Win32UnloadApi(fpl__Win32Api *wapi) {
	FPL_ASSERT(wapi != fpl_null);
	if(wapi->ole.oleLibrary != fpl_null) {
		FreeLibrary(wapi->ole.oleLibrary);
		FPL_CLEAR_STRUCT(&wapi->ole);
	}
	if(wapi->gdi.gdiLibrary != fpl_null) {
		FreeLibrary(wapi->gdi.gdiLibrary);
		FPL_CLEAR_STRUCT(&wapi->gdi);
	}
	if(wapi->user.userLibrary != fpl_null) {
		FreeLibrary(wapi->user.userLibrary);
		FPL_CLEAR_STRUCT(&wapi->user);
	}
	if(wapi->shell.shellLibrary != fpl_null) {
		FreeLibrary(wapi->shell.shellLibrary);
		FPL_CLEAR_STRUCT(&wapi->shell);
	}
	wapi->isValid = false;
}

fpl_internal bool fpl__Win32LoadApi(fpl__Win32Api *wapi) {
	FPL_ASSERT(wapi != fpl_null);
	FPL_CLEAR_STRUCT(wapi);

	// Shell32
	{
		const char *shellLibraryName = "shell32.dll";
		HMODULE library = wapi->shell.shellLibrary = LoadLibraryA(shellLibraryName);
		if(library == fpl_null) {
			FPL_ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", shellLibraryName);
			return false;
		}
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.CommandLineToArgvW, fpl__win32_func_CommandLineToArgvW, "CommandLineToArgvW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.ShGetFolderPathA, fpl__win32_func_SHGetFolderPathA, "SHGetFolderPathA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.ShGetFolderPathW, fpl__win32_func_SHGetFolderPathW, "SHGetFolderPathW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.DragQueryFileA, fpl__win32_func_DragQueryFileA, "DragQueryFileA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.DragQueryFileW, fpl__win32_func_DragQueryFileW, "DragQueryFileW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, shellLibraryName, wapi->shell.DragAcceptFiles, fpl__win32_func_DragAcceptFiles, "DragAcceptFiles");
	}

	// User32
	{
		const char *userLibraryName = "user32.dll";
		HMODULE library = wapi->user.userLibrary = LoadLibraryA(userLibraryName);
		if(library == fpl_null) {
			FPL_ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", userLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.RegisterClassExA, fpl__win32_func_RegisterClassExA, "RegisterClassExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.RegisterClassExW, fpl__win32_func_RegisterClassExW, "RegisterClassExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.UnregisterClassA, fpl__win32_func_UnregisterClassA, "UnregisterClassA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.UnregisterClassW, fpl__win32_func_UnregisterClassW, "UnregisterClassW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ShowWindow, fpl__win32_func_ShowWindow, "ShowWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.DestroyWindow, fpl__win32_func_DestroyWindow, "DestroyWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.UpdateWindow, fpl__win32_func_UpdateWindow, "UpdateWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.TranslateMessage, fpl__win32_func_TranslateMessage, "TranslateMessage");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.DispatchMessageA, fpl__win32_func_DispatchMessageA, "DispatchMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.DispatchMessageW, fpl__win32_func_DispatchMessageW, "DispatchMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.PeekMessageA, fpl__win32_func_PeekMessageA, "PeekMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.PeekMessageW, fpl__win32_func_PeekMessageW, "PeekMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.DefWindowProcA, fpl__win32_func_DefWindowProcA, "DefWindowProcA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.DefWindowProcW, fpl__win32_func_DefWindowProcW, "DefWindowProcW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.CreateWindowExA, fpl__win32_func_CreateWindowExA, "CreateWindowExA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.CreateWindowExW, fpl__win32_func_CreateWindowExW, "CreateWindowExW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowPos, fpl__win32_func_SetWindowPos, "SetWindowPos");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowPlacement, fpl__win32_func_GetWindowPlacement, "GetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowPlacement, fpl__win32_func_SetWindowPlacement, "SetWindowPlacement");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetClientRect, fpl__win32_func_GetClientRect, "GetClientRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowRect, fpl__win32_func_GetWindowRect, "GetWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.AdjustWindowRect, fpl__win32_func_AdjustWindowRect, "AdjustWindowRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetAsyncKeyState, fpl__win32_func_GetAsyncKeyState, "GetAsyncKeyState");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.MapVirtualKeyA, fpl__win32_func_MapVirtualKeyA, "MapVirtualKeyA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.MapVirtualKeyW, fpl__win32_func_MapVirtualKeyW, "MapVirtualKeyW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetCursor, fpl__win32_func_SetCursor, "SetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetCursor, fpl__win32_func_GetCursor, "GetCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.LoadCursorA, fpl__win32_func_LoadCursorA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.LoadCursorW, fpl__win32_func_LoadCursorW, "LoadCursorW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetCursorPos, fpl__win32_func_GetCursorPos, "GetCursorPos");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.WindowFromPoint, fpl__win32_func_WindowFromPoint, "WindowFromPoint");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.LoadIconA, fpl__win32_func_LoadIconA, "LoadCursorA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.LoadIconW, fpl__win32_func_LoadIconW, "LoadIconW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowTextA, fpl__win32_func_SetWindowTextA, "SetWindowTextA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowTextW, fpl__win32_func_SetWindowTextW, "SetWindowTextW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowLongA, fpl__win32_func_SetWindowLongA, "SetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowLongW, fpl__win32_func_SetWindowLongW, "SetWindowLongW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowLongA, fpl__win32_func_GetWindowLongA, "GetWindowLongA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowLongW, fpl__win32_func_GetWindowLongW, "GetWindowLongW");

#		if defined(FPL_ARCH_X64)
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowLongPtrA, fpl__win32_func_SetWindowLongPtrA, "SetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetWindowLongPtrW, fpl__win32_func_SetWindowLongPtrW, "SetWindowLongPtrW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowLongPtrA, fpl__win32_func_GetWindowLongPtrA, "GetWindowLongPtrA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetWindowLongPtrW, fpl__win32_func_GetWindowLongPtrW, "GetWindowLongPtrW");
#		endif

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ReleaseDC, fpl__win32_func_ReleaseDC, "ReleaseDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetDC, fpl__win32_func_GetDC, "GetDC");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ChangeDisplaySettingsA, fpl__win32_func_ChangeDisplaySettingsA, "ChangeDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ChangeDisplaySettingsW, fpl__win32_func_ChangeDisplaySettingsW, "ChangeDisplaySettingsW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.EnumDisplaySettingsA, fpl__win32_func_EnumDisplaySettingsA, "EnumDisplaySettingsA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.EnumDisplaySettingsW, fpl__win32_func_EnumDisplaySettingsW, "EnumDisplaySettingsW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.OpenClipboard, fpl__win32_func_OpenClipboard, "OpenClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.CloseClipboard, fpl__win32_func_CloseClipboard, "CloseClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.EmptyClipboard, fpl__win32_func_EmptyClipboard, "EmptyClipboard");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SetClipboardData, fpl__win32_func_SetClipboardData, "SetClipboardData");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetClipboardData, fpl__win32_func_GetClipboardData, "GetClipboardData");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetDesktopWindow, fpl__win32_func_GetDesktopWindow, "GetDesktopWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetForegroundWindow, fpl__win32_func_GetForegroundWindow, "GetForegroundWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.IsZoomed, fpl__win32_func_IsZoomed, "IsZoomed");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.IsIconic, fpl__win32_func_IsIconic, "IsIconic");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SendMessageA, fpl__win32_func_SendMessageA, "SendMessageA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.SendMessageW, fpl__win32_func_SendMessageW, "SendMessageW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetMonitorInfoA, fpl__win32_func_GetMonitorInfoA, "GetMonitorInfoA");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetMonitorInfoW, fpl__win32_func_GetMonitorInfoW, "GetMonitorInfoW");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.MonitorFromRect, fpl__win32_func_MonitorFromRect, "MonitorFromRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.MonitorFromWindow, fpl__win32_func_MonitorFromWindow, "MonitorFromWindow");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ClientToScreen, fpl__win32_func_ClientToScreen, "ClientToScreen");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.PtInRect, fpl__win32_func_PtInRect, "PtInRect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.RegisterRawInputDevices, fpl__win32_func_RegisterRawInputDevices, "RegisterRawInputDevices");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.ClipCursor, fpl__win32_func_ClipCursor, "ClipCursor");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.PostQuitMessage, fpl__win32_func_PostQuitMessage, "PostQuitMessage");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.CreateIconIndirect, fpl__win32_func_CreateIconIndirect, "CreateIconIndirect");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetKeyboardLayout, fpl__win32_func_GetKeyboardLayout, "GetKeyboardLayout");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, userLibraryName, wapi->user.GetKeyState, fpl__win32_func_GetKeyState, "GetKeyState");
	}

	// GDI32
	{
		const char *gdiLibraryName = "gdi32.dll";
		HMODULE library = wapi->gdi.gdiLibrary = LoadLibraryA(gdiLibraryName);
		if(library == fpl_null) {
			FPL_ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", gdiLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.ChoosePixelFormat, fpl__win32_func_ChoosePixelFormat, "ChoosePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.SetPixelFormat, fpl__win32_func_SetPixelFormat, "SetPixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.DescribePixelFormat, fpl__win32_func_DescribePixelFormat, "DescribePixelFormat");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.StretchDIBits, fpl__win32_func_StretchDIBits, "StretchDIBits");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.DeleteObject, fpl__win32_func_DeleteObject, "DeleteObject");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.SwapBuffers, fpl__win32_func_SwapBuffers, "SwapBuffers");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.GetDeviceCaps, fpl__win32_func_GetDeviceCaps, "GetDeviceCaps");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.CreateDIBSection, fpl__win32_func_CreateDIBSection, "CreateDIBSection");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, gdiLibraryName, wapi->gdi.CreateBitmap, fpl__win32_func_CreateBitmap, "CreateBitmap");
	}

	// OLE32
	{
		const char *oleLibraryName = "ole32.dll";
		HMODULE library = wapi->ole.oleLibrary = LoadLibraryA(oleLibraryName);
		if(library == fpl_null) {
			FPL_ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", oleLibraryName);
			return false;
		}

		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, oleLibraryName, wapi->ole.CoInitializeEx, fpl__win32_func_CoInitializeEx, "CoInitializeEx");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, oleLibraryName, wapi->ole.CoUninitialize, fpl__win32_func_CoUninitialize, "CoUninitialize");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, oleLibraryName, wapi->ole.CoCreateInstance, fpl__win32_func_CoCreateInstance, "CoCreateInstance");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, oleLibraryName, wapi->ole.CoTaskMemFree, fpl__win32_func_CoTaskMemFree, "CoTaskMemFree");
		FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_WIN32, library, oleLibraryName, wapi->ole.PropVariantClear, fpl__win32_func_PropVariantClear, "PropVariantClear");
	}

	wapi->isValid = true;

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
	fpl_b32 isConnected[XUSER_MAX_COUNT];
	LARGE_INTEGER lastDeviceSearchTime;
	fpl__Win32XInputApi xinputApi;
} fpl__Win32XInputState;

typedef struct fpl__Win32ConsoleState {
	fpl_b32 isAllocated;
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
	fpl_b32 isMaximized;
	fpl_b32 isMinimized;
	fpl_b32 wasResolutionChanged;
} fpl__Win32LastWindowInfo;

typedef struct fpl__Win32WindowState {
#if _UNICODE
	wchar_t windowClass[256];
#else
	char windowClass[256];
#endif
	fpl__Win32LastWindowInfo lastFullscreenInfo;
	HWND windowHandle;
	HDC deviceContext;
	HCURSOR defaultCursor;
	int pixelFormat;
	fpl_b32 isCursorActive;
	fpl_b32 isFrameInteraction;
} fpl__Win32WindowState;
#endif // FPL_ENABLE_WINDOW

#endif // FPL_PLATFORM_WIN32

// ############################################################################
//
// > TYPES_POSIX
//
// ############################################################################
#if defined(FPL_SUBPLATFORM_POSIX)
#	include <sys/types.h> // data types
#	include <sys/mman.h> // mmap, munmap
#	include <sys/stat.h> // mkdir
#	include <sys/errno.h> // errno
#	include <sys/time.h> // gettimeofday
#	include <signal.h> // pthread_kill
#	include <time.h> // clock_gettime, nanosleep
#	include <dlfcn.h> // dlopen, dlclose
#	include <fcntl.h> // open
#	include <unistd.h> // read, write, close, access, rmdir, getpid

// Little macro to not write 5 lines of code all the time
#define FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(mod, libHandle, libName, target, type, name) \
	target = (type *)dlsym(libHandle, name); \
	if (target == fpl_null) { \
		FPL_ERROR(mod, "Failed getting '%s' from library '%s'", name, libName); \
		break; \
	}

#define FPL__FUNC_PTHREAD_pthread_create(name) int name(pthread_t *, const pthread_attr_t *, void *(*__start_routine) (void *), void *)
typedef FPL__FUNC_PTHREAD_pthread_create(fpl__pthread_func_pthread_create);
#define FPL__FUNC_PTHREAD_pthread_kill(name) int name(pthread_t thread, int sig)
typedef FPL__FUNC_PTHREAD_pthread_kill(fpl__pthread_func_pthread_kill);
#define FPL__FUNC_PTHREAD_pthread_join(name) int name(pthread_t __th, void **retval)
typedef FPL__FUNC_PTHREAD_pthread_join(fpl__pthread_func_pthread_join);
#define FPL__FUNC_PTHREAD_pthread_exit(name) void name(void *__retval)
typedef FPL__FUNC_PTHREAD_pthread_exit(fpl__pthread_func_pthread_exit);
#define FPL__FUNC_PTHREAD_pthread_yield(name) int name(void)
typedef FPL__FUNC_PTHREAD_pthread_yield(fpl__pthread_func_pthread_yield);
#define FPL__FUNC_PTHREAD_pthread_timedjoin_np(name) int name(pthread_t thread, void **retval, const struct timespec *abstime)
typedef FPL__FUNC_PTHREAD_pthread_timedjoin_np(fpl__pthread_func_pthread_timedjoin_np);

#define FPL__FUNC_PTHREAD_pthread_mutex_init(name) int name(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
typedef FPL__FUNC_PTHREAD_pthread_mutex_init(fpl__pthread_func_pthread_mutex_init);
#define FPL__FUNC_PTHREAD_pthread_mutex_destroy(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_pthread_mutex_destroy(fpl__pthread_func_pthread_mutex_destroy);
#define FPL__FUNC_PTHREAD_pthread_mutex_lock(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_pthread_mutex_lock(fpl__pthread_func_pthread_mutex_lock);
#define FPL__FUNC_PTHREAD_pthread_mutex_trylock(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_pthread_mutex_trylock(fpl__pthread_func_pthread_mutex_trylock);
#define FPL__FUNC_PTHREAD_pthread_mutex_unlock(name) int name(pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_pthread_mutex_unlock(fpl__pthread_func_pthread_mutex_unlock);

#define FPL__FUNC_PTHREAD_pthread_cond_init(name) int name(pthread_cond_t *cond, const pthread_condattr_t *attr)
typedef FPL__FUNC_PTHREAD_pthread_cond_init(fpl__pthread_func_pthread_cond_init);
#define FPL__FUNC_PTHREAD_pthread_cond_destroy(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_pthread_cond_destroy(fpl__pthread_func_pthread_cond_destroy);
#define FPL__FUNC_PTHREAD_pthread_cond_timedwait(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
typedef FPL__FUNC_PTHREAD_pthread_cond_timedwait(fpl__pthread_func_pthread_cond_timedwait);
#define FPL__FUNC_PTHREAD_pthread_cond_wait(name) int name(pthread_cond_t *cond, pthread_mutex_t *mutex)
typedef FPL__FUNC_PTHREAD_pthread_cond_wait(fpl__pthread_func_pthread_cond_wait);
#define FPL__FUNC_PTHREAD_pthread_cond_broadcast(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_pthread_cond_broadcast(fpl__pthread_func_pthread_cond_broadcast);
#define FPL__FUNC_PTHREAD_pthread_cond_signal(name) int name(pthread_cond_t *cond)
typedef FPL__FUNC_PTHREAD_pthread_cond_signal(fpl__pthread_func_pthread_cond_signal);

#define FPL__FUNC_PTHREAD_sem_init(name) int name(sem_t *__sem, int __pshared, unsigned int __value)
typedef FPL__FUNC_PTHREAD_sem_init(fpl__pthread_func_sem_init);
#define FPL__FUNC_PTHREAD_sem_destroy(name) int name(sem_t *__sem)
typedef FPL__FUNC_PTHREAD_sem_destroy(fpl__pthread_func_sem_destroy);
#define FPL__FUNC_PTHREAD_sem_wait(name) int name(sem_t *__sem)
typedef FPL__FUNC_PTHREAD_sem_wait(fpl__pthread_func_sem_wait);
#define FPL__FUNC_PTHREAD_sem_timedwait(name) int name(sem_t *__restrict __sem, const struct timespec *__restrict __abstime)
typedef FPL__FUNC_PTHREAD_sem_timedwait(fpl__pthread_func_sem_timedwait);
#define FPL__FUNC_PTHREAD_sem_trywait(name) int name(sem_t *__sem)
typedef FPL__FUNC_PTHREAD_sem_trywait(fpl__pthread_func_sem_trywait);
#define FPL__FUNC_PTHREAD_sem_post(name) int name(sem_t *__sem)
typedef FPL__FUNC_PTHREAD_sem_post(fpl__pthread_func_sem_post);
#define FPL__FUNC_PTHREAD_sem_getvalue(name) int name(sem_t *__restrict __sem, int *__restrict __sval)
typedef FPL__FUNC_PTHREAD_sem_getvalue(fpl__pthread_func_sem_getvalue);

typedef struct fpl__PThreadApi {
	void *libHandle;
	fpl__pthread_func_pthread_create *pthread_create;
	fpl__pthread_func_pthread_kill *pthread_kill;
	fpl__pthread_func_pthread_join *pthread_join;
	fpl__pthread_func_pthread_exit *pthread_exit;
	fpl__pthread_func_pthread_yield *pthread_yield;
	fpl__pthread_func_pthread_timedjoin_np *pthread_timedjoin_np;

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

	fpl__pthread_func_sem_init *sem_init;
	fpl__pthread_func_sem_destroy *sem_destroy;
	fpl__pthread_func_sem_wait *sem_wait;
	fpl__pthread_func_sem_timedwait *sem_timedwait;
	fpl__pthread_func_sem_trywait *sem_trywait;
	fpl__pthread_func_sem_post *sem_post;
	fpl__pthread_func_sem_getvalue *sem_getvalue;
} fpl__PThreadApi;

#define FPL__POSIX_DL_LOADTYPE RTLD_NOW

fpl_internal void fpl__PThreadUnloadApi(fpl__PThreadApi *pthreadApi) {
	FPL_ASSERT(pthreadApi != fpl_null);
	if(pthreadApi->libHandle != fpl_null) {
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
	for(uint32_t index = 0; index < FPL_ARRAYCOUNT(libpthreadFileNames); ++index) {
		const char * libName = libpthreadFileNames[index];
		void *libHandle = pthreadApi->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if(libHandle != fpl_null) {
			FPL_CLEAR_STRUCT(pthreadApi);
			do {
				// pthread_t
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_create, fpl__pthread_func_pthread_create, "pthread_create");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_kill, fpl__pthread_func_pthread_kill, "pthread_kill");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_join, fpl__pthread_func_pthread_join, "pthread_join");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_exit, fpl__pthread_func_pthread_exit, "pthread_exit");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_yield, fpl__pthread_func_pthread_yield, "pthread_yield");
				pthreadApi->pthread_timedjoin_np = (fpl__pthread_func_pthread_timedjoin_np *)dlsym(libHandle, "pthread_timedjoin_np");

				// pthread_mutex_t
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_mutex_init, fpl__pthread_func_pthread_mutex_init, "pthread_mutex_init");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_mutex_destroy, fpl__pthread_func_pthread_mutex_destroy, "pthread_mutex_destroy");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_mutex_lock, fpl__pthread_func_pthread_mutex_lock, "pthread_mutex_lock");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_mutex_trylock, fpl__pthread_func_pthread_mutex_trylock, "pthread_mutex_trylock");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_mutex_unlock, fpl__pthread_func_pthread_mutex_unlock, "pthread_mutex_unlock");

				// pthread_cond_t
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_init, fpl__pthread_func_pthread_cond_init, "pthread_cond_init");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_destroy, fpl__pthread_func_pthread_cond_destroy, "pthread_cond_destroy");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_timedwait, fpl__pthread_func_pthread_cond_timedwait, "pthread_cond_timedwait");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_wait, fpl__pthread_func_pthread_cond_wait, "pthread_cond_wait");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_broadcast, fpl__pthread_func_pthread_cond_broadcast, "pthread_cond_broadcast");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->pthread_cond_signal, fpl__pthread_func_pthread_cond_signal, "pthread_cond_signal");

				// sem_t
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_init, fpl__pthread_func_sem_init, "sem_init");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_destroy, fpl__pthread_func_sem_destroy, "sem_destroy");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_wait, fpl__pthread_func_sem_wait, "sem_wait");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_timedwait, fpl__pthread_func_sem_timedwait, "sem_timedwait");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_trywait, fpl__pthread_func_sem_trywait, "sem_trywait");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_post, fpl__pthread_func_sem_post, "sem_post");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi->sem_getvalue, fpl__pthread_func_sem_getvalue, "sem_getvalue");

				result = true;
			} while(0);
			if(result) {
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

#if defined(FPL_ENABLE_WINDOW)
#define FPL__LINUX_MAX_GAME_CONTROLLER_COUNT 4
typedef struct fpl__LinuxGameController {
	char deviceName[512];
	char displayName[1024];
	int fd;
	uint8_t axisCount;
	uint8_t buttonCount;
	fplGamepadState state;
} fpl__LinuxGameController;

typedef struct fpl__LinuxGameControllersState {
	fpl__LinuxGameController controllers[FPL__LINUX_MAX_GAME_CONTROLLER_COUNT];
	uint64_t lastCheckTime;
} fpl__LinuxGameControllersState;
#endif

typedef struct fpl__LinuxAppState {
#if defined(FPL_ENABLE_WINDOW)
	fpl__LinuxGameControllersState controllersState;
#endif
	int dummy;
} fpl__LinuxAppState;

// Forward declarations
#if defined(FPL_ENABLE_WINDOW)
fpl_internal void fpl__LinuxFreeGameControllers(fpl__LinuxGameControllersState *controllersState);
fpl_internal void fpl__LinuxPollGameControllers(const fplSettings *settings, fpl__LinuxGameControllersState *controllersState);
#endif

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
#define FPL__FUNC_X11_XFree(name) int name(void *data)
typedef FPL__FUNC_X11_XFree(fpl__func_x11_XFree);
#define FPL__FUNC_X11_XFlush(name) int name(Display *display)
typedef FPL__FUNC_X11_XFlush(fpl__func_x11_XFlush);
#define FPL__FUNC_X11_XOpenDisplay(name) Display *name(char *display_name)
typedef FPL__FUNC_X11_XOpenDisplay(fpl__func_x11_XOpenDisplay);
#define FPL__FUNC_X11_XCloseDisplay(name) int name(Display *display)
typedef FPL__FUNC_X11_XCloseDisplay(fpl__func_x11_XCloseDisplay);
#define FPL__FUNC_X11_XDefaultScreen(name) int name(Display *display)
typedef FPL__FUNC_X11_XDefaultScreen(fpl__func_x11_XDefaultScreen);
#define FPL__FUNC_X11_XRootWindow(name) Window name(Display *display, int screen_number)
typedef FPL__FUNC_X11_XRootWindow(fpl__func_x11_XRootWindow);
#define FPL__FUNC_X11_XCreateWindow(name) Window name(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int clazz, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
typedef FPL__FUNC_X11_XCreateWindow(fpl__func_x11_XCreateWindow);
#define FPL__FUNC_X11_XDestroyWindow(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_XDestroyWindow(fpl__func_x11_XDestroyWindow);
#define FPL__FUNC_X11_XCreateColormap(name) Colormap name(Display *display, Window w, Visual *visual, int alloc)
typedef FPL__FUNC_X11_XCreateColormap(fpl__func_x11_XCreateColormap);
#define FPL__FUNC_X11_XDefaultColormap(name) Colormap name(Display *display, int screen_number)
typedef FPL__FUNC_X11_XDefaultColormap(fpl__func_x11_XDefaultColormap);
#define FPL__FUNC_X11_XFreeColormap(name) int name(Display *display, Colormap colormap)
typedef FPL__FUNC_X11_XFreeColormap(fpl__func_x11_XFreeColormap);
#define FPL__FUNC_X11_XMapWindow(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_XMapWindow(fpl__func_x11_XMapWindow);
#define FPL__FUNC_X11_XUnmapWindow(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_XUnmapWindow(fpl__func_x11_XUnmapWindow);
#define FPL__FUNC_X11_XStoreName(name) int name(Display *display, Window w, _Xconst char *window_name)
typedef FPL__FUNC_X11_XStoreName(fpl__func_x11_XStoreName);
#define FPL__FUNC_X11_XDefaultVisual(name) Visual *name(Display *display, int screen_number)
typedef FPL__FUNC_X11_XDefaultVisual(fpl__func_x11_XDefaultVisual);
#define FPL__FUNC_X11_XDefaultDepth(name) int name(Display *display, int screen_number)
typedef FPL__FUNC_X11_XDefaultDepth(fpl__func_x11_XDefaultDepth);
#define FPL__FUNC_X11_XInternAtom(name) Atom name(Display *display, char *atom_name, Bool only_if_exists)
typedef FPL__FUNC_X11_XInternAtom(fpl__func_x11_XInternAtom);
#define FPL__FUNC_X11_XSetWMProtocols(name) Status name(Display *display, Window w, Atom *protocols, int count)
typedef FPL__FUNC_X11_XSetWMProtocols(fpl__func_x11_XSetWMProtocols);
#define FPL__FUNC_X11_XPending(name) int name(Display *display)
typedef FPL__FUNC_X11_XPending(fpl__func_x11_XPending);
#define FPL__FUNC_X11_XSync(name) int name(Display *display, Bool discard)
typedef FPL__FUNC_X11_XSync(fpl__func_x11_XSync);
#define FPL__FUNC_X11_XNextEvent(name) int name(Display *display, XEvent *event_return)
typedef FPL__FUNC_X11_XNextEvent(fpl__func_x11_XNextEvent);
#define FPL__FUNC_X11_XPeekEvent(name) int name(Display *display, XEvent *event_return)
typedef FPL__FUNC_X11_XPeekEvent(fpl__func_x11_XPeekEvent);
#define FPL__FUNC_X11_XEventsQueued(name) int name(Display *display, int mode)
typedef FPL__FUNC_X11_XEventsQueued(fpl__func_x11_XEventsQueued);
#define FPL__FUNC_X11_XGetWindowAttributes(name) Status name(Display *display, Window w, XWindowAttributes *window_attributes_return)
typedef FPL__FUNC_X11_XGetWindowAttributes(fpl__func_x11_XGetWindowAttributes);
#define FPL__FUNC_X11_XResizeWindow(name) int name(Display *display, Window w, unsigned int width, unsigned int height)
typedef FPL__FUNC_X11_XResizeWindow(fpl__func_x11_XResizeWindow);
#define FPL__FUNC_X11_XMoveWindow(name) int name(Display *display, Window w, int x, int y)
typedef FPL__FUNC_X11_XMoveWindow(fpl__func_x11_XMoveWindow);
#define FPL__FUNC_X11_XGetKeyboardMapping(name) KeySym *name(Display *display, KeyCode first_keycode, int keycode_count, int *keysyms_per_keycode_return)
typedef FPL__FUNC_X11_XGetKeyboardMapping(fpl__func_x11_XGetKeyboardMapping);
#define FPL__FUNC_X11_XSendEvent(name) Status name(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
typedef FPL__FUNC_X11_XSendEvent(fpl__func_x11_XSendEvent);
#define FPL__FUNC_X11_XMatchVisualInfo(name) Status name(Display* display, int screen, int depth, int clazz, XVisualInfo* vinfo_return)
typedef FPL__FUNC_X11_XMatchVisualInfo(fpl__func_x11_XMatchVisualInfo);
#define FPL__FUNC_X11_XCreateGC(name) GC name(Display* display, Drawable d, unsigned long valuemask, XGCValues* values)
typedef FPL__FUNC_X11_XCreateGC(fpl__func_x11_XCreateGC);
#define FPL__FUNC_X11_XGetImage(name) XImage *name(Display* display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format)
typedef FPL__FUNC_X11_XGetImage(fpl__func_x11_XGetImage);
#define FPL__FUNC_X11_XCreateImage(name) XImage *name(Display *display, Visual *visual, unsigned int depth, int format, int offset, char *data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line)
typedef FPL__FUNC_X11_XCreateImage(fpl__func_x11_XCreateImage);
#define FPL__FUNC_X11_XPutImage(name) int name(Display *display, Drawable d, GC gc, XImage *image, int src_x, int src_y, int dest_x, int	dest_y, unsigned int width, unsigned int height)
typedef FPL__FUNC_X11_XPutImage(fpl__func_x11_XPutImage);
#define FPL__FUNC_X11_XMapRaised(name) int name(Display *display, Window w)
typedef FPL__FUNC_X11_XMapRaised(fpl__func_x11_XMapRaised);
#define FPL__FUNC_X11_XCreatePixmap(name) Pixmap name(Display * display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
typedef FPL__FUNC_X11_XCreatePixmap(fpl__func_x11_XCreatePixmap);
#define FPL__FUNC_X11_XSelectInput(name) int name(Display * display, Window w, long eventMask)
typedef FPL__FUNC_X11_XSelectInput(fpl__func_x11_XSelectInput);
#define FPL__FUNC_X11_XGetWindowProperty(name) int name(Display* display, Window w, Atom prop, long long_offset, long long_length, Bool del, Atom req_type, Atom* actual_type_return, int* actual_format_return, unsigned long* nitems_return, unsigned long*	bytes_after_return, unsigned char**	prop_return)
typedef FPL__FUNC_X11_XGetWindowProperty(fpl__func_x11_XGetWindowProperty);
#define FPL__FUNC_X11_XChangeProperty(name) int name(Display* display, Window w, Atom property, Atom type, int format, int mode, _Xconst unsigned char* data, int nelements)
typedef FPL__FUNC_X11_XChangeProperty(fpl__func_x11_XChangeProperty);
#define FPL__FUNC_X11_XDeleteProperty(name) int name(Display* display, Window w,Atom prop)
typedef FPL__FUNC_X11_XDeleteProperty(fpl__func_x11_XDeleteProperty);
#define FPL__FUNC_X11_XStringListToTextProperty(name) Status name(char** list, int count, XTextProperty* text_prop_return)
typedef FPL__FUNC_X11_XStringListToTextProperty(fpl__func_x11_XStringListToTextProperty);
#define FPL__FUNC_X11_XSetWMIconName(name) void name(Display* display, Window w, XTextProperty *text_prop)
typedef FPL__FUNC_X11_XSetWMIconName(fpl__func_x11_XSetWMIconName);
#define FPL__FUNC_X11_XSetWMName(name) void name(Display* display, Window w, XTextProperty *text_prop)
typedef FPL__FUNC_X11_XSetWMName(fpl__func_x11_XSetWMName);
#define FPL__FUNC_X11_XQueryKeymap(name) int name(Display* display, char [32])
typedef FPL__FUNC_X11_XQueryKeymap(fpl__func_x11_XQueryKeymap);




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
	fpl__func_x11_XEventsQueued *XEventsQueued;
	fpl__func_x11_XGetWindowAttributes *XGetWindowAttributes;
	fpl__func_x11_XResizeWindow *XResizeWindow;
	fpl__func_x11_XMoveWindow *XMoveWindow;
	fpl__func_x11_XGetKeyboardMapping *XGetKeyboardMapping;
	fpl__func_x11_XSendEvent *XSendEvent;
	fpl__func_x11_XMatchVisualInfo *XMatchVisualInfo;
	fpl__func_x11_XCreateGC *XCreateGC;
	fpl__func_x11_XGetImage *XGetImage;
	fpl__func_x11_XPutImage *XPutImage;
	fpl__func_x11_XMapRaised *XMapRaised;
	fpl__func_x11_XCreateImage *XCreateImage;
	fpl__func_x11_XCreatePixmap *XCreatePixmap;
	fpl__func_x11_XSelectInput *XSelectInput;
	fpl__func_x11_XGetWindowProperty *XGetWindowProperty;
	fpl__func_x11_XChangeProperty *XChangeProperty;
	fpl__func_x11_XDeleteProperty *XDeleteProperty;
	fpl__func_x11_XStringListToTextProperty *XStringListToTextProperty;
	fpl__func_x11_XSetWMIconName *XSetWMIconName;
	fpl__func_x11_XSetWMName *XSetWMName;
	fpl__func_x11_XQueryKeymap *XQueryKeymap;
} fpl__X11Api;

fpl_internal void fpl__UnloadX11Api(fpl__X11Api *x11Api) {
	FPL_ASSERT(x11Api != fpl_null);
	if(x11Api->libHandle != fpl_null) {
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
	for(uint32_t index = 0; index < FPL_ARRAYCOUNT(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		void *libHandle = x11Api->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if(libHandle != fpl_null) {
			do {
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XFlush, fpl__func_x11_XFlush, "XFlush");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XFree, fpl__func_x11_XFree, "XFree");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XOpenDisplay, fpl__func_x11_XOpenDisplay, "XOpenDisplay");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCloseDisplay, fpl__func_x11_XCloseDisplay, "XCloseDisplay");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDefaultScreen, fpl__func_x11_XDefaultScreen, "XDefaultScreen");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XRootWindow, fpl__func_x11_XRootWindow, "XRootWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCreateWindow, fpl__func_x11_XCreateWindow, "XCreateWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDestroyWindow, fpl__func_x11_XDestroyWindow, "XDestroyWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCreateColormap, fpl__func_x11_XCreateColormap, "XCreateColormap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XFreeColormap, fpl__func_x11_XFreeColormap, "XFreeColormap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDefaultColormap, fpl__func_x11_XDefaultColormap, "XDefaultColormap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XMapWindow, fpl__func_x11_XMapWindow, "XMapWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XUnmapWindow, fpl__func_x11_XUnmapWindow, "XUnmapWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XStoreName, fpl__func_x11_XStoreName, "XStoreName");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDefaultVisual, fpl__func_x11_XDefaultVisual, "XDefaultVisual");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDefaultDepth, fpl__func_x11_XDefaultDepth, "XDefaultDepth");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XInternAtom, fpl__func_x11_XInternAtom, "XInternAtom");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSetWMProtocols, fpl__func_x11_XSetWMProtocols, "XSetWMProtocols");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XPending, fpl__func_x11_XPending, "XPending");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSync, fpl__func_x11_XSync, "XSync");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XNextEvent, fpl__func_x11_XNextEvent, "XNextEvent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XPeekEvent, fpl__func_x11_XPeekEvent, "XPeekEvent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XEventsQueued, fpl__func_x11_XEventsQueued, "XEventsQueued");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XGetWindowAttributes, fpl__func_x11_XGetWindowAttributes, "XGetWindowAttributes");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XResizeWindow, fpl__func_x11_XResizeWindow, "XResizeWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XMoveWindow, fpl__func_x11_XMoveWindow, "XMoveWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XGetKeyboardMapping, fpl__func_x11_XGetKeyboardMapping, "XGetKeyboardMapping");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSendEvent, fpl__func_x11_XSendEvent, "XSendEvent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XMatchVisualInfo, fpl__func_x11_XMatchVisualInfo, "XMatchVisualInfo");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCreateGC, fpl__func_x11_XCreateGC, "XCreateGC");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XGetImage, fpl__func_x11_XGetImage, "XGetImage");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XPutImage, fpl__func_x11_XPutImage, "XPutImage");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XMapRaised, fpl__func_x11_XMapRaised, "XMapRaised");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCreateImage, fpl__func_x11_XCreateImage, "XCreateImage");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XCreatePixmap, fpl__func_x11_XCreatePixmap, "XCreatePixmap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSelectInput, fpl__func_x11_XSelectInput, "XSelectInput");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XGetWindowProperty, fpl__func_x11_XGetWindowProperty, "XGetWindowProperty");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XChangeProperty, fpl__func_x11_XChangeProperty, "XChangeProperty");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XDeleteProperty, fpl__func_x11_XDeleteProperty, "XDeleteProperty");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XStringListToTextProperty, fpl__func_x11_XStringListToTextProperty, "XStringListToTextProperty");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSetWMIconName, fpl__func_x11_XSetWMIconName, "XSetWMIconName");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XSetWMName, fpl__func_x11_XSetWMName, "XSetWMName");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_X11, libHandle, libName, x11Api->XQueryKeymap, fpl__func_x11_XQueryKeymap, "XQueryKeymap");

				result = true;
			} while(0);
			if(result) {
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
	Visual *visual;
	Atom wmProtocols;
	Atom wmDeleteWindow;
	Atom netWMStateFullscreen;
	Atom netWMPing;
	Atom netWMState;
	Atom netWMPid;
	Atom netWMIcon;
	Atom netWMName;
	Atom netWMIconName;
	Atom utf8String;
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
	fpl_b32 isInitialized;

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
	fplButtonState keyStates[256];
	fplButtonState mouseStates[5];
	fpl_b32 isRunning;

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
fpl_internal fplKey fpl__GetMappedKey(const fpl__PlatformWindowState *windowState, const uint64_t keyCode) {
	fplKey result;
	if(keyCode < FPL_ARRAYCOUNT(windowState->keyMap))
		result = windowState->keyMap[keyCode];
	else
		result = fplKey_None;
	return(result);
}

fpl_internal void fpl__PushEvent(const fplEvent *event) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	if(eventQueue->pushCount < FPL__MAX_EVENT_COUNT) {
		uint32_t eventIndex = fplAtomicAddU32(&eventQueue->pushCount, 1);
		FPL_ASSERT(eventIndex < FPL__MAX_EVENT_COUNT);
		eventQueue->events[eventIndex] = *event;
	}
}

fpl_internal void fpl__PushWindowEvent(const fplWindowEventType windowType, const uint32_t w, uint32_t h) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = windowType;
	newEvent.window.width = w;
	newEvent.window.height = h;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushWindowDropSingleFileEvent(const char *filePath) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = fplWindowEventType_DropSingleFile;
	newEvent.window.dropFiles.fileCount = 1;
	fplCopyAnsiString(filePath, newEvent.window.dropFiles.single.filePath, FPL_ARRAYCOUNT(newEvent.window.dropFiles.single.filePath));
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushKeyboardButtonEvent(const uint64_t keyCode, const fplKey mappedKey, const fplKeyboardModifierFlags modifiers, const fplButtonState buttonState) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.type = fplKeyboardEventType_Button;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.modifiers = modifiers;
	newEvent.keyboard.buttonState = buttonState;
	newEvent.keyboard.mappedKey = mappedKey;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushKeyboardInputEvent(const uint64_t keyCode, const fplKey mappedKey) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.type = fplKeyboardEventType_Input;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = mappedKey;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushMouseButtonEvent(const int32_t x, const int32_t y, const fplMouseButtonType mouseButton, const fplButtonState buttonState) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Button;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	newEvent.mouse.mouseButton = mouseButton;
	newEvent.mouse.buttonState = buttonState;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushMouseWheelEvent(const int32_t x, const int32_t y, const float wheelDelta) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Wheel;
	newEvent.mouse.mouseButton = fplMouseButtonType_None;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	newEvent.mouse.wheelDelta = wheelDelta;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__PushMouseMoveEvent(const int32_t x, const int32_t y) {
	fplEvent newEvent = FPL_ZERO_INIT;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Move;
	newEvent.mouse.mouseButton = fplMouseButtonType_None;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	fpl__PushEvent(&newEvent);
}

fpl_internal void fpl__HandleKeyboardButtonEvent(fpl__PlatformWindowState *windowState, const uint64_t keyCode, const fplKeyboardModifierFlags modifiers, const fplButtonState buttonState, const bool force) {
	fplKey mappedKey = fpl__GetMappedKey(windowState, keyCode);
	bool repeat = false;
	if(force) {
		windowState->keyStates[keyCode] = buttonState;
	} else {
		FPL_ASSERT(buttonState != fplButtonState_Repeat);
		if(keyCode < FPL_ARRAYCOUNT(windowState->keyStates)) {
			if((buttonState == fplButtonState_Release) && (windowState->keyStates[keyCode] == fplButtonState_Release)) {
				return;
			}
			if((buttonState == fplButtonState_Press) && (windowState->keyStates[keyCode] >= fplButtonState_Press)) {
				repeat = true;
			}
			windowState->keyStates[keyCode] = buttonState;
		}
	}
	fpl__PushKeyboardButtonEvent(keyCode, mappedKey, modifiers, repeat ? fplButtonState_Repeat : buttonState);
}

fpl_internal void fpl__HandleKeyboardInputEvent(fpl__PlatformWindowState *windowState, const uint64_t keyCode) {
	fplKey mappedKey = fpl__GetMappedKey(windowState, keyCode);
	fpl__PushKeyboardInputEvent(keyCode, mappedKey);
}

fpl_internal void fpl__HandleMouseButtonEvent(fpl__PlatformWindowState *windowState, const int32_t x, const int32_t y, const fplMouseButtonType mouseButton, const fplButtonState buttonState) {
	if(mouseButton < FPL_ARRAYCOUNT(windowState->mouseStates)) {
		windowState->mouseStates[(int)mouseButton] = buttonState;
	}
	fpl__PushMouseButtonEvent(x, y, mouseButton, buttonState);
}

fpl_internal void fpl__HandleMouseMoveEvent(fpl__PlatformWindowState *windowState, const int32_t x, const int32_t y) {
	fpl__PushMouseMoveEvent(x, y);
}

fpl_internal void fpl__HandleMouseWheelEvent(fpl__PlatformWindowState *windowState, const int32_t x, const int32_t y, const float wheelDelta) {
	fpl__PushMouseWheelEvent(x, y, wheelDelta);
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
		size_t setBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T setValue = 0; \
			for (int bytesIncrement = 0; bytesIncrement < sizeof(T); ++bytesIncrement) { \
				int bitShift = bytesIncrement * 8; \
				setValue |= ((T)value << bitShift); \
			} \
			T *dataBlock = (T *)(memory); \
			T *dataBlockEnd = (T *)(memory) + (size >> shift); \
			while (dataBlock != dataBlockEnd) { \
				*dataBlock++ = setValue; \
				setBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + setBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 != data8End) { \
			*data8++ = value; \
		} \
	} while (0);

#define FPL__MEMORY_CLEAR(T, memory, size, shift, mask) \
	do { \
		size_t clearBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = (T *)(memory); \
			T *dataBlockEnd = (T *)(memory) + (size >> shift); \
			while (dataBlock != dataBlockEnd) { \
				*dataBlock++ = 0; \
				clearBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + clearBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 != data8End) { \
			*data8++ = 0; \
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
#define FPL__MAX_LAST_ERROR_STRING_LENGTH 256
#define FPL__MAX_ERRORSTATE_COUNT 256

typedef struct fpl__ErrorState {
	char errors[FPL__MAX_ERRORSTATE_COUNT][FPL__MAX_LAST_ERROR_STRING_LENGTH];
	uint32_t count;
} fpl__ErrorState;

fpl_globalvar fpl__ErrorState fpl__global__LastErrorState = FPL_ZERO_INIT;

fpl_internal void fpl__PushError_Formatted(const fplLogLevel level, const char *format, va_list argList) {
	fpl__ErrorState *state = &fpl__global__LastErrorState;
	FPL_ASSERT(format != fpl_null);
	char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH] = FPL_ZERO_INIT;
	fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	size_t messageLen = fplGetAnsiStringLength(buffer);
	FPL_ASSERT(state->count < FPL__MAX_ERRORSTATE_COUNT);
	size_t errorIndex = state->count;
	state->count = (state->count + 1) % FPL__MAX_ERRORSTATE_COUNT;
	fplCopyAnsiStringLen(buffer, messageLen, state->errors[errorIndex], FPL__MAX_LAST_ERROR_STRING_LENGTH);
#if defined(FPL_ENABLE_LOGGING)
	fpl__LogWriteArgs(level, format, argList);
#endif
}

fpl_internal void fpl__PushError(const fplLogLevel level, const char *format, ...) {
	va_list valist;
	va_start(valist, format);
	fpl__PushError_Formatted(level, format, valist);
	va_end(valist);
}

//
// Argument Errors
//

fpl_internal void fpl__ArgumentInvalidError(const char *paramName) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter are not valid", paramName);
}
fpl_internal void fpl__ArgumentNullError(const char *paramName) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter are not allowed to be null", paramName);
}
fpl_internal void fpl__ArgumentZeroError(const char *paramName) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter must be greater than zero", paramName);
}
fpl_internal void fpl__ArgumentMinError(const char *paramName, const size_t value, const size_t minValue) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter '%zu' must be greater or equal than '%zu'", paramName, value, minValue);
}
fpl_internal void fpl__ArgumentMaxError(const char *paramName, const size_t value, const size_t maxValue) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter '%zu' must be less or equal than '%zu'", paramName, value, maxValue);
}
fpl_internal void fpl__ArgumentRangeError(const char *paramName, const size_t value, const size_t minValue, const size_t maxValue) {
	FPL_ERROR(FPL__MODULE_ARGS, "%s parameter '%zu' must be in range of '%zu' to '%zu'", paramName, value, minValue, maxValue);
}

#define FPL__CheckArgumentInvalid(arg, cond, ret) \
	if((cond)) { \
		fpl__ArgumentInvalidError(#arg); \
		return (ret); \
	}
#define FPL__CheckArgumentInvalidNoRet(arg, cond) \
	if((cond)) { \
		fpl__ArgumentInvalidError(#arg); \
		return; \
	}
#define FPL__CheckArgumentNull(arg, ret) \
	if((arg) == fpl_null) { \
		fpl__ArgumentNullError(#arg); \
		return (ret); \
	}
#define FPL__CheckArgumentNullNoRet(arg) \
	if((arg) == fpl_null) { \
		fpl__ArgumentNullError(#arg); \
		return; \
	}
#define FPL__CheckArgumentZero(arg, ret) \
	if((arg) == 0) { \
		fpl__ArgumentZeroError(#arg); \
		return (ret); \
	}
#define FPL__CheckArgumentZeroNoRet(arg) \
	if((arg) == 0) { \
		fpl__ArgumentZeroError(#arg); \
		return; \
	}
#define FPL__CheckArgumentMin(arg, minValue, ret) \
	if((arg) < (minValue)) { \
		fpl__ArgumentMinError(#arg, arg, minValue); \
		return (ret); \
	}
#define FPL__CheckArgumentMax(arg, maxValue, ret) \
	if((arg) > (maxValue)) { \
		fpl__ArgumentMaxError(#arg, arg, maxValue); \
		return (ret); \
	}
#define FPL__CheckPlatform(ret) \
	if(fpl__global__AppState == fpl_null) { \
		FPL_ERROR(FPL__MODULE_CORE, "[%s] Platform is not initialized", FPL_FUNCTION_NAME); \
		return (ret); \
	}
#define FPL__CheckPlatformNoRet() \
	if(fpl__global__AppState == fpl_null) { \
		FPL_ERROR(FPL__MODULE_CORE, "[%s] Platform is not initialized", FPL_FUNCTION_NAME); \
		return; \
	}

#define FPL__CheckApi(cond, name, ret) \
	if(!(cond)) { \
		FPL_ERROR(FPL__MODULE_API, "The API '%s' is not loaded", (name)); \
		return (ret); \
	}
#define FPL__CheckApiNoRet(cond, name) \
	if(!(cond)) { \
		FPL_ERROR(FPL__MODULE_API, "The API '%s' is not loaded", (name)); \
		return; \
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

fpl_internal fplThreadHandle *fpl__GetFreeThread() {
	fplThreadHandle *result = fpl_null;
	for(uint32_t index = 0; index < FPL__MAX_THREAD_COUNT; ++index) {
		fplThreadHandle *thread = fpl__global__ThreadState.threads + index;
		fplThreadState state = fplGetThreadState(thread);
		if(state == fplThreadState_Stopped) {
			result = thread;
			break;
		}
	}
	return(result);
}

//
// Common Strings
//
#if !defined(FPL__COMMON_STRINGS_DEFINED)
#define FPL__COMMON_STRINGS_DEFINED

fpl_common_api bool fplIsStringMatchWildcard(const char *source, const char *wildcard) {
	// Supported patterns: 
	// * = Match zero or more characters
	// ? = Match one character
	if(source == fpl_null || wildcard == fpl_null) {
		return false;
	}
	const char *s = source;
	const char *w = wildcard;
	while(*w) {
		if(*w == '?') {
			if(!*s) {
				return false;
			}
			++s;
		} else if(*w == '*') {
			while(*s) {
				char nw = w[1];
				if(nw != 0) {
					if((*s == nw) || (nw == '?') || (nw == '*')) {
						break;
					}
				}
				++s;
			}
		} else {
			if(*s != *w) {
				return false;
			}
			++s;
		}
		++w;
	}
	return true;
}

fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen) {
	if((a == fpl_null) || (b == fpl_null)) {
		return false;
	}
	if(aLen != bLen) {
		return false;
	}
	bool result = true;
	for(size_t index = 0; index < aLen; ++index) {
		char aChar = a[index];
		char bChar = b[index];
		if(aChar != bChar) {
			result = false;
			break;
		}
	}
	return(result);
}

fpl_common_api bool fplIsStringEqual(const char *a, const char *b) {
	if((a == fpl_null) || (b == fpl_null)) {
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

fpl_common_api char *fplEnforcePathSeparatorLen(char *path, size_t maxPathLen) {
	FPL__CheckArgumentNull(path, fpl_null);
	FPL__CheckArgumentZero(maxPathLen, fpl_null);
	char *end = path;
	while(*end) {
		end++;
	}
	size_t len = end - path;
	char *result = fpl_null;
	if(len > 0) {
		if(path[len - 1] != FPL_PATH_SEPARATOR) {
			if(len + 1 <= maxPathLen) {
				path[len] = FPL_PATH_SEPARATOR;
				path[len + 1] = 0;
				result = &path[len + 1];
			} else {
				FPL_ERROR(FPL__MODULE_PATHS, "Cannot append path separator: Max length '%zu' of path '%s' is exceeded", maxPathLen, path);
			}
		} else {
			result = &path[len];
		}
	}
	return(result);
}

fpl_common_api char *fplEnforcePathSeparator(char *path) {
	FPL__CheckArgumentNull(path, fpl_null);
	char *end = path;
	while(*end) {
		end++;
	}
	size_t len = end - path;
	char *result = fpl_null;
	if(len > 0) {
		if(path[len - 1] != FPL_PATH_SEPARATOR) {
			path[len] = FPL_PATH_SEPARATOR;
			path[len + 1] = 0;
			result = &path[len + 1];
		} else {
			result = &path[len];
		}
	}
	return(result);
}

fpl_common_api char *fplStringAppendLen(const char *appended, const size_t appendedLen, char *buffer, size_t maxBufferLen) {
	FPL__CheckArgumentNull(appended, fpl_null);
	FPL__CheckArgumentZero(maxBufferLen, fpl_null);
	if(appendedLen == 0) {
		return buffer;
	}
	size_t curBufferLen = fplGetAnsiStringLength(buffer);
	size_t requiredSize = curBufferLen + appendedLen + 1;
	FPL__CheckArgumentMin(maxBufferLen, requiredSize, fpl_null);
	char *str = buffer + curBufferLen;
	size_t remainingBufferSize = maxBufferLen - (curBufferLen > 0 ? curBufferLen + 1 : 0);
	fplCopyAnsiStringLen(appended, appendedLen, str, remainingBufferSize);
	return(str);
}

fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen) {
	size_t appendedLen = fplGetAnsiStringLength(appended);
	char *result = fplStringAppendLen(appended, appendedLen, buffer, maxBufferLen);
	return(result);
}

fpl_common_api size_t fplGetAnsiStringLength(const char *str) {
	uint32_t result = 0;
	if(str != fpl_null) {
		while(*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api size_t fplGetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if(str != fpl_null) {
		while(*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api char *fplCopyAnsiStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen) {
	if(source != fpl_null && dest != fpl_null) {
		size_t requiredLen = sourceLen + 1;
		FPL__CheckArgumentMin(maxDestLen, requiredLen, fpl_null);
		fplMemoryCopy(source, sourceLen * sizeof(char), dest);
		char *result = dest + sourceLen;
		*result = 0;
		return(result);
	} else {
		return(fpl_null);
	}
}

fpl_common_api char *fplCopyAnsiString(const char *source, char *dest, const size_t maxDestLen) {
	char *result = fpl_null;
	if(source != fpl_null) {
		size_t sourceLen = fplGetAnsiStringLength(source);
		result = fplCopyAnsiStringLen(source, sourceLen, dest, maxDestLen);
	}
	return(result);
}

fpl_common_api wchar_t *fplCopyWideStringLen(const wchar_t *source, const size_t sourceLen, wchar_t *dest, const size_t maxDestLen) {
	if(source != fpl_null && dest != fpl_null) {
		size_t requiredLen = sourceLen + 1;
		FPL__CheckArgumentMin(maxDestLen, requiredLen, fpl_null);
		fplMemoryCopy(source, sourceLen * sizeof(wchar_t), dest);
		wchar_t *result = dest + sourceLen;
		*result = 0;
		return(result);
	} else {
		return(fpl_null);
	}
}

fpl_common_api wchar_t *fplCopyWideString(const wchar_t *source, wchar_t *dest, const size_t maxDestLen) {
	wchar_t *result = fpl_null;
	if(source != fpl_null) {
		size_t sourceLen = fplGetWideStringLength(source);
		result = fplCopyWideStringLen(source, sourceLen, dest, maxDestLen);
	}
	return(result);
}

fpl_common_api char *fplFormatAnsiStringArgs(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, va_list argList) {
	FPL__CheckArgumentNull(ansiDestBuffer, fpl_null);
	FPL__CheckArgumentZero(maxAnsiDestBufferLen, fpl_null);
	FPL__CheckArgumentNull(format, fpl_null);
	FPL__CheckArgumentNull(argList, fpl_null);
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
	if(charCount < 0) {
		FPL_ERROR(FPL__MODULE_STRINGS, "Format parameter are '%s' are invalid", format);
		return fpl_null;
	}
	size_t requiredMaxAnsiDestBufferLen = charCount + 1;
	FPL__CheckArgumentMin(maxAnsiDestBufferLen, requiredMaxAnsiDestBufferLen, fpl_null);
	ansiDestBuffer[charCount] = 0;
	return(&ansiDestBuffer[charCount]);
}

fpl_common_api char *fplFormatAnsiString(char *ansiDestBuffer, const size_t maxAnsiDestBufferLen, const char *format, ...) {
	FPL__CheckArgumentNull(ansiDestBuffer, fpl_null);
	FPL__CheckArgumentZero(maxAnsiDestBufferLen, fpl_null);
	FPL__CheckArgumentNull(format, fpl_null);
	va_list argList;
	va_start(argList, format);
	char *result = fplFormatAnsiStringArgs(ansiDestBuffer, maxAnsiDestBufferLen, format, argList);
	va_end(argList);
	return(result);
}

fpl_common_api char *fplS32ToString(const int32_t value, const size_t maxBufferLen, char *buffer) {
	FPL__CheckArgumentNull(buffer, fpl_null);
	FPL__CheckArgumentZero(maxBufferLen, fpl_null);
	int32_t v = value;
	char *p = buffer;
	if(v < 0) {
		*p++ = '-';
		v = -v;
	}
	int tmp = v;
	do {
		++p;
		tmp = tmp / 10;
	} while(tmp);
	size_t digitCount = (p - buffer);
	FPL__CheckArgumentMin(maxBufferLen, digitCount + 1, fpl_null);
	*p = 0;
	const char *digits = "0123456789";
	v = value;
	do {
		*--p = digits[v % 10];
		v /= 10;
	} while(v != 0);
	return (p);
}

fpl_common_api int32_t fplStringToS32Len(const char *str, const size_t len) {
	FPL__CheckArgumentNull(str, 0);
	FPL__CheckArgumentZero(len, 0);
	const char *p = str;
	bool isNegative = false;
	if(*p == '-') {
		if(len == 1) {
			return 0;
		}
		isNegative = true;
		++p;
	}
	uint32_t value = 0;
	while(*p && ((size_t)(p - str) < len)) {
		char c = *p;
		if(c < '0' || c > '9') {
			return(0);
		}
		int v = (int)(*p - '0');
		value *= 10;
		value += (uint32_t)v;
		++p;
	}
	int32_t result = isNegative ? -(int32_t)value : (int32_t)value;
	return(result);
}

fpl_common_api int32_t fplStringToS32(const char *str) {
	size_t len = fplGetAnsiStringLength(str);
	int32_t result = fplStringToS32Len(str, len);
	return(result);
}
#endif // FPL__COMMON_STRINGS_DEFINED

//
// Common Console
//
#if !defined(FPL__COMMON_CONSOLE_DEFINED)
#define FPL__COMMON_CONSOLE_DEFINED

fpl_common_api void fplConsoleFormatOut(const char *format, ...) {
	FPL__CheckArgumentNullNoRet(format);
	char buffer[1024 * 10];
	va_list argList;
	va_start(argList, format);
	char *str = fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	va_end(argList);
	if(str != fpl_null) {
		fplConsoleOut(buffer);
	}
}

fpl_common_api void fplConsoleFormatError(const char *format, ...) {
	FPL__CheckArgumentNullNoRet(format);
	char buffer[1024];
	va_list argList;
	va_start(argList, format);
	char *str = fplFormatAnsiStringArgs(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
	va_end(argList);
	if(str != fpl_null) {
		fplConsoleError(buffer);
	}
}
#endif // FPL__COMMON_CONSOLE_DEFINED

//
// Common Memory
//
#if !defined(FPL__COMMON_MEMORY_DEFINED)
#define FPL__COMMON_MEMORY_DEFINED

fpl_common_api void *fplMemoryAlignedAllocate(const size_t size, const size_t alignment) {
	FPL__CheckArgumentZero(size, fpl_null);
	FPL__CheckArgumentZero(alignment, fpl_null);
	if(alignment & (alignment - 1)) {
		FPL_ERROR(FPL__MODULE_MEMORY, "Alignment parameter '%zu' must be a power of two", alignment);
		return fpl_null;
	}
	// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
	size_t newSize = sizeof(void *) + size + (alignment << 1);
	void *basePtr = fplMemoryAllocate(newSize);
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

fpl_common_api void fplMemoryAlignedFree(void *ptr) {
	FPL__CheckArgumentNullNoRet(ptr);
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
	FPL__CheckArgumentNullNoRet(mem);
	FPL__CheckArgumentZeroNoRet(size);
	if(size % 8 == 0) {
		FPL__MEMORY_SET(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64, value);
	} else if(size % 4 == 0) {
		FPL__MEMORY_SET(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32, value);
	} else if(size % 2 == 0) {
		FPL__MEMORY_SET(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16, value);
	} else {
		FPL__MEMORY_SET(uint8_t, mem, size, 0, 0, value);
	}
}

fpl_common_api void fplMemoryClear(void *mem, const size_t size) {
	FPL__CheckArgumentNullNoRet(mem);
	FPL__CheckArgumentZeroNoRet(size);
	if(size % 8 == 0) {
		FPL__MEMORY_CLEAR(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if(size % 4 == 0) {
		FPL__MEMORY_CLEAR(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if(size % 2 == 0) {
		FPL__MEMORY_CLEAR(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16);
	} else {
		FPL__MEMORY_CLEAR(uint8_t, mem, size, 0, 0);
	}
}

fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem) {
	FPL__CheckArgumentNullNoRet(sourceMem);
	FPL__CheckArgumentZeroNoRet(sourceSize);
	FPL__CheckArgumentNullNoRet(targetMem);
	if(sourceSize % 8 == 0) {
		FPL__MEMORY_COPY(uint64_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if(sourceSize % 4 == 0) {
		FPL__MEMORY_COPY(uint32_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if(sourceSize % 2 == 0) {
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

fpl_common_api size_t fplAtomicAddSize(volatile size_t *dest, const size_t addend) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicAddU64((volatile uint64_t *)dest, (uint64_t)addend);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicAddU32((volatile uint32_t *)dest, (uint32_t)addend);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicIncSize(volatile size_t *dest) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicIncU64((volatile uint64_t *)dest);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicIncU32((volatile uint32_t *)dest);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value) {
	FPL_ASSERT(target != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicExchangeSize(volatile size_t *target, const size_t value) {
	FPL_ASSERT(target != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicCompareAndExchangeSize(volatile size_t *dest, const size_t comparand, const size_t exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api bool fplIsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	bool result = fplIsAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	bool result = fplIsAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api bool fplIsAtomicCompareAndExchangeSize(volatile size_t *dest, const size_t comparand, const size_t exchange) {
	FPL_ASSERT(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	bool result = fplIsAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	bool result = fplIsAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicLoadPtr(volatile void **source) {
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicLoadU64((volatile uint64_t *)source);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicLoadU32((volatile uint32_t *)source);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return(result);
}

fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value) {
#if defined(FPL_CPU_64BIT)
	fplAtomicStoreU64((volatile uint64_t *)dest, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	fplAtomicStoreU32((volatile uint32_t *)dest, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
}

fpl_common_api size_t fplAtomicLoadSize(volatile size_t *source) {
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicLoadU64((volatile uint64_t *)source);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicLoadU32((volatile uint32_t *)source);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return(result);
}

fpl_common_api void fplAtomicStoreSize(volatile size_t *dest, const size_t value) {
#if defined(FPL_CPU_64BIT)
	fplAtomicStoreU64((volatile uint64_t *)dest, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	fplAtomicStoreU32((volatile uint32_t *)dest, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
}

#endif // FPL__COMMON_ATOMICS_DEFINED

//
// Common Threading
//
fpl_common_api fplThreadState fplGetThreadState(fplThreadHandle *thread) {
	if(thread == fpl_null) {
		return fplThreadState_Stopped;
	}
	fplThreadState result = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&thread->currentState);
	return(result);
}

//
// Common Files
//
#if !defined(FPL__COMMON_FILES_DEFINED)
#define FPL__COMMON_FILES_DEFINED

fpl_common_api size_t fplReadFileBlock(const fplFileHandle *fileHandle, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize) {
#if defined(FPL_CPU_64BIT)
	return fplReadFileBlock64(fileHandle, sizeToRead, targetBuffer, maxTargetBufferSize);
#else
	return fplReadFileBlock32(fileHandle, (uint32_t)sizeToRead, targetBuffer, (uint32_t)maxTargetBufferSize);
#endif
}

fpl_common_api size_t fplWriteFileBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize) {
#if defined(FPL_CPU_64BIT)
	return fplWriteFileBlock64(fileHandle, sourceBuffer, sourceSize);
#else
	return fplWriteFileBlock32(fileHandle, sourceBuffer, (uint32_t)sourceSize);
#endif
}

fpl_common_api size_t fplSetFilePosition(const fplFileHandle *fileHandle, const intptr_t position, const fplFilePositionMode mode) {
#if defined(FPL_CPU_64BIT)
	return fplSetFilePosition64(fileHandle, position, mode);
#else
	return fplSetFilePosition32(fileHandle, (int32_t)position, mode);
#endif
}

fpl_common_api size_t fplGetFilePosition(const fplFileHandle *fileHandle) {
#if defined(FPL_CPU_64BIT)
	return fplGetFilePosition64(fileHandle);
#else
	return fplGetFilePosition32(fileHandle);
#endif
}

fpl_common_api size_t fplGetFileSizeFromPath(const char *filePath) {
#if defined(FPL_CPU_64BIT)
	return fplGetFileSizeFromPath64(filePath);
#else
	return fplGetFileSizeFromPath32(filePath);
#endif
}

fpl_common_api size_t fplGetFileSizeFromHandle(const fplFileHandle *fileHandle) {
#if defined(FPL_CPU_64BIT)
	return fplGetFileSizeFromHandle64(fileHandle);
#else
	return fplGetFileSizeFromHandle32(fileHandle);
#endif
}

#endif // FPL__COMMON_FILES_DEFINED

//
// Common Paths
//
#if !defined(FPL__COMMON_PATHS_DEFINED)
#define FPL__COMMON_PATHS_DEFINED

fpl_common_api char *fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(sourcePath, fpl_null);
	size_t sourceLen = fplGetAnsiStringLength(sourcePath);
	FPL__CheckArgumentZero(sourceLen, fpl_null);
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredDestLen = sourceLen + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredDestLen, fpl_null);
	char *result = fpl_null;
	if(sourcePath) {
		int copyLen = 0;
		char *chPtr = (char *)sourcePath;
		while(*chPtr) {
			if(*chPtr == FPL_PATH_SEPARATOR) {
				copyLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		if(copyLen) {
			result = fplCopyAnsiStringLen(sourcePath, copyLen, destPath, maxDestLen);
		}
	}
	return(result);
}

fpl_common_api const char *fplExtractFileExtension(const char *sourcePath) {
	const char *result = fpl_null;
	if(sourcePath != fpl_null) {
		const char *chPtr = sourcePath;
		const char *last = fpl_null;
		while(*chPtr) {
			if(*chPtr == FPL_FILE_EXT_SEPARATOR) {
				last = chPtr;
			}
			++chPtr;
		}
		if(last != fpl_null) {
			result = last;
		}
	}
	return(result);
}

fpl_common_api const char *fplExtractFileName(const char *sourcePath) {
	const char *result = fpl_null;
	if(sourcePath) {
		result = sourcePath;
		const char *chPtr = sourcePath;
		while(*chPtr) {
			if(*chPtr == FPL_PATH_SEPARATOR) {
				result = chPtr + 1;
			}
			++chPtr;
		}
	}
	return(result);
}

fpl_common_api char *fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(filePath, fpl_null);
	FPL__CheckArgumentNull(newFileExtension, fpl_null);
	size_t pathLen = fplGetAnsiStringLength(filePath);
	FPL__CheckArgumentZero(pathLen, fpl_null);
	size_t extLen = fplGetAnsiStringLength(newFileExtension);
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredDestLen = pathLen + extLen + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredDestLen, fpl_null);
	char *result = fpl_null;
	if(filePath != fpl_null) {
		// Find last path
		char *chPtr = (char *)filePath;
		char *lastPathSeparatorPtr = fpl_null;
		while(*chPtr) {
			if(*chPtr == FPL_PATH_SEPARATOR) {
				lastPathSeparatorPtr = chPtr;
			}
			++chPtr;
		}
		// Find last ext separator
		if(lastPathSeparatorPtr != fpl_null) {
			chPtr = lastPathSeparatorPtr + 1;
		} else {
			chPtr = (char *)filePath;
		}
		char *lastExtSeparatorPtr = fpl_null;
		while(*chPtr) {
			if(*chPtr == FPL_FILE_EXT_SEPARATOR) {
				lastExtSeparatorPtr = chPtr;
			}
			++chPtr;
		}
		size_t copyLen;
		if(lastExtSeparatorPtr != fpl_null) {
			copyLen = (size_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
		} else {
			copyLen = pathLen;
		}
		// Copy parts
		fplCopyAnsiStringLen(filePath, copyLen, destPath, maxDestLen);
		char *destExtPtr = destPath + copyLen;
		result = fplCopyAnsiStringLen(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
	}
	return(result);
}

fpl_common_api char *fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	FPL__CheckArgumentZero(maxDestPathLen, fpl_null);
	FPL__CheckArgumentZero(pathCount, fpl_null);
	size_t curDestPosition = 0;
	char *currentDestPtr = destPath;
	va_list vargs;
	va_start(vargs, pathCount);
	for(size_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		char *path = va_arg(vargs, char *);
		size_t pathLen = fplGetAnsiStringLength(path);
		bool requireSeparator = pathIndex < (pathCount - 1);
		size_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
		FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
		fplCopyAnsiStringLen(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
		currentDestPtr += pathLen;
		if(requireSeparator) {
			*currentDestPtr++ = FPL_PATH_SEPARATOR;
		}
		curDestPosition += requiredPathLen;
	}
	*currentDestPtr = 0;
	va_end(vargs);
	return currentDestPtr;
}
#endif // FPL__COMMON_PATHS_DEFINED

#if defined(FPL_ENABLE_WINDOW)

#if !defined(FPL__COMMON_WINDOW_DEFINED)
#define FPL__COMMON_WINDOW_DEFINED

fpl_common_api bool fplPollEvent(fplEvent *ev) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	bool result = false;
	if(eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
		uint32_t eventIndex = fplAtomicAddU32(&eventQueue->pollIndex, 1);
		*ev = eventQueue->events[eventIndex];
		result = true;
	} else if(eventQueue->pushCount > 0) {
		fplAtomicExchangeU32(&eventQueue->pollIndex, 0);
		fplAtomicExchangeU32(&eventQueue->pushCount, 0);
	}
	return result;
}

fpl_common_api void fplClearEvents() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	fplAtomicExchangeU32(&eventQueue->pollIndex, 0);
	fplAtomicExchangeU32(&eventQueue->pushCount, 0);
}
#endif // FPL__COMMON_WINDOW_DEFINED

#endif // FPL_ENABLE_WINDOW

#if defined(FPL_ENABLE_LOGGING)
fpl_common_api void fplSetLogSettings(const fplLogSettings *params) {
	FPL__CheckArgumentNullNoRet(params);
	fpl__global__LogSettings = *params;
	fpl__global__LogSettings.isInitialized = true;
}
fpl_common_api const fplLogSettings *fplGetLogSettings() {
	return &fpl__global__LogSettings;
}
fpl_common_api void fplSetMaxLogLevel(const fplLogLevel maxLevel) {
	fpl__global__LogSettings.maxLevel = maxLevel;
}
fpl_common_api fplLogLevel fplGetMaxLogLevel() {
	return fpl__global__LogSettings.maxLevel;
}
#endif

fpl_common_api const char *fplGetLastError() {
	const char *result = "";
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	if(errorState->count > 0) {
		size_t index = errorState->count - 1;
		result = fplGetErrorByIndex(index);
	}
	return (result);
}

fpl_common_api const char *fplGetErrorByIndex(const size_t index) {
	const char *result = "";
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	if(index < errorState->count) {
		result = errorState->errors[index];
	} else {
		result = errorState->errors[errorState->count - 1];
	}
	return (result);
}

fpl_common_api size_t fplGetErrorCount() {
	size_t result = 0;
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	result = errorState->count;
	return (result);
}

fpl_common_api void fplClearErrors() {
	fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	FPL_CLEAR_STRUCT(errorState);
}

fpl_common_api const fplSettings *fplGetCurrentSettings() {
	FPL__CheckPlatform(fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	return &appState->currentSettings;
}

fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video) {
	FPL__CheckArgumentNullNoRet(video);
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
	FPL__CheckArgumentNullNoRet(audio);
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
	FPL__CheckArgumentNullNoRet(window);
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
	FPL__CheckArgumentNullNoRet(input);
	FPL_CLEAR_STRUCT(input);
	input->controllerDetectionFrequency = 100;
}

fpl_common_api void fplSetDefaultSettings(fplSettings *settings) {
	FPL__CheckArgumentNullNoRet(settings);
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

fpl_common_api const char *fplGetInitResultTypeString(const fplInitResultType type) {
	switch(type) {
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

fpl_common_api const char *fplGetArchTypeString(const fplArchType type) {
	switch(type) {
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
	if(settings->isFullscreen) {
		result |= WS_POPUP;
	} else {
		result |= WS_SYSMENU | WS_MINIMIZEBOX;
		if(settings->isDecorated) {
			result |= WS_CAPTION;
			if(settings->isResizable) {
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
	if(settings->isFullscreen || settings->isFloating) {
		result |= WS_EX_TOPMOST;
	}
	return(result);
}

fpl_internal_inline void fpl__Win32UpdateWindowStyles(const fplWindowSettings *settings, const fpl__Win32WindowState *windowState) {
	DWORD style = fpl__Win32GetWindowStyle(settings);
	DWORD exStyle = fpl__Win32GetWindowExStyle(settings);
	fpl__win32_SetWindowLong(windowState->windowHandle, GWL_STYLE, style);
	fpl__win32_SetWindowLong(windowState->windowHandle, GWL_EXSTYLE, exStyle);
}

fpl_internal void fpl__Win32SaveWindowState(const fpl__Win32Api *wapi, fpl__Win32LastWindowInfo *target, HWND windowHandle) {
	target->isMaximized = !!wapi->user.IsZoomed(windowHandle);
	target->isMinimized = !!wapi->user.IsIconic(windowHandle);
	target->style = fpl__win32_GetWindowLong(windowHandle, GWL_STYLE);
	target->exStyle = fpl__win32_GetWindowLong(windowHandle, GWL_EXSTYLE);
	wapi->user.GetWindowPlacement(windowHandle, &target->placement);
}

fpl_internal void fpl__Win32RestoreWindowState(const fpl__Win32Api *wapi, const fpl__Win32LastWindowInfo *target, HWND windowHandle) {
	FPL_ASSERT(target->style > 0 && target->exStyle > 0);
	fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, target->style);
	fpl__win32_SetWindowLong(windowHandle, GWL_EXSTYLE, target->exStyle);
	wapi->user.SetWindowPlacement(windowHandle, &target->placement);
	wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	if(target->isMaximized) {
		fpl__win32_SendMessage(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	} else if(target->isMinimized) {
		fpl__win32_SendMessage(windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
}

fpl_internal bool fpl__Win32LeaveFullscreen() {
	const fpl__PlatformAppState *platState = fpl__global__AppState;
	FPL_ASSERT(platState != fpl_null);
	const fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	const fpl__Win32WindowState *win32Window = &platState->window.win32;
	const fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;
	HWND windowHandle = win32Window->windowHandle;
	fpl__Win32RestoreWindowState(wapi, fullscreenInfo, windowHandle);
	bool result;
	if(fullscreenInfo->wasResolutionChanged) {
		result = (wapi->user.ChangeDisplaySettingsA(fpl_null, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
	} else {
		result = true;
	}
	return(result);
}

fpl_internal bool fpl__Win32EnterFullscreen(const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits) {
	fpl__PlatformAppState *platState = fpl__global__AppState;
	FPL_ASSERT(platState != fpl_null);
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
	if(fullscreenWidth > 0 && fullscreenHeight > 0) {
		DWORD useFullscreenWidth = fullscreenWidth;
		DWORD useFullscreenHeight = fullscreenHeight;

		DWORD useRefreshRate = refreshRate;
		if(!useRefreshRate) {
			useRefreshRate = wapi->gdi.GetDeviceCaps(deviceContext, VREFRESH);
		}

		DWORD useColourBits = colorBits;
		if(!useColourBits) {
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
		placement.showCmd = SW_SHOWNORMAL;
		wapi->user.SetWindowPlacement(windowHandle, &placement);
		wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		result = true;
		fullscreenInfo->wasResolutionChanged = false;
	}

	return(result);
}

fpl_internal_inline float fpl__Win32XInputProcessStickValue(const SHORT value, const SHORT deadZoneThreshold) {
	float result = 0;
	if(value < -deadZoneThreshold) {
		result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	} else if(value > deadZoneThreshold) {
		result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}
	return(result);
}

fpl_internal void fpl__Win32XInputGamepadToGamepadState(const XINPUT_GAMEPAD *pad, fplGamepadState *outState) {
	outState->isConnected = true;

	// Analog sticks
	outState->leftStickX = fpl__Win32XInputProcessStickValue(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	outState->leftStickY = fpl__Win32XInputProcessStickValue(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	outState->rightStickX = fpl__Win32XInputProcessStickValue(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	outState->rightStickY = fpl__Win32XInputProcessStickValue(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

	// Triggers
	outState->leftTrigger = (float)pad->bLeftTrigger / 255.0f;
	outState->rightTrigger = (float)pad->bRightTrigger / 255.0f;

	// Digital pad buttons
	if(pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
		outState->dpadUp.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		outState->dpadDown.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		outState->dpadLeft.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		outState->dpadRight.isDown = true;

	// Action buttons
	if(pad->wButtons & XINPUT_GAMEPAD_A)
		outState->actionA.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_B)
		outState->actionB.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_X)
		outState->actionX.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_Y)
		outState->actionY.isDown = true;

	// Center buttons
	if(pad->wButtons & XINPUT_GAMEPAD_START)
		outState->start.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_BACK)
		outState->back.isDown = true;

	// Shoulder buttons
	if(pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		outState->leftShoulder.isDown = true;
	if(pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		outState->rightShoulder.isDown = true;
}

fpl_internal void fpl__Win32PollControllers(const fplSettings *settings, const fpl__Win32InitState *initState, fpl__Win32XInputState *xinputState) {
	FPL_ASSERT(settings != fpl_null);
	FPL_ASSERT(xinputState != fpl_null);
	if(xinputState->xinputApi.xInputGetState != fpl_null) {
		//
		// Detect new controller (Only on a fixed frequency)
		//
		if(xinputState->lastDeviceSearchTime.QuadPart == 0) {
			QueryPerformanceCounter(&xinputState->lastDeviceSearchTime);
		}
		LARGE_INTEGER currentDeviceSearchTime;
		QueryPerformanceCounter(&currentDeviceSearchTime);
		uint64_t deviceSearchDifferenceTimeInMs = ((currentDeviceSearchTime.QuadPart - xinputState->lastDeviceSearchTime.QuadPart) / (initState->performanceFrequency.QuadPart / 1000));
		if((settings->input.controllerDetectionFrequency == 0) || (deviceSearchDifferenceTimeInMs > settings->input.controllerDetectionFrequency)) {
			xinputState->lastDeviceSearchTime = currentDeviceSearchTime;
			for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
				XINPUT_STATE controllerState = FPL_ZERO_INIT;
				if(xinputState->xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					if(!xinputState->isConnected[controllerIndex]) {
						// Connected
						xinputState->isConnected[controllerIndex] = true;
						fplEvent ev = FPL_ZERO_INIT;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Connected;
						ev.gamepad.deviceIndex = controllerIndex;
						fpl__PushEvent(&ev);
					}
				} else {
					if(xinputState->isConnected[controllerIndex]) {
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
		for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
			if(xinputState->isConnected[controllerIndex]) {
				XINPUT_STATE controllerState = FPL_ZERO_INIT;
				if(xinputState->xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					// State changed
					fplEvent ev = FPL_ZERO_INIT;
					ev.type = fplEventType_Gamepad;
					ev.gamepad.type = fplGamepadEventType_StateChanged;
					ev.gamepad.deviceIndex = controllerIndex;
					XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
					fpl__Win32XInputGamepadToGamepadState(pad, &ev.gamepad.state);
					fpl__PushEvent(&ev);
				}
			}
		}
	}
}

fpl_internal_inline bool fpl__Win32IsKeyDown(const fpl__Win32Api *wapi, const int virtualKey) {
	bool result = (wapi->user.GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	return(result);
}

fpl_internal_inline bool fpl__Win32IsKeyActive(const fpl__Win32Api *wapi, const int virtualKey) {
	bool result = (wapi->user.GetKeyState(virtualKey) & 0x0001) != 0;
	return(result);
}

fpl_internal_inline bool fpl__Win32IsCursorInWindow(const fpl__Win32Api *wapi, const fpl__Win32WindowState *win32Window) {
	POINT pos;
	if(!wapi->user.GetCursorPos(&pos)) {
		return false;
	}
	// Not this window?
	if(wapi->user.WindowFromPoint(pos) != win32Window->windowHandle) {
		return false;
	}
	// Cursor in client rect?
	RECT area;
	wapi->user.GetClientRect(win32Window->windowHandle, &area);

	wapi->user.ClientToScreen(win32Window->windowHandle, (LPPOINT)&area.left);
	wapi->user.ClientToScreen(win32Window->windowHandle, (LPPOINT)&area.right);
	bool result = wapi->user.PtInRect(&area, pos) == TRUE;

	return(result);
}

fpl_internal void fpl__Win32LoadCursor(const fpl__Win32Api *wapi, const fpl__Win32WindowState *window) {
	if(window->isCursorActive) {
		wapi->user.SetCursor(fpl__win32_LoadCursor(fpl_null, IDC_ARROW));
	} else {
		wapi->user.SetCursor(fpl_null);
	}
}

fpl_internal void fpl__Win32UpdateClipRect(const fpl__Win32Api *wapi, const fpl__Win32WindowState *window) {
	if(window != fpl_null) {
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
	// @NOTE(final): We use RAWINPUT to remove the mouse device entirely when it needs to be hidden
	if(!state) {
		const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window->windowHandle };
		if(!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			FPL_ERROR(FPL__MODULE_WINDOW, "Failed register raw input mouse device for window handle '%p'", window->windowHandle);
		}
	} else {
		const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, fpl_null };
		if(!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			FPL_ERROR(FPL__MODULE_WINDOW, "Failed to unregister raw input mouse device");
		}
	}
	if(fpl__Win32IsCursorInWindow(wapi, window)) {
		fpl__Win32LoadCursor(wapi, window);
	}
}

fpl_internal_inline void fpl__Win32ShowCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
	fpl__Win32SetCursorState(wapi, window, false);
}
fpl_internal_inline void fpl__Win32HideCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
	fpl__Win32SetCursorState(wapi, window, true);
}

fpl_internal fplKeyboardModifierFlags fpl__Win32GetKeyboardModifiers(const fpl__Win32Api *wapi) {
	fplKeyboardModifierFlags modifiers = fplKeyboardModifierFlags_None;
	bool lAltKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_LMENU);
	bool rAltKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_RMENU);
	bool lShiftKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_LSHIFT);
	bool rShiftKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_RSHIFT);
	bool lCtrlKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_LCONTROL);
	bool rCtrlKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_RCONTROL);
	bool lSuperKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_LWIN);
	bool rSuperKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_RWIN);
	bool capsLockActive = fpl__Win32IsKeyActive(wapi, VK_CAPITAL);
	bool numLockActive = fpl__Win32IsKeyActive(wapi, VK_NUMLOCK);
	bool scrollLockActive = fpl__Win32IsKeyActive(wapi, VK_SCROLL);
	if(lAltKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LAlt;
	}
	if(rAltKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RAlt;
	}
	if(lShiftKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LShift;
	}
	if(rShiftKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RShift;
	}
	if(lCtrlKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LCtrl;
	}
	if(rCtrlKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RCtrl;
	}
	if(lSuperKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LSuper;
	}
	if(rSuperKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RSuper;
	}
	if(capsLockActive) {
		modifiers |= fplKeyboardModifierFlags_CapsLock;
	}
	if(numLockActive) {
		modifiers |= fplKeyboardModifierFlags_NumLock;
	}
	if(scrollLockActive) {
		modifiers |= fplKeyboardModifierFlags_ScrollLock;
	}
	return(modifiers);
}

LRESULT CALLBACK fpl__Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_ASSERT(appState != fpl_null);

	fpl__Win32AppState *win32State = &appState->win32;
	fpl__Win32WindowState *win32Window = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32State->winApi;

	if(!win32Window->windowHandle) {
		return fpl__win32_DefWindowProc(hwnd, msg, wParam, lParam);
	}

	LRESULT result = 0;
	switch(msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			appState->window.isRunning = false;
		} break;

		case WM_SIZE:
		{
			// @TODO(final): Win32 save maximize/minimize state here
			DWORD newWidth = LOWORD(lParam);
			DWORD newHeight = HIWORD(lParam);
			if(wParam == SIZE_MAXIMIZED) {
				fpl__PushWindowEvent(fplWindowEventType_Maximized, newWidth, newHeight);
			} else if(wParam == SIZE_MINIMIZED) {
				fpl__PushWindowEvent(fplWindowEventType_Minimized, newWidth, newHeight);
			} else if(wParam == SIZE_RESTORED) {
				fpl__PushWindowEvent(fplWindowEventType_Restored, newWidth, newHeight);
			}

#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			if(appState->currentSettings.video.driver == fplVideoDriverType_Software) {
				if(appState->initSettings.video.isAutoSize) {
					fplResizeVideoBackBuffer(newWidth, newHeight);
				}
			}
#			endif

			fpl__PushWindowEvent(fplWindowEventType_Resized, newWidth, newHeight);

			return 0;
		} break;

		case WM_DROPFILES:
		{
			HDROP dropHandle = (HDROP)wParam;
			char fileBufferA[FPL_MAX_PATH_LENGTH];
			UINT fileCount;
#if UNICODE
			wchar_t fileBufferW[FPL_MAX_PATH_LENGTH];
			fileCount = wapi->shell.DragQueryFileW(dropHandle, 0xFFFFFFFF, fileBufferW, 0);
#else
			fileCount = wapi->shell.DragQueryFileA(dropHandle, 0xFFFFFFFF, fileBufferA, 0);
#endif
			// @TODO(final): Win32 support for dropping in multiple files
			if(fileCount > 0) {
				UINT dragResult;
#if UNICODE
				fileBufferW[0] = 0;
				dragResult = wapi->shell.DragQueryFileW(dropHandle, 0, fileBufferW, FPL_ARRAYCOUNT(fileBufferW));
				size_t sourceLen = fplGetWideStringLength(fileBufferW);
				int destLen = WideCharToMultiByte(CP_UTF8, 0, fileBufferW, (int)sourceLen, fileBufferA, FPL_ARRAYCOUNT(fileBufferA), fpl_null, fpl_null);
				if(destLen >= 0) {
					fileBufferA[destLen] = 0;
				}
#else
				dragResult = wapi->shell.DragQueryFileA(dropHandle, 0, fileBufferA, FPL_ARRAYCOUNT(fileBufferA));
#endif
				if(dragResult != 0) {
					fpl__PushWindowDropSingleFileEvent(fileBufferA);
				}
	}
} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint64_t keyCode = wParam;
			bool isDown = ((int)(lParam & (1 << 31)) == 0);
			bool wasDown = ((int)(lParam & (1 << 30)) != 0);
			bool altKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_MENU);
			fplButtonState keyState = isDown ? fplButtonState_Press : fplButtonState_Release;
			fplKeyboardModifierFlags modifiers = fpl__Win32GetKeyboardModifiers(wapi);
			fpl__HandleKeyboardButtonEvent(&appState->window, keyCode, modifiers, keyState, false);
			if(wasDown != isDown) {
				if(isDown) {
					if(keyCode == VK_F4 && altKeyIsDown) {
						appState->window.isRunning = false;
					}
				}
			}
		} break;

		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_UNICHAR:
		{
			if((msg == WM_UNICHAR) && (wParam == UNICODE_NOCHAR)) {
				// @NOTE(final): WM_UNICHAR was sent by a third-party input method. Do not add any chars here!
				return TRUE;
			}
			uint64_t keyCode = wParam;
			fpl__HandleKeyboardInputEvent(&appState->window, keyCode);
			return 0;
		} break;

		case WM_ACTIVATE:
		{
		} break;

		case WM_MOUSEACTIVATE:
		{
			// @NOTE(final): User starts to click/move the window frame
			if(HIWORD(lParam) == WM_LBUTTONDOWN) {
				if(LOWORD(lParam) == HTCLOSE || LOWORD(lParam) == HTMINBUTTON || LOWORD(lParam) == HTMAXBUTTON) {
					win32Window->isFrameInteraction = true;
				}
			}
		} break;

		case WM_CAPTURECHANGED:
		{
			// User is done with interaction with the the window frame
			if(lParam == 0 && win32Window->isFrameInteraction) {
				// Hide cursor when needed
				if(!win32Window->isCursorActive) {
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
			if(win32Window->isFrameInteraction) {
				break;
			}
			// Hide cursor when needed
			if(!win32Window->isCursorActive) {
				fpl__Win32HideCursor(wapi, win32Window);
			}
			return 0;
		} break;

		case WM_KILLFOCUS:
		{
			// Restore cursor when needed
			if(!win32Window->isCursorActive) {
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
			if(!win32Window->isCursorActive) {
				fpl__Win32ShowCursor(wapi, win32Window);
			}
		} break;

		case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
		{
			// Hide cursor when needed
			if(!win32Window->isCursorActive) {
				fpl__Win32HideCursor(wapi, win32Window);
			}
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			fplButtonState buttonState;
			if(msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN) {
				buttonState = fplButtonState_Press;
			} else {
				buttonState = fplButtonState_Release;
			}
			fplMouseButtonType mouseButton;
			if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) {
				mouseButton = fplMouseButtonType_Left;
			} else if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
				mouseButton = fplMouseButtonType_Right;
			} else if(msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) {
				mouseButton = fplMouseButtonType_Middle;
			} else {
				mouseButton = fplMouseButtonType_None;
			}
			if(mouseButton != fplMouseButtonType_None) {
				int32_t mouseX = GET_X_LPARAM(lParam);
				int32_t mouseY = GET_Y_LPARAM(lParam);
				fpl__HandleMouseButtonEvent(&appState->window, mouseX, mouseY, mouseButton, buttonState);
			}
		} break;
		case WM_MOUSEMOVE:
		{
			int32_t mouseX = GET_X_LPARAM(lParam);
			int32_t mouseY = GET_Y_LPARAM(lParam);
			fpl__HandleMouseMoveEvent(&appState->window, mouseX, mouseY);
		} break;
		case WM_MOUSEWHEEL:
		{
			int32_t mouseX = GET_X_LPARAM(lParam);
			int32_t mouseY = GET_Y_LPARAM(lParam);
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			float wheelDelta = zDelta / (float)WHEEL_DELTA;
			fpl__HandleMouseWheelEvent(&appState->window, mouseX, mouseY, wheelDelta);
		} break;

		case WM_SETCURSOR:
		{
			// @NOTE(final): Load cursor only when we are in the window client area
			if(LOWORD(lParam) == HTCLIENT) {
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

fpl_internal HICON fpl__Win32LoadIconFromImageSource(const fpl__Win32Api *wapi, const HINSTANCE appInstance, const fplImageSource *imageSource) {
	FPL_ASSERT(imageSource != fpl_null);
	HICON result = 0;
	if(imageSource->width > 0 && imageSource->height > 0 && imageSource->data > 0) {
		BITMAPV5HEADER bi = FPL_ZERO_INIT;
		bi.bV5Size = sizeof(bi);
		bi.bV5Width = (LONG)imageSource->width;
		bi.bV5Height = -(LONG)imageSource->height;
		bi.bV5Planes = 1;
		bi.bV5BitCount = 32;
		bi.bV5Compression = BI_BITFIELDS;
		bi.bV5RedMask = 0x00ff0000;
		bi.bV5GreenMask = 0x0000ff00;
		bi.bV5BlueMask = 0x000000ff;
		bi.bV5AlphaMask = 0xff000000;

		uint8_t* targetData = fpl_null;
		HDC dc = wapi->user.GetDC(fpl_null);
		HBITMAP colorBitmap = wapi->gdi.CreateDIBSection(dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&targetData, fpl_null, (DWORD)0);
		if(colorBitmap == fpl_null) {
			fpl__PushError(fplLogLevel_Error, "Failed to create DIBSection from image with size %lu x %lu", imageSource->width, imageSource->height);
		}
		wapi->user.ReleaseDC(fpl_null, dc);

		HBITMAP maskBitmap = wapi->gdi.CreateBitmap(imageSource->width, imageSource->height, 1, 1, fpl_null);
		if(maskBitmap == fpl_null) {
			fpl__PushError(fplLogLevel_Error, "Failed to create Bitmap Mask from image with size %lu x %lu", imageSource->width, imageSource->height);
		}

		if(colorBitmap != fpl_null && maskBitmap != fpl_null) {
			FPL_ASSERT(targetData != fpl_null);
			uint8_t *dst = targetData;
			const uint8_t *src = imageSource->data;

			if(imageSource->type == fplImageType_RGBA) {
				for(uint32_t i = 0; i < imageSource->width * imageSource->height; ++i) {
					dst[0] = src[2]; // R > B
					dst[1] = src[1]; // G > G
					dst[2] = src[0]; // B > R
					dst[3] = src[3]; // A > A
					src += 4;
					dst += 4;
				}
				ICONINFO ii = FPL_ZERO_INIT;
				ii.fIcon = TRUE;
				ii.xHotspot = 0;
				ii.yHotspot = 0;
				ii.hbmMask = maskBitmap;
				ii.hbmColor = colorBitmap;
				result = wapi->user.CreateIconIndirect(&ii);
			} else {
				fpl__PushError(fplLogLevel_Warning, "Image source type '%d' for icon is not supported", imageSource->type);
			}
		}
		if(colorBitmap != fpl_null) {
			wapi->gdi.DeleteObject(colorBitmap);
		}
		if(maskBitmap != fpl_null) {
			wapi->gdi.DeleteObject(maskBitmap);
		}
	}
	if(result == 0) {
		result = fpl__win32_LoadIcon(appInstance, IDI_APPLICATION);
	}
	return(result);
}

fpl_internal bool fpl__Win32InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *platAppState, fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	FPL_ASSERT(appState != fpl_null);
	const fpl__Win32Api *wapi = &appState->winApi;
	const fplWindowSettings *initWindowSettings = &initSettings->window;

	// Presetup window
	fpl__PreSetupWindowResult preSetupResult = FPL_ZERO_INIT;
	if(setupCallbacks->preSetup != fpl_null) {
		setupCallbacks->preSetup(platAppState, platAppState->initFlags, &platAppState->initSettings, &preSetupResult);
	}

	// Register window class
	fpl__win32_WNDCLASSEX windowClass = FPL_ZERO_INIT;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.hInstance = GetModuleHandleA(fpl_null);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hCursor = fpl__win32_LoadCursor(windowClass.hInstance, IDC_ARROW);
	windowClass.hIconSm = fpl__Win32LoadIconFromImageSource(wapi, windowClass.hInstance, &initWindowSettings->icons[0]);
	windowClass.hIcon = fpl__Win32LoadIconFromImageSource(wapi, windowClass.hInstance, &initWindowSettings->icons[1]);
	windowClass.lpszClassName = FPL__WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl__Win32MessageProc;
	windowClass.style |= CS_OWNDC;
#if _UNICODE
	fplCopyWideString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#else
	fplCopyAnsiString(windowClass.lpszClassName, windowState->windowClass, FPL_ARRAYCOUNT(windowState->windowClass));
#endif
	if(fpl__win32_RegisterClassEx(&windowClass) == 0) {
		FPL_ERROR(FPL__MODULE_WINDOW, "Failed registering window class '%s'", windowState->windowClass);
		return false;
	}

	// Set window title
#if _UNICODE
	wchar_t windowTitleBuffer[1024];
	if(fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplAnsiStringToWideString(initWindowSettings->windowTitle, fplGetAnsiStringLength(initWindowSettings->windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const wchar_t *defaultTitle = FPL__WIN32_UNNAMED_WINDOW;
		fplCopyWideString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	wchar_t *windowTitle = windowTitleBuffer;
	fplWideStringToAnsiString(windowTitle, fplGetWideStringLength(windowTitle), currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#else
	char windowTitleBuffer[1024];
	if(fplGetAnsiStringLength(initWindowSettings->windowTitle) > 0) {
		fplCopyAnsiString(initWindowSettings->windowTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	} else {
		const char *defaultTitle = FPL__WIN32_UNNAMED_WINDOW;
		fplCopyAnsiString(defaultTitle, windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
	}
	char *windowTitle = windowTitleBuffer;
	fplCopyAnsiString(windowTitle, currentWindowSettings->windowTitle, FPL_ARRAYCOUNT(currentWindowSettings->windowTitle));
#endif

	// Prepare window style, size and position
	DWORD style = fpl__Win32GetWindowStyle(&initSettings->window);
	DWORD exStyle = fpl__Win32GetWindowExStyle(&initSettings->window);
	if(initSettings->window.isResizable) {
		currentWindowSettings->isResizable = true;
	} else {
		currentWindowSettings->isResizable = false;
	}
	int windowX = CW_USEDEFAULT;
	int windowY = CW_USEDEFAULT;
	int windowWidth;
	int windowHeight;
	if((initWindowSettings->windowWidth > 0) &&
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
	if(windowState->windowHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_WINDOW, "Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", windowState->windowClass, windowX, windowY, windowWidth, windowHeight);
		return false;
	}

	// Accept files as drag & drop source
	wapi->shell.DragAcceptFiles(windowState->windowHandle, TRUE);

	// Get actual window size and store results
	currentWindowSettings->windowWidth = windowWidth;
	currentWindowSettings->windowHeight = windowHeight;
	RECT clientRect;
	if(wapi->user.GetClientRect(windowState->windowHandle, &clientRect)) {
		currentWindowSettings->windowWidth = clientRect.right - clientRect.left;
		currentWindowSettings->windowHeight = clientRect.bottom - clientRect.top;
	}

	// Get device context so we can swap the back and front buffer
	windowState->deviceContext = wapi->user.GetDC(windowState->windowHandle);
	if(windowState->deviceContext == fpl_null) {
		FPL_ERROR(FPL__MODULE_WINDOW, "Failed aquiring device context from window '%d'", windowState->windowHandle);
		return false;
	}

	// Call post window setup callback
	if(setupCallbacks->postSetup != fpl_null) {
		setupCallbacks->postSetup(platAppState, platAppState->initFlags, initSettings);
	}

	// Enter fullscreen if needed
	if(initWindowSettings->isFullscreen) {
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
	if(windowState->deviceContext != fpl_null) {
		wapi->user.ReleaseDC(windowState->windowHandle, windowState->deviceContext);
		windowState->deviceContext = fpl_null;
	}
	if(windowState->windowHandle != fpl_null) {
		wapi->user.DestroyWindow(windowState->windowHandle);
		windowState->windowHandle = fpl_null;
		fpl__win32_UnregisterClass(windowState->windowClass, initState->appInstance);
	}
}

#endif // FPL_ENABLE_WINDOW

fpl_api fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseWideArguments(LPWSTR cmdLine) {
	fpl__Win32CommandLineUTF8Arguments args = FPL_ZERO_INIT;

	// @NOTE(final): Temporary load and unload shell32 for parsing the arguments
	HMODULE shellapiLibrary = LoadLibraryA("shell32.dll");
	if(shellapiLibrary != fpl_null) {
		fpl__win32_func_CommandLineToArgvW *commandLineToArgvW = (fpl__win32_func_CommandLineToArgvW *)GetProcAddress(shellapiLibrary, "CommandLineToArgvW");
		if(commandLineToArgvW != fpl_null) {
			// Parse arguments and compute total UTF8 string length
			int executableFilePathArgumentCount = 0;
			wchar_t **executableFilePathArgs = commandLineToArgvW(L"", &executableFilePathArgumentCount);
			size_t executableFilePathLen = 0;
			for(int i = 0; i < executableFilePathArgumentCount; ++i) {
				if(i > 0) {
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
			if(cmdLine != fpl_null && fplGetWideStringLength(cmdLine) > 0) {
				actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
				for(int i = 0; i < actualArgumentCount; ++i) {
					size_t sourceLen = fplGetWideStringLength(actualArgs[i]);
					int destLen = WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], (int)sourceLen, fpl_null, 0, fpl_null, fpl_null);
					actualArgumentsLen += destLen;
				}
			}

			// Calculate argument 
			args.count = 1 + actualArgumentCount;
			size_t totalStringLen = executableFilePathLen + actualArgumentsLen + args.count;
			size_t singleArgStringSize = sizeof(char) * (totalStringLen);
			size_t arbitaryPadding = FPL__ARBITARY_PADDING;
			size_t argArraySize = sizeof(char **) * args.count;
			size_t totalArgSize = singleArgStringSize + arbitaryPadding + argArraySize;

			args.mem = (uint8_t *)fplMemoryAllocate(totalArgSize);
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
					size_t sourceArgLen = fplGetWideStringLength(sourceArg);
					int destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, fpl_null, 0, fpl_null, fpl_null);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, destArg, destArgLen, fpl_null, fpl_null);
					destArg += destArgLen;
				}
				*destArg++ = 0;
				LocalFree(executableFilePathArgs);
			}

			// Convert actual arguments to UTF8
			if(actualArgumentCount > 0) {
				FPL_ASSERT(actualArgs != fpl_null);
				for(int i = 0; i < actualArgumentCount; ++i) {
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

fpl_api fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseAnsiArguments(LPSTR cmdLine) {
	fpl__Win32CommandLineUTF8Arguments result;
	if(cmdLine != fpl_null) {
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
	FPL__CheckArgumentNull(threads, false);
	FPL__CheckArgumentMax(count, FPL__MAX_THREAD_COUNT, false);
	FPL_STATICASSERT(FPL__MAX_THREAD_COUNT >= MAXIMUM_WAIT_OBJECTS);
	for(size_t index = 0; index < count; ++index) {
		fplThreadHandle *thread = threads[index];
		if(thread == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Thread for index '%d' are not allowed to be null", index);
			return false;
		}
		if(fplGetThreadState(thread) != fplThreadState_Stopped) {
			if(thread->internalHandle.win32ThreadHandle == fpl_null) {
				FPL_ERROR(FPL__MODULE_THREADING, "Thread handle for index '%d' are not allowed to be null", index);
				return false;
			}
		}
	}

	// @NOTE(final): WaitForMultipleObjects does not work for us here, because each thread will close its handle automatically
	// So we screw it and use a simple while loop and wait until either the timeout has been reached or all threads has been stopped.
	uint64_t startTime = fplGetTimeInMillisecondsLP();
	size_t minThreads = waitForAll ? count : 1;
	size_t stoppedThreads = 0;
	while(stoppedThreads < minThreads) {
		stoppedThreads = 0;
		for(size_t index = 0; index < count; ++index) {
			fplThreadHandle *thread = threads[index];
			if(fplGetThreadState(thread) == fplThreadState_Stopped) {
				++stoppedThreads;
			}
		}
		if(stoppedThreads >= minThreads) {
			break;
		}
		if(timeout != FPL_TIMEOUT_INFINITE) {
			if((fplGetTimeInMillisecondsLP() - startTime) >= timeout) {
				break;
			}
		}
		fplThreadYield();
	}
	bool result = stoppedThreads >= minThreads;
	return(result);
}

fpl_internal bool fpl__Win32SignalWaitForMultiple(fplSignalHandle *signals[], const size_t count, const bool waitForAll, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signals, false);
	FPL__CheckArgumentMax(count, FPL__MAX_SIGNAL_COUNT, false);
	HANDLE signalHandles[FPL__MAX_SIGNAL_COUNT];
	for(uint32_t index = 0; index < count; ++index) {
		fplSignalHandle *availableSignal = signals[index];
		if(availableSignal == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if(availableSignal->internalHandle.win32EventHandle == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Signal handle for index '%d' are not allowed to be null", index);
			return false;
		}
		HANDLE handle = availableSignal->internalHandle.win32EventHandle;
		signalHandles[index] = handle;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	DWORD code = WaitForMultipleObjects((DWORD)count, signalHandles, waitForAll ? TRUE : FALSE, t);
	bool result = (code >= WAIT_OBJECT_0);
	return(result);
}

fpl_internal void fpl__Win32ReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32InitState *win32InitState = &initState->win32;
	if(win32AppState->console.isAllocated) {
		FreeConsole();
		win32AppState->console.isAllocated = false;
	}
	fpl__Win32UnloadXInputApi(&win32AppState->xinput.xinputApi);
	fpl__Win32UnloadApi(&win32AppState->winApi);
}

#if defined(FPL_ENABLE_WINDOW)
fpl_internal fplKey fpl__Win32TranslateVirtualKey(const fpl__Win32Api *wapi, const uint64_t virtualKey) {
	switch(virtualKey) {
		case VK_BACK:
			return fplKey_Backspace;
		case VK_TAB:
			return fplKey_Tab;

		case VK_CLEAR:
			return fplKey_Clear;
		case VK_RETURN:
			return fplKey_Return;

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
			return fplKey_LeftSuper;
		case VK_RWIN:
			return fplKey_RightSuper;
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

		case VK_VOLUME_MUTE:
			return fplKey_VolumeMute;
		case VK_VOLUME_DOWN:
			return fplKey_VolumeDown;
		case VK_VOLUME_UP:
			return fplKey_VolumeUp;
		case VK_MEDIA_NEXT_TRACK:
			return fplKey_MediaNextTrack;
		case VK_MEDIA_PREV_TRACK:
			return fplKey_MediaPrevTrack;
		case VK_MEDIA_STOP:
			return fplKey_MediaStop;
		case VK_MEDIA_PLAY_PAUSE:
			return fplKey_MediaPlayPause;

		case VK_OEM_MINUS:
			return fplKey_OemMinus;
		case VK_OEM_PLUS:
			return fplKey_OemPlus;
		case VK_OEM_COMMA:
			return fplKey_OemComma;
		case VK_OEM_PERIOD:
			return fplKey_OemPeriod;

		case VK_OEM_1:
			return fplKey_Oem1;
		case VK_OEM_2:
			return fplKey_Oem2;
		case VK_OEM_3:
			return fplKey_Oem3;
		case VK_OEM_4:
			return fplKey_Oem4;
		case VK_OEM_5:
			return fplKey_Oem5;
		case VK_OEM_6:
			return fplKey_Oem6;
		case VK_OEM_7:
			return fplKey_Oem7;
		case VK_OEM_8:
			return fplKey_Oem8;

		default:
			return fplKey_None;
	}
}
#endif

fpl_internal bool fpl__Win32InitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	FPL_ASSERT(initState != fpl_null);
	FPL_ASSERT(appState != fpl_null);

	fpl__Win32InitState *win32InitState = &initState->win32;
	win32InitState->appInstance = GetModuleHandleA(fpl_null);
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
	if(!fpl__Win32LoadApi(&win32AppState->winApi)) {
		// @NOTE(final): Assume that errors are pushed on already.
		fpl__Win32ReleasePlatform(initState, appState);
		return false;
	}

	// Load XInput
	fpl__Win32LoadXInputApi(&win32AppState->xinput.xinputApi);

	// Init console
	if(!(initFlags & fplInitFlags_Window) && (initFlags & fplInitFlags_Console)) {
		HWND consoleHandle = GetConsoleWindow();
		if(consoleHandle == fpl_null) {
			AllocConsole();
			win32AppState->console.isAllocated = true;
		}
	}

	// Init keymap
#	if defined(FPL_ENABLE_WINDOW)
	FPL_CLEAR_STRUCT(appState->window.keyMap);
	for(int i = 0; i < 256; ++i) {
		int vk = win32AppState->winApi.user.MapVirtualKeyA(MAPVK_VSC_TO_VK, i);
		if(vk == 0) {
			vk = i;
		}
		appState->window.keyMap[i] = fpl__Win32TranslateVirtualKey(&win32AppState->winApi, vk);
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
	uint32_t result = _InterlockedExchange((volatile LONG *)target, value);
	return (result);
}
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value) {
	FPL_ASSERT(target != fpl_null);
	int32_t result = _InterlockedExchange((volatile LONG *)target, value);
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
	uint32_t result = _InterlockedExchangeAdd((volatile LONG *)value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicAddS32(volatile int32_t *value, const int32_t addend) {
	FPL_ASSERT(value != fpl_null);
	int32_t result = _InterlockedExchangeAdd((volatile LONG *)value, addend);
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

fpl_platform_api uint32_t fplAtomicIncU32(volatile uint32_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = _InterlockedIncrement((volatile LONG *)value);
	return (result);
}
fpl_platform_api int32_t fplAtomicIncS32(volatile int32_t *value) {
	FPL_ASSERT(value != fpl_null);
	int32_t result = _InterlockedIncrement((volatile LONG *)value);
	return (result);
}
fpl_platform_api uint64_t fplAtomicIncU64(volatile uint64_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint64_t result = _InterlockedIncrement64((volatile LONG64 *)value);
	return (result);
}
fpl_platform_api int64_t fplAtomicIncS64(volatile int64_t *value) {
	FPL_ASSERT(value != fpl_null);
	int64_t result = _InterlockedIncrement64((volatile LONG64 *)value);
	return(result);
}

fpl_platform_api uint32_t fplAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	uint32_t result = _InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api int32_t fplAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t result = _InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
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
	uint32_t value = _InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplIsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	FPL_ASSERT(dest != fpl_null);
	int32_t value = _InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
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
	uint32_t result = _InterlockedCompareExchange((volatile LONG *)source, 0, 0);
	return(result);
}
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source) {
	uint64_t result = _InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
	return(result);
}
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source) {
	int32_t result = _InterlockedCompareExchange((volatile LONG *)source, 0, 0);
	return(result);
}
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source) {
	int64_t result = _InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
	return(result);
}

fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
	_InterlockedExchange((volatile LONG *)dest, value);
}
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
	_InterlockedExchange64((volatile LONG64 *)dest, value);
}
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value) {
	_InterlockedExchange((volatile LONG *)dest, value);
}
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value) {
	_InterlockedExchange64((volatile LONG64 *)dest, value);
}

//
// Win32 OS
//
fpl_internal const char *fpl__Win32GetVersionName(DWORD major, DWORD minor) {
	const char *result;
	if(major == 5 && minor == 0) {
		result = "Windows 2000";
	} else if(major == 5 && minor == 1) {
		result = "Windows XP";
	} else if(major == 5 && minor == 2) {
		result = "Windows XP";
	} else if(major == 6 && minor == 0) {
		result = "Windows Vista";
	} else if(major == 6 && minor == 1) {
		result = "Windows 7";
	} else if(major == 6 && minor == 2) {
		result = "Windows 8";
	} else if(major == 6 && minor == 3) {
		result = "Windows 8.1";
	} else if(major == 10) {
		result = "Windows 10";
	} else {
		result = "Windows";
	}
	return(result);
}

#define FPL__FUNC_KERNEL32_GetVersion(name) DWORD name()
typedef FPL__FUNC_KERNEL32_GetVersion(fpl__func_kernel32_GetVersion);
#define FPL__FUNC_KERNEL32_GetVersionExA(name) BOOL WINAPI name(LPOSVERSIONINFOA lpVersionInfo)
typedef FPL__FUNC_KERNEL32_GetVersionExA(fpl__func_kernel32_GetVersionEx);
fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);

	// @NOTE(final): GetVersion() and GetVersionExA() is deprecated as of windows 8.1, so we load it manually always
	HMODULE kernelLib = LoadLibraryA("kernel32.dll");
	if(kernelLib == fpl_null) {
		FPL_ERROR(FPL__MODULE_WIN32, "Kernel32 library could not be loaded");
		return false;
	}
	fpl__func_kernel32_GetVersion *getVersionProc = (fpl__func_kernel32_GetVersion *)GetProcAddress(kernelLib, "GetVersion");
	fpl__func_kernel32_GetVersionEx *getVersionExProc = (fpl__func_kernel32_GetVersionEx *)GetProcAddress(kernelLib, "GetVersionExA");
	FreeLibrary(kernelLib);

	FPL_CLEAR_STRUCT(outInfos);

	bool result = false;
	DWORD dwVersion = 0;
	if(getVersionExProc != fpl_null) {
		OSVERSIONINFOA info = FPL_ZERO_INIT;
		info.dwOSVersionInfoSize = sizeof(info);
		if(getVersionExProc(&info) == TRUE) {
			fplS32ToString((int32_t)info.dwMajorVersion, FPL_ARRAYCOUNT(outInfos->systemVersion.major), outInfos->systemVersion.major);
			fplS32ToString((int32_t)info.dwMinorVersion, FPL_ARRAYCOUNT(outInfos->systemVersion.minor), outInfos->systemVersion.minor);
			fplS32ToString(0, FPL_ARRAYCOUNT(outInfos->systemVersion.fix), outInfos->systemVersion.fix);
			fplS32ToString((int32_t)info.dwBuildNumber, FPL_ARRAYCOUNT(outInfos->systemVersion.build), outInfos->systemVersion.build);
			const char *versionName = fpl__Win32GetVersionName(info.dwMajorVersion, info.dwMinorVersion);
			fplCopyAnsiString(versionName, outInfos->systemName, FPL_ARRAYCOUNT(outInfos->systemName));

			result = true;
		}
	} else if(getVersionProc != fpl_null) {
		dwVersion = getVersionProc();
		DWORD major = (DWORD)(LOBYTE(LOWORD(dwVersion)));
		DWORD minor = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		DWORD build = 0;
		if(dwVersion < 0x80000000) {
			build = (DWORD)((DWORD)(HIWORD(dwVersion)));
		}
		fplS32ToString((int32_t)major, FPL_ARRAYCOUNT(outInfos->systemVersion.major), outInfos->systemVersion.major);
		fplS32ToString((int32_t)minor, FPL_ARRAYCOUNT(outInfos->systemVersion.minor), outInfos->systemVersion.minor);
		fplS32ToString(0, FPL_ARRAYCOUNT(outInfos->systemVersion.fix), outInfos->systemVersion.fix);
		fplS32ToString((int32_t)build, FPL_ARRAYCOUNT(outInfos->systemVersion.build), outInfos->systemVersion.build);
		const char *versionName = fpl__Win32GetVersionName(major, minor);
		fplCopyAnsiString(versionName, outInfos->systemName, FPL_ARRAYCOUNT(outInfos->systemName));
		result = dwVersion > 0;
	}

	return(result);
}

#define FPL__FUNC_ADV32_GetUserNameA(name) BOOL WINAPI name(LPSTR lpBuffer, LPDWORD pcbBuffer)
typedef FPL__FUNC_ADV32_GetUserNameA(fpl__func_adv32_GetUserNameA);
fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, const size_t maxNameBufferLen) {
	FPL__CheckArgumentNull(nameBuffer, false);
	FPL__CheckArgumentZero(maxNameBufferLen, false);
	const char *libName = "advapi32.dll";
	HMODULE adv32Lib = LoadLibraryA(libName);
	if(adv32Lib == fpl_null) {
		FPL_ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", libName);
		return false;
	}
	fpl__func_adv32_GetUserNameA *getUserNameProc = (fpl__func_adv32_GetUserNameA *)GetProcAddress(adv32Lib, "GetUserNameA");
	bool result = false;
	if(getUserNameProc != fpl_null) {
		DWORD size = (DWORD)maxNameBufferLen;
		if(getUserNameProc(nameBuffer, &size) == TRUE) {
			result = true;
		}
	}
	if(adv32Lib != fpl_null) {
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
	switch(sysInfo.wProcessorArchitecture) {
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

#define FPL__FUNC_WIN32_KERNEL32_GetPhysicallyInstalledSystemMemory(name) BOOL WINAPI name(PULONGLONG TotalMemoryInKilobytes)
typedef FPL__FUNC_WIN32_KERNEL32_GetPhysicallyInstalledSystemMemory(fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory);
fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);
	bool result = false;

	HMODULE kernel32lib = LoadLibraryA("kernel32.dll");
	if(kernel32lib == fpl_null) {
		return false;
	}
	fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory *getPhysicallyInstalledSystemMemory = (fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory *)GetProcAddress(kernel32lib, "GetPhysicallyInstalledSystemMemory");
	FreeLibrary(kernel32lib);

	ULONGLONG installedMemorySize = 0;
	if(getPhysicallyInstalledSystemMemory != fpl_null) {
		getPhysicallyInstalledSystemMemory(&installedMemorySize);
	}

	SYSTEM_INFO systemInfo = FPL_ZERO_INIT;
	GetSystemInfo(&systemInfo);

	MEMORYSTATUSEX statex = FPL_ZERO_INIT;
	statex.dwLength = sizeof(statex);

	if(GlobalMemoryStatusEx(&statex)) {
		FPL_CLEAR_STRUCT(outInfos);
		outInfos->installedPhysicalSize = installedMemorySize * 1024ull;
		outInfos->totalPhysicalSize = statex.ullTotalPhys;
		outInfos->freePhysicalSize = statex.ullAvailPhys;
		outInfos->totalCacheSize = statex.ullTotalVirtual;
		outInfos->freeCacheSize = statex.ullAvailVirtual;
		outInfos->pageSize = systemInfo.dwPageSize;
		if(outInfos->pageSize > 0) {
			outInfos->totalPageCount = statex.ullTotalPageFile / outInfos->pageSize;
			outInfos->freePageCount = statex.ullAvailPageFile / outInfos->pageSize;
		}
		result = true;
	}
	return(result);
}

fpl_platform_api char *fplGetProcessorName(char *destBuffer, const size_t maxDestBufferLen) {
#	define CPU_BRAND_BUFFER_SIZE 0x40
	FPL__CheckArgumentNull(destBuffer, fpl_null);
	size_t requiredDestBufferLen = CPU_BRAND_BUFFER_SIZE + 1;
	FPL__CheckArgumentMin(maxDestBufferLen, requiredDestBufferLen, fpl_null);

	// @TODO(final): __cpuid may not be available on other Win32 Compilers!
	int cpuInfo[4] = { -1 };
	char cpuBrandBuffer[CPU_BRAND_BUFFER_SIZE] = FPL_ZERO_INIT;
	__cpuid(cpuInfo, 0x80000000);
	uint32_t extendedIds = cpuInfo[0];
	// Get the information associated with each extended ID. Interpret CPU brand string.
	uint32_t max = FPL_MIN(extendedIds, 0x80000004);
	for(uint32_t i = 0x80000002; i <= max; ++i) {
		__cpuid(cpuInfo, i);
		uint32_t offset = (i - 0x80000002) << 4;
		fplMemoryCopy(cpuInfo, sizeof(cpuInfo), cpuBrandBuffer + offset);
	}
	// Copy result back to the dest buffer
	size_t sourceLen = fplGetAnsiStringLength(cpuBrandBuffer);
	char *result = fplCopyAnsiStringLen(cpuBrandBuffer, sourceLen, destBuffer, maxDestBufferLen);

#	undef CPU_BRAND_BUFFER_SIZE

	return(result);
}

//
// Win32 Threading
//
fpl_internal DWORD WINAPI fpl__Win32ThreadProc(void *data) {
	fplThreadHandle *thread = (fplThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);
	if(thread->runFunc != fpl_null) {
		thread->runFunc(thread, thread->data);
	}
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	if(handle != fpl_null) {
		CloseHandle(handle);
	}
	thread->isValid = false;
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	ExitThread(0);
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data) {
	FPL__CheckArgumentNull(runFunc, fpl_null);
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if(thread != fpl_null) {
		DWORD creationFlags = 0;
		DWORD threadId = 0;
		thread->data = data;
		thread->runFunc = runFunc;
		thread->currentState = fplThreadState_Starting;
		HANDLE handle = CreateThread(fpl_null, 0, fpl__Win32ThreadProc, thread, creationFlags, &threadId);
		if(handle != fpl_null) {
			thread->isValid = true;
			thread->id = threadId;
			thread->internalHandle.win32ThreadHandle = handle;
			result = thread;
		} else {
			FPL_ERROR(FPL__MODULE_THREADING, "Failed creating thread, error code: %d", GetLastError());
		}
	} else {
		FPL_ERROR(FPL__MODULE_THREADING, "All %d threads are in use, you cannot create until you free one", FPL__MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api void fplThreadSleep(const uint32_t milliseconds) {
	Sleep((DWORD)milliseconds);
}

fpl_platform_api bool fplThreadYield() {
	bool result = SwitchToThread() == TRUE;
	return(result);
}

fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread) {
	FPL__CheckArgumentNull(thread, false);
	fplThreadState state = fplGetThreadState(thread);
	if(thread->isValid && (state != fplThreadState_Stopped && state != fplThreadState_Stopping)) {
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
		HANDLE handle = thread->internalHandle.win32ThreadHandle;
		if(handle != fpl_null) {
			TerminateThread(handle, 0);
			CloseHandle(handle);
		}
		thread->isValid = false;
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
		return true;
	} else {
		return false;
	}
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(thread, false);
	bool result;
	if(fplGetThreadState(thread) != fplThreadState_Stopped) {
		if(thread->internalHandle.win32ThreadHandle == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Win32 thread handle are not allowed to be null");
			return false;
		}
		HANDLE handle = thread->internalHandle.win32ThreadHandle;
		DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
		result = (WaitForSingleObject(handle, t) == WAIT_OBJECT_0);
	} else {
		result = true;
	}
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
	FPL__CheckArgumentNull(mutex, false);
	if(mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex '%p' is already initialized", mutex);
		return false;
	}
	FPL_CLEAR_STRUCT(mutex);
	InitializeCriticalSection(&mutex->internalHandle.win32CriticalSection);
	mutex->isValid = true;
	return true;
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	FPL__CheckArgumentNullNoRet(mutex);
	if(mutex->isValid) {
		DeleteCriticalSection(&mutex->internalHandle.win32CriticalSection);
		FPL_CLEAR_STRUCT(mutex);
	}
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if(!mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	EnterCriticalSection(&mutex->internalHandle.win32CriticalSection);
	return true;
}

fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if(!mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	bool result = TryEnterCriticalSection(&mutex->internalHandle.win32CriticalSection) == TRUE;
	return(result);
}

fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if(!mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	LeaveCriticalSection(&mutex->internalHandle.win32CriticalSection);
	return true;
}

fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	FPL__CheckArgumentNull(signal, false);
	if(signal->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal '%p' is already initialized", signal);
		return false;
	}
	HANDLE handle = CreateEventA(fpl_null, FALSE, (initialValue == fplSignalValue_Set) ? TRUE : FALSE, fpl_null);
	if(handle == fpl_null) {
		FPL_ERROR(FPL__MODULE_THREADING, "Failed creating signal (Win32 event): %d", GetLastError());
		return false;
	}
	FPL_CLEAR_STRUCT(signal);
	signal->isValid = true;
	signal->internalHandle.win32EventHandle = handle;
	return(true);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	FPL__CheckArgumentNullNoRet(signal);
	if(signal->internalHandle.win32EventHandle != fpl_null) {
		HANDLE handle = signal->internalHandle.win32EventHandle;
		CloseHandle(handle);
		FPL_CLEAR_STRUCT(signal);
	}
}

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signal, false);
	if(signal->internalHandle.win32EventHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
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
	FPL__CheckArgumentNull(signal, false);
	if(signal->internalHandle.win32EventHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = SetEvent(handle) == TRUE;
	return(result);
}

fpl_platform_api bool fplSignalReset(fplSignalHandle *signal) {
	FPL__CheckArgumentNull(signal, false);
	if(signal->internalHandle.win32EventHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = ResetEvent(handle) == TRUE;
	return(result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	FPL_CLEAR_STRUCT(condition);
	InitializeConditionVariable(&condition->internalHandle.win32Condition);
	condition->isValid = true;
	return true;
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	FPL__CheckArgumentNullNoRet(condition);
	if(condition->isValid) {
		FPL_CLEAR_STRUCT(condition);
	}
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckArgumentNull(mutex, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	if(!mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex '%p' is not valid", mutex);
		return false;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = SleepConditionVariableCS(&condition->internalHandle.win32Condition, &mutex->internalHandle.win32CriticalSection, t) != 0;
	return(result);
}

fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	WakeConditionVariable(&condition->internalHandle.win32Condition);
	return true;
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	WakeAllConditionVariable(&condition->internalHandle.win32Condition);
	return true;
}

fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue) {
	FPL__CheckArgumentNull(semaphore, false);
	FPL__CheckArgumentMax(initialValue, INT32_MAX, false);
	if(semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is already initialized", semaphore);
		return false;
	}
	HANDLE handle = CreateSemaphoreA(fpl_null, (LONG)initialValue, INT32_MAX, fpl_null);
	if(handle == fpl_null) {
		FPL_ERROR(FPL__MODULE_THREADING, "Failed creating semaphore");
		return false;
	}
	FPL_CLEAR_STRUCT(semaphore);
	semaphore->isValid = true;
	semaphore->internalHandle.win32.handle = handle;
	semaphore->internalHandle.win32.value = (int32_t)initialValue;
	return true;
}

fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNullNoRet(semaphore);
	if(semaphore->isValid) {
		CloseHandle(semaphore->internalHandle.win32.handle);
		FPL_CLEAR_STRUCT(semaphore);
	}
}

fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = false;
	if(WaitForSingleObject(semaphore->internalHandle.win32.handle, timeout) == WAIT_OBJECT_0) {
		fplAtomicAddS32(&semaphore->internalHandle.win32.value, -1);
		result = true;
	}
	return(result);
}

fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	bool result = false;
	if(WaitForSingleObject(semaphore->internalHandle.win32.handle, 0) == WAIT_OBJECT_0) {
		fplAtomicAddS32(&semaphore->internalHandle.win32.value, -1);
		result = true;
	}
	return(result);
}

fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	int32_t result = fplAtomicLoadS32(&semaphore->internalHandle.win32.value);
	return(result);
}

fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	bool result = true;
	int32_t prevValue = fplAtomicAddS32(&semaphore->internalHandle.win32.value, 1);
	if(ReleaseSemaphore(semaphore->internalHandle.win32.handle, 1, fpl_null) == FALSE) {
		// Restore value when it fails
		FPL_ERROR(FPL__MODULE_THREADING, "Failed releasing the semaphore '%p'", semaphore);
		fplAtomicStoreS32(&semaphore->internalHandle.win32.value, prevValue);
		result = false;
	}
	return(result);
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
	if(WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0) {
		DWORD charsRead = 0;
		char inputBuffer[2] = FPL_ZERO_INIT;
		if(ReadFile(handle, inputBuffer, 1, &charsRead, fpl_null) != 0) {
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
	FPL__CheckArgumentZero(size, fpl_null);
	void *result = VirtualAlloc(fpl_null, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(result == fpl_null) {
		FPL_ERROR(FPL__MODULE_MEMORY, "Failed allocating memory of %xu bytes", size);
	}
	return(result);
}

fpl_platform_api void fplMemoryFree(void *ptr) {
	FPL__CheckArgumentNullNoRet(ptr);
	VirtualFree(ptr, 0, MEM_RELEASE);
}

//
// Win32 Files
//
fpl_internal uint64_t fpl__Win32ConvertFileTimeToUnixTimestamp(const FILETIME *fileTime) {
	// Ticks are defined in 100 ns = 10000000 secs
	// Windows ticks starts at 1601-01-01T00:00:00Z
	// Unix secs starts at 1970-01-01T00:00:00Z
	const uint64_t UNIX_TIME_START = 0x019DB1DED53E8000;
	const uint64_t TICKS_PER_SECOND = 10000000;
	ULARGE_INTEGER largeInteger;
	largeInteger.LowPart = fileTime->dwLowDateTime;
	largeInteger.HighPart = fileTime->dwHighDateTime;
	uint64_t result = (largeInteger.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;
	return(result);
}

fpl_platform_api bool fplOpenAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplOpenWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api bool fplCreateAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, fpl_null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplCreateWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, fpl_null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api uint32_t fplReadFileBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if(fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for reading");
		return 0;
	}
	uint32_t result = 0;
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	DWORD bytesRead = 0;
	if(ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, fpl_null) == TRUE) {
		result = bytesRead;
	}
	return(result);
}

fpl_platform_api uint64_t fplReadFileBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if(fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for reading");
		return 0;
	}
	// @NOTE(final): There is no ReadFile64 function in win32, so we have to read it in chunks
	uint64_t result = 0;
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint64_t remainingSize = sizeToRead;
	uint64_t bufferPos = 0;
	const uint64_t MaxDWORD = (uint64_t)(DWORD)-1;
	while(remainingSize >= MaxDWORD) {
		DWORD bytesRead = 0;
		uint8_t *target = (uint8_t *)targetBuffer + bufferPos;
		uint64_t size = FPL_MIN(remainingSize, MaxDWORD);
		FPL_ASSERT(size <= MaxDWORD);
		if(ReadFile(win32FileHandle, target, (DWORD)size, &bytesRead, fpl_null) == TRUE) {
			result = bytesRead;
		} else {
			break;
		}
		remainingSize -= bytesRead;
		bufferPos += bytesRead;
	}
	return(result);
}

fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if(fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
		return 0;
	}
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint32_t result = 0;
	DWORD bytesWritten = 0;
	if(WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, fpl_null) == TRUE) {
		result = bytesWritten;
	}
	return(result);
}

fpl_platform_api uint64_t fplWriteFileBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if(fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
		return 0;
	}
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint64_t result = 0;
	uint64_t bufferPos = 0;
	uint64_t remainingSize = sourceSize;
	const uint64_t MaxDWORD = (uint64_t)(DWORD)-1;
	while(remainingSize >= MaxDWORD) {
		uint8_t *source = (uint8_t *)sourceBuffer + bufferPos;
		uint64_t size = FPL_MIN(remainingSize, MaxDWORD);
		FPL_ASSERT(size <= MaxDWORD);
		DWORD bytesWritten = 0;
		if(WriteFile(win32FileHandle, source, (DWORD)size, &bytesWritten, fpl_null) == TRUE) {
			result = bytesWritten;
		} else {
			break;
		}
		remainingSize -= bytesWritten;
		bufferPos += bytesWritten;
	}
	return(result);
}

fpl_platform_api uint32_t fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD moveMethod = FILE_BEGIN;
		if(mode == fplFilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if(mode == fplFilePositionMode_End) {
			moveMethod = FILE_END;
		}
		DWORD r = 0;
		r = SetFilePointer(win32FileHandle, (LONG)position, fpl_null, moveMethod);
		result = (uint32_t)r;
	}
	return(result);
}

fpl_platform_api uint64_t fplSetFilePosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD moveMethod = FILE_BEGIN;
		if(mode == fplFilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if(mode == fplFilePositionMode_End) {
			moveMethod = FILE_END;
		}
		LARGE_INTEGER r = FPL_ZERO_INIT;
		LARGE_INTEGER li;
		li.QuadPart = position;
		if(SetFilePointerEx(win32FileHandle, li, &r, moveMethod) == TRUE) {
			result = (uint64_t)r.QuadPart;
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD filePosition = SetFilePointer(win32FileHandle, 0L, fpl_null, FILE_CURRENT);
		if(filePosition != INVALID_SET_FILE_POINTER) {
			return filePosition;
		}
	}
	return 0;
}

fpl_platform_api uint64_t fplGetFilePosition64(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		LARGE_INTEGER r = FPL_ZERO_INIT;
		LARGE_INTEGER li;
		li.QuadPart = 0;
		if(SetFilePointerEx(win32FileHandle, li, &r, FILE_CURRENT) == TRUE) {
			result = (uint64_t)r.QuadPart;
		}
	}
	return 0;
}

fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle) {
	if((fileHandle != fpl_null) && (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE)) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		CloseHandle(win32FileHandle);
		FPL_CLEAR_STRUCT(fileHandle);
	}
}

fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath) {
	uint32_t result = 0;
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
			result = (uint32_t)fileSize;
			CloseHandle(win32FileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplGetFileSizeFromPath64(const char *filePath) {
	uint64_t result = 0;
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER li = FPL_ZERO_INIT;
			if(GetFileSizeEx(win32FileHandle, &li) == TRUE) {
				result = (uint64_t)li.QuadPart;
			}
			CloseHandle(win32FileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
		result = (uint32_t)fileSize;
	}
	return(result);
}

fpl_platform_api uint64_t fplGetFileSizeFromHandle64(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		LARGE_INTEGER li = FPL_ZERO_INIT;
		if(GetFileSizeEx(win32FileHandle, &li) == TRUE) {
			result = (uint64_t)li.QuadPart;
		}
	}
	return(result);
}

fpl_platform_api bool fplGetFileTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(outStamps, false);
	if(filePath != fpl_null) {
		HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		bool result = false;
		if(win32FileHandle != INVALID_HANDLE_VALUE) {
			FILETIME times[3];
			if(GetFileTime(win32FileHandle, &times[0], &times[1], &times[2]) == TRUE) {
				FPL_CLEAR_STRUCT(outStamps);
				outStamps->creationTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[0]);
				outStamps->lastAccessTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[1]);
				outStamps->lastModifyTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[2]);
				result = true;
			}
			CloseHandle(win32FileHandle);
		}
		return(result);
	}
	return(false);
}

fpl_platform_api bool fplGetFileTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentNull(outStamps, 0);
	if(fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		FILETIME times[3];
		if(GetFileTime(win32FileHandle, &times[0], &times[1], &times[2]) == TRUE) {
			FPL_CLEAR_STRUCT(outStamps);
			outStamps->creationTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[0]);
			outStamps->lastAccessTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[1]);
			outStamps->lastModifyTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[2]);
			return(true);
		}
	}
	return(false);
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if(filePath != fpl_null) {
		WIN32_FIND_DATAA findData;
		HANDLE searchHandle = FindFirstFileA(filePath, &findData);
		if(searchHandle != INVALID_HANDLE_VALUE) {
			result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	bool result = (CopyFileA(sourceFilePath, targetFilePath, !overwrite) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	bool result = (MoveFileA(sourceFilePath, targetFilePath) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileDelete(const char *filePath) {
	FPL__CheckArgumentNull(filePath, false);
	bool result = (DeleteFileA(filePath) == TRUE);
	return(result);
}

fpl_platform_api bool fplDirectoryExists(const char *path) {
	bool result = false;
	if(path != fpl_null) {
		WIN32_FIND_DATAA findData;
		HANDLE searchHandle = FindFirstFileA(path, &findData);
		if(searchHandle != INVALID_HANDLE_VALUE) {
			result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplDirectoriesCreate(const char *path) {
	FPL__CheckArgumentNull(path, false);
	bool result = CreateDirectoryA(path, fpl_null) > 0;
	return(result);
}
fpl_platform_api bool fplDirectoryRemove(const char *path) {
	FPL__CheckArgumentNull(path, false);
	bool result = RemoveDirectoryA(path) > 0;
	return(result);
}
fpl_internal_inline void fpl__Win32FillFileEntry(const char *rootPath, const WIN32_FIND_DATAA *findData, fplFileEntry *entry) {
	FPL_ASSERT(findData != fpl_null);
	FPL_ASSERT(entry != fpl_null);
	fplCopyAnsiString(rootPath, entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	fplEnforcePathSeparatorLen(entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	fplStringAppend(findData->cFileName, entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	entry->type = fplFileEntryType_Unknown;
	if(findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		entry->type = fplFileEntryType_Directory;
	} else if(
		(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
		(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
		entry->type = fplFileEntryType_File;
	}

	// @TODO(final): Win32 Read ACL for full permission detection!
	entry->attributes = fplFileAttributeFlags_None;
	entry->permissions.umask = 0;
	if(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		entry->attributes = fplFileAttributeFlags_Normal;
	} else {
		if(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			entry->attributes |= fplFileAttributeFlags_Hidden;
		}
		if(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			entry->attributes |= fplFileAttributeFlags_Archive;
		}
		if(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			entry->attributes |= fplFileAttributeFlags_System;
		}
		entry->permissions.user |= fplFilePermissionFlags_CanWrite;
		entry->permissions.user |= fplFilePermissionFlags_CanRead;
		entry->permissions.user |= fplFilePermissionFlags_CanExecuteSearch;
		if((findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) || (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
			entry->permissions.user &= ~fplFilePermissionFlags_CanWrite;
		}
	}
	if(entry->type == fplFileEntryType_File) {
		ULARGE_INTEGER ul;
		ul.LowPart = findData->nFileSizeLow;
		ul.HighPart = findData->nFileSizeHigh;
		entry->size = (size_t)ul.QuadPart;
	} else {
		entry->size = 0;
	}
	entry->timeStamps.creationTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&findData->ftCreationTime);
	entry->timeStamps.lastAccessTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&findData->ftLastAccessTime);
	entry->timeStamps.lastModifyTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&findData->ftLastWriteTime);
}
fpl_platform_api bool fplListDirBegin(const char *path, const char *filter, fplFileEntry *entry) {
	FPL__CheckArgumentNull(path, false);
	FPL__CheckArgumentNull(entry, false);
	if(fplGetAnsiStringLength(filter) == 0) {
		filter = "*";
	}
	WIN32_FIND_DATAA findData;
	char pathAndFilter[MAX_PATH + 1] = FPL_ZERO_INIT;
	fplCopyAnsiString(path, pathAndFilter, FPL_ARRAYCOUNT(pathAndFilter));
	fplEnforcePathSeparatorLen(pathAndFilter, FPL_ARRAYCOUNT(pathAndFilter));
	fplStringAppend(filter, pathAndFilter, FPL_ARRAYCOUNT(pathAndFilter));
	HANDLE searchHandle = FindFirstFileA(pathAndFilter, &findData);
	bool result = false;
	if(searchHandle != INVALID_HANDLE_VALUE) {
		FPL_CLEAR_STRUCT(entry);
		entry->internalHandle.win32FileHandle = searchHandle;
		entry->internalRoot.rootPath = path;
		entry->internalRoot.filter = filter;

		bool foundNext = true;
		while(foundNext) {
			if(fplIsStringEqual(".", findData.cFileName) || fplIsStringEqual("..", findData.cFileName)) {
				foundNext = FindNextFileA(searchHandle, &findData) == TRUE;
			} else {
				result = true;
				fpl__Win32FillFileEntry(path, &findData, entry);
				break;
			}
		}
	}
	return(result);
}
fpl_platform_api bool fplListDirNext(fplFileEntry *entry) {
	FPL__CheckArgumentNull(entry, false);
	bool result = false;
	if(entry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = entry->internalHandle.win32FileHandle;
		WIN32_FIND_DATAA findData;
		bool foundNext;
		do {
			foundNext = FindNextFileA(searchHandle, &findData) == TRUE;
			if(foundNext) {
				if(fplIsStringEqual(".", findData.cFileName) || fplIsStringEqual("..", findData.cFileName)) {
					continue;
				} else {
					fpl__Win32FillFileEntry(entry->internalRoot.rootPath, &findData, entry);
					result = true;
					break;
				}
			}
		} while(foundNext);
	}
	return(result);
}
fpl_platform_api void fplListDirEnd(fplFileEntry *entry) {
	FPL__CheckArgumentNullNoRet(entry);
	if(entry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = entry->internalHandle.win32FileHandle;
		FindClose(searchHandle);
		FPL_CLEAR_STRUCT(entry);
	}
}

//
// Win32 Path/Directories
//
#if defined(UNICODE)
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredMaxDestLen = MAX_PATH + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredMaxDestLen, fpl_null);
	wchar_t modulePath[MAX_PATH];
	GetModuleFileNameW(fpl_null, modulePath, MAX_PATH);
	fplWideStringToAnsiString(modulePath, fplGetWideStringLength(modulePath), destPath, maxDestLen);
	return(destPath);
}
#else
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredMaxDestLen = MAX_PATH + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredMaxDestLen, fpl_null);
	char modulePath[MAX_PATH];
	GetModuleFileNameA(fpl_null, modulePath, MAX_PATH);
	fplCopyAnsiStringLen(modulePath, fplGetAnsiStringLength(modulePath), destPath, maxDestLen);
	return(destPath);
}
#endif // UNICODE

#if defined(UNICODE)
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredMaxDestLen = MAX_PATH + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredMaxDestLen, fpl_null);
	FPL__CheckPlatform(fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	wchar_t homePath[MAX_PATH];
	wapi->shell.ShGetFolderPathW(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	fplWideStringToAnsiString(homePath, fplGetWideStringLength(homePath), destPath, maxDestLen);
	return(destPath);
}
#else
fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	size_t requiredMaxDestLen = MAX_PATH + 1;
	FPL__CheckArgumentMin(maxDestLen, requiredMaxDestLen, fpl_null);
	FPL__CheckPlatform(fpl_null);
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
	double result = (time.QuadPart / (double)initState->performanceFrequency.QuadPart) * 1000.0;
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
	FPL__CheckArgumentNull(wideSource, fpl_null);
	FPL__CheckArgumentNull(ansiDest, fpl_null);
	int requiredLen = WideCharToMultiByte(CP_ACP, 0, wideSource, (int)maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxAnsiDestLen, minRequiredLen, fpl_null);
	WideCharToMultiByte(CP_ACP, 0, wideSource, (int)maxWideSourceLen, ansiDest, (int)maxAnsiDestLen, fpl_null, fpl_null);
	ansiDest[requiredLen] = 0;

	return(&ansiDest[requiredLen]);
}
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	FPL__CheckArgumentNull(wideSource, fpl_null);
	FPL__CheckArgumentNull(utf8Dest, fpl_null);
	int requiredLen = WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)maxWideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxUtf8DestLen, minRequiredLen, fpl_null);
	WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)maxWideSourceLen, utf8Dest, (int)maxUtf8DestLen, fpl_null, fpl_null);
	utf8Dest[requiredLen] = 0;

	return(&utf8Dest[requiredLen]);
}
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	FPL__CheckArgumentNull(ansiSource, fpl_null);
	FPL__CheckArgumentNull(wideDest, fpl_null);
	int requiredLen = MultiByteToWideChar(CP_ACP, 0, ansiSource, (int)ansiSourceLen, fpl_null, 0);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxWideDestLen, minRequiredLen, fpl_null);
	MultiByteToWideChar(CP_ACP, 0, ansiSource, (int)ansiSourceLen, wideDest, (int)maxWideDestLen);
	wideDest[requiredLen] = 0;

	return(&wideDest[requiredLen]);
}
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	FPL__CheckArgumentNull(utf8Source, fpl_null);
	FPL__CheckArgumentNull(wideDest, fpl_null);
	int requiredLen = MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, fpl_null, 0);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxWideDestLen, minRequiredLen, fpl_null);
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, wideDest, (int)maxWideDestLen);
	wideDest[requiredLen] = 0;

	return(&wideDest[requiredLen]);
}

//
// Win32 Library
//
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle) {
	bool result = false;
	if(libraryFilePath != fpl_null && outHandle != fpl_null) {
		HMODULE libModule = LoadLibraryA(libraryFilePath);
		if(libModule != fpl_null) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->internalHandle.win32LibraryHandle = libModule;
			outHandle->isValid = true;
			result = true;
		}
	}
	return(result);
}
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name) {
	if((handle != fpl_null) && (handle->internalHandle.win32LibraryHandle != fpl_null) && (name != fpl_null)) {
		HMODULE libModule = handle->internalHandle.win32LibraryHandle;
		return (void *)GetProcAddress(libModule, name);
	}
	return fpl_null;
}
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if((handle != fpl_null) && (handle->internalHandle.win32LibraryHandle != fpl_null)) {
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
	FPL__CheckArgumentNull(outSize, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	RECT windowRect;
	if(wapi->user.GetClientRect(windowState->windowHandle, &windowRect)) {
		outSize->width = windowRect.right - windowRect.left;
		outSize->height = windowRect.bottom - windowRect.top;
		result = true;
	}
	return(result);
}

fpl_platform_api void fplSetWindowArea(const uint32_t width, const uint32_t height) {
	FPL__CheckPlatformNoRet();
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	RECT clientRect, windowRect;
	if(wapi->user.GetClientRect(windowState->windowHandle, &clientRect) &&
	   wapi->user.GetWindowRect(windowState->windowHandle, &windowRect)) {
		int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
		int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
		int newWidth = width + borderWidth;
		int newHeight = height + borderHeight;
		wapi->user.SetWindowPos(windowState->windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

fpl_platform_api bool fplIsWindowResizable() {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isResizable != 0;
	return(result);
}

fpl_platform_api void fplSetWindowResizeable(const bool value) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if(!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isResizable = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowDecorated() {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isDecorated != 0;
	return(result);
}

fpl_platform_api void fplSetWindowDecorated(const bool value) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if(!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isDecorated = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowFloating() {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFloating != 0;
	return(result);
}

fpl_platform_api void fplSetWindowFloating(const bool value) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32WindowState *windowState = &appState->window.win32;
	if(!appState->currentSettings.window.isFullscreen) {
		appState->currentSettings.window.isFloating = value;
		fpl__Win32UpdateWindowStyles(&appState->currentSettings.window, windowState);
	}
}

fpl_platform_api bool fplIsWindowFullscreen() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFullscreen != 0;
	return(result);
}

fpl_platform_api bool fplSetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32WindowState *windowState = &appState->window.win32;
	fplWindowSettings *windowSettings = &appState->currentSettings.window;
	fpl__Win32LastWindowInfo *fullscreenState = &windowState->lastFullscreenInfo;
	const fpl__Win32Api *wapi = &win32AppState->winApi;

	HWND windowHandle = windowState->windowHandle;
	// Save current window info if not already fullscreen
	if(!windowSettings->isFullscreen) {
		fpl__Win32SaveWindowState(wapi, fullscreenState, windowHandle);
		if(fullscreenState->isMaximized || fullscreenState->isMinimized) {
			wapi->user.ShowWindow(windowHandle, SW_RESTORE);
		}
	}

	if(value) {
		// Enter fullscreen mode or fallback to window mode
		windowSettings->isFullscreen = fpl__Win32EnterFullscreen(fullscreenWidth, fullscreenHeight, refreshRate, 0);
		if(!windowSettings->isFullscreen) {
			fpl__Win32LeaveFullscreen();
		}
	} else {
		fpl__Win32LeaveFullscreen();
		windowSettings->isFullscreen = false;
	}
	bool result = windowSettings->isFullscreen != 0;
	return(result);
}

fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos) {
	FPL__CheckArgumentNull(outPos, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	if(wapi->user.GetWindowPlacement(windowState->windowHandle, &placement) == TRUE) {
		switch(placement.showCmd) {
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
	FPL__CheckPlatformNoRet();
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HWND handle = windowState->windowHandle;
	wapi->user.SetWindowTextA(handle, ansiTitle);
}

fpl_platform_api void fplSetWindowWideTitle(const wchar_t *wideTitle) {
	FPL__CheckPlatformNoRet();
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HWND handle = windowState->windowHandle;
	wapi->user.SetWindowTextW(handle, wideTitle);
}

fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top) {
	FPL__CheckPlatformNoRet();
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	WINDOWPLACEMENT placement = FPL_ZERO_INIT;
	placement.length = sizeof(WINDOWPLACEMENT);
	RECT windowRect;
	if(wapi->user.GetWindowPlacement(windowState->windowHandle, &placement) &&
	   wapi->user.GetWindowRect(windowState->windowHandle, &windowRect)) {
		switch(placement.showCmd) {
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
	FPL__CheckPlatformNoRet();
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	windowState->isCursorActive = value;
}

fpl_platform_api bool fplPushEvent() {
	FPL__CheckPlatform(false);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	bool result = false;
	MSG msg;
	BOOL R = fpl__win32_PeekMessage(&msg, fpl_null, 0, 0, PM_REMOVE);
	if(R == TRUE) {
		wapi->user.TranslateMessage(&msg);
		fpl__win32_DispatchMessage(&msg);
		result = true;
	}
	return (result);
}

fpl_platform_api void fplUpdateGameControllers() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	fpl__Win32PollControllers(&appState->currentSettings, win32InitState, &win32AppState->xinput);
}

fpl_platform_api bool fplWindowUpdate() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	bool result = false;
	fplClearEvents();
	fpl__Win32PollControllers(&appState->currentSettings, win32InitState, &win32AppState->xinput);
	if(windowState->windowHandle != 0) {
		MSG msg;
		while(fpl__win32_PeekMessage(&msg, fpl_null, 0, 0, PM_REMOVE) != 0) {
			wapi->user.TranslateMessage(&msg);
			fpl__win32_DispatchMessage(&msg);
		}
		result = appState->window.isRunning != 0;
	}
	return(result);
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL__CheckPlatform(false);
	bool result = fpl__global__AppState->window.isRunning != 0;
	return(result);
}

fpl_platform_api void fplWindowShutdown() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32AppState *win32AppState = &appState->win32;
	if(appState->window.isRunning) {
		appState->window.isRunning = false;
		const fpl__Win32Api *wapi = &win32AppState->winApi;
		wapi->user.PostQuitMessage(0);
	}
}

fpl_platform_api bool fplGetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if(wapi->user.OpenClipboard(windowState->windowHandle)) {
		if(wapi->user.IsClipboardFormatAvailable(CF_TEXT)) {
			HGLOBAL dataHandle = wapi->user.GetClipboardData(CF_TEXT);
			if(dataHandle != fpl_null) {
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
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if(wapi->user.OpenClipboard(windowState->windowHandle)) {
		if(wapi->user.IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			HGLOBAL dataHandle = wapi->user.GetClipboardData(CF_UNICODETEXT);
			if(dataHandle != fpl_null) {
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
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if(wapi->user.OpenClipboard(windowState->windowHandle)) {
		const size_t ansiLen = fplGetAnsiStringLength(ansiSource);
		const size_t ansiBufferLen = ansiLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
		if(handle != fpl_null) {
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
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if(wapi->user.OpenClipboard(windowState->windowHandle)) {
		const size_t wideLen = fplGetWideStringLength(wideSource);
		const size_t wideBufferLen = wideLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
		if(handle != fpl_null) {
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

fpl_platform_api bool fplGetKeyboardState(fplKeyboardState *outState) {
	FPL__CheckArgumentNull(outState, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	FPL_CLEAR_STRUCT(outState);
	outState->modifiers = fpl__Win32GetKeyboardModifiers(wapi);
	for(uint32_t keyCode = 0; keyCode < 256; ++keyCode) {
		int k = wapi->user.MapVirtualKeyA(MAPVK_VSC_TO_VK, keyCode);
		if(k == 0) {
			k = (int)keyCode;
		}
		bool down = fpl__Win32IsKeyDown(wapi, k);
		fplKey key = fpl__GetMappedKey(&fpl__global__AppState->window, keyCode);
		outState->keyStatesRaw[keyCode] = down;
		outState->buttonStatesMapped[(int)key] = down ? fplButtonState_Press : fplButtonState_Release;
	}
	return(true);
}

fpl_platform_api bool fplGetGamepadStates(fplGamepadStates *outStates) {
	FPL__CheckArgumentNull(outStates, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	const fpl__Win32XInputState *xinputState = &appState->xinput;
	FPL_ASSERT(xinputState != fpl_null);
	FPL_CLEAR_STRUCT(outStates);
	if(xinputState->xinputApi.xInputGetState != fpl_null) {
		for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
			XINPUT_STATE controllerState = FPL_ZERO_INIT;
			if(xinputState->xinputApi.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
				XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
				fplGamepadState *gamepadState = &outStates->deviceStates[controllerIndex];
				fpl__Win32XInputGamepadToGamepadState(pad, gamepadState);
			}
		}
		return(true);
	}
	return(false);
}

#endif // FPL_ENABLE_WINDOW

fpl_internal LCTYPE fpl__Win32GetLocaleLCIDFromFormat(const fplLocaleFormat format) {
	switch(format) {
		case fplLocaleFormat_ISO639:
			return LOCALE_SNAME;
		default:
			return LOCALE_SABBREVLANGNAME;
	}
}

fpl_platform_api bool fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	int r = GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, lcType, buffer, (int)maxBufferLen);
	bool result = r > 0;
	return(result);
}

fpl_platform_api bool fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	int r = GetLocaleInfoA(LOCALE_USER_DEFAULT, lcType, buffer, (int)maxBufferLen);
	bool result = r > 0;
	return(result);
}

fpl_platform_api bool fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HKL kbLayout = wapi->user.GetKeyboardLayout(GetCurrentThreadId());
	LCID langId = (DWORD)(intptr_t)kbLayout & 0xFFFF;
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	int r = GetLocaleInfoA(langId, lcType, buffer, (int)maxBufferLen);
	bool result = r > 0;
	return(result);
}

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
	if(!fpl__PThreadLoadApi(&appState->pthreadApi)) {
		FPL_ERROR(FPL__MODULE_POSIX, "Failed initializing PThread API");
		return false;
	}
	return true;
}

fpl_internal void fpl__InitWaitTimeSpec(const uint32_t milliseconds, struct timespec *outSpec) {
	time_t secs = milliseconds / 1000;
	uint64_t nanoSecs = (milliseconds - (secs * 1000)) * 1000000;
	if(nanoSecs >= 1000000000) {
		time_t addonSecs = (time_t)(nanoSecs / 1000000000);
		nanoSecs -= (addonSecs * 1000000000);
		secs += addonSecs;
	}
	clock_gettime(CLOCK_REALTIME, outSpec);
	outSpec->tv_sec += secs;
	outSpec->tv_nsec += nanoSecs;
}

void *fpl__PosixThreadProc(void *data) {
	FPL_ASSERT(fpl__global__AppState != fpl_null);
	const fpl__PThreadApi *pthreadApi = &fpl__global__AppState->posix.pthreadApi;
	fplThreadHandle *thread = (fplThreadHandle *)data;
	FPL_ASSERT(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);
	if(thread->runFunc != fpl_null) {
		thread->runFunc(thread, thread->data);
	}
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
	thread->isValid = false;
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	pthreadApi->pthread_exit(data);
	return 0;
}

fpl_internal bool fpl__PosixMutexLock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int lockRes;
	do {
		lockRes = pthreadApi->pthread_mutex_lock(handle);
	} while(lockRes == EAGAIN);
	bool result = (lockRes == 0);
	return(result);
}

fpl_internal bool fpl__PosixMutexTryLock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int lockRes;
	do {
		lockRes = pthreadApi->pthread_mutex_trylock(handle);
	} while(lockRes == EAGAIN);
	bool result = (lockRes == 0);
	return(result);
}

fpl_internal bool fpl__PosixMutexUnlock(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	int unlockRes;
	do {
		unlockRes = pthreadApi->pthread_mutex_unlock(handle);
	} while(unlockRes == EAGAIN);
	bool result = (unlockRes == 0);
	return(result);
}

fpl_internal int fpl__PosixMutexCreate(const fpl__PThreadApi *pthreadApi, pthread_mutex_t *handle) {
	FPL_STRUCT_SET(handle, pthread_mutex_t, PTHREAD_MUTEX_INITIALIZER);
	int mutexRes;
	do {
		mutexRes = pthreadApi->pthread_mutex_init(handle, fpl_null);
	} while(mutexRes == EAGAIN);
	return(mutexRes);
}

fpl_internal bool fpl__PosixThreadWaitForMultiple(fplThreadHandle *threads[], const uint32_t minCount, const uint32_t maxCount, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(threads, false);
	FPL__CheckArgumentMax(maxCount, FPL__MAX_THREAD_COUNT, false);
	for(uint32_t index = 0; index < maxCount; ++index) {
		fplThreadHandle *thread = threads[index];
		if(thread == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Thread for index '%d' are not allowed to be null", index);
			return false;
		}
	}

	uint32_t completeCount = 0;
	bool isRunning[FPL__MAX_THREAD_COUNT];
	for(uint32_t index = 0; index < maxCount; ++index) {
		fplThreadHandle *thread = threads[index];
		isRunning[index] = fplGetThreadState(thread) != fplThreadState_Stopped;
		if(!isRunning[index]) {
			++completeCount;
		}
	}

	uint64_t startTime = fplGetTimeInMilliseconds();
	bool result = false;
	while(completeCount < minCount) {
		for(uint32_t index = 0; index < maxCount; ++index) {
			fplThreadHandle *thread = threads[index];
			if(isRunning[index]) {
				fplThreadState state = fplGetThreadState(thread);
				if(state == fplThreadState_Stopped) {
					isRunning[index] = false;
					++completeCount;
					if(completeCount >= minCount) {
						result = true;
						break;
					}
				}
			}
			fplThreadSleep(10);
		}
		if((timeout != FPL_TIMEOUT_INFINITE) && (fplGetTimeInMilliseconds() - startTime) >= timeout) {
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

fpl_platform_api uint32_t fplAtomicIncU32(volatile uint32_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api int32_t fplAtomicIncS32(volatile int32_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api uint64_t fplAtomicIncU64(volatile uint64_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api int64_t fplAtomicIncS64(volatile int64_t *value) {
	FPL_ASSERT(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, 1);
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
	struct timespec t;
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
	struct timespec t;
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
	FPL__CheckArgumentNull(thread, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if(thread->isValid && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		pthread_t threadHandle = thread->internalHandle.posixThread;
		if(pthreadApi->pthread_kill(threadHandle, 0) == 0) {
			pthreadApi->pthread_join(threadHandle, fpl_null);
		}
		thread->isValid = false;
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
		return true;
	} else {
		return false;
	}
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_function *runFunc, void *data) {
	FPL__CheckPlatform(fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if(thread != fpl_null) {
		thread->currentState = fplThreadState_Stopped;
		thread->data = data;
		thread->runFunc = runFunc;
		thread->isValid = false;
		thread->isStopping = false;

		// Create thread
		thread->currentState = fplThreadState_Starting;
		fplMemoryCopy(&thread->internalHandle.posixThread, FPL_MIN(sizeof(thread->id), sizeof(thread->internalHandle.posixThread)), &thread->id);
		int threadRes;
		do {
			threadRes = pthreadApi->pthread_create(&thread->internalHandle.posixThread, fpl_null, fpl__PosixThreadProc, (void *)thread);
		} while(threadRes == EAGAIN);
		if(threadRes != 0) {
			FPL_ERROR(FPL__MODULE_THREADING, "Failed creating thread, error code: %d", threadRes);
		}
		if(threadRes == 0) {
			thread->isValid = true;
			result = thread;
		} else {
			FPL_CLEAR_STRUCT(thread);
		}
	} else {
		FPL_ERROR(FPL__MODULE_THREADING, "All %d threads are in use, you cannot create until you free one", FPL__MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout) {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if((thread != fpl_null) && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		pthread_t threadHandle = thread->internalHandle.posixThread;

		// @NOTE(final): We optionally use the GNU extension "pthread_timedjoin_np" to support joining with a timeout.
		int joinRes;
		if((pthreadApi->pthread_timedjoin_np != fpl_null) && (timeout != FPL_TIMEOUT_INFINITE)) {
			struct timespec t;
			fpl__InitWaitTimeSpec(timeout, &t);
			joinRes = pthreadApi->pthread_timedjoin_np(threadHandle, fpl_null, &t);
		} else {
			joinRes = pthreadApi->pthread_join(threadHandle, fpl_null);
		}

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

fpl_platform_api bool fplThreadYield() {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = (pthreadApi->pthread_yield() == 0);
	return(result);
}

fpl_platform_api void fplThreadSleep(const uint32_t milliseconds) {
	uint32_t ms;
	uint32_t s;
	if(milliseconds > 1000) {
		s = milliseconds / 1000;
		ms = milliseconds % 1000;
	} else {
		s = 0;
		ms = milliseconds;
	}
	struct timespec input, output;
	input.tv_sec = s;
	input.tv_nsec = ms * 1000000;
	nanosleep(&input, &output);
}

fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if(mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex '%p' is already initialized", mutex);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_mutex_t mutexHandle;
	int mutexRes = fpl__PosixMutexCreate(pthreadApi, &mutexHandle);
	if(mutexRes != 0) {
		FPL_ERROR(FPL__MODULE_THREADING, "Failed creating POSIX condition");
		return false;
	}
	FPL_CLEAR_STRUCT(mutex);
	mutex->internalHandle.posixMutex = mutexHandle;
	mutex->isValid = true;
	return(true);
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if((mutex != fpl_null) && mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		pthreadApi->pthread_mutex_destroy(handle);
		FPL_CLEAR_STRUCT(mutex);
	}
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if(mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexLock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if(mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexTryLock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if(mutex->isValid) {
		pthread_mutex_t *handle = &mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexUnlock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t handle = PTHREAD_COND_INITIALIZER;
	int condRes;
	do {
		condRes = pthreadApi->pthread_cond_init(&handle, fpl_null);
	} while(condRes == EAGAIN);
	if(condRes == 0) {
		FPL_CLEAR_STRUCT(condition);
		condition->internalHandle.posixCondition = handle;
		condition->isValid = true;
	}
	return(condition->isValid);
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if((condition != fpl_null) && condition->isValid) {
		pthread_cond_t *handle = &condition->internalHandle.posixCondition;
		pthreadApi->pthread_cond_destroy(handle);
		FPL_CLEAR_STRUCT(condition);
	}
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckArgumentNull(mutex, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	if(!mutex->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Mutex is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *cond = &condition->internalHandle.posixCondition;
	pthread_mutex_t *mut = &mutex->internalHandle.posixMutex;
	bool result;
	if(timeout == FPL_TIMEOUT_INFINITE) {
		result = pthreadApi->pthread_cond_wait(cond, mut) == 0;
	} else {
		struct timespec t;
		fpl__InitWaitTimeSpec(timeout, &t);
		result = pthreadApi->pthread_cond_timedwait(cond, mut, &t) == 0;
	}
	return(result);
}

fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *handle = &condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_signal(handle) == 0;
	return(result);
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if(!condition->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *handle = &condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_broadcast(handle) == 0;
	return(result);
}

fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue) {
	FPL__CheckArgumentNull(semaphore, false);
	FPL__CheckArgumentMax(initialValue, INT32_MAX, false);
	if(semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is already initialized", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	FPL_CLEAR_STRUCT(semaphore);
	int res = pthreadApi->sem_init(&semaphore->internalHandle.posixHandle, 0, (int)initialValue);
	if(res < 0) {
		FPL_ERROR(FPL__MODULE_THREADING, "Failed creating semaphore");
		return false;
	}
	semaphore->isValid = true;
	return true;
}

fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if(semaphore != fpl_null) {
		if(semaphore->isValid) {
			pthreadApi->sem_destroy(&semaphore->internalHandle.posixHandle);
		}
		FPL_CLEAR_STRUCT(semaphore);
	}
}

fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	int res;
	if(timeout == FPL_TIMEOUT_INFINITE) {
		res = pthreadApi->sem_wait(&semaphore->internalHandle.posixHandle);
	} else {
		struct timespec t;
		fpl__InitWaitTimeSpec(timeout, &t);
		res = pthreadApi->sem_timedwait(&semaphore->internalHandle.posixHandle, &t);
	}
	bool result = res == 0;
	return(result);
}

fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	int res = pthreadApi->sem_trywait(&semaphore->internalHandle.posixHandle);
	bool result = (res == 0);
	return(result);
}

fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(0);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	int value = 0;
	int res = pthreadApi->sem_getvalue(&semaphore->internalHandle.posixHandle, &value);
	if(res < 0) {
		return 0;
	}
	return((int32_t)value);
}

fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if(!semaphore->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(0);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	int res = pthreadApi->sem_post(&semaphore->internalHandle.posixHandle);
	bool result = (res == 0);
	return(result);
}

//
// POSIX Library
//
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle) {
	bool result = false;
	if(libraryFilePath != fpl_null && outHandle != fpl_null) {
		void *p = dlopen(libraryFilePath, FPL__POSIX_DL_LOADTYPE);
		if(p != fpl_null) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->internalHandle.posixLibraryHandle = p;
			outHandle->isValid = true;
			result = true;
		}
	}
	return(result);
}

fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name) {
	void *result = fpl_null;
	if((handle != fpl_null) && (handle->internalHandle.posixLibraryHandle != fpl_null) && (name != fpl_null)) {
		void *p = handle->internalHandle.posixLibraryHandle;
		result = dlsym(p, name);
	}
	return(result);
}

fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if((handle != fpl_null) && (handle->internalHandle.posixLibraryHandle != fpl_null)) {
		void *p = handle->internalHandle.posixLibraryHandle;
		dlclose(p);
		FPL_CLEAR_STRUCT(handle);
	}
}

//
// POSIX Memory
//
fpl_platform_api void *fplMemoryAllocate(const size_t size) {
	FPL__CheckArgumentZero(size, fpl_null);
	// @NOTE(final): MAP_ANONYMOUS ensures that the memory is cleared to zero.
	// Allocate empty memory to hold the size + some arbitary padding + the actual data
	size_t newSize = sizeof(size_t) + FPL__ARBITARY_PADDING + size;
	void *basePtr = mmap(fpl_null, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	// Write the size at the beginning
	*(size_t *)basePtr = newSize;
	// The resulting address starts after the arbitary padding
	void *result = (uint8_t *)basePtr + sizeof(size_t) + FPL__ARBITARY_PADDING;
	return(result);
}

fpl_platform_api void fplMemoryFree(void *ptr) {
	FPL__CheckArgumentNullNoRet(ptr);
	// Free the base pointer which is stored to the left at the start of the size_t
	void *basePtr = (void *)((uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(size_t)));
	size_t storedSize = *(size_t *)basePtr;
	munmap(basePtr, storedSize);
}

//
// POSIX Files
//
fpl_platform_api bool fplOpenAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while(posixFileHandle == -1 && errno == EINTR);
		if(posixFileHandle != -1) {
			FPL_CLEAR_STRUCT(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.posixFileHandle = posixFileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplOpenBinaryWideFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		char utf8FilePath[1024] = FPL_ZERO_INIT;
		fplWideStringToAnsiString(filePath, fplGetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		bool result = fplOpenAnsiBinaryFile(utf8FilePath, outHandle);
		return(result);
	}
	return false;
}

fpl_platform_api bool fplCreateAnsiBinaryFile(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		} while(posixFileHandle == -1 && errno == EINTR);
		if(posixFileHandle != -1) {
			outHandle->isValid = true;
			outHandle->internalHandle.posixFileHandle = posixFileHandle;
			return true;
		}
	}
	return false;
}
fpl_platform_api bool fplCreateWideBinaryFile(const wchar_t *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if(filePath != fpl_null) {
		char utf8FilePath[1024] = FPL_ZERO_INIT;
		fplWideStringToAnsiString(filePath, fplGetWideStringLength(filePath), utf8FilePath, FPL_ARRAYCOUNT(utf8FilePath));
		bool result = fplCreateAnsiBinaryFile(utf8FilePath, outHandle);
		return(result);
	}
	return false;
}

fpl_internal size_t fpl__PosixReadFileBlock(const fplFileHandle *fileHandle, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize) {
    FPL__CheckArgumentNull(fileHandle, 0);
    FPL__CheckArgumentZero(sizeToRead, 0);
    FPL__CheckArgumentNull(targetBuffer, 0);
    if(!fileHandle->internalHandle.posixFileHandle) {
        FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for reading");
        return 0;
    }
    int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
    ssize_t res;
    do {
        res = read(posixFileHandle, targetBuffer, sizeToRead);
    } while(res == -1 && errno == EINTR);
    size_t result = 0;
    if(res != -1) {
        result = (size_t)res;
    }
    return(result);
}

fpl_platform_api size_t fpl__PosixWriteFileBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize) {
    FPL__CheckArgumentNull(fileHandle, 0);
    FPL__CheckArgumentZero(sourceSize, 0);
    FPL__CheckArgumentNull(sourceBuffer, 0);
    if(!fileHandle->internalHandle.posixFileHandle) {
        FPL_ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
        return 0;
    }
    int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
    ssize_t res;
    do {
        res = write(posixFileHandle, sourceBuffer, sourceSize);
    } while(res == -1 && errno == EINTR);
    size_t result = 0;
    if(res != -1) {
        result = (size_t)res;
    }
    return(result);
}

fpl_platform_api uint32_t fplReadFileBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
    uint32_t result = (uint32_t)fpl__PosixReadFileBlock(fileHandle, sizeToRead, targetBuffer, maxTargetBufferSize);
    return(result);
}

fpl_platform_api uint64_t fplReadFileBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize) {
    uint64_t result = (uint64_t)fpl__PosixReadFileBlock(fileHandle, sizeToRead, targetBuffer, maxTargetBufferSize);
	return(result);
}

fpl_platform_api uint32_t fplWriteFileBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
    uint32_t result = (uint32_t)fpl__PosixWriteFileBlock(fileHandle, sourceBuffer, sourceSize);
    return(result);
}

fpl_platform_api uint64_t fplWriteFileBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize) {
    uint64_t result = (uint64_t)fpl__PosixWriteFileBlock(fileHandle, sourceBuffer, sourceSize);
    return(result);
}

fpl_platform_api uint32_t fplSetFilePosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if(fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		int whence = SEEK_SET;
		if(mode == fplFilePositionMode_Current) {
			whence = SEEK_CUR;
		} else if(mode == fplFilePositionMode_End) {
			whence = SEEK_END;
		}
		off_t r = lseek(posixFileHandle, position, whence);
		result = (uint32_t)r;
	}
	return(result);
}

fpl_platform_api uint64_t fplSetFilePosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode) {
    FPL__CheckArgumentNull(fileHandle, 0);
    uint64_t result = 0;
    if(fileHandle->internalHandle.posixFileHandle) {
        int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
        int whence = SEEK_SET;
        if(mode == fplFilePositionMode_Current) {
            whence = SEEK_CUR;
        } else if(mode == fplFilePositionMode_End) {
            whence = SEEK_END;
        }
        off64_t r = lseek64(posixFileHandle, position, whence);
        result = (uint64_t)r;
    }
    return(result);
}

fpl_platform_api uint32_t fplGetFilePosition32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if(fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		off_t res = lseek(posixFileHandle, 0, SEEK_CUR);
		if(res != -1) {
			result = (uint32_t)res;
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplGetFilePosition64(const fplFileHandle *fileHandle) {
    FPL__CheckArgumentNull(fileHandle, 0);
    uint64_t result = 0;
    if(fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
        int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
        off64_t res = lseek64(posixFileHandle, 0, SEEK_CUR);
        if(res != -1) {
            result = (uint64_t)res;
        }
    }
    return(result);
}

fpl_platform_api void fplCloseFile(fplFileHandle *fileHandle) {
	if((fileHandle != fpl_null) && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		close(posixFileHandle);
		FPL_CLEAR_STRUCT(fileHandle);
	}
}

fpl_platform_api uint32_t fplGetFileSizeFromPath32(const char *filePath) {
	uint32_t result = 0;
	if(filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while(posixFileHandle == -1 && errno == EINTR);
		if(posixFileHandle != -1) {
			off_t res = lseek(posixFileHandle, 0, SEEK_END);
			if(res != -1) {
				result = (uint32_t)res;
			}
			close(posixFileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplGetFileSizeFromPath64(const char *filePath) {
    uint64_t result = 0;
    if(filePath != fpl_null) {
        int posixFileHandle;
        do {
            posixFileHandle = open(filePath, O_RDONLY);
        } while(posixFileHandle == -1 && errno == EINTR);
        if(posixFileHandle != -1) {
            off64_t res = lseek64(posixFileHandle, 0, SEEK_END);
            if(res != -1) {
                result = (uint64_t)res;
            }
            close(posixFileHandle);
        }
    }
    return(result);
}

fpl_platform_api uint32_t fplGetFileSizeFromHandle32(const fplFileHandle *fileHandle) {
	uint32_t result = 0;
	if(fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		off_t curPos = lseek(posixFileHandle, 0, SEEK_CUR);
		if(curPos != -1) {
			result = (uint32_t)lseek(posixFileHandle, 0, SEEK_END);
			lseek(posixFileHandle, curPos, SEEK_SET);
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplGetFileSizeFromHandle64(const fplFileHandle *fileHandle) {
    uint64_t result = 0;
    if(fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
        int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
        off64_t curPos = lseek64(posixFileHandle, 0, SEEK_CUR);
        if(curPos != -1) {
            result = (uint64_t)lseek(posixFileHandle, 0, SEEK_END);
            lseek64(posixFileHandle, curPos, SEEK_SET);
        }
    }
    return(result);
}

fpl_internal uint64_t fpl__PosixConvertTimespecToUnixTimeStamp(const struct timespec *spec) {
	uint64_t result = (uint64_t)spec->tv_sec;
	return(result);
}

fpl_platform_api bool fplGetFileTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(outStamps, false);
	bool result = false;
	if(filePath != fpl_null) {
		struct stat statBuf;
		if(stat(filePath, &statBuf) != -1) {
			outStamps->creationTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_ctim);
			outStamps->lastAccessTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_atim);
			outStamps->lastModifyTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_mtim);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplGetFileTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(fileHandle, false);
	FPL__CheckArgumentNull(outStamps, false);
	bool result = false;
	if(fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		struct stat statBuf;
		if(fstat(posixFileHandle, &statBuf) != -1) {
			outStamps->creationTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_ctim);
			outStamps->lastAccessTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_atim);
			outStamps->lastModifyTime = fpl__PosixConvertTimespecToUnixTimeStamp(&statBuf.st_mtim);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if(filePath != fpl_null) {
		result = access(filePath, F_OK) != -1;
	}
	return(result);
}

fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	if(access(sourceFilePath, F_OK) == -1) {
		FPL_ERROR(FPL__MODULE_FILES, "Source file '%s' does not exits", sourceFilePath);
		return false;
	}
	if(!overwrite && access(sourceFilePath, F_OK) != -1) {
		FPL_ERROR(FPL__MODULE_FILES, "Target file '%s' already exits", targetFilePath);
		return false;
	}
	int inputFileHandle;
	do {
		inputFileHandle = open(sourceFilePath, O_RDONLY);
	} while(inputFileHandle == -1 && errno == EINTR);
	if(inputFileHandle == -1) {
		FPL_ERROR(FPL__MODULE_FILES, "Failed open source file '%s', error code: %d", sourceFilePath, inputFileHandle);
		return false;
	}
	int outputFileHandle;
	do {
		outputFileHandle = open(targetFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	} while(outputFileHandle == -1 && errno == EINTR);
	if(outputFileHandle == -1) {
		close(inputFileHandle);
		FPL_ERROR(FPL__MODULE_FILES, "Failed creating target file '%s', error code: %d", targetFilePath, inputFileHandle);
		return false;
	}
	uint8_t buffer[1024 * 10]; // 10 kb buffer
	for(;;) {
		ssize_t readbytes;
		do {
			readbytes = read(inputFileHandle, buffer, FPL_ARRAYCOUNT(buffer));
		} while(readbytes == -1 && errno == EINTR);
		if(readbytes > 0) {
			ssize_t writtenBytes;
			do {
				writtenBytes = write(outputFileHandle, buffer, readbytes);
			} while(writtenBytes == -1 && errno == EINTR);
			if(writtenBytes <= 0) {
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
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	bool result = rename(sourceFilePath, targetFilePath) == 0;
	return(result);
}

fpl_platform_api bool fplFileDelete(const char *filePath) {
	FPL__CheckArgumentNull(filePath, false);
	bool result = unlink(filePath) == 0;
	return(result);
}

fpl_platform_api bool fplDirectoryExists(const char *path) {
	bool result = false;
	if(path != fpl_null) {
		struct stat sb;
		result = (stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
	}
	return(result);
}

fpl_platform_api bool fplDirectoriesCreate(const char *path) {
	FPL__CheckArgumentNull(path, false);
	bool result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == 0;
	return(result);
}
fpl_platform_api bool fplRemoveDirectory(const char *path) {
	FPL__CheckArgumentNull(path, false);
	bool result = rmdir(path) == 0;
	return(result);
}

fpl_internal void fpl__PosixFillFileEntry(struct dirent *dp, fplFileEntry *entry) {
	FPL_ASSERT((dp != fpl_null) && (entry != fpl_null));
	fplCopyAnsiString(entry->internalRoot.rootPath, entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	fplEnforcePathSeparatorLen(entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	fplStringAppend(dp->d_name, entry->fullPath, FPL_ARRAYCOUNT(entry->fullPath));
	entry->type = fplFileEntryType_Unknown;
	entry->attributes = fplFileAttributeFlags_None;
	entry->size = 0;
	entry->permissions.umask = 0;
	struct stat sb;
	if(stat(entry->fullPath, &sb) == 0) {
		if(S_ISDIR(sb.st_mode)) {
			entry->type = fplFileEntryType_Directory;
		} else if(S_ISREG(sb.st_mode)) {
			entry->type = fplFileEntryType_File;
		}
		entry->size = (size_t)sb.st_size;
		if(dp->d_name[0] == '.') {
			// @NOTE(final): Any filename starting with dot is hidden in POSIX
			entry->attributes |= fplFileAttributeFlags_Hidden;
		}
		if(sb.st_mode & S_IRUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanRead;
		}
		if(sb.st_mode & S_IWUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanWrite;
		}
		if(sb.st_mode & S_IXUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanExecuteSearch;
		}
		if(sb.st_mode & S_IRGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanRead;
		}
		if(sb.st_mode & S_IWGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanWrite;
		}
		if(sb.st_mode & S_IXGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanExecuteSearch;
		}
		if(sb.st_mode & S_IROTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanRead;
		}
		if(sb.st_mode & S_IWOTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanWrite;
		}
		if(sb.st_mode & S_IXOTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanExecuteSearch;
		}
	}
}

fpl_platform_api bool fplListDirBegin(const char *path, const char *filter, fplFileEntry *entry) {
	FPL__CheckArgumentNull(path, false);
	FPL__CheckArgumentNull(entry, false);
	DIR *dir = opendir(path);
	if(dir == fpl_null) {
		return false;
	}
	if(fplGetAnsiStringLength(filter) == 0) {
		filter = "*";
	}
	FPL_CLEAR_STRUCT(entry);
	entry->internalHandle.posixDirHandle = dir;
	entry->internalRoot.rootPath = path;
	entry->internalRoot.filter = filter;
	bool result = fplListDirNext(entry);
	return(result);
}

fpl_platform_api bool fplListDirNext(fplFileEntry *entry) {
	FPL__CheckArgumentNull(entry, false);
	bool result = false;
	if(entry->internalHandle.posixDirHandle != fpl_null) {
		struct dirent *dp = readdir(entry->internalHandle.posixDirHandle);
		do {
			if(dp == fpl_null) {
				break;
			}
			bool isMatch = fplIsStringMatchWildcard(dp->d_name, entry->internalRoot.filter);
			if(isMatch) {
				break;
			}
			dp = readdir(entry->internalHandle.posixDirHandle);
		} while(dp != fpl_null);
		if(dp == fpl_null) {
			closedir(entry->internalHandle.posixDirHandle);
			FPL_CLEAR_STRUCT(entry);
		} else {
			fpl__PosixFillFileEntry(dp, entry);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api void fplListDirEnd(fplFileEntry *entry) {
	FPL__CheckArgumentNullNoRet(entry);
	if(entry->internalHandle.posixDirHandle != fpl_null) {
		closedir(entry->internalHandle.posixDirHandle);
		FPL_CLEAR_STRUCT(entry);
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
	// @NOTE(final): Changes the locale to ANSI temporary
	FPL__CheckArgumentNull(wideSource, fpl_null);
	FPL__CheckArgumentNull(ansiDest, fpl_null);
	const char* lastLocale = setlocale(LC_CTYPE, fpl_null);
	setlocale(LC_CTYPE, "C"); size_t requiredLen = wcstombs(fpl_null, wideSource, maxWideSourceLen); setlocale(LC_CTYPE, lastLocale);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxAnsiDestLen, minRequiredLen, fpl_null);
	setlocale(LC_CTYPE, "C"); wcstombs(ansiDest, wideSource, maxWideSourceLen); setlocale(LC_CTYPE, lastLocale);
	ansiDest[requiredLen] = 0;
	return(ansiDest);
}
fpl_platform_api char *fplWideStringToUTF8String(const wchar_t *wideSource, const size_t maxWideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	// @NOTE(final): Expect locale to be UTF-8
	FPL__CheckArgumentNull(wideSource, fpl_null);
	FPL__CheckArgumentNull(utf8Dest, fpl_null);
	size_t requiredLen = wcstombs(fpl_null, wideSource, maxWideSourceLen);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxUtf8DestLen, minRequiredLen, fpl_null);
	wcstombs(utf8Dest, wideSource, maxWideSourceLen);
	utf8Dest[requiredLen] = 0;
	return(utf8Dest);
}
fpl_platform_api wchar_t *fplAnsiStringToWideString(const char *ansiSource, const size_t ansiSourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	// @NOTE(final): Changes the locale to ANSI temporary
	FPL__CheckArgumentNull(ansiSource, fpl_null);
	FPL__CheckArgumentNull(wideDest, fpl_null);
	const char* lastLocale = setlocale(LC_CTYPE, fpl_null);
	setlocale(LC_CTYPE, "C");	size_t requiredLen = mbstowcs(fpl_null, ansiSource, ansiSourceLen);	setlocale(LC_CTYPE, lastLocale);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxWideDestLen, minRequiredLen, fpl_null);
	setlocale(LC_CTYPE, "C");	mbstowcs(wideDest, ansiSource, ansiSourceLen);	setlocale(LC_CTYPE, lastLocale);
	wideDest[requiredLen] = 0;
	return(wideDest);
}
fpl_platform_api wchar_t *fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	// @NOTE(final): Expect locale to be UTF-8
	FPL__CheckArgumentNull(utf8Source, fpl_null);
	FPL__CheckArgumentNull(wideDest, fpl_null);
	size_t requiredLen = mbstowcs(fpl_null, utf8Source, utf8SourceLen);
	size_t minRequiredLen = requiredLen + 1;
	FPL__CheckArgumentMin(maxWideDestLen, minRequiredLen, fpl_null);
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
	if(text != fpl_null) {
		fprintf(stdout, "%s", text);
	}
}
fpl_platform_api void fplConsoleError(const char *text) {
	if(text != fpl_null) {
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
	FPL_ASSERT(subplatform != fpl_null);
	fpl__UnloadX11Api(&subplatform->api);
}

fpl_internal bool fpl__X11InitSubplatform(fpl__X11SubplatformState *subplatform) {
	FPL_ASSERT(subplatform != fpl_null);
	if(!fpl__LoadX11Api(&subplatform->api)) {
		FPL_ERROR(FPL__MODULE_X11, "Failed loading x11 api");
		return false;
	}
	return true;
}

fpl_internal void fpl__X11ReleaseWindow(const fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState) {
	FPL_ASSERT((subplatform != fpl_null) && (windowState != fpl_null));
	const fpl__X11Api *x11Api = &subplatform->api;
	if(windowState->window) {
		FPL_LOG_DEBUG("X11", "Hide window '%d' from display '%p'", (int)windowState->window, windowState->display);
		x11Api->XUnmapWindow(windowState->display, windowState->window);
		FPL_LOG_DEBUG("X11", "Destroy window '%d' on display '%p'", (int)windowState->window, windowState->display);
		x11Api->XDestroyWindow(windowState->display, windowState->window);
		windowState->window = 0;
	}
	if(windowState->colorMap) {
		FPL_LOG_DEBUG("X11", "Release color map '%d' from display '%p'", (int)windowState->colorMap, windowState->display);
		x11Api->XFreeColormap(windowState->display, windowState->colorMap);
		windowState->colorMap = 0;
	}
	if(windowState->display) {
		FPL_LOG_DEBUG("X11", "Close display '%p'", windowState->display);
		x11Api->XCloseDisplay(windowState->display);
		windowState->display = fpl_null;
	}
	FPL_CLEAR_STRUCT(windowState);
}

fpl_internal fplKey fpl__X11TranslateKeySymbol(const KeySym keySym) {
	switch(keySym) {
		case XK_BackSpace:
			return fplKey_Backspace;
		case XK_Tab:
			return fplKey_Tab;

		case XK_Return:
			return fplKey_Return;

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
			return fplKey_LeftSuper;
		case XK_Super_R:
			return fplKey_RightSuper;

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

		case XK_comma:
			return fplKey_OemComma;
		case XK_period:
			return fplKey_OemPeriod;
		case XK_minus:
			return fplKey_OemMinus;
		case XK_plus:
			return fplKey_OemPlus;

		// @TODO(final): X11 map OEM1-OEM8 key

		default:
			return fplKey_None;
	}
}

fpl_internal void fpl__X11LoadWindowIcon(const fpl__X11Api *x11Api, fpl__X11WindowState *x11WinState, fplWindowSettings *windowSettings) {
	// @TODO(final): Setting the window icon on X11 does not fail, but it does not show up in any of the bars
	// In gnome/ubuntu the icon is always shown as "unset"

	int iconSourceCount = 0;
	fplImageSource iconSources[2] = FPL_ZERO_INIT;

	if(windowSettings->icons[0].width > 0) {
		iconSources[iconSourceCount++] = windowSettings->icons[0];
	}
	if(windowSettings->icons[1].width > 0) {
		iconSources[iconSourceCount++] = windowSettings->icons[1];
	}

	if(iconSourceCount > 0) {
		int targetSize = 0;
		for(int i = 0; i < iconSourceCount; ++i) {
			targetSize += 2 + iconSources[i].width * iconSources[i].height;
		}

		// @MEMORY(final): Do not allocate memory here, use a static memory block or introduce a temporary memory arena!
		uint32_t *data = (uint32_t *)fplMemoryAllocate(sizeof(uint32_t) * targetSize);
		uint32_t *target = data;

		for(int i = 0; i < iconSourceCount; ++i) {
			const fplImageSource *iconSource = iconSources + i;
			FPL_ASSERT(iconSource->type == fplImageType_RGBA);
			*target++ = (int32_t)iconSource->width;
			*target++ = (int32_t)iconSource->height;
			const uint32_t *source = (const uint32_t *)iconSource->data;
			for(int j = 0; j < iconSource->width * iconSource->height; ++j) {
				// @TODO(final): Do we need to swap the byte order of the icon in X11?
#if 0
				* target++ = (iconSource->data[j * 4 + 0] << 16) |
					(iconSource->data[j * 4 + 1] << 8) |
					(iconSource->data[j * 4 + 2] << 0) |
					(iconSource->data[j * 4 + 3] << 24);
#else
				*target++ = *source;
#endif
			}
		}

		x11Api->XChangeProperty(x11WinState->display, x11WinState->window, x11WinState->netWMIcon, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, targetSize);

		// @MEMORY(final): Do not free memory here, see note above!
		fplMemoryFree(data);
	} else {
		x11Api->XDeleteProperty(x11WinState->display, x11WinState->window, x11WinState->netWMIcon);
	}

	x11Api->XFlush(x11WinState->display);
}

fpl_internal bool fpl__X11InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *appState, fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	FPL_ASSERT((initSettings != fpl_null) && (currentWindowSettings != fpl_null) && (appState != fpl_null) && (subplatform != fpl_null) && (windowState != fpl_null) && (setupCallbacks != fpl_null));
	const fpl__X11Api *x11Api = &subplatform->api;

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Open default Display");
	windowState->display = x11Api->XOpenDisplay(fpl_null);
	if(windowState->display == fpl_null) {
		FPL_ERROR(FPL__MODULE_X11, "Failed opening default Display!");
		return false;
	}
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Successfully opened default Display: %p", windowState->display);

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Get default screen from display '%p'", windowState->display);
	windowState->screen = x11Api->XDefaultScreen(windowState->display);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Got default screen from display '%p': %d", windowState->display, windowState->screen);

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Get root window from display '%p' and screen '%d'", windowState->display, windowState->screen);
	windowState->root = x11Api->XRootWindow(windowState->display, windowState->screen);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Got root window from display '%p' and screen '%d': %d", windowState->display, windowState->screen, (int)windowState->root);

	bool usePreSetupWindow = false;
	fpl__PreSetupWindowResult setupResult = FPL_ZERO_INIT;
	if(setupCallbacks->preSetup != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Call Pre-Setup for Window");
		usePreSetupWindow = setupCallbacks->preSetup(appState, appState->initFlags, initSettings, &setupResult);
	}

	Visual *visual = fpl_null;
	int colorDepth = 0;
	Colormap colormap;
	if(usePreSetupWindow) {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Got visual '%p' and color depth '%d' from pre-setup", setupResult.x11.visual, setupResult.x11.colorDepth);
		FPL_ASSERT(setupResult.x11.visual != fpl_null);
		visual = setupResult.x11.visual;
		colorDepth = setupResult.x11.colorDepth;
		colormap = x11Api->XCreateColormap(windowState->display, windowState->root, visual, AllocNone);
	} else {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Using default colormap, visual, color depth");
		visual = x11Api->XDefaultVisual(windowState->display, windowState->root);
		colorDepth = x11Api->XDefaultDepth(windowState->display, windowState->root);
		colormap = x11Api->XDefaultColormap(windowState->display, windowState->root);
	}
	int flags = CWColormap | CWEventMask | CWBorderPixel | CWBackPixel | CWBitGravity | CWWinGravity;

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using visual: %p", visual);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using color depth: %d", colorDepth);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using color map: %d", (int)colormap);

	windowState->colorMap = colormap;

	XSetWindowAttributes swa = FPL_ZERO_INIT;
	swa.colormap = colormap;
	swa.event_mask =
		StructureNotifyMask |
		ExposureMask | FocusChangeMask | VisibilityChangeMask |
		EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
		KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;
	swa.background_pixel = WhitePixel(windowState->display, windowState->screen);
	swa.bit_gravity = NorthWestGravity;
	swa.win_gravity = NorthWestGravity;

	int windowX = 0;
	int windowY = 0;
	int windowWidth;
	int windowHeight;
	if((initSettings->window.windowWidth > 0) &&
		(initSettings->window.windowHeight > 0)) {
		windowWidth = initSettings->window.windowWidth;
		windowHeight = initSettings->window.windowHeight;
	} else {
		windowWidth = 400;
		windowHeight = 400;
	}

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Create window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
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
												flags,
												&swa);
	if(!windowState->window) {
		FPL_ERROR(FPL__MODULE_X11, "Failed creating window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'!", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
		fpl__X11ReleaseWindow(subplatform, windowState);
		return false;
	}
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Successfully created window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d': %d", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap, (int)windowState->window);

	// Force keyboard and button events
	x11Api->XSelectInput(windowState->display, windowState->window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask);

	windowState->visual = visual;

	char idBuffer[100];

	// Type atoms
	fplCopyAnsiString("UTF8_STRING", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->utf8String = x11Api->XInternAtom(windowState->display, idBuffer, False);

	// Window manager atoms
	fplCopyAnsiString("WM_DELETE_WINDOW", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->wmDeleteWindow = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("WM_PROTOCOLS", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->wmProtocols = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_STATE", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMState = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_PING", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMPing = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_STATE_FULLSCREEN", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMStateFullscreen = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_PID", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMPid = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_ICON", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMIcon = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_NAME", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMName = x11Api->XInternAtom(windowState->display, idBuffer, False);
	fplCopyAnsiString("_NET_WM_ICON_NAME", idBuffer, FPL_ARRAYCOUNT(idBuffer));
	windowState->netWMIconName = x11Api->XInternAtom(windowState->display, idBuffer, False);

	// Register window manager protocols
	{
		Atom protocols[] = {
				windowState->wmDeleteWindow,
				windowState->netWMPing
		};
		x11Api->XSetWMProtocols(windowState->display, windowState->window, protocols, FPL_ARRAYCOUNT(protocols));
	}

	// Declare our process id
	{
		const long pid = getpid();
		x11Api->XChangeProperty(windowState->display, windowState->window, windowState->netWMPid, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&pid, 1);
	}

	char nameBuffer[1024] = FPL_ZERO_INIT;
	if(fplGetAnsiStringLength(initSettings->window.windowTitle) > 0) {
		fplCopyAnsiString(initSettings->window.windowTitle, nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
	} else {
		fplCopyAnsiString("Unnamed FPL X11 Window", nameBuffer, FPL_ARRAYCOUNT(nameBuffer));
	}
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Show window '%d' on display '%p' with title '%s'", (int)windowState->window, windowState->display, nameBuffer);
	fpl__X11LoadWindowIcon(x11Api, windowState, currentWindowSettings);
	fplSetWindowAnsiTitle(nameBuffer);
	x11Api->XMapWindow(windowState->display, windowState->window);
	x11Api->XFlush(windowState->display);

	FPL_ASSERT(FPL_ARRAYCOUNT(appState->window.keyMap) >= 256);

	// @NOTE(final): Valid key range for XLib is 8 to 255
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Build X11 Keymap");
	FPL_CLEAR_STRUCT(appState->window.keyMap);
	for(int keyCode = 8; keyCode <= 255; ++keyCode) {
		int dummy = 0;
		KeySym *keySyms = x11Api->XGetKeyboardMapping(windowState->display, keyCode, 1, &dummy);
		KeySym keySym = keySyms[0];
		fplKey mappedKey = fpl__X11TranslateKeySymbol(keySym);
		appState->window.keyMap[keyCode] = mappedKey;
		x11Api->XFree(keySyms);
	}

	if(initSettings->window.isFullscreen) {
		fplSetWindowFullscreen(true, initSettings->window.fullscreenWidth, initSettings->window.fullscreenHeight, 0);
	}

	appState->window.isRunning = true;

	return true;
}

fpl_internal fplKeyboardModifierFlags fpl__X11TranslateModifierFlags(const int state) {
	fplKeyboardModifierFlags result = fplKeyboardModifierFlags_None;
	if(state & ShiftMask) {
		result |= fplKeyboardModifierFlags_LShift;
		result |= fplKeyboardModifierFlags_RShift;
	}
	if(state & ControlMask) {
		result |= fplKeyboardModifierFlags_LCtrl;
		result |= fplKeyboardModifierFlags_RCtrl;
	}
	if(state & Mod1Mask) {
		result |= fplKeyboardModifierFlags_LAlt;
		result |= fplKeyboardModifierFlags_RAlt;
	}
	if(state & Mod4Mask) {
		result |= fplKeyboardModifierFlags_LSuper;
		result |= fplKeyboardModifierFlags_RSuper;
	}
	return(result);
}

fpl_internal unsigned long fpl__X11GetWindowProperty(const fpl__X11Api *x11Api, Display *display, Window window, Atom prop, Atom type, unsigned char** value) {
	Atom actualType;
	int actualFormat;
	unsigned long itemCount, bytesAfter;
	x11Api->XGetWindowProperty(display, window, prop, 0, LONG_MAX, False, type, &actualType, &actualFormat, &itemCount, &bytesAfter, value);
	return(itemCount);
}

fpl_internal bool fpl__X11HandleEvent(const fpl__X11SubplatformState *subplatform, fpl__PlatformAppState *appState, XEvent *ev) {
	FPL_ASSERT((subplatform != fpl_null) && (appState != fpl_null) && (ev != fpl_null));
	fpl__PlatformWindowState *winState = &appState->window;
	fpl__X11WindowState *x11WinState = &winState->x11;
	const fpl__X11Api *x11Api = &appState->x11.api;
	bool result = true;
	switch(ev->type) {
		case ConfigureNotify:
		{
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			if(appState->currentSettings.video.driver == fplVideoDriverType_Software) {
				if(appState->initSettings.video.isAutoSize) {
					uint32_t w = (uint32_t)ev->xconfigure.width;
					uint32_t h = (uint32_t)ev->xconfigure.height;
					fplResizeVideoBackBuffer(w, h);
				}
			}
#			endif

			// Window resized
			fpl__PushWindowEvent(fplWindowEventType_Resized, (uint32_t)ev->xconfigure.width, (uint32_t)ev->xconfigure.height);
		} break;

		case ClientMessage:
		{
			if(ev->xclient.message_type == x11WinState->wmProtocols) {
				const Atom protocol = (Atom)ev->xclient.data.l[0];
				if(protocol != None) {
					if(protocol == x11WinState->wmDeleteWindow) {
						// Window asked for closing
						winState->isRunning = false;
						result = false;
					} else if(protocol == x11WinState->netWMPing) {
						// Window manager asks us if we are still alive
						XEvent reply = *ev;
						reply.xclient.window = x11WinState->root;
						x11Api->XSendEvent(x11WinState->display, x11WinState->root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
					}
				}
			}
		} break;

		case KeyPress:
		{
			// Keyboard button down
			int keyState = ev->xkey.state;
			int keyCode = ev->xkey.keycode;
			fpl__HandleKeyboardButtonEvent(winState, (uint64_t)keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Press, false);
		} break;

		case KeyRelease:
		{
			// Keyboard button up
			bool physical = true;
			if(x11Api->XPending(x11WinState->display)) {
				XEvent nextEvent;
				x11Api->XPeekEvent(x11WinState->display, &nextEvent);
				if(nextEvent.type == KeyPress && nextEvent.xkey.time == ev->xkey.time && nextEvent.xkey.keycode == ev->xkey.keycode) {
					// Delete future key press event, as it is a key-repeat
					x11Api->XNextEvent(x11WinState->display, ev);
					physical = false;
				}
			}
			int keyState = ev->xkey.state;
			int keyCode = ev->xkey.keycode;
			if(physical) {
				fpl__HandleKeyboardButtonEvent(winState, (uint64_t)keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Release, true);
			} else {
				fpl__HandleKeyboardButtonEvent(winState, (uint64_t)keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Repeat, false);
			}
		} break;

		case ButtonPress:
		{
			// Mouse down
			int x = ev->xbutton.x;
			int y = ev->xbutton.y;
			if(ev->xbutton.button == Button1) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Left, fplButtonState_Press);
			} else if(ev->xbutton.button == Button2) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Middle, fplButtonState_Press);
			} else if(ev->xbutton.button == Button3) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Right, fplButtonState_Press);
			} else if(ev->xbutton.button == Button4) {
				fpl__HandleMouseWheelEvent(winState, x, y, 1.0f);
			} else if(ev->xbutton.button == Button5) {
				fpl__HandleMouseWheelEvent(winState, x, y, -1.0f);
			}
		} break;

		case ButtonRelease:
		{
			// Mouse up
			int x = ev->xbutton.x;
			int y = ev->xbutton.y;
			if(ev->xbutton.button == Button1) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Left, fplButtonState_Release);
			} else if(ev->xbutton.button == Button2) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Middle, fplButtonState_Release);
			} else if(ev->xbutton.button == Button3) {
				fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Right, fplButtonState_Release);
			}

		} break;

		case MotionNotify:
		{
			// Mouse move
			fpl__HandleMouseMoveEvent(winState, ev->xmotion.x, ev->xmotion.y);
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
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	if(x11Api->XPending(windowState->display)) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		result = fpl__X11HandleEvent(&appState->x11, appState, &ev);
	}
	return (result);
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL__CheckPlatform(false);
	bool result = fpl__global__AppState->window.isRunning;
	return(result);
}

fpl_platform_api void fplWindowShutdown() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	if(appState->window.isRunning) {
		appState->window.isRunning = false;
		const fpl__X11SubplatformState *subplatform = &appState->x11;
		const fpl__X11Api *x11Api = &subplatform->api;
		const fpl__X11WindowState *windowState = &appState->window.x11;
		XEvent ev = FPL_ZERO_INIT;
		ev.type = ClientMessage;
		ev.xclient.window = windowState->window;
		ev.xclient.message_type = windowState->wmProtocols;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = windowState->wmDeleteWindow;
		ev.xclient.data.l[1] = 0;
		x11Api->XSendEvent(windowState->display, windowState->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
	}
}

fpl_platform_api bool fplWindowUpdate() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	fplClearEvents();

	// Dont like this, maybe a callback would be better?
#if defined(FPL_PLATFORM_LINUX)
	fpl__LinuxAppState *linuxAppState = &appState->plinux;
	fpl__LinuxPollGameControllers(&appState->currentSettings, &linuxAppState->controllersState);
#endif

	while(x11Api->XPending(windowState->display)) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		fpl__X11HandleEvent(&appState->x11, appState, &ev);
	}
	x11Api->XFlush(windowState->display);
	result = appState->window.isRunning;
	return(result);
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	// @IMPLEMENT(final): X11 fplSetWindowCursorEnabled
}

fpl_platform_api bool fplGetWindowArea(fplWindowSize *outSize) {
	FPL__CheckArgumentNull(outSize, false);
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
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
	FPL__CheckPlatformNoRet();
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
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	// https://stackoverflow.com/questions/10897503/opening-a-fullscreen-opengl-window
	XEvent xev = FPL_ZERO_INIT;
	xev.type = ClientMessage;
	xev.xclient.window = windowState->window;
	xev.xclient.message_type = windowState->netWMState;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = value ? 1 : 0;
	xev.xclient.data.l[1] = windowState->netWMStateFullscreen;
	xev.xclient.data.l[2] = 0;
	bool result = x11Api->XSendEvent(windowState->display, windowState->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev) != 0;
	if(result) {
		appState->currentSettings.window.isFullscreen = value;
	}
	return(result);
}

fpl_platform_api bool fplIsWindowFullscreen() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFullscreen;
	return(result);
}

fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos) {
	FPL__CheckArgumentNull(outPos, false);
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
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
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	x11Api->XMoveWindow(windowState->display, windowState->window, left, top);
}

fpl_platform_api void fplSetWindowAnsiTitle(const char *ansiTitle) {
	// @TODO(final): Setting the window title on X11 works, but it wont be set for the icon in the bars
	// In gnome/ubuntu the icon title is always "Unbekannt" on my german environment.

	FPL__CheckArgumentNullNoRet(ansiTitle);
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	char nameBuffer[256];
	fplCopyAnsiString(ansiTitle, nameBuffer, FPL_ARRAYCOUNT(nameBuffer));

	x11Api->XChangeProperty(windowState->display, windowState->window,
							windowState->netWMName, windowState->utf8String, 8,
							PropModeReplace,
							(unsigned char*)ansiTitle, (int)fplGetAnsiStringLength(ansiTitle));

	x11Api->XChangeProperty(windowState->display, windowState->window,
							windowState->netWMIconName, windowState->utf8String, 8,
							PropModeReplace,
							(unsigned char*)ansiTitle, (int)fplGetAnsiStringLength(ansiTitle));

	x11Api->XFlush(windowState->display);
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

fpl_platform_api bool fplGetKeyboardState(fplKeyboardState *outState) {
	FPL__CheckPlatform(false);
	FPL__CheckArgumentNull(outState, false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	char keysReturn[32];
	bool result = false;
	if(x11Api->XQueryKeymap(windowState->display, keysReturn) == 0) {
		FPL_CLEAR_STRUCT(outState);
		for(int i = 0; i < 32; ++i) {
			for(int bit = 0; bit < 8; ++bit) {
				uint64_t keyCode = (uint64_t)(i * bit);
				int value = (keysReturn[i] >> bit) & 0x01;
				outState->keyStatesRaw[keyCode] = (value == 1);
				fplKey mappedKey = fpl__GetMappedKey(&appState->window, keyCode);
				outState->buttonStatesMapped[(int)mappedKey] = (value == 1) ? fplButtonState_Press : fplButtonState_Release;
			}
		}
		result = true;
	}
	return(result);
}

fpl_platform_api void fplUpdateGameControllers() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
#if defined(FPL_PLATFORM_LINUX)
	fpl__LinuxAppState *linuxAppState = &appState->plinux;
	fpl__LinuxPollGameControllers(&appState->currentSettings, &linuxAppState->controllersState);
#endif
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
#   include <locale.h> // setlocale
#	include <sys/eventfd.h> // eventfd
#	include <sys/epoll.h> // epoll_create, epoll_ctl, epoll_wait
#	include <sys/select.h> // select
#	include <sys/utsname.h> // uname
#	include <linux/joystick.h> // js_event, axis_state, etc.

fpl_internal void fpl__LinuxReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
#if defined(FPL_ENABLE_WINDOW)
	fpl__LinuxFreeGameControllers(&appState->plinux.controllersState);
#endif
}

fpl_internal bool fpl__LinuxInitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	setlocale(LC_ALL, "");
	return true;
}

//
// Linux OS
//
fpl_internal void fpl__ParseVersionString(const char *versionStr, fplVersionInfo *versionInfo) {
	fplCopyAnsiString(versionStr, versionInfo->fullName, FPL_ARRAYCOUNT(versionInfo->fullName));
	if(versionStr != fpl_null) {
		const char *p = versionStr;
		for(int i = 0; i < 4; ++i) {
			const char *digitStart = p;
			while(isdigit(*p)) {
				++p;
			}
			size_t len = p - digitStart;
			if(len <= FPL_ARRAYCOUNT(versionInfo->values[i])) {
				fplCopyAnsiStringLen(digitStart, len, versionInfo->values[i], FPL_ARRAYCOUNT(versionInfo->values[i]));
			} else {
				versionInfo->values[i][0] = 0;
			}
			if(*p != '.' && *p != '-') break;
			++p;
		}
	}
}

fpl_platform_api bool fplGetOperatingSystemInfos(fplOSInfos *outInfos) {
	bool result = false;
	struct utsname nameInfos;
	if(uname(&nameInfos) == 0) {
		const char *kernelName = nameInfos.sysname;
		const char *kernelVersion = nameInfos.release;
		const char *systemName = nameInfos.version;
		fplCopyAnsiString(kernelName, outInfos->kernelName, FPL_ARRAYCOUNT(outInfos->kernelName));
		fplCopyAnsiString(systemName, outInfos->systemName, FPL_ARRAYCOUNT(outInfos->systemName));
		fpl__ParseVersionString(kernelVersion, &outInfos->kernelVersion);

		// @TODO(final): Get linux distro version into systemVersion
		// cat /etc/os-release

		result = true;
	}
	return(result);
}

fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, const size_t maxNameBufferLen) {
	FPL__CheckArgumentNull(nameBuffer, false);
	FPL__CheckArgumentZero(maxNameBufferLen, false);
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	bool result = false;
	if(pw != fpl_null) {
		fplCopyAnsiString(pw->pw_name, nameBuffer, maxNameBufferLen);
		result = true;
	}
	return(result);
}

//
// Linux Input
//
#if defined(FPL_ENABLE_WINDOW)
fpl_internal void fpl__LinuxFreeGameControllers(fpl__LinuxGameControllersState *controllersState) {
	for(int controllerIndex = 0; controllerIndex < FPL_ARRAYCOUNT(controllersState->controllers); ++controllerIndex) {
		fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
		if(controller->fd > 0) {
			close(controller->fd);
			controller->fd = 0;
		}
	}
}

fpl_internal_inline float fpl__LinuxJoystickProcessStickValue(const int16_t value, const int16_t deadZoneThreshold) {
	float result = 0;
	if(value < -deadZoneThreshold) {
		result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	} else if(value > deadZoneThreshold) {
		result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}
	return(result);
}

fpl_internal void fpl__LinuxPushGameControllerStateUpdateEvent(const struct js_event *event, fplGamepadState *padState) {
	fplGamepadButton *buttonMappingTable[12] = FPL_ZERO_INIT;
	buttonMappingTable[0] = &padState->actionA;
	buttonMappingTable[1] = &padState->actionB;
	buttonMappingTable[2] = &padState->actionX;
	buttonMappingTable[3] = &padState->actionY;
	buttonMappingTable[4] = &padState->leftShoulder;
	buttonMappingTable[5] = &padState->rightShoulder;
	buttonMappingTable[6] = &padState->back;
	buttonMappingTable[7] = &padState->start;
	// 8 = XBox-Button
	buttonMappingTable[9] = &padState->leftThumb;
	buttonMappingTable[10] = &padState->rightThumb;

	const int16_t deadZoneThresholdLeftStick = 5000;
	const int16_t deadZoneThresholdRightStick = 5000;

	switch(event->type & ~JS_EVENT_INIT) {
		case JS_EVENT_AXIS:
		{
			switch(event->number) {
				// Left stick
				case 0:
				{
					padState->leftStickX = fpl__LinuxJoystickProcessStickValue(event->value, deadZoneThresholdLeftStick);
				} break;
				case 1:
				{
					padState->leftStickY = fpl__LinuxJoystickProcessStickValue(-event->value, deadZoneThresholdLeftStick);
				} break;

				// Right stick
				case 3:
				{
					padState->rightStickX = fpl__LinuxJoystickProcessStickValue(event->value, deadZoneThresholdRightStick);
				} break;
				case 4:
				{
					padState->rightStickY = fpl__LinuxJoystickProcessStickValue(-event->value, deadZoneThresholdRightStick);
				} break;

				// Left/right trigger
				case 2:
				{
					padState->leftTrigger = (float)((event->value + 32768) >> 8) / 255.0f;
				} break;
				case 5:
				{
					padState->rightTrigger = (float)((event->value + 32768) >> 8) / 255.0f;
				} break;

				// DPad X-Axis
				case 6:
				{
					if(event->value == -32767) {
						padState->dpadLeft.isDown = true;
						padState->dpadRight.isDown = false;
					} else if(event->value == 32767) {
						padState->dpadLeft.isDown = false;
						padState->dpadRight.isDown = true;
					} else {
						padState->dpadLeft.isDown = false;
						padState->dpadRight.isDown = false;
					}
				} break;

				// DPad Y-Axis
				case 7:
				{
					if(event->value == -32767) {
						padState->dpadUp.isDown = true;
						padState->dpadDown.isDown = false;
					} else if(event->value == 32767) {
						padState->dpadUp.isDown = false;
						padState->dpadDown.isDown = true;
					} else {
						padState->dpadUp.isDown = false;
						padState->dpadDown.isDown = false;
					}
				} break;

				default:
					break;
			}
		} break;

		case JS_EVENT_BUTTON:
		{
			if((event->number >= 0) && (event->number < FPL_ARRAYCOUNT(buttonMappingTable))) {
				fplGamepadButton *mappedButton = buttonMappingTable[event->number];
				if(mappedButton != fpl_null) {
					mappedButton->isDown = event->value != 0;
				}
			}
		} break;

		default:
			break;
	}
}

fpl_internal void fpl__LinuxPollGameControllers(const fplSettings *settings, fpl__LinuxGameControllersState *controllersState) {
	// https://github.com/underdoeg/ofxGamepad
	// https://github.com/elanthis/gamepad
	// https://gist.github.com/jasonwhite/c5b2048c15993d285130
	// https://github.com/Tasssadar/libenjoy/blob/master/src/libenjoy_linux.c

	if((controllersState->lastCheckTime == 0) || ((fplGetTimeInMillisecondsLP() - controllersState->lastCheckTime) >= settings->input.controllerDetectionFrequency)) {
		controllersState->lastCheckTime = fplGetTimeInMillisecondsLP();

		//
		// Detect new controllers
		//
		const char *deviceNames[] = {
			"/dev/input/js0",
		};
		for(int deviceNameIndex = 0; deviceNameIndex < FPL_ARRAYCOUNT(deviceNames); ++deviceNameIndex) {
			const char *deviceName = deviceNames[deviceNameIndex];
			bool alreadyFound = false;
			int freeIndex = -1;
			for(uint32_t controllerIndex = 0; controllerIndex < FPL_ARRAYCOUNT(controllersState->controllers); ++controllerIndex) {
				fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
				if((controller->fd > 0) && fplIsStringEqual(deviceName, controller->deviceName)) {
					alreadyFound = true;
					break;
				}
				if(controller->fd == 0) {
					if(freeIndex == -1) {
						freeIndex = controllerIndex;
					}
				}
			}
			if(!alreadyFound && freeIndex >= 0) {
				int fd = open(deviceName, O_RDONLY);
				if(fd < 0) {
					FPL_LOG_ERROR(FPL__MODULE_LINUX, "Failed opening joystick device '%s'", deviceName);
					continue;
				}
				uint8_t numAxis = 0;
				uint8_t numButtons = 0;
				ioctl(fd, JSIOCGAXES, &numAxis);
				ioctl(fd, JSIOCGBUTTONS, &numButtons);
				if(numAxis == 0 || numButtons == 0) {
					FPL_LOG_ERROR(FPL__MODULE_LINUX, "Joystick device '%s' does not have enough buttons/axis to map to a XInput controller!", deviceName);
					close(fd);
					continue;
				}
				fpl__LinuxGameController *controller = controllersState->controllers + freeIndex;
				FPL_CLEAR_STRUCT(controller);
				controller->fd = fd;
				controller->axisCount = numAxis;
				controller->buttonCount = numButtons;
				fplCopyAnsiString(deviceName, controller->deviceName, FPL_ARRAYCOUNT(controller->deviceName));
				ioctl(fd, JSIOCGNAME(FPL_ARRAYCOUNT(controller->displayName)), controller->displayName);
				fcntl(fd, F_SETFL, O_NONBLOCK);

				// Connected
				fplEvent ev = FPL_ZERO_INIT;
				ev.type = fplEventType_Gamepad;
				ev.gamepad.type = fplGamepadEventType_Connected;
				ev.gamepad.deviceIndex = (uint32_t)freeIndex;
				fpl__PushEvent(&ev);
			}
		}
	}


	for(uint32_t controllerIndex = 0; controllerIndex < FPL_ARRAYCOUNT(controllersState->controllers); ++controllerIndex) {
		fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
		if(controller->fd > 0) {
			// Update button/axis state
			struct js_event event;
			for(;;) {
				errno = 0;
				if(read(controller->fd, &event, sizeof(event)) < 0) {
					if(errno == ENODEV) {
						close(controller->fd);
						controller->fd = 0;

						// Disconnected
						fplEvent ev = FPL_ZERO_INIT;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Disconnected;
						ev.gamepad.deviceIndex = controllerIndex;
						fpl__PushEvent(&ev);
					}
					break;
				}
				fpl__LinuxPushGameControllerStateUpdateEvent(&event, &controller->state);
			}

			if(controller->fd > 0) {
				fplEvent ev = FPL_ZERO_INIT;
				ev.type = fplEventType_Gamepad;
				ev.gamepad.type = fplGamepadEventType_StateChanged;
				ev.gamepad.deviceIndex = 0;
				ev.gamepad.state = controller->state;
				fpl__PushEvent(&ev);
			}
		}
	}
}
#endif // FPL_ENABLE_WINDOW

//
// Linux Threading
//
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	FPL__CheckArgumentNull(signal, false);
	if(signal->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal '%p' is already valid", signal);
		return false;
	}
	int linuxEventHandle = eventfd((initialValue == fplSignalValue_Set) ? 1 : 0, EFD_CLOEXEC);
	if(linuxEventHandle == -1) {
		FPL_ERROR(FPL__MODULE_THREADING, "Failed initializing signal '%p'", signal);
		return false;
	}
	FPL_CLEAR_STRUCT(signal);
	signal->isValid = true;
	signal->internalHandle.linuxEventHandle = linuxEventHandle;
	return(true);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	if(signal != fpl_null && signal->isValid) {
		close(signal->internalHandle.linuxEventHandle);
		FPL_CLEAR_STRUCT(signal);
	}
}

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signal, false);
	if(!signal->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal '%p' is not valid", signal);
		return(false);
	}
	int ev = signal->internalHandle.linuxEventHandle;
	if(timeout == FPL_TIMEOUT_INFINITE) {
		uint64_t value;
		read(ev, &value, sizeof(value));
		return true;
	} else {
		fd_set f;
		FD_ZERO(&f);
		FD_SET(ev, &f);
		struct timeval t = { 0, timeout * 1000 };
		int selectResult = select(1, &f, NULL, NULL, &t);
		if(selectResult == 0) {
			// Timeout
			return false;
		} else if(selectResult == -1) {
			// Error
			return false;
		} else {
			return true;
		}
	}
}

fpl_internal bool fpl__LinuxSignalWaitForMultiple(fplSignalHandle *signals[], const uint32_t minCount, const uint32_t maxCount, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signals, false);
	FPL__CheckArgumentMax(maxCount, FPL__MAX_SIGNAL_COUNT, false);
	for(uint32_t index = 0; index < maxCount; ++index) {
		fplSignalHandle *signal = signals[index];
		if(signal == fpl_null) {
			FPL_ERROR(FPL__MODULE_THREADING, "Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if(!signal->isValid) {
			FPL_ERROR(FPL__MODULE_THREADING, "Signal '%p' for index '%d' is not valid", signal, index);
			return false;
		}
	}

	int e = epoll_create(maxCount);
	FPL_ASSERT(e != 0);

	// Register events and map each to the array index
	struct epoll_event events[FPL__MAX_SIGNAL_COUNT];
	for(int i = 0; i < maxCount; i++) {
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
	while(waiting > 0) {
		int ret = epoll_wait(e, revent, waiting, t);
		if(ret == 0) {
			if(minCount == maxCount) {
				eventsResult = -1;
			}
			break;
		}
		for(int i = 0; i < ret; i++) {
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
	FPL__CheckArgumentNull(signal, false);
	if(!signal->isValid) {
		FPL_ERROR(FPL__MODULE_THREADING, "Signal '%p' is not valid", signal);
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
	FPL__CheckArgumentNull(destBuffer, fpl_null);
	FPL__CheckArgumentZero(maxDestBufferLen, fpl_null);
	char *result = fpl_null;
	FILE *fileHandle = fopen("/proc/cpuinfo", "rb");
	if(fileHandle != fpl_null) {
		char buffer[256];
		char line[256];
		const size_t maxBufferSize = FPL_ARRAYCOUNT(buffer);
		size_t readSize = maxBufferSize;
		size_t readPos = 0;
		bool found = false;
		int bytesRead = 0;
		while((bytesRead = fread(&buffer[readPos], readSize, 1, fileHandle)) > 0) {
			char *lastP = &buffer[0];
			char *p = &buffer[0];
			while(*p) {
				if(*p == '\n') {
					int len = p - lastP;
					FPL_ASSERT(len > 0);
					if(fplIsStringEqualLen(lastP, 10, "model name", 10)) {
						fplCopyAnsiStringLen(lastP, len, line, FPL_ARRAYCOUNT(line));
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
			int remaining = &buffer[maxBufferSize] - lastP;
			FPL_ASSERT(remaining >= 0);
			if(remaining > 0) {
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
				fplCopyAnsiString(p, destBuffer, maxDestBufferLen);
				result = destBuffer;
			}
		}
		fclose(fileHandle);
	}
	return(result);
}

fpl_platform_api bool fplGetRunningMemoryInfos(fplMemoryInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);
	bool result = false;

	// https://git.i-scream.org/?p=libstatgrab.git;a=blob;f=src/libstatgrab/memory_stats.c;h=a6f6fb926b72d3b691848202e397e3db58255648;hb=HEAD

	return(result);
}

fpl_platform_api fplArchType fplGetRunningArchitecture() {
	fplArchType result = fplArchType_Unknown;
	struct utsname nameInfos;
	if(uname(&nameInfos) == 0) {
		const char *machineName = nameInfos.machine;
		if(fplIsStringEqual("x86_64", machineName) || fplIsStringEqual("amd64", machineName)) {
			result = fplArchType_x86_64;
		} else {
			// @TODO(final): Detect other running CPU architectures (x64, Arm32, Arm64)
			result = fplArchType_x86;
		}
	}
	return(result);
}

//
// Linux Paths
//
fpl_platform_api char *fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	FPL__CheckArgumentZero(maxDestLen, fpl_null);
	char buf[1024];
	if(readlink("/proc/self/exe", buf, FPL_ARRAYCOUNT(buf) - 1)) {
		int len = fplGetAnsiStringLength(buf);
		char *lastP = buf + (len - 1);
		char *p = lastP;
		while(p != buf) {
			if(*p == '/') {
				len = (lastP - buf) + 1;
				break;
			}
			--p;
		}
		size_t requiredLen = len + 1;
		FPL__CheckArgumentMin(maxDestLen, requiredLen, fpl_null);
		char *result = fplCopyAnsiStringLen(buf, len, destPath, maxDestLen);
		return(result);
	}
	return fpl_null;
}

fpl_platform_api char *fplGetHomePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(destPath, fpl_null);
	FPL__CheckArgumentZero(maxDestLen, fpl_null);
	const char *homeDir = getenv("HOME");
	if(homeDir == fpl_null) {
		int userId = getuid();
		struct passwd *userPwd = getpwuid(userId);
		homeDir = userPwd->pw_dir;
	}
	char *result = fplCopyAnsiString(homeDir, destPath, maxDestLen);
	return(result);
}

fpl_internal void fpl__LinuxLocaleToISO639(const char *source, char *target, const size_t maxTargetLen) {
	fplCopyAnsiString(source, target, maxTargetLen);
	char *p = target;
	while(*p) {
		if(*p == '_') {
			*p = '-';
		} else if(*p == '.') {
			*p = '\0';
			break;
		}
		++p;
	}
}

// Linux internationalisation
fpl_platform_api bool fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	bool result = true;
	char *locale = setlocale(LC_CTYPE, NULL);
	fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
	return(result);
}

fpl_platform_api bool fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	bool result = true;
	char *locale = setlocale(LC_ALL, NULL);
	fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
	return(result);
}

fpl_platform_api bool fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, false);
	bool result = true;
	char *locale = setlocale(LC_ALL, NULL);
	fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
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

fpl_platform_api bool fplGetCurrentUsername(char *nameBuffer, const size_t maxNameBufferLen) {
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
	FPL__CheckArgumentNull(destBuffer, fpl_null);
	FPL__CheckArgumentZero(maxDestBufferLen, fpl_null);
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
	FPL__CheckArgumentNull(destPath, fpl_null);
	FPL__CheckArgumentZero(maxDestLen, fpl_null);
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
#define FPL_WGL_ALPHA_BITS_ARB 0x201B
#define FPL_WGL_DEPTH_BITS_ARB 0x2022
#define FPL_WGL_STENCIL_BITS_ARB 0x2023
#define FPL_WGL_FULL_ACCELERATION_ARB 0x2027
#define FPL_WGL_SWAP_EXCHANGE_ARB 0x2028
#define FPL_WGL_TYPE_RGBA_ARB 0x202B
#define FPL_WGL_SAMPLE_BUFFERS_ARB 0x2041
#define FPL_WGL_SAMPLES_ARB 0x2042

#define FPL__FUNC_WGL_wglMakeCurrent(name) BOOL WINAPI name(HDC deviceContext, HGLRC renderingContext)
typedef FPL__FUNC_WGL_wglMakeCurrent(fpl__win32_func_wglMakeCurrent);
#define FPL__FUNC_WGL_wglGetProcAddress(name) PROC WINAPI name(LPCSTR procedure)
typedef FPL__FUNC_WGL_wglGetProcAddress(fpl__win32_func_wglGetProcAddress);
#define FPL__FUNC_WGL_wglDeleteContext(name) BOOL WINAPI name(HGLRC renderingContext)
typedef FPL__FUNC_WGL_wglDeleteContext(fpl__win32_func_wglDeleteContext);
#define FPL__FUNC_WGL_wglCreateContext(name) HGLRC WINAPI name(HDC deviceContext)
typedef FPL__FUNC_WGL_wglCreateContext(fpl__win32_func_wglCreateContext);

#define FPL__FUNC_WGL_wglChoosePixelFormatARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
typedef FPL__FUNC_WGL_wglChoosePixelFormatARB(fpl__win32_func_wglChoosePixelFormatARB);
#define FPL__FUNC_WGL_wglCreateContextAttribsARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
typedef FPL__FUNC_WGL_wglCreateContextAttribsARB(fpl__win32_func_wglCreateContextAttribsARB);
#define FPL__FUNC_WGL_wglSwapIntervalEXT(name) BOOL WINAPI name(int interval)
typedef FPL__FUNC_WGL_wglSwapIntervalEXT(fpl__win32_func_wglSwapIntervalEXT);

typedef struct fpl__Win32OpenGLApi {
	HMODULE openglLibrary;
	fpl__win32_func_wglMakeCurrent *wglMakeCurrent;
	fpl__win32_func_wglGetProcAddress *wglGetProcAddress;
	fpl__win32_func_wglDeleteContext *wglDeleteContext;
	fpl__win32_func_wglCreateContext *wglCreateContext;
	fpl__win32_func_wglChoosePixelFormatARB *wglChoosePixelFormatARB;
	fpl__win32_func_wglCreateContextAttribsARB *wglCreateContextAttribsARB;
	fpl__win32_func_wglSwapIntervalEXT *wglSwapIntervalEXT;
} fpl__Win32OpenGLApi;

fpl_internal void fpl__Win32UnloadVideoOpenGLApi(fpl__Win32OpenGLApi *api) {
	if(api->openglLibrary != fpl_null) {
		FreeLibrary(api->openglLibrary);
	}
	FPL_CLEAR_STRUCT(api);
}

fpl_internal bool fpl__Win32LoadVideoOpenGLApi(fpl__Win32OpenGLApi *api) {
	const char *openglLibraryName = "opengl32.dll";
	api->openglLibrary = LoadLibraryA("opengl32.dll");
	if(api->openglLibrary == fpl_null) {
		FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed loading opengl library '%s'", openglLibraryName);
		return false;
	}
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_VIDEO_OPENGL, api->openglLibrary, openglLibraryName, api->wglGetProcAddress, fpl__win32_func_wglGetProcAddress, "wglGetProcAddress");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_VIDEO_OPENGL, api->openglLibrary, openglLibraryName, api->wglCreateContext, fpl__win32_func_wglCreateContext, "wglCreateContext");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_VIDEO_OPENGL, api->openglLibrary, openglLibraryName, api->wglDeleteContext, fpl__win32_func_wglDeleteContext, "wglDeleteContext");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_VIDEO_OPENGL, api->openglLibrary, openglLibraryName, api->wglMakeCurrent, fpl__win32_func_wglMakeCurrent, "wglMakeCurrent");
	return true;
}

typedef struct fpl__Win32VideoOpenGLState {
	HGLRC renderingContext;
	fpl__Win32OpenGLApi api;
} fpl__Win32VideoOpenGLState;

fpl_internal LRESULT CALLBACK fpl__Win32TemporaryWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	switch(message) {
		case WM_CLOSE:
			wapi->user.PostQuitMessage(0);
			break;
		default:
			return wapi->user.DefWindowProcA(hWnd, message, wParam, lParam);
	}
	return 0;
}

fpl_internal bool fpl__Win32PreSetupWindowForOpenGL(fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fplVideoSettings *videoSettings) {
	const fpl__Win32Api *wapi = &appState->winApi;

	windowState->pixelFormat = 0;

	if(videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) {
		fpl__Win32OpenGLApi glApi;
		if(fpl__Win32LoadVideoOpenGLApi(&glApi)) {
			// Register temporary window class
			WNDCLASSEXA windowClass = FPL_ZERO_INIT;
			windowClass.cbSize = sizeof(windowClass);
			windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			windowClass.lpfnWndProc = fpl__Win32TemporaryWindowProc;
			windowClass.hInstance = GetModuleHandleA(fpl_null);
			windowClass.hCursor = fpl__win32_LoadCursor(fpl_null, IDC_ARROW);
			windowClass.lpszClassName = "FPL_Temp_GL_Window";
			if(wapi->user.RegisterClassExA(&windowClass)) {
				// Create temporary window
				HWND tempWindowHandle = wapi->user.CreateWindowExA(0, windowClass.lpszClassName, "FPL Temp GL Window", 0, 0, 0, 1, 1, fpl_null, fpl_null, windowClass.hInstance, fpl_null);
				if(tempWindowHandle != fpl_null) {
					// Get temporary device context
					HDC tempDC = wapi->user.GetDC(tempWindowHandle);
					if(tempDC != fpl_null) {
						// Get legacy pixel format
						PIXELFORMATDESCRIPTOR fakePFD = FPL_ZERO_INIT;
						fakePFD.nSize = sizeof(fakePFD);
						fakePFD.nVersion = 1;
						fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
						fakePFD.iPixelType = PFD_TYPE_RGBA;
						fakePFD.cColorBits = 32;
						fakePFD.cAlphaBits = 8;
						fakePFD.cDepthBits = 24;
						int fakePFDID = wapi->gdi.ChoosePixelFormat(tempDC, &fakePFD);
						if(fakePFDID != 0) {
							if(wapi->gdi.SetPixelFormat(tempDC, fakePFDID, &fakePFD)) {
								// Create temporary rendering context
								HGLRC tempCtx = glApi.wglCreateContext(tempDC);
								if(tempCtx != fpl_null) {
									if(glApi.wglMakeCurrent(tempDC, tempCtx)) {
										glApi.wglChoosePixelFormatARB = (fpl__win32_func_wglChoosePixelFormatARB *)glApi.wglGetProcAddress("wglChoosePixelFormatARB");
										if(glApi.wglChoosePixelFormatARB != fpl_null) {
											int multisampleCount = (int)videoSettings->graphics.opengl.multiSamplingCount;
											const int pixelAttribs[] = {
												FPL_WGL_DRAW_TO_WINDOW_ARB, 1,
												FPL_WGL_SUPPORT_OPENGL_ARB, 1,
												FPL_WGL_DOUBLE_BUFFER_ARB, 1,
												FPL_WGL_PIXEL_TYPE_ARB, FPL_WGL_TYPE_RGBA_ARB,
												FPL_WGL_ACCELERATION_ARB, FPL_WGL_FULL_ACCELERATION_ARB,
												FPL_WGL_COLOR_BITS_ARB, 32,
												FPL_WGL_ALPHA_BITS_ARB, 8,
												FPL_WGL_DEPTH_BITS_ARB, 24,
												FPL_WGL_STENCIL_BITS_ARB, 8,
												FPL_WGL_SAMPLE_BUFFERS_ARB, (multisampleCount > 0) ? 1 : 0,
												FPL_WGL_SAMPLES_ARB, multisampleCount,
												0
											};
											int pixelFormat;
											UINT numFormats;
											if(glApi.wglChoosePixelFormatARB(tempDC, pixelAttribs, NULL, 1, &pixelFormat, &numFormats)) {
												windowState->pixelFormat = pixelFormat;
											}
										}
										glApi.wglMakeCurrent(fpl_null, fpl_null);
									}
									glApi.wglDeleteContext(tempCtx);
								}
							}
						}
						wapi->user.ReleaseDC(tempWindowHandle, tempDC);
					}
					wapi->user.DestroyWindow(tempWindowHandle);
				}
			}
			fpl__Win32UnloadVideoOpenGLApi(&glApi);
		}
	}
	return(true);
}

fpl_internal bool fpl__Win32PostSetupWindowForOpenGL(fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fplVideoSettings *videoSettings) {
	const fpl__Win32Api *wapi = &appState->winApi;

	//
	// Prepare window for OpenGL
	//
	HDC deviceContext = windowState->deviceContext;
	HWND handle = windowState->windowHandle;
	PIXELFORMATDESCRIPTOR pfd = FPL_ZERO_INIT;

	// We may got a pixel format from the pre setup
	bool pixelFormatSet = false;
	if(windowState->pixelFormat != 0) {
		wapi->gdi.DescribePixelFormat(deviceContext, windowState->pixelFormat, sizeof(pfd), &pfd);
		pixelFormatSet = wapi->gdi.SetPixelFormat(deviceContext, windowState->pixelFormat, &pfd) == TRUE;
		if(!pixelFormatSet) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed setting Pixelformat '%d' from pre setup", windowState->pixelFormat);
		}
	}
	if(!pixelFormatSet) {
		FPL_CLEAR_STRUCT(&pfd);
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cAlphaBits = 8;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int pixelFormat = wapi->gdi.ChoosePixelFormat(deviceContext, &pfd);
		if(!pixelFormat) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
			return false;
		}
		wapi->gdi.DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);
		if(!wapi->gdi.SetPixelFormat(deviceContext, pixelFormat, &pfd)) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
			return false;
		}
	}
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
	if(!legacyRenderingContext) {
		FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed creating Legacy OpenGL Rendering Context for DC '%x')", deviceContext);
		return false;
	}
	if(!glapi->wglMakeCurrent(deviceContext, legacyRenderingContext)) {
		FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')", deviceContext, legacyRenderingContext);
		glapi->wglDeleteContext(legacyRenderingContext);
		return false;
	}

	// Load WGL Extensions
	glapi->wglSwapIntervalEXT = (fpl__win32_func_wglSwapIntervalEXT *)glapi->wglGetProcAddress("wglSwapIntervalEXT");
	glapi->wglChoosePixelFormatARB = (fpl__win32_func_wglChoosePixelFormatARB *)glapi->wglGetProcAddress("wglChoosePixelFormatARB");
	glapi->wglCreateContextAttribsARB = (fpl__win32_func_wglCreateContextAttribsARB *)glapi->wglGetProcAddress("wglCreateContextAttribsARB");

	// Disable legacy context
	glapi->wglMakeCurrent(fpl_null, fpl_null);

	HGLRC activeRenderingContext;
	if(videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if(!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			return false;
		}
		if(glapi->wglChoosePixelFormatARB == fpl_null) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
			return false;
		}
		if(glapi->wglCreateContextAttribsARB == fpl_null) {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			return false;
		}

		int profile = 0;
		int flags = 0;
		if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "No opengl compability profile selected, please specific Core fplOpenGLCompabilityFlags_Core or fplOpenGLCompabilityFlags_Compability");
			return false;
		}
		if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
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
		if(flags > 0) {
			contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}

		// Create modern opengl rendering context
		HGLRC modernRenderingContext = glapi->wglCreateContextAttribsARB(deviceContext, 0, contextAttribList);
		if(modernRenderingContext) {
			if(!glapi->wglMakeCurrent(deviceContext, modernRenderingContext)) {
				FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

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
			FPL_ERROR(FPL__MODULE_VIDEO_OPENGL, "Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

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
	if(glapi->wglSwapIntervalEXT != fpl_null) {
		int swapInterval = videoSettings->isVSync ? 1 : 0;
		glapi->wglSwapIntervalEXT(swapInterval);
	}

	return true;
}

fpl_internal void fpl__Win32ReleaseVideoOpenGL(fpl__Win32VideoOpenGLState *glState) {
	const fpl__Win32OpenGLApi *glapi = &glState->api;
	if(glState->renderingContext) {
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
#ifndef __gl_h_
typedef uint8_t GLubyte;
#endif
#ifndef GLX_H
typedef XID GLXDrawable;
typedef XID GLXWindow;
typedef void GLXContext_Void;
typedef GLXContext_Void* GLXContext;
typedef void GLXFBConfig_Void;
typedef GLXFBConfig_Void* GLXFBConfig;

#define GLX_RGBA 4
#define GLX_DOUBLEBUFFER 5
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_SAMPLE_BUFFERS 0x186a0
#define GLX_SAMPLES 0x186a1

#define GLX_X_VISUAL_TYPE 0x22
#define GLX_TRUE_COLOR 0x8002
#define GLX_RGBA_TYPE 0x8014
#endif

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
	if(api->libHandle != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Unload Api (Library '%p')", api->libHandle);
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
	for(uint32_t index = 0; index < FPL_ARRAYCOUNT(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Load GLX Api from Library: %s", libName);
		void *libHandle = api->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if(libHandle != fpl_null) {
			FPL_LOG_DEBUG(FPL__MODULE_GLX, "Library Found: '%s', Resolving Procedures", libName);
			do {
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXQueryVersion, fpl__func_glx_glXQueryVersion, "glXQueryVersion");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXChooseVisual, fpl__func_glx_glXChooseVisual, "glXChooseVisual");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXCreateContext, fpl__func_glx_glXCreateContext, "glXCreateContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXDestroyContext, fpl__func_glx_glXDestroyContext, "glXDestroyContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXCreateNewContext, fpl__func_glx_glXCreateNewContext, "glXCreateNewContext");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXMakeCurrent, fpl__func_glx_glXMakeCurrent, "glXMakeCurrent");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXSwapBuffers, fpl__func_glx_glXSwapBuffers, "glXSwapBuffers");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXGetProcAddress, fpl__func_glx_glXGetProcAddress, "glXGetProcAddress");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXChooseFBConfig, fpl__func_glx_glXChooseFBConfig, "glXChooseFBConfig");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXGetFBConfigs, fpl__func_glx_glXGetFBConfigs, "glXGetFBConfigs");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXGetVisualFromFBConfig, fpl__func_glx_glXGetVisualFromFBConfig, "glXGetVisualFromFBConfig");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXGetFBConfigAttrib, fpl__func_glx_glXGetFBConfigAttrib, "glXGetFBConfigAttrib");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXCreateWindow, fpl__func_glx_glXCreateWindow, "glXCreateWindow");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXQueryExtension, fpl__func_glx_glXQueryExtension, "glXQueryExtension");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_GLX, libHandle, libName, api->glXQueryExtensionsString, fpl__func_glx_glXQueryExtensionsString, "glXQueryExtensionsString");
				result = true;
			} while(0);
			if(result) {
				FPL_LOG_DEBUG(FPL__MODULE_GLX, , "Successfully loaded GLX Api from Library '%s'", libName);
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

fpl_internal bool fpl__X11InitFrameBufferConfigVideoOpenGL(const fpl__X11Api *x11Api, const fpl__X11WindowState *windowState, const fplVideoSettings *videoSettings, fpl__X11VideoOpenGLState *glState) {
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Query GLX version for display '%p'", windowState->display);
	int major = 0, minor = 0;
	if(!glApi->glXQueryVersion(windowState->display, &major, &minor)) {
		FPL_LOG_ERROR(FPL__MODULE_GLX, "Failed querying GLX version for display '%p'", windowState->display);
		return false;
	}
	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully queried GLX version for display '%p': %d.%d", windowState->display, major, minor);

	// @NOTE(final): Required for AMD Drivers?

	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Query OpenGL extension on display '%p'", windowState->display);
	if(!glApi->glXQueryExtension(windowState->display, fpl_null, fpl_null)) {
		FPL_ERROR(FPL__MODULE_GLX, "OpenGL GLX Extension is not supported by the active display '%p'", windowState->display);
		return false;
	}

	const char *extensionString = glApi->glXQueryExtensionsString(windowState->display, windowState->screen);
	if(extensionString != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "OpenGL GLX extensions: %s", extensionString);
	}

	bool isModern = major > 1 || (major == 1 && minor >= 3);

	int attr[32] = FPL_ZERO_INIT;
	int attrIndex = 0;

	attr[attrIndex++] = GLX_X_VISUAL_TYPE;
	attr[attrIndex++] = GLX_TRUE_COLOR;

	if(!isModern) {
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

	attr[attrIndex++] = GLX_ALPHA_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex++] = GLX_DEPTH_SIZE;
	attr[attrIndex++] = 24;

	attr[attrIndex++] = GLX_STENCIL_SIZE;
	attr[attrIndex++] = 8;

	if(videoSettings->graphics.opengl.multiSamplingCount > 0) {
		attr[attrIndex++] = GLX_SAMPLE_BUFFERS;
		attr[attrIndex++] = 1;

		attr[attrIndex++] = GLX_SAMPLES;
		attr[attrIndex++] = (int)videoSettings->graphics.opengl.multiSamplingCount;
	}

	attr[attrIndex] = 0;

	if(isModern) {
		// Use frame buffer config approach (GLX >= 1.3)
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Get framebuffer configuration from display '%p' and screen '%d'", windowState->display, windowState->screen);
		int configCount = 0;
		GLXFBConfig *configs = glApi->glXChooseFBConfig(windowState->display, windowState->screen, attr, &configCount);
		if(configs == fpl_null || !configCount) {
			FPL_ERROR(FPL__MODULE_GLX, "No framebuffer configuration from display '%p' and screen '%d' found!", windowState->display, windowState->screen);
			glState->fbConfig = fpl_null;
			return false;
		}
		glState->fbConfig = configs[0];
		glState->visualInfo = fpl_null;
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got framebuffer configuration from display '%p' and screen '%d': %p", windowState->display, windowState->screen, glState->fbConfig);

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Release %d framebuffer configurations", configCount);
		x11Api->XFree(configs);
	} else {
		// Use choose visual (Old way)
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Choose visual from display '%p' and screen '%d'", windowState->display, windowState->screen);
		XVisualInfo *visualInfo = glApi->glXChooseVisual(windowState->display, windowState->screen, attr);
		if(visualInfo == fpl_null) {
			FPL_ERROR(FPL__MODULE_GLX, "No visual info for display '%p' and screen '%d' found!", windowState->display, windowState->screen);
			return false;
		}
		glState->visualInfo = visualInfo;
		glState->fbConfig = fpl_null;
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got visual info from display '%p' and screen '%d': %p", windowState->display, windowState->screen, glState->visualInfo);
	}

	return true;
}

fpl_internal bool fpl__X11SetPreWindowSetupForOpenGL(const fpl__X11Api *x11Api, const fpl__X11WindowState *windowState, const fpl__X11VideoOpenGLState *glState, fpl__X11PreWindowSetupResult *outResult) {
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	if(glState->fbConfig != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Get visual info from display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
		XVisualInfo *visualInfo = glApi->glXGetVisualFromFBConfig(windowState->display, glState->fbConfig);
		if(visualInfo == fpl_null) {
			FPL_ERROR(FPL__MODULE_GLX, "Failed getting visual info from display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
			return false;
		}
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got visual info from display '%p' and frame buffer config '%p': %p", windowState->display, glState->fbConfig, visualInfo);

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using visual: %p", visualInfo->visual);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using color depth: %d", visualInfo->depth);

		outResult->visual = visualInfo->visual;
		outResult->colorDepth = visualInfo->depth;

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Release visual info '%p'", visualInfo);
		x11Api->XFree(visualInfo);
	} else if(glState->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using existing visual info: %p", glState->visualInfo);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using visual: %p", glState->visualInfo->visual);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using color depth: %d", glState->visualInfo->depth);
		outResult->visual = glState->visualInfo->visual;
		outResult->colorDepth = glState->visualInfo->depth;
	} else {
		FPL_ERROR(FPL__MODULE_GLX, "No visual info or frame buffer config defined!");
		return false;
	}

	return true;
}

fpl_internal void fpl__X11ReleaseVideoOpenGL(const fpl__X11SubplatformState *subplatform, const fpl__X11WindowState *windowState, fpl__X11VideoOpenGLState *glState) {
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11VideoOpenGLApi *glApi = &glState->api;

	if(glState->isActiveContext) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Deactivate GLX rendering context for display '%p'", windowState->display);
		glApi->glXMakeCurrent(windowState->display, 0, fpl_null);
		glState->isActiveContext = false;
	}

	if(glState->context != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy GLX rendering context '%p' for display '%p'", glState->context, windowState->display);
		glApi->glXDestroyContext(windowState->display, glState->context);
		glState->context = fpl_null;
	}

	if(glState->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy visual info '%p' (Fallback)", glState->visualInfo);
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
	if(glState->fbConfig != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Create GLX legacy rendering context on display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
		legacyRenderingContext = glApi->glXCreateNewContext(windowState->display, glState->fbConfig, GLX_RGBA_TYPE, fpl_null, 1);
		if(!legacyRenderingContext) {
			FPL_ERROR(FPL__MODULE_GLX, "Failed creating GLX legacy rendering context on display '%p' and frame buffer config '%p'", windowState->display, glState->fbConfig);
			goto failed_x11_glx;
		}
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully created GLX legacy rendering context '%p' on display '%p' and frame buffer config '%p'", legacyRenderingContext, windowState->display, glState->fbConfig);
	} else if(glState->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Create GLX legacy rendering context on display '%p' and visual info '%p'", windowState->display, glState->visualInfo);
		legacyRenderingContext = glApi->glXCreateContext(windowState->display, glState->visualInfo, fpl_null, 1);
		if(!legacyRenderingContext) {
			FPL_ERROR(FPL__MODULE_GLX, "Failed creating GLX legacy rendering context on display '%p' and visual info '%p'", windowState->display, glState->visualInfo);
			goto failed_x11_glx;
		}
	} else {
		FPL_ERROR(FPL__MODULE_GLX, "No visual info or frame buffer config defined!");
		goto failed_x11_glx;
	}

	//
	// Activate legacy context
	//
	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Activate GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
	if(!glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext)) {
		FPL_ERROR(FPL__MODULE_GLX, "Failed activating GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
		goto failed_x11_glx;
	} else {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully activated GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, windowState->display, (int)windowState->window);
	}

	//
	// Load extensions
	//
	glApi->glXCreateContextAttribsARB = (fpl__func_glx_glXCreateContextAttribsARB *)glApi->glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

	// Disable legacy rendering context
	glApi->glXMakeCurrent(windowState->display, 0, fpl_null);

	GLXContext activeRenderingContext;

	if((videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) && (glState->fbConfig != fpl_null)) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if(!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			FPL_ERROR(FPL__MODULE_GLX, "You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			goto failed_x11_glx;
		}

		if(glApi->glXCreateContextAttribsARB == fpl_null) {
			FPL_ERROR(FPL__MODULE_GLX, "glXCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			goto failed_x11_glx;
		}

		int flags = 0;
		int profile = 0;
		if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL__GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL__GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			FPL_ERROR(FPL__MODULE_GLX, "No opengl compability profile selected, please specific Core OpenGLCompabilityFlags_Core or OpenGLCompabilityFlags_Compability");
			goto failed_x11_glx;
		}
		if(videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
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
		if(flags > 0) {
			contextAttribList[contextAttribIndex++] = FPL__GLX_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}
		contextAttribList[contextAttribIndex] = 0;

		GLXContext modernRenderingContext = glApi->glXCreateContextAttribsARB(windowState->display, glState->fbConfig, fpl_null, True, contextAttribList);
		if(!modernRenderingContext) {
			FPL_ERROR(FPL__MODULE_GLX, "Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags);

			// Fallback to legacy rendering context
			glApi->glXMakeCurrent(windowState->display, windowState->window, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		} else {
			if(!glApi->glXMakeCurrent(windowState->display, windowState->window, modernRenderingContext)) {
				FPL_ERROR(FPL__MODULE_GLX, "Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags);

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
	if(glState->visualInfo != fpl_null) {
		// If there is a cached visual info, get rid of it now - regardless of its result
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy visual info '%p'", glState->visualInfo);
		x11Api->XFree(glState->visualInfo);
		glState->visualInfo = fpl_null;
	}

	if(!result) {
		if(legacyRenderingContext) {
			glApi->glXDestroyContext(windowState->display, legacyRenderingContext);
		}
		fpl__X11ReleaseVideoOpenGL(subplatform, windowState, glState);
	}

	return (result);
}
#endif // FPL_ENABLE_VIDEO_OPENGL && FPL_SUBPLATFORM_X11

// ############################################################################
//
// > VIDEO_DRIVER_SOFTWARE_X11
//
// ############################################################################
#if defined(FPL_ENABLE_VIDEO_SOFTWARE) && defined(FPL_SUBPLATFORM_X11)
typedef struct fpl__X11VideoSoftwareState {
	GC graphicsContext;
	XImage *buffer;
} fpl__X11VideoSoftwareState;

fpl_internal bool fpl__X11SetPreWindowSetupForSoftware(const fpl__X11Api *x11Api, const fpl__X11WindowState *windowState, const fpl__X11VideoSoftwareState *softwareState, fpl__X11PreWindowSetupResult *outResult) {
	XVisualInfo vinfo = FPL_ZERO_INIT;
	if(!x11Api->XMatchVisualInfo(windowState->display, windowState->screen, 32, DirectColor, &vinfo)) {
		if(!x11Api->XMatchVisualInfo(windowState->display, windowState->screen, 24, DirectColor, &vinfo)) {
			/* No proper color depth available */
			x11Api->XCloseDisplay(windowState->display); /* Close X communication */
			FPL_ERROR(FPL__MODULE_X11, "No visual info found for either 32 or 24-bit colors!");
			return false;
		}
	}
	outResult->visual = vinfo.visual;
	outResult->colorDepth = vinfo.depth;
	return true;
}

fpl_internal void fpl__X11ReleaseVideoSoftware(const fpl__X11SubplatformState *subplatform, const fpl__X11WindowState *windowState, fpl__X11VideoSoftwareState *softwareState) {
	const fpl__X11Api *x11Api = &subplatform->api;

	if(softwareState->buffer != fpl_null) {
		// @NOTE(final): Dont use XDestroyImage here, as it points to the backbuffer memory directly - which is released later
		softwareState->buffer = fpl_null;
	}

	if(softwareState->graphicsContext != fpl_null) {
		softwareState->graphicsContext = fpl_null;
	}
}

fpl_internal bool fpl__X11InitVideoSoftware(const fpl__X11SubplatformState *subplatform, const fpl__X11WindowState *windowState, const fplVideoSettings *videoSettings, const fplVideoBackBuffer *backbuffer, fpl__X11VideoSoftwareState *softwareState) {
	const fpl__X11Api *x11Api = &subplatform->api;

	// Based on: https://bbs.archlinux.org/viewtopic.php?id=225741
	softwareState->graphicsContext = x11Api->XCreateGC(windowState->display, windowState->window, 0, 0);
	if(softwareState->graphicsContext == fpl_null) {
		return false;
	}

	softwareState->buffer = x11Api->XCreateImage(windowState->display, windowState->visual, 24, ZPixmap, 0, (char *)backbuffer->pixels, backbuffer->width, backbuffer->height, 32, (int)backbuffer->lineWidth);
	if(softwareState->buffer == fpl_null) {
		fpl__X11ReleaseVideoSoftware(subplatform, windowState, softwareState);
		return false;
	}

	// Initial draw pixels to the window
	x11Api->XPutImage(windowState->display, windowState->window, softwareState->graphicsContext, softwareState->buffer, 0, 0, 0, 0, backbuffer->width, backbuffer->height);
	x11Api->XSync(windowState->display, False);

	return (true);
	}
#endif // FPL_ENABLE_VIDEO_SOFTWARE && FPL_SUBPLATFORM_X11

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

fpl_internal uint32_t fpl__ReadAudioFramesFromClient(const fpl__CommonAudioState *commonAudio, uint32_t frameCount, void *pSamples) {
	uint32_t outputSamplesWritten = 0;
	if(commonAudio->clientReadCallback != fpl_null) {
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
#   include <mmsystem.h>
#	include <dsound.h>

#define FPL__FUNC_DSOUND_DirectSoundCreate(name) HRESULT WINAPI name(const GUID* pcGuidDevice, LPDIRECTSOUND *ppDS8, LPUNKNOWN pUnkOuter)
typedef FPL__FUNC_DSOUND_DirectSoundCreate(func_DirectSoundCreate);
#define FPL__FUNC_DSOUND_DirectSoundEnumerateA(name) HRESULT WINAPI name(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
typedef FPL__FUNC_DSOUND_DirectSoundEnumerateA(func_DirectSoundEnumerateA);
static GUID FPL__IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16} };
#define FPL__DIRECTSOUND_MAX_PERIODS 4

typedef struct fpl__DirectSoundApi {
	HMODULE dsoundLibrary;
	func_DirectSoundCreate *DirectSoundCreate;
	func_DirectSoundEnumerateA *DirectSoundEnumerateA;
} fpl__DirectSoundApi;

fpl_internal void fpl__UnloadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	FPL_ASSERT(dsoundApi != fpl_null);
	if(dsoundApi->dsoundLibrary != fpl_null) {
		FreeLibrary(dsoundApi->dsoundLibrary);
	}
	FPL_CLEAR_STRUCT(dsoundApi);
}

fpl_internal bool fpl__LoadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	FPL_ASSERT(dsoundApi != fpl_null);

	const char *dsoundLibraryName = "dsound.dll";
	HMODULE library = dsoundApi->dsoundLibrary = LoadLibraryA(dsoundLibraryName);
	if(library == fpl_null) {
		FPL_ERROR(FPL__MODULE_AUDIO_DIRECTSOUND, "Failed loading library '%s'", dsoundLibraryName);
		return false;
	}
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_AUDIO_DIRECTSOUND, library, dsoundLibraryName, dsoundApi->DirectSoundCreate, func_DirectSoundCreate, "DirectSoundCreate");
	FPL__WIN32_GET_FUNCTION_ADDRESS_RETURN(FPL__MODULE_AUDIO_DIRECTSOUND, library, dsoundLibraryName, dsoundApi->DirectSoundEnumerateA, func_DirectSoundEnumerateA, "DirectSoundEnumerateA");

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
	if(infos->deviceInfos != fpl_null) {
		uint32_t index = infos->foundDeviceCount++;
		if(index < infos->maxDeviceCount) {
			fplAudioDeviceInfo *deviceInfo = infos->deviceInfos + index;
			FPL_CLEAR_STRUCT(deviceInfo);
			fplCopyAnsiString(lpcstrDescription, deviceInfo->name, FPL_ARRAYCOUNT(deviceInfo->name));
			if(lpGuid != fpl_null) {
				fplMemoryCopy(lpGuid, sizeof(deviceInfo->id.dshow), &deviceInfo->id.dshow);
			}
			return TRUE;
		}
	}
	return FALSE;
}

fpl_internal uint32_t fpl__GetAudioDevicesDirectSound(fpl__DirectSoundAudioState *dsoundState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
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
	if(dsoundState->stopEvent != fpl_null) {
		CloseHandle(dsoundState->stopEvent);
	}

	for(uint32_t i = 0; i < commonAudio->internalFormat.periods; ++i) {
		if(dsoundState->notifyEvents[i]) {
			CloseHandle(dsoundState->notifyEvents[i]);
		}
	}

	if(dsoundState->notify != fpl_null) {
		IDirectSoundNotify_Release(dsoundState->notify);
	}

	if(dsoundState->secondaryBuffer != fpl_null) {
		IDirectSoundBuffer_Release(dsoundState->secondaryBuffer);
	}

	if(dsoundState->primaryBuffer != fpl_null) {
		IDirectSoundBuffer_Release(dsoundState->primaryBuffer);
	}

	if(dsoundState->directSound != fpl_null) {
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
	if(!fpl__LoadDirectSoundApi(dsoundApi)) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_ApiFailed;
	}

	// Load direct sound object
	const GUID *deviceId = fpl_null;
	if(fplGetAnsiStringLength(audioSettings->deviceInfo.name) > 0) {
		deviceId = &audioSettings->deviceInfo.id.dshow;
	}
	if(!SUCCEEDED(dsoundApi->DirectSoundCreate(deviceId, &dsoundState->directSound, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_NoDeviceFound;
	}

	// Setup wave format ex
	WAVEFORMATEXTENSIBLE wf = FPL_ZERO_INIT;
	wf.Format.cbSize = sizeof(wf);
	wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wf.Format.nChannels = (WORD)audioSettings->deviceFormat.channels;
	wf.Format.nSamplesPerSec = (DWORD)audioSettings->deviceFormat.sampleRate;
	wf.Format.wBitsPerSample = (WORD)fplGetAudioSampleSizeInBytes(audioSettings->deviceFormat.type) * 8;
	wf.Format.nBlockAlign = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
	wf.Format.nAvgBytesPerSec = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
	wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
	if((audioSettings->deviceFormat.type == fplAudioFormatType_F32) || (audioSettings->deviceFormat.type == fplAudioFormatType_F64)) {
		wf.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	} else {
		wf.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM;
	}

	// Get either local window handle or desktop handle
	HWND windowHandle = fpl_null;
#	if defined(FPL_ENABLE_WINDOW)
	if(appState->initFlags & fplInitFlags_Window) {
		windowHandle = appState->window.win32.windowHandle;
	}
#	endif
	if(windowHandle == fpl_null) {
		windowHandle = apiFuncs->user.GetDesktopWindow();
	}

	// The cooperative level must be set before doing anything else
	if(FAILED(IDirectSound_SetCooperativeLevel(dsoundState->directSound, windowHandle, (audioSettings->preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create primary buffer
	DSBUFFERDESC descDSPrimary = FPL_ZERO_INIT;
	descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
	descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	if(FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDSPrimary, &dsoundState->primaryBuffer, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Set format
	if(FAILED(IDirectSoundBuffer_SetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)&wf))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get the required size in bytes
	DWORD requiredSize;
	if(FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, fpl_null, 0, &requiredSize))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Get actual format
	char rawdata[1024];
	WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
	if(FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX*)pActualFormat, requiredSize, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Set internal format
	fplAudioDeviceFormat internalFormat = FPL_ZERO_INIT;
	if(fpl__Win32IsEqualGuid(pActualFormat->SubFormat, FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
		if(pActualFormat->Format.wBitsPerSample == 64) {
			internalFormat.type = fplAudioFormatType_F64;
		} else {
			internalFormat.type = fplAudioFormatType_F32;
		}
	} else {
		switch(pActualFormat->Format.wBitsPerSample) {
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
	if(FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDS, &dsoundState->secondaryBuffer, fpl_null))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
	if(FAILED(IDirectSoundBuffer_QueryInterface(dsoundState->secondaryBuffer, guid_IID_IDirectSoundNotify, (void**)&dsoundState->notify))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Setup notifications
	uint32_t periodSizeInBytes = internalFormat.bufferSizeInBytes / internalFormat.periods;
	DSBPOSITIONNOTIFY notifyPoints[FPL__DIRECTSOUND_MAX_PERIODS];
	for(uint32_t i = 0; i < internalFormat.periods; ++i) {
		dsoundState->notifyEvents[i] = CreateEventA(fpl_null, false, false, fpl_null);
		if(dsoundState->notifyEvents[i] == fpl_null) {
			fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
			return fplAudioResult_Failed;
		}

		// The notification offset is in bytes.
		notifyPoints[i].dwOffset = i * periodSizeInBytes;
		notifyPoints[i].hEventNotify = dsoundState->notifyEvents[i];
	}
	if(FAILED(IDirectSoundNotify_SetNotificationPositions(dsoundState->notify, internalFormat.periods, notifyPoints))) {
		fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
		return fplAudioResult_Failed;
	}

	// Create stop event
	dsoundState->stopEvent = CreateEventA(fpl_null, false, false, fpl_null);
	if(dsoundState->stopEvent == fpl_null) {
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
	if(FAILED(IDirectSoundBuffer_GetCurrentPosition(dsoundState->secondaryBuffer, fpl_null, &dwCurrentPosition))) {
		return false;
	}

	FPL_ASSERT(commonAudio->internalFormat.channels > 0);
	*pCurrentPos = (uint32_t)dwCurrentPosition / fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type) / commonAudio->internalFormat.channels;
	return true;
}

fpl_internal uint32_t fpl__GetAvailableFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
	// Get current frame from current play position
	uint32_t currentFrame;
	if(!fpl__GetCurrentFrameDirectSound(commonAudio, dsoundState, &currentFrame)) {
		return 0;
	}

	// In a playback device the last processed frame should always be ahead of the current frame. The space between
	// the last processed and current frame (moving forward, starting from the last processed frame) is the amount
	// of space available to write.
	uint32_t totalFrameCount = commonAudio->internalFormat.bufferSizeInFrames;
	uint32_t committedBeg = currentFrame;
	uint32_t committedEnd;
	committedEnd = dsoundState->lastProcessedFrame;
	if(committedEnd <= committedBeg) {
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
	if(timeoutInMilliseconds < 1) {
		timeoutInMilliseconds = 1;
	}

	// Copy event handles so we can wait for each one
	unsigned int eventCount = commonAudio->internalFormat.periods + 1;
	HANDLE pEvents[FPL__DIRECTSOUND_MAX_PERIODS + 1]; // +1 for the stop event.
	fplMemoryCopy(dsoundState->notifyEvents, sizeof(HANDLE) * commonAudio->internalFormat.periods, pEvents);
	pEvents[eventCount - 1] = dsoundState->stopEvent;

	while(!dsoundState->breakMainLoop) {
		// Get available frames from directsound
		uint32_t framesAvailable = fpl__GetAvailableFramesDirectSound(commonAudio, dsoundState);
		if(framesAvailable > 0) {
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
	if(FAILED(IDirectSoundBuffer_Stop(dsoundState->secondaryBuffer))) {
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

	if(SUCCEEDED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, 0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
		framesToRead = actualLockSize / audioSampleSizeBytes / commonAudio->internalFormat.channels;
		fpl__ReadAudioFramesFromClient(commonAudio, framesToRead, pLockPtr);
		IDirectSoundBuffer_Unlock(dsoundState->secondaryBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
		dsoundState->lastProcessedFrame = framesToRead;
		if(FAILED(IDirectSoundBuffer_Play(dsoundState->secondaryBuffer, 0, 0, DSBPLAY_LOOPING))) {
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
	while(!dsoundState->breakMainLoop) {
		// Wait until we get available frames from directsound
		uint32_t framesAvailable = fpl__WaitForFramesDirectSound(commonAudio, dsoundState);
		if(framesAvailable == 0) {
			continue;
		}

		// Don't bother grabbing more data if the device is being stopped.
		if(dsoundState->breakMainLoop) {
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
			if(FAILED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
				FPL_ERROR(FPL__MODULE_AUDIO_DIRECTSOUND, "Failed to lock directsound secondary buffer '%p' for offset/size (%lu / %lu)", dsoundState->secondaryBuffer, lockOffset, lockSize);
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
	if(alsaApi->libHandle != fpl_null) {
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
	for(uint32_t index = 0; index < FPL_ARRAYCOUNT(libraryNames); ++index) {
		const char * libName = libraryNames[index];
		void *libHandle = alsaApi->libHandle = dlopen(libName, FPL__POSIX_DL_LOADTYPE);
		if(libHandle != fpl_null) {
			do {
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_open, fpl__alsa_func_snd_pcm_open, "snd_pcm_open");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_close, fpl__alsa_func_snd_pcm_close, "snd_pcm_close");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_sizeof, fpl__alsa_func_snd_pcm_hw_params_sizeof, "snd_pcm_hw_params_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params, fpl__alsa_func_snd_pcm_hw_params, "snd_pcm_hw_params");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_any, fpl__alsa_func_snd_pcm_hw_params_any, "snd_pcm_hw_params_any");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_format, fpl__alsa_func_snd_pcm_hw_params_set_format, "snd_pcm_hw_params_set_format");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_format_first, fpl__alsa_func_snd_pcm_hw_params_set_format_first, "snd_pcm_hw_params_set_format_first");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_format_mask, fpl__alsa_func_snd_pcm_hw_params_get_format_mask, "snd_pcm_hw_params_get_format_mask");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_channels_near, fpl__alsa_func_snd_pcm_hw_params_set_channels_near, "snd_pcm_hw_params_set_channels_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_rate_resample, fpl__alsa_func_snd_pcm_hw_params_set_rate_resample, "snd_pcm_hw_params_set_rate_resample");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_rate_near, fpl__alsa_func_snd_pcm_hw_params_set_rate_near, "snd_pcm_hw_params_set_rate_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_buffer_size_near, fpl__alsa_func_snd_pcm_hw_params_set_buffer_size_near, "snd_pcm_hw_params_set_buffer_size_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_periods_near, fpl__alsa_func_snd_pcm_hw_params_set_periods_near, "snd_pcm_hw_params_set_periods_near");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_set_access, fpl__alsa_func_snd_pcm_hw_params_set_access, "snd_pcm_hw_params_set_access");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_format, fpl__alsa_func_snd_pcm_hw_params_get_format, "snd_pcm_hw_params_get_format");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_channels, fpl__alsa_func_snd_pcm_hw_params_get_channels, "snd_pcm_hw_params_get_channels");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_rate, fpl__alsa_func_snd_pcm_hw_params_get_rate, "snd_pcm_hw_params_get_rate");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_buffer_size, fpl__alsa_func_snd_pcm_hw_params_get_buffer_size, "snd_pcm_hw_params_get_buffer_size");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_periods, fpl__alsa_func_snd_pcm_hw_params_get_periods, "snd_pcm_hw_params_get_periods");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_access, fpl__alsa_func_snd_pcm_hw_params_get_access, "snd_pcm_hw_params_get_access");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_hw_params_get_sbits, fpl__alsa_func_snd_pcm_hw_params_get_sbits, "snd_pcm_hw_params_get_sbits");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_sw_params_sizeof, fpl__alsa_func_snd_pcm_sw_params_sizeof, "snd_pcm_sw_params_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_sw_params_current, fpl__alsa_func_snd_pcm_sw_params_current, "snd_pcm_sw_params_current");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_sw_params_set_avail_min, fpl__alsa_func_snd_pcm_sw_params_set_avail_min, "snd_pcm_sw_params_set_avail_min");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_sw_params_set_start_threshold, fpl__alsa_func_snd_pcm_sw_params_set_start_threshold, "snd_pcm_sw_params_set_start_threshold");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_sw_params, fpl__alsa_func_snd_pcm_sw_params, "snd_pcm_sw_params");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_format_mask_sizeof, fpl__alsa_func_snd_pcm_format_mask_sizeof, "snd_pcm_format_mask_sizeof");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_format_mask_test, fpl__alsa_func_snd_pcm_format_mask_test, "snd_pcm_format_mask_test");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_get_chmap, fpl__alsa_func_snd_pcm_get_chmap, "snd_pcm_get_chmap");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_prepare, fpl__alsa_func_snd_pcm_prepare, "snd_pcm_prepare");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_start, fpl__alsa_func_snd_pcm_start, "snd_pcm_start");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_drop, fpl__alsa_func_snd_pcm_drop, "snd_pcm_drop");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_device_name_hint, fpl__alsa_func_snd_device_name_hint, "snd_device_name_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_device_name_get_hint, fpl__alsa_func_snd_device_name_get_hint, "snd_device_name_get_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_card_get_index, fpl__alsa_func_snd_card_get_index, "snd_card_get_index");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_device_name_free_hint, fpl__alsa_func_snd_device_name_free_hint, "snd_device_name_free_hint");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_mmap_begin, fpl__alsa_func_snd_pcm_mmap_begin, "snd_pcm_mmap_begin");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_mmap_commit, fpl__alsa_func_snd_pcm_mmap_commit, "snd_pcm_mmap_commit");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_recover, fpl__alsa_func_snd_pcm_recover, "snd_pcm_recover");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_writei, fpl__alsa_func_snd_pcm_writei, "snd_pcm_writei");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_avail, fpl__alsa_func_snd_pcm_avail, "snd_pcm_avail");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_avail_update, fpl__alsa_func_snd_pcm_avail_update, "snd_pcm_avail_update");
				FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi->snd_pcm_wait, fpl__alsa_func_snd_pcm_wait, "snd_pcm_wait");
				result = true;
			} while(0);
			if(result) {
				break;
			}
		}
		fpl__UnloadAlsaApi(alsaApi);
	}
	return(result);
}

fpl_internal uint32_t fpl__AudioWaitForFramesAlsa(const fplAudioDeviceFormat *deviceFormat, fpl__AlsaAudioState *alsaState, bool *requiresRestart) {
	FPL_ASSERT(commonAudio != fpl_null && deviceFormat != fpl_null);
	if(requiresRestart != fpl_null) {
		*requiresRestart = false;
	}
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	uint32_t periodSizeInFrames = deviceFormat->bufferSizeInFrames / deviceFormat->periods;
	while(!alsaState->breakMainLoop) {
		const int timeoutInMilliseconds = 10;
		int waitResult = alsaApi->snd_pcm_wait(alsaState->pcmDevice, timeoutInMilliseconds);
		if(waitResult < 0) {
			if(waitResult == -EPIPE) {
				if(alsaApi->snd_pcm_recover(alsaState->pcmDevice, waitResult, 1) < 0) {
					return 0;
				}
				if(requiresRestart != fpl_null) {
					*requiresRestart = true;
				}
			}
		}

		if(alsaState->breakMainLoop) {
			return 0;
		}

		snd_pcm_sframes_t framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
		if(framesAvailable < 0) {
			if(framesAvailable == -EPIPE) {
				if(alsaApi->snd_pcm_recover(alsaState->pcmDevice, framesAvailable, 1) < 0) {
					return 0;
				}
				if(requiresRestart != fpl_null) {
					*requiresRestart = true;
				}
				framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
				if(framesAvailable < 0) {
					return 0;
				}
			}
		}

		// Keep the returned number of samples consistent and based on the period size.
		if(framesAvailable >= periodSizeInFrames) {
			return periodSizeInFrames;
		}

		// We'll get here if the loop was terminated. Just return whatever's available.
		framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
		if(framesAvailable < 0) {
			return 0;
		}
		return framesAvailable;
	}
}

fpl_internal bool fpl__GetAudioFramesFromClientAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(commonAudio != fpl_null && alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;

	if(!fpl__IsAudioDeviceStarted(commonAudio) && fpl__AudioGetDeviceState(commonAudio) != fpl__AudioDeviceState_Starting) {
		return false;
	}
	if(alsaState->breakMainLoop) {
		return false;
	}

	if(alsaState->isUsingMMap) {
		// mmap path
		bool requiresRestart;
		uint32_t framesAvailable = fpl__AudioWaitForFramesAlsa(&commonAudio->internalFormat, alsaState, &requiresRestart);
		if(framesAvailable == 0) {
			return false;
		}
		if(alsaState->breakMainLoop) {
			return false;
		}

		const snd_pcm_channel_area_t* channelAreas;
		snd_pcm_uframes_t mappedOffset;
		snd_pcm_uframes_t mappedFrames = framesAvailable;
		while(framesAvailable > 0) {
			int result = alsaApi->snd_pcm_mmap_begin(alsaState->pcmDevice, &channelAreas, &mappedOffset, &mappedFrames);
			if(result < 0) {
				return false;
			}
			if(mappedFrames > 0) {
				void *bufferPtr = (uint8_t *)channelAreas[0].addr + ((channelAreas[0].first + (mappedOffset * channelAreas[0].step)) / 8);
				fpl__ReadAudioFramesFromClient(commonAudio, mappedFrames, bufferPtr);
			}
			result = alsaApi->snd_pcm_mmap_commit(alsaState->pcmDevice, mappedOffset, mappedFrames);
			if(result < 0 || (snd_pcm_uframes_t)result != mappedFrames) {
				alsaApi->snd_pcm_recover(alsaState->pcmDevice, result, 1);
				return false;
			}
			framesAvailable -= mappedFrames;
			if(requiresRestart) {
				if(alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
					return false;
				}
			}
		}
	} else {
		// readi/writei path
		while(!alsaState->breakMainLoop) {
			uint32_t framesAvailable = fpl__AudioWaitForFramesAlsa(&commonAudio->internalFormat, alsaState, fpl_null);
			if(framesAvailable == 0) {
				continue;
			}
			if(alsaState->breakMainLoop) {
				return false;
			}
			fpl__ReadAudioFramesFromClient(commonAudio, framesAvailable, alsaState->intermediaryBuffer);
			snd_pcm_sframes_t framesWritten = alsaApi->snd_pcm_writei(alsaState->pcmDevice, alsaState->intermediaryBuffer, framesAvailable);
			if(framesWritten < 0) {
				if(framesWritten == -EAGAIN) {
					// Keep trying
					continue;
				} else if(framesWritten == -EPIPE) {
					// Underrun -> Recover and try again
					if(alsaApi->snd_pcm_recover(alsaState->pcmDevice, framesWritten, 1) < 0) {
						FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to recover device after underrun!");
						return false;
					}
					framesWritten = alsaApi->snd_pcm_writei(alsaState->pcmDevice, alsaState->intermediaryBuffer, framesAvailable);
					if(framesWritten < 0) {
						FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to write data to the PCM device!");
						return false;
					}
					// Success
					break;
				} else {
					FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to write audio frames from client, error code: %d!", framesWritten);
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

fpl_internal void fpl__AudioStopMainLoopAlsa(fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(alsaState != fpl_null);
	alsaState->breakMainLoop = true;
}

fpl_internal bool fpl__AudioReleaseAlsa(const fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(commonAudio != fpl_null && alsaState != fpl_null);
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if(alsaState->pcmDevice != fpl_null) {
		alsaApi->snd_pcm_close(alsaState->pcmDevice);
		alsaState->pcmDevice = fpl_null;
		if(alsaState->intermediaryBuffer != fpl_null) {
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
	if(alsaApi->snd_pcm_prepare(alsaState->pcmDevice) < 0) {
		FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to prepare PCM device '%p'!", alsaState->pcmDevice);
		return fplAudioResult_Failed;
	}

	// Get initial frames to fill from the client
	if(!fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
		FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to get initial audio frames from client!");
		return fplAudioResult_Failed;
	}

	if(alsaState->isUsingMMap) {
		if(alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
			FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to start PCM device '%p'!", alsaState->pcmDevice);
			return fplAudioResult_Failed;
		}
	}

	return fplAudioResult_Success;
}

fpl_internal bool fpl__AudioStopAlsa(fpl__AlsaAudioState *alsaState) {
	FPL_ASSERT(alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if(alsaApi->snd_pcm_drop(alsaState->pcmDevice)) {
		FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to drop the PCM device '%p'!", alsaState->pcmDevice);
		return false;
	}
	return true;
}

fpl_internal snd_pcm_format_t fpl__MapAudioFormatToAlsaFormat(fplAudioFormatType format) {
	switch(format) {
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
	while(!alsaState->breakMainLoop && fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
	}
}

fpl_internal fplAudioFormatType fpl__MapAlsaFormatToAudioFormat(snd_pcm_format_t format) {
	switch(format) {
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
		FPL_ERROR(FPL__MODULE_AUDIO_ALSA, format, ## __VA_ARGS__); \
		fpl__AudioReleaseAlsa(commonAudio, alsaState); \
		return fplAudioResult_Failed; \
	} while (0)

	// Load ALSA library
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if(!fpl__LoadAlsaApi(alsaApi)) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_ApiFailed, "Failed loading ALSA api!");
	}

	//
	// Open PCM Device
	//
	char deviceName[256] = FPL_ZERO_INIT;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	if(fplGetAnsiStringLength(audioSettings->deviceInfo.id.alsa) > 0) {
		const char *forcedDeviceId = audioSettings->deviceInfo.id.alsa;
		if(alsaApi->snd_pcm_open(&alsaState->pcmDevice, forcedDeviceId, stream, 0) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_NoDeviceFound, "PCM audio device by id '%s' not found!", forcedDeviceId);
		}
		fplCopyAnsiString(forcedDeviceId, deviceName, FPL_ARRAYCOUNT(deviceName));
	} else {
		const char *defaultDeviceNames[16];
		int defaultDeviceCount = 0;
		defaultDeviceNames[defaultDeviceCount++] = "default";
		if(!audioSettings->preferExclusiveMode) {
			defaultDeviceNames[defaultDeviceCount++] = "dmix";
			defaultDeviceNames[defaultDeviceCount++] = "dmix:0";
			defaultDeviceNames[defaultDeviceCount++] = "dmix:0,0";
		}
		defaultDeviceNames[defaultDeviceCount++] = "hw";
		defaultDeviceNames[defaultDeviceCount++] = "hw:0";
		defaultDeviceNames[defaultDeviceCount++] = "hw:0,0";

		bool isDeviceOpen = false;
		for(size_t defaultDeviceIndex = 0; defaultDeviceIndex < defaultDeviceCount; ++defaultDeviceIndex) {
			const char *defaultDeviceName = defaultDeviceNames[defaultDeviceIndex];
			FPL_LOG_DEBUG("ALSA", "Opening PCM audio device '%s'", defaultDeviceName);
			if(alsaApi->snd_pcm_open(&alsaState->pcmDevice, defaultDeviceName, stream, 0) == 0) {
				FPL_LOG_DEBUG("ALSA", "Successfully opened PCM audio device '%s'", defaultDeviceName);
				isDeviceOpen = true;
				fplCopyAnsiString(defaultDeviceName, deviceName, FPL_ARRAYCOUNT(deviceName));
				break;
			} else {
				FPL_LOG_ERROR("ALSA", "Failed opening PCM audio device '%s'!", defaultDeviceName);
			}
		}
		if(!isDeviceOpen) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_NoDeviceFound, "No PCM audio device found!");
		}
	}

	//
	// Get hardware parameters
	//
	FPL_ASSERT(alsaState->pcmDevice != fpl_null);
	FPL_ASSERT(fplGetAnsiStringLength(deviceName) > 0);

	FPL_LOG_DEBUG("ALSA", "Get hardware parameters from device '%s'", deviceName);
	size_t hardwareParamsSize = alsaApi->snd_pcm_hw_params_sizeof();
	snd_pcm_hw_params_t *hardwareParams = (snd_pcm_hw_params_t *)FPL_STACKALLOCATE(hardwareParamsSize);
	fplMemoryClear(hardwareParams, hardwareParamsSize);
	if(alsaApi->snd_pcm_hw_params_any(alsaState->pcmDevice, hardwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed getting hardware parameters from device '%s'!", deviceName);
	}
	FPL_LOG_DEBUG("ALSA", "Successfullyy got hardware parameters from device '%s'", deviceName);

	//
	// Access mode (Interleaved MMap or Standard readi/writei)
	//
	alsaState->isUsingMMap = false;
	if(!audioSettings->specific.alsa.noMMap) {
		if(alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_MMAP_INTERLEAVED) == 0) {
			alsaState->isUsingMMap = true;
		} else {
			FPL_LOG_ERROR("ALSA", "Failed setting MMap access mode for device '%s', trying fallback to standard mode!", deviceName);
		}
	}
	if(!alsaState->isUsingMMap) {
		if(alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
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
	if(!alsaApi->snd_pcm_format_mask_test(formatMask, preferredFormat)) {
		// The required format is not supported. Try a list of default formats.
		snd_pcm_format_t defaultFormats[] = {
			SND_PCM_FORMAT_FLOAT_LE,
			SND_PCM_FORMAT_S32_LE,
			SND_PCM_FORMAT_S24_3LE,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_FORMAT_U8,
		};
		foundFormat = SND_PCM_FORMAT_UNKNOWN;
		for(size_t defaultFormatIndex = 0; defaultFormatIndex < FPL_ARRAYCOUNT(defaultFormats); ++defaultFormatIndex) {
			snd_pcm_format_t defaultFormat = defaultFormats[defaultFormatIndex];
			if(alsaApi->snd_pcm_format_mask_test(formatMask, defaultFormat)) {
				foundFormat = defaultFormat;
				break;
			}
		}
	} else {
		foundFormat = preferredFormat;
	}
	if(foundFormat == SND_PCM_FORMAT_UNKNOWN) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "No supported audio format for device '%s' found!", deviceName);
	}

	//
	// Set format
	//
	fplAudioFormatType internalFormatType = fpl__MapAlsaFormatToAudioFormat(foundFormat);
	if(alsaApi->snd_pcm_hw_params_set_format(alsaState->pcmDevice, hardwareParams, foundFormat) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM format '%s' for device '%s'!", fplGetAudioFormatString(internalFormatType), deviceName);
	}

	//
	// Set channels
	//
	uint32_t internalChannels = audioSettings->deviceFormat.channels;
	if(alsaApi->snd_pcm_hw_params_set_channels_near(alsaState->pcmDevice, hardwareParams, &internalChannels) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM channels '%lu' for device '%s'!", internalChannels, deviceName);
	}

	//
	// Set sample rate
	//

	// @NOTE(final): We disable the resampling of the sample rate to not get caught into any driver bugs
	alsaApi->snd_pcm_hw_params_set_rate_resample(alsaState->pcmDevice, hardwareParams, 0);

	uint32_t internalSampleRate = audioSettings->deviceFormat.sampleRate;
	if(alsaApi->snd_pcm_hw_params_set_rate_near(alsaState->pcmDevice, hardwareParams, &internalSampleRate, 0) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM sample rate '%lu' for device '%s'!", internalSampleRate, deviceName);
	}

	//
	// Set periods
	//
	uint32_t internalPeriods = audioSettings->deviceFormat.periods;
	int periodsDir = 0;
	if(alsaApi->snd_pcm_hw_params_set_periods_near(alsaState->pcmDevice, hardwareParams, &internalPeriods, &periodsDir) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM periods '%lu' for device '%s'!", internalPeriods, deviceName);
	}

	//
	// Set buffer size
	//
	snd_pcm_uframes_t actualBufferSize = audioSettings->deviceFormat.bufferSizeInFrames;
	if(alsaApi->snd_pcm_hw_params_set_buffer_size_near(alsaState->pcmDevice, hardwareParams, &actualBufferSize) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed setting PCM buffer size '%lu' for device '%s'!", actualBufferSize, deviceName);
	}
	uint32_t internalBufferSizeInFrame = actualBufferSize;

	//
	// Set hardware parameters
	//
	if(alsaApi->snd_pcm_hw_params(alsaState->pcmDevice, hardwareParams) < 0) {
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
	if(alsaApi->snd_pcm_sw_params_current(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to get software parameters for device '%s'!", deviceName);
	}
	snd_pcm_uframes_t minAvailableFrames = (internalFormat.sampleRate / 1000) * 1;
	if(alsaApi->snd_pcm_sw_params_set_avail_min(alsaState->pcmDevice, softwareParams, minAvailableFrames) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to set software available min for device '%s'!", deviceName);
	}
	if(!alsaState->isUsingMMap) {
		if(alsaApi->snd_pcm_sw_params_set_start_threshold(alsaState->pcmDevice, softwareParams, minAvailableFrames) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to set start threshold of '%lu' for device '%s'!", minAvailableFrames, deviceName);
		}
	}
	if(alsaApi->snd_pcm_sw_params(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed to install PCM software parameters for device '%s'!", deviceName);
	}

	if(!alsaState->isUsingMMap) {
		alsaState->intermediaryBuffer = fplMemoryAllocate(internalFormat.bufferSizeInBytes);
		if(alsaState->intermediaryBuffer == fpl_null) {
			FPL__ALSA_INIT_ERROR(fplAudioResult_Failed, "Failed allocating intermediary buffer of size '%lu' for device '%s'!", internalFormat.bufferSizeInBytes, deviceName);
		}
	}

	// @NOTE(final): We do not ALSA support channel mapping right know, so we limit it to mono or stereo
	FPL_ASSERT(internalFormat.channels <= 2);

#undef FPL__ALSA_INIT_ERROR

	return fplAudioResult_Success;
}

fpl_internal uint32_t fpl__GetAudioDevicesAlsa(fpl__AlsaAudioState *alsaState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
	FPL_ASSERT((alsaState != fpl_null) && (deviceInfos != fpl_null));
	FPL_ASSERT(maxDeviceCount > 0);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	char** ppDeviceHints;
	if(alsaApi->snd_device_name_hint(-1, "pcm", (void ***)&ppDeviceHints) < 0) {
		return 0;
	}
	uint32_t capacityOverflow = 0;
	uint32_t result = 0;
	char** ppNextDeviceHint = ppDeviceHints;
	while(*ppNextDeviceHint != fpl_null) {
		char* name = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "NAME");
		char* ioid = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "IOID");

		// Only allow output or default devices
		if(name != fpl_null && (fplIsStringEqual(name, "default") || fplIsStringEqual(name, "pulse") || fplIsStringEqual(ioid, "Output"))) {
			if(result >= maxDeviceCount) {
				++capacityOverflow;
			} else {
				fplAudioDeviceInfo *outDeviceInfo = deviceInfos + result++;
				FPL_CLEAR_STRUCT(outDeviceInfo);
				fplCopyAnsiString(name, outDeviceInfo->id.alsa, FPL_ARRAYCOUNT(outDeviceInfo->id.alsa));
				char* desc = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "DESC");
				if(desc != fpl_null) {
					fplCopyAnsiString(desc, outDeviceInfo->name, FPL_ARRAYCOUNT(outDeviceInfo->name));
					free(desc);
				} else {
					fplCopyAnsiString(name, outDeviceInfo->name, FPL_ARRAYCOUNT(outDeviceInfo->name));
				}
			}
		}
		if(ioid != fpl_null) {
			free(ioid);
		}
		if(name != fpl_null) {
			free(name);
		}
		++ppNextDeviceHint;
	}
	alsaApi->snd_device_name_free_hint((void**)ppDeviceHints);
	if(capacityOverflow > 0) {
		FPL_ERROR(FPL__MODULE_AUDIO_ALSA, "Capacity of '%lu' for audio device infos has been reached. '%lu' audio devices are not included in the result", maxDeviceCount, capacityOverflow);
}
	return(result);
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

fpl_internal fpl__AudioState *fpl__GetAudioState(fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__AudioState *result = fpl_null;
	if(appState->audio.mem != fpl_null) {
		result = (fpl__AudioState *)appState->audio.mem;
	}
	return(result);
}

fpl_internal void fpl__StopAudioDeviceMainLoop(fpl__AudioState *audioState) {
	FPL_ASSERT(audioState->activeDriver > fplAudioDriverType_Auto);
	switch(audioState->activeDriver) {

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
	switch(audioState->activeDriver) {

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
	switch(audioState->activeDriver) {

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
	switch(audioState->activeDriver) {

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
	switch(audioState->activeDriver) {

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

fpl_internal bool fpl__IsAudioDriverAsync(fplAudioDriverType audioDriver) {
	switch(audioDriver) {
		case fplAudioDriverType_DirectSound:
		case fplAudioDriverType_Alsa:
			return false;
		default:
			return false;
	}
}

fpl_internal void fpl__AudioSetDeviceState(fpl__CommonAudioState *audioState, fpl__AudioDeviceState newState) {
	fplAtomicStoreU32((volatile uint32_t *)&audioState->state, (uint32_t)newState);
}

fpl_internal fpl__AudioDeviceState fpl__AudioGetDeviceState(fpl__CommonAudioState *audioState) {
	fpl__AudioDeviceState result = (fpl__AudioDeviceState)fplAtomicLoadU32((volatile uint32_t *)&audioState->state);
	return(result);
}

fpl_internal bool fpl__IsAudioDeviceInitialized(fpl__CommonAudioState *audioState) {
	if(audioState == fpl_null) {
		return false;
	}
	fpl__AudioDeviceState state = fpl__AudioGetDeviceState(audioState);
	return(state != fpl__AudioDeviceState_Uninitialized);
}

fpl_internal bool fpl__IsAudioDeviceStarted(fpl__CommonAudioState *audioState) {
	if(audioState == fpl_null) {
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

	for(;;) {
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
		if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Uninitialized) {
			break;
		}

		// Expect that the device is currently be started by the client
		FPL_ASSERT(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting);

		// Start audio device
		audioState->workResult = fpl__StartAudioDevice(audioState);
		if(audioState->workResult != fplAudioResult_Success) {
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

	if(fpl__IsAudioDeviceInitialized(commonAudioState)) {

		// Wait until the audio device is stopped
		if(fpl__IsAudioDeviceStarted(commonAudioState)) {
			while(fplStopAudio() == fplAudioResult_DeviceBusy) {
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

	if(audioState->activeDriver != fplAudioDriverType_None) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	if(audioSettings->deviceFormat.channels == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if(audioSettings->deviceFormat.sampleRate == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if(audioSettings->bufferSizeInMilliSeconds == 0) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	audioState->common.clientReadCallback = audioSettings->clientReadCallback;
	audioState->common.clientUserData = audioSettings->userData;

#if defined(FPL_PLATFORM_WIN32)
	wapi->ole.CoInitializeEx(fpl_null, 0);
#endif

	// Create mutex and signals
	if(!fplMutexInit(&audioState->lock)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if(!fplSignalInit(&audioState->wakeupSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if(!fplSignalInit(&audioState->startSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}
	if(!fplSignalInit(&audioState->stopSignal, fplSignalValue_Unset)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResult_Failed;
	}

	// Prope drivers
	fplAudioDriverType propeDrivers[16];
	uint32_t driverCount = 0;
	if(audioSettings->driver == fplAudioDriverType_Auto) {
		// @NOTE(final): Add all audio drivers here, regardless of the platform.
		propeDrivers[driverCount++] = fplAudioDriverType_DirectSound;
		propeDrivers[driverCount++] = fplAudioDriverType_Alsa;
	} else {
		// @NOTE(final): Forced audio driver
		propeDrivers[driverCount++] = audioSettings->driver;
	}
	fplAudioResult initResult = fplAudioResult_Failed;
	for(uint32_t driverIndex = 0; driverIndex < driverCount; ++driverIndex) {
		fplAudioDriverType propeDriver = propeDrivers[driverIndex];

		initResult = fplAudioResult_Failed;
		switch(propeDriver) {

#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioDriverType_DirectSound:
			{
				initResult = fpl__AudioInitDirectSound(audioSettings, &audioState->common, &audioState->dsound);
				if(initResult != fplAudioResult_Success) {
					fpl__AudioReleaseDirectSound(&audioState->common, &audioState->dsound);
				}
			} break;
#		endif

#		if defined(FPL_ENABLE_AUDIO_ALSA)
			case fplAudioDriverType_Alsa:
			{
				initResult = fpl__AudioInitAlsa(audioSettings, &audioState->common, &audioState->alsa);
				if(initResult != fplAudioResult_Success) {
					fpl__AudioReleaseAlsa(&audioState->common, &audioState->alsa);
				}
				} break;
#		endif

			default:
				break;
			}
		if(initResult == fplAudioResult_Success) {
			audioState->activeDriver = propeDriver;
			audioState->isAsyncDriver = fpl__IsAudioDriverAsync(propeDriver);
			break;
		}
		}

	if(initResult != fplAudioResult_Success) {
		fpl__ReleaseAudio(audioState);
		return initResult;
	}

	if(!audioState->isAsyncDriver) {
		// Create and start worker thread
		audioState->workerThread = fplThreadCreate(fpl__AudioWorkerThread, audioState);
		if(audioState->workerThread == fpl_null) {
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
typedef union fpl__Win32VideoState {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
	fpl__Win32VideoOpenGLState opengl;
#	endif
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	fpl__Win32VideoSoftwareState software;
#	endif
} fpl__Win32VideoState;
#endif // FPL_PLATFORM_WIN32

#if defined(FPL_SUBPLATFORM_X11)
typedef union fpl__X11VideoState {
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
	fpl__X11VideoOpenGLState opengl;
#	endif
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	fpl__X11VideoSoftwareState software;
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

fpl_internal fpl__VideoState *fpl__GetVideoState(fpl__PlatformAppState *appState) {
	FPL_ASSERT(appState != fpl_null);
	fpl__VideoState *result = fpl_null;
	if(appState->video.mem != fpl_null) {
		result = (fpl__VideoState *)appState->video.mem;
	}
	return(result);
}

fpl_internal void fpl__ShutdownVideo(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	FPL_ASSERT(appState != fpl_null);
	if(videoState != fpl_null) {
		switch(videoState->activeDriver) {
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
				fpl__X11ReleaseVideoSoftware(&appState->x11, &appState->window.x11, &videoState->x11.software);
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_SOFTWARE

			default:
			{
			} break;
		}

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
		if(backbuffer->pixels != fpl_null) {
			fplMemoryAlignedFree(backbuffer->pixels);
		}
		FPL_CLEAR_STRUCT(backbuffer);
#	endif
	}
}

fpl_internal void fpl__ReleaseVideoState(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	switch(videoState->activeDriver) {
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

	switch(driver) {
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
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	if(driver == fplVideoDriverType_Software) {
		fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
		backbuffer->width = windowWidth;
		backbuffer->height = windowHeight;
		backbuffer->pixelStride = sizeof(uint32_t);
		backbuffer->lineWidth = backbuffer->width * backbuffer->pixelStride;
		size_t size = backbuffer->lineWidth * backbuffer->height;
		backbuffer->pixels = (uint32_t *)fplMemoryAlignedAllocate(size, 4);
		if(backbuffer->pixels == fpl_null) {
			FPL_ERROR(FPL__MODULE_VIDEO_SOFTWARE, "Failed allocating video software backbuffer of size %xu bytes", size);
			fpl__ShutdownVideo(appState, videoState);
			return false;
		}

		// Clear to black by default
		// @NOTE(final): Bitmap is top-down, 0xAABBGGRR
		uint32_t *p = backbuffer->pixels;
		for(uint32_t y = 0; y < backbuffer->height; ++y) {
			uint32_t color = 0xFF000000;
			for(uint32_t x = 0; x < backbuffer->width; ++x) {
				*p++ = color;
			}
		}
	}
#	endif // FPL_ENABLE_VIDEO_SOFTWARE

	bool videoInitResult = false;
	switch(driver) {
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
			videoInitResult = fpl__X11InitVideoSoftware(&appState->x11, &appState->window.x11, videoSettings, &videoState->softwareBackbuffer, &videoState->x11.software);
#		endif
		} break;
#	endif // FPL_ENABLE_VIDEO_SOFTWARE

		default:
		{
			FPL_ERROR(FPL__MODULE_VIDEO, "Unsupported video driver '%s' for this platform", fplGetVideoDriverString(videoSettings->driver));
		} break;
	}
	if(!videoInitResult) {
		FPL_ASSERT(fplGetErrorCount() > 0);
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
	if(initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		switch(initSettings->video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_PLATFORM_WIN32)
				if(!fpl__Win32PreSetupWindowForOpenGL(&appState->win32, &appState->window.win32, &initSettings->video)) {
					return false;
				}
#			endif
#			if defined(FPL_SUBPLATFORM_X11)
				if(fpl__X11InitFrameBufferConfigVideoOpenGL(&appState->x11.api, &appState->window.x11, &initSettings->video, &videoState->x11.opengl)) {
					result = fpl__X11SetPreWindowSetupForOpenGL(&appState->x11.api, &appState->window.x11, &videoState->x11.opengl, &outResult->x11);
				}
#			endif
			} break;
#		endif // FPL_ENABLE_VIDEO_OPENGL

#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case fplVideoDriverType_Software:
			{
#			if defined(FPL_SUBPLATFORM_X11)
				result = fpl__X11SetPreWindowSetupForSoftware(&appState->x11.api, &appState->window.x11, &videoState->x11.software, &outResult->x11);
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
	if(initFlags & fplInitFlags_Video) {
		switch(initSettings->video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
#			if defined(FPL_PLATFORM_WIN32)
				if(!fpl__Win32PostSetupWindowForOpenGL(&appState->win32, &appState->window.win32, &initSettings->video)) {
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
	if(appState != fpl_null) {
#	if defined(FPL_PLATFORM_WIN32)
		result = fpl__Win32InitWindow(initSettings, currentWindowSettings, appState, &appState->win32, &appState->window.win32, setupCallbacks);
#	elif defined(FPL_SUBPLATFORM_X11)
		result = fpl__X11InitWindow(initSettings, currentWindowSettings, appState, &appState->x11, &appState->window.x11, setupCallbacks);
#	endif
	}
	return (result);
}

fpl_internal void fpl__ReleaseWindow(const fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	if(appState != fpl_null) {
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
fpl_common_api uint32_t fplGetAudioSampleSizeInBytes(const fplAudioFormatType format) {
	switch(format) {
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

fpl_common_api const char *fplGetAudioFormatString(const fplAudioFormatType format) {
	switch(format) {
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

fpl_common_api const char *fplGetAudioDriverString(fplAudioDriverType driver) {
	switch(driver) {
		case fplAudioDriverType_Auto:
			return "Auto";
		case fplAudioDriverType_DirectSound:
			return "DirectSound";
		case fplAudioDriverType_Alsa:
			return "ALSA";
		case fplAudioDriverType_None:
			return "None";
		default:
			return "";
	}
}

fpl_common_api uint32_t fplGetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds) {
	uint32_t result = (sampleRate / 1000) * bufferSizeInMilliSeconds;
	return(result);
}
fpl_common_api uint32_t fplGetAudioFrameSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount) {
	uint32_t result = fplGetAudioSampleSizeInBytes(format) * channelCount;
	return(result);
}
fpl_common_api uint32_t fplGetAudioBufferSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount, const uint32_t frameCount) {
	uint32_t frameSize = fplGetAudioFrameSizeInBytes(format, channelCount);
	uint32_t result = frameSize * frameCount;
	return(result);
}

fpl_common_api fplAudioResult fplStopAudio() {
	FPL__CheckPlatform(fplAudioResult_PlatformNotInitialized);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if(audioState == fpl_null) {
		return fplAudioResult_Failed;
	}

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Uninitialized) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock);
	{
		// Check if the device is already stopped
		if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopping) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}
		if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStopped;
		}

		// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
		if(fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Stopping);

		if(audioState->isAsyncDriver) {
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
	FPL__CheckPlatform(fplAudioResult_PlatformNotInitialized);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if(audioState == fpl_null) {
		return fplAudioResult_Failed;
	}

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if(!fpl__IsAudioDeviceInitialized(commonAudioState)) {
		return fplAudioResult_DeviceNotInitialized;
	}

	fplAudioResult result = fplAudioResult_Failed;
	fplMutexLock(&audioState->lock);
	{
		// Be a bit more descriptive if the device is already started or is already in the process of starting.
		if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}
		if(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceAlreadyStarted;
		}

		// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
		if(fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResult_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Starting);

		if(audioState->isAsyncDriver) {
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

fpl_common_api bool fplGetAudioHardwareFormat(fplAudioDeviceFormat *outFormat) {
	FPL__CheckArgumentNull(outFormat, false);
	FPL__CheckPlatform(false);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if(audioState != fpl_null) {
		FPL_CLEAR_STRUCT(outFormat);
		*outFormat = audioState->common.internalFormat;
		return true;
	}
	return false;
}

fpl_common_api bool fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData) {
	FPL__CheckPlatform(false);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if((audioState != fpl_null) && (audioState->activeDriver > fplAudioDriverType_Auto)) {
		if(fpl__AudioGetDeviceState(&audioState->common) == fpl__AudioDeviceState_Stopped) {
			audioState->common.clientReadCallback = newCallback;
			audioState->common.clientUserData = userData;
			return true;
		}
	}
	return false;
}

fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount) {
	FPL__CheckArgumentNull(devices, 0);
	FPL__CheckArgumentZero(maxDeviceCount, 0);
	FPL__CheckPlatform(0);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if(audioState == fpl_null) {
		return 0;
	}
	uint32_t result = 0;
	if(audioState->activeDriver > fplAudioDriverType_Auto) {
		switch(audioState->activeDriver) {
#		if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioDriverType_DirectSound:
			{
				result = fpl__GetAudioDevicesDirectSound(&audioState->dsound, devices, maxDeviceCount);
			} break;
#		endif

#		if defined(FPL_ENABLE_AUDIO_ALSA)
			case fplAudioDriverType_Alsa:
			{
				result = fpl__GetAudioDevicesAlsa(&audioState->alsa, devices, maxDeviceCount);
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
fpl_common_api const char *fplGetVideoDriverString(fplVideoDriverType driver) {
	switch(driver) {
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

fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer() {
	FPL__CheckPlatform(fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fplVideoBackBuffer *result = fpl_null;
	if(appState->video.mem != fpl_null) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		if(appState->currentSettings.video.driver == fplVideoDriverType_Software) {
			result = &videoState->softwareBackbuffer;
		}
#	endif
	}
	return(result);
}

fpl_common_api fplVideoDriverType fplGetVideoDriver() {
	FPL__CheckPlatform(fplVideoDriverType_None);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	fplVideoDriverType result = appState->currentSettings.video.driver;
	return(result);
}

fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	bool result = false;
	if(videoState != fpl_null) {
#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		if(videoState->activeDriver == fplVideoDriverType_Software) {
			fpl__ShutdownVideo(appState, videoState);
			result = fpl__InitVideo(fplVideoDriverType_Software, &appState->currentSettings.video, width, height, appState, videoState);
		}
#	endif
	}
	return (result);
}

fpl_common_api void fplVideoFlip() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__VideoState *videoState = fpl__GetVideoState(appState);
	if(videoState != fpl_null) {
#	if defined(FPL_PLATFORM_WIN32)
		const fpl__Win32AppState *win32AppState = &appState->win32;
		const fpl__Win32WindowState *win32WindowState = &appState->window.win32;
		const fpl__Win32Api *wapi = &win32AppState->winApi;
		switch(appState->currentSettings.video.driver) {
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case fplVideoDriverType_Software:
			{
				const fpl__Win32VideoSoftwareState *software = &videoState->win32.software;
				const fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
				fplWindowSize area;
				if(fplGetWindowArea(&area)) {
					int32_t targetX = 0;
					int32_t targetY = 0;
					int32_t targetWidth = area.width;
					int32_t targetHeight = area.height;
					int32_t sourceWidth = backbuffer->width;
					int32_t sourceHeight = backbuffer->height;
					if(backbuffer->useOutputRect) {
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
		switch(appState->currentSettings.video.driver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case fplVideoDriverType_OpenGL:
			{
				const fpl__X11VideoOpenGLApi *glApi = &videoState->x11.opengl.api;
				glApi->glXSwapBuffers(x11WinState->display, x11WinState->window);
			} break;
#		endif

#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case fplVideoDriverType_Software:
			{
				const fpl__X11Api *x11Api = &appState->x11.api;
				const fpl__X11VideoSoftwareState *softwareState = &videoState->x11.software;
				const fplVideoBackBuffer *backbuffer = &videoState->softwareBackbuffer;
				x11Api->XPutImage(x11WinState->display, x11WinState->window, softwareState->graphicsContext, softwareState->buffer, 0, 0, 0, 0, backbuffer->width, backbuffer->height);
				x11Api->XSync(x11WinState->display, False);
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
		FPL_LOG_DEBUG("Core", "Release Audio");
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		if(audioState != fpl_null) {
			fpl__ReleaseAudio(audioState);
		}
	}
#	endif

	// Shutdown video (Release context only)
#	if defined(FPL_ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if(videoState != fpl_null) {
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Shutdown Video for Driver '%s'", fplGetVideoDriverString(videoState->activeDriver));
			fpl__ShutdownVideo(appState, videoState);
		}
	}
#	endif

	// Release window
#	if defined(FPL_ENABLE_WINDOW)
	{
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Window");
		fpl__ReleaseWindow(initState, appState);
	}
#	endif

	// Release video state
#	if defined(FPL_ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if(videoState != fpl_null) {
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Video for Driver '%s'", fplGetVideoDriverString(videoState->activeDriver));
			fpl__ReleaseVideoState(appState, videoState);
		}
	}
#	endif

	if(appState != fpl_null) {
		// Release actual platform (There can only be one platform!)
		{
#		if defined(FPL_PLATFORM_WIN32)
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Win32 Platform");
			fpl__Win32ReleasePlatform(initState, appState);
#		elif defined(FPL_PLATFORM_LINUX)
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Linux Platform");
			fpl__LinuxReleasePlatform(initState, appState);
#		elif defined(FPL_PLATFORM_UNIX)
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Unix Platform");
			fpl__UnixReleasePlatform(initState, appState);
#		endif
		}

		// Release sub platforms
		{
#		if defined(FPL_SUBPLATFORM_X11)
			FPL_LOG_DEBUG("Core", "Release X11 Subplatform");
			fpl__X11ReleaseSubplatform(&appState->x11);
#		endif
#		if defined(FPL_SUBPLATFORM_POSIX)
			FPL_LOG_DEBUG("Core", "Release POSIX Subplatform");
			fpl__PosixReleaseSubplatform(&appState->posix);
#		endif
	}

	// Release platform applicatiom state memory
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release allocated Platform App State Memory");
		fplMemoryFree(appState);
		fpl__global__AppState = fpl_null;
}
	initState->isInitialized = false;
}

fpl_common_api const char *fplGetPlatformTypeString(const fplPlatformType type) {
	switch(type) {
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

fpl_common_api void fplPlatformRelease() {
	// Exit out if platform is not initialized
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	if(!initState->isInitialized) {
		FPL_CRITICAL(FPL__MODULE_CORE, "Platform is not initialized");
		return;
	}
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Platform");
	fpl__ReleasePlatformStates(initState, appState);
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Platform released");
}

fpl_common_api fplInitResultType fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {
	// Exit out if platform is already initialized
	if(fpl__global__InitState.isInitialized) {
		FPL_CRITICAL(FPL__MODULE_CORE, "Platform is already initialized");
		return fplInitResultType_AlreadyInitialized;
	}

	// Allocate platform app state memory (By boundary of 16-bytes)
	size_t platformAppStateSize = FPL_ALIGNED_SIZE(sizeof(fpl__PlatformAppState), 16);

	// Include video/audio state memory in app state memory as well
#if defined(FPL_ENABLE_VIDEO)
	size_t videoMemoryOffset = 0;
	if(initFlags & fplInitFlags_Video) {
		platformAppStateSize += FPL__ARBITARY_PADDING;
		videoMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__VideoState);
	}
#endif

#if defined(FPL_ENABLE_AUDIO)
	size_t audioMemoryOffset = 0;
	if(initFlags & fplInitFlags_Audio) {
		platformAppStateSize += FPL__ARBITARY_PADDING;
		audioMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__AudioState);
	}
#endif

	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Allocate Platform App State Memory of size '%zu':", platformAppStateSize);
	FPL_ASSERT(fpl__global__AppState == fpl_null);
	void *platformAppStateMemory = fplMemoryAllocate(platformAppStateSize);
	if(platformAppStateMemory == fpl_null) {
		FPL_CRITICAL(FPL__MODULE_CORE, "Failed Allocating Platform App State Memory of size '%zu'", platformAppStateSize);
		return fplInitResultType_FailedAllocatingMemory;
	}

	fpl__PlatformAppState *appState = fpl__global__AppState = (fpl__PlatformAppState *)platformAppStateMemory;
	appState->initFlags = initFlags;
	if(initSettings != fpl_null) {
		appState->initSettings = *initSettings;
	} else {
		fplSetDefaultSettings(&appState->initSettings);
	}
	appState->currentSettings = appState->initSettings;

	fpl__PlatformInitState *initState = &fpl__global__InitState;
	FPL_CLEAR_STRUCT(initState);
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully allocated Platform App State Memory of size '%zu'", platformAppStateSize);

// Window is required for video always
#	if defined(FPL_ENABLE_VIDEO)
	if(appState->initFlags & fplInitFlags_Video) {
		appState->initFlags |= fplInitFlags_Window;
	}
#	endif
#	if !defined(FPL_ENABLE_WINDOW)
	appState->initFlags = (fplInitFlags)(appState->initFlags & ~fplInitFlags_Window);
#	endif
#	if defined(FPL_APPTYPE_CONSOLE)
	appState->initFlags |= fplInitFlags_Console;
#	endif

	// Initialize sub-platforms
#	if defined(FPL_SUBPLATFORM_POSIX)
	{
		FPL_LOG_DEBUG("Core", "Initialize POSIX Subplatform:");
		if(!fpl__PosixInitSubplatform(initFlags, initSettings, &initState->posix, &appState->posix)) {
			FPL_CRITICAL("Core", "Failed initializing POSIX Subplatform!");
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedPlatform;
		}
		FPL_LOG_DEBUG("Core", "Successfully initialized POSIX Subplatform");
	}
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
	{
		FPL_LOG_DEBUG("Core", "Initialize X11 Subplatform:");
		if(!fpl__X11InitSubplatform(&appState->x11)) {
			FPL_CRITICAL("Core", "Failed initializing X11 Subplatform!");
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedPlatform;
}
		FPL_LOG_DEBUG("Core", "Successfully initialized X11 Subplatform");
		}
#	endif // FPL_SUBPLATFORM_X11

		// Initialize the actual platform (There can only be one at a time!)
	bool isInitialized = false;
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Initialize %s Platform:", FPL_PLATFORM_NAME);
#	if defined(FPL_PLATFORM_WIN32)
	isInitialized = fpl__Win32InitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#   elif defined(FPL_PLATFORM_LINUX)
	isInitialized = fpl__LinuxInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#   elif defined(FPL_PLATFORM_UNIX)
	isInitialized = fpl__UnixInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	endif

	if(!isInitialized) {
		FPL_CRITICAL(FPL__MODULE_CORE, "Failed initializing %s Platform!", FPL_PLATFORM_NAME);
		fpl__ReleasePlatformStates(initState, appState);
		return fplInitResultType_FailedPlatform;
	}
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized %s Platform", FPL_PLATFORM_NAME);

// Init video state
#	if defined(FPL_ENABLE_VIDEO)
	if(appState->initFlags & fplInitFlags_Video) {
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init video state:");
		appState->video.mem = (uint8_t *)platformAppStateMemory + videoMemoryOffset;
		appState->video.memSize = sizeof(fpl__VideoState);
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		FPL_ASSERT(videoState != fpl_null);

		fplVideoDriverType videoDriver = appState->initSettings.video.driver;
		const char *videoDriverString = fplGetVideoDriverString(videoDriver);
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Load Video API for Driver '%s':", videoDriverString);
		{
			if(!fpl__LoadVideoState(videoDriver, videoState)) {
				FPL_CRITICAL(FPL__MODULE_CORE, "Failed loading Video API for Driver '%s'!", videoDriverString);
				fpl__ReleasePlatformStates(initState, appState);
				return fplInitResultType_FailedVideo;
			}
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully loaded Video API for Driver '%s'", videoDriverString);
	}
#	endif // FPL_ENABLE_VIDEO

	// Init Window & event queue
#	if defined(FPL_ENABLE_WINDOW)
	if(appState->initFlags & fplInitFlags_Window) {
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Window:");
		fpl__SetupWindowCallbacks winCallbacks = FPL_ZERO_INIT;
		winCallbacks.postSetup = fpl__PostSetupWindowDefault;
		winCallbacks.preSetup = fpl__PreSetupWindowDefault;
		if(!fpl__InitWindow(&appState->initSettings, &appState->currentSettings.window, appState, &winCallbacks)) {
			FPL_CRITICAL(FPL__MODULE_CORE, "Failed initializing Window!");
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedWindow;
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized Window");
	}
#	endif // FPL_ENABLE_WINDOW

	// Init Video
#	if defined(FPL_ENABLE_VIDEO)
	if(appState->initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		FPL_ASSERT(videoState != fpl_null);
		uint32_t windowWidth, windowHeight;
		if(appState->currentSettings.window.isFullscreen) {
			windowWidth = appState->currentSettings.window.fullscreenWidth;
			windowHeight = appState->currentSettings.window.fullscreenHeight;
		} else {
			windowWidth = appState->currentSettings.window.windowWidth;
			windowHeight = appState->currentSettings.window.windowWidth;
		}
		const char *videoDriverName = fplGetVideoDriverString(appState->initSettings.video.driver);
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Video with Driver '%s':", videoDriverName);
		if(!fpl__InitVideo(appState->initSettings.video.driver, &appState->initSettings.video, windowWidth, windowHeight, appState, videoState)) {
			FPL_CRITICAL(FPL__MODULE_CORE, "Failed initialization video with settings (Driver=%s, Width=%d, Height=%d)", videoDriverName, windowWidth, windowHeight);
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedVideo;
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized Video Driver '%s'", videoDriverName);
	}
#	endif // FPL_ENABLE_VIDEO

	// Init Audio
#	if defined(FPL_ENABLE_AUDIO)
	if(appState->initFlags & fplInitFlags_Audio) {
		appState->audio.mem = (uint8_t *)platformAppStateMemory + audioMemoryOffset;
		appState->audio.memSize = sizeof(fpl__AudioState);
		const char *audioDriverName = fplGetAudioDriverString(appState->initSettings.audio.driver);
		FPL_LOG_DEBUG("Core", "Init Audio with Driver '%s':", audioDriverName);
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		FPL_ASSERT(audioState != fpl_null);
		if(fpl__InitAudio(&appState->initSettings.audio, audioState) != fplAudioResult_Success) {
			FPL_CRITICAL("Core", "Failed initialization audio with settings (Driver=%s, Format=%s, SampleRate=%d, Channels=%d, BufferSize=%d)", audioDriverName, fplGetAudioFormatString(initSettings->audio.deviceFormat.type), initSettings->audio.deviceFormat.sampleRate, initSettings->audio.deviceFormat.channels);
			fpl__ReleasePlatformStates(initState, appState);
			return fplInitResultType_FailedAudio;
		}
		FPL_LOG_DEBUG("Core", "Successfully initialized Audio Driver '%s'", audioDriverName);
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

#if defined(FPL_COMPILER_MSVC)
	//! Don't spill our preferences to the outside
#	pragma warning( pop )
#endif

#endif // FPL_IMPLEMENTATION && !FPL_IMPLEMENTED

// ****************************************************************************
//
// Entry-Points Implementation
//
// ****************************************************************************
#if defined(FPL_ENTRYPOINT) && !defined(FPL_ENTRYPOINT_IMPLEMENTED)
#	define FPL_ENTRYPOINT_IMPLEMENTED

#	if defined(FPL_PLATFORM_WIN32)

#		if defined(FPL_NO_CRT)
			//
			// Win32 without CRT
			//
#			if defined(FPL_APPTYPE_WINDOW)
#				if defined(UNICODE)
void __stdcall wWinMainCRTStartup(void) {
	LPWSTR argsW = GetCommandLineW();
	int result = wWinMain(GetModuleHandleW(fpl_null), fpl_null, argsW, SW_SHOW);
	ExitProcess(result);
}
#				else
void __stdcall WinMainCRTStartup(void) {
	LPSTR argsA = GetCommandLineA();
	int result = WinMain(GetModuleHandleA(fpl_null), fpl_null, argsA, SW_SHOW);
	ExitProcess(result);
}
#				endif // UNICODE
#			elif defined(FPL_APPTYPE_CONSOLE)
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
#			else
#				error "Application type not set!"
#			endif // FPL_APPTYPE

#		else
			//
			// Win32 with CRT
			//
#			if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseWideArguments(cmdLine);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	return(result);
}
#			else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseAnsiArguments(cmdLine);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	return(result);
}
#			endif // UNICODE

#		endif // FPL_NO_CRT

#	endif // FPL_PLATFORM_WIN32

#endif // FPL_ENTRYPOINT && !FPL_ENTRYPOINT_IMPLEMENTED

// Undef useless constants for callers
#if !defined(FPL_NO_UNDEF)

#	if defined(FPL_SUBPLATFORM_X11)
#		undef Bool
#		undef Expose
#		undef KeyPress
#		undef KeyRelease
#		undef FocusIn
#		undef FocusOut
#		undef FontChange
#		undef None
#		undef Success
#		undef Status
#		undef Unsorted
#	endif // FPL_SUBPLATFORM_X11

#	if defined(FPL_PLATFORM_WIN32)
#		undef TRUE
#		undef FALSE
#		undef far
#		undef near
#		undef IN
#		undef OUT
#		undef OPTIONAL
#	endif // FPL_SUBPLATFORM_X11

#endif // !FPL_NO_UNDEF

//! \endcond

// end-of-file