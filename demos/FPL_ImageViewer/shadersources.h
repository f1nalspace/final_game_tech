// GLSL sources for the user interface of the viewer, OpenGL 3.3 core
//
// Everything is drawn as a rectangle in viewport pixels with the origin at the top-left corner and y pointing down.
// Pictures are scaled and drawn by resamplepipeline.h.

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
