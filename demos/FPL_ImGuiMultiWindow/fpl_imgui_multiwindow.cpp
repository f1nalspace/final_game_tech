/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | ImGui Multi Window

Description:
	Runs the docking version of Dear ImGui with multiple viewports on top of the additional windows of FPL.
	An ImGui window that is dragged out of the main window gets its own window from fplWindowCreate().
	All windows share the OpenGL context of the main window, the OpenGL2 backend of ImGui renders them.
	The platform backend for ImGui is part of this demo, see the struct FplPlatformBackend.
	Every additional window has the main window as parent (io.ConfigViewportsNoDefaultParent = false) and stays in front of it.
	The window of a destroyed viewport stays two more frames, so a window that is dragged back into the main window does not vanish for a moment.
	The demo window of ImGui shows the docking: drag a window by its title bar onto another window to dock it there,
	its menu Examples > Dockspace docks into the main window, its section Configuration has the options for docking and viewports.
	Parameters:
	  --frames=<n>      Quit by itself after this many frames, for tests
	  --no-viewports    Keep all ImGui windows inside the main window
	  --own-windows     Every ImGui window gets its own window, also inside the main window
	  --detach          Start with the window "Viewports" right of the main window, so it has its own window

Requirements:
	- C++ Compiler
	- Final Platform Layer
	- Dear ImGui v1.93.0-docking (Included in demo)

Author:
	Torsten Spaete

Changelog:
	## 2026-11-27
	- Initial version, duplicated from FPL_ImGui with the docking version of ImGui and its multiple viewports

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/
#ifndef FPL_IMPLEMENTATION
#	define FPL_IMPLEMENTATION
#endif
#ifndef FPL_LOGGING
#	define FPL_LOGGING
#endif
#define FPL_NO_VIDEO_VULKAN
#include <final_platform_layer.h>

#include <imgui.h>
#include <imgui_impl_opengl2.h>

// The demo clears the main window itself, everything else is drawn by the OpenGL2 backend of ImGui
#include <GL/gl.h>

#include <float.h> // FLT_MAX
#include <stdlib.h> // realloc, free, atoi

constexpr uint32_t MainWindowWidth = 1280;
constexpr uint32_t MainWindowHeight = 720;
constexpr size_t MaxDisplayCount = 16;
constexpr size_t MaxAudioDeviceCount = 16;
constexpr float DefaultDeltaTime = 1.0f / 60.0f;
constexpr int MouseButtonCount = ImGuiMouseButton_COUNT;
// Frames the window of a destroyed viewport stays, until the main window shows its content for sure, see FplPlatformBackend::RetireWindow()
constexpr int RetiredWindowFrameCount = 2;
constexpr const char *ViewportWindowTitle = "ImGui Viewport";

// Per viewport data of the FPL platform backend, ImGui keeps it in ImGuiViewport::PlatformUserData
struct FplViewportData {
	fplWindowHandle *window;
	// The main window belongs to the application, every other window was created by the backend
	bool isOwned;
	// FPL has no query for the focus, the window events keep it up to date
	bool isFocused;
	// Position and size events that the backend caused itself are ignored until the next frame, like the GLFW backend of ImGui does
	int ignorePositionEventFrame;
	int ignoreSizeEventFrame;
	// Viewport flags that the window has, ImGui changes them at runtime, e.g. with io.ConfigViewportsNoDecoration
	ImGuiViewportFlags windowFlags;

	static FplViewportData *Get(const ImGuiViewport *viewport) {
		FplViewportData *result = (FplViewportData *)viewport->PlatformUserData;
		return(result);
	}

	static FplViewportData *Make(fplWindowHandle *window, const bool isOwned) {
		FplViewportData *result = IM_NEW(FplViewportData)();
		result->window = window;
		result->isOwned = isOwned;
		result->isFocused = false;
		result->ignorePositionEventFrame = -1;
		result->ignoreSizeEventFrame = -1;
		result->windowFlags = 0;
		return(result);
	}
};

// Window of a destroyed viewport that waits for its destroy
struct FplRetiredWindow {
	fplWindowHandle *window;
	// Frame count of ImGui from which on the window is destroyed
	int destroyFrame;
};

// Platform backend of ImGui for FPL: input, clipboard, monitors and one FPL window per ImGui viewport
struct FplPlatformBackend {
	fplWindowHandle *mainWindow;
	fplWindowHandle *mouseWindow;
	ImVector<FplRetiredWindow> retiredWindows;
	// Frame in which the mouse counts as gone, zero when no leave is pending
	int mouseLeaveFrame;
	// One bit per mouse button that is held down
	uint32_t mouseButtonsDown;
	bool isViewportsEnabled;
	bool isCursorEnabled;
	char *clipboardBuffer;
	size_t clipboardBufferSize;
	fplDisplayInfo displays[MaxDisplayCount];
	size_t displayCount;

	static FplPlatformBackend *Get() {
		ImGuiIO &io = ImGui::GetIO();
		FplPlatformBackend *result = (FplPlatformBackend *)io.BackendPlatformUserData;
		return(result);
	}

