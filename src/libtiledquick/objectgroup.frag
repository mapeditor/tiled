#version 440

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 tintAlpha;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;

vec4 tint = vec4(tintAlpha.rgb, 1);
float alpha = tintAlpha.a;

void main()
{
    vec4 texColor = texture(tex, texCoord);
    fragColor = texColor * tint * alpha;
}
