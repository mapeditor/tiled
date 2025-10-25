/*
 * tilepainter.h
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jksb@member.fsf.org>
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

#include "tilelayer.h"

#include <QRegion>

namespace Tiled {

class MapDocument;

/**
 * The tile painter is meant for painting cells of a tile layer. It makes sure
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
     * Returns the cell at the given coordinates. The coordinates are relative
     * to the map origin. Returns an empty cell if the coordinates lay outside
     * of the layer.
     */
    Cell cellAt(int x, int y) const;
    Cell cellAt(QPoint pos) const;

    /**
     * Sets the cells at the given coordinates to the cells in the given tile
     * layer. The coordinates \a x and \a y are relative to the map origin.
     *
     * Only cells that fall within this mask are set. The mask is applied in
     * map coordinates.
     */
    void setCells(int x, int y, const TileLayer *tileLayer, const QRegion &mask);

    /**
     * Draws the cells in the given tile layer at the given coordinates. The
     * coordinates \a x and \a y are relative to the map origin.
     *
     * Empty cells are skipped.
     */
    void drawCells(int x, int y, TileLayer *tileLayer);

    /**
     * Draws the stamp within the given \a drawRegion region, repeating the
     * stamp as needed.
     */
    void drawStamp(const TileLayer *stamp, const QRegion &drawRegion);

    /**
     * Erases the cells in the given region.
     */
    void erase(const QRegion &region);

    /**
     * Computes the paintable fill region starting at \a fillOrigin containing
     * all connected cells for which the given \a condition returns true.
     */
    QRegion computePaintableFillRegion(QPoint fillOrigin,
                                       std::function<bool(const Cell &)> condition) const;

    /**
     * Computes a fill region starting at \a fillOrigin containing all
     * connected cells for which the given \a condition returns true. Does not
     * take into account the current selection.
     */
    QRegion computeFillRegion(QPoint fillOrigin, std::function<bool(const Cell &)> condition) const;

private:
    QRegion paintableRegion(const QRegion &region) const;
    QRegion paintableRegion(int x, int y, int width, int height) const
    { return paintableRegion(QRect(x, y, width, height)); }

    MapDocument *mMapDocument;
    TileLayer *mTileLayer;
};

inline Cell TilePainter::cellAt(QPoint pos) const
{
    return cellAt(pos.x(), pos.y());
}

} // namespace Tiled
