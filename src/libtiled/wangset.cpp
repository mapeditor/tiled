/*
 * wangset.cpp
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

#include "tilelayer.h"
#include "wangset.h"

#include <QDebug>
#include <QStack>
#include <QtMath>

namespace Tiled {

/**
 * These return the color of the edge of the WangId.
 * 0 being the top edge:
 *
 *       |0|
 *      3|.|1
 *       |2|
 */
int WangId::edgeColor(int index) const
{
    Q_ASSERT(index >= 0 && index < NumEdges);
    return indexColor(index * 2);
}

/**
 * These return the color of the corner of the WangId.
 * 0 being the top right corner:
 *
 *      3| |0
 *       |.|
 *      2| |1
 */
int WangId::cornerColor(int index) const
{
    Q_ASSERT(index >= 0 && index < NumCorners);
    return indexColor(index * 2 + 1);
}

/**
 * Returns the color of a certain index 0 - 7.
 *
 *      7|0|1
 *      6|.|2
 *      5|4|3
 */
int WangId::indexColor(int index) const
{
    Q_ASSERT(index >= 0 && index < NumIndexes);
    return (mId >> (index * BITS_PER_INDEX)) & INDEX_MASK;
}

void WangId::setEdgeColor(int index, unsigned value)
{
    Q_ASSERT(index >= 0 && index < NumEdges);
    setIndexColor(index * 2, value);
}

void WangId::setCornerColor(int index, unsigned value)
{
    Q_ASSERT(index >= 0 && index < NumCorners);
    setIndexColor(index * 2 + 1, value);
}

/**
 * Sets the color of a certain grid index:
 *
 *      y
 *    x 0|1|2
 *      1|.|.
 *      2|.|.
 */
void WangId::setGridColor(int x, int y, unsigned value)
{
    const int index = indexByGrid(x, y);
    if (index < NumIndexes)
        setIndexColor(index, value);
}

/**
 * Sets the color of a certain index 0 - 7.
 *
 *      7|0|1
 *      6|.|2
 *      5|4|3
 */
void WangId::setIndexColor(int index, unsigned value)
{
    Q_ASSERT(index >= 0 && index < NumIndexes);
    mId &= ~(INDEX_MASK << (index * BITS_PER_INDEX));
    mId |= quint64(value & INDEX_MASK) << (index * BITS_PER_INDEX);
}

/**
 * Matches this WangId's edges/corners with an \a adjacent one.
 * Where \a position is 0-7 with 0 being top, and 7 being top left:
 *
 *      7|0|1
 *      6|.|2
 *      5|4|3
 */
void WangId::updateToAdjacent(WangId adjacent, int position)
{
    setIndexColor(position, adjacent.indexColor(oppositeIndex(position)));

    if (!isCorner(position)) {
        const int cornerIndex = position / 2;
        setCornerColor(cornerIndex, adjacent.cornerColor((cornerIndex + 1) % NumCorners));
        setCornerColor((cornerIndex + 3) % NumCorners, adjacent.cornerColor((cornerIndex + 2) % NumCorners));
    }
}

/**
 * Returns true if one or more indexes have no color.
 */
bool WangId::hasWildCards() const
{
    for (int i = 0; i < NumIndexes; ++i)
        if (!indexColor(i))
            return true;

    return false;
}

/**
 * Returns true if one or more corners have no color.
 */
bool WangId::hasCornerWildCards() const
{
    for (int i = 0; i < NumCorners; ++i)
        if (!cornerColor(i))
            return true;

    return false;
}

/**
 * Returns true if one or more edges have no color.
 */
bool WangId::hasEdgeWildCards() const
{
    for (int i = 0; i < NumEdges; ++i)
        if (!edgeColor(i))
            return true;

    return false;
}

/**
 * Returns a mask that is 0 for any indexes that have no color defined.
 */
WangId WangId::mask() const
{
    quint64 mask = 0;
    for (int i = 0; i < NumIndexes; ++i) {
        if (indexColor(i))
            mask |= INDEX_MASK << (i * BITS_PER_INDEX);
    }
    return mask;
}

