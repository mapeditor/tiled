/*
 * changetileobjectgroup.cpp
 * Copyright 2013, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changetileobjectgroup.h"

#include "tilesetdocument.h"
#include "objectgroup.h"
#include "tile.h"

#include <QCoreApplication>

namespace Tiled {

ChangeTileObjectGroup::ChangeTileObjectGroup(TilesetDocument *tilesetDocument,
                                             Tile *tile,
                                             std::unique_ptr<ObjectGroup> objectGroup,
                                             QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate(
                       "Undo Commands", "Change Tile Collision"),
                   parent)
    , mTilesetDocument(tilesetDocument)
    , mTile(tile)
    , mObjectGroup(std::move(objectGroup))
{
}

void ChangeTileObjectGroup::swap()
{
    mTilesetDocument->swapTileObjectGroup(mTile, mObjectGroup);
}

} // namespace Tiled
