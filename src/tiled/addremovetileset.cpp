/*
 * addremovetileset.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovetileset.h"

#include "map.h"
#include "mapdocument.h"
#include "tilesetmanager.h"

using namespace Tiled;
using namespace Tiled::Internal;

AddRemoveTileset::AddRemoveTileset(MapDocument *mapDocument,
                                   int index,
                                   Tileset *tileset)
    : mMapDocument(mapDocument)
    , mTileset(tileset)
    , mIndex(index)
{
    // Make sure the tileset manager keeps this tileset around
    TilesetManager::instance()->addReference(mTileset);
}

AddRemoveTileset::~AddRemoveTileset()
{
    TilesetManager::instance()->removeReference(mTileset);
}

void AddRemoveTileset::removeTileset()
{
    mMapDocument->removeTilesetAt(mIndex);
}

void AddRemoveTileset::addTileset()
{
    mMapDocument->insertTileset(mIndex, mTileset);
}


AddTileset::AddTileset(MapDocument *mapDocument, Tileset *tileset)
    : AddRemoveTileset(mapDocument,
                       mapDocument->map()->tilesets().size(),
                       tileset)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Tileset"));
}