/**
 * Returns a mask that is 0 for any indexes that don't match the given color.
 */
WangId WangId::mask(int value) const
{
    quint64 mask = 0;
    for (int i = 0; i < NumIndexes; ++i) {
        if (indexColor(i) == value)
            mask |= INDEX_MASK << (i * BITS_PER_INDEX);
    }
    return mask;
}

bool WangId::hasCornerWithColor(int value) const
{
    for (int i = 0; i < NumCorners; ++i) {
        if (cornerColor(i) == value)
            return true;
    }
    return false;
}

bool WangId::hasEdgeWithColor(int value) const
{
    for (int i = 0; i < NumEdges; ++i) {
        if (edgeColor(i) == value)
            return true;
    }
    return false;
}

/**
 * Rotates the wang Id clockwise by (90 * rotations) degrees.
 * Meaning with one rotation, the top edge becomes the right edge,
 * and the top right corner, becomes the bottom right.
 */
void WangId::rotate(int rotations)
{
    *this = rotated(rotations);
}

/**
 * @see rotate
 */
WangId WangId::rotated(int rotations) const
{
    if (rotations < 0)
        rotations = 4 + (rotations % 4);
    else
        rotations %= 4;

    quint64 rotated = mId << (rotations * BITS_PER_INDEX * 2);
    rotated = rotated | (mId >> ((4 - rotations) * BITS_PER_INDEX * 2));

    return rotated;
}

/**
 * Flips the wang Id horizontally.
 */
void WangId::flipHorizontally()
{
    *this = flippedHorizontally();
}

/**
 * Flips the wang Id vertically.
 */
void WangId::flipVertically()
{
    flipHorizontally();
    rotate(2);
}

WangId WangId::flippedHorizontally() const
{
    WangId newWangId = mId;

    newWangId.setIndexColor(WangId::Right, indexColor(WangId::Left));
    newWangId.setIndexColor(WangId::Left, indexColor(WangId::Right));

    for (int i = 0; i < NumCorners; ++i)
        newWangId.setCornerColor(i, cornerColor(NumCorners - 1 - i));

    return newWangId;
}

WangId WangId::flippedVertically() const
{
    WangId newWangId = mId;
    newWangId.flipVertically();
    return newWangId;
}

WangId::Index WangId::indexByGrid(int x, int y)
{
    Q_ASSERT(x >= 0 && x < 3);
    Q_ASSERT(y >= 0 && y < 3);

    static constexpr Index map[3][3] = {
        { TopLeft,      Top,        TopRight },
        { Left,         NumIndexes, Right },
        { BottomLeft,   Bottom,     BottomRight },
    };

    return map[y][x];
}

/**
 * Creates a WangId based on the 32-bit value, which uses 4 bits per index.
 * Provided for compatibility.
 */
WangId WangId::fromUint(unsigned id)
{
    quint64 id64 = 0;
    for (int i = 0; i < NumIndexes; ++i) {
        const quint64 color = (id >> (i * 4)) & 0xF;
        id64 |= color << (i * BITS_PER_INDEX);
    }
    return id64;
}

/**
 * Converts the WangId to a 32-bit value, using 4 bits per index.
 * Provided for compatibility.
 */
unsigned WangId::toUint() const
{
    unsigned id = 0;
    for (int i = 0; i < NumIndexes; ++i) {
        const unsigned color = (mId >> (i * BITS_PER_INDEX)) & INDEX_MASK;
        id |= color << (i * 4);
    }
    return id;
}

WangId WangId::fromString(QStringRef string, bool *ok)
{
    WangId id;

    const auto parts = string.split(QLatin1Char(','));
    if (parts.size() == NumIndexes) {
        for (int i = 0; i < NumIndexes; ++i) {
            unsigned color = parts[i].toUInt(ok);
            if (ok && !(*ok))
                return id;

            if (color > WangId::MAX_COLOR_COUNT) {
                if (ok)
                    *ok = false;
                return id;
            }

            id.setIndexColor(i, color);
        }
    } else if (ok) {
        *ok = false;
    }

    return id;
}

