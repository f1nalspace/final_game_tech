/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Input Grab

Description:
	Test bench for the keyboard grab, the mouse confinement, the cursor warping and the relative mouse mode of FPL.
	Draws a crosshair at the mouse position and writes every event as one machine readable line (--log-events),
	which the test scripts in the tests folder evaluate.

	Hotkeys (like qemu, never logged as a normal key):
	  Ctrl+Alt+G         Toggle the full grab (keyboard grab + relative mouse)
	  Ctrl+Alt+M         Toggle the mouse grab
	  Ctrl+Alt+R         Toggle the relative mouse mode
	  Ctrl+Alt+K         Toggle the keyboard grab
	  Ctrl+Alt+W         Warp the cursor to the window center
	  Ctrl+Alt+Q         Quit

	Parameters:
	  --log-events         Write every event as one line to the standard output
	  --mouse-grab         Request the mouse grab at the start
	  --relative-mouse     Request the relative mouse mode at the start
	  --keyboard-grab      Request the keyboard grab at the start
	  --selftest           Warp the cursor to five positions and check the move events and the screen positions, the exit code is 1 when a check fails
	  --timeout=<seconds>  Quit by itself after this time, the safety net for tests that grab the input
	  --stall=<ms>         Sleep this long in every frame, simulates a main loop that pumps the events rarely
	  --window=<w>x<h>     Inner size of the window (Default: 640x400)
	  --title=<text>       Window title prefix, tells several instances apart
	  --help               Print the parameters

	Log format, one event per line:
	  t=<ms since start> <category> <name> [key=value ...]
	  e.g. "t=1520 key button state=press code=38 key=A mods=0x0" or "t=1733 mouse move x=100 y=50 dx=3 dy=-1"
	  The self test lines ("selftest ...") are written even without --log-events.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2026-09-24
	- Initial version: event log, crosshair, focus border, timeout, stall
	- Move deltas in the log, grab hotkeys and parameters, warp self test

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

// Self test: four corners and the center
#define SELF_TEST_TARGET_COUNT 5

// The self test starts this long after the window got the focus, so a test script can place the window first
static const fplMilliseconds selfTestStartDelayMilliseconds = 1000;
// Without a focus event the self test starts anyway after this time, the warp itself needs no focus
static const fplMilliseconds selfTestFocusFallbackMilliseconds = 2000;
// Longest wait for the move event that follows one warp
static const fplMilliseconds selfTestMoveTimeoutMilliseconds = 2000;
// Distance of the corner targets from the edges of the client area
static const int32_t selfTestEdgeMargin = 10;

typedef struct DemoOptions {
	char titlePrefix[TITLE_CAPACITY];
	uint32_t windowWidth;
	uint32_t windowHeight;
	// Zero means no timeout
	uint32_t timeoutSeconds;
	uint32_t stallMilliseconds;
	bool logEvents;
	bool requestMouseGrab;
	bool requestRelativeMouse;
	bool requestKeyboardGrab;
	bool runSelfTest;
} DemoOptions;

typedef enum SelfTestPhase {
	SelfTestPhase_WaitForFocus = 0,
	SelfTestPhase_WaitForStart,
	SelfTestPhase_Warp,
	SelfTestPhase_WaitForMove,
	SelfTestPhase_Finished,
} SelfTestPhase;

typedef struct SelfTestState {
	int32_t targetX[SELF_TEST_TARGET_COUNT];
	int32_t targetY[SELF_TEST_TARGET_COUNT];
	fplMilliseconds phaseStartTime;
	SelfTestPhase phase;
	uint32_t targetIndex;
	uint32_t failureCount;
	// Screen position of the client area origin, found with the first warp
	int32_t clientOriginX;
	int32_t clientOriginY;
	bool hasClientOrigin;
} SelfTestState;

