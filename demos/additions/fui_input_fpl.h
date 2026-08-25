/***
fui_input_fpl.h

--- About ---

The input bridge between final_platform_layer.h and final_ui.h: one fuiInput, filled once per frame.

final_ui.h deliberately knows nothing about a platform. It takes a fuiInput and asks the host to fill it,
which is about sixty lines of real work plus a key table. This header is that work, done once, so a demo
does not carry its own copy of it.

The held state comes from POLLING rather than from the event queue, which is the lesson a game learns the
hard way: a key pressed and released inside one frame latches as stuck-down when its state is accumulated
from events. The cost is that such a tap is not seen at all - two half transitions in one frame is
something only an event stream can express - and for an interface that is the right trade. What polling
cannot give at all is drained from the queue instead: typed characters, the wheel, and the focus.

--- Getting started ---

- Include this AFTER final_platform_layer.h and final_ui.h
- Define FUI_INPUT_FPL_IMPLEMENTATION in ONE translation unit before including it
- Call fuiFplInputInit once, then Pump and Build once per frame before fuiBeginFrame

--- Usage ---

	fuiFplInput bridge;
	fuiFplInputInit(&bridge);
	while(fplWindowUpdate()) {
		fuiFplInputPumpEvents(&bridge);
		fuiFplInputBuild(&bridge);
		fuiBeginFrame(&context, &bridge.input, FUI_PASS_BOTH);
		...
		fuiEndFrame(&context);
	}

--- License ---

MIT License, Copyright (c) 2017-2026 Torsten Spaete
***/

#ifndef FUI_INPUT_FPL_INCLUDE_H
#define FUI_INPUT_FPL_INCLUDE_H

#include <final_platform_layer.h>
#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @struct fuiFplInput
* @brief One frame of input, plus the previous frame's held state that the edges are worked out against.
* @note Only `input` and `rightPressedThisFrame` are meant to be read, everything else is bookkeeping.
*/
typedef struct fuiFplInput {
	//! The finished input, ready to hand to @ref fuiBeginFrame
	fuiInput input;
	//! Whether each key was down on the PREVIOUS frame, which is what a half transition is measured against
	bool keyWasDown[FUI_KEY_COUNT];
	//! Whether each mouse button was down on the previous frame
	bool mouseWasDown[FUI_MOUSE_BUTTON_COUNT];
	//! Whether the window had the focus, which only the event queue can answer
	bool windowHasFocus;
	//! How far the wheel turned this frame, accumulated from the queue
	float wheelThisFrame;
	//! Codepoints typed this frame, accumulated from the queue
	uint32_t typedCodePoints[FUI_MAX_TEXT_INPUT];
	//! How many entries of typedCodePoints are filled
	int32_t typedCount;
	//! Whether the right button went down this frame, which is what a context menu opens on
	bool rightPressedThisFrame;
	//! When the previous frame was built, which is what the delta time is measured from
	fplTimestamp lastTimestamp;
} fuiFplInput;

/**
* @brief Maps one FPL key to the key final_ui.h knows it as.
* @param[in] key The FPL key.
* @return Returns the matching @ref fuiKey, or FUI_KEY_NONE for a key the interface has no use for.
*/
fui_api fuiKey fuiFplMapKey(const fplKey key);

/**
* @brief Prepares a bridge for its first frame.
* @param[out] bridge Reference to the bridge @ref fuiFplInput to initialize.
*/
fui_api void fuiFplInputInit(fuiFplInput *bridge);

/**
* @brief Drains the event queue for the things polling cannot give: typed characters, the wheel, and focus.
* @param[in,out] bridge Reference to the bridge @ref fuiFplInput.
* @note Call this once per frame, BEFORE @ref fuiFplInputBuild.
*/
fui_api void fuiFplInputPumpEvents(fuiFplInput *bridge);

/**
* @brief Fills this frame's fuiInput from the polled device state and the drained events.
* @param[in,out] bridge Reference to the bridge @ref fuiFplInput.
*/
fui_api void fuiFplInputBuild(fuiFplInput *bridge);

/**
* @brief Reads the system clipboard, shaped as the @ref fuiPlatform callback final_ui.h asks for.
* @param[in] userData Ignored, FPL's clipboard is global.
* @param[out] destination Receives the text.
* @param[in] maxDestinationLength Capacity of destination in bytes, including the terminator.
* @return Returns true when there was text to read.
*/
fui_api bool fuiFplGetClipboardText(void *userData, char *destination, uint32_t maxDestinationLength);

