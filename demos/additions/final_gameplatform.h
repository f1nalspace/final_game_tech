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

#include "final_game.h"

struct GameConfiguration {
	const char *title;
	bool hideMouseCursor;
	bool disableInactiveDetection;
	bool noUpdateRenderSeparation;
};

extern int GameMain(const GameConfiguration &config);

#endif // FINAL_GAMEPLATFORM_H

#if defined(FINAL_GAMEPLATFORM_IMPLEMENTATION) && !defined(FINAL_GAMEPLATFORM_IMPLEMENTED)
#define FINAL_GAMEPLATFORM_IMPLEMENTED

#include <final_platform_layer.h>

#define FMEM_IMPLEMENTATION
#include <final_memory.h>

#define FINAL_RENDER_IMPLEMENTATION
#include <final_render.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_OPENGL_RENDER_IMPLEMENTATION
#include <final_opengl_render.h>

static void UpdateKeyboardButtonState(const fplButtonState state, const ButtonState &oldState, ButtonState &targetButton) {
	if(state != targetButton.state) {
		if(state == fplButtonState_Press && oldState.state == fplButtonState_Release) {
			targetButton.state = fplButtonState_Press;
		} else if(state == fplButtonState_Release && oldState.state >= fplButtonState_Press) {
			targetButton.state = fplButtonState_Release;
		} else {
			targetButton.state = state;
		}
		++targetButton.halfTransitionCount;
	}
}

