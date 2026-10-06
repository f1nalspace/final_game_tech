/*
Name:
	Final OpenGL Renderer

Description:
	Implements a simple 2D-based Renderer using legacy OpenGL.
	This is always used in combination with the final_gameplatform.h.

	This file is part of the final_framework.

Changelog:
	## 2026-08-10
	- Execute CommandType_RectangleGradient: one glColor4fv per quad corner, so the ramp is interpolated by the pipeline (GL_SMOOTH) instead of by a run of flat strips

	## 2026-08-07
	- Sprites carry a free rotation angle: _ExpandSpriteInstance turns the quad corners about the sprite center, and CommandType_Sprite now expands through that same function instead of emitting its own corners, so a single sprite and a batched one cannot drift apart

	## 2026-07-23
	- Changed AllocateTexture repeatable to TextureWrapMode

	## 2026-07-19
	- Fixed: RenderWithOpenGL now stops when sizeof(CommandHeader) + dataSize exceeds the remaining buffer, so a truncated/mis-sized command can never underflow 'remaining' and walk off the buffer

	## 2026-07-15
	- Execute TextureOperationType_SetFilter: re-set an uploaded texture's GL_TEXTURE_MIN/MAG_FILTER in place (no re-upload)

	## 2026-07-14
	- Use final_log.h to log out OpenGL related warnings and errors

	## 2026-07-08
	- Render targets (FBOs): _CreateRenderTarget / _DestroyRenderTarget (deferred flush), CommandType_BindRenderTarget / _UnbindRenderTarget execution
	- Format table for RGBA8/R16F/R32F/RG16F/RGBA16F color attachments (float formats downgrade to RGBA8 without caps.supportsFloatTextures)
	- CommandType_SetBlendMode (Alpha/Additive/Opaque); frame resets to screen framebuffer + alpha blend
	- InitOpenGLRenderer detects render-target / float-texture support + MRT limits into caps

	## 2026-07-07
	- Shader support: compile/link/validate programs (deferred create/destroy), CommandType_UseProgram / CommandType_Uniform execution
	- CommandType_BindTexture / CommandType_UnbindTexture bind a texture to a unit for a bound shader
	- InitOpenGLRenderer(RenderCapabilities *outCaps) detects GPU capabilities (GL/GLSL version, max texture image units, shader support) into the passed caps

	## 2026-06-32
	- Render CommandType_SpriteBatch that renders a batch of sprites

	## 2026-06-08
	- Fixed CommandType_Clear was not clearing the entire frame, due to Scissor rectangle of the viewport

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_OPENGL_RENDER_H
#define FINAL_OPENGL_RENDER_H

#include <final_math.h>
#include <final_render.h>
#include <final_fontloader.h>
#include <final_dynamic_opengl.h>
#include <final_log.h>

#include <stdint.h>

fpl_extern_inline TextureHandle GetTextureHandleFromID(const GLuint texId) {
	return (TextureHandle)(uintptr_t)(texId);
}

fpl_extern_inline GLuint GetTextureIDFromHandle(const TextureHandle handle) {
	return (GLuint)(uintptr_t)(handle);
}

fpl_extern_inline ShaderProgramHandle GetProgramHandleFromID(const GLuint programId) {
	return (ShaderProgramHandle)(uintptr_t)(programId);
}

fpl_extern_inline GLuint GetProgramIDFromHandle(const ShaderProgramHandle handle) {
	return (GLuint)(uintptr_t)(handle);
}

fpl_extern void DrawSprite(const GLuint texId, const Vec2f offset, const Vec2f ext, const UVRect uv);
fpl_extern void DrawPoint(const float x, const float y, const float radius, const Vec4f color);
fpl_extern void DrawTextFont(const float x, const float y, const char *text, const size_t textLen, const LoadedFont *fontDesc, const GLuint fontTexture, const float maxCharHeight, const float sx, const float sy);
fpl_extern void DrawCircle(const float centerX, const float centerY, const float radius, const bool isFilled, const Vec4f color, const int segments);
fpl_extern void DrawNormal(const Vec2f pos, const Vec2f normal, const float length, const Vec4f color);

fpl_extern GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const TextureWrapMode wrap, const GLint filter, const bool isAlphaOnly);

// Initialize GL state and detect GPU capabilities. When outCaps is non-null it is filled with the
// GL/GLSL version, max texture image units and shader support -- the caller may then inspect or change
// the returned caps. Pass null to skip capability detection.
fpl_extern bool InitOpenGLRenderer(RenderCapabilities *outCaps);

fpl_extern void RenderWithOpenGL(RenderState *renderState);

#endif // FINAL_OPENGL_RENDER_H

#if defined(FINAL_OPENGL_RENDER_IMPLEMENTATION) && !defined(FINAL_OPENGL_RENDER_IMPLEMENTED)
#define FINAL_OPENGL_RENDER_IMPLEMENTED

#include <final_utils.h>

// Everything the GPU driver tells us is tagged with this, padded to the same width as the other log
// categories so the column lines up. These lines are the first thing to read when a machine we have
// no access to renders a black screen.
#define OPENGLRENDER_LOG_CATEGORY "OpenGL-Renderer"

// Turn a glCheckFramebufferStatus result into something a human can act on. "0x8CD6" in a bug report
// costs a lookup; "INCOMPLETE_ATTACHMENT" names the problem.
static const char *_GetFramebufferStatusName(const GLenum status) {
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:                      return "COMPLETE";
		case GL_FRAMEBUFFER_UNDEFINED:                     return "UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return "INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return "INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:                   return "UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return "INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return "INCOMPLETE_LAYER_TARGETS";
		default:                                           return "UNKNOWN";
	}
}

// Names for the render-target formats, so the log says RGBA16F rather than an enum ordinal. Indexed
// by RenderTargetFormat.
static const char *_GetRenderTargetFormatName(const RenderTargetFormat format) {
	switch(format) {
		case RenderTargetFormat_RGBA8:   return "RGBA8";
		case RenderTargetFormat_R16F:    return "R16F";
		case RenderTargetFormat_R32F:    return "R32F";
		case RenderTargetFormat_RG16F:   return "RG16F";
		case RenderTargetFormat_RGBA16F: return "RGBA16F";
		default:                         return "UNKNOWN";
	}
}

fpl_extern void DrawSprite(const GLuint texId, const Vec2f offset, const Vec2f ext, const UVRect uv) {
	const float uMin = uv.uMin;
	const float vMin = uv.vMin;
	const float uMax = uv.uMax;
	const float vMax = uv.vMax;
	const float xoffset = offset.x;
	const float yoffset = offset.y;
	const float rx = ext.w;
	const float ry = ext.h;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glTexCoord2f(uMax, vMax); glVertex2f(xoffset + rx, yoffset + ry);
	glTexCoord2f(uMin, vMax); glVertex2f(xoffset + -rx, yoffset + ry);
	glTexCoord2f(uMin, vMin); glVertex2f(xoffset + -rx, yoffset + -ry);
	glTexCoord2f(uMax, vMin); glVertex2f(xoffset + rx, yoffset + -ry);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

fpl_extern void DrawPoint(const float x, const float y, const float radius, const Vec4f color) {
	glColor4fv(&color.r);
	glPointSize(radius);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
	glPointSize(1);
}

fpl_extern void DrawTextFont(const float x, const float y, const char *text, const size_t textLen, const LoadedFont *fontDesc, const GLuint fontTexture, const float maxCharHeight, const float sx, const float sy) {
	if(fontDesc != fpl_null) {
		Vec2f textSize = FontGetTextSize(fontDesc, text, textLen, maxCharHeight);
		float xpos = x - textSize.w * 0.5f + (textSize.w * 0.5f * sx);
		float ypos = y - textSize.h * 0.5f + (textSize.h * 0.5f * sy);
		uint32_t lastChar = fontDesc->firstChar + (fontDesc->charCount - 1);
		for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
			char at = text[textPos];
			char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
			float advance;
			if((uint32_t)at >= fontDesc->firstChar && (uint32_t)at <= lastChar) {
				FontQuad quad = FontGetQuad(fontDesc, at, maxCharHeight);

				Vec2f pos = V2fInit(xpos,ypos);
				Vec2f offset = V2fAdd(pos, quad.offset);
				Vec2f ext = V2fMultScalar(quad.size, 0.5f);
				Vec2f uvMin = quad.uvMin;
				Vec2f uvMax = quad.uvMax;
				UVRect uvRect = UVRectInit(uvMin.x, uvMin.y, uvMax.x, uvMax.y);

#if 0
				Vec2f size = quad.size;
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glBegin(GL_LINE_LOOP);
				glVertex2f(offset.x + size.x * 0.5f, offset.y + size.y * -0.5f);
				glVertex2f(offset.x + size.x * -0.5f, offset.y + size.y * -0.5f);
				glVertex2f(offset.x + size.x * -0.5f, offset.y + size.y * 0.5f);
				glVertex2f(offset.x + size.x * 0.5f, offset.y + size.y * 0.5f);
				glEnd();
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#endif

				DrawSprite(fontTexture, offset, ext, uvRect);

				advance = FontGetCharacterAdvance(fontDesc, at, atNext) * maxCharHeight;
			} else {
				advance = fontDesc->info.spaceAdvance * maxCharHeight;
			}
			xpos += advance;
		}
	}
}

fpl_extern void DrawCircle(const float centerX, const float centerY, const float radius, const bool isFilled, const Vec4f color, const int segments) {
	float seg = F32Tau / (float)segments;
	glColor4fv(&color.r);
	glBegin(isFilled ? GL_POLYGON : GL_LINE_LOOP);
	for(int segmentIndex = 0; segmentIndex < segments; ++segmentIndex) {
		float x = centerX + F32Cos(segmentIndex * seg) * radius;
		float y = centerY + F32Sin(segmentIndex * seg) * radius;
		glVertex2f(x, y);
	}
	glEnd();
}

fpl_extern void DrawNormal(const Vec2f pos, const Vec2f normal, const float length, const Vec4f color) {
	glColor4fv(&color.r);
	glBegin(GL_LINES);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x + normal.x * length, pos.y + normal.y * length);
	glEnd();
}

// A uniform the driver did not give us a location for is dropped silently, and that is worth knowing: it
// means the shader never uses it (often because the driver optimized a whole branch away), which explains
// a feature that quietly does nothing. But the uniform is set EVERY FRAME, so logging it per occurrence
// would write a line per frame -- and the log flushes to disk on every line. Report each (program,
// uniform) pair once and then stay quiet.
#define MAX_REPORTED_MISSING_UNIFORMS 64
#define MAX_REPORTED_UNIFORM_NAME_LENGTH 64

typedef struct MissingUniformReport {
	GLuint program;
	char name[MAX_REPORTED_UNIFORM_NAME_LENGTH];
} MissingUniformReport;

static MissingUniformReport _missingUniforms[MAX_REPORTED_MISSING_UNIFORMS];
static int _missingUniformCount = 0;

static void _ReportMissingUniformOnce(RenderState *renderState, const GLuint program, const char *uniformName) {
	for(int reportIndex = 0; reportIndex < _missingUniformCount; ++reportIndex) {
		const MissingUniformReport *report = &_missingUniforms[reportIndex];
		if(report->program == program && fplIsStringEqual(report->name, uniformName)) {
			return;
		}
	}

	// The table is a fixed budget, not a guarantee. Once it is full we stop reporting rather than start
	// repeating: a flood here would be worse than the missing diagnostic.
	if(_missingUniformCount >= MAX_REPORTED_MISSING_UNIFORMS) {
		return;
	}

	MissingUniformReport *report = &_missingUniforms[_missingUniformCount++];
	report->program = program;
	fplCopyString(uniformName, report->name, fplArrayCount(report->name));

	LogWrite(LogLevel_Verbose, OPENGLRENDER_LOG_CATEGORY, "Uniform '%s' has no location in program %u -- the shader does not use it (reported once)", uniformName, (unsigned)program);
}

// GL_CLAMP is the legacy border-sampling clamp; GL_CLAMP_TO_EDGE is the one that does not bleed the
// border color into the last texel row, which is what every sheet in the game wants.
static GLint _GetTextureWrapParameter(const TextureWrapMode wrap) {
	switch(wrap) {
		case TextureWrapMode_Repeat:        return GL_REPEAT;
		case TextureWrapMode_ClampToBorder: return GL_CLAMP;
		case TextureWrapMode_ClampToEdge:
		default:                            return GL_CLAMP_TO_EDGE;
	}
}

fpl_extern GLuint AllocateTexture(const uint32_t width, const uint32_t height, const void *data, const TextureWrapMode wrap, const GLint filter, const bool isAlphaOnly) {
	if (width == 0 || height == 0) {
		return 0;
	}

	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	GLuint internalFormat = isAlphaOnly ? GL_ALPHA8 : GL_RGBA8;
	GLenum format = isAlphaOnly ? GL_ALPHA : GL_RGBA;

	// Drain any error left by an earlier call, so what we read after the upload is the upload's own.
	while(glGetError() != GL_NO_ERROR) {
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	GLenum uploadError = glGetError();
	if(uploadError != GL_NO_ERROR) {
		const uint32_t bytesPerPixel = isAlphaOnly ? 1 : 4;
		const size_t uploadBytes = (size_t)width * (size_t)height * bytesPerPixel;
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "Failed to upload a %ux%u %s texture (%zu bytes): GL error 0x%X", width, height, isAlphaOnly ? "ALPHA8" : "RGBA8", uploadBytes, (unsigned)uploadError);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLint wrapParameter = _GetTextureWrapParameter(wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParameter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParameter);

	glBindTexture(GL_TEXTURE_2D, 0);

	return(handle);
}

// The driver's own account of what it dislikes: deprecated calls, performance traps, undefined behaviour,
// and the errors we never thought to check for. On hardware we cannot debug this is the closest thing to
// standing behind the tester, so it goes straight into the log.
static void FGL_APIENTRY _GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	LogLevel level;
	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:         level = LogLevel_Error; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       level = LogLevel_Warning; break;
		case GL_DEBUG_SEVERITY_LOW:          level = LogLevel_Info; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: level = LogLevel_Verbose; break;
		default:                             level = LogLevel_Info; break;
	}
	LogWrite(level, OPENGLRENDER_LOG_CATEGORY, "Driver message (id %u): %s", (unsigned)id, message);
}

fpl_extern bool InitOpenGLRenderer(RenderCapabilities *outCaps) {
	// GL_KHR_debug, when the driver has it. Synchronous so the message arrives while the offending call is
	// still on the stack -- worth the cost, since nothing here is hot enough to notice it.
	if(glDebugMessageCallback != fpl_null) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(_GLDebugMessageCallback, fpl_null);
		LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Driver debug output is enabled (GL_KHR_debug)");
	} else {
		LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Driver debug output is unavailable (no GL_KHR_debug) -- driver-side messages will not appear in this log");
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_SCISSOR_TEST);

	// GPU capability detection. Generic: contains no shaders, only asks the driver what it can do.
	if(outCaps != fpl_null) {
		RenderCapabilities *caps = outCaps;
		fplClearStruct(caps);

		const GLubyte *versionString = glGetString(GL_VERSION);
		if(versionString != fpl_null) {
			// GL_VERSION begins with "<major>.<minor>"; parse leniently and ignore the vendor tail.
			int major = 0;
			int minor = 0;
			const char *p = (const char *)versionString;
			while(*p >= '0' && *p <= '9') { major = major * 10 + (*p - '0'); ++p; }
			if(*p == '.') {
				++p;
				while(*p >= '0' && *p <= '9') { minor = minor * 10 + (*p - '0'); ++p; }
			}
			caps->glMajor = major;
			caps->glMinor = minor;
		}

		const GLubyte *glslString = glGetString(GL_SHADING_LANGUAGE_VERSION);
		if(glslString != fpl_null) {
			size_t glslLength = fplGetStringLength((const char *)glslString);
			fplCopyStringLen((const char *)glslString, glslLength, caps->glslVersion, fplArrayCount(caps->glslVersion));
		}

		// Who the driver is. Every bug report from a machine we cannot reach starts here.
		const GLubyte *vendorString = glGetString(GL_VENDOR);
		if(vendorString != fpl_null) {
			fplCopyString((const char *)vendorString, caps->vendor, fplArrayCount(caps->vendor));
		}
		const GLubyte *rendererString = glGetString(GL_RENDERER);
		if(rendererString != fpl_null) {
			fplCopyString((const char *)rendererString, caps->renderer, fplArrayCount(caps->renderer));
		}
		if(versionString != fpl_null) {
			fplCopyString((const char *)versionString, caps->version, fplArrayCount(caps->version));
		}

		GLint maxTextureImageUnits = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
		caps->maxTextureImageUnits = (int)maxTextureImageUnits;

		GLint maxTextureSize = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		caps->maxTextureSize = (int)maxTextureSize;

		// The most reliable shader-support check: the driver both reports >= GL 2.0 AND the loader
		// actually resolved the shader entrypoints.
		bool hasShaderEntrypoints = (glCreateShader != fpl_null) && (glShaderSource != fpl_null) && (glCreateProgram != fpl_null) && (glUseProgram != fpl_null) && (glGetUniformLocation != fpl_null);
		bool hasShaderVersion = caps->glMajor >= 2;
		caps->supportsShaders = hasShaderEntrypoints && hasShaderVersion;

		// Framebuffer objects: the core FBO entrypoints resolved AND the driver is new enough (FBOs are
		// core since GL 3.0; ARB/EXT extensions bring them to older 2.x drivers but we keep the check simple).
		bool hasFramebufferEntrypoints = (glGenFramebuffers != fpl_null) && (glBindFramebuffer != fpl_null) && (glFramebufferTexture2D != fpl_null) && (glCheckFramebufferStatus != fpl_null) && (glDeleteFramebuffers != fpl_null);
		caps->supportsRenderTargets = hasFramebufferEntrypoints && (caps->glMajor >= 2);

		// Float color-attachment formats are core since GL 3.0, but a version number is a claim, not a
		// guarantee: whether an RGBA16F attachment actually yields a COMPLETE framebuffer is a driver
		// decision. The deferred light buffer lives or dies on that one answer, so ASK the driver --
		// build a 4x4 RGBA16F target and see. Getting this wrong the optimistic way is a black screen on
		// someone else's machine; probing costs one framebuffer at startup.
		caps->supportsFloatTextures = false;
		if(caps->supportsRenderTargets && caps->glMajor >= 3) {
			GLuint probeTexture = 0;
			glGenTextures(1, &probeTexture);
			glBindTexture(GL_TEXTURE_2D, probeTexture);
			const GLsizei probeSize = 4;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, probeSize, probeSize, 0, GL_RGBA, GL_HALF_FLOAT, fpl_null);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			GLuint probeFramebuffer = 0;
			glGenFramebuffers(1, &probeFramebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, probeFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, probeTexture, 0);

			GLenum probeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			caps->supportsFloatTextures = probeStatus == GL_FRAMEBUFFER_COMPLETE;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteFramebuffers(1, &probeFramebuffer);
			glDeleteTextures(1, &probeTexture);

			if(!caps->supportsFloatTextures) {
				const char *probeStatusName = _GetFramebufferStatusName(probeStatus);
				LogWrite(LogLevel_Warning, OPENGLRENDER_LOG_CATEGORY, "The GPU reports GL %d.%d but will not complete an RGBA16F framebuffer (%s) -- float render targets are unavailable and will be downgraded to RGBA8", caps->glMajor, caps->glMinor, probeStatusName);
			}
		}

		GLint maxColorAttachments = 1;
		GLint maxDrawBuffers = 1;
		if(caps->supportsRenderTargets) {
			glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
		}
		caps->maxColorAttachments = (int)maxColorAttachments;
		caps->maxDrawBuffers = (int)maxDrawBuffers;

		LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "GPU: %s -- %s", caps->vendor, caps->renderer);
		LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Driver: GL %s, GLSL %s", caps->version, caps->glslVersion);
		LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Caps: shaders %s, render targets %s, float targets %s (probed), max texture %dpx, %d texture unit(s), %d draw buffer(s)",
			caps->supportsShaders ? "yes" : "no",
			caps->supportsRenderTargets ? "yes" : "no",
			caps->supportsFloatTextures ? "yes" : "no",
			caps->maxTextureSize, caps->maxTextureImageUnits, caps->maxDrawBuffers);
	}

	return true;
}

// Sprite batches are expanded into client vertex arrays and drawn in chunks of this
// many sprites, so the scratch buffers stay a fixed ~1 MB regardless of the batch size.
#define SPRITE_BATCH_CHUNK_SPRITES 8192
// A sprite quad is four corners; one batched sprite therefore expands to that many verts.
#define SPRITE_QUAD_CORNER_COUNT 4
#define SPRITE_BATCH_VERTS_PER_SPRITE SPRITE_QUAD_CORNER_COUNT

static Vec2f _spriteBatchPos[SPRITE_BATCH_CHUNK_SPRITES * SPRITE_BATCH_VERTS_PER_SPRITE];
static Vec2f _spriteBatchUV[SPRITE_BATCH_CHUNK_SPRITES * SPRITE_BATCH_VERTS_PER_SPRITE];
static Vec4f _spriteBatchColor[SPRITE_BATCH_CHUNK_SPRITES * SPRITE_BATCH_VERTS_PER_SPRITE];

// Expand one sprite instance into 4 quad corners (position + uv), applying the flip flags, the 90-degree
// rotate flags and the free rotation angle. BOTH sprite paths go through here -- the batch below and the
// single CommandType_Sprite -- so a rotated sprite cannot come out different depending on how it was pushed.
//
// The corners are built as OFFSETS from the sprite center first and only translated at the end, because
// that is what the free rotation needs something to turn. The 90-degree flags swap the two extents: a
// quarter-turned cell is as wide as the cell is tall.
static void _ExpandSpriteInstance(const SpriteInstance *s, Vec2f *outPos, Vec2f *outUV) {
	const bool flipU = (s->flags & SpriteFlags_FlipU) == SpriteFlags_FlipU;
	const bool flipV = (s->flags & SpriteFlags_FlipV) == SpriteFlags_FlipV;
	const bool rot90CW = (s->flags & SpriteFlags_Rotate_90_CW) == SpriteFlags_Rotate_90_CW;
	const bool rot90CCW = (s->flags & SpriteFlags_Rotate_90_CCW) == SpriteFlags_Rotate_90_CCW;
	const float uMin = flipU ? s->uvMax.x : s->uvMin.x;
	const float uMax = flipU ? s->uvMin.x : s->uvMax.x;
	const float vMin = flipV ? s->uvMax.y : s->uvMin.y;
	const float vMax = flipV ? s->uvMin.y : s->uvMax.y;
	const bool notRotated = (!rot90CW && !rot90CCW) || (rot90CW && rot90CCW);
	const float ex = s->ext.w;
	const float ey = s->ext.h;
	if (notRotated) {
		outUV[0] = V2fInit(uMax, vMax); outPos[0] = V2fInit(ex, ey);
		outUV[1] = V2fInit(uMin, vMax); outPos[1] = V2fInit(-ex, ey);
		outUV[2] = V2fInit(uMin, vMin); outPos[2] = V2fInit(-ex, -ey);
		outUV[3] = V2fInit(uMax, vMin); outPos[3] = V2fInit(ex, -ey);
	} else if (rot90CW) {
		outUV[0] = V2fInit(uMin, vMax); outPos[0] = V2fInit(ey, ex);
		outUV[1] = V2fInit(uMin, vMin); outPos[1] = V2fInit(-ey, ex);
		outUV[2] = V2fInit(uMax, vMin); outPos[2] = V2fInit(-ey, -ex);
		outUV[3] = V2fInit(uMax, vMax); outPos[3] = V2fInit(ey, -ex);
	} else {
		outUV[0] = V2fInit(uMax, vMax); outPos[0] = V2fInit(-ey, ex);
		outUV[1] = V2fInit(uMin, vMax); outPos[1] = V2fInit(-ey, -ex);
		outUV[2] = V2fInit(uMin, vMin); outPos[2] = V2fInit(ey, -ex);
		outUV[3] = V2fInit(uMax, vMin); outPos[3] = V2fInit(ey, ex);
	}
	const float px = s->position.x;
	const float py = s->position.y;
	if (s->rotation != 0.0f) {
		const float cosAngle = F32Cos(s->rotation);
		const float sinAngle = F32Sin(s->rotation);
		for (int cornerIndex = 0; cornerIndex < SPRITE_QUAD_CORNER_COUNT; ++cornerIndex) {
			const float offsetX = outPos[cornerIndex].x;
			const float offsetY = outPos[cornerIndex].y;
			float rotatedX = offsetX * cosAngle - offsetY * sinAngle;
			float rotatedY = offsetX * sinAngle + offsetY * cosAngle;
			outPos[cornerIndex] = V2fInit(px + rotatedX, py + rotatedY);
		}
	} else {
		for (int cornerIndex = 0; cornerIndex < SPRITE_QUAD_CORNER_COUNT; ++cornerIndex) {
			outPos[cornerIndex] = V2fInit(px + outPos[cornerIndex].x, py + outPos[cornerIndex].y);
		}
	}
}

// Compile a single shader stage from source; logs the info log and returns 0 on failure.
static GLuint _CompileShader(RenderState *renderState, const char *programName, const GLenum shaderType, const char *source) {
	const char *shaderTypeName = shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment";

	GLuint shader = glCreateShader(shaderType);
	if(shader == 0) {
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "glCreateShader failed for the %s shader of program '%s'", shaderTypeName, programName);
		return 0;
	}
	const GLchar *sources[1] = { (const GLchar *)source };
	glShaderSource(shader, 1, sources, fpl_null);
	glCompileShader(shader);
	GLint compileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

	GLchar infoLog[2048] = fplZeroInit;
	glGetShaderInfoLog(shader, (GLsizei)fplArrayCount(infoLog), fpl_null, infoLog);
	const bool hasInfoLog = infoLog[0] != '\0';

	if(compileStatus != GL_TRUE) {
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "Failed to compile the %s shader of program '%s':\n%s", shaderTypeName, programName, infoLog);
		glDeleteShader(shader);
		return 0;
	}

	// A SUCCESSFUL compile can still carry an info log, and that is where a portability problem shows
	// up first: AMD and Intel warn about constructs NVIDIA accepts silently. Dropping it means the one
	// hint we get before the shader misbehaves on someone else's GPU is thrown away.
	if(hasInfoLog) {
		LogWrite(LogLevel_Warning, OPENGLRENDER_LOG_CATEGORY, "The %s shader of program '%s' compiled with messages:\n%s", shaderTypeName, programName, infoLog);
	} else {
		LogWrite(LogLevel_Verbose, OPENGLRENDER_LOG_CATEGORY, "Compiled the %s shader of program '%s'", shaderTypeName, programName);
	}

	return shader;
}

// Create + compile + link + validate a program from vertex/fragment source. Returns 0 on failure.
static GLuint _CreateShaderProgram(RenderState *renderState, const char *name, const char *vertexSource, const char *fragmentSource) {
	const char *programName = name != fpl_null ? name : "unnamed";

	GLuint vertexShader = _CompileShader(renderState, programName, GL_VERTEX_SHADER, vertexSource);
	if(vertexShader == 0) {
		return 0;
	}
	GLuint fragmentShader = _CompileShader(renderState, programName, GL_FRAGMENT_SHADER, fragmentSource);
	if(fragmentShader == 0) {
		glDeleteShader(vertexShader);
		return 0;
	}
	GLuint program = glCreateProgram();
	if(program == 0) {
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "glCreateProgram failed for program '%s'", programName);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint linkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if(linkStatus != GL_TRUE) {
		GLchar infoLog[2048] = fplZeroInit;
		glGetProgramInfoLog(program, (GLsizei)fplArrayCount(infoLog), fpl_null, infoLog);
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "Failed to link program '%s':\n%s", programName, infoLog);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(program);
		return 0;
	}
	// Validation is advisory: it can fail spuriously when samplers reference texture units with no
	// texture bound yet (as here, right after link). So we log a warning but keep the program.
	glValidateProgram(program);
	GLint validateStatus = 0;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
	if(validateStatus != GL_TRUE) {
		GLchar infoLog[2048] = fplZeroInit;
		glGetProgramInfoLog(program, (GLsizei)fplArrayCount(infoLog), fpl_null, infoLog);
		LogWrite(LogLevel_Warning, OPENGLRENDER_LOG_CATEGORY, "Program '%s' failed validation (advisory, keeping it):\n%s", programName, infoLog);
	}

	LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Linked program '%s' (id %u)", programName, (unsigned)program);
	// After a successful link the shaders can be detached and deleted; the program keeps its own copy.
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

// Map a RenderTargetFormat to the GL (internalFormat, format, type) triple. When the GPU has no float
// textures the float formats fall back to RGBA8 so the target still allocates (content just loses range).
static void _RenderTargetFormatToGL(const RenderTargetFormat fmt, const bool supportsFloat, GLint *outInternal, GLenum *outFormat, GLenum *outType) {
	RenderTargetFormat effective = fmt;
	if(!supportsFloat && fmt != RenderTargetFormat_RGBA8) {
		effective = RenderTargetFormat_RGBA8;
	}
	switch(effective) {
		case RenderTargetFormat_R16F:    *outInternal = GL_R16F;    *outFormat = GL_RED;  *outType = GL_HALF_FLOAT;    break;
		case RenderTargetFormat_R32F:    *outInternal = GL_R32F;    *outFormat = GL_RED;  *outType = GL_FLOAT;         break;
		case RenderTargetFormat_RG16F:   *outInternal = GL_RG16F;   *outFormat = GL_RG;   *outType = GL_HALF_FLOAT;    break;
		case RenderTargetFormat_RGBA16F: *outInternal = GL_RGBA16F; *outFormat = GL_RGBA; *outType = GL_HALF_FLOAT;    break;
		case RenderTargetFormat_RGBA8:
		default:                         *outInternal = GL_RGBA8;   *outFormat = GL_RGBA; *outType = GL_UNSIGNED_BYTE; break;
	}
}

// Allocate the FBO + its color attachment textures (+ optional depth renderbuffer) for a render target.
// Fills target->colorTextures / internalHandle / depthHandle. Leaves internalHandle null on failure so
// BindRenderTarget becomes a no-op rather than binding an incomplete framebuffer.
static void _CreateRenderTarget(RenderState *renderState, RenderTarget *target, const bool supportsFloat) {
	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	int colorCount = target->colorCount < 1 ? 1 : target->colorCount;
	if(colorCount > RENDER_TARGET_MAX_COLOR_ATTACHMENTS) {
		colorCount = RENDER_TARGET_MAX_COLOR_ATTACHMENTS;
	}
	GLint filter = target->filter == TextureFilterType_Nearest ? GL_NEAREST : GL_LINEAR;
	GLenum drawBuffers[RENDER_TARGET_MAX_COLOR_ATTACHMENTS];
	for(int i = 0; i < colorCount; ++i) {
		GLint internalFormat;
		GLenum format;
		GLenum type;
		_RenderTargetFormatToGL(target->formats[i], supportsFloat, &internalFormat, &format, &type);
		GLuint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, target->width, target->height, 0, format, type, fpl_null);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex, 0);
		target->colorTextures[i] = GetTextureHandleFromID(tex);
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint depthRenderbuffer = 0;
	if(target->hasDepth) {
		glGenRenderbuffers(1, &depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, target->width, target->height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	// The draw-buffer mapping is FBO state, so setting it once here persists for every later bind.
	if(colorCount > 1 && glDrawBuffers != fpl_null) {
		glDrawBuffers(colorCount, drawBuffers);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		const char *statusName = _GetFramebufferStatusName(status);
		LogWrite(LogLevel_Error, OPENGLRENDER_LOG_CATEGORY, "Render target %ux%u is incomplete: %s (0x%X) -- the feature using it will be unavailable", target->width, target->height, statusName, (unsigned)status);
		for(int i = 0; i < colorCount; ++i) {
			GLuint tex = GetTextureIDFromHandle(target->colorTextures[i]);
			if(tex > 0) {
				glDeleteTextures(1, &tex);
			}
			target->colorTextures[i] = fpl_null;
		}
		if(depthRenderbuffer > 0) {
			glDeleteRenderbuffers(1, &depthRenderbuffer);
		}
		glDeleteFramebuffers(1, &fbo);
		target->internalHandle = fpl_null;
		target->depthHandle = fpl_null;
	} else {
		target->internalHandle = GetProgramHandleFromID(fbo);
		target->depthHandle = GetProgramHandleFromID(depthRenderbuffer);

		// Report the format we actually GOT, not the one that was asked for: without float textures the
		// mapping above silently substitutes RGBA8, and a light buffer that lost its range explains a
		// washed-out or banded image that no shader log would account for.
		const char *requestedFormatName = _GetRenderTargetFormatName(target->formats[0]);
		const bool wasDowngraded = !supportsFloat && target->formats[0] != RenderTargetFormat_RGBA8;
		const char *effectiveFormatName = wasDowngraded ? _GetRenderTargetFormatName(RenderTargetFormat_RGBA8) : requestedFormatName;
		if(wasDowngraded) {
			LogWrite(LogLevel_Warning, OPENGLRENDER_LOG_CATEGORY, "Created render target %ux%u as %s: the GPU has no float textures, so the requested %s was downgraded", target->width, target->height, effectiveFormatName, requestedFormatName);
		} else {
			LogWrite(LogLevel_Info, OPENGLRENDER_LOG_CATEGORY, "Created render target %ux%u, format %s, %d color attachment(s), depth: %s", target->width, target->height, effectiveFormatName, colorCount, target->hasDepth ? "yes" : "no");
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Release the FBO, its color attachment textures and the optional depth renderbuffer.
static void _DestroyRenderTarget(RenderTarget *target) {
	int colorCount = target->colorCount;
	for(int i = 0; i < colorCount && i < RENDER_TARGET_MAX_COLOR_ATTACHMENTS; ++i) {
		GLuint tex = GetTextureIDFromHandle(target->colorTextures[i]);
		if(tex > 0) {
			glDeleteTextures(1, &tex);
		}
		target->colorTextures[i] = fpl_null;
	}
	GLuint depthRenderbuffer = GetProgramIDFromHandle(target->depthHandle);
	if(depthRenderbuffer > 0) {
		glDeleteRenderbuffers(1, &depthRenderbuffer);
	}
	GLuint fbo = GetProgramIDFromHandle(target->internalHandle);
	if(fbo > 0) {
		glDeleteFramebuffers(1, &fbo);
	}
	target->internalHandle = fpl_null;
	target->depthHandle = fpl_null;
}

// Translate a BlendMode to glBlendFunc. Blending stays enabled at all times (Opaque = one,zero).
static void _ApplyBlendMode(const BlendMode mode) {
	switch(mode) {
		case BlendMode_Additive: glBlendFunc(GL_ONE, GL_ONE); break;
		case BlendMode_Opaque:   glBlendFunc(GL_ONE, GL_ZERO); break;
		case BlendMode_Alpha:
		default:                 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
	}
}

fpl_extern void RenderWithOpenGL(RenderState *renderState) {
	size_t index = 0;
	while(renderState->textureOperationCount > 0) {
		TextureOperation *op = &renderState->textureOperations[index];
		if(op->type == TextureOperationType_Upload) {
			bool isAlphaOnly = op->bytesPerPixel == 1;
			GLint filter = op->filter == TextureFilterType_Nearest ? GL_NEAREST : GL_LINEAR;
			GLuint texId = AllocateTexture(op->width, op->height, op->data, op->wrap, filter, isAlphaOnly);
			*op->handle = GetTextureHandleFromID(texId);
		} else if(op->type == TextureOperationType_Release) {
			GLuint texId = GetTextureIDFromHandle(*op->handle);
			if(texId > 0) {
				glDeleteTextures(1, &texId);
				*op->handle = fpl_null;
			}
		} else if(op->type == TextureOperationType_SetFilter) {
			GLuint texId = GetTextureIDFromHandle(*op->handle);
			if(texId > 0) {
				GLint filter = op->filter == TextureFilterType_Nearest ? GL_NEAREST : GL_LINEAR;
				glBindTexture(GL_TEXTURE_2D, texId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		--renderState->textureOperationCount;
		++index;
	}
	fplAssert(renderState->textureOperationCount == 0);

	// Flush queued shader-program create/destroy operations (mirrors the texture-op flush above).
	{
		size_t shaderIndex = 0;
		while(renderState->shaderOperationCount > 0) {
			ShaderOperation *op = &renderState->shaderOperations[shaderIndex];
			if(op->type == ShaderOperationType_CreateProgram) {
				GLuint programId = _CreateShaderProgram(renderState, op->name, op->vertexSource, op->fragmentSource);
				*op->handle = GetProgramHandleFromID(programId);
			} else if(op->type == ShaderOperationType_DestroyProgram) {
				GLuint programId = GetProgramIDFromHandle(*op->handle);
				if(programId > 0) {
					glDeleteProgram(programId);
					*op->handle = fpl_null;
				}
			}
			--renderState->shaderOperationCount;
			++shaderIndex;
		}
	}
	fplAssert(renderState->shaderOperationCount == 0);

	// Flush queued render-target create/destroy operations (mirrors the shader-op flush above).
	{
		size_t renderTargetIndex = 0;
		while(renderState->renderTargetOperationCount > 0) {
			RenderTargetOperation *op = &renderState->renderTargetOperations[renderTargetIndex];
			if(op->type == RenderTargetOperationType_Create) {
				_CreateRenderTarget(renderState, op->target, renderState->caps.supportsFloatTextures);
			} else if(op->type == RenderTargetOperationType_Destroy) {
				_DestroyRenderTarget(op->target);
			}
			--renderState->renderTargetOperationCount;
			++renderTargetIndex;
		}
	}
	fplAssert(renderState->renderTargetOperationCount == 0);

	// Start every frame at the fixed-function pipeline; the command stream binds a program explicitly.
	glUseProgram(0);

	// Start every frame on the screen framebuffer with alpha blending; the command stream opts into
	// off-screen targets / additive blending explicitly and is expected to balance its binds.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	_ApplyBlendMode(BlendMode_Alpha);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(renderState->memory.size > sizeof(CommandHeader)) {
		uint8_t *mem = (uint8_t *)renderState->memory.base;
		size_t remaining = renderState->memory.used;
		Mat4f mvpCur = M4fInit(1.0f);
		// Program bound by the command stream; needed to resolve uniform locations for CommandType_Uniform.
		GLuint currentProgram = 0;
		renderState->matrixTop = 0;
		while(remaining > 0) {
			uint8_t *startMem = mem;
			CommandHeader *header = (CommandHeader *)mem;
			mem += sizeof(*header);
			uint8_t *dataStart = mem;
			size_t dataSize = header->dataSize;
			// Defensive: never let a truncated/mis-sized command walk the executor past the command buffer
			if (sizeof(*header) + dataSize > remaining) {
				break;
			}
			switch(header->type) {
				case CommandType_Viewport:
				{
					fplAssert(dataSize == sizeof(ViewportCommand));
					ViewportCommand *cmd = (ViewportCommand *)dataStart;
					glViewport(cmd->x, cmd->y, cmd->w, cmd->h);

					// NOTE(final): Scissor-Test is enabled by default, so we always use the same from the viewport
					glScissor(cmd->x, cmd->y, cmd->w, cmd->h);
				} break;

				case CommandType_Scissor:
				{
					fplAssert(dataSize == sizeof(ScissorCommand));
					ScissorCommand *cmd = (ScissorCommand *)dataStart;
					glScissor(cmd->x, cmd->y, cmd->w, cmd->h);
				} break;

				case CommandType_Clear:
				{
					fplAssert(dataSize == sizeof(ClearCommand));
					ClearCommand *cmd = (ClearCommand *)dataStart;
					GLbitfield mask = 0;
					if((cmd->flags & ClearFlags_Color) == ClearFlags_Color) {
						mask |= GL_COLOR_BUFFER_BIT;
					}
					if((cmd->flags & ClearFlags_Depth) == ClearFlags_Depth) {
						mask |= GL_DEPTH_BUFFER_BIT;
					}
					// NOTE(final): glClear honors the scissor box, so disable it for the clear to wipe the whole backbuffer (incl. the letterbox bars), then restore it.
					glDisable(GL_SCISSOR_TEST);
					glClearColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
					glClear(mask);
					glEnable(GL_SCISSOR_TEST);
				} break;

				case CommandType_Matrix:
				{
					fplAssert(dataSize == sizeof(MatrixCommand));
					MatrixCommand *cmd = (MatrixCommand *)dataStart;
					if(cmd->mode == MatrixMode_Set) {
						renderState->matrixTop = 0;
						mvpCur = cmd->mat;
					} else if(cmd->mode == MatrixMode_Push) {
						fplAssert(renderState->matrixTop < fplArrayCount(renderState->matrixStack));
						Mat4f *newMatrix = &renderState->matrixStack[renderState->matrixTop++];
						*newMatrix = mvpCur;
						mvpCur = M4fMult(*newMatrix, cmd->mat);
					} else if(cmd->mode == MatrixMode_Pop) {
						fplAssert(renderState->matrixTop > 0);
						mvpCur = renderState->matrixStack[--renderState->matrixTop];
					}
					glMatrixMode(GL_MODELVIEW);
					glLoadMatrixf(&mvpCur.m[0]);
				} break;

				case CommandType_Rectangle:
				{
					fplAssert(dataSize == sizeof(RectangleCommand));
					RectangleCommand *cmd = (RectangleCommand *)dataStart;
					if(!cmd->isFilled) {
						glLineWidth(cmd->lineWidth);
					}
					glColor4fv(&cmd->color.m[0]);
					glBegin(cmd->isFilled ? GL_QUADS : GL_LINE_LOOP);
					glVertex2f(cmd->bottomLeft.x + cmd->size.w, cmd->bottomLeft.y + cmd->size.h);
					glVertex2f(cmd->bottomLeft.x, cmd->bottomLeft.y + cmd->size.h);
					glVertex2f(cmd->bottomLeft.x, cmd->bottomLeft.y);
					glVertex2f(cmd->bottomLeft.x + cmd->size.w, cmd->bottomLeft.y);
					glEnd();
				} break;

				case CommandType_RectangleGradient:
				{
					fplAssert(dataSize == sizeof(RectangleGradientCommand));
					RectangleGradientCommand *cmd = (RectangleGradientCommand *)dataStart;
					// One color per CORNER and the pipeline interpolates between them (GL_SMOOTH is the default
					// shade model), which is the whole point: a ramp drawn as flat strips bands at any step count.
					const bool isHorizontal = cmd->axis == GradientAxis_Horizontal;
					const float left = cmd->bottomLeft.x;
					const float right = cmd->bottomLeft.x + cmd->size.w;
					const float bottom = cmd->bottomLeft.y;
					const float top = cmd->bottomLeft.y + cmd->size.h;
					// The start color sits on the axis' MIN edge, so both corners on that edge carry it.
					Vec4f topRightColor = cmd->endColor;
					Vec4f bottomLeftColor = cmd->startColor;
					Vec4f topLeftColor = isHorizontal ? cmd->startColor : cmd->endColor;
					Vec4f bottomRightColor = isHorizontal ? cmd->endColor : cmd->startColor;
					glBegin(GL_QUADS);
					glColor4fv(&topRightColor.m[0]); glVertex2f(right, top);
					glColor4fv(&topLeftColor.m[0]); glVertex2f(left, top);
					glColor4fv(&bottomLeftColor.m[0]); glVertex2f(left, bottom);
					glColor4fv(&bottomRightColor.m[0]); glVertex2f(right, bottom);
					glEnd();
				} break;

				case CommandType_Sprite:
				{
					fplAssert(dataSize == sizeof(SpriteCommand));
					SpriteCommand *cmd = (SpriteCommand *)dataStart;
					const GLuint texId = GetTextureIDFromHandle(cmd->texture);
					// One sprite is one instance. Expanded by the SAME function the batch path uses, so the
					// flips, the 90-degree steps and the free rotation can never behave differently between
					// a sprite pushed on its own and the same sprite pushed inside a batch.
					SpriteInstance singleInstance = fplZeroInit;
					singleInstance.color = cmd->color;
					singleInstance.position = cmd->position;
					singleInstance.ext = cmd->ext;
					singleInstance.uvMin = cmd->uvMin;
					singleInstance.uvMax = cmd->uvMax;
					singleInstance.flags = cmd->flags;
					singleInstance.rotation = cmd->rotation;
					Vec2f cornerPositions[SPRITE_QUAD_CORNER_COUNT];
					Vec2f cornerTexCoords[SPRITE_QUAD_CORNER_COUNT];
					_ExpandSpriteInstance(&singleInstance, cornerPositions, cornerTexCoords);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, texId);
					glColor4fv(&cmd->color.m[0]);
					glBegin(GL_QUADS);
					for (int cornerIndex = 0; cornerIndex < SPRITE_QUAD_CORNER_COUNT; ++cornerIndex) {
						glTexCoord2f(cornerTexCoords[cornerIndex].x, cornerTexCoords[cornerIndex].y);
						glVertex2f(cornerPositions[cornerIndex].x, cornerPositions[cornerIndex].y);
					}
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
				} break;

				case CommandType_SpriteBatch:
				{
					fplAssert(dataSize >= sizeof(SpriteBatchCommand));
					SpriteBatchCommand *cmd = (SpriteBatchCommand *)dataStart;
					if(cmd->count > 0) {
						const GLuint texId = GetTextureIDFromHandle(cmd->texture);
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, texId);
						glEnableClientState(GL_VERTEX_ARRAY);
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
						glEnableClientState(GL_COLOR_ARRAY);
						glVertexPointer(2, GL_FLOAT, 0, _spriteBatchPos);
						glTexCoordPointer(2, GL_FLOAT, 0, _spriteBatchUV);
						glColorPointer(4, GL_FLOAT, 0, _spriteBatchColor);
						size_t remainingSprites = cmd->count;
						size_t spriteOffset = 0;
						while(remainingSprites > 0) {
							size_t chunkSprites = remainingSprites < SPRITE_BATCH_CHUNK_SPRITES ? remainingSprites : SPRITE_BATCH_CHUNK_SPRITES;
							for(size_t s = 0; s < chunkSprites; ++s) {
								const SpriteInstance *instance = &cmd->instances[spriteOffset + s];
								size_t vertexBase = s * SPRITE_BATCH_VERTS_PER_SPRITE;
								_ExpandSpriteInstance(instance, &_spriteBatchPos[vertexBase], &_spriteBatchUV[vertexBase]);
								for(size_t v = 0; v < SPRITE_BATCH_VERTS_PER_SPRITE; ++v) {
									_spriteBatchColor[vertexBase + v] = instance->color;
								}
							}
							glDrawArrays(GL_QUADS, 0, (GLsizei)(chunkSprites * SPRITE_BATCH_VERTS_PER_SPRITE));
							spriteOffset += chunkSprites;
							remainingSprites -= chunkSprites;
						}
						glDisableClientState(GL_COLOR_ARRAY);
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
						glDisableClientState(GL_VERTEX_ARRAY);
						glBindTexture(GL_TEXTURE_2D, 0);
						glDisable(GL_TEXTURE_2D);
					}
				} break;

				case CommandType_Vertices:
				{
					fplAssert(dataSize >= sizeof(VerticesCommand));
					VerticesCommand *cmd = (VerticesCommand *)dataStart;
					glColor4fv(&cmd->color.m[0]);
					GLenum drawMode;
					switch(cmd->drawMode) {
						case DrawMode_Lines:
						{
							glLineWidth(cmd->thickness);
							drawMode = cmd->isLoop ? GL_LINE_LOOP : GL_LINES;
						} break;

						case DrawMode_Points:
						{
							glPointSize(cmd->thickness);
							drawMode = GL_POINTS;
						} break;

						case DrawMode_Polygon:
						{
							drawMode = GL_POLYGON;
						} break;

						case DrawMode_Triangles:
						{
							drawMode = cmd->isLoop ? GL_TRIANGLE_FAN : GL_POLYGON;
						} break;

						default:
							drawMode = GL_POLYGON;
					}
					glBegin(drawMode);
					for(size_t i = 0; i < cmd->count; ++i) {
						glVertex2fv(&cmd->verts[i].m[0]);
					}
					glEnd();
				} break;

				case CommandType_Text:
				{
					fplAssert(dataSize >= sizeof(TextCommand));
					TextCommand *cmd = (TextCommand *)dataStart;
					fplAssert(dataSize == (sizeof(TextCommand) + cmd->textLength + 1));
					const char *text = (const char *)(dataStart + sizeof(TextCommand));
					const size_t textLen = cmd->textLength;
					const LoadedFont *fontDesc = cmd->font;
					const TextureHandle texture = cmd->texture;
					if(fontDesc != fpl_null && fontDesc->charCount > 0 && texture != fpl_null) {
						const float maxHeight = cmd->maxHeight;
						const float ax = cmd->horizontalAlignment;
						const float ay = cmd->verticalAlignment;
						Vec2f textSize = FontGetTextSize(fontDesc, text, cmd->textLength, maxHeight);
						float xpos = cmd->position.x - textSize.w * 0.5f + (textSize.w * 0.5f * ax);
						float ypos = cmd->position.y - textSize.h * 0.5f + (textSize.h * 0.5f * ay);
						uint32_t lastChar = fontDesc->firstChar + (fontDesc->charCount - 1);

						GLuint texId = GetTextureIDFromHandle(texture);

						glColor4fv(&cmd->color.m[0]);
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, texId);
						for(uint32_t textPos = 0; textPos < textLen; ++textPos) {
							char at = text[textPos];
							char atNext = textPos < (textLen - 1) ? (text[textPos + 1]) : 0;
							float advance;
							if((uint32_t)at >= fontDesc->firstChar && (uint32_t)at <= lastChar) {
								uint32_t codePoint = at - fontDesc->firstChar;
								const FontGlyph *glyph = &fontDesc->glyphs[codePoint];
								Vec2f size = V2fMultScalar(glyph->charSize, maxHeight);
								Vec2f offset = V2fInit(xpos, ypos);
								offset = V2fAddMultScalar(offset, glyph->offset, maxHeight);
								offset = V2fAddMultScalar(offset, V2fInit(size.x, -size.y), 0.5f);

								float extW = size.w * 0.5f;
								float extH = size.h * 0.5f;

								glBegin(GL_QUADS);
								glTexCoord2f(glyph->uvMax.x, glyph->uvMax.y); glVertex2f(offset.x + extW, offset.y + extH);
								glTexCoord2f(glyph->uvMin.x, glyph->uvMax.y); glVertex2f(offset.x + -extW, offset.y + extH);
								glTexCoord2f(glyph->uvMin.x, glyph->uvMin.y); glVertex2f(offset.x + -extW, offset.y + -extH);
								glTexCoord2f(glyph->uvMax.x, glyph->uvMin.y); glVertex2f(offset.x + extW, offset.y + -extH);
								glEnd();

								advance = FontGetCharacterAdvance(fontDesc, at, atNext) * maxHeight;
							} else {
								advance = fontDesc->info.spaceAdvance * maxHeight;
							}
							xpos += advance;
						}
						glBindTexture(GL_TEXTURE_2D, 0);
						glDisable(GL_TEXTURE_2D);
					}
				} break;

				case CommandType_UseProgram:
				{
					fplAssert(dataSize == sizeof(UseProgramCommand));
					UseProgramCommand *cmd = (UseProgramCommand *)dataStart;
					currentProgram = GetProgramIDFromHandle(cmd->program);
					glUseProgram(currentProgram);
				} break;

				case CommandType_BindTexture:
				{
					fplAssert(dataSize == sizeof(BindTextureCommand));
					BindTextureCommand *cmd = (BindTextureCommand *)dataStart;
					const GLuint texId = GetTextureIDFromHandle(cmd->texture);
					glActiveTexture(GL_TEXTURE0 + cmd->unit);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, texId);
					// Leave unit 0 active so the Sprite/Text commands (which assume unit 0) keep working.
					glActiveTexture(GL_TEXTURE0);
				} break;

				case CommandType_UnbindTexture:
				{
					fplAssert(dataSize == sizeof(UnbindTextureCommand));
					UnbindTextureCommand *cmd = (UnbindTextureCommand *)dataStart;
					glActiveTexture(GL_TEXTURE0 + cmd->unit);
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
				} break;

				case CommandType_BindRenderTarget:
				{
					fplAssert(dataSize == sizeof(BindRenderTargetCommand));
					BindRenderTargetCommand *cmd = (BindRenderTargetCommand *)dataStart;
					const RenderTarget *target = cmd->target;
					GLuint fbo = GetProgramIDFromHandle(target->internalHandle);
					// A failed/unallocated target (fbo == 0) would rebind the screen -- skip so we do not
					// silently draw the off-screen pass onto the visible framebuffer.
					if(fbo > 0) {
						glBindFramebuffer(GL_FRAMEBUFFER, fbo);
						glViewport(0, 0, (GLsizei)target->width, (GLsizei)target->height);
						// Color-only targets have no depth buffer; depth testing there is undefined, so disable it.
						if(!target->hasDepth) {
							glDisable(GL_DEPTH_TEST);
						}
					}
				} break;

				case CommandType_UnbindRenderTarget:
				{
					fplAssert(dataSize == sizeof(UnbindRenderTargetCommand));
					// Back to the screen framebuffer + the always-on depth test. The caller pushes a
					// window-sized viewport next (as it already does around the UI).
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glEnable(GL_DEPTH_TEST);
				} break;

				case CommandType_SetBlendMode:
				{
					fplAssert(dataSize == sizeof(SetBlendModeCommand));
					SetBlendModeCommand *cmd = (SetBlendModeCommand *)dataStart;
					_ApplyBlendMode(cmd->mode);
				} break;

				case CommandType_Uniform:
				{
					fplAssert(dataSize >= sizeof(UniformCommand));
					UniformCommand *cmd = (UniformCommand *)dataStart;
					// A uniform with no bound program is a no-op (fixed-function has no uniforms).
					if(currentProgram != 0) {
						const void *payload = (const void *)(dataStart + sizeof(UniformCommand));
						GLint loc = glGetUniformLocation(currentProgram, cmd->name);
						if(loc >= 0) {
							const GLfloat *f = (const GLfloat *)payload;
							const GLint *iv = (const GLint *)payload;
							GLsizei count = (GLsizei)cmd->count;
							switch(cmd->type) {
								case UniformType_Int:   glUniform1iv(loc, count, iv); break;
								case UniformType_Float: glUniform1fv(loc, count, f); break;
								case UniformType_Vec2:  glUniform2fv(loc, count, f); break;
								case UniformType_Vec3:  glUniform3fv(loc, count, f); break;
								case UniformType_Vec4:  glUniform4fv(loc, count, f); break;
								case UniformType_Mat4:  glUniformMatrix4fv(loc, count, GL_FALSE, f); break;
								default: break;
							}
						} else {
							_ReportMissingUniformOnce(renderState, currentProgram, cmd->name);
						}
					}
				} break;

				default:
					fplAssert(!"Invalid default case!");
			}
			mem += dataSize;
			size_t consumed = (size_t)(mem - startMem);
			remaining -= consumed;
		}
	}
}

#endif // FINAL_OPENGL_RENDER_IMPLEMENTATION