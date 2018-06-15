/*
Name:
	Final Game Platform
Description:
	One main function for bootstrapping a game.
	This contains all the platform specific code.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_GAMEPLATFORM_H
#define FINAL_GAMEPLATFORM_H

#if !(defined(__cplusplus) && ((__cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900)))
#error "C++/11 compiler not detected!"
#endif

struct GameConfiguration {
	const char *title;
	bool hideMouseCursor;
	bool disableInactiveDetection;
};

extern int GameMain(const GameConfiguration &config);

#endif // FINAL_GAMEPLATFORM_H

#if defined(FINAL_GAMEPLATFORM_IMPLEMENTATION) && !defined(FINAL_GAMEPLATFORM_IMPLEMENTED)
#define FINAL_GAMEPLATFORM_IMPLEMENTED

#include <final_platform_layer.h>

#include "final_game.h"

static void UpdateKeyboardButtonState(const bool isDown, ButtonState &targetButton) {
	if(isDown != targetButton.isDown) {
		targetButton.isDown = isDown;
		++targetButton.halfTransitionCount;
	}
}

static void UpdateDigitalButtonState(const bool isDown, const ButtonState &oldState, ButtonState &newState) {
	newState.isDown = isDown;
	newState.halfTransitionCount = (oldState.isDown != newState.isDown) ? 1 : 0;
}

static void UpdateDefaultController(Input *currentInput, int newIndex) {
	if(newIndex != -1) {
		currentInput->defaultControllerIndex = newIndex;
	} else {
		currentInput->defaultControllerIndex = -1;
		for(int i = FPL_ARRAYCOUNT(currentInput->controllers) - 1; i > 0; i--) {
			if(currentInput->controllers[i].isConnected) {
				currentInput->defaultControllerIndex = i;
				break;
			}
		}
	}
}

static void ProcessEvents(Input *currentInput, Input *prevInput, bool &isWindowActive, Vec2i &lastMousePos) {
	Controller *currentKeyboardController = &currentInput->keyboard;
	Controller *prevKeyboardController = &prevInput->keyboard;
	fplEvent event;
	while(fplPollEvent(&event)) {
		switch(event.type) {
			case fplEventType_Window:
			{
				switch(event.window.type) {
					case fplWindowEventType_GotFocus:
						isWindowActive = true;
						break;
					case fplWindowEventType_LostFocus:
						isWindowActive = false;
						break;
				}
			} break;

			case fplEventType_Gamepad:
			{
				// @CLEANUP: For now we just use the device index, but later it should be "added" to the controllers array and remembered somehow
				uint32_t controllerIndex = 1 + event.gamepad.deviceIndex;
				FPL_ASSERT(controllerIndex < FPL_ARRAYCOUNT(currentInput->controllers));
				Controller *currentController = &currentInput->controllers[controllerIndex];
				Controller *prevController = &prevInput->controllers[controllerIndex];
				switch(event.gamepad.type) {
					case fplGamepadEventType_Connected:
					{
						currentController->isConnected = true;
						UpdateDefaultController(currentInput, controllerIndex);
					} break;
					case fplGamepadEventType_Disconnected:
					{
						currentController->isConnected = false;
						UpdateDefaultController(currentInput, -1);
					} break;
					case fplGamepadEventType_StateChanged:
					{
						fplGamepadState &padstate = event.gamepad.state;
						assert(currentController->isConnected);
						if(fabs(padstate.leftStickX) > 0.0f || fabs(padstate.leftStickY) > 0.0f) {
							currentController->isAnalog = true;
							currentController->analogMovement.x = padstate.leftStickX;
							currentController->analogMovement.y = padstate.leftStickY;
						} else {
							currentController->isAnalog = false;
							UpdateDigitalButtonState(padstate.dpadDown.isDown, prevController->moveDown, currentController->moveDown);
							UpdateDigitalButtonState(padstate.dpadUp.isDown, prevController->moveUp, currentController->moveUp);
							UpdateDigitalButtonState(padstate.dpadLeft.isDown, prevController->moveLeft, currentController->moveLeft);
							UpdateDigitalButtonState(padstate.dpadRight.isDown, prevController->moveRight, currentController->moveRight);
						}
						UpdateDigitalButtonState(padstate.actionA.isDown, prevController->actionDown, currentController->actionDown);
						UpdateDigitalButtonState(padstate.actionB.isDown, prevController->actionRight, currentController->actionRight);
						UpdateDigitalButtonState(padstate.actionX.isDown, prevController->actionLeft, currentController->actionLeft);
						UpdateDigitalButtonState(padstate.actionY.isDown, prevController->actionUp, currentController->actionUp);
						UpdateDigitalButtonState(padstate.back.isDown, prevController->actionBack, currentController->actionBack);
					} break;
				}
			} break;

			case fplEventType_Mouse:
			{
				switch(event.mouse.type) {
					case fplMouseEventType_Move:
					{
						currentInput->mouse.pos = lastMousePos = V2i(event.mouse.mouseX, event.mouse.mouseY);
					} break;

					case fplMouseEventType_ButtonDown:
					case fplMouseEventType_ButtonUp:
					{
						bool isDown = event.mouse.type == fplMouseEventType_ButtonDown;
						if(event.mouse.mouseButton == fplMouseButtonType_Left) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.left);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Right) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.right);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Middle) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.middle);
						}
					} break;

					case fplMouseEventType_Wheel:
					{
						currentInput->mouse.wheelDelta = event.mouse.wheelDelta;
					} break;
				}
			} break;

			case fplEventType_Keyboard:
			{
				switch(event.keyboard.type) {
					case fplKeyboardEventType_CharInput:
					{
					} break;
					case fplKeyboardEventType_KeyDown:
					case fplKeyboardEventType_KeyUp:
					{
						if(!currentKeyboardController->isConnected) {
							currentKeyboardController->isConnected = true;
							if(currentInput->defaultControllerIndex == -1) {
								UpdateDefaultController(currentInput, 0);
							}
						}
						bool isDown = (event.keyboard.type == fplKeyboardEventType_KeyDown) ? 1 : 0;
						switch(event.keyboard.mappedKey) {
							case fplKey_A:
							case fplKey_Left:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveLeft);
								break;
							case fplKey_D:
							case fplKey_Right:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveRight);
								break;
							case fplKey_W:
							case fplKey_Up:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveUp);
								break;
							case fplKey_S:
							case fplKey_Down:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveDown);
								break;
							case fplKey_Space:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->actionDown);
								break;
							case fplKey_F:
							{
								if(!isDown) {
									bool wasFullscreen = fplIsWindowFullscreen();
									fplSetWindowFullscreen(!wasFullscreen, 0, 0, 0);
								}
							} break;
							case fplKey_Escape:
							{
								UpdateKeyboardButtonState(isDown, currentKeyboardController->actionBack);
							} break;
						}
					} break;
				}
			} break;
		}
	}
}

extern int GameMain(const GameConfiguration &config) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	settings.video.isVSync = true;
	fplCopyAnsiString(config.title, settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	int result = 0;
	if(fplPlatformInit(fplInitFlags_All, &settings)) {
		GameMemory gameMem = GameCreate();
		if(gameMem.base != nullptr) {
			const double TargetDeltaTime = 1.0 / 60.0;

			if(config.hideMouseCursor) {
				fplSetWindowCursorEnabled(false);
			}

			Input inputs[2] = {};
			Input *curInput = &inputs[0];
			Input *prevInput = &inputs[1];
			Vec2i lastMousePos = V2i(-1, -1);
			bool isWindowActive = true;
			curInput->defaultControllerIndex = -1;

			uint32_t frameCount = 0;
			uint32_t updateCount = 0;
			double lastTime = fplGetTimeInSecondsHP();
			double fpsTimerInSecs = fplGetTimeInSecondsHP();
			double frameAccumulator = TargetDeltaTime;

			while(!IsGameExiting(gameMem) && fplWindowUpdate()) {
				// Window size
				fplWindowSize winArea;
				if(fplGetWindowArea(&winArea)) {
					curInput->windowSize.x = winArea.width;
					curInput->windowSize.y = winArea.height;
				}

				// Remember previous keyboard and mouse state
				Controller *currentKeyboardController = &curInput->keyboard;
				Controller *prevKeyboardController = &prevInput->keyboard;
				Mouse *currentMouse = &curInput->mouse;
				Mouse *prevMouse = &prevInput->mouse;
				*currentKeyboardController = {};
				*currentMouse = {};
				currentKeyboardController->isConnected = prevKeyboardController->isConnected;
				for(uint32_t buttonIndex = 0; buttonIndex < FPL_ARRAYCOUNT(currentKeyboardController->buttons); ++buttonIndex) {
					currentKeyboardController->buttons[buttonIndex].isDown = prevKeyboardController->buttons[buttonIndex].isDown;
				}
				for(uint32_t buttonIndex = 0; buttonIndex < FPL_ARRAYCOUNT(currentMouse->buttons); ++buttonIndex) {
					currentMouse->buttons[buttonIndex] = prevMouse->buttons[buttonIndex];
					currentMouse->buttons[buttonIndex].halfTransitionCount = 0;
				}
				currentMouse->pos = lastMousePos;

				// Remember previous gamepad connected states
				for(uint32_t controllerIndex = 1; controllerIndex < FPL_ARRAYCOUNT(curInput->controllers); ++controllerIndex) {
					Controller *currentGamepadController = &curInput->controllers[controllerIndex];
					Controller *prevGamepadController = &prevInput->controllers[controllerIndex];
					currentGamepadController->isConnected = prevGamepadController->isConnected;
					currentGamepadController->isAnalog = prevGamepadController->isAnalog;
				}

				// Set time states
				curInput->deltaTime = (float)TargetDeltaTime;

				// Events
				ProcessEvents(curInput, prevInput, isWindowActive, lastMousePos);

				// Game Update
				bool gameActive = config.disableInactiveDetection ? true : isWindowActive;
				GameInput(gameMem, *curInput, gameActive);
#if 1
				while(frameAccumulator >= TargetDeltaTime) {
					GameUpdate(gameMem, *curInput, gameActive);
					frameAccumulator -= TargetDeltaTime;
					++updateCount;
				}
#else
				GameUpdate(gameMem, *curInput, isWindowActive);
				++updateCount;
#endif

				// @TODO(final): Yield thread when we are running too fast
				{
					double endWorkTime = fplGetTimeInSecondsHP();
					double workDuration = endWorkTime - lastTime;
				}

				// Render
				float alpha = (float)frameAccumulator / (float)TargetDeltaTime;
				GameRender(gameMem, alpha, curInput->deltaTime);
				fplVideoFlip();
				++frameCount;

				// Timing
				double endTime = fplGetTimeInSecondsHP();
				double frameDuration = endTime - lastTime;
				frameAccumulator += frameDuration;
				frameAccumulator = FPL_MIN(0.1, frameAccumulator);
				lastTime = endTime;
				if(endTime >= (fpsTimerInSecs + 1.0)) {
					fpsTimerInSecs = endTime;
					char charBuffer[256];
					fplFormatAnsiString(charBuffer, FPL_ARRAYCOUNT(charBuffer), "Fps: %d, Ups: %d\n", frameCount, updateCount);
					//OutputDebugStringA(charBuffer);
					frameCount = 0;
					updateCount = 0;
				}

				// Swap input
				{
					Input *tmp = curInput;
					curInput = prevInput;
					prevInput = tmp;
				}
			}

			if(config.hideMouseCursor) {
				fplSetWindowCursorEnabled(true);
			}

			GameDestroy(gameMem);
		} else {
			result = -1;
		}
		fplPlatformRelease();
	} else {
		result = -1;
	}
	return (result);
}

#endif // FINAL_GAMEPLATFORM_IMPLEMENTATION