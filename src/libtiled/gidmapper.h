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

#pragma once

#include "map.h"
#include "tilelayer.h"

#include <QMap>

namespace Tiled {

/**
 * A class that maps cells to global IDs (gids) and back.
 */
class TILEDSHARED_EXPORT GidMapper
{
public:
    GidMapper();
    GidMapper(const QVector<SharedTileset> &tilesets);

    void insert(unsigned firstGid, const SharedTileset &tileset);
    void clear();
    bool isEmpty() const;

    Cell gidToCell(unsigned gid, bool &ok) const;
    unsigned cellToGid(const Cell &cell) const;

    QByteArray encodeLayerData(const TileLayer &tileLayer,
                               Map::LayerDataFormat format,
                               QRect bounds = QRect()) const;

    enum DecodeError {
        NoError = 0,
        CorruptLayerData,
        TileButNoTilesets,
        InvalidTile
    };

    DecodeError decodeLayerData(TileLayer &tileLayer,
                                const QByteArray &layerData,
                                Map::LayerDataFormat format,
                                QRect bounds = QRect()) const;

    unsigned invalidTile() const;

private:
    QMap<unsigned, SharedTileset> mFirstGidToTileset;

    mutable unsigned mInvalidTile;
};


/**
 * Insert the given \a tileset with \a firstGid as its first global ID.
 */
inline void GidMapper::insert(unsigned firstGid, const SharedTileset &tileset)
{
    mFirstGidToTileset.insert(firstGid, tileset);
}

/**
 * Clears the gid mapper, so that it can be reused.
 */
inline void GidMapper::clear()
{
    mFirstGidToTileset.clear();
}

/**
 * Returns true when no tilesets are known to this gid mapper.
 */
inline bool GidMapper::isEmpty() const
{
    return mFirstGidToTileset.isEmpty();
}

/**
 * Returns the GID of the invalid tile in case decodeLayerData() returns
 * the InvalidTile error.
 */
inline unsigned GidMapper::invalidTile() const
{
    return mInvalidTile;
}

} // namespace Tiled
