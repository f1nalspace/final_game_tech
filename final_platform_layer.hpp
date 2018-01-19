/***
final_platform_layer.hpp

A Open source C++ single file header platform abstraction layer library.

This library is designed to abstract the underlying platform to a very simple and easy to use api.
The main focus is game/simulation development, so the default settings will create a window, setup a opengl rendering context and initialize audio playback on any platform.

The only dependencies are built-in operating system libraries, a C++/11 compiler and the C runtime library.

Copyright 2017 Torsten Spaete

Final Platform Layer is released under the MIT License:

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
***/

/*!
	\file final_platform_layer.hpp
	\version v0.5.7.2 beta
	\author Torsten Spaete
	\brief Final Platform Layer (FPL) - A Open source C++ single file header platform abstraction layer library.
*/

/*!
	\mainpage
	\tableofcontents

	\section section_intro Introduction

	<em>This library is designed to abstract the underlying platform to a very simple and easy to use api.</em><br>
	<br>
	The main focus is game/simulation development, so the default settings will create a window, setup a opengl rendering context and initialize audio playback on any platform.<br>
	The only dependencies are built-in operating system libraries, a C++/11 compiler and the C runtime library.<br>

	It works very well with other libraries like for example:

	- STB Libraries
	- Standard C++ Library (STL)
	- Glew/Glad
	- Box2D
	- GLM
	- ImGUI
	- etc.

	\section section_features Features

	- Creating a fixed or resizeable window
	- Handling window, keyboard, mouse events
	- Enable/Disable fullscreen
	- Full event buffering
	- Polling gamepad informations
	- Clipboard string reading and writing
	- Multi-monitor support for window creation and fullscreen switching (Todo)

	- Video output:
		- Legacy 1.x opengl rendering context
		- Modern 3.x + opengl rendering context
		- Software backbuffer

	- Asyncronous audio playback
		- DirectSound
		- Alsa (Todo)

	- Memory allocation and de-allocation with custom alignment support
	- Atomic operations
	- Path functions
	- File functions
	- Hardware functions
	- String conversion functions
	- Thread/Mutex/Signal handling

	\subsection section_supported_archs Supported Architectures

	- x86 / x86_64
	- x64

	\subsection section_supported_compilers Supported Compilers

	- Compiles with MSVC
	- Compiles with GCC
	- Compiles with Clang (Partially)
	- Compiles with MingW (Not tested)
	- Compiles with Intel C++ Compiler (Not tested)

	\subsection section_supported_platforms Supported Platforms

	- Win32
	- Compiles with GCC
	- Linux/Posix (Partially)
	- Unix/BSD/Posix (Todo)

	\subsection section_todo Todo

	See \subpage page_todo

	\section section_howtocompile How to Compile

	You need a C++/11 compatible compiler like MSVC, GCC, Clang, etc.

	\subpage page_compiler_options

	\subsection sectionCompileWin32 Win32

	- Link against "kernel32.lib"

	\subsection sectionCompileLinux Linux

	- Link against "ld"

	\section section_usage Usage

	In one of your C++ translation units include this:

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	#include "final_platform_layer.hpp"
	\endcode

	You can then include this file in any other C++ source or header file as you would with any other header file.

	Provide the typical main entry point with at least the initialization and release of the platform:

	\code{.cpp}
	int main(int argc, char **args) {
		// Initialize the platform
		fpl::InitPlatform(fpl::InitFlags::All);
		// ..
		// your code goes here
		// ..
		// Release the platform
		fpl::ReleasePlatform();
	}
	\endcode

	\section section_examples Examples

	- \subpage page_example_helloworld_console
	- \subpage page_example_opengl1x
	- \subpage page_example_opengl33
	- \subpage page_example_simple_audio

	\section section_notes Notes

	\subsection section_audio Audio

	This library uses the operating system libraries to initialize a playback audio device.<br>
	To get any audio samples to play you need to set the callback function with \ref fpl::AudioSettings.clientReadCallback or fpl::audio::SetAudioClientReadCallback() before start playing the audio using PlayAudio().<br>
	When the audio device requires more audio samples this callback is automatically called.<br>
	It is expected that the audio samples are filled in properly by the native format of the device, see \ref fpl::AudioDeviceFormat.<br>
	This library does not provide any functionality for doing any kind of DSP.<br>
	To start and stop the playback, you need to call fpl::audio::PlayAudio() and fpl::audio::StopAudio() respectively.<br>
	There is no guarantee that you get a audio device with the exact same format you specified back, but S16 with 48 KHz is a common format which almost every audio card supports.<br>

	\section section_license License

	Final Platform Layer is released under the \subpage page_license "MIT License".

	\section section_version_history Version History

	\subpage page_changelog "Changelog / Version History"

	\section section_thanksto Acknowledgements

	The audio playback system in this library is a port of "mini_al.h", a small but fine C-Library written by David Reid.
*/

/*!
	\page page_compiler_options Compiler Options
	\tableofcontents

	Define these options before including this file.

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	\endcode

	Define this to include the actual implementation code as well.
	Set this only once per translation unit, otherwise you will get linking errors.

	\code{.cpp}
	#define FPL_API_AS_PRIVATE
	\endcode

	Define this to make all functions be private "static".
	Default is "export".

	\code{.cpp}
	#define FPL_NO_ASSERTIONS
	\endcode

	Define this to disable all internal assertions.

	\code{.cpp}
	#define FPL_FORCE_ASSERTIONS
	\endcode

	Define this to enable internal assertions always, even in debug builds.
	NOTE: When enabled, all assertions wont use the C assert() at all, because it may be compiled out!

	\code{.cpp}
	#define FPL_NO_C_ASSERT
	\endcode

	Define this to disable C runtime assert.
	Has no effect when FPL_FORCE_ASSERTIONS is set!

	\code{.cpp}
	#define FPL_NO_WINDOW
	\endcode

	Define this to disable window support entirely.

	\code{.cpp}
	#define FPL_NO_VIDEO
	\endcode

	Define this to disable any rendering device entirely.

	\code{.cpp}
	#define FPL_NO_VIDEO_OPENGL
	\endcode

	Define this to disable opengl rendering support entirely.

	\code{.cpp}
	#define FPL_NO_VIDEO_SOFTWARE
	\endcode

	Define this to disable software rendering support entirely.

	\code{.cpp}
	#define FPL_NO_AUDIO
	\endcode

	Define this to disable audio playback entirely.

	\code{.cpp}
	#define FPL_NO_AUDIO_DIRECTSOUND
	\endcode

	Define this to disable directsound support entirely.

	\code{.cpp}
	#define FPL_NO_MULTIPLE_ERRORSTATES
	\endcode

	Define this to use a single error state for GetPlatformLastError() instead of multiple ones.

	\code{.cpp}
	#define FPL_NO_ERROR_IN_CONSOLE
	\endcode

	Define this to disable printing any platform specific errors to the console.

	\code{.cpp}
	#define FPL_AUTO_NAMESPACE
	\endcode

	Define this to include all required namespaces automatically.
*/

/*!
	\page page_example_helloworld_console Hello World Console Application!
	\tableofcontents

	\section section_maincpp1 main.cpp

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	#include "final_platform_layer.hpp"

	int main(int argc, char **args) {
		int result;
		if (fpl::InitPlatform(fpl::InitFlags::None)) {
			fpl::console::ConsoleOut("Hello World!");
			fpl::ReleasePlatform();
			result = 0;
		} else {
			result = -1;
		}
		return(result);
	}
	\endcode

*/

/*!
	\page page_example_opengl1x Simple OpenGL 1.x Triangle
	\tableofcontents

	\section section_maincpp2 main.cpp

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	#include "final_platform_layer.hpp"

	// You have to include GL.h yourself or use any other opengl loader you want.
	// This library just creates a opengl rendering context for you, but nothing more.
	// But GL.h will be included when FPL_IMPLEMENTATION is set always.
	#include <GL\GL.h>

	using namespace fpl;
	using namespace fpl::window;

	int main(int argc, char **args) {
		int result = 0;
		if (InitPlatform(InitFlags::Video)) {
			glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
			while (WindowUpdate()) {
				WindowSize windowArea = GetWindowArea();
				glViewport(0, 0, windowArea.width, windowArea.height);
				glClear(GL_COLOR_BUFFER_BIT);
				glBegin(GL_TRIANGLES);
				glVertex2f(0.0f, 0.5f);
				glVertex2f(-0.5f, -0.5f);
				glVertex2f(0.5f, -0.5f);
				glEnd();
				WindowFlip();
			}
			ReleasePlatform();
			result = 0;
		} else {
			result = -1;
		}
		return(result);
	}
	\endcode

*/

/*!
	\page page_example_opengl33 Modern OpenGL 3.3+
	\tableofcontents

	\section section_maincpp3 main.cpp

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	#include "final_platform_layer.hpp"

	// You have to include GL.h yourself or use any other opengl loader you want.
	// This library just creates a opengl rendering context for you, but nothing more.
	// But GL.h will be included when FPL_IMPLEMENTATION is set always.
	#include <GL\GL.h>

	using namespace fpl;
	using namespace fpl::window;

	int main(int argc, char **args) {
		int result = 0;

		// Legacy opengl is default, so we force it to be forward or backward compability
		Settings settings = DefaultSettings();
		settings.video.driverType = VideoDriverType::OpenGL;
		settings.video.profile = VideoCompabilityProfile::Forward;
		settings.video.majorVersion = 3;
		settings.video.minorVersion = 3;
		if (InitPlatform(InitFlags::Video, settings)) {
			glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
			while (WindowUpdate()) {
				WindowSize windowArea = GetWindowArea();
				glViewport(0, 0, windowArea.width, windowArea.height);
				glClear(GL_COLOR_BUFFER_BIT);

				// Your code using modern opengl (VBO, IBO, VAO, GLSL, etc.)

				WindowFlip();
			}
			ReleasePlatform();
			result = 0;
		} else {
			result = -1;
		}
		return(result);
	}
	\endcode

*/

/*!
	\page page_example_simple_audio Simple audio playback (Infinite square or sine wave)
	\tableofcontents

	\section section_maincpp4 main.cpp

	\code{.cpp}
	#define FPL_IMPLEMENTATION
	#define FPL_AUTO_NAMESPACE
	#define FPL_NO_WINDOW
	#include <final_platform_layer.hpp>
	#include <math.h>

	struct AudioTest {
		uint32_t toneHz;
		uint32_t toneVolume;
		uint32_t runningSampleIndex;
		uint32_t wavePeriod;
		bool useSquareWave;
	};

	static const float PI32 = 3.14159265359f;

	static uint32_t FillAudioBuffer(const AudioDeviceFormat &nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
		AudioTest *audioTest = (AudioTest *)userData;
		FPL_ASSERT(audioTest != nullptr);
		FPL_ASSERT(nativeFormat.type == AudioFormatType::S16);
		uint32_t result = 0;
		int16_t *outSamples = (int16_t *)outputSamples;
		uint32_t halfWavePeriod = audioTest->wavePeriod / 2;
		for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			int16_t sampleValue;
			if (audioTest->useSquareWave) {
				sampleValue = ((audioTest->runningSampleIndex++ / halfWavePeriod) % 2) ? (int16_t)audioTest->toneVolume : -(int16_t)audioTest->toneVolume;
			} else {
				float t = 2.0f * PI32 * (float)audioTest->runningSampleIndex++ / (float)audioTest->wavePeriod;
				sampleValue = (int16_t)(sinf(t) * audioTest->toneVolume);
			}
			for (uint32_t channelIndex = 0; channelIndex < nativeFormat.channels; ++channelIndex) {
				*outSamples++ = sampleValue;
				++result;
			}
		}
		return result;
	}

	int main(int argc, char **args) {
		int result = -1;

		Settings settings = DefaultSettings();

		AudioTest audioTest = {};
		audioTest.toneHz = 256;
		audioTest.toneVolume = 1000;
		audioTest.wavePeriod = settings.audio.desiredFormat.sampleRate / audioTest.toneHz;
		audioTest.useSquareWave = false;

		settings.audio.clientReadCallback = FillAudioBuffer;
		settings.audio.userData = &audioTest;

		if (InitPlatform(InitFlags::Audio, settings)) {
			if (PlayAudio() == AudioResults::Success) {
				ConsoleOut("Press any key to quit playing...\n");
				getchar();
				StopAudio();
			}
			ReleasePlatform();
		}
		return(result);
	}
	\endcode

*/

