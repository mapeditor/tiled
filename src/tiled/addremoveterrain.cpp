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

#include "terrain.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetterrainmodel.h"

#include <QCoreApplication>

using namespace Tiled;

AddRemoveTerrain::AddRemoveTerrain(TilesetDocument *tilesetDocument,
                                   int index,
                                   Terrain *terrain)
    : mTilesetDocument(tilesetDocument)
    , mTileset(tilesetDocument->tileset().data())
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
    mTerrain = mTilesetDocument->terrainModel()->takeTerrainAt(mIndex);
}

void AddRemoveTerrain::addTerrain()
{
    Q_ASSERT(mTerrain);
    mTilesetDocument->terrainModel()->insertTerrain(mIndex, mTerrain);
    mTerrain = nullptr;
}


AddTerrain::AddTerrain(TilesetDocument *tilesetDocument, Terrain *terrain)
    : AddRemoveTerrain(tilesetDocument,
                       terrain->tileset()->terrainCount(),
                       terrain)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Terrain"));
}


RemoveTerrain::RemoveTerrain(TilesetDocument *tilesetDocument, Terrain *terrain)
    : AddRemoveTerrain(tilesetDocument,
                       terrain->id(),
                       nullptr)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Terrain"));
}
