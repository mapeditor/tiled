/*
 * addremoveterrain.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremoveterrain.h"

#include "mapdocument.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "tileset.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

AddRemoveTerrain::AddRemoveTerrain(MapDocument *mapDocument,
                                   Tileset *tileset,
                                   int index,
                                   Terrain *terrain)
    : mMapDocument(mapDocument)
    , mTileset(tileset)
    , mIndex(index)
    , mTerrain(terrain)
{
}

AddRemoveTerrain::~AddRemoveTerrain()
{
    delete mTerrain;
}

void AddRemoveTerrain::removeTerrain()
{
    Q_ASSERT(!mTerrain);
    mTerrain = mMapDocument->terrainModel()->takeTerrainAt(mTileset, mIndex);
}

void AddRemoveTerrain::addTerrain()
{
    Q_ASSERT(mTerrain);
    mMapDocument->terrainModel()->insertTerrain(mTileset, mIndex, mTerrain);
    mTerrain = 0;
}


AddTerrain::AddTerrain(MapDocument *mapDocument, Terrain *terrain)
    : AddRemoveTerrain(mapDocument,
                       terrain->tileset(),
                       terrain->tileset()->terrainCount(),
                       terrain)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Terrain"));
}


RemoveTerrain::RemoveTerrain(MapDocument *mapDocument, Terrain *terrain)
    : AddRemoveTerrain(mapDocument,
                       terrain->tileset(),
                       terrain->id(),
                       0)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Terrain"));
}
