/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "maprenderer.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

MapRenderer::MapRenderer(Map *map)
    : mMap(map)
{
}

QRect MapRenderer::layerBoundingRect(Layer *layer) const
{
    const int tileWidth = mMap->tileWidth();
    const int tileHeight = mMap->tileHeight();

    return QRect(layer->x() * tileWidth,
                 layer->y() * tileHeight,
                 layer->width() * tileWidth,
                 layer->height() * tileHeight);
}

void MapRenderer::drawTileLayer(QPainter *painter, TileLayer *layer,
                                const QRect &exposed)
{
    const int tileWidth = mMap->tileWidth();
    const int tileHeight = mMap->tileHeight();

    int startX = 0;
    int startY = 0;
    int endX = layer->width();
    int endY = layer->height();

    if (!exposed.isNull()) {
        const int extraHeight = layer->maxTileHeight() - tileHeight;
        const QRect rect = exposed.adjusted(0, 0, 0, extraHeight);

        startX = rect.x() / tileWidth;
        startY = rect.y() / tileHeight;
        endX = qMin(rect.right() / tileWidth + 1, endX);
        endY = qMin(rect.bottom() / tileHeight + 1, endY);
    }

    const qreal opacity = painter->opacity();
    painter->setOpacity(opacity * layer->opacity());

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile *tile = layer->tileAt(x, y);
            if (!tile)
                continue;

            const QPixmap &img = tile->image();
            painter->drawPixmap(x * tileWidth,
                                (y + 1) * tileHeight - img.height(),
                                img);
        }
    }

    painter->setOpacity(opacity);
}
