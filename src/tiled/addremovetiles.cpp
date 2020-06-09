/*
 * addremovetiles.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovetiles.h"

#include "tile.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

AddRemoveTiles::AddRemoveTiles(TilesetDocument *tilesetDocument,
                               const QList<Tile *> &tiles,
                               bool add)
    : mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
    , mTilesAdded(!add)
{
}

AddRemoveTiles::~AddRemoveTiles()
{
    if (!mTilesAdded)
        qDeleteAll(mTiles);
}

void AddRemoveTiles::addTiles()
{
    mTilesetDocument->addTiles(mTiles);
    mTilesAdded = true;
}

void AddRemoveTiles::removeTiles()
{
    mTilesetDocument->removeTiles(mTiles);
    mTilesAdded = false;
}


AddTiles::AddTiles(TilesetDocument *tilesetDocument,
                   const QList<Tile *> &tiles)
    : AddRemoveTiles(tilesetDocument, tiles, true)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Tiles"));
}


RemoveTiles::RemoveTiles(TilesetDocument *tilesetDocument,
                         const QList<Tile *> &tiles)
    : AddRemoveTiles(tilesetDocument, tiles, false)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Tiles"));
}

} // namespace Tiled
