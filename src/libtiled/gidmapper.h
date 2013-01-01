/*
 * gidmapper.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    void insert(unsigned firstGid, Tileset *tileset)
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
    Cell gidToCell(unsigned gid, bool &ok) const;

    /**
     * Returns the global tile ID for the given \a cell. Returns 0 when the
     * cell is empty or when its tileset isn't known.
     */
    unsigned cellToGid(const Cell &cell) const;

    /**
     * This sets the original tileset width. In case the image size has
     * changed, the tile indexes will be adjusted automatically when using
     * gidToCell().
     */
    void setTilesetWidth(const Tileset *tileset, int width);

private:
    QMap<unsigned, Tileset*> mFirstGidToTileset;
    QMap<const Tileset*, int> mTilesetColumnCounts;
};

} // namespace Tiled

#endif // TILED_GIDMAPPER_H