/*!
	\page page_license License
	\tableofcontents

	\section section_mit_license MIT License

	Copyright (c) 2017-2018 Torsten Spaete<br>
	<br>
	Permission is hereby granted, free of charge, to any person obtaining a copy of<br>
	this software and associated documentation files (the "Software"), to deal in<br>
	the Software without restriction, including without limitation the rights to<br>
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies<br>
	of the Software, and to permit persons to whom the Software is furnished to do<br>
	so, subject to the following conditions:<br>
	<br>
	The above copyright notice and this permission notice shall be included in all<br>
	copies or substantial portions of the Software.<br>
	<br>
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR<br>
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,<br>
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE<br>
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER<br>
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,<br>
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE<br>
	SOFTWARE.<br>
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

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
	\page page_todo ToDo / Planned (Top priority order)
	\tableofcontents

	- Change most assertions to normal comparisons and make it rock solid, so it wont crash for the most part. Returning nullptr or empty is much more preferred.

	- Finish Linux Platform:
		- Strings
		- Files & Path (Look out for .DS_Store and . files/folders, handle it properly)
		- Hardware
		- Threading (pthread)
		- Window (X11)
		- Video opengl (GLX)
		- Video software
		- Audio (Alsa)

	- Support other compilers for Win32 (Clang, MingW, Intel)
		- Fix Clang errors

	- REFERENCE.MD generation using doxygen

	- Feature completeness for Win32 (Multimonitor)

	- Additional parameters for passing pointers instead of returning structs (Method overloading)

	- Solidify file/path system:
		- Decide to a fixed encoding, either unicode 16 bit or UTF8 or
			Leave it as it is, but give the caller informations about the platform (which separator, what character encoding for files/path etc.)<br>
			And use a custom main entry point for every platform so we can ensure that the arguments come in as native always.<br>
			Reason: I want unicode support for arguments in win32 and UTF8 for the other platforms.<br>

	- Remove the need of the C/C++ Runtime

	- Additional features for later:
		- Open/Save file/folder dialog
		- Networking (UDP, TCP)
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
#if defined(_WIN32)
#	define FPL_PLATFORM_WIN32
#	define FPL_PLATFORM_NAME "Win32"
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FPL_PLATFORM_LINUX
#	define FPL_PLATFORM_NAME "Linux"
#	define FPL_PLATFORM_POSIX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "Unix"
#	define FPL_PLATFORM_POSIX
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
#if defined(__clang__)
	//! CLANG compiler detected
#	define FPL_COMPILER_CLANG
#elif defined(__llvm__)
	//! LLVM compiler detected
#	define FPL_COMPILER_LLVM
#elif defined(__INTEL_COMPILER) || defined(__ICC)
	//! Intel compiler detected
#	define FPL_COMPILER_INTEL
#elif defined(__GNUC__) || defined(__GNUG__)
	//! GCC compiler detected
#	define FPL_COMPILER_GCC
#elif defined(__MINGW32__)
	//! MingW compiler detected
#	define FPL_COMPILER_MINGW
#elif defined(_MSC_VER)
	//! Visual studio compiler detected
#	define FPL_COMPILER_MSVC
#else
	//! No compiler detected
#	define FPL_COMPILER_UNKNOWN
#endif

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
#endif

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
#endif
#if defined(FPL_ENABLE_ASSERTIONS)
#	if !defined(FPL_NO_C_ASSERT)
		//! Enable C-Runtime Assertions by default
#		define FPL_ENABLE_C_ASSERT
#	endif
#endif
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
#endif

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
#endif

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
#endif
#if defined(FPL_SUPPORT_AUDIO)
	//! Enable Audio
#	define FPL_ENABLE_AUDIO
#	if defined(FPL_SUPPORT_AUDIO_DIRECTSOUND)
		//! Enable DirectSound Audio
#		define FPL_ENABLE_AUDIO_DIRECTSOUND
#	endif
#endif

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
		fpl_api void AtomicReadFence();
		/**
		  * \brief Insert a memory write fence/barrier.
		  *
		  * This will complete previous writes before future writes and prevents the compiler from reordering memory writes across this fence.
		  */
		fpl_api void AtomicWriteFence();
		/**
		  * \brief Insert a memory read/write fence/barrier.
		  *
		  * This will complete previous reads/writes before future reads/writes and prevents the compiler from reordering memory access across this fence.
		  */
		fpl_api void AtomicReadWriteFence();

		//! Replace a 32-bit unsigned integer with the given value atomically. Returns the target before the replacement.
		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
		//! Replace a 32-bit signed integer with the given value atomically. Returns the target before the replacement.
		fpl_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value);
		//! Replace a 64-bit unsigned integer with the given value atomically. Returns the target before the replacement.
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
		//! Replace a 64-bit signed integer with the given value atomically. Returns the target before the replacement.
		fpl_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value);
		//! Replace a pointer with the given value atomically. Returns the target before the replacement.
		fpl_api void *AtomicExchangePtr(volatile void **target, const void *value);

		//! Adds a 32-bit unsigned integer atomatically. Returns the value before the addition.
		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
		//! Adds a 32-bit signed integer atomatically. Returns the value before the addition.
		fpl_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend);
		//! Adds a 64-bit unsigned integer atomatically. Returns the value before the addition.
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
		//! Adds a 64-bit signed integer atomatically. Returns the value before the addition.
		fpl_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend);

		//! Compares a 32-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
		//! Compares a 32-bit signed integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
		//! Compares a 64-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
		//! Compares a 64-bit signed integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
		//! Compares a pointer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api void *AtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

		//! Returns true when a 32-bit unsigned integer equals the comparand. In addition dest will be changed to the exchange when the result is true as well.
		fpl_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange);
		//! Returns true when a 32-bit signed integer equals the comparand. In addition dest will be changed to the exchange when the result is true as well.
		fpl_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange);
		//! Returns true when a 64-bit unsigned integer equals the comparand. In addition dest will be changed to the exchange when the result is true as well.
		fpl_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange);
		//! Returns true when a 64-bit signed integer equals the comparand. In addition dest will be changed to the exchange when the result is true as well.
		fpl_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange);
		//! Returns true when a pointer equals the comparand. In addition dest will be changed to the exchange when the result is true as well.
		fpl_api bool IsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange);

		//! Loads the 32-bit unsigned value atomically and returns the result.
		fpl_api uint32_t AtomicLoadU32(volatile uint32_t *source);
		//! Loads the 64-bit unsigned value atomically and returns the result.
		fpl_api uint64_t AtomicLoadU64(volatile uint64_t *source);
		//! Loads the 32-bit signed value atomically and returns the result.
		fpl_api int32_t AtomicLoadS32(volatile int32_t *source);
		//! Loads the 64-bit signed value atomically and returns the result.
		fpl_api int64_t AtomicLoadS64(volatile int64_t *source);
		//! Loads the 64-bit signed value atomically and returns the result.
		fpl_api void *AtomicLoadPtr(volatile void **source);

		//! Stores the 32-bit unsigned value atomically into the target.
		fpl_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value);
		//! Stores the 64-bit unsigned value atomically into the target.
		fpl_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value);
		//! Stores the 32-bit signed value atomically into the target.
		fpl_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value);
		//! Stores the 64-bit signed value atomically into the target.
		fpl_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value);
		//! Stores the pointer value atomically into the target.
		fpl_api void AtomicStorePtr(volatile void **dest, const void *value);

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
			size_t totalPhysicalSize;
			//! Available size of physical memory in bytes (May be less than the amount of RAM installed)
			size_t availablePhysicalSize;
			//! Free size of physical memory in bytes
			size_t usedPhysicalSize;
			//! Total size of virtual memory in bytes
			size_t totalVirtualSize;
			//! Used size of virtual memory in bytes
			size_t usedVirtualSize;
			//! Total page size in bytes
			size_t totalPageSize;
			//! Used page size in bytes
			size_t usedPageSize;
		};

		//! Returns the total number of processor cores
		fpl_api uint32_t GetProcessorCoreCount();
		//! Returns the processor name/identifier. Result is written in the required destination buffer.
		fpl_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen);
		//! Returns the current system memory informations.
		fpl_api MemoryInfos GetSystemMemoryInfos();

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
		//! Use a video backbuffer (A window without video is pretty much useless)
		Video = 1 << 1,
		//! Use audio playback
		Audio = 1 << 2,
		//! Default init flags for a window + video + audio
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

	//! Video compability profile 
	enum class VideoCompabilityProfile {
		//! Use legacy context
		Legacy,
		//! Use core context with backwards compability
		Core,
		//! Use foward context without backwards compability
		Forward
	};

	//! Video settings container (Compability Profile, Version, VSync, etc.)
	struct VideoSettings {
		//! Video driver type
		VideoDriverType driverType;
		//! Compability profile
		VideoCompabilityProfile profile;
		//! Desired major version
		uint32_t majorVersion;
		//! Desired minor version
		uint32_t minorVersion;
		//! Vertical syncronisation is wanted
		bool isVSync;
		//! Backbuffer size is automatically resized. Useable only for software rendering!
		bool isAutoSize;
	};

	//! Make default settings for video
	fpl_inline VideoSettings DefaultVideoSettings() {
		VideoSettings result = {};
		result.profile = VideoCompabilityProfile::Legacy;
		result.majorVersion = result.minorVersion = 0;
		result.isVSync = false;
		result.isAutoSize = true;

		// @NOTE(final): Auto detect video driver
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		result.driverType = VideoDriverType::OpenGL;
#	elif defined(FPL_ENABLE_VIDEO_SOFTWARE)
		result.driverType = VideoDriverType::Software;
#	else
		result.driverType = VideoDriverType::None;
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
		DirectSound
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

	//! Audio Client Read Callback Function
	typedef uint32_t(AudioClientReadFunction)(const AudioDeviceFormat &deviceFormat, const uint32_t frameCount, void *outputSamples, void *userData);

	//! Audio settings
	struct AudioSettings {
		//! The targeted format
		AudioDeviceFormat desiredFormat;
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

	//! Make default audio settings (S16 PCM, 48 KHz, 2 Channels)
	fpl_inline AudioSettings DefaultAudioSettings() {
		AudioSettings result = {};
		result.bufferSizeInMilliSeconds = 25;
		result.preferExclusiveMode = false;
		result.desiredFormat.channels = 2;
		result.desiredFormat.sampleRate = 48000;
		result.desiredFormat.type = AudioFormatType::S16;

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

	//! Make default settings for the window
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

	//! Make default settings for input devices
	fpl_inline InputSettings DefaultInputSettings() {
		InputSettings result = {};
		result.controllerDetectionFrequency = 100;
		return(result);
	}

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
	};

	//! Default settings for video, window, etc.
	fpl_inline Settings DefaultSettings() {
		Settings result = {};
		result.window = DefaultWindowSettings();
		result.video = DefaultVideoSettings();
		result.audio = DefaultAudioSettings();
		result.input = DefaultInputSettings();
		return(result);
	}

	//! Returns the current settings
	fpl_api const Settings &GetCurrentSettings();

	/** \}*/

	/**
	  * \defgroup Initialization Initialization functions
	  * \brief Initialization and release functions
	  * \{
	  */

	//! Initialize the platform layer.
	fpl_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings = DefaultSettings());
	//! Releases the platform layer and resets all structures to zero.
	fpl_api void ReleasePlatform();

	/** \}*/

	/**
	  * \defgroup ErrorHandling Error Handling
	  * \brief Functions for error handling
	  * \{
	  */

	//! Returns last error string
	fpl_api const char *GetPlatformLastError();
	//! Returns last error string from the given index
	fpl_api const char *GetPlatformLastError(const size_t index);
	//! Returns number of last errors
	fpl_api size_t GetPlatformLastErrorCount();

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
#			if defined(FPL_PLATFORM_POSIX)
				void *posixHandle;
#			endif
			} internalHandle;
			//! Library opened successfully
			bool isValid;
		};

		//! Loads a dynamic library and returns the loaded handle for it.
		fpl_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath);
		//! Returns the dynamic library procedure address for the given procedure name.
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name);
		//! Releases the loaded library and resets the handle to zero.
		fpl_api void DynamicLibraryUnload(DynamicLibraryHandle &handle);

		/** \}*/
	}

	//! Console functions
	namespace console {
		/**
		  * \defgroup Console Console functions
		  * \brief Console out/in functions
		  * \{
		  */

		//! Writes the given text to the default console output
		fpl_api void ConsoleOut(const char *text);
		//! Writes the given formatted text to the default console output 
		fpl_api void ConsoleFormatOut(const char *format, ...);
		//! Writes the given text to the console error output
		fpl_api void ConsoleError(const char *text);
		//! Writes the given formatted text to the console error output
		fpl_api void ConsoleFormatError(const char *format, ...);

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
			//! Thread is still running
			Running,
			// Thread is suspended
			Suspended
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
				HANDLE win32Handle;
#			endif
			} internalHandle;
			//! Thread state
			volatile ThreadState currentState;
		};

		//! Mutex context
		struct ThreadMutex {
			//! The internal mutex handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				CRITICAL_SECTION win32CriticalSection;
#			endif
			} internalHandle;			//! Is it valid
			bool isValid;
		};

		//! Signal context
		struct ThreadSignal {
			//! The internal signal handle
			union {
#			if defined(FPL_PLATFORM_WIN32)
				HANDLE win32Handle;
#			endif
			} internalHandle;
			//! Is it valid
			bool isValid;
		};

		//! Create a thread and return the context for it. When autoStart is set to true, it will start immediatly. 
		fpl_api ThreadContext *ThreadCreate(run_thread_function *runFunc, void *data, const bool autoStart = true);
		//! Let the current thread sleep for the number of given milliseconds
		fpl_api void ThreadSleep(const uint32_t milliseconds);
		//! Suspend the given thread, so any user code will get be freezed at the current point. Returns true when the thread was successfully suspended.
		fpl_api bool ThreadSuspend(ThreadContext *context);
		//! Resume a suspended thread, so any user code will continue to run. Returns true when the thread was successfully resumed.
		fpl_api bool ThreadResume(ThreadContext *context);
		//! Stop the given thread and release all underlying resources.
		fpl_api void ThreadDestroy(ThreadContext *context);
		//! Wait until the given thread is done running. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool ThreadWaitForOne(ThreadContext *context, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Wait until all given threads are done running. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool ThreadWaitForAll(ThreadContext **contexts, const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Wait until one of given threads are done running. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool ThreadWaitForAny(ThreadContext **contexts, const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);

		//! Creates a mutex
		fpl_api ThreadMutex MutexCreate();
		//! Destroys the given mutex
		fpl_api void MutexDestroy(ThreadMutex &mutex);
		/**
		 * \brief Locks the given mutex and waits until it gets unlocked.
		 * When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		 * \param mutex The mutex reference to lock
		 * \param maxMilliseconds The maximum number of milliseconds to wait (Optional)
		 * \returns Mutex was locked or not
		 */
		fpl_api bool MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds = UINT32_MAX);
		/**
		 * \brief Unlocks the given mutex
		 * \param mutex The mutex reference to unlock
		 * \returns Mutex was unlocked or not
		 */
		fpl_api bool MutexUnlock(ThreadMutex &mutex);

		//! Creates a signal
		fpl_api ThreadSignal SignalCreate();
		//! Destroys the given signal
		fpl_api void SignalDestroy(ThreadSignal &availableSignal);
		//! Waits until the given signal are waked up. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool SignalWaitForOne(ThreadSignal &availableSignal, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Waits until all the given signal are waked up. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool SignalWaitForAll(ThreadSignal **availableSignal, const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Waits until any of the given signals wakes up. When maxMilliseconds is set to UINT32_MAX it will wait infinitly.
		fpl_api bool SignalWaitForAny(ThreadSignal **availableSignal, const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Wakes up the given signal
		fpl_api bool SignalWakeUp(ThreadSignal &availableSignal);
		//! Resets the given signal
		fpl_api bool SignalReset(ThreadSignal &availableSignal);

		/** \}*/
	}

	//! Memory allocation, clearing and copy functions
	namespace memory {
		/**
		  * \defgroup Memory Memory functions
		  * \brief Memory allocation, clearing and copy functions
		  * \{
		  */

		//! Clears the given memory by the given size to zero.
		fpl_api void MemoryClear(void *mem, const size_t size);
		//! Copies the given source memory with its length to the target memory
		fpl_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem);
		//! Allocates memory from the operating system by the given size. The memory is guaranteed to be initialized by zero.
		fpl_api void *MemoryAllocate(const size_t size);
		//! Releases the memory allocated from the operating system.
		fpl_api void MemoryFree(void *ptr);
		//! Allocates memory on the current stack. Use this very carefully, it will be released then the current scope goes out of scope!
		fpl_api void *MemoryStackAllocate(const size_t size);
		//! Allocates aligned memory from the operating system by the given alignment. The memory is guaranteed to be initialized by zero.
		fpl_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment);
		//! Releases aligned memory from the operating system.
		fpl_api void MemoryAlignedFree(void *ptr);

		/** \}*/
	}

	//! Timing and measurement functions
	namespace timings {
		/**
		  * \defgroup Timings Timing functions
		  * \brief Functions for retrieving timebased informations
		  * \{
		  */

		//! Returns the current system clock in seconds with the highest precision.
		fpl_api double GetHighResolutionTimeInSeconds();

		/** \}*/
	}

	//! String functions
	namespace strings {
		/**
		  * \defgroup Strings String manipulation functions
		  * \brief Functions for converting/manipulating strings
		  * \{
		  */

		//! Returns the number of characters of the given 8-bit Ansi string. Null terminator is not included.
		fpl_api uint32_t GetAnsiStringLength(const char *str);
		//! Returns the number of characters of the given 16-bit Wide string. Null terminator is not included.
		fpl_api uint32_t GetWideStringLength(const wchar_t *str);
		//! Copies the given 8-bit source Ansi string with length into a destination Ansi string. Does not allocate any memory.
		fpl_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
		//! Copies the given 8-bit source Ansi string into a destination Ansi string. Does not allocate any memory.
		fpl_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen);
		//! Copies the given 16-bit source Wide string with length into a destination Wide string. Does not allocate any memory.
		fpl_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
		//! Copies the given 16-bit source Wide string into a destination Wide string. Does not allocate any memory.
		fpl_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen);
		//! Converts the given 16-bit source Wide string with length in a 8-bit Ansi string. Does not allocate any memory.
		fpl_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
		//! Converts the given 16-bit Wide string in a 8-bit UTF-8 string. Does not allocate any memory.
		fpl_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
		//! Converts the given 8-bit Ansi string in a 16-bit Wide string. Does not allocate any memory.
		fpl_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
		//! Converts the given 8-bit UTF-8 string in a 16-bit Wide string. Does not allocate any memory.
		fpl_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);

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
				HANDLE win32Handle;
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
				HANDLE win32Handle;
#			endif
			} internalHandle;
		};

		//! Opens a binary file from a ansi string path for reading and returns the handle of it.
		fpl_api FileHandle OpenBinaryFile(const char *filePath);
		//! Opens a binary file from a wide string path for reading and returns the handle of it.
		fpl_api FileHandle OpenBinaryFile(const wchar_t *filePath);
		//! Creates a binary file to a ansi string path and returns the handle of it.
		fpl_api FileHandle CreateBinaryFile(const char *filePath);
		//! Creates a binary file to a wide string path and returns the handle of it.
		fpl_api FileHandle CreateBinaryFile(const wchar_t *filePath);
		//! Reads a block from the given file handle and returns the number of bytes readed. Operation is limited to a 2 GB byte boundary.
		fpl_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
		//! Writes a block to the given file handle and returns the number of bytes written. Operation is limited to a 2 GB byte boundary.
		fpl_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize);
		//! Sets the current file position by the given position, depending on the mode its absolute or relative. Position is limited to a 2 GB byte boundary.
		fpl_api void SetFilePosition32(const FileHandle &fileHandle, const uint32_t position, const FilePositionMode mode);
		//! Returns the current file position. Position is limited to a 2 GB byte boundary.
		fpl_api uint32_t GetFilePosition32(const FileHandle &fileHandle);
		//! Closes the given file handle and resets the handle to zero.
		fpl_api void CloseFile(FileHandle &fileHandle);

		// @TODO(final): Add 64-bit file operations as well!

		//! Returns the 32-bit file size in bytes for the given file. Its limited to files < 2 GB.
		fpl_api uint32_t GetFileSize32(const char *filePath);
		//! Returns the 32-bit file size in bytes for a opened file. Its limited to files < 2 GB.
		fpl_api uint32_t GetFileSize32(const FileHandle &fileHandle);
		//! Returns true when the given file path physically exists.
		fpl_api bool FileExists(const char *filePath);
		//! Copies the given source file to the target path and returns true when copy was successful. Target path must include the full path to the file. When overwrite is set, the target file path will always be overwritten.
		fpl_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
		//! Deletes the given file without confirmation and returns true when the deletion was successful.
		fpl_api bool FileDelete(const char *filePath);

		//! Creates all the directories in the path when not exists.
		fpl_api bool CreateDirectories(const char *path);
		//! Returns true when the given directory path physically exists.
		fpl_api bool DirectoryExists(const char *path);
		//! Deletes the given directory without confirmation and returns true when the deletion was successful. When recursive is set, all files and folders in sub-directories will be deleted as well.
		fpl_api bool RemoveEmptyDirectory(const char *path);
		//! Iterates through files / directories in the given directory. The path must contain the filter as well. Returns true when there was a first entry found.
		fpl_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry);
		//! Get next file entry from iterating through files / directories. Returns false when there is no entry found.
		fpl_api bool ListFilesNext(FileEntry &nextEntry);
		//! Releases opened resources from iterating through files / directories.
		fpl_api void ListFilesEnd(FileEntry &lastEntry);

		/** \}*/
	}

	//! Directory and paths functions
	namespace paths {
		/**
		  * \defgroup Paths Path functions
		  * \brief Functions for retrieving paths like HomePath, ExecutablePath, etc.
		  * \{
		  */

		//! Returns the full path to this executable, including the executable file name. Result is written in the required destination buffer.
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
		//! Returns the full path to your home directory. Result is written in the required destination buffer.
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen);
		//! Returns the path from the given source path. Result is written in the required destination buffer.
		fpl_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen);
		//! Returns the file extension from the given source path.
		fpl_api char *ExtractFileExtension(const char *sourcePath);
		//! Returns the file name including the file extension from the given source path.
		fpl_api char *ExtractFileName(const char *sourcePath);
		//! Changes the file extension on the given source path and writes the result into the destination path. Returns the pointer of the destination path. Result is written in the required destination buffer.
		fpl_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen);
		//! Combines all included path by the systems path separator. Returns the pointer of the destination path. Result is written in the required destination buffer.
		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);

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
			Char,
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

		//! Gets and removes the top event from the internal queue and fills out the "event" argument. Returns false when there are no events left, otherwise true.
		fpl_api bool PollWindowEvent(Event &ev);

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

		//! Returns true when the window is active.
		fpl_api bool IsWindowRunning();
		//! Processes the message queue of the window.
		fpl_api bool WindowUpdate();
		//! Forces the window to redraw or swap the back/front buffer.
		fpl_api void WindowFlip();
		//! Enables or disables the window cursor.
		fpl_api void SetWindowCursorEnabled(const bool value);
		//! Returns the inner window area.
		fpl_api WindowSize GetWindowArea();
		//! Resizes the window to fit the inner area to the given size.
		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height);
		//! Returns true when the window is resizable.
		fpl_api bool IsWindowResizable();
		//! Enables or disables the ability to resize the window.
		fpl_api void SetWindowResizeable(const bool value);
		//! Enables or disables fullscreen mode
		fpl_api void SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth = 0, const uint32_t fullscreenHeight = 0, const uint32_t refreshRate = 0);
		//! Returns true when the window is in fullscreen mode
		fpl_api bool IsWindowFullscreen();
		//! Returns the absolute window position.
		fpl_api WindowPosition GetWindowPosition();
		//! Sets the window absolut position to the given coordinates.
		fpl_api void SetWindowPosition(const int32_t left, const int32_t top);
		//! Sets the window title
		fpl_api void SetWindowTitle(const char *title);

		/*\}*/

		/**
		  * \defgroup WindowClipboard Clipboard functions
		  * \brief Functions for reading/writting clipboard data
		  * \{
		  */

		//! Returns the current clipboard ansi text
		fpl_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
		//! Returns the current clipboard wide text
		fpl_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
		//! Overwrites the current clipboard ansi text with the given one.
		fpl_api bool SetClipboardText(const char *ansiSource);
		//! Overwrites the current clipboard wide text with the given one.
		fpl_api bool SetClipboardText(const wchar_t *wideSource);

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

		//! Video backbuffer container. Use this for accessing the pixels directly. Use with care!
		struct VideoBackBuffer {
			//! The 32-bit pixel top-down array, format: 0xAABBGGRR. Do not modify before WindowUpdate
			uint32_t *pixels;
			//! The width of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t width;
			//! The height of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t height;
			//! The size of one scanline. Do not modify, it will be set automatically.
			size_t stride;
		};

		//! Returns the pointer to the video software context.
		fpl_api VideoBackBuffer *GetVideoBackBuffer();
		//! Resizes the current video backbuffer
		fpl_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height);

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

		//! Start playing audio
		fpl_api AudioResult PlayAudio();
		//! Stop playing audio
		fpl_api AudioResult StopAudio();
		//! Returns the native format for the current audio device
		fpl_api const AudioDeviceFormat &GetAudioHardwareFormat();
		//! Overwrites the audio client read callback. This has no effect when audio is already playing, you have to call it when audio is in a stopped state.
		fpl_api void SetAudioClientReadCallback(AudioClientReadFunction *newCallback, void *userData);

		/** \}*/
	};
#endif
}

//
// Platform specific defines/includes and forward declarations
//
#if defined(FPL_PLATFORM_WIN32)
// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_api int main(int argc, char **args);
#endif // FPL_PLATFORM_WIN32

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

// ****************************************************************************
//
// Common Functions (Non-Platform specific)
//
// ****************************************************************************

// Standard includes
#include <stdarg.h> // va_start, va_end, va_list, va_arg
#include <stdio.h> // fprintf, vfprintf, vsprintf

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
	namespace platform {
		//
		// Platform constants
		//
#	if defined(FPL_PLATFORM_WIN32)
		fpl_constant char PATH_SEPARATOR = '\\';
		fpl_constant char FILE_EXT_SEPARATOR = '.';
#	else
		fpl_constant char PATH_SEPARATOR = '/';
		fpl_constant char FILE_EXT_SEPARATOR = '.';
#	endif
	} // platform

	namespace common {
		//
		// Internal types and functions
		//
		fpl_constant uint32_t MAX_LAST_ERROR_STRING_LENGTH = 1024;
#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		fpl_constant size_t MAX_ERRORSTATE_COUNT = 1024;
#	else
		fpl_constant size_t MAX_ERRORSTATE_COUNT = 1;
#	endif

		struct ErrorState {
			char errors[MAX_ERRORSTATE_COUNT][MAX_LAST_ERROR_STRING_LENGTH];
			size_t count;
		};

		fpl_globalvar ErrorState *global__LastErrorState = nullptr;

#	if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		fpl_internal_inline void PushError_Formatted(const char *format, va_list &argList) {
			ErrorState *state = global__LastErrorState;
			if (state != nullptr) {
				FPL_ASSERT(format != nullptr);
				char buffer[MAX_LAST_ERROR_STRING_LENGTH];
				vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
				uint32_t messageLen = strings::GetAnsiStringLength(buffer);
				FPL_ASSERT(state->count < MAX_ERRORSTATE_COUNT);
				size_t errorIndex = state->count++;
				strings::CopyAnsiString(buffer, messageLen, state->errors[errorIndex], MAX_LAST_ERROR_STRING_LENGTH);
#		if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
				console::ConsoleError(buffer);
#		endif
			}
		}
