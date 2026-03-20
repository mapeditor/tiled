#include "mapgridmaterial.h"

using namespace TiledQuick;

struct GridUniformBuffer
{
    float qt_Matrix[16];

    float qt_Opacity;
    float scale;
    float pixelWidth;
    float pixelHeight;

    float tileWidth;
    float tileHeight;
    // Needed to fill the remaining 8 bytes in this 16-byte "block".
    float _padding[2];

    float color[4];
};

class MapGridShader : public QSGMaterialShader
{
public:
    MapGridShader()
    {
        setShaderFileName(VertexStage, QStringLiteral(":/grid.vert.qsb"));
        setShaderFileName(FragmentStage, QStringLiteral(":/grid.frag.qsb"));
    }

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *) override
    {
        QByteArray *buffer = state.uniformData();
        auto *u = reinterpret_cast<GridUniformBuffer *>(buffer->data());
        auto *material = static_cast<MapGridMaterial *>(newMaterial);

        memcpy(u->qt_Matrix, state.combinedMatrix().constData(), 64);
        u->qt_Opacity = state.opacity();

        u->scale = material->mScale;

        u->pixelWidth = material->mPixelWidth;
        u->pixelHeight = material->mPixelHeight;

        u->tileWidth = material->mTileWidth;
        u->tileHeight = material->mTileHeight;

        u->color[0] = material->mColor.redF();
        u->color[1] = material->mColor.greenF();
        u->color[2] = material->mColor.blueF();
        u->color[3] = material->mColor.alphaF();

        return true;
    }
};

MapGridMaterial::MapGridMaterial()
{
    setFlag(RequiresFullMatrix);
    setFlag(QSGMaterial::Blending);
}

MapGridMaterial::~MapGridMaterial() = default;

QSGMaterialShader *MapGridMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new MapGridShader();
}

int MapGridMaterial::compare(const QSGMaterial *other) const
{
    auto *m = static_cast<const MapGridMaterial *>(other);
    return (mColor == m->mColor &&
            qFuzzyCompare(mScale, m->mScale) &&
            qFuzzyCompare(mPixelWidth, m->mPixelWidth) ? 0 : 1);
}
