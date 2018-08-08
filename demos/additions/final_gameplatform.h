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

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define FINAL_RENDER_IMPLEMENTATION
#include "final_render.h"

#define FINAL_AUDIOSYSTEM_IMPLEMENTATION
#include "final_audiosystem.h"

#define FINAL_OPENGL_RENDER_IMPLEMENTATION
#include "final_opengl_render.h"

static void UpdateKeyboardButtonState(ButtonState &newState, const fpl_b32 isDown) {
	FPL_ASSERT(newState.endedDown != isDown);
	newState.endedDown = isDown;
	++newState.halfTransitionCount;
}

static void UpdateDigitalButtonState(const ButtonState &oldState, ButtonState &newState, const fpl_b32 isDown) {
	newState.endedDown = isDown;
	newState.halfTransitionCount = ((newState.endedDown == oldState.endedDown) ? 0 : 1);
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
	Controller *newKeyboardController = &currentInput->keyboard;
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
				// @TODO(final): For now we just use the device index, but later it should be "added" to the controllers array and remembered somehow
				uint32_t controllerIndex = 1 + event.gamepad.deviceIndex;
				FPL_ASSERT(controllerIndex < FPL_ARRAYCOUNT(currentInput->controllers));
				Controller *newController = &currentInput->controllers[controllerIndex];
				Controller *oldController = &prevInput->controllers[controllerIndex];
				switch(event.gamepad.type) {
					case fplGamepadEventType_Connected:
					{
						newController->isConnected = true;
						UpdateDefaultController(currentInput, controllerIndex);
					} break;
					case fplGamepadEventType_Disconnected:
					{
						newController->isConnected = false;
						UpdateDefaultController(currentInput, -1);
					} break;
					case fplGamepadEventType_StateChanged:
					{
						fplGamepadState &padstate = event.gamepad.state;
						assert(newController->isConnected);
						if(Abs(padstate.leftStickX) > 0.0f || Abs(padstate.leftStickY) > 0.0f) {
							newController->isAnalog = true;
							newController->analogMovement.x = padstate.leftStickX;
							newController->analogMovement.y = padstate.leftStickY;
						} else {
							newController->isAnalog = false;
							UpdateDigitalButtonState(oldController->moveDown, newController->moveDown, padstate.dpadDown.isDown != 0);
							UpdateDigitalButtonState(oldController->moveUp, newController->moveUp, padstate.dpadUp.isDown != 0);
							UpdateDigitalButtonState(oldController->moveLeft, newController->moveLeft, padstate.dpadLeft.isDown != 0);
							UpdateDigitalButtonState(oldController->moveRight, newController->moveRight, padstate.dpadRight.isDown != 0);
						}
						UpdateDigitalButtonState(oldController->actionDown, newController->actionDown, padstate.actionA.isDown != 0);
						UpdateDigitalButtonState(oldController->actionRight, newController->actionRight, padstate.actionB.isDown != 0);
						UpdateDigitalButtonState(oldController->actionLeft, newController->actionLeft, padstate.actionX.isDown != 0);
						UpdateDigitalButtonState(oldController->actionUp, newController->actionUp, padstate.actionY.isDown != 0);
						UpdateDigitalButtonState(oldController->actionBack, newController->actionBack, padstate.back.isDown != 0);
						UpdateDigitalButtonState(oldController->actionStart, newController->actionStart, padstate.start.isDown != 0);
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
						bool isDown = event.mouse.buttonState >= fplButtonState_Press;
						if(event.mouse.mouseButton == fplMouseButtonType_Left) {
							UpdateKeyboardButtonState(currentInput->mouse.left, isDown);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Right) {
							UpdateKeyboardButtonState(currentInput->mouse.right, isDown);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Middle) {
							UpdateKeyboardButtonState(currentInput->mouse.middle, isDown);
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
						if(!newKeyboardController->isConnected) {
							newKeyboardController->isConnected = true;
							if(currentInput->defaultControllerIndex == -1) {
								UpdateDefaultController(currentInput, 0);
							}
						}
						bool isDown = event.keyboard.buttonState >= fplButtonState_Press;
						bool wasDown = event.keyboard.buttonState == fplButtonState_Release || event.keyboard.buttonState == fplButtonState_Repeat;
						if(isDown != wasDown) {
							//fplDebugFormatOut("Frame %d, Key button change[%d] = %d\n", currentInput->frameIndex, event.keyboard.keyCode, event.keyboard.buttonState);
							switch(event.keyboard.mappedKey) {
								case fplKey_A:
								case fplKey_Left:
									UpdateKeyboardButtonState(newKeyboardController->moveLeft, isDown);
									break;
								case fplKey_D:
								case fplKey_Right:
									UpdateKeyboardButtonState(newKeyboardController->moveRight, isDown);
									break;
								case fplKey_W:
								case fplKey_Up:
									UpdateKeyboardButtonState(newKeyboardController->moveUp, isDown);
									break;
								case fplKey_S:
								case fplKey_Down:
									UpdateKeyboardButtonState(newKeyboardController->moveDown, isDown);
									break;
								case fplKey_Space:
									UpdateKeyboardButtonState(newKeyboardController->actionDown, isDown);
									break;
								case fplKey_F4:
									UpdateKeyboardButtonState(newKeyboardController->debugToggle, isDown);
									break;
								case fplKey_R:
									UpdateKeyboardButtonState(newKeyboardController->debugReload, isDown);
									break;
								case fplKey_Return:
									UpdateKeyboardButtonState(newKeyboardController->actionStart, isDown);
									break;
								case fplKey_Escape:
									UpdateKeyboardButtonState(newKeyboardController->actionBack, isDown);
									break;
							}
						} else {
							//fplDebugFormatOut("Frame %d, Key button press[%d] = %d\n", currentInput->frameIndex, event.keyboard.keyCode, event.keyboard.buttonState);
						}
						if(wasDown) {
							if(event.keyboard.mappedKey == fplKey_F) {
								bool wasFullscreen = fplIsWindowFullscreen();
								fplSetWindowFullscreen(!wasFullscreen, 0, 0, 0);
							}
						}
					} break;
				}
			} break;
		}
	}
}

