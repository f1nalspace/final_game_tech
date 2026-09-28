/***
fui_backend_gl3.h

--- About ---

A render backend for final_ui.h on an OpenGL 3.3 CORE profile: one shader program, one vertex array, a vertex and an index buffer streamed every frame, and a white texel for untextured geometry.

It draws the same colors as fui_backend_gl1.h, without the fixed function pipeline and the client arrays a core context no longer has.
Only the alpha written into the framebuffer differs: it adds up instead of being squared, so an opaque framebuffer stays opaque under a translucent panel.
Texture times vertex color is all the shader does, so text, rectangles and images all go through the same program: untextured commands bind the white texel and the font atlas is swizzled to white with its coverage in alpha.

--- Drawing over a host scene ---

Unlike the GL1 backend this one is meant to draw on top of a scene somebody else owns.
It saves every piece of state it touches and puts it back, so the host does not have to know what an interface pass changes.
It also sets everything a host may have left that would keep its triangles from reaching the framebuffer whole: depth, stencil, culling, polygon mode, primitive restart, rasterizer discard, logic op and color mask.
A clip origin the host moved to the upper left with glClipControl (OpenGL 4.5) is followed, the interface stays upright.
Left to the host: the bound framebuffer and its draw buffers (only the first one gets a defined value), clip distances, alpha to coverage, the sample mask, and in a compatibility profile the fixed function fragment state such as GL_ALPHA_TEST.

--- Getting started ---

- Needs a desktop OpenGL 3.3 context, core or compatibility profile. OpenGL ES and WebGL have neither the swizzle nor glPolygonMode it uses
- Include this AFTER a header that declares the OpenGL 3.3 core entry points (final_dynamic_opengl.h will do)
- Define FUI_GL3_IMPLEMENTATION in ONE translation unit before including it
- Call fuiGL3Init once while the context is current, and fuiGL3Release before the context goes away or before Init runs again on the same backend
- One fuiGL3Backend per context, because a vertex array is never shared between contexts, and every call with that context current on the calling thread
- Fill fuiInput::windowSize with the FRAMEBUFFER size in pixels, the viewport and the scissor come from it
- Upload the font atlas once with fuiGL3UploadFontAtlas, and pass the texture it returns to the fuiFont
- Upload a COLORED sheet -- an icon sheet, a preview image -- with fuiGL3UploadImageRGBA instead
- Works with FUI_USE_16BIT_INDICES as well

--- Textures of your own ---

A texture handed to fuiDrawImage or put into a fuiFont is drawn exactly as it samples, so one of your own has to be:
- a GL_TEXTURE_2D name, which also means FUI_TEXTURE_ID_TYPE stays an integer type
- complete: with only level 0 the min filter must not be a mipmap filter, or it samples black
- a normalized format without sRGB: an sRGB texture is decoded to linear light while GL_FRAMEBUFFER_SRGB is off, and comes out darker
- four channels: a one channel texture draws red, only fuiGL3UploadFontAtlas swizzles its coverage into alpha
A sheet cut into cells and drawn with the linear filter can pick up the edge texel of the cell next to it, so leave a texel of empty space between the cells.

--- Usage ---

	fuiGL3Backend backend;
	if(!fuiGL3Init(&backend)) {
		// backend.errorLog says what failed, for a shader that is the compiler or linker log
	}
	uint32_t atlasTexture = 0;
	fuiGL3UploadFontAtlas(bakedFont.atlasPixels, bakedFont.atlasWidth, bakedFont.atlasHeight, &atlasTexture);
	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, (fuiTextureId)atlasTexture);
	...
	fuiEndFrame(&context);
	fuiGL3Render(&backend, fuiGetDrawData(&context));
	fplVideoFlip();
	...
	fuiGL3DeleteTexture(atlasTexture);
	fuiGL3Release(&backend);

--- sRGB framebuffers ---

The colors of final_ui.h go into the framebuffer as they are and are blended there as they are, which is what the GL1 backend does and how every other demo shows them.
A host that draws its scene with GL_FRAMEBUFFER_SRGB enabled would otherwise get an interface that is encoded once more and looks brighter than anywhere else.
So the interface pass turns GL_FRAMEBUFFER_SRGB OFF for its draw calls and restores it afterwards.

--- License ---

MIT License, Copyright (c) 2017-2026 Torsten Spaete
***/

#ifndef FUI_BACKEND_GL3_INCLUDE_H
#define FUI_BACKEND_GL3_INCLUDE_H

#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Capacity of fuiGL3Backend::errorLog in bytes, including the terminator
#define FUI_GL3_ERROR_LOG_CAPACITY 1024

