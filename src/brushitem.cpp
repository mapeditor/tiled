/*
 * Tiled Map Editor (Qt)
 * Copyright 2008-2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "brushitem.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

BrushItem::BrushItem():
    mMapDocument(0),
    mTileLayer(0),
    mExtend(0)
{
}

void BrushItem::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    // The tiles in the stamp may no longer be valid
    setTileLayer(0);
    updateBoundingRect();
}

void BrushItem::setTileLayer(TileLayer *tileLayer)
{
    if (mTileLayer == tileLayer)
        return;

    mTileLayer = tileLayer;
    updateBoundingRect();
    update();
}

void BrushItem::setTilePos(int x, int y)
{
    if (!mMapDocument)
        return;

    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    setPos(x * tileWidth, y * tileHeight);
}

void BrushItem::setTileSize(int width, int height)
{
    if (mWidth == width && mHeight == height)
        return;

    mWidth = width;
    mHeight = height;
    updateBoundingRect();
}

QRectF BrushItem::boundingRect() const
{
    return mBoundingRect;
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    QColor highlight = QApplication::palette().highlight().color();
    highlight.setAlpha(64);

    if (mTileLayer) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        mMapDocument->renderer()->drawTileLayer(painter, mTileLayer);
        painter->setOpacity(opacity);

        QRectF redraw = option->exposedRect;
        if (redraw.top() < 0)
            redraw.setTop(0);

        painter->fillRect(redraw, highlight);
    } else {
        painter->fillRect(option->exposedRect, highlight);
    }
}

void BrushItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    const Map *map = mMapDocument->map();
    int w = mWidth;
    int h = mHeight;

    // Update the amount of pixels tiles extend above the brush
    if (mTileLayer) {
        w = mTileLayer->width();
        h = mTileLayer->height();

        mExtend = qMax(0, mTileLayer->maxTileHeight() - map->tileHeight());
    } else {
        mExtend = 0;
    }

    mBoundingRect = QRectF(0, -mExtend,
                           map->tileWidth() * w,
                           map->tileHeight() * h + mExtend);
}
