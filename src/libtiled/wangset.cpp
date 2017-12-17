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

int WangId::edgeColor(int index) const
{
    return indexColor(index * 2);
}

int WangId::cornerColor(int index) const
{
    return indexColor(index * 2 + 1);
}

int WangId::indexColor(int index) const
{
    return (mId >> (index * 4)) & 0xf;
}

void WangId::setEdgeColor(int index, unsigned value)
{
    setIndexColor(index * 2, value);
}

void WangId::setCornerColor(int index, unsigned value)
{
    setIndexColor(index * 2 + 1, value);
}

void WangId::setIndexColor(int index, unsigned value)
{
    value &= 0xf;
    mId &= ~(0xf << (index * 4));
    mId |= value << (index * 4);
}

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

bool WangId::hasEdgeWildCards() const
{
    for (int i = 0; i < 4; ++i) {
        if (!edgeColor(i))
            return true;
    }

    return false;
}

bool WangId::hasCornerWildCards() const
{
    for (int i = 0; i < 4; ++i) {
        if (!cornerColor(i))
            return true;
    }

    return false;
}

WangIdVariations WangId::variations(int edgeColors, int cornerColors) const
{
    return WangIdVariations(edgeColors, cornerColors, mId);
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

void WangId::flipHorizontally()
{
    WangId newWangId = mId;

    newWangId.setEdgeColor(1, edgeColor(3));
    newWangId.setEdgeColor(3, edgeColor(1));

    for (int i = 0; i < 4; ++i)
        newWangId.setCornerColor(i, cornerColor(3-i));

    mId = newWangId;
}

void WangId::flipVertically()
{
    flipHorizontally();
    rotate(2);
}

WangIdVariations::iterator::iterator(int edgeColors, int cornerColors, WangId wangId)
    : mCurrent(wangId)
    , mMax(wangId)
    , mEdgeColors(edgeColors)
    , mCornerColors(cornerColors)
{
    if (mEdgeColors > 1) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.edgeColor(i)) {
                mZeroSpots.append(i*2);

                mMax.setEdgeColor(i, mEdgeColors);
            }
        }
    }

    if (mCornerColors > 1) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.cornerColor(i)) {
                mZeroSpots.append(i*2 + 1);

                mMax.setCornerColor(i, mCornerColors);
            }
        }
    }
}

WangIdVariations::iterator &WangIdVariations::iterator::operator ++()
{
    if (mCurrent == mMax) {
        mCurrent = mCurrent + 1;
        return *this;
    }

    if (mZeroSpots.isEmpty())
        return *this;

    int index = 0;
    int currentSpot = mZeroSpots[0];
    while (true) {
        mCurrent.setIndexColor(currentSpot, mCurrent.indexColor(currentSpot) + 1);

        if (mCurrent.indexColor(currentSpot) > (currentSpot & 1? mCornerColors : mEdgeColors)) {
            mCurrent.setIndexColor(currentSpot, 0);
            if (++index >= mZeroSpots.size())
                break;
            currentSpot = mZeroSpots[index];
        } else {
            break;
        }
    }

    return *this;
}

WangIdVariations::iterator WangIdVariations::end() const
{
    WangId id = mWangId;

    if (mEdgeColors > 1) {
        for (int i = 0; i < 4; ++i) {
            if (!id.edgeColor(i))
                id.setEdgeColor(i, mEdgeColors);
        }
    }
    if (mCornerColors > 1) {
        for (int i = 0; i < 4; ++i) {
            if (!id.cornerColor(i))
                id.setCornerColor(i, mCornerColors);
        }
    }

    id = id + 1;

    return iterator(mEdgeColors, mCornerColors, id);
}

void WangTile::translate(int map[])
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
    int map[] = {5, 4, 1, 0, 7, 6, 3, 2};
    mWangId.rotate(1);

    translate(map);
}

void WangTile::rotateLeft()
{
    int map[] = {3, 2, 7, 6, 1, 0, 5, 4};
    mWangId.rotate(3);

    translate(map);
}

void WangTile::flipHorizontally()
{
    int map[] = {4, 3, 6, 1, 0, 7, 2, 5};
    mWangId.flipHorizontally();

    translate(map);
}

