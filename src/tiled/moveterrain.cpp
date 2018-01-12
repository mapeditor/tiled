/*
 * moveterrain.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Justin Jacobs <jajdorkster@gmail.com>
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

#include "moveterrain.h"

#include "terrain.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetterrainmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

MoveTerrain::MoveTerrain(TilesetDocument *tilesetDocument, int index)
    : mTilesetDocument(tilesetDocument)
    , mTileset(tilesetDocument->tileset().data())
    , mIndex(index)
{
}

MoveTerrain::~MoveTerrain()
{
}

void MoveTerrain::moveTerrainUp()
{
    mTilesetDocument->terrainModel()->swapTerrains(mIndex, mIndex - 1);
    mIndex -= 1;
}

void MoveTerrain::moveTerrainDown()
{
    mTilesetDocument->terrainModel()->swapTerrains(mIndex, mIndex + 1);
    mIndex += 1;
}


MoveTerrainUp::MoveTerrainUp(TilesetDocument *tilesetDocument, Terrain *terrain)
    : MoveTerrain(tilesetDocument, terrain->id())
{
    setText(QCoreApplication::translate("Undo Commands", "Move Terrain Up"));
}

MoveTerrainDown::MoveTerrainDown(TilesetDocument *tilesetDocument, Terrain *terrain)
    : MoveTerrain(tilesetDocument, terrain->id())
{
    setText(QCoreApplication::translate("Undo Commands", "Move Terrain Down"));
}
