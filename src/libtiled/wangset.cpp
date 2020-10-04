/*
 * wangset.cpp
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

#include "tilelayer.h"
#include "wangset.h"

#include <QDebug>
#include <QStack>
#include <QtMath>

#include "qtcompat_p.h"

namespace Tiled {

static unsigned cellToTileInfo(const Cell &cell)
{
    return cell.tileId()
            | (cell.flippedHorizontally() << 29)
            | (cell.flippedVertically() << 28)
            | (cell.flippedAntiDiagonally() << 27);
}

static unsigned wangTileToTileInfo(const WangTile &wangTile)
{
    return wangTile.tile()->id()
            | (wangTile.flippedHorizontally() << 29)
            | (wangTile.flippedVertically() << 28)
            | (wangTile.flippedAntiDiagonally() << 27);
}

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
quint64 WangId::mask() const
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
quint64 WangId::mask(int value) const
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
 * and the top right corner, becomes the top bottom.
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
    WangId newWangId = mId;

    newWangId.setIndexColor(WangId::Right, indexColor(WangId::Left));
    newWangId.setIndexColor(WangId::Left, indexColor(WangId::Right));

    for (int i = 0; i < NumCorners; ++i)
        newWangId.setCornerColor(i, cornerColor(NumCorners - 1 - i));

    mId = newWangId;
}

/**
 * Flips the wang Id vertically.
 */
void WangId::flipVertically()
{
    flipHorizontally();
    rotate(2);
}