typedef struct DemoState {
	DemoOptions options;
	SelfTestState selfTest;
	fplMilliseconds startTime;
	int32_t mouseX;
	int32_t mouseY;
	fplKey lastKey;
	fplButtonState lastKeyState;
	int exitCode;
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
static void WriteLogLine(const DemoState *state, const char *format, va_list argList) {
	char message[LOG_MESSAGE_CAPACITY];
	fplStringFormatArgs(message, fplArrayCount(message), format, argList);
	fplMilliseconds now = fplMillisecondsQuery();
	unsigned long long elapsedMilliseconds = (unsigned long long)(now - state->startTime);
	fplConsoleFormatOut("t=%llu %s\n", elapsedMilliseconds, message);
	// The tests read the log while the demo is still running, a pipe or a file is fully buffered otherwise
	fflush(stdout);
}

static void LogEvent(const DemoState *state, const char *format, ...) {
	if (!state->options.logEvents) {
		return;
	}
	va_list argList;
	va_start(argList, format);
	WriteLogLine(state, format, argList);
	va_end(argList);
}

// Self test results are written even without --log-events
static void LogSelfTest(const DemoState *state, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	WriteLogLine(state, format, argList);
	va_end(argList);
}

static const char *GetOnOffName(const bool value) {
	return value ? "on" : "off";
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
	fplConsoleOut("  --mouse-grab         Request the mouse grab at the start\n");
	fplConsoleOut("  --relative-mouse     Request the relative mouse mode at the start\n");
	fplConsoleOut("  --keyboard-grab      Request the keyboard grab at the start\n");
	fplConsoleOut("  --selftest           Check the cursor warping, the exit code is 1 when a check fails\n");
	fplConsoleOut("  --timeout=<seconds>  Quit by itself after this time\n");
	fplConsoleOut("  --stall=<ms>         Sleep this long in every frame\n");
	fplConsoleOut("  --window=<w>x<h>     Inner size of the window\n");
	fplConsoleOut("  --title=<text>       Window title prefix\n");
	fplConsoleOut("  Hotkeys: Ctrl+Alt+G full grab, Ctrl+Alt+M mouse grab, Ctrl+Alt+R relative mouse, Ctrl+Alt+K keyboard grab, Ctrl+Alt+W warp to the center, Ctrl+Alt+Q quit\n");
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
		} else if (fplIsStringEqual(argument, "--mouse-grab")) {
			outOptions->requestMouseGrab = true;
		} else if (fplIsStringEqual(argument, "--relative-mouse")) {
			outOptions->requestRelativeMouse = true;
		} else if (fplIsStringEqual(argument, "--keyboard-grab")) {
			outOptions->requestKeyboardGrab = true;
		} else if (fplIsStringEqual(argument, "--selftest")) {
			outOptions->runSelfTest = true;
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
	bool isMouseGrabbed = fplIsWindowMouseGrabbed();
	bool isRelativeMouse = fplIsWindowRelativeMouse();
	bool isKeyboardGrabbed = fplIsWindowKeyboardGrabbed();
	const char *mouseGrabText = GetOnOffName(isMouseGrabbed);
	const char *relativeMouseText = GetOnOffName(isRelativeMouse);
	const char *keyboardGrabText = GetOnOffName(isKeyboardGrabbed);
	char title[TITLE_CAPACITY];
	fplStringFormat(title, fplArrayCount(title), "%s - Focus: %s - Mouse grab: %s, Relative: %s, Keyboard grab: %s - Last key: %s %s", state->options.titlePrefix, focusText, mouseGrabText, relativeMouseText, keyboardGrabText, lastKeyText, lastKeyStateText);
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

static void RequestMouseGrab(DemoState *state, const bool enabled) {
	bool result = fplSetWindowMouseGrab(enabled);
	LogEvent(state, "grab mouse requested=%s result=%d", GetOnOffName(enabled), result ? 1 : 0);
	state->isTitleDirty = true;
}

static void RequestRelativeMouse(DemoState *state, const bool enabled) {
	bool result = fplSetWindowRelativeMouse(enabled);
	LogEvent(state, "grab relative requested=%s result=%d", GetOnOffName(enabled), result ? 1 : 0);
	state->isTitleDirty = true;
}

static void RequestKeyboardGrab(DemoState *state, const bool enabled) {
	bool result = fplSetWindowKeyboardGrab(enabled);
	LogEvent(state, "grab keyboard requested=%s result=%d", GetOnOffName(enabled), result ? 1 : 0);
	state->isTitleDirty = true;
}

static void WarpToWindowCenter(DemoState *state) {
	fplWindowSize windowSize = fplZeroInit;
	if (!fplGetWindowSize(&windowSize)) {
		return;
	}
	int32_t centerX = (int32_t)windowSize.width / 2;
	int32_t centerY = (int32_t)windowSize.height / 2;
	bool result = fplWarpWindowCursor(centerX, centerY);
	LogEvent(state, "warp x=%d y=%d result=%d", centerX, centerY, result ? 1 : 0);
}

// Returns true when the key event was a hotkey of the demo, which is never logged as a normal key
static bool HandleHotkey(DemoState *state, const fplKeyboardEvent *keyboardEvent) {
	if (!IsControlAndAltDown(keyboardEvent->modifiers)) {
		return false;
	}
	bool isPress = keyboardEvent->buttonState == fplButtonState_Press;
	switch (keyboardEvent->mappedKey) {
		case fplKey_Q:
			if (isPress) {
				LogEvent(state, "hotkey quit");
				state->isQuitRequested = true;
			}
			return true;
		case fplKey_G:
			if (isPress) {
				// Like qemu: one hotkey grabs or releases keyboard and mouse together
				bool isAnyGrabbed = fplIsWindowKeyboardGrabbed() || fplIsWindowRelativeMouse();
				bool enableGrab = !isAnyGrabbed;
				RequestKeyboardGrab(state, enableGrab);
				RequestRelativeMouse(state, enableGrab);
			}
			return true;
		case fplKey_M:
			if (isPress) {
				bool isMouseGrabbed = fplIsWindowMouseGrabbed();
				RequestMouseGrab(state, !isMouseGrabbed);
			}
			return true;
		case fplKey_R:
			if (isPress) {
				bool isRelativeMouse = fplIsWindowRelativeMouse();
				RequestRelativeMouse(state, !isRelativeMouse);
			}
			return true;
		case fplKey_K:
			if (isPress) {
				bool isKeyboardGrabbed = fplIsWindowKeyboardGrabbed();
				RequestKeyboardGrab(state, !isKeyboardGrabbed);
			}
			return true;
		case fplKey_W:
			if (isPress) {
				WarpToWindowCenter(state);
			}
			return true;
		default:
			return false;
	}
}

//
// Self test: warps the cursor to the corners and the center, the move event must arrive with the target position and no delta,
// and the screen position must lie at the same client area origin for every target
//
static void FinishSelfTest(DemoState *state) {
	SelfTestState *selfTest = &state->selfTest;
	selfTest->phase = SelfTestPhase_Finished;
	if (selfTest->failureCount == 0) {
		LogSelfTest(state, "selftest pass");
		state->exitCode = 0;
	} else {
		LogSelfTest(state, "selftest fail failures=%u", selfTest->failureCount);
		state->exitCode = 1;
	}
	state->isQuitRequested = true;
}

static void AdvanceSelfTest(DemoState *state) {
	SelfTestState *selfTest = &state->selfTest;
	++selfTest->targetIndex;
	if (selfTest->targetIndex >= SELF_TEST_TARGET_COUNT) {
		FinishSelfTest(state);
	} else {
		selfTest->phase = SelfTestPhase_Warp;
	}
}

static void FailSelfTest(DemoState *state, const char *reason) {
	SelfTestState *selfTest = &state->selfTest;
	uint32_t targetIndex = selfTest->targetIndex;
	LogSelfTest(state, "selftest target=%u x=%d y=%d result=fail reason=%s", targetIndex, selfTest->targetX[targetIndex], selfTest->targetY[targetIndex], reason);
	++selfTest->failureCount;
	AdvanceSelfTest(state);
}

static void PrepareSelfTestTargets(SelfTestState *selfTest) {
	fplWindowSize windowSize = fplZeroInit;
	fplGetWindowSize(&windowSize);
	int32_t right = (int32_t)windowSize.width - 1 - selfTestEdgeMargin;
	int32_t bottom = (int32_t)windowSize.height - 1 - selfTestEdgeMargin;
	int32_t centerX = (int32_t)windowSize.width / 2;
	int32_t centerY = (int32_t)windowSize.height / 2;
	int32_t xs[SELF_TEST_TARGET_COUNT] = { selfTestEdgeMargin, right, selfTestEdgeMargin, right, centerX };
	int32_t ys[SELF_TEST_TARGET_COUNT] = { selfTestEdgeMargin, selfTestEdgeMargin, bottom, bottom, centerY };
	for (uint32_t targetIndex = 0; targetIndex < SELF_TEST_TARGET_COUNT; ++targetIndex) {
		selfTest->targetX[targetIndex] = xs[targetIndex];
		selfTest->targetY[targetIndex] = ys[targetIndex];
	}
}

static void WarpToSelfTestTarget(DemoState *state) {
	SelfTestState *selfTest = &state->selfTest;
	int32_t targetX = selfTest->targetX[selfTest->targetIndex];
	int32_t targetY = selfTest->targetY[selfTest->targetIndex];
	if (!fplWarpWindowCursor(targetX, targetY)) {
		FailSelfTest(state, "warp-returned-false");
		return;
	}
	int32_t screenX = 0;
	int32_t screenY = 0;
	if (!fplQueryCursorPosition(&screenX, &screenY)) {
		FailSelfTest(state, "cursor-position-unknown");
		return;
	}
	int32_t originX = screenX - targetX;
	int32_t originY = screenY - targetY;
	if (!selfTest->hasClientOrigin) {
		selfTest->clientOriginX = originX;
		selfTest->clientOriginY = originY;
		selfTest->hasClientOrigin = true;
	} else if (originX != selfTest->clientOriginX || originY != selfTest->clientOriginY) {
		LogSelfTest(state, "selftest screen x=%d y=%d expected-x=%d expected-y=%d", screenX, screenY, selfTest->clientOriginX + targetX, selfTest->clientOriginY + targetY);
		FailSelfTest(state, "screen-position");
		return;
	}
	selfTest->phase = SelfTestPhase_WaitForMove;
	selfTest->phaseStartTime = fplMillisecondsQuery();
}

static void UpdateSelfTest(DemoState *state) {
	SelfTestState *selfTest = &state->selfTest;
	fplMilliseconds now = fplMillisecondsQuery();
	fplMilliseconds phaseDuration = now - selfTest->phaseStartTime;
	switch (selfTest->phase) {
		case SelfTestPhase_WaitForFocus:
		{
			fplMilliseconds runningDuration = now - state->startTime;
			if (state->hasFocus) {
				selfTest->phase = SelfTestPhase_WaitForStart;
				selfTest->phaseStartTime = now;
			} else if (runningDuration >= selfTestFocusFallbackMilliseconds) {
				LogSelfTest(state, "selftest no-focus-event");
				selfTest->phase = SelfTestPhase_WaitForStart;
				selfTest->phaseStartTime = now;
			}
		} break;
		case SelfTestPhase_WaitForStart:
			if (phaseDuration >= selfTestStartDelayMilliseconds) {
				PrepareSelfTestTargets(selfTest);
				selfTest->targetIndex = 0;
				selfTest->phase = SelfTestPhase_Warp;
			}
			break;
		case SelfTestPhase_Warp:
			WarpToSelfTestTarget(state);
			break;
		case SelfTestPhase_WaitForMove:
			if (phaseDuration >= selfTestMoveTimeoutMilliseconds) {
				FailSelfTest(state, "no-move-event");
			}
			break;
		default:
			break;
	}
}

static void CheckSelfTestMove(DemoState *state, const fplMouseEvent *mouseEvent) {
	SelfTestState *selfTest = &state->selfTest;
	if (selfTest->phase != SelfTestPhase_WaitForMove) {
		return;
	}
	uint32_t targetIndex = selfTest->targetIndex;
	int32_t targetX = selfTest->targetX[targetIndex];
	int32_t targetY = selfTest->targetY[targetIndex];
	// Other moves (the cursor on its way into the window) are ignored until the target arrives or the wait times out
	if (mouseEvent->mouseX != targetX || mouseEvent->mouseY != targetY) {
		return;
	}
	if (mouseEvent->deltaX != 0 || mouseEvent->deltaY != 0) {
		LogSelfTest(state, "selftest delta dx=%d dy=%d", mouseEvent->deltaX, mouseEvent->deltaY);
		FailSelfTest(state, "move-delta-not-zero");
		return;
	}
	LogSelfTest(state, "selftest target=%u x=%d y=%d result=pass", targetIndex, targetX, targetY);
	AdvanceSelfTest(state);
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
			LogEvent(state, "mouse move x=%d y=%d dx=%d dy=%d", mouseEvent->mouseX, mouseEvent->mouseY, mouseEvent->deltaX, mouseEvent->deltaY);
			if (state->options.runSelfTest) {
				CheckSelfTestMove(state, mouseEvent);
			}
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

	if (state.options.requestMouseGrab) {
		RequestMouseGrab(&state, true);
	}
	if (state.options.requestRelativeMouse) {
		RequestRelativeMouse(&state, true);
	}
	if (state.options.requestKeyboardGrab) {
		RequestKeyboardGrab(&state, true);
	}

	while (fplWindowUpdate() && !state.isQuitRequested) {
		bool hadEvents = ProcessEvents(&state);
		if (state.isQuitRequested) {
			break;
		}
		if (state.options.runSelfTest) {
			UpdateSelfTest(&state);
			if (state.isQuitRequested) {
				break;
			}
		}
		if (IsTimeoutReached(&state)) {
			LogEvent(&state, "demo timeout");
			if (state.options.runSelfTest) {
				LogSelfTest(&state, "selftest fail reason=timeout");
				state.exitCode = 1;
			}
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
	return state.exitCode;
}
