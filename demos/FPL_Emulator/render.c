/*
Name:
	Final Gamebox

	Frontend-Part (Renderer Implementation)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.
*/

#include "render.h"

#include <final_memory.h>

#define FGL_IMPLEMENTATION
#define FGL_AS_PRIVATE
#include <final_dynamic_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "utils.h"

const char *RendererGetOpenGLLoaderVersion(void) {
	const char *result = fglGetVersion();
	return result;
}

typedef struct {
	ShaderProgram *activeProgram;
} RendererContextImpl;

const APITextureFormat InvalidAPITextureFormat = fplZeroInit;

const Texture InvalidTexture = fplZeroInit;

const Color4f ColorBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
const Color4f ColorYellow = { 1.0f, 1.0f, 0.0f, 1.0f };
const Color4f ColorGray = { 0.5f, 0.5f, 0.5f, 1.0f };

static char glErrorCodeBuffer[16];
static const char *GetGLErrorString(const GLenum err) {
	switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			fplStringFormat(glErrorCodeBuffer, fplArrayCount(glErrorCodeBuffer), "%u", err);
			return (const char *)glErrorCodeBuffer;
	}
}

static void CheckGLError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		const char *msg = GetGLErrorString(err);
		assert(!msg);
	}
}

static int GetComponentsFromTextureFormat(const TextureFormat format) {
	switch (format) {
		case TextureFormat_RGBA:
			return 4;
		case TextureFormat_RGB:
			return 3;
		case TextureFormat_Alpha:
			return 1;
		case TextureFormat_Automatic:
			return 0;
		default:
			return 0;
	}
}

static TextureFormat GetTextureFormatFromComponents(const int components) {
	if (components <= 0) {
		return TextureFormat_None;
	}
	switch (components) {
		case 1:
			return TextureFormat_Alpha;
		case 3:
			return TextureFormat_RGB;
		case 4:
			return TextureFormat_RGBA;
		default:
			return TextureFormat_None;
	}
}

static GLuint GetOpenGLTextureFilter(const TextureFilter filter) {
	switch (filter) {
		case TextureFilter_Linear:
			return GL_LINEAR;
		case TextureFilter_Nearest:
			return GL_NEAREST;
		default:
			return 0;
	}
}

static APITextureFormat GetOpenGLTextureFormat(const TextureFormat format) {
	switch (format) {
		case TextureFormat_RGBA:
			return fplStructInit(APITextureFormat, GL_RGBA8, GL_RGBA);
		case TextureFormat_RGB:
			return fplStructInit(APITextureFormat, GL_RGB8, GL_RGB);
		case TextureFormat_Alpha:
			return fplStructInit(APITextureFormat, GL_ALPHA8, GL_ALPHA);
		default:
			return InvalidAPITextureFormat;
	}
}

extern RendererContext *RendererCreate(fmemMemoryBlock *memory, RendererSupport *outSupport) {
	if (memory == fpl_null) {
		return fpl_null;
	}

	if (!fglLoadOpenGL(true)) {
		return fpl_null;
	}

	RendererContextImpl *result = fmemPushStruct(memory, RendererContextImpl, fmemPushFlags_Clear);
	if (result == fpl_null) {
		return fpl_null;
	}

	const char *openglVersion = glGetString(GL_VERSION);

	fplVersionInfo versionInfo = fplZeroInit;
	fpl__ParseVersionString(openglVersion, &versionInfo);

	outSupport->majorVersion = fplStringToS32(versionInfo.version.parts.major);
	outSupport->minorVersion = fplStringToS32(versionInfo.version.parts.minor);
	outSupport->hasGLSL = ((outSupport->majorVersion == 2 && outSupport->minorVersion >= 1) || (outSupport->majorVersion >= 3));

	glClearColor(0.1f, 0.3f, 0.7f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return result;
}

extern void RendererDestroy(RendererContext *context) {
	fglUnloadOpenGL();

	RendererContextImpl *impl = (RendererContextImpl *)context;

	if (impl != fpl_null) {
		// Nothing todo here
	}
}

extern void RendererTextureRelease(Texture *texture) {
	if (texture == fpl_null) {
		return;
	}

	if (texture->id != 0) {
		glDeleteTextures(1, &texture->id);
		texture->id = 0;
	}

	fplClearStruct(texture);
}

extern Texture RendererTextureUpload(const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter, const void *data) {
	APITextureFormat openGLTextureFormat = GetOpenGLTextureFormat(format);
	GLuint openglTextureFilter = GetOpenGLTextureFilter(textureFilter);

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, openGLTextureFormat.internalFormat, width, height, 0, openGLTextureFormat.format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, openglTextureFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, openglTextureFilter);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError();

	Texture result = fplZeroInit;
	result.width = width;
	result.height = height;
	result.id = id;
	result.format = openGLTextureFormat;
	result.uScale = 1.0f;
	result.vScale = 1.0f;
	result.isValid = true;

	return result;
}

