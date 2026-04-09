#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float scale;
    float pixelWidth;
    float pixelHeight;
    float tileWidth;
    float tileHeight;
    vec4 color;
} ubuf;

void main()
{
    float thickness = 1;
    float dashLength = thickness * 2;
    float spaceLength = dashLength;

    vec2 pixelPos = vTexCoord * vec2(ubuf.pixelWidth, ubuf.pixelHeight) * ubuf.scale + thickness * 0.5;

    if ((mod(pixelPos.x, ubuf.tileWidth * ubuf.scale) < thickness &&
         mod(pixelPos.y, dashLength + spaceLength) < dashLength) ||
        (mod(pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
         mod(pixelPos.x, dashLength + spaceLength) < dashLength))
    {
        fragColor = ubuf.color * ubuf.qt_Opacity;
    } else {
        discard;
    }
}
