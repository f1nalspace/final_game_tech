#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 varTexCoord;

layout(push_constant) uniform PushConsts {
    mat4 inMVP;
} pc;

void main() {
    varTexCoord = inTexCoord;
    gl_Position = pc.inMVP * vec4(inPosition, 0.0, 1.0);
}