extern Texture RendererTextureAllocate(fmemMemoryBlock *mem, const uint32_t width, const uint32_t height, const TextureFormat format, const TextureFilter textureFilter) {
	APITextureFormat openGLTextureFormat = GetOpenGLTextureFormat(format);
	GLuint openglTextureFilter = GetOpenGLTextureFilter(textureFilter);

	uint32_t roundedWidth = RoundToPowerOfTwo(width);
	uint32_t roundedHeight = RoundToPowerOfTwo(height);

	size_t dataSize = roundedWidth * roundedHeight * 4;

	uint32_t *data = (uint32_t *)fmemPush(mem, dataSize, fmemPushFlags_Clear);

	float uScale = (1.0f / (float)roundedWidth) * (float)width;
	float vScale = (1.0f / (float)roundedHeight) * (float)height;

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, openGLTextureFormat.internalFormat, roundedWidth, roundedHeight, 0, openGLTextureFormat.format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, openglTextureFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, openglTextureFilter);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError();

	Texture result = fplZeroInit;
	result.width = roundedWidth;
	result.height = roundedHeight;
	result.id = id;
	result.format = openGLTextureFormat;
	result.uScale = uScale;
	result.vScale = vScale;
	result.pixels = data;
	result.isValid = true;

	return result;
}

extern Texture RendererTextureLoadFromMemory(const uint8_t *data, const size_t size, const TextureFormat format, const TextureFilter textureFilter, const uint32_t actualWidth, const uint32_t actualHeight) {
	int requiredComponents = GetComponentsFromTextureFormat(format);

	stbi_set_flip_vertically_on_load(1);

	int w = 0, h = 0, comp = 0;
	stbi_uc *image = stbi_load_from_memory(data, (int)size, &w, &h, &comp, requiredComponents);
	if (image == fpl_null) {
		return InvalidTexture;
	}

	float texelX = 1.0f / (float)w;
	float texelY = 1.0f / (float)h;

	TextureFormat actualFormat = GetTextureFormatFromComponents(comp);

	APITextureFormat openGLTextureFormat = GetOpenGLTextureFormat(actualFormat);
	GLuint openglTextureFilter = GetOpenGLTextureFilter(textureFilter);

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, openGLTextureFormat.internalFormat, w, h, 0, openGLTextureFormat.format, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, openglTextureFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, openglTextureFilter);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);

	CheckGLError();

	float uscale = 1.0f;
	float vscale = 1.0f;

	if (actualWidth > 0 && (int)actualWidth < w) {
		uscale = texelX * actualWidth;
	}
	if (actualHeight > 0 && (int)actualHeight < w) {
		vscale = texelY * actualHeight;
	}

	Texture result = fplZeroInit;
	result.width = w;
	result.height = h;
	result.id = id;
	result.format = openGLTextureFormat;
	result.uScale = uscale;
	result.vScale = vscale;
	result.isValid = true;

	return result;
}

extern void RendererTextureClear(const Texture *texture) {
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
				texture->width, texture->height,
				texture->format.format,
				GL_UNSIGNED_BYTE,
				texture->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError();
}

extern void RendererTextureUpdate(const Texture *texture) {
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
				texture->width, texture->height,
				texture->format.format,
				GL_UNSIGNED_BYTE,
				texture->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError();
}

extern void RendererSetViewport(const int x, const int y, const int w, const int h) {
	glViewport(x, y, w, h);
}




extern void RendererClear(const float r, const float g, const float b, const float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

extern void RendererSetModelViewProjectionMatrix(const float *mvp) {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(mvp);
}


extern void RendererDrawFilledQuad(const float x, const float y, const float w, const float h, const Color4f color) {
	glEnable(GL_BLEND);
	glColor4fv(&color.r);
	glBegin(GL_QUADS);
	glVertex2f(x + w, y);
	glVertex2f(x, y);
	glVertex2f(x, y + h);
	glVertex2f(x + w, y + h);
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}





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
	const float v1) {
	glEnable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glColor4fv(&color.r);
	glBegin(GL_QUADS);
	glTexCoord2f(u1, v0); glVertex2f(x + w, y);
	glTexCoord2f(u0, v0); glVertex2f(x, y);
	glTexCoord2f(u0, v1); glVertex2f(x, y + h);
	glTexCoord2f(u1, v1); glVertex2f(x + w, y + h);
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
}


