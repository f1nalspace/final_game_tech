/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Input Grab

Description:
	Test bench for the keyboard grab, the mouse confinement, the cursor warping and the relative mouse mode of FPL.
	Draws a crosshair at the mouse position and writes every event as one machine readable line (--log-events),
	which the test scripts in the tests folder evaluate.

	Hotkeys (like qemu, never logged as a normal key):
	  Ctrl+Alt+Q         Quit

	Parameters:
	  --log-events         Write every event as one line to the standard output
	  --timeout=<seconds>  Quit by itself after this time, the safety net for tests that grab the input
	  --stall=<ms>         Sleep this long in every frame, simulates a main loop that pumps the events rarely
	  --window=<w>x<h>     Inner size of the window (Default: 640x400)
	  --title=<text>       Window title prefix, tells several instances apart
	  --help               Print the parameters

	Log format, one event per line:
	  t=<ms since start> <category> <name> [key=value ...]
	  e.g. "t=1520 key button state=press code=38 key=A mods=0x0" or "t=1733 mouse move x=100 y=50"

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2026-09-24
	- Initial version: event log, crosshair, focus border, timeout, stall

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_VIDEO_VULKAN
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#include <stdio.h> // fflush

// Default inner window size, small enough to keep the test windows out of the way
#define DEFAULT_WINDOW_WIDTH 640
#define DEFAULT_WINDOW_HEIGHT 400

// Largest accepted window size and timeout, anything above is a typo
#define MAX_WINDOW_EXTENT 8192
#define MAX_TIMEOUT_SECONDS 3600
#define MAX_STALL_MILLISECONDS 10000

// Sleep per frame when nothing has changed, keeps the idle CPU load low without adding noticeable input latency
static const uint32_t idleSleepMilliseconds = 2;

static const uint32_t millisecondsPerSecond = 1000;

// Crosshair: arms of this length, with a gap of this size around the center
static const int32_t crosshairArmLength = 12;
static const int32_t crosshairGapLength = 3;

// Width of the frame that shows the focus state
static const int32_t focusFrameThickness = 2;

// Colors in the backbuffer format 0xAABBGGRR
static const uint32_t backgroundColor = 0xFF3A2727;
static const uint32_t crosshairFocusedColor = 0xFFFFFFFF;
static const uint32_t crosshairUnfocusedColor = 0xFF808080;
static const uint32_t focusFrameFocusedColor = 0xFF78C850;
static const uint32_t focusFrameUnfocusedColor = 0xFF5A5A5A;

// Size of one formatted log message and of the window title
#define LOG_MESSAGE_CAPACITY 512
#define TITLE_CAPACITY 256

typedef struct DemoOptions {
	char titlePrefix[TITLE_CAPACITY];
	uint32_t windowWidth;
	uint32_t windowHeight;
	// Zero means no timeout
	uint32_t timeoutSeconds;
	uint32_t stallMilliseconds;
	bool logEvents;
} DemoOptions;

typedef struct DemoState {
	DemoOptions options;
	fplMilliseconds startTime;
	int32_t mouseX;
	int32_t mouseY;
	fplKey lastKey;
	fplButtonState lastKeyState;
	bool hasMousePosition;
	bool hasFocus;
	bool hasLastKey;
	bool isDirty;
	bool isTitleDirty;
	bool isQuitRequested;
} DemoState;

typedef enum ParseResult {
	ParseResult_Run = 0,
	ParseResult_Help,
	ParseResult_Error,
} ParseResult;

//
// Logging
//
static void LogEvent(const DemoState *state, const char *format, ...) {
	if (!state->options.logEvents) {
		return;
	}
	char message[LOG_MESSAGE_CAPACITY];
	va_list argList;
	va_start(argList, format);
	fplStringFormatArgs(message, fplArrayCount(message), format, argList);
	va_end(argList);
	fplMilliseconds now = fplMillisecondsQuery();
	unsigned long long elapsedMilliseconds = (unsigned long long)(now - state->startTime);
	fplConsoleFormatOut("t=%llu %s\n", elapsedMilliseconds, message);
	// The tests read the log while the demo is still running, a pipe or a file is fully buffered otherwise
	fflush(stdout);
}

static const char *GetButtonStateName(const fplButtonState buttonState) {
	switch (buttonState) {
		case fplButtonState_Press:
			return "press";
		case fplButtonState_Repeat:
			return "repeat";
		case fplButtonState_Release:
			return "release";
		default:
			return "unknown";
	}
}

