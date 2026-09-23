/*
Name:
	FPL_ImageViewer | Resample pipeline

Description:
	Scales a picture on the GPU in two separable passes, in linear light with premultiplied alpha, for downscaling and upscaling alike.
	Downscaling widens the kernel by the reduction factor, so every source pixel contributes ("correct downscaling").
	Per axis the math is the one of ImageMagick -resize, so every result can be measured against it:

		bisect  = (x + 0.5 - origin) / scale               source position of the output pixel center, source pixel i spans [i, i + 1)
		widen   = max(1, 1 / scale)
		support = radius * widen
		taps    = floor(bisect - support + 0.5) ... floor(bisect + support + 0.5) - 1, taps outside the picture are left out
		weight  = Kernel((tap + 0.5 - bisect) / widen), normalized to a sum of 1

	Nearest is point sampling (the tap at floor(bisect)) and is never widened.
	Pass 1 filters horizontally into an RGBA16F intermediate (output columns x the source rows pass 2 needs),
	pass 2 filters vertically into an RGBA16F result of the output size. The result stays premultiplied and unclamped,
	so negative lobes survive until ResampleComposite() clamps it and blends it over the background in linear light.
	A result is cached, ResampleUpdate() only runs the passes when the request differs from the last one.

	Coordinates are pixels with y pointing down: row 0 of every texture is the top row.
	A picture that is displayed turned or mirrored (EXIF orientation) is resampled as displayed: pass 1 maps each displayed pixel
	through an integer axis mapping to the texel that stores it, so nothing is copied and pass 2 never knows.

Usage:
	C++ (raw string literals for GLSL), needs final_dynamic_opengl.h loaded and an OpenGL 3.3 core context.
	Define RESAMPLE_PIPELINE_IMPLEMENTATION in exactly one translation unit before including this header.

License:
	Copyright (c) 2017-2026 Torsten Spaete
	MIT License (See LICENSE file)
*/

#ifndef RESAMPLE_PIPELINE_H
#define RESAMPLE_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

#include <final_dynamic_opengl.h>

typedef enum ResampleKernel {
	ResampleKernel_Nearest = 0,
	ResampleKernel_Box,
	ResampleKernel_Triangle,
	ResampleKernel_Triangular,
	ResampleKernel_Bell,
	ResampleKernel_BSpline,
	ResampleKernel_Mitchell,
	ResampleKernel_CatmullRom,
	ResampleKernel_Lanczos3,
	ResampleKernel_Count,
} ResampleKernel;

typedef struct ResampleKernelDefinition {
	// Shown in the window title and the log
	const char *name;
	// Used on the command line
	const char *key;
	// Weight function in the GLSL kernel library, null for nearest
	const char *functionName;
	// Support in source pixels before widening
	float radius;
	// Weight 1 at distance 0 and 0 at every other integer distance, so 100 % reproduces the source
	bool isInterpolating;
} ResampleKernelDefinition;

typedef enum ResampleBackground {
	ResampleBackground_Checker = 0,
	ResampleBackground_Black,
	ResampleBackground_Gray,
	ResampleBackground_Count,
} ResampleBackground;

typedef struct ResampleBackgroundDefinition {
	const char *name;
	const char *key;
} ResampleBackgroundDefinition;

// Source pixels [first, end) one axis of an output range reads
typedef struct ResampleSourceRange {
	int32_t first;
	int32_t end;
} ResampleSourceRange;

// Displayed picture pixel (u = column, v = row) to stored texel: origin + u * stepU + v * stepV. All zero means as stored.
typedef struct ResampleAxisMapping {
	int32_t originX;
	int32_t originY;
	int32_t stepUX;
	int32_t stepUY;
	int32_t stepVX;
	int32_t stepVY;
} ResampleAxisMapping;

typedef struct ResampleRequest {
	// Picture texture (RGBA, sRGB when the framebuffer encodes sRGB)
	GLuint sourceTexture;
	// Changes whenever the texture gets new content, texture names are reused by OpenGL
	uint64_t sourceSerial;
	// Size of the picture as displayed, after the axis mapping
	uint32_t sourceWidth;
	uint32_t sourceHeight;
	ResampleAxisMapping axisMapping;
	ResampleKernel kernel;
	// Output pixels per source pixel on each axis
	float scaleX;
	float scaleY;
	// Output position of the top-left picture corner relative to the computed output area, negative when the picture starts left of or above it
	float originX;
	float originY;
	// Size of the computed output area, usually the visible part of the picture
	uint32_t outputWidth;
	uint32_t outputHeight;
} ResampleRequest;

typedef struct ResampleResult {
	ResampleRequest request;
	// RGBA16F, premultiplied linear light, outputWidth x outputHeight of the request
	GLuint texture;
	uint32_t textureWidth;
	uint32_t textureHeight;
	bool isValid;
} ResampleResult;

