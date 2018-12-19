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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class Tile;

class TilesetDocument;

/**
 * Abstract base class for AddTiles and RemoveTiles.
 */
class AddRemoveTiles : public QUndoCommand
{
public:
    AddRemoveTiles(TilesetDocument *tilesetDocument,
                   const QList<Tile*> &tiles,
                   bool add);

    ~AddRemoveTiles();

protected:
    void addTiles();
    void removeTiles();

private:
    TilesetDocument *mTilesetDocument;
    QList<Tile*> mTiles;
    bool mTilesAdded;
};

/**
 * Undo command that adds tiles to a tileset.
 */
class AddTiles : public AddRemoveTiles
{
public:
    AddTiles(TilesetDocument *tilesetDocument,
             const QList<Tile*> &tiles);

    void undo() override
    { removeTiles(); }

    void redo() override
    { addTiles(); }
};

/**
 * Undo command that removes tiles from a tileset.
 */
class RemoveTiles : public AddRemoveTiles
{
public:
    RemoveTiles(TilesetDocument *tilesetDocument,
                const QList<Tile *> &tiles);

    void undo() override
    { addTiles(); }

    void redo() override
    { removeTiles(); }
};

} // namespace Tiled