extern bool RendererShaderIsValid(const ShaderProgram *program) {
	return (program != fpl_null && program->id != 0);
}

static GLuint Renderer__CreateAndCompileShader(const GLuint shaderType, const char *source, const size_t sourceLen, ShaderError *outError) {
	GLuint id = glCreateShader(shaderType);
	if (id == 0) {
		outError->type = ShaderErrorType_FailedCreatingHandle;
		return 0;
	}

	GLint outSourceLen = sourceLen;
	glShaderSource(id, 1, &source, &outSourceLen);
	glCompileShader(id);

	GLint isCompiled = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);

	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
		if (maxLength > fplArrayCount(outError->message)) {
			maxLength = fplArrayCount(outError->message);
		}
		glGetShaderInfoLog(id, maxLength, &maxLength, outError->message);
		glDeleteShader(id);
		return 0;
	}

	return id;
}

extern bool RendererShaderCreate(const char *vertexSource, const char *fragmentSource, ShaderProgram *outProgram, ShaderError *outError) {
	if (vertexSource == fpl_null || fragmentSource == fpl_null || outProgram == fpl_null || outError == fpl_null) {
		return false;
	}

	const size_t vertexSourceLen = fplGetStringLength(vertexSource);
	const size_t fragmentSourceLen = fplGetStringLength(fragmentSource);
	if (vertexSourceLen == 0 || fragmentSourceLen == 0) {
		outError->type = ShaderErrorType_InvalidSource;
		return false;
	}

	fplClearStruct(outError);

	bool result = false;

	GLuint vertexShaderId = 0;
	GLuint fragmentShaderId = 0;

	vertexShaderId = Renderer__CreateAndCompileShader(GL_VERTEX_SHADER, vertexSource, vertexSourceLen, outError);
	if (vertexShaderId == 0) {
		goto cleanup;
	}

	fragmentShaderId = Renderer__CreateAndCompileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentSourceLen, outError);
	if (fragmentShaderId == 0) {
		goto cleanup;
	}

	GLuint program = glCreateProgram();
	if (program == 0) {
		outError->type = ShaderErrorType_FailedCreatingHandle;
		goto cleanup;
	}

	glAttachShader(program, vertexShaderId);
	glAttachShader(program, fragmentShaderId);
	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		if (maxLength > fplArrayCount(outError->message)) {
			maxLength = fplArrayCount(outError->message);
		}
		glGetProgramInfoLog(program, maxLength, &maxLength, outError->message);
		glDeleteProgram(program);
		goto cleanup;
	}

	glDetachShader(program, vertexShaderId);
	glDetachShader(program, fragmentShaderId);

	fplClearStruct(outProgram);
	outProgram->id = program;

	result = true;

cleanup:
	if (vertexShaderId != 0) {
		glDeleteShader(vertexShaderId);
	}
	if (fragmentShaderId != 0) {
		glDeleteShader(fragmentShaderId);
	}
	return result;
}

extern void RendererShaderRelease(ShaderProgram *program) {
	if (!RendererShaderIsValid(program)) {
		return;
	}
	glDeleteProgram(program->id);
	fplClearStruct(program);
}

extern void RendererShaderBind(const ShaderProgram *program) {
	if (!RendererShaderIsValid(program)) {
		return;
	}
	glUseProgram(program->id);
}

extern void RendererShaderUnbind(const ShaderProgram *program) {
	if (!RendererShaderIsValid(program)) {
		return;
	}
	glUseProgram(0);
}

extern int32_t RendererShaderGetUniformLocation(const ShaderProgram *program, const char *name) {
	if (!RendererShaderIsValid(program) || name == fpl_null || name[0] == '\0') {
		return -1;
	}
	return glGetUniformLocation(program->id, name);
}

extern int32_t RendererShaderGetAttribLocation(const ShaderProgram *program, const char *name) {
	if (!RendererShaderIsValid(program) || name == fpl_null || name[0] == '\0') {
		return -1;
	}
	return fgl_glGetAttribLocation(program->id, name);
}


extern void RendererShaderUniform1f(const ShaderProgram *program, const int32_t location, const float v0) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform1f(location, v0);
}

extern void RendererShaderUniform2f(const ShaderProgram *program, const int32_t location, const float v0, const float v1) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform2f(location, v0, v1);
}

