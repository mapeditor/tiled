/*
 * mapgridmaterial.cpp
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mapgridmaterial.h"

using namespace TiledQuick;

struct alignas(4) GridUniformBuffer
{
    float qt_Matrix[16];

    float qt_Opacity;
    float scale;
    float pixelWidth;
    float pixelHeight;

    float mapWidth;
    float mapHeight;
    float tileWidth;
    float tileHeight;

    float color[4];

    float skew[2];
    int orientation;
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

        if (state.isMatrixDirty())
            memcpy(u->qt_Matrix, state.combinedMatrix().constData(), 64);

        u->qt_Opacity = state.opacity();

        u->color[0] = material->mColor.redF();
        u->color[1] = material->mColor.greenF();
        u->color[2] = material->mColor.blueF();
        u->color[3] = material->mColor.alphaF();
        u->scale = material->mScale;
        u->pixelWidth = material->mPixelWidth;
        u->pixelHeight = material->mPixelHeight;
        u->mapWidth = material->mMapWidth;
        u->mapHeight = material->mMapHeight;
        u->tileWidth = material->mTileWidth;
        u->tileHeight = material->mTileHeight;
        u->skew[0] = material->mSkew.x();
        u->skew[1] = material->mSkew.y();
        u->orientation = material->mOrientation;

        return true;
    }
};

MapGridMaterial::MapGridMaterial()
{
    setFlag(RequiresFullMatrix | Blending);
}

MapGridMaterial::~MapGridMaterial() = default;

QSGMaterialShader *MapGridMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new MapGridShader;
}

int MapGridMaterial::compare(const QSGMaterial *other) const
{
    auto *m = static_cast<const MapGridMaterial *>(other);
    return ((
                mColor == m->mColor &&
                qFuzzyCompare(mScale, m->mScale) &&
                qFuzzyCompare(mPixelWidth, m->mPixelWidth) &&
                qFuzzyCompare(mPixelHeight, m->mPixelHeight) &&
                qFuzzyCompare(mMapWidth, m->mMapWidth) &&
                qFuzzyCompare(mMapHeight, m->mMapHeight) &&
                qFuzzyCompare(mTileWidth, m->mTileWidth) &&
                qFuzzyCompare(mTileHeight, m->mTileHeight) &&
                qFuzzyCompare(mSkew, m->mSkew) &&
                mOrientation == m->mOrientation
                )? 0 : 1
            );
}