typedef struct ResamplePassProgram {
	GLuint programId;
	GLint locationSource;
	GLint locationScale;
	GLint locationOrigin;
	GLint locationRadius;
	GLint locationTapMinimum;
	GLint locationTapEnd;
	GLint locationRowOffset;
	GLint locationAxisOrigin;
	GLint locationAxisStepU;
	GLint locationAxisStepV;
} ResamplePassProgram;

typedef struct ResampleCompositeProgram {
	GLuint programId;
	GLint locationViewportSize;
	GLint locationRect;
	GLint locationResult;
	GLint locationBackgroundMode;
	GLint locationBackgroundColor;
	GLint locationCheckerColors;
	GLint locationCheckerOrigin;
	GLint locationCheckerSize;
} ResampleCompositeProgram;

typedef struct ResamplePipeline {
	ResamplePassProgram horizontalPrograms[ResampleKernel_Count];
	ResamplePassProgram verticalPrograms[ResampleKernel_Count];
	ResampleCompositeProgram compositeProgram;
	GLuint framebuffer;
	// Shared by all updates, only grows
	GLuint intermediateTexture;
	uint32_t intermediateWidth;
	uint32_t intermediateHeight;
	// GPU time of the two passes of the last timed update
	GLuint timerQueries[2];
	char timerLabel[256];
	bool isTimerPending;
} ResamplePipeline;

typedef struct ResampleCompositeParameters {
	// Size of the bound framebuffer
	uint32_t viewportWidth;
	uint32_t viewportHeight;
	// Viewport position of the top-left result pixel
	float left;
	float top;
	ResampleBackground background;
	// The checker board is anchored here, usually at the top-left picture corner, so it moves with the picture
	float checkerOriginX;
	float checkerOriginY;
} ResampleCompositeParameters;

extern const ResampleKernelDefinition *ResampleGetKernelDefinition(const ResampleKernel kernel);
// Finds a kernel by its command line key, ignoring case
extern bool ResampleFindKernel(const char *key, ResampleKernel *outKernel);
extern const ResampleBackgroundDefinition *ResampleGetBackgroundDefinition(const ResampleBackground background);
extern bool ResampleFindBackground(const char *key, ResampleBackground *outBackground);

// Source pixels one axis of the output pixels [firstOutput, endOutput) reads, the same computation the shaders do
extern ResampleSourceRange ResampleComputeSourceRange(const ResampleKernel kernel, const float scale, const float origin, const uint32_t firstOutput, const uint32_t endOutput, const uint32_t sourceLength);

extern bool ResamplePipelineInit(ResamplePipeline *pipeline);
extern void ResamplePipelineRelease(ResamplePipeline *pipeline);
extern void ResampleResultRelease(ResampleResult *result);

// Runs both passes when the request differs from the cached one and returns true then.
// With a timer label the GPU time of the passes is measured, ResamplePollTimer() hands it out once it is available.
// Keeps the bound framebuffer, the viewport and the blend state.
extern bool ResampleUpdate(ResamplePipeline *pipeline, const ResampleRequest *request, ResampleResult *result, const char *timerLabel);

// Draws the result into the bound framebuffer, clamped to 0 <= rgb <= alpha <= 1 and blended over the background in linear light
extern void ResampleComposite(ResamplePipeline *pipeline, const ResampleResult *result, const ResampleCompositeParameters *parameters);

// True once when the GPU time of the last timed update is available
extern bool ResamplePollTimer(ResamplePipeline *pipeline, const char **outLabel, double *outHorizontalMilliseconds, double *outVerticalMilliseconds);

#endif // RESAMPLE_PIPELINE_H

#if defined(RESAMPLE_PIPELINE_IMPLEMENTATION) && !defined(RESAMPLE_PIPELINE_IMPLEMENTED)
#define RESAMPLE_PIPELINE_IMPLEMENTED

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>

static const ResampleKernelDefinition ResampleKernelDefinitions[ResampleKernel_Count] = {
	{ "Nearest", "nearest", NULL, 0.5f, true },
	{ "Box", "box", "KernelBox", 0.5f, true },
	{ "Bilinear", "bilinear", "KernelTriangle", 1.0f, true },
	{ "Bicubic (Triangular)", "triangular", "KernelTriangular", 2.0f, false },
	{ "Bicubic (Bell)", "bell", "KernelBell", 2.0f, false },
	{ "Bicubic (B-Spline)", "bspline", "KernelBSpline", 2.0f, false },
	{ "Mitchell", "mitchell", "KernelMitchell", 2.0f, false },
	{ "Catmull-Rom", "catmullrom", "KernelCatmullRom", 2.0f, true },
	{ "Lanczos3", "lanczos3", "KernelLanczos3", 3.0f, true },
};