WangId::Index WangId::indexByGrid(int x, int y)
{
    Q_ASSERT(x >= 0 && x < 3);
    Q_ASSERT(y >= 0 && y < 3);

    const Index map[3][3] = {
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


// performs a translation (either flipping or rotating) based on a one to
// one map of size 8 (from 0 - 7)
void WangTile::translate(const int map[])
{
    int mask = (mFlippedHorizontally << 2)
            | (mFlippedVertically << 1)
            | (mFlippedAntiDiagonally << 0);

    mask = map[mask];

    mFlippedHorizontally = mask & 4;
    mFlippedVertically = mask & 2;
    mFlippedAntiDiagonally = mask & 1;
}

void WangTile::rotateRight()
{
    const int map[] = { 5, 4, 1, 0, 7, 6, 3, 2 };
    mWangId.rotate(1);

    translate(map);
}

void WangTile::rotateLeft()
{
    const int map[] = { 3, 2, 7, 6, 1, 0, 5, 4 };
    mWangId.rotate(3);

    translate(map);
}

void WangTile::flipHorizontally()
{
    const int map[] = { 4, 3, 6, 1, 0, 7, 2, 5 };
    mWangId.flipHorizontally();

    translate(map);
}

void WangTile::flipVertically()
{
    const int map[] = { 2, 5, 0, 7, 6, 1, 4, 3 };
    mWangId.flipVertically();

    translate(map);
}

Cell WangTile::makeCell() const
{
    if (!mTile)
        return Cell();

    Cell cell(mTile);
    cell.setFlippedHorizontally(mFlippedHorizontally);
    cell.setFlippedVertically(mFlippedVertically);
    cell.setFlippedAntiDiagonally(mFlippedAntiDiagonally);

    return cell;
}

QDebug operator<<(QDebug debug, WangId wangId)
{
    const bool oldSetting = debug.autoInsertSpaces();
    debug.nospace() << "WangId(" << wangId.toString() << ')';
    debug.setAutoInsertSpaces(oldSetting);
    return debug.maybeSpace();
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
    , mType(type)
    , mImageTileId(imageTileId)
{
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
 * Removes a given \a color.
 *
 * This can make wangIds invalid, so should only be used from
 * changewangsetdata.h
 */
void WangSet::removeWangColorAt(int color)
{
    Q_ASSERT(color > 0 && color - 1 < colorCount());

    mColors.at(color - 1)->mWangSet = nullptr;
    mColors.removeAt(color - 1);

    for (int i = color - 1; i < colorCount(); ++i)
        mColors.at(i)->setColorIndex(i + 1);

    mColorDistancesDirty = true;
}

/**
 * Adds a \a wangTile to the wang set.
 *
 * If the given WangTile is already in the set with a different wangId, then
 * that reference is removed, and replaced with the new wangId. If the wangId
 * provided is zero then the wangTile is removed if already in the set. Updates
 * the UniqueFullWangIdCount.
 */
void WangSet::addWangTile(const WangTile &wangTile)
{
    Q_ASSERT(wangTile.tile()->tileset() == mTileset);
    Q_ASSERT(wangIdIsValid(wangTile.wangId()));

    if (WangId previousWangId = mTileInfoToWangId.value(wangTileToTileInfo(wangTile))) {
        // return when the same tile is already part of this set with the same WangId
        if (previousWangId == wangTile.wangId())
            return;

        removeWangTile(wangTile);
    }

    if (wangTile.wangId() == 0)
        return;

    if (!wangTile.wangId().hasWildCards() && !mWangIdToWangTile.contains(wangTile.wangId()))
        ++mUniqueFullWangIdCount;

    mWangIdToWangTile.insert(wangTile.wangId(), wangTile);
    mTileInfoToWangId.insert(wangTileToTileInfo(wangTile), wangTile.wangId());

    mColorDistancesDirty = true;
}

void WangSet::removeWangTile(const WangTile &wangTile)
{
    WangId wangId = mTileInfoToWangId.take(wangTileToTileInfo(wangTile));

    WangTile w = wangTile;
    w.setWangId(wangId);

    mWangIdToWangTile.remove(wangId, w);

    if (wangId
            && !mWangIdToWangTile.contains(wangId)
            && !wangId.hasWildCards())
        --mUniqueFullWangIdCount;

    mColorDistancesDirty = true;
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
        for (const WangTile &tile : qAsConst(mWangIdToWangTile)) {

            // Don't consider edges and corners to be connected. This helps
            // avoid seeing transitions to "no color" for edge or corner
            // based sets.

            if (tile.wangId().hasCornerWithColor(i)) {
                for (int index = 0; index < 4; ++index)
                    distance[tile.wangId().cornerColor(index)] = 1;
            }

            if (tile.wangId().hasEdgeWithColor(i)) {
                for (int index = 0; index < 4; ++index)
                    distance[tile.wangId().edgeColor(index)] = 1;
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
        // number of iterations for distant terrain types to connect)
    } while (newConnections);

    mMaximumColorDistance = maximumDistance;
    mColorDistancesDirty = false;
}

/**
 * Returns a sorted list of the wangTiles in this set.
 * Sorted by tileId.
 */
QList<WangTile> WangSet::sortedWangTiles() const
{
    QList<WangTile> wangTiles = mWangIdToWangTile.values();
    std::stable_sort(wangTiles.begin(), wangTiles.end());
    return wangTiles;
}

/**
 * Returns a WangId matching that of the provided \a surroundingWangIds.
 *
 * This is based off a provided array, { 0, 1, 2, 3, 4, 5, 6, 7 },
 * which corresponds to:
 *
 *      7|0|1
 *      6|X|2
 *      5|4|3
 */
WangId WangSet::wangIdFromSurrounding(const WangId surroundingWangIds[]) const
{
    quint64 id = 0;

    // Edges
    for (int i = 0; i < WangId::NumEdges; ++i)
        id |= quint64(surroundingWangIds[i*2].edgeColor((2 + i) % WangId::NumEdges)) << (i * WangId::BITS_PER_INDEX * 2);

    // Corners
    for (int i = 0; i < WangId::NumCorners; ++i) {
        int color = surroundingWangIds[i*2 + 1].cornerColor((2 + i) % WangId::NumCorners);

        if (!color)
            color = surroundingWangIds[i*2].cornerColor((1 + i) % WangId::NumCorners);

        if (!color)
            color = surroundingWangIds[(i*2 + 2) % WangId::NumIndexes].cornerColor((3 + i) % WangId::NumCorners);

        id |= quint64(color) << (WangId::BITS_PER_INDEX + i * WangId::BITS_PER_INDEX * 2);
    }

    return id;
}

/**
 * Returns a wangId matching that of the provided surrounding tiles.
 *
 * This is based off a provided array, { 0, 1, 2, 3, 4, 5, 6, 7 },
 * which corresponds to:
 *
 *      7|0|1
 *      6|X|2
 *      5|4|3
 */
WangId WangSet::wangIdFromSurrounding(const Cell surroundingCells[]) const
{
    WangId wangIds[WangId::NumIndexes];

    for (int i = 0; i < WangId::NumIndexes; ++i)
        wangIds[i] = wangIdOfCell(surroundingCells[i]);

    return wangIdFromSurrounding(wangIds);
}

/**
 * Returns the WangId of a given \a tile.
 *
 * The tile is expected to be from the tileset to which this WangSet belongs.
 */
WangId WangSet::wangIdOfTile(const Tile *tile) const
{
    Q_ASSERT(tile->tileset() == mTileset);
    return mTileInfoToWangId.value(tile->id());
}

/**
 * Returns the WangId of a given \a cell.
 *
 * If the cell refers to a different tileset than the one to which this WangSet
 * belongs, an empty WangId is returned.
 */
WangId WangSet::wangIdOfCell(const Cell &cell) const
{
    if (cell.tileset() == mTileset)
        return mTileInfoToWangId.value(cellToTileInfo(cell));
    return WangId();
}

/**
 * The probability of a given wang tile of being selected.
 */
qreal WangSet::wangTileProbability(const WangTile &wangTile) const
{
    qreal probability = 1.0;
    WangId wangId = wangTile.wangId();

    for (int i = 0; i < WangId::NumIndexes; ++i) {
        if (int color = wangId.indexColor(i))
            probability *= colorAt(color)->probability();
    }

    if (Tile *tile = wangTile.tile())
        probability *= tile->probability();

    return probability;
}

/**
 * Returns whether or not the given wangId is valid in the contex of the
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
 */
bool WangSet::wangIdIsUsed(WangId wangId, WangId mask) const
{
    if (mask == WangId::FULL_MASK)
        return mWangIdToWangTile.contains(wangId);

    const quint64 maskedWangId = wangId & mask;

    for (const WangTile &wangTile : mWangIdToWangTile)
        if ((wangTile.wangId() & mask) == maskedWangId)
            return true;

    return false;
}

int WangSet::transitionPenalty(int colorA, int colorB) const
{
    if (mColorDistancesDirty)
        const_cast<WangSet*>(this)->recalculateColorDistances();

    // Do some magic, since we don't have a transition array for no-terrain
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
    return mUniqueFullWangIdCount == completeSetSize();
}

/**
 * Returns the amount of tiles expected in a complete Wang set.
 */
quint64 WangSet::completeSetSize() const
{
    quint64 c = static_cast<quint64>(colorCount());

    switch (mType) {
    case Corner:
    case Edge:
        return c * c * c * c;
    case Mixed:
    default:
        return c * c * c * c * c * c * c * c;
    }
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
            //this is the number of permutations possible bellow this point in the wangId
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
    WangSet *c = new WangSet(tileset, mName, mType, mImageTileId);

    c->mUniqueFullWangIdCount = mUniqueFullWangIdCount;
    c->mColors = mColors;
    c->mWangIdToWangTile = mWangIdToWangTile;
    c->mTileInfoToWangId = mTileInfoToWangId;
    c->mMaximumColorDistance = mMaximumColorDistance;
    c->mColorDistancesDirty = mColorDistancesDirty;
    c->setProperties(properties());

    // Avoid sharing Wang colors
    for (QSharedPointer<WangColor> &wangColor : c->mColors) {
        const auto properties = wangColor->properties();
        const auto distanceToColor = wangColor->mDistanceToColor;

        wangColor = QSharedPointer<WangColor>::create(wangColor->colorIndex(),
                                                      wangColor->name(),
                                                      wangColor->color(),
                                                      wangColor->imageId(),
                                                      wangColor->probability());
        wangColor->setProperties(properties);
        wangColor->mWangSet = c;
        wangColor->mDistanceToColor = distanceToColor;
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

void WangSet::addRotations(bool alternate) {
    QList<WangTile> to_add;
    for (const WangTile & i: mWangIdToWangTile) {
        for (unsigned rotation=1;rotation<4;++rotation) {
            WangId rotid=i.wangId().rotated(rotation);
            if (alternate || !mWangIdToWangTile.contains(rotid))
            {
                WangTile newtile = i;
                switch (rotation) {
                    case 1: newtile.rotateRight(); break;
                    case 2: newtile.rotateRight(); newtile.rotateRight(); break;
                    case 3: newtile.rotateLeft(); break;
                }
                to_add.push_back(std::move(newtile));
            }
        }
    }
    for (WangTile const& i:to_add) {
        addWangTile(i);
    }
}

} // namespace Tiled