	static ImGuiKey TranslateKey(const fplKey key) {
		if (key >= fplKey_0 && key <= fplKey_9) {
			int offset = (int)key - (int)fplKey_0;
			return (ImGuiKey)(ImGuiKey_0 + offset);
		}
		if (key >= fplKey_A && key <= fplKey_Z) {
			int offset = (int)key - (int)fplKey_A;
			return (ImGuiKey)(ImGuiKey_A + offset);
		}
		if (key >= fplKey_F1 && key <= fplKey_F24) {
			int offset = (int)key - (int)fplKey_F1;
			return (ImGuiKey)(ImGuiKey_F1 + offset);
		}
		if (key >= fplKey_NumPad0 && key <= fplKey_NumPad9) {
			int offset = (int)key - (int)fplKey_NumPad0;
			return (ImGuiKey)(ImGuiKey_Keypad0 + offset);
		}
		switch (key) {
			case fplKey_Tab: return ImGuiKey_Tab;
			case fplKey_Left: return ImGuiKey_LeftArrow;
			case fplKey_Right: return ImGuiKey_RightArrow;
			case fplKey_Up: return ImGuiKey_UpArrow;
			case fplKey_Down: return ImGuiKey_DownArrow;
			case fplKey_PageUp: return ImGuiKey_PageUp;
			case fplKey_PageDown: return ImGuiKey_PageDown;
			case fplKey_Home: return ImGuiKey_Home;
			case fplKey_End: return ImGuiKey_End;
			case fplKey_Insert: return ImGuiKey_Insert;
			case fplKey_Delete: return ImGuiKey_Delete;
			case fplKey_Backspace: return ImGuiKey_Backspace;
			case fplKey_Space: return ImGuiKey_Space;
			case fplKey_Return: return ImGuiKey_Enter;
			case fplKey_Escape: return ImGuiKey_Escape;
			case fplKey_Shift: return ImGuiKey_LeftShift;
			case fplKey_LeftShift: return ImGuiKey_LeftShift;
			case fplKey_RightShift: return ImGuiKey_RightShift;
			case fplKey_Control: return ImGuiKey_LeftCtrl;
			case fplKey_LeftControl: return ImGuiKey_LeftCtrl;
			case fplKey_RightControl: return ImGuiKey_RightCtrl;
			case fplKey_Alt: return ImGuiKey_LeftAlt;
			case fplKey_LeftAlt: return ImGuiKey_LeftAlt;
			case fplKey_RightAlt: return ImGuiKey_RightAlt;
			case fplKey_LeftSuper: return ImGuiKey_LeftSuper;
			case fplKey_RightSuper: return ImGuiKey_RightSuper;
			case fplKey_Apps: return ImGuiKey_Menu;
			case fplKey_CapsLock: return ImGuiKey_CapsLock;
			case fplKey_Scroll: return ImGuiKey_ScrollLock;
			case fplKey_NumLock: return ImGuiKey_NumLock;
			case fplKey_Snapshot: return ImGuiKey_PrintScreen;
			case fplKey_Pause: return ImGuiKey_Pause;
			case fplKey_Decimal: return ImGuiKey_KeypadDecimal;
			case fplKey_Divide: return ImGuiKey_KeypadDivide;
			case fplKey_Multiply: return ImGuiKey_KeypadMultiply;
			case fplKey_Substract: return ImGuiKey_KeypadSubtract;
			case fplKey_Add: return ImGuiKey_KeypadAdd;
			// The OEM keys follow the virtual key codes of Win32 for the US layout
			case fplKey_Oem1: return ImGuiKey_Semicolon;
			case fplKey_OemPlus: return ImGuiKey_Equal;
			case fplKey_OemComma: return ImGuiKey_Comma;
			case fplKey_OemMinus: return ImGuiKey_Minus;
			case fplKey_OemPeriod: return ImGuiKey_Period;
			case fplKey_Oem2: return ImGuiKey_Slash;
			case fplKey_Oem3: return ImGuiKey_GraveAccent;
			case fplKey_Oem4: return ImGuiKey_LeftBracket;
			case fplKey_Oem5: return ImGuiKey_Backslash;
			case fplKey_Oem6: return ImGuiKey_RightBracket;
			case fplKey_Oem7: return ImGuiKey_Apostrophe;
			default: return ImGuiKey_None;
		}
	}

	static const char *GetClipboardText(ImGuiContext *) {
		// ImGui expects the text to stay put until the next call, so the buffer is kept around and only grows
		FplPlatformBackend *backend = Get();
		size_t requiredLength = fplClipboardGetText(fpl_null, 0);
		if (requiredLength == 0) {
			return nullptr;
		}
		size_t requiredSize = requiredLength + 1;
		if (requiredSize > backend->clipboardBufferSize) {
			char *grownBuffer = (char *)realloc(backend->clipboardBuffer, requiredSize);
			if (grownBuffer == nullptr) {
				return nullptr;
			}
			backend->clipboardBuffer = grownBuffer;
			backend->clipboardBufferSize = requiredSize;
		}
		size_t copiedLength = fplClipboardGetText(backend->clipboardBuffer, backend->clipboardBufferSize);
		if (copiedLength == 0) {
			return nullptr;
		}
		return backend->clipboardBuffer;
	}

	static void SetClipboardText(ImGuiContext *, const char *text) {
		fplClipboardSetText(text);
	}

	// ImGui works in screen coordinates when the viewports are enabled, without them in the coordinates of the main window
	ImVec2 GetMousePosition(fplWindowHandle *window, const int32_t x, const int32_t y) const {
		ImVec2 result = ImVec2((float)x, (float)y);
		if (isViewportsEnabled && window != fpl_null) {
			fplWindowPosition windowPosition = fplZeroInit;
			if (fplWindowGetClientPositionMulti(window, &windowPosition)) {
				result.x += (float)windowPosition.left;
				result.y += (float)windowPosition.top;
			}
		}
		return(result);
	}