static const char *GetMouseButtonName(const fplMouseButtonType mouseButton) {
	switch (mouseButton) {
		case fplMouseButtonType_Left:
			return "left";
		case fplMouseButtonType_Right:
			return "right";
		case fplMouseButtonType_Middle:
			return "middle";
		case fplMouseButtonType_X1:
			return "x1";
		case fplMouseButtonType_X2:
			return "x2";
		default:
			return "none";
	}
}

static const char *GetKeyNameOrUnknown(const fplKey key) {
	const char *keyName = fplKeyGetName(key);
	if (keyName == fpl_null) {
		return "Unknown";
	}
	return keyName;
}

//
// Arguments
//
static bool StartsWith(const char *text, const char *prefix) {
	size_t textLength = fplGetStringLength(text);
	size_t prefixLength = fplGetStringLength(prefix);
	if (textLength < prefixLength) {
		return false;
	}
	bool result = fplIsStringEqualLen(text, prefixLength, prefix, prefixLength);
	return result;
}

static bool TryParseBoundedValue(const char *text, const size_t textLength, const int32_t minimumValue, const int32_t maximumValue, uint32_t *outValue) {
	int32_t value = 0;
	if (textLength == 0 || !fplTryStringToS32Len(text, textLength, &value)) {
		return false;
	}
	if (value < minimumValue || value > maximumValue) {
		return false;
	}
	*outValue = (uint32_t)value;
	return true;
}

static bool TryParseWindowSize(const char *text, uint32_t *outWidth, uint32_t *outHeight) {
	const char *separator = text;
	while (*separator != 0 && *separator != 'x') {
		++separator;
	}
	if (*separator != 'x') {
		return false;
	}
	size_t widthLength = (size_t)(separator - text);
	const char *heightText = separator + 1;
	size_t heightLength = fplGetStringLength(heightText);
	if (!TryParseBoundedValue(text, widthLength, 1, MAX_WINDOW_EXTENT, outWidth)) {
		return false;
	}
	if (!TryParseBoundedValue(heightText, heightLength, 1, MAX_WINDOW_EXTENT, outHeight)) {
		return false;
	}
	return true;
}

static void PrintHelp(void) {
	fplConsoleOut("FPL_InputGrab - test bench for keyboard grab, mouse confinement, cursor warping and relative mouse mode\n");
	fplConsoleOut("  --log-events         Write every event as one line to the standard output\n");
	fplConsoleOut("  --timeout=<seconds>  Quit by itself after this time\n");
	fplConsoleOut("  --stall=<ms>         Sleep this long in every frame\n");
	fplConsoleOut("  --window=<w>x<h>     Inner size of the window\n");
	fplConsoleOut("  --title=<text>       Window title prefix\n");
	fplConsoleOut("  Hotkeys: Ctrl+Alt+Q quits\n");
}

