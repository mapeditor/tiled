/*
 * swaptiles.cpp
 * Copyright 2015, Alexander "theHacker" Münch <git@thehacker.biz>
 *
 * This file is part of Tiled.
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

#include "swaptiles.h"

#include "mapdocument.h"
#include "tileset.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

SwapTiles::SwapTiles(MapDocument *mapDocument,
                     TileLayer *tileLayer,
                     Tile *tile1,
                     Tile *tile2)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Swap Tiles"))
    , mMapDocument(mapDocument)
    , mTileLayer(tileLayer)
    , mTile1(tile1)
    , mTile2(tile2)
{}

void SwapTiles::swap()
{
    for (int y = 0; y < mTileLayer->height(); y++) {
        for (int x = 0; x < mTileLayer->width(); x++) {
            const Cell &cell = mTileLayer->cellAt(x, y);

            if (cell.tile == mTile1) {
                Cell swapCell = cell;
                swapCell.tile = mTile2;
                mTileLayer->setCell(x, y, swapCell);
            }
            else if (cell.tile == mTile2) {
                Cell swapCell = cell;
                swapCell.tile = mTile1;
                mTileLayer->setCell(x, y, swapCell);
            }
        }
    }
}

} // namespace Internal
} // namespace Tiled