/**
* @struct fuiGL3Backend
* @brief The OpenGL objects the backend draws with, created by @ref fuiGL3Init.
* @note All fields are public for inspection, but only the backend writes them.
*/
typedef struct fuiGL3Backend {
	//! The one program: texture times vertex color
	uint32_t program;
	//! Holds the vertex layout and the index buffer binding
	uint32_t vertexArray;
	//! Streamed every frame from fuiDrawData::vertices
	uint32_t vertexBuffer;
	//! Streamed every frame from fuiDrawData::indices
	uint32_t indexBuffer;
	//! One opaque white texel, bound for untextured commands so the same program draws them
	uint32_t whiteTexture;
	//! Uniform with the matrix from window pixels to clip space
	int32_t locationProjection;
	//! Uniform of the sampler, always texture unit 0
	int32_t locationTexture;
	//! Bytes the vertex buffer holds, it only grows
	size_t vertexBufferCapacity;
	//! Bytes the index buffer holds, it only grows
	size_t indexBufferCapacity;
	//! Why @ref fuiGL3Init failed: the compiler or linker log of a shader, or which object could not be created
	char errorLog[FUI_GL3_ERROR_LOG_CAPACITY];
	//! True on a 4.3 or newer context, where a host may have enabled GL_PRIMITIVE_RESTART_FIXED_INDEX
	bool hasPrimitiveRestartFixedIndex;
	//! True on a 4.5 or newer context, where a host may have moved the clip origin with glClipControl
	bool canQueryClipOrigin;
	//! True between a successful @ref fuiGL3Init and @ref fuiGL3Release
	bool isInitialized;
} fuiGL3Backend;

/**
* @brief Creates the program, the vertex array, the buffers and the white texel.
* @param[out] backend Reference to the backend @ref fuiGL3Backend to initialize.
* @return Returns true when everything was created, false with the reason in fuiGL3Backend::errorLog otherwise.
* @note Needs a current OpenGL 3.3 core (or compatible) context. Whatever was created before a failure is deleted again.
* @note Starts from a cleared struct, so a backend that is still initialized has to go through @ref fuiGL3Release first or its objects leak.
*/
fui_api bool fuiGL3Init(fuiGL3Backend *backend);

/**
* @brief Deletes everything @ref fuiGL3Init created.
* @param[in,out] backend Reference to the backend @ref fuiGL3Backend.
* @note Textures uploaded through this header are NOT deleted here, they belong to the caller.
* @note Safe to call twice, and on a backend that was zeroed but never initialized.
*/
fui_api void fuiGL3Release(fuiGL3Backend *backend);

/**
* @brief Uploads a one channel coverage atlas as a texture this backend can draw text with.
* @param[in] alphaPixels One byte of coverage per texel, width * height of them.
* @param[in] width Width of the atlas in texels.
* @param[in] height Height of the atlas in texels.
* @param[out] outTexture Receives the OpenGL texture name.
* @return Returns true when the texture was created, false as well when a side is larger than GL_MAX_TEXTURE_SIZE.
* @note The atlas stays one channel on the GPU (GL_R8), a swizzle reads it as white with the coverage in alpha.
*       GL_ALPHA and GL_LUMINANCE_ALPHA, which the GL1 backend uses, do not exist in a core profile.
* @note The texture binding of the active unit, the pixel unpack buffer and the unpack alignment, row length and skips are restored afterwards.
*/
fui_api bool fuiGL3UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture);

/**
* @brief Uploads a four channel image as a texture this backend can draw pictures with.
* @param[in] rgbaPixels Four bytes per texel in the order red, green, blue, alpha, width * height of them.
* @param[in] width Width of the image in texels.
* @param[in] height Height of the image in texels.
* @param[in] useLinearFilter Smooths the image when it is drawn at another size, rather than keeping its texels hard.
* @param[out] outTexture Receives the OpenGL texture name.
* @return Returns true when the texture was created, false as well when a side is larger than GL_MAX_TEXTURE_SIZE.
* @note The alpha is expected STRAIGHT rather than premultiplied, the same blend as in the GL1 backend.
* @note The texture binding of the active unit, the pixel unpack buffer and the unpack alignment, row length and skips are restored afterwards.
*/
fui_api bool fuiGL3UploadImageRGBA(const unsigned char *rgbaPixels, const uint32_t width, const uint32_t height, const bool useLinearFilter, uint32_t *outTexture);

/**
* @brief Deletes a texture created by @ref fuiGL3UploadFontAtlas or @ref fuiGL3UploadImageRGBA.
* @param[in] texture The OpenGL texture name.
*/
fui_api void fuiGL3DeleteTexture(const uint32_t texture);

/**
* @brief Draws one finished frame of user interface into the bound framebuffer.
* @param[in,out] backend Reference to the backend @ref fuiGL3Backend, its buffers grow when a frame needs more.
* @param[in] drawData The draw data from @ref fuiGetDrawData.
* @note Everything it changes is restored afterwards: program, vertex array, array buffer, active texture unit, texture and sampler of unit 0, blending, scissor, depth test, face culling, stencil test, viewport, polygon mode, primitive restart (also the fixed index one from 4.3), rasterizer discard, logic op, color mask and GL_FRAMEBUFFER_SRGB.
*/
fui_api void fuiGL3Render(fuiGL3Backend *backend, const fuiDrawData *drawData);

