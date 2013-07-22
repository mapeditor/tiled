/*
 * renameterrain.cpp
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "renameterrain.h"

#include "mapdocument.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "tileset.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

RenameTerrain::RenameTerrain(MapDocument *mapDocument,
                             Tileset *tileset,
                             int terrainId,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Terrain Name"))
    , mTerrainModel(mapDocument->terrainModel())
    , mTileset(tileset)
    , mTerrainId(terrainId)
    , mOldName(tileset->terrain(terrainId)->name())
    , mNewName(newName)
{}

void RenameTerrain::undo()
{
    mTerrainModel->setTerrainName(mTileset, mTerrainId, mOldName);
}

void RenameTerrain::redo()
{
    mTerrainModel->setTerrainName(mTileset, mTerrainId, mNewName);
}

} // namespace Internal
} // namespace Tiled