static const ResampleBackgroundDefinition ResampleBackgroundDefinitions[ResampleBackground_Count] = {
	{ "Checker", "checker" },
	{ "Black", "black" },
	{ "Gray", "gray" },
};

// Background colors as sRGB values, converted to linear light for the shader
static const float ResampleCheckerLightSRGB = 204.0f / 255.0f;
static const float ResampleCheckerDarkSRGB = 153.0f / 255.0f;
static const float ResampleGraySRGB = 128.0f / 255.0f;
// Edge length of a checker field in viewport pixels
static const float ResampleCheckerFieldSize = 8.0f;
// Extra source rows around the rows the vertical pass needs, covers float differences between the CPU and the GPU computation
static const int32_t ResampleRowMargin = 1;
static const double ResampleNanosecondsPerMillisecond = 1000000.0;
// The passes draw one triangle that covers the whole target
static const GLsizei ResampleFullTriangleVertexCount = 3;
static const GLsizei ResampleRectangleVertexCount = 4;
// Longest shader compile or link message that is printed
#define RESAMPLE_INFO_LOG_LENGTH 4096

static const char ResamplePassVertexSource[] = R"(
	#version 330 core
	void main() {
		// One triangle covering the target: (-1,-1) (3,-1) (-1,3)
		vec2 corner = vec2(float((gl_VertexID & 1) * 4 - 1), float((gl_VertexID >> 1) * 4 - 1));
		gl_Position = vec4(corner, 0.0, 1.0);
	}
)";

static const char ResampleKernelFunctionsSource[] = R"(
	const float Pi = 3.14159265358979;

	// The support alone limits the box, like ImageMagick
	float KernelBox(float x) {
		return 1.0;
	}

	float KernelTriangle(float x) {
		return max(0.0, 1.0 - abs(x));
	}

	// Triangle stretched to radius 2
	float KernelTriangular(float x) {
		const float radius = 2.0;
		return max(0.0, 1.0 - abs(x) / radius);
	}

	// Quadratic B-spline (radius 1.5) stretched to radius 2
	float KernelBell(float x) {
		const float stretch = 1.5 / 2.0;
		float a = abs(x) * stretch;
		if (a < 0.5) {
			return 0.75 - a * a;
		}
		if (a < 1.5) {
			float d = a - 1.5;
			return 0.5 * d * d;
		}
		return 0.0;
	}

	// Mitchell-Netravali family of cubic kernels with radius 2
	float KernelCubic(float x, float B, float C) {
		float a = abs(x);
		if (a < 1.0) {
			return ((12.0 - 9.0 * B - 6.0 * C) * a * a * a + (-18.0 + 12.0 * B + 6.0 * C) * a * a + (6.0 - 2.0 * B)) / 6.0;
		}
		if (a < 2.0) {
			return ((-B - 6.0 * C) * a * a * a + (6.0 * B + 30.0 * C) * a * a + (-12.0 * B - 48.0 * C) * a + (8.0 * B + 24.0 * C)) / 6.0;
		}
		return 0.0;
	}

	float KernelBSpline(float x) {
		const float B = 1.0;
		const float C = 0.0;
		return KernelCubic(x, B, C);
	}

	float KernelMitchell(float x) {
		const float B = 1.0 / 3.0;
		const float C = 1.0 / 3.0;
		return KernelCubic(x, B, C);
	}

	float KernelCatmullRom(float x) {
		const float B = 0.0;
		const float C = 0.5;
		return KernelCubic(x, B, C);
	}

	// sin(pi x) / (pi x). GPU sin() has an absolute error around 4e-7, which swamps sin(t) / t for a tiny t,
	// so small arguments use the Taylor series (its next term t^4 / 120 is below 1e-10 there)
	float Sinc(float x) {
		const float taylorLimit = 0.01;
		float t = Pi * x;
		if (abs(t) < taylorLimit) {
			return 1.0 - t * t / 6.0;
		}
		return sin(t) / t;
	}

	float KernelLanczos3(float x) {
		const float lobes = 3.0;
		float a = abs(x);
		if (a >= lobes) {
			return 0.0;
		}
		return Sinc(a) * Sinc(a / lobes);
	}
)";