	void UpdateMonitors() {
		displayCount = fplGetDisplays(displays, MaxDisplayCount);
		ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
		platformIO.Monitors.resize(0);
		for (size_t displayIndex = 0; displayIndex < displayCount; ++displayIndex) {
			const fplDisplayInfo *display = displays + displayIndex;
			ImGuiPlatformMonitor monitor;
			monitor.MainPos = ImVec2((float)display->virtualPosition.left, (float)display->virtualPosition.top);
			monitor.MainSize = ImVec2((float)display->virtualSize.width, (float)display->virtualSize.height);
			// FPL does not know the work area without the task bars, so it is the whole display
			monitor.WorkPos = monitor.MainPos;
			monitor.WorkSize = monitor.MainSize;
			monitor.PlatformHandle = (void *)display;
			// ImGui falls back to the first monitor, so the primary display goes first
			if (display->isPrimary) {
				platformIO.Monitors.push_front(monitor);
			} else {
				platformIO.Monitors.push_back(monitor);
			}
		}
		if (platformIO.Monitors.Size == 0) {
			// ImGui needs at least one monitor for the viewports, the main window stands in
			fplWindowSize mainWindowSize = fplZeroInit;
			fplWindowGetSizeMulti(mainWindow, &mainWindowSize);
			ImGuiPlatformMonitor monitor;
			monitor.MainPos = ImVec2(0.0f, 0.0f);
			monitor.MainSize = ImVec2((float)mainWindowSize.width, (float)mainWindowSize.height);
			monitor.WorkPos = monitor.MainPos;
			monitor.WorkSize = monitor.MainSize;
			platformIO.Monitors.push_back(monitor);
		}
	}

