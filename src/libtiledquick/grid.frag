#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;

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

const float sqrt3 = 1.7320508;

const float thickness = 1;
const float dashLength = thickness * 2;
const float spaceLength = dashLength;

bool posInOrthogonalGrid(vec2 pixelPos)
{
    pixelPos = pixelPos * ubuf.scale + 0.5 * thickness;

    if ((mod(pixelPos.x, ubuf.tileWidth * ubuf.scale) < thickness &&
         mod(pixelPos.y, dashLength + spaceLength) < dashLength) ||
        (mod(pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
         mod(pixelPos.x, dashLength + spaceLength) < dashLength))
        return true;

    return false;
}

bool posInIsometricGrid(vec2 pixelPos, bool staggered)
{
    pixelPos = pixelPos * ubuf.scale;

    const float leftSideLength = ubuf.mapHeight * ubuf.tileHeight;

    // If not staggered and out of bounds, don't draw
    if (!staggered &&
        (pixelPos.x + pixelPos.y < 0.5 * leftSideLength * ubuf.scale ||
         pixelPos.x + pixelPos.y > (ubuf.pixelHeight + ubuf.pixelWidth - 0.5 * leftSideLength) * ubuf.scale + thickness ||
         abs(pixelPos.x - pixelPos.y) > 0.5 * leftSideLength * ubuf.scale + thickness))
        return false;

    if (staggered)
        pixelPos = pixelPos - 0.5 * vec2(ubuf.tileWidth, 0) * ubuf.scale;

    if ((mod(pixelPos.x + pixelPos.y, ubuf.tileWidth * ubuf.scale) < thickness &&
         mod(pixelPos.x - pixelPos.y, (dashLength + spaceLength) * 2) < dashLength * 2) ||
        (mod(pixelPos.x - pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
         mod(pixelPos.x + pixelPos.y, (dashLength + spaceLength) * 2) < dashLength * 2))
        return true;

    return false;
}

bool posInObliqueGrid(vec2 pixelPos)
{
    // Map pixelPos to skewed coordinates
    vec2 scaledSkew = vec2(ubuf.skew.x / ubuf.tileHeight, ubuf.skew.y / ubuf.tileWidth);
    float det = 1.0 - (scaledSkew.x * scaledSkew.y);
    mat2 invShear = mat2(
        1.0, -scaledSkew.y,
       -scaledSkew.x,  1.0
    ) / det;

    pixelPos = invShear * pixelPos;

    // If out of bounds, don't draw
    if (pixelPos.x < 0 ||
        pixelPos.x > ubuf.mapWidth * ubuf.tileWidth + thickness ||
        pixelPos.y < 0 ||
        pixelPos.y > ubuf.mapHeight * ubuf.tileHeight + thickness)
        return false;

    pixelPos = pixelPos * ubuf.scale;

    if ((mod(pixelPos.x, ubuf.tileWidth * ubuf.scale) < thickness &&
         mod(pixelPos.y, dashLength + spaceLength) < dashLength) ||
        (mod(pixelPos.y, ubuf.tileHeight * ubuf.scale) < thickness &&
         mod(pixelPos.x, dashLength + spaceLength) < dashLength))
        return true;

    return false;
}

bool posInHexagonalGrid(vec2 pixelPos)
{
    pixelPos = pixelPos - vec2(ubuf.tileWidth/2, ubuf.tileHeight/2);

    vec2 size = vec2(ubuf.tileWidth, ubuf.tileHeight);
    vec2 gridSpacing = vec2(ubuf.tileWidth, ubuf.tileHeight * 1.5);

    // Need to test 2 grid centers because every other row is offset by gridSpacing/2
    vec2 a = mod(pixelPos + gridSpacing * 0.5, gridSpacing) - gridSpacing * 0.5;
    vec2 b = mod(pixelPos, gridSpacing) - gridSpacing * 0.5;

    vec2 scaleMetric = vec2(sqrt3, 2.0) / size;
    vec2 sa = a * scaleMetric;
    vec2 sb = b * scaleMetric;

    vec2 p = dot(sa, sa) < dot(sb, sb) ? a : b;
    vec2 pos = abs(p);
    vec2 n = normalize(vec2(size.y, size.x * 2.0));

    float flatDist = pos.x - size.x * 0.5;
    float cornerDist = dot(pos, n) - (size.y * 0.5 * n.y);
    float dist = max(flatDist, cornerDist);

    // Exit early if pixel does not fall within line bounds
    if (abs(dist) * ubuf.scale > (thickness)) {
        return false;
    }

    // TODO: May be able to improve line consistency by increasing thickness based on angle, simialar to the isometric shader
    float lineDistance = 0.0;
    // Right edge
    if (p.x >= 0.0 && p.y >= -size.y * 0.25 && p.y < size.y * 0.25) {
        lineDistance = pixelPos.y;
    }
    // Bottom right edge
    else if (p.x >= 0.0 && p.y >= size.y * 0.25) {
        vec2 direction = normalize(vec2(-size.x, size.y * 0.5));
        lineDistance = dot(pixelPos, direction);
    }
    // Bottom left edge
    else if (p.x < 0.0 && p.y >= size.y * 0.25) {
        vec2 direction = normalize(vec2(-size.x, -size.y * 0.5));
        lineDistance = dot(pixelPos, direction);
    }
    // Top, top right, and top left edges are drawn by neighboring cells
    else {
        return false;
    }

    float dashPeriod = dashLength + spaceLength;
    return mod(lineDistance * ubuf.scale, dashPeriod) < dashLength;
}

void main()
{
    vec2 pixelPos = vTexCoord * vec2(ubuf.pixelWidth, ubuf.pixelHeight);

    bool posInGrid = false;
    // Synchroniezed with Map::Orientation
    switch (ubuf.orientation) {
    case 0: // Unknown
    case 1: // Orthogonal
        posInGrid = posInOrthogonalGrid(pixelPos);
        break;
    case 2: // Isometric
        posInGrid = posInIsometricGrid(pixelPos, false);
        break;
    case 3: // Staggered
        posInGrid = posInIsometricGrid(pixelPos, true);
        break;
    case 4: // Hexagonal
        posInGrid = posInHexagonalGrid(pixelPos);
        break;
    case 5: // Oblique
        posInGrid = posInObliqueGrid(pixelPos);
        break;
    default:
        break;
    }

    if (posInGrid) {
        fragColor = ubuf.color * ubuf.qt_Opacity;
        return;
    }

    discard;
}
