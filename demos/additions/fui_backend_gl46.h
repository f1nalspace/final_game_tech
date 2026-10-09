/***
fui_backend_gl46.h

--- About ---

A render backend for final_ui.h on an OpenGL 4.6 CORE profile, built from what OpenGL 4 added rather than from what 3.3 had.
It draws the same pixels as fui_backend_gl3.h and keeps the same promise to a host scene, with far fewer calls per frame:

- Direct state access (4.5): objects are created, filled and configured by name, so neither Init nor an upload rebinds anything the host has bound
- Immutable storage (4.4 buffers, 4.2 textures): a texture is complete the moment it exists, a buffer never moves
- Persistent mapped ring buffers (4.4) with one fence per frame in flight: a frame is copied straight into memory the GPU reads, nothing is orphaned and nothing waits for the previous frame
- Multi draw indirect (4.3) with a base instance (4.2): the commands go out in batches rather than one draw call each
- Clipping in the fragment shader: the clip box of every command travels with it, the shader discards exactly the pixels the scissor would have dropped, so a new clip does not end a batch
- Indexed state (4.1 viewport arrays, 4.0 blending per draw buffer): only viewport 0 and draw buffer 0 are set, every other draw buffer is masked off for the pass
- Explicit locations and bindings in the shaders (4.3, 4.2): nothing is looked up by name

A batch only ends where a command needs another texture than the one bound, and untextured commands join any batch.
All text and all untextured geometry therefore go out in ONE draw call, however many panels and clips they are spread over, as long as no other texture comes in between.

--- The draw record ---

Every drawn command gets one 40 byte record in a buffer that is bound twice.
Its first twenty bytes are the DrawElementsIndirectCommand glMultiDrawElementsIndirect reads, the rest is the clip box and the texture flag, which the vertex shader reads as instanced attributes.
The base instance of every record is its own index, so the instanced attributes of a draw come from the record that started it.

--- Drawing over a host scene ---

Like the GL3 backend this one is meant to draw on top of a scene somebody else owns.
It saves every piece of state it touches and puts it back, so the host does not have to know what an interface pass changes.
It also sets everything a host may have left that would keep its triangles from reaching the framebuffer whole: depth, stencil, culling, polygon mode, primitive restart, rasterizer discard, logic op, color masks, clip distances, alpha to coverage, alpha to one and the sample mask.
A clip origin the host moved to the upper left with glClipControl is followed, the interface stays upright.
Left to the host: the bound framebuffer and its draw buffer mapping, transform feedback, conditional rendering, and in a compatibility profile the fixed function fragment state such as GL_ALPHA_TEST.

--- Frames in flight ---

Every call writes into the next of FUI_GL46_FRAMES_IN_FLIGHT segments of its stream buffers, and a fence after its draw calls guards that segment.
A segment is written again only once its fence has signaled, which with three segments has long happened by then.
A GPU that still has not finished with a segment after one second costs one interface frame: the call draws nothing rather than overwrite what the GPU is reading.
Every call counts as a frame, so a host that draws two interfaces per presented frame may want to define FUI_GL46_FRAMES_IN_FLIGHT higher.

--- Getting started ---

- Needs a desktop OpenGL 4.6 context, core or compatibility profile. OpenGL ES and WebGL have no multi draw indirect
- Include this AFTER a header that declares the OpenGL 4.6 core entry points (final_dynamic_opengl.h will do)
- Define FUI_GL46_IMPLEMENTATION in ONE translation unit before including it
- Call fuiGL46Init once while the context is current, and fuiGL46Release before the context goes away or before Init runs again on the same backend
- One fuiGL46Backend per context, because a vertex array is never shared between contexts, and every call with that context current on the calling thread
- Fill fuiInput::windowSize with the FRAMEBUFFER size in pixels, the viewport and the clip boxes come from it
- Upload the font atlas once with fuiGL46UploadFontAtlas, and pass the texture it returns to the fuiFont
- Upload a COLORED sheet -- an icon sheet, a preview image -- with fuiGL46UploadImageRGBA instead
- Works with FUI_USE_16BIT_INDICES and with fuiSetDrawBatching either way

--- Textures of your own ---

A texture handed to fuiDrawImage or put into a fuiFont is drawn exactly as it samples, so one of your own has to be:
- a GL_TEXTURE_2D name, which also means FUI_TEXTURE_ID_TYPE stays an integer type
- complete: a mutable texture with only level 0 must not have a mipmap min filter, or it samples black
- a normalized format without sRGB: an sRGB texture is decoded to linear light while GL_FRAMEBUFFER_SRGB is off, and comes out darker
- four channels: a one channel texture draws red, only fuiGL46UploadFontAtlas swizzles its coverage into alpha
A sheet cut into cells and drawn with the linear filter can pick up the edge texel of the cell next to it, so leave a texel of empty space between the cells.

--- Usage ---

	fuiGL46Backend backend;
	if(!fuiGL46Init(&backend)) {
		// backend.errorLog says what failed: the context version, the compiler or linker log of a shader, or an object
	}
	uint32_t atlasTexture = 0;
	fuiGL46UploadFontAtlas(bakedFont.atlasPixels, bakedFont.atlasWidth, bakedFont.atlasHeight, &atlasTexture);
	fuiFont font = fuiStbttFontToFuiFont(&bakedFont, (fuiTextureId)atlasTexture);
	...
	fuiEndFrame(&context);
	fuiGL46Render(&backend, fuiGetDrawData(&context));
	fplVideoFlip();
	...
	fuiGL46DeleteTexture(atlasTexture);
	fuiGL46Release(&backend);

--- sRGB framebuffers ---

The colors of final_ui.h go into the framebuffer as they are and are blended there as they are, which is what the GL1 and GL3 backends do and how every other demo shows them.
So the interface pass turns GL_FRAMEBUFFER_SRGB OFF for its draw calls and restores it afterwards, the same as the GL3 backend.

--- License ---

MIT License, Copyright (c) 2017-2026 Torsten Spaete
***/

#ifndef FUI_BACKEND_GL46_INCLUDE_H
#define FUI_BACKEND_GL46_INCLUDE_H

#include <final_ui.h>

