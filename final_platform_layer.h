/*
final_platform_layer.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

Final Platform Layer is a Single-Header-File cross-platform C development library designed to abstract the underlying platform to a simple and easy-to-use API - providing low-level access to (Window, Video, Audio, Input, File/Path IO, Threads, Memory, Hardware, etc.).

The main focus is game/media/simulation development, so the default settings will create a window, set up an OpenGL rendering context, and initialize audio playback on any platform.

It is written in C99 for simplicity and best portability but is C++ compatible as well.

FPL supports the platforms Windows/Linux/Unix for the architectures x86/x64/arm.

The only dependencies are built-in operating system libraries and a C99 compliant compiler.

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
	// Create default settings
	fplSettings settings = fplMakeDefaultSettings();

	// Overwrite the video backend
	settings.video.backend = fplVideoBackendType_OpenGL;

	// Legacy OpenGL
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;

	// or

	// Modern OpenGL
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;

	if (fplPlatformInit(fplInitFlags_Video, &settings)) {
		// Event/Main loop
		while (fplWindowUpdate()) {
			// Poll events
			fplEvent ev;
			while (fplPollEvent(&ev)) {
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

Copyright (c) 2017-2023 Torsten Spaete

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
	@file final_platform_layer.h
	@version v0.9.8-beta
	@author Torsten Spaete
	@brief Final Platform Layer (FPL) - A C99 Single-Header-File Platform Abstraction Library
*/

// ----------------------------------------------------------------------------
// > CHANGELOG
// ----------------------------------------------------------------------------
/*!
	@page page_changelog Changelog
	@tableofcontents

	## v0.9.8-beta

	### Overview
	- Added useful functions for multithreading
	- Added support for changing the default window background color
	- Added support for preventing the screensaver for kicking-in
	- Several Bugfixes for several platforms
	- Several Bugfixes for several audio backends
	- Several Improvements for platforms
	- Renamed tons of functions to match naming scheme

	### Details

	#### Core
	- New: Added function GetAvailableThreadCount() that returns the number of available threads
	- New: Added function GetUsedThreadCount() that returns the number of used/active threads
	- New: Added union fplColor32 for representing a 32-bit color value
	- New: Added typedef fplMilliseconds that specifies milliseconds as 64-bit integer
	- New: Added typedef fplSeconds that specifies seconds as 64-bit floating point
	- New: Added field background as @ref fplColor32 to @ref fplWindowSettings
	- New: Added fields isScreenSaverPrevented/isMonitorPowerPrevented to configure, if monitor-off or screensaver is prevented
	- Fixed[#135]: Stackoverflow in fpl__PushError_Formatted() when FPL_USERFUNC_vsnprintf is overloaded
	- Fixed[#139]: Assertion on machine with 32 logical cores -> fplThreadHandle array capacity too small
	- Fixed[#141]: fplMemoryCopy() was wrong for 16-bit optimized operations
	- Changed[#95]: Platform support for fplMemorySet, fplMemoryClear, fplMemoryCopy (See FPL_NO_MEMORY_MACROS / FPL_USE_MEMORY_MACROS for more details)

	#### Platform
	- New[#150]: [Win32] Prevent screensaver for kicking-in
	- Fixed[#127]: [Win32/Window] Incorrect initial window background color
	- Fixed[#130]: [Win32/Window] Main fiber was never properly released
	- Fixed[#131]: [Win32/Console] Console window was not shown the second time fplPlatformInit() was called
	- Fixed[#134]: [Win32] Duplicate executable arguments in main() passed when CRT is disabled
	- Fixed[#137]: [POSIX] pthread_yield is not always present, so it may fail the FPL startup
	- Fixed[#138]: [X11] Compile error in function fpl__X11InitWindow, initSettings was not found

	#### Audio
	- Changed: [Audio/ALSA] Use *bcm2835* device pattern for buffer scale instead of individual ones
	
	#### Video
	- New: Added field libraryFile to @ref fplOpenGLSettings, for passing in a custom driver library file for OpenGL
	- New: Added field libraryFile to @ref fplVulkanSettings, for passing in a custom driver library file for Vulkan
	- New[#117]: Support for passing in a driver dll for OpenGL/Vulkan
	- Fixed[#136]: Video initialization failed due to wrong @ref fplGraphicsApiSettings union
	- Fixed[#124]: [Video/Vulkan] Fallback when creation with validation failed
	- Improvement[#148]: Refactoring of video backends

	#### Breaking Changes
	- Changed: Renamed function fplOpenBinaryFile() to fplFileOpenBinary()
	- Changed: Renamed function fplCreateBinaryFile() to fplFileCreateBinary()
	- Changed: Renamed functions fplReadFileBlock*() to fplFileReadBlock*()
	- Changed: Renamed functions fplWriteFileBlock*() to fplFileWriteBlock*()
	- Changed: Renamed functions fplSetFilePosition*() to fplFileSetPosition*()
	- Changed: Renamed functions fplGetFilePosition*() to fplFileGetPosition*()
	- Changed: Renamed function fplFlushFile() to fplFileFlush()
	- Changed: Renamed function fplCloseFile() to fplFileClose()
	- Changed: Renamed functions fplGetFileSizeFromPath*() to fplFileGetSizeFromPath*()
	- Changed: Renamed functions fplGetFileSizeFromHandle*() to fplFileGetSizeFromHandle*()
	- Changed: Renamed function fplGetFileTimestampsFromPath() to fplFileGetTimestampsFromPath()
	- Changed: Renamed function fplGetFileTimestampsFromHandle() to fplFileGetTimestampsFromHandle()
	- Changed: Renamed function fplSetFileTimestamps() to fplFileSetTimestamps()
	- Changed: Renamed function fplListDirBegin() to fplDirectoryListBegin()
	- Changed: Renamed function fplListDirNext() to fplDirectoryListNext()
	- Changed: Renamed function fplListDirEnd() to fplDirectoryListEnd()
	- Changed: Renamed function fplGetPlatformResultName() to fplPlatformGetResultName()
	- Changed: Renamed function fplFormatString() to fplStrngFormat()
	- Changed: Renamed function fplFormatStringArgs() to fplStrngFormatArgs()
	- Changed: Renamed function fplGetWallClock() to fplTimestampQuery()
	- Changed: Renamed function fplGetTimeInMilliseconds() to fplMillisecondsQuery()
	- Changed: Renamed function fplGetWallDelta() to fplTimestampElapsed()
	- Changed: Renamed struct fplWallClock to fplTimestamp
	- Changed: Removed obsolete functions fplGetTimeInSeconds*()
	- Changed: Removed obsolete functions fplGetTimeInMillisecondsHP()
	- Changed: Removed obsolete functions fplGetTimeInMillisecondsLP()
	- Changed: Replaced enum flag fplVulkanValidationLayerMode_User with fplVulkanValidationLayerMode_Optional
	- Changed: Replaced enum flag fplVulkanValidationLayerMode_Callback with fplVulkanValidationLayerMode_Required
	- Changed: Moved fplTimeoutValue and FPL_TIMEOUT_INFINITE to time sections

	## v0.9.7-beta

	### Short
	- Added Vulkan support
	- Added seamless window resizing support for Win32
	- Added query functions for accessing video backend handles
	- Improved video system stability
	- Improved console window handling for Win32
	- Fixed some major & minor bugs

	### Detail
	- New: Added structs fplVideoRequirements, fplVideoRequirementsVulkan
	- New: Added function fplGetVideoRequirements() to query any requirements for a particular video backend
	- New: Added macro fplAssertPtr() for simply (exp != fpl_null) assertions
	- New: Added typedef fplVulkanValidationLayerCallback
	- New: Added enums fplVulkanValidationLayerMode, fplVulkanValidationSeverity
	- New: Added struct fplVulkanSettings
	- New: Added structs fplVideoWindow, fplVideoWindowWin32, fplVideoWindowX11
	- New: Added structs fplVideoSurface, fplVideoSurfaceOpenGL, fplVideoSurfaceVulkan
	- New: Added function fplGetVideoSurface() for query the current @ref fplVideoSurface
	- New[#48]: Added function fplGetVideoProcedure() for query functions from the active video backend
	- New[#18]: [Win32] Support for message proc fibers to support seamless window resize -> works only with fplPollEvents()
	- New[#105]: [Win32] Added support for creating and using a console in addition to a window
	- New[#31]: [Win32] Added support for Vulkan
	- New[#32]: [X11] Added support for Vulkan

	- Changed: Changed video system to use jump tables instead, to support more backends in the future
	- Changed: Added field @ref fplVulkanSettings to @ref fplGraphicsApiSettings
	- Changed[#116]: AudioDriver/VideoDriver renamed to AudioBackend/VideoBackend
	- Changed[#98]: [Win32] Use YieldProcessor instead of SwitchToThread for fplThreadYield()
	- Changed[#111]: [Win32] Use SetConsoleTitle with @ref fplConsoleSettings.title
	- Changed[#113]: [Win32] Properly show window on initialize (Foreground, Focus)

	- Fixed[#109]: fplS32ToString is not working anymore
	- Fixed[#121]: fplMutexHandle isValid flag was never working properly
	- Fixed[#122]: Compile errors on Arm32/C99 for "asm volatile"
	- Fixed[#123]: Compile error for wrong usage of fplStructInit in fplCPUID() for non-x86 architectures
	- Fixed[#110]: [Win32] Erasing of background when no video driver hides window always
	- Fixed[#114]: [Win32] Console window does not appear at the very first

	## v0.9.6-beta
	### Features
	- New[#73]: Added fplAsm macro to handle different inline assembler keywords (Clang, GCC, MSVC)
	- New[#75]: Added fplAlignAs macro for aligning structures to N-bytes (Clang, GCC, MSVC, C++/11)
	- New[#79]: Added function fplGetAudioDriver()
	- New[#81]: Added function fplGetAudioBufferSizeInMilliseconds() to compute milliseconds from frame-count + sample-rate
	- New[#85]: Added fpl*_First and fpl*_Last to every enum
	- New[#84]: Added support for controlling the inclusion of platform includes with define FPL_NO_PLATFORM_INCLUDES
	- New[#84]: Added support for use opaque handles instead of OS handles with define FPL_OPAQUE_HANDLES
	- New[#75]: Added fplMinAlignment macro to get the minimum required alignment
	- New[#90]: Added struct fplThreadParameters
	- New[#90]: Added function fplThreadCreateWithParameters() which allows to set the stack size and the priority in the creation directly

	- New[#51]: [POSIX] Implemented fplGetThreadPriority() and fplSetThreadPriority()
	- New[#60]: [POSIX] Implemented fplGetCurrentThreadId()
	- New[#90]: [POSIX] Implemented fplThreadCreateWithParameters()
	- New[#90]: [Win32] Implemented fplThreadCreateWithParameters()
	- New[#86]: [X11] Implemented fplEnableWindowFullscreen() and fplDisableWindowFullscreen()

	### Improvements
	- Changed: New Changelog format with categories (Features, bugfixes, improvements, breaking changes, internal changes)
	- Changed[#74]: fplGetAudioDevices() allows to pass null-pointer as devices argument to return the number of audio devices only
	- Changed[#74]: fplWideStringToUTF8String() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplUTF8StringToWideString() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetExecutableFilePath() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetHomePath() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplExtractFilePath() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplChangeFileExtension() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplPathCombine() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplFormatStringArgs() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplFormatString() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetCurrentUsername() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetDisplayModes() allows to pass null-pointer as output argument to return the number of modes only
	- Changed[#74]: fplGetInputLocale() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetUserLocale() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetSystemLocale() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#74]: fplGetProcessorName() allows to pass null-pointer as output argument to return the number of characters only
	- Changed[#63]: Replaced usage of fplStackAllocate() with fpl__AllocateTemporaryMemory()
	- Changed[#92]: Use _offsetof() and ARRAY_SIZE() for fplArrayCount() instead
	- Updated: Better documentation for fplAssert*

	- Changed[#72]: [Win32] Query QueryPerformanceFrequency for every High-Precision timer calls instead of once per app start
	- Changed[#84]: [Win32] Moved windows.h include to the implementation and entry point block (all handles are void* or have correct size)

	### Bugfixes
	- Fixed[#76]: FPL__ERROR, FPL__WARNING, FPL__INFO was not passing the correct function name and line number in some cases
	- Fixed[#83]: fplGetAudioBufferSizeInFrames() does not return correct values always
	- Fixed[#83]: fplGetAudioBufferSizeInMilliseconds() does not return correct values always
	- Fixed[#94]: Fixed MINGW compile error for typo FPL_SUBPLATFORM_WINDOWS vs FPL_PLATFORM_WINDOWS (_WIN32_WINNT 0x0600)
	- Fixed: Corrected a few code documentation translation bugs

	- Fixed[#69]: [Win32] Removed the manual handling of ALT + F4 shut down of event handling
	- Fixed[#87]: [POSIX] fplGetWallDelta() was returning incorrect values

	### Internal changes
	- Renamed a lot of internal FPL_ defines to FPL__

	### Breaking changes
	- Removed[#85]: Removed obsolete FPL_FIRST_* and FPL_LAST_* defines

	- Renamed[#78]: fplGetAudioResultTypeString to fplGetAudioResultName
	- Renamed[#78]: fplGetArchTypeString to fplCPUGetArchName
	- Renamed[#78]: fplGetVideoDriverString to fplGetVideoDriverName
	- Renamed[#78]: fplGetAudioDriverString to fplGetAudioDriverName
	- Renamed[#78]: fplGetAudioFormatTypeString to fplGetAudioFormatName
	- Renamed[#78]: fplGetPlatformResultTypeString to fplGetPlatformResultName

	- Renamed[#50]: struct fplOSInfos to fplOSVersionInfos
	- Renamed[#50]: fplGetCurrentUsername() to fplSessionGetUsername()
	- Renamed[#50]: fplGetOperatingSystemInfos() to fplOSGetVersionInfos()
	- Renamed[#50]: fplIsAtomicCompareAndSwapPtr() to fplAtomicIsCompareAndSwapPtr()
	- Renamed[#50]: fplIsAtomicCompareAndSwapSize() to fplAtomicIsCompareAndSwapSize()
	- Renamed[#50]: fplIsAtomicCompareAndSwapS64() to fplAtomicIsCompareAndSwapS64()
	- Renamed[#50]: fplIsAtomicCompareAndSwapS32() to fplAtomicIsCompareAndSwapS32()
	- Renamed[#50]: fplIsAtomicCompareAndSwapU64() to fplAtomicIsCompareAndSwapU64()
	- Renamed[#50]: fplIsAtomicCompareAndSwapU32() to fplAtomicIsCompareAndSwapU32()
	- Renamed[#50]: fplXCR0() to fplCPUXCR0()
	- Renamed[#50]: fplRDTSC() to fplCPURDTSC()
	- Renamed[#50]: fplGetArchName() to fplCPUGetArchName()
	- Renamed[#50]: fplGetProcessorCoreCount() to fplCPUGetCoreCount()
	- Renamed[#50]: fplGetProcessorName() to fplCPUGetName()
	- Renamed[#50]: fplGetProcessorCapabilities() to fplCPUGetCapabilities()
	- Renamed[#50]: fplGetProcessorArchitecture() to fplCPUGetArchitecture()
	- Renamed[#50]: fplGetRunningMemoryInfos() to fplMemoryGetInfos()
	- Renamed[#50]: struct fplProcessorCapabilities to fplCPUCapabilities
	- Renamed[#50]: enum fplArchType to fplCPUArchType
	- Renamed[#50]: enum values fplArchType_* to fplCPUArchType_

	- Changed[#74]: fplWideStringToUTF8String() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplUTF8StringToWideString() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetExecutableFilePath() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetHomePath() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplExtractFilePath() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplChangeFileExtension() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplCopyString() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplCopyStringLen() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplPathCombine() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplFormatStringArgs() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplFormatString() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetCurrentUsername() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetInputLocale() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetUserLocale() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetSystemLocale() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetProcessorName() returns the number of characters instead of a char-pointer
	- Changed[#74]: fplGetDisplayModeCount() -> Use fplGetDisplayModes() with null-pointer instead

	## v0.9.5-beta
	- New: Added enum fplAudioDefaultFields
	- New: Added field defaultFields to fplAudioDeviceFormat struct
	- New: Added C++/11 detection (FPL_IS_CPP11)
	- New: Added enum fplAudioLatencyMode to fplAudioTargetFormat
	- New: Added function fplSetFileTimestamps()
	- New: Added fplAudioDriverType to fplAudioDeviceFormat
	- New: Added fplWallClock struct
	- New: Added function fplGetWallClock()
	- New: Added function fplGetWallDelta()
	- New: Support for logging out keyboard button events (FPL_LOG_KEY_EVENTS)
	- New: Added support for hard crashing on errors or warnings (define FPL_CRASH_ON_ERROR or FPL_CRASH_ON_WARNING) to enable it

	- New: [Win32] Added implementation for fplSetFileTimestamps()
	- New: [Win32] Added implementation for fplGetWallClock()
	- New: [Win32] Added implementation for fplGetWallDelta()

	- New: [POSIX] Added implementation for fplGetWallClock()
	- New: [POSIX] Added implementation for fplGetWallDelta()


	- Fixed: fplS32ToString() was not returning the last written character
	- Fixed: fplStringAppendLen() was not returning the last written character
	- Fixed: Fixed several warnings for doxygen

	- Fixed: [Core] Added empty functions for fplCPUID(), fplGetXCR0() for non-x86 platforms
	- Fixed: [Core] Implemented fplRDTSC() for non-x86 platforms

	- Fixed: [POSIX] Fixed several compile errors

	- Fixed: [Linux] Fixed non-joystick devices as gamepad detected
	- Fixed: [X11] Fixed keyrepeat issue for text input (finally!)
	- Fixed: [X11] Fixed duplicated key events in some cases

	- Changed: FPL_MAX_THREAD_COUNT and FPL_MAX_SIGNAL_COUNT can now be overridden by the user
	- Changed: Removed redundant field bufferSizeInBytes from fplAudioDeviceFormat struct
	- Changed: Simplified audio system default values initialization
	- Changed: Use default audio buffer size based on set fplAudioLatencyMode in fplAudioTargetFormat

	- Changed: [ALSA] Introduced audio buffer scaling for bad latency devices

	## v0.9.4 beta

	- New: Added callbacks fpl_memory_allocate_callback & fpl_memory_release_callback
	- New: Added enum fplMemoryAllocationMode
	- New: Added struct fplMemoryAllocationSettings
	- New: Added struct fplMemorySettings for controlling dynamic and temporary memory allocations
	- New: Added enum fplThreadPriority
	- New: Added function fplGetThreadPriority
	- New: Added function fplSetThreadPriority
	- New: Added function fplGetCurrentThreadId
	- New: Introduced dynamic/temporary allocations wrapping
	- New: Added FPL_WARNING macro for pushing on warnings only
	- New: Print out function name and line number in all log outputs
	- New: Added functions fplAtomicIncrement* used for incrementing a value by one atomically
	- New: Added macro fplRDTSC
	- New: Added struct fplProcessorCapabilities
	- New: Added function fplGetProcessorCapabilities
	- New: Added macro fplIsBitSet
	- New: Added function fplGetMainThread
	- New: Added field isActive to struct fplGamepadState
	- New: Added field deviceName to struct fplGamepadState and fplGamepadEvent
	- New: Added function fplSetWindowInputEvents
	- New: Added struct fplCPUIDLeaf
	- New: Added function fplCPUID and implemented it for X86/X64
	- New: Added function fplGetXCR0 and implemented it for X86/X64
	- New: Added macro fplHasInclude
	- New: Added power-pc 32/64 architecture detection
	- New: Added storage class identifier fpl_no_inline
	- New: Added macro fplAlwaysAssert()
	- New: Added function fplIsPlatformInitialized()
	- New: Added FPL_IS_IDE macro for checking if any source editor is active.
	- New: [MSVC] Always show implementation block when FPL_IS_IDE is set

	- Fixed: Corrected opengl example code in the header file
	- Fixed: Tons of documentation improvements
	- Fixed: fpl__PushError_Formatted was always pushing errors on regardless of the log level
	- Fixed: Invalid memory clear with zero bytes, when there was no audio samples to clear
	- Fixed: All non inlineable functions changed from fpl_internal_inline to fpl_internal instead (GCC/Clang compatible)
	- Fixed: Use actual window size for video initialization always instead of fixed size
	- Fixed: fplAtomicAddAndFetch* had no addend parameter
	- Fixed: All non-tab spacings replaced with tab spacings
	- Fixed: ARM64 was not detected properly
	- Fixed: Atomics was not detected for ICC (Intel C/C++ Compiler)

	- Changed: Removed fake thread-safe implementation of the internal event queue
	- Changed: Changed drop event structure in fplWindowEvent to support multiple dropped files
	- Changed: Renamed fplGetPlatformTypeString() to fplGetPlatformName()
	- Changed: Added stride to fplThreadWaitForAll() to support custom sized user structs
	- Changed: Added stride to fplThreadWaitForAny() to support custom sized user structs
	- Changed: Added stride to fplSignalWaitForAll() to support custom sized user structs
	- Changed: Added stride to fplSignalWaitForAny() to support custom sized user structs
	- Changed: Set audio worker thread to realtime priority
	- Changed: Use name table for fplArchType to string conversion
	- Changed: Use name table for fplPlatformType to string conversion
	- Changed: Use name table for fplPlatformResultType to string conversion
	- Changed: Use name table for fplVideoDriverType to string conversion
	- Changed: Use name table for fplAudioDriverType to string conversion
	- Changed: Use name table for fplAudioFormatType to string conversion
	- Changed: Use name table for fplLogLevel to string conversion
	- Changed: Values for fplPlatformResultType changed to be ascending
	- Changed: Moved fplInitFlags_All constant to fplInitFlags enum
	- Changed: CPU size detection is now independent from CPU architecture detection
	- Changed: fplGetProcessorName moved to common section with fallback to not supported architectures
	- Changed: Removed wrong compiler detection for LLVM (Clang is LLVM)
	- Changed: Removed pre-spaces from all code documentation comments
	- Changed: Changed from default audio sample rate of 48000 to 44100
	- Changed: Moved compiler includes into its own section
	- Changed: Moved rdtsc/cpuid, etc. into the implementation block
	- Changed: Renamed enum fplAudioResult to fplAudioResultType
	- Changed: Use fplLogLevel_Warning as max by default for all log functions
	- Renamed function fplGetAudioFormatString to fplGetAudioFormatTypeString

	- New: [Win32] Implemented function fplGetCurrentThreadId
	- New: [Win32] Support for multiple files in WM_DROPFILES
	- New: [Win32] Implemented fplGetThreadPriority
	- New: [Win32] Implemented fplSetThreadPriority
	- New: [Win32/Linux] Implemented isActive field for fplGamepadState for state querying, indicating any buttons are pressed or triggers have been moved
	- New: [Win32/XInput] Fill out the deviceName in fplGamepadState and fplGamepadEvent -> Generic name for now
	- New: [POSIX/Win32] Implemented functions fplAtomicIncrement*
	- New: [Linux/ALSA] Print compiler warning when alsa include was not found
	- New: [X11] Implemented fplSetWindowDecorated
	- New: [X11] Implemented fplIsWindowDecorated

	- Fixed: [Win32] Fixed missing WINAPI keyword for fpl__Win32MonitorCountEnumProc/fpl__Win32MonitorInfoEnumProc/fpl__Win32PrimaryMonitorEnumProc
	- Fixed: [Win32] Software video output was not outputing the image as top-down
	- Fixed: [Win32/X11] Mouse wheel delta message is not ignored anymore
	- Fixed: [X11] Fixed software video output (was broken)
	- Fixed: [POSIX] Moved __sync_synchronize before __sync_lock_test_and_set in fplAtomicStore*
	- Fixed: [POSIX/Win32] fplAtomicAddAndFetch* uses now addend parameter
	- Fixed: [Linux] Previous gamepad state was not cleared before filling in the new state
	- Fixed: [X11] Gamepad controller handling was broken

	- Changed: [POSIX] Use __sync_add_and_fetch instead of __sync_fetch_and_or in fplAtomicLoad*
	- Changed: [POSIX/Win32] When a dynamic library failed to load, it will push on a warning instead of a error
	- Changed: [POSIX/Win32] When a dynamic library procedure address failed to retrieve, it will push on a warning instead of a error
	- Changed: [POSIX/Win32] Reflect api changes for fplThreadWaitForAll()
	- Changed: [POSIX/Win32] Reflect api changes for fplThreadWaitForAny()
	- Changed: [POSIX/Win32] Reflect api changes for fplSignalWaitForAll()
	- Changed: [POSIX/Win32] Reflect api changes for fplSignalWaitForAny()
	- Changed: [X86/X64] fplGetProcessorName are only enabled on X86 or X64 architecture
	- Changed: [X86/X64] fplGetProcessorCapabilities are only enabled on X86 or X64 architecture
	- Changed: [MSVC] Implemented fplCPUID/fplGetXCR0 for MSVC
	- Changed: [GCC/Clang] Implemented fplCPUID/fplGetXCR0 for GCC/Clang
	- Changed: [Win32] Input events are not flushed anymore, when disabled

	## v0.9.3.0 beta
	- Changed: Renamed fplSetWindowFullscreen to fplSetWindowFullscreenSize
	- Changed: Replaced windowWidth and windowHeight from fplWindowSettings with windowSize structure
	- Changed: Replaced fullscreenWidth and fullscreenHeight from fplWindowSettings with fullscreenSize structure
	- Changed: Renamed macro FPL_PLATFORM_WIN32 to FPL_PLATFORM_WINDOWS
	- Changed: Renamed fplGetWindowArea() to fplGetWindowSize()
	- Changed: Renamed fplSetWindowArea() to fplSetWindowSize()
	- Changed: Renamed fplWindowSettings.windowTitle to fplWindowSettings.title
	- Changed: Reversed buffer/max-size argument of fplS32ToString()
	- Changed: Renamed fullPath into name in the fplFileEntry structure and limit its size to FPL_MAX_FILENAME_LENGTH
	- Changed: Introduced fpl__m_ for internal defines mapped to the public define
	- Changed: FPL_ENABLE renamed to FPL__ENABLE
	- Changed: FPL_SUPPORT renamed to FPL__SUPPORT
	- Changed: [CPP] Export every function without name mangling -> extern "C"
	- Changed: [Win32] Moved a bit of entry point code around, so that every linking configuration works
	- Fixed: fplPlatformInit() was using the width for the height for the default window size
	- Fixed: fplExtractFileExtension() was not favour the last path part
	- Fixed [Win32/MSVC]: Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here"
	- Fixed: [POSIX] Parse version string (isdigit was not found)
	- Fixed: [X11] Added missing fpl*Display* stubs
	- New: Added fplSetWindowFullscreenRect()
	- New: Added fplGetDisplayCount()
	- New: Added fplGetDisplays()
	- New: Added fplGetWindowDisplay()
	- New: Added fplGetPrimaryDisplay()
	- New: Added fplGetDisplayFromPosition()
	- New: Added fplGetDisplayModeCount()
	- New: Added fplGetDisplayModes()
	- New: Added fplGetWindowTitle()
	- New: Added docs back-references from atomic functions
	- New: Added support for compiling FPL as a dynamic library

	- Changed: [Win32] fplSetWindowFullscreenSize does not use virtual screen coordinates anymore
	- Changed: [Win32/POSIX] Store filename in fplFileEntry instead of the full path
	- Fixed: [Win32] fplGetExecutableFilePath was not returning the last written character
	- Fixed: [Win32] fplGetHomePath was not returning the last written character
	- New: [Win32/X11] Use fplWindowSettings.fullscreenRefreshRate for startup when needed
	- New: [Win32] Implemented fplSetWindowFullscreenRect()
	- New: [Win32] Implemented fplGetDisplayCount()
	- New: [Win32] Implemented fplGetDisplays()
	- New: [Win32] Implemented fplGetWindowDisplay()
	- New: [Win32] Implemented fplGetPrimaryDisplay()
	- New: [Win32] Implemented fplGetDisplayFromPosition()
	- New: [Win32] Implemented fplGetDisplayModeCount()
	- New: [Win32] Implemented fplGetDisplayModes()

	## v0.9.2.0 beta
	- Changed: Removed "Ansi" part from all ansi functions
	- Changed: Debug/Release user define is always prefered
	- Changed: Properly define posix defines like _XOPEN_SOURCE
	- Changed: Renamed FPL_STACKALLOCATE to fplStackAllocate
	- Changed: Renamed FPL_OFFSETOF to fplOffsetOf
	- Changed: Renamed FPL_ARRAYCOUNT to fplArrayCount
	- Changed: Renamed FPL_ASSERT to fplAssert
	- Changed: Renamed FPL_STATICASSERT to fplStaticAssert
	- Changed: Renamed FPL_CLEAR_STRUCT to fplClearStruct
	- Changed: Renamed FPL_MIN to fplMin
	- Changed: Renamed FPL_MAX to fplMax
	- Changed: Renamed FPL_ZERO_INIT to fplZeroInit
	- Changed: Renamed FPL_STRUCT_SET to fplStructSet
	- Changed: Renamed FPL_STRUCT_INIT to fplStructInit
	- Changed: Renamed FPL_KILOBYTES to fplKiloBytes
	- Changed: Renamed FPL_MEGABYTES to fplMegaBytes
	- Changed: Renamed FPL_GIGABYTES to fplGigaBytes
	- Changed: Renamed FPL_TERABYTES to fplTeraBytes
	- Changed: Renamed FPL_ALIGNMENT_OFFSET to fplGetAlignmentOffset
	- Changed: Renamed FPL_ALIGNED_SIZE to fplGetAlignedSize
	- Changed: Renamed FPL_IS_ALIGNED to fplIsAligned
	- Changed: Renamed FPL_IS_POWEROFTWO to fplIsPowerOfTwo
	- Changed: Renamed fplFormatAnsiStringArgs() to fplFormatStringArgs()
	- Changed: Renamed fplFormatAnsiString() to fplFormatString()
	- Changed: Renamed fplCopyAnsiString() to fplCopyString()
	- Changed: Renamed fplCopyAnsiStringLen() to fplCopyStringLen()
	- Changed: Renamed fplGetAnsiStringLength() to fplGetStringLength()
	- Changed: Renamed fplOpenAnsiBinaryFile() to fplOpenBinaryFile()
	- Changed: Renamed fplCreateAnsiBinaryFile() to fplCreateBinaryFile()
	- Changed: Renamed fplSetWindowAnsiTitle() to fplSetWindowTitle()
	- Changed: Renamed fplGetClipboardAnsiText() to fplGetClipboardText()
	- Changed: Renamed fplSetClipboardAnsiText() to fplSetClipboardText()
	- Changed: Renamed all fplAtomicCompareAndExchange*() to fplAtomicCompareAndSwap*()
	- Changed: Renamed all fplAtomicAdd*() to fplAtomicFetchAndAdd*()
	- Changed: Renamed all fplAtomicInc*() to fplAtomicAddAndFetch*()
	- Changed: Renamed fplGetRunningArchitecture() to fplGetProcessorArchitecture()
	- Changed: Renamed fplAudioSettings.deviceFormat to fplAudioSettings.targetFormat
	- Changed: Renamed fplAudioSettings.deviceInfo to fplAudioSettings.targetDevice
	- Changed: All buffers or fixed char/byte arrays uses now either FPL_MAX_BUFFER_LENGTH or FPL_MAX_NAME_LENGTH except for device-id
	- Changed: All callback typedefs are now properly named as such
	- Changed: Auto-play/stop of audio samples when enabled in configuration
	- Changed: fplPlayAudio() will return fplAudioResult_Success when playback is already started
	- Changed: fplStopAudio() will return fplAudioResult_Success when playback is already stopped
	- Changed: Use nullptr for fpl_null when C++/11 is detected
	- Changed: Separated audio target format and device format
	- Removed: Removed obsolete to fplGetWideStringLength()
	- Removed: Removed obsolete to fplCopyWideString()
	- Removed: Removed obsolete to fplCopyWideStringLen()
	- Removed: Removed obsolete fplOpenWideBinaryFile()
	- Removed: Removed obsolete fplCreateWideBinaryFile()
	- Removed: Removed obsolete fplSetWindowWideTitle()
	- Removed: Removed obsolete fplGetClipboardWideText()
	- Removed: Removed obsolete fplSetClipboardWideText()
	- Removed: Removed obsolete fplWideStringToAnsiString()
	- Removed: Removed obsolete fplAnsiStringToWideString()
	- Removed: fplUpdateGameControllers()
	- Removed: fplPushEvent()
	- Removed: fplClearEvents()
	- Fixed: fplStaticAssert was not compiling on gcc/clang C99 mode
	- Fixed: Corrected a ton of misspellings in the documentation
	- Fixed: Define for FPL_DEBUG was missing a raute symbol
	- Fixed: Use va_copy for all va_list function arguments
	- New: Added fplFlushFile()
	- New: Added fplAtomicAddAndFetchPtr()
	- New: Added fplAtomicFetchAndAddPtr()
	- New: Added startAuto field to fplAudioSettings structure
	- New: Added stopAuto field to fplAudioSettings structure
	- New: Added fplInitFlags_GameController to fplInitFlags enumeration
	- New: Added fplPollEvents()
	- New: Added typedef fpl_window_event_callback
	- New: Added typedef fpl_window_exposed_callback
	- New: Added structure fplWindowCallbacks as a field in fplWindowSettings
	- New: Added support for disabling runtime linking -> FPL_NO_RUNTIME_LINKING
	- New: Added structure fplAudioTargetFormat
	- New: Added macro fplCopyStruct()
	- New: Added macro fplIsBigEndian()
	- New: Added macro fplIsLittleEndian()
	- New: Added fplGetWindowState()
	- New: Added fplSetWindowState()

	- Changed: [Win32] GetTickCount() replaced with GetTickCount64()
	- Changed: [Win32] Use unicode (*W) win32 api functions for everything now
	- Changed: [Win32] fplGetOperatingSystemInfos uses additional RtlGetVersion
	- Changed: [DirectSound] fplGetAudioDevices() uses DirectSoundEnumerateW instead of DirectSoundEnumerateA
	- Changed: [Win32/X11] Changed event handling from indirect to direct
	- Changed[*]: Optional runtime linking for OS library calls
	- Fixed: [Win32] fplGetProcessorArchitecture was not handling the WOW64-case
	- Fixed: [Win32] fplGetClipboardAnsiText() was broken
	- Fixed: [Win32] Forced compile error when compiling on < vista (FPL uses several features which requires vista or higher)
	- Fixed: [Win32] fplGetOperatingSystemInfos had no WINAPI call defined for GetVersion prototype
	- Fixed: [Win32] fplSetWindowFloating was not working
	- Fixed: [Win32] fplSetWindowDecorated was not working
	- Fixed: [Win32] fplSetWindowResizeable was not working
	- Fixed: [Alsa] fpl__AudioWaitForFramesAlsa was not compiling (commonAudio missing)
	- Fixed: [Alsa] Alsa output was not working anymore for certain playback devices
	- Fixed: [X11] fplButtonState_Repeat was not handled in keyboard events
	- New: [Win32/POSIX] Implemented fplFlushFile()
	- New: [Win32/X11] Handle fplInitFlags_GameController to enable/disable game controllers
	- New: [Win32/X11] Support for handling the OS event directly -> fpl_window_event_callback
	- New: [Win32/X11] Support for handling the exposed (Repaint) event directly -> fpl_window_exposed_callback
	- New: [Win32] Implemented fplSetWindowState / fplGetWindowState

	## v0.9.1.0 beta
	- Changed: Updated all the lists
	- Changed: Added known issues section
	- Changed: Renamed fplPlatformResultType to fplPlatformResultType
	- Changed: fplPlatformInit returns now bool instead fplPlatformResultType
	- Changed: Changed all comment prefix from \\ to @@, so it matches java-doc style
	- Changed: Disable compile error when unix or bsd is detected
	- Changed: Renamed fplGetKeyboardState to fplPollKeyboardState
	- Changed: Renamed fplGetGamepadStates to fplPollGamepadStates
	- Changed: fplDebugOut prints out to the console on non-MSVC
	- Changed: Added enum value fplLogWriterFlags_StandardConsole
	- Changed: Added enum value fplLogWriterFlags_ErrorConsole
	- Changed: Removed field logToError from fplLogWriterConsole
	- Changed: Internal audio system uses condition variables instead of signals now
	- Removed: Removed obsolete field skipRepeatKeys in fplInputSettings
	- Fixed: Clang compiler detection was not working because LLVM was detected first
	- Fixed: UINT32_MAX was missing on android POSIX
	- Fixed: fplFormatAnsiStringArgs was checking for argList as pointer which is wrong
	- Fixed: leftTrigger/rightTrigger field in fplGamepadState had incorrect documentation
	- New: Added function fplGetPlatformResult()
	- New: Added struct fplMouseState
	- New: Added function fplPollMouseState()
	- New: Added field disabledEvents to fplInputSettings
	- New: Added android platform detection
	- New: Added enum fplGamepadButtonType

	- Changed: [POSIX] Proper detection of all architecturess (x86, x86_64, x64, arm32, arm64)
	- Changed: [POSIX] Moved fplGetCurrentUsername from the linux-section into the posix-section
	- Changed: [POSIX] Moved fplGetProcessorCoreCount from Linux into the POSIX section
	- Changed: [POSIX] Moved fplGetOperatingSystemInfos from Linux into the POSIX section
	- Changed: [POSIX] Moved fplGetHomePath from Linux into the POSIX section
	- Changed: [POSIX] Made fplGetExecutableFilePath Unix/Linux complaint and moved it into the POSIX section
	- Changed: [POSIX] Added Clang/LLVM to atomics detection
	- Fixed: [POSIX]: Removed alloca.h include when nor win32 or linux is detected
	- Fixed: [POSIX]: Fixed typo in fplGetRunningArchitecture
	- Fixed: [POSIX]: fplSemaphoreInit() was testing INT32_MAX instead of UINT32_MAX
	- Fixed: [Win32] PeekMessage was not using our windowHandle at all
	- Fixed: [Win32] fplReadFileBlock64/fplWriteFileBlock64 was not working at all
	- Fixed: [Win32/XInput] Left/Right gamepad thumb buttons was not mapped
	- Fixed: [POSIX] fplReadFileBlock64/fplWriteFileBlock64 was not working at all
	- Fixed: [POSIX] Removed initialization (PTHREAD_MUTEX_INITIALIZER) of pthread_mutex_t in fpl__PosixMutexCreate
	- Fixed: [X11] Fixed broken fplPollKeyboardState
	- Fixed: [MSVC] Removed duplicated warning override in header
	- New: [Win32]: Use SetCapture/ReleaseCapture for mouse down/up events
	- New: [Win32]: Implemented fplPollMouseState()
	- New: [Win32] Disable keyboard/mouse/gamepad events when fplInputSettings.disabledEvents is enabled
	- New: [POSIX] Added __USE_LARGEFILE64 before including sys/types.h
	- New: [X11] Implemented fplPollMouseState
	- New: [X11] Implemented modifier keys in fplPollKeyboardState
	- New: [Linux] Implemented fplPollGamepadStates

	## v0.9.0.1 beta
	- Changed: Renamed fields "kernel*" to "os*" in fplOSInfos
	- Changed: Renamed fields "system*" to "distribution*" in fplOSInfos
	- Fixed: [X11] Fixed icon loading was not working at all
	- Fixed: [POSIX] fplWriteFileBlock64 was not properly implemented
	- Fixed: [POSIX] fplReadFileBlock64 was not properly implemented

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
	- New: Added function fplIsAtomicCompareAndSwapSize
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
	- Changed: Changed from inline to api call for fplGetArchTypeString / fplGetPlatformResultTypeString / fplGetPlatformTypeString
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
	- Fixed: MSVC _Interlocked* replaced with Interlocked* macros

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
	@page page_support_status Support/Platform Status
	@tableofcontents

	@section section_support_status_supported_archs Supported Architectures

	- x86
	- x86_64
	- x64 (untested)
	- Arm32 (partially)
	- Arm64 (untested)

	@section section_support_status_supported_platforms Supported Platforms

	- Windows 7 or higher
	- Linux 2.6 or higher
	- Unix/BSD (Partially)
	- Raspberry Pi (Partially)

	@section section_support_status_supported_compilers Supported Compilers

	- MSVC
	- GCC
	- CLANG
	- Intel (untested)
	- MingW32 (untested)
	- MingW32 (untested)
	- CC ARM (untested)

	@section section_support_status_supported_subplatforms Supported Sub-Platforms

	FPL contains special code blocks which may be shared across platforms.

	- STD Strings
	- STD Console
	- POSIX
	- X11
*/

// ****************************************************************************
//
// > HEADER
//
// ****************************************************************************
#ifndef FPL_HEADER_H
#define FPL_HEADER_H

//
// C99 detection
//
// https://en.wikipedia.org/wiki/C99#Version_detection
// C99 is partially supported since MSVC 2015
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! C99 compiler detected
#	define FPL_IS_C99
#elif defined(__cplusplus)
	//! C++ compiler detected
#	define FPL_IS_CPP
#	if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
		//! C++/11 compiler detected
#		define FPL_IS_CPP11
#	endif
#else
#	error "This C/C++ compiler is not supported!"
#endif

//
// Architecture detection (x86, x64)
// https://sourceforge.net/p/predef/wiki/Architectures/
//
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#	define FPL_ARCH_X64
#elif defined(__i386__) || defined(_M_IX86) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#	define FPL_ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#	define FPL_ARCH_ARM32
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(_ARCH_PPC64)
#	define FPL_ARCH_POWERPC64
#elif defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
#	define FPL_ARCH_POWERPC32
#else
#	error "This architecture is not supported!"
#endif // FPL_ARCH

//
// 32-bit or 64-bit
//
#if defined(_WIN32)
#	if defined(_WIN64)
#		define FPL__M_CPU_64BIT
#	else
#		define FPL__M_CPU_32BIT
#	endif
#elif defined(__GNUC__)
#	if defined(__LP64__)
#		define FPL__M_CPU_64BIT
#	else
#		define FPL__M_CPU_32BIT
#	endif
#else
#	if (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8) || (sizeof(void *) == 8)
#		define FPL__M_CPU_64BIT
#	else
#		define FPL__M_CPU_32BIT
#	endif
#endif

#if defined(FPL__M_CPU_64BIT)
	//! 64-bit CPU detected
#	define FPL_CPU_64BIT
#elif defined(FPL__M_CPU_32BIT)
	//! 32-bit CPU detected
#	define FPL_CPU_32BIT
#endif

//
// Compiler detection
// http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
//
#if defined(__clang__)
	//! CLANG compiler detected
#	define FPL_COMPILER_CLANG
#elif defined(__INTEL_COMPILER)
	//! Intel compiler detected
#	define FPL_COMPILER_INTEL
#elif defined(__MINGW32__)
	//! MingW compiler detected
#	define FPL_COMPILER_MINGW
#elif defined(__CC_ARM)
	//! ARM compiler detected
#	define FPL_COMPILER_ARM
#elif defined(__GNUC__)
	//! GCC compiler detected
#	define FPL_COMPILER_GCC
#elif defined(_MSC_VER)
	//! Visual studio compiler detected
#	define FPL_COMPILER_MSVC
#else
	//! No compiler detected
#error "This compiler is not supported!"
#endif // FPL_COMPILER

//
// Platform detection
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
//
#if defined(_WIN32) || defined(_WIN64)
#	define FPL_PLATFORM_WINDOWS
#	define FPL_PLATFORM_NAME "Windows"
#elif defined(__ANDROID__)
#	define FPL_PLATFORM_ANDROID
#	define FPL_PLATFORM_NAME "Android"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
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
#elif defined(unix) || defined(__unix) || defined(__unix__)
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "Unix"
#	define FPL_SUBPLATFORM_POSIX
#	define FPL_SUBPLATFORM_X11
#	define FPL_SUBPLATFORM_STD_STRINGS
#	define FPL_SUBPLATFORM_STD_CONSOLE
#else
#	error "This platform is not supported!"
#endif // FPL_PLATFORM

// Assembler keyword is compiler specific
#if defined(FPL_COMPILER_CLANG) || defined(FPL_COMPILER_GCC)
#define fpl__m_Asm __asm__
#elif defined(FPL_COMPILER_MSVC)
#define fpl__m_Asm __asm
#else
#define fpl__m_Asm asm
#endif

//! A assembler compiler instruction (asm)
#define fplAsm fpl__m_Asm

// Minimum alignment
#if defined(FPL_COMPILER_MSVC)
#	define fpl__MinAlignment 8
#elif defined(FPL_COMPILER_GCC) || defined(FPL_COMPILER_CLANG)
#	if defined(FPL_CPU_64BIT)
#		define fpl__MinAlignment 8
#	else
#		define fpl__MinAlignment 4
#	endif
#else
#	define fpl__MinAlignment 8
#endif
//! Minimum structure alignment
#define fplMinAlignment fpl__MinAlignment

// Alignment keyword
#if defined(FPL_IS_CPP11)
#define fpl__m_AlignAs(N) alignas(N)
#elif defined(FPL_COMPILER_MSVC)
#define fpl__m_AlignAs(N) __declspec(align(N))
#elif defined(FPL_COMPILER_GCC) || defined(FPL_COMPILER_CLANG)
#define fpl__m_AlignAs(N) __attribute__((aligned(N)))
#else
#define fpl__m_AlignAs(N)
#endif
#define fpl__m_AlignAsAuto(N) fpl__m_AlignAs(((N) < fplMinAlignment ? fplMinAlignment : (N)))

//! Structure alignment in bytes
#define fplAlignAs(N) fpl__m_AlignAsAuto(N)

//
// Defines required for POSIX (mmap, 64-bit file io, etc.)
//
#if defined(FPL_SUBPLATFORM_POSIX)
#	if !defined(_XOPEN_SOURCE)
#		define _XOPEN_SOURCE 600
#	endif
#	if !defined(_DEFAULT_SOURCE)
#		define _DEFAULT_SOURCE 1
#	endif
#	if !defined(__STDC_FORMAT_MACROS)
#		define __STDC_FORMAT_MACROS
#	endif
#	if !defined(__STDC_LIMIT_MACROS)
#		define __STDC_LIMIT_MACROS
#	endif
#	if !defined(_LARGEFILE_SOURCE)
#		define _LARGEFILE_SOURCE
#	endif
#	if !defined(_LARGEFILE64_SOURCE)
#		define _LARGEFILE64_SOURCE
#	endif
#	if !defined(_FILE_OFFSET_BITS)
#		define _FILE_OFFSET_BITS 64
#	endif
#endif

#if defined(FPL_PLATFORM_LINUX)
#	define FPL__INCLUDE_ALLOCA
#else
#	define FPL__INCLUDE_MALLOC
#endif

// MingW compiler hack
#if defined(FPL_PLATFORM_WINDOWS) && defined(FPL_COMPILER_MINGW)
#	if !defined(_WIN32_WINNT)
#		define _WIN32_WINNT 0x0600
#	endif //!_WIN32_WINNT
#endif // FPL_COMPILER_MINGW

//
// Storage class identifiers
//

/**
* @defgroup StorageClassIdents Storage class identifiers
* @brief This category contains storage class identifiers, such as static, extern, inline, etc.
* @{
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
//! External call
#if defined(FPL_IS_CPP)
#	define fpl_extern
#else
#	define fpl_extern extern
#endif

//
// DLL Export/Import definition
//
#if defined(_WIN32) || defined(__CYGWIN__)
#	ifdef __GNUC__
#		define fpl__m_dllexport __attribute__ ((dllexport))
#		define fpl__m_dllimport __attribute__ ((dllimport))
#	else
#		define fpl__m_dllexport __declspec(dllexport)
#		define fpl__m_dllimport __declspec(dllimport)
#	endif
#	define fpl__m_dlllocal
#else
#	if __GNUC__ >= 4
#		define fpl__m_dllimport __attribute__((visibility("default")))
#		define fpl__m_dllexport __attribute__((visibility("default")))
#		define fpl__m_dlllocal __attribute__((visibility("hidden")))
#	else
#		define fpl__m_dllimport
#		define fpl__m_dllexport
#		define fpl__m_dlllocal
#	endif
#endif

//! Link-library Import
#define fpl_dllimport fpl__m_dllimport
//! Link-library Export
#define fpl_dllexport fpl__m_dllexport
//! Link-library Local
#define fpl_dlllocal fpl__m_dlllocal

//
// API Call
//
#if defined(FPL_API_AS_PRIVATE)
#	define fpl__m_api static
#elif defined(FPL_DLLEXPORT)
#	define fpl__m_api fpl_dllexport
#elif defined(FPL_DLLIMPORT)
#	define fpl__m_api fpl_dllimport
#else
#	define fpl__m_api fpl_extern
#endif // FPL_API_AS_PRIVATE

//! Api call
#define fpl_api fpl__m_api

//! Main entry point api definition
#define fpl_main

#if defined(FPL_IS_CPP)
#	define fpl__m_platform_api extern "C" fpl_api
#	define fpl__m_common_api extern "C" fpl_api
#else
#	define fpl__m_platform_api fpl_api
#	define fpl__m_common_api fpl_api
#endif

//! Platform api
#define fpl_platform_api fpl__m_platform_api
//! Common api
#define fpl_common_api fpl__m_common_api

//
// Inlining
//
#if defined(FPL_COMPILER_MSVC)
#	define fpl__m_force_inline __forceinline
#	define fpl__m_no_inline __declspec(noinline)
#elif defined(FPL_COMPILER_GCC) || defined(FPL_COMPILER_CLANG)
#	define fpl__m_force_inline __attribute__((__always_inline__)) inline
#	define fpl__m_no_inline __attribute__((noinline))
#else
#	define fpl__m_force_inline inline
#	define fpl__m_no_inline
#endif

//! Always inlines this function
#define fpl_force_inline fpl__m_force_inline
//! Prevents inlining of this function
#define fpl_no_inline fpl__m_no_inline

/** @} */

//
// When C-Runtime is disabled we cannot use any function from the C-Standard Library <stdio.h> or <stdlib.h>
//
#if defined(FPL_NO_CRT)
#	if defined(FPL_SUBPLATFORM_STD_CONSOLE)
#		undef FPL_SUBPLATFORM_STD_CONSOLE
#	endif
#	if defined(FPL_SUBPLATFORM_STD_STRINGS)
#		undef FPL_SUBPLATFORM_STD_STRINGS
#	endif
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
// Debug/Release detection
//
#if defined(FPL_DEBUG)
#	define FPL__ENABLE_DEBUG
#elif defined(FPL_RELEASE)
#	define FPL__ENABLE_RELEASE
#endif

//
// Compiler settings
//
#if defined(FPL_COMPILER_MSVC)
	// Debug/Release detection
#	if !defined(FPL__ENABLE_DEBUG) && !defined(FPL__ENABLE_RELEASE)
#		if defined(_DEBUG) || (!defined(NDEBUG))
#			define FPL__ENABLE_DEBUG
#		else
#			define FPL__ENABLE_RELEASE
#		endif
#	endif

	// Function name macro (Win32)
#	define FPL__M_FUNCTION_NAME __FUNCTION__

	// Setup MSVC subsystem hints
#	if defined(FPL_APPTYPE_WINDOW)
#		pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#	elif defined(FPL_APPTYPE_CONSOLE)
#		pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#	endif

	// Setup MSVC linker hints
#	pragma comment(lib, "kernel32.lib")
#else
	// Function name macro (Other compilers)
#	define FPL__M_FUNCTION_NAME __FUNCTION__
#endif // FPL_COMPILER

// Debug Release fallback
#if !defined(FPL__ENABLE_DEBUG) && !defined(FPL__ENABLE_RELEASE)
#	define FPL__ENABLE_DEBUG
#endif

//! Is any IDE active, such as Intellisense or any jetbrains IDE?
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__)
#	define FPL_IS_IDE 1
#else
#	define FPL_IS_IDE 0
#endif

//! Function name macro
#define FPL_FUNCTION_NAME FPL__M_FUNCTION_NAME

//
// Options & Feature detection
//

//
// CPU Instruction Set Detection based on compiler settings
//
#if defined(__AVX512F__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 9
#elif defined(__AVX2__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 8
#elif defined(__AVX__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 7
#elif defined(__SSE4_2__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 6
#elif defined(__SSE4_1__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 5
#elif defined(__SSSE3__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 4
#elif defined(__SSE3__)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 3
#elif defined(__SSE2__) || (_M_IX86_FP >= 2)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 2
#elif defined(__SSE__) || (_M_IX86_FP >= 1)
#	define FPL__COMPILED_X86_CPU_INSTR_SET 1
#elif defined(_M_IX86_FP)
#	define FPL__COMPILED_X86_CPU_INSTR_SET _M_IX86_FP
#else
#	define FPL__COMPILED_X86_CPU_INSTR_SET 0
#endif

//
// Assertions
//
#if !defined(FPL_NO_ASSERTIONS)
#	if !defined(FPL_FORCE_ASSERTIONS)
#		if defined(FPL__ENABLE_DEBUG)
#			define FPL__ENABLE_ASSERTIONS
#		endif
#	else
#		define FPL__ENABLE_ASSERTIONS
#	endif
#endif // !FPL_NO_ASSERTIONS
#if defined(FPL__ENABLE_ASSERTIONS)
#	if !defined(FPL_NO_C_ASSERT) && !defined(FPL_NO_CRT)
#		define FPL__ENABLE_C_ASSERT
#	endif
#endif // FPL__ENABLE_ASSERTIONS

//
// HasInclude
//
#if defined(__has_include)
#	define fpl__m_HasInclude(inc) __has_include(inc)
#else
#	define fpl__m_HasInclude(inc) (1)
#endif
//! Test for include existence.
#define fplHasInclude(inc) fpl__m_HasInclude(inc)

//
// Window
//
#if !defined(FPL_NO_WINDOW) && !defined(FPL_APPTYPE_CONSOLE)
#	define FPL__SUPPORT_WINDOW
#endif

//
// Video
//
#if !defined(FPL_NO_VIDEO)
#	define FPL__SUPPORT_VIDEO
#endif
#if defined(FPL__SUPPORT_VIDEO)
#	if !defined(FPL_NO_VIDEO_OPENGL)
#		define FPL__SUPPORT_VIDEO_OPENGL
#	endif
#	if !defined(FPL_NO_VIDEO_VULKAN)
#		define FPL__SUPPORT_VIDEO_VULKAN
#	endif
#	if !defined(FPL_NO_VIDEO_SOFTWARE)
#		define FPL__SUPPORT_VIDEO_SOFTWARE
#	endif
#endif // FPL__SUPPORT_VIDEO

//
// Audio
//
#if !defined(FPL_NO_AUDIO)
	// Audio support
#	define FPL__SUPPORT_AUDIO
#endif
#if defined(FPL__SUPPORT_AUDIO)
#	if !defined(FPL_NO_AUDIO_DIRECTSOUND) && defined(FPL_PLATFORM_WINDOWS)
#		define FPL__SUPPORT_AUDIO_DIRECTSOUND // <dsound.h> is always present on windows
#	endif
#	if !defined(FPL_NO_AUDIO_ALSA) && defined(FPL_PLATFORM_LINUX)
#		if fplHasInclude(<alsa/asoundlib.h>)
#			define FPL__SUPPORT_AUDIO_ALSA
#		else
#			warning "FPL-Warning: ALSA audio development library is missing. Please install 'libasound2-dev' and try again!"
#		endif
#	endif
#endif // FPL__SUPPORT_AUDIO

//
// Remove video support when the window is disabled
//
#if !defined(FPL__SUPPORT_WINDOW)
#	if defined(FPL_SUBPLATFORM_X11)
#		undef FPL_SUBPLATFORM_X11
#	endif

#	if defined(FPL__SUPPORT_VIDEO)
#		undef FPL__SUPPORT_VIDEO
#	endif
#	if defined(FPL__SUPPORT_VIDEO_OPENGL)
#		undef FPL__SUPPORT_VIDEO_OPENGL
#	endif
#	if defined(FPL__SUPPORT_VIDEO_VULKAN)
#		undef FPL__SUPPORT_VIDEO_VULKAN
#	endif
#	if defined(FPL__SUPPORT_VIDEO_SOFTWARE)
#		undef FPL__SUPPORT_VIDEO_SOFTWARE
#	endif
#endif // !FPL__SUPPORT_WINDOW

//
// Enable supports (FPL uses _ENABLE_ internally only)
//
#if defined(FPL__SUPPORT_WINDOW)
#	define FPL__ENABLE_WINDOW
#endif

#if defined(FPL__SUPPORT_VIDEO)
#	define FPL__ENABLE_VIDEO
#	if defined(FPL__SUPPORT_VIDEO_OPENGL)
#		define FPL__ENABLE_VIDEO_OPENGL
#	endif
#	if defined(FPL__SUPPORT_VIDEO_VULKAN)
#		define FPL__ENABLE_VIDEO_VULKAN
#	endif
#	if defined(FPL__SUPPORT_VIDEO_SOFTWARE)
#		define FPL__ENABLE_VIDEO_SOFTWARE
#	endif
#endif // FPL__SUPPORT_VIDEO

#if defined(FPL__SUPPORT_AUDIO)
#	define FPL__ENABLE_AUDIO
#	if defined(FPL__SUPPORT_AUDIO_DIRECTSOUND)
#		define FPL__ENABLE_AUDIO_DIRECTSOUND
#	endif
#	if defined(FPL__SUPPORT_AUDIO_ALSA)
#		define FPL__ENABLE_AUDIO_ALSA
#	endif
#endif // FPL__SUPPORT_AUDIO

#if defined(FPL_LOGGING)
#	define FPL__ENABLE_LOGGING
#	if defined(FPL_LOG_MULTIPLE_WRITERS)
#		define FPL__ENABLE_LOG_MULTIPLE_WRITERS
#	endif
#endif

//
// Assertions & Debug
//

/**
* @defgroup Debug Assertion & Debug
* @brief This category contains assertion & debug macro functions
* @{
*/

#if defined(FPL__ENABLE_ASSERTIONS)
#	if defined(FPL__ENABLE_C_ASSERT) && !defined(FPL_FORCE_ASSERTIONS)
#		define FPL__INCLUDE_ASSERT
#		define fpl__m_Assert(exp) assert(exp)
#		if defined(__cplusplus)
#			define fpl__m_StaticAssert(exp) static_assert(exp, "fpl_static_assert")
#		endif
#	else
#		define fpl__m_Assert(exp) if(!(exp)) {*(int *)0 = 0;}
#	endif // FPL__ENABLE_C_ASSERT
#	if !defined(fpl__m_StaticAssert)
#		define FPL__M_STATICASSERT_0(exp, line, counter) \
			int fpl__ct_assert_##line_##counter(int ct_assert_failed[(exp)?1:-1])
#		define fpl__m_StaticAssert(exp) \
			FPL__M_STATICASSERT_0(exp, __LINE__, __COUNTER__)
#	endif
#else
#	define fpl__m_Assert(exp)
#	define fpl__m_StaticAssert(exp)
#endif // FPL__ENABLE_ASSERTIONS

//! Breaks with an runtime assertion, when the specified expression evaluates to @c false
#define fplAssert(exp) fpl__m_Assert(exp)
//! Breaks the compilation, when the specified expression evaluates to @c false
#define fplStaticAssert(exp) fpl__m_StaticAssert(exp)
//! Always crashes the application with a null-pointer assignment, when the specified expression evaluates to @c false
#define fplAlwaysAssert(exp) if(!(exp)) {*(int *)0 = 0;}
//! Breaks when the specified pointer is @ref fpl_null
#define fplAssertPtr(ptr) fpl__m_Assert((ptr) != fpl_null)

//
// Debug-Break
// Based on: https://stackoverflow.com/questions/173618/is-there-a-portable-equivalent-to-debugbreak-debugbreak
//
#if defined(__has_builtin)
#	if __has_builtin(__builtin_debugtrap)
#		define fpl__m_DebugBreak() __builtin_debugtrap()
#	elif __has_builtin(__debugbreak)
#		define fpl__m_DebugBreak() __debugbreak()
#	endif
#endif
#if !defined(fpl__m_DebugBreak)
#	if defined(FPL_COMPILER_MSVC) || defined(FPL_COMPILER_INTEL)
#		define fpl__m_DebugBreak() __debugbreak()
#	elif defined(FPL_COMPILER_ARM)
#		define fpl__m_DebugBreak() __breakpoint(42)
#	elif defined(FPL_ARCH_X86) || defined(FPL_ARCH_X64)
fpl_internal fpl_force_inline void fpl__m_DebugBreak() { __asm__ __volatile__("int $03"); }
#	elif defined(__thumb__)
fpl_internal fpl_force_inline void fpl__m_DebugBreak() { __asm__ __volatile__(".inst 0xde01"); }
#	elif defined(FPL_ARCH_ARM64)
fpl_internal fpl_force_inline void fpl__m_DebugBreak() { __asm__ __volatile__(".inst 0xd4200000"); }
#	elif defined(FPL_ARCH_ARM32)
fpl_internal fpl_force_inline void fpl__m_DebugBreak() { __asm__ __volatile__(".inst 0xe7f001f0"); }
#	elif defined(FPL_COMPILER_GCC)
#		define fpl__m_DebugBreak() __builtin_trap()
#	else
#		define FPL__INCLUDE_SIGNAL
#		if defined(SIGTRAP)
#			define fpl__m_DebugBreak() raise(SIGTRAP)
#		else
#			define fpl__m_DebugBreak() raise(SIGABRT)
#		endif
#	endif
#endif

//! Stops the debugger on this line always
#define fplDebugBreak() fpl__m_DebugBreak()

/** @} */

//
// Memory macros
//

/**
* @defgroup Memory macros
* @brief This category contains memory configurations
* @{
*/

#if !defined(FPL_NO_MEMORY_MACROS) || defined(FPL_FORCE_MEMORY_MACROS)
#	define FPL__ENABLE_MEMORY_MACROS
#endif

/** @} */

//
// Types & Limits
//
#include <stdint.h> // uint32_t, ...
#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <limits.h> // UINT32_MAX, ...
#if defined(FPL__INCLUDE_ASSERT)
#	include <assert.h>
#endif
#if defined(FPL__INCLUDE_SIGNAL)
#	include <signal.h>
#endif
#if defined(FPL__INCLUDE_MALLOC)
#	include <malloc.h>
#endif
#if defined(FPL__INCLUDE_ALLOCA)
#	include <alloca.h>
#endif

/// @cond FPL_INTERNALS
#if !defined(UINT32_MAX)
	// On android or older posix versions there is no UINT32_MAX
#	define UINT32_MAX ((uint32_t)-1)
#endif
/// @endcond

#if defined(FPL_IS_CPP11)
#	define fpl__m_null nullptr
#elif defined(NULL)
#	define fpl__m_null NULL
#else
#	define fpl__m_null 0
#endif
//! Null
#define fpl_null fpl__m_null

//! 32-bit boolean
typedef int32_t fpl_b32;

//
// Test sizes
//
//! @cond FPL_INTERNAL
#if defined(FPL_CPU_64BIT)
fplStaticAssert(sizeof(uintptr_t) >= sizeof(uint64_t));
fplStaticAssert(sizeof(size_t) >= sizeof(uint64_t));
#elif defined(FPL_CPU_32BIT)
fplStaticAssert(sizeof(uintptr_t) >= sizeof(uint32_t));
fplStaticAssert(sizeof(size_t) >= sizeof(uint32_t));
#endif
//! @endcond

//
// Macro functions
//

/**
* @defgroup Macros Function macros
* @brief This category contains several useful macro functions
* @{
*/

//! This will full-on crash when something is not implemented always
#define FPL_NOT_IMPLEMENTED {*(int *)0 = 0xBAD;}

#if defined(FPL_IS_C99)
#	define fpl__m_ZeroInit {0}
#	define fpl__m_StructSet(ptr, type, value) *(ptr) = (type)value
#	define fpl__m_StructInit(type, ...) (type){__VA_ARGS__}
#else
#	define fpl__m_ZeroInit {}
#	define fpl__m_StructSet(ptr, type, value) *(ptr) = value
#	define fpl__m_StructInit(type, ...) {__VA_ARGS__}
#endif

//! Initializes a struct to zero
#define fplZeroInit fpl__m_ZeroInit
//! Sets a struct pointer to the given value
#define fplStructSet fpl__m_StructSet
//! Initializes a struct by the given type
#define fplStructInit fpl__m_StructInit

//! Returns the offset for the value to satisfy the given alignment boundary
#define fplGetAlignmentOffset(value, alignment) ( (((alignment) > 1) && (((value) & ((alignment) - 1)) != 0)) ? ((alignment) - ((value) & (alignment - 1))) : 0)			
//! Returns the given size, extended to satisfy the given alignment boundary
#define fplGetAlignedSize(size, alignment) (((size) > 0 && (alignment) > 0) ? ((size) + fplGetAlignmentOffset(size, alignment)) : (size))
//! Returns true when the given pointer address is aligned to the given alignment
#define fplIsAligned(ptr, alignment) (((uintptr_t)(const void *)(ptr)) % (alignment) == 0)
//! Returns true when the given value is a power of two value
#define fplIsPowerOfTwo(value) (((value) != 0) && (((value) & (~(value) + 1)) == (value)))
//! Returns true when the given platform is big-endian
#define fplIsBigEndian() (*(uint16_t *)"\0\xff" < 0x100)
//! Returns true when the given platform is little-endian
#define fplIsLittleEndian() (!fplIsBigEndian())
//! Returns true when the given value has the given bit set
#define fplIsBitSet(value, bit) (((value) >> (bit)) & 0x1)

//! Returns the number of bytes for the given kilobytes
#define fplKiloBytes(value) (((value) * 1024ull))
//! Returns the number of bytes for the given megabytes
#define fplMegaBytes(value) ((fplKiloBytes(value) * 1024ull))
//! Returns the number of bytes for the given gigabytes
#define fplGigaBytes(value) ((fplMegaBytes(value) * 1024ull))
//! Returns the number of bytes for the given terabytes
#define fplTeraBytes(value) ((fplGigaBytes(value) * 1024ull))

//! Clears the given struct pointer to zero
#define fplClearStruct(ptr) fplMemoryClear((void *)(ptr), sizeof(*(ptr)))
//! Copies the given source struct into the destination struct
#define fplCopyStruct(src, dst) fplMemoryCopy(src, sizeof(*(src)), dst);

// Array count
#if defined(FPL_COMPILER_MSVC) && !defined(FPL_NO_CRT) // @TODO(final): Find a better way to detect no-crt inclusion!
#	define fpl__m_ArrayCount(arr) _countof(arr)
#elif defined(ARRAY_SIZE)
#	define fpl__m_ArrayCount(arr) ARRAY_SIZE(arr)
#else
	//! The @ref fplArrayCount() validation is disabled
#	define FPL__NO_ARRAYCOUNT_VALIDATION
#	define fpl__m_ArrayCount(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
//! Returns the element count from a static array. This should ideally produce a compile error when passing a pointer to it.
#define fplArrayCount(arr) fpl__m_ArrayCount(arr)

//! Returns the offset in bytes for the specified structure type and field name
#define fplOffsetOf(type, field) ((size_t)(&(((type*)(0))->field)))

//! Returns the smallest value of A and B
#define fplMin(a, b) ((a) < (b) ? (a) : (b))

//! Returns the biggest value of A and B
#define fplMax(a, b) ((a) > (b) ? (a) : (b))

#if defined(FPL_PLATFORM_WINDOWS)
#	define fpl__m_StackAllocate(size) _alloca(size)
#else
#	define fpl__m_StackAllocate(size) alloca(size)
#endif

//! Manually allocate the number of specified bytes of memory on the stack
#define fplStackAllocate(size) fpl__m_StackAllocate(size)

/** @} */

#if defined(FPL_IS_CPP)
#	define FPL__M_ENUM_AS_FLAGS_OPERATORS(etype) \
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
#	define FPL__M_ENUM_AS_FLAGS_OPERATORS(etype)
#endif

//! Macro for optionally adding enum operators for bitwise and/or/xor
#define FPL_ENUM_AS_FLAGS_OPERATORS(type) FPL__M_ENUM_AS_FLAGS_OPERATORS(type)

// ****************************************************************************
//
// Platform Includes
//
// ****************************************************************************
#if !defined(FPL_NO_PLATFORM_INCLUDES) && !defined(FPL__HAS_PLATFORM_INCLUDES)
#	define FPL__HAS_PLATFORM_INCLUDES

#	if defined(FPL_PLATFORM_WINDOWS)
		// @NOTE(final): windef.h defines min/max macros in lowerspace, this will break for example std::min/max, so we have to tell the header we dont want this!
#		if !defined(NOMINMAX)
#			define NOMINMAX
#		endif
		// @NOTE(final): For now we dont want any network, com or gdi stuff at all, maybe later who knows.
#		if !defined(WIN32_LEAN_AND_MEAN)
#			define WIN32_LEAN_AND_MEAN 1
#		endif
		// @STUPID(final): Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here"
struct IUnknown;
#		include <windows.h> // Win32 api
#		if _WIN32_WINNT < 0x0600
#			error "Windows Vista or higher required!"
#		endif
#	endif // FPL_PLATFORM_WINDOWS

#	if defined(FPL_SUBPLATFORM_POSIX)
#		include <pthread.h> // pthread_t, pthread_mutex_, pthread_cond_, pthread_barrier_
#		include <sched.h> // sched_param, sched_get_priority_max, SCHED_FIFO
#		include <semaphore.h> // sem_t
#		include <dirent.h> // DIR, dirent
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
#		include <X11/X.h> // Window
#		include <X11/Xlib.h> // Display
#		include <X11/Xutil.h> // XVisualInfo
#		include <X11/Xatom.h> // XA_CARDINAL
#	endif // FPL_SUBPLATFORM_X11

#endif // !FPL_NO_PLATFORM_INCLUDES

//
// Platform handles
//
#if !defined(FPL__HAS_PLATFORM_INCLUDES) || defined(FPL_OPAQUE_HANDLES)

#	if defined(FPL_PLATFORM_WINDOWS)

//! A win32 GUID (opaque, min 16 bytes)
typedef uint64_t fpl__Win32Guid[4];
//! A win32 handle (opaque, min 4/8 bytes)
typedef void *fpl__Win32Handle;
//! A win32 instance handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32InstanceHandle;
//! A win32 library handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32LibraryHandle;
//! A win32 filehandle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32FileHandle;
//! A win32 thread handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32ThreadHandle;
//! A win32 mutex handle (opaque, min 80 bytes)
typedef uint64_t fpl__Win32MutexHandle[16];
//! A win32 signal handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32SignalHandle;
//! A win32 condition variable (opaque, min 4/8 bytes)
typedef void *fpl__Win32ConditionVariable;
//! A win32 semaphore handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32SemaphoreHandle;
//! A win32 window handle (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32WindowHandle;
//! A win32 device context (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32DeviceContext;
//! A win32 rendering context (opaque, min 4/8 bytes)
typedef fpl__Win32Handle fpl__Win32RenderingContext;
//! A win32 structure for storing a large integer for QPC (opaque, 8 bytes)
typedef union fpl__Win32LargeInteger {
	//! 64-bit part
	int64_t QuadPart;
	struct {
		//! 32-bit low part
		int32_t LowPart;
		//! 32-bit high part
		int32_t HighPart;
	};
} fpl__Win32LargeInteger;

#	endif // FPL_PLATFORM_WINDOWS

#	if defined(FPL_SUBPLATFORM_POSIX)

//! A POSIX library handle (opaque, min 4/8 bytes)
typedef void *fpl__POSIXLibraryHandle;
//! A POSIX filehandle (opaque, min 4 bytes)
typedef int fpl__POSIXFileHandle;
//! A POSIX directory handle (opaque, min 4/8 bytes)
typedef void *fpl__POSIXDirHandle;
//! A POSIX thread handle (opaque, min 8 bytes)
typedef uint64_t fpl__POSIXThreadHandle;
//! A POSIX mutex handle (opaque, min 40 bytes)
typedef uint64_t fpl__POSIXMutexHandle[16];
//! A POSIX semaphore handle (opaque, min 32 bytes)
typedef uint64_t fpl__POSIXSemaphoreHandle[8];
//! A POSIX condition variable (opaque, min: 48 bytes)
typedef uint64_t fpl__POSIXConditionVariable[16];

#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)

//! A X11 Display (opaque, 4/8 bytes)
typedef void *fpl__X11Display;
//! A X11 window (opaque, 4 bytes)
typedef int fpl__X11Window;
//! A X11 Visual (opaque, 4/8 bytes)
typedef void *fpl__X11Visual;
//! A X11 GC (opaque, 4/8 bytes)
typedef void *fpl__X11GC;
//! A X11 Image (opaque, 4/8 bytes)
typedef void *fpl__X11Image;
//! A GLX Context (opaque, 4/8 bytes)
typedef void *fpl__GLXContext;

#	endif // FPL_SUBPLATFORM_X11

#	if defined(FPL_PLATFORM_LINUX)

//! A Linux signal handle (opaque, min 4 bytes)
typedef int fpl__LinuxSignalHandle;

#	endif // FPL_PLATFORM_LINUX

#else

#	if defined(FPL_PLATFORM_WINDOWS)

//! A win32 GUID
typedef GUID fpl__Win32Guid;
//! A win32 handle
typedef HANDLE fpl__Win32Handle;
//! A win32 instance handle
typedef HINSTANCE fpl__Win32InstanceHandle;
//! A win32 library handle
typedef HMODULE fpl__Win32LibraryHandle;
//! A win32 thread handle
typedef HANDLE fpl__Win32ThreadHandle;
//! A win32 filehandle
typedef HANDLE fpl__Win32FileHandle;
//! A win32 mutex handle
typedef CRITICAL_SECTION fpl__Win32MutexHandle;
//! A win32 signal handle
typedef HANDLE fpl__Win32SignalHandle;
//! A win32 condition variable
typedef CONDITION_VARIABLE fpl__Win32ConditionVariable;
//! A win32 semaphore handle
typedef HANDLE fpl__Win32SemaphoreHandle;
//! A win32 window handle
typedef HWND fpl__Win32WindowHandle;
//! A win32 device context
typedef HDC fpl__Win32DeviceContext;
//! A win32 rendering context
typedef HGLRC fpl__Win32RenderingContext;
//! A win32 structure for storing a large integer for QPC
typedef LARGE_INTEGER fpl__Win32LargeInteger;

#	endif // FPL_PLATFORM_WINDOWS

#	if defined(FPL_SUBPLATFORM_POSIX)

//! A POSIX library handle
typedef void *fpl__POSIXLibraryHandle;
//! A POSIX filehandle
typedef int fpl__POSIXFileHandle;
//! A POSIX directory handle
typedef DIR *fpl__POSIXDirHandle;
//! A POSIX thread handle
typedef pthread_t fpl__POSIXThreadHandle;
//! A POSIX mutex handle
typedef pthread_mutex_t fpl__POSIXMutexHandle;
//! A POSIX semaphore handle
typedef sem_t fpl__POSIXSemaphoreHandle;
//! A POSIX condition variable
typedef pthread_cond_t fpl__POSIXConditionVariable;

#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)

//! A X11 Display
typedef Display *fpl__X11Display;
//! A X11 window
typedef Window fpl__X11Window;
//! A X11 Visual
typedef Visual *fpl__X11Visual;
//! A X11 GC
typedef GC fpl__X11GC;
//! A X11 Image
typedef XImage *fpl__X11Image;
//! A GLX Context (opaque, 4/8 bytes)
typedef void *fpl__GLXContext;

#	endif // FPL_SUBPLATFORM_X11


#	if defined(FPL_PLATFORM_LINUX)

//! A Linux signal handle
typedef int fpl__LinuxSignalHandle;

#	endif // FPL_PLATFORM_LINUX


#endif

//
// Constants
//

/**
* @defgroup Constants Constants
* @brief This category contains constants
* @{
*/
#if defined(FPL_PLATFORM_WINDOWS)
#	if defined(MAX_PATH)
#		define FPL__M_MAX_FILENAME_LENGTH (MAX_PATH)
#		define FPL__M_MAX_PATH_LENGTH (MAX_PATH * 2)
#	else
#		define FPL__M_MAX_FILENAME_LENGTH (260)
#		define FPL__M_MAX_PATH_LENGTH (260 * 2)
#	endif
#	define FPL__M_PATH_SEPARATOR '\\'
#	define FPL__M_FILE_EXT_SEPARATOR '.'
#else
#	define FPL__M_MAX_FILENAME_LENGTH (512)
#	define FPL__M_MAX_PATH_LENGTH (2048)
#	define FPL__M_PATH_SEPARATOR '/'
#	define FPL__M_FILE_EXT_SEPARATOR '.'
#endif

//! Maximum length of a filename
#define FPL_MAX_FILENAME_LENGTH FPL__M_MAX_FILENAME_LENGTH
//! Maximum length of a path
#define FPL_MAX_PATH_LENGTH FPL__M_MAX_PATH_LENGTH
//! Path separator character
#define FPL_PATH_SEPARATOR FPL__M_PATH_SEPARATOR
//! File extension character
#define FPL_FILE_EXT_SEPARATOR FPL__M_FILE_EXT_SEPARATOR
//! Maximum length of a name (in characters)
#define FPL_MAX_NAME_LENGTH (256)
//! Maximum length of an internal buffer (in bytes)
#define FPL_MAX_BUFFER_LENGTH (2048)

/** @} */

// ****************************************************************************
//
// > API
//
// ****************************************************************************

// ----------------------------------------------------------------------------
/**
* @defgroup Atomics Atomic operations
* @brief This category contains functions for handling atomic operations, such as Add, Compare And/Or Exchange, Fences, Loads/Stores, etc.
* @see @ref page_category_threading_atomics
* @{
*/
// ----------------------------------------------------------------------------

//
// Barrier/Fence
//

/**
* @brief Inserts a memory read fence/barrier.
* @note This will complete previous reads before future reads and prevents the compiler from reordering memory reads across this fence.
* @see @ref section_category_threading_atomics_barriers
*/
fpl_platform_api void fplAtomicReadFence();
/**
* @brief Inserts a memory write fence/barrier.
* @note This will complete previous writes before future writes and prevents the compiler from reordering memory writes across this fence.
* @see @ref section_category_threading_atomics_barriers
*/
fpl_platform_api void fplAtomicWriteFence();
/**
* @brief Inserts a memory read and write fence/barrier.
* @note This will complete previous reads and writes before future reads and writes and prevents the compiler from reordering memory access across this fence.
* @see @ref section_category_threading_atomics_barriers
*/
fpl_platform_api void fplAtomicReadWriteFence();

//
// Exchange
//

/**
* @brief Replaces a 32-bit unsigned integer with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
/**
* @brief Replaces a 64-bit unsigned integer with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
/**
* @brief Replaces a 32-bit signed integer with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value);
/**
* @brief Replaces a 64-bit signed integer with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value);
/**
* @brief Replaces a pointer with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value);
/**
* @brief Replaces a size with the given value atomically.
* @param target The target value to write into
* @param value The source value used for exchange
* @return Returns the initial value before the replacement.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_exchange
*/
fpl_common_api size_t fplAtomicExchangeSize(volatile size_t *target, const size_t value);

//
// Fetch and Add
//

/**
* @brief Adds a 32-bit unsigned integer to the value by the given addend atomically.
* @param value The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api uint32_t fplAtomicFetchAndAddU32(volatile uint32_t *value, const uint32_t addend);
/**
* @brief Adds a 64-bit unsigned integer to the value by the given addend atomically.
* @param value The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api uint64_t fplAtomicFetchAndAddU64(volatile uint64_t *value, const uint64_t addend);
/**
* @brief Adds a 32-bit signed integer to the value by the given addend atomically.
* @param value The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api int32_t fplAtomicFetchAndAddS32(volatile int32_t *value, const int32_t addend);
/**
* @brief Adds a 64-bit signed integer to the value by the given addend atomically.
* @param value The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api int64_t fplAtomicFetchAndAddS64(volatile int64_t *value, const int64_t addend);
/**
* @brief Adds a size to the value by the given addend atomically.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_common_api size_t fplAtomicFetchAndAddSize(volatile size_t *dest, const size_t addend);
/**
* @brief Adds a addend to the pointer atomically and returns the initial value before the add.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the initial value before the add.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_common_api void *fplAtomicFetchAndAddPtr(volatile void **dest, const intptr_t addend);

//
// Add and Fetch
//

/**
* @brief Adds the addend to destination 32-bit unsigned integer atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api uint32_t fplAtomicAddAndFetchU32(volatile uint32_t *dest, const uint32_t addend);
/**
* @brief Adds the addend to destination 64-bit unsigned integer atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api uint64_t fplAtomicAddAndFetchU64(volatile uint64_t *dest, const uint64_t addend);
/**
* @brief Adds the addend to destination 32-bit signed integer atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api int32_t fplAtomicAddAndFetchS32(volatile int32_t *dest, const int32_t addend);
/**
* @brief Adds the addend to destination 64-bit signed integer atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_platform_api int64_t fplAtomicAddAndFetchS64(volatile int64_t *dest, const int64_t addend);
/**
* @brief Adds the addend to destination size atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_common_api size_t fplAtomicAddAndFetchSize(volatile size_t *dest, const size_t addend);
/**
* @brief Adds the addend to destination pointer atomically and returns the result after the addition.
* @param dest The target value to add to.
* @param addend The value used for adding.
* @return Returns the value after the addition.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_add
*/
fpl_common_api void *fplAtomicAddAndFetchPtr(volatile void **dest, const intptr_t addend);

//
// Increment
//

/**
* @brief Increments the given 32-bit unsigned integer by one atomically.
* @param dest The target value to increment to.
* @return Returns the value after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_platform_api uint32_t fplAtomicIncrementU32(volatile uint32_t *dest);
/**
* @brief Increments the given 64-bit unsigned integer by one atomically.
* @param dest The target value to increment to.
* @return Returns the value after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_platform_api uint64_t fplAtomicIncrementU64(volatile uint64_t *dest);
/**
* @brief Increments the given 32-bit signed integer by one atomically.
* @param dest The target value to increment to.
* @return Returns the value after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_platform_api int32_t fplAtomicIncrementS32(volatile int32_t *dest);
/**
* @brief Increments the given 64-bit signed integer by one atomically.
* @param dest The target value to increment to.
* @return Returns the value after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_platform_api int64_t fplAtomicIncrementS64(volatile int64_t *dest);
/**
* @brief Increments the given size by one atomically.
* @param dest The target value to increment to.
* @return Returns the value after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_common_api size_t fplAtomicIncrementSize(volatile size_t *dest);
/**
* @brief Increments/Advances the given pointer by one atomically.
* @param dest The target value to increment to.
* @return Returns the next address, after the increment.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_inc
*/
fpl_common_api void *fplAtomicIncrementPtr(volatile void **dest);

//
// CAS
//

/**
* @brief Compares a 32-bit unsigned integer with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapU32() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api uint32_t fplAtomicCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
* @brief Compares a 64-bit unsigned integer with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapU64() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api uint64_t fplAtomicCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
* @brief Compares a 32-bit signed integer with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapS32() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api int32_t fplAtomicCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
* @brief Compares a 64-bit signed integer with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapS64() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api int64_t fplAtomicCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
* @brief Compares a size with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapPtr() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_common_api size_t fplAtomicCompareAndSwapSize(volatile size_t *dest, const size_t comparand, const size_t exchange);
/**
* @brief Compares a pointer with a comparand and swaps it when comparand matches the destination.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns the value of the destination before the swap, regardless of the result.
* @note Ensures that memory operations are completed in order.
* @note Use @ref fplAtomicIsCompareAndSwapPtr() when you want to check if the exchange has happened or not.
* @see @ref category_threading_atomics_cas
*/
fpl_common_api void *fplAtomicCompareAndSwapPtr(volatile void **dest, const void *comparand, const void *exchange);

/**
* @brief Compares a 32-bit unsigned integer with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api bool fplAtomicIsCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
/**
* @brief Compares a 64-bit unsigned integer with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api bool fplAtomicIsCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
/**
* @brief Compares a 32-bit signed integer with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api bool fplAtomicIsCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
/**
* @brief Compares a 64-bit signed integer with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_platform_api bool fplAtomicIsCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
/**
* @brief Compares a size with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_common_api bool fplAtomicIsCompareAndSwapSize(volatile size_t *dest, const size_t comparand, const size_t exchange);
/**
* @brief Compares a pointer with a comparand and swaps it when comparand matches the destination and returns a bool indicating the result.
* @param dest The target value to write into
* @param comparand The value to compare with
* @param exchange The value to exchange with
* @return Returns true when the exchange happened, false otherwise.
* @note Ensures that memory operations are completed in order.
* @see @ref category_threading_atomics_cas
*/
fpl_common_api bool fplAtomicIsCompareAndSwapPtr(volatile void **dest, const void *comparand, const void *exchange);

//
// Load
//

/**
* @brief Loads the 32-bit unsigned value atomically and returns the value.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @see @ref category_threading_atomics_load
*/
fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source);
/**
* @brief Loads the 64-bit unsigned value atomically and returns the value.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @see @ref category_threading_atomics_load
*/
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source);
/**
* @brief Loads the 32-bit signed value atomically and returns the value.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @see @ref category_threading_atomics_load
*/
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source);
/**
* @brief Loads the 64-bit signed value atomically and returns the value.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @see @ref category_threading_atomics_load
*/
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source);
/**
* @brief Loads the size value atomically and returns the value.
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @see @ref category_threading_atomics_load
*/
fpl_common_api size_t fplAtomicLoadSize(volatile size_t *source);
/**
* @brief Loads the pointer value atomically and returns the value.
* @note Ensures that memory operations are completed before the reading.
* @note This may use a CAS instruction when there are no suitable compiler intrinsics found.
* @param source The source value to read from
* @return Returns the atomically loaded source value
* @see @ref category_threading_atomics_load
*/
fpl_common_api void *fplAtomicLoadPtr(volatile void **source);

//
// Store
//

/**
* @brief Overwrites the 32-bit unsigned value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
/**
* @brief Overwrites the 64-bit unsigned value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
/**
* @brief Overwrites the 32-bit signed value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value);
/**
* @brief Overwrites the 64-bit signed value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value);
/**
* @brief Overwrites the size value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_common_api void fplAtomicStoreSize(volatile size_t *dest, const size_t value);
/**
* @brief Overwrites the pointer value atomically.
* @param dest The destination to write to
* @param value The value to exchange with
* @note Ensures that memory operations are completed before the write.
* @see @ref category_threading_atomics_store
*/
fpl_common_api void fplAtomicStorePtr(volatile void **dest, const void *value);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Memory Memory functions
* @brief This category contains functions for allocating/manipulating memory
* @{
*/
// ----------------------------------------------------------------------------

//! Defines a memory block
typedef struct fplMemoryBlock {
	//! The base pointer
	void *base;
	//! The size of the allocated memory
	size_t size;
} fplMemoryBlock;

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

/**
* @brief Clears the given memory by the given size to zero.
* @param mem The pointer to the memory
* @param size The number of bytes to be cleared to zero
* @see @ref subsection_category_memory_handling_ops_clear
*/
fpl_common_api void fplMemoryClear(void *mem, const size_t size);
/**
* @brief Sets the given memory by the given size to the given value.
* @param mem The pointer to the memory
* @param value The value to be set
* @param size The number of bytes to be set
* @see @ref subsection_category_memory_handling_ops_set
*/
fpl_common_api void fplMemorySet(void *mem, const uint8_t value, const size_t size);
/**
* @brief Copies the given source memory with its length to the target memory.
* @param sourceMem The pointer to the source memory to copy from
* @param sourceSize The size in bytes to be copied
* @param targetMem The pointer to the target memory to copy into
* @see @ref subsection_category_memory_handling_ops_copy
*/
fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem);
/**
* @brief Allocates memory from the operating system by the given size.
* @param size The size to be allocated in bytes.
* @return Returns a pointer to the newly allocated memory.
* @warning Alignment is not ensured here, the OS decides how to handle this. If you want to force a specific alignment use @ref fplMemoryAlignedAllocate() instead.
* @note The memory is guaranteed to be initialized to zero.
* @note This function can be called without the platform to be initialized.
* @see @ref subsection_category_memory_handling_normal_allocate
*/
fpl_platform_api void *fplMemoryAllocate(const size_t size);
/**
* @brief Releases the memory allocated from the operating system.
* @param ptr The pointer to the allocated memory
* @warning This should never be called with an aligned memory pointer! For freeing aligned memory, use @ref fplMemoryAlignedFree() instead.
* @note This function can be called without the platform to be initialized.
* @see @ref section_category_memory_normal_free
*/
fpl_platform_api void fplMemoryFree(void *ptr);
/**
* @brief Allocates aligned memory from the operating system by the given alignment.
* @param size The size amount in bytes
* @param alignment The alignment in bytes (Must be a power-of-two!)
* @return Returns the pointer to the new allocated aligned memory.
* @note The memory is guaranteed to be initialized to zero.
* @note This function can be called without the platform to be initialized.
* @see @ref subsection_category_memory_handling_aligned_allocate
*/
fpl_common_api void *fplMemoryAlignedAllocate(const size_t size, const size_t alignment);
/**
* @brief Releases the aligned memory allocated from the operating system.
* @param ptr The pointer to the aligned allocated memory
* @warning This should never be called with a not-aligned memory pointer! For freeing not-aligned memory, use @ref fplMemoryFree() instead.
* @note This function can be called without the platform to be initialized.
* @see @ref subsection_category_memory_handling_aligned_free
*/
fpl_common_api void fplMemoryAlignedFree(void *ptr);
/**
* @brief Retrieves the current system memory usage.
* @param outInfos The target @ref fplMemoryInfos structure
* @return Returns true when the memory infos was retrieved, false otherwise.
* @see @ref section_category_hardware_memstate
*/
fpl_platform_api bool fplMemoryGetInfos(fplMemoryInfos *outInfos);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup PlatformOS Operating system Infos
* @brief This category contains functions for retrieving several operating system information such as version, name, etc.
* @{
*/
// ----------------------------------------------------------------------------

//! A type definition for mapping a part of a version number
typedef char fplVersionNumberPart[4 + 1];

//! A structure that contains version informations
typedef struct fplVersionInfo {
	//! Full name
	char fullName[FPL_MAX_NAME_LENGTH];
	union {
		//! Version number parts
		fplVersionNumberPart values[4];
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
	};
} fplVersionInfo;

//! A structure that contains the version information for the operating system
typedef struct fplOSVersionInfos {
	//! Name of the operating system
	char osName[FPL_MAX_NAME_LENGTH];
	//! Name of the distribution (May be empty)
	char distributionName[FPL_MAX_NAME_LENGTH];
	//! Version of the operating system
	fplVersionInfo osVersion;
	//! Version of the distribution (May be empty)
	fplVersionInfo distributionVersion;
} fplOSVersionInfos;

/**
* @brief Gets version informations from the operating system
* @param outInfos The target @ref fplOSVersionInfos structure
* @return Returns true when the infos could be retrieved, false otherwise.
* @note This may be called without initializing the platform
* @see @ref section_category_platform_os_version
*/
fpl_platform_api bool fplOSGetVersionInfos(fplOSVersionInfos *outInfos);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup OSSession Session Infos
* @brief This category contains functions for retrieving current session informations, such as username, etc.
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Gets the username of the current logged-in user from the session
* @param nameBuffer The target buffer
* @param maxNameBufferLen The max length of the target buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref section_category_platform_os_username
*/
fpl_platform_api size_t fplSessionGetUsername(char *nameBuffer, const size_t maxNameBufferLen);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Hardware Hardware Infos
* @brief This category contains functions for retrieving hardware information such as memory usage, CPU info, etc.
* @{
*
*/
// ----------------------------------------------------------------------------

//! An enumeration of architecture types
typedef enum fplCPUArchType {
	//! Unknown architecture
	fplCPUArchType_Unknown = 0,
	//! X86 architecture
	fplCPUArchType_x86,
	//! X86 with 64-bit architecture
	fplCPUArchType_x86_64,
	//! X64 only architecture
	fplCPUArchType_x64,
	//! ARM32 architecture
	fplCPUArchType_Arm32,
	//! ARM64 architecture
	fplCPUArchType_Arm64,

	//! First @ref fplCPUArchType
	fplCPUArchType_First = fplCPUArchType_Unknown,
	//! Last @ref fplCPUArchType
	fplCPUArchType_Last = fplCPUArchType_Arm64,
} fplCPUArchType;

//! A structure that containing the processor capabilities, like MMX,SSE,AVX etc.
typedef struct fplCPUCapabilities {
	//! Is MMX supported
	fpl_b32 hasMMX;
	//! Is SSE supported
	fpl_b32 hasSSE;
	//! Is SSE-2 supported
	fpl_b32 hasSSE2;
	//! Is SSE-3 supported
	fpl_b32 hasSSE3;
	//! Is SSSE-3 supported
	fpl_b32 hasSSSE3;
	//! Is SSE-4.1 supported
	fpl_b32 hasSSE4_1;
	//! Is SSE-4.2 supported
	fpl_b32 hasSSE4_2;
	//! Is AVX supported
	fpl_b32 hasAVX;
	//! Is AVX-2 supported
	fpl_b32 hasAVX2;
	//! Is AVX-512 supported
	fpl_b32 hasAVX512;
	//! Is FMA-3 supported
	fpl_b32 hasFMA3;
} fplCPUCapabilities;

//! A structure containing the 4-registers (EAX, EBX, ECX, EDX) for a CPU-Leaf.
typedef union fplCPUIDLeaf {
	struct {
		//! The 32-bit EAX Register
		uint32_t eax;
		//! The 32-bit EBX Register
		uint32_t ebx;
		//! The 32-bit ECX Register
		uint32_t ecx;
		//! The 32-bit EDX Register
		uint32_t edx;
	};
	//! The raw 32-bit register array
	uint32_t raw[4];
} fplCPUIDLeaf;

/**
* @brief Queries the x86 CPUID leaf register (EAX, EBX, ECX, EDX) for the given function id
* @param outLeaf The target fplCPUIDLeaf reference
* @param functionId The CPUID function id
* @warning This function works on X86 architectures only
*/
fpl_common_api void fplCPUID(fplCPUIDLeaf *outLeaf, const uint32_t functionId);
/**
* @brief Gets the x86 extended control register for index zero.
* @return Returns the extended control register on x86 or zero for non-x86 architectures.
* @warning This function works on X86 architectures only
*/
fpl_common_api uint64_t fplCPUXCR0();
/**
* @brief Reads the current time stamp counter (RDTSC)
* @return Returns the number of cycles since the system start or zero for non-x86 architectures.
* @warning This function works on X86 architectures only
*/
fpl_common_api uint64_t fplCPURDTSC();
/**
* @brief Gets the string representation of the given architecture type
* @param type The @ref fplCPUArchType enumeration value
* @return Returns a string for the given architecture type
* @see @ref section_category_hardware_cpuarch
*/
fpl_common_api const char *fplCPUGetArchName(const fplCPUArchType type);
/**
* @brief Retrieves the total number of processor cores
* @return Returns the total number of processor cores.
* @see @ref section_category_hardware_corecount
*/
fpl_platform_api size_t fplCPUGetCoreCount();
/**
* @brief Retrieves the name of the processor
* @param destBuffer The destination buffer
* @param maxDestBufferLen The max length of the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref section_category_hardware_cpuname
*/
fpl_common_api size_t fplCPUGetName(char *destBuffer, const size_t maxDestBufferLen);
/**
* @brief Gets the capabilities of the processor
* @param outCaps Pointer to the output @ref fplCPUCapabilities
* @return Returns true when the capabilities could be retrieved, false otherwise.
* @see @ref section_category_hardware_cpucaps
*/
fpl_common_api bool fplCPUGetCapabilities(fplCPUCapabilities *outCaps);
/**
* @brief Gets the processor architecture type
* @return Returns the processor architecture type
* @see @ref section_category_hardware_cpuarch
*/
fpl_platform_api fplCPUArchType fplCPUGetArchitecture();

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Settings Settings & Configurations
* @brief This category contains global settings structures/enumerations and functions to initialize/set them
* @{
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
	//! Use a video backbuffer (This flag ensures that @ref fplInitFlags_Window is included always)
	fplInitFlags_Video = 1 << 2,
	//! Use asynchronous audio playback
	fplInitFlags_Audio = 1 << 3,
	//! Support for game controllers
	fplInitFlags_GameController = 1 << 4,
	//! All init flags
	fplInitFlags_All = fplInitFlags_Console | fplInitFlags_Window | fplInitFlags_Video | fplInitFlags_Audio | fplInitFlags_GameController,
} fplInitFlags;
//! InitFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplInitFlags);

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

	//! First @ref fplPlatformType
	fplPlatformType_First = fplPlatformType_Unknown,
	//! Last @ref fplPlatformType
	fplPlatformType_Last = fplPlatformType_Unix,
} fplPlatformType;

//! An enumeration of platform result types
typedef enum fplPlatformResultType {
	//! Window creation failed
	fplPlatformResultType_FailedWindow = -6,
	//! Audio initialization failed
	fplPlatformResultType_FailedAudio = -5,
	//! Video initialization failed
	fplPlatformResultType_FailedVideo = -4,
	//! Platform initialization failed
	fplPlatformResultType_FailedPlatform = -3,
	//! Failed allocating required memory
	fplPlatformResultType_FailedAllocatingMemory = -2,
	//! Platform is already initialized
	fplPlatformResultType_AlreadyInitialized = -1,
	//! Platform is not initialized
	fplPlatformResultType_NotInitialized = 0,
	//! Everything is fine
	fplPlatformResultType_Success = 1,

	//! First @ref fplPlatformResultType
	fplPlatformResultType_First = fplPlatformResultType_FailedWindow,
	//! Last @ref fplPlatformResultType
	fplPlatformResultType_Last = fplPlatformResultType_Success,
} fplPlatformResultType;

/**
* @brief Gets the string representation of a platform result type.
* @param type The platform result type as @ref fplPlatformResultType
* @return Returns the string representation of a platform result type.
* @see @ref section_category_initialization_result
*/
fpl_common_api const char *fplPlatformGetResultName(const fplPlatformResultType type);

//! An enumeration of video backend types
typedef enum fplVideoBackendType {
	//! No video backend
	fplVideoBackendType_None = 0,
	//! Software
	fplVideoBackendType_Software,
	//! OpenGL
	fplVideoBackendType_OpenGL,
	//! Vulkan
	fplVideoBackendType_Vulkan,

	//! First @ref fplVideoBackendType
	fplVideoBackendType_First = fplVideoBackendType_None,
	//! Last @ref fplVideoBackendType
	fplVideoBackendType_Last = fplVideoBackendType_Vulkan,
} fplVideoBackendType;

#if defined(FPL__ENABLE_VIDEO_OPENGL)
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
typedef struct fplOpenGLSettings {
	//! Custom OpenGL driver library file name/path (null = Default OpenGL library)
	const char *libraryFile;
	//! Compability flags
	fplOpenGLCompabilityFlags compabilityFlags;
	//! Desired major version
	uint32_t majorVersion;
	//! Desired minor version
	uint32_t minorVersion;
	//! Multisampling count
	uint8_t multiSamplingCount;
} fplOpenGLSettings;
#endif // FPL__ENABLE_VIDEO_OPENGL

#if defined(FPL__ENABLE_VIDEO_VULKAN)

//! The debug callback called when the validation layer writes something
typedef void (fplVulkanValidationLayerCallback)(void *userData, const char *message, const uint32_t messageSeverity, const uint32_t messageType, const void *debugUtilsMessengerCallbackData);

//! The validation layer modes for Vulkan
typedef enum fplVulkanValidationLayerMode {
	//! Do not use the validation
	fplVulkanValidationLayerMode_Disabled = 0,
	//! Enable validations when its possible
	fplVulkanValidationLayerMode_Optional,
	//! Enable validations and stop when its not supported
	fplVulkanValidationLayerMode_Required,
} fplVulkanValidationLayerMode;

//! The validation layer logging severity for Vulkan
typedef enum fplVulkanValidationSeverity {
	//! Log nothing
	fplVulkanValidationSeverity_Off = 0,
	//! Log error only
	fplVulkanValidationSeverity_Error = 1,
	//! Log warning and error
	fplVulkanValidationSeverity_Warning = 2,
	//! Log warning, error, infos
	fplVulkanValidationSeverity_Info = 3,
	//! Log warning, error, info, verbose
	fplVulkanValidationSeverity_Verbose = 4,
	//! Log out everything
	fplVulkanValidationSeverity_All = INT32_MAX,
} fplVulkanValidationSeverity;

//! A structure that contains Vulkan video settings
typedef struct fplVulkanSettings {
	//! The application version (Only required if @ref fplVulkanSettings.instanceHandle is @ref fpl_null)
	fplVersionInfo appVersion;
	//! The engine version (Only required if @ref fplVulkanSettings.instanceHandle is @ref fpl_null)
	fplVersionInfo engineVersion;
	//! The preferred Vulkan api version (Only required if @ref fplVulkanSettings.instanceHandle is @ref fpl_null)
	fplVersionInfo apiVersion;
	//! Custom Vulkan driver library file name/path (null = Default Vulkan library)
	const char *libraryFile;
	//! The application name (Only required if @ref fplVulkanSettings.instanceHandle is @ref fpl_null)
	const char *appName;
	//! The engine name (Only required if @ref fplVulkanSettings.instanceHandle is @ref fpl_null)
	const char *engineName;
	//! The vulkan instance (VkInstance), when null it will be automatically created
	void *instanceHandle;
	//! The vulkan allocator (VkAllocationCallbacks)
	const void *allocator;
	//! The validation layer callback @ref fplVulkanValidationLayerCallback
	fplVulkanValidationLayerCallback *validationLayerCallback;
	//! User data passed to any callbacks
	void *userData;
	//! The @ref fplVulkanValidationLayerMode
	fplVulkanValidationLayerMode validationLayerMode;
	//! The @ref fplVulkanValidationSeverity
	fplVulkanValidationSeverity validationSeverity;
} fplVulkanSettings;
#endif // FPL__ENABLE_VIDEO_VULKAN

//! A union that contains graphics api settings
typedef struct fplGraphicsApiSettings {
#if defined(FPL__ENABLE_VIDEO_OPENGL)
	//! OpenGL settings
	fplOpenGLSettings opengl;
#endif
#if defined(FPL__ENABLE_VIDEO_VULKAN)
	//! Vulkan settings
	fplVulkanSettings vulkan;
#endif
	//! Field for preventing union to be empty
	int dummy;
} fplGraphicsApiSettings;

//! A structure that contains video settings such as backend, v-sync, API-settings, etc.
typedef struct fplVideoSettings {
	//! Graphics API settings
	fplGraphicsApiSettings graphics;
	//! video backend type
	fplVideoBackendType backend;
	//! Is vertical synchronization enabled. Usable only for hardware rendering!
	fpl_b32 isVSync;
	//! Is backbuffer automatically resized. Usable only for software rendering!
	fpl_b32 isAutoSize;
} fplVideoSettings;

/**
* @brief Resets the given video settings to default values
* @param video The target @ref fplVideoSettings structure
* @note This will not change any video settings! To change the actual settings you have to pass the entire @ref fplSettings container as an argument in @ref fplPlatformInit().
* @see @ref category_video_general_notes
*/
fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video);

//! An enumeration of audio backend types
typedef enum fplAudioBackendType {
	//! No audio backend
	fplAudioBackendType_None = 0,
	//! Auto detect
	fplAudioBackendType_Auto,
	//! DirectSound
	fplAudioBackendType_DirectSound,
	//! ALSA
	fplAudioBackendType_Alsa,

	//! First @ref fplAudioBackendType
	fplAudioBackendType_First = fplAudioBackendType_None,
	//! Last @ref fplAudioBackendType
	fplAudioBackendType_Last = fplAudioBackendType_Alsa,
} fplAudioBackendType;

//! An enumeration of audio format types
typedef enum fplAudioFormatType {
	//! No audio format
	fplAudioFormatType_None = 0,
	//! Unsigned 8-bit integer PCM
	fplAudioFormatType_U8,
	//! Signed 16-bit integer PCM
	fplAudioFormatType_S16,
	//! Signed 24-bit integer PCM
	fplAudioFormatType_S24,
	//! Signed 32-bit integer PCM
	fplAudioFormatType_S32,
	//! Signed 64-bit integer PCM
	fplAudioFormatType_S64,
	//! 32-bit IEEE_FLOAT
	fplAudioFormatType_F32,
	//! 64-bit IEEE_FLOAT
	fplAudioFormatType_F64,

	//! First @ref fplAudioFormatType
	fplAudioFormatType_First = fplAudioFormatType_None,
	//! Last @ref fplAudioFormatType
	fplAudioFormatType_Last = fplAudioFormatType_F64,
} fplAudioFormatType;

//! An enumeration of audio default fields
typedef enum fplAudioDefaultFields {
	//! No default fields
	fplAudioDefaultFields_None = 0,
	//! Buffer size is default
	fplAudioDefaultFields_BufferSize = 1 << 0,
	//! Samples per seconds is default
	fplAudioDefaultFields_SampleRate = 1 << 1,
	//! Number of channels is default
	fplAudioDefaultFields_Channels = 1 << 2,
	//! Number of periods is default
	fplAudioDefaultFields_Periods = 1 << 3,
	//! Audio format is default
	fplAudioDefaultFields_Type = 1 << 4,
} fplAudioDefaultFields;
//! fplAudioDefaultFields operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplAudioDefaultFields);

//! An enumeration of audio latency modes
typedef enum fplAudioLatencyMode {
	//! Conservative latency
	fplAudioLatencyMode_Conservative = 0,
	//! Low latency
	fplAudioLatencyMode_Low,
} fplAudioLatencyMode;

//! A structure containing audio device format runtime properties, such as type, samplerate, channels, etc.
typedef struct fplAudioDeviceFormat {
	//! Buffer size in frames
	uint32_t bufferSizeInFrames;
	//! Samples per seconds
	uint32_t sampleRate;
	//! Number of channels
	uint32_t channels;
	//! Number of periods
	uint32_t periods;
	//! Format
	fplAudioFormatType type;
	//! Is exclusive mode preferred
	fpl_b32 preferExclusiveMode;
	//! Default fields
	fplAudioDefaultFields defaultFields;
	//! Audio backend
	fplAudioBackendType backend;
} fplAudioDeviceFormat;

//! A structure containing audio target format configurations, such as type, sample rate, channels, etc.
typedef struct fplAudioTargetFormat {
	//! Samples per seconds (uses default of 44100 when zero)
	uint32_t sampleRate;
	//! Number of channels (uses default of 2 when zero)
	uint32_t channels;
	//! Buffer size in frames (First choice)
	uint32_t bufferSizeInFrames;
	//! Buffer size in milliseconds (Second choice)
	uint32_t bufferSizeInMilliseconds;
	//! Number of periods (uses default of 3 when zero)
	uint32_t periods;
	//! Audio format (uses default of S16 when zero)
	fplAudioFormatType type;
	//! Latency mode
	fplAudioLatencyMode latencyMode;
	//! Is exclusive mode preferred
	fpl_b32 preferExclusiveMode;
} fplAudioTargetFormat;

//! A union containing a id of the underlying backend
typedef union fplAudioDeviceID {
#if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
	//! DirectShow Device GUID
	fpl__Win32Guid dshow;
#endif
#if defined(FPL__ENABLE_AUDIO_ALSA)
	//! ALSA Device ID
	char alsa[256];
#endif
	//! Field for preventing union to be empty
	int dummy;
} fplAudioDeviceID;

//! A structure containing the name and the id of the audio device
typedef struct fplAudioDeviceInfo {
	//! Device name
	char name[FPL_MAX_NAME_LENGTH];
	//! Device id
	fplAudioDeviceID id;
} fplAudioDeviceInfo;

#if defined(FPL__ENABLE_AUDIO_ALSA)
//! A structure containing settings for the ALSA audio backend
typedef struct fplAlsaAudioSettings {
	//! Disable the usage of MMap in ALSA
	fpl_b32 noMMap;
} fplAlsaAudioSettings;
#endif

//! A union containing backend specific audio settings
typedef union fplSpecificAudioSettings {
#if defined(FPL__ENABLE_AUDIO_ALSA)
	//! Alsa specific settings
	fplAlsaAudioSettings alsa;
#endif
	//! Field for preventing union to be empty
	int dummy;
} fplSpecificAudioSettings;

/**
* @brief A callback for reading audio samples from the client
* @param deviceFormat The pointer to the @ref fplAudioDeviceFormat structure, the audio cards expects
* @param frameCount The numbers if frames the client should write at max
* @param outputSamples The pointer to the target samples
* @param userData The pointer to the user data specified in @ref fplAudioSettings
* @return Returns the number written frames
* @see @ref subsection_category_audio_general_default_init_clientcallback
*/
typedef uint32_t(fpl_audio_client_read_callback)(const fplAudioDeviceFormat *deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

//! A structure containing audio settings, such as format, device info, callbacks, backend, etc.
typedef struct fplAudioSettings {
	//! The target format
	fplAudioTargetFormat targetFormat;
	//! The target device
	fplAudioDeviceInfo targetDevice;
	//! Specific settings
	fplSpecificAudioSettings specific;
	//! The callback for retrieving audio data from the client
	fpl_audio_client_read_callback *clientReadCallback;
	//! User data pointer for client read callback
	void *userData;
	//! The targeted backend
	fplAudioBackendType backend;
	//! Start playing of audio samples after platform initialization automatically
	fpl_b32 startAuto;
	//! Stop playing of audio samples before platform release automatically
	fpl_b32 stopAuto;
} fplAudioSettings;

/**
* @brief Resets the given audio settings to default settings (S16 PCM, 48 kHz, 2 Channels)
* @param audio The target @ref fplAudioSettings structure
* @note This will not change any audio settings! To change the actual settings you have to pass the entire @ref fplSettings container to an argument in @ref fplPlatformInit().
* @see @ref section_category_audio_general_notes
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

/**
* @brief A callback executed for each raw window event
* @param platformType The current @ref fplPlatformType
* @param windowState The opaque window state, mapping to fpl internal window state
* @param rawEventData The raw event data structure for the current OS (XEvent for POSIX, MSG for Win32, etc.)
* @param userData The pointer to the specific user data specified in @ref fplWindowCallbacks
* @return Needs to return true if the event is handled
*/
typedef bool (fpl_window_event_callback)(const fplPlatformType platformType, void *windowState, void *rawEventData, void *userData);

/**
* @brief A callback executed when the window needs to be exposed/repainted
* @param platformType The current @ref fplPlatformType
* @param windowState The opaque window state, mapping to internal window state
* @param rawEventData The raw event data structure for the current OS (XEvent for POSIX, MSG for Win32, etc.)
* @param userData The pointer to the specific user data specified in @ref fplWindowCallbacks
* @return Needs to return true, if the event is handled
*/
typedef fpl_window_event_callback fpl_window_exposed_callback;

//! A structure containing the window callbacks
typedef struct fplWindowCallbacks {
	//! Expose callback
	fpl_window_exposed_callback *exposedCallback;
	//! User data pointer for the expose callback
	void *exposedUserData;
	//! Expose callback
	fpl_window_event_callback *eventCallback;
	//! User data pointer for the event callback
	void *eventUserData;
} fplWindowCallbacks;

//! A structure containing the size of a window
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

//! @brief Defines a 32-bit color in format BGRA.
typedef union fplColor32 {
	struct {
		//! The 8-bit blue component
		uint8_t b;
		//! The 8-bit green component
		uint8_t g;
		//! The 8-bit red component
		uint8_t r;
		//! The 8-bit alpha component
		uint8_t a;
	};
	//! The 32-bit color value in format 0xBBGGRRAA
	uint32_t value;
	//! The color components array, stored as B, G, R, A
	uint8_t m[4];
} fplColor32;

/**
* @brief Creates a @ref fplColor32 from the specified r, g, b, a components.
* @param r The red component in range of 0-255
* @param g The green component in range of 0-255
* @param b The blue component in range of 0-255
* @param a The alpha component in range of 0-255
* @return The resulting @ref fplColor32
*/
fpl_inline fplColor32 fplCreateColorRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
	fplColor32 result = fplStructInit(fplColor32, b, g, r, a);
	return(result);
}

//! A structure containing window settings, such as size, title etc.
typedef struct fplWindowSettings {
	//! Window title
	char title[FPL_MAX_NAME_LENGTH];
	//! Window icons (0 = Small, 1 = Large)
	fplImageSource icons[2];
	//! Window callbacks
	fplWindowCallbacks callbacks;
	//! The background @ref fplColor32 for the window when using @ref fplVideoBackendType_Software or @ref fplVideoBackendType_None. When all RGBA components are zero, the default system background color is used instead.
	fplColor32 background;
	//! Window size in screen coordinates
	fplWindowSize windowSize;
	//! Fullscreen size in screen coordinates
	fplWindowSize fullscreenSize;
	//! Fullscreen refresh rate in Hz
	uint32_t fullscreenRefreshRate;
	//! Is window resizable
	fpl_b32 isResizable;
	//! Is window decorated
	fpl_b32 isDecorated;
	//! Is floating
	fpl_b32 isFloating;
	//! Is window in fullscreen mode
	fpl_b32 isFullscreen;
	//! Is screen saver prevented (true: prevents the screensaver to kick-in, false: system behavior)
	fpl_b32 isScreenSaverPrevented;
	//! Is monitor power change prevented (true: prevents the monitor for powering off automatically, false: system behavior)
	fpl_b32 isMonitorPowerPrevented;
} fplWindowSettings;

/**
* @brief Resets the given window settings container to default settings
* @param window The target @ref fplWindowSettings structure
* @note This will not change any window settings! To change the actual settings you have to pass the entire @ref fplSettings container to an argument in @ref fplPlatformInit().
* @see @ref section_category_window_style_notes
*/
fpl_common_api void fplSetDefaultWindowSettings(fplWindowSettings *window);

//! A structure containing the title and options for the console
typedef struct fplConsoleSettings {
	//! Console title
	char title[FPL_MAX_NAME_LENGTH];
} fplConsoleSettings;

/**
* @brief Resets the given console settings container to default settings
* @param console The target @ref fplConsoleSettings structure
* @note This will not change any console settings! To change the actual settings you have to pass the entire @ref fplSettings container to an argument in @ref fplPlatformInit().
*/
fpl_common_api void fplSetDefaultConsoleSettings(fplConsoleSettings *console);

//! A structure containing input settings
typedef struct fplInputSettings {
	//! Frequency in ms for detecting new or removed controllers (Default: 200)
	uint32_t controllerDetectionFrequency;
	//! Disable input events entirely (Default: false)
	fpl_b32 disabledEvents;
} fplInputSettings;

/**
* @brief Resets the given input settings container to default values.
* @param input The target @ref fplInputSettings structure
* @note This will not change any input settings! To change the actual settings you have to pass the entire @ref fplSettings container to an argument in @ref fplPlatformInit().
* @see @ref page_category_input_config
*/
fpl_common_api void fplSetDefaultInputSettings(fplInputSettings *input);

//! Custom memory allocation callback
typedef void *(fpl_memory_allocate_callback)(void *userData, const size_t size, const size_t alignment);
//! Custom memory release callback
typedef void (fpl_memory_release_callback)(void *userData, void *ptr);

//! A enumeration of dynamic memory allocation modes
typedef enum fplMemoryAllocationMode {
	//! Use OS memory allocation
	fplMemoryAllocationMode_Automatic = 0,
	//! Use custom memory allocation
	fplMemoryAllocationMode_Custom,
} fplMemoryAllocationMode;

//! A structure for setting up memory allocation usage
typedef struct fplMemoryAllocationSettings {
	//! Memory allocation mode
	fplMemoryAllocationMode mode;
	//! Callback for allocating memory
	fpl_memory_allocate_callback *allocateCallback;
	//! Callback for releasing memory
	fpl_memory_release_callback *releaseCallback;
	//! User data passed through callbacks
	void *userData;
} fplMemoryAllocationSettings;

//! A structure for setting up memory settings for dynamic and temporary allocations
typedef struct fplMemorySettings {
	//! Dynamic memory allocation settings
	fplMemoryAllocationSettings dynamic;
	//! Temporary memory allocation settings
	fplMemoryAllocationSettings temporary;
} fplMemorySettings;

//! A structure containing settings, such as window, video, etc.
typedef struct fplSettings {
	//! Window settings
	fplWindowSettings window;
	//! Video settings
	fplVideoSettings video;
	//! Audio settings
	fplAudioSettings audio;
	//! Input settings
	fplInputSettings input;
	//! Console settings
	fplConsoleSettings console;
	//! Memory settings
	fplMemorySettings memory;
} fplSettings;

/**
* @brief Resets the given settings container to default values for window, video, audio, etc.
* @param settings The target @ref fplSettings structure
* @note This will not change the active settings! To change the actual settings you have to pass this settings container to an argument in @ref fplPlatformInit().
* @see @ref section_category_initialization_with_settings
*/
fpl_common_api void fplSetDefaultSettings(fplSettings *settings);
/**
* @brief Creates a full settings structure containing default values
* @return Returns a defaulted @ref fplSettings structure
* @see @ref section_category_initialization_tips
*/
fpl_common_api fplSettings fplMakeDefaultSettings();
/**
* @brief Gets the current settings
* @return Returns a pointer to the @ref fplSettings structure
* @see @ref section_category_initialization_tips
*/
fpl_common_api const fplSettings *fplGetCurrentSettings();

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Platform Platform functions
* @brief This category contains structures, enumerations and functions for initializing/releasing the platform.
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Gets the type of the platform
* @return Returns the @ref fplPlatformType
* @see @ref section_category_platform_type
*/
fpl_common_api fplPlatformType fplGetPlatformType();
/**
* @brief Gets the string representation of the given platform type
* @param type The platform type @ref fplPlatformType
* @return Returns the string representation for the given platform type @ref fplPlatformType
* @see @ref section_category_platform_type
*/
fpl_common_api const char *fplGetPlatformName(const fplPlatformType type);
/**
* @brief Initializes the platform layer.
* @param initFlags The init flags @ref fplInitFlags used to enable certain features, like video/audio, etc.
* @param initSettings The @ref fplSettings structure to control the platform layer behavior or systems, if null is passed here default values are used automatically.
* @return Returns true when it was successful, false otherwise.
* @note @ref fplPlatformRelease() must be called when you are done! After @ref fplPlatformRelease() has been called you can call this function again if needed.
* @see @ref section_category_initialization_simple
*/
fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings);
/**
* @brief Gets the result type of the platform initialization
* @return Returns the result type as @ref fplPlatformResultType
* @see @ref section_category_errorhandling_getplatformresult
*/
fpl_common_api fplPlatformResultType fplGetPlatformResult();
/**
* @brief Releases the resources allocated by the platform layer.
* @note Can only be called when @ref fplPlatformInit() was successful.
* @see @ref section_category_initialization_release
*/
fpl_common_api void fplPlatformRelease();
/**
* @brief Returns true when the platform is initialized, or false if not.
*/
fpl_common_api bool fplIsPlatformInitialized();

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Logging Logging
* @brief This category contains functions and types for controlling logging output
* @{
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
	//! Trace
	fplLogLevel_Trace = 6,

	//! First @ref fplLogLevel
	fplLogLevel_First = fplLogLevel_All,
	//! Last @ref fplLogLevel
	fplLogLevel_Last = fplLogLevel_Trace,
} fplLogLevel;

#if defined(FPL__ENABLE_LOGGING)
/**
* @brief A callback for printing a log message
* @param funcName The function name
* @param lineNumber The line number
* @param level The log level @ref fplLogLevel
* @param message The log message string
* @see @ref subsection_category_logging_logging_example_custom
*/
typedef void (fpl_log_func_callback)(const char *funcName, const int lineNumber, const fplLogLevel level, const char *message);

//! An enumeration of log writer flags
typedef enum fplLogWriterFlags {
	//! No appender flags
	fplLogWriterFlags_None = 0,
	//! Standard-Console output
	fplLogWriterFlags_StandardConsole = 1 << 0,
	//! Error-Console output
	fplLogWriterFlags_ErrorConsole = 1 << 1,
	//! Debug output
	fplLogWriterFlags_DebugOut = 1 << 2,
	//! Custom output
	fplLogWriterFlags_Custom = 1 << 3,
} fplLogWriterFlags;
//! Log writer flags enumeration operators
FPL_ENUM_AS_FLAGS_OPERATORS(fplLogWriterFlags);

//! A structure containing console logging properties
typedef struct fplLogWriterConsole {
	//! Field for preventing struct to be empty
	int dummy;
} fplLogWriterConsole;

//! A structure containing properties custom logging properties
typedef struct fplLogWriterCustom {
	//! User callback
	fpl_log_func_callback *callback;
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
#if defined(FPL__ENABLE_LOG_MULTIPLE_WRITERS)
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
* @brief Overwrites the current log settings
* @param params The source @ref fplLogSettings structure
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_logging_logging
*/
fpl_common_api void fplSetLogSettings(const fplLogSettings *params);
/**
* @brief Gets the current log settings
* @return Returns a pointer the @ref fplLogSettings structure
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_logging_logging
*/
fpl_common_api const fplLogSettings *fplGetLogSettings();
/**
* @brief Changes the current maximum log level to the given value
* @param maxLevel The new maximum log level @ref fplLogLevel
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_logging_logging
*/
fpl_common_api void fplSetMaxLogLevel(const fplLogLevel maxLevel);
/**
* @brief Gets the current maximum allowed log level
* @return Returns the current maximum log level @ref fplLogLevel
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_logging_logging
*/
fpl_common_api fplLogLevel fplGetMaxLogLevel();
#endif // FPL__ENABLE_LOGGING

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup ErrorHandling Error Handling
* @brief This category contains functions for handling errors
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Gets the last internal error string
* @return Returns the last error string or empty string when there was no error.
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_errorhandling_getlatest
*/
fpl_common_api const char *fplGetLastError();
/**
* @brief Gets the last error string from the given index
* @param index The index
* @return Returns the last error string from the given index or empty when there was no error.
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_errorhandling_getbyindex
*/
fpl_common_api const char *fplGetErrorByIndex(const size_t index);
/**
* @brief Gets the count of total last errors
* @note This function can be called regardless of the initialization state!
* @return Returns the number of last errors or zero when there was no error.
* @see @ref section_category_errorhandling_count
*/
fpl_common_api size_t fplGetErrorCount();
/**
* @brief Clears all the current errors in the platform
* @note This function can be called regardless of the initialization state!
* @see @ref section_category_errorhandling_clear
*/
fpl_common_api void fplClearErrors();

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup DynamicLibrary Dynamic library loading
* @brief This category contains functions for loading dynamic libraries.
* @{
*/
// ----------------------------------------------------------------------------

//! A union containing the library handle for any platform
typedef union fplInternalDynamicLibraryHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 library handle
	fpl__Win32LibraryHandle win32LibraryHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX library handle
	fpl__POSIXLibraryHandle posixLibraryHandle;
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
* @brief Loads a dynamic library and returns if the load was successful or not.
* @param libraryFilePath The path to the library with included file extension (.dll / .so)
* @param outHandle The output handle @ref fplDynamicLibraryHandle
* @return Returns true when the library was loaded successfully, false otherwise.
* @see @ref section_category_dll_load
*/
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle);
/**
* @brief Returns the dynamic library procedure address for the given procedure name.
* @param handle The @ref fplDynamicLibraryHandle handle to the loaded library
* @param name The name of the procedure
* @return Returns the procedure address for the given procedure name or @ref fpl_null when procedure not found or library is not loaded.
* @see @ref section_category_dll_getprocaddr
*/
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name);
/**
* @brief Unloads the loaded library and resets the handle to zero.
* @param handle The library handle @ref fplDynamicLibraryHandle
* @see @ref fplDynamicLibraryUnload
*/
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Debug Debug
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Writes the given text into the debugger output stream
* @param text The text to write into the debugger output stream
* @note This function will only work in IDEs such as MSVC
* @see @ref subsection_category_logging_debug_out
*/
fpl_platform_api void fplDebugOut(const char *text);
/**
* @brief Writes the given formatted text into the debugger output stream
* @param format The format used for writing into the debugger output stream
* @param ... The dynamic arguments used for formatting the text.
* @note This function will only work in IDEs such as MSVC
* @see @ref subsection_category_logging_debug_out
*/
fpl_common_api void fplDebugFormatOut(const char *format, ...);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Console Console functions
* @brief This category contains function for handling console in/out
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Writes the given text to the standard output console buffer.
* @param text The text to write into standard output console.
* @note This is most likely just a wrapper call to fprintf(stdout)
*/
fpl_platform_api void fplConsoleOut(const char *text);
/**
* @brief Writes the given text to the standard error console buffer.
* @param text The text to write into standard error console.
* @note This is most likely just a wrapper call to fprintf(stderr)
*/
fpl_platform_api void fplConsoleError(const char *text);
/**
* @brief Wait for a character to be typed in the console input and return it.
* @note This is most likely just a wrapper call to getchar()
* @return Returns the character typed in in the console input
*/
fpl_platform_api char fplConsoleWaitForCharInput();

/**
* @brief Writes the given formatted text to the standard output console buffer.
* @param format The format used for writing into the standard output console
* @param ... The dynamic arguments used for formatting the text
* @note This is most likely just a wrapper call to vfprintf(stdout)
*/
fpl_common_api void fplConsoleFormatOut(const char *format, ...);
/**
* @brief Writes the given formatted text to the standard error console buffer.
* @param format The format used for writing into the standard error console
* @param ... The dynamic arguments used for formatting the text
* @note This is most likely just a wrapper call to vfprintf(stderr)
*/
fpl_common_api void fplConsoleFormatError(const char *format, ...);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Timings Timing functions
* @brief This category contains functions for time comparisons
* @{
*/
// ----------------------------------------------------------------------------

//! A structure storing a timestamp, used for delta measurements only.
typedef union fplTimestamp {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 specifics
	struct {
		//! Query performance count in 10th nanoseconds
		fpl__Win32LargeInteger qpc;
		//! Tick count in milliseconds
		uint64_t ticks;
	} win32;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX specifics
	struct {
		//! Number of seconds
		uint64_t seconds;
		//! Number of nanoseconds
		int64_t nanoSeconds;
	} posix;
#endif
	//! Field for preventing union to be empty
	uint64_t unused;
} fplTimestamp;

//! A type definition for a timeout value in milliseconds
typedef uint32_t fplTimeoutValue;
//! Infinite timeout constant
#define FPL_TIMEOUT_INFINITE UINT32_MAX

//! A type definition for seconds / 64-bit.
typedef double fplSeconds;

//! A type definition for milliseconds / 64-bit.
typedef uint64_t fplMilliseconds;

/**
* @brief Gets the current @ref fplTimestamp with most precision, used for time delta measurements only.
* @return Returns the resulting @ref fplTimestamp.
* @note Use @ref fplTimestampElapsed() to get the elapsed time.
*/
fpl_platform_api fplTimestamp fplTimestampQuery();
/**
* @brief Gets the current system clock in milliseconds, since some fixed starting point (OS start, System start, etc), used for time delta measurements only.
* @return Returns the number of milliseconds as @ref fplMilliseconds.
*/
fpl_platform_api fplMilliseconds fplMillisecondsQuery();
/**
* @brief Gets the delta value from two @ref fplTimestamp values in seconds.
* @return Returns the resulting elapsed time in seconds as @ref fplSeconds.
*/
fpl_platform_api fplSeconds fplTimestampElapsed(const fplTimestamp start, const fplTimestamp finish);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Threading Threading and synchronizations routines
* @brief This category contains functions/types for dealing with concurrent programming, such as threads, mutexes, conditions, etc.
* @{
*/
// ----------------------------------------------------------------------------

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

//! A type definition for mapping @ref fplThreadState into a 32-bit integer
typedef uint32_t fplThreadState;

//! Defines all possible thread priorities
typedef enum fplThreadPriority {
	//! Unknown priority
	fplThreadPriority_Unknown = -10,

	//! Idle priority (Only when nothing is going on)
	fplThreadPriority_Idle = -2,
	//! Low priority
	fplThreadPriority_Low = -1,
	//! Normal priority
	fplThreadPriority_Normal = 0,
	//! High priority
	fplThreadPriority_High = 1,
	//! Realtime priority (Time critical)
	fplThreadPriority_RealTime = 2,

	//! Lowest @ref fplThreadPriority
	fplThreadPriority_Lowest = fplThreadPriority_Idle,
	//! Highest @ref fplThreadPriority
	fplThreadPriority_Highest = fplThreadPriority_RealTime,

	//! First @ref fplThreadPriority
	fplThreadPriority_First = fplThreadPriority_Lowest,
	//! Last @ref fplThreadPriority
	fplThreadPriority_Last = fplThreadPriority_Highest,
} fplThreadPriority;

//! Forward declared thread handle
typedef struct fplThreadHandle fplThreadHandle;

/**
* @brief A callback to execute user code inside another thread
* @param thread The thread handle
* @param data The user data pointer
*/
typedef void (fpl_run_thread_callback)(const fplThreadHandle *thread, void *data);

//! A union containing the thread handle for any platform
typedef union fplInternalThreadHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 thread handle
	fpl__Win32ThreadHandle win32ThreadHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX thread handle
	fpl__POSIXThreadHandle posixThread;
#endif
} fplInternalThreadHandle;

//! Contains creation parameters for @ref fplThreadCreateWithParameters()
typedef struct fplThreadParameters {
	//! The user data passed to the run callback
	void *userData;
	//! The @ref fpl_run_thread_callback
	fpl_run_thread_callback *runFunc;
	//! The stack size in bytes or zero for using the default size
	size_t stackSize;
	//! The @ref fplThreadPriority
	fplThreadPriority priority;
} fplThreadParameters;

//! The thread handle structure
typedef struct fplThreadHandle {
	//! The internal thread handle
	fplInternalThreadHandle internalHandle;
	//! The initial @ref fplThreadParameters
	fplThreadParameters parameters;
	//! Thread state
	volatile fplThreadState currentState;
	//! The identifier of the thread
	uint32_t id;
	//! Is this thread valid
	volatile fpl_b32 isValid;
	//! Is this thread stopping
	volatile fpl_b32 isStopping;
} fplThreadHandle;

#if defined(FPL_PLATFORM_WINDOWS)
typedef struct fpl__Win32InternalSemaphore {
	//! Semaphore handle
	fpl__Win32SemaphoreHandle handle;
	//! Semaphore value
	volatile int32_t value;
} fpl__Win32InternalSemaphore;
#endif

//! A union containing the internal semaphore handle for any platform
typedef union fplInternalSemaphoreHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 semaphore handle
	fpl__Win32InternalSemaphore win32;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX semaphore handle
	fpl__POSIXSemaphoreHandle posixHandle;
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
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 mutex handle
	fpl__Win32MutexHandle win32CriticalSection;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX mutex handle
	fpl__POSIXMutexHandle posixMutex;
#endif
} fplInternalMutexHandle;

//! The mutex handle structure
typedef struct fplMutexHandle {
	//! Is it valid
	fpl_b32 isValid;
	//! The internal mutex handle
	fplInternalMutexHandle internalHandle;
} fplMutexHandle;

//! A union containing the internal signal handle for any platform
typedef union fplInternalSignalHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 event handle
	fpl__Win32SignalHandle win32EventHandle;
#elif defined(FPL_PLATFORM_LINUX)
	//! Linux event handle
	fpl__LinuxSignalHandle linuxEventHandle;
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
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 condition variable
	fpl__Win32ConditionVariable win32Condition;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX condition variable
	fpl__POSIXConditionVariable posixCondition;
#endif
	//! Field for preventing union to be empty
	int dummy;
} fplInternalConditionVariable;

//! The condition variable structure
typedef struct fplConditionVariable {
	//! The internal condition handle
	fplInternalConditionVariable internalHandle;
	//! Is it valid
	fpl_b32 isValid;
} fplConditionVariable;

/**
* @brief Gets the current thread state for the given thread
* @param thread The thread handle @ref fplThreadHandle
* @return Returns the current thread state @ref fplThreadState for the given thread
* @see @ref section_category_threading_threads_states
*/
fpl_common_api fplThreadState fplGetThreadState(fplThreadHandle *thread);
/**
* @brief Gets the thread handle for the main thread.
* @return Returns the immutable pointer to the @ref fplThreadHandle .
*/
fpl_common_api const fplThreadHandle *fplGetMainThread();
/**
* @brief Gets the number of available threads.
* @return Returns the number of available threads.
*/
fpl_common_api size_t GetAvailableThreadCount();
/**
* @brief Gets the number of used/active threads.
* @return Returns the number of used/acvtive threads.
*/
fpl_common_api size_t GetUsedThreadCount();
/**
* @brief Gets the thread id for the current thread.
* @return Returns the thread id for the current thread.
*/
fpl_platform_api uint32_t fplGetCurrentThreadId();
/**
* @brief Creates and starts a thread and returns the handle to it.
* @param runFunc The pointer to the @ref fpl_run_thread_callback
* @param data The user data pointer passed to the execution function callback
* @return Returns a pointer to the @ref fplThreadHandle structure or @ref fpl_null when the limit of active threads has been reached.
* @warning Do not free this thread context directly!
* @note The resources are automatically cleaned up when the thread terminates.
* @see @ref section_category_threading_threads_create
*/
fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_callback *runFunc, void *data);
/**
* @brief Creates and starts a thread from the specified @ref fplThreadParameters and returns the handle to it.
* @param parameters The pointer to the @ref fplThreadParameters
* @return Returns a pointer to the @ref fplThreadHandle structure or @ref fpl_null when the limit of active threads has been reached.
* @warning Do not free this thread context directly!
* @note The resources are automatically cleaned up when the thread terminates.
*/
fpl_platform_api fplThreadHandle *fplThreadCreateWithParameters(fplThreadParameters *parameters);
/**
* @brief Retrieves the current thread priority from the OS from the given @ref fplThreadHandle .
* @param thread The pointer to the @ref fplThreadHandle structure
* @return Returns the current @ref fplThreadPriority .
*/
fpl_platform_api fplThreadPriority fplGetThreadPriority(fplThreadHandle *thread);
/**
* @brief Changes the thread priority to the given one, for the given @ref fplThreadHandle .
* @param thread The pointer to the @ref fplThreadHandle structure
* @param newPriority The new @ref fplThreadPriority for the given thread
* @return Returns true when the priority was changed, false otherwise.
*/
fpl_platform_api bool fplSetThreadPriority(fplThreadHandle *thread, const fplThreadPriority newPriority);
/**
* @brief Let the current thread sleep for the given amount of milliseconds.
* @param milliseconds Number of milliseconds to sleep
* @note There is no guarantee that the OS sleeps for the exact amount of milliseconds! This can vary based on the OS scheduler's granularity.
*/
fpl_platform_api void fplThreadSleep(const uint32_t milliseconds);
/**
* @brief Let the current thread yield execution to another thread that is ready to run on this core.
* @return Returns true when the functions succeed, false otherwise.
*/
fpl_platform_api bool fplThreadYield();
/**
* @brief Forces the given thread to stop and release all underlying resources.
* @param thread The pointer to the @ref fplThreadHandle structure
* @return True when the thread was terminated, false otherwise.
* @warning Do not free the given thread context manually!
* @note This thread context may get re-used for another thread in the future.
* @note Returns true when the threads were terminated, false otherwise.
* @see @ref section_category_threading_threads_terminate
*/
fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread);
/**
* @brief Wait until the given thread is done running or the given timeout has been reached.
* @param thread The pointer to the @ref fplThreadHandle structure
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when the thread completes or when the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_threads_wait_single
*/
fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout);
/**
* @brief Wait until all given threads are done running or the given timeout has been reached.
* @param threads The pointer to the first @ref fplThreadHandle pointer
* @param count The number of threads
* @param stride The size in bytes to the next thread handle. When this is set to zero, the array default is used.
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when all threads complete or when the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_threads_wait_all
*/
fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout);
/**
* @brief Wait until one of the given threads is done running or the given timeout has been reached.
* @param threads The pointer to the first @ref fplThreadHandle pointer
* @param count The number of threads
* @param stride The size in bytes to the next thread handle. When this is set to zero, the array default is used.
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when one thread completes or when the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_threads_wait_any
*/
fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout);

/**
* @brief Initializes the given mutex
* @param mutex The pointer to the @ref fplMutexHandle structure
* @return Returns true when the mutex was initialized, false otherwise.
* @note Use @ref fplMutexDestroy() when you are done with this mutex.
* @see @ref section_category_threading_mutexes_init
*/
fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex);
/**
* @brief Releases the given mutex and clears the structure to zero.
* @param mutex The pointer to the @ref fplMutexHandle structure
* @see @ref section_category_threading_mutexes_init
*/
fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex);
/**
* @brief Locks the given mutex and blocks any other threads.
* @param mutex The pointer to the @ref fplMutexHandle structure
* @return Returns true when the mutex was locked, false otherwise.
* @see @ref subsection_category_threading_mutexes_locking_lock
*/
fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex);
/**
* @brief Tries to lock the given mutex without blocking other threads.
* @param mutex The pointer to the @ref fplMutexHandle structure
* @return Returns true when the mutex was locked, false otherwise.
* @see @ref subsection_category_threading_mutexes_locking_probe
*/
fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex);
/**
 * @brief Unlocks the given mutex
 * @param mutex The pointer to the @ref fplMutexHandle structure
 * @return Returns true when the mutex was unlocked, false otherwise.
 * @see @ref subsection_category_threading_mutexes_locking_unlock
 */
fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex);

/**
* @brief Initializes the given signal
* @param signal The pointer to the @ref fplSignalHandle structure
* @param initialValue The initial value the signal is set to
* @return Returns true when initialization was successful, false otherwise.
* @note Use @ref fplSignalDestroy() when you are done with this Signal to release it.
* @see @ref section_category_threading_signals_init
*/
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue);
/**
* @brief Releases the given signal and clears the structure to zero.
* @param signal The pointer to the @ref fplSignalHandle structure
* @see @ref section_category_threading_signals_init
*/
fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal);
/**
* @brief Waits until the given signal is waked up.
* @param signal The pointer to the @ref fplSignalHandle structure
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when the signal woke up or the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_signals_wait_single
*/
fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout);
/**
* @brief Waits until all the given signals are waked up.
* @param signals The pointer to the first @ref fplSignalHandle pointer
* @param count The number of signals
* @param stride The size in bytes to the next signal handle. When this is set to zero, the array default is used.
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when all signals woke up or the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_signals_wait_all
*/
fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout);
/**
* @brief Waits until any of the given signals wakes up or the timeout has been reached.
* @param signals The pointer to the first @ref fplSignalHandle pointer
* @param count The number of signals
* @param stride The size in bytes to the next signal handle. When this is set to zero, the array default is used.
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when any of the signals woke up or the timeout has been reached, false otherwise.
* @see @ref subsection_category_threading_signals_wait_any
*/
fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout);
/**
* @brief Sets the signal and wakes up the given signal.
* @param signal The pointer to the @ref fplSignalHandle structure
* @return Returns true when the signal was set and broadcasted or false otherwise.
* @see @ref section_category_threading_signals_set
*/
fpl_platform_api bool fplSignalSet(fplSignalHandle *signal);
/**
* @brief Resets the signal.
* @param signal The pointer to the @ref fplSignalHandle structure
* @return Returns true when the signal was reset, false otherwise.
* @see @ref section_category_threading_signals_reset
*/
fpl_platform_api bool fplSignalReset(fplSignalHandle *signal);

/**
* @brief Initialize
  s the given condition
* @param condition The pointer to the @ref fplConditionVariable structure
* @return Returns true when initialization was successful, false otherwise.
* @note Use @ref fplSignalDestroy() when you are done with this Condition Variable to release its resources.
* @see @ref category_threading_conditions_init
*/
fpl_platform_api bool fplConditionInit(fplConditionVariable *condition);
/**
* @brief Releases the given condition and clears the structure to zero.
* @param condition The pointer to the @ref fplConditionVariable structure
* @see @ref category_threading_conditions_init
*/
fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition);
/**
* @brief Sleeps on the given condition and releases the mutex when done.
* @param condition The pointer to the @ref fplConditionVariable structure
* @param mutex The pointer to the mutex handle @ref fplMutexHandle structure
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when the function succeeds, false otherwise.
* @see @ref category_threading_conditions_wait_single
*/
fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout);
/**
* @brief Wakes up one thread that waits on the given condition.
* @param condition The pointer to the @ref fplConditionVariable structure
* @return Returns true when the function succeeds, false otherwise.
* @see @ref category_threading_conditions_signal
*/
fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition);
/**
* @brief Wakes up all threads that wait on the given condition.
* @param condition The pointer to the @ref fplConditionVariable structure
* @return Returns true when the function succeeds, false otherwise.
* @see @ref category_threading_conditions_broadcast
*/
fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition);

/**
* @brief Initializes the semaphore with the given initial value
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @param initialValue The initial value
* @return Returns true when the semaphores got initialized, false otherwise.
* @see @ref category_threading_semaphores_init
*/
fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue);
/**
* @brief Releases the internal semaphore resources
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @warning Do not call this when a thread is still waiting on this semaphore
* @see @ref category_threading_semaphores_init
*/
fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore);
/**
* @brief Waits for the semaphore until it gets signaled or the timeout has been reached.
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @param timeout The number of milliseconds to wait. When this is set to @ref FPL_TIMEOUT_INFINITE it will wait infinitely.
* @return Returns true when the semaphore got signaled, false otherwise.
* @note When a semaphore got signaled, the semaphore value is decreased by one.
* @see @ref category_threading_semaphores_wait
*/
fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout);
/**
* @brief Tries to wait for the semaphore until it gets signaled or return immediately.
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @return Returns true when the semaphore got signaled, false otherwise.
* @note When a semaphore got signaled, the semaphore value is decreased by one.
* @see @ref subsection_category_threading_semaphores_trywait
*/
fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore);
/**
* @brief Gets the current semaphore value
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @return Returns the current semaphore value
* @see @ref category_threading_semaphores_getvalue
*/
fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore);
/**
* @brief Increments the semaphore value by one
* @param semaphore The pointer to the @ref fplSemaphoreHandle structure
* @return Returns true when semaphore was incremented, false otherwise.
* @see @ref category_threading_semaphores_post
*/
fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Strings String functions
* @brief This category contains tons of functions for converting/manipulating strings
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Matches the given string by the given wildcard and returns a boolean indicating the match.
* @param source The source string
* @param wildcard The wildcard string
* @return Returns true when source string matches the wildcard, false otherwise.
*/
fpl_common_api bool fplIsStringMatchWildcard(const char *source, const char *wildcard);
/**
* @brief Compares two strings with constrained lengths and returns a boolean indicating the equality.
* @param a The first string
* @param aLen The number of characters for the first string
* @param b The second string
* @param bLen The number of characters for the second string
* @return Returns true when both strings are equal, false otherwise.
* @note Len parameters do not include the null-terminator!
*/
fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen);
/**
* @brief Compares two strings and returns a boolean indicating the equality.
* @param a The first string
* @param b The second string
* @return Returns true when both strings are equal, false otherwise.
*/
fpl_common_api bool fplIsStringEqual(const char *a, const char *b);
/**
* @brief Ensures that the given string always ends with a path separator with length constrained
* @param path The target path string
* @param maxPathLen The max length of the target path
* @return Returns a pointer to the last written character or @ref fpl_null.
*/
fpl_common_api char *fplEnforcePathSeparatorLen(char *path, size_t maxPathLen);
/**
* @brief Ensures that the given string always ends with a path separator
* @param path The path string
* @return Returns a pointer to the last written character or @ref fpl_null.
* @note This function is unsafe as it does not know the maximum length of the string!
*/
fpl_common_api char *fplEnforcePathSeparator(char *path);
/**
* @brief Appends the source string to the given buffer constrained by length
* @param appended The appending source string
* @param appendedLen The length of the appending source string
* @param buffer The target buffer
* @param maxBufferLen The max length of the target buffer
* @return Returns a pointer to the last written character or @ref fpl_null.
*/
fpl_common_api char *fplStringAppendLen(const char *appended, const size_t appendedLen, char *buffer, size_t maxBufferLen);
/**
* @brief Appends the source string to the given buffer
* @param appended The appending source string
* @param buffer The target buffer
* @param maxBufferLen The max length of the target buffer
* @return Returns a pointer to the last written character or @ref fpl_null.
*/
fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen);
/**
* @brief Counts the number of characters without including the zero terminator.
* @param str The string source
* @return Returns the number of characters of the given string or zero when the input string is fpl_null.
*/
fpl_common_api size_t fplGetStringLength(const char *str);
/**
* @brief Copies the given source string with a constrained length into a destination string.
* @param source The source string
* @param sourceLen The number of characters to copy
* @param dest The destination string buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the pointer to the last written character or @ref fpl_null.
* @note Null terminator is included always.
*/
fpl_common_api char *fplCopyStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen);
/**
* @brief Copies the given source string into a destination string.
* @param source The source string
* @param dest The destination string buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the pointer to the last written character or @ref fpl_null.
* @note Null terminator is included always.
*/
fpl_common_api char *fplCopyString(const char *source, char *dest, const size_t maxDestLen);
/**
* @brief Converts the given 16-bit source wide string with length in an 8-bit UTF-8 ANSI string.
* @param wideSource The 16-bit source wide string
* @param wideSourceLen The number of characters of the source wide string
* @param utf8Dest The 8-bit destination ANSI string buffer
* @param maxUtf8DestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @note Null terminator is included always.
*/
fpl_platform_api size_t fplWideStringToUTF8String(const wchar_t *wideSource, const size_t wideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen);
/**
* @brief Converts the given 8-bit UTF-8 source ANSI string with length in a 16-bit wide string.
* @param utf8Source The 8-bit source ANSI string
* @param utf8SourceLen The number of characters of the UTF-8 source ANSI string
* @param wideDest The 16-bit destination wide string buffer
* @param maxWideDestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @note Null terminator is included always.
*/
fpl_platform_api size_t fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen);
/**
* @brief Fills out the given destination string buffer with a formatted string, using the format specifier and variable arguments.
* @param destBuffer The destination string buffer
* @param maxDestBufferLen The total number of characters available in the destination buffer
* @param format The string format
* @param ... The variable arguments
* @return Returns the number of required/written characters, excluding the null-terminator
* @note This is most likely just a wrapper call to vsnprintf()
*/
fpl_common_api size_t fplStringFormat(char *destBuffer, const size_t maxDestBufferLen, const char *format, ...);
/**
* @brief Fills out the given destination string buffer with a formatted string, using the format specifier and the arguments list.
* @param destBuffer The destination string buffer
* @param maxDestBufferLen The total number of characters available in the destination buffer
* @param format The string format
* @param argList The arguments list
* @return Returns the number of required/written characters, excluding the null-terminator
* @note This is most likely just a wrapper call to vsnprintf()
*/
fpl_common_api size_t fplStringFormatArgs(char *destBuffer, const size_t maxDestBufferLen, const char *format, va_list argList);

/**
* @brief Converts the given string into a 32-bit integer constrained by string length
* @param str The source string
* @param len The length of the source string
* @return Returns a 32-bit integer converted from the given string
*/
fpl_common_api int32_t fplStringToS32Len(const char *str, const size_t len);

/**
* @brief Converts the given string into a 32-bit integer.
* @param str The source string
* @return Returns a 32-bit integer converted from the given string
*/
fpl_common_api int32_t fplStringToS32(const char *str);

/**
* @brief Converts the given 32-bit integer value into a string.
* @param value The source value
* @param maxBufferLen The maximum length of the buffer
* @param buffer The target buffer
* @return Returns the number of required/written characters, excluding the null-terminator
*/
fpl_common_api size_t fplS32ToString(const int32_t value, char *buffer, const size_t maxBufferLen);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Files Files/IO functions
* @brief This category contains and types and functions for handling files & directories
* @{
*/
// ----------------------------------------------------------------------------

//! A union containing the internal filehandle for any platform
typedef union fplInternalFileHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 filehandle
	fpl__Win32FileHandle win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! POSIX filehandle
	fpl__POSIXFileHandle posixFileHandle;
#endif
} fplInternalFileHandle;

//! The filehandle structure
typedef struct fplFileHandle {
	//! Internal filehandle
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
	fplFileAttributeFlags_Normal = 1 << 0,
	//! Hidden
	fplFileAttributeFlags_Hidden = 1 << 1,
	//! System
	fplFileAttributeFlags_System = 1 << 2,
	//! Archive
	fplFileAttributeFlags_Archive = 1 << 3
} fplFileAttributeFlags;
//! FileAttributeFlags operator overloads for C++
FPL_ENUM_AS_FLAGS_OPERATORS(fplFileAttributeFlags);

//! A union containing the internal filehandle for any platform
typedef union fplInternalFileEntryHandle {
#if defined(FPL_PLATFORM_WINDOWS)
	//! Win32 filehandle
	fpl__Win32FileHandle win32FileHandle;
#elif defined(FPL_SUBPLATFORM_POSIX)
	//! Posix directory handle
	fpl__POSIXDirHandle posixDirHandle;
#endif
} fplInternalFileEntryHandle;

//! A structure containing the internal root file informations
typedef struct fplInternalFileRootInfo {
	//! Saved root path
	const char *rootPath;
	//! Saved filter wildcard
	const char *filter;
} fplInternalFileRootInfo;

//! The elapsed seconds since the UNIX epoch (1970-01-01 00:00:00)
typedef uint64_t fplFileTimeStamp;

//! A structure containing filestamps for creation/access/modify date
typedef struct fplFileTimeStamps {
	//! Creation timestamp
	fplFileTimeStamp creationTime;
	//! Last access timestamp
	fplFileTimeStamp lastAccessTime;
	//! Last modify timestamp
	fplFileTimeStamp lastModifyTime;
} fplFileTimeStamps;

//! A structure containing the informations for a file or directory (name, type, attributes, etc.)
typedef struct fplFileEntry {
	//! Name
	char name[FPL_MAX_FILENAME_LENGTH];
	//! Internal filehandle
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
* @brief Opens a binary file for reading from a string path and returns the handle of it.
* @param filePath The file path
* @param outHandle The pointer to the @ref fplFileHandle structure
* @return Returns true when the binary file was opened, false otherwise.
* @see @ref subsection_category_io_binaryfiles_read_open
*/
fpl_platform_api bool fplFileOpenBinary(const char *filePath, fplFileHandle *outHandle);
/**
* @brief Create a binary file for writing to the given string path and returns the handle of it.
* @param filePath The file path
* @param outHandle The pointer to the @ref fplFileHandle structure
* @return Returns true when the binary file was created, false otherwise.
* @see @ref subsection_category_io_binaryfiles_write_create
*/
fpl_platform_api bool fplFileCreateBinary(const char *filePath, fplFileHandle *outHandle);
/**
* @brief Reads a block from the given file and returns the number of bytes read.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param sizeToRead The number of bytes to read
* @param targetBuffer The target memory to write into
* @param maxTargetBufferSize Total number of bytes available in the target buffer
* @return Returns the number of bytes read or zero.
* @note Supports max size of 2^31
* @see @ref subsection_category_io_binaryfiles_read_readblock
*/
fpl_platform_api uint32_t fplFileReadBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
/**
* @brief Reads a block from the given file and returns the number of bytes read.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param sizeToRead The number of bytes to read
* @param targetBuffer The target memory to write into
* @param maxTargetBufferSize Total number of bytes available in the target buffer
* @return Returns the number of bytes read or zero.
* @note Supports max size of 2^63
* @see @ref subsection_category_io_binaryfiles_read_readblock
*/
fpl_platform_api uint64_t fplFileReadBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize);
/**
* @brief Reads a block from the given file and returns the number of bytes read.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param sizeToRead The number of bytes to read
* @param targetBuffer The target memory to write into
* @param maxTargetBufferSize Total number of bytes available in the target buffer
* @return Returns the number of bytes read or zero.
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
* @see @ref subsection_category_io_binaryfiles_read_readblock
*/
fpl_platform_api size_t fplFileReadBlock(const fplFileHandle *fileHandle, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize);
/**
* @brief Writes a block to the given file and returns the number of written bytes.
* @param fileHandle The pointer to the filehandle @ref fplFileHandle
* @param sourceBuffer Source memory to read from
* @param sourceSize Number of bytes to write
* @return Returns the number of bytes written or zero.
* @note Supports max size of 2^31
* @see @ref subsection_category_io_binaryfiles_write_writeblock
*/
fpl_platform_api uint32_t fplFileWriteBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
/**
* @brief Writes a block to the given file and returns the number of written bytes.
* @param fileHandle The pointer to the filehandle @ref fplFileHandle
* @param sourceBuffer Source memory to read from
* @param sourceSize Number of bytes to write
* @return Returns the number of bytes written or zero.
* @note Supports max size of 2^63
* @see @ref subsection_category_io_binaryfiles_write_writeblock
*/
fpl_platform_api uint64_t fplFileWriteBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize);
/**
* @brief Writes a block to the given file and returns the number of written bytes.
* @param fileHandle The pointer to the filehandle @ref fplFileHandle
* @param sourceBuffer Source memory to read from
* @param sourceSize Number of bytes to write
* @return Returns the number of bytes written or zero.
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
* @see @ref subsection_category_io_binaryfiles_write_writeblock
*/
fpl_common_api size_t fplFileWriteBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize);
/**
* @brief Sets the current file position by the given position, depending on the mode it's absolute or relative.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param position Position in bytes
* @param mode Position mode
* @note Supports max size of 2^31
* @see @ref subsection_category_io_binaryfiles_pos_set
*/
fpl_platform_api uint32_t fplFileSetPosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode);
/**
* @brief Sets the current file position by the given position, depending on the mode it's absolute or relative.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param position Position in bytes
* @param mode Position mode
* @note Supports max size of 2^63
* @see @ref subsection_category_io_binaryfiles_pos_set
*/
fpl_platform_api uint64_t fplFileSetPosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode);
/**
* @brief Sets the current file position by the given position, depending on the mode it's absolute or relative.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param position Position in bytes
* @param mode Position mode
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
* @see @ref subsection_category_io_binaryfiles_pos_set
*/
fpl_common_api size_t fplFileSetPosition(const fplFileHandle *fileHandle, const intptr_t position, const fplFilePositionMode mode);
/**
* @brief Gets the current file position in bytes.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the current file position in bytes.
* @note Supports max size of 2^31
* @see @ref subsection_category_io_binaryfiles_pos_get
*/
fpl_platform_api uint32_t fplFileGetPosition32(const fplFileHandle *fileHandle);
/**
* @brief Gets the current file position in bytes.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the current file position in bytes.
* @note Supports max size of 2^63
* @see @ref subsection_category_io_binaryfiles_pos_get
*/
fpl_platform_api uint64_t fplFileGetPosition64(const fplFileHandle *fileHandle);
/**
* @brief Gets the current file position in bytes.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the current file position in bytes.
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
* @see @ref subsection_category_io_binaryfiles_pos_get
*/
fpl_common_api size_t fplFileGetPosition(const fplFileHandle *fileHandle);
/**
* @brief Flushes the buffers of the given file and causes all buffered data to be written to a file.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns true when file buffer was flushed, false otherwise.
*/
fpl_platform_api bool fplFileFlush(fplFileHandle *fileHandle);
/**
* @brief Closes the given file and releases the underlying resources and clears the handle to zero.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @see @ref subsection_category_io_binaryfiles_read_open
*/
fpl_platform_api void fplFileClose(fplFileHandle *fileHandle);

/**
* @brief Gets the file size in bytes for the given file.
* @param filePath The path to the file
* @return Returns the file size in bytes or zero.
* @note Supports max size of 2^31
*/
fpl_platform_api uint32_t fplFileGetSizeFromPath32(const char *filePath);
/**
* @brief Gets the file size in bytes for the given file.
* @param filePath The path to the file
* @return Returns the file size in bytes or zero.
* @note Supports max size of 2^63
*/
fpl_platform_api uint64_t fplFileGetSizeFromPath64(const char *filePath);
/**
* @brief Gets the file size in bytes for the given file.
* @param filePath The path to the file
* @return Returns the file size in bytes or zero.
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
*/
fpl_platform_api size_t fplFileGetSizeFromPath(const char *filePath);
/**
* @brief Gets the file size in bytes for an opened file.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the file size in bytes or zero.
* @note Supports max size of 2^31
*/
fpl_platform_api uint32_t fplFileGetSizeFromHandle32(const fplFileHandle *fileHandle);
/**
* @brief Gets the file size in bytes for an opened file.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the file size in bytes or zero.
* @note Supports max size of 2^63
*/
fpl_platform_api uint64_t fplFileGetSizeFromHandle64(const fplFileHandle *fileHandle);
/**
* @brief Gets the file size in bytes for an opened file.
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @return Returns the file size in bytes or zero.
* @note Depending on the platform/architecture, this supports a max size of 2^31 or 2^63 bytes
*/
fpl_common_api size_t fplFileGetSizeFromHandle(const fplFileHandle *fileHandle);
/**
* @brief Gets the timestamps for the given file
* @param filePath The path to the file
* @param outStamps The pointer to the @ref fplFileTimeStamps structure
* @return Returns true when the function succeeded, false otherwise.
*/
fpl_platform_api bool fplFileGetTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps);
/**
* @brief Gets the timestamps for an opened file
* @param fileHandle The pointer to the @ref fplFileHandle structure
* @param outStamps The pointer to the @ref fplFileTimeStamps structure
* @return Returns true when the function succeeded, false otherwise.
*/
fpl_platform_api bool fplFileGetTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps);
/**
* @brief Sets the timestamps for the given file
* @param filePath The path to the file
* @param timeStamps The pointer to the @ref fplFileTimeStamps structure
* @return Returns true when the function succeeded, false otherwise.
*/
fpl_platform_api bool fplFileSetTimestamps(const char *filePath, const fplFileTimeStamps *timeStamps);
/**
* @brief Checks if the file exists and returns a boolean indicating the existence.
* @param filePath The path to the file
* @return Returns true when the file exists, false otherwise.
*/
fpl_platform_api bool fplFileExists(const char *filePath);
/**
* @brief Copies the given source file to the target path and returns true when the copy was successful.
* @param sourceFilePath The source file path
* @param targetFilePath The target file path
* @param overwrite The overwrite boolean indicating if the file can be overwritten or not
* @return Returns true when the file was copied, false otherwise.
*/
fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
/**
* @brief Movies the given source file to the target file and returns true when the move was successful.
* @param sourceFilePath The source file path
* @param targetFilePath The target file path
* @return Returns true when the file was moved, false otherwise.
*/
fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath);
/**
* @brief Deletes the given file without confirmation and returns true when the deletion was successful.
* @param filePath The path to the file
* @return Returns true when the file was deleted, false otherwise.
*/
fpl_platform_api bool fplFileDelete(const char *filePath);

/**
* @brief Creates all the directories in the given path.
* @param path The path to the directory
* @return Returns true when at least one directory was created, false otherwise.
*/
fpl_platform_api bool fplDirectoriesCreate(const char *path);
/**
* @brief Checks if the given directory exists and returns a boolean indicating its existence.
* @param path The path to the directory
* @return Returns true when the directory exists, false otherwise.
*/
fpl_platform_api bool fplDirectoryExists(const char *path);
/**
* @brief Deletes the given empty directory without confirmation and returns true when the deletion was successful.
* @param path The path to the directory.
* @return Returns true when the empty directory was deleted, false otherwise.
*/
fpl_platform_api bool fplDirectoryRemove(const char *path);
/**
* @brief Iterates through files/directories in the given directory.
* @param path The full path
* @param filter The filter wildcard (If empty or null it will not filter anything at all)
* @param entry The pointer to the @ref fplFileEntry structure
* @return Returns true when there was a first entry found, false otherwise.
* @note This function is not recursive, so it will traverse the first level only!
* @note When no initial entry is found, the resources are automatically cleaned up.
* @see @ref section_category_io_paths_traversing
*/
fpl_platform_api bool fplDirectoryListBegin(const char *path, const char *filter, fplFileEntry *entry);
/**
* @brief Gets the next file entry from iterating through files/directories.
* @param entry The pointer to the @ref fplFileEntry structure
* @return Returns true when there was a next file otherwise false if not.
* @note This function is not recursive, so it will traverse the first level only!
* @note When no entries are found, the resources are automatically cleaned up.
* @see @ref section_category_io_paths_traversing
*/
fpl_platform_api bool fplDirectoryListNext(fplFileEntry *entry);
/**
* @brief Releases opened resources from iterating through files/directories.
* @param entry The pointer to the @ref fplFileEntry structure
* @note It's safe to call this when the file entry is already closed.
* @see @ref section_category_io_paths_traversing
*/
fpl_platform_api void fplDirectoryListEnd(fplFileEntry *entry);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup Paths Path functions
* @brief This category contains functions for retrieving and building paths
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Gets the full path to this executable, including the executable file name.
* @param destPath The destination buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref subsection_category_io_paths_get_exepath
*/
fpl_platform_api size_t fplGetExecutableFilePath(char *destPath, const size_t maxDestLen);
/**
* @brief Gets the full path to your home directory.
* @param destPath The destination buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref subsection_category_io_paths_get_home
*/
fpl_platform_api size_t fplGetHomePath(char *destPath, const size_t maxDestLen);
/**
* @brief Extracts the directory path from the given file path.
* @param sourcePath The source path to extract from
* @param destPath The destination buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref subsection_category_io_paths_utils_extractfilepath
*/
fpl_common_api size_t fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen);
/**
* @brief Extracts the file extension from the given source path.
* @param sourcePath The source path to extract from
* @return Returns the pointer to the first character of the extension.
* @see @ref subsection_category_io_paths_utils_extractfileext
*/
fpl_common_api const char *fplExtractFileExtension(const char *sourcePath);
/**
* @brief Extracts the file name including the file extension from the given source path.
* @param sourcePath The source path to extract from
* @return Returns the pointer to the first character of the filename.
* @see @ref subsection_category_io_paths_utils_extractfilename
*/
fpl_common_api const char *fplExtractFileName(const char *sourcePath);
/**
* @brief Changes the file extension on the given source path and writes the result into a destination buffer.
* @param filePath The File path to search for the extension
* @param newFileExtension The new file extension
* @param destPath The destination buffer
* @param maxDestLen The total number of characters available in the destination buffer
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref subsection_category_io_paths_utils_changefileext
*/
fpl_common_api size_t fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen);
/**
* @brief Combines all given paths by the platforms path separator for a fixed number of arguments
* @param destPath The destination buffer
* @param maxDestPathLen The total number of characters available in the destination buffer
* @param pathCount The number of dynamic path arguments
* @param ... The dynamic path arguments
* @return Returns the number of required/written characters, excluding the null-terminator
* @see @ref subsection_category_io_paths_utils_pathcombine
*/
fpl_common_api size_t fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...);

/** @} */

#if defined(FPL__ENABLE_WINDOW)
// ----------------------------------------------------------------------------
/**
* @defgroup WindowEvents Window events
* @brief This category contains types/functions for handling window events
* @{
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
	//! Dropped one or more files into the window
	fplWindowEventType_DroppedFiles,
	//! Window was exposed
	fplWindowEventType_Exposed,
	//! Window was moved
	fplWindowEventType_PositionChanged,
	//! Window was closed
	fplWindowEventType_Closed,
	//! Window was shown
	fplWindowEventType_Shown,
	//! Window was hidden
	fplWindowEventType_Hidden,
} fplWindowEventType;

//! A structure containing number and dropped files informations
typedef struct fplWindowDropFiles {
	//! The internal memory block
	fplMemoryBlock internalMemory;
	//! File paths (Do not release this memory, its automatically released after the event is processed)
	const char **files;
	//! Number of dropped in files
	size_t fileCount;
} fplWindowDropFiles;

//! A structure containing window event data (Size, Position, etc.)
typedef struct fplWindowEvent {
	//! Window event type
	fplWindowEventType type;
	union {
		//! Window size
		fplWindowSize size;
		//! Window position
		fplWindowPosition position;
		//! Drop files
		fplWindowDropFiles dropFiles;
	};
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
	//! Raw ascii key code or 32-bit unicode for text input.
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
	//! Max mouse button count
	fplMouseButtonType_MaxCount,
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

//! An enumeration of gamepad buttons
typedef enum fplGamepadButtonType {
	//! DPad up
	fplGamepadButtonType_DPadUp = 0,
	//! DPad right
	fplGamepadButtonType_DPadRight,
	//! DPad down
	fplGamepadButtonType_DPadDown,
	//! DPad left
	fplGamepadButtonType_DPadLeft,

	//! Action-A
	fplGamepadButtonType_ActionA,
	//! Action-B
	fplGamepadButtonType_ActionB,
	//! Action-X
	fplGamepadButtonType_ActionX,
	//! Action-Y
	fplGamepadButtonType_ActionY,

	//! Start
	fplGamepadButtonType_Start,
	//! Back
	fplGamepadButtonType_Back,

	//! Left-Thumb
	fplGamepadButtonType_LeftThumb,
	//! Right-Thumb
	fplGamepadButtonType_RightThumb,

	//! Left-Shoulder
	fplGamepadButtonType_LeftShoulder,
	//! Right-Shoulder
	fplGamepadButtonType_RightShoulder,
} fplGamepadButtonType;

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

	//! Device name
	const char *deviceName;

	//! Analog left thumb X in range (-1.0 to 1.0f)
	float leftStickX;
	//! Analog left thumb Y in range (-1.0 to 1.0f)
	float leftStickY;
	//! Analog right thumb X in range (-1.0 to 1.0f)
	float rightStickX;
	//! Analog right thumb Y in range (-1.0 to 1.0f)
	float rightStickY;

	//! Analog left trigger in range (0.0 to 1.0f)
	float leftTrigger;
	//! Analog right trigger in range (0.0 to 1.0f)
	float rightTrigger;

	//! Is device physical connected
	fpl_b32 isConnected;
	//! Is this device active, which means are any buttons pressed or positions stick changed.
	fpl_b32 isActive;
} fplGamepadState;

//! A structure containing gamepad event data (Type, Device, State, etc.)
typedef struct fplGamepadEvent {
	//! Full gamepad state
	fplGamepadState state;
	//! Device name of the controller
	const char *deviceName;
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
* @brief Polls the next event from the internal event queue or from the OS, handles them, and removes it from the queue.
* @param ev The pointer to the @ref fplEvent structure
* @return Returns false when there are no events left, true otherwise.
* @see @ref section_category_window_events_polling
*/
fpl_platform_api bool fplPollEvent(fplEvent *ev);

/**
* @brief Polls all the events from the OS and clears the internal event queue.
* @warning Dont use this function if you want to handle the events. Use @ref fplPollEvent() instead!
* @see @ref section_category_window_events_process
*/
fpl_platform_api void fplPollEvents();

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup InputData Input functions
* @brief This category contains functions for retrieving input data
* @{
*/
// ----------------------------------------------------------------------------

//! Max number of keyboard states
#define FPL_MAX_KEYBOARD_STATE_COUNT 256

//! A struct containing the full keyboard state
typedef struct fplKeyboardState {
	//! Modifier flags
	fplKeyboardModifierFlags modifiers;
	//! Key states
	fpl_b32 keyStatesRaw[FPL_MAX_KEYBOARD_STATE_COUNT];
	//! Mapped button states
	fplButtonState buttonStatesMapped[FPL_MAX_KEYBOARD_STATE_COUNT];
} fplKeyboardState;

//! Max number of gamepad states
#define FPL_MAX_GAMEPAD_STATE_COUNT 4

//! A struct containing the full state for all gamepad devices
typedef struct fplGamepadStates {
	//! Device states
	fplGamepadState deviceStates[FPL_MAX_GAMEPAD_STATE_COUNT];
} fplGamepadStates;

//! A struct containing the full mouse state
typedef struct fplMouseState {
	//! Mouse button states mapped to @ref fplMouseButtonType
	fplButtonState buttonStates[fplMouseButtonType_MaxCount];
	//! X-Position in pixels
	int32_t x;
	//! Y-Position in pixels
	int32_t y;
} fplMouseState;

/**
* @brief Polls the current keyboard state and writes it out into the output structure.
* @param outState The pointer to the @ref fplKeyboardState structure
* @see @ref subsection_category_input_polling_keyboard
*/
fpl_platform_api bool fplPollKeyboardState(fplKeyboardState *outState);
/**
* @brief Polls the current gamepad states and writes it out into the output structure.
* @param outStates The pointer to the @ref fplGamepadStates structure
* @see @ref subsection_category_input_polling_gamepad
*/
fpl_platform_api bool fplPollGamepadStates(fplGamepadStates *outStates);
/**
* @brief Polls the current mouse state and writes it out into the output structure.
* @param outState The pointer to the @ref fplMouseState structure
* @see @ref subsection_category_input_polling_mouse
*/
fpl_platform_api bool fplPollMouseState(fplMouseState *outState);

/**
* @brief Queries the cursor position in screen coordinates, relative to the root screen.
* @param outX The pointer to the out going X position
* @param outY The pointer to the out going Y position
*/
fpl_platform_api bool fplQueryCursorPosition(int32_t *outX, int32_t *outY);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup WindowBase Window functions
* @brief This category contains functions for handling the window
* @{
*/
// ----------------------------------------------------------------------------

//! An enumeration containg the states of a window
typedef enum fplWindowState {
	//! Unknown state
	fplWindowState_Unknown = 0,
	//! Normal window state
	fplWindowState_Normal,
	//! Iconify/Minimize window state
	fplWindowState_Iconify,
	//! Maximize window state
	fplWindowState_Maximize,
	//! Fullscreen state
	fplWindowState_Fullscreen,
} fplWindowState;

//! An enumeration containing the visibility state of a window
typedef enum fplWindowVisibilityState {
	//! Unknown state
	fplWindowVisibilityState_Unknown = 0,
	//! Window is visible
	fplWindowVisibilityState_Show,
	//! Window is hidden
	fplWindowVisibilityState_Hide,
} fplWindowVisibilityState;

/**
* @brief Gets the window running state as a boolean
* @return Returns true when the window is running, false otherwise
*/
fpl_platform_api bool fplIsWindowRunning();
/**
* @brief Closes the window and stops the event loop
*/
fpl_platform_api void fplWindowShutdown();
/**
* @brief Clears the internal event queue and updates input devices if needed
* @return Returns true when the window is still active, false otherwise
*/
fpl_platform_api bool fplWindowUpdate();
/**
* @brief Enables or disables the window cursor
* @param value The new cursor visibility state
*/
fpl_platform_api void fplSetWindowCursorEnabled(const bool value);
/**
* @brief Retrieves the inner window size.
* @param outSize The pointer to the @ref fplWindowSize structure
* @return Returns true when we got the inner size from the current window, false otherwise.
*/
fpl_platform_api bool fplGetWindowSize(fplWindowSize *outSize);
/**
* @brief Resizes the window to fit the inner size based on the given size.
* @param width The width in screen units
* @param height The height in screen units
*/
fpl_platform_api void fplSetWindowSize(const uint32_t width, const uint32_t height);
/**
* @brief Gets the window resizable state as boolean.
* @return Returns true when the window is resizable, false otherwise.
*/
fpl_platform_api bool fplIsWindowResizable();
/**
* @brief Enables or disables the ability to resize the window.
* @param value The new resizable state
*/
fpl_platform_api void fplSetWindowResizeable(const bool value);
/**
* @brief Gets the window decorated state as boolean.
* @return Returns true when the window is decorated, false otherwise.
*/
fpl_platform_api bool fplIsWindowDecorated();
/**
* @brief Enables or disables the window decoration (Titlebar, Border, etc.).
* @param value The new decorated state
*/
fpl_platform_api void fplSetWindowDecorated(const bool value);
/**
* @brief Gets the window floating state as boolean.
* @return Returns true when the window is floating, false otherwise.
*/
fpl_platform_api bool fplIsWindowFloating();
/**
* @brief Enables or disables the window floating (Top-most)
* @param value The new floating state
*/
fpl_platform_api void fplSetWindowFloating(const bool value);
/**
* @brief Enables or disables fullscreen mode based on the given size and the current display.
* @param value The new fullscreen state
* @param fullscreenWidth The fullscreen width in screen units. When set to zero the current display position is used.
* @param fullscreenHeight The fullscreen height in screen units. When set to zero the current display position is used.
* @param refreshRate The refresh rate in Hz. When set to zero the current display setting is used.
* @return Returns true when the window was changed to the desire fullscreen mode, false otherwise.
* @attention This may alter the display resolution or the refresh rate.
*/
fpl_platform_api bool fplSetWindowFullscreenSize(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate);
/**
* @brief Enables or disables fullscreen mode based on the given rectangle.
* @param value The new fullscreen state
* @param x The left position in virtual screen coordinates
* @param y The top position in virtual screen coordinates
* @param width The width in virtual screen coordinates
* @param height The height in virtual screen coordinates
* @return Returns true when the window was changed to the rectangle, false otherwise.
* @attention This will not alter the display resolution or the refresh rate.
*/
fpl_platform_api bool fplSetWindowFullscreenRect(const bool value, const int32_t x, const int32_t y, const int32_t width, const int32_t height);
/**
* @brief Enables fullscreen mode on the nearest display
* @return Returns true when the window was changed to the fullscreen, false otherwise.
* @attention This will not alter the display resolution or the refresh rate.
*/
fpl_platform_api bool fplEnableWindowFullscreen();
/**
* @brief Switches the window back to window mode
* @return Returns true when the window was changed to the window mode, false otherwise.
* @attention This will not alter the display resolution or the refresh rate.
*/
fpl_platform_api bool fplDisableWindowFullscreen();
/**
* @brief Gets the window fullscreen state as boolean.
* @return Returns true when the window is in fullscreen mode, false otherwise.
*/
fpl_platform_api bool fplIsWindowFullscreen();
/**
* @brief Retrieves the absolute window position.
* @param outPos The pointer to the @ref fplWindowPosition structure
* @return Returns true when we got the position, false otherwise.
*/
fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos);
/**
* @brief Changes the window absolute position to the given coordinates.
* @param left The left position in screen units
* @param top The top position in screen units
*/
fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top);
/**
* @brief Changes the window title to the given string.
* @param title The title string
*/
fpl_platform_api void fplSetWindowTitle(const char *title);
/**
* @brief Retrieves the window title and writes it into the output string
* @param outTitle The output title string
* @param maxOutTitleLength The maximum length of the output title
* @return Returns the char pointer of the last written character or fpl_null
*/
fpl_common_api char *fplGetWindowTitle(char *outTitle, const size_t maxOutTitleLength);
/**
* @brief Gets the current window state
* @return Returns the current window state
*/
fpl_platform_api fplWindowState fplGetWindowState();
/**
* @brief Changes the current window state
* @param newState The new window state
* @return Returns true when the window state was changed, false otherwise.
*/
fpl_platform_api bool fplSetWindowState(const fplWindowState newState);
/**
* @brief Enables or Disables the input events for the window entirely.
* @param enabled If set to true, the input handled are processed, if false no input events are handled.
* @note The text input event is always handled, regardless of this setting.
*/
fpl_common_api void fplSetWindowInputEvents(const bool enabled);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup WindowDisplay Display/Monitor functions
* @brief This category contains functions for handling multiple monitors
* @{
*/
// ----------------------------------------------------------------------------

//! A struct containing informations about a display
typedef struct fplDisplayInfo {
	//! ID of the display
	char id[FPL_MAX_NAME_LENGTH];
	//! Virtual size in screen coordinates
	fplWindowSize virtualSize;
	//! Virtual position in screen coordinates
	fplWindowPosition virtualPosition;
	//! Actual absolute size in screen coordinates
	fplWindowSize physicalSize;
	//! Is primary display
	fpl_b32 isPrimary;
} fplDisplayInfo;

//! A structure containing one set of display mode settings, such as size, refresh rate, etc.
typedef struct fplDisplayMode {
	//! The width in screen coordinates
	uint32_t width;
	//! The height in screen coordinates
	uint32_t height;
	//! Color depth in bits per pixel
	uint32_t colorBits;
	//! The refresh rate in Hz
	uint32_t refreshRate;
} fplDisplayMode;

/**
* @brief Gets the number of active displays
* @return Returns the number of active displays
*/
fpl_platform_api size_t fplGetDisplayCount();
/**
* @brief Gets informations about all active displays
* @param outDisplays The array of @ref fplDisplayInfo
* @param maxDisplayCount The maximum number of display infos available in the output array
* @return Returns the total number of active displays
*/
fpl_platform_api size_t fplGetDisplays(fplDisplayInfo *outDisplays, const size_t maxDisplayCount);
/**
* @brief Gets information about the display for the FPL window
* @param outInfo A pointer to a @ref fplDisplayInfo structure
* @return Returns true when the display for the window was found, false otherwise.
*/
fpl_platform_api bool fplGetWindowDisplay(fplDisplayInfo *outInfo);
/**
* @brief Gets information about the primary display
* @param outInfo A pointer to a @ref fplDisplayInfo structure
* @return Returns true when the primary display was found, false otherwise.
*/
fpl_platform_api bool fplGetPrimaryDisplay(fplDisplayInfo *outInfo);
/**
* @brief Finds the display from a cursor position and retrieves the information for it.
* @param x The x position in screen coordinates
* @param y The y position in screen coordinates
* @param outInfo A pointer to a @ref fplDisplayInfo structure
* @return Returns true when the display was found, false otherwise.
*/
fpl_platform_api bool fplGetDisplayFromPosition(const int32_t x, const int32_t y, fplDisplayInfo *outInfo);
/**
* @brief Gets the information about the available display modes for the given display id
* @param id The display id
* @param outModes The array of @ref fplDisplayMode
* @param maxDisplayModeCount The maximum number of display modes available in the output array
* @return Returns the number of found display modes
*/
fpl_platform_api size_t fplGetDisplayModes(const char *id, fplDisplayMode *outModes, const size_t maxDisplayModeCount);

/** @} */

// ----------------------------------------------------------------------------
/**
* @defgroup WindowClipboard Clipboard functions
* @brief This category contains functions for reading/writing clipboard data
* @{
*/
// ----------------------------------------------------------------------------

/**
* @brief Retrieves the current clipboard text.
* @param dest The destination string buffer to write the clipboard text into.
* @param maxDestLen The total number of characters available in the destination buffer.
* @return Returns true when the clipboard contained text which is copied into the dest buffer, @ref fpl_null otherwise.
*/
fpl_platform_api bool fplGetClipboardText(char *dest, const uint32_t maxDestLen);
/**
* @brief Overwrites the current clipboard text with the given one.
* @param text The new clipboard string.
* @return Returns true when the text in the clipboard was changed, false otherwise.
*/
fpl_platform_api bool fplSetClipboardText(const char *text);

/** @} */
#endif // FPL__ENABLE_WINDOW

#if defined(FPL__ENABLE_VIDEO)
// ----------------------------------------------------------------------------
/**
* @defgroup Video Video functions
* @brief This category contains functions for retrieving or resizing the video buffer
* @{
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
* @brief Makes a video rectangle from a LTRB rectangle
* @param left The left position in screen units
* @param top The top position in screen units
* @param right The right position in screen units
* @param bottom The bottom position in screen units
* @return Returns the computed video rectangle @ref fplVideoRect
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

#if defined(FPL__ENABLE_VIDEO_VULKAN)
//! Stores the surface properties for the Vulkan video backend
typedef struct fplVideoSurfaceVulkan {
	//! The Vulkan Instance (VkInstance)
	void *instance;
	//! The Vulkan Surface KHR (VkSurfaceKHR)
	void *surfaceKHR;
} fplVideoSurfaceVulkan;
#endif

#if defined(FPL__ENABLE_VIDEO_OPENGL)
//! Stores the surface properties for the OpenGL video backend
typedef struct fplVideoSurfaceOpenGL {
	//! The OpenGL rendering context (HGLRC or XRC)
	void *renderingContext;
} fplVideoSurfaceOpenGL;
#endif

#if defined(FPL_PLATFORM_WINDOWS)
//! Stores the window properties for Win32
typedef struct fplVideoWindowWin32 {
	//! The window handle
	fpl__Win32WindowHandle windowHandle;
	//! The device context
	fpl__Win32DeviceContext deviceContext;
} fplVideoWindowWin32;
#endif

#if defined(FPL_SUBPLATFORM_X11)
//! Stores the window properties X11
typedef struct fplVideoWindowX11 {
	//! The window handle
	fpl__X11Window window;
	//! The display handle
	fpl__X11Display display;
	//! The visual handle
	fpl__X11Visual visual;
	//! The screen id
	int screen;
} fplVideoWindowX11;
#endif // FPL_SUBPLATFORM_X11

//! Stores the video window handles
typedef union fplVideoWindow {
#if defined(FPL_PLATFORM_WINDOWS)
	fplVideoWindowWin32 win32;
#elif defined(FPL_SUBPLATFORM_X11)
	fplVideoWindowX11 x11;
#endif
	//! Field for preventing union to be empty
	int dummy;
} fplVideoWindow;

//! Stores the surface properties for the active video backend
typedef struct fplVideoSurface {
	//! The video window
	fplVideoWindow window;

#if defined(FPL__ENABLE_VIDEO_VULKAN)
	//! The Vulkan surface properties
	fplVideoSurfaceVulkan vulkan;
#endif

#if defined(FPL__ENABLE_VIDEO_OPENGL)
	//! The OpenGL surface properties
	fplVideoSurfaceOpenGL opengl;
#endif

	//! Field for preventing union to be empty
	int dummy;
} fplVideoSurface;

#if defined(FPL__ENABLE_VIDEO_VULKAN)
//! Stores the requirements for the Vulkan video backend
typedef struct fplVideoRequirementsVulkan {
	//! The required instance extensions
	const char *instanceExtensions[2];
	//! The number of required instance extensions
	uint32_t instanceExtensionCount;
} fplVideoRequirementsVulkan;
#endif // FPL__ENABLE_VIDEO_VULKAN


//! Stores the video requirements for the desired video backend
typedef union fplVideoRequirements {
#if defined(FPL__ENABLE_VIDEO_VULKAN)
	//! The requirements for Vulkan backend
	fplVideoRequirementsVulkan vulkan;
#endif // FPL__ENABLE_VIDEO_VULKAN
	//! Field for preventing union to be empty
	int dummy;
} fplVideoRequirements;

/**
* @brief Gets the current video backend
* @return Returns the current video backend type @ref fplVideoBackendType
*/
fpl_common_api fplVideoBackendType fplGetVideoBackendType();
/**
* @brief Gets a string that represents the given video backend
* @param backendType The video backend type @ref fplVideoBackendType
* @return Returns a string for the given video backend type
*/
fpl_common_api const char *fplGetVideoBackendName(fplVideoBackendType backendType);
/**
* @brief Retrieves the pointer to the current video backbuffer.
* @return Returns the pointer to the current @ref fplVideoBackBuffer.
* @warning Do not release this memory by any means, otherwise you will corrupt heap memory!
*/
fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer();
/**
* @brief Resizes the current video backbuffer.
* @param width The width in pixels
* @param height The height in pixels
* @return Returns true when video back buffer could be resized, false otherwise.
*/
fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height);

/**
* @brief Forces the window to be redrawn or to swap the back/front buffer.
*/
fpl_common_api void fplVideoFlip();

/**
* @brief Gets the procedure by the specified name from the active video backend
* @param procName The name of the procedure.
* @return Returns the function pointer of the procedure.
*/
fpl_common_api const void *fplGetVideoProcedure(const char *procName);

/**
* @brief Gets the current @ref fplVideoSurface that stores all handles used for the active video backend.
* @return The resulting @ref fplVideoSurface reference.
*/
fpl_common_api const fplVideoSurface *fplGetVideoSurface();

/**
* @brief Gets the video requirements for the specified video backend.
* @param backendType The @ref fplVideoBackendType
* @param requirements The reference to the @ref fplVideoRequirements
* @return Returns true when the @ref fplVideoRequirements are filled out, false otherwise.
*/
fpl_common_api bool fplGetVideoRequirements(const fplVideoBackendType backendType, fplVideoRequirements *requirements);

/** @} */
#endif // FPL__ENABLE_VIDEO

#if defined(FPL__ENABLE_AUDIO)
// ----------------------------------------------------------------------------
/**
* @defgroup Audio Audio functions
* @brief This category contains functions for start/stop playing audio and retrieving/changing some audio-related settings.
* @{
*/
// ----------------------------------------------------------------------------

//! An enumeration of audio results
typedef enum fplAudioResultType {
	//! No result
	fplAudioResultType_None = 0,
	//! Success
	fplAudioResultType_Success,
	//! The audio device is not initialized
	fplAudioResultType_DeviceNotInitialized,
	//! The audio device is already stopped
	fplAudioResultType_DeviceAlreadyStopped,
	//! The audio device is already started
	fplAudioResultType_DeviceAlreadyStarted,
	//! The audio device is busy/waiting
	fplAudioResultType_DeviceBusy,
	//! No audio device is found
	fplAudioResultType_NoDeviceFound,
	//! Failed to load the audio api
	fplAudioResultType_ApiFailed,
	//! The platform is not initialized
	fplAudioResultType_PlatformNotInitialized,
	//! The audio backend is already initialized
	fplAudioResultType_BackendAlreadyInitialized,
	//! The @ref fplAudioFormatType is not set
	fplAudioResultType_UnsetAudioFormat,
	//! The number of audio channels is not set
	fplAudioResultType_UnsetAudioChannels,
	//! The sample rate is not set
	fplAudioResultType_UnsetAudioSampleRate,
	//! The audio buffer size is not set
	fplAudioResultType_UnsetAudioBufferSize,
	//! Unknown error
	fplAudioResultType_Failed,

	//! First @ref fplAudioResultType
	fplAudioResultType_First = fplAudioResultType_None,
	//! Last @ref fplAudioResultType
	fplAudioResultType_Last = fplAudioResultType_Failed,
} fplAudioResultType;

/**
* @brief Gets the current audio backend type
* @return Returns the current audio backend type @ref fplAudioBackendType
*/
fpl_common_api fplAudioBackendType fplGetAudioBackendType();
/**
* @brief Start playing asynchronous audio.
* @return Returns the audio result @ref fplAudioResultType
*/
fpl_common_api fplAudioResultType fplPlayAudio();
/**
* @brief Stop playing asynchronous audio.
* @return Returns the audio result @ref fplAudioResultType
*/
fpl_common_api fplAudioResultType fplStopAudio();
/**
* @brief Retrieves the native format for the current audio device.
* @param outFormat The pointer to the @ref fplAudioDeviceFormat structure
* @return Returns true when a hardware format was active, false otherwise.
*/
fpl_common_api bool fplGetAudioHardwareFormat(fplAudioDeviceFormat *outFormat);
/**
* @brief Overwrites the audio client read callback.
* @param newCallback The pointer to the @ref fpl_audio_client_read_callback callback
* @param userData The pointer to the client/user data
* @return Returns true when an audio device is ready and the callback was set, false otherwise.
* @note This has no effect when audio is already playing, you have to call it when audio is in a stopped state!
*/
fpl_common_api bool fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData);
/**
* @brief Retrieves all playback audio devices.
* @param devices A array of audio device info @ref fplAudioDeviceInfo
* @param maxDeviceCount The total number of devices available in the devices array.
* @return Returns the number of devices found.
*/
fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount);
/**
* @brief Computes the number of bytes required to write one sample with one channel.
* @param format The audio format type @ref fplAudioFormatType
* @return Returns the number of bytes for one sample with one channel
*/
fpl_common_api uint32_t fplGetAudioSampleSizeInBytes(const fplAudioFormatType format);
/**
* @brief Gets the string that represents the given audio format type.
* @param format The audio format type @ref fplAudioFormatType
* @return Returns a string for the given audio format type
*/
fpl_common_api const char *fplGetAudioFormatName(const fplAudioFormatType format);
/**
* @brief Gets the string that represents the given audio backend type.
* @param backendType The audio backend type @ref fplAudioBackendType
* @return Returns a string for the given audio backend type
*/
fpl_common_api const char *fplGetAudioBackendName(fplAudioBackendType backendType);
/**
* @brief Computes the total number of frames for given sample rate and buffer size.
* @param sampleRate The sample rate in Hz
* @param bufferSizeInMilliSeconds The buffer size in milliseconds
* @return Returns the total number of frames for given sample rate and buffer size
*/
fpl_common_api uint32_t fplGetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds);
/**
* @brief Computes the duration in milliseconds for the given sample rate and frame count
* @param sampleRate The sample rate in Hz
* @param frameCount The number of frames
* @return Returns the duration in milliseconds
*/
fpl_common_api uint32_t fplGetAudioBufferSizeInMilliseconds(uint32_t sampleRate, uint32_t frameCount);
/**
* @brief Computes the number of bytes required for one interleaved audio frame - containing all the channels.
* @param format The audio format
* @param channelCount The number of channels
* @return Returns the number of bytes for one frame in bytes
*/
fpl_common_api uint32_t fplGetAudioFrameSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount);
/**
* @brief Computes the total number of bytes for the buffer and the given parameters
* @param format The audio format
* @param channelCount The number of channels
* @param frameCount The number of frames
* @return Returns the total number of bytes for the buffer
*/
fpl_common_api uint32_t fplGetAudioBufferSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount, const uint32_t frameCount);
/**
* @brief Converts a @ref fplAudioTargetFormat into a @ref fplAudioDeviceFormat structure
* @param inFormat The input format
* @param outFormat The output format
*/
fpl_common_api void fplConvertAudioTargetFormatToDeviceFormat(const fplAudioTargetFormat *inFormat, fplAudioDeviceFormat *outFormat);

/** @} */
#endif // FPL__ENABLE_AUDIO

// ----------------------------------------------------------------------------
/**
* @defgroup Localization Localization functions
* @brief This category contains functions for getting informations about current locale
* @{
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
* @brief Gets the user locale in the given target format
* @param targetFormat Target @ref fplLocaleFormat
* @param buffer Target string buffer for writing the locale into
* @param maxBufferLen The maximum length of the buffer
* @return Returns the number of required/written characters, excluding the null-terminator
*/
fpl_platform_api size_t fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/**
* @brief Gets the system locale in the given target format
* @param targetFormat Target @ref fplLocaleFormat
* @param buffer Target string buffer for writing the locale into
* @param maxBufferLen The maximum length of the buffer
* @return Returns the number of required/written characters, excluding the null-terminator
*/
fpl_platform_api size_t fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/**
* @brief Gets the input locale in the given target format
* @param targetFormat Target @ref fplLocaleFormat
* @param buffer Target string buffer for writing the locale into
* @param maxBufferLen The maximum length of the buffer
* @return Returns the number of required/written characters, excluding the null-terminator
*/
fpl_platform_api size_t fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen);

/** @} */

// Ignore any doxygen documentation from here
/// @cond FPL_INTERNALS

// ****************************************************************************
//
// > EXPORT
//
// Internal functions for static library
// Entry-Point forward declaration
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WINDOWS)
#	if defined(FPL_ENTRYPOINT)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_main int main(int argc, char **args);
#	endif
#endif // FPL_PLATFORM_WINDOWS

#endif // FPL_HEADER_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// FPL uses several implementation blocks to structure things in categories.
// Each block has its own ifdef definition to collapse it down if needed.
// But the baseline structure is the following:
//
// - Compiler settings (Disable warnings, etc.)
// - Platform Constants & Types (All required structs, Constants, Global variables, etc.)
// - Common implementations
// - Actual platform implementations (Win32, Linux)
// - Sub platform implementations (X11, POSIX, STD)
// - Backend implementations (Video: OpenGL/Software/Vulkan, Audio: DirectSound/Alsa)
// - Systems (Audio, Video, Window systems)
// - Core (Init & Release of the specific platform by selection)
//
// You can use the following strings to search for implementation blocks - including the > prefix:
//
// > COMPILER_CONFIG
// > PLATFORM_INCLUDES
//
// > INTERNAL_TOP
// > INTERNAL_LOGGING
//
// > PLATFORM_CONSTANTS
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
// > POSIX_SUBPLATFORM (Linux, Unix)
// > STD_STRINGS_SUBPLATFORM
// > STD_CONSOLE_SUBPLATFORM
// > X11_SUBPLATFORM
// > LINUX_PLATFORM
// > UNIX_PLATFORM
//
// > VIDEO_BACKENDS
// > VIDEO_BACKEND_OPENGL_WIN32
// > VIDEO_BACKEND_OPENGL_X11
// > VIDEO_BACKEND_SOFTWARE_WIN32
// > VIDEO_BACKEND_SOFTWARE_X11
// > VIDEO_BACKEND_VULKAN
//
// > AUDIO_BACKENDS
// > AUDIO_BACKEND_DIRECTSOUND
// > AUDIO_BACKEND_ALSA
//
// > SYSTEM_AUDIO_L1
// > SYSTEM_VIDEO_L1
// > SYSTEM_WINDOW
// > SYSTEM_AUDIO_L2
// > SYSTEM_VIDEO_L2 (Video backbuffer access and present of the frame)
// > SYSTEM_INIT (Init & Release of the Platform)
//
// ****************************************************************************
#if (defined(FPL_IMPLEMENTATION) || FPL_IS_IDE) && !defined(FPL__IMPLEMENTED)
#define FPL__IMPLEMENTED

// ############################################################################
//
// > COMPILER_CONFIG
//
// ############################################################################
#if !defined(FPL__COMPILER_CONFIG_DEFINED)
#define FPL__COMPILER_CONFIG_DEFINED

//
// Compiler warnings
//
#if defined(FPL_COMPILER_MSVC)

	// Start to overwrite warning settings (MSVC)
#	pragma warning( push )

	// Disable noexcept compiler warning for C++
#	pragma warning( disable : 4577 )
	// Disable "switch statement contains 'default' but no 'case' labels" compiler warning for C++
#	pragma warning( disable : 4065 )
	// Disable "conditional expression is constant" warning
#	pragma warning( disable : 4127 )
	// Disable "unreferenced formal parameter" warning
#	pragma warning( disable : 4100 )
	// Disable "nonstandard extension used: nameless struct/union" warning
#	pragma warning( disable : 4201 )
	// Disable "local variable is initialized but not referenced" warning
#	pragma warning( disable : 4189 )
	// Disable "nonstandard extension used: non-constant aggregate initializer" warning
#	pragma warning( disable : 4204 )

#elif defined(FPL_COMPILER_GCC)

	// Start to overwrite warning settings (GCC)
#	pragma GCC diagnostic push
// Disable warning -Wunused-variable
#	pragma GCC diagnostic ignored "-Wunused-variable"
// Disable warning -Wunused-function
#	pragma GCC diagnostic ignored "-Wunused-function"

#elif defined(FPL_COMPILER_CLANG)

	// Start to overwrite warning settings (Clang)
#	pragma clang diagnostic push

// Disable warning -Wunused-variable
#	pragma clang diagnostic ignored "-Wunused-variable"
// Disable warning -Wunused-function
#	pragma clang diagnostic ignored "-Wunused-function"

#endif // FPL_COMPILER

#endif // FPL__COMPILER_CONFIG_DEFINED

// ############################################################################
//
// > PLATFORM_INCLUDES
//
// ############################################################################
#if !defined(FPL__PLATFORM_INCLUDES_DEFINED)
#define FPL__PLATFORM_INCLUDES_DEFINED

#if !defined(FPL__HAS_PLATFORM_INCLUDES)
#	define FPL__HAS_PLATFORM_INCLUDES

#	if defined(FPL_PLATFORM_WINDOWS)
		// @NOTE(final): windef.h defines min/max macros in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#		if !defined(NOMINMAX)
#			define NOMINMAX
#		endif
		// @NOTE(final): For now we dont want any network, com or gdi stuff at all, maybe later who knows.
#		if !defined(WIN32_LEAN_AND_MEAN)
#			define WIN32_LEAN_AND_MEAN 1
#		endif
		// @STUPID(final): Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here"
struct IUnknown;
#		include <windows.h> // Win32 api
#		if _WIN32_WINNT < 0x0600
#		error "Windows Vista or higher required!"
#		endif
#	endif // FPL_PLATFORM_WINDOWS

#	if defined(FPL_SUBPLATFORM_POSIX)
#		include <pthread.h> // pthread_t, pthread_mutex_, pthread_cond_, pthread_barrier_
#		include <sched.h> // sched_param, sched_get_priority_max, SCHED_FIFO
#		include <semaphore.h> // sem_t
#		include <dirent.h> // DIR, dirent
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
#		include <X11/X.h> // Window
#		include <X11/Xlib.h> // Display
#		include <X11/Xutil.h> // XVisualInfo
#		include <X11/Xatom.h> // XA_CARDINAL
#	endif // FPL_SUBPLATFORM_X11

#endif // !FPL__HAS_PLATFORM_INCLUDES

//
// Test OS handles
//
#if defined(FPL_PLATFORM_WINDOWS)
fplStaticAssert(sizeof(fpl__Win32Handle) >= sizeof(HANDLE));
fplStaticAssert(sizeof(fpl__Win32LibraryHandle) >= sizeof(HANDLE));
fplStaticAssert(sizeof(fpl__Win32FileHandle) >= sizeof(HANDLE));
fplStaticAssert(sizeof(fpl__Win32ThreadHandle) >= sizeof(HANDLE));
fplStaticAssert(sizeof(fpl__Win32MutexHandle) >= sizeof(CRITICAL_SECTION));
fplStaticAssert(sizeof(fpl__Win32SemaphoreHandle) >= sizeof(HANDLE));
fplStaticAssert(sizeof(fpl__Win32ConditionVariable) >= sizeof(CONDITION_VARIABLE));
#elif defined(FPL_SUBPLATFORM_POSIX)
fplStaticAssert(sizeof(fpl__POSIXLibraryHandle) >= sizeof(void *));
fplStaticAssert(sizeof(fpl__POSIXFileHandle) >= sizeof(int));
fplStaticAssert(sizeof(fpl__POSIXDirHandle) >= sizeof(DIR *));
fplStaticAssert(sizeof(fpl__POSIXThreadHandle) >= sizeof(pthread_t));
fplStaticAssert(sizeof(fpl__POSIXMutexHandle) >= sizeof(pthread_mutex_t));
fplStaticAssert(sizeof(fpl__POSIXSemaphoreHandle) >= sizeof(sem_t));
fplStaticAssert(sizeof(fpl__POSIXConditionVariable) >= sizeof(pthread_cond_t));
#endif // FPL_PLATFORM_WINDOWS / FPL_SUBPLATFORM_POSIX
#if defined(FPL_PLATFORM_LINUX)
fplStaticAssert(sizeof(fpl__LinuxSignalHandle) >= sizeof(int));
#endif // FPL_PLATFORM_LINUX

//
// Compiler Includes
//
#if defined(FPL_COMPILER_MSVC)
#	include <intrin.h> // __cpuid, _Interlocked*
#elif defined(FPL_COMPILER_GCC) || defined(FPL_COMPILER_CLANG)
#	if defined(FPL_ARCH_X86) || defined(FPL_ARCH_X64)
#		include <cpuid.h> // __cpuid_count
#	endif // X86 or X64
#endif

// Only include C-Runtime functions when CRT is enabled
#if !defined(FPL_NO_CRT)
#	include <stdio.h> // stdin, stdout, stderr, fprintf, vfprintf, vsnprintf, getchar
#	include <stdlib.h> // wcstombs, mbstowcs, getenv
#	include <locale.h> // setlocale, struct lconv, localeconv
#endif

#endif // FPL__PLATFORM_INCLUDES_DEFINED

// ############################################################################
//
// > INTERNAL_TOP
//
// ############################################################################
#if !defined(FPL__INTERNAL_TOP_DEFINED)
#define FPL__INTERNAL_TOP_DEFINED

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
#define FPL__MODULE_VIDEO_VULKAN "Vulkan"
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
// Enum macros
//
#define FPL__ENUM_COUNT(first, last) ((last) - (first) + 1)
#define FPL__ENUM_VALUE_TO_ARRAY_INDEX(value, first, last) (((value) >= (first) && (value) <= (last)) ? ((value) - (first)) : 0)

//
// Internal memory
//
fpl_internal void *fpl__AllocateMemory(const fplMemoryAllocationSettings *allocSettings, const size_t size, const size_t alignment);
fpl_internal void fpl__ReleaseMemory(const fplMemoryAllocationSettings *allocSettings, void *ptr);

#endif // FPL__INTERNAL_TOP_DEFINED

// ############################################################################
//
// > INTERNAL_LOGGING
//
// ############################################################################
#if !defined(FPL__INTERNAL_LOGGING_DEFINED)
#define FPL__INTERNAL_LOGGING_DEFINED

#define FPL__MODULE_CONCAT(mod, format) "[" mod "] " format

#if defined(FPL__ENABLE_LOGGING)
fpl_globalvar fplLogSettings fpl__global__LogSettings = fplZeroInit;

#define FPL__LOGLEVEL_COUNT FPL__ENUM_COUNT(fplLogLevel_First, fplLogLevel_Last)
fpl_globalvar const char *fpl__LogLevelNameTable[] = {
	"All", // fplLogLevel_All (-1)
	"Critical", // fplLogLevel_Critical (0)
	"Error", // fplLogLevel_Error (1)
	"Warning", // fplLogLevel_Warning (2)
	"Info", // fplLogLevel_Info (3)
	"Verbose", // fplLogLevel_Verbose (4)
	"Debug", // fplLogLevel_Debug (5)
	"Trace", // fplLogLevel_Trace (6)
};
fplStaticAssert(fplArrayCount(fpl__LogLevelNameTable) == FPL__LOGLEVEL_COUNT);

fpl_internal const char *fpl__LogLevelToString(const fplLogLevel level) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(level, fplLogLevel_First, fplLogLevel_Last);
	const char *result = fpl__LogLevelNameTable[index];
	return(result);
}

fpl_internal void fpl__LogWrite(const char *funcName, const int lineNumber, const fplLogLevel level, const char *message) {
	fplLogSettings *settings = &fpl__global__LogSettings;
	if (!settings->isInitialized) {
#if defined(FPL_LOG_MULTIPLE_WRITERS)
		settings->criticalWriter.console.logToError = true;
		settings->criticalWriter.flags = fplLogWriterFlags_ErrorConsole | fplLogWriterFlags_DebugOut;
		settings->errorWriter = settings->criticalWriter;
		settings->warningWriter = settings->criticalWriter;
		settings->infoWriter.flags = fplLogWriterFlags_StandardConsole | fplLogWriterFlags_DebugOut;
		settings->verboseWriter = settings->infoWriter;
		settings->debugWriter.flags = fplLogWriterFlags_DebugOut;
#else
		settings->writers[0].flags = fplLogWriterFlags_StandardConsole | fplLogWriterFlags_DebugOut;
#endif
		settings->maxLevel = fplLogLevel_Warning;
		settings->isInitialized = true;
	}

	if ((settings->maxLevel == -1) || (level <= settings->maxLevel)) {
#if defined(FPL_LOG_MULTIPLE_WRITERS)
		fplAssert(level < fplArrayCount(settings->writers));
		const fplLogWriter *writer = &settings->writers[(int)level];
#else
		const fplLogWriter *writer = &settings->writers[0];
#endif
		const char *levelStr = fpl__LogLevelToString(level);

		if (writer->flags & fplLogWriterFlags_StandardConsole) {
			fplConsoleFormatOut("[%s:%d][%s] %s\n", funcName, lineNumber, levelStr, message);
		}
		if (writer->flags & fplLogWriterFlags_ErrorConsole) {
			fplConsoleFormatError("[%s:%d][%s] %s\n", funcName, lineNumber, levelStr, message);
		}
		if (writer->flags & fplLogWriterFlags_DebugOut) {
			fplDebugFormatOut("[%s:%d][%s] %s\n", funcName, lineNumber, levelStr, message);
		}
		if (writer->flags & fplLogWriterFlags_Custom && writer->custom.callback != fpl_null) {
			writer->custom.callback(funcName, lineNumber, level, message);
		}
	}
}
fpl_internal void fpl__LogWriteArgs(const char *funcName, const int lineNumber, const fplLogLevel level, const char *format, va_list argList) {
	va_list listCopy;
	va_copy(listCopy, argList);
	char buffer[FPL_MAX_BUFFER_LENGTH];
	size_t formattedLen = fplStringFormatArgs(buffer, fplArrayCount(buffer), format, listCopy);
	if (formattedLen > 0) {
		fpl__LogWrite(funcName, lineNumber, level, buffer);
	}
	va_end(listCopy);
}

fpl_internal void fpl__LogWriteVarArgs(const char *funcName, const int lineNumber, const fplLogLevel level, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	fpl__LogWriteArgs(funcName, lineNumber, level, format, argList);
	va_end(argList);
}

#	define FPL_LOG(lvl, mod, format, ...) fpl__LogWriteVarArgs(FPL_FUNCTION_NAME, __LINE__, lvl, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)
#	define FPL_LOG_CRITICAL(mod, format, ...) FPL_LOG(fplLogLevel_Critical, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_ERROR(mod, format, ...) FPL_LOG(fplLogLevel_Error, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_WARN(mod, format, ...) FPL_LOG(fplLogLevel_Warning, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_INFO(mod, format, ...) FPL_LOG(fplLogLevel_Info, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_VERBOSE(mod, format, ...) FPL_LOG(fplLogLevel_Verbose, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_DEBUG(mod, format, ...) FPL_LOG(fplLogLevel_Debug, mod, format, ## __VA_ARGS__)
#	define FPL_LOG_TRACE(mod, format, ...) FPL_LOG(fplLogLevel_Trace, mod, format, ## __VA_ARGS__)

#	define FPL__LOG_FUNCTION_N(mod, name) FPL_LOG(fplLogLevel_Debug, mod, "-> %s()", name)
#	define FPL_LOG_FUNCTION(mod) FPL__LOG_FUNCTION_N(mod, FPL_FUNCTION_NAME)

#else

#	define FPL_LOG(lvl, mod, format, ...)
#	define FPL_LOG_CRITICAL(mod, format, ...)
#	define FPL_LOG_ERROR(mod, format, ...)
#	define FPL_LOG_WARN(mod, format, ...)
#	define FPL_LOG_INFO(mod, format, ...)
#	define FPL_LOG_VERBOSE(mod, format, ...)
#	define FPL_LOG_DEBUG(mod, format, ...)
#	define FPL_LOG_FUNCTION(mod)

#endif

//
// Error handling
//

#define FPL__M_CRITICAL(funcName, line, mod, format, ...)  fpl__HandleError(funcName, line, fplLogLevel_Critical, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)
#define FPL__M_ERROR(funcName, line, mod, format, ...) fpl__HandleError(funcName, line, fplLogLevel_Error, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)
#define FPL__M_WARNING(funcName, line, mod, format, ...) fpl__HandleError(funcName, line, fplLogLevel_Warning, FPL__MODULE_CONCAT(mod, format), ## __VA_ARGS__)

#define FPL__CRITICAL(mod, format, ...)  FPL__M_CRITICAL(FPL_FUNCTION_NAME, __LINE__, mod, format, ## __VA_ARGS__)
#define FPL__ERROR(mod, format, ...) FPL__M_ERROR(FPL_FUNCTION_NAME, __LINE__, mod, format, ## __VA_ARGS__)
#define FPL__WARNING(mod, format, ...) FPL__M_WARNING(FPL_FUNCTION_NAME, __LINE__, mod, format, ## __VA_ARGS__)

#endif // FPL__INTERNAL_LOGGING_DEFINED

// ############################################################################
//
// > PLATFORM_CONSTANTS
//
// ############################################################################
#if !defined(FPL__PLATFORM_CONSTANTS_DEFINED)
#define FPL__PLATFORM_CONSTANTS_DEFINED

// One cacheline worth of padding
#define FPL__ARBITARY_PADDING 64
// Small padding to split sections in memory blocks
#define FPL__MEMORY_PADDING sizeof(uintptr_t)

fpl_globalvar struct fpl__PlatformAppState *fpl__global__AppState = fpl_null;

fpl_internal void fpl__HandleError(const char *funcName, const int lineNumber, const fplLogLevel level, const char *format, ...);
#endif // FPL__PLATFORM_CONSTANTS_DEFINED

// ############################################################################
//
// > UTILITY_FUNCTIONS
//
// ############################################################################
fpl_internal void *fpl__AllocateMemory(const fplMemoryAllocationSettings *allocSettings, const size_t size, const size_t alignment) {
	if (allocSettings->mode == fplMemoryAllocationMode_Custom) {
		if (allocSettings->allocateCallback != fpl_null && allocSettings->releaseCallback != fpl_null) {
			return allocSettings->allocateCallback(allocSettings->userData, size, alignment);
		}
	}
	return fplMemoryAlignedAllocate(size, alignment);
}

fpl_internal void fpl__ReleaseMemory(const fplMemoryAllocationSettings *allocSettings, void *ptr) {
	if (allocSettings->mode == fplMemoryAllocationMode_Custom) {
		if (allocSettings->allocateCallback != fpl_null && allocSettings->releaseCallback != fpl_null) {
			allocSettings->releaseCallback(allocSettings->userData, ptr);
			return;
		}
	}
	fplMemoryAlignedFree(ptr);
}

// Forward declarations of internal memory
fpl_internal void *fpl__AllocateDynamicMemory(const size_t size, const size_t alignment);
fpl_internal void fpl__ReleaseDynamicMemory(void *ptr);
fpl_internal void *fpl__AllocateTemporaryMemory(const size_t size, const size_t alignment);
fpl_internal void fpl__ReleaseTemporaryMemory(void *ptr);

fpl_internal uint32_t fpl__NextPowerOfTwo(const uint32_t input) {
	uint32_t x = input;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return(x);
}
fpl_internal uint32_t fpl__PrevPowerOfTwo(const uint32_t input) {
	uint32_t result = fpl__NextPowerOfTwo(input) >> 1;
	return(result);
}

fpl_internal uint32_t fpl__RoundToPowerOfTwo(const uint32_t input) {
	uint32_t prev = fpl__PrevPowerOfTwo(input);
	uint32_t next = fpl__NextPowerOfTwo(input);
	if ((next - input) < (input - prev)) {
		return prev;
	} else {
		return next;
	}
}

fpl_internal bool fpl__AddLineWhenAnyMatches(const char *line, const char **wildcards, const size_t maxWildcardCount, const size_t maxLineSize, const size_t maxLineCount, char **outLines, size_t *outCount) {
	for (size_t i = 0; i < maxWildcardCount; ++i) {
		const char *wildcard = wildcards[i];
		if (fplIsStringMatchWildcard(line, wildcard)) {
			size_t index = *outCount;
			char *target = outLines[index];
			fplCopyString(line, target, maxLineSize);
			*outCount = index + 1;
			break;
		}
	}
	bool result = *outCount < maxLineCount;
	return(result);
}

fpl_internal size_t fpl__ParseTextFile(const char *filePath, const char **wildcards, const size_t maxWildcardCount, const size_t maxLineSize, const size_t maxLineCount, char **outLines) {
	if (filePath == fpl_null || wildcards == fpl_null || maxWildcardCount == 0 || maxLineSize == 0 || maxLineCount == 0 || outLines == fpl_null) {
		return(0);
	}
	// @NOTE(final): Forced Zero-Terminator is not nessecary here, but we do it here to debug it better
	// This function supports maxLineSize < fplArrayCount(buffer)
	// We allocate the line buffer on the stack because we do not know how large the line will be on compile time
	size_t result = 0;
	fplFileHandle fileHandle = fplZeroInit;
	if (fplFileOpenBinary(filePath, &fileHandle)) {
		char *line = (char *)fpl__AllocateTemporaryMemory(maxLineSize, 8);
		char buffer[FPL_MAX_BUFFER_LENGTH];
		const size_t maxBufferSize = fplArrayCount(buffer) - 1;
		size_t bytesRead = 0;
		size_t posLineBytes = 0;
		bool done = false;
		while (!done && ((bytesRead = fplFileReadBlock(&fileHandle, maxBufferSize, &buffer[0], maxBufferSize)) > 0)) {
			buffer[bytesRead] = 0;
			char *start = &buffer[0];
			char *p = start;
			size_t readPos = 0;
			size_t lineSizeToRead = 0;
			while (readPos < bytesRead) {
				if (*p == '\n') {
					size_t remainingLineBytes = maxLineSize - posLineBytes;
					char *lineTargetP = line + posLineBytes;
					if (lineSizeToRead < remainingLineBytes) {
						fplCopyStringLen(start, lineSizeToRead, lineTargetP, remainingLineBytes);
					} else {
						fplCopyStringLen(start, remainingLineBytes - 1, lineTargetP, remainingLineBytes);
					}
					if (!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
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
			if (done) {
				break;
			}
			if (lineSizeToRead > 0) {
				size_t remainingLineBytes = maxLineSize - posLineBytes;
				char *lineTargetP = line + posLineBytes;
				if (lineSizeToRead < remainingLineBytes) {
					fplCopyStringLen(start, lineSizeToRead, lineTargetP, remainingLineBytes);
					posLineBytes += lineSizeToRead;
					if (bytesRead <= maxBufferSize) {
						if (!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
							done = true;
						}
					}
				} else {
					fplCopyStringLen(start, remainingLineBytes - 1, lineTargetP, remainingLineBytes);
					line[0] = 0;
					lineSizeToRead = 0;
					posLineBytes = 0;
					if (!fpl__AddLineWhenAnyMatches(line, wildcards, maxWildcardCount, maxLineSize, maxLineCount, outLines, &result)) {
						done = true;
					}
				}
			}
		}
		fpl__ReleaseTemporaryMemory(line);
		fplFileClose(&fileHandle);
	}
	return(result);
}

fpl_internal void fpl__ParseVersionString(const char *versionStr, fplVersionInfo *versionInfo) {
	fplCopyString(versionStr, versionInfo->fullName, fplArrayCount(versionInfo->fullName));
	if (versionStr != fpl_null) {
		const char *p = versionStr;
		for (int i = 0; i < 4; ++i) {
			const char *digitStart = p;
			while (*p >= '0' && *p <= '9') {
				++p;
			}
			size_t len = p - digitStart;
			if (len <= fplArrayCount(versionInfo->values[i])) {
				fplCopyStringLen(digitStart, len, versionInfo->values[i], fplArrayCount(versionInfo->values[i]));
			} else {
				versionInfo->values[i][0] = 0;
			}
			if (*p != '.' && *p != '-') break;
			++p;
		}
	}
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
#if defined(FPL_PLATFORM_WINDOWS)
#	include <windowsx.h>	// Macros for window messages
#	include <shlobj.h>		// SHGetFolderPath
#	include <xinput.h>		// XInputGetState
#	include <shellapi.h>	// HDROP

#	if defined(FPL_IS_CPP)
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(a, b)
#	else
#		define fpl__Win32IsEqualGuid(a, b) InlineIsEqualGUID(&a, &b)
#	endif

fpl_internal const char *fpl__Win32FormatGuidString(char *buffer, const size_t maxBufferLen, const GUID *guid) {
	fplStringFormat(buffer, maxBufferLen, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	return(buffer);
}

// Little macro to not write 5 lines of code all the time
#define FPL__WIN32_LOAD_LIBRARY_BREAK(mod, target, libName) \
	(target) = LoadLibraryA(libName); \
	if((target) == fpl_null) { \
		FPL__WARNING(mod, "Failed loading library '%s'", (libName)); \
		break; \
	}
#define FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK(mod, libHandle, libName, target, type, name) \
	(target)->name = (type *)GetProcAddress(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING(mod, "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		break; \
	}
#if !defined(FPL_NO_RUNTIME_LINKING)
#	define FPL__WIN32_LOAD_LIBRARY FPL__WIN32_LOAD_LIBRARY_BREAK
#	define FPL__WIN32_GET_FUNCTION_ADDRESS FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK
#else
#	define FPL__WIN32_LOAD_LIBRARY(mod, target, libName)
#	define FPL__WIN32_GET_FUNCTION_ADDRESS(mod, libHandle, libName, target, type, name) \
		(target)->name = name
#endif

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
	fpl__win32_func_XInputGetState *XInputGetState;
	fpl__win32_func_XInputGetCapabilities *XInputGetCapabilities;
} fpl__Win32XInputApi;

fpl_internal void fpl__Win32UnloadXInputApi(fpl__Win32XInputApi *xinputApi) {
	fplAssert(xinputApi != fpl_null);
	if (xinputApi->xinputLibrary) {
		FPL_LOG_DEBUG("XInput", "Unload XInput Library");
		FreeLibrary(xinputApi->xinputLibrary);
		xinputApi->xinputLibrary = fpl_null;
		xinputApi->XInputGetState = fpl__Win32XInputGetStateStub;
		xinputApi->XInputGetCapabilities = fpl__Win32XInputGetCapabilitiesStub;
	}
}

fpl_internal void fpl__Win32LoadXInputApi(fpl__Win32XInputApi *xinputApi) {
	fplAssert(xinputApi != fpl_null);
	const char *xinputFileNames[] = {
		"xinput1_4.dll",
		"xinput1_3.dll",
		"xinput9_1_0.dll",
	};
	bool result = false;
	for (uint32_t index = 0; index < fplArrayCount(xinputFileNames); ++index) {
		const char *libName = xinputFileNames[index];
		fplClearStruct(xinputApi);
		do {
			HMODULE libHandle = fpl_null;
			FPL__WIN32_LOAD_LIBRARY_BREAK(FPL__MODULE_XINPUT, libHandle, libName);
			xinputApi->xinputLibrary = libHandle;
			FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_XINPUT, libHandle, libName, xinputApi, fpl__win32_func_XInputGetState, XInputGetState);
			FPL__WIN32_GET_FUNCTION_ADDRESS_BREAK(FPL__MODULE_XINPUT, libHandle, libName, xinputApi, fpl__win32_func_XInputGetCapabilities, XInputGetCapabilities);
			result = true;
		} while (0);
		if (result) {
			break;
		}
		fpl__Win32UnloadXInputApi(xinputApi);
	}

	if (!result) {
		xinputApi->XInputGetState = fpl__Win32XInputGetStateStub;
		xinputApi->XInputGetCapabilities = fpl__Win32XInputGetCapabilitiesStub;
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
#define FPL__FUNC_WIN32_CreateSolidBrush(name) HBRUSH WINAPI name(COLORREF color)
typedef FPL__FUNC_WIN32_CreateSolidBrush(fpl__win32_func_CreateSolidBrush);

// ShellAPI
#define FPL__FUNC_WIN32_SHGetFolderPathW(name) HRESULT WINAPI name(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
typedef FPL__FUNC_WIN32_SHGetFolderPathW(fpl__win32_func_SHGetFolderPathW);
#define FPL__FUNC_WIN32_DragQueryFileW(name) UINT WINAPI name(HDROP hDrop, UINT iFile, LPWSTR lpszFile, UINT cch)
typedef FPL__FUNC_WIN32_DragQueryFileW(fpl__win32_func_DragQueryFileW);
#define FPL__FUNC_WIN32_DragAcceptFiles(name) void WINAPI name(HWND hWnd, BOOL fAccept)
typedef FPL__FUNC_WIN32_DragAcceptFiles(fpl__win32_func_DragAcceptFiles);

// User32
#define FPL__FUNC_WIN32_RegisterClassExW(name) ATOM WINAPI name(CONST WNDCLASSEXW *)
typedef FPL__FUNC_WIN32_RegisterClassExW(fpl__win32_func_RegisterClassExW);
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
#define FPL__FUNC_WIN32_DispatchMessageW(name) LRESULT WINAPI name(CONST MSG *lpMsg)
typedef FPL__FUNC_WIN32_DispatchMessageW(fpl__win32_func_DispatchMessageW);
#define FPL__FUNC_WIN32_PeekMessageW(name) BOOL WINAPI name(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
typedef FPL__FUNC_WIN32_PeekMessageW(fpl__win32_func_PeekMessageW);
#define FPL__FUNC_WIN32_DefWindowProcW(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_DefWindowProcW(fpl__win32_func_DefWindowProcW);
#define FPL__FUNC_WIN32_CreateWindowExW(name) HWND WINAPI name(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
typedef FPL__FUNC_WIN32_CreateWindowExW(fpl__win32_func_CreateWindowExW);
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
#define FPL__FUNC_WIN32_SetWindowTextW(name) BOOL WINAPI name(HWND hWnd, LPCWSTR lpString)
typedef FPL__FUNC_WIN32_SetWindowTextW(fpl__win32_func_SetWindowTextW);
#define FPL__FUNC_WIN32_GetWindowTextW(name) int WINAPI name(HWND hWnd, LPWSTR lpString, int nMaxCount)
typedef FPL__FUNC_WIN32_GetWindowTextW(fpl__win32_func_GetWindowTextW);
#define FPL__FUNC_WIN32_SetWindowLongW(name) LONG WINAPI name(HWND hWnd, int nIndex, LONG dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongW(fpl__win32_func_SetWindowLongW);
#define FPL__FUNC_WIN32_GetWindowLongW(name) LONG WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongW(fpl__win32_func_GetWindowLongW);

#if defined(FPL_ARCH_X64)
#define FPL__FUNC_WIN32_SetWindowLongPtrW(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
typedef FPL__FUNC_WIN32_SetWindowLongPtrW(fpl__win32_func_SetWindowLongPtrW);
#define FPL__FUNC_WIN32_GetWindowLongPtrW(name) LONG_PTR WINAPI name(HWND hWnd, int nIndex)
typedef FPL__FUNC_WIN32_GetWindowLongPtrW(fpl__win32_func_GetWindowLongPtrW);
#endif

#define FPL__FUNC_WIN32_ReleaseDC(name) int WINAPI name(HWND hWnd, HDC hDC)
typedef FPL__FUNC_WIN32_ReleaseDC(fpl__win32_func_ReleaseDC);
#define FPL__FUNC_WIN32_GetDC(name) HDC WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_GetDC(fpl__win32_func_GetDC);
#define FPL__FUNC_WIN32_ChangeDisplaySettingsW(name) LONG WINAPI name(DEVMODEW* lpDevMode, DWORD dwFlags)
typedef FPL__FUNC_WIN32_ChangeDisplaySettingsW(fpl__win32_func_ChangeDisplaySettingsW);
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
#define FPL__FUNC_WIN32_SendMessageW(name) LRESULT WINAPI name(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
typedef FPL__FUNC_WIN32_SendMessageW(fpl__win32_func_SendMessageW);
#define FPL__FUNC_WIN32_GetMonitorInfoW(name) BOOL WINAPI name(HMONITOR hMonitor, LPMONITORINFO lpmi)
typedef FPL__FUNC_WIN32_GetMonitorInfoW(fpl__win32_func_GetMonitorInfoW);
#define FPL__FUNC_WIN32_EnumDisplayMonitors(name) BOOL WINAPI name(HDC hdc, LPCRECT lprcClip, MONITORENUMPROC lpfnEnum,LPARAM dwData)
typedef FPL__FUNC_WIN32_EnumDisplayMonitors(fpl__win32_func_EnumDisplayMonitors);
#define FPL__FUNC_WIN32_MonitorFromRect(name) HMONITOR WINAPI name(LPCRECT lprc, DWORD dwFlags)
typedef FPL__FUNC_WIN32_MonitorFromRect(fpl__win32_func_MonitorFromRect);
#define FPL__FUNC_WIN32_MonitorFromPoint(name) HMONITOR WINAPI name(POINT pt, DWORD dwFlags)
typedef FPL__FUNC_WIN32_MonitorFromPoint(fpl__win32_func_MonitorFromPoint);
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
#define FPL__FUNC_WIN32_SetCapture(name) HWND WINAPI name(HWND hWnd)
typedef FPL__FUNC_WIN32_SetCapture(fpl__win32_func_SetCapture);
#define FPL__FUNC_WIN32_ReleaseCapture(name) BOOL WINAPI name(VOID)
typedef FPL__FUNC_WIN32_ReleaseCapture(fpl__win32_func_ReleaseCapture);
#define FPL__FUNC_WIN32_ScreenToClient(name) BOOL WINAPI name(HWND hWnd, LPPOINT lpPoint)
typedef FPL__FUNC_WIN32_ScreenToClient(fpl__win32_func_ScreenToClient);
#define FPL__FUNC_WIN32_BeginPaint(name) HDC WINAPI name(_In_ HWND hWnd, _Out_ LPPAINTSTRUCT lpPaint)
typedef FPL__FUNC_WIN32_BeginPaint(fpl__win32_func_BeginPaint);
#define FPL__FUNC_WIN32_EndPaint(name) BOOL WINAPI name(_In_ HWND hWnd, _In_ CONST PAINTSTRUCT *lpPaint)
typedef FPL__FUNC_WIN32_EndPaint(fpl__win32_func_EndPaint);
#define FPL__FUNC_WIN32_SetForegroundWindow(name) BOOL WINAPI name(_In_ HWND hWnd)
typedef FPL__FUNC_WIN32_SetForegroundWindow(fpl__win32_func_SetForegroundWindow);
#define FPL__FUNC_WIN32_SetFocus(name) HWND WINAPI name(_In_opt_ HWND hWnd)
typedef FPL__FUNC_WIN32_SetFocus(fpl__win32_func_SetFocus);
#define FPL__FUNC_WIN32_SetTimer(name) UINT_PTR WINAPI name(_In_opt_ HWND hWnd, _In_ UINT_PTR nIDEvent, _In_ UINT uElapse, _In_opt_ TIMERPROC lpTimerFunc)
typedef FPL__FUNC_WIN32_SetTimer(fpl__win32_func_SetTimer);
#define FPL__FUNC_WIN32_GetSysColorBrush(name) HBRUSH WINAPI name(_In_ int nIndex)
typedef FPL__FUNC_WIN32_GetSysColorBrush(fpl__win32_func_GetSysColorBrush);
#define FPL__FUNC_WIN32_GetSysColorBrush(name) HBRUSH WINAPI name(_In_ int nIndex)
typedef FPL__FUNC_WIN32_GetSysColorBrush(fpl__win32_func_GetSysColorBrush);

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
	fpl__win32_func_CreateSolidBrush *CreateSolidBrush;
} fpl__Win32GdiApi;

typedef struct fpl__Win32ShellApi {
	HMODULE shellLibrary;
	fpl__win32_func_SHGetFolderPathW *SHGetFolderPathW;
	fpl__win32_func_DragQueryFileW *DragQueryFileW;
	fpl__win32_func_DragAcceptFiles *DragAcceptFiles;
} fpl__Win32ShellApi;

typedef struct fpl__Win32UserApi {
	HMODULE userLibrary;
	fpl__win32_func_RegisterClassExW *RegisterClassExW;
	fpl__win32_func_UnregisterClassW *UnregisterClassW;
	fpl__win32_func_ShowWindow *ShowWindow;
	fpl__win32_func_DestroyWindow *DestroyWindow;
	fpl__win32_func_UpdateWindow *UpdateWindow;
	fpl__win32_func_TranslateMessage *TranslateMessage;
	fpl__win32_func_DispatchMessageW *DispatchMessageW;
	fpl__win32_func_PeekMessageW *PeekMessageW;
	fpl__win32_func_DefWindowProcW *DefWindowProcW;
	fpl__win32_func_CreateWindowExW *CreateWindowExW;
	fpl__win32_func_SetWindowPos *SetWindowPos;
	fpl__win32_func_GetWindowPlacement *GetWindowPlacement;
	fpl__win32_func_SetWindowPlacement *SetWindowPlacement;
	fpl__win32_func_GetClientRect *GetClientRect;
	fpl__win32_func_GetWindowRect *GetWindowRect;
	fpl__win32_func_AdjustWindowRect *AdjustWindowRect;
	fpl__win32_func_GetAsyncKeyState *GetAsyncKeyState;
	fpl__win32_func_MapVirtualKeyW *MapVirtualKeyW;
	fpl__win32_func_SetCursor *SetCursor;
	fpl__win32_func_GetCursor *GetCursor;
	fpl__win32_func_LoadCursorA *LoadCursorA;
	fpl__win32_func_LoadCursorW *LoadCursorW;
	fpl__win32_func_LoadIconA *LoadIconA;
	fpl__win32_func_LoadIconW *LoadIconW;
	fpl__win32_func_SetWindowTextW *SetWindowTextW;
	fpl__win32_func_GetWindowTextW *GetWindowTextW;
	fpl__win32_func_SetWindowLongW *SetWindowLongW;
	fpl__win32_func_GetWindowLongW *GetWindowLongW;
#if defined(FPL_ARCH_X64)
	fpl__win32_func_SetWindowLongPtrW *SetWindowLongPtrW;
	fpl__win32_func_GetWindowLongPtrW *GetWindowLongPtrW;
#endif
	fpl__win32_func_ReleaseDC *ReleaseDC;
	fpl__win32_func_GetDC *GetDC;
	fpl__win32_func_ChangeDisplaySettingsW *ChangeDisplaySettingsW;
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
	fpl__win32_func_SendMessageW *SendMessageW;
	fpl__win32_func_GetMonitorInfoW *GetMonitorInfoW;
	fpl__win32_func_EnumDisplayMonitors *EnumDisplayMonitors;
	fpl__win32_func_MonitorFromRect *MonitorFromRect;
	fpl__win32_func_MonitorFromPoint *MonitorFromPoint;
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
	fpl__win32_func_SetCapture *SetCapture;
	fpl__win32_func_ReleaseCapture *ReleaseCapture;
	fpl__win32_func_ScreenToClient *ScreenToClient;
	fpl__win32_func_BeginPaint *BeginPaint;
	fpl__win32_func_EndPaint *EndPaint;
	fpl__win32_func_SetForegroundWindow *SetForegroundWindow;
	fpl__win32_func_SetFocus *SetFocus;
	fpl__win32_func_SetTimer *SetTimer;
	fpl__win32_func_GetSysColorBrush *GetSysColorBrush;
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
	fplAssert(wapi != fpl_null);
	if (wapi->ole.oleLibrary != fpl_null) {
		FreeLibrary(wapi->ole.oleLibrary);
	}
	fplClearStruct(&wapi->ole);
	if (wapi->gdi.gdiLibrary != fpl_null) {
		FreeLibrary(wapi->gdi.gdiLibrary);
	}
	fplClearStruct(&wapi->gdi);
	if (wapi->user.userLibrary != fpl_null) {
		FreeLibrary(wapi->user.userLibrary);
	}
	fplClearStruct(&wapi->user);
	if (wapi->shell.shellLibrary != fpl_null) {
		FreeLibrary(wapi->shell.shellLibrary);
	}
	fplClearStruct(&wapi->shell);
	wapi->isValid = false;
}

fpl_internal bool fpl__Win32LoadApi(fpl__Win32Api *wapi) {
	fplAssert(wapi != fpl_null);
	bool result = false;
	fplClearStruct(wapi);
	do {
		// Shell32
		const char *shellLibraryName = "shell32.dll";
		HMODULE shellLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_WIN32, shellLibrary, shellLibraryName);
		wapi->shell.shellLibrary = shellLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, shellLibrary, shellLibraryName, &wapi->shell, fpl__win32_func_SHGetFolderPathW, SHGetFolderPathW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, shellLibrary, shellLibraryName, &wapi->shell, fpl__win32_func_DragQueryFileW, DragQueryFileW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, shellLibrary, shellLibraryName, &wapi->shell, fpl__win32_func_DragAcceptFiles, DragAcceptFiles);

		// User32
		const char *userLibraryName = "user32.dll";
		HMODULE userLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_WIN32, userLibrary, userLibraryName);
		wapi->user.userLibrary = userLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_RegisterClassExW, RegisterClassExW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_UnregisterClassW, UnregisterClassW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ShowWindow, ShowWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_DestroyWindow, DestroyWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_UpdateWindow, UpdateWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_TranslateMessage, TranslateMessage);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_DispatchMessageW, DispatchMessageW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_PeekMessageW, PeekMessageW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_DefWindowProcW, DefWindowProcW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_CreateWindowExW, CreateWindowExW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetWindowPos, SetWindowPos);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetWindowPlacement, GetWindowPlacement);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetWindowPlacement, SetWindowPlacement);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetClientRect, GetClientRect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetWindowRect, GetWindowRect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_AdjustWindowRect, AdjustWindowRect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetAsyncKeyState, GetAsyncKeyState);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_MapVirtualKeyW, MapVirtualKeyW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetCursor, SetCursor);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetCursor, GetCursor);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_LoadCursorA, LoadCursorA);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_LoadCursorW, LoadCursorW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetCursorPos, GetCursorPos);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_WindowFromPoint, WindowFromPoint);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_LoadIconA, LoadIconA);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_LoadIconW, LoadIconW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetWindowTextW, SetWindowTextW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetWindowLongW, SetWindowLongW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetWindowLongW, GetWindowLongW);

#	if defined(FPL_ARCH_X64)
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetWindowLongPtrW, SetWindowLongPtrW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetWindowLongPtrW, GetWindowLongPtrW);
#	endif

		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ReleaseDC, ReleaseDC);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetDC, GetDC);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ChangeDisplaySettingsW, ChangeDisplaySettingsW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_EnumDisplaySettingsW, EnumDisplaySettingsW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_IsClipboardFormatAvailable, IsClipboardFormatAvailable);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_OpenClipboard, OpenClipboard);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_CloseClipboard, CloseClipboard);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_EmptyClipboard, EmptyClipboard);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetClipboardData, SetClipboardData);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetClipboardData, GetClipboardData);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetDesktopWindow, GetDesktopWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetForegroundWindow, GetForegroundWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_IsZoomed, IsZoomed);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_IsIconic, IsIconic);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SendMessageW, SendMessageW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetMonitorInfoW, GetMonitorInfoW);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_EnumDisplayMonitors, EnumDisplayMonitors);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_MonitorFromRect, MonitorFromRect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_MonitorFromPoint, MonitorFromPoint);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_MonitorFromWindow, MonitorFromWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ClientToScreen, ClientToScreen);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_PtInRect, PtInRect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_RegisterRawInputDevices, RegisterRawInputDevices);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ClipCursor, ClipCursor);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_PostQuitMessage, PostQuitMessage);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_CreateIconIndirect, CreateIconIndirect);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetKeyboardLayout, GetKeyboardLayout);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetKeyState, GetKeyState);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetCapture, SetCapture);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ReleaseCapture, ReleaseCapture);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_ScreenToClient, ScreenToClient);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_BeginPaint, BeginPaint);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_EndPaint, EndPaint);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetForegroundWindow, SetForegroundWindow);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetFocus, SetFocus);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_SetTimer, SetTimer);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, userLibrary, userLibraryName, &wapi->user, fpl__win32_func_GetSysColorBrush, GetSysColorBrush);

		// GDI32
		const char *gdiLibraryName = "gdi32.dll";
		HMODULE gdiLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName);
		wapi->gdi.gdiLibrary = gdiLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_ChoosePixelFormat, ChoosePixelFormat);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_SetPixelFormat, SetPixelFormat);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_DescribePixelFormat, DescribePixelFormat);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_StretchDIBits, StretchDIBits);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_DeleteObject, DeleteObject);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_SwapBuffers, SwapBuffers);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_GetDeviceCaps, GetDeviceCaps);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_CreateDIBSection, CreateDIBSection);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_CreateBitmap, CreateBitmap);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, gdiLibrary, gdiLibraryName, &wapi->gdi, fpl__win32_func_CreateSolidBrush, CreateSolidBrush);

		// OLE32
		const char *oleLibraryName = "ole32.dll";
		HMODULE oleLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_WIN32, oleLibrary, oleLibraryName);
		wapi->ole.oleLibrary = oleLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, oleLibrary, oleLibraryName, &wapi->ole, fpl__win32_func_CoInitializeEx, CoInitializeEx);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, oleLibrary, oleLibraryName, &wapi->ole, fpl__win32_func_CoUninitialize, CoUninitialize);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, oleLibrary, oleLibraryName, &wapi->ole, fpl__win32_func_CoCreateInstance, CoCreateInstance);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, oleLibrary, oleLibraryName, &wapi->ole, fpl__win32_func_CoTaskMemFree, CoTaskMemFree);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_WIN32, oleLibrary, oleLibraryName, &wapi->ole, fpl__win32_func_PropVariantClear, PropVariantClear);

		result = true;
	} while (0);
	if (!result) {
		fpl__Win32UnloadApi(wapi);
	}
	wapi->isValid = result;
	return(result);
}

// Win32 unicode dependend stuff
#define FPL__WIN32_CLASSNAME L"FPLWindowClassW"
#define FPL__WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
#define FPL__WIN32_UNNAMED_CONSOLE L"Unnamed FPL Unicode Console"
#if defined(FPL_ARCH_X64)
#	define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongPtrW
#else
#	define fpl__win32_SetWindowLongPtr fpl__global__AppState->win32.winApi.user.SetWindowLongW
#endif
#define fpl__win32_SetWindowLong fpl__global__AppState->win32.winApi.user.SetWindowLongW
#define fpl__win32_GetWindowLong fpl__global__AppState->win32.winApi.user.GetWindowLongW
#if UNICODE
#	define fpl__win32_LoadIcon fpl__global__AppState->win32.winApi.user.LoadIconW
#	define fpl__win32_LoadCursor fpl__global__AppState->win32.winApi.user.LoadCursorW
#else
#	define fpl__win32_LoadIcon fpl__global__AppState->win32.winApi.user.LoadIconA
#	define fpl__win32_LoadCursor fpl__global__AppState->win32.winApi.user.LoadCursorA
#endif

typedef char fpl__GameControllerName[FPL_MAX_NAME_LENGTH];

typedef struct fpl__Win32XInputState {
	fpl__GameControllerName deviceNames[XUSER_MAX_COUNT];
	fpl_b32 isConnected[XUSER_MAX_COUNT];
	fpl__Win32XInputApi xinputApi;
	LARGE_INTEGER lastDeviceSearchTime;
} fpl__Win32XInputState;

typedef struct fpl__Win32InitState {
	HINSTANCE appInstance;
	LARGE_INTEGER qpf;
} fpl__Win32InitState;

typedef struct fpl__Win32AppState {
	fpl__Win32XInputState xinput;
	fpl__Win32Api winApi;
} fpl__Win32AppState;

#if defined(FPL__ENABLE_WINDOW)
typedef struct fpl__Win32LastWindowInfo {
	WINDOWPLACEMENT placement;
	DWORD style;
	DWORD exStyle;
	fpl_b32 isMaximized;
	fpl_b32 isMinimized;
	fpl_b32 wasResolutionChanged;
} fpl__Win32LastWindowInfo;

typedef struct fpl__Win32WindowState {
	wchar_t windowClass[256];
	fpl__Win32LastWindowInfo lastFullscreenInfo;
	void *mainFiber;
	void *messageFiber;
	HWND windowHandle;
	HDC deviceContext;
	HBRUSH backgroundBrush;
	HCURSOR defaultCursor;
	int pixelFormat;
	fpl_b32 isCursorActive;
	fpl_b32 isFrameInteraction;
} fpl__Win32WindowState;
#endif // FPL__ENABLE_WINDOW

#endif // FPL_PLATFORM_WINDOWS

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
#	include <sys/utsname.h> // uname
#	include <signal.h> // pthread_kill
#	include <time.h> // clock_gettime, nanosleep
#	include <dlfcn.h> // dlopen, dlclose
#	include <fcntl.h> // open
#	include <unistd.h> // read, write, close, access, rmdir, getpid, sysconf, geteuid
#	include <ctype.h> // isspace
#	include <pwd.h> // getpwuid

// @TODO(final): Detect the case of (Older POSIX versions where st_atim != st_atime)
#if !defined(FPL_PLATFORM_ANDROID)
# define st_atime st_atim.tv_sec
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
#endif

#if defined(FPL_PLATFORM_LINUX)
#	define fpl__lseek64 lseek64
#	define fpl__off64_t off64_t
#else
#	define fpl__lseek64 lseek
#	define fpl__off64_t off_t
#endif

// Little macros for loading a library and getting proc address for POSIX
#define FPL__POSIX_LOAD_LIBRARY_BREAK(mod, target, libName) \
	(target) = dlopen(libName, FPL__POSIX_DL_LOADTYPE); \
	if((target) == fpl_null) { \
		FPL__WARNING(mod, "Failed loading library '%s'", (libName)); \
		break; \
	}

#define FPL__POSIX_GET_FUNCTION_ADDRESS_OPTIONAL(mod, libHandle, libName, target, type, name) \
	(target)->name = (type *)dlsym(libHandle, #name)

#define FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK(mod, libHandle, libName, target, type, name) \
	(target)->name = (type *)dlsym(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING(mod, "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		break; \
	}
#if !defined(FPL_NO_RUNTIME_LINKING)
#	define FPL__POSIX_LOAD_LIBRARY FPL__POSIX_LOAD_LIBRARY_BREAK
#	define FPL__POSIX_GET_FUNCTION_ADDRESS FPL__POSIX_GET_FUNCTION_ADDRESS_BREAK
#else
#	define FPL__POSIX_LOAD_LIBRARY(mod, target, libName)
#	define FPL__POSIX_GET_FUNCTION_ADDRESS_OPTIONAL(mod, libHandle, libName, target, type, name) \
		(target)->name = name
#	define FPL__POSIX_GET_FUNCTION_ADDRESS(mod, libHandle, libName, target, type, name) \
		(target)->name = name
#endif

#define FPL__FUNC_PTHREAD_pthread_self(name) pthread_t name(void)
typedef FPL__FUNC_PTHREAD_pthread_self(fpl__pthread_func_pthread_self);
#define FPL__FUNC_PTHREAD_pthread_setschedparam(name) int name(pthread_t thread, int policy, const struct sched_param *param)
typedef FPL__FUNC_PTHREAD_pthread_setschedparam(fpl__pthread_func_pthread_setschedparam);
#define FPL__FUNC_PTHREAD_pthread_getschedparam(name) int name(pthread_t thread, int *policy, struct sched_param *param)
typedef FPL__FUNC_PTHREAD_pthread_getschedparam(fpl__pthread_func_pthread_getschedparam);
#define FPL__FUNC_PTHREAD_pthread_setschedprio(name) int name(pthread_t thread, int prio)
typedef FPL__FUNC_PTHREAD_pthread_setschedprio(fpl__pthread_func_pthread_setschedprio);

#define FPL__FUNC_PTHREAD_pthread_attr_init(name) int name(pthread_attr_t *attr)
typedef FPL__FUNC_PTHREAD_pthread_attr_init(fpl__pthread_func_pthread_attr_init);
#define FPL__FUNC_PTHREAD_pthread_attr_getschedparam(name) int name(const pthread_attr_t *__restrict__ attr, struct sched_param *__restrict__ param)
typedef FPL__FUNC_PTHREAD_pthread_attr_getschedparam(fpl__pthread_func_pthread_attr_getschedparam);
#define FPL__FUNC_PTHREAD_pthread_attr_setschedparam(name) int name(pthread_attr_t *__restrict__ attr, const struct sched_param *__restrict__ param)
typedef FPL__FUNC_PTHREAD_pthread_attr_setschedparam(fpl__pthread_func_pthread_attr_setschedparam);
#define FPL__FUNC_PTHREAD_pthread_attr_setstacksize(name) int name(pthread_attr_t *attr, size_t stacksize)
typedef FPL__FUNC_PTHREAD_pthread_attr_setstacksize(fpl__pthread_func_pthread_attr_setstacksize);
#define FPL__FUNC_PTHREAD_pthread_attr_setdetachstate(name) int name(pthread_attr_t *attr, int detachstate);
typedef FPL__FUNC_PTHREAD_pthread_attr_setdetachstate(fpl__pthread_func_pthread_attr_setdetachstate);
#define FPL__FUNC_PTHREAD_pthread_attr_setschedpolicy(name) int name(pthread_attr_t *__attr, int __policy)
typedef FPL__FUNC_PTHREAD_pthread_attr_setschedpolicy(fpl__pthread_func_pthread_attr_setschedpolicy);

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

	// pthread_t
	fpl__pthread_func_pthread_self *pthread_self;
	fpl__pthread_func_pthread_setschedparam *pthread_setschedparam;
	fpl__pthread_func_pthread_getschedparam *pthread_getschedparam;
	fpl__pthread_func_pthread_setschedprio *pthread_setschedprio;

	fpl__pthread_func_pthread_create *pthread_create;
	fpl__pthread_func_pthread_kill *pthread_kill;
	fpl__pthread_func_pthread_join *pthread_join;
	fpl__pthread_func_pthread_exit *pthread_exit;
	fpl__pthread_func_pthread_yield *pthread_yield;
	fpl__pthread_func_pthread_timedjoin_np *pthread_timedjoin_np;

	// pthread_attr_t
	fpl__pthread_func_pthread_attr_init *pthread_attr_init;
	fpl__pthread_func_pthread_attr_getschedparam *pthread_attr_getschedparam;
	fpl__pthread_func_pthread_attr_setschedparam *pthread_attr_setschedparam;
	fpl__pthread_func_pthread_attr_setstacksize *pthread_attr_setstacksize;
	fpl__pthread_func_pthread_attr_setdetachstate *pthread_attr_setdetachstate;
	fpl__pthread_func_pthread_attr_setschedpolicy *pthread_attr_setschedpolicy;

	// pthread_mutex_t
	fpl__pthread_func_pthread_mutex_init *pthread_mutex_init;
	fpl__pthread_func_pthread_mutex_destroy *pthread_mutex_destroy;
	fpl__pthread_func_pthread_mutex_lock *pthread_mutex_lock;
	fpl__pthread_func_pthread_mutex_trylock *pthread_mutex_trylock;
	fpl__pthread_func_pthread_mutex_unlock *pthread_mutex_unlock;

	// pthread_cond_t
	fpl__pthread_func_pthread_cond_init *pthread_cond_init;
	fpl__pthread_func_pthread_cond_destroy *pthread_cond_destroy;
	fpl__pthread_func_pthread_cond_timedwait *pthread_cond_timedwait;
	fpl__pthread_func_pthread_cond_wait *pthread_cond_wait;
	fpl__pthread_func_pthread_cond_broadcast *pthread_cond_broadcast;
	fpl__pthread_func_pthread_cond_signal *pthread_cond_signal;

	// sem_t
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
	fplAssert(pthreadApi != fpl_null);
	if (pthreadApi->libHandle != fpl_null) {
		dlclose(pthreadApi->libHandle);
	}
	fplClearStruct(pthreadApi);
}

fpl_internal bool fpl__PThreadLoadApi(fpl__PThreadApi *pthreadApi) {
	const char *libpthreadFileNames[] = {
		"libpthread.so",
		"libpthread.so.0",
	};
	bool result = false;
	for (uint32_t index = 0; index < fplArrayCount(libpthreadFileNames); ++index) {
		const char *libName = libpthreadFileNames[index];
		fplClearStruct(pthreadApi);
		do {
			void *libHandle = fpl_null;
			FPL__POSIX_LOAD_LIBRARY(FPL__MODULE_PTHREAD, libHandle, libName);

			// pthread_t
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_self, pthread_self);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_setschedparam, pthread_setschedparam);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_getschedparam, pthread_getschedparam);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_setschedprio, pthread_setschedprio);

			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_create, pthread_create);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_kill, pthread_kill);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_join, pthread_join);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_exit, pthread_exit);
			FPL__POSIX_GET_FUNCTION_ADDRESS_OPTIONAL(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_yield, pthread_yield);
			FPL__POSIX_GET_FUNCTION_ADDRESS_OPTIONAL(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_timedjoin_np, pthread_timedjoin_np);

			// pthread_attr_t
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_init, pthread_attr_init);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_getschedparam, pthread_attr_getschedparam);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_setschedparam, pthread_attr_setschedparam);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_setstacksize, pthread_attr_setstacksize);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_setdetachstate, pthread_attr_setdetachstate);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_attr_setschedpolicy, pthread_attr_setschedpolicy);

			// pthread_mutex_t
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_mutex_init, pthread_mutex_init);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_mutex_destroy, pthread_mutex_destroy);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_mutex_lock, pthread_mutex_lock);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_mutex_trylock, pthread_mutex_trylock);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_mutex_unlock, pthread_mutex_unlock);

			// pthread_cond_t
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_init, pthread_cond_init);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_destroy, pthread_cond_destroy);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_timedwait, pthread_cond_timedwait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_wait, pthread_cond_wait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_broadcast, pthread_cond_broadcast);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_pthread_cond_signal, pthread_cond_signal);

			// sem_t
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_init, sem_init);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_destroy, sem_destroy);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_wait, sem_wait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_timedwait, sem_timedwait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_trywait, sem_trywait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_post, sem_post);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_PTHREAD, libHandle, libName, pthreadApi, fpl__pthread_func_sem_getvalue, sem_getvalue);

			pthreadApi->libHandle = libHandle;
			result = true;
		} while (0);
		if (result) {
			break;
		}
		fpl__PThreadUnloadApi(pthreadApi);
	}
	return(result);
}

typedef struct fpl__PosixInitState {
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
	int dummy;
} fpl__LinuxInitState;

#if defined(FPL__ENABLE_WINDOW)
#define FPL__LINUX_MAX_GAME_CONTROLLER_COUNT 4
typedef struct fpl__LinuxGameController {
	char deviceName[512 + 1];
	char displayName[FPL_MAX_NAME_LENGTH];
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
#if defined(FPL__ENABLE_WINDOW)
	fpl__LinuxGameControllersState controllersState;
#endif
	int dummy;
} fpl__LinuxAppState;

// Forward declarations
#if defined(FPL__ENABLE_WINDOW)
fpl_internal void fpl__LinuxFreeGameControllers(fpl__LinuxGameControllersState *controllersState);
fpl_internal void fpl__LinuxPollGameControllers(const fplSettings *settings, fpl__LinuxGameControllersState *controllersState, const bool useEvents);
#endif

#endif // FPL_PLATFORM_LINUX

// ############################################################################
//
// > TYPES_UNIX
//
// ############################################################################
#if defined(FPL_PLATFORM_UNIX)
typedef struct fpl__UnixInitState {
	int dummy;
} fpl__UnixInitState;

typedef struct fpl__UnixAppState {
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
#define FPL__FUNC_X11_XInternAtom(name) Atom name(Display *display, const char *atom_name, Bool only_if_exists)
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
#define FPL__FUNC_X11_XLookupString(name) int name(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer, KeySym* keysym_return, XComposeStatus* status_in_out)
typedef FPL__FUNC_X11_XLookupString(fpl__func_x11_XLookupString);
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
#define FPL__FUNC_X11_XQueryPointer(name) Bool name(Display* display, Window w, Window* root_return, Window* child_return, int* root_x_return, int* root_y_return, int* win_x_return, int* win_y_return, unsigned int* mask_return)
typedef FPL__FUNC_X11_XQueryPointer(fpl__func_x11_XQueryPointer);
#define FPL__FUNC_X11_XConvertSelection(name) int name(Display *display, Atom selection, Atom target, Atom property, Window requestor, Time time)
typedef FPL__FUNC_X11_XConvertSelection(fpl__func_x11_XConvertSelection);
#define FPL__FUNC_X11_XInitThreads(name) Status name(void)
typedef FPL__FUNC_X11_XInitThreads(fpl__func_x11_XInitThreads);
#define FPL__FUNC_X11_XSetErrorHandler(name) XErrorHandler name(XErrorHandler *handler)
typedef FPL__FUNC_X11_XSetErrorHandler(fpl__func_x11_XSetErrorHandler);

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
	fpl__func_x11_XLookupString *XLookupString;
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
	fpl__func_x11_XQueryPointer *XQueryPointer;
	fpl__func_x11_XConvertSelection *XConvertSelection;
	fpl__func_x11_XInitThreads *XInitThreads;
	fpl__func_x11_XSetErrorHandler *XSetErrorHandler;
} fpl__X11Api;

fpl_internal void fpl__UnloadX11Api(fpl__X11Api *x11Api) {
	fplAssert(x11Api != fpl_null);
	if (x11Api->libHandle != fpl_null) {
		dlclose(x11Api->libHandle);
	}
	fplClearStruct(x11Api);
}

fpl_internal bool fpl__LoadX11Api(fpl__X11Api *x11Api) {
	fplAssert(x11Api != fpl_null);
	const char *libFileNames[] = {
		"libX11.so",
		"libX11.so.7",
		"libX11.so.6",
		"libX11.so.5",
	};
	bool result = false;
	for (uint32_t index = 0; index < fplArrayCount(libFileNames); ++index) {
		const char *libName = libFileNames[index];
		fplClearStruct(x11Api);
		do {
			void *libHandle = fpl_null;
			FPL__POSIX_LOAD_LIBRARY(FPL__MODULE_X11, libHandle, libName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XFlush, XFlush);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XFree, XFree);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XOpenDisplay, XOpenDisplay);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCloseDisplay, XCloseDisplay);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDefaultScreen, XDefaultScreen);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XRootWindow, XRootWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCreateWindow, XCreateWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDestroyWindow, XDestroyWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCreateColormap, XCreateColormap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XFreeColormap, XFreeColormap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDefaultColormap, XDefaultColormap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XMapWindow, XMapWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XUnmapWindow, XUnmapWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XStoreName, XStoreName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDefaultVisual, XDefaultVisual);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDefaultDepth, XDefaultDepth);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XInternAtom, XInternAtom);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSetWMProtocols, XSetWMProtocols);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XPending, XPending);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSync, XSync);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XNextEvent, XNextEvent);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XPeekEvent, XPeekEvent);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XEventsQueued, XEventsQueued);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XGetWindowAttributes, XGetWindowAttributes);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XResizeWindow, XResizeWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XMoveWindow, XMoveWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XGetKeyboardMapping, XGetKeyboardMapping);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XLookupString, XLookupString);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSendEvent, XSendEvent);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XMatchVisualInfo, XMatchVisualInfo);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCreateGC, XCreateGC);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XGetImage, XGetImage);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XPutImage, XPutImage);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XMapRaised, XMapRaised);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCreateImage, XCreateImage);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XCreatePixmap, XCreatePixmap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSelectInput, XSelectInput);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XGetWindowProperty, XGetWindowProperty);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XChangeProperty, XChangeProperty);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XDeleteProperty, XDeleteProperty);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XStringListToTextProperty, XStringListToTextProperty);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSetWMIconName, XSetWMIconName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSetWMName, XSetWMName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XQueryKeymap, XQueryKeymap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XQueryPointer, XQueryPointer);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XConvertSelection, XConvertSelection);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XInitThreads, XInitThreads);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_X11, libHandle, libName, x11Api, fpl__func_x11_XSetErrorHandler, XSetErrorHandler);
			x11Api->libHandle = libHandle;
			result = true;
		} while (0);
		if (result) {
			break;
		}
		fpl__UnloadX11Api(x11Api);
	}
	return(result);
}

typedef struct fpl__X11SubplatformState {
	fpl__X11Api api;
} fpl__X11SubplatformState;

typedef struct fpl__X11WindowStateInfo {
	fplWindowState state;
	fplWindowVisibilityState visibility;
	fplWindowPosition position;
	fplWindowSize size;
} fpl__X11WindowStateInfo;

typedef struct fpl__X11Xdnd {
	int version;
	Window source;
	Atom format;
} fpl__X11Xdnd;

#define FPL__FUNC_X11_ErrorHandlerCallback(name) int name(Display *display, XErrorEvent *ev)
typedef FPL__FUNC_X11_ErrorHandlerCallback(fpl__func_X11ErrorHandlerCallback);

typedef struct fpl__X11WindowState {
	fpl__X11WindowStateInfo lastWindowStateInfo;
	Colormap colorMap;
	Display *display;
	fpl__func_X11ErrorHandlerCallback *lastErrorHandler;
	fpl__X11Xdnd xdnd;
	Window root;
	Window window;
	Visual *visual;
	Atom wmProtocols;
	Atom wmDeleteWindow;
	Atom wmState;
	Atom netWMPing;
	Atom netWMState;
	Atom netWMStateFocused;
	Atom netWMStateFullscreen;
	Atom netWMStateHidden;
	Atom netWMStateMaximizedVert;
	Atom netWMStateMaximizedHorz;
	Atom netWMPid;
	Atom netWMIcon;
	Atom netWMName;
	Atom netWMIconName;
	Atom utf8String;
	Atom motifWMHints;
	// drag and drop
	Atom xdndAware;
	Atom xdndEnter;
	Atom xdndPosition;
	Atom xdndStatus;
	Atom xdndActionCopy;
	Atom xdndDrop;
	Atom xdndFinished;
	Atom xdndSelection;
	Atom xdndTypeList;
	Atom textUriList;
	int screen;
	int colorDepth;
} fpl__X11WindowState;

#define FPL__XDND_VERSION 5

#endif // FPL_SUBPLATFORM_X11

// ****************************************************************************
//
// > PLATFORM_STATES
//
// - Defines the final PlatformInitState and PlatformAppState
// - Declares all required global variables
//
// ****************************************************************************
#if !defined(FPL__PLATFORM_STATES_DEFINED)
#define FPL__PLATFORM_STATES_DEFINED
//
// Platform initialization state
//
typedef struct fpl__PlatformInitSettings {
	fplMemorySettings memorySettings;
} fpl__PlatformInitSettings;

typedef struct fpl__PlatformInitState {
#if defined(FPL_SUBPLATFORM_POSIX)
	fpl__PosixInitState posix;
#endif

	fpl__PlatformInitSettings initSettings;
	fplPlatformResultType initResult;
	fpl_b32 isInitialized;

	union {
#	if defined(FPL_PLATFORM_WINDOWS)
		fpl__Win32InitState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxInitState plinux;
#	elif defined(FPL_PLATFORM_UNIX)
		fpl__UnixInitState punix;
#	endif				
	};
} fpl__PlatformInitState;
fpl_globalvar fpl__PlatformInitState fpl__global__InitState = fplZeroInit;

#if defined(FPL__ENABLE_WINDOW)
#define FPL__MAX_EVENT_COUNT 32768
typedef struct fpl__EventQueue {
	// @FIXME(final): Internal events are not Thread-Safe!
	fplEvent events[FPL__MAX_EVENT_COUNT];
	uint32_t pollIndex;
	uint32_t pushCount;
} fpl__EventQueue;

typedef struct fpl__PlatformWindowState {
	fpl__EventQueue eventQueue;
	fplKey keyMap[256];
	fplButtonState keyStates[256];
	uint64_t keyPressTimes[256];
	fplButtonState mouseStates[5];
	fpl_b32 isRunning;

#if defined(FPL_PLATFORM_WINDOWS)
	fpl__Win32WindowState win32;
#endif
#if defined(FPL_SUBPLATFORM_X11)
	fpl__X11WindowState x11;
#endif
} fpl__PlatformWindowState;
#endif // FPL__ENABLE_WINDOW

#if defined(FPL__ENABLE_VIDEO)
typedef struct fpl__PlatformVideoState {
	void *mem; // Points to fpl__VideoState
	size_t memSize;
} fpl__PlatformVideoState;
#endif // FPL__ENABLE_VIDEO

#if defined(FPL__ENABLE_AUDIO)
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
#if defined(FPL__ENABLE_WINDOW)
	fpl__PlatformWindowState window;
#endif
#if defined(FPL__ENABLE_VIDEO)
	fpl__PlatformVideoState video;
#endif
#if defined(FPL__ENABLE_AUDIO)
	fpl__PlatformAudioState audio;
#endif

	// Settings
	fplSettings initSettings;
	fplSettings currentSettings;
	fplInitFlags initFlags;

	// Platforms
	union {
#	if defined(FPL_PLATFORM_WINDOWS)
		fpl__Win32AppState win32;
#	elif defined(FPL_PLATFORM_LINUX)
		fpl__LinuxAppState plinux;
#	elif defined(FPL_PLATFORM_UNIX)
		fpl__UnixAppState plinux;
#	endif
	};
};

//
// Internal window
//
#if defined(FPL__ENABLE_WINDOW)
fpl_internal fplKey fpl__GetMappedKey(const fpl__PlatformWindowState *windowState, const uint64_t keyCode) {
	fplKey result;
	if (keyCode < fplArrayCount(windowState->keyMap))
		result = windowState->keyMap[keyCode];
	else
		result = fplKey_None;
	return(result);
}

fpl_internal void fpl__ClearInternalEvents() {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fplAssert(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	// @FIXME(final): Internal events are not thread-safe, introduce a proper thread-safe queue here!
	uint32_t eventCount = eventQueue->pollIndex;
	eventQueue->pollIndex = 0;
	for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex) {
		fplEvent *ev = &eventQueue->events[eventIndex];
		if (ev->window.dropFiles.internalMemory.base != fpl_null) {
			fpl__ReleaseDynamicMemory(ev->window.dropFiles.internalMemory.base);
			fplClearStruct(&ev->window.dropFiles.internalMemory);
		}
	}
	eventQueue->pushCount = 0;
}

fpl_internal bool fpl__PollInternalEvent(fplEvent *ev) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = false;
	if (appState != fpl_null) {
		fpl__EventQueue *eventQueue = &appState->window.eventQueue;
		// @FIXME(final): Internal events are not thread-safe, introduce a proper thread-safe queue here!
		if (eventQueue->pollIndex < eventQueue->pushCount) {
			uint32_t eventIndex = eventQueue->pollIndex++;
			*ev = eventQueue->events[eventIndex];
			result = true;
		} else if (eventQueue->pushCount > 0) {
			eventQueue->pollIndex = 0;
			eventQueue->pushCount = 0;
		}
	}
	return(result);
}

fpl_internal void fpl__PushInternalEvent(const fplEvent *event) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fplAssert(appState != fpl_null);
	fpl__EventQueue *eventQueue = &appState->window.eventQueue;
	// @FIXME(final): Internal events are not thread-safe, introduce a proper thread-safe queue here!
	if (eventQueue->pushCount < FPL__MAX_EVENT_COUNT) {
		uint32_t eventIndex = eventQueue->pushCount++;
		eventQueue->events[eventIndex] = *event;
	}
}

fpl_internal void fpl__PushWindowStateEvent(const fplWindowEventType windowType) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = windowType;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushWindowSizeEvent(const fplWindowEventType windowType, uint32_t w, uint32_t h) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = windowType;
	newEvent.window.size.width = w;
	newEvent.window.size.height = h;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushWindowPositionEvent(const fplWindowEventType windowType, int32_t x, int32_t y) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = windowType;
	newEvent.window.position.left = x;
	newEvent.window.position.top = y;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushWindowDropFilesEvent(const char *filePath, const size_t fileCount, const char **files, const fplMemoryBlock *memory) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Window;
	newEvent.window.type = fplWindowEventType_DroppedFiles;
	newEvent.window.dropFiles.fileCount = fileCount;
	newEvent.window.dropFiles.files = files;
	newEvent.window.dropFiles.internalMemory = *memory;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushKeyboardButtonEvent(const uint64_t keyCode, const fplKey mappedKey, const fplKeyboardModifierFlags modifiers, const fplButtonState buttonState) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.type = fplKeyboardEventType_Button;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.modifiers = modifiers;
	newEvent.keyboard.buttonState = buttonState;
	newEvent.keyboard.mappedKey = mappedKey;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushKeyboardInputEvent(const uint32_t textCode, const fplKey mappedKey) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Keyboard;
	newEvent.keyboard.type = fplKeyboardEventType_Input;
	newEvent.keyboard.keyCode = (uint64_t)textCode;
	newEvent.keyboard.mappedKey = mappedKey;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushMouseButtonEvent(const int32_t x, const int32_t y, const fplMouseButtonType mouseButton, const fplButtonState buttonState) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Button;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	newEvent.mouse.mouseButton = mouseButton;
	newEvent.mouse.buttonState = buttonState;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushMouseWheelEvent(const int32_t x, const int32_t y, const float wheelDelta) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Wheel;
	newEvent.mouse.mouseButton = fplMouseButtonType_None;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	newEvent.mouse.wheelDelta = wheelDelta;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__PushMouseMoveEvent(const int32_t x, const int32_t y) {
	fplEvent newEvent = fplZeroInit;
	newEvent.type = fplEventType_Mouse;
	newEvent.mouse.type = fplMouseEventType_Move;
	newEvent.mouse.mouseButton = fplMouseButtonType_None;
	newEvent.mouse.mouseX = x;
	newEvent.mouse.mouseY = y;
	fpl__PushInternalEvent(&newEvent);
}

fpl_internal void fpl__HandleKeyboardButtonEvent(fpl__PlatformWindowState *windowState, const uint64_t time, const uint64_t keyCode, const fplKeyboardModifierFlags modifiers, const fplButtonState buttonState, const bool force) {
#if defined(FPL_LOG_KEY_EVENTS)
	const char *buttonStateName = "";
	if (buttonState == fplButtonState_Press)
		buttonStateName = "Press";
	else if (buttonState == fplButtonState_Repeat)
		buttonStateName = "Repeat";
	else
		buttonStateName = "Released";
	FPL_LOG_INFO(FPL__MODULE_OS, "[%llu] Keyboard button event with keycode: '%llu', state: '%s'", time, keyCode, buttonStateName);
#endif

	fplKey mappedKey = fpl__GetMappedKey(windowState, keyCode);
	bool repeat = false;
	if (force) {
		repeat = (buttonState == fplButtonState_Repeat);
		windowState->keyStates[keyCode] = buttonState;
	} else {
		if (keyCode < fplArrayCount(windowState->keyStates)) {
			if ((buttonState == fplButtonState_Release) && (windowState->keyStates[keyCode] == fplButtonState_Release)) {
				return;
			}
			if ((buttonState == fplButtonState_Press) && (windowState->keyStates[keyCode] >= fplButtonState_Press)) {
				repeat = true;
			}
			windowState->keyStates[keyCode] = buttonState;
		}
	}
	fpl__PushKeyboardButtonEvent(keyCode, mappedKey, modifiers, repeat ? fplButtonState_Repeat : buttonState);
}

fpl_internal void fpl__HandleKeyboardInputEvent(fpl__PlatformWindowState *windowState, const uint64_t keyCode, const uint32_t textCode) {
	fplKey mappedKey = fpl__GetMappedKey(windowState, keyCode);
	fpl__PushKeyboardInputEvent(textCode, mappedKey);
}

fpl_internal void fpl__HandleMouseButtonEvent(fpl__PlatformWindowState *windowState, const int32_t x, const int32_t y, const fplMouseButtonType mouseButton, const fplButtonState buttonState) {
	if (mouseButton < fplArrayCount(windowState->mouseStates)) {
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

// @NOTE(final): Callback used for setup a window before it is created
#define FPL__FUNC_PREPARE_VIDEO_WINDOW(name) bool name(fpl__PlatformAppState *appState, const fplInitFlags initFlags, const fplSettings *initSettings)
typedef FPL__FUNC_PREPARE_VIDEO_WINDOW(callback_PreSetupWindow);

// @NOTE(final): Callback used for setup a window after it was created
#define FPL__FUNC_FINALIZE_VIDEO_WINDOW(name) bool name(fpl__PlatformAppState *appState, const fplInitFlags initFlags, const fplSettings *initSettings)
typedef FPL__FUNC_FINALIZE_VIDEO_WINDOW(callback_PostSetupWindow);

typedef struct fpl__SetupWindowCallbacks {
	callback_PreSetupWindow *preSetup;
	callback_PostSetupWindow *postSetup;
} fpl__SetupWindowCallbacks;
#endif // FPL__ENABLE_WINDOW

#endif // FPL__PLATFORM_STATES_DEFINED

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
#if !defined(FPL__COMMON_DEFINED)
#define FPL__COMMON_DEFINED

//
// Internal memory
//
fpl_internal void *fpl__AllocateDynamicMemory(const size_t size, const size_t alignment) {
	void *result = fpl__AllocateMemory(&fpl__global__InitState.initSettings.memorySettings.dynamic, size, alignment);
	return(result);
}

fpl_internal void fpl__ReleaseDynamicMemory(void *ptr) {
	fpl__ReleaseMemory(&fpl__global__InitState.initSettings.memorySettings.dynamic, ptr);
}

fpl_internal void *fpl__AllocateTemporaryMemory(const size_t size, const size_t alignment) {
	void *result = fpl__AllocateMemory(&fpl__global__InitState.initSettings.memorySettings.temporary, size, alignment);
	return(result);
}

fpl_internal void fpl__ReleaseTemporaryMemory(void *ptr) {
	fpl__ReleaseMemory(&fpl__global__InitState.initSettings.memorySettings.temporary, ptr);
}

//
// Audio constants
//
static const uint32_t FPL__DEFAULT_AUDIO_SAMPLERATE = 44100;
static const fplAudioFormatType FPL__DEFAULT_AUDIO_FORMAT = fplAudioFormatType_S16;
static const uint32_t FPL__DEFAULT_AUDIO_CHANNELS = 2;
static const uint32_t FPL__DEFAULT_AUDIO_PERIODS = 3;
static const uint32_t FPL__DEFAULT_AUDIO_BUFFERSIZE_LOWLATENCY_IN_MSECS = 10;
static const uint32_t FPL__DEFAULT_AUDIO_BUFFERSIZE_CONSERVATIVE_IN_MSECS = 25;

//
// Internal types and functions
//
#define FPL__MAX_LAST_ERROR_STRING_LENGTH 256
#define FPL__MAX_ERRORSTATE_COUNT 256

typedef struct fpl__ErrorState {
	char errors[FPL__MAX_ERRORSTATE_COUNT][FPL__MAX_LAST_ERROR_STRING_LENGTH];
	uint32_t count;
} fpl__ErrorState;

fpl_globalvar fpl__ErrorState fpl__global__LastErrorState = fplZeroInit;

fpl_internal void fpl__PushError_Formatted(const char *funcName, const int lineNumber, const fplLogLevel level, const char *format, va_list argList) {
	fplAssert(format != fpl_null);

	if (level <= fplLogLevel_Error) {
		fpl__ErrorState *state = &fpl__global__LastErrorState;
		char buffer[FPL__MAX_LAST_ERROR_STRING_LENGTH] = fplZeroInit;
		size_t formattedLen = fplStringFormatArgs(buffer, fplArrayCount(buffer), format, argList);
		if (formattedLen > 0) {
			fplAssert(state->count < FPL__MAX_ERRORSTATE_COUNT);
			size_t errorIndex = state->count;
			state->count = (state->count + 1) % FPL__MAX_ERRORSTATE_COUNT;
			fplCopyStringLen(buffer, formattedLen, state->errors[errorIndex], FPL__MAX_LAST_ERROR_STRING_LENGTH);
		}
	}

#if defined(FPL__ENABLE_LOGGING)
	va_list listCopy;
	va_copy(listCopy, argList);
	fpl__LogWriteArgs(funcName, lineNumber, level, format, listCopy);
	va_end(listCopy);
#endif
}

fpl_internal void fpl__HandleError(const char *funcName, const int lineNumber, const fplLogLevel level, const char *format, ...) {
	va_list valist;
	va_start(valist, format);
	fpl__PushError_Formatted(funcName, lineNumber, level, format, valist);
	va_end(valist);

#if defined(FPL_CRASH_ON_ERROR) || defined(FPL_CRASH_ON_WARNING)
	fplLogLevel minErrorLevel = fplLogLevel_Error;
#	if defined(FPL_CRASH_ON_WARNING)
	minErrorLevel = fplLogLevel_Warning;
#	endif
	if (level >= minErrorLevel) {
		// @NOTE(final): Force a null pointer assignment crash here
		*(int *)0 = 0;
	}
#endif
}

//
// Argument Errors
//

// 


fpl_internal void fpl__ArgumentInvalidError(const char *funcName, const int line, const char *paramName) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter are not valid", paramName);
}
fpl_internal void fpl__ArgumentNullError(const char *funcName, const int line, const char *paramName) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter are not allowed to be null", paramName);
}
fpl_internal void fpl__ArgumentZeroError(const char *funcName, const int line, const char *paramName) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter must be greater than zero", paramName);
}
fpl_internal void fpl__ArgumentMinError(const char *funcName, const int line, const char *paramName, const size_t value, const size_t minValue) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter '%zu' must be greater or equal than '%zu'", paramName, value, minValue);
}
fpl_internal void fpl__ArgumentMaxError(const char *funcName, const int line, const char *paramName, const size_t value, const size_t maxValue) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter '%zu' must be less or equal than '%zu'", paramName, value, maxValue);
}
fpl_internal void fpl__ArgumentRangeError(const char *funcName, const int line, const char *paramName, const size_t value, const size_t minValue, const size_t maxValue) {
	FPL__M_ERROR(funcName, line, FPL__MODULE_ARGS, "%s parameter '%zu' must be in range of '%zu' to '%zu'", paramName, value, minValue, maxValue);
}

#define FPL__CheckArgumentInvalid(arg, cond, ret) \
	if((cond)) { \
		fpl__ArgumentInvalidError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return (ret); \
	}

#define FPL__CheckArgumentInvalidNoRet(arg, cond) \
	if((cond)) { \
		fpl__ArgumentInvalidError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return; \
	}
#define FPL__CheckArgumentNull(arg, ret) \
	if((arg) == fpl_null) { \
		fpl__ArgumentNullError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return (ret); \
	}
#define FPL__CheckArgumentNullNoRet(arg) \
	if((arg) == fpl_null) { \
		fpl__ArgumentNullError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return; \
	}
#define FPL__CheckArgumentZero(arg, ret) \
	if((arg) == 0) { \
		fpl__ArgumentZeroError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return (ret); \
	}
#define FPL__CheckArgumentZeroNoRet(arg) \
	if((arg) == 0) { \
		fpl__ArgumentZeroError(FPL_FUNCTION_NAME, __LINE__, #arg); \
		return; \
	}
#define FPL__CheckArgumentMin(arg, minValue, ret) \
	if((arg) < (minValue)) { \
		fpl__ArgumentMinError(FPL_FUNCTION_NAME, __LINE__, #arg, arg, minValue); \
		return (ret); \
	}
#define FPL__CheckArgumentMax(arg, maxValue, ret) \
	if((arg) > (maxValue)) { \
		fpl__ArgumentMaxError(FPL_FUNCTION_NAME, __LINE__, #arg, arg, maxValue); \
		return (ret); \
	}
#define FPL__CheckPlatform(ret) \
	if(fpl__global__AppState == fpl_null) { \
		FPL__ERROR(FPL__MODULE_CORE, "[%s] Platform is not initialized", FPL_FUNCTION_NAME); \
		return (ret); \
	}
#define FPL__CheckPlatformNoRet() \
	if(fpl__global__AppState == fpl_null) { \
		FPL__ERROR(FPL__MODULE_CORE, "[%s] Platform is not initialized", FPL_FUNCTION_NAME); \
		return; \
	}

#define FPL__CheckApi(cond, name, ret) \
	if(!(cond)) { \
		FPL__ERROR(FPL__MODULE_API, "The API '%s' is not loaded", (name)); \
		return (ret); \
	}
#define FPL__CheckApiNoRet(cond, name) \
	if(!(cond)) { \
		FPL__ERROR(FPL__MODULE_API, "The API '%s' is not loaded", (name)); \
		return; \
	}

#if !defined(FPL_MAX_THREAD_COUNT)
	// Maximum number of active threads you can have in your process
#	define FPL_MAX_THREAD_COUNT 256
#endif

#if !defined(FPL_MAX_SIGNAL_COUNT)
	// Maximum number of active signals you can wait for
#	define FPL_MAX_SIGNAL_COUNT 256
#endif

typedef struct fpl__ThreadState {
	fplThreadHandle mainThread;
	fplThreadHandle threads[FPL_MAX_THREAD_COUNT];
} fpl__ThreadState;

fpl_globalvar fpl__ThreadState fpl__global__ThreadState = fplZeroInit;

fpl_internal fplThreadHandle *fpl__GetFreeThread() {
	fplThreadHandle *result = fpl_null;
	for (uint32_t index = 0; index < FPL_MAX_THREAD_COUNT; ++index) {
		fplThreadHandle *thread = fpl__global__ThreadState.threads + index;
		fplThreadState state = fplGetThreadState(thread);
		if (state == fplThreadState_Stopped) {
			result = thread;
			break;
		}
	}
	return(result);
}

fpl_internal bool fpl__IsEqualsMemory(const void *a, const void *b, const size_t size) {
	const uint8_t *ptrA = (const uint8_t *)a;
	const uint8_t *ptrB = (const uint8_t *)b;
	size_t s = size;
	// @SPEED(final): This may be very slow, so we should use a faster function for comparing memory.
	bool result = true;
#if 1
	while (s-- > 0) {
		if (*ptrA != *ptrB) {
			result = false;
			break;
		}
		ptrA++;
		ptrB++;
	}
#else
	result = memcmp(a, b, size) == 0;
#endif
	return(result);
}

fpl_internal bool fpl__IsZeroMemory(const void *memory, const size_t size) {
	const uint8_t *ptr = (const uint8_t *)memory;
	// @SPEED(final): This may be very slow, so we should use a faster function for comparing memory.
	bool result = true;
#if 1
	size_t s = size;
	while (s-- > 0) {
		if (*ptr) {
			result = false;
			break;
		}
		ptr++;
	}
#else
	result = memcmp(a, b, size) == 0;
#endif
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
	if (source == fpl_null || wildcard == fpl_null) {
		return false;
	}
	const char *s = source;
	const char *w = wildcard;
	while (*w) {
		if (*w == '?') {
			if (!*s) {
				return false;
			}
			++s;
		} else if (*w == '*') {
			while (*s) {
				char nw = w[1];
				if (nw != 0) {
					if ((*s == nw) || (nw == '?') || (nw == '*')) {
						break;
					}
				}
				++s;
			}
		} else {
			if (*s != *w) {
				return false;
			}
			++s;
		}
		++w;
	}
	return true;
}

fpl_common_api bool fplIsStringEqualLen(const char *a, const size_t aLen, const char *b, const size_t bLen) {
	if ((a == fpl_null) || (b == fpl_null)) {
		return false;
	}
	if (aLen != bLen) {
		return false;
	}
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

fpl_common_api char *fplEnforcePathSeparatorLen(char *path, size_t maxPathLen) {
	FPL__CheckArgumentNull(path, fpl_null);
	FPL__CheckArgumentZero(maxPathLen, fpl_null);
	char *end = path;
	while (*end) {
		end++;
	}
	size_t len = end - path;
	char *result = fpl_null;
	if (len > 0) {
		if (path[len - 1] != FPL_PATH_SEPARATOR) {
			if (len + 1 <= maxPathLen) {
				path[len] = FPL_PATH_SEPARATOR;
				path[len + 1] = 0;
				result = &path[len + 1];
			} else {
				FPL__ERROR(FPL__MODULE_PATHS, "Cannot append path separator: Max length '%zu' of path '%s' is exceeded", maxPathLen, path);
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
	while (*end) {
		end++;
	}
	size_t len = end - path;
	char *result = fpl_null;
	if (len > 0) {
		if (path[len - 1] != FPL_PATH_SEPARATOR) {
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
	if (appendedLen == 0) {
		return buffer;
	}
	size_t curBufferLen = fplGetStringLength(buffer);
	size_t requiredSize = curBufferLen + appendedLen + 1;
	FPL__CheckArgumentMin(maxBufferLen, requiredSize, fpl_null);
	char *start = buffer + curBufferLen;
	size_t remainingBufferSize = maxBufferLen - (curBufferLen > 0 ? curBufferLen + 1 : 0);
	char *result = fplCopyStringLen(appended, appendedLen, start, remainingBufferSize);
	return(result);
}

fpl_common_api char *fplStringAppend(const char *appended, char *buffer, size_t maxBufferLen) {
	size_t appendedLen = fplGetStringLength(appended);
	char *result = fplStringAppendLen(appended, appendedLen, buffer, maxBufferLen);
	return(result);
}

fpl_common_api size_t fplGetStringLength(const char *str) {
	uint32_t result = 0;
	if (str != fpl_null) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_common_api char *fplCopyStringLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen) {
	if (source != fpl_null && dest != fpl_null) {
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

fpl_common_api char *fplCopyString(const char *source, char *dest, const size_t maxDestLen) {
	char *result = fpl_null;
	if (source != fpl_null) {
		size_t sourceLen = fplGetStringLength(source);
		result = fplCopyStringLen(source, sourceLen, dest, maxDestLen);
	}
	return(result);
}

fpl_common_api size_t fplStringFormatArgs(char *destBuffer, const size_t maxDestBufferLen, const char *format, va_list argList) {
	FPL__CheckArgumentNull(format, 0);

	va_list listCopy;
	va_copy(listCopy, argList);

	// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
	if (destBuffer != fpl_null) {
		FPL__CheckArgumentMin(maxDestBufferLen, 1, 0);
		destBuffer[0] = 0;
	}

	int charCount = 0;
#	if defined(FPL_NO_CRT)
#		if defined(FPL_USERFUNC_vsnprintf)
	charCount = FPL_USERFUNC_vsnprintf(destBuffer, maxDestBufferLen, format, listCopy);
#		else
	charCount = 0;
#		endif
#	else
	charCount = vsnprintf(destBuffer, maxDestBufferLen, format, listCopy);
#	endif
	if (charCount < 0) {
		FPL__ERROR(FPL__MODULE_STRINGS, "Format parameter are '%s' are invalid", format);
		return 0;
	}
	size_t result = charCount;
	if (destBuffer != fpl_null) {
		size_t requiredMaxDestBufferLen = charCount + 1;
		FPL__CheckArgumentMin(maxDestBufferLen, requiredMaxDestBufferLen, 0);
		destBuffer[charCount] = 0;
	}
	va_end(listCopy);
	return(result);
}

fpl_common_api size_t fplStringFormat(char *destBuffer, const size_t maxDestBufferLen, const char *format, ...) {
	FPL__CheckArgumentNull(format, 0);
	va_list argList;
	va_start(argList, format);
	size_t result = fplStringFormatArgs(destBuffer, maxDestBufferLen, format, argList);
	va_end(argList);
	return(result);
}

fpl_common_api size_t fplS32ToString(const int32_t value, char *buffer, const size_t maxBufferLen) {
	int32_t v = value;

	bool isNegative = false;
	if (v < 0) {
		isNegative = true;
		v = -v;
	}

	int32_t tmp = v;
	size_t digitCount = 0;
	do {
		tmp = tmp / 10;
		++digitCount;
	} while (tmp != 0);

	size_t result = digitCount + (isNegative ? 1 : 0);

	if (buffer != fpl_null) {
		size_t requiredLen = result + 1;
		FPL__CheckArgumentMin(maxBufferLen, requiredLen, 0);

		char *p = buffer;
		if (isNegative) {
			*p++ = '-';
		}
		p += digitCount;// Go back to the very end, because we are writing the digits back in reverse order
		char *lastP = p;

		const char *digits = "0123456789";
		tmp = value;
		do {
			*--p = digits[tmp % 10];
			tmp /= 10;
		} while (tmp != 0);

		*lastP = 0;
	}

	return (result);
}

fpl_common_api int32_t fplStringToS32Len(const char *str, const size_t len) {
	FPL__CheckArgumentNull(str, 0);
	FPL__CheckArgumentZero(len, 0);
	const char *p = str;
	bool isNegative = false;
	if (*p == '-') {
		if (len == 1) {
			return 0;
		}
		isNegative = true;
		++p;
	}
	uint32_t value = 0;
	while (*p && ((size_t)(p - str) < len)) {
		char c = *p;
		if (c < '0' || c > '9') {
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
	size_t len = fplGetStringLength(str);
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
	char buffer[FPL_MAX_BUFFER_LENGTH];
	va_list argList;
	va_start(argList, format);
	size_t len = fplStringFormatArgs(buffer, fplArrayCount(buffer), format, argList);
	va_end(argList);
	if (len > 0) {
		fplConsoleOut(buffer);
	}
}

fpl_common_api void fplConsoleFormatError(const char *format, ...) {
	FPL__CheckArgumentNullNoRet(format);
	char buffer[FPL_MAX_BUFFER_LENGTH];
	va_list argList;
	va_start(argList, format);
	size_t len = fplStringFormatArgs(buffer, fplArrayCount(buffer), format, argList);
	va_end(argList);
	if (len > 0) {
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
	if (alignment & (alignment - 1)) {
		FPL__ERROR(FPL__MODULE_MEMORY, "Alignment parameter '%zu' must be a power of two", alignment);
		return fpl_null;
	}
	// Allocate empty memory to hold a size of a pointer + alignment padding + the actual data
	size_t newSize = sizeof(void *) + (alignment << 1) + size;
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
	fplAssert(fplIsAligned(alignedPtr, alignment));
	return(alignedPtr);
}

fpl_common_api void fplMemoryAlignedFree(void *ptr) {
	FPL__CheckArgumentNullNoRet(ptr);
	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = *(void **)((void *)((uint8_t *)ptr - sizeof(void *)));
	fplAssert(basePtr != fpl_null);
	fplMemoryFree(basePtr);
}

#define FPL__MEM_SHIFT_64 3
#define FPL__MEM_MASK_64 0x00000007
#define FPL__MEM_SHIFT_32 2
#define FPL__MEM_MASK_32 0x00000003
#define FPL__MEM_SHIFT_16 1
#define FPL__MEM_MASK_16 0x0000000

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
			while (dataBlock < dataBlockEnd) { \
				*dataBlock++ = setValue; \
				setBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + setBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 < data8End) { \
			*data8++ = value; \
		} \
	} while (0);

#define FPL__MEMORY_CLEAR(T, memory, size, shift, mask) \
	do { \
		size_t clearBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = (T *)(memory); \
			T *dataBlockEnd = (T *)(memory) + (size >> shift); \
			while (dataBlock < dataBlockEnd) { \
				*dataBlock++ = 0; \
				clearBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + clearBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 < data8End) { \
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
			while (sourceDataBlock < sourceDataBlockEnd) { \
				*destDataBlock++ = *sourceDataBlock++; \
				copiedBytes += sizeof(T); \
			} \
		} \
		const uint8_t *sourceData8 = (const uint8_t *)source + copiedBytes; \
        const uint8_t *sourceData8End = (const uint8_t *)source + sourceSize; \
        uint8_t *destData8 = (uint8_t *)dest + copiedBytes; \
        while (sourceData8 < sourceData8End) { \
            *destData8++ = *sourceData8++; \
        } \
	} while (0);

fpl_common_api void fplMemorySet(void *mem, const uint8_t value, const size_t size) {
	FPL__CheckArgumentNullNoRet(mem);
	FPL__CheckArgumentZeroNoRet(size);
#if defined(FPL__ENABLE_MEMORY_MACROS)
	if (size % 8 == 0) {
		FPL__MEMORY_SET(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64, value);
	} else if (size % 4 == 0) {
		FPL__MEMORY_SET(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32, value);
	} else if (size % 2 == 0) {
		FPL__MEMORY_SET(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16, value);
	} else {
		FPL__MEMORY_SET(uint8_t, mem, size, 0, 0, value);
	}
#elif defined(FPL_PLATFORM_WINDOWS)
	FillMemory(mem, size, value);
#else
	memset(mem, value, size);
#endif
}

fpl_common_api void fplMemoryClear(void *mem, const size_t size) {
	FPL__CheckArgumentNullNoRet(mem);
	FPL__CheckArgumentZeroNoRet(size);
#if defined(FPL__ENABLE_MEMORY_MACROS)
	if (size % 8 == 0) {
		FPL__MEMORY_CLEAR(uint64_t, mem, size, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if (size % 4 == 0) {
		FPL__MEMORY_CLEAR(uint32_t, mem, size, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if (size % 2 == 0) {
		FPL__MEMORY_CLEAR(uint16_t, mem, size, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16);
	} else {
		FPL__MEMORY_CLEAR(uint8_t, mem, size, 0, 0);
	}
#elif defined(FPL_PLATFORM_WINDOWS)
	ZeroMemory(mem, size);
#else
	memset(mem, 0, size);
#endif
}

fpl_common_api void fplMemoryCopy(const void *sourceMem, const size_t sourceSize, void *targetMem) {
	FPL__CheckArgumentNullNoRet(sourceMem);
	FPL__CheckArgumentZeroNoRet(sourceSize);
	FPL__CheckArgumentNullNoRet(targetMem);
#if defined(FPL__ENABLE_MEMORY_MACROS)
	if (sourceSize % 8 == 0) {
		FPL__MEMORY_COPY(uint64_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_64, FPL__MEM_MASK_64);
	} else if (sourceSize % 4 == 0) {
		FPL__MEMORY_COPY(uint32_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_32, FPL__MEM_MASK_32);
	} else if (sourceSize % 2 == 0) {
		FPL__MEMORY_COPY(uint16_t, sourceMem, sourceSize, targetMem, FPL__MEM_SHIFT_16, FPL__MEM_MASK_16);
	} else {
		FPL__MEMORY_COPY(uint8_t, sourceMem, sourceSize, targetMem, 0, 0);
	}
#elif defined(FPL_PLATFORM_WINDOWS)
	CopyMemory(targetMem, sourceMem, sourceSize);
#else
	memcpy(targetMem, sourceMem, sourceSize);
#endif
}
#endif // FPL__COMMON_MEMORY_DEFINED

//
// Common Hardware
//
// https://github.com/google/cpu_features

//
// X86/X64 only (CPUID, XCR0, RDTSC)
//
#if defined(FPL_ARCH_X64) || defined(FPL_ARCH_X86)

#	define FPL__CPU_BRAND_BUFFER_SIZE 0x40

#	if defined(FPL_COMPILER_MSVC)

		// CPUID/XCR0 for MSVC
#		if _MSC_VER >= 1400
#			define fpl__m_CPUID(outLeaf, functionId) __cpuid((int *)(outLeaf)->raw, (int)(functionId))
#		endif
#		if _MSC_VER >= 1600
#			define fpl__m_GetXCR0() ((uint64_t)_xgetbv(0))
#		endif

		// RDTSC for MSVC
#		define fpl__m_RDTSC() ((uint64_t)__rdtsc())

#	elif defined(FPL_COMPILER_GCC) ||defined(FPL_COMPILER_CLANG)

		// CPUID for GCC/CLANG
fpl_internal void fpl__m_CPUID(fplCPUIDLeaf *outLeaf, const uint32_t functionId) {
	int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__cpuid_count(functionId, 0, eax, ebx, ecx, edx);
	outLeaf->eax = eax;
	outLeaf->ebx = ebx;
	outLeaf->ecx = ecx;
	outLeaf->edx = edx;
}

		// XCR0 for GCC/CLANG
fpl_internal uint64_t fpl__m_GetXCR0(void) {
	uint32_t eax, edx;
	__asm(".byte 0x0F, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(0));
	return eax;
}

		// RDTSC for non-MSVC
#		if defined(FPL_ARCH_X86)
fpl_force_inline uint64_t fpl__m_RDTSC(void) {
	unsigned long long int result;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (result));
	return((uint64_t)result);
}
#		elif defined(FPL_ARCH_X64)
fpl_force_inline uint64_t fpl__m_RDTSC(void) {
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	uint64_t result = (uint64_t)(((unsigned long long)lo) | (((unsigned long long)hi) << 32));
	return (result);
}
#		endif
#	endif

fpl_common_api void fplCPUID(fplCPUIDLeaf *outLeaf, const uint32_t functionId) {
#if defined(fpl__m_CPUID)
	fpl__m_CPUID(outLeaf, functionId);
#endif
}

fpl_common_api uint64_t fplCPUXCR0() {
#if defined(fpl__m_GetXCR0)
	uint64_t result = fpl__m_GetXCR0();
	return(result);
#else
	return(0ULL);
#endif
}

fpl_common_api uint64_t fplCPURDTSC() {
#if defined(fpl__m_RDTSC)
	uint64_t result = fpl__m_RDTSC();
	return(result);
#else
	return(0ULL);
#endif
}

fpl_common_api bool fplCPUGetCapabilities(fplCPUCapabilities *outCaps) {
	fplClearStruct(outCaps);

	fplCPUIDLeaf info0 = fplZeroInit;
	fplCPUIDLeaf info1 = fplZeroInit;
	fplCPUIDLeaf info7 = fplZeroInit;

	fplCPUID(&info0, 0);
	uint32_t maxFunctionId = info0.eax;

	if (1 <= maxFunctionId) {
		fplCPUID(&info1, 1);
	}

	if (7 <= maxFunctionId) {
		fplCPUID(&info7, 7);
	}

	bool hasXSave = fplIsBitSet(info1.ecx, 26) && fplIsBitSet(info1.ecx, 27);
	uint64_t xcr0 = hasXSave ? fplCPUXCR0() : 0;

	const uint32_t MASK_XMM = 0x2;
	const uint32_t MASK_YMM = 0x4;
	const uint32_t MASK_MASKREG = 0x20;
	const uint32_t MASK_ZMM0_15 = 0x40;
	const uint32_t MASK_ZMM16_31 = 0x80;

	const uint32_t MASK_SSE = MASK_XMM;
	const uint32_t MASK_AVX = MASK_XMM | MASK_YMM;
	const uint32_t MASK_AVX_512 = MASK_XMM | MASK_YMM | MASK_MASKREG | MASK_ZMM0_15 | MASK_ZMM16_31;

	bool hasSSESupport = (xcr0 & MASK_SSE) == MASK_SSE;
	bool hasAVXSupport = (xcr0 & MASK_AVX) == MASK_AVX;
	bool hasAVX512Support = (xcr0 & MASK_AVX_512) == MASK_AVX_512;

	outCaps->hasMMX = fplIsBitSet(info1.edx, 23);

	if (hasSSESupport) {
		outCaps->hasSSE = fplIsBitSet(info1.edx, 25);
		outCaps->hasSSE2 = fplIsBitSet(info1.edx, 26);
		outCaps->hasSSE3 = fplIsBitSet(info1.ecx, 0);
		outCaps->hasSSSE3 = fplIsBitSet(info1.ecx, 9);
		outCaps->hasSSE4_1 = fplIsBitSet(info1.ecx, 19);
		outCaps->hasSSE4_2 = fplIsBitSet(info1.ecx, 20);
	}

	if (hasAVXSupport) {
		outCaps->hasAVX = fplIsBitSet(info1.ecx, 28);
		outCaps->hasAVX2 = fplIsBitSet(info7.ebx, 5);
	}

	if (hasAVX512Support) {
		outCaps->hasAVX512 = fplIsBitSet(info7.ebx, 16);
		outCaps->hasFMA3 = fplIsBitSet(info7.ecx, 12);
	}

	return(true);
}

fpl_common_api size_t fplCPUGetName(char *destBuffer, const size_t maxDestBufferLen) {
	fplCPUIDLeaf cpuInfo = fplZeroInit;
	fplCPUID(&cpuInfo, 0x80000000);
	uint32_t extendedIds = cpuInfo.eax;

	// Get the information associated with each extended ID. Interpret CPU brand string.
	char cpuBrandBuffer[FPL__CPU_BRAND_BUFFER_SIZE] = fplZeroInit;
	uint32_t max = fplMin(extendedIds, 0x80000004);
	for (uint32_t i = 0x80000002; i <= max; ++i) {
		fplCPUID(&cpuInfo, i);
		uint32_t offset = (i - 0x80000002) << 4;
		fplMemoryCopy(cpuInfo.raw, sizeof(cpuInfo), cpuBrandBuffer + offset);
	}

	size_t result = fplGetStringLength(cpuBrandBuffer);
	if (destBuffer != fpl_null) {
		// Copy result back to the dest buffer
		size_t requiredDestBufferLen = result + 1;
		FPL__CheckArgumentMin(maxDestBufferLen, requiredDestBufferLen, 0);
		fplCopyStringLen(cpuBrandBuffer, result, destBuffer, maxDestBufferLen);
	}

	return(result);
}
#else

fpl_common_api uint64_t fplCPURDTSC() {
	// Based on: https://github.com/google/benchmark/blob/v1.1.0/src/cycleclock.h
#if defined(FPL_ARCH_ARM64)
	int64_t virtual_timer_value;
	fplAsm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
	return (uint64_t)virtual_timer_value;
#elif defined(FPL_ARCH_ARM32) && (__ARM_ARCH >= 6)
	uint32_t pmccntr;
	uint32_t pmuseren;
	uint32_t pmcntenset;
	// Read the user mode perf monitor counter access permissions.
	fplAsm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
	if (pmuseren & 1) {
		// Allows reading perfmon counters for user mode code.
		fplAsm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
		if (pmcntenset & 0x80000000ul) {
			// Is it counting?
			fplAsm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
			// The counter is set up to count every 64th cycle
			return (uint64_t)pmccntr * 64ULL;  // Should optimize to << 6
		}
	}
#endif
	struct timeval tv;
	gettimeofday(&tv, fpl_null);
	return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}

fpl_common_api uint64_t fplCPUXCR0() {
	// Not supported on non-x86 platforms
	return(0);
}

fpl_common_api void fplCPUID(fplCPUIDLeaf *outLeaf, const uint32_t functionId) {
	// Not supported on non-x86 platforms
	fplClearStruct(outLeaf);
}

fpl_common_api bool fplCPUGetCapabilities(fplCPUCapabilities *outCaps) {
	// @IMPLEMENT(final): fplCPUGetCapabilities for non-x86 architectures
	return(false);
}

fpl_common_api size_t fplCPUGetName(char *destBuffer, const size_t maxDestBufferLen) {
	// @IMPLEMENT(final): fplCPUGetName for non-x86 architectures
	return(0);
}
#endif

//
// Common Atomics
//
#if !defined(FPL__COMMON_ATOMICS_DEFINED)
#define FPL__COMMON_ATOMICS_DEFINED

fpl_common_api size_t fplAtomicFetchAndAddSize(volatile size_t *dest, const size_t addend) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicFetchAndAddU64((volatile uint64_t *)dest, (uint64_t)addend);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicFetchAndAddU32((volatile uint32_t *)dest, (uint32_t)addend);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicFetchAndAddPtr(volatile void **dest, const intptr_t addend) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicFetchAndAddS64((volatile int64_t *)dest, (int64_t)addend);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicFetchAndAddS32((volatile int32_t *)dest, (int32_t)addend);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicAddAndFetchSize(volatile size_t *value, const size_t addend) {
	fplAssert(value != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicAddAndFetchU64((volatile uint64_t *)value, (uint64_t)addend);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicAddAndFetchU32((volatile uint32_t *)value, (uint32_t)addend);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicAddAndFetchPtr(volatile void **value, const intptr_t addend) {
	fplAssert(value != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicAddAndFetchU64((volatile uint64_t *)value, (uint64_t)addend);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicAddAndFetchU32((volatile uint32_t *)value, (uint32_t)addend);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicIncrementSize(volatile size_t *value) {
	fplAssert(value != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicIncrementU64((volatile uint64_t *)value);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicIncrementU32((volatile uint32_t *)value);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api void *fplAtomicIncrementPtr(volatile void **value) {
	fplAssert(value != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicAddAndFetchU64((volatile uint64_t *)value, 8);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicAddAndFetchU32((volatile uint32_t *)value, 4);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicExchangeSize(volatile size_t *target, const size_t value) {
	fplAssert(target != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return (result);
}
fpl_common_api void *fplAtomicExchangePtr(volatile void **target, const void *value) {
	fplAssert(target != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicExchangeU64((volatile uint64_t *)target, (uint64_t)value);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicExchangeU32((volatile uint32_t *)target, (uint32_t)value);
#else
#	error "Unsupported architecture/platform!"
#endif  // FPL_ARCH
	return (result);
}

fpl_common_api size_t fplAtomicCompareAndSwapSize(volatile size_t *dest, const size_t comparand, const size_t exchange) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	size_t result = (size_t)fplAtomicCompareAndSwapU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	size_t result = (size_t)fplAtomicCompareAndSwapU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}
fpl_common_api void *fplAtomicCompareAndSwapPtr(volatile void **dest, const void *comparand, const void *exchange) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	void *result = (void *)fplAtomicCompareAndSwapU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	void *result = (void *)fplAtomicCompareAndSwapU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}

fpl_common_api bool fplAtomicIsCompareAndSwapSize(volatile size_t *dest, const size_t comparand, const size_t exchange) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	bool result = fplAtomicIsCompareAndSwapU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	bool result = fplAtomicIsCompareAndSwapU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
}
fpl_common_api bool fplAtomicIsCompareAndSwapPtr(volatile void **dest, const void *comparand, const void *exchange) {
	fplAssert(dest != fpl_null);
#if defined(FPL_CPU_64BIT)
	bool result = fplAtomicIsCompareAndSwapU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#elif defined(FPL_CPU_32BIT)
	bool result = fplAtomicIsCompareAndSwapU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#else
#	error "Unsupported architecture/platform!"
#endif // FPL_ARCH
	return (result);
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

#endif // FPL__COMMON_ATOMICS_DEFINED

//
// Common Threading
//
fpl_common_api fplThreadState fplGetThreadState(fplThreadHandle *thread) {
	if (thread == fpl_null) {
		return fplThreadState_Stopped;
	}
	fplThreadState result = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&thread->currentState);
	return(result);
}

fpl_common_api const fplThreadHandle *fplGetMainThread() {
	const fplThreadHandle *result = &fpl__global__ThreadState.mainThread;
	return(result);
}

fpl_common_api size_t GetAvailableThreadCount() {
	size_t result = 0;
	for (size_t threadIndex = 0; threadIndex < FPL_MAX_THREAD_COUNT; ++threadIndex) {
		fplThreadState state = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&fpl__global__ThreadState.threads[threadIndex].currentState);
		if (state == fplThreadState_Stopped) {
			++result;
		}
	}
	return(result);
}

fpl_common_api size_t GetUsedThreadCount() {
	size_t result = 0;
	for (size_t threadIndex = 0; threadIndex < FPL_MAX_THREAD_COUNT; ++threadIndex) {
		fplThreadState state = (fplThreadState)fplAtomicLoadU32((volatile uint32_t *)&fpl__global__ThreadState.threads[threadIndex].currentState);
		if (state != fplThreadState_Stopped) {
			++result;
		}
	}
	return(result);
}

//
// Common Files
//
#if !defined(FPL__COMMON_FILES_DEFINED)
#define FPL__COMMON_FILES_DEFINED

fpl_common_api size_t fplFileReadBlock(const fplFileHandle *fileHandle, const size_t sizeToRead, void *targetBuffer, const size_t maxTargetBufferSize) {
#if defined(FPL_CPU_64BIT)
	return fplFileReadBlock64(fileHandle, sizeToRead, targetBuffer, maxTargetBufferSize);
#else
	return fplFileReadBlock32(fileHandle, (uint32_t)sizeToRead, targetBuffer, (uint32_t)maxTargetBufferSize);
#endif
}

fpl_common_api size_t fplFileWriteBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize) {
#if defined(FPL_CPU_64BIT)
	return fplFileWriteBlock64(fileHandle, sourceBuffer, sourceSize);
#else
	return fplFileWriteBlock32(fileHandle, sourceBuffer, (uint32_t)sourceSize);
#endif
}

fpl_common_api size_t fplFileSetPosition(const fplFileHandle *fileHandle, const intptr_t position, const fplFilePositionMode mode) {
#if defined(FPL_CPU_64BIT)
	return fplFileSetPosition64(fileHandle, position, mode);
#else
	return fplFileSetPosition32(fileHandle, (int32_t)position, mode);
#endif
}

fpl_common_api size_t fplFileGetPosition(const fplFileHandle *fileHandle) {
#if defined(FPL_CPU_64BIT)
	return fplFileGetPosition64(fileHandle);
#else
	return fplFileGetPosition32(fileHandle);
#endif
}

fpl_common_api size_t fplFileGetSizeFromPath(const char *filePath) {
#if defined(FPL_CPU_64BIT)
	return fplFileGetSizeFromPath64(filePath);
#else
	return fplFileGetSizeFromPath32(filePath);
#endif
}

fpl_common_api size_t fplFileGetSizeFromHandle(const fplFileHandle *fileHandle) {
#if defined(FPL_CPU_64BIT)
	return fplFileGetSizeFromHandle64(fileHandle);
#else
	return fplFileGetSizeFromHandle32(fileHandle);
#endif
}

#endif // FPL__COMMON_FILES_DEFINED

//
// Common Paths
//
#if !defined(FPL__COMMON_PATHS_DEFINED)
#define FPL__COMMON_PATHS_DEFINED

fpl_common_api size_t fplExtractFilePath(const char *sourcePath, char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(sourcePath, 0);
	size_t sourceLen = fplGetStringLength(sourcePath);
	size_t result = 0;
	if (sourceLen > 0) {
		int pathLen = 0;
		const char *chPtr = (const char *)sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				pathLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		result = pathLen;
		if (destPath != fpl_null) {
			size_t requiredDestLen = pathLen + 1;
			FPL__CheckArgumentMin(maxDestLen, requiredDestLen, 0);
			fplCopyStringLen(sourcePath, pathLen, destPath, maxDestLen);
		}
	}
	return(result);
}

fpl_common_api const char *fplExtractFileExtension(const char *sourcePath) {
	const char *result = fpl_null;
	if (sourcePath != fpl_null) {
		const char *chPtr = sourcePath;
		// Find last separator first
		const char *lastPathSep = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				lastPathSep = chPtr;
			}
			++chPtr;
		}
		// Start either by the last found separator or from the very start
		if (lastPathSep != fpl_null) {
			chPtr = lastPathSep;
		} else {
			chPtr = sourcePath;
		}
		const char *lastExt = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
				lastExt = chPtr;
			}
			++chPtr;
		}
		if (lastExt != fpl_null) {
			result = lastExt;
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
			if (*chPtr == FPL_PATH_SEPARATOR) {
				result = chPtr + 1;
			}
			++chPtr;
		}
	}
	return(result);
}

fpl_common_api size_t fplChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const size_t maxDestLen) {
	FPL__CheckArgumentNull(filePath, 0);
	FPL__CheckArgumentNull(newFileExtension, 0);

	size_t filePathLen = fplGetStringLength(filePath);
	FPL__CheckArgumentZero(filePathLen, 0);

	size_t extLen = fplGetStringLength(newFileExtension);

	size_t result = 0;
	if (filePath != fpl_null) {
		// Find last path
		const char *chPtr = (const char *)filePath;
		const char *lastPathSeparatorPtr = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				lastPathSeparatorPtr = chPtr;
			}
			++chPtr;
		}
		// Find last ext separator
		if (lastPathSeparatorPtr != fpl_null) {
			chPtr = lastPathSeparatorPtr + 1;
		} else {
			chPtr = (const char *)filePath;
		}
		const char *lastExtSeparatorPtr = fpl_null;
		while (*chPtr) {
			if (*chPtr == FPL_FILE_EXT_SEPARATOR) {
				lastExtSeparatorPtr = chPtr;
			}
			++chPtr;
		}

		size_t filenameLen;
		if (lastExtSeparatorPtr != fpl_null) {
			filenameLen = (size_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
		} else {
			filenameLen = filePathLen;
		}

		result = filenameLen + extLen;

		// Copy parts
		if (destPath != fpl_null) {
			size_t requiredDestLen = result + 1;
			FPL__CheckArgumentMin(maxDestLen, requiredDestLen, 0);

			fplCopyStringLen(filePath, filenameLen, destPath, maxDestLen);
			char *destExtPtr = destPath + filenameLen;
			fplCopyStringLen(newFileExtension, extLen, destExtPtr, maxDestLen - filenameLen);
		}
	}
	return(result);
}

fpl_common_api size_t fplPathCombine(char *destPath, const size_t maxDestPathLen, const size_t pathCount, ...) {
	FPL__CheckArgumentZero(pathCount, 0);

	size_t result = 0;

	size_t curDestPosition = 0;
	char *currentDestPtr = destPath;

	va_list vargs;
	va_start(vargs, pathCount);
	for (size_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
		const char *path = va_arg(vargs, const char *);
		size_t pathLen = fplGetStringLength(path);

		bool requireSeparator = pathIndex < (pathCount - 1);
		size_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;

		result += requiredPathLen;

		if (destPath != fpl_null) {
			size_t requiredDestLen = result + 1;
			FPL__CheckArgumentMin(maxDestPathLen, requiredDestLen, 0);

			fplCopyStringLen(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
			currentDestPtr += pathLen;
			if (requireSeparator) {
				*currentDestPtr++ = FPL_PATH_SEPARATOR;
			}
			curDestPosition += requiredPathLen;
		}
	}
	if (currentDestPtr != fpl_null) {
		*currentDestPtr = 0;
	}
	va_end(vargs);
	return(result);
}
#endif // FPL__COMMON_PATHS_DEFINED

//
// Common Window
//
#if !defined(FPL__COMMON_WINDOW_DEFINED)
#define FPL__COMMON_WINDOW_DEFINED
fpl_common_api char *fplGetWindowTitle(char *outTitle, const size_t maxOutTitleLength) {
	FPL__CheckPlatform(fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	char *result = fplCopyString(appState->currentSettings.window.title, outTitle, maxOutTitleLength);
	return(result);
}

fpl_common_api void fplSetWindowInputEvents(const bool enabled) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	appState->currentSettings.input.disabledEvents = !enabled;
}
#endif // FPL__COMMON_WINDOW_DEFINED

//
// Common Logging
//
#if defined(FPL__ENABLE_LOGGING)
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
	if (errorState->count > 0) {
		size_t index = errorState->count - 1;
		result = fplGetErrorByIndex(index);
	}
	return (result);
}

fpl_common_api const char *fplGetErrorByIndex(const size_t index) {
	const char *result = "";
	const fpl__ErrorState *errorState = &fpl__global__LastErrorState;
	if (index < errorState->count) {
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
	fplClearStruct(errorState);
}

fpl_common_api const fplSettings *fplGetCurrentSettings() {
	FPL__CheckPlatform(fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	return &appState->currentSettings;
}

fpl_common_api void fplSetDefaultVideoSettings(fplVideoSettings *video) {
	FPL__CheckArgumentNullNoRet(video);
	fplClearStruct(video);
	video->isVSync = false;
	video->isAutoSize = true;

#if defined(FPL__ENABLE_VIDEO_OPENGL)
	video->graphics.opengl.libraryFile = fpl_null;
	video->graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#endif

#if defined(FPL__ENABLE_VIDEO_VULKAN)
	video->graphics.vulkan.libraryFile = fpl_null;
	video->graphics.vulkan.appVersion = fplStructInit(fplVersionInfo, "1.0.0", "1", "0", "0");
	video->graphics.vulkan.engineVersion = fplStructInit(fplVersionInfo, "1.0.0", "1", "0", "0");
	video->graphics.vulkan.apiVersion = fplStructInit(fplVersionInfo, "1.1.0", "1", "1", "0");
	video->graphics.vulkan.validationLayerMode = fplVulkanValidationLayerMode_Disabled;
#endif

	// @NOTE(final): Auto detect video backend
#if defined(FPL__ENABLE_VIDEO_OPENGL)
	video->backend = fplVideoBackendType_OpenGL;
#elif defined(FPL__ENABLE_VIDEO_SOFTWARE)
	video->backend = fplVideoBackendType_Software;
#elif defined(FPL__ENABLE_VIDEO_VULKAN)
	video->backend = fplVideoBackendType_Vulkan;
#else
	video->backend = fplVideoBackendType_None;
#endif
}

fpl_common_api void fplSetDefaultAudioTargetFormat(fplAudioTargetFormat *targetFormat) {
	FPL__CheckArgumentNullNoRet(targetFormat);
	fplClearStruct(targetFormat);

#if defined(FPL__ENABLE_AUDIO)
	fplAudioTargetFormat emptyFormat = fplZeroInit;
	fplAudioDeviceFormat deviceFormat = fplZeroInit;
	fplConvertAudioTargetFormatToDeviceFormat(&emptyFormat, &deviceFormat);

	targetFormat->preferExclusiveMode = deviceFormat.preferExclusiveMode;
	targetFormat->channels = deviceFormat.channels;
	targetFormat->sampleRate = deviceFormat.sampleRate;
	targetFormat->periods = deviceFormat.periods;
	targetFormat->type = deviceFormat.type;
	targetFormat->bufferSizeInFrames = deviceFormat.bufferSizeInFrames;
#endif // FPL__ENABLE_AUDIO
}

fpl_common_api void fplSetDefaultAudioSettings(fplAudioSettings *audio) {
	FPL__CheckArgumentNullNoRet(audio);
	fplClearStruct(audio);
	fplSetDefaultAudioTargetFormat(&audio->targetFormat);

	audio->backend = fplAudioBackendType_None;
#	if defined(FPL_PLATFORM_WINDOWS) && defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
	audio->backend = fplAudioBackendType_DirectSound;
#	endif
#	if defined(FPL_PLATFORM_LINUX) && defined(FPL__ENABLE_AUDIO_ALSA)
	audio->backend = fplAudioBackendType_Alsa;
#	endif

	audio->startAuto = true;
	audio->stopAuto = true;
}

fpl_common_api void fplSetDefaultWindowSettings(fplWindowSettings *window) {
	FPL__CheckArgumentNullNoRet(window);
	fplClearStruct(window);
	window->title[0] = 0;
	window->windowSize.width = 0;
	window->windowSize.height = 0;
	window->fullscreenSize.width = 0;
	window->fullscreenSize.height = 0;
	window->isFullscreen = false;
	window->isResizable = true;
	window->isDecorated = true;
	window->isFloating = false;
	window->isScreenSaverPrevented = false;
	window->isMonitorPowerPrevented = false;
}

fpl_common_api void fplSetDefaultConsoleSettings(fplConsoleSettings *console) {
	FPL__CheckArgumentNullNoRet(console);
	fplClearStruct(console);
	console->title[0] = 0;
}

fpl_common_api void fplSetDefaultInputSettings(fplInputSettings *input) {
	FPL__CheckArgumentNullNoRet(input);
	fplClearStruct(input);
	input->controllerDetectionFrequency = 100;
}

fpl_common_api void fplSetDefaultSettings(fplSettings *settings) {
	FPL__CheckArgumentNullNoRet(settings);
	fplClearStruct(settings);
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

#define FPL__PLATFORMRESULTTYPE_COUNT FPL__ENUM_COUNT(fplPlatformResultType_First, fplPlatformResultType_Last)
fpl_globalvar const char *fpl__global_platformResultTypeNameTable[] = {
	"Failed Window", // fplPlatformResultType_FailedWindow (-6)
	"Failed Audio", // fplPlatformResultType_FailedAudio (-5)
	"Failed Video", // fplPlatformResultType_FailedVideo (-4)
	"Failed Platform", // fplPlatformResultType_FailedPlatform (-3)
	"Failed Allocating Memory", // fplPlatformResultType_FailedAllocatingMemory (-2)
	"Already Initialized", // fplPlatformResultType_AlreadyInitialized (-1)
	"Not Initialized", // fplPlatformResultType_NotInitialized (0)
	"Success", // fplPlatformResultType_Success (1)
};
fplStaticAssert(fplArrayCount(fpl__global_platformResultTypeNameTable) == FPL__PLATFORMRESULTTYPE_COUNT);

fpl_common_api const char *fplPlatformGetResultName(const fplPlatformResultType type) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(type, fplPlatformResultType_First, fplPlatformResultType_Last);
	const char *result = fpl__global_platformResultTypeNameTable[index];
	return(result);
}

#define FPL__ARCHTYPE_COUNT FPL__ENUM_COUNT(fplCPUArchType_First, fplCPUArchType_Last)
fpl_globalvar const char *fpl__global_ArchTypeNameTable[] = {
	"Unknown", // Unknown architecture
	"x86", // X86 architecture
	"x86_64", // X86 with 64-bit architecture
	"x64", // X64 only architecture
	"arm32", // ARM32 architecture
	"arm64", // ARM64 architecture
};
fplStaticAssert(fplArrayCount(fpl__global_ArchTypeNameTable) == FPL__ARCHTYPE_COUNT);

fpl_common_api const char *fplCPUGetArchName(const fplCPUArchType type) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(type, fplCPUArchType_First, fplCPUArchType_Last);
	const char *result = fpl__global_ArchTypeNameTable[index];
	return(result);
}

//
// Debug out
//
#if defined(FPL_PLATFORM_WINDOWS)
fpl_platform_api void fplDebugOut(const char *text) {
	wchar_t buffer[FPL_MAX_BUFFER_LENGTH];
	fplUTF8StringToWideString(text, fplGetStringLength(text), buffer, fplArrayCount(buffer));
	OutputDebugStringW(buffer);
}
#else
fpl_platform_api void fplDebugOut(const char *text) {
	fplConsoleOut(text);
}
#endif

fpl_common_api void fplDebugFormatOut(const char *format, ...) {
	if (format != fpl_null) {
		char buffer[FPL_MAX_BUFFER_LENGTH];
		va_list argList;
		va_start(argList, format);
		fplStringFormatArgs(buffer, fplArrayCount(buffer), format, argList);
		va_end(argList);
		fplDebugOut(buffer);
	}
}
#endif // FPL__COMMON_DEFINED

// ############################################################################
//
// > WIN32_PLATFORM (Win32, Win64)
//
// ############################################################################
#if defined(FPL_PLATFORM_WINDOWS)

#	if defined(FPL_ARCH_X86)
#		define FPL_MEMORY_BARRIER() \
			LONG barrier; \
			_InterlockedOr(&barrier, 0);
#	elif defined(FPL_ARCH_X64)
		// @NOTE(final): No need for hardware memory fence on X64 because the hardware guarantees memory order always.
#		define FPL_MEMORY_BARRIER()
#	endif

#if defined(FPL__ENABLE_WINDOW)

fpl_internal DWORD fpl__Win32MakeWindowStyle(const fplWindowSettings *settings) {
	DWORD result = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (settings->isFullscreen || !settings->isDecorated) {
		result |= WS_POPUP;
	} else {
		result |= WS_OVERLAPPEDWINDOW;
		if (!settings->isResizable) {
			result &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
		}
	}
	return(result);
}

fpl_internal DWORD fpl__Win32MakeWindowExStyle(const fplWindowSettings *settings) {
	DWORD result = WS_EX_APPWINDOW;
	if (settings->isFullscreen || settings->isFloating) {
		result |= WS_EX_TOPMOST;
	}
	return(result);
}

fpl_internal void fpl__Win32SaveWindowState(const fpl__Win32Api *wapi, fpl__Win32LastWindowInfo *target, HWND windowHandle) {
	target->isMaximized = !!wapi->user.IsZoomed(windowHandle);
	target->isMinimized = !!wapi->user.IsIconic(windowHandle);
	target->style = fpl__win32_GetWindowLong(windowHandle, GWL_STYLE);
	target->exStyle = fpl__win32_GetWindowLong(windowHandle, GWL_EXSTYLE);
	wapi->user.GetWindowPlacement(windowHandle, &target->placement);
}

fpl_internal void fpl__Win32RestoreWindowState(const fpl__Win32Api *wapi, const fpl__Win32LastWindowInfo *target, HWND windowHandle) {
	fplAssert(target->style > 0 && target->exStyle > 0);
	fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, target->style);
	fpl__win32_SetWindowLong(windowHandle, GWL_EXSTYLE, target->exStyle);
	wapi->user.SetWindowPlacement(windowHandle, &target->placement);
	wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	if (target->isMaximized) {
		wapi->user.SendMessageW(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	} else if (target->isMinimized) {
		wapi->user.SendMessageW(windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
}

fpl_internal bool fpl__Win32LeaveFullscreen() {
	const fpl__PlatformAppState *platState = fpl__global__AppState;
	fplAssert(platState != fpl_null);
	const fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	const fpl__Win32WindowState *win32Window = &platState->window.win32;
	const fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;
	HWND windowHandle = win32Window->windowHandle;
	fpl__Win32RestoreWindowState(wapi, fullscreenInfo, windowHandle);
	bool result;
	if (fullscreenInfo->wasResolutionChanged) {
		result = (wapi->user.ChangeDisplaySettingsW(fpl_null, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
	} else {
		result = true;
	}
	return(result);
}

fpl_internal bool fpl__Win32EnterFullscreen(const int32_t xpos, const int32_t ypos, const int32_t fullscreenWidth, const int32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits, const bool allowResolutionChange) {
	fpl__PlatformAppState *platState = fpl__global__AppState;
	fplAssert(platState != fpl_null);
	fpl__Win32AppState *win32State = &platState->win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	const fplWindowSettings *settings = &platState->currentSettings.window;
	fpl__Win32WindowState *win32Window = &platState->window.win32;
	fpl__Win32LastWindowInfo *fullscreenInfo = &win32Window->lastFullscreenInfo;

	HWND windowHandle = win32Window->windowHandle;
	HDC deviceContext = win32Window->deviceContext;

	fplAssert(fullscreenInfo->style > 0 && fullscreenInfo->exStyle > 0);
	fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, fullscreenInfo->style & ~(WS_CAPTION | WS_THICKFRAME));
	fpl__win32_SetWindowLong(windowHandle, GWL_EXSTYLE, fullscreenInfo->exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

	MONITORINFO monitor = fplZeroInit;
	monitor.cbSize = sizeof(monitor);
	wapi->user.GetMonitorInfoW(wapi->user.MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST), &monitor);

	bool result;
	if (allowResolutionChange && (fullscreenWidth > 0) && (fullscreenHeight > 0)) {
		int32_t useFullscreenWidth = fullscreenWidth;
		int32_t useFullscreenHeight = fullscreenHeight;

		DWORD useRefreshRate = refreshRate;
		if (!useRefreshRate) {
			useRefreshRate = wapi->gdi.GetDeviceCaps(deviceContext, VREFRESH);
		}

		DWORD useColourBits = colorBits;
		if (!useColourBits) {
			useColourBits = wapi->gdi.GetDeviceCaps(deviceContext, BITSPIXEL);
		}

		RECT windowRect;
		// @TODO(final/Win32): This may not be correct to assume 0, 0 as origin for the current display
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = windowRect.left + useFullscreenWidth;
		windowRect.bottom = windowRect.top + useFullscreenHeight;

		WINDOWPLACEMENT placement = fplZeroInit;
		placement.length = sizeof(placement);
		placement.rcNormalPosition = windowRect;
		placement.showCmd = SW_SHOW;
		wapi->user.SetWindowPlacement(windowHandle, &placement);
		wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		DEVMODEW fullscreenSettings = fplZeroInit;
		wapi->user.EnumDisplaySettingsW(fpl_null, 0, &fullscreenSettings);
		fullscreenSettings.dmPelsWidth = useFullscreenWidth;
		fullscreenSettings.dmPelsHeight = useFullscreenHeight;
		fullscreenSettings.dmBitsPerPel = useColourBits;
		fullscreenSettings.dmDisplayFrequency = useRefreshRate;
		fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		result = (wapi->user.ChangeDisplaySettingsW(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
		fullscreenInfo->wasResolutionChanged = true;
	} else {
		RECT windowRect = fplZeroInit;
		if ((xpos != INT32_MAX) && (ypos != INT32_MAX) && (fullscreenWidth != 0) && (fullscreenHeight != 0)) {
			windowRect.left = xpos;
			windowRect.top = ypos;
			windowRect.right = xpos + fullscreenWidth;
			windowRect.top = ypos + fullscreenHeight;
		} else {
			windowRect = monitor.rcMonitor;
		}
		WINDOWPLACEMENT placement = fplZeroInit;
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

fpl_internal bool fpl__Win32SetWindowFullscreen(const bool value, const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t refreshRate, const bool allowResolutionChange) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32WindowState *windowState = &appState->window.win32;
	fplWindowSettings *windowSettings = &appState->currentSettings.window;
	fpl__Win32LastWindowInfo *fullscreenState = &windowState->lastFullscreenInfo;
	const fpl__Win32Api *wapi = &win32AppState->winApi;

	HWND windowHandle = windowState->windowHandle;

	// Save current window info if not already fullscreen
	if (!windowSettings->isFullscreen) {
		fpl__Win32SaveWindowState(wapi, fullscreenState, windowHandle);
		if (fullscreenState->isMaximized || fullscreenState->isMinimized) {
			wapi->user.ShowWindow(windowHandle, SW_RESTORE);
		}
	}

	if (value) {
		// Enter fullscreen mode or fallback to window mode
		windowSettings->isFullscreen = fpl__Win32EnterFullscreen(x, y, w, h, refreshRate, 0, allowResolutionChange);
		if (!windowSettings->isFullscreen) {
			fpl__Win32LeaveFullscreen();
		}
	} else {
		fpl__Win32LeaveFullscreen();
		windowSettings->isFullscreen = false;
	}
	bool result = windowSettings->isFullscreen != 0;
	return(result);
}

fpl_internal float fpl__Win32XInputProcessStickValue(const SHORT value, const SHORT deadZoneThreshold) {
	float result = 0;
	if (value < -deadZoneThreshold) {
		result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	} else if (value > deadZoneThreshold) {
		result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}
	return(result);
}

fpl_internal void fpl__Win32XInputGamepadToGamepadState(const XINPUT_GAMEPAD *newState, fplGamepadState *outState) {
	// If we got here, the controller is definitily by connected
	outState->isConnected = true;

	// Analog sticks
	outState->leftStickX = fpl__Win32XInputProcessStickValue(newState->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	outState->leftStickY = fpl__Win32XInputProcessStickValue(newState->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	outState->rightStickX = fpl__Win32XInputProcessStickValue(newState->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	outState->rightStickY = fpl__Win32XInputProcessStickValue(newState->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

	// Triggers
	outState->leftTrigger = (float)newState->bLeftTrigger / 255.0f;
	outState->rightTrigger = (float)newState->bRightTrigger / 255.0f;

	// Digital pad buttons
	if (newState->wButtons & XINPUT_GAMEPAD_DPAD_UP)
		outState->dpadUp.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		outState->dpadDown.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		outState->dpadLeft.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		outState->dpadRight.isDown = true;

	// Action buttons
	if (newState->wButtons & XINPUT_GAMEPAD_A)
		outState->actionA.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_B)
		outState->actionB.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_X)
		outState->actionX.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_Y)
		outState->actionY.isDown = true;

	// Center buttons
	if (newState->wButtons & XINPUT_GAMEPAD_START)
		outState->start.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_BACK)
		outState->back.isDown = true;

	// Shoulder buttons
	if (newState->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		outState->leftShoulder.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		outState->rightShoulder.isDown = true;

	// Thumb buttons
	if (newState->wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		outState->leftThumb.isDown = true;
	if (newState->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		outState->rightThumb.isDown = true;

	// The controller is only active, when any button or any movement happened
	outState->isActive = !fpl__IsZeroMemory(newState, sizeof(*newState));
}

fpl_internal void fpl__Win32UpdateGameControllers(const fplSettings *settings, const fpl__Win32InitState *initState, fpl__Win32XInputState *xinputState) {
	fplAssert(settings != fpl_null);
	fplAssert(xinputState != fpl_null);
	if (xinputState->xinputApi.XInputGetState != fpl_null) {
		//
		// Detect new controller (Only at a fixed frequency)
		//
		if (xinputState->lastDeviceSearchTime.QuadPart == 0) {
			QueryPerformanceCounter(&xinputState->lastDeviceSearchTime);
		}
		LARGE_INTEGER currentDeviceSearchTime, currentDeviceSearchFreq;
		QueryPerformanceCounter(&currentDeviceSearchTime);
		QueryPerformanceFrequency(&currentDeviceSearchFreq);
		uint64_t deviceSearchDifferenceTimeInMs = ((currentDeviceSearchTime.QuadPart - xinputState->lastDeviceSearchTime.QuadPart) / (currentDeviceSearchFreq.QuadPart / 1000));
		if ((settings->input.controllerDetectionFrequency == 0) || (deviceSearchDifferenceTimeInMs > settings->input.controllerDetectionFrequency)) {
			xinputState->lastDeviceSearchTime = currentDeviceSearchTime;
			for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
				XINPUT_STATE controllerState = fplZeroInit;
				if (xinputState->xinputApi.XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					if (!xinputState->isConnected[controllerIndex]) {
						// Connected
						xinputState->isConnected[controllerIndex] = true;
						fplStringFormat(xinputState->deviceNames[controllerIndex], fplArrayCount(xinputState->deviceNames[controllerIndex]), "XInput-Device [%d]", controllerIndex);

						fplEvent ev = fplZeroInit;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Connected;
						ev.gamepad.deviceIndex = controllerIndex;
						ev.gamepad.deviceName = xinputState->deviceNames[controllerIndex];
						fpl__PushInternalEvent(&ev);
					}
				} else {
					if (xinputState->isConnected[controllerIndex]) {
						// Disconnected
						xinputState->isConnected[controllerIndex] = false;

						fplEvent ev = fplZeroInit;
						ev.type = fplEventType_Gamepad;
						ev.gamepad.type = fplGamepadEventType_Disconnected;
						ev.gamepad.deviceIndex = controllerIndex;
						ev.gamepad.deviceName = xinputState->deviceNames[controllerIndex];
						fpl__PushInternalEvent(&ev);
					}
				}
			}
		}

		//
		// Update controller state when connected only
		//
		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
			if (xinputState->isConnected[controllerIndex]) {
				XINPUT_STATE controllerState = fplZeroInit;
				if (xinputState->xinputApi.XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					// State changed
					fplEvent ev = fplZeroInit;
					ev.type = fplEventType_Gamepad;
					ev.gamepad.type = fplGamepadEventType_StateChanged;
					ev.gamepad.deviceIndex = controllerIndex;
					ev.gamepad.deviceName = xinputState->deviceNames[controllerIndex];
					const XINPUT_GAMEPAD *newPadState = &controllerState.Gamepad;
					fpl__Win32XInputGamepadToGamepadState(newPadState, &ev.gamepad.state);
					ev.gamepad.state.deviceName = ev.gamepad.deviceName;
					fpl__PushInternalEvent(&ev);
				}
			}
		}
	}
}

fpl_internal bool fpl__Win32IsKeyDown(const fpl__Win32Api *wapi, const int virtualKey) {
	bool result = (wapi->user.GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	return(result);
}

fpl_internal bool fpl__Win32IsKeyActive(const fpl__Win32Api *wapi, const int virtualKey) {
	bool result = (wapi->user.GetKeyState(virtualKey) & 0x0001) != 0;
	return(result);
}

fpl_internal bool fpl__Win32IsCursorInWindow(const fpl__Win32Api *wapi, const fpl__Win32WindowState *win32Window) {
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

	wapi->user.ClientToScreen(win32Window->windowHandle, (LPPOINT)&area.left);
	wapi->user.ClientToScreen(win32Window->windowHandle, (LPPOINT)&area.right);
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
	// @NOTE(final): We use RAWINPUT to remove the mouse device entirely when it needs to be hidden
	if (!state) {
		const RAWINPUTDEVICE rid = fplStructInit(RAWINPUTDEVICE, 0x01, 0x02, 0, window->windowHandle);
		if (!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			FPL__ERROR(FPL__MODULE_WINDOW, "Failed register raw input mouse device for window handle '%p'", window->windowHandle);
		}
	} else {
		const RAWINPUTDEVICE rid = fplStructInit(RAWINPUTDEVICE, 0x01, 0x02, RIDEV_REMOVE, fpl_null);
		if (!wapi->user.RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			FPL__ERROR(FPL__MODULE_WINDOW, "Failed to unregister raw input mouse device");
		}
	}
	if (fpl__Win32IsCursorInWindow(wapi, window)) {
		fpl__Win32LoadCursor(wapi, window);
	}
}

fpl_internal void fpl__Win32ShowCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
	fpl__Win32SetCursorState(wapi, window, false);
}
fpl_internal void fpl__Win32HideCursor(const fpl__Win32Api *wapi, fpl__Win32WindowState *window) {
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
	if (lAltKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LAlt;
	}
	if (rAltKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RAlt;
	}
	if (lShiftKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LShift;
	}
	if (rShiftKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RShift;
	}
	if (lCtrlKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LCtrl;
	}
	if (rCtrlKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RCtrl;
	}
	if (lSuperKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_LSuper;
	}
	if (rSuperKeyIsDown) {
		modifiers |= fplKeyboardModifierFlags_RSuper;
	}
	if (capsLockActive) {
		modifiers |= fplKeyboardModifierFlags_CapsLock;
	}
	if (numLockActive) {
		modifiers |= fplKeyboardModifierFlags_NumLock;
	}
	if (scrollLockActive) {
		modifiers |= fplKeyboardModifierFlags_ScrollLock;
	}
	return(modifiers);
}

fpl_internal void fpl__Win32HandleMessage(const fpl__Win32Api *wapi, fpl__PlatformAppState *appState, fpl__Win32WindowState *windowState, MSG *msg) {
	if (appState->currentSettings.window.callbacks.eventCallback != fpl_null) {
		appState->currentSettings.window.callbacks.eventCallback(fplGetPlatformType(), windowState, &msg, appState->currentSettings.window.callbacks.eventUserData);
	}
	wapi->user.TranslateMessage(msg);
	wapi->user.DispatchMessageW(msg);
}

fpl_internal void CALLBACK fpl__Win32MessageFiberProc(struct fpl__PlatformAppState *appState) {
	fpl__Win32AppState *win32State = &appState->win32;
	fpl__Win32WindowState *windowState = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32State->winApi;
	wapi->user.SetTimer(appState->window.win32.windowHandle, 1, 1, 0);
	for (;;) {
		MSG message;
		while (wapi->user.PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
			fpl__Win32HandleMessage(wapi, appState, windowState, &message);
		}
		SwitchToFiber(appState->window.win32.mainFiber);
	}
}

LRESULT CALLBACK fpl__Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fplAssert(appState != fpl_null);

	fpl__Win32AppState *win32State = &appState->win32;
	fpl__Win32WindowState *win32Window = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32State->winApi;

	if (!win32Window->windowHandle) {
		return wapi->user.DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	LRESULT result = 0;
	switch (msg) {
		case WM_TIMER:
		{
			if (win32Window->mainFiber != fpl_null) {
				SwitchToFiber(win32Window->mainFiber);
			}
		} break;

		case WM_DESTROY:
		case WM_CLOSE:
		{
			appState->window.isRunning = false;
		} break;

		case WM_SIZE:
		{
			DWORD newWidth = LOWORD(lParam);
			DWORD newHeight = HIWORD(lParam);
			if (wParam == SIZE_MAXIMIZED) {
				fpl__PushWindowSizeEvent(fplWindowEventType_Maximized, newWidth, newHeight);
			} else if (wParam == SIZE_MINIMIZED) {
				fpl__PushWindowSizeEvent(fplWindowEventType_Minimized, newWidth, newHeight);
			} else if (wParam == SIZE_RESTORED) {
				fpl__PushWindowSizeEvent(fplWindowEventType_Restored, newWidth, newHeight);
			}

#			if defined(FPL__ENABLE_VIDEO_SOFTWARE)
			if (appState->currentSettings.video.backend == fplVideoBackendType_Software) {
				if (appState->initSettings.video.isAutoSize) {
					fplResizeVideoBackBuffer(newWidth, newHeight);
				}
			}
#			endif

			fpl__PushWindowSizeEvent(fplWindowEventType_Resized, newWidth, newHeight);

			return 0;
		} break;

		case WM_DROPFILES:
		{
			HDROP dropHandle = (HDROP)wParam;
			char fileBufferA[FPL_MAX_PATH_LENGTH];
			UINT fileCount;
			wchar_t fileBufferW[FPL_MAX_PATH_LENGTH];
			fileCount = wapi->shell.DragQueryFileW(dropHandle, 0xFFFFFFFF, fileBufferW, 0);
			if (fileCount > 0) {
				size_t filesTableSize = fileCount * sizeof(char **);
				size_t maxFileStride = FPL_MAX_PATH_LENGTH * 2 + 1;
				size_t filesMemorySize = filesTableSize + FPL__ARBITARY_PADDING + maxFileStride * fileCount;
				void *filesTableMemory = fpl__AllocateDynamicMemory(filesMemorySize, 16);
				char **filesTable = (char **)filesTableMemory;
				for (UINT fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
					filesTable[fileIndex] = (char *)((uint8_t *)filesTableMemory + filesTableSize + FPL__ARBITARY_PADDING + fileIndex * maxFileStride);
				}
				for (UINT fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
					char *file = filesTable[fileIndex];
					fileBufferW[0] = 0;
					UINT dragResult = wapi->shell.DragQueryFileW(dropHandle, 0, fileBufferW, fplArrayCount(fileBufferW));
					size_t sourceLen = lstrlenW(fileBufferW);
					fplWideStringToUTF8String(fileBufferW, sourceLen, file, maxFileStride);
				}
				fplMemoryBlock memory = fplZeroInit;
				memory.size = filesMemorySize;
				memory.base = filesTableMemory;
				fpl__PushWindowDropFilesEvent(fileBufferA, fileCount, (const char **)filesTable, &memory);
			}
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			if (!appState->currentSettings.input.disabledEvents) {
				uint64_t keyCode = wParam;
				bool isDown = (lParam & (1 << 31)) == 0;
				bool wasDown = (lParam & (1 << 30)) != 0;
				bool altKeyIsDown = fpl__Win32IsKeyDown(wapi, VK_MENU);
				fplButtonState keyState = isDown ? fplButtonState_Press : fplButtonState_Release;
				fplKeyboardModifierFlags modifiers = fpl__Win32GetKeyboardModifiers(wapi);
				fpl__HandleKeyboardButtonEvent(&appState->window, GetTickCount(), keyCode, modifiers, keyState, false);
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
			fpl__HandleKeyboardInputEvent(&appState->window, (uint64_t)wParam, (uint32_t)wParam);
			return 0;
		} break;

		case WM_ACTIVATE:
		{
		} break;

		case WM_MOUSEACTIVATE:
		{
			if (HIWORD(lParam) == WM_LBUTTONDOWN) {
				if (LOWORD(lParam) == HTCLOSE || LOWORD(lParam) == HTMINBUTTON || LOWORD(lParam) == HTMAXBUTTON) {
					win32Window->isFrameInteraction = true;
				}
			}
		} break;

		case WM_CAPTURECHANGED:
		{
			if (lParam == 0 && win32Window->isFrameInteraction) {
				if (!win32Window->isCursorActive) {
					fpl__Win32HideCursor(wapi, win32Window);
				}
				win32Window->isFrameInteraction = false;
			}
		} break;

		case WM_SETFOCUS:
		{
			fplEvent newEvent = fplZeroInit;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_GotFocus;
			fpl__PushInternalEvent(&newEvent);
			if (win32Window->isFrameInteraction) {
				break;
			}
			if (!win32Window->isCursorActive) {
				fpl__Win32HideCursor(wapi, win32Window);
			}
			return 0;
		} break;

		case WM_KILLFOCUS:
		{
			if (!win32Window->isCursorActive) {
				fpl__Win32ShowCursor(wapi, win32Window);
			}
			fplEvent newEvent = fplZeroInit;
			newEvent.type = fplEventType_Window;
			newEvent.window.type = fplWindowEventType_LostFocus;
			fpl__PushInternalEvent(&newEvent);
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
			if (!win32Window->isCursorActive) {
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
			if (!appState->currentSettings.input.disabledEvents) {
				fplButtonState buttonState;
				if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN) {
					buttonState = fplButtonState_Press;
				} else {
					buttonState = fplButtonState_Release;
				}
				if (buttonState == fplButtonState_Press) {
					wapi->user.SetCapture(hwnd);
				} else {
					wapi->user.ReleaseCapture();
				}
				fplMouseButtonType mouseButton;
				if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) {
					mouseButton = fplMouseButtonType_Left;
				} else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) {
					mouseButton = fplMouseButtonType_Right;
				} else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) {
					mouseButton = fplMouseButtonType_Middle;
				} else {
					mouseButton = fplMouseButtonType_None;
				}
				if (mouseButton != fplMouseButtonType_None) {
					int32_t mouseX = GET_X_LPARAM(lParam);
					int32_t mouseY = GET_Y_LPARAM(lParam);
					fpl__HandleMouseButtonEvent(&appState->window, mouseX, mouseY, mouseButton, buttonState);
				}
			}
		} break;
		case WM_MOUSEMOVE:
		{
			if (!appState->currentSettings.input.disabledEvents) {
				int32_t mouseX = GET_X_LPARAM(lParam);
				int32_t mouseY = GET_Y_LPARAM(lParam);
				fpl__HandleMouseMoveEvent(&appState->window, mouseX, mouseY);
			}
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
			if (LOWORD(lParam) == HTCLIENT) {
				fpl__Win32LoadCursor(wapi, win32Window);
				return TRUE;
			}
		} break;

		case WM_PAINT:
		{
			if (appState->currentSettings.window.callbacks.exposedCallback != fpl_null) {
				MSG msgData = fplZeroInit;
				msgData.message = msg;
				msgData.hwnd = hwnd;
				msgData.wParam = wParam;
				msgData.lParam = lParam;
				appState->currentSettings.window.callbacks.exposedCallback(fplGetPlatformType(), win32Window, &msgData, appState->currentSettings.window.callbacks.exposedUserData);
			} else {
				if (appState->currentSettings.video.backend == fplVideoBackendType_None) {
					PAINTSTRUCT ps;
					HDC hdc = wapi->user.BeginPaint(hwnd, &ps);
					wapi->user.EndPaint(hwnd, &ps);
					return(0);
				}
			}
		} break;

		case WM_ERASEBKGND:
		{
			// Prevent erasing of the background always, but only if a video backend is being used
			// NOTE(tspaete): Do not prevent the erasing of the backend for software video output, otherwise we will get weird flickering issue at startup
			if (appState->currentSettings.video.backend != fplVideoBackendType_None && appState->currentSettings.video.backend != fplVideoBackendType_Software) {
				return 1;
			}
		} break;

		case WM_SYSCOMMAND:
		{
			WPARAM masked = wParam & 0xFFF0;
			switch (masked) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER: {
					if (appState->currentSettings.window.isScreenSaverPrevented || appState->currentSettings.window.isMonitorPowerPrevented) {
						return 0;
					}
				} break;
			}
		} break;

		default:
			break;
	}
	result = wapi->user.DefWindowProcW(hwnd, msg, wParam, lParam);
	return (result);
}

fpl_internal HICON fpl__Win32LoadIconFromImageSource(const fpl__Win32Api *wapi, const HINSTANCE appInstance, const fplImageSource *imageSource) {
	fplAssert(imageSource != fpl_null);
	HICON result = 0;
	if (imageSource->width > 0 && imageSource->height > 0 && imageSource->data != fpl_null) {
		BITMAPV5HEADER bi = fplZeroInit;
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

		uint8_t *targetData = fpl_null;
		HDC dc = wapi->user.GetDC(fpl_null);
		HBITMAP colorBitmap = wapi->gdi.CreateDIBSection(dc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&targetData, fpl_null, (DWORD)0);
		if (colorBitmap == fpl_null) {
			FPL__ERROR(FPL__MODULE_WIN32, "Failed to create DIBSection from image with size %lu x %lu", imageSource->width, imageSource->height);
		}
		wapi->user.ReleaseDC(fpl_null, dc);

		HBITMAP maskBitmap = wapi->gdi.CreateBitmap(imageSource->width, imageSource->height, 1, 1, fpl_null);
		if (maskBitmap == fpl_null) {
			FPL__ERROR(FPL__MODULE_WIN32, "Failed to create Bitmap Mask from image with size %lu x %lu", imageSource->width, imageSource->height);
		}
		if (colorBitmap != fpl_null && maskBitmap != fpl_null) {
			fplAssert(targetData != fpl_null);
			uint8_t *dst = targetData;
			const uint8_t *src = imageSource->data;
			if (imageSource->type == fplImageType_RGBA) {
				for (uint32_t i = 0; i < imageSource->width * imageSource->height; ++i) {
					dst[0] = src[2]; // R > B
					dst[1] = src[1]; // G > G
					dst[2] = src[0]; // B > R
					dst[3] = src[3]; // A > A
					src += 4;
					dst += 4;
				}
				ICONINFO ii = fplZeroInit;
				ii.fIcon = TRUE;
				ii.xHotspot = 0;
				ii.yHotspot = 0;
				ii.hbmMask = maskBitmap;
				ii.hbmColor = colorBitmap;
				result = wapi->user.CreateIconIndirect(&ii);
			} else {
				FPL__ERROR(FPL__MODULE_WIN32, "Image source type '%d' for icon is not supported", imageSource->type);
			}
		}
		if (colorBitmap != fpl_null) {
			wapi->gdi.DeleteObject(colorBitmap);
		}
		if (maskBitmap != fpl_null) {
			wapi->gdi.DeleteObject(maskBitmap);
		}
	}
	if (result == 0) {
		result = fpl__win32_LoadIcon(appInstance, IDI_APPLICATION);
	}
	return(result);
}

fpl_internal bool fpl__Win32InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *platAppState, fpl__Win32AppState *appState, fpl__Win32WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	fplAssert(appState != fpl_null);
	const fpl__Win32Api *wapi = &appState->winApi;
	const fplWindowSettings *initWindowSettings = &initSettings->window;

	// Presetup window
	if (setupCallbacks->preSetup != fpl_null) {
		setupCallbacks->preSetup(platAppState, platAppState->initFlags, &platAppState->initSettings);
	}

	// Register window class
	WNDCLASSEXW windowClass = fplZeroInit;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.hInstance = GetModuleHandleA(fpl_null);

	// Set window background, either as system brush or custom brush which needs to be released when changed or the platform is released
	if (initWindowSettings->background.value == 0) {
		windowState->backgroundBrush = fpl_null;
		windowClass.hbrBackground = wapi->user.GetSysColorBrush(COLOR_BACKGROUND);
	} else {
		COLORREF brushColor = RGB(initWindowSettings->background.r, initWindowSettings->background.g, initWindowSettings->background.b);
		windowState->backgroundBrush = wapi->gdi.CreateSolidBrush(brushColor);
		windowClass.hbrBackground = windowState->backgroundBrush;
	}

	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hCursor = fpl__win32_LoadCursor(windowClass.hInstance, IDC_ARROW);
	windowClass.hIconSm = fpl__Win32LoadIconFromImageSource(wapi, windowClass.hInstance, &initWindowSettings->icons[0]);
	windowClass.hIcon = fpl__Win32LoadIconFromImageSource(wapi, windowClass.hInstance, &initWindowSettings->icons[1]);
	windowClass.lpszClassName = FPL__WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl__Win32MessageProc;
	windowClass.style |= CS_OWNDC;
	lstrcpynW(windowState->windowClass, windowClass.lpszClassName, fplArrayCount(windowState->windowClass));
	if (wapi->user.RegisterClassExW(&windowClass) == 0) {
		FPL__ERROR(FPL__MODULE_WINDOW, "Failed registering window class '%s'", windowState->windowClass);
		return false;
	}

	// Set window title
	wchar_t windowTitleBuffer[FPL_MAX_NAME_LENGTH];
	if (fplGetStringLength(initWindowSettings->title) > 0) {
		fplUTF8StringToWideString(initWindowSettings->title, fplGetStringLength(initWindowSettings->title), windowTitleBuffer, fplArrayCount(windowTitleBuffer));
	} else {
		const wchar_t *defaultTitle = FPL__WIN32_UNNAMED_WINDOW;
		lstrcpynW(windowTitleBuffer, defaultTitle, fplArrayCount(windowTitleBuffer));
	}
	wchar_t *windowTitle = windowTitleBuffer;
	fplWideStringToUTF8String(windowTitle, lstrlenW(windowTitle), currentWindowSettings->title, fplArrayCount(currentWindowSettings->title));

	// Create Fibers
	windowState->mainFiber = ConvertThreadToFiber(0);
	windowState->messageFiber = CreateFiber(0, (PFIBER_START_ROUTINE)fpl__Win32MessageFiberProc, platAppState);

	// Prepare window style, size and position
	DWORD style = fpl__Win32MakeWindowStyle(&initSettings->window);
	DWORD exStyle = fpl__Win32MakeWindowExStyle(&initSettings->window);
	if (initSettings->window.isResizable) {
		currentWindowSettings->isResizable = true;
	} else {
		currentWindowSettings->isResizable = false;
	}
	int windowX = CW_USEDEFAULT;
	int windowY = CW_USEDEFAULT;
	int windowWidth;
	int windowHeight;
	if ((initWindowSettings->windowSize.width > 0) &&
		(initWindowSettings->windowSize.height > 0)) {
		RECT windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = initWindowSettings->windowSize.width;
		windowRect.bottom = initWindowSettings->windowSize.height;
		wapi->user.AdjustWindowRect(&windowRect, style, false);
		windowWidth = windowRect.right - windowRect.left;
		windowHeight = windowRect.bottom - windowRect.top;
	} else {
		// @NOTE(final): Operating system decide how big the window should be.
		windowWidth = CW_USEDEFAULT;
		windowHeight = CW_USEDEFAULT;
	}

	// Create window
	windowState->windowHandle = wapi->user.CreateWindowExW(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, fpl_null, fpl_null, windowClass.hInstance, fpl_null);
	if (windowState->windowHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_WINDOW, "Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", windowState->windowClass, windowX, windowY, windowWidth, windowHeight);
		return false;
	}

	// Accept files as drag & drop source
	wapi->shell.DragAcceptFiles(windowState->windowHandle, TRUE);

	// Get actual window size and store results
	currentWindowSettings->windowSize.width = windowWidth;
	currentWindowSettings->windowSize.height = windowHeight;
	RECT clientRect;
	if (wapi->user.GetClientRect(windowState->windowHandle, &clientRect)) {
		currentWindowSettings->windowSize.width = clientRect.right - clientRect.left;
		currentWindowSettings->windowSize.height = clientRect.bottom - clientRect.top;
	}

	// Get device context so we can swap the back and front buffer
	windowState->deviceContext = wapi->user.GetDC(windowState->windowHandle);
	if (windowState->deviceContext == fpl_null) {
		FPL__ERROR(FPL__MODULE_WINDOW, "Failed aquiring device context from window '%d'", windowState->windowHandle);
		return false;
	}

	// Call post window setup callback
	if (setupCallbacks->postSetup != fpl_null) {
		setupCallbacks->postSetup(platAppState, platAppState->initFlags, initSettings);
	}

	// Enter fullscreen if needed
	if (initWindowSettings->isFullscreen) {
		fplSetWindowFullscreenSize(true, initWindowSettings->fullscreenSize.width, initWindowSettings->fullscreenSize.height, initWindowSettings->fullscreenRefreshRate);
	}

	// Show window
	wapi->user.ShowWindow(windowState->windowHandle, SW_SHOW);
	wapi->user.SetForegroundWindow(windowState->windowHandle);
	wapi->user.SetFocus(windowState->windowHandle);

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
		wapi->user.UnregisterClassW(windowState->windowClass, initState->appInstance);
	}
	if (windowState->backgroundBrush != fpl_null) {
		wapi->gdi.DeleteObject(windowState->backgroundBrush);
		windowState->backgroundBrush = fpl_null;
	}
	if (windowState->messageFiber != fpl_null) {
		DeleteFiber(windowState->messageFiber);
		windowState->messageFiber = fpl_null;
	}
	if (windowState->mainFiber != fpl_null) {
		ConvertFiberToThread();
		windowState->mainFiber = fpl_null;
	}
}

#endif // FPL__ENABLE_WINDOW

fpl_internal bool fpl__Win32ThreadWaitForMultiple(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout, const bool waitForAll) {
	FPL__CheckArgumentNull(threads, false);
	FPL__CheckArgumentMax(count, FPL_MAX_THREAD_COUNT, false);
	fplStaticAssert(FPL_MAX_THREAD_COUNT >= MAXIMUM_WAIT_OBJECTS);
	const size_t actualStride = stride > 0 ? stride : sizeof(fplThreadHandle *);
	for (size_t index = 0; index < count; ++index) {
		fplThreadHandle *thread = *(fplThreadHandle **)((uint8_t *)threads + index * actualStride);
		if (thread == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Thread for index '%d' are not allowed to be null", index);
			return false;
		}
		if (fplGetThreadState(thread) != fplThreadState_Stopped) {
			if (thread->internalHandle.win32ThreadHandle == fpl_null) {
				FPL__ERROR(FPL__MODULE_THREADING, "Thread handle for index '%d' are not allowed to be null", index);
				return false;
			}
		}
	}

	// @NOTE(final): WaitForMultipleObjects does not work for us here, because each thread will close its handle automatically
	// So we screw it and use a simple while loop and wait until either the timeout has been reached or all threads has been stopped.
	fplMilliseconds startTime = fplMillisecondsQuery();
	size_t minThreads = waitForAll ? count : 1;
	size_t stoppedThreads = 0;
	while (stoppedThreads < minThreads) {
		stoppedThreads = 0;
		for (size_t index = 0; index < count; ++index) {
			fplThreadHandle *thread = *(fplThreadHandle **)((uint8_t *)threads + index * actualStride);
			if (fplGetThreadState(thread) == fplThreadState_Stopped) {
				++stoppedThreads;
			}
		}
		if (stoppedThreads >= minThreads) {
			break;
		}
		if (timeout != FPL_TIMEOUT_INFINITE) {
			if ((fplMillisecondsQuery() - startTime) >= timeout) {
				break;
			}
		}
		fplThreadYield();
	}
	bool result = stoppedThreads >= minThreads;
	return(result);
}

fpl_internal bool fpl__Win32SignalWaitForMultiple(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout, const bool waitForAll) {
	FPL__CheckArgumentNull(signals, false);
	FPL__CheckArgumentMax(count, FPL_MAX_SIGNAL_COUNT, false);
	// @MEMORY(final): This wastes a lof memory, use temporary memory allocation here
	HANDLE signalHandles[FPL_MAX_SIGNAL_COUNT];
	const size_t actualStride = stride > 0 ? stride : sizeof(fplSignalHandle *);
	for (uint32_t index = 0; index < count; ++index) {
		fplSignalHandle *availableSignal = *(fplSignalHandle **)((uint8_t *)signals + index * actualStride);
		if (availableSignal == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if (availableSignal->internalHandle.win32EventHandle == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Signal handle for index '%d' are not allowed to be null", index);
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
	fplAssert(appState != fpl_null);
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32InitState *win32InitState = &initState->win32;
	if (appState->initFlags & fplInitFlags_GameController) {
		fpl__Win32UnloadXInputApi(&win32AppState->xinput.xinputApi);
	}
	fpl__Win32UnloadApi(&win32AppState->winApi);
}

#if defined(FPL__ENABLE_WINDOW)
fpl_internal fplKey fpl__Win32TranslateVirtualKey(const fpl__Win32Api *wapi, const uint64_t virtualKey) {
	switch (virtualKey) {
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
	fplAssert(initState != fpl_null);
	fplAssert(appState != fpl_null);

	fpl__Win32InitState *win32InitState = &initState->win32;
	fpl__Win32AppState *win32AppState = &appState->win32;

	// @NOTE(final): Expect kernel32.lib to be linked always, so VirtualAlloc, LoadLibrary, CreateThread, etc. will always work.

	// Get application instance handle
	win32InitState->appInstance = GetModuleHandleA(fpl_null);

	// Query performance frequency and store it once, it will never change during runtime
	QueryPerformanceFrequency(&win32InitState->qpf);

	// Get main thread infos
	HANDLE mainThreadHandle = GetCurrentThread();
	DWORD mainThreadHandleId = GetCurrentThreadId();
	fplThreadHandle *mainThread = &fpl__global__ThreadState.mainThread;
	fplClearStruct(mainThread);
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
	if (initFlags & fplInitFlags_GameController) {
		fpl__Win32LoadXInputApi(&win32AppState->xinput.xinputApi);
	}

	// Show/Hide console
	bool showConsole = (initFlags & fplInitFlags_Console);
	HWND consoleWindow = GetConsoleWindow();
	if (!showConsole) {
		if (consoleWindow != fpl_null) {
			win32AppState->winApi.user.ShowWindow(consoleWindow, SW_HIDE);
		} else {
			FreeConsole();
		}
	} else if (consoleWindow != fpl_null) {
		const fplConsoleSettings *initConsoleSettings = &initSettings->console;
		fplConsoleSettings *currentConsoleSettings = &appState->currentSettings.console;

		// Setup a console title
		wchar_t consoleTitleBuffer[FPL_MAX_NAME_LENGTH];
		if (fplGetStringLength(initConsoleSettings->title) > 0) {
			fplUTF8StringToWideString(initConsoleSettings->title, fplGetStringLength(initConsoleSettings->title), consoleTitleBuffer, fplArrayCount(consoleTitleBuffer));
		} else {
			const wchar_t *defaultTitle = FPL__WIN32_UNNAMED_CONSOLE;
			lstrcpynW(consoleTitleBuffer, defaultTitle, fplArrayCount(consoleTitleBuffer));
		}
		wchar_t *windowTitle = consoleTitleBuffer;
		fplWideStringToUTF8String(windowTitle, lstrlenW(windowTitle), currentConsoleSettings->title, fplArrayCount(currentConsoleSettings->title));
		SetConsoleTitleW(windowTitle);

		win32AppState->winApi.user.ShowWindow(consoleWindow, SW_SHOW);
	}

	// Init keymap
#	if defined(FPL__ENABLE_WINDOW)
	fplClearStruct(appState->window.keyMap);
	for (int i = 0; i < 256; ++i) {
		int vk = win32AppState->winApi.user.MapVirtualKeyW(MAPVK_VSC_TO_VK, i);
		if (vk == 0) {
			vk = i;
		}
		appState->window.keyMap[i] = fpl__Win32TranslateVirtualKey(&win32AppState->winApi, vk);
	}
#	endif

	// Hint for windows to know, that the application is in use always
#	if defined(FPL__ENABLE_WINDOW)
	if (initSettings->window.isMonitorPowerPrevented || initSettings->window.isScreenSaverPrevented) {
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
	} else {
		SetThreadExecutionState(ES_CONTINUOUS);
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
	fplAssert(target != fpl_null);
	uint32_t result = InterlockedExchange((volatile LONG *)target, value);
	return (result);
}
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value) {
	fplAssert(target != fpl_null);
	int32_t result = InterlockedExchange((volatile LONG *)target, value);
	return (result);
}
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	fplAssert(target != fpl_null);
	uint64_t result = InterlockedExchange64((volatile LONG64 *)target, value);
	return (result);
}
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value) {
	fplAssert(target != fpl_null);
	int64_t result = InterlockedExchange64((volatile LONG64 *)target, value);
	return (result);
}

fpl_platform_api uint32_t fplAtomicFetchAndAddU32(volatile uint32_t *value, const uint32_t addend) {
	fplAssert(value != fpl_null);
	uint32_t result = InterlockedExchangeAdd((volatile LONG *)value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicFetchAndAddS32(volatile int32_t *value, const int32_t addend) {
	fplAssert(value != fpl_null);
	int32_t result = InterlockedExchangeAdd((volatile LONG *)value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicFetchAndAddU64(volatile uint64_t *value, const uint64_t addend) {
	fplAssert(value != fpl_null);
	uint64_t result = InterlockedExchangeAdd64((volatile LONG64 *)value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicFetchAndAddS64(volatile int64_t *value, const int64_t addend) {
	fplAssert(value != fpl_null);
	int64_t result = InterlockedExchangeAdd64((volatile LONG64 *)value, addend);
	return(result);
}

fpl_platform_api uint32_t fplAtomicAddAndFetchU32(volatile uint32_t *value, const uint32_t addend) {
	fplAssert(value != fpl_null);
	uint32_t result = InterlockedAdd((volatile LONG *)value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicAddAndFetchS32(volatile int32_t *value, const int32_t addend) {
	fplAssert(value != fpl_null);
	int32_t result = InterlockedAdd((volatile LONG *)value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicAddAndFetchU64(volatile uint64_t *value, const uint64_t addend) {
	fplAssert(value != fpl_null);
	uint64_t result = InterlockedAdd64((volatile LONG64 *)value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicAddAndFetchS64(volatile int64_t *value, const int64_t addend) {
	fplAssert(value != fpl_null);
	int64_t result = InterlockedAdd64((volatile LONG64 *)value, addend);
	return(result);
}

fpl_platform_api uint32_t fplAtomicIncrementU32(volatile uint32_t *value) {
	fplAssert(value != fpl_null);
	uint32_t result = InterlockedIncrement((volatile LONG *)value);
	return (result);
}
fpl_platform_api int32_t fplAtomicIncrementS32(volatile int32_t *value) {
	fplAssert(value != fpl_null);
	int32_t result = InterlockedIncrement((volatile LONG *)value);
	return (result);
}
fpl_platform_api uint64_t fplAtomicIncrementU64(volatile uint64_t *value) {
	fplAssert(value != fpl_null);
	uint64_t result = InterlockedIncrement64((volatile LONG64 *)value);
	return (result);
}
fpl_platform_api int64_t fplAtomicIncrementS64(volatile int64_t *value) {
	fplAssert(value != fpl_null);
	int64_t result = InterlockedIncrement64((volatile LONG64 *)value);
	return(result);
}

fpl_platform_api uint32_t fplAtomicCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	fplAssert(dest != fpl_null);
	uint32_t result = InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api int32_t fplAtomicCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	fplAssert(dest != fpl_null);
	int32_t result = InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api uint64_t fplAtomicCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	fplAssert(dest != fpl_null);
	uint64_t result = InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	return (result);
}
fpl_platform_api int64_t fplAtomicCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	fplAssert(dest != fpl_null);
	int64_t result = InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	return (result);
}

fpl_platform_api bool fplAtomicIsCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	fplAssert(dest != fpl_null);
	uint32_t value = InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	fplAssert(dest != fpl_null);
	int32_t value = InterlockedCompareExchange((volatile LONG *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	fplAssert(dest != fpl_null);
	uint64_t value = InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	fplAssert(dest != fpl_null);
	int64_t value = InterlockedCompareExchange64((volatile LONG64 *)dest, exchange, comparand);
	bool result = (value == comparand);
	return (result);
}

fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source) {
	uint32_t result = InterlockedCompareExchange((volatile LONG *)source, 0, 0);
	return(result);
}
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source) {
	uint64_t result = InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
	return(result);
}
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source) {
	int32_t result = InterlockedCompareExchange((volatile LONG *)source, 0, 0);
	return(result);
}
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source) {
	int64_t result = InterlockedCompareExchange64((volatile LONG64 *)source, 0, 0);
	return(result);
}

fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
	InterlockedExchange((volatile LONG *)dest, value);
}
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
	InterlockedExchange64((volatile LONG64 *)dest, value);
}
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value) {
	InterlockedExchange((volatile LONG *)dest, value);
}
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value) {
	InterlockedExchange64((volatile LONG64 *)dest, value);
}

//
// Win32 OS
//
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

#define FPL__FUNC_NTDLL_RtlGetVersion(name) DWORD WINAPI name(PRTL_OSVERSIONINFOW lpVersionInformation)
typedef FPL__FUNC_NTDLL_RtlGetVersion(fpl__func_ntdll_RtlGetVersionProc);
#define FPL__FUNC_KERNEL32_GetVersion(name) DWORD WINAPI name()
typedef FPL__FUNC_KERNEL32_GetVersion(fpl__func_kernel32_GetVersion);
#define FPL__FUNC_KERNEL32_GetVersionExW(name) BOOL WINAPI name(LPOSVERSIONINFOEXW lpVersionInfo)
typedef FPL__FUNC_KERNEL32_GetVersionExW(fpl__func_kernel32_GetVersionExW);
fpl_platform_api bool fplOSGetVersionInfos(fplOSVersionInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);

	fplClearStruct(outInfos);

	// @NOTE(final): Prefer RtlGetVersion always, because MS might decide to totally remove GetVersion() and GetVersionEx()
	HMODULE ntdllModule = GetModuleHandleA("ntdll");
	fpl__func_ntdll_RtlGetVersionProc *rtlGetVersionProc = (fpl__func_ntdll_RtlGetVersionProc *)GetProcAddress(ntdllModule, "RtlGetVersion");
	if (rtlGetVersionProc != fpl_null) {
		RTL_OSVERSIONINFOW info = fplZeroInit;
		info.dwOSVersionInfoSize = sizeof(info);
		if (rtlGetVersionProc(&info) == 0) {
			fplS32ToString((int32_t)info.dwMajorVersion, outInfos->osVersion.major, fplArrayCount(outInfos->osVersion.major));
			fplS32ToString((int32_t)info.dwMinorVersion, outInfos->osVersion.minor, fplArrayCount(outInfos->osVersion.minor));
			fplS32ToString(0, outInfos->osVersion.fix, fplArrayCount(outInfos->osVersion.fix));
			fplS32ToString((int32_t)info.dwBuildNumber, outInfos->osVersion.build, fplArrayCount(outInfos->osVersion.build));
			fplStringFormat(outInfos->osVersion.fullName, fplArrayCount(outInfos->osVersion.fullName), "%u.%u.%u.%u", info.dwMajorVersion, info.dwMinorVersion, 0, info.dwBuildNumber);
			const char *versionName = fpl__Win32GetVersionName(info.dwMajorVersion, info.dwMinorVersion);
			fplCopyString(versionName, outInfos->osName, fplArrayCount(outInfos->osName));
			return(true);
		}
	}

	// @NOTE(final): GetVersion() and GetVersionExA() is deprecated as of windows 8.1, so we load it manually always
	HMODULE kernelLib = LoadLibraryA("kernel32.dll");
	if (kernelLib == fpl_null) {
		FPL__ERROR(FPL__MODULE_WIN32, "Kernel32 library could not be loaded");
		return false;
	}
	fpl__func_kernel32_GetVersion *getVersionProc = (fpl__func_kernel32_GetVersion *)GetProcAddress(kernelLib, "GetVersion");
	fpl__func_kernel32_GetVersionExW *getVersionExProc = (fpl__func_kernel32_GetVersionExW *)GetProcAddress(kernelLib, "GetVersionExW");
	FreeLibrary(kernelLib);

	if (getVersionExProc != fpl_null) {
		OSVERSIONINFOEXW infoEx = fplZeroInit;
		infoEx.dwOSVersionInfoSize = sizeof(infoEx);
		if (getVersionExProc(&infoEx) == TRUE) {
			fplS32ToString((int32_t)infoEx.dwMajorVersion, outInfos->osVersion.major, fplArrayCount(outInfos->osVersion.major));
			fplS32ToString((int32_t)infoEx.dwMinorVersion, outInfos->osVersion.minor, fplArrayCount(outInfos->osVersion.minor));
			fplS32ToString(0, outInfos->osVersion.fix, fplArrayCount(outInfos->osVersion.fix));
			fplS32ToString((int32_t)infoEx.dwBuildNumber, outInfos->osVersion.build, fplArrayCount(outInfos->osVersion.build));
			fplStringFormat(outInfos->osVersion.fullName, fplArrayCount(outInfos->osVersion.fullName), "%u.%u.%u.%u", infoEx.dwMajorVersion, infoEx.dwMinorVersion, 0, infoEx.dwBuildNumber);
			const char *versionName = fpl__Win32GetVersionName(infoEx.dwMajorVersion, infoEx.dwMinorVersion);
			fplCopyString(versionName, outInfos->osName, fplArrayCount(outInfos->osName));
			return(true);
		}
	}

	if (getVersionProc != fpl_null) {
		DWORD dwVersion = getVersionProc();
		if (dwVersion > 0) {
			DWORD major = (DWORD)(LOBYTE(LOWORD(dwVersion)));
			DWORD minor = (DWORD)(HIBYTE(LOWORD(dwVersion)));
			DWORD build = 0;
			if (dwVersion < 0x80000000) {
				build = (DWORD)((DWORD)(HIWORD(dwVersion)));
			}
			fplS32ToString((int32_t)major, outInfos->osVersion.major, fplArrayCount(outInfos->osVersion.major));
			fplS32ToString((int32_t)minor, outInfos->osVersion.minor, fplArrayCount(outInfos->osVersion.minor));
			fplS32ToString(0, outInfos->osVersion.fix, fplArrayCount(outInfos->osVersion.fix));
			fplS32ToString((int32_t)build, outInfos->osVersion.build, fplArrayCount(outInfos->osVersion.build));
			fplStringFormat(outInfos->osVersion.fullName, fplArrayCount(outInfos->osVersion.fullName), "%u.%u.%u.%u", major, minor, 0, build);
			const char *versionName = fpl__Win32GetVersionName(major, minor);
			fplCopyString(versionName, outInfos->osName, fplArrayCount(outInfos->osName));
			return(true);
		}
	}

	return(false);
}

#define FPL__FUNC_ADV32_GetUserNameW(name) BOOL WINAPI name(LPWSTR lpBuffer, LPDWORD pcbBuffer)
typedef FPL__FUNC_ADV32_GetUserNameW(fpl__func_adv32_GetUserNameW);
fpl_platform_api size_t fplSessionGetUsername(char *nameBuffer, const size_t maxNameBufferLen) {
	const char *libName = "advapi32.dll";
	HMODULE adv32Lib = LoadLibraryA(libName);
	if (adv32Lib == fpl_null) {
		FPL__ERROR(FPL__MODULE_WIN32, "Failed loading library '%s'", libName);
		return false;
	}
	fpl__func_adv32_GetUserNameW *getUserNameProc = (fpl__func_adv32_GetUserNameW *)GetProcAddress(adv32Lib, "GetUserNameW");
	size_t result = 0;
	if (getUserNameProc != fpl_null) {
		wchar_t wideBuffer[FPL_MAX_BUFFER_LENGTH];
		DWORD size = (DWORD)fplArrayCount(wideBuffer);
		if (getUserNameProc(wideBuffer, &size) == TRUE) {
			result = fplWideStringToUTF8String(wideBuffer, size, nameBuffer, maxNameBufferLen);
		}
	}
	FreeLibrary(adv32Lib);
	return(result);
}

//
// Win32 Hardware
//
fpl_platform_api size_t fplCPUGetCoreCount() {
	SYSTEM_INFO sysInfo = fplZeroInit;
	GetSystemInfo(&sysInfo);
	// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
	size_t result = sysInfo.dwNumberOfProcessors;
	return(result);
}

#define FPL__WIN32_PROCESSOR_ARCHITECTURE_ARM64 12
fpl_platform_api fplCPUArchType fplCPUGetArchitecture() {
	fplCPUArchType result;
	SYSTEM_INFO sysInfo = fplZeroInit;
	BOOL isWow64;
	if (IsWow64Process(GetCurrentProcess(), &isWow64)) {
		if (isWow64)
			GetNativeSystemInfo(&sysInfo);
		else
			GetSystemInfo(&sysInfo);
	} else {
		GetSystemInfo(&sysInfo);
	}
	switch (sysInfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			result = fplCPUArchType_x86_64;
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			result = fplCPUArchType_x64;
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			result = fplCPUArchType_Arm32;
			break;
		case FPL__WIN32_PROCESSOR_ARCHITECTURE_ARM64:
			result = fplCPUArchType_Arm64;
			break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			result = fplCPUArchType_Unknown;
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
		default:
			result = fplCPUArchType_x86;
			break;
	}
	return(result);
}

#define FPL__FUNC_WIN32_KERNEL32_GetPhysicallyInstalledSystemMemory(name) BOOL WINAPI name(PULONGLONG TotalMemoryInKilobytes)
typedef FPL__FUNC_WIN32_KERNEL32_GetPhysicallyInstalledSystemMemory(fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory);
fpl_platform_api bool fplMemoryGetInfos(fplMemoryInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);
	bool result = false;

	HMODULE kernel32lib = LoadLibraryA("kernel32.dll");
	if (kernel32lib == fpl_null) {
		return false;
	}
	fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory *getPhysicallyInstalledSystemMemory = (fpl__win32_kernel_func_GetPhysicallyInstalledSystemMemory *)GetProcAddress(kernel32lib, "GetPhysicallyInstalledSystemMemory");
	FreeLibrary(kernel32lib);

	ULONGLONG installedMemorySize = 0;
	if (getPhysicallyInstalledSystemMemory != fpl_null) {
		getPhysicallyInstalledSystemMemory(&installedMemorySize);
	}

	SYSTEM_INFO systemInfo = fplZeroInit;
	GetSystemInfo(&systemInfo);

	MEMORYSTATUSEX statex = fplZeroInit;
	statex.dwLength = sizeof(statex);

	if (GlobalMemoryStatusEx(&statex)) {
		fplClearStruct(outInfos);
		outInfos->installedPhysicalSize = installedMemorySize * 1024ull;
		outInfos->totalPhysicalSize = statex.ullTotalPhys;
		outInfos->freePhysicalSize = statex.ullAvailPhys;
		outInfos->totalCacheSize = statex.ullTotalVirtual;
		outInfos->freeCacheSize = statex.ullAvailVirtual;
		outInfos->pageSize = systemInfo.dwPageSize;
		if (outInfos->pageSize > 0) {
			outInfos->totalPageCount = statex.ullTotalPageFile / outInfos->pageSize;
			outInfos->freePageCount = statex.ullAvailPageFile / outInfos->pageSize;
		}
		result = true;
	}
	return(result);
}

//
// Win32 Threading
//
fpl_internal DWORD WINAPI fpl__Win32ThreadProc(void *data) {
	fplThreadHandle *thread = (fplThreadHandle *)data;
	fplAssert(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);

	fplThreadParameters parameters = thread->parameters;



	if (parameters.runFunc != fpl_null) {
		parameters.runFunc(thread, parameters.userData);
	}

	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
	HANDLE handle = thread->internalHandle.win32ThreadHandle;
	if (handle != fpl_null) {
		CloseHandle(handle);
	}
	thread->isValid = false;
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
	ExitThread(0);
}

fpl_platform_api uint32_t fplGetCurrentThreadId() {
	DWORD threadId = GetCurrentThreadId();
	uint32_t result = (uint32_t)threadId;
	return(result);
}

fpl_internal bool fpl__Win32SetThreadPriority(HANDLE threadHandle, const fplThreadPriority newPriority) {
	int win32Priority = 0;
	switch (newPriority) {
		case fplThreadPriority_Idle:
			win32Priority = THREAD_PRIORITY_IDLE;
			break;
		case fplThreadPriority_Low:
			win32Priority = THREAD_PRIORITY_LOWEST;
			break;
		case fplThreadPriority_Normal:
			win32Priority = THREAD_PRIORITY_NORMAL;
			break;
		case fplThreadPriority_High:
			win32Priority = THREAD_PRIORITY_HIGHEST;
			break;
		case fplThreadPriority_RealTime:
			win32Priority = THREAD_PRIORITY_TIME_CRITICAL;
			break;
		default:
			FPL__ERROR("Threading", "The thread priority %d is not supported", newPriority);
			return(false);
	}
	bool result = SetThreadPriority(threadHandle, win32Priority) == TRUE;
	return(result);
}

fpl_platform_api fplThreadHandle *fplThreadCreateWithParameters(fplThreadParameters *parameters) {
	FPL__CheckArgumentNull(parameters, fpl_null);
	FPL__CheckArgumentNull(parameters->runFunc, fpl_null);
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if (thread != fpl_null) {
		DWORD creationFlags = 0;
		DWORD threadId = 0;
		SIZE_T stackSize = parameters->stackSize;
		thread->parameters = *parameters;
		thread->currentState = fplThreadState_Starting;
		HANDLE handle = CreateThread(fpl_null, stackSize, fpl__Win32ThreadProc, thread, creationFlags, &threadId);
		if (handle != fpl_null) {
			fpl__Win32SetThreadPriority(handle, thread->parameters.priority);
			thread->isValid = true;
			thread->id = threadId;
			thread->internalHandle.win32ThreadHandle = handle;
			result = thread;
		} else {
			FPL__ERROR(FPL__MODULE_THREADING, "Failed creating thread, error code: %d", GetLastError());
		}
	} else {
		FPL__ERROR(FPL__MODULE_THREADING, "All %d threads are in use, you cannot create until you free one", FPL_MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_callback *runFunc, void *data) {
	FPL__CheckArgumentNull(runFunc, fpl_null);
	fplThreadParameters parameters = fplZeroInit;
	parameters.priority = fplThreadPriority_Normal;
	parameters.runFunc = runFunc;
	parameters.userData = data;
	fplThreadHandle *result = fplThreadCreateWithParameters(&parameters);
	return(result);
}

fpl_internal fplThreadPriority fpl__Win32MapNativeThreadPriority(const int win32ThreadPriority) {
	switch (win32ThreadPriority) {
		case THREAD_PRIORITY_IDLE:
			return fplThreadPriority_Idle;
		case THREAD_PRIORITY_LOWEST:
		case THREAD_PRIORITY_BELOW_NORMAL:
			return fplThreadPriority_Low;
		case THREAD_PRIORITY_NORMAL:
			return fplThreadPriority_Normal;
		case THREAD_PRIORITY_ABOVE_NORMAL:
		case THREAD_PRIORITY_HIGHEST:
			return fplThreadPriority_High;
		case THREAD_PRIORITY_TIME_CRITICAL:
			return fplThreadPriority_RealTime;
		default:
			return fplThreadPriority_Unknown;
	}
}

fpl_platform_api fplThreadPriority fplGetThreadPriority(fplThreadHandle *thread) {
	FPL__CheckArgumentNull(thread, fplThreadPriority_Unknown);
	fplThreadPriority result = fplThreadPriority_Unknown;
	if (thread->isValid && thread->internalHandle.win32ThreadHandle != fpl_null) {
		HANDLE threadHandle = thread->internalHandle.win32ThreadHandle;
		int win32ThreadPriority = GetThreadPriority(threadHandle);
		result = fpl__Win32MapNativeThreadPriority(win32ThreadPriority);
	}
	return(result);
}

fpl_platform_api bool fplSetThreadPriority(fplThreadHandle *thread, const fplThreadPriority newPriority) {
	FPL__CheckArgumentNull(thread, false);
	bool result = false;
	if (thread->isValid && thread->internalHandle.win32ThreadHandle != fpl_null) {
		HANDLE threadHandle = thread->internalHandle.win32ThreadHandle;
		result = fpl__Win32SetThreadPriority(threadHandle, newPriority);
	}
	return(result);
}

fpl_platform_api void fplThreadSleep(const uint32_t milliseconds) {
	Sleep((DWORD)milliseconds);
}

fpl_platform_api bool fplThreadYield() {
	YieldProcessor();
	return(true);
}

fpl_platform_api bool fplThreadTerminate(fplThreadHandle *thread) {
	FPL__CheckArgumentNull(thread, false);
	fplThreadState state = fplGetThreadState(thread);
	if (thread->isValid && (state != fplThreadState_Stopped && state != fplThreadState_Stopping)) {
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopping);
		HANDLE handle = thread->internalHandle.win32ThreadHandle;
		if (handle != fpl_null) {
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
	if (fplGetThreadState(thread) != fplThreadState_Stopped) {
		if (thread->internalHandle.win32ThreadHandle == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Win32 thread handle are not allowed to be null");
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

fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, stride, timeout, true);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__Win32ThreadWaitForMultiple(threads, count, stride, timeout, false);
	return(result);
}

fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if (mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex '%p' is already initialized", mutex);
		return false;
	}
	fplClearStruct(mutex);
	mutex->isValid = true;
	fplAssert(sizeof(mutex->internalHandle.win32CriticalSection) >= sizeof(CRITICAL_SECTION));
	CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
	InitializeCriticalSection(critSection);
	return true;
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	FPL__CheckArgumentNullNoRet(mutex);
	if (mutex->isValid) {
		fplAssert(sizeof(mutex->internalHandle.win32CriticalSection) >= sizeof(CRITICAL_SECTION));
		CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
		DeleteCriticalSection(critSection);
		fplClearStruct(mutex);
	}
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if (!mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	fplAssert(sizeof(mutex->internalHandle.win32CriticalSection) >= sizeof(CRITICAL_SECTION));
	CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
	EnterCriticalSection(critSection);
	return true;
}

fpl_platform_api bool fplMutexTryLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if (!mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	fplAssert(sizeof(mutex->internalHandle.win32CriticalSection) >= sizeof(CRITICAL_SECTION));
	CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
	bool result = TryEnterCriticalSection(critSection) == TRUE;
	return(result);
}

fpl_platform_api bool fplMutexUnlock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if (!mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex parameter must be valid");
		return false;
	}
	fplAssert(sizeof(mutex->internalHandle.win32CriticalSection) >= sizeof(CRITICAL_SECTION));
	CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
	LeaveCriticalSection(critSection);
	return true;
}

fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	FPL__CheckArgumentNull(signal, false);
	if (signal->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal '%p' is already initialized", signal);
		return false;
	}
	HANDLE handle = CreateEventA(fpl_null, FALSE, (initialValue == fplSignalValue_Set) ? TRUE : FALSE, fpl_null);
	if (handle == fpl_null) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed creating signal (Win32 event): %d", GetLastError());
		return false;
	}
	fplClearStruct(signal);
	signal->isValid = true;
	signal->internalHandle.win32EventHandle = handle;
	return(true);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	FPL__CheckArgumentNullNoRet(signal);
	if (signal->internalHandle.win32EventHandle != fpl_null) {
		HANDLE handle = signal->internalHandle.win32EventHandle;
		CloseHandle(handle);
		fplClearStruct(signal);
	}
}

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signal, false);
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = (WaitForSingleObject(handle, t) == WAIT_OBJECT_0);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__Win32SignalWaitForMultiple(signals, count, stride, timeout, true);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__Win32SignalWaitForMultiple(signals, count, stride, timeout, false);
	return(result);
}

fpl_platform_api bool fplSignalSet(fplSignalHandle *signal) {
	FPL__CheckArgumentNull(signal, false);
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = SetEvent(handle) == TRUE;
	return(result);
}

fpl_platform_api bool fplSignalReset(fplSignalHandle *signal) {
	FPL__CheckArgumentNull(signal, false);
	if (signal->internalHandle.win32EventHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal handle are not allowed to be null");
		return false;
	}
	HANDLE handle = signal->internalHandle.win32EventHandle;
	bool result = ResetEvent(handle) == TRUE;
	return(result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	fplClearStruct(condition);
	fplAssert(sizeof(condition->internalHandle.win32Condition) == sizeof(CONDITION_VARIABLE));
	CONDITION_VARIABLE *condVar = (CONDITION_VARIABLE *)&condition->internalHandle.win32Condition;
	InitializeConditionVariable(condVar);
	condition->isValid = true;
	return true;
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	FPL__CheckArgumentNullNoRet(condition);
	if (condition->isValid) {
		fplClearStruct(condition);
	}
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckArgumentNull(mutex, false);
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	if (!mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex '%p' is not valid", mutex);
		return false;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	CRITICAL_SECTION *critSection = (CRITICAL_SECTION *)&mutex->internalHandle.win32CriticalSection;
	CONDITION_VARIABLE *condVar = (CONDITION_VARIABLE *)&condition->internalHandle.win32Condition;
	bool result = SleepConditionVariableCS(condVar, critSection, t) != 0;
	return(result);
}

fpl_platform_api bool fplConditionSignal(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	CONDITION_VARIABLE *critSection = (CONDITION_VARIABLE *)&condition->internalHandle.win32Condition;
	WakeConditionVariable(critSection);
	return true;
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition '%p' is not valid", condition);
		return false;
	}
	CONDITION_VARIABLE *critSection = (CONDITION_VARIABLE *)&condition->internalHandle.win32Condition;
	WakeAllConditionVariable(critSection);
	return true;
}

fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue) {
	FPL__CheckArgumentNull(semaphore, false);
	FPL__CheckArgumentMax(initialValue, INT32_MAX, false);
	if (semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is already initialized", semaphore);
		return false;
	}
	HANDLE handle = CreateSemaphoreA(fpl_null, (LONG)initialValue, INT32_MAX, fpl_null);
	if (handle == fpl_null) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed creating semaphore");
		return false;
	}
	fplClearStruct(semaphore);
	semaphore->isValid = true;
	semaphore->internalHandle.win32.handle = handle;
	semaphore->internalHandle.win32.value = (int32_t)initialValue;
	return true;
}

fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNullNoRet(semaphore);
	if (semaphore->isValid) {
		CloseHandle(semaphore->internalHandle.win32.handle);
		fplClearStruct(semaphore);
	}
}

fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	DWORD t = timeout == FPL_TIMEOUT_INFINITE ? INFINITE : timeout;
	bool result = false;
	if (WaitForSingleObject(semaphore->internalHandle.win32.handle, timeout) == WAIT_OBJECT_0) {
		fplAtomicFetchAndAddS32(&semaphore->internalHandle.win32.value, -1);
		result = true;
	}
	return(result);
}

fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	bool result = false;
	if (WaitForSingleObject(semaphore->internalHandle.win32.handle, 0) == WAIT_OBJECT_0) {
		fplAtomicFetchAndAddS32(&semaphore->internalHandle.win32.value, -1);
		result = true;
	}
	return(result);
}

fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	int32_t result = fplAtomicLoadS32(&semaphore->internalHandle.win32.value);
	return(result);
}

fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	bool result = true;
	int32_t prevValue = fplAtomicFetchAndAddS32(&semaphore->internalHandle.win32.value, 1);
	if (ReleaseSemaphore(semaphore->internalHandle.win32.handle, 1, fpl_null) == FALSE) {
		// Restore value when it fails
		FPL__ERROR(FPL__MODULE_THREADING, "Failed releasing the semaphore '%p'", semaphore);
		fplAtomicStoreS32(&semaphore->internalHandle.win32.value, prevValue);
		result = false;
	}
	return(result);
}

//
// Win32 Console
//
fpl_platform_api void fplConsoleOut(const char *text) {
	DWORD charsToWrite = (DWORD)fplGetStringLength(text);
	DWORD writtenChars = 0;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	wchar_t wideBuffer[FPL_MAX_BUFFER_LENGTH];
	fplUTF8StringToWideString(text, charsToWrite, wideBuffer, fplArrayCount(wideBuffer));
	WriteConsoleW(handle, wideBuffer, charsToWrite, &writtenChars, fpl_null);
}

fpl_platform_api void fplConsoleError(const char *text) {
	DWORD charsToWrite = (DWORD)fplGetStringLength(text);
	DWORD writtenChars = 0;
	HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
	wchar_t wideBuffer[FPL_MAX_BUFFER_LENGTH];
	fplUTF8StringToWideString(text, charsToWrite, wideBuffer, fplArrayCount(wideBuffer));
	WriteConsoleW(handle, wideBuffer, charsToWrite, &writtenChars, fpl_null);
}

fpl_platform_api char fplConsoleWaitForCharInput() {
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	DWORD savedMode;
	GetConsoleMode(handle, &savedMode);
	SetConsoleMode(handle, ENABLE_PROCESSED_INPUT);
	char result = 0;
	if (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0) {
		DWORD charsRead = 0;
		char inputBuffer[2] = fplZeroInit;
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
	FPL__CheckArgumentZero(size, fpl_null);
	void *result = VirtualAlloc(fpl_null, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (result == fpl_null) {
		FPL__ERROR(FPL__MODULE_MEMORY, "Failed allocating memory of %xu bytes", size);
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
fpl_internal const uint64_t FPL__WIN32_TICKS_PER_SEC = 10000000ULL;
fpl_internal const uint64_t FPL__WIN32_UNIX_EPOCH_DIFFERENCE = 11644473600ULL;

fpl_internal fplFileTimeStamp fpl__Win32ConvertFileTimeToUnixTimestamp(const FILETIME *fileTime) {
	// Ticks are defined in 100 ns = 10000000 secs
	// Windows ticks starts at 1601-01-01T00:00:00Z
	// Unix secs starts at 1970-01-01T00:00:00Z
	fplFileTimeStamp result = 0;
	if (fileTime != fpl_null && (fileTime->dwLowDateTime > 0 || fileTime->dwHighDateTime > 0)) {
		// Convert to SYSTEMTIME and remove milliseconds
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(fileTime, &sysTime);
		sysTime.wMilliseconds = 0; // Really important, because unix-timestamps does not support milliseconds

		// Reconvert to FILETIME to account for removed milliseconds
		FILETIME withoutMSecs;
		SystemTimeToFileTime(&sysTime, &withoutMSecs);

		// Convert to large integer so we can access U64 directly
		ULARGE_INTEGER ticks;
		ticks.LowPart = withoutMSecs.dwLowDateTime;
		ticks.HighPart = withoutMSecs.dwHighDateTime;

		// Final conversion from ticks to unix-timestamp
		result = (ticks.QuadPart / FPL__WIN32_TICKS_PER_SEC) - FPL__WIN32_UNIX_EPOCH_DIFFERENCE;
	}
	return(result);
}

fpl_internal FILETIME fpl__Win32ConvertUnixTimestampToFileTime(const fplFileTimeStamp unixTimeStamp) {
	// Ticks are defined in 100 ns = 10000000 secs
	// 100 ns = milliseconds * 10000
	// Windows ticks starts at 1601-01-01T00:00:00Z
	// Unix secs starts at 1970-01-01T00:00:00Z
	if (unixTimeStamp > 0) {
		uint64_t ticks = (unixTimeStamp + FPL__WIN32_UNIX_EPOCH_DIFFERENCE) * FPL__WIN32_TICKS_PER_SEC;
		FILETIME result;
		result.dwLowDateTime = (DWORD)ticks;
		result.dwHighDateTime = ticks >> 32;
		return(result);
	}
	FILETIME empty = fplZeroInit;
	return(empty);
}

fpl_platform_api bool fplFileOpenBinary(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			fplClearStruct(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api bool fplFileCreateBinary(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, GENERIC_WRITE, FILE_SHARE_WRITE, fpl_null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			fplClearStruct(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.win32FileHandle = (void *)win32FileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api uint32_t fplFileReadBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_FILES, "Filehandle is not opened for reading");
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

fpl_platform_api uint64_t fplFileReadBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_FILES, "Filehandle is not opened for reading");
		return 0;
	}
	// @NOTE(final): There is no ReadFile64 function in win32, so we have to read it in chunks
	uint64_t result = 0;
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint64_t remainingSize = sizeToRead;
	uint64_t bufferPos = 0;
	const uint64_t MaxDWORD = (uint64_t)(DWORD)-1;
	while (remainingSize > 0) {
		DWORD bytesRead = 0;
		uint8_t *target = (uint8_t *)targetBuffer + bufferPos;
		uint64_t size = fplMin(remainingSize, MaxDWORD);
		fplAssert(size <= MaxDWORD);
		if (ReadFile(win32FileHandle, target, (DWORD)size, &bytesRead, fpl_null) == TRUE) {
			result = bytesRead;
		} else {
			break;
		}
		remainingSize -= bytesRead;
		bufferPos += bytesRead;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileWriteBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_FILES, "Filehandle is not opened for writing");
		return 0;
	}
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint32_t result = 0;
	DWORD bytesWritten = 0;
	if (WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, fpl_null) == TRUE) {
		result = bytesWritten;
	}
	return(result);
}

fpl_platform_api uint64_t fplFileWriteBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if (fileHandle->internalHandle.win32FileHandle == fpl_null) {
		FPL__ERROR(FPL__MODULE_FILES, "Filehandle is not opened for writing");
		return 0;
	}
	HANDLE win32FileHandle = (HANDLE)fileHandle->internalHandle.win32FileHandle;
	uint64_t result = 0;
	uint64_t bufferPos = 0;
	uint64_t remainingSize = sourceSize;
	const uint64_t MaxDWORD = (uint64_t)(DWORD)-1;
	while (remainingSize > 0) {
		uint8_t *source = (uint8_t *)sourceBuffer + bufferPos;
		uint64_t size = fplMin(remainingSize, MaxDWORD);
		fplAssert(size <= MaxDWORD);
		DWORD bytesWritten = 0;
		if (WriteFile(win32FileHandle, source, (DWORD)size, &bytesWritten, fpl_null) == TRUE) {
			result = bytesWritten;
		} else {
			break;
		}
		remainingSize -= bytesWritten;
		bufferPos += bytesWritten;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileSetPosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD moveMethod = FILE_BEGIN;
		if (mode == fplFilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if (mode == fplFilePositionMode_End) {
			moveMethod = FILE_END;
		}
		DWORD r = 0;
		r = SetFilePointer(win32FileHandle, (LONG)position, fpl_null, moveMethod);
		result = (uint32_t)r;
	}
	return(result);
}

fpl_platform_api uint64_t fplFileSetPosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD moveMethod = FILE_BEGIN;
		if (mode == fplFilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if (mode == fplFilePositionMode_End) {
			moveMethod = FILE_END;
		}
		LARGE_INTEGER r = fplZeroInit;
		LARGE_INTEGER li;
		li.QuadPart = position;
		if (SetFilePointerEx(win32FileHandle, li, &r, moveMethod) == TRUE) {
			result = (uint64_t)r.QuadPart;
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplFileGetPosition32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD filePosition = SetFilePointer(win32FileHandle, 0L, fpl_null, FILE_CURRENT);
		if (filePosition != INVALID_SET_FILE_POINTER) {
			return filePosition;
		}
	}
	return 0;
}

fpl_platform_api uint64_t fplFileGetPosition64(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		LARGE_INTEGER r = fplZeroInit;
		LARGE_INTEGER li;
		li.QuadPart = 0;
		if (SetFilePointerEx(win32FileHandle, li, &r, FILE_CURRENT) == TRUE) {
			result = (uint64_t)r.QuadPart;
		}
	}
	return 0;
}

fpl_platform_api bool fplFileFlush(fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, false);
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		bool result = FlushFileBuffers(win32FileHandle) == TRUE;
		return(result);
	}
	return(false);
}

fpl_platform_api void fplFileClose(fplFileHandle *fileHandle) {
	if ((fileHandle != fpl_null) && (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE)) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		CloseHandle(win32FileHandle);
		fplClearStruct(fileHandle);
	}
}

fpl_platform_api uint32_t fplFileGetSizeFromPath32(const char *filePath) {
	uint32_t result = 0;
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
			result = (uint32_t)fileSize;
			CloseHandle(win32FileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplFileGetSizeFromPath64(const char *filePath) {
	uint64_t result = 0;
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER li = fplZeroInit;
			if (GetFileSizeEx(win32FileHandle, &li) == TRUE) {
				result = (uint64_t)li.QuadPart;
			}
			CloseHandle(win32FileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplFileGetSizeFromHandle32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		DWORD fileSize = GetFileSize(win32FileHandle, fpl_null);
		result = (uint32_t)fileSize;
	}
	return(result);
}

fpl_platform_api uint64_t fplFileGetSizeFromHandle64(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		LARGE_INTEGER li = fplZeroInit;
		if (GetFileSizeEx(win32FileHandle, &li) == TRUE) {
			result = (uint64_t)li.QuadPart;
		}
	}
	return(result);
}

fpl_platform_api bool fplFileGetTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(outStamps, false);
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, GENERIC_READ, FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, fpl_null);
		bool result = false;
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			FILETIME times[3];
			if (GetFileTime(win32FileHandle, &times[0], &times[1], &times[2]) == TRUE) {
				fplClearStruct(outStamps);
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

fpl_platform_api bool fplFileGetTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentNull(outStamps, 0);
	if (fileHandle->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle.win32FileHandle;
		FILETIME times[3];
		if (GetFileTime(win32FileHandle, &times[0], &times[1], &times[2]) == TRUE) {
			fplClearStruct(outStamps);
			outStamps->creationTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[0]);
			outStamps->lastAccessTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[1]);
			outStamps->lastModifyTime = fpl__Win32ConvertFileTimeToUnixTimestamp(&times[2]);
			return(true);
		}
	}
	return(false);
}

fpl_platform_api bool fplFileSetTimestamps(const char *filePath, const fplFileTimeStamps *timeStamps) {
	FPL__CheckArgumentNull(timeStamps, false);
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		HANDLE win32FileHandle = CreateFileW(filePathWide, FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE | FILE_SHARE_READ, fpl_null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, fpl_null);
		bool result = false;
		if (win32FileHandle != INVALID_HANDLE_VALUE) {
			FILETIME times[3];
			times[0] = fpl__Win32ConvertUnixTimestampToFileTime(timeStamps->creationTime);
			times[1] = fpl__Win32ConvertUnixTimestampToFileTime(timeStamps->lastAccessTime);
			times[2] = fpl__Win32ConvertUnixTimestampToFileTime(timeStamps->lastModifyTime);
			if (SetFileTime(win32FileHandle, &times[0], NULL, NULL) == TRUE) {
				return(true);
			}
			CloseHandle(win32FileHandle);
		}
		return(result);
	}
	return(false);
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if (filePath != fpl_null) {
		wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
		WIN32_FIND_DATAW findData;
		HANDLE searchHandle = FindFirstFileW(filePathWide, &findData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	wchar_t sourceFilePathWide[FPL_MAX_PATH_LENGTH];
	wchar_t targetFilePathWide[FPL_MAX_PATH_LENGTH];
	fplUTF8StringToWideString(sourceFilePath, fplGetStringLength(sourceFilePath), sourceFilePathWide, fplArrayCount(sourceFilePathWide));
	fplUTF8StringToWideString(sourceFilePath, fplGetStringLength(sourceFilePath), targetFilePathWide, fplArrayCount(targetFilePathWide));
	bool result = (CopyFileW(sourceFilePathWide, targetFilePathWide, !overwrite) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileMove(const char *sourceFilePath, const char *targetFilePath) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	wchar_t sourceFilePathWide[FPL_MAX_PATH_LENGTH];
	wchar_t targetFilePathWide[FPL_MAX_PATH_LENGTH];
	fplUTF8StringToWideString(sourceFilePath, fplGetStringLength(sourceFilePath), sourceFilePathWide, fplArrayCount(sourceFilePathWide));
	fplUTF8StringToWideString(sourceFilePath, fplGetStringLength(sourceFilePath), targetFilePathWide, fplArrayCount(targetFilePathWide));
	bool result = (MoveFileW(sourceFilePathWide, targetFilePathWide) == TRUE);
	return(result);
}

fpl_platform_api bool fplFileDelete(const char *filePath) {
	FPL__CheckArgumentNull(filePath, false);
	wchar_t filePathWide[FPL_MAX_PATH_LENGTH];
	fplUTF8StringToWideString(filePath, fplGetStringLength(filePath), filePathWide, fplArrayCount(filePathWide));
	bool result = (DeleteFileW(filePathWide) == TRUE);
	return(result);
}

fpl_platform_api bool fplDirectoryExists(const char *path) {
	bool result = false;
	if (path != fpl_null) {
		wchar_t pathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(path, fplGetStringLength(path), pathWide, fplArrayCount(pathWide));
		WIN32_FIND_DATAW findData;
		HANDLE searchHandle = FindFirstFileW(pathWide, &findData);
		if (searchHandle != INVALID_HANDLE_VALUE) {
			result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
			FindClose(searchHandle);
		}
	}
	return(result);
}

fpl_platform_api bool fplDirectoriesCreate(const char *path) {
	FPL__CheckArgumentNull(path, false);
	wchar_t pathWide[FPL_MAX_PATH_LENGTH];
	fplUTF8StringToWideString(path, fplGetStringLength(path), pathWide, fplArrayCount(pathWide));
	bool result = CreateDirectoryW(pathWide, fpl_null) > 0;
	return(result);
}
fpl_platform_api bool fplDirectoryRemove(const char *path) {
	FPL__CheckArgumentNull(path, false);
	wchar_t pathWide[FPL_MAX_PATH_LENGTH];
	fplUTF8StringToWideString(path, fplGetStringLength(path), pathWide, fplArrayCount(pathWide));
	bool result = RemoveDirectoryW(pathWide) > 0;
	return(result);
}
fpl_internal void fpl__Win32FillFileEntry(const char *rootPath, const WIN32_FIND_DATAW *findData, fplFileEntry *entry) {
	fplAssert(findData != fpl_null);
	fplAssert(entry != fpl_null);
	fplWideStringToUTF8String(findData->cFileName, lstrlenW(findData->cFileName), entry->name, fplArrayCount(entry->name));
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

	// @TODO(final/Win32): Win32 Read ACL for full permission detection!
	entry->attributes = fplFileAttributeFlags_None;
	entry->permissions.umask = 0;
	if (findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
		entry->attributes = fplFileAttributeFlags_Normal;
	} else {
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			entry->attributes |= fplFileAttributeFlags_Hidden;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			entry->attributes |= fplFileAttributeFlags_Archive;
		}
		if (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			entry->attributes |= fplFileAttributeFlags_System;
		}
		entry->permissions.user |= fplFilePermissionFlags_CanWrite;
		entry->permissions.user |= fplFilePermissionFlags_CanRead;
		entry->permissions.user |= fplFilePermissionFlags_CanExecuteSearch;
		if ((findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) || (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
			entry->permissions.user &= ~fplFilePermissionFlags_CanWrite;
		}
	}
	if (entry->type == fplFileEntryType_File) {
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
fpl_platform_api bool fplDirectoryListBegin(const char *path, const char *filter, fplFileEntry *entry) {
	FPL__CheckArgumentNull(path, false);
	FPL__CheckArgumentNull(entry, false);
	if (fplGetStringLength(filter) == 0) {
		filter = "*";
	}
	char pathAndFilter[MAX_PATH + 1] = fplZeroInit;
	fplCopyString(path, pathAndFilter, fplArrayCount(pathAndFilter));
	fplEnforcePathSeparatorLen(pathAndFilter, fplArrayCount(pathAndFilter));
	fplStringAppend(filter, pathAndFilter, fplArrayCount(pathAndFilter));
	wchar_t pathAndFilterWide[MAX_PATH + 1];
	fplUTF8StringToWideString(pathAndFilter, fplGetStringLength(pathAndFilter), pathAndFilterWide, fplArrayCount(pathAndFilterWide));
	WIN32_FIND_DATAW findData;
	HANDLE searchHandle = FindFirstFileW(pathAndFilterWide, &findData);
	bool result = false;
	if (searchHandle != INVALID_HANDLE_VALUE) {
		fplClearStruct(entry);
		entry->internalHandle.win32FileHandle = searchHandle;
		entry->internalRoot.rootPath = path;
		entry->internalRoot.filter = filter;
		bool foundFirst = true;
		while (foundFirst) {
			if (lstrcmpW(findData.cFileName, L".") == 0 || lstrcmpW(findData.cFileName, L"..") == 0) {
				foundFirst = FindNextFileW(searchHandle, &findData) == TRUE;
			} else {
				result = true;
				fpl__Win32FillFileEntry(path, &findData, entry);
				break;
			}
		}
	}
	return(result);
}
fpl_platform_api bool fplDirectoryListNext(fplFileEntry *entry) {
	FPL__CheckArgumentNull(entry, false);
	bool result = false;
	if (entry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = entry->internalHandle.win32FileHandle;
		WIN32_FIND_DATAW findData;
		bool foundNext;
		do {
			foundNext = FindNextFileW(searchHandle, &findData) == TRUE;
			if (foundNext) {
				if (lstrcmpW(findData.cFileName, L".") == 0 || lstrcmpW(findData.cFileName, L"..") == 0) {
					continue;
				}
				fpl__Win32FillFileEntry(entry->internalRoot.rootPath, &findData, entry);
				result = true;
				break;
			}
		} while (foundNext);
	}
	return(result);
}
fpl_platform_api void fplDirectoryListEnd(fplFileEntry *entry) {
	FPL__CheckArgumentNullNoRet(entry);
	if (entry->internalHandle.win32FileHandle != INVALID_HANDLE_VALUE) {
		HANDLE searchHandle = entry->internalHandle.win32FileHandle;
		FindClose(searchHandle);
		fplClearStruct(entry);
	}
}

//
// Win32 Path/Directories
//
fpl_platform_api size_t fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	wchar_t modulePath[MAX_PATH];
	GetModuleFileNameW(fpl_null, modulePath, MAX_PATH);
	size_t modulePathLen = lstrlenW(modulePath);
	size_t result = fplWideStringToUTF8String(modulePath, modulePathLen, destPath, maxDestLen);
	return(result);
}

fpl_platform_api size_t fplGetHomePath(char *destPath, const size_t maxDestLen) {
	FPL__CheckPlatform(0);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
	wchar_t homePath[MAX_PATH];
	wapi->shell.SHGetFolderPathW(fpl_null, CSIDL_PROFILE, fpl_null, 0, homePath);
	size_t homePathLen = lstrlenW(homePath);
	size_t result = fplWideStringToUTF8String(homePath, homePathLen, destPath, maxDestLen);
	return(result);
}

//
// Win32 Timings
//
fpl_platform_api fplTimestamp fplTimestampQuery() {
	const fpl__Win32InitState *initState = &fpl__global__InitState.win32;
	fplTimestamp result = fplZeroInit;
	if (initState->qpf.QuadPart > 0) {
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		result.win32.qpc.QuadPart = time.QuadPart;
	} else {
		result.win32.ticks = GetTickCount64();
	}
	return(result);
}

fpl_platform_api fplSeconds fplTimestampElapsed(const fplTimestamp start, const fplTimestamp finish) {
	const fpl__Win32InitState *initState = &fpl__global__InitState.win32;
	fplSeconds result;
	LARGE_INTEGER freq = initState->qpf;
	if (freq.QuadPart > 0) {
		uint64_t delta = finish.win32.qpc.QuadPart - start.win32.qpc.QuadPart;
		result = (fplSeconds)(delta / (double)freq.QuadPart);
	} else {
		uint64_t delta = finish.win32.ticks - start.win32.ticks;
		result = (fplSeconds)(delta / 1000.0);
	}
	return(result);
}

fpl_platform_api fplMilliseconds fplMillisecondsQuery() {
	fplMilliseconds result = (fplMilliseconds)GetTickCount64();
	return(result);
}

//
// Win32 Strings
//
fpl_platform_api size_t fplWideStringToUTF8String(const wchar_t *wideSource, const size_t wideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	FPL__CheckArgumentNull(wideSource, 0);
	FPL__CheckArgumentZero(wideSourceLen, 0);
	size_t result = WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)wideSourceLen, fpl_null, 0, fpl_null, fpl_null);
	if (utf8Dest != fpl_null) {
		size_t minRequiredLen = result + 1;
		FPL__CheckArgumentMin(maxUtf8DestLen, minRequiredLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, wideSource, (int)wideSourceLen, utf8Dest, (int)maxUtf8DestLen, fpl_null, fpl_null);
		utf8Dest[result] = 0;
	}
	return(result);
}
fpl_platform_api size_t fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	FPL__CheckArgumentNull(utf8Source, 0);
	FPL__CheckArgumentZero(utf8SourceLen, 0);
	size_t result = MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, fpl_null, 0);
	if (wideDest != fpl_null) {
		size_t minRequiredLen = result + 1;
		FPL__CheckArgumentMin(maxWideDestLen, minRequiredLen, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf8Source, (int)utf8SourceLen, wideDest, (int)maxWideDestLen);
		wideDest[result] = 0;
	}
	return(result);
}

//
// Win32 Library
//
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle) {
	bool result = false;
	if (libraryFilePath != fpl_null && outHandle != fpl_null) {
		wchar_t libraryFilePathWide[FPL_MAX_PATH_LENGTH];
		fplUTF8StringToWideString(libraryFilePath, fplGetStringLength(libraryFilePath), libraryFilePathWide, fplArrayCount(libraryFilePathWide));
		HMODULE libModule = LoadLibraryW(libraryFilePathWide);
		if (libModule != fpl_null) {
			fplClearStruct(outHandle);
			outHandle->internalHandle.win32LibraryHandle = libModule;
			outHandle->isValid = true;
			result = true;
		}
	}
	return(result);
}
fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name) {
	if ((handle != fpl_null) && (handle->internalHandle.win32LibraryHandle != fpl_null) && (name != fpl_null)) {
		HMODULE libModule = (HMODULE)handle->internalHandle.win32LibraryHandle;
		return (void *)GetProcAddress(libModule, name);
	}
	return fpl_null;
}
fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if ((handle != fpl_null) && (handle->internalHandle.win32LibraryHandle != fpl_null)) {
		HMODULE libModule = (HMODULE)handle->internalHandle.win32LibraryHandle;
		FreeLibrary(libModule);
		fplClearStruct(handle);
	}
}

#if defined(FPL__ENABLE_WINDOW)
//
// Win32 Window
//
fpl_platform_api bool fplGetWindowSize(fplWindowSize *outSize) {
	FPL__CheckArgumentNull(outSize, false);
	FPL__CheckPlatform(false);
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

fpl_platform_api void fplSetWindowSize(const uint32_t width, const uint32_t height) {
	FPL__CheckPlatformNoRet();
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
		wapi->user.SetWindowPos(windowState->windowHandle, fpl_null, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
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
	if (!appState->currentSettings.window.isFullscreen && appState->currentSettings.window.isDecorated) {
		DWORD style = fpl__win32_GetWindowLong(windowState->windowHandle, GWL_STYLE);
		DWORD exStyle = fpl__win32_GetWindowLong(windowState->windowHandle, GWL_EXSTYLE);
		if (value) {
			style |= (WS_MAXIMIZEBOX | WS_THICKFRAME);
		} else {
			style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
		}
		fpl__win32_SetWindowLong(windowState->windowHandle, GWL_STYLE, style);
		appState->currentSettings.window.isResizable = value;
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
	const fpl__Win32Api *wapi = &appState->win32.winApi;
	if (!appState->currentSettings.window.isFullscreen) {
		HWND windowHandle = windowState->windowHandle;
		DWORD style = fpl__win32_GetWindowLong(windowHandle, GWL_STYLE);
		DWORD exStyle = fpl__win32_GetWindowLong(windowHandle, GWL_EXSTYLE);
		if (value) {
			style &= ~WS_POPUP;
			style |= WS_OVERLAPPEDWINDOW;
			if (!appState->currentSettings.window.isResizable) {
				style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
			}
		} else {
			style &= ~WS_OVERLAPPEDWINDOW;
			style |= WS_POPUP;
		}
		fpl__win32_SetWindowLong(windowHandle, GWL_STYLE, style);
		wapi->user.SetWindowPos(windowHandle, fpl_null, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		appState->currentSettings.window.isDecorated = value;
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
	const fpl__Win32Api *wapi = &appState->win32.winApi;
	if (!appState->currentSettings.window.isFullscreen) {
		if (value) {
			wapi->user.SetWindowPos(windowState->windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		} else {
			wapi->user.SetWindowPos(windowState->windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		appState->currentSettings.window.isFloating = value;
	}
}

fpl_platform_api bool fplIsWindowFullscreen() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isFullscreen != 0;
	return(result);
}


fpl_platform_api bool fplSetWindowFullscreenSize(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
	bool result = fpl__Win32SetWindowFullscreen(value, INT32_MAX, INT32_MAX, fullscreenWidth, fullscreenHeight, refreshRate, true);
	return(result);
}

fpl_platform_api bool fplSetWindowFullscreenRect(const bool value, const int32_t x, const int32_t y, const int32_t width, const int32_t height) {
	bool result = fpl__Win32SetWindowFullscreen(value, x, y, width, height, 0, false);
	return(result);
}

fpl_platform_api bool fplEnableWindowFullscreen() {
	bool result = fpl__Win32SetWindowFullscreen(true, INT32_MAX, INT32_MAX, 0, 0, 0, false);
	return(result);
}

fpl_platform_api bool fplDisableWindowFullscreen() {
	bool result = fpl__Win32SetWindowFullscreen(false, 0, 0, 0, 0, 0, false);
	return(result);
}

fpl_platform_api bool fplGetWindowPosition(fplWindowPosition *outPos) {
	FPL__CheckArgumentNull(outPos, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	WINDOWPLACEMENT placement = fplZeroInit;
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

fpl_platform_api void fplSetWindowTitle(const char *title) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32AppState *win32AppState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	HWND handle = windowState->windowHandle;
	fplCopyString(title, appState->currentSettings.window.title, fplArrayCount(appState->currentSettings.window.title));
	wchar_t titleWide[FPL_MAX_BUFFER_LENGTH];
	fplUTF8StringToWideString(title, fplGetStringLength(title), titleWide, fplArrayCount(titleWide));
	wapi->user.SetWindowTextW(handle, titleWide);
}

fpl_platform_api void fplSetWindowPosition(const int32_t left, const int32_t top) {
	FPL__CheckPlatformNoRet();
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	WINDOWPLACEMENT placement = fplZeroInit;
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

fpl_platform_api fplWindowState fplGetWindowState() {
	FPL__CheckPlatform(fplWindowState_Unknown);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	HWND windowHandle = windowState->windowHandle;
	fplWindowState result;
	if (appState->currentSettings.window.isFullscreen) {
		result = fplWindowState_Fullscreen;
	} else {
		bool isMaximized = !!wapi->user.IsZoomed(windowHandle);
		bool isMinimized = !!wapi->user.IsIconic(windowHandle);
		if (isMinimized) {
			result = fplWindowState_Iconify;
		} else if (isMaximized) {
			result = fplWindowState_Maximize;
		} else {
			result = fplWindowState_Normal;
		}
	}
	return(result);
}

fpl_platform_api bool fplSetWindowState(const fplWindowState newState) {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	HWND windowHandle = windowState->windowHandle;
	bool result = false;
	switch (newState) {
		case fplWindowState_Iconify:
		{
			wapi->user.SendMessageW(windowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			result = true;
		} break;

		case fplWindowState_Maximize:
		{
			if (!appState->currentSettings.window.isFullscreen && appState->currentSettings.window.isResizable) {
				wapi->user.SendMessageW(windowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				result = true;
			}
		} break;

		case fplWindowState_Normal:
		{
			wapi->user.SendMessageW(windowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
			result = true;
		} break;

		default:
			break;
	}
	return(true);
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	FPL__CheckPlatformNoRet();
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	windowState->isCursorActive = value;
}

fpl_internal bool fpl__Win32ProcessNextEvent(const fpl__Win32Api *wapi, fpl__PlatformAppState *appState, fpl__Win32WindowState *windowState) {
	bool result = false;
	if (windowState->windowHandle != 0) {
		MSG msg;
		if (wapi->user.PeekMessageW(&msg, windowState->windowHandle, 0, 0, PM_REMOVE) == TRUE) {
			fpl__Win32HandleMessage(wapi, appState, windowState, &msg);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplPollEvent(fplEvent *ev) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;

	// Poll next event from the internal queue first
	if (fpl__PollInternalEvent(ev)) {
		return(true);
	}

	// Create new event from the OS message queue
	if (!fpl__Win32ProcessNextEvent(wapi, appState, windowState)) {
		return(false);
	}

	// Poll the first event from the internal queue
	if (fpl__PollInternalEvent(ev)) {
		return(true);
	}

	// No events left
	return(false);
}

fpl_platform_api void fplPollEvents() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	if (windowState->windowHandle != 0) {
		if (windowState->mainFiber != fpl_null && windowState->messageFiber != fpl_null) {
			SwitchToFiber(windowState->messageFiber);
		} else {
			MSG msg;
			while (wapi->user.PeekMessageW(&msg, windowState->windowHandle, 0, 0, PM_REMOVE)) {
				fpl__Win32HandleMessage(wapi, appState, windowState, &msg);
			}
		}
	}
	fpl__ClearInternalEvents();
}

fpl_platform_api bool fplWindowUpdate() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32InitState *win32InitState = &fpl__global__InitState.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	fpl__ClearInternalEvents();
	if ((!appState->currentSettings.input.disabledEvents) && (appState->initFlags & fplInitFlags_GameController)) {
		fpl__Win32UpdateGameControllers(&appState->currentSettings, win32InitState, &win32AppState->xinput);
	}
	bool result = appState->window.isRunning != 0;
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
	if (appState->window.isRunning) {
		appState->window.isRunning = false;
		const fpl__Win32Api *wapi = &win32AppState->winApi;
		wapi->user.PostQuitMessage(0);
	}
}

fpl_platform_api bool fplGetClipboardText(char *dest, const uint32_t maxDestLen) {
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		if (wapi->user.IsClipboardFormatAvailable(CF_UNICODETEXT)) {
			HGLOBAL dataHandle = wapi->user.GetClipboardData(CF_UNICODETEXT);
			if (dataHandle != fpl_null) {
				const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
				fplWideStringToUTF8String(stringValue, lstrlenW(stringValue), dest, maxDestLen);
				GlobalUnlock(dataHandle);
				result = true;
			}
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

fpl_platform_api bool fplSetClipboardText(const char *text) {
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	bool result = false;
	if (wapi->user.OpenClipboard(windowState->windowHandle)) {
		const size_t textLen = fplGetStringLength(text);
		const size_t bufferLen = textLen + 1;
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)bufferLen * sizeof(wchar_t));
		if (handle != fpl_null) {
			wchar_t *target = (wchar_t *)GlobalLock(handle);
			fplUTF8StringToWideString(text, textLen, target, bufferLen);
			GlobalUnlock(handle);
			wapi->user.EmptyClipboard();
			wapi->user.SetClipboardData(CF_UNICODETEXT, handle);
			result = true;
		}
		wapi->user.CloseClipboard();
	}
	return(result);
}

fpl_platform_api bool fplPollKeyboardState(fplKeyboardState *outState) {
	FPL__CheckArgumentNull(outState, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	fplClearStruct(outState);
	outState->modifiers = fpl__Win32GetKeyboardModifiers(wapi);
	for (uint32_t keyCode = 0; keyCode < 256; ++keyCode) {
		int k = wapi->user.MapVirtualKeyW(MAPVK_VSC_TO_VK, keyCode);
		if (k == 0) {
			k = (int)keyCode;
		}
		bool down = fpl__Win32IsKeyDown(wapi, k);
		fplKey key = fpl__GetMappedKey(&fpl__global__AppState->window, keyCode);
		outState->keyStatesRaw[keyCode] = down;
		outState->buttonStatesMapped[(int)key] = down ? fplButtonState_Press : fplButtonState_Release;
	}
	return(true);
}

fpl_platform_api bool fplPollGamepadStates(fplGamepadStates *outStates) {
	FPL__CheckArgumentNull(outStates, false);
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *platformAppState = fpl__global__AppState;
	if (platformAppState->initFlags & fplInitFlags_GameController) {
		fpl__Win32AppState *appState = &platformAppState->win32;
		const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
		const fpl__Win32Api *wapi = &appState->winApi;
		fpl__Win32XInputState *xinputState = &appState->xinput;
		fplAssert(xinputState != fpl_null);
		if (xinputState->xinputApi.XInputGetState != fpl_null) {
			// @TODO(final): fpl__Win32UpdateGameControllers() uses duplicate code
			QueryPerformanceCounter(&xinputState->lastDeviceSearchTime);

			fplClearStruct(outStates);
			for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
				XINPUT_STATE controllerState = fplZeroInit;
				if (xinputState->xinputApi.XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					if (!xinputState->isConnected[controllerIndex]) {
						xinputState->isConnected[controllerIndex] = true;
						fplStringFormat(xinputState->deviceNames[controllerIndex], fplArrayCount(xinputState->deviceNames[controllerIndex]), "XInput-Device [%d]", controllerIndex);
					}
					const XINPUT_GAMEPAD *newPadState = &controllerState.Gamepad;
					fplGamepadState *targetPadState = &outStates->deviceStates[controllerIndex];
					fpl__Win32XInputGamepadToGamepadState(newPadState, targetPadState);
					targetPadState->deviceName = xinputState->deviceNames[controllerIndex];
				} else {
					if (xinputState->isConnected[controllerIndex]) {
						xinputState->isConnected[controllerIndex] = false;
					}
				}
			}
			return(true);
		}
	}
	return(false);
}

fpl_platform_api bool fplQueryCursorPosition(int32_t *outX, int32_t *outY) {
	FPL__CheckArgumentNull(outX, false);
	FPL__CheckArgumentNull(outY, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	POINT p;
	if (wapi->user.GetCursorPos(&p) == TRUE) {
#if 0
		HMONITOR monitor = wapi->user.MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST);
		if (monitor != fpl_null) {
			MONITORINFOEXW info = fplZeroInit;
			info.cbSize = sizeof(info);
			if (wapi->user.GetMonitorInfoW(monitor, (LPMONITORINFO)&info) != 0) {
				*outX = p.x - info.rcMonitor.left;
				*outY = p.y - info.rcMonitor.top;
				return(true);
			}
		}
#else
		*outX = p.x;
		*outY = p.y;
		return(true);
#endif
	}
	return(false);
}

fpl_platform_api bool fplPollMouseState(fplMouseState *outState) {
	FPL__CheckArgumentNull(outState, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	POINT p;
	if ((wapi->user.GetCursorPos(&p) == TRUE) && (wapi->user.ScreenToClient(windowState->windowHandle, &p))) {
		fplClearStruct(outState);
		outState->x = p.x;
		outState->y = p.y;

		bool leftDown = fpl__Win32IsKeyDown(wapi, VK_LBUTTON);
		bool rightDown = fpl__Win32IsKeyDown(wapi, VK_RBUTTON);
		bool middleDown = fpl__Win32IsKeyDown(wapi, VK_MBUTTON);
		outState->buttonStates[fplMouseButtonType_Left] = leftDown ? fplButtonState_Press : fplButtonState_Release;
		outState->buttonStates[fplMouseButtonType_Right] = rightDown ? fplButtonState_Press : fplButtonState_Release;
		outState->buttonStates[fplMouseButtonType_Middle] = middleDown ? fplButtonState_Press : fplButtonState_Release;

		return(true);
	}
	return(false);
}

fpl_internal BOOL WINAPI fpl__Win32MonitorCountEnumProc(HMONITOR monitorHandle, HDC hdc, LPRECT rect, LPARAM userData) {
	size_t *count = (size_t *)(uintptr_t)userData;
	*count = *count + 1;
	return(TRUE);
}

fpl_platform_api size_t fplGetDisplayCount() {
	FPL__CheckPlatform(0);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	size_t result = 0;
	LPARAM param = (LPARAM)&result;
	wapi->user.EnumDisplayMonitors(fpl_null, fpl_null, fpl__Win32MonitorCountEnumProc, param);
	return(result);
}
fpl_internal void fpl__Win32FillDisplayInfo(const MONITORINFOEXW *info, fplDisplayInfo *outInfo) {
	fplAssert(info != fpl_null);
	fplAssert(outInfo != fpl_null);
	size_t idLen = lstrlenW(info->szDevice);
	fplWideStringToUTF8String(info->szDevice, idLen, outInfo->id, fplArrayCount(outInfo->id));
	outInfo->virtualPosition.left = info->rcMonitor.left;
	outInfo->virtualPosition.top = info->rcMonitor.top;
	outInfo->virtualSize.width = info->rcMonitor.right - info->rcMonitor.left;
	outInfo->virtualSize.height = info->rcMonitor.bottom - info->rcMonitor.top;
	outInfo->physicalSize.width = (info->rcMonitor.right > info->rcMonitor.left) ? (info->rcMonitor.right - info->rcMonitor.left) : (info->rcMonitor.left - info->rcMonitor.right);
	outInfo->physicalSize.height = (info->rcMonitor.bottom > info->rcMonitor.top) ? (info->rcMonitor.bottom - info->rcMonitor.top) : (info->rcMonitor.top - info->rcMonitor.bottom);
	outInfo->isPrimary = (info->dwFlags & MONITORINFOF_PRIMARY) ? 1 : 0;
}

typedef struct fpl__Win32DisplayEnumState {
	fplDisplayInfo *baseInfo;
	const fpl__Win32Api *wapi;
	size_t count;
	size_t maxCount;
} fpl__Win32DisplayEnumState;

fpl_internal BOOL WINAPI fpl__Win32MonitorInfoEnumProc(HMONITOR monitorHandle, HDC hdc, LPRECT rect, LPARAM userData) {
	fpl__Win32DisplayEnumState *enumState = (fpl__Win32DisplayEnumState *)(uintptr_t)userData;
	const fpl__Win32Api *wapi = enumState->wapi;
	if (enumState->count < enumState->maxCount) {
		fplDisplayInfo *targetInfo = enumState->baseInfo + enumState->count;
		fplClearStruct(targetInfo);
		MONITORINFOEXW info = fplZeroInit;
		info.cbSize = sizeof(info);
		if (wapi->user.GetMonitorInfoW(monitorHandle, (LPMONITORINFO)&info) != 0) {
			fpl__Win32FillDisplayInfo(&info, targetInfo);
		}
		++enumState->count;
		BOOL result = enumState->count < enumState->maxCount;
		return(result);
	} else {
		return(FALSE);
	}
}

fpl_platform_api size_t fplGetDisplays(fplDisplayInfo *displays, const size_t maxDisplayCount) {
	FPL__CheckArgumentNull(displays, 0);
	FPL__CheckArgumentZero(maxDisplayCount, 0);
	FPL__CheckPlatform(0);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	fpl__Win32DisplayEnumState enumState = fplZeroInit;
	enumState.baseInfo = displays;
	enumState.maxCount = maxDisplayCount;
	enumState.wapi = wapi;
	LPARAM param = (LPARAM)&enumState;
	wapi->user.EnumDisplayMonitors(fpl_null, fpl_null, fpl__Win32MonitorInfoEnumProc, param);
	return(enumState.count);
}

fpl_internal BOOL WINAPI fpl__Win32PrimaryMonitorEnumProc(HMONITOR monitorHandle, HDC hdc, LPRECT rect, LPARAM userData) {
	fpl__Win32DisplayEnumState *enumState = (fpl__Win32DisplayEnumState *)(uintptr_t)userData;
	const fpl__Win32Api *wapi = enumState->wapi;
	MONITORINFOEXW info = fplZeroInit;
	info.cbSize = sizeof(info);
	if (wapi->user.GetMonitorInfoW(monitorHandle, (LPMONITORINFO)&info) != 0) {
		if (info.dwFlags & MONITORINFOF_PRIMARY) {
			fplClearStruct(enumState->baseInfo);
			fpl__Win32FillDisplayInfo(&info, enumState->baseInfo);
			enumState->count = 1;
			return(FALSE);
		}
	}
	return(TRUE);
}

fpl_platform_api bool fplGetPrimaryDisplay(fplDisplayInfo *display) {
	FPL__CheckArgumentNull(display, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	fpl__Win32DisplayEnumState enumState = fplZeroInit;
	enumState.baseInfo = display;
	enumState.maxCount = 1;
	enumState.wapi = wapi;
	LPARAM param = (LPARAM)&enumState;
	wapi->user.EnumDisplayMonitors(fpl_null, fpl_null, fpl__Win32PrimaryMonitorEnumProc, param);
	bool result = (enumState.count == 1);
	return(result);
}

fpl_platform_api bool fplGetWindowDisplay(fplDisplayInfo *outDisplay) {
	FPL__CheckArgumentNull(outDisplay, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HMONITOR foundMonitor = wapi->user.MonitorFromWindow(windowState->windowHandle, MONITOR_DEFAULTTONULL);
	bool result = false;
	if (foundMonitor != fpl_null) {
		MONITORINFOEXW info = fplZeroInit;
		info.cbSize = sizeof(info);
		if (wapi->user.GetMonitorInfoW(foundMonitor, (LPMONITORINFO)&info) != 0) {
			fplClearStruct(outDisplay);
			fpl__Win32FillDisplayInfo(&info, outDisplay);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplGetDisplayFromPosition(const int32_t x, const int32_t y, fplDisplayInfo *outDisplay) {
	FPL__CheckArgumentNull(outDisplay, false);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	POINT pt;
	pt.x = x;
	pt.y = y;
	bool result = false;
	HMONITOR foundMonitor = wapi->user.MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
	if (foundMonitor != fpl_null) {
		MONITORINFOEXW info = fplZeroInit;
		info.cbSize = sizeof(info);
		if (wapi->user.GetMonitorInfoW(foundMonitor, (LPMONITORINFO)&info) != 0) {
			fplClearStruct(outDisplay);
			fpl__Win32FillDisplayInfo(&info, outDisplay);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api size_t fplGetDisplayModes(const char *id, fplDisplayMode *modes, const size_t maxDisplayModeCount) {
	FPL__CheckArgumentNull(id, 0);
	FPL__CheckPlatform(0);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32WindowState *windowState = &fpl__global__AppState->window.win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	wchar_t deviceName[CCHDEVICENAME + 1];
	fplUTF8StringToWideString(id, fplGetStringLength(id), deviceName, fplArrayCount(deviceName));
	size_t result = 0;
	DEVMODEW devMode;
	while (wapi->user.EnumDisplaySettingsW(deviceName, (DWORD)result, &devMode)) {
		if (modes != fpl_null) {
			if (result == maxDisplayModeCount) {
				break;
			}
			fplDisplayMode *outMode = modes + result;
			fplClearStruct(outMode);
			outMode->width = devMode.dmPelsWidth;
			outMode->height = devMode.dmPelsHeight;
			outMode->colorBits = devMode.dmBitsPerPel;
			outMode->refreshRate = devMode.dmDisplayFrequency;
		}
		++result;
	}
	return(result);
}

#endif // FPL__ENABLE_WINDOW

fpl_internal LCTYPE fpl__Win32GetLocaleLCIDFromFormat(const fplLocaleFormat format) {
	switch (format) {
		case fplLocaleFormat_ISO639:
			return LOCALE_SNAME;
		default:
			return LOCALE_SABBREVLANGNAME;
	}
}

fpl_platform_api size_t fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	wchar_t bufferWide[FPL_MAX_BUFFER_LENGTH];
	int r = GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT, lcType, bufferWide, fplArrayCount(bufferWide));
	size_t result = fplWideStringToUTF8String(bufferWide, lstrlenW(bufferWide), buffer, maxBufferLen);
	return(result);
}

fpl_platform_api size_t fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	wchar_t bufferWide[FPL_MAX_BUFFER_LENGTH];
	int r = GetLocaleInfoW(LOCALE_USER_DEFAULT, lcType, bufferWide, fplArrayCount(bufferWide));
	size_t result = fplWideStringToUTF8String(bufferWide, lstrlenW(bufferWide), buffer, maxBufferLen);
	return(result);
}

fpl_platform_api size_t fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	FPL__CheckPlatform(false);
	const fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	HKL kbLayout = wapi->user.GetKeyboardLayout(GetCurrentThreadId());
	LCID langId = (DWORD)(intptr_t)kbLayout & 0xFFFF;
	LCTYPE lcType = fpl__Win32GetLocaleLCIDFromFormat(targetFormat);
	wchar_t bufferWide[FPL_MAX_BUFFER_LENGTH];
	int r = GetLocaleInfoW(langId, lcType, bufferWide, fplArrayCount(bufferWide));
	size_t result = fplWideStringToUTF8String(bufferWide, lstrlenW(bufferWide), buffer, maxBufferLen);
	return(result);
}
#endif // FPL_PLATFORM_WINDOWS

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
		FPL__ERROR(FPL__MODULE_POSIX, "Failed initializing PThread API");
		return false;
	}
	return true;
}

fpl_internal void fpl__InitWaitTimeSpec(const uint32_t milliseconds, struct timespec *outSpec) {
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

void *fpl__PosixThreadProc(void *data) {
	fplAssert(fpl__global__AppState != fpl_null);
	const fpl__PThreadApi *pthreadApi = &fpl__global__AppState->posix.pthreadApi;
	fplThreadHandle *thread = (fplThreadHandle *)data;
	fplAssert(thread != fpl_null);
	fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Running);

	fplThreadParameters parameters = thread->parameters;
	if (parameters.runFunc != fpl_null) {
		parameters.runFunc(thread, parameters.userData);
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
	int mutexRes;
	do {
		mutexRes = pthreadApi->pthread_mutex_init(handle, fpl_null);
	} while (mutexRes == EAGAIN);
	return(mutexRes);
}

fpl_internal bool fpl__PosixThreadWaitForMultiple(fplThreadHandle **threads, const uint32_t minCount, const uint32_t maxCount, const size_t stride, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(threads, false);
	FPL__CheckArgumentMax(maxCount, FPL_MAX_THREAD_COUNT, false);
	const size_t actualStride = stride > 0 ? stride : sizeof(fplThreadHandle *);
	for (uint32_t index = 0; index < maxCount; ++index) {
		fplThreadHandle *thread = *(fplThreadHandle **)((uint8_t *)threads + index * actualStride);
		if (thread == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Thread for index '%d' are not allowed to be null", index);
			return false;
		}
	}

	uint32_t completeCount = 0;
	bool isRunning[FPL_MAX_THREAD_COUNT];
	for (uint32_t index = 0; index < maxCount; ++index) {
		fplThreadHandle *thread = *(fplThreadHandle **)((uint8_t *)threads + index * actualStride);
		isRunning[index] = fplGetThreadState(thread) != fplThreadState_Stopped;
		if (!isRunning[index]) {
			++completeCount;
		}
	}

	fplMilliseconds startTime = fplMillisecondsQuery();
	bool result = false;
	while (completeCount < minCount) {
		for (uint32_t index = 0; index < maxCount; ++index) {
			fplThreadHandle *thread = *(fplThreadHandle **)((uint8_t *)threads + index * actualStride);
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
		if ((timeout != FPL_TIMEOUT_INFINITE) && (fplMillisecondsQuery() - startTime) >= timeout) {
			result = false;
			break;
		}
	}
	return(result);
}

//
// POSIX Atomics
//
#if defined(FPL_COMPILER_GCC) || defined(FPL_COMPILER_CLANG) || defined(__GNUC__)
// @NOTE(final): See: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#g_t_005f_005fsync-Builtins
// @NOTE(final): There is only one barrier in POSIX (read and write)
fpl_platform_api void fplAtomicReadFence() {
	__sync_synchronize();
}
fpl_platform_api void fplAtomicWriteFence() {
	__sync_synchronize();
}
fpl_platform_api void fplAtomicReadWriteFence() {
	__sync_synchronize();
}

fpl_platform_api uint32_t fplAtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
	__sync_synchronize();
	uint32_t result = __sync_lock_test_and_set(target, value);
	return(result);
}
fpl_platform_api uint64_t fplAtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	__sync_synchronize();
	uint64_t result = __sync_lock_test_and_set(target, value);
	return(result);
}
fpl_platform_api int32_t fplAtomicExchangeS32(volatile int32_t *target, const int32_t value) {
	__sync_synchronize();
	int32_t result = __sync_lock_test_and_set(target, value);
	return(result);
}
fpl_platform_api int64_t fplAtomicExchangeS64(volatile int64_t *target, const int64_t value) {
	__sync_synchronize();
	int64_t result = __sync_lock_test_and_set(target, value);
	return(result);
}

fpl_platform_api uint32_t fplAtomicFetchAndAddU32(volatile uint32_t *value, const uint32_t addend) {
	fplAssert(value != fpl_null);
	uint32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicFetchAndAddU64(volatile uint64_t *value, const uint64_t addend) {
	fplAssert(value != fpl_null);
	uint64_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicFetchAndAddS32(volatile int32_t *value, const int32_t addend) {
	fplAssert(value != fpl_null);
	int32_t result = __sync_fetch_and_add(value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicFetchAndAddS64(volatile int64_t *value, const int64_t addend) {
	fplAssert(value != fpl_null);
	int64_t result = __sync_fetch_and_add(value, addend);
	return (result);
}

fpl_platform_api uint32_t fplAtomicAddAndFetchU32(volatile uint32_t *value, const uint32_t addend) {
	fplAssert(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, addend);
	return (result);
}
fpl_platform_api int32_t fplAtomicAddAndFetchS32(volatile int32_t *value, const int32_t addend) {
	fplAssert(value != fpl_null);
	int32_t result = __sync_add_and_fetch(value, addend);
	return (result);
}
fpl_platform_api uint64_t fplAtomicAddAndFetchU64(volatile uint64_t *value, const uint64_t addend) {
	fplAssert(value != fpl_null);
	uint64_t result = __sync_add_and_fetch(value, addend);
	return (result);
}
fpl_platform_api int64_t fplAtomicAddAndFetchS64(volatile int64_t *value, const int64_t addend) {
	fplAssert(value != fpl_null);
	int64_t result = __sync_add_and_fetch(value, addend);
	return (result);
}

fpl_platform_api uint32_t fplAtomicIncrementU32(volatile uint32_t *value) {
	fplAssert(value != fpl_null);
	uint32_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api uint64_t fplAtomicIncrementU64(volatile uint64_t *value) {
	fplAssert(value != fpl_null);
	uint64_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api int32_t fplAtomicIncrementS32(volatile int32_t *value) {
	fplAssert(value != fpl_null);
	int32_t result = __sync_add_and_fetch(value, 1);
	return (result);
}
fpl_platform_api int64_t fplAtomicIncrementS64(volatile int64_t *value) {
	fplAssert(value != fpl_null);
	int64_t result = __sync_add_and_fetch(value, 1);
	return (result);
}

fpl_platform_api uint32_t fplAtomicCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	fplAssert(dest != fpl_null);
	uint32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api uint64_t fplAtomicCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	fplAssert(dest != fpl_null);
	uint64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int32_t fplAtomicCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	fplAssert(dest != fpl_null);
	int32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api int64_t fplAtomicCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	fplAssert(dest != fpl_null);
	int64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
	return (result);
}

fpl_platform_api bool fplAtomicIsCompareAndSwapU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
	fplAssert(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
	fplAssert(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
	fplAssert(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}
fpl_platform_api bool fplAtomicIsCompareAndSwapS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
	fplAssert(dest != fpl_null);
	bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
	return (result);
}

fpl_platform_api uint32_t fplAtomicLoadU32(volatile uint32_t *source) {
	uint32_t result = __sync_add_and_fetch(source, 0);
	return(result);
}
fpl_platform_api uint64_t fplAtomicLoadU64(volatile uint64_t *source) {
	uint64_t result = __sync_add_and_fetch(source, 0);
	return(result);
}
fpl_platform_api int32_t fplAtomicLoadS32(volatile int32_t *source) {
	int32_t result = __sync_add_and_fetch(source, 0);
	return(result);
}
fpl_platform_api int64_t fplAtomicLoadS64(volatile int64_t *source) {
	int64_t result = __sync_add_and_fetch(source, 0);
	return(result);
}

fpl_platform_api void fplAtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
	__sync_synchronize();
	__sync_lock_test_and_set(dest, value);
}
fpl_platform_api void fplAtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
	__sync_synchronize();
	__sync_lock_test_and_set(dest, value);
}
fpl_platform_api void fplAtomicStoreS32(volatile int32_t *dest, const int32_t value) {
	__sync_synchronize();
	__sync_lock_test_and_set(dest, value);
}
fpl_platform_api void fplAtomicStoreS64(volatile int64_t *dest, const int64_t value) {
	__sync_synchronize();
	__sync_lock_test_and_set(dest, value);
}
#else
#	error "This POSIX compiler/platform is not supported!"
#endif

//
// POSIX Timings
//
fpl_platform_api fplTimestamp fplTimestampQuery() {
	fplTimestamp result = fplZeroInit;
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	result.posix.seconds = (uint64_t)t.tv_sec;
	result.posix.nanoSeconds = (int64_t)t.tv_nsec;
	return(result);
}

fpl_platform_api fplSeconds fplTimestampElapsed(const fplTimestamp start, const fplTimestamp finish) {
	uint64_t deltaSeconds = finish.posix.seconds - start.posix.seconds;
	int64_t deltaNanos = finish.posix.nanoSeconds - start.posix.nanoSeconds;
	if (deltaNanos < 0) {
		--deltaSeconds;
		deltaNanos += 1000000000L;
	}
	fplSeconds result = (fplSeconds)deltaSeconds + ((fplSeconds)deltaNanos * 1e-9);
	return(result);
}

fpl_platform_api fplMilliseconds fplMillisecondsQuery() {
	struct timeval  tv;
	gettimeofday(&tv, fpl_null);
	fplMilliseconds result = (fplMilliseconds)(tv.tv_sec * 1000 + ((uint64_t)tv.tv_usec / 1000));
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
	if (thread->isValid && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		pthread_t threadHandle = thread->internalHandle.posixThread;
		if (pthreadApi->pthread_kill(threadHandle, 0) == 0) {
			pthreadApi->pthread_join(threadHandle, fpl_null);
		}
		thread->isValid = false;
		fplAtomicStoreU32((volatile uint32_t *)&thread->currentState, (uint32_t)fplThreadState_Stopped);
		return true;
	} else {
		return false;
	}
}

fpl_platform_api uint32_t fplGetCurrentThreadId() {
	FPL__CheckPlatform(0);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_t currentThread = pthreadApi->pthread_self();
	uint32_t result = (uint32_t)currentThread;
	return(result);
}

fpl_platform_api fplThreadHandle *fplThreadCreateWithParameters(fplThreadParameters *parameters) {
	FPL__CheckArgumentNull(parameters, fpl_null);
	FPL__CheckPlatform(fpl_null);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	fplThreadHandle *result = fpl_null;
	fplThreadHandle *thread = fpl__GetFreeThread();
	if (thread != fpl_null) {
		thread->currentState = fplThreadState_Stopped;
		thread->parameters = *parameters;
		thread->isValid = false;
		thread->isStopping = false;

		fplThreadPriority initialPriority = parameters->priority;

		// Setup attributes
		pthread_attr_t *attrPtr = fpl_null;
		pthread_attr_t attr;
		if (pthreadApi->pthread_attr_init(&attr) == 0) {
			// Scheduler policy
			int scheduler = -1;
			if (initialPriority == fplThreadPriority_Idle) {
#if defined(SCHED_IDLE)
				if (pthreadApi->pthread_attr_setschedpolicy(&attr, SCHED_IDLE) == 0) {
					scheduler = SCHED_IDLE;
				}
#endif
			} else if (initialPriority >= fplThreadPriority_High) {
#if defined(SCHED_FIFO)
				if ((scheduler == -1) && (pthreadApi->pthread_attr_setschedpolicy(&attr, SCHED_FIFO) == 0)) {
					scheduler = SCHED_FIFO;
				}
#endif
#if defined(SCHED_RR)
				if ((scheduler == -1) && (pthreadApi->pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0)) {
					scheduler = SCHED_RR;
				}
#endif
			} else {
				// @TODO(final): Is sched_getscheduler supported for all POSIX standards?
				scheduler = sched_getscheduler(0);
			}

			// Stack size
			if (parameters->stackSize > 0) {
				pthreadApi->pthread_attr_setstacksize(&attr, parameters->stackSize);
			}

			// Priority
			if (scheduler != -1) {
				struct sched_param sched;
				if (pthreadApi->pthread_attr_getschedparam(&attr, &sched) == 0) {
					int maxThreadPrioCount = (fplThreadPriority_Last - fplThreadPriority_First) + 1;
					fplAssert(maxThreadPrioCount > 0);

					int minPrio = sched_get_priority_min(scheduler);
					int maxPrio = sched_get_priority_max(scheduler);
					int range = maxPrio - minPrio;
					int step = range / maxThreadPrioCount;

					int priority;
					if (initialPriority == fplThreadPriority_Lowest) {
						priority = minPrio;
					} else if (initialPriority == fplThreadPriority_Highest) {
						priority = maxPrio;
					} else {
						int threadPrioNumber = (int)(initialPriority - fplThreadPriority_First) + 1;
						priority = minPrio + threadPrioNumber * step;
						if (priority < minPrio) {
							priority = minPrio;
						} else if (priority > maxPrio) {
							priority = maxPrio;
						}
					}

					sched.sched_priority = priority;
					pthreadApi->pthread_attr_setschedparam(&attr, &sched);
				}
			}

			attrPtr = &attr;
		}

		// Create thread
		thread->currentState = fplThreadState_Starting;
		int threadRes;
		do {
			threadRes = pthreadApi->pthread_create(&thread->internalHandle.posixThread, attrPtr, fpl__PosixThreadProc, (void *)thread);
		} while (threadRes == EAGAIN);
		if (threadRes != 0) {
			FPL__ERROR(FPL__MODULE_THREADING, "Failed creating thread, error code: %d", threadRes);
		}
		if (threadRes == 0) {
			thread->id = (uint32_t)thread->internalHandle.posixThread;
			thread->isValid = true;
			result = thread;
		} else {
			fplClearStruct(thread);
		}
	} else {
		FPL__ERROR(FPL__MODULE_THREADING, "All %d threads are in use, you cannot create until you free one", FPL_MAX_THREAD_COUNT);
	}
	return(result);
}

fpl_platform_api fplThreadHandle *fplThreadCreate(fpl_run_thread_callback *runFunc, void *data) {
	FPL__CheckArgumentNull(runFunc, fpl_null);
	fplThreadParameters parameters = fplZeroInit;
	parameters.runFunc = runFunc;
	parameters.userData = data;
	fplThreadHandle *result = fplThreadCreateWithParameters(&parameters);
	return(result);
}

fpl_platform_api fplThreadPriority fplGetThreadPriority(fplThreadHandle *thread) {
	FPL__CheckPlatform(fplThreadPriority_Unknown);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;

	pthread_t curThread = pthreadApi->pthread_self();

	int currentSchedulerPolicy;
	struct sched_param params;
	if (pthreadApi->pthread_getschedparam(curThread, &currentSchedulerPolicy, &params) != 0) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed getting scheduler parameters for pthread '%d'", curThread);
		return(fplThreadPriority_Unknown);
	}

	int maxThreadPrioCount = (fplThreadPriority_Last - fplThreadPriority_First) + 1;
	fplAssert(maxThreadPrioCount > 0);

	int minPrio = sched_get_priority_min(currentSchedulerPolicy);
	int maxPrio = sched_get_priority_max(currentSchedulerPolicy);
	int range = maxPrio - minPrio;
	int step = range / maxThreadPrioCount;

	int currentPrio = params.sched_priority;

	fplThreadPriority result;
	if (minPrio == maxPrio || currentPrio == minPrio) {
		result = fplThreadPriority_Lowest;
	} else if (currentPrio == maxPrio) {
		result = fplThreadPriority_Highest;
	} else {
		int index = (currentPrio - minPrio) / step;
		fplAssert(index >= 0 && index < maxThreadPrioCount);
		result = (fplThreadPriority)index;
	}

	return(result);
}

fpl_platform_api bool fplSetThreadPriority(fplThreadHandle *thread, const fplThreadPriority newPriority) {
	if (newPriority == fplThreadPriority_Unknown) return(false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;

	pthread_t curThread = pthreadApi->pthread_self();

	int currentSchedulerPolicy;
	struct sched_param params;
	if (pthreadApi->pthread_getschedparam(curThread, &currentSchedulerPolicy, &params) != 0) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed getting scheduler parameters for pthread '%d'", curThread);
		return(false);
	}

	// Build policy table
	int newSchedulerPolicies[3];
	int schedulerPolicyCount = 0;
	switch (newPriority) {
		case fplThreadPriority_Idle:
		case fplThreadPriority_Low:
		case fplThreadPriority_Normal:
			newSchedulerPolicies[schedulerPolicyCount++] = currentSchedulerPolicy;
			break;
		case fplThreadPriority_High:
#if defined(SCHED_RR)
			newSchedulerPolicies[schedulerPolicyCount++] = SCHED_RR;
#endif
			newSchedulerPolicies[schedulerPolicyCount++] = currentSchedulerPolicy;
			break;
		case fplThreadPriority_RealTime:
#if defined(SCHED_FIFO)
			newSchedulerPolicies[schedulerPolicyCount++] = SCHED_FIFO;
#endif
#if defined(SCHED_RR)
			newSchedulerPolicies[schedulerPolicyCount++] = SCHED_RR;
#endif
			newSchedulerPolicies[schedulerPolicyCount++] = currentSchedulerPolicy;
			break;
		default:
			break;
	}

	int maxThreadPrioCount = (fplThreadPriority_Last - fplThreadPriority_First) + 1;
	fplAssert(maxThreadPrioCount > 0);

	// Bring priority in range of 1-N
	int threadPrioNumber = (int)(newPriority - fplThreadPriority_First) + 1;

	for (int i = 0; i < schedulerPolicyCount; ++i) {
		int policy = newSchedulerPolicies[i];
		int minPrio = sched_get_priority_min(policy);
		int maxPrio = sched_get_priority_max(policy);
		int range = maxPrio - minPrio;
		int step = range / maxThreadPrioCount;

		int priority;
		if (newPriority == fplThreadPriority_Lowest) {
			priority = minPrio;
		} else if (newPriority == fplThreadPriority_Highest) {
			priority = maxPrio;
		} else {
			priority = minPrio + threadPrioNumber * step;
			if (priority < minPrio) {
				priority = minPrio;
			} else if (priority > maxPrio) {
				priority = maxPrio;
			}
		}
		params.sched_priority = priority;
		if (pthreadApi->pthread_setschedparam(curThread, policy, &params) == 0) {
			return(true); // Finally we found a policy and priority which is supported		
		} else {
			FPL__WARNING(FPL__MODULE_THREADING, "Failed to set thread priority '%d' with policy '%d'", priority, policy);
		}
	}

	return(false);
}

fpl_platform_api bool fplThreadWaitForOne(fplThreadHandle *thread, const fplTimeoutValue timeout) {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if ((thread != fpl_null) && (fplGetThreadState(thread) != fplThreadState_Stopped)) {
		pthread_t threadHandle = thread->internalHandle.posixThread;

		// @NOTE(final): We optionally use the GNU extension "pthread_timedjoin_np" to support joining with a timeout.
		int joinRes;
		if ((pthreadApi->pthread_timedjoin_np != fpl_null) && (timeout != FPL_TIMEOUT_INFINITE)) {
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

fpl_platform_api bool fplThreadWaitForAll(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__PosixThreadWaitForMultiple(threads, count, count, stride, timeout);
	return(result);
}

fpl_platform_api bool fplThreadWaitForAny(fplThreadHandle **threads, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__PosixThreadWaitForMultiple(threads, 1, count, stride, timeout);
	return(result);
}

fpl_platform_api bool fplThreadYield() {
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result;
	if (pthreadApi->pthread_yield != fpl_null) {
		result = (pthreadApi->pthread_yield() == 0);
	} else {
		result = (sched_yield() == 0);
	}
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
	struct timespec input, output;
	input.tv_sec = s;
	input.tv_nsec = ms * 1000000;
	nanosleep(&input, &output);
}

fpl_platform_api bool fplMutexInit(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	if (mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex '%p' is already initialized", mutex);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	fplAssert(sizeof(fpl__POSIXMutexHandle) >= sizeof(pthread_mutex_t));
	pthread_mutex_t mutexHandle;
	int mutexRes = fpl__PosixMutexCreate(pthreadApi, &mutexHandle);
	if (mutexRes != 0) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed creating POSIX condition");
		return false;
	}
	fplClearStruct(mutex);
	fplMemoryCopy(&mutexHandle, sizeof(mutexHandle), &mutex->internalHandle.posixMutex);
	mutex->isValid = true;
	return(true);
}

fpl_platform_api void fplMutexDestroy(fplMutexHandle *mutex) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if ((mutex != fpl_null) && mutex->isValid) {
		pthread_mutex_t *handle = (pthread_mutex_t *)&mutex->internalHandle.posixMutex;
		pthreadApi->pthread_mutex_destroy(handle);
		fplClearStruct(mutex);
	}
}

fpl_platform_api bool fplMutexLock(fplMutexHandle *mutex) {
	FPL__CheckArgumentNull(mutex, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	bool result = false;
	if (mutex->isValid) {
		pthread_mutex_t *handle = (pthread_mutex_t *)&mutex->internalHandle.posixMutex;
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
	if (mutex->isValid) {
		pthread_mutex_t *handle = (pthread_mutex_t *)&mutex->internalHandle.posixMutex;
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
	if (mutex->isValid) {
		pthread_mutex_t *handle = (pthread_mutex_t *)&mutex->internalHandle.posixMutex;
		result = fpl__PosixMutexUnlock(pthreadApi, handle);
	}
	return (result);
}

fpl_platform_api bool fplConditionInit(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	fplAssert(sizeof(fpl__POSIXConditionVariable) >= sizeof(pthread_cond_t));
	pthread_cond_t handle = PTHREAD_COND_INITIALIZER;
	int condRes;
	do {
		condRes = pthreadApi->pthread_cond_init(&handle, fpl_null);
	} while (condRes == EAGAIN);
	if (condRes == 0) {
		fplClearStruct(condition);
		fplMemoryCopy(&handle, sizeof(handle), &condition->internalHandle.posixCondition);
		condition->isValid = true;
	}
	return(condition->isValid);
}

fpl_platform_api void fplConditionDestroy(fplConditionVariable *condition) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if ((condition != fpl_null) && condition->isValid) {
		pthread_cond_t *handle = (pthread_cond_t *)&condition->internalHandle.posixCondition;
		pthreadApi->pthread_cond_destroy(handle);
		fplClearStruct(condition);
	}
}

fpl_platform_api bool fplConditionWait(fplConditionVariable *condition, fplMutexHandle *mutex, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(condition, false);
	FPL__CheckArgumentNull(mutex, false);
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	if (!mutex->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Mutex is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *cond = (pthread_cond_t *)&condition->internalHandle.posixCondition;
	pthread_mutex_t *mut = (pthread_mutex_t *)&mutex->internalHandle.posixMutex;
	bool result;
	if (timeout == FPL_TIMEOUT_INFINITE) {
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
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *handle = (pthread_cond_t *)&condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_signal(handle) == 0;
	return(result);
}

fpl_platform_api bool fplConditionBroadcast(fplConditionVariable *condition) {
	FPL__CheckArgumentNull(condition, false);
	if (!condition->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Condition is not valid");
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	pthread_cond_t *handle = (pthread_cond_t *)&condition->internalHandle.posixCondition;
	bool result = pthreadApi->pthread_cond_broadcast(handle) == 0;
	return(result);
}

fpl_platform_api bool fplSemaphoreInit(fplSemaphoreHandle *semaphore, const uint32_t initialValue) {
	FPL__CheckArgumentNull(semaphore, false);
	FPL__CheckArgumentMax(initialValue, UINT32_MAX, false);
	if (semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is already initialized", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);

	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	sem_t handle;
	int res = pthreadApi->sem_init(&handle, 0, (int)initialValue);
	if (res < 0) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed creating semaphore");
		return false;
	}
	fplClearStruct(semaphore);
	fplMemoryCopy(&handle, sizeof(handle), &semaphore->internalHandle.posixHandle);
	semaphore->isValid = true;
	return true;
}

fpl_platform_api void fplSemaphoreDestroy(fplSemaphoreHandle *semaphore) {
	FPL__CheckPlatformNoRet();
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	if (semaphore != fpl_null) {
		if (semaphore->isValid) {
			sem_t *handle = (sem_t *)&semaphore->internalHandle.posixHandle;
			pthreadApi->sem_destroy(handle);
		}
		fplClearStruct(semaphore);
	}
}

fpl_platform_api bool fplSemaphoreWait(fplSemaphoreHandle *semaphore, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	sem_t *handle = (sem_t *)&semaphore->internalHandle.posixHandle;
	int res;
	if (timeout == FPL_TIMEOUT_INFINITE) {
		res = pthreadApi->sem_wait(handle);
	} else {
		struct timespec t;
		fpl__InitWaitTimeSpec(timeout, &t);
		res = pthreadApi->sem_timedwait(handle, &t);
	}
	bool result = res == 0;
	return(result);
}

fpl_platform_api bool fplSemaphoreTryWait(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(false);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	sem_t *handle = (sem_t *)&semaphore->internalHandle.posixHandle;
	int res = pthreadApi->sem_trywait(handle);
	bool result = (res == 0);
	return(result);
}

fpl_platform_api int32_t fplSemaphoreValue(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(0);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	sem_t *handle = (sem_t *)&semaphore->internalHandle.posixHandle;
	int value = 0;
	int res = pthreadApi->sem_getvalue(handle, &value);
	if (res < 0) {
		return 0;
	}
	return((int32_t)value);
}

fpl_platform_api bool fplSemaphoreRelease(fplSemaphoreHandle *semaphore) {
	FPL__CheckArgumentNull(semaphore, false);
	if (!semaphore->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Semaphore '%p' is not valid", semaphore);
		return false;
	}
	FPL__CheckPlatform(0);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__PThreadApi *pthreadApi = &appState->posix.pthreadApi;
	sem_t *handle = (sem_t *)&semaphore->internalHandle.posixHandle;
	int res = pthreadApi->sem_post(handle);
	bool result = (res == 0);
	return(result);
}

//
// POSIX Library
//
fpl_platform_api bool fplDynamicLibraryLoad(const char *libraryFilePath, fplDynamicLibraryHandle *outHandle) {
	bool result = false;
	if (libraryFilePath != fpl_null && outHandle != fpl_null) {
		void *p = dlopen(libraryFilePath, FPL__POSIX_DL_LOADTYPE);
		if (p != fpl_null) {
			fplClearStruct(outHandle);
			outHandle->internalHandle.posixLibraryHandle = p;
			outHandle->isValid = true;
			result = true;
		}
	}
	return(result);
}

fpl_platform_api void *fplGetDynamicLibraryProc(const fplDynamicLibraryHandle *handle, const char *name) {
	void *result = fpl_null;
	if ((handle != fpl_null) && (handle->internalHandle.posixLibraryHandle != fpl_null) && (name != fpl_null)) {
		void *p = handle->internalHandle.posixLibraryHandle;
		result = dlsym(p, name);
	}
	return(result);
}

fpl_platform_api void fplDynamicLibraryUnload(fplDynamicLibraryHandle *handle) {
	if ((handle != fpl_null) && (handle->internalHandle.posixLibraryHandle != fpl_null)) {
		void *p = handle->internalHandle.posixLibraryHandle;
		dlclose(p);
		fplClearStruct(handle);
	}
}

//
// POSIX Memory
//
fpl_platform_api void *fplMemoryAllocate(const size_t size) {
	FPL__CheckArgumentZero(size, fpl_null);
	// @NOTE(final): MAP_ANONYMOUS ensures that the memory is cleared to zero.
	// Allocate empty memory to hold the size + some arbitary padding + the actual data
	size_t newSize = sizeof(size_t) + FPL__MEMORY_PADDING + size;
	void *basePtr = mmap(fpl_null, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	// Write the size at the beginning
	*(size_t *)basePtr = newSize;
	// The resulting address starts after the arbitary padding
	void *result = (uint8_t *)basePtr + sizeof(size_t) + FPL__MEMORY_PADDING;
	return(result);
}

fpl_platform_api void fplMemoryFree(void *ptr) {
	FPL__CheckArgumentNullNoRet(ptr);
	// Free the base pointer which is stored to the left at the start of the size_t
	void *basePtr = (void *)((uint8_t *)ptr - (FPL__MEMORY_PADDING + sizeof(size_t)));
	size_t storedSize = *(size_t *)basePtr;
	munmap(basePtr, storedSize);
}

//
// POSIX Files
//
fpl_platform_api bool fplFileOpenBinary(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			fplClearStruct(outHandle);
			outHandle->isValid = true;
			outHandle->internalHandle.posixFileHandle = posixFileHandle;
			return true;
		}
	}
	return false;
}

fpl_platform_api bool fplFileCreateBinary(const char *filePath, fplFileHandle *outHandle) {
	FPL__CheckArgumentNull(outHandle, false);
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

fpl_platform_api size_t fpl__PosixWriteFileBlock(const fplFileHandle *fileHandle, void *sourceBuffer, const size_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if (!fileHandle->internalHandle.posixFileHandle) {
		FPL__ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
		return 0;
	}
	int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
	ssize_t res;
	do {
		res = write(posixFileHandle, sourceBuffer, sourceSize);
	} while (res == -1 && errno == EINTR);
	size_t result = 0;
	if (res != -1) {
		result = (size_t)res;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileReadBlock32(const fplFileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if (!fileHandle->internalHandle.posixFileHandle) {
		FPL__ERROR(FPL__MODULE_FILES, "File handle is not opened for reading");
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

fpl_platform_api uint64_t fplFileReadBlock64(const fplFileHandle *fileHandle, const uint64_t sizeToRead, void *targetBuffer, const uint64_t maxTargetBufferSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sizeToRead, 0);
	FPL__CheckArgumentNull(targetBuffer, 0);
	if (!fileHandle->internalHandle.posixFileHandle) {
		FPL__ERROR(FPL__MODULE_FILES, "File handle is not opened for reading");
		return 0;
	}
	int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
	uint64_t result = 0;
	uint64_t remainingSize = sizeToRead;
	uint64_t bufferPos = 0;
	const uint64_t MaxValue = (uint64_t)(size_t)-1;
	while (remainingSize > 0) {
		uint8_t *target = (uint8_t *)targetBuffer + bufferPos;
		size_t size = fplMin(remainingSize, MaxValue);
		ssize_t res;
		do {
			res = read(posixFileHandle, target, size);
		} while (res == -1 && errno == EINTR);
		if (res != -1) {
			result += res;
		} else {
			break;
		}
		remainingSize -= res;
		bufferPos += res;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileWriteBlock32(const fplFileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if (!fileHandle->internalHandle.posixFileHandle) {
		FPL__ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
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

fpl_platform_api uint64_t fplFileWriteBlock64(const fplFileHandle *fileHandle, void *sourceBuffer, const uint64_t sourceSize) {
	FPL__CheckArgumentNull(fileHandle, 0);
	FPL__CheckArgumentZero(sourceSize, 0);
	FPL__CheckArgumentNull(sourceBuffer, 0);
	if (!fileHandle->internalHandle.posixFileHandle) {
		FPL__ERROR(FPL__MODULE_FILES, "File handle is not opened for writing");
		return 0;
	}
	int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
	uint64_t result = 0;
	uint64_t remainingSize = sourceSize;
	uint64_t bufferPos = 0;
	const uint64_t MaxValue = (uint64_t)(size_t)-1;
	while (remainingSize > 0) {
		uint8_t *source = (uint8_t *)sourceBuffer + bufferPos;
		size_t size = fplMin(remainingSize, MaxValue);
		ssize_t res;
		do {
			res = write(posixFileHandle, source, size);
		} while (res == -1 && errno == EINTR);
		if (res != -1) {
			result += res;
		}
		remainingSize -= res;
		bufferPos += res;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileSetPosition32(const fplFileHandle *fileHandle, const int32_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if (fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		int whence = SEEK_SET;
		if (mode == fplFilePositionMode_Current) {
			whence = SEEK_CUR;
		} else if (mode == fplFilePositionMode_End) {
			whence = SEEK_END;
		}
		off_t r = lseek(posixFileHandle, position, whence);
		result = (uint32_t)r;
	}
	return(result);
}

fpl_platform_api uint64_t fplFileSetPosition64(const fplFileHandle *fileHandle, const int64_t position, const fplFilePositionMode mode) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if (fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		int whence = SEEK_SET;
		if (mode == fplFilePositionMode_Current) {
			whence = SEEK_CUR;
		} else if (mode == fplFilePositionMode_End) {
			whence = SEEK_END;
		}
		fpl__off64_t r = fpl__lseek64(posixFileHandle, position, whence);
		result = (uint64_t)r;
	}
	return(result);
}

fpl_platform_api uint32_t fplFileGetPosition32(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint32_t result = 0;
	if (fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		off_t res = lseek(posixFileHandle, 0, SEEK_CUR);
		if (res != -1) {
			result = (uint32_t)res;
		}
	}
	return(result);
}

fpl_platform_api uint64_t fplFileGetPosition64(const fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, 0);
	uint64_t result = 0;
	if (fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		fpl__off64_t r = fpl__lseek64(posixFileHandle, 0, SEEK_CUR);
		if (r != -1) {
			result = (uint64_t)r;
		}
	}
	return(result);
}

fpl_platform_api bool fplFileFlush(fplFileHandle *fileHandle) {
	FPL__CheckArgumentNull(fileHandle, false);
	bool result = false;
	if (fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		result = fsync(posixFileHandle) == 0;
	}
	return(result);
}

fpl_platform_api void fplFileClose(fplFileHandle *fileHandle) {
	if ((fileHandle != fpl_null) && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		close(posixFileHandle);
		fplClearStruct(fileHandle);
	}
}

fpl_platform_api uint32_t fplFileGetSizeFromPath32(const char *filePath) {
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

fpl_platform_api uint64_t fplFileGetSizeFromPath64(const char *filePath) {
	uint64_t result = 0;
	if (filePath != fpl_null) {
		int posixFileHandle;
		do {
			posixFileHandle = open(filePath, O_RDONLY);
		} while (posixFileHandle == -1 && errno == EINTR);
		if (posixFileHandle != -1) {
			fpl__off64_t r = fpl__lseek64(posixFileHandle, 0, SEEK_END);
			if (r != -1) {
				result = (uint64_t)r;
			}
			close(posixFileHandle);
		}
	}
	return(result);
}

fpl_platform_api uint32_t fplFileGetSizeFromHandle32(const fplFileHandle *fileHandle) {
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

fpl_platform_api uint64_t fplFileGetSizeFromHandle64(const fplFileHandle *fileHandle) {
	uint64_t result = 0;
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		fpl__off64_t curPos = fpl__lseek64(posixFileHandle, 0, SEEK_CUR);
		if (curPos != -1) {
			result = (uint64_t)fpl__lseek64(posixFileHandle, 0, SEEK_END);
			fpl__lseek64(posixFileHandle, curPos, SEEK_SET);
		}
	}
	return(result);
}

fpl_internal uint64_t fpl__PosixConvertTimeToUnixTimeStamp(const time_t secs) {
	uint64_t result = (uint64_t)secs;
	return(result);
}

fpl_platform_api bool fplFileGetTimestampsFromPath(const char *filePath, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(outStamps, false);
	bool result = false;
	if (filePath != fpl_null) {
		struct stat statBuf;
		if (stat(filePath, &statBuf) != -1) {
			outStamps->creationTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_ctime);
			outStamps->lastAccessTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_atime);
			outStamps->lastModifyTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_mtime);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplFileGetTimestampsFromHandle(const fplFileHandle *fileHandle, fplFileTimeStamps *outStamps) {
	FPL__CheckArgumentNull(fileHandle, false);
	FPL__CheckArgumentNull(outStamps, false);
	bool result = false;
	if (fileHandle != fpl_null && fileHandle->internalHandle.posixFileHandle) {
		int posixFileHandle = fileHandle->internalHandle.posixFileHandle;
		struct stat statBuf;
		if (fstat(posixFileHandle, &statBuf) != -1) {
			outStamps->creationTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_ctime);
			outStamps->lastAccessTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_atime);
			outStamps->lastModifyTime = fpl__PosixConvertTimeToUnixTimeStamp(statBuf.st_mtime);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api bool fplFileSetTimestamps(const char *filePath, const fplFileTimeStamps *timeStamps) {
	// @IMPLEMENT(final/POSIX): fplSetFileTimestamps
	return(false);
}

fpl_platform_api bool fplFileExists(const char *filePath) {
	bool result = false;
	if (filePath != fpl_null) {
		result = access(filePath, F_OK) != -1;
	}
	return(result);
}

fpl_platform_api bool fplFileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
	FPL__CheckArgumentNull(sourceFilePath, false);
	FPL__CheckArgumentNull(targetFilePath, false);
	if (access(sourceFilePath, F_OK) == -1) {
		FPL__ERROR(FPL__MODULE_FILES, "Source file '%s' does not exits", sourceFilePath);
		return false;
	}
	if (!overwrite && access(sourceFilePath, F_OK) != -1) {
		FPL__ERROR(FPL__MODULE_FILES, "Target file '%s' already exits", targetFilePath);
		return false;
	}
	int inputFileHandle;
	do {
		inputFileHandle = open(sourceFilePath, O_RDONLY);
	} while (inputFileHandle == -1 && errno == EINTR);
	if (inputFileHandle == -1) {
		FPL__ERROR(FPL__MODULE_FILES, "Failed open source file '%s', error code: %d", sourceFilePath, inputFileHandle);
		return false;
	}
	int outputFileHandle;
	do {
		outputFileHandle = open(targetFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	} while (outputFileHandle == -1 && errno == EINTR);
	if (outputFileHandle == -1) {
		close(inputFileHandle);
		FPL__ERROR(FPL__MODULE_FILES, "Failed creating target file '%s', error code: %d", targetFilePath, inputFileHandle);
		return false;
	}
	uint8_t buffer[1024 * 10]; // 10 kb buffer
	for (;;) {
		ssize_t readbytes;
		do {
			readbytes = read(inputFileHandle, buffer, fplArrayCount(buffer));
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
	if (path != fpl_null) {
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
	fplAssert((dp != fpl_null) && (entry != fpl_null));
	fplCopyString(dp->d_name, entry->name, fplArrayCount(entry->name));
	entry->type = fplFileEntryType_Unknown;
	entry->attributes = fplFileAttributeFlags_None;
	entry->size = 0;
	entry->permissions.umask = 0;
	char fullPath[FPL_MAX_PATH_LENGTH];
	fplCopyString(entry->internalRoot.rootPath, fullPath, fplArrayCount(fullPath));
	fplEnforcePathSeparatorLen(fullPath, fplArrayCount(fullPath));
	fplStringAppend(dp->d_name, fullPath, fplArrayCount(fullPath));
	struct stat sb;
	if (stat(fullPath, &sb) == 0) {
		if (S_ISDIR(sb.st_mode)) {
			entry->type = fplFileEntryType_Directory;
		} else if (S_ISREG(sb.st_mode)) {
			entry->type = fplFileEntryType_File;
		}
		entry->size = (size_t)sb.st_size;
		if (dp->d_name[0] == '.') {
			// @NOTE(final): Any filename starting with dot is hidden in POSIX
			entry->attributes |= fplFileAttributeFlags_Hidden;
		}
		if (sb.st_mode & S_IRUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanRead;
		}
		if (sb.st_mode & S_IWUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanWrite;
		}
		if (sb.st_mode & S_IXUSR) {
			entry->permissions.user |= fplFilePermissionFlags_CanExecuteSearch;
		}
		if (sb.st_mode & S_IRGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanRead;
		}
		if (sb.st_mode & S_IWGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanWrite;
		}
		if (sb.st_mode & S_IXGRP) {
			entry->permissions.group |= fplFilePermissionFlags_CanExecuteSearch;
		}
		if (sb.st_mode & S_IROTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanRead;
		}
		if (sb.st_mode & S_IWOTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanWrite;
		}
		if (sb.st_mode & S_IXOTH) {
			entry->permissions.owner |= fplFilePermissionFlags_CanExecuteSearch;
		}
	}
}

fpl_platform_api bool fplDirectoryListBegin(const char *path, const char *filter, fplFileEntry *entry) {
	FPL__CheckArgumentNull(path, false);
	FPL__CheckArgumentNull(entry, false);
	DIR *dir = opendir(path);
	if (dir == fpl_null) {
		return false;
	}
	if (fplGetStringLength(filter) == 0) {
		filter = "*";
	}
	fplClearStruct(entry);
	entry->internalHandle.posixDirHandle = dir;
	entry->internalRoot.rootPath = path;
	entry->internalRoot.filter = filter;
	bool result = fplDirectoryListNext(entry);
	return(result);
}

fpl_platform_api bool fplDirectoryListNext(fplFileEntry *entry) {
	FPL__CheckArgumentNull(entry, false);
	bool result = false;
	if (entry->internalHandle.posixDirHandle != fpl_null) {
		DIR *dirHandle = (DIR *)entry->internalHandle.posixDirHandle;
		struct dirent *dp = readdir(dirHandle);
		do {
			if (dp == fpl_null) {
				break;
			}
			bool isMatch = fplIsStringMatchWildcard(dp->d_name, entry->internalRoot.filter);
			if (isMatch) {
				break;
			}
			dp = readdir(dirHandle);
		} while (dp != fpl_null);
		if (dp == fpl_null) {
			closedir(dirHandle);
			fplClearStruct(entry);
		} else {
			fpl__PosixFillFileEntry(dp, entry);
			result = true;
		}
	}
	return(result);
}

fpl_platform_api void fplDirectoryListEnd(fplFileEntry *entry) {
	FPL__CheckArgumentNullNoRet(entry);
	if (entry->internalHandle.posixDirHandle != fpl_null) {
		DIR *dirHandle = (DIR *)entry->internalHandle.posixDirHandle;
		closedir(dirHandle);
		fplClearStruct(entry);
	}
}

//
// POSIX Operating System
//
fpl_platform_api size_t fplSessionGetUsername(char *nameBuffer, const size_t maxNameBufferLen) {
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	size_t result = 0;
	if (pw != fpl_null) {
		size_t nameLen = result = fplGetStringLength(pw->pw_name);
		if (nameLen > 0 && nameBuffer != fpl_null) {
			size_t requiredLen = nameLen + 1;
			FPL__CheckArgumentMin(maxNameBufferLen, requiredLen, 0);
			fplCopyStringLen(pw->pw_name, nameLen, nameBuffer, maxNameBufferLen);
		}
	}
	return(result);
}

fpl_platform_api size_t fplCPUGetCoreCount() {
	size_t result = sysconf(_SC_NPROCESSORS_ONLN);
	return(result);
}

fpl_platform_api size_t fplGetExecutableFilePath(char *destPath, const size_t maxDestLen) {
	const char *procNames[] = {
		"/proc/self/exe",
		"/proc/curproc/exe",
		"/proc/curproc/file",
	};
	char buf[FPL_MAX_PATH_LENGTH];
	size_t result = 0;
	for (int i = 0; i < fplArrayCount(procNames); ++i) {
		const char *procName = procNames[i];
		if (readlink(procName, buf, fplArrayCount(buf) - 1)) {
			int len = fplGetStringLength(buf);
			if (len > 0) {
				char *lastP = buf + (len - 1);
				char *p = lastP;
				while (p != buf) {
					if (*p == '/') {
						len = (lastP - buf) + 1;
						break;
					}
					--p;
				}
				result = len;
				if (destPath != fpl_null) {
					size_t requiredLen = len + 1;
					FPL__CheckArgumentMin(maxDestLen, requiredLen, 0);
					fplCopyStringLen(buf, len, destPath, maxDestLen);
					break;
				}
			}
		}
	}
	return(result);
}

fpl_platform_api size_t fplGetHomePath(char *destPath, const size_t maxDestLen) {
	const char *homeDir = getenv("HOME");
	if (homeDir == fpl_null) {
		int userId = getuid();
		struct passwd *userPwd = getpwuid(userId);
		homeDir = userPwd->pw_dir;
	}
	size_t result = fplGetStringLength(homeDir);
	if (destPath != fpl_null) {
		size_t requiredLen = result + 1;
		FPL__CheckArgumentMin(maxDestLen, requiredLen, 0);
		fplCopyStringLen(homeDir, result, destPath, maxDestLen);
	}
	return(result);
}


fpl_platform_api fplCPUArchType fplCPUGetArchitecture() {
	fplCPUArchType result = fplCPUArchType_Unknown;
	struct utsname nameInfos;
	if (uname(&nameInfos) == 0) {
		const char *machineName = nameInfos.machine;
		if (fplIsStringEqual("x86_64", machineName) || fplIsStringEqual("amd64", machineName)) {
			result = fplCPUArchType_x86_64;
		} else if (fplIsStringEqual("x86", machineName) || fplIsStringEqual("i386", machineName) || fplIsStringEqual("i686", machineName)) {
			result = fplCPUArchType_x86;
		} else if (fplIsStringEqual("ia64", machineName) || fplIsStringEqual("i686-64", machineName)) {
			result = fplCPUArchType_x64;
		} else {
			if (fplIsStringEqualLen("armv", 4, machineName, 4)) {
				const char *m = machineName + 4;
				const char *p = m;
				while (*p >= '0' && *p <= '9') {
					++p;
				}
				size_t l = p - m;
				int32_t version = fplStringToS32Len(m, l);
				if (version == 6) {
					result = fplCPUArchType_Arm32;
				} else if (version >= 7) {
					result = fplCPUArchType_Arm64;
				}
			}
		}
	}
	return(result);
}

fpl_platform_api bool fplOSGetVersionInfos(fplOSVersionInfos *outInfos) {
	bool result = false;
	struct utsname nameInfos;
	if (uname(&nameInfos) == 0) {
		const char *kernelName = nameInfos.sysname;
		const char *kernelVersion = nameInfos.release;
		const char *systemName = nameInfos.version;
		fplCopyString(kernelName, outInfos->osName, fplArrayCount(outInfos->osName));
		fplCopyString(systemName, outInfos->distributionName, fplArrayCount(outInfos->distributionName));
		fpl__ParseVersionString(kernelVersion, &outInfos->osVersion);

		// @TODO(final/POSIX): Get distro version
		// /etc/os-release

		result = true;
	}
	return(result);
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
fpl_platform_api size_t fplWideStringToUTF8String(const wchar_t *wideSource, const size_t wideSourceLen, char *utf8Dest, const size_t maxUtf8DestLen) {
	// @NOTE(final): Expect locale to be UTF-8
	FPL__CheckArgumentNull(wideSource, 0);
	FPL__CheckArgumentZero(wideSourceLen, 0);
	size_t result = wcstombs(fpl_null, wideSource, wideSourceLen);
	if (utf8Dest != fpl_null) {
		size_t requiredLen = result + 1;
		FPL__CheckArgumentMin(maxUtf8DestLen, requiredLen, 0);
		wcstombs(utf8Dest, wideSource, wideSourceLen);
		utf8Dest[result] = 0;
	}
	return(result);
}
fpl_platform_api size_t fplUTF8StringToWideString(const char *utf8Source, const size_t utf8SourceLen, wchar_t *wideDest, const size_t maxWideDestLen) {
	// @NOTE(final): Expect locale to be UTF-8
	FPL__CheckArgumentNull(utf8Source, 0);
	FPL__CheckArgumentZero(utf8SourceLen, 0);
	size_t result = mbstowcs(fpl_null, utf8Source, utf8SourceLen);
	if (wideDest != fpl_null) {
		size_t requiredLen = result + 1;
		FPL__CheckArgumentMin(maxWideDestLen, requiredLen, 0);
		mbstowcs(wideDest, utf8Source, utf8SourceLen);
		wideDest[result] = 0;
	}
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

#define FPL__X11_DEFAULT_WINDOW_WIDTH 400
#define FPL__X11_DEFAULT_WINDOW_HEIGHT 400

fpl_internal void fpl__X11ReleaseSubplatform(fpl__X11SubplatformState *subplatform) {
	fplAssert(subplatform != fpl_null);
	fpl__UnloadX11Api(&subplatform->api);
}

fpl_internal bool fpl__X11InitSubplatform(fpl__X11SubplatformState *subplatform) {
	fplAssert(subplatform != fpl_null);
	if (!fpl__LoadX11Api(&subplatform->api)) {
		FPL__ERROR(FPL__MODULE_X11, "Failed loading x11 api");
		return false;
	}
	return true;
}

fpl_internal void fpl__X11ReleaseWindow(const fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState) {
	fplAssert((subplatform != fpl_null) && (windowState != fpl_null));
	const fpl__X11Api *x11Api = &subplatform->api;
	if (windowState->window) {
		FPL_LOG_DEBUG("X11", "Hide window '%d' from display '%p'", (int)windowState->window, windowState->display);
		x11Api->XUnmapWindow(windowState->display, windowState->window);
		FPL_LOG_DEBUG("X11", "Destroy window '%d' on display '%p'", (int)windowState->window, windowState->display);
		x11Api->XDestroyWindow(windowState->display, windowState->window);
		x11Api->XFlush(windowState->display);
		windowState->window = 0;
	}
	if (windowState->colorMap) {
		FPL_LOG_DEBUG("X11", "Release color map '%d' from display '%p'", (int)windowState->colorMap, windowState->display);
		x11Api->XFreeColormap(windowState->display, windowState->colorMap);
		windowState->colorMap = 0;
	}
	if (windowState->display) {
		FPL_LOG_DEBUG("X11", "Close display '%p'", windowState->display);
		x11Api->XCloseDisplay(windowState->display);
		windowState->display = fpl_null;

#if 0
		FPL_LOG_DEBUG("X11", "Restore previous error handler '%p'", windowState->lastErrorHandler);
		x11Api->XSetErrorHandler(windowState->lastErrorHandler);
#endif

	}
	fplClearStruct(windowState);
}

fpl_internal fplKey fpl__X11TranslateKeySymbol(const KeySym keySym) {
	switch (keySym) {
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

			// @TODO(final/X11): X11 map OEM1-OEM8 key

		default:
			return fplKey_None;
	}
}

fpl_internal void fpl__X11LoadWindowIcon(const fpl__X11Api *x11Api, fpl__X11WindowState *x11WinState, fplWindowSettings *windowSettings) {
	// @BUG(final/X11): Setting the window icon on X11 does not fail, but it does not show up in any of the bars in gnome/ubuntu the icon is always shown as "unset"

	int iconSourceCount = 0;
	fplImageSource iconSources[2] = fplZeroInit;

	if (windowSettings->icons[0].width > 0) {
		iconSources[iconSourceCount++] = windowSettings->icons[0];
	}
	if (windowSettings->icons[1].width > 0) {
		iconSources[iconSourceCount++] = windowSettings->icons[1];
	}

	if (iconSourceCount > 0) {
		int targetSize = 0;
		for (int i = 0; i < iconSourceCount; ++i) {
			targetSize += 2 + iconSources[i].width * iconSources[i].height;
		}

		long *data = (long *)fpl__AllocateTemporaryMemory(sizeof(long) * targetSize, 16);
		long *target = data;

		for (int i = 0; i < iconSourceCount; ++i) {
			const fplImageSource *iconSource = iconSources + i;
			fplAssert(iconSource->type == fplImageType_RGBA);
			*target++ = (int32_t)iconSource->width;
			*target++ = (int32_t)iconSource->height;
			const uint32_t *source = (const uint32_t *)iconSource->data;
			for (int j = 0; j < iconSource->width * iconSource->height; ++j) {
				*target++ = (iconSource->data[j * 4 + 0] << 16) | (iconSource->data[j * 4 + 1] << 8) | (iconSource->data[j * 4 + 2] << 0) | (iconSource->data[j * 4 + 3] << 24);
			}
		}

		x11Api->XChangeProperty(x11WinState->display, x11WinState->window, x11WinState->netWMIcon, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data, targetSize);

		fpl__ReleaseTemporaryMemory(data);
	} else {
		x11Api->XDeleteProperty(x11WinState->display, x11WinState->window, x11WinState->netWMIcon);
	}

	x11Api->XFlush(x11WinState->display);
}

#if 0
fpl_internal int fpl__X11ErrorHandler(Display *display, XErrorEvent *ev) {
	FPL__CheckPlatform(0);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	if (windowState->lastErrorHandler != fpl_null) {
		return windowState->lastErrorHandler(display, ev);
	}

	return(0);
}
#endif

fpl_internal bool fpl__X11InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *appState, fpl__X11SubplatformState *subplatform, fpl__X11WindowState *windowState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	fplAssert((initSettings != fpl_null) && (currentWindowSettings != fpl_null) && (appState != fpl_null) && (subplatform != fpl_null) && (windowState != fpl_null) && (setupCallbacks != fpl_null));
	const fpl__X11Api *x11Api = &subplatform->api;

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Set init threads");
	x11Api->XInitThreads();

#if 0
	FPL_LOG_DEBUG("X11", "Enable error handler");
	windowState->lastErrorHandler = x11Api->XSetErrorHandler(fpl__X11ErrorHandler);
#endif

	const fplWindowSettings *initWindowSettings = &initSettings->window;

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Open default Display");
	windowState->display = x11Api->XOpenDisplay(fpl_null);
	if (windowState->display == fpl_null) {
		FPL__ERROR(FPL__MODULE_X11, "Failed opening default Display!");
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
	if (setupCallbacks->preSetup != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Call Pre-Setup for Window");
		setupCallbacks->preSetup(appState, appState->initFlags, initSettings);
	}

	Visual *visual = windowState->visual;
	int colorDepth = windowState->colorDepth;
	Colormap colormap;
	if (visual != fpl_null && colorDepth > 0) {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Got visual '%p' and color depth '%d' from pre-setup", visual, colorDepth);
		windowState->colorMap = colormap = x11Api->XCreateColormap(windowState->display, windowState->root, visual, AllocNone);
	} else {
		FPL_LOG_DEBUG(FPL__MODULE_X11, "Using default visual, color depth, colormap");
		windowState->visual = visual = x11Api->XDefaultVisual(windowState->display, windowState->screen);
		windowState->colorDepth = colorDepth = x11Api->XDefaultDepth(windowState->display, windowState->screen);
		windowState->colorMap = colormap = x11Api->XDefaultColormap(windowState->display, windowState->screen);
	}

	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using visual: %p", visual);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using color depth: %d", colorDepth);
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Using color map: %d", (int)colormap);

	int flags = CWColormap | CWBorderPixel | CWEventMask | CWBitGravity | CWWinGravity;

	// Window background, either as default or custom color
	unsigned long backgroundPixel;
	if (initWindowSettings->background.value == 0) {
		backgroundPixel = 0;
	} else {
		flags |= CWBackPixel;
		backgroundPixel = (unsigned long)((0xFF << 24) | (initWindowSettings->background.r << 16) | (initWindowSettings->background.g << 8) | initWindowSettings->background.b);
	}

	XSetWindowAttributes swa = fplZeroInit;
	swa.colormap = colormap;
	swa.event_mask =
		StructureNotifyMask |
		ExposureMask | FocusChangeMask | VisibilityChangeMask |
		EnterWindowMask | LeaveWindowMask | PropertyChangeMask |
		KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;
	swa.background_pixel = backgroundPixel;
	swa.border_pixel = 0; // NOTE(final): Use default X11 border
	swa.bit_gravity = NorthWestGravity;
	swa.win_gravity = NorthWestGravity;

	int windowX = 0;
	int windowY = 0;
	int windowWidth;
	int windowHeight;
	if ((initSettings->window.windowSize.width > 0) &&
		(initSettings->window.windowSize.height > 0)) {
		windowWidth = initSettings->window.windowSize.width;
		windowHeight = initSettings->window.windowSize.height;
	} else {
		windowWidth = FPL__X11_DEFAULT_WINDOW_WIDTH;
		windowHeight = FPL__X11_DEFAULT_WINDOW_HEIGHT;
	}

	windowState->lastWindowStateInfo.state = fplWindowState_Normal;
	windowState->lastWindowStateInfo.visibility = fplWindowVisibilityState_Show;
	windowState->lastWindowStateInfo.position = fplStructInit(fplWindowPosition, windowWidth, windowHeight);
	windowState->lastWindowStateInfo.size = fplStructInit(fplWindowSize, windowX, windowY);

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
	if (!windowState->window) {
		FPL__ERROR(FPL__MODULE_X11, "Failed creating window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d'!", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap);
		fpl__X11ReleaseWindow(subplatform, windowState);
		return false;
	}
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Successfully created window with (Display='%p', Root='%d', Size=%dx%d, Colordepth='%d', visual='%p', colormap='%d': %d", windowState->display, (int)windowState->root, windowWidth, windowHeight, colorDepth, visual, (int)swa.colormap, (int)windowState->window);

	// Type atoms
	windowState->utf8String = x11Api->XInternAtom(windowState->display, "UTF8_STRING", False);

	// Window manager atoms
	windowState->wmDeleteWindow = x11Api->XInternAtom(windowState->display, "WM_DELETE_WINDOW", False);
	windowState->wmProtocols = x11Api->XInternAtom(windowState->display, "WM_PROTOCOLS", False);
	windowState->wmState = x11Api->XInternAtom(windowState->display, "WM_STATE", False);
	windowState->netWMPing = x11Api->XInternAtom(windowState->display, "_NET_WM_PING", False);
	windowState->netWMState = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE", False);
	windowState->netWMStateFocused = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE_FOCUSED", False);
	windowState->netWMStateFullscreen = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE_FULLSCREEN", False);
	windowState->netWMStateHidden = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE_HIDDEN", False);
	windowState->netWMStateMaximizedVert = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	windowState->netWMStateMaximizedHorz = x11Api->XInternAtom(windowState->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	windowState->netWMPid = x11Api->XInternAtom(windowState->display, "_NET_WM_PID", False);
	windowState->netWMIcon = x11Api->XInternAtom(windowState->display, "_NET_WM_ICON", False);
	windowState->netWMName = x11Api->XInternAtom(windowState->display, "_NET_WM_NAME", False);
	windowState->netWMIconName = x11Api->XInternAtom(windowState->display, "_NET_WM_ICON_NAME", False);
	windowState->motifWMHints = x11Api->XInternAtom(windowState->display, "_MOTIF_WM_HINTS", False);
	// xdnd atoms
	windowState->xdndAware = x11Api->XInternAtom(windowState->display, "XdndAware", False);
	windowState->xdndEnter = x11Api->XInternAtom(windowState->display, "XdndEnter", False);
	windowState->xdndPosition = x11Api->XInternAtom(windowState->display, "XdndPosition", False);
	windowState->xdndStatus = x11Api->XInternAtom(windowState->display, "XdndStatus", False);
	windowState->xdndActionCopy = x11Api->XInternAtom(windowState->display, "XdndActionCopy", False);
	windowState->xdndDrop = x11Api->XInternAtom(windowState->display, "XdndDrop", False);
	windowState->xdndFinished = x11Api->XInternAtom(windowState->display, "XdndFinished", False);
	windowState->xdndSelection = x11Api->XInternAtom(windowState->display, "XdndSelection", False);
	windowState->xdndTypeList = x11Api->XInternAtom(windowState->display, "XdndTypeList", False);
	windowState->textUriList = x11Api->XInternAtom(windowState->display, "text/uri-list", False);

	// Register window manager protocols
	{
		Atom protocols[] = {
				windowState->wmDeleteWindow,
				windowState->netWMPing
		};
		x11Api->XSetWMProtocols(windowState->display, windowState->window, protocols, fplArrayCount(protocols));
	}

	// Declare our process id
	{
		const long pid = getpid();
		x11Api->XChangeProperty(windowState->display, windowState->window, windowState->netWMPid, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&pid, 1);
	}

	char nameBuffer[FPL_MAX_NAME_LENGTH] = fplZeroInit;
	if (fplGetStringLength(initSettings->window.title) > 0) {
		fplCopyString(initSettings->window.title, nameBuffer, fplArrayCount(nameBuffer));
	} else {
		fplCopyString("Unnamed FPL X11 Window", nameBuffer, fplArrayCount(nameBuffer));
	}
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Show window '%d' on display '%p' with title '%s'", (int)windowState->window, windowState->display, nameBuffer);
	fpl__X11LoadWindowIcon(x11Api, windowState, currentWindowSettings);
	fplSetWindowTitle(nameBuffer);
	x11Api->XMapWindow(windowState->display, windowState->window);
	x11Api->XFlush(windowState->display);

	fplAssert(fplArrayCount(appState->window.keyMap) >= 256);

	// @NOTE(final): Valid key range for XLib is 8 to 255
	FPL_LOG_DEBUG(FPL__MODULE_X11, "Build X11 Keymap");
	fplClearStruct(appState->window.keyMap);
	for (int keyCode = 8; keyCode <= 255; ++keyCode) {
		int dummy = 0;
		KeySym *keySyms = x11Api->XGetKeyboardMapping(windowState->display, keyCode, 1, &dummy);
		KeySym keySym = keySyms[0];
		fplKey mappedKey = fpl__X11TranslateKeySymbol(keySym);
		appState->window.keyMap[keyCode] = mappedKey;
		x11Api->XFree(keySyms);
	}

	if (initSettings->window.isFullscreen) {
		fplSetWindowFullscreenSize(true, initSettings->window.fullscreenSize.width, initSettings->window.fullscreenSize.height, initSettings->window.fullscreenRefreshRate);
	}

	// Announce support for Xdnd (drag and drop)
	{
		const Atom version = FPL__XDND_VERSION;
		x11Api->XChangeProperty(windowState->display, windowState->window, windowState->xdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char *)&version, 1);
	}

	appState->window.isRunning = true;

	return true;
}

fpl_internal fplKeyboardModifierFlags fpl__X11TranslateModifierFlags(const int state) {
	fplKeyboardModifierFlags result = fplKeyboardModifierFlags_None;
	if (state & ShiftMask) {
		result |= fplKeyboardModifierFlags_LShift;
		result |= fplKeyboardModifierFlags_RShift;
	}
	if (state & ControlMask) {
		result |= fplKeyboardModifierFlags_LCtrl;
		result |= fplKeyboardModifierFlags_RCtrl;
	}
	if (state & Mod1Mask) {
		result |= fplKeyboardModifierFlags_LAlt;
		result |= fplKeyboardModifierFlags_RAlt;
	}
	if (state & Mod4Mask) {
		result |= fplKeyboardModifierFlags_LSuper;
		result |= fplKeyboardModifierFlags_RSuper;
	}
	return(result);
}

fpl_internal unsigned long fpl__X11GetWindowProperty(const fpl__X11Api *x11Api, Display *display, Window window, Atom prop, Atom type, unsigned char **value) {
	Atom actualType;
	int actualFormat;
	unsigned long itemCount, bytesAfter;
	x11Api->XGetWindowProperty(display, window, prop, 0, LONG_MAX, False, type, &actualType, &actualFormat, &itemCount, &bytesAfter, value);
	return(itemCount);
}

fpl_internal const int fpl__X11GetWMState(const fpl__X11Api *x11Api, fpl__X11WindowState *windowState) {
	struct { int state; Window icon; } *value = NULL;
	unsigned long numItems = fpl__X11GetWindowProperty(x11Api, windowState->display, windowState->window, windowState->wmState, windowState->wmState, (unsigned char **)&value);
	int state = WithdrawnState;
	if (value) {
		state = value->state;
		x11Api->XFree(value);
	}
	return state;
}

#define fpl__X11NetWMStateHiddenFlag (1 << 0)
#define fpl__X11NetWMStateMaximizedFlag (1 << 1)
#define fpl__X11NetWMStateFullscreenFlag (1 << 2)

fpl_internal unsigned int fpl__X11GetNetWMState(const fpl__X11Api *x11Api, fpl__X11WindowState *windowState) {
	Atom *atoms = NULL;
	unsigned long numItems = fpl__X11GetWindowProperty(x11Api, windowState->display, windowState->window, windowState->netWMState, XA_ATOM, (unsigned char **)&atoms);
	unsigned int flags = 0;
	if (atoms) {
		int i, maximized = 0;
		for (i = 0; i < numItems; ++i) {
			if (atoms[i] == windowState->netWMStateHidden) {
				flags |= fpl__X11NetWMStateHiddenFlag;
			} else if (atoms[i] == windowState->netWMStateMaximizedVert) {
				maximized |= 1;
			} else if (atoms[i] == windowState->netWMStateMaximizedHorz) {
				maximized |= 2;
			} else if (atoms[i] == windowState->netWMStateFullscreen) {
				flags |= fpl__X11NetWMStateFullscreenFlag;
			}
		}
		if (maximized == 3) {
			flags |= fpl__X11NetWMStateMaximizedFlag;
		}
		// additional visibility check for unmapped window
		{
			XWindowAttributes attr;
			fplMemorySet(&attr, 0, sizeof(attr));
			x11Api->XGetWindowAttributes(windowState->display, windowState->window, &attr);
			if (attr.map_state == IsUnmapped) {
				flags |= fpl__X11NetWMStateHiddenFlag;
			}
		}
		x11Api->XFree(atoms);
	}
	return flags;
}

fpl_internal fpl__X11WindowStateInfo fpl__X11GetWindowStateInfo(const fpl__X11Api *x11Api, fpl__X11WindowState *windowState) {
	fpl__X11WindowStateInfo result = fplZeroInit;
	const int state = fpl__X11GetWMState(x11Api, windowState);
	unsigned int flags = fpl__X11GetNetWMState(x11Api, windowState);
	if (state == NormalState) {
		result.state = fplWindowState_Normal;
	} else if (state == IconicState) {
		result.state = fplWindowState_Iconify;
	}
	// reset visibility to default
	result.visibility = fplWindowVisibilityState_Show;
	if (flags & fpl__X11NetWMStateHiddenFlag) {
		result.visibility = fplWindowVisibilityState_Hide;
	}
	if (flags & fpl__X11NetWMStateFullscreenFlag) {
		result.state = fplWindowState_Fullscreen;
	} else if (state != IconicState && flags & fpl__X11NetWMStateMaximizedFlag) {
		result.state = fplWindowState_Maximize;
	}
	return result;
}

fpl_internal fpl__X11WindowStateInfo fpl__X11ReconcilWindowStateInfo(fpl__X11WindowStateInfo *last, fpl__X11WindowStateInfo *next) {
	fpl__X11WindowStateInfo change = fplZeroInit;
	if (last->state != next->state) {
		change.state = next->state;
	}
	if (last->visibility != next->visibility) {
		change.visibility = next->visibility;
	}
	return change;
}

fpl_internal void *fpl__X11ParseUriPaths(const char *text, size_t *size, int *count, int textLength) {
	const char *textCursor = text;
	const char *textEnd = text + textLength;
	int fileCount = 0;
	// count file entries
	while (*textCursor != '\0' || textCursor != textEnd) {
		if (*textCursor == '\r' && *(textCursor + 1) == '\n')
			++fileCount;
		++textCursor;
	}
	textCursor = text;
	size_t filesTableSize = fileCount * sizeof(char **);
	size_t maxFileStride = FPL_MAX_PATH_LENGTH + 1;
	size_t filesMemorySize = filesTableSize + FPL__ARBITARY_PADDING + maxFileStride * fileCount;
	void *filesTableMemory = fpl__AllocateDynamicMemory(filesMemorySize, 8);
	char **filesTable = (char **)filesTableMemory;
	for (int fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		filesTable[fileIndex] = (char *)((uint8_t *)filesTableMemory + filesTableSize + FPL__ARBITARY_PADDING + fileIndex * maxFileStride);
	}
	for (int fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
		char *file = filesTable[fileIndex];
		const char *line = textCursor;
		// split on '\r\n' divider 
		while (*textCursor != '\r' && (*textCursor != '\0' || textCursor != textEnd)) {
			++textCursor;
		}
		// strip protocol
		if (fplIsStringEqualLen(line, 7, "file://", 7)) {
			line += 7;
		}
		fplCopyStringLen(line, (textCursor - line), file, maxFileStride);
		textCursor += 2;
	}
	*size = filesMemorySize;
	*count = fileCount;
	return filesTableMemory;
}

fpl_internal void fpl__X11HandleTextInputEvent(const fpl__X11Api *x11Api, fpl__PlatformWindowState *winState, const uint64_t keyCode, XEvent *ev) {
	char buf[32];
	KeySym keysym = 0;
	if (x11Api->XLookupString(&ev->xkey, buf, 32, &keysym, NULL) != NoSymbol) {
		wchar_t wideBuffer[4] = fplZeroInit;
		fplUTF8StringToWideString(buf, fplGetStringLength(buf), wideBuffer, fplArrayCount(wideBuffer));
		uint32_t textCode = (uint32_t)wideBuffer[0];
		if (textCode > 0) {
			fpl__HandleKeyboardInputEvent(winState, keyCode, textCode);
		}
	}
}

fpl_internal void fpl__X11HandleEvent(const fpl__X11SubplatformState *subplatform, fpl__PlatformAppState *appState, XEvent *ev) {
	fplAssert((subplatform != fpl_null) && (appState != fpl_null) && (ev != fpl_null));
	fpl__PlatformWindowState *winState = &appState->window;
	fpl__X11WindowState *x11WinState = &winState->x11;
	fpl__X11WindowStateInfo *lastX11WinInfo = &x11WinState->lastWindowStateInfo;
	const fpl__X11Api *x11Api = &appState->x11.api;

	if (appState->currentSettings.window.callbacks.eventCallback != fpl_null) {
		appState->currentSettings.window.callbacks.eventCallback(fplGetPlatformType(), x11WinState, ev, appState->currentSettings.window.callbacks.eventUserData);
	}

	switch (ev->type) {
		case ConfigureNotify:
		{
#		if defined(FPL__ENABLE_VIDEO_SOFTWARE)
			if (appState->currentSettings.video.backend == fplVideoBackendType_Software) {
				if (appState->initSettings.video.isAutoSize) {
					uint32_t w = (uint32_t)ev->xconfigure.width;
					uint32_t h = (uint32_t)ev->xconfigure.height;
					fplResizeVideoBackBuffer(w, h);
				}
			}
#		endif
			// Window resized
			if (ev->xconfigure.width != lastX11WinInfo->size.width || ev->xconfigure.height != lastX11WinInfo->size.height) {
				fpl__PushWindowSizeEvent(fplWindowEventType_Resized, (uint32_t)ev->xconfigure.width, (uint32_t)ev->xconfigure.height);
				lastX11WinInfo->size.width = (int32_t)ev->xconfigure.width;
				lastX11WinInfo->size.height = (int32_t)ev->xconfigure.height;
			}

			// Window moved
			if (ev->xconfigure.x != lastX11WinInfo->position.left || ev->xconfigure.y != lastX11WinInfo->position.top) {
				fpl__PushWindowPositionEvent(fplWindowEventType_PositionChanged, (int32_t)ev->xconfigure.x, (int32_t)ev->xconfigure.y);
				lastX11WinInfo->position.left = (int32_t)ev->xconfigure.x;
				lastX11WinInfo->position.top = (int32_t)ev->xconfigure.y;
			}
		} break;

		case ClientMessage:
		{
			if (ev->xclient.message_type == x11WinState->wmProtocols) {
				const Atom protocol = (Atom)ev->xclient.data.l[0];
				if (protocol != None) {
					if (protocol == x11WinState->wmDeleteWindow) {
						// Window asked for closing
						winState->isRunning = false;
						fpl__PushWindowStateEvent(fplWindowEventType_Closed);
					} else if (protocol == x11WinState->netWMPing) {
						// Window manager asks us if we are still alive
						XEvent reply = *ev;
						reply.xclient.window = x11WinState->root;
						x11Api->XSendEvent(x11WinState->display, x11WinState->root, False, SubstructureNotifyMask | SubstructureRedirectMask, &reply);
					}
				}
			} else if (ev->xclient.message_type == x11WinState->xdndEnter) {
				// A drag operation has entered the window
				unsigned long i, count;
				Atom *formats = NULL;
				bool list = ev->xclient.data.l[1] & 1;
				x11WinState->xdnd.source = ev->xclient.data.l[0];
				x11WinState->xdnd.version = ev->xclient.data.l[1] >> 24;
				x11WinState->xdnd.format = None;
				if (x11WinState->xdnd.version > FPL__XDND_VERSION) {
					return;
				}
				if (list) {
					count = fpl__X11GetWindowProperty(x11Api, x11WinState->display, x11WinState->xdnd.source, x11WinState->xdndTypeList, XA_ATOM, (unsigned char **)&formats);
				} else {
					count = 3;
					formats = (Atom *)ev->xclient.data.l + 2;
				}
				for (i = 0; i < count; ++i) {
					if (formats[i] == x11WinState->textUriList) {
						x11WinState->xdnd.format = x11WinState->textUriList;
						break;
					}
				}
				if (list && formats) {
					x11Api->XFree(formats);
				}
			} else if (ev->xclient.message_type == x11WinState->xdndDrop) {
				// The drag operation has finished by dropping on the window
				Time time = CurrentTime;
				if (x11WinState->xdnd.version > FPL__XDND_VERSION) {
					return;
				}
				if (x11WinState->xdnd.format) {
					if (x11WinState->xdnd.version >= 1) {
						time = ev->xclient.data.l[2];
					}
					// Request the chosen format from the source window
					x11Api->XConvertSelection(x11WinState->display, x11WinState->xdndSelection, x11WinState->xdnd.format, x11WinState->xdndSelection, x11WinState->window, time);
				} else if (x11WinState->xdnd.version >= 2) {
					XEvent reply;
					fplMemorySet(&reply, 0, sizeof(reply));

					reply.type = ClientMessage;
					reply.xclient.window = x11WinState->xdnd.source;
					reply.xclient.message_type = x11WinState->xdndFinished;
					reply.xclient.format = 32;
					reply.xclient.data.l[0] = x11WinState->window;
					reply.xclient.data.l[1] = 0; // The drag was rejected
					reply.xclient.data.l[2] = None;

					x11Api->XSendEvent(x11WinState->display, x11WinState->xdnd.source, False, NoEventMask, &reply);
					x11Api->XFlush(x11WinState->display);
				}
			} else if (ev->xclient.message_type == x11WinState->xdndPosition) {
				// The drag operation has moved over the window
				Window dummy;
				int xpos, ypos;
				const int xabs = (ev->xclient.data.l[2] >> 16) & 0xffff;
				const int yabs = (ev->xclient.data.l[2]) & 0xffff;
				if (x11WinState->xdnd.version > FPL__XDND_VERSION) {
					return;
				}
				XEvent reply;
				fplMemorySet(&reply, 0, sizeof(reply));

				reply.type = ClientMessage;
				reply.xclient.window = x11WinState->xdnd.source;
				reply.xclient.message_type = x11WinState->xdndStatus;
				reply.xclient.format = 32;
				reply.xclient.data.l[0] = x11WinState->window;
				reply.xclient.data.l[2] = 0; // Specify an empty rectangle
				reply.xclient.data.l[3] = 0;

				if (x11WinState->xdnd.format) {
					// Reply that we are ready to copy the dragged data
					reply.xclient.data.l[1] = 1; // Accept with no rectangle
					if (x11WinState->xdnd.version >= 2)
						reply.xclient.data.l[4] = x11WinState->xdndActionCopy;
				}
				x11Api->XSendEvent(x11WinState->display, x11WinState->xdnd.source, False, NoEventMask, &reply);
				x11Api->XFlush(x11WinState->display);
			}
		} break;

		case SelectionNotify:
		{
			if (ev->xselection.property == x11WinState->xdndSelection) {
				// The converted data from the drag operation has arrived
				char *data;
				const unsigned long result = fpl__X11GetWindowProperty(x11Api, x11WinState->display, ev->xselection.requestor, ev->xselection.property, ev->xselection.target, (unsigned char **)&data);
				if (result) {
					size_t filesTableSize;
					int fileCount;
					void *filesTable = fpl__X11ParseUriPaths(data, &filesTableSize, &fileCount, result);
					fplMemoryBlock memory = fplZeroInit;
					memory.size = filesTableSize;
					memory.base = filesTable;
					fpl__PushWindowDropFilesEvent(NULL, fileCount, (const char **)filesTable, &memory);
				}
				if (data) {
					x11Api->XFree(data);
				}
				if (x11WinState->xdnd.version >= 2) {
					XEvent reply;
					fplMemorySet(&reply, 0, sizeof(reply));

					reply.type = ClientMessage;
					reply.xclient.window = x11WinState->xdnd.source;
					reply.xclient.message_type = x11WinState->xdndFinished;
					reply.xclient.format = 32;
					reply.xclient.data.l[0] = x11WinState->window;
					reply.xclient.data.l[1] = result;
					reply.xclient.data.l[2] = x11WinState->xdndActionCopy;

					x11Api->XSendEvent(x11WinState->display, x11WinState->xdnd.source, False, NoEventMask, &reply);
					x11Api->XFlush(x11WinState->display);
				}
			}
		} break;

		case KeyPress:
		{
			// Keyboard button down
			if (!appState->currentSettings.input.disabledEvents) {
				int keyState = ev->xkey.state;
				uint64_t keyCode = (uint64_t)ev->xkey.keycode;
				Time keyTime = ev->xkey.time;
				Time lastPressTime = winState->keyPressTimes[keyCode];
				Time diffTime = keyTime - lastPressTime;
				FPL_LOG_INFO("X11", "Diff for key '%llu', time: %lu, diff: %lu, last: %lu", keyCode, keyTime, diffTime, lastPressTime);
				if (diffTime == keyTime || (diffTime > 0 && diffTime < (1 << 31))) {
					if (keyCode) {
						fpl__HandleKeyboardButtonEvent(winState, (uint64_t)keyTime, keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Press, false);
						fpl__X11HandleTextInputEvent(x11Api, winState, keyCode, ev);
					}
					winState->keyPressTimes[keyCode] = keyTime;
				}
			}
		} break;

		case KeyRelease:
		{
			// Keyboard button up
			if (!appState->currentSettings.input.disabledEvents) {
				bool isRepeat = false;
				if (x11Api->XEventsQueued(x11WinState->display, QueuedAfterReading)) {
					XEvent nextEvent;
					x11Api->XPeekEvent(x11WinState->display, &nextEvent);
					if ((nextEvent.type == KeyPress) && (nextEvent.xkey.time == ev->xkey.time) && (nextEvent.xkey.keycode == ev->xkey.keycode)) {
						// Delete the peeked event, which is a key-repeat
						x11Api->XNextEvent(x11WinState->display, ev);
						isRepeat = true;
					}
				}

				int keyState = ev->xkey.state;
				uint64_t keyCode = (uint64_t)ev->xkey.keycode;

				if (isRepeat) {
					fpl__X11HandleTextInputEvent(x11Api, winState, keyCode, ev);
					fpl__HandleKeyboardButtonEvent(winState, (uint64_t)ev->xkey.time, (uint64_t)keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Repeat, false);
				} else {
					fpl__HandleKeyboardButtonEvent(winState, (uint64_t)ev->xkey.time, (uint64_t)keyCode, fpl__X11TranslateModifierFlags(keyState), fplButtonState_Release, true);
				}
			}
		} break;

		case ButtonPress:
		{
			int x = ev->xbutton.x;
			int y = ev->xbutton.y;

			if (!appState->currentSettings.input.disabledEvents) {
				// Mouse button
				if (ev->xbutton.button == Button1) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Left, fplButtonState_Press);
				} else if (ev->xbutton.button == Button2) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Middle, fplButtonState_Press);
				} else if (ev->xbutton.button == Button3) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Right, fplButtonState_Press);
				}
			}

			// Mouse wheel
			if (ev->xbutton.button == Button4) {
				fpl__HandleMouseWheelEvent(winState, x, y, 1.0f);
			} else if (ev->xbutton.button == Button5) {
				fpl__HandleMouseWheelEvent(winState, x, y, -1.0f);
			}
		} break;

		case ButtonRelease:
		{
			// Mouse up
			if (!appState->currentSettings.input.disabledEvents) {
				int x = ev->xbutton.x;
				int y = ev->xbutton.y;
				if (ev->xbutton.button == Button1) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Left, fplButtonState_Release);
				} else if (ev->xbutton.button == Button2) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Middle, fplButtonState_Release);
				} else if (ev->xbutton.button == Button3) {
					fpl__HandleMouseButtonEvent(winState, x, y, fplMouseButtonType_Right, fplButtonState_Release);
				}
			}
		} break;

		case MotionNotify:
		{
			// Mouse move
			if (!appState->currentSettings.input.disabledEvents) {
				fpl__HandleMouseMoveEvent(winState, ev->xmotion.x, ev->xmotion.y);
			}
		} break;

		case Expose:
		{
			// Repaint
			if (appState->currentSettings.window.callbacks.exposedCallback != fpl_null) {
				appState->currentSettings.window.callbacks.exposedCallback(fplGetPlatformType(), x11WinState, ev, appState->currentSettings.window.callbacks.exposedUserData);
			}
		} break;

		case FocusIn:
		{
			// Ignore focus events from popup indicator windows, window menu
			// key chords and window dragging
			if (ev->xfocus.mode == NotifyGrab || ev->xfocus.mode == NotifyUngrab) {
				return;
			}
			fpl__PushWindowStateEvent(fplWindowEventType_GotFocus);
		} break;

		case FocusOut:
		{
			// Ignore focus events from popup indicator windows, window menu
			// key chords and window dragging
			if (ev->xfocus.mode == NotifyGrab || ev->xfocus.mode == NotifyUngrab) {
				return;
			}
			fpl__PushWindowStateEvent(fplWindowEventType_LostFocus);
		} break;

		case PropertyNotify:
		{
			if (ev->xproperty.atom == x11WinState->netWMState || ev->xproperty.atom == x11WinState->wmState) {
				fpl__X11WindowStateInfo nextWindowStateInfo = fpl__X11GetWindowStateInfo(x11Api, x11WinState);
				fpl__X11WindowStateInfo changedWindowStateInfo = fpl__X11ReconcilWindowStateInfo(&x11WinState->lastWindowStateInfo, &nextWindowStateInfo);
				switch (changedWindowStateInfo.visibility) {
					case fplWindowVisibilityState_Hide:
						fpl__PushWindowStateEvent(fplWindowEventType_Hidden);
						break;
					case fplWindowVisibilityState_Show:
						fpl__PushWindowStateEvent(fplWindowEventType_Shown);
						break;
					default:
						break;
				}
				switch (changedWindowStateInfo.state) {
					case fplWindowState_Iconify:
						fpl__PushWindowStateEvent(fplWindowEventType_Minimized);
						break;
					case fplWindowState_Maximize:
						fpl__PushWindowStateEvent(fplWindowEventType_Maximized);
						break;
					case fplWindowState_Normal:
						fpl__PushWindowStateEvent(fplWindowEventType_Restored);
						break;
					default:
						break;
				}
				x11WinState->lastWindowStateInfo.state = nextWindowStateInfo.state;
				x11WinState->lastWindowStateInfo.visibility = nextWindowStateInfo.visibility;
			}
		} break;

		default:
			break;
	}
}

fpl_platform_api bool fplIsWindowRunning() {
	FPL__CheckPlatform(false);
	bool result = fpl__global__AppState->window.isRunning;
	return(result);
}

fpl_platform_api void fplWindowShutdown() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	if (appState->window.isRunning) {
		appState->window.isRunning = false;
		const fpl__X11SubplatformState *subplatform = &appState->x11;
		const fpl__X11Api *x11Api = &subplatform->api;
		const fpl__X11WindowState *windowState = &appState->window.x11;
		XEvent ev = fplZeroInit;
		ev.type = ClientMessage;
		ev.xclient.window = windowState->window;
		ev.xclient.message_type = windowState->wmProtocols;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = windowState->wmDeleteWindow;
		ev.xclient.data.l[1] = 0;
		x11Api->XSendEvent(windowState->display, windowState->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
	}
}

fpl_internal bool fpl__X11ProcessNextEvent(const fpl__X11SubplatformState *subplatform, fpl__PlatformAppState *appState) {
	bool result = false;
	const fpl__X11Api *x11Api = &subplatform->api;
	fpl__X11WindowState *windowState = &appState->window.x11;
	if (x11Api->XPending(windowState->display)) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		fpl__X11HandleEvent(subplatform, appState, &ev);
		result = true;
	}
	return(result);
}

fpl_platform_api bool fplPollEvent(fplEvent *ev) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	// Poll next event from the internal queue first
	if (fpl__PollInternalEvent(ev)) {
		return(true);
	}

	// Create new event(s) from the X11 event queue
	if (!fpl__X11ProcessNextEvent(subplatform, appState)) {
		return(false);
	}

	// Poll the first event from the internal queue
	if (fpl__PollInternalEvent(ev)) {
		return(true);
	}

	// No events left
	return(false);
}

fpl_platform_api void fplPollEvents() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	while (x11Api->XPending(windowState->display)) {
		XEvent ev;
		x11Api->XNextEvent(windowState->display, &ev);
		fpl__X11HandleEvent(subplatform, appState, &ev);
	}
	fpl__ClearInternalEvents();
}

fpl_platform_api bool fplWindowUpdate() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	fpl__ClearInternalEvents();

	// Dont like this, maybe a callback would be better?
#if defined(FPL_PLATFORM_LINUX)
	if ((!appState->currentSettings.input.disabledEvents) && (appState->initFlags & fplInitFlags_GameController)) {
		fpl__LinuxAppState *linuxAppState = &appState->plinux;
		fpl__LinuxPollGameControllers(&appState->currentSettings, &linuxAppState->controllersState, true);
	}
#endif

	bool result = appState->window.isRunning;
	return(result);
}

fpl_platform_api void fplSetWindowCursorEnabled(const bool value) {
	// @IMPLEMENT(final/X11): fplSetWindowCursorEnabled
}

fpl_platform_api bool fplGetWindowSize(fplWindowSize *outSize) {
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

fpl_platform_api void fplSetWindowSize(const uint32_t width, const uint32_t height) {
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL__CheckPlatformNoRet();
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	x11Api->XResizeWindow(windowState->display, windowState->window, width, height);
	x11Api->XFlush(windowState->display);
}

fpl_platform_api bool fplIsWindowResizable() {
	// @IMPLEMENT(final/X11): fplIsWindowResizable
	return false;
}

fpl_platform_api void fplSetWindowResizeable(const bool value) {
	// @IMPLEMENT(final/X11): fplSetWindowResizeable
}

fpl_platform_api bool fplIsWindowDecorated() {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	bool result = appState->currentSettings.window.isDecorated;
	return(result);
}

#define FPL__MWM_HINTS_DECORATIONS (1L << 1)
#define FPL__MWM_HINTS_FUNCTIONS (1L << 0)
#define FPL__MWM_FUNC_ALL (1L<<0)
#define FPL__PROPERTY_MOTIF_WM_HINTS_ELEMENT_COUNT 5

typedef struct {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long input_mode;
	unsigned long status;
} fpl__MotifWMHints;

fpl_platform_api void fplSetWindowDecorated(const bool value) {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	fpl__MotifWMHints hints = fplZeroInit;
	hints.flags = FPL__MWM_HINTS_DECORATIONS | FPL__MWM_HINTS_FUNCTIONS;
	hints.decorations = value ? 1 : 0;
	hints.functions = value ? FPL__MWM_FUNC_ALL : 0;

	x11Api->XChangeProperty(windowState->display, windowState->window,
		windowState->motifWMHints,
		windowState->motifWMHints, 32,
		PropModeReplace,
		(unsigned char *)&hints,
		FPL__PROPERTY_MOTIF_WM_HINTS_ELEMENT_COUNT);

	appState->currentSettings.window.isDecorated = value;
}

fpl_platform_api bool fplIsWindowFloating() {
	// @IMPLEMENT(final/X11): fplIsWindowFloating
	return false;
}

fpl_platform_api void fplSetWindowFloating(const bool value) {
	// @IMPLEMENT(final/X11): fplSetWindowFloating
}

fpl_platform_api fplWindowState fplGetWindowState() {
	// @IMPLEMENT(final/X11): fplGetWindowState
	return(fplWindowState_Unknown);
}

fpl_platform_api bool fplSetWindowState(const fplWindowState newState) {
	// @IMPLEMENT(final/X11): fplSetWindowState
	return(false);
}

fpl_platform_api size_t fplGetDisplayCount() {
	// @IMPLEMENT(final/X11): fplGetDisplayCount
	return(0);
}

fpl_platform_api size_t fplGetDisplays(fplDisplayInfo *displays, const size_t maxDisplayCount) {
	// @IMPLEMENT(final/X11): fplGetDisplays
	return(0);
}

fpl_platform_api bool fplGetPrimaryDisplay(fplDisplayInfo *display) {
	// @IMPLEMENT(final/X11): fplGetPrimaryDisplay
	return(false);
}

fpl_platform_api bool fplGetWindowDisplay(fplDisplayInfo *outDisplay) {
	// @IMPLEMENT(final/X11): fplGetWindowDisplay
	return(false);
}

fpl_platform_api bool fplGetDisplayFromPosition(const int32_t x, const int32_t y, fplDisplayInfo *outDisplay) {
	// @IMPLEMENT(final/X11): fplGetDisplayFromPosition
	return(false);
}

fpl_platform_api size_t fplGetDisplayModes(const char *id, fplDisplayMode *modes, const size_t maxDisplayModeCount) {
	// @IMPLEMENT(final/X11): fplGetDisplayModes
	return(0);
}

fpl_platform_api bool fplSetWindowFullscreenSize(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	// https://stackoverflow.com/questions/10897503/opening-a-fullscreen-opengl-window
	XEvent xev = fplZeroInit;
	xev.type = ClientMessage;
	xev.xclient.window = windowState->window;
	xev.xclient.message_type = windowState->netWMState;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = value ? 1 : 0; // 1 = Add, 0 = Remove
	xev.xclient.data.l[1] = windowState->netWMStateFullscreen; // _NET_WM_STATE_FULLSCREEN
	xev.xclient.data.l[3] = 1l; // Application source

	// @TODO(final/X11): Support for changing the display resolution + refresh rate in X11

	bool result = x11Api->XSendEvent(windowState->display, windowState->root, 0, SubstructureRedirectMask | SubstructureNotifyMask, &xev) != 0;
	if (result) {
		appState->currentSettings.window.isFullscreen = value;
	}
	return(result);
}

fpl_platform_api bool fplSetWindowFullscreenRect(const bool value, const int32_t x, const int32_t y, const int32_t width, const int32_t height) {
	// @IMPLEMENT(final/X11): fplSetWindowFullscreenRect
	return(false);
}

fpl_platform_api bool fplEnableWindowFullscreen() {
	bool result = fplSetWindowFullscreenSize(true, 0, 0, 0);
	return(result);
}

fpl_platform_api bool fplDisableWindowFullscreen() {
	bool result = fplSetWindowFullscreenSize(false, 0, 0, 0);
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

fpl_platform_api void fplSetWindowTitle(const char *title) {
	// @BUG(final/X11): Setting the window title on X11 works, but it wont be set for the icon in the bars in gnome/ubuntu the icon title is always "Unbekannt" on my german environment.

	FPL__CheckArgumentNullNoRet(title);
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;

	fplCopyString(title, appState->currentSettings.window.title, fplArrayCount(appState->currentSettings.window.title));

	x11Api->XChangeProperty(windowState->display, windowState->window,
		windowState->netWMName, windowState->utf8String, 8,
		PropModeReplace,
		(unsigned char *)title, (int)fplGetStringLength(title));

	x11Api->XChangeProperty(windowState->display, windowState->window,
		windowState->netWMIconName, windowState->utf8String, 8,
		PropModeReplace,
		(unsigned char *)title, (int)fplGetStringLength(title));

	x11Api->XFlush(windowState->display);
}

fpl_platform_api bool fplGetClipboardText(char *dest, const uint32_t maxDestLen) {
	// @IMPLEMENT(final/X11): fplGetClipboardText
	return false;
}

fpl_platform_api bool fplSetClipboardText(const char *text) {
	// @IMPLEMENT(final/X11): fplSetClipboardText
	return false;
}

fpl_platform_api bool fplPollKeyboardState(fplKeyboardState *outState) {
	FPL__CheckPlatform(false);
	FPL__CheckArgumentNull(outState, false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	char keysReturn[32] = fplZeroInit;
	if (x11Api->XQueryKeymap(windowState->display, keysReturn)) {
		fplClearStruct(outState);
		for (uint64_t keyCode = 0; keyCode < 256; ++keyCode) {
			bool isDown = (keysReturn[keyCode / 8] & (1 << (keyCode % 8))) != 0;
			outState->keyStatesRaw[keyCode] = isDown ? 1 : 0;
			fplKey mappedKey = fpl__GetMappedKey(&appState->window, keyCode);
			if (outState->buttonStatesMapped[(int)mappedKey] == fplButtonState_Release) {
				outState->buttonStatesMapped[(int)mappedKey] = isDown ? fplButtonState_Press : fplButtonState_Release;
			}
		}
		outState->modifiers = fplKeyboardModifierFlags_None;
		if (outState->buttonStatesMapped[fplKey_LeftShift] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_LShift;
		}
		if (outState->buttonStatesMapped[fplKey_RightShift] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_RShift;
		}
		if (outState->buttonStatesMapped[fplKey_LeftControl] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_LCtrl;
		}
		if (outState->buttonStatesMapped[fplKey_RightControl] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_RCtrl;
		}
		if (outState->buttonStatesMapped[fplKey_LeftAlt] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_LAlt;
		}
		if (outState->buttonStatesMapped[fplKey_RightAlt] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_RAlt;
		}
		if (outState->buttonStatesMapped[fplKey_LeftSuper] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_LSuper;
		}
		if (outState->buttonStatesMapped[fplKey_RightSuper] == fplButtonState_Press) {
			outState->modifiers |= fplKeyboardModifierFlags_RSuper;
		}
		// @FINISH(fina/X11): Get caps states (Capslock, Numlock, Scrolllock)

		result = true;
	}
	return(result);
}

fpl_platform_api bool fplPollMouseState(fplMouseState *outState) {
	FPL__CheckPlatform(false);
	FPL__CheckArgumentNull(outState, false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__X11SubplatformState *subplatform = &appState->x11;
	const fpl__X11Api *x11Api = &subplatform->api;
	const fpl__X11WindowState *windowState = &appState->window.x11;
	bool result = false;
	Window root, child;
	int rootx, rooty, winx, winy;
	unsigned int mask;
	if (x11Api->XQueryPointer(windowState->display, windowState->window, &root, &child, &rootx, &rooty, &winx, &winy, &mask)) {
		outState->x = winx;
		outState->y = winy;
		outState->buttonStates[fplMouseButtonType_Left] = (mask & Button1Mask) ? fplButtonState_Press : fplButtonState_Release;
		outState->buttonStates[fplMouseButtonType_Right] = (mask & Button3Mask) ? fplButtonState_Press : fplButtonState_Release;
		outState->buttonStates[fplMouseButtonType_Middle] = (mask & Button2Mask) ? fplButtonState_Press : fplButtonState_Release;
		result = true;
	}
	return(result);
}

fpl_platform_api bool fplQueryCursorPosition(int32_t *outX, int32_t *outY) {
	// @IMPLEMENT(final/X11) fplQueryCursorPosition
	return(false);
}
#endif // FPL_SUBPLATFORM_X11

// ############################################################################
//
// > LINUX_PLATFORM
//
// ############################################################################
#if defined(FPL_PLATFORM_LINUX)
#	include <locale.h> // setlocale
#	include <sys/eventfd.h> // eventfd
#	include <sys/epoll.h> // epoll_create, epoll_ctl, epoll_wait
#	include <sys/select.h> // select
#	include <linux/joystick.h> // js_event, axis_state, etc.

fpl_internal void fpl__LinuxReleasePlatform(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
#if defined(FPL__ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_GameController) {
		fpl__LinuxFreeGameControllers(&appState->plinux.controllersState);
	}
#endif
}

fpl_internal bool fpl__LinuxInitPlatform(const fplInitFlags initFlags, const fplSettings *initSettings, fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	setlocale(LC_ALL, "");
	return true;
}

//
// Linux OS
//


//
// Linux Input
//
#if defined(FPL__ENABLE_WINDOW)
fpl_internal void fpl__LinuxFreeGameControllers(fpl__LinuxGameControllersState *controllersState) {
	for (int controllerIndex = 0; controllerIndex < fplArrayCount(controllersState->controllers); ++controllerIndex) {
		fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
		if (controller->fd > 0) {
			close(controller->fd);
			controller->fd = 0;
		}
	}
}

fpl_internal float fpl__LinuxJoystickProcessStickValue(const int16_t value, const int16_t deadZoneThreshold) {
	float result = 0;
	if (value < -deadZoneThreshold) {
		result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
	} else if (value > deadZoneThreshold) {
		result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
	}
	return(result);
}

fpl_internal void fpl__LinuxPushGameControllerStateUpdateEvent(const struct js_event *event, fpl__LinuxGameController *controller) {
	fplGamepadState *padState = &controller->state;

	// @TODO(final): Use a static offset table instead of a pointer mapping table
	fplGamepadButton *buttonMappingTable[12] = fplZeroInit;
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

	switch (event->type & ~JS_EVENT_INIT) {
		case JS_EVENT_AXIS:
		{
			switch (event->number) {
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
					if (event->value == -32767) {
						padState->dpadLeft.isDown = true;
						padState->dpadRight.isDown = false;
					} else if (event->value == 32767) {
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
					if (event->value == -32767) {
						padState->dpadUp.isDown = true;
						padState->dpadDown.isDown = false;
					} else if (event->value == 32767) {
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
			if ((event->number >= 0) && (event->number < fplArrayCount(buttonMappingTable))) {
				fplGamepadButton *mappedButton = buttonMappingTable[event->number];
				if (mappedButton != fpl_null) {
					mappedButton->isDown = event->value != 0;
				}
			}
		} break;

		default:
			break;
	}
}

fpl_internal void fpl__LinuxPollGameControllers(const fplSettings *settings, fpl__LinuxGameControllersState *controllersState, const bool useEvents) {
	// https://github.com/underdoeg/ofxGamepad
	// https://github.com/elanthis/gamepad
	// https://gist.github.com/jasonwhite/c5b2048c15993d285130
	// https://github.com/Tasssadar/libenjoy/blob/master/src/libenjoy_linux.c

	if (((controllersState->lastCheckTime == 0) || ((fplMillisecondsQuery() - controllersState->lastCheckTime) >= settings->input.controllerDetectionFrequency)) || !useEvents) {
		controllersState->lastCheckTime = fplMillisecondsQuery();

		//
		// Detect new controllers
		//
		const char *deviceNames[] = {
			"/dev/input/js0",
		};
		for (int deviceNameIndex = 0; deviceNameIndex < fplArrayCount(deviceNames); ++deviceNameIndex) {
			const char *deviceName = deviceNames[deviceNameIndex];
			bool alreadyFound = false;
			int freeIndex = -1;
			for (uint32_t controllerIndex = 0; controllerIndex < fplArrayCount(controllersState->controllers); ++controllerIndex) {
				fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
				if ((controller->fd > 0) && fplIsStringEqual(deviceName, controller->deviceName)) {
					alreadyFound = true;
					break;
				}
				if (controller->fd == 0) {
					if (freeIndex == -1) {
						freeIndex = controllerIndex;
					}
				}
			}
			if (!alreadyFound && freeIndex >= 0) {
				int fd = open(deviceName, O_RDONLY);
				if (fd < 0) {
					FPL_LOG_ERROR(FPL__MODULE_LINUX, "Failed opening joystick device '%s'", deviceName);
					continue;
				}
				uint8_t numAxis = 0;
				uint8_t numButtons = 0;
				ioctl(fd, JSIOCGAXES, &numAxis);
				ioctl(fd, JSIOCGBUTTONS, &numButtons);
				if (numAxis == 0 || numButtons == 0) {
					FPL_LOG_ERROR(FPL__MODULE_LINUX, "Joystick device '%s' does not have enough buttons/axis to map to a XInput controller!", deviceName);
					close(fd);
					continue;
				}

				// NOTE(final): We do not want to detect devices which are not proper joysticks, such as gaming keyboards
				struct js_event msg;
				if ((read(fd, &msg, sizeof(struct js_event)) != sizeof(struct js_event)) || !((msg.type == JS_EVENT_INIT) || (msg.type == JS_EVENT_AXIS) || (msg.type == JS_EVENT_BUTTON))) {
					// No joystick message
					close(fd);
					continue;
				}

				fpl__LinuxGameController *controller = controllersState->controllers + freeIndex;
				fplClearStruct(controller);
				controller->fd = fd;
				controller->axisCount = numAxis;
				controller->buttonCount = numButtons;
				fplCopyString(deviceName, controller->deviceName, fplArrayCount(controller->deviceName));
				ioctl(fd, JSIOCGNAME(fplArrayCount(controller->displayName)), controller->displayName);
				fcntl(fd, F_SETFL, O_NONBLOCK);

				if (useEvents) {
					// Push connected event
					fplEvent ev = fplZeroInit;
					ev.type = fplEventType_Gamepad;
					ev.gamepad.type = fplGamepadEventType_Connected;
					ev.gamepad.deviceIndex = (uint32_t)freeIndex;
					ev.gamepad.deviceName = controller->deviceName;
					fpl__PushInternalEvent(&ev);
				}
			}
		}
	}

	// Update controller states
	for (uint32_t controllerIndex = 0; controllerIndex < fplArrayCount(controllersState->controllers); ++controllerIndex) {
		fpl__LinuxGameController *controller = controllersState->controllers + controllerIndex;
		if (controller->fd > 0) {
			// Update button/axis state
			struct js_event event;
			bool wasDisconnected = false;
			for (;;) {
				errno = 0;
				if (read(controller->fd, &event, sizeof(event)) < 0) {
					if (errno == ENODEV) {
						close(controller->fd);
						controller->fd = 0;
						fplClearStruct(&controller->state);
						wasDisconnected = true;
						if (useEvents) {
							// Push disconnected event
							fplEvent ev = fplZeroInit;
							ev.type = fplEventType_Gamepad;
							ev.gamepad.type = fplGamepadEventType_Disconnected;
							ev.gamepad.deviceIndex = controllerIndex;
							ev.gamepad.deviceName = controller->deviceName;
							fpl__PushInternalEvent(&ev);
						}
					}
					break;
				}
				fpl__LinuxPushGameControllerStateUpdateEvent(&event, controller);
			}

			controller->state.isActive = !fpl__IsZeroMemory(&controller->state, sizeof(fplGamepadState));
			controller->state.isConnected = !wasDisconnected;
			controller->state.deviceName = controller->deviceName;

			if (controller->fd > 0) {
				if (useEvents) {
					// Push state event
					fplEvent ev = fplZeroInit;
					ev.type = fplEventType_Gamepad;
					ev.gamepad.type = fplGamepadEventType_StateChanged;
					ev.gamepad.deviceIndex = controllerIndex;
					ev.gamepad.deviceName = controller->deviceName;
					ev.gamepad.state = controller->state;
					fpl__PushInternalEvent(&ev);
				}
			}
		}
	}
}

fpl_platform_api bool fplPollGamepadStates(fplGamepadStates *outStates) {
	FPL__CheckPlatform(false);
	FPL__CheckArgumentNull(outStates, false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	if (appState->initFlags & fplInitFlags_GameController) {
#if defined(FPL_PLATFORM_LINUX)
		fpl__LinuxGameControllersState *controllersState = &appState->plinux.controllersState;
		fpl__LinuxPollGameControllers(&appState->currentSettings, controllersState, false);

		fplAssert(fplArrayCount(controllersState->controllers) == fplArrayCount(outStates->deviceStates));
		for (int i = 0; i < fplArrayCount(controllersState->controllers); ++i) {
			outStates->deviceStates[i] = controllersState->controllers[i].state;
		}
		return(true);
#endif
	}
	return(false);
}

#endif // FPL__ENABLE_WINDOW

//
// Linux Threading
//
fpl_platform_api bool fplSignalInit(fplSignalHandle *signal, const fplSignalValue initialValue) {
	FPL__CheckArgumentNull(signal, false);
	if (signal->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal '%p' is already valid", signal);
		return false;
	}
	int linuxEventHandle = eventfd((initialValue == fplSignalValue_Set) ? 1 : 0, EFD_CLOEXEC);
	if (linuxEventHandle == -1) {
		FPL__ERROR(FPL__MODULE_THREADING, "Failed initializing signal '%p'", signal);
		return false;
	}
	fplClearStruct(signal);
	signal->isValid = true;
	signal->internalHandle.linuxEventHandle = linuxEventHandle;
	return(true);
}

fpl_platform_api void fplSignalDestroy(fplSignalHandle *signal) {
	if (signal != fpl_null && signal->isValid) {
		close(signal->internalHandle.linuxEventHandle);
		fplClearStruct(signal);
	}
}

fpl_platform_api bool fplSignalWaitForOne(fplSignalHandle *signal, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signal, false);
	if (!signal->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal '%p' is not valid", signal);
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

fpl_internal bool fpl__LinuxSignalWaitForMultiple(fplSignalHandle *signals[], const uint32_t minCount, const uint32_t maxCount, const size_t stride, const fplTimeoutValue timeout) {
	FPL__CheckArgumentNull(signals, false);
	FPL__CheckArgumentMax(maxCount, FPL_MAX_SIGNAL_COUNT, false);
	const size_t actualStride = stride > 0 ? stride : sizeof(fplSignalHandle *);
	for (uint32_t index = 0; index < maxCount; ++index) {
		fplSignalHandle *signal = *(fplSignalHandle **)((uint8_t *)signals + index * actualStride);
		if (signal == fpl_null) {
			FPL__ERROR(FPL__MODULE_THREADING, "Signal for index '%d' are not allowed to be null", index);
			return false;
		}
		if (!signal->isValid) {
			FPL__ERROR(FPL__MODULE_THREADING, "Signal '%p' for index '%d' is not valid", signal, index);
			return false;
		}
	}

	int e = epoll_create(maxCount);
	fplAssert(e != 0);

	// @MEMORY(final): This wastes a lof memory, use temporary memory allocation here

	// Register events and map each to the array index
	struct epoll_event events[FPL_MAX_SIGNAL_COUNT];
	for (int index = 0; index < maxCount; index++) {
		events[index].events = EPOLLIN;
		events[index].data.u32 = index;
		fplSignalHandle *signal = *(fplSignalHandle **)((uint8_t *)signals + index * actualStride);
		int x = epoll_ctl(e, EPOLL_CTL_ADD, signal->internalHandle.linuxEventHandle, events + index);
		fplAssert(x == 0);
	}

	// Wait
	int t = timeout == FPL_TIMEOUT_INFINITE ? -1 : timeout;
	int eventsResult = -1;
	int waiting = minCount;
	struct epoll_event revent[FPL_MAX_SIGNAL_COUNT];
	while (waiting > 0) {
		int ret = epoll_wait(e, revent, waiting, t);
		if (ret == 0) {
			if (minCount == maxCount) {
				eventsResult = -1;
			}
			break;
		}
		for (int eventIndex = 0; eventIndex < ret; eventIndex++) {
			uint32_t signalIndex = revent[eventIndex].data.u32;
			fplSignalHandle *signal = *(fplSignalHandle **)((uint8_t *)signals + signalIndex * actualStride);
			epoll_ctl(e, EPOLL_CTL_DEL, signal->internalHandle.linuxEventHandle, NULL);
		}
		eventsResult = revent[0].data.u32;
		waiting -= ret;
	}
	close(e);
	bool result = (waiting == 0);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAll(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__LinuxSignalWaitForMultiple(signals, count, count, stride, timeout);
	return(result);
}

fpl_platform_api bool fplSignalWaitForAny(fplSignalHandle **signals, const size_t count, const size_t stride, const fplTimeoutValue timeout) {
	bool result = fpl__LinuxSignalWaitForMultiple(signals, 1, count, stride, timeout);
	return(result);
}

fpl_platform_api bool fplSignalSet(fplSignalHandle *signal) {
	FPL__CheckArgumentNull(signal, false);
	if (!signal->isValid) {
		FPL__ERROR(FPL__MODULE_THREADING, "Signal '%p' is not valid", signal);
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
fpl_platform_api bool fplMemoryGetInfos(fplMemoryInfos *outInfos) {
	FPL__CheckArgumentNull(outInfos, false);
	bool result = false;

	// https://git.i-scream.org/?p=libstatgrab.git;a=blob;f=src/libstatgrab/memory_stats.c;h=a6f6fb926b72d3b691848202e397e3db58255648;hb=HEAD

	return(result);
}

//
// Linux Paths
//
fpl_internal size_t fpl__LinuxLocaleToISO639(const char *source, char *target, const size_t maxTargetLen) {
	size_t result = fplGetStringLength(source);
	if (target != fpl_null) {
		fplCopyStringLen(source, result, target, maxTargetLen);
		char *p = target;
		while (*p) {
			if (*p == '_') {
				*p = '-'; // Replace underscore with minus
			} else if (*p == '.') {
				*p = '\0'; // Replace dot with zero char
				break;
			}
			++p;
		}
	}
	return(result);
}

//
// Linux Localization
//
fpl_platform_api size_t fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	char *locale = setlocale(LC_CTYPE, NULL);
	size_t result = fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
	return(result);
}

fpl_platform_api size_t fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	char *locale = setlocale(LC_ALL, NULL);
	size_t result = fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
	return(result);
}

fpl_platform_api size_t fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	FPL__CheckArgumentInvalid(targetFormat, targetFormat == fplLocaleFormat_None, 0);
	char *locale = setlocale(LC_ALL, NULL);
	size_t result = fpl__LinuxLocaleToISO639(locale, buffer, maxBufferLen);
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
// Unix Hardware
//
fpl_platform_api bool fplMemoryGetInfos(fplMemoryInfos *outInfos) {
	// @IMPLEMENT(final/Unix): fplMemoryGetInfos
	return(false);
}

//
// Unix Localization
//
fpl_platform_api size_t fplGetSystemLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	// @IMPLEMENT(final/Unix): fplGetSystemLocale
	return(0);
}

fpl_platform_api size_t fplGetUserLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	// @IMPLEMENT(final/Unix): fplGetUserLocale
	return(0);
}

fpl_platform_api size_t fplGetInputLocale(const fplLocaleFormat targetFormat, char *buffer, const size_t maxBufferLen) {
	// @IMPLEMENT(final/Unix): fplGetInputLocale
	return(0);
}
#endif // FPL_PLATFORM_UNIX

// ****************************************************************************
//
// > VIDEO_BACKENDS
//
// ****************************************************************************
#if !defined(FPL__VIDEO_BACKENDS_IMPLEMENTED) && defined(FPL__ENABLE_VIDEO)
#	define FPL__VIDEO_BACKENDS_IMPLEMENTED

//
// Video Backend Abstraction
//

// Video data used for every backend (Software backbuffer for now)
typedef struct fpl__VideoData {
#if defined(FPL__ENABLE_VIDEO_SOFTWARE)
	fplVideoBackBuffer backbuffer;
#endif
	uint64_t dummy;
} fpl__VideoData;

struct fpl__VideoBackend;

#define FPL__FUNC_VIDEO_BACKEND_LOAD(name) bool name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__func_VideoBackendLoad);

#define FPL__FUNC_VIDEO_BACKEND_UNLOAD(name) void name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__func_VideoBackendUnload);

#define FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(name) bool name(const fpl__PlatformAppState *appState, const fplVideoSettings *videoSettings, fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__func_VideoBackendPrepareWindow);

#define FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(name) bool name(const fpl__PlatformAppState *appState, const fplVideoSettings *videoSettings, fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__func_VideoBackendFinalizeWindow);

#define FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(name) void name(const fpl__PlatformAppState *appState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(fpl__func_VideoBackendDestroyedWindow);

#define FPL__FUNC_VIDEO_BACKEND_INITIALIZE(name) bool name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, const fplVideoSettings *videoSettings, const fpl__VideoData *data, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__func_VideoBackendInitialize);

#define FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(name) void name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__func_VideoBackendShutdown);

#define FPL__FUNC_VIDEO_BACKEND_PRESENT(name) void name(const fpl__PlatformAppState *appState, const fpl__PlatformWindowState *windowState, const fpl__VideoData *data, const struct fpl__VideoBackend *backend)
typedef FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__func_VideoBackendPresent);

#define FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(name) const void *name(const struct fpl__VideoBackend *backend, const char *procName)
typedef FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__func_VideoBackendGetProcedure);

#define FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(name) bool name(fplVideoRequirements *requirements)
typedef FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(fpl__func_VideoBackendGetRequirements);

typedef struct fpl__VideoContext {
	fpl__func_VideoBackendLoad *loadFunc;
	fpl__func_VideoBackendUnload *unloadFunc;
	fpl__func_VideoBackendInitialize *initializeFunc;
	fpl__func_VideoBackendShutdown *shutdownFunc;
	fpl__func_VideoBackendPrepareWindow *prepareWindowFunc;
	fpl__func_VideoBackendFinalizeWindow *finalizeWindowFunc;
	fpl__func_VideoBackendDestroyedWindow *destroyedWindowFunc;
	fpl__func_VideoBackendPresent *presentFunc;
	fpl__func_VideoBackendGetProcedure *getProcedureFunc;
	fpl__func_VideoBackendGetRequirements *getRequirementsFunc;
	fpl_b32 recreateOnResize;
} fpl__VideoContext;

// Video context stubs
fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_Load_Stub) { return(true); }
fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_Unload_Stub) {}
fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_PrepareWindow_Stub) { return(true); }
fpl_internal FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__VideoBackend_FinalizeWindow_Stub) { return(true); }
fpl_internal FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(fpl__VideoBackend_DestroyedWindow_Stub) {}
fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_Initialize_Stub) { return(false); }
fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_Shutdown_Stub) {}
fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_Present_Stub) {}
fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_GetProcedure_Stub) { return(fpl_null); }
fpl_internal FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(fpl__VideoBackend_GetRequirements_Stub) { return(false); }

fpl_internal fpl__VideoContext fpl__StubVideoContext() {
	fpl__VideoContext result = fplZeroInit;
	result.loadFunc = fpl__VideoBackend_Load_Stub;
	result.unloadFunc = fpl__VideoBackend_Unload_Stub;
	result.prepareWindowFunc = fpl__VideoBackend_PrepareWindow_Stub;
	result.finalizeWindowFunc = fpl__VideoBackend_FinalizeWindow_Stub;
	result.destroyedWindowFunc = fpl__VideoBackend_DestroyedWindow_Stub;
	result.initializeFunc = fpl__VideoBackend_Initialize_Stub;
	result.shutdownFunc = fpl__VideoBackend_Shutdown_Stub;
	result.presentFunc = fpl__VideoBackend_Present_Stub;
	result.getProcedureFunc = fpl__VideoBackend_GetProcedure_Stub;
	result.getRequirementsFunc = fpl__VideoBackend_GetRequirements_Stub;
	return(result);
}

// "VIDEOSYS" Video Backend Magic 8CC
#define FPL__VIDEOBACKEND_MAGIC (uint64_t)0x564944454f535953

typedef struct fpl__VideoBackend {
	uint64_t magic;
	fplVideoSurface surface;
} fpl__VideoBackend;

// ############################################################################
//
// > VIDEO_BACKEND_OPENGL_WIN32
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_OPENGL) && defined(FPL_PLATFORM_WINDOWS)

#define FPL__GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define FPL__GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define FPL__GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define FPL__GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#define FPL__GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define FPL__GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

#define FPL__WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define FPL__WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define FPL__WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define FPL__WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define FPL__WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define FPL__WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define FPL__WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define FPL__WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define FPL__WGL_CONTEXT_FLAGS_ARB 0x2094
#define FPL__WGL_DRAW_TO_WINDOW_ARB 0x2001
#define FPL__WGL_ACCELERATION_ARB 0x2003
#define FPL__WGL_SWAP_METHOD_ARB 0x2007
#define FPL__WGL_SUPPORT_OPENGL_ARB 0x2010
#define FPL__WGL_DOUBLE_BUFFER_ARB 0x2011
#define FPL__WGL_PIXEL_TYPE_ARB 0x2013
#define FPL__WGL_COLOR_BITS_ARB 0x2014
#define FPL__WGL_ALPHA_BITS_ARB 0x201B
#define FPL__WGL_DEPTH_BITS_ARB 0x2022
#define FPL__WGL_STENCIL_BITS_ARB 0x2023
#define FPL__WGL_FULL_ACCELERATION_ARB 0x2027
#define FPL__WGL_SWAP_EXCHANGE_ARB 0x2028
#define FPL__WGL_TYPE_RGBA_ARB 0x202B
#define FPL__WGL_SAMPLE_BUFFERS_ARB 0x2041
#define FPL__WGL_SAMPLES_ARB 0x2042

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

fpl_internal void fpl__UnloadWin32OpenGLApi(fpl__Win32OpenGLApi *api) {
	if (api->openglLibrary != fpl_null) {
		FreeLibrary(api->openglLibrary);
	}
	fplClearStruct(api);
}

fpl_internal bool fpl__LoadWin32OpenGLApi(fpl__Win32OpenGLApi *api, const char *libraryName) {
	if (fplGetStringLength(libraryName) == 0) {
		libraryName = "opengl32.dll";
	}
	bool result = false;
	fplClearStruct(api);
	do {
		HMODULE openglLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_VIDEO_OPENGL, openglLibrary, libraryName);
		api->openglLibrary = openglLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_VIDEO_OPENGL, openglLibrary, libraryName, api, fpl__win32_func_wglGetProcAddress, wglGetProcAddress);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_VIDEO_OPENGL, openglLibrary, libraryName, api, fpl__win32_func_wglCreateContext, wglCreateContext);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_VIDEO_OPENGL, openglLibrary, libraryName, api, fpl__win32_func_wglDeleteContext, wglDeleteContext);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_VIDEO_OPENGL, openglLibrary, libraryName, api, fpl__win32_func_wglMakeCurrent, wglMakeCurrent);
		result = true;
	} while (0);
	if (!result) {
		fpl__UnloadWin32OpenGLApi(api);
	}
	return(result);
}

typedef struct fpl__VideoBackendWin32OpenGL {
	fpl__VideoBackend base;
	fpl__Win32OpenGLApi api;
	HGLRC renderingContext;
} fpl__VideoBackendWin32OpenGL;

fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_Win32OpenGL_GetProcedure) {
	const fpl__VideoBackendWin32OpenGL *nativeBackend = (const fpl__VideoBackendWin32OpenGL *)backend;
	void *result = (void *)GetProcAddress(nativeBackend->api.openglLibrary, procName);
	return(result);
}

fpl_internal LRESULT CALLBACK fpl__Win32OpenGLTemporaryWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	fpl__Win32AppState *appState = &fpl__global__AppState->win32;
	const fpl__Win32Api *wapi = &appState->winApi;
	switch (message) {
		case WM_CLOSE:
			wapi->user.PostQuitMessage(0);
			break;
		default:
			return wapi->user.DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_Win32OpenGL_PrepareWindow) {
	const fpl__Win32AppState *nativeAppState = &appState->win32;
	const fpl__Win32Api *wapi = &nativeAppState->winApi;
	fpl__Win32WindowState *nativeWindowState = &windowState->win32;

	nativeWindowState->pixelFormat = 0;

	if (videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) {
		fpl__Win32OpenGLApi glApi;
		if (fpl__LoadWin32OpenGLApi(&glApi, videoSettings->graphics.opengl.libraryFile)) {
			// Register temporary window class
			WNDCLASSEXW windowClass = fplZeroInit;
			windowClass.cbSize = sizeof(windowClass);
			windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			windowClass.lpfnWndProc = fpl__Win32OpenGLTemporaryWindowProc;
			windowClass.hInstance = GetModuleHandleW(fpl_null);
			windowClass.hCursor = fpl__win32_LoadCursor(fpl_null, IDC_ARROW);
			windowClass.lpszClassName = L"FPL_Temp_GL_Window";
			if (wapi->user.RegisterClassExW(&windowClass)) {
				// Create temporary window
				HWND tempWindowHandle = wapi->user.CreateWindowExW(0, windowClass.lpszClassName, L"FPL Temp GL Window", 0, 0, 0, 1, 1, fpl_null, fpl_null, windowClass.hInstance, fpl_null);
				if (tempWindowHandle != fpl_null) {
					// Get temporary device context
					HDC tempDC = wapi->user.GetDC(tempWindowHandle);
					if (tempDC != fpl_null) {
						// Get legacy pixel format
						PIXELFORMATDESCRIPTOR fakePFD = fplZeroInit;
						fakePFD.nSize = sizeof(fakePFD);
						fakePFD.nVersion = 1;
						fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
						fakePFD.iPixelType = PFD_TYPE_RGBA;
						fakePFD.cColorBits = 32;
						fakePFD.cAlphaBits = 8;
						fakePFD.cDepthBits = 24;
						int fakePFDID = wapi->gdi.ChoosePixelFormat(tempDC, &fakePFD);
						if (fakePFDID != 0) {
							if (wapi->gdi.SetPixelFormat(tempDC, fakePFDID, &fakePFD)) {
								// Create temporary rendering context
								HGLRC tempCtx = glApi.wglCreateContext(tempDC);
								if (tempCtx != fpl_null) {
									if (glApi.wglMakeCurrent(tempDC, tempCtx)) {
										glApi.wglChoosePixelFormatARB = (fpl__win32_func_wglChoosePixelFormatARB *)glApi.wglGetProcAddress("wglChoosePixelFormatARB");
										if (glApi.wglChoosePixelFormatARB != fpl_null) {
											int multisampleCount = (int)videoSettings->graphics.opengl.multiSamplingCount;
											const int pixelAttribs[] = {
												FPL__WGL_DRAW_TO_WINDOW_ARB, 1,
												FPL__WGL_SUPPORT_OPENGL_ARB, 1,
												FPL__WGL_DOUBLE_BUFFER_ARB, 1,
												FPL__WGL_PIXEL_TYPE_ARB, FPL__WGL_TYPE_RGBA_ARB,
												FPL__WGL_ACCELERATION_ARB, FPL__WGL_FULL_ACCELERATION_ARB,
												FPL__WGL_COLOR_BITS_ARB, 32,
												FPL__WGL_ALPHA_BITS_ARB, 8,
												FPL__WGL_DEPTH_BITS_ARB, 24,
												FPL__WGL_STENCIL_BITS_ARB, 8,
												FPL__WGL_SAMPLE_BUFFERS_ARB, (multisampleCount > 0) ? 1 : 0,
												FPL__WGL_SAMPLES_ARB, multisampleCount,
												0
											};
											int pixelFormat;
											UINT numFormats;
											if (glApi.wglChoosePixelFormatARB(tempDC, pixelAttribs, NULL, 1, &pixelFormat, &numFormats)) {
												nativeWindowState->pixelFormat = pixelFormat;
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
			fpl__UnloadWin32OpenGLApi(&glApi);
		}
	}
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__VideoBackend_Win32OpenGL_FinalizeWindow) {
	const fpl__Win32AppState *nativeAppState = &appState->win32;
	const fpl__Win32Api *wapi = &nativeAppState->winApi;
	fpl__Win32WindowState *nativeWindowState = &windowState->win32;

	//
	// Prepare window for OpenGL
	//
	HDC deviceContext = nativeWindowState->deviceContext;
	HWND handle = nativeWindowState->windowHandle;
	PIXELFORMATDESCRIPTOR pfd = fplZeroInit;

	// We may got a pixel format from the pre-setup
	bool pixelFormatSet = false;
	if (nativeWindowState->pixelFormat != 0) {
		wapi->gdi.DescribePixelFormat(deviceContext, nativeWindowState->pixelFormat, sizeof(pfd), &pfd);
		pixelFormatSet = wapi->gdi.SetPixelFormat(deviceContext, nativeWindowState->pixelFormat, &pfd) == TRUE;
		if (!pixelFormatSet) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed setting Pixelformat '%d' from pre setup", nativeWindowState->pixelFormat);
		}
	}
	if (!pixelFormatSet) {
		fplClearStruct(&pfd);
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
		if (!pixelFormat) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
			return false;
		}
		wapi->gdi.DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);
		if (!wapi->gdi.SetPixelFormat(deviceContext, pixelFormat, &pfd)) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
			return false;
		}
	}
	return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_Win32OpenGL_Shutdown) {
	const fpl__Win32AppState *nativeAppState = &appState->win32;

	fpl__VideoBackendWin32OpenGL *nativeBackend = (fpl__VideoBackendWin32OpenGL *)backend;
	const fpl__Win32OpenGLApi *glapi = &nativeBackend->api;

	if (nativeBackend->renderingContext) {
		glapi->wglMakeCurrent(fpl_null, fpl_null);
		glapi->wglDeleteContext(nativeBackend->renderingContext);
		nativeBackend->renderingContext = fpl_null;
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_Win32OpenGL_Initialize) {
	const fpl__Win32AppState *nativeAppState = &appState->win32;
	const fpl__Win32WindowState *nativeWindowState = &windowState->win32;
	const fpl__Win32Api *wapi = &nativeAppState->winApi;

	fpl__VideoBackendWin32OpenGL *nativeBackend = (fpl__VideoBackendWin32OpenGL *)backend;

	fpl__Win32OpenGLApi *glapi = &nativeBackend->api;

	//
	// Create opengl rendering context
	//
	HDC deviceContext = nativeWindowState->deviceContext;
	HGLRC legacyRenderingContext = glapi->wglCreateContext(deviceContext);
	if (!legacyRenderingContext) {
		FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed creating Legacy OpenGL Rendering Context for DC '%x')", deviceContext);
		return false;
	}
	if (!glapi->wglMakeCurrent(deviceContext, legacyRenderingContext)) {
		FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')", deviceContext, legacyRenderingContext);
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
	if (videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if (!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			return false;
		}
		if (glapi->wglChoosePixelFormatARB == fpl_null) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
			return false;
		}
		if (glapi->wglCreateContextAttribsARB == fpl_null) {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			return false;
		}

		int profile = 0;
		int flags = 0;
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL__WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL__WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "No opengl compability profile selected, please specific Core fplOpenGLCompabilityFlags_Core or fplOpenGLCompabilityFlags_Compability");
			return false;
		}
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
			flags = FPL__WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		}

		int contextAttribIndex = 0;
		int contextAttribList[20 + 1] = fplZeroInit;
		contextAttribList[contextAttribIndex++] = FPL__WGL_CONTEXT_MAJOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)videoSettings->graphics.opengl.majorVersion;
		contextAttribList[contextAttribIndex++] = FPL__WGL_CONTEXT_MINOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)videoSettings->graphics.opengl.minorVersion;
		contextAttribList[contextAttribIndex++] = FPL__WGL_CONTEXT_PROFILE_MASK_ARB;
		contextAttribList[contextAttribIndex++] = profile;
		if (flags > 0) {
			contextAttribList[contextAttribIndex++] = FPL__WGL_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}

		// Create modern opengl rendering context
		HGLRC modernRenderingContext = glapi->wglCreateContextAttribsARB(deviceContext, 0, contextAttribList);
		if (modernRenderingContext) {
			if (!glapi->wglMakeCurrent(deviceContext, modernRenderingContext)) {
				FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

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
			FPL__ERROR(FPL__MODULE_VIDEO_OPENGL, "Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) and DC '%x') -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags, deviceContext);

			// Fallback to legacy context
			glapi->wglMakeCurrent(deviceContext, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		}
	} else {
		// Caller wants legacy context
		glapi->wglMakeCurrent(deviceContext, legacyRenderingContext);
		activeRenderingContext = legacyRenderingContext;
	}

	fplAssert(activeRenderingContext != fpl_null);
	nativeBackend->renderingContext = activeRenderingContext;

	// Set vertical syncronisation if available
	if (glapi->wglSwapIntervalEXT != fpl_null) {
		int swapInterval = videoSettings->isVSync ? 1 : 0;
		glapi->wglSwapIntervalEXT(swapInterval);
	}

	backend->surface.window.win32.deviceContext = deviceContext;
	backend->surface.window.win32.windowHandle = nativeWindowState->windowHandle;
	backend->surface.opengl.renderingContext = (void *)activeRenderingContext;

	return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_Win32OpenGL_Unload) {
	fpl__VideoBackendWin32OpenGL *nativeBackend = (fpl__VideoBackendWin32OpenGL *)backend;
	fpl__UnloadWin32OpenGLApi(&nativeBackend->api);
	fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_Win32OpenGL_Load) {
	fpl__VideoBackendWin32OpenGL *nativeBackend = (fpl__VideoBackendWin32OpenGL *)backend;
	const fplVideoSettings *videoSettings = &appState->currentSettings.video;
	fplClearStruct(nativeBackend);
	nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;
	if (!fpl__LoadWin32OpenGLApi(&nativeBackend->api, videoSettings->graphics.opengl.libraryFile)) {
		return(false);
	}
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_Win32OpenGL_Present) {
	const fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *win32WindowState = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	const fpl__VideoBackendWin32OpenGL *nativeBackend = (fpl__VideoBackendWin32OpenGL *)backend;
	wapi->gdi.SwapBuffers(win32WindowState->deviceContext);
}

fpl_internal fpl__VideoContext fpl__VideoBackend_Win32OpenGL_Construct() {
	fpl__VideoContext result = fpl__StubVideoContext();
	result.loadFunc = fpl__VideoBackend_Win32OpenGL_Load;
	result.unloadFunc = fpl__VideoBackend_Win32OpenGL_Unload;
	result.getProcedureFunc = fpl__VideoBackend_Win32OpenGL_GetProcedure;
	result.initializeFunc = fpl__VideoBackend_Win32OpenGL_Initialize;
	result.shutdownFunc = fpl__VideoBackend_Win32OpenGL_Shutdown;
	result.prepareWindowFunc = fpl__VideoBackend_Win32OpenGL_PrepareWindow;
	result.finalizeWindowFunc = fpl__VideoBackend_Win32OpenGL_FinalizeWindow;
	result.presentFunc = fpl__VideoBackend_Win32OpenGL_Present;
	return(result);
}
#endif // FPL__ENABLE_VIDEO_OPENGL && FPL_PLATFORM_WINDOWS

// ############################################################################
//
// > VIDEO_BACKEND_OPENGL_X11
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_OPENGL) && defined(FPL_SUBPLATFORM_X11)
#ifndef __gl_h_
typedef uint8_t GLubyte;
#endif
#ifndef GLX_H
typedef XID GLXDrawable;
typedef XID GLXWindow;
typedef void GLXContext_Void;
typedef GLXContext_Void *GLXContext;
typedef void GLXFBConfig_Void;
typedef GLXFBConfig_Void *GLXFBConfig;

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

fpl_internal void fpl__UnloadX11OpenGLApi(fpl__X11VideoOpenGLApi *api) {
	if (api->libHandle != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Unload Api (Library '%p')", api->libHandle);
		dlclose(api->libHandle);
	}
	fplClearStruct(api);
}

fpl_internal bool fpl__LoadX11OpenGLApi(fpl__X11VideoOpenGLApi *api, const char *libraryName) {
	uint32_t libFileCount = 0;
	
	const char *libFileNames[4];
	if (fplGetStringLength(libraryName) > 0) {
		libFileNames[libFileCount++] = libraryName;
	} else {
		libFileNames[libFileCount++] = "libGL.so.1";
		libFileNames[libFileCount++] = "libGL.so";
	}

	bool result = false;
	for (uint32_t index = 0; index < libFileCount; ++index) {
		const char *libName = libFileNames[index];
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Load GLX Api from Library: %s", libName);
		do {
			void *libHandle = fpl_null;
			FPL__POSIX_LOAD_LIBRARY(FPL__MODULE_GLX, libHandle, libName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXQueryVersion, glXQueryVersion);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXChooseVisual, glXChooseVisual);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXCreateContext, glXCreateContext);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXDestroyContext, glXDestroyContext);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXCreateNewContext, glXCreateNewContext);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXMakeCurrent, glXMakeCurrent);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXSwapBuffers, glXSwapBuffers);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXGetProcAddress, glXGetProcAddress);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXChooseFBConfig, glXChooseFBConfig);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXGetFBConfigs, glXGetFBConfigs);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXGetVisualFromFBConfig, glXGetVisualFromFBConfig);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXGetFBConfigAttrib, glXGetFBConfigAttrib);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXCreateWindow, glXCreateWindow);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXQueryExtension, glXQueryExtension);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_GLX, libHandle, libName, api, fpl__func_glx_glXQueryExtensionsString, glXQueryExtensionsString);
			api->libHandle = libHandle;
			result = true;
		} while (0);
		if (result) {
			FPL_LOG_DEBUG(FPL__MODULE_GLX, , "Successfully loaded GLX Api from Library '%s'", libName);
			break;
		}
		fpl__UnloadX11OpenGLApi(api);
	}
	return (result);
}

typedef struct fpl__VideoBackendX11OpenGL {
	fpl__VideoBackend base;
	fpl__X11VideoOpenGLApi api;
	GLXFBConfig fbConfig;
	XVisualInfo *visualInfo;
	GLXContext context;
	bool isActiveContext;
} fpl__VideoBackendX11OpenGL;

fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_X11OpenGL_GetProcedure) {
	const fpl__VideoBackendX11OpenGL *nativeBackend = (const fpl__VideoBackendX11OpenGL *)backend;
	void *result = dlsym(nativeBackend->api.libHandle, procName);
	return(result);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_X11OpenGL_PrepareWindow) {
	const fpl__X11SubplatformState *nativeAppState = &appState->x11;
	const fpl__X11Api *x11Api = &nativeAppState->api;

	fpl__X11WindowState *nativeWindowState = &windowState->x11;
	fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	fpl__X11VideoOpenGLApi *glApi = &nativeBackend->api;

	Display *display = nativeWindowState->display;
	Window window = nativeWindowState->window;
	int screen = nativeWindowState->screen;

	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Query GLX version for display '%p'", display);
	int major = 0, minor = 0;
	if (!glApi->glXQueryVersion(display, &major, &minor)) {
		FPL_LOG_ERROR(FPL__MODULE_GLX, "Failed querying GLX version for display '%p'", display);
		return false;
	}
	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully queried GLX version for display '%p': %d.%d", display, major, minor);

	// @NOTE(final): Required for AMD Drivers?

	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Query OpenGL extension on display '%p'", display);
	if (!glApi->glXQueryExtension(display, fpl_null, fpl_null)) {
		FPL__ERROR(FPL__MODULE_GLX, "OpenGL GLX Extension is not supported by the active display '%p'", display);
		return false;
	}

	const char *extensionString = glApi->glXQueryExtensionsString(display, screen);
	if (extensionString != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "OpenGL GLX extensions: %s", extensionString);
	}

	bool isModern = major > 1 || (major == 1 && minor >= 3);

	int attr[32] = fplZeroInit;
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

	attr[attrIndex++] = GLX_ALPHA_SIZE;
	attr[attrIndex++] = 8;

	attr[attrIndex++] = GLX_DEPTH_SIZE;
	attr[attrIndex++] = 24;

	attr[attrIndex++] = GLX_STENCIL_SIZE;
	attr[attrIndex++] = 8;

	if (videoSettings->graphics.opengl.multiSamplingCount > 0) {
		attr[attrIndex++] = GLX_SAMPLE_BUFFERS;
		attr[attrIndex++] = 1;

		attr[attrIndex++] = GLX_SAMPLES;
		attr[attrIndex++] = (int)videoSettings->graphics.opengl.multiSamplingCount;
	}

	attr[attrIndex] = 0;

	if (isModern) {
		// Use frame buffer config approach (GLX >= 1.3)
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Get framebuffer configuration from display '%p' and screen '%d'", display, screen);
		int configCount = 0;
		GLXFBConfig *configs = glApi->glXChooseFBConfig(display, screen, attr, &configCount);
		if (configs == fpl_null || !configCount) {
			FPL__ERROR(FPL__MODULE_GLX, "No framebuffer configuration from display '%p' and screen '%d' found!", display, screen);
			nativeBackend->fbConfig = fpl_null;
			return false;
		}
		nativeBackend->fbConfig = configs[0];
		nativeBackend->visualInfo = fpl_null;
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got framebuffer configuration from display '%p' and screen '%d': %p", display, screen, nativeBackend->fbConfig);

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Release %d framebuffer configurations", configCount);
		x11Api->XFree(configs);
	} else {
		// Use choose visual (Old way)
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Choose visual from display '%p' and screen '%d'", display, screen);
		XVisualInfo *visualInfo = glApi->glXChooseVisual(display, screen, attr);
		if (visualInfo == fpl_null) {
			FPL__ERROR(FPL__MODULE_GLX, "No visual info for display '%p' and screen '%d' found!", display, screen);
			return false;
		}
		nativeBackend->visualInfo = visualInfo;
		nativeBackend->fbConfig = fpl_null;
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got visual info from display '%p' and screen '%d': %p", display, screen, nativeBackend->visualInfo);
	}

	if (nativeBackend->fbConfig != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Get visual info from display '%p' and frame buffer config '%p'", display, nativeBackend->fbConfig);
		XVisualInfo *visualInfo = glApi->glXGetVisualFromFBConfig(display, nativeBackend->fbConfig);
		if (visualInfo == fpl_null) {
			FPL__ERROR(FPL__MODULE_GLX, "Failed getting visual info from display '%p' and frame buffer config '%p'", display, nativeBackend->fbConfig);
			return false;
		}
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully got visual info from display '%p' and frame buffer config '%p': %p", display, nativeBackend->fbConfig, visualInfo);

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using visual: %p", visualInfo->visual);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using color depth: %d", visualInfo->depth);

		nativeWindowState->visual = visualInfo->visual;
		nativeWindowState->colorDepth = visualInfo->depth;

		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Release visual info '%p'", visualInfo);
		x11Api->XFree(visualInfo);
	} else if (nativeBackend->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using existing visual info: %p", nativeBackend->visualInfo);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using visual: %p", nativeBackend->visualInfo->visual);
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Using color depth: %d", nativeBackend->visualInfo->depth);
		nativeWindowState->visual = nativeBackend->visualInfo->visual;
		nativeWindowState->colorDepth = nativeBackend->visualInfo->depth;
	} else {
		FPL__ERROR(FPL__MODULE_GLX, "No visual info or frame buffer config defined!");
		return false;
	}

	return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_X11OpenGL_Shutdown) {
	const fpl__X11SubplatformState *nativeAppState = &appState->x11;
	const fpl__X11Api *x11Api = &nativeAppState->api;
	const fpl__X11WindowState *nativeWindowState = &windowState->x11;

	fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	fpl__X11VideoOpenGLApi *glApi = &nativeBackend->api;

	if (nativeBackend->isActiveContext) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Deactivate GLX rendering context for display '%p'", nativeWindowState->display);
		glApi->glXMakeCurrent(nativeWindowState->display, 0, fpl_null);
		nativeBackend->isActiveContext = false;
	}

	if (nativeBackend->context != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy GLX rendering context '%p' for display '%p'", nativeBackend->context, nativeWindowState->display);
		glApi->glXDestroyContext(nativeWindowState->display, nativeBackend->context);
		nativeBackend->context = fpl_null;
	}

	if (nativeBackend->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy visual info '%p' (Fallback)", nativeBackend->visualInfo);
		x11Api->XFree(nativeBackend->visualInfo);
		nativeBackend->visualInfo = fpl_null;
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_X11OpenGL_Initialize) {
	const fpl__X11SubplatformState *nativeAppState = &appState->x11;
	const fpl__X11Api *x11Api = &nativeAppState->api;
	const fpl__X11WindowState *nativeWindowState = &windowState->x11;

	fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	fpl__X11VideoOpenGLApi *glApi = &nativeBackend->api;

	Display *display = nativeWindowState->display;
	Window window = nativeWindowState->window;

	//
	// Create legacy context
	//
	GLXContext legacyRenderingContext;
	if (nativeBackend->fbConfig != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Create GLX legacy rendering context on display '%p' and frame buffer config '%p'", display, nativeBackend->fbConfig);
		legacyRenderingContext = glApi->glXCreateNewContext(display, nativeBackend->fbConfig, GLX_RGBA_TYPE, fpl_null, 1);
		if (!legacyRenderingContext) {
			FPL__ERROR(FPL__MODULE_GLX, "Failed creating GLX legacy rendering context on display '%p' and frame buffer config '%p'", display, nativeBackend->fbConfig);
			goto failed_x11_glx;
		}
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully created GLX legacy rendering context '%p' on display '%p' and frame buffer config '%p'", legacyRenderingContext, display, nativeBackend->fbConfig);
	} else if (nativeBackend->visualInfo != fpl_null) {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Create GLX legacy rendering context on display '%p' and visual info '%p'", display, nativeBackend->visualInfo);
		legacyRenderingContext = glApi->glXCreateContext(display, nativeBackend->visualInfo, fpl_null, 1);
		if (!legacyRenderingContext) {
			FPL__ERROR(FPL__MODULE_GLX, "Failed creating GLX legacy rendering context on display '%p' and visual info '%p'", display, nativeBackend->visualInfo);
			goto failed_x11_glx;
		}
	} else {
		FPL__ERROR(FPL__MODULE_GLX, "No visual info or frame buffer config defined!");
		goto failed_x11_glx;
	}

	//
	// Activate legacy context
	//
	FPL_LOG_DEBUG(FPL__MODULE_GLX, "Activate GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, display, (int)window);
	if (!glApi->glXMakeCurrent(display, window, legacyRenderingContext)) {
		FPL__ERROR(FPL__MODULE_GLX, "Failed activating GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, display, (int)window);
		goto failed_x11_glx;
	} else {
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Successfully activated GLX legacy rendering context '%p' on display '%p' and window '%d'", legacyRenderingContext, display, (int)window);
	}

	//
	// Load extensions
	//
	glApi->glXCreateContextAttribsARB = (fpl__func_glx_glXCreateContextAttribsARB *)glApi->glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

	// Disable legacy rendering context
	glApi->glXMakeCurrent(display, 0, fpl_null);

	GLXContext activeRenderingContext;

	if ((videoSettings->graphics.opengl.compabilityFlags != fplOpenGLCompabilityFlags_Legacy) && (nativeBackend->fbConfig != fpl_null)) {
		// @NOTE(final): This is only available in OpenGL 3.0+
		if (!(videoSettings->graphics.opengl.majorVersion >= 3 && videoSettings->graphics.opengl.minorVersion >= 0)) {
			FPL__ERROR(FPL__MODULE_GLX, "You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
			goto failed_x11_glx;
		}

		if (glApi->glXCreateContextAttribsARB == fpl_null) {
			FPL__ERROR(FPL__MODULE_GLX, "glXCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			goto failed_x11_glx;
		}

		int flags = 0;
		int profile = 0;
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Core) {
			profile = FPL__GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Compability) {
			profile = FPL__GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			FPL__ERROR(FPL__MODULE_GLX, "No opengl compability profile selected, please specific Core OpenGLCompabilityFlags_Core or OpenGLCompabilityFlags_Compability");
			goto failed_x11_glx;
		}
		if (videoSettings->graphics.opengl.compabilityFlags & fplOpenGLCompabilityFlags_Forward) {
			flags = FPL__GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		}

		int contextAttribIndex = 0;
		int contextAttribList[32] = fplZeroInit;
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

		GLXContext modernRenderingContext = glApi->glXCreateContextAttribsARB(display, nativeBackend->fbConfig, fpl_null, True, contextAttribList);
		if (!modernRenderingContext) {
			FPL__ERROR(FPL__MODULE_GLX, "Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags);

			// Fallback to legacy rendering context
			glApi->glXMakeCurrent(display, window, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		} else {
			if (!glApi->glXMakeCurrent(display, window, modernRenderingContext)) {
				FPL__ERROR(FPL__MODULE_GLX, "Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and compability flags (%d) -> Fallback to legacy context", videoSettings->graphics.opengl.majorVersion, videoSettings->graphics.opengl.minorVersion, videoSettings->graphics.opengl.compabilityFlags);

				// Destroy modern rendering context
				glApi->glXDestroyContext(display, modernRenderingContext);

				// Fallback to legacy rendering context
				glApi->glXMakeCurrent(display, window, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			} else {
				// Destroy legacy rendering context
				glApi->glXDestroyContext(display, legacyRenderingContext);
				legacyRenderingContext = fpl_null;
				activeRenderingContext = modernRenderingContext;
			}
		}
	} else {
		// Caller wants legacy context
		glApi->glXMakeCurrent(display, window, legacyRenderingContext);
		activeRenderingContext = legacyRenderingContext;
	}

	bool result;

	fplAssert(activeRenderingContext != fpl_null);
	nativeBackend->context = activeRenderingContext;
	nativeBackend->isActiveContext = true;

	backend->surface.window.x11.display = display;
	backend->surface.window.x11.window = window;
	backend->surface.window.x11.visual = nativeWindowState->visual;
	backend->surface.window.x11.screen = nativeWindowState->screen;
	backend->surface.opengl.renderingContext = (void *)activeRenderingContext;

	result = true;

	goto done_x11_glx;

failed_x11_glx:
	result = false;

done_x11_glx:
	if (nativeBackend->visualInfo != fpl_null) {
		// If there is a cached visual info, get rid of it now - regardless of its result
		FPL_LOG_DEBUG(FPL__MODULE_GLX, "Destroy visual info '%p'", nativeBackend->visualInfo);
		x11Api->XFree(nativeBackend->visualInfo);
		nativeBackend->visualInfo = fpl_null;
	}

	if (!result) {
		if (legacyRenderingContext) {
			glApi->glXDestroyContext(display, legacyRenderingContext);
		}
		fpl__VideoBackend_X11OpenGL_Shutdown(appState, windowState, backend);
	}

	return (result);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_X11OpenGL_Unload) {
	fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	fpl__UnloadX11OpenGLApi(&nativeBackend->api);
	fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_X11OpenGL_Load) {
	fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	const fplVideoSettings *videoSettings = &appState->currentSettings.video;
	fplClearStruct(nativeBackend);
	nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;
	if (!fpl__LoadX11OpenGLApi(&nativeBackend->api, videoSettings->graphics.opengl.libraryFile)) {
		return(false);
	}
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_X11OpenGL_Present) {
	const fpl__VideoBackendX11OpenGL *nativeBackend = (fpl__VideoBackendX11OpenGL *)backend;
	const fpl__X11WindowState *x11WinState = &appState->window.x11;
	const fpl__X11VideoOpenGLApi *glApi = &nativeBackend->api;
	glApi->glXSwapBuffers(x11WinState->display, x11WinState->window);
}

fpl_internal fpl__VideoContext fpl__VideoBackend_X11OpenGL_Construct() {
	fpl__VideoContext result = fpl__StubVideoContext();
	result.loadFunc = fpl__VideoBackend_X11OpenGL_Load;
	result.unloadFunc = fpl__VideoBackend_X11OpenGL_Unload;
	result.getProcedureFunc = fpl__VideoBackend_X11OpenGL_GetProcedure;
	result.initializeFunc = fpl__VideoBackend_X11OpenGL_Initialize;
	result.shutdownFunc = fpl__VideoBackend_X11OpenGL_Shutdown;
	result.prepareWindowFunc = fpl__VideoBackend_X11OpenGL_PrepareWindow;
	result.presentFunc = fpl__VideoBackend_X11OpenGL_Present;
	return(result);
}
#endif // FPL__ENABLE_VIDEO_OPENGL && FPL_SUBPLATFORM_X11

// ############################################################################
//
// > VIDEO_BACKEND_SOFTWARE_X11
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_SOFTWARE) && defined(FPL_SUBPLATFORM_X11)
typedef struct fpl__VideoBackendX11Software {
	fpl__VideoBackend base;
	GC graphicsContext;
	XImage *buffer;
} fpl__VideoBackendX11Software;

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_X11Software_Shutdown) {
	const fpl__X11SubplatformState *nativeAppState = &appState->x11;
	const fpl__X11Api *x11Api = &nativeAppState->api;
	const fpl__X11WindowState *nativeWindowState = &windowState->x11;

	fpl__VideoBackendX11Software *nativeBackend = (fpl__VideoBackendX11Software *)backend;

	if (nativeBackend->buffer != fpl_null) {
		// @NOTE(final): Dont use XDestroyImage here, as it points to the backbuffer memory directly - which is released later
		nativeBackend->buffer = fpl_null;
	}

	if (nativeBackend->graphicsContext != fpl_null) {
		nativeBackend->graphicsContext = fpl_null;
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_X11Software_Initialize) {
	const fpl__X11SubplatformState *nativeAppState = &appState->x11;
	const fpl__X11Api *x11Api = &nativeAppState->api;
	const fpl__X11WindowState *nativeWindowState = &windowState->x11;

	fpl__VideoBackendX11Software *nativeBackend = (fpl__VideoBackendX11Software *)backend;

	const fplVideoBackBuffer *backbuffer = &data->backbuffer;

	// Based on: https://bbs.archlinux.org/viewtopic.php?id=225741
	nativeBackend->graphicsContext = x11Api->XCreateGC(nativeWindowState->display, nativeWindowState->window, 0, 0);
	if (nativeBackend->graphicsContext == fpl_null) {
		return false;
	}

	nativeBackend->buffer = x11Api->XCreateImage(nativeWindowState->display, nativeWindowState->visual, 24, ZPixmap, 0, (char *)backbuffer->pixels, backbuffer->width, backbuffer->height, 32, (int)backbuffer->lineWidth);
	if (nativeBackend->buffer == fpl_null) {
		fpl__VideoBackend_X11Software_Shutdown(appState, windowState, backend);
		return false;
	}

	// Initial draw pixels to the window
	x11Api->XPutImage(nativeWindowState->display, nativeWindowState->window, nativeBackend->graphicsContext, nativeBackend->buffer, 0, 0, 0, 0, backbuffer->width, backbuffer->height);
	x11Api->XSync(nativeWindowState->display, False);

	backend->surface.window.x11.display = nativeWindowState->display;
	backend->surface.window.x11.window = nativeWindowState->window;
	backend->surface.window.x11.visual = nativeWindowState->visual;
	backend->surface.window.x11.screen = nativeWindowState->screen;

	return (true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_X11Software_Load) {
	fpl__VideoBackendX11Software *nativeBackend = (fpl__VideoBackendX11Software *)backend;
	fplClearStruct(nativeBackend);
	nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_X11Software_Unload) {
	fpl__VideoBackendX11Software *nativeBackend = (fpl__VideoBackendX11Software *)backend;
	fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_X11Software_Present) {
	const fpl__VideoBackendX11Software *nativeBackend = (fpl__VideoBackendX11Software *)backend;
	const fpl__X11WindowState *x11WinState = &appState->window.x11;
	const fpl__X11Api *x11Api = &appState->x11.api;
	const fplVideoBackBuffer *backbuffer = &data->backbuffer;
	x11Api->XPutImage(x11WinState->display, x11WinState->window, nativeBackend->graphicsContext, nativeBackend->buffer, 0, 0, 0, 0, backbuffer->width, backbuffer->height);
	x11Api->XSync(x11WinState->display, False);
}

fpl_internal fpl__VideoContext fpl__VideoBackend_X11Software_Construct() {
	fpl__VideoContext result = fpl__StubVideoContext();
	result.loadFunc = fpl__VideoBackend_X11Software_Load;
	result.unloadFunc = fpl__VideoBackend_X11Software_Unload;
	result.initializeFunc = fpl__VideoBackend_X11Software_Initialize;
	result.shutdownFunc = fpl__VideoBackend_X11Software_Shutdown;
	result.presentFunc = fpl__VideoBackend_X11Software_Present;
	result.recreateOnResize = true;
	return(result);
}
#endif // FPL__ENABLE_VIDEO_SOFTWARE && FPL_SUBPLATFORM_X11

// ############################################################################
//
// > VIDEO_BACKEND_SOFTWARE_WIN32
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_SOFTWARE) && defined(FPL_PLATFORM_WINDOWS)
typedef struct fpl__VideoBackendWin32Software {
	fpl__VideoBackend base;
	BITMAPINFO bitmapInfo;
} fpl__VideoBackendWin32Software;

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_Win32Software_Shutdown) {
	fpl__VideoBackendWin32Software *nativeBackend = (fpl__VideoBackendWin32Software *)backend;
	BITMAPINFO *bitmapInfo = &nativeBackend->bitmapInfo;
	fplClearStruct(bitmapInfo);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_Win32Software_Initialize) {
	fpl__VideoBackendWin32Software *nativeBackend = (fpl__VideoBackendWin32Software *)backend;
	const fplVideoBackBuffer *backbuffer = &data->backbuffer;
	BITMAPINFO *bitmapInfo = &nativeBackend->bitmapInfo;
	fplClearStruct(bitmapInfo);
	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo->bmiHeader.biWidth = (LONG)backbuffer->width;
	// @NOTE(final): Top-down bitmap requires a negative height
	bitmapInfo->bmiHeader.biHeight = -(LONG)backbuffer->height;
	bitmapInfo->bmiHeader.biBitCount = 32;
	bitmapInfo->bmiHeader.biCompression = BI_RGB;
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biSizeImage = (DWORD)(backbuffer->height * backbuffer->lineWidth);
	return true;
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_Win32Software_Unload) {
	fpl__VideoBackendWin32Software *nativeBackend = (fpl__VideoBackendWin32Software *)backend;
	fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_Win32Software_Load) {
	fpl__VideoBackendWin32Software *nativeBackend = (fpl__VideoBackendWin32Software *)backend;
	fplClearStruct(nativeBackend);
	nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_Win32Software_Present) {
	const fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32WindowState *win32WindowState = &appState->window.win32;
	const fpl__Win32Api *wapi = &win32AppState->winApi;
	const fpl__VideoBackendWin32Software *nativeBackend = (fpl__VideoBackendWin32Software *)backend;
	const fplVideoBackBuffer *backbuffer = &data->backbuffer;
	fplWindowSize area;
	if (fplGetWindowSize(&area)) {
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
		wapi->gdi.StretchDIBits(win32WindowState->deviceContext, targetX, targetY, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, backbuffer->pixels, &nativeBackend->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	}
}

fpl_internal fpl__VideoContext fpl__VideoBackend_Win32Software_Construct() {
	fpl__VideoContext result = fpl__StubVideoContext();
	result.loadFunc = fpl__VideoBackend_Win32Software_Load;
	result.unloadFunc = fpl__VideoBackend_Win32Software_Unload;
	result.initializeFunc = fpl__VideoBackend_Win32Software_Initialize;
	result.shutdownFunc = fpl__VideoBackend_Win32Software_Shutdown;
	result.presentFunc = fpl__VideoBackend_Win32Software_Present;
	result.recreateOnResize = true;
	return(result);
}
#endif // FPL__ENABLE_VIDEO_SOFTWARE && FPL_PLATFORM_WINDOWS

// ############################################################################
//
// > VIDEO_BACKEND_VULKAN (Win32, X11)
//
// ############################################################################
#if defined(FPL__ENABLE_VIDEO_VULKAN)

#if !fplHasInclude(<vulkan/vulkan.h>) || defined(FPL_NO_PLATFORM_INCLUDES)

#if defined(FPL_PLATFORM_WINDOWS)
#	define fpl__VKAPI_CALL __stdcall
#	define fpl__VKAPI_PTR fpl__VKAPI_CALL
#	define fpl__VKAPI_ATTR
#else
#	define fpl__VKAPI_CALL
#	define fpl__VKAPI_PTR fpl__VKAPI_CALL
#	define fpl__VKAPI_ATTR
#endif

#define FPL__VK_NULL_HANDLE fpl_null

#define FPL__VK_MAX_EXTENSION_NAME_SIZE 256
#define FPL__VK_MAX_DESCRIPTION_SIZE 256

#define FPL__VK_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

typedef enum fpl__VkResult {
	FPL__VK_ERROR_OUT_OF_HOST_MEMORY = -1,
	FPL__VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
	FPL__VK_ERROR_INITIALIZATION_FAILED = -3,
	FPL__VK_ERROR_DEVICE_LOST = -4,
	FPL__VK_ERROR_MEMORY_MAP_FAILED = -5,
	FPL__VK_ERROR_LAYER_NOT_PRESENT = -6,
	FPL__VK_ERROR_EXTENSION_NOT_PRESENT = -7,
	FPL__VK_ERROR_FEATURE_NOT_PRESENT = -8,
	FPL__VK_ERROR_INCOMPATIBLE_DRIVER = -9,
	FPL__VK_ERROR_TOO_MANY_OBJECTS = -10,
	FPL__VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
	FPL__VK_ERROR_FRAGMENTED_POOL = -12,
	FPL__VK_ERROR_UNKNOWN = -13,

	FPL__VK_SUCCESS = 0,

	FPL__VK_NOT_READY = 1,
	FPL__VK_TIMEOUT = 2,
	FPL__VK_EVENT_SET = 3,
	FPL__VK_EVENT_RESET = 4,
	FPL__VK_INCOMPLETE = 5,	

	FPL__VK_RESULT_MAX_ENUM = 0x7FFFFFFF
} fpl__VkResult;

typedef uint32_t fpl__VkFlags;
typedef uint32_t fpl__VkBool32;

typedef void fpl__VkAllocationCallbacks;

typedef void *fpl__VkInstance;
typedef void *fpl__VkSurfaceKHR;
typedef void *fpl__VkPhysicalDevice;

typedef enum fpl__VkStructureType {
	FPL__VK_STRUCTURE_TYPE_APPLICATION_INFO = 0,
	FPL__VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1,
	FPL__VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR = 1000004000,
	FPL__VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
	FPL__VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT = 1000128002,
	FPL__VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT = 1000128004,
	FPL__VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} fpl__VkStructureType;

typedef struct fpl__VkApplicationInfo {
	fpl__VkStructureType sType;
	const void *pNext;
	const char *pApplicationName;
	uint32_t applicationVersion;
	const char *pEngineName;
	uint32_t engineVersion;
	uint32_t apiVersion;
} fpl__VkApplicationInfo;

typedef fpl__VkFlags fpl__VkInstanceCreateFlags;

typedef struct fpl__VkInstanceCreateInfo {
	fpl__VkStructureType sType;
	const void *pNext;
	fpl__VkInstanceCreateFlags flags;
	const fpl__VkApplicationInfo *pApplicationInfo;
	uint32_t enabledLayerCount;
	const char *const *ppEnabledLayerNames;
	uint32_t enabledExtensionCount;
	const char *const *ppEnabledExtensionNames;
} fpl__VkInstanceCreateInfo;

typedef struct fpl__VkExtensionProperties {
	char extensionName[FPL__VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t specVersion;
} fpl__VkExtensionProperties;

typedef struct fpl__VkLayerProperties {
	char layerName[FPL__VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t specVersion;
	uint32_t implementationVersion;
	char description[FPL__VK_MAX_DESCRIPTION_SIZE];
} fpl__VkLayerProperties;

// Core procs (opaque)
typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkCreateInstance)(const fpl__VkInstanceCreateInfo *pCreateInfo, const fpl__VkAllocationCallbacks *pAllocator, fpl__VkInstance *pInstance);
typedef void (fpl__VKAPI_PTR *fpl__func_vkDestroyInstance)(fpl__VkInstance instance, const fpl__VkAllocationCallbacks *pAllocator);
typedef void *(fpl__VKAPI_PTR *fpl__func_vkGetInstanceProcAddr)(fpl__VkInstance instance, const char *pName);
typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkEnumerateInstanceExtensionProperties)(const char *pLayerName, uint32_t *pPropertyCount, fpl__VkExtensionProperties *pProperties);
typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkEnumerateInstanceLayerProperties)(uint32_t *pPropertyCount, fpl__VkLayerProperties *pProperties);

// Surface KHR procs (opaque)
typedef void (fpl__VKAPI_PTR *fpl__func_vkDestroySurfaceKHR)(fpl__VkInstance instance, fpl__VkSurfaceKHR surface, const fpl__VkAllocationCallbacks *pAllocator);

#if defined(FPL_PLATFORM_WINDOWS)

typedef fpl__VkFlags fpl__VkWin32SurfaceCreateFlagsKHR;
typedef struct fpl__VkWin32SurfaceCreateInfoKHR {
	fpl__VkStructureType sType;
	const void *pNext;
	fpl__VkWin32SurfaceCreateFlagsKHR flags;
	fpl__Win32InstanceHandle hinstance;
	fpl__Win32WindowHandle hwnd;
} fpl__VkWin32SurfaceCreateInfoKHR;

typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkCreateWin32SurfaceKHR)(fpl__VkInstance instance, const fpl__VkWin32SurfaceCreateInfoKHR *pCreateInfo, const fpl__VkAllocationCallbacks *pAllocator, fpl__VkSurfaceKHR *pSurface);
typedef fpl__VkBool32(fpl__VKAPI_PTR *fpl__func_vkGetPhysicalDeviceWin32PresentationSupportKHR)(fpl__VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

#elif defined(FPL_SUBPLATFORM_X11)

typedef fpl__VkFlags fpl__VkXlibSurfaceCreateFlagsKHR;
typedef struct fpl__VkXlibSurfaceCreateInfoKHR {
	fpl__VkStructureType sType;
	const void *pNext;
	fpl__VkXlibSurfaceCreateFlagsKHR flags;
	fpl__X11Display *dpy;
	fpl__X11Window window;
} fpl__VkXlibSurfaceCreateInfoKHR;

typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkCreateXlibSurfaceKHR)(fpl__VkInstance instance, const fpl__VkXlibSurfaceCreateInfoKHR *pCreateInfo, const fpl__VkAllocationCallbacks *pAllocator, fpl__VkSurfaceKHR *pSurface);
typedef fpl__VkBool32(fpl__VKAPI_PTR *fpl__func_vkGetPhysicalDeviceXlibPresentationSupportKHR)(fpl__VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

#endif

typedef void *fpl__VkDebugUtilsMessengerEXT;

typedef enum fpl__VkDebugUtilsMessageSeverityFlagBitsEXT {
	FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
	FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
	FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
	FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
	FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} fpl__VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef fpl__VkFlags fpl__VkDebugUtilsMessageSeverityFlagsEXT;

typedef enum fpl__VkDebugUtilsMessageTypeFlagBitsEXT {
	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} fpl__VkDebugUtilsMessageTypeFlagBitsEXT;
typedef fpl__VkFlags fpl__VkDebugUtilsMessageTypeFlagsEXT;

typedef struct fpl__VkDebugUtilsLabelEXT {
	fpl__VkStructureType sType;
	const void *pNext;
	const char *pLabelName;
	float color[4];
} fpl__VkDebugUtilsLabelEXT;

typedef void fpl__VkDebugUtilsObjectNameInfoEXT;
typedef fpl__VkFlags fpl__VkDebugUtilsMessengerCallbackDataFlagsEXT;
typedef struct fpl__VkDebugUtilsMessengerCallbackDataEXT {
	fpl__VkStructureType sType;
	const void *pNext;
	fpl__VkDebugUtilsMessengerCallbackDataFlagsEXT flags;
	const char *pMessageIdName;
	int32_t messageIdNumber;
	const char *pMessage;
	uint32_t queueLabelCount;
	const fpl__VkDebugUtilsLabelEXT *pQueueLabels;
	uint32_t cmdBufLabelCount;
	const fpl__VkDebugUtilsLabelEXT *pCmdBufLabels;
	uint32_t objectCount;
	const fpl__VkDebugUtilsObjectNameInfoEXT *pObjects;
} fpl__VkDebugUtilsMessengerCallbackDataEXT;

typedef fpl__VkFlags fpl__VkDebugUtilsMessengerCreateFlagsEXT;

typedef fpl__VkBool32(fpl__VKAPI_PTR *fpl__func_vkDebugUtilsMessengerCallbackEXT) (
	fpl__VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	fpl__VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const fpl__VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData);

typedef struct fpl__VkDebugUtilsMessengerCreateInfoEXT {
	fpl__VkStructureType sType;
	const void *pNext;
	fpl__VkDebugUtilsMessengerCreateFlagsEXT flags;
	fpl__VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
	fpl__VkDebugUtilsMessageTypeFlagsEXT messageType;
	fpl__func_vkDebugUtilsMessengerCallbackEXT pfnUserCallback;
	void *pUserData;
} fpl__VkDebugUtilsMessengerCreateInfoEXT;

typedef fpl__VkResult(fpl__VKAPI_PTR *fpl__func_vkCreateDebugUtilsMessengerEXT)(fpl__VkInstance instance, const fpl__VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const fpl__VkAllocationCallbacks *pAllocator, fpl__VkDebugUtilsMessengerEXT *pMessenger);
typedef void (fpl__VKAPI_PTR *fpl__func_vkDestroyDebugUtilsMessengerEXT)(fpl__VkInstance instance, fpl__VkDebugUtilsMessengerEXT messenger, const fpl__VkAllocationCallbacks *pAllocator);

#else

#	if defined(FPL_PLATFORM_WINDOWS)
#		define VK_USE_PLATFORM_WIN32_KHR
#	elif defined(FPL_SUBPLATFORM_X11)
#		define VK_USE_PLATFORM_XLIB_KHR
#	endif

#	if !defined(FPL_NO_RUNTIME_LINKING)
#		define VK_NO_PROTOTYPES
#	endif
#	include <vulkan/vulkan.h>

#	define fpl__VKAPI_CALL VKAPI_CALL
#	define fpl__VKAPI_PTR VKAPI_PTR
#	define fpl__VKAPI_ATTR VKAPI_ATTR

#	define FPL__VK_STRUCTURE_TYPE_APPLICATION_INFO VK_STRUCTURE_TYPE_APPLICATION_INFO
#	define FPL__VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
#	define FPL__VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR
#	define FPL__VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
#	define FPL__VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT

typedef enum fpl__VkResult {
	FPL__VK_ERROR_OUT_OF_HOST_MEMORY = VK_ERROR_OUT_OF_HOST_MEMORY,
	FPL__VK_ERROR_OUT_OF_DEVICE_MEMORY = VK_ERROR_OUT_OF_DEVICE_MEMORY,
	FPL__VK_ERROR_INITIALIZATION_FAILED = VK_ERROR_INITIALIZATION_FAILED,
	FPL__VK_ERROR_DEVICE_LOST = VK_ERROR_DEVICE_LOST,
	FPL__VK_ERROR_MEMORY_MAP_FAILED = VK_ERROR_MEMORY_MAP_FAILED,
	FPL__VK_ERROR_LAYER_NOT_PRESENT = VK_ERROR_LAYER_NOT_PRESENT,
	FPL__VK_ERROR_EXTENSION_NOT_PRESENT = VK_ERROR_EXTENSION_NOT_PRESENT,
	FPL__VK_ERROR_FEATURE_NOT_PRESENT = VK_ERROR_FEATURE_NOT_PRESENT,
	FPL__VK_ERROR_INCOMPATIBLE_DRIVER = VK_ERROR_INCOMPATIBLE_DRIVER,
	FPL__VK_ERROR_TOO_MANY_OBJECTS = VK_ERROR_TOO_MANY_OBJECTS,
	FPL__VK_ERROR_FORMAT_NOT_SUPPORTED = VK_ERROR_FORMAT_NOT_SUPPORTED,
	FPL__VK_ERROR_FRAGMENTED_POOL = VK_ERROR_FRAGMENTED_POOL,
	FPL__VK_ERROR_UNKNOWN = VK_ERROR_UNKNOWN,

	FPL__VK_SUCCESS = VK_SUCCESS,

	FPL__VK_NOT_READY = VK_NOT_READY,
	FPL__VK_TIMEOUT = VK_TIMEOUT,
	FPL__VK_EVENT_SET = VK_EVENT_SET,
	FPL__VK_EVENT_RESET = VK_EVENT_RESET,
	FPL__VK_INCOMPLETE = VK_INCOMPLETE,	

	FPL__VK_RESULT_MAX_ENUM = VK_RESULT_MAX_ENUM
} fpl__VkResult;

#	define FPL__VK_NULL_HANDLE VK_NULL_HANDLE

#	define FPL__VK_MAKE_VERSION(major, minor, patch) VK_MAKE_VERSION(major, minor, patch)

typedef VkFlags fpl__VkFlags;
typedef VkBool32 fpl__VkBool32;

typedef VkAllocationCallbacks fpl__VkAllocationCallbacks;

typedef VkInstance fpl__VkInstance;
typedef VkSurfaceKHR fpl__VkSurfaceKHR;
typedef VkPhysicalDevice fpl__VkPhysicalDevice;

typedef VkApplicationInfo fpl__VkApplicationInfo;
typedef VkInstanceCreateInfo fpl__VkInstanceCreateInfo;

typedef VkExtensionProperties fpl__VkExtensionProperties;
typedef VkLayerProperties fpl__VkLayerProperties;

// Core procs
typedef PFN_vkCreateInstance fpl__func_vkCreateInstance;
typedef PFN_vkDestroyInstance fpl__func_vkDestroyInstance;
typedef PFN_vkGetInstanceProcAddr fpl__func_vkGetInstanceProcAddr;
typedef PFN_vkEnumerateInstanceExtensionProperties fpl__func_vkEnumerateInstanceExtensionProperties;
typedef PFN_vkEnumerateInstanceLayerProperties fpl__func_vkEnumerateInstanceLayerProperties;

// Instance procs
typedef PFN_vkDestroySurfaceKHR fpl__func_vkDestroySurfaceKHR;
#if defined(FPL_PLATFORM_WINDOWS)
typedef VkWin32SurfaceCreateInfoKHR fpl__VkWin32SurfaceCreateInfoKHR;
typedef PFN_vkCreateWin32SurfaceKHR fpl__func_vkCreateWin32SurfaceKHR;
typedef PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR fpl__func_vkGetPhysicalDeviceWin32PresentationSupportKHR;
#elif defined(FPL_SUBPLATFORM_X11)
typedef VkXlibSurfaceCreateInfoKHR fpl__VkXlibSurfaceCreateInfoKHR;
typedef PFN_vkCreateXlibSurfaceKHR fpl__func_vkCreateXlibSurfaceKHR;
typedef PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR fpl__func_vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif

typedef VkDebugUtilsMessageSeverityFlagBitsEXT fpl__VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef VkDebugUtilsMessageSeverityFlagsEXT fpl__VkDebugUtilsMessageSeverityFlagsEXT;
#	define FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
#	define FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
#	define FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
#	define FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#	define FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT
typedef VkDebugUtilsMessageTypeFlagBitsEXT fpl__VkDebugUtilsMessageTypeFlagBitsEXT;
typedef VkDebugUtilsMessageTypeFlagsEXT fpl__VkDebugUtilsMessageTypeFlagsEXT;
#	define	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
#	define	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
#	define	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
#	define	FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT
typedef VkDebugUtilsMessengerCallbackDataEXT fpl__VkDebugUtilsMessengerCallbackDataEXT;
typedef VkDebugUtilsMessengerEXT fpl__VkDebugUtilsMessengerEXT;
typedef PFN_vkDebugUtilsMessengerCallbackEXT fpl__func_vkDebugUtilsMessengerCallbackEXT;
typedef VkDebugUtilsMessengerCreateInfoEXT fpl__VkDebugUtilsMessengerCreateInfoEXT;

typedef PFN_vkCreateDebugUtilsMessengerEXT fpl__func_vkCreateDebugUtilsMessengerEXT;
typedef PFN_vkDestroyDebugUtilsMessengerEXT fpl__func_vkDestroyDebugUtilsMessengerEXT;

#endif // !fplHasInclude(<vulkan/vulkan.h>) || defined(FPL_NO_PLATFORM_INCLUDES)

typedef struct fpl__VulkanApi {
	fplDynamicLibraryHandle libraryHandle;
	fpl__func_vkCreateInstance vkCreateInstance;
	fpl__func_vkDestroyInstance vkDestroyInstance;
	fpl__func_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	fpl__func_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	fpl__func_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
} fpl__VulkanApi;

fpl_internal void fpl__UnloadVulkanApi(fpl__VulkanApi *api) {
	if (api->libraryHandle.isValid) {
		fplDynamicLibraryUnload(&api->libraryHandle);
	}
	fplClearStruct(api);
}

fpl_internal bool fpl__LoadVulkanApi(fpl__VulkanApi *api, const char *libraryName) {

	fplClearStruct(api);

#if defined(FPL_NO_RUNTIME_LINKING)
	api->vkCreateInstance = vkCreateInstance;
	api->vkDestroyInstance = vkDestroyInstance;
	api->vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	api->vkEnumerateInstanceExtensionProperties = vkEnumerateInstanceExtensionProperties;
	api->vkEnumerateInstanceLayerProperties = vkEnumerateInstanceLayerProperties;
	return(true);
#endif

	uint32_t libraryCount = 0;

	const char *libraryNames[4];
	if (fplGetStringLength(libraryName) > 0) {
		libraryNames[libraryCount++] = libraryName;
	} else {
		// Automatic detection of vulkan library
#if defined(FPL_PLATFORM_WINDOWS)
		libraryNames[libraryCount++] = "vulkan-1.dll";
#elif defined(FPL_SUBPLATFORM_POSIX)
		libraryNames[libraryCount++] = "libvulkan.so";
		libraryNames[libraryCount++] = "libvulkan.so.1";
#else
		FPL__WARNING(FPL__MODULE_VIDEO_VULKAN, "Unsupported Platform!");
		return(false);
#endif
	}

#define FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libName, target, type, name) \
		(target)->name = (type)fplGetDynamicLibraryProc(&libHandle, #name); \
		if ((target)->name == fpl_null) { \
			FPL__WARNING(FPL__MODULE_VIDEO_VULKAN, "Failed getting procedure address '%s' from library '%s'", #name, libName); \
			continue; \
		}

	bool result = false;
	for (uint32_t i = 0; i < libraryCount; ++i) {
		const char *libraryName = libraryNames[i];

		if (api->libraryHandle.isValid) {
			fplDynamicLibraryUnload(&api->libraryHandle);
		}
		fplClearStruct(api);

		fplDynamicLibraryHandle libHandle = fplZeroInit;
		if (!fplDynamicLibraryLoad(libraryName, &libHandle)) {
			continue;
		}
		api->libraryHandle = libHandle;

		FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libraryName, api, fpl__func_vkCreateInstance, vkCreateInstance);
		FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libraryName, api, fpl__func_vkDestroyInstance, vkDestroyInstance);
		FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libraryName, api, fpl__func_vkGetInstanceProcAddr, vkGetInstanceProcAddr);
		FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libraryName, api, fpl__func_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
		FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE(libHandle, libraryName, api, fpl__func_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);

		result = true;
		break;
	}

	if (!result) {
		fpl__UnloadVulkanApi(api);
	}

#undef FPL__VULKAN_GET_FUNCTION_ADDRESS_CONTINUE

	return(result);
}

typedef struct fpl__VulkanDebugMessengerUserData {
	fplVulkanValidationLayerCallback *userCallback;
	fplVulkanValidationLayerMode validationMode;
	void *userData;
} fpl__VulkanDebugMessengerUserData;

typedef struct fpl__VideoBackendVulkan {
	fpl__VideoBackend base;
	fpl__VulkanApi api;
	fpl__VkInstance instanceHandle;
	fpl__VkSurfaceKHR surfaceHandle;
	fpl__VkDebugUtilsMessengerEXT debugMessenger;
	fpl__VulkanDebugMessengerUserData debugMessengerUserData;
	const fpl__VkAllocationCallbacks *allocator;
	fpl_b32 isInstanceUserDefined;
} fpl__VideoBackendVulkan;

fpl_internal const char *fpl__GetVulkanResultString(const fpl__VkResult result) {
	switch (result) {
		case FPL__VK_ERROR_OUT_OF_HOST_MEMORY:
			return "Out of Host-Memory";
		case FPL__VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "Out of Device-Memory";
		case FPL__VK_ERROR_INITIALIZATION_FAILED:
			return "Initialization failed";
		case FPL__VK_ERROR_DEVICE_LOST:
			return "Device lost";
		case FPL__VK_ERROR_MEMORY_MAP_FAILED:
			return "Memory map failed";
		case FPL__VK_ERROR_LAYER_NOT_PRESENT:
			return "Layer not present";
		case FPL__VK_ERROR_EXTENSION_NOT_PRESENT:
			return "Extension not present";
		case FPL__VK_ERROR_FEATURE_NOT_PRESENT:
			return "Feature not present";
		case FPL__VK_ERROR_INCOMPATIBLE_DRIVER:
			return "Incompatible driver";
		case FPL__VK_ERROR_TOO_MANY_OBJECTS:
			return "Too many objects";
		case FPL__VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "Format not supported";
		case FPL__VK_ERROR_FRAGMENTED_POOL:
			return "Fragmented pool";
		case FPL__VK_SUCCESS:
			return "Success";
		case FPL__VK_NOT_READY:
			return "Not-Ready";
		case FPL__VK_TIMEOUT:
			return "Timeout";
		case FPL__VK_EVENT_SET:
			return "Event-Set";
		case FPL__VK_EVENT_RESET:
			return "Event-Reset";
		case FPL__VK_INCOMPLETE:
			return "Incomplete";
		default:
			return "Unknown";
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_GETPROCEDURE(fpl__VideoBackend_Vulkan_GetProcedure) {
	const fpl__VideoBackendVulkan *nativeBackend = (const fpl__VideoBackendVulkan *)backend;
	const fpl__VulkanApi *api = &nativeBackend->api;
	if (api->libraryHandle.isValid) {
		return fplGetDynamicLibraryProc(&api->libraryHandle, procName);
	}
	return(fpl_null);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_GETREQUIREMENTS(fpl__VideoBackend_Vulkan_GetRequirements) {
	FPL__CheckArgumentNull(requirements, false);
	fplClearStruct(requirements);

	fplAssert(requirements->vulkan.instanceExtensionCount < fplArrayCount(requirements->vulkan.instanceExtensions));
	requirements->vulkan.instanceExtensions[requirements->vulkan.instanceExtensionCount++] = "VK_KHR_surface";
#if defined(FPL_PLATFORM_WINDOWS)
	fplAssert(requirements->vulkan.instanceExtensionCount < fplArrayCount(requirements->vulkan.instanceExtensions));
	requirements->vulkan.instanceExtensions[requirements->vulkan.instanceExtensionCount++] = "VK_KHR_win32_surface";
#elif defined(FPL_SUBPLATFORM_X11)
	fplAssert(requirements->vulkan.instanceExtensionCount < fplArrayCount(requirements->vulkan.instanceExtensions));
	requirements->vulkan.instanceExtensions[requirements->vulkan.instanceExtensionCount++] = "VK_KHR_xlib_surface";
#else
	return(false);
#endif
	return(true);
}

fpl_internal uint32_t fpl__VersionInfoToVulkanVersion(const fplVersionInfo *versionInfo) {
	uint32_t major = fplStringToS32(versionInfo->major);
	uint32_t minor = fplStringToS32(versionInfo->minor);
	uint32_t patch = fplStringToS32(versionInfo->fix);
	uint32_t result = FPL__VK_MAKE_VERSION(major, minor, patch);
	return(result);
}

fpl_internal const char *fpl__GetVulkanMessageSeverityName(const fpl__VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
	switch (messageSeverity) {
		case FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return "ERROR";
		case FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "WARNING";
		case FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return "INFO";
		case FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "VERBOSE";
		default: return "Unknown";
	}
}

fpl_internal fpl__VKAPI_ATTR fpl__VkBool32 fpl__VKAPI_CALL fpl__VulkanDebugCallback(fpl__VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, fpl__VkDebugUtilsMessageTypeFlagsEXT messageType, const fpl__VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
	fpl__VulkanDebugMessengerUserData *data = (fpl__VulkanDebugMessengerUserData *)pUserData;
	const char *message = pCallbackData->pMessage;
	if (data->userCallback != fpl_null) {
		data->userCallback(data->userData, message, messageSeverity, messageType, pCallbackData);
	} else {
		if (messageSeverity == FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			FPL_LOG_ERROR(FPL__MODULE_VIDEO_VULKAN, "Validation: %s", message);
		else if (messageSeverity == FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			FPL_LOG_WARN(FPL__MODULE_VIDEO_VULKAN, "Validation: %s", message);
		else if (messageSeverity == FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Validation: %s", message);
		else if (messageSeverity == FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			FPL_LOG_DEBUG(FPL__MODULE_VIDEO_VULKAN, "Validation: %s", message);
		else {
			FPL_LOG_DEBUG(FPL__MODULE_VIDEO_VULKAN, "Validation: %s", message);
		}
	}
	return 0;
}

fpl_internal void fpl__VulkanDestroyDebugMessenger(fpl__VideoBackendVulkan *nativeBackend) {
	if (nativeBackend->debugMessenger != fpl_null) {
		FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Destroy Vulkan Debug Messenger '%p'", nativeBackend->debugMessenger);
		const fpl__VulkanApi *api = &nativeBackend->api;
		fpl__func_vkDestroyDebugUtilsMessengerEXT destroyFunc = (fpl__func_vkDestroyDebugUtilsMessengerEXT)api->vkGetInstanceProcAddr(nativeBackend->instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
		fplAssert(destroyFunc != fpl_null);
		destroyFunc(nativeBackend->instanceHandle, nativeBackend->debugMessenger, nativeBackend->allocator);
		nativeBackend->debugMessenger = fpl_null;
	}
	fplClearStruct(&nativeBackend->debugMessengerUserData);
}

fpl_internal bool fpl__VulkanCreateDebugMessenger(const fplVulkanSettings *settings, fpl__VideoBackendVulkan *nativeBackend) {
	fpl__VkDebugUtilsMessageSeverityFlagsEXT severities = 0;
	switch (settings->validationSeverity) {
		case fplVulkanValidationSeverity_Error:
			severities = FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			break;
		case fplVulkanValidationSeverity_Warning:
			severities = FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			break;
		case fplVulkanValidationSeverity_Info:
			severities = FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
			break;
		case fplVulkanValidationSeverity_Verbose:
		case fplVulkanValidationSeverity_All:
			severities = FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | FPL__VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
			break;
		case fplVulkanValidationSeverity_Off:
		default:
			return(false);
	}

	fpl__VulkanDebugMessengerUserData *userData = &nativeBackend->debugMessengerUserData;
	userData->userCallback = settings->validationLayerCallback;
	userData->userData = settings->userData;
	userData->validationMode = settings->validationLayerMode;

	// @TODO(final): Message type filtering in fplVulkanSettings
	fpl__VkDebugUtilsMessageTypeFlagsEXT messageTypes =
		FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		FPL__VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	fpl__VkDebugUtilsMessengerCreateInfoEXT createInfo = fplZeroInit;
	createInfo.sType = FPL__VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = severities;
	createInfo.messageType = messageTypes;
	createInfo.pfnUserCallback = fpl__VulkanDebugCallback;
	createInfo.pUserData = userData;

	const fpl__VulkanApi *api = &nativeBackend->api;

	fpl__func_vkCreateDebugUtilsMessengerEXT createFunc = (fpl__func_vkCreateDebugUtilsMessengerEXT)api->vkGetInstanceProcAddr(nativeBackend->instanceHandle, "vkCreateDebugUtilsMessengerEXT");
	if (createFunc == fpl_null) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Vulkan instance proc 'vkCreateDebugUtilsMessengerEXT' not found! Maybe the instance extension 'VK_EXT_debug_utils' was not set?");
		return(false);
	}

	FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Create Vulkan Debug Messenger for Instance '%p' with severity flags of '%lu'", nativeBackend->instanceHandle, severities);
	fpl__VkResult creationResult = (fpl__VkResult)createFunc(nativeBackend->instanceHandle, &createInfo, nativeBackend->allocator, &nativeBackend->debugMessenger);
	if (creationResult != FPL__VK_SUCCESS) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Failed creating Vulkan Debug Messenger for Instance '%p' with severity flags of '%lu' -> (VkResult: %d)!", nativeBackend->instanceHandle, severities, creationResult);
		return(false);
	}

	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PREPAREWINDOW(fpl__VideoBackend_Vulkan_PrepareWindow) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;
	if (videoSettings->graphics.vulkan.instanceHandle == fpl_null) {

		const fpl__VulkanApi *api = &nativeBackend->api;

		nativeBackend->allocator = fpl_null;
		nativeBackend->instanceHandle = fpl_null;
		nativeBackend->isInstanceUserDefined = false;

		fplVideoRequirements requirements = fplZeroInit;
		if (!fpl__VideoBackend_Vulkan_GetRequirements(&requirements) || requirements.vulkan.instanceExtensionCount == 0) {
			FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Failed getting required instance extensions for creating an Vulkan instance!");
			return(false);
		}

		// @TODO(final): Validate vulkan video settings

		//
		// Validate required instance extensions and layers
		//
		bool supportsDebugUtils = false;

		fpl__VkExtensionProperties *supportedInstanceExtensions = fpl_null;
		uint32_t supportedInstanceExtensionCount = 0;
		api->vkEnumerateInstanceExtensionProperties(fpl_null, &supportedInstanceExtensionCount, fpl_null);
		if (supportedInstanceExtensionCount > 0) {
			supportedInstanceExtensions = (fpl__VkExtensionProperties *)fpl__AllocateTemporaryMemory(sizeof(fpl__VkExtensionProperties) * supportedInstanceExtensionCount, 16);
			api->vkEnumerateInstanceExtensionProperties(fpl_null, &supportedInstanceExtensionCount, supportedInstanceExtensions);
			for (uint32_t i = 0; i < supportedInstanceExtensionCount; ++i) {
				if (fplIsStringEqual(supportedInstanceExtensions[i].extensionName, "VK_EXT_debug_utils")) {
					supportsDebugUtils = true;
				}
			}
			fpl__ReleaseTemporaryMemory(supportedInstanceExtensions);
		}

		bool supportsValidationLayer = false;

		fpl__VkLayerProperties *supportedLayers = fpl_null;
		uint32_t supportedLayerCount = 0;
		api->vkEnumerateInstanceLayerProperties(&supportedLayerCount, fpl_null);
		if (supportedLayerCount > 0) {
			supportedLayers = (fpl__VkLayerProperties *)fpl__AllocateTemporaryMemory(sizeof(fpl__VkLayerProperties) * supportedLayerCount, 16);
			api->vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers);
			for (uint32_t i = 0; i < supportedLayerCount; ++i) {
				if (fplIsStringEqual(supportedLayers[i].layerName, "VK_LAYER_KHRONOS_validation")) {
					supportsValidationLayer = true;
				}
			}
			fpl__ReleaseTemporaryMemory(supportedInstanceExtensions);
		}

		const char *enabledValidationLayers[4] = fplZeroInit;
		const char *enabledInstanceExtensions[8] = fplZeroInit;
		uint32_t enabledValidationLayerCount = 0;
		uint32_t enabledInstanceExtensionCount = 0;
		if (videoSettings->graphics.vulkan.validationLayerMode != fplVulkanValidationLayerMode_Disabled) {
			if (videoSettings->graphics.vulkan.validationLayerMode == fplVulkanValidationLayerMode_Required) {
				if (!supportsDebugUtils) {
					FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "VK_EXT_debug_utils instance extension is not supported!");
					return(false);
				}
				if (!supportsValidationLayer) {
					FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "VK_LAYER_KHRONOS_validation instance layer is not supported!");
					return(false);
				}
			}
			if (supportsDebugUtils && supportsValidationLayer) {
				enabledValidationLayers[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
				enabledInstanceExtensions[enabledInstanceExtensionCount++] = "VK_EXT_debug_utils";
			}
		}
		for (uint32_t i = 0; i < requirements.vulkan.instanceExtensionCount; ++i) {
			fplAssert(enabledInstanceExtensionCount < fplArrayCount(enabledInstanceExtensions));
			enabledInstanceExtensions[enabledInstanceExtensionCount++] = requirements.vulkan.instanceExtensions[i];
		}

		fpl__VkApplicationInfo applicationInfo = fplZeroInit;
		applicationInfo.sType = FPL__VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pApplicationName = videoSettings->graphics.vulkan.appName;
		applicationInfo.pEngineName = videoSettings->graphics.vulkan.engineName;
		applicationInfo.apiVersion = fpl__VersionInfoToVulkanVersion(&videoSettings->graphics.vulkan.apiVersion);
		applicationInfo.engineVersion = fpl__VersionInfoToVulkanVersion(&videoSettings->graphics.vulkan.engineVersion);
		applicationInfo.applicationVersion = fpl__VersionInfoToVulkanVersion(&videoSettings->graphics.vulkan.appVersion);

		fpl__VkInstanceCreateInfo instanceCreateInfo = fplZeroInit;
		instanceCreateInfo.sType = FPL__VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
		instanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		instanceCreateInfo.ppEnabledLayerNames = enabledValidationLayers;

		const fpl__VkAllocationCallbacks *allocator = (const fpl__VkAllocationCallbacks *)videoSettings->graphics.vulkan.allocator;

		FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Create Vulkan Instance with %lu extensions and %lu layers", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount);
		fpl__VkInstance instance = fpl_null;
		fpl__VkResult creationResult = (fpl__VkResult)api->vkCreateInstance(&instanceCreateInfo, allocator, &instance);
		if (creationResult != FPL__VK_SUCCESS) {
			const char *creationError = fpl__GetVulkanResultString(creationResult);
			FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Failed creating Vulkan Instance with %lu extensions and %lu layers -> (VkResult: %d, Error: %s)!", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount, creationResult, creationError);
			return(false);
		}

		nativeBackend->allocator = allocator;
		nativeBackend->instanceHandle = instance;
		nativeBackend->isInstanceUserDefined = false;

		// Debug utils
		if (videoSettings->graphics.vulkan.validationLayerMode != fplVulkanValidationLayerMode_Disabled) {
			if (!fpl__VulkanCreateDebugMessenger(&videoSettings->graphics.vulkan, nativeBackend)) {
				if (videoSettings->graphics.vulkan.validationLayerMode == fplVulkanValidationLayerMode_Optional) {
					FPL__WARNING(FPL__MODULE_VIDEO_VULKAN, "The debug messenger could not be created, no validation message are printed");
				} else {
					FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "The debug messenger could not be created, stop vulkan initialization!");
					return(false);
				}
			}
		}

		return(true);
	} else {
		// Instance passed by user
		nativeBackend->allocator = (const fpl__VkAllocationCallbacks *)videoSettings->graphics.vulkan.allocator;
		nativeBackend->instanceHandle = (fpl__VkInstance)videoSettings->graphics.vulkan.instanceHandle;
		nativeBackend->isInstanceUserDefined = true;
		return(true);
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_FINALIZEWINDOW(fpl__VideoBackend_Vulkan_FinalizeWindow) {
	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_SHUTDOWN(fpl__VideoBackend_Vulkan_Shutdown) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;

	const fpl__VulkanApi *api = &nativeBackend->api;

	if (nativeBackend->surfaceHandle != FPL__VK_NULL_HANDLE) {
		fpl__func_vkDestroySurfaceKHR destroyProc = (fpl__func_vkDestroySurfaceKHR)api->vkGetInstanceProcAddr(nativeBackend->instanceHandle, "vkDestroySurfaceKHR");
		fplAssert(destroyProc != fpl_null);

		FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Destroy Vulkan Surface '%p'", nativeBackend->surfaceHandle);
		destroyProc(nativeBackend->instanceHandle, nativeBackend->surfaceHandle, nativeBackend->allocator);

		nativeBackend->surfaceHandle = FPL__VK_NULL_HANDLE;
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_INITIALIZE(fpl__VideoBackend_Vulkan_Initialize) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;

	const fpl__VulkanApi *api = &nativeBackend->api;

	if (nativeBackend->instanceHandle == FPL__VK_NULL_HANDLE) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Cannot create a Vulkan surface without a Vulkan instance!");
		return(false);
	}

	fpl__VkSurfaceKHR surfaceHandle = FPL__VK_NULL_HANDLE;

#if defined(FPL_PLATFORM_WINDOWS)
	FPL_LOG_DEBUG(FPL__MODULE_VIDEO_VULKAN, "Query Vulkan Instance Proc 'vkCreateWin32SurfaceKHR' for instance '%p'", nativeBackend->instanceHandle);
	fpl__func_vkCreateWin32SurfaceKHR createProc = (fpl__func_vkCreateWin32SurfaceKHR)api->vkGetInstanceProcAddr(nativeBackend->instanceHandle, "vkCreateWin32SurfaceKHR");
	if (createProc == fpl_null) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Vulkan instance proc 'vkCreateWin32SurfaceKHR' not found! Maybe the instance extension 'VK_KHR_win32_surface' was not set?");
		return(false);
	}

	fpl__VkWin32SurfaceCreateInfoKHR creationInfo = fplZeroInit;
	creationInfo.sType = FPL__VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	creationInfo.hinstance = fpl__global__InitState.win32.appInstance;
	creationInfo.hwnd = windowState->win32.windowHandle;

	FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Create Vulkan Win32 Surface for hwnd '%p', hinstance '%p' and Vulkan instance '%p'", creationInfo.hwnd, creationInfo.hinstance, nativeBackend->instanceHandle);
	fpl__VkResult creationResult = (fpl__VkResult)createProc(nativeBackend->instanceHandle, &creationInfo, nativeBackend->allocator, &surfaceHandle);
	if (creationResult != FPL__VK_SUCCESS) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Failed creating vulkan surface KHR for Win32 -> (VkResult: %d)!", creationResult);
		return(false);
	}
#elif defined(FPL_SUBPLATFORM_X11)
	FPL_LOG_DEBUG(FPL__MODULE_VIDEO_VULKAN, "Query Vulkan Instance Proc 'vkCreateXlibSurfaceKHR' for instance '%p'", nativeBackend->instanceHandle);
	fpl__func_vkCreateXlibSurfaceKHR createProc = (fpl__func_vkCreateXlibSurfaceKHR)api->vkGetInstanceProcAddr(nativeBackend->instanceHandle, "vkCreateXlibSurfaceKHR");
	if (createProc == fpl_null) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Vulkan instance proc 'vkCreateXlibSurfaceKHR' not found! Maybe the instance extension 'VK_KHR_xlib_surface' was not set?");
		return(false);
	}

	fpl__VkXlibSurfaceCreateInfoKHR creationInfo = fplZeroInit;
	creationInfo.sType = FPL__VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	creationInfo.dpy = (fpl__X11Display *)windowState->x11.display;
	creationInfo.window = windowState->x11.window;

	FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Create Vulkan X11 Surface for display '%p', window '%d' and Vulkan instance '%p'", creationInfo.dpy, creationInfo.window, nativeBackend->instanceHandle);
	fpl__VkResult creationResult = createProc(nativeBackend->instanceHandle, &creationInfo, nativeBackend->allocator, &surfaceHandle);
	if (creationResult != FPL__VK_SUCCESS) {
		FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Failed creating vulkan surface KHR for X11 -> (VkResult: %d)!", creationResult);
		return(false);
	}
#else
	FPL__ERROR(FPL__MODULE_VIDEO_VULKAN, "Unsupported Platform!");
	return(false);
#endif

	fplAssert(surfaceHandle != FPL__VK_NULL_HANDLE);
	nativeBackend->surfaceHandle = surfaceHandle;

	backend->surface.vulkan.instance = nativeBackend->instanceHandle;
	backend->surface.vulkan.surfaceKHR = nativeBackend->surfaceHandle;

#if defined(FPL_PLATFORM_WINDOWS)
	backend->surface.window.win32.windowHandle = windowState->win32.windowHandle;
	backend->surface.window.win32.deviceContext = windowState->win32.deviceContext;
#elif defined(FPL_SUBPLATFORM_X11)
	backend->surface.window.x11.display = windowState->x11.display;
	backend->surface.window.x11.window = windowState->x11.window;
	backend->surface.window.x11.screen = windowState->x11.screen;
	backend->surface.window.x11.visual = windowState->x11.visual;
#endif

	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_DESTROYEDWINDOW(fpl__VideoBackend_Vulkan_DestroyedWindow) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;

	// Destroy Vulkan Debug Messenger
	if (!nativeBackend->isInstanceUserDefined && nativeBackend->instanceHandle != fpl_null && nativeBackend->debugMessenger != fpl_null) {
		fpl__VulkanDestroyDebugMessenger(nativeBackend);
	}

	// Destroy Vulkan instance
	const fpl__VulkanApi *api = &nativeBackend->api;
	if (!nativeBackend->isInstanceUserDefined && nativeBackend->instanceHandle != fpl_null) {
		FPL_LOG_INFO(FPL__MODULE_VIDEO_VULKAN, "Destroy Vulkan Instance '%p'", nativeBackend->instanceHandle);
		api->vkDestroyInstance(nativeBackend->instanceHandle, nativeBackend->allocator);
		nativeBackend->instanceHandle = fpl_null;
	}
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_UNLOAD(fpl__VideoBackend_Vulkan_Unload) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;

	// Unload core api
	fpl__UnloadVulkanApi(&nativeBackend->api);

	// Clear everything
	fplClearStruct(nativeBackend);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_LOAD(fpl__VideoBackend_Vulkan_Load) {
	fpl__VideoBackendVulkan *nativeBackend = (fpl__VideoBackendVulkan *)backend;
	const fplVideoSettings *videoSettings = &appState->currentSettings.video;

	// Clear and set magic id
	fplClearStruct(nativeBackend);
	nativeBackend->base.magic = FPL__VIDEOBACKEND_MAGIC;

	// Load core api
	if (!fpl__LoadVulkanApi(&nativeBackend->api, videoSettings->graphics.vulkan.libraryFile)) {
		return(false);
	}

	return(true);
}

fpl_internal FPL__FUNC_VIDEO_BACKEND_PRESENT(fpl__VideoBackend_Vulkan_Present) {
	const fpl__VideoBackendVulkan *nativeBackend = (const fpl__VideoBackendVulkan *)backend;
}

fpl_internal fpl__VideoContext fpl__VideoBackend_Vulkan_Construct() {
	fpl__VideoContext result = fpl__StubVideoContext();
	result.loadFunc = fpl__VideoBackend_Vulkan_Load;
	result.unloadFunc = fpl__VideoBackend_Vulkan_Unload;
	result.getProcedureFunc = fpl__VideoBackend_Vulkan_GetProcedure;
	result.initializeFunc = fpl__VideoBackend_Vulkan_Initialize;
	result.shutdownFunc = fpl__VideoBackend_Vulkan_Shutdown;
	result.prepareWindowFunc = fpl__VideoBackend_Vulkan_PrepareWindow;
	result.finalizeWindowFunc = fpl__VideoBackend_Vulkan_FinalizeWindow;
	result.destroyedWindowFunc = fpl__VideoBackend_Vulkan_DestroyedWindow;
	result.presentFunc = fpl__VideoBackend_Vulkan_Present;
	result.getRequirementsFunc = fpl__VideoBackend_Vulkan_GetRequirements;
	return(result);
}

#endif // FPL__ENABLE_VIDEO_VULKAN


#endif // FPL__VIDEO_BACKENDS_IMPLEMENTED

// ****************************************************************************
//
// > AUDIO_BACKENDS
//
// ****************************************************************************
#if !defined(FPL__AUDIO_BACKENDS_IMPLEMENTED) && defined(FPL__ENABLE_AUDIO)
#	define FPL__AUDIO_BACKENDS_IMPLEMENTED

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
	uint32_t framesRead = 0;
	if (commonAudio->clientReadCallback != fpl_null) {
		framesRead = commonAudio->clientReadCallback(&commonAudio->internalFormat, frameCount, pSamples, commonAudio->clientUserData);
	}
	uint32_t channels = commonAudio->internalFormat.channels;
	uint32_t samplesRead = framesRead * channels;
	uint32_t sampleSize = fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type);
	uint32_t consumedBytes = samplesRead * sampleSize;
	uint32_t remainingBytes = ((frameCount * channels) - samplesRead) * sampleSize;
	if (remainingBytes > 0) {
		fplMemoryClear((uint8_t *)pSamples + consumedBytes, remainingBytes);
	}
	return(samplesRead);
}

// Global Audio GUIDs
#if defined(FPL_PLATFORM_WINDOWS)
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
static GUID FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
#endif

// Forward declarations
fpl_internal fpl__AudioDeviceState fpl__AudioGetDeviceState(fpl__CommonAudioState *audioState);
fpl_internal bool fpl__IsAudioDeviceInitialized(fpl__CommonAudioState *audioState);
fpl_internal bool fpl__IsAudioDeviceStarted(fpl__CommonAudioState *audioState);

// ############################################################################
//
// > AUDIO_BACKEND_DIRECTSOUND
//
// ############################################################################
#if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
#	include <mmreg.h>
#	include <mmsystem.h>
#	include <dsound.h>

#define FPL__FUNC_DSOUND_DirectSoundCreate(name) HRESULT WINAPI name(const GUID* pcGuidDevice, LPDIRECTSOUND *ppDS8, LPUNKNOWN pUnkOuter)
typedef FPL__FUNC_DSOUND_DirectSoundCreate(func_DirectSoundCreate);
#define FPL__FUNC_DSOUND_DirectSoundEnumerateW(name) HRESULT WINAPI name(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
typedef FPL__FUNC_DSOUND_DirectSoundEnumerateW(func_DirectSoundEnumerateW);
static GUID FPL__IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16} };
#define FPL__DIRECTSOUND_MAX_PERIODS 4

typedef struct fpl__DirectSoundApi {
	HMODULE dsoundLibrary;
	func_DirectSoundCreate *DirectSoundCreate;
	func_DirectSoundEnumerateW *DirectSoundEnumerateW;
} fpl__DirectSoundApi;

fpl_internal void fpl__UnloadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	fplAssert(dsoundApi != fpl_null);
	if (dsoundApi->dsoundLibrary != fpl_null) {
		FreeLibrary(dsoundApi->dsoundLibrary);
	}
	fplClearStruct(dsoundApi);
}

fpl_internal bool fpl__LoadDirectSoundApi(fpl__DirectSoundApi *dsoundApi) {
	fplAssert(dsoundApi != fpl_null);
	bool result = false;
	const char *dsoundLibraryName = "dsound.dll";
	fplClearStruct(dsoundApi);
	do {
		HMODULE dsoundLibrary = fpl_null;
		FPL__WIN32_LOAD_LIBRARY(FPL__MODULE_AUDIO_DIRECTSOUND, dsoundLibrary, dsoundLibraryName);
		dsoundApi->dsoundLibrary = dsoundLibrary;
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_DIRECTSOUND, dsoundLibrary, dsoundLibraryName, dsoundApi, func_DirectSoundCreate, DirectSoundCreate);
		FPL__WIN32_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_DIRECTSOUND, dsoundLibrary, dsoundLibraryName, dsoundApi, func_DirectSoundEnumerateW, DirectSoundEnumerateW);
		result = true;
	} while (0);
	if (!result) {
		fpl__UnloadDirectSoundApi(dsoundApi);
	}
	return(result);
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
	fplAudioDeviceInfo *deviceInfos;
	uint32_t foundDeviceCount;
	uint32_t maxDeviceCount;
	uint32_t capacityOverflow;
} fpl__DirectSoundDeviceInfos;

fpl_internal BOOL CALLBACK fpl__GetDeviceCallbackDirectSound(LPGUID lpGuid, LPCWSTR lpwstrDescription, LPCWSTR lpwstrModule, LPVOID lpContext) {
	fpl__DirectSoundDeviceInfos *infos = (fpl__DirectSoundDeviceInfos *)lpContext;
	fplAssert(infos != fpl_null);
	if (infos->deviceInfos != fpl_null) {
		uint32_t index = infos->foundDeviceCount++;
		if (index < infos->maxDeviceCount) {
			fplAudioDeviceInfo *deviceInfo = infos->deviceInfos + index;
			fplClearStruct(deviceInfo);
			fplWideStringToUTF8String(lpwstrDescription, lstrlenW(lpwstrDescription), deviceInfo->name, fplArrayCount(deviceInfo->name));
			if (lpGuid != fpl_null) {
				fplMemoryCopy(lpGuid, sizeof(deviceInfo->id.dshow), &deviceInfo->id.dshow);
			}
		} else {
			infos->capacityOverflow++;
		}
	} else {
		infos->foundDeviceCount++;
	}
	return TRUE;
}

fpl_internal uint32_t fpl__GetAudioDevicesDirectSound(fpl__DirectSoundAudioState *dsoundState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
	uint32_t result = 0;
	const fpl__DirectSoundApi *dsoundApi = &dsoundState->api;
	fpl__DirectSoundDeviceInfos infos = fplZeroInit;
	infos.maxDeviceCount = maxDeviceCount;
	infos.deviceInfos = deviceInfos;
	infos.capacityOverflow = 0;
	dsoundApi->DirectSoundEnumerateW(fpl__GetDeviceCallbackDirectSound, &infos);
	result = infos.foundDeviceCount;
	if (infos.capacityOverflow > 0) {
		FPL__ERROR(FPL__MODULE_AUDIO_DIRECTSOUND, "Capacity of '%lu' for audio device infos has been reached. '%lu' audio devices are not included in the result", maxDeviceCount, infos.capacityOverflow);
	}
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

	fplClearStruct(dsoundState);

	return true;
}

fpl_internal fplAudioResultType fpl__AudioInitDirectSound(const fplAudioSettings *audioSettings, const fplAudioDeviceFormat *targetFormat, fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
#ifdef __cplusplus
	GUID guid_IID_IDirectSoundNotify = FPL__IID_IDirectSoundNotify;
#else
	GUID *guid_IID_IDirectSoundNotify = &FPL__IID_IDirectSoundNotify;
#endif

#define FPL__DSOUND_INIT_ERROR(ret, format, ...) do { \
	FPL__ERROR(FPL__MODULE_AUDIO_DIRECTSOUND, format, ## __VA_ARGS__); \
	fpl__AudioReleaseDirectSound(commonAudio, dsoundState); \
	return ret; \
} while (0)

	fplAssert(fpl__global__AppState != fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__Win32AppState *win32AppState = &appState->win32;
	const fpl__Win32Api *apiFuncs = &win32AppState->winApi;

	// Load direct sound library
	fpl__DirectSoundApi *dsoundApi = &dsoundState->api;
	if (!fpl__LoadDirectSoundApi(dsoundApi)) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_ApiFailed, "API could not be loaded!");
	}

	// Load direct sound object
	const fplAudioDeviceInfo *targetDevice = &audioSettings->targetDevice;
	const GUID *deviceId = fpl_null;
	if (fplGetStringLength(targetDevice->name) > 0) {
		fplAssert(sizeof(GUID) == sizeof(targetDevice->id.dshow));
		deviceId = (const GUID *)&targetDevice->id.dshow;
	}
	if (!SUCCEEDED(dsoundApi->DirectSoundCreate(deviceId, &dsoundState->directSound, fpl_null))) {
		char idString[64];
		fpl__Win32FormatGuidString(idString, fplArrayCount(idString), deviceId);
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_NoDeviceFound, "Audio device by id '%s' could not be created!", idString);
	}

	// Setup wave format ex
	WAVEFORMATEXTENSIBLE waveFormat = fplZeroInit;
	waveFormat.Format.cbSize = sizeof(waveFormat);
	waveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	waveFormat.Format.nChannels = (WORD)targetFormat->channels;
	waveFormat.Format.nSamplesPerSec = (DWORD)targetFormat->sampleRate;
	waveFormat.Format.wBitsPerSample = (WORD)fplGetAudioSampleSizeInBytes(targetFormat->type) * 8;
	waveFormat.Format.nBlockAlign = (waveFormat.Format.nChannels * waveFormat.Format.wBitsPerSample) / 8;
	waveFormat.Format.nAvgBytesPerSec = waveFormat.Format.nBlockAlign * waveFormat.Format.nSamplesPerSec;
	waveFormat.Samples.wValidBitsPerSample = waveFormat.Format.wBitsPerSample;
	if ((targetFormat->type == fplAudioFormatType_F32) || (targetFormat->type == fplAudioFormatType_F64)) {
		waveFormat.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	} else {
		waveFormat.SubFormat = FPL__GUID_KSDATAFORMAT_SUBTYPE_PCM;
	}

	// Get either local window handle or desktop handle
	HWND windowHandle = fpl_null;
#	if defined(FPL__ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_Window) {
		windowHandle = appState->window.win32.windowHandle;
	}
#	endif
	if (windowHandle == fpl_null) {
		windowHandle = apiFuncs->user.GetDesktopWindow();
	}

	// The cooperative level must be set before doing anything else
	if (FAILED(IDirectSound_SetCooperativeLevel(dsoundState->directSound, windowHandle, (targetFormat->preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed setting DirectSound Cooperative Level to '%s' mode!", (targetFormat->preferExclusiveMode ? "Exclusive" : "Priority"));
	}

	// Create primary buffer
	DSBUFFERDESC descDSPrimary = fplZeroInit;
	descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
	descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	if (FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDSPrimary, &dsoundState->primaryBuffer, fpl_null))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed creating primary DirectSound sound buffer!");
	}

	// Set format
	if (FAILED(IDirectSoundBuffer_SetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX *)&waveFormat))) {
		char subformatString[64];
		fpl__Win32FormatGuidString(subformatString, fplArrayCount(subformatString), &waveFormat.SubFormat);
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed setting audio format for primary sound buffer to (nChannels: %d, nSamplesPerSec: %u, wBitsPerSample: %d, subformat: '%s')!", waveFormat.Format.nChannels, waveFormat.Format.nSamplesPerSec, waveFormat.Format.wBitsPerSample, subformatString);
	}

	// Get the required size in bytes
	DWORD requiredSize;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, fpl_null, 0, &requiredSize))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed getting format size for primary sound buffer!");
	}

	// Get actual format
	char actualFormatData[1024];
	WAVEFORMATEXTENSIBLE *actualFormat = (WAVEFORMATEXTENSIBLE *)actualFormatData;
	if (FAILED(IDirectSoundBuffer_GetFormat(dsoundState->primaryBuffer, (WAVEFORMATEX *)actualFormat, requiredSize, fpl_null))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed getting actual wave format from size '{%u}' for primary sound buffer!", requiredSize);
	}

	// Set internal format
	fplAudioDeviceFormat internalFormat = fplZeroInit;
	internalFormat.backend = fplAudioBackendType_DirectSound;
	if (fpl__Win32IsEqualGuid(actualFormat->SubFormat, FPL__GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
		if (actualFormat->Format.wBitsPerSample == 64) {
			internalFormat.type = fplAudioFormatType_F64;
		} else {
			internalFormat.type = fplAudioFormatType_F32;
		}
	} else {
		switch (actualFormat->Format.wBitsPerSample) {
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
	internalFormat.channels = actualFormat->Format.nChannels;
	internalFormat.sampleRate = actualFormat->Format.nSamplesPerSec;

	// @NOTE(final): We divide up our playback buffer into this number of periods and let directsound notify us when one of it needs to play.
	internalFormat.periods = fplMax(2, fplMin(targetFormat->periods, 4));
	internalFormat.bufferSizeInFrames = targetFormat->bufferSizeInFrames;
	uint32_t bufferSizeInBytes = fplGetAudioBufferSizeInBytes(internalFormat.type, internalFormat.channels, internalFormat.bufferSizeInFrames);

	commonAudio->internalFormat = internalFormat;

	const char *internalFormatTypeName = fplGetAudioFormatName(internalFormat.type);
	FPL_LOG(fplLogLevel_Info, FPL__MODULE_AUDIO_DIRECTSOUND,
		"Using internal format (Channels: %u, Samplerate: %u, Type: %s, Periods: %u, Buffer size frames/bytes: %u/%u)",
		internalFormat.channels,
		internalFormat.sampleRate,
		internalFormatTypeName,
		internalFormat.periods,
		internalFormat.bufferSizeInFrames,
		bufferSizeInBytes);

// Create secondary buffer
	DSBUFFERDESC descDS = fplZeroInit;
	descDS.dwSize = sizeof(DSBUFFERDESC);
	descDS.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	descDS.dwBufferBytes = (DWORD)bufferSizeInBytes;
	descDS.lpwfxFormat = (WAVEFORMATEX *)&waveFormat;
	if (FAILED(IDirectSound_CreateSoundBuffer(dsoundState->directSound, &descDS, &dsoundState->secondaryBuffer, fpl_null))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed creating secondary sound buffer with buffer size of '%u' bytes", bufferSizeInBytes);
	}

	// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
	if (FAILED(IDirectSoundBuffer_QueryInterface(dsoundState->secondaryBuffer, guid_IID_IDirectSoundNotify, (void **)&dsoundState->notify))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed query direct sound notify interface");
	}

	// Setup notifications
	DWORD periodSizeInBytes = bufferSizeInBytes / internalFormat.periods;
	DSBPOSITIONNOTIFY notifyPoints[FPL__DIRECTSOUND_MAX_PERIODS];
	for (uint32_t i = 0; i < internalFormat.periods; ++i) {
		dsoundState->notifyEvents[i] = CreateEventA(fpl_null, false, false, fpl_null);
		if (dsoundState->notifyEvents[i] == fpl_null) {
			fpl__AudioReleaseDirectSound(commonAudio, dsoundState);
			return fplAudioResultType_Failed;
		}

		// The notification offset is in bytes.
		notifyPoints[i].dwOffset = i * periodSizeInBytes;
		notifyPoints[i].hEventNotify = dsoundState->notifyEvents[i];
	}
	if (FAILED(IDirectSoundNotify_SetNotificationPositions(dsoundState->notify, internalFormat.periods, notifyPoints))) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed setting notification position for %u periods", internalFormat.periods);
	}

	// Create stop event
	dsoundState->stopEvent = CreateEventA(fpl_null, false, false, fpl_null);
	if (dsoundState->stopEvent == fpl_null) {
		FPL__DSOUND_INIT_ERROR(fplAudioResultType_Failed, "Failed creating stop event");
	}

	return fplAudioResultType_Success;
}

fpl_internal void fpl__AudioStopMainLoopDirectSound(fpl__DirectSoundAudioState *dsoundState) {
	dsoundState->breakMainLoop = true;
	SetEvent(dsoundState->stopEvent);
}

fpl_internal bool fpl__GetCurrentFrameDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState, uint32_t *pCurrentPos) {
	fplAssert(pCurrentPos != fpl_null);
	*pCurrentPos = 0;

	fplAssert(dsoundState->secondaryBuffer != fpl_null);
	DWORD dwCurrentPosition;
	if (FAILED(IDirectSoundBuffer_GetCurrentPosition(dsoundState->secondaryBuffer, fpl_null, &dwCurrentPosition))) {
		return false;
	}

	fplAssert(commonAudio->internalFormat.channels > 0);
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
	fplAssert(committedSize <= totalFrameCount);

	return totalFrameCount - committedSize;
}

fpl_internal uint32_t fpl__WaitForFramesDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
	fplAssert(commonAudio->internalFormat.sampleRate > 0);
	fplAssert(commonAudio->internalFormat.periods > 0);

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
	fplAssert(dsoundState->secondaryBuffer != fpl_null);
	if (FAILED(IDirectSoundBuffer_Stop(dsoundState->secondaryBuffer))) {
		return false;
	}
	IDirectSoundBuffer_SetCurrentPosition(dsoundState->secondaryBuffer, 0);
	return true;
}

fpl_internal fplAudioResultType fpl__AudioStartDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
	fplAssert(commonAudio->internalFormat.channels > 0);
	fplAssert(commonAudio->internalFormat.periods > 0);
	uint32_t audioSampleSizeBytes = fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type);
	fplAssert(audioSampleSizeBytes > 0);

	// Before playing anything we need to grab an initial group of samples from the client.
	uint32_t framesToRead = commonAudio->internalFormat.bufferSizeInFrames / commonAudio->internalFormat.periods;
	uint32_t desiredLockSize = framesToRead * commonAudio->internalFormat.channels * audioSampleSizeBytes;

	void *pLockPtr;
	DWORD actualLockSize;
	void *pLockPtr2;
	DWORD actualLockSize2;

	if (SUCCEEDED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, 0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
		framesToRead = actualLockSize / audioSampleSizeBytes / commonAudio->internalFormat.channels;
		fpl__ReadAudioFramesFromClient(commonAudio, framesToRead, pLockPtr);
		IDirectSoundBuffer_Unlock(dsoundState->secondaryBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
		dsoundState->lastProcessedFrame = framesToRead;
		if (FAILED(IDirectSoundBuffer_Play(dsoundState->secondaryBuffer, 0, 0, DSBPLAY_LOOPING))) {
			return fplAudioResultType_Failed;
		}
	} else {
		return fplAudioResultType_Failed;
	}
	return fplAudioResultType_Success;
}

fpl_internal void fpl__AudioRunMainLoopDirectSound(const fpl__CommonAudioState *commonAudio, fpl__DirectSoundAudioState *dsoundState) {
	fplAssert(commonAudio->internalFormat.channels > 0);
	uint32_t audioSampleSizeBytes = fplGetAudioSampleSizeInBytes(commonAudio->internalFormat.type);
	fplAssert(audioSampleSizeBytes > 0);

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
			void *pLockPtr;
			DWORD actualLockSize;
			void *pLockPtr2;
			DWORD actualLockSize2;
			if (FAILED(IDirectSoundBuffer_Lock(dsoundState->secondaryBuffer, lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
				FPL__ERROR(FPL__MODULE_AUDIO_DIRECTSOUND, "Failed to lock directsound secondary buffer '%p' for offset/size (%lu / %lu)", dsoundState->secondaryBuffer, lockOffset, lockSize);
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
#endif // FPL__ENABLE_AUDIO_DIRECTSOUND

// ############################################################################
//
// > AUDIO_BACKEND_ALSA
//
// ############################################################################
#if defined(FPL__ENABLE_AUDIO_ALSA)

// NOTE(final): ALSA on Raspberry, due to high latency requires large audio buffers, so below we have a table mapped from device names to a scaling factor.
typedef struct fpl__AlsaBufferScale {
	const char *deviceName;
	float scale;
} fpl__AlsaBufferScale;

fpl_globalvar fpl__AlsaBufferScale fpl__globalAlsaBufferScales[] = {
	fplStructInit(fpl__AlsaBufferScale, "*bcm2835*", 2.0f),
};

fpl_internal float fpl__AlsaGetBufferScale(const char *deviceName) {
	if (fplGetStringLength(deviceName) > 0) {
		for (int i = 0; i < fplArrayCount(fpl__globalAlsaBufferScales); ++i) {
			const char *testDeviceName = fpl__globalAlsaBufferScales[i].deviceName;
			if (fplIsStringMatchWildcard(deviceName, testDeviceName)) {
				float scale = fpl__globalAlsaBufferScales[i].scale;
				return(scale);
			}
		}
	}
	return(1.0f);
}

fpl_internal uint32_t fpl__AlsaScaleBufferSize(const uint32_t bufferSize, const float scale) {
	uint32_t result = fplMax(1, (uint32_t)(bufferSize * scale));
	return(result);
}

#if defined(FPL__ANONYMOUS_ALSA_HEADERS)
typedef void snd_pcm_t;
typedef void snd_pcm_format_mask_t;
typedef void snd_pcm_hw_params_t;
typedef void snd_pcm_sw_params_t;

typedef uint64_t snd_pcm_uframes_t;
typedef int64_t snd_pcm_sframes_t;

typedef struct snd_pcm_chmap_t {
	unsigned int channels;
	unsigned int pos[0];
} snd_pcm_chmap_t;

typedef enum snd_pcm_stream_t {
	SND_PCM_STREAM_PLAYBACK = 0,
	SND_PCM_STREAM_CAPTURE,
} snd_pcm_stream_t;

typedef struct snd_pcm_channel_area_t {
	void *addr;
	unsigned int first;
	unsigned int step;
} snd_pcm_channel_area_t;

typedef enum snd_pcm_format_t {
	SND_PCM_FORMAT_UNKNOWN = -1,
	SND_PCM_FORMAT_S8 = 0,
	SND_PCM_FORMAT_U8,
	SND_PCM_FORMAT_S16_LE,
	SND_PCM_FORMAT_S16_BE,
	SND_PCM_FORMAT_U16_LE,
	SND_PCM_FORMAT_U16_BE,
	SND_PCM_FORMAT_S24_LE,
	SND_PCM_FORMAT_S24_BE,
	SND_PCM_FORMAT_U24_LE,
	SND_PCM_FORMAT_U24_BE,
	SND_PCM_FORMAT_S32_LE,
	SND_PCM_FORMAT_S32_BE,
	SND_PCM_FORMAT_U32_LE,
	SND_PCM_FORMAT_U32_BE,
	SND_PCM_FORMAT_FLOAT_LE,
	SND_PCM_FORMAT_FLOAT_BE,
	SND_PCM_FORMAT_FLOAT64_LE,
	SND_PCM_FORMAT_FLOAT64_BE,
	SND_PCM_FORMAT_IEC958_SUBFRAME_LE,
	SND_PCM_FORMAT_IEC958_SUBFRAME_BE,
	SND_PCM_FORMAT_MU_LAW,
	SND_PCM_FORMAT_A_LAW,
	SND_PCM_FORMAT_IMA_ADPCM,
	SND_PCM_FORMAT_MPEG,
	SND_PCM_FORMAT_GSM,
	SND_PCM_FORMAT_S20_LE,
	SND_PCM_FORMAT_S20_BE,
	SND_PCM_FORMAT_U20_LE,
	SND_PCM_FORMAT_U20_BE,
	SND_PCM_FORMAT_SPECIAL,
	SND_PCM_FORMAT_S24_3LE,
	SND_PCM_FORMAT_S24_3BE,
	SND_PCM_FORMAT_U24_3LE,
	SND_PCM_FORMAT_U24_3BE,
	SND_PCM_FORMAT_S20_3LE,
	SND_PCM_FORMAT_S20_3BE,
	SND_PCM_FORMAT_U20_3LE,
	SND_PCM_FORMAT_U20_3BE,
	SND_PCM_FORMAT_S18_3LE,
	SND_PCM_FORMAT_S18_3BE,
	SND_PCM_FORMAT_U18_3LE,
	SND_PCM_FORMAT_U18_3BE,
	SND_PCM_FORMAT_S16,
	SND_PCM_FORMAT_U16,
	SND_PCM_FORMAT_S24,
	SND_PCM_FORMAT_U24,
	SND_PCM_FORMAT_S32,
	SND_PCM_FORMAT_U32,
	SND_PCM_FORMAT_FLOAT,
	SND_PCM_FORMAT_FLOAT64,
	SND_PCM_FORMAT_IEC958_SUBFRAME,
	SND_PCM_FORMAT_S20,
	SND_PCM_FORMAT_U20
} snd_pcm_format_t;

typedef enum snd_pcm_access_t {
	SND_PCM_ACCESS_MMAP_INTERLEAVED = 0,
	SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
	SND_PCM_ACCESS_MMAP_COMPLEX,
	SND_PCM_ACCESS_RW_INTERLEAVED,
	SND_PCM_ACCESS_RW_NONINTERLEAVED
} snd_pcm_access_t;

#define SND_PCM_NO_AUTO_RESAMPLE 0x00010000
#define SND_PCM_NO_AUTO_CHANNELS 0x00020000
#define	SND_PCM_NO_AUTO_FORMAT 0x00040000
#else
// @TODO(final/ALSA): Remove ALSA include when runtime linking is enabled
#	include <alsa/asoundlib.h>
#endif // FPL__ANONYMOUS_ALSA_HEADERS

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
#define FPL__ALSA_FUNC_snd_device_name_free_hint(name) int name(void **hints)
typedef FPL__ALSA_FUNC_snd_device_name_free_hint(fpl__alsa_func_snd_device_name_free_hint);
#define FPL__ALSA_FUNC_snd_card_get_index(name) int name(const char *name)
typedef FPL__ALSA_FUNC_snd_card_get_index(fpl__alsa_func_snd_card_get_index);
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
#define FPL__ALSA_FUNC_snd_pcm_info_sizeof(name) size_t name(void)
typedef FPL__ALSA_FUNC_snd_pcm_info_sizeof(fpl__alsa_func_snd_pcm_info_sizeof);
#define FPL__ALSA_FUNC_snd_pcm_info(name) int name(snd_pcm_t *handle, snd_pcm_info_t *info)
typedef FPL__ALSA_FUNC_snd_pcm_info(fpl__alsa_func_snd_pcm_info);
#define FPL__ALSA_FUNC_snd_pcm_info_get_name(name) const char* name(const snd_pcm_info_t *obj)
typedef FPL__ALSA_FUNC_snd_pcm_info_get_name(fpl__alsa_func_snd_pcm_info_get_name);

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
	fpl__alsa_func_snd_device_name_free_hint *snd_device_name_free_hint;
	fpl__alsa_func_snd_card_get_index *snd_card_get_index;
	fpl__alsa_func_snd_pcm_mmap_begin *snd_pcm_mmap_begin;
	fpl__alsa_func_snd_pcm_mmap_commit *snd_pcm_mmap_commit;
	fpl__alsa_func_snd_pcm_recover *snd_pcm_recover;
	fpl__alsa_func_snd_pcm_writei *snd_pcm_writei;
	fpl__alsa_func_snd_pcm_avail *snd_pcm_avail;
	fpl__alsa_func_snd_pcm_avail_update *snd_pcm_avail_update;
	fpl__alsa_func_snd_pcm_wait *snd_pcm_wait;
	fpl__alsa_func_snd_pcm_info_sizeof *snd_pcm_info_sizeof;
	fpl__alsa_func_snd_pcm_info *snd_pcm_info;
	fpl__alsa_func_snd_pcm_info_get_name *snd_pcm_info_get_name;
} fpl__AlsaAudioApi;

typedef struct fpl__AlsaAudioState {
	fpl__AlsaAudioApi api;
	snd_pcm_t *pcmDevice;
	void *intermediaryBuffer;
	bool isUsingMMap;
	bool breakMainLoop;
} fpl__AlsaAudioState;

fpl_internal void fpl__UnloadAlsaApi(fpl__AlsaAudioApi *alsaApi) {
	fplAssert(alsaApi != fpl_null);
	if (alsaApi->libHandle != fpl_null) {
		dlclose(alsaApi->libHandle);
	}
	fplClearStruct(alsaApi);
}

fpl_internal bool fpl__LoadAlsaApi(fpl__AlsaAudioApi *alsaApi) {
	fplAssert(alsaApi != fpl_null);
	const char *libraryNames[] = {
		"libasound.so",
	};
	bool result = false;
	for (uint32_t index = 0; index < fplArrayCount(libraryNames); ++index) {
		const char *libName = libraryNames[index];
		fplClearStruct(alsaApi);
		do {
			void *libHandle = fpl_null;
			FPL__POSIX_LOAD_LIBRARY(FPL__MODULE_AUDIO_ALSA, libHandle, libName);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_open, snd_pcm_open);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_close, snd_pcm_close);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_sizeof, snd_pcm_hw_params_sizeof);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params, snd_pcm_hw_params);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_any, snd_pcm_hw_params_any);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_format, snd_pcm_hw_params_set_format);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_format_first, snd_pcm_hw_params_set_format_first);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_format_mask, snd_pcm_hw_params_get_format_mask);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_channels_near, snd_pcm_hw_params_set_channels_near);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_rate_resample, snd_pcm_hw_params_set_rate_resample);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_rate_near, snd_pcm_hw_params_set_rate_near);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_buffer_size_near, snd_pcm_hw_params_set_buffer_size_near);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_periods_near, snd_pcm_hw_params_set_periods_near);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_set_access, snd_pcm_hw_params_set_access);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_format, snd_pcm_hw_params_get_format);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_channels, snd_pcm_hw_params_get_channels);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_rate, snd_pcm_hw_params_get_rate);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_buffer_size, snd_pcm_hw_params_get_buffer_size);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_periods, snd_pcm_hw_params_get_periods);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_access, snd_pcm_hw_params_get_access);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_hw_params_get_sbits, snd_pcm_hw_params_get_sbits);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_sw_params_sizeof, snd_pcm_sw_params_sizeof);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_sw_params_current, snd_pcm_sw_params_current);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_sw_params_set_avail_min, snd_pcm_sw_params_set_avail_min);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_sw_params_set_start_threshold, snd_pcm_sw_params_set_start_threshold);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_sw_params, snd_pcm_sw_params);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_format_mask_sizeof, snd_pcm_format_mask_sizeof);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_format_mask_test, snd_pcm_format_mask_test);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_get_chmap, snd_pcm_get_chmap);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_prepare, snd_pcm_prepare);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_start, snd_pcm_start);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_drop, snd_pcm_drop);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_device_name_hint, snd_device_name_hint);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_device_name_get_hint, snd_device_name_get_hint);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_device_name_free_hint, snd_device_name_free_hint);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_card_get_index, snd_card_get_index);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_mmap_begin, snd_pcm_mmap_begin);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_mmap_commit, snd_pcm_mmap_commit);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_recover, snd_pcm_recover);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_writei, snd_pcm_writei);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_avail, snd_pcm_avail);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_avail_update, snd_pcm_avail_update);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_wait, snd_pcm_wait);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_info_sizeof, snd_pcm_info_sizeof);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_info, snd_pcm_info);
			FPL__POSIX_GET_FUNCTION_ADDRESS(FPL__MODULE_AUDIO_ALSA, libHandle, libName, alsaApi, fpl__alsa_func_snd_pcm_info_get_name, snd_pcm_info_get_name);
			alsaApi->libHandle = libHandle;
			result = true;
		} while (0);
		if (result) {
			break;
		}
		fpl__UnloadAlsaApi(alsaApi);
	}
	return(result);
}

fpl_internal uint32_t fpl__AudioWaitForFramesAlsa(const fplAudioDeviceFormat *deviceFormat, fpl__AlsaAudioState *alsaState, bool *requiresRestart) {
	fplAssert(deviceFormat != fpl_null);
	if (requiresRestart != fpl_null) {
		*requiresRestart = false;
	}
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	uint32_t periodSizeInFrames = deviceFormat->bufferSizeInFrames / deviceFormat->periods;
	while (!alsaState->breakMainLoop) {
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

		if (framesAvailable < periodSizeInFrames) {
			// Less than a whole period is available so keep waiting.
			int waitResult = alsaApi->snd_pcm_wait(alsaState->pcmDevice, -1);
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
		}
	}

	// We'll get here if the loop was terminated. Just return whatever's available.
	snd_pcm_sframes_t framesAvailable = alsaApi->snd_pcm_avail_update(alsaState->pcmDevice);
	if (framesAvailable < 0) {
		return 0;
	}
	return framesAvailable;
}

fpl_internal bool fpl__GetAudioFramesFromClientAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	fplAssert(commonAudio != fpl_null && alsaState != fpl_null);
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

		const snd_pcm_channel_area_t *channelAreas;
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
			if (requiresRestart) {
				if (alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
					return false;
				}
			}
			if (framesAvailable >= mappedFrames) {
				framesAvailable -= mappedFrames;
			} else {
				framesAvailable = 0;
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
						FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to recover device after underrun!");
						return false;
					}
					framesWritten = alsaApi->snd_pcm_writei(alsaState->pcmDevice, alsaState->intermediaryBuffer, framesAvailable);
					if (framesWritten < 0) {
						FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to write data to the PCM device!");
						return false;
					}
					// Success
					break;
				} else {
					FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to write audio frames from client, error code: %d!", framesWritten);
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
	fplAssert(alsaState != fpl_null);
	alsaState->breakMainLoop = true;
}

fpl_internal bool fpl__AudioReleaseAlsa(const fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	fplAssert(commonAudio != fpl_null && alsaState != fpl_null);
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (alsaState->pcmDevice != fpl_null) {
		alsaApi->snd_pcm_close(alsaState->pcmDevice);
		alsaState->pcmDevice = fpl_null;
		if (alsaState->intermediaryBuffer != fpl_null) {
			fpl__ReleaseDynamicMemory(alsaState->intermediaryBuffer);
			alsaState->intermediaryBuffer = fpl_null;
		}
	}
	fpl__UnloadAlsaApi(alsaApi);
	fplClearStruct(alsaState);
	return true;
}

fpl_internal fplAudioResultType fpl__AudioStartAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	fplAssert(commonAudio != fpl_null && alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;

	// Prepare the device
	if (alsaApi->snd_pcm_prepare(alsaState->pcmDevice) < 0) {
		FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to prepare PCM device '%p'!", alsaState->pcmDevice);
		return fplAudioResultType_Failed;
	}

	// Get initial frames to fill from the client
	if (!fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
		FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to get initial audio frames from client!");
		return fplAudioResultType_Failed;
	}

	if (alsaState->isUsingMMap) {
		if (alsaApi->snd_pcm_start(alsaState->pcmDevice) < 0) {
			FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to start PCM device '%p'!", alsaState->pcmDevice);
			return fplAudioResultType_Failed;
		}
	}

	return fplAudioResultType_Success;
}

fpl_internal bool fpl__AudioStopAlsa(fpl__AlsaAudioState *alsaState) {
	fplAssert(alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (alsaApi->snd_pcm_drop(alsaState->pcmDevice)) {
		FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Failed to drop the PCM device '%p'!", alsaState->pcmDevice);
		return false;
	}
	return true;
}

fpl_internal snd_pcm_format_t fpl__MapAudioFormatToAlsaFormat(fplAudioFormatType format) {
	// @TODO(final): [ALSA] Mapping table from fplAudioFormatType to snd_pcm_format_t here!
	bool isBigEndian = fplIsBigEndian();
	if (isBigEndian) {
		switch (format) {
			case fplAudioFormatType_U8:
				return SND_PCM_FORMAT_U8;
			case fplAudioFormatType_S16:
				return SND_PCM_FORMAT_S16_BE;
			case fplAudioFormatType_S24:
				return SND_PCM_FORMAT_S24_3BE;
			case fplAudioFormatType_S32:
				return SND_PCM_FORMAT_S32_BE;
			case fplAudioFormatType_F32:
				return SND_PCM_FORMAT_FLOAT_BE;
			default:
				return SND_PCM_FORMAT_UNKNOWN;
		}
	} else {
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
}

fpl_internal void fpl__AudioRunMainLoopAlsa(fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	fplAssert(alsaState != fpl_null);
	alsaState->breakMainLoop = false;
	while (!alsaState->breakMainLoop && fpl__GetAudioFramesFromClientAlsa(commonAudio, alsaState)) {
	}
}

fpl_internal fplAudioFormatType fpl__MapAlsaFormatToAudioFormat(snd_pcm_format_t format) {
	// @TODO(final): [ALSA] Mapping table from snd_pcm_format_t to fplAudioFormatType here!
	switch (format) {
		case SND_PCM_FORMAT_U8:
			return fplAudioFormatType_U8;
		case SND_PCM_FORMAT_S16_BE:
		case SND_PCM_FORMAT_S16_LE:
			return fplAudioFormatType_S16;
		case SND_PCM_FORMAT_S24_3BE:
		case SND_PCM_FORMAT_S24_3LE:
			return fplAudioFormatType_S24;
		case SND_PCM_FORMAT_S32_BE:
		case SND_PCM_FORMAT_S32_LE:
			return fplAudioFormatType_S32;
		case SND_PCM_FORMAT_FLOAT_BE:
		case SND_PCM_FORMAT_FLOAT_LE:
			return fplAudioFormatType_F32;
		default:
			return fplAudioFormatType_None;
	}
}

fpl_internal fplAudioResultType fpl__AudioInitAlsa(const fplAudioSettings *audioSettings, const fplAudioDeviceFormat *targetFormat, fpl__CommonAudioState *commonAudio, fpl__AlsaAudioState *alsaState) {
	snd_pcm_hw_params_t *hardwareParams = fpl_null;
	snd_pcm_sw_params_t *softwareParams = fpl_null;

#	define FPL__ALSA_INIT_ERROR(ret, format, ...) do { \
		if (softwareParams != fpl_null) fpl__ReleaseTemporaryMemory(softwareParams); \
		if (hardwareParams != fpl_null) fpl__ReleaseTemporaryMemory(hardwareParams); \
		FPL__ERROR(FPL__MODULE_AUDIO_ALSA, format, ## __VA_ARGS__); \
		fpl__AudioReleaseAlsa(commonAudio, alsaState); \
		return fplAudioResultType_Failed; \
	} while (0)

	// Load ALSA library
	fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	if (!fpl__LoadAlsaApi(alsaApi)) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_ApiFailed, "Failed loading ALSA api!");
	}

	//
	// Open PCM Device
	//
	fplAudioDeviceInfo deviceInfo = audioSettings->targetDevice;
	char deviceName[256] = fplZeroInit;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	int openMode = SND_PCM_NO_AUTO_RESAMPLE | SND_PCM_NO_AUTO_CHANNELS | SND_PCM_NO_AUTO_FORMAT;
	if (fplGetStringLength(deviceInfo.id.alsa) == 0) {
		const char *defaultDeviceNames[16] = fplZeroInit;
		int defaultDeviceCount = 0;
		defaultDeviceNames[defaultDeviceCount++] = "default";
		if (!targetFormat->preferExclusiveMode) {
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
			FPL_LOG_DEBUG("ALSA", "Opening PCM audio device '%s'", defaultDeviceName);
			if (alsaApi->snd_pcm_open(&alsaState->pcmDevice, defaultDeviceName, stream, openMode) == 0) {
				FPL_LOG_DEBUG("ALSA", "Successfully opened PCM audio device '%s'", defaultDeviceName);
				isDeviceOpen = true;
				fplCopyString(defaultDeviceName, deviceName, fplArrayCount(deviceName));
				break;
			} else {
				FPL_LOG_ERROR("ALSA", "Failed opening PCM audio device '%s'!", defaultDeviceName);
			}
		}
		if (!isDeviceOpen) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_NoDeviceFound, "No PCM audio device found!");
		}
	} else {
		const char *forcedDeviceId = audioSettings->targetDevice.id.alsa;
		// @TODO(final/ALSA): Do we want to allow device ids to be :%d,%d so we can probe "dmix" and "hw" ?
		if (alsaApi->snd_pcm_open(&alsaState->pcmDevice, forcedDeviceId, stream, openMode) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_NoDeviceFound, "PCM audio device by id '%s' not found!", forcedDeviceId);
		}
		fplCopyString(forcedDeviceId, deviceName, fplArrayCount(deviceName));
	}

	//
	// Buffer sizes
	//
	// Some audio devices have high latency, so using the default buffer size will not work.
	// We have to scale the buffer sizes for special devices, such as broadcom audio (Raspberry Pi)
	// See fpl__AlsaGetBufferScale for details
	// Idea comes from miniaudio, which does the same thing - so the code is almost identically here
	//
	float bufferSizeScaleFactor = 1.0f;
	if ((targetFormat->defaultFields & fplAudioDefaultFields_BufferSize) == fplAudioDefaultFields_BufferSize) {
		size_t pcmInfoSize = alsaApi->snd_pcm_info_sizeof();
		snd_pcm_info_t *pcmInfo = (snd_pcm_info_t *)fpl__AllocateTemporaryMemory(pcmInfoSize, 8);
		if (pcmInfo == fpl_null) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_OutOfMemory, "Out of stack memory for snd_pcm_info_t!");
		}

		// Query device name
		if (alsaApi->snd_pcm_info(alsaState->pcmDevice, pcmInfo) == 0) {
			const char *deviceName = alsaApi->snd_pcm_info_get_name(pcmInfo);
			if (deviceName != fpl_null) {
				if (fplIsStringEqual("default", deviceName)) {
					// The device name "default" is useless for buffer-scaling, so we search for the real device name in the hint-table
					char **ppDeviceHints;
					if (alsaApi->snd_device_name_hint(-1, "pcm", (void ***)&ppDeviceHints) == 0) {
						char **ppNextDeviceHint = ppDeviceHints;

						while (*ppNextDeviceHint != fpl_null) {
							char *hintName = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "NAME");
							char *hintDesc = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "DESC");
							char *hintIOID = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "IOID");

							bool foundDevice = false;
							if (hintIOID == fpl_null || fplIsStringEqual(hintIOID, "Output")) {
								if (fplIsStringEqual(hintName, deviceName)) {
									// We found the default device and can now get the scale for the description
									bufferSizeScaleFactor = fpl__AlsaGetBufferScale(hintDesc);
									foundDevice = true;
								}
							}

							// Unfortunatly the hint strings are malloced, so we have to free it :(
							free(hintName);
							free(hintDesc);
							free(hintIOID);

							++ppNextDeviceHint;

							if (foundDevice) {
								break;
							}
						}

						alsaApi->snd_device_name_free_hint((void **)ppDeviceHints);
					}
				} else {
					bufferSizeScaleFactor = fpl__AlsaGetBufferScale(deviceName);
				}
			}
		}
		fpl__ReleaseTemporaryMemory(pcmInfo);
	}

	//
	// Get hardware parameters
	//
	fplAssert(alsaState->pcmDevice != fpl_null);
	fplAssert(fplGetStringLength(deviceName) > 0);

	FPL_LOG_DEBUG("ALSA", "Get hardware parameters from device '%s'", deviceName);
	size_t hardwareParamsSize = alsaApi->snd_pcm_hw_params_sizeof();
	hardwareParams = (snd_pcm_hw_params_t *)fpl__AllocateTemporaryMemory(hardwareParamsSize, 8);
	fplMemoryClear(hardwareParams, hardwareParamsSize);
	if (alsaApi->snd_pcm_hw_params_any(alsaState->pcmDevice, hardwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed getting hardware parameters from device '%s'!", deviceName);
	}
	FPL_LOG_DEBUG("ALSA", "Successfullyy got hardware parameters from device '%s'", deviceName);

	//
	// Access mode (Interleaved MMap or Standard readi/writei)
	//
	alsaState->isUsingMMap = false;
	if (!audioSettings->specific.alsa.noMMap) {
		if (alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_MMAP_INTERLEAVED) == 0) {
			alsaState->isUsingMMap = true;
		} else {
			FPL_LOG_ERROR("ALSA", "Failed setting MMap access mode for device '%s', trying fallback to standard mode!", deviceName);
		}
	}
	if (!alsaState->isUsingMMap) {
		if (alsaApi->snd_pcm_hw_params_set_access(alsaState->pcmDevice, hardwareParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting default access mode for device '%s'!", deviceName);
		}
	}

	fplAudioDeviceFormat internalFormat = fplZeroInit;
	internalFormat.backend = fplAudioBackendType_Alsa;

	//
	// Format
	//

	// Get all supported formats
	size_t formatMaskSize = alsaApi->snd_pcm_format_mask_sizeof();
	snd_pcm_format_mask_t *formatMask = (snd_pcm_format_mask_t *)fpl__AllocateTemporaryMemory(formatMaskSize, 8);
	fplMemoryClear(formatMask, formatMaskSize);
	alsaApi->snd_pcm_hw_params_get_format_mask(hardwareParams, formatMask);

	snd_pcm_format_t foundFormat;
	snd_pcm_format_t preferredFormat = fpl__MapAudioFormatToAlsaFormat(targetFormat->type);
	if (!alsaApi->snd_pcm_format_mask_test(formatMask, preferredFormat)) {
		// The required format is not supported. Try a list of default formats.
		bool isBigEndian = fplIsBigEndian();
		snd_pcm_format_t defaultFormats[] = {
			isBigEndian ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE,
			isBigEndian ? SND_PCM_FORMAT_FLOAT_BE : SND_PCM_FORMAT_FLOAT_LE,
			isBigEndian ? SND_PCM_FORMAT_S32_BE : SND_PCM_FORMAT_S32_LE,
			isBigEndian ? SND_PCM_FORMAT_S24_3BE : SND_PCM_FORMAT_S24_3LE,
			SND_PCM_FORMAT_U8,
		};
		foundFormat = SND_PCM_FORMAT_UNKNOWN;
		for (size_t defaultFormatIndex = 0; defaultFormatIndex < fplArrayCount(defaultFormats); ++defaultFormatIndex) {
			snd_pcm_format_t defaultFormat = defaultFormats[defaultFormatIndex];
			if (alsaApi->snd_pcm_format_mask_test(formatMask, defaultFormat)) {
				foundFormat = defaultFormat;
				break;
			}
		}
	} else {
		foundFormat = preferredFormat;
	}

	fpl__ReleaseTemporaryMemory(formatMask);

	if (foundFormat == SND_PCM_FORMAT_UNKNOWN) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "No supported audio format for device '%s' found!", deviceName);
	}

	if (alsaApi->snd_pcm_hw_params_set_format(alsaState->pcmDevice, hardwareParams, foundFormat) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting PCM format '%s' for device '%s'!", fplGetAudioFormatName(fpl__MapAlsaFormatToAudioFormat(foundFormat)), deviceName);
	}
	internalFormat.type = fpl__MapAlsaFormatToAudioFormat(foundFormat);

	//
	// Channels
	//
	unsigned int internalChannels = targetFormat->channels;
	if (alsaApi->snd_pcm_hw_params_set_channels_near(alsaState->pcmDevice, hardwareParams, &internalChannels) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting PCM channels '%lu' for device '%s'!", internalChannels, deviceName);
	}
	internalFormat.channels = internalChannels;

	//
	// Sample rate
	//

	// @NOTE(final): The caller is responsible to convert to the sample rate FPL expects, so we disable any resampling
	alsaApi->snd_pcm_hw_params_set_rate_resample(alsaState->pcmDevice, hardwareParams, 0);
	unsigned int actualSampleRate = targetFormat->sampleRate;
	fplAssert(actualSampleRate > 0);
	if (alsaApi->snd_pcm_hw_params_set_rate_near(alsaState->pcmDevice, hardwareParams, &actualSampleRate, 0) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting PCM sample rate '%lu' for device '%s'!", actualSampleRate, deviceName);
	}
	internalFormat.sampleRate = actualSampleRate;

	//
	// Buffer size + Scaling
	//
	snd_pcm_uframes_t actualBufferSize;
	if ((targetFormat->defaultFields & fplAudioDefaultFields_BufferSize) == fplAudioDefaultFields_BufferSize) {
		actualBufferSize = fpl__AlsaScaleBufferSize(targetFormat->bufferSizeInFrames, bufferSizeScaleFactor);
	} else {
		actualBufferSize = targetFormat->bufferSizeInFrames;
	}
	fplAssert(actualBufferSize > 0);
	if (alsaApi->snd_pcm_hw_params_set_buffer_size_near(alsaState->pcmDevice, hardwareParams, &actualBufferSize) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting PCM buffer size '%lu' for device '%s'!", actualBufferSize, deviceName);
	}
	internalFormat.bufferSizeInFrames = actualBufferSize;

	uint32_t bufferSizeInBytes = fplGetAudioBufferSizeInBytes(internalFormat.type, internalFormat.channels, internalFormat.bufferSizeInFrames);

	//
	// Periods
	//
	uint32_t internalPeriods = targetFormat->periods;
	int periodsDir = 0;
	if (alsaApi->snd_pcm_hw_params_set_periods_near(alsaState->pcmDevice, hardwareParams, &internalPeriods, &periodsDir) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed setting PCM periods '%lu' for device '%s'!", internalPeriods, deviceName);
	}
	internalFormat.periods = internalPeriods;

	//
	// Hardware parameters
	//
	if (alsaApi->snd_pcm_hw_params(alsaState->pcmDevice, hardwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed to install PCM hardware parameters for device '%s'!", deviceName);
	}

	// Save internal format
	commonAudio->internalFormat = internalFormat;

	//
	// Software parameters
	//
	size_t softwareParamsSize = alsaApi->snd_pcm_sw_params_sizeof();
	softwareParams = (snd_pcm_sw_params_t *)fpl__AllocateTemporaryMemory(softwareParamsSize, 8);
	fplMemoryClear(softwareParams, softwareParamsSize);
	if (alsaApi->snd_pcm_sw_params_current(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed to get software parameters for device '%s'!", deviceName);
	}
	snd_pcm_uframes_t minAvailableFrames = fpl__PrevPowerOfTwo(internalFormat.bufferSizeInFrames / internalFormat.periods);
	if (alsaApi->snd_pcm_sw_params_set_avail_min(alsaState->pcmDevice, softwareParams, minAvailableFrames) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed to set software available min frames of '%lu' for device '%s'!", minAvailableFrames, deviceName);
	}
	if (!alsaState->isUsingMMap) {
		snd_pcm_uframes_t threshold = internalFormat.bufferSizeInFrames / internalFormat.periods;
		if (alsaApi->snd_pcm_sw_params_set_start_threshold(alsaState->pcmDevice, softwareParams, threshold) < 0) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed to set start threshold of '%lu' for device '%s'!", threshold, deviceName);
		}
	}
	if (alsaApi->snd_pcm_sw_params(alsaState->pcmDevice, softwareParams) < 0) {
		FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed to install PCM software parameters for device '%s'!", deviceName);
	}

	if (!alsaState->isUsingMMap) {
		fplAssert(bufferSizeInBytes > 0);
		alsaState->intermediaryBuffer = fpl__AllocateDynamicMemory(bufferSizeInBytes, 16);
		if (alsaState->intermediaryBuffer == fpl_null) {
			FPL__ALSA_INIT_ERROR(fplAudioResultType_Failed, "Failed allocating intermediary buffer of size '%lu' for device '%s'!", bufferSizeInBytes, deviceName);
		}
	}

	// @NOTE(final): We do not support channel mapping right know, so we limit it to mono or stereo
	fplAssert(internalFormat.channels <= 2);

#undef FPL__ALSA_INIT_ERROR

	fpl__ReleaseTemporaryMemory(softwareParams);
	fpl__ReleaseTemporaryMemory(hardwareParams);

	return fplAudioResultType_Success;
}

fpl_internal uint32_t fpl__GetAudioDevicesAlsa(fpl__AlsaAudioState *alsaState, fplAudioDeviceInfo *deviceInfos, uint32_t maxDeviceCount) {
	fplAssert(alsaState != fpl_null);
	const fpl__AlsaAudioApi *alsaApi = &alsaState->api;
	char **ppDeviceHints;
	if (alsaApi->snd_device_name_hint(-1, "pcm", (void ***)&ppDeviceHints) < 0) {
		return 0;
	}
	uint32_t capacityOverflow = 0;
	uint32_t result = 0;
	char **ppNextDeviceHint = ppDeviceHints;
	while (*ppNextDeviceHint != fpl_null) {
		char *name = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "NAME");
		char *ioid = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "IOID");

		// Only allow output or default devices
		if (name != fpl_null && (fplIsStringEqual(name, "default") || fplIsStringEqual(name, "pulse") || fplIsStringEqual(ioid, "Output"))) {
			if (deviceInfos != fpl_null) {
				if (result >= maxDeviceCount) {
					++capacityOverflow;
				} else {
					fplAudioDeviceInfo *outDeviceInfo = deviceInfos + result;
					fplClearStruct(outDeviceInfo);
					fplCopyString(name, outDeviceInfo->id.alsa, fplArrayCount(outDeviceInfo->id.alsa));
					char *desc = alsaApi->snd_device_name_get_hint(*ppNextDeviceHint, "DESC");
					if (desc != fpl_null) {
						fplCopyString(desc, outDeviceInfo->name, fplArrayCount(outDeviceInfo->name));
						free(desc);
					} else {
						fplCopyString(name, outDeviceInfo->name, fplArrayCount(outDeviceInfo->name));
					}
					++result;
				}
			} else {
				++result;
			}
		}
		if (ioid != fpl_null) {
			free(ioid);
		}
		if (name != fpl_null) {
			free(name);
		}
		++ppNextDeviceHint;
	}
	alsaApi->snd_device_name_free_hint((void **)ppDeviceHints);
	if (capacityOverflow > 0) {
		FPL__ERROR(FPL__MODULE_AUDIO_ALSA, "Capacity of '%lu' for audio device infos has been reached. '%lu' audio devices are not included in the result", maxDeviceCount, capacityOverflow);
	}
	return(result);
}

#endif // FPL__ENABLE_AUDIO_ALSA

#endif // FPL__AUDIO_BACKENDS_IMPLEMENTED

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_AUDIO_L1 (Audio System, Private Implementation)
//
// The audio system is based on a stripped down version of "mini_al.h" by David Reid.
// See: https://github.com/dr-soft/mini_al
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL__ENABLE_AUDIO)

#define FPL__AUDIO_RESULT_TYPE_COUNT FPL__ENUM_COUNT(fplAudioResultType_First, fplAudioResultType_Last)
fpl_globalvar const char *fpl__global_audioResultTypeNameTable[] = {
	"None", // fplAudioResultType_None = 0,
	"Success", // fplAudioResultType_Success,
	"Audio-Device not initialized",// fplAudioResultType_DeviceNotInitialized,
	"Audio-Device already stopped",// fplAudioResultType_DeviceAlreadyStopped,
	"Audio-Device already started",// fplAudioResultType_DeviceAlreadyStarted,
	"Audio-Device is busy", // fplAudioResultType_DeviceBusy,
	"No Audio-Device found", // fplAudioResultType_NoDeviceFound,
	"Api failure", // fplAudioResultType_ApiFailed,
	"Platform not initialized", // fplAudioResultType_PlatformNotInitialized,
	"Backend already initialized", // fplAudioResultType_BackendAlreadyInitialized,
	"Audio format was not set", // fplAudioResultType_UnsetAudioFormat,
	"Number of audio channels was not set", // fplAudioResultType_UnsetAudioChannels,
	"Audio sample rate was not set", // fplAudioResultType_UnsetAudioSampleRate,
	"Audio buffer sizes was not set", // fplAudioResultType_UnsetAudioBufferSize,
	"Unknown audio failure", // fplAudioResultType_Failed,
};
fplStaticAssert(fplArrayCount(fpl__global_audioResultTypeNameTable) == FPL__AUDIO_RESULT_TYPE_COUNT);

fpl_common_api const char *fplGetAudioResultName(const fplAudioResultType type) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(type, fplAudioResultType_First, fplAudioResultType_Last);
	const char *result = fpl__global_audioResultTypeNameTable[index];
	return(result);
}

typedef struct fpl__AudioEvent {
	fplMutexHandle mutex;
	fplConditionVariable cond;
	volatile int32_t signaled;
} fpl__AudioEvent;

fpl_internal bool fpl__InitAudioEvent(fpl__AudioEvent *ev) {
	fplClearStruct(ev);
	if (!fplMutexInit(&ev->mutex)) {
		return(false);
	}
	if (!fplConditionInit(&ev->cond)) {
		return(false);
	}
	ev->signaled = 0;
	return(true);
}

fpl_internal void fpl__ReleaseAudioEvent(fpl__AudioEvent *ev) {
	fplConditionDestroy(&ev->cond);
	fplMutexDestroy(&ev->mutex);
}

fpl_internal void fpl__WaitForAudioEvent(fpl__AudioEvent *ev) {
	fplMutexLock(&ev->mutex);
	while (!ev->signaled) {
		fplConditionWait(&ev->cond, &ev->mutex, FPL_TIMEOUT_INFINITE);
	}
	ev->signaled = 0;
	fplMutexUnlock(&ev->mutex);
}

fpl_internal void fpl__SetAudioEvent(fpl__AudioEvent *ev) {
	fplMutexLock(&ev->mutex);
	ev->signaled = 1;
	fplConditionSignal(&ev->cond);
	fplMutexUnlock(&ev->mutex);
}

typedef struct fpl__AudioState {
	fpl__CommonAudioState common;

	fplMutexHandle lock;
	fplThreadHandle *workerThread;
	fpl__AudioEvent startEvent;
	fpl__AudioEvent stopEvent;
	fpl__AudioEvent wakeupEvent;
	volatile fplAudioResultType workResult;

	fplAudioBackendType backendType;
	bool isAsyncBackend;

	union {
#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		fpl__DirectSoundAudioState dsound;
#	endif
#	if defined(FPL__ENABLE_AUDIO_ALSA)
		fpl__AlsaAudioState alsa;
#	endif
	};
} fpl__AudioState;

fpl_internal fpl__AudioState *fpl__GetAudioState(fpl__PlatformAppState *appState) {
	fplAssert(appState != fpl_null);
	fpl__AudioState *result = fpl_null;
	if (appState->audio.mem != fpl_null) {
		result = (fpl__AudioState *)appState->audio.mem;
	}
	return(result);
}

fpl_internal void fpl__StopAudioDeviceMainLoop(fpl__AudioState *audioState) {
	fplAssert(audioState->backendType > fplAudioBackendType_Auto);
	switch (audioState->backendType) {

#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioBackendType_DirectSound:
		{
			fpl__AudioStopMainLoopDirectSound(&audioState->dsound);
		} break;
#	endif

#	if defined(FPL__ENABLE_AUDIO_ALSA)
		case fplAudioBackendType_Alsa:
		{
			fpl__AudioStopMainLoopAlsa(&audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal bool fpl__ReleaseAudioDevice(fpl__AudioState *audioState) {
	fplAssert(audioState->backendType > fplAudioBackendType_Auto);
	bool result = false;
	switch (audioState->backendType) {

#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioBackendType_DirectSound:
		{
			result = fpl__AudioReleaseDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL__ENABLE_AUDIO_ALSA)
		case fplAudioBackendType_Alsa:
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
	fplAssert(audioState->backendType > fplAudioBackendType_Auto);
	bool result = false;
	switch (audioState->backendType) {

#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioBackendType_DirectSound:
		{
			result = fpl__AudioStopDirectSound(&audioState->dsound);
		} break;
#	endif

#	if defined(FPL__ENABLE_AUDIO_ALSA)
		case fplAudioBackendType_Alsa:
		{
			result = fpl__AudioStopAlsa(&audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
	return (result);
}

fpl_internal fplAudioResultType fpl__StartAudioDevice(fpl__AudioState *audioState) {
	fplAssert(audioState->backendType > fplAudioBackendType_Auto);
	fplAudioResultType result = fplAudioResultType_Failed;
	switch (audioState->backendType) {

#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioBackendType_DirectSound:
		{
			result = fpl__AudioStartDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL__ENABLE_AUDIO_ALSA)
		case fplAudioBackendType_Alsa:
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
	fplAssert(audioState->backendType > fplAudioBackendType_Auto);
	switch (audioState->backendType) {

#	if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
		case fplAudioBackendType_DirectSound:
		{
			fpl__AudioRunMainLoopDirectSound(&audioState->common, &audioState->dsound);
		} break;
#	endif

#	if defined(FPL__ENABLE_AUDIO_ALSA)
		case fplAudioBackendType_Alsa:
		{
			fpl__AudioRunMainLoopAlsa(&audioState->common, &audioState->alsa);
		} break;
#	endif

		default:
			break;
	}
}

fpl_internal bool fpl__IsAudioBackendAsync(const fplAudioBackendType backendType) {
	switch (backendType) {
		case fplAudioBackendType_DirectSound:
		case fplAudioBackendType_Alsa:
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
#if defined(FPL_PLATFORM_WINDOWS)
	fplAssert(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	fpl__AudioState *audioState = (fpl__AudioState *)data;
	fpl__CommonAudioState *commonAudioState = &audioState->common;
	fplAssert(audioState != fpl_null);
	fplAssert(audioState->backendType != fplAudioBackendType_None);

#if defined(FPL_PLATFORM_WINDOWS)
	wapi->ole.CoInitializeEx(fpl_null, 0);
#endif

	for (;;) {
		// Stop the device at the start of the iteration always
		fpl__StopAudioDevice(audioState);

		// Let the other threads know that the device has been stopped.
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Stopped);
		fpl__SetAudioEvent(&audioState->stopEvent);

		// We wait until the audio device gets wake up
		fpl__WaitForAudioEvent(&audioState->wakeupEvent);

		// Default result code.
		audioState->workResult = fplAudioResultType_Success;

		// Just break if we're terminating.
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Uninitialized) {
			break;
		}

		// Expect that the device is currently be started by the client
		fplAssert(fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting);

		// Start audio device
		audioState->workResult = fpl__StartAudioDevice(audioState);
		if (audioState->workResult != fplAudioResultType_Success) {
			fpl__SetAudioEvent(&audioState->startEvent);
			continue;
		}

		// The audio device is started, mark it as such
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Started);
		fpl__SetAudioEvent(&audioState->startEvent);

		// Enter audio device main loop
		fpl__RunAudioDeviceMainLoop(audioState);
	}

	// Signal to stop any audio threads, in case there are some waiting
	fpl__SetAudioEvent(&audioState->stopEvent);

#if defined(FPL_PLATFORM_WINDOWS)
	wapi->ole.CoUninitialize();
#endif
}

fpl_internal void fpl__ReleaseAudio(fpl__AudioState *audioState) {
	fplAssert(audioState != fpl_null);

#if defined(FPL_PLATFORM_WINDOWS)
	fplAssert(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (fpl__IsAudioDeviceInitialized(commonAudioState)) {

		// Wait until the audio device is stopped
		if (fpl__IsAudioDeviceStarted(commonAudioState)) {
			while (fplStopAudio() == fplAudioResultType_DeviceBusy) {
				fplThreadSleep(1);
			}
		}

		// Putting the device into an uninitialized state will make the worker thread return.
		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Uninitialized);

		// Wake up the worker thread and wait for it to properly terminate.
		fpl__SetAudioEvent(&audioState->wakeupEvent);

		fplThreadWaitForOne(audioState->workerThread, FPL_TIMEOUT_INFINITE);
		fplThreadTerminate(audioState->workerThread);

		// Release signals and thread
		fpl__ReleaseAudioEvent(&audioState->stopEvent);
		fpl__ReleaseAudioEvent(&audioState->startEvent);
		fpl__ReleaseAudioEvent(&audioState->wakeupEvent);
		fplMutexDestroy(&audioState->lock);

		// Release audio device
		fpl__ReleaseAudioDevice(audioState);

		// Clear audio state
		fplClearStruct(audioState);
	}

#if defined(FPL_PLATFORM_WINDOWS)
	wapi->ole.CoUninitialize();
#endif
}

fpl_internal fplAudioResultType fpl__InitAudio(const fplAudioSettings *audioSettings, fpl__AudioState *audioState) {
	fplAssert(audioState != fpl_null);

#if defined(FPL_PLATFORM_WINDOWS)
	fplAssert(fpl__global__AppState != fpl_null);
	const fpl__Win32Api *wapi = &fpl__global__AppState->win32.winApi;
#endif

	if (audioState->backendType != fplAudioBackendType_None) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResultType_BackendAlreadyInitialized;
	}

	fplAudioDeviceFormat actualTargetFormat = fplZeroInit;
	fplConvertAudioTargetFormatToDeviceFormat(&audioSettings->targetFormat, &actualTargetFormat);

	audioState->common.clientReadCallback = audioSettings->clientReadCallback;
	audioState->common.clientUserData = audioSettings->userData;

#if defined(FPL_PLATFORM_WINDOWS)
	wapi->ole.CoInitializeEx(fpl_null, 0);
#endif

	// Create mutex and signals
	if (!fplMutexInit(&audioState->lock)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResultType_Failed;
	}
	if (!fpl__InitAudioEvent(&audioState->wakeupEvent)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResultType_Failed;
	}
	if (!fpl__InitAudioEvent(&audioState->startEvent)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResultType_Failed;
	}
	if (!fpl__InitAudioEvent(&audioState->stopEvent)) {
		fpl__ReleaseAudio(audioState);
		return fplAudioResultType_Failed;
	}

	// Prope backends
	fplAudioBackendType propeBackendTypes[16];
	uint32_t backendCount = 0;
	if (audioSettings->backend == fplAudioBackendType_Auto) {
		// @NOTE(final): Add all audio backends here, regardless of the platform.
		propeBackendTypes[backendCount++] = fplAudioBackendType_DirectSound;
		propeBackendTypes[backendCount++] = fplAudioBackendType_Alsa;
	} else {
		// @NOTE(final): Forced audio backend
		propeBackendTypes[backendCount++] = audioSettings->backend;
	}
	fplAudioResultType initResult = fplAudioResultType_Failed;
	for (uint32_t backendIndex = 0; backendIndex < backendCount; ++backendIndex) {
		fplAudioBackendType propeBackendType = propeBackendTypes[backendIndex];

		initResult = fplAudioResultType_Failed;
		switch (propeBackendType) {
#		if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioBackendType_DirectSound:
			{
				initResult = fpl__AudioInitDirectSound(audioSettings, &actualTargetFormat, &audioState->common, &audioState->dsound);
				if (initResult != fplAudioResultType_Success) {
					fpl__AudioReleaseDirectSound(&audioState->common, &audioState->dsound);
				}
			} break;
#		endif

#		if defined(FPL__ENABLE_AUDIO_ALSA)
			case fplAudioBackendType_Alsa:
			{
				initResult = fpl__AudioInitAlsa(audioSettings, &actualTargetFormat, &audioState->common, &audioState->alsa);
				if (initResult != fplAudioResultType_Success) {
					fpl__AudioReleaseAlsa(&audioState->common, &audioState->alsa);
				}
			} break;
#		endif

			default:
				break;
		}
		if (initResult == fplAudioResultType_Success) {
			audioState->backendType = propeBackendType;
			audioState->isAsyncBackend = fpl__IsAudioBackendAsync(propeBackendType);
			break;
		}
	}

	if (initResult != fplAudioResultType_Success) {
		fpl__ReleaseAudio(audioState);
		return initResult;
	}

	if (!audioState->isAsyncBackend) {
		// Create and start worker thread
		fplThreadParameters audioThreadParams = fplZeroInit;
		audioThreadParams.priority = fplThreadPriority_RealTime;
		audioThreadParams.runFunc = fpl__AudioWorkerThread;
		audioThreadParams.userData = audioState;
		audioState->workerThread = fplThreadCreateWithParameters(&audioThreadParams);
		if (audioState->workerThread == fpl_null) {
			fpl__ReleaseAudio(audioState);
			return fplAudioResultType_Failed;
		}
		// Change to realtime thread
		fplSetThreadPriority(audioState->workerThread, fplThreadPriority_RealTime);
		// Wait for the worker thread to put the device into the stopped state.
		fpl__WaitForAudioEvent(&audioState->stopEvent);
	} else {
		fpl__AudioSetDeviceState(&audioState->common, fpl__AudioDeviceState_Stopped);
	}

	fplAssert(fpl__AudioGetDeviceState(&audioState->common) == fpl__AudioDeviceState_Stopped);

	return(fplAudioResultType_Success);
}
#endif // FPL__ENABLE_AUDIO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_VIDEO_L1 (Video System, Private Implementation)
//
// The audio system is based on a stripped down version of "mini_al.h" by David Reid.
// See: https://github.com/dr-soft/mini_al
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL__ENABLE_VIDEO)

typedef union fpl__ActiveVideoBackend {
	fpl__VideoBackend base;

#if defined(FPL_PLATFORM_WINDOWS)
#	if defined(FPL__ENABLE_VIDEO_OPENGL)
	fpl__VideoBackendWin32OpenGL win32_opengl;
#	endif
#	if defined(FPL__ENABLE_VIDEO_SOFTWARE)
	fpl__VideoBackendWin32Software win32_software;
#	endif
#endif

#if defined(FPL_SUBPLATFORM_X11)
#	if defined(FPL__ENABLE_VIDEO_OPENGL)
	fpl__VideoBackendX11OpenGL x11_opengl;
#	endif
#	if defined(FPL__ENABLE_VIDEO_SOFTWARE)
	fpl__VideoBackendX11Software x11_software;
#	endif
#endif

#if defined(FPL__ENABLE_VIDEO_VULKAN)
	fpl__VideoBackendVulkan vulkan;
#endif

} fpl__ActiveVideoBackend;

typedef struct fpl__VideoState {
	fpl__VideoContext context;
	fpl__VideoData data;
	fplVideoBackendType backendType;
	fpl__ActiveVideoBackend activeBackend;
} fpl__VideoState;

fpl_internal fpl__VideoState *fpl__GetVideoState(fpl__PlatformAppState *appState) {
	fplAssert(appState != fpl_null);
	fpl__VideoState *result = fpl_null;
	if (appState->video.mem != fpl_null) {
		result = (fpl__VideoState *)appState->video.mem;
	}
	return(result);
}

fpl_internal void fpl__DestroySurfaceBackend(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	const fpl__VideoContext *ctx = &videoState->context;
	fplAssert(ctx->destroyedWindowFunc != fpl_null);
	ctx->destroyedWindowFunc(appState, &videoState->activeBackend.base);
}

fpl_internal void fpl__UnloadVideoBackend(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	const fpl__VideoContext *ctx = &videoState->context;
	fplAssert(ctx->unloadFunc != fpl_null);
	ctx->unloadFunc(appState, &videoState->activeBackend.base);
	fplClearStruct(videoState);
}

fpl_internal bool fpl__LoadVideoBackend(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	const fpl__VideoContext *ctx = &videoState->context;
	fplAssert(ctx->loadFunc != fpl_null);
	bool result = ctx->loadFunc(appState, &videoState->activeBackend.base);
	return(result);
}

fpl_internal void fpl__ShutdownVideoBackend(fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	fplAssert(appState != fpl_null);
	if (videoState != fpl_null) {
		const fpl__VideoContext *ctx = &videoState->context;
		fplAssert(ctx->shutdownFunc != fpl_null);
		ctx->shutdownFunc(appState, &appState->window, &videoState->activeBackend.base);

#	if defined(FPL__ENABLE_VIDEO_SOFTWARE)
		fplVideoBackBuffer *backbuffer = &videoState->data.backbuffer;
		if (backbuffer->pixels != fpl_null) {
			fpl__ReleaseDynamicMemory(backbuffer->pixels);
		}
		fplClearStruct(backbuffer);
#	endif
	}
}

fpl_internal bool fpl__InitializeVideoBackend(const fplVideoBackendType backendType, const fplVideoSettings *videoSettings, const uint32_t windowWidth, const uint32_t windowHeight, fpl__PlatformAppState *appState, fpl__VideoState *videoState) {
	// @NOTE(final): video backends are platform independent, so we cannot have to same system as audio.
	fplAssert(appState != fpl_null);
	fplAssert(videoState != fpl_null);

	const fpl__VideoContext *ctx = &videoState->context;

	// Allocate backbuffer context if needed
#	if defined(FPL__ENABLE_VIDEO_SOFTWARE)
	if (backendType == fplVideoBackendType_Software) {
		fplVideoBackBuffer *backbuffer = &videoState->data.backbuffer;
		backbuffer->width = windowWidth;
		backbuffer->height = windowHeight;
		backbuffer->pixelStride = sizeof(uint32_t);
		backbuffer->lineWidth = backbuffer->width * backbuffer->pixelStride;
		size_t size = backbuffer->lineWidth * backbuffer->height;
		backbuffer->pixels = (uint32_t *)fpl__AllocateDynamicMemory(size, 4);
		if (backbuffer->pixels == fpl_null) {
			FPL__ERROR(FPL__MODULE_VIDEO_SOFTWARE, "Failed allocating video software backbuffer of size %xu bytes", size);
			fpl__ShutdownVideoBackend(appState, videoState);
			return false;
		}

		// Clear to black by default
		// @NOTE(final): Bitmap is top-down, 0xAABBGGRR
		uint32_t *p = backbuffer->pixels;
		uint32_t color = appState->initSettings.window.background.value == 0 ? 0xFF000000 : appState->initSettings.window.background.value;
		for (uint32_t y = 0; y < backbuffer->height; ++y) {
			for (uint32_t x = 0; x < backbuffer->width; ++x) {
				*p++ = color;
			}
		}
	}
#	endif // FPL__ENABLE_VIDEO_SOFTWARE

	fplAssert(ctx->initializeFunc != fpl_null);
	bool videoInitResult = ctx->initializeFunc(appState, &appState->window, videoSettings, &videoState->data, &videoState->activeBackend.base);
	if (!videoInitResult) {
		fplAssert(fplGetErrorCount() > 0);
		fpl__ShutdownVideoBackend(appState, videoState);
		return false;
	}

	return true;
}

fpl_internal fpl__VideoContext fpl__ConstructVideoContext(const fplVideoBackendType backendType) {
	switch (backendType) {
#if defined(FPL__ENABLE_VIDEO_OPENGL)
		case fplVideoBackendType_OpenGL:
		{
#	if defined(FPL_PLATFORM_WINDOWS)
			return fpl__VideoBackend_Win32OpenGL_Construct();
#	elif defined(FPL_SUBPLATFORM_X11)
			return fpl__VideoBackend_X11OpenGL_Construct();
#	endif
		} break;
#endif

#if defined(FPL__ENABLE_VIDEO_VULKAN)
		case fplVideoBackendType_Vulkan:
		{
			return fpl__VideoBackend_Vulkan_Construct();
		} break;
#endif

#if defined(FPL__ENABLE_VIDEO_SOFTWARE)
		case fplVideoBackendType_Software:
		{
#	if defined(FPL_PLATFORM_WINDOWS)
			return fpl__VideoBackend_Win32Software_Construct();
#	elif defined(FPL_SUBPLATFORM_X11)
			return fpl__VideoBackend_X11Software_Construct();
#	endif
		} break;
#endif

		default:
		{
			// No backend found, just return a stub
			FPL__ERROR(FPL__MODULE_VIDEO, "The video backend '%s' is not supported for this platform", fplGetVideoBackendName(backendType));
			return(fpl__StubVideoContext());
		} break;
	}
}
#endif // FPL__ENABLE_VIDEO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_WINDOW (Window System, Private Implementation)
//
// - Init window
// - Release window
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL__ENABLE_WINDOW)
fpl_internal FPL__FUNC_PREPARE_VIDEO_WINDOW(fpl__PrepareVideoWindowDefault) {
	fplAssert(appState != fpl_null);

#	if defined(FPL__ENABLE_VIDEO)
	if (initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState->context.prepareWindowFunc != fpl_null) {
			bool result = videoState->context.prepareWindowFunc(appState, &initSettings->video, &appState->window, &videoState->activeBackend.base);
			return(result);
		}
	}
#	endif // FPL__ENABLE_VIDEO

	return(true);
}

fpl_internal FPL__FUNC_FINALIZE_VIDEO_WINDOW(fpl__FinalizeVideoWindowDefault) {
	fplAssert(appState != fpl_null);

#if defined(FPL__ENABLE_VIDEO)
	if (initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState->context.finalizeWindowFunc != fpl_null) {
			bool result = videoState->context.finalizeWindowFunc(appState, &initSettings->video, &appState->window, &videoState->activeBackend.base);
			return(result);
		}
	}
#endif // FPL__ENABLE_VIDEO

	return true;
}

fpl_internal void fpl__ReleaseWindow(const fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	if (appState != fpl_null) {
#	if defined(FPL_PLATFORM_WINDOWS)
		fpl__Win32ReleaseWindow(&initState->win32, &appState->win32, &appState->window.win32);
#	elif defined(FPL_SUBPLATFORM_X11)
		fpl__X11ReleaseWindow(&appState->x11, &appState->window.x11);
#	endif
	}
}

fpl_internal bool fpl__InitWindow(const fplSettings *initSettings, fplWindowSettings *currentWindowSettings, fpl__PlatformAppState *appState, const fpl__SetupWindowCallbacks *setupCallbacks) {
	bool result = false;
	if (appState != fpl_null) {
#	if defined(FPL_PLATFORM_WINDOWS)
		result = fpl__Win32InitWindow(initSettings, currentWindowSettings, appState, &appState->win32, &appState->window.win32, setupCallbacks);
#	elif defined(FPL_SUBPLATFORM_X11)
		result = fpl__X11InitWindow(initSettings, currentWindowSettings, appState, &appState->x11, &appState->window.x11, setupCallbacks);
#	endif
	}
	return (result);
}
#endif // FPL__ENABLE_WINDOW

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_AUDIO_L2 (Audio System, Public Implementation)
//
// - Stop audio
// - Play audio
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL__ENABLE_AUDIO)

#define FPL__AUDIOFORMATTYPE_COUNT FPL__ENUM_COUNT(fplAudioFormatType_First, fplAudioFormatType_Last)

fpl_globalvar uint32_t fpl__globalAudioFormatSampleSizeTable[] = {
	0, // No audio format
	1, // Unsigned 8-bit integer PCM
	2, // Signed 16-bit integer PCM
	3, // Signed 24-bit integer PCM
	4, // Signed 32-bit integer PCM
	8, // Signed 64-bit integer PCM
	4, // 32-bit IEEE_FLOAT
	8, // 64-bit IEEE_FLOAT
};
fplStaticAssert(fplArrayCount(fpl__globalAudioFormatSampleSizeTable) == FPL__AUDIOFORMATTYPE_COUNT);

fpl_globalvar const char *fpl__globalAudioFormatNameTable[] = {
	"None", // 0 = No audio format
	"U8",	// = Unsigned 8-bit integer PCM
	"S16",	// = Signed 16-bit integer PCM
	"S24",	// = Signed 24-bit integer PCM
	"S32",	// = Signed 32-bit integer PCM
	"S64",  // = Signed 64-bit integer PCM
	"F32",	// = 32-bit IEEE_FLOAT
	"F64",	// = 64-bit IEEE_FLOAT
};
fplStaticAssert(fplArrayCount(fpl__globalAudioFormatNameTable) == FPL__AUDIOFORMATTYPE_COUNT);

fpl_common_api uint32_t fplGetAudioSampleSizeInBytes(const fplAudioFormatType format) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(format, fplAudioFormatType_First, fplAudioFormatType_Last);
	uint32_t result = fpl__globalAudioFormatSampleSizeTable[index];
	return(result);
}

fpl_common_api const char *fplGetAudioFormatName(const fplAudioFormatType format) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(format, fplAudioFormatType_First, fplAudioFormatType_Last);
	const char *result = fpl__globalAudioFormatNameTable[index];
	return(result);
}

#define FPL__AUDIOBACKENDTYPE_COUNT FPL__ENUM_COUNT(fplAudioBackendType_First, fplAudioBackendType_Last)
fpl_globalvar const char *fpl__globalAudioBackendNameTable[] = {
	"None", // No audio backend
	"Automatic", // Automatic backend detection
	"DirectSound", // DirectSound
	"ALSA", // Alsa
};
fplStaticAssert(fplArrayCount(fpl__globalAudioBackendNameTable) == FPL__AUDIOBACKENDTYPE_COUNT);

fpl_common_api fplAudioBackendType fplGetAudioBackendType() {
	FPL__CheckPlatform(fplAudioBackendType_None);
	const fpl__PlatformAppState *appState = fpl__global__AppState;
	fplAudioBackendType result = appState->currentSettings.audio.backend;
	return(result);
}

fpl_common_api const char *fplGetAudioBackendName(fplAudioBackendType backendType) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(backendType, fplAudioBackendType_First, fplAudioBackendType_Last);
	const char *result = fpl__globalAudioBackendNameTable[index];
	return(result);
}

fpl_common_api uint32_t fplGetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds) {
	if (sampleRate == 0 || bufferSizeInMilliSeconds == 0) return(0);
	uint32_t result = bufferSizeInMilliSeconds * sampleRate / 1000UL;
	return(result);
}

fpl_common_api uint32_t fplGetAudioBufferSizeInMilliseconds(uint32_t sampleRate, uint32_t frameCount) {
	if (sampleRate == 0 || frameCount == 0) return(0);
	uint32_t result = frameCount * 1000UL / sampleRate;
	return(result);
}

fpl_common_api uint32_t fplGetAudioFrameSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount) {
	if (channelCount == 0) return(0);
	uint32_t result = fplGetAudioSampleSizeInBytes(format) * channelCount;
	return(result);
}

fpl_common_api uint32_t fplGetAudioBufferSizeInBytes(const fplAudioFormatType format, const uint32_t channelCount, const uint32_t frameCount) {
	if (channelCount == 0 || frameCount == 0) return(0);
	uint32_t frameSize = fplGetAudioFrameSizeInBytes(format, channelCount);
	uint32_t result = frameSize * frameCount;
	return(result);
}

fpl_common_api void fplConvertAudioTargetFormatToDeviceFormat(const fplAudioTargetFormat *inFormat, fplAudioDeviceFormat *outFormat) {
	FPL__CheckArgumentNullNoRet(inFormat);
	FPL__CheckArgumentNullNoRet(outFormat);

	fplClearStruct(outFormat);

	// Channels
	if (inFormat->channels > 0) {
		outFormat->channels = inFormat->channels;
	} else {
		outFormat->channels = FPL__DEFAULT_AUDIO_CHANNELS;
		outFormat->defaultFields |= fplAudioDefaultFields_Channels;
	}

	// Sample rate
	if (inFormat->sampleRate > 0) {
		outFormat->sampleRate = inFormat->sampleRate;
	} else {
		outFormat->sampleRate = FPL__DEFAULT_AUDIO_SAMPLERATE;
		outFormat->defaultFields |= fplAudioDefaultFields_SampleRate;
	}

	// Format
	if (outFormat->type != fplAudioFormatType_None) {
		outFormat->type = inFormat->type;
	} else {
		outFormat->type = FPL__DEFAULT_AUDIO_FORMAT;
		outFormat->defaultFields |= fplAudioDefaultFields_Type;
	}

	// Periods
	if (outFormat->periods > 0) {
		outFormat->periods = inFormat->periods;
	} else {
		outFormat->periods = FPL__DEFAULT_AUDIO_PERIODS;
		outFormat->defaultFields |= fplAudioDefaultFields_Periods;
	}

	// Buffer size
	if (inFormat->bufferSizeInFrames > 0) {
		outFormat->bufferSizeInFrames = inFormat->bufferSizeInFrames;
	} else if (inFormat->bufferSizeInMilliseconds > 0) {
		outFormat->bufferSizeInFrames = fplGetAudioBufferSizeInFrames(inFormat->sampleRate, inFormat->bufferSizeInMilliseconds);
	} else {
		uint32_t bufferSizeInMilliseconds = (inFormat->latencyMode == fplAudioLatencyMode_Conservative) ? FPL__DEFAULT_AUDIO_BUFFERSIZE_CONSERVATIVE_IN_MSECS : FPL__DEFAULT_AUDIO_BUFFERSIZE_LOWLATENCY_IN_MSECS;
		outFormat->bufferSizeInFrames = fplGetAudioBufferSizeInFrames(inFormat->sampleRate, bufferSizeInMilliseconds);
		outFormat->defaultFields |= fplAudioDefaultFields_BufferSize;
	}

	// Exclusive mode
	outFormat->preferExclusiveMode = inFormat->preferExclusiveMode;
}

fpl_common_api fplAudioResultType fplStopAudio() {
	FPL__CheckPlatform(fplAudioResultType_PlatformNotInitialized);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return fplAudioResultType_Failed;
	}

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (!fpl__IsAudioDeviceInitialized(commonAudioState)) {
		return fplAudioResultType_DeviceNotInitialized;
	}

	fpl__AudioDeviceState firstDeviceState = fpl__AudioGetDeviceState(commonAudioState);
	if (firstDeviceState == fpl__AudioDeviceState_Stopped) {
		return fplAudioResultType_Success;
	}

	fplAudioResultType result = fplAudioResultType_Failed;
	fplMutexLock(&audioState->lock);
	{
		// Check if the device is already stopped
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopping) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceAlreadyStopped;
		}
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceAlreadyStopped;
		}

		// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Stopping);

		if (audioState->isAsyncBackend) {
			// Asynchronous backends (Has their own thread)
			fpl__StopAudioDevice(audioState);
		} else {
			// Synchronous backends

			// The audio worker thread is most likely in a wait state, so let it stop properly.
			fpl__StopAudioDeviceMainLoop(audioState);

			// We need to wait for the worker thread to become available for work before returning.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the stopped state.
			fpl__WaitForAudioEvent(&audioState->stopEvent);
			result = fplAudioResultType_Success;
		}
	}
	fplMutexUnlock(&audioState->lock);

	return result;
}

fpl_common_api fplAudioResultType fplPlayAudio() {
	FPL__CheckPlatform(fplAudioResultType_PlatformNotInitialized);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return fplAudioResultType_Failed;
	}

	fpl__CommonAudioState *commonAudioState = &audioState->common;

	if (!fpl__IsAudioDeviceInitialized(commonAudioState)) {
		return fplAudioResultType_DeviceNotInitialized;
	}

	if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Started) {
		return fplAudioResultType_Success;
	}

	fplAudioResultType result = fplAudioResultType_Failed;
	fplMutexLock(&audioState->lock);
	{
		// If device is already in started/starting state we cannot start playback of it
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Starting) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceAlreadyStarted;
		}
		if (fpl__AudioGetDeviceState(commonAudioState) == fpl__AudioDeviceState_Started) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceAlreadyStarted;
		}

		// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
		if (fpl__AudioGetDeviceState(commonAudioState) != fpl__AudioDeviceState_Stopped) {
			fplMutexUnlock(&audioState->lock);
			return fplAudioResultType_DeviceBusy;
		}

		fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Starting);

		if (audioState->isAsyncBackend) {
			// Asynchronous backends (Has their own thread)
			fpl__StartAudioDevice(audioState);
			fpl__AudioSetDeviceState(commonAudioState, fpl__AudioDeviceState_Started);
		} else {
			// Synchronous backends
			fpl__SetAudioEvent(&audioState->wakeupEvent);

			// Wait for the worker thread to finish starting the device.
			// @NOTE(final): The audio worker thread will be the one who puts the device into the started state.
			fpl__WaitForAudioEvent(&audioState->startEvent);
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
	if (audioState != fpl_null) {
		fplClearStruct(outFormat);
		*outFormat = audioState->common.internalFormat;
		return true;
	}
	return false;
}

fpl_common_api bool fplSetAudioClientReadCallback(fpl_audio_client_read_callback *newCallback, void *userData) {
	FPL__CheckPlatform(false);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if ((audioState != fpl_null) && (audioState->backendType > fplAudioBackendType_Auto)) {
		if (fpl__AudioGetDeviceState(&audioState->common) == fpl__AudioDeviceState_Stopped) {
			audioState->common.clientReadCallback = newCallback;
			audioState->common.clientUserData = userData;
			return true;
		}
	}
	return false;
}

fpl_common_api uint32_t fplGetAudioDevices(fplAudioDeviceInfo *devices, uint32_t maxDeviceCount) {
	if (devices != fpl_null) {
		FPL__CheckArgumentZero(maxDeviceCount, 0);
	}
	FPL__CheckPlatform(0);
	fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
	if (audioState == fpl_null) {
		return 0;
	}
	uint32_t result = 0;
	if (audioState->backendType > fplAudioBackendType_Auto) {
		switch (audioState->backendType) {
#		if defined(FPL__ENABLE_AUDIO_DIRECTSOUND)
			case fplAudioBackendType_DirectSound:
			{
				result = fpl__GetAudioDevicesDirectSound(&audioState->dsound, devices, maxDeviceCount);
			} break;
#		endif

#		if defined(FPL__ENABLE_AUDIO_ALSA)
			case fplAudioBackendType_Alsa:
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
#endif // FPL__ENABLE_AUDIO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_VIDEO_L2 (Video System, Public Implementation)
//
// - Video Backbuffer Access
// - Frame flipping
// - Utiltity functions
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if defined(FPL__ENABLE_VIDEO)
#define FPL__VIDEOBACKENDTYPE_COUNT FPL__ENUM_COUNT(fplVideoBackendType_First, fplVideoBackendType_Last)
fpl_globalvar const char *fpl__globalVideoBackendNameTable[FPL__VIDEOBACKENDTYPE_COUNT] = {
	"None", // fplVideoBackendType_None
	"Software", // fplVideoBackendType_Software
	"OpenGL", // fplVideoBackendType_OpenGL
	"Vulkan", // fplVideoBackendType_Vulkan
};

fpl_common_api const char *fplGetVideoBackendName(fplVideoBackendType backendType) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(backendType, fplVideoBackendType_First, fplVideoBackendType_Last);
	const char *result = fpl__globalVideoBackendNameTable[index];
	return(result);
}

fpl_common_api fplVideoBackendType fplGetVideoBackendType() {
	FPL__CheckPlatform(fplVideoBackendType_None);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	if (videoState != fpl_null) {
		return(videoState->backendType);
	}
	return(fplVideoBackendType_None);
}

fpl_common_api fplVideoBackBuffer *fplGetVideoBackBuffer() {
	FPL__CheckPlatform(fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	fplVideoBackBuffer *result = fpl_null;
	if (videoState != fpl_null) {
#if defined(FPL__ENABLE_VIDEO_SOFTWARE)
		if (appState->currentSettings.video.backend == fplVideoBackendType_Software) {
			result = &videoState->data.backbuffer;
		}
#endif
	}
	return(result);
}

fpl_common_api bool fplResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
	FPL__CheckPlatform(false);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	fpl__VideoState *videoState = fpl__GetVideoState(appState);
	bool result = false;
	if (videoState != fpl_null) {
		fplVideoBackendType backendType = videoState->backendType;
		if (backendType != fplVideoBackendType_None && videoState->context.recreateOnResize) {
			fpl__ShutdownVideoBackend(appState, videoState);
			result = fpl__InitializeVideoBackend(videoState->backendType, &appState->currentSettings.video, width, height, appState, videoState);
		}
	}
	return (result);
}

fpl_common_api void fplVideoFlip() {
	FPL__CheckPlatformNoRet();
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__VideoState *videoState = fpl__GetVideoState(appState);
	if (videoState != fpl_null && videoState->backendType != fplVideoBackendType_None) {
		fplAssert(videoState->context.presentFunc != fpl_null);
		videoState->context.presentFunc(appState, &appState->window, &videoState->data, &videoState->activeBackend.base);
	}
}

fpl_common_api const void *fplGetVideoProcedure(const char *procName) {
	FPL__CheckPlatform(fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__VideoState *videoState = fpl__GetVideoState(appState);
	const void *result = fpl_null;
	if (videoState != fpl_null && videoState->backendType != fplVideoBackendType_None) {
		fplAssert(videoState->context.getProcedureFunc != fpl_null);
		result = videoState->context.getProcedureFunc(&videoState->activeBackend.base, procName);
	}
	return(result);
}

fpl_common_api const fplVideoSurface *fplGetVideoSurface() {
	FPL__CheckPlatform(fpl_null);
	fpl__PlatformAppState *appState = fpl__global__AppState;
	const fpl__VideoState *videoState = fpl__GetVideoState(appState);
	const fplVideoSurface *result = fpl_null;
	if (videoState != fpl_null && videoState->backendType != fplVideoBackendType_None) {
		result = &videoState->activeBackend.base.surface;
	}
	return(result);
}

fpl_common_api bool fplGetVideoRequirements(const fplVideoBackendType backendType, fplVideoRequirements *requirements) {
	fpl__VideoContext context = fpl__ConstructVideoContext(backendType);
	bool result = false;
	if (context.getRequirementsFunc != fpl_null) {
		result = context.getRequirementsFunc(requirements);
	}
	return(result);
}
#endif // FPL__ENABLE_VIDEO

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// > SYSTEM_INIT
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#if !defined(FPL__SYSTEM_INIT_DEFINED)
#define FPL__SYSTEM_INIT_DEFINED

fpl_internal void fpl__ReleasePlatformStates(fpl__PlatformInitState *initState, fpl__PlatformAppState *appState) {
	fplAssert(initState != fpl_null);

	// Release audio
#	if defined(FPL__ENABLE_AUDIO)
	{
		// Auto-Stop audio if enabled
		if (appState->currentSettings.audio.stopAuto) {
			fpl__AudioState *audioState = fpl__GetAudioState(fpl__global__AppState);
			if (audioState != fpl_null) {
				fpl__CommonAudioState *commonAudioState = &audioState->common;
				fpl__AudioDeviceState deviceState = fpl__AudioGetDeviceState(commonAudioState);
				if (deviceState != fpl__AudioDeviceState_Stopped) {
					FPL_LOG_DEBUG("Core", "Stop Audio (Auto)");
					fplStopAudio();
				}
			}
		}

		FPL_LOG_DEBUG("Core", "Release Audio");
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		if (audioState != fpl_null) {
			fpl__ReleaseAudio(audioState);
		}
	}
#	endif

	// Shutdown video backend
#	if defined(FPL__ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState != fpl_null) {
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Shutdown Video Backend '%s'", fplGetVideoBackendName(videoState->backendType));
			fpl__ShutdownVideoBackend(appState, videoState);
		}
	}
#	endif

	// Release window
#	if defined(FPL__ENABLE_WINDOW)
	{
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Window");
		fpl__ReleaseWindow(initState, appState);
		fpl__ClearInternalEvents();
	}
#	endif

	// Release video backend
#	if defined(FPL__ENABLE_VIDEO)
	{
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		if (videoState != fpl_null) {
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Destroy surface for Video Backend '%s'", fplGetVideoBackendName(videoState->backendType));
			fpl__DestroySurfaceBackend(appState, videoState);

			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Video Backend '%s'", fplGetVideoBackendName(videoState->backendType));
			fpl__UnloadVideoBackend(appState, videoState);
		}
	}
#	endif

	if (appState != fpl_null) {
		// Release actual platform (There can only be one platform!)
		{
#		if defined(FPL_PLATFORM_WINDOWS)
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
		fplMemoryAlignedFree(appState);
		fpl__global__AppState = fpl_null;
	}

	fplClearStruct(initState);
}

#define FPL__PLATFORMTYPE_COUNT FPL__ENUM_COUNT(fplPlatformType_First, fplPlatformType_Last)
fpl_globalvar const char *fpl__globalPlatformTypeNameTable[] = {
	"Unknown", // fplPlatformType_Unknown
	"Windows", // fplPlatformType_Windows
	"Linux", // fplPlatformType_Linux
	"Unix", // fplPlatformType_Unix
};
fplStaticAssert(fplArrayCount(fpl__globalPlatformTypeNameTable) == FPL__PLATFORMTYPE_COUNT);

fpl_common_api bool fplIsPlatformInitialized() {
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	bool result = initState->isInitialized;
	return(result);
}

fpl_common_api const char *fplGetPlatformName(const fplPlatformType type) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(type, fplPlatformType_First, fplPlatformType_Last);
	const char *result = fpl__globalPlatformTypeNameTable[index];
	return(result);
}

fpl_common_api fplPlatformResultType fplGetPlatformResult() {
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	return(initState->initResult);
}

fpl_internal bool fpl__SetPlatformResult(const fplPlatformResultType resultType) {
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	initState->initResult = resultType;
	return(initState->initResult == fplPlatformResultType_Success);
}

fpl_common_api void fplPlatformRelease() {
	// Exit out if platform is not initialized
	fpl__PlatformInitState *initState = &fpl__global__InitState;
	if (!initState->isInitialized) {
		FPL__CRITICAL(FPL__MODULE_CORE, "Platform is not initialized");
		return;
	}
	fpl__PlatformAppState *appState = fpl__global__AppState;
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Release Platform");
	fpl__ReleasePlatformStates(initState, appState);
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Platform released");
}

fpl_common_api bool fplPlatformInit(const fplInitFlags initFlags, const fplSettings *initSettings) {
	// Exit out if platform is already initialized
	if (fpl__global__InitState.isInitialized) {
		FPL__CRITICAL(FPL__MODULE_CORE, "Platform is already initialized");
		return(fpl__SetPlatformResult(fplPlatformResultType_AlreadyInitialized));
	}

	fpl__PlatformInitState *initState = &fpl__global__InitState;
	fplClearStruct(initState);

	// Copy over init settings, such as memory allocation settings, etc.
	if (initSettings != fpl_null) {
		fplCopyStruct(&initSettings->memory, &initState->initSettings.memorySettings);
	}

	// Allocate platform app state memory (By boundary of 16-bytes)
	size_t platformAppStateSize = fplGetAlignedSize(sizeof(fpl__PlatformAppState), 16);

	// Include video/audio state memory in app state memory as well
#if defined(FPL__ENABLE_VIDEO)
	size_t videoMemoryOffset = 0;
	if (initFlags & fplInitFlags_Video) {
		platformAppStateSize += FPL__ARBITARY_PADDING;
		videoMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__VideoState);
	}
#endif

#if defined(FPL__ENABLE_AUDIO)
	size_t audioMemoryOffset = 0;
	if (initFlags & fplInitFlags_Audio) {
		platformAppStateSize += FPL__ARBITARY_PADDING;
		audioMemoryOffset = platformAppStateSize;
		platformAppStateSize += sizeof(fpl__AudioState);
	}
#endif

	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Allocate Platform App State Memory of size '%zu':", platformAppStateSize);
	fplAssert(fpl__global__AppState == fpl_null);
	void *platformAppStateMemory = fplMemoryAlignedAllocate(platformAppStateSize, 16);
	if (platformAppStateMemory == fpl_null) {
		FPL__CRITICAL(FPL__MODULE_CORE, "Failed Allocating Platform App State Memory of size '%zu'", platformAppStateSize);
		return(fpl__SetPlatformResult(fplPlatformResultType_FailedAllocatingMemory));
	}

	fpl__PlatformAppState *appState = fpl__global__AppState = (fpl__PlatformAppState *)platformAppStateMemory;
	appState->initFlags = initFlags;
	if (initSettings != fpl_null) {
		appState->initSettings = *initSettings;
	} else {
		fplSetDefaultSettings(&appState->initSettings);
	}
	appState->currentSettings = appState->initSettings;

	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully allocated Platform App State Memory of size '%zu'", platformAppStateSize);

	// Application types
#	if defined(FPL_APPTYPE_WINDOW)
	appState->initFlags |= fplInitFlags_Window;
#	elif defined(FPL_APPTYPE_CONSOLE)
	appState->initFlags |= fplInitFlags_Console;
#	endif

	// Force the inclusion of window when Video flags is set or remove the video flags when video is disabled
#	if defined(FPL__ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		appState->initFlags |= fplInitFlags_Window;
	}
#	else
	appState->initFlags = (fplInitFlags)(appState->initFlags & ~fplInitFlags_Video);
#	endif

	// Window flag are removed when windowing is disabled
#	if !defined(FPL__ENABLE_WINDOW)
	appState->initFlags = (fplInitFlags)(appState->initFlags & ~fplInitFlags_Window);
#	endif

	// Initialize sub-platforms
#	if defined(FPL_SUBPLATFORM_POSIX)
	{
		FPL_LOG_DEBUG("Core", "Initialize POSIX Subplatform:");
		if (!fpl__PosixInitSubplatform(initFlags, initSettings, &initState->posix, &appState->posix)) {
			FPL__CRITICAL("Core", "Failed initializing POSIX Subplatform!");
			fpl__ReleasePlatformStates(initState, appState);
			return(fpl__SetPlatformResult(fplPlatformResultType_FailedPlatform));
		}
		FPL_LOG_DEBUG("Core", "Successfully initialized POSIX Subplatform");
	}
#	endif // FPL_SUBPLATFORM_POSIX

#	if defined(FPL_SUBPLATFORM_X11)
	{
		FPL_LOG_DEBUG("Core", "Initialize X11 Subplatform:");
		if (!fpl__X11InitSubplatform(&appState->x11)) {
			FPL__CRITICAL("Core", "Failed initializing X11 Subplatform!");
			fpl__ReleasePlatformStates(initState, appState);
			return(fpl__SetPlatformResult(fplPlatformResultType_FailedPlatform));
		}
		FPL_LOG_DEBUG("Core", "Successfully initialized X11 Subplatform");
	}
#	endif // FPL_SUBPLATFORM_X11

	// Initialize the actual platform (There can only be one at a time!)
	bool isInitialized = false;
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Initialize %s Platform:", FPL_PLATFORM_NAME);
#	if defined(FPL_PLATFORM_WINDOWS)
	isInitialized = fpl__Win32InitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	elif defined(FPL_PLATFORM_LINUX)
	isInitialized = fpl__LinuxInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	elif defined(FPL_PLATFORM_UNIX)
	isInitialized = fpl__UnixInitPlatform(appState->initFlags, &appState->initSettings, initState, appState);
#	endif

	if (!isInitialized) {
		FPL__CRITICAL(FPL__MODULE_CORE, "Failed initializing %s Platform!", FPL_PLATFORM_NAME);
		fpl__ReleasePlatformStates(initState, appState);
		return(fpl__SetPlatformResult(fplPlatformResultType_FailedPlatform));
	}
	FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized %s Platform", FPL_PLATFORM_NAME);

	// Init video state
#	if defined(FPL__ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Video State");
		appState->video.mem = (uint8_t *)platformAppStateMemory + videoMemoryOffset;
		appState->video.memSize = sizeof(fpl__VideoState);
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		fplAssert(videoState != fpl_null);

		fplVideoBackendType videoBackendType = appState->initSettings.video.backend;

		// Construct video context (Function table)
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Construct Video Context:");
		videoState->context = fpl__ConstructVideoContext(videoBackendType);
		videoState->backendType = videoBackendType;

		// Load video backend (API)
		const char *videoBackendName = fplGetVideoBackendName(videoBackendType);
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Load Video API for Backend '%s':", videoBackendName);
		{
			if (!fpl__LoadVideoBackend(appState, videoState)) {
				FPL__CRITICAL(FPL__MODULE_CORE, "Failed loading Video API for Backend '%s'!", videoBackendName);
				fpl__ReleasePlatformStates(initState, appState);
				return(fpl__SetPlatformResult(fplPlatformResultType_FailedVideo));
			}
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully loaded Video API for Backend '%s'", videoBackendName);
	}
#	endif // FPL__ENABLE_VIDEO

	// Init Window & event queue
#	if defined(FPL__ENABLE_WINDOW)
	if (appState->initFlags & fplInitFlags_Window) {
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Window:");
		fpl__SetupWindowCallbacks winCallbacks = fplZeroInit;
		winCallbacks.preSetup = fpl__PrepareVideoWindowDefault;
		winCallbacks.postSetup = fpl__FinalizeVideoWindowDefault;
		if (!fpl__InitWindow(&appState->initSettings, &appState->currentSettings.window, appState, &winCallbacks)) {
			FPL__CRITICAL(FPL__MODULE_CORE, "Failed initializing Window!");
			fpl__ReleasePlatformStates(initState, appState);
			return(fpl__SetPlatformResult(fplPlatformResultType_FailedWindow));
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized Window");
	}
#	endif // FPL__ENABLE_WINDOW

	// Init Video
#	if defined(FPL__ENABLE_VIDEO)
	if (appState->initFlags & fplInitFlags_Video) {
		fpl__VideoState *videoState = fpl__GetVideoState(appState);
		fplAssert(videoState != fpl_null);
		fplWindowSize windowSize = fplZeroInit;
		fplGetWindowSize(&windowSize);
		const char *videoBackendName = fplGetVideoBackendName(appState->initSettings.video.backend);
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Video with Backend '%s':", videoBackendName);
		if (!fpl__InitializeVideoBackend(appState->initSettings.video.backend, &appState->initSettings.video, windowSize.width, windowSize.height, appState, videoState)) {
			FPL__CRITICAL(FPL__MODULE_CORE, "Failed initialization Video with Backend '%s' with settings (Width=%d, Height=%d)", videoBackendName, windowSize.width, windowSize.height);
			fpl__ReleasePlatformStates(initState, appState);
			return(fpl__SetPlatformResult(fplPlatformResultType_FailedVideo));
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized Video with Backend '%s'", videoBackendName);
	}
#	endif // FPL__ENABLE_VIDEO

	// Init Audio
#	if defined(FPL__ENABLE_AUDIO)
	if (appState->initFlags & fplInitFlags_Audio) {
		appState->audio.mem = (uint8_t *)platformAppStateMemory + audioMemoryOffset;
		appState->audio.memSize = sizeof(fpl__AudioState);
		const char *audioBackendName = fplGetAudioBackendName(appState->initSettings.audio.backend);
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Init Audio with Backend '%s':", audioBackendName);
		fpl__AudioState *audioState = fpl__GetAudioState(appState);
		fplAssert(audioState != fpl_null);
		fplAudioResultType initAudioResult = fpl__InitAudio(&appState->initSettings.audio, audioState);
		if (initAudioResult != fplAudioResultType_Success) {
			const char *initAudioResultName = fplGetAudioResultName(initAudioResult);
			const char *audioFormatName = fplGetAudioFormatName(initSettings->audio.targetFormat.type);
			FPL__CRITICAL(FPL__MODULE_CORE, "Failed initialization audio with Backend '%s' settings (Format=%s, SampleRate=%d, Channels=%d) -> %s",
				audioBackendName,
				audioFormatName,
				initSettings->audio.targetFormat.sampleRate,
				initSettings->audio.targetFormat.channels,
				initAudioResultName);
			fpl__ReleasePlatformStates(initState, appState);
			return(fpl__SetPlatformResult(fplPlatformResultType_FailedAudio));
		}
		FPL_LOG_DEBUG(FPL__MODULE_CORE, "Successfully initialized Audio with Backend '%s'", audioBackendName);

		// Auto play audio if needed
		if (appState->initSettings.audio.startAuto && (appState->initSettings.audio.clientReadCallback != fpl_null)) {
			FPL_LOG_DEBUG(FPL__MODULE_CORE, "Play Audio (Auto)");
			fplAudioResultType playResult = fplPlayAudio();
			if (playResult != fplAudioResultType_Success) {
				FPL__CRITICAL(FPL__MODULE_CORE, "Failed auto-play of audio, code: %d!", playResult);
				fpl__ReleasePlatformStates(initState, appState);
				return(fpl__SetPlatformResult(fplPlatformResultType_FailedAudio));
			}
		}
	}
#	endif // FPL__ENABLE_AUDIO

	initState->isInitialized = true;
	return(fpl__SetPlatformResult(fplPlatformResultType_Success));
}

fpl_common_api fplPlatformType fplGetPlatformType() {
	fplPlatformType result;
#if defined(FPL_PLATFORM_WINDOWS)
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

#endif // FPL__SYSTEM_INIT_DEFINED

#if defined(FPL_COMPILER_MSVC)
//! Reset MSVC warning settings
#	pragma warning( pop )
#elif defined(FPL_COMPILER_GCC)
//! Reset GCC warning settings
#	pragma GCC diagnostic pop
#elif defined(FPL_COMPILER_CLANG)
//! Reset Clang warning settings
#	pragma clang diagnostic pop
#endif

#endif // FPL_IMPLEMENTATION && !FPL__IMPLEMENTED

// ****************************************************************************
//
// Entry-Points Implementation
//
// ****************************************************************************
#if defined(FPL_ENTRYPOINT) && !defined(FPL__ENTRYPOINT_IMPLEMENTED)
#	define FPL__ENTRYPOINT_IMPLEMENTED

// ***************************************************************
//
// Platform includes for entry point
//
// ***************************************************************
#if !defined(FPL__HAS_PLATFORM_INCLUDES)
#	define FPL__HAS_PLATFORM_INCLUDES

#	if defined(FPL_PLATFORM_WINDOWS)
		// @NOTE(final): windef.h defines min/max macros in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#		if !defined(NOMINMAX)
#			define NOMINMAX
#		endif
		// @NOTE(final): For now we dont want any network, com or gdi stuff at all, maybe later who knows.
#		if !defined(WIN32_LEAN_AND_MEAN)
#			define WIN32_LEAN_AND_MEAN 1
#		endif
		// @STUPID(final): Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here"
struct IUnknown;
#		include <windows.h> // Win32 api
#		if _WIN32_WINNT < 0x0600
#			error "Windows Vista or higher required!"
#		endif
#	endif // FPL_PLATFORM_WINDOWS

#endif // !FPL__HAS_PLATFORM_INCLUDES

// ***************************************************************
//
// Win32 Entry point
//
// ***************************************************************
#	if defined(FPL_PLATFORM_WINDOWS)

#define FPL__FUNC_WIN32_CommandLineToArgvW(name) LPWSTR* WINAPI name(LPCWSTR lpCmdLine, int *pNumArgs)
typedef FPL__FUNC_WIN32_CommandLineToArgvW(fpl__win32_func_CommandLineToArgvW);

typedef struct fpl__Win32CommandLineUTF8Arguments {
	void *mem;
	char **args;
	uint32_t count;
} fpl__Win32CommandLineUTF8Arguments;

fpl_internal fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseWideArguments(LPWSTR cmdLine, const bool appendExecutable) {
	fpl__Win32CommandLineUTF8Arguments args = fplZeroInit;

	// @NOTE(final): Temporary load and unload shell32 for parsing the arguments
	HMODULE shellapiLibrary = LoadLibraryA("shell32.dll");
	if (shellapiLibrary != fpl_null) {
		fpl__win32_func_CommandLineToArgvW *commandLineToArgvW = (fpl__win32_func_CommandLineToArgvW *)GetProcAddress(shellapiLibrary, "CommandLineToArgvW");
		if (commandLineToArgvW != fpl_null) {
			// Parse executable arguments
			int cmdLineLen = lstrlenW(cmdLine);
			int executableFilePathArgumentCount = 0;
			wchar_t **executableFilePathArgs = NULL;
			size_t executableFilePathLen = 0;
			if (appendExecutable || (cmdLineLen == 0)) {
				executableFilePathArgumentCount = 0;
				executableFilePathArgs = commandLineToArgvW(L"", &executableFilePathArgumentCount);
				executableFilePathLen = 0;
				for (int i = 0; i < executableFilePathArgumentCount; ++i) {
					if (i > 0) {
						// Include whitespace
						executableFilePathLen++;
					}
					size_t sourceLen = lstrlenW(executableFilePathArgs[i]);
					int destLen = WideCharToMultiByte(CP_UTF8, 0, executableFilePathArgs[i], (int)sourceLen, fpl_null, 0, fpl_null, fpl_null);
					executableFilePathLen += destLen;
				}
			}

			// Parse arguments and add to total UTF8 string length
			int actualArgumentCount = 0;
			wchar_t **actualArgs = fpl_null;
			size_t actualArgumentsLen = 0;
			if (cmdLine != fpl_null && cmdLineLen > 0) {
				actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
				for (int i = 0; i < actualArgumentCount; ++i) {
					size_t sourceLen = lstrlenW(actualArgs[i]);
					int destLen = WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], (int)sourceLen, fpl_null, 0, fpl_null, fpl_null);
					actualArgumentsLen += destLen;
				}
			}

			// Calculate argument
			uint32_t totalArgumentCount = 0;
			if (executableFilePathArgumentCount > 0) {
				totalArgumentCount++;
			}
			totalArgumentCount += actualArgumentCount;

			// @NOTE(final): We allocate one memory block that contains
			// - The arguments as one string, each terminated by zero-character -> char*
			// - A padding
			// - The size of the string array -> char**
			size_t totalStringLen = executableFilePathLen + actualArgumentsLen + totalArgumentCount;
			size_t singleArgStringSize = sizeof(char) * (totalStringLen);
			size_t arbitaryPadding = 64;
			size_t argArraySize = sizeof(char **) * totalArgumentCount;
			size_t totalArgSize = singleArgStringSize + arbitaryPadding + argArraySize;

			// @NOTE(final): We cannot use fpl__AllocateDynamicMemory here, because the main function is not called yet - therefore we dont have any fplMemorySettings set at this point.
			args.count = totalArgumentCount;
			args.mem = (uint8_t *)fplMemoryAllocate(totalArgSize);
			args.args = (char **)((uint8_t *)args.mem + singleArgStringSize + arbitaryPadding);

			// Convert executable path to UTF8 and add it, if needed
			char *destArg = (char *)args.mem;
			int startArgIndex = 0;
			if (executableFilePathArgumentCount > 0)
			{
				args.args[startArgIndex++] = destArg;
				for (int i = 0; i < executableFilePathArgumentCount; ++i) {
					if (i > 0) {
						*destArg++ = ' ';
					}
					wchar_t *sourceArg = executableFilePathArgs[i];
					size_t sourceArgLen = lstrlenW(sourceArg);
					int destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, fpl_null, 0, fpl_null, fpl_null);
					WideCharToMultiByte(CP_UTF8, 0, sourceArg, (int)sourceArgLen, destArg, destArgLen, fpl_null, fpl_null);
					destArg += destArgLen;
				}
				*destArg++ = 0;
				LocalFree(executableFilePathArgs);
			}

			// Convert actual arguments to UTF8 and add it, if needed
			if (actualArgumentCount > 0) {
				fplAssert(actualArgs != fpl_null);
				for (int i = 0; i < actualArgumentCount; ++i) {
					args.args[startArgIndex++] = destArg;
					wchar_t *sourceArg = actualArgs[i];
					size_t sourceArgLen = lstrlenW(sourceArg);
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

fpl_internal fpl__Win32CommandLineUTF8Arguments fpl__Win32ParseAnsiArguments(LPSTR cmdLine, const bool appendExecutable) {
	fpl__Win32CommandLineUTF8Arguments result;
	if (cmdLine != fpl_null) {
		size_t ansiSourceLen = fplGetStringLength(cmdLine);
		int wideDestLen = MultiByteToWideChar(CP_ACP, 0, cmdLine, (int)ansiSourceLen, fpl_null, 0);
		// @NOTE(final): We cannot use fpl__AllocateDynamicMemory here, because the main function is not called yet - therefore we dont have any fplMemorySettings set at this point.
		wchar_t *wideCmdLine = (wchar_t *)fplMemoryAllocate(sizeof(wchar_t) * (wideDestLen + 1));
		MultiByteToWideChar(CP_ACP, 0, cmdLine, (int)ansiSourceLen, wideCmdLine, wideDestLen);
		wideCmdLine[wideDestLen] = 0;
		result = fpl__Win32ParseWideArguments(wideCmdLine, appendExecutable);
		fplMemoryFree(wideCmdLine);
	} else {
		wchar_t tmp[1] = { 0 };
		result = fpl__Win32ParseWideArguments(tmp, appendExecutable);
	}
	return(result);
}

#if !defined(FPL_NO_CRT)
#	include <stdio.h>
#endif

fpl_internal void fpl__Win32FreeConsole() {
	HWND consoleHandle = GetConsoleWindow();
	if (consoleHandle != fpl_null) {
		FreeConsole();
	}
}

fpl_internal void fpl__Win32InitConsole() {
	HWND consoleHandle = GetConsoleWindow();
	if (consoleHandle == fpl_null) {
		// Create or attach console
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		// Redirect out/in/err to console
		HANDLE hConOut = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hConIn = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
		SetStdHandle(STD_ERROR_HANDLE, hConOut);
		SetStdHandle(STD_INPUT_HANDLE, hConIn);

#if !defined(FPL_NO_CRT)
		FILE *dummy;
		freopen_s(&dummy, "CONIN$", "r", stdin);
		freopen_s(&dummy, "CONOUT$", "w", stderr);
		freopen_s(&dummy, "CONOUT$", "w", stdout);
#endif
	}
}

#		if defined(FPL_NO_CRT)
//
// Win32 without CRT
//
#			if defined(FPL_APPTYPE_WINDOW)
#				if defined(UNICODE)
void __stdcall wWinMainCRTStartup(void) {
	fpl__Win32InitConsole();
	LPWSTR argsW = GetCommandLineW();
	int result = wWinMain(GetModuleHandleW(fpl_null), fpl_null, argsW, SW_SHOW);
	fpl__Win32FreeConsole();
	ExitProcess(result);
}
#				else
void __stdcall WinMainCRTStartup(void) {
	fpl__Win32InitConsole();
	LPSTR argsA = GetCommandLineA();
	int result = WinMain(GetModuleHandleA(fpl_null), fpl_null, argsA, SW_SHOW);
	fpl__Win32FreeConsole();
	ExitProcess(result);
}
#				endif // UNICODE
#			elif defined(FPL_APPTYPE_CONSOLE)
void __stdcall mainCRTStartup(void) {
	fpl__Win32InitConsole();
	fpl__Win32CommandLineUTF8Arguments args;
#	if defined(UNICODE)
	LPWSTR argsW = GetCommandLineW();
	args = fpl__Win32ParseWideArguments(argsW, false);
#	else
	LPSTR argsA = GetCommandLineA();
	args = fpl__Win32ParseAnsiArguments(argsA, false);
#	endif
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	fpl__Win32FreeConsole();
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
	fpl__Win32InitConsole();
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseWideArguments(cmdLine, true);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	fpl__Win32FreeConsole();
	return(result);
}
#			else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl__Win32InitConsole();
	fpl__Win32CommandLineUTF8Arguments args = fpl__Win32ParseAnsiArguments(cmdLine, true);
	int result = main(args.count, args.args);
	fplMemoryFree(args.mem);
	fpl__Win32FreeConsole();
	return(result);
}
#			endif // UNICODE

#		endif // FPL_NO_CRT

#	endif // FPL_PLATFORM_WINDOWS

#endif // FPL_ENTRYPOINT && !FPL__ENTRYPOINT_IMPLEMENTED

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

#	if defined(FPL_PLATFORM_WINDOWS)
#		undef TRUE
#		undef FALSE
#		undef far
#		undef near
#		undef IN
#		undef OUT
#		undef OPTIONAL
#	endif // FPL_SUBPLATFORM_X11

#endif // !FPL_NO_UNDEF

/// @endcond

// end-of-file