void WangTile::flipVertically()
{
    int map[] = {2, 5, 0, 7, 6, 1, 4, 3};
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
    : WangColor(0, true, QString(), Qt::red, -1)
{}

WangColor::WangColor(int colorIndex, bool isEdge, const QString &name, const QColor &color, int imageId, float probability)
    : Object(WangColorType)
    , mColorIndex(colorIndex)
    , mIsEdge(isEdge)
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
                 QString name,
                 int imageTileId):
    Object(Object::WangSetType),
    mTileset(tileset),
    mName(std::move(name)),
    mImageTileId(imageTileId),
    mUniqueFullWangIdCount(0)
{
    Q_ASSERT(tileset);
}

int WangSet::edgeColorCount() const
{
    return qMax(1, mEdgeColors.size());
}

int WangSet::cornerColorCount() const
{
    return qMax(1, mCornerColors.size());
}

void WangSet::setEdgeColorCount(int n)
{
    Q_ASSERT(n > 0 && n <= 15);

    if (n == edgeColorCount())
        return;

    if (n == 1) {
        mEdgeColors.clear();
        return;
    }

    if (n < edgeColorCount()) {
        mEdgeColors.resize(n);
    } else {
        while (mEdgeColors.size() != n) {
            mEdgeColors.append(QSharedPointer<WangColor>(new WangColor(mEdgeColors.size() + 1,
                                                                       true,
                                                                       QString(),
                                                                       defaultWangColors[mEdgeColors.size()],
                                                                       -1)));
        }
    }
}

void WangSet::setCornerColorCount(int n)
{
    Q_ASSERT(n > 0 && n <= 15);

    if (n == cornerColorCount())
        return;

    if (n == 1) {
        mCornerColors.clear();
        return;
    }

    if (n < cornerColorCount()) {
        mCornerColors.resize(n);
    } else {
        while (mCornerColors.size() != n) {
            mCornerColors.append(QSharedPointer<WangColor>(new WangColor(mCornerColors.size() + 1,
                                                                         false,
                                                                         QString(),
                                                                         defaultWangColors[mCornerColors.size()],
                                                                         -1)));
        }
    }
}

void WangSet::insertWangColor(QSharedPointer<WangColor> wangColor)
{
    if (wangColor->isEdge())
        insertEdgeWangColor(wangColor);
    else
        insertCornerWangColor(wangColor);
}


void WangSet::addWangColor(QSharedPointer<WangColor> wangColor)
{
    if (wangColor->isEdge()) {
        wangColor->setColorIndex(mEdgeColors.size() + 1);
        insertEdgeWangColor(wangColor);
    } else {
        wangColor->setColorIndex(mCornerColors.size() + 1);
        insertCornerWangColor(wangColor);
    }
}

void WangSet::insertEdgeWangColor(QSharedPointer<WangColor> wangColor)
{
    Q_ASSERT(edgeColorCount() + 1 >= wangColor->colorIndex());

    mEdgeColors.insert(wangColor->colorIndex() - 1, wangColor);

    for (int i = wangColor->colorIndex(); i < edgeColorCount(); ++i)
        mEdgeColors.at(i)->setColorIndex(i + 1);
}

void WangSet::insertCornerWangColor(QSharedPointer<WangColor> wangColor)
{
    Q_ASSERT(cornerColorCount() + 1 >= wangColor->colorIndex());

    mCornerColors.insert(wangColor->colorIndex() - 1, wangColor);

    for (int i = wangColor->colorIndex(); i < cornerColorCount(); ++i)
        mCornerColors.at(i)->setColorIndex(i + 1);
}

void WangSet::removeWangColorAt(int color, bool isEdge)
{
    if (isEdge)
        removeEdgeWangColor(color);
    else
        removeCornerWangColor(color);
}

void WangSet::removeEdgeWangColor(int color)
{
    Q_ASSERT(edgeColorCount() > 1 && color <= edgeColorCount() - 1);

    mEdgeColors.removeAt(color - 1);

    for (int i = color - 1; i < edgeColorCount(); ++i)
        mEdgeColors.at(i)->setColorIndex(i + 1);
}

void WangSet::removeCornerWangColor(int color)
{
    Q_ASSERT(cornerColorCount() > 1 && color <= cornerColorCount() - 1);

    mCornerColors.removeAt(color - 1);

    for (int i = color - 1; i < cornerColorCount(); ++i)
        mCornerColors.at(i)->setColorIndex(i + 1);
}

QSharedPointer<WangColor> WangSet::edgeColorAt(int index) const
{
    Q_ASSERT(index > 0 && index <= edgeColorCount());

    return mEdgeColors.at(index - 1);
}

