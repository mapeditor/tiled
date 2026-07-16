#version 440

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_tc;
layout(location = 2) in vec4 in_tint_alpha;

layout(location = 0) out vec2 out_tc;
layout(location = 1) out vec4 out_tint_alpha;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
} ubuf;

void main()
{
    out_tc = in_tc;
    out_tint_alpha = vec4(in_tint_alpha.rgb, in_tint_alpha.a * ubuf.opacity);
    gl_Position = ubuf.matrix * vec4(in_pos, 0.0, 1.0);
}