// Needs HORIZONTAL (0 or 1), NEAREST (0 or 1) and, unless nearest, KERNEL defined.
// Horizontal: source is the picture, fragment (x, y) is output column x of displayed picture row y + uniRowOffset, taps are displayed picture columns,
// the axis mapping turns a displayed pixel into the stored texel.
// Vertical: source is the intermediate, fragment (x, y) is output pixel (x, y), taps are picture rows, stored at row tap - uniRowOffset.
static const char ResamplePassBodySource[] = R"(
	layout(location = 0) out vec4 outColor;
	uniform sampler2D uniSource;
	uniform float uniScale;
	uniform float uniOrigin;
	uniform float uniRadius;
	uniform int uniTapMinimum;
	uniform int uniTapEnd;
	uniform int uniRowOffset;
	uniform ivec2 uniAxisOrigin;
	uniform ivec2 uniAxisStepU;
	uniform ivec2 uniAxisStepV;

	vec4 FetchTap(int tap, ivec2 fragment) {
	#if HORIZONTAL
		int displayedRow = fragment.y + uniRowOffset;
		ivec2 stored = uniAxisOrigin + tap * uniAxisStepU + displayedRow * uniAxisStepV;
		vec4 texel = texelFetch(uniSource, stored, 0);
		return vec4(texel.rgb * texel.a, texel.a);
	#else
		return texelFetch(uniSource, ivec2(fragment.x, tap - uniRowOffset), 0);
	#endif
	}

	void main() {
		ivec2 fragment = ivec2(gl_FragCoord.xy);
	#if HORIZONTAL
		float outputCenter = gl_FragCoord.x;
	#else
		float outputCenter = gl_FragCoord.y;
	#endif
		float bisect = (outputCenter - uniOrigin) / uniScale;

	#if NEAREST
		int tap = clamp(int(floor(bisect)), uniTapMinimum, uniTapEnd - 1);
		outColor = FetchTap(tap, fragment);
	#else
		float widen = max(1.0, 1.0 / uniScale);
		float support = uniRadius * widen;
		int firstTap = max(int(floor(bisect - support + 0.5)), uniTapMinimum);
		int endTap = min(int(floor(bisect + support + 0.5)), uniTapEnd);
		vec4 sum = vec4(0.0);
		float weightSum = 0.0;
		for (int tap = firstTap; tap < endTap; ++tap) {
			float weight = KERNEL((float(tap) + 0.5 - bisect) / widen);
			sum += FetchTap(tap, fragment) * weight;
			weightSum += weight;
		}
		outColor = weightSum != 0.0 ? sum / weightSum : vec4(0.0);
	#endif
	}
)";

static const char ResampleCompositeVertexSource[] = R"(
	#version 330 core
	uniform vec2 uniViewportSize;
	uniform vec4 uniRect; // left, top, width, height in viewport pixels
	void main() {
		// Triangle strip corners 0..3: (0,0) (1,0) (0,1) (1,1)
		vec2 corner = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));
		vec2 position = uniRect.xy + corner * uniRect.zw;
		vec2 normalized = position / uniViewportSize;
		gl_Position = vec4(normalized.x * 2.0 - 1.0, 1.0 - normalized.y * 2.0, 0.0, 1.0);
	}
)";

static const char ResampleCompositeFragmentSource[] = R"(
	#version 330 core
	layout(origin_upper_left) in vec4 gl_FragCoord;
	layout(location = 0) out vec4 outColor;
	uniform vec4 uniRect;
	uniform sampler2D uniResult;
	uniform int uniBackgroundMode; // 0 = checker, otherwise uniBackgroundColor
	uniform vec3 uniBackgroundColor;
	uniform vec3 uniCheckerColors[2];
	uniform vec2 uniCheckerOrigin;
	uniform float uniCheckerSize;
	void main() {
		ivec2 texel = ivec2(gl_FragCoord.xy - uniRect.xy);
		vec4 premultiplied = texelFetch(uniResult, texel, 0);
		float alpha = clamp(premultiplied.a, 0.0, 1.0);
		vec3 color = clamp(premultiplied.rgb, vec3(0.0), vec3(alpha));
		vec3 background = uniBackgroundColor;
		if (uniBackgroundMode == 0) {
			ivec2 field = ivec2(floor((gl_FragCoord.xy - uniCheckerOrigin) / uniCheckerSize));
			background = uniCheckerColors[(field.x + field.y) & 1];
		}
		outColor = vec4(color + background * (1.0 - alpha), 1.0);
	}
)";

extern const ResampleKernelDefinition *ResampleGetKernelDefinition(const ResampleKernel kernel) {
	int index = (kernel >= 0 && kernel < ResampleKernel_Count) ? (int)kernel : (int)ResampleKernel_Nearest;
	const ResampleKernelDefinition *result = &ResampleKernelDefinitions[index];
	return(result);
}

static bool ResampleIsKeyEqual(const char *a, const char *b) {
	const char caseOffset = 'a' - 'A';
	while (*a && *b) {
		char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + caseOffset) : *a;
		char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b + caseOffset) : *b;
		if (ca != cb) {
			return(false);
		}
		++a;
		++b;
	}
	bool result = *a == 0 && *b == 0;
	return(result);
}