QString WangId::toString() const
{
    QString result;
    for (int i = 0; i < NumIndexes; ++i ) {
        if (i > 0)
            result += QLatin1Char(',');
        result += QString::number(indexColor(i));
    }
    return result;
}


QDebug operator<<(QDebug debug, WangId wangId)
{
    QDebugStateSaver state(debug);
    debug.nospace().noquote() << "WangId(" << wangId.toString() << ')';
    return debug;
}

QDebug operator<<(QDebug debug, const WangTile &wangTile)
{
    QDebugStateSaver state(debug);
    debug.nospace() << "WangTile(" << wangTile.tileId() << ", " << wangTile.wangId() << ')';
    return debug;
}


WangColor::WangColor()
    : WangColor(0, QString(), Qt::red, -1)
{}

WangColor::WangColor(int colorIndex, const QString &name, const QColor &color, int imageId, qreal probability)
    : Object(WangColorType)
    , mColorIndex(colorIndex)
    , mName(name)
    , mColor(color)
    , mImageId(imageId)
    , mProbability(probability)
{}


static const QColor defaultWangColors[] = {
    QColor(255, 0, 0),
    QColor(0, 255, 0),
    QColor(0, 0, 255),
    QColor(255, 119, 0),
    QColor(0, 233, 255),
    QColor(255, 0, 216),
    QColor(255, 255, 0),
    QColor(160, 0, 255),
    QColor(0, 255, 161),
    QColor(255, 168, 168),
    QColor(180, 168, 255),
    QColor(150, 255, 167),
    QColor(142, 120, 72),
    QColor(90, 90, 90),
    QColor(14, 122, 70)
};

WangSet::WangSet(Tileset *tileset,
                 const QString &name,
                 Type type,
                 int imageTileId)
    : Object(Object::WangSetType)
    , mTileset(tileset)
    , mName(name)
    , mImageTileId(imageTileId)
{
    setType(type);
}

/**
 * Changes the type of this Wang set.
 *
 * Does not modify any WangIds to make sure they adhere to the type! Instead,
 * a type mask is applied where relevant.
 */
void WangSet::setType(Type type)
{
    mType = type;

    switch (type) {
    case Corner:
        mTypeMask = WangId::MaskCorners;
        break;
    case Edge:
        mTypeMask = WangId::MaskEdges;
        break;
    default:
    case Mixed:
        mTypeMask = WangId::FULL_MASK;
        break;
    }

    mColorDistancesDirty = true;
    mCellsDirty = true;
}

/**
 * Sets the color count.
 *
 * This can make wangIds already in the set invalid, so should only be used
 * from ChangeWangSetColorCount.
 */
void WangSet::setColorCount(int n)
{
    Q_ASSERT(n >= 0 && n <= WangId::MAX_COLOR_COUNT);

    if (n == colorCount())
        return;

    if (n < colorCount()) {
        mColors.resize(n);
    } else {
        while (mColors.size() < n) {
            QColor color;
            if (mColors.size() < 16)
                color = defaultWangColors[mColors.size()];
            else
                color = QColor(rand() % 256, rand() % 256, rand() % 256);

            mColors.append(QSharedPointer<WangColor>::create(mColors.size() + 1,
                                                             QString(),
                                                             color));
            mColors.last()->mWangSet = this;
        }
    }
}

/**
 * Inserts a given wangColor into the wangSet.
 * If the color is greater than current count, it must only be one greater.
 * For use in an undo command (does not adjust currently assigned tiles).
 */
void WangSet::insertWangColor(const QSharedPointer<WangColor> &wangColor)
{
    Q_ASSERT(colorCount() + 1 >= wangColor->colorIndex());

    wangColor->mWangSet = this;
    mColors.insert(wangColor->colorIndex() - 1, wangColor);

    for (int i = wangColor->colorIndex(); i < colorCount(); ++i)
        mColors.at(i)->setColorIndex(i + 1);

    mColorDistancesDirty = true;
}