QSharedPointer<WangColor> WangSet::cornerColorAt(int index) const
{
    Q_ASSERT(index > 0 && index <= cornerColorCount());

    return mCornerColors.at(index - 1);
}

QList<Tile *> WangSet::tilesChangedOnSetEdgeColors(int newEdgeColors) const
{
    QList<Tile *> tiles;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        if (!wangIdIsValid(i.value(), newEdgeColors, cornerColorCount())) {
            int tileId = i.key() & 0x1fffffff;
            tiles.append(mTileset->tileAt(tileId));
        }
    }

    return tiles;
}

QList<Tile *> WangSet::tilesChangedOnSetCornerColors(int newCornerColors) const
{
    QList<Tile *> tiles;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        if (!wangIdIsValid(i.value(), edgeColorCount(), newCornerColors)) {
            int tileId = i.key() & 0x1fffffff;
            tiles.append(mTileset->tileAt(tileId));
        }
    }

    return tiles;
}

QList<Tile *> WangSet::tilesChangedOnRemoveColor(int color, bool isEdge) const
{
    QList<Tile *> tiles;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        for (int j = 0; j < 4; ++j) {
            int c = isEdge? i.value().edgeColor(j) : i.value().cornerColor(j);
            int tileId = i.key() & 0x1fffffff;
            if (c >= color) {
                tiles.append(mTileset->tileAt(tileId));
                break;
            }
            if (isEdge) {
                if (edgeColorCount() == 2 && c) {
                    tiles.append(mTileset->tileAt(tileId));
                    break;
                }
            } else {
                if (cornerColorCount() == 2 && c) {
                    tiles.append(mTileset->tileAt(tileId));
                    break;
                }
            }
        }
    }

    return tiles;
}

void WangSet::addTile(Tile *tile, WangId wangId)
{
    addWangTile(WangTile(tile, wangId));
}

void WangSet::addCell(const Cell &cell, WangId wangId)
{
    addWangTile(WangTile(cell, wangId));
}

void WangSet::addWangTile(const WangTile &wangTile)
{
    Q_ASSERT(wangTile.tile()->tileset() == mTileset);
    Q_ASSERT(wangIdIsValid(wangTile.wangId()));

    if (WangId previousWangId = mTileInfoToWangId.value(wangTileToTileInfo(wangTile))) {
        if (wangTile.wangId() == 0) {
            removeWangTile(wangTile);
            return;
        }
        if (previousWangId == wangTile.wangId())
            return;
        removeWangTile(wangTile);
    }

    if (wangTile.wangId() == 0)
        return;

    if ((edgeColorCount() <= 1 || !wangTile.wangId().hasEdgeWildCards())
            && (cornerColorCount() <= 1 || !wangTile.wangId().hasCornerWildCards())
            && !mWangIdToWangTile.contains(wangTile.wangId()))
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
            && (edgeColorCount() <= 1 || !wangId.hasEdgeWildCards())
            && (cornerColorCount() <= 1 || !wangId.hasCornerWildCards()))
        --mUniqueFullWangIdCount;
}

QList<WangTile> WangSet::wangTiles() const
{
    QList<WangTile> wangTiles = mWangIdToWangTile.values();

    qStableSort(wangTiles.begin(), wangTiles.end());
    return wangTiles;
}

QList<WangTile> WangSet::findMatchingWangTiles(WangId wangId) const
{
    if (wangId == 0)
        return mWangIdToWangTile.values();

    QList<WangTile> list;

    for (WangId id : wangId.variations(edgeColorCount(), cornerColorCount())) {
        auto i = mWangIdToWangTile.find(id);
        while (i != mWangIdToWangTile.end() && i.key() == id) {
            list.append(i.value());
            ++i;
        }
    }

    return list;
}

WangId WangSet::wangIdFromSurrounding(WangId surroundingWangIds[]) const
{
    unsigned id = 0;

    if (edgeColorCount() > 1) {
        for (int i = 0; i < 4; ++i)
            id |= (surroundingWangIds[i*2].edgeColor((2 + i) % 4)) << (i*8);
    }

    if (cornerColorCount() > 1) {
        for (int i = 0; i < 4; ++i) {
            int color = surroundingWangIds[i*2 + 1].cornerColor((2 + i) % 4);

            if (!color)
                color = surroundingWangIds[i*2].cornerColor((1 + i) % 4);

            if (!color)
                color = surroundingWangIds[(i*2 + 2) % 8].cornerColor((3 + i) % 4);

            id |= color << (4 + i*8);
        }
    }

    return id;
}

