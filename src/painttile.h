/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PAINTTILE_H
#define PAINTTILE_H

#include <QUndoCommand>

namespace Tiled {

class Tile;
class TileLayer;

namespace Internal {

class MapDocument;

/**
 * A command that paints a single tile at a certain position.
 */
class PaintTile : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being edited
     * @param layer       the layer of the tile to edit
     * @param x           the x position of the tile to edit
     * @param y           the y position of the tile to edit
     * @param tile        the new tile for this position
     */
    PaintTile(MapDocument *mapDocument, int layer, int x, int y, Tile *tile);

    void undo();
    void redo();

private:
    void swapTile();

    MapDocument *mMapDocument;
    TileLayer *mLayer;
    int mX, mY;
    Tile *mTile;
};

} // namespace Internal
} // namespace Tiled

#endif // PAINTTILE_H
