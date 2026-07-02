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

#include <QPainterPath>
#include <QApplication>
#include <QPalette>

using namespace TiledQuick;

RegionOverlay::RegionOverlay(QQuickItem *parent)
    : QQuickItem(parent)
    , mTileSize(0, 0)
    , mRegion(QRegion())
    , mColor(QApplication::palette().highlight().color())
{
}

RegionOverlay::~RegionOverlay() = default;

QList<QPolygonF> RegionOverlay::polygons() const
{
    QPainterPath path;

    for (const QRect &r : mRegion.rects())
        path.addRect(r);

    QList<QPolygonF> polygons = path.simplified().toFillPolygons();

    QTransform transform;
    transform.scale(mTileSize.x(), mTileSize.y());

    for (QPolygonF &polygon : polygons)
        polygon = polygon * transform;

    return polygons;
}

QColor RegionOverlay::strokeColor() const
{
    return mColor;
}

QColor RegionOverlay::fillColor() const
{
    QColor fillColor = mColor;
    fillColor.setAlpha(64);
    return fillColor;
}

QPointF RegionOverlay::tileSize() const
{
    return mTileSize;
}

void RegionOverlay::setTileSize(const QPointF &tileSize)
{
    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged();
}

QRegion RegionOverlay::region() const
{
    return mRegion;
}

void RegionOverlay::setRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    emit regionChanged();
}
