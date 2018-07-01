/***
final_dynamic_opengl.h

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

A open source single header file OpenGL-Loader C99 library.

This library is designed to load all the opengl functions for you so you can start right away with OpenGL up to version 4.6.
It even can create a rendering context for you, if needed.

Due to removing any kind of linking madness, all opengl functions are defined as static - so its private to this header file only!
Therefore you can use this library in one file only and cannot use in combination with other opengl libraries.

The only dependencies are built-in operating system libraries and a C99 complaint compiler.

Required linking is bare minimum:
	Win32: Link to kernel32.lib
	Unix/Linux: Link to ld.so

-------------------------------------------------------------------------------
	Getting started
-------------------------------------------------------------------------------

- Drop this file into your main C/C++ project and include it in one place you do the rendering.
- Define FGL_IMPLEMENTATION before including this header file in that translation unit.
- Load the library with fglLoadOpenGL(), while a opengl rendering context is already activated - or create a context using fglCreateOpenGLContext()
- Use all the OpenGL features you want
- Unload the library with fglUnloadOpenGL() when you are done
- Destroy the context when you created it using fglDestroyOpenGLContext()

-------------------------------------------------------------------------------
	Usage from a already activated rendering context
-------------------------------------------------------------------------------

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

if (fglLoadOpenGL(true)) {
	// ... load shader, whatever you want to do
	fglUnloadOpenGL();
}

-------------------------------------------------------------------------------
	Usage (Without a rendering context, but with an existing window)
-------------------------------------------------------------------------------

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

// Load opengl library without loading all the functions - functions are loaded separately later
if (fglLoadOpenGL(false)) {

	// Fill out window handle (This is platform dependent!)
	fglOpenGLContextCreationParameters contextCreationParams = {0};
#	if defined(FGL_PLATFORM_WIN32)
		contextCreationParams.windowHandle.win32.deviceContext = // ... pass your current device context here
		// or
		contextCreationParams.windowHandle.win32.windowHandle = // ... pass your current window handle here
#	endif

	// Create context and load opengl functions
	fglOpenGLContext glContext = {0};
	if (fglCreateOpenGLContext(&contextCreationParams, &glContext)) {
		fglLoadOpenGLFunctions();

		// ... load shader, whatever you want to do

		fglDestroyOpenGLContext(&glContext);
	}
	fglUnloadOpenGL();
}

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

Final Dynamic OpenGL is released under the following license:

MIT License

Copyright (c) 2018 Torsten Spaete

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***/

/*!
	\file final_dynamic_opengl.h
	\version v0.3.5.0 beta
	\author Torsten Spaete
	\brief Final Dynamic OpenGL (FGL) - A open source C99 single file header OpenGL-Loader library.
*/

/*!
	\page page_changelog Changelog
	\tableofcontents

 	## v0.3.5.0 beta:
 	- Fixed: Fixed incompabilties with MingW compiler (FARPROC)

    ## v0.3.4.0 beta:
    - Fixed: Removed fglOpenGLState struct dependency for fgl__SetLastError()
    - Fixed: Removed fglOpenGLState struct dependency for Win32/X11 opengl functions
    - Fixed: GLX initialization was not working anymore

	## v0.3.3.0 beta:
	- Changed: Prevent including FGL before any other OpenGL library/header
	- Changed: Moved all platform specific code into its own block
	- Changed: fglOpenGLWindowHandle/fglOpenGLRenderingContext is now a union
	- Fixed: Corrected documentation errors
	- Fixed: Fixed fgl__ClearMemory was not working properly
	- New: [Win32] Implemented modern context creation

	## v0.3.2.0 beta:
	- Fixed: Fixed incompatibilties with C99

	## v0.3.1.0 beta:
	- Fixed: Fixed tons of compile errors on linux

	## v0.3.0.0 beta:
	- Changed: Transition from C++ to C99
	- Renamed fdyngl to fgl

	## v0.2.0.0 beta:
	- Changed: Added parameter for controlling to load of the extensions to LoadOpenGL()
	- Changed:
	- Fixed: Use stdint.h instead of inttypes.h
	- Fixed: WINGDIAPI detection was wrong
	- New: Added context creation support (CreateOpenGLContext, DestroyOpenGLContext)
	- New: Added LoadOpenGLFunctions()
	- New: Added PresentOpenGL()
	- New: Added GetLastError()
	- New: Written documentation

	## v0.1.0.0 beta:
	- Initial version
*/

/*!
	\page page_todo Todo
	\tableofcontents

	- [POSIX, GLX] Implement context creation

*/

// ****************************************************************************
//
// Header
//
// ****************************************************************************
#ifndef FGL_INCLUDE_H
#define FGL_INCLUDE_H

//
// C99 detection
//
// https://en.wikipedia.org/wiki/C99#Version_detection
// Visual Studio 2015+
#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
	//! Detected C99 compiler
#	define FGL_IS_C99
#elif defined(__cplusplus)
	//! Detected C++ compiler
#	define FGL_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

//
// Includes
//
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // uint32_t
#include <stdlib.h> // NULL
#include <stdbool.h> // bool

#define fgl_null NULL

//! Macro for initialize a struct to zero
#if defined(FGL_IS_C99)
#	define FGL_ZERO_INIT {0}
#else
#	define FGL_ZERO_INIT {}
#endif

//
// Platform detection
//
#if defined(_WIN32)
#	define FGL_PLATFORM_WIN32
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FGL_PLATFORM_LINUX
#	define FGL_PLATFORM_POSIX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FGL_PLATFORM_UNIX
#	define FGL_PLATFORM_POSIX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// We do not support already active opengl headers/libraries
//
#if (defined(__gl_h_) || defined(__GL_H__) || defined(GL_VERSION_1_1)) || (defined(__glext_h_) || defined(GL_GLEXT_VERSION) || defined(GL_VERSION_1_2))
#	error "You cannot have any OpenGL library already included/loaded before using this library!"
#endif

//
// Required api defines
//
#ifdef FGL_PLATFORM_WIN32
#	ifdef APIENTRY
#		define FGL_GLAPIENTRY APIENTRY
#		define FGL_APIENTRY APIENTRY
#	else
#		if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#			define FGL_APIENTRY __stdcall
#			ifndef FGL_GLAPIENTRY
#				define FGL_GLAPIENTRY __stdcall
#			endif
#			ifndef FGL_APIENTRY
#				define FGL_APIENTRY __stdcall
#			endif
#		else
#			define FGL_APIENTRY
#		endif
#	endif
#	ifdef WINGDIAPI
#		define FGL_WINGDIAPI WINGDIAPI
#	else
#		define FGL_WINGDIAPI __declspec(dllimport)
#	endif
#	if defined(__MINGW32__) || defined(__CYGWIN__)
#		define FGL_GLAPI extern
#	else
#		define FGL_GLAPI FGL_WINGDIAPI
#	endif
#else
#	define FGL_GLAPI extern
#	define FGL_APIENTRY
#endif

#if FGL_AS_PRIVATE
	//! API functions exported as static
#	define fdyngl_api static
#else
	//! API functions exported as extern
#	define fdyngl_api extern
#endif

//
// Platform includes
//
#if defined(FGL_PLATFORM_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#			define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <Windows.h>

//! Win32 OpenGL window handle
typedef struct fglWin32OpenGLWindowHandle {
	//! Window handle
	HWND windowHandle;
	//! Device context
	HDC deviceContext;
	//! Bool to indicate to release DC when done
	bool requireToReleaseDC;
} fglWin32OpenGLWindowHandle;

//! Win32 OpenGL rendering context
typedef struct fglWin32OpenGLRenderingContext {
	//! Rendering context
	HGLRC renderingContext;
} fglWin32OpenGLRenderingContext;

#elif defined(FGL_PLATFORM_POSIX)
#	include <dlfcn.h> // dlopen
#	include <X11/X.h>
#	include <X11/Xlib.h>
#	include <X11/Xutil.h> // XVisualInfo

//! Posix OpenGL window handle
typedef struct fglPosixOpenGLWindowHandle {
	//! Display
	Display *display;
	//! Window
	Window *window;
} fglPosixOpenGLWindowHandle;

typedef struct fglPosixOpenGLRenderingContext {
	//! Dummy
	int dummy;
} fglPosixOpenGLRenderingContext;
#endif

//
// API
//

#ifdef __cplusplus
extern "C" {
#endif

//! OpenGL profile type
	typedef enum fglOpenGLProfileType {
		//! No or legacy profile
		fglOpenGLProfileType_LegacyProfile = 0,
		//! Core profile
		fglOpenGLProfileType_CoreProfile,
		//! Compability profile
		fglOpenGLProfileType_CompabilityProfile,
	} fglOpenGLProfileType;

	//! OpenGL window handle
	typedef union fglOpenGLWindowHandle {
#if defined(FGL_PLATFORM_WIN32)
		//! Win32 window handle
		fglWin32OpenGLWindowHandle win32;
#elif defined(FGL_PLATFORM_POSIX)
		//! Posix window handle
		fglPosixOpenGLWindowHandle posix;
#endif
	} fglOpenGLWindowHandle;

	//! OpenGL window handle
	typedef union fglOpenGLRenderingContext {
#if defined(FGL_PLATFORM_WIN32)
		//! Win32 rendering context
		fglWin32OpenGLRenderingContext win32;
#elif defined(FGL_PLATFORM_POSIX)
		//! Posix rendering context
		fglPosixOpenGLRenderingContext posix;
#endif
	} fglOpenGLRenderingContext;

	//! OpenGL rendering context
	typedef struct fglOpenGLContext {
		//! Window handle container
		fglOpenGLWindowHandle windowHandle;
		//! Rendering context container
		fglOpenGLRenderingContext renderingContext;
		//! Is context valid
		bool isValid;
	} fglOpenGLContext;

	//! OpenGL Context Creation Parameters Container
	typedef struct fglOpenGLContextCreationParameters {
		//! Window handle
		fglOpenGLWindowHandle windowHandle;
		//! Desired major version
		uint32_t majorVersion;
		//! Desired minor version
		uint32_t minorVersion;
		//! Desired profile type
		fglOpenGLProfileType profile;
		//! Is forward compability enabled
		bool forwardCompability;
	} fglOpenGLContextCreationParameters;

	//! Sets the context parameters to default values
	fdyngl_api void fglSetDefaultOpenGLContextCreationParameters(fglOpenGLContextCreationParameters *outParams);

	//! Create a opengl context
	fdyngl_api bool fglCreateOpenGLContext(const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext);

	//! Destroy the given opengl context
	fdyngl_api void fglDestroyOpenGLContext(fglOpenGLContext *context);

	//! Does all the things to get opengl up and running
	fdyngl_api bool fglLoadOpenGL(const bool loadFunctions);

	//! Releases all resources allocated for opengl
	fdyngl_api void fglUnloadOpenGL();

	//! Load all opengl functions
	fdyngl_api void fglLoadOpenGLFunctions();

	//! Presents the current frame for the given opengl context
	fdyngl_api void fglPresentOpenGL(const fglOpenGLContext *context);

	//! Returns last error string
	fdyngl_api const char *fglGetLastError();

	//
	// OpenGL types and function prototypes (gl.h, glext.h)
	// This is automatically generated by a tool, do not modify by hand!
	//
#	ifndef GL_VERSION_1_1
#	define GL_VERSION_1_1 1
	static bool isGL_VERSION_1_1;
	typedef unsigned int GLenum;
	typedef unsigned int GLbitfield;
	typedef unsigned int GLuint;
	typedef int GLint;
	typedef int GLsizei;
	typedef unsigned char GLboolean;
	typedef signed char GLbyte;
	typedef short GLshort;
	typedef unsigned char GLubyte;
	typedef unsigned short GLushort;
	typedef unsigned long GLulong;
	typedef float GLfloat;
	typedef float GLclampf;
	typedef double GLdouble;
	typedef double GLclampd;
	typedef void GLvoid;
#	define GL_ACCUM 0x0100
#	define GL_LOAD 0x0101
#	define GL_RETURN 0x0102
#	define GL_MULT 0x0103
#	define GL_ADD 0x0104
#	define GL_NEVER 0x0200
#	define GL_LESS 0x0201
#	define GL_EQUAL 0x0202
#	define GL_LEQUAL 0x0203
#	define GL_GREATER 0x0204
#	define GL_NOTEQUAL 0x0205
#	define GL_GEQUAL 0x0206
#	define GL_ALWAYS 0x0207
#	define GL_CURRENT_BIT 0x00000001
#	define GL_POINT_BIT 0x00000002
#	define GL_LINE_BIT 0x00000004
#	define GL_POLYGON_BIT 0x00000008
#	define GL_POLYGON_STIPPLE_BIT 0x00000010
#	define GL_PIXEL_MODE_BIT 0x00000020
#	define GL_LIGHTING_BIT 0x00000040
#	define GL_FOG_BIT 0x00000080
#	define GL_DEPTH_BUFFER_BIT 0x00000100
#	define GL_ACCUM_BUFFER_BIT 0x00000200
#	define GL_STENCIL_BUFFER_BIT 0x00000400
#	define GL_VIEWPORT_BIT 0x00000800
#	define GL_TRANSFORM_BIT 0x00001000
#	define GL_ENABLE_BIT 0x00002000
#	define GL_COLOR_BUFFER_BIT 0x00004000
#	define GL_HINT_BIT 0x00008000
#	define GL_EVAL_BIT 0x00010000
#	define GL_LIST_BIT 0x00020000
#	define GL_TEXTURE_BIT 0x00040000
#	define GL_SCISSOR_BIT 0x00080000
#	define GL_ALL_ATTRIB_BITS 0x000fffff
#	define GL_POINTS 0x0000
#	define GL_LINES 0x0001
#	define GL_LINE_LOOP 0x0002
#	define GL_LINE_STRIP 0x0003
#	define GL_TRIANGLES 0x0004
#	define GL_TRIANGLE_STRIP 0x0005
#	define GL_TRIANGLE_FAN 0x0006
#	define GL_QUADS 0x0007
#	define GL_QUAD_STRIP 0x0008
#	define GL_POLYGON 0x0009
#	define GL_ZERO 0
#	define GL_ONE 1
#	define GL_SRC_COLOR 0x0300
#	define GL_ONE_MINUS_SRC_COLOR 0x0301
#	define GL_SRC_ALPHA 0x0302
#	define GL_ONE_MINUS_SRC_ALPHA 0x0303
#	define GL_DST_ALPHA 0x0304
#	define GL_ONE_MINUS_DST_ALPHA 0x0305
#	define GL_DST_COLOR 0x0306
#	define GL_ONE_MINUS_DST_COLOR 0x0307
#	define GL_SRC_ALPHA_SATURATE 0x0308
#	define GL_TRUE 1
#	define GL_FALSE 0
#	define GL_CLIP_PLANE0 0x3000
#	define GL_CLIP_PLANE1 0x3001
#	define GL_CLIP_PLANE2 0x3002
#	define GL_CLIP_PLANE3 0x3003
#	define GL_CLIP_PLANE4 0x3004
#	define GL_CLIP_PLANE5 0x3005
#	define GL_BYTE 0x1400
#	define GL_UNSIGNED_BYTE 0x1401
#	define GL_SHORT 0x1402
#	define GL_UNSIGNED_SHORT 0x1403
#	define GL_INT 0x1404
#	define GL_UNSIGNED_INT 0x1405
#	define GL_FLOAT 0x1406
#	define GL_2_BYTES 0x1407
#	define GL_3_BYTES 0x1408
#	define GL_4_BYTES 0x1409
#	define GL_DOUBLE 0x140A
#	define GL_NONE 0
#	define GL_FRONT_LEFT 0x0400
#	define GL_FRONT_RIGHT 0x0401
#	define GL_BACK_LEFT 0x0402
#	define GL_BACK_RIGHT 0x0403
#	define GL_FRONT 0x0404
#	define GL_BACK 0x0405
#	define GL_LEFT 0x0406
#	define GL_RIGHT 0x0407
#	define GL_FRONT_AND_BACK 0x0408
#	define GL_AUX0 0x0409
#	define GL_AUX1 0x040A
#	define GL_AUX2 0x040B
#	define GL_AUX3 0x040C
#	define GL_NO_ERROR 0
#	define GL_INVALID_ENUM 0x0500
#	define GL_INVALID_VALUE 0x0501
#	define GL_INVALID_OPERATION 0x0502
#	define GL_STACK_OVERFLOW 0x0503
#	define GL_STACK_UNDERFLOW 0x0504
#	define GL_OUT_OF_MEMORY 0x0505
#	define GL_2D 0x0600
#	define GL_3D 0x0601
#	define GL_3D_COLOR 0x0602
#	define GL_3D_COLOR_TEXTURE 0x0603
#	define GL_4D_COLOR_TEXTURE 0x0604
#	define GL_PASS_THROUGH_TOKEN 0x0700
#	define GL_POINT_TOKEN 0x0701
#	define GL_LINE_TOKEN 0x0702
#	define GL_POLYGON_TOKEN 0x0703
#	define GL_BITMAP_TOKEN 0x0704
#	define GL_DRAW_PIXEL_TOKEN 0x0705
#	define GL_COPY_PIXEL_TOKEN 0x0706
#	define GL_LINE_RESET_TOKEN 0x0707
#	define GL_EXP 0x0800
#	define GL_EXP2 0x0801
#	define GL_CW 0x0900
#	define GL_CCW 0x0901
#	define GL_COEFF 0x0A00
#	define GL_ORDER 0x0A01
#	define GL_DOMAIN 0x0A02
#	define GL_CURRENT_COLOR 0x0B00
#	define GL_CURRENT_INDEX 0x0B01
#	define GL_CURRENT_NORMAL 0x0B02
#	define GL_CURRENT_TEXTURE_COORDS 0x0B03
#	define GL_CURRENT_RASTER_COLOR 0x0B04
#	define GL_CURRENT_RASTER_INDEX 0x0B05
#	define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#	define GL_CURRENT_RASTER_POSITION 0x0B07
#	define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#	define GL_CURRENT_RASTER_DISTANCE 0x0B09
#	define GL_POINT_SMOOTH 0x0B10
#	define GL_POINT_SIZE 0x0B11
#	define GL_POINT_SIZE_RANGE 0x0B12
#	define GL_POINT_SIZE_GRANULARITY 0x0B13
#	define GL_LINE_SMOOTH 0x0B20
#	define GL_LINE_WIDTH 0x0B21
#	define GL_LINE_WIDTH_RANGE 0x0B22
#	define GL_LINE_WIDTH_GRANULARITY 0x0B23
#	define GL_LINE_STIPPLE 0x0B24
#	define GL_LINE_STIPPLE_PATTERN 0x0B25
#	define GL_LINE_STIPPLE_REPEAT 0x0B26
#	define GL_LIST_MODE 0x0B30
#	define GL_MAX_LIST_NESTING 0x0B31
#	define GL_LIST_BASE 0x0B32
#	define GL_LIST_INDEX 0x0B33
#	define GL_POLYGON_MODE 0x0B40
#	define GL_POLYGON_SMOOTH 0x0B41
#	define GL_POLYGON_STIPPLE 0x0B42
#	define GL_EDGE_FLAG 0x0B43
#	define GL_CULL_FACE 0x0B44
#	define GL_CULL_FACE_MODE 0x0B45
#	define GL_FRONT_FACE 0x0B46
#	define GL_LIGHTING 0x0B50
#	define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#	define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#	define GL_LIGHT_MODEL_AMBIENT 0x0B53
#	define GL_SHADE_MODEL 0x0B54
#	define GL_COLOR_MATERIAL_FACE 0x0B55
#	define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#	define GL_COLOR_MATERIAL 0x0B57
#	define GL_FOG 0x0B60
#	define GL_FOG_INDEX 0x0B61
#	define GL_FOG_DENSITY 0x0B62
#	define GL_FOG_START 0x0B63
#	define GL_FOG_END 0x0B64
#	define GL_FOG_MODE 0x0B65
#	define GL_FOG_COLOR 0x0B66
#	define GL_DEPTH_RANGE 0x0B70
#	define GL_DEPTH_TEST 0x0B71
#	define GL_DEPTH_WRITEMASK 0x0B72
#	define GL_DEPTH_CLEAR_VALUE 0x0B73
#	define GL_DEPTH_FUNC 0x0B74
#	define GL_ACCUM_CLEAR_VALUE 0x0B80
#	define GL_STENCIL_TEST 0x0B90
#	define GL_STENCIL_CLEAR_VALUE 0x0B91
#	define GL_STENCIL_FUNC 0x0B92
#	define GL_STENCIL_VALUE_MASK 0x0B93
#	define GL_STENCIL_FAIL 0x0B94
#	define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#	define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#	define GL_STENCIL_REF 0x0B97
#	define GL_STENCIL_WRITEMASK 0x0B98
#	define GL_MATRIX_MODE 0x0BA0
#	define GL_NORMALIZE 0x0BA1
#	define GL_VIEWPORT 0x0BA2
#	define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#	define GL_PROJECTION_STACK_DEPTH 0x0BA4
#	define GL_TEXTURE_STACK_DEPTH 0x0BA5
#	define GL_MODELVIEW_MATRIX 0x0BA6
#	define GL_PROJECTION_MATRIX 0x0BA7
#	define GL_TEXTURE_MATRIX 0x0BA8
#	define GL_ATTRIB_STACK_DEPTH 0x0BB0
#	define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#	define GL_ALPHA_TEST 0x0BC0
#	define GL_ALPHA_TEST_FUNC 0x0BC1
#	define GL_ALPHA_TEST_REF 0x0BC2
#	define GL_DITHER 0x0BD0
#	define GL_BLEND_DST 0x0BE0
#	define GL_BLEND_SRC 0x0BE1
#	define GL_BLEND 0x0BE2
#	define GL_LOGIC_OP_MODE 0x0BF0
#	define GL_INDEX_LOGIC_OP 0x0BF1
#	define GL_COLOR_LOGIC_OP 0x0BF2
#	define GL_AUX_BUFFERS 0x0C00
#	define GL_DRAW_BUFFER 0x0C01
#	define GL_READ_BUFFER 0x0C02
#	define GL_SCISSOR_BOX 0x0C10
#	define GL_SCISSOR_TEST 0x0C11
#	define GL_INDEX_CLEAR_VALUE 0x0C20
#	define GL_INDEX_WRITEMASK 0x0C21
#	define GL_COLOR_CLEAR_VALUE 0x0C22
#	define GL_COLOR_WRITEMASK 0x0C23
#	define GL_INDEX_MODE 0x0C30
#	define GL_RGBA_MODE 0x0C31
#	define GL_DOUBLEBUFFER 0x0C32
#	define GL_STEREO 0x0C33
#	define GL_RENDER_MODE 0x0C40
#	define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#	define GL_POINT_SMOOTH_HINT 0x0C51
#	define GL_LINE_SMOOTH_HINT 0x0C52
#	define GL_POLYGON_SMOOTH_HINT 0x0C53
#	define GL_FOG_HINT 0x0C54
#	define GL_TEXTURE_GEN_S 0x0C60
#	define GL_TEXTURE_GEN_T 0x0C61
#	define GL_TEXTURE_GEN_R 0x0C62
#	define GL_TEXTURE_GEN_Q 0x0C63
#	define GL_PIXEL_MAP_I_TO_I 0x0C70
#	define GL_PIXEL_MAP_S_TO_S 0x0C71
#	define GL_PIXEL_MAP_I_TO_R 0x0C72
#	define GL_PIXEL_MAP_I_TO_G 0x0C73
#	define GL_PIXEL_MAP_I_TO_B 0x0C74
#	define GL_PIXEL_MAP_I_TO_A 0x0C75
#	define GL_PIXEL_MAP_R_TO_R 0x0C76
#	define GL_PIXEL_MAP_G_TO_G 0x0C77
#	define GL_PIXEL_MAP_B_TO_B 0x0C78
#	define GL_PIXEL_MAP_A_TO_A 0x0C79
#	define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#	define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#	define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#	define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#	define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#	define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#	define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#	define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#	define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#	define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#	define GL_UNPACK_SWAP_BYTES 0x0CF0
#	define GL_UNPACK_LSB_FIRST 0x0CF1
#	define GL_UNPACK_ROW_LENGTH 0x0CF2
#	define GL_UNPACK_SKIP_ROWS 0x0CF3
#	define GL_UNPACK_SKIP_PIXELS 0x0CF4
#	define GL_UNPACK_ALIGNMENT 0x0CF5
#	define GL_PACK_SWAP_BYTES 0x0D00
#	define GL_PACK_LSB_FIRST 0x0D01
#	define GL_PACK_ROW_LENGTH 0x0D02
#	define GL_PACK_SKIP_ROWS 0x0D03
#	define GL_PACK_SKIP_PIXELS 0x0D04
#	define GL_PACK_ALIGNMENT 0x0D05
#	define GL_MAP_COLOR 0x0D10
#	define GL_MAP_STENCIL 0x0D11
#	define GL_INDEX_SHIFT 0x0D12
#	define GL_INDEX_OFFSET 0x0D13
#	define GL_RED_SCALE 0x0D14
#	define GL_RED_BIAS 0x0D15
#	define GL_ZOOM_X 0x0D16
#	define GL_ZOOM_Y 0x0D17
#	define GL_GREEN_SCALE 0x0D18
#	define GL_GREEN_BIAS 0x0D19
#	define GL_BLUE_SCALE 0x0D1A
#	define GL_BLUE_BIAS 0x0D1B
#	define GL_ALPHA_SCALE 0x0D1C
#	define GL_ALPHA_BIAS 0x0D1D
#	define GL_DEPTH_SCALE 0x0D1E
#	define GL_DEPTH_BIAS 0x0D1F
#	define GL_MAX_EVAL_ORDER 0x0D30
#	define GL_MAX_LIGHTS 0x0D31
#	define GL_MAX_CLIP_PLANES 0x0D32
#	define GL_MAX_TEXTURE_SIZE 0x0D33
#	define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#	define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#	define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#	define GL_MAX_NAME_STACK_DEPTH 0x0D37
#	define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#	define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#	define GL_MAX_VIEWPORT_DIMS 0x0D3A
#	define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#	define GL_SUBPIXEL_BITS 0x0D50
#	define GL_INDEX_BITS 0x0D51
#	define GL_RED_BITS 0x0D52
#	define GL_GREEN_BITS 0x0D53
#	define GL_BLUE_BITS 0x0D54
#	define GL_ALPHA_BITS 0x0D55
#	define GL_DEPTH_BITS 0x0D56
#	define GL_STENCIL_BITS 0x0D57
#	define GL_ACCUM_RED_BITS 0x0D58
#	define GL_ACCUM_GREEN_BITS 0x0D59
#	define GL_ACCUM_BLUE_BITS 0x0D5A
#	define GL_ACCUM_ALPHA_BITS 0x0D5B
#	define GL_NAME_STACK_DEPTH 0x0D70
#	define GL_AUTO_NORMAL 0x0D80
#	define GL_MAP1_COLOR_4 0x0D90
#	define GL_MAP1_INDEX 0x0D91
#	define GL_MAP1_NORMAL 0x0D92
#	define GL_MAP1_TEXTURE_COORD_1 0x0D93
#	define GL_MAP1_TEXTURE_COORD_2 0x0D94
#	define GL_MAP1_TEXTURE_COORD_3 0x0D95
#	define GL_MAP1_TEXTURE_COORD_4 0x0D96
#	define GL_MAP1_VERTEX_3 0x0D97
#	define GL_MAP1_VERTEX_4 0x0D98
#	define GL_MAP2_COLOR_4 0x0DB0
#	define GL_MAP2_INDEX 0x0DB1
#	define GL_MAP2_NORMAL 0x0DB2
#	define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#	define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#	define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#	define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#	define GL_MAP2_VERTEX_3 0x0DB7
#	define GL_MAP2_VERTEX_4 0x0DB8
#	define GL_MAP1_GRID_DOMAIN 0x0DD0
#	define GL_MAP1_GRID_SEGMENTS 0x0DD1
#	define GL_MAP2_GRID_DOMAIN 0x0DD2
#	define GL_MAP2_GRID_SEGMENTS 0x0DD3
#	define GL_TEXTURE_1D 0x0DE0
#	define GL_TEXTURE_2D 0x0DE1
#	define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#	define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#	define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#	define GL_SELECTION_BUFFER_POINTER 0x0DF3
#	define GL_SELECTION_BUFFER_SIZE 0x0DF4
#	define GL_TEXTURE_WIDTH 0x1000
#	define GL_TEXTURE_HEIGHT 0x1001
#	define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#	define GL_TEXTURE_BORDER_COLOR 0x1004
#	define GL_TEXTURE_BORDER 0x1005
#	define GL_DONT_CARE 0x1100
#	define GL_FASTEST 0x1101
#	define GL_NICEST 0x1102
#	define GL_LIGHT0 0x4000
#	define GL_LIGHT1 0x4001
#	define GL_LIGHT2 0x4002
#	define GL_LIGHT3 0x4003
#	define GL_LIGHT4 0x4004
#	define GL_LIGHT5 0x4005
#	define GL_LIGHT6 0x4006
#	define GL_LIGHT7 0x4007
#	define GL_AMBIENT 0x1200
#	define GL_DIFFUSE 0x1201
#	define GL_SPECULAR 0x1202
#	define GL_POSITION 0x1203
#	define GL_SPOT_DIRECTION 0x1204
#	define GL_SPOT_EXPONENT 0x1205
#	define GL_SPOT_CUTOFF 0x1206
#	define GL_CONSTANT_ATTENUATION 0x1207
#	define GL_LINEAR_ATTENUATION 0x1208
#	define GL_QUADRATIC_ATTENUATION 0x1209
#	define GL_COMPILE 0x1300
#	define GL_COMPILE_AND_EXECUTE 0x1301
#	define GL_CLEAR 0x1500
#	define GL_AND 0x1501
#	define GL_AND_REVERSE 0x1502
#	define GL_COPY 0x1503
#	define GL_AND_INVERTED 0x1504
#	define GL_NOOP 0x1505
#	define GL_XOR 0x1506
#	define GL_OR 0x1507
#	define GL_NOR 0x1508
#	define GL_EQUIV 0x1509
#	define GL_INVERT 0x150A
#	define GL_OR_REVERSE 0x150B
#	define GL_COPY_INVERTED 0x150C
#	define GL_OR_INVERTED 0x150D
#	define GL_NAND 0x150E
#	define GL_SET 0x150F
#	define GL_EMISSION 0x1600
#	define GL_SHININESS 0x1601
#	define GL_AMBIENT_AND_DIFFUSE 0x1602
#	define GL_COLOR_INDEXES 0x1603
#	define GL_MODELVIEW 0x1700
#	define GL_PROJECTION 0x1701
#	define GL_TEXTURE 0x1702
#	define GL_COLOR 0x1800
#	define GL_DEPTH 0x1801
#	define GL_STENCIL 0x1802
#	define GL_COLOR_INDEX 0x1900
#	define GL_STENCIL_INDEX 0x1901
#	define GL_DEPTH_COMPONENT 0x1902
#	define GL_RED 0x1903
#	define GL_GREEN 0x1904
#	define GL_BLUE 0x1905
#	define GL_ALPHA 0x1906
#	define GL_RGB 0x1907
#	define GL_RGBA 0x1908
#	define GL_LUMINANCE 0x1909
#	define GL_LUMINANCE_ALPHA 0x190A
#	define GL_BITMAP 0x1A00
#	define GL_POINT 0x1B00
#	define GL_LINE 0x1B01
#	define GL_FILL 0x1B02
#	define GL_RENDER 0x1C00
#	define GL_FEEDBACK 0x1C01
#	define GL_SELECT 0x1C02
#	define GL_FLAT 0x1D00
#	define GL_SMOOTH 0x1D01
#	define GL_KEEP 0x1E00
#	define GL_REPLACE 0x1E01
#	define GL_INCR 0x1E02
#	define GL_DECR 0x1E03
#	define GL_VENDOR 0x1F00
#	define GL_RENDERER 0x1F01
#	define GL_VERSION 0x1F02
#	define GL_EXTENSIONS 0x1F03
#	define GL_S 0x2000
#	define GL_T 0x2001
#	define GL_R 0x2002
#	define GL_Q 0x2003
#	define GL_MODULATE 0x2100
#	define GL_DECAL 0x2101
#	define GL_TEXTURE_ENV_MODE 0x2200
#	define GL_TEXTURE_ENV_COLOR 0x2201
#	define GL_TEXTURE_ENV 0x2300
#	define GL_EYE_LINEAR 0x2400
#	define GL_OBJECT_LINEAR 0x2401
#	define GL_SPHERE_MAP 0x2402
#	define GL_TEXTURE_GEN_MODE 0x2500
#	define GL_OBJECT_PLANE 0x2501
#	define GL_EYE_PLANE 0x2502
#	define GL_NEAREST 0x2600
#	define GL_LINEAR 0x2601
#	define GL_NEAREST_MIPMAP_NEAREST 0x2700
#	define GL_LINEAR_MIPMAP_NEAREST 0x2701
#	define GL_NEAREST_MIPMAP_LINEAR 0x2702
#	define GL_LINEAR_MIPMAP_LINEAR 0x2703
#	define GL_TEXTURE_MAG_FILTER 0x2800
#	define GL_TEXTURE_MIN_FILTER 0x2801
#	define GL_TEXTURE_WRAP_S 0x2802
#	define GL_TEXTURE_WRAP_T 0x2803
#	define GL_CLAMP 0x2900
#	define GL_REPEAT 0x2901
#	define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#	define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#	define GL_CLIENT_ALL_ATTRIB_BITS 0xffffffff
#	define GL_POLYGON_OFFSET_FACTOR 0x8038
#	define GL_POLYGON_OFFSET_UNITS 0x2A00
#	define GL_POLYGON_OFFSET_POINT 0x2A01
#	define GL_POLYGON_OFFSET_LINE 0x2A02
#	define GL_POLYGON_OFFSET_FILL 0x8037
#	define GL_ALPHA4 0x803B
#	define GL_ALPHA8 0x803C
#	define GL_ALPHA12 0x803D
#	define GL_ALPHA16 0x803E
#	define GL_LUMINANCE4 0x803F
#	define GL_LUMINANCE8 0x8040
#	define GL_LUMINANCE12 0x8041
#	define GL_LUMINANCE16 0x8042
#	define GL_LUMINANCE4_ALPHA4 0x8043
#	define GL_LUMINANCE6_ALPHA2 0x8044
#	define GL_LUMINANCE8_ALPHA8 0x8045
#	define GL_LUMINANCE12_ALPHA4 0x8046
#	define GL_LUMINANCE12_ALPHA12 0x8047
#	define GL_LUMINANCE16_ALPHA16 0x8048
#	define GL_INTENSITY 0x8049
#	define GL_INTENSITY4 0x804A
#	define GL_INTENSITY8 0x804B
#	define GL_INTENSITY12 0x804C
#	define GL_INTENSITY16 0x804D
#	define GL_R3_G3_B2 0x2A10
#	define GL_RGB4 0x804F
#	define GL_RGB5 0x8050
#	define GL_RGB8 0x8051
#	define GL_RGB10 0x8052
#	define GL_RGB12 0x8053
#	define GL_RGB16 0x8054
#	define GL_RGBA2 0x8055
#	define GL_RGBA4 0x8056
#	define GL_RGB5_A1 0x8057
#	define GL_RGBA8 0x8058
#	define GL_RGB10_A2 0x8059
#	define GL_RGBA12 0x805A
#	define GL_RGBA16 0x805B
#	define GL_TEXTURE_RED_SIZE 0x805C
#	define GL_TEXTURE_GREEN_SIZE 0x805D
#	define GL_TEXTURE_BLUE_SIZE 0x805E
#	define GL_TEXTURE_ALPHA_SIZE 0x805F
#	define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#	define GL_TEXTURE_INTENSITY_SIZE 0x8061
#	define GL_PROXY_TEXTURE_1D 0x8063
#	define GL_PROXY_TEXTURE_2D 0x8064
#	define GL_TEXTURE_PRIORITY 0x8066
#	define GL_TEXTURE_RESIDENT 0x8067
#	define GL_TEXTURE_BINDING_1D 0x8068
#	define GL_TEXTURE_BINDING_2D 0x8069
#	define GL_VERTEX_ARRAY 0x8074
#	define GL_NORMAL_ARRAY 0x8075
#	define GL_COLOR_ARRAY 0x8076
#	define GL_INDEX_ARRAY 0x8077
#	define GL_TEXTURE_COORD_ARRAY 0x8078
#	define GL_EDGE_FLAG_ARRAY 0x8079
#	define GL_VERTEX_ARRAY_SIZE 0x807A
#	define GL_VERTEX_ARRAY_TYPE 0x807B
#	define GL_VERTEX_ARRAY_STRIDE 0x807C
#	define GL_NORMAL_ARRAY_TYPE 0x807E
#	define GL_NORMAL_ARRAY_STRIDE 0x807F
#	define GL_COLOR_ARRAY_SIZE 0x8081
#	define GL_COLOR_ARRAY_TYPE 0x8082
#	define GL_COLOR_ARRAY_STRIDE 0x8083
#	define GL_INDEX_ARRAY_TYPE 0x8085
#	define GL_INDEX_ARRAY_STRIDE 0x8086
#	define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#	define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#	define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#	define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#	define GL_VERTEX_ARRAY_POINTER 0x808E
#	define GL_NORMAL_ARRAY_POINTER 0x808F
#	define GL_COLOR_ARRAY_POINTER 0x8090
#	define GL_INDEX_ARRAY_POINTER 0x8091
#	define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#	define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#	define GL_V2F 0x2A20
#	define GL_V3F 0x2A21
#	define GL_C4UB_V2F 0x2A22
#	define GL_C4UB_V3F 0x2A23
#	define GL_C3F_V3F 0x2A24
#	define GL_N3F_V3F 0x2A25
#	define GL_C4F_N3F_V3F 0x2A26
#	define GL_T2F_V3F 0x2A27
#	define GL_T4F_V4F 0x2A28
#	define GL_T2F_C4UB_V3F 0x2A29
#	define GL_T2F_C3F_V3F 0x2A2A
#	define GL_T2F_N3F_V3F 0x2A2B
#	define GL_T2F_C4F_N3F_V3F 0x2A2C
#	define GL_T4F_C4F_N3F_V4F 0x2A2D
#	define GL_EXT_vertex_array 1
#	define GL_EXT_bgra 1
#	define GL_EXT_paletted_texture 1
#	define GL_WIN_swap_hint 1
#	define GL_WIN_draw_range_elements 1
#	define GL_WIN_phong_shading 1
#	define GL_WIN_specular_fog 1
#	define GL_VERTEX_ARRAY_EXT 0x8074
#	define GL_NORMAL_ARRAY_EXT 0x8075
#	define GL_COLOR_ARRAY_EXT 0x8076
#	define GL_INDEX_ARRAY_EXT 0x8077
#	define GL_TEXTURE_COORD_ARRAY_EXT 0x8078
#	define GL_EDGE_FLAG_ARRAY_EXT 0x8079
#	define GL_VERTEX_ARRAY_SIZE_EXT 0x807A
#	define GL_VERTEX_ARRAY_TYPE_EXT 0x807B
#	define GL_VERTEX_ARRAY_STRIDE_EXT 0x807C
#	define GL_VERTEX_ARRAY_COUNT_EXT 0x807D
#	define GL_NORMAL_ARRAY_TYPE_EXT 0x807E
#	define GL_NORMAL_ARRAY_STRIDE_EXT 0x807F
#	define GL_NORMAL_ARRAY_COUNT_EXT 0x8080
#	define GL_COLOR_ARRAY_SIZE_EXT 0x8081
#	define GL_COLOR_ARRAY_TYPE_EXT 0x8082
#	define GL_COLOR_ARRAY_STRIDE_EXT 0x8083
#	define GL_COLOR_ARRAY_COUNT_EXT 0x8084
#	define GL_INDEX_ARRAY_TYPE_EXT 0x8085
#	define GL_INDEX_ARRAY_STRIDE_EXT 0x8086
#	define GL_INDEX_ARRAY_COUNT_EXT 0x8087
#	define GL_TEXTURE_COORD_ARRAY_SIZE_EXT 0x8088
#	define GL_TEXTURE_COORD_ARRAY_TYPE_EXT 0x8089
#	define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#	define GL_TEXTURE_COORD_ARRAY_COUNT_EXT 0x808B
#	define GL_EDGE_FLAG_ARRAY_STRIDE_EXT 0x808C
#	define GL_EDGE_FLAG_ARRAY_COUNT_EXT 0x808D
#	define GL_VERTEX_ARRAY_POINTER_EXT 0x808E
#	define GL_NORMAL_ARRAY_POINTER_EXT 0x808F
#	define GL_COLOR_ARRAY_POINTER_EXT 0x8090
#	define GL_INDEX_ARRAY_POINTER_EXT 0x8091
#	define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#	define GL_EDGE_FLAG_ARRAY_POINTER_EXT 0x8093
#	define GL_DOUBLE_EXT GL_DOUBLE
#	define GL_BGR_EXT 0x80E0
#	define GL_BGRA_EXT 0x80E1
#	define GL_COLOR_TABLE_FORMAT_EXT 0x80D8
#	define GL_COLOR_TABLE_WIDTH_EXT 0x80D9
#	define GL_COLOR_TABLE_RED_SIZE_EXT 0x80DA
#	define GL_COLOR_TABLE_GREEN_SIZE_EXT 0x80DB
#	define GL_COLOR_TABLE_BLUE_SIZE_EXT 0x80DC
#	define GL_COLOR_TABLE_ALPHA_SIZE_EXT 0x80DD
#	define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#	define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF
#	define GL_COLOR_INDEX1_EXT 0x80E2
#	define GL_COLOR_INDEX2_EXT 0x80E3
#	define GL_COLOR_INDEX4_EXT 0x80E4
#	define GL_COLOR_INDEX8_EXT 0x80E5
#	define GL_COLOR_INDEX12_EXT 0x80E6
#	define GL_COLOR_INDEX16_EXT 0x80E7
#	define GL_MAX_ELEMENTS_VERTICES_WIN 0x80E8
#	define GL_MAX_ELEMENTS_INDICES_WIN 0x80E9
#	define GL_PHONG_WIN 0x80EA
#	define GL_PHONG_HINT_WIN 0x80EB
#	define GL_FOG_SPECULAR_TEXTURE_WIN 0x80EC
#	define GL_LOGIC_OP GL_INDEX_LOGIC_OP
#	define GL_TEXTURE_COMPONENTS GL_TEXTURE_INTERNAL_FORMAT
	typedef void (FGL_APIENTRY gl_accum_func)(GLenum op, GLfloat value);
	static gl_accum_func* glAccum;
	typedef void (FGL_APIENTRY gl_alpha_func_func)(GLenum func, GLclampf ref);
	static gl_alpha_func_func* glAlphaFunc;
	typedef GLboolean(FGL_APIENTRY gl_are_textures_resident_func)(GLsizei n, const GLuint *textures, GLboolean *residences);
	static gl_are_textures_resident_func* glAreTexturesResident;
	typedef void (FGL_APIENTRY gl_array_element_func)(GLint i);
	static gl_array_element_func* glArrayElement;
	typedef void (FGL_APIENTRY gl_begin_func)(GLenum mode);
	static gl_begin_func* glBegin;
	typedef void (FGL_APIENTRY gl_bind_texture_func)(GLenum target, GLuint texture);
	static gl_bind_texture_func* glBindTexture;
	typedef void (FGL_APIENTRY gl_bitmap_func)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
	static gl_bitmap_func* glBitmap;
	typedef void (FGL_APIENTRY gl_blend_func_func)(GLenum sfactor, GLenum dfactor);
	static gl_blend_func_func* glBlendFunc;
	typedef void (FGL_APIENTRY gl_call_list_func)(GLuint list);
	static gl_call_list_func* glCallList;
	typedef void (FGL_APIENTRY gl_call_lists_func)(GLsizei n, GLenum type, const GLvoid *lists);
	static gl_call_lists_func* glCallLists;
	typedef void (FGL_APIENTRY gl_clear_func)(GLbitfield mask);
	static gl_clear_func* glClear;
	typedef void (FGL_APIENTRY gl_clear_accum_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_clear_accum_func* glClearAccum;
	typedef void (FGL_APIENTRY gl_clear_color_func)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	static gl_clear_color_func* glClearColor;
	typedef void (FGL_APIENTRY gl_clear_depth_func)(GLclampd depth);
	static gl_clear_depth_func* glClearDepth;
	typedef void (FGL_APIENTRY gl_clear_index_func)(GLfloat c);
	static gl_clear_index_func* glClearIndex;
	typedef void (FGL_APIENTRY gl_clear_stencil_func)(GLint s);
	static gl_clear_stencil_func* glClearStencil;
	typedef void (FGL_APIENTRY gl_clip_plane_func)(GLenum plane, const GLdouble *equation);
	static gl_clip_plane_func* glClipPlane;
	typedef void (FGL_APIENTRY gl_color3b_func)(GLbyte red, GLbyte green, GLbyte blue);
	static gl_color3b_func* glColor3b;
	typedef void (FGL_APIENTRY gl_color3bv_func)(const GLbyte *v);
	static gl_color3bv_func* glColor3bv;
	typedef void (FGL_APIENTRY gl_color3d_func)(GLdouble red, GLdouble green, GLdouble blue);
	static gl_color3d_func* glColor3d;
	typedef void (FGL_APIENTRY gl_color3dv_func)(const GLdouble *v);
	static gl_color3dv_func* glColor3dv;
	typedef void (FGL_APIENTRY gl_color3f_func)(GLfloat red, GLfloat green, GLfloat blue);
	static gl_color3f_func* glColor3f;
	typedef void (FGL_APIENTRY gl_color3fv_func)(const GLfloat *v);
	static gl_color3fv_func* glColor3fv;
	typedef void (FGL_APIENTRY gl_color3i_func)(GLint red, GLint green, GLint blue);
	static gl_color3i_func* glColor3i;
	typedef void (FGL_APIENTRY gl_color3iv_func)(const GLint *v);
	static gl_color3iv_func* glColor3iv;
	typedef void (FGL_APIENTRY gl_color3s_func)(GLshort red, GLshort green, GLshort blue);
	static gl_color3s_func* glColor3s;
	typedef void (FGL_APIENTRY gl_color3sv_func)(const GLshort *v);
	static gl_color3sv_func* glColor3sv;
	typedef void (FGL_APIENTRY gl_color3ub_func)(GLubyte red, GLubyte green, GLubyte blue);
	static gl_color3ub_func* glColor3ub;
	typedef void (FGL_APIENTRY gl_color3ubv_func)(const GLubyte *v);
	static gl_color3ubv_func* glColor3ubv;
	typedef void (FGL_APIENTRY gl_color3ui_func)(GLuint red, GLuint green, GLuint blue);
	static gl_color3ui_func* glColor3ui;
	typedef void (FGL_APIENTRY gl_color3uiv_func)(const GLuint *v);
	static gl_color3uiv_func* glColor3uiv;
	typedef void (FGL_APIENTRY gl_color3us_func)(GLushort red, GLushort green, GLushort blue);
	static gl_color3us_func* glColor3us;
	typedef void (FGL_APIENTRY gl_color3usv_func)(const GLushort *v);
	static gl_color3usv_func* glColor3usv;
	typedef void (FGL_APIENTRY gl_color4b_func)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	static gl_color4b_func* glColor4b;
	typedef void (FGL_APIENTRY gl_color4bv_func)(const GLbyte *v);
	static gl_color4bv_func* glColor4bv;
	typedef void (FGL_APIENTRY gl_color4d_func)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	static gl_color4d_func* glColor4d;
	typedef void (FGL_APIENTRY gl_color4dv_func)(const GLdouble *v);
	static gl_color4dv_func* glColor4dv;
	typedef void (FGL_APIENTRY gl_color4f_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_color4f_func* glColor4f;
	typedef void (FGL_APIENTRY gl_color4fv_func)(const GLfloat *v);
	static gl_color4fv_func* glColor4fv;
	typedef void (FGL_APIENTRY gl_color4i_func)(GLint red, GLint green, GLint blue, GLint alpha);
	static gl_color4i_func* glColor4i;
	typedef void (FGL_APIENTRY gl_color4iv_func)(const GLint *v);
	static gl_color4iv_func* glColor4iv;
	typedef void (FGL_APIENTRY gl_color4s_func)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
	static gl_color4s_func* glColor4s;
	typedef void (FGL_APIENTRY gl_color4sv_func)(const GLshort *v);
	static gl_color4sv_func* glColor4sv;
	typedef void (FGL_APIENTRY gl_color4ub_func)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	static gl_color4ub_func* glColor4ub;
	typedef void (FGL_APIENTRY gl_color4ubv_func)(const GLubyte *v);
	static gl_color4ubv_func* glColor4ubv;
	typedef void (FGL_APIENTRY gl_color4ui_func)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
	static gl_color4ui_func* glColor4ui;
	typedef void (FGL_APIENTRY gl_color4uiv_func)(const GLuint *v);
	static gl_color4uiv_func* glColor4uiv;
	typedef void (FGL_APIENTRY gl_color4us_func)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
	static gl_color4us_func* glColor4us;
	typedef void (FGL_APIENTRY gl_color4usv_func)(const GLushort *v);
	static gl_color4usv_func* glColor4usv;
	typedef void (FGL_APIENTRY gl_color_mask_func)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	static gl_color_mask_func* glColorMask;
	typedef void (FGL_APIENTRY gl_color_material_func)(GLenum face, GLenum mode);
	static gl_color_material_func* glColorMaterial;
	typedef void (FGL_APIENTRY gl_color_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_color_pointer_func* glColorPointer;
	typedef void (FGL_APIENTRY gl_copy_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
	static gl_copy_pixels_func* glCopyPixels;
	typedef void (FGL_APIENTRY gl_copy_tex_image1d_func)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
	static gl_copy_tex_image1d_func* glCopyTexImage1D;
	typedef void (FGL_APIENTRY gl_copy_tex_image2d_func)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	static gl_copy_tex_image2d_func* glCopyTexImage2D;
	typedef void (FGL_APIENTRY gl_copy_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static gl_copy_tex_sub_image1d_func* glCopyTexSubImage1D;
	typedef void (FGL_APIENTRY gl_copy_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_tex_sub_image2d_func* glCopyTexSubImage2D;
	typedef void (FGL_APIENTRY gl_cull_face_func)(GLenum mode);
	static gl_cull_face_func* glCullFace;
	typedef void (FGL_APIENTRY gl_delete_lists_func)(GLuint list, GLsizei range);
	static gl_delete_lists_func* glDeleteLists;
	typedef void (FGL_APIENTRY gl_delete_textures_func)(GLsizei n, const GLuint *textures);
	static gl_delete_textures_func* glDeleteTextures;
	typedef void (FGL_APIENTRY gl_depth_func_func)(GLenum func);
	static gl_depth_func_func* glDepthFunc;
	typedef void (FGL_APIENTRY gl_depth_mask_func)(GLboolean flag);
	static gl_depth_mask_func* glDepthMask;
	typedef void (FGL_APIENTRY gl_depth_range_func)(GLclampd zNear, GLclampd zFar);
	static gl_depth_range_func* glDepthRange;
	typedef void (FGL_APIENTRY gl_disable_func)(GLenum cap);
	static gl_disable_func* glDisable;
	typedef void (FGL_APIENTRY gl_disable_client_state_func)(GLenum array);
	static gl_disable_client_state_func* glDisableClientState;
	typedef void (FGL_APIENTRY gl_draw_arrays_func)(GLenum mode, GLint first, GLsizei count);
	static gl_draw_arrays_func* glDrawArrays;
	typedef void (FGL_APIENTRY gl_draw_buffer_func)(GLenum mode);
	static gl_draw_buffer_func* glDrawBuffer;
	typedef void (FGL_APIENTRY gl_draw_elements_func)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	static gl_draw_elements_func* glDrawElements;
	typedef void (FGL_APIENTRY gl_draw_pixels_func)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_draw_pixels_func* glDrawPixels;
	typedef void (FGL_APIENTRY gl_edge_flag_func)(GLboolean flag);
	static gl_edge_flag_func* glEdgeFlag;
	typedef void (FGL_APIENTRY gl_edge_flag_pointer_func)(GLsizei stride, const GLvoid *pointer);
	static gl_edge_flag_pointer_func* glEdgeFlagPointer;
	typedef void (FGL_APIENTRY gl_edge_flagv_func)(const GLboolean *flag);
	static gl_edge_flagv_func* glEdgeFlagv;
	typedef void (FGL_APIENTRY gl_enable_func)(GLenum cap);
	static gl_enable_func* glEnable;
	typedef void (FGL_APIENTRY gl_enable_client_state_func)(GLenum array);
	static gl_enable_client_state_func* glEnableClientState;
	typedef void (FGL_APIENTRY gl_end_func)(void);
	static gl_end_func* glEnd;
	typedef void (FGL_APIENTRY gl_end_list_func)(void);
	static gl_end_list_func* glEndList;
	typedef void (FGL_APIENTRY gl_eval_coord1d_func)(GLdouble u);
	static gl_eval_coord1d_func* glEvalCoord1d;
	typedef void (FGL_APIENTRY gl_eval_coord1dv_func)(const GLdouble *u);
	static gl_eval_coord1dv_func* glEvalCoord1dv;
	typedef void (FGL_APIENTRY gl_eval_coord1f_func)(GLfloat u);
	static gl_eval_coord1f_func* glEvalCoord1f;
	typedef void (FGL_APIENTRY gl_eval_coord1fv_func)(const GLfloat *u);
	static gl_eval_coord1fv_func* glEvalCoord1fv;
	typedef void (FGL_APIENTRY gl_eval_coord2d_func)(GLdouble u, GLdouble v);
	static gl_eval_coord2d_func* glEvalCoord2d;
	typedef void (FGL_APIENTRY gl_eval_coord2dv_func)(const GLdouble *u);
	static gl_eval_coord2dv_func* glEvalCoord2dv;
	typedef void (FGL_APIENTRY gl_eval_coord2f_func)(GLfloat u, GLfloat v);
	static gl_eval_coord2f_func* glEvalCoord2f;
	typedef void (FGL_APIENTRY gl_eval_coord2fv_func)(const GLfloat *u);
	static gl_eval_coord2fv_func* glEvalCoord2fv;
	typedef void (FGL_APIENTRY gl_eval_mesh1_func)(GLenum mode, GLint i1, GLint i2);
	static gl_eval_mesh1_func* glEvalMesh1;
	typedef void (FGL_APIENTRY gl_eval_mesh2_func)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
	static gl_eval_mesh2_func* glEvalMesh2;
	typedef void (FGL_APIENTRY gl_eval_point1_func)(GLint i);
	static gl_eval_point1_func* glEvalPoint1;
	typedef void (FGL_APIENTRY gl_eval_point2_func)(GLint i, GLint j);
	static gl_eval_point2_func* glEvalPoint2;
	typedef void (FGL_APIENTRY gl_feedback_buffer_func)(GLsizei size, GLenum type, GLfloat *buffer);
	static gl_feedback_buffer_func* glFeedbackBuffer;
	typedef void (FGL_APIENTRY gl_finish_func)(void);
	static gl_finish_func* glFinish;
	typedef void (FGL_APIENTRY gl_flush_func)(void);
	static gl_flush_func* glFlush;
	typedef void (FGL_APIENTRY gl_fogf_func)(GLenum pname, GLfloat param);
	static gl_fogf_func* glFogf;
	typedef void (FGL_APIENTRY gl_fogfv_func)(GLenum pname, const GLfloat *params);
	static gl_fogfv_func* glFogfv;
	typedef void (FGL_APIENTRY gl_fogi_func)(GLenum pname, GLint param);
	static gl_fogi_func* glFogi;
	typedef void (FGL_APIENTRY gl_fogiv_func)(GLenum pname, const GLint *params);
	static gl_fogiv_func* glFogiv;
	typedef void (FGL_APIENTRY gl_front_face_func)(GLenum mode);
	static gl_front_face_func* glFrontFace;
	typedef void (FGL_APIENTRY gl_frustum_func)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static gl_frustum_func* glFrustum;
	typedef GLuint(FGL_APIENTRY gl_gen_lists_func)(GLsizei range);
	static gl_gen_lists_func* glGenLists;
	typedef void (FGL_APIENTRY gl_gen_textures_func)(GLsizei n, GLuint *textures);
	static gl_gen_textures_func* glGenTextures;
	typedef void (FGL_APIENTRY gl_get_booleanv_func)(GLenum pname, GLboolean *params);
	static gl_get_booleanv_func* glGetBooleanv;
	typedef void (FGL_APIENTRY gl_get_clip_plane_func)(GLenum plane, GLdouble *equation);
	static gl_get_clip_plane_func* glGetClipPlane;
	typedef void (FGL_APIENTRY gl_get_doublev_func)(GLenum pname, GLdouble *params);
	static gl_get_doublev_func* glGetDoublev;
	typedef GLenum(FGL_APIENTRY gl_get_error_func)(void);
	static gl_get_error_func* glGetError;
	typedef void (FGL_APIENTRY gl_get_floatv_func)(GLenum pname, GLfloat *params);
	static gl_get_floatv_func* glGetFloatv;
	typedef void (FGL_APIENTRY gl_get_integerv_func)(GLenum pname, GLint *params);
	static gl_get_integerv_func* glGetIntegerv;
	typedef void (FGL_APIENTRY gl_get_lightfv_func)(GLenum light, GLenum pname, GLfloat *params);
	static gl_get_lightfv_func* glGetLightfv;
	typedef void (FGL_APIENTRY gl_get_lightiv_func)(GLenum light, GLenum pname, GLint *params);
	static gl_get_lightiv_func* glGetLightiv;
	typedef void (FGL_APIENTRY gl_get_mapdv_func)(GLenum target, GLenum query, GLdouble *v);
	static gl_get_mapdv_func* glGetMapdv;
	typedef void (FGL_APIENTRY gl_get_mapfv_func)(GLenum target, GLenum query, GLfloat *v);
	static gl_get_mapfv_func* glGetMapfv;
	typedef void (FGL_APIENTRY gl_get_mapiv_func)(GLenum target, GLenum query, GLint *v);
	static gl_get_mapiv_func* glGetMapiv;
	typedef void (FGL_APIENTRY gl_get_materialfv_func)(GLenum face, GLenum pname, GLfloat *params);
	static gl_get_materialfv_func* glGetMaterialfv;
	typedef void (FGL_APIENTRY gl_get_materialiv_func)(GLenum face, GLenum pname, GLint *params);
	static gl_get_materialiv_func* glGetMaterialiv;
	typedef void (FGL_APIENTRY gl_get_pixel_mapfv_func)(GLenum map, GLfloat *values);
	static gl_get_pixel_mapfv_func* glGetPixelMapfv;
	typedef void (FGL_APIENTRY gl_get_pixel_mapuiv_func)(GLenum map, GLuint *values);
	static gl_get_pixel_mapuiv_func* glGetPixelMapuiv;
	typedef void (FGL_APIENTRY gl_get_pixel_mapusv_func)(GLenum map, GLushort *values);
	static gl_get_pixel_mapusv_func* glGetPixelMapusv;
	typedef void (FGL_APIENTRY gl_get_pointerv_func)(GLenum pname, GLvoid* *params);
	static gl_get_pointerv_func* glGetPointerv;
	typedef void (FGL_APIENTRY gl_get_polygon_stipple_func)(GLubyte *mask);
	static gl_get_polygon_stipple_func* glGetPolygonStipple;
	typedef const GLubyte * (FGL_APIENTRY gl_get_string_func)(GLenum name);
	static gl_get_string_func* glGetString;
	typedef void (FGL_APIENTRY gl_get_tex_envfv_func)(GLenum target, GLenum pname, GLfloat *params);
	static gl_get_tex_envfv_func* glGetTexEnvfv;
	typedef void (FGL_APIENTRY gl_get_tex_enviv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_enviv_func* glGetTexEnviv;
	typedef void (FGL_APIENTRY gl_get_tex_gendv_func)(GLenum coord, GLenum pname, GLdouble *params);
	static gl_get_tex_gendv_func* glGetTexGendv;
	typedef void (FGL_APIENTRY gl_get_tex_genfv_func)(GLenum coord, GLenum pname, GLfloat *params);
	static gl_get_tex_genfv_func* glGetTexGenfv;
	typedef void (FGL_APIENTRY gl_get_tex_geniv_func)(GLenum coord, GLenum pname, GLint *params);
	static gl_get_tex_geniv_func* glGetTexGeniv;
	typedef void (FGL_APIENTRY gl_get_tex_image_func)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	static gl_get_tex_image_func* glGetTexImage;
	typedef void (FGL_APIENTRY gl_get_tex_level_parameterfv_func)(GLenum target, GLint level, GLenum pname, GLfloat *params);
	static gl_get_tex_level_parameterfv_func* glGetTexLevelParameterfv;
	typedef void (FGL_APIENTRY gl_get_tex_level_parameteriv_func)(GLenum target, GLint level, GLenum pname, GLint *params);
	static gl_get_tex_level_parameteriv_func* glGetTexLevelParameteriv;
	typedef void (FGL_APIENTRY gl_get_tex_parameterfv_func)(GLenum target, GLenum pname, GLfloat *params);
	static gl_get_tex_parameterfv_func* glGetTexParameterfv;
	typedef void (FGL_APIENTRY gl_get_tex_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_parameteriv_func* glGetTexParameteriv;
	typedef void (FGL_APIENTRY gl_hint_func)(GLenum target, GLenum mode);
	static gl_hint_func* glHint;
	typedef void (FGL_APIENTRY gl_index_mask_func)(GLuint mask);
	static gl_index_mask_func* glIndexMask;
	typedef void (FGL_APIENTRY gl_index_pointer_func)(GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_index_pointer_func* glIndexPointer;
	typedef void (FGL_APIENTRY gl_indexd_func)(GLdouble c);
	static gl_indexd_func* glIndexd;
	typedef void (FGL_APIENTRY gl_indexdv_func)(const GLdouble *c);
	static gl_indexdv_func* glIndexdv;
	typedef void (FGL_APIENTRY gl_indexf_func)(GLfloat c);
	static gl_indexf_func* glIndexf;
	typedef void (FGL_APIENTRY gl_indexfv_func)(const GLfloat *c);
	static gl_indexfv_func* glIndexfv;
	typedef void (FGL_APIENTRY gl_indexi_func)(GLint c);
	static gl_indexi_func* glIndexi;
	typedef void (FGL_APIENTRY gl_indexiv_func)(const GLint *c);
	static gl_indexiv_func* glIndexiv;
	typedef void (FGL_APIENTRY gl_indexs_func)(GLshort c);
	static gl_indexs_func* glIndexs;
	typedef void (FGL_APIENTRY gl_indexsv_func)(const GLshort *c);
	static gl_indexsv_func* glIndexsv;
	typedef void (FGL_APIENTRY gl_indexub_func)(GLubyte c);
	static gl_indexub_func* glIndexub;
	typedef void (FGL_APIENTRY gl_indexubv_func)(const GLubyte *c);
	static gl_indexubv_func* glIndexubv;
	typedef void (FGL_APIENTRY gl_init_names_func)(void);
	static gl_init_names_func* glInitNames;
	typedef void (FGL_APIENTRY gl_interleaved_arrays_func)(GLenum format, GLsizei stride, const GLvoid *pointer);
	static gl_interleaved_arrays_func* glInterleavedArrays;
	typedef GLboolean(FGL_APIENTRY gl_is_enabled_func)(GLenum cap);
	static gl_is_enabled_func* glIsEnabled;
	typedef GLboolean(FGL_APIENTRY gl_is_list_func)(GLuint list);
	static gl_is_list_func* glIsList;
	typedef GLboolean(FGL_APIENTRY gl_is_texture_func)(GLuint texture);
	static gl_is_texture_func* glIsTexture;
	typedef void (FGL_APIENTRY gl_light_modelf_func)(GLenum pname, GLfloat param);
	static gl_light_modelf_func* glLightModelf;
	typedef void (FGL_APIENTRY gl_light_modelfv_func)(GLenum pname, const GLfloat *params);
	static gl_light_modelfv_func* glLightModelfv;
	typedef void (FGL_APIENTRY gl_light_modeli_func)(GLenum pname, GLint param);
	static gl_light_modeli_func* glLightModeli;
	typedef void (FGL_APIENTRY gl_light_modeliv_func)(GLenum pname, const GLint *params);
	static gl_light_modeliv_func* glLightModeliv;
	typedef void (FGL_APIENTRY gl_lightf_func)(GLenum light, GLenum pname, GLfloat param);
	static gl_lightf_func* glLightf;
	typedef void (FGL_APIENTRY gl_lightfv_func)(GLenum light, GLenum pname, const GLfloat *params);
	static gl_lightfv_func* glLightfv;
	typedef void (FGL_APIENTRY gl_lighti_func)(GLenum light, GLenum pname, GLint param);
	static gl_lighti_func* glLighti;
	typedef void (FGL_APIENTRY gl_lightiv_func)(GLenum light, GLenum pname, const GLint *params);
	static gl_lightiv_func* glLightiv;
	typedef void (FGL_APIENTRY gl_line_stipple_func)(GLint factor, GLushort pattern);
	static gl_line_stipple_func* glLineStipple;
	typedef void (FGL_APIENTRY gl_line_width_func)(GLfloat width);
	static gl_line_width_func* glLineWidth;
	typedef void (FGL_APIENTRY gl_list_base_func)(GLuint base);
	static gl_list_base_func* glListBase;
	typedef void (FGL_APIENTRY gl_load_identity_func)(void);
	static gl_load_identity_func* glLoadIdentity;
	typedef void (FGL_APIENTRY gl_load_matrixd_func)(const GLdouble *m);
	static gl_load_matrixd_func* glLoadMatrixd;
	typedef void (FGL_APIENTRY gl_load_matrixf_func)(const GLfloat *m);
	static gl_load_matrixf_func* glLoadMatrixf;
	typedef void (FGL_APIENTRY gl_load_name_func)(GLuint name);
	static gl_load_name_func* glLoadName;
	typedef void (FGL_APIENTRY gl_logic_op_func)(GLenum opcode);
	static gl_logic_op_func* glLogicOp;
	typedef void (FGL_APIENTRY gl_map1d_func)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
	static gl_map1d_func* glMap1d;
	typedef void (FGL_APIENTRY gl_map1f_func)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
	static gl_map1f_func* glMap1f;
	typedef void (FGL_APIENTRY gl_map2d_func)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
	static gl_map2d_func* glMap2d;
	typedef void (FGL_APIENTRY gl_map2f_func)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
	static gl_map2f_func* glMap2f;
	typedef void (FGL_APIENTRY gl_map_grid1d_func)(GLint un, GLdouble u1, GLdouble u2);
	static gl_map_grid1d_func* glMapGrid1d;
	typedef void (FGL_APIENTRY gl_map_grid1f_func)(GLint un, GLfloat u1, GLfloat u2);
	static gl_map_grid1f_func* glMapGrid1f;
	typedef void (FGL_APIENTRY gl_map_grid2d_func)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	static gl_map_grid2d_func* glMapGrid2d;
	typedef void (FGL_APIENTRY gl_map_grid2f_func)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	static gl_map_grid2f_func* glMapGrid2f;
	typedef void (FGL_APIENTRY gl_materialf_func)(GLenum face, GLenum pname, GLfloat param);
	static gl_materialf_func* glMaterialf;
	typedef void (FGL_APIENTRY gl_materialfv_func)(GLenum face, GLenum pname, const GLfloat *params);
	static gl_materialfv_func* glMaterialfv;
	typedef void (FGL_APIENTRY gl_materiali_func)(GLenum face, GLenum pname, GLint param);
	static gl_materiali_func* glMateriali;
	typedef void (FGL_APIENTRY gl_materialiv_func)(GLenum face, GLenum pname, const GLint *params);
	static gl_materialiv_func* glMaterialiv;
	typedef void (FGL_APIENTRY gl_matrix_mode_func)(GLenum mode);
	static gl_matrix_mode_func* glMatrixMode;
	typedef void (FGL_APIENTRY gl_mult_matrixd_func)(const GLdouble *m);
	static gl_mult_matrixd_func* glMultMatrixd;
	typedef void (FGL_APIENTRY gl_mult_matrixf_func)(const GLfloat *m);
	static gl_mult_matrixf_func* glMultMatrixf;
	typedef void (FGL_APIENTRY gl_new_list_func)(GLuint list, GLenum mode);
	static gl_new_list_func* glNewList;
	typedef void (FGL_APIENTRY gl_normal3b_func)(GLbyte nx, GLbyte ny, GLbyte nz);
	static gl_normal3b_func* glNormal3b;
	typedef void (FGL_APIENTRY gl_normal3bv_func)(const GLbyte *v);
	static gl_normal3bv_func* glNormal3bv;
	typedef void (FGL_APIENTRY gl_normal3d_func)(GLdouble nx, GLdouble ny, GLdouble nz);
	static gl_normal3d_func* glNormal3d;
	typedef void (FGL_APIENTRY gl_normal3dv_func)(const GLdouble *v);
	static gl_normal3dv_func* glNormal3dv;
	typedef void (FGL_APIENTRY gl_normal3f_func)(GLfloat nx, GLfloat ny, GLfloat nz);
	static gl_normal3f_func* glNormal3f;
	typedef void (FGL_APIENTRY gl_normal3fv_func)(const GLfloat *v);
	static gl_normal3fv_func* glNormal3fv;
	typedef void (FGL_APIENTRY gl_normal3i_func)(GLint nx, GLint ny, GLint nz);
	static gl_normal3i_func* glNormal3i;
	typedef void (FGL_APIENTRY gl_normal3iv_func)(const GLint *v);
	static gl_normal3iv_func* glNormal3iv;
	typedef void (FGL_APIENTRY gl_normal3s_func)(GLshort nx, GLshort ny, GLshort nz);
	static gl_normal3s_func* glNormal3s;
	typedef void (FGL_APIENTRY gl_normal3sv_func)(const GLshort *v);
	static gl_normal3sv_func* glNormal3sv;
	typedef void (FGL_APIENTRY gl_normal_pointer_func)(GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_normal_pointer_func* glNormalPointer;
	typedef void (FGL_APIENTRY gl_ortho_func)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static gl_ortho_func* glOrtho;
	typedef void (FGL_APIENTRY gl_pass_through_func)(GLfloat token);
	static gl_pass_through_func* glPassThrough;
	typedef void (FGL_APIENTRY gl_pixel_mapfv_func)(GLenum map, GLsizei mapsize, const GLfloat *values);
	static gl_pixel_mapfv_func* glPixelMapfv;
	typedef void (FGL_APIENTRY gl_pixel_mapuiv_func)(GLenum map, GLsizei mapsize, const GLuint *values);
	static gl_pixel_mapuiv_func* glPixelMapuiv;
	typedef void (FGL_APIENTRY gl_pixel_mapusv_func)(GLenum map, GLsizei mapsize, const GLushort *values);
	static gl_pixel_mapusv_func* glPixelMapusv;
	typedef void (FGL_APIENTRY gl_pixel_storef_func)(GLenum pname, GLfloat param);
	static gl_pixel_storef_func* glPixelStoref;
	typedef void (FGL_APIENTRY gl_pixel_storei_func)(GLenum pname, GLint param);
	static gl_pixel_storei_func* glPixelStorei;
	typedef void (FGL_APIENTRY gl_pixel_transferf_func)(GLenum pname, GLfloat param);
	static gl_pixel_transferf_func* glPixelTransferf;
	typedef void (FGL_APIENTRY gl_pixel_transferi_func)(GLenum pname, GLint param);
	static gl_pixel_transferi_func* glPixelTransferi;
	typedef void (FGL_APIENTRY gl_pixel_zoom_func)(GLfloat xfactor, GLfloat yfactor);
	static gl_pixel_zoom_func* glPixelZoom;
	typedef void (FGL_APIENTRY gl_point_size_func)(GLfloat size);
	static gl_point_size_func* glPointSize;
	typedef void (FGL_APIENTRY gl_polygon_mode_func)(GLenum face, GLenum mode);
	static gl_polygon_mode_func* glPolygonMode;
	typedef void (FGL_APIENTRY gl_polygon_offset_func)(GLfloat factor, GLfloat units);
	static gl_polygon_offset_func* glPolygonOffset;
	typedef void (FGL_APIENTRY gl_polygon_stipple_func)(const GLubyte *mask);
	static gl_polygon_stipple_func* glPolygonStipple;
	typedef void (FGL_APIENTRY gl_pop_attrib_func)(void);
	static gl_pop_attrib_func* glPopAttrib;
	typedef void (FGL_APIENTRY gl_pop_client_attrib_func)(void);
	static gl_pop_client_attrib_func* glPopClientAttrib;
	typedef void (FGL_APIENTRY gl_pop_matrix_func)(void);
	static gl_pop_matrix_func* glPopMatrix;
	typedef void (FGL_APIENTRY gl_pop_name_func)(void);
	static gl_pop_name_func* glPopName;
	typedef void (FGL_APIENTRY gl_prioritize_textures_func)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
	static gl_prioritize_textures_func* glPrioritizeTextures;
	typedef void (FGL_APIENTRY gl_push_attrib_func)(GLbitfield mask);
	static gl_push_attrib_func* glPushAttrib;
	typedef void (FGL_APIENTRY gl_push_client_attrib_func)(GLbitfield mask);
	static gl_push_client_attrib_func* glPushClientAttrib;
	typedef void (FGL_APIENTRY gl_push_matrix_func)(void);
	static gl_push_matrix_func* glPushMatrix;
	typedef void (FGL_APIENTRY gl_push_name_func)(GLuint name);
	static gl_push_name_func* glPushName;
	typedef void (FGL_APIENTRY gl_raster_pos2d_func)(GLdouble x, GLdouble y);
	static gl_raster_pos2d_func* glRasterPos2d;
	typedef void (FGL_APIENTRY gl_raster_pos2dv_func)(const GLdouble *v);
	static gl_raster_pos2dv_func* glRasterPos2dv;
	typedef void (FGL_APIENTRY gl_raster_pos2f_func)(GLfloat x, GLfloat y);
	static gl_raster_pos2f_func* glRasterPos2f;
	typedef void (FGL_APIENTRY gl_raster_pos2fv_func)(const GLfloat *v);
	static gl_raster_pos2fv_func* glRasterPos2fv;
	typedef void (FGL_APIENTRY gl_raster_pos2i_func)(GLint x, GLint y);
	static gl_raster_pos2i_func* glRasterPos2i;
	typedef void (FGL_APIENTRY gl_raster_pos2iv_func)(const GLint *v);
	static gl_raster_pos2iv_func* glRasterPos2iv;
	typedef void (FGL_APIENTRY gl_raster_pos2s_func)(GLshort x, GLshort y);
	static gl_raster_pos2s_func* glRasterPos2s;
	typedef void (FGL_APIENTRY gl_raster_pos2sv_func)(const GLshort *v);
	static gl_raster_pos2sv_func* glRasterPos2sv;
	typedef void (FGL_APIENTRY gl_raster_pos3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_raster_pos3d_func* glRasterPos3d;
	typedef void (FGL_APIENTRY gl_raster_pos3dv_func)(const GLdouble *v);
	static gl_raster_pos3dv_func* glRasterPos3dv;
	typedef void (FGL_APIENTRY gl_raster_pos3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_raster_pos3f_func* glRasterPos3f;
	typedef void (FGL_APIENTRY gl_raster_pos3fv_func)(const GLfloat *v);
	static gl_raster_pos3fv_func* glRasterPos3fv;
	typedef void (FGL_APIENTRY gl_raster_pos3i_func)(GLint x, GLint y, GLint z);
	static gl_raster_pos3i_func* glRasterPos3i;
	typedef void (FGL_APIENTRY gl_raster_pos3iv_func)(const GLint *v);
	static gl_raster_pos3iv_func* glRasterPos3iv;
	typedef void (FGL_APIENTRY gl_raster_pos3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_raster_pos3s_func* glRasterPos3s;
	typedef void (FGL_APIENTRY gl_raster_pos3sv_func)(const GLshort *v);
	static gl_raster_pos3sv_func* glRasterPos3sv;
	typedef void (FGL_APIENTRY gl_raster_pos4d_func)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_raster_pos4d_func* glRasterPos4d;
	typedef void (FGL_APIENTRY gl_raster_pos4dv_func)(const GLdouble *v);
	static gl_raster_pos4dv_func* glRasterPos4dv;
	typedef void (FGL_APIENTRY gl_raster_pos4f_func)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_raster_pos4f_func* glRasterPos4f;
	typedef void (FGL_APIENTRY gl_raster_pos4fv_func)(const GLfloat *v);
	static gl_raster_pos4fv_func* glRasterPos4fv;
	typedef void (FGL_APIENTRY gl_raster_pos4i_func)(GLint x, GLint y, GLint z, GLint w);
	static gl_raster_pos4i_func* glRasterPos4i;
	typedef void (FGL_APIENTRY gl_raster_pos4iv_func)(const GLint *v);
	static gl_raster_pos4iv_func* glRasterPos4iv;
	typedef void (FGL_APIENTRY gl_raster_pos4s_func)(GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_raster_pos4s_func* glRasterPos4s;
	typedef void (FGL_APIENTRY gl_raster_pos4sv_func)(const GLshort *v);
	static gl_raster_pos4sv_func* glRasterPos4sv;
	typedef void (FGL_APIENTRY gl_read_buffer_func)(GLenum mode);
	static gl_read_buffer_func* glReadBuffer;
	typedef void (FGL_APIENTRY gl_read_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
	static gl_read_pixels_func* glReadPixels;
	typedef void (FGL_APIENTRY gl_rectd_func)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	static gl_rectd_func* glRectd;
	typedef void (FGL_APIENTRY gl_rectdv_func)(const GLdouble *v1, const GLdouble *v2);
	static gl_rectdv_func* glRectdv;
	typedef void (FGL_APIENTRY gl_rectf_func)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	static gl_rectf_func* glRectf;
	typedef void (FGL_APIENTRY gl_rectfv_func)(const GLfloat *v1, const GLfloat *v2);
	static gl_rectfv_func* glRectfv;
	typedef void (FGL_APIENTRY gl_recti_func)(GLint x1, GLint y1, GLint x2, GLint y2);
	static gl_recti_func* glRecti;
	typedef void (FGL_APIENTRY gl_rectiv_func)(const GLint *v1, const GLint *v2);
	static gl_rectiv_func* glRectiv;
	typedef void (FGL_APIENTRY gl_rects_func)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	static gl_rects_func* glRects;
	typedef void (FGL_APIENTRY gl_rectsv_func)(const GLshort *v1, const GLshort *v2);
	static gl_rectsv_func* glRectsv;
	typedef GLint(FGL_APIENTRY gl_render_mode_func)(GLenum mode);
	static gl_render_mode_func* glRenderMode;
	typedef void (FGL_APIENTRY gl_rotated_func)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	static gl_rotated_func* glRotated;
	typedef void (FGL_APIENTRY gl_rotatef_func)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	static gl_rotatef_func* glRotatef;
	typedef void (FGL_APIENTRY gl_scaled_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_scaled_func* glScaled;
	typedef void (FGL_APIENTRY gl_scalef_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_scalef_func* glScalef;
	typedef void (FGL_APIENTRY gl_scissor_func)(GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_scissor_func* glScissor;
	typedef void (FGL_APIENTRY gl_select_buffer_func)(GLsizei size, GLuint *buffer);
	static gl_select_buffer_func* glSelectBuffer;
	typedef void (FGL_APIENTRY gl_shade_model_func)(GLenum mode);
	static gl_shade_model_func* glShadeModel;
	typedef void (FGL_APIENTRY gl_stencil_func_func)(GLenum func, GLint ref, GLuint mask);
	static gl_stencil_func_func* glStencilFunc;
	typedef void (FGL_APIENTRY gl_stencil_mask_func)(GLuint mask);
	static gl_stencil_mask_func* glStencilMask;
	typedef void (FGL_APIENTRY gl_stencil_op_func)(GLenum fail, GLenum zfail, GLenum zpass);
	static gl_stencil_op_func* glStencilOp;
	typedef void (FGL_APIENTRY gl_tex_coord1d_func)(GLdouble s);
	static gl_tex_coord1d_func* glTexCoord1d;
	typedef void (FGL_APIENTRY gl_tex_coord1dv_func)(const GLdouble *v);
	static gl_tex_coord1dv_func* glTexCoord1dv;
	typedef void (FGL_APIENTRY gl_tex_coord1f_func)(GLfloat s);
	static gl_tex_coord1f_func* glTexCoord1f;
	typedef void (FGL_APIENTRY gl_tex_coord1fv_func)(const GLfloat *v);
	static gl_tex_coord1fv_func* glTexCoord1fv;
	typedef void (FGL_APIENTRY gl_tex_coord1i_func)(GLint s);
	static gl_tex_coord1i_func* glTexCoord1i;
	typedef void (FGL_APIENTRY gl_tex_coord1iv_func)(const GLint *v);
	static gl_tex_coord1iv_func* glTexCoord1iv;
	typedef void (FGL_APIENTRY gl_tex_coord1s_func)(GLshort s);
	static gl_tex_coord1s_func* glTexCoord1s;
	typedef void (FGL_APIENTRY gl_tex_coord1sv_func)(const GLshort *v);
	static gl_tex_coord1sv_func* glTexCoord1sv;
	typedef void (FGL_APIENTRY gl_tex_coord2d_func)(GLdouble s, GLdouble t);
	static gl_tex_coord2d_func* glTexCoord2d;
	typedef void (FGL_APIENTRY gl_tex_coord2dv_func)(const GLdouble *v);
	static gl_tex_coord2dv_func* glTexCoord2dv;
	typedef void (FGL_APIENTRY gl_tex_coord2f_func)(GLfloat s, GLfloat t);
	static gl_tex_coord2f_func* glTexCoord2f;
	typedef void (FGL_APIENTRY gl_tex_coord2fv_func)(const GLfloat *v);
	static gl_tex_coord2fv_func* glTexCoord2fv;
	typedef void (FGL_APIENTRY gl_tex_coord2i_func)(GLint s, GLint t);
	static gl_tex_coord2i_func* glTexCoord2i;
	typedef void (FGL_APIENTRY gl_tex_coord2iv_func)(const GLint *v);
	static gl_tex_coord2iv_func* glTexCoord2iv;
	typedef void (FGL_APIENTRY gl_tex_coord2s_func)(GLshort s, GLshort t);
	static gl_tex_coord2s_func* glTexCoord2s;
	typedef void (FGL_APIENTRY gl_tex_coord2sv_func)(const GLshort *v);
	static gl_tex_coord2sv_func* glTexCoord2sv;
	typedef void (FGL_APIENTRY gl_tex_coord3d_func)(GLdouble s, GLdouble t, GLdouble r);
	static gl_tex_coord3d_func* glTexCoord3d;
	typedef void (FGL_APIENTRY gl_tex_coord3dv_func)(const GLdouble *v);
	static gl_tex_coord3dv_func* glTexCoord3dv;
	typedef void (FGL_APIENTRY gl_tex_coord3f_func)(GLfloat s, GLfloat t, GLfloat r);
	static gl_tex_coord3f_func* glTexCoord3f;
	typedef void (FGL_APIENTRY gl_tex_coord3fv_func)(const GLfloat *v);
	static gl_tex_coord3fv_func* glTexCoord3fv;
	typedef void (FGL_APIENTRY gl_tex_coord3i_func)(GLint s, GLint t, GLint r);
	static gl_tex_coord3i_func* glTexCoord3i;
	typedef void (FGL_APIENTRY gl_tex_coord3iv_func)(const GLint *v);
	static gl_tex_coord3iv_func* glTexCoord3iv;
	typedef void (FGL_APIENTRY gl_tex_coord3s_func)(GLshort s, GLshort t, GLshort r);
	static gl_tex_coord3s_func* glTexCoord3s;
	typedef void (FGL_APIENTRY gl_tex_coord3sv_func)(const GLshort *v);
	static gl_tex_coord3sv_func* glTexCoord3sv;
	typedef void (FGL_APIENTRY gl_tex_coord4d_func)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	static gl_tex_coord4d_func* glTexCoord4d;
	typedef void (FGL_APIENTRY gl_tex_coord4dv_func)(const GLdouble *v);
	static gl_tex_coord4dv_func* glTexCoord4dv;
	typedef void (FGL_APIENTRY gl_tex_coord4f_func)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	static gl_tex_coord4f_func* glTexCoord4f;
	typedef void (FGL_APIENTRY gl_tex_coord4fv_func)(const GLfloat *v);
	static gl_tex_coord4fv_func* glTexCoord4fv;
	typedef void (FGL_APIENTRY gl_tex_coord4i_func)(GLint s, GLint t, GLint r, GLint q);
	static gl_tex_coord4i_func* glTexCoord4i;
	typedef void (FGL_APIENTRY gl_tex_coord4iv_func)(const GLint *v);
	static gl_tex_coord4iv_func* glTexCoord4iv;
	typedef void (FGL_APIENTRY gl_tex_coord4s_func)(GLshort s, GLshort t, GLshort r, GLshort q);
	static gl_tex_coord4s_func* glTexCoord4s;
	typedef void (FGL_APIENTRY gl_tex_coord4sv_func)(const GLshort *v);
	static gl_tex_coord4sv_func* glTexCoord4sv;
	typedef void (FGL_APIENTRY gl_tex_coord_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_tex_coord_pointer_func* glTexCoordPointer;
	typedef void (FGL_APIENTRY gl_tex_envf_func)(GLenum target, GLenum pname, GLfloat param);
	static gl_tex_envf_func* glTexEnvf;
	typedef void (FGL_APIENTRY gl_tex_envfv_func)(GLenum target, GLenum pname, const GLfloat *params);
	static gl_tex_envfv_func* glTexEnvfv;
	typedef void (FGL_APIENTRY gl_tex_envi_func)(GLenum target, GLenum pname, GLint param);
	static gl_tex_envi_func* glTexEnvi;
	typedef void (FGL_APIENTRY gl_tex_enviv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_enviv_func* glTexEnviv;
	typedef void (FGL_APIENTRY gl_tex_gend_func)(GLenum coord, GLenum pname, GLdouble param);
	static gl_tex_gend_func* glTexGend;
	typedef void (FGL_APIENTRY gl_tex_gendv_func)(GLenum coord, GLenum pname, const GLdouble *params);
	static gl_tex_gendv_func* glTexGendv;
	typedef void (FGL_APIENTRY gl_tex_genf_func)(GLenum coord, GLenum pname, GLfloat param);
	static gl_tex_genf_func* glTexGenf;
	typedef void (FGL_APIENTRY gl_tex_genfv_func)(GLenum coord, GLenum pname, const GLfloat *params);
	static gl_tex_genfv_func* glTexGenfv;
	typedef void (FGL_APIENTRY gl_tex_geni_func)(GLenum coord, GLenum pname, GLint param);
	static gl_tex_geni_func* glTexGeni;
	typedef void (FGL_APIENTRY gl_tex_geniv_func)(GLenum coord, GLenum pname, const GLint *params);
	static gl_tex_geniv_func* glTexGeniv;
	typedef void (FGL_APIENTRY gl_tex_image1d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_image1d_func* glTexImage1D;
	typedef void (FGL_APIENTRY gl_tex_image2d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_image2d_func* glTexImage2D;
	typedef void (FGL_APIENTRY gl_tex_parameterf_func)(GLenum target, GLenum pname, GLfloat param);
	static gl_tex_parameterf_func* glTexParameterf;
	typedef void (FGL_APIENTRY gl_tex_parameterfv_func)(GLenum target, GLenum pname, const GLfloat *params);
	static gl_tex_parameterfv_func* glTexParameterfv;
	typedef void (FGL_APIENTRY gl_tex_parameteri_func)(GLenum target, GLenum pname, GLint param);
	static gl_tex_parameteri_func* glTexParameteri;
	typedef void (FGL_APIENTRY gl_tex_parameteriv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_parameteriv_func* glTexParameteriv;
	typedef void (FGL_APIENTRY gl_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_sub_image1d_func* glTexSubImage1D;
	typedef void (FGL_APIENTRY gl_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static gl_tex_sub_image2d_func* glTexSubImage2D;
	typedef void (FGL_APIENTRY gl_translated_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_translated_func* glTranslated;
	typedef void (FGL_APIENTRY gl_translatef_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_translatef_func* glTranslatef;
	typedef void (FGL_APIENTRY gl_vertex2d_func)(GLdouble x, GLdouble y);
	static gl_vertex2d_func* glVertex2d;
	typedef void (FGL_APIENTRY gl_vertex2dv_func)(const GLdouble *v);
	static gl_vertex2dv_func* glVertex2dv;
	typedef void (FGL_APIENTRY gl_vertex2f_func)(GLfloat x, GLfloat y);
	static gl_vertex2f_func* glVertex2f;
	typedef void (FGL_APIENTRY gl_vertex2fv_func)(const GLfloat *v);
	static gl_vertex2fv_func* glVertex2fv;
	typedef void (FGL_APIENTRY gl_vertex2i_func)(GLint x, GLint y);
	static gl_vertex2i_func* glVertex2i;
	typedef void (FGL_APIENTRY gl_vertex2iv_func)(const GLint *v);
	static gl_vertex2iv_func* glVertex2iv;
	typedef void (FGL_APIENTRY gl_vertex2s_func)(GLshort x, GLshort y);
	static gl_vertex2s_func* glVertex2s;
	typedef void (FGL_APIENTRY gl_vertex2sv_func)(const GLshort *v);
	static gl_vertex2sv_func* glVertex2sv;
	typedef void (FGL_APIENTRY gl_vertex3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex3d_func* glVertex3d;
	typedef void (FGL_APIENTRY gl_vertex3dv_func)(const GLdouble *v);
	static gl_vertex3dv_func* glVertex3dv;
	typedef void (FGL_APIENTRY gl_vertex3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_vertex3f_func* glVertex3f;
	typedef void (FGL_APIENTRY gl_vertex3fv_func)(const GLfloat *v);
	static gl_vertex3fv_func* glVertex3fv;
	typedef void (FGL_APIENTRY gl_vertex3i_func)(GLint x, GLint y, GLint z);
	static gl_vertex3i_func* glVertex3i;
	typedef void (FGL_APIENTRY gl_vertex3iv_func)(const GLint *v);
	static gl_vertex3iv_func* glVertex3iv;
	typedef void (FGL_APIENTRY gl_vertex3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_vertex3s_func* glVertex3s;
	typedef void (FGL_APIENTRY gl_vertex3sv_func)(const GLshort *v);
	static gl_vertex3sv_func* glVertex3sv;
	typedef void (FGL_APIENTRY gl_vertex4d_func)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex4d_func* glVertex4d;
	typedef void (FGL_APIENTRY gl_vertex4dv_func)(const GLdouble *v);
	static gl_vertex4dv_func* glVertex4dv;
	typedef void (FGL_APIENTRY gl_vertex4f_func)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_vertex4f_func* glVertex4f;
	typedef void (FGL_APIENTRY gl_vertex4fv_func)(const GLfloat *v);
	static gl_vertex4fv_func* glVertex4fv;
	typedef void (FGL_APIENTRY gl_vertex4i_func)(GLint x, GLint y, GLint z, GLint w);
	static gl_vertex4i_func* glVertex4i;
	typedef void (FGL_APIENTRY gl_vertex4iv_func)(const GLint *v);
	static gl_vertex4iv_func* glVertex4iv;
	typedef void (FGL_APIENTRY gl_vertex4s_func)(GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_vertex4s_func* glVertex4s;
	typedef void (FGL_APIENTRY gl_vertex4sv_func)(const GLshort *v);
	static gl_vertex4sv_func* glVertex4sv;
	typedef void (FGL_APIENTRY gl_vertex_pointer_func)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static gl_vertex_pointer_func* glVertexPointer;
	typedef void (FGL_APIENTRY gl_viewport_func)(GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_viewport_func* glViewport;
#	endif // GL_VERSION_1_1
#	ifndef GL_VERSION_1_2
#		define GL_VERSION_1_2 1
	static bool isGL_VERSION_1_2;
#	define GL_UNSIGNED_BYTE_3_3_2 0x8032
#	define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#	define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#	define GL_UNSIGNED_INT_8_8_8_8 0x8035
#	define GL_UNSIGNED_INT_10_10_10_2 0x8036
#	define GL_TEXTURE_BINDING_3D 0x806A
#	define GL_PACK_SKIP_IMAGES 0x806B
#	define GL_PACK_IMAGE_HEIGHT 0x806C
#	define GL_UNPACK_SKIP_IMAGES 0x806D
#	define GL_UNPACK_IMAGE_HEIGHT 0x806E
#	define GL_TEXTURE_3D 0x806F
#	define GL_PROXY_TEXTURE_3D 0x8070
#	define GL_TEXTURE_DEPTH 0x8071
#	define GL_TEXTURE_WRAP_R 0x8072
#	define GL_MAX_3D_TEXTURE_SIZE 0x8073
#	define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#	define GL_UNSIGNED_SHORT_5_6_5 0x8363
#	define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#	define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#	define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#	define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#	define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#	define GL_BGR 0x80E0
#	define GL_BGRA 0x80E1
#	define GL_MAX_ELEMENTS_VERTICES 0x80E8
#	define GL_MAX_ELEMENTS_INDICES 0x80E9
#	define GL_CLAMP_TO_EDGE 0x812F
#	define GL_TEXTURE_MIN_LOD 0x813A
#	define GL_TEXTURE_MAX_LOD 0x813B
#	define GL_TEXTURE_BASE_LEVEL 0x813C
#	define GL_TEXTURE_MAX_LEVEL 0x813D
#	define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#	define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#	define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#	define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#	define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#	define GL_RESCALE_NORMAL 0x803A
#	define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#	define GL_SINGLE_COLOR 0x81F9
#	define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#	define GL_ALIASED_POINT_SIZE_RANGE 0x846D
	typedef void (FGL_APIENTRY gl_draw_range_elements_func)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
	static gl_draw_range_elements_func* glDrawRangeElements;
	typedef void (FGL_APIENTRY gl_tex_image3d_func)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
	static gl_tex_image3d_func* glTexImage3D;
	typedef void (FGL_APIENTRY gl_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	static gl_tex_sub_image3d_func* glTexSubImage3D;
	typedef void (FGL_APIENTRY gl_copy_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_tex_sub_image3d_func* glCopyTexSubImage3D;
#	endif // GL_VERSION_1_2
#	ifndef GL_VERSION_1_3
#		define GL_VERSION_1_3 1
	static bool isGL_VERSION_1_3;
#	define GL_TEXTURE0 0x84C0
#	define GL_TEXTURE1 0x84C1
#	define GL_TEXTURE2 0x84C2
#	define GL_TEXTURE3 0x84C3
#	define GL_TEXTURE4 0x84C4
#	define GL_TEXTURE5 0x84C5
#	define GL_TEXTURE6 0x84C6
#	define GL_TEXTURE7 0x84C7
#	define GL_TEXTURE8 0x84C8
#	define GL_TEXTURE9 0x84C9
#	define GL_TEXTURE10 0x84CA
#	define GL_TEXTURE11 0x84CB
#	define GL_TEXTURE12 0x84CC
#	define GL_TEXTURE13 0x84CD
#	define GL_TEXTURE14 0x84CE
#	define GL_TEXTURE15 0x84CF
#	define GL_TEXTURE16 0x84D0
#	define GL_TEXTURE17 0x84D1
#	define GL_TEXTURE18 0x84D2
#	define GL_TEXTURE19 0x84D3
#	define GL_TEXTURE20 0x84D4
#	define GL_TEXTURE21 0x84D5
#	define GL_TEXTURE22 0x84D6
#	define GL_TEXTURE23 0x84D7
#	define GL_TEXTURE24 0x84D8
#	define GL_TEXTURE25 0x84D9
#	define GL_TEXTURE26 0x84DA
#	define GL_TEXTURE27 0x84DB
#	define GL_TEXTURE28 0x84DC
#	define GL_TEXTURE29 0x84DD
#	define GL_TEXTURE30 0x84DE
#	define GL_TEXTURE31 0x84DF
#	define GL_ACTIVE_TEXTURE 0x84E0
#	define GL_MULTISAMPLE 0x809D
#	define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#	define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#	define GL_SAMPLE_COVERAGE 0x80A0
#	define GL_SAMPLE_BUFFERS 0x80A8
#	define GL_SAMPLES 0x80A9
#	define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#	define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#	define GL_TEXTURE_CUBE_MAP 0x8513
#	define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#	define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#	define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#	define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#	define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#	define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#	define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#	define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#	define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#	define GL_COMPRESSED_RGB 0x84ED
#	define GL_COMPRESSED_RGBA 0x84EE
#	define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#	define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#	define GL_TEXTURE_COMPRESSED 0x86A1
#	define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#	define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#	define GL_CLAMP_TO_BORDER 0x812D
#	define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#	define GL_MAX_TEXTURE_UNITS 0x84E2
#	define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#	define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#	define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#	define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#	define GL_MULTISAMPLE_BIT 0x20000000
#	define GL_NORMAL_MAP 0x8511
#	define GL_REFLECTION_MAP 0x8512
#	define GL_COMPRESSED_ALPHA 0x84E9
#	define GL_COMPRESSED_LUMINANCE 0x84EA
#	define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#	define GL_COMPRESSED_INTENSITY 0x84EC
#	define GL_COMBINE 0x8570
#	define GL_COMBINE_RGB 0x8571
#	define GL_COMBINE_ALPHA 0x8572
#	define GL_SOURCE0_RGB 0x8580
#	define GL_SOURCE1_RGB 0x8581
#	define GL_SOURCE2_RGB 0x8582
#	define GL_SOURCE0_ALPHA 0x8588
#	define GL_SOURCE1_ALPHA 0x8589
#	define GL_SOURCE2_ALPHA 0x858A
#	define GL_OPERAND0_RGB 0x8590
#	define GL_OPERAND1_RGB 0x8591
#	define GL_OPERAND2_RGB 0x8592
#	define GL_OPERAND0_ALPHA 0x8598
#	define GL_OPERAND1_ALPHA 0x8599
#	define GL_OPERAND2_ALPHA 0x859A
#	define GL_RGB_SCALE 0x8573
#	define GL_ADD_SIGNED 0x8574
#	define GL_INTERPOLATE 0x8575
#	define GL_SUBTRACT 0x84E7
#	define GL_CONSTANT 0x8576
#	define GL_PRIMARY_COLOR 0x8577
#	define GL_PREVIOUS 0x8578
#	define GL_DOT3_RGB 0x86AE
#	define GL_DOT3_RGBA 0x86AF
	typedef void (FGL_APIENTRY gl_active_texture_func)(GLenum texture);
	static gl_active_texture_func* glActiveTexture;
	typedef void (FGL_APIENTRY gl_sample_coverage_func)(GLfloat value, GLboolean invert);
	static gl_sample_coverage_func* glSampleCoverage;
	typedef void (FGL_APIENTRY gl_compressed_tex_image3d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image3d_func* glCompressedTexImage3D;
	typedef void (FGL_APIENTRY gl_compressed_tex_image2d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image2d_func* glCompressedTexImage2D;
	typedef void (FGL_APIENTRY gl_compressed_tex_image1d_func)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
	static gl_compressed_tex_image1d_func* glCompressedTexImage1D;
	typedef void (FGL_APIENTRY gl_compressed_tex_sub_image3d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image3d_func* glCompressedTexSubImage3D;
	typedef void (FGL_APIENTRY gl_compressed_tex_sub_image2d_func)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image2d_func* glCompressedTexSubImage2D;
	typedef void (FGL_APIENTRY gl_compressed_tex_sub_image1d_func)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_tex_sub_image1d_func* glCompressedTexSubImage1D;
	typedef void (FGL_APIENTRY gl_get_compressed_tex_image_func)(GLenum target, GLint level, void *img);
	static gl_get_compressed_tex_image_func* glGetCompressedTexImage;
	typedef void (FGL_APIENTRY gl_client_active_texture_func)(GLenum texture);
	static gl_client_active_texture_func* glClientActiveTexture;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1d_func)(GLenum target, GLdouble s);
	static gl_multi_tex_coord1d_func* glMultiTexCoord1d;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord1dv_func* glMultiTexCoord1dv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1f_func)(GLenum target, GLfloat s);
	static gl_multi_tex_coord1f_func* glMultiTexCoord1f;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord1fv_func* glMultiTexCoord1fv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1i_func)(GLenum target, GLint s);
	static gl_multi_tex_coord1i_func* glMultiTexCoord1i;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord1iv_func* glMultiTexCoord1iv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1s_func)(GLenum target, GLshort s);
	static gl_multi_tex_coord1s_func* glMultiTexCoord1s;
	typedef void (FGL_APIENTRY gl_multi_tex_coord1sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord1sv_func* glMultiTexCoord1sv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2d_func)(GLenum target, GLdouble s, GLdouble t);
	static gl_multi_tex_coord2d_func* glMultiTexCoord2d;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord2dv_func* glMultiTexCoord2dv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2f_func)(GLenum target, GLfloat s, GLfloat t);
	static gl_multi_tex_coord2f_func* glMultiTexCoord2f;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord2fv_func* glMultiTexCoord2fv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2i_func)(GLenum target, GLint s, GLint t);
	static gl_multi_tex_coord2i_func* glMultiTexCoord2i;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord2iv_func* glMultiTexCoord2iv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2s_func)(GLenum target, GLshort s, GLshort t);
	static gl_multi_tex_coord2s_func* glMultiTexCoord2s;
	typedef void (FGL_APIENTRY gl_multi_tex_coord2sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord2sv_func* glMultiTexCoord2sv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3d_func)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
	static gl_multi_tex_coord3d_func* glMultiTexCoord3d;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord3dv_func* glMultiTexCoord3dv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3f_func)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	static gl_multi_tex_coord3f_func* glMultiTexCoord3f;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord3fv_func* glMultiTexCoord3fv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3i_func)(GLenum target, GLint s, GLint t, GLint r);
	static gl_multi_tex_coord3i_func* glMultiTexCoord3i;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord3iv_func* glMultiTexCoord3iv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3s_func)(GLenum target, GLshort s, GLshort t, GLshort r);
	static gl_multi_tex_coord3s_func* glMultiTexCoord3s;
	typedef void (FGL_APIENTRY gl_multi_tex_coord3sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord3sv_func* glMultiTexCoord3sv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4d_func)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	static gl_multi_tex_coord4d_func* glMultiTexCoord4d;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4dv_func)(GLenum target, const GLdouble *v);
	static gl_multi_tex_coord4dv_func* glMultiTexCoord4dv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4f_func)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	static gl_multi_tex_coord4f_func* glMultiTexCoord4f;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4fv_func)(GLenum target, const GLfloat *v);
	static gl_multi_tex_coord4fv_func* glMultiTexCoord4fv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4i_func)(GLenum target, GLint s, GLint t, GLint r, GLint q);
	static gl_multi_tex_coord4i_func* glMultiTexCoord4i;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4iv_func)(GLenum target, const GLint *v);
	static gl_multi_tex_coord4iv_func* glMultiTexCoord4iv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4s_func)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	static gl_multi_tex_coord4s_func* glMultiTexCoord4s;
	typedef void (FGL_APIENTRY gl_multi_tex_coord4sv_func)(GLenum target, const GLshort *v);
	static gl_multi_tex_coord4sv_func* glMultiTexCoord4sv;
	typedef void (FGL_APIENTRY gl_load_transpose_matrixf_func)(const GLfloat *m);
	static gl_load_transpose_matrixf_func* glLoadTransposeMatrixf;
	typedef void (FGL_APIENTRY gl_load_transpose_matrixd_func)(const GLdouble *m);
	static gl_load_transpose_matrixd_func* glLoadTransposeMatrixd;
	typedef void (FGL_APIENTRY gl_mult_transpose_matrixf_func)(const GLfloat *m);
	static gl_mult_transpose_matrixf_func* glMultTransposeMatrixf;
	typedef void (FGL_APIENTRY gl_mult_transpose_matrixd_func)(const GLdouble *m);
	static gl_mult_transpose_matrixd_func* glMultTransposeMatrixd;
#	endif // GL_VERSION_1_3
#	ifndef GL_VERSION_1_4
#		define GL_VERSION_1_4 1
	static bool isGL_VERSION_1_4;
#	define GL_BLEND_DST_RGB 0x80C8
#	define GL_BLEND_SRC_RGB 0x80C9
#	define GL_BLEND_DST_ALPHA 0x80CA
#	define GL_BLEND_SRC_ALPHA 0x80CB
#	define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#	define GL_DEPTH_COMPONENT16 0x81A5
#	define GL_DEPTH_COMPONENT24 0x81A6
#	define GL_DEPTH_COMPONENT32 0x81A7
#	define GL_MIRRORED_REPEAT 0x8370
#	define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#	define GL_TEXTURE_LOD_BIAS 0x8501
#	define GL_INCR_WRAP 0x8507
#	define GL_DECR_WRAP 0x8508
#	define GL_TEXTURE_DEPTH_SIZE 0x884A
#	define GL_TEXTURE_COMPARE_MODE 0x884C
#	define GL_TEXTURE_COMPARE_FUNC 0x884D
#	define GL_POINT_SIZE_MIN 0x8126
#	define GL_POINT_SIZE_MAX 0x8127
#	define GL_POINT_DISTANCE_ATTENUATION 0x8129
#	define GL_GENERATE_MIPMAP 0x8191
#	define GL_GENERATE_MIPMAP_HINT 0x8192
#	define GL_FOG_COORDINATE_SOURCE 0x8450
#	define GL_FOG_COORDINATE 0x8451
#	define GL_FRAGMENT_DEPTH 0x8452
#	define GL_CURRENT_FOG_COORDINATE 0x8453
#	define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#	define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#	define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#	define GL_FOG_COORDINATE_ARRAY 0x8457
#	define GL_COLOR_SUM 0x8458
#	define GL_CURRENT_SECONDARY_COLOR 0x8459
#	define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#	define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#	define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#	define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#	define GL_SECONDARY_COLOR_ARRAY 0x845E
#	define GL_TEXTURE_FILTER_CONTROL 0x8500
#	define GL_DEPTH_TEXTURE_MODE 0x884B
#	define GL_COMPARE_R_TO_TEXTURE 0x884E
#	define GL_BLEND_COLOR 0x8005
#	define GL_BLEND_EQUATION 0x8009
#	define GL_CONSTANT_COLOR 0x8001
#	define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#	define GL_CONSTANT_ALPHA 0x8003
#	define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#	define GL_FUNC_ADD 0x8006
#	define GL_FUNC_REVERSE_SUBTRACT 0x800B
#	define GL_FUNC_SUBTRACT 0x800A
#	define GL_MIN 0x8007
#	define GL_MAX 0x8008
	typedef void (FGL_APIENTRY gl_blend_func_separate_func)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	static gl_blend_func_separate_func* glBlendFuncSeparate;
	typedef void (FGL_APIENTRY gl_multi_draw_arrays_func)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
	static gl_multi_draw_arrays_func* glMultiDrawArrays;
	typedef void (FGL_APIENTRY gl_multi_draw_elements_func)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
	static gl_multi_draw_elements_func* glMultiDrawElements;
	typedef void (FGL_APIENTRY gl_point_parameterf_func)(GLenum pname, GLfloat param);
	static gl_point_parameterf_func* glPointParameterf;
	typedef void (FGL_APIENTRY gl_point_parameterfv_func)(GLenum pname, const GLfloat *params);
	static gl_point_parameterfv_func* glPointParameterfv;
	typedef void (FGL_APIENTRY gl_point_parameteri_func)(GLenum pname, GLint param);
	static gl_point_parameteri_func* glPointParameteri;
	typedef void (FGL_APIENTRY gl_point_parameteriv_func)(GLenum pname, const GLint *params);
	static gl_point_parameteriv_func* glPointParameteriv;
	typedef void (FGL_APIENTRY gl_fog_coordf_func)(GLfloat coord);
	static gl_fog_coordf_func* glFogCoordf;
	typedef void (FGL_APIENTRY gl_fog_coordfv_func)(const GLfloat *coord);
	static gl_fog_coordfv_func* glFogCoordfv;
	typedef void (FGL_APIENTRY gl_fog_coordd_func)(GLdouble coord);
	static gl_fog_coordd_func* glFogCoordd;
	typedef void (FGL_APIENTRY gl_fog_coorddv_func)(const GLdouble *coord);
	static gl_fog_coorddv_func* glFogCoorddv;
	typedef void (FGL_APIENTRY gl_fog_coord_pointer_func)(GLenum type, GLsizei stride, const void *pointer);
	static gl_fog_coord_pointer_func* glFogCoordPointer;
	typedef void (FGL_APIENTRY gl_secondary_color3b_func)(GLbyte red, GLbyte green, GLbyte blue);
	static gl_secondary_color3b_func* glSecondaryColor3b;
	typedef void (FGL_APIENTRY gl_secondary_color3bv_func)(const GLbyte *v);
	static gl_secondary_color3bv_func* glSecondaryColor3bv;
	typedef void (FGL_APIENTRY gl_secondary_color3d_func)(GLdouble red, GLdouble green, GLdouble blue);
	static gl_secondary_color3d_func* glSecondaryColor3d;
	typedef void (FGL_APIENTRY gl_secondary_color3dv_func)(const GLdouble *v);
	static gl_secondary_color3dv_func* glSecondaryColor3dv;
	typedef void (FGL_APIENTRY gl_secondary_color3f_func)(GLfloat red, GLfloat green, GLfloat blue);
	static gl_secondary_color3f_func* glSecondaryColor3f;
	typedef void (FGL_APIENTRY gl_secondary_color3fv_func)(const GLfloat *v);
	static gl_secondary_color3fv_func* glSecondaryColor3fv;
	typedef void (FGL_APIENTRY gl_secondary_color3i_func)(GLint red, GLint green, GLint blue);
	static gl_secondary_color3i_func* glSecondaryColor3i;
	typedef void (FGL_APIENTRY gl_secondary_color3iv_func)(const GLint *v);
	static gl_secondary_color3iv_func* glSecondaryColor3iv;
	typedef void (FGL_APIENTRY gl_secondary_color3s_func)(GLshort red, GLshort green, GLshort blue);
	static gl_secondary_color3s_func* glSecondaryColor3s;
	typedef void (FGL_APIENTRY gl_secondary_color3sv_func)(const GLshort *v);
	static gl_secondary_color3sv_func* glSecondaryColor3sv;
	typedef void (FGL_APIENTRY gl_secondary_color3ub_func)(GLubyte red, GLubyte green, GLubyte blue);
	static gl_secondary_color3ub_func* glSecondaryColor3ub;
	typedef void (FGL_APIENTRY gl_secondary_color3ubv_func)(const GLubyte *v);
	static gl_secondary_color3ubv_func* glSecondaryColor3ubv;
	typedef void (FGL_APIENTRY gl_secondary_color3ui_func)(GLuint red, GLuint green, GLuint blue);
	static gl_secondary_color3ui_func* glSecondaryColor3ui;
	typedef void (FGL_APIENTRY gl_secondary_color3uiv_func)(const GLuint *v);
	static gl_secondary_color3uiv_func* glSecondaryColor3uiv;
	typedef void (FGL_APIENTRY gl_secondary_color3us_func)(GLushort red, GLushort green, GLushort blue);
	static gl_secondary_color3us_func* glSecondaryColor3us;
	typedef void (FGL_APIENTRY gl_secondary_color3usv_func)(const GLushort *v);
	static gl_secondary_color3usv_func* glSecondaryColor3usv;
	typedef void (FGL_APIENTRY gl_secondary_color_pointer_func)(GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_secondary_color_pointer_func* glSecondaryColorPointer;
	typedef void (FGL_APIENTRY gl_window_pos2d_func)(GLdouble x, GLdouble y);
	static gl_window_pos2d_func* glWindowPos2d;
	typedef void (FGL_APIENTRY gl_window_pos2dv_func)(const GLdouble *v);
	static gl_window_pos2dv_func* glWindowPos2dv;
	typedef void (FGL_APIENTRY gl_window_pos2f_func)(GLfloat x, GLfloat y);
	static gl_window_pos2f_func* glWindowPos2f;
	typedef void (FGL_APIENTRY gl_window_pos2fv_func)(const GLfloat *v);
	static gl_window_pos2fv_func* glWindowPos2fv;
	typedef void (FGL_APIENTRY gl_window_pos2i_func)(GLint x, GLint y);
	static gl_window_pos2i_func* glWindowPos2i;
	typedef void (FGL_APIENTRY gl_window_pos2iv_func)(const GLint *v);
	static gl_window_pos2iv_func* glWindowPos2iv;
	typedef void (FGL_APIENTRY gl_window_pos2s_func)(GLshort x, GLshort y);
	static gl_window_pos2s_func* glWindowPos2s;
	typedef void (FGL_APIENTRY gl_window_pos2sv_func)(const GLshort *v);
	static gl_window_pos2sv_func* glWindowPos2sv;
	typedef void (FGL_APIENTRY gl_window_pos3d_func)(GLdouble x, GLdouble y, GLdouble z);
	static gl_window_pos3d_func* glWindowPos3d;
	typedef void (FGL_APIENTRY gl_window_pos3dv_func)(const GLdouble *v);
	static gl_window_pos3dv_func* glWindowPos3dv;
	typedef void (FGL_APIENTRY gl_window_pos3f_func)(GLfloat x, GLfloat y, GLfloat z);
	static gl_window_pos3f_func* glWindowPos3f;
	typedef void (FGL_APIENTRY gl_window_pos3fv_func)(const GLfloat *v);
	static gl_window_pos3fv_func* glWindowPos3fv;
	typedef void (FGL_APIENTRY gl_window_pos3i_func)(GLint x, GLint y, GLint z);
	static gl_window_pos3i_func* glWindowPos3i;
	typedef void (FGL_APIENTRY gl_window_pos3iv_func)(const GLint *v);
	static gl_window_pos3iv_func* glWindowPos3iv;
	typedef void (FGL_APIENTRY gl_window_pos3s_func)(GLshort x, GLshort y, GLshort z);
	static gl_window_pos3s_func* glWindowPos3s;
	typedef void (FGL_APIENTRY gl_window_pos3sv_func)(const GLshort *v);
	static gl_window_pos3sv_func* glWindowPos3sv;
	typedef void (FGL_APIENTRY gl_blend_color_func)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static gl_blend_color_func* glBlendColor;
	typedef void (FGL_APIENTRY gl_blend_equation_func)(GLenum mode);
	static gl_blend_equation_func* glBlendEquation;
#	endif // GL_VERSION_1_4
#	ifndef GL_VERSION_1_5
#		define GL_VERSION_1_5 1
	static bool isGL_VERSION_1_5;
	typedef ptrdiff_t GLsizeiptr;
	typedef ptrdiff_t GLintptr;
#	define GL_BUFFER_SIZE 0x8764
#	define GL_BUFFER_USAGE 0x8765
#	define GL_QUERY_COUNTER_BITS 0x8864
#	define GL_CURRENT_QUERY 0x8865
#	define GL_QUERY_RESULT 0x8866
#	define GL_QUERY_RESULT_AVAILABLE 0x8867
#	define GL_ARRAY_BUFFER 0x8892
#	define GL_ELEMENT_ARRAY_BUFFER 0x8893
#	define GL_ARRAY_BUFFER_BINDING 0x8894
#	define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#	define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#	define GL_READ_ONLY 0x88B8
#	define GL_WRITE_ONLY 0x88B9
#	define GL_READ_WRITE 0x88BA
#	define GL_BUFFER_ACCESS 0x88BB
#	define GL_BUFFER_MAPPED 0x88BC
#	define GL_BUFFER_MAP_POINTER 0x88BD
#	define GL_STREAM_DRAW 0x88E0
#	define GL_STREAM_READ 0x88E1
#	define GL_STREAM_COPY 0x88E2
#	define GL_STATIC_DRAW 0x88E4
#	define GL_STATIC_READ 0x88E5
#	define GL_STATIC_COPY 0x88E6
#	define GL_DYNAMIC_DRAW 0x88E8
#	define GL_DYNAMIC_READ 0x88E9
#	define GL_DYNAMIC_COPY 0x88EA
#	define GL_SAMPLES_PASSED 0x8914
#	define GL_SRC1_ALPHA 0x8589
#	define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#	define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#	define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#	define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#	define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#	define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#	define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#	define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#	define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#	define GL_FOG_COORD_SRC 0x8450
#	define GL_FOG_COORD 0x8451
#	define GL_CURRENT_FOG_COORD 0x8453
#	define GL_FOG_COORD_ARRAY_TYPE 0x8454
#	define GL_FOG_COORD_ARRAY_STRIDE 0x8455
#	define GL_FOG_COORD_ARRAY_POINTER 0x8456
#	define GL_FOG_COORD_ARRAY 0x8457
#	define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#	define GL_SRC0_RGB 0x8580
#	define GL_SRC1_RGB 0x8581
#	define GL_SRC2_RGB 0x8582
#	define GL_SRC0_ALPHA 0x8588
#	define GL_SRC2_ALPHA 0x858A
	typedef void (FGL_APIENTRY gl_gen_queries_func)(GLsizei n, GLuint *ids);
	static gl_gen_queries_func* glGenQueries;
	typedef void (FGL_APIENTRY gl_delete_queries_func)(GLsizei n, const GLuint *ids);
	static gl_delete_queries_func* glDeleteQueries;
	typedef GLboolean(FGL_APIENTRY gl_is_query_func)(GLuint id);
	static gl_is_query_func* glIsQuery;
	typedef void (FGL_APIENTRY gl_begin_query_func)(GLenum target, GLuint id);
	static gl_begin_query_func* glBeginQuery;
	typedef void (FGL_APIENTRY gl_end_query_func)(GLenum target);
	static gl_end_query_func* glEndQuery;
	typedef void (FGL_APIENTRY gl_get_queryiv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_queryiv_func* glGetQueryiv;
	typedef void (FGL_APIENTRY gl_get_query_objectiv_func)(GLuint id, GLenum pname, GLint *params);
	static gl_get_query_objectiv_func* glGetQueryObjectiv;
	typedef void (FGL_APIENTRY gl_get_query_objectuiv_func)(GLuint id, GLenum pname, GLuint *params);
	static gl_get_query_objectuiv_func* glGetQueryObjectuiv;
	typedef void (FGL_APIENTRY gl_bind_buffer_func)(GLenum target, GLuint buffer);
	static gl_bind_buffer_func* glBindBuffer;
	typedef void (FGL_APIENTRY gl_delete_buffers_func)(GLsizei n, const GLuint *buffers);
	static gl_delete_buffers_func* glDeleteBuffers;
	typedef void (FGL_APIENTRY gl_gen_buffers_func)(GLsizei n, GLuint *buffers);
	static gl_gen_buffers_func* glGenBuffers;
	typedef GLboolean(FGL_APIENTRY gl_is_buffer_func)(GLuint buffer);
	static gl_is_buffer_func* glIsBuffer;
	typedef void (FGL_APIENTRY gl_buffer_data_func)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
	static gl_buffer_data_func* glBufferData;
	typedef void (FGL_APIENTRY gl_buffer_sub_data_func)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
	static gl_buffer_sub_data_func* glBufferSubData;
	typedef void (FGL_APIENTRY gl_get_buffer_sub_data_func)(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
	static gl_get_buffer_sub_data_func* glGetBufferSubData;
	typedef void * (FGL_APIENTRY gl_map_buffer_func)(GLenum target, GLenum access);
	static gl_map_buffer_func* glMapBuffer;
	typedef GLboolean(FGL_APIENTRY gl_unmap_buffer_func)(GLenum target);
	static gl_unmap_buffer_func* glUnmapBuffer;
	typedef void (FGL_APIENTRY gl_get_buffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_buffer_parameteriv_func* glGetBufferParameteriv;
	typedef void (FGL_APIENTRY gl_get_buffer_pointerv_func)(GLenum target, GLenum pname, void **params);
	static gl_get_buffer_pointerv_func* glGetBufferPointerv;
#	endif // GL_VERSION_1_5
#	ifndef GL_VERSION_2_0
#		define GL_VERSION_2_0 1
	static bool isGL_VERSION_2_0;
	typedef char GLchar;
#	define GL_BLEND_EQUATION_RGB 0x8009
#	define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#	define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#	define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#	define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#	define GL_CURRENT_VERTEX_ATTRIB 0x8626
#	define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#	define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#	define GL_STENCIL_BACK_FUNC 0x8800
#	define GL_STENCIL_BACK_FAIL 0x8801
#	define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#	define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#	define GL_MAX_DRAW_BUFFERS 0x8824
#	define GL_DRAW_BUFFER0 0x8825
#	define GL_DRAW_BUFFER1 0x8826
#	define GL_DRAW_BUFFER2 0x8827
#	define GL_DRAW_BUFFER3 0x8828
#	define GL_DRAW_BUFFER4 0x8829
#	define GL_DRAW_BUFFER5 0x882A
#	define GL_DRAW_BUFFER6 0x882B
#	define GL_DRAW_BUFFER7 0x882C
#	define GL_DRAW_BUFFER8 0x882D
#	define GL_DRAW_BUFFER9 0x882E
#	define GL_DRAW_BUFFER10 0x882F
#	define GL_DRAW_BUFFER11 0x8830
#	define GL_DRAW_BUFFER12 0x8831
#	define GL_DRAW_BUFFER13 0x8832
#	define GL_DRAW_BUFFER14 0x8833
#	define GL_DRAW_BUFFER15 0x8834
#	define GL_BLEND_EQUATION_ALPHA 0x883D
#	define GL_MAX_VERTEX_ATTRIBS 0x8869
#	define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#	define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#	define GL_FRAGMENT_SHADER 0x8B30
#	define GL_VERTEX_SHADER 0x8B31
#	define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#	define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#	define GL_MAX_VARYING_FLOATS 0x8B4B
#	define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#	define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#	define GL_SHADER_TYPE 0x8B4F
#	define GL_FLOAT_VEC2 0x8B50
#	define GL_FLOAT_VEC3 0x8B51
#	define GL_FLOAT_VEC4 0x8B52
#	define GL_INT_VEC2 0x8B53
#	define GL_INT_VEC3 0x8B54
#	define GL_INT_VEC4 0x8B55
#	define GL_BOOL 0x8B56
#	define GL_BOOL_VEC2 0x8B57
#	define GL_BOOL_VEC3 0x8B58
#	define GL_BOOL_VEC4 0x8B59
#	define GL_FLOAT_MAT2 0x8B5A
#	define GL_FLOAT_MAT3 0x8B5B
#	define GL_FLOAT_MAT4 0x8B5C
#	define GL_SAMPLER_1D 0x8B5D
#	define GL_SAMPLER_2D 0x8B5E
#	define GL_SAMPLER_3D 0x8B5F
#	define GL_SAMPLER_CUBE 0x8B60
#	define GL_SAMPLER_1D_SHADOW 0x8B61
#	define GL_SAMPLER_2D_SHADOW 0x8B62
#	define GL_DELETE_STATUS 0x8B80
#	define GL_COMPILE_STATUS 0x8B81
#	define GL_LINK_STATUS 0x8B82
#	define GL_VALIDATE_STATUS 0x8B83
#	define GL_INFO_LOG_LENGTH 0x8B84
#	define GL_ATTACHED_SHADERS 0x8B85
#	define GL_ACTIVE_UNIFORMS 0x8B86
#	define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#	define GL_SHADER_SOURCE_LENGTH 0x8B88
#	define GL_ACTIVE_ATTRIBUTES 0x8B89
#	define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#	define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#	define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#	define GL_CURRENT_PROGRAM 0x8B8D
#	define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#	define GL_LOWER_LEFT 0x8CA1
#	define GL_UPPER_LEFT 0x8CA2
#	define GL_STENCIL_BACK_REF 0x8CA3
#	define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#	define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#	define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#	define GL_POINT_SPRITE 0x8861
#	define GL_COORD_REPLACE 0x8862
#	define GL_MAX_TEXTURE_COORDS 0x8871
	typedef void (FGL_APIENTRY gl_blend_equation_separate_func)(GLenum modeRGB, GLenum modeAlpha);
	static gl_blend_equation_separate_func* glBlendEquationSeparate;
	typedef void (FGL_APIENTRY gl_draw_buffers_func)(GLsizei n, const GLenum *bufs);
	static gl_draw_buffers_func* glDrawBuffers;
	typedef void (FGL_APIENTRY gl_stencil_op_separate_func)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	static gl_stencil_op_separate_func* glStencilOpSeparate;
	typedef void (FGL_APIENTRY gl_stencil_func_separate_func)(GLenum face, GLenum func, GLint ref, GLuint mask);
	static gl_stencil_func_separate_func* glStencilFuncSeparate;
	typedef void (FGL_APIENTRY gl_stencil_mask_separate_func)(GLenum face, GLuint mask);
	static gl_stencil_mask_separate_func* glStencilMaskSeparate;
	typedef void (FGL_APIENTRY gl_attach_shader_func)(GLuint program, GLuint shader);
	static gl_attach_shader_func* glAttachShader;
	typedef void (FGL_APIENTRY gl_bind_attrib_location_func)(GLuint program, GLuint index, const GLchar *name);
	static gl_bind_attrib_location_func* glBindAttribLocation;
	typedef void (FGL_APIENTRY gl_compile_shader_func)(GLuint shader);
	static gl_compile_shader_func* glCompileShader;
	typedef GLuint(FGL_APIENTRY gl_create_program_func)(void);
	static gl_create_program_func* glCreateProgram;
	typedef GLuint(FGL_APIENTRY gl_create_shader_func)(GLenum type);
	static gl_create_shader_func* glCreateShader;
	typedef void (FGL_APIENTRY gl_delete_program_func)(GLuint program);
	static gl_delete_program_func* glDeleteProgram;
	typedef void (FGL_APIENTRY gl_delete_shader_func)(GLuint shader);
	static gl_delete_shader_func* glDeleteShader;
	typedef void (FGL_APIENTRY gl_detach_shader_func)(GLuint program, GLuint shader);
	static gl_detach_shader_func* glDetachShader;
	typedef void (FGL_APIENTRY gl_disable_vertex_attrib_array_func)(GLuint index);
	static gl_disable_vertex_attrib_array_func* glDisableVertexAttribArray;
	typedef void (FGL_APIENTRY gl_enable_vertex_attrib_array_func)(GLuint index);
	static gl_enable_vertex_attrib_array_func* glEnableVertexAttribArray;
	typedef void (FGL_APIENTRY gl_get_active_attrib_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	static gl_get_active_attrib_func* glGetActiveAttrib;
	typedef void (FGL_APIENTRY gl_get_active_uniform_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	static gl_get_active_uniform_func* glGetActiveUniform;
	typedef void (FGL_APIENTRY gl_get_attached_shaders_func)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
	static gl_get_attached_shaders_func* glGetAttachedShaders;
	typedef GLint(FGL_APIENTRY gl_get_attrib_location_func)(GLuint program, const GLchar *name);
	static gl_get_attrib_location_func* glGetAttribLocation;
	typedef void (FGL_APIENTRY gl_get_programiv_func)(GLuint program, GLenum pname, GLint *params);
	static gl_get_programiv_func* glGetProgramiv;
	typedef void (FGL_APIENTRY gl_get_program_info_log_func)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_program_info_log_func* glGetProgramInfoLog;
	typedef void (FGL_APIENTRY gl_get_shaderiv_func)(GLuint shader, GLenum pname, GLint *params);
	static gl_get_shaderiv_func* glGetShaderiv;
	typedef void (FGL_APIENTRY gl_get_shader_info_log_func)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_shader_info_log_func* glGetShaderInfoLog;
	typedef void (FGL_APIENTRY gl_get_shader_source_func)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
	static gl_get_shader_source_func* glGetShaderSource;
	typedef GLint(FGL_APIENTRY gl_get_uniform_location_func)(GLuint program, const GLchar *name);
	static gl_get_uniform_location_func* glGetUniformLocation;
	typedef void (FGL_APIENTRY gl_get_uniformfv_func)(GLuint program, GLint location, GLfloat *params);
	static gl_get_uniformfv_func* glGetUniformfv;
	typedef void (FGL_APIENTRY gl_get_uniformiv_func)(GLuint program, GLint location, GLint *params);
	static gl_get_uniformiv_func* glGetUniformiv;
	typedef void (FGL_APIENTRY gl_get_vertex_attribdv_func)(GLuint index, GLenum pname, GLdouble *params);
	static gl_get_vertex_attribdv_func* glGetVertexAttribdv;
	typedef void (FGL_APIENTRY gl_get_vertex_attribfv_func)(GLuint index, GLenum pname, GLfloat *params);
	static gl_get_vertex_attribfv_func* glGetVertexAttribfv;
	typedef void (FGL_APIENTRY gl_get_vertex_attribiv_func)(GLuint index, GLenum pname, GLint *params);
	static gl_get_vertex_attribiv_func* glGetVertexAttribiv;
	typedef void (FGL_APIENTRY gl_get_vertex_attrib_pointerv_func)(GLuint index, GLenum pname, void **pointer);
	static gl_get_vertex_attrib_pointerv_func* glGetVertexAttribPointerv;
	typedef GLboolean(FGL_APIENTRY gl_is_program_func)(GLuint program);
	static gl_is_program_func* glIsProgram;
	typedef GLboolean(FGL_APIENTRY gl_is_shader_func)(GLuint shader);
	static gl_is_shader_func* glIsShader;
	typedef void (FGL_APIENTRY gl_link_program_func)(GLuint program);
	static gl_link_program_func* glLinkProgram;
	typedef void (FGL_APIENTRY gl_shader_source_func)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
	static gl_shader_source_func* glShaderSource;
	typedef void (FGL_APIENTRY gl_use_program_func)(GLuint program);
	static gl_use_program_func* glUseProgram;
	typedef void (FGL_APIENTRY gl_uniform1f_func)(GLint location, GLfloat v0);
	static gl_uniform1f_func* glUniform1f;
	typedef void (FGL_APIENTRY gl_uniform2f_func)(GLint location, GLfloat v0, GLfloat v1);
	static gl_uniform2f_func* glUniform2f;
	typedef void (FGL_APIENTRY gl_uniform3f_func)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static gl_uniform3f_func* glUniform3f;
	typedef void (FGL_APIENTRY gl_uniform4f_func)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static gl_uniform4f_func* glUniform4f;
	typedef void (FGL_APIENTRY gl_uniform1i_func)(GLint location, GLint v0);
	static gl_uniform1i_func* glUniform1i;
	typedef void (FGL_APIENTRY gl_uniform2i_func)(GLint location, GLint v0, GLint v1);
	static gl_uniform2i_func* glUniform2i;
	typedef void (FGL_APIENTRY gl_uniform3i_func)(GLint location, GLint v0, GLint v1, GLint v2);
	static gl_uniform3i_func* glUniform3i;
	typedef void (FGL_APIENTRY gl_uniform4i_func)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static gl_uniform4i_func* glUniform4i;
	typedef void (FGL_APIENTRY gl_uniform1fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform1fv_func* glUniform1fv;
	typedef void (FGL_APIENTRY gl_uniform2fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform2fv_func* glUniform2fv;
	typedef void (FGL_APIENTRY gl_uniform3fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform3fv_func* glUniform3fv;
	typedef void (FGL_APIENTRY gl_uniform4fv_func)(GLint location, GLsizei count, const GLfloat *value);
	static gl_uniform4fv_func* glUniform4fv;
	typedef void (FGL_APIENTRY gl_uniform1iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform1iv_func* glUniform1iv;
	typedef void (FGL_APIENTRY gl_uniform2iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform2iv_func* glUniform2iv;
	typedef void (FGL_APIENTRY gl_uniform3iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform3iv_func* glUniform3iv;
	typedef void (FGL_APIENTRY gl_uniform4iv_func)(GLint location, GLsizei count, const GLint *value);
	static gl_uniform4iv_func* glUniform4iv;
	typedef void (FGL_APIENTRY gl_uniform_matrix2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2fv_func* glUniformMatrix2fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3fv_func* glUniformMatrix3fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4fv_func* glUniformMatrix4fv;
	typedef void (FGL_APIENTRY gl_validate_program_func)(GLuint program);
	static gl_validate_program_func* glValidateProgram;
	typedef void (FGL_APIENTRY gl_vertex_attrib1d_func)(GLuint index, GLdouble x);
	static gl_vertex_attrib1d_func* glVertexAttrib1d;
	typedef void (FGL_APIENTRY gl_vertex_attrib1dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib1dv_func* glVertexAttrib1dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib1f_func)(GLuint index, GLfloat x);
	static gl_vertex_attrib1f_func* glVertexAttrib1f;
	typedef void (FGL_APIENTRY gl_vertex_attrib1fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib1fv_func* glVertexAttrib1fv;
	typedef void (FGL_APIENTRY gl_vertex_attrib1s_func)(GLuint index, GLshort x);
	static gl_vertex_attrib1s_func* glVertexAttrib1s;
	typedef void (FGL_APIENTRY gl_vertex_attrib1sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib1sv_func* glVertexAttrib1sv;
	typedef void (FGL_APIENTRY gl_vertex_attrib2d_func)(GLuint index, GLdouble x, GLdouble y);
	static gl_vertex_attrib2d_func* glVertexAttrib2d;
	typedef void (FGL_APIENTRY gl_vertex_attrib2dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib2dv_func* glVertexAttrib2dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib2f_func)(GLuint index, GLfloat x, GLfloat y);
	static gl_vertex_attrib2f_func* glVertexAttrib2f;
	typedef void (FGL_APIENTRY gl_vertex_attrib2fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib2fv_func* glVertexAttrib2fv;
	typedef void (FGL_APIENTRY gl_vertex_attrib2s_func)(GLuint index, GLshort x, GLshort y);
	static gl_vertex_attrib2s_func* glVertexAttrib2s;
	typedef void (FGL_APIENTRY gl_vertex_attrib2sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib2sv_func* glVertexAttrib2sv;
	typedef void (FGL_APIENTRY gl_vertex_attrib3d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex_attrib3d_func* glVertexAttrib3d;
	typedef void (FGL_APIENTRY gl_vertex_attrib3dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib3dv_func* glVertexAttrib3dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib3f_func)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
	static gl_vertex_attrib3f_func* glVertexAttrib3f;
	typedef void (FGL_APIENTRY gl_vertex_attrib3fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib3fv_func* glVertexAttrib3fv;
	typedef void (FGL_APIENTRY gl_vertex_attrib3s_func)(GLuint index, GLshort x, GLshort y, GLshort z);
	static gl_vertex_attrib3s_func* glVertexAttrib3s;
	typedef void (FGL_APIENTRY gl_vertex_attrib3sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib3sv_func* glVertexAttrib3sv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nbv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib4_nbv_func* glVertexAttrib4Nbv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_niv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib4_niv_func* glVertexAttrib4Niv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nsv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib4_nsv_func* glVertexAttrib4Nsv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nub_func)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
	static gl_vertex_attrib4_nub_func* glVertexAttrib4Nub;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib4_nubv_func* glVertexAttrib4Nubv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nuiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib4_nuiv_func* glVertexAttrib4Nuiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4_nusv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib4_nusv_func* glVertexAttrib4Nusv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4bv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib4bv_func* glVertexAttrib4bv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex_attrib4d_func* glVertexAttrib4d;
	typedef void (FGL_APIENTRY gl_vertex_attrib4dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib4dv_func* glVertexAttrib4dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4f_func)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static gl_vertex_attrib4f_func* glVertexAttrib4f;
	typedef void (FGL_APIENTRY gl_vertex_attrib4fv_func)(GLuint index, const GLfloat *v);
	static gl_vertex_attrib4fv_func* glVertexAttrib4fv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib4iv_func* glVertexAttrib4iv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4s_func)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
	static gl_vertex_attrib4s_func* glVertexAttrib4s;
	typedef void (FGL_APIENTRY gl_vertex_attrib4sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib4sv_func* glVertexAttrib4sv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4ubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib4ubv_func* glVertexAttrib4ubv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib4uiv_func* glVertexAttrib4uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib4usv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib4usv_func* glVertexAttrib4usv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_pointer_func)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_pointer_func* glVertexAttribPointer;
#	endif // GL_VERSION_2_0
#	ifndef GL_VERSION_2_1
#		define GL_VERSION_2_1 1
	static bool isGL_VERSION_2_1;
#	define GL_PIXEL_PACK_BUFFER 0x88EB
#	define GL_PIXEL_UNPACK_BUFFER 0x88EC
#	define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#	define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#	define GL_FLOAT_MAT2x3 0x8B65
#	define GL_FLOAT_MAT2x4 0x8B66
#	define GL_FLOAT_MAT3x2 0x8B67
#	define GL_FLOAT_MAT3x4 0x8B68
#	define GL_FLOAT_MAT4x2 0x8B69
#	define GL_FLOAT_MAT4x3 0x8B6A
#	define GL_SRGB 0x8C40
#	define GL_SRGB8 0x8C41
#	define GL_SRGB_ALPHA 0x8C42
#	define GL_SRGB8_ALPHA8 0x8C43
#	define GL_COMPRESSED_SRGB 0x8C48
#	define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#	define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#	define GL_SLUMINANCE_ALPHA 0x8C44
#	define GL_SLUMINANCE8_ALPHA8 0x8C45
#	define GL_SLUMINANCE 0x8C46
#	define GL_SLUMINANCE8 0x8C47
#	define GL_COMPRESSED_SLUMINANCE 0x8C4A
#	define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B
	typedef void (FGL_APIENTRY gl_uniform_matrix2x3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2x3fv_func* glUniformMatrix2x3fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3x2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3x2fv_func* glUniformMatrix3x2fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix2x4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix2x4fv_func* glUniformMatrix2x4fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4x2fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4x2fv_func* glUniformMatrix4x2fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3x4fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix3x4fv_func* glUniformMatrix3x4fv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4x3fv_func)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_uniform_matrix4x3fv_func* glUniformMatrix4x3fv;
#	endif // GL_VERSION_2_1
#	ifndef GL_VERSION_3_0
#		define GL_VERSION_3_0 1
	static bool isGL_VERSION_3_0;
#	define GL_COMPARE_REF_TO_TEXTURE 0x884E
#	define GL_CLIP_DISTANCE0 0x3000
#	define GL_CLIP_DISTANCE1 0x3001
#	define GL_CLIP_DISTANCE2 0x3002
#	define GL_CLIP_DISTANCE3 0x3003
#	define GL_CLIP_DISTANCE4 0x3004
#	define GL_CLIP_DISTANCE5 0x3005
#	define GL_CLIP_DISTANCE6 0x3006
#	define GL_CLIP_DISTANCE7 0x3007
#	define GL_MAX_CLIP_DISTANCES 0x0D32
#	define GL_MAJOR_VERSION 0x821B
#	define GL_MINOR_VERSION 0x821C
#	define GL_NUM_EXTENSIONS 0x821D
#	define GL_CONTEXT_FLAGS 0x821E
#	define GL_COMPRESSED_RED 0x8225
#	define GL_COMPRESSED_RG 0x8226
#	define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#	define GL_RGBA32F 0x8814
#	define GL_RGB32F 0x8815
#	define GL_RGBA16F 0x881A
#	define GL_RGB16F 0x881B
#	define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#	define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#	define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#	define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#	define GL_CLAMP_READ_COLOR 0x891C
#	define GL_FIXED_ONLY 0x891D
#	define GL_MAX_VARYING_COMPONENTS 0x8B4B
#	define GL_TEXTURE_1D_ARRAY 0x8C18
#	define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#	define GL_TEXTURE_2D_ARRAY 0x8C1A
#	define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
#	define GL_TEXTURE_BINDING_1D_ARRAY 0x8C1C
#	define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#	define GL_R11F_G11F_B10F 0x8C3A
#	define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#	define GL_RGB9_E5 0x8C3D
#	define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#	define GL_TEXTURE_SHARED_SIZE 0x8C3F
#	define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#	define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#	define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#	define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#	define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#	define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#	define GL_PRIMITIVES_GENERATED 0x8C87
#	define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#	define GL_RASTERIZER_DISCARD 0x8C89
#	define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#	define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#	define GL_INTERLEAVED_ATTRIBS 0x8C8C
#	define GL_SEPARATE_ATTRIBS 0x8C8D
#	define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#	define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#	define GL_RGBA32UI 0x8D70
#	define GL_RGB32UI 0x8D71
#	define GL_RGBA16UI 0x8D76
#	define GL_RGB16UI 0x8D77
#	define GL_RGBA8UI 0x8D7C
#	define GL_RGB8UI 0x8D7D
#	define GL_RGBA32I 0x8D82
#	define GL_RGB32I 0x8D83
#	define GL_RGBA16I 0x8D88
#	define GL_RGB16I 0x8D89
#	define GL_RGBA8I 0x8D8E
#	define GL_RGB8I 0x8D8F
#	define GL_RED_INTEGER 0x8D94
#	define GL_GREEN_INTEGER 0x8D95
#	define GL_BLUE_INTEGER 0x8D96
#	define GL_RGB_INTEGER 0x8D98
#	define GL_RGBA_INTEGER 0x8D99
#	define GL_BGR_INTEGER 0x8D9A
#	define GL_BGRA_INTEGER 0x8D9B
#	define GL_SAMPLER_1D_ARRAY 0x8DC0
#	define GL_SAMPLER_2D_ARRAY 0x8DC1
#	define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#	define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#	define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#	define GL_UNSIGNED_INT_VEC2 0x8DC6
#	define GL_UNSIGNED_INT_VEC3 0x8DC7
#	define GL_UNSIGNED_INT_VEC4 0x8DC8
#	define GL_INT_SAMPLER_1D 0x8DC9
#	define GL_INT_SAMPLER_2D 0x8DCA
#	define GL_INT_SAMPLER_3D 0x8DCB
#	define GL_INT_SAMPLER_CUBE 0x8DCC
#	define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#	define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#	define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#	define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#	define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#	define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#	define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#	define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#	define GL_QUERY_WAIT 0x8E13
#	define GL_QUERY_NO_WAIT 0x8E14
#	define GL_QUERY_BY_REGION_WAIT 0x8E15
#	define GL_QUERY_BY_REGION_NO_WAIT 0x8E16
#	define GL_BUFFER_ACCESS_FLAGS 0x911F
#	define GL_BUFFER_MAP_LENGTH 0x9120
#	define GL_BUFFER_MAP_OFFSET 0x9121
#	define GL_DEPTH_COMPONENT32F 0x8CAC
#	define GL_DEPTH32F_STENCIL8 0x8CAD
#	define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#	define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#	define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#	define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#	define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#	define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#	define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#	define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#	define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#	define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#	define GL_FRAMEBUFFER_DEFAULT 0x8218
#	define GL_FRAMEBUFFER_UNDEFINED 0x8219
#	define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#	define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#	define GL_DEPTH_STENCIL 0x84F9
#	define GL_UNSIGNED_INT_24_8 0x84FA
#	define GL_DEPTH24_STENCIL8 0x88F0
#	define GL_TEXTURE_STENCIL_SIZE 0x88F1
#	define GL_TEXTURE_RED_TYPE 0x8C10
#	define GL_TEXTURE_GREEN_TYPE 0x8C11
#	define GL_TEXTURE_BLUE_TYPE 0x8C12
#	define GL_TEXTURE_ALPHA_TYPE 0x8C13
#	define GL_TEXTURE_DEPTH_TYPE 0x8C16
#	define GL_UNSIGNED_NORMALIZED 0x8C17
#	define GL_FRAMEBUFFER_BINDING 0x8CA6
#	define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#	define GL_RENDERBUFFER_BINDING 0x8CA7
#	define GL_READ_FRAMEBUFFER 0x8CA8
#	define GL_DRAW_FRAMEBUFFER 0x8CA9
#	define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#	define GL_RENDERBUFFER_SAMPLES 0x8CAB
#	define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#	define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#	define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#	define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#	define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#	define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#	define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#	define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#	define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#	define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#	define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#	define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#	define GL_COLOR_ATTACHMENT0 0x8CE0
#	define GL_COLOR_ATTACHMENT1 0x8CE1
#	define GL_COLOR_ATTACHMENT2 0x8CE2
#	define GL_COLOR_ATTACHMENT3 0x8CE3
#	define GL_COLOR_ATTACHMENT4 0x8CE4
#	define GL_COLOR_ATTACHMENT5 0x8CE5
#	define GL_COLOR_ATTACHMENT6 0x8CE6
#	define GL_COLOR_ATTACHMENT7 0x8CE7
#	define GL_COLOR_ATTACHMENT8 0x8CE8
#	define GL_COLOR_ATTACHMENT9 0x8CE9
#	define GL_COLOR_ATTACHMENT10 0x8CEA
#	define GL_COLOR_ATTACHMENT11 0x8CEB
#	define GL_COLOR_ATTACHMENT12 0x8CEC
#	define GL_COLOR_ATTACHMENT13 0x8CED
#	define GL_COLOR_ATTACHMENT14 0x8CEE
#	define GL_COLOR_ATTACHMENT15 0x8CEF
#	define GL_COLOR_ATTACHMENT16 0x8CF0
#	define GL_COLOR_ATTACHMENT17 0x8CF1
#	define GL_COLOR_ATTACHMENT18 0x8CF2
#	define GL_COLOR_ATTACHMENT19 0x8CF3
#	define GL_COLOR_ATTACHMENT20 0x8CF4
#	define GL_COLOR_ATTACHMENT21 0x8CF5
#	define GL_COLOR_ATTACHMENT22 0x8CF6
#	define GL_COLOR_ATTACHMENT23 0x8CF7
#	define GL_COLOR_ATTACHMENT24 0x8CF8
#	define GL_COLOR_ATTACHMENT25 0x8CF9
#	define GL_COLOR_ATTACHMENT26 0x8CFA
#	define GL_COLOR_ATTACHMENT27 0x8CFB
#	define GL_COLOR_ATTACHMENT28 0x8CFC
#	define GL_COLOR_ATTACHMENT29 0x8CFD
#	define GL_COLOR_ATTACHMENT30 0x8CFE
#	define GL_COLOR_ATTACHMENT31 0x8CFF
#	define GL_DEPTH_ATTACHMENT 0x8D00
#	define GL_STENCIL_ATTACHMENT 0x8D20
#	define GL_FRAMEBUFFER 0x8D40
#	define GL_RENDERBUFFER 0x8D41
#	define GL_RENDERBUFFER_WIDTH 0x8D42
#	define GL_RENDERBUFFER_HEIGHT 0x8D43
#	define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#	define GL_STENCIL_INDEX1 0x8D46
#	define GL_STENCIL_INDEX4 0x8D47
#	define GL_STENCIL_INDEX8 0x8D48
#	define GL_STENCIL_INDEX16 0x8D49
#	define GL_RENDERBUFFER_RED_SIZE 0x8D50
#	define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#	define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#	define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#	define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#	define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#	define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#	define GL_MAX_SAMPLES 0x8D57
#	define GL_INDEX 0x8222
#	define GL_TEXTURE_LUMINANCE_TYPE 0x8C14
#	define GL_TEXTURE_INTENSITY_TYPE 0x8C15
#	define GL_FRAMEBUFFER_SRGB 0x8DB9
#	define GL_HALF_FLOAT 0x140B
#	define GL_MAP_READ_BIT 0x0001
#	define GL_MAP_WRITE_BIT 0x0002
#	define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#	define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#	define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#	define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#	define GL_COMPRESSED_RED_RGTC1 0x8DBB
#	define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#	define GL_COMPRESSED_RG_RGTC2 0x8DBD
#	define GL_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#	define GL_RG 0x8227
#	define GL_RG_INTEGER 0x8228
#	define GL_R8 0x8229
#	define GL_R16 0x822A
#	define GL_RG8 0x822B
#	define GL_RG16 0x822C
#	define GL_R16F 0x822D
#	define GL_R32F 0x822E
#	define GL_RG16F 0x822F
#	define GL_RG32F 0x8230
#	define GL_R8I 0x8231
#	define GL_R8UI 0x8232
#	define GL_R16I 0x8233
#	define GL_R16UI 0x8234
#	define GL_R32I 0x8235
#	define GL_R32UI 0x8236
#	define GL_RG8I 0x8237
#	define GL_RG8UI 0x8238
#	define GL_RG16I 0x8239
#	define GL_RG16UI 0x823A
#	define GL_RG32I 0x823B
#	define GL_RG32UI 0x823C
#	define GL_VERTEX_ARRAY_BINDING 0x85B5
#	define GL_CLAMP_VERTEX_COLOR 0x891A
#	define GL_CLAMP_FRAGMENT_COLOR 0x891B
#	define GL_ALPHA_INTEGER 0x8D97
	typedef void (FGL_APIENTRY gl_color_maski_func)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	static gl_color_maski_func* glColorMaski;
	typedef void (FGL_APIENTRY gl_get_booleani_v_func)(GLenum target, GLuint index, GLboolean *data);
	static gl_get_booleani_v_func* glGetBooleani_v;
	typedef void (FGL_APIENTRY gl_get_integeri_v_func)(GLenum target, GLuint index, GLint *data);
	static gl_get_integeri_v_func* glGetIntegeri_v;
	typedef void (FGL_APIENTRY gl_enablei_func)(GLenum target, GLuint index);
	static gl_enablei_func* glEnablei;
	typedef void (FGL_APIENTRY gl_disablei_func)(GLenum target, GLuint index);
	static gl_disablei_func* glDisablei;
	typedef GLboolean(FGL_APIENTRY gl_is_enabledi_func)(GLenum target, GLuint index);
	static gl_is_enabledi_func* glIsEnabledi;
	typedef void (FGL_APIENTRY gl_begin_transform_feedback_func)(GLenum primitiveMode);
	static gl_begin_transform_feedback_func* glBeginTransformFeedback;
	typedef void (FGL_APIENTRY gl_end_transform_feedback_func)(void);
	static gl_end_transform_feedback_func* glEndTransformFeedback;
	typedef void (FGL_APIENTRY gl_bind_buffer_range_func)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_bind_buffer_range_func* glBindBufferRange;
	typedef void (FGL_APIENTRY gl_bind_buffer_base_func)(GLenum target, GLuint index, GLuint buffer);
	static gl_bind_buffer_base_func* glBindBufferBase;
	typedef void (FGL_APIENTRY gl_transform_feedback_varyings_func)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
	static gl_transform_feedback_varyings_func* glTransformFeedbackVaryings;
	typedef void (FGL_APIENTRY gl_get_transform_feedback_varying_func)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
	static gl_get_transform_feedback_varying_func* glGetTransformFeedbackVarying;
	typedef void (FGL_APIENTRY gl_clamp_color_func)(GLenum target, GLenum clamp);
	static gl_clamp_color_func* glClampColor;
	typedef void (FGL_APIENTRY gl_begin_conditional_render_func)(GLuint id, GLenum mode);
	static gl_begin_conditional_render_func* glBeginConditionalRender;
	typedef void (FGL_APIENTRY gl_end_conditional_render_func)(void);
	static gl_end_conditional_render_func* glEndConditionalRender;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i_pointer_func)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_i_pointer_func* glVertexAttribIPointer;
	typedef void (FGL_APIENTRY gl_get_vertex_attrib_iiv_func)(GLuint index, GLenum pname, GLint *params);
	static gl_get_vertex_attrib_iiv_func* glGetVertexAttribIiv;
	typedef void (FGL_APIENTRY gl_get_vertex_attrib_iuiv_func)(GLuint index, GLenum pname, GLuint *params);
	static gl_get_vertex_attrib_iuiv_func* glGetVertexAttribIuiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i1i_func)(GLuint index, GLint x);
	static gl_vertex_attrib_i1i_func* glVertexAttribI1i;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i2i_func)(GLuint index, GLint x, GLint y);
	static gl_vertex_attrib_i2i_func* glVertexAttribI2i;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i3i_func)(GLuint index, GLint x, GLint y, GLint z);
	static gl_vertex_attrib_i3i_func* glVertexAttribI3i;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4i_func)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	static gl_vertex_attrib_i4i_func* glVertexAttribI4i;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i1ui_func)(GLuint index, GLuint x);
	static gl_vertex_attrib_i1ui_func* glVertexAttribI1ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i2ui_func)(GLuint index, GLuint x, GLuint y);
	static gl_vertex_attrib_i2ui_func* glVertexAttribI2ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i3ui_func)(GLuint index, GLuint x, GLuint y, GLuint z);
	static gl_vertex_attrib_i3ui_func* glVertexAttribI3ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4ui_func)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static gl_vertex_attrib_i4ui_func* glVertexAttribI4ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i1iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i1iv_func* glVertexAttribI1iv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i2iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i2iv_func* glVertexAttribI2iv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i3iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i3iv_func* glVertexAttribI3iv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4iv_func)(GLuint index, const GLint *v);
	static gl_vertex_attrib_i4iv_func* glVertexAttribI4iv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i1uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i1uiv_func* glVertexAttribI1uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i2uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i2uiv_func* glVertexAttribI2uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i3uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i3uiv_func* glVertexAttribI3uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4uiv_func)(GLuint index, const GLuint *v);
	static gl_vertex_attrib_i4uiv_func* glVertexAttribI4uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4bv_func)(GLuint index, const GLbyte *v);
	static gl_vertex_attrib_i4bv_func* glVertexAttribI4bv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4sv_func)(GLuint index, const GLshort *v);
	static gl_vertex_attrib_i4sv_func* glVertexAttribI4sv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4ubv_func)(GLuint index, const GLubyte *v);
	static gl_vertex_attrib_i4ubv_func* glVertexAttribI4ubv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i4usv_func)(GLuint index, const GLushort *v);
	static gl_vertex_attrib_i4usv_func* glVertexAttribI4usv;
	typedef void (FGL_APIENTRY gl_get_uniformuiv_func)(GLuint program, GLint location, GLuint *params);
	static gl_get_uniformuiv_func* glGetUniformuiv;
	typedef void (FGL_APIENTRY gl_bind_frag_data_location_func)(GLuint program, GLuint color, const GLchar *name);
	static gl_bind_frag_data_location_func* glBindFragDataLocation;
	typedef GLint(FGL_APIENTRY gl_get_frag_data_location_func)(GLuint program, const GLchar *name);
	static gl_get_frag_data_location_func* glGetFragDataLocation;
	typedef void (FGL_APIENTRY gl_uniform1ui_func)(GLint location, GLuint v0);
	static gl_uniform1ui_func* glUniform1ui;
	typedef void (FGL_APIENTRY gl_uniform2ui_func)(GLint location, GLuint v0, GLuint v1);
	static gl_uniform2ui_func* glUniform2ui;
	typedef void (FGL_APIENTRY gl_uniform3ui_func)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	static gl_uniform3ui_func* glUniform3ui;
	typedef void (FGL_APIENTRY gl_uniform4ui_func)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static gl_uniform4ui_func* glUniform4ui;
	typedef void (FGL_APIENTRY gl_uniform1uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform1uiv_func* glUniform1uiv;
	typedef void (FGL_APIENTRY gl_uniform2uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform2uiv_func* glUniform2uiv;
	typedef void (FGL_APIENTRY gl_uniform3uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform3uiv_func* glUniform3uiv;
	typedef void (FGL_APIENTRY gl_uniform4uiv_func)(GLint location, GLsizei count, const GLuint *value);
	static gl_uniform4uiv_func* glUniform4uiv;
	typedef void (FGL_APIENTRY gl_tex_parameter_iiv_func)(GLenum target, GLenum pname, const GLint *params);
	static gl_tex_parameter_iiv_func* glTexParameterIiv;
	typedef void (FGL_APIENTRY gl_tex_parameter_iuiv_func)(GLenum target, GLenum pname, const GLuint *params);
	static gl_tex_parameter_iuiv_func* glTexParameterIuiv;
	typedef void (FGL_APIENTRY gl_get_tex_parameter_iiv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_tex_parameter_iiv_func* glGetTexParameterIiv;
	typedef void (FGL_APIENTRY gl_get_tex_parameter_iuiv_func)(GLenum target, GLenum pname, GLuint *params);
	static gl_get_tex_parameter_iuiv_func* glGetTexParameterIuiv;
	typedef void (FGL_APIENTRY gl_clear_bufferiv_func)(GLenum buffer, GLint drawbuffer, const GLint *value);
	static gl_clear_bufferiv_func* glClearBufferiv;
	typedef void (FGL_APIENTRY gl_clear_bufferuiv_func)(GLenum buffer, GLint drawbuffer, const GLuint *value);
	static gl_clear_bufferuiv_func* glClearBufferuiv;
	typedef void (FGL_APIENTRY gl_clear_bufferfv_func)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static gl_clear_bufferfv_func* glClearBufferfv;
	typedef void (FGL_APIENTRY gl_clear_bufferfi_func)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static gl_clear_bufferfi_func* glClearBufferfi;
	typedef const GLubyte * (FGL_APIENTRY gl_get_stringi_func)(GLenum name, GLuint index);
	static gl_get_stringi_func* glGetStringi;
	typedef GLboolean(FGL_APIENTRY gl_is_renderbuffer_func)(GLuint renderbuffer);
	static gl_is_renderbuffer_func* glIsRenderbuffer;
	typedef void (FGL_APIENTRY gl_bind_renderbuffer_func)(GLenum target, GLuint renderbuffer);
	static gl_bind_renderbuffer_func* glBindRenderbuffer;
	typedef void (FGL_APIENTRY gl_delete_renderbuffers_func)(GLsizei n, const GLuint *renderbuffers);
	static gl_delete_renderbuffers_func* glDeleteRenderbuffers;
	typedef void (FGL_APIENTRY gl_gen_renderbuffers_func)(GLsizei n, GLuint *renderbuffers);
	static gl_gen_renderbuffers_func* glGenRenderbuffers;
	typedef void (FGL_APIENTRY gl_renderbuffer_storage_func)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_renderbuffer_storage_func* glRenderbufferStorage;
	typedef void (FGL_APIENTRY gl_get_renderbuffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_renderbuffer_parameteriv_func* glGetRenderbufferParameteriv;
	typedef GLboolean(FGL_APIENTRY gl_is_framebuffer_func)(GLuint framebuffer);
	static gl_is_framebuffer_func* glIsFramebuffer;
	typedef void (FGL_APIENTRY gl_bind_framebuffer_func)(GLenum target, GLuint framebuffer);
	static gl_bind_framebuffer_func* glBindFramebuffer;
	typedef void (FGL_APIENTRY gl_delete_framebuffers_func)(GLsizei n, const GLuint *framebuffers);
	static gl_delete_framebuffers_func* glDeleteFramebuffers;
	typedef void (FGL_APIENTRY gl_gen_framebuffers_func)(GLsizei n, GLuint *framebuffers);
	static gl_gen_framebuffers_func* glGenFramebuffers;
	typedef GLenum(FGL_APIENTRY gl_check_framebuffer_status_func)(GLenum target);
	static gl_check_framebuffer_status_func* glCheckFramebufferStatus;
	typedef void (FGL_APIENTRY gl_framebuffer_texture1d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static gl_framebuffer_texture1d_func* glFramebufferTexture1D;
	typedef void (FGL_APIENTRY gl_framebuffer_texture2d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static gl_framebuffer_texture2d_func* glFramebufferTexture2D;
	typedef void (FGL_APIENTRY gl_framebuffer_texture3d_func)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static gl_framebuffer_texture3d_func* glFramebufferTexture3D;
	typedef void (FGL_APIENTRY gl_framebuffer_renderbuffer_func)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static gl_framebuffer_renderbuffer_func* glFramebufferRenderbuffer;
	typedef void (FGL_APIENTRY gl_get_framebuffer_attachment_parameteriv_func)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
	static gl_get_framebuffer_attachment_parameteriv_func* glGetFramebufferAttachmentParameteriv;
	typedef void (FGL_APIENTRY gl_generate_mipmap_func)(GLenum target);
	static gl_generate_mipmap_func* glGenerateMipmap;
	typedef void (FGL_APIENTRY gl_blit_framebuffer_func)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static gl_blit_framebuffer_func* glBlitFramebuffer;
	typedef void (FGL_APIENTRY gl_renderbuffer_storage_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_renderbuffer_storage_multisample_func* glRenderbufferStorageMultisample;
	typedef void (FGL_APIENTRY gl_framebuffer_texture_layer_func)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static gl_framebuffer_texture_layer_func* glFramebufferTextureLayer;
	typedef void * (FGL_APIENTRY gl_map_buffer_range_func)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static gl_map_buffer_range_func* glMapBufferRange;
	typedef void (FGL_APIENTRY gl_flush_mapped_buffer_range_func)(GLenum target, GLintptr offset, GLsizeiptr length);
	static gl_flush_mapped_buffer_range_func* glFlushMappedBufferRange;
	typedef void (FGL_APIENTRY gl_bind_vertex_array_func)(GLuint array);
	static gl_bind_vertex_array_func* glBindVertexArray;
	typedef void (FGL_APIENTRY gl_delete_vertex_arrays_func)(GLsizei n, const GLuint *arrays);
	static gl_delete_vertex_arrays_func* glDeleteVertexArrays;
	typedef void (FGL_APIENTRY gl_gen_vertex_arrays_func)(GLsizei n, GLuint *arrays);
	static gl_gen_vertex_arrays_func* glGenVertexArrays;
	typedef GLboolean(FGL_APIENTRY gl_is_vertex_array_func)(GLuint array);
	static gl_is_vertex_array_func* glIsVertexArray;
#	endif // GL_VERSION_3_0
#	ifndef GL_VERSION_3_1
#		define GL_VERSION_3_1 1
	static bool isGL_VERSION_3_1;
#	define GL_SAMPLER_2D_RECT 0x8B63
#	define GL_SAMPLER_2D_RECT_SHADOW 0x8B64
#	define GL_SAMPLER_BUFFER 0x8DC2
#	define GL_INT_SAMPLER_2D_RECT 0x8DCD
#	define GL_INT_SAMPLER_BUFFER 0x8DD0
#	define GL_UNSIGNED_INT_SAMPLER_2D_RECT 0x8DD5
#	define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#	define GL_TEXTURE_BUFFER 0x8C2A
#	define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#	define GL_TEXTURE_BINDING_BUFFER 0x8C2C
#	define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#	define GL_TEXTURE_RECTANGLE 0x84F5
#	define GL_TEXTURE_BINDING_RECTANGLE 0x84F6
#	define GL_PROXY_TEXTURE_RECTANGLE 0x84F7
#	define GL_MAX_RECTANGLE_TEXTURE_SIZE 0x84F8
#	define GL_R8_SNORM 0x8F94
#	define GL_RG8_SNORM 0x8F95
#	define GL_RGB8_SNORM 0x8F96
#	define GL_RGBA8_SNORM 0x8F97
#	define GL_R16_SNORM 0x8F98
#	define GL_RG16_SNORM 0x8F99
#	define GL_RGB16_SNORM 0x8F9A
#	define GL_RGBA16_SNORM 0x8F9B
#	define GL_SIGNED_NORMALIZED 0x8F9C
#	define GL_PRIMITIVE_RESTART 0x8F9D
#	define GL_PRIMITIVE_RESTART_INDEX 0x8F9E
#	define GL_COPY_READ_BUFFER 0x8F36
#	define GL_COPY_WRITE_BUFFER 0x8F37
#	define GL_UNIFORM_BUFFER 0x8A11
#	define GL_UNIFORM_BUFFER_BINDING 0x8A28
#	define GL_UNIFORM_BUFFER_START 0x8A29
#	define GL_UNIFORM_BUFFER_SIZE 0x8A2A
#	define GL_MAX_VERTEX_UNIFORM_BLOCKS 0x8A2B
#	define GL_MAX_GEOMETRY_UNIFORM_BLOCKS 0x8A2C
#	define GL_MAX_FRAGMENT_UNIFORM_BLOCKS 0x8A2D
#	define GL_MAX_COMBINED_UNIFORM_BLOCKS 0x8A2E
#	define GL_MAX_UNIFORM_BUFFER_BINDINGS 0x8A2F
#	define GL_MAX_UNIFORM_BLOCK_SIZE 0x8A30
#	define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#	define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#	define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#	define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#	define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#	define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#	define GL_UNIFORM_TYPE 0x8A37
#	define GL_UNIFORM_SIZE 0x8A38
#	define GL_UNIFORM_NAME_LENGTH 0x8A39
#	define GL_UNIFORM_BLOCK_INDEX 0x8A3A
#	define GL_UNIFORM_OFFSET 0x8A3B
#	define GL_UNIFORM_ARRAY_STRIDE 0x8A3C
#	define GL_UNIFORM_MATRIX_STRIDE 0x8A3D
#	define GL_UNIFORM_IS_ROW_MAJOR 0x8A3E
#	define GL_UNIFORM_BLOCK_BINDING 0x8A3F
#	define GL_UNIFORM_BLOCK_DATA_SIZE 0x8A40
#	define GL_UNIFORM_BLOCK_NAME_LENGTH 0x8A41
#	define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS 0x8A42
#	define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#	define GL_INVALID_INDEX 0xFFFFFFFFu
	typedef void (FGL_APIENTRY gl_draw_arrays_instanced_func)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
	static gl_draw_arrays_instanced_func* glDrawArraysInstanced;
	typedef void (FGL_APIENTRY gl_draw_elements_instanced_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
	static gl_draw_elements_instanced_func* glDrawElementsInstanced;
	typedef void (FGL_APIENTRY gl_tex_buffer_func)(GLenum target, GLenum internalformat, GLuint buffer);
	static gl_tex_buffer_func* glTexBuffer;
	typedef void (FGL_APIENTRY gl_primitive_restart_index_func)(GLuint index);
	static gl_primitive_restart_index_func* glPrimitiveRestartIndex;
	typedef void (FGL_APIENTRY gl_copy_buffer_sub_data_func)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static gl_copy_buffer_sub_data_func* glCopyBufferSubData;
	typedef void (FGL_APIENTRY gl_get_uniform_indices_func)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
	static gl_get_uniform_indices_func* glGetUniformIndices;
	typedef void (FGL_APIENTRY gl_get_active_uniformsiv_func)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
	static gl_get_active_uniformsiv_func* glGetActiveUniformsiv;
	typedef void (FGL_APIENTRY gl_get_active_uniform_name_func)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
	static gl_get_active_uniform_name_func* glGetActiveUniformName;
	typedef GLuint(FGL_APIENTRY gl_get_uniform_block_index_func)(GLuint program, const GLchar *uniformBlockName);
	static gl_get_uniform_block_index_func* glGetUniformBlockIndex;
	typedef void (FGL_APIENTRY gl_get_active_uniform_blockiv_func)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
	static gl_get_active_uniform_blockiv_func* glGetActiveUniformBlockiv;
	typedef void (FGL_APIENTRY gl_get_active_uniform_block_name_func)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
	static gl_get_active_uniform_block_name_func* glGetActiveUniformBlockName;
	typedef void (FGL_APIENTRY gl_uniform_block_binding_func)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	static gl_uniform_block_binding_func* glUniformBlockBinding;
#	endif // GL_VERSION_3_1
#	ifndef GL_VERSION_3_2
#		define GL_VERSION_3_2 1
	static bool isGL_VERSION_3_2;
	typedef struct __GLsync *GLsync;
	typedef uint64_t GLuint64;
	typedef int64_t GLint64;
#	define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#	define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#	define GL_LINES_ADJACENCY 0x000A
#	define GL_LINE_STRIP_ADJACENCY 0x000B
#	define GL_TRIANGLES_ADJACENCY 0x000C
#	define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#	define GL_PROGRAM_POINT_SIZE 0x8642
#	define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#	define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#	define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#	define GL_GEOMETRY_SHADER 0x8DD9
#	define GL_GEOMETRY_VERTICES_OUT 0x8916
#	define GL_GEOMETRY_INPUT_TYPE 0x8917
#	define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#	define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#	define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#	define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#	define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#	define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#	define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#	define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#	define GL_CONTEXT_PROFILE_MASK 0x9126
#	define GL_DEPTH_CLAMP 0x864F
#	define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#	define GL_FIRST_VERTEX_CONVENTION 0x8E4D
#	define GL_LAST_VERTEX_CONVENTION 0x8E4E
#	define GL_PROVOKING_VERTEX 0x8E4F
#	define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#	define GL_MAX_SERVER_WAIT_TIMEOUT 0x9111
#	define GL_OBJECT_TYPE 0x9112
#	define GL_SYNC_CONDITION 0x9113
#	define GL_SYNC_STATUS 0x9114
#	define GL_SYNC_FLAGS 0x9115
#	define GL_SYNC_FENCE 0x9116
#	define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#	define GL_UNSIGNALED 0x9118
#	define GL_SIGNALED 0x9119
#	define GL_ALREADY_SIGNALED 0x911A
#	define GL_TIMEOUT_EXPIRED 0x911B
#	define GL_CONDITION_SATISFIED 0x911C
#	define GL_WAIT_FAILED 0x911D
#	define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFFull
#	define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#	define GL_SAMPLE_POSITION 0x8E50
#	define GL_SAMPLE_MASK 0x8E51
#	define GL_SAMPLE_MASK_VALUE 0x8E52
#	define GL_MAX_SAMPLE_MASK_WORDS 0x8E59
#	define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#	define GL_PROXY_TEXTURE_2D_MULTISAMPLE 0x9101
#	define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102
#	define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#	define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#	define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#	define GL_TEXTURE_SAMPLES 0x9106
#	define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#	define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#	define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#	define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#	define GL_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910B
#	define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#	define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#	define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#	define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#	define GL_MAX_INTEGER_SAMPLES 0x9110
	typedef void (FGL_APIENTRY gl_draw_elements_base_vertex_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
	static gl_draw_elements_base_vertex_func* glDrawElementsBaseVertex;
	typedef void (FGL_APIENTRY gl_draw_range_elements_base_vertex_func)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
	static gl_draw_range_elements_base_vertex_func* glDrawRangeElementsBaseVertex;
	typedef void (FGL_APIENTRY gl_draw_elements_instanced_base_vertex_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
	static gl_draw_elements_instanced_base_vertex_func* glDrawElementsInstancedBaseVertex;
	typedef void (FGL_APIENTRY gl_multi_draw_elements_base_vertex_func)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
	static gl_multi_draw_elements_base_vertex_func* glMultiDrawElementsBaseVertex;
	typedef void (FGL_APIENTRY gl_provoking_vertex_func)(GLenum mode);
	static gl_provoking_vertex_func* glProvokingVertex;
	typedef GLsync(FGL_APIENTRY gl_fence_sync_func)(GLenum condition, GLbitfield flags);
	static gl_fence_sync_func* glFenceSync;
	typedef GLboolean(FGL_APIENTRY gl_is_sync_func)(GLsync sync);
	static gl_is_sync_func* glIsSync;
	typedef void (FGL_APIENTRY gl_delete_sync_func)(GLsync sync);
	static gl_delete_sync_func* glDeleteSync;
	typedef GLenum(FGL_APIENTRY gl_client_wait_sync_func)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static gl_client_wait_sync_func* glClientWaitSync;
	typedef void (FGL_APIENTRY gl_wait_sync_func)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static gl_wait_sync_func* glWaitSync;
	typedef void (FGL_APIENTRY gl_get_integer64v_func)(GLenum pname, GLint64 *data);
	static gl_get_integer64v_func* glGetInteger64v;
	typedef void (FGL_APIENTRY gl_get_synciv_func)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
	static gl_get_synciv_func* glGetSynciv;
	typedef void (FGL_APIENTRY gl_get_integer64i_v_func)(GLenum target, GLuint index, GLint64 *data);
	static gl_get_integer64i_v_func* glGetInteger64i_v;
	typedef void (FGL_APIENTRY gl_get_buffer_parameteri64v_func)(GLenum target, GLenum pname, GLint64 *params);
	static gl_get_buffer_parameteri64v_func* glGetBufferParameteri64v;
	typedef void (FGL_APIENTRY gl_framebuffer_texture_func)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static gl_framebuffer_texture_func* glFramebufferTexture;
	typedef void (FGL_APIENTRY gl_tex_image2_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_tex_image2_d_multisample_func* glTexImage2DMultisample;
	typedef void (FGL_APIENTRY gl_tex_image3_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_tex_image3_d_multisample_func* glTexImage3DMultisample;
	typedef void (FGL_APIENTRY gl_get_multisamplefv_func)(GLenum pname, GLuint index, GLfloat *val);
	static gl_get_multisamplefv_func* glGetMultisamplefv;
	typedef void (FGL_APIENTRY gl_sample_maski_func)(GLuint maskNumber, GLbitfield mask);
	static gl_sample_maski_func* glSampleMaski;
#	endif // GL_VERSION_3_2
#	ifndef GL_VERSION_3_3
#		define GL_VERSION_3_3 1
	static bool isGL_VERSION_3_3;
#	define GL_VERTEX_ATTRIB_ARRAY_DIVISOR 0x88FE
#	define GL_SRC1_COLOR 0x88F9
#	define GL_ONE_MINUS_SRC1_COLOR 0x88FA
#	define GL_ONE_MINUS_SRC1_ALPHA 0x88FB
#	define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS 0x88FC
#	define GL_ANY_SAMPLES_PASSED 0x8C2F
#	define GL_SAMPLER_BINDING 0x8919
#	define GL_RGB10_A2UI 0x906F
#	define GL_TEXTURE_SWIZZLE_R 0x8E42
#	define GL_TEXTURE_SWIZZLE_G 0x8E43
#	define GL_TEXTURE_SWIZZLE_B 0x8E44
#	define GL_TEXTURE_SWIZZLE_A 0x8E45
#	define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#	define GL_TIME_ELAPSED 0x88BF
#	define GL_TIMESTAMP 0x8E28
#	define GL_INT_2_10_10_10_REV 0x8D9F
	typedef void (FGL_APIENTRY gl_bind_frag_data_location_indexed_func)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
	static gl_bind_frag_data_location_indexed_func* glBindFragDataLocationIndexed;
	typedef GLint(FGL_APIENTRY gl_get_frag_data_index_func)(GLuint program, const GLchar *name);
	static gl_get_frag_data_index_func* glGetFragDataIndex;
	typedef void (FGL_APIENTRY gl_gen_samplers_func)(GLsizei count, GLuint *samplers);
	static gl_gen_samplers_func* glGenSamplers;
	typedef void (FGL_APIENTRY gl_delete_samplers_func)(GLsizei count, const GLuint *samplers);
	static gl_delete_samplers_func* glDeleteSamplers;
	typedef GLboolean(FGL_APIENTRY gl_is_sampler_func)(GLuint sampler);
	static gl_is_sampler_func* glIsSampler;
	typedef void (FGL_APIENTRY gl_bind_sampler_func)(GLuint unit, GLuint sampler);
	static gl_bind_sampler_func* glBindSampler;
	typedef void (FGL_APIENTRY gl_sampler_parameteri_func)(GLuint sampler, GLenum pname, GLint param);
	static gl_sampler_parameteri_func* glSamplerParameteri;
	typedef void (FGL_APIENTRY gl_sampler_parameteriv_func)(GLuint sampler, GLenum pname, const GLint *param);
	static gl_sampler_parameteriv_func* glSamplerParameteriv;
	typedef void (FGL_APIENTRY gl_sampler_parameterf_func)(GLuint sampler, GLenum pname, GLfloat param);
	static gl_sampler_parameterf_func* glSamplerParameterf;
	typedef void (FGL_APIENTRY gl_sampler_parameterfv_func)(GLuint sampler, GLenum pname, const GLfloat *param);
	static gl_sampler_parameterfv_func* glSamplerParameterfv;
	typedef void (FGL_APIENTRY gl_sampler_parameter_iiv_func)(GLuint sampler, GLenum pname, const GLint *param);
	static gl_sampler_parameter_iiv_func* glSamplerParameterIiv;
	typedef void (FGL_APIENTRY gl_sampler_parameter_iuiv_func)(GLuint sampler, GLenum pname, const GLuint *param);
	static gl_sampler_parameter_iuiv_func* glSamplerParameterIuiv;
	typedef void (FGL_APIENTRY gl_get_sampler_parameteriv_func)(GLuint sampler, GLenum pname, GLint *params);
	static gl_get_sampler_parameteriv_func* glGetSamplerParameteriv;
	typedef void (FGL_APIENTRY gl_get_sampler_parameter_iiv_func)(GLuint sampler, GLenum pname, GLint *params);
	static gl_get_sampler_parameter_iiv_func* glGetSamplerParameterIiv;
	typedef void (FGL_APIENTRY gl_get_sampler_parameterfv_func)(GLuint sampler, GLenum pname, GLfloat *params);
	static gl_get_sampler_parameterfv_func* glGetSamplerParameterfv;
	typedef void (FGL_APIENTRY gl_get_sampler_parameter_iuiv_func)(GLuint sampler, GLenum pname, GLuint *params);
	static gl_get_sampler_parameter_iuiv_func* glGetSamplerParameterIuiv;
	typedef void (FGL_APIENTRY gl_query_counter_func)(GLuint id, GLenum target);
	static gl_query_counter_func* glQueryCounter;
	typedef void (FGL_APIENTRY gl_get_query_objecti64v_func)(GLuint id, GLenum pname, GLint64 *params);
	static gl_get_query_objecti64v_func* glGetQueryObjecti64v;
	typedef void (FGL_APIENTRY gl_get_query_objectui64v_func)(GLuint id, GLenum pname, GLuint64 *params);
	static gl_get_query_objectui64v_func* glGetQueryObjectui64v;
	typedef void (FGL_APIENTRY gl_vertex_attrib_divisor_func)(GLuint index, GLuint divisor);
	static gl_vertex_attrib_divisor_func* glVertexAttribDivisor;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p1ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p1ui_func* glVertexAttribP1ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p1uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p1uiv_func* glVertexAttribP1uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p2ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p2ui_func* glVertexAttribP2ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p2uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p2uiv_func* glVertexAttribP2uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p3ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p3ui_func* glVertexAttribP3ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p3uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p3uiv_func* glVertexAttribP3uiv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p4ui_func)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static gl_vertex_attrib_p4ui_func* glVertexAttribP4ui;
	typedef void (FGL_APIENTRY gl_vertex_attrib_p4uiv_func)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static gl_vertex_attrib_p4uiv_func* glVertexAttribP4uiv;
	typedef void (FGL_APIENTRY gl_vertex_p2ui_func)(GLenum type, GLuint value);
	static gl_vertex_p2ui_func* glVertexP2ui;
	typedef void (FGL_APIENTRY gl_vertex_p2uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p2uiv_func* glVertexP2uiv;
	typedef void (FGL_APIENTRY gl_vertex_p3ui_func)(GLenum type, GLuint value);
	static gl_vertex_p3ui_func* glVertexP3ui;
	typedef void (FGL_APIENTRY gl_vertex_p3uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p3uiv_func* glVertexP3uiv;
	typedef void (FGL_APIENTRY gl_vertex_p4ui_func)(GLenum type, GLuint value);
	static gl_vertex_p4ui_func* glVertexP4ui;
	typedef void (FGL_APIENTRY gl_vertex_p4uiv_func)(GLenum type, const GLuint *value);
	static gl_vertex_p4uiv_func* glVertexP4uiv;
	typedef void (FGL_APIENTRY gl_tex_coord_p1ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p1ui_func* glTexCoordP1ui;
	typedef void (FGL_APIENTRY gl_tex_coord_p1uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p1uiv_func* glTexCoordP1uiv;
	typedef void (FGL_APIENTRY gl_tex_coord_p2ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p2ui_func* glTexCoordP2ui;
	typedef void (FGL_APIENTRY gl_tex_coord_p2uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p2uiv_func* glTexCoordP2uiv;
	typedef void (FGL_APIENTRY gl_tex_coord_p3ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p3ui_func* glTexCoordP3ui;
	typedef void (FGL_APIENTRY gl_tex_coord_p3uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p3uiv_func* glTexCoordP3uiv;
	typedef void (FGL_APIENTRY gl_tex_coord_p4ui_func)(GLenum type, GLuint coords);
	static gl_tex_coord_p4ui_func* glTexCoordP4ui;
	typedef void (FGL_APIENTRY gl_tex_coord_p4uiv_func)(GLenum type, const GLuint *coords);
	static gl_tex_coord_p4uiv_func* glTexCoordP4uiv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p1ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p1ui_func* glMultiTexCoordP1ui;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p1uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p1uiv_func* glMultiTexCoordP1uiv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p2ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p2ui_func* glMultiTexCoordP2ui;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p2uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p2uiv_func* glMultiTexCoordP2uiv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p3ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p3ui_func* glMultiTexCoordP3ui;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p3uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p3uiv_func* glMultiTexCoordP3uiv;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p4ui_func)(GLenum texture, GLenum type, GLuint coords);
	static gl_multi_tex_coord_p4ui_func* glMultiTexCoordP4ui;
	typedef void (FGL_APIENTRY gl_multi_tex_coord_p4uiv_func)(GLenum texture, GLenum type, const GLuint *coords);
	static gl_multi_tex_coord_p4uiv_func* glMultiTexCoordP4uiv;
	typedef void (FGL_APIENTRY gl_normal_p3ui_func)(GLenum type, GLuint coords);
	static gl_normal_p3ui_func* glNormalP3ui;
	typedef void (FGL_APIENTRY gl_normal_p3uiv_func)(GLenum type, const GLuint *coords);
	static gl_normal_p3uiv_func* glNormalP3uiv;
	typedef void (FGL_APIENTRY gl_color_p3ui_func)(GLenum type, GLuint color);
	static gl_color_p3ui_func* glColorP3ui;
	typedef void (FGL_APIENTRY gl_color_p3uiv_func)(GLenum type, const GLuint *color);
	static gl_color_p3uiv_func* glColorP3uiv;
	typedef void (FGL_APIENTRY gl_color_p4ui_func)(GLenum type, GLuint color);
	static gl_color_p4ui_func* glColorP4ui;
	typedef void (FGL_APIENTRY gl_color_p4uiv_func)(GLenum type, const GLuint *color);
	static gl_color_p4uiv_func* glColorP4uiv;
	typedef void (FGL_APIENTRY gl_secondary_color_p3ui_func)(GLenum type, GLuint color);
	static gl_secondary_color_p3ui_func* glSecondaryColorP3ui;
	typedef void (FGL_APIENTRY gl_secondary_color_p3uiv_func)(GLenum type, const GLuint *color);
	static gl_secondary_color_p3uiv_func* glSecondaryColorP3uiv;
#	endif // GL_VERSION_3_3
#	ifndef GL_VERSION_4_0
#		define GL_VERSION_4_0 1
	static bool isGL_VERSION_4_0;
#	define GL_SAMPLE_SHADING 0x8C36
#	define GL_MIN_SAMPLE_SHADING_VALUE 0x8C37
#	define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#	define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#	define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#	define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#	define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY 0x900B
#	define GL_SAMPLER_CUBE_MAP_ARRAY 0x900C
#	define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW 0x900D
#	define GL_INT_SAMPLER_CUBE_MAP_ARRAY 0x900E
#	define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#	define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#	define GL_DRAW_INDIRECT_BUFFER_BINDING 0x8F43
#	define GL_GEOMETRY_SHADER_INVOCATIONS 0x887F
#	define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#	define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#	define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#	define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#	define GL_MAX_VERTEX_STREAMS 0x8E71
#	define GL_DOUBLE_VEC2 0x8FFC
#	define GL_DOUBLE_VEC3 0x8FFD
#	define GL_DOUBLE_VEC4 0x8FFE
#	define GL_DOUBLE_MAT2 0x8F46
#	define GL_DOUBLE_MAT3 0x8F47
#	define GL_DOUBLE_MAT4 0x8F48
#	define GL_DOUBLE_MAT2x3 0x8F49
#	define GL_DOUBLE_MAT2x4 0x8F4A
#	define GL_DOUBLE_MAT3x2 0x8F4B
#	define GL_DOUBLE_MAT3x4 0x8F4C
#	define GL_DOUBLE_MAT4x2 0x8F4D
#	define GL_DOUBLE_MAT4x3 0x8F4E
#	define GL_ACTIVE_SUBROUTINES 0x8DE5
#	define GL_ACTIVE_SUBROUTINE_UNIFORMS 0x8DE6
#	define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS 0x8E47
#	define GL_ACTIVE_SUBROUTINE_MAX_LENGTH 0x8E48
#	define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH 0x8E49
#	define GL_MAX_SUBROUTINES 0x8DE7
#	define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS 0x8DE8
#	define GL_NUM_COMPATIBLE_SUBROUTINES 0x8E4A
#	define GL_COMPATIBLE_SUBROUTINES 0x8E4B
#	define GL_PATCHES 0x000E
#	define GL_PATCH_VERTICES 0x8E72
#	define GL_PATCH_DEFAULT_INNER_LEVEL 0x8E73
#	define GL_PATCH_DEFAULT_OUTER_LEVEL 0x8E74
#	define GL_TESS_CONTROL_OUTPUT_VERTICES 0x8E75
#	define GL_TESS_GEN_MODE 0x8E76
#	define GL_TESS_GEN_SPACING 0x8E77
#	define GL_TESS_GEN_VERTEX_ORDER 0x8E78
#	define GL_TESS_GEN_POINT_MODE 0x8E79
#	define GL_ISOLINES 0x8E7A
#	define GL_FRACTIONAL_ODD 0x8E7B
#	define GL_FRACTIONAL_EVEN 0x8E7C
#	define GL_MAX_PATCH_VERTICES 0x8E7D
#	define GL_MAX_TESS_GEN_LEVEL 0x8E7E
#	define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E7F
#	define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E80
#	define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS 0x8E81
#	define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS 0x8E82
#	define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS 0x8E83
#	define GL_MAX_TESS_PATCH_COMPONENTS 0x8E84
#	define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS 0x8E85
#	define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS 0x8E86
#	define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS 0x8E89
#	define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS 0x8E8A
#	define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS 0x886C
#	define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS 0x886D
#	define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E1E
#	define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER 0x84F0
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER 0x84F1
#	define GL_TESS_EVALUATION_SHADER 0x8E87
#	define GL_TESS_CONTROL_SHADER 0x8E88
#	define GL_TRANSFORM_FEEDBACK 0x8E22
#	define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED 0x8E23
#	define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE 0x8E24
#	define GL_TRANSFORM_FEEDBACK_BINDING 0x8E25
#	define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS 0x8E70
	typedef void (FGL_APIENTRY gl_min_sample_shading_func)(GLfloat value);
	static gl_min_sample_shading_func* glMinSampleShading;
	typedef void (FGL_APIENTRY gl_blend_equationi_func)(GLuint buf, GLenum mode);
	static gl_blend_equationi_func* glBlendEquationi;
	typedef void (FGL_APIENTRY gl_blend_equation_separatei_func)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
	static gl_blend_equation_separatei_func* glBlendEquationSeparatei;
	typedef void (FGL_APIENTRY gl_blend_funci_func)(GLuint buf, GLenum src, GLenum dst);
	static gl_blend_funci_func* glBlendFunci;
	typedef void (FGL_APIENTRY gl_blend_func_separatei_func)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
	static gl_blend_func_separatei_func* glBlendFuncSeparatei;
	typedef void (FGL_APIENTRY gl_draw_arrays_indirect_func)(GLenum mode, const void *indirect);
	static gl_draw_arrays_indirect_func* glDrawArraysIndirect;
	typedef void (FGL_APIENTRY gl_draw_elements_indirect_func)(GLenum mode, GLenum type, const void *indirect);
	static gl_draw_elements_indirect_func* glDrawElementsIndirect;
	typedef void (FGL_APIENTRY gl_uniform1d_func)(GLint location, GLdouble x);
	static gl_uniform1d_func* glUniform1d;
	typedef void (FGL_APIENTRY gl_uniform2d_func)(GLint location, GLdouble x, GLdouble y);
	static gl_uniform2d_func* glUniform2d;
	typedef void (FGL_APIENTRY gl_uniform3d_func)(GLint location, GLdouble x, GLdouble y, GLdouble z);
	static gl_uniform3d_func* glUniform3d;
	typedef void (FGL_APIENTRY gl_uniform4d_func)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_uniform4d_func* glUniform4d;
	typedef void (FGL_APIENTRY gl_uniform1dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform1dv_func* glUniform1dv;
	typedef void (FGL_APIENTRY gl_uniform2dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform2dv_func* glUniform2dv;
	typedef void (FGL_APIENTRY gl_uniform3dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform3dv_func* glUniform3dv;
	typedef void (FGL_APIENTRY gl_uniform4dv_func)(GLint location, GLsizei count, const GLdouble *value);
	static gl_uniform4dv_func* glUniform4dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2dv_func* glUniformMatrix2dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3dv_func* glUniformMatrix3dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4dv_func* glUniformMatrix4dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix2x3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2x3dv_func* glUniformMatrix2x3dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix2x4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix2x4dv_func* glUniformMatrix2x4dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3x2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3x2dv_func* glUniformMatrix3x2dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix3x4dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix3x4dv_func* glUniformMatrix3x4dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4x2dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4x2dv_func* glUniformMatrix4x2dv;
	typedef void (FGL_APIENTRY gl_uniform_matrix4x3dv_func)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_uniform_matrix4x3dv_func* glUniformMatrix4x3dv;
	typedef void (FGL_APIENTRY gl_get_uniformdv_func)(GLuint program, GLint location, GLdouble *params);
	static gl_get_uniformdv_func* glGetUniformdv;
	typedef GLint(FGL_APIENTRY gl_get_subroutine_uniform_location_func)(GLuint program, GLenum shadertype, const GLchar *name);
	static gl_get_subroutine_uniform_location_func* glGetSubroutineUniformLocation;
	typedef GLuint(FGL_APIENTRY gl_get_subroutine_index_func)(GLuint program, GLenum shadertype, const GLchar *name);
	static gl_get_subroutine_index_func* glGetSubroutineIndex;
	typedef void (FGL_APIENTRY gl_get_active_subroutine_uniformiv_func)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
	static gl_get_active_subroutine_uniformiv_func* glGetActiveSubroutineUniformiv;
	typedef void (FGL_APIENTRY gl_get_active_subroutine_uniform_name_func)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static gl_get_active_subroutine_uniform_name_func* glGetActiveSubroutineUniformName;
	typedef void (FGL_APIENTRY gl_get_active_subroutine_name_func)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static gl_get_active_subroutine_name_func* glGetActiveSubroutineName;
	typedef void (FGL_APIENTRY gl_uniform_subroutinesuiv_func)(GLenum shadertype, GLsizei count, const GLuint *indices);
	static gl_uniform_subroutinesuiv_func* glUniformSubroutinesuiv;
	typedef void (FGL_APIENTRY gl_get_uniform_subroutineuiv_func)(GLenum shadertype, GLint location, GLuint *params);
	static gl_get_uniform_subroutineuiv_func* glGetUniformSubroutineuiv;
	typedef void (FGL_APIENTRY gl_get_program_stageiv_func)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
	static gl_get_program_stageiv_func* glGetProgramStageiv;
	typedef void (FGL_APIENTRY gl_patch_parameteri_func)(GLenum pname, GLint value);
	static gl_patch_parameteri_func* glPatchParameteri;
	typedef void (FGL_APIENTRY gl_patch_parameterfv_func)(GLenum pname, const GLfloat *values);
	static gl_patch_parameterfv_func* glPatchParameterfv;
	typedef void (FGL_APIENTRY gl_bind_transform_feedback_func)(GLenum target, GLuint id);
	static gl_bind_transform_feedback_func* glBindTransformFeedback;
	typedef void (FGL_APIENTRY gl_delete_transform_feedbacks_func)(GLsizei n, const GLuint *ids);
	static gl_delete_transform_feedbacks_func* glDeleteTransformFeedbacks;
	typedef void (FGL_APIENTRY gl_gen_transform_feedbacks_func)(GLsizei n, GLuint *ids);
	static gl_gen_transform_feedbacks_func* glGenTransformFeedbacks;
	typedef GLboolean(FGL_APIENTRY gl_is_transform_feedback_func)(GLuint id);
	static gl_is_transform_feedback_func* glIsTransformFeedback;
	typedef void (FGL_APIENTRY gl_pause_transform_feedback_func)(void);
	static gl_pause_transform_feedback_func* glPauseTransformFeedback;
	typedef void (FGL_APIENTRY gl_resume_transform_feedback_func)(void);
	static gl_resume_transform_feedback_func* glResumeTransformFeedback;
	typedef void (FGL_APIENTRY gl_draw_transform_feedback_func)(GLenum mode, GLuint id);
	static gl_draw_transform_feedback_func* glDrawTransformFeedback;
	typedef void (FGL_APIENTRY gl_draw_transform_feedback_stream_func)(GLenum mode, GLuint id, GLuint stream);
	static gl_draw_transform_feedback_stream_func* glDrawTransformFeedbackStream;
	typedef void (FGL_APIENTRY gl_begin_query_indexed_func)(GLenum target, GLuint index, GLuint id);
	static gl_begin_query_indexed_func* glBeginQueryIndexed;
	typedef void (FGL_APIENTRY gl_end_query_indexed_func)(GLenum target, GLuint index);
	static gl_end_query_indexed_func* glEndQueryIndexed;
	typedef void (FGL_APIENTRY gl_get_query_indexediv_func)(GLenum target, GLuint index, GLenum pname, GLint *params);
	static gl_get_query_indexediv_func* glGetQueryIndexediv;
#	endif // GL_VERSION_4_0
#	ifndef GL_VERSION_4_1
#		define GL_VERSION_4_1 1
	static bool isGL_VERSION_4_1;
#	define GL_FIXED 0x140C
#	define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#	define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#	define GL_LOW_FLOAT 0x8DF0
#	define GL_MEDIUM_FLOAT 0x8DF1
#	define GL_HIGH_FLOAT 0x8DF2
#	define GL_LOW_INT 0x8DF3
#	define GL_MEDIUM_INT 0x8DF4
#	define GL_HIGH_INT 0x8DF5
#	define GL_SHADER_COMPILER 0x8DFA
#	define GL_SHADER_BINARY_FORMATS 0x8DF8
#	define GL_NUM_SHADER_BINARY_FORMATS 0x8DF9
#	define GL_MAX_VERTEX_UNIFORM_VECTORS 0x8DFB
#	define GL_MAX_VARYING_VECTORS 0x8DFC
#	define GL_MAX_FRAGMENT_UNIFORM_VECTORS 0x8DFD
#	define GL_RGB565 0x8D62
#	define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#	define GL_PROGRAM_BINARY_LENGTH 0x8741
#	define GL_NUM_PROGRAM_BINARY_FORMATS 0x87FE
#	define GL_PROGRAM_BINARY_FORMATS 0x87FF
#	define GL_VERTEX_SHADER_BIT 0x00000001
#	define GL_FRAGMENT_SHADER_BIT 0x00000002
#	define GL_GEOMETRY_SHADER_BIT 0x00000004
#	define GL_TESS_CONTROL_SHADER_BIT 0x00000008
#	define GL_TESS_EVALUATION_SHADER_BIT 0x00000010
#	define GL_ALL_SHADER_BITS 0xFFFFFFFF
#	define GL_PROGRAM_SEPARABLE 0x8258
#	define GL_ACTIVE_PROGRAM 0x8259
#	define GL_PROGRAM_PIPELINE_BINDING 0x825A
#	define GL_MAX_VIEWPORTS 0x825B
#	define GL_VIEWPORT_SUBPIXEL_BITS 0x825C
#	define GL_VIEWPORT_BOUNDS_RANGE 0x825D
#	define GL_LAYER_PROVOKING_VERTEX 0x825E
#	define GL_VIEWPORT_INDEX_PROVOKING_VERTEX 0x825F
#	define GL_UNDEFINED_VERTEX 0x8260
	typedef void (FGL_APIENTRY gl_release_shader_compiler_func)(void);
	static gl_release_shader_compiler_func* glReleaseShaderCompiler;
	typedef void (FGL_APIENTRY gl_shader_binary_func)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
	static gl_shader_binary_func* glShaderBinary;
	typedef void (FGL_APIENTRY gl_get_shader_precision_format_func)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
	static gl_get_shader_precision_format_func* glGetShaderPrecisionFormat;
	typedef void (FGL_APIENTRY gl_depth_rangef_func)(GLfloat n, GLfloat f);
	static gl_depth_rangef_func* glDepthRangef;
	typedef void (FGL_APIENTRY gl_clear_depthf_func)(GLfloat d);
	static gl_clear_depthf_func* glClearDepthf;
	typedef void (FGL_APIENTRY gl_get_program_binary_func)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
	static gl_get_program_binary_func* glGetProgramBinary;
	typedef void (FGL_APIENTRY gl_program_binary_func)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
	static gl_program_binary_func* glProgramBinary;
	typedef void (FGL_APIENTRY gl_program_parameteri_func)(GLuint program, GLenum pname, GLint value);
	static gl_program_parameteri_func* glProgramParameteri;
	typedef void (FGL_APIENTRY gl_use_program_stages_func)(GLuint pipeline, GLbitfield stages, GLuint program);
	static gl_use_program_stages_func* glUseProgramStages;
	typedef void (FGL_APIENTRY gl_active_shader_program_func)(GLuint pipeline, GLuint program);
	static gl_active_shader_program_func* glActiveShaderProgram;
	typedef GLuint(FGL_APIENTRY gl_create_shader_programv_func)(GLenum type, GLsizei count, const GLchar *const*strings);
	static gl_create_shader_programv_func* glCreateShaderProgramv;
	typedef void (FGL_APIENTRY gl_bind_program_pipeline_func)(GLuint pipeline);
	static gl_bind_program_pipeline_func* glBindProgramPipeline;
	typedef void (FGL_APIENTRY gl_delete_program_pipelines_func)(GLsizei n, const GLuint *pipelines);
	static gl_delete_program_pipelines_func* glDeleteProgramPipelines;
	typedef void (FGL_APIENTRY gl_gen_program_pipelines_func)(GLsizei n, GLuint *pipelines);
	static gl_gen_program_pipelines_func* glGenProgramPipelines;
	typedef GLboolean(FGL_APIENTRY gl_is_program_pipeline_func)(GLuint pipeline);
	static gl_is_program_pipeline_func* glIsProgramPipeline;
	typedef void (FGL_APIENTRY gl_get_program_pipelineiv_func)(GLuint pipeline, GLenum pname, GLint *params);
	static gl_get_program_pipelineiv_func* glGetProgramPipelineiv;
	typedef void (FGL_APIENTRY gl_program_uniform1i_func)(GLuint program, GLint location, GLint v0);
	static gl_program_uniform1i_func* glProgramUniform1i;
	typedef void (FGL_APIENTRY gl_program_uniform1iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform1iv_func* glProgramUniform1iv;
	typedef void (FGL_APIENTRY gl_program_uniform1f_func)(GLuint program, GLint location, GLfloat v0);
	static gl_program_uniform1f_func* glProgramUniform1f;
	typedef void (FGL_APIENTRY gl_program_uniform1fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform1fv_func* glProgramUniform1fv;
	typedef void (FGL_APIENTRY gl_program_uniform1d_func)(GLuint program, GLint location, GLdouble v0);
	static gl_program_uniform1d_func* glProgramUniform1d;
	typedef void (FGL_APIENTRY gl_program_uniform1dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform1dv_func* glProgramUniform1dv;
	typedef void (FGL_APIENTRY gl_program_uniform1ui_func)(GLuint program, GLint location, GLuint v0);
	static gl_program_uniform1ui_func* glProgramUniform1ui;
	typedef void (FGL_APIENTRY gl_program_uniform1uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform1uiv_func* glProgramUniform1uiv;
	typedef void (FGL_APIENTRY gl_program_uniform2i_func)(GLuint program, GLint location, GLint v0, GLint v1);
	static gl_program_uniform2i_func* glProgramUniform2i;
	typedef void (FGL_APIENTRY gl_program_uniform2iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform2iv_func* glProgramUniform2iv;
	typedef void (FGL_APIENTRY gl_program_uniform2f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
	static gl_program_uniform2f_func* glProgramUniform2f;
	typedef void (FGL_APIENTRY gl_program_uniform2fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform2fv_func* glProgramUniform2fv;
	typedef void (FGL_APIENTRY gl_program_uniform2d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
	static gl_program_uniform2d_func* glProgramUniform2d;
	typedef void (FGL_APIENTRY gl_program_uniform2dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform2dv_func* glProgramUniform2dv;
	typedef void (FGL_APIENTRY gl_program_uniform2ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1);
	static gl_program_uniform2ui_func* glProgramUniform2ui;
	typedef void (FGL_APIENTRY gl_program_uniform2uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform2uiv_func* glProgramUniform2uiv;
	typedef void (FGL_APIENTRY gl_program_uniform3i_func)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
	static gl_program_uniform3i_func* glProgramUniform3i;
	typedef void (FGL_APIENTRY gl_program_uniform3iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform3iv_func* glProgramUniform3iv;
	typedef void (FGL_APIENTRY gl_program_uniform3f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static gl_program_uniform3f_func* glProgramUniform3f;
	typedef void (FGL_APIENTRY gl_program_uniform3fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform3fv_func* glProgramUniform3fv;
	typedef void (FGL_APIENTRY gl_program_uniform3d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
	static gl_program_uniform3d_func* glProgramUniform3d;
	typedef void (FGL_APIENTRY gl_program_uniform3dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform3dv_func* glProgramUniform3dv;
	typedef void (FGL_APIENTRY gl_program_uniform3ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
	static gl_program_uniform3ui_func* glProgramUniform3ui;
	typedef void (FGL_APIENTRY gl_program_uniform3uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform3uiv_func* glProgramUniform3uiv;
	typedef void (FGL_APIENTRY gl_program_uniform4i_func)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static gl_program_uniform4i_func* glProgramUniform4i;
	typedef void (FGL_APIENTRY gl_program_uniform4iv_func)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static gl_program_uniform4iv_func* glProgramUniform4iv;
	typedef void (FGL_APIENTRY gl_program_uniform4f_func)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static gl_program_uniform4f_func* glProgramUniform4f;
	typedef void (FGL_APIENTRY gl_program_uniform4fv_func)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static gl_program_uniform4fv_func* glProgramUniform4fv;
	typedef void (FGL_APIENTRY gl_program_uniform4d_func)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
	static gl_program_uniform4d_func* glProgramUniform4d;
	typedef void (FGL_APIENTRY gl_program_uniform4dv_func)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static gl_program_uniform4dv_func* glProgramUniform4dv;
	typedef void (FGL_APIENTRY gl_program_uniform4ui_func)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static gl_program_uniform4ui_func* glProgramUniform4ui;
	typedef void (FGL_APIENTRY gl_program_uniform4uiv_func)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static gl_program_uniform4uiv_func* glProgramUniform4uiv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2fv_func* glProgramUniformMatrix2fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3fv_func* glProgramUniformMatrix3fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4fv_func* glProgramUniformMatrix4fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2dv_func* glProgramUniformMatrix2dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3dv_func* glProgramUniformMatrix3dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4dv_func* glProgramUniformMatrix4dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2x3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2x3fv_func* glProgramUniformMatrix2x3fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3x2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3x2fv_func* glProgramUniformMatrix3x2fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2x4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix2x4fv_func* glProgramUniformMatrix2x4fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4x2fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4x2fv_func* glProgramUniformMatrix4x2fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3x4fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix3x4fv_func* glProgramUniformMatrix3x4fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4x3fv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static gl_program_uniform_matrix4x3fv_func* glProgramUniformMatrix4x3fv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2x3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2x3dv_func* glProgramUniformMatrix2x3dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3x2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3x2dv_func* glProgramUniformMatrix3x2dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix2x4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix2x4dv_func* glProgramUniformMatrix2x4dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4x2dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4x2dv_func* glProgramUniformMatrix4x2dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix3x4dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix3x4dv_func* glProgramUniformMatrix3x4dv;
	typedef void (FGL_APIENTRY gl_program_uniform_matrix4x3dv_func)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static gl_program_uniform_matrix4x3dv_func* glProgramUniformMatrix4x3dv;
	typedef void (FGL_APIENTRY gl_validate_program_pipeline_func)(GLuint pipeline);
	static gl_validate_program_pipeline_func* glValidateProgramPipeline;
	typedef void (FGL_APIENTRY gl_get_program_pipeline_info_log_func)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static gl_get_program_pipeline_info_log_func* glGetProgramPipelineInfoLog;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l1d_func)(GLuint index, GLdouble x);
	static gl_vertex_attrib_l1d_func* glVertexAttribL1d;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l2d_func)(GLuint index, GLdouble x, GLdouble y);
	static gl_vertex_attrib_l2d_func* glVertexAttribL2d;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l3d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static gl_vertex_attrib_l3d_func* glVertexAttribL3d;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l4d_func)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static gl_vertex_attrib_l4d_func* glVertexAttribL4d;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l1dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l1dv_func* glVertexAttribL1dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l2dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l2dv_func* glVertexAttribL2dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l3dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l3dv_func* glVertexAttribL3dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l4dv_func)(GLuint index, const GLdouble *v);
	static gl_vertex_attrib_l4dv_func* glVertexAttribL4dv;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l_pointer_func)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
	static gl_vertex_attrib_l_pointer_func* glVertexAttribLPointer;
	typedef void (FGL_APIENTRY gl_get_vertex_attrib_ldv_func)(GLuint index, GLenum pname, GLdouble *params);
	static gl_get_vertex_attrib_ldv_func* glGetVertexAttribLdv;
	typedef void (FGL_APIENTRY gl_viewport_arrayv_func)(GLuint first, GLsizei count, const GLfloat *v);
	static gl_viewport_arrayv_func* glViewportArrayv;
	typedef void (FGL_APIENTRY gl_viewport_indexedf_func)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	static gl_viewport_indexedf_func* glViewportIndexedf;
	typedef void (FGL_APIENTRY gl_viewport_indexedfv_func)(GLuint index, const GLfloat *v);
	static gl_viewport_indexedfv_func* glViewportIndexedfv;
	typedef void (FGL_APIENTRY gl_scissor_arrayv_func)(GLuint first, GLsizei count, const GLint *v);
	static gl_scissor_arrayv_func* glScissorArrayv;
	typedef void (FGL_APIENTRY gl_scissor_indexed_func)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
	static gl_scissor_indexed_func* glScissorIndexed;
	typedef void (FGL_APIENTRY gl_scissor_indexedv_func)(GLuint index, const GLint *v);
	static gl_scissor_indexedv_func* glScissorIndexedv;
	typedef void (FGL_APIENTRY gl_depth_range_arrayv_func)(GLuint first, GLsizei count, const GLdouble *v);
	static gl_depth_range_arrayv_func* glDepthRangeArrayv;
	typedef void (FGL_APIENTRY gl_depth_range_indexed_func)(GLuint index, GLdouble n, GLdouble f);
	static gl_depth_range_indexed_func* glDepthRangeIndexed;
	typedef void (FGL_APIENTRY gl_get_floati_v_func)(GLenum target, GLuint index, GLfloat *data);
	static gl_get_floati_v_func* glGetFloati_v;
	typedef void (FGL_APIENTRY gl_get_doublei_v_func)(GLenum target, GLuint index, GLdouble *data);
	static gl_get_doublei_v_func* glGetDoublei_v;
#	endif // GL_VERSION_4_1
#	ifndef GL_VERSION_4_2
#		define GL_VERSION_4_2 1
	static bool isGL_VERSION_4_2;
#	define GL_COPY_READ_BUFFER_BINDING 0x8F36
#	define GL_COPY_WRITE_BUFFER_BINDING 0x8F37
#	define GL_TRANSFORM_FEEDBACK_ACTIVE 0x8E24
#	define GL_TRANSFORM_FEEDBACK_PAUSED 0x8E23
#	define GL_UNPACK_COMPRESSED_BLOCK_WIDTH 0x9127
#	define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT 0x9128
#	define GL_UNPACK_COMPRESSED_BLOCK_DEPTH 0x9129
#	define GL_UNPACK_COMPRESSED_BLOCK_SIZE 0x912A
#	define GL_PACK_COMPRESSED_BLOCK_WIDTH 0x912B
#	define GL_PACK_COMPRESSED_BLOCK_HEIGHT 0x912C
#	define GL_PACK_COMPRESSED_BLOCK_DEPTH 0x912D
#	define GL_PACK_COMPRESSED_BLOCK_SIZE 0x912E
#	define GL_NUM_SAMPLE_COUNTS 0x9380
#	define GL_MIN_MAP_BUFFER_ALIGNMENT 0x90BC
#	define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#	define GL_ATOMIC_COUNTER_BUFFER_BINDING 0x92C1
#	define GL_ATOMIC_COUNTER_BUFFER_START 0x92C2
#	define GL_ATOMIC_COUNTER_BUFFER_SIZE 0x92C3
#	define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE 0x92C4
#	define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#	define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES 0x92C6
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER 0x92C7
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER 0x92CA
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER 0x92CB
#	define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#	define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#	define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#	define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#	define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#	define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#	define GL_MAX_VERTEX_ATOMIC_COUNTERS 0x92D2
#	define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#	define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#	define GL_MAX_GEOMETRY_ATOMIC_COUNTERS 0x92D5
#	define GL_MAX_FRAGMENT_ATOMIC_COUNTERS 0x92D6
#	define GL_MAX_COMBINED_ATOMIC_COUNTERS 0x92D7
#	define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#	define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#	define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS 0x92D9
#	define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX 0x92DA
#	define GL_UNSIGNED_INT_ATOMIC_COUNTER 0x92DB
#	define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#	define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#	define GL_UNIFORM_BARRIER_BIT 0x00000004
#	define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#	define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#	define GL_COMMAND_BARRIER_BIT 0x00000040
#	define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#	define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#	define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#	define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#	define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#	define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000
#	define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#	define GL_MAX_IMAGE_UNITS 0x8F38
#	define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS 0x8F39
#	define GL_IMAGE_BINDING_NAME 0x8F3A
#	define GL_IMAGE_BINDING_LEVEL 0x8F3B
#	define GL_IMAGE_BINDING_LAYERED 0x8F3C
#	define GL_IMAGE_BINDING_LAYER 0x8F3D
#	define GL_IMAGE_BINDING_ACCESS 0x8F3E
#	define GL_IMAGE_1D 0x904C
#	define GL_IMAGE_2D 0x904D
#	define GL_IMAGE_3D 0x904E
#	define GL_IMAGE_2D_RECT 0x904F
#	define GL_IMAGE_CUBE 0x9050
#	define GL_IMAGE_BUFFER 0x9051
#	define GL_IMAGE_1D_ARRAY 0x9052
#	define GL_IMAGE_2D_ARRAY 0x9053
#	define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#	define GL_IMAGE_2D_MULTISAMPLE 0x9055
#	define GL_IMAGE_2D_MULTISAMPLE_ARRAY 0x9056
#	define GL_INT_IMAGE_1D 0x9057
#	define GL_INT_IMAGE_2D 0x9058
#	define GL_INT_IMAGE_3D 0x9059
#	define GL_INT_IMAGE_2D_RECT 0x905A
#	define GL_INT_IMAGE_CUBE 0x905B
#	define GL_INT_IMAGE_BUFFER 0x905C
#	define GL_INT_IMAGE_1D_ARRAY 0x905D
#	define GL_INT_IMAGE_2D_ARRAY 0x905E
#	define GL_INT_IMAGE_CUBE_MAP_ARRAY 0x905F
#	define GL_INT_IMAGE_2D_MULTISAMPLE 0x9060
#	define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x9061
#	define GL_UNSIGNED_INT_IMAGE_1D 0x9062
#	define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#	define GL_UNSIGNED_INT_IMAGE_3D 0x9064
#	define GL_UNSIGNED_INT_IMAGE_2D_RECT 0x9065
#	define GL_UNSIGNED_INT_IMAGE_CUBE 0x9066
#	define GL_UNSIGNED_INT_IMAGE_BUFFER 0x9067
#	define GL_UNSIGNED_INT_IMAGE_1D_ARRAY 0x9068
#	define GL_UNSIGNED_INT_IMAGE_2D_ARRAY 0x9069
#	define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#	define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 0x906B
#	define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x906C
#	define GL_MAX_IMAGE_SAMPLES 0x906D
#	define GL_IMAGE_BINDING_FORMAT 0x906E
#	define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#	define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#	define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#	define GL_MAX_VERTEX_IMAGE_UNIFORMS 0x90CA
#	define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
#	define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
#	define GL_MAX_GEOMETRY_IMAGE_UNIFORMS 0x90CD
#	define GL_MAX_FRAGMENT_IMAGE_UNIFORMS 0x90CE
#	define GL_MAX_COMBINED_IMAGE_UNIFORMS 0x90CF
#	define GL_COMPRESSED_RGBA_BPTC_UNORM 0x8E8C
#	define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#	define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#	define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#	define GL_TEXTURE_IMMUTABLE_FORMAT 0x912F
	typedef void (FGL_APIENTRY gl_draw_arrays_instanced_base_instance_func)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
	static gl_draw_arrays_instanced_base_instance_func* glDrawArraysInstancedBaseInstance;
	typedef void (FGL_APIENTRY gl_draw_elements_instanced_base_instance_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
	static gl_draw_elements_instanced_base_instance_func* glDrawElementsInstancedBaseInstance;
	typedef void (FGL_APIENTRY gl_draw_elements_instanced_base_vertex_base_instance_func)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
	static gl_draw_elements_instanced_base_vertex_base_instance_func* glDrawElementsInstancedBaseVertexBaseInstance;
	typedef void (FGL_APIENTRY gl_get_internalformativ_func)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
	static gl_get_internalformativ_func* glGetInternalformativ;
	typedef void (FGL_APIENTRY gl_get_active_atomic_counter_bufferiv_func)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
	static gl_get_active_atomic_counter_bufferiv_func* glGetActiveAtomicCounterBufferiv;
	typedef void (FGL_APIENTRY gl_bind_image_texture_func)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	static gl_bind_image_texture_func* glBindImageTexture;
	typedef void (FGL_APIENTRY gl_memory_barrier_func)(GLbitfield barriers);
	static gl_memory_barrier_func* glMemoryBarrier;
	typedef void (FGL_APIENTRY gl_tex_storage1d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static gl_tex_storage1d_func* glTexStorage1D;
	typedef void (FGL_APIENTRY gl_tex_storage2d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_tex_storage2d_func* glTexStorage2D;
	typedef void (FGL_APIENTRY gl_tex_storage3d_func)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static gl_tex_storage3d_func* glTexStorage3D;
	typedef void (FGL_APIENTRY gl_draw_transform_feedback_instanced_func)(GLenum mode, GLuint id, GLsizei instancecount);
	static gl_draw_transform_feedback_instanced_func* glDrawTransformFeedbackInstanced;
	typedef void (FGL_APIENTRY gl_draw_transform_feedback_stream_instanced_func)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
	static gl_draw_transform_feedback_stream_instanced_func* glDrawTransformFeedbackStreamInstanced;
#	endif // GL_VERSION_4_2
#	ifndef GL_VERSION_4_3
#		define GL_VERSION_4_3 1
	static bool isGL_VERSION_4_3;
	typedef void (FGL_APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#	define GL_NUM_SHADING_LANGUAGE_VERSIONS 0x82E9
#	define GL_VERTEX_ATTRIB_ARRAY_LONG 0x874E
#	define GL_COMPRESSED_RGB8_ETC2 0x9274
#	define GL_COMPRESSED_SRGB8_ETC2 0x9275
#	define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#	define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#	define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#	define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#	define GL_COMPRESSED_R11_EAC 0x9270
#	define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#	define GL_COMPRESSED_RG11_EAC 0x9272
#	define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#	define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#	define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#	define GL_MAX_ELEMENT_INDEX 0x8D6B
#	define GL_COMPUTE_SHADER 0x91B9
#	define GL_MAX_COMPUTE_UNIFORM_BLOCKS 0x91BB
#	define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#	define GL_MAX_COMPUTE_IMAGE_UNIFORMS 0x91BD
#	define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#	define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#	define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#	define GL_MAX_COMPUTE_ATOMIC_COUNTERS 0x8265
#	define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#	define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#	define GL_MAX_COMPUTE_WORK_GROUP_COUNT 0x91BE
#	define GL_MAX_COMPUTE_WORK_GROUP_SIZE 0x91BF
#	define GL_COMPUTE_WORK_GROUP_SIZE 0x8267
#	define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#	define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#	define GL_DISPATCH_INDIRECT_BUFFER 0x90EE
#	define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#	define GL_COMPUTE_SHADER_BIT 0x00000020
#	define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#	define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#	define GL_DEBUG_CALLBACK_FUNCTION 0x8244
#	define GL_DEBUG_CALLBACK_USER_PARAM 0x8245
#	define GL_DEBUG_SOURCE_API 0x8246
#	define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#	define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#	define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#	define GL_DEBUG_SOURCE_APPLICATION 0x824A
#	define GL_DEBUG_SOURCE_OTHER 0x824B
#	define GL_DEBUG_TYPE_ERROR 0x824C
#	define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#	define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#	define GL_DEBUG_TYPE_PORTABILITY 0x824F
#	define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#	define GL_DEBUG_TYPE_OTHER 0x8251
#	define GL_MAX_DEBUG_MESSAGE_LENGTH 0x9143
#	define GL_MAX_DEBUG_LOGGED_MESSAGES 0x9144
#	define GL_DEBUG_LOGGED_MESSAGES 0x9145
#	define GL_DEBUG_SEVERITY_HIGH 0x9146
#	define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#	define GL_DEBUG_SEVERITY_LOW 0x9148
#	define GL_DEBUG_TYPE_MARKER 0x8268
#	define GL_DEBUG_TYPE_PUSH_GROUP 0x8269
#	define GL_DEBUG_TYPE_POP_GROUP 0x826A
#	define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#	define GL_MAX_DEBUG_GROUP_STACK_DEPTH 0x826C
#	define GL_DEBUG_GROUP_STACK_DEPTH 0x826D
#	define GL_BUFFER 0x82E0
#	define GL_SHADER 0x82E1
#	define GL_PROGRAM 0x82E2
#	define GL_QUERY 0x82E3
#	define GL_PROGRAM_PIPELINE 0x82E4
#	define GL_SAMPLER 0x82E6
#	define GL_MAX_LABEL_LENGTH 0x82E8
#	define GL_DEBUG_OUTPUT 0x92E0
#	define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#	define GL_MAX_UNIFORM_LOCATIONS 0x826E
#	define GL_FRAMEBUFFER_DEFAULT_WIDTH 0x9310
#	define GL_FRAMEBUFFER_DEFAULT_HEIGHT 0x9311
#	define GL_FRAMEBUFFER_DEFAULT_LAYERS 0x9312
#	define GL_FRAMEBUFFER_DEFAULT_SAMPLES 0x9313
#	define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#	define GL_MAX_FRAMEBUFFER_WIDTH 0x9315
#	define GL_MAX_FRAMEBUFFER_HEIGHT 0x9316
#	define GL_MAX_FRAMEBUFFER_LAYERS 0x9317
#	define GL_MAX_FRAMEBUFFER_SAMPLES 0x9318
#	define GL_INTERNALFORMAT_SUPPORTED 0x826F
#	define GL_INTERNALFORMAT_PREFERRED 0x8270
#	define GL_INTERNALFORMAT_RED_SIZE 0x8271
#	define GL_INTERNALFORMAT_GREEN_SIZE 0x8272
#	define GL_INTERNALFORMAT_BLUE_SIZE 0x8273
#	define GL_INTERNALFORMAT_ALPHA_SIZE 0x8274
#	define GL_INTERNALFORMAT_DEPTH_SIZE 0x8275
#	define GL_INTERNALFORMAT_STENCIL_SIZE 0x8276
#	define GL_INTERNALFORMAT_SHARED_SIZE 0x8277
#	define GL_INTERNALFORMAT_RED_TYPE 0x8278
#	define GL_INTERNALFORMAT_GREEN_TYPE 0x8279
#	define GL_INTERNALFORMAT_BLUE_TYPE 0x827A
#	define GL_INTERNALFORMAT_ALPHA_TYPE 0x827B
#	define GL_INTERNALFORMAT_DEPTH_TYPE 0x827C
#	define GL_INTERNALFORMAT_STENCIL_TYPE 0x827D
#	define GL_MAX_WIDTH 0x827E
#	define GL_MAX_HEIGHT 0x827F
#	define GL_MAX_DEPTH 0x8280
#	define GL_MAX_LAYERS 0x8281
#	define GL_MAX_COMBINED_DIMENSIONS 0x8282
#	define GL_COLOR_COMPONENTS 0x8283
#	define GL_DEPTH_COMPONENTS 0x8284
#	define GL_STENCIL_COMPONENTS 0x8285
#	define GL_COLOR_RENDERABLE 0x8286
#	define GL_DEPTH_RENDERABLE 0x8287
#	define GL_STENCIL_RENDERABLE 0x8288
#	define GL_FRAMEBUFFER_RENDERABLE 0x8289
#	define GL_FRAMEBUFFER_RENDERABLE_LAYERED 0x828A
#	define GL_FRAMEBUFFER_BLEND 0x828B
#	define GL_READ_PIXELS 0x828C
#	define GL_READ_PIXELS_FORMAT 0x828D
#	define GL_READ_PIXELS_TYPE 0x828E
#	define GL_TEXTURE_IMAGE_FORMAT 0x828F
#	define GL_TEXTURE_IMAGE_TYPE 0x8290
#	define GL_GET_TEXTURE_IMAGE_FORMAT 0x8291
#	define GL_GET_TEXTURE_IMAGE_TYPE 0x8292
#	define GL_MIPMAP 0x8293
#	define GL_MANUAL_GENERATE_MIPMAP 0x8294
#	define GL_AUTO_GENERATE_MIPMAP 0x8295
#	define GL_COLOR_ENCODING 0x8296
#	define GL_SRGB_READ 0x8297
#	define GL_SRGB_WRITE 0x8298
#	define GL_FILTER 0x829A
#	define GL_VERTEX_TEXTURE 0x829B
#	define GL_TESS_CONTROL_TEXTURE 0x829C
#	define GL_TESS_EVALUATION_TEXTURE 0x829D
#	define GL_GEOMETRY_TEXTURE 0x829E
#	define GL_FRAGMENT_TEXTURE 0x829F
#	define GL_COMPUTE_TEXTURE 0x82A0
#	define GL_TEXTURE_SHADOW 0x82A1
#	define GL_TEXTURE_GATHER 0x82A2
#	define GL_TEXTURE_GATHER_SHADOW 0x82A3
#	define GL_SHADER_IMAGE_LOAD 0x82A4
#	define GL_SHADER_IMAGE_STORE 0x82A5
#	define GL_SHADER_IMAGE_ATOMIC 0x82A6
#	define GL_IMAGE_TEXEL_SIZE 0x82A7
#	define GL_IMAGE_COMPATIBILITY_CLASS 0x82A8
#	define GL_IMAGE_PIXEL_FORMAT 0x82A9
#	define GL_IMAGE_PIXEL_TYPE 0x82AA
#	define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST 0x82AC
#	define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST 0x82AD
#	define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE 0x82AE
#	define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE 0x82AF
#	define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH 0x82B1
#	define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT 0x82B2
#	define GL_TEXTURE_COMPRESSED_BLOCK_SIZE 0x82B3
#	define GL_CLEAR_BUFFER 0x82B4
#	define GL_TEXTURE_VIEW 0x82B5
#	define GL_VIEW_COMPATIBILITY_CLASS 0x82B6
#	define GL_FULL_SUPPORT 0x82B7
#	define GL_CAVEAT_SUPPORT 0x82B8
#	define GL_IMAGE_CLASS_4_X_32 0x82B9
#	define GL_IMAGE_CLASS_2_X_32 0x82BA
#	define GL_IMAGE_CLASS_1_X_32 0x82BB
#	define GL_IMAGE_CLASS_4_X_16 0x82BC
#	define GL_IMAGE_CLASS_2_X_16 0x82BD
#	define GL_IMAGE_CLASS_1_X_16 0x82BE
#	define GL_IMAGE_CLASS_4_X_8 0x82BF
#	define GL_IMAGE_CLASS_2_X_8 0x82C0
#	define GL_IMAGE_CLASS_1_X_8 0x82C1
#	define GL_IMAGE_CLASS_11_11_10 0x82C2
#	define GL_IMAGE_CLASS_10_10_10_2 0x82C3
#	define GL_VIEW_CLASS_128_BITS 0x82C4
#	define GL_VIEW_CLASS_96_BITS 0x82C5
#	define GL_VIEW_CLASS_64_BITS 0x82C6
#	define GL_VIEW_CLASS_48_BITS 0x82C7
#	define GL_VIEW_CLASS_32_BITS 0x82C8
#	define GL_VIEW_CLASS_24_BITS 0x82C9
#	define GL_VIEW_CLASS_16_BITS 0x82CA
#	define GL_VIEW_CLASS_8_BITS 0x82CB
#	define GL_VIEW_CLASS_S3TC_DXT1_RGB 0x82CC
#	define GL_VIEW_CLASS_S3TC_DXT1_RGBA 0x82CD
#	define GL_VIEW_CLASS_S3TC_DXT3_RGBA 0x82CE
#	define GL_VIEW_CLASS_S3TC_DXT5_RGBA 0x82CF
#	define GL_VIEW_CLASS_RGTC1_RED 0x82D0
#	define GL_VIEW_CLASS_RGTC2_RG 0x82D1
#	define GL_VIEW_CLASS_BPTC_UNORM 0x82D2
#	define GL_VIEW_CLASS_BPTC_FLOAT 0x82D3
#	define GL_UNIFORM 0x92E1
#	define GL_UNIFORM_BLOCK 0x92E2
#	define GL_PROGRAM_INPUT 0x92E3
#	define GL_PROGRAM_OUTPUT 0x92E4
#	define GL_BUFFER_VARIABLE 0x92E5
#	define GL_SHADER_STORAGE_BLOCK 0x92E6
#	define GL_VERTEX_SUBROUTINE 0x92E8
#	define GL_TESS_CONTROL_SUBROUTINE 0x92E9
#	define GL_TESS_EVALUATION_SUBROUTINE 0x92EA
#	define GL_GEOMETRY_SUBROUTINE 0x92EB
#	define GL_FRAGMENT_SUBROUTINE 0x92EC
#	define GL_COMPUTE_SUBROUTINE 0x92ED
#	define GL_VERTEX_SUBROUTINE_UNIFORM 0x92EE
#	define GL_TESS_CONTROL_SUBROUTINE_UNIFORM 0x92EF
#	define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM 0x92F0
#	define GL_GEOMETRY_SUBROUTINE_UNIFORM 0x92F1
#	define GL_FRAGMENT_SUBROUTINE_UNIFORM 0x92F2
#	define GL_COMPUTE_SUBROUTINE_UNIFORM 0x92F3
#	define GL_TRANSFORM_FEEDBACK_VARYING 0x92F4
#	define GL_ACTIVE_RESOURCES 0x92F5
#	define GL_MAX_NAME_LENGTH 0x92F6
#	define GL_MAX_NUM_ACTIVE_VARIABLES 0x92F7
#	define GL_MAX_NUM_COMPATIBLE_SUBROUTINES 0x92F8
#	define GL_NAME_LENGTH 0x92F9
#	define GL_TYPE 0x92FA
#	define GL_ARRAY_SIZE 0x92FB
#	define GL_OFFSET 0x92FC
#	define GL_BLOCK_INDEX 0x92FD
#	define GL_ARRAY_STRIDE 0x92FE
#	define GL_MATRIX_STRIDE 0x92FF
#	define GL_IS_ROW_MAJOR 0x9300
#	define GL_ATOMIC_COUNTER_BUFFER_INDEX 0x9301
#	define GL_BUFFER_BINDING 0x9302
#	define GL_BUFFER_DATA_SIZE 0x9303
#	define GL_NUM_ACTIVE_VARIABLES 0x9304
#	define GL_ACTIVE_VARIABLES 0x9305
#	define GL_REFERENCED_BY_VERTEX_SHADER 0x9306
#	define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#	define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#	define GL_REFERENCED_BY_GEOMETRY_SHADER 0x9309
#	define GL_REFERENCED_BY_FRAGMENT_SHADER 0x930A
#	define GL_REFERENCED_BY_COMPUTE_SHADER 0x930B
#	define GL_TOP_LEVEL_ARRAY_SIZE 0x930C
#	define GL_TOP_LEVEL_ARRAY_STRIDE 0x930D
#	define GL_LOCATION 0x930E
#	define GL_LOCATION_INDEX 0x930F
#	define GL_IS_PER_PATCH 0x92E7
#	define GL_SHADER_STORAGE_BUFFER 0x90D2
#	define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#	define GL_SHADER_STORAGE_BUFFER_START 0x90D4
#	define GL_SHADER_STORAGE_BUFFER_SIZE 0x90D5
#	define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#	define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#	define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#	define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#	define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#	define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#	define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#	define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#	define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#	define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#	define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#	define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#	define GL_DEPTH_STENCIL_TEXTURE_MODE 0x90EA
#	define GL_TEXTURE_BUFFER_OFFSET 0x919D
#	define GL_TEXTURE_BUFFER_SIZE 0x919E
#	define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT 0x919F
#	define GL_TEXTURE_VIEW_MIN_LEVEL 0x82DB
#	define GL_TEXTURE_VIEW_NUM_LEVELS 0x82DC
#	define GL_TEXTURE_VIEW_MIN_LAYER 0x82DD
#	define GL_TEXTURE_VIEW_NUM_LAYERS 0x82DE
#	define GL_TEXTURE_IMMUTABLE_LEVELS 0x82DF
#	define GL_VERTEX_ATTRIB_BINDING 0x82D4
#	define GL_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D5
#	define GL_VERTEX_BINDING_DIVISOR 0x82D6
#	define GL_VERTEX_BINDING_OFFSET 0x82D7
#	define GL_VERTEX_BINDING_STRIDE 0x82D8
#	define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#	define GL_MAX_VERTEX_ATTRIB_BINDINGS 0x82DA
#	define GL_VERTEX_BINDING_BUFFER 0x8F4F
#	define GL_DISPLAY_LIST 0x82E7
	typedef void (FGL_APIENTRY gl_clear_buffer_data_func)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
	static gl_clear_buffer_data_func* glClearBufferData;
	typedef void (FGL_APIENTRY gl_clear_buffer_sub_data_func)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
	static gl_clear_buffer_sub_data_func* glClearBufferSubData;
	typedef void (FGL_APIENTRY gl_dispatch_compute_func)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	static gl_dispatch_compute_func* glDispatchCompute;
	typedef void (FGL_APIENTRY gl_dispatch_compute_indirect_func)(GLintptr indirect);
	static gl_dispatch_compute_indirect_func* glDispatchComputeIndirect;
	typedef void (FGL_APIENTRY gl_copy_image_sub_data_func)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
	static gl_copy_image_sub_data_func* glCopyImageSubData;
	typedef void (FGL_APIENTRY gl_framebuffer_parameteri_func)(GLenum target, GLenum pname, GLint param);
	static gl_framebuffer_parameteri_func* glFramebufferParameteri;
	typedef void (FGL_APIENTRY gl_get_framebuffer_parameteriv_func)(GLenum target, GLenum pname, GLint *params);
	static gl_get_framebuffer_parameteriv_func* glGetFramebufferParameteriv;
	typedef void (FGL_APIENTRY gl_get_internalformati64v_func)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
	static gl_get_internalformati64v_func* glGetInternalformati64v;
	typedef void (FGL_APIENTRY gl_invalidate_tex_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
	static gl_invalidate_tex_sub_image_func* glInvalidateTexSubImage;
	typedef void (FGL_APIENTRY gl_invalidate_tex_image_func)(GLuint texture, GLint level);
	static gl_invalidate_tex_image_func* glInvalidateTexImage;
	typedef void (FGL_APIENTRY gl_invalidate_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static gl_invalidate_buffer_sub_data_func* glInvalidateBufferSubData;
	typedef void (FGL_APIENTRY gl_invalidate_buffer_data_func)(GLuint buffer);
	static gl_invalidate_buffer_data_func* glInvalidateBufferData;
	typedef void (FGL_APIENTRY gl_invalidate_framebuffer_func)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
	static gl_invalidate_framebuffer_func* glInvalidateFramebuffer;
	typedef void (FGL_APIENTRY gl_invalidate_sub_framebuffer_func)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_invalidate_sub_framebuffer_func* glInvalidateSubFramebuffer;
	typedef void (FGL_APIENTRY gl_multi_draw_arrays_indirect_func)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
	static gl_multi_draw_arrays_indirect_func* glMultiDrawArraysIndirect;
	typedef void (FGL_APIENTRY gl_multi_draw_elements_indirect_func)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
	static gl_multi_draw_elements_indirect_func* glMultiDrawElementsIndirect;
	typedef void (FGL_APIENTRY gl_get_program_interfaceiv_func)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
	static gl_get_program_interfaceiv_func* glGetProgramInterfaceiv;
	typedef GLuint(FGL_APIENTRY gl_get_program_resource_index_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_index_func* glGetProgramResourceIndex;
	typedef void (FGL_APIENTRY gl_get_program_resource_name_func)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
	static gl_get_program_resource_name_func* glGetProgramResourceName;
	typedef void (FGL_APIENTRY gl_get_program_resourceiv_func)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
	static gl_get_program_resourceiv_func* glGetProgramResourceiv;
	typedef GLint(FGL_APIENTRY gl_get_program_resource_location_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_location_func* glGetProgramResourceLocation;
	typedef GLint(FGL_APIENTRY gl_get_program_resource_location_index_func)(GLuint program, GLenum programInterface, const GLchar *name);
	static gl_get_program_resource_location_index_func* glGetProgramResourceLocationIndex;
	typedef void (FGL_APIENTRY gl_shader_storage_block_binding_func)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
	static gl_shader_storage_block_binding_func* glShaderStorageBlockBinding;
	typedef void (FGL_APIENTRY gl_tex_buffer_range_func)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_tex_buffer_range_func* glTexBufferRange;
	typedef void (FGL_APIENTRY gl_tex_storage2_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_tex_storage2_d_multisample_func* glTexStorage2DMultisample;
	typedef void (FGL_APIENTRY gl_tex_storage3_d_multisample_func)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_tex_storage3_d_multisample_func* glTexStorage3DMultisample;
	typedef void (FGL_APIENTRY gl_texture_view_func)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
	static gl_texture_view_func* glTextureView;
	typedef void (FGL_APIENTRY gl_bind_vertex_buffer_func)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static gl_bind_vertex_buffer_func* glBindVertexBuffer;
	typedef void (FGL_APIENTRY gl_vertex_attrib_format_func)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static gl_vertex_attrib_format_func* glVertexAttribFormat;
	typedef void (FGL_APIENTRY gl_vertex_attrib_i_format_func)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_attrib_i_format_func* glVertexAttribIFormat;
	typedef void (FGL_APIENTRY gl_vertex_attrib_l_format_func)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_attrib_l_format_func* glVertexAttribLFormat;
	typedef void (FGL_APIENTRY gl_vertex_attrib_binding_func)(GLuint attribindex, GLuint bindingindex);
	static gl_vertex_attrib_binding_func* glVertexAttribBinding;
	typedef void (FGL_APIENTRY gl_vertex_binding_divisor_func)(GLuint bindingindex, GLuint divisor);
	static gl_vertex_binding_divisor_func* glVertexBindingDivisor;
	typedef void (FGL_APIENTRY gl_debug_message_control_func)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
	static gl_debug_message_control_func* glDebugMessageControl;
	typedef void (FGL_APIENTRY gl_debug_message_insert_func)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
	static gl_debug_message_insert_func* glDebugMessageInsert;
	typedef void (FGL_APIENTRY gl_debug_message_callback_func)(GLDEBUGPROC callback, const void *userParam);
	static gl_debug_message_callback_func* glDebugMessageCallback;
	typedef GLuint(FGL_APIENTRY gl_get_debug_message_log_func)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
	static gl_get_debug_message_log_func* glGetDebugMessageLog;
	typedef void (FGL_APIENTRY gl_push_debug_group_func)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
	static gl_push_debug_group_func* glPushDebugGroup;
	typedef void (FGL_APIENTRY gl_pop_debug_group_func)(void);
	static gl_pop_debug_group_func* glPopDebugGroup;
	typedef void (FGL_APIENTRY gl_object_label_func)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
	static gl_object_label_func* glObjectLabel;
	typedef void (FGL_APIENTRY gl_get_object_label_func)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
	static gl_get_object_label_func* glGetObjectLabel;
	typedef void (FGL_APIENTRY gl_object_ptr_label_func)(const void *ptr, GLsizei length, const GLchar *label);
	static gl_object_ptr_label_func* glObjectPtrLabel;
	typedef void (FGL_APIENTRY gl_get_object_ptr_label_func)(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
	static gl_get_object_ptr_label_func* glGetObjectPtrLabel;
#	endif // GL_VERSION_4_3
#	ifndef GL_VERSION_4_4
#		define GL_VERSION_4_4 1
	static bool isGL_VERSION_4_4;
#	define GL_MAX_VERTEX_ATTRIB_STRIDE 0x82E5
#	define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED 0x8221
#	define GL_TEXTURE_BUFFER_BINDING 0x8C2A
#	define GL_MAP_PERSISTENT_BIT 0x0040
#	define GL_MAP_COHERENT_BIT 0x0080
#	define GL_DYNAMIC_STORAGE_BIT 0x0100
#	define GL_CLIENT_STORAGE_BIT 0x0200
#	define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#	define GL_BUFFER_IMMUTABLE_STORAGE 0x821F
#	define GL_BUFFER_STORAGE_FLAGS 0x8220
#	define GL_CLEAR_TEXTURE 0x9365
#	define GL_LOCATION_COMPONENT 0x934A
#	define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX 0x934B
#	define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE 0x934C
#	define GL_QUERY_BUFFER 0x9192
#	define GL_QUERY_BUFFER_BARRIER_BIT 0x00008000
#	define GL_QUERY_BUFFER_BINDING 0x9193
#	define GL_QUERY_RESULT_NO_WAIT 0x9194
#	define GL_MIRROR_CLAMP_TO_EDGE 0x8743
	typedef void (FGL_APIENTRY gl_buffer_storage_func)(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
	static gl_buffer_storage_func* glBufferStorage;
	typedef void (FGL_APIENTRY gl_clear_tex_image_func)(GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
	static gl_clear_tex_image_func* glClearTexImage;
	typedef void (FGL_APIENTRY gl_clear_tex_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
	static gl_clear_tex_sub_image_func* glClearTexSubImage;
	typedef void (FGL_APIENTRY gl_bind_buffers_base_func)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
	static gl_bind_buffers_base_func* glBindBuffersBase;
	typedef void (FGL_APIENTRY gl_bind_buffers_range_func)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
	static gl_bind_buffers_range_func* glBindBuffersRange;
	typedef void (FGL_APIENTRY gl_bind_textures_func)(GLuint first, GLsizei count, const GLuint *textures);
	static gl_bind_textures_func* glBindTextures;
	typedef void (FGL_APIENTRY gl_bind_samplers_func)(GLuint first, GLsizei count, const GLuint *samplers);
	static gl_bind_samplers_func* glBindSamplers;
	typedef void (FGL_APIENTRY gl_bind_image_textures_func)(GLuint first, GLsizei count, const GLuint *textures);
	static gl_bind_image_textures_func* glBindImageTextures;
	typedef void (FGL_APIENTRY gl_bind_vertex_buffers_func)(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
	static gl_bind_vertex_buffers_func* glBindVertexBuffers;
#	endif // GL_VERSION_4_4
#	ifndef GL_VERSION_4_5
#		define GL_VERSION_4_5 1
	static bool isGL_VERSION_4_5;
#	define GL_CONTEXT_LOST 0x0507
#	define GL_NEGATIVE_ONE_TO_ONE 0x935E
#	define GL_ZERO_TO_ONE 0x935F
#	define GL_CLIP_ORIGIN 0x935C
#	define GL_CLIP_DEPTH_MODE 0x935D
#	define GL_QUERY_WAIT_INVERTED 0x8E17
#	define GL_QUERY_NO_WAIT_INVERTED 0x8E18
#	define GL_QUERY_BY_REGION_WAIT_INVERTED 0x8E19
#	define GL_QUERY_BY_REGION_NO_WAIT_INVERTED 0x8E1A
#	define GL_MAX_CULL_DISTANCES 0x82F9
#	define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES 0x82FA
#	define GL_TEXTURE_TARGET 0x1006
#	define GL_QUERY_TARGET 0x82EA
#	define GL_GUILTY_CONTEXT_RESET 0x8253
#	define GL_INNOCENT_CONTEXT_RESET 0x8254
#	define GL_UNKNOWN_CONTEXT_RESET 0x8255
#	define GL_RESET_NOTIFICATION_STRATEGY 0x8256
#	define GL_LOSE_CONTEXT_ON_RESET 0x8252
#	define GL_NO_RESET_NOTIFICATION 0x8261
#	define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#	define GL_CONTEXT_RELEASE_BEHAVIOR 0x82FB
#	define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82FC
	typedef void (FGL_APIENTRY gl_clip_control_func)(GLenum origin, GLenum depth);
	static gl_clip_control_func* glClipControl;
	typedef void (FGL_APIENTRY gl_create_transform_feedbacks_func)(GLsizei n, GLuint *ids);
	static gl_create_transform_feedbacks_func* glCreateTransformFeedbacks;
	typedef void (FGL_APIENTRY gl_transform_feedback_buffer_base_func)(GLuint xfb, GLuint index, GLuint buffer);
	static gl_transform_feedback_buffer_base_func* glTransformFeedbackBufferBase;
	typedef void (FGL_APIENTRY gl_transform_feedback_buffer_range_func)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_transform_feedback_buffer_range_func* glTransformFeedbackBufferRange;
	typedef void (FGL_APIENTRY gl_get_transform_feedbackiv_func)(GLuint xfb, GLenum pname, GLint *param);
	static gl_get_transform_feedbackiv_func* glGetTransformFeedbackiv;
	typedef void (FGL_APIENTRY gl_get_transform_feedbacki_v_func)(GLuint xfb, GLenum pname, GLuint index, GLint *param);
	static gl_get_transform_feedbacki_v_func* glGetTransformFeedbacki_v;
	typedef void (FGL_APIENTRY gl_get_transform_feedbacki64_v_func)(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
	static gl_get_transform_feedbacki64_v_func* glGetTransformFeedbacki64_v;
	typedef void (FGL_APIENTRY gl_create_buffers_func)(GLsizei n, GLuint *buffers);
	static gl_create_buffers_func* glCreateBuffers;
	typedef void (FGL_APIENTRY gl_named_buffer_storage_func)(GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
	static gl_named_buffer_storage_func* glNamedBufferStorage;
	typedef void (FGL_APIENTRY gl_named_buffer_data_func)(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
	static gl_named_buffer_data_func* glNamedBufferData;
	typedef void (FGL_APIENTRY gl_named_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
	static gl_named_buffer_sub_data_func* glNamedBufferSubData;
	typedef void (FGL_APIENTRY gl_copy_named_buffer_sub_data_func)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static gl_copy_named_buffer_sub_data_func* glCopyNamedBufferSubData;
	typedef void (FGL_APIENTRY gl_clear_named_buffer_data_func)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
	static gl_clear_named_buffer_data_func* glClearNamedBufferData;
	typedef void (FGL_APIENTRY gl_clear_named_buffer_sub_data_func)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
	static gl_clear_named_buffer_sub_data_func* glClearNamedBufferSubData;
	typedef void * (FGL_APIENTRY gl_map_named_buffer_func)(GLuint buffer, GLenum access);
	static gl_map_named_buffer_func* glMapNamedBuffer;
	typedef void * (FGL_APIENTRY gl_map_named_buffer_range_func)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static gl_map_named_buffer_range_func* glMapNamedBufferRange;
	typedef GLboolean(FGL_APIENTRY gl_unmap_named_buffer_func)(GLuint buffer);
	static gl_unmap_named_buffer_func* glUnmapNamedBuffer;
	typedef void (FGL_APIENTRY gl_flush_mapped_named_buffer_range_func)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static gl_flush_mapped_named_buffer_range_func* glFlushMappedNamedBufferRange;
	typedef void (FGL_APIENTRY gl_get_named_buffer_parameteriv_func)(GLuint buffer, GLenum pname, GLint *params);
	static gl_get_named_buffer_parameteriv_func* glGetNamedBufferParameteriv;
	typedef void (FGL_APIENTRY gl_get_named_buffer_parameteri64v_func)(GLuint buffer, GLenum pname, GLint64 *params);
	static gl_get_named_buffer_parameteri64v_func* glGetNamedBufferParameteri64v;
	typedef void (FGL_APIENTRY gl_get_named_buffer_pointerv_func)(GLuint buffer, GLenum pname, void **params);
	static gl_get_named_buffer_pointerv_func* glGetNamedBufferPointerv;
	typedef void (FGL_APIENTRY gl_get_named_buffer_sub_data_func)(GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
	static gl_get_named_buffer_sub_data_func* glGetNamedBufferSubData;
	typedef void (FGL_APIENTRY gl_create_framebuffers_func)(GLsizei n, GLuint *framebuffers);
	static gl_create_framebuffers_func* glCreateFramebuffers;
	typedef void (FGL_APIENTRY gl_named_framebuffer_renderbuffer_func)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static gl_named_framebuffer_renderbuffer_func* glNamedFramebufferRenderbuffer;
	typedef void (FGL_APIENTRY gl_named_framebuffer_parameteri_func)(GLuint framebuffer, GLenum pname, GLint param);
	static gl_named_framebuffer_parameteri_func* glNamedFramebufferParameteri;
	typedef void (FGL_APIENTRY gl_named_framebuffer_texture_func)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
	static gl_named_framebuffer_texture_func* glNamedFramebufferTexture;
	typedef void (FGL_APIENTRY gl_named_framebuffer_texture_layer_func)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static gl_named_framebuffer_texture_layer_func* glNamedFramebufferTextureLayer;
	typedef void (FGL_APIENTRY gl_named_framebuffer_draw_buffer_func)(GLuint framebuffer, GLenum buf);
	static gl_named_framebuffer_draw_buffer_func* glNamedFramebufferDrawBuffer;
	typedef void (FGL_APIENTRY gl_named_framebuffer_draw_buffers_func)(GLuint framebuffer, GLsizei n, const GLenum *bufs);
	static gl_named_framebuffer_draw_buffers_func* glNamedFramebufferDrawBuffers;
	typedef void (FGL_APIENTRY gl_named_framebuffer_read_buffer_func)(GLuint framebuffer, GLenum src);
	static gl_named_framebuffer_read_buffer_func* glNamedFramebufferReadBuffer;
	typedef void (FGL_APIENTRY gl_invalidate_named_framebuffer_data_func)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
	static gl_invalidate_named_framebuffer_data_func* glInvalidateNamedFramebufferData;
	typedef void (FGL_APIENTRY gl_invalidate_named_framebuffer_sub_data_func)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_invalidate_named_framebuffer_sub_data_func* glInvalidateNamedFramebufferSubData;
	typedef void (FGL_APIENTRY gl_clear_named_framebufferiv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
	static gl_clear_named_framebufferiv_func* glClearNamedFramebufferiv;
	typedef void (FGL_APIENTRY gl_clear_named_framebufferuiv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
	static gl_clear_named_framebufferuiv_func* glClearNamedFramebufferuiv;
	typedef void (FGL_APIENTRY gl_clear_named_framebufferfv_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static gl_clear_named_framebufferfv_func* glClearNamedFramebufferfv;
	typedef void (FGL_APIENTRY gl_clear_named_framebufferfi_func)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static gl_clear_named_framebufferfi_func* glClearNamedFramebufferfi;
	typedef void (FGL_APIENTRY gl_blit_named_framebuffer_func)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static gl_blit_named_framebuffer_func* glBlitNamedFramebuffer;
	typedef GLenum(FGL_APIENTRY gl_check_named_framebuffer_status_func)(GLuint framebuffer, GLenum target);
	static gl_check_named_framebuffer_status_func* glCheckNamedFramebufferStatus;
	typedef void (FGL_APIENTRY gl_get_named_framebuffer_parameteriv_func)(GLuint framebuffer, GLenum pname, GLint *param);
	static gl_get_named_framebuffer_parameteriv_func* glGetNamedFramebufferParameteriv;
	typedef void (FGL_APIENTRY gl_get_named_framebuffer_attachment_parameteriv_func)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
	static gl_get_named_framebuffer_attachment_parameteriv_func* glGetNamedFramebufferAttachmentParameteriv;
	typedef void (FGL_APIENTRY gl_create_renderbuffers_func)(GLsizei n, GLuint *renderbuffers);
	static gl_create_renderbuffers_func* glCreateRenderbuffers;
	typedef void (FGL_APIENTRY gl_named_renderbuffer_storage_func)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_named_renderbuffer_storage_func* glNamedRenderbufferStorage;
	typedef void (FGL_APIENTRY gl_named_renderbuffer_storage_multisample_func)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_named_renderbuffer_storage_multisample_func* glNamedRenderbufferStorageMultisample;
	typedef void (FGL_APIENTRY gl_get_named_renderbuffer_parameteriv_func)(GLuint renderbuffer, GLenum pname, GLint *params);
	static gl_get_named_renderbuffer_parameteriv_func* glGetNamedRenderbufferParameteriv;
	typedef void (FGL_APIENTRY gl_create_textures_func)(GLenum target, GLsizei n, GLuint *textures);
	static gl_create_textures_func* glCreateTextures;
	typedef void (FGL_APIENTRY gl_texture_buffer_func)(GLuint texture, GLenum internalformat, GLuint buffer);
	static gl_texture_buffer_func* glTextureBuffer;
	typedef void (FGL_APIENTRY gl_texture_buffer_range_func)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static gl_texture_buffer_range_func* glTextureBufferRange;
	typedef void (FGL_APIENTRY gl_texture_storage1d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
	static gl_texture_storage1d_func* glTextureStorage1D;
	typedef void (FGL_APIENTRY gl_texture_storage2d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static gl_texture_storage2d_func* glTextureStorage2D;
	typedef void (FGL_APIENTRY gl_texture_storage3d_func)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static gl_texture_storage3d_func* glTextureStorage3D;
	typedef void (FGL_APIENTRY gl_texture_storage2_d_multisample_func)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static gl_texture_storage2_d_multisample_func* glTextureStorage2DMultisample;
	typedef void (FGL_APIENTRY gl_texture_storage3_d_multisample_func)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static gl_texture_storage3_d_multisample_func* glTextureStorage3DMultisample;
	typedef void (FGL_APIENTRY gl_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image1d_func* glTextureSubImage1D;
	typedef void (FGL_APIENTRY gl_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image2d_func* glTextureSubImage2D;
	typedef void (FGL_APIENTRY gl_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
	static gl_texture_sub_image3d_func* glTextureSubImage3D;
	typedef void (FGL_APIENTRY gl_compressed_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image1d_func* glCompressedTextureSubImage1D;
	typedef void (FGL_APIENTRY gl_compressed_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image2d_func* glCompressedTextureSubImage2D;
	typedef void (FGL_APIENTRY gl_compressed_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
	static gl_compressed_texture_sub_image3d_func* glCompressedTextureSubImage3D;
	typedef void (FGL_APIENTRY gl_copy_texture_sub_image1d_func)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static gl_copy_texture_sub_image1d_func* glCopyTextureSubImage1D;
	typedef void (FGL_APIENTRY gl_copy_texture_sub_image2d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_texture_sub_image2d_func* glCopyTextureSubImage2D;
	typedef void (FGL_APIENTRY gl_copy_texture_sub_image3d_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static gl_copy_texture_sub_image3d_func* glCopyTextureSubImage3D;
	typedef void (FGL_APIENTRY gl_texture_parameterf_func)(GLuint texture, GLenum pname, GLfloat param);
	static gl_texture_parameterf_func* glTextureParameterf;
	typedef void (FGL_APIENTRY gl_texture_parameterfv_func)(GLuint texture, GLenum pname, const GLfloat *param);
	static gl_texture_parameterfv_func* glTextureParameterfv;
	typedef void (FGL_APIENTRY gl_texture_parameteri_func)(GLuint texture, GLenum pname, GLint param);
	static gl_texture_parameteri_func* glTextureParameteri;
	typedef void (FGL_APIENTRY gl_texture_parameter_iiv_func)(GLuint texture, GLenum pname, const GLint *params);
	static gl_texture_parameter_iiv_func* glTextureParameterIiv;
	typedef void (FGL_APIENTRY gl_texture_parameter_iuiv_func)(GLuint texture, GLenum pname, const GLuint *params);
	static gl_texture_parameter_iuiv_func* glTextureParameterIuiv;
	typedef void (FGL_APIENTRY gl_texture_parameteriv_func)(GLuint texture, GLenum pname, const GLint *param);
	static gl_texture_parameteriv_func* glTextureParameteriv;
	typedef void (FGL_APIENTRY gl_generate_texture_mipmap_func)(GLuint texture);
	static gl_generate_texture_mipmap_func* glGenerateTextureMipmap;
	typedef void (FGL_APIENTRY gl_bind_texture_unit_func)(GLuint unit, GLuint texture);
	static gl_bind_texture_unit_func* glBindTextureUnit;
	typedef void (FGL_APIENTRY gl_get_texture_image_func)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_get_texture_image_func* glGetTextureImage;
	typedef void (FGL_APIENTRY gl_get_compressed_texture_image_func)(GLuint texture, GLint level, GLsizei bufSize, void *pixels);
	static gl_get_compressed_texture_image_func* glGetCompressedTextureImage;
	typedef void (FGL_APIENTRY gl_get_texture_level_parameterfv_func)(GLuint texture, GLint level, GLenum pname, GLfloat *params);
	static gl_get_texture_level_parameterfv_func* glGetTextureLevelParameterfv;
	typedef void (FGL_APIENTRY gl_get_texture_level_parameteriv_func)(GLuint texture, GLint level, GLenum pname, GLint *params);
	static gl_get_texture_level_parameteriv_func* glGetTextureLevelParameteriv;
	typedef void (FGL_APIENTRY gl_get_texture_parameterfv_func)(GLuint texture, GLenum pname, GLfloat *params);
	static gl_get_texture_parameterfv_func* glGetTextureParameterfv;
	typedef void (FGL_APIENTRY gl_get_texture_parameter_iiv_func)(GLuint texture, GLenum pname, GLint *params);
	static gl_get_texture_parameter_iiv_func* glGetTextureParameterIiv;
	typedef void (FGL_APIENTRY gl_get_texture_parameter_iuiv_func)(GLuint texture, GLenum pname, GLuint *params);
	static gl_get_texture_parameter_iuiv_func* glGetTextureParameterIuiv;
	typedef void (FGL_APIENTRY gl_get_texture_parameteriv_func)(GLuint texture, GLenum pname, GLint *params);
	static gl_get_texture_parameteriv_func* glGetTextureParameteriv;
	typedef void (FGL_APIENTRY gl_create_vertex_arrays_func)(GLsizei n, GLuint *arrays);
	static gl_create_vertex_arrays_func* glCreateVertexArrays;
	typedef void (FGL_APIENTRY gl_disable_vertex_array_attrib_func)(GLuint vaobj, GLuint index);
	static gl_disable_vertex_array_attrib_func* glDisableVertexArrayAttrib;
	typedef void (FGL_APIENTRY gl_enable_vertex_array_attrib_func)(GLuint vaobj, GLuint index);
	static gl_enable_vertex_array_attrib_func* glEnableVertexArrayAttrib;
	typedef void (FGL_APIENTRY gl_vertex_array_element_buffer_func)(GLuint vaobj, GLuint buffer);
	static gl_vertex_array_element_buffer_func* glVertexArrayElementBuffer;
	typedef void (FGL_APIENTRY gl_vertex_array_vertex_buffer_func)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static gl_vertex_array_vertex_buffer_func* glVertexArrayVertexBuffer;
	typedef void (FGL_APIENTRY gl_vertex_array_vertex_buffers_func)(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
	static gl_vertex_array_vertex_buffers_func* glVertexArrayVertexBuffers;
	typedef void (FGL_APIENTRY gl_vertex_array_attrib_binding_func)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
	static gl_vertex_array_attrib_binding_func* glVertexArrayAttribBinding;
	typedef void (FGL_APIENTRY gl_vertex_array_attrib_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static gl_vertex_array_attrib_format_func* glVertexArrayAttribFormat;
	typedef void (FGL_APIENTRY gl_vertex_array_attrib_i_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_array_attrib_i_format_func* glVertexArrayAttribIFormat;
	typedef void (FGL_APIENTRY gl_vertex_array_attrib_l_format_func)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static gl_vertex_array_attrib_l_format_func* glVertexArrayAttribLFormat;
	typedef void (FGL_APIENTRY gl_vertex_array_binding_divisor_func)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
	static gl_vertex_array_binding_divisor_func* glVertexArrayBindingDivisor;
	typedef void (FGL_APIENTRY gl_get_vertex_arrayiv_func)(GLuint vaobj, GLenum pname, GLint *param);
	static gl_get_vertex_arrayiv_func* glGetVertexArrayiv;
	typedef void (FGL_APIENTRY gl_get_vertex_array_indexediv_func)(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
	static gl_get_vertex_array_indexediv_func* glGetVertexArrayIndexediv;
	typedef void (FGL_APIENTRY gl_get_vertex_array_indexed64iv_func)(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
	static gl_get_vertex_array_indexed64iv_func* glGetVertexArrayIndexed64iv;
	typedef void (FGL_APIENTRY gl_create_samplers_func)(GLsizei n, GLuint *samplers);
	static gl_create_samplers_func* glCreateSamplers;
	typedef void (FGL_APIENTRY gl_create_program_pipelines_func)(GLsizei n, GLuint *pipelines);
	static gl_create_program_pipelines_func* glCreateProgramPipelines;
	typedef void (FGL_APIENTRY gl_create_queries_func)(GLenum target, GLsizei n, GLuint *ids);
	static gl_create_queries_func* glCreateQueries;
	typedef void (FGL_APIENTRY gl_get_query_buffer_objecti64v_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objecti64v_func* glGetQueryBufferObjecti64v;
	typedef void (FGL_APIENTRY gl_get_query_buffer_objectiv_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectiv_func* glGetQueryBufferObjectiv;
	typedef void (FGL_APIENTRY gl_get_query_buffer_objectui64v_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectui64v_func* glGetQueryBufferObjectui64v;
	typedef void (FGL_APIENTRY gl_get_query_buffer_objectuiv_func)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static gl_get_query_buffer_objectuiv_func* glGetQueryBufferObjectuiv;
	typedef void (FGL_APIENTRY gl_memory_barrier_by_region_func)(GLbitfield barriers);
	static gl_memory_barrier_by_region_func* glMemoryBarrierByRegion;
	typedef void (FGL_APIENTRY gl_get_texture_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_get_texture_sub_image_func* glGetTextureSubImage;
	typedef void (FGL_APIENTRY gl_get_compressed_texture_sub_image_func)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
	static gl_get_compressed_texture_sub_image_func* glGetCompressedTextureSubImage;
	typedef GLenum(FGL_APIENTRY gl_get_graphics_reset_status_func)(void);
	static gl_get_graphics_reset_status_func* glGetGraphicsResetStatus;
	typedef void (FGL_APIENTRY gl_getn_compressed_tex_image_func)(GLenum target, GLint lod, GLsizei bufSize, void *pixels);
	static gl_getn_compressed_tex_image_func* glGetnCompressedTexImage;
	typedef void (FGL_APIENTRY gl_getn_tex_image_func)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
	static gl_getn_tex_image_func* glGetnTexImage;
	typedef void (FGL_APIENTRY gl_getn_uniformdv_func)(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
	static gl_getn_uniformdv_func* glGetnUniformdv;
	typedef void (FGL_APIENTRY gl_getn_uniformfv_func)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
	static gl_getn_uniformfv_func* glGetnUniformfv;
	typedef void (FGL_APIENTRY gl_getn_uniformiv_func)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
	static gl_getn_uniformiv_func* glGetnUniformiv;
	typedef void (FGL_APIENTRY gl_getn_uniformuiv_func)(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
	static gl_getn_uniformuiv_func* glGetnUniformuiv;
	typedef void (FGL_APIENTRY gl_readn_pixels_func)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
	static gl_readn_pixels_func* glReadnPixels;
	typedef void (FGL_APIENTRY gl_getn_mapdv_func)(GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);
	static gl_getn_mapdv_func* glGetnMapdv;
	typedef void (FGL_APIENTRY gl_getn_mapfv_func)(GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
	static gl_getn_mapfv_func* glGetnMapfv;
	typedef void (FGL_APIENTRY gl_getn_mapiv_func)(GLenum target, GLenum query, GLsizei bufSize, GLint *v);
	static gl_getn_mapiv_func* glGetnMapiv;
	typedef void (FGL_APIENTRY gl_getn_pixel_mapfv_func)(GLenum map, GLsizei bufSize, GLfloat *values);
	static gl_getn_pixel_mapfv_func* glGetnPixelMapfv;
	typedef void (FGL_APIENTRY gl_getn_pixel_mapuiv_func)(GLenum map, GLsizei bufSize, GLuint *values);
	static gl_getn_pixel_mapuiv_func* glGetnPixelMapuiv;
	typedef void (FGL_APIENTRY gl_getn_pixel_mapusv_func)(GLenum map, GLsizei bufSize, GLushort *values);
	static gl_getn_pixel_mapusv_func* glGetnPixelMapusv;
	typedef void (FGL_APIENTRY gl_getn_polygon_stipple_func)(GLsizei bufSize, GLubyte *pattern);
	static gl_getn_polygon_stipple_func* glGetnPolygonStipple;
	typedef void (FGL_APIENTRY gl_getn_color_table_func)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table);
	static gl_getn_color_table_func* glGetnColorTable;
	typedef void (FGL_APIENTRY gl_getn_convolution_filter_func)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image);
	static gl_getn_convolution_filter_func* glGetnConvolutionFilter;
	typedef void (FGL_APIENTRY gl_getn_separable_filter_func)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span);
	static gl_getn_separable_filter_func* glGetnSeparableFilter;
	typedef void (FGL_APIENTRY gl_getn_histogram_func)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
	static gl_getn_histogram_func* glGetnHistogram;
	typedef void (FGL_APIENTRY gl_getn_minmax_func)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values);
	static gl_getn_minmax_func* glGetnMinmax;
	typedef void (FGL_APIENTRY gl_texture_barrier_func)(void);
	static gl_texture_barrier_func* glTextureBarrier;
#	endif // GL_VERSION_4_5
#	ifndef GL_VERSION_4_6
#		define GL_VERSION_4_6 1
	static bool isGL_VERSION_4_6;
#	define GL_SHADER_BINARY_FORMAT_SPIR_V 0x9551
#	define GL_SPIR_V_BINARY 0x9552
#	define GL_PARAMETER_BUFFER 0x80EE
#	define GL_PARAMETER_BUFFER_BINDING 0x80EF
#	define GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#	define GL_VERTICES_SUBMITTED 0x82EE
#	define GL_PRIMITIVES_SUBMITTED 0x82EF
#	define GL_VERTEX_SHADER_INVOCATIONS 0x82F0
#	define GL_TESS_CONTROL_SHADER_PATCHES 0x82F1
#	define GL_TESS_EVALUATION_SHADER_INVOCATIONS 0x82F2
#	define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED 0x82F3
#	define GL_FRAGMENT_SHADER_INVOCATIONS 0x82F4
#	define GL_COMPUTE_SHADER_INVOCATIONS 0x82F5
#	define GL_CLIPPING_INPUT_PRIMITIVES 0x82F6
#	define GL_CLIPPING_OUTPUT_PRIMITIVES 0x82F7
#	define GL_POLYGON_OFFSET_CLAMP 0x8E1B
#	define GL_SPIR_V_EXTENSIONS 0x9553
#	define GL_NUM_SPIR_V_EXTENSIONS 0x9554
#	define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
#	define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
#	define GL_TRANSFORM_FEEDBACK_OVERFLOW 0x82EC
#	define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW 0x82ED
	typedef void (FGL_APIENTRY gl_specialize_shader_func)(GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
	static gl_specialize_shader_func* glSpecializeShader;
	typedef void (FGL_APIENTRY gl_multi_draw_arrays_indirect_count_func)(GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static gl_multi_draw_arrays_indirect_count_func* glMultiDrawArraysIndirectCount;
	typedef void (FGL_APIENTRY gl_multi_draw_elements_indirect_count_func)(GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static gl_multi_draw_elements_indirect_count_func* glMultiDrawElementsIndirectCount;
	typedef void (FGL_APIENTRY gl_polygon_offset_clamp_func)(GLfloat factor, GLfloat units, GLfloat clamp);
	static gl_polygon_offset_clamp_func* glPolygonOffsetClamp;
#	endif // GL_VERSION_4_6


#ifdef __cplusplus
}
#endif

#endif // FGL_INCLUDE_H

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FGL_IMPLEMENTATION) && !defined(FGL_IMPLEMENTED)
#	define FGL_IMPLEMENTED

#define FGL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#include <assert.h> // assert
#include <stdarg.h> // va_start, va_end
#include <stdio.h> // vsnprintf

static size_t fgl__GetStringLen(const char *str) {
	size_t result = 0;
	if(str != fgl_null) {
		while(*str++) {
			++result;
		}
	}
	return(result);
}

static void fgl__ClearMemory(void *mem, size_t size) {
	if(mem != fgl_null) {
		uint8_t *p = (uint8_t *)mem;
		while(size > 0) {
			*p++ = 0;
			--size;
		}
	}
}

static void fgl__SetLastError(const char *format, ...);

#if defined(FGL_PLATFORM_WIN32)
// User32.dll
#define FGL_FUNC_WIN32_USER32_ReleaseDC(name) int WINAPI name(HWND hWnd, HDC hDC)
typedef FGL_FUNC_WIN32_USER32_ReleaseDC(fgl_func_win32_user32_ReleaseDC);
#define FGL_FUNC_WIN32_USER32_GetDC(name) HDC WINAPI name(HWND hWnd)
typedef FGL_FUNC_WIN32_USER32_GetDC(fgl_func_win32_user32_GetDC);

// Gdi32.dll
#define FGL_FUNC_WIN32_GDI32_ChoosePixelFormat(name) int WINAPI name(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FGL_FUNC_WIN32_GDI32_ChoosePixelFormat(fgl_func_win32_gdi32_ChoosePixelFormat);
#define FGL_FUNC_WIN32_GDI32_SetPixelFormat(name) BOOL WINAPI name(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
typedef FGL_FUNC_WIN32_GDI32_SetPixelFormat(fgl_func_win32_gdi32_SetPixelFormat);
#define FGL_FUNC_WIN32_GDI32_DescribePixelFormat(name) int WINAPI name(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
typedef FGL_FUNC_WIN32_GDI32_DescribePixelFormat(fgl_func_win32_gdi32_DescribePixelFormat);
#define FGL_FUNC_WIN32_GDI32_SwapBuffers(name) BOOL WINAPI name(HDC)
typedef FGL_FUNC_WIN32_GDI32_SwapBuffers(fgl_func_win32_gdi32_SwapBuffers);

// OpenGL32.dll
#define FGL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#define FGL_GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define FGL_GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define FGL_GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#define FGL_GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define FGL_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002

#define FGL_WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define FGL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define FGL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define FGL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define FGL_WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define FGL_WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define FGL_WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define FGL_WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define FGL_WGL_CONTEXT_FLAGS_ARB 0x2094

#define FGL_WGL_DRAW_TO_WINDOW_ARB 0x2001
#define FGL_WGL_ACCELERATION_ARB 0x2003
#define FGL_WGL_SWAP_METHOD_ARB 0x2007
#define FGL_WGL_SUPPORT_OPENGL_ARB 0x2010
#define FGL_WGL_DOUBLE_BUFFER_ARB 0x2011
#define FGL_WGL_PIXEL_TYPE_ARB 0x2013
#define FGL_WGL_COLOR_BITS_ARB 0x2014
#define FGL_WGL_DEPTH_BITS_ARB 0x2022
#define FGL_WGL_STENCIL_BITS_ARB 0x2023
#define FGL_WGL_FULL_ACCELERATION_ARB 0x2027
#define FGL_WGL_SWAP_EXCHANGE_ARB 0x2028
#define FGL_WGL_TYPE_RGBA_ARB 0x202B

#define FGL_FUNC_WIN32_OPENGL32_wglMakeCurrent(name) BOOL WINAPI name(HDC deviceContext, HGLRC renderingContext)
typedef FGL_FUNC_WIN32_OPENGL32_wglMakeCurrent(fgl_func_win32_opengl32_wglMakeCurrent);
#define FGL_FUNC_WIN32_OPENGL32_wglGetProcAddress(name) PROC WINAPI name(LPCSTR procedure)
typedef FGL_FUNC_WIN32_OPENGL32_wglGetProcAddress(fgl_func_win32_opengl32_wglGetProcAddress);
#define FGL_FUNC_WIN32_OPENGL32_wglDeleteContext(name) BOOL WINAPI name(HGLRC renderingContext)
typedef FGL_FUNC_WIN32_OPENGL32_wglDeleteContext(fgl_func_win32_opengl32_wglDeleteContext);
#define FGL_FUNC_WIN32_OPENGL32_wglCreateContext(name) HGLRC WINAPI name(HDC deviceContext)
typedef FGL_FUNC_WIN32_OPENGL32_wglCreateContext(fgl_func_win32_opengl32_wglCreateContext);
#define FGL_FUNC_WIN32_OPENGL32_wglChoosePixelFormatARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
typedef FGL_FUNC_WIN32_OPENGL32_wglChoosePixelFormatARB(fgl_func_win32_opengl32_wglChoosePixelFormatARB);
#define FGL_FUNC_WIN32_OPENGL32_wglCreateContextAttribsARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
typedef FGL_FUNC_WIN32_OPENGL32_wglCreateContextAttribsARB(fgl_func_win32_opengl32_wglCreateContextAttribsARB);
#define FGL_FUNC_WIN32_OPENGL32_wglSwapIntervalEXT(name) BOOL WINAPI name(int interval)
typedef FGL_FUNC_WIN32_OPENGL32_wglSwapIntervalEXT(fgl_func_win32_opengl32_wglSwapIntervalEXT);

typedef struct fglWin32OpenGLApi {
	struct {
		HMODULE libraryHandle;
		fgl_func_win32_user32_GetDC *GetDC;
		fgl_func_win32_user32_ReleaseDC *ReleaseDC;
	} user32;

	struct {
		HMODULE libraryHandle;
		fgl_func_win32_gdi32_ChoosePixelFormat *ChoosePixelFormat;
		fgl_func_win32_gdi32_SetPixelFormat *SetPixelFormat;
		fgl_func_win32_gdi32_DescribePixelFormat *DescribePixelFormat;
		fgl_func_win32_gdi32_SwapBuffers *SwapBuffers;
	} gdi32;

	struct {
		HMODULE libraryHandle;
		fgl_func_win32_opengl32_wglMakeCurrent *wglMakeCurrent;
		fgl_func_win32_opengl32_wglGetProcAddress *wglGetProcAddress;
		fgl_func_win32_opengl32_wglDeleteContext *wglDeleteContext;
		fgl_func_win32_opengl32_wglCreateContext *wglCreateContext;
		fgl_func_win32_opengl32_wglChoosePixelFormatARB *wglChoosePixelFormatARB;
		fgl_func_win32_opengl32_wglCreateContextAttribsARB *wglCreateContextAttribsARB;
		fgl_func_win32_opengl32_wglSwapIntervalEXT *wglSwapIntervalEXT;
	} opengl32;
} fglWin32OpenGLApi;

static void fgl__Win32UnloadOpenGL(fglWin32OpenGLApi *api) {
	if(api->opengl32.libraryHandle != fgl_null) {
		FreeLibrary(api->opengl32.libraryHandle);
	}
	if(api->gdi32.libraryHandle != fgl_null) {
		FreeLibrary(api->gdi32.libraryHandle);
	}
	if(api->user32.libraryHandle != fgl_null) {
		FreeLibrary(api->user32.libraryHandle);
	}
	fgl__ClearMemory(api, sizeof(*api));
}

static bool fgl__Win32LoadOpenGL(fglWin32OpenGLApi *api) {
	// user.dll
	api->user32.libraryHandle = LoadLibraryA("user32.dll");
	if(api->user32.libraryHandle == fgl_null) {
		fgl__SetLastError("Failed loading win32 user32.dll!");
		return false;
	}
	api->user32.GetDC = (fgl_func_win32_user32_GetDC *)GetProcAddress(api->user32.libraryHandle, "GetDC");
	api->user32.ReleaseDC = (fgl_func_win32_user32_ReleaseDC *)GetProcAddress(api->user32.libraryHandle, "ReleaseDC");

	// gdi.dll
	api->gdi32.libraryHandle = LoadLibraryA("gdi32.dll");
	if(api->gdi32.libraryHandle == fgl_null) {
		fgl__SetLastError("Failed loading win32 gdi32.dll!");
		return false;
	}
	api->gdi32.ChoosePixelFormat = (fgl_func_win32_gdi32_ChoosePixelFormat *)GetProcAddress(api->gdi32.libraryHandle, "ChoosePixelFormat");
	api->gdi32.SetPixelFormat = (fgl_func_win32_gdi32_SetPixelFormat *)GetProcAddress(api->gdi32.libraryHandle, "SetPixelFormat");
	api->gdi32.DescribePixelFormat = (fgl_func_win32_gdi32_DescribePixelFormat *)GetProcAddress(api->gdi32.libraryHandle, "DescribePixelFormat");
	api->gdi32.SwapBuffers = (fgl_func_win32_gdi32_SwapBuffers *)GetProcAddress(api->gdi32.libraryHandle, "SwapBuffers");

	// opengl32.dll
	const char *win32LibraryNames[] = {
		"opengl32.dll",
	};
	HMODULE glLibraryHandle = fgl_null;
	for(int i = 0; i < FGL_ARRAYCOUNT(win32LibraryNames); ++i) {
		glLibraryHandle = LoadLibraryA(win32LibraryNames[i]);
		if(glLibraryHandle != fgl_null) {
			api->opengl32.wglGetProcAddress = (fgl_func_win32_opengl32_wglGetProcAddress *)GetProcAddress(glLibraryHandle, "wglGetProcAddress");
			api->opengl32.wglCreateContext = (fgl_func_win32_opengl32_wglCreateContext *)GetProcAddress(glLibraryHandle, "wglCreateContext");
			api->opengl32.wglDeleteContext = (fgl_func_win32_opengl32_wglDeleteContext *)GetProcAddress(glLibraryHandle, "wglDeleteContext");
			api->opengl32.wglMakeCurrent = (fgl_func_win32_opengl32_wglMakeCurrent *)GetProcAddress(glLibraryHandle, "wglMakeCurrent");
			break;
		}
	}
	if(glLibraryHandle == fgl_null) {
		fgl__SetLastError("Failed loading win32 opengl32.dll!");
		return false;
	}
	api->opengl32.libraryHandle = glLibraryHandle;
	return true;
}

static void fgl__Win32DestroyOpenGLContext(fglWin32OpenGLApi *api, fglOpenGLContext *context) {
	if(context->renderingContext.win32.renderingContext != fgl_null) {
		api->opengl32.wglMakeCurrent(fgl_null, fgl_null);
		api->opengl32.wglDeleteContext(context->renderingContext.win32.renderingContext);
		context->renderingContext.win32.renderingContext = fgl_null;
	}
	if(context->windowHandle.win32.requireToReleaseDC) {
		api->user32.ReleaseDC(context->windowHandle.win32.windowHandle, context->windowHandle.win32.deviceContext);
		context->windowHandle.win32.deviceContext = fgl_null;
		context->windowHandle.win32.requireToReleaseDC = false;
	}
}

static bool fgl__Win32CreateOpenGLContext(fglWin32OpenGLApi *api, const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext) {
	HDC deviceContext = contextCreationParams->windowHandle.win32.deviceContext;
	HWND handle = contextCreationParams->windowHandle.win32.windowHandle;
	bool requireToReleaseDC = false;
	if(deviceContext == fgl_null) {
		if(handle == fgl_null) {
			fgl__SetLastError("Missing win32 window handle in opengl context creation!");
			return false;
		}
		deviceContext = api->user32.GetDC(handle);
		requireToReleaseDC = true;
	}

	outContext->windowHandle.win32.deviceContext = deviceContext;
	outContext->windowHandle.win32.windowHandle = handle;
	outContext->windowHandle.win32.requireToReleaseDC = requireToReleaseDC;

	PIXELFORMATDESCRIPTOR pfd = FGL_ZERO_INIT;
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = api->gdi32.ChoosePixelFormat(deviceContext, &pfd);
	if(!pixelFormat) {
		fgl__SetLastError("Failed win32 choosing pixel format for device context '%p'!", deviceContext);
		fgl__Win32DestroyOpenGLContext(api, outContext);
		return false;
	}

	if(!api->gdi32.SetPixelFormat(deviceContext, pixelFormat, &pfd)) {
		fgl__SetLastError("Failed win32 setting pixel format '%d' for device context '%p'!", pixelFormat, deviceContext);
		fgl__Win32DestroyOpenGLContext(api, outContext);
		return false;
	}

	api->gdi32.DescribePixelFormat(deviceContext, pixelFormat, sizeof(pfd), &pfd);

	HGLRC legacyRenderingContext = api->opengl32.wglCreateContext(deviceContext);
	if(!legacyRenderingContext) {
		fgl__SetLastError("Failed win32 creating opengl legacy rendering context for device context '%p'!", deviceContext);
		fgl__Win32DestroyOpenGLContext(api, outContext);
		return false;
	}

	if(!api->opengl32.wglMakeCurrent(deviceContext, legacyRenderingContext)) {
		fgl__SetLastError("Failed win32 activating opengl legacy rendering context '%p' for device context '%p'!", legacyRenderingContext, deviceContext);
		fgl__Win32DestroyOpenGLContext(api, outContext);
		return false;
	}

	api->opengl32.wglChoosePixelFormatARB = (fgl_func_win32_opengl32_wglChoosePixelFormatARB *)api->opengl32.wglGetProcAddress("wglChoosePixelFormatARB");
	api->opengl32.wglCreateContextAttribsARB = (fgl_func_win32_opengl32_wglCreateContextAttribsARB *)api->opengl32.wglGetProcAddress("wglCreateContextAttribsARB");

	api->opengl32.wglMakeCurrent(fgl_null, fgl_null);

	HGLRC activeRenderingContext;
	if(contextCreationParams->profile != fglOpenGLProfileType_LegacyProfile) {
		// @NOTE(final): This is only available in OpenGL 3.0+

		if(!(contextCreationParams->majorVersion >= 3 && contextCreationParams->minorVersion >= 0)) {
			fgl__SetLastError("You have not specified the 'majorVersion' and 'minorVersion' in the Context Creation Params!");
			fgl__Win32DestroyOpenGLContext(api, outContext);
			return false;
		}
		if(api->opengl32.wglChoosePixelFormatARB == fgl_null) {
			fgl__SetLastError("wglChoosePixelFormatARB is not available, modern OpenGL is not available for your video card");
			fgl__Win32DestroyOpenGLContext(api, outContext);
			return false;
		}
		if(api->opengl32.wglCreateContextAttribsARB == fgl_null) {
			fgl__SetLastError("wglCreateContextAttribsARB is not available, modern OpenGL is not available for your video card");
			fgl__Win32DestroyOpenGLContext(api, outContext);
			return false;
		}

		int profile = 0;
		int flags = 0;
		if(contextCreationParams->profile == fglOpenGLProfileType_CoreProfile) {
			profile = FGL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		} else if(contextCreationParams->profile == fglOpenGLProfileType_CompabilityProfile) {
			profile = FGL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		} else {
			fgl__SetLastError("No opengl compability profile selected, please specific either fglOpenGLProfileType_CoreProfile or fglOpenGLProfileType_CompabilityProfile");
			fgl__Win32DestroyOpenGLContext(api, outContext);
			return false;
		}
		if(contextCreationParams->forwardCompability) {
			flags = FGL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		}

		int contextAttribIndex = 0;
		int contextAttribList[20 + 1] = FGL_ZERO_INIT;
		contextAttribList[contextAttribIndex++] = FGL_WGL_CONTEXT_MAJOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)contextCreationParams->majorVersion;
		contextAttribList[contextAttribIndex++] = FGL_WGL_CONTEXT_MINOR_VERSION_ARB;
		contextAttribList[contextAttribIndex++] = (int)contextCreationParams->minorVersion;
		contextAttribList[contextAttribIndex++] = FGL_WGL_CONTEXT_PROFILE_MASK_ARB;
		contextAttribList[contextAttribIndex++] = profile;
		if(flags > 0) {
			contextAttribList[contextAttribIndex++] = FGL_WGL_CONTEXT_FLAGS_ARB;
			contextAttribList[contextAttribIndex++] = flags;
		}

		// Create modern opengl rendering context
		HGLRC modernRenderingContext = api->opengl32.wglCreateContextAttribsARB(deviceContext, 0, contextAttribList);
		if(modernRenderingContext) {
			if(!api->opengl32.wglMakeCurrent(deviceContext, modernRenderingContext)) {
				fgl__SetLastError("Warning: Failed activating Modern OpenGL Rendering Context for version (%d.%d) and profile (%d) and DC '%x') -> Fallback to legacy context", contextCreationParams->majorVersion, contextCreationParams->minorVersion, contextCreationParams->profile, deviceContext);

				api->opengl32.wglDeleteContext(modernRenderingContext);
				modernRenderingContext = fgl_null;

				// Fallback to legacy context
				api->opengl32.wglMakeCurrent(deviceContext, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			} else {
				// Destroy legacy rendering context
				api->opengl32.wglDeleteContext(legacyRenderingContext);
				legacyRenderingContext = fgl_null;
				activeRenderingContext = modernRenderingContext;
			}
		} else {
			fgl__SetLastError("Warning: Failed creating Modern OpenGL Rendering Context for version (%d.%d) and profile (%d) and DC '%x') -> Fallback to legacy context", contextCreationParams->majorVersion, contextCreationParams->minorVersion, contextCreationParams->profile, deviceContext);

			// Fallback to legacy context
			api->opengl32.wglMakeCurrent(deviceContext, legacyRenderingContext);
			activeRenderingContext = legacyRenderingContext;
		}
	} else {
		// Caller wants legacy context
		api->opengl32.wglMakeCurrent(deviceContext, legacyRenderingContext);
		activeRenderingContext = legacyRenderingContext;
	}
	assert(activeRenderingContext != fgl_null);
	outContext->renderingContext.win32.renderingContext = activeRenderingContext;
	outContext->isValid = true;
	return true;
}
#elif defined(FGL_PLATFORM_POSIX)
#define FGL_FUNC_POSIX_GLX_glXGetProcAddress(name) void *name(const char *name)
typedef FGL_FUNC_POSIX_GLX_glXGetProcAddress(fgl_func_posix_glx_glXGetProcAddress);

typedef struct fglPosixOpenGLApi {
	void *libraryHandle;
	fgl_func_posix_glx_glXGetProcAddress *glXGetProcAddress;
} fglPosixOpenGLApi;

static void fgl__PosixUnloadOpenGL(fglPosixOpenGLApi *api) {
	if(api->libraryHandle != fgl_null) {
		dlclose(api->libraryHandle);
	}
	fgl__ClearMemory(api, sizeof(*api));
}

static bool fgl__PosixLoadOpenGL(fglPosixOpenGLApi *api) {
	const char *posixLibraryNames[] = {
		"libGL.so.1",
		"libGL.so",
	};
	void *glLibraryHandle = fgl_null;
	for(int i = 0; i < FGL_ARRAYCOUNT(posixLibraryNames); ++i) {
		glLibraryHandle = dlopen(posixLibraryNames[i], RTLD_NOW);
		if(glLibraryHandle != fgl_null) {
			api->glXGetProcAddress = (fgl_func_posix_glx_glXGetProcAddress *)dlsym(glLibraryHandle, "glXGetProcAddress");
			break;
		}
	}
	if(glLibraryHandle == fgl_null) {
		fgl__SetLastError("Failed loading posix libGL.so!");
		return false;
	}
	api->libraryHandle = glLibraryHandle;
	return(true);
}

static bool fgl__PosixCreateOpenGLContext(fglPosixOpenGLApi *api, const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext) {
	// @TODO(final): Implement POSIX/GLX context creation
	return false;
}

static void fgl__PosixDestroyOpenGLContext(fglPosixOpenGLApi *api, fglOpenGLContext *context) {
	// @TODO(final): Implement POSIX/GLX context destroying
}
#endif

typedef struct fglOpenGLState {
	union {
#		if defined(FGL_PLATFORM_WIN32)
		fglWin32OpenGLApi win32;
#		elif defined(FGL_PLATFORM_POSIX)
		fglPosixOpenGLApi posix;
#		endif
	};
	char lastError[256];
	bool isLoaded;
} fglOpenGLState;

static fglOpenGLState fgl__globalOpenGLState = FGL_ZERO_INIT;

static void fgl__SetLastError(const char *format, ...) {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	if(format != fgl_null) {
		va_list argList;
		va_start(argList, format);
		vsnprintf(state->lastError, FGL_ARRAYCOUNT(state->lastError), format, argList);
		va_end(argList);
	} else {
		fgl__ClearMemory(state->lastError, sizeof(state->lastError));
	}
}

static void *fgl__GetOpenGLProcAddress(const fglOpenGLState *state, const char *name) {
	assert(state != fgl_null);
	void *result = fgl_null;
#	if defined(FGL_PLATFORM_WIN32)
	result = (void *)GetProcAddress(state->win32.opengl32.libraryHandle, name);
	if(result == fgl_null) {
		result = (void *)state->win32.opengl32.wglGetProcAddress(name);
	}
#	elif defined(FGL_PLATFORM_POSIX)
	result = dlsym(state->posix.libraryHandle, name);
	if(result == fgl_null) {
		result = state->posix.glXGetProcAddress(name);
	}
#	endif
	return(result);
}

static void fgl__LoadOpenGLExtensions(const fglOpenGLState *state) {
	assert(state != fgl_null);
#	if GL_VERSION_1_1
	glAccum = (gl_accum_func *)fgl__GetOpenGLProcAddress(state, "glAccum");
	glAlphaFunc = (gl_alpha_func_func *)fgl__GetOpenGLProcAddress(state, "glAlphaFunc");
	glAreTexturesResident = (gl_are_textures_resident_func *)fgl__GetOpenGLProcAddress(state, "glAreTexturesResident");
	glArrayElement = (gl_array_element_func *)fgl__GetOpenGLProcAddress(state, "glArrayElement");
	glBegin = (gl_begin_func *)fgl__GetOpenGLProcAddress(state, "glBegin");
	glBindTexture = (gl_bind_texture_func *)fgl__GetOpenGLProcAddress(state, "glBindTexture");
	glBitmap = (gl_bitmap_func *)fgl__GetOpenGLProcAddress(state, "glBitmap");
	glBlendFunc = (gl_blend_func_func *)fgl__GetOpenGLProcAddress(state, "glBlendFunc");
	glCallList = (gl_call_list_func *)fgl__GetOpenGLProcAddress(state, "glCallList");
	glCallLists = (gl_call_lists_func *)fgl__GetOpenGLProcAddress(state, "glCallLists");
	glClear = (gl_clear_func *)fgl__GetOpenGLProcAddress(state, "glClear");
	glClearAccum = (gl_clear_accum_func *)fgl__GetOpenGLProcAddress(state, "glClearAccum");
	glClearColor = (gl_clear_color_func *)fgl__GetOpenGLProcAddress(state, "glClearColor");
	glClearDepth = (gl_clear_depth_func *)fgl__GetOpenGLProcAddress(state, "glClearDepth");
	glClearIndex = (gl_clear_index_func *)fgl__GetOpenGLProcAddress(state, "glClearIndex");
	glClearStencil = (gl_clear_stencil_func *)fgl__GetOpenGLProcAddress(state, "glClearStencil");
	glClipPlane = (gl_clip_plane_func *)fgl__GetOpenGLProcAddress(state, "glClipPlane");
	glColor3b = (gl_color3b_func *)fgl__GetOpenGLProcAddress(state, "glColor3b");
	glColor3bv = (gl_color3bv_func *)fgl__GetOpenGLProcAddress(state, "glColor3bv");
	glColor3d = (gl_color3d_func *)fgl__GetOpenGLProcAddress(state, "glColor3d");
	glColor3dv = (gl_color3dv_func *)fgl__GetOpenGLProcAddress(state, "glColor3dv");
	glColor3f = (gl_color3f_func *)fgl__GetOpenGLProcAddress(state, "glColor3f");
	glColor3fv = (gl_color3fv_func *)fgl__GetOpenGLProcAddress(state, "glColor3fv");
	glColor3i = (gl_color3i_func *)fgl__GetOpenGLProcAddress(state, "glColor3i");
	glColor3iv = (gl_color3iv_func *)fgl__GetOpenGLProcAddress(state, "glColor3iv");
	glColor3s = (gl_color3s_func *)fgl__GetOpenGLProcAddress(state, "glColor3s");
	glColor3sv = (gl_color3sv_func *)fgl__GetOpenGLProcAddress(state, "glColor3sv");
	glColor3ub = (gl_color3ub_func *)fgl__GetOpenGLProcAddress(state, "glColor3ub");
	glColor3ubv = (gl_color3ubv_func *)fgl__GetOpenGLProcAddress(state, "glColor3ubv");
	glColor3ui = (gl_color3ui_func *)fgl__GetOpenGLProcAddress(state, "glColor3ui");
	glColor3uiv = (gl_color3uiv_func *)fgl__GetOpenGLProcAddress(state, "glColor3uiv");
	glColor3us = (gl_color3us_func *)fgl__GetOpenGLProcAddress(state, "glColor3us");
	glColor3usv = (gl_color3usv_func *)fgl__GetOpenGLProcAddress(state, "glColor3usv");
	glColor4b = (gl_color4b_func *)fgl__GetOpenGLProcAddress(state, "glColor4b");
	glColor4bv = (gl_color4bv_func *)fgl__GetOpenGLProcAddress(state, "glColor4bv");
	glColor4d = (gl_color4d_func *)fgl__GetOpenGLProcAddress(state, "glColor4d");
	glColor4dv = (gl_color4dv_func *)fgl__GetOpenGLProcAddress(state, "glColor4dv");
	glColor4f = (gl_color4f_func *)fgl__GetOpenGLProcAddress(state, "glColor4f");
	glColor4fv = (gl_color4fv_func *)fgl__GetOpenGLProcAddress(state, "glColor4fv");
	glColor4i = (gl_color4i_func *)fgl__GetOpenGLProcAddress(state, "glColor4i");
	glColor4iv = (gl_color4iv_func *)fgl__GetOpenGLProcAddress(state, "glColor4iv");
	glColor4s = (gl_color4s_func *)fgl__GetOpenGLProcAddress(state, "glColor4s");
	glColor4sv = (gl_color4sv_func *)fgl__GetOpenGLProcAddress(state, "glColor4sv");
	glColor4ub = (gl_color4ub_func *)fgl__GetOpenGLProcAddress(state, "glColor4ub");
	glColor4ubv = (gl_color4ubv_func *)fgl__GetOpenGLProcAddress(state, "glColor4ubv");
	glColor4ui = (gl_color4ui_func *)fgl__GetOpenGLProcAddress(state, "glColor4ui");
	glColor4uiv = (gl_color4uiv_func *)fgl__GetOpenGLProcAddress(state, "glColor4uiv");
	glColor4us = (gl_color4us_func *)fgl__GetOpenGLProcAddress(state, "glColor4us");
	glColor4usv = (gl_color4usv_func *)fgl__GetOpenGLProcAddress(state, "glColor4usv");
	glColorMask = (gl_color_mask_func *)fgl__GetOpenGLProcAddress(state, "glColorMask");
	glColorMaterial = (gl_color_material_func *)fgl__GetOpenGLProcAddress(state, "glColorMaterial");
	glColorPointer = (gl_color_pointer_func *)fgl__GetOpenGLProcAddress(state, "glColorPointer");
	glCopyPixels = (gl_copy_pixels_func *)fgl__GetOpenGLProcAddress(state, "glCopyPixels");
	glCopyTexImage1D = (gl_copy_tex_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTexImage1D");
	glCopyTexImage2D = (gl_copy_tex_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTexImage2D");
	glCopyTexSubImage1D = (gl_copy_tex_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTexSubImage1D");
	glCopyTexSubImage2D = (gl_copy_tex_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTexSubImage2D");
	glCullFace = (gl_cull_face_func *)fgl__GetOpenGLProcAddress(state, "glCullFace");
	glDeleteLists = (gl_delete_lists_func *)fgl__GetOpenGLProcAddress(state, "glDeleteLists");
	glDeleteTextures = (gl_delete_textures_func *)fgl__GetOpenGLProcAddress(state, "glDeleteTextures");
	glDepthFunc = (gl_depth_func_func *)fgl__GetOpenGLProcAddress(state, "glDepthFunc");
	glDepthMask = (gl_depth_mask_func *)fgl__GetOpenGLProcAddress(state, "glDepthMask");
	glDepthRange = (gl_depth_range_func *)fgl__GetOpenGLProcAddress(state, "glDepthRange");
	glDisable = (gl_disable_func *)fgl__GetOpenGLProcAddress(state, "glDisable");
	glDisableClientState = (gl_disable_client_state_func *)fgl__GetOpenGLProcAddress(state, "glDisableClientState");
	glDrawArrays = (gl_draw_arrays_func *)fgl__GetOpenGLProcAddress(state, "glDrawArrays");
	glDrawBuffer = (gl_draw_buffer_func *)fgl__GetOpenGLProcAddress(state, "glDrawBuffer");
	glDrawElements = (gl_draw_elements_func *)fgl__GetOpenGLProcAddress(state, "glDrawElements");
	glDrawPixels = (gl_draw_pixels_func *)fgl__GetOpenGLProcAddress(state, "glDrawPixels");
	glEdgeFlag = (gl_edge_flag_func *)fgl__GetOpenGLProcAddress(state, "glEdgeFlag");
	glEdgeFlagPointer = (gl_edge_flag_pointer_func *)fgl__GetOpenGLProcAddress(state, "glEdgeFlagPointer");
	glEdgeFlagv = (gl_edge_flagv_func *)fgl__GetOpenGLProcAddress(state, "glEdgeFlagv");
	glEnable = (gl_enable_func *)fgl__GetOpenGLProcAddress(state, "glEnable");
	glEnableClientState = (gl_enable_client_state_func *)fgl__GetOpenGLProcAddress(state, "glEnableClientState");
	glEnd = (gl_end_func *)fgl__GetOpenGLProcAddress(state, "glEnd");
	glEndList = (gl_end_list_func *)fgl__GetOpenGLProcAddress(state, "glEndList");
	glEvalCoord1d = (gl_eval_coord1d_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord1d");
	glEvalCoord1dv = (gl_eval_coord1dv_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord1dv");
	glEvalCoord1f = (gl_eval_coord1f_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord1f");
	glEvalCoord1fv = (gl_eval_coord1fv_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord1fv");
	glEvalCoord2d = (gl_eval_coord2d_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord2d");
	glEvalCoord2dv = (gl_eval_coord2dv_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord2dv");
	glEvalCoord2f = (gl_eval_coord2f_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord2f");
	glEvalCoord2fv = (gl_eval_coord2fv_func *)fgl__GetOpenGLProcAddress(state, "glEvalCoord2fv");
	glEvalMesh1 = (gl_eval_mesh1_func *)fgl__GetOpenGLProcAddress(state, "glEvalMesh1");
	glEvalMesh2 = (gl_eval_mesh2_func *)fgl__GetOpenGLProcAddress(state, "glEvalMesh2");
	glEvalPoint1 = (gl_eval_point1_func *)fgl__GetOpenGLProcAddress(state, "glEvalPoint1");
	glEvalPoint2 = (gl_eval_point2_func *)fgl__GetOpenGLProcAddress(state, "glEvalPoint2");
	glFeedbackBuffer = (gl_feedback_buffer_func *)fgl__GetOpenGLProcAddress(state, "glFeedbackBuffer");
	glFinish = (gl_finish_func *)fgl__GetOpenGLProcAddress(state, "glFinish");
	glFlush = (gl_flush_func *)fgl__GetOpenGLProcAddress(state, "glFlush");
	glFogf = (gl_fogf_func *)fgl__GetOpenGLProcAddress(state, "glFogf");
	glFogfv = (gl_fogfv_func *)fgl__GetOpenGLProcAddress(state, "glFogfv");
	glFogi = (gl_fogi_func *)fgl__GetOpenGLProcAddress(state, "glFogi");
	glFogiv = (gl_fogiv_func *)fgl__GetOpenGLProcAddress(state, "glFogiv");
	glFrontFace = (gl_front_face_func *)fgl__GetOpenGLProcAddress(state, "glFrontFace");
	glFrustum = (gl_frustum_func *)fgl__GetOpenGLProcAddress(state, "glFrustum");
	glGenLists = (gl_gen_lists_func *)fgl__GetOpenGLProcAddress(state, "glGenLists");
	glGenTextures = (gl_gen_textures_func *)fgl__GetOpenGLProcAddress(state, "glGenTextures");
	glGetBooleanv = (gl_get_booleanv_func *)fgl__GetOpenGLProcAddress(state, "glGetBooleanv");
	glGetClipPlane = (gl_get_clip_plane_func *)fgl__GetOpenGLProcAddress(state, "glGetClipPlane");
	glGetDoublev = (gl_get_doublev_func *)fgl__GetOpenGLProcAddress(state, "glGetDoublev");
	glGetError = (gl_get_error_func *)fgl__GetOpenGLProcAddress(state, "glGetError");
	glGetFloatv = (gl_get_floatv_func *)fgl__GetOpenGLProcAddress(state, "glGetFloatv");
	glGetIntegerv = (gl_get_integerv_func *)fgl__GetOpenGLProcAddress(state, "glGetIntegerv");
	glGetLightfv = (gl_get_lightfv_func *)fgl__GetOpenGLProcAddress(state, "glGetLightfv");
	glGetLightiv = (gl_get_lightiv_func *)fgl__GetOpenGLProcAddress(state, "glGetLightiv");
	glGetMapdv = (gl_get_mapdv_func *)fgl__GetOpenGLProcAddress(state, "glGetMapdv");
	glGetMapfv = (gl_get_mapfv_func *)fgl__GetOpenGLProcAddress(state, "glGetMapfv");
	glGetMapiv = (gl_get_mapiv_func *)fgl__GetOpenGLProcAddress(state, "glGetMapiv");
	glGetMaterialfv = (gl_get_materialfv_func *)fgl__GetOpenGLProcAddress(state, "glGetMaterialfv");
	glGetMaterialiv = (gl_get_materialiv_func *)fgl__GetOpenGLProcAddress(state, "glGetMaterialiv");
	glGetPixelMapfv = (gl_get_pixel_mapfv_func *)fgl__GetOpenGLProcAddress(state, "glGetPixelMapfv");
	glGetPixelMapuiv = (gl_get_pixel_mapuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetPixelMapuiv");
	glGetPixelMapusv = (gl_get_pixel_mapusv_func *)fgl__GetOpenGLProcAddress(state, "glGetPixelMapusv");
	glGetPointerv = (gl_get_pointerv_func *)fgl__GetOpenGLProcAddress(state, "glGetPointerv");
	glGetPolygonStipple = (gl_get_polygon_stipple_func *)fgl__GetOpenGLProcAddress(state, "glGetPolygonStipple");
	glGetString = (gl_get_string_func *)fgl__GetOpenGLProcAddress(state, "glGetString");
	glGetTexEnvfv = (gl_get_tex_envfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexEnvfv");
	glGetTexEnviv = (gl_get_tex_enviv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexEnviv");
	glGetTexGendv = (gl_get_tex_gendv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexGendv");
	glGetTexGenfv = (gl_get_tex_genfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexGenfv");
	glGetTexGeniv = (gl_get_tex_geniv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexGeniv");
	glGetTexImage = (gl_get_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glGetTexImage");
	glGetTexLevelParameterfv = (gl_get_tex_level_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexLevelParameterfv");
	glGetTexLevelParameteriv = (gl_get_tex_level_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexLevelParameteriv");
	glGetTexParameterfv = (gl_get_tex_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexParameterfv");
	glGetTexParameteriv = (gl_get_tex_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexParameteriv");
	glHint = (gl_hint_func *)fgl__GetOpenGLProcAddress(state, "glHint");
	glIndexMask = (gl_index_mask_func *)fgl__GetOpenGLProcAddress(state, "glIndexMask");
	glIndexPointer = (gl_index_pointer_func *)fgl__GetOpenGLProcAddress(state, "glIndexPointer");
	glIndexd = (gl_indexd_func *)fgl__GetOpenGLProcAddress(state, "glIndexd");
	glIndexdv = (gl_indexdv_func *)fgl__GetOpenGLProcAddress(state, "glIndexdv");
	glIndexf = (gl_indexf_func *)fgl__GetOpenGLProcAddress(state, "glIndexf");
	glIndexfv = (gl_indexfv_func *)fgl__GetOpenGLProcAddress(state, "glIndexfv");
	glIndexi = (gl_indexi_func *)fgl__GetOpenGLProcAddress(state, "glIndexi");
	glIndexiv = (gl_indexiv_func *)fgl__GetOpenGLProcAddress(state, "glIndexiv");
	glIndexs = (gl_indexs_func *)fgl__GetOpenGLProcAddress(state, "glIndexs");
	glIndexsv = (gl_indexsv_func *)fgl__GetOpenGLProcAddress(state, "glIndexsv");
	glIndexub = (gl_indexub_func *)fgl__GetOpenGLProcAddress(state, "glIndexub");
	glIndexubv = (gl_indexubv_func *)fgl__GetOpenGLProcAddress(state, "glIndexubv");
	glInitNames = (gl_init_names_func *)fgl__GetOpenGLProcAddress(state, "glInitNames");
	glInterleavedArrays = (gl_interleaved_arrays_func *)fgl__GetOpenGLProcAddress(state, "glInterleavedArrays");
	glIsEnabled = (gl_is_enabled_func *)fgl__GetOpenGLProcAddress(state, "glIsEnabled");
	glIsList = (gl_is_list_func *)fgl__GetOpenGLProcAddress(state, "glIsList");
	glIsTexture = (gl_is_texture_func *)fgl__GetOpenGLProcAddress(state, "glIsTexture");
	glLightModelf = (gl_light_modelf_func *)fgl__GetOpenGLProcAddress(state, "glLightModelf");
	glLightModelfv = (gl_light_modelfv_func *)fgl__GetOpenGLProcAddress(state, "glLightModelfv");
	glLightModeli = (gl_light_modeli_func *)fgl__GetOpenGLProcAddress(state, "glLightModeli");
	glLightModeliv = (gl_light_modeliv_func *)fgl__GetOpenGLProcAddress(state, "glLightModeliv");
	glLightf = (gl_lightf_func *)fgl__GetOpenGLProcAddress(state, "glLightf");
	glLightfv = (gl_lightfv_func *)fgl__GetOpenGLProcAddress(state, "glLightfv");
	glLighti = (gl_lighti_func *)fgl__GetOpenGLProcAddress(state, "glLighti");
	glLightiv = (gl_lightiv_func *)fgl__GetOpenGLProcAddress(state, "glLightiv");
	glLineStipple = (gl_line_stipple_func *)fgl__GetOpenGLProcAddress(state, "glLineStipple");
	glLineWidth = (gl_line_width_func *)fgl__GetOpenGLProcAddress(state, "glLineWidth");
	glListBase = (gl_list_base_func *)fgl__GetOpenGLProcAddress(state, "glListBase");
	glLoadIdentity = (gl_load_identity_func *)fgl__GetOpenGLProcAddress(state, "glLoadIdentity");
	glLoadMatrixd = (gl_load_matrixd_func *)fgl__GetOpenGLProcAddress(state, "glLoadMatrixd");
	glLoadMatrixf = (gl_load_matrixf_func *)fgl__GetOpenGLProcAddress(state, "glLoadMatrixf");
	glLoadName = (gl_load_name_func *)fgl__GetOpenGLProcAddress(state, "glLoadName");
	glLogicOp = (gl_logic_op_func *)fgl__GetOpenGLProcAddress(state, "glLogicOp");
	glMap1d = (gl_map1d_func *)fgl__GetOpenGLProcAddress(state, "glMap1d");
	glMap1f = (gl_map1f_func *)fgl__GetOpenGLProcAddress(state, "glMap1f");
	glMap2d = (gl_map2d_func *)fgl__GetOpenGLProcAddress(state, "glMap2d");
	glMap2f = (gl_map2f_func *)fgl__GetOpenGLProcAddress(state, "glMap2f");
	glMapGrid1d = (gl_map_grid1d_func *)fgl__GetOpenGLProcAddress(state, "glMapGrid1d");
	glMapGrid1f = (gl_map_grid1f_func *)fgl__GetOpenGLProcAddress(state, "glMapGrid1f");
	glMapGrid2d = (gl_map_grid2d_func *)fgl__GetOpenGLProcAddress(state, "glMapGrid2d");
	glMapGrid2f = (gl_map_grid2f_func *)fgl__GetOpenGLProcAddress(state, "glMapGrid2f");
	glMaterialf = (gl_materialf_func *)fgl__GetOpenGLProcAddress(state, "glMaterialf");
	glMaterialfv = (gl_materialfv_func *)fgl__GetOpenGLProcAddress(state, "glMaterialfv");
	glMateriali = (gl_materiali_func *)fgl__GetOpenGLProcAddress(state, "glMateriali");
	glMaterialiv = (gl_materialiv_func *)fgl__GetOpenGLProcAddress(state, "glMaterialiv");
	glMatrixMode = (gl_matrix_mode_func *)fgl__GetOpenGLProcAddress(state, "glMatrixMode");
	glMultMatrixd = (gl_mult_matrixd_func *)fgl__GetOpenGLProcAddress(state, "glMultMatrixd");
	glMultMatrixf = (gl_mult_matrixf_func *)fgl__GetOpenGLProcAddress(state, "glMultMatrixf");
	glNewList = (gl_new_list_func *)fgl__GetOpenGLProcAddress(state, "glNewList");
	glNormal3b = (gl_normal3b_func *)fgl__GetOpenGLProcAddress(state, "glNormal3b");
	glNormal3bv = (gl_normal3bv_func *)fgl__GetOpenGLProcAddress(state, "glNormal3bv");
	glNormal3d = (gl_normal3d_func *)fgl__GetOpenGLProcAddress(state, "glNormal3d");
	glNormal3dv = (gl_normal3dv_func *)fgl__GetOpenGLProcAddress(state, "glNormal3dv");
	glNormal3f = (gl_normal3f_func *)fgl__GetOpenGLProcAddress(state, "glNormal3f");
	glNormal3fv = (gl_normal3fv_func *)fgl__GetOpenGLProcAddress(state, "glNormal3fv");
	glNormal3i = (gl_normal3i_func *)fgl__GetOpenGLProcAddress(state, "glNormal3i");
	glNormal3iv = (gl_normal3iv_func *)fgl__GetOpenGLProcAddress(state, "glNormal3iv");
	glNormal3s = (gl_normal3s_func *)fgl__GetOpenGLProcAddress(state, "glNormal3s");
	glNormal3sv = (gl_normal3sv_func *)fgl__GetOpenGLProcAddress(state, "glNormal3sv");
	glNormalPointer = (gl_normal_pointer_func *)fgl__GetOpenGLProcAddress(state, "glNormalPointer");
	glOrtho = (gl_ortho_func *)fgl__GetOpenGLProcAddress(state, "glOrtho");
	glPassThrough = (gl_pass_through_func *)fgl__GetOpenGLProcAddress(state, "glPassThrough");
	glPixelMapfv = (gl_pixel_mapfv_func *)fgl__GetOpenGLProcAddress(state, "glPixelMapfv");
	glPixelMapuiv = (gl_pixel_mapuiv_func *)fgl__GetOpenGLProcAddress(state, "glPixelMapuiv");
	glPixelMapusv = (gl_pixel_mapusv_func *)fgl__GetOpenGLProcAddress(state, "glPixelMapusv");
	glPixelStoref = (gl_pixel_storef_func *)fgl__GetOpenGLProcAddress(state, "glPixelStoref");
	glPixelStorei = (gl_pixel_storei_func *)fgl__GetOpenGLProcAddress(state, "glPixelStorei");
	glPixelTransferf = (gl_pixel_transferf_func *)fgl__GetOpenGLProcAddress(state, "glPixelTransferf");
	glPixelTransferi = (gl_pixel_transferi_func *)fgl__GetOpenGLProcAddress(state, "glPixelTransferi");
	glPixelZoom = (gl_pixel_zoom_func *)fgl__GetOpenGLProcAddress(state, "glPixelZoom");
	glPointSize = (gl_point_size_func *)fgl__GetOpenGLProcAddress(state, "glPointSize");
	glPolygonMode = (gl_polygon_mode_func *)fgl__GetOpenGLProcAddress(state, "glPolygonMode");
	glPolygonOffset = (gl_polygon_offset_func *)fgl__GetOpenGLProcAddress(state, "glPolygonOffset");
	glPolygonStipple = (gl_polygon_stipple_func *)fgl__GetOpenGLProcAddress(state, "glPolygonStipple");
	glPopAttrib = (gl_pop_attrib_func *)fgl__GetOpenGLProcAddress(state, "glPopAttrib");
	glPopClientAttrib = (gl_pop_client_attrib_func *)fgl__GetOpenGLProcAddress(state, "glPopClientAttrib");
	glPopMatrix = (gl_pop_matrix_func *)fgl__GetOpenGLProcAddress(state, "glPopMatrix");
	glPopName = (gl_pop_name_func *)fgl__GetOpenGLProcAddress(state, "glPopName");
	glPrioritizeTextures = (gl_prioritize_textures_func *)fgl__GetOpenGLProcAddress(state, "glPrioritizeTextures");
	glPushAttrib = (gl_push_attrib_func *)fgl__GetOpenGLProcAddress(state, "glPushAttrib");
	glPushClientAttrib = (gl_push_client_attrib_func *)fgl__GetOpenGLProcAddress(state, "glPushClientAttrib");
	glPushMatrix = (gl_push_matrix_func *)fgl__GetOpenGLProcAddress(state, "glPushMatrix");
	glPushName = (gl_push_name_func *)fgl__GetOpenGLProcAddress(state, "glPushName");
	glRasterPos2d = (gl_raster_pos2d_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2d");
	glRasterPos2dv = (gl_raster_pos2dv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2dv");
	glRasterPos2f = (gl_raster_pos2f_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2f");
	glRasterPos2fv = (gl_raster_pos2fv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2fv");
	glRasterPos2i = (gl_raster_pos2i_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2i");
	glRasterPos2iv = (gl_raster_pos2iv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2iv");
	glRasterPos2s = (gl_raster_pos2s_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2s");
	glRasterPos2sv = (gl_raster_pos2sv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos2sv");
	glRasterPos3d = (gl_raster_pos3d_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3d");
	glRasterPos3dv = (gl_raster_pos3dv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3dv");
	glRasterPos3f = (gl_raster_pos3f_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3f");
	glRasterPos3fv = (gl_raster_pos3fv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3fv");
	glRasterPos3i = (gl_raster_pos3i_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3i");
	glRasterPos3iv = (gl_raster_pos3iv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3iv");
	glRasterPos3s = (gl_raster_pos3s_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3s");
	glRasterPos3sv = (gl_raster_pos3sv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos3sv");
	glRasterPos4d = (gl_raster_pos4d_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4d");
	glRasterPos4dv = (gl_raster_pos4dv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4dv");
	glRasterPos4f = (gl_raster_pos4f_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4f");
	glRasterPos4fv = (gl_raster_pos4fv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4fv");
	glRasterPos4i = (gl_raster_pos4i_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4i");
	glRasterPos4iv = (gl_raster_pos4iv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4iv");
	glRasterPos4s = (gl_raster_pos4s_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4s");
	glRasterPos4sv = (gl_raster_pos4sv_func *)fgl__GetOpenGLProcAddress(state, "glRasterPos4sv");
	glReadBuffer = (gl_read_buffer_func *)fgl__GetOpenGLProcAddress(state, "glReadBuffer");
	glReadPixels = (gl_read_pixels_func *)fgl__GetOpenGLProcAddress(state, "glReadPixels");
	glRectd = (gl_rectd_func *)fgl__GetOpenGLProcAddress(state, "glRectd");
	glRectdv = (gl_rectdv_func *)fgl__GetOpenGLProcAddress(state, "glRectdv");
	glRectf = (gl_rectf_func *)fgl__GetOpenGLProcAddress(state, "glRectf");
	glRectfv = (gl_rectfv_func *)fgl__GetOpenGLProcAddress(state, "glRectfv");
	glRecti = (gl_recti_func *)fgl__GetOpenGLProcAddress(state, "glRecti");
	glRectiv = (gl_rectiv_func *)fgl__GetOpenGLProcAddress(state, "glRectiv");
	glRects = (gl_rects_func *)fgl__GetOpenGLProcAddress(state, "glRects");
	glRectsv = (gl_rectsv_func *)fgl__GetOpenGLProcAddress(state, "glRectsv");
	glRenderMode = (gl_render_mode_func *)fgl__GetOpenGLProcAddress(state, "glRenderMode");
	glRotated = (gl_rotated_func *)fgl__GetOpenGLProcAddress(state, "glRotated");
	glRotatef = (gl_rotatef_func *)fgl__GetOpenGLProcAddress(state, "glRotatef");
	glScaled = (gl_scaled_func *)fgl__GetOpenGLProcAddress(state, "glScaled");
	glScalef = (gl_scalef_func *)fgl__GetOpenGLProcAddress(state, "glScalef");
	glScissor = (gl_scissor_func *)fgl__GetOpenGLProcAddress(state, "glScissor");
	glSelectBuffer = (gl_select_buffer_func *)fgl__GetOpenGLProcAddress(state, "glSelectBuffer");
	glShadeModel = (gl_shade_model_func *)fgl__GetOpenGLProcAddress(state, "glShadeModel");
	glStencilFunc = (gl_stencil_func_func *)fgl__GetOpenGLProcAddress(state, "glStencilFunc");
	glStencilMask = (gl_stencil_mask_func *)fgl__GetOpenGLProcAddress(state, "glStencilMask");
	glStencilOp = (gl_stencil_op_func *)fgl__GetOpenGLProcAddress(state, "glStencilOp");
	glTexCoord1d = (gl_tex_coord1d_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1d");
	glTexCoord1dv = (gl_tex_coord1dv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1dv");
	glTexCoord1f = (gl_tex_coord1f_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1f");
	glTexCoord1fv = (gl_tex_coord1fv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1fv");
	glTexCoord1i = (gl_tex_coord1i_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1i");
	glTexCoord1iv = (gl_tex_coord1iv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1iv");
	glTexCoord1s = (gl_tex_coord1s_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1s");
	glTexCoord1sv = (gl_tex_coord1sv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord1sv");
	glTexCoord2d = (gl_tex_coord2d_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2d");
	glTexCoord2dv = (gl_tex_coord2dv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2dv");
	glTexCoord2f = (gl_tex_coord2f_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2f");
	glTexCoord2fv = (gl_tex_coord2fv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2fv");
	glTexCoord2i = (gl_tex_coord2i_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2i");
	glTexCoord2iv = (gl_tex_coord2iv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2iv");
	glTexCoord2s = (gl_tex_coord2s_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2s");
	glTexCoord2sv = (gl_tex_coord2sv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord2sv");
	glTexCoord3d = (gl_tex_coord3d_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3d");
	glTexCoord3dv = (gl_tex_coord3dv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3dv");
	glTexCoord3f = (gl_tex_coord3f_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3f");
	glTexCoord3fv = (gl_tex_coord3fv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3fv");
	glTexCoord3i = (gl_tex_coord3i_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3i");
	glTexCoord3iv = (gl_tex_coord3iv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3iv");
	glTexCoord3s = (gl_tex_coord3s_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3s");
	glTexCoord3sv = (gl_tex_coord3sv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord3sv");
	glTexCoord4d = (gl_tex_coord4d_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4d");
	glTexCoord4dv = (gl_tex_coord4dv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4dv");
	glTexCoord4f = (gl_tex_coord4f_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4f");
	glTexCoord4fv = (gl_tex_coord4fv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4fv");
	glTexCoord4i = (gl_tex_coord4i_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4i");
	glTexCoord4iv = (gl_tex_coord4iv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4iv");
	glTexCoord4s = (gl_tex_coord4s_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4s");
	glTexCoord4sv = (gl_tex_coord4sv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoord4sv");
	glTexCoordPointer = (gl_tex_coord_pointer_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordPointer");
	glTexEnvf = (gl_tex_envf_func *)fgl__GetOpenGLProcAddress(state, "glTexEnvf");
	glTexEnvfv = (gl_tex_envfv_func *)fgl__GetOpenGLProcAddress(state, "glTexEnvfv");
	glTexEnvi = (gl_tex_envi_func *)fgl__GetOpenGLProcAddress(state, "glTexEnvi");
	glTexEnviv = (gl_tex_enviv_func *)fgl__GetOpenGLProcAddress(state, "glTexEnviv");
	glTexGend = (gl_tex_gend_func *)fgl__GetOpenGLProcAddress(state, "glTexGend");
	glTexGendv = (gl_tex_gendv_func *)fgl__GetOpenGLProcAddress(state, "glTexGendv");
	glTexGenf = (gl_tex_genf_func *)fgl__GetOpenGLProcAddress(state, "glTexGenf");
	glTexGenfv = (gl_tex_genfv_func *)fgl__GetOpenGLProcAddress(state, "glTexGenfv");
	glTexGeni = (gl_tex_geni_func *)fgl__GetOpenGLProcAddress(state, "glTexGeni");
	glTexGeniv = (gl_tex_geniv_func *)fgl__GetOpenGLProcAddress(state, "glTexGeniv");
	glTexImage1D = (gl_tex_image1d_func *)fgl__GetOpenGLProcAddress(state, "glTexImage1D");
	glTexImage2D = (gl_tex_image2d_func *)fgl__GetOpenGLProcAddress(state, "glTexImage2D");
	glTexParameterf = (gl_tex_parameterf_func *)fgl__GetOpenGLProcAddress(state, "glTexParameterf");
	glTexParameterfv = (gl_tex_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glTexParameterfv");
	glTexParameteri = (gl_tex_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glTexParameteri");
	glTexParameteriv = (gl_tex_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glTexParameteriv");
	glTexSubImage1D = (gl_tex_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glTexSubImage1D");
	glTexSubImage2D = (gl_tex_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glTexSubImage2D");
	glTranslated = (gl_translated_func *)fgl__GetOpenGLProcAddress(state, "glTranslated");
	glTranslatef = (gl_translatef_func *)fgl__GetOpenGLProcAddress(state, "glTranslatef");
	glVertex2d = (gl_vertex2d_func *)fgl__GetOpenGLProcAddress(state, "glVertex2d");
	glVertex2dv = (gl_vertex2dv_func *)fgl__GetOpenGLProcAddress(state, "glVertex2dv");
	glVertex2f = (gl_vertex2f_func *)fgl__GetOpenGLProcAddress(state, "glVertex2f");
	glVertex2fv = (gl_vertex2fv_func *)fgl__GetOpenGLProcAddress(state, "glVertex2fv");
	glVertex2i = (gl_vertex2i_func *)fgl__GetOpenGLProcAddress(state, "glVertex2i");
	glVertex2iv = (gl_vertex2iv_func *)fgl__GetOpenGLProcAddress(state, "glVertex2iv");
	glVertex2s = (gl_vertex2s_func *)fgl__GetOpenGLProcAddress(state, "glVertex2s");
	glVertex2sv = (gl_vertex2sv_func *)fgl__GetOpenGLProcAddress(state, "glVertex2sv");
	glVertex3d = (gl_vertex3d_func *)fgl__GetOpenGLProcAddress(state, "glVertex3d");
	glVertex3dv = (gl_vertex3dv_func *)fgl__GetOpenGLProcAddress(state, "glVertex3dv");
	glVertex3f = (gl_vertex3f_func *)fgl__GetOpenGLProcAddress(state, "glVertex3f");
	glVertex3fv = (gl_vertex3fv_func *)fgl__GetOpenGLProcAddress(state, "glVertex3fv");
	glVertex3i = (gl_vertex3i_func *)fgl__GetOpenGLProcAddress(state, "glVertex3i");
	glVertex3iv = (gl_vertex3iv_func *)fgl__GetOpenGLProcAddress(state, "glVertex3iv");
	glVertex3s = (gl_vertex3s_func *)fgl__GetOpenGLProcAddress(state, "glVertex3s");
	glVertex3sv = (gl_vertex3sv_func *)fgl__GetOpenGLProcAddress(state, "glVertex3sv");
	glVertex4d = (gl_vertex4d_func *)fgl__GetOpenGLProcAddress(state, "glVertex4d");
	glVertex4dv = (gl_vertex4dv_func *)fgl__GetOpenGLProcAddress(state, "glVertex4dv");
	glVertex4f = (gl_vertex4f_func *)fgl__GetOpenGLProcAddress(state, "glVertex4f");
	glVertex4fv = (gl_vertex4fv_func *)fgl__GetOpenGLProcAddress(state, "glVertex4fv");
	glVertex4i = (gl_vertex4i_func *)fgl__GetOpenGLProcAddress(state, "glVertex4i");
	glVertex4iv = (gl_vertex4iv_func *)fgl__GetOpenGLProcAddress(state, "glVertex4iv");
	glVertex4s = (gl_vertex4s_func *)fgl__GetOpenGLProcAddress(state, "glVertex4s");
	glVertex4sv = (gl_vertex4sv_func *)fgl__GetOpenGLProcAddress(state, "glVertex4sv");
	glVertexPointer = (gl_vertex_pointer_func *)fgl__GetOpenGLProcAddress(state, "glVertexPointer");
	glViewport = (gl_viewport_func *)fgl__GetOpenGLProcAddress(state, "glViewport");
#	endif //GL_VERSION_1_1

#	if GL_VERSION_1_2
	glDrawRangeElements = (gl_draw_range_elements_func *)fgl__GetOpenGLProcAddress(state, "glDrawRangeElements");
	glTexImage3D = (gl_tex_image3d_func *)fgl__GetOpenGLProcAddress(state, "glTexImage3D");
	glTexSubImage3D = (gl_tex_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glTexSubImage3D");
	glCopyTexSubImage3D = (gl_copy_tex_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTexSubImage3D");
#	endif //GL_VERSION_1_2

#	if GL_VERSION_1_3
	glActiveTexture = (gl_active_texture_func *)fgl__GetOpenGLProcAddress(state, "glActiveTexture");
	glSampleCoverage = (gl_sample_coverage_func *)fgl__GetOpenGLProcAddress(state, "glSampleCoverage");
	glCompressedTexImage3D = (gl_compressed_tex_image3d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexImage3D");
	glCompressedTexImage2D = (gl_compressed_tex_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexImage2D");
	glCompressedTexImage1D = (gl_compressed_tex_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexImage1D");
	glCompressedTexSubImage3D = (gl_compressed_tex_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexSubImage3D");
	glCompressedTexSubImage2D = (gl_compressed_tex_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexSubImage2D");
	glCompressedTexSubImage1D = (gl_compressed_tex_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTexSubImage1D");
	glGetCompressedTexImage = (gl_get_compressed_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glGetCompressedTexImage");
	glClientActiveTexture = (gl_client_active_texture_func *)fgl__GetOpenGLProcAddress(state, "glClientActiveTexture");
	glMultiTexCoord1d = (gl_multi_tex_coord1d_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1d");
	glMultiTexCoord1dv = (gl_multi_tex_coord1dv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1dv");
	glMultiTexCoord1f = (gl_multi_tex_coord1f_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1f");
	glMultiTexCoord1fv = (gl_multi_tex_coord1fv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1fv");
	glMultiTexCoord1i = (gl_multi_tex_coord1i_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1i");
	glMultiTexCoord1iv = (gl_multi_tex_coord1iv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1iv");
	glMultiTexCoord1s = (gl_multi_tex_coord1s_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1s");
	glMultiTexCoord1sv = (gl_multi_tex_coord1sv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord1sv");
	glMultiTexCoord2d = (gl_multi_tex_coord2d_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2d");
	glMultiTexCoord2dv = (gl_multi_tex_coord2dv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2dv");
	glMultiTexCoord2f = (gl_multi_tex_coord2f_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2f");
	glMultiTexCoord2fv = (gl_multi_tex_coord2fv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2fv");
	glMultiTexCoord2i = (gl_multi_tex_coord2i_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2i");
	glMultiTexCoord2iv = (gl_multi_tex_coord2iv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2iv");
	glMultiTexCoord2s = (gl_multi_tex_coord2s_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2s");
	glMultiTexCoord2sv = (gl_multi_tex_coord2sv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord2sv");
	glMultiTexCoord3d = (gl_multi_tex_coord3d_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3d");
	glMultiTexCoord3dv = (gl_multi_tex_coord3dv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3dv");
	glMultiTexCoord3f = (gl_multi_tex_coord3f_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3f");
	glMultiTexCoord3fv = (gl_multi_tex_coord3fv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3fv");
	glMultiTexCoord3i = (gl_multi_tex_coord3i_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3i");
	glMultiTexCoord3iv = (gl_multi_tex_coord3iv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3iv");
	glMultiTexCoord3s = (gl_multi_tex_coord3s_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3s");
	glMultiTexCoord3sv = (gl_multi_tex_coord3sv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord3sv");
	glMultiTexCoord4d = (gl_multi_tex_coord4d_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4d");
	glMultiTexCoord4dv = (gl_multi_tex_coord4dv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4dv");
	glMultiTexCoord4f = (gl_multi_tex_coord4f_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4f");
	glMultiTexCoord4fv = (gl_multi_tex_coord4fv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4fv");
	glMultiTexCoord4i = (gl_multi_tex_coord4i_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4i");
	glMultiTexCoord4iv = (gl_multi_tex_coord4iv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4iv");
	glMultiTexCoord4s = (gl_multi_tex_coord4s_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4s");
	glMultiTexCoord4sv = (gl_multi_tex_coord4sv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoord4sv");
	glLoadTransposeMatrixf = (gl_load_transpose_matrixf_func *)fgl__GetOpenGLProcAddress(state, "glLoadTransposeMatrixf");
	glLoadTransposeMatrixd = (gl_load_transpose_matrixd_func *)fgl__GetOpenGLProcAddress(state, "glLoadTransposeMatrixd");
	glMultTransposeMatrixf = (gl_mult_transpose_matrixf_func *)fgl__GetOpenGLProcAddress(state, "glMultTransposeMatrixf");
	glMultTransposeMatrixd = (gl_mult_transpose_matrixd_func *)fgl__GetOpenGLProcAddress(state, "glMultTransposeMatrixd");
#	endif //GL_VERSION_1_3

#	if GL_VERSION_1_4
	glBlendFuncSeparate = (gl_blend_func_separate_func *)fgl__GetOpenGLProcAddress(state, "glBlendFuncSeparate");
	glMultiDrawArrays = (gl_multi_draw_arrays_func *)fgl__GetOpenGLProcAddress(state, "glMultiDrawArrays");
	glMultiDrawElements = (gl_multi_draw_elements_func *)fgl__GetOpenGLProcAddress(state, "glMultiDrawElements");
	glPointParameterf = (gl_point_parameterf_func *)fgl__GetOpenGLProcAddress(state, "glPointParameterf");
	glPointParameterfv = (gl_point_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glPointParameterfv");
	glPointParameteri = (gl_point_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glPointParameteri");
	glPointParameteriv = (gl_point_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glPointParameteriv");
	glFogCoordf = (gl_fog_coordf_func *)fgl__GetOpenGLProcAddress(state, "glFogCoordf");
	glFogCoordfv = (gl_fog_coordfv_func *)fgl__GetOpenGLProcAddress(state, "glFogCoordfv");
	glFogCoordd = (gl_fog_coordd_func *)fgl__GetOpenGLProcAddress(state, "glFogCoordd");
	glFogCoorddv = (gl_fog_coorddv_func *)fgl__GetOpenGLProcAddress(state, "glFogCoorddv");
	glFogCoordPointer = (gl_fog_coord_pointer_func *)fgl__GetOpenGLProcAddress(state, "glFogCoordPointer");
	glSecondaryColor3b = (gl_secondary_color3b_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3b");
	glSecondaryColor3bv = (gl_secondary_color3bv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3bv");
	glSecondaryColor3d = (gl_secondary_color3d_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3d");
	glSecondaryColor3dv = (gl_secondary_color3dv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3dv");
	glSecondaryColor3f = (gl_secondary_color3f_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3f");
	glSecondaryColor3fv = (gl_secondary_color3fv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3fv");
	glSecondaryColor3i = (gl_secondary_color3i_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3i");
	glSecondaryColor3iv = (gl_secondary_color3iv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3iv");
	glSecondaryColor3s = (gl_secondary_color3s_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3s");
	glSecondaryColor3sv = (gl_secondary_color3sv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3sv");
	glSecondaryColor3ub = (gl_secondary_color3ub_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3ub");
	glSecondaryColor3ubv = (gl_secondary_color3ubv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3ubv");
	glSecondaryColor3ui = (gl_secondary_color3ui_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3ui");
	glSecondaryColor3uiv = (gl_secondary_color3uiv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3uiv");
	glSecondaryColor3us = (gl_secondary_color3us_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3us");
	glSecondaryColor3usv = (gl_secondary_color3usv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColor3usv");
	glSecondaryColorPointer = (gl_secondary_color_pointer_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColorPointer");
	glWindowPos2d = (gl_window_pos2d_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2d");
	glWindowPos2dv = (gl_window_pos2dv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2dv");
	glWindowPos2f = (gl_window_pos2f_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2f");
	glWindowPos2fv = (gl_window_pos2fv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2fv");
	glWindowPos2i = (gl_window_pos2i_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2i");
	glWindowPos2iv = (gl_window_pos2iv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2iv");
	glWindowPos2s = (gl_window_pos2s_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2s");
	glWindowPos2sv = (gl_window_pos2sv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos2sv");
	glWindowPos3d = (gl_window_pos3d_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3d");
	glWindowPos3dv = (gl_window_pos3dv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3dv");
	glWindowPos3f = (gl_window_pos3f_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3f");
	glWindowPos3fv = (gl_window_pos3fv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3fv");
	glWindowPos3i = (gl_window_pos3i_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3i");
	glWindowPos3iv = (gl_window_pos3iv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3iv");
	glWindowPos3s = (gl_window_pos3s_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3s");
	glWindowPos3sv = (gl_window_pos3sv_func *)fgl__GetOpenGLProcAddress(state, "glWindowPos3sv");
	glBlendColor = (gl_blend_color_func *)fgl__GetOpenGLProcAddress(state, "glBlendColor");
	glBlendEquation = (gl_blend_equation_func *)fgl__GetOpenGLProcAddress(state, "glBlendEquation");
#	endif //GL_VERSION_1_4

#	if GL_VERSION_1_5
	glGenQueries = (gl_gen_queries_func *)fgl__GetOpenGLProcAddress(state, "glGenQueries");
	glDeleteQueries = (gl_delete_queries_func *)fgl__GetOpenGLProcAddress(state, "glDeleteQueries");
	glIsQuery = (gl_is_query_func *)fgl__GetOpenGLProcAddress(state, "glIsQuery");
	glBeginQuery = (gl_begin_query_func *)fgl__GetOpenGLProcAddress(state, "glBeginQuery");
	glEndQuery = (gl_end_query_func *)fgl__GetOpenGLProcAddress(state, "glEndQuery");
	glGetQueryiv = (gl_get_queryiv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryiv");
	glGetQueryObjectiv = (gl_get_query_objectiv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryObjectiv");
	glGetQueryObjectuiv = (gl_get_query_objectuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryObjectuiv");
	glBindBuffer = (gl_bind_buffer_func *)fgl__GetOpenGLProcAddress(state, "glBindBuffer");
	glDeleteBuffers = (gl_delete_buffers_func *)fgl__GetOpenGLProcAddress(state, "glDeleteBuffers");
	glGenBuffers = (gl_gen_buffers_func *)fgl__GetOpenGLProcAddress(state, "glGenBuffers");
	glIsBuffer = (gl_is_buffer_func *)fgl__GetOpenGLProcAddress(state, "glIsBuffer");
	glBufferData = (gl_buffer_data_func *)fgl__GetOpenGLProcAddress(state, "glBufferData");
	glBufferSubData = (gl_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glBufferSubData");
	glGetBufferSubData = (gl_get_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glGetBufferSubData");
	glMapBuffer = (gl_map_buffer_func *)fgl__GetOpenGLProcAddress(state, "glMapBuffer");
	glUnmapBuffer = (gl_unmap_buffer_func *)fgl__GetOpenGLProcAddress(state, "glUnmapBuffer");
	glGetBufferParameteriv = (gl_get_buffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetBufferParameteriv");
	glGetBufferPointerv = (gl_get_buffer_pointerv_func *)fgl__GetOpenGLProcAddress(state, "glGetBufferPointerv");
#	endif //GL_VERSION_1_5

#	if GL_VERSION_2_0
	glBlendEquationSeparate = (gl_blend_equation_separate_func *)fgl__GetOpenGLProcAddress(state, "glBlendEquationSeparate");
	glDrawBuffers = (gl_draw_buffers_func *)fgl__GetOpenGLProcAddress(state, "glDrawBuffers");
	glStencilOpSeparate = (gl_stencil_op_separate_func *)fgl__GetOpenGLProcAddress(state, "glStencilOpSeparate");
	glStencilFuncSeparate = (gl_stencil_func_separate_func *)fgl__GetOpenGLProcAddress(state, "glStencilFuncSeparate");
	glStencilMaskSeparate = (gl_stencil_mask_separate_func *)fgl__GetOpenGLProcAddress(state, "glStencilMaskSeparate");
	glAttachShader = (gl_attach_shader_func *)fgl__GetOpenGLProcAddress(state, "glAttachShader");
	glBindAttribLocation = (gl_bind_attrib_location_func *)fgl__GetOpenGLProcAddress(state, "glBindAttribLocation");
	glCompileShader = (gl_compile_shader_func *)fgl__GetOpenGLProcAddress(state, "glCompileShader");
	glCreateProgram = (gl_create_program_func *)fgl__GetOpenGLProcAddress(state, "glCreateProgram");
	glCreateShader = (gl_create_shader_func *)fgl__GetOpenGLProcAddress(state, "glCreateShader");
	glDeleteProgram = (gl_delete_program_func *)fgl__GetOpenGLProcAddress(state, "glDeleteProgram");
	glDeleteShader = (gl_delete_shader_func *)fgl__GetOpenGLProcAddress(state, "glDeleteShader");
	glDetachShader = (gl_detach_shader_func *)fgl__GetOpenGLProcAddress(state, "glDetachShader");
	glDisableVertexAttribArray = (gl_disable_vertex_attrib_array_func *)fgl__GetOpenGLProcAddress(state, "glDisableVertexAttribArray");
	glEnableVertexAttribArray = (gl_enable_vertex_attrib_array_func *)fgl__GetOpenGLProcAddress(state, "glEnableVertexAttribArray");
	glGetActiveAttrib = (gl_get_active_attrib_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveAttrib");
	glGetActiveUniform = (gl_get_active_uniform_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveUniform");
	glGetAttachedShaders = (gl_get_attached_shaders_func *)fgl__GetOpenGLProcAddress(state, "glGetAttachedShaders");
	glGetAttribLocation = (gl_get_attrib_location_func *)fgl__GetOpenGLProcAddress(state, "glGetAttribLocation");
	glGetProgramiv = (gl_get_programiv_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramiv");
	glGetProgramInfoLog = (gl_get_program_info_log_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramInfoLog");
	glGetShaderiv = (gl_get_shaderiv_func *)fgl__GetOpenGLProcAddress(state, "glGetShaderiv");
	glGetShaderInfoLog = (gl_get_shader_info_log_func *)fgl__GetOpenGLProcAddress(state, "glGetShaderInfoLog");
	glGetShaderSource = (gl_get_shader_source_func *)fgl__GetOpenGLProcAddress(state, "glGetShaderSource");
	glGetUniformLocation = (gl_get_uniform_location_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformLocation");
	glGetUniformfv = (gl_get_uniformfv_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformfv");
	glGetUniformiv = (gl_get_uniformiv_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformiv");
	glGetVertexAttribdv = (gl_get_vertex_attribdv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribdv");
	glGetVertexAttribfv = (gl_get_vertex_attribfv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribfv");
	glGetVertexAttribiv = (gl_get_vertex_attribiv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribiv");
	glGetVertexAttribPointerv = (gl_get_vertex_attrib_pointerv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribPointerv");
	glIsProgram = (gl_is_program_func *)fgl__GetOpenGLProcAddress(state, "glIsProgram");
	glIsShader = (gl_is_shader_func *)fgl__GetOpenGLProcAddress(state, "glIsShader");
	glLinkProgram = (gl_link_program_func *)fgl__GetOpenGLProcAddress(state, "glLinkProgram");
	glShaderSource = (gl_shader_source_func *)fgl__GetOpenGLProcAddress(state, "glShaderSource");
	glUseProgram = (gl_use_program_func *)fgl__GetOpenGLProcAddress(state, "glUseProgram");
	glUniform1f = (gl_uniform1f_func *)fgl__GetOpenGLProcAddress(state, "glUniform1f");
	glUniform2f = (gl_uniform2f_func *)fgl__GetOpenGLProcAddress(state, "glUniform2f");
	glUniform3f = (gl_uniform3f_func *)fgl__GetOpenGLProcAddress(state, "glUniform3f");
	glUniform4f = (gl_uniform4f_func *)fgl__GetOpenGLProcAddress(state, "glUniform4f");
	glUniform1i = (gl_uniform1i_func *)fgl__GetOpenGLProcAddress(state, "glUniform1i");
	glUniform2i = (gl_uniform2i_func *)fgl__GetOpenGLProcAddress(state, "glUniform2i");
	glUniform3i = (gl_uniform3i_func *)fgl__GetOpenGLProcAddress(state, "glUniform3i");
	glUniform4i = (gl_uniform4i_func *)fgl__GetOpenGLProcAddress(state, "glUniform4i");
	glUniform1fv = (gl_uniform1fv_func *)fgl__GetOpenGLProcAddress(state, "glUniform1fv");
	glUniform2fv = (gl_uniform2fv_func *)fgl__GetOpenGLProcAddress(state, "glUniform2fv");
	glUniform3fv = (gl_uniform3fv_func *)fgl__GetOpenGLProcAddress(state, "glUniform3fv");
	glUniform4fv = (gl_uniform4fv_func *)fgl__GetOpenGLProcAddress(state, "glUniform4fv");
	glUniform1iv = (gl_uniform1iv_func *)fgl__GetOpenGLProcAddress(state, "glUniform1iv");
	glUniform2iv = (gl_uniform2iv_func *)fgl__GetOpenGLProcAddress(state, "glUniform2iv");
	glUniform3iv = (gl_uniform3iv_func *)fgl__GetOpenGLProcAddress(state, "glUniform3iv");
	glUniform4iv = (gl_uniform4iv_func *)fgl__GetOpenGLProcAddress(state, "glUniform4iv");
	glUniformMatrix2fv = (gl_uniform_matrix2fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2fv");
	glUniformMatrix3fv = (gl_uniform_matrix3fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3fv");
	glUniformMatrix4fv = (gl_uniform_matrix4fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4fv");
	glValidateProgram = (gl_validate_program_func *)fgl__GetOpenGLProcAddress(state, "glValidateProgram");
	glVertexAttrib1d = (gl_vertex_attrib1d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1d");
	glVertexAttrib1dv = (gl_vertex_attrib1dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1dv");
	glVertexAttrib1f = (gl_vertex_attrib1f_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1f");
	glVertexAttrib1fv = (gl_vertex_attrib1fv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1fv");
	glVertexAttrib1s = (gl_vertex_attrib1s_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1s");
	glVertexAttrib1sv = (gl_vertex_attrib1sv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib1sv");
	glVertexAttrib2d = (gl_vertex_attrib2d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2d");
	glVertexAttrib2dv = (gl_vertex_attrib2dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2dv");
	glVertexAttrib2f = (gl_vertex_attrib2f_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2f");
	glVertexAttrib2fv = (gl_vertex_attrib2fv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2fv");
	glVertexAttrib2s = (gl_vertex_attrib2s_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2s");
	glVertexAttrib2sv = (gl_vertex_attrib2sv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib2sv");
	glVertexAttrib3d = (gl_vertex_attrib3d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3d");
	glVertexAttrib3dv = (gl_vertex_attrib3dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3dv");
	glVertexAttrib3f = (gl_vertex_attrib3f_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3f");
	glVertexAttrib3fv = (gl_vertex_attrib3fv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3fv");
	glVertexAttrib3s = (gl_vertex_attrib3s_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3s");
	glVertexAttrib3sv = (gl_vertex_attrib3sv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib3sv");
	glVertexAttrib4Nbv = (gl_vertex_attrib4_nbv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nbv");
	glVertexAttrib4Niv = (gl_vertex_attrib4_niv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Niv");
	glVertexAttrib4Nsv = (gl_vertex_attrib4_nsv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nsv");
	glVertexAttrib4Nub = (gl_vertex_attrib4_nub_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nub");
	glVertexAttrib4Nubv = (gl_vertex_attrib4_nubv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nubv");
	glVertexAttrib4Nuiv = (gl_vertex_attrib4_nuiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nuiv");
	glVertexAttrib4Nusv = (gl_vertex_attrib4_nusv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4Nusv");
	glVertexAttrib4bv = (gl_vertex_attrib4bv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4bv");
	glVertexAttrib4d = (gl_vertex_attrib4d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4d");
	glVertexAttrib4dv = (gl_vertex_attrib4dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4dv");
	glVertexAttrib4f = (gl_vertex_attrib4f_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4f");
	glVertexAttrib4fv = (gl_vertex_attrib4fv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4fv");
	glVertexAttrib4iv = (gl_vertex_attrib4iv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4iv");
	glVertexAttrib4s = (gl_vertex_attrib4s_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4s");
	glVertexAttrib4sv = (gl_vertex_attrib4sv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4sv");
	glVertexAttrib4ubv = (gl_vertex_attrib4ubv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4ubv");
	glVertexAttrib4uiv = (gl_vertex_attrib4uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4uiv");
	glVertexAttrib4usv = (gl_vertex_attrib4usv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttrib4usv");
	glVertexAttribPointer = (gl_vertex_attrib_pointer_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribPointer");
#	endif //GL_VERSION_2_0

#	if GL_VERSION_2_1
	glUniformMatrix2x3fv = (gl_uniform_matrix2x3fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2x3fv");
	glUniformMatrix3x2fv = (gl_uniform_matrix3x2fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3x2fv");
	glUniformMatrix2x4fv = (gl_uniform_matrix2x4fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2x4fv");
	glUniformMatrix4x2fv = (gl_uniform_matrix4x2fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4x2fv");
	glUniformMatrix3x4fv = (gl_uniform_matrix3x4fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3x4fv");
	glUniformMatrix4x3fv = (gl_uniform_matrix4x3fv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4x3fv");
#	endif //GL_VERSION_2_1

#	if GL_VERSION_3_0
	glColorMaski = (gl_color_maski_func *)fgl__GetOpenGLProcAddress(state, "glColorMaski");
	glGetBooleani_v = (gl_get_booleani_v_func *)fgl__GetOpenGLProcAddress(state, "glGetBooleani_v");
	glGetIntegeri_v = (gl_get_integeri_v_func *)fgl__GetOpenGLProcAddress(state, "glGetIntegeri_v");
	glEnablei = (gl_enablei_func *)fgl__GetOpenGLProcAddress(state, "glEnablei");
	glDisablei = (gl_disablei_func *)fgl__GetOpenGLProcAddress(state, "glDisablei");
	glIsEnabledi = (gl_is_enabledi_func *)fgl__GetOpenGLProcAddress(state, "glIsEnabledi");
	glBeginTransformFeedback = (gl_begin_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glBeginTransformFeedback");
	glEndTransformFeedback = (gl_end_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glEndTransformFeedback");
	glBindBufferRange = (gl_bind_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glBindBufferRange");
	glBindBufferBase = (gl_bind_buffer_base_func *)fgl__GetOpenGLProcAddress(state, "glBindBufferBase");
	glTransformFeedbackVaryings = (gl_transform_feedback_varyings_func *)fgl__GetOpenGLProcAddress(state, "glTransformFeedbackVaryings");
	glGetTransformFeedbackVarying = (gl_get_transform_feedback_varying_func *)fgl__GetOpenGLProcAddress(state, "glGetTransformFeedbackVarying");
	glClampColor = (gl_clamp_color_func *)fgl__GetOpenGLProcAddress(state, "glClampColor");
	glBeginConditionalRender = (gl_begin_conditional_render_func *)fgl__GetOpenGLProcAddress(state, "glBeginConditionalRender");
	glEndConditionalRender = (gl_end_conditional_render_func *)fgl__GetOpenGLProcAddress(state, "glEndConditionalRender");
	glVertexAttribIPointer = (gl_vertex_attrib_i_pointer_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribIPointer");
	glGetVertexAttribIiv = (gl_get_vertex_attrib_iiv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribIiv");
	glGetVertexAttribIuiv = (gl_get_vertex_attrib_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribIuiv");
	glVertexAttribI1i = (gl_vertex_attrib_i1i_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI1i");
	glVertexAttribI2i = (gl_vertex_attrib_i2i_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI2i");
	glVertexAttribI3i = (gl_vertex_attrib_i3i_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI3i");
	glVertexAttribI4i = (gl_vertex_attrib_i4i_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4i");
	glVertexAttribI1ui = (gl_vertex_attrib_i1ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI1ui");
	glVertexAttribI2ui = (gl_vertex_attrib_i2ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI2ui");
	glVertexAttribI3ui = (gl_vertex_attrib_i3ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI3ui");
	glVertexAttribI4ui = (gl_vertex_attrib_i4ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4ui");
	glVertexAttribI1iv = (gl_vertex_attrib_i1iv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI1iv");
	glVertexAttribI2iv = (gl_vertex_attrib_i2iv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI2iv");
	glVertexAttribI3iv = (gl_vertex_attrib_i3iv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI3iv");
	glVertexAttribI4iv = (gl_vertex_attrib_i4iv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4iv");
	glVertexAttribI1uiv = (gl_vertex_attrib_i1uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI1uiv");
	glVertexAttribI2uiv = (gl_vertex_attrib_i2uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI2uiv");
	glVertexAttribI3uiv = (gl_vertex_attrib_i3uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI3uiv");
	glVertexAttribI4uiv = (gl_vertex_attrib_i4uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4uiv");
	glVertexAttribI4bv = (gl_vertex_attrib_i4bv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4bv");
	glVertexAttribI4sv = (gl_vertex_attrib_i4sv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4sv");
	glVertexAttribI4ubv = (gl_vertex_attrib_i4ubv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4ubv");
	glVertexAttribI4usv = (gl_vertex_attrib_i4usv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribI4usv");
	glGetUniformuiv = (gl_get_uniformuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformuiv");
	glBindFragDataLocation = (gl_bind_frag_data_location_func *)fgl__GetOpenGLProcAddress(state, "glBindFragDataLocation");
	glGetFragDataLocation = (gl_get_frag_data_location_func *)fgl__GetOpenGLProcAddress(state, "glGetFragDataLocation");
	glUniform1ui = (gl_uniform1ui_func *)fgl__GetOpenGLProcAddress(state, "glUniform1ui");
	glUniform2ui = (gl_uniform2ui_func *)fgl__GetOpenGLProcAddress(state, "glUniform2ui");
	glUniform3ui = (gl_uniform3ui_func *)fgl__GetOpenGLProcAddress(state, "glUniform3ui");
	glUniform4ui = (gl_uniform4ui_func *)fgl__GetOpenGLProcAddress(state, "glUniform4ui");
	glUniform1uiv = (gl_uniform1uiv_func *)fgl__GetOpenGLProcAddress(state, "glUniform1uiv");
	glUniform2uiv = (gl_uniform2uiv_func *)fgl__GetOpenGLProcAddress(state, "glUniform2uiv");
	glUniform3uiv = (gl_uniform3uiv_func *)fgl__GetOpenGLProcAddress(state, "glUniform3uiv");
	glUniform4uiv = (gl_uniform4uiv_func *)fgl__GetOpenGLProcAddress(state, "glUniform4uiv");
	glTexParameterIiv = (gl_tex_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glTexParameterIiv");
	glTexParameterIuiv = (gl_tex_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glTexParameterIuiv");
	glGetTexParameterIiv = (gl_get_tex_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexParameterIiv");
	glGetTexParameterIuiv = (gl_get_tex_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetTexParameterIuiv");
	glClearBufferiv = (gl_clear_bufferiv_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferiv");
	glClearBufferuiv = (gl_clear_bufferuiv_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferuiv");
	glClearBufferfv = (gl_clear_bufferfv_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferfv");
	glClearBufferfi = (gl_clear_bufferfi_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferfi");
	glGetStringi = (gl_get_stringi_func *)fgl__GetOpenGLProcAddress(state, "glGetStringi");
	glIsRenderbuffer = (gl_is_renderbuffer_func *)fgl__GetOpenGLProcAddress(state, "glIsRenderbuffer");
	glBindRenderbuffer = (gl_bind_renderbuffer_func *)fgl__GetOpenGLProcAddress(state, "glBindRenderbuffer");
	glDeleteRenderbuffers = (gl_delete_renderbuffers_func *)fgl__GetOpenGLProcAddress(state, "glDeleteRenderbuffers");
	glGenRenderbuffers = (gl_gen_renderbuffers_func *)fgl__GetOpenGLProcAddress(state, "glGenRenderbuffers");
	glRenderbufferStorage = (gl_renderbuffer_storage_func *)fgl__GetOpenGLProcAddress(state, "glRenderbufferStorage");
	glGetRenderbufferParameteriv = (gl_get_renderbuffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetRenderbufferParameteriv");
	glIsFramebuffer = (gl_is_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glIsFramebuffer");
	glBindFramebuffer = (gl_bind_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glBindFramebuffer");
	glDeleteFramebuffers = (gl_delete_framebuffers_func *)fgl__GetOpenGLProcAddress(state, "glDeleteFramebuffers");
	glGenFramebuffers = (gl_gen_framebuffers_func *)fgl__GetOpenGLProcAddress(state, "glGenFramebuffers");
	glCheckFramebufferStatus = (gl_check_framebuffer_status_func *)fgl__GetOpenGLProcAddress(state, "glCheckFramebufferStatus");
	glFramebufferTexture1D = (gl_framebuffer_texture1d_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferTexture1D");
	glFramebufferTexture2D = (gl_framebuffer_texture2d_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferTexture2D");
	glFramebufferTexture3D = (gl_framebuffer_texture3d_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferTexture3D");
	glFramebufferRenderbuffer = (gl_framebuffer_renderbuffer_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferRenderbuffer");
	glGetFramebufferAttachmentParameteriv = (gl_get_framebuffer_attachment_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetFramebufferAttachmentParameteriv");
	glGenerateMipmap = (gl_generate_mipmap_func *)fgl__GetOpenGLProcAddress(state, "glGenerateMipmap");
	glBlitFramebuffer = (gl_blit_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glBlitFramebuffer");
	glRenderbufferStorageMultisample = (gl_renderbuffer_storage_multisample_func *)fgl__GetOpenGLProcAddress(state, "glRenderbufferStorageMultisample");
	glFramebufferTextureLayer = (gl_framebuffer_texture_layer_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferTextureLayer");
	glMapBufferRange = (gl_map_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glMapBufferRange");
	glFlushMappedBufferRange = (gl_flush_mapped_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glFlushMappedBufferRange");
	glBindVertexArray = (gl_bind_vertex_array_func *)fgl__GetOpenGLProcAddress(state, "glBindVertexArray");
	glDeleteVertexArrays = (gl_delete_vertex_arrays_func *)fgl__GetOpenGLProcAddress(state, "glDeleteVertexArrays");
	glGenVertexArrays = (gl_gen_vertex_arrays_func *)fgl__GetOpenGLProcAddress(state, "glGenVertexArrays");
	glIsVertexArray = (gl_is_vertex_array_func *)fgl__GetOpenGLProcAddress(state, "glIsVertexArray");
#	endif //GL_VERSION_3_0

#	if GL_VERSION_3_1
	glDrawArraysInstanced = (gl_draw_arrays_instanced_func *)fgl__GetOpenGLProcAddress(state, "glDrawArraysInstanced");
	glDrawElementsInstanced = (gl_draw_elements_instanced_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsInstanced");
	glTexBuffer = (gl_tex_buffer_func *)fgl__GetOpenGLProcAddress(state, "glTexBuffer");
	glPrimitiveRestartIndex = (gl_primitive_restart_index_func *)fgl__GetOpenGLProcAddress(state, "glPrimitiveRestartIndex");
	glCopyBufferSubData = (gl_copy_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glCopyBufferSubData");
	glGetUniformIndices = (gl_get_uniform_indices_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformIndices");
	glGetActiveUniformsiv = (gl_get_active_uniformsiv_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveUniformsiv");
	glGetActiveUniformName = (gl_get_active_uniform_name_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveUniformName");
	glGetUniformBlockIndex = (gl_get_uniform_block_index_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformBlockIndex");
	glGetActiveUniformBlockiv = (gl_get_active_uniform_blockiv_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveUniformBlockiv");
	glGetActiveUniformBlockName = (gl_get_active_uniform_block_name_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveUniformBlockName");
	glUniformBlockBinding = (gl_uniform_block_binding_func *)fgl__GetOpenGLProcAddress(state, "glUniformBlockBinding");
#	endif //GL_VERSION_3_1

#	if GL_VERSION_3_2
	glDrawElementsBaseVertex = (gl_draw_elements_base_vertex_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsBaseVertex");
	glDrawRangeElementsBaseVertex = (gl_draw_range_elements_base_vertex_func *)fgl__GetOpenGLProcAddress(state, "glDrawRangeElementsBaseVertex");
	glDrawElementsInstancedBaseVertex = (gl_draw_elements_instanced_base_vertex_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsInstancedBaseVertex");
	glMultiDrawElementsBaseVertex = (gl_multi_draw_elements_base_vertex_func *)fgl__GetOpenGLProcAddress(state, "glMultiDrawElementsBaseVertex");
	glProvokingVertex = (gl_provoking_vertex_func *)fgl__GetOpenGLProcAddress(state, "glProvokingVertex");
	glFenceSync = (gl_fence_sync_func *)fgl__GetOpenGLProcAddress(state, "glFenceSync");
	glIsSync = (gl_is_sync_func *)fgl__GetOpenGLProcAddress(state, "glIsSync");
	glDeleteSync = (gl_delete_sync_func *)fgl__GetOpenGLProcAddress(state, "glDeleteSync");
	glClientWaitSync = (gl_client_wait_sync_func *)fgl__GetOpenGLProcAddress(state, "glClientWaitSync");
	glWaitSync = (gl_wait_sync_func *)fgl__GetOpenGLProcAddress(state, "glWaitSync");
	glGetInteger64v = (gl_get_integer64v_func *)fgl__GetOpenGLProcAddress(state, "glGetInteger64v");
	glGetSynciv = (gl_get_synciv_func *)fgl__GetOpenGLProcAddress(state, "glGetSynciv");
	glGetInteger64i_v = (gl_get_integer64i_v_func *)fgl__GetOpenGLProcAddress(state, "glGetInteger64i_v");
	glGetBufferParameteri64v = (gl_get_buffer_parameteri64v_func *)fgl__GetOpenGLProcAddress(state, "glGetBufferParameteri64v");
	glFramebufferTexture = (gl_framebuffer_texture_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferTexture");
	glTexImage2DMultisample = (gl_tex_image2_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTexImage2DMultisample");
	glTexImage3DMultisample = (gl_tex_image3_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTexImage3DMultisample");
	glGetMultisamplefv = (gl_get_multisamplefv_func *)fgl__GetOpenGLProcAddress(state, "glGetMultisamplefv");
	glSampleMaski = (gl_sample_maski_func *)fgl__GetOpenGLProcAddress(state, "glSampleMaski");
#	endif //GL_VERSION_3_2

#	if GL_VERSION_3_3
	glBindFragDataLocationIndexed = (gl_bind_frag_data_location_indexed_func *)fgl__GetOpenGLProcAddress(state, "glBindFragDataLocationIndexed");
	glGetFragDataIndex = (gl_get_frag_data_index_func *)fgl__GetOpenGLProcAddress(state, "glGetFragDataIndex");
	glGenSamplers = (gl_gen_samplers_func *)fgl__GetOpenGLProcAddress(state, "glGenSamplers");
	glDeleteSamplers = (gl_delete_samplers_func *)fgl__GetOpenGLProcAddress(state, "glDeleteSamplers");
	glIsSampler = (gl_is_sampler_func *)fgl__GetOpenGLProcAddress(state, "glIsSampler");
	glBindSampler = (gl_bind_sampler_func *)fgl__GetOpenGLProcAddress(state, "glBindSampler");
	glSamplerParameteri = (gl_sampler_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameteri");
	glSamplerParameteriv = (gl_sampler_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameteriv");
	glSamplerParameterf = (gl_sampler_parameterf_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameterf");
	glSamplerParameterfv = (gl_sampler_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameterfv");
	glSamplerParameterIiv = (gl_sampler_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameterIiv");
	glSamplerParameterIuiv = (gl_sampler_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glSamplerParameterIuiv");
	glGetSamplerParameteriv = (gl_get_sampler_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetSamplerParameteriv");
	glGetSamplerParameterIiv = (gl_get_sampler_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glGetSamplerParameterIiv");
	glGetSamplerParameterfv = (gl_get_sampler_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glGetSamplerParameterfv");
	glGetSamplerParameterIuiv = (gl_get_sampler_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetSamplerParameterIuiv");
	glQueryCounter = (gl_query_counter_func *)fgl__GetOpenGLProcAddress(state, "glQueryCounter");
	glGetQueryObjecti64v = (gl_get_query_objecti64v_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryObjecti64v");
	glGetQueryObjectui64v = (gl_get_query_objectui64v_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryObjectui64v");
	glVertexAttribDivisor = (gl_vertex_attrib_divisor_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribDivisor");
	glVertexAttribP1ui = (gl_vertex_attrib_p1ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP1ui");
	glVertexAttribP1uiv = (gl_vertex_attrib_p1uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP1uiv");
	glVertexAttribP2ui = (gl_vertex_attrib_p2ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP2ui");
	glVertexAttribP2uiv = (gl_vertex_attrib_p2uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP2uiv");
	glVertexAttribP3ui = (gl_vertex_attrib_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP3ui");
	glVertexAttribP3uiv = (gl_vertex_attrib_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP3uiv");
	glVertexAttribP4ui = (gl_vertex_attrib_p4ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP4ui");
	glVertexAttribP4uiv = (gl_vertex_attrib_p4uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribP4uiv");
	glVertexP2ui = (gl_vertex_p2ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexP2ui");
	glVertexP2uiv = (gl_vertex_p2uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexP2uiv");
	glVertexP3ui = (gl_vertex_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexP3ui");
	glVertexP3uiv = (gl_vertex_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexP3uiv");
	glVertexP4ui = (gl_vertex_p4ui_func *)fgl__GetOpenGLProcAddress(state, "glVertexP4ui");
	glVertexP4uiv = (gl_vertex_p4uiv_func *)fgl__GetOpenGLProcAddress(state, "glVertexP4uiv");
	glTexCoordP1ui = (gl_tex_coord_p1ui_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP1ui");
	glTexCoordP1uiv = (gl_tex_coord_p1uiv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP1uiv");
	glTexCoordP2ui = (gl_tex_coord_p2ui_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP2ui");
	glTexCoordP2uiv = (gl_tex_coord_p2uiv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP2uiv");
	glTexCoordP3ui = (gl_tex_coord_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP3ui");
	glTexCoordP3uiv = (gl_tex_coord_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP3uiv");
	glTexCoordP4ui = (gl_tex_coord_p4ui_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP4ui");
	glTexCoordP4uiv = (gl_tex_coord_p4uiv_func *)fgl__GetOpenGLProcAddress(state, "glTexCoordP4uiv");
	glMultiTexCoordP1ui = (gl_multi_tex_coord_p1ui_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP1ui");
	glMultiTexCoordP1uiv = (gl_multi_tex_coord_p1uiv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP1uiv");
	glMultiTexCoordP2ui = (gl_multi_tex_coord_p2ui_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP2ui");
	glMultiTexCoordP2uiv = (gl_multi_tex_coord_p2uiv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP2uiv");
	glMultiTexCoordP3ui = (gl_multi_tex_coord_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP3ui");
	glMultiTexCoordP3uiv = (gl_multi_tex_coord_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP3uiv");
	glMultiTexCoordP4ui = (gl_multi_tex_coord_p4ui_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP4ui");
	glMultiTexCoordP4uiv = (gl_multi_tex_coord_p4uiv_func *)fgl__GetOpenGLProcAddress(state, "glMultiTexCoordP4uiv");
	glNormalP3ui = (gl_normal_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glNormalP3ui");
	glNormalP3uiv = (gl_normal_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glNormalP3uiv");
	glColorP3ui = (gl_color_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glColorP3ui");
	glColorP3uiv = (gl_color_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glColorP3uiv");
	glColorP4ui = (gl_color_p4ui_func *)fgl__GetOpenGLProcAddress(state, "glColorP4ui");
	glColorP4uiv = (gl_color_p4uiv_func *)fgl__GetOpenGLProcAddress(state, "glColorP4uiv");
	glSecondaryColorP3ui = (gl_secondary_color_p3ui_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColorP3ui");
	glSecondaryColorP3uiv = (gl_secondary_color_p3uiv_func *)fgl__GetOpenGLProcAddress(state, "glSecondaryColorP3uiv");
#	endif //GL_VERSION_3_3

#	if GL_VERSION_4_0
	glMinSampleShading = (gl_min_sample_shading_func *)fgl__GetOpenGLProcAddress(state, "glMinSampleShading");
	glBlendEquationi = (gl_blend_equationi_func *)fgl__GetOpenGLProcAddress(state, "glBlendEquationi");
	glBlendEquationSeparatei = (gl_blend_equation_separatei_func *)fgl__GetOpenGLProcAddress(state, "glBlendEquationSeparatei");
	glBlendFunci = (gl_blend_funci_func *)fgl__GetOpenGLProcAddress(state, "glBlendFunci");
	glBlendFuncSeparatei = (gl_blend_func_separatei_func *)fgl__GetOpenGLProcAddress(state, "glBlendFuncSeparatei");
	glDrawArraysIndirect = (gl_draw_arrays_indirect_func *)fgl__GetOpenGLProcAddress(state, "glDrawArraysIndirect");
	glDrawElementsIndirect = (gl_draw_elements_indirect_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsIndirect");
	glUniform1d = (gl_uniform1d_func *)fgl__GetOpenGLProcAddress(state, "glUniform1d");
	glUniform2d = (gl_uniform2d_func *)fgl__GetOpenGLProcAddress(state, "glUniform2d");
	glUniform3d = (gl_uniform3d_func *)fgl__GetOpenGLProcAddress(state, "glUniform3d");
	glUniform4d = (gl_uniform4d_func *)fgl__GetOpenGLProcAddress(state, "glUniform4d");
	glUniform1dv = (gl_uniform1dv_func *)fgl__GetOpenGLProcAddress(state, "glUniform1dv");
	glUniform2dv = (gl_uniform2dv_func *)fgl__GetOpenGLProcAddress(state, "glUniform2dv");
	glUniform3dv = (gl_uniform3dv_func *)fgl__GetOpenGLProcAddress(state, "glUniform3dv");
	glUniform4dv = (gl_uniform4dv_func *)fgl__GetOpenGLProcAddress(state, "glUniform4dv");
	glUniformMatrix2dv = (gl_uniform_matrix2dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2dv");
	glUniformMatrix3dv = (gl_uniform_matrix3dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3dv");
	glUniformMatrix4dv = (gl_uniform_matrix4dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4dv");
	glUniformMatrix2x3dv = (gl_uniform_matrix2x3dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2x3dv");
	glUniformMatrix2x4dv = (gl_uniform_matrix2x4dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix2x4dv");
	glUniformMatrix3x2dv = (gl_uniform_matrix3x2dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3x2dv");
	glUniformMatrix3x4dv = (gl_uniform_matrix3x4dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix3x4dv");
	glUniformMatrix4x2dv = (gl_uniform_matrix4x2dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4x2dv");
	glUniformMatrix4x3dv = (gl_uniform_matrix4x3dv_func *)fgl__GetOpenGLProcAddress(state, "glUniformMatrix4x3dv");
	glGetUniformdv = (gl_get_uniformdv_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformdv");
	glGetSubroutineUniformLocation = (gl_get_subroutine_uniform_location_func *)fgl__GetOpenGLProcAddress(state, "glGetSubroutineUniformLocation");
	glGetSubroutineIndex = (gl_get_subroutine_index_func *)fgl__GetOpenGLProcAddress(state, "glGetSubroutineIndex");
	glGetActiveSubroutineUniformiv = (gl_get_active_subroutine_uniformiv_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveSubroutineUniformiv");
	glGetActiveSubroutineUniformName = (gl_get_active_subroutine_uniform_name_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveSubroutineUniformName");
	glGetActiveSubroutineName = (gl_get_active_subroutine_name_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveSubroutineName");
	glUniformSubroutinesuiv = (gl_uniform_subroutinesuiv_func *)fgl__GetOpenGLProcAddress(state, "glUniformSubroutinesuiv");
	glGetUniformSubroutineuiv = (gl_get_uniform_subroutineuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetUniformSubroutineuiv");
	glGetProgramStageiv = (gl_get_program_stageiv_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramStageiv");
	glPatchParameteri = (gl_patch_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glPatchParameteri");
	glPatchParameterfv = (gl_patch_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glPatchParameterfv");
	glBindTransformFeedback = (gl_bind_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glBindTransformFeedback");
	glDeleteTransformFeedbacks = (gl_delete_transform_feedbacks_func *)fgl__GetOpenGLProcAddress(state, "glDeleteTransformFeedbacks");
	glGenTransformFeedbacks = (gl_gen_transform_feedbacks_func *)fgl__GetOpenGLProcAddress(state, "glGenTransformFeedbacks");
	glIsTransformFeedback = (gl_is_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glIsTransformFeedback");
	glPauseTransformFeedback = (gl_pause_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glPauseTransformFeedback");
	glResumeTransformFeedback = (gl_resume_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glResumeTransformFeedback");
	glDrawTransformFeedback = (gl_draw_transform_feedback_func *)fgl__GetOpenGLProcAddress(state, "glDrawTransformFeedback");
	glDrawTransformFeedbackStream = (gl_draw_transform_feedback_stream_func *)fgl__GetOpenGLProcAddress(state, "glDrawTransformFeedbackStream");
	glBeginQueryIndexed = (gl_begin_query_indexed_func *)fgl__GetOpenGLProcAddress(state, "glBeginQueryIndexed");
	glEndQueryIndexed = (gl_end_query_indexed_func *)fgl__GetOpenGLProcAddress(state, "glEndQueryIndexed");
	glGetQueryIndexediv = (gl_get_query_indexediv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryIndexediv");
#	endif //GL_VERSION_4_0

#	if GL_VERSION_4_1
	glReleaseShaderCompiler = (gl_release_shader_compiler_func *)fgl__GetOpenGLProcAddress(state, "glReleaseShaderCompiler");
	glShaderBinary = (gl_shader_binary_func *)fgl__GetOpenGLProcAddress(state, "glShaderBinary");
	glGetShaderPrecisionFormat = (gl_get_shader_precision_format_func *)fgl__GetOpenGLProcAddress(state, "glGetShaderPrecisionFormat");
	glDepthRangef = (gl_depth_rangef_func *)fgl__GetOpenGLProcAddress(state, "glDepthRangef");
	glClearDepthf = (gl_clear_depthf_func *)fgl__GetOpenGLProcAddress(state, "glClearDepthf");
	glGetProgramBinary = (gl_get_program_binary_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramBinary");
	glProgramBinary = (gl_program_binary_func *)fgl__GetOpenGLProcAddress(state, "glProgramBinary");
	glProgramParameteri = (gl_program_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glProgramParameteri");
	glUseProgramStages = (gl_use_program_stages_func *)fgl__GetOpenGLProcAddress(state, "glUseProgramStages");
	glActiveShaderProgram = (gl_active_shader_program_func *)fgl__GetOpenGLProcAddress(state, "glActiveShaderProgram");
	glCreateShaderProgramv = (gl_create_shader_programv_func *)fgl__GetOpenGLProcAddress(state, "glCreateShaderProgramv");
	glBindProgramPipeline = (gl_bind_program_pipeline_func *)fgl__GetOpenGLProcAddress(state, "glBindProgramPipeline");
	glDeleteProgramPipelines = (gl_delete_program_pipelines_func *)fgl__GetOpenGLProcAddress(state, "glDeleteProgramPipelines");
	glGenProgramPipelines = (gl_gen_program_pipelines_func *)fgl__GetOpenGLProcAddress(state, "glGenProgramPipelines");
	glIsProgramPipeline = (gl_is_program_pipeline_func *)fgl__GetOpenGLProcAddress(state, "glIsProgramPipeline");
	glGetProgramPipelineiv = (gl_get_program_pipelineiv_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramPipelineiv");
	glProgramUniform1i = (gl_program_uniform1i_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1i");
	glProgramUniform1iv = (gl_program_uniform1iv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1iv");
	glProgramUniform1f = (gl_program_uniform1f_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1f");
	glProgramUniform1fv = (gl_program_uniform1fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1fv");
	glProgramUniform1d = (gl_program_uniform1d_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1d");
	glProgramUniform1dv = (gl_program_uniform1dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1dv");
	glProgramUniform1ui = (gl_program_uniform1ui_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1ui");
	glProgramUniform1uiv = (gl_program_uniform1uiv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform1uiv");
	glProgramUniform2i = (gl_program_uniform2i_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2i");
	glProgramUniform2iv = (gl_program_uniform2iv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2iv");
	glProgramUniform2f = (gl_program_uniform2f_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2f");
	glProgramUniform2fv = (gl_program_uniform2fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2fv");
	glProgramUniform2d = (gl_program_uniform2d_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2d");
	glProgramUniform2dv = (gl_program_uniform2dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2dv");
	glProgramUniform2ui = (gl_program_uniform2ui_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2ui");
	glProgramUniform2uiv = (gl_program_uniform2uiv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform2uiv");
	glProgramUniform3i = (gl_program_uniform3i_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3i");
	glProgramUniform3iv = (gl_program_uniform3iv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3iv");
	glProgramUniform3f = (gl_program_uniform3f_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3f");
	glProgramUniform3fv = (gl_program_uniform3fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3fv");
	glProgramUniform3d = (gl_program_uniform3d_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3d");
	glProgramUniform3dv = (gl_program_uniform3dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3dv");
	glProgramUniform3ui = (gl_program_uniform3ui_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3ui");
	glProgramUniform3uiv = (gl_program_uniform3uiv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform3uiv");
	glProgramUniform4i = (gl_program_uniform4i_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4i");
	glProgramUniform4iv = (gl_program_uniform4iv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4iv");
	glProgramUniform4f = (gl_program_uniform4f_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4f");
	glProgramUniform4fv = (gl_program_uniform4fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4fv");
	glProgramUniform4d = (gl_program_uniform4d_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4d");
	glProgramUniform4dv = (gl_program_uniform4dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4dv");
	glProgramUniform4ui = (gl_program_uniform4ui_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4ui");
	glProgramUniform4uiv = (gl_program_uniform4uiv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniform4uiv");
	glProgramUniformMatrix2fv = (gl_program_uniform_matrix2fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2fv");
	glProgramUniformMatrix3fv = (gl_program_uniform_matrix3fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3fv");
	glProgramUniformMatrix4fv = (gl_program_uniform_matrix4fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4fv");
	glProgramUniformMatrix2dv = (gl_program_uniform_matrix2dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2dv");
	glProgramUniformMatrix3dv = (gl_program_uniform_matrix3dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3dv");
	glProgramUniformMatrix4dv = (gl_program_uniform_matrix4dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4dv");
	glProgramUniformMatrix2x3fv = (gl_program_uniform_matrix2x3fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2x3fv");
	glProgramUniformMatrix3x2fv = (gl_program_uniform_matrix3x2fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3x2fv");
	glProgramUniformMatrix2x4fv = (gl_program_uniform_matrix2x4fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2x4fv");
	glProgramUniformMatrix4x2fv = (gl_program_uniform_matrix4x2fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4x2fv");
	glProgramUniformMatrix3x4fv = (gl_program_uniform_matrix3x4fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3x4fv");
	glProgramUniformMatrix4x3fv = (gl_program_uniform_matrix4x3fv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4x3fv");
	glProgramUniformMatrix2x3dv = (gl_program_uniform_matrix2x3dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2x3dv");
	glProgramUniformMatrix3x2dv = (gl_program_uniform_matrix3x2dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3x2dv");
	glProgramUniformMatrix2x4dv = (gl_program_uniform_matrix2x4dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix2x4dv");
	glProgramUniformMatrix4x2dv = (gl_program_uniform_matrix4x2dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4x2dv");
	glProgramUniformMatrix3x4dv = (gl_program_uniform_matrix3x4dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix3x4dv");
	glProgramUniformMatrix4x3dv = (gl_program_uniform_matrix4x3dv_func *)fgl__GetOpenGLProcAddress(state, "glProgramUniformMatrix4x3dv");
	glValidateProgramPipeline = (gl_validate_program_pipeline_func *)fgl__GetOpenGLProcAddress(state, "glValidateProgramPipeline");
	glGetProgramPipelineInfoLog = (gl_get_program_pipeline_info_log_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramPipelineInfoLog");
	glVertexAttribL1d = (gl_vertex_attrib_l1d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL1d");
	glVertexAttribL2d = (gl_vertex_attrib_l2d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL2d");
	glVertexAttribL3d = (gl_vertex_attrib_l3d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL3d");
	glVertexAttribL4d = (gl_vertex_attrib_l4d_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL4d");
	glVertexAttribL1dv = (gl_vertex_attrib_l1dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL1dv");
	glVertexAttribL2dv = (gl_vertex_attrib_l2dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL2dv");
	glVertexAttribL3dv = (gl_vertex_attrib_l3dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL3dv");
	glVertexAttribL4dv = (gl_vertex_attrib_l4dv_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribL4dv");
	glVertexAttribLPointer = (gl_vertex_attrib_l_pointer_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribLPointer");
	glGetVertexAttribLdv = (gl_get_vertex_attrib_ldv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexAttribLdv");
	glViewportArrayv = (gl_viewport_arrayv_func *)fgl__GetOpenGLProcAddress(state, "glViewportArrayv");
	glViewportIndexedf = (gl_viewport_indexedf_func *)fgl__GetOpenGLProcAddress(state, "glViewportIndexedf");
	glViewportIndexedfv = (gl_viewport_indexedfv_func *)fgl__GetOpenGLProcAddress(state, "glViewportIndexedfv");
	glScissorArrayv = (gl_scissor_arrayv_func *)fgl__GetOpenGLProcAddress(state, "glScissorArrayv");
	glScissorIndexed = (gl_scissor_indexed_func *)fgl__GetOpenGLProcAddress(state, "glScissorIndexed");
	glScissorIndexedv = (gl_scissor_indexedv_func *)fgl__GetOpenGLProcAddress(state, "glScissorIndexedv");
	glDepthRangeArrayv = (gl_depth_range_arrayv_func *)fgl__GetOpenGLProcAddress(state, "glDepthRangeArrayv");
	glDepthRangeIndexed = (gl_depth_range_indexed_func *)fgl__GetOpenGLProcAddress(state, "glDepthRangeIndexed");
	glGetFloati_v = (gl_get_floati_v_func *)fgl__GetOpenGLProcAddress(state, "glGetFloati_v");
	glGetDoublei_v = (gl_get_doublei_v_func *)fgl__GetOpenGLProcAddress(state, "glGetDoublei_v");
#	endif //GL_VERSION_4_1

#	if GL_VERSION_4_2
	glDrawArraysInstancedBaseInstance = (gl_draw_arrays_instanced_base_instance_func *)fgl__GetOpenGLProcAddress(state, "glDrawArraysInstancedBaseInstance");
	glDrawElementsInstancedBaseInstance = (gl_draw_elements_instanced_base_instance_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsInstancedBaseInstance");
	glDrawElementsInstancedBaseVertexBaseInstance = (gl_draw_elements_instanced_base_vertex_base_instance_func *)fgl__GetOpenGLProcAddress(state, "glDrawElementsInstancedBaseVertexBaseInstance");
	glGetInternalformativ = (gl_get_internalformativ_func *)fgl__GetOpenGLProcAddress(state, "glGetInternalformativ");
	glGetActiveAtomicCounterBufferiv = (gl_get_active_atomic_counter_bufferiv_func *)fgl__GetOpenGLProcAddress(state, "glGetActiveAtomicCounterBufferiv");
	glBindImageTexture = (gl_bind_image_texture_func *)fgl__GetOpenGLProcAddress(state, "glBindImageTexture");
	glMemoryBarrier = (gl_memory_barrier_func *)fgl__GetOpenGLProcAddress(state, "glMemoryBarrier");
	glTexStorage1D = (gl_tex_storage1d_func *)fgl__GetOpenGLProcAddress(state, "glTexStorage1D");
	glTexStorage2D = (gl_tex_storage2d_func *)fgl__GetOpenGLProcAddress(state, "glTexStorage2D");
	glTexStorage3D = (gl_tex_storage3d_func *)fgl__GetOpenGLProcAddress(state, "glTexStorage3D");
	glDrawTransformFeedbackInstanced = (gl_draw_transform_feedback_instanced_func *)fgl__GetOpenGLProcAddress(state, "glDrawTransformFeedbackInstanced");
	glDrawTransformFeedbackStreamInstanced = (gl_draw_transform_feedback_stream_instanced_func *)fgl__GetOpenGLProcAddress(state, "glDrawTransformFeedbackStreamInstanced");
#	endif //GL_VERSION_4_2

#	if GL_VERSION_4_3
	glClearBufferData = (gl_clear_buffer_data_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferData");
	glClearBufferSubData = (gl_clear_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glClearBufferSubData");
	glDispatchCompute = (gl_dispatch_compute_func *)fgl__GetOpenGLProcAddress(state, "glDispatchCompute");
	glDispatchComputeIndirect = (gl_dispatch_compute_indirect_func *)fgl__GetOpenGLProcAddress(state, "glDispatchComputeIndirect");
	glCopyImageSubData = (gl_copy_image_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glCopyImageSubData");
	glFramebufferParameteri = (gl_framebuffer_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glFramebufferParameteri");
	glGetFramebufferParameteriv = (gl_get_framebuffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetFramebufferParameteriv");
	glGetInternalformati64v = (gl_get_internalformati64v_func *)fgl__GetOpenGLProcAddress(state, "glGetInternalformati64v");
	glInvalidateTexSubImage = (gl_invalidate_tex_sub_image_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateTexSubImage");
	glInvalidateTexImage = (gl_invalidate_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateTexImage");
	glInvalidateBufferSubData = (gl_invalidate_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateBufferSubData");
	glInvalidateBufferData = (gl_invalidate_buffer_data_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateBufferData");
	glInvalidateFramebuffer = (gl_invalidate_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateFramebuffer");
	glInvalidateSubFramebuffer = (gl_invalidate_sub_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateSubFramebuffer");
	glMultiDrawArraysIndirect = (gl_multi_draw_arrays_indirect_func *)fgl__GetOpenGLProcAddress(state, "glMultiDrawArraysIndirect");
	glMultiDrawElementsIndirect = (gl_multi_draw_elements_indirect_func *)fgl__GetOpenGLProcAddress(state, "glMultiDrawElementsIndirect");
	glGetProgramInterfaceiv = (gl_get_program_interfaceiv_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramInterfaceiv");
	glGetProgramResourceIndex = (gl_get_program_resource_index_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramResourceIndex");
	glGetProgramResourceName = (gl_get_program_resource_name_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramResourceName");
	glGetProgramResourceiv = (gl_get_program_resourceiv_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramResourceiv");
	glGetProgramResourceLocation = (gl_get_program_resource_location_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramResourceLocation");
	glGetProgramResourceLocationIndex = (gl_get_program_resource_location_index_func *)fgl__GetOpenGLProcAddress(state, "glGetProgramResourceLocationIndex");
	glShaderStorageBlockBinding = (gl_shader_storage_block_binding_func *)fgl__GetOpenGLProcAddress(state, "glShaderStorageBlockBinding");
	glTexBufferRange = (gl_tex_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glTexBufferRange");
	glTexStorage2DMultisample = (gl_tex_storage2_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTexStorage2DMultisample");
	glTexStorage3DMultisample = (gl_tex_storage3_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTexStorage3DMultisample");
	glTextureView = (gl_texture_view_func *)fgl__GetOpenGLProcAddress(state, "glTextureView");
	glBindVertexBuffer = (gl_bind_vertex_buffer_func *)fgl__GetOpenGLProcAddress(state, "glBindVertexBuffer");
	glVertexAttribFormat = (gl_vertex_attrib_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribFormat");
	glVertexAttribIFormat = (gl_vertex_attrib_i_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribIFormat");
	glVertexAttribLFormat = (gl_vertex_attrib_l_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribLFormat");
	glVertexAttribBinding = (gl_vertex_attrib_binding_func *)fgl__GetOpenGLProcAddress(state, "glVertexAttribBinding");
	glVertexBindingDivisor = (gl_vertex_binding_divisor_func *)fgl__GetOpenGLProcAddress(state, "glVertexBindingDivisor");
	glDebugMessageControl = (gl_debug_message_control_func *)fgl__GetOpenGLProcAddress(state, "glDebugMessageControl");
	glDebugMessageInsert = (gl_debug_message_insert_func *)fgl__GetOpenGLProcAddress(state, "glDebugMessageInsert");
	glDebugMessageCallback = (gl_debug_message_callback_func *)fgl__GetOpenGLProcAddress(state, "glDebugMessageCallback");
	glGetDebugMessageLog = (gl_get_debug_message_log_func *)fgl__GetOpenGLProcAddress(state, "glGetDebugMessageLog");
	glPushDebugGroup = (gl_push_debug_group_func *)fgl__GetOpenGLProcAddress(state, "glPushDebugGroup");
	glPopDebugGroup = (gl_pop_debug_group_func *)fgl__GetOpenGLProcAddress(state, "glPopDebugGroup");
	glObjectLabel = (gl_object_label_func *)fgl__GetOpenGLProcAddress(state, "glObjectLabel");
	glGetObjectLabel = (gl_get_object_label_func *)fgl__GetOpenGLProcAddress(state, "glGetObjectLabel");
	glObjectPtrLabel = (gl_object_ptr_label_func *)fgl__GetOpenGLProcAddress(state, "glObjectPtrLabel");
	glGetObjectPtrLabel = (gl_get_object_ptr_label_func *)fgl__GetOpenGLProcAddress(state, "glGetObjectPtrLabel");
#	endif //GL_VERSION_4_3

#	if GL_VERSION_4_4
	glBufferStorage = (gl_buffer_storage_func *)fgl__GetOpenGLProcAddress(state, "glBufferStorage");
	glClearTexImage = (gl_clear_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glClearTexImage");
	glClearTexSubImage = (gl_clear_tex_sub_image_func *)fgl__GetOpenGLProcAddress(state, "glClearTexSubImage");
	glBindBuffersBase = (gl_bind_buffers_base_func *)fgl__GetOpenGLProcAddress(state, "glBindBuffersBase");
	glBindBuffersRange = (gl_bind_buffers_range_func *)fgl__GetOpenGLProcAddress(state, "glBindBuffersRange");
	glBindTextures = (gl_bind_textures_func *)fgl__GetOpenGLProcAddress(state, "glBindTextures");
	glBindSamplers = (gl_bind_samplers_func *)fgl__GetOpenGLProcAddress(state, "glBindSamplers");
	glBindImageTextures = (gl_bind_image_textures_func *)fgl__GetOpenGLProcAddress(state, "glBindImageTextures");
	glBindVertexBuffers = (gl_bind_vertex_buffers_func *)fgl__GetOpenGLProcAddress(state, "glBindVertexBuffers");
#	endif //GL_VERSION_4_4

#	if GL_VERSION_4_5
	glClipControl = (gl_clip_control_func *)fgl__GetOpenGLProcAddress(state, "glClipControl");
	glCreateTransformFeedbacks = (gl_create_transform_feedbacks_func *)fgl__GetOpenGLProcAddress(state, "glCreateTransformFeedbacks");
	glTransformFeedbackBufferBase = (gl_transform_feedback_buffer_base_func *)fgl__GetOpenGLProcAddress(state, "glTransformFeedbackBufferBase");
	glTransformFeedbackBufferRange = (gl_transform_feedback_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glTransformFeedbackBufferRange");
	glGetTransformFeedbackiv = (gl_get_transform_feedbackiv_func *)fgl__GetOpenGLProcAddress(state, "glGetTransformFeedbackiv");
	glGetTransformFeedbacki_v = (gl_get_transform_feedbacki_v_func *)fgl__GetOpenGLProcAddress(state, "glGetTransformFeedbacki_v");
	glGetTransformFeedbacki64_v = (gl_get_transform_feedbacki64_v_func *)fgl__GetOpenGLProcAddress(state, "glGetTransformFeedbacki64_v");
	glCreateBuffers = (gl_create_buffers_func *)fgl__GetOpenGLProcAddress(state, "glCreateBuffers");
	glNamedBufferStorage = (gl_named_buffer_storage_func *)fgl__GetOpenGLProcAddress(state, "glNamedBufferStorage");
	glNamedBufferData = (gl_named_buffer_data_func *)fgl__GetOpenGLProcAddress(state, "glNamedBufferData");
	glNamedBufferSubData = (gl_named_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glNamedBufferSubData");
	glCopyNamedBufferSubData = (gl_copy_named_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glCopyNamedBufferSubData");
	glClearNamedBufferData = (gl_clear_named_buffer_data_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedBufferData");
	glClearNamedBufferSubData = (gl_clear_named_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedBufferSubData");
	glMapNamedBuffer = (gl_map_named_buffer_func *)fgl__GetOpenGLProcAddress(state, "glMapNamedBuffer");
	glMapNamedBufferRange = (gl_map_named_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glMapNamedBufferRange");
	glUnmapNamedBuffer = (gl_unmap_named_buffer_func *)fgl__GetOpenGLProcAddress(state, "glUnmapNamedBuffer");
	glFlushMappedNamedBufferRange = (gl_flush_mapped_named_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glFlushMappedNamedBufferRange");
	glGetNamedBufferParameteriv = (gl_get_named_buffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedBufferParameteriv");
	glGetNamedBufferParameteri64v = (gl_get_named_buffer_parameteri64v_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedBufferParameteri64v");
	glGetNamedBufferPointerv = (gl_get_named_buffer_pointerv_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedBufferPointerv");
	glGetNamedBufferSubData = (gl_get_named_buffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedBufferSubData");
	glCreateFramebuffers = (gl_create_framebuffers_func *)fgl__GetOpenGLProcAddress(state, "glCreateFramebuffers");
	glNamedFramebufferRenderbuffer = (gl_named_framebuffer_renderbuffer_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferRenderbuffer");
	glNamedFramebufferParameteri = (gl_named_framebuffer_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferParameteri");
	glNamedFramebufferTexture = (gl_named_framebuffer_texture_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferTexture");
	glNamedFramebufferTextureLayer = (gl_named_framebuffer_texture_layer_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferTextureLayer");
	glNamedFramebufferDrawBuffer = (gl_named_framebuffer_draw_buffer_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferDrawBuffer");
	glNamedFramebufferDrawBuffers = (gl_named_framebuffer_draw_buffers_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferDrawBuffers");
	glNamedFramebufferReadBuffer = (gl_named_framebuffer_read_buffer_func *)fgl__GetOpenGLProcAddress(state, "glNamedFramebufferReadBuffer");
	glInvalidateNamedFramebufferData = (gl_invalidate_named_framebuffer_data_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateNamedFramebufferData");
	glInvalidateNamedFramebufferSubData = (gl_invalidate_named_framebuffer_sub_data_func *)fgl__GetOpenGLProcAddress(state, "glInvalidateNamedFramebufferSubData");
	glClearNamedFramebufferiv = (gl_clear_named_framebufferiv_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedFramebufferiv");
	glClearNamedFramebufferuiv = (gl_clear_named_framebufferuiv_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedFramebufferuiv");
	glClearNamedFramebufferfv = (gl_clear_named_framebufferfv_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedFramebufferfv");
	glClearNamedFramebufferfi = (gl_clear_named_framebufferfi_func *)fgl__GetOpenGLProcAddress(state, "glClearNamedFramebufferfi");
	glBlitNamedFramebuffer = (gl_blit_named_framebuffer_func *)fgl__GetOpenGLProcAddress(state, "glBlitNamedFramebuffer");
	glCheckNamedFramebufferStatus = (gl_check_named_framebuffer_status_func *)fgl__GetOpenGLProcAddress(state, "glCheckNamedFramebufferStatus");
	glGetNamedFramebufferParameteriv = (gl_get_named_framebuffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedFramebufferParameteriv");
	glGetNamedFramebufferAttachmentParameteriv = (gl_get_named_framebuffer_attachment_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedFramebufferAttachmentParameteriv");
	glCreateRenderbuffers = (gl_create_renderbuffers_func *)fgl__GetOpenGLProcAddress(state, "glCreateRenderbuffers");
	glNamedRenderbufferStorage = (gl_named_renderbuffer_storage_func *)fgl__GetOpenGLProcAddress(state, "glNamedRenderbufferStorage");
	glNamedRenderbufferStorageMultisample = (gl_named_renderbuffer_storage_multisample_func *)fgl__GetOpenGLProcAddress(state, "glNamedRenderbufferStorageMultisample");
	glGetNamedRenderbufferParameteriv = (gl_get_named_renderbuffer_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetNamedRenderbufferParameteriv");
	glCreateTextures = (gl_create_textures_func *)fgl__GetOpenGLProcAddress(state, "glCreateTextures");
	glTextureBuffer = (gl_texture_buffer_func *)fgl__GetOpenGLProcAddress(state, "glTextureBuffer");
	glTextureBufferRange = (gl_texture_buffer_range_func *)fgl__GetOpenGLProcAddress(state, "glTextureBufferRange");
	glTextureStorage1D = (gl_texture_storage1d_func *)fgl__GetOpenGLProcAddress(state, "glTextureStorage1D");
	glTextureStorage2D = (gl_texture_storage2d_func *)fgl__GetOpenGLProcAddress(state, "glTextureStorage2D");
	glTextureStorage3D = (gl_texture_storage3d_func *)fgl__GetOpenGLProcAddress(state, "glTextureStorage3D");
	glTextureStorage2DMultisample = (gl_texture_storage2_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTextureStorage2DMultisample");
	glTextureStorage3DMultisample = (gl_texture_storage3_d_multisample_func *)fgl__GetOpenGLProcAddress(state, "glTextureStorage3DMultisample");
	glTextureSubImage1D = (gl_texture_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glTextureSubImage1D");
	glTextureSubImage2D = (gl_texture_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glTextureSubImage2D");
	glTextureSubImage3D = (gl_texture_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glTextureSubImage3D");
	glCompressedTextureSubImage1D = (gl_compressed_texture_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTextureSubImage1D");
	glCompressedTextureSubImage2D = (gl_compressed_texture_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTextureSubImage2D");
	glCompressedTextureSubImage3D = (gl_compressed_texture_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glCompressedTextureSubImage3D");
	glCopyTextureSubImage1D = (gl_copy_texture_sub_image1d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTextureSubImage1D");
	glCopyTextureSubImage2D = (gl_copy_texture_sub_image2d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTextureSubImage2D");
	glCopyTextureSubImage3D = (gl_copy_texture_sub_image3d_func *)fgl__GetOpenGLProcAddress(state, "glCopyTextureSubImage3D");
	glTextureParameterf = (gl_texture_parameterf_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameterf");
	glTextureParameterfv = (gl_texture_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameterfv");
	glTextureParameteri = (gl_texture_parameteri_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameteri");
	glTextureParameterIiv = (gl_texture_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameterIiv");
	glTextureParameterIuiv = (gl_texture_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameterIuiv");
	glTextureParameteriv = (gl_texture_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glTextureParameteriv");
	glGenerateTextureMipmap = (gl_generate_texture_mipmap_func *)fgl__GetOpenGLProcAddress(state, "glGenerateTextureMipmap");
	glBindTextureUnit = (gl_bind_texture_unit_func *)fgl__GetOpenGLProcAddress(state, "glBindTextureUnit");
	glGetTextureImage = (gl_get_texture_image_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureImage");
	glGetCompressedTextureImage = (gl_get_compressed_texture_image_func *)fgl__GetOpenGLProcAddress(state, "glGetCompressedTextureImage");
	glGetTextureLevelParameterfv = (gl_get_texture_level_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureLevelParameterfv");
	glGetTextureLevelParameteriv = (gl_get_texture_level_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureLevelParameteriv");
	glGetTextureParameterfv = (gl_get_texture_parameterfv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureParameterfv");
	glGetTextureParameterIiv = (gl_get_texture_parameter_iiv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureParameterIiv");
	glGetTextureParameterIuiv = (gl_get_texture_parameter_iuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureParameterIuiv");
	glGetTextureParameteriv = (gl_get_texture_parameteriv_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureParameteriv");
	glCreateVertexArrays = (gl_create_vertex_arrays_func *)fgl__GetOpenGLProcAddress(state, "glCreateVertexArrays");
	glDisableVertexArrayAttrib = (gl_disable_vertex_array_attrib_func *)fgl__GetOpenGLProcAddress(state, "glDisableVertexArrayAttrib");
	glEnableVertexArrayAttrib = (gl_enable_vertex_array_attrib_func *)fgl__GetOpenGLProcAddress(state, "glEnableVertexArrayAttrib");
	glVertexArrayElementBuffer = (gl_vertex_array_element_buffer_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayElementBuffer");
	glVertexArrayVertexBuffer = (gl_vertex_array_vertex_buffer_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayVertexBuffer");
	glVertexArrayVertexBuffers = (gl_vertex_array_vertex_buffers_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayVertexBuffers");
	glVertexArrayAttribBinding = (gl_vertex_array_attrib_binding_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayAttribBinding");
	glVertexArrayAttribFormat = (gl_vertex_array_attrib_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayAttribFormat");
	glVertexArrayAttribIFormat = (gl_vertex_array_attrib_i_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayAttribIFormat");
	glVertexArrayAttribLFormat = (gl_vertex_array_attrib_l_format_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayAttribLFormat");
	glVertexArrayBindingDivisor = (gl_vertex_array_binding_divisor_func *)fgl__GetOpenGLProcAddress(state, "glVertexArrayBindingDivisor");
	glGetVertexArrayiv = (gl_get_vertex_arrayiv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexArrayiv");
	glGetVertexArrayIndexediv = (gl_get_vertex_array_indexediv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexArrayIndexediv");
	glGetVertexArrayIndexed64iv = (gl_get_vertex_array_indexed64iv_func *)fgl__GetOpenGLProcAddress(state, "glGetVertexArrayIndexed64iv");
	glCreateSamplers = (gl_create_samplers_func *)fgl__GetOpenGLProcAddress(state, "glCreateSamplers");
	glCreateProgramPipelines = (gl_create_program_pipelines_func *)fgl__GetOpenGLProcAddress(state, "glCreateProgramPipelines");
	glCreateQueries = (gl_create_queries_func *)fgl__GetOpenGLProcAddress(state, "glCreateQueries");
	glGetQueryBufferObjecti64v = (gl_get_query_buffer_objecti64v_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryBufferObjecti64v");
	glGetQueryBufferObjectiv = (gl_get_query_buffer_objectiv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryBufferObjectiv");
	glGetQueryBufferObjectui64v = (gl_get_query_buffer_objectui64v_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryBufferObjectui64v");
	glGetQueryBufferObjectuiv = (gl_get_query_buffer_objectuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetQueryBufferObjectuiv");
	glMemoryBarrierByRegion = (gl_memory_barrier_by_region_func *)fgl__GetOpenGLProcAddress(state, "glMemoryBarrierByRegion");
	glGetTextureSubImage = (gl_get_texture_sub_image_func *)fgl__GetOpenGLProcAddress(state, "glGetTextureSubImage");
	glGetCompressedTextureSubImage = (gl_get_compressed_texture_sub_image_func *)fgl__GetOpenGLProcAddress(state, "glGetCompressedTextureSubImage");
	glGetGraphicsResetStatus = (gl_get_graphics_reset_status_func *)fgl__GetOpenGLProcAddress(state, "glGetGraphicsResetStatus");
	glGetnCompressedTexImage = (gl_getn_compressed_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glGetnCompressedTexImage");
	glGetnTexImage = (gl_getn_tex_image_func *)fgl__GetOpenGLProcAddress(state, "glGetnTexImage");
	glGetnUniformdv = (gl_getn_uniformdv_func *)fgl__GetOpenGLProcAddress(state, "glGetnUniformdv");
	glGetnUniformfv = (gl_getn_uniformfv_func *)fgl__GetOpenGLProcAddress(state, "glGetnUniformfv");
	glGetnUniformiv = (gl_getn_uniformiv_func *)fgl__GetOpenGLProcAddress(state, "glGetnUniformiv");
	glGetnUniformuiv = (gl_getn_uniformuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetnUniformuiv");
	glReadnPixels = (gl_readn_pixels_func *)fgl__GetOpenGLProcAddress(state, "glReadnPixels");
	glGetnMapdv = (gl_getn_mapdv_func *)fgl__GetOpenGLProcAddress(state, "glGetnMapdv");
	glGetnMapfv = (gl_getn_mapfv_func *)fgl__GetOpenGLProcAddress(state, "glGetnMapfv");
	glGetnMapiv = (gl_getn_mapiv_func *)fgl__GetOpenGLProcAddress(state, "glGetnMapiv");
	glGetnPixelMapfv = (gl_getn_pixel_mapfv_func *)fgl__GetOpenGLProcAddress(state, "glGetnPixelMapfv");
	glGetnPixelMapuiv = (gl_getn_pixel_mapuiv_func *)fgl__GetOpenGLProcAddress(state, "glGetnPixelMapuiv");
	glGetnPixelMapusv = (gl_getn_pixel_mapusv_func *)fgl__GetOpenGLProcAddress(state, "glGetnPixelMapusv");
	glGetnPolygonStipple = (gl_getn_polygon_stipple_func *)fgl__GetOpenGLProcAddress(state, "glGetnPolygonStipple");
	glGetnColorTable = (gl_getn_color_table_func *)fgl__GetOpenGLProcAddress(state, "glGetnColorTable");
	glGetnConvolutionFilter = (gl_getn_convolution_filter_func *)fgl__GetOpenGLProcAddress(state, "glGetnConvolutionFilter");
	glGetnSeparableFilter = (gl_getn_separable_filter_func *)fgl__GetOpenGLProcAddress(state, "glGetnSeparableFilter");
	glGetnHistogram = (gl_getn_histogram_func *)fgl__GetOpenGLProcAddress(state, "glGetnHistogram");
	glGetnMinmax = (gl_getn_minmax_func *)fgl__GetOpenGLProcAddress(state, "glGetnMinmax");
	glTextureBarrier = (gl_texture_barrier_func *)fgl__GetOpenGLProcAddress(state, "glTextureBarrier");
#	endif //GL_VERSION_4_5
}

static bool fgl__LoadOpenGL(fglOpenGLState *state) {
	assert(state != fgl_null);
#if defined(FGL_PLATFORM_WIN32)
	if(!fgl__Win32LoadOpenGL(&state->win32)) {
		return false;
	}
#elif defined(FGL_PLATFORM_POSIX)
	if(!fgl__PosixLoadOpenGL(&state->posix)) {
		return false;
	}
#endif
	state->isLoaded = true;
	return(true);
}

static void fgl__UnloadOpenGL(fglOpenGLState *state) {
	assert(state != fgl_null);
	if(state->isLoaded) {
#if defined(FGL_PLATFORM_WIN32)
		fgl__Win32UnloadOpenGL(&state->win32);
#elif defined(FGL_PLATFORM_POSIX)
		fgl__PosixUnloadOpenGL(&state->posix);
#endif
	}
	fgl__ClearMemory(state, sizeof(*state));
}

static void fgl__DestroyOpenGLContext(fglOpenGLState *state, fglOpenGLContext *context) {
	assert(state != fgl_null);
	assert(context != fgl_null);
	if(!state->isLoaded) {
		fgl__SetLastError("OpenGL library was not loaded!");
		return;
	}
#if defined(FGL_PLATFORM_WIN32)
	fgl__Win32DestroyOpenGLContext(&state->win32, context);
#elif defined(FGL_PLATFORM_POSIX)
	fgl__PosixDestroyOpenGLContext(&state->posix, context);
#endif
	fgl__ClearMemory(context, sizeof(*context));
}

static bool fgl__CreateOpenGLContext(fglOpenGLState *state, const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext) {
	if(state == fgl_null) {
		fgl__SetLastError("State is missing!");
		return false;
	}
	if(contextCreationParams == fgl_null) {
		fgl__SetLastError("Context creation params is missing!");
		return false;
	}
	if(outContext == fgl_null) {
		fgl__SetLastError("Out context is missing!");
		return false;
	}
	if(!state->isLoaded) {
		fgl__SetLastError("OpenGL library is not loaded!");
		return false;
	}
	fgl__ClearMemory(outContext, sizeof(*outContext));
#	if defined(FGL_PLATFORM_WIN32)
	if(!fgl__Win32CreateOpenGLContext(&state->win32, contextCreationParams, outContext)) {
		return false;
	}
#	elif defined(FGL_PLATFORM_POSIX)
	if(!fgl__PosixCreateOpenGLContext(&state->posix, contextCreationParams, outContext)) {
		return false;
	}
#	endif
	return (outContext->isValid);
}

fdyngl_api void fglSetDefaultOpenGLContextCreationParameters(fglOpenGLContextCreationParameters *outParams) {
	fgl__ClearMemory(outParams, sizeof(*outParams));
	outParams->majorVersion = 3;
	outParams->minorVersion = 3;
	outParams->profile = fglOpenGLProfileType_LegacyProfile;
}

fdyngl_api bool fglCreateOpenGLContext(const fglOpenGLContextCreationParameters *contextCreationParams, fglOpenGLContext *outContext) {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	if(!state->isLoaded) {
		if(!fgl__LoadOpenGL(state)) {
			assert(fgl__GetStringLen(state->lastError) > 0);
			return false;
		}
	}
	bool result = fgl__CreateOpenGLContext(state, contextCreationParams, outContext);
	if(!result) {
		assert(fgl__GetStringLen(state->lastError) > 0);
	}
	return(result);
}

fdyngl_api void fglDestroyOpenGLContext(fglOpenGLContext *context) {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	fgl__DestroyOpenGLContext(state, context);
}

fdyngl_api void fglLoadOpenGLFunctions() {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	if(state->isLoaded) {
		fgl__LoadOpenGLExtensions(state);
	}
}

fdyngl_api bool fglLoadOpenGL(const bool loadFunctions) {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	if(!state->isLoaded) {
		if(!fgl__LoadOpenGL(state)) {
			assert(fgl__GetStringLen(state->lastError) > 0);
			return false;
		}
	}
	if(loadFunctions) {
		fgl__LoadOpenGLExtensions(state);
	}
	return true;
}

fdyngl_api void fglUnloadOpenGL() {
	fglOpenGLState *state = &fgl__globalOpenGLState;
	fgl__UnloadOpenGL(state);
	assert(!state->isLoaded);
}

fdyngl_api void fglPresentOpenGL(const fglOpenGLContext *context) {
	if(context == fgl_null) {
		return;
	}
	const fglOpenGLState *state = &fgl__globalOpenGLState;
#	if defined(FGL_PLATFORM_WIN32)
	if(context->windowHandle.win32.deviceContext != fgl_null) {
		state->win32.gdi32.SwapBuffers(context->windowHandle.win32.deviceContext);
	}
#	endif
}

fdyngl_api const char *fglGetLastError() {
	const fglOpenGLState *state = &fgl__globalOpenGLState;
	return state->lastError;
}

#endif // FGL_IMPLEMENTATION