WangId WangSet::wangIdFromSurrounding(const Cell surroundingCells[]) const
{
    WangId wangIds[8];

    for (int i = 0; i < 8; ++i)
        wangIds[i] = wangIdOfCell(surroundingCells[i]);

    return wangIdFromSurrounding(wangIds);
}

QList<Tile *> WangSet::tilesWithWangId() const
{
    if (!mTileset)
        return QList<Tile *>();

    QList<Tile *> tiles;

    for (WangTile wangTile : mWangIdToWangTile)
        tiles.append(wangTile.tile());

    return tiles;
}

WangId WangSet::wangIdOfTile(const Tile *tile) const
{
    if (tile->tileset() == mTileset)
        return mTileInfoToWangId.value(tile->id());
    else
        return 0;
}

WangId WangSet::wangIdOfCell(const Cell &cell) const
{
    if (cell.tileset() == mTileset)
        return mTileInfoToWangId.value(cellToTileInfo(cell));
    else
        return 0;
}

float WangSet::wangTileProbability(const WangTile &wangTile) const
{
    float probability = 1.0f;
    WangId wangId = wangTile.wangId();

    if (edgeColorCount() > 1) {
        for (int i = 0; i < 4; ++i) {
            if (int color = wangId.edgeColor(i))
                probability *= edgeColorAt(color)->probability();
        }
    }

    if (cornerColorCount() > 1) {
        for (int i = 0; i < 4; ++i) {
            if (int color = wangId.cornerColor(i))
                probability *= cornerColorAt(color)->probability();
        }
    }

    if (Tile *tile = wangTile.tile())
        probability *= tile->probability();

    return probability;
}

bool WangSet::wangIdIsValid(WangId wangId) const
{
    return wangIdIsValid(wangId, edgeColorCount(), cornerColorCount());
}

bool WangSet::wangIdIsValid(WangId wangId, int edgeCount, int cornerCount)
{
    for (int i = 0; i < 4; ++i) {
        if (wangId.edgeColor(i) > edgeCount
                || wangId.cornerColor(i) > cornerCount)
            return false;

        if (edgeCount <= 1)
            if (wangId.edgeColor(i))
                return false;

        if (cornerCount <= 1)
            if (wangId.cornerColor(i))
                return false;
    }

    return true;
}

bool WangSet::wangIdIsUsed(WangId wangId) const
{
    return mWangIdToWangTile.contains(wangId);
}

bool WangSet::wildWangIdIsUsed(WangId wangId) const
{
    if (isEmpty())
        return false;
    if (!wangId)
        return true;

    for (WangId id : wangId.variations(edgeColorCount(), cornerColorCount())) {
        if (wangIdIsUsed(id))
            return true;
    }

    return false;
}

bool WangSet::isComplete() const
{
    return mUniqueFullWangIdCount == completeSetSize();
}

unsigned WangSet::completeSetSize() const
{
    return qPow(edgeColorCount(), 4) * qPow(cornerColorCount(), 4);
}

WangId WangSet::templateWangIdAt(unsigned n) const
{
    unsigned wangId = 0;
    //number of permutations of a corner and edge together.
    int cornerEdgePermutations = edgeColorCount() * cornerColorCount();

    for (int i = 7; i >= 0; --i) {
        //this is the number of permutations possible bellow this point in the wangId
        int belowPermutations = qPow(cornerEdgePermutations, i/2) * ((i&1)? edgeColorCount() : 1);
        int value = n / belowPermutations;
        n -= value * belowPermutations;

        wangId |= value << i * 4;
    }

    //before this is like a base 10 range (0 - 9) where we want (1 - 10) for each digit
    wangId += 0x11111111;
    //If edges/corners don't have variations then those spots should be wild.
    if (edgeColorCount() <= 1)
        wangId &= 0xf0f0f0f0;
    if (cornerColorCount() <= 1)
        wangId &= 0x0f0f0f0f;

    return wangId;
}

WangSet *WangSet::clone(Tileset *tileset) const
{
    WangSet *c = new WangSet(tileset, mName, mImageTileId);

    c->mWangIdToWangTile = mWangIdToWangTile;
    c->mTileInfoToWangId = mTileInfoToWangId;

    return c;
}
