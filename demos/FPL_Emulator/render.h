/*
Name:
	Final Gamebox

	Frontend-Part (Renderer Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#pragma once

#include <final_platform_layer.h>

#include <final_memory.h>

#include "final_fontloader.h"

#include "final_math.h"

typedef struct Color4f {
	float r, g, b, a;
} Color4f;

typedef enum TextureFormat {
	TextureFormat_Automatic = -1,
	TextureFormat_None = 0,
	TextureFormat_Alpha = 8,
	TextureFormat_RGB = 24,
	TextureFormat_RGBA = 32,
} TextureFormat;

typedef enum TextureFilter {
	TextureFilter_Linear = 0,
	TextureFilter_Nearest,
} TextureFilter;

typedef struct {
	uint32_t internalFormat;
	uint32_t format;
} APITextureFormat;

typedef uint32_t TextureID;

typedef enum {
	TextureState_None = 0,
	TextureState_Update,
	TextureState_Clear,
} TextureState;

typedef struct {
	uint32_t *pixels;
	APITextureFormat format;
	TextureID id;
	uint32_t width;
	uint32_t height;
	float uScale;
	float vScale;
	uint32_t isValid;
	uint32_t hasPixels;
	TextureState state;
} Texture;

extern const APITextureFormat InvalidAPITextureFormat;

extern const Texture InvalidTexture;

extern const Color4f Color4fWhite;
extern const Color4f Color4fBlack;
extern const Color4f Color4fRed;
extern const Color4f Color4fGreen;
extern const Color4f Color4fBlue;
extern const Color4f Color4fYellow;
extern const Color4f Color4fGray;
extern const Color4f Color4fDarkGray;

typedef void RendererContext;

typedef struct {
	int majorVersion;
	int minorVersion;
	bool hasGLSL;
} RendererSupport;

extern RendererContext *RendererCreate(fmemMemoryBlock *memory, RendererSupport *outSupport);
extern void RendererDestroy(RendererContext *context);

extern Texture RendererTextureUpload(const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter, const void *data);
extern Texture RendererTextureAllocate(fmemMemoryBlock *mem, const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter);
extern Texture RendererTextureLoadFromMemory(const uint8_t *data, const size_t size, const TextureFormat format, const TextureFilter textureFilter, const uint32_t actualWidth, const uint32_t actualHeight);
extern void RendererTextureRelease(Texture *texture);
extern void RendererTextureUpdate(const Texture *texture);
extern void RendererTextureClear(const Texture *texture);

typedef enum ShaderType {
	ShaderType_Vertex = 0,
	ShaderType_Fragment,
	ShaderType_Count,
} ShaderType;

typedef enum ShaderErrorType {
	ShaderErrorType_InvalidSource,
	ShaderErrorType_FailedCreatingHandle,
	ShaderErrorType_Parse,
	ShaderErrorType_Compilation,
	ShaderErrorType_Linking,
} ShaderErrorType;

typedef struct ShaderError {
	ShaderErrorType type;
	char message[1024];
} ShaderError;

typedef struct ShaderProgram {
	uint32_t id;
} ShaderProgram;

extern bool RendererShaderIsValid(const ShaderProgram *program);
extern bool RendererShaderCreate(const char *vertexSource, const char *fragmentSource, ShaderProgram *outProgram, ShaderError *outError);
extern void RendererShaderRelease(ShaderProgram *program);
extern void RendererShaderBind(const ShaderProgram *program);
extern void RendererShaderUnbind(const ShaderProgram *program);

extern int32_t RendererShaderGetUniformLocation(const ShaderProgram *program, const char *name);
extern int32_t RendererShaderGetAttribLocation(const ShaderProgram *program, const char *name);

extern void RendererShaderUniform1f(const ShaderProgram *program, const int32_t location, const float v0);
extern void RendererShaderUniform2f(const ShaderProgram *program, const int32_t location, const float v0, const float v1);
extern void RendererShaderUniform3f(const ShaderProgram *program, const int32_t location, const float v0, const float v1, const float v2);
extern void RendererShaderUniform4f(const ShaderProgram *program, const int32_t location, const float v0, const float v1, const float v2, const float v3);

extern void RendererShaderUniform1i(const ShaderProgram *program, const int32_t location, const int32_t v0);
extern void RendererShaderUniform2i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1);
extern void RendererShaderUniform3i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1, const int32_t v2);
extern void RendererShaderUniform4i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1, const int32_t v2, const int32_t v3);

extern void RendererShaderUniform1ui(const ShaderProgram *program, const int32_t location, const uint32_t v0);
extern void RendererShaderUniform2ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1);
extern void RendererShaderUniform3ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1, const uint32_t v2);
extern void RendererShaderUniform4ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1, const uint32_t v2, const uint32_t v3);

extern void RendererShaderUniformVec2f(const ShaderProgram *program, const int32_t location, const Vec2f v);
extern void RendererShaderUniformVec3f(const ShaderProgram *program, const int32_t location, const Vec3f v);
extern void RendererShaderUniformVec4f(const ShaderProgram *program, const int32_t location, const Vec4f v);
extern void RendererShaderUniformMat4f(const ShaderProgram *program, const int32_t location, const Mat4f m);

extern void RendererSetViewport(const int x, const int y, const int w, const int h);

extern void RendererEnableClipping();
extern void RendererSetViewClipRect(const Mat4f *projMat, const Mat4f *viewMat, const Viewport4i *viewport, const float x, const float y, const float w, const float h);
extern void RendererDisableClipping();

extern void RendererClear(const float r, const float g, const float b, const float a);

extern void RendererSetModelViewProjectionMatrix(const float *mvp);

extern void RendererDrawString(const LoadedFont *font, const TextureID textureId, const char *text, const size_t textLen, const float x, const float y, const float scale, const Color4f color);

extern void RendererDrawTexturedQuad(
	const TextureID textureId,
	const float x,
	const float y,
	const float w,
	const float h,
	const Color4f color,
	const float u0,
	const float v0,
	const float u1,
	const float v1);

extern void RendererDrawTexturedQuadShader(
	const ShaderProgram *program,
	const TextureID samplerId,
	const int32_t samplerLocation,
	const float x,
	const float y,
	const float w,
	const float h,
	const float u0,
	const float v0,
	const float u1,
	const float v1);

extern void RendererDrawFilledQuad(const float x, const float y, const float w, const float h, const Color4f color);

extern void RendererDrawStrokedQuad(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f color);

extern void RendererDrawLine(const float x0, const float y0, const float x1, const float y1, const float lineWidth, const Color4f color);

typedef enum {
	ControlBorderFlags_None = 0,
	ControlBorderFlags_Left = 1 << 0,
	ControlBorderFlags_Right = 1 << 1,
	ControlBorderFlags_Top = 1 << 2,
	ControlBorderFlags_Bottom = 1 << 3,
	ControlBorderFlags_All = ControlBorderFlags_Left | ControlBorderFlags_Right | ControlBorderFlags_Top | ControlBorderFlags_Bottom,
} ControlBorderFlags;

extern void RendererDrawControlBorders(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f *color0, const Color4f *color1, const Color4f *color2, const ControlBorderFlags flags, const bool isDown);

extern void RendererDrawControlBorder(const float x, const float y, const float w, const float h, const float lineWidth, const Color4f *color0, const Color4f *color1, const Color4f *color2, const bool isDown);