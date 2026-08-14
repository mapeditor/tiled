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

#include "mapdocument.h"
#include "isometricrenderer.h"
#include "staggeredrenderer.h"
#include "hexagonalrenderer.h"
#include "obliquerenderer.h"

#include "regionoverlay.h"

#include <QPainterPath>
#include <QApplication>
#include <QPalette>

using namespace TiledQuick;

RegionOverlay::RegionOverlay(QQuickItem *parent)
    : QQuickItem(parent)
    , mTileSize(0, 0)
    , mValidColor(QApplication::palette().highlight().color())
    , mInvalidColor(QColor(255,0,0))
    , mRegionAlpha(64)
    , mMapDocument(nullptr)
{
}

RegionOverlay::~RegionOverlay() = default;

QColor RegionOverlay::validFillColor() const
{
    QColor fillColor = mValidColor;
    fillColor.setAlpha(mRegionAlpha);
    return fillColor;
}

QColor RegionOverlay::invalidFillColor() const
{
    QColor fillColor = mInvalidColor;
    fillColor.setAlpha(mRegionAlpha);
    return fillColor;
}

void RegionOverlay::setTileSize(const QPointF &tileSize)
{
    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged();
}

void RegionOverlay::setRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    emit regionChanged();
}

void RegionOverlay::setMapRect(const QRect &rect)
{
    if (mMapRect == rect)
        return;

    mMapRect = rect;
    emit mapRectChanged();
}

void RegionOverlay::setRegionAlpha(const int &alpha)
{
    if (mRegionAlpha == alpha)
        return;

    mRegionAlpha = alpha;
    emit regionAlphaChanged();
}

void RegionOverlay::setMapDocument(Tiled::MapDocument *document)
{
    if (mMapDocument == document)
        return;

    mMapDocument = document;
    emit mapDocumentChanged();
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
    if (!mMapDocument)
        return {};

    QPainterPath path;

    switch (mMapDocument->map()->orientation())
    {
    case Tiled::Map::Orientation::Orthogonal: {
        QTransform transform;
        transform.scale(mTileSize.x(), mTileSize.y());

        for (const QRect &r : region)
            path.addRect(transform.mapRect(r));

        break;
    }
    case Tiled::Map::Orientation::Isometric: {
        auto *renderer = dynamic_cast<Tiled::IsometricRenderer*>(mMapDocument->renderer());

        for (const QRect &r : region)
            path.addPolygon(renderer->tileRectToScreenPolygon(r));
        break;
    }
    case Tiled::Map::Orientation::Staggered: {
        auto *renderer = dynamic_cast<Tiled::StaggeredRenderer*>(mMapDocument->renderer());

        for (const QRect &r : region)
            for (int y = r.top(); y <= r.bottom(); ++y)
                for (int x = r.left(); x <= r.right(); ++x)
                    path.addPolygon(renderer->tileToScreenPolygon(x, y));

        break;
    }
    case Tiled::Map::Orientation::Hexagonal: {
        auto *renderer = dynamic_cast<Tiled::HexagonalRenderer*>(mMapDocument->renderer());

        for (const QRect &r : region)
            for (int y = r.top(); y <= r.bottom(); ++y)
                for (int x = r.left(); x <= r.right(); ++x)
                    path.addPolygon(renderer->tileToScreenPolygon(x, y));

        break;
    }
    case Tiled::Map::Orientation::Oblique: {
        auto *renderer = dynamic_cast<Tiled::ObliqueRenderer*>(mMapDocument->renderer());

        for (const QRect &r : region)
            path.addPolygon(renderer->tileRectToScreenPolygon(r));
        break;
    }
    case Tiled::Map::Orientation::Unknown:
    default:
        break;
    }

    QList<QPolygonF> polygons = path.simplified().toSubpathPolygons();

    return polygons;
}
