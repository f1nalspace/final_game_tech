#pragma once

#include "defines.h"

namespace BasicShaderSource {
	static const char *Name = "Basic";

	static const char *Vertex =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec2 inPosition;\n"
		"layout(location = 1) in vec2 inTexcoord;\n"
		"\n"
		"uniform mat4 uniProjMat;\n"
		"out vec2 attrTexcoord;\n"
		"\n"
		"void main() {\n"
		"  attrTexcoord = inTexcoord;"
		"  gl_Position = uniProjMat * vec4(inPosition.xy, 0.0, 1.0);\n"
		"}\n"
		;

	static const char *Fragment =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
#if USE_GL_RECTANGLE_TEXTURES
		"uniform sampler2DRect uniTextures[4];\n"
#else
		"uniform sampler2D uniTextures[4];\n"
#endif
		"uniform float uniTextureScaleY;\n"
		"uniform float uniTextureOffsetY;\n"
		"in vec2 attrTexcoord;\n"
		"\n"
		"void main() {\n"
		"  vec2 texcoord = vec2(attrTexcoord.x, attrTexcoord.y * uniTextureScaleY + uniTextureOffsetY);\n"
		"  outColor = texture(uniTextures[0], texcoord);\n"
		"}\n"
		;

};

#define SHARED_CODE() \
	"const float y_const = 0.0625;\n" \
	"const float vu_const = 0.5;\n" \
	"vec4 YUVToRGBA(float y, float u, float v) {\n" \
	"  vec4 result;\n" \
	"  result.r = (1.164 * (y - y_const)) + (2.018 * (v - vu_const));\n" \
	"  result.g = (1.164 * (y - y_const)) - (0.813 * (u - vu_const)) - (0.391 * (v - vu_const));\n" \
	"  result.b = (1.164 * (y - y_const)) + (1.596 * (u - vu_const));\n" \
	"  result.a = 0.0;\n" \
	"  return result;\n" \
	"}\n"


namespace YUV420PShaderSource {
	static const char *Name = "YUV420p";

	static const char *Vertex = BasicShaderSource::Vertex;

	static const char *Fragment =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
#if USE_GL_RECTANGLE_TEXTURES
		"uniform sampler2DRect uniTextures[4];\n"
#else
		"uniform sampler2D uniTextures[4];\n"
#endif
		"uniform float uniTextureScaleY;\n"
		"uniform float uniTextureOffsetY;\n"
		"in vec2 attrTexcoord;\n"
		"\n"
		SHARED_CODE()
		"\n"
		"void main() {\n"
		"  vec2 texcoord = vec2(attrTexcoord.x, attrTexcoord.y * uniTextureScaleY + uniTextureOffsetY);\n"
		"  float y = texture(uniTextures[0], texcoord).r;"
		"  float u = texture(uniTextures[1], texcoord * 0.5).r;"
		"  float v = texture(uniTextures[2], texcoord * 0.5).r;"
		"  outColor = YUVToRGBA(y, u, v);\n"
		"}\n"
		;

};

namespace FontShaderSource {
	static const char *Name = "Font";

	static const char *Vertex =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 inPosition;\n"
		"layout(location = 1) in vec4 inColor;\n"
		"layout(location = 2) in vec2 inTexcoord;\n"
		"\n"
		"uniform mat4 uniViewProjMat;\n"
		"out vec2 attrTexcoord;\n"
		"out vec4 attrColor;\n"
		"\n"
		"void main() {\n"
		"  attrTexcoord = inTexcoord;"
		"  attrColor = inColor;"
		"  gl_Position = uniViewProjMat * inPosition;\n"
		"}\n"
		;

	static const char *Fragment =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 outColor;\n"
		"\n"
		"uniform sampler2D uniTexture;\n"
		"in vec4 attrColor;\n"
		"in vec2 attrTexcoord;\n"
		"\n"
		SHARED_CODE()
		"\n"
		"void main() {\n"
		"  float r = texture(uniTexture, attrTexcoord).r;\n"
		"  outColor = vec4(r, r, r, r) * attrColor;\n"
		"}\n"
		;
};

#undef SHARED_CODE