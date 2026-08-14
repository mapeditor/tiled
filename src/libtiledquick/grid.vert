#version 440

layout(location = 0) in vec4 qt_Vertex;
layout(location = 1) in vec2 qt_MultiTexCoord0;

layout(location = 0) out vec2 vTexCoord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float scale;
    float pixelWidth;
    float pixelHeight;
    float mapWidth;
    float mapHeight;
    float tileWidth;
    float tileHeight;
    vec4 color;
    vec2 skew;
    int orientation;
} ubuf;

void main()
{
    vTexCoord = qt_MultiTexCoord0;
    gl_Position = ubuf.qt_Matrix * qt_Vertex;
}
