struct IUnknown; // Stupid MSVC
#include <Windows.h>

#include "GL/GL.h"

struct fpl__Win32 {
	LPCWSTR className;
	void* mainFiber;
	void* messageFiber;
	HWND windowHandle;
	HDC devicContext;
	HGLRC renderingContext;
	int quit;
};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	fpl__Win32* state = (fpl__Win32*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	switch (message) {
	case WM_SIZE:
		break;
	case WM_DESTROY:
		state->quit = true;
		break;
	case WM_TIMER:
		SwitchToFiber(state->mainFiber);
		break;
	default:
		result = DefWindowProcA(hwnd, message, wParam, lParam);
	}
	return(result);
}

static void CALLBACK WindowMessageFiberProc(fpl__Win32* state) {
	SetTimer(state->windowHandle, 1, 1, 0);
	for (;;) {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		SwitchToFiber(state->mainFiber);
	}
}

static bool CreateOpenGL(fpl__Win32 *state) {
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	int pixel_format = ChoosePixelFormat(state->devicContext, &pfd);
	if (!pixel_format) {
		return false;
	}
	if (!DescribePixelFormat(state->devicContext, pixel_format, sizeof(pfd), &pfd)) {
		return false;
	}
	if (!SetPixelFormat(state->devicContext, pixel_format, &pfd)) {
		return false;
	}
	state->renderingContext = wglCreateContext(state->devicContext);
	if (!state->renderingContext) {
		return false;
	}
	wglMakeCurrent(state->devicContext, state->renderingContext);
	return(true);
}

static void DestroyOpenGL(fpl__Win32* state) {
	wglMakeCurrent(state->devicContext, nullptr);
	wglDeleteContext(state->renderingContext);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	fpl__Win32 state = {};

	int userWidth = 1280 / 2;
	int userHeight = 720 / 2;

	state.mainFiber = ConvertThreadToFiber(0);
	if (state.mainFiber == nullptr) {
		return(-1);
	}

	state.messageFiber = CreateFiber(0, (PFIBER_START_ROUTINE)WindowMessageFiberProc, &state);
	if (state.messageFiber == nullptr) {
		return(-1);
	}

	WNDCLASSW windowClass = {};
	windowClass.lpfnWndProc = WindowProc;
	windowClass.lpszClassName = L"MyWindow";
	windowClass.hInstance = hInstance;
	windowClass.style = CS_HREDRAW | CS_HREDRAW;
	if (RegisterClassW(&windowClass) == 0) {
		return(-1);
	}

	state.className = windowClass.lpszClassName;

	int windowWidth;
	int windowHeight;
	if (userWidth)
		windowWidth = userWidth;
	else
		windowWidth = CW_USEDEFAULT;
	if (userHeight)
		windowHeight = userHeight;
	else
		windowHeight = CW_USEDEFAULT;

	if (windowWidth != CW_USEDEFAULT && windowHeight != CW_USEDEFAULT) {
		RECT windowRect;
		windowRect.left = 0;
		windowRect.right = windowWidth;
		windowRect.top = 0;
		windowRect.bottom = windowHeight;
		if (AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0)) {
			windowWidth = windowRect.right - windowRect.left;
			windowHeight = windowRect.bottom - windowRect.top;
		}
	}

	state.windowHandle = CreateWindowExW(
		0,
		windowClass.lpszClassName,
		L"Win32",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		NULL,
		NULL,
		hInstance,
		0);
	if (state.windowHandle == INVALID_HANDLE_VALUE) {
		return(-1);
	}

	SetWindowLongPtr(state.windowHandle, GWLP_USERDATA, (LONG_PTR)&state);

	ShowWindow(state.windowHandle, SW_SHOW);

	state.devicContext = GetDC(state.windowHandle);

	if (CreateOpenGL(&state)) {
		glClearColor(0.1f, 0.3f, 0.8f, 1.0f);
		glMatrixMode(GL_MODELVIEW);

		while (!state.quit) {
			SwitchToFiber(state.messageFiber);

			RECT clientRect;
			GetClientRect(state.windowHandle, &clientRect);

			int w = clientRect.right - clientRect.left;
			int h = clientRect.bottom - clientRect.top;

			glViewport(0, 0, w, h);

			glLoadIdentity();

			glClear(GL_COLOR_BUFFER_BIT);

			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_QUADS);
			glVertex2f(0.5f, 0.5f);
			glVertex2f(-0.5f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();

			SwapBuffers(state.devicContext);
		}
		DestroyOpenGL(&state);
	}

	DestroyWindow(state.windowHandle);
	UnregisterClassW(windowClass.lpszClassName, hInstance);

	return(0);
}