extern bool ResampleFindKernel(const char *key, ResampleKernel *outKernel) {
	for (int index = 0; index < ResampleKernel_Count; ++index) {
		if (ResampleIsKeyEqual(key, ResampleKernelDefinitions[index].key)) {
			*outKernel = (ResampleKernel)index;
			return(true);
		}
	}
	return(false);
}

extern const ResampleBackgroundDefinition *ResampleGetBackgroundDefinition(const ResampleBackground background) {
	int index = (background >= 0 && background < ResampleBackground_Count) ? (int)background : (int)ResampleBackground_Checker;
	const ResampleBackgroundDefinition *result = &ResampleBackgroundDefinitions[index];
	return(result);
}

extern bool ResampleFindBackground(const char *key, ResampleBackground *outBackground) {
	for (int index = 0; index < ResampleBackground_Count; ++index) {
		if (ResampleIsKeyEqual(key, ResampleBackgroundDefinitions[index].key)) {
			*outBackground = (ResampleBackground)index;
			return(true);
		}
	}
	return(false);
}

static float ResampleSRGBToLinear(const float value) {
	const float linearLimit = 0.04045f;
	const float linearSlope = 12.92f;
	const float offset = 0.055f;
	const float gamma = 2.4f;
	if (value <= linearLimit) {
		return(value / linearSlope);
	}
	float base = (value + offset) / (1.0f + offset);
	float result = powf(base, gamma);
	return(result);
}

static int32_t ResampleClampInt(const int32_t value, const int32_t minimum, const int32_t maximum) {
	int32_t result = value < minimum ? minimum : (value > maximum ? maximum : value);
	return(result);
}

// Taps of one output pixel center, not yet clamped to the picture
static ResampleSourceRange ResampleComputeTaps(const ResampleKernel kernel, const double scale, const double origin, const double outputCenter) {
	const ResampleKernelDefinition *definition = ResampleGetKernelDefinition(kernel);
	const double half = 0.5;
	double bisect = (outputCenter - origin) / scale;
	ResampleSourceRange result;
	if (kernel == ResampleKernel_Nearest) {
		double nearest = floor(bisect);
		result.first = (int32_t)nearest;
		result.end = result.first + 1;
	} else {
		double widen = scale < 1.0 ? 1.0 / scale : 1.0;
		double support = (double)definition->radius * widen;
		double first = floor(bisect - support + half);
		double end = floor(bisect + support + half);
		result.first = (int32_t)first;
		result.end = (int32_t)end;
	}
	return(result);
}

extern ResampleSourceRange ResampleComputeSourceRange(const ResampleKernel kernel, const float scale, const float origin, const uint32_t firstOutput, const uint32_t endOutput, const uint32_t sourceLength) {
	const double half = 0.5;
	ResampleSourceRange result = { 0, 0 };
	if (endOutput <= firstOutput || sourceLength == 0 || scale <= 0.0f) {
		return(result);
	}
	double firstCenter = (double)firstOutput + half;
	double lastCenter = (double)(endOutput - 1) + half;
	ResampleSourceRange firstTaps = ResampleComputeTaps(kernel, scale, origin, firstCenter);
	ResampleSourceRange lastTaps = ResampleComputeTaps(kernel, scale, origin, lastCenter);
	int32_t length = (int32_t)sourceLength;
	result.first = ResampleClampInt(firstTaps.first - ResampleRowMargin, 0, length);
	result.end = ResampleClampInt(lastTaps.end + ResampleRowMargin, result.first, length);
	return(result);
}

static GLuint ResampleCreateShader(const GLenum type, const char *name, const char *source) {
	GLuint shaderId = glCreateShader(type);
	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);
	GLint compileResult = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileResult);
	if (!compileResult) {
		char info[RESAMPLE_INFO_LOG_LENGTH] = { 0 };
		glGetShaderInfoLog(shaderId, (GLsizei)sizeof(info), NULL, info);
		fprintf(stderr, "Failed compiling resample shader '%s': %s\n", name, info);
		glDeleteShader(shaderId);
		shaderId = 0;
	}
	return(shaderId);
}

static GLuint ResampleCreateProgram(const char *name, const char *vertexSource, const char *fragmentSource) {
	GLuint vertexShader = ResampleCreateShader(GL_VERTEX_SHADER, name, vertexSource);
	GLuint fragmentShader = ResampleCreateShader(GL_FRAGMENT_SHADER, name, fragmentSource);
	if (vertexShader == 0 || fragmentShader == 0) {
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return(0);
	}
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint linkResult = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkResult);
	if (!linkResult) {
		char info[RESAMPLE_INFO_LOG_LENGTH] = { 0 };
		glGetProgramInfoLog(programId, (GLsizei)sizeof(info), NULL, info);
		fprintf(stderr, "Failed linking resample program '%s': %s\n", name, info);
		glDeleteProgram(programId);
		programId = 0;
	}
	return(programId);
}

