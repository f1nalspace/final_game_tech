/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Multi Window

Description:
	Shows the additional windows of FPL: the main window comes from fplPlatformInit(), every other window from fplWindowCreate().
	Each event tells the window it belongs to. Closing an additional window destroys it, closing the main window ends the demo.
	Hotkeys (in every window):
	  N          Create an additional window
	  T          Change the title of the window
	  Escape     Close the window, the main window ends the demo
	Parameters:
	  --windows=<n>  Create this many additional windows at the start
	  --frames=<n>   Quit by itself after this many frames, for tests
	  --quiet        Do not write the mouse move events

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2026-11-20
	- Initial version, the windows have no video yet

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#define MAX_WINDOW_COUNT 16
#define MAX_TITLE_LENGTH 128

static const uint32_t frameSleepMilliseconds = 10;
static const uint32_t mainWindowWidth = 640;
static const uint32_t mainWindowHeight = 400;
static const uint32_t additionalWindowWidth = 400;
static const uint32_t additionalWindowHeight = 260;
static const int32_t firstWindowOffset = 80;
static const int32_t windowOffsetStep = 40;
static const uint8_t colorOpaque = 255;

typedef struct DemoWindow {
	fplWindowHandle *handle;
	int number;
	bool isTitleChanged;
} DemoWindow;

typedef struct DemoOptions {
	int startWindowCount;
	int frameLimit;
	bool isQuiet;
} DemoOptions;

typedef struct DemoState {
	DemoWindow windows[MAX_WINDOW_COUNT];
	DemoOptions options;
	int windowCount;
	int nextNumber;
} DemoState;

static fplColor32 GetWindowColor(const int number) {
	static const uint8_t colorTable[][3] = {
		{ 40, 40, 48 },
		{ 160, 40, 40 },
		{ 40, 140, 60 },
		{ 40, 70, 170 },
		{ 170, 140, 30 },
		{ 120, 50, 150 },
		{ 30, 140, 150 },
	};
	const int colorCount = (int)fplArrayCount(colorTable);
	int index = number % colorCount;
	fplColor32 result = fplCreateColorRGBA(colorTable[index][0], colorTable[index][1], colorTable[index][2], colorOpaque);
	return(result);
}

static DemoWindow *FindDemoWindow(DemoState *state, const fplWindowHandle *handle) {
	for (int windowIndex = 0; windowIndex < state->windowCount; ++windowIndex) {
		if (state->windows[windowIndex].handle == handle) {
			return(&state->windows[windowIndex]);
		}
	}
	return(fpl_null);
}

static void SetWindowName(const DemoWindow *window, char *outName, const size_t maxNameLength) {
	if (window == fpl_null) {
		fplCopyString("Unknown", outName, maxNameLength);
	} else if (window->number == 0) {
		fplCopyString("Main", outName, maxNameLength);
	} else {
		fplStringFormat(outName, maxNameLength, "Window %d", window->number);
	}
}

static void CreateAdditionalWindow(DemoState *state) {
	if (state->windowCount >= MAX_WINDOW_COUNT) {
		fplConsoleFormatOut("Not more than %d windows\n", MAX_WINDOW_COUNT);
		return;
	}
	int number = state->nextNumber;
	fplWindowSettings settings;
	fplSetDefaultWindowSettings(&settings);
	fplStringFormat(settings.title, fplArrayCount(settings.title), "FPL Demo - Multi Window %d", number);
	settings.windowSize.width = additionalWindowWidth;
	settings.windowSize.height = additionalWindowHeight;
	settings.background = GetWindowColor(number);
	fplWindowHandle *handle = fplWindowCreate(&settings);
	if (handle == fpl_null) {
		fplConsoleFormatOut("Failed creating window %d\n", number);
		return;
	}
	int32_t offset = firstWindowOffset + windowOffsetStep * number;
	fplWindowSetPositionMulti(handle, offset, offset);
	DemoWindow *window = &state->windows[state->windowCount];
	window->handle = handle;
	window->number = number;
	window->isTitleChanged = false;
	state->windowCount++;
	state->nextNumber++;
	fplConsoleFormatOut("Created window %d\n", number);
}

static void DestroyAdditionalWindow(DemoState *state, DemoWindow *window) {
	int number = window->number;
	fplWindowDestroy(window->handle);
	int windowIndex = (int)(window - state->windows);
	int lastIndex = state->windowCount - 1;
	state->windows[windowIndex] = state->windows[lastIndex];
	state->windowCount--;
	fplConsoleFormatOut("Destroyed window %d\n", number);
}

static void ToggleTitle(DemoWindow *window, const char *name) {
	char title[MAX_TITLE_LENGTH];
	window->isTitleChanged = !window->isTitleChanged;
	if (window->isTitleChanged) {
		fplStringFormat(title, fplArrayCount(title), "FPL Demo - %s - title changed", name);
	} else {
		fplStringFormat(title, fplArrayCount(title), "FPL Demo - %s", name);
	}
	fplWindowSetTitleMulti(window->handle, title);
}

static const char *GetWindowEventName(const fplWindowEventType type) {
	switch (type) {
		case fplWindowEventType_Resized: return "resized";
		case fplWindowEventType_GotFocus: return "got focus";
		case fplWindowEventType_LostFocus: return "lost focus";
		case fplWindowEventType_Minimized: return "minimized";
		case fplWindowEventType_Maximized: return "maximized";
		case fplWindowEventType_Restored: return "restored";
		case fplWindowEventType_DroppedFiles: return "dropped files";
		case fplWindowEventType_Exposed: return "exposed";
		case fplWindowEventType_PositionChanged: return "moved";
		case fplWindowEventType_Closed: return "closed";
		case fplWindowEventType_Shown: return "shown";
		case fplWindowEventType_Hidden: return "hidden";
		default: return "unknown";
	}
}

