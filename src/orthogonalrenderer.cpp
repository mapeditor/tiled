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

#include "orthogonalrenderer.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled::Internal;

QSize OrthogonalRenderer::mapSize() const
{
    return QSize(map()->width() * map()->tileWidth(),
                 map()->height() * map()->tileHeight());
}

QRect OrthogonalRenderer::layerBoundingRect(const Layer *layer) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    return QRect(layer->x() * tileWidth,
                 layer->y() * tileHeight,
                 layer->width() * tileWidth,
                 layer->height() * tileHeight);
}

void OrthogonalRenderer::drawTileLayer(QPainter *painter,
                                       const TileLayer *layer,
                                       const QRectF &exposed)
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    int startX = 0;
    int startY = 0;
    int endX = layer->width();
    int endY = layer->height();

    if (!exposed.isNull()) {
        const int extraHeight = layer->maxTileHeight() - tileHeight;
        const QRectF rect = exposed.adjusted(0, 0, 0, extraHeight);

        startX = (int) rect.x() / tileWidth;
        startY = (int) rect.y() / tileHeight;
        endX = qMin((int) std::ceil(rect.right()) / tileWidth + 1, endX);
        endY = qMin((int) std::ceil(rect.bottom()) / tileHeight + 1, endY);
    }

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
}
