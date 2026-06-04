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

const float thickness = 1;
const float dashLength = thickness * 2;
const float spaceLength = dashLength;

bool posInGrid(vec2 pixelPos)
{
    if ((mod(pixelPos.x, ubuf.tileWidth * ubuf.scale) < thickness &&
         mod(pixelPos.y, dashLength + spaceLength) < dashLength) ||
        (mod(pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
         mod(pixelPos.x, dashLength + spaceLength) < dashLength))
    {
        return true;
    }
    return false;
}

void main()
{
    vec2 pixelPos = vTexCoord * vec2(ubuf.pixelWidth, ubuf.pixelHeight) * ubuf.scale + ceil(thickness * 0.5);

    if (posInGrid(floor(pixelPos)))
    {
        fragColor = ubuf.color * ubuf.qt_Opacity;
        return;
    }

    discard;
}