#ifdef __cplusplus
extern "C" {
#endif

//! Capacity of fuiGL46Backend::errorLog in bytes, including the terminator
#define FUI_GL46_ERROR_LOG_CAPACITY 1024

#if !defined(FUI_GL46_FRAMES_IN_FLIGHT)
//! Frames the CPU may write ahead of the GPU, each one has its own segment in every stream buffer
#	define FUI_GL46_FRAMES_IN_FLIGHT 3
#endif

/**
* @struct fuiGL46StreamBuffer
* @brief A buffer that stays mapped for its whole life, cut into one segment per frame in flight.
*/
typedef struct fuiGL46StreamBuffer {
	//! Persistent, coherent and write only mapping of all segments, never read from
	unsigned char *mapped;
	//! Bytes of one segment, it only grows
	size_t segmentBytes;
	//! The buffer name, its immutable storage holds FUI_GL46_FRAMES_IN_FLIGHT segments
	uint32_t buffer;
} fuiGL46StreamBuffer;

/**
* @struct fuiGL46Backend
* @brief The OpenGL objects the backend draws with, created by @ref fuiGL46Init.
* @note All fields are public for inspection, but only the backend writes them.
*/
typedef struct fuiGL46Backend {
	//! The vertices of a frame, copied from fuiDrawData::vertices
	fuiGL46StreamBuffer vertexStream;
	//! The indices of a frame, copied from fuiDrawData::indices
	fuiGL46StreamBuffer indexStream;
	//! One draw record per drawn command, read as indirect draw commands and as instanced attributes
	fuiGL46StreamBuffer recordStream;
	//! One GLsync per segment, set after the draw calls of the frame that wrote it, null while nothing is pending
	void *frameFences[FUI_GL46_FRAMES_IN_FLIGHT];
	//! The one program: texture times vertex color, clipped to the clip box of the draw record
	uint32_t program;
	//! Holds the per vertex and the per draw layout and the index buffer binding
	uint32_t vertexArray;
	//! Segment the next frame writes into
	uint32_t frameSlot;
	//! Draw buffers whose color mask is saved for the pass, GL_MAX_DRAW_BUFFERS capped at what the saved state holds
	uint32_t savedDrawBufferCount;
	//! Clip distances saved and disabled for the pass, GL_MAX_CLIP_DISTANCES capped at what the saved state holds
	uint32_t savedClipDistanceCount;
	//! Multi draw calls the last @ref fuiGL46Render issued, one per batch
	uint32_t lastDrawCallCount;
	//! Commands the last @ref fuiGL46Render drew, without those that had no indices or an empty clip
	uint32_t lastDrawnCommandCount;
	//! Why @ref fuiGL46Init failed or the last frame was not drawn: the context version, the compiler or linker log of a shader, or which object could not be created
	char errorLog[FUI_GL46_ERROR_LOG_CAPACITY];
	//! True between a successful @ref fuiGL46Init and @ref fuiGL46Release
	bool isInitialized;
} fuiGL46Backend;

/**
* @brief Creates the program, the vertex array and the stream buffers.
* @param[out] backend Reference to the backend @ref fuiGL46Backend to initialize.
* @return Returns true when everything was created, false with the reason in fuiGL46Backend::errorLog otherwise.
* @note Needs a current OpenGL 4.6 core (or compatibility) context, an older one is refused before any 4.x function is called. Whatever was created before a failure is deleted again.
* @note Starts from a cleared struct, so a backend that is still initialized has to go through @ref fuiGL46Release first or its objects leak.
* @note Binds nothing: every object is created and set up through direct state access.
*/
fui_api bool fuiGL46Init(fuiGL46Backend *backend);

/**
* @brief Deletes everything @ref fuiGL46Init created.
* @param[in,out] backend Reference to the backend @ref fuiGL46Backend.
* @note Textures uploaded through this header are NOT deleted here, they belong to the caller.
* @note Does not wait for frames still in flight, the driver keeps their storage until the GPU is done with it.
* @note Safe to call twice, and on a backend that was zeroed but never initialized.
*/
fui_api void fuiGL46Release(fuiGL46Backend *backend);

/**
* @brief Uploads a one channel coverage atlas as a texture this backend can draw text with.
* @param[in] alphaPixels One byte of coverage per texel, width * height of them.
* @param[in] width Width of the atlas in texels.
* @param[in] height Height of the atlas in texels.
* @param[out] outTexture Receives the OpenGL texture name.
* @return Returns true when the texture was created, false as well when a side is larger than GL_MAX_TEXTURE_SIZE.
* @note The atlas stays one channel on the GPU (GL_R8), a swizzle reads it as white with the coverage in alpha.
* @note No texture binding changes. The pixel unpack buffer and the unpack alignment, row length and skips are restored afterwards.
*/
fui_api bool fuiGL46UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture);

/**
* @brief Uploads a four channel image as a texture this backend can draw pictures with.
* @param[in] rgbaPixels Four bytes per texel in the order red, green, blue, alpha, width * height of them.
* @param[in] width Width of the image in texels.
* @param[in] height Height of the image in texels.
* @param[in] useLinearFilter Smooths the image when it is drawn at another size, rather than keeping its texels hard.
* @param[out] outTexture Receives the OpenGL texture name.
* @return Returns true when the texture was created, false as well when a side is larger than GL_MAX_TEXTURE_SIZE.
* @note The alpha is expected STRAIGHT rather than premultiplied, the same blend as in the GL1 and GL3 backends.
* @note No texture binding changes. The pixel unpack buffer and the unpack alignment, row length and skips are restored afterwards.
*/
fui_api bool fuiGL46UploadImageRGBA(const unsigned char *rgbaPixels, const uint32_t width, const uint32_t height, const bool useLinearFilter, uint32_t *outTexture);

/**
* @brief Deletes a texture created by @ref fuiGL46UploadFontAtlas or @ref fuiGL46UploadImageRGBA.
* @param[in] texture The OpenGL texture name.
*/
fui_api void fuiGL46DeleteTexture(const uint32_t texture);

/**
* @brief Draws one finished frame of user interface into the bound framebuffer.
* @param[in,out] backend Reference to the backend @ref fuiGL46Backend, its stream buffers grow when a frame needs more.
* @param[in] drawData The draw data from @ref fuiGetDrawData.
* @note Everything it changes is restored afterwards: program, vertex array, draw indirect buffer, active texture unit, texture and sampler of unit 0, viewport 0, scissor test 0, blending of draw buffer 0, the color masks of all draw buffers, depth test, stencil test, face culling, polygon mode, both primitive restarts, rasterizer discard, logic op, clip distances, alpha to coverage, alpha to one, sample mask and GL_FRAMEBUFFER_SRGB.
* @note Draws nothing when a stream buffer cannot grow, or when the GPU has not released the segment of this frame within a second, fuiGL46Backend::errorLog says which.
*/
fui_api void fuiGL46Render(fuiGL46Backend *backend, const fuiDrawData *drawData);

#ifdef __cplusplus
}
#endif

#endif // FUI_BACKEND_GL46_INCLUDE_H

// ****************************************************************************
//
// > IMPLEMENTATION
//
// ****************************************************************************
#if defined(FUI_GL46_IMPLEMENTATION) && !defined(FUI_GL46_IMPLEMENTED)
#define FUI_GL46_IMPLEMENTED

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//! The context version fuiGL46Init asks for
#define FUI__GL46_REQUIRED_MAJOR_VERSION 4
#define FUI__GL46_REQUIRED_MINOR_VERSION 6
//! Smallest segment a stream buffer is created with, so the first frames do not grow it again and again
#define FUI__GL46_MINIMUM_SEGMENT_BYTES (64u * 1024u)
//! Every segment starts on a multiple of this, more than the vertex, index and indirect offsets need
#define FUI__GL46_SEGMENT_ALIGNMENT ((size_t)256u)
//! How long a frame waits for the GPU to release its segment before it is skipped, one second in nanoseconds
#define FUI__GL46_FENCE_TIMEOUT_NANOSECONDS 1000000000ull
//! Bytes of a compiler or linker log that are kept, the error log adds the name of the stage in front
#define FUI__GL46_INFO_LOG_CAPACITY 512
//! Draw buffers and clip distances the saved state has room for, the minimum maximum of both is 8
#define FUI__GL46_MAX_SAVED_DRAW_BUFFERS 16
#define FUI__GL46_MAX_SAVED_CLIP_DISTANCES 8
//! Components of a color write mask, of a viewport and of a clip box
#define FUI__GL46_COLOR_MASK_COMPONENTS 4
#define FUI__GL46_VIEWPORT_COMPONENTS 4
#define FUI__GL46_CLIP_BOX_COMPONENTS 4

// Vertex attribute locations, binding points and the uniform location, the shaders below use the same numbers
#define FUI__GL46_LOCATION_POSITION 0
#define FUI__GL46_LOCATION_UV 1
#define FUI__GL46_LOCATION_COLOR 2
#define FUI__GL46_LOCATION_CLIP_BOX 3
#define FUI__GL46_LOCATION_IS_TEXTURED 4
#define FUI__GL46_VERTEX_BINDING 0
#define FUI__GL46_RECORD_BINDING 1
#define FUI__GL46_UNIFORM_LOCATION_PROJECTION 0
#define FUI__GL46_TEXTURE_UNIT 0

