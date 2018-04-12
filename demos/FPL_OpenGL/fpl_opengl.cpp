#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_SOFTWARE
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#include <GL/gl.h>

#ifndef APIENTRY
#define APIENTRY
#endif
#define APIENTRYP APIENTRY *

#include <stddef.h>
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

#ifndef GL_CONTEXT_PROFILE_MASK

#define GL_CONTEXT_PROFILE_MASK           0x9126
#define GL_CONTEXT_FLAGS                  0x821E

#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT	0x0001
#define GL_CONTEXT_FLAG_DEBUG_BIT				0x00000002
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT		0x00000004
#define GL_CONTEXT_FLAG_NO_ERROR_BIT			0x00000008
#define GL_CONTEXT_CORE_PROFILE_BIT				0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT	0x00000002

#endif // GL_CONTEXT_PROFILE_MASK

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

#if defined(FPL_PLATFORM_WIN32)
static void *GLProcAddress(const char *name) {
	fpl__VideoState *videoState = (fpl__VideoState *)fpl__global__AppState->video.mem;
	FPL_ASSERT(videoState != nullptr);
	FPL_ASSERT(videoState->win32.opengl.api.wglGetProcAddress != nullptr);
	void *result = videoState->win32.opengl.api.wglGetProcAddress((const GLubyte *)name);
	return(result);
}
#else
static void *GLProcAddress(const char *name) {
	fpl__VideoState *videoState = (fpl__VideoState *)fpl__global__AppState->video.mem;
	FPL_ASSERT(videoState != nullptr);
	FPL_ASSERT(videoState->x11.opengl.api.glXGetProcAddress != nullptr);
	void *result = videoState->x11.opengl.api.glXGetProcAddress((const GLubyte *)name);
	return(result);
}
#endif

static void LoadGLExtensions() {
	glCreateShader = (PFNGLCREATESHADERPROC)GLProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GLProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GLProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)GLProcAddress("glGetShaderiv");
	glAttachShader = (PFNGLATTACHSHADERPROC)GLProcAddress("glAttachShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GLProcAddress("glCreateProgram");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GLProcAddress("glGetShaderInfoLog");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)GLProcAddress("glLinkProgram");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)GLProcAddress("glValidateProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GLProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GLProcAddress("glGetProgramInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)GLProcAddress("glDeleteShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GLProcAddress("glUseProgram");

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GLProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GLProcAddress("glBindVertexArray");
	glGenBuffers = (PFNGLGENBUFFERSPROC)GLProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)GLProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)GLProcAddress("glBufferData");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GLProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GLProcAddress("glDisableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GLProcAddress("glVertexAttribPointer");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)GLProcAddress("glDeleteVertexArrays");
}

#define MODERN_OPENGL 1

static void RunLegacy() {
	fplConsoleOut("Running legacy opengl\n");

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	while (fplWindowUpdate()) {
		fplWindowSize windowArea;
		FPL_ASSERT(fplGetWindowArea(&windowArea));
		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT);

		glBegin(GL_TRIANGLES);
		glVertex2f(0.0f, 0.5f);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.5f, -0.5f);
		glEnd();

		fplVideoFlip();
	}
}

static GLuint CreateShaderType(GLenum type, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, nullptr);
	glCompileShader(shaderId);

	char info[1024 * 10] = {};

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		FPL_ASSERT(infoLen <= FPL_ARRAYCOUNT(info));
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

	char info[1024 * 10] = {};

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
		FPL_ASSERT(infoLen <= FPL_ARRAYCOUNT(info));
		glGetProgramInfoLog(programId, infoLen, &infoLen, info);
		fplConsoleFormatError("Failed linking '%s' shader!\n", name);
		fplConsoleFormatError("%s\n", info);
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
	while (fplWindowUpdate()) {
		fplWindowSize windowArea;
		fplGetWindowArea(&windowArea);
		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		fplVideoFlip();
	}

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArrayID);

	return true;
}

int main(int argc, char **args) {
	int result = 0;
	fplSettings settings;
	fplSetDefaultSettings(&settings);
	settings.video.driver = fplVideoDriverType_OpenGL;
#if MODERN_OPENGL
	fplCopyAnsiString("FPL Modern OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
#else
	fplCopyAnsiString("FPL Legacy OpenGL", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
#endif
	if (fplPlatformInit(fplInitFlags_Video, &settings)) {

		const char *version = (const char *)glGetString(GL_VERSION);
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		const char *renderer = (const char *)glGetString(GL_RENDERER);
		fplConsoleFormatOut("OpenGL version: %s\n", version);
		fplConsoleFormatOut("OpenGL vendor: %s\n", vendor);
		fplConsoleFormatOut("OpenGL renderer: %s\n", renderer);

	#if MODERN_OPENGL
		RunModern();
	#else
		RunLegacy();
	#endif

		fplPlatformRelease();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}