	// The window of the parent viewport becomes the parent of the window, so it stays in front of it
	static void UpdateViewportParent(ImGuiViewport *viewport) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		if (viewportData == nullptr || !viewportData->isOwned || viewportData->window == fpl_null) {
			return;
		}
		fplWindowHandle *parentWindow = fpl_null;
		if (viewport->ParentViewportId != 0) {
			ImGuiViewport *parentViewport = ImGui::FindViewportByID(viewport->ParentViewportId);
			if (parentViewport != nullptr) {
				parentWindow = GetViewportWindow(parentViewport);
			}
		}
		fplWindowHandle *currentParentWindow = fplWindowGetParentMulti(viewportData->window);
		if (parentWindow != currentParentWindow) {
			fplWindowSetParentMulti(viewportData->window, parentWindow);
		}
	}

	static void CreateViewportWindow(ImGuiViewport *viewport) {
		const bool hasNoDecoration = (viewport->Flags & ImGuiViewportFlags_NoDecoration) != 0;
		const bool isTopMost = (viewport->Flags & ImGuiViewportFlags_TopMost) != 0;
		const bool hasNoFocusOnAppearing = (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing) != 0;

		fplWindowSettings settings;
		fplSetDefaultWindowSettings(&settings);
		fplCopyString(ViewportWindowTitle, settings.title, fplArrayCount(settings.title));
		settings.windowSize.width = (uint32_t)viewport->Size.x;
		settings.windowSize.height = (uint32_t)viewport->Size.y;
		settings.isDecorated = !hasNoDecoration;
		settings.isFloating = isTopMost;
		settings.isResizable = true;
		// ImGui shows the window with Platform_ShowWindow after it has set the position, the size and the title
		settings.initialVisibility = fplWindowVisibilityState_Hide;
		settings.isFocusedOnStart = !hasNoFocusOnAppearing;

		fplWindowHandle *window = fplWindowCreate(&settings);
		FplViewportData *viewportData = FplViewportData::Make(window, true);
		viewportData->windowFlags = viewport->Flags;
		viewport->PlatformUserData = viewportData;
		viewport->PlatformHandle = window;
		if (window == fpl_null) {
			fplConsoleFormatOut("Viewport %08X: failed creating a window\n", viewport->ID);
			return;
		}

		// The parent is set before the window is shown, some window managers read it only then
		UpdateViewportParent(viewport);
		const int32_t left = (int32_t)viewport->Pos.x;
		const int32_t top = (int32_t)viewport->Pos.y;
		viewportData->ignorePositionEventFrame = ImGui::GetFrameCount();
		fplWindowSetClientPositionMulti(window, left, top);
		fplConsoleFormatOut("Viewport %08X: created window at %d, %d with %u x %u\n", viewport->ID, left, top, settings.windowSize.width, settings.windowSize.height);
	}

	// The main window flips with V-Sync, so a compositor shows its frame up to a few refreshes later, but the destroy of a window takes effect at once.
	// A window that ImGui merges back into the main window would vanish for a moment, so the window of a destroyed viewport stays a few frames longer.
	void RetireWindow(fplWindowHandle *window) {
		const int frameCount = ImGui::GetFrameCount();
		FplRetiredWindow retiredWindow;
		retiredWindow.window = window;
		retiredWindow.destroyFrame = frameCount + RetiredWindowFrameCount;
		retiredWindows.push_back(retiredWindow);
	}

	void DestroyRetiredWindows(const bool isAll) {
		const int frameCount = ImGui::GetFrameCount();
		int keptCount = 0;
		for (int retiredIndex = 0; retiredIndex < retiredWindows.Size; ++retiredIndex) {
			const FplRetiredWindow retiredWindow = retiredWindows[retiredIndex];
			const bool isDue = isAll || frameCount >= retiredWindow.destroyFrame;
			if (!isDue) {
				retiredWindows[keptCount] = retiredWindow;
				++keptCount;
				continue;
			}
			if (mouseWindow == retiredWindow.window) {
				mouseWindow = fpl_null;
			}
			fplWindowDestroy(retiredWindow.window);
			fplConsoleFormatOut("Destroyed retired window\n");
		}
		retiredWindows.resize(keptCount);
	}

	static void DestroyViewportWindow(ImGuiViewport *viewport) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		if (viewportData != nullptr) {
			if (viewportData->isOwned && viewportData->window != fpl_null) {
				FplPlatformBackend *backend = Get();
				backend->RetireWindow(viewportData->window);
				fplConsoleFormatOut("Viewport %08X: retired window\n", viewport->ID);
			}
			viewportData->window = fpl_null;
			IM_DELETE(viewportData);
		}
		viewport->PlatformUserData = nullptr;
		viewport->PlatformHandle = nullptr;
	}

	static fplWindowHandle *GetViewportWindow(const ImGuiViewport *viewport) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		fplWindowHandle *result = viewportData != nullptr ? viewportData->window : fpl_null;
		return(result);
	}

	static void ShowViewportWindow(ImGuiViewport *viewport) {
		fplWindowHandle *window = GetViewportWindow(viewport);
		if (window != fpl_null) {
			fplWindowSetVisibilityMulti(window, fplWindowVisibilityState_Show);
		}
	}

	static void SetViewportPosition(ImGuiViewport *viewport, ImVec2 position) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		if (viewportData != nullptr && viewportData->window != fpl_null) {
			viewportData->ignorePositionEventFrame = ImGui::GetFrameCount();
			fplWindowSetClientPositionMulti(viewportData->window, (int32_t)position.x, (int32_t)position.y);
		}
	}

	static ImVec2 GetViewportPosition(ImGuiViewport *viewport) {
		ImVec2 result = ImVec2(0.0f, 0.0f);
		fplWindowHandle *window = GetViewportWindow(viewport);
		fplWindowPosition windowPosition = fplZeroInit;
		if (window != fpl_null && fplWindowGetClientPositionMulti(window, &windowPosition)) {
			result = ImVec2((float)windowPosition.left, (float)windowPosition.top);
		}
		return(result);
	}

	static void SetViewportSize(ImGuiViewport *viewport, ImVec2 size) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		if (viewportData != nullptr && viewportData->window != fpl_null) {
			viewportData->ignoreSizeEventFrame = ImGui::GetFrameCount();
			fplWindowSetSizeMulti(viewportData->window, (uint32_t)size.x, (uint32_t)size.y);
		}
	}

	static ImVec2 GetViewportSize(ImGuiViewport *viewport) {
		ImVec2 result = ImVec2(0.0f, 0.0f);
		fplWindowHandle *window = GetViewportWindow(viewport);
		fplWindowSize windowSize = fplZeroInit;
		if (window != fpl_null && fplWindowGetSizeMulti(window, &windowSize)) {
			result = ImVec2((float)windowSize.width, (float)windowSize.height);
		}
		return(result);
	}

	static bool GetViewportFocus(ImGuiViewport *viewport) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		bool result = viewportData != nullptr && viewportData->isFocused;
		return(result);
	}

	static bool GetViewportMinimized(ImGuiViewport *viewport) {
		fplWindowHandle *window = GetViewportWindow(viewport);
		if (window == fpl_null) {
			return false;
		}
		fplWindowState windowState = fplWindowGetStateMulti(window);
		bool result = windowState == fplWindowState_Iconify;
		return(result);
	}

	static void SetViewportTitle(ImGuiViewport *viewport, const char *title) {
		fplWindowHandle *window = GetViewportWindow(viewport);
		if (window != fpl_null) {
			fplWindowSetTitleMulti(window, title);
		}
	}

	static void UpdateViewportWindow(ImGuiViewport *viewport) {
		FplViewportData *viewportData = FplViewportData::Get(viewport);
		if (viewportData == nullptr || !viewportData->isOwned || viewportData->window == fpl_null) {
			return;
		}
		// ImGui changes the parent viewport at runtime, e.g. with io.ConfigViewportsNoDefaultParent
		UpdateViewportParent(viewport);

		// FPL has no taskbar icon for a window, so ImGuiViewportFlags_NoTaskBarIcon has no effect
		const ImGuiViewportFlags styleFlags = ImGuiViewportFlags_NoDecoration | ImGuiViewportFlags_TopMost;
		const ImGuiViewportFlags changedFlags = (viewport->Flags ^ viewportData->windowFlags) & styleFlags;
		if (changedFlags == 0) {
			return;
		}
		fplWindowHandle *window = viewportData->window;
		const bool hasNoDecoration = (viewport->Flags & ImGuiViewportFlags_NoDecoration) != 0;
		const bool isTopMost = (viewport->Flags & ImGuiViewportFlags_TopMost) != 0;
		if (changedFlags & ImGuiViewportFlags_NoDecoration) {
			fplWindowSetDecoratedMulti(window, !hasNoDecoration);
		}
		if (changedFlags & ImGuiViewportFlags_TopMost) {
			fplWindowSetFloatingMulti(window, isTopMost);
		}
		viewportData->windowFlags = viewport->Flags;

		// A frame that comes or goes moves the client area, so it is put back where ImGui has the viewport
		const int frameCount = ImGui::GetFrameCount();
		const int32_t left = (int32_t)viewport->Pos.x;
		const int32_t top = (int32_t)viewport->Pos.y;
		const uint32_t width = (uint32_t)viewport->Size.x;
		const uint32_t height = (uint32_t)viewport->Size.y;
		viewportData->ignorePositionEventFrame = frameCount;
		viewportData->ignoreSizeEventFrame = frameCount;
		fplWindowSetSizeMulti(window, width, height);
		fplWindowSetClientPositionMulti(window, left, top);
		const char *decorationText = hasNoDecoration ? "without" : "with";
		fplConsoleFormatOut("Viewport %08X: window %s decoration, top most %d\n", viewport->ID, decorationText, isTopMost);
	}

	static void RenderViewportWindow(ImGuiViewport *viewport, void *) {
		// All windows share the OpenGL context of the main window, so the window only becomes the target
		fplWindowHandle *window = GetViewportWindow(viewport);
		if (window != fpl_null) {
			fplVideoMakeCurrentMulti(window);
		}
	}

	static void SwapViewportBuffers(ImGuiViewport *viewport, void *) {
		fplWindowHandle *window = GetViewportWindow(viewport);
		if (window != fpl_null) {
			fplVideoFlipMulti(window);
		}
	}

	bool Init(fplWindowHandle *window, const bool enableViewports) {
		mainWindow = window;
		mouseWindow = fpl_null;
		mouseLeaveFrame = 0;
		mouseButtonsDown = 0;
		isViewportsEnabled = enableViewports;
		isCursorEnabled = true;
		clipboardBuffer = nullptr;
		clipboardBufferSize = 0;
		displayCount = 0;

		ImGuiIO &io = ImGui::GetIO();
		io.BackendPlatformUserData = this;
		io.BackendPlatformName = "imgui_impl_fpl";
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags |= ImGuiBackendFlags_HasParentViewport;

		ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_GetClipboardTextFn = GetClipboardText;
		platformIO.Platform_SetClipboardTextFn = SetClipboardText;

		platformIO.Platform_CreateWindow = CreateViewportWindow;
		platformIO.Platform_DestroyWindow = DestroyViewportWindow;
		platformIO.Platform_ShowWindow = ShowViewportWindow;
		platformIO.Platform_SetWindowPos = SetViewportPosition;
		platformIO.Platform_GetWindowPos = GetViewportPosition;
		platformIO.Platform_SetWindowSize = SetViewportSize;
		platformIO.Platform_GetWindowSize = GetViewportSize;
		platformIO.Platform_GetWindowFocus = GetViewportFocus;
		platformIO.Platform_GetWindowMinimized = GetViewportMinimized;
		platformIO.Platform_SetWindowTitle = SetViewportTitle;
		platformIO.Platform_UpdateWindow = UpdateViewportWindow;
		platformIO.Platform_RenderWindow = RenderViewportWindow;
		platformIO.Platform_SwapBuffers = SwapViewportBuffers;
		// Platform_SetWindowFocus stays empty, FPL cannot move a window to the front and focus it

		UpdateMonitors();

		// The main window belongs to the application, it is registered so that the events find their viewport the same way
		ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		FplViewportData *mainViewportData = FplViewportData::Make(mainWindow, false);
		mainViewport->PlatformUserData = mainViewportData;
		mainViewport->PlatformHandle = mainWindow;
		return(true);
	}

	void Shutdown() {
		ImGui::DestroyPlatformWindows();
		const bool isAllRetiredWindows = true;
		DestroyRetiredWindows(isAllRetiredWindows);
		retiredWindows.clear();

		ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		FplViewportData *mainViewportData = FplViewportData::Get(mainViewport);
		if (mainViewportData != nullptr) {
			IM_DELETE(mainViewportData);
		}
		mainViewport->PlatformUserData = nullptr;
		mainViewport->PlatformHandle = nullptr;

		free(clipboardBuffer);
		clipboardBuffer = nullptr;
		clipboardBufferSize = 0;

		ImGuiIO &io = ImGui::GetIO();
		ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
		platformIO.ClearPlatformHandlers();
		io.BackendPlatformName = nullptr;
		io.BackendPlatformUserData = nullptr;
		io.BackendFlags &= ~ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags &= ~ImGuiBackendFlags_HasParentViewport;
	}

	void NewFrame(const float deltaTime) {
		ImGuiIO &io = ImGui::GetIO();
		fplWindowSize mainWindowSize = fplZeroInit;
		fplWindowGetSizeMulti(mainWindow, &mainWindowSize);
		io.DisplaySize = ImVec2((float)mainWindowSize.width, (float)mainWindowSize.height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.DeltaTime = deltaTime > 0.0f ? deltaTime : DefaultDeltaTime;

		const bool isAllRetiredWindows = false;
		DestroyRetiredWindows(isAllRetiredWindows);

		// A leave takes effect one frame later and only without a held button, see HandleMouseEvent()
		const int frameCount = ImGui::GetFrameCount();
		const bool isMouseGone = mouseLeaveFrame != 0 && frameCount >= mouseLeaveFrame && mouseButtonsDown == 0;
		if (isMouseGone) {
			mouseWindow = fpl_null;
			mouseLeaveFrame = 0;
			io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
		}

		// ImGui may draw the cursor itself, then the cursor of the system is hidden
		const bool wantsCursor = !io.MouseDrawCursor;
		if (wantsCursor != isCursorEnabled) {
			fplWindowSetCursorEnabledMulti(mainWindow, wantsCursor);
			isCursorEnabled = wantsCursor;
		}
	}

	void HandleWindowEvent(const fplEvent &ev) {
		ImGuiViewport *viewport = ImGui::FindViewportByPlatformHandle(ev.windowHandle);
		FplViewportData *viewportData = viewport != nullptr ? FplViewportData::Get(viewport) : nullptr;
		if (viewportData == nullptr) {
			return;
		}
		ImGuiIO &io = ImGui::GetIO();
		const int frameCount = ImGui::GetFrameCount();
		switch (ev.window.type) {
			case fplWindowEventType_GotFocus:
			{
				viewportData->isFocused = true;
				io.AddFocusEvent(true);
			} break;

			case fplWindowEventType_LostFocus:
			{
				viewportData->isFocused = false;
				io.AddFocusEvent(false);
			} break;

			case fplWindowEventType_PositionChanged:
			{
				const bool isCausedByBackend = frameCount <= viewportData->ignorePositionEventFrame + 1;
				if (!isCausedByBackend) {
					viewport->PlatformRequestMove = true;
				}
			} break;

			case fplWindowEventType_Resized:
			{
				const bool isCausedByBackend = frameCount <= viewportData->ignoreSizeEventFrame + 1;
				if (!isCausedByBackend) {
					viewport->PlatformRequestResize = true;
				}
			} break;

			case fplWindowEventType_Closed:
			{
				// Closing the main window ends the application in fplWindowUpdate(), an additional window is closed by ImGui
				if (viewportData->isOwned) {
					viewport->PlatformRequestClose = true;
				}
			} break;

			default:
				break;
		}
	}

	void HandleKeyboardEvent(const fplKeyboardEvent &keyboard) {
		ImGuiIO &io = ImGui::GetIO();
		switch (keyboard.type) {
			case fplKeyboardEventType_Button:
			{
				const fplKeyboardModifierFlags modifiers = keyboard.modifiers;
				const bool isCtrlDown = (modifiers & (fplKeyboardModifierFlags_LCtrl | fplKeyboardModifierFlags_RCtrl)) != 0;
				const bool isShiftDown = (modifiers & (fplKeyboardModifierFlags_LShift | fplKeyboardModifierFlags_RShift)) != 0;
				const bool isAltDown = (modifiers & (fplKeyboardModifierFlags_LAlt | fplKeyboardModifierFlags_RAlt)) != 0;
				const bool isSuperDown = (modifiers & (fplKeyboardModifierFlags_LSuper | fplKeyboardModifierFlags_RSuper)) != 0;
				io.AddKeyEvent(ImGuiMod_Ctrl, isCtrlDown);
				io.AddKeyEvent(ImGuiMod_Shift, isShiftDown);
				io.AddKeyEvent(ImGuiMod_Alt, isAltDown);
				io.AddKeyEvent(ImGuiMod_Super, isSuperDown);

				const ImGuiKey key = TranslateKey(keyboard.mappedKey);
				if (key != ImGuiKey_None) {
					const bool isDown = keyboard.buttonState >= fplButtonState_Press;
					io.AddKeyEvent(key, isDown);
				}
			} break;

			case fplKeyboardEventType_Input:
			{
				if (keyboard.keyCode > 0) {
					io.AddInputCharacter((unsigned int)keyboard.keyCode);
				}
			} break;

			default:
				break;
		}
	}

	void HandleMouseEvent(const fplEvent &ev) {
		ImGuiIO &io = ImGui::GetIO();
		const fplMouseEvent &mouse = ev.mouse;
		switch (mouse.type) {
			case fplMouseEventType_Move:
			{
				mouseWindow = ev.windowHandle;
				mouseLeaveFrame = 0;
				ImVec2 position = GetMousePosition(ev.windowHandle, mouse.mouseX, mouse.mouseY);
				io.AddMousePosEvent(position.x, position.y);
			} break;

			case fplMouseEventType_Button:
			{
				ImVec2 position = GetMousePosition(ev.windowHandle, mouse.mouseX, mouse.mouseY);
				io.AddMousePosEvent(position.x, position.y);
				// ImGui knows left, right, middle and the two side buttons, in the same order as FPL
				const int mouseButtonIndex = (int)mouse.mouseButton;
				if (mouseButtonIndex >= 0 && mouseButtonIndex < MouseButtonCount) {
					const bool isDown = mouse.buttonState >= fplButtonState_Press;
					const uint32_t mouseButtonBit = 1u << mouseButtonIndex;
					if (isDown) {
						mouseButtonsDown |= mouseButtonBit;
					} else {
						mouseButtonsDown &= ~mouseButtonBit;
					}
					io.AddMouseButtonEvent(mouseButtonIndex, isDown);
				}
			} break;

			case fplMouseEventType_Wheel:
			{
				io.AddMouseWheelEvent(0.0f, mouse.wheelDelta);
			} break;

			case fplMouseEventType_HorizontalWheel:
			{
				// FPL counts to the right as positive, ImGui to the left
				io.AddMouseWheelEvent(-mouse.wheelDelta, 0.0f);
			} break;

			case fplMouseEventType_Enter:
			{
				mouseWindow = ev.windowHandle;
				mouseLeaveFrame = 0;
			} break;

			case fplMouseEventType_Leave:
			{
				// A window that ImGui creates under the mouse while it drags gets the mouse from the window it came from, but the drag goes on there with the held button.
				// So the leave waits for the next frame and for all buttons to be up, an enter or move until then cancels it (like the SDL backend of ImGui).
				if (mouseWindow == ev.windowHandle) {
					const int frameCount = ImGui::GetFrameCount();
					mouseLeaveFrame = frameCount + 1;
				}
			} break;

			default:
				break;
		}
	}

	void HandleEvent(const fplEvent &ev) {
		switch (ev.type) {
			case fplEventType_Window:
				HandleWindowEvent(ev);
				break;
			case fplEventType_Keyboard:
				HandleKeyboardEvent(ev.keyboard);
				break;
			case fplEventType_Mouse:
				HandleMouseEvent(ev);
				break;
			default:
				break;
		}
	}
};

