#define FPL_IMPLEMENTATION
#define FPL_NO_AUDIO
#define FPL_LOGGING
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include <imgui/imgui.h>

#include <math.h> // fabsf

#define FINAL_GAMEBOY_IMPLEMENTATION
#include "final_gameboy.h"

#define STR_CONCAT(a, b) a ## b
#define STR_FORMAT_CONCAT_3(a, b, c) STR_CONCAT(STR_CONCAT(#a, b), #c)

static int currentMousePosition[2] = { -1, -1 };
static bool currentMouseStates[3] = { 0 };
static float currentMouseWheelDelta = 0.0f;
static GLuint fontTextureId = 0;

static void ImGUIRenderDrawLists(ImDrawData *draw_data) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO &io = ImGui::GetIO();
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
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid *)((const char *)vtx_buffer + fplOffsetOf(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid *)((const char *)vtx_buffer + fplOffsetOf(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid *)((const char *)vtx_buffer + fplOffsetOf(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
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
	if (fplGetClipboardText(clipboardBuffer, fplArrayCount(clipboardBuffer))) {
		return clipboardBuffer;
	}
	return nullptr;
}
static void ClipboardSetFunc(void *user, const char *text) {
	fplSetClipboardText(text);
}

static void InitImGUI() {
	ImGuiIO &io = ImGui::GetIO();

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
	if (fontTextureId) {
		glDeleteTextures(1, &fontTextureId);
		ImGui::GetIO().Fonts->TexID = 0;
		fontTextureId = 0;
	}
}

static void ImGUIKeyEvent(uint64_t keyCode, fplKey mappedKey, fplKeyboardModifierFlags modifiers, bool down) {
	ImGuiIO &io = ImGui::GetIO();
	if (mappedKey != fplKey_None) {
		io.KeysDown[(uint32_t)mappedKey] = down;
	} else {
		io.KeysDown[keyCode] = down;
	}
	io.KeyCtrl = ((modifiers & fplKeyboardModifierFlags_LCtrl) == fplKeyboardModifierFlags_LCtrl) || ((modifiers & fplKeyboardModifierFlags_RCtrl) == fplKeyboardModifierFlags_RCtrl);
	io.KeyShift = ((modifiers & fplKeyboardModifierFlags_LShift) == fplKeyboardModifierFlags_LShift) || ((modifiers & fplKeyboardModifierFlags_RShift) == fplKeyboardModifierFlags_RShift);
	io.KeyAlt = ((modifiers & fplKeyboardModifierFlags_LAlt) == fplKeyboardModifierFlags_LAlt) || ((modifiers & fplKeyboardModifierFlags_RAlt) == fplKeyboardModifierFlags_RAlt);
	io.KeySuper = ((modifiers & fplKeyboardModifierFlags_LSuper) == fplKeyboardModifierFlags_LSuper) || ((modifiers & fplKeyboardModifierFlags_RSuper) == fplKeyboardModifierFlags_RSuper);
}

static ImVec2 ImGUIRenderTextWithBackground(const char *text, const ImColor &foreground, const ImColor &background, const float horizontalPadding = 0.0f) {
	ImVec2 cursor = ImGui::GetCursorScreenPos();

	ImVec2 textSize = ImGui::CalcTextSize(text);
	textSize.x += horizontalPadding * 2.0f;

	ImGui::GetWindowDrawList()->AddRectFilled(cursor, ImVec2(cursor.x + textSize.x, cursor.y + textSize.y), background);

	cursor.x += horizontalPadding;
	ImGui::SetCursorScreenPos(cursor);

	ImGui::TextColored(foreground, text);

	//ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 0, 255, 255));

	return textSize;
}

static inline bool IsCharNumeric(const char c) {
	bool result = c >= '0' && c <= '9';
	return result;
}

static inline bool IsCharHexAlpha(const char c) {
	bool result = (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
	return result;
}

static inline bool IsCharHex(const char c) {
	bool result = IsCharNumeric(c) || IsCharHexAlpha(c);
	return result;
}

fpl_common_api int32_t ParseHex32(const char *str, const size_t len) {
	FPL__CheckArgumentNull(str, 0);
	FPL__CheckArgumentZero(len, 0);
	const char *p = str;
	bool isNegative = false;
	if (*p == '#' || *p == '$') {
		++p;
	}
	uint32_t value = 0;
	while (*p && ((size_t)(p - str) < len)) {
		char c = *p;

		int v;
		if (IsCharNumeric(c))
			v = (int)(*p - '0');
		else if (c >= 'a' && c <= 'f')
			v = (int)(*p - 'a');
		else if (c >= 'A' && c <= 'F')
			v = (int)(*p - 'A');
		else
			return 0;

		value *= 16;

		value += (uint32_t)v;

		++p;
	}
	int32_t result = (int32_t)value;
	return(result);
}

struct UIRegisterState {
	bool showAsInt;
};

static ImColor registerNameColor = IM_COL32(255, 255, 255, 255);

static ImColor registerValueForeground = IM_COL32(255, 255, 255, 255);
static ImColor registerValueBackground = IM_COL32(32, 32, 32, 255);

static void RenderRegisterPair(const char *name, const uint8_t first, const uint8_t second, const char *valueFormat) {
	char textBuffer[10];

	ImGui::SameLine();
	ImGui::TextColored(registerNameColor, name);

	ImGui::SameLine();
	fplStringFormat(textBuffer, fplArrayCount(textBuffer), valueFormat, first);
	ImGUIRenderTextWithBackground(textBuffer, registerValueForeground, registerValueBackground);

	ImGui::SameLine();
	fplStringFormat(textBuffer, fplArrayCount(textBuffer), valueFormat, second);
	ImGUIRenderTextWithBackground(textBuffer, registerValueForeground, registerValueBackground);
}

static void RenderRegister16(const char *name, const uint16_t value, const char *valueFormat) {
	char textBuffer[10];

	ImGui::SameLine();
	ImGui::TextColored(registerNameColor, name);

	ImGui::SameLine();
	fplStringFormat(textBuffer, fplArrayCount(textBuffer), valueFormat, value);
	ImGUIRenderTextWithBackground(textBuffer, registerValueForeground, registerValueBackground);
}

static void RenderRegister(const char *name, UIRegisterState &state, const fgbRegister &reg) {
	ImGui::Begin(name, nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

	ImGui::BeginGroup();

	ImGui::SameLine();
	ImGui::Checkbox("As Int", &state.showAsInt);

	ImGui::Separator();

	float regionWidth = ImGui::GetWindowContentRegionWidth();

	ImGui::BeginChild(ImGuiID(1), ImVec2(0,0), false, ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar);

	const char *valueFormat8;
	const char *valueFormat16;
	if (state.showAsInt) {
		valueFormat8 = "%d";
		valueFormat16 = "%d";
	} else {
		valueFormat8 = "$%02X";
		valueFormat16 = "$%04X";
	}

	RenderRegisterPair("AF", reg.a, reg.f.value, valueFormat8);
	ImGui::NewLine();
	RenderRegisterPair("BC", reg.b, reg.c, valueFormat8);
	ImGui::NewLine();
	RenderRegisterPair("DE", reg.d, reg.e, valueFormat8);
	ImGui::NewLine();
	RenderRegisterPair("HL", reg.h, reg.l, valueFormat8);
	ImGui::Separator();
	ImGui::NewLine();
	RenderRegister16("SP", reg.sp, valueFormat16);
	ImGui::NewLine();
	RenderRegister16("PC", reg.pc, valueFormat16);

	ImGui::EndChild();

	ImGui::EndGroup();

	ImGui::End();
}

static const char *memoryColumnCountLabels[] = { "2", "4", "6", "8", "16" };
static int memoryColumnCounts[] = { 2, 4, 6, 8, 16 };
static int memoryDefaultColumnIndex = 3;

ImColor memoryAddressBackgroundColor = IM_COL32(54, 54, 54, 255);
ImColor memoryAddressForegroundColor = IM_COL32(200, 200, 200, 255);

ImColor memoryValueBackgroundColor = IM_COL32(54, 54, 54, 255);
ImColor memoryValueForegroundColor = IM_COL32(255, 255, 255, 255);

struct UIMemoryTableState {
	char jumpLineText[100];
	int columnCountIndex;
	bool valuesAsInteger;
};


static void RenderMemory(const char *name, UIMemoryTableState &state, const uint8_t *data, const size_t dataSize, const uint32_t dataOffset = 0) {
	const uint8_t *memoryBase = data;
	size_t memorySize = dataSize;

	uint32_t selectedMemoryAddress = 0;
	uint32_t memoryAddressBase = 0;

	int maxColumnCounts = fplArrayCount(memoryColumnCounts);

		//ImGui::SetNextWindowPos(pos);

	ImGui::Begin(name);

	ImGui::BeginGroup();

	ImGui::SameLine();
	ImGui::PushItemWidth(60);
	ImGui::Combo("Columns", &state.columnCountIndex, memoryColumnCountLabels, fplArrayCount(memoryColumnCountLabels));
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::Checkbox("Values as Int", &state.valuesAsInteger);

	ImGui::PushItemWidth(150);
	ImGui::SameLine();
	bool jumpToAddress = ImGui::Button("Goto");
	ImGui::SameLine();
	jumpToAddress |= ImGui::InputText("##Line", state.jumpLineText, fplArrayCount(state.jumpLineText), ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::PopItemWidth();

	uint32_t addressToJump = 0;
	size_t jumpLineTextLen;
	if (jumpToAddress && (jumpLineTextLen = fplGetStringLength(state.jumpLineText)) > 0) {
		if (state.jumpLineText[0] == '#' || state.jumpLineText[0] == '$') {
			int32_t jumpLine = ParseHex32(state.jumpLineText, jumpLineTextLen);
			addressToJump = (uint32_t)jumpLine;
		} else {
			int32_t jumpLine = fplStringToS32Len(state.jumpLineText, jumpLineTextLen);
			addressToJump = (uint32_t)jumpLine;
		}
		fplClearStruct(state.jumpLineText);
	}

	float regionWidth = ImGui::GetWindowContentRegionWidth();

	ImGui::Separator();

	ImVec2 scrollSize = ImVec2(regionWidth * 1.0f, 300);

	uint32_t memoryAddress = memoryAddressBase;
	uint32_t memoryIndex = 0;

	int columnCount = memoryColumnCounts[state.columnCountIndex % maxColumnCounts];
	int rowCount = 1 + ((int)memorySize / columnCount);

	const char *addressFormat;
	if (memorySize <= UINT8_MAX) {
		addressFormat = "$%02X";
	} else if (memorySize <= UINT16_MAX) {
		addressFormat = "$%04X";
	} else if (memorySize <= UINT32_MAX) {
		addressFormat = "$%08X";
	} else {
		addressFormat = "$%16X";
	}

	ImGui::BeginChild(ImGuiID(1), scrollSize, false, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar);

	char textBuffer[255];

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {

		fplStringFormat(textBuffer, fplArrayCount(textBuffer), addressFormat, memoryAddress);

		ImGui::SameLine();
		ImGUIRenderTextWithBackground(textBuffer, memoryAddressForegroundColor, memoryAddressBackgroundColor);

		for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex) {

			if (memoryIndex < memorySize) {
				uint8_t u8 = memoryBase[memoryIndex];
				const char *byteFormat;
				if (state.valuesAsInteger)
					byteFormat = "%d";
				else
					byteFormat = "%02X";

				char buf[32];
				fplStringFormat(buf, fplArrayCount(buf), byteFormat, u8);

				ImGui::SameLine();

				ImGUIRenderTextWithBackground(buf, memoryValueForegroundColor, memoryValueBackgroundColor, 0.0f);

				if (jumpToAddress && memoryIndex == addressToJump)
					ImGui::SetScrollHere();

				if (ImGui::IsItemHovered()) {
					char hoverFormat[32];
					fplStringFormat(hoverFormat, fplArrayCount(hoverFormat), "%s = %s", addressFormat, byteFormat);
					fplStringFormat(buf, fplArrayCount(buf), hoverFormat, memoryIndex, u8);
					ImGui::SetTooltip(buf);
				}
			}

			memoryIndex++;
			memoryAddress++;
		}

		if (rowIndex < rowCount - 1)
			ImGui::NewLine();
	}

	ImGui::EndChild();

	ImGui::EndGroup();

	ImGui::End();
}

struct Emulator {
	fgbCPU cpu;
	fgbCartridge cartridge;
	UIRegisterState uiRegisterState;
	UIMemoryTableState uiRomMemoryState;
};

static Emulator _emulator = {
	.uiRomMemoryState {
		.columnCountIndex = memoryDefaultColumnIndex
	}
};

static void UpdateAndRender(const float deltaTime) {
	ImGuiIO &io = ImGui::GetIO();
	fplWindowSize windowArea = {};
	fplGetWindowSize(&windowArea);
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

	fplSetWindowCursorEnabled(!io.MouseDrawCursor);

	ImGui::NewFrame();

#if 0
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Open...");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#endif

	ImVec2 availableRegion = ImGui::GetWindowSize();

	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.75f), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	RenderMemory("ROM", _emulator.uiRomMemoryState, _emulator.cartridge.rom, _emulator.cartridge.size);

	//ImGui::SetNextWindowSize(ImVec2(150, 150), ImGuiSetCond_FirstUseEver);
	//ImGui::SetNextWindowPos(ImVec2(windowArea.width - 150, 20), ImGuiSetCond_FirstUseEver);
	//RenderRegister("Register", _emulator.uiRegisterState, _emulator.cpu.reg);

	glViewport(0, 0, windowArea.width, windowArea.height);
	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}

int main(int argc, char **argv) {
	int result = 0;
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("ImGUI Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = 1280;
	settings.window.windowSize.height = 720;
	settings.video.backend = fplVideoBackendType_OpenGL;

	if (!fplPlatformInit(fplInitFlags_Video, &settings)) {
		return -1;
	}

	if (!fglLoadOpenGL(true)) {
		fplPlatformRelease();
		return -2;
	}

	InitImGUI();

	ImGuiIO &io = ImGui::GetIO();

	fplTimestamp lastTime = fplTimestampQuery();
	fplTimestamp currentTime = fplZeroInit;
	float lastDeltaTime = 1.0f / 60.0f;

	while (fplWindowUpdate()) {
		fplEvent event;
		while (fplPollEvent(&event)) {
			switch (event.type) {
				case fplEventType_Keyboard:
				{
					switch (event.keyboard.type) {
						case fplKeyboardEventType_Button:
						{
							bool isDown = event.keyboard.buttonState >= fplButtonState_Press;
							ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, event.keyboard.modifiers, isDown);
						} break;
						case fplKeyboardEventType_Input:
						{
							if (event.keyboard.keyCode > 0 && event.keyboard.keyCode < 0x10000) {
								io.AddInputCharacter(ImWchar(event.keyboard.keyCode));
							}
						} break;
						default:
							break;
					}
				} break;
				case fplEventType_Mouse:
				{
					switch (event.mouse.type) {
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

	fglUnloadOpenGL();

	fplPlatformRelease();
	return 0;
}