static ParseResult ParseArguments(const int argumentCount, char **arguments, DemoOptions *outOptions) {
	fplClearStruct(outOptions);
	fplCopyString("FPL Input Grab", outOptions->titlePrefix, fplArrayCount(outOptions->titlePrefix));
	outOptions->windowWidth = DEFAULT_WINDOW_WIDTH;
	outOptions->windowHeight = DEFAULT_WINDOW_HEIGHT;

	const char *timeoutPrefix = "--timeout=";
	const char *stallPrefix = "--stall=";
	const char *windowPrefix = "--window=";
	const char *titlePrefix = "--title=";
	size_t timeoutPrefixLength = fplGetStringLength(timeoutPrefix);
	size_t stallPrefixLength = fplGetStringLength(stallPrefix);
	size_t windowPrefixLength = fplGetStringLength(windowPrefix);
	size_t titlePrefixLength = fplGetStringLength(titlePrefix);

	for (int argumentIndex = 1; argumentIndex < argumentCount; ++argumentIndex) {
		const char *argument = arguments[argumentIndex];
		if (fplIsStringEqual(argument, "--help")) {
			return ParseResult_Help;
		} else if (fplIsStringEqual(argument, "--log-events")) {
			outOptions->logEvents = true;
		} else if (StartsWith(argument, timeoutPrefix)) {
			const char *valueText = argument + timeoutPrefixLength;
			size_t valueLength = fplGetStringLength(valueText);
			if (!TryParseBoundedValue(valueText, valueLength, 1, MAX_TIMEOUT_SECONDS, &outOptions->timeoutSeconds)) {
				fplConsoleFormatError("Invalid timeout '%s', expected 1 to %d seconds\n", valueText, MAX_TIMEOUT_SECONDS);
				return ParseResult_Error;
			}
		} else if (StartsWith(argument, stallPrefix)) {
			const char *valueText = argument + stallPrefixLength;
			size_t valueLength = fplGetStringLength(valueText);
			if (!TryParseBoundedValue(valueText, valueLength, 0, MAX_STALL_MILLISECONDS, &outOptions->stallMilliseconds)) {
				fplConsoleFormatError("Invalid stall '%s', expected 0 to %d milliseconds\n", valueText, MAX_STALL_MILLISECONDS);
				return ParseResult_Error;
			}
		} else if (StartsWith(argument, windowPrefix)) {
			const char *valueText = argument + windowPrefixLength;
			if (!TryParseWindowSize(valueText, &outOptions->windowWidth, &outOptions->windowHeight)) {
				fplConsoleFormatError("Invalid window size '%s', expected <width>x<height>\n", valueText);
				return ParseResult_Error;
			}
		} else if (StartsWith(argument, titlePrefix)) {
			const char *valueText = argument + titlePrefixLength;
			fplCopyString(valueText, outOptions->titlePrefix, fplArrayCount(outOptions->titlePrefix));
		} else {
			fplConsoleFormatError("Unknown parameter '%s', see --help\n", argument);
			return ParseResult_Error;
		}
	}
	return ParseResult_Run;
}

//
// Drawing
//
static void FillRectangle(fplVideoBackBuffer *backBuffer, const int32_t left, const int32_t top, const int32_t right, const int32_t bottom, const uint32_t color) {
	int32_t bufferWidth = (int32_t)backBuffer->width;
	int32_t bufferHeight = (int32_t)backBuffer->height;
	int32_t clippedLeft = fplMax(left, 0);
	int32_t clippedTop = fplMax(top, 0);
	int32_t clippedRight = fplMin(right, bufferWidth);
	int32_t clippedBottom = fplMin(bottom, bufferHeight);
	for (int32_t y = clippedTop; y < clippedBottom; ++y) {
		uint8_t *lineStart = (uint8_t *)backBuffer->pixels + (size_t)y * backBuffer->lineWidth;
		uint32_t *line = (uint32_t *)lineStart;
		for (int32_t x = clippedLeft; x < clippedRight; ++x) {
			line[x] = color;
		}
	}
}

static void DrawCrosshair(fplVideoBackBuffer *backBuffer, const int32_t centerX, const int32_t centerY, const uint32_t color) {
	int32_t innerStart = crosshairGapLength;
	int32_t outerEnd = crosshairGapLength + crosshairArmLength;
	// Left, right, top and bottom arm, each one pixel thick
	FillRectangle(backBuffer, centerX - outerEnd, centerY, centerX - innerStart, centerY + 1, color);
	FillRectangle(backBuffer, centerX + innerStart + 1, centerY, centerX + outerEnd + 1, centerY + 1, color);
	FillRectangle(backBuffer, centerX, centerY - outerEnd, centerX + 1, centerY - innerStart, color);
	FillRectangle(backBuffer, centerX, centerY + innerStart + 1, centerX + 1, centerY + outerEnd + 1, color);
	// Center pixel, marks the exact position
	FillRectangle(backBuffer, centerX, centerY, centerX + 1, centerY + 1, color);
}

static void DrawFocusFrame(fplVideoBackBuffer *backBuffer, const uint32_t color) {
	int32_t width = (int32_t)backBuffer->width;
	int32_t height = (int32_t)backBuffer->height;
	FillRectangle(backBuffer, 0, 0, width, focusFrameThickness, color);
	FillRectangle(backBuffer, 0, height - focusFrameThickness, width, height, color);
	FillRectangle(backBuffer, 0, 0, focusFrameThickness, height, color);
	FillRectangle(backBuffer, width - focusFrameThickness, 0, width, height, color);
}

static void Render(const DemoState *state, fplVideoBackBuffer *backBuffer) {
	int32_t width = (int32_t)backBuffer->width;
	int32_t height = (int32_t)backBuffer->height;
	FillRectangle(backBuffer, 0, 0, width, height, backgroundColor);
	uint32_t frameColor = state->hasFocus ? focusFrameFocusedColor : focusFrameUnfocusedColor;
	DrawFocusFrame(backBuffer, frameColor);
	if (state->hasMousePosition) {
		uint32_t crosshairColor = state->hasFocus ? crosshairFocusedColor : crosshairUnfocusedColor;
		DrawCrosshair(backBuffer, state->mouseX, state->mouseY, crosshairColor);
	}
}

