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
#include "tileselectionmodel.h"
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
    const QRegion selection = mMapDocument->selectionModel()->selection();
    if (!(selection.isEmpty() || selection.contains(QPoint(x, y))))
        return;

    const int layerX = x - mTileLayer->x();
    const int layerY = y - mTileLayer->y();

    if (!mTileLayer->contains(layerX, layerY))
        return;

    mTileLayer->setTile(layerX, layerY, tile);
    mMapDocument->emitRegionChanged(QRegion(x, y, 1, 1));
}

void TilePainter::setTiles(int x, int y, TileLayer *tiles, const QRegion &mask)
{
    QRegion region = paintableRegion(x, y, tiles->width(), tiles->height());
    if (!mask.isEmpty())
        region &= mask;
    if (region.isEmpty())
        return;

    foreach (const QRect &rect, region.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                mTileLayer->setTile(_x - mTileLayer->x(),
                                    _y - mTileLayer->y(),
                                    tiles->tileAt(_x - x, _y - y));
            }
        }
    }

    mMapDocument->emitRegionChanged(region);
}

void TilePainter::drawTiles(int x, int y, TileLayer *tiles)
{
    const QRegion region = paintableRegion(x, y,
                                           tiles->width(),
                                           tiles->height());
    if (region.isEmpty())
        return;

    foreach (const QRect &rect, region.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                Tile * const tile = tiles->tileAt(_x - x, _y - y);
                if (!tile)
                    continue;

                mTileLayer->setTile(_x - mTileLayer->x(),
                                    _y - mTileLayer->y(),
                                    tile);
            }
        }
    }

    mMapDocument->emitRegionChanged(region);
}

void TilePainter::erase(const QRegion &region)
{
    const QRegion paintable = paintableRegion(region);
    if (paintable.isEmpty())
        return;

    foreach (const QRect &rect, paintable.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                mTileLayer->setTile(_x - mTileLayer->x(),
                                    _y - mTileLayer->y(),
                                    0);
            }
        }
    }

    mMapDocument->emitRegionChanged(paintable);
}

QRegion TilePainter::paintableRegion(const QRegion &region) const
{
    const QRegion bounds = QRegion(mTileLayer->bounds());
    QRegion intersection = bounds.intersected(region);

    const QRegion selection = mMapDocument->selectionModel()->selection();
    if (!selection.isEmpty())
        intersection &= selection;

    return intersection;
}
