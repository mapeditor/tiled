/*
 * addremoveterrain.h
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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class Terrain;
class Tileset;

class TilesetDocument;

/**
 * Abstract base class for AddTerrain and RemoveTerrain.
 */
class AddRemoveTerrain : public QUndoCommand
{
public:
    AddRemoveTerrain(TilesetDocument *tilesetDocument,
                     int index,
                     Terrain *terrain);
    ~AddRemoveTerrain();

protected:
    void addTerrain();
    void removeTerrain();

private:
    TilesetDocument *mTilesetDocument;
    Tileset *mTileset;
    int mIndex;
    Terrain *mTerrain;
};


/**
 * Adds a terrain to a map.
 */
class AddTerrain : public AddRemoveTerrain
{
public:
    AddTerrain(TilesetDocument *tilesetDocument, Terrain *terrain);

    void undo() override { removeTerrain(); }
    void redo() override { addTerrain(); }
};

/**
 * Removes a terrain from a map.
 */
class RemoveTerrain : public AddRemoveTerrain
{
public:
    RemoveTerrain(TilesetDocument *tilesetDocument, Terrain *terrain);

    void undo() override { addTerrain(); }
    void redo() override { removeTerrain(); }
};

} // namespace Tiled