/**
 * Adds a \a wangColor to the set.
 * The Wang colors color index may be changed.
 */
void WangSet::addWangColor(const QSharedPointer<WangColor> &wangColor)
{
    wangColor->setColorIndex(mColors.size() + 1);
    insertWangColor(wangColor);
}

/**
 * Removes and returns a given \a color.
 *
 * This can make wangIds invalid, so should only be used from
 * changewangsetdata.h
 */
QSharedPointer<WangColor> WangSet::takeWangColorAt(int color)
{
    Q_ASSERT(color > 0 && color - 1 < colorCount());

    auto wangColor = mColors.takeAt(color - 1);
    wangColor->mWangSet = nullptr;

    for (int i = color - 1; i < colorCount(); ++i)
        mColors.at(i)->setColorIndex(i + 1);

    mColorDistancesDirty = true;
    return wangColor;
}

/**
 * Associates the given \a wangId with the given \a tileId.
 *
 * If the given WangTile is already in the set with a different wangId, then
 * that reference is removed, and replaced with the new wangId. If the wangId
 * provided is zero then the wangTile is removed if already in the set.
 */
void WangSet::setWangId(int tileId, WangId wangId)
{
    Q_ASSERT(wangIdIsValid(wangId));

    if (WangId previousWangId = mTileIdToWangId.value(tileId)) {
        // return when the same tile is already part of this set with the same WangId
        if (previousWangId == wangId)
            return;

        removeTileId(tileId);
    }

    if (wangId.isEmpty())
        return;

    mTileIdToWangId.insert(tileId, wangId);
    mColorDistancesDirty = true;
    mCellsDirty = true;
}

void WangSet::removeTileId(int tileId)
{
    mTileIdToWangId.remove(tileId);
    mColorDistancesDirty = true;
    mCellsDirty = true;
}

/**
 * Returns the list of WangIds and their corresponding cells, as defined by
 * this Wang set.
 *
 * The WangIds are masked based on the type of the Wang set.
 */
const QVector<WangSet::WangIdAndCell> &WangSet::wangIdsAndCells() const
{
    if (cellsDirty())
        const_cast<WangSet*>(this)->recalculateCells();
    return mWangIdAndCells;
}

void WangSet::recalculateCells()
{
    mWangIdAndCells.clear();
    mCellsDirty = false;
    mUniqueFullWangIdCount = 0;

    const auto mask = typeMask();
    QSet<WangId> addedWangIds;

    // First insert all available tiles
    QHashIterator<int, WangId> it(mTileIdToWangId);
    while (it.hasNext()) {
        it.next();

        const auto wangId = WangId(it.value() & mask);

        mUniqueFullWangIdCount += !wangId.hasWildCards() && !addedWangIds.contains(wangId);
        addedWangIds.insert(wangId);
        mWangIdAndCells.append({wangId, Cell(mTileset, it.key())});
    }

    const auto transformationFlags = tileset()->transformationFlags();
    mLastSeenTranslationFlags = transformationFlags;

    if (!(transformationFlags & ~Tileset::PreferUntransformed))
        return;

    // Then insert variations based on flipping
    it.toFront();
    while (it.hasNext()) {
        it.next();

        const auto wangId = WangId(it.value() & mask);

        Cell cells[8] = { Cell(mTileset, it.key()) };
        WangId wangIds[8] = { wangId };
        int count = 1;
        const bool hasWildCards = wangId.hasWildCards();

        // Add 90, 180 and 270 degree rotations if enabled
        if (transformationFlags.testFlag(Tileset::AllowRotate)) {
            for (int i = 0; i < 3; ++i) {
                cells[count + i] = cells[i];
                cells[count + i].rotate(RotateRight);
                wangIds[count + i] = wangIds[i].rotated(1);
            }

            count = 4;
        }

        if (transformationFlags.testFlag(Tileset::AllowFlipHorizontally)) {
            for (int i = 0; i < count; ++i) {
                cells[count + i] = cells[i];
                cells[count + i].setFlippedHorizontally(!cells[count + i].flippedHorizontally());
                wangIds[count + i] = wangIds[i].flippedHorizontally();
            }

            count *= 2;
        }

        if (count <= 4 && transformationFlags.testFlag(Tileset::AllowFlipVertically)) {
            for (int i = 0; i < count; ++i) {
                cells[count + i] = cells[i];
                cells[count + i].setFlippedVertically(!cells[count + i].flippedVertically());
                wangIds[count + i] = wangIds[i].flippedVertically();
            }

            count *= 2;
        }

        for (int i = 1; i < count; ++i) {
            const bool exists = addedWangIds.contains(wangIds[i]);
            if (transformationFlags.testFlag(Tileset::PreferUntransformed) && exists)
                continue;
            mUniqueFullWangIdCount += !hasWildCards && !exists;
            addedWangIds.insert(wangIds[i]);
            mWangIdAndCells.append({wangIds[i], cells[i]});
        }
    }
}