#ifdef __cplusplus
}
#endif

#endif // FUI_BACKEND_GL3_INCLUDE_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
#if defined(FUI_GL3_IMPLEMENTATION) && !defined(FUI_GL3_IMPLEMENTED)
#define FUI_GL3_IMPLEMENTED

#include <stddef.h>
#include <stdio.h>
#include <string.h>

//! Smallest size a streamed buffer is created with, so the first frames do not grow it again and again
#define FUI__GL3_MINIMUM_BUFFER_BYTES (64u * 1024u)
//! Bytes of a compiler or linker log that are kept, the error log adds the name of the stage in front
#define FUI__GL3_INFO_LOG_CAPACITY 512
//! GL_PRIMITIVE_RESTART_FIXED_INDEX of OpenGL 4.3, spelled out because a loader for 3.3 does not declare it
#define FUI__GL3_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
//! The first version that has GL_PRIMITIVE_RESTART_FIXED_INDEX
#define FUI__GL3_FIXED_INDEX_MAJOR_VERSION 4
#define FUI__GL3_FIXED_INDEX_MINOR_VERSION 3
//! GL_CLIP_ORIGIN of OpenGL 4.5, spelled out because a loader for 3.3 does not declare it
#define FUI__GL3_CLIP_ORIGIN 0x935C
//! The first version that has glClipControl and GL_CLIP_ORIGIN
#define FUI__GL3_CLIP_CONTROL_MAJOR_VERSION 4
#define FUI__GL3_CLIP_CONTROL_MINOR_VERSION 5

// Window pixels to clip space through the same matrix glOrtho(0, width, height, 0, -1, 1) gives the GL1 backend.
// A full matrix times vector puts every vertex on exactly the position the fixed function pipeline computes, a scale and an offset did not: glyph edges came out one or two levels apart.
static const char *fui__GL3VertexSource =
	"#version 330 core\n"
	"layout(location = 0) in vec2 inPosition;\n"
	"layout(location = 1) in vec2 inUV;\n"
	"layout(location = 2) in vec4 inColor;\n"
	"uniform mat4 projection;\n"
	"out vec2 fragmentUV;\n"
	"out vec4 fragmentColor;\n"
	"void main() {\n"
	"	gl_Position = projection * vec4(inPosition, 0.0, 1.0);\n"
	"	fragmentUV = inUV;\n"
	"	fragmentColor = inColor;\n"
	"}\n";

// Texture times color is GL_MODULATE of the fixed function pipeline, the white texel stands in for a disabled texture
static const char *fui__GL3FragmentSource =
	"#version 330 core\n"
	"in vec2 fragmentUV;\n"
	"in vec4 fragmentColor;\n"
	"uniform sampler2D sourceTexture;\n"
	"out vec4 outColor;\n"
	"void main() {\n"
	"	vec4 texel = texture(sourceTexture, fragmentUV);\n"
	"	outColor = texel * fragmentColor;\n"
	"}\n";

//! Everything @ref fuiGL3Render changes, read before and written back after
typedef struct fui__GL3SavedState {
	GLint program;
	GLint vertexArray;
	GLint arrayBuffer;
	GLint activeTexture;
	GLint texture;
	GLint sampler;
	GLint blendSourceRGB;
	GLint blendDestinationRGB;
	GLint blendSourceAlpha;
	GLint blendDestinationAlpha;
	GLint blendEquationRGB;
	GLint blendEquationAlpha;
	GLint scissorBox[4];
	GLint viewport[4];
	GLint polygonMode[2];
	GLboolean colorWriteMask[4];
	GLboolean isBlendEnabled;
	GLboolean isScissorTestEnabled;
	GLboolean isDepthTestEnabled;
	GLboolean isCullFaceEnabled;
	GLboolean isStencilTestEnabled;
	GLboolean isFramebufferSRGBEnabled;
	GLboolean isPrimitiveRestartEnabled;
	GLboolean isPrimitiveRestartFixedIndexEnabled;
	GLboolean isRasterizerDiscardEnabled;
	GLboolean isColorLogicOpEnabled;
} fui__GL3SavedState;

//! The unpack state a texture upload depends on, read before and written back after
typedef struct fui__GL3SavedUnpackState {
	GLint texture;
	GLint pixelUnpackBuffer;
	GLint alignment;
	GLint rowLength;
	GLint skipRows;
	GLint skipPixels;
} fui__GL3SavedUnpackState;

static void fui__GL3SetError(fuiGL3Backend *backend, const char *what, const char *log) {
	snprintf(backend->errorLog, sizeof(backend->errorLog), "%s%s", what, log);
}

