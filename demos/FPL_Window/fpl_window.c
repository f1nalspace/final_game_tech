/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Simple Window

Description:
	Simple demo that shows to initialize the platform with a window and how to handle the events.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2025-03-25
	- Transitioned to a simple window demo

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2025 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_VULKAN
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

int main(int argc, char **args) {
	// Create default settings and setup window title
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo - Simple Window", settings.window.title, fplArrayCount(settings.window.title));

	// Initialize platform as window only, no audio, no video
	if(fplPlatformInit(fplInitFlags_Window, &settings)) {

		// Keep window alive, by calling its update function
		while(fplWindowUpdate()) {
			// Poll(handle) incoming events
			fplEvent ev;
			while(fplPollEvent(&ev)) {
				switch (ev.type) {
					// Window events
					case fplEventType_Window:
					{
						switch (ev.window.type) {
							case fplWindowEventType_Closed:
								// Window was closed
								break;
							case fplWindowEventType_LostFocus:
								// Window has lost the focus
								break;
							case fplWindowEventType_GotFocus:
								// Window is now active
								break;
							case fplWindowEventType_PositionChanged:
								// Window position was changed
								break;
							case fplWindowEventType_Resized:
								// Window size was changed
								break;
							case fplWindowEventType_Minimized:
								// Window was minimized
								break;
							case fplWindowEventType_Maximized:
								// Window was maximized
								break;
							case fplWindowEventType_Restored:
								// Window was restored from the minimized state
								break;
							case fplWindowEventType_DroppedFiles: {
								// N-Files was dragged into the window area
								size_t fileCount = ev.window.dropFiles.fileCount;
								for (size_t i = 0; i < fileCount; ++i) {
									// Full path to file
									// WARNING: Copy the string to your own storage, because the "files" array will be cleaned up after the next event poll or the platform was released
									const char *filePath = ev.window.dropFiles.files[i];

								}
							} break;
						}
					} break;

					// Keyboard events
					case fplEventType_Keyboard:
					{
						switch (ev.keyboard.type) {
							// A button was pressed, released, hold
							case fplKeyboardEventType_Button:
							{
								fplKey mappedKey = ev.keyboard.mappedKey;
								uint64_t rawKeyCode = ev.keyboard.keyCode;
							} break;

							// Handle text character input (This is called after the button event)
							case fplKeyboardEventType_Input:
							{
								// Use this unicode char to append it to a text field or something
								uint64_t unicodeChar = ev.keyboard.keyCode;

							} break;
						}
					} break;

					// Keyboard events
					case fplEventType_Mouse:
					{
						switch (ev.mouse.type) {
							// A mouse button was pressed, released, hold
							case fplMouseEventType_Button:
							{
								fplButtonState state = ev.mouse.buttonState;

								bool isDown = state >= fplButtonState_Press;

								// While a button press, the mouse may be moved
								int32_t x = ev.mouse.mouseX;
								int32_t y = ev.mouse.mouseY;
							} break;

							// Handle mouse motion/move
							case fplMouseEventType_Move:
							{
								int32_t x = ev.mouse.mouseX;
								int32_t y = ev.mouse.mouseY;

								// While a move buttons may be pressed
								fplButtonState state = ev.mouse.buttonState;
							} break;

							// Handle mouse wheel
							case fplMouseEventType_Wheel:
							{
								// Wheel delta
								float delta = ev.mouse.wheelDelta;

								// While a mouse wheel change, the mouse may be moved
								int32_t x = ev.mouse.mouseX;
								int32_t y = ev.mouse.mouseY;
							} break;
						}
					} break;

					// Gamepad events
					case fplEventType_Gamepad:
					{
						switch (ev.gamepad.type) {
							// The full state of a connected gamepad has been changed
							case fplGamepadEventType_StateChanged:
							{
								// The index of the device
								uint32_t deviceIndex = ev.gamepad.deviceIndex;

								// The name of the device (may be empty or null)
								const char *deviceName = ev.gamepad.deviceName;

								// Holds the full gamepad state
								const fplGamepadState *state = &ev.gamepad.state;

								// This is true when this is connected, which should always be true for StateChanged
								bool isConnected = state->isConnected;

								// This is true when any button/analog states has been changed
								bool isActive = state->isActive;
								
								if (state->start.isDown) {
									// Start button is pressed/hold
								}
							} break;

							// A new gamepad was connected
							case fplGamepadEventType_Connected:
							{
								// The index of the device
								uint32_t deviceIndex = ev.gamepad.deviceIndex;

								// The name of the device (may be empty or null)
								const char *deviceName = ev.gamepad.deviceName;
							} break;

							// An old gamepad was disconnected
							case fplGamepadEventType_Disconnected:
							{
								// The index of the device
								uint32_t deviceIndex = ev.gamepad.deviceIndex;

								// The name of the device (may be empty or null)
								const char *deviceName = ev.gamepad.deviceName;
							} break;
						}
					} break;

					default:
						break;
				}
			}
		}

		// Release the window and free any internal platform resources
		fplPlatformRelease();
	}

	// We are done
	return 0;
}