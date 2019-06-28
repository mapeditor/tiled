/*
 * changeterrain.cpp
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

#include "changeterrain.h"

#include "terrain.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetterrainmodel.h"

#include <QCoreApplication>

namespace Tiled {

RenameTerrain::RenameTerrain(TilesetDocument *tilesetDocument,
                             int terrainId,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Terrain Name"))
    , mTerrainModel(tilesetDocument->terrainModel())
    , mTileset(tilesetDocument->tileset().data())
    , mTerrainId(terrainId)
    , mOldName(mTileset->terrain(terrainId)->name())
    , mNewName(newName)
{}

void RenameTerrain::undo()
{
    mTerrainModel->setTerrainName(mTerrainId, mOldName);
}

void RenameTerrain::redo()
{
    mTerrainModel->setTerrainName(mTerrainId, mNewName);
}


SetTerrainImage::SetTerrainImage(TilesetDocument *tilesetDocument, int terrainId, int tileId)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Terrain Image"))
    , mTerrainModel(tilesetDocument->terrainModel())
    , mTerrainId(terrainId)
    , mOldImageTileId(tilesetDocument->tileset()->terrain(terrainId)->imageTileId())
    , mNewImageTileId(tileId)
{}

void SetTerrainImage::undo()
{
    mTerrainModel->setTerrainImage(mTerrainId, mOldImageTileId);
}

void SetTerrainImage::redo()
{
    mTerrainModel->setTerrainImage(mTerrainId, mNewImageTileId);
}

} // namespace Tiled
