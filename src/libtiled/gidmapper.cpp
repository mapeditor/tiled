/*
 * gidmapper.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
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

#include "gidmapper.h"

#include "tile.h"
#include "tileset.h"
#include "map.h"

using namespace Tiled;

// Bits on the far end of the 32-bit global tile ID are used for tile flags
const int FlippedHorizontallyFlag   = 0x80000000;
const int FlippedVerticallyFlag     = 0x40000000;
const int FlippedAntiDiagonallyFlag = 0x20000000;

GidMapper::GidMapper()
{
}

GidMapper::GidMapper(const QList<Tileset *> &tilesets)
{
    uint firstGid = 1;
    foreach (Tileset *tileset, tilesets) {
        insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
}

Cell GidMapper::gidToCell(uint gid, bool &ok) const
{
    Cell result;

    // Read out the flags
    result.flippedHorizontally = (gid & FlippedHorizontallyFlag);
    result.flippedVertically = (gid & FlippedVerticallyFlag);
    result.flippedAntiDiagonally = (gid & FlippedAntiDiagonallyFlag);

    // Clear the flags
    gid &= ~(FlippedHorizontallyFlag |
             FlippedVerticallyFlag |
             FlippedAntiDiagonallyFlag);

    if (gid == 0) {
        ok = true;
    } else if (isEmpty()) {
        ok = false;
    } else {
        // Find the tileset containing this tile
        QMap<uint, Tileset*>::const_iterator i = mFirstGidToTileset.upperBound(gid);
        --i; // Navigate one tileset back since upper bound finds the next
        int tileId = gid - i.key();
        const Tileset *tileset = i.value();

        if (tileset) {
            const int columnCount = mTilesetColumnCounts.value(tileset);
            if (columnCount > 0 && columnCount != tileset->columnCount()) {
                // Correct tile index for changes in image width
                const int row = tileId / columnCount;
                const int column = tileId % columnCount;
                tileId = row * tileset->columnCount() + column;
            }

            result.tile = tileset->tileAt(tileId);
        } else {
            result.tile = 0;
        }

        ok = true;
    }

    return result;
}

uint GidMapper::cellToGid(const Cell &cell) const
{
    if (cell.isEmpty())
        return 0;

    const Tileset *tileset = cell.tile->tileset();

    // Find the first GID for the tileset
    QMap<uint, Tileset*>::const_iterator i = mFirstGidToTileset.begin();
    QMap<uint, Tileset*>::const_iterator i_end = mFirstGidToTileset.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    if (i == i_end) // tileset not found
        return 0;

    uint gid = i.key() + cell.tile->id();
    if (cell.flippedHorizontally)
        gid |= FlippedHorizontallyFlag;
    if (cell.flippedVertically)
        gid |= FlippedVerticallyFlag;
    if (cell.flippedAntiDiagonally)
        gid |= FlippedAntiDiagonallyFlag;

    return gid;
}

void GidMapper::setTilesetWidth(const Tileset *tileset, int width)
{
    if (tileset->tileWidth() == 0)
        return;

    mTilesetColumnCounts.insert(tileset, tileset->columnCountForWidth(width));
}