//! Labels the objects carry in a debugger such as RenderDoc and in debug output
#define FUI__GL46_LABEL_PROGRAM "fuiGL46 program"
#define FUI__GL46_LABEL_VERTEX_ARRAY "fuiGL46 vertex array"
#define FUI__GL46_LABEL_VERTEX_STREAM "fuiGL46 vertices"
#define FUI__GL46_LABEL_INDEX_STREAM "fuiGL46 indices"
#define FUI__GL46_LABEL_RECORD_STREAM "fuiGL46 draw records"
#define FUI__GL46_LABEL_FONT_ATLAS "fuiGL46 font atlas"
#define FUI__GL46_LABEL_IMAGE "fuiGL46 image"
//! The length argument of glObjectLabel for a terminated string
#define FUI__GL46_TERMINATED_LABEL (-1)

/**
* One per drawn command, in the buffer that is bound as GL_DRAW_INDIRECT_BUFFER and as vertex buffer binding 1 at the same time.
* The first five fields are the DrawElementsIndirectCommand of glMultiDrawElementsIndirect, the rest is read by the vertex shader as instanced attributes.
*/
typedef struct fui__GL46DrawRecord {
	//! Indices of the command
	uint32_t indexCount;
	//! Always one, a command is one instance
	uint32_t instanceCount;
	//! Index of the first index in the index buffer, counted from the start of the buffer rather than the segment
	uint32_t firstIndex;
	//! Always zero, the indices are absolute and the vertex binding starts at the segment
	int32_t baseVertex;
	//! The index of this record in its segment, which is where the instanced attributes are read from
	uint32_t baseInstance;
	//! Pixels the command may touch, counted from the bottom left: x and y of the first one, then x and y one past the last
	int32_t clipBox[FUI__GL46_CLIP_BOX_COMPONENTS];
	//! Zero for untextured geometry, the shader does not sample then
	uint32_t isTextured;
} fui__GL46DrawRecord;

// Window pixels to clip space through the same matrix the GL3 backend uses, so every vertex lands on the same position.
// The clip box and the texture flag come from the draw record and are handed on unchanged.
static const char *fui__GL46VertexSource =
	"#version 460 core\n"
	"layout(location = 0) in vec2 inPosition;\n"
	"layout(location = 1) in vec2 inUV;\n"
	"layout(location = 2) in vec4 inColor;\n"
	"layout(location = 3) in ivec4 inClipBox;\n"
	"layout(location = 4) in uint inIsTextured;\n"
	"layout(location = 0) uniform mat4 projection;\n"
	"layout(location = 0) out vec2 fragmentUV;\n"
	"layout(location = 1) out vec4 fragmentColor;\n"
	"layout(location = 2) flat out ivec4 fragmentClipBox;\n"
	"layout(location = 3) flat out uint fragmentIsTextured;\n"
	"void main() {\n"
	"	gl_Position = projection * vec4(inPosition, 0.0, 1.0);\n"
	"	fragmentUV = inUV;\n"
	"	fragmentColor = inColor;\n"
	"	fragmentClipBox = inClipBox;\n"
	"	fragmentIsTextured = inIsTextured;\n"
	"}\n";

// Texture times color as in the GL3 backend, with an untextured command multiplying by one instead of sampling a white texel.
// The pixel test is the scissor test: gl_FragCoord is the pixel center, so its integer part is the pixel the scissor box would test.
// It is sampled BEFORE the discard, so the derivatives of the texture lookup never come from a partly discarded quad.
static const char *fui__GL46FragmentSource =
	"#version 460 core\n"
	"layout(location = 0) in vec2 fragmentUV;\n"
	"layout(location = 1) in vec4 fragmentColor;\n"
	"layout(location = 2) flat in ivec4 fragmentClipBox;\n"
	"layout(location = 3) flat in uint fragmentIsTextured;\n"
	"layout(binding = 0) uniform sampler2D sourceTexture;\n"
	"layout(location = 0) out vec4 outColor;\n"
	"void main() {\n"
	"	vec4 texel = vec4(1.0);\n"
	"	if(fragmentIsTextured != 0u) {\n"
	"		texel = texture(sourceTexture, fragmentUV);\n"
	"	}\n"
	"	ivec2 pixel = ivec2(gl_FragCoord.xy);\n"
	"	bvec2 isBeforeClipBox = lessThan(pixel, fragmentClipBox.xy);\n"
	"	bvec2 isPastClipBox = greaterThanEqual(pixel, fragmentClipBox.zw);\n"
	"	if(any(isBeforeClipBox) || any(isPastClipBox)) {\n"
	"		discard;\n"
	"	}\n"
	"	outColor = texel * fragmentColor;\n"
	"}\n";

//! Everything @ref fuiGL46Render changes, read before and written back after
typedef struct fui__GL46SavedState {
	GLfloat viewport[FUI__GL46_VIEWPORT_COMPONENTS];
	GLint program;
	GLint vertexArray;
	GLint drawIndirectBuffer;
	GLint activeTexture;
	GLint texture;
	GLint sampler;
	GLint blendSourceRGB;
	GLint blendDestinationRGB;
	GLint blendSourceAlpha;
	GLint blendDestinationAlpha;
	GLint blendEquationRGB;
	GLint blendEquationAlpha;
	GLint polygonMode[2];
	GLboolean colorWriteMasks[FUI__GL46_MAX_SAVED_DRAW_BUFFERS][FUI__GL46_COLOR_MASK_COMPONENTS];
	GLboolean isClipDistanceEnabled[FUI__GL46_MAX_SAVED_CLIP_DISTANCES];
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
	GLboolean isSampleAlphaToCoverageEnabled;
	GLboolean isSampleAlphaToOneEnabled;
	GLboolean isSampleMaskEnabled;
} fui__GL46SavedState;

//! The unpack state a texture upload depends on, read before and written back after
typedef struct fui__GL46SavedUnpackState {
	GLint pixelUnpackBuffer;
	GLint alignment;
	GLint rowLength;
	GLint skipRows;
	GLint skipPixels;
} fui__GL46SavedUnpackState;

static void fui__GL46SetError(fuiGL46Backend *backend, const char *what, const char *log) {
	snprintf(backend->errorLog, sizeof(backend->errorLog), "%s%s", what, log);
}

static bool fui__GL46IsVersionAtLeast(const GLint majorVersion, const GLint minorVersion, const GLint requiredMajorVersion, const GLint requiredMinorVersion) {
	if(majorVersion != requiredMajorVersion) {
		return(majorVersion > requiredMajorVersion);
	}
	return(minorVersion >= requiredMinorVersion);
}

static GLuint fui__GL46CompileShader(fuiGL46Backend *backend, const GLenum type, const char *source, const char *name) {
	GLuint shader = glCreateShader(type);
	if(shader == 0) {
		fui__GL46SetError(backend, name, "glCreateShader failed");
		return(0);
	}
	glShaderSource(shader, 1, &source, fui_null);
	glCompileShader(shader);
	GLint compileStatus = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if(compileStatus != GL_TRUE) {
		char log[FUI__GL46_INFO_LOG_CAPACITY] = { 0 };
		glGetShaderInfoLog(shader, (GLsizei)sizeof(log), fui_null, log);
		fui__GL46SetError(backend, name, log);
		glDeleteShader(shader);
		return(0);
	}
	return(shader);
}