static void UpdateTitle(const DemoState *state) {
	const char *focusText = state->hasFocus ? "yes" : "no";
	const char *lastKeyText = "-";
	const char *lastKeyStateText = "";
	if (state->hasLastKey) {
		lastKeyText = GetKeyNameOrUnknown(state->lastKey);
		lastKeyStateText = GetButtonStateName(state->lastKeyState);
	}
	char title[TITLE_CAPACITY];
	fplStringFormat(title, fplArrayCount(title), "%s - Focus: %s - Last key: %s %s - Ctrl+Alt+Q quits", state->options.titlePrefix, focusText, lastKeyText, lastKeyStateText);
	fplSetWindowTitle(title);
}

//
// Events
//
static bool IsControlAndAltDown(const fplKeyboardModifierFlags modifiers) {
	const unsigned int controlMask = fplKeyboardModifierFlags_LCtrl | fplKeyboardModifierFlags_RCtrl;
	const unsigned int altMask = fplKeyboardModifierFlags_LAlt | fplKeyboardModifierFlags_RAlt;
	unsigned int modifierBits = (unsigned int)modifiers;
	bool isControlDown = (modifierBits & controlMask) != 0;
	bool isAltDown = (modifierBits & altMask) != 0;
	return isControlDown && isAltDown;
}

// Returns true when the key event was a hotkey of the demo, which is never logged as a normal key
static bool HandleHotkey(DemoState *state, const fplKeyboardEvent *keyboardEvent) {
	if (!IsControlAndAltDown(keyboardEvent->modifiers)) {
		return false;
	}
	if (keyboardEvent->mappedKey == fplKey_Q) {
		if (keyboardEvent->buttonState == fplButtonState_Press) {
			LogEvent(state, "hotkey quit");
			state->isQuitRequested = true;
		}
		return true;
	}
	return false;
}

static void HandleWindowEvent(DemoState *state, const fplWindowEvent *windowEvent) {
	switch (windowEvent->type) {
		case fplWindowEventType_GotFocus:
			state->hasFocus = true;
			state->isTitleDirty = true;
			LogEvent(state, "window gotfocus");
			break;
		case fplWindowEventType_LostFocus:
			state->hasFocus = false;
			state->isTitleDirty = true;
			LogEvent(state, "window lostfocus");
			break;
		case fplWindowEventType_Resized:
			LogEvent(state, "window resized w=%u h=%u", windowEvent->size.width, windowEvent->size.height);
			break;
		case fplWindowEventType_PositionChanged:
			LogEvent(state, "window moved x=%d y=%d", windowEvent->position.left, windowEvent->position.top);
			break;
		case fplWindowEventType_Minimized:
			LogEvent(state, "window minimized");
			break;
		case fplWindowEventType_Maximized:
			LogEvent(state, "window maximized");
			break;
		case fplWindowEventType_Restored:
			LogEvent(state, "window restored");
			break;
		case fplWindowEventType_Shown:
			LogEvent(state, "window shown");
			break;
		case fplWindowEventType_Hidden:
			LogEvent(state, "window hidden");
			break;
		case fplWindowEventType_Closed:
			LogEvent(state, "window closed");
			break;
		default:
			break;
	}
	state->isDirty = true;
}

static void HandleKeyboardEvent(DemoState *state, const fplKeyboardEvent *keyboardEvent) {
	if (keyboardEvent->type == fplKeyboardEventType_Button) {
		if (HandleHotkey(state, keyboardEvent)) {
			return;
		}
		const char *stateName = GetButtonStateName(keyboardEvent->buttonState);
		const char *keyName = GetKeyNameOrUnknown(keyboardEvent->mappedKey);
		unsigned long long keyCode = (unsigned long long)keyboardEvent->keyCode;
		unsigned int modifiers = (unsigned int)keyboardEvent->modifiers;
		LogEvent(state, "key button state=%s code=%llu key=%s mods=0x%x", stateName, keyCode, keyName, modifiers);
		state->lastKey = keyboardEvent->mappedKey;
		state->lastKeyState = keyboardEvent->buttonState;
		state->hasLastKey = true;
		state->isTitleDirty = true;
	} else if (keyboardEvent->type == fplKeyboardEventType_Input) {
		unsigned long long codePoint = (unsigned long long)keyboardEvent->keyCode;
		LogEvent(state, "key input code=%llu", codePoint);
	}
}