/**
* @brief Writes the system clipboard, shaped as the @ref fuiPlatform callback final_ui.h asks for.
* @param[in] userData Ignored, FPL's clipboard is global.
* @param[in] text The text to put on the clipboard.
* @return Returns true when the clipboard took it.
*/
fui_api bool fuiFplSetClipboardText(void *userData, const char *text);

#ifdef __cplusplus
}
#endif

#endif // FUI_INPUT_FPL_INCLUDE_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
#if defined(FUI_INPUT_FPL_IMPLEMENTATION) && !defined(FUI_INPUT_FPL_IMPLEMENTED)
#define FUI_INPUT_FPL_IMPLEMENTED

fui_api fuiKey fuiFplMapKey(const fplKey key) {
	if(key >= fplKey_A && key <= fplKey_Z) {
		return (fuiKey)((int)FUI_KEY_A + ((int)key - (int)fplKey_A));
	}
	if(key >= fplKey_0 && key <= fplKey_9) {
		return (fuiKey)((int)FUI_KEY_0 + ((int)key - (int)fplKey_0));
	}
	if(key >= fplKey_F1 && key <= fplKey_F12) {
		return (fuiKey)((int)FUI_KEY_F1 + ((int)key - (int)fplKey_F1));
	}
	switch(key) {
		case fplKey_Backspace: return FUI_KEY_BACKSPACE;
		case fplKey_Tab: return FUI_KEY_TAB;
		case fplKey_Return: return FUI_KEY_RETURN;
		case fplKey_Escape: return FUI_KEY_ESCAPE;
		case fplKey_Space: return FUI_KEY_SPACE;
		case fplKey_PageUp: return FUI_KEY_PAGE_UP;
		case fplKey_PageDown: return FUI_KEY_PAGE_DOWN;
		case fplKey_End: return FUI_KEY_END;
		case fplKey_Home: return FUI_KEY_HOME;
		case fplKey_Left: return FUI_KEY_LEFT;
		case fplKey_Up: return FUI_KEY_UP;
		case fplKey_Right: return FUI_KEY_RIGHT;
		case fplKey_Down: return FUI_KEY_DOWN;
		case fplKey_Insert: return FUI_KEY_INSERT;
		case fplKey_Delete: return FUI_KEY_DELETE;
		case fplKey_LeftShift: return FUI_KEY_LEFT_SHIFT;
		case fplKey_RightShift: return FUI_KEY_RIGHT_SHIFT;
		case fplKey_LeftControl: return FUI_KEY_LEFT_CONTROL;
		case fplKey_RightControl: return FUI_KEY_RIGHT_CONTROL;
		case fplKey_LeftAlt: return FUI_KEY_LEFT_ALT;
		case fplKey_RightAlt: return FUI_KEY_RIGHT_ALT;
		// X11 reports the side, Win32 can report the generic one. Both have to arrive, or a shortcut works
		// on one platform and silently does not on the other.
		case fplKey_Shift: return FUI_KEY_LEFT_SHIFT;
		case fplKey_Control: return FUI_KEY_LEFT_CONTROL;
		case fplKey_Alt: return FUI_KEY_LEFT_ALT;
		default: return FUI_KEY_NONE;
	}
}

fui_api void fuiFplInputInit(fuiFplInput *bridge) {
	fplClearStruct(bridge);
	bridge->input = fuiZeroInput();
	bridge->windowHasFocus = true;
	bridge->lastTimestamp = fplTimestampQuery();
}

fui_api void fuiFplInputPumpEvents(fuiFplInput *bridge) {
	bridge->wheelThisFrame = 0.0f;
	bridge->typedCount = 0;
	bridge->rightPressedThisFrame = false;

	fplEvent event;
	while(fplPollEvent(&event)) {
		switch(event.type) {
			case fplEventType_Window:
			{
				if(event.window.type == fplWindowEventType_GotFocus) {
					bridge->windowHasFocus = true;
				} else if(event.window.type == fplWindowEventType_LostFocus) {
					bridge->windowHasFocus = false;
				}
			} break;

			case fplEventType_Keyboard:
			{
				if(event.keyboard.type == fplKeyboardEventType_Input) {
					// The FULL codepoint, not a byte. final_ui.h takes codepoints precisely so that a
					// platform layer narrowing this to a char cannot corrupt anything above U+00FF.
					uint32_t codePoint = (uint32_t)event.keyboard.keyCode;
					bool isPrintable = (codePoint >= 32u) && (codePoint != 127u);
					if(isPrintable && bridge->typedCount < (int32_t)FUI_MAX_TEXT_INPUT) {
						bridge->typedCodePoints[bridge->typedCount++] = codePoint;
					}
				}
			} break;

			case fplEventType_Mouse:
			{
				if(event.mouse.type == fplMouseEventType_Wheel) {
					bridge->wheelThisFrame += event.mouse.wheelDelta;
				} else if(event.mouse.type == fplMouseEventType_Button) {
					bool isRightPress = (event.mouse.mouseButton == fplMouseButtonType_Right) && (event.mouse.buttonState != fplButtonState_Release);
					if(isRightPress) {
						bridge->rightPressedThisFrame = true;
					}
				}
			} break;

			default:
				break;
		}
	}
}

