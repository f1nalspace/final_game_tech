/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImGui

Description:
	Full implementation for running ImGui with all features

Requirements:
	- C++ Compiler
	- Final Platform Layer
	- ImGui v1.51 (Included in demo)

Author:
	Torsten Spaete

Changelog:
    ## 2019-08-13
    - Fixed compiler warnings for GCC

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3

	## 2018-10-16
	- Show display informations

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-06-29
	- Changed to use new keyboard/mouse button state

	## 2018-06-19
	- Corrected for FPL enum operator changes

	## 2018-06-06
	- Refactored files

	## 2018-05-05:
	- Added m to linker flags in CMakeLists and Makefile

	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C++ always

License:
	Copyright (c) 2017-2023 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

// @TODO(final): Use final_dynamic_opengl here, so we dont need any linking like any other opengl demo
#include <GL/gl.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include <imgui/imgui.h>

#include <math.h> // fabsf

static int currentMousePosition[2] = { -1, -1 };
static bool currentMouseStates[3] = { 0 };
static float currentMouseWheelDelta = 0.0f;
static GLuint fontTextureId = 0;

static void ImGUIRenderDrawLists(ImDrawData* draw_data) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if(fb_width == 0 || fb_height == 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	//glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render command lists
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
	for(int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for(int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if(pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			} else {
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}
#undef OFFSETOF

	// Restore modified state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

static char clipboardBuffer[1024];
static const char *ClipboardGetFunc(void *user) {
	if(fplGetClipboardText(clipboardBuffer, fplArrayCount(clipboardBuffer))) {
		return clipboardBuffer;
	}
	return nullptr;
}
static void ClipboardSetFunc(void *user, const char *text) {
	fplSetClipboardText(text);
}

static void InitImGUI() {
	ImGuiIO& io = ImGui::GetIO();

	io.GetClipboardTextFn = ClipboardGetFunc;
	io.SetClipboardTextFn = ClipboardSetFunc;
	io.RenderDrawListsFn = ImGUIRenderDrawLists;
	io.IniFilename = nullptr;
	io.KeyMap[ImGuiKey_Tab] = (uint32_t)fplKey_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = (uint32_t)fplKey_Left;
	io.KeyMap[ImGuiKey_RightArrow] = (uint32_t)fplKey_Right;
	io.KeyMap[ImGuiKey_UpArrow] = (uint32_t)fplKey_Up;
	io.KeyMap[ImGuiKey_DownArrow] = (uint32_t)fplKey_Down;
	io.KeyMap[ImGuiKey_PageUp] = (uint32_t)fplKey_PageUp;
	io.KeyMap[ImGuiKey_PageDown] = (uint32_t)fplKey_PageDown;
	io.KeyMap[ImGuiKey_Home] = (uint32_t)fplKey_Home;
	io.KeyMap[ImGuiKey_End] = (uint32_t)fplKey_End;
	io.KeyMap[ImGuiKey_Delete] = (uint32_t)fplKey_Delete;
	io.KeyMap[ImGuiKey_Backspace] = (uint32_t)fplKey_Backspace;
	io.KeyMap[ImGuiKey_Enter] = (uint32_t)fplKey_Return;
	io.KeyMap[ImGuiKey_Escape] = (uint32_t)fplKey_Escape;
	io.KeyMap[ImGuiKey_A] = (uint32_t)fplKey_A;
	io.KeyMap[ImGuiKey_C] = (uint32_t)fplKey_C;
	io.KeyMap[ImGuiKey_V] = (uint32_t)fplKey_V;
	io.KeyMap[ImGuiKey_X] = (uint32_t)fplKey_X;
	io.KeyMap[ImGuiKey_Y] = (uint32_t)fplKey_Y;
	io.KeyMap[ImGuiKey_Z] = (uint32_t)fplKey_Z;

	io.Fonts->AddFontDefault();

	// Build texture atlas
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &fontTextureId);
	glBindTexture(GL_TEXTURE_2D, fontTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)fontTextureId;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

static void ReleaseImGUI() {
	if(fontTextureId) {
		glDeleteTextures(1, &fontTextureId);
		ImGui::GetIO().Fonts->TexID = 0;
		fontTextureId = 0;
	}
}

static void ImGUIKeyEvent(uint64_t keyCode, fplKey mappedKey, fplKeyboardModifierFlags modifiers, bool down) {
	ImGuiIO& io = ImGui::GetIO();
	if(mappedKey != fplKey_None) {
		io.KeysDown[(uint32_t)mappedKey] = down;
	} else {
		io.KeysDown[keyCode] = down;
	}
	io.KeyCtrl = ((modifiers & fplKeyboardModifierFlags_LCtrl) == fplKeyboardModifierFlags_LCtrl) || ((modifiers & fplKeyboardModifierFlags_RCtrl) == fplKeyboardModifierFlags_RCtrl);
	io.KeyShift = ((modifiers & fplKeyboardModifierFlags_LShift) == fplKeyboardModifierFlags_LShift) || ((modifiers & fplKeyboardModifierFlags_RShift) == fplKeyboardModifierFlags_RShift);
	io.KeyAlt = ((modifiers & fplKeyboardModifierFlags_LAlt) == fplKeyboardModifierFlags_LAlt) || ((modifiers & fplKeyboardModifierFlags_RAlt) == fplKeyboardModifierFlags_RAlt);
	io.KeySuper = ((modifiers & fplKeyboardModifierFlags_LSuper) == fplKeyboardModifierFlags_LSuper) || ((modifiers & fplKeyboardModifierFlags_RSuper) == fplKeyboardModifierFlags_RSuper);
}

static bool show_test_window = true;
static bool show_another_window = false;

static bool showDisplaysWindow = false;
static bool showAudioDevicesWindow = false;

static ImVec4 clear_color = ImColor(114, 144, 154);
static size_t displayCount = 0;
static size_t audioDeviceCount = 0;
static fplDisplayInfo displays[16] = fplZeroInit;
static fplAudioDeviceInfoExtended audioDevices[16] = fplZeroInit;

static void UpdateAndRender(const float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	fplWindowSize windowArea = {};
	fplGetWindowSize(&windowArea);
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)windowArea.width;
	io.DisplaySize.y = (float)windowArea.height;
	io.DisplayFramebufferScale = ImVec2(1, 1);

	io.MousePos = ImVec2((float)currentMousePosition[0], (float)currentMousePosition[1]);
	for(int mouseButton = 0; mouseButton < 3; ++mouseButton) {
		io.MouseDown[mouseButton] = currentMouseStates[mouseButton];
	}
	if(fabsf(currentMouseWheelDelta) > 0.0f) {
		io.MouseWheel = currentMouseWheelDelta > 0.0f ? 1.0f : -1.0f;
	} else {
		io.MouseWheel = 0.0f;
	}
	currentMouseWheelDelta = 0.0f;

	fplSetWindowCursorEnabled(!io.MouseDrawCursor);

	ImGui::NewFrame();

	// 1. Show a simple window
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if(ImGui::Button("Test Window")) show_test_window ^= 1;
		if(ImGui::Button("Another Window")) show_another_window ^= 1;
		if(ImGui::Button("Toggle app floating")) {
			fplSetWindowFloating(!fplIsWindowFloating());
		}
		if(ImGui::Button("Toggle app decorated")) {
			fplSetWindowDecorated(!fplIsWindowDecorated());
		}
		if(ImGui::Button("Toggle app resizable")) {
			fplSetWindowResizeable(!fplIsWindowResizable());
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if(show_another_window) {
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	fplDisplayInfo primaryDisplay = fplZeroInit;
	fplGetPrimaryDisplay(&primaryDisplay);

	fplDisplayInfo windowDisplay = fplZeroInit;
	fplGetWindowDisplay(&windowDisplay);

	ImGui::SetNextWindowPos(ImVec2(60, 480), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Displays", &showDisplaysWindow);
	ImGui::Text("Count: %zu", displayCount);
	for(size_t i = 0; i < displayCount; ++i) {
		fplDisplayInfo *display = displays + i;
		ImGui::BulletText("Display[%zu]: %s, Pos: %d x %d, Size: %d x %d%s", i, display->id, display->virtualPosition.left, display->virtualPosition.top, display->virtualSize.width, display->virtualSize.height, (display->isPrimary ? " [Primary]" : ""));
	}
	ImGui::Text("Window Display: %s, Pos: %d x %d, Size: %d x %d, Is primary: %s", windowDisplay.id, windowDisplay.virtualPosition.left, windowDisplay.virtualPosition.top, windowDisplay.virtualSize.width, windowDisplay.virtualSize.height, (windowDisplay.isPrimary ? "true" : "false"));
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(560, 480), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Audio Devices", &showAudioDevicesWindow);
	ImGui::Text("Count: %zu", audioDeviceCount);
	for(size_t i = 0; i < audioDeviceCount; ++i) {
		fplAudioDeviceInfoExtended *info = audioDevices + i;
		ImGui::BulletText("Audio Device[%zu]: %s%s", i, info->info.name, (info->info.isDefault ? " [Default]" : ""));
	}
	ImGui::End();

	glViewport(0, 0, windowArea.width, windowArea.height);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}

int main(int argc, char **args) {
	int result = 0;
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("ImGUI Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = 1280;
	settings.window.windowSize.height = 720;
	settings.video.backend = fplVideoBackendType_OpenGL;
	if(fplPlatformInit(fplInitFlags_All, &settings)) {
		displayCount = fplGetDisplays(displays, fplArrayCount(displays));

		audioDeviceCount = fplGetAudioDevices(fplArrayCount(audioDevices), sizeof(audioDevices[0]), (fplAudioDeviceInfo *)audioDevices);

		InitImGUI();

		ImGuiIO& io = ImGui::GetIO();

		fplTimestamp lastTime = fplTimestampQuery();
		fplTimestamp currentTime = fplZeroInit;
		float lastDeltaTime = 1.0f / 60.0f;

		while(fplWindowUpdate()) {
			fplEvent event;
			while(fplPollEvent(&event)) {
				switch(event.type) {
					case fplEventType_Keyboard:
					{
						switch(event.keyboard.type) {
							case fplKeyboardEventType_Button:
							{
								bool isDown = event.keyboard.buttonState >= fplButtonState_Press;
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, event.keyboard.modifiers, isDown);
							} break;
							case fplKeyboardEventType_Input:
							{
								if(event.keyboard.keyCode > 0 && event.keyboard.keyCode < 0x10000) {
									io.AddInputCharacter(ImWchar(event.keyboard.keyCode));
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
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case fplMouseEventType_Wheel:
							{
								currentMouseWheelDelta += event.mouse.wheelDelta;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case fplMouseEventType_Button:
							{
								currentMouseStates[(int32_t)event.mouse.mouseButton] = event.mouse.buttonState >= fplButtonState_Press;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							default:
								break;
						}
					} break;
					default:
						break;
				}
			}

			UpdateAndRender(lastDeltaTime);

			fplVideoFlip();

			currentTime = fplTimestampQuery();
			double elapsedTime = fplTimestampElapsed(lastTime, currentTime);
			lastDeltaTime = elapsedTime > 0.0 ? (float)elapsedTime : (float)(1.0f / 60.0f);
			lastTime = currentTime;
		}

		ReleaseImGUI();
		ImGui::Shutdown();

		fplPlatformRelease();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}