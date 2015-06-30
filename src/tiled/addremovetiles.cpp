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

#include "mapdocument.h"
#include "tile.h"
#include "tileset.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

AddRemoveTiles::AddRemoveTiles(MapDocument *mapDocument,
                               Tileset *tileset,
                               int index,
                               int count,
                               const QList<Tile *> &tiles)
    : mMapDocument(mapDocument)
    , mTileset(tileset)
    , mIndex(index)
    , mCount(count)
    , mTiles(tiles)
{
}

AddRemoveTiles::~AddRemoveTiles()
{
    qDeleteAll(mTiles);
}

void AddRemoveTiles::addTiles()
{
    mTileset->insertTiles(mIndex, mTiles);
    mTiles.clear();
    mMapDocument->emitTilesetChanged(mTileset);
}

void AddRemoveTiles::removeTiles()
{
    mTiles = mTileset->tiles().mid(mIndex, mCount);
    mTileset->removeTiles(mIndex, mCount);
    mMapDocument->emitTilesetChanged(mTileset);
}


AddTiles::AddTiles(MapDocument *mapDocument,
                   Tileset *tileset,
                   const QList<Tile *> &tiles)
    : AddRemoveTiles(mapDocument,
                     tileset,
                     tileset->tileCount(),
                     tiles.count(),
                     tiles)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Tiles"));
}


RemoveTiles::RemoveTiles(MapDocument *mapDocument,
                         Tileset *tileset,
                         int index,
                         int count)
    : AddRemoveTiles(mapDocument, tileset, index, count)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Tiles"));
}

} // namespace Internal
} // namespace Tiled
