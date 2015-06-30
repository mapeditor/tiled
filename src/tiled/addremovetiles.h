/*
 * addremovetiles.h
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

#ifndef ADDREMOVETILES_H
#define ADDREMOVETILES_H

#include <QUndoCommand>

namespace Tiled {

class Tile;
class Tileset;

namespace Internal {

class MapDocument;

/**
 * Abstract base class for AddTiles and RemoveTiles.
 */
class AddRemoveTiles : public QUndoCommand
{
public:
    AddRemoveTiles(MapDocument *mapDocument,
                   Tileset *tileset,
                   int index,
                   int count,
                   const QList<Tile*> &tiles = QList<Tile*>());

    ~AddRemoveTiles();

protected:
    void addTiles();
    void removeTiles();

private:
    MapDocument *mMapDocument;
    Tileset *mTileset;
    int mIndex;
    int mCount;
    QList<Tile*> mTiles;
};

/**
 * Undo command that adds tiles to a tileset.
 */
class AddTiles : public AddRemoveTiles
{
public:
    AddTiles(MapDocument *mapDocument,
             Tileset *tileset,
             const QList<Tile*> &tiles);

    void undo()
    { removeTiles(); }

    void redo()
    { addTiles(); }
};

/**
 * Undo command that removes tiles from a tileset.
 */
class RemoveTiles : public AddRemoveTiles
{
public:
    RemoveTiles(MapDocument *mapDocument,
                Tileset *tileset,
                int index,
                int count);

    void undo()
    { addTiles(); }

    void redo()
    { removeTiles(); }
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_ADDREMOVETILES_H
