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

#include "tilepainter.h"

#include "mapdocument.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

TilePainter::TilePainter(MapDocument *mapDocument, TileLayer *tileLayer)
    : mMapDocument(mapDocument)
    , mTileLayer(tileLayer)
{
}

Tile *TilePainter::tileAt(int x, int y)
{
    const int layerX = x - mTileLayer->x();
    const int layerY = y - mTileLayer->y();

    if (!mTileLayer->contains(layerX, layerY))
        return 0;

    return mTileLayer->tileAt(layerX, layerY);
}

void TilePainter::setTile(int x, int y, Tile *tile)
{
    const int layerX = x - mTileLayer->x();
    const int layerY = y - mTileLayer->y();

    if (!mTileLayer->contains(layerX, layerY))
        return;

    mTileLayer->setTile(layerX, layerY, tile);
    mMapDocument->emitRegionChanged(QRegion(x, y, 1, 1));
}

void TilePainter::setTiles(int x, int y, TileLayer *tiles)
{
    const QRect area = intersectionWithLayer(x, y,
                                             tiles->width(),
                                             tiles->height());
    if (area.isEmpty())
        return;

    for (int _x = area.left(); _x <= area.right(); ++_x) {
        for (int _y = area.top(); _y <= area.bottom(); ++_y) {
            mTileLayer->setTile(_x - mTileLayer->x(),
                                _y - mTileLayer->y(),
                                tiles->tileAt(_x - x, _y - y));
        }
    }

    mMapDocument->emitRegionChanged(area);
}

void TilePainter::drawTiles(int x, int y, TileLayer *tiles)
{
    const QRect area = intersectionWithLayer(x, y,
                                             tiles->width(),
                                             tiles->height());
    if (area.isEmpty())
        return;

    for (int _x = area.left(); _x <= area.right(); ++_x) {
        for (int _y = area.top(); _y <= area.bottom(); ++_y) {
            Tile * const tile = tiles->tileAt(_x - x, _y - y);
            if (!tile)
                continue;

            mTileLayer->setTile(_x - mTileLayer->x(),
                                _y - mTileLayer->y(),
                                tile);
        }
    }

    mMapDocument->emitRegionChanged(area);
}

QRect TilePainter::intersectionWithLayer(int x, int y,
                                         int width, int height) const
{
    return mTileLayer->bounds().intersected(QRect(x, y, width, height));
}
