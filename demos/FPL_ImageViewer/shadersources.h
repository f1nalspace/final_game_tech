// GLSL Filters: Bilinear, Bicubic, etc.
// https://stackoverflow.com/questions/13501081/efficient-bicubic-filtering-code-in-glsl
// https://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL
// https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl

#include <string>

static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

static const char *ColorVertexSource = R"(
	#version 330 core
	layout(location = 0) in vec4 inPosition;
	uniform mat4 uniVP;
	uniform mat4 uniModel;
	void main() {
		mat4 mvp = uniVP * uniModel;
		gl_Position = mvp * inPosition;
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

static const char FilterVertexSource[] = R"(
	#version 330 core
	layout(location = 0) in vec4 inPosition;
	layout(location = 1) in vec2 inTexcoord;
	out vec2 texCoord;
	uniform mat4 uniVP;
	uniform mat4 uniModel;
	void main() {
		mat4 mvp = uniVP * uniModel;
		gl_Position = mvp * inPosition;
		texCoord = inTexcoord;
	}
)";

static std::string HeaderFragmentSource(const char *samplerType) {
	std::string result = R"(
		#version 330 core
		layout(location = 0) out vec4 outColor;
		in vec2 texCoord;
		uniform vec2 uniTexSize;
		uniform vec2 uniTexScale;
		uniform vec4 uniColor;
		uniform %SAMPLER_TYPE% uniImage;
	)";
	result = ReplaceAll(result, "%SAMPLER_TYPE%", samplerType);
	return(result);
}

static const std::string SRGBFunctionSources = R"(
	// Converts a color from linear light gamma to sRGB gamma
	vec4 fromLinear(vec4 linearRGB)
	{
		bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
		vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
		vec4 lower = linearRGB * vec4(12.92);

		return mix(higher, lower, cutoff);
	}

	// Converts a color from sRGB gamma to linear light gamma
	vec4 toLinear(vec4 sRGB)
	{
		bvec4 cutoff = lessThan(sRGB, vec4(0.04045));
		vec4 higher = pow((sRGB + vec4(0.055))/vec4(1.055), vec4(2.4));
		vec4 lower = sRGB/vec4(12.92);

		return mix(higher, lower, cutoff);
	}
)";

static const std::string NoSRGBFunctionSources = R"(
	vec4 fromLinear(vec4 linearRGB)
	{
		return linearRGB;
	}
	vec4 toLinear(vec4 sRGB)
	{
		return sRGB;
	}
)";

static std::string UtilsFragmentSource() {
	std::string result = NoSRGBFunctionSources;
	return(result);
}

static std::string NoFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) + UtilsFragmentSource() + R"(
		void main() {
			vec2 uv = vec2(texCoord.x, 1.0 - texCoord.y);
			vec4 linearColor = toLinear(texture(uniImage, uv * uniTexScale)) * uniColor;
			outColor = fromLinear(linearColor);
		}
	)";
	return(result);
}

static std::string BilinearFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) + UtilsFragmentSource() + R"(
		vec4 BiLinear(vec2 texel, vec2 uv) {
			vec2 u0v0 = uv + vec2(0, 0);
			vec2 u1v0 = uv + vec2(texel.x, 0);
			vec2 u0v1 = uv + vec2(0, texel.y);
			vec2 u1v1 = uv + vec2(texel.x, texel.y);
			vec4 p0q0 = toLinear(texture(uniImage, u0v0 * uniTexScale));
			vec4 p1q0 = toLinear(texture(uniImage, u1v0 * uniTexScale));
			vec4 p0q1 = toLinear(texture(uniImage, u0v1 * uniTexScale));
			vec4 p1q1 = toLinear(texture(uniImage, u1v1 * uniTexScale));
			float a = fract(uv.x * uniTexSize.x);
			vec4 pInterp_q0 = mix(p0q0, p1q0, a);
			vec4 pInterp_q1 = mix(p0q1, p1q1, a);
			float b = fract(uv.y * uniTexSize.y);
			return fromLinear(mix(pInterp_q0, pInterp_q1, b));
		}

		void main() {
			vec2 uv = vec2(texCoord.x, 1.0 - texCoord.y);
			vec2 texel = vec2(1.0 / uniTexSize.x, 1.0 / uniTexSize.y);
			outColor = BiLinear(texel, uv);
		}
	)";
	return(result);
}

static std::string CreateBicubicFilter(const char *name) {
	std::string result = R"(
		vec4 BiCubic(vec2 texel, vec2 uv) {
			vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 denom = vec4(0.0, 0.0, 0.0, 0.0);
			float a = fract(uv.x * uniTexSize.x);
			float b = fract(uv.y * uniTexSize.y);
			for (int m = -1; m <= 2; ++m) {
				for (int n = -1; n <= 2; ++n) {
					vec2 vecUV = uv + vec2(texel.x * float(m), texel.y * float(n));
					vec4 vecData = toLinear(texture(uniImage, vecUV * uniTexScale));
					float f = %BICUBIC_FUNC%(float(m) - a);
					vec4 vecCooef1 = vec4(f,f,f,f);
					float f1 = %BICUBIC_FUNC%(-(float(n) - b));
					vec4 vecCoeef2 = vec4(f1, f1, f1, f1);
					sum += (vecData * vecCoeef2 * vecCooef1);
					denom += ((vecCoeef2 * vecCooef1));
				}
			}
			return fromLinear(sum / denom);
		}
	)";
	result = ReplaceAll(result, "%BICUBIC_FUNC%", name);
	return(result);
}