static GLuint fui__GL3CompileShader(fuiGL3Backend *backend, const GLenum type, const char *source, const char *name) {
	GLuint shader = glCreateShader(type);
	if(shader == 0) {
		fui__GL3SetError(backend, name, "glCreateShader failed");
		return(0);
	}
	glShaderSource(shader, 1, &source, fui_null);
	glCompileShader(shader);
	GLint compileStatus = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if(compileStatus != GL_TRUE) {
		char log[FUI__GL3_INFO_LOG_CAPACITY] = { 0 };
		glGetShaderInfoLog(shader, (GLsizei)sizeof(log), fui_null, log);
		fui__GL3SetError(backend, name, log);
		glDeleteShader(shader);
		return(0);
	}
	return(shader);
}

static GLuint fui__GL3CreateProgram(fuiGL3Backend *backend) {
	GLuint vertexShader = fui__GL3CompileShader(backend, GL_VERTEX_SHADER, fui__GL3VertexSource, "Vertex shader: ");
	if(vertexShader == 0) {
		return(0);
	}
	GLuint fragmentShader = fui__GL3CompileShader(backend, GL_FRAGMENT_SHADER, fui__GL3FragmentSource, "Fragment shader: ");
	if(fragmentShader == 0) {
		glDeleteShader(vertexShader);
		return(0);
	}

	GLuint program = glCreateProgram();
	if(program != 0) {
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE) {
			char log[FUI__GL3_INFO_LOG_CAPACITY] = { 0 };
			glGetProgramInfoLog(program, (GLsizei)sizeof(log), fui_null, log);
			fui__GL3SetError(backend, "Program: ", log);
			glDeleteProgram(program);
			program = 0;
		} else {
			// The program keeps what it linked, the shader objects are not needed any more
			glDetachShader(program, vertexShader);
			glDetachShader(program, fragmentShader);
		}
	} else {
		fui__GL3SetError(backend, "Program: ", "glCreateProgram failed");
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return(program);
}

static void fui__GL3SaveUnpackState(fui__GL3SavedUnpackState *saved) {
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved->texture);
	glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &saved->pixelUnpackBuffer);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &saved->alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &saved->rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &saved->skipRows);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &saved->skipPixels);
	// Tightly packed rows straight from client memory, whatever the host left behind
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
}

static void fui__GL3RestoreUnpackState(const fui__GL3SavedUnpackState *saved) {
	glBindTexture(GL_TEXTURE_2D, (GLuint)saved->texture);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)saved->pixelUnpackBuffer);
	glPixelStorei(GL_UNPACK_ALIGNMENT, saved->alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, saved->rowLength);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, saved->skipRows);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, saved->skipPixels);
}

//! Checked before an upload rather than through glGetError afterwards, which would also swallow errors the host has not read yet
static bool fui__GL3FitsMaxTextureSize(const uint32_t width, const uint32_t height) {
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	bool widthFits = width <= (uint32_t)maxTextureSize;
	bool heightFits = height <= (uint32_t)maxTextureSize;
	return(widthFits && heightFits);
}

static void fui__GL3SetTextureSampling(const GLint textureFilter) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilter);
	// Clamped, so the edge texel of one glyph never bleeds into the next one and an image does not wrap around
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// One level only, so a texture without mipmaps is complete with any filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

//! Orphans the buffer and writes the frame into the fresh storage, so the driver never waits for the previous frame still reading it
static void fui__GL3StreamBuffer(const GLenum target, size_t *capacity, const void *data, const size_t size) {
	if(size > *capacity) {
		size_t newCapacity = (*capacity > 0) ? *capacity : (size_t)FUI__GL3_MINIMUM_BUFFER_BYTES;
		const size_t largestDoublableCapacity = SIZE_MAX / 2u;
		while(newCapacity < size) {
			// Doubling would wrap around first on a 32 bit build, the exact size is the last step then
			if(newCapacity > largestDoublableCapacity) {
				newCapacity = size;
				break;
			}
			newCapacity *= 2u;
		}
		*capacity = newCapacity;
	}
	glBufferData(target, (GLsizeiptr)*capacity, fui_null, GL_STREAM_DRAW);
	glBufferSubData(target, 0, (GLsizeiptr)size, data);
}

static void fui__GL3SetEnabled(const GLenum capability, const GLboolean isEnabled) {
	if(isEnabled) {
		glEnable(capability);
	} else {
		glDisable(capability);
	}
}

