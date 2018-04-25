/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Crackout
Description:
	A breakout-like game based on FPL
Requirements:
	- C++ Compiler
	- GLM
	- Final Dynamic OpenGL
Author:
	Torsten Spaete
Changelog:
	## 2018-04-24:
	- Initial creation of this description block
State:
	- Incomplete
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#include "game.cpp"

#include "utils.h"

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

static void ProcessEvents(Input *currentInput, Input *prevInput, bool &isWindowActive) {
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
				glm::u32 controllerIndex = 1 + event.gamepad.deviceIndex;
				assert(controllerIndex < ArrayCount(currentInput->controllers));

				Controller *currentController = &currentInput->controllers[controllerIndex];
				Controller *prevController = &prevInput->controllers[controllerIndex];
				switch(event.gamepad.type) {
					case fplGamepadEventType_Connected:
					{
						currentController->isConnected = true;
					} break;
					case fplGamepadEventType_Disconnected:
					{
						currentController->isConnected = false;
					} break;
					case fplGamepadEventType_StateChanged:
					{
						fplGamepadState &padstate = event.gamepad.state;
						assert(currentController->isConnected);
						if(glm::abs<float>(padstate.leftStickX) > 0.0f || glm::abs<float>(padstate.leftStickY) > 0.0f) {
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
					} break;
				}
			} break;

			case fplEventType_Mouse:
			{
				switch(event.mouse.type) {
					case fplMouseEventType_Move:
					{
						currentInput->mouse.pos = glm::ivec2(event.mouse.mouseX, event.mouse.mouseY);
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
						if (!currentKeyboardController->isConnected) {
							currentKeyboardController->isConnected = true;
						}

						bool isDown = (event.keyboard.type == fplKeyboardEventType_KeyDown) ? 1 : 0;
						switch(event.keyboard.mappedKey) {
							case fplKey_F1:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->editorToggle);
								break;
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
						}
					} break;
				}
			} break;
		}
	}
}

int main(int argc, char *argv[]) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	settings.video.isVSync = true;
	int result = 0;
	if(fplPlatformInit(fplInitFlags_All, &settings)) {
		GameState *game = GameCreate();
		if(game != nullptr) {
			Input inputs[2] = {};
			Input *curInput = &inputs[0];
			Input *prevInput = &inputs[1];
			bool gameActive = true;
			double lastTime = fplGetTimeInSecondsHP();
			double targetDeltaTime = 1.0 / 60.0;
			double accumulatedDelta = 0.0;
			while(fplWindowUpdate()) {
				ProcessEvents(curInput, prevInput, gameActive);
				fplWindowSize winArea;
				if(fplGetWindowArea(&winArea)) {
					curInput->windowSize.x = winArea.width;
					curInput->windowSize.y = winArea.height;
				}
				if(gameActive) {
					curInput->deltaTime = (glm::f32)targetDeltaTime;
#if 1
					while(accumulatedDelta >= targetDeltaTime) {
						GameUpdate(*game, *curInput);
						accumulatedDelta -= targetDeltaTime;
					}
#else
					GameUpdate(*game, *curInput);
#endif
					GameDraw(*game);
				} else {
					lastTime = fplGetTimeInSecondsHP();
				}
				fplVideoFlip();

				if(gameActive) {
					double deltaTime = fplGetTimeInSecondsHP() - lastTime;
					accumulatedDelta += deltaTime;
					accumulatedDelta = glm::min<double>(accumulatedDelta, 0.1);
				}
			}
			GameDestroy(game);
		}
		fplPlatformRelease();
	} else {
	}
	return (result);
}