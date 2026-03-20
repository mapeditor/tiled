#pragma once

#include <QSGMaterial>

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapGridMaterial : public QSGMaterial
{
public:
    MapGridMaterial();
    ~MapGridMaterial() override;

    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QSGMaterialType *type() const override { static QSGMaterialType t; return &t; }

    int compare(const QSGMaterial *other) const override;

    QColor mColor = Qt::black;
    float mScale = 1;
    float mPixelWidth = 0;
    float mPixelHeight = 0;
    float mTileWidth = 0;
    float mTileHeight = 0;
};

}
