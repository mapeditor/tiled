#version 440

layout(location = 0) in vec2 in_tc;
layout(location = 1) in vec4 in_tint_alpha;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;

void main()
{
    vec4 color = texture(tex, in_tc);
    vec4 tint = vec4(in_tint_alpha.rgb, 1);
    float alpha = in_tint_alpha.a;

    fragColor = color * tint * alpha;
}