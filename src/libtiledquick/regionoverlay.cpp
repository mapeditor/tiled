/*
 * regionoverlay.cpp
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

#include "regionoverlay.h"

#include <QSGMaterial>
//#include "regionoverlaymaterial"

using namespace TiledQuick;

RegionOverlay::RegionOverlay(QQuickItem *parent)
    : QQuickItem{parent}
{

}

RegionOverlay::~RegionOverlay() = default;

QSGNode *RegionOverlay::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    auto regionNode = static_cast<QSGGeometryNode *>(node);

    return regionNode;
}

void RegionOverlay::setTileSize(const QPointF &tileSize)
{
    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged();
    update();
}

QPointF RegionOverlay::tileSize() const
{
    return mTileSize;
}

void RegionOverlay::setScale(const qreal &scale)
{
    if (mScale == scale)
        return;

    mScale = scale;
    emit scaleChanged();
    update();
}

qreal RegionOverlay::scale() const
{
    return mScale;
}

void RegionOverlay::setValidColor(const QColor &color)
{
    if (mValidColor == color)
        return;

    mValidColor = color;
    emit validColorChanged();
    update();
}

QColor RegionOverlay::validColor() const
{
    return mValidColor;
}

void RegionOverlay::setInvalidColor(const QColor &color)
{
    if (mInvalidColor == color)
        return;

    mInvalidColor = color;
    emit invalidColorChanged();
    update();
}

QColor RegionOverlay::invalidColor() const
{
    return mInvalidColor;
}