static void HandleMouseEvent(DemoState *state, const fplMouseEvent *mouseEvent) {
	switch (mouseEvent->type) {
		case fplMouseEventType_Move:
			LogEvent(state, "mouse move x=%d y=%d", mouseEvent->mouseX, mouseEvent->mouseY);
			break;
		case fplMouseEventType_Button:
		{
			const char *buttonName = GetMouseButtonName(mouseEvent->mouseButton);
			const char *stateName = GetButtonStateName(mouseEvent->buttonState);
			LogEvent(state, "mouse button name=%s state=%s x=%d y=%d", buttonName, stateName, mouseEvent->mouseX, mouseEvent->mouseY);
		} break;
		case fplMouseEventType_Wheel:
			LogEvent(state, "mouse wheel delta=%.2f x=%d y=%d", mouseEvent->wheelDelta, mouseEvent->mouseX, mouseEvent->mouseY);
			break;
		default:
			break;
	}
	state->mouseX = mouseEvent->mouseX;
	state->mouseY = mouseEvent->mouseY;
	state->hasMousePosition = true;
	state->isDirty = true;
}

static bool ProcessEvents(DemoState *state) {
	bool hadEvents = false;
	fplEvent ev;
	while (fplPollEvent(&ev)) {
		hadEvents = true;
		switch (ev.type) {
			case fplEventType_Window:
				HandleWindowEvent(state, &ev.window);
				break;
			case fplEventType_Keyboard:
				HandleKeyboardEvent(state, &ev.keyboard);
				break;
			case fplEventType_Mouse:
				HandleMouseEvent(state, &ev.mouse);
				break;
			default:
				break;
		}
	}
	return hadEvents;
}

static bool IsTimeoutReached(const DemoState *state) {
	if (state->options.timeoutSeconds == 0) {
		return false;
	}
	fplMilliseconds now = fplMillisecondsQuery();
	fplMilliseconds elapsedMilliseconds = now - state->startTime;
	fplMilliseconds timeoutMilliseconds = (fplMilliseconds)state->options.timeoutSeconds * millisecondsPerSecond;
	bool result = elapsedMilliseconds >= timeoutMilliseconds;
	return result;
}

int main(int argc, char **args) {
	DemoState state = fplZeroInit;
	ParseResult parseResult = ParseArguments(argc, args, &state.options);
	if (parseResult == ParseResult_Help) {
		PrintHelp();
		return 0;
	}
	if (parseResult == ParseResult_Error) {
		return 1;
	}

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString(state.options.titlePrefix, settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = state.options.windowWidth;
	settings.window.windowSize.height = state.options.windowHeight;
	settings.video.backend = fplVideoBackendType_Software;
	settings.video.isAutoSize = true;

	if (!fplPlatformInit(fplInitFlags_Video, &settings)) {
		fplConsoleError("Failed to initialize the platform\n");
		return 1;
	}

	state.startTime = fplMillisecondsQuery();
	state.isDirty = true;
	state.isTitleDirty = true;

	fplWindowSize windowSize = fplZeroInit;
	fplGetWindowSize(&windowSize);
	LogEvent(&state, "demo ready w=%u h=%u", windowSize.width, windowSize.height);

	while (fplWindowUpdate() && !state.isQuitRequested) {
		bool hadEvents = ProcessEvents(&state);
		if (state.isQuitRequested) {
			break;
		}
		if (IsTimeoutReached(&state)) {
			LogEvent(&state, "demo timeout");
			break;
		}
		if (state.isTitleDirty) {
			UpdateTitle(&state);
			state.isTitleDirty = false;
		}
		if (state.isDirty) {
			fplVideoBackBuffer *backBuffer = fplGetVideoBackBuffer();
			Render(&state, backBuffer);
			fplVideoFlip();
			state.isDirty = false;
		}
		if (state.options.stallMilliseconds > 0) {
			fplThreadSleep(state.options.stallMilliseconds);
		} else if (!hadEvents) {
			fplThreadSleep(idleSleepMilliseconds);
		}
	}

	LogEvent(&state, "demo quit");
	fplPlatformRelease();
	return 0;
}
