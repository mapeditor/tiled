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

#ifndef TILEPAINTER_H
#define TILEPAINTER_H

#include <QRegion>

namespace Tiled {

class Tile;
class TileLayer;

namespace Internal {

class MapDocument;

/**
 * The tile painter is meant for painting tiles on a tile layer. It makes sure
 * that each paint operation sends out the proper events, so that views can
 * redraw the changed parts.
 *
 * This class also does bounds checking and when there is a tile selection, it
 * will only draw within this selection.
 */
class TilePainter
{
public:
    /**
     * Constructs a tile painter.
     *
     * @param mapDocument the map document to send change notifications to
     * @param tileLayer   the tile layer to edit
     */
    TilePainter(MapDocument *mapDocument, TileLayer *tileLayer);

    /**
     * Returns the tile at the given coordinates. The coordinates are relative
     * to the map origin. Returns 0 if the coordinates lay outside of the
     * layer.
     */
    Tile *tileAt(int x, int y);

    /**
     * Sets the tile for the given coordinates. The coordinates are relative to
     * the map origin.
     */
    void setTile(int x, int y, Tile *tile);

    /**
     * Sets the tiles at the given coordinates to the tiles in the given tile
     * layer. The coordinates \a x and \a y are relative to the map origin.
     */
    void setTiles(int x, int y, TileLayer *tile);

    /**
     * Draws the tiles in the given tile layer at the given coordinates. The
     * coordinates \a x and \a y are relative to the map origin.
     *
     * Empty tiles are skipped.
     */
    void drawTiles(int x, int y, TileLayer *tiles);

    /**
     * Erases the tiles in the given region.
     */
    void erase(const QRegion &region);

    // TODO: Add more operations (fill, copy)

private:
    QRegion paintableRegion(const QRegion &region) const;
    QRegion paintableRegion(int x, int y, int width, int height) const
    { return paintableRegion(QRect(x, y, width, height)); }

    MapDocument *mMapDocument;
    TileLayer *mTileLayer;
};

} // namespace Tiled
} // namespace Internal

#endif // TILEPAINTER_H