static GLuint fui__GL46CreateProgram(fuiGL46Backend *backend) {
	GLuint vertexShader = fui__GL46CompileShader(backend, GL_VERTEX_SHADER, fui__GL46VertexSource, "Vertex shader: ");
	if(vertexShader == 0) {
		return(0);
	}
	GLuint fragmentShader = fui__GL46CompileShader(backend, GL_FRAGMENT_SHADER, fui__GL46FragmentSource, "Fragment shader: ");
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
			char log[FUI__GL46_INFO_LOG_CAPACITY] = { 0 };
			glGetProgramInfoLog(program, (GLsizei)sizeof(log), fui_null, log);
			fui__GL46SetError(backend, "Program: ", log);
			glDeleteProgram(program);
			program = 0;
		} else {
			// The program keeps what it linked, the shader objects are not needed any more
			glDetachShader(program, vertexShader);
			glDetachShader(program, fragmentShader);
		}
	} else {
		fui__GL46SetError(backend, "Program: ", "glCreateProgram failed");
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return(program);
}

//! Both vertex layouts live in the one vertex array: the vertices on binding 0, the draw records on binding 1 advancing once per instance
static void fui__GL46SetupVertexArray(const GLuint vertexArray) {
	const GLint positionComponents = 2;
	const GLint uvComponents = 2;
	const GLint colorComponents = 4;
	const GLint isTexturedComponents = 1;
	const GLuint positionOffset = (GLuint)offsetof(fuiVertex, position);
	const GLuint uvOffset = (GLuint)offsetof(fuiVertex, uv);
	const GLuint colorOffset = (GLuint)offsetof(fuiVertex, color);
	const GLuint clipBoxOffset = (GLuint)offsetof(fui__GL46DrawRecord, clipBox);
	const GLuint isTexturedOffset = (GLuint)offsetof(fui__GL46DrawRecord, isTextured);
	const GLuint recordsPerInstance = 1;

	glEnableVertexArrayAttrib(vertexArray, FUI__GL46_LOCATION_POSITION);
	glEnableVertexArrayAttrib(vertexArray, FUI__GL46_LOCATION_UV);
	glEnableVertexArrayAttrib(vertexArray, FUI__GL46_LOCATION_COLOR);
	glEnableVertexArrayAttrib(vertexArray, FUI__GL46_LOCATION_CLIP_BOX);
	glEnableVertexArrayAttrib(vertexArray, FUI__GL46_LOCATION_IS_TEXTURED);

	glVertexArrayAttribFormat(vertexArray, FUI__GL46_LOCATION_POSITION, positionComponents, GL_FLOAT, GL_FALSE, positionOffset);
	glVertexArrayAttribFormat(vertexArray, FUI__GL46_LOCATION_UV, uvComponents, GL_FLOAT, GL_FALSE, uvOffset);
	// The packed color is byte order red, green, blue, alpha, normalized to 0..1 on the way in
	glVertexArrayAttribFormat(vertexArray, FUI__GL46_LOCATION_COLOR, colorComponents, GL_UNSIGNED_BYTE, GL_TRUE, colorOffset);
	// Integer attributes, the I format hands them to the shader unconverted
	glVertexArrayAttribIFormat(vertexArray, FUI__GL46_LOCATION_CLIP_BOX, FUI__GL46_CLIP_BOX_COMPONENTS, GL_INT, clipBoxOffset);
	glVertexArrayAttribIFormat(vertexArray, FUI__GL46_LOCATION_IS_TEXTURED, isTexturedComponents, GL_UNSIGNED_INT, isTexturedOffset);

	glVertexArrayAttribBinding(vertexArray, FUI__GL46_LOCATION_POSITION, FUI__GL46_VERTEX_BINDING);
	glVertexArrayAttribBinding(vertexArray, FUI__GL46_LOCATION_UV, FUI__GL46_VERTEX_BINDING);
	glVertexArrayAttribBinding(vertexArray, FUI__GL46_LOCATION_COLOR, FUI__GL46_VERTEX_BINDING);
	glVertexArrayAttribBinding(vertexArray, FUI__GL46_LOCATION_CLIP_BOX, FUI__GL46_RECORD_BINDING);
	glVertexArrayAttribBinding(vertexArray, FUI__GL46_LOCATION_IS_TEXTURED, FUI__GL46_RECORD_BINDING);
	// One record per instance, and the base instance of a draw picks its record
	glVertexArrayBindingDivisor(vertexArray, FUI__GL46_RECORD_BINDING, recordsPerInstance);
}

static void fui__GL46ReleaseStream(fuiGL46StreamBuffer *stream) {
	GLuint buffer = (GLuint)stream->buffer;
	if(buffer != 0) {
		// Deleting unmaps it, and the driver keeps the storage until the GPU is done with the frames still in flight
		glDeleteBuffers(1, &buffer);
	}
	stream->buffer = 0;
	stream->mapped = fui_null;
	stream->segmentBytes = 0;
}

static bool fui__GL46CreateStream(fuiGL46StreamBuffer *stream, const size_t segmentBytes, const char *label) {
	// GLsizeiptr is signed, so the storage of all segments has to stay below PTRDIFF_MAX
	const size_t largestSegmentBytes = (size_t)PTRDIFF_MAX / FUI_GL46_FRAMES_IN_FLIGHT;
	if(segmentBytes > largestSegmentBytes) {
		return(false);
	}
	size_t storageBytes = segmentBytes * FUI_GL46_FRAMES_IN_FLIGHT;

	GLuint buffer = 0;
	glCreateBuffers(1, &buffer);
	if(buffer == 0) {
		return(false);
	}
	// Coherent, so what the CPU writes reaches the draw calls issued after it without a barrier or a flush
	const GLbitfield storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glNamedBufferStorage(buffer, (GLsizeiptr)storageBytes, fui_null, storageFlags);
	void *mapped = glMapNamedBufferRange(buffer, 0, (GLsizeiptr)storageBytes, storageFlags);
	if(mapped == fui_null) {
		glDeleteBuffers(1, &buffer);
		return(false);
	}
	glObjectLabel(GL_BUFFER, buffer, FUI__GL46_TERMINATED_LABEL, label);

	stream->buffer = (uint32_t)buffer;
	stream->mapped = (unsigned char *)mapped;
	stream->segmentBytes = segmentBytes;
	return(true);
}

//! Makes every segment at least requiredBytes large. A persistent buffer cannot be resized, so a larger one replaces it.
static bool fui__GL46ReserveStream(fuiGL46StreamBuffer *stream, const size_t requiredBytes, const char *label) {
	bool isLargeEnough = (stream->mapped != fui_null) && (requiredBytes <= stream->segmentBytes);
	if(isLargeEnough) {
		return(true);
	}
	size_t newSegmentBytes = (stream->segmentBytes > 0) ? stream->segmentBytes : (size_t)FUI__GL46_MINIMUM_SEGMENT_BYTES;
	const size_t largestDoublableBytes = SIZE_MAX / 2u;
	while(newSegmentBytes < requiredBytes) {
		// Doubling would wrap around first on a 32 bit build, the exact size is the last step then
		if(newSegmentBytes > largestDoublableBytes) {
			newSegmentBytes = requiredBytes;
			break;
		}
		newSegmentBytes *= 2u;
	}
	// Rounded up, so every segment of the buffer starts aligned
	const size_t alignmentMask = FUI__GL46_SEGMENT_ALIGNMENT - 1u;
	const size_t largestAlignableBytes = SIZE_MAX - alignmentMask;
	if(newSegmentBytes > largestAlignableBytes) {
		return(false);
	}
	newSegmentBytes = (newSegmentBytes + alignmentMask) & ~alignmentMask;

	fui__GL46ReleaseStream(stream);
	bool isCreated = fui__GL46CreateStream(stream, newSegmentBytes, label);
	return(isCreated);
}

