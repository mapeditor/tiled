/*
 * tilesnode.h
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include <QSGGeometryNode>
#include <QSGTextureMaterial>

namespace TiledQuick {

struct TileData {
    float x;
    float y;
    float width;
    float height;
    float tx;
    float ty;
    bool flippedHorizontally;
    bool flippedVertically;
};

class TilesNode : public QSGGeometryNode
{
public:
    enum {
        MaxTileCount = 65536 / 6
    };

    TilesNode(QSGTexture *texture, const QVector<TileData> &tileData);

    QSGTexture *texture() const;

private:
    void processTileData(const QVector<TileData> &tileData);

    QSGGeometry mGeometry;
    QSGTextureMaterial mMaterial;
    QSGOpaqueTextureMaterial mOpaqueMaterial;
};

inline QSGTexture *TilesNode::texture() const
{
    return mMaterial.texture();
}

} // namespace TiledQuick
