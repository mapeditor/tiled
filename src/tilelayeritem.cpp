/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "tilelayeritem.h"

#include "tilelayer.h"
#include "map.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

using namespace Tiled::Internal;

TileLayerItem::TileLayerItem(TileLayer *layer):
    mLayer(layer)
{
}

QRectF TileLayerItem::boundingRect() const
{
    const Map* const map = mLayer->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    return QRectF(mLayer->x() * tileWidth,
                  mLayer->y() * tileHeight,
                  mLayer->width() * tileWidth,
                  mLayer->height() * tileHeight);
}

void TileLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget);

    const Map* const map = mLayer->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const QRectF rect =
            option->exposedRect.adjusted(0, 0, 0, mLayer->maxTileHeight());
    const int startX = (int) (rect.x() / tileWidth);
    const int startY = (int) (rect.y() / tileHeight);
    const int endX = qMin((int) rect.right() / tileWidth + 1, mLayer->width());
    const int endY = qMin((int) rect.bottom() / tileHeight + 1,
                          mLayer->height());

    // TODO: Display a border around the layer when selected
    //painter->fillRect(boundingRect(), Qt::blue);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const QPixmap& tile = mLayer->tileAt(x, y);
            painter->drawPixmap((mLayer->x() + x) * tileWidth,
                                (mLayer->y() + y + 1) * tileHeight
                                    - tile.height(),
                                tile);
        }
    }
}