//! Waits until the GPU is done with the segments of this slot, false when it did not finish in time
static bool fui__GL46WaitForSlot(fuiGL46Backend *backend, const uint32_t slot) {
	GLsync fence = (GLsync)backend->frameFences[slot];
	if(fence == fui_null) {
		return(true);
	}
	// The flush sends the fence on its way, a fence still queued in this context would never signal
	GLenum waitResult = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, FUI__GL46_FENCE_TIMEOUT_NANOSECONDS);
	if(waitResult == GL_TIMEOUT_EXPIRED) {
		return(false);
	}
	// Signaled, or a failed wait that would not get better by waiting again
	glDeleteSync(fence);
	backend->frameFences[slot] = fui_null;
	return(true);
}

static void fui__GL46SaveUnpackState(fui__GL46SavedUnpackState *saved) {
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

static void fui__GL46RestoreUnpackState(const fui__GL46SavedUnpackState *saved) {
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)saved->pixelUnpackBuffer);
	glPixelStorei(GL_UNPACK_ALIGNMENT, saved->alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, saved->rowLength);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, saved->skipRows);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, saved->skipPixels);
}

//! Checked before an upload rather than through glGetError afterwards, which would also swallow errors the host has not read yet
static bool fui__GL46FitsMaxTextureSize(const uint32_t width, const uint32_t height) {
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	bool widthFits = width <= (uint32_t)maxTextureSize;
	bool heightFits = height <= (uint32_t)maxTextureSize;
	return(widthFits && heightFits);
}

//! Creates a texture with one immutable level and fills it, by name and without binding it
static GLuint fui__GL46CreateTexture(const uint32_t width, const uint32_t height, const GLenum internalFormat, const GLenum pixelFormat, const GLint rowAlignment, const void *pixels, const char *label) {
	GLuint texture = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	if(texture == 0) {
		return(0);
	}
	// One level only, so the texture is complete with any filter
	const GLsizei levelCount = 1;
	const GLint baseLevel = 0;
	const GLint originX = 0;
	const GLint originY = 0;
	glTextureStorage2D(texture, levelCount, internalFormat, (GLsizei)width, (GLsizei)height);

	fui__GL46SavedUnpackState savedUnpack;
	fui__GL46SaveUnpackState(&savedUnpack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, rowAlignment);
	glTextureSubImage2D(texture, baseLevel, originX, originY, (GLsizei)width, (GLsizei)height, pixelFormat, GL_UNSIGNED_BYTE, pixels);
	fui__GL46RestoreUnpackState(&savedUnpack);

	glObjectLabel(GL_TEXTURE, texture, FUI__GL46_TERMINATED_LABEL, label);
	return(texture);
}

static void fui__GL46SetTextureSampling(const GLuint texture, const GLint textureFilter) {
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, textureFilter);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, textureFilter);
	// Clamped, so the edge texel of one glyph never bleeds into the next one and an image does not wrap around
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void fui__GL46SetEnabled(const GLenum capability, const GLboolean isEnabled) {
	if(isEnabled) {
		glEnable(capability);
	} else {
		glDisable(capability);
	}
}

static void fui__GL46SetEnabledIndexed(const GLenum capability, const GLuint index, const GLboolean isEnabled) {
	if(isEnabled) {
		glEnablei(capability, index);
	} else {
		glDisablei(capability, index);
	}
}

static void fui__GL46SaveState(const fuiGL46Backend *backend, fui__GL46SavedState *saved) {
	const GLuint firstIndex = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &saved->program);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &saved->vertexArray);
	glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &saved->drawIndirectBuffer);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &saved->activeTexture);
	// The texture and the sampler are per unit and are only read through the active one, only unit 0 is used
	glActiveTexture(GL_TEXTURE0 + FUI__GL46_TEXTURE_UNIT);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &saved->texture);
	glGetIntegerv(GL_SAMPLER_BINDING, &saved->sampler);
	// Only viewport 0 and draw buffer 0 are drawn to, the others keep their own state
	glGetFloati_v(GL_VIEWPORT, firstIndex, saved->viewport);
	saved->isScissorTestEnabled = glIsEnabledi(GL_SCISSOR_TEST, firstIndex);
	saved->isBlendEnabled = glIsEnabledi(GL_BLEND, firstIndex);
	glGetIntegeri_v(GL_BLEND_SRC_RGB, firstIndex, &saved->blendSourceRGB);
	glGetIntegeri_v(GL_BLEND_DST_RGB, firstIndex, &saved->blendDestinationRGB);
	glGetIntegeri_v(GL_BLEND_SRC_ALPHA, firstIndex, &saved->blendSourceAlpha);
	glGetIntegeri_v(GL_BLEND_DST_ALPHA, firstIndex, &saved->blendDestinationAlpha);
	glGetIntegeri_v(GL_BLEND_EQUATION_RGB, firstIndex, &saved->blendEquationRGB);
	glGetIntegeri_v(GL_BLEND_EQUATION_ALPHA, firstIndex, &saved->blendEquationAlpha);
	for(uint32_t drawBufferIndex = 0; drawBufferIndex < backend->savedDrawBufferCount; ++drawBufferIndex) {
		glGetBooleani_v(GL_COLOR_WRITEMASK, drawBufferIndex, saved->colorWriteMasks[drawBufferIndex]);
	}
	for(uint32_t clipDistanceIndex = 0; clipDistanceIndex < backend->savedClipDistanceCount; ++clipDistanceIndex) {
		GLenum clipDistance = GL_CLIP_DISTANCE0 + clipDistanceIndex;
		saved->isClipDistanceEnabled[clipDistanceIndex] = glIsEnabled(clipDistance);
	}
	// Front and back, a driver that answers with one value leaves the back untouched and it follows the front
	const GLint polygonModeNotWritten = 0;
	saved->polygonMode[0] = GL_FILL;
	saved->polygonMode[1] = polygonModeNotWritten;
	glGetIntegerv(GL_POLYGON_MODE, saved->polygonMode);
	if(saved->polygonMode[1] == polygonModeNotWritten) {
		saved->polygonMode[1] = saved->polygonMode[0];
	}
	saved->isDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	saved->isCullFaceEnabled = glIsEnabled(GL_CULL_FACE);
	saved->isStencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
	saved->isFramebufferSRGBEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	saved->isPrimitiveRestartEnabled = glIsEnabled(GL_PRIMITIVE_RESTART);
	saved->isPrimitiveRestartFixedIndexEnabled = glIsEnabled(GL_PRIMITIVE_RESTART_FIXED_INDEX);
	saved->isRasterizerDiscardEnabled = glIsEnabled(GL_RASTERIZER_DISCARD);
	saved->isColorLogicOpEnabled = glIsEnabled(GL_COLOR_LOGIC_OP);
	saved->isSampleAlphaToCoverageEnabled = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
	saved->isSampleAlphaToOneEnabled = glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE);
	saved->isSampleMaskEnabled = glIsEnabled(GL_SAMPLE_MASK);
}

