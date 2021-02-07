/*
 * relocatetile.cpp
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

#include "relocatetile.h"

#include "changeevents.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

RelocateTile::RelocateTile(TilesetDocument *tilesetDocument,
                     Tile *tile,
                     int location)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Relocate Tile"))
    , mTilesetDocument(tilesetDocument)
    , mTile(tile)
    , mLocation(location)
{
    mPrevLocation = mTilesetDocument->tileset()->findTileLocation(tile->id());
}

void RelocateTile::relocate(const Tile *tile, int location)
{
    mTilesetDocument->tileset()->moveTile(tile->id(), location);
}

} // namespace Tiled
