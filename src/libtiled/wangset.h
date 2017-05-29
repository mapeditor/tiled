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

#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QList>

namespace Tiled {

class WangId
{
public:
    WangId(unsigned id){ mId = id; }
    WangId() { mId = 0; }

    inline unsigned id() const { return mId; }
    inline void setId(unsigned id) { mId = id; }

    /**
     * @brief getColor returns the color of a desired edge/corner of a given wang id
     * @param index index 0-3 with zero being the top right, and 3 left top
     * @param edges requesting edge color (corners if false)
     * @return
     */
    int getColor(int index, bool edges) const;

    /**
     * @brief rotateWangId rotates the wang id 90 * rotations degrees ccw
     * @param rotations 1-3
     * @return
     */
    void rotate(int rotations);

    //used in maping
    bool operator <(const WangId w) const
    {
        return mId < w.id();
    }

private:
    unsigned mId;
};

inline int WangId::getColor(int index, bool edges) const
{
    int shift = (index * 8) + ((!edges) * 4);

    int color = (mId >> shift) & 0xf;

    return color;
}

void WangId::rotate(int rotations)
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

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    int imageTileId() const { return mImageTileId; }
    void setImageTileId(int imageTileId) { mImageTileId = imageTileId; }

    Tile *imageTile() const { return mTileSet->findTile(mImageTileId); }

    int edgeColors() const { return mEdgeColors; }
    int cornerColors() const { return mCornerColors; }
    void setEdgeColors(int n) { mEdgeColors = n; }
    void setCornerColors(int n) { mCornerColors = n; }

    /**
     * @brief addTile Adds a tile to the wang set
     * @param tile
     * @param wangId
     */
    void addTile(Tile *tile, WangId wangId);

    /**
     * @brief getMatchingTile Returns a tile matching the given wangId.
     * 0s in the id are wild cards, and can be filled with any color.
     * If there are multiple possible options, one will be choosen at random.
     * @param wangId
     * @return
     */
    Tile *getMatchingTile(WangId wangId) const;

    /**
     * @brief getAllTiles Returns a list of all tiles which match a wangId
     * 0s in the id are wild cards, and can be filled with any color.
     * @param wangId
     * @return
     */
    QList<Tile*> getAllTiles(WangId wangId) const;


    /**
     * @brief wangIdOfTile returns the wangId of a given tileId
     * @param tileId
     */
    WangId getWangIdOfTile(Tile *tile) const;

private:
    Tileset *mTileSet;
    QString mName;
    int mImageTileId;
    QMultiMap<WangId, Tile*> mWangIdToTile;
    QMap<int, WangId> mTileIdToWangId; //This could be stored in the tile object.
    int mEdgeColors;
    int mCornerColors;
};

} // namespace Tiled