static void fui__GL46RestoreState(const fuiGL46Backend *backend, const fui__GL46SavedState *saved) {
	const GLuint firstIndex = 0;
	// A host program deleted while it was current is gone for good once the interface switched away, binding its name again would fail
	GLuint savedProgram = (GLuint)saved->program;
	bool isSavedProgramAlive = (savedProgram == 0) || glIsProgram(savedProgram);
	GLuint restoredProgram = isSavedProgramAlive ? savedProgram : 0;
	glUseProgram(restoredProgram);
	glBindVertexArray((GLuint)saved->vertexArray);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, (GLuint)saved->drawIndirectBuffer);
	// glBindTextureUnit with zero would unbind every target of the unit, glBindTexture puts back only the 2D one
	glActiveTexture(GL_TEXTURE0 + FUI__GL46_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, (GLuint)saved->texture);
	glBindSampler(FUI__GL46_TEXTURE_UNIT, (GLuint)saved->sampler);
	glActiveTexture((GLenum)saved->activeTexture);
	glViewportIndexedfv(firstIndex, saved->viewport);
	fui__GL46SetEnabledIndexed(GL_SCISSOR_TEST, firstIndex, saved->isScissorTestEnabled);
	fui__GL46SetEnabledIndexed(GL_BLEND, firstIndex, saved->isBlendEnabled);
	glBlendEquationSeparatei(firstIndex, (GLenum)saved->blendEquationRGB, (GLenum)saved->blendEquationAlpha);
	glBlendFuncSeparatei(firstIndex, (GLenum)saved->blendSourceRGB, (GLenum)saved->blendDestinationRGB, (GLenum)saved->blendSourceAlpha, (GLenum)saved->blendDestinationAlpha);
	for(uint32_t drawBufferIndex = 0; drawBufferIndex < backend->savedDrawBufferCount; ++drawBufferIndex) {
		const GLboolean *mask = saved->colorWriteMasks[drawBufferIndex];
		glColorMaski(drawBufferIndex, mask[0], mask[1], mask[2], mask[3]);
	}
	for(uint32_t clipDistanceIndex = 0; clipDistanceIndex < backend->savedClipDistanceCount; ++clipDistanceIndex) {
		GLenum clipDistance = GL_CLIP_DISTANCE0 + clipDistanceIndex;
		fui__GL46SetEnabled(clipDistance, saved->isClipDistanceEnabled[clipDistanceIndex]);
	}
	// Only a compatibility context can have front and back apart, and only there GL_FRONT and GL_BACK are allowed on their own
	if(saved->polygonMode[0] == saved->polygonMode[1]) {
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)saved->polygonMode[0]);
	} else {
		glPolygonMode(GL_FRONT, (GLenum)saved->polygonMode[0]);
		glPolygonMode(GL_BACK, (GLenum)saved->polygonMode[1]);
	}
	fui__GL46SetEnabled(GL_DEPTH_TEST, saved->isDepthTestEnabled);
	fui__GL46SetEnabled(GL_CULL_FACE, saved->isCullFaceEnabled);
	fui__GL46SetEnabled(GL_STENCIL_TEST, saved->isStencilTestEnabled);
	fui__GL46SetEnabled(GL_FRAMEBUFFER_SRGB, saved->isFramebufferSRGBEnabled);
	fui__GL46SetEnabled(GL_PRIMITIVE_RESTART, saved->isPrimitiveRestartEnabled);
	fui__GL46SetEnabled(GL_PRIMITIVE_RESTART_FIXED_INDEX, saved->isPrimitiveRestartFixedIndexEnabled);
	fui__GL46SetEnabled(GL_RASTERIZER_DISCARD, saved->isRasterizerDiscardEnabled);
	fui__GL46SetEnabled(GL_COLOR_LOGIC_OP, saved->isColorLogicOpEnabled);
	fui__GL46SetEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE, saved->isSampleAlphaToCoverageEnabled);
	fui__GL46SetEnabled(GL_SAMPLE_ALPHA_TO_ONE, saved->isSampleAlphaToOneEnabled);
	fui__GL46SetEnabled(GL_SAMPLE_MASK, saved->isSampleMaskEnabled);
}