static void fui__GL3SaveState(const fuiGL3Backend *backend, fui__GL3SavedState *saved) {
	glGetIntegerv(GL_CURRENT_PROGRAM, &saved->program);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &saved->vertexArray);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &saved->arrayBuffer);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &saved->activeTexture);
	// The texture and the sampler are per unit, only unit 0 is used
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved->texture);
	glGetIntegerv(GL_SAMPLER_BINDING, &saved->sampler);
	glGetIntegerv(GL_BLEND_SRC_RGB, &saved->blendSourceRGB);
	glGetIntegerv(GL_BLEND_DST_RGB, &saved->blendDestinationRGB);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &saved->blendSourceAlpha);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &saved->blendDestinationAlpha);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &saved->blendEquationRGB);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &saved->blendEquationAlpha);
	glGetIntegerv(GL_SCISSOR_BOX, saved->scissorBox);
	glGetIntegerv(GL_VIEWPORT, saved->viewport);
	// Front and back, a driver that answers with one value leaves the back untouched and it follows the front
	const GLint polygonModeNotWritten = 0;
	saved->polygonMode[0] = GL_FILL;
	saved->polygonMode[1] = polygonModeNotWritten;
	glGetIntegerv(GL_POLYGON_MODE, saved->polygonMode);
	if(saved->polygonMode[1] == polygonModeNotWritten) {
		saved->polygonMode[1] = saved->polygonMode[0];
	}
	glGetBooleanv(GL_COLOR_WRITEMASK, saved->colorWriteMask);
	saved->isBlendEnabled = glIsEnabled(GL_BLEND);
	saved->isScissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
	saved->isDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	saved->isCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	saved->isStencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
	saved->isFramebufferSRGBEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	saved->isPrimitiveRestartEnabled = glIsEnabled(GL_PRIMITIVE_RESTART);
	saved->isPrimitiveRestartFixedIndexEnabled = GL_FALSE;
	if(backend->hasPrimitiveRestartFixedIndex) {
		saved->isPrimitiveRestartFixedIndexEnabled = glIsEnabled(FUI__GL3_PRIMITIVE_RESTART_FIXED_INDEX);
	}
	saved->isRasterizerDiscardEnabled = glIsEnabled(GL_RASTERIZER_DISCARD);
	saved->isColorLogicOpEnabled = glIsEnabled(GL_COLOR_LOGIC_OP);
}

static void fui__GL3RestoreState(const fuiGL3Backend *backend, const fui__GL3SavedState *saved) {
	// A host program deleted while it was current is gone for good once the interface switched away, binding its name again would fail
	GLuint savedProgram = (GLuint)saved->program;
	bool isSavedProgramAlive = (savedProgram == 0) || glIsProgram(savedProgram);
	GLuint restoredProgram = isSavedProgramAlive ? savedProgram : 0;
	glUseProgram(restoredProgram);
	glBindVertexArray((GLuint)saved->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)saved->arrayBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (GLuint)saved->texture);
	glBindSampler(0, (GLuint)saved->sampler);
	glActiveTexture((GLenum)saved->activeTexture);
	glBlendEquationSeparate((GLenum)saved->blendEquationRGB, (GLenum)saved->blendEquationAlpha);
	glBlendFuncSeparate((GLenum)saved->blendSourceRGB, (GLenum)saved->blendDestinationRGB, (GLenum)saved->blendSourceAlpha, (GLenum)saved->blendDestinationAlpha);
	glScissor(saved->scissorBox[0], saved->scissorBox[1], (GLsizei)saved->scissorBox[2], (GLsizei)saved->scissorBox[3]);
	glViewport(saved->viewport[0], saved->viewport[1], (GLsizei)saved->viewport[2], (GLsizei)saved->viewport[3]);
	// Only a compatibility context can have front and back apart, and only there GL_FRONT and GL_BACK are allowed on their own
	if(saved->polygonMode[0] == saved->polygonMode[1]) {
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)saved->polygonMode[0]);
	} else {
		glPolygonMode(GL_FRONT, (GLenum)saved->polygonMode[0]);
		glPolygonMode(GL_BACK, (GLenum)saved->polygonMode[1]);
	}
	glColorMask(saved->colorWriteMask[0], saved->colorWriteMask[1], saved->colorWriteMask[2], saved->colorWriteMask[3]);
	fui__GL3SetEnabled(GL_BLEND, saved->isBlendEnabled);
	fui__GL3SetEnabled(GL_SCISSOR_TEST, saved->isScissorTestEnabled);
	fui__GL3SetEnabled(GL_DEPTH_TEST, saved->isDepthTestEnabled);
	fui__GL3SetEnabled(GL_CULL_FACE, saved->isCullFaceEnabled);
	fui__GL3SetEnabled(GL_STENCIL_TEST, saved->isStencilTestEnabled);
	fui__GL3SetEnabled(GL_FRAMEBUFFER_SRGB, saved->isFramebufferSRGBEnabled);
	fui__GL3SetEnabled(GL_PRIMITIVE_RESTART, saved->isPrimitiveRestartEnabled);
	if(backend->hasPrimitiveRestartFixedIndex) {
		fui__GL3SetEnabled(FUI__GL3_PRIMITIVE_RESTART_FIXED_INDEX, saved->isPrimitiveRestartFixedIndexEnabled);
	}
	fui__GL3SetEnabled(GL_RASTERIZER_DISCARD, saved->isRasterizerDiscardEnabled);
	fui__GL3SetEnabled(GL_COLOR_LOGIC_OP, saved->isColorLogicOpEnabled);
}

static bool fui__GL3IsVersionAtLeast(const GLint majorVersion, const GLint minorVersion, const GLint requiredMajorVersion, const GLint requiredMinorVersion) {
	if(majorVersion != requiredMajorVersion) {
		return(majorVersion > requiredMajorVersion);
	}
	return(minorVersion >= requiredMinorVersion);
}