// Returns false when the demo ends
static bool HandleEvent(DemoState *state, const fplEvent *ev) {
	DemoWindow *window = FindDemoWindow(state, ev->windowHandle);
	char name[MAX_TITLE_LENGTH];
	SetWindowName(window, name, fplArrayCount(name));
	switch (ev->type) {
		case fplEventType_Window:
		{
			const char *eventName = GetWindowEventName(ev->window.type);
			if (ev->window.type == fplWindowEventType_Resized) {
				fplConsoleFormatOut("[%s] window %s %ux%u\n", name, eventName, ev->window.size.width, ev->window.size.height);
			} else if (ev->window.type == fplWindowEventType_PositionChanged) {
				fplConsoleFormatOut("[%s] window %s %d,%d\n", name, eventName, ev->window.position.left, ev->window.position.top);
			} else {
				fplConsoleFormatOut("[%s] window %s\n", name, eventName);
			}
			// An additional window stays until it is destroyed, the main window ends the loop by itself
			bool isClosedAdditionalWindow = ev->window.type == fplWindowEventType_Closed && window != fpl_null && window->number != 0;
			if (isClosedAdditionalWindow) {
				DestroyAdditionalWindow(state, window);
			}
		} break;

		case fplEventType_Keyboard:
		{
			if (ev->keyboard.type != fplKeyboardEventType_Button || ev->keyboard.buttonState != fplButtonState_Press) {
				break;
			}
			fplConsoleFormatOut("[%s] key press code=%llu scan=0x%x\n", name, (unsigned long long)ev->keyboard.keyCode, ev->keyboard.scanCode);
			if (window == fpl_null) {
				break;
			}
			if (ev->keyboard.mappedKey == fplKey_N) {
				CreateAdditionalWindow(state);
			} else if (ev->keyboard.mappedKey == fplKey_T) {
				ToggleTitle(window, name);
			} else if (ev->keyboard.mappedKey == fplKey_Escape) {
				if (window->number == 0) {
					fplWindowShutdown();
					return(false);
				}
				DestroyAdditionalWindow(state, window);
			}
		} break;

		case fplEventType_Mouse:
		{
			if (ev->mouse.type == fplMouseEventType_Button) {
				const char *stateName = ev->mouse.buttonState == fplButtonState_Press ? "press" : "release";
				fplConsoleFormatOut("[%s] mouse button %d %s at %d,%d\n", name, (int)ev->mouse.mouseButton, stateName, ev->mouse.mouseX, ev->mouse.mouseY);
			} else if (ev->mouse.type == fplMouseEventType_Enter) {
				fplConsoleFormatOut("[%s] mouse enter\n", name);
			} else if (ev->mouse.type == fplMouseEventType_Leave) {
				fplConsoleFormatOut("[%s] mouse leave\n", name);
			} else if (ev->mouse.type == fplMouseEventType_Move && !state->options.isQuiet) {
				fplConsoleFormatOut("[%s] mouse move %d,%d\n", name, ev->mouse.mouseX, ev->mouse.mouseY);
			}
		} break;

		default:
			break;
	}
	return(true);
}

static void ParseOptions(const int argCount, char **args, DemoOptions *outOptions) {
	const char *windowsPrefix = "--windows=";
	const char *framesPrefix = "--frames=";
	size_t windowsPrefixLength = fplGetStringLength(windowsPrefix);
	size_t framesPrefixLength = fplGetStringLength(framesPrefix);
	for (int argIndex = 1; argIndex < argCount; ++argIndex) {
		const char *arg = args[argIndex];
		size_t argLength = fplGetStringLength(arg);
		if (argLength > windowsPrefixLength && fplIsStringEqualLen(arg, windowsPrefixLength, windowsPrefix, windowsPrefixLength)) {
			const char *value = arg + windowsPrefixLength;
			outOptions->startWindowCount = fplStringToS32(value);
		} else if (argLength > framesPrefixLength && fplIsStringEqualLen(arg, framesPrefixLength, framesPrefix, framesPrefixLength)) {
			const char *value = arg + framesPrefixLength;
			outOptions->frameLimit = fplStringToS32(value);
		} else if (fplIsStringEqual(arg, "--quiet")) {
			outOptions->isQuiet = true;
		}
	}
}

int main(int argCount, char **args) {
	DemoState state = fplZeroInit;
	ParseOptions(argCount, args, &state.options);

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo - Multi Window", settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = mainWindowWidth;
	settings.window.windowSize.height = mainWindowHeight;
	settings.window.background = GetWindowColor(0);
	if (!fplPlatformInit(fplInitFlags_Window | fplInitFlags_Input | fplInitFlags_Console, &settings)) {
		return(-1);
	}

	DemoWindow *mainWindow = &state.windows[0];
	mainWindow->handle = fplWindowGetMain();
	mainWindow->number = 0;
	state.windowCount = 1;
	state.nextNumber = 1;
	for (int windowIndex = 0; windowIndex < state.options.startWindowCount; ++windowIndex) {
		CreateAdditionalWindow(&state);
	}
	fplConsoleFormatOut("Ready, %d windows\n", state.windowCount);

	int frameIndex = 0;
	bool isRunning = true;
	while (isRunning && fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			if (!HandleEvent(&state, &ev)) {
				isRunning = false;
			}
		}
		frameIndex++;
		if (state.options.frameLimit > 0 && frameIndex >= state.options.frameLimit) {
			break;
		}
		fplThreadSleep(frameSleepMilliseconds);
	}

	fplConsoleFormatOut("Done after %d frames, %d windows open\n", frameIndex, state.windowCount);
	fplPlatformRelease();
	return(0);
}