//! Everything the interface pass needs, on top of whatever the host left
static void fui__GL46SetPassState(const fuiGL46Backend *backend, const GLsizei windowWidth, const GLsizei windowHeight) {
	const GLuint firstIndex = 0;
	const GLuint noSampler = 0;
	const GLfloat viewportX = 0.0f;
	const GLfloat viewportY = 0.0f;
	GLfloat viewportWidth = (GLfloat)windowWidth;
	GLfloat viewportHeight = (GLfloat)windowHeight;
	glViewportIndexedf(firstIndex, viewportX, viewportY, viewportWidth, viewportHeight);
	// The clip boxes are tested in the fragment shader, the scissor would only cut away what the next box allows again
	glDisablei(GL_SCISSOR_TEST, firstIndex);
	glEnablei(GL_BLEND, firstIndex);
	glBlendEquationSeparatei(firstIndex, GL_FUNC_ADD, GL_FUNC_ADD);
	// The color blends like in the GL1 backend. The alpha adds up instead of being squared, so an opaque framebuffer stays opaque under a translucent panel.
	glBlendFuncSeparatei(firstIndex, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	// Only the first draw buffer gets a color, the shader leaves the others undefined, so they are not written at all
	glColorMaski(firstIndex, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	for(uint32_t drawBufferIndex = 1; drawBufferIndex < backend->savedDrawBufferCount; ++drawBufferIndex) {
		glColorMaski(drawBufferIndex, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
	// The shader writes no clip distance, an enabled one would clip by an undefined value
	for(uint32_t clipDistanceIndex = 0; clipDistanceIndex < backend->savedClipDistanceCount; ++clipDistanceIndex) {
		GLenum clipDistance = GL_CLIP_DISTANCE0 + clipDistanceIndex;
		glDisable(clipDistance);
	}
	// Whatever else a host may have left that keeps the triangles from reaching the framebuffer whole
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_PRIMITIVE_RESTART);
	glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
	glDisable(GL_RASTERIZER_DISCARD);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glDisable(GL_SAMPLE_ALPHA_TO_ONE);
	glDisable(GL_SAMPLE_MASK);
	// Unit 0 without a sampler object, so the filter and the wrap set at upload apply
	glBindSampler(FUI__GL46_TEXTURE_UNIT, noSampler);
	glUseProgram(backend->program);
	glBindVertexArray(backend->vertexArray);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, backend->recordStream.buffer);
}

//! y-down window pixels to clip space, the same matrix as in the GL3 backend, turned upside down for a clip origin in the upper left
static void fui__GL46SetProjection(const fuiGL46Backend *backend, const GLsizei windowWidth, const GLsizei windowHeight) {
	// glClipControl turns clip space upside down, the projection turns it back. Viewport, gl_FragCoord and so the clip boxes stay in window coordinates either way.
	GLint clipOrigin = GL_LOWER_LEFT;
	glGetIntegerv(GL_CLIP_ORIGIN, &clipOrigin);
	bool isClipOriginUpperLeft = (clipOrigin == GL_UPPER_LEFT);

	// x' = x * 2 / width - 1 and y' = 1 - y * 2 / height, column major like glOrtho builds it
	const double clipSpaceSize = 2.0;
	const float clipSpaceLeft = -1.0f;
	const float clipSpaceTopLowerLeftOrigin = 1.0f;
	const float depthScale = -1.0f;
	const GLsizei matrixCount = 1;
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
	glProgramUniformMatrix4fv(backend->program, FUI__GL46_UNIFORM_LOCATION_PROJECTION, matrixCount, GL_FALSE, projection);
}

//! Draws the records of one batch with one call, after binding its texture when it has one
static void fui__GL46DrawBatch(fuiGL46Backend *backend, const GLenum indexType, const size_t recordSegmentOffset, const uint32_t firstRecord, const uint32_t recordCount, const uint32_t batchTexture, uint32_t *boundTexture) {
	if(recordCount == 0) {
		return;
	}
	// A batch of untextured commands samples nothing, whatever is bound may stay
	bool needsTextureBound = (batchTexture != 0) && (batchTexture != *boundTexture);
	if(needsTextureBound) {
		glBindTextureUnit(FUI__GL46_TEXTURE_UNIT, (GLuint)batchTexture);
		*boundTexture = batchTexture;
	}
	size_t firstRecordOffset = recordSegmentOffset + (size_t)firstRecord * sizeof(fui__GL46DrawRecord);
	const void *indirectOffset = (const void *)(uintptr_t)firstRecordOffset;
	const GLsizei recordStride = (GLsizei)sizeof(fui__GL46DrawRecord);
	glMultiDrawElementsIndirect(GL_TRIANGLES, indexType, indirectOffset, (GLsizei)recordCount, recordStride);
	backend->lastDrawCallCount += 1u;
}

fui_api bool fuiGL46Init(fuiGL46Backend *backend) {
	if(backend == fui_null) {
		return(false);
	}
	memset(backend, 0, sizeof(*backend));

	// Asked first, so an older context fails here rather than in a 4.x entry point the loader never found
	GLint majorVersion = 0;
	GLint minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	bool hasRequiredVersion = fui__GL46IsVersionAtLeast(majorVersion, minorVersion, FUI__GL46_REQUIRED_MAJOR_VERSION, FUI__GL46_REQUIRED_MINOR_VERSION);
	if(!hasRequiredVersion) {
		snprintf(backend->errorLog, sizeof(backend->errorLog), "Context: OpenGL %d.%d, the backend needs %d.%d", (int)majorVersion, (int)minorVersion, FUI__GL46_REQUIRED_MAJOR_VERSION, FUI__GL46_REQUIRED_MINOR_VERSION);
		return(false);
	}

	backend->program = fui__GL46CreateProgram(backend);
	if(backend->program == 0) {
		return(false);
	}
	glObjectLabel(GL_PROGRAM, backend->program, FUI__GL46_TERMINATED_LABEL, FUI__GL46_LABEL_PROGRAM);

	GLint maxDrawBuffers = 0;
	GLint maxClipDistances = 0;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	glGetIntegerv(GL_MAX_CLIP_DISTANCES, &maxClipDistances);
	uint32_t drawBufferCount = (maxDrawBuffers > 0) ? (uint32_t)maxDrawBuffers : 0u;
	uint32_t clipDistanceCount = (maxClipDistances > 0) ? (uint32_t)maxClipDistances : 0u;
	backend->savedDrawBufferCount = (drawBufferCount < FUI__GL46_MAX_SAVED_DRAW_BUFFERS) ? drawBufferCount : FUI__GL46_MAX_SAVED_DRAW_BUFFERS;
	backend->savedClipDistanceCount = (clipDistanceCount < FUI__GL46_MAX_SAVED_CLIP_DISTANCES) ? clipDistanceCount : FUI__GL46_MAX_SAVED_CLIP_DISTANCES;

	GLuint vertexArray = 0;
	glCreateVertexArrays(1, &vertexArray);
	backend->vertexArray = (uint32_t)vertexArray;
	if(vertexArray == 0) {
		fui__GL46SetError(backend, "Objects: ", "the vertex array could not be created");
		fuiGL46Release(backend);
		return(false);
	}
	fui__GL46SetupVertexArray(vertexArray);
	glObjectLabel(GL_VERTEX_ARRAY, vertexArray, FUI__GL46_TERMINATED_LABEL, FUI__GL46_LABEL_VERTEX_ARRAY);

	// Created at their smallest now, so a driver that cannot map persistently fails here rather than on the first frame
	const size_t initialBytes = FUI__GL46_MINIMUM_SEGMENT_BYTES;
	bool hasVertexStream = fui__GL46ReserveStream(&backend->vertexStream, initialBytes, FUI__GL46_LABEL_VERTEX_STREAM);
	bool hasIndexStream = fui__GL46ReserveStream(&backend->indexStream, initialBytes, FUI__GL46_LABEL_INDEX_STREAM);
	bool hasRecordStream = fui__GL46ReserveStream(&backend->recordStream, initialBytes, FUI__GL46_LABEL_RECORD_STREAM);
	if(!hasVertexStream || !hasIndexStream || !hasRecordStream) {
		fui__GL46SetError(backend, "Objects: ", "a persistently mapped stream buffer could not be created");
		fuiGL46Release(backend);
		return(false);
	}

	backend->isInitialized = true;
	return(true);
}

fui_api void fuiGL46Release(fuiGL46Backend *backend) {
	if(backend == fui_null) {
		return;
	}
	for(uint32_t slot = 0; slot < FUI_GL46_FRAMES_IN_FLIGHT; ++slot) {
		GLsync fence = (GLsync)backend->frameFences[slot];
		if(fence != fui_null) {
			glDeleteSync(fence);
		}
		backend->frameFences[slot] = fui_null;
	}
	fui__GL46ReleaseStream(&backend->recordStream);
	fui__GL46ReleaseStream(&backend->indexStream);
	fui__GL46ReleaseStream(&backend->vertexStream);
	GLuint vertexArray = (GLuint)backend->vertexArray;
	if(vertexArray != 0) {
		glDeleteVertexArrays(1, &vertexArray);
	}
	if(backend->program != 0) {
		glDeleteProgram(backend->program);
	}
	backend->vertexArray = 0;
	backend->program = 0;
	backend->frameSlot = 0;
	backend->isInitialized = false;
}

fui_api bool fuiGL46UploadFontAtlas(const unsigned char *alphaPixels, const uint32_t width, const uint32_t height, uint32_t *outTexture) {
	if(alphaPixels == fui_null || outTexture == fui_null || width == 0 || height == 0) {
		return(false);
	}
	bool fitsMaxTextureSize = fui__GL46FitsMaxTextureSize(width, height);
	if(!fitsMaxTextureSize) {
		return(false);
	}

	// One byte per texel, so the rows are byte tight rather than the four byte default
	const GLint byteRowAlignment = 1;
	GLuint texture = fui__GL46CreateTexture(width, height, GL_R8, GL_RED, byteRowAlignment, alphaPixels, FUI__GL46_LABEL_FONT_ATLAS);
	if(texture == 0) {
		return(false);
	}
	// Read as white with the coverage in alpha: the vertex color then gives the text color and the coverage its alpha, as with luminance-alpha in the GL1 backend
	const GLint whiteWithCoverageInAlpha[4] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
	glTextureParameteriv(texture, GL_TEXTURE_SWIZZLE_RGBA, whiteWithCoverageInAlpha);
	// Linear, because a glyph drawn smaller than it was baked would alias to pieces with nearest
	fui__GL46SetTextureSampling(texture, GL_LINEAR);

	*outTexture = (uint32_t)texture;
	return(true);
}

fui_api bool fuiGL46UploadImageRGBA(const unsigned char *rgbaPixels, const uint32_t width, const uint32_t height, const bool useLinearFilter, uint32_t *outTexture) {
	if(rgbaPixels == fui_null || outTexture == fui_null || width == 0 || height == 0) {
		return(false);
	}
	bool fitsMaxTextureSize = fui__GL46FitsMaxTextureSize(width, height);
	if(!fitsMaxTextureSize) {
		return(false);
	}

	// Four bytes per texel keep every row a multiple of four on its own
	const GLint rgbaRowAlignment = 4;
	GLuint texture = fui__GL46CreateTexture(width, height, GL_RGBA8, GL_RGBA, rgbaRowAlignment, rgbaPixels, FUI__GL46_LABEL_IMAGE);
	if(texture == 0) {
		return(false);
	}
	// An icon authored bigger than it is drawn wants linear, pixel art wants nearest, so the caller chooses
	GLint textureFilter = useLinearFilter ? GL_LINEAR : GL_NEAREST;
	fui__GL46SetTextureSampling(texture, textureFilter);

	*outTexture = (uint32_t)texture;
	return(true);
}

fui_api void fuiGL46DeleteTexture(const uint32_t texture) {
	if(texture != 0) {
		GLuint textureName = (GLuint)texture;
		glDeleteTextures(1, &textureName);
	}
}

fui_api void fuiGL46Render(fuiGL46Backend *backend, const fuiDrawData *drawData) {
	if(backend == fui_null || !backend->isInitialized) {
		return;
	}
	backend->lastDrawCallCount = 0;
	backend->lastDrawnCommandCount = 0;
	backend->errorLog[0] = 0;
	if(drawData == fui_null || drawData->commandCount == 0 || drawData->vertexCount == 0) {
		return;
	}
	GLsizei windowWidth = (GLsizei)drawData->windowSize.x;
	GLsizei windowHeight = (GLsizei)drawData->windowSize.y;
	if(windowWidth <= 0 || windowHeight <= 0) {
		return;
	}

	// Everything up to the copies touches only the backend's own objects, so a frame that is given up leaves no host state behind
	uint32_t slot = backend->frameSlot;
	bool isSlotFree = fui__GL46WaitForSlot(backend, slot);
	if(!isSlotFree) {
		fui__GL46SetError(backend, "Render: ", "the GPU did not release the segment of this frame in time, the frame was skipped");
		return;
	}
	size_t vertexBytes = (size_t)drawData->vertexCount * sizeof(fuiVertex);
	size_t indexBytes = (size_t)drawData->indexCount * sizeof(fuiDrawIndex);
	size_t recordBytes = (size_t)drawData->commandCount * sizeof(fui__GL46DrawRecord);
	bool hasVertexRoom = fui__GL46ReserveStream(&backend->vertexStream, vertexBytes, FUI__GL46_LABEL_VERTEX_STREAM);
	bool hasIndexRoom = fui__GL46ReserveStream(&backend->indexStream, indexBytes, FUI__GL46_LABEL_INDEX_STREAM);
	bool hasRecordRoom = fui__GL46ReserveStream(&backend->recordStream, recordBytes, FUI__GL46_LABEL_RECORD_STREAM);
	if(!hasVertexRoom || !hasIndexRoom || !hasRecordRoom) {
		fui__GL46SetError(backend, "Render: ", "a stream buffer could not grow to the size of this frame, the frame was skipped");
		return;
	}

	size_t vertexSegmentOffset = (size_t)slot * backend->vertexStream.segmentBytes;
	size_t indexSegmentOffset = (size_t)slot * backend->indexStream.segmentBytes;
	size_t recordSegmentOffset = (size_t)slot * backend->recordStream.segmentBytes;
	unsigned char *vertexSegment = backend->vertexStream.mapped + vertexSegmentOffset;
	unsigned char *indexSegment = backend->indexStream.mapped + indexSegmentOffset;
	unsigned char *recordSegment = backend->recordStream.mapped + recordSegmentOffset;
	memcpy(vertexSegment, drawData->vertices, vertexBytes);
	memcpy(indexSegment, drawData->indices, indexBytes);
	fui__GL46DrawRecord *records = (fui__GL46DrawRecord *)recordSegment;

	// The vertex binding starts at the segment, so the absolute indices of the frame need no base vertex. The records start at their segment too, so a base instance is a record index.
	const GLsizei vertexStride = (GLsizei)sizeof(fuiVertex);
	const GLsizei recordStride = (GLsizei)sizeof(fui__GL46DrawRecord);
	glVertexArrayVertexBuffer(backend->vertexArray, FUI__GL46_VERTEX_BINDING, backend->vertexStream.buffer, (GLintptr)vertexSegmentOffset, vertexStride);
	glVertexArrayVertexBuffer(backend->vertexArray, FUI__GL46_RECORD_BINDING, backend->recordStream.buffer, (GLintptr)recordSegmentOffset, recordStride);
	// Set every frame, because a grown stream is a new buffer
	glVertexArrayElementBuffer(backend->vertexArray, backend->indexStream.buffer);
	fui__GL46SetProjection(backend, windowWidth, windowHeight);

	fui__GL46SavedState saved;
	fui__GL46SaveState(backend, &saved);
	fui__GL46SetPassState(backend, windowWidth, windowHeight);

#if FUI_USE_16BIT_INDICES
	const GLenum indexType = GL_UNSIGNED_SHORT;
#else
	const GLenum indexType = GL_UNSIGNED_INT;
#endif
	// The index buffer is not offset like the vertex binding, so the first index of every record counts from the start of the buffer
	uint32_t segmentFirstIndex = (uint32_t)(indexSegmentOffset / sizeof(fuiDrawIndex));
	const uint32_t oneInstance = 1;
	const int32_t noBaseVertex = 0;

	uint32_t recordCount = 0;
	uint32_t batchFirstRecord = 0;
	uint32_t batchTexture = 0;
	uint32_t boundTexture = 0;
	for(uint32_t commandIndex = 0; commandIndex < drawData->commandCount; ++commandIndex) {
		const fuiDrawCommand *command = &drawData->commands[commandIndex];
		if(command->indexCount == 0) {
			continue;
		}

		// The scissor box of the GL1 and GL3 backends, counted from the BOTTOM of the window
		fuiRect clip = command->clipRect;
		GLint clipX = (GLint)clip.x;
		GLint clipY = (GLint)((float)windowHeight - (clip.y + clip.h));
		GLsizei clipWidth = (GLsizei)clip.w;
		GLsizei clipHeight = (GLsizei)clip.h;
		if(clipWidth <= 0 || clipHeight <= 0) {
			continue;
		}

		// An untextured command fits into any batch, a textured one only into a batch without a texture yet or with its own
		uint32_t commandTexture = (uint32_t)command->texture;
		bool isTextured = (commandTexture != 0);
		bool needsOtherTexture = isTextured && (batchTexture != 0) && (commandTexture != batchTexture);
		if(needsOtherTexture) {
			uint32_t batchRecordCount = recordCount - batchFirstRecord;
			fui__GL46DrawBatch(backend, indexType, recordSegmentOffset, batchFirstRecord, batchRecordCount, batchTexture, &boundTexture);
			batchFirstRecord = recordCount;
			batchTexture = 0;
		}
		if(isTextured) {
			batchTexture = commandTexture;
		}

		fui__GL46DrawRecord record;
		record.indexCount = command->indexCount;
		record.instanceCount = oneInstance;
		record.firstIndex = segmentFirstIndex + command->indexOffset;
		record.baseVertex = noBaseVertex;
		record.baseInstance = recordCount;
		record.clipBox[0] = clipX;
		record.clipBox[1] = clipY;
		record.clipBox[2] = clipX + clipWidth;
		record.clipBox[3] = clipY + clipHeight;
		record.isTextured = isTextured ? 1u : 0u;
		// Written whole and in order, the mapping is write combined memory
		records[recordCount] = record;
		recordCount += 1u;
	}
	uint32_t lastBatchRecordCount = recordCount - batchFirstRecord;
	fui__GL46DrawBatch(backend, indexType, recordSegmentOffset, batchFirstRecord, lastBatchRecordCount, batchTexture, &boundTexture);
	backend->lastDrawnCommandCount = recordCount;

	// The segments of this slot are written again only after the GPU has passed this point
	const GLbitfield noFenceFlags = 0;
	GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, noFenceFlags);
	backend->frameFences[slot] = (void *)fence;
	backend->frameSlot = (slot + 1u) % FUI_GL46_FRAMES_IN_FLIGHT;

	fui__GL46RestoreState(backend, &saved);
}

#endif // FUI_GL46_IMPLEMENTATION
