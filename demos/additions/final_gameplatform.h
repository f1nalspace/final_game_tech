/*
Name:
	Final Game Platform

Description:
	One main function for bootstrapping a game.
	This contains all the platform specific code.

	This file is part of the final_framework.

Changelog:
	## 2025-12-30
	- Fixed compile warnings
	- Fixed lost/focus was not detected properly
	- Fixed input was not reset when focus was changed
	- Moved Input to GameMemory
	- Poll full keyboard states
	- Renamed internal functions and global variables

	## 2025-12-03
	- Introduce automatic logging using final_log.h

	## 2025-11-21
	- Changed: Reflect for API changes in final_game.h (Added Input argument to GameRender)
	- Fixed: Mouse button state was preserving the half-transition count which is incorrect

	## 2025-11-15
	- Added: Target fps to the GameConfiguration, to make the frames per second configurable
	- Fixed: Input controllers was properly preserved
	- Fixed: Active controller was always set, even when no buttons was pressed

	## 2022-01-23
	- Proper game timing is accumulated delta time method
	- Configurable vsync

	## 2019-01-31
	- Center window on center from nearest display

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-08-09:
	- Fixed WasPressed() was not working reliably (defaultControllerIndex issue)
    - Fixed controller swapping was not working properly

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_GAMEPLATFORM_H
#define FINAL_GAMEPLATFORM_H

#include "final_game.h"

typedef struct GameConfiguration {
	// Keyboard mappings
	const KeyboardButtonMappings *keyboardMappings;
	// Title of the game
	const char *title;
	// Name of the user folder name (if empty, the title is used instead)
	const char *userFolderName;
	// Name of the log file name (if empty, the default name is used)
	const char *logFileName;
	// Preferred sample rate in Hz
	uint32_t audioSampleRate;
	// Preferred number of channels
	uint32_t audioChannels;
	// Target game updates in Hz
	uint32_t targetHz;
	// Maximum render updates in Hz. If this is zero, the rendering happens on every frame - melting the GPU/CPU core.
	uint32_t maxRenderHz;
	// Preferred audio format
	fplAudioFormatType audioFormat;
	// Indicates whether to hide the mouse cursor or not
	bool hideMouseCursor;
	// Indicates that the detection of a inactive vs active window
	bool disableInactiveDetection;
	// Indicates that vertical sync is disabled or not
	bool disableVerticalSync;
	// Indicates whether text file logging is enabled or not
	bool enableLog;
} GameConfiguration;

fpl_extern int GameMain(const GameConfiguration *config, const int argumentCount, char **arguments);

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

#define FINAL_LOG_IMPLEMENTATION
#include "final_log.h"

fpl_internal_inline void InternalGamePlatformResetButtonState(ButtonState *button) {
    if(button->endedDown) {
        button->endedDown = false;
        button->halfTransitionCount = 1;
    } else {
        button->halfTransitionCount = 0;
    }
}

fpl_internal_inline void InternalGamePlatformResetController(Controller *controller) {
	const uint32_t count = fplArrayCount(controller->buttons);
	for (uint32_t buttonIndex = 0; buttonIndex < count; ++buttonIndex) {
		InternalGamePlatformResetButtonState(&controller->buttons[buttonIndex]);
	}
}

fpl_internal_inline void InternalGamePlatformResetKeyboard(Keyboard *keyboard) {
	const uint32_t count = fplArrayCount(keyboard->keys);
	for (uint32_t keyIndex = 0; keyIndex < count; ++keyIndex) {
		InternalGamePlatformResetButtonState(&keyboard->keys[keyIndex]);
	}
}

fpl_internal_inline void InternalGamePlatformResetMouse(Mouse *mouse) {
	const uint32_t count = fplArrayCount(mouse->buttons);
	for (uint32_t buttonIndex = 0; buttonIndex < count; ++buttonIndex) {
		InternalGamePlatformResetButtonState(&mouse->buttons[buttonIndex]);
	}
}

fpl_internal_inline void InternalGamePlatformResetInput(Input *input) {
	const uint32_t controllerCount = fplArrayCount(input->controllers);
	for (uint32_t controllerIndex = 0; controllerIndex < controllerCount; ++controllerIndex) {
		InternalGamePlatformResetController(&input->controllers[controllerIndex]);
	}
	InternalGamePlatformResetKeyboard(&input->tastatur);
	InternalGamePlatformResetMouse(&input->mouse);
}

fpl_internal_inline bool InternalGamePlatformAddKeyboardControllerButtonMapping(KeyboardButtonStates *states, KeyboardButtonMappings *mappings, const fplKey key, const ControllerButtonType buttonType) {
	if (mappings == fpl_null || mappings->count >= fplArrayCount(mappings->values) || buttonType < ControllerButtonType_First || buttonType > ControllerButtonType_Last) {
		return false;
	}
	uint32_t index = mappings->count++;
	mappings->values[index] = fplStructInit(KeyboardControllerButtonMapping, key, buttonType);
	states->mapped[buttonType] = true;
	return true;
}

fpl_internal_inline void InternalGamePlatformUpdateKeyboardButtonState(ButtonState *newState, const fpl_b32 isDown) {
	newState->endedDown = isDown;
	++newState->halfTransitionCount;
}

fpl_internal_inline bool InternalGamePlatformUpdateDigitalButtonState(const ButtonState *oldState, ButtonState *newState, const fpl_b32 isDown) {
	newState->endedDown = isDown;
	newState->halfTransitionCount = ((newState->endedDown == oldState->endedDown) ? 0 : 1);
	return(newState->endedDown == 1);
}

fpl_internal_inline void InternalGamePlatformPreserveButtonState(ButtonState *newState, const ButtonState *oldState) {
	newState->halfTransitionCount = 0;
	newState->endedDown = oldState->endedDown;
}

fpl_internal void InternalGamePlatformUpdateDefaultController(Input *currentInput, int newIndex) {
	if(newIndex != -1) {
		currentInput->defaultControllerIndex = newIndex;
	} else {
		currentInput->defaultControllerIndex = -1;
		for(int i = fplArrayCount(currentInput->controllers) - 1; i > 0; i--) {
			if(currentInput->controllers[i].isConnected) {
				currentInput->defaultControllerIndex = i;
				break;
			}
		}
	}
}

fpl_internal void InternalGamePlatformProcessEvents(const KeyboardButtonMappings *keyboardMappings, KeyboardButtonStates *keyboardButtonStates, Input *currentInput, Input *prevInput, GameWindowActiveType *windowActiveType, Vec2i *lastMousePos) {
	fplAssertPtr(keyboardMappings);
	fplAssertPtr(currentInput);
	fplAssertPtr(prevInput);
	fplAssertPtr(windowActiveType);
	fplAssertPtr(lastMousePos);
	Controller *newKeyboardController = &currentInput->keyboard;
	fplEvent event;
	while(fplPollEvent(&event)) {
		switch(event.type) {
			case fplEventType_Window:
			{
				switch(event.window.type) {
					case fplWindowEventType_GotFocus:
						*windowActiveType = GameWindowActiveType_GotFocus;
						break;
					case fplWindowEventType_Restored:
						*windowActiveType = GameWindowActiveType_Restored;
						break;
					case fplWindowEventType_Maximized:
						*windowActiveType = GameWindowActiveType_Maximized;
						break;
					case fplWindowEventType_LostFocus:
						*windowActiveType = GameWindowActiveType_LostFocus;
						break;
					case fplWindowEventType_Minimized:
						*windowActiveType = GameWindowActiveType_Minimized;
				        break;
				    default:
				        break;
				}
			} break;

			case fplEventType_Gamepad:
			{
				// @TODO(final): For now we just use the device index, but later it should be "added" to the controllers array and remembered somehow
				uint32_t controllerIndex = 1 + event.gamepad.deviceIndex;
				fplAssert(controllerIndex < fplArrayCount(currentInput->controllers));
				Controller *newController = &currentInput->controllers[controllerIndex];
				Controller *oldController = &prevInput->controllers[controllerIndex];
				switch(event.gamepad.type) {
					case fplGamepadEventType_Connected:
					{
						newController->isConnected = true;
						if(event.gamepad.state.isActive) {
							InternalGamePlatformUpdateDefaultController(currentInput,controllerIndex);
						}
					} break;
					case fplGamepadEventType_Disconnected:
					{
						newController->isConnected = false;
						InternalGamePlatformUpdateDefaultController(currentInput, -1);
					} break;
					case fplGamepadEventType_StateChanged:
					{
						fplGamepadState *padstate = &event.gamepad.state;
						assert(newController->isConnected);
						bool changed = false;
						if(F32Abs(padstate->leftStickX) > 0.0f || F32Abs(padstate->leftStickY) > 0.0f) {
							newController->isAnalog = true;
							newController->analogMovement.x = padstate->leftStickX;
							newController->analogMovement.y = padstate->leftStickY;
							changed = true;
						} else {
							newController->isAnalog = false;
							changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->moveDown, &newController->moveDown, padstate->dpadDown.isDown);
							changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->moveUp, &newController->moveUp, padstate->dpadUp.isDown);
							changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->moveLeft, &newController->moveLeft, padstate->dpadLeft.isDown);
							changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->moveRight, &newController->moveRight, padstate->dpadRight.isDown);
						}
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionDown, &newController->actionDown, padstate->actionA.isDown);
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionRight, &newController->actionRight, padstate->actionB.isDown);
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionLeft, &newController->actionLeft, padstate->actionX.isDown);
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionUp, &newController->actionUp, padstate->actionY.isDown);
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionBack, &newController->actionBack, padstate->back.isDown);
						changed |= InternalGamePlatformUpdateDigitalButtonState(&oldController->actionStart, &newController->actionStart, padstate->start.isDown);
						if (changed) {
							InternalGamePlatformUpdateDefaultController(currentInput, controllerIndex);
						}
					} break;

				    default:
				        break;
				}
			} break;

			case fplEventType_Mouse:
			{
				switch(event.mouse.type) {
					case fplMouseEventType_Move:
					{
						currentInput->mouse.pos = *lastMousePos = V2iInit(event.mouse.mouseX, event.mouse.mouseY);
					} break;

					case fplMouseEventType_Button:
					{
						bool isDown = (event.mouse.buttonState >= fplButtonState_Press);
						if(event.mouse.mouseButton == fplMouseButtonType_Left) {
							InternalGamePlatformUpdateKeyboardButtonState(&currentInput->mouse.left, isDown);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Right) {
							InternalGamePlatformUpdateKeyboardButtonState(&currentInput->mouse.right, isDown);
						} else if(event.mouse.mouseButton == fplMouseButtonType_Middle) {
							InternalGamePlatformUpdateKeyboardButtonState(&currentInput->mouse.middle, isDown);
						}
					} break;

					case fplMouseEventType_Wheel:
					{
						currentInput->mouse.wheelDelta = event.mouse.wheelDelta;
					} break;

				    default:
				        break;
				}
			} break;

			case fplEventType_Keyboard:
			{
				switch(event.keyboard.type) {
					case fplKeyboardEventType_Button:
					{
						bool isDown = event.keyboard.buttonState >= fplButtonState_Press;
						bool wasDown = event.keyboard.buttonState == fplButtonState_Release || event.keyboard.buttonState == fplButtonState_Repeat;
						if(isDown != wasDown) {
							if(!newKeyboardController->isConnected) {
								newKeyboardController->isConnected = true;
							}
							InternalGamePlatformUpdateDefaultController(currentInput, 0);
							for (uint32_t mappingIndex = 0; mappingIndex < keyboardMappings->count; ++mappingIndex) {
								const KeyboardControllerButtonMapping *mapping = keyboardMappings->values + mappingIndex;
								if (mapping->type < ControllerButtonType_First || mapping->type > ControllerButtonType_Last) {
									continue; // Invalid mapping
								}
								if (mapping->key == event.keyboard.mappedKey) {
									uint32_t buttonIndex = mapping->type - ControllerButtonType_First;
									keyboardButtonStates->changed[buttonIndex] |= true;
									if (event.keyboard.buttonState > keyboardButtonStates->states[buttonIndex]) {
										keyboardButtonStates->states[buttonIndex] = event.keyboard.buttonState;
									}
								}
							}
						}
						if(wasDown) {
							if(event.keyboard.mappedKey == fplKey_F) {
								bool wasFullscreen = fplIsWindowFullscreen();
								fplSetWindowFullscreenSize(!wasFullscreen, 0, 0, 0);
							}
						}
					} break;

				    default:
				        break;
				}
			} break;

			default:
				break;
		}
	}
}

fpl_internal uint32_t InternalGamePlatformAudioPlayback(const fplAudioFormat *outFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	AudioSystem *audioSys = (AudioSystem *)userData;
	uint32_t result = AudioSystemWriteFrames(audioSys, outputSamples, outFormat, frameCount, true);
	return(result);
}

fpl_internal void InternalGamePlatformSetupInputForFrame(KeyboardButtonStates *keyboardButtonStates, Input *oldInput, Input *newInput, const double targetDeltaTime, const double framesPerSecond, const double lastFrameTime) {
	newInput->fixedDeltaTime = (float)targetDeltaTime;
	newInput->dynamicFrameTime = (float)lastFrameTime;
	newInput->framesPerSeconds = (float)framesPerSecond;
	newInput->defaultControllerIndex = oldInput->defaultControllerIndex;
	newInput->isFirstUpdateOfFrame = true;

	const uint32_t controllerButtonCount = MAX_CONTROLLER_BUTTON_COUNT;

	// Preserve keyboard controller buttons
	Controller *oldKeyboardController = &oldInput->keyboard;
	Controller *newKeyboardController = &newInput->keyboard;
	fplClearStruct(newKeyboardController);
	newKeyboardController->isConnected = oldKeyboardController->isConnected;

	fplAssert(fplArrayCount(keyboardButtonStates->changed) == controllerButtonCount);
	fplAssert(fplArrayCount(keyboardButtonStates->mapped) == controllerButtonCount);

	for(uint32_t buttonIndex = 0; buttonIndex < controllerButtonCount; ++buttonIndex) {
		InternalGamePlatformPreserveButtonState(&newKeyboardController->buttons[buttonIndex], &oldKeyboardController->buttons[buttonIndex]);
	}

	// Clear keyboard states button pressed
	for(uint32_t buttonIndex = 0; buttonIndex < controllerButtonCount; ++buttonIndex) {
		keyboardButtonStates->changed[buttonIndex] = false;
		keyboardButtonStates->states[buttonIndex] = fplButtonState_Release;
	}

	// Preserve mouse buttons
	Mouse *newMouse = &newInput->mouse;
	Mouse *oldMouse = &oldInput->mouse;
	fplClearStruct(newMouse);
	newMouse->pos = oldMouse->pos;
	const uint32_t mouseButtonCount = fplArrayCount(newMouse->buttons);
	for(uint32_t buttonIndex = 0; buttonIndex < mouseButtonCount; ++buttonIndex) {
		InternalGamePlatformPreserveButtonState(&newMouse->buttons[buttonIndex], &oldMouse->buttons[buttonIndex]);
	}

	// Preserve keyboard controller buttons
	Keyboard *newKeyboard = &newInput->tastatur;
	Keyboard *oldKeyboard = &oldInput->tastatur;
	fplClearStruct(newKeyboard);
	const uint32_t maxKeyCount = fplArrayCount(newKeyboard->keys);
	for(uint32_t keyIndex = 0; keyIndex < maxKeyCount; ++keyIndex) {
		InternalGamePlatformPreserveButtonState(&newKeyboard->keys[keyIndex], &oldKeyboard->keys[keyIndex]);
	}

	// Preserve gamepad connection and button states
	const uint32_t maxControllerCount = fplArrayCount(newInput->controllers);
	for(uint32_t controllerIndex = 1; controllerIndex < maxControllerCount; ++controllerIndex) {
		Controller *newGamepadController = &newInput->controllers[controllerIndex];
		Controller *oldGamepadController = &oldInput->controllers[controllerIndex];
		fplClearStruct(newGamepadController);
		newGamepadController->isConnected = oldGamepadController->isConnected;
		newGamepadController->isAnalog = oldGamepadController->isAnalog;
		for(uint32_t buttonIndex = 0; buttonIndex < controllerButtonCount; ++buttonIndex) {
			InternalGamePlatformPreserveButtonState(&newGamepadController->buttons[buttonIndex], &oldGamepadController->buttons[buttonIndex]);
		}
	}
}

fpl_globalvar char g__GamePlatform__HomePathBuffer[FPL_MAX_PATH_LENGTH] = fplZeroInit;
fpl_globalvar char g__GamePlatform__FilePathBuffer[FPL_MAX_PATH_LENGTH] = fplZeroInit;

fpl_internal void InternalGamePlatformLoggingInitialize(const GameConfiguration *config) {
	fplSettings settings = fplZeroInit;

	const char *userFolderName;
	if (fplGetStringLength(config->userFolderName) > 0) {
		userFolderName = config->userFolderName;
	} else {
		userFolderName = config->title;
	}

	const char *logFileName;
	if (fplGetStringLength(config->logFileName) > 0) {
		logFileName = config->logFileName;
	} else {
		logFileName = "game.log";
	}

	// Log initialization, with short platform initialization so we can access several IO functions in FPL
	if (fplGetStringLength(userFolderName) > 0 && fplGetStringLength(logFileName) > 0 && config->enableLog) {
		if (fplPlatformInit(fplInitFlags_None, &settings)) {
			size_t homePathLen = fplGetHomePath(g__GamePlatform__HomePathBuffer, fplArrayCount(g__GamePlatform__HomePathBuffer));
			if (homePathLen > 0) {
				fplPathCombine(g__GamePlatform__FilePathBuffer, fplArrayCount(g__GamePlatform__FilePathBuffer), 2, g__GamePlatform__HomePathBuffer, userFolderName);
				if (!fplDirectoryExists(g__GamePlatform__FilePathBuffer))
					fplDirectoriesCreate(g__GamePlatform__FilePathBuffer);
				fplPathCombine(g__GamePlatform__FilePathBuffer, fplArrayCount(g__GamePlatform__FilePathBuffer), 3, g__GamePlatform__HomePathBuffer, userFolderName, logFileName);
			}

			LogInit(g__GamePlatform__FilePathBuffer);

			fplPlatformRelease();
		}
	}
}

#define GAMEPLATFORM_LOGPREFIX "[ PLATFORM ] "

typedef struct {
	Input inputs[2];
	fplKeyboardState keyboardState;
	KeyboardButtonMappings keyboardMappings;
	KeyboardButtonStates keyboardButtonStates;
	GameMemory gameMemory;
} GamePlatformState;

#define CONTROLLER_BUTTON_TYPE_COUNT FPL__ENUM_COUNT(ControllerButtonType_First, ControllerButtonType_Last)

fpl_globalvar const char *g__GamePlatform__ControllerButtonTypeNameTable[] = {
	FPL__ENUM_NAME("MoveUp", ControllerButtonType_MoveUp),
	FPL__ENUM_NAME("MoveDown", ControllerButtonType_MoveDown),
	FPL__ENUM_NAME("MoveLeft", ControllerButtonType_MoveLeft),
	FPL__ENUM_NAME("MoveRight", ControllerButtonType_MoveRight),
	FPL__ENUM_NAME("ActionUp", ControllerButtonType_ActionUp),
	FPL__ENUM_NAME("ActionDown", ControllerButtonType_ActionDown),
	FPL__ENUM_NAME("ActionLeft", ControllerButtonType_ActionLeft),
	FPL__ENUM_NAME("ActionRight", ControllerButtonType_ActionRight),
	FPL__ENUM_NAME("ActionBack", ControllerButtonType_ActionBack),
	FPL__ENUM_NAME("ActionStart", ControllerButtonType_ActionStart),
};

fplStaticAssert(ControllerButtonType_MoveUp == ControllerButtonType_First);
fplStaticAssert(ControllerButtonType_ActionStart == ControllerButtonType_Last);

fplStaticAssert(CONTROLLER_BUTTON_TYPE_COUNT == fplArrayCount(g__GamePlatform__ControllerButtonTypeNameTable));

fpl_internal const char *InternalGamePlatformGetControllerButtonTypeName(const ControllerButtonType type) {
	uint32_t index = FPL__ENUM_VALUE_TO_ARRAY_INDEX(type, ControllerButtonType_First, ControllerButtonType_Last);
	const char *result = g__GamePlatform__ControllerButtonTypeNameTable[index];
	return(result);
}

fpl_internal void GameMainShutdown(const GameConfiguration *config, GameMemory *gameMem, AudioSystem *audioSys, fmemMemoryBlock *gameMemoryBlock, fmemMemoryBlock *renderMemoryBlock) {
	LogWriteRaw("======================================================================");
	LogWrite(LogLevel_Info, "Shutdown Game '%s'", config->title);
	LogWriteRaw("======================================================================");

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Stop Audio Playback");
	fplStopAudio();

	if (gameMem != fpl_null) {
		LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Release Game");
		GameRelease(gameMem);
	}

	if (audioSys != fpl_null) {
		LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Shutdown Audio System");
		AudioSystemShutdown(audioSys);
	}

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Free Memory Blocks");
	fmemFree(gameMemoryBlock);
	fmemFree(renderMemoryBlock);

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Unload OpenGL");
	fglUnloadOpenGL();

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Release Platform Layer");
	fplPlatformRelease();

	LogShutdown();
}

fpl_extern int GameMain(const GameConfiguration *config, const int argumentCount, char **arguments) {
	if(config == fpl_null) {
		return -1;
	}

	InternalGamePlatformLoggingInitialize(config);

	LogWriteLineBreak();
	LogWriteRaw("======================================================================");
	LogWrite(LogLevel_Info, "Startup Game '%s'", config->title);
	LogWriteRaw("======================================================================");

	LogWrite(LogLevel_Debug, GAMEPLATFORM_LOGPREFIX "Detect Platform Configuration");

	fplSettings settings = fplZeroInit;
	fplSetDefaultSettings(&settings);
	settings.locale.isCultureInvariant = true;
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings.video.isVSync = !config->disableVerticalSync;
	if (config->audioSampleRate > 0) {
		settings.audio.targetFormat.sampleRate = config->audioSampleRate;
	}
	if (config->audioFormat != fplAudioFormatType_None) {
		settings.audio.targetFormat.type = config->audioFormat;
	}
	if (config->audioChannels > 0) {
		settings.audio.targetFormat.channels = config->audioChannels;
	}
	fplCopyString(config->title, settings.window.title, fplArrayCount(settings.window.title));

	fplInitFlags initFlags = fplInitFlags_All;
	initFlags &= ~fplInitFlags_Console;

	const char *platformName = fplGetPlatformName(fplGetPlatformType());
	const char *archName = fplCPUGetArchName(fplCPUGetArchitecture());
	fplMemoryInfos memInfos = fplZeroInit;
	fplMemoryGetUsage(&memInfos);

	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Platform: %s", platformName);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Architecture: %s", archName);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Total Physical Memory: %zu bytes", memInfos.totalPhysicalSize);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Free Physical Memory: %zu bytes", memInfos.freePhysicalSize);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Title: %s", config->title);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Video Backend: %s", fplGetVideoBackendName(settings.video.backend));
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Video VSync: %s", settings.video.isVSync ? "On" : "Off");
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Audio format: [SampleRate: %u, Channels: %u, Type: %s]", settings.audio.targetFormat.sampleRate, settings.audio.targetFormat.channels, fplGetAudioFormatName(settings.audio.targetFormat.type));

	if (config->keyboardMappings == fpl_null || !config->keyboardMappings->isCustom) {
		LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Keyboard Button Mappings: Default Keyboard Mapping");
	} else if (config->keyboardMappings->count == 0) {
		LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- Keyboard Button Mappings: Disabled");
	} else {
		const uint32_t mappingCount = fplMin(config->keyboardMappings->count, MAX_KEYBOARD_CONTROLLER_BUTTON_MAPPING_COUNT);
		LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- %u Keyboard Button Mappings:", mappingCount);
		for (uint32_t mappingIndex = 0; mappingIndex < mappingCount; ++mappingIndex) {
			const char *keyName = fplKeyGetName(config->keyboardMappings->values[mappingIndex].key);
			const char *typeName = InternalGamePlatformGetControllerButtonTypeName(config->keyboardMappings->values[mappingIndex].type);
			LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "[%u] Key '%s' to '%s'", mappingIndex, keyName, typeName);
		}
	}

	fmemMemoryBlock gameMemoryBlock;
	fmemMemoryBlock renderMemoryBlock;
	fplAudioFormat targetAudioFormat;

	GamePlatformState *gamePlatformState = fpl_null;

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Initialize Platform Layer");
	if(!fplPlatformInit(initFlags, &settings)) {
		const char *lastError = fplGetLastError();
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to initialize Platform Layer -> %s", lastError);
		GameMainShutdown(config, fpl_null, fpl_null, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	//
	// Center window on nearest display from cursor
	//
	int32_t cursorX, cursorY;
	if (fplQueryCursorPosition(&cursorX, &cursorY)) {
		fplDisplayInfo display = fplZeroInit;
		fplWindowSize winSize = fplZeroInit;
		if (fplGetWindowSize(&winSize) && fplGetDisplayFromPosition(cursorX, cursorY, &display)) {
			int32_t newX = display.virtualPosition.left + (display.virtualSize.width - winSize.width) / 2;
			int32_t newY = display.virtualPosition.top + (display.virtualSize.height - winSize.height) / 2;
			fplSetWindowPosition(newX, newY);
		}
	}

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Load OpenGL Library");
	if(!fglLoadOpenGL(true)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to load OpenGL library!");
		GameMainShutdown(config, fpl_null, fpl_null, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	const GLubyte *glversion = glGetString(GL_VERSION);
	const GLubyte *glvendor = glGetString(GL_VENDOR);
	const GLubyte *glrenderer = glGetString(GL_RENDERER);
	const GLubyte *glextensions = glGetString(GL_EXTENSIONS);

	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- OpenGL Version: %s", glversion);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- OpenGL Vendor: %s", glvendor);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- OpenGL Renderer: %s", glrenderer);
	LogWrite(LogLevel_Verbose, GAMEPLATFORM_LOGPREFIX "- OpenGL Extensions: %s", glextensions);

	const size_t gameMemoryBlockSize = FMEM_MEGABYTES(128);

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Allocate game memory block with size %zu bytes", gameMemoryBlockSize);
	if(!fmemInit(&gameMemoryBlock, fmemType_Growable, gameMemoryBlockSize, 0)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to allocate game memory block with size %zu!", gameMemoryBlockSize);
		GameMainShutdown(config, fpl_null, fpl_null, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	const size_t renderMemoryBlockSize = FMEM_MEGABYTES(32);
	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Allocate render memory block with size %zu bytes ]", renderMemoryBlockSize);
	if(!fmemInit(&renderMemoryBlock, fmemType_Growable, renderMemoryBlockSize, 0)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to allocate render memory block with size %zu!", renderMemoryBlockSize);
		GameMainShutdown(config, fpl_null, fpl_null, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	const size_t audioSystemSize = sizeof(AudioSystem);
	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Aquire memory for audio system with size %zu bytes", audioSystemSize);
	AudioSystem *audioSys = fmemPushStruct(&gameMemoryBlock, AudioSystem, fmemPushFlags_Clear);
	if (audioSys == fpl_null) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Insufficient memory for audio system, capacity is '%zu bytes', used is '%zu bytes', required is '%zu bytes'!", gameMemoryBlock.size, gameMemoryBlock.used, audioSystemSize);
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	const size_t gamePlatformStateSize = sizeof(GamePlatformState);
	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Aquire memory for game platform state with size %zu bytes", gamePlatformStateSize);
	gamePlatformState = fmemPushStruct(&gameMemoryBlock, GamePlatformState, fmemPushFlags_Clear);
	if (audioSys == fpl_null) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Insufficient memory for game platform state, capacity is '%zu bytes', used is '%zu bytes', required is '%zu bytes'!", gameMemoryBlock.size, gameMemoryBlock.used, gamePlatformStateSize);
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Query Audio Hardware Format");
	if (!fplGetAudioHardwareFormat(&targetAudioFormat)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to query Audio Hardware Format!");
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Initialize Audio System with target format: SampleRate: %u, Channels: %u, Type: %s", targetAudioFormat.sampleRate, targetAudioFormat.channels, fplGetAudioFormatName(targetAudioFormat.type));
	if(!AudioSystemInit(audioSys, &targetAudioFormat)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to initialize Audio System with target format 'SampleRate: %u, Channels: %u, Type: %s'!", targetAudioFormat.sampleRate, targetAudioFormat.channels, fplGetAudioFormatName(targetAudioFormat.type));
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	size_t renderStateSize = sizeof(RenderState);
	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Aquire memory for render state with size %zu bytes", renderStateSize);
	RenderState *renderState = fmemPushStruct(&gameMemoryBlock, RenderState, fmemPushFlags_Clear);
	if (renderState == fpl_null) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Insufficient memory for render state, capacity is '%zu bytes', used is '%zu bytes', required is '%zu bytes'!", gameMemoryBlock.size, gameMemoryBlock.used, renderStateSize);
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	RenderInit(renderState, renderMemoryBlock);
	InitOpenGLRenderer();

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Start Audio Playback");
	fplSetAudioClientReadCallback(InternalGamePlatformAudioPlayback, audioSys);
	fplAudioResultType playAudioResult = fplPlayAudio();
	if(playAudioResult != fplAudioResultType_Success) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Failed to start Audio Playback -> %s!", fplGetAudioResultName(playAudioResult));
		GameMainShutdown(config, fpl_null, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	GameMemory *gameMem = &gamePlatformState->gameMemory;
	gameMem->render = renderState;
	gameMem->memory = &gameMemoryBlock;
	gameMem->audio = audioSys;

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Initialize Game");
	if(!GameInit(gameMem, argumentCount, arguments)) {
		LogWrite(LogLevel_Fatal, GAMEPLATFORM_LOGPREFIX "Game failed to initialize!");
		GameMainShutdown(config, gameMem, audioSys, &gameMemoryBlock, &renderMemoryBlock);
		return -1;
	}

	const uint32_t targetFramesHz = config->targetHz > 0 ? config->targetHz : 60;
	const uint32_t maxRenderFramesHz = config->maxRenderHz;

	const double targetDeltaTime = 1.0 / (double)targetFramesHz;
	const double maxRenderTime = maxRenderFramesHz > 0 ? 1.0 / (double)maxRenderFramesHz : 0.0;

	if(config->hideMouseCursor) {
		fplSetWindowCursorEnabled(false);
	}

	Input *newInput = &gamePlatformState->inputs[0];
	Input *oldInput = &gamePlatformState->inputs[1];
	Vec2i lastMousePos = V2iInit(-1, -1);
	GameWindowActiveType windowActiveType[2] = { GameWindowActiveType_None, GameWindowActiveType_None };
	newInput->defaultControllerIndex = oldInput->defaultControllerIndex = -1;

	uint32_t frameCount = 0;
	uint32_t updateCount = 0;

	fplTimestamp lastTime = fplTimestampQuery();
	double frameAccumulator = 0.0;
	double totalTime = 0.0;
	double lastFrameTime = targetDeltaTime;

	uint64_t lastFPSTime = fplMillisecondsQuery();
	double framesPerSecond = 0.0;
	int frameIndex = 0;

	KeyboardButtonMappings *keyboardMappings = &gamePlatformState->keyboardMappings;
	KeyboardButtonStates *keyboardButtonStates = &gamePlatformState->keyboardButtonStates;

	if (config->keyboardMappings != fpl_null && config->keyboardMappings->isCustom) {
		fplMemoryCopy(config->keyboardMappings, sizeof(KeyboardButtonMappings), keyboardMappings);
	} else {
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_A, ControllerButtonType_MoveLeft);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Left, ControllerButtonType_MoveLeft);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_D, ControllerButtonType_MoveRight);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Right, ControllerButtonType_MoveRight);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_W, ControllerButtonType_MoveUp);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Up, ControllerButtonType_MoveUp);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_S, ControllerButtonType_MoveDown);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Down, ControllerButtonType_MoveDown);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Space, ControllerButtonType_ActionDown);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Return, ControllerButtonType_ActionStart);
		InternalGamePlatformAddKeyboardControllerButtonMapping(keyboardButtonStates, keyboardMappings, fplKey_Escape, ControllerButtonType_ActionBack);
	}

	LogWrite(LogLevel_Info, GAMEPLATFORM_LOGPREFIX "Main Loop");
	while(!IsGameExiting(gameMem) && fplWindowUpdate()) {
		// Get window size
		fplWindowSize winArea;
		if(fplGetWindowSize(&winArea)) {
			newInput->windowSize.x = winArea.width;
			newInput->windowSize.y = winArea.height;
		}

		// Setup input (Clear new and preserve important states)
		InternalGamePlatformSetupInputForFrame(keyboardButtonStates, oldInput, newInput, targetDeltaTime, framesPerSecond, lastFrameTime);
		newInput->frameIndex = frameIndex++;

		// Events
		windowActiveType[1] = windowActiveType[0];
		InternalGamePlatformProcessEvents(keyboardMappings, keyboardButtonStates, newInput, oldInput, &windowActiveType[0], &lastMousePos);

		// Keyboard keys
		Keyboard *oldKeyboard = &oldInput->tastatur;
		Keyboard *newKeyboard = &newInput->tastatur;
		fplKeyboardState *keyboardState = &gamePlatformState->keyboardState;

		const uint32_t keyCount = fplArrayCount(keyboardState->buttonStatesMapped);
		fplPollKeyboardState(keyboardState);
		fplAssert(fplArrayCount(newKeyboard->keys) == keyCount);
		for (uint32_t keyIndex = 0; keyIndex < keyCount; ++keyIndex) {
			fplButtonState buttonState = keyboardState->buttonStatesMapped[keyIndex];
			fpl_b32 isDown = buttonState != fplButtonState_Release ? 1 : 0;
			if (newKeyboard->keys[keyIndex].endedDown != isDown) {
				InternalGamePlatformUpdateKeyboardButtonState(&newKeyboard->keys[keyIndex], isDown);
			}
		}
		
		// Keyboard controller buttons from all mappings
		for (uint32_t buttonTypeIndex = 0; buttonTypeIndex < MAX_CONTROLLER_BUTTON_TYPE_COUNT; ++buttonTypeIndex) {
			if (keyboardButtonStates->mapped[buttonTypeIndex] && keyboardButtonStates->changed[buttonTypeIndex]) {
				ButtonState *button = &newInput->keyboard.buttons[buttonTypeIndex];
				bool isDown = keyboardButtonStates->states[buttonTypeIndex] > fplButtonState_Release;
				InternalGamePlatformUpdateKeyboardButtonState(button, isDown);
			}
		}

#if 0
		// Logging of input change
		for(uint32_t buttonIndex = 0; buttonIndex < fplArrayCount(newKeyboardController->buttons); ++buttonIndex) {
			ButtonState newState = newKeyboardController->buttons[buttonIndex];
			ButtonState oldState = oldKeyboardController->buttons[buttonIndex];
			if ((newState.endedDown != oldState.endedDown) ||
				(newState.halfTransitionCount != oldState.halfTransitionCount)) {
				bool wasPressed = WasPressed(newState);
				fplDebugFormatOut("Button [%d] changed, down: [%u/%u] transitions: [%d/%d] => %s\n", buttonIndex, newState.endedDown, oldState.endedDown, newState.halfTransitionCount, oldState.halfTransitionCount, (wasPressed ? "yes" : "no"));
			}
		}
#endif
			
		if(config->disableInactiveDetection) {
			newInput->isActive = (windowActiveType[0] & GameWindowActiveType_Minimized) != GameWindowActiveType_Minimized;
		} else {
			newInput->isActive = ((windowActiveType[0] & GameWindowActiveType_Minimized) != GameWindowActiveType_Minimized) && ((windowActiveType[0] & GameWindowActiveType_LostFocus) != GameWindowActiveType_LostFocus);
		}

		//
		// If activation toggled, reset timings and new-input
		//
		if(windowActiveType[0] != windowActiveType[1]) {
			lastTime = fplTimestampQuery();
			frameAccumulator = 0.0;
			framesPerSecond = 0.0f;
			lastFPSTime = fplMillisecondsQuery();
			updateCount = frameCount = 0;
			InternalGamePlatformResetInput(newInput);
		}

		//
		// Game Input once per frame
		//
		GameInput(gameMem, newInput);

		//
		// Compute frame time once and advance accumulator
		//
		fplTimestamp currTime = fplTimestampQuery();
		double frameTime = fplTimestampElapsed(lastTime, currTime);
		lastTime = currTime;
		if (frameTime > 0.25) frameTime = 0.25;
		frameAccumulator += frameTime;
		framesPerSecond = frameTime > 0 ? 1.0 / frameTime : 0;
		lastFrameTime = frameTime;

		//
		// Game update accumulator loop (Allow button edge events only on the first tick of this render frame)
		//
		int ticksThisFrame = 0;
		while (frameAccumulator >= targetDeltaTime) {
			newInput->isFirstUpdateOfFrame = (ticksThisFrame == 0);
			GameUpdate(gameMem, newInput);
			frameAccumulator -= targetDeltaTime;
			totalTime += targetDeltaTime;
			++updateCount;
			++ticksThisFrame;
		}

		//
		// Game Render without any interpolation
		//
		const float alphaRaw = (float)(frameAccumulator / targetDeltaTime);
		const float alpha = F32Clamp(alphaRaw, 0.0f, 1.0f);
		RenderReset(renderState);
		GameRender(gameMem, newInput, alpha);
		RenderWithOpenGL(renderState);
		fplVideoFlip();
		++frameCount;

		//
		// FPS-Timer
		//
		if((fplMillisecondsQuery() - lastFPSTime) >= 1000) {
#if 0
			fplDebugFormatOut("Fps: %d, Ups: %d\n", frameCount, updateCount);
#endif
			lastFPSTime = fplMillisecondsQuery();
			frameCount = 0;
			updateCount = 0;
		}

		// Swap input
		{
			Input *tmp = newInput;
			newInput = oldInput;
			oldInput = tmp;
		}

		// Throttle if vsync is disabled and there is a limit of max frames
		if (config->disableVerticalSync && maxRenderTime > 0.0) {
			if (frameTime < maxRenderTime) {
				double sleepSec = maxRenderTime - frameTime;
				uint32_t sleepMS = (uint32_t)(sleepSec * 1000.0);
				if (sleepMS > 0) {
					// TODO(final): Use a better approach!
					fplThreadSleep(sleepMS);
				}
			}
		}
	}

	if(config->hideMouseCursor) {
		fplSetWindowCursorEnabled(true);
	}

	GameMainShutdown(config, gameMem, audioSys, &gameMemoryBlock, &renderMemoryBlock);

	return 0;
}

#endif // FINAL_GAMEPLATFORM_IMPLEMENTATION