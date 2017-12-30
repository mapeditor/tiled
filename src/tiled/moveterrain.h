/*
 * moveterrain.h
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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class Terrain;
class Tileset;

namespace Internal {

class TilesetDocument;

/**
 * Abstract base class for MoveTerrainUp and MoveTerrainDown.
 */
class MoveTerrain : public QUndoCommand
{
public:
    MoveTerrain(TilesetDocument *tilesetDocument,
                int index);
    ~MoveTerrain();

protected:
    void moveTerrainUp();
    void moveTerrainDown();

private:
    TilesetDocument *mTilesetDocument;
    Tileset *mTileset;
    int mIndex;
};


/**
 * Moves a terrain up in the list of terrains.
 */
class MoveTerrainUp : public MoveTerrain
{
public:
    MoveTerrainUp(TilesetDocument *tilesetDocument, Terrain *terrain);

    void undo() override { moveTerrainDown(); }
    void redo() override { moveTerrainUp(); }
};

/**
 * Moves a terrain down in the list of terrains.
 */
class MoveTerrainDown : public MoveTerrain
{
public:
    MoveTerrainDown(TilesetDocument *tilesetDocument, Terrain *terrain);

    void undo() override { moveTerrainUp(); }
    void redo() override { moveTerrainDown(); }
};

} // namespace Internal
} // namespace Tiled