/**
 * Calculates the distances between Wang colors.
 *
 * Distance between colors is the minimum number of tiles required before one
 * color may meet another. Colors that have no transition path have a distance
 * of -1.
 */
void WangSet::recalculateColorDistances()
{
    int maximumDistance = 1;

    for (int i = 1; i <= colorCount(); ++i) {
        WangColor &color = *colorAt(i);
        QVector<int> distance(colorCount() + 1, -1);

        // Check all tiles for transitions to other Wang colors
        for (WangId wangId : std::as_const(mTileIdToWangId)) {
            wangId &= typeMask();

            // Don't consider edges and corners to be connected. This helps
            // avoid seeing transitions to "no color" for edge or corner
            // based sets.

            if (wangId.hasCornerWithColor(i)) {
                for (int index = 0; index < 4; ++index)
                    distance[wangId.cornerColor(index)] = 1;
            }

            if (wangId.hasEdgeWithColor(i)) {
                for (int index = 0; index < 4; ++index)
                    distance[wangId.edgeColor(index)] = 1;
            }
        }

        // Color has at least one tile of its own type
        distance[i] = 0;

        color.mDistanceToColor = distance;
    }

    // Calculate indirect transition distances
    bool newConnections;
    do {
        newConnections = false;

        // For each combination of colors
        for (int i = 1; i <= colorCount(); ++i) {
            WangColor &colorI = *colorAt(i);

            for (int j = 1; j <= colorCount(); ++j) {
                if (i == j)
                    continue;

                WangColor &colorJ = *colorAt(j);

                // Scan through each color, and see if we have any in common
                for (int t = 0; t <= colorCount(); ++t) {
                    const int d0 = colorI.distanceToColor(t);
                    const int d1 = colorJ.distanceToColor(t);
                    if (d0 == -1 || d1 == -1)
                        continue;

                    // We have found a common connection
                    int d = colorI.distanceToColor(j);
                    Q_ASSERT(colorJ.distanceToColor(i) == d);

                    // If the new path is shorter, record the new distance
                    if (d == -1 || d0 + d1 < d) {
                        d = d0 + d1;
                        colorI.mDistanceToColor[j] = d;
                        colorJ.mDistanceToColor[i] = d;
                        maximumDistance = qMax(maximumDistance, d);

                        // We're making progress, flag for another iteration...
                        newConnections = true;
                    }
                }
            }
        }

        // Repeat while we are still making new connections (could take a
        // number of iterations for distant colors to connect)
    } while (newConnections);

    mMaximumColorDistance = maximumDistance;
    mColorDistancesDirty = false;
}

/**
 * Returns a list of the wangTiles in this set, sorted by tileId.
 */
QList<WangTile> WangSet::sortedWangTiles() const
{
    QList<WangTile> wangTiles;
    wangTiles.reserve(mTileIdToWangId.size());

    QHashIterator<int, WangId> it(mTileIdToWangId);
    while (it.hasNext()) {
        it.next();
        wangTiles.append(WangTile(it.key(), it.value()));
    }

    std::stable_sort(wangTiles.begin(), wangTiles.end());
    return wangTiles;
}

