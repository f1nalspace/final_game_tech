/*
-------------------------------------------------------------------------------
Name:
	FOGL | Test
Description:
	This demo shows how to use the "Final Dynamic OpenGL" library, which is a
	full OpenGL-Header and Loader.
Requirements:
	- C++
	- Final Platform Layer v0.7.7+
Author:
	Torsten Spaete
Changelog:
	## 2018-05-05:
 	- Fixed CMakeLists compile errors
 	- Fixed Makefile compile errors
	## 2018-04-24:
	- Fixed compile errors (FPL api change)
	## 2018-04-23:
	- Initial creation of this description block
	- Forced Visual-Studio-Project to compile in C++ always
-------------------------------------------------------------------------------
*/

#define USE_LEGACY_OPENGL 1
#define USE_FPL_OPENGL_CONTEXT_CREATION 1

#define FPL_IMPLEMENTATION
#if !USE_FPL_OPENGL_CONTEXT_CREATION
#	define FPL_NO_VIDEO_OPENGL
#endif
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

static GLuint CreateShaderType(GLenum type, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, fgl_null);
	glCompileShader(shaderId);

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if(!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		char *info = (char *)FPL_STACKALLOCATE(infoLen);
		glGetShaderInfoLog(shaderId, infoLen, &infoLen, info);
		fplConsoleFormatError("Failed compiling %s shader!\n", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
		fplConsoleFormatError("%s\n", info);
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
	if(!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);

		char *info = (char *)FPL_STACKALLOCATE(infoLen);
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		fplConsoleFormatError("Failed linking '%s' shader!\n", name);
		fplConsoleFormatError("%s\n", info);
	}

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return(programId);
}

static void RunModern(const fglOpenGLContext *context) {
	const char *version = (const char *)glGetString(GL_VERSION);
	const char *vendor = (const char *)glGetString(GL_VENDOR);
	const char *renderer = (const char *)glGetString(GL_RENDERER);
	fplConsoleFormatOut("OpenGL version: %s\n", version);
	fplConsoleFormatOut("OpenGL vendor: %s\n", vendor);
	fplConsoleFormatOut("OpenGL renderer: %s\n", renderer);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	const char *glslVersion = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	fplConsoleFormatOut("OpenGL GLSL Version %s:\n", glslVersion);

	int profileMask;
	int contextFlags;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
	glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
	fplConsoleFormatOut("OpenGL supported profiles:\n");
	fplConsoleFormatOut("\tCore: %s\n", ((profileMask & GL_CONTEXT_CORE_PROFILE_BIT) ? "yes" : "no"));
	fplConsoleFormatOut("\tForward: %s\n", ((contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "yes" : "no"));

	fplConsoleOut("Running modern opengl\n");

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, fgl_null);

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	while(fplWindowUpdate()) {
		fplWindowSize windowArea;
		fplGetWindowArea(&windowArea);
		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

#if USE_FPL_OPENGL_CONTEXT_CREATION
		fplVideoFlip();
#else
		fglPresentOpenGL(context);
#endif
	}

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArrayID);

}

int main(int argc, char **args) {
	int result = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);

	fplInitFlags initFlags;
#if USE_FPL_OPENGL_CONTEXT_CREATION
	initFlags = fplInitFlags_Video;
	settings.video.driver = fplVideoDriverType_OpenGL;
#	if !USE_LEGACY_OPENGL
	fplCopyAnsiString("FPL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
#	else
	fplCopyAnsiString("FPL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#	endif
#else
	initFlags = fplInitFlags_Window;
#endif // USE_FPL_OPENGL_CONTEXT_CREATION

	if(fplPlatformInit(initFlags, &settings)) {

#if USE_FPL_OPENGL_CONTEXT_CREATION
		if(fglLoadOpenGL(true)) {
			RunModern(fgl_null);
			fglUnloadOpenGL();
		}
#else
		fglOpenGLContextCreationParameters contextCreationParams = { 0 };
#	if !USE_LEGACY_OPENGL
		fplCopyAnsiString("DYNGL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
		contextCreationParams.profile = fglOpenGLProfileType_CoreProfile;
		contextCreationParams.majorVersion = 3;
		contextCreationParams.minorVersion = 3;
#	else
		fplCopyAnsiString("DYNGL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
		contextCreationParams.profile = fdyngl::OpenGLProfileType::LegacyProfile;
#	endif
#	if defined(FGL_PLATFORM_WIN32)
		contextCreationParams.windowHandle.win32.deviceContext = fpl__global__AppState->window.win32.deviceContext;
#	endif
		fglOpenGLContext glContext = { 0 };
		if(fglLoadOpenGL(false)) {
			if(fglCreateOpenGLContext(&contextCreationParams, &glContext)) {
				fglLoadOpenGLFunctions();
				RunModern(&glContext);
				fglDestroyOpenGLContext(&glContext);
			}
			fglUnloadOpenGL();
		}
#endif // USE_FPL_OPENGL_CONTEXT_CREATION

		fplPlatformRelease();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}