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
    , mMapRect(QRect())
    , mValidColor(QApplication::palette().highlight().color())
    , mInvalidColor(QColor(255,0,0))
    , mRegionAlpha(64)
{
}

RegionOverlay::~RegionOverlay() = default;

QColor RegionOverlay::validStrokeColor() const
{
    return mValidColor;
}

QColor RegionOverlay::validFillColor() const
{
    QColor fillColor = mValidColor;
    fillColor.setAlpha(mRegionAlpha);
    return fillColor;
}

QColor RegionOverlay::invalidStrokeColor() const
{
    return mInvalidColor;
}

QColor RegionOverlay::invalidFillColor() const
{
    QColor fillColor = mInvalidColor;
    fillColor.setAlpha(mRegionAlpha);
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

QRect RegionOverlay::mapRect() const
{
    return mMapRect;
}

void RegionOverlay::setMapRect(const QRect &rect)
{
    if (mMapRect == rect)
        return;

    mMapRect = rect;
    emit mapRectChanged();
}

int RegionOverlay::regionAlpha() const
{
    return mRegionAlpha;
}

void RegionOverlay::setRegionAlpha(const int &alpha)
{
    if (mRegionAlpha == alpha)
        return;

    mRegionAlpha = alpha;
    emit regionAlphaChanged();
}

QList<QPolygonF> RegionOverlay::validPolygons() const
{
    QRegion insideMapRegion;
    if (mMapRect == QRect())
        insideMapRegion = mRegion;
    else
        insideMapRegion = mRegion.intersected(mMapRect);

    return polygons(insideMapRegion);
}

QList<QPolygonF> RegionOverlay::invalidPolygons() const
{
    if (mMapRect == QRect())
        return QList<QPolygonF>();

    QRegion outsideMapRegion = mRegion.subtracted(mMapRect);

    return polygons(outsideMapRegion);
}

QList<QPolygonF> RegionOverlay::polygons(const QRegion &region) const
{
    QPainterPath path;
    for (const QRect &r : region.rects())
        path.addRect(r);

    QList<QPolygonF> polygons = path.simplified().toSubpathPolygons();

    QTransform transform;
    transform.scale(mTileSize.x(), mTileSize.y());

    for (QPolygonF &polygon : polygons)
        polygon = transform.map(polygon);

    return polygons;
}
