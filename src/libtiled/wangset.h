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
#include "tilelayer.h"

#include <QHash>
#include <QMultiHash>
#include <QString>
#include <QList>

namespace Tiled {

class TILEDSHARED_EXPORT WangId
{
public:

    WangId() : mId(0) {}
    WangId(unsigned id) : mId(id) {}

    operator unsigned() const { return mId; }
    inline void setId(unsigned id) { mId = id; }

    /* These return the color of the edge/corner of the wangId.
     * 0 being the top right corner, or top edge
     * */
    int edgeColor(int index) const;
    int cornerColor(int index) const;

    void setEdgeColor(int index, unsigned value);
    void setCornerColor(int index, unsigned value);

    /* Rotates the wang Id clockwise by (90 * rotations) degrees.
     * Meaning with one rotation, the top edge becomes the right edge,
     * and the top right corner, becomes the top bottom.
     * */
    void rotate(int rotations);

    /* Flips the wang Id horizontally
     * */
    void flipHorizontally();

    /* Flips the wang Id vertically
     * */
    void flipVertically();

private:
    unsigned mId;
};

//Class for holding info about rotation and flipping.
class TILEDSHARED_EXPORT WangTile
{
public:
    WangTile():
        mTile(nullptr),
        mWangId(0),
        mFlippedHorizontally(false),
        mFlippedVertically(false),
        mFlippedAntiDiagonally(false)
    {}

    WangTile(Tile *tile,
             WangId wangId):
        mTile(tile),
        mWangId(wangId),
        mFlippedHorizontally(false),
        mFlippedVertically(false),
        mFlippedAntiDiagonally(false)
    {}

    WangTile(const Cell &cell, WangId wangId):
        mTile(cell.tile()),
        mWangId(wangId),
        mFlippedHorizontally(cell.flippedHorizontally()),
        mFlippedVertically(cell.flippedVertically()),
        mFlippedAntiDiagonally(cell.flippedAntiDiagonally())
    {}

    Tile *tile() const { return mTile; }

    WangId wangId() const { return mWangId; }

    bool flippedHorizontally() const { return mFlippedHorizontally; }
    bool flippedVertically() const { return mFlippedVertically; }
    bool flippedAntiDiagonally() const { return mFlippedAntiDiagonally; }

    void setFlippedHorizontally(bool b) { mFlippedHorizontally = b; }
    void setFlippedVertically(bool b) { mFlippedVertically = b; }
    void setFlippedAntiDiagonally(bool b) { mFlippedAntiDiagonally = b; }

    void rotateRight();
    void rotateLeft();
    void flipHorizontally();
    void flipVertically();

    Cell makeCell() const;

private:
    //performs a translation (either flipping or rotating) based on a one to one
    //map of size 8 (from 0 - 7)
    void translate(int map[]);

    Tile *mTile;
    WangId mWangId;
    bool mFlippedHorizontally;
    bool mFlippedVertically;
    bool mFlippedAntiDiagonally;
};

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

    void addCell(const Cell &cell, WangId wangId);

    void addWangTile(const WangTile &wangTile);

    /* Finds a tile whos WangId matches with the one provided,
     * where zeros in the id are treated as wild cards, and can be
     * any color.
     * */
    WangTile findMatchingWangTile(WangId wangId) const;

    /* Finds all the tiles which match the given wangId,
     * where zeros in the id are treated as wild cards, and can be
     * any color.
     * */
    QList<WangTile> findMatchingWangTiles(WangId wangId) const;

    /* Finds a wangTile mathcing the provided wangId, and creates,
     * and returns a Cell matching the WangTile.
     * */
    Cell findMatchingCell(WangId wangId) const;

    QList<WangTile> wangTiles() const { return mWangIdToWangTile.values(); }

    /* Returns a wangId matching that of the provided surrounding wangIds.
     * This is based off a provided array, {a, b, c, d, e, f, g, h},
     * which corrisponds to  h|a|b
     *                       g|X|c
     *                       f|e|d
     * */
    WangId wangIdFromSurrounding(WangId surroundingWangIds[]) const;

    /* Returns a wangId matching that of the provided surrounding tiles.
     * This is based off a provided array, {a, b, c, d, e, f, g, h},
     * which corrisponds to  h|a|b
     *                       g|X|c
     *                       f|e|d
     * */
    WangId wangIdFromSurrounding(const Cell surroundingCells[]) const;

    /* Returns the wangId of a given Tile.
     * */
    WangId wangIdOfTile(const Tile *tile) const;

    WangId wangIdOfCell(const Cell &cell) const;

    /* Returns whether or not the given wangId is valid in the contex of the current wangSet
     * */
    bool wangIdIsValid(WangId wangId) const;

    /* Returns a clone of this wangset
     * */
    WangSet *clone(Tileset *tileset) const;

private:
    Tileset *mTileset;
    QString mName;
    int mImageTileId;
    int mEdgeColors;
    int mCornerColors;
    QMultiHash<WangId, WangTile> mWangIdToWangTile;

    //Tile info being the tileId, with the last three bits (32, 31, 30)
    //being info on flip (horizontal, vertical, and antidiagonal)
    QHash<unsigned, WangId> mTileInfoToWangId;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WangSet*)
