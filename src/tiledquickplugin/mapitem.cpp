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

#include "isometricrenderer.h"
#include "map.h"
#include "orthogonalrenderer.h"
#include "tilelayer.h"

#include <cmath>

using namespace TiledQuick;

MapItem::MapItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mMap(0)
    , mRenderer(0)
{
}

void MapItem::setMap(Tiled::Map *map)
{
    if (mMap == map)
        return;

    mMap = map;
    emit mapChanged();

    refresh();
}

void MapItem::setVisibleArea(const QRectF &visibleArea)
{
    mVisibleArea = visibleArea;
    emit visibleAreaChanged();
}

/**
 * Determines the rectangle of visible tiles of the given tile \a layer, based
 * on the visible area of this MapItem instance.
 *
 * Only works for orthogonal maps.
 */
QRect MapItem::visibleTileArea(const Tiled::TileLayer *layer) const
{
    const int tileWidth = mMap->tileWidth();
    const int tileHeight = mMap->tileHeight();

    QMargins drawMargins = layer->drawMargins();
    drawMargins.setTop(drawMargins.top() - tileHeight);
    drawMargins.setRight(drawMargins.right() - tileWidth);

    QRectF rect = visibleArea().adjusted(-drawMargins.right(),
                                         -drawMargins.bottom(),
                                         drawMargins.left(),
                                         drawMargins.top());

    int startX = qMax((int) rect.x() / tileWidth, 0);
    int startY = qMax((int) rect.y() / tileHeight, 0);
    int endX = qMin((int) std::ceil(rect.right()) / tileWidth, layer->width() - 1);
    int endY = qMin((int) std::ceil(rect.bottom()) / tileHeight, layer->height() - 1);

    return QRect(QPoint(startX, startY), QPoint(endX, endY));
}

QRectF MapItem::boundingRect() const
{
    if (!mRenderer)
        return QRectF();

    return QRectF(QPointF(), mRenderer->mapSize());
}

QPointF MapItem::screenToTileCoords(qreal x, qreal y) const
{
    return mRenderer->screenToTileCoords(x, y);
}

QPointF MapItem::screenToTileCoords(const QPoint &position) const
{
    return mRenderer->screenToTileCoords(position);
}

QPointF MapItem::tileToScreenCoords(qreal x, qreal y) const
{
    return mRenderer->tileToScreenCoords(x, y);
}

QPointF MapItem::tileToScreenCoords(const QPoint &position) const
{
    return mRenderer->tileToScreenCoords(position);
}

QPointF MapItem::screenToPixelCoords(qreal x, qreal y) const
{
    return mRenderer->screenToPixelCoords(x, y);
}

QPointF MapItem::screenToPixelCoords(const QPoint &position) const
{
    return mRenderer->screenToPixelCoords(position);
}

QPointF MapItem::pixelToScreenCoords(qreal x, qreal y) const
{
    return mRenderer->pixelToScreenCoords(x, y);
}

QPointF MapItem::pixelToScreenCoords(const QPoint &position) const
{
    return mRenderer->pixelToScreenCoords(position);
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

    delete mRenderer;
    mRenderer = 0;

    if (!mMap)
        return;

    switch (mMap->orientation()) {
    case Tiled::Map::Isometric:
        mRenderer = new Tiled::IsometricRenderer(mMap);
        break;
    default:
        mRenderer = new Tiled::OrthogonalRenderer(mMap);
        break;
    }

    foreach (Tiled::Layer *layer, mMap->layers()) {
        if (Tiled::TileLayer *tl = layer->asTileLayer()) {
            TileLayerItem *layerItem = new TileLayerItem(tl, mRenderer, this);
            mTileLayerItems.append(layerItem);
        }
    }

    const QSize size = mRenderer->mapSize();
    setImplicitSize(size.width(), size.height());
}