static void UpdateDigitalButtonState(const bool isDown, const ButtonState &oldState, ButtonState &newState) {
	newState.state = isDown ? fplButtonState_Press : fplButtonState_Release;
	newState.halfTransitionCount = (oldState.state != newState.state) ? 1 : 0;
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

static void ProcessEvents(Input *currentInput, Input *prevInput, GameWindowActiveType &windowActiveType, Vec2i &lastMousePos) {
	Controller *currentKeyboardController = &currentInput->keyboard;
	Controller *prevKeyboardController = &prevInput->keyboard;
	fplEvent event;
	while(fplPollEvent(&event)) {
		switch(event.type) {
			case fplEventType_Window:
			{
				switch(event.window.type) {
					case fplWindowEventType_GotFocus:
						windowActiveType = GameWindowActiveType::GotFocus;
						break;
					case fplWindowEventType_Restored:
						windowActiveType = GameWindowActiveType::Restored;
						break;
					case fplWindowEventType_Maximized:
						windowActiveType = GameWindowActiveType::Maximized;
						break;
					case fplWindowEventType_LostFocus:
						windowActiveType = GameWindowActiveType::LostFocus;
						break;
					case fplWindowEventType_Minimized:
						windowActiveType = GameWindowActiveType::Minimized;
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
						UpdateDigitalButtonState(padstate.start.isDown, prevController->actionStart, currentController->actionStart);
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

					case fplMouseEventType_Button:
					{
						if(event.mouse.mouseButton == fplMouseButtonType_Left) {
							UpdateKeyboardButtonState(event.mouse.buttonState, prevInput->mouse.left, currentInput->mouse.left);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Right) {
							UpdateKeyboardButtonState(event.mouse.buttonState, prevInput->mouse.right, currentInput->mouse.right);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Middle) {
							UpdateKeyboardButtonState(event.mouse.buttonState, prevInput->mouse.middle, currentInput->mouse.middle);
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
					case fplKeyboardEventType_Input:
					{
					} break;
					case fplKeyboardEventType_Button:
					{
						if(!currentKeyboardController->isConnected) {
							currentKeyboardController->isConnected = true;
							if(currentInput->defaultControllerIndex == -1) {
								UpdateDefaultController(currentInput, 0);
							}
						}
						switch(event.keyboard.mappedKey) {
							case fplKey_A:
							case fplKey_Left:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->moveLeft, currentKeyboardController->moveLeft);
								break;
							case fplKey_D:
							case fplKey_Right:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->moveRight, currentKeyboardController->moveRight);
								break;
							case fplKey_W:
							case fplKey_Up:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->moveUp, currentKeyboardController->moveUp);
								break;
							case fplKey_S:
							case fplKey_Down:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->moveDown, currentKeyboardController->moveDown);
								break;
							case fplKey_Space:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->actionDown, currentKeyboardController->actionDown);
								break;
							case fplKey_F:
							{
								if(event.keyboard.buttonState == fplButtonState_Release) {
									bool wasFullscreen = fplIsWindowFullscreen();
									fplSetWindowFullscreen(!wasFullscreen, 0, 0, 0);
								}
							} break;
							case fplKey_F4:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->debugToggle, currentKeyboardController->debugToggle);
								break;
							case fplKey_Enter:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->actionStart, currentKeyboardController->actionStart);
								break;
							case fplKey_Escape:
								UpdateKeyboardButtonState(event.keyboard.buttonState, prevKeyboardController->actionBack, currentKeyboardController->actionBack);
								break;
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

	if(!fplPlatformInit(fplInitFlags_All, &settings)) {
		return -1;
	}

	if(!fglLoadOpenGL(true)) {
		fplPlatformRelease();
		return -1;
	}

	bool wasError = false;

	fmemMemoryBlock gameMemoryBlock = {};
	if(!fmemInit(&gameMemoryBlock, fmemType_Growable, FMEM_MEGABYTES(128))) {
		wasError = true;
	}
	fmemMemoryBlock renderMemoryBlock = {};
	if(!fmemInit(&renderMemoryBlock, fmemType_Growable, FMEM_MEGABYTES(32))) {
		wasError = true;
	}
	RenderState renderState = {};
	InitRenderState(renderState, renderMemoryBlock);

	GameMemory gameMem = {};
	gameMem.persistentMemory = gameMemoryBlock;
	gameMem.render = &renderState;
	if(!GameInit(gameMem)) {
		wasError = true;
	}

	if(!wasError) {
		const double TargetDeltaTime = 1.0 / 60.0;

		if(config.hideMouseCursor) {
			fplSetWindowCursorEnabled(false);
		}

		Input inputs[2] = {};
		Input *curInput = &inputs[0];
		Input *prevInput = &inputs[1];
		Vec2i lastMousePos = V2i(-1, -1);
		GameWindowActiveType windowActiveType[2] = { GameWindowActiveType::None, GameWindowActiveType::None };
		curInput->defaultControllerIndex = -1;

		uint32_t frameCount = 0;
		uint32_t updateCount = 0;
		double lastTime = fplGetTimeInSecondsHP();
		double fpsTimerInSecs = fplGetTimeInSecondsHP();
		double frameAccumulator = TargetDeltaTime;
		double lastFramesPerSecond = 0.0;
		double lastFrameTime = 0.0;

		while(!IsGameExiting(gameMem) && fplWindowUpdate()) {
			// Window size
			fplWindowSize winArea;
			if(fplGetWindowArea(&winArea)) {
				curInput->windowSize.x = winArea.width;
				curInput->windowSize.y = winArea.height;
			}

			// Remember previous keyboard and mouse state
			windowActiveType[1] = windowActiveType[0];
			Controller *currentKeyboardController = &curInput->keyboard;
			Controller *prevKeyboardController = &prevInput->keyboard;
			Mouse *currentMouse = &curInput->mouse;
			Mouse *prevMouse = &prevInput->mouse;
			*currentKeyboardController = {};
			*currentMouse = {};
			currentKeyboardController->isConnected = prevKeyboardController->isConnected;
			for(uint32_t buttonIndex = 0; buttonIndex < FPL_ARRAYCOUNT(currentKeyboardController->buttons); ++buttonIndex) {
				currentKeyboardController->buttons[buttonIndex].state = prevKeyboardController->buttons[buttonIndex].state;
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

			// Events
			ProcessEvents(curInput, prevInput, windowActiveType[0], lastMousePos);
			if(config.disableInactiveDetection) {
				curInput->isActive = (windowActiveType[0] & GameWindowActiveType::Minimized) != GameWindowActiveType::Minimized;
			} else {
				curInput->isActive = ((windowActiveType[0] & GameWindowActiveType::Minimized) != GameWindowActiveType::Minimized) && ((windowActiveType[0] & GameWindowActiveType::LostFocus) != GameWindowActiveType::LostFocus);
			}
			curInput->deltaTime = (float)TargetDeltaTime;
			curInput->framesPerSeconds = (float)lastFramesPerSecond;

			if(windowActiveType[0] != windowActiveType[1]) {
				// We dont want to have delta time jumps
				lastTime = fplGetTimeInSecondsHP();
				lastFramesPerSecond = 0.0f;
				fpsTimerInSecs = fplGetTimeInSecondsHP();
				updateCount = frameCount = 0;
				frameAccumulator = TargetDeltaTime;
			}

			ResetRenderState(renderState);

			// Game Update
			if(config.noUpdateRenderSeparation) {
				float alpha;
				if(lastFrameTime > 0) {
					alpha = 1.0f - (float)(lastFrameTime / TargetDeltaTime);
				} else {
					alpha = 1.0f;
				}
				GameUpdateAndRender(gameMem, *curInput, alpha);
			} else {
				GameInput(gameMem, *curInput);
#if 1
				while(frameAccumulator >= TargetDeltaTime) {
					GameUpdate(gameMem, *curInput);
					frameAccumulator -= TargetDeltaTime;
					++updateCount;
				}
#else
				GameUpdate(gameMem, *curInput);
				++updateCount;
#endif
				}

				// @TODO(final): Yield thread when we are running too fast
			double endWorkTime = fplGetTimeInSecondsHP();
			double workDuration = endWorkTime - lastTime;

			// Render
			if(!config.noUpdateRenderSeparation) {
				float alpha = (float)frameAccumulator / (float)TargetDeltaTime;
				GameRender(gameMem, alpha);
			}
			RenderWithOpenGL(renderState);
			fplVideoFlip();
			++frameCount;

			// Timing
			double endTime = fplGetTimeInSecondsHP();
			double frameDuration = endTime - lastTime;
			lastFrameTime = frameDuration;
			lastFramesPerSecond = 1.0f / frameDuration;
			if(!config.noUpdateRenderSeparation) {
				frameAccumulator += frameDuration;
				frameAccumulator = FPL_MIN(0.1, frameAccumulator);
			}
			lastTime = endTime;
			if(endTime >= (fpsTimerInSecs + 1.0)) {
				fpsTimerInSecs = endTime;
#if 0
				char charBuffer[256];
				fplFormatAnsiString(charBuffer, FPL_ARRAYCOUNT(charBuffer), "Fps: %d, Ups: %d\n", frameCount, updateCount);
				OutputDebugStringA(charBuffer);
#endif
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

		GameRelease(gameMem);
	}

	fmemFree(&gameMemoryBlock);
	fmemFree(&renderMemoryBlock);

	fglUnloadOpenGL();

	fplPlatformRelease();

	int result = wasError ? -1 : 0;
	return (result);
}

#endif // FINAL_GAMEPLATFORM_IMPLEMENTATION