static uint32_t GameAudioPlayback(const fplAudioDeviceFormat *outFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioSystem *audioSys = (AudioSystem *)userData;
	uint32_t result = AudioSystemWriteSamples(audioSys, outFormat, frameCount, (uint8_t *)outputSamples);
	return(result);
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

	AudioSystem audioSys = {};
	if(!AudioSystemInit(&audioSys)) {
		wasError = true;
	}

	fplSetAudioClientReadCallback(GameAudioPlayback, &audioSys);
	if(fplPlayAudio() != fplAudioResult_Success) {
		wasError = true;
	}

	RenderState renderState = {};
	InitRenderState(renderState, renderMemoryBlock);
	InitOpenGLRenderer();

	GameMemory gameMem = {};
	gameMem.memory = &gameMemoryBlock;
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
		Input *newInput = &inputs[0];
		Input *oldInput = &inputs[1];
		Vec2i lastMousePos = V2i(-1, -1);
		GameWindowActiveType windowActiveType[2] = { GameWindowActiveType::None, GameWindowActiveType::None };
		newInput->defaultControllerIndex = -1;

		uint32_t frameCount = 0;
		uint32_t updateCount = 0;
		double lastTime = fplGetTimeInSecondsHP();
		double fpsTimerInSecs = fplGetTimeInSecondsHP();
		double frameAccumulator = TargetDeltaTime;
		double lastFramesPerSecond = 0.0;
		double lastFrameTime = 0.0;
		int frameIndex = 0;

		while(!IsGameExiting(gameMem) && fplWindowUpdate()) {
			// Window size
			fplWindowSize winArea;
			if(fplGetWindowArea(&winArea)) {
				newInput->windowSize.x = winArea.width;
				newInput->windowSize.y = winArea.height;
			}

			// Remember previous keyboard and mouse state
			windowActiveType[1] = windowActiveType[0];
			Controller *oldKeyboardController = &oldInput->keyboard;
			Controller *newKeyboardController = &newInput->keyboard;
			*newKeyboardController = {};
			newKeyboardController->isConnected = oldKeyboardController->isConnected;
			newKeyboardController->actionDown.name = "KB-ActionDown";

			Mouse *newMouse = &newInput->mouse;
			Mouse *oldMouse = &oldInput->mouse;
			*newMouse = {};
			for(uint32_t buttonIndex = 0; buttonIndex < FPL_ARRAYCOUNT(newMouse->buttons); ++buttonIndex) {
				newMouse->buttons[buttonIndex] = oldMouse->buttons[buttonIndex];
				newMouse->buttons[buttonIndex].halfTransitionCount = 0;
			}
			newMouse->pos = lastMousePos;

			// Remember previous gamepad connected states
			for(uint32_t controllerIndex = 1; controllerIndex < FPL_ARRAYCOUNT(newInput->controllers); ++controllerIndex) {
				Controller *newGamepadController = &newInput->controllers[controllerIndex];
				Controller *oldGamepadController = &oldInput->controllers[controllerIndex];
				newGamepadController->isConnected = oldGamepadController->isConnected;
				newGamepadController->isAnalog = oldGamepadController->isAnalog;
			}

			for(uint32_t buttonIndex = 0; buttonIndex < FPL_ARRAYCOUNT(newKeyboardController->buttons); ++buttonIndex) {
				newKeyboardController->buttons[buttonIndex].endedDown = oldKeyboardController->buttons[buttonIndex].endedDown;
			}

			newInput->deltaTime = (float)TargetDeltaTime;
			newInput->framesPerSeconds = (float)lastFramesPerSecond;
			newInput->frameIndex = frameIndex++;

			// Events
			ProcessEvents(newInput, oldInput, windowActiveType[0], lastMousePos);
			if(config.disableInactiveDetection) {
				newInput->isActive = (windowActiveType[0] & GameWindowActiveType::Minimized) != GameWindowActiveType::Minimized;
			} else {
				newInput->isActive = ((windowActiveType[0] & GameWindowActiveType::Minimized) != GameWindowActiveType::Minimized) && ((windowActiveType[0] & GameWindowActiveType::LostFocus) != GameWindowActiveType::LostFocus);
			}

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
				GameUpdateAndRender(gameMem, *newInput, alpha);
			} else {
				GameInput(gameMem, *newInput);
				while(frameAccumulator >= TargetDeltaTime) {
					GameUpdate(gameMem, *newInput);
					frameAccumulator -= TargetDeltaTime;
					++updateCount;
				}
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
				Input *tmp = newInput;
				newInput = oldInput;
				oldInput = tmp;
			}
		}

		if(config.hideMouseCursor) {
			fplSetWindowCursorEnabled(true);
		}

		GameRelease(gameMem);
	}

	fplStopAudio();

	AudioSystemShutdown(&audioSys);

	fmemFree(&gameMemoryBlock);
	fmemFree(&renderMemoryBlock);

	fglUnloadOpenGL();

	fplPlatformRelease();

	int result = wasError ? -1 : 0;
	return (result);
}

#endif // FINAL_GAMEPLATFORM_IMPLEMENTATION