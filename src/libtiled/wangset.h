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
    enum Index {
        Top = 0,
        TopRight = 1,
        Right = 2,
        BottomRight = 3,
        Bottom = 4,
        BottomLeft = 5,
        Left = 6,
        TopLeft = 7,
        NumIndexes,
    };

    WangId() : mId(0) {}
    WangId(unsigned id) : mId(id) {}

    operator unsigned() const { return mId; }
    inline void setId(unsigned id) { mId = id; }

    int edgeColor(int index) const;
    int cornerColor(int index) const;

    int indexColor(int index) const;

    void setEdgeColor(int index, unsigned value);
    void setCornerColor(int index, unsigned value);
    void setGridColor(int x, int y, unsigned value);

    void setIndexColor(int index, unsigned value);

    void updateToAdjacent(WangId adjacent, int position);

    bool hasWildCards() const;

    WangIdVariations variations(int colorCount) const;

    void rotate(int rotations);
    void flipHorizontally();
    void flipVertically();

    static Index indexByGrid(int x, int y);
    static Index oppositeIndex(int index);

private:
    unsigned mId;
};

inline WangId::Index WangId::oppositeIndex(int index)
{
    return static_cast<Index>((index + 4) % NumIndexes);
}


class TILEDSHARED_EXPORT WangIdVariations
{
public:
    class iterator
    {
    public:
        iterator(int colorCount, WangId wangId = 0);
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
        const int mColorCount;
    };

    WangIdVariations(int colorCount, WangId wangId = 0)
        : mWangId(wangId)
        , mColorCount(colorCount)
    {}

    iterator begin() const { return iterator(mColorCount, mWangId); }
    iterator end() const;

private:
    WangId mWangId;
    int mColorCount;
};

/**
 * Class for holding info about rotation and flipping.
 */
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
    void translate(const int map[]);

    Tile *mTile;
    WangId mWangId;
    bool mFlippedHorizontally;
    bool mFlippedVertically;
    bool mFlippedAntiDiagonally;
};

class TILEDSHARED_EXPORT WangColor : public Object
{
public:
    WangColor();
    WangColor(int colorIndex,
              const QString &name,
              const QColor &color,
              int imageId = -1,
              qreal probability = 1);

    int colorIndex() const { return mColorIndex; }
    QString name() const { return mName; }
    QColor color() const { return mColor; }
    int imageId() const { return mImageId; }
    qreal probability() const { return mProbability; }

    void setName(const QString &name) { mName = name; }
    void setColor(const QColor &color) { mColor = color; }
    void setImageId(int imageId) { mImageId = imageId; }
    void setProbability(qreal probability) { mProbability = probability; }

    WangSet *wangSet() const { return mWangSet; }

private:
    friend class WangSet;

    void setColorIndex(int colorIndex) { mColorIndex = colorIndex; }

    WangSet *mWangSet = nullptr;
    int mColorIndex;
    QString mName;
    QColor mColor;
    int mImageId;
    qreal mProbability;
};

/**
 * Represents a Wang set.
 */
class TILEDSHARED_EXPORT WangSet : public Object
{
public:
    WangSet(Tileset *tileset,
            const QString &name,
            int imageTileId);

    Tileset *tileset() const;
    void setTileset(Tileset *tileset);

    QString name() const;
    void setName(const QString &name);

    int imageTileId() const;
    void setImageTileId(int imageTileId);
    Tile *imageTile() const;

    int colorCount() const;
    void setColorCount(int n);

    void insertWangColor(const QSharedPointer<WangColor> &wangColor);
    void addWangColor(const QSharedPointer<WangColor> &wangColor);
    void removeWangColorAt(int color);

    const QSharedPointer<WangColor> &colorAt(int index) const;
    const QVector<QSharedPointer<WangColor>> &colors() const { return mColors; }

    QList<Tile *> tilesChangedOnSetColorCount(int newColorCount) const;
    QList<Tile *> tilesChangedOnRemoveColor(int color) const;

    void addTile(Tile *tile, WangId wangId);
    void addCell(const Cell &cell, WangId wangId);
    void addWangTile(const WangTile &wangTile);

    QList<WangTile> findMatchingWangTiles(WangId wangId) const;

    const QMultiHash<WangId, WangTile> &wangTilesByWangId() const { return mWangIdToWangTile; }

    QList<WangTile> sortedWangTiles() const;

    WangId wangIdFromSurrounding(const WangId surroundingWangIds[]) const;
    WangId wangIdFromSurrounding(const Cell surroundingCells[]) const;

    QList<Tile *> tilesWithWangId() const;

    WangId wangIdOfTile(const Tile *tile) const;
    WangId wangIdOfCell(const Cell &cell) const;

    qreal wangTileProbability(const WangTile &wangTile) const;

    bool wangIdIsValid(WangId wangId) const;

    static bool wangIdIsValid(WangId wangId, int colorCount);

    bool wangIdIsUsed(WangId wangId) const;

    bool wildWangIdIsUsed(WangId wangId) const;

    bool isEmpty() const;
    bool isComplete() const;
    unsigned completeSetSize() const;

    WangId templateWangIdAt(unsigned n) const;

    /* Returns a clone of this wangset
     */
    WangSet *clone(Tileset *tileset) const;

private:
    void removeWangTile(const WangTile &wangTile);

    void removeWangColor(int color);

    Tileset *mTileset;
    QString mName;
    int mImageTileId;

    // How many unique, full wangIds are active in this set.
    // Where full means the id has no wildcards
    unsigned mUniqueFullWangIdCount;

    QVector<QSharedPointer<WangColor>> mColors;
    QMultiHash<WangId, WangTile> mWangIdToWangTile;

    // Tile info being the tileId, with the last three bits (32, 31, 30)
    // being info on flip (horizontal, vertical, and antidiagonal)
    QHash<unsigned, WangId> mTileInfoToWangId;
};


inline Tileset *WangSet::tileset() const
{
    return mTileset;
}

inline void WangSet::setTileset(Tileset *tileset)
{
    mTileset = tileset;
}

inline QString WangSet::name() const
{
    return mName;
}

inline void WangSet::setName(const QString &name)
{
    mName = name;
}

inline int WangSet::imageTileId() const
{
    return mImageTileId;
}

inline void WangSet::setImageTileId(int imageTileId)
{
    mImageTileId = imageTileId;
}

inline Tile *WangSet::imageTile() const
{
    return mTileset->findTile(mImageTileId);
}

inline int WangSet::colorCount() const
{
    return qMax(1, mColors.size());
}

inline const QSharedPointer<WangColor> &WangSet::colorAt(int index) const
{
    Q_ASSERT(index > 0 && index <= colorCount());

    return mColors.at(index - 1);
}

inline void WangSet::addTile(Tile *tile, WangId wangId)
{
    addWangTile(WangTile(tile, wangId));
}

inline void WangSet::addCell(const Cell &cell, WangId wangId)
{
    addWangTile(WangTile(cell, wangId));
}

inline bool WangSet::isEmpty() const
{
    return mWangIdToWangTile.isEmpty();
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WangSet*)
Q_DECLARE_METATYPE(Tiled::WangId)