#	else
		fpl_internal_inline void PushError_Formatted(const char *format, va_list &argList) {
			ErrorState *state = global__LastErrorState;
			if (state != nullptr) {
				FPL_ASSERT(format != nullptr);
				char buffer[MAX_LAST_ERROR_STRING_LENGTH];
				vsnprintf(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
				uint32_t messageLen = strings::GetAnsiStringLength(buffer);
				strings::CopyAnsiString(buffer, messageLen, state->errors[0], MAX_LAST_ERROR_STRING_LENGTH);
#		if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
				console::ConsoleError(buffer);
#		endif
			}
		}
#	endif // FPL_ENABLE_MULTIPLE_ERRORSTATES

		fpl_internal_inline void PushError(const char *format, ...) {
			va_list valist;
			va_start(valist, format);
			PushError_Formatted(format, valist);
			va_end(valist);
		}

		// Maximum number of threads you can have in your process
		fpl_constant uint32_t MAX_THREAD_COUNT = 64;

		// Maximum number of signals you can wait for
		fpl_constant uint32_t MAX_SIGNAL_COUNT = 256;

		struct ThreadState {
			threading::ThreadContext mainThread;
			threading::ThreadContext threads[MAX_THREAD_COUNT];
		};

		fpl_globalvar ThreadState global__ThreadState = {};

		fpl_internal_inline threading::ThreadContext *GetThreadContext() {
			threading::ThreadContext *result = nullptr;
			for (uint32_t index = 0; index < MAX_THREAD_COUNT; ++index) {
				threading::ThreadContext *thread = global__ThreadState.threads + index;
				uint32_t state = atomics::AtomicLoadU32((volatile uint32_t *)&thread->currentState);
				if (state == (uint32_t)threading::ThreadState::Stopped) {
					result = thread;
					break;
				}
			}
			return(result);
		}

	} // common

	//
	// Common Audio
	//