struct DemoOptions {
	int maxFrameCount;
	bool isViewportsEnabled;
	bool isOwnWindowForEach;
	bool isDetached;

	static DemoOptions Parse(const int argCount, char **args) {
		DemoOptions result = fplZeroInit;
		result.maxFrameCount = 0;
		result.isViewportsEnabled = true;
		const char framesPrefix[] = "--frames=";
		const size_t framesPrefixLength = sizeof(framesPrefix) - 1;
		for (int argIndex = 1; argIndex < argCount; ++argIndex) {
			const char *arg = args[argIndex];
			const size_t argLength = fplGetStringLength(arg);
			if (argLength > framesPrefixLength && fplIsStringEqualLen(arg, framesPrefixLength, framesPrefix, framesPrefixLength)) {
				result.maxFrameCount = atoi(arg + framesPrefixLength);
			} else if (fplIsStringEqual(arg, "--no-viewports")) {
				result.isViewportsEnabled = false;
			} else if (fplIsStringEqual(arg, "--own-windows")) {
				result.isOwnWindowForEach = true;
			} else if (fplIsStringEqual(arg, "--detach")) {
				result.isDetached = true;
			}
		}
		return(result);
	}
};

struct DemoState {
	FplPlatformBackend platform;
	DemoOptions options;
	ImVec4 clearColor;
	bool showDemoWindow;
	bool showAnotherWindow;
	bool showDisplaysWindow;
	bool showAudioDevicesWindow;
	bool showViewportsWindow;
	float sliderValue;
	size_t audioDeviceCount;
	fplAudioDeviceInfo audioDevices[MaxAudioDeviceCount];

