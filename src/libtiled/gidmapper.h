/*
 * gidmapper.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
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

#ifndef TILED_GIDMAPPER_H
#define TILED_GIDMAPPER_H

#include "tilelayer.h"

#include <QMap>

namespace Tiled {

/**
 * A class that maps cells to global IDs (gids) and back.
 */
class TILEDSHARED_EXPORT GidMapper
{
public:
    /**
     * Default constructor. Use \l insert to initialize the gid mapper
     * incrementally.
     */
    GidMapper();

    /**
     * Constructor that initializes the gid mapper using the given \a tilesets.
     */
    GidMapper(const QList<Tileset *> &tilesets);

    /**
     * Insert the given \a tileset with \a firstGid as its first global ID.
     */
    void insert(uint firstGid, Tileset *tileset)
    { mFirstGidToTileset.insert(firstGid, tileset); }

    /**
     * Clears the gid mapper, so that it can be reused.
     */
    void clear() { mFirstGidToTileset.clear(); }

    /**
     * Returns true when no tilesets are known to this gid mapper.
     */
    bool isEmpty() const { return mFirstGidToTileset.isEmpty(); }

    /**
     * Returns the cell data matched by the given \a gid. The \a ok parameter
     * indicates whether an error occurred.
     */
    Cell gidToCell(uint gid, bool &ok) const;

    /**
     * Returns the global tile ID for the given \a cell. Returns 0 when the
     * cell is empty or when its tileset isn't known.
     */
    uint cellToGid(const Cell &cell) const;

    /**
     * This sets the original tileset width. In case the image size has
     * changed, the tile indexes will be adjusted automatically when using
     * gidToCell().
     */
    void setTilesetWidth(const Tileset *tileset, int width);

private:
    QMap<uint, Tileset*> mFirstGidToTileset;
    QMap<const Tileset*, int> mTilesetColumnCounts;
};

} // namespace Tiled

#endif // TILED_GIDMAPPER_H
