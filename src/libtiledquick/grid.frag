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
    vec2 pixelPos = vTexCoord * vec2(ubuf.pixelWidth, ubuf.pixelHeight) * ubuf.scale;

    float thickness = 1;
    float dashLength = 2;
    float spaceLength = 2;

    if (pixelPos.x >= thickness && // Hides the leftmost grid lines
       (mod(pixelPos.x, ubuf.tileWidth * ubuf.scale) < thickness &&
        mod(pixelPos.y, dashLength + spaceLength) < dashLength) ||
       (pixelPos.y >= thickness && // Hides the topmost grid lines
        mod(pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
        mod(pixelPos.x, dashLength + spaceLength) < dashLength))
    {
        fragColor = ubuf.color * ubuf.qt_Opacity;
    } else {
        discard;
    }
}