/**
 * Returns the WangId of a given \a tile.
 *
 * The tile is expected to be from the tileset to which this WangSet belongs.
 */
WangId WangSet::wangIdOfTile(const Tile *tile) const
{
    Q_ASSERT(tile->tileset() == mTileset);
    return mTileIdToWangId.value(tile->id());
}

/**
 * Returns the WangId of a given \a cell.
 *
 * If the cell refers to a different tileset than the one to which this WangSet
 * belongs, an empty WangId is returned.
 *
 * The result is masked based on the type of the Wang set.
 */
WangId WangSet::wangIdOfCell(const Cell &cell) const
{
    WangId wangId;

    if (cell.tileset() == mTileset) {
        wangId = mTileIdToWangId.value(cell.tileId());

        if (cell.flippedAntiDiagonally()) {
            wangId.rotate(1);
            wangId.flipHorizontally();
        }
        if (cell.flippedHorizontally())
            wangId.flipHorizontally();
        if (cell.flippedVertically())
            wangId.flipVertically();
    }

    return wangId & typeMask();
}

/**
 * The probability of a given WangId of being selected.
 */
qreal WangSet::wangIdProbability(WangId wangId) const
{
    qreal probability = 1.0;

    for (int i = 0; i < WangId::NumIndexes; ++i) {
        if (int color = wangId.indexColor(i))
            probability *= colorAt(color)->probability();
    }

    return probability;
}

/**
 * Returns whether or not the given wangId is valid in the context of the
 * current wangSet
 */
bool WangSet::wangIdIsValid(WangId wangId) const
{
    return wangIdIsValid(wangId, colorCount());
}

bool WangSet::wangIdIsValid(WangId wangId, int colorCount)
{
    for (int i = 0; i < WangId::NumIndexes; ++i)
        if (wangId.indexColor(i) > colorCount)
            return false;

    return true;
}

/**
 * Returns whether the given \a wangId is assigned to a WangTile.
 *
 * When \a mask is given, returns whether there is a WangId assigned to a
 * WangTile matching the part of the \a wangId indicated by the mask.
 *
 * The \a wangId is automatically masked based on the type of the Wang set.
 */
bool WangSet::wangIdIsUsed(WangId wangId, WangId mask) const
{
    mask &= typeMask();
    const WangId maskedWangId = wangId & mask;

    for (const auto &wangIdAndCell : wangIdsAndCells())
        if ((wangIdAndCell.wangId & mask) == maskedWangId)
            return true;

    return false;
}

int WangSet::transitionPenalty(int colorA, int colorB) const
{
    if (mColorDistancesDirty)
        const_cast<WangSet*>(this)->recalculateColorDistances();

    // Do some magic, since we don't have a transition array for no-color
    if (colorA == 0 && colorB == 0)
        return 0;

    if (colorA == 0)
        return colorAt(colorB)->mDistanceToColor[colorA];

    return colorAt(colorA)->mDistanceToColor[colorB];
}

int WangSet::maximumColorDistance() const
{
    if (mColorDistancesDirty)
        const_cast<WangSet*>(this)->recalculateColorDistances();

    return mMaximumColorDistance;
}

/**
 * Returns whether every template wangTile is filled.
 */
bool WangSet::isComplete() const
{
    if (cellsDirty())
        const_cast<WangSet*>(this)->recalculateCells();

    return mUniqueFullWangIdCount == completeSetSize();
}

/**
 * Returns the amount of tiles expected in a complete Wang set.
 */
quint64 WangSet::completeSetSize() const
{
    auto c = static_cast<quint64>(colorCount());

    switch (mType) {
    case Corner:
    case Edge:
        return c * c * c * c;
    case Mixed:
    default:
        return c * c * c * c * c * c * c * c;
    }
}

