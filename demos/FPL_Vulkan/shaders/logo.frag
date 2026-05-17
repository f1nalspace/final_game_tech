#version 450

layout(location = 0) in vec2 varTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D inTexture;

void main() {
    outColor = texture(inTexture, varTexCoord);
}