fui_api bool fuiGL3Init(fuiGL3Backend *backend) {
	if(backend == fui_null) {
		return(false);
	}
	memset(backend, 0, sizeof(*backend));

	backend->program = fui__GL3CreateProgram(backend);
	if(backend->program == 0) {
		return(false);
	}
	backend->locationProjection = glGetUniformLocation(backend->program, "projection");
	backend->locationTexture = glGetUniformLocation(backend->program, "sourceTexture");

	// Asked once here: the enums of 4.3 and 4.5 are invalid below those versions, and NVIDIA hands out exactly 3.3 when 3.3 core is asked for
	GLint majorVersion = 0;
	GLint minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	backend->hasPrimitiveRestartFixedIndex = fui__GL3IsVersionAtLeast(majorVersion, minorVersion, FUI__GL3_FIXED_INDEX_MAJOR_VERSION, FUI__GL3_FIXED_INDEX_MINOR_VERSION);
	backend->canQueryClipOrigin = fui__GL3IsVersionAtLeast(majorVersion, minorVersion, FUI__GL3_CLIP_CONTROL_MAJOR_VERSION, FUI__GL3_CLIP_CONTROL_MINOR_VERSION);

	const GLsizei bufferCount = 2;
	GLuint vertexArray = 0;
	GLuint buffers[2] = { 0, 0 };
	GLuint whiteTexture = 0;
	glGenVertexArrays(1, &vertexArray);
	glGenBuffers(bufferCount, buffers);
	glGenTextures(1, &whiteTexture);
	backend->vertexArray = (uint32_t)vertexArray;
	backend->vertexBuffer = (uint32_t)buffers[0];
	backend->indexBuffer = (uint32_t)buffers[1];
	backend->whiteTexture = (uint32_t)whiteTexture;
	bool hasAllObjects = backend->vertexArray != 0 && backend->vertexBuffer != 0 && backend->indexBuffer != 0 && backend->whiteTexture != 0;
	if(!hasAllObjects) {
		fui__GL3SetError(backend, "Objects: ", "the vertex array, a buffer or the white texture could not be created");
		fuiGL3Release(backend);
		return(false);
	}

	// The layout lives in the vertex array together with the index buffer binding, so a frame only binds the vertex array
	GLint previousVertexArray = 0;
	GLint previousArrayBuffer = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousArrayBuffer);
	glBindVertexArray(backend->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, backend->vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->indexBuffer);
	const GLuint positionLocation = 0;
	const GLuint uvLocation = 1;
	const GLuint colorLocation = 2;
	const GLint positionComponents = 2;
	const GLint uvComponents = 2;
	const GLint colorComponents = 4;
	const GLsizei vertexStride = (GLsizei)sizeof(fuiVertex);
	const void *positionOffset = (const void *)(uintptr_t)offsetof(fuiVertex, position);
	const void *uvOffset = (const void *)(uintptr_t)offsetof(fuiVertex, uv);
	const void *colorOffset = (const void *)(uintptr_t)offsetof(fuiVertex, color);
	glEnableVertexAttribArray(positionLocation);
	glEnableVertexAttribArray(uvLocation);
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(positionLocation, positionComponents, GL_FLOAT, GL_FALSE, vertexStride, positionOffset);
	glVertexAttribPointer(uvLocation, uvComponents, GL_FLOAT, GL_FALSE, vertexStride, uvOffset);
	// The packed color is byte order red, green, blue, alpha, normalized to 0..1 on the way in
	glVertexAttribPointer(colorLocation, colorComponents, GL_UNSIGNED_BYTE, GL_TRUE, vertexStride, colorOffset);
	glBindVertexArray((GLuint)previousVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)previousArrayBuffer);

	fui__GL3SavedUnpackState savedUnpack;
	fui__GL3SaveUnpackState(&savedUnpack);
	const unsigned char whiteTexel[4] = { 255, 255, 255, 255 };
	const GLsizei whiteTextureSize = 1;
	const GLint rgbaRowAlignment = 4;
	glBindTexture(GL_TEXTURE_2D, backend->whiteTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, rgbaRowAlignment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, whiteTextureSize, whiteTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteTexel);
	fui__GL3SetTextureSampling(GL_NEAREST);
	fui__GL3RestoreUnpackState(&savedUnpack);

	backend->isInitialized = true;
	return(true);
}

