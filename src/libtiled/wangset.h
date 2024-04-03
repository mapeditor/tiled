/*
 * wangset.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "tile.h"
#include "tileset.h"
#include "tilelayer.h"

#include <QHash>
#include <QMultiHash>
#include <QString>
#include <QList>

#include "qtcompat_p.h"

namespace Tiled {

class TILEDSHARED_EXPORT WangId
{
public:
    constexpr static unsigned BITS_PER_INDEX = 8;
    constexpr static quint64 INDEX_MASK = 0xFF;
    constexpr static quint64 FULL_MASK = Q_UINT64_C(0xFFFFFFFFFFFFFFFF);
    constexpr static int MAX_COLOR_COUNT = (1 << BITS_PER_INDEX) - 2;

    enum Index {
        Top         = 0,
        TopRight    = 1,
        Right       = 2,
        BottomRight = 3,
        Bottom      = 4,
        BottomLeft  = 5,
        Left        = 6,
        TopLeft     = 7,

        NumCorners  = 4,
        NumEdges    = 4,
        NumIndexes  = 8,
    };

    enum Masks : quint64 {
        MaskTop         = INDEX_MASK << (BITS_PER_INDEX * Top),
        MaskTopRight    = INDEX_MASK << (BITS_PER_INDEX * TopRight),
        MaskRight       = INDEX_MASK << (BITS_PER_INDEX * Right),
        MaskBottomRight = INDEX_MASK << (BITS_PER_INDEX * BottomRight),
        MaskBottom      = INDEX_MASK << (BITS_PER_INDEX * Bottom),
        MaskBottomLeft  = INDEX_MASK << (BITS_PER_INDEX * BottomLeft),
        MaskLeft        = INDEX_MASK << (BITS_PER_INDEX * Left),
        MaskTopLeft     = INDEX_MASK << (BITS_PER_INDEX * TopLeft),

        MaskTopSide     = MaskTopLeft | MaskTop | MaskTopRight,
        MaskRightSide   = MaskTopRight | MaskRight | MaskBottomRight,
        MaskBottomSide  = MaskBottomLeft | MaskBottom | MaskBottomRight,
        MaskLeftSide    = MaskTopLeft | MaskLeft | MaskBottomLeft,

        MaskEdges       = MaskTop | MaskRight | MaskBottom | MaskLeft,
        MaskCorners     = MaskTopRight | MaskBottomRight | MaskBottomLeft | MaskTopLeft,
    };

    constexpr WangId(quint64 id = 0) : mId(id) {}

    constexpr operator quint64() const { return mId; }
    inline void setId(quint64 id) { mId = id; }

    bool isEmpty() const { return mId == 0; }

    int edgeColor(int index) const;
    int cornerColor(int index) const;

    int indexColor(int index) const;

    void setEdgeColor(int index, unsigned value);
    void setCornerColor(int index, unsigned value);
    void setGridColor(int x, int y, unsigned value);

    void setIndexColor(int index, unsigned value);

    void updateToAdjacent(WangId adjacent, int position);
    void mergeWith(WangId wangId, WangId mask);

    bool hasWildCards() const;
    bool hasCornerWildCards() const;
    bool hasEdgeWildCards() const;
    WangId mask() const;
    WangId mask(int value) const;

    bool hasCornerWithColor(int value) const;
    bool hasEdgeWithColor(int value) const;

    void rotate(int rotations);
    WangId rotated(int rotations) const;
    void flipHorizontally();
    void flipVertically();
    WangId flippedHorizontally() const;
    WangId flippedVertically() const;

    bool operator==(WangId other) const { return mId == other.mId; }
    bool operator!=(WangId other) const { return mId != other.mId; }
    WangId operator& (quint64 mask) const { return mId & mask; }
    WangId operator&=(quint64 mask) { return mId &= mask; }

    static Index indexByGrid(int x, int y);
    static Index oppositeIndex(int index);
    static Index nextIndex(int index);
    static Index previousIndex(int index);
    static bool isCorner(int index);

    static WangId fromUint(unsigned id);
    unsigned toUint() const;

    static WangId fromString(QStringRef string, bool *ok = nullptr);
    QString toString() const;

private:
    quint64 mId;
};

inline void WangId::mergeWith(WangId wangId, WangId mask)
{
    *this = (*this & ~mask) | (wangId & mask);
}

inline WangId::Index WangId::oppositeIndex(int index)
{
    return static_cast<Index>((index + 4) % NumIndexes);
}

inline WangId::Index WangId::nextIndex(int index)
{
    return static_cast<Index>((index + 1) % NumIndexes);
}

inline WangId::Index WangId::previousIndex(int index)
{
    return static_cast<Index>((index + NumIndexes - 1) % NumIndexes);
}

inline bool WangId::isCorner(int index)
{
    return index & 1;
}

TILEDSHARED_EXPORT QDebug operator<<(QDebug debug, WangId wangId);


/**
 * Class for holding a tile and its WangId.
 */
