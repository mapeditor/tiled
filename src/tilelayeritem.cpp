/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#include "layer.h"
#include "map.h"

#include <QPainter>

using namespace Tiled::Internal;

TileLayerItem::TileLayerItem(Layer *layer):
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
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const Map* const map = mLayer->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    //painter->fillRect(boundingRect(), Qt::blue);

    // TODO: Only paint option->exposedRect (huge optimization)
    for (int y = 0; y < mLayer->height(); ++y) {
        for (int x = 0; x < mLayer->width(); ++x) {
            const QImage& tile = mLayer->tileAt(x, y);
            painter->drawImage((mLayer->x() + x) * tileWidth,
                               (mLayer->y() + y) * tileHeight, tile);
        }
    }
}