static ResamplePassProgram ResampleCreatePassProgram(const ResampleKernel kernel, const bool isHorizontal) {
	const ResampleKernelDefinition *definition = ResampleGetKernelDefinition(kernel);
	bool isNearest = definition->functionName == NULL;
	std::string source = "#version 330 core\n";
	source += isHorizontal ? "#define HORIZONTAL 1\n" : "#define HORIZONTAL 0\n";
	source += isNearest ? "#define NEAREST 1\n" : "#define NEAREST 0\n";
	if (!isNearest) {
		source += "#define KERNEL " + std::string(definition->functionName) + "\n";
	}
	source += ResampleKernelFunctionsSource;
	source += ResamplePassBodySource;

	std::string name = std::string(isHorizontal ? "Horizontal " : "Vertical ") + definition->name;
	ResamplePassProgram result = { 0 };
	result.programId = ResampleCreateProgram(name.c_str(), ResamplePassVertexSource, source.c_str());
	if (result.programId > 0) {
		result.locationSource = glGetUniformLocation(result.programId, "uniSource");
		result.locationScale = glGetUniformLocation(result.programId, "uniScale");
		result.locationOrigin = glGetUniformLocation(result.programId, "uniOrigin");
		result.locationRadius = glGetUniformLocation(result.programId, "uniRadius");
		result.locationTapMinimum = glGetUniformLocation(result.programId, "uniTapMinimum");
		result.locationTapEnd = glGetUniformLocation(result.programId, "uniTapEnd");
		result.locationRowOffset = glGetUniformLocation(result.programId, "uniRowOffset");
		result.locationAxisOrigin = glGetUniformLocation(result.programId, "uniAxisOrigin");
		result.locationAxisStepU = glGetUniformLocation(result.programId, "uniAxisStepU");
		result.locationAxisStepV = glGetUniformLocation(result.programId, "uniAxisStepV");
	}
	return(result);
}

static ResampleCompositeProgram ResampleCreateCompositeProgram() {
	ResampleCompositeProgram result = { 0 };
	result.programId = ResampleCreateProgram("Composite", ResampleCompositeVertexSource, ResampleCompositeFragmentSource);
	if (result.programId > 0) {
		result.locationViewportSize = glGetUniformLocation(result.programId, "uniViewportSize");
		result.locationRect = glGetUniformLocation(result.programId, "uniRect");
		result.locationResult = glGetUniformLocation(result.programId, "uniResult");
		result.locationBackgroundMode = glGetUniformLocation(result.programId, "uniBackgroundMode");
		result.locationBackgroundColor = glGetUniformLocation(result.programId, "uniBackgroundColor");
		result.locationCheckerColors = glGetUniformLocation(result.programId, "uniCheckerColors");
		result.locationCheckerOrigin = glGetUniformLocation(result.programId, "uniCheckerOrigin");
		result.locationCheckerSize = glGetUniformLocation(result.programId, "uniCheckerSize");
	}
	return(result);
}

extern bool ResamplePipelineInit(ResamplePipeline *pipeline) {
	memset(pipeline, 0, sizeof(*pipeline));
	bool result = true;
	for (int index = 0; index < ResampleKernel_Count; ++index) {
		pipeline->horizontalPrograms[index] = ResampleCreatePassProgram((ResampleKernel)index, true);
		pipeline->verticalPrograms[index] = ResampleCreatePassProgram((ResampleKernel)index, false);
		result = result && pipeline->horizontalPrograms[index].programId > 0 && pipeline->verticalPrograms[index].programId > 0;
	}
	pipeline->compositeProgram = ResampleCreateCompositeProgram();
	result = result && pipeline->compositeProgram.programId > 0;
	glGenFramebuffers(1, &pipeline->framebuffer);
	glGenQueries(2, pipeline->timerQueries);
	return(result);
}

extern void ResampleResultRelease(ResampleResult *result) {
	if (result->texture > 0) {
		glDeleteTextures(1, &result->texture);
	}
	memset(result, 0, sizeof(*result));
}

extern void ResamplePipelineRelease(ResamplePipeline *pipeline) {
	for (int index = 0; index < ResampleKernel_Count; ++index) {
		glDeleteProgram(pipeline->horizontalPrograms[index].programId);
		glDeleteProgram(pipeline->verticalPrograms[index].programId);
	}
	glDeleteProgram(pipeline->compositeProgram.programId);
	if (pipeline->intermediateTexture > 0) {
		glDeleteTextures(1, &pipeline->intermediateTexture);
	}
	glDeleteFramebuffers(1, &pipeline->framebuffer);
	glDeleteQueries(2, pipeline->timerQueries);
	memset(pipeline, 0, sizeof(*pipeline));
}