class TILEDSHARED_EXPORT WangTile
{
public:
    WangTile(int tileId, WangId wangId)
        : mTileId(tileId)
        , mWangId(wangId)
    {}

    int tileId() const { return mTileId; }
    WangId wangId() const { return mWangId; }

    bool operator< (const WangTile &other) const
    { return mTileId < other.mTileId; }

private:
    int mTileId;
    WangId mWangId;
};

TILEDSHARED_EXPORT QDebug operator<<(QDebug debug, const WangTile &wangTile);


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

    int distanceToColor(int targetColor) const;

private:
    friend class WangSet;

    void setColorIndex(int colorIndex) { mColorIndex = colorIndex; }

    WangSet *mWangSet = nullptr;
    int mColorIndex;
    QString mName;
    QColor mColor;
    int mImageId;
    qreal mProbability;

    QVector<int> mDistanceToColor;
};

/**
 * Returns the transition penalty(/distance) from this color to another.
 */
inline int WangColor::distanceToColor(int targetColor) const
{
    return mDistanceToColor.at(targetColor);
}

/**
 * Represents a Wang set.
 */
class TILEDSHARED_EXPORT WangSet : public Object
{
public:
    enum Type {
        Corner,
        Edge,
        Mixed
    };

    WangSet(Tileset *tileset,
            const QString &name,
            Type type,
            int imageTileId = -1);

    Tileset *tileset() const;
    void setTileset(Tileset *tileset);

    QString name() const;
    void setName(const QString &name);

    Type type() const;
    void setType(Type type);

    WangId typeMask() const;

    int imageTileId() const;
    void setImageTileId(int imageTileId);
    Tile *imageTile() const;

    int colorCount() const;
    void setColorCount(int n);

    void insertWangColor(const QSharedPointer<WangColor> &wangColor);
    void addWangColor(const QSharedPointer<WangColor> &wangColor);
    QSharedPointer<WangColor> takeWangColorAt(int color);

    const QSharedPointer<WangColor> &colorAt(int index) const;
    const QVector<QSharedPointer<WangColor>> &colors() const { return mColors; }

    void setWangId(int tileId, WangId wangId);

    const QHash<int, WangId> &wangIdByTileId() const { return mTileIdToWangId; }

    struct WangIdAndCell
    {
        WangId wangId;
        Cell cell;
    };

    const QVector<WangIdAndCell> &wangIdsAndCells() const;

    QList<WangTile> sortedWangTiles() const;

    WangId wangIdOfTile(const Tile *tile) const;
    WangId wangIdOfCell(const Cell &cell) const;

    qreal wangIdProbability(WangId wangId) const;

    bool wangIdIsValid(WangId wangId) const;

    static bool wangIdIsValid(WangId wangId, int colorCount);

    bool wangIdIsUsed(WangId wangId, WangId mask = WangId::FULL_MASK) const;

    int transitionPenalty(int colorA, int colorB) const;
    int maximumColorDistance() const;

    bool isEmpty() const;
    bool isComplete() const;
    quint64 completeSetSize() const;

    Type effectiveTypeForColor(int color) const;

    WangId templateWangIdAt(unsigned n) const;

    WangSet *clone(Tileset *tileset) const;

private:
    void removeTileId(int tileId);

    bool cellsDirty() const;
    void recalculateCells();
    void recalculateColorDistances();

    Tileset *mTileset;
    QString mName;
    Type mType;
    WangId mTypeMask;
    int mImageTileId;

    // How many unique, full WangIds are active in this set.
    // Where full means the id has no wildcards
    quint64 mUniqueFullWangIdCount = 0;

    QVector<QSharedPointer<WangColor>> mColors;
    QHash<int, WangId> mTileIdToWangId;

    QVector<WangIdAndCell> mWangIdAndCells;

    int mMaximumColorDistance = 0;
    bool mColorDistancesDirty = true;
    bool mCellsDirty = true;
    Tileset::TransformationFlags mLastSeenTranslationFlags;
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

inline WangSet::Type WangSet::type() const
{
    return mType;
}

inline WangId WangSet::typeMask() const
{
    return mTypeMask;
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
    return mColors.size();
}

inline const QSharedPointer<WangColor> &WangSet::colorAt(int index) const
{
    Q_ASSERT(index > 0 && index <= colorCount());

    return mColors.at(index - 1);
}

inline bool WangSet::isEmpty() const
{
    return mTileIdToWangId.isEmpty();
}

inline bool WangSet::cellsDirty() const
{
    return mCellsDirty || mLastSeenTranslationFlags != mTileset->transformationFlags();
}

TILEDSHARED_EXPORT QString wangSetTypeToString(WangSet::Type type);
TILEDSHARED_EXPORT WangSet::Type wangSetTypeFromString(const QString &);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::WangSet*)
Q_DECLARE_METATYPE(Tiled::WangId)
