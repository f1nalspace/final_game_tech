#define FPL_IMPLEMENTATION
#define FPL_ENABLE_CLIB_ASSERTIONS 1
#define FPL_DEFAULT_WINDOW_WIDTH 1280
#define FPL_DEFAULT_WINDOW_HEIGHT 720
#include <final_platform_layer.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "imgui\imgui.h"

static int currentMousePosition[2] = { -1, -1 };
static bool currentMouseStates[3] = { 0 };
static float currentMouseWheelDelta = 0.0f;

static GLuint fontTextureId = 0;

static void ImGUIRenderDrawLists(ImDrawData* draw_data) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
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
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback) {
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

static void InitImGUI() {
	ImGuiIO& io = ImGui::GetIO();

	io.RenderDrawListsFn = ImGUIRenderDrawLists;
	io.KeyMap[ImGuiKey_Tab] = fpl_Key_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = fpl_Key_Left;
	io.KeyMap[ImGuiKey_RightArrow] = fpl_Key_Right;
	io.KeyMap[ImGuiKey_UpArrow] = fpl_Key_Up;
	io.KeyMap[ImGuiKey_DownArrow] = fpl_Key_Down;
	io.KeyMap[ImGuiKey_PageUp] = fpl_Key_PageUp;
	io.KeyMap[ImGuiKey_PageDown] = fpl_Key_PageDown;
	io.KeyMap[ImGuiKey_Home] = fpl_Key_Home;
	io.KeyMap[ImGuiKey_End] = fpl_Key_End;
	io.KeyMap[ImGuiKey_Delete] = fpl_Key_Delete;
	io.KeyMap[ImGuiKey_Backspace] = fpl_Key_Backspace;
	io.KeyMap[ImGuiKey_Enter] = fpl_Key_Enter;
	io.KeyMap[ImGuiKey_Escape] = fpl_Key_Escape;
	io.KeyMap[ImGuiKey_A] = fpl_Key_A;
	io.KeyMap[ImGuiKey_C] = fpl_Key_C;
	io.KeyMap[ImGuiKey_V] = fpl_Key_V;
	io.KeyMap[ImGuiKey_X] = fpl_Key_X;
	io.KeyMap[ImGuiKey_Y] = fpl_Key_Y;
	io.KeyMap[ImGuiKey_Z] = fpl_Key_Z;

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
	if (fontTextureId) {
		glDeleteTextures(1, &fontTextureId);
		ImGui::GetIO().Fonts->TexID = 0;
		fontTextureId = 0;
	}
}

static void ImGUIKeyEvent(uint64_t keyCode, fpl_Key mappedKey, bool down) {
	ImGuiIO& io = ImGui::GetIO();
	if (mappedKey != fpl_Key_None) {
		io.KeysDown[mappedKey] = down;
	} else {
		io.KeysDown[keyCode] = down;
	}
	io.KeyCtrl = io.KeysDown[fpl_Key_LeftControl] || io.KeysDown[fpl_Key_RightControl];
	io.KeyShift = io.KeysDown[fpl_Key_LeftShift] || io.KeysDown[fpl_Key_RightShift];
	io.KeyAlt = io.KeysDown[fpl_Key_LeftAlt] || io.KeysDown[fpl_Key_RightAlt];
	io.KeySuper = io.KeysDown[fpl_Key_LeftWin] || io.KeysDown[fpl_Key_RightWin];
}

static bool show_test_window = true;
static bool show_another_window = false;
static ImVec4 clear_color = ImColor(114, 144, 154);

static void UpdateAndRender(const float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	int32_t windowWidth = fpl_GetWindowWidth();
	int32_t windowHeight = fpl_GetWindowHeight();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)windowWidth;
	io.DisplaySize.y = (float)windowHeight;
	io.DisplayFramebufferScale = ImVec2(1, 1);

	io.MousePos = ImVec2((float)currentMousePosition[0], (float)currentMousePosition[1]);
	for (int mouseButton = 0; mouseButton < 3; ++mouseButton) {
		io.MouseDown[mouseButton] = currentMouseStates[mouseButton];
	}
	io.MouseWheel = currentMouseWheelDelta;
	currentMouseWheelDelta = 0.0f;

	ImGui::NewFrame();

	// 1. Show a simple window
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window) {
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}

int main(int argc, char **args) {
	int result = 0;
	if (fpl_Init({ fpl_InitFlag_VideoOpenGL })) {
		InitImGUI();

		ImGuiIO& io = ImGui::GetIO();

		double lastTime = fpl_GetHighResolutionTimeInSeconds();
		float lastDeltaTime = 1.0f / 60.0f;

		while (fpl_IsWindowRunning()) {
			fpl_WindowUpdate();

			fpl_Event event;
			while (fpl_PollEvent(&event)) {
				switch (event.type) {
					case fpl_EventType_Keyboard:
					{
						switch (event.keyboard.type) {
							case fpl_KeyboardEventType_KeyDown:
							{
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, true);
							} break;
							case fpl_KeyboardEventType_KeyUp:
							{
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, false);
							} break;
							case fpl_KeyboardEventType_Char:
							{
								if (event.keyboard.keyCode > 0 && event.keyboard.keyCode < 0x10000) {
									io.AddInputCharacter(ImWchar(event.keyboard.keyCode));
								}
							} break;
						}
					} break;
					case fpl_EventType_Mouse:
					{
						switch (event.mouse.type) {
							case fpl_MouseEventType_Move:
							{
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case fpl_MouseEventType_Wheel:
							{
								currentMouseWheelDelta += event.mouse.wheelDelta;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case fpl_MouseEventType_ButtonDown:
							{
								currentMouseStates[event.mouse.mouseButton] = true;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case fpl_MouseEventType_ButtonUp:
							{
								currentMouseStates[event.mouse.mouseButton] = false;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
						}
					} break;
				}
			}

			UpdateAndRender(lastDeltaTime);

			fpl_WindowFlip();

			double currentTime = fpl_GetHighResolutionTimeInSeconds();
			lastDeltaTime = lastTime > 0.0 ? (float)(currentTime - lastTime) : (float)(1.0f / 60.0f);
			lastTime = currentTime;
		}

		ReleaseImGUI();
		ImGui::Shutdown();

		fpl_Release();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}