static void ResampleAllocateFloatTexture(GLuint *texture, const uint32_t width, const uint32_t height) {
	if (*texture == 0) {
		glGenTextures(1, texture);
	}
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static bool ResampleIsRequestEqual(const ResampleRequest *a, const ResampleRequest *b) {
	bool result =
		a->sourceTexture == b->sourceTexture &&
		a->sourceSerial == b->sourceSerial &&
		a->sourceWidth == b->sourceWidth &&
		a->sourceHeight == b->sourceHeight &&
		memcmp(&a->axisMapping, &b->axisMapping, sizeof(a->axisMapping)) == 0 &&
		a->kernel == b->kernel &&
		a->scaleX == b->scaleX &&
		a->scaleY == b->scaleY &&
		a->originX == b->originX &&
		a->originY == b->originY &&
		a->outputWidth == b->outputWidth &&
		a->outputHeight == b->outputHeight;
	return(result);
}

static bool ResampleIsMappingEmpty(const ResampleAxisMapping *mapping) {
	bool result = mapping->stepUX == 0 && mapping->stepUY == 0 && mapping->stepVX == 0 && mapping->stepVY == 0;
	return(result);
}

static void ResampleRunPass(const ResamplePassProgram *program, const GLuint sourceTexture, const ResampleAxisMapping *mapping, const float scale, const float origin, const float radius, const int32_t tapMinimum, const int32_t tapEnd, const int32_t rowOffset, const uint32_t targetWidth, const uint32_t targetHeight) {
	const GLint textureUnit = 0;
	glViewport(0, 0, (GLsizei)targetWidth, (GLsizei)targetHeight);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);
	glUseProgram(program->programId);
	glUniform1i(program->locationSource, textureUnit);
	glUniform1f(program->locationScale, scale);
	glUniform1f(program->locationOrigin, origin);
	glUniform1f(program->locationRadius, radius);
	glUniform1i(program->locationTapMinimum, tapMinimum);
	glUniform1i(program->locationTapEnd, tapEnd);
	glUniform1i(program->locationRowOffset, rowOffset);
	glUniform2i(program->locationAxisOrigin, mapping->originX, mapping->originY);
	glUniform2i(program->locationAxisStepU, mapping->stepUX, mapping->stepUY);
	glUniform2i(program->locationAxisStepV, mapping->stepVX, mapping->stepVY);
	glDrawArrays(GL_TRIANGLES, 0, ResampleFullTriangleVertexCount);
}

extern bool ResampleUpdate(ResamplePipeline *pipeline, const ResampleRequest *request, ResampleResult *result, const char *timerLabel) {
	if (result->isValid && ResampleIsRequestEqual(&result->request, request)) {
		return(false);
	}
	bool isEmpty = request->outputWidth == 0 || request->outputHeight == 0 || request->sourceWidth == 0 || request->sourceHeight == 0 || request->scaleX <= 0.0f || request->scaleY <= 0.0f;
	if (isEmpty) {
		result->isValid = false;
		return(false);
	}

	const ResampleKernelDefinition *definition = ResampleGetKernelDefinition(request->kernel);
	ResampleAxisMapping mapping = request->axisMapping;
	if (ResampleIsMappingEmpty(&mapping)) {
		const ResampleAxisMapping asStored = { 0, 0, 1, 0, 0, 1 };
		mapping = asStored;
	}
	ResampleSourceRange rows = ResampleComputeSourceRange(request->kernel, request->scaleY, request->originY, 0, request->outputHeight, request->sourceHeight);
	uint32_t rowCount = (uint32_t)(rows.end - rows.first);
	if (rowCount == 0) {
		result->isValid = false;
		return(false);
	}

	if (result->texture == 0 || result->textureWidth != request->outputWidth || result->textureHeight != request->outputHeight) {
		ResampleAllocateFloatTexture(&result->texture, request->outputWidth, request->outputHeight);
		result->textureWidth = request->outputWidth;
		result->textureHeight = request->outputHeight;
	}
	if (pipeline->intermediateTexture == 0 || pipeline->intermediateWidth < request->outputWidth || pipeline->intermediateHeight < rowCount) {
		uint32_t width = pipeline->intermediateWidth > request->outputWidth ? pipeline->intermediateWidth : request->outputWidth;
		uint32_t height = pipeline->intermediateHeight > rowCount ? pipeline->intermediateHeight : rowCount;
		ResampleAllocateFloatTexture(&pipeline->intermediateTexture, width, height);
		pipeline->intermediateWidth = width;
		pipeline->intermediateHeight = height;
	}

	GLint previousFramebuffer = 0;
	GLint previousViewport[4] = { 0 };
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFramebuffer);
	glGetIntegerv(GL_VIEWPORT, previousViewport);
	GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, pipeline->framebuffer);

	bool isTimed = timerLabel != NULL && !pipeline->isTimerPending;

	// Pass 1: picture columns -> output columns, only the rows pass 2 reads
	const ResamplePassProgram *horizontalProgram = &pipeline->horizontalPrograms[request->kernel];
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->intermediateTexture, 0);
	if (isTimed) {
		glBeginQuery(GL_TIME_ELAPSED, pipeline->timerQueries[0]);
	}
	ResampleRunPass(horizontalProgram, request->sourceTexture, &mapping, request->scaleX, request->originX, definition->radius, 0, (int32_t)request->sourceWidth, rows.first, request->outputWidth, rowCount);
	if (isTimed) {
		glEndQuery(GL_TIME_ELAPSED);
	}

	// Pass 2: picture rows -> output rows
	const ResamplePassProgram *verticalProgram = &pipeline->verticalPrograms[request->kernel];
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result->texture, 0);
	if (isTimed) {
		glBeginQuery(GL_TIME_ELAPSED, pipeline->timerQueries[1]);
	}
	ResampleRunPass(verticalProgram, pipeline->intermediateTexture, &mapping, request->scaleY, request->originY, definition->radius, rows.first, rows.end, rows.first, request->outputWidth, request->outputHeight);
	if (isTimed) {
		glEndQuery(GL_TIME_ELAPSED);
		snprintf(pipeline->timerLabel, sizeof(pipeline->timerLabel), "%s", timerLabel);
		pipeline->isTimerPending = true;
	}

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)previousFramebuffer);
	glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
	if (wasBlendEnabled) {
		glEnable(GL_BLEND);
	}

	result->request = *request;
	result->isValid = true;
	return(true);
}

