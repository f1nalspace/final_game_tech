#version 450

layout(location = 0) in vec4 varColor;
layout(location = 0) out vec4 outColor;

// inFrame sits at offset 64, right after the vertex shader's mat4 inMVP,
// so the vertex and fragment push-constant ranges never overlap in memory.
layout(push_constant) uniform PushConsts {
    layout(offset = 64) int inFrame;
} pc;

const uint k = 1103515245U;

vec3 hash(uvec3 x) {
    x = ((x >> 8U) ^ x.yzx) * k;
    x = ((x >> 8U) ^ x.yzx) * k;
    x = ((x >> 8U) ^ x.yzx) * k;
    return vec3(x) * (1.0 / float(0xffffffffU));
}

void main() {
    uvec3 p = uvec3(gl_FragCoord.xy, pc.inFrame);
    vec4 randomColor = vec4(hash(p), 1.0);
    outColor = randomColor * varColor;
}