/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ConsoleGamepad

Description:
	Console-only gamepad polling demo. No window, no video, no audio.
	Verifies that gamepad backends (XInput on Win32, Linux joystick on Linux)
	work without a window after step 9a of the input refactor.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Usage:
	Run the binary, plug in a gamepad. Press buttons or move sticks.
	Stick/trigger values and pressed buttons are printed once per second.
	Press Ctrl+C to quit.

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
	if (!fplPlatformInit(fplInitFlags_GameController, &settings)) {
		return 1;
	}

	fplConsoleOut("Console gamepad demo. Press Ctrl+C to quit.\n");
	fplConsoleOut("Plug in a gamepad and move sticks / press buttons.\n\n");

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
		}

		fplThreadSleep(16);
	}

	fplPlatformRelease();
	return 0;
}
