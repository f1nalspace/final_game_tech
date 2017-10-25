#define GLEW_STATIC
#include <GL\glew.h>

#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

#define MODERN_OPENGL 1

using namespace fpl;
using namespace fpl::memory;
using namespace fpl::window;

static void RunLegacy() {
	console::ConsoleOut("Running legacy opengl\n");

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
}

static GLuint CreateShaderType(GLenum type, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, nullptr);
	glCompileShader(shaderId);

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (compileResult == GL_FALSE) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);

		char *info = (char *)alloca(infoLen);
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		console::ConsoleFormatError("Failed compiling %s shader: %s\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), info);
	}

	return(shaderId);
}

static GLuint CreateShaderProgram(const char *name, const char *vertexSource, const char *fragmentSource) {
	GLuint programId = glCreateProgram();

	GLuint vertexShader = CreateShaderType(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = CreateShaderType(GL_FRAGMENT_SHADER, fragmentSource);

	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	glValidateProgram(programId);

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (linkResult == GL_FALSE) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);

		char *info = (char *)alloca(infoLen);
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		console::ConsoleFormatError("Failed linking shader '%s': %s\n", name, info);
	}

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return(programId);
}

static bool RunModern() {
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_NO_ERROR) {
		console::ConsoleFormatError("Failed initializing glew: %s\n", (const char *)glewGetErrorString(glewErr));
		return false;
	}

	float vertices[] = {
		0.0f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};

	console::ConsoleOut("Running modern opengl\n");

	const char vertexSource[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 inPosition;\n"
		"\n"
		"void main() {\n"
		"\tgl_Position = inPosition;\n"
		"}\n"
	};

	const char fragmentSource[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
		"void main() {\n"
		"\toutColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n"
	};

	GLuint shaderProgram = CreateShaderProgram("Test", vertexSource, fragmentSource);
	glUseProgram(shaderProgram);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	while (WindowUpdate()) {
		WindowSize windowArea = GetWindowArea();
		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		WindowFlip();
	}

	return true;
	}

int main(int argc, char **args) {
	using namespace fpl;
	using namespace fpl::memory;
	using namespace fpl::window;
	int result = 0;
	InitSettings settings = InitSettings();
#if MODERN_OPENGL
	strings::CopyAnsiString("FPL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.profile = VideoCompabilityProfile::Any;
	settings.video.minMajor = 3;
	settings.video.minMinor = 1;
#else
	strings::CopyAnsiString("FPL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.profile = VideoCompabilityProfile::Legacy;
#endif
	if (InitPlatform(InitFlags::VideoOpenGL, settings)) {

		const char *version = (const char *)glGetString(GL_VERSION);
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		const char *renderer = (const char *)glGetString(GL_RENDERER);
		console::ConsoleFormatOut("OpenGL version: %s\n", version);
		console::ConsoleFormatOut("OpenGL vendor: %s\n", vendor);
		console::ConsoleFormatOut("OpenGL renderer: %s\n", renderer);

#if MODERN_OPENGL
		RunModern();
#else
		RunLegacy();
#endif

		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}