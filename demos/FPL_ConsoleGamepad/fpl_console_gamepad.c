/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ConsoleGamepad

Description:
	Console-only input polling demo. No window, no video, no audio.
	Polls keyboard, mouse, and gamepad without a window — exercises the
	detached X11 Display* (Linux) and HWND_MESSAGE fallback (Win32) added in
	step 9b of the input refactor.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Usage:
	Run the binary. Plug in a gamepad. Move the mouse or press keys.
	Once per second the demo prints: held keys (raw indices + modifiers),
	cursor position + held mouse buttons, and per-controller stick/trigger
	values + held gamepad buttons.
	Press Ctrl+C to quit.

	Note: mouse wheel deltas are always zero in this demo. Without a window
	there is no event source for wheel input (X11 delivers wheel as
	ButtonPress on buttons 4-7; Win32 delivers it as WM_MOUSEWHEEL) and a
	pure-poll path has no "wheel position" to query. Wheel support in
	detached mode is part of the pending step 9b event-flow work.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

static void PrintKeyboard(const fplKeyboardState *s) {
	fplConsoleOut("KB modifiers:");
	if ((s->modifiers & fplKeyboardModifierFlags_LShift) || (s->modifiers & fplKeyboardModifierFlags_RShift)) fplConsoleOut(" Shift");
	if ((s->modifiers & fplKeyboardModifierFlags_LCtrl) || (s->modifiers & fplKeyboardModifierFlags_RCtrl))   fplConsoleOut(" Ctrl");
	if ((s->modifiers & fplKeyboardModifierFlags_LAlt) || (s->modifiers & fplKeyboardModifierFlags_RAlt))     fplConsoleOut(" Alt");
	if ((s->modifiers & fplKeyboardModifierFlags_LSuper) || (s->modifiers & fplKeyboardModifierFlags_RSuper)) fplConsoleOut(" Super");
	fplConsoleOut("  held(raw):");
	uint32_t heldCount = 0;
	for (uint32_t i = 0; i < FPL_MAX_KEYBOARD_STATE_COUNT; ++i) {
		if (!s->keyStatesRaw[i]) continue;
		if (heldCount++ < 16) {
			fplConsoleFormatOut(" 0x%02X", i);
		}
	}
	if (heldCount == 0) fplConsoleOut(" (none)");
	else if (heldCount > 16) fplConsoleFormatOut(" ... +%u", heldCount - 16);
	fplConsoleOut("\n");
}

static void PrintMouse(const fplMouseState *s) {
	fplConsoleFormatOut("MS pos=(%d, %d) wheel=(% .2f, % .2f)  buttons:",
		s->x, s->y, s->wheelDeltaX, s->wheelDeltaY);
	if (s->buttonStates[fplMouseButtonType_Left]   != fplButtonState_Release) fplConsoleOut(" L");
	if (s->buttonStates[fplMouseButtonType_Right]  != fplButtonState_Release) fplConsoleOut(" R");
	if (s->buttonStates[fplMouseButtonType_Middle] != fplButtonState_Release) fplConsoleOut(" M");
	if (s->buttonStates[fplMouseButtonType_X1]     != fplButtonState_Release) fplConsoleOut(" X1");
	if (s->buttonStates[fplMouseButtonType_X2]     != fplButtonState_Release) fplConsoleOut(" X2");
	fplConsoleOut("\n");
}

static void PrintGamepad(uint32_t index, const fplGamepadState *s) {
	fplConsoleFormatOut("[%u] %s  L(% .2f, % .2f) R(% .2f, % .2f) LT=%.2f RT=%.2f  buttons:",
		index,
		s->deviceName != fpl_null ? s->deviceName : "(unnamed)",
		s->leftStickX, s->leftStickY,
		s->rightStickX, s->rightStickY,
		s->leftTrigger, s->rightTrigger);
	if (s->dpadUp.isDown)        fplConsoleOut(" DUp");
	if (s->dpadDown.isDown)      fplConsoleOut(" DDown");
	if (s->dpadLeft.isDown)      fplConsoleOut(" DLeft");
	if (s->dpadRight.isDown)     fplConsoleOut(" DRight");
	if (s->actionA.isDown)       fplConsoleOut(" A");
	if (s->actionB.isDown)       fplConsoleOut(" B");
	if (s->actionX.isDown)       fplConsoleOut(" X");
	if (s->actionY.isDown)       fplConsoleOut(" Y");
	if (s->leftShoulder.isDown)  fplConsoleOut(" LB");
	if (s->rightShoulder.isDown) fplConsoleOut(" RB");
	if (s->leftThumb.isDown)     fplConsoleOut(" LS");
	if (s->rightThumb.isDown)    fplConsoleOut(" RS");
	if (s->back.isDown)          fplConsoleOut(" Back");
	if (s->start.isDown)         fplConsoleOut(" Start");
	fplConsoleOut("\n");
}

int main(int argc, char *args[]) {
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	if (!fplPlatformInit(fplInitFlags_Input, &settings)) {
		return 1;
	}

	fplConsoleOut("Console input demo (no window). Press Ctrl+C to quit.\n");
	fplConsoleOut("Move the mouse, press keys, plug in a gamepad.\n\n");

	uint64_t lastPrint = fplMillisecondsQuery();
	bool wasConnected[4] = { false, false, false, false };

	for (;;) {
		fplUpdateInputDevices();

		fplGamepadStates states = fplZeroInit;
		fplPollGamepadStates(&states);

		for (uint32_t i = 0; i < fplArrayCount(states.deviceStates); ++i) {
			const fplGamepadState *s = &states.deviceStates[i];
			if (s->isConnected != wasConnected[i]) {
				fplConsoleFormatOut("[%u] %s\n", i, s->isConnected ? "connected" : "disconnected");
				wasConnected[i] = s->isConnected;
			}
		}

		uint64_t now = fplMillisecondsQuery();
		if (now - lastPrint >= 1000) {
			lastPrint = now;

			fplKeyboardState kbState = fplZeroInit;
			if (fplPollKeyboardState(&kbState)) {
				PrintKeyboard(&kbState);
			} else {
				fplConsoleOut("(keyboard polling not available)\n");
			}

			fplMouseState msState = fplZeroInit;
			if (fplPollMouseState(&msState)) {
				PrintMouse(&msState);
			} else {
				fplConsoleOut("(mouse polling not available)\n");
			}

			bool anyConnected = false;
			for (uint32_t i = 0; i < fplArrayCount(states.deviceStates); ++i) {
				if (states.deviceStates[i].isConnected) {
					PrintGamepad(i, &states.deviceStates[i]);
					anyConnected = true;
				}
			}
			if (!anyConnected) {
				fplConsoleOut("(no gamepad connected)\n");
			}
			fplConsoleOut("\n");
		}

		fplThreadSleep(16);
	}

	fplPlatformRelease();
	return 0;
}
