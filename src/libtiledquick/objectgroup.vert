#version 440

layout(location = 0) in vec2 in_globalPos;
layout(location = 1) in vec2 in_texCoord;
layout(location = 2) in vec4 in_tintAlpha;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec4 tintAlpha;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    float opacity;
} ubuf;

void main()
{
    texCoord = in_texCoord;
    tintAlpha = vec4(in_tintAlpha.rgb, in_tintAlpha.a * ubuf.opacity);
    gl_Position = ubuf.matrix * vec4(in_globalPos, 0.0, 1.0);
}
