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

#include <QStack>
#include <QtMath>

using namespace Tiled;

unsigned cellToTileInfo(const Cell &cell)
{
    return cell.tileId()
            | (cell.flippedHorizontally() << 29)
            | (cell.flippedVertically() << 28)
            | (cell.flippedAntiDiagonally() << 27);
}

unsigned wangTileToTileInfo(const WangTile &wangTile)
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
    Q_ASSERT(index >= 0 && index < 4);
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
    Q_ASSERT(index >= 0 && index < 4);
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
    return (mId >> (index * 4)) & 0xf;
}

void WangId::setEdgeColor(int index, unsigned value)
{
    Q_ASSERT(index >= 0 && index < 4);
    setIndexColor(index * 2, value);
}

void WangId::setCornerColor(int index, unsigned value)
{
    Q_ASSERT(index >= 0 && index < 4);
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
    mId &= ~(0xf << (index * 4));
    mId |= (value & 0xf) << (index * 4);
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
    int index = position / 2;
    bool isCorner = position & 1;

    if (isCorner) {
        setCornerColor(index, adjacent.cornerColor((index + 2) % 4));
    } else {
        setEdgeColor(index, adjacent.edgeColor((index + 2) % 4));
        setCornerColor(index, adjacent.cornerColor((index + 1) % 4));
        setCornerColor((index + 3) % 4, adjacent.cornerColor((index + 2) % 4));
    }
}

/**
 * Returns true if one or more indexes have no color.
 */
bool WangId::hasWildCards() const
{
    for (int i = 0; i < NumIndexes; ++i) {
        if (!indexColor(i))
            return true;
    }

    return false;
}

/**
 * Returns a mask that is 0 for any indexes that have no color defined.
 */
unsigned WangId::mask() const
{
    unsigned mask = 0;
    for (int i = 0; i < NumIndexes; ++i) {
        if (indexColor(i))
            mask |= 0xf << (i * 4);
    }
    return mask;
}

/**
 * Rotates the wang Id clockwise by (90 * rotations) degrees.
 * Meaning with one rotation, the top edge becomes the right edge,
 * and the top right corner, becomes the top bottom.
 */
void WangId::rotate(int rotations)
{
    if (rotations < 0)
        rotations = 4 + (rotations % 4);
    else
        rotations %= 4;

    unsigned rotated = mId << rotations * 8;
    rotated = rotated | (mId >> ((4 - rotations) * 8));

    mId = rotated;
}

/**
 * Flips the wang Id horizontally.
 */
