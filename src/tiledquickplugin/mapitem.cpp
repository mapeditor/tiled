/*
 * mapitem.cpp
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

#include "mapitem.h"

#include "tilelayeritem.h"

#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <cmath>

using namespace TiledQuick;

MapItem::MapItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mMap(nullptr)
{
}

MapItem::~MapItem() = default;

void MapItem::setMap(MapRef map)
{
    if (mMap == map.mMap)
        return;

    mMap = map.mMap;
    refresh();
    emit mapChanged();
}

void MapItem::setVisibleArea(const QRectF &visibleArea)
{
    mVisibleArea = visibleArea;
    emit visibleAreaChanged();
}

QRectF MapItem::boundingRect() const
{
    if (!mRenderer)
        return QRectF();

    return mRenderer->mapBoundingRect();
}

QPointF MapItem::screenToTileCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->screenToTileCoords(x, y);
}

QPointF MapItem::screenToTileCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->screenToTileCoords(position);
}

QPointF MapItem::tileToScreenCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->tileToScreenCoords(x, y);
}

QPointF MapItem::tileToScreenCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->tileToScreenCoords(position);
}

QPointF MapItem::screenToPixelCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->screenToPixelCoords(x, y);
}

QPointF MapItem::screenToPixelCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->screenToPixelCoords(position);
}

QPointF MapItem::pixelToScreenCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->pixelToScreenCoords(x, y);
}

QPointF MapItem::pixelToScreenCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->pixelToScreenCoords(position);
}

QPointF MapItem::pixelToTileCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->pixelToTileCoords(x, y);
}

QPointF MapItem::pixelToTileCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->pixelToTileCoords(position);
}

void MapItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (mMap)
        refresh();
}

void MapItem::refresh()
{
    if (!isComponentComplete())
        return;

    qDeleteAll(mTileLayerItems);
    mTileLayerItems.clear();

    mRenderer = nullptr;

    if (!mMap)
        return;

    switch (mMap->orientation()) {
    default:
        mRenderer = std::make_unique<Tiled::OrthogonalRenderer>(mMap);
        break;
    case Tiled::Map::Isometric:
        mRenderer = std::make_unique<Tiled::IsometricRenderer>(mMap);
        break;
    case Tiled::Map::Staggered:
        mRenderer = std::make_unique<Tiled::StaggeredRenderer>(mMap);
        break;
    case Tiled::Map::Hexagonal:
        mRenderer = std::make_unique<Tiled::HexagonalRenderer>(mMap);
        break;
    }

    for (Tiled::Layer *layer : mMap->layers()) {
        if (Tiled::TileLayer *tl = layer->asTileLayer()) {
            TileLayerItem *layerItem = new TileLayerItem(tl, mRenderer.get(), this);
            mTileLayerItems.append(layerItem);
        }
    }

    const QRect rect = mRenderer->mapBoundingRect();
    setImplicitSize(rect.width(), rect.height());
}
