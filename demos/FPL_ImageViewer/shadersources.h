// GLSL sources for the viewer, OpenGL 3.3 core
//
// Everything is drawn as a rectangle in viewport pixels with the origin at the top-left corner and y pointing down.
// A picture filter computes the picture position of each fragment from gl_FragCoord (texel centers are at +0.5),
// weights the texels around it with a separable kernel and fetches them with texelFetch.
// Taps outside the picture get no weight and the remaining weights are renormalized, the edge handling of ImageMagick.
// Textures and framebuffer are sRGB, so the filters run in linear light without any conversion here.

#include <string>

static const char RectangleVertexSource[] = R"(
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

static const char ColorFragmentSource[] = R"(
	#version 330 core
	layout(location = 0) out vec4 outColor;
	uniform vec4 uniColor;
	void main() {
		outColor = uniColor;
	}
)";

static const char PictureFragmentHeaderSource[] = R"(
	#version 330 core
	layout(origin_upper_left) in vec4 gl_FragCoord;
	layout(location = 0) out vec4 outColor;
	uniform sampler2D uniImage;
	uniform vec2 uniImageOrigin; // top-left corner of the picture in viewport pixels
	uniform vec2 uniImageScale; // viewport pixels per picture pixel
	uniform vec4 uniColor;

	// Picture position of the fragment center in picture pixels
	vec2 PicturePosition() {
		return (gl_FragCoord.xy - uniImageOrigin) / uniImageScale;
	}
)";

static const char NearestFilterBodySource[] = R"(
	void main() {
		ivec2 imageSize = textureSize(uniImage, 0);
		vec2 position = PicturePosition();
		ivec2 texel = clamp(ivec2(floor(position)), ivec2(0), imageSize - ivec2(1));
		outColor = texelFetch(uniImage, texel, 0) * uniColor;
	}
)";

static const char KernelFunctionsSource[] = R"(
	const float Pi = 3.14159265358979;

	// Bilinear: triangle with radius 1
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

// Needs KERNEL, TAP_FIRST and TAP_LAST defined
static const char KernelFilterBodySource[] = R"(
	void main() {
		ivec2 imageSize = textureSize(uniImage, 0);

		// Position relative to the texel centers: base is the texel at or before it, fraction the distance to that texel
		vec2 p = PicturePosition() - vec2(0.5);
		vec2 base = floor(p);
		vec2 fraction = p - base;
		ivec2 baseTexel = ivec2(base);

		vec4 sum = vec4(0.0);
		float weightSum = 0.0;
		for (int tapY = TAP_FIRST; tapY <= TAP_LAST; ++tapY) {
			int y = baseTexel.y + tapY;
			if (y < 0 || y >= imageSize.y) {
				continue;
			}
			float weightY = KERNEL(float(tapY) - fraction.y);
			for (int tapX = TAP_FIRST; tapX <= TAP_LAST; ++tapX) {
				int x = baseTexel.x + tapX;
				if (x < 0 || x >= imageSize.x) {
					continue;
				}
				float weight = KERNEL(float(tapX) - fraction.x) * weightY;
				sum += texelFetch(uniImage, ivec2(x, y), 0) * weight;
				weightSum += weight;
			}
		}
		vec4 filtered = weightSum > 0.0 ? sum / weightSum : vec4(0.0);
		outColor = filtered * uniColor;
	}
)";

static std::string NearestFilterFragmentSource() {
	std::string result = std::string(PictureFragmentHeaderSource) + NearestFilterBodySource;
	return(result);
}

// A kernel with the given integer radius needs the taps 1 - radius ... radius around the texel at or before the position
static std::string KernelFilterFragmentSource(const char *kernelFunctionName, const int kernelRadius) {
	int firstTap = 1 - kernelRadius;
	int lastTap = kernelRadius;
	std::string defines = "#define KERNEL " + std::string(kernelFunctionName) + "\n";
	defines += "#define TAP_FIRST " + std::to_string(firstTap) + "\n";
	defines += "#define TAP_LAST " + std::to_string(lastTap) + "\n";
	std::string result = std::string(PictureFragmentHeaderSource) + defines + KernelFunctionsSource + KernelFilterBodySource;
	return(result);
}
