/*
 * relocatetile.cpp
 * Copyright 2021, José Miguel Sánchez García <soy.jmi2k@gmail.com>
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

#include "relocatetiles.h"

#include "changeevents.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

RelocateTiles::RelocateTiles(TilesetDocument *tilesetDocument,
                             const QList<Tile *> &tiles,
                             int location)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Relocate Tile"))
    , mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
    , mLocation(location)
{
}

void RelocateTiles::undo()
{
    for (int i = mTiles.size() - 1; i >= 0; --i) {
        Tile *tile = mTiles.at(i);
        int prevLocation = mPrevLocations.at(i);

        mTilesetDocument->relocateTiles({ tile }, prevLocation);
    }
}

void RelocateTiles::redo()
{
    mPrevLocations = mTilesetDocument->relocateTiles(mTiles, mLocation);
}

} // namespace Tiled
