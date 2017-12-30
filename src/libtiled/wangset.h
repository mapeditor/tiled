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

class WangIdVariations;

class TILEDSHARED_EXPORT WangId
{
public:
    enum Corner {
        TopRight = 0,
        BottomRight = 1,
        BottomLeft = 2,
        TopLeft = 3
    };

    enum Edge {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

    WangId() : mId(0) {}
    WangId(unsigned id) : mId(id) {}

    operator unsigned() const { return mId; }
    inline void setId(unsigned id) { mId = id; }

    /* These return the color of the edge/corner of the wangId.
     * 0 being the top right corner, or top edge
     * */
    int edgeColor(int index) const;
    int cornerColor(int index) const;

    /* Returns the color of a certain index 0 - 7.
     * 7|0|1
     * 6|X|2
     * 5|4|3
     * */
    int indexColor(int index) const;

    void setEdgeColor(int index, unsigned value);
    void setCornerColor(int index, unsigned value);

    /* Sets the color of a certain index 0 - 7.
     * 7|0|1
     * 6|X|2
     * 5|4|3
     * */
    void setIndexColor(int index, unsigned value);

    /* Matches this wangId's edges/corners with an adjacent one.
     * Where position is 0-7 with 0 being top, and 7 being top left:
     * 7|0|1
     * 6|X|2
     * 5|4|3
     * */
    void updateToAdjacent(WangId adjacent, int position);

    /* Returns true if one or more edges are zero
     * */
    bool hasEdgeWildCards() const;
    /* Returns true if one or more corners are zero
     * */
    bool hasCornerWildCards() const;

    /* Gives a list of wangId variations of this one
     * where every 0 is a wildCard (can be 0 - edgeColors or cornerColors)
     * */
    WangIdVariations variations(int edgeColors, int cornerColors) const;

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

class TILEDSHARED_EXPORT WangIdVariations
{
public:
    class iterator
    {
    public:
        iterator(int edgeColors, int cornerColors, WangId wangId = 0);
        iterator &operator++();
        iterator operator++(int)
        { iterator vI = *this; ++(*this); return vI; }
        bool operator==(iterator other) { return mCurrent == other.mCurrent; }
        bool operator!=(iterator other) { return mCurrent != other.mCurrent; }
        const WangId operator*() const { return mCurrent; }
        const WangId operator->() const { return mCurrent; }

    private:
        WangId mCurrent;
        WangId mMax;
        QList<int> mZeroSpots;
        int mEdgeColors;
        int mCornerColors;
    };

    WangIdVariations(int edgeColors, int cornerColors, WangId wangId = 0)
        : mWangId(wangId)
        , mEdgeColors(edgeColors)
        , mCornerColors(cornerColors) {}

    iterator begin() const { return iterator(mEdgeColors, mCornerColors, mWangId); }
    iterator end() const;

private:
    WangId mWangId;
    int mEdgeColors;
    int mCornerColors;
};

//Class for holding info about rotation and flipping
class TILEDSHARED_EXPORT WangTile
{
public:
    WangTile() : WangTile(nullptr, 0)
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
    void setWangId(WangId wangId) { mWangId = wangId; }

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

    bool operator== (const WangTile &other) const
    { return mTile == other.mTile
                && mWangId == other.mWangId
                && mFlippedHorizontally == other.mFlippedHorizontally
                && mFlippedVertically == other.mFlippedVertically
                && mFlippedAntiDiagonally == other.mFlippedAntiDiagonally; }

    bool operator< (const WangTile &other) const
    { return mTile->id() < other.mTile->id(); }

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

class WangColor : public Object
{
public:
    WangColor();
    WangColor(int colorIndex,
              bool isEdge,
              const QString &name,
              const QColor &color,
              int imageId,
              float probability = 1);

    int colorIndex() const { return mColorIndex; }
    bool isEdge() const { return mIsEdge; }
    QString name() const { return mName; }
    QColor color() const { return mColor; }
    int imageId() const { return mImageId; }
    float probability() const { return mProbability; }

    void setColorIndex(int colorIndex) { mColorIndex = colorIndex; }
    void setIsEdge(bool isEdge) { mIsEdge = isEdge; }
    void setName(const QString &name) { mName = name; }
    void setColor(const QColor &color) { mColor = color; }
    void setImageId(int imageId) { mImageId = imageId; }
    void setProbability(float probability) { mProbability = probability; }

private:
    int mColorIndex;
    bool mIsEdge;
    QString mName;
    QColor mColor;
    int mImageId;
    float mProbability;
};

/**
 * Represents a wang set.
 */
class TILEDSHARED_EXPORT WangSet : public Object
{
public:
    WangSet(Tileset *tileset,
            QString name,
            int imageTileId);

    Tileset *tileset() const { return mTileset; }
    void setTileset(Tileset *tileset) { mTileset = tileset; }

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    int imageTileId() const { return mImageTileId; }
    void setImageTileId(int imageTileId) { mImageTileId = imageTileId; }

