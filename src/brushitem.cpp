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
#include "painttile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

BrushItem::BrushItem():
    mTileX(0),
    mTileY(0),
    mMapDocument(0),
    mTile(0),
    mExtend(0),
    mPainting(false)
{
}

void BrushItem::setMapDocument(MapDocument *mapDocument)
{
    // A different map may have a different tile size
    prepareGeometryChange();
    mMapDocument = mapDocument;
}

void BrushItem::setTile(Tile *tile)
{
    if (mTile == tile)
        return;

    prepareGeometryChange();
    mTile = tile;
    updateExtend();
    update();
}

void BrushItem::setTilePos(int x, int y)
{
    if (mTileX == x && mTileY == y)
        return;

    mTileX = x;
    mTileY = y;

    if (!mMapDocument)
        return;

    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    // Update the pixel position
    setPos(x * tileWidth, y * tileHeight);

    if (mPainting)
        doPaint();
}

void BrushItem::beginPaint()
{
    mPainting = true;
    doPaint();
}

void BrushItem::endPaint()
{
    mPainting = false;
}

void BrushItem::doPaint()
{
    // This method shouldn't be called when current layer is not a tile layer
    const int currentLayerIndex = mMapDocument->currentLayer();
    Layer *currentLayer = mMapDocument->map()->layerAt(currentLayerIndex);
    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    Q_ASSERT(tileLayer);

    const int layerX = mTileX - tileLayer->x();
    const int layerY = mTileY - tileLayer->y();

    if (!tileLayer->contains(layerX, layerY)
        || tileLayer->tileAt(layerX, layerY) == mTile)
        return;

    PaintTile *paintTile = new PaintTile(mMapDocument, tileLayer,
                                         mTileX, mTileY, mTile);
    mMapDocument->undoStack()->push(paintTile);
}

void BrushItem::updateExtend()
{
    const Map *map = mMapDocument->map();
    mExtend = mTile ? (mTile->height() - map->tileHeight()) : 0;
}

QRectF BrushItem::boundingRect() const
{
    if (!mMapDocument)
        return QRectF();

    const Map *map = mMapDocument->map();
    return QRectF(0, -mExtend, map->tileWidth(), map->tileHeight() + mExtend);
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
    Q_UNUSED(widget);

    if (mTile) {
        const QPixmap img = mTile->image();
        const int tileHeight = mMapDocument->map()->tileHeight();
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        painter->drawPixmap(0, tileHeight - img.height(), img);
        painter->setOpacity(opacity);
    }

    QRectF redraw = option->exposedRect;
    if (redraw.top() < 0)
        redraw.setTop(0);

    QColor red(Qt::red);
    red.setAlpha(64);
    painter->fillRect(redraw, red);
}
