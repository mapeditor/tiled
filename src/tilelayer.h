/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef TILELAYER_H
#define TILELAYER_H

#include "layer.h"

#include <QString>
#include <QVector>

namespace Tiled {

class Tile;

/**
 * A tile layer.
 */
class TileLayer : public Layer
{
public:
    /**
     * Constructor.
     */
    TileLayer(const QString &name, int x, int y, int width, int height,
              Map *map = 0);

    /**
     * Returns the maximum tile height of this layer. Used by the layer
     * rendering code to determine the area that needs to be redrawn.
     */
    int maxTileHeight() const { return mMaxTileHeight; }

    /**
     * Returns the tile at the given coordinates. The coordinates have to
     * be within this layer.
     */
    Tile *tileAt(int x, int y) const;

    /**
     * Sets the tile for the given coordinates.
     */
    void setTile(int x, int y, Tile *tile);

private:
    int mMaxTileHeight;
    QVector<Tile*> mTiles;
};

} // namespace Tiled

#endif // TILELAYER_H
