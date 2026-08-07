/*
Name:
	Final Render

Description:
	Contains structures for computing render specific stuff and
	has a render command system.

	This file is part of the final_framework.

Changelog:
	## 2026-08-07
	- Added a free ROTATION angle to sprites: SpriteCommand.rotation / SpriteInstance.rotation (radians, CCW, about the sprite center) plus RenderPushSpriteRotate. RenderPushSprite is unchanged and pushes no rotation. Rotated sprites still batch, unlike the caller-side model-matrix trick they replace

	## 2026-07-19
	- Fixed: A body-allocation failure mid-command now rolls back the whole command (header + partial body) so the command stream can never contain a truncated command (_RenderPushTypes no longer over-counts dataSize on failure)

	## 2026-07-15
	- Added TextureOperationType_SetFilter + RenderPushSetTextureFilter: change an uploaded texture's min/mag filter without re-uploading (runtime nearest<->linear switch)

	## 2026-07-14
	- Added maxTextureSize, vendor, renderer, version fields to RenderCapabilities

	## 2026-07-08
	- Added RenderTarget (framebuffer objects): RenderTargetFormat (RGBA8/R16F/R32F/RG16F/RGBA16F), up to 4 MRT color attachments + optional depth
	- Added deferred RenderPushCreateRenderTarget / RenderPushDestroyRenderTarget (mirrors the shader-op flush) and CommandType_BindRenderTarget / _UnbindRenderTarget
	- Added CommandType_SetBlendMode (Alpha/Additive/Opaque) for light accumulation
	- Extended RenderCapabilities with supportsRenderTargets / supportsFloatTextures / maxColorAttachments / maxDrawBuffers

	## 2026-07-07
	- Added generic (shader-agnostic) shader support: ShaderProgramHandle + deferred RenderPushShaderProgram / RenderPopShaderProgram
	- Added CommandType_UseProgram / CommandType_Uniform (scalar int/float/Vec2/Vec3/Vec4/Mat4 + array variants) and their RenderPush* pushers
	- Added CommandType_BindTexture / CommandType_UnbindTexture (bind a texture to a unit for a bound shader)
	- Added RenderCapabilities (GL/GLSL version, max texture image units, shader support) on RenderState

	## 2026-06-32
	- Added render statistic fields to RenderState (last render build/submit/swap timings)
	- Added types and functions for rendering a batch of sprites (RenderAllocateSpriteBatch / RenderPushSpriteBatch)

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_RENDER_H
#define FINAL_RENDER_H

#include <final_platform_layer.h>
#include <final_memory.h>
#include <final_math.h>
#include <final_fontloader.h>

typedef struct UVRect {
	float uMin;
	float vMin;
	float uMax;
	float vMax;
} UVRect;

fpl_extern_inline UVRect UVRectInit(const float uMin, const float vMin, const float uMax, const float vMax) {
	UVRect result = fplZeroInit;
	result.uMin = uMin;
	result.vMin = vMin;
	result.uMax = uMax;
	result.vMax = vMax;
	return(result);
}

fpl_extern_inline UVRect UVRectDefault() {
	return UVRectInit(0.0f, 0.0f, 1.0f, 1.0f);
}

fpl_extern_inline UVRect UVRectFromTile(const Vec2i imageSize, const Vec2i tileSize, const int border, const Vec2i pos) {
	Vec2f texel = V2fInit(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	int imgX = border + pos.x * tileSize.x + border * pos.x;
	int imgY = border + pos.y * tileSize.y + border * pos.y;
	UVRect result = fplZeroInit;
	result.uMin = imgX * texel.x;
	result.vMin = imgY * texel.y;
	result.uMax = result.uMin + tileSize.x * texel.x;
	result.vMax = result.vMin + tileSize.y * texel.y;
	return(result);
}

fpl_extern_inline UVRect UVRectFromPos(const Vec2i imageSize, const Vec2i partSize, const Vec2i pos) {
	Vec2f texel = V2fInit(1.0f / (float)imageSize.x, 1.0f / (float)imageSize.y);
	UVRect result = fplZeroInit;
	result.uMin = pos.x * texel.x;
	result.vMin = pos.y * texel.y;
	result.uMax = result.uMin + partSize.x * texel.x;
	result.vMax = result.vMin + partSize.y * texel.y;
	return(result);
}

typedef enum TextureOperationType {
	TextureOperationType_None = 0,
	TextureOperationType_Upload,
	TextureOperationType_Release,
	TextureOperationType_SetFilter // re-set an already-uploaded texture's min/mag filter, no re-upload
} TextureOperationType;

typedef void *TextureHandle;

typedef enum TextureFilterType {
	TextureFilterType_Nearest = 0,
	TextureFilterType_Linear
} TextureFilterType;

typedef enum TextureWrapMode {
	TextureWrapMode_Repeat = 0,
	TextureWrapMode_ClampToEdge,
	TextureWrapMode_ClampToBorder,
} TextureWrapMode;

typedef struct TextureOperation {
	TextureHandle *handle;
	const void *data;
	TextureOperationType type;
	TextureFilterType filter;
	TextureWrapMode wrap;
	uint32_t width;
	uint32_t height;
	uint32_t bytesPerPixel;
	bool isTopDown;
	bool isPreMultiplied;
} TextureOperation;

#define MAX_TEXTURE_OPERATION_COUNT 1024
#define MAX_MATRIX_STACK_COUNT 32
#define MAX_SHADER_OPERATION_COUNT 64
#define MAX_RENDER_TARGET_OPERATION_COUNT 64
#define MAX_UNIFORM_NAME_LENGTH 64

// Opaque handle to a linked shader program (wraps a GLuint program id; 0/null = invalid/unsupported).
// The renderer stays generic: it never ships any shaders, it only compiles/uses whatever source the
// caller hands it.
typedef void *ShaderProgramHandle;

typedef enum ShaderOperationType {
	ShaderOperationType_None = 0,
	ShaderOperationType_CreateProgram,
	ShaderOperationType_DestroyProgram
} ShaderOperationType;

// Deferred create/destroy of a shader program, mirroring TextureOperation: the source strings are
// owned by the caller and must stay valid until the operation is flushed at the top of RenderWithOpenGL.
typedef struct ShaderOperation {
	ShaderProgramHandle *handle;
	// Identifies the program in the log. The compile/link happens later, at the flush, far from the
	// call that pushed it -- without a name a driver rejecting one shader out of ten reports only
	// that "a fragment shader" failed, which is not actionable on a machine we cannot reach.
	const char *name;
	const char *vertexSource;
	const char *fragmentSource;
	ShaderOperationType type;
} ShaderOperation;

// A render target (framebuffer object) the game renders INTO instead of the screen: an off-screen
// color buffer (or up to RENDER_TARGET_MAX_COLOR_ATTACHMENTS of them for MRT / deferred shading) plus
// an optional depth buffer. Its color attachments are ordinary TextureHandles, so a later pass can
// sample the result with RenderPushBindTexture. Like Lighting the caller owns the struct; the backend
// only fills the opaque GL ids on the deferred create flush.
#define RENDER_TARGET_MAX_COLOR_ATTACHMENTS 4

// The curated set of attachment formats. RGBA8 is the default color buffer; the float formats back the
// SDF distance field (R16F/R32F), jump-flood seed coords (RG16F) and HDR light accumulation (RGBA16F).
// Float formats require caps.supportsFloatTextures; the backend substitutes RGBA8 when absent.
typedef enum RenderTargetFormat {
	RenderTargetFormat_RGBA8 = 0,
	RenderTargetFormat_R16F,
	RenderTargetFormat_R32F,
	RenderTargetFormat_RG16F,
	RenderTargetFormat_RGBA16F
} RenderTargetFormat;

typedef struct RenderTarget {
	uint32_t width;
	uint32_t height;
	int colorCount;                                                       // 1..RENDER_TARGET_MAX_COLOR_ATTACHMENTS
	RenderTargetFormat formats[RENDER_TARGET_MAX_COLOR_ATTACHMENTS];
	TextureFilterType filter;                                             // filter applied to every color attachment
	bool hasDepth;                                                        // attach a depth renderbuffer (for depth-tested passes)
	TextureHandle colorTextures[RENDER_TARGET_MAX_COLOR_ATTACHMENTS];     // filled on create; sample these later
	void *internalHandle;                                                 // wraps the GL framebuffer id (null = unallocated / failed)
	void *depthHandle;                                                    // wraps the GL depth renderbuffer id (null = none)
} RenderTarget;

typedef enum RenderTargetOperationType {
	RenderTargetOperationType_None = 0,
	RenderTargetOperationType_Create,
	RenderTargetOperationType_Destroy
} RenderTargetOperationType;

// Deferred create/destroy of a render target, mirroring ShaderOperation: the RenderTarget struct is
// owned by the caller and must stay valid until the operation is flushed at the top of RenderWithOpenGL.
typedef struct RenderTargetOperation {
	RenderTarget *target;
	RenderTargetOperationType type;
} RenderTargetOperation;

// Blend state as a persistent command (like UseProgram): stays active until the next SetBlendMode. The
// frame starts at Alpha. Additive is the light-accumulation mode for deferred shading; Opaque overwrites.
typedef enum BlendMode {
	BlendMode_Alpha = 0,   // src_alpha, one_minus_src_alpha (the default everywhere)
	BlendMode_Additive,    // one, one -- sums contributions (light accumulation)
	BlendMode_Opaque       // one, zero -- overwrite the destination
} BlendMode;

// GPU capabilities queried once at startup by the backend, exposed on RenderState so renderer-agnostic
// game code can read them (e.g. to decide whether to enable shader features) without touching GL.
typedef struct RenderCapabilities {
	int glMajor;
	int glMinor;
	int maxTextureImageUnits;
	// Largest texture the GPU accepts, per side (GL_MAX_TEXTURE_SIZE). Worth checking against: the
	// reference scans are ~2480x3509, which a GPU capped at 2048 rejects -- and glTexImage2D fails
	// silently, leaving a blank texture and no reason for it anywhere.
	int maxTextureSize;
	bool supportsShaders;
	// Off-screen framebuffer objects: the FBO entrypoints resolved and the driver is new enough. Gates
	// the whole RenderTarget path (SDF, shadow + deferred lighting buffers).
	bool supportsRenderTargets;
	// Float color-attachment formats (R16F/RG16F/RGBA16F/...). When false the backend downgrades those
	// render-target formats to RGBA8.
	bool supportsFloatTextures;
	// MRT limits (GL_MAX_COLOR_ATTACHMENTS / GL_MAX_DRAW_BUFFERS); attachment usage is clamped to these.
	int maxColorAttachments;
	int maxDrawBuffers;
	char glslVersion[64];
	// Who the driver says it is. Kept here so renderer-agnostic code (a system report, a bug template)
	// can name the GPU without touching GL.
	char vendor[128];
	char renderer[256];
	char version[128];
} RenderCapabilities;

typedef struct RenderState {
	TextureOperation textureOperations[MAX_TEXTURE_OPERATION_COUNT];
	ShaderOperation shaderOperations[MAX_SHADER_OPERATION_COUNT];
	Mat4f matrixStack[MAX_MATRIX_STACK_COUNT];
	RenderTargetOperation renderTargetOperations[MAX_RENDER_TARGET_OPERATION_COUNT];
	RenderCapabilities caps;
	fmemMemoryBlock memory;
	size_t matrixTop;
	size_t textureOperationCount;
	size_t shaderOperationCount;
	size_t renderTargetOperationCount;
	size_t lastMemoryUsage;
	// Arena offset where the command currently being pushed started, used to roll back a partial command on allocation failure
	size_t commandStartMemoryUsed;
	// Last frame's render command buffer push timings (seconds)
	double lastRenderBuildSeconds;
	// Last frame's submit to rendering backend timings (seconds)
	double lastRenderSubmitSeconds;
	// Last frame's swap/flip/v-sync timings (seconds)
	double lastRenderSwapSeconds;
} RenderState;

typedef enum CommandType {
	CommandType_None = 0,
	CommandType_Clear,
	CommandType_Viewport,
	CommandType_Scissor,
	CommandType_Matrix,
	CommandType_Rectangle,
	CommandType_Vertices,
	CommandType_Sprite,
	CommandType_SpriteBatch,
	CommandType_Text,
	CommandType_UseProgram,
	CommandType_Uniform,
	CommandType_BindTexture,
	CommandType_UnbindTexture,
	CommandType_BindRenderTarget,
	CommandType_UnbindRenderTarget,
	CommandType_SetBlendMode
} CommandType;

typedef struct CommandHeader {
	size_t dataSize;
	CommandType type;
} CommandHeader;

typedef enum MatrixMode {
	MatrixMode_Set = 0,
	MatrixMode_Push,
	MatrixMode_Pop
} MatrixMode;

typedef struct MatrixCommand {
	Mat4f mat;
	MatrixMode mode;
} MatrixCommand;

typedef enum ClearFlags {
	ClearFlags_None = 0,
	ClearFlags_Color = 1 << 0,
	ClearFlags_Depth = 1 << 1,
} ClearFlags;
FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

typedef struct ClearCommand {
	Vec4f color;
	ClearFlags flags;
} ClearCommand;

typedef struct ViewportCommand {
	int x;
	int y;
	int w;
	int h;
} ViewportCommand;

typedef struct ScissorCommand {
	int x;
	int y;
	int w;
	int h;
} ScissorCommand;

typedef struct RectangleCommand {
	Vec4f color;
	Vec2f bottomLeft;
	Vec2f size;
	float lineWidth;
	bool isFilled;
} RectangleCommand;

typedef enum DrawMode {
	DrawMode_None,
	DrawMode_Points,
	DrawMode_Lines,
	DrawMode_Triangles,
	DrawMode_Polygon
} DrawMode;

typedef struct VertexAllocation {
	Vec2f *verts;
	size_t *count;
} VertexAllocation;

typedef struct VerticesCommand {
	Vec4f color;
	const Vec2f *verts;
	size_t capacity;
	size_t count;
	DrawMode drawMode;
	float thickness;
	bool isLoop;
} VerticesCommand;

typedef enum SpriteFlags {
	// No flags
	SpriteFlags_None = 0,
	// Flip the U min/max of the UV rectangle
	SpriteFlags_FlipU = 1 << 0,
	// Flip the V min/max of the UV rectangle
	SpriteFlags_FlipV = 1 << 1,
	// Rotate the sprite by 90 degrees clockwise
	SpriteFlags_Rotate_90_CW = 1 << 2,
	// Rotate the sprite by 90 degrees counter-clockwise
	SpriteFlags_Rotate_90_CCW = 1 << 3,
} SpriteFlags;

typedef struct SpriteCommand {
	Vec4f color;
	Vec2f position;
	Vec2f ext;
	Vec2f uvMin;
	Vec2f uvMax;
	TextureHandle texture;
	SpriteFlags flags;
	// Free rotation about the sprite's own center, radians, counter-clockwise. 0 = axis-aligned, which is
	// what everything that never rotates passes. This is applied to the quad's CORNERS, so it composes with
	// the flip and 90-degree flags above rather than replacing them: those permute the UVs, this turns the
	// geometry.
	float rotation;
} SpriteCommand;

// One sprite inside a batch -- all instances in a SpriteBatchCommand share the same
// texture, so they collapse into a single draw call (see RenderAllocateSpriteBatch).
typedef struct SpriteInstance {
	Vec4f color;
	Vec2f position;
	Vec2f ext;
	Vec2f uvMin;
	Vec2f uvMax;
	SpriteFlags flags;
	// Same meaning as SpriteCommand.rotation above. RenderAllocateSpriteBatch hands out CLEARED instances,
	// so a filler that never rotates simply leaves this alone.
	float rotation;
} SpriteInstance;

typedef struct SpriteBatchCommand {
	TextureHandle texture;
	SpriteInstance *instances;
	size_t capacity;
	size_t count;
} SpriteBatchCommand;

// Returned by RenderAllocateSpriteBatch: fill instances[0..count) then write *count.
typedef struct SpriteBatchAllocation {
	SpriteInstance *instances;
	size_t *count;
} SpriteBatchAllocation;

typedef struct TextCommand {
	Vec4f color;
	Vec2f position;
	TextureHandle texture;
	const LoadedFont *font;
	float horizontalAlignment;
	float verticalAlignment;
	float maxHeight;
	size_t textLength;
} TextCommand;

// Bind a shader program as persistent state (like the matrix): it stays active for every following
// draw command until the next UseProgram. A null program returns to fixed-function (glUseProgram(0)).
typedef struct UseProgramCommand {
	ShaderProgramHandle program;
} UseProgramCommand;

typedef enum UniformType {
	UniformType_Int = 0,
	UniformType_Float,
	UniformType_Vec2,
	UniformType_Vec3,
	UniformType_Vec4,
	UniformType_Mat4
} UniformType;

// Set a uniform on the currently-bound program. The value payload (count elements of the type) is
// copied inline right after this struct in the command buffer, just like TextCommand's characters.
typedef struct UniformCommand {
	char name[MAX_UNIFORM_NAME_LENGTH];
	UniformType type;
	uint32_t count;
} UniformCommand;

// Bind a texture to a texture unit as persistent state, so a bound shader can sample it (e.g. for
// untextured primitives, or a secondary texture on a higher unit). Independent of the Sprite/Text
// commands, which still bind their own texture to unit 0.
typedef struct BindTextureCommand {
	TextureHandle texture;
	int32_t unit;
} BindTextureCommand;

typedef struct UnbindTextureCommand {
	int32_t unit;
} UnbindTextureCommand;

// Redirect all following draws into an off-screen render target: binds its framebuffer and sets the
// viewport to the target's size. Stays active until UnbindRenderTarget returns to the screen. The
// target pointer stays valid for the frame (the caller owns it), so the backend reads its ids here.
typedef struct BindRenderTargetCommand {
	const RenderTarget *target;
} BindRenderTargetCommand;

// (No payload) restore the default framebuffer. The caller pushes a screen viewport afterwards.
typedef struct UnbindRenderTargetCommand {
	int32_t unused;
} UnbindRenderTargetCommand;

typedef struct SetBlendModeCommand {
	BlendMode mode;
} SetBlendModeCommand;

fpl_extern void RenderInit(RenderState *state, fmemMemoryBlock block);
fpl_extern void RenderReset(RenderState *state);
fpl_extern void RenderPushClear(RenderState *state, const Vec4f color, const ClearFlags flags);
fpl_extern void RenderPushViewport(RenderState *state, const int x, const int y, const int w, const int h);
fpl_extern void RenderPushScissor(RenderState *state, const int x, const int y, const int w, const int h);
fpl_extern void RenderPushMatrix(RenderState *state, const Mat4f *mat, const MatrixMode mode);
fpl_extern void RenderSetMatrix(RenderState *state, const Mat4f *mat);
fpl_extern void RenderPopMatrix(RenderState *state);
fpl_extern void RenderPushRectangle(RenderState *state, const Vec2f bottomLeft, const Vec2f size, const Vec4f color, const bool isFilled, const float lineWidth);
fpl_extern void RenderPushRectangleCenter(RenderState *state, const Vec2f center, const Vec2f ext, const Vec4f color, const bool isFilled, const float lineWidth);
fpl_extern void RenderPushRectangleMinMax(RenderState *state, const Vec2f min, const Vec2f max, const Vec4f color, const bool isFilled, const float lineWidth);
fpl_extern void RenderPushQuad(RenderState *state, const Vec2f center, const float radius, const Vec4f color, const bool isFilled, const float lineWidth);
fpl_extern VertexAllocation RenderAllocateVertices(RenderState *state, const size_t capacity, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness);
fpl_extern void RenderPushVertices(RenderState *state, const Vec2f *verts, const size_t vertexCount, const bool copyVerts, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness);
fpl_extern void RenderPushSprite(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect, const SpriteFlags flags);
// Same sprite, turned by `rotation` radians (counter-clockwise) about its own center. Everything that does
// not rotate keeps calling RenderPushSprite above.
fpl_extern void RenderPushSpriteRotate(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect, const SpriteFlags flags, const float rotation);
fpl_extern SpriteBatchAllocation RenderAllocateSpriteBatch(RenderState *state, const size_t capacity, const TextureHandle texture);
fpl_extern void RenderPushSpriteBatch(RenderState *state, const TextureHandle texture, const SpriteInstance *instances, const size_t count);
fpl_extern void RenderPushTexture(RenderState *state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied);
fpl_extern void RenderPushSetTextureFilter(RenderState *state, TextureHandle *targetTexture, const TextureFilterType filter);
fpl_extern void RenderPopTexture(RenderState *state, TextureHandle *targetTexture);
fpl_extern void RenderPushText(RenderState *state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle texture, const Vec2f position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f color);
fpl_extern void RenderPushCircle(RenderState *state, const Vec2f position, const float radius, const size_t segmentCount, const Vec4f color, const bool isFilled, const float lineWidth);
fpl_extern void RenderPushArc(RenderState *state, const Vec2f position, const float radius, const float startAngle, const float endAngle, const size_t segmentCount, const Vec4f color, const bool isFilled, const bool withClosingLines, const float lineWidth);
fpl_extern void RenderPushLine(RenderState *state, const Vec2f a, const Vec2f b, const Vec4f color, const float lineWidth);

// Shader programs: queued create/destroy (flushed in the backend), plus binding + uniforms as
// commands. The renderer contains no shaders of its own -- the caller supplies the GLSL source.
fpl_extern void RenderPushShaderProgram(RenderState *state, ShaderProgramHandle *handle, const char *name, const char *vertexSource, const char *fragmentSource);
fpl_extern void RenderPopShaderProgram(RenderState *state, ShaderProgramHandle *handle);
fpl_extern void RenderPushUseProgram(RenderState *state, ShaderProgramHandle program);
fpl_extern void RenderPushUniformInt(RenderState *state, const char *name, const int32_t value);
fpl_extern void RenderPushUniformFloat(RenderState *state, const char *name, const float value);
fpl_extern void RenderPushUniformVec2(RenderState *state, const char *name, const Vec2f value);
fpl_extern void RenderPushUniformVec3(RenderState *state, const char *name, const Vec3f value);
fpl_extern void RenderPushUniformVec4(RenderState *state, const char *name, const Vec4f value);
fpl_extern void RenderPushUniformMat4(RenderState *state, const char *name, const Mat4f *value);
fpl_extern void RenderPushUniformFloatArray(RenderState *state, const char *name, const float *values, const size_t count);
fpl_extern void RenderPushUniformVec2Array(RenderState *state, const char *name, const Vec2f *values, const size_t count);
fpl_extern void RenderPushUniformVec3Array(RenderState *state, const char *name, const Vec3f *values, const size_t count);
fpl_extern void RenderPushBindTexture(RenderState *state, const TextureHandle texture, const int32_t unit);
fpl_extern void RenderPushUnbindTexture(RenderState *state, const int32_t unit);

// Render targets (framebuffers): queued create/destroy (flushed in the backend), plus bind/unbind and a
// blend-mode command. Sampling a target's output is just RenderPushBindTexture(target.colorTextures[i]).
fpl_extern void RenderPushCreateRenderTarget(RenderState *state, RenderTarget *target, const uint32_t width, const uint32_t height, const RenderTargetFormat *formats, const int colorCount, const bool hasDepth, const TextureFilterType filter);
fpl_extern void RenderPushDestroyRenderTarget(RenderState *state, RenderTarget *target);
fpl_extern void RenderPushBindRenderTarget(RenderState *state, const RenderTarget *target);
fpl_extern void RenderPushUnbindRenderTarget(RenderState *state);
fpl_extern void RenderPushSetBlendMode(RenderState *state, const BlendMode mode);

#endif // FINAL_RENDER_H

#if (defined(FINAL_RENDER_IMPLEMENTATION) && !defined(FINAL_RENDER_IMPLEMENTED)) || FPL_IS_IDE
#define FINAL_RENDER_IMPLEMENTED

static CommandHeader *_RenderPushHeader(RenderState *state, const CommandType type) {
	if (state == fpl_null) {
		return fpl_null;
	}
	// Remember where this command begins so a later body-allocation failure can roll back the whole command
	state->commandStartMemoryUsed = state->memory.used;
	CommandHeader *result = (CommandHeader *)fmemPush(&state->memory, sizeof(CommandHeader), fmemPushFlags_None);
	if (result == fpl_null) {
		return fpl_null;
	}
	result->type = type;
	result->dataSize = 0;
	return result;
}

static void *_RenderPushTypes(RenderState *state, CommandHeader *header, const size_t count, const size_t typeSize, const bool clear) {
	if (state == fpl_null || header == fpl_null || count == 0 || typeSize == 0) {
		return fpl_null;
	}
	size_t size = typeSize * count;
	void *result = fmemPush(&state->memory, size, clear ? fmemPushFlags_Clear : fmemPushFlags_None);
	if (result == fpl_null) {
		// Roll the whole partial command (header + any earlier body parts) out of the stream so the executor never sees a truncated command
		state->memory.used = state->commandStartMemoryUsed;
		return fpl_null;
	}
	header->dataSize += size;
	return result;
}

#define _RenderPushTypesAs(state, header, count, type, clear) (type*)_RenderPushTypes(state, header, count, sizeof(type), clear)
#define _RenderPushTypeAs(state, header, type, clear) (type*)_RenderPushTypes(state, header, 1, sizeof(type), clear)

fpl_extern void RenderInit(RenderState *state, fmemMemoryBlock block) {
	if (state == fpl_null) {
		return;
	}
	state->memory = block;
	state->textureOperationCount = 0;
	state->shaderOperationCount = 0;
	state->renderTargetOperationCount = 0;
}

fpl_extern void RenderReset(RenderState *state) {
	if (state == fpl_null) {
		return;
	}
	state->lastMemoryUsage = state->memory.used;
	state->memory.used = 0;
}

fpl_extern void RenderPushMatrix(RenderState *state, const Mat4f *mat, const MatrixMode mode) {
	if (state == fpl_null || mat == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Matrix);
	MatrixCommand *cmd = _RenderPushTypeAs(state, header, MatrixCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->mat = *mat;
	cmd->mode = mode;
}

fpl_extern void RenderPopMatrix(RenderState *state) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Matrix);
	MatrixCommand *cmd = _RenderPushTypeAs(state, header, MatrixCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->mode = MatrixMode_Pop;
}

fpl_extern void RenderSetMatrix(RenderState *state, const Mat4f *mat) {
	if (state == fpl_null || mat == fpl_null) {
		return;
	}
	RenderPushMatrix(state, mat, MatrixMode_Set);
}

fpl_extern void RenderPushClear(RenderState *state, const Vec4f color, const ClearFlags flags) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Clear);
	ClearCommand *cmd = _RenderPushTypeAs(state, header, ClearCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->color = color;
	cmd->flags = flags;
}

fpl_extern void RenderPushViewport(RenderState *state, const int x, const int y, const int w, const int h) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Viewport);
	ViewportCommand *cmd = _RenderPushTypeAs(state, header, ViewportCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
}

fpl_extern void RenderPushScissor(RenderState *state, const int x, const int y, const int w, const int h) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Scissor);
	ScissorCommand *cmd = _RenderPushTypeAs(state, header, ScissorCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
}

fpl_extern void RenderPushRectangle(RenderState *state, const Vec2f bottomLeft, const Vec2f size, const Vec4f color, const bool isFilled, const float lineWidth) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Rectangle);
	RectangleCommand *cmd = _RenderPushTypeAs(state, header, RectangleCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->bottomLeft = bottomLeft;
	cmd->size = size;
	cmd->color = color;
	cmd->isFilled = isFilled;
	cmd->lineWidth = lineWidth;
}

fpl_extern void RenderPushRectangleCenter(RenderState *state, const Vec2f center, const Vec2f ext, const Vec4f color, const bool isFilled, const float lineWidth) {
	Vec2f bottomLeft = V2fSub(center, ext);
	Vec2f size = V2fMultScalar(ext, 2.0f);
	RenderPushRectangle(state, bottomLeft, size, color, isFilled, lineWidth);
}

fpl_extern void RenderPushRectangleMinMax(RenderState *state, const Vec2f min, const Vec2f max, const Vec4f color, const bool isFilled, const float lineWidth) {
	Vec2f size = V2fSub(max, min);
	RenderPushRectangle(state, min, size, color, isFilled, lineWidth);
}

fpl_extern void RenderPushQuad(RenderState *state, const Vec2f center, const float radius, const Vec4f color, const bool isFilled, const float lineWidth) {
	Vec2f ext = V2fInitScalar(radius);
	RenderPushRectangleCenter(state, center, ext, color, isFilled, lineWidth);
}

fpl_extern VertexAllocation RenderAllocateVertices(RenderState *state, const size_t capacity, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness) {
	VertexAllocation result = fplZeroInit;
	if (state == fpl_null) {
		return result;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Vertices);
	VerticesCommand *cmd = _RenderPushTypeAs(state, header, VerticesCommand, true);
	Vec2f *verts = _RenderPushTypesAs(state, header, capacity, Vec2f, true);
	if (cmd == fpl_null || verts == fpl_null) {
		return result;
	}
	cmd->capacity = capacity;
	cmd->count = 0;
	cmd->color = color;
	cmd->drawMode = drawMode;
	cmd->thickness = thickness;
	cmd->isLoop = isLoop;
	cmd->verts = verts;

	result.verts = verts;
	result.count = &cmd->count;
	return result;
}

fpl_extern void RenderPushVertices(RenderState *state, const Vec2f *verts, const size_t vertexCount, const bool copyVerts, const Vec4f color, const DrawMode drawMode, const bool isLoop, const float thickness) {
	if (state == fpl_null || verts == fpl_null || vertexCount == 0) {
		return;
	}

	CommandHeader *header = _RenderPushHeader(state, CommandType_Vertices);
	VerticesCommand *cmd = _RenderPushTypeAs(state, header, VerticesCommand, true);
	if (cmd == fpl_null) {
		return;
	}

	if (copyVerts) {
		Vec2f *dstVerts = _RenderPushTypesAs(state, header, vertexCount, Vec2f, true);
		if (dstVerts == fpl_null) {
			return;
		}
		for (size_t i = 0; i < vertexCount; ++i) {
			dstVerts[i] = verts[i];
		}
		cmd->verts = dstVerts;
	} else {
		cmd->verts = verts;
	}

	cmd->capacity = vertexCount;
	cmd->count = vertexCount;
	cmd->color = color;
	cmd->drawMode = drawMode;
	cmd->thickness = thickness;
	cmd->isLoop = isLoop;
}

fpl_extern void RenderPushSpriteRotate(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect, const SpriteFlags flags, const float rotation) {
	if (state == fpl_null) {
		return;
	}

	CommandHeader *header = _RenderPushHeader(state, CommandType_Sprite);
	SpriteCommand *cmd = _RenderPushTypeAs(state, header, SpriteCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->position = position;
	cmd->ext = ext;
	cmd->texture = texture;
	cmd->color = color;
	cmd->uvMin = V2fInit(uvRect.uMin, uvRect.vMin);
	cmd->uvMax = V2fInit(uvRect.uMax, uvRect.vMax);
	cmd->flags = flags;
	cmd->rotation = rotation;
}

fpl_extern void RenderPushSprite(RenderState *state, const Vec2f position, const Vec2f ext, const TextureHandle texture, const Vec4f color, const UVRect uvRect, const SpriteFlags flags) {
	const float noRotation = 0.0f;
	RenderPushSpriteRotate(state, position, ext, texture, color, uvRect, flags, noRotation);
}

fpl_extern SpriteBatchAllocation RenderAllocateSpriteBatch(RenderState *state, const size_t capacity, const TextureHandle texture) {
	SpriteBatchAllocation result = fplZeroInit;
	if (state == fpl_null || capacity == 0) {
		return result;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_SpriteBatch);
	SpriteBatchCommand *cmd = _RenderPushTypeAs(state, header, SpriteBatchCommand, true);
	SpriteInstance *instances = _RenderPushTypesAs(state, header, capacity, SpriteInstance, true);
	if (cmd == fpl_null || instances == fpl_null) {
		return result;
	}
	cmd->texture = texture;
	cmd->capacity = capacity;
	cmd->count = 0;
	cmd->instances = instances;

	result.instances = instances;
	result.count = &cmd->count;
	return result;
}

fpl_extern void RenderPushSpriteBatch(RenderState *state, const TextureHandle texture, const SpriteInstance *instances, const size_t count) {
	if (state == fpl_null || instances == fpl_null || count == 0) {
		return;
	}
	SpriteBatchAllocation batch = RenderAllocateSpriteBatch(state, count, texture);
	if (batch.instances == fpl_null) {
		return;
	}
	for (size_t i = 0; i < count; ++i) {
		batch.instances[i] = instances[i];
	}
	*batch.count = count;
}

fpl_extern void RenderPushTexture(RenderState *state, TextureHandle *targetTexture, const void *data, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel, const TextureFilterType filter, const TextureWrapMode wrap, const bool isTopDown, const bool isPreMultiplied) {
	if (state == fpl_null || targetTexture == fpl_null || data == fpl_null || width == 0 || height == 0) {
		return;
	}
	if (state->textureOperationCount >= fplArrayCount(state->textureOperations)) {
		return;
	}
	TextureOperation *op = &state->textureOperations[state->textureOperationCount++];
	fplClearStruct(op);
	op->data = data;
	op->width = width;
	op->height = height;
	op->bytesPerPixel = bytesPerPixel;
	op->type = TextureOperationType_Upload;
	op->wrap = wrap;
	op->filter = filter;
	op->handle = targetTexture;
	op->isPreMultiplied = isPreMultiplied;
	op->isTopDown = isTopDown;
}

fpl_extern void RenderPopTexture(RenderState *state, TextureHandle *targetTexture) {
	if (state == fpl_null || targetTexture == fpl_null) {
		return;
	}
	if (state->textureOperationCount >= fplArrayCount(state->textureOperations)) {
		return;
	}
	TextureOperation *op = &state->textureOperations[state->textureOperationCount++];
	fplClearStruct(op);
	op->handle = targetTexture;
	op->type = TextureOperationType_Release;
}

// Change the min/mag filter of a texture that is already uploaded, without re-decoding or re-uploading its
// pixels. Used to switch the whole game between nearest and linear filtering at runtime -- glTexParameteri
// only, so it is cheap enough to re-apply to every loaded texture on a toggle.
fpl_extern void RenderPushSetTextureFilter(RenderState *state, TextureHandle *targetTexture, const TextureFilterType filter) {
	if (state == fpl_null || targetTexture == fpl_null) {
		return;
	}
	if (state->textureOperationCount >= fplArrayCount(state->textureOperations)) {
		return;
	}
	TextureOperation *op = &state->textureOperations[state->textureOperationCount++];
	fplClearStruct(op);
	op->handle = targetTexture;
	op->filter = filter;
	op->type = TextureOperationType_SetFilter;
}

fpl_extern void RenderPushCircle(RenderState *state, const Vec2f position, const float radius, const size_t segmentCount, const Vec4f color, const bool isFilled, const float lineWidth) {
	if (state == fpl_null || radius <= 0.0f || segmentCount < 3) {
		return;
	}
	float seg = F32Tau / (float)segmentCount;
	size_t vertexCapacity = segmentCount;
	DrawMode drawMode;
	if (isFilled) {
		drawMode = DrawMode_Polygon;
	} else {
		drawMode = DrawMode_Lines;
	}
	VertexAllocation vertAlloc = RenderAllocateVertices(state, vertexCapacity, color, drawMode, true, lineWidth);
	if (vertAlloc.count == 0 || vertAlloc.verts == fpl_null) {
		return;
	}
	size_t vertexCount = 0;
	Vec2f *p = vertAlloc.verts;
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
		float x = position.x + F32Cos(segmentIndex * seg) * radius;
		float y = position.y + F32Sin(segmentIndex * seg) * radius;
		*p++ = V2fInit(x, y);
		++vertexCount;
	}
	*vertAlloc.count = vertexCount;
}

fpl_extern void RenderPushArc(RenderState *state, const Vec2f position, const float radius, const float startAngle, const float endAngle, const size_t segmentCount, const Vec4f color, const bool isFilled, const bool withClosingLines, const float lineWidth) {
	if (state == fpl_null || radius <= 0.0f || segmentCount < 3 || startAngle == endAngle) {
		return;
	}

	// Normalize angles and compute range
	float s = F32AngleNormalize(endAngle < startAngle ? endAngle : startAngle);
	float e = F32AngleNormalize(endAngle < startAngle ? startAngle : endAngle);

	float angleRange = e - s;
	if (angleRange <= 0.0f) {
		angleRange += F32Tau;
	}
	if (angleRange <= 1e-6f) {
		return;
	}

	const float step = angleRange / (float)segmentCount;

	if (isFilled) {
		// Filled Pie Slice
		size_t vertexCapacity = segmentCount + 2; // center + arc + close

		VertexAllocation va = RenderAllocateVertices(state, vertexCapacity, color, DrawMode_Polygon, true, lineWidth);
		if (va.count == 0 || va.verts == fpl_null) {
			return;
		}

		size_t vc = 0;
		Vec2f *p = va.verts;

		// Center
		*p++ = position; 
		vc++;

		// Arc
		for (size_t i = 0; i <= segmentCount; ++i) {
			float a = s + step * (float)i;
			*p++ = V2fInit(position.x + F32Cos(a) * radius,
				position.y + F32Sin(a) * radius);
			vc++;
		}

		*va.count = vc;
	} else {
		// Line Arc
		size_t vertexCapacity = segmentCount * 2;
		if (withClosingLines) {
			vertexCapacity += 4;
		}

		VertexAllocation va = RenderAllocateVertices(state, vertexCapacity, color, DrawMode_Lines, false, lineWidth);
		if (va.count == 0 || va.verts == fpl_null) {
			return;
		}

		size_t vc = 0;
		Vec2f *p = va.verts;

		// First arc vertex
		Vec2f prev = V2fInit(position.x + F32Cos(s) * radius, position.y + F32Sin(s) * radius);

		// Arc segments
		for (size_t i = 1; i <= segmentCount; ++i) {
			float a = s + step * (float)i;
			Vec2f curr = V2fInit(position.x + F32Cos(a) * radius, position.y + F32Sin(a) * radius);
			*p++ = prev; vc++;
			*p++ = curr; vc++;
			prev = curr;
		}

		// Extra lines: center -> start, center -> end
		if (withClosingLines) {
			Vec2f startPt = V2fInit(position.x + F32Cos(s) * radius, position.y + F32Sin(s) * radius);
			Vec2f endPt = V2fInit(position.x + F32Cos(e) * radius, position.y + F32Sin(e) * radius);
			*p++ = position; vc++;
			*p++ = startPt;  vc++;
			*p++ = position; vc++;
			*p++ = endPt;    vc++;
		}

		*va.count = vc;
	}
}

fpl_extern void RenderPushText(RenderState *state, const char *text, const size_t textLen, const LoadedFont *font, const TextureHandle texture, const Vec2f position, const float maxHeight, const float horizontalAlignment, const float verticalAlignment, const Vec4f color) {
	if (state == fpl_null || text == fpl_null || textLen == 0 || font == fpl_null || texture == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Text);
	TextCommand *cmd = _RenderPushTypeAs(state, header, TextCommand, true);
	if (cmd == fpl_null) {
		return;
	}

	size_t capacity = textLen + 1;

	char *pt = _RenderPushTypesAs(state, header, capacity, char, false);
	if (pt == fpl_null) {
		return;
	}
	fplCopyStringLen(text, textLen, pt, capacity);

	cmd->position = position;
	cmd->texture = texture;
	cmd->font = font;
	cmd->color = color;
	cmd->textLength = textLen;
	cmd->maxHeight = maxHeight;
	cmd->horizontalAlignment = horizontalAlignment;
	cmd->verticalAlignment = verticalAlignment;
}

fpl_extern void RenderPushLine(RenderState *state, const Vec2f a, const Vec2f b, const Vec4f color, const float lineWidth) {
	Vec2f verts[] = { a, b };
	RenderPushVertices(state, verts, 2, true, color, DrawMode_Lines, false, lineWidth);
}

fpl_extern void RenderPushShaderProgram(RenderState *state, ShaderProgramHandle *handle, const char *name, const char *vertexSource, const char *fragmentSource) {
	if (state == fpl_null || handle == fpl_null || vertexSource == fpl_null || fragmentSource == fpl_null) {
		return;
	}
	if (state->shaderOperationCount >= fplArrayCount(state->shaderOperations)) {
		return;
	}
	ShaderOperation *op = &state->shaderOperations[state->shaderOperationCount++];
	fplClearStruct(op);
	op->handle = handle;
	op->name = name != fpl_null ? name : "unnamed";
	op->vertexSource = vertexSource;
	op->fragmentSource = fragmentSource;
	op->type = ShaderOperationType_CreateProgram;
}

fpl_extern void RenderPopShaderProgram(RenderState *state, ShaderProgramHandle *handle) {
	if (state == fpl_null || handle == fpl_null) {
		return;
	}
	if (state->shaderOperationCount >= fplArrayCount(state->shaderOperations)) {
		return;
	}
	ShaderOperation *op = &state->shaderOperations[state->shaderOperationCount++];
	fplClearStruct(op);
	op->handle = handle;
	op->type = ShaderOperationType_DestroyProgram;
}

fpl_extern void RenderPushUseProgram(RenderState *state, ShaderProgramHandle program) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_UseProgram);
	UseProgramCommand *cmd = _RenderPushTypeAs(state, header, UseProgramCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->program = program;
}

static void _RenderPushUniform(RenderState *state, const char *name, const UniformType type, const uint32_t count, const void *data, const size_t dataSize) {
	if (state == fpl_null || name == fpl_null || count == 0 || data == fpl_null || dataSize == 0) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_Uniform);
	UniformCommand *cmd = _RenderPushTypeAs(state, header, UniformCommand, true);
	uint8_t *payload = (uint8_t *)_RenderPushTypes(state, header, dataSize, sizeof(uint8_t), false);
	if (cmd == fpl_null || payload == fpl_null) {
		return;
	}
	size_t nameLength = fplGetStringLength(name);
	fplCopyStringLen(name, nameLength, cmd->name, fplArrayCount(cmd->name));
	cmd->type = type;
	cmd->count = count;
	const uint8_t *src = (const uint8_t *)data;
	for (size_t i = 0; i < dataSize; ++i) {
		payload[i] = src[i];
	}
}

fpl_extern void RenderPushUniformInt(RenderState *state, const char *name, const int32_t value) {
	_RenderPushUniform(state, name, UniformType_Int, 1, &value, sizeof(int32_t));
}

fpl_extern void RenderPushUniformFloat(RenderState *state, const char *name, const float value) {
	_RenderPushUniform(state, name, UniformType_Float, 1, &value, sizeof(float));
}

fpl_extern void RenderPushUniformVec2(RenderState *state, const char *name, const Vec2f value) {
	_RenderPushUniform(state, name, UniformType_Vec2, 1, &value.m[0], sizeof(float) * 2);
}

fpl_extern void RenderPushUniformVec3(RenderState *state, const char *name, const Vec3f value) {
	_RenderPushUniform(state, name, UniformType_Vec3, 1, &value.m[0], sizeof(float) * 3);
}

fpl_extern void RenderPushUniformVec4(RenderState *state, const char *name, const Vec4f value) {
	_RenderPushUniform(state, name, UniformType_Vec4, 1, &value.m[0], sizeof(float) * 4);
}

fpl_extern void RenderPushUniformMat4(RenderState *state, const char *name, const Mat4f *value) {
	if (value == fpl_null) {
		return;
	}
	_RenderPushUniform(state, name, UniformType_Mat4, 1, &value->m[0], sizeof(float) * 16);
}

fpl_extern void RenderPushUniformFloatArray(RenderState *state, const char *name, const float *values, const size_t count) {
	_RenderPushUniform(state, name, UniformType_Float, (uint32_t)count, values, sizeof(float) * count);
}

fpl_extern void RenderPushUniformVec2Array(RenderState *state, const char *name, const Vec2f *values, const size_t count) {
	_RenderPushUniform(state, name, UniformType_Vec2, (uint32_t)count, values, sizeof(float) * 2 * count);
}

fpl_extern void RenderPushUniformVec3Array(RenderState *state, const char *name, const Vec3f *values, const size_t count) {
	_RenderPushUniform(state, name, UniformType_Vec3, (uint32_t)count, values, sizeof(float) * 3 * count);
}

fpl_extern void RenderPushBindTexture(RenderState *state, const TextureHandle texture, const int32_t unit) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_BindTexture);
	BindTextureCommand *cmd = _RenderPushTypeAs(state, header, BindTextureCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->texture = texture;
	cmd->unit = unit;
}

fpl_extern void RenderPushUnbindTexture(RenderState *state, const int32_t unit) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_UnbindTexture);
	UnbindTextureCommand *cmd = _RenderPushTypeAs(state, header, UnbindTextureCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->unit = unit;
}

fpl_extern void RenderPushCreateRenderTarget(RenderState *state, RenderTarget *target, const uint32_t width, const uint32_t height, const RenderTargetFormat *formats, const int colorCount, const bool hasDepth, const TextureFilterType filter) {
	if (state == fpl_null || target == fpl_null || formats == fpl_null || colorCount <= 0) {
		return;
	}
	if (state->renderTargetOperationCount >= fplArrayCount(state->renderTargetOperations)) {
		return;
	}
	int clampedColorCount = colorCount > RENDER_TARGET_MAX_COLOR_ATTACHMENTS ? RENDER_TARGET_MAX_COLOR_ATTACHMENTS : colorCount;
	fplClearStruct(target);
	target->width = width;
	target->height = height;
	target->colorCount = clampedColorCount;
	target->hasDepth = hasDepth;
	target->filter = filter;
	for (int i = 0; i < clampedColorCount; ++i) {
		target->formats[i] = formats[i];
	}
	RenderTargetOperation *op = &state->renderTargetOperations[state->renderTargetOperationCount++];
	fplClearStruct(op);
	op->target = target;
	op->type = RenderTargetOperationType_Create;
}

fpl_extern void RenderPushDestroyRenderTarget(RenderState *state, RenderTarget *target) {
	if (state == fpl_null || target == fpl_null) {
		return;
	}
	if (state->renderTargetOperationCount >= fplArrayCount(state->renderTargetOperations)) {
		return;
	}
	RenderTargetOperation *op = &state->renderTargetOperations[state->renderTargetOperationCount++];
	fplClearStruct(op);
	op->target = target;
	op->type = RenderTargetOperationType_Destroy;
}

fpl_extern void RenderPushBindRenderTarget(RenderState *state, const RenderTarget *target) {
	if (state == fpl_null || target == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_BindRenderTarget);
	BindRenderTargetCommand *cmd = _RenderPushTypeAs(state, header, BindRenderTargetCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->target = target;
}

fpl_extern void RenderPushUnbindRenderTarget(RenderState *state) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_UnbindRenderTarget);
	UnbindRenderTargetCommand *cmd = _RenderPushTypeAs(state, header, UnbindRenderTargetCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->unused = 0;
}

fpl_extern void RenderPushSetBlendMode(RenderState *state, const BlendMode mode) {
	if (state == fpl_null) {
		return;
	}
	CommandHeader *header = _RenderPushHeader(state, CommandType_SetBlendMode);
	SetBlendModeCommand *cmd = _RenderPushTypeAs(state, header, SetBlendModeCommand, true);
	if (cmd == fpl_null) {
		return;
	}
	cmd->mode = mode;
}

#endif // FINAL_RENDER_IMPLEMENTATION