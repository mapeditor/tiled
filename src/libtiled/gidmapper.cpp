/*
 * gidmapper.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gidmapper.h"

#include "tile.h"
#include "tileset.h"

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
    unsigned firstGid = 1;
    foreach (Tileset *tileset, tilesets) {
        insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
}

Cell GidMapper::gidToCell(unsigned gid, bool &ok) const
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
        QMap<unsigned, Tileset*>::const_iterator i = mFirstGidToTileset.upperBound(gid);
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

unsigned GidMapper::cellToGid(const Cell &cell) const
{
    if (cell.isEmpty())
        return 0;

    const Tileset *tileset = cell.tile->tileset();

    // Find the first GID for the tileset
    QMap<unsigned, Tileset*>::const_iterator i = mFirstGidToTileset.begin();
    QMap<unsigned, Tileset*>::const_iterator i_end = mFirstGidToTileset.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    if (i == i_end) // tileset not found
        return 0;

    unsigned gid = i.key() + cell.tile->id();
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
