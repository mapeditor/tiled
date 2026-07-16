/*
 * mapgriditem.cpp
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

#include "mapgriditem.h"
#include "mapgridmaterial.h"

using namespace TiledQuick;

MapGridItem::MapGridItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

MapGridItem::~MapGridItem() = default;

QSGNode *MapGridItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    auto gridNode = static_cast<QSGGeometryNode *>(node);

    if (!gridNode) {
        gridNode = new QSGGeometryNode;

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
        gridNode->setGeometry(geometry);

        gridNode->setMaterial(new MapGridMaterial);

        gridNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    }

    auto *vertices = gridNode->geometry()->vertexDataAsTexturedPoint2D();
    vertices[0].set(0, 0, 0, 0);
    vertices[1].set(width(), 0, 1, 0);
    vertices[2].set(0, height(), 0, 1);
    vertices[3].set(width(), height(), 1, 1);

    auto *material = static_cast<MapGridMaterial *>(gridNode->material());
    material->mColor = mColor;
    material->mScale = mScale;
    material->mPixelWidth = width();
    material->mPixelHeight = height();
    material->mTileWidth = mTileSize.x();
    material->mTileHeight = mTileSize.y();

    gridNode->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);

    return gridNode;
}

void MapGridItem::setTileSize(const QPointF &tileSize)
{
    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged();
    update();
}

QPointF MapGridItem::tileSize() const
{
    return mTileSize;
}

void MapGridItem::setScale(const qreal &scale)
{
    if (mScale == scale)
        return;

    mScale = scale;
    emit scaleChanged();
    update();
}

qreal MapGridItem::scale() const
{
    return mScale;
}

void MapGridItem::setColor(const QColor &color)
{
    if (mColor == color)
        return;

    mColor = color;
    emit colorChanged();
    update();
}

QColor MapGridItem::color() const
{
    return mColor;
}