	void ShowMainControls() {
		constexpr float controlsLeft = 20.0f;
		constexpr float controlsTop = 40.0f;
		const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		ImVec2 controlsPosition = ImVec2(mainViewport->WorkPos.x + controlsLeft, mainViewport->WorkPos.y + controlsTop);
		ImGui::SetNextWindowPos(controlsPosition, ImGuiCond_FirstUseEver);
		ImGui::Begin("Hello, world!");
		ImGui::SliderFloat("float", &sliderValue, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float *)&clearColor);
		ImGui::Checkbox("Demo Window", &showDemoWindow);
		ImGui::Checkbox("Another Window", &showAnotherWindow);
		ImGui::Checkbox("Displays", &showDisplaysWindow);
		ImGui::Checkbox("Audio Devices", &showAudioDevicesWindow);
		ImGui::Checkbox("Viewports", &showViewportsWindow);
		ImGui::TextUnformatted("Docking: drag a window by its title bar onto another window.");
		ImGui::TextUnformatted("Demo window > Examples > Dockspace docks into the main window.");
		if (ImGui::Button("Toggle app floating")) {
			bool isFloating = fplIsWindowFloating();
			fplSetWindowFloating(!isFloating);
		}
		if (ImGui::Button("Toggle app decorated")) {
			bool isDecorated = fplIsWindowDecorated();
			fplSetWindowDecorated(!isDecorated);
		}
		if (ImGui::Button("Toggle app resizable")) {
			bool isResizable = fplIsWindowResizable();
			fplSetWindowResizeable(!isResizable);
		}
		ImGuiIO &io = ImGui::GetIO();
		if (options.isViewportsEnabled) {
			ImGui::Checkbox("Every ImGui window in its own window", &io.ConfigViewportsNoAutoMerge);
		}
		const float framerate = io.Framerate;
		const float frameMilliseconds = framerate > 0.0f ? 1000.0f / framerate : 0.0f;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", frameMilliseconds, framerate);
		ImGui::End();
	}

	void ShowAnotherWindow() {
		constexpr float anotherWidth = 200.0f;
		constexpr float anotherHeight = 100.0f;
		const ImVec2 anotherSize = ImVec2(anotherWidth, anotherHeight);
		ImGui::SetNextWindowSize(anotherSize, ImGuiCond_FirstUseEver);
		ImGui::Begin("Another Window", &showAnotherWindow);
		ImGui::Text("Hello");
		ImGui::End();
	}

	void ShowDisplaysWindow() {
		constexpr float displaysLeft = 60.0f;
		constexpr float displaysTop = 480.0f;
		constexpr float displaysWidth = 600.0f;
		constexpr float displaysHeight = 200.0f;
		const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		ImVec2 displaysPosition = ImVec2(mainViewport->WorkPos.x + displaysLeft, mainViewport->WorkPos.y + displaysTop);
		ImGui::SetNextWindowPos(displaysPosition, ImGuiCond_FirstUseEver);
		const ImVec2 displaysSize = ImVec2(displaysWidth, displaysHeight);
		ImGui::SetNextWindowSize(displaysSize, ImGuiCond_FirstUseEver);
		ImGui::Begin("Displays", &showDisplaysWindow);
		const size_t displayCount = platform.displayCount;
		ImGui::Text("Count: %zu", displayCount);
		for (size_t displayIndex = 0; displayIndex < displayCount; ++displayIndex) {
			const fplDisplayInfo *display = platform.displays + displayIndex;
			const char *primaryText = display->isPrimary ? " [Primary]" : "";
			ImGui::BulletText("Display[%zu]: %s, Pos: %d x %d, Size: %d x %d%s", displayIndex, display->id, display->virtualPosition.left, display->virtualPosition.top, display->virtualSize.width, display->virtualSize.height, primaryText);
		}
		fplDisplayInfo windowDisplay = fplZeroInit;
		fplGetWindowDisplay(&windowDisplay);
		const char *isPrimaryText = windowDisplay.isPrimary ? "true" : "false";
		ImGui::Text("Window Display: %s, Pos: %d x %d, Size: %d x %d, Is primary: %s", windowDisplay.id, windowDisplay.virtualPosition.left, windowDisplay.virtualPosition.top, windowDisplay.virtualSize.width, windowDisplay.virtualSize.height, isPrimaryText);
		ImGui::End();
	}

	void ShowAudioDevicesWindow() {
		constexpr float audioLeft = 560.0f;
		constexpr float audioTop = 480.0f;
		constexpr float audioWidth = 600.0f;
		constexpr float audioHeight = 200.0f;
		const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		ImVec2 audioPosition = ImVec2(mainViewport->WorkPos.x + audioLeft, mainViewport->WorkPos.y + audioTop);
		ImGui::SetNextWindowPos(audioPosition, ImGuiCond_FirstUseEver);
		const ImVec2 audioSize = ImVec2(audioWidth, audioHeight);
		ImGui::SetNextWindowSize(audioSize, ImGuiCond_FirstUseEver);
		ImGui::Begin("Audio Devices", &showAudioDevicesWindow);
		ImGui::Text("Count: %zu", audioDeviceCount);
		for (size_t deviceIndex = 0; deviceIndex < audioDeviceCount; ++deviceIndex) {
			const fplAudioDeviceInfo *info = audioDevices + deviceIndex;
			const char *defaultText = info->isDefault ? " [Default]" : "";
			ImGui::BulletText("Audio Device[%zu]: %s%s", deviceIndex, info->name, defaultText);
		}
		ImGui::End();
	}

	void ShowViewportsWindow() {
		constexpr float viewportsGap = 20.0f;
		constexpr float viewportsInsideLeft = 700.0f;
		constexpr float viewportsInsideTop = 40.0f;
		constexpr float viewportsWidth = 300.0f;
		constexpr float viewportsHeight = 260.0f;
		const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
		ImVec2 viewportsPosition;
		if (options.isDetached) {
			// Right of the main window, so ImGui gives the window its own window from the start
			viewportsPosition = ImVec2(mainViewport->Pos.x + mainViewport->Size.x + viewportsGap, mainViewport->Pos.y);
		} else {
			viewportsPosition = ImVec2(mainViewport->WorkPos.x + viewportsInsideLeft, mainViewport->WorkPos.y + viewportsInsideTop);
		}
		ImGui::SetNextWindowPos(viewportsPosition, ImGuiCond_FirstUseEver);
		const ImVec2 viewportsSize = ImVec2(viewportsWidth, viewportsHeight);
		ImGui::SetNextWindowSize(viewportsSize, ImGuiCond_FirstUseEver);
		ImGui::Begin("Viewports", &showViewportsWindow);
		ImGuiPlatformIO &platformIO = ImGui::GetPlatformIO();
		ImGui::Text("Viewports: %d", platformIO.Viewports.Size);
		for (int viewportIndex = 0; viewportIndex < platformIO.Viewports.Size; ++viewportIndex) {
			const ImGuiViewport *viewport = platformIO.Viewports[viewportIndex];
			const FplViewportData *viewportData = FplViewportData::Get(viewport);
			const bool isFocused = viewportData != nullptr && viewportData->isFocused;
			const char *kindText = viewport == mainViewport ? "main" : "additional";
			const char *focusText = isFocused ? ", focused" : "";
			ImGui::BulletText("%08X %s, Pos: %.0f x %.0f, Size: %.0f x %.0f%s", viewport->ID, kindText, viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y, focusText);
		}
		const ImVec2 mousePosition = ImGui::GetMousePos();
		ImGui::Text("Mouse: %.0f x %.0f", mousePosition.x, mousePosition.y);
		ImGui::End();
	}

	void UpdateAndRender(const float deltaTime) {
		ImGui_ImplOpenGL2_NewFrame();
		platform.NewFrame(deltaTime);
		ImGui::NewFrame();

		// The demo window comes first like in the examples of ImGui, its Examples > Dockspace must be submitted before the other windows.
		// The demo has no dockspace of its own, a second one over the main window would have the same ID as the one of Examples > Dockspace.
		if (showDemoWindow) {
			ImGui::ShowDemoWindow(&showDemoWindow);
		}
		ShowMainControls();
		if (showAnotherWindow) {
			ShowAnotherWindow();
		}
		if (showDisplaysWindow) {
			ShowDisplaysWindow();
		}
		if (showAudioDevicesWindow) {
			ShowAudioDevicesWindow();
		}
		if (showViewportsWindow) {
			ShowViewportsWindow();
		}

		ImGui::Render();

		ImGuiIO &io = ImGui::GetIO();
		const GLsizei viewportWidth = (GLsizei)io.DisplaySize.x;
		const GLsizei viewportHeight = (GLsizei)io.DisplaySize.y;
		glViewport(0, 0, viewportWidth, viewportHeight);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImDrawData *drawData = ImGui::GetDrawData();
		ImGui_ImplOpenGL2_RenderDrawData(drawData);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			// Creates, moves and destroys the additional windows, then renders and flips each of them
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			fplVideoMakeCurrentMulti(platform.mainWindow);
		}
	}
};