#if defined(FPL_ENABLE_AUDIO)
	namespace audio {
		struct CommonAudioState {
			AudioDeviceFormat internalFormat;
			AudioClientReadFunction *clientReadCallback;
			void *clientUserData;
		};

		fpl_internal_inline uint32_t GetAudioSampleSizeInBytes(const AudioFormatType format) {
			switch (format) {
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
		fpl_internal_inline const char *GetAudioFormatString(AudioFormatType format) {
			uint32_t index = (uint32_t)format;
			FPL_ASSERT(index < FPL_ARRAYCOUNT(AUDIO_FORMAT_TYPE_STRINGS));
			const char *result = AUDIO_FORMAT_TYPE_STRINGS[index];
			return(result);
		}

		fpl_constant char *AUDIO_DRIVER_TYPE_STRINGS[] = {
			"None",
			"Auto",
			"DirectSound",
		};
		fpl_internal_inline const char *GetAudioDriverString(AudioDriverType driver) {
			uint32_t index = (uint32_t)driver;
			FPL_ASSERT(index < FPL_ARRAYCOUNT(AUDIO_DRIVER_TYPE_STRINGS));
			const char *result = AUDIO_DRIVER_TYPE_STRINGS[index];
			return(result);
		}

		fpl_internal_inline uint32_t GetAudioBufferSizeInFrames(uint32_t sampleRate, uint32_t bufferSizeInMilliSeconds) {
			uint32_t result = (sampleRate / 1000) * bufferSizeInMilliSeconds;
			return(result);
		}

		fpl_internal_inline uint32_t ReadAudioFramesFromClient(const CommonAudioState &commonAudio, uint32_t frameCount, void *pSamples) {
			uint32_t outputSamplesWritten = 0;
			if (commonAudio.clientReadCallback != nullptr) {
				outputSamplesWritten = commonAudio.clientReadCallback(commonAudio.internalFormat, frameCount, pSamples, commonAudio.clientUserData);
			}
			return outputSamplesWritten;
		}

		// Forward declarations
		fpl_internal void ReleaseAudio();
		fpl_internal AudioResult InitAudio(const AudioSettings &audioSettings);
	} // audio
#endif // FPL_ENABLE_AUDIO

	//
	// Common Strings
	//
	namespace strings {
		fpl_api uint32_t GetAnsiStringLength(const char *str) {
			uint32_t result = 0;
			if (str != nullptr) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api uint32_t GetWideStringLength(const wchar_t *str) {
			uint32_t result = 0;
			if (str != nullptr) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
			char *result = nullptr;
			if ((source != nullptr && dest != nullptr) && ((sourceLen + 1) <= maxDestLen)) {
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

		fpl_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen) {
			char *result = nullptr;
			if (source != nullptr) {
				uint32_t sourceLen = GetAnsiStringLength(source);
				char *result = CopyAnsiString(source, sourceLen, dest, maxDestLen);
			} else {
				// @TODO(final): Do we want to push a error here?
			}
			return(result);
		}

		fpl_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
			wchar_t *result = nullptr;
			if ((source != nullptr && dest != nullptr) && ((sourceLen + 1) <= maxDestLen)) {
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

		fpl_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen) {
			wchar_t *result = nullptr;
			if (source != nullptr) {
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
		fpl_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment) {
			if (size == 0) {
				common::PushError("Memory size parameter must be greater than zero");
				return nullptr;
			}
			if ((alignment == 0) || (alignment & (alignment - 1))) {
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

		fpl_api void MemoryAlignedFree(void *ptr) {
			if (ptr == nullptr) {
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

		fpl_api void MemoryClear(void *mem, const size_t size) {
			if (mem == nullptr) {
				common::PushError("Memory parameter are not allowed to be null");
				return;
			}
			if (size == 0) {
				common::PushError("Size parameter must be greater than zero");
				return;
			}
			if (size % 8 == 0) {
				FPL_CLEARMEM(uint64_t, mem, size, MEM_SHIFT_64, MEM_MASK_64);
			} else if (size % 4 == 0) {
				FPL_CLEARMEM(uint32_t, mem, size, MEM_SHIFT_32, MEM_MASK_32);
			} else if (size % 2 == 0) {
				FPL_CLEARMEM(uint16_t, mem, size, MEM_SHIFT_16, MEM_MASK_16);
			} else {
				FPL_CLEARMEM(uint8_t, mem, size, 0, 0);
			}
		}

		fpl_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem) {
			if (sourceMem == nullptr) {
				common::PushError("Source memory parameter are not allowed to be null");
				return;
			}
			if (sourceSize == 0) {
				common::PushError("Source size parameter must be greater than zero");
				return;
			}
			if (targetMem == nullptr) {
				common::PushError("Target memory parameter are not allowed to be null");
				return;
			}
			if (sourceSize % 8 == 0) {
				FPL_COPYMEM(uint64_t, sourceMem, sourceSize, targetMem, MEM_SHIFT_64, MEM_MASK_64);
			} else if (sourceSize % 4 == 0) {
				FPL_COPYMEM(uint32_t, sourceMem, sourceSize, targetMem, MEM_SHIFT_32, MEM_MASK_32);
			} else if (sourceSize % 2 == 0) {
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
		fpl_api void *AtomicExchangePtr(volatile void **target, const void *value) {
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

		fpl_api void *AtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
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

		fpl_api bool IsAtomicCompareAndExchangePtr(volatile void **dest, const void *comparand, const void *exchange) {
			FPL_ASSERT(dest != nullptr);
#		if defined(FPL_ARCH_X64)
			bool result = IsAtomicCompareAndExchangeU64((volatile uint64_t *)dest, (uint64_t)comparand, (uint64_t)exchange);
#		elif defined(FPL_ARCH_X86)
			bool result = IsAtomicCompareAndExchangeU32((volatile uint32_t *)dest, (uint32_t)comparand, (uint32_t)exchange);
#		else
#			error "Unsupported architecture/platform!"
#endif
			return (result);
		}

		fpl_api void *AtomicLoadPtr(volatile void **source) {
#		if defined(FPL_ARCH_X64)
			void *result = (void *)AtomicLoadU64((volatile uint64_t *)source);
#		elif defined(FPL_ARCH_X86)
			void *result = (void *)AtomicLoadU32((volatile uint32_t *)source);
#		else
#			error "Unsupported architecture/platform!"
#		endif
			return(result);
		}

		fpl_api void AtomicStorePtr(volatile void **dest, const void *value) {
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
		fpl_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen) {
			if (sourcePath == nullptr) {
				return nullptr;
			}
			uint32_t sourceLen = strings::GetAnsiStringLength(sourcePath);
			if (sourceLen == 0) {
				return nullptr;
			}
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (sourceLen + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, sourceLen + 1);
				return nullptr;
			}

			char *result = nullptr;
			if (sourcePath) {
				int copyLen = 0;
				char *chPtr = (char *)sourcePath;
				while (*chPtr) {
					if (*chPtr == platform::PATH_SEPARATOR) {
						copyLen = (int)(chPtr - sourcePath);
					}
					++chPtr;
				}
				if (copyLen) {
					result = strings::CopyAnsiString(sourcePath, copyLen, destPath, maxDestLen);
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
						if (*chPtr == platform::FILE_EXT_SEPARATOR) {
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
					if (*chPtr == platform::PATH_SEPARATOR) {
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

		fpl_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen) {
			if (filePath == nullptr) {
				return nullptr;
			}
			if (newFileExtension == nullptr) {
				return nullptr;
			}
			uint32_t pathLen = strings::GetAnsiStringLength(filePath);
			if (pathLen == 0) {
				return nullptr;
			}
			uint32_t extLen = strings::GetAnsiStringLength(newFileExtension);

			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (pathLen + extLen + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, pathLen + extLen + 1);
				return nullptr;
			}

			char *result = nullptr;
			if (filePath != nullptr) {
				// Find last path
				char *chPtr = (char *)filePath;
				char *lastPathSeparatorPtr = nullptr;
				while (*chPtr) {
					if (*chPtr == platform::PATH_SEPARATOR) {
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
				char *lastExtSeparatorPtr = nullptr;
				while (*chPtr) {
					if (*chPtr == platform::FILE_EXT_SEPARATOR) {
						lastExtSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				uint32_t copyLen;
				if (lastExtSeparatorPtr != nullptr) {
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

		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
			if (pathCount == 0) {
				return nullptr;
			}
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestPathLen == 0) {
				common::PushError("Max dest path len parameter must be greater than zero");
				return nullptr;
			}

			uint32_t curDestPosition = 0;
			char *currentDestPtr = destPath;
			va_list vargs;
			va_start(vargs, pathCount);
			for (uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
				char *path = va_arg(vargs, char *);
				uint32_t pathLen = strings::GetAnsiStringLength(path);
				bool requireSeparator = pathIndex < (pathCount - 1);
				uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
				FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
				strings::CopyAnsiString(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
				currentDestPtr += pathLen;
				if (requireSeparator) {
					*currentDestPtr++ = platform::PATH_SEPARATOR;
				}
				curDestPosition += requiredPathLen;
			}
			*currentDestPtr = 0;
			va_end(vargs);
			return destPath;
		}

	} // paths

} // fpl

// ****************************************************************************
//
// WIN32 Platform
//
// ****************************************************************************
#if defined(FPL_PLATFORM_WIN32)
#	include <windowsx.h>	// Macros for window messages
#	include <shlobj.h>		// SHGetFolderPath
#	include <intrin.h>		// Interlock*

#	include <xinput.h>		// XInputGetState

#	if defined(FPL_ARCH_X86)
#		define FPL_MEMORY_BARRIER() \
			LONG barrier; \
			_InterlockedOr(&barrier, 0);
#	elif defined(FPL_ARCH_X64)
		// @NOTE(final): No need for hardware memory fence on X64 because the hardware guarantees memory order always.
#		define FPL_MEMORY_BARRIER()
#	endif

	// @NOTE(final): Little macro to not write 5 lines of code all the time
#	define FPL_WIN32_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)GetProcAddress(libHandle, name); \
	if (target == nullptr) { \
		fpl::common::PushError("Failed getting '%s' from library '%s'", name, libName); \
		return false; \
	}

namespace fpl {
	namespace platform {
		//
		// XInputGetState
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

		struct Win32InputFunctions {
			struct {
				HMODULE xinputLibrary;
				win32_func_XInputGetState *xInputGetState = Win32XInputGetStateStub;
				win32_func_XInputGetCapabilities *xInputGetCapabilities = Win32XInputGetCapabilitiesStub;
			} xinput;
		};
		fpl_globalvar Win32InputFunctions global__Win32__Input__Functions = {};

		//
		// WGL and Extensions
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
		fpl_globalvar Win32OpenGLFunctions global__Win32__OpenGL_Functions = {};

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

//
// OLE32
//
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
		fpl_globalvar Win32APIFunctions global__Win32__API__Functions = {};

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
#			define win32_setWindowLongPtr global__Win32__API__Functions.user.setWindowLongPtrA
#		else
#			define win32_setWindowLongPtr global__Win32__API__Functions.user.setWindowLongA
#		endif
#		define win32_setWindowLong global__Win32__API__Functions.user.setWindowLongA
#		define win32_getWindowLong global__Win32__API__Functions.user.getWindowLongA
#		define win32_peekMessage global__Win32__API__Functions.user.peekMessageA
#		define win32_dispatchMessage global__Win32__API__Functions.user.dispatchMessageA
#		define win32_defWindowProc global__Win32__API__Functions.user.defWindowProcA
#		define win32_registerClassEx global__Win32__API__Functions.user.registerClassExA
#		define win32_unregisterClass global__Win32__API__Functions.user.unregisterClassA
#		define win32_createWindowEx global__Win32__API__Functions.user.createWindowExA
#		define win32_loadIcon global__Win32__API__Functions.user.loadIconA
#		define win32_loadCursor global__Win32__API__Functions.user.loadCursorA
#	else
#		define FPL_WIN32_CLASSNAME L"FPLWindowClassW"
#		define FPL_WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
		typedef wchar_t win32_char;
#		define win32_copyString strings::CopyWideString
#		define win32_getStringLength strings::GetWideStringLength
#		define win32_ansiToString strings::AnsiStringToWideString
#		define win32_wndclassex WNDCLASSEXW
#		if defined(FPL_ARCH_X64)
#			define win32_setWindowLongPtr global__Win32__API__Functions.user.setWindowLongPtrW
#		else
#			define win32_setWindowLongPtr global__Win32__API__Functions.user.setWindowLongW
#		endif
#		define win32_setWindowLong global__Win32__API__Functions.user.setWindowLongW
#		define win32_getWindowLong global__Win32__API__Functions.user.getWindowLongW
#		define win32_peekMessage global__Win32__API__Functions.user.peekMessageW
#		define win32_dispatchMessage global__Win32__API__Functions.user.dispatchMessageW
#		define win32_defWindowProc global__Win32__API__Functions.user.defWindowProcW
#		define win32_registerClassEx global__Win32__API__Functions.user.registerClassExW
#		define win32_unregisterClass global__Win32__API__Functions.user.unregisterClassW
#		define win32_createWindowEx global__Win32__API__Functions.user.createWindowExW
#		define win32_loadIcon global__Win32__API__Functions.user.loadIconW
#		define win32_loadCursor global__Win32__API__Functions.user.loadCursorW
#	endif // UNICODE

#	if defined(FPL_ENABLE_WINDOW)
		struct Win32WindowState {
			win32_char windowClass[256];
			HWND windowHandle;
			HDC deviceContext;
			HCURSOR defaultCursor;
			WINDOWPLACEMENT lastWindowPlacement;
			uint32_t lastWindowWidth;
			uint32_t lastWindowHeight;
			bool isRunning;
			bool isCursorActive;
		};
		struct Win32XInputState {
			bool isConnected[XUSER_MAX_COUNT];
			LARGE_INTEGER lastDeviceSearchTime;
		};
#	else
		typedef void *Win32WindowState;
		typedef void *Win32XInputState;
#	endif // FPL_ENABLE_WINDOW

		struct Win32VideoState {
			VideoDriverType activeVideoDriver;
			union {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
				struct {
					HGLRC renderingContext;
				} opengl;
#		endif

#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				struct {
					video::VideoBackBuffer context;
					BITMAPINFO bitmapInfo;
				} software;
#		endif
			};
		};

		struct Win32ApplicationState {
			bool isInitialized;
			HINSTANCE appInstance;
			LARGE_INTEGER performanceFrequency;
		};
		fpl_globalvar Win32ApplicationState global__Win32__AppState = {};

		struct Win32State {
			Settings initSettings;
			Settings currentSettings;
			Win32WindowState window;
			Win32VideoState video;
			Win32XInputState xinput;
			InitFlags initFlags;
		};
		fpl_globalvar Win32State *global__Win32__State = nullptr;

#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		fpl_internal bool Win32InitVideoOpenGL(platform::Win32State *state, const VideoSettings &videoSettings) {
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			platform::Win32OpenGLFunctions &glFuncs = platform::global__Win32__OpenGL_Functions;

			//
			// Load OpenGL Library
			//
			{
				const char *openglLibraryName = "opengl32.dll";
				glFuncs.openglLibrary = LoadLibraryA("opengl32.dll");
				if (glFuncs.openglLibrary == nullptr) {
					common::PushError("Failed loading opengl library '%s'", openglLibraryName);
					return false;
				}

				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglGetProcAddress, platform::win32_func_wglGetProcAddress, "wglGetProcAddress");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglCreateContext, platform::win32_func_wglCreateContext, "wglCreateContext");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglDeleteContext, platform::win32_func_wglDeleteContext, "wglDeleteContext");
				FPL_WIN32_GET_FUNCTION_ADDRESS(glFuncs.openglLibrary, openglLibraryName, glFuncs.wglMakeCurrent, platform::win32_func_wglMakeCurrent, "wglMakeCurrent");
			}

			//
			// Prepare window for Opengl
			//
			HDC deviceContext = state->window.deviceContext;
			HWND handle = state->window.windowHandle;

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
			if (!pixelFormat) {
				common::PushError("Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			if (!wapi.gdi.setPixelFormat(deviceContext, pixelFormat, &pfd)) {
				common::PushError("Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			wapi.gdi.describePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

			//
			// Create opengl rendering context
			//
			HGLRC legacyRenderingContext = glFuncs.wglCreateContext(deviceContext);
			if (!legacyRenderingContext) {
				common::PushError("Failed creating Legacy OpenGL Rendering Context for DC '%x')", deviceContext);
				return false;
			}

			if (!glFuncs.wglMakeCurrent(deviceContext, legacyRenderingContext)) {
				common::PushError("Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')", deviceContext, legacyRenderingContext);
				glFuncs.wglDeleteContext(legacyRenderingContext);
				return false;
			}

			// Load WGL Extensions
			glFuncs.wglSwapIntervalExt = (platform::win32_func_wglSwapIntervalEXT *)glFuncs.wglGetProcAddress("wglSwapIntervalEXT");
			glFuncs.wglChoosePixelFormatArb = (platform::win32_func_wglChoosePixelFormatARB *)glFuncs.wglGetProcAddress("wglChoosePixelFormatARB");
			glFuncs.wglCreateContextAttribsArb = (platform::win32_func_wglCreateContextAttribsARB *)glFuncs.wglGetProcAddress("wglCreateContextAttribsARB");

			// Disable legacy context
			glFuncs.wglMakeCurrent(nullptr, nullptr);

			HGLRC activeRenderingContext;
			if (videoSettings.profile != VideoCompabilityProfile::Legacy) {
				// @NOTE(final): This is only available in OpenGL 3.0+
				if (!(videoSettings.majorVersion >= 3 && videoSettings.minorVersion >= 0)) {
					common::PushError("You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings");
					return false;
				}

				if (glFuncs.wglChoosePixelFormatArb == nullptr) {
					common::PushError("wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
					return false;
				}
				if (glFuncs.wglCreateContextAttribsArb == nullptr) {
					common::PushError("wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
					return false;
				}

				int contextAttribIndex = 0;
				int contextAttribList[20 + 1] = {};
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MAJOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.majorVersion;
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MINOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.minorVersion;
				if (videoSettings.profile == VideoCompabilityProfile::Core) {
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_PROFILE_MASK_ARB;
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
				} else {
					FPL_ASSERT(videoSettings.profile == VideoCompabilityProfile::Forward);
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FLAGS_ARB;
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
				}

				// Create modern opengl rendering context
				HGLRC modernRenderingContext = glFuncs.wglCreateContextAttribsArb(deviceContext, 0, contextAttribList);
				if (modernRenderingContext) {
					if (!glFuncs.wglMakeCurrent(deviceContext, modernRenderingContext)) {
						common::PushError("Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and DC '%x') -> Fallback to legacy context", videoSettings.majorVersion, videoSettings.minorVersion, deviceContext);

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
					common::PushError("Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and DC '%x') -> Fallback to legacy context", videoSettings.majorVersion, videoSettings.minorVersion, deviceContext);

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

			state->video.opengl.renderingContext = activeRenderingContext;

			// Set vertical syncronisation if available
			if (glFuncs.wglSwapIntervalExt != nullptr) {
				int swapInterval = videoSettings.isVSync ? 1 : 0;
				glFuncs.wglSwapIntervalExt(swapInterval);
			}

			return true;
		}

		fpl_internal void Win32ReleaseVideoOpenGL(platform::Win32State *state) {
			platform::Win32OpenGLFunctions &glFuncs = platform::global__Win32__OpenGL_Functions;
			if (state->video.opengl.renderingContext) {
				glFuncs.wglMakeCurrent(nullptr, nullptr);
				glFuncs.wglDeleteContext(state->video.opengl.renderingContext);
				state->video.opengl.renderingContext = nullptr;
			}
			if (glFuncs.openglLibrary != nullptr) {
				FreeLibrary(glFuncs.openglLibrary);
				glFuncs = {};
			}
		}
#	endif // FPL_ENABLE_VIDEO_OPENGL

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		fpl_internal bool Win32InitVideoSoftware(platform::Win32State *state, const uint32_t width, const uint32_t height) {
			platform::Win32VideoState &videoState = state->video;

			// Allocate memory/fill context
			videoState.software = {};
			videoState.software.context.width = width;
			videoState.software.context.height = height;
			videoState.software.context.stride = videoState.software.context.width * sizeof(uint32_t);
			size_t size = videoState.software.context.stride * videoState.software.context.height;
			videoState.software.context.pixels = (uint32_t *)memory::MemoryAlignedAllocate(size, 16);

			// Clear to black by default
			// @NOTE(final): Bitmap is top-down, 0xAABBGGRR
			uint32_t *p = videoState.software.context.pixels;
			for (uint32_t y = 0; y < videoState.software.context.height; ++y) {
				uint32_t color = 0xFF000000;
				for (uint32_t x = 0; x < videoState.software.context.width; ++x) {
					*p++ = color;
				}
			}

			// Create DIB section from bitmapinfo
			videoState.software.bitmapInfo = {};
			videoState.software.bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			videoState.software.bitmapInfo.bmiHeader.biWidth = (LONG)videoState.software.context.width;
			videoState.software.bitmapInfo.bmiHeader.biHeight = (LONG)videoState.software.context.height;
			videoState.software.bitmapInfo.bmiHeader.biBitCount = 32;
			videoState.software.bitmapInfo.bmiHeader.biCompression = BI_RGB;
			videoState.software.bitmapInfo.bmiHeader.biPlanes = 1;
			videoState.software.bitmapInfo.bmiHeader.biSizeImage = (DWORD)size;

			return true;
		}

		fpl_internal void Win32ReleaseVideoSoftware(platform::Win32State *state) {
			platform::Win32VideoState &videoState = state->video;
			FPL_ASSERT(videoState.software.context.pixels != nullptr);
			memory::MemoryAlignedFree(videoState.software.context.pixels);
			videoState.software = {};
		}
#	endif // FPL_ENABLE_VIDEO_SOFTWARE

#	if defined(FPL_ENABLE_WINDOW)
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
			if (eventQueue->pushCount < MAX_EVENT_COUNT) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pushCount, 1);
				FPL_ASSERT(eventIndex < MAX_EVENT_COUNT);
				eventQueue->events[eventIndex] = event;
			}
		}

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
			// @TODO(final): Support for multi-monitor (Leave)!
			FPL_ASSERT(global__Win32__State != nullptr);
			Win32State *state = global__Win32__State;
			Win32APIFunctions &wapi = global__Win32__API__Functions;

			WindowSettings &settings = state->currentSettings.window;
			FPL_ASSERT(settings.isFullscreen);
			HWND handle = state->window.windowHandle;

			if (settings.isResizable) {
				win32_setWindowLongPtr(handle, GWL_STYLE, Win32ResizeableWindowStyle);
				win32_setWindowLongPtr(handle, GWL_EXSTYLE, Win32ResizeableWindowExtendedStyle);
			} else {
				win32_setWindowLongPtr(handle, GWL_STYLE, Win32NonResizableWindowStyle);
				win32_setWindowLongPtr(handle, GWL_EXSTYLE, Win32NonResizableWindowExtendedStyle);
			}

			wapi.user.setWindowPos(handle, HWND_NOTOPMOST, 0, 0, state->window.lastWindowWidth, state->window.lastWindowHeight, SWP_SHOWWINDOW | SWP_NOMOVE);
			wapi.user.setWindowPlacement(handle, &state->window.lastWindowPlacement);
			bool result = (wapi.user.changeDisplaySettingsA(nullptr, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
			wapi.user.showWindow(handle, SW_RESTORE);

			settings.isFullscreen = false;
			state->window.lastWindowPlacement = {};
			state->window.lastWindowWidth = state->window.lastWindowHeight = 0;

			return(result);
		}

		fpl_internal bool Win32EnterFullscreen(const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits) {
			// @TODO(final): Support for multi-monitor (Enter)!
			FPL_ASSERT(global__Win32__State != nullptr);
			WindowSettings &settings = global__Win32__State->currentSettings.window;
			FPL_ASSERT(!settings.isFullscreen);

			HWND windowHandle = global__Win32__State->window.windowHandle;
			HDC deviceContext = global__Win32__State->window.deviceContext;
			Win32APIFunctions &wapi = global__Win32__API__Functions;

			DWORD useRefreshRate = refreshRate;
			if (!useRefreshRate) {
				useRefreshRate = wapi.gdi.getDeviceCaps(deviceContext, VREFRESH);
			}

			DWORD useColourBits = colorBits;
			if (!useColourBits) {
				useColourBits = wapi.gdi.getDeviceCaps(deviceContext, BITSPIXEL);
			}

			DWORD useFullscreenWidth = fullscreenWidth;
			DWORD useFullscreenHeight = fullscreenHeight;
			if (!useFullscreenWidth || !useFullscreenHeight) {
				useFullscreenWidth = wapi.gdi.getDeviceCaps(deviceContext, HORZRES);
				useFullscreenHeight = wapi.gdi.getDeviceCaps(deviceContext, VERTRES);
			}

			win32_setWindowLong(windowHandle, GWL_STYLE, Win32FullscreenWindowStyle);
			win32_setWindowLong(windowHandle, GWL_EXSTYLE, Win32FullscreenWindowExtendedStyle);

			wapi.user.setWindowPos(windowHandle, HWND_TOPMOST, 0, 0, useFullscreenWidth, useFullscreenHeight, SWP_SHOWWINDOW);

			DEVMODEA fullscreenSettings = {};
			wapi.user.enumDisplaySettingsA(nullptr, 0, &fullscreenSettings);
			fullscreenSettings.dmPelsWidth = useFullscreenWidth;
			fullscreenSettings.dmPelsHeight = useFullscreenHeight;
			fullscreenSettings.dmBitsPerPel = useColourBits;
			fullscreenSettings.dmDisplayFrequency = useRefreshRate;
			fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
			bool result = (wapi.user.changeDisplaySettingsA(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);

			wapi.user.showWindow(windowHandle, SW_MAXIMIZE);

			settings.isFullscreen = result;

			return(result);
		}

		fpl_internal_inline float Win32XInputProcessStickValue(SHORT value, SHORT deadZoneThreshold) {
			float result = 0;
			if (value < -deadZoneThreshold) {
				result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
			} else if (value > deadZoneThreshold) {
				result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
			}
			return(result);
		}

		fpl_internal void Win32PollControllers(Win32State *win32State) {
			using namespace window;
			Win32InputFunctions &inputFuncs = global__Win32__Input__Functions;
			if (inputFuncs.xinput.xInputGetState != nullptr) {
				//
				// Detect new controller (Only in a fixed frequency)
				//
				if (win32State->xinput.lastDeviceSearchTime.QuadPart == 0) {
					QueryPerformanceCounter(&win32State->xinput.lastDeviceSearchTime);
				}
				LARGE_INTEGER currentDeviceSearchTime;
				QueryPerformanceCounter(&currentDeviceSearchTime);
				uint64_t deviceSearchDifferenceTimeInMs = ((currentDeviceSearchTime.QuadPart - win32State->xinput.lastDeviceSearchTime.QuadPart) / (global__Win32__AppState.performanceFrequency.QuadPart / 1000));
				if ((win32State->currentSettings.input.controllerDetectionFrequency == 0) || (deviceSearchDifferenceTimeInMs > win32State->currentSettings.input.controllerDetectionFrequency)) {
					win32State->xinput.lastDeviceSearchTime = currentDeviceSearchTime;
					for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
						XINPUT_STATE controllerState = {};
						if (inputFuncs.xinput.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
							if (!win32State->xinput.isConnected[controllerIndex]) {
								// Connected
								win32State->xinput.isConnected[controllerIndex] = true;
								Event ev = {};
								ev.type = EventType::Gamepad;
								ev.gamepad.type = GamepadEventType::Connected;
								ev.gamepad.deviceIndex = controllerIndex;
								PushEvent(ev);
							}
						} else {
							if (win32State->xinput.isConnected[controllerIndex]) {
								// Disonnected
								win32State->xinput.isConnected[controllerIndex] = false;
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
				for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) {
					if (win32State->xinput.isConnected[controllerIndex]) {
						XINPUT_STATE controllerState = {};
						if (inputFuncs.xinput.xInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
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
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
								ev.gamepad.state.dpadUp = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
								ev.gamepad.state.dpadDown = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
								ev.gamepad.state.dpadLeft = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
								ev.gamepad.state.dpadRight = { true };

							// Action buttons
							if (pad->wButtons & XINPUT_GAMEPAD_A)
								ev.gamepad.state.actionA = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_B)
								ev.gamepad.state.actionB = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_X)
								ev.gamepad.state.actionX = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_Y)
								ev.gamepad.state.actionY = { true };

							// Center buttons
							if (pad->wButtons & XINPUT_GAMEPAD_START)
								ev.gamepad.state.start = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_BACK)
								ev.gamepad.state.back = { true };

							// Shoulder buttons
							if (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
								ev.gamepad.state.leftShoulder = { true };
							if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
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
			if (mouseEventType == MouseEventType::Wheel) {
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
			}
			PushEvent(newEvent);
		}

		fpl_internal window::Key Win32MapVirtualKey(const uint64_t keyCode) {
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

		fpl_internal_inline bool Win32IsKeyDown(const uint64_t keyCode) {
			Win32APIFunctions &api = global__Win32__API__Functions;
			bool result = (api.user.getAsyncKeyState((int)keyCode) & 0x8000) > 0;
			return(result);
		}

		LRESULT CALLBACK Win32MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			using namespace window;

			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *win32State = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;

			if (!win32State->window.windowHandle) {
				return win32_defWindowProc(hwnd, msg, wParam, lParam);
			}

			LRESULT result = 0;
			switch (msg) {
				case WM_DESTROY:
				case WM_CLOSE:
				{
					win32State->window.isRunning = false;
				} break;

				case WM_SIZE:
				{
#				if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					if (win32State->video.activeVideoDriver == VideoDriverType::Software) {
						if (win32State->initSettings.video.isAutoSize) {
							uint32_t w = LOWORD(lParam);
							uint32_t h = HIWORD(lParam);
							if ((w != win32State->video.software.context.width) ||
								(h != win32State->video.software.context.height)) {
								platform::Win32ReleaseVideoSoftware(win32State);
								platform::Win32InitVideoSoftware(win32State, w, h);
							}
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
					bool wasDown = ((lParam & (1 << 30)) != 0);
					bool isDown = ((lParam & (1 << 31)) == 0);

					bool altKeyWasDown = platform::Win32IsKeyDown(VK_MENU);
					bool shiftKeyWasDown = platform::Win32IsKeyDown(VK_LSHIFT);
					bool ctrlKeyWasDown = platform::Win32IsKeyDown(VK_LCONTROL);
					bool superKeyWasDown = platform::Win32IsKeyDown(VK_LMENU);

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
					platform::Win32PushKeyboardEvent(keyEventType, keyCode, modifiers, isDown);

					if (wasDown != isDown) {
						if (isDown) {
							if (keyCode == VK_F4 && altKeyWasDown) {
								win32State->window.isRunning = 0;
							}
						}
					}
				} break;

				case WM_CHAR:
				{
					uint64_t keyCode = wParam;
					KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
					platform::Win32PushKeyboardEvent(KeyboardEventType::Char, keyCode, modifiers, 0);
				} break;

				case WM_ACTIVATE:
				{
					Event newEvent = {};
					newEvent.type = EventType::Window;
					if (wParam == WA_INACTIVE) {
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
					if (msg == WM_LBUTTONDOWN) {
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
					if (msg == WM_RBUTTONDOWN) {
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
					if (msg == WM_MBUTTONDOWN) {
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
					if (win32State->window.isCursorActive) {
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

		fpl_internal bool Win32InitWindow(Win32State &win32State, const Settings &initSettings) {
			Win32APIFunctions &wapi = global__Win32__API__Functions;

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
			if (initSettings.video.driverType == VideoDriverType::OpenGL) {
				windowClass.style |= CS_OWNDC;
			}
			win32_copyString(windowClass.lpszClassName, win32_getStringLength(windowClass.lpszClassName), win32State.window.windowClass, FPL_ARRAYCOUNT(win32State.window.windowClass));
			if (win32_registerClassEx(&windowClass) == 0) {
				common::PushError("Failed Registering Window Class '%s'", win32State.window.windowClass);
				return false;
			}

			// Allocate event queue
			size_t eventQueueMemorySize = sizeof(EventQueue);
			void *eventQueueMemory = memory::MemoryAlignedAllocate(eventQueueMemorySize, 16);
			if (eventQueueMemory == nullptr) {
				common::PushError("Failed Allocating Event Queue Memory with size '%zu'", eventQueueMemorySize);
				return false;
			}
			global__EventQueue = (EventQueue *)eventQueueMemory;

			// Create window
			platform::win32_char windowTitleBuffer[1024];
			platform::win32_char *windowTitle = FPL_WIN32_UNNAMED_WINDOW;
			WindowSettings &currentWindowSettings = win32State.currentSettings.window;
			currentWindowSettings.isFullscreen = false;
			if (strings::GetAnsiStringLength(initSettings.window.windowTitle) > 0) {
				win32_ansiToString(initSettings.window.windowTitle, strings::GetAnsiStringLength(initSettings.window.windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
				windowTitle = windowTitleBuffer;
				strings::CopyAnsiString(initSettings.window.windowTitle, strings::GetAnsiStringLength(initSettings.window.windowTitle), currentWindowSettings.windowTitle, FPL_ARRAYCOUNT(currentWindowSettings.windowTitle));
			}

			DWORD style;
			DWORD exStyle;
			if (initSettings.window.isResizable) {
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
			if ((initSettings.window.windowWidth > 0) &&
				(initSettings.window.windowHeight > 0)) {
				RECT windowRect;
				windowRect.left = 0;
				windowRect.top = 0;
				windowRect.right = initSettings.window.windowWidth;
				windowRect.bottom = initSettings.window.windowHeight;
				wapi.user.adjustWindowRect(&windowRect, style, false);
				windowWidth = windowRect.right - windowRect.left;
				windowHeight = windowRect.bottom - windowRect.top;
			} else {
				// @NOTE(final): Operating system decide how big the window should be.
				windowWidth = CW_USEDEFAULT;
				windowHeight = CW_USEDEFAULT;
			}


			// Create window
			win32State.window.windowHandle = win32_createWindowEx(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, windowClass.hInstance, nullptr);
			if (win32State.window.windowHandle == nullptr) {
				common::PushError("Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", win32State.window.windowClass, windowWidth, windowHeight, windowWidth, windowHeight);
				return false;
			}

			// Get actual window size and store results
			currentWindowSettings.windowWidth = windowWidth;
			currentWindowSettings.windowHeight = windowHeight;
			RECT clientRect;
			if (wapi.user.getClientRect(global__Win32__State->window.windowHandle, &clientRect)) {
				currentWindowSettings.windowWidth = clientRect.right - clientRect.left;
				currentWindowSettings.windowHeight = clientRect.bottom - clientRect.top;
			}

			// Get device context so we can swap the back and front buffer
			win32State.window.deviceContext = wapi.user.getDC(win32State.window.windowHandle);
			if (win32State.window.deviceContext == nullptr) {
				common::PushError("Failed aquiring device context from window '%d'", win32State.window.windowHandle);
				return false;
			}

			// Enter fullscreen if needed
			if (initSettings.window.isFullscreen) {
				window::SetWindowFullscreen(true, initSettings.window.fullscreenWidth, initSettings.window.fullscreenHeight);
			}

#		if defined(FPL_ENABLE_VIDEO)
			// Create opengl rendering context if required
			win32State.video.activeVideoDriver = VideoDriverType::None;
			switch (initSettings.video.driverType) {
#			if defined(FPL_ENABLE_VIDEO_OPENGL)
				case VideoDriverType::OpenGL:
				{
					bool openglResult = platform::Win32InitVideoOpenGL(&win32State, initSettings.video);
					if (!openglResult) {
						common::PushError("Failed initializing OpenGL for window '%d'/'%s'", win32State.window.windowHandle, win32State.window.windowClass);
						return false;
					}
					win32State.video.activeVideoDriver = VideoDriverType::OpenGL;
				} break;
#			endif // FPL_ENABLE_VIDEO_OPENGL

#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				case VideoDriverType::Software:
				{
					bool softwareResult = platform::Win32InitVideoSoftware(&win32State, windowWidth, windowHeight);
					if (!softwareResult) {
						common::PushError("Failed creating software rendering buffer for window '%d'/'%s'", win32State.window.windowHandle, win32State.window.windowClass);
						return false;
					}
					win32State.video.activeVideoDriver = VideoDriverType::Software;
				} break;
#			endif // FPL_ENABLE_VIDEO_SOFTWARE

				default:
					break;
			}
#		endif // FPL_ENABLE_VIDEO

			// Show window
			wapi.user.showWindow(win32State.window.windowHandle, SW_SHOW);
			wapi.user.updateWindow(win32State.window.windowHandle);

			// Cursor is visible at start
			win32State.window.defaultCursor = windowClass.hCursor;
			win32State.window.isCursorActive = true;
			win32State.window.isRunning = true;

			return true;
		}

		fpl_internal void Win32ReleaseWindow(Win32State &win32State) {
			Win32APIFunctions &api = global__Win32__API__Functions;

			if (win32State.window.deviceContext != nullptr) {
				api.user.releaseDC(win32State.window.windowHandle, win32State.window.deviceContext);
				win32State.window.deviceContext = nullptr;
			}

			if (win32State.window.windowHandle != nullptr) {
				api.user.destroyWindow(win32State.window.windowHandle);
				win32State.window.windowHandle = nullptr;
				win32_unregisterClass(win32State.window.windowClass, global__Win32__AppState.appInstance);
			}

			if (global__EventQueue != nullptr) {
				memory::MemoryAlignedFree(global__EventQueue);
				global__EventQueue = nullptr;
			}
		}

		fpl_internal void Win32LoadXInput() {
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
			Win32InputFunctions &inputFuncs = global__Win32__Input__Functions;
			if (xinputLibrary) {
				inputFuncs.xinput.xinputLibrary = xinputLibrary;
				inputFuncs.xinput.xInputGetState = (win32_func_XInputGetState *)GetProcAddress(xinputLibrary, "XInputGetState");
				inputFuncs.xinput.xInputGetCapabilities = (win32_func_XInputGetCapabilities *)GetProcAddress(xinputLibrary, "XInputGetCapabilities");
			}
			if (inputFuncs.xinput.xInputGetState == nullptr) {
				inputFuncs.xinput.xInputGetState = Win32XInputGetStateStub;
			}
			if (inputFuncs.xinput.xInputGetCapabilities == nullptr) {
				inputFuncs.xinput.xInputGetCapabilities = Win32XInputGetCapabilitiesStub;
			}
		}

		fpl_internal void Win32UnloadXInput() {
			Win32InputFunctions &inputFuncs = global__Win32__Input__Functions;
			if (inputFuncs.xinput.xinputLibrary) {
				FreeLibrary(inputFuncs.xinput.xinputLibrary);
				inputFuncs.xinput.xinputLibrary = nullptr;
				inputFuncs.xinput.xInputGetState = Win32XInputGetStateStub;
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
			HMODULE shellapiLibrary = LoadLibraryA("shell32.dll");
			if (shellapiLibrary != nullptr) {
				win32_func_CommandLineToArgvW *commandLineToArgvW = (win32_func_CommandLineToArgvW *)GetProcAddress(shellapiLibrary, "CommandLineToArgvW");
				if (commandLineToArgvW != nullptr) {
					// Parse arguments and compute total UTF8 string length
					int executableFilePathArgumentCount = 0;
					wchar_t **executableFilePathArgs = commandLineToArgvW(L"", &executableFilePathArgumentCount);
					uint32_t executableFilePathLen = 0;
					for (int i = 0; i < executableFilePathArgumentCount; ++i) {
						if (i > 0) {
							// Include whitespace
							executableFilePathLen++;
						}
						uint32_t sourceLen = strings::GetWideStringLength(executableFilePathArgs[i]);
						uint32_t destLen = WideCharToMultiByte(CP_UTF8, 0, executableFilePathArgs[i], sourceLen, nullptr, 0, 0, 0);
						executableFilePathLen += destLen;
					}

					// @NOTE(final): Do not parse the arguments when there are no actual arguments, otherwise we will get back the executable arguments again.
					int actualArgumentCount = 0;
					wchar_t **actualArgs = nullptr;
					uint32_t actualArgumentsLen = 0;
					if (cmdLine != nullptr && strings::GetWideStringLength(cmdLine) > 0) {
						actualArgs = commandLineToArgvW(cmdLine, &actualArgumentCount);
						for (int i = 0; i < actualArgumentCount; ++i) {
							uint32_t sourceLen = strings::GetWideStringLength(actualArgs[i]);
							uint32_t destLen = WideCharToMultiByte(CP_UTF8, 0, actualArgs[i], sourceLen, nullptr, 0, 0, 0);
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
						for (int i = 0; i < executableFilePathArgumentCount; ++i) {
							if (i > 0) {
								*destArg++ = ' ';
							}
							wchar_t *sourceArg = executableFilePathArgs[i];
							uint32_t sourceArgLen = strings::GetWideStringLength(sourceArg);
							uint32_t destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, nullptr, 0, 0, 0);
							WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
							destArg += destArgLen;
						}
						*destArg++ = 0;
						LocalFree(executableFilePathArgs);
					}

					// Convert actual arguments to UTF8
					if (actualArgumentCount > 0) {
						FPL_ASSERT(actualArgs != nullptr);
						for (int i = 0; i < actualArgumentCount; ++i) {
							args.args[1 + i] = destArg;
							wchar_t *sourceArg = actualArgs[i];
							uint32_t sourceArgLen = strings::GetWideStringLength(sourceArg);
							uint32_t destArgLen = WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, nullptr, 0, 0, 0);
							WideCharToMultiByte(CP_UTF8, 0, sourceArg, sourceArgLen, destArg, destArgLen, 0, 0);
							destArg += destArgLen;
							*destArg++ = 0;
						}
						LocalFree(actualArgs);
					}
				}
				FreeLibrary(shellapiLibrary);
				shellapiLibrary = nullptr;
			}
			return(args);
		}

		fpl_internal Win32CommandLineUTF8Arguments Win32ParseAnsiArguments(LPSTR cmdLine) {
			Win32CommandLineUTF8Arguments result;
			if (cmdLine != nullptr) {
				uint32_t ansiSourceLen = strings::GetAnsiStringLength(cmdLine);
				uint32_t wideDestLen = MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, nullptr, 0);
				// @TODO(final): Can we use a stack allocation here?
				wchar_t *wideCmdLine = (wchar_t *)memory::MemoryAllocate(sizeof(wchar_t) * (wideDestLen + 1));
				MultiByteToWideChar(CP_ACP, 0, cmdLine, ansiSourceLen, wideCmdLine, wideDestLen);
				wideCmdLine[wideDestLen] = 0;
				result = Win32ParseWideArguments(wideCmdLine);
				memory::MemoryFree(wideCmdLine);
			} else {
				result = Win32ParseWideArguments(L"");
			}
			return(result);
		}

#	endif // FPL_ENABLE_WINDOW

		fpl_api bool Win32LoadAPI(Win32State &win32State) {
			Win32APIFunctions &wapi = global__Win32__API__Functions;

			// Shell32
			{
				const char *shellLibraryName = "shell32.dll";
				HMODULE library = wapi.shell.shellLibrary = LoadLibraryA(shellLibraryName);
				if (library == nullptr) {
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
				HMODULE library = wapi.user.userLibrary = LoadLibraryA(userLibraryName);
				if (library == nullptr) {
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
			}

			// GDI32
			{
				const char *gdiLibraryName = "gdi32.dll";
				HMODULE library = wapi.gdi.gdiLibrary = LoadLibraryA(gdiLibraryName);
				if (library == nullptr) {
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
				HMODULE library = wapi.ole.oleLibrary = LoadLibraryA(oleLibraryName);
				if (library == nullptr) {
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

		fpl_api void Win32UnloadAPI() {
			Win32APIFunctions &api = global__Win32__API__Functions;
			if (api.ole.oleLibrary != nullptr) {
				FreeLibrary(api.ole.oleLibrary);
				api.ole = {};
			}
			if (api.gdi.gdiLibrary != nullptr) {
				FreeLibrary(api.gdi.gdiLibrary);
				api.gdi = {};
			}
			if (api.user.userLibrary != nullptr) {
				FreeLibrary(api.user.userLibrary);
				api.user = {};
			}
			if (api.shell.shellLibrary != nullptr) {
				FreeLibrary(api.shell.shellLibrary);
				api.shell = {};
			}
		}

		fpl_internal bool Win32ThreadWaitForMultiple(threading::ThreadContext **contexts, const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
			if (contexts == nullptr) {
				common::PushError("Contexts parameter are not allowed to be null");
				return false;
			}
			if (count > common::MAX_THREAD_COUNT) {
				common::PushError("Count parameter '%d' must be greater or equal than '%d'", count, common::MAX_THREAD_COUNT);
				return false;
			}
			HANDLE threadHandles[common::MAX_THREAD_COUNT];
			for (uint32_t index = 0; index < count; ++index) {
				threading::ThreadContext *context = contexts[index];
				if (context == nullptr) {
					common::PushError("Thread context for index '%d' are not allowed to be null", index);
					return false;
				}
				if (context->internalHandle.win32Handle == nullptr) {
					common::PushError("Thread handle for index '%d' are not allowed to be null", index);
					return false;
				}
				HANDLE handle = (HANDLE)context->internalHandle.win32Handle;
				threadHandles[index] = handle;
			}
			DWORD code = WaitForMultipleObjects(count, threadHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
			bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
			return(result);
		}

		fpl_internal bool Win32SignalWaitForMultiple(threading::ThreadSignal **signals, const uint32_t count, const bool waitForAll, const uint32_t maxMilliseconds) {
			if (signals == nullptr) {
				common::PushError("Signals parameter are not allowed to be null");
				return false;
			}
			if (count > common::MAX_SIGNAL_COUNT) {
				common::PushError("Count parameter '%d' must be greater or equal than '%d'", count, common::MAX_SIGNAL_COUNT);
				return false;
			}
			HANDLE signalHandles[common::MAX_SIGNAL_COUNT];
			for (uint32_t index = 0; index < count; ++index) {
				threading::ThreadSignal *availableSignal = signals[index];
				if (availableSignal == nullptr) {
					common::PushError("Signal for index '%d' are not allowed to be null", index);
					return false;
				}
				if (availableSignal->internalHandle.win32Handle == nullptr) {
					common::PushError("Signal handle for index '%d' are not allowed to be null", index);
					return false;
				}
				HANDLE handle = (HANDLE)availableSignal->internalHandle.win32Handle;
				signalHandles[index] = handle;
			}
			DWORD code = WaitForMultipleObjects(count, signalHandles, waitForAll ? TRUE : FALSE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
			bool result = (code != WAIT_TIMEOUT) && (code != WAIT_FAILED);
			return(result);
		}

		} // platform

		//
		// Win32 Atomics
		//
	namespace atomics {
		fpl_api void AtomicReadFence() {
			FPL_MEMORY_BARRIER();
			_ReadBarrier();
		}
		fpl_api void AtomicWriteFence() {
			FPL_MEMORY_BARRIER();
			_WriteBarrier();
		}
		fpl_api void AtomicReadWriteFence() {
			FPL_MEMORY_BARRIER();
			_ReadWriteBarrier();
		}

		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			FPL_ASSERT(target != nullptr);
			uint32_t result = _InterlockedExchange((volatile unsigned long *)target, value);
			return (result);
		}
		fpl_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value) {
			FPL_ASSERT(target != nullptr);
			int32_t result = _InterlockedExchange((volatile long *)target, value);
			return (result);
		}
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			FPL_ASSERT(target != nullptr);
			uint64_t result = _InterlockedExchange((volatile unsigned __int64 *)target, value);
			return (result);
		}
		fpl_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value) {
			FPL_ASSERT(target != nullptr);
			int64_t result = _InterlockedExchange64((volatile long long *)target, value);
			return (result);
		}

		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = _InterlockedExchangeAdd((volatile unsigned long *)value, addend);
			return (result);
		}
		fpl_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
			FPL_ASSERT(value != nullptr);
			int32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
			return (result);
		}
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint64_t result = _InterlockedExchangeAdd((volatile unsigned __int64 *)value, addend);
			return (result);
		}
		fpl_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
			FPL_ASSERT(value != nullptr);
			int64_t result = _InterlockedExchangeAdd64((volatile long long *)value, addend);
			return (result);
		}

		fpl_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = _InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
			return (result);
		}
		fpl_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			return (result);
		}
		fpl_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = _InterlockedCompareExchange((volatile unsigned __int64 *)dest, exchange, comparand);
			return (result);
		}
		fpl_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t result = _InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			return (result);
		}

		fpl_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t value = _InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t value = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t value = _InterlockedCompareExchange((volatile unsigned __int64 *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t value = _InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			bool result = (value == comparand);
			return (result);
		}

		fpl_api uint32_t AtomicLoadU32(volatile uint32_t *source) {
			uint32_t result = _InterlockedCompareExchange((volatile unsigned long *)source, 0, 0);
			return(result);
		}
		fpl_api uint64_t AtomicLoadU64(volatile uint64_t *source) {
			uint64_t result = _InterlockedCompareExchange((volatile unsigned __int64 *)source, 0, 0);
			return(result);
		}
		fpl_api int32_t AtomicLoadS32(volatile int32_t *source) {
			int32_t result = _InterlockedCompareExchange((volatile long *)source, 0, 0);
			return(result);
		}
		fpl_api int64_t AtomicLoadS64(volatile int64_t *source) {
			int64_t result = _InterlockedCompareExchange64((volatile __int64 *)source, 0, 0);
			return(result);
		}

		fpl_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
			_InterlockedExchange((volatile unsigned long *)dest, value);
		}
		fpl_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
			_InterlockedExchange((volatile unsigned __int64 *)dest, value);
		}
		fpl_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value) {
			_InterlockedExchange((volatile long *)dest, value);
		}
		fpl_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value) {
			_InterlockedExchange64((volatile __int64 *)dest, value);
		}
	} // atomics

	//
	// Win32 Hardware
	//
	namespace hardware {
		fpl_api uint32_t GetProcessorCoreCount() {
			SYSTEM_INFO sysInfo = {};
			GetSystemInfo(&sysInfo);
			// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
			uint32_t result = sysInfo.dwNumberOfProcessors;
			return(result);
		}

		fpl_api MemoryInfos GetSystemMemoryInfos() {
			MemoryInfos result = {};
			MEMORYSTATUSEX statex = {};
			statex.dwLength = sizeof(statex);
			ULONGLONG totalMemorySize;
			if (GetPhysicallyInstalledSystemMemory(&totalMemorySize) && GlobalMemoryStatusEx(&statex)) {
				result.totalPhysicalSize = totalMemorySize * 1024;
				result.availablePhysicalSize = statex.ullTotalPhys;
				result.usedPhysicalSize = result.availablePhysicalSize - statex.ullAvailPhys;
				result.totalVirtualSize = statex.ullTotalVirtual;
				result.usedVirtualSize = result.totalVirtualSize - statex.ullAvailVirtual;
				result.totalPageSize = statex.ullTotalPageFile;
				result.usedPageSize = result.totalPageSize - statex.ullAvailPageFile;
			}
			return(result);
		}

		fpl_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
			fpl_constant uint32_t CPU_BRAND_BUFFER_SIZE = 0x40;

			if (destBuffer == nullptr) {
				common::PushError("Dest buffer parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestBufferLen < (CPU_BRAND_BUFFER_SIZE + 1)) {
				common::PushError("Max dest buffer len parameter '%d' must be greater or equal than '%d'", maxDestBufferLen, CPU_BRAND_BUFFER_SIZE + 1);
				return nullptr;
			}

			// @TODO(final): __cpuid may not be available on other Win32 Compilers!

			int cpuInfo[4] = { -1 };
			char cpuBrandBuffer[CPU_BRAND_BUFFER_SIZE] = {};
			__cpuid(cpuInfo, 0x80000000);
			uint32_t extendedIds = cpuInfo[0];

			// Get the information associated with each extended ID. Interpret CPU brand string.
			uint32_t max = FPL_MIN(extendedIds, 0x80000004);
			for (uint32_t i = 0x80000002; i <= max; ++i) {
				__cpuid(cpuInfo, i);
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
		fpl_api void ConsoleOut(const char *text) {
			if (text != nullptr) {
				fprintf(stdout, text);
			}
		}
		fpl_api void ConsoleFormatOut(const char *format, ...) {
			if (format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stdout, format, vaList);
				va_end(vaList);
			}
		}
		fpl_api void ConsoleError(const char *text) {
			if (text != nullptr) {
				fprintf(stderr, "%s", text);
			}
		}
		fpl_api void ConsoleFormatError(const char *format, ...) {
			if (format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stderr, format, vaList);
				va_end(vaList);
			}
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
			if (context->runFunc != nullptr) {
				context->runFunc(*context, context->data);
			}
			atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
			return(result);
		}

		fpl_api ThreadContext *ThreadCreate(run_thread_function *runFunc, void *data, const bool autoStart) {
			ThreadContext *result = nullptr;
			ThreadContext *context = common::GetThreadContext();
			if (context != nullptr) {
				DWORD creationFlags = 0;
				DWORD threadId = 0;
				HANDLE handle = CreateThread(nullptr, 0, Win32ThreadProc, context, CREATE_SUSPENDED, &threadId);
				if (handle != nullptr) {
					context->currentState = ThreadState::Suspended;
					context->data = data;
					context->id = threadId;
					context->internalHandle.win32Handle = (void *)handle;
					context->runFunc = runFunc;
					if (autoStart) {
						ResumeThread(handle);
					}
					result = context;
				} else {
					common::PushError("Failed creating thread, error code: %d", GetLastError());
				}
			} else {
				common::PushError("All %d threads are in use, you cannot create until you free one", common::MAX_THREAD_COUNT);
			}
			return(result);
		}

		fpl_api void ThreadSleep(const uint32_t milliseconds) {
			Sleep((DWORD)milliseconds);
		}

		fpl_api bool ThreadSuspend(ThreadContext *context) {
			if (context == nullptr) {
				common::PushError("Context parameter are not allowed to be null");
				return false;
			}
			if (context->internalHandle.win32Handle == nullptr) {
				common::PushError("Thread context handle are not allowed to be null");
				return false;
			}
			HANDLE handle = (HANDLE)context->internalHandle.win32Handle;
			DWORD err = SuspendThread(handle);
			bool result = err != -1;
			if (result) {
				// @TODO(final): Is this really needed to use a atomic store for setting the thread state?
				atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Suspended);
			}
			return(result);
		}

		fpl_api bool ThreadResume(ThreadContext *context) {
			if (context == nullptr) {
				common::PushError("Context parameter are not allowed to be null");
				return false;
			}
			if (context->internalHandle.win32Handle == nullptr) {
				common::PushError("Thread context handle are not allowed to be null");
				return false;
			}
			HANDLE handle = (HANDLE)context->internalHandle.win32Handle;
			DWORD err = ResumeThread(handle);
			bool result = err != -1;
			if (result) {
				// @TODO(final): Is this really needed to use a atomic store for setting the thread state?
				atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Running);
			}
			return(result);
		}

		fpl_api void ThreadDestroy(ThreadContext *context) {
			if (context != nullptr && context->internalHandle.win32Handle != nullptr) {
				HANDLE handle = (HANDLE)context->internalHandle.win32Handle;
				TerminateThread(handle, 0);
				CloseHandle(handle);
				// @TODO(final): Is this really needed to use a atomic store for setting the thread state?
				atomics::AtomicStoreU32((volatile uint32_t *)&context->currentState, (uint32_t)ThreadState::Stopped);
				*context = {};
			}
		}

		fpl_api bool ThreadWaitForOne(ThreadContext *context, const uint32_t maxMilliseconds) {
			if (context == nullptr) {
				common::PushError("Context parameter are not allowed to be null");
				return false;
			}
			if (context->internalHandle.win32Handle == nullptr) {
				common::PushError("Thread context handle are not allowed to be null");
				return false;
			}
			HANDLE handle = (HANDLE)context->internalHandle.win32Handle;
			bool result = (WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
			return(result);
		}

		fpl_api bool ThreadWaitForAll(ThreadContext **contexts, const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32ThreadWaitForMultiple(contexts, count, true, maxMilliseconds);
			return(result);
		}

		fpl_api bool ThreadWaitForAny(ThreadContext **contexts, const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32ThreadWaitForMultiple(contexts, count, false, maxMilliseconds);
			return(result);
		}

		fpl_api ThreadMutex MutexCreate() {
			ThreadMutex result = {};
			InitializeCriticalSection(&result.internalHandle.win32CriticalSection);
			result.isValid = true;
			return(result);
		}

		fpl_api void MutexDestroy(ThreadMutex &mutex) {
			if (mutex.isValid) {
				DeleteCriticalSection(&mutex.internalHandle.win32CriticalSection);
				mutex = {};
			}
		}

		fpl_api bool MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds) {
			if (!mutex.isValid) {
				return false;
			}
			EnterCriticalSection(&mutex.internalHandle.win32CriticalSection);
			return true;
		}

		fpl_api bool MutexUnlock(ThreadMutex &mutex) {
			if (!mutex.isValid) {
				return false;
			}
			LeaveCriticalSection(&mutex.internalHandle.win32CriticalSection);
			return true;
		}

		fpl_api ThreadSignal SignalCreate() {
			ThreadSignal result = {};
			HANDLE handle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
			if (handle != nullptr) {
				result.isValid = true;
				result.internalHandle.win32Handle = handle;
			}
			return(result);
		}

		fpl_api void SignalDestroy(ThreadSignal &availableSignal) {
			if (availableSignal.internalHandle.win32Handle != nullptr) {
				HANDLE handle = (HANDLE)availableSignal.internalHandle.win32Handle;
				CloseHandle(handle);
				availableSignal = {};
			}
		}

		fpl_api bool SignalWaitForOne(ThreadSignal &availableSignal, const uint32_t maxMilliseconds) {
			if (availableSignal.internalHandle.win32Handle == nullptr) {
				return false;
			}
			HANDLE handle = (HANDLE)availableSignal.internalHandle.win32Handle;
			bool result = (WaitForSingleObject(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
			return(result);
		}

		fpl_api bool SignalWaitForAll(ThreadSignal **signals, const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32SignalWaitForMultiple(signals, count, true, maxMilliseconds);
			return(result);
		}

		fpl_api bool SignalWaitForAny(ThreadSignal **signals, const uint32_t count, const uint32_t maxMilliseconds) {
			bool result = platform::Win32SignalWaitForMultiple(signals, count, false, maxMilliseconds);
			return(result);
		}

		fpl_api bool SignalWakeUp(ThreadSignal &availableSignal) {
			if (availableSignal.internalHandle.win32Handle == nullptr) {
				return false;
			}
			HANDLE handle = (HANDLE)availableSignal.internalHandle.win32Handle;
			bool result = SetEvent(handle) == TRUE;
			return(result);
		}

		fpl_api bool SignalReset(ThreadSignal &availableSignal) {
			if (availableSignal.internalHandle.win32Handle == nullptr) {
				return false;
			}
			HANDLE handle = (HANDLE)availableSignal.internalHandle.win32Handle;
			bool result = ResetEvent(handle) == TRUE;
			return(result);
		}

	} // threading

	//
	// Win32 Memory
	//
	namespace memory {
		fpl_api void *MemoryAllocate(const size_t size) {
			if (size == 0) {
				common::PushError("Size parameter must be greater than zero");
				return nullptr;
			}
			void *result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (result == nullptr) {
				common::PushError("Failed allocating memory of %xu bytes", size);
			}
			return(result);
		}

		fpl_api void MemoryFree(void *ptr) {
			if (ptr == nullptr) {
				common::PushError("Pointer parameter are not allowed to be null");
			}
			VirtualFree(ptr, 0, MEM_FREE);
		}

		fpl_api void *MemoryStackAllocate(const size_t size) {
			if (size == 0) {
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
		fpl_api FileHandle OpenBinaryFile(const char *filePath) {
			FileHandle result = {};
			if (filePath != nullptr) {
				HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32Handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}
		fpl_api FileHandle OpenBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if (filePath != nullptr) {
				HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32Handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}

		fpl_api FileHandle CreateBinaryFile(const char *filePath) {
			FileHandle result = {};
			if (filePath != nullptr) {
				HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32Handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}
		fpl_api FileHandle CreateBinaryFile(const wchar_t *filePath) {
			FileHandle result = {};
			if (filePath != nullptr) {
				HANDLE win32FileHandle = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (win32FileHandle != INVALID_HANDLE_VALUE) {
					result.isValid = true;
					result.internalHandle.win32Handle = (void *)win32FileHandle;
				}
			}
			return(result);
		}

		fpl_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
			if (sizeToRead == 0) {
				return 0;
			}
			if (targetBuffer == nullptr) {
				common::PushError("Target buffer parameter are now allowed to be null");
				return 0;
			}
			if (fileHandle.internalHandle.win32Handle == nullptr) {
				common::PushError("File handle is not opened for reading");
				return 0;
			}
			uint32_t result = 0;
			HANDLE win32FileHandle = (HANDLE)fileHandle.internalHandle.win32Handle;
			DWORD bytesRead = 0;
			if (ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, nullptr) == TRUE) {
				result = bytesRead;
			}
			return(result);
		}

		fpl_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
			if (sourceSize == 0) {
				common::PushError("Source size parameter must be greater than zero");
				return 0;
			}
			if (sourceBuffer == nullptr) {
				common::PushError("Source buffer parameter are now allowed to be null");
				return 0;
			}
			if (fileHandle.internalHandle.win32Handle == nullptr) {
				common::PushError("File handle is not opened for writing");
				return 0;
			}
			uint32_t result = 0;
			HANDLE win32FileHandle = (HANDLE)fileHandle.internalHandle.win32Handle;
			DWORD bytesWritten = 0;
			if (WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, nullptr) == TRUE) {
				result = bytesWritten;
			}
			return(result);
		}

		fpl_api void SetFilePosition32(const FileHandle &fileHandle, const uint32_t position, const FilePositionMode mode) {
			if (fileHandle.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32Handle;
				DWORD moveMethod = FILE_BEGIN;
				if (mode == FilePositionMode::Current) {
					moveMethod = FILE_CURRENT;
				} else if (mode == FilePositionMode::End) {
					moveMethod = FILE_END;
				}
				SetFilePointer(win32FileHandle, (LONG)position, nullptr, moveMethod);
			}
		}

		fpl_api uint32_t GetFilePosition32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if (fileHandle.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32Handle;
				DWORD filePosition = SetFilePointer(win32FileHandle, 0L, nullptr, FILE_CURRENT);
				if (filePosition != INVALID_SET_FILE_POINTER) {
					result = filePosition;
				}
			}
			return(result);
		}

		fpl_api void CloseFile(FileHandle &fileHandle) {
			if (fileHandle.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32Handle;
				CloseHandle(win32FileHandle);
				fileHandle = {};
			}
		}

		fpl_api uint32_t GetFileSize32(const char *filePath) {
			uint32_t result = 0;
			if (filePath != nullptr) {
				HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (win32FileHandle != INVALID_HANDLE_VALUE) {
					DWORD fileSize = GetFileSize(win32FileHandle, nullptr);
					result = fileSize;
					CloseHandle(win32FileHandle);
				}
			}
			return(result);
		}

		fpl_api uint32_t GetFileSize32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if (fileHandle.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle.win32Handle;
				DWORD fileSize = GetFileSize(win32FileHandle, nullptr);
				result = fileSize;
			}
			return(result);
		}

		fpl_api bool FileExists(const char *filePath) {
			bool result = false;
			if (filePath != nullptr) {
				WIN32_FIND_DATAA findData;
				HANDLE searchHandle = FindFirstFileA(filePath, &findData);
				if (searchHandle != INVALID_HANDLE_VALUE) {
					result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
					FindClose(searchHandle);
				}
			}
			return(result);
		}

		fpl_api bool FileCopy(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
			if (sourceFilePath == nullptr) {
				common::PushError("Source file path parameter are not allowed to be null");
				return false;
			}
			if (targetFilePath == nullptr) {
				common::PushError("Target file path parameter are not allowed to be null");
				return false;
			}
			bool result = (CopyFileA(sourceFilePath, targetFilePath, !overwrite) == TRUE);
			return(result);
		}

		fpl_api bool FileDelete(const char *filePath) {
			if (filePath == nullptr) {
				common::PushError("File path parameter are not allowed to be null");
				return false;
			}
			bool result = (DeleteFileA(filePath) == TRUE);
			return(result);
		}

		fpl_api bool DirectoryExists(const char *path) {
			bool result = false;
			if (path != nullptr) {
				WIN32_FIND_DATAA findData;
				HANDLE searchHandle = FindFirstFileA(path, &findData);
				if (searchHandle != INVALID_HANDLE_VALUE) {
					result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
					FindClose(searchHandle);
				}
			}
			return(result);
		}

		fpl_api bool CreateDirectories(const char *path) {
			if (path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = CreateDirectoryA(path, nullptr) > 0;
			return(result);
		}
		fpl_api bool RemoveEmptyDirectory(const char *path) {
			if (path == nullptr) {
				common::PushError("Path parameter are not allowed to be null");
				return false;
			}
			bool result = RemoveDirectoryA(path) > 0;
			return(result);
		}
		fpl_internal_inline void Win32FillFileEntry(const WIN32_FIND_DATAA &findData, FileEntry &entry) {
			strings::CopyAnsiString(findData.cFileName, strings::GetAnsiStringLength(findData.cFileName), entry.path, FPL_ARRAYCOUNT(entry.path));

			entry.type = FileEntryType::Unknown;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				entry.type = FileEntryType::Directory;
			} else if (
				(findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
				entry.type = FileEntryType::File;
			}

			entry.attributes = FileAttributeFlags::None;
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
				entry.attributes = FileAttributeFlags::Normal;
			} else {
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
					entry.attributes |= FileAttributeFlags::Hidden;
				}
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					entry.attributes |= FileAttributeFlags::ReadOnly;
				}
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
					entry.attributes |= FileAttributeFlags::Archive;
				}
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
					entry.attributes |= FileAttributeFlags::System;
				}
			}
		}
		fpl_api bool ListFilesBegin(const char *pathAndFilter, FileEntry &firstEntry) {
			if (pathAndFilter == nullptr) {
				return false;
			}
			bool result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FindFirstFileA(pathAndFilter, &findData);
			if (searchHandle != INVALID_HANDLE_VALUE) {
				firstEntry = {};
				firstEntry.internalHandle.win32Handle = searchHandle;
				Win32FillFileEntry(findData, firstEntry);
				result = true;
			}
			return(result);
		}
		fpl_api bool ListFilesNext(FileEntry &nextEntry) {
			bool result = false;
			if (nextEntry.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = nextEntry.internalHandle.win32Handle;
				WIN32_FIND_DATAA findData;
				if (FindNextFileA(searchHandle, &findData)) {
					Win32FillFileEntry(findData, nextEntry);
					result = true;
				}
			}
			return(result);
		}
		fpl_api void ListFilesEnd(FileEntry &lastEntry) {
			if (lastEntry.internalHandle.win32Handle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = lastEntry.internalHandle.win32Handle;
				FindClose(searchHandle);
				lastEntry = {};
			}
		}
	} // files

	//
	// Win32 Path/Directories
	//
	namespace paths {
#	if defined(UNICODE)
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			wchar_t modulePath[MAX_PATH];
			GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
			strings::WideStringToAnsiString(modulePath, strings::GetWideStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
	}
#	else
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			char modulePath[MAX_PATH];
			GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
			strings::CopyAnsiString(modulePath, strings::GetAnsiStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
		}
#	endif // UNICODE

#	if defined(UNICODE)
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			const platform::Win32APIFunctions &api = platform::global__Win32__API__Functions;
			wchar_t homePath[MAX_PATH];
			api.shell.shGetFolderPathW(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			strings::WideStringToAnsiString(homePath, strings::GetWideStringLength(homePath), destPath, maxDestLen);
			return(destPath);
}
#	else
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			if (destPath == nullptr) {
				common::PushError("Dest path parameter are not allowed to be null");
				return nullptr;
			}
			if (maxDestLen < (MAX_PATH + 1)) {
				common::PushError("Max dest len parameter '%d' must be greater or equal than '%d'", maxDestLen, MAX_PATH + 1);
				return nullptr;
			}
			const platform::Win32APIFunctions &api = platform::global__Win32__API__Functions;
			char homePath[MAX_PATH];
			api.shell.shGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			strings::CopyAnsiString(homePath, strings::GetAnsiStringLength(homePath), destPath, maxDestLen);
			return(destPath);
		}
#	endif // UNICODE
	} // paths

	//
	// Win32 Timings
	//
	namespace timings {
		fpl_api double GetHighResolutionTimeInSeconds() {
			LARGE_INTEGER time;
			QueryPerformanceCounter(&time);
			double result = time.QuadPart / (double)platform::global__Win32__AppState.performanceFrequency.QuadPart;
			return(result);
		}
	} // timings

	//
	// Win32 Strings
	//
	namespace strings {
		fpl_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
			if (wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if (ansiDest == nullptr) {
				common::PushError("Ansi dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			if (maxAnsiDestLen < (requiredLen + 1)) {
				common::PushError("Max ansi dest len parameter '%d' must be greater or equal than ''", maxAnsiDestLen, (requiredLen + 1));
				return nullptr;
			}
			WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, nullptr, nullptr);
			ansiDest[requiredLen] = 0;
			return(ansiDest);
		}
		fpl_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
			if (wideSource == nullptr) {
				common::PushError("Wide source parameter are not allowed to be null");
				return nullptr;
			}
			if (utf8Dest == nullptr) {
				common::PushError("UTF8 dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			if (maxUtf8DestLen < (requiredLen + 1)) {
				common::PushError("Max utf8 dest len parameter '%d' must be greater or equal than ''", maxUtf8DestLen, (requiredLen + 1));
				return nullptr;
			}
			WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, nullptr, nullptr);
			utf8Dest[requiredLen] = 0;
			return(utf8Dest);
		}
		fpl_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if (ansiSource == nullptr) {
				common::PushError("Ansi source parameter are not allowed to be null");
				return nullptr;
			}
			if (wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, nullptr, 0);
			if (maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than ''", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			MultiByteToWideChar(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			if (utf8Source == nullptr) {
				common::PushError("UTF8 source parameter are not allowed to be null");
				return nullptr;
			}
			if (wideDest == nullptr) {
				common::PushError("Wide dest parameter are not allowed to be null");
				return nullptr;
			}
			uint32_t requiredLen = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, nullptr, 0);
			if (maxWideDestLen < (requiredLen + 1)) {
				common::PushError("Max wide dest len parameter '%d' must be greater or equal than ''", maxWideDestLen, (requiredLen + 1));
				return nullptr;
			}
			MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
	} // strings

	//
	// Win32 Library
	//
	namespace library {
		fpl_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
			DynamicLibraryHandle result = {};
			if (libraryFilePath != nullptr) {
				HMODULE libModule = LoadLibraryA(libraryFilePath);
				if (libModule != nullptr) {
					result.internalHandle.win32Handle = libModule;
					result.isValid = true;
				}
			}
			return(result);
		}
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
			void *result = nullptr;
			if (handle.internalHandle.win32Handle != nullptr && name != nullptr) {
				HMODULE libModule = handle.internalHandle.win32Handle;
				result = (void *)GetProcAddress(libModule, name);
			}
			return(result);
		}
		fpl_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
			if (handle.internalHandle.win32Handle != nullptr) {
				HMODULE libModule = (HMODULE)handle.internalHandle.win32Handle;
				FreeLibrary(libModule);
				handle = {};
			}
		}
	} // library

	//
	// Win32 Video
	//
#if defined(FPL_ENABLE_VIDEO)
	namespace video {
		fpl_api VideoBackBuffer *GetVideoBackBuffer() {
			VideoBackBuffer *result = nullptr;
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			platform::Win32VideoState &video = state->video;
			if (video.activeVideoDriver == VideoDriverType::Software) {
				result = &video.software.context;
			}
#		endif
			return(result);
		}

		fpl_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
			bool result = false;
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			if (state->video.activeVideoDriver == VideoDriverType::Software) {
				Win32ReleaseVideoSoftware(state);
				result = Win32InitVideoSoftware(state, width, height);
			}
#		endif
			return (result);
		}
	} // video
#endif // FPL_ENABLE_VIDEO

#if defined(FPL_ENABLE_WINDOW)
	//
	// Win32 Window
	//
	namespace window {
		fpl_api void WindowFlip() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			platform::Win32VideoState &video = state->video;
			switch (video.activeVideoDriver) {
#			if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				case VideoDriverType::Software:
				{
					WindowSize area = GetWindowArea();
					uint32_t targetWidth = area.width;
					uint32_t targetHeight = area.height;
					uint32_t sourceWidth = video.software.context.width;
					uint32_t sourceHeight = video.software.context.height;
					wapi.gdi.stretchDIBits(state->window.deviceContext, 0, 0, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, video.software.context.pixels, &video.software.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
				} break;
#			endif

#			if defined(FPL_ENABLE_VIDEO_OPENGL)
				case VideoDriverType::OpenGL:
				{
					wapi.gdi.swapBuffers(state->window.deviceContext);
				} break;
#			endif

				default:
					break;
			}
		}

		fpl_api WindowSize GetWindowArea() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			WindowSize result = {};
			RECT windowRect;
			if (wapi.user.getClientRect(state->window.windowHandle, &windowRect)) {
				result.width = windowRect.right - windowRect.left;
				result.height = windowRect.bottom - windowRect.top;
			}
			return(result);
		}

		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			RECT clientRect, windowRect;
			if (wapi.user.getClientRect(state->window.windowHandle, &clientRect) &&
				wapi.user.getWindowRect(state->window.windowHandle, &windowRect)) {
				int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
				int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
				int newWidth = width + borderWidth;
				int newHeight = height + borderHeight;
				wapi.user.setWindowPos(state->window.windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

		fpl_api bool IsWindowResizable() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			DWORD style = platform::win32_getWindowLong(state->window.windowHandle, GWL_STYLE);
			bool result = (style & WS_THICKFRAME) > 0;
			return(result);
		}

		fpl_api void SetWindowResizeable(const bool value) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			if (!state->currentSettings.window.isFullscreen) {
				DWORD style;
				DWORD exStyle;
				if (value) {
					style = platform::Win32ResizeableWindowStyle;
					exStyle = platform::Win32ResizeableWindowExtendedStyle;
				} else {
					style = platform::Win32NonResizableWindowStyle;
					exStyle = platform::Win32NonResizableWindowExtendedStyle;
				}
				platform::win32_setWindowLong(state->window.windowHandle, GWL_STYLE, style);
				platform::win32_setWindowLong(state->window.windowHandle, GWL_EXSTYLE, exStyle);
				state->currentSettings.window.isResizable = value;
			}
		}

		fpl_api bool IsWindowFullscreen() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			HWND windowHandle = state->window.windowHandle;
			DWORD style = platform::win32_getWindowLong(windowHandle, GWL_STYLE);
			bool result = (style & platform::Win32FullscreenWindowStyle) > 0;
			return(result);
		}

		fpl_api void SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			if (value) {
				WindowSettings &windowSettings = state->currentSettings.window;

				// Save window placement and size
				HWND windowHandle = state->window.windowHandle;
				state->window.lastWindowPlacement = {};
				wapi.user.getWindowPlacement(windowHandle, &state->window.lastWindowPlacement);

				RECT windowRect = {};
				wapi.user.getWindowRect(windowHandle, &windowRect);
				state->window.lastWindowWidth = windowRect.right - windowRect.left;
				state->window.lastWindowHeight = windowRect.bottom - windowRect.top;

				// Enter fullscreen mode or fallback to window mode
				windowSettings.isFullscreen = platform::Win32EnterFullscreen(fullscreenWidth, fullscreenHeight, refreshRate, 0);
				if (!windowSettings.isFullscreen) {
					platform::Win32LeaveFullscreen();
				}
			} else {
				platform::Win32LeaveFullscreen();
			}
		}

		fpl_api WindowPosition GetWindowPosition() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			WindowPosition result = {};
			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			if (wapi.user.getWindowPlacement(state->window.windowHandle, &placement) == TRUE) {
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

		fpl_api void SetWindowTitle(const char *title) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			HWND handle = state->window.windowHandle;

			// @FIXME(final): This is not correct to set a ansi text, in unicode we have to use the Wide version!
			wapi.user.setWindowTextA(handle, title);
		}

		fpl_api void SetWindowPosition(const int32_t left, const int32_t top) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			RECT windowRect;
			if (wapi.user.getWindowPlacement(state->window.windowHandle, &placement) &&
				wapi.user.getWindowRect(state->window.windowHandle, &windowRect)) {
				switch (placement.showCmd) {
					case SW_NORMAL:
					case SW_SHOW:
					{
						placement.rcNormalPosition.left = left;
						placement.rcNormalPosition.top = top;
						placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
						placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
						wapi.user.setWindowPlacement(state->window.windowHandle, &placement);
					} break;
				}
			}
		}

		fpl_api void SetWindowCursorEnabled(const bool value) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			state->window.isCursorActive = value;
		}

		fpl_api bool WindowUpdate() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			bool result = false;

			// Poll gamepad controller states
			platform::Win32PollControllers(state);

			// Poll window events
			if (state->window.windowHandle != 0) {
				MSG msg;
				while (platform::win32_peekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
					wapi.user.translateMessage(&msg);
					platform::win32_dispatchMessage(&msg);
				}
				result = state->window.isRunning;
			}

			return(result);
		}

		fpl_api bool IsWindowRunning() {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			bool result = state->window.isRunning;
			return(result);
		}

		fpl_api bool PollWindowEvent(Event &ev) {
			platform::EventQueue *eventQueue = platform::global__EventQueue;
			FPL_ASSERT(eventQueue != nullptr);
			bool result = false;
			if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pollIndex, 1);
				ev = eventQueue->events[eventIndex];
				result = true;
			} else if (eventQueue->pushCount > 0) {
				atomics::AtomicExchangeU32(&eventQueue->pollIndex, 0);
				atomics::AtomicExchangeU32(&eventQueue->pushCount, 0);
			}
			return result;
		}

		fpl_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			char *result = nullptr;
			if (wapi.user.openClipboard(state->window.windowHandle)) {
				if (wapi.user.isClipboardFormatAvailable(CF_TEXT)) {
					HGLOBAL dataHandle = wapi.user.getClipboardData(CF_TEXT);
					if (dataHandle != nullptr) {
						const char *stringValue = (const char *)GlobalLock(dataHandle);
						result = strings::CopyAnsiString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}

		fpl_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			wchar_t *result = nullptr;
			if (wapi.user.openClipboard(state->window.windowHandle)) {
				if (wapi.user.isClipboardFormatAvailable(CF_UNICODETEXT)) {
					HGLOBAL dataHandle = wapi.user.getClipboardData(CF_UNICODETEXT);
					if (dataHandle != nullptr) {
						const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
						result = strings::CopyWideString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				wapi.user.closeClipboard();
			}
			return(result);
		}

		fpl_api bool SetClipboardText(const char *ansiSource) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			bool result = false;
			if (wapi.user.openClipboard(state->window.windowHandle)) {
				const uint32_t ansiLen = strings::GetAnsiStringLength(ansiSource);
				const uint32_t ansiBufferLen = ansiLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
				if (handle != nullptr) {
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

		fpl_api bool SetClipboardText(const wchar_t *wideSource) {
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			platform::Win32State *state = platform::global__Win32__State;
			platform::Win32APIFunctions &wapi = platform::global__Win32__API__Functions;
			bool result = false;
			if (wapi.user.openClipboard(state->window.windowHandle)) {
				const uint32_t wideLen = strings::GetWideStringLength(wideSource);
				const uint32_t wideBufferLen = wideLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
				if (handle != nullptr) {
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

//
// Core Win32
//
	fpl_api const char *GetPlatformLastError() {
		const char *result = nullptr;
		if (common::global__LastErrorState != nullptr) {
#		if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			if (common::global__LastErrorState->count > 0) {
				size_t index = common::global__LastErrorState->count - 1;
				result = GetPlatformLastError(index);
			}
#		else
			result = global__LastErrorState->errors[0];
#		endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
			}
		return (result);
		}

	fpl_api const char *GetPlatformLastError(const size_t index) {
		const char *result = nullptr;
		if (common::global__LastErrorState != nullptr) {
#		if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			if (index > -1 && index < (int32_t)common::global__LastErrorState->count) {
				result = common::global__LastErrorState->errors[index];
			} else {
				result = common::global__LastErrorState->errors[common::global__LastErrorState->count - 1];
			}
#		else
			result = global__LastErrorState->errors[0];
#		endif // FPL_ENABLE_MULTIPLE_ERRORSTATES
			}
		return (result);
		}

	fpl_api size_t GetPlatformLastErrorCount() {
		size_t result = 0;
		if (common::global__LastErrorState != nullptr) {
#		if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			result = common::global__LastErrorState->count;
#		else
			result = strings::GetAnsiStringLength(common::global__LastErrorState->errors[0]) > 0 ? 1 : 0;
#		endif
		}
		return (result);
	}

	fpl_api const Settings &GetCurrentSettings() {
		FPL_ASSERT(platform::global__Win32__State != nullptr);
		const platform::Win32State *state = platform::global__Win32__State;
		return (state->currentSettings);
	}

	fpl_api void ReleasePlatform() {
		FPL_ASSERT(platform::global__Win32__AppState.isInitialized);
		FPL_ASSERT(platform::global__Win32__State != nullptr);
		platform::Win32State &win32State = *platform::global__Win32__State;

#	if defined(FPL_ENABLE_AUDIO)
		audio::ReleaseAudio();
#	endif

#	if defined(FPL_ENABLE_WINDOW)
		if (win32State.currentSettings.window.isFullscreen) {
			platform::Win32LeaveFullscreen();
		}

#	if defined(FPL_ENABLE_VIDEO)
		switch (win32State.video.activeVideoDriver) {
#		if defined(FPL_ENABLE_VIDEO_OPENGL)
			case VideoDriverType::OpenGL:
			{
				platform::Win32ReleaseVideoOpenGL(&win32State);
			} break;
#		endif

#		if defined(FPL_ENABLE_VIDEO_SOFTWARE)
			case VideoDriverType::Software:
			{
				platform::Win32ReleaseVideoSoftware(&win32State);
			} break;
#		endif

			default:
				break;
		}
#	endif // FPL_ENABLE_VIDEO

		platform::Win32ReleaseWindow(win32State);

		platform::Win32UnloadXInput();
#	endif // FPL_ENABLE_WINDOW

		platform::Win32UnloadAPI();

		memory::MemoryAlignedFree(common::global__LastErrorState);
		common::global__LastErrorState = nullptr;

		fpl::memory::MemoryAlignedFree(platform::global__Win32__State);
		platform::global__Win32__State = nullptr;

		platform::global__Win32__AppState.isInitialized = false;
	}

	fpl_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings) {
		if (platform::global__Win32__AppState.isInitialized) {
			common::PushError("Platform is already initialized");
			return false;
		}

		platform::global__Win32__AppState.appInstance = GetModuleHandleA(nullptr);

		// @NOTE(final): Expect kernel32.lib to be linked always, so VirtualAlloc and LoadLibrary will always work.

		// Allocate win32 state
		FPL_ASSERT(platform::global__Win32__State == nullptr);
		void *win32StateMemory = fpl::memory::MemoryAlignedAllocate(sizeof(platform::Win32State), 16);
		FPL_ASSERT(win32StateMemory != nullptr);
		platform::global__Win32__State = (platform::Win32State *)win32StateMemory;
		platform::Win32State &win32State = *platform::global__Win32__State;
		win32State.initSettings = initSettings;
		win32State.initFlags = initFlags;
		win32State.currentSettings = initSettings;
		win32State.video = {};

		// Allocate last error state
		void *lastErrorStateMemory = memory::MemoryAlignedAllocate(sizeof(common::ErrorState), 16);
		FPL_ASSERT(lastErrorStateMemory != nullptr);
		common::global__LastErrorState = (common::ErrorState *)lastErrorStateMemory;

		// Timing
		QueryPerformanceFrequency(&platform::global__Win32__AppState.performanceFrequency);

		// Get main thread infos
		HANDLE mainThreadHandle = GetCurrentThread();
		DWORD mainThreadHandleId = GetCurrentThreadId();
		threading::ThreadContext *context = &common::global__ThreadState.mainThread;
		*context = {};
		context->id = mainThreadHandleId;
		context->internalHandle.win32Handle = (void *)mainThreadHandle;
		context->currentState = threading::ThreadState::Running;

#	if defined(FPL_ENABLE_WINDOW)
		// Window is required for video always
		if (win32State.initFlags & InitFlags::Video) {
			win32State.initFlags |= InitFlags::Window;
		}
#	endif

		// Load windows api library
		if (!Win32LoadAPI(win32State)) {
			// @NOTE(final): Assume that errors are pushed on already.
			return false;
		}

#	if defined(FPL_ENABLE_WINDOW)
		// Load XInput
		platform::Win32LoadXInput();

		if (win32State.initFlags & InitFlags::Window) {
			if (!platform::Win32InitWindow(win32State, initSettings)) {
				common::PushError("Failed creating a window with flags '%d' and settings (Width=%d, Height=%d, Videoprofile=%d)", win32State.initFlags, initSettings.window.windowWidth, initSettings.window.windowHeight, initSettings.video.profile);
				return false;
			}
		}
#	endif // FPL_ENABLE_WINDOW

#	if defined(FPL_ENABLE_AUDIO)
		if (win32State.initFlags & InitFlags::Audio) {
			if (audio::InitAudio(initSettings.audio) != audio::AudioResult::Success) {
				common::PushError("Failed initialization audio with settings (Driver=%s, Format=%s, SampleRate=%d, Channels=%d, BufferSize=%d)", audio::GetAudioDriverString(initSettings.audio.driver), audio::GetAudioFormatString(initSettings.audio.desiredFormat.type), initSettings.audio.desiredFormat.sampleRate, initSettings.audio.desiredFormat.channels);
				return false;
			}
		}
#	endif

		platform::global__Win32__AppState.isInitialized = true;

		return (true);
	}

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
// POSIX Platform (Linux, Unix)
//
// ****************************************************************************
#if defined(FPL_PLATFORM_POSIX)

#include <time.h> // clock_gettime, nanosleep
#include <dlfcn.h> // dlopen, dlclose

namespace fpl {
	// POSIX Atomics
	namespace atomics {
#	if defined(FPL_COMPILER_GCC)
		// @NOTE(final): See: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html#g_t_005f_005fsync-Builtins
		fpl_api void AtomicReadFence() {
			// @TODO(final): Wrong to ensure a full memory fence here!
			__sync_synchronize();
		}
		fpl_api void AtomicWriteFence() {
			// @TODO(final): Wrong to ensure a full memory fence here!
			__sync_synchronize();
		}
		fpl_api void AtomicReadWriteFence() {
			__sync_synchronize();
		}

		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			uint32_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value) {
			int32_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			uint64_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}
		fpl_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value) {
			int64_t result = __sync_lock_test_and_set(target, value);
			__sync_synchronize();
			return(result);
		}

		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}
		fpl_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = __sync_fetch_and_add(value, addend);
			return (result);
		}

		fpl_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int32_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api uint64_t AtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			int64_t result = __sync_val_compare_and_swap(dest, comparand, exchange);
			return (result);
		}

		fpl_api bool IsAtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t comparand, const uint32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t comparand, const int32_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeU64(volatile uint64_t *dest, const uint64_t comparand, const uint64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}
		fpl_api bool IsAtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t comparand, const int64_t exchange) {
			FPL_ASSERT(dest != nullptr);
			bool result = __sync_bool_compare_and_swap(dest, comparand, exchange);
			return (result);
		}

		fpl_api uint32_t AtomicLoadU32(volatile uint32_t *source) {
			uint32_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_api uint64_t AtomicLoadU64(volatile uint64_t *source) {
			uint64_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_api int32_t AtomicLoadS32(volatile int32_t *source) {
			int32_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}
		fpl_api int64_t AtomicLoadS64(volatile int64_t *source) {
			int64_t result = __sync_fetch_and_or(source, 0);
			return(result);
		}

		fpl_api void AtomicStoreU32(volatile uint32_t *dest, const uint32_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_api void AtomicStoreU64(volatile uint64_t *dest, const uint64_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_api void AtomicStoreS32(volatile int32_t *dest, const int32_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
		fpl_api void AtomicStoreS64(volatile int64_t *dest, const int64_t value) {
			__sync_lock_test_and_set(dest, value);
			__sync_synchronize();
		}
#	else
#		error "This POSIX compiler/platform is supported!"
#	endif
	}

	// ṔOSIX Timings
	namespace timings {
		fpl_api double GetHighResolutionTimeInSeconds() {
		  // @TODO(final): Do we need to take the performance frequency into account?
			timespec t;
			clock_gettime(CLOCK_MONOTONIC, &t);
			double result = (double)t.tv_sec + ((double)t.tv_nsec * 1e-9);
			return(result);
		}
	}

	// ṔOSIX Threading
	namespace threading {
		fpl_api void ThreadSleep(const uint32_t milliseconds) {
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
	}

	// POSIX Library
	namespace library {
		fpl_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
			DynamicLibraryHandle result = {};
			if (libraryFilePath != nullptr) {
				void *p = dlopen(libraryFilePath, RTLD_NOW);
				if (p != nullptr) {
					result.internalHandle.posixHandle = p;
					result.isValid = true;
				}
			}
			return(result);
		}

		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
			void *result = nullptr;
			if (handle.internalHandle.posixHandle != nullptr && name != nullptr) {
				void *p = handle.internalHandle.posixHandle;
				result = dlsym(p, name);
			}
			return(result);
		}

		fpl_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
			if (handle.internalHandle.posixHandle != nullptr) {
				void *p = handle.internalHandle.posixHandle;
				dlclose(p);
				handle = {};
			}
		}
	}


	// ṔOSIX Console
	namespace console {
		fpl_api void ConsoleOut(const char *text) {
			if (text != nullptr) {
				fprintf(stdout, text);
			}
		}
		fpl_api void ConsoleFormatOut(const char *format, ...) {
			if (format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stdout, format, vaList);
				va_end(vaList);
			}
		}
		fpl_api void ConsoleError(const char *text) {
			if (text != nullptr) {
				fprintf(stderr, "%s", text);
			}
		}
		fpl_api void ConsoleFormatError(const char *format, ...) {
			if (format != nullptr) {
				va_list vaList;
				va_start(vaList, format);
				vfprintf(stderr, format, vaList);
				va_end(vaList);
			}
		}
	}



}

#endif // FPL_PLATFORM_POSIX

// ****************************************************************************
//
// Linux Platform
//
// ****************************************************************************
#if defined(FPL_PLATFORM_LINUX)
#	include <sys/mman.h> // mmap, munmap

namespace fpl {
	// Linux Memory
	namespace memory {
		fpl_api void *MemoryAllocate(const size_t size) {
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

		fpl_api void MemoryFree(void *ptr) {
			// Free the base pointer which is stored to the left at the start of the size_t
			void *basePtr = (void *)((uint8_t *)ptr - (sizeof(uintptr_t) + sizeof(size_t)));
			size_t storedSize = *(size_t *)basePtr;
			munmap(basePtr, storedSize);
		}
	}

	fpl_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings) {
		return true;
	}

	fpl_api void ReleasePlatform() {
	}
	}
#endif // FPL_PLATFORM_LINUX

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

		fpl_internal bool ReleaseDirectSound(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			if (dsoundState.dsoundLibrary != nullptr) {
				if (dsoundState.stopEvent != nullptr) {
					CloseHandle(dsoundState.stopEvent);
				}

				for (uint32_t i = 0; i < commonAudio.internalFormat.periods; ++i) {
					if (dsoundState.notifyEvents[i]) {
						CloseHandle(dsoundState.notifyEvents[i]);
					}
				}

				if (dsoundState.notify != nullptr) {
					dsoundState.notify->Release();
				}

				if (dsoundState.secondaryBuffer != nullptr) {
					dsoundState.secondaryBuffer->Release();
				}

				if (dsoundState.primaryBuffer != nullptr) {
					dsoundState.primaryBuffer->Release();
				}

				if (dsoundState.directSound != nullptr) {
					dsoundState.directSound->Release();
				}

				FreeLibrary(dsoundState.dsoundLibrary);
				dsoundState = {};
			}

			return true;
		}

		fpl_internal audio::AudioResult InitDirectSound(const AudioSettings &audioSettings, audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			// Load direct sound library
			dsoundState.dsoundLibrary = LoadLibraryA("dsound.dll");
			func_DirectSoundCreate *directSoundCreate = (func_DirectSoundCreate *)GetProcAddress(dsoundState.dsoundLibrary, "DirectSoundCreate");
			if (directSoundCreate == nullptr) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Load direct sound object
			if (!SUCCEEDED(directSoundCreate(nullptr, &dsoundState.directSound, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Setup wave format ex
			WAVEFORMATEXTENSIBLE wf = {};
			wf.Format.cbSize = sizeof(wf);
			wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			wf.Format.nChannels = (WORD)audioSettings.desiredFormat.channels;
			wf.Format.nSamplesPerSec = (DWORD)audioSettings.desiredFormat.sampleRate;
			wf.Format.wBitsPerSample = audio::GetAudioSampleSizeInBytes(audioSettings.desiredFormat.type) * 8;
			wf.Format.nBlockAlign = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
			wf.Format.nAvgBytesPerSec = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
			wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
			if ((audioSettings.desiredFormat.type == AudioFormatType::F32) || (audioSettings.desiredFormat.type == AudioFormatType::F64)) {
				wf.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
			} else {
				wf.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			}

			// Get either local window handle or desktop handle
			FPL_ASSERT(platform::global__Win32__State != nullptr);
			HWND windowHandle = nullptr;
#if defined(FPL_ENABLE_WINDOW)
			if (platform::global__Win32__State->initFlags & InitFlags::Window) {
				windowHandle = platform::global__Win32__State->window.windowHandle;
			}
#endif
			if (windowHandle == nullptr) {
				windowHandle = platform::global__Win32__API__Functions.user.getDesktopWindow();
			}

			// The cooperative level must be set before doing anything else
			if (FAILED(dsoundState.directSound->SetCooperativeLevel(windowHandle, (audioSettings.preferExclusiveMode) ? DSSCL_EXCLUSIVE : DSSCL_PRIORITY))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Create primary buffer
			DSBUFFERDESC descDSPrimary = {};
			descDSPrimary.dwSize = sizeof(DSBUFFERDESC);
			descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
			if (FAILED(dsoundState.directSound->CreateSoundBuffer(&descDSPrimary, &dsoundState.primaryBuffer, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Set format
			if (FAILED(dsoundState.primaryBuffer->SetFormat((WAVEFORMATEX*)&wf))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Get the required size in bytes
			DWORD requiredSize;
			if (FAILED(dsoundState.primaryBuffer->GetFormat(nullptr, 0, &requiredSize))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Get actual format
			char rawdata[1024];
			WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
			if (FAILED(dsoundState.primaryBuffer->GetFormat((WAVEFORMATEX*)pActualFormat, requiredSize, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Set internal format
			AudioDeviceFormat internalFormat = {};
			if (IsEqualGUID(pActualFormat->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
				if (pActualFormat->Format.wBitsPerSample == 64) {
					internalFormat.type = AudioFormatType::F64;
				} else {
					internalFormat.type = AudioFormatType::F32;
				}
			} else {
				switch (pActualFormat->Format.wBitsPerSample) {
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
			if (FAILED(dsoundState.directSound->CreateSoundBuffer(&descDS, &dsoundState.secondaryBuffer, nullptr))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
			if (FAILED(dsoundState.secondaryBuffer->QueryInterface(FPL_IID_IDirectSoundNotify, (void**)&dsoundState.notify))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Setup notifications
			uint32_t periodSizeInBytes = internalFormat.bufferSizeInBytes / internalFormat.periods;
			DSBPOSITIONNOTIFY notifyPoints[FPL_DIRECTSOUND_MAX_PERIODS];
			for (uint32_t i = 0; i < internalFormat.periods; ++i) {
				dsoundState.notifyEvents[i] = CreateEventA(nullptr, false, false, nullptr);
				if (dsoundState.notifyEvents[i] == nullptr) {
					ReleaseDirectSound(commonAudio, dsoundState);
					return audio::AudioResult::Failed;
				}

				// The notification offset is in bytes.
				notifyPoints[i].dwOffset = i * periodSizeInBytes;
				notifyPoints[i].hEventNotify = dsoundState.notifyEvents[i];
			}
			if (FAILED(dsoundState.notify->SetNotificationPositions(internalFormat.periods, notifyPoints))) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			// Create stop event
			dsoundState.stopEvent = CreateEventA(nullptr, false, false, nullptr);
			if (dsoundState.stopEvent == nullptr) {
				ReleaseDirectSound(commonAudio, dsoundState);
				return audio::AudioResult::Failed;
			}

			return audio::AudioResult::Success;
		}

		fpl_internal_inline void StopMainLoopDirectSound(DirectSoundState &dsoundState) {
			dsoundState.breakMainLoop = true;
			SetEvent(dsoundState.stopEvent);
		}

		fpl_internal bool GetCurrentFrameDirectSound(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState, uint32_t* pCurrentPos) {
			FPL_ASSERT(pCurrentPos != nullptr);
			*pCurrentPos = 0;

			FPL_ASSERT(dsoundState.secondaryBuffer != nullptr);
			DWORD dwCurrentPosition;
			if (FAILED(dsoundState.secondaryBuffer->GetCurrentPosition(nullptr, &dwCurrentPosition))) {
				return false;
			}

			FPL_ASSERT(commonAudio.internalFormat.channels > 0);
			*pCurrentPos = (uint32_t)dwCurrentPosition / audio::GetAudioSampleSizeInBytes(commonAudio.internalFormat.type) / commonAudio.internalFormat.channels;
			return true;
		}

		fpl_internal uint32_t GetAvailableFramesDirectSound(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			// Get current frame from current play position
			uint32_t currentFrame;
			if (!GetCurrentFrameDirectSound(commonAudio, dsoundState, &currentFrame)) {
				return 0;
			}

			// In a playback device the last processed frame should always be ahead of the current frame. The space between
			// the last processed and current frame (moving forward, starting from the last processed frame) is the amount
			// of space available to write.
			uint32_t totalFrameCount = commonAudio.internalFormat.bufferSizeInFrames;
			uint32_t committedBeg = currentFrame;
			uint32_t committedEnd;
			committedEnd = dsoundState.lastProcessedFrame;
			if (committedEnd <= committedBeg) {
				committedEnd += totalFrameCount;
			}

			uint32_t committedSize = (committedEnd - committedBeg);
			FPL_ASSERT(committedSize <= totalFrameCount);

			return totalFrameCount - committedSize;
		}

		fpl_internal uint32_t WaitForFramesDirectSound(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(commonAudio.internalFormat.sampleRate > 0);
			FPL_ASSERT(commonAudio.internalFormat.periods > 0);

			// The timeout to use for putting the thread to sleep is based on the size of the buffer and the period count.
			DWORD timeoutInMilliseconds = (commonAudio.internalFormat.bufferSizeInFrames / (commonAudio.internalFormat.sampleRate / 1000)) / commonAudio.internalFormat.periods;
			if (timeoutInMilliseconds < 1) {
				timeoutInMilliseconds = 1;
			}

			// Copy event handles so we can wait for each one
			unsigned int eventCount = commonAudio.internalFormat.periods + 1;
			HANDLE pEvents[FPL_DIRECTSOUND_MAX_PERIODS + 1]; // +1 for the stop event.
			memory::MemoryCopy(dsoundState.notifyEvents, sizeof(HANDLE) * commonAudio.internalFormat.periods, pEvents);
			pEvents[eventCount - 1] = dsoundState.stopEvent;

			while (!dsoundState.breakMainLoop) {
				// Get available frames from directsound
				uint32_t framesAvailable = GetAvailableFramesDirectSound(commonAudio, dsoundState);
				if (framesAvailable > 0) {
					return framesAvailable;
				}

				// If we get here it means we weren't able to find any frames. We'll just wait here for a bit.
				WaitForMultipleObjects(eventCount, pEvents, FALSE, timeoutInMilliseconds);
			}

			// We'll get here if the loop was terminated. Just return whatever's available.
			return GetAvailableFramesDirectSound(commonAudio, dsoundState);
		}

		fpl_internal bool StopDirectSound(DirectSoundState &dsoundState) {
			FPL_ASSERT(dsoundState.secondaryBuffer != nullptr);
			if (FAILED(dsoundState.secondaryBuffer->Stop())) {
				return false;
			}
			dsoundState.secondaryBuffer->SetCurrentPosition(0);
			return true;
		}

		fpl_internal audio::AudioResult StartDirectSound(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
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
			if (SUCCEEDED(dsoundState.secondaryBuffer->Lock(0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
				framesToRead = actualLockSize / audioSampleSizeBytes / commonAudio.internalFormat.channels;
				audio::ReadAudioFramesFromClient(commonAudio, framesToRead, pLockPtr);
				dsoundState.secondaryBuffer->Unlock(pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
				dsoundState.lastProcessedFrame = framesToRead;
				if (FAILED(dsoundState.secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
					return audio::AudioResult::Failed;
				}
			} else {
				return audio::AudioResult::Failed;
			}
			return audio::AudioResult::Success;
		}

		fpl_internal void DirectSoundMainLoop(const audio::CommonAudioState &commonAudio, DirectSoundState &dsoundState) {
			FPL_ASSERT(commonAudio.internalFormat.channels > 0);
			uint32_t audioSampleSizeBytes = audio::GetAudioSampleSizeInBytes(commonAudio.internalFormat.type);
			FPL_ASSERT(audioSampleSizeBytes > 0);

			// Make sure the stop event is not signaled to ensure we don't end up immediately returning from WaitForMultipleObjects().
			ResetEvent(dsoundState.stopEvent);

			// Main loop
			dsoundState.breakMainLoop = false;
			while (!dsoundState.breakMainLoop) {
				// Wait until we get available frames from directsound
				uint32_t framesAvailable = WaitForFramesDirectSound(commonAudio, dsoundState);
				if (framesAvailable == 0) {
					continue;
				}

				// Don't bother grabbing more data if the device is being stopped.
				if (dsoundState.breakMainLoop) {
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
					if (FAILED(dsoundState.secondaryBuffer->Lock(lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
						// @TODO(final): Handle error
						break;
					}

					// Read actual frames from user
					uint32_t frameCount = actualLockSize / audioSampleSizeBytes / commonAudio.internalFormat.channels;
					audio::ReadAudioFramesFromClient(commonAudio, frameCount, pLockPtr);
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

//
// Audio
//
#if defined(FPL_ENABLE_AUDIO)
	namespace audio {
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
			volatile AudioResult workResult;

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

		fpl_globalvar AudioState global__Audio__State = {};

		fpl_internal void AudioDeviceStopMainLoop(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			switch (audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					StopMainLoopDirectSound(audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
		}

		fpl_internal bool AudioReleaseDevice(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			bool result = false;
			switch (audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = ReleaseDirectSound(audioState.common, audioState.dsound);
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
			switch (audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = StopDirectSound(audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
			return (result);
		}

		fpl_internal AudioResult AudioStartDevice(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			AudioResult result = AudioResult::Failed;
			switch (audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					result = StartDirectSound(audioState.common, audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
			return (result);
		}

		fpl_internal void AudioDeviceMainLoop(AudioState &audioState) {
			FPL_ASSERT(audioState.activeDriver > AudioDriverType::Auto);
			switch (audioState.activeDriver) {

#			if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
				case AudioDriverType::DirectSound:
				{
					DirectSoundMainLoop(audioState.common, audioState.dsound);
				} break;
#			endif

				default:
					break;
			}
		}

		fpl_internal_inline bool IsAudioDriverAsync(AudioDriverType audioDriver) {
			switch (audioDriver) {
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
			if (audioState == nullptr) {
				return false;
			}
			AudioDeviceState state = AudioGetDeviceState(*audioState);
			return(state != AudioDeviceState::Uninitialized);
		}

		fpl_internal_inline bool IsAudioDeviceStarted(AudioState *audioState) {
			if (audioState == nullptr) {
				return false;
			}
			AudioDeviceState state = AudioGetDeviceState(*audioState);
			return(state == AudioDeviceState::Started);
		}

		fpl_internal void AudioWorkerThread(const threading::ThreadContext &context, void *data) {
			AudioState *audioState = (AudioState *)data;
			FPL_ASSERT(audioState != nullptr);
			FPL_ASSERT(audioState->activeDriver != AudioDriverType::None);

#		if defined(FPL_PLATFORM_WIN32)
			platform::global__Win32__API__Functions.ole.coInitializeEx(nullptr, 0);
#		endif

			// This is only used to prevent posting onStop() when the device is first initialized.
			bool skipNextStopEvent = true;

			for (;;) {
				// At the start of iteration the device is stopped - we must explicitly mark it as such.
				AudioStopDevice(*audioState);

				if (!skipNextStopEvent) {
					// @TODO(final): Call stop callback to notify client
				} else {
					skipNextStopEvent = false;
				}


				// Let the other threads know that the device has stopped.
				AudioSetDeviceState(*audioState, AudioDeviceState::Stopped);
				threading::SignalWakeUp(audioState->stopSignal);

				// We use an event to wait for a request to wake up.
				threading::SignalWaitForOne(audioState->wakeupSignal);

				// Default result code.
				audioState->workResult = AudioResult::Success;

				// Just break if we're terminating.
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Uninitialized) {
					break;
				}

				// Getting here means we just started the device and we need to wait for the device to request more data.
				FPL_ASSERT(AudioGetDeviceState(*audioState) == AudioDeviceState::Starting);

				audioState->workResult = AudioStartDevice(*audioState);
				if (audioState->workResult != AudioResult::Success) {
					threading::SignalWakeUp(audioState->startSignal);
					continue;
				}

				// The thread that requested the device to start playing is waiting for this thread to start the device for real, which is now.
				AudioSetDeviceState(*audioState, AudioDeviceState::Started);
				threading::SignalWakeUp(audioState->startSignal);

				// Now we just enter the main loop.
				AudioDeviceMainLoop(*audioState);
			}

			// Make sure we aren't continuously waiting on a stop event.
			threading::SignalWakeUp(audioState->stopSignal);

#		if defined(FPL_PLATFORM_WIN32)
			platform::global__Win32__API__Functions.ole.coUninitialize();
#		endif
		}

		fpl_api AudioResult StopAudio() {
			AudioState *audioState = &global__Audio__State;

			if (AudioGetDeviceState(*audioState) == AudioDeviceState::Uninitialized) {
				return AudioResult::DeviceNotInitialized;
			}

			AudioResult result = AudioResult::Failed;
			MutexLock(audioState->lock);
			{
				// Check if the device is already stopped
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Stopping) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStopped;
				}
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStopped;
				}

				// The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
				if (AudioGetDeviceState(*audioState) != AudioDeviceState::Started) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceBusy;
				}

				AudioSetDeviceState(*audioState, AudioDeviceState::Stopping);

				if (audioState->isAsyncDriver) {
					// Asynchronous drivers (Has their own thread)
					AudioStopDevice(*audioState);
				} else {
					// Synchronous drivers

					// When we get here the worker thread is likely in a wait state while waiting for the backend device to request audio data. We need to force these to return as quickly as possible.
					AudioDeviceStopMainLoop(*audioState);

					// We need to wait for the worker thread to become available for work before returning.
					// Note that the worker thread will be the one who puts the device into the stopped state.
					SignalWaitForOne(audioState->stopSignal);
					result = AudioResult::Success;
				}
			}
			MutexUnlock(audioState->lock);

			return result;
		}

		fpl_api AudioResult PlayAudio() {
			AudioState *audioState = &global__Audio__State;

			if (!IsAudioDeviceInitialized(audioState)) {
				return AudioResult::DeviceNotInitialized;
			}

			AudioResult result = AudioResult::Failed;
			MutexLock(audioState->lock);
			{
				// Be a bit more descriptive if the device is already started or is already in the process of starting.
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Starting) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStarted;
				}
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Started) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceAlreadyStarted;
				}

				// The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
				if (AudioGetDeviceState(*audioState) != AudioDeviceState::Stopped) {
					MutexUnlock(audioState->lock);
					return AudioResult::DeviceBusy;
				}

				AudioSetDeviceState(*audioState, AudioDeviceState::Starting);

				if (audioState->isAsyncDriver) {
					// Asynchronous drivers (Has their own thread)
					AudioStartDevice(*audioState);
					AudioSetDeviceState(*audioState, AudioDeviceState::Started);
				} else {
					// Synchronous drivers
					SignalWakeUp(audioState->wakeupSignal);

					// Wait for the worker thread to finish starting the device.
					// Note that the worker thread will be the one who puts the device into the started state.
					SignalWaitForOne(audioState->startSignal);
					result = audioState->workResult;
				}
			}
			MutexUnlock(audioState->lock);

			return result;
		}

		fpl_internal void ReleaseAudio() {
			AudioState *audioState = &global__Audio__State;

			if (!IsAudioDeviceInitialized(audioState)) {
				return;
			}

			// Make sure the device is stopped first. The backends will probably handle this naturally,
			// but I like to do it explicitly for my own sanity.
			if (IsAudioDeviceStarted(audioState)) {
				while (StopAudio() == AudioResult::DeviceBusy) {
					threading::ThreadSleep(1);
				}
			}

			// Putting the device into an uninitialized state will make the worker thread return.
			AudioSetDeviceState(*audioState, AudioDeviceState::Uninitialized);

			// Wake up the worker thread and wait for it to properly terminate.
			SignalWakeUp(audioState->wakeupSignal);
			ThreadWaitForOne(audioState->workerThread);
			ThreadDestroy(audioState->workerThread);

			// Release signals and thread
			threading::SignalDestroy(audioState->stopSignal);
			threading::SignalDestroy(audioState->startSignal);
			threading::SignalDestroy(audioState->wakeupSignal);
			threading::MutexDestroy(audioState->lock);

			AudioReleaseDevice(*audioState);

			*audioState = {};

#		if defined(FPL_PLATFORM_WIN32)
			platform::global__Win32__API__Functions.ole.coUninitialize();
#		endif
		}

		fpl_internal AudioResult InitAudio(const AudioSettings &audioSettings) {
			AudioState *audioState = &global__Audio__State;

			if (audioState->activeDriver != AudioDriverType::None) {
				return AudioResult::Failed;
			}

			if (audioSettings.desiredFormat.channels == 0) {
				return AudioResult::Failed;
			}
			if (audioSettings.desiredFormat.sampleRate == 0) {
				return AudioResult::Failed;
			}
			if (audioSettings.bufferSizeInMilliSeconds == 0) {
				return AudioResult::Failed;
			}

			*audioState = {};
			audioState->common.clientReadCallback = audioSettings.clientReadCallback;
			audioState->common.clientUserData = audioSettings.userData;

#		if defined(FPL_PLATFORM_WIN32)
			platform::global__Win32__API__Functions.ole.coInitializeEx(nullptr, 0);
#		endif

			// Create mutex and signals
			audioState->lock = threading::MutexCreate();
			if (!audioState->lock.isValid) {
				ReleaseAudio();
				return AudioResult::Failed;
			}
			audioState->wakeupSignal = threading::SignalCreate();
			if (!audioState->wakeupSignal.isValid) {
				ReleaseAudio();
				return AudioResult::Failed;
			}
			audioState->startSignal = threading::SignalCreate();
			if (!audioState->startSignal.isValid) {
				ReleaseAudio();
				return AudioResult::Failed;
			}
			audioState->stopSignal = threading::SignalCreate();
			if (!audioState->stopSignal.isValid) {
				ReleaseAudio();
				return AudioResult::Failed;
			}

			// Prope drivers
			AudioDriverType propeDrivers[] = {
				AudioDriverType::DirectSound,
			};
			uint32_t driverCount = FPL_ARRAYCOUNT(propeDrivers);
			AudioResult initResult = AudioResult::Failed;
			for (uint32_t driverIndex = 0; driverIndex < driverCount; ++driverIndex) {
				AudioDriverType propeDriver = propeDrivers[driverIndex];

				initResult = AudioResult::Failed;
				switch (audioSettings.driver) {

#				if defined(FPL_ENABLE_AUDIO_DIRECTSOUND)
					case AudioDriverType::DirectSound:
					{
						initResult = InitDirectSound(audioSettings, audioState->common, audioState->dsound);
						if (initResult != AudioResult::Success) {
							ReleaseDirectSound(audioState->common, audioState->dsound);
						}
					} break;
#				endif

					default:
						break;
				}
				if (initResult == AudioResult::Success) {
					audioState->activeDriver = propeDriver;
					audioState->isAsyncDriver = IsAudioDriverAsync(propeDriver);
					break;
				}
			}

			if (initResult != AudioResult::Success) {
				ReleaseAudio();
				return initResult;
			}

			if (!audioState->isAsyncDriver) {
				// Create worker thread
				audioState->workerThread = threading::ThreadCreate(AudioWorkerThread, audioState, true);
				if (audioState->workerThread == nullptr) {
					ReleaseAudio();
					return AudioResult::Failed;
				}
				// Wait for the worker thread to put the device into it's stopped state for real.
				SignalWaitForOne(audioState->stopSignal);
			} else {
				AudioSetDeviceState(*audioState, AudioDeviceState::Stopped);
			}

			FPL_ASSERT(AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped);

			return(AudioResult::Success);
		}

		fpl_api const AudioDeviceFormat &GetAudioHardwareFormat() {
			AudioState *audioState = &global__Audio__State;
			return audioState->common.internalFormat;
		}

		fpl_api void SetAudioClientReadCallback(AudioClientReadFunction *newCallback, void *userData) {
			AudioState *audioState = &global__Audio__State;
			if (audioState->activeDriver > AudioDriverType::Auto) {
				if (AudioGetDeviceState(*audioState) == AudioDeviceState::Stopped) {
					audioState->common.clientReadCallback = newCallback;
					audioState->common.clientUserData = userData;
				}
			}
		}
	} // audio
#endif // FPL_ENABLE_AUDIO

} // fpl

#endif // FPL_IMPLEMENTATION && !FPL_IMPLEMENTED

