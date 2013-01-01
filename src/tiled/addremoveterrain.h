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

#ifndef ADDREMOVETERRAIN_H
#define ADDREMOVETERRAIN_H

#include <QUndoCommand>

namespace Tiled {

class Terrain;
class Tileset;

namespace Internal {

class MapDocument;

/**
 * Abstract base class for AddTerrain and RemoveTerrain.
 */
class AddRemoveTerrain : public QUndoCommand
{
public:
    AddRemoveTerrain(MapDocument *mapDocument,
                     Tileset *tileset,
                     int index,
                     Terrain *terrain);
    ~AddRemoveTerrain();

protected:
    void addTerrain();
    void removeTerrain();

private:
    MapDocument *mMapDocument;
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
    AddTerrain(MapDocument *mapDocument, Terrain *terrain);

    void undo() { removeTerrain(); }
    void redo() { addTerrain(); }
};

/**
 * Removes a terrain from a map.
 */
class RemoveTerrain : public AddRemoveTerrain
{
public:
    RemoveTerrain(MapDocument *mapDocument, Terrain *terrain);

    void undo() { addTerrain(); }
    void redo() { removeTerrain(); }
};

} // namespace Internal
} // namespace Tiled

#endif // ADDREMOVETERRAIN_H