int main(int argc, char **args) {
	static DemoState state = {};
	state.options = DemoOptions::Parse(argc, args);
	state.clearColor = ImColor(114, 144, 154);
	state.showDemoWindow = true;
	state.showDisplaysWindow = true;
	state.showAudioDevicesWindow = true;
	state.showViewportsWindow = true;

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("ImGui Multi Window Example", settings.window.title, fplArrayCount(settings.window.title));
	settings.window.windowSize.width = MainWindowWidth;
	settings.window.windowSize.height = MainWindowHeight;
	settings.video.backend = fplVideoBackendType_OpenGL;
	settings.video.graphics.opengl.compatibilityFlags = fplOpenGLCompatibilityFlags_Legacy;
	settings.video.isVSync = true;
	if (!fplPlatformInit(fplInitFlags_All, &settings)) {
		return(-1);
	}

	state.audioDeviceCount = fplGetAudioDevices(MaxAudioDeviceCount, sizeof(state.audioDevices[0]), state.audioDevices);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	if (state.options.isViewportsEnabled) {
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	}
	io.ConfigViewportsNoAutoMerge = state.options.isOwnWindowForEach;
	// The main window is the parent of every additional window, ImGui makes them top-level windows by default.
	// KWin would put a window that ImGui creates while dragging (shown without the focus) behind the active main window.
	io.ConfigViewportsNoDefaultParent = false;

	ImGui::StyleColorsDark();
	if (state.options.isViewportsEnabled) {
		// The additional windows look the same as the ImGui windows inside the main window
		ImGuiStyle &style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	fplWindowHandle *mainWindow = fplWindowGetMain();
	state.platform.Init(mainWindow, state.options.isViewportsEnabled);
	ImGui_ImplOpenGL2_Init();

	fplTimestamp lastTime = fplTimestampQuery();
	float lastDeltaTime = DefaultDeltaTime;
	int frameCount = 0;
	while (fplWindowUpdate()) {
		fplEvent ev;
		while (fplPollEvent(&ev)) {
			state.platform.HandleEvent(ev);
		}
		// On Win32 the main window is destroyed with its close already, ImGui would see a main window of zero size and give every ImGui window its own window
		if (!fplIsWindowRunning()) {
			break;
		}

		state.UpdateAndRender(lastDeltaTime);

		fplVideoFlip();

		fplTimestamp currentTime = fplTimestampQuery();
		double elapsedTime = fplTimestampElapsed(lastTime, currentTime);
		lastDeltaTime = elapsedTime > 0.0 ? (float)elapsedTime : DefaultDeltaTime;
		lastTime = currentTime;

		++frameCount;
		if (state.options.maxFrameCount > 0 && frameCount >= state.options.maxFrameCount) {
			break;
		}
	}

	ImGui_ImplOpenGL2_Shutdown();
	state.platform.Shutdown();
	ImGui::DestroyContext();

	fplPlatformRelease();
	return(0);
}