WangSet::Type WangSet::effectiveTypeForColor(int color) const
{
    if (type() == Mixed) {
        // Determine a meaningful mode by looking at where the color is used.
        bool usedAsCorner = false;
        bool usedAsEdge = false;

        if (color > 0 && color <= colorCount()) {
            for (const WangId wangId : wangIdByTileId()) {
                for (int i = 0; i < WangId::NumIndexes; ++i) {
                    if (wangId.indexColor(i) == color) {
                        const bool isCorner = WangId::isCorner(i);
                        usedAsCorner |= isCorner;
                        usedAsEdge |= !isCorner;
                    }
                }
            }
        }

        if (usedAsEdge == usedAsCorner)
            return Mixed;
        if (usedAsEdge)
            return Edge;
        return Corner;
    }

    return type();
}

/**
 * Returns the Nth WangId starting at 0x11111111
 * and, when C is the number of colors, ending at 0xCCCCCCCC.
 *
 * Note this does NOT include wildcards (no zeros).
 */
WangId WangSet::templateWangIdAt(unsigned n) const
{
    if (colorCount() <= 0)
        return {};

    WangId wangId;

    switch (mType) {
    case Corner:
        for (int i = WangId::NumCorners - 1; i >= 0; --i) {
            const int belowPermutations = qPow(colorCount(), i);
            const int value = n / belowPermutations;

            n -= value * belowPermutations;

            wangId.setCornerColor(i, value + 1);
        }
        break;
    case Edge:
        for (int i = WangId::NumEdges - 1; i >= 0; --i) {
            //this is the number of permutations possible below this point in the wangId
            const int belowPermutations = qPow(colorCount(), i);
            const int value = n / belowPermutations;

            n -= value * belowPermutations;

            wangId.setEdgeColor(i, value + 1);
        }
        break;
    case Mixed:
        for (int i = WangId::NumIndexes - 1; i >= 0; --i) {
            const int belowPermutations = qPow(colorCount(), i);
            const int value = n / belowPermutations;

            n -= value * belowPermutations;

            wangId.setIndexColor(i, value + 1);
        }
        break;
    }

    return wangId;
}

WangSet *WangSet::clone(Tileset *tileset) const
{
    // Caller is responsible for adding the WangSet to this tileset
    auto c = new WangSet(tileset, mName, mType, mImageTileId);

    c->setClassName(className());
    c->setProperties(properties());

    c->mUniqueFullWangIdCount = mUniqueFullWangIdCount;
    c->mColors = mColors;
    c->mTileIdToWangId = mTileIdToWangId;
    c->mWangIdAndCells = mWangIdAndCells;
    c->mMaximumColorDistance = mMaximumColorDistance;
    c->mColorDistancesDirty = mColorDistancesDirty;
    c->mCellsDirty = mCellsDirty;
    c->mLastSeenTranslationFlags = mLastSeenTranslationFlags;

    // Avoid sharing Wang colors
    for (QSharedPointer<WangColor> &wangColor : c->mColors) {
        const auto distanceToColor = wangColor->mDistanceToColor;


        auto clonedColor = QSharedPointer<WangColor>::create(wangColor->colorIndex(),
                                                             wangColor->name(),
                                                             wangColor->color(),
                                                             wangColor->imageId(),
                                                             wangColor->probability());
        clonedColor->setClassName(wangColor->className());
        clonedColor->setProperties(wangColor->properties());
        clonedColor->mWangSet = c;
        clonedColor->mDistanceToColor = distanceToColor;

        wangColor = std::move(clonedColor);
    }

    return c;
}

QString wangSetTypeToString(WangSet::Type type)
{
    switch (type) {
    case WangSet::Corner:
        return QStringLiteral("corner");
    case WangSet::Edge:
        return QStringLiteral("edge");
    case WangSet::Mixed:
        return QStringLiteral("mixed");
    }
    return QString();
}

WangSet::Type wangSetTypeFromString(const QString &string)
{
    WangSet::Type type = WangSet::Mixed;

    if (string == QLatin1String("edge"))
        type = WangSet::Edge;
    else if (string == QLatin1String("corner"))
        type = WangSet::Corner;

    return type;
}

} // namespace Tiled
