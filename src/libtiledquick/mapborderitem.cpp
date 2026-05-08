/*
 * mapborderitem.cpp
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

#include "mapborderitem.h"

#include <QSGFlatColorMaterial>

using namespace TiledQuick;

MapBorderItem::MapBorderItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

MapBorderItem::~MapBorderItem() = default;

QSGNode *MapBorderItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    auto borderNode = static_cast<QSGGeometryNode *>(node);

    if (!borderNode) {
        borderNode = new QSGGeometryNode;

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 5);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);
        borderNode->setGeometry(geometry);

        borderNode->setMaterial(new QSGFlatColorMaterial);

        borderNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    }

    auto *geometry = borderNode->geometry();
    auto *vertex = geometry->vertexDataAsPoint2D();
    vertex[0].set(0, 0);
    vertex[1].set(width(), 0);
    vertex[2].set(width(), height());
    vertex[3].set(0, height());
    vertex[4].set(0, 0);
    borderNode->markDirty(QSGNode::DirtyGeometry);

    static_cast<QSGFlatColorMaterial *>(borderNode->material())->setColor(mColor);

    return borderNode;
}

void MapBorderItem::setColor(const QColor &color)
{
    if (mColor != color) {
        mColor = color;
        emit colorChanged();
        update();
    }
}

QColor MapBorderItem::color() const
{
    return mColor;
}