static const std::string BicubicFunctions = R"(
	float TriangularFunc(float x) {
		float f = x / 2.0;
		if (f < 0.0) {
			return (f + 1.0);
		} else {
			return (1.0 - f);
		}
		return 0.0;
	}

	float BellFunc(float x) {
		float f = (x / 2.0) * 1.5;
		if(f > -1.5 && f < -0.5)
		{
			return(0.5 * pow(f + 1.5, 2.0));
		}
		else if(f > -0.5 && f < 0.5)
		{
			return 3.0 / 4.0 - (f * f);
		}
		else if((f > 0.5 && f < 1.5))
		{
			return(0.5 * pow(f - 1.5, 2.0));
		}
		return 0.0;
	}

	float BSplineFunc(float x)
	{
		float f = x;
		if(f < 0.0)
		{
			f = -f;
		}

		if(f >= 0.0 && f <= 1.0)
		{
			return (2.0 / 3.0) + (0.5) * (f* f * f) - (f*f);
		}
		else if(f > 1.0 && f <= 2.0)
		{
			return 1.0 / 6.0 * pow((2.0 - f), 3.0);
		}
		return 1.0;
	}  

	float CatMullRomFunc(float x)
	{
		const float B = 0.0;
		const float C = 0.5;
		float f = x;
		if(f < 0.0)
		{
			f = -f;
		}
		if(f < 1.0)
		{
			return ( ( 12 - 9 * B - 6 * C ) * ( f * f * f ) +
				( -18 + 12 * B + 6 *C ) * ( f * f ) +
				( 6 - 2 * B ) ) / 6.0;
		}
		else if(f >= 1.0 && f < 2.0)
		{
			return ( ( -B - 6 * C ) * ( f * f * f )
				+ ( 6 * B + 30 * C ) * ( f *f ) +
				( - ( 12 * B ) - 48 * C  ) * f +
				8 * B + 24 * C)/ 6.0;
		}
		else
		{
			return 0.0;
		}
	}
)";

static const std::string BicubicBody = R"(
	void main() {
		vec2 uv = vec2(texCoord.x, 1.0 - texCoord.y);
		vec2 texel = vec2(1.0 / uniTexSize.x, 1.0 / uniTexSize.y);
		outColor = BiCubic(texel, uv);
	}
)";

static std::string BicubicTriangularFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) +
		UtilsFragmentSource() +
		BicubicFunctions +
		CreateBicubicFilter("TriangularFunc") +
		BicubicBody;
	return(result);
}

static std::string BicubicBellFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) +
		UtilsFragmentSource() +
		BicubicFunctions +
		CreateBicubicFilter("BellFunc") +
		BicubicBody;
	return(result);
}

static std::string BicubicBSplineFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) +
		UtilsFragmentSource() +
		BicubicFunctions +
		CreateBicubicFilter("BSplineFunc") +
		BicubicBody;
	return(result);
}

static std::string BicubicCatMullRowFilterFragmentSource(const char *samplerType) {
	std::string result = HeaderFragmentSource(samplerType) +
		UtilsFragmentSource() +
		BicubicFunctions +
		CreateBicubicFilter("CatMullRomFunc") +
		BicubicBody;
	return(result);
}

static std::string Lanczos3FilterFragmentSource(const char *samplerType) {
	std::string result =
		HeaderFragmentSource(samplerType) +
		UtilsFragmentSource() +
		R"(
		float sinc(float x)
		{
			return sin(x * 3.1415926535897932384626433) / (x * 3.1415926535897932384626433); 
		}

		float lanczosWeight(float d, float n)
		{
			return (d == 0.0) ? (1.0) : (d*d < n*n ? sinc(d) * sinc(d / n) : 0.0);
		}

		vec4 Lanczos3(vec2 texel, vec2 uv)
		{
			vec2 center = uv - (mod(uv / texel, 1.0)-0.5) * texel;
			vec2 offset = (uv - center) / texel;
			vec4 col = vec4(0,0,0,1);
			float weight = 0.0;
			for(int x = -3; x < 3; x++){
				for(int y = -3; y < 3; y++){
					float wx = lanczosWeight(float(x)-offset.x, 3.0);
					float wy = lanczosWeight(float(y)-offset.y, 3.0);
					float w = wx * wy;
					col += w * toLinear(texture(uniImage, (center + vec2(x,y) * texel) * uniTexScale));
					weight += w;
				}
			}
			col /= weight;
			return fromLinear(col);
		}

		void main() {
			vec2 uv = vec2(texCoord.x, 1.0 - texCoord.y);
			vec2 texel = vec2(1.0 / uniTexSize.x, 1.0 / uniTexSize.y);
			outColor = Lanczos3(texel, uv);
		}
	)";
	return(result);
}