extern void RendererShaderUniform3f(const ShaderProgram *program, const int32_t location, const float v0, const float v1, const float v2) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform3f(location, v0, v1, v2);
}

extern void RendererShaderUniform4f(const ShaderProgram *program, const int32_t location, const float v0, const float v1, const float v2, const float v3) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform4f(location, v0, v1, v2, v3);
}

extern void RendererShaderUniform1i(const ShaderProgram *program, const int32_t location, const int32_t v0) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform1i(location, v0);
}

extern void RendererShaderUniform2i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform2i(location, v0, v1);
}

extern void RendererShaderUniform3i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1, const int32_t v2) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform3i(location, v0, v1, v2);
}

extern void RendererShaderUniform4i(const ShaderProgram *program, const int32_t location, const int32_t v0, const int32_t v1, const int32_t v2, const int32_t v3) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform4i(location, v0, v1, v2, v3);
}

extern void RendererShaderUniform1ui(const ShaderProgram *program, const int32_t location, const uint32_t v0) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform1ui(location, v0);
}

extern void RendererShaderUniform2ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform2ui(location, v0, v1);
}

extern void RendererShaderUniform3ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1, const uint32_t v2) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform3ui(location, v0, v1, v2);
}

extern void RendererShaderUniform4ui(const ShaderProgram *program, const int32_t location, const uint32_t v0, const uint32_t v1, const uint32_t v2, const uint32_t v3) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform4ui(location, v0, v1, v2, v3);
}

extern void RendererShaderUniformVec2f(const ShaderProgram *program, const int32_t location, const Vec2f v) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform2f(location, v.x, v.y);
}

extern void RendererShaderUniformVec3f(const ShaderProgram *program, const int32_t location, const Vec3f v) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform3f(location, v.x, v.y, v.z);
}

extern void RendererShaderUniformVec4f(const ShaderProgram *program, const int32_t location, const Vec4f v) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniform4f(location, v.x, v.y, v.z, v.w);
}

extern void RendererShaderUniformMat4f(const ShaderProgram *program, const int32_t location, const Mat4f m) {
	if (!RendererShaderIsValid(program) || location < 0) {
		return;
	}
	glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat *)m.m);
}

//
// Final UI draw data
//

#if FUI_USE_16BIT_INDICES
#	define RENDERER_UI_INDEX_TYPE GL_UNSIGNED_SHORT
#else
#	define RENDERER_UI_INDEX_TYPE GL_UNSIGNED_INT
#endif

extern void RendererDrawUIDrawData(const fuiDrawData *drawData) {
	if (drawData == fpl_null || drawData->commandCount == 0 || drawData->vertices == fpl_null || drawData->indices == fpl_null) {
		return;
	}

	const int windowHeight = drawData->windowSize.y;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	// The vertex color has to reach the fragment untouched, or every glyph and every panel comes out tinted by whatever the last draw left behind
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	const fuiVertex *vertices = drawData->vertices;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(fuiVertex), &vertices->position);
	glTexCoordPointer(2, GL_FLOAT, sizeof(fuiVertex), &vertices->uv);
	// Packed in RGBA byte order, which is exactly how four unsigned bytes are read back
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(fuiVertex), &vertices->color);

	for (uint32_t commandIndex = 0; commandIndex < drawData->commandCount; ++commandIndex) {
		const fuiDrawCommand *command = drawData->commands + commandIndex;
		if (command->indexCount == 0) {
			continue;
		}

		// The clip rectangle is y-down from the top-left corner, while the scissor box is y-up from the bottom-left one
		const fuiRect clipRect = command->clipRect;
		const int scissorX = (int)clipRect.x;
		const int scissorY = windowHeight - (int)(clipRect.y + clipRect.h);
		const int scissorWidth = (int)clipRect.w;
		const int scissorHeight = (int)clipRect.h;
		if (scissorWidth <= 0 || scissorHeight <= 0) {
			continue;
		}
		glScissor(scissorX, scissorY, scissorWidth, scissorHeight);

		if (command->texture != FUI_TEXTURE_ID_NONE) {
			// An alpha atlas modulates only the alpha and leaves the vertex color as the text color, a color texture modulates all four components
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, (GLuint)command->texture);
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		const fuiDrawIndex *indices = drawData->indices + command->indexOffset;
		glDrawElements(GL_TRIANGLES, (GLsizei)command->indexCount, RENDERER_UI_INDEX_TYPE, indices);
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);

	// Back to the state RendererCreate left the context in, so the raw draws around this one behave as they always did
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	CheckGLError();
}