fui_api void fuiGL3Release(fuiGL3Backend *backend) {
	if(backend == fui_null) {
		return;
	}
	GLuint whiteTexture = (GLuint)backend->whiteTexture;
	GLuint indexBuffer = (GLuint)backend->indexBuffer;
	GLuint vertexBuffer = (GLuint)backend->vertexBuffer;
	GLuint vertexArray = (GLuint)backend->vertexArray;
	if(whiteTexture != 0) {
		glDeleteTextures(1, &whiteTexture);
	}
	if(indexBuffer != 0) {
		glDeleteBuffers(1, &indexBuffer);
	}
	if(vertexBuffer != 0) {
		glDeleteBuffers(1, &vertexBuffer);
	}
	if(vertexArray != 0) {
		glDeleteVertexArrays(1, &vertexArray);
	}
	if(backend->program != 0) {
		glDeleteProgram(backend->program);
	}
	backend->whiteTexture = 0;
	backend->indexBuffer = 0;
	backend->vertexBuffer = 0;
	backend->vertexArray = 0;
	backend->program = 0;
	backend->vertexBufferCapacity = 0;
	backend->indexBufferCapacity = 0;
	backend->isInitialized = false;
}

fui_api bool fuiGL3UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture) {
	if(alphaPixels == fui_null || outTexture == fui_null || width == 0 || height == 0) {
		return(false);
	}
	bool fitsMaxTextureSize = fui__GL3FitsMaxTextureSize(width, height);
	if(!fitsMaxTextureSize) {
		return(false);
	}

	fui__GL3SavedUnpackState savedUnpack;
	fui__GL3SaveUnpackState(&savedUnpack);

	GLuint textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	// One byte per texel, so the rows are byte tight rather than the four byte default
	const GLint byteRowAlignment = 1;
	glPixelStorei(GL_UNPACK_ALIGNMENT, byteRowAlignment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)width, (GLsizei)height, 0, GL_RED, GL_UNSIGNED_BYTE, alphaPixels);
	// Read as white with the coverage in alpha: the vertex color then gives the text color and the coverage its alpha, as with luminance-alpha in the GL1 backend
	const GLint whiteWithCoverageInAlpha[4] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, whiteWithCoverageInAlpha);
	// Linear, because a glyph drawn smaller than it was baked would alias to pieces with nearest
	fui__GL3SetTextureSampling(GL_LINEAR);

	fui__GL3RestoreUnpackState(&savedUnpack);
	*outTexture = (uint32_t)textureName;
	return(textureName != 0);
}