fui_api void fuiFplInputBuild(fuiFplInput *bridge) {
	fplWindowSize windowSize = fplZeroInit;
	fplGetWindowSize(&windowSize);

	fplTimestamp now = fplTimestampQuery();
	double elapsedSeconds = fplTimestampElapsed(bridge->lastTimestamp, now);
	bridge->lastTimestamp = now;

	fuiInput *input = &bridge->input;
	input->windowSize = fuiV2i((int32_t)windowSize.width, (int32_t)windowSize.height);
	input->deltaTime = (float)elapsedSeconds;
	input->isActive = bridge->windowHasFocus;
	input->mouseWheelDelta = bridge->wheelThisFrame;

	fplMouseState mouseState = fplZeroInit;
	fplPollMouseState(&mouseState);
	input->mousePosition = fuiV2((float)mouseState.x, (float)mouseState.y);

	// FPL orders its buttons left, right, middle; final_ui.h orders them left, middle, right.
	const fplMouseButtonType fplButtonForFuiButton[FUI_MOUSE_BUTTON_COUNT] = {
		fplMouseButtonType_Left, fplMouseButtonType_Middle, fplMouseButtonType_Right,
	};
	for(int32_t buttonIndex = 0; buttonIndex < FUI_MOUSE_BUTTON_COUNT; ++buttonIndex) {
		bool isDown = mouseState.buttonStates[fplButtonForFuiButton[buttonIndex]] != fplButtonState_Release;
		bool wasDown = bridge->mouseWasDown[buttonIndex];
		input->mouseButtons[buttonIndex].endedDown = isDown;
		input->mouseButtons[buttonIndex].halfTransitionCount = (isDown != wasDown) ? 1 : 0;
		bridge->mouseWasDown[buttonIndex] = isDown;
	}

	fplKeyboardState keyboardState = fplZeroInit;
	fplPollKeyboardState(&keyboardState);
	for(int32_t keyIndex = 0; keyIndex < (int32_t)FUI_KEY_COUNT; ++keyIndex) {
		input->keys[keyIndex].endedDown = false;
		input->keys[keyIndex].halfTransitionCount = 0;
	}
	for(uint32_t rawKey = 0; rawKey < fplArrayCount(keyboardState.buttonStatesMapped); ++rawKey) {
		fuiKey mappedKey = fuiFplMapKey((fplKey)rawKey);
		if(mappedKey == FUI_KEY_NONE) {
			continue;
		}
		bool isDown = keyboardState.buttonStatesMapped[rawKey] != fplButtonState_Release;
		// Several raw keys can land on one fui key, so a key already reported down stays down.
		if(isDown) {
			input->keys[mappedKey].endedDown = true;
		}
	}
	for(int32_t keyIndex = 0; keyIndex < (int32_t)FUI_KEY_COUNT; ++keyIndex) {
		bool isDown = input->keys[keyIndex].endedDown;
		bool wasDown = bridge->keyWasDown[keyIndex];
		input->keys[keyIndex].halfTransitionCount = (isDown != wasDown) ? 1 : 0;
		bridge->keyWasDown[keyIndex] = isDown;
	}

	for(int32_t typedIndex = 0; typedIndex < bridge->typedCount; ++typedIndex) {
		input->textInput[typedIndex] = bridge->typedCodePoints[typedIndex];
	}
	input->textInputLength = bridge->typedCount;
}

fui_api bool fuiFplGetClipboardText(void *userData, char *destination, uint32_t maxDestinationLength) {
	(void)userData;
	return fplGetClipboardText(destination, maxDestinationLength);
}

fui_api bool fuiFplSetClipboardText(void *userData, const char *text) {
	(void)userData;
	return fplSetClipboardText(text);
}

#endif // FUI_INPUT_FPL_IMPLEMENTATION
