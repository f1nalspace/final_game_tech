/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | OpenGL

Description:
	This demo showcases the initialization and rendering of legacy and modern OpenGL.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Changelog:
	## 2020-04-20
	- Rotating example for legacy and modern

	## 2018-10-22
	- Reflect api changes in FPL 0.9.3
	- Added a random color effect (Modern only)
	- Added a smooth color effect

	## 2018-09-24
	- Reflect api changes in FPL 0.9.2

	## 2018-05-5:
	- Fixed CMakeLists to compile properly
	- Fixed Makefile to compile properly

	## 2018-04-23:
	- Initial creation of this description block
	- Changed from C++ to C99
	- Forced Visual-Studio-Project to compile in C always

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

#define MODERN_OPENGL 1 // Enable this to use OpenGL 3.3+

#define FPL_IMPLEMENTATION
#define FPL_NO_VIDEO_SOFTWARE
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#include <final_math.h> // Vec3f, Mat4f, etc.

// You have to include GL.h yourself or use any other opengl loader you want.
// FPL just creates a opengl rendering context for you, but nothing more.
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
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
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

static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;

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
typedef GLint(APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FV)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv = NULL;

// @TODO(final): Add a FPL function for retrieving a proc from the current video driver
#if defined(FPL_PLATFORM_WINDOWS)
static void *GLProcAddress(const char *name) {
	fpl__VideoState *videoState = (fpl__VideoState *)fpl__global__AppState->video.mem;
	fplAssert(videoState != NULL);
	fplAssert(videoState->win32.opengl.api.wglGetProcAddress != NULL);
	void *result = videoState->win32.opengl.api.wglGetProcAddress(name);
	return(result);
}
#else
static void *GLProcAddress(const char *name) {
	fpl__VideoState *videoState = (fpl__VideoState *)fpl__global__AppState->video.mem;
	fplAssert(videoState != NULL);
	fplAssert(videoState->x11.opengl.api.glXGetProcAddress != NULL);
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
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GLProcAddress("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)GLProcAddress("glUniform1i");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FV)GLProcAddress("glUniformMatrix4fv");

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

#define DT (1.0f / 60.0f)
#define FloorRW 1.0f
#define FloorRD 1.0f
#define FloorYPlane 0.0f
#define CrossRadius 1.5f
#define GrayColor {0.25f, 0.25f, 0.25f, 1.0f}
#define TriangleExtW 0.5
#define TriangleHeight 1.0

typedef struct Vertex {
	Vec4f color;
	Vec4f pos;
} Vertex;

static Vertex CrossVerts[] = {
	{ {1, 0, 0, 1}, {0,0,0,1} },
	{ {1, 0, 0, 1}, {CrossRadius,0,0,1} },
	{ {0, 1, 0, 1}, {0,0,0,1} },
	{ {0, 1, 0, 1}, {0,CrossRadius,0,1} },
	{ {0, 0, 1, 1}, {0,0,0,1} },
	{ {0, 0, 1, 1}, {0,0,CrossRadius,1} },
};

static Vertex FloorVerts[] = {
	{ GrayColor, {-FloorRW, FloorYPlane, -FloorRD, 1} },
	{ GrayColor, {-FloorRW, FloorYPlane, FloorRD, 1} },
	{ GrayColor, {-FloorRW, FloorYPlane, FloorRD, 1} },
	{ GrayColor, {FloorRW, FloorYPlane, FloorRD, 1} },
	{ GrayColor, {FloorRW, FloorYPlane, FloorRD, 1} },
	{ GrayColor, {FloorRW, FloorYPlane, -FloorRD, 1} },
	{ GrayColor, {FloorRW, FloorYPlane, -FloorRD, 1} },
	{ GrayColor, {-FloorRW, FloorYPlane, -FloorRD, 1} },
};

static Vertex TriangleVerts[] = {
	{ {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, TriangleHeight, 0.0f, 1} },
	{ {0.0f, 1.0f, 0.0f, 1.0f}, {-TriangleExtW, 0.0f, 0.0f, 1} },
	{ {0.0f, 0.0f, 1.0f, 1.0f}, {TriangleExtW, 0.0f, 0.0f, 1} },
};


static void RunLegacy() {
	fplConsoleOut("Running legacy opengl\n");

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);

	fplWallClock lastFrameTime = fplGetWallClock();
	float rot = 0.0f;
	while (fplWindowUpdate()) {
		fplPollEvents();

		fplWindowSize windowArea;
		fplGetWindowSize(&windowArea);

		float aspect = windowArea.width / (float)windowArea.height;
		Mat4f proj = Mat4PerspectiveRH(DegreesToRadians(35), aspect, 0.1f, 100.0f);
		Mat4f camera = Mat4LookAtRH(V3fInit(2, 2, 3), V3fInit(0, 0, 0), V3fInit(0, 1, 0));
		Mat4f model = Mat4RotationY(rot);
		Mat4f vp = Mat4Mult(proj, camera);
		Mat4f mvp = Mat4Mult(vp, model);

		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0 // Lines needs special shaders for rendering in GLSL, so dont render it in legacy mode
		// Coordinate-System
		glLoadMatrixf(&vp.m[0]);
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		for (int i = 0, c = fplArrayCount(CrossVerts); i < c; ++i) {
			glColor4fv(&CrossVerts[i].color.m[0]);
			glVertex4fv(&CrossVerts[i].pos.m[0]);
		}
		glEnd();
		glLineWidth(1.0f);
#endif

		// Plane
		glLoadMatrixf(&mvp.m[0]);
		glBegin(GL_TRIANGLE_FAN);
		for (int i = 0, c = fplArrayCount(FloorVerts); i < c; ++i) {
			glColor4fv(&FloorVerts[i].color.m[0]);
			glVertex4fv(&FloorVerts[i].pos.m[0]);
		}
		glEnd();

		// Triangle
		glLoadMatrixf(&mvp.m[0]);
		glBegin(GL_TRIANGLES);
		for (int i = 0, c = fplArrayCount(TriangleVerts); i < c; ++i) {
			glColor4fv(&TriangleVerts[i].color.m[0]);
			glVertex4fv(&TriangleVerts[i].pos.m[0]);
		}
		glEnd();

		fplVideoFlip();

		fplWallClock endFrameTime = fplGetWallClock();
		double frameDuration = fplGetWallDelta(lastFrameTime, endFrameTime);
		lastFrameTime = endFrameTime;

		float dt = fplMin((float)frameDuration, DT);
		
		rot += 0.5f * dt;
	}
}

static GLuint CreateShaderType(GLenum type, const char *source) {
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);

	char info[1024 * 10] = fplZeroInit;

	GLint compileResult;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		GLint infoLen;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
		fplAssert(infoLen <= fplArrayCount(info));
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

	char info[1024 * 10] = fplZeroInit;

	GLint linkResult;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		GLint infoLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
		fplAssert(infoLen <= fplArrayCount(info));
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
		"layout(location = 0) in vec4 inColor;\n"
		"layout(location = 1) in vec4 inPosition;\n"
		"\n"
		"uniform mat4 inMVP;\n"
		"\n"
		"out vec4 varColor;\n"
		"\n"
		"void main() {\n"
		"\tvarColor = inColor;\n"
		"\tgl_Position = inMVP * inPosition;\n"
		"}\n"
	};

	const char fragmentSourceRandom[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
		"uniform int inFrame;\n"
		"\n"
		"in vec4 varColor;"
		"\n"
		"const uint k = 1103515245U;\n"
		"\n"
		"vec3 hash(uvec3 x) {\n"
		"\tx = ((x>>8U)^x.yzx)*k;\n"
		"\tx = ((x>>8U)^x.yzx)*k;\n"
		"\tx = ((x>>8U)^x.yzx)*k;\n"
		"\treturn vec3(x)*(1.0/float(0xffffffffU));\n"
		"}\n"
		"\n"
		"void main() {\n"
		"\tvec4 fragCoord = gl_FragCoord;\n"
		"\tuvec3 p = uvec3(fragCoord.xy, inFrame);\n"
		"\tvec4 randomColor = vec4(hash(p), 1.0);\n"
		"\toutColor = randomColor * varColor;\n"
		"}\n"
	};

	const char fragmentSourceColor[] = {
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
		"in vec4 varColor;"
		"\n"
		"void main() {\n"
		"\toutColor = varColor;\n"
		"}\n"
	};

	GLuint shaderProgramRandom = CreateShaderProgram("Random", vertexSource, fragmentSourceRandom);
	GLuint shaderProgramColor = CreateShaderProgram("Color", vertexSource, fragmentSourceColor);

	GLuint inFrameLocationRandom = glGetUniformLocation(shaderProgramRandom, "inFrame");
	GLuint inMVPLocationRandom = glGetUniformLocation(shaderProgramRandom, "inMVP");
	GLuint inMVPLocationColor = glGetUniformLocation(shaderProgramColor, "inMVP");

	//
	// Triangle VAO
	//
	GLuint triangleVAO;
	glGenVertexArrays(1, &triangleVAO);
	glBindVertexArray(triangleVAO);
	GLuint triangleBuffer;
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVerts), TriangleVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(4 * sizeof(float)));

	//
	// Floor VAO
	//
	GLuint floorVAO;
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	GLuint floorBuffer;
	glGenBuffers(1, &floorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, floorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FloorVerts), FloorVerts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(4 * sizeof(float)));

	// Disable VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f);

	fplWallClock lastFrameTime = fplGetWallClock();
	int frameIndex = 0;
	float rot = 0.0f;
	while (fplWindowUpdate()) {
		fplPollEvents();

		fplWindowSize windowArea;
		fplGetWindowSize(&windowArea);

		float aspect = windowArea.width / (float)windowArea.height;
		Mat4f proj = Mat4PerspectiveRH(DegreesToRadians(35), aspect, 0.1f, 100.0f);
		Mat4f camera = Mat4LookAtRH(V3fInit(2, 2, 3), V3fInit(0, 0, 0), V3fInit(0, 1, 0));
		Mat4f model = Mat4RotationY(rot);
		Mat4f vp = Mat4Mult(proj, camera);
		Mat4f mvp = Mat4Mult(vp, model);

		glViewport(0, 0, windowArea.width, windowArea.height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(triangleVAO);
		glUseProgram(shaderProgramRandom);
		glUniform1i(inFrameLocationRandom, frameIndex);
		glUniformMatrix4fv(inMVPLocationRandom, 1, GL_FALSE, &mvp.m[0]);
		glDrawArrays(GL_TRIANGLES, 0, fplArrayCount(TriangleVerts));

		glBindVertexArray(floorVAO);
		glUseProgram(shaderProgramColor);
		glUniformMatrix4fv(inMVPLocationColor, 1, GL_FALSE, &mvp.m[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, fplArrayCount(FloorVerts));

		fplVideoFlip();
				
		fplWallClock endFrameTime = fplGetWallClock();
		double frameDuration = fplGetWallDelta(lastFrameTime, endFrameTime);
		lastFrameTime = endFrameTime;

		float dt = fplMin((float)frameDuration, DT);
		
		++frameIndex;
		rot += 0.5f * dt;
	}

	glDeleteVertexArrays(1, &floorVAO);
	glDeleteVertexArrays(1, &triangleVAO);

	return true;
}

int main(int argc, char **args) {
	int result = 0;
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_OpenGL;
#if MODERN_OPENGL
	fplCopyString("FPL Modern OpenGL", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Core;
	settings.video.graphics.opengl.majorVersion = 3;
	settings.video.graphics.opengl.minorVersion = 3;
	settings.video.graphics.opengl.multiSamplingCount = 4;
	settings.video.isVSync = true;
#else
	fplCopyString("FPL Legacy OpenGL", settings.window.title, fplArrayCount(settings.window.title));
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