    Tile *imageTile() const { return mTileset->findTile(mImageTileId); }

    int edgeColorCount() const;
    int cornerColorCount() const;

    /* Sets the edge/corner color count
     * This can make wangIds already in the set invalid, so should only be used from
     * ChangeWangSet(Edges/Corners)
     * */
    void setEdgeColorCount(int n);
    void setCornerColorCount(int n);

    /* Inserts a given wangColor into the wangSet.
     * If the color is greater than current count, it must only be one greater.
     * For use in an undo command
     * Does not adjust currently assigned tiles.
     * */
    void insertWangColor(QSharedPointer<WangColor> wangColor);

    // Adds a wangcolor to the set. The wangcolors color index may
    // be changed
    void addWangColor(QSharedPointer<WangColor> wangColor);

    /* Removes a given color.
     * This can make wangIds invalid, so should only be used from
     * changewangsetdata.h
     * */
    void removeWangColorAt(int color, bool isEdge);

    QSharedPointer<WangColor> edgeColorAt(int index) const;
    QSharedPointer<WangColor> cornerColorAt(int index) const;

    QList<Tile *> tilesChangedOnSetEdgeColors(int newEdgeColors) const;
    QList<Tile *> tilesChangedOnSetCornerColors(int newCornerColors) const;
    QList<Tile *> tilesChangedOnRemoveColor(int color, bool isEdge) const;

    /* Adds a wangtile to the wang set with a given wangId
     * If the given WangTile is already in the set with a
     * different wangId, then that reference is removed, and
     * replaced with the new wangId. If the wangId provided is zero
     * then the wangTile is removed if already in the set.
     * Updates the UniqueFullWangIdCount
     * */
    void addTile(Tile *tile, WangId wangId);
    void addCell(const Cell &cell, WangId wangId);
    void addWangTile(const WangTile &wangTile);

    /* Finds all the tiles which match the given wangId,
     * where zeros in the id are treated as wild cards, and can be
     * any color.
     * */
    QList<WangTile> findMatchingWangTiles(WangId wangId) const;

    /* Returns a sorted list of the wangTiles in this set.
     * Sorted by tileId.
     * */
    QList<WangTile> wangTiles() const;

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

    /* Returns a list of all the tiles with a wangId.
     * */
    QList<Tile *> tilesWithWangId() const;

    /* Returns the wangId of a given Tile.
     * */
    WangId wangIdOfTile(const Tile *tile) const;

    WangId wangIdOfCell(const Cell &cell) const;

    /* The probability of a given wang tile of being selected.
     * */
    float wangTileProbability(const WangTile &wangTile) const;

    /* Returns whether or not the given wangId is valid in the contex of the current wangSet
     * */
    bool wangIdIsValid(WangId wangId) const;

    static bool wangIdIsValid(WangId wangId, int edgeCount, int cornerCount);

    /* Returns whether the given wangId is assigned to a WangTile.
     * */
    bool wangIdIsUsed(WangId wangId) const;

    /* Returns true if the given wangId is assigned to a tile,
     * or any variations of the 0 spots are.
     * */
    bool wildWangIdIsUsed(WangId wangId) const;

    bool isEmpty() const { return mWangIdToWangTile.isEmpty(); }

    // Is every template wangTile filled
    bool isComplete() const;

    // How many tiles would be in a complete tileset
    unsigned completeSetSize() const;

    // How many unique, full wangIds are active in this set.
    // Where full means the id has no wildcards
    unsigned uniqueFullWangIdCount() const { return mUniqueFullWangIdCount; }

    /* Returns the nth wangId starting at 0x11111111
     * and, when C is the number of corners,
     * and E is the number of edges,
     * ending at 0xCECECECE
     *
     * Note this does NOT include wildcards (no zeros)
     * */
    WangId templateWangIdAt(unsigned n) const;

    /* Returns a clone of this wangset
     * */
    WangSet *clone(Tileset *tileset) const;

private:
    void removeWangTile(const WangTile &wangTile);

    void insertEdgeWangColor(QSharedPointer<WangColor> wangColor);
    void insertCornerWangColor(QSharedPointer<WangColor> wangColor);

    void removeEdgeWangColor(int color);
    void removeCornerWangColor(int color);

    Tileset *mTileset;
    QString mName;
    int mImageTileId;
    unsigned mUniqueFullWangIdCount;
    QVector<QSharedPointer<WangColor>> mEdgeColors;
    QVector<QSharedPointer<WangColor>> mCornerColors;
    QMultiHash<WangId, WangTile> mWangIdToWangTile;

    //Tile info being the tileId, with the last three bits (32, 31, 30)
    //being info on flip (horizontal, vertical, and antidiagonal)
    QHash<unsigned, WangId> mTileInfoToWangId;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WangSet*)
Q_DECLARE_METATYPE(Tiled::WangId)