void WangId::flipHorizontally()
{
    WangId newWangId = mId;

    newWangId.setIndexColor(WangId::Right, indexColor(WangId::Left));
    newWangId.setIndexColor(WangId::Left, indexColor(WangId::Right));

    for (int i = 0; i < 4; ++i)
        newWangId.setCornerColor(i, cornerColor(3-i));

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


static const QColor defaultWangColors[] =  {
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
                 int imageTileId):
    Object(Object::WangSetType),
    mTileset(tileset),
    mName(name),
    mImageTileId(imageTileId),
    mUniqueFullWangIdCount(0)
{
    Q_ASSERT(tileset);
}

/**
 * Sets the color count.
 *
 * This can make wangIds already in the set invalid, so should only be used
 * from ChangeWangSetColorCount.
 */
void WangSet::setColorCount(int n)
{
    Q_ASSERT(n >= 0 && n <= 15);

    if (n == colorCount())
        return;

    if (n < colorCount()) {
        mColors.resize(n);
    } else {
        while (mColors.size() < n) {
            const QColor &color = defaultWangColors[mColors.size()];
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
}

QList<Tile *> WangSet::tilesChangedOnSetColorCount(int newColorCount) const
{
    QList<Tile *> tiles;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        if (!wangIdIsValid(i.value(), newColorCount)) {
            int tileId = i.key() & 0x1fffffff;
            tiles.append(mTileset->findTile(tileId));
        }
    }

    return tiles;
}

QList<Tile *> WangSet::tilesChangedOnRemoveColor(int color) const
{
    QList<Tile *> tiles;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        for (int j = 0; j < WangId::NumIndexes; ++j) {
            int c = i.value().indexColor(j);
            int tileId = i.key() & 0x1fffffff;
            if (c >= color) {
                tiles.append(mTileset->findTile(tileId));
                break;
            }
        }
    }

    return tiles;
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
 * Finds all the tiles which match the given \a wangId, where zeros in the id
 * are treated as wild cards, and can be any color.
 */
QList<WangTile> WangSet::findMatchingWangTiles(WangId wangId) const
{
    QList<WangTile> list;

    const unsigned mask = wangId.mask();

    for (const WangTile &wangTile : mWangIdToWangTile) {
        if ((wangTile.wangId() & mask) == wangId)
            list.append(wangTile);
    }

    return list;
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
    unsigned id = 0;

    // Edges
    for (int i = 0; i < 4; ++i)
        id |= (surroundingWangIds[i*2].edgeColor((2 + i) % 4)) << (i*8);

    // Corners
    for (int i = 0; i < 4; ++i) {
        int color = surroundingWangIds[i*2 + 1].cornerColor((2 + i) % 4);

        if (!color)
            color = surroundingWangIds[i*2].cornerColor((1 + i) % 4);

        if (!color)
            color = surroundingWangIds[(i*2 + 2) % 8].cornerColor((3 + i) % 4);

        id |= color << (4 + i*8);
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
    WangId wangIds[8];

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
 */
bool WangSet::wangIdIsUsed(WangId wangId) const
{
    return mWangIdToWangTile.contains(wangId);
}

/**
 * Returns true if the given wangId is assigned to a tile,
 * or any variations of the 0 spots are.
 */
bool WangSet::wildWangIdIsUsed(WangId wangId) const
{
    const unsigned mask = wangId.mask();

    for (const WangTile &wangTile : mWangIdToWangTile) {
        if ((wangTile.wangId() & mask) == wangId)
            return true;
    }

    return false;
}

/**
 * Returns whether every template wangTile is filled.
 */
bool WangSet::isComplete() const
{
    return mUniqueFullWangIdCount == completeSetSize();
}

/**
 * Returns the amount of tiles expected in a complete tileset.
 */
unsigned WangSet::completeSetSize() const
{
    // TODO: When we support WangSet type (Edges, Corners, etc.) this will need adjustment.
    unsigned c = static_cast<unsigned>(colorCount());
    return c * c * c * c * c * c * c * c;
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
        return 0;

    unsigned wangId = 0;

    for (int i = 7; i >= 0; --i) {
        //this is the number of permutations possible bellow this point in the wangId
        const int belowPermutations = qPow(colorCount(), i);
        const int value = n / belowPermutations;

        n -= value * belowPermutations;

        wangId |= value << i * 4;
    }

    //before this is like a base 10 range (0 - 9) where we want (1 - 10) for each digit
    wangId += 0x11111111;

    return wangId;
}

WangSet *WangSet::clone(Tileset *tileset) const
{
    // Caller is responsible for adding the WangSet to this tileset
    WangSet *c = new WangSet(tileset, mName, mImageTileId);

    c->mUniqueFullWangIdCount = mUniqueFullWangIdCount;
    c->mColors = mColors;
    c->mWangIdToWangTile = mWangIdToWangTile;
    c->mTileInfoToWangId = mTileInfoToWangId;
    c->setProperties(properties());

    // Avoid sharing Wang colors
    for (QSharedPointer<WangColor> &wangColor : c->mColors) {
        const auto properties = wangColor->properties();
        wangColor = QSharedPointer<WangColor>::create(wangColor->colorIndex(),
                                                      wangColor->name(),
                                                      wangColor->color(),
                                                      wangColor->imageId(),
                                                      wangColor->probability());
        wangColor->setProperties(properties);
        wangColor->mWangSet = c;
    }

    return c;
}
