/*
 * swaptiles.h
 * Copyright 2015, Alexander "theHacker" Münch <git@thehacker.biz>
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

#include "undocommands.h"

#include <QUndoCommand>

namespace Tiled {

class Tile;

class MapDocument;

/**
 * A command that swaps two tiles on the map.
 */
class SwapTiles : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being edited
     * @param tile1       the first tile
     * @param tile2       the second tile
     */
    SwapTiles(MapDocument *mapDocument,
              Tile *tile1,
              Tile *tile2);

    void undo() { swap(); }
    void redo() { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    Tile *mTile1;
    Tile *mTile2;
};

} // namespace Tiled
