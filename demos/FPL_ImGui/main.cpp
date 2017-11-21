#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#include <final_platform_layer.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include <imgui\imgui.h>

#include <math.h>

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

static char clipboardBuffer[1024];
static const char *ClipboardGetFunc(void *user) {
	char *result = GetClipboardAnsiText(clipboardBuffer, FPL_ARRAYCOUNT(clipboardBuffer));
	return(result);
}
static void ClipboardSetFunc(void *user, const char *text) {
	SetClipboardText(text);
}

static void InitImGUI() {
	ImGuiIO& io = ImGui::GetIO();

	io.GetClipboardTextFn = ClipboardGetFunc;
	io.SetClipboardTextFn = ClipboardSetFunc;
	io.RenderDrawListsFn = ImGUIRenderDrawLists;
	io.IniFilename = nullptr;
	io.KeyMap[ImGuiKey_Tab] = (uint32_t)Key::Key_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = (uint32_t)Key::Key_Left;
	io.KeyMap[ImGuiKey_RightArrow] = (uint32_t)Key::Key_Right;
	io.KeyMap[ImGuiKey_UpArrow] = (uint32_t)Key::Key_Up;
	io.KeyMap[ImGuiKey_DownArrow] = (uint32_t)Key::Key_Down;
	io.KeyMap[ImGuiKey_PageUp] = (uint32_t)Key::Key_PageUp;
	io.KeyMap[ImGuiKey_PageDown] = (uint32_t)Key::Key_PageDown;
	io.KeyMap[ImGuiKey_Home] = (uint32_t)Key::Key_Home;
	io.KeyMap[ImGuiKey_End] = (uint32_t)Key::Key_End;
	io.KeyMap[ImGuiKey_Delete] = (uint32_t)Key::Key_Delete;
	io.KeyMap[ImGuiKey_Backspace] = (uint32_t)Key::Key_Backspace;
	io.KeyMap[ImGuiKey_Enter] = (uint32_t)Key::Key_Enter;
	io.KeyMap[ImGuiKey_Escape] = (uint32_t)Key::Key_Escape;
	io.KeyMap[ImGuiKey_A] = (uint32_t)Key::Key_A;
	io.KeyMap[ImGuiKey_C] = (uint32_t)Key::Key_C;
	io.KeyMap[ImGuiKey_V] = (uint32_t)Key::Key_V;
	io.KeyMap[ImGuiKey_X] = (uint32_t)Key::Key_X;
	io.KeyMap[ImGuiKey_Y] = (uint32_t)Key::Key_Y;
	io.KeyMap[ImGuiKey_Z] = (uint32_t)Key::Key_Z;

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

static void ImGUIKeyEvent(uint64_t keyCode, Key mappedKey, KeyboardModifierFlags modifiers, bool down) {
	ImGuiIO& io = ImGui::GetIO();
	if (mappedKey != Key::Key_None) {
		io.KeysDown[(uint32_t)mappedKey] = down;
	} else {
		io.KeysDown[keyCode] = down;
	}
	io.KeyCtrl = modifiers & KeyboardModifierFlags::Ctrl;
	io.KeyShift = modifiers & KeyboardModifierFlags::Shift;
	io.KeyAlt = modifiers & KeyboardModifierFlags::Alt;
	io.KeySuper = modifiers & KeyboardModifierFlags::Super;
}

static bool show_test_window = true;
static bool show_another_window = false;
static ImVec4 clear_color = ImColor(114, 144, 154);

static void UpdateAndRender(const float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	WindowSize windowArea = GetWindowArea();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)windowArea.width;
	io.DisplaySize.y = (float)windowArea.height;
	io.DisplayFramebufferScale = ImVec2(1, 1);

	io.MousePos = ImVec2((float)currentMousePosition[0], (float)currentMousePosition[1]);
	for (int mouseButton = 0; mouseButton < 3; ++mouseButton) {
		io.MouseDown[mouseButton] = currentMouseStates[mouseButton];
	}
	if (fabsf(currentMouseWheelDelta) > 0.0f) {
		io.MouseWheel = currentMouseWheelDelta > 0.0f ? 1.0f : -1.0f;
	} else {
		io.MouseWheel = 0.0f;
	}
	currentMouseWheelDelta = 0.0f;

	SetWindowCursorEnabled(!io.MouseDrawCursor);

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

	glViewport(0, 0, windowArea.width, windowArea.height);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}

int main(int argc, char **args) {
	int result = 0;
	Settings settings = DefaultSettings();
	CopyAnsiString("ImGUI Example", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle) - 1);
	settings.window.windowWidth = 1280;
	settings.window.windowHeight = 720;
	settings.video.driverType = VideoDriverType::OpenGL;
	if (InitPlatform(InitFlags::Video, settings)) {
		InitImGUI();

		ImGuiIO& io = ImGui::GetIO();

		double lastTime = GetHighResolutionTimeInSeconds();
		float lastDeltaTime = 1.0f / 60.0f;

		while (IsWindowRunning()) {
			WindowUpdate();

			Event event;
			while (PollWindowEvent(event)) {
				switch (event.type) {
					case EventType::Keyboard:
					{
						switch (event.keyboard.type) {
							case KeyboardEventType::KeyDown:
							{
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, event.keyboard.modifiers, true);
							} break;
							case KeyboardEventType::KeyUp:
							{
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, event.keyboard.modifiers, false);
							} break;
							case KeyboardEventType::Char:
							{
								if (event.keyboard.keyCode > 0 && event.keyboard.keyCode < 0x10000) {
									io.AddInputCharacter(ImWchar(event.keyboard.keyCode));
								}
							} break;
						}
					} break;
					case EventType::Mouse:
					{
						switch (event.mouse.type) {
							case MouseEventType::Move:
							{
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case MouseEventType::Wheel:
							{
								currentMouseWheelDelta += event.mouse.wheelDelta;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case MouseEventType::ButtonDown:
							{
								currentMouseStates[(int32_t)event.mouse.mouseButton] = true;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
							case MouseEventType::ButtonUp:
							{
								currentMouseStates[(int32_t)event.mouse.mouseButton] = false;
								currentMousePosition[0] = event.mouse.mouseX;
								currentMousePosition[1] = event.mouse.mouseY;
							} break;
						}
					} break;
				}
			}

			UpdateAndRender(lastDeltaTime);

			WindowFlip();

			double currentTime = GetHighResolutionTimeInSeconds();
			lastDeltaTime = lastTime > 0.0 ? (float)(currentTime - lastTime) : (float)(1.0f / 60.0f);
			lastTime = currentTime;
		}

		ReleaseImGUI();
		ImGui::Shutdown();

		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}