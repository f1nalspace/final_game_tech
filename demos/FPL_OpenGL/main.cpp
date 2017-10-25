#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

#define MODERN_OPENGL 0

int main(int argc, char **args) {
	using namespace fpl;
	using namespace fpl::memory;
	using namespace fpl::window;
	int result = 0;
	InitSettings settings = InitSettings();
#if MODERN_OPENGL
	settings.video.profile = VideoCompabilityProfile::Any;
	settings.video.minMajor = 3;
	settings.video.minMinor = 1;
#else
	settings.video.profile = VideoCompabilityProfile::Legacy;
#endif
	if (InitPlatform(InitFlags::VideoOpenGL, settings)) {
		const char *version = (const char *)glGetString(GL_VERSION);
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		const char *renderer = (const char *)glGetString(GL_RENDERER);
		console::ConsoleFormatOut("OpenGL version: %s\n", version);
		console::ConsoleFormatOut("OpenGL vendor: %s\n", vendor);
		console::ConsoleFormatOut("OpenGL renderer: %s\n", renderer);

		glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
		while (WindowUpdate()) {
			WindowSize windowArea = GetWindowArea();
			glViewport(0, 0, windowArea.width, windowArea.height);
			glClear(GL_COLOR_BUFFER_BIT);
			glBegin(GL_TRIANGLES);
			glVertex2f(0.0f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();
			WindowFlip();
		}
		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}