#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inPosition;

layout(location = 0) out vec4 varColor;

layout(push_constant) uniform PushConsts {
    mat4 inMVP;
} pc;

void main() {
    varColor = inColor;
    gl_Position = pc.inMVP * inPosition;
}