extern void ResampleComposite(ResamplePipeline *pipeline, const ResampleResult *result, const ResampleCompositeParameters *parameters) {
	if (!result->isValid) {
		return;
	}
	const ResampleCompositeProgram *program = &pipeline->compositeProgram;
	const GLint textureUnit = 0;
	const int backgroundModeChecker = 0;
	const int backgroundModeColor = 1;
	float checkerLight = ResampleSRGBToLinear(ResampleCheckerLightSRGB);
	float checkerDark = ResampleSRGBToLinear(ResampleCheckerDarkSRGB);
	float gray = ResampleSRGBToLinear(ResampleGraySRGB);
	float checkerColors[6] = { checkerLight, checkerLight, checkerLight, checkerDark, checkerDark, checkerDark };
	float backgroundColor = parameters->background == ResampleBackground_Gray ? gray : 0.0f;
	int backgroundMode = parameters->background == ResampleBackground_Checker ? backgroundModeChecker : backgroundModeColor;
	float width = (float)result->request.outputWidth;
	float height = (float)result->request.outputHeight;

	GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, result->texture);
	glUseProgram(program->programId);
	glUniform2f(program->locationViewportSize, (float)parameters->viewportWidth, (float)parameters->viewportHeight);
	glUniform4f(program->locationRect, parameters->left, parameters->top, width, height);
	glUniform1i(program->locationResult, textureUnit);
	glUniform1i(program->locationBackgroundMode, backgroundMode);
	glUniform3f(program->locationBackgroundColor, backgroundColor, backgroundColor, backgroundColor);
	glUniform3fv(program->locationCheckerColors, 2, checkerColors);
	glUniform2f(program->locationCheckerOrigin, parameters->checkerOriginX, parameters->checkerOriginY);
	glUniform1f(program->locationCheckerSize, ResampleCheckerFieldSize);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, ResampleRectangleVertexCount);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (wasBlendEnabled) {
		glEnable(GL_BLEND);
	}
}

extern bool ResamplePollTimer(ResamplePipeline *pipeline, const char **outLabel, double *outHorizontalMilliseconds, double *outVerticalMilliseconds) {
	if (!pipeline->isTimerPending) {
		return(false);
	}
	GLint isAvailable = 0;
	glGetQueryObjectiv(pipeline->timerQueries[1], GL_QUERY_RESULT_AVAILABLE, &isAvailable);
	if (!isAvailable) {
		return(false);
	}
	GLuint64 horizontalNanoseconds = 0;
	GLuint64 verticalNanoseconds = 0;
	glGetQueryObjectui64v(pipeline->timerQueries[0], GL_QUERY_RESULT, &horizontalNanoseconds);
	glGetQueryObjectui64v(pipeline->timerQueries[1], GL_QUERY_RESULT, &verticalNanoseconds);
	pipeline->isTimerPending = false;
	*outLabel = pipeline->timerLabel;
	*outHorizontalMilliseconds = (double)horizontalNanoseconds / ResampleNanosecondsPerMillisecond;
	*outVerticalMilliseconds = (double)verticalNanoseconds / ResampleNanosecondsPerMillisecond;
	return(true);
}

#endif // RESAMPLE_PIPELINE_IMPLEMENTATION
