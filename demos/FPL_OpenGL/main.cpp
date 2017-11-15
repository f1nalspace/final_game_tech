#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE 1
#include "final_platform_layer.hpp"

#ifndef APIENTRY
#define APIENTRY
#endif
#define APIENTRYP APIENTRY *

#include <stddef.h>
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

#define GL_CONTEXT_PROFILE_MASK           0x9126
#define GL_CONTEXT_FLAGS                  0x821E

#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_LINK_STATUS                    0x8B82
#define GL_STATIC_DRAW                    0x88E4

typedef GLuint(APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef GLuint(APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);

static PFNGLCREATESHADERPROC glCreateShader = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

#define GL_ARRAY_BUFFER                   0x8892

typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);

static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
static PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
static PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
static PFNGLBUFFERDATAPROC glBufferData = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;

static void LoadGLExtensions() {
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)wglGetProcAddress("glValidateProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
}

#define MODERN_OPENGL 1
#define OPENGL_MAJOR 3
#define OPENGL_MINOR 3
#define VIDEO_PROFILE VideoCompabilityProfile::Core

static void RunLegacy() {
	ConsoleOut("Running legacy opengl\n");

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
	if (!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		char *info = (char *)alloca(infoLen);
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		ConsoleFormatError("Failed compiling %s shader!\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		ConsoleFormatError("%s\n", info);
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
	if (!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);

		char *info = (char *)alloca(infoLen);
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		ConsoleFormatError("Failed linking '%s' shader!\n", name);
		ConsoleFormatError("%s\n", info);
	}

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return(programId);
}

static bool RunModern() {
	LoadGLExtensions();

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	const char *glslVersion = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	ConsoleFormatOut("OpenGL GLSL Version %s:\n", glslVersion);

	int profileMask;
	int contextFlags;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
	glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
	ConsoleFormatOut("OpenGL supported profiles:\n");
	ConsoleFormatOut("\tCore: %s\n", ((profileMask & FPL_GL_CONTEXT_CORE_PROFILE_BIT) ? "yes" : "no"));
	ConsoleFormatOut("\tForward: %s\n", ((contextFlags & FPL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "yes" : "no"));

	ConsoleOut("Running modern opengl\n");

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

	float vertices[] = {
		0.0f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
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

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArrayID);

	return true;
}

int main(int argc, char **args) {
	int result = 0;
	Settings settings = Settings();
	settings.video.driverType = VideoDriverType::OpenGL;
#if MODERN_OPENGL
	CopyAnsiString("FPL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.profile = VIDEO_PROFILE;
	settings.video.majorVersion = OPENGL_MAJOR;
	settings.video.minorVersion = OPENGL_MINOR;
#else
	CopyAnsiString("FPL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.profile = VideoCompabilityProfile::Legacy;
#endif
	if (InitPlatform(InitFlags::Video, settings)) {

		const char *version = (const char *)glGetString(GL_VERSION);
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		const char *renderer = (const char *)glGetString(GL_RENDERER);
		ConsoleFormatOut("OpenGL version: %s\n", version);
		ConsoleFormatOut("OpenGL vendor: %s\n", vendor);
		ConsoleFormatOut("OpenGL renderer: %s\n", renderer);

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