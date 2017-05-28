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
    void addTile(Tile *tile, unsigned wangId);

    /**
     * @brief getMatchingTile Returns a tile matching the given wangId.
     * 0s in the id are wild cards, and can be filled with any color.
     * If there are multiple possible options, one will be choosen at random.
     * @param wangId
     * @return
     */
    Tile *getMatchingTile(unsigned wangId) const;

    /**
     * @brief getAllTiles Returns a list of all tiles which match a wangId
     * 0s in the id are wild cards, and can be filled with any color.
     * @param wangId
     * @return
     */
    QList<Tile*> getAllTiles(unsigned wangId) const;


    /**
     * @brief wangIdOfTile returns the wangId of a given tileId
     * @param tileId
     */
    unsigned getWangIdOfTile(Tile *tile) const;

private:
    Tileset *mTileSet;
    QString mName;
    int mImageTileId;
    QMultiMap<unsigned, Tile*> mWangIdToTile;
    QMap<int, unsigned> mTileIdToWangId; //This could be stored in the tile object.
    int mEdgeColors;
    int mCornerColors;
};

/**
 * public functions for working with "wangIds"
 */

/**
 * @brief makeWangId Constructs a wang id from given edges/corner colors
 * @param a Top edge, or top right corner
 * @param b Right edge, or bottom right corner
 * @param c Bottom edge, or bottom left corner
 * @param d Left edge, or top left corner
 * @param edges Defining edges (Corners if false)
 */
unsigned makeWangId(int a, int b, int c, int d, bool edges)
{
    //Makes sure inputs aren't bigger than 15
    a = a & 0xf;
    b = b & 0xf;
    c = c & 0xf;
    d = d & 0xf;

    int id = ((((((d << 8) | c) << 8) | b) << 8) | a) << ((!edges) * 4);

    return id;
}

/**
 * @brief getColor returns the color of a desired edge/corner of a given wang id
 * @param wangId
 * @param index index 0-3 with zero being the top right, and 3 left top
 * @param edges requesting edge color (corners if false)
 * @return
 */
int getColorOfWangId(unsigned wangId, int index, bool edges)
{
    int shift = (index * 8) + ((!edges) * 4);

    int color = (wangId >> shift) & 0xf;

    return color;
}

/**
 * @brief rotateWangId returns the given wang id rotated 90 * rotations degrees ccw
 * @param wangId
 * @param rotations 1-3
 * @return
 */
unsigned rotateWangId(unsigned wangId, int rotations)
{
    if (rotations < 0)
        rotations = 4 + (rotations % 4);
    else
        rotations %= 4;

    unsigned rotated = wangId << rotations*8;
    rotated = rotated | (wangId >> ((4 - rotations) * 8));

    return rotated;
}

} // namespace Tiled
