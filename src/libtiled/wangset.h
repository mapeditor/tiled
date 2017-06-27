/*
 * wangset.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "tile.h"
#include "tileset.h"

#include <QHash>
#include <QMultiHash>
#include <QString>
#include <QList>

namespace Tiled {

class WangId
{
public:
    WangId(unsigned id) : mId(id) {}
    WangId() : mId(0) {}

    operator unsigned() const { return mId; }
    inline void setId(unsigned id) { mId = id; }

    /* These return the color of the edge/corner of the wangId.
     * 0 being the top right corner, or top edge
     * */
    int edgeColor(int index) const;
    int cornerColor(int index) const;

    /* Rotates the wang Id clockwise by (90 * rotations) degrees.
     * Meaning with one rotation, the top edge becomes the right edge,
     * and the top right corner, becomes the top bottom.
     * */
    void rotate(int rotations);

private:
    unsigned mId;
};

inline int WangId::edgeColor(int index) const
{
    int shift = (index * 8);

    int color = (mId >> shift) & 0xf;

    return color;
}

inline int WangId::cornerColor(int index) const
{
    int shift = (index * 8) + 4;

    int color = (mId >> shift) & 0xf;

    return color;
}

inline void WangId::rotate(int rotations)
{
    if (rotations < 0)
        rotations = 4 + (rotations % 4);
    else
        rotations %= 4;

    unsigned rotated = mId << rotations*8;
    rotated = rotated | (mId >> ((4 - rotations) * 8));

    mId = rotated;
}

/**
 * Represents a wang set.
 */
class TILEDSHARED_EXPORT WangSet : public Object
{
public:
    WangSet(Tileset *tileset,
            int edgeColors,
            int cornerColors,
            QString name,
            int imageTileId);

    Tileset *tileset() const { return mTileset; }
    void setTileset(Tileset *tileset) { mTileset = tileset; }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    int imageTileId() const { return mImageTileId; }
    void setImageTileId(int imageTileId) { mImageTileId = imageTileId; }

    Tile *imageTile() const { return mTileset->findTile(mImageTileId); }

    int edgeColors() const { return mEdgeColors; }
    int cornerColors() const { return mCornerColors; }
    void setEdgeColors(int n) { mEdgeColors = n; }
    void setCornerColors(int n) { mCornerColors = n; }

    /* Adds a tile to the wang set with a given wangId
     * */
    void addTile(Tile *tile, WangId wangId);

    /* Finds a tile whos WangId matches with the one provided,
     * where zeros in the id are treated as wild cards, and can be
     * any color.
     * */
    Tile *findMatchingTile(WangId wangId) const;

    /* Finds all the tiles which match the given wangId,
     * where zeros in the id are treated as wild cards, and can be
     * any color.
     * */
    QList<Tile*> findMatchingTiles(WangId wangId) const;


    /* Returns the wangId of a given Tile.
     * */
    WangId wangIdOfTile(const Tile *tile) const;

    /* Returns a clone of this wangset
     * */
    WangSet *clone(Tileset *tileset) const;

private:
    Tileset *mTileset;
    QString mName;
    int mImageTileId;
    int mEdgeColors;
    int mCornerColors;
    QMultiHash<WangId, Tile*> mWangIdToTile;
    QHash<int, WangId> mTileIdToWangId;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WangSet*)