fui_api bool fuiGL3UploadImageRGBA(const unsigned char *rgbaPixels, const uint32_t width, const uint32_t height, const bool useLinearFilter, uint32_t *outTexture) {
	if(rgbaPixels == fui_null || outTexture == fui_null || width == 0 || height == 0) {
		return(false);
	}
	bool fitsMaxTextureSize = fui__GL3FitsMaxTextureSize(width, height);
	if(!fitsMaxTextureSize) {
		return(false);
	}

	fui__GL3SavedUnpackState savedUnpack;
	fui__GL3SaveUnpackState(&savedUnpack);

	GLuint textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	// Four bytes per texel keep every row a multiple of four on its own
	const GLint rgbaRowAlignment = 4;
	glPixelStorei(GL_UNPACK_ALIGNMENT, rgbaRowAlignment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
	// An icon authored bigger than it is drawn wants linear, pixel art wants nearest, so the caller chooses
	GLint textureFilter = useLinearFilter ? GL_LINEAR : GL_NEAREST;
	fui__GL3SetTextureSampling(textureFilter);

	fui__GL3RestoreUnpackState(&savedUnpack);
	*outTexture = (uint32_t)textureName;
	return(textureName != 0);
}

fui_api void fuiGL3DeleteTexture(const uint32_t texture) {
	if(texture != 0) {
		GLuint textureName = (GLuint)texture;
		glDeleteTextures(1, &textureName);
	}
}

fui_api void fuiGL3Render(fuiGL3Backend *backend, const fuiDrawData *drawData) {
	if(backend == fui_null || !backend->isInitialized) {
		return;
	}
	if(drawData == fui_null || drawData->commandCount == 0 || drawData->vertexCount == 0) {
		return;
	}
	GLsizei windowWidth = (GLsizei)drawData->windowSize.x;
	GLsizei windowHeight = (GLsizei)drawData->windowSize.y;
	if(windowWidth <= 0 || windowHeight <= 0) {
		return;
	}

	fui__GL3SavedState saved;
	fui__GL3SaveState(backend, &saved);

	glViewport(0, 0, windowWidth, windowHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);
	// Whatever else a host may have left that keeps the triangles from reaching the framebuffer whole
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_PRIMITIVE_RESTART);
	if(backend->hasPrimitiveRestartFixedIndex) {
		glDisable(FUI__GL3_PRIMITIVE_RESTART_FIXED_INDEX);
	}
	glDisable(GL_RASTERIZER_DISCARD);
	glDisable(GL_COLOR_LOGIC_OP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	// The color blends like in the GL1 backend. The alpha adds up instead of being squared, so an opaque framebuffer stays opaque under a translucent panel.
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	// A clip origin in the upper left (glClipControl) turns clip space upside down, so the projection turns it back. Scissor and viewport stay in window coordinates either way.
	bool isClipOriginUpperLeft = false;
	if(backend->canQueryClipOrigin) {
		GLint clipOrigin = GL_LOWER_LEFT;
		glGetIntegerv(FUI__GL3_CLIP_ORIGIN, &clipOrigin);
		isClipOriginUpperLeft = (clipOrigin == GL_UPPER_LEFT);
	}

	// y-down window pixels to clip space: x' = x * 2 / width - 1 and y' = 1 - y * 2 / height, column major like glOrtho builds it
	const double clipSpaceSize = 2.0;
	const float clipSpaceLeft = -1.0f;
	const float clipSpaceTopLowerLeftOrigin = 1.0f;
	const float depthScale = -1.0f;
	float clipSpaceTop = isClipOriginUpperLeft ? -clipSpaceTopLowerLeftOrigin : clipSpaceTopLowerLeftOrigin;
	double clipSpaceHeight = isClipOriginUpperLeft ? clipSpaceSize : -clipSpaceSize;
	float pixelToClipScaleX = (float)(clipSpaceSize / (double)windowWidth);
	float pixelToClipScaleY = (float)(clipSpaceHeight / (double)windowHeight);
	const float projection[16] = {
		pixelToClipScaleX, 0.0f, 0.0f, 0.0f,
		0.0f, pixelToClipScaleY, 0.0f, 0.0f,
		0.0f, 0.0f, depthScale, 0.0f,
		clipSpaceLeft, clipSpaceTop, 0.0f, 1.0f,
	};
	// Unit 0 without a sampler object, so the filter and the wrap set at upload apply
	const GLuint textureUnit = 0;
	const GLuint noSampler = 0;
	glUseProgram(backend->program);
	glUniformMatrix4fv(backend->locationProjection, 1, GL_FALSE, projection);
	glUniform1i(backend->locationTexture, (GLint)textureUnit);

	glBindVertexArray(backend->vertexArray);
	size_t vertexBytes = (size_t)drawData->vertexCount * sizeof(fuiVertex);
	size_t indexBytes = (size_t)drawData->indexCount * sizeof(fuiDrawIndex);
	glBindBuffer(GL_ARRAY_BUFFER, backend->vertexBuffer);
	fui__GL3StreamBuffer(GL_ARRAY_BUFFER, &backend->vertexBufferCapacity, drawData->vertices, vertexBytes);
	// Bound to the vertex array, which is what keeps it bound for the draw calls
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backend->indexBuffer);
	fui__GL3StreamBuffer(GL_ELEMENT_ARRAY_BUFFER, &backend->indexBufferCapacity, drawData->indices, indexBytes);

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindSampler(textureUnit, noSampler);
	uint32_t boundTexture = (uint32_t)backend->whiteTexture;
	glBindTexture(GL_TEXTURE_2D, backend->whiteTexture);

#if FUI_USE_16BIT_INDICES
	const GLenum indexType = GL_UNSIGNED_SHORT;
#else
	const GLenum indexType = GL_UNSIGNED_INT;
#endif

	// The scissor is set only when it CHANGES, a run of commands inside one panel all carry the same clip. The first command always sets it.
	GLint lastScissorX = 0;
	GLint lastScissorY = 0;
	GLsizei lastScissorWidth = -1;
	GLsizei lastScissorHeight = -1;

	for(uint32_t commandIndex = 0; commandIndex < drawData->commandCount; ++commandIndex) {
		const fuiDrawCommand *command = &drawData->commands[commandIndex];
		if(command->indexCount == 0) {
			continue;
		}

		// The same conversion as in the GL1 backend, the scissor box counts from the BOTTOM of the window
		fuiRect clip = command->clipRect;
		GLint scissorX = (GLint)clip.x;
		GLint scissorY = (GLint)((float)windowHeight - (clip.y + clip.h));
		GLsizei scissorWidth = (GLsizei)clip.w;
		GLsizei scissorHeight = (GLsizei)clip.h;
		if(scissorWidth <= 0 || scissorHeight <= 0) {
			continue;
		}
		bool scissorChanged = (scissorX != lastScissorX) || (scissorY != lastScissorY) || (scissorWidth != lastScissorWidth) || (scissorHeight != lastScissorHeight);
		if(scissorChanged) {
			glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
			lastScissorX = scissorX;
			lastScissorY = scissorY;
			lastScissorWidth = scissorWidth;
			lastScissorHeight = scissorHeight;
		}

		uint32_t commandTexture = (uint32_t)command->texture;
		uint32_t wantedTexture = (commandTexture != 0) ? commandTexture : (uint32_t)backend->whiteTexture;
		if(wantedTexture != boundTexture) {
			glBindTexture(GL_TEXTURE_2D, (GLuint)wantedTexture);
			boundTexture = wantedTexture;
		}

		size_t indexByteOffset = (size_t)command->indexOffset * sizeof(fuiDrawIndex);
		const void *indexPointer = (const void *)(uintptr_t)indexByteOffset;
		glDrawElements(GL_TRIANGLES, (GLsizei)command->indexCount, indexType, indexPointer);
	}

	fui__GL3RestoreState(backend, &saved);
}

#endif // FUI_GL3_